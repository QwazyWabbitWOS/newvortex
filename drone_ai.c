#include "g_local.h"

#define DRONE_TARGET_RANGE		1024	// maximum range for finding targets
#define DRONE_ALLY_RANGE		512		// range allied units can be called for assistance
#define DRONE_FOV				90		// viewing fov used by drone
#define DRONE_MIN_GOAL_DIST		32		// goals are tagged as being reached at this distance
#define DRONE_FINDTARGET_FRAMES	2		// drones search for targets every few frames
#define	DRONE_TELEPORT_DELAY	30		// delay in seconds before a drone can teleport
#define DRONE_SLEEP_FRAMES		100		// frames before drone becomes less alert
#define DRONE_SEARCH_TIMEOUT	300	// frames before drone gives up trying to reach an enemy
#define DRONE_SUICIDE_FRAMES	1800	// idle frames before a world monster suicides
#define STEPHEIGHT				18		// standard quake2 step size

#ifdef max
#undef max
#endif

qboolean drone_ValidChaseTarget (edict_t *self, edict_t *target);
float distance (vec3_t p1, vec3_t p2);
edict_t *SpawnGoalEntity (edict_t *ent, vec3_t org);
void drone_ai_checkattack (edict_t *self);
qboolean drone_findtarget (edict_t *self);
int max(int a, int b);

int max(int a, int b) {
	return a > b ? a : b;
}

/*
=============
ai_move

Move the specified distance at current facing.
This replaces the QC functions: ai_forward, ai_back, ai_pain, and ai_painforward
==============
*/
void ai_move (edict_t *self, float dist)
{
	M_walkmove (self, self->s.angles[YAW], dist);
}

/*
=============
ai_charge

Turns towards target and advances
Use this call with a distance of 0 to replace ai_face
==============
*/
void ai_charge (edict_t *self, float dist)
{
	vec3_t	v;

	if (!self || !self->inuse)
		return;

	// if our enemy is invalid, try to find a new one
	if (!G_ValidTarget(self, self->enemy, true) && !drone_findtarget(self))
		return;

	VectorSubtract (self->enemy->s.origin, self->s.origin, v);
	self->ideal_yaw = vectoyaw(v);
	M_ChangeYaw (self);
}

/*
=============
ai_run_slide

Strafe sideways, but stay at aproximately the same range
=============
*/
void ai_run_slide(edict_t *self, float distance)
{
	float	ofs;

	if (!self->enemy)
		return;

	self->ideal_yaw = self->enemy->s.angles[YAW];
	M_ChangeYaw (self);

	if (self->monsterinfo.lefty)
		ofs = 90;
	else
		ofs = -90;
	
	if (M_walkmove (self, self->ideal_yaw + ofs, distance))
		return;
		
	self->monsterinfo.lefty = 1 - self->monsterinfo.lefty;
	M_walkmove (self, self->ideal_yaw - ofs, distance);

	// walk backwards to maintain distance
	if (entdist(self, self->enemy)+distance < 256)
		M_walkmove(self, self->s.angles[YAW], -distance);
}

/*
=============
drone_wakeallies

Makes allied drones chase our enemy if they aren't busy
=============
*/
void drone_wakeallies (edict_t *self)
{
	edict_t *e=NULL;

	if (!self->enemy)
		return; // what are we doing here?

	// if allied monsters arent chasing a visible enemy
	// then enlist their help
	while ((e = findradius (e, self->s.origin, DRONE_ALLY_RANGE)) != NULL)
	{
		if (!G_EntExists(e))
			continue;
		if (e->client)
			continue;
		if (!(e->svflags & SVF_MONSTER))
			continue;
		if (strcmp(e->classname, "drone"))
			continue;
		if (!OnSameTeam(self, e))
			continue;
		if (!visible(self, e))
			continue;
		if (e->enemy && visible(e, e->enemy))
			continue;
	//	gi.dprintf("ally found!\n");
		e->enemy = self->enemy;
	}
}

qboolean M_ValidMedicTarget (edict_t *self, edict_t *target)
{
	if (target == self)
		return false;
	if (!G_EntExists(target))
		return false;
	//gi.dprintf("looking for medic target\n");
	// don't target players with invulnerability
	if (target->client && (target->client->invincible_framenum > level.framenum))
		return false;
	// don't target spawning players
	if (target->client && (target->client->respawn_time > level.time))
		return false;
	// don't target players in chat-protect
	if (target->client && ((target->flags & FL_CHATPROTECT) || (target->flags & FL_GODMODE)))
		return false;
	// don't target frozen players
	if (que_typeexists(target->curses, CURSE_FROZEN))
		return false;
	// invasion medics shouldn't try to heal base defenders
	if (invasion->value && (self->monsterinfo.aiflags & AI_FIND_NAVI) && !(target->monsterinfo.aiflags & AI_FIND_NAVI))
		return false;

	// the target is dead
	if ((target->health < 1) || (target->deadflag == DEAD_DEAD))
	{
		// don't target player or non-monster corpses
		if (target->client || strcmp(target->classname, "drone"))
		{
			//gi.dprintf("invalid corpse\n");
			return false;
		}

		// if we're owned by a player, don't spawn more monsters than our limit
		if (self->activator && self->activator->client && (self->activator->num_monsters
			+ target->monsterinfo.control_cost > MAX_MONSTERS))
			return false;
	}
	// target must be on our team and require healing
	else if (!OnSameTeam(self, target) || !M_NeedRegen(target))
		return false;

	if (!visible(self, target))
		return false;

	//gi.dprintf("found medic target\n");
	return true;
}

qboolean drone_validnavi (edict_t *self, edict_t *other, qboolean visibility_check)
{
	// only world-spawn monsters should search for invasion mode navi
	// don't bother if they are not mobile
	if (self->activator->client || (self->monsterinfo.aiflags & AI_STAND_GROUND))
		return false;
	// make sure this is actually a beacon
	if (!other || !other->inuse || (other->mtype != INVASION_NAVI))
		return false;
	// do a visibility check if required
	if (visibility_check && !visible(self, other))
		return false;
	return true;
}

