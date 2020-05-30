#include "g_local.h"
#include "m_player.h"

void drone_ai_stand (edict_t *self, float dist);
void drone_ai_run (edict_t *self, float dist);


mframe_t decoy_frames_stand1 [] =
{
    drone_ai_stand, 0, NULL,
    drone_ai_stand, 0, NULL,
    drone_ai_stand, 0, NULL,
    drone_ai_stand, 0, NULL,
    drone_ai_stand, 0, NULL,
    drone_ai_stand, 0, NULL,
    drone_ai_stand, 0, NULL,
    drone_ai_stand, 0, NULL,
    drone_ai_stand, 0, NULL,
    drone_ai_stand, 0, NULL,

    drone_ai_stand, 0, NULL,
    drone_ai_stand, 0, NULL,
    drone_ai_stand, 0, NULL,
    drone_ai_stand, 0, NULL,
    drone_ai_stand, 0, NULL,
    drone_ai_stand, 0, NULL,
    drone_ai_stand, 0, NULL,
    drone_ai_stand, 0, NULL,
    drone_ai_stand, 0, NULL,
    drone_ai_stand, 0, NULL,

    drone_ai_stand, 0, NULL,
    drone_ai_stand, 0, NULL,
    drone_ai_stand, 0, NULL,
    drone_ai_stand, 0, NULL,
    drone_ai_stand, 0, NULL,
    drone_ai_stand, 0, NULL,
    drone_ai_stand, 0, NULL,
    drone_ai_stand, 0, NULL,
    drone_ai_stand, 0, NULL,
    drone_ai_stand, 0, NULL,

    drone_ai_stand, 0, NULL,
    drone_ai_stand, 0, NULL,
    drone_ai_stand, 0, NULL,
    drone_ai_stand, 0, NULL,
    drone_ai_stand, 0, NULL,
    drone_ai_stand, 0, NULL,
    drone_ai_stand, 0, NULL,
    drone_ai_stand, 0, NULL,
    drone_ai_stand, 0, NULL,
    drone_ai_stand, 0, NULL
};
mmove_t decoy_move_stand = {FRAME_stand01, FRAME_stand40, decoy_frames_stand1, NULL};

void decoy_stand (edict_t *self)
{
	self->monsterinfo.currentmove = &decoy_move_stand;
}
mframe_t decoy_frames_run [] =
{
    drone_ai_run, 30, NULL,
    drone_ai_run, 30, NULL,
    drone_ai_run, 30, NULL,
    drone_ai_run, 30, NULL,
    drone_ai_run, 30, NULL,
    drone_ai_run, 30, NULL
};
mmove_t decoy_move_run = {FRAME_run1, FRAME_run6, decoy_frames_run, NULL};

void actor_run (edict_t *self);

mframe_t decoy_frames_runandtaunt [] =
{
    drone_ai_run, 30, NULL,
    drone_ai_run, 30, NULL,
    drone_ai_run, 30, NULL,
    drone_ai_run, 30, NULL,
    drone_ai_run, 30, NULL,
    drone_ai_run, 30, NULL,
    drone_ai_run, 30, NULL,
    drone_ai_run, 30, NULL,
    drone_ai_run, 30, NULL,
    drone_ai_run, 30, NULL,
    drone_ai_run, 30, NULL,
    drone_ai_run, 30, NULL,
    drone_ai_run, 30, NULL,
    drone_ai_run, 30, NULL,
    drone_ai_run, 30, NULL,
    drone_ai_run, 30, NULL,
    drone_ai_run, 30, NULL
};
mmove_t decoy_move_runandtaunt = {FRAME_taunt01, FRAME_taunt17, decoy_frames_runandtaunt, actor_run};

mframe_t decoy_frames_runandflipoff [] =
{
    drone_ai_run, 30, NULL,
    drone_ai_run, 30, NULL,
    drone_ai_run, 30, NULL,
    drone_ai_run, 30, NULL,
    drone_ai_run, 30, NULL,
    drone_ai_run, 30, NULL,
    drone_ai_run, 30, NULL,
    drone_ai_run, 30, NULL,
    drone_ai_run, 30, NULL,
    drone_ai_run, 30, NULL,
    drone_ai_run, 30, NULL,
    drone_ai_run, 30, NULL
};
mmove_t decoy_move_runandflipoff = {FRAME_flip01, FRAME_flip12, decoy_frames_runandflipoff, actor_run};

