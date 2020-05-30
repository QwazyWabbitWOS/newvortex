#include "g_local.h"

#define STATION_TARGET_RADIUS	256
#define STATION_COST			50
#define STATION_DELAY			2
#define STATION_BUILD_TIME		2

qboolean ValidStationTarget (edict_t *self, edict_t *targ) {
	return (G_EntExists(targ) && (targ->health>0) 
		&& targ->client && visible(self, targ));
}

qboolean station_findtarget (edict_t *self)
{
	edict_t *e=NULL;

	while ((e = findclosestradius(e, self->s.origin, STATION_TARGET_RADIUS)) != NULL)
	{
		if (!ValidStationTarget(self, e))
			continue;
		if (!OnSameTeam(self, e))
		{
			gi.centerprintf(self->creator, "%s is using\nyour supply station!\n", 
				e->client->pers.netname);
			continue;
		}
		self->enemy = e;
		return true;
	}
	self->enemy = NULL;
	return false;
}

qboolean NeedHealth (edict_t *player) {
	return (player->health < player->max_health);
}

qboolean NeedArmor (edict_t *player) {
	return (player->client->pers.inventory[body_armor_index] < MAX_ARMOR(player));
}

float HealthLevel (edict_t *ent) {
	return ((float)ent->health/ent->max_health);
}

float ArmorLevel (edict_t *ent) {
	return ((float)ent->client->pers.inventory[body_armor_index]/MAX_ARMOR(ent));
}

edict_t *DropRandomAmmo (edict_t *self)
{
	gitem_t *item;

	switch (GetRandom(1, 6))
	{
	case 1: item = FindItemByClassname("ammo_shells"); break;
	case 2: item = FindItemByClassname("ammo_bullets"); break;
	case 3: item = FindItemByClassname("ammo_grenades"); break;
	case 4: item = FindItemByClassname("ammo_rockets"); break;
	case 5: item = FindItemByClassname("ammo_cells"); break;
	case 6: item = FindItemByClassname("ammo_slugs"); break;
	}
	return Drop_Item(self, item);
}

#define HEALTHBOX_SMALL	1
#define HEALTHBOX_LARGE	2

void healthbox_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	float	temp = 1.0;

	if (G_EntExists(other) && other->client && (other->health > 0)
		&& (NeedHealth(other) || (self->style == HEALTHBOX_SMALL)))
	{
		// special rules disable flag carrier abilities
		if(!(ctf->value && ctf_enable_balanced_fc->value && HasFlag(other)))
		{
			if(!other->myskills.abilities[HA_PICKUP].disable)
				temp += 0.3*other->myskills.abilities[HA_PICKUP].current_level;

			//Talent: Basic HA Pickup
			//if(getTalentSlot(other, TALENT_BASIC_HA) != -1)
			//	temp += 0.2 * getTalentLevel(other, TALENT_BASIC_HA);
		}

		other->health += self->count*temp;
		// check for maximum health cap
		if ((other->health > other->max_health) && (self->style != HEALTHBOX_SMALL))
			other->health = other->max_health;
		// play appropriate sound
		if (self->style == HEALTHBOX_LARGE)
			gi.sound(self, CHAN_ITEM, gi.soundindex("items/l_health.wav"), 1, ATTN_NORM, 0);
		else if (self->style == HEALTHBOX_SMALL)
			gi.sound(self, CHAN_ITEM, gi.soundindex("items/s_health.wav"), 1, ATTN_NORM, 0);
		// remove the ent
		G_FreeEdict(self);
	}
}

void healthbox_think (edict_t *self)
{
	if (level.time > self->delay)
	{
		G_FreeEdict(self);
		return;
	}
	self->nextthink = level.time + FRAMETIME;
}