qboolean drone_heartarget (edict_t *target)
{
	// monster always senses other AI
	if (!target->client)
		return true;

	// target made a sound (necessary mostly for weapon firing)
	if (target->lastsound + 4 >= level.framenum)
		return true;

	// target used an ability
	if (target->client->ability_delay + 0.4 >= level.time)
		return true;

	return false;
}
/*
=============
drone_findtarget

Searches a spherical area for enemies and 
returns true if one is found within range
=============
*/
qboolean drone_findtarget (edict_t *self)
{
	int			frames, range=self->monsterinfo.sight_range;
	edict_t		*target=NULL;
	qboolean	foundnavi=false;

	if (level.time < pregame_time->value)
		return false; // pre-game time

	// if a monster hasn't found a target for awhile, it becomes less alert
	// and searches less often, freeing up CPU cycles
	if (!(self->monsterinfo.aiflags & AI_STAND_GROUND)
		&& self->monsterinfo.idle_frames > DRONE_SLEEP_FRAMES)
		frames = 4;
	else
		frames = DRONE_FINDTARGET_FRAMES;

	if (level.framenum % frames)
		return false;

	// if we're a medic, do a sweep for those with medical need!
	if (self->monsterinfo.aiflags & AI_MEDIC)
	{
		// don't bother finding targets out of our range
		if (self->monsterinfo.aiflags & AI_STAND_GROUND)
			range = 256;

		while ((target = findclosestradius (target, self->s.origin, range)) != NULL)
		{
			if (!M_ValidMedicTarget(self, target))
				continue;

			self->enemy = target;
			return true;
		}
	}

	// find an enemy
	while ((target = findclosestradius (target, self->s.origin, 
		self->monsterinfo.sight_range)) != NULL)
	{
		if (!G_ValidTarget(self, target, true))
			continue;
		// limit drone/monster FOV, but check for recent sounds
		if (!nearfov(self, target, 0, DRONE_FOV) && !drone_heartarget(target))//(level.framenum > target->lastsound + 5))
			continue;

		//4.4 if this is an invasion player spawn (base), then clear goal AI flags
		if (target->mtype == INVASION_PLAYERSPAWN)
		{
			self->monsterinfo.aiflags &= ~AI_FIND_NAVI;
			if (!self->monsterinfo.melee)
				self->monsterinfo.aiflags  &= ~AI_NO_CIRCLE_STRAFE;
		}

		self->enemy = target;

		// trigger sight
		if (self->monsterinfo.sight)
			self->monsterinfo.sight(self, self->enemy);

		// alert nearby monsters and make them chase our enemy
		drone_wakeallies(self);
		//gi.dprintf("found an enemy\n");
		return true;
	}

	// find a navigational beacon IF we're not already chasing something
	// monster must be owned by world and not instructed to ignore navi
	if (!self->enemy && (self->monsterinfo.aiflags & AI_FIND_NAVI))
	{
		while ((target = findclosestradius1 (target, self->s.origin, 
			self->monsterinfo.sight_range)) != NULL)
		{
			if (!drone_validnavi(self, target, true))
				continue;

			// set ai flags to chase goal and turn off circle strafing
			self->monsterinfo.aiflags |= (AI_COMBAT_POINT|AI_NO_CIRCLE_STRAFE);
			
			//gi.dprintf("found navi\n");
			self->enemy = target;
			return true;
		}
	}

	return false;
}

/*
=============
drone_ai_stand

Called when the drone isn't chasing an enemy or goal
It is also called when the drone is holding position
=============
*/
void drone_ai_stand (edict_t *self, float dist)
{
	vec3_t	v;

	if (self->deadflag == DEAD_DEAD)
		return;
	if (self->monsterinfo.pausetime > level.time)
		return;

	//gi.dprintf("drone_ai_stand\n");

	// used for slight position adjustments for animations
	if (dist)
		M_walkmove(self, self->s.angles[YAW], dist);

	if (!self->enemy)
	{
		// if we had a previous enemy, go after him
		if (G_EntExists(self->oldenemy))
		{
			if (drone_ValidChaseTarget(self, self->oldenemy))
			{
				self->enemy = self->oldenemy;
				self->monsterinfo.run(self);
			}
			else
			{
				// no longer valid, so forget about him
				self->oldenemy = NULL;
				return;
			}
		}
		else if (drone_findtarget(self))
		{
			self->monsterinfo.run(self);
			//if (self->monsterinfo.sight)
			//	self->monsterinfo.sight(self, self->enemy);
		}
		else // didn't find anything to get mad at
		{
			// regenerate to full in 30 seconds
			//if (self->monsterinfo.control_cost < 3) // non-boss
			if (self->mtype == M_MEDIC)
				M_Regenerate(self, 300, 10, true, true, false, &self->monsterinfo.regen_delay1);
			else if (self->health >= 0.3*self->max_health)
			// change skin if we are being healed by someone else
				self->s.skinnum &= ~1;

			// call idle func every so often
			if (self->monsterinfo.idle && (level.time > self->monsterinfo.idle_delay))
			{
				self->monsterinfo.idle(self);
				self->monsterinfo.idle_delay = level.time + GetRandom(15, 30);
			}
			self->monsterinfo.idle_frames++;

			// world monsters suicide if they haven't found an enemy in awhile
			if (self->activator && !self->activator->client && !(self->monsterinfo.aiflags & AI_STAND_GROUND)
				&& (self->monsterinfo.idle_frames > DRONE_SUICIDE_FRAMES))
			{
				if (self->monsterinfo.control_cost > 2) // we're a boss
					self->activator->num_sentries--;
				if (self->activator)
					self->activator->num_monsters -= self->monsterinfo.control_cost;
				if (self->activator->num_monsters < 0)
					self->activator->num_monsters = 0;
				BecomeTE(self);
				return;
			}
		}
	}
	else
	{
		// we're not going anywhere
		if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		{
			// if enemy isn't visible, ignore him
			if (!visible(self, self->enemy))
			{
				self->enemy = NULL;
				return;
			}

			if (drone_ValidChaseTarget(self, self->enemy))
			{
				// turn towards our enemy
				VectorSubtract(self->enemy->s.origin, self->s.origin, v);
				self->ideal_yaw = vectoyaw(v);
				M_ChangeYaw(self);
				drone_ai_checkattack(self);
				return;
			}
			else
			{
				self->enemy = NULL;
				return;
			}
		}

		if (drone_ValidChaseTarget(self, self->enemy))
			self->monsterinfo.run(self);
		else if (drone_findtarget(self))
			self->monsterinfo.run(self);
		else
			self->enemy = NULL;
	}
}