void actor_run (edict_t *self)
{
//	gi.dprintf("actor_run()\n");
	if (self->monsterinfo.aiflags & AI_STAND_GROUND)
		decoy_stand(self);
	else
		self->monsterinfo.currentmove = &decoy_move_run;
}

char *player_msg[] =
{
	"Watch it",
	"Lmao, you fail",
	"Get off me",
	"Rofl, keep failing"
};

char *other_msg[] =
{
	"Even Ivan is better than you",
	"Nice shot",
	"Haha, that was a close one",
	"Not bad"
};

void drone_pain (edict_t *self, edict_t *other, float kick, int damage);
void actor_pain (edict_t *self, edict_t *other, float kick, int damage)
{
//	char	*s;

//	gi.dprintf("actor_pain()\n");

	//if (self->health < (self->max_health / 2))
	//	self->s.skinnum = 1;

	if ((level.time < self->pain_debounce_time) && (random() > 0.2))
		return;

//	gi.sound (self, CHAN_VOICE, actor.sound_pain, 1, ATTN_NORM, 0);

    if (self->health > 40)
	gi.sound(self, CHAN_VOICE, gi.soundindex("player/male/pain50_1.wav"), 1, ATTN_NORM, 0);
	
//	if (random () < 0.5)
//		s = HiPrint(va("%s's decoy: %s %s", self->activator->client->pers.netname, 
//			player_msg[GetRandom(0, 3)], other->client->pers.netname));
//	else
//		s = HiPrint(va("%s's decoy: %s %s", self->activator->client->pers.netname, 
//			other_msg[GetRandom(0, 3)], other->client->pers.netname));
//	gi.bprintf(PRINT_HIGH, "%s\n", s);

//	if (random() < 0.5)
//	self->monsterinfo.currentmove = &decoy_move_runandflipoff;
//	else
//		self->monsterinfo.currentmove = &decoy_move_runandtaunt;

	drone_pain(self, other, kick, damage);

	self->pain_debounce_time = level.time + GetRandom(6, 10);
}

void decoy_rocket_old (edict_t *self)
{
	//int		damage, speed ;
	//vec3_t	forward, start;

	//speed = 650 + 30*self->activator->myskills.level;
	//if (speed > 950)
	//speed = 950;

	//if (random() <= 0.1)
	//	damage = 50 + 10*self->activator->myskills.level;	
	//else
	//	damage = 1;

	//MonsterAim(self, damage, speed, true, 0, forward, start);
	//monster_fire_rocket (self, start, forward, damage, speed, 0);

	//gi.WriteByte (svc_muzzleflash);
	//gi.WriteShort (self-g_edicts);
	//gi.WriteByte (MZ_ROCKET);
	//gi.multicast (self->s.origin, MULTICAST_PVS);
}

void decoy_sword (edict_t *self)
{    
	int     damage, kick;
	vec3_t	forward, start;

	if (self->decoy_next <= level.time){

	damage = SABRE_INITIAL_DAMAGE + (SABRE_ADDON_DAMAGE * self->activator->myskills.weapons[WEAPON_SWORD].mods[0].current_level);
	kick = 100;

	MonsterAim(self, 1, 0, false, 0, forward, start);
    monster_fire_sword (self, start, forward, damage, kick, 0);

	self->decoy_next = level.time + 0.1;}
}

void decoy_blaster (edict_t *self)
{
    int		damage, speed, max, min;
	vec3_t	forward, start;

	if (self->decoy_next <= level.time){

	speed = BLASTER_INITIAL_SPEED + BLASTER_ADDON_SPEED * self->activator->myskills.weapons[WEAPON_BLASTER].mods[2].current_level;
	min = BLASTER_INITIAL_DAMAGE_MIN + (BLASTER_ADDON_DAMAGE_MIN * self->activator->myskills.weapons[WEAPON_BLASTER].mods[0].current_level);
	max = BLASTER_INITIAL_DAMAGE_MAX + (BLASTER_ADDON_DAMAGE_MAX * self->activator->myskills.weapons[WEAPON_BLASTER].mods[0].current_level);
	damage = GetRandom(min, max);

	MonsterAim(self, 1, speed, false, 0, forward, start);
	monster_fire_blaster (self, start, forward, damage, speed, EF_BLASTER, BLASTER_PROJ_BOLT, 2.0, true, 0);

	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (self-g_edicts);
	gi.WriteByte (MZ_BLASTER);
	gi.multicast (self->s.origin, MULTICAST_PVS);

	self->decoy_next = level.time + 0.3;}
}