edict_t *SpawnHealthBox (edict_t *ent, int type)
{
	vec3_t forward;
	edict_t *health;

	health = G_Spawn();
	if (type == HEALTHBOX_LARGE)
	{
		health->model = "models/items/healing/large/tris.md2";
		health->s.modelindex = gi.modelindex("models/items/healing/large/tris.md2");
		health->count = 25;
		health->style = HEALTHBOX_LARGE;
	}
	else if (type == HEALTHBOX_SMALL)
	{
		health->model = "models/items/healing/stimpack/tris.md2";
		health->s.modelindex = gi.modelindex("models/items/healing/stimpack/tris.md2");
		health->count = 2;
		health->style = HEALTHBOX_SMALL;
	}
	else
	{
		gi.dprintf("WARNING: SpawnHealthBox() called without a valid type.\n");
		return NULL;
	}
	health->touch = healthbox_touch;
	health->think = healthbox_think;
	health->nextthink = level.time + FRAMETIME;
	health->delay = level.time + 30;
	health->s.renderfx = (RF_GLOW|RF_IR_VISIBLE);
	health->solid = SOLID_TRIGGER;
	health->movetype = MOVETYPE_TOSS;
	VectorSet(health->mins, -15, -15, -15);
	VectorSet(health->maxs, 15, 15, 15);
	
	// toss the item
	VectorCopy(ent->s.angles, forward);
	forward[YAW] = GetRandom(0, 360);
	health->s.angles[YAW] = GetRandom(0, 360);
	AngleVectors(forward, forward, NULL, NULL);
	VectorCopy (ent->s.origin, health->s.origin);
	gi.linkentity(health);
	VectorScale (forward, GetRandom(50, 150), health->velocity);
	health->velocity[2] = 300;

	return health;
}

int MaxAmmoType (edict_t *ent, int ammo_index)
{
	if (!ammo_index)
		return 0;
	else if (ammo_index == ITEM_INDEX(FindItemByClassname("ammo_shells")))
		return ent->client->pers.max_shells;
	else if (ammo_index == ITEM_INDEX(FindItemByClassname("ammo_bullets")))
		return ent->client->pers.max_bullets;
	else if (ammo_index ==ITEM_INDEX(FindItemByClassname("ammo_grenades")))
		return ent->client->pers.max_grenades;
	else if (ammo_index == ITEM_INDEX(FindItemByClassname("ammo_rockets")))
		return ent->client->pers.max_rockets;
	else if (ammo_index == ITEM_INDEX(FindItemByClassname("ammo_cells")))
		return ent->client->pers.max_cells;
	else if (ammo_index == ITEM_INDEX(FindItemByClassname("ammo_slugs")))
		return ent->client->pers.max_slugs;
	else return 0;
}

int G_GetRespawnWeaponIndex (edict_t *ent)
{
	switch (ent->myskills.respawn_weapon)
	{
	case 2:		return ITEM_INDEX(Fdi_SHOTGUN);
	case 3:		return ITEM_INDEX(Fdi_SUPERSHOTGUN);
	case 4:		return ITEM_INDEX(Fdi_MACHINEGUN);
	case 5:		return ITEM_INDEX(Fdi_CHAINGUN);
	case 6:		return ITEM_INDEX(Fdi_GRENADELAUNCHER);
	case 7:		return ITEM_INDEX(Fdi_ROCKETLAUNCHER);
	case 8:		return ITEM_INDEX(Fdi_HYPERBLASTER);
	case 9:		return ITEM_INDEX(Fdi_RAILGUN);
	case 10:	return ITEM_INDEX(Fdi_BFG);
	case 11:	return grenade_index;
	case 12:	return ITEM_INDEX(Fdi_20MM);
	default:	return 0;
	}
}

