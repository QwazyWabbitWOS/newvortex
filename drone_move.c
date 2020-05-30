// m_move.c -- monster movement

#include "g_local.h"

#define	STEPSIZE	18

/*
=============
M_CheckBottom

Returns false if any part of the bottom of the entity is off an edge that
is not a staircase.

=============
*/
int c_yes, c_no;

qboolean M_CheckBottom (edict_t *ent)
{
	vec3_t	mins, maxs, start, stop;
	trace_t	trace;
	int		x, y;
	float	mid, bottom;
	
	VectorAdd (ent->s.origin, ent->mins, mins);
	VectorAdd (ent->s.origin, ent->maxs, maxs);

// if all of the points under the corners are solid world, don't bother
// with the tougher checks
// the corners must be within 16 of the midpoint
	start[2] = mins[2] - 1;
	for	(x=0 ; x<=1 ; x++)
		for	(y=0 ; y<=1 ; y++)
		{
			start[0] = x ? maxs[0] : mins[0];
			start[1] = y ? maxs[1] : mins[1];
			if (gi.pointcontents (start) != CONTENTS_SOLID)
				goto realcheck;
		}

	c_yes++;
	return true;		// we got out easy

realcheck:
	c_no++;
//
// check it for real...
//
	start[2] = mins[2];
	
// the midpoint must be within 16 of the bottom
	start[0] = stop[0] = (mins[0] + maxs[0])*0.5;
	start[1] = stop[1] = (mins[1] + maxs[1])*0.5;
	stop[2] = start[2] - 2*STEPSIZE;
	trace = gi.trace (start, vec3_origin, vec3_origin, stop, ent,MASK_PLAYERSOLID /*MASK_MONSTERSOLID*/);

	if (trace.fraction == 1.0)
		return false;
	mid = bottom = trace.endpos[2];
	
// the corners must be within 16 of the midpoint	
	for	(x=0 ; x<=1 ; x++)
		for	(y=0 ; y<=1 ; y++)
		{
			start[0] = stop[0] = x ? maxs[0] : mins[0];
			start[1] = stop[1] = y ? maxs[1] : mins[1];
			
			trace = gi.trace (start, vec3_origin, vec3_origin, stop, ent, MASK_PLAYERSOLID /*MASK_MONSTERSOLID*/);
			
			if (trace.fraction != 1.0 && trace.endpos[2] > bottom)
				bottom = trace.endpos[2];
			if (trace.fraction == 1.0 || mid - trace.endpos[2] > STEPSIZE)
				return false;
		}

	c_yes++;
	return true;
}

float distance (vec3_t p1, vec3_t p2);
qboolean CanJumpDown (edict_t *self, vec3_t neworg)
{
	float	dist_jump, dist_current;
	vec3_t	start;
	trace_t	tr;
	edict_t *goal;

	if (self->monsterinfo.jumpdn < 1)
		return false; // we can't jump down!

	if (self->movetarget && self->movetarget->inuse)
		goal = self->movetarget;
	else if (self->enemy && self->enemy->inuse)
		goal = self->enemy;
	else if (self->goalentity && self->goalentity->inuse)
		goal = self->goalentity;
	else
		return false;

	VectorCopy(neworg, start);
	start[2] -= 8192;
	tr = gi.trace(neworg, NULL, NULL, start, self, MASK_MONSTERSOLID);
	// don't jump in water if we aren't already in it
	if (!self->waterlevel)
	{
		//gi.dprintf("searching for hazards...\n");
		VectorCopy(tr.endpos, start);
		start[2] = tr.endpos[2] + self->mins[2] + 1;	

		if (gi.pointcontents(start) & (CONTENTS_LAVA|CONTENTS_SLIME))//MASK_WATER)
		{
			//gi.dprintf("hazard detected! jumpdown aborted.\n");
			return false;
		}
	}
	// don't jump farther than we're supposed to
	if (distance(neworg, tr.endpos) > self->monsterinfo.jumpdn)
		return false;
	
	// allow monster to jump if we've been stuck for awhile
	if (self->monsterinfo.stuck_frames > 50)
	{
		//gi.dprintf("monster got stuck and was allowed to fall\n");
		self->monsterinfo.stuck_frames = 0;
		return true;
	}

	// don't jump if the trace ends at our feet
	// FIXME: check in all directions since we don't always
	// walk directly off the edge
	if (tr.endpos[2] == self->absmin[2])
		return false;

	// don't jump unless it places us closer to our goal
	dist_jump = distance(tr.endpos, goal->s.origin);
	dist_current = distance(self->s.origin, goal->s.origin);
	if (dist_jump+STEPSIZE > dist_current)
	{
		/*
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_BFG_LASER);
		gi.WritePosition (neworg);
		gi.WritePosition (tr.endpos);
		gi.multicast (self->s.origin, MULTICAST_PHS);
		*/
		
	
		// we'll let this go if our goal is below us
	//	if ((dist_jump+16 > dist_current) && (goal->absmin[2] < self->absmin[2]))
	//		return true;
	//	gi.dprintf("*** FAILED *** dist jump %d dist current %d\n", (int)dist_jump, (int)dist_current);
		return false;
	}
	
	//gi.dprintf("dist jump %d dist current %d\n", (int)dist_jump, (int)dist_current);
/*
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_DEBUGTRAIL);
	gi.WritePosition (neworg);
	gi.WritePosition (tr.endpos);
	gi.multicast (neworg, MULTICAST_ALL);

	gi.dprintf("allowed monster to jump down\n");
*/
	return true;
}