void decoy_shotgun (edict_t *self)
{    
	int		damage, pellets;
	vec3_t	forward, start;

	if (self->decoy_next <= level.time){

	damage = SHOTGUN_INITIAL_DAMAGE + SHOTGUN_ADDON_DAMAGE * self->activator->myskills.weapons[WEAPON_SHOTGUN].mods[0].current_level;
	pellets = SHOTGUN_INITIAL_BULLETS + SHOTGUN_ADDON_BULLETS * self->activator->myskills.weapons[WEAPON_SHOTGUN].mods[2].current_level;

	MonsterAim(self, 1, 0, false, 0, forward, start);
	monster_fire_shotgun (self, start, forward, damage, damage, 500, 500, pellets, 0);

	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (self-g_edicts);
	gi.WriteByte (MZ_SHOTGUN);
	gi.multicast (self->s.origin, MULTICAST_PVS);

	self->decoy_next = level.time + 0.8;}
}

void decoy_supershotgun (edict_t *self)
{    
	int		damage, pellets;
	vec3_t	forward, start;

	if (self->decoy_next <= level.time){

	damage = SUPERSHOTGUN_INITIAL_DAMAGE + SUPERSHOTGUN_ADDON_DAMAGE * self->activator->myskills.weapons[WEAPON_SUPERSHOTGUN].mods[0].current_level;
	pellets = SUPERSHOTGUN_INITIAL_BULLETS + SUPERSHOTGUN_ADDON_BULLETS * self->activator->myskills.weapons[WEAPON_SUPERSHOTGUN].mods[2].current_level;

	MonsterAim(self, 1, 0, false, 0, forward, start);
	monster_fire_shotgun (self, start, forward, damage, damage, 500, 1000, pellets, 0);

	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (self-g_edicts);
	gi.WriteByte (MZ_SSHOTGUN);
	gi.multicast (self->s.origin, MULTICAST_PVS);

	self->decoy_next = level.time + 1.0;}
}

void decoy_machinegun (edict_t *self)
{    
	int		damage;
	vec3_t	forward, start;

	damage = MACHINEGUN_INITIAL_DAMAGE + MACHINEGUN_ADDON_DAMAGE * self->activator->myskills.weapons[WEAPON_MACHINEGUN].mods[0].current_level;

	MonsterAim(self, 1, 0, false, 0, forward, start);
	monster_fire_bullet (self, start, forward, damage, damage, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, 0);

	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (self-g_edicts);
	gi.WriteByte (MZ_MACHINEGUN);
	gi.multicast (self->s.origin, MULTICAST_PVS);
}

void decoy_chaingun (edict_t *self)
{    
	int		damage;
	vec3_t	forward, start;

	damage = CHAINGUN_INITIAL_DAMAGE + CHAINGUN_ADDON_DAMAGE * self->activator->myskills.weapons[WEAPON_CHAINGUN].mods[0].current_level;

	MonsterAim(self, 1, 0, false, 0, forward, start);
	monster_fire_bullet (self, start, forward, damage, damage, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, 0);
	monster_fire_bullet (self, start, forward, damage, damage, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, 0);
	monster_fire_bullet (self, start, forward, damage, damage, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, 0);
}

void decoy_grenades (edict_t *self)
{    
	float	radius;
	vec3_t	forward, start;

	if (self->decoy_next <= level.time){

	int		damage = GRENADE_INITIAL_DAMAGE + GRENADE_ADDON_DAMAGE * self->activator->myskills.weapons[WEAPON_HANDGRENADE].mods[0].current_level;
	int		radius_damage = GRENADE_INITIAL_RADIUS_DAMAGE + GRENADE_ADDON_RADIUS_DAMAGE * self->activator->myskills.weapons[WEAPON_HANDGRENADE].mods[0].current_level;
	int		speed = 800 + 40 * self->activator->myskills.weapons[WEAPON_HANDGRENADE].mods[1].current_level;

	radius = 100;

	MonsterAim(self, 1, 0, false, 0, forward, start);
	fire_grenade2 (self, start, forward, damage, speed, 2.0, radius, radius_damage, 0);

	self->decoy_next = level.time + 2.0;}
}

void decoy_grenadelauncher (edict_t *self)
{    
//	float	radius;
	vec3_t	forward, start;

	if (self->decoy_next <= level.time){

	int damage = GRENADELAUNCHER_INITIAL_DAMAGE + GRENADELAUNCHER_ADDON_DAMAGE * self->activator->myskills.weapons[WEAPON_GRENADELAUNCHER].mods[0].current_level;
	float radius = GRENADELAUNCHER_INITIAL_RADIUS + GRENADELAUNCHER_ADDON_RADIUS * self->activator->myskills.weapons[WEAPON_GRENADELAUNCHER].mods[1].current_level;
	int speed = GRENADELAUNCHER_INITIAL_SPEED + (GRENADELAUNCHER_ADDON_SPEED * self->activator->myskills.weapons[WEAPON_GRENADELAUNCHER].mods[2].current_level);
	int	radius_damage = GRENADELAUNCHER_INITIAL_RADIUS_DAMAGE + GRENADELAUNCHER_ADDON_RADIUS_DAMAGE * self->activator->myskills.weapons[WEAPON_GRENADELAUNCHER].mods[0].current_level;

	MonsterAim(self, 1, 0, false, 0, forward, start);
	fire_grenade (self, start, forward, damage, speed, 2.5, radius, radius_damage);

	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (self-g_edicts);
	gi.WriteByte (MZ_GRENADE);
	gi.multicast (self->s.origin, MULTICAST_PVS);

	self->decoy_next = level.time + 0.7;}
}

void decoy_rocketlauncher (edict_t *self)
{    
	vec3_t	forward, start;

	if (self->decoy_next <= level.time){

    int speed = ROCKETLAUNCHER_INITIAL_SPEED + ROCKETLAUNCHER_ADDON_SPEED * self->activator->myskills.weapons[WEAPON_ROCKETLAUNCHER].mods[2].current_level;
	int damage = ROCKETLAUNCHER_INITIAL_DAMAGE + ROCKETLAUNCHER_ADDON_DAMAGE * self->activator->myskills.weapons[WEAPON_ROCKETLAUNCHER].mods[0].current_level;
	int radius_damage = ROCKETLAUNCHER_INITIAL_RADIUS_DAMAGE + ROCKETLAUNCHER_ADDON_RADIUS_DAMAGE * self->activator->myskills.weapons[WEAPON_ROCKETLAUNCHER].mods[0].current_level;
	int damage_radius = ROCKETLAUNCHER_INITIAL_DAMAGE_RADIUS + ROCKETLAUNCHER_ADDON_DAMAGE_RADIUS * self->activator->myskills.weapons[WEAPON_ROCKETLAUNCHER].mods[1].current_level;

	MonsterAim(self, 1, 0, true, 0, forward, start);
	fire_rocket (self, start, forward, damage, speed, damage_radius, radius_damage);

	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (self-g_edicts);
	gi.WriteByte (MZ_ROCKET);
	gi.multicast (self->s.origin, MULTICAST_PVS);

	self->decoy_next = level.time + 0.7;}
}

void decoy_hyperblaster (edict_t *self)
{    
	int damage;
	int speed;
	vec3_t	forward, start;

	if (self->decoy_next <= level.time){

	damage = HYPERBLASTER_INITIAL_DAMAGE + HYPERBLASTER_ADDON_DAMAGE * self->activator->myskills.weapons[WEAPON_HYPERBLASTER].mods[0].current_level;
	speed = HYPERBLASTER_INITIAL_SPEED + HYPERBLASTER_ADDON_SPEED *  self->activator->myskills.weapons[WEAPON_HYPERBLASTER].mods[2].current_level;

	MonsterAim(self, 1, 0, false, 0, forward, start);
	monster_fire_blaster (self, start, forward, damage, speed, EF_HYPERBLASTER, BLASTER_PROJ_BOLT, 10.0, false, 0);

	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (self-g_edicts);
	gi.WriteByte (MZ_HYPERBLASTER);
	gi.multicast (self->s.origin, MULTICAST_PVS);

	self->decoy_next = level.time + 0.2;}
}

void decoy_railgun (edict_t *self)
{    
	int damage;
	int kick = 200;

	vec3_t	forward, start;

	if (self->decoy_next <= level.time){

	damage = RAILGUN_INITIAL_DAMAGE + RAILGUN_ADDON_DAMAGE * self->activator->myskills.weapons[WEAPON_RAILGUN].mods[0].current_level;

	MonsterAim(self, 1, 0, false, 0, forward, start);
	fire_rail (self, start, forward, damage, kick);

	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (self-g_edicts);
	gi.WriteByte (MZ_RAILGUN);
	gi.multicast (self->s.origin, MULTICAST_PVS);

	self->decoy_next = level.time + 1.4;}
}

void decoy_bfg10k (edict_t *self)
{    
	int dmg;
	int speed;
	float range;
	vec3_t	forward, start;

	if (self->decoy_next <= level.time){

	speed = BFG10K_INITIAL_SPEED + BFG10K_ADDON_SPEED * self->activator->myskills.weapons[WEAPON_BFG10K].mods[2].current_level;
	range = BFG10K_RADIUS;
	dmg = BFG10K_INITIAL_DAMAGE + BFG10K_ADDON_DAMAGE * self->activator->myskills.weapons[WEAPON_BFG10K].mods[0].current_level;

	MonsterAim(self, 1, 0, false, 0, forward, start);
	fire_bfg (self, start, forward, dmg, speed, range);

	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (self-g_edicts);
	gi.WriteByte (MZ_BFG);
	gi.multicast (self->s.origin, MULTICAST_PVS);

	self->decoy_next = level.time + 2.3;}
}

void decoy_20mm (edict_t *self)
{    
    int damage, min, max;
	int kick;

	vec3_t	forward, start;

	if (self->decoy_next <= level.time){

	min = WEAPON_20MM_INITIAL_DMG_MIN + WEAPON_20MM_ADDON_DMG_MIN*self->activator->myskills.weapons[WEAPON_20MM].mods[0].current_level;
	max = WEAPON_20MM_INITIAL_DMG_MAX + WEAPON_20MM_ADDON_DMG_MAX*self->activator->myskills.weapons[WEAPON_20MM].mods[0].current_level;
	damage = GetRandom(min, max);
	kick = damage;

	MonsterAim(self, 1, 0, false, 0, forward, start);
	fire_20mm (self, start, forward, damage, kick);

    gi.sound (self, CHAN_WEAPON, gi.soundindex("weapons/sgun1.wav"), 1, ATTN_NORM, 0);

	self->decoy_next = level.time + 0.2;}
}

void decoy_weapon (edict_t *self)
{
	if (self->activator->client->pers.weapon == FindItem("Sword"))
		decoy_sword(self);

	if (self->activator->client->pers.weapon == FindItem("Blaster"))
		decoy_blaster(self);

	if (self->activator->client->pers.weapon == FindItem("Shotgun"))
		decoy_shotgun(self);

	if (self->activator->client->pers.weapon == FindItem("Super Shotgun"))
		decoy_supershotgun(self);

	if (self->activator->client->pers.weapon == FindItem("Machinegun"))
		decoy_machinegun(self);

	if (self->activator->client->pers.weapon == FindItem("Chaingun"))
		decoy_chaingun(self);

	if (self->activator->client->pers.weapon == FindItem("Grenades"))
		decoy_grenades(self);

	if (self->activator->client->pers.weapon == FindItem("Grenade Launcher"))
		decoy_grenadelauncher(self);

	if (self->activator->client->pers.weapon == FindItem("Rocket Launcher"))
		decoy_rocketlauncher(self);

	if (self->activator->client->pers.weapon == FindItem("HyperBlaster"))
		decoy_hyperblaster(self);

	if (self->activator->client->pers.weapon == FindItem("Railgun"))
		decoy_railgun(self);

	if (self->activator->client->pers.weapon == FindItem("BFG10K"))
		decoy_bfg10k(self);

	if (self->activator->client->pers.weapon == FindItem("20MM Cannon"))
		decoy_20mm(self);
}

void actor_dead (edict_t *self)
{
	VectorSet (self->mins, -16, -16, -24);
	VectorSet (self->maxs, 16, 16, -8);
	self->movetype = MOVETYPE_TOSS;
	self->svflags |= SVF_DEADMONSTER;
	self->nextthink = 0;
	gi.linkentity (self);
}

mframe_t actor_frames_death1 [] =
{
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0, NULL,
	ai_move, 0,  NULL,
	ai_move, 0,   NULL,
	ai_move, 0,  NULL,
};
mmove_t actor_move_death1 = {FRAME_death101, FRAME_death106, actor_frames_death1, actor_dead};