int G_GetAmmoIndexByWeaponIndex (int weapon_index)
{
	//gi.dprintf("weapon_index=%d\n", weapon_index);

	if (!weapon_index)
		return 0;
	else if (weapon_index == ITEM_INDEX(Fdi_SHOTGUN))
		return shell_index;
	else if (weapon_index == ITEM_INDEX(Fdi_SUPERSHOTGUN))
		return shell_index;
	else if (weapon_index == ITEM_INDEX(Fdi_MACHINEGUN))
		return bullet_index;
	else if (weapon_index == ITEM_INDEX(Fdi_CHAINGUN))
		return bullet_index;
	else if (weapon_index == ITEM_INDEX(Fdi_GRENADELAUNCHER))
		return grenade_index;
	else if (weapon_index == ITEM_INDEX(Fdi_ROCKETLAUNCHER))
		return rocket_index;
	else if (weapon_index == ITEM_INDEX(Fdi_HYPERBLASTER))
		return cell_index;
	else if (weapon_index == ITEM_INDEX(Fdi_RAILGUN))
		return slug_index;
	else if (weapon_index == ITEM_INDEX(Fdi_BFG))
		return cell_index;
	else if (weapon_index == ITEM_INDEX(Fdi_20MM))
		return shell_index;
	else if (weapon_index == grenade_index)
		return grenade_index;
	return 0;
}

float AmmoLevel (edict_t *ent, int ammo_index)
{
	int	max;

	// unknown weapon, return full ammo status
	if (!ammo_index)
		return 1.0;

	// check for max ammo of current weapon
	max = MaxAmmoType(ent, ammo_index);

	// can't determine max ammo, so return full ammo status
	if (!max)
		return 1.0;

	// return ratio of current ammo to max
	return ((float)ent->client->pers.inventory[ammo_index]/max);
}

qboolean PlayerHasSentry (edict_t *ent)
{
	edict_t *e=NULL;

	while((e = G_Find(e, FOFS(classname), "Sentry_Gun")) != NULL) 
	{
		if (e && e->inuse && e->creator && (e->creator == ent))
			return true;
	}
	return false;
}

void station_dropitem (edict_t *self)
{
	int		index;
	float	health, armor, ammo;
	gitem_t	*item=NULL;
	edict_t *item_ent;

	// if there's nobody around, drop a random item
	if (!self->enemy)
	{
		// don't cause an entity overflow
		if (numNearbyEntities(self, 128, true) > 5)
			return;

		switch (GetRandom(1,3))
		{
		case 1: item_ent = SpawnHealthBox(self, HEALTHBOX_LARGE); break;
		case 2: item = FindItemByClassname("item_armor_combat"); break;
		case 3: item_ent = DropRandomAmmo(self); break;
		}

		if (item)	item_ent = Drop_Item(self, item);
	}
	else
	{
		health = HealthLevel(self->enemy);
		armor = ArmorLevel(self->enemy);

		// get ammo level for respawn weapon
		index = G_GetAmmoIndexByWeaponIndex(G_GetRespawnWeaponIndex(self->enemy));
		ammo = AmmoLevel(self->enemy, index);

		// if we have some ammo for respawn weapon, then get ammo level for equipped weapon
		if (ammo >= 0.25)
		{
			index = self->enemy->client->ammo_index;
			ammo = AmmoLevel(self->enemy, index);
		}

		// if we have full ammo AND we have a sentry built, make sure we
		// have some rockets or bullets to feed the sentry
		if ((ammo == 1.0) && PlayerHasSentry(self->enemy))
		{
			int bullet_ammo = self->enemy->client->pers.inventory[bullet_index];
			int rocket_ammo = self->enemy->client->pers.inventory[rocket_index];

			if (!bullet_ammo || !rocket_ammo)
			{
				if (!bullet_ammo)
					index = bullet_index;
				else if (!rocket_ammo)
					index = rocket_index;
				ammo = 0; // indicate that we need ammo dropped
			}
		}
		// drop health
		if ((health < 1) && (health <= armor) && (health <= ammo))
			item_ent = SpawnHealthBox(self, HEALTHBOX_LARGE);
		// drop armor
		else if ((armor < 1) && (armor <= health) && (armor <= ammo))
			item = FindItemByClassname("item_armor_combat");
		// drop ammo
		else if (index && (ammo < 1))
			item = GetItemByIndex(index);
		else
		{
			// drop armor shards or stimpaks
			if (random() > 0.5)
				item = FindItemByClassname("item_armor_shard");
			else
				item_ent = SpawnHealthBox(self, HEALTHBOX_SMALL);
		}

		// drop selected item
		if (item)
			item_ent = Drop_Item(self, item);
	}
}