qboolean CanJumpUp (edict_t *self, vec3_t neworg, vec3_t end)
{
	int		jumpdist;
	vec3_t	angles, start;
	trace_t	tr;

	if (self->monsterinfo.jumpup < 1)
		return false; // we can't jump up!

	VectorCopy(neworg, start);
	jumpdist = STEPSIZE;

	do
	{
		// add an additional step above the obstacle
		neworg[2] = start[2]+jumpdist;
		tr = gi.trace(neworg, self->mins, self->maxs, end, self, MASK_MONSTERSOLID);

		// we've hit a ceiling, so we can't go any higher
		if (tr.startsolid)
			return false;

		// try to jump a little higher
		jumpdist += STEPSIZE;

		// we've reached our limit
		if (jumpdist > self->monsterinfo.jumpup)
			return false;
	}
	// keep trying until we clear the obstacle OR we can't jump any higher
	while (tr.allsolid);

	// make sure we land on a flat plane
	vectoangles(tr.plane.normal, angles);
	if (angles[PITCH] > 360)
		angles[PITCH] -= 360;
	else if (angles[PITCH] < 0)
		angles[PITCH] += 360;
	if (angles[PITCH] != 270)
		return false;
	/*
	gi.dprintf("checking for hazards...\n");
	// calculate position one step beyond this obstacle
	VectorAdd(self->s.origin, move, pos1);
	VectorAdd(pos1, move, pos1);
	// find out what's below this position
	VectorCopy(pos1, pos2);
	pos2[2] -= 8192;
	tr = gi.trace(pos1, NULL, NULL, pos2, self, MASK_MONSTERSOLID);
	// don't allow monster to jump into lava or slime!
	if (self->waterlevel == 0)
	{
		//gi.dprintf("hazard detected!\n");

		pos2[0] = tr.endpos[0];
		pos2[1] = tr.endpos[1];
		pos2[2] = tr.endpos[2] + self->mins[2] + 1;	

		if (gi.pointcontents(end) & (CONTENTS_LAVA|CONTENTS_SLIME))
			return false;
	}

	//if (distance(tr.endpos, goal->s.origin)+16 > distance(self->s.origin, goal->s.origin))
	//	return false;
	*/
	VectorCopy(tr.endpos, self->s.origin);
	//gi.dprintf("allowed monster to jump up\n");
	return true;
}

// modified SV_movestep for use with player-controlled monsters
qboolean M_Move (edict_t *ent, vec3_t move, qboolean relink)
{
	vec3_t		oldorg, neworg, end;
	trace_t		trace;//, tr;
	float		stepsize=STEPSIZE;

// try the move	
	VectorCopy (ent->s.origin, oldorg);
	VectorAdd (ent->s.origin, move, neworg);

	neworg[2] += stepsize;
	VectorCopy (neworg, end);
	end[2] -= stepsize*2;

	trace = gi.trace (neworg, ent->mins, ent->maxs, end, ent, MASK_MONSTERSOLID);

	if (trace.allsolid)
	{
		// if we would have collided with a live entity, call its touch function
		// this prevents player-monsters from being invulnerable to obstacles
		if (G_EntIsAlive(trace.ent) && trace.ent->touch)
			trace.ent->touch(trace.ent, ent, &trace.plane, trace.surface);
		return false;
	}

	if (trace.startsolid)
	{
		neworg[2] -= stepsize;
		trace = gi.trace (neworg, ent->mins, ent->maxs, end, ent, MASK_MONSTERSOLID);
		if (trace.allsolid || trace.startsolid)
			return false;
	}

	if (trace.fraction == 1)
	{
	//	gi.dprintf("going to fall\n");
	// if monster had the ground pulled out, go ahead and fall
	//	VectorSubtract(trace.endpos, oldorg, forward);
	//	VectorMA(oldorg, 64, forward, end);
	
		if ( ent->flags & FL_PARTIALGROUND )
		{
			VectorAdd (ent->s.origin, move, ent->s.origin);
			if (relink)
			{
				gi.linkentity (ent);
				G_TouchTriggers (ent);
			}
			ent->groundentity = NULL;
			return true;
		}
	}

// check point traces down for dangling corners
	VectorCopy (trace.endpos, ent->s.origin);
	
	if (!M_CheckBottom (ent))
	{
		if (ent->flags & FL_PARTIALGROUND)
		{	// entity had floor mostly pulled out from underneath it
			// and is trying to correct
			if (relink)
			{
				gi.linkentity (ent);
				G_TouchTriggers (ent);
			}
			return true;
		}
	}
	
	if (ent->flags & FL_PARTIALGROUND)
		ent->flags &= ~FL_PARTIALGROUND;

	ent->groundentity = trace.ent;
	if (trace.ent)
		ent->groundentity_linkcount = trace.ent->linkcount;

// the move is ok
	if (relink)
	{
		gi.linkentity (ent);
		G_TouchTriggers (ent);
	}

	return true;
}

/*
=============
SV_movestep

Called by monster program code.
The move will be adjusted for slopes and stairs, but if the move isn't
possible, no move is done, false is returned, and
pr_global_struct->trace_normal is set to the normal of the blocking wall
=============
*/
//FIXME since we need to test end position contents here, can we avoid doing
//it again later in catagorize position?
qboolean SV_movestep (edict_t *ent, vec3_t move, qboolean relink)
{
	float		dz;
	vec3_t		oldorg, neworg, end;
	trace_t		trace;//, tr;
	int			i;
	float		stepsize;
	vec3_t		test;
	int			contents;
	int			jump=0;

// try the move	
	VectorCopy (ent->s.origin, oldorg);
	VectorAdd (ent->s.origin, move, neworg);

// flying monsters don't step up
	if ((ent->flags & (FL_SWIM|FL_FLY)) || (ent->waterlevel > 1))
	{
	//	gi.dprintf("trying to swim\n");
	// try one move with vertical motion, then one without
		for (i=0 ; i<2 ; i++)
		{
			VectorAdd (ent->s.origin, move, neworg);
			if (i == 0 && ent->enemy)
			{
				if (!ent->goalentity)
					ent->goalentity = ent->enemy;
				dz = ent->s.origin[2] - ent->goalentity->s.origin[2];
				if (ent->goalentity->client)
				{
					if (dz > 40)
						neworg[2] -= 8;
					if (!((ent->flags & FL_SWIM) && (ent->waterlevel < 2)))
						if (dz < 30)
							neworg[2] += 8;
				}
				else
				{
					if (dz > 8)
						neworg[2] -= 8;
					else if (dz > 0)
						neworg[2] -= dz;
					else if (dz < -8)
						neworg[2] += 8;
					else
						neworg[2] += dz;
				}
			}
			trace = gi.trace (ent->s.origin, ent->mins, ent->maxs, neworg, ent, MASK_MONSTERSOLID);
	
			// fly monsters don't enter water voluntarily
			if (ent->flags & FL_FLY)
			{
				if (!ent->waterlevel)
				{
					test[0] = trace.endpos[0];
					test[1] = trace.endpos[1];
					test[2] = trace.endpos[2] + ent->mins[2] + 1;
					contents = gi.pointcontents(test);
					if (contents & MASK_WATER)
						return false;
				}
			}

			// swim monsters don't exit water voluntarily
			if (ent->flags & FL_SWIM)
			{
				if (ent->waterlevel < 2)
				{
					test[0] = trace.endpos[0];
					test[1] = trace.endpos[1];
					test[2] = trace.endpos[2] + ent->mins[2] + 1;
					contents = gi.pointcontents(test);
					if (!(contents & MASK_WATER))
						return false;
				}
			}

			if (trace.fraction == 1)
			{
				VectorCopy (trace.endpos, ent->s.origin);
				if (relink)
				{
					gi.linkentity (ent);
					G_TouchTriggers (ent);
				}
				return true;
			}
			//gi.dprintf("swim move failed\n");
			
			if (!ent->enemy)
				break;
		}
		
		return false;
	}

// push down from a step height above the wished position
	if (!(ent->monsterinfo.aiflags & AI_NOSTEP))
		stepsize = STEPSIZE;
	else
		stepsize = 1;

	neworg[2] += stepsize;
	VectorCopy (neworg, end);
	end[2] -= stepsize*2;

	// this trace checks from a position one step above the entity (at top of bbox)
	// to one step below the entity (bottom of bbox)
	trace = gi.trace (neworg, ent->mins, ent->maxs, end, ent, MASK_MONSTERSOLID);

	// there is an obstruction bigger than a step
	if (trace.allsolid)
//GHz START
	{
		// try to jump over it
		if (!CanJumpUp(ent, neworg, end))
			return false;
		else
			jump = 1;
	}
//GHz END

	// not enough room at this height--head of bbox intersects something solid
	// so push down and just try to walk forward at floor height
	else if (trace.startsolid)
	{
		neworg[2] -= stepsize;
		trace = gi.trace (neworg, ent->mins, ent->maxs, end, ent, MASK_MONSTERSOLID);
		if (trace.allsolid || trace.startsolid)
			return false;
	}

	// don't go in to water
	if (ent->waterlevel == 0)
	{
		test[0] = trace.endpos[0];
		test[1] = trace.endpos[1];
		test[2] = trace.endpos[2] + ent->mins[2] + 1;	
		contents = gi.pointcontents(test);

		if (contents & (CONTENTS_LAVA|CONTENTS_SLIME))
			return false;
	}

//GHz START
//	if (CanJumpDown(ent, trace.endpos))
//		jump = -1;
//GHz END

//	VectorSubtract(trace.endpos, oldorg, forward);
//	VectorNormalize(forward);
//	VectorMA(oldorg, 32, forward, end);

//	VectorAdd(trace.endpos, move, end);
//	VectorAdd(end, move, end);

	if ((trace.fraction == 1) && (jump != 1))
	{
	//	gi.dprintf("going to fall\n");
	// if monster had the ground pulled out, go ahead and fall
	//	VectorSubtract(trace.endpos, oldorg, forward);
	//	VectorMA(oldorg, 64, forward, end);
		if (!CanJumpDown(ent, trace.endpos))
		{
			if ( ent->flags & FL_PARTIALGROUND )
			{
				VectorAdd (ent->s.origin, move, ent->s.origin);
				if (relink)
				{
					gi.linkentity (ent);
					G_TouchTriggers (ent);
				}
				ent->groundentity = NULL;
				return true;
			}
			return false;		// walked off an edge
		}
		else
			jump = -1;
	}

// check point traces down for dangling corners
	//GHz START
	/*
	// fix for monsters walking thru walls
	tr = gi.trace(trace.endpos, ent->mins, ent->maxs, trace.endpos, ent, MASK_SOLID);
	if (tr.contents & MASK_SOLID)
		return false;
	*/
	//GHz END
	
	if (jump != 1)
		VectorCopy (trace.endpos, ent->s.origin);
	
	if (!M_CheckBottom (ent))
	{
		if (ent->flags & FL_PARTIALGROUND)
		{	// entity had floor mostly pulled out from underneath it
			// and is trying to correct
			if (relink)
			{
				gi.linkentity (ent);
				G_TouchTriggers (ent);
			}
			return true;
		}
		if (CanJumpDown(ent, trace.endpos))
			jump = -1;
		if (!jump)
		{
			VectorCopy (oldorg, ent->s.origin);
			return false;
		}
	}
	else if (jump == -1)
		jump = 0;
/*
	if (jump)
	{
		VectorCopy(oldorg, ent->s.origin);
		CanJumpDown(ent, trace.endpos, true);
		VectorCopy(trace.endpos, ent->s.origin);
	}
*/
	
	if ( ent->flags & FL_PARTIALGROUND )
	{
		ent->flags &= ~FL_PARTIALGROUND;
	}

	ent->groundentity = trace.ent;
	if (trace.ent)
		ent->groundentity_linkcount = trace.ent->linkcount;

	if (jump == -1)
	{
		/*
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_DEBUGTRAIL);
		gi.WritePosition (oldorg);
		gi.WritePosition (end);
		gi.multicast (end, MULTICAST_ALL);
		*/

		//VectorScale(move, 10, ent->velocity);
		//ent->velocity[2] = 200;
	}
	else if (jump == 1)
	{
		ent->velocity[2] = 200;
		//gi.dprintf("jumped at %d\n",level.framenum);
	}

// the move is ok
	if (relink)
	{
		gi.linkentity (ent);
		G_TouchTriggers (ent);
	}
	return true;
}

/*
======================
SV_StepDirection

Turns to the movement direction, and walks the current distance if
facing it.

======================
*/
qboolean SV_StepDirection (edict_t *ent, float yaw, float dist, qboolean try_smallstep)
{
	vec3_t		move, oldorigin;
	//float		delta;
	float		old_dist;
	
	//gi.dprintf("SV_StepDirection\n");
	old_dist = dist;
	ent->ideal_yaw = yaw;
	M_ChangeYaw (ent);
	yaw = yaw*M_PI*2 / 360;
	VectorCopy (ent->s.origin, oldorigin);
	
	// loop until we can move successfully
	while (dist > 0)
	{
		move[0] = cos(yaw)*dist;
		move[1] = sin(yaw)*dist;
		move[2] = 0;

		if (SV_movestep (ent, move, false))
		{
			/*
			delta = ent->s.angles[YAW] - ent->ideal_yaw;
			if (delta > 45 && delta < 315)
			{		// not turned far enough, so don't take the step
				VectorCopy (oldorigin, ent->s.origin);
				gi.dprintf("not turned far enough!\n");
			}
			*/
			gi.linkentity (ent);
			G_TouchTriggers (ent);
			/*
			if (old_dist != dist)
				gi.dprintf("took a smaller step of %d instead of %d\n", (int)dist, (int)old_dist);
			else
				gi.dprintf("moved full distance\n");
			*/
			return true;
		}

		if (!try_smallstep)
			break;

		dist -= 5;
	}

	gi.linkentity (ent);
	G_TouchTriggers (ent);
//	gi.dprintf("Can't walk forward!\n");
	return false;
}

/*
======================
SV_FixCheckBottom

======================
*/
void SV_FixCheckBottom (edict_t *ent)
{
	ent->flags |= FL_PARTIALGROUND;
}


/*
================
SV_NewChaseDir

================
*/
#define	DI_NODIR	-1
void SV_NewChaseDir (edict_t *actor, edict_t *enemy, float dist)
{
	float	deltax,deltay;
	float	d[3];
	float	tdir, olddir, turnaround;

	//FIXME: how did we get here with no enemy
	if (!enemy)
		return;

	olddir = anglemod( (int)(actor->ideal_yaw/45)*45 );
	turnaround = anglemod(olddir - 180);

	deltax = enemy->s.origin[0] - actor->s.origin[0];
	deltay = enemy->s.origin[1] - actor->s.origin[1];
	if (deltax>10)
		d[1]= 0;
	else if (deltax<-10)
		d[1]= 180;
	else
		d[1]= DI_NODIR;
	if (deltay<-10)
		d[2]= 270;
	else if (deltay>10)
		d[2]= 90;
	else
		d[2]= DI_NODIR;

// try direct route
	if (d[1] != DI_NODIR && d[2] != DI_NODIR)
	{
		if (d[1] == 0)
			tdir = d[2] == 90 ? 45 : 315;
		else
			tdir = d[2] == 90 ? 135 : 215;
			
		if (tdir != turnaround && SV_StepDirection(actor, tdir, dist, true))
			return;
	}

// try other directions
	if ( ((randomMT()&3) & 1) ||  abs(deltay)>abs(deltax))
	{
		tdir=d[1];
		d[1]=d[2];
		d[2]=tdir;
	}

	if (d[1]!=DI_NODIR && d[1]!=turnaround 
	&& SV_StepDirection(actor, d[1], dist, true))
			return;

	if (d[2]!=DI_NODIR && d[2]!=turnaround
	&& SV_StepDirection(actor, d[2], dist, true))
			return;

/* there is no direct path to the player, so pick another direction */

	if (olddir!=DI_NODIR && SV_StepDirection(actor, olddir, dist, true))
			return;

	if (randomMT()&1) 	/*randomly determine direction of search*/
	{
		for (tdir=0 ; tdir<=315 ; tdir += 45)
			if (tdir!=turnaround && SV_StepDirection(actor, tdir, dist, true) )
					return;
	}
	else
	{
		for (tdir=315 ; tdir >=0 ; tdir -= 45)
			if (tdir!=turnaround && SV_StepDirection(actor, tdir, dist, true) )
					return;
	}

	if (turnaround != DI_NODIR && SV_StepDirection(actor, turnaround, dist, true) )
			return;

	actor->ideal_yaw = olddir;		// can't move

// if a bridge was pulled out from underneath a monster, it may not have
// a valid standing position at all

	if (!M_CheckBottom (actor))
		SV_FixCheckBottom (actor);
}

void SV_NewChaseDir1 (edict_t *self, edict_t *goal, float dist)
{
	int		i, bestyaw, minyaw, maxyaw;	
	vec3_t	v;

	if (!goal)
		return;

	VectorSubtract(goal->s.origin, self->s.origin, v);
	bestyaw = vectoyaw(v);

	minyaw = bestyaw - 90;
	maxyaw = bestyaw + 90;

	if (minyaw < 0)
		minyaw += 360;
	if (maxyaw > 360)
		maxyaw -= 360;

	for (i=minyaw; i<maxyaw; i+=30) {
		if (SV_StepDirection(self, i, dist, false))
			return;
	}

	//gi.dprintf("couldnt find a better direction!\n");
	SV_NewChaseDir(self, goal, dist);
}

/*
===============
M_ChangeYaw

===============
*/
void M_ChangeYaw (edict_t *ent)
{
	float	ideal;
	float	current;
	float	move;
	float	speed;
	
	current = anglemod(ent->s.angles[YAW]);
	ideal = ent->ideal_yaw;

	if (current == ideal)
		return;

	move = ideal - current;
	speed = ent->yaw_speed;
	if (ideal > current)
	{
		if (move >= 180)
			move = move - 360;
	}
	else
	{
		if (move <= -180)
			move = move + 360;
	}
	if (move > 0)
	{
		if (move > speed)
			move = speed;
	}
	else
	{
		if (move < -speed)
			move = -speed;
	}
	
	ent->s.angles[YAW] = anglemod (current + move);

}

/*
======================
SV_CloseEnough

======================
*/
qboolean SV_CloseEnough (edict_t *ent, edict_t *goal, float dist)
{
	int		i;
	
	for (i=0 ; i<3 ; i++)
	{
		if (goal->absmin[i] > ent->absmax[i] + dist)
			return false;
		if (goal->absmax[i] < ent->absmin[i] - dist)
			return false;
	}
	return true;
}

/*
======================
M_MoveToGoal
======================
*/
void M_MoveToGoal (edict_t *ent, float dist)
{
	edict_t		*goal;

	goal = ent->goalentity;

	if (ent->holdtime > level.time) // stay in-place for medic healing
		return;

	if (!ent->groundentity && !(ent->flags & (FL_FLY|FL_SWIM)) && !ent->waterlevel)
	{
		//gi.dprintf("not touching ground\n");
		return;
	}

// if the next step hits the enemy, return immediately
	if (ent->enemy && (ent->enemy->solid == SOLID_BBOX) 
		&& SV_CloseEnough (ent, ent->enemy, dist) )
//GHz START
	{
		vec3_t v;

		// we need to keep turning to avoid getting stuck doing nothing
		if (ent->goalentity && ent->goalentity->inuse) // 3.89 make sure the monster still has a goal!
		{
			VectorSubtract(ent->goalentity->s.origin, ent->s.origin, v);
			VectorNormalize(v);
			ent->ideal_yaw = vectoyaw(v);
			M_ChangeYaw(ent);
		}
		return;
	}
//GHz END

	// dont move so fast in the water
	if (!(ent->flags & (FL_FLY|FL_SWIM)) && (ent->waterlevel > 1))
		dist *= 0.5;

// bump around...
	// if we can't take a step, try moving in another direction
	if (!SV_StepDirection (ent, ent->ideal_yaw, dist, true))
	{
		// if the monster hasn't moved much, then increment
		// the number of frames it has been stuck
		if (distance(ent->s.origin, ent->monsterinfo.stuck_org) < 64)
			ent->monsterinfo.stuck_frames++;
		else
			ent->monsterinfo.stuck_frames = 0;

		// record current position for comparison
		VectorCopy(ent->s.origin, ent->monsterinfo.stuck_org);

		// attempt a course-correction
		if (ent->inuse && (level.time > ent->monsterinfo.bump_delay))
		{
			//gi.dprintf("monster stuck, attempting course-correction\n");
			SV_NewChaseDir (ent, goal, dist);
			ent->monsterinfo.bump_delay = level.time + FRAMETIME*GetRandom(2, 5);
			return;
		}
	}
}


/*
===============
M_walkmove
===============
*/
qboolean M_walkmove (edict_t *ent, float yaw, float dist)
{
	float	original_yaw=yaw;//GHz
	vec3_t	move;
	
	if (!ent->groundentity && !(ent->flags & (FL_FLY|FL_SWIM)))
		return false;

	yaw = yaw*M_PI*2 / 360;
	
	move[0] = cos(yaw)*dist;
	move[1] = sin(yaw)*dist;
	move[2] = 0;
	
	if (IsABoss(ent) || ent->mtype == P_TANK)
	{
		//return M_Move(ent, move, true);
		if (!M_Move(ent, move, true))
		{
			float	angle1, angle2, delta1, delta2, cl_yaw, ideal_yaw, mult;
			vec3_t	start, end, angles, right;
			trace_t tr;

			// get monster angles
			AngleVectors(ent->s.angles, NULL, right, NULL);
			//vectoangles(forward, forward);
			vectoangles(right, right);

			// get client yaw (FIXME: use mons yaw instead?)
			cl_yaw = ent->s.angles[YAW];
			AngleCheck(&cl_yaw);

			// trace from monster to wall
			VectorCopy(ent->s.origin, start);
			VectorMA(start, 64, move, end);
			tr = gi.trace(start, ent->mins, ent->maxs, end, ent, MASK_SHOT);

			// get monster angles in relation to wall
			VectorCopy(tr.plane.normal, angles);
			vectoangles(angles, angles);

			// monster is moving sideways, so use sideways vector instead
			if ((int)original_yaw==(int)right[YAW])
			{
				cl_yaw = right[YAW];
				AngleCheck(&cl_yaw);
			}

			// delta between monster yaw and wall yaw should be no more than 90 degrees
			// else, turn wall angles around 180 degrees
			if (fabs(cl_yaw-angles[YAW]) > 90)
				angles[YAW]+=180;
			ValidateAngles(angles);

			// possible escape angle 1
			angle1 = angles[YAW]+90;
			AngleCheck(&angle1);
			delta1 = fabs(angle1-cl_yaw);
			// possible escape angle 2
			angle2 = angles[YAW]-90;
			AngleCheck(&angle2);
			delta2 = fabs(angle2-cl_yaw);

			// take the shorter route
			if (delta1 > delta2)
			{
				ideal_yaw = angle2;
				mult = 1.0-delta2/90.0;
			}
			else
			{
				ideal_yaw = angle1;
				mult = 1.0-delta1/90.0;
			}
			
			// go full speed if we are turned at least half way towards ideal yaw
			if (mult >= 0.5)
				mult = 1.0;

			// modify speed
			dist*=mult;

			// recalculate with new heading
			yaw = ideal_yaw*M_PI*2 / 360;
			move[0] = cos(yaw)*dist;
			move[1] = sin(yaw)*dist;
			move[2] = 0;
				
			//gi.dprintf("can't walk wall@%.1f you@%.1f ideal@%.1f\n", angles[YAW], cl_yaw, ideal_yaw);

			return M_Move(ent, move, true);
		}
		return true;
	}
	else if (level.time > ent->holdtime) // stay in-place for medic healing
		return SV_movestep(ent, move, true);
	else
		return false;
}

