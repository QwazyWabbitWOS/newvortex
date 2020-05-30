#include "g_local.h"

#define LASER_SPAWN_DELAY		1.0	// time before emitter creates laser beam
#define LASER_INITIAL_DAMAGE	100	// beam damage per frame
#define LASER_ADDON_DAMAGE		40
#define LASER_TIMEOUT_DELAY		120

// cumulative maximum damage a laser can deal
#define LASER_INITIAL_HEALTH	0
#define LASER_ADDON_HEALTH		100

void RemoveLasers (edict_t *ent)
{
	edict_t *e=NULL;

	while((e = G_Find(e, FOFS(classname), "emitter")) != NULL)
	{
		if (e && (e->owner == ent))
		{
			// remove the laser beam
			if (e->creator)
			{
				e->creator->think = G_FreeEdict;
				e->creator->nextthink = level.time + FRAMETIME;
				//G_FreeEdict(e->creator);
			}
			// remove the emitter
			
			e->think = BecomeExplosion1;
			e->nextthink = level.time + FRAMETIME;
			//BecomeExplosion1(e);
		}
	}

	// reset laser counter
	ent->num_lasers = 0;
}

qboolean NearbyLasers (edict_t *ent, vec3_t org)
{
	edict_t *e=NULL;

	while((e = findradius(e, org, 8)) != NULL)
	{
		// is this a laser than we own?
		if (e && e->inuse && G_EntExists(e->owner) 
			&& (e->owner == ent) && !strcmp(e->classname, "emitter"))
			return true;
	}

	return false;
}

qboolean NearbyProxy (edict_t *ent, vec3_t org)
{
	edict_t *e=NULL;

	while((e = findradius(e, org, 8)) != NULL)
	{
		// is this a proxy than we own?
		if (e && e->inuse && G_EntExists(e->owner) 
			&& (e->owner == ent) && !strcmp(e->classname, "proxygrenade"))
			return true;
	}

	return false;
}

void laser_remove (edict_t *self)
{
	// remove emitter/grenade
	self->think = BecomeExplosion1;
	self->nextthink = level.time+FRAMETIME;

	// remove laser beam
	if (self->creator && self->creator->inuse)
	{
		self->creator->think = G_FreeEdict;
		self->creator->nextthink = level.time+FRAMETIME;
	}

	// decrement laser counter
	if (self->owner && self->owner->inuse)
	{
		self->owner->num_lasers--;
		gi.cprintf(self->owner, PRINT_HIGH, "Laser destroyed. %d/%d remaining.\n", 
			self->owner->num_lasers, MAX_LASERS);
	}
}

void laser_beam_think (edict_t *self)
{
	int		size;
	int		damage = self->dmg;
	vec3_t	forward;
	trace_t tr;

	// can't have a laser beam without an emitter!
	if (!self->creator)
	{
		G_FreeEdict(self);
		return;
	}

	if (self->monsterinfo.level >= 10)
		size = 4;
	else
		size = 2;

	// recharge
	if (self->health < self->max_health)
	{
		if (level.framenum > self->monsterinfo.regen_delay1)
		{
			self->health += 0.2*self->max_health;
			if (self->health > self->max_health)
				self->health = self->max_health;

			self->monsterinfo.regen_delay1 = level.framenum + 10;
		}

		if (size > 2 && self->health < self->dmg)
			size *= 0.5;
	}

	// set beam diameter
	self->s.frame = size;

	// flash yellow if the laser is about to expire
	if ((level.time+10 >= self->creator->nextthink) && !(level.framenum%5) || (self->health < self->dmg))
	{
		self->s.skinnum = 0xdcdddedf; // yellow
	}
	else
	{
		if (self->owner->teamnum == 1)
			self->s.skinnum = 0xf2f2f0f0; // red
		else if (self->owner->teamnum == 2)
			self->s.skinnum = 0xf3f3f1f1; // blue
		else
			self->s.skinnum = 0xf2f2f0f0; // red
	}

	// trace from beam emitter out as far as we can go
	AngleVectors(self->s.angles, forward, NULL, NULL);
	VectorMA(self->pos1, 8192, forward, self->pos2);
	tr = gi.trace (self->pos1, NULL, NULL, self->pos2, self->creator, MASK_SHOT);
	VectorCopy(tr.endpos, self->s.origin);
	VectorCopy(self->pos1, self->s.old_origin);

	// what is in lasers path?
	if (G_EntExists(tr.ent))
	{
		// remove lasers near spawn positions
		if (tr.ent->client && (tr.ent->client->respawn_time-1.5 > level.time))
		{
			gi.cprintf(self->owner, PRINT_HIGH, "Laser touched respawning player, so it was removed. (%d/%d remain)\n", self->owner->num_lasers, MAX_LASERS);
			laser_remove(self->creator);
			return;
		}

		// don't deal more damage than our health
		if (damage > self->health)
			damage = self->health;

		// deal damage to anything in the beam's path
		if (damage && T_Damage(tr.ent, self, self->owner, forward, tr.endpos, 
			vec3_origin, damage, 0, DAMAGE_ENERGY, MOD_LASER_DEFENSE))
		{
			// reduce maximum damage counter
			self->health -= damage;

			// if the counter reaches 0, then self-destruct
			if (self->health < 1)
			{
				self->health = 0;
				//gi.cprintf(self->owner, PRINT_HIGH, "Laser burned out. (%d/%d remain)\n", self->owner->num_lasers-1, MAX_LASERS);
				//laser_remove(self->creator);
				//return;
			}
		}
	}

	self->nextthink = level.time + FRAMETIME;
}

void SpawnLaser (edict_t *ent, int cost, float skill_mult, float delay_mult)
{
	int		talentLevel = getTalentLevel(ent, TALENT_RAPID_ASSEMBLY);//Talent: Rapid Assembly
	float	delay;
	vec3_t	forward, right, start, end, offset;
	trace_t	tr;
	edict_t *grenade, *laser;

	// get starting position and forward vector
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorSet(offset, 0, 8,  ent->viewheight-8);
	P_ProjectSource(ent->client, ent->s.origin, offset, forward, right, start);
	// get end position
	VectorMA(start, 64, forward, end);

	tr = gi.trace (start, NULL, NULL, end, ent, MASK_SOLID);

	// can't build a laser on sky
	if (tr.surface && (tr.surface->flags & SURF_SKY))
		return;

	if (tr.fraction == 1)
	{
		gi.cprintf(ent, PRINT_HIGH, "Too far from wall.\n");
		return;
	}

	if (NearbyLasers(ent, tr.endpos))
	{
		gi.cprintf(ent, PRINT_HIGH, "Too close to another laser.\n");
		return;
	}

	if (NearbyProxy(ent, tr.endpos))
	{
		gi.cprintf(ent, PRINT_HIGH, "Too close to a proxy grenade.\n");
		return;
	}

	laser = G_Spawn();
	grenade = G_Spawn();

	// create the laser beam
	laser->monsterinfo.level = ent->myskills.abilities[BUILD_LASER].current_level * skill_mult;
	laser->dmg = LASER_INITIAL_DAMAGE+LASER_ADDON_DAMAGE*laser->monsterinfo.level;
	laser->health = LASER_INITIAL_HEALTH+LASER_ADDON_HEALTH*laser->monsterinfo.level;

	// nerf lasers in CTF and invasion
	if (ctf->value || invasion->value)
		laser->health *= 0.5;

	laser->max_health = laser->health;

	// set beam diameter
	if (laser->monsterinfo.level >= 10)
		laser->s.frame = 4;
	else
		laser->s.frame = 2;

	laser->movetype	= MOVETYPE_NONE;
	laser->solid = SOLID_NOT;
	laser->s.renderfx = RF_BEAM|RF_TRANSLUCENT;
	laser->s.modelindex = 1; // must be non-zero
	laser->s.sound = gi.soundindex ("world/laser.wav");
	laser->classname = "laser";
    laser->owner = ent; // link to player
	laser->creator = grenade; // link to grenade
	laser->s.skinnum = 0xf2f2f0f0; // red beam color
    laser->think = laser_beam_think;
	laser->nextthink = level.time + LASER_SPAWN_DELAY * delay_mult;
	VectorCopy(ent->s.origin, laser->s.origin);
	VectorCopy(tr.endpos, laser->s.old_origin);
	VectorCopy(tr.endpos, laser->pos1); // beam origin
	vectoangles(tr.plane.normal, laser->s.angles);
	gi.linkentity(laser);

	delay = LASER_TIMEOUT_DELAY+GetRandom(0, (int)(0.5*LASER_TIMEOUT_DELAY));

	// laser times out faster in CTF because it's too strong
	if (ctf->value || invasion->value)
		delay *= 0.5;

	// create the laser emmitter (grenade)
    VectorCopy(tr.endpos, grenade->s.origin);
    vectoangles(tr.plane.normal, grenade->s.angles);
	grenade->movetype = MOVETYPE_NONE;
	grenade->clipmask = MASK_SHOT;
	grenade->solid = SOLID_BBOX;
	VectorSet(grenade->mins, -3, -3, 0);
	VectorSet(grenade->maxs, 3, 3, 6);
	grenade->takedamage = DAMAGE_NO;
	grenade->s.modelindex = gi.modelindex("models/objects/grenade2/tris.md2");
    grenade->owner = ent; // link to player
    grenade->creator = laser; // link to laser
	grenade->classname = "emitter";
	grenade->nextthink = level.time+delay; // time before self-destruct
	grenade->think = laser_remove;
	gi.linkentity(grenade);

	// cost is doubled if you are a flyer or cacodemon below skill level 5
	if ((ent->mtype == MORPH_FLYER && ent->myskills.abilities[FLYER].current_level < 5) 
		|| (ent->mtype == MORPH_CACODEMON && ent->myskills.abilities[CACODEMON].current_level < 5))
		cost *= 2;

	ent->num_lasers++;
	gi.cprintf(ent, PRINT_HIGH, "Laser built. You have %d/%d lasers.\n", ent->num_lasers, MAX_LASERS);
	ent->client->pers.inventory[power_cube_index] -= cost;
	ent->client->ability_delay = level.time + 0.5 * delay_mult;
	ent->holdtime = level.time + 0.5 * delay_mult;

	//decino: firewall talent
	talentLevel = getTalentLevel(ent, TALENT_PRECISION_TUNING);
	if (talentLevel == 1)
	grenade->firewall_weak = true;
	if (talentLevel == 2)
	grenade->firewall_medium = true;
	if (talentLevel == 3)
	grenade->firewall_strong = true;

	ent->lastsound = level.framenum;
}

void Cmd_BuildLaser (edict_t *ent)
{
	int talentLevel, cost=LASER_COST;
	float skill_mult=1.0, cost_mult=1.0, delay_mult=1.0;//Talent: Rapid Assembly & Precision Tuning

	if(ent->myskills.abilities[BUILD_LASER].disable)
		return;

	if (Q_strcasecmp (gi.args(), "remove") == 0)
	{
		RemoveLasers(ent);
		gi.cprintf(ent, PRINT_HIGH, "All lasers removed.\n");
		return;
	}

	// cost is doubled if you are a flyer or cacodemon below skill level 5
	if ((ent->mtype == MORPH_FLYER && ent->myskills.abilities[FLYER].current_level < 5) 
		|| (ent->mtype == MORPH_CACODEMON && ent->myskills.abilities[CACODEMON].current_level < 5))
		cost *= 2;

	//Talent: Rapid Assembly
	talentLevel = getTalentLevel(ent, TALENT_RAPID_ASSEMBLY);
	if (talentLevel > 0)
		delay_mult -= 0.167 * talentLevel;

	cost *= cost_mult;

	if (!G_CanUseAbilities(ent, ent->myskills.abilities[BUILD_LASER].current_level, cost))
		return;

	if (ent->num_lasers >= MAX_LASERS)
	{
		gi.cprintf(ent, PRINT_HIGH, "Can't build any more lasers.\n");
		return;
	}

	if (ctf->value && (CTF_DistanceFromBase(ent, NULL, CTF_GetEnemyTeam(ent->teamnum)) < CTF_BASE_DEFEND_RANGE))
	{
		gi.cprintf(ent, PRINT_HIGH, "Can't build in enemy base!\n");
		return;
	}

	SpawnLaser(ent, cost, skill_mult, delay_mult);
}