void depot_giveitems (edict_t *self)
{

}

void station_seteffects (edict_t *self)
{
	vec3_t start, forward;
	
	self->s.effects = EF_ROTATE;
	if (!self->s.sound)
		self->s.sound = gi.soundindex("world/amb15.wav");
	if (level.time > self->delay)
	{
		self->s.angles[YAW] = GetRandom(0, 360);
		AngleVectors(self->s.angles, forward, NULL, NULL);
		VectorCopy(self->s.origin, start);
		start[2] = self->absmax[2] + GetRandom(8, 32);
		VectorMA(start, GetRandom(0, 32), forward, start);

		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_SPARKS);
		gi.WritePosition (start);
		gi.WriteDir (vec3_origin);
		gi.multicast (start, MULTICAST_PVS);

		self->delay = level.time + 0.1*GetRandom(1, 5);
	}
}

void supplystation_think (edict_t *self)
{
	float	time, temp;

	if (level.time > self->wait)
	{
		station_findtarget(self);
		station_dropitem(self);

		temp = 1 + 0.9*self->monsterinfo.level;
		time = 50 / temp;
		self->wait = level.time + time; // time until we drop another item
	}
	
	station_seteffects(self);
	self->nextthink = level.time + FRAMETIME;
}

void depot_think (edict_t *self)
{
	/*
	float	time, temp;

	if (level.time > self->wait)
	{
		depot_findtarget(self);
		depot_giveitems(self);

		temp = 1 + 0.9*self->creator->myskills.abilities[MOBILE_DEPOT].current_level;
		time = 50 / temp;
		self->wait = level.time + time; // time until we drop another item
	}
	
	depot_seteffects(self);
	*/
	self->nextthink = level.time + FRAMETIME;

}

void supplystation_explode (edict_t *self, char *message)
{
	if (self->creator && self->creator->inuse)
	{
		gi.cprintf(self->creator, PRINT_HIGH, message);
		self->creator->supplystation = NULL;
		T_RadiusDamage(self, self->creator, 150, self, 150, MOD_SUPPLYSTATION);
	}
	BecomeExplosion1(self);
}

void supplystation_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	char *s;

	if (attacker->client)
		s = va("Your supply station was destroyed by %s.\n", attacker->client->pers.netname);
	else
		s = "Your supply station was destroyed.\n";
	supplystation_explode(self, s);
}

void supplystation_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	if (G_EntExists(other) && !OnSameTeam(self, other) && (level.time > self->random))
	{
		if (other->client)
			gi.centerprintf(self->creator, "%s is attacking\nyour station!\n", other->client->pers.netname);
		else
			gi.centerprintf(self->creator, "Your station is under\nattack!", other->client->pers.netname);
		self->random = level.time + 5;
	}
}