mframe_t actor_frames_death2 [] =
{
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
	ai_move, 0,  NULL,
	ai_move, 0,  NULL,
	ai_move, 0,   NULL,
	ai_move, 0,   NULL,
};
mmove_t actor_move_death2 = {FRAME_death201, FRAME_death206, actor_frames_death2, actor_dead};

void decoy_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	int		n;

	M_Notify(self);

	// check for gib
	if (self->health <= self->gib_health)
	{
		for (n= 0; n < 2; n++)
			ThrowGib (self, "models/objects/gibs/bone/tris.md2", damage, GIB_ORGANIC);
		for (n= 0; n < 4; n++)
			ThrowGib (self, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);
		//ThrowHead (self, "models/objects/gibs/head2/tris.md2", damage, GIB_ORGANIC);
		//self->deadflag = DEAD_DEAD;
		M_Remove(self, false, false);
		return;
	}

	if (self->deadflag == DEAD_DEAD)
		return;
	self->deadflag = DEAD_DEAD;

	//Talent: Exploding Decoy
	/*
	if(damage > 0 && self->activator && getTalentLevel(self->activator, TALENT_EXPLODING_DECOY) != -1)
	{
		int talentLevel = getTalentLevel(self->activator, TALENT_EXPLODING_DECOY);
		int decoyDamage = 100 + talentLevel * 50;
		int decoyRadius = 100 + talentLevel * 25;

		T_RadiusDamage(self, self->activator, decoyDamage, self, decoyRadius, MOD_EXPLODING_DECOY);

		self->takedamage = DAMAGE_NO;
		self->think = BecomeExplosion1;
		self->nextthink = level.time + FRAMETIME;
		return;
	}*/

	// regular death
	self->takedamage = DAMAGE_YES;
	self->s.modelindex2 = 0;

	if (random() < 0.5)
	gi.sound(self, CHAN_VOICE, gi.soundindex("player/male/death1.wav"), 1, ATTN_NORM, 0);
	else
	gi.sound(self, CHAN_VOICE, gi.soundindex("player/male/death2.wav"), 1, ATTN_NORM, 0);

//	gi.bprintf(PRINT_HIGH, "%s's decoy was killed by %s.\n", self->activator->client->pers.netname, attacker->client->pers.netname);

	n = randomMT() % 2;
	if (n == 0)		self->monsterinfo.currentmove = &actor_move_death1;
	else			self->monsterinfo.currentmove = &actor_move_death2;	
}

mframe_t actor_frames_attack [] =
{
	drone_ai_run, 30,  decoy_weapon,
	drone_ai_run, 30,  decoy_weapon,
	drone_ai_run, 30,  decoy_weapon,
	drone_ai_run, 30,  decoy_weapon,
	drone_ai_run, 30,  decoy_weapon,
	drone_ai_run, 30,  decoy_weapon,
	drone_ai_run, 30,  decoy_weapon,
	drone_ai_run, 30,  decoy_weapon
};
mmove_t actor_move_attack = {FRAME_attack1, FRAME_attack8, actor_frames_attack, actor_run};

void actor_attack(edict_t *self)
{
//	int		n;

//	gi.dprintf("actor_attack()\n");

	self->monsterinfo.currentmove = &actor_move_attack;

	if (self->activator->client->pers.weapon == FindItem("Blaster"))
	self->monsterinfo.attack_finished = level.time + 0.3;

	if (self->activator->client->pers.weapon == FindItem("Sword"))
	self->monsterinfo.attack_finished = level.time + 1.5;

	if (self->activator->client->pers.weapon == FindItem("Shotgun"))
	self->monsterinfo.attack_finished = level.time + 0.3;

	if (self->activator->client->pers.weapon == FindItem("Super Shotgun"))
	self->monsterinfo.attack_finished = level.time + 1.0;

	if (self->activator->client->pers.weapon == FindItem("Machinegun"))
	self->monsterinfo.attack_finished = level.time + 0;

	if (self->activator->client->pers.weapon == FindItem("Chaingun"))
	self->monsterinfo.attack_finished = level.time + 0;

	if (self->activator->client->pers.weapon == FindItem("Grenades"))
	self->monsterinfo.attack_finished = level.time + 2;

	if (self->activator->client->pers.weapon == FindItem("Grenade Launcher"))
	self->monsterinfo.attack_finished = level.time + 0.7;

	if (self->activator->client->pers.weapon == FindItem("Rocket Launcher"))
	self->monsterinfo.attack_finished = level.time + 0.7;

	if (self->activator->client->pers.weapon == FindItem("Hyperblaster"))
	self->monsterinfo.attack_finished = level.time + 0;

	if (self->activator->client->pers.weapon == FindItem("Railgun"))
	self->monsterinfo.attack_finished = level.time + 1.4;

	if (self->activator->client->pers.weapon == FindItem("BFG10k"))
	self->monsterinfo.attack_finished = level.time + 2.3;

	if (self->activator->client->pers.weapon == FindItem("20MM Cannon"))
	self->monsterinfo.attack_finished = level.time + 0;

//	n = (rand() & 15) + 3 + 7;
//	self->monsterinfo.pausetime = level.time + n * FRAMETIME;
}

/*QUAKED misc_actor (1 .5 0) (-16 -16 -24) (16 16 32)
*/

void decoy_copy (edict_t *self)
{
	edict_t *target = self->activator;

//	if (self->mtype != M_DECOY)
//		return;

//	if (PM_PlayerHasMonster(self->activator))
//		target = self->activator->owner;

	// copy everything from our owner
	self->model = target->model;
	self->s.skinnum = target->s.skinnum;
	self->s.modelindex = target->s.modelindex;
	self->s.modelindex2 = target->s.modelindex2;
	self->s.effects = target->s.effects;
	self->s.renderfx = target->s.renderfx;

	// try to copy target's bounding box if there's room
	if (!VectorCompare(self->mins, target->mins) || !VectorCompare(self->maxs, target->maxs))
	{
		trace_t tr = gi.trace(self->s.origin, target->mins, target->maxs, self->s.origin, self, MASK_SHOT);
		if (tr.fraction == 1.0)
		{
			VectorCopy(target->mins, self->mins);
			VectorCopy(target->maxs, self->maxs);
			gi.linkentity(self);
		}
	}
}

void init_drone_decoy (edict_t *self)
{
	self->movetype = MOVETYPE_STEP;
	self->solid = SOLID_BBOX;
	VectorSet (self->mins, -16, -16, -24);
	VectorSet (self->maxs, 16, 16, 32);
	decoy_copy(self);

	self->health = 7500;

	//Limit decoy health to 1000
//	if(self->health > 7500)		self->health = 1000;

	self->max_health = self->health;
	self->gib_health = -40;
	self->mass = 200;
	self->mtype = M_DECOY;

	self->pain = actor_pain;
	self->die = decoy_die;

	self->monsterinfo.level = 99;

	self->monsterinfo.stand = decoy_stand;
	self->monsterinfo.run = actor_run;
	self->monsterinfo.attack = actor_attack;
	self->monsterinfo.control_cost = 0;
	self->monsterinfo.cost = 0;
	self->monsterinfo.jumpup = 64;
	self->monsterinfo.jumpdn = 512;

	//K03 Begin
//	self->monsterinfo.power_armor_type = POWER_ARMOR_SHIELD;
//	self->monsterinfo.power_armor_power = 400;
	//K03 End

	//decino: if using sword try to get close
	if (self->activator->client->pers.weapon == FindItem("Sword"))
	{
		self->monsterinfo.aiflags |= AI_NO_CIRCLE_STRAFE;
	}

//	self->monsterinfo.aiflags |= AI_GOOD_GUY;

	gi.linkentity (self);

	self->monsterinfo.currentmove = &decoy_move_stand;
	self->monsterinfo.scale = MODEL_SCALE;
}

qboolean MirroredEntitiesExist (edict_t *ent);

void Cmd_Decoy_f (edict_t *ent)
{
	if (ent->myskills.administrator < 90)
		return;

	if (ent->client->ability_delay > level.time)
		return;

//	if (level.time < pregame_time->value)
//	{
//		gi.cprintf(ent, PRINT_HIGH, "You can't use abilities during pre-game.\n");
//		return;
//	}

//	if (ent->client->pers.inventory[power_cube_index] < 50)
//	{
//		gi.cprintf(ent, PRINT_HIGH, "You must have at least 50 cubes to spawn a Decoy!\n");
//		return;
//	}

	SpawnDrone(ent, 20, false);
}