#define	STATE_TOP			0
#define	STATE_BOTTOM		1
#define STATE_UP			2
#define STATE_DOWN			3

qboolean G_IsClearPath (edict_t *ignore, int mask, vec3_t spot1, vec3_t spot2);

/*
=============
FindPlat
returns true if a nearby platform/elevator is found

self		the entity searching for the platform
plat_pos	the position of the platform (if found)	
=============
*/
qboolean FindPlat (edict_t *self, vec3_t plat_pos)
{
	vec3_t	start, end;
	edict_t *e=NULL;
	trace_t	tr;

	if (!self->enemy)
		return false; // what are we doing here?

	while((e = G_Find(e, FOFS(classname), "func_plat")) != NULL) 
	{
		// this is an ugly hack, but it's the only way to test the distance
		// or visiblity of a plat, since the origin and bbox yield no useful
		// information
		tr = gi.trace(self->s.origin, NULL, NULL, e->absmax, self, MASK_SOLID);
		VectorCopy(tr.endpos, start);
		VectorCopy(tr.endpos, end);
		end[2] -= 8192;
		tr = gi.trace(start, NULL, NULL, end, self, MASK_SOLID);

		VectorCopy(tr.endpos, start);
		VectorCopy(tr.endpos, plat_pos);
		if (!tr.ent || (tr.ent && (tr.ent != e)))
			continue; // we can't see this plat
		if (distance(start, self->s.origin) > 512)
			continue; // too far
		if (e->moveinfo.state != STATE_BOTTOM)
			continue; // plat must be down
		if (start[2] > self->absmin[2]+2*STEPHEIGHT)
			continue;

		VectorCopy(start, end);
		end[2] += abs(e->s.origin[2]);
		if (distance(end, self->enemy->s.origin)
			> distance(self->s.origin, self->enemy->s.origin))
			continue; // plat must bring us closer to our goal

		self->goalentity = e;
		self->monsterinfo.aiflags |= AI_PURSUE_PLAT_GOAL;
		return true;
	}
	return false;
}

/*
=============
FindHigherGoal
finds a goal higher than our current position and moves to it


self	the drone searching for the goal
dist	the distance the drone wants to move
=============
*/
void FindHigherGoal (edict_t *self, float dist)
{
	int		i;
	float	yaw, range=128;
	vec3_t	forward, start, end, best, angles;
	trace_t	tr;

//	gi.dprintf("finding a higher goal\n");
	VectorCopy(self->absmin, best);

	// try to run forward first
	VectorCopy(self->s.origin, start);
	start[2] = self->absmin[2]+2*STEPHEIGHT;
	AngleVectors(self->s.angles, forward, NULL, NULL);
	VectorMA(start, 128, forward, end);
	tr = gi.trace(start, NULL, NULL, end, self, MASK_SOLID);
	vectoangles(tr.plane.normal, angles);
	AngleCheck(&angles[PITCH]);
	if (angles[PITCH] != 0)
	{
		self->ideal_yaw = vectoyaw(forward);
		M_MoveToGoal(self, dist);
		return;
	}

	while (range <= 512)
	{
		// check 8 angles at 45 degree intervals
		for(i=0; i<8; i++)
		{
			yaw = anglemod(i*45);
			forward[0] = cos(DEG2RAD(yaw));
			forward[1] = sin(DEG2RAD(yaw));
			forward[2] = 0;

			// start scanning above step height
			VectorCopy(self->s.origin, start);
			if (self->waterlevel > 1)
				start[2] = self->absmax[2];//VectorCopy(self->absmax, start);
			else
				start[2] = self->absmin[2];//VectorCopy(self->absmin, start);
			start[2] += 2*STEPHEIGHT;
			// dont trace too far forward, or you will find multiple paths!
			VectorMA(start, range, forward, end);
			tr = gi.trace(start, NULL, NULL, end, self, MASK_SOLID);
			// trace down
			VectorCopy(tr.endpos, start);
			VectorCopy(tr.endpos, end);
			end[2] -= 8192;
			tr = gi.trace(start, NULL, NULL, end, self, MASK_SOLID);
			if (distance(self->absmin, tr.endpos) < DRONE_MIN_GOAL_DIST)
				continue; // too close
			// if we can still see our enemy, dont take a path that will
			// put him out of sight
			if (self->enemy && !(self->monsterinfo.aiflags & AI_LOST_SIGHT)
				&& !G_IsClearPath(self, MASK_SOLID, tr.endpos, self->enemy->s.origin))
				continue;
			vectoangles(tr.plane.normal, angles);
			if (angles[PITCH] > 360)
				angles[PITCH] -= 360;
			if (angles[PITCH] < 360)
				angles[PITCH] += 360;
			if (angles[PITCH] != 270)
				continue; // must be flat ground, or we risk getting stuck
			// is this point higher than the last?
			if (tr.endpos[2] > best[2])
				VectorCopy(tr.endpos, best);
			// we may find more than one position at the same height
			// so take it if it's farther from us
			else if ((tr.endpos[2] == best[2]) && (distance(self->absmin, 
				tr.endpos) > distance(self->absmin, best)))
				VectorCopy(tr.endpos, best);
		}

		// were we able to find anything?
		if (best[2] <= self->absmin[2]+1)
			range *= 2;
		else
			break;
	}

	if (range > 512)
	{
	//	gi.dprintf("couldnt find a higher goal!!!\n");
		self->goalentity = self->enemy;
		M_MoveToGoal(self, dist);
		return;
	}

//	gi.dprintf("found higher goal at %d, current %d\n", (int)best[2], (int)self->absmin[2]);
	
	self->goalentity = SpawnGoalEntity(self, best);
	VectorSubtract(self->goalentity->s.origin, self->s.origin, forward);
	self->ideal_yaw = vectoyaw(forward);
	M_MoveToGoal(self, dist);
	
}

/*
=============
FindLowerGoal
finds a goal lower than our current position and moves to it


self	the drone searching for the goal
dist	the distance the drone wants to move
=============
*/
void FindLowerGoal (edict_t *self, float dist)
{
	int		i;
	float	yaw, range=128;
	vec3_t	forward, start, end, best;
	trace_t	tr;

//	gi.dprintf("finding a lower goal\n");
	VectorCopy(self->absmin, best);

	while (range <= 512)
	{
		// check 8 angles at 45 degree intervals
		for(i=0; i<8; i++)
		{
			yaw = anglemod(i*45);
			forward[0] = cos(DEG2RAD(yaw));
			forward[1] = sin(DEG2RAD(yaw));
			forward[2] = 0;

			// start scanning above step height
			VectorCopy(self->s.origin, start);
			//VectorCopy(self->absmin, start);
			//start[2] += STEPHEIGHT;
			start[2] = self->absmin[2] + STEPHEIGHT;
			// dont trace too far forward, or you will find multiple paths!
			VectorMA(start, range, forward, end);
			tr = gi.trace(start, NULL, NULL, end, self, MASK_SOLID);
			// trace down
			VectorCopy(tr.endpos, start);
			VectorCopy(tr.endpos, end);
			end[2] -= 64;
			tr = gi.trace(start, NULL, NULL, end, self, MASK_SOLID);

			if ((tr.fraction == 1) && (self->monsterinfo.jumpdn < 1))
				continue; // don't fall off an edge
			if (distance(self->absmin, tr.endpos) < DRONE_MIN_GOAL_DIST)
				continue; // too close
			// is this point lower than the last?
			if ((tr.endpos[2] < best[2]))
				VectorCopy(tr.endpos, best);
			// we may find more than one position at the same height
			// so take it if it's farther from us
			else if ((tr.endpos[2] == best[2]) && (distance(self->absmin, 
				tr.endpos) > distance(self->absmin, best)))
				VectorCopy(tr.endpos, best);
		}

		// were we able to find anything?
		if (VectorEmpty(best) || (best[2] >= self->absmin[2]))
			range *= 2;
		else
			break;
	}

	if (range > 512)
	{
	//	gi.dprintf("couldn't find a goal!!!\n");
		self->goalentity = self->enemy;
		M_MoveToGoal(self, dist);
		return;
	}

	self->goalentity = SpawnGoalEntity(self, best);
	VectorSubtract(self->goalentity->s.origin, self->s.origin, forward);
	self->ideal_yaw = vectoyaw(forward);
	M_MoveToGoal(self, dist);
	

}

/*
qboolean FollowWall (edict_t *self, vec3_t endpos, vec3_t wall_normal, float dist)
{
	int		i;
	vec3_t	forward, start, end, angles;
	trace_t tr;

	if (self->monsterinfo.aiflags & AI_PURSUIT_LAST_SEEN)
		return false; // give a chance for the regular AI to work

	for (i=90; i<180; i*=2)
	{
		
		vectoangles(wall_normal, angles);
		angles[YAW] += i;
		if (angles[YAW] > 360)
			angles[YAW] -= 360;
		if (angles[YAW] < 360)
			angles[YAW] += 360;
		forward[0] = cos(DEG2RAD(angles[YAW]));
		forward[1] = sin(DEG2RAD(angles[YAW]));
		forward[2] = 0;

		VectorCopy(endpos, start);
		VectorMA(start, 64, forward, end);
		tr = gi.trace(start, NULL, NULL, end, self, MASK_SHOT);
		if (tr.fraction < 1)
			continue;
		VectorCopy(tr.endpos, start);
		VectorCopy(tr.endpos, end);
		end[2] -= 64;
		tr = gi.trace(start, NULL, NULL, end, self, MASK_SOLID);
		if (tr.fraction == 1) // don't fall
			continue;
		self->goalentity = SpawnGoalEntity(self, tr.endpos);
		VectorSubtract(self->goalentity->s.origin, self->s.origin, forwa
			rd);
		self->ideal_yaw = vectoyaw(forward);
		M_MoveToGoal(self, dist);
		gi.dprintf("following wall at %d degrees...\n", i);
		return true;
	}
	gi.dprintf("*** FAILED at %d degrees ***\n", i);
	return false;
	
}
*/

void FindCloserGoal (edict_t *self, vec3_t target_origin, float dist)
{
	int		i;
	float	yaw, best_dist=8192;
	vec3_t	forward, start, end, best;
	trace_t	tr;

//	gi.dprintf("finding a closer goal\n");
	VectorCopy(self->s.origin, best);
	// check 8 angles at 45 degree intervals
	for(i=0; i<8; i++)
	{
		yaw = anglemod(i*45);
		forward[0] = cos(DEG2RAD(yaw));
		forward[1] = sin(DEG2RAD(yaw));
		forward[2] = 0;

		// start scanning above step height
		VectorCopy(self->absmin, start);
		start[2] += STEPHEIGHT;
		// trace forward
		VectorMA(start, 64, forward, end);
		tr = gi.trace(start, NULL, NULL, end, self, MASK_SHOT);

		// trace down
		VectorCopy(tr.endpos, start);
		VectorCopy(tr.endpos, end);
		end[2] -= 64;
		tr = gi.trace(start, NULL, NULL, end, self, MASK_SOLID);

		if (tr.fraction == 1) // don't fall
			continue;
		if (distance(self->absmin, tr.endpos) < DRONE_MIN_GOAL_DIST)
			continue; // too close

		// is this point closer?
		if (distance(target_origin, tr.endpos) < best_dist)
		{
			best_dist = distance(target_origin, tr.endpos);
			VectorCopy(tr.endpos, best);
		}
	}

	if (distance(best, target_origin) >= distance(tr.endpos, target_origin))
	{
		self->goalentity = self->enemy;
		M_MoveToGoal(self, dist);
	//	gi.dprintf("couldnt find a closer goal!!!\n");
		return;
	}

	self->goalentity = SpawnGoalEntity(self, best);
	VectorSubtract(self->goalentity->s.origin, self->s.origin, forward);
	self->ideal_yaw = vectoyaw(forward);
	M_MoveToGoal(self, dist);
}

void drone_unstuck (edict_t *self)
{
	int		i, yaw;
	vec3_t	forward, start;
	trace_t	tr;

	// check 8 angles at 45 degree intervals
	for(i=0; i<8; i++)
	{
		// get new vector
		yaw = anglemod(i*45);
		forward[0] = cos(DEG2RAD(yaw));
		forward[1] = sin(DEG2RAD(yaw));
		forward[2] = 0;
		
		// trace from current position
		VectorMA(self->s.origin, 64, forward, start);
		tr = gi.trace(self->s.origin, self->mins, self->maxs, start, self, MASK_SHOT);
		if ((tr.fraction == 1) && !(gi.pointcontents(start) & CONTENTS_SOLID))
		{
			VectorCopy(tr.endpos, self->s.origin);
			gi.linkentity(self);
			self->monsterinfo.air_frames = 0;
			//gi.dprintf("freed stuck monster!\n");
			return;
		}

	}
//	gi.dprintf("couldnt fix\n");
}

void drone_pursue_goal (edict_t *self, float dist)
{
	float	goal_elevation;
	vec3_t	goal_origin, v;
	vec3_t	forward, start, end;
	trace_t	tr;

	// if we are not on the ground and we can see our goal
	// then turn towards it
	if (!self->groundentity && !self->waterlevel && visible(self, self->goalentity))
	{
		//gi.dprintf("off ground %d\n", level.framenum);
		VectorSubtract(self->goalentity->s.origin, self->s.origin, v);
		self->ideal_yaw = vectoyaw(v);
		M_ChangeYaw(self);

		if (!self->waterlevel)
			self->monsterinfo.air_frames++;
		if (self->monsterinfo.air_frames > 20)
			drone_unstuck(self);
		return;
	}

	self->monsterinfo.air_frames = 0;

	// don't bother pursuing an enemy that isn't on the ground
	if (self->goalentity->groundentity || !self->goalentity->takedamage
		|| (self->waterlevel != self->goalentity->waterlevel))
	{
		// try to find a plat first
		if (FindPlat(self, goal_origin))
		{
			// top of plat
			goal_elevation = goal_origin[2];
		}
		else
		{
			goal_elevation = self->goalentity->absmin[2]; // ent's feet
			VectorCopy(self->goalentity->s.origin, goal_origin);
		}


		// can we see our enemy?
		if (self->monsterinfo.aiflags & AI_LOST_SIGHT)
		{
			// is our enemy higher?
			if (goal_elevation > self->absmin[2]+2*STEPHEIGHT)
				FindHigherGoal(self, dist);
			// is our enemy lower?
			else if (goal_elevation < self->absmin[2]-2*STEPHEIGHT)
				FindLowerGoal(self, dist);
			else
				FindCloserGoal(self, goal_origin, dist);
		}
		else
		{
			// is our enemy higher?
			if (goal_elevation > self->absmin[2]+max(self->monsterinfo.jumpup, 2*STEPHEIGHT)+1)
				FindHigherGoal(self, dist);
			// is our enemy lower?
			else if (goal_elevation < self->absmin[2]-max(self->monsterinfo.jumpdn, 2*STEPHEIGHT))
				FindLowerGoal(self, dist);
			// is anyone standing in our way?
			else
			{
				VectorCopy(self->s.origin, start);
				AngleVectors(self->s.angles, forward, NULL, NULL);
				VectorMA(self->s.origin, self->maxs[1]+dist, forward, start);
				VectorCopy(start, end);
				end[2] -= 2*STEPHEIGHT;
				tr = gi.trace(start, NULL, NULL, end, self, MASK_SOLID);
				// did we fall off an edge?
				if (tr.fraction == 1.0)
				{
					M_MoveToGoal(self, dist);
					return;
				}

				tr = gi.trace(self->s.origin, NULL, NULL, goal_origin, self, MASK_SOLID);
				// is anyone in our way?
				//if (!tr.ent || (tr.ent != self->goalentity))

				// is the path clear of obstruction?
				if (tr.fraction < 1)
				{
					M_MoveToGoal(self, dist);
					return;	
				}

				VectorSubtract(goal_origin, self->s.origin, v);
				self->ideal_yaw = vectoyaw(v);
				M_MoveToGoal(self, dist);
			}
		}
	}
	else
	{
		if (self->monsterinfo.aiflags & AI_LOST_SIGHT)
			FindCloserGoal(self, self->goalentity->s.origin, dist);
		else if (self->waterlevel && self->goalentity->waterlevel)
		{
			VectorSubtract(self->goalentity->s.origin, self->s.origin, v);
			self->ideal_yaw = vectoyaw(v);
			M_MoveToGoal(self, dist);
		}
		else
			M_MoveToGoal(self, dist);
	}
}

void drone_ai_run_slide (edict_t *self, float dist)
{
	float	ofs, range;
	vec3_t	v;
	
	VectorSubtract(self->enemy->s.origin, self->s.origin, v);
	self->ideal_yaw = vectoyaw(v);
	M_ChangeYaw (self);

	//4.4 try to maintain ideal range/distance to target
	range = VectorLength(v) - 196;
	if (fabs(range) >= dist)
	{
		if (range > dist)
			range = dist;
		else if (range < -dist)
			range = -dist;
		M_walkmove (self, self->ideal_yaw, range);
		//gi.dprintf("moved %.0f units\n", range);
	}

	if (self->monsterinfo.lefty)
		ofs = 90;
	else
		ofs = -90;
	
	if (M_walkmove (self, self->ideal_yaw + ofs, dist))
		return;
		
	// change direction and try strafing in the opposite direction
	self->monsterinfo.lefty = 1 - self->monsterinfo.lefty;
	M_walkmove (self, self->ideal_yaw - ofs, dist);
}
		
void TeleportForward (edict_t *ent, vec3_t vec, float dist);

void drone_cleargoal (edict_t *self)
{
	self->enemy = NULL;
	self->oldenemy = NULL;
	self->goalentity = NULL;

	// clear ai flags
	self->monsterinfo.aiflags &= ~AI_COMBAT_POINT;
	if (!self->monsterinfo.melee)
		self->monsterinfo.aiflags  &= ~AI_NO_CIRCLE_STRAFE;

	self->monsterinfo.search_frames = 0;

	self->monsterinfo.stand(self);
}

edict_t *drone_findnavi (edict_t *self)
{
	edict_t *e=NULL;

	while ((e = findclosestradius1 (e, self->s.origin, 
		self->monsterinfo.sight_range)) != NULL)
	{
		if (!drone_validnavi(self, e, true))
			continue;
		return e;
	}
	return NULL;
}

/*
=============
drone_ai_run

Called when the monster is chasing an enemy or goal
=============
*/
void drone_ai_run (edict_t *self, float dist)
{
	float		time;
	vec3_t		start, end, v;
	trace_t		tr;
	edict_t		*tempgoal=NULL;
	qboolean	enemy_vis=false;

	// if we're dead, we shouldn't be here
	if (self->deadflag == DEAD_DEAD)
		return;

	// decoys make step sounds
	if ((self->mtype == M_DECOY) && (level.time > self->wait)) 
	{
		gi.sound (self, CHAN_BODY, gi.soundindex(va("player/step%i.wav", (randomMT()%4)+1)), 1, ATTN_NORM, 0);
		self->wait = level.time + 0.3;
	}

	// monster is no longer idle
	self->monsterinfo.idle_frames = 0;

	// if the drone is standing, we shouldn't be here
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
	{
		self->monsterinfo.stand(self);
		return;
	}

	if (drone_ValidChaseTarget(self, self->enemy))
	{
		if ((self->monsterinfo.aiflags & AI_COMBAT_POINT) && !self->enemy->takedamage)
		{
			if (!drone_findtarget(self) && (entdist(self, self->enemy) <= 64))
			{
				// the goal is a navi
				if (self->enemy->mtype == INVASION_NAVI) 
				{
					edict_t *e;
					
					// we're close enough to this navi; follow the next one in the chain
					if (self->enemy->target && self->enemy->targetname &&
						((e = G_Find(NULL, FOFS(targetname), self->enemy->target)) != NULL))
					{
						//gi.dprintf("following next navi in chain\n");
						self->enemy = e;
					}
					else
					{
						//FIXME: this never happens because monsters usually see the player/bases and begin attacking before they reach the last navi
						// we've reached our final destination
						drone_cleargoal(self);
						self->monsterinfo.aiflags &= ~AI_FIND_NAVI;
						//gi.dprintf("reached final destination\n");
						return;
					}
				}
				else
				{
					// the goal is a combat point
					//gi.dprintf("cleared combat point\n");
					drone_cleargoal(self);
					return;
				}
			}
		}

		drone_ai_checkattack(self); // try attacking

		// constantly update the last place we remember
		// seeing our enemy
		if (visible(self, self->enemy))
		{
			VectorCopy(self->enemy->s.origin, self->monsterinfo.last_sighting);
			self->monsterinfo.aiflags &= ~(AI_LOST_SIGHT|AI_PURSUIT_LAST_SEEN);
			self->monsterinfo.teleport_delay = level.time + DRONE_TELEPORT_DELAY; // teleport delay
			self->monsterinfo.search_frames = 0;
			enemy_vis = true;

			// circle-strafe our target if we are close
			if ((entdist(self, self->enemy) < 256) 
				&& !(self->monsterinfo.aiflags & AI_NO_CIRCLE_STRAFE))
			{
				drone_ai_run_slide(self, dist);
				return;
			}
		}
		else
		{
			// drone should try to find a navi if enemy isn't visible
			if ((self->monsterinfo.aiflags & AI_FIND_NAVI) && self->enemy->takedamage)
			{
				//gi.dprintf("can't see %s, trying another target...", self->enemy->classname);
				
				// save current target
				self->oldenemy = self->enemy;
				self->enemy = NULL; // must be cleared to find navi

				// try to find a new target
				if (drone_findtarget(self))
				{
					//gi.dprintf("found %s!\n", self->enemy->classname);
					drone_ai_checkattack(self);
				}
				else
				{
					// restore current target
					self->enemy = self->oldenemy;
					self->oldenemy = NULL;
					//gi.dprintf("no target.\n");
				}
			}

			// drones give up if they can't reach their enemy
			if (!self->enemy || (self->monsterinfo.search_frames > DRONE_SEARCH_TIMEOUT))
			{
				//gi.dprintf("drone gave up\n");

				self->enemy = NULL;
				self->oldenemy = NULL;
				self->goalentity = NULL;

				//self->monsterinfo.aiflags &= ~AI_COMBAT_POINT;

				if (!self->monsterinfo.melee)
					self->monsterinfo.aiflags  &= ~AI_NO_CIRCLE_STRAFE;

				self->monsterinfo.stand(self);
				self->monsterinfo.search_frames = 0;
				return;
			}
			self->monsterinfo.search_frames++;
			//gi.dprintf("%d\n", self->monsterinfo.search_frames);
		}

		if (dist < 1)
		{
			VectorSubtract(self->enemy->s.origin, self->s.origin, v);
			self->ideal_yaw = vectoyaw(v);
			M_ChangeYaw(self);
			return;
		}

		// if we are on a platform going up, wait around until we've reached the top
		if (self->groundentity && (self->monsterinfo.aiflags & AI_PURSUE_PLAT_GOAL)
			&& (self->groundentity->style == FUNC_PLAT) 
			&& (self->groundentity->moveinfo.state == STATE_UP)) 
		{
		//	gi.dprintf("standing on plat!\n");
			// divide by speed to get time to reach destination
			time = 0.1 * (abs(self->groundentity->s.origin[2]) / self->groundentity->moveinfo.speed);
			self->monsterinfo.pausetime = level.time + time;
			self->monsterinfo.stand(self);
			self->monsterinfo.aiflags &= ~AI_PURSUE_PLAT_GOAL;
			return;
		}

		// if we're stuck, then follow new course
		if (self->monsterinfo.bump_delay > level.time)
		{
			M_ChangeYaw(self);
			M_MoveToGoal(self, dist);
			return;
		}

		// we're following a temporary goal
		if (self->movetarget && self->movetarget->inuse)
		{
			/*
			gi.WriteByte (svc_temp_entity);
			gi.WriteByte (TE_BFG_LASER);
			gi.WritePosition (self->s.origin);
			gi.WritePosition (self->movetarget->s.origin);
			gi.multicast (self->s.origin, MULTICAST_PHS);
			*/
			
			// pursue the movetarget
			M_MoveToGoal(self, dist);

			// occasionally, a monster will bump to a better course
			// than the one he was previously following
			if ((self->enemy->absmin[2] > self->absmin[2]+2*STEPHEIGHT) &&
				(self->movetarget->s.origin[2] < self->absmin[2]))
			{
				// we are higher than our current goal
				G_FreeEdict(self->movetarget);
				self->movetarget = NULL;
			}
			else if ((self->enemy->absmin[2] < self->absmin[2]-2*STEPHEIGHT) &&
				(self->movetarget->s.origin[2] > self->absmin[2]))
			{
				// we are lower than our current goal
				G_FreeEdict(self->movetarget);
				self->movetarget = NULL;
			}
			else if (entdist(self, self->movetarget) <= DRONE_MIN_GOAL_DIST)
			{
				// we are close 'nuff to our goal
				G_FreeEdict(self->movetarget);
				self->movetarget = NULL;
			}
		}
		else
		{	
			/*
			// we're following a goal
			if (self->goalentity && self->goalentity->inuse)
			{
				gi.WriteByte (svc_temp_entity);
				gi.WriteByte (TE_BFG_LASER);
				gi.WritePosition (self->s.origin);
				gi.WritePosition (self->goalentity->s.origin);
				gi.multicast (self->s.origin, MULTICAST_PHS);

				drone_pursue_goal(self, dist);
				return;
			}
			else
			*/
				// we don't already have a goal, make the enemy our goal
				self->goalentity = self->enemy;

			// pursue enemy if he's visible, otherwise pursue the last place
			// he was seen if we haven't already been there
			if (!enemy_vis)
			{
				/*
				if (level.time > self->monsterinfo.teleport_delay)
				{
					VectorSubtract(self->enemy->s.origin, self->s.origin, v);
					VectorNormalize(v);
					TeleportForward(self, v, 512);
					self->monsterinfo.teleport_delay = level.time + DRONE_TELEPORT_DELAY;
				}
				*/

				if (!(self->monsterinfo.aiflags & AI_LOST_SIGHT))
					self->monsterinfo.aiflags |= (AI_LOST_SIGHT|AI_PURSUIT_LAST_SEEN);
				
				if (self->monsterinfo.aiflags & AI_PURSUIT_LAST_SEEN)
				{
					VectorCopy(self->monsterinfo.last_sighting, start);
					VectorCopy(start, end);
					end[2] -= 64;
					tr = gi.trace(start, NULL, NULL, end, self, MASK_SOLID);
					// we are done chasing player last sighting if we're close to it
					// or it's hanging in mid-air
					if ((tr.fraction == 1) || (distance(self->monsterinfo.last_sighting, self->s.origin) < DRONE_MIN_GOAL_DIST))
					{
						self->monsterinfo.aiflags &= ~AI_PURSUIT_LAST_SEEN;
					}
					else
					{
						tempgoal = G_Spawn();
						VectorCopy(self->monsterinfo.last_sighting, tempgoal->s.origin);
						VectorCopy(self->monsterinfo.last_sighting, tempgoal->absmin);
						self->goalentity = tempgoal;
					}
				}
			}
			
			// go get him!
			drone_pursue_goal(self, dist);

			if (tempgoal)
				G_FreeEdict(tempgoal);
		}
	}
	else
	{
	//	gi.dprintf("stand was called because monster has no valid target!\n");
		self->monsterinfo.stand(self);
	}
}				

void drone_togglelight (edict_t *self)
{
	if (self->health > 0)
	{
		if (level.daytime && self->flashlight) 
		{
			// turn light off
			G_FreeEdict(self->flashlight);
			self->flashlight = NULL;
			return;;
		}
		else if (!level.daytime && !self->flashlight)
		{
			FL_make(self);
		}
	}
	else
	{
		// turn off the flashlight if we're dead!
		if (self->flashlight)
		{
			G_FreeEdict(self->flashlight);
			self->flashlight = NULL;
			return;
		}
	}
}

void drone_dodgeprojectiles (edict_t *self)
{
	// dodge incoming projectiles
	if (self->health > 0 && self->monsterinfo.dodge && (self->monsterinfo.aiflags & AI_DODGE)
		&& !(self->monsterinfo.aiflags & AI_STAND_GROUND)) // don't dodge if we are holding position
	{
		if (((self->monsterinfo.radius > 0) && ((level.time + 0.3) > self->monsterinfo.eta))
			|| (level.time + FRAMETIME) > self->monsterinfo.eta) 
		{
			self->monsterinfo.dodge(self, self->monsterinfo.attacker, self->monsterinfo.dir, self->monsterinfo.radius);
			self->monsterinfo.aiflags &= ~AI_DODGE;
		}
	}
}

/*
=============
drone_validposition

Returns false if the monster gets stuck in a solid object
and cannot be re-spawned
=============
*/
qboolean drone_validposition (edict_t *self)
{
	//trace_t		tr;
	qboolean	respawned = false;

	if (gi.pointcontents(self->s.origin) & CONTENTS_SOLID)
	{
		if (self->activator && self->activator->inuse && !self->activator->client)
			respawned = FindValidSpawnPoint(self, false);
		
		if (!respawned)
		{
			WriteServerMsg("A drone was removed from a solid object.", "Info", true, false);
			M_Remove(self, true, false);
			return false;
		}
	}
	return true;
}

qboolean drone_boss_stuff (edict_t *self)
{
	if (self->deadflag == DEAD_DEAD)
		return true; // we're dead

	//4.05 boss always regenerates
	if (self->monsterinfo.control_cost > 2)
	{
		if (self->enemy)
			M_Regenerate(self, 900, 10, true, true, false, &self->monsterinfo.regen_delay1);
		else // idle
			M_Regenerate(self, 600, 10, true, true, false, &self->monsterinfo.regen_delay1);
	}

	// bosses teleport away from danger when they are weakened
	if (self->activator && !self->activator->client && (self->monsterinfo.control_cost >= 3)
		&& (level.time > self->sentrydelay) && (self->health < (0.3*self->max_health))
		&& self->enemy && visible(self, self->enemy))
	{
		gi.bprintf(PRINT_HIGH, "Boss teleported away.\n");

		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_BOSSTPORT);
		gi.WritePosition (self->s.origin);
		gi.multicast (self->s.origin, MULTICAST_PVS);

		if(!FindValidSpawnPoint(self, false))
		{
			// boss failed to re-spawn
			/*
			if (self->activator)
				self->activator->num_sentries--;
			G_FreeEdict(self);
			*/
			M_Remove(self, false, false);
			gi.dprintf("WARNING: Boss failed to re-spawn!\n");
			return false;
		}
		self->enemy = NULL; // find a new target
		self->oldenemy = NULL;
		self->monsterinfo.stand(self);
		self->sentrydelay = level.time + GetRandom(10, 30);

		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_BOSSTPORT);
		gi.WritePosition (self->s.origin);
		gi.multicast (self->s.origin, MULTICAST_PVS);
	}
	return true;
}

void decoy_copy (edict_t *self);

void drone_think (edict_t *self)
{
	//gi.dprintf("drone_think()\n");

	if (!drone_validposition(self))
		return;
	if (!drone_boss_stuff(self))
		return;

//	decoy_copy(self);

	CTF_SummonableCheck(self);

	// monster will auto-remove if it is asked to
	if (self->removetime > 0)
	{
		qboolean converted=false;

		if (self->flags & FL_CONVERTED)
			converted = true;

		if (level.time > self->removetime)
		{
			// if we were converted, try to convert back to previous owner
			if (!RestorePreviousOwner(self))
			{
				M_Remove(self, false, true);
				return;
			}
		}
		// warn the converted monster's current owner
		else if (converted && self->activator && self->activator->inuse && self->activator->client 
			&& (level.time > self->removetime-5) && !(level.framenum%10))
				safe_cprintf(self->activator, PRINT_HIGH, "%s conversion will expire in %.0f seconds\n", 
					V_GetMonsterName(self->mtype), self->removetime-level.time);	
	}

	// if owner can't pay upkeep, monster dies
	if (!M_Upkeep(self, 10, self->monsterinfo.control_cost))
		return;

	V_HealthCache(self, (int)(0.2 * self->max_health), 1);
	V_ArmorCache(self, (int)(0.2 * self->monsterinfo.max_armor), 1);

	//Talent: Life Tap
	if (self->activator && self->activator->inuse && self->activator->client && !(level.framenum % 10))
	{
		if (getTalentLevel(self->activator, TALENT_LIFE_TAP) > 0)
		{
			int damage = 0.01 * self->max_health;
			if (damage < 1)
				damage = 1;
			T_Damage(self, world, world, vec3_origin, self->s.origin, vec3_origin, damage, 0, DAMAGE_NO_ABILITIES, 0);
		}
	}

	if (self->linkcount != self->monsterinfo.linkcount)
	{
		self->monsterinfo.linkcount = self->linkcount;
		M_CheckGround (self);
	}

	if (self->groundentity)
		VectorClear(self->velocity); // don't slide

	drone_togglelight(self);
	drone_dodgeprojectiles(self);
	
	// this must come before M_MoveFrame() because a monster's dead
	// function may set the nextthink to 0
	self->nextthink = level.time + FRAMETIME;

	M_MoveFrame (self);
	M_CatagorizePosition (self);
	M_WorldEffects (self);
	M_SetEffects (self);
}