void BuildSupplyStation (edict_t *ent, int cost, float skill_mult, float delay_mult)
{
	vec3_t	angles, forward, end;
	edict_t *station;
	trace_t	tr;
	int talentLevel;

	station = G_Spawn();
	station->creator = ent;
	station->think = supplystation_think;
	station->nextthink = level.time + STATION_BUILD_TIME * delay_mult;
	station->s.modelindex = gi.modelindex ("models/objects/dmspot/tris.md2");
	station->s.effects |= EF_PLASMA;
	station->s.renderfx |= RF_IR_VISIBLE;
	station->solid = SOLID_BBOX;
	station->movetype = MOVETYPE_TOSS;
	station->clipmask = MASK_MONSTERSOLID;
	station->mass = 500;
	station->mtype = SUPPLY_STATION;
	station->classname = "supplystation";
	station->takedamage = DAMAGE_YES;
	station->health = 1000;
	station->monsterinfo.level = ent->myskills.abilities[SUPPLY_STATION].current_level * skill_mult;
	station->touch = V_Touch;
	station->die = supplystation_die;
	VectorSet(station->mins, -32, -32, -24);
	VectorSet(station->maxs, 32, 32, -16);
	station->s.skinnum = 1;
	// starting position for station
	AngleVectors (ent->client->v_angle, forward, NULL, NULL);
	VectorMA(ent->s.origin, 128, forward, end);
	tr = gi.trace(ent->s.origin, NULL, NULL, end, ent, MASK_SOLID);
	VectorCopy(tr.endpos, end);
	vectoangles(tr.plane.normal, angles);
	ValidateAngles(angles);
	// player is aiming at the ground
	if ((tr.fraction != 1.0) && (tr.endpos[2] < ent->s.origin[2]) && (angles[PITCH] == 270))
		end[2] += abs(station->mins[2])+1;
	// make sure station doesn't spawn in a solid
	tr = gi.trace(end, station->mins, station->maxs, end, NULL, MASK_SHOT);
	if (tr.contents & MASK_SHOT)
	{
		gi.cprintf (ent, PRINT_HIGH, "Can't build supply station there.\n");
		G_FreeEdict(station);
		return;
	}
	VectorCopy(tr.endpos, station->s.origin);
	gi.linkentity(station);

	ent->supplystation = station;
	ent->client->ability_delay = level.time + STATION_DELAY * delay_mult;
	ent->client->pers.inventory[power_cube_index] -= cost;
	ent->holdtime = level.time + STATION_BUILD_TIME * delay_mult;
	gi.sound(station, CHAN_ITEM, gi.soundindex("weapons/repair.wav"), 1, ATTN_NORM, 0);

	station->firewall_weak = false;
	station->firewall_medium = false;
	station->firewall_strong = false;

	talentLevel = getTalentLevel(ent, TALENT_PRECISION_TUNING);
	if (talentLevel == 1)
	station->firewall_weak = true;
	if (talentLevel == 2)
	station->firewall_medium = true;
	if (talentLevel == 3)
	station->firewall_strong = true;
}

void supplystation_remove (edict_t *self)
{
	if ((self->health > 1) && (level.time > self->delay))
	{
		if (self->creator && self->creator->inuse)
		{
			gi.cprintf(self->creator, PRINT_HIGH, "Supply station successfully removed.\n");
			self->creator->client->pers.inventory[power_cube_index] += STATION_COST;
		}
		G_FreeEdict(self);
	}
	self->nextthink = level.time + FRAMETIME;
}

void Cmd_CreateSupplyStation_f (edict_t *ent)
{
	int talentLevel, cost=STATION_COST;
	float skill_mult=1.0, cost_mult=1.0, delay_mult=1.0;//Talent: Rapid Assembly & Precision Tuning

	if (debuginfo->value)
		gi.dprintf("%s just called Cmd_CreateSupplyStation_f\n", ent->client->pers.netname);

	if (ent->supplystation && (level.time > ent->holdtime) 
		&& !Q_strcasecmp(gi.args(), "remove"))
	{
		gi.cprintf(ent, PRINT_HIGH, "Removing supply station...\n");
		ent->holdtime = level.time + STATION_BUILD_TIME;
		ent->client->ability_delay = level.time + STATION_DELAY;
		ent->supplystation->think = supplystation_remove;
		ent->supplystation->nextthink = level.time + FRAMETIME;
		ent->supplystation->s.effects = EF_PLASMA;
		ent->supplystation->s.sound = 0;
		return;
	}

	if (ent->supplystation)
	{
		if (ent->holdtime > level.time) // the station isn't done building
			return;
		supplystation_explode(ent->supplystation, "Your supply station blew itself up.\n");
		return;
	}

	if(ent->myskills.abilities[SUPPLY_STATION].disable)
		return;

	//Talent: Rapid Assembly
	talentLevel = getTalentLevel(ent, TALENT_RAPID_ASSEMBLY);
	if (talentLevel > 0)
		delay_mult -= 0.167 * talentLevel;

	cost *= cost_mult;

	if (!G_CanUseAbilities(ent, ent->myskills.abilities[SUPPLY_STATION].current_level, cost))
		return;
	BuildSupplyStation(ent, cost, skill_mult, delay_mult);
}

	