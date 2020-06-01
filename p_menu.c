#include "g_local.h"

#define VOTE_MAP	1
#define	VOTE_MODE	2

qboolean loc_CanSee (edict_t *targ, edict_t *inflictor);
void check_for_levelup(edict_t *ent);

//Function prototypes required for this .c file:
void GDS_LoadPlayer(edict_t *ent);
void OpenDOMJoinMenu (edict_t *ent);

void disableAbilities (edict_t *ent)
{
	int i;

	for (i=0; i<MAX_ABILITIES; ++i)
	{
		ent->myskills.abilities[i].disable = true;
		ent->myskills.abilities[i].hidden = false;
	}
}

void setHardMax(edict_t *ent, int index)
{
	switch(index)
	{
		//Skills that max at level 1
		case ID:
		case WORLD_RESIST:
		case BULLET_RESIST:
		case SHELL_RESIST:
		case ENERGY_RESIST:
		case PIERCING_RESIST:
		case SPLASH_RESIST:
		//case FREEZE_SPELL:
		case CLOAK:
		case CREATE_QUAD:
		case CREATE_INVIN:
		case BOOST_SPELL:
		case SUPER_SPEED:
		case DECOY:
		case WEAPON_KNOCK:
		case TELEPORT:
		case JETPACK:
		case SHIELD:
			ent->myskills.abilities[index].hard_max = 1; break;
		//Everything else
		default:
			ent->myskills.abilities[index].hard_max = ent->myskills.abilities[index].max_level * 2; break;//decino: changed max cap to 2x
	}	
}

void enableAbility (edict_t *ent, int index, int level, int max_level, qboolean general)
{
	ent->myskills.abilities[index].disable = false;

	// we can pass this function -1 if we don't want to alter these variables
	if (max_level != -1)
		ent->myskills.abilities[index].max_level = max_level;

	if (level != -1)
	{
		ent->myskills.abilities[index].level = level;
		ent->myskills.abilities[index].current_level = level;
	}

	ent->myskills.abilities[index].general_skill = general;
	setHardMax(ent, index);
}

// NOTE: abilities must match with getClassRuneStat() in v_utils.c
void setClassAbilities (edict_t *ent)
{
	switch (ent->myskills.class_num)
	{
	case CLASS_SOLDIER:// 70 points
		enableAbility(ent, STRENGTH, 0, 10, false);
		enableAbility(ent, RESISTANCE, 0, 10, false);
		enableAbility(ent, HA_PICKUP, 0, 10, false);
		enableAbility(ent, NAPALM, 0, 10, false);
		enableAbility(ent, SPIKE_GRENADE, 0, 10, false);
		enableAbility(ent, EMP, 0, 10, false);
		enableAbility(ent, MIRV, 0, 10, false);
		enableAbility(ent, GRAPPLE_HOOK, 3, 3, false);
		break;
	case CLASS_KNIGHT:// 70 points
		enableAbility(ent, ARMOR_UPGRADE, 0, 10, false);
		enableAbility(ent, REGENERATION, 0, 10, false);
		enableAbility(ent, POWER_SHIELD, 0, 10, false);
		enableAbility(ent, ARMOR_REGEN, 0, 10, false);
		enableAbility(ent, EXPLODING_ARMOR, 0, 10, false);
		enableAbility(ent, BEAM, 0, 10, false);
		enableAbility(ent, PLASMA_BOLT, 0, 10, false);
		enableAbility(ent, SHIELD, 1, 1, false);
		enableAbility(ent, BOOST_SPELL, 1, 1, false);
		ent->myskills.respawn_weapon = 1;	//sword only
		break;
	case CLASS_VAMPIRE:// 80 points
		enableAbility(ent, VAMPIRE, 0, 10, false);
		enableAbility(ent, GHOST, 0, 10, false);
		enableAbility(ent, LIFE_DRAIN, 0, 10, false);
		enableAbility(ent, FLESH_EATER, 0, 10, false);
		enableAbility(ent, CORPSE_EXPLODE, 0, 10, false);
		enableAbility(ent, MIND_ABSORB, 0, 10, false);
		enableAbility(ent, AMMO_STEAL, 0, 10, false);
		enableAbility(ent, CONVERSION, 0, 10, false);
		enableAbility(ent, CLOAK, 1, 1, false);
		break;
	case CLASS_NECROMANCER:// 80 points
		enableAbility(ent, MONSTER_SUMMON, 0, 10, false);
		enableAbility(ent, HELLSPAWN, 0, 10, false);
		enableAbility(ent, PLAGUE, 0, 10, false);
		enableAbility(ent, AMP_DAMAGE, 0, 10, false);
		enableAbility(ent, CRIPPLE, 0, 10, false);
		enableAbility(ent, CURSE, 0, 10, false);
		enableAbility(ent, WEAKEN, 0, 10, false);
		enableAbility(ent, LOWER_RESIST, 0, 10, false);
		enableAbility(ent, JETPACK, 1, 1, false);
		break;
	case CLASS_ENGINEER:// 80 points
		enableAbility(ent, PROXY, 0, 10, false);
		enableAbility(ent, BUILD_SENTRY, 0, 10, false);
		enableAbility(ent, SUPPLY_STATION, 0, 10, false);
		enableAbility(ent, BUILD_LASER, 0, 10, false);
		enableAbility(ent, MAGMINE, 0, 10, false);
		enableAbility(ent, CALTROPS, 0, 10, false);
		enableAbility(ent, AUTOCANNON, 0, 10, false);
		enableAbility(ent, DETECTOR, 0, 10, false);
		enableAbility(ent, DECOY, 1, 1, false);
		break;
	case CLASS_MAGE:// 80 points
		enableAbility(ent, MAGICBOLT, 0, 10, false);
		enableAbility(ent, NOVA, 0, 10, false);
		enableAbility(ent, BOMB_SPELL, 0, 10, false);
		enableAbility(ent, FORCE_WALL, 0, 10, false);
		enableAbility(ent, LIGHTNING, 0, 10, false);
		enableAbility(ent, METEOR, 0, 10, false);
		enableAbility(ent, FIREBALL, 0, 10, false);
		enableAbility(ent, LIGHTNING_STORM, 0, 10, false);
		enableAbility(ent, TELEPORT, 1, 1, false);
		break;
	case CLASS_CLERIC:// 80 points
		enableAbility(ent, SALVATION, 0, 10, false);
		enableAbility(ent, HOLY_FREEZE, 0, 10, false);
		enableAbility(ent, HEALING, 0, 10, false);
		enableAbility(ent, BLESS, 0, 10, false);
		enableAbility(ent, YIN, 0, 10, false);
		enableAbility(ent, YANG, 0, 10, false);
		enableAbility(ent, HAMMER, 0, 10, false);
		enableAbility(ent, DEFLECT, 0, 10, false);
		enableAbility(ent, DOUBLE_JUMP, 1, 1, false);
		break;
	case CLASS_POLTERGEIST:// 80 points
		enableAbility(ent, BERSERK, 1, 10, false);
		enableAbility(ent, CACODEMON, 1, 10, false);
		enableAbility(ent, BLOOD_SUCKER, 1, 10, false);
		enableAbility(ent, BRAIN, 1, 10, false);
		enableAbility(ent, FLYER, 1, 10, false);
		enableAbility(ent, MUTANT, 1, 10, false);
		enableAbility(ent, TANK, 1, 10, false);
		enableAbility(ent, MEDIC, 1, 10, false);
		enableAbility(ent, GHOST, 99, 99, false);
		enableAbility(ent, MORPH_MASTERY, 1, 1, false);
		break;
	case CLASS_SHAMAN:// 80 points
		enableAbility(ent, FIRE_TOTEM, 0, 10, false);
		enableAbility(ent, WATER_TOTEM, 0, 10, false);
		enableAbility(ent, AIR_TOTEM, 0, 10, false);
		enableAbility(ent, EARTH_TOTEM, 0, 10, false);
		enableAbility(ent, DARK_TOTEM, 0, 10, false);
		enableAbility(ent, NATURE_TOTEM, 0, 10, false);
		enableAbility(ent, FURY, 0, 10, false);
		enableAbility(ent, HASTE, 0, 10, false);//decino: added haste back
		enableAbility(ent, TOTEM_MASTERY, 1, 1, false); //decino: replaced with superspeed
		break;
	case CLASS_ALIEN:// 80 points
		enableAbility(ent, SPIKER, 0, 10, false);
		enableAbility(ent, OBSTACLE, 0, 10, false);
		enableAbility(ent, GASSER, 0, 10, false);
		enableAbility(ent, HEALER, 0, 10, false);
		enableAbility(ent, SPORE, 0, 10, false);
		enableAbility(ent, ACID, 0, 10, false);
		enableAbility(ent, SPIKE, 0, 10, false);
		enableAbility(ent, COCOON, 0, 10, false);
		enableAbility(ent, BLACKHOLE, 1, 1, false);
		break;
	case CLASS_WEAPONMASTER:// 0 points
		break;
	}
}

void setGeneralAbilities (edict_t *ent)
{
	// general
	enableAbility(ent, VITALITY, 0, 10, true);
	enableAbility(ent, ID, 0, 1, true);
	enableAbility(ent, SHELL_RESIST, 0, 1, true);
	enableAbility(ent, BULLET_RESIST, 0, 1, true);
	enableAbility(ent, SPLASH_RESIST, 0, 1, true);
	enableAbility(ent, PIERCING_RESIST, 0, 1, true);
	enableAbility(ent, ENERGY_RESIST, 0, 1, true);
	enableAbility(ent, MAX_AMMO, 0, 10, true);
	enableAbility(ent, POWER_REGEN, 0, 5, true);
	enableAbility(ent, WORLD_RESIST, 0, 1, true);

	// ammo regen is hard capped at 5
	enableAbility(ent, AMMO_REGEN, 0, 5, true);
//	ent->myskills.abilities[AMMO_REGEN].hard_max = 5; decino: wtf? no

	// shared
	enableAbility(ent, REGENERATION, 0, 5, true);
	enableAbility(ent, RESISTANCE, 0, 5, true);
	enableAbility(ent, STRENGTH, 0, 5, true);
	enableAbility(ent, HASTE, 0, 5, true);
	enableAbility(ent, VAMPIRE, 0, 5, true);
	enableAbility(ent, JETPACK, 0, 1, true);
	enableAbility(ent, CLOAK, 0, 1, true);
	enableAbility(ent, WEAPON_KNOCK, 0, 1, true);
	enableAbility(ent, ARMOR_UPGRADE, 0, 5, true);
	enableAbility(ent, AMMO_STEAL, 0, 5, true);
	enableAbility(ent, GRAPPLE_HOOK, 0, 3, true);
	enableAbility(ent, SUPPLY_STATION, 0, 5, true);
	//enableAbility(ent, FREEZE_SPELL, 0, 1, true);
	enableAbility(ent, CREATE_QUAD, 0, 1, true);
	enableAbility(ent, CREATE_INVIN, 0, 1, true);
	enableAbility(ent, POWER_SHIELD, 0, 5, true);
	enableAbility(ent, CORPSE_EXPLODE, 0, 5, true);
	enableAbility(ent, GHOST, 0, 5, true);
	enableAbility(ent, SALVATION, 0, 5, true);
	enableAbility(ent, FORCE_WALL, 0, 5, true);
//	enableAbility(ent, AMMO_REGEN, 0, 5, true);
	enableAbility(ent, BUILD_LASER, 0, 5, true);
	enableAbility(ent, HA_PICKUP, 0, 5, true);
	enableAbility(ent, BUILD_SENTRY, 0, 5, true);
	enableAbility(ent, BOOST_SPELL, 0, 1, true);
	enableAbility(ent, BLOOD_SUCKER, 0, 5, true);
	enableAbility(ent, PROXY, 0, 5, true);
	enableAbility(ent, MONSTER_SUMMON, 0, 5, true);
	enableAbility(ent, SUPER_SPEED, 0, 1, true);
	enableAbility(ent, ARMOR_REGEN, 0, 5, true);
	enableAbility(ent, BOMB_SPELL, 0, 5, true);
	enableAbility(ent, LIGHTNING, 0, 5, true);
	enableAbility(ent, DECOY, 0, 1, true);
	enableAbility(ent, HOLY_FREEZE, 0, 5, true);
	enableAbility(ent, CACODEMON, 0, 5, true);
	enableAbility(ent, PLAGUE, 0, 5, true);
	enableAbility(ent, FLESH_EATER, 0, 5, true);
	enableAbility(ent, HELLSPAWN, 0, 5, true);
	enableAbility(ent, BEAM, 0, 5, true);
	enableAbility(ent, BRAIN, 0, 5, true);
	enableAbility(ent, MAGMINE, 0, 5, true);
	enableAbility(ent, CRIPPLE, 0, 5, true);
	enableAbility(ent, MAGICBOLT, 0, 5, true);
	enableAbility(ent, TELEPORT, 0, 1, true);
	enableAbility(ent, NOVA, 0, 5, true);
	enableAbility(ent, EXPLODING_ARMOR, 0, 5, true);
	enableAbility(ent, MIND_ABSORB, 0, 5, true);
	enableAbility(ent, LIFE_DRAIN, 0, 5, true);
	enableAbility(ent, AMP_DAMAGE, 0, 5, true);
	enableAbility(ent, CURSE, 0, 5, true);
	enableAbility(ent, BLESS, 0, 5, true);
	enableAbility(ent, WEAKEN, 0, 5, true);
	enableAbility(ent, HEALING, 0, 5, true);
//	enableAbility(ent, AMMO_UPGRADE, 0, 5, true);
	enableAbility(ent, YIN, 0, 5, true);
	enableAbility(ent, YANG, 0, 5, true);
	enableAbility(ent, FLYER, 0, 5, true);
	enableAbility(ent, MUTANT, 0, 5, true);
	enableAbility(ent, TANK, 0, 5, true);
	enableAbility(ent, BERSERK, 0, 5, true);
	enableAbility(ent, SPIKE, 0, 5, true);
	enableAbility(ent, MORPH_MASTERY, 0, 1, true);
	enableAbility(ent, NAPALM, 0, 5, true);
	enableAbility(ent, MEDIC, 0, 5, true);
	enableAbility(ent, METEOR, 0, 5, true);
	enableAbility(ent, AUTOCANNON, 0, 5, true);
	enableAbility(ent, HAMMER, 0, 5, true);
	enableAbility(ent, BLACKHOLE, 0, 1, true);
	enableAbility(ent, FIRE_TOTEM, 0, 5, true);
	enableAbility(ent, WATER_TOTEM, 0, 5, true);
	enableAbility(ent, AIR_TOTEM, 0, 5, true);
	enableAbility(ent, EARTH_TOTEM, 0, 5, true);
	enableAbility(ent, DARK_TOTEM, 0, 5, true);
	enableAbility(ent, NATURE_TOTEM, 0, 5, true);
	enableAbility(ent, FURY, 0, 5, true);
	enableAbility(ent, TOTEM_MASTERY, 0, 1, true);
	enableAbility(ent, SHIELD, 0, 1, true);
	enableAbility(ent, CALTROPS, 0, 5, true);
	enableAbility(ent, SPIKE_GRENADE, 0, 5, true);
	enableAbility(ent, DETECTOR, 0, 5, true);
	enableAbility(ent, CONVERSION, 0, 5, true);
	enableAbility(ent, DEFLECT, 0, 5, true);
	enableAbility(ent, SCANNER, 0, 1, true);
	enableAbility(ent, EMP, 0, 5, true);
	enableAbility(ent, DOUBLE_JUMP, 0, 1, true);
	enableAbility(ent, LOWER_RESIST, 0, 5, true);
	enableAbility(ent, FIREBALL, 0, 5, true);
	enableAbility(ent, PLASMA_BOLT, 0, 5, true);
	enableAbility(ent, LIGHTNING_STORM, 0, 5, true);
	enableAbility(ent, MIRV, 0, 5, true);
	enableAbility(ent, SPIKER, 0, 5, true);
	enableAbility(ent, OBSTACLE, 0, 5, true);
	enableAbility(ent, GASSER, 0, 5, true);
	enableAbility(ent, HEALER, 0, 5, true);
	enableAbility(ent, SPORE, 0, 5, true);
	enableAbility(ent, ACID, 0, 5, true);
	enableAbility(ent, COCOON, 0, 5, true);
}

void ChaseCam(edict_t *ent)
{
	if (ent->client->menustorage.menu_active)
		closemenu(ent);

	if (ent->client->chase_target) {
		ent->client->chase_target = NULL;
		ent->client->ps.gunindex = 0;
		return;
	}

	GetChaseTarget(ent);
}

int ClassNum(edict_t *ent, int team); //GHz
int HighestLevelPlayer(void); //GHz
int TotalPlayersInGame(void)
{
	edict_t *player;
	int i, total = 0;

	for (i = 1; i <= maxclients->value; i++){
		player = &g_edicts[i];

		if (!player->inuse)
			continue;
		if (!player->client)
			continue;
		if (player->solid == SOLID_NOT)
			continue;

		total++;
	}
//	gi.dprintf("found %d players in game\n", total);
	return total;
}

void player_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	V_Touch(self, other, plane, surf);
}

void StartGame (edict_t *ent)
{
	int		i;
	gitem_t *item=itemlist;

	ent->svflags &= ~SVF_NOCLIENT;
	ent->client->resp.spectator = false;
	ent->client->pers.spectator = false;
	ent->client->ps.stats[STAT_SPECTATOR] = 0;
	PutClientInServer (ent);
	average_player_level = AveragePlayerLevel();
	ent->health = ent->myskills.current_health;
	for (i=0; i<game.num_items; i++, item++)
		ent->client->pers.inventory[ITEM_INDEX(item)] = ent->myskills.inventory[ITEM_INDEX(item)];
	ent->client->pers.inventory[flag_index] = 0; // NO FLAG FOR YOU!!!
	ent->client->pers.inventory[red_flag_index] = 0;
	ent->client->pers.inventory[blue_flag_index] = 0;
	//4.2 remove techs
	ent->client->pers.inventory[resistance_index] = 0;
	ent->client->pers.inventory[strength_index] = 0;
	ent->client->pers.inventory[haste_index] = 0;
	ent->client->pers.inventory[regeneration_index] = 0;
	ent->touch = player_touch;
	modify_max(ent);
	Pick_respawnweapon(ent);
		
	// add a teleportation effect
	ent->s.event = EV_PLAYER_TELEPORT;
	// hold in place briefly
	ent->client->ps.pmove.pm_flags = PMF_TIME_TELEPORT;
	ent->client->ps.pmove.pm_time = 14;

	if (ent->myskills.boss > 0)
	{
		gi.bprintf(PRINT_HIGH, "A level %d boss known as %s starts their reign.\n", 
			ent->myskills.boss, ent->client->pers.netname);
	}
	else if (IsNewbieBasher(ent) && !ptr->value && !domination->value)
	{
		gi.bprintf(PRINT_HIGH, "A level %d mini-boss known as %s begins their domination.\n", 
		ent->myskills.level, ent->client->pers.netname);
		gi.bprintf(PRINT_HIGH, "Kill %s, and every non-boss gets a reward!\n", ent->client->pers.netname);
		gi.cprintf(ent, PRINT_HIGH, "You are currently a mini-boss. You will receive no point loss, but cannot war.\n");
	}
	else
	{
		gi.bprintf(PRINT_HIGH, "%s starts their reign.\n", ent->client->pers.netname);
	}

	ent->ghetto_pc = false;

	//Set the player's name
	strcpy(ent->myskills.player_name, ent->client->pers.netname);

	if (level.time < pregame_time->value) {
		gi.centerprintf(ent, "This map is currently in pre-game\nPlease warm up, upgrade and\naccess the Armory now.\n");
		ent->s.effects |= EF_COLOR_SHELL;
		ent->s.renderfx |= (RF_SHELL_RED | RF_SHELL_GREEN | RF_SHELL_BLUE);
	}
	gi.sound(ent, CHAN_VOICE, gi.soundindex("misc/startup.wav"), 1, ATTN_NORM, 0);
	WriteToLogfile(ent, "Logged in.\n");

	savePlayer(ent);
}

void JoinTheGame (edict_t *ent)
{
	int		returned;

	if (ent->client->menustorage.menu_active)
	{
		closemenu(ent);
		return;
	}

	if(gds->value)	returned = GDS_OpenConfigFile(ent);
	else			returned = OpenConfigFile(ent);

	switch(returned)
	{
	case -1:	//bad password
		gi.cprintf(ent, PRINT_HIGH, "Access denied. Incorrect password.\n");
		return;
//	case -2:	//below minimum level
//		gi.cprintf(ent, PRINT_HIGH, "You have to be at least level %d to play on this server.\n", ((int)min_level->value));
//		return;
//	case -3:	//above maximum level
//		gi.cprintf(ent, PRINT_HIGH, "You have to be level %d or below to play here.\n", ((int)max_level->value));
//		if (strcmp(reconnect_ip->string, "0") != 0)
//		{
//			gi.cprintf(ent, PRINT_HIGH, "You are being sent to an alternate server where you can play.\n");
//			stuffcmd(ent, va("connect %s\n", reconnect_ip->string));
//		}
		return;
	case -4:	//invalid player name
		gi.cprintf(ent, PRINT_HIGH, "Your name must be greater than 2 characters long.\n");
		return;
//	case -5:	//playing too much
//		gi.cprintf(ent, PRINT_HIGH, "Can't join: %d hour play-time limit reached.\n", MAX_HOURS);
//		gi.cprintf(ent, PRINT_HIGH, "Please try a different character, or try again tommorow.\n");
//		return;
//	case -6:	//newbie basher can't play
//		gi.cprintf(ent, PRINT_HIGH, "Unable to join: The current maximum level is %d.\n", NEWBIE_BASHER_MAX);
//		gi.cprintf(ent, PRINT_HIGH, "Please return at a later time, or try a different character.\n");
//		gi.dprintf("INFO: %s exceeds maximum level allowed by server (level %d)!", ent->client->pers.netname, NEWBIE_BASHER_MAX);
//		return;
//	case -7:	//boss can't play
//		gi.cprintf(ent, PRINT_HIGH, "Unable to join: Bosses are not allowed unless the server is at least half capacity.\n");
//		gi.cprintf(ent, PRINT_HIGH, "Please come back at a later time, or try a different character.\n");
//		return;
	default:	//passed every check
		break;
	}

	if (ptr->value)
	{
		OpenPTRJoinMenu(ent);
		return;
	}

	if (domination->value)
	{
		OpenDOMJoinMenu(ent);
		return;
	}

	if (ctf->value)
	{
		CTF_OpenJoinMenu(ent);
		return;
	}

	if (ent->myskills.class_num == 0)
	{
		OpenClassMenu(ent, 1); //GHz
		return;
	}

	StartGame(ent);

	/*
	if (ent->myskills.inuse)
	{
		gi.cprintf(ent, PRINT_HIGH, "WARNING: Your character file is marked as already being open!\n");
		gi.cprintf(ent, PRINT_HIGH, "Logging into a server twice is not permitted. A message will be sent to an administrator.\n");
		//gi.dprintf("WARNING: %s's file is marked as already open at %s on %s.\n", ent->client->pers.netname, CURRENT_TIME, CURRENT_DATE);
	}
	ent->myskills.inuse = 1;
	*/
}

qboolean StartClient(edict_t *ent)
{
	if (ent->fail_name)
		return false;

	if (ent->client->pers.spectator) 
	{
		OpenJoinMenu(ent);
		return true;
	}
	return false;
}

void OpenJoinMenu (edict_t *ent);

void StopPrompt(edict_t *ent)
{
	ent->enteringPW = false;
	ent->enteringNAME = false;
	closemenu(ent);

	if (ent->movetype == MOVETYPE_NOCLIP)
	OpenJoinMenu(ent);
}

void joinmenu_handler (edict_t *ent, int option);

void PasswordMenu(edict_t *ent)
{
	if (!ShowMenu(ent))
        return;
	clearmenu(ent);

	//				    xxxxxxxxxxxxxxxxxxxxxxxxxxx (max length 27 chars)
	addlinetomenu(ent, " ", 0);
	addlinetomenu(ent, " ", 0);
    addlinetomenu(ent, "Enter your password in the", 0);
	addlinetomenu(ent, "above prompt and hit enter.", 0);
	addlinetomenu(ent, " ", 0);
	addlinetomenu(ent, "Abort by opening and", 0);
	addlinetomenu(ent, "closing the console and", 0);
	addlinetomenu(ent, "use the abort option below.", 0);
	addlinetomenu(ent, " ", 0);
	addlinetomenu(ent, " ", 0);
	addlinetomenu(ent, " ", 0);
	addlinetomenu(ent, " ", 0);
	addlinetomenu(ent, "Abort", 6);

	setmenuhandler(ent, joinmenu_handler);
	ent->client->menustorage.currentline = 13;
	stuffcmd(ent, "messagemode\n");
	ent->enteringPW = true;
	showmenu(ent);
}

void ProgrammersMenu(edict_t *ent)
{
	if (!ShowMenu(ent))
        return;
	clearmenu(ent);

	//				    xxxxxxxxxxxxxxxxxxxxxxxxxxx (max length 27 chars)

	addlinetomenu(ent, "Credits", MENU_GREEN_CENTERED);
	addlinetomenu(ent, " ", 0);
    addlinetomenu(ent, "Original design by Kombat03.", 0);
	addlinetomenu(ent, "Ideas borrowed from KOTS,", 0);
	addlinetomenu(ent, "with input from our players!", 0);
	addlinetomenu(ent, " ", 0);
	addlinetomenu(ent, "Programmers:", 0);
	addlinetomenu(ent, "GHz, Kombat03, Chamooze", 0);
	addlinetomenu(ent, "NewB, doomie, apple", 0);
	addlinetomenu(ent, "R1CH, Paril and decino", 0);
	addlinetomenu(ent, "", 0);
	addlinetomenu(ent, " ", 0);
	addlinetomenu(ent, "Return", 6);

	setmenuhandler(ent, joinmenu_handler);
	ent->client->menustorage.currentline = 13;
	showmenu(ent);
}

void SwitchCharacterMenu (edict_t *ent);

void joinmenu_handler (edict_t *ent, int option)
{
	switch (option)
	{
	case 1:
		//If no GDS is running, join the game right away.
		if(!gds->value)
			JoinTheGame(ent);
		else
		{
			gi.cprintf(ent, PRINT_HIGH, "Attempting to find your character file. Do not disconnect.\n");
			createOpenPlayerThread(ent);
		}
		break;
	case 2: 
		ChaseCam(ent); break;
	case 3: 
		closemenu(ent); break;
	//decino: added programmers menu
	case 4:
		ProgrammersMenu(ent);break;
	//decino: added a menu for passwords
	case 5:
		PasswordMenu(ent);break;
	case 6:
		StopPrompt(ent);break;
	case 7:
		SwitchCharacterMenu(ent);break;
	}
}

void OpenJoinMenu (edict_t *ent)
{
	if (!ShowMenu(ent))
        return;
	clearmenu(ent);

	//				    xxxxxxxxxxxxxxxxxxxxxxxxxxx (max length 27 chars)

	addlinetomenu(ent, "Welcome to New Vortex!", MENU_GREEN_CENTERED);
	addlinetomenu(ent, "Version: 1.0 BETA", MENU_GREEN_CENTERED);
	addlinetomenu(ent, " ", 0);
	addlinetomenu(ent, "Kill players and monsters", 0);
	addlinetomenu(ent, "for EXP to earn levels.", 0);
	addlinetomenu(ent, "Every level you recieve", 0);
	addlinetomenu(ent, "ability and weapon points", 0);
	addlinetomenu(ent, "to become stronger!", 0);
	addlinetomenu(ent, "Give us your feedback at:", 0);
	addlinetomenu(ent, "newvortex.userboard.net", 0);
	//addlinetomenu(ent, "Use points to get unique", 0);
	//addlinetomenu(ent, "abilities and upgrade", 0);
	//addlinetomenu(ent, "your weapons!", 0);
	addlinetomenu(ent, " ", 0);
	addlinetomenu(ent, "Start your reign", 1);
	addlinetomenu(ent, "Set password", 5);
	addlinetomenu(ent, "Switch characters", 7);
	addlinetomenu(ent, "Credits", 4);

//	Become a tank, spawn monsters to fight for you, or deploy devices 
//	to aid you in battle! With 11 classes and 100+ abilities,

	/* 
	decino: seperate menu for this, no one gives a shit about this

    addlinetomenu(ent, "Original design by Kombat03.", 0);
	addlinetomenu(ent, "Ideas borrowed from KOTS,", 0);
	addlinetomenu(ent, "with input from our players!", 0);
	addlinetomenu(ent, " ", 0);
	addlinetomenu(ent, "Programmers:", 0);
	addlinetomenu(ent, "GHz, Kombat03, Chamooze", 0);
	addlinetomenu(ent, "NewB, and doomie.", 0);
	addlinetomenu(ent, " ", 0);
	*/

	setmenuhandler(ent, joinmenu_handler);
	ent->client->menustorage.currentline = 12;
	showmenu(ent);
}

void myinfo_handler (edict_t *ent, int option)
{
	closemenu(ent);
}

void OpenMyinfoMenu (edict_t *ent)
{
	if (!ShowMenu(ent))
        return;
	clearmenu(ent);

	//				    xxxxxxxxxxxxxxxxxxxxxxxxxxx (max length 27 chars)
	addlinetomenu(ent, va("%s (%s)", ent->client->pers.netname, 
		GetClassString(ent->myskills.class_num)), MENU_GREEN_CENTERED);
	addlinetomenu(ent, " ", 0);
	addlinetomenu(ent, va("Level:        %d", ent->myskills.level), 0);
	addlinetomenu(ent, va("Experience:   %d", ent->myskills.experience), 0);
	addlinetomenu(ent, va("Next Level:   %d", (ent->myskills.next_level-ent->myskills.experience)+ent->myskills.nerfme), 0);
	addlinetomenu(ent, va("Credits:      %d", ent->myskills.credits), 0);
//	addlinetomenu(ent, va("Respawns:     %d", ent->myskills.respawns), 0);
	if (ent->myskills.shots > 0)
		addlinetomenu(ent, va("Hit Percent:  %d%c", (int)(100*((float)ent->myskills.shots_hit/ent->myskills.shots)), '%'), 0);
	else
		addlinetomenu(ent, "Hit Percent:  --", 0);
	addlinetomenu(ent, va("Frags:        %d", ent->myskills.frags), 0);
	addlinetomenu(ent, va("Fragged:      %d", ent->myskills.fragged), 0);
	if (ent->myskills.fragged > 0)
		addlinetomenu(ent, va("Frag Percent: %d%c", (int)(100*((float)ent->myskills.frags/ent->myskills.fragged)), '%'), 0);
	else
		addlinetomenu(ent, "Frag Percent: --", 0);
	addlinetomenu(ent, va("Played Hrs:   %.1f", (float)ent->myskills.playingtime/3600), 0);
	addlinetomenu(ent, " ", 0);
	addlinetomenu(ent, "Exit", 1);

	setmenuhandler(ent, myinfo_handler);
		ent->client->menustorage.currentline = 14;
	showmenu(ent);
}

void respawnmenu_handler (edict_t *ent, int option)
{
	if (option == 99)
	{
		closemenu(ent);
		return;
	}

	ent->myskills.respawn_weapon = option;
}

char *GetRespawnString (edict_t *ent)
{
	switch (ent->myskills.respawn_weapon)
	{
	case 1: return "Sword";
	case 2: return "Shotgun";
	case 3: return "Super Shotgun";
	case 4: return "Machinegun";
	case 5: return "Chaingun";
	case 6: return "Grenade Launcher";
	case 7: return "Rocket Launcher";
	case 8: return "Hyperblaster";
	case 9: return "Railgun";
	case 10: return "BFG10k";
	case 11: return "Hand Grenades";
	case 12: return "20mm Cannon";
	case 13: return "Blaster";
	default: return "Unknown";
	}
}

void OpenRespawnWeapMenu(edict_t *ent)
{
	if (!ShowMenu(ent))
        return;

	if(ent->myskills.class_num == CLASS_KNIGHT)
	{
		gi.cprintf(ent, PRINT_HIGH, "Knights are restricted to a sword.\n");
		return;
	}

	clearmenu(ent);

	//				xxxxxxxxxxxxxxxxxxxxxxxxxxx (max length 27 chars)
	addlinetomenu(ent, "Pick a respawn weapon.", MENU_GREEN_CENTERED);
	addlinetomenu(ent, " ", 0);
	addlinetomenu(ent, "Sword", 1);
	addlinetomenu(ent, "Shotgun", 2);
	addlinetomenu(ent, "Super Shotgun", 3);
	addlinetomenu(ent, "Machinegun", 4);
	addlinetomenu(ent, "Chaingun", 5);
	addlinetomenu(ent, "Hand Grenades", 11);
	addlinetomenu(ent, "Grenade Launcher", 6);
	addlinetomenu(ent, "Rocket Launcher", 7);
	addlinetomenu(ent, "Hyperblaster", 8);
	addlinetomenu(ent, "Railgun", 9);
	addlinetomenu(ent, "BFG10k", 10);
	addlinetomenu(ent, "20mm Cannon", 12);
	addlinetomenu(ent, "Blaster", 13);
	addlinetomenu(ent, " ", 0);
	addlinetomenu(ent, va("Respawn: %s", GetRespawnString(ent)), 0);
	addlinetomenu(ent, " ", 0);
	addlinetomenu(ent, "Exit", 99);

	setmenuhandler(ent, respawnmenu_handler);
		ent->client->menustorage.currentline = 3;
	showmenu(ent);
}

void classmenu_handler (edict_t *ent, int option)
{
	int points_needed;
	int page_num = (option / 1000);
	int page_choice = (option % 1000);
	int i;

	if (option == 99)
	{
		OpenClassMenu(ent, 1);//was OpenJoinMenu
		return;
	}
	else if (page_num > 0)
	{
		if (page_choice == 2)	//next
            OpenClassMenu (ent, page_num+1);
		else if (page_choice == 1)	//prev
			OpenClassMenu (ent, page_num-1);
		return;
	}
	//don't cause an invalid class to be created (option 99 is checked as well)
	else if ((option > CLASS_MAX) || (option < 1))
	{
		closemenu(ent);
		return;
	}

	// assign new players a team if the start level is low
	// otherwise, deny them access until the next game
	if (ctf->value && (level.time > pregame_time->value))
	{
		if (start_level->value > 1)
		{
			gi.cprintf(ent, PRINT_HIGH, "Can't join in the middle of a CTF game.\n");
			return;
		}

		// assign a random team
		ent->teamnum = GetRandom(1, 2);
	}

	for (i = 0; i < start_level->value; ++i)
	{
		points_needed = start_nextlevel->value * pow(nextlevel_mult->value, i);
		if (points_needed > 50000)
			points_needed = 50000;
		ent->myskills.experience += points_needed;
	}

	ent->myskills.class_num = option;
	disableAbilities(ent);
	setGeneralAbilities(ent);
	setClassAbilities(ent);
	setTalents(ent);
	gi.dprintf("INFO: %s created a new %s!\n", ent->client->pers.netname, GetClassString(ent->myskills.class_num));
	WriteToLogfile(ent, va("%s created a %s.\n", ent->client->pers.netname, GetClassString(ent->myskills.class_num)));
	check_for_levelup(ent);
	modify_max(ent);
	Give_respawnweapon(ent, ent->myskills.respawn_weapon);
	Give_respawnitems(ent);
	//StartGame(ent);
	resetWeapons(ent);

	// FIXME: we should do a better job of selecting a team OR block them from joining (temp make non-spec to make savechar() happy)
	// we need to select a team if we're past pre-game time in CTF or Domination modes
	if ((ctf->value || domination->value) && (level.time > pregame_time->value))
		ent->teamnum = GetRandom(1, 2);

	//decino: give ID at level 0
	ent->myskills.abilities[ID].level++;
	ent->myskills.abilities[ID].current_level++;

	StartGame(ent);
	SaveCharacter(ent);
}

void ClassDescription (edict_t *ent, int option)
{
	if (!ShowMenu(ent))
        return;
	clearmenu(ent);

	addlinetomenu(ent, "Class description", MENU_GREEN_CENTERED);
	addlinetomenu(ent, " ", 0);

	if (option == 1)
	{
	addlinetomenu(ent, "The soldier's abilities", 0);
	addlinetomenu(ent, "are suited for players", 0);
	addlinetomenu(ent, "that are used to the", 0);
	addlinetomenu(ent, "standard deathmatch play", 0);
	addlinetomenu(ent, "style.", 0);
	}

	if (option == 2)
	{
	addlinetomenu(ent, "The poltergeist relies", 0);
	addlinetomenu(ent, "on its ability to morph", 0);
	addlinetomenu(ent, "into monsters. In order", 0);
	addlinetomenu(ent, "to deal damage you must", 0);
	addlinetomenu(ent, "morph first.", 0);
	}

	if (option == 3)
	{
	addlinetomenu(ent, "The vampire is an expert", 0);
	addlinetomenu(ent, "in stealing their enemy's", 0);
	addlinetomenu(ent, "health. Combined with their", 0);
	addlinetomenu(ent, "high health allows them to", 0);
	addlinetomenu(ent, "live longer than most", 0);
	addlinetomenu(ent, "people would expect.", 0);
	}

	if (option == 4)
	{
	addlinetomenu(ent, "The mage offers strong and", 0);
	addlinetomenu(ent, "offensive magical spells.", 0);
	addlinetomenu(ent, "With low health and defence", 0);
	addlinetomenu(ent, "they are mostly used as a", 0);
	addlinetomenu(ent, "spell spammer. Don't forget", 0);
	addlinetomenu(ent, "to upgrade PC regen.", 0);
	}

	if (option == 5)
	{
	addlinetomenu(ent, "The engineer is a class", 0);
	addlinetomenu(ent, "more built towards camping.", 0);
	addlinetomenu(ent, "Using sentry guns and", 0);
	addlinetomenu(ent, "lasers, engineers can just", 0);
	addlinetomenu(ent, "sit back as other players", 0);
	addlinetomenu(ent, "get sliced and diced.", 0);
	}

	if (option == 6)
	{
	addlinetomenu(ent, "The knight is a very", 0);
	addlinetomenu(ent, "defensive class.", 0);
	addlinetomenu(ent, "However, he's limited to", 0);
	addlinetomenu(ent, "one weapon: the sword,", 0);
	addlinetomenu(ent, "which has limited range", 0);
	addlinetomenu(ent, "but deals high damage.", 0);
	}

	if (option == 7)
	{
	addlinetomenu(ent, "The cleric uses its holy", 0);
	addlinetomenu(ent, "powers to heal themselves", 0);
	addlinetomenu(ent, "and also for defensive", 0);
	addlinetomenu(ent, "purposes. It has higher", 0);
	addlinetomenu(ent, "health than the other", 0);
	addlinetomenu(ent, "classes.", 0);
	}

	if (option == 8)
	{
	addlinetomenu(ent, "The necromancer can summon", 0);
	addlinetomenu(ent, "monsters that will fight", 0);
	addlinetomenu(ent, "for him, but he can also", 0);
	addlinetomenu(ent, "cast curses which will", 0);
	addlinetomenu(ent, "affect the enemy's", 0);
	addlinetomenu(ent, "strength and durability.", 0);
	}

	if (option == 9)
	{
	addlinetomenu(ent, "The shaman has totems", 0);
	addlinetomenu(ent, "which will aid him while", 0);
	addlinetomenu(ent, "fighting. The totems can", 0);
	addlinetomenu(ent, "increase his damage, defense", 0);
	addlinetomenu(ent, "and even give him health.", 0);
	}

	if (option == 10)
	{
	addlinetomenu(ent, "The alien can create", 0);
	addlinetomenu(ent, "otherworldly species", 0);
	addlinetomenu(ent, "with multiple purposes.", 0);
	addlinetomenu(ent, "Some species will attack", 0);
	addlinetomenu(ent, "enemies, while others will", 0);
	addlinetomenu(ent, "heal him.", 0);
	}

	if (option == 11)
	{
	addlinetomenu(ent, "The weapon master can", 0);
	addlinetomenu(ent, "upgrade his weapons", 0);
	addlinetomenu(ent, "further than any class.", 0);
	addlinetomenu(ent, "With no class abilities,", 0);
	addlinetomenu(ent, "this class relies on his", 0);
	addlinetomenu(ent, "extra upgraded weapons.", 0);
	}

	addlinetomenu(ent, " ", 0);
	addlinetomenu(ent, "Proceed", option);
	addlinetomenu(ent, "Back", 99);

	setmenuhandler(ent, classmenu_handler);

	if (option == 1 || option == 2 || option == 9)
	ent->client->menustorage.currentline = 10;
	else
	ent->client->menustorage.currentline = 11;
	showmenu(ent);
}

void OpenClassMenu (edict_t *ent, int page_num)
{
	int i;

	if (!ShowMenu(ent))
        return;
	clearmenu(ent);

	//				xxxxxxxxxxxxxxxxxxxxxxxxxxx (max length 27 chars)
	addlinetomenu(ent, "Please select your", MENU_GREEN_CENTERED);
	addlinetomenu(ent, "character class:", MENU_GREEN_CENTERED);
	addlinetomenu(ent, " ", 0);

	for (i = ((page_num-1)*11); i < (page_num*11); ++i)
	{
		if (i < CLASS_MAX)
			addlinetomenu(ent, va("%s", GetClassString(i+1)), i+1);
		else addlinetomenu(ent, " ", 0);
	}

	addlinetomenu(ent, " ", 0);
	if (i < CLASS_MAX) addlinetomenu(ent, "Next", (page_num*1000)+2);
//	addlinetomenu(ent, "Exit", 99);

//	setmenuhandler(ent, classmenu_handler);
	setmenuhandler(ent, ClassDescription);
//	if (CLASS_MAX > 11)
	ent->client->menustorage.currentline = 4;
//	else ent->client->menustorage.currentline = 5 + i;
	showmenu(ent);
}

void masterpw_handler (edict_t *ent, int option)
{
	closemenu(ent);
}

void OpenMasterPasswordMenu (edict_t *ent)
{
	if (!ShowMenu(ent))
        return;
	clearmenu(ent);

	addlinetomenu(ent, "Master Password", MENU_GREEN_CENTERED);
	addlinetomenu(ent, " ", 0);

	if (strcmp(ent->myskills.email, ""))
	{
		addlinetomenu(ent, "A master password has", 0);
		addlinetomenu(ent, "already been set and can't", 0);
		addlinetomenu(ent, "be changed!", 0);
		addlinetomenu(ent, " ", 0);
		addlinetomenu(ent, " ", 0);
		addlinetomenu(ent, " ", 0);
	}
	else
	{
		// open prompt for password
		stuffcmd(ent, "messagemode\n");

		addlinetomenu(ent, "Please enter a master", 0);
		addlinetomenu(ent, "password above. If you", 0);
		addlinetomenu(ent, "forget your normal", 0);
		addlinetomenu(ent, "password, you can use", 0);
		addlinetomenu(ent, "this one to recover your", 0);
		addlinetomenu(ent, "character.", 0);
	}

	addlinetomenu(ent, " ", 0);
	addlinetomenu(ent, "Close", 1);

	setmenuhandler(ent, masterpw_handler);
	ent->client->menustorage.currentline = 10;
	ent->client->menustorage.menu_index = MENU_MASTER_PASSWORD;
	showmenu(ent);
}

void talentmenu_handler (edict_t *ent, int option)
{
	if (option == 1)
	{
	    OpenTalentUpgradeMenu(ent, 0);
	}
	else if (option == 2)
		OpenGeneralTalentMenu(ent, 0);//OpenGeneralUpgradeMenu(ent, 0, 0);
	else closemenu(ent);
}

void ChooseTalentUpgradeMenu(edict_t* ent)
{
	if (!ShowMenu(ent))
		return;
	clearmenu(ent);
	//					xxxxxxxxxxxxxxxxxxxxxxxxxxx (max length 27 chars)
	addlinetomenu(ent, "Talents Menu", MENU_GREEN_CENTERED);
	addlinetomenu(ent, " ", 0);
	addlinetomenu(ent, "Select a talent you want", 0);
	addlinetomenu(ent, "to upgrade. You get", 0);
	addlinetomenu(ent, "a talent point every", 0);
	addlinetomenu(ent, "two levels. Talents", 0);
	addlinetomenu(ent, "increase your skills.", 0);
	addlinetomenu(ent, " ", 0);
	addlinetomenu(ent, va("You have %d talent points.", ent->myskills.talents.talentPoints), 0);
	addlinetomenu(ent, " ", 0);
	addlinetomenu(ent, "Class talents", 1);
	addlinetomenu(ent, "General talents", 2);
	addlinetomenu(ent, "Exit", 3);
	setmenuhandler(ent, talentmenu_handler);
	ent->client->menustorage.currentline = 11;
	showmenu(ent);
}

void SwitchCharacterMenu (edict_t *ent)
{
	if (!ShowMenu(ent))
        return;
	clearmenu(ent);

	//				    xxxxxxxxxxxxxxxxxxxxxxxxxxx (max length 27 chars)
	addlinetomenu(ent, " ", 0);
	addlinetomenu(ent, " ", 0);
    addlinetomenu(ent, "Enter a username in the", 0);
	addlinetomenu(ent, "above prompt and hit enter.", 0);
	addlinetomenu(ent, " ", 0);
	addlinetomenu(ent, "Abort by opening and", 0);
	addlinetomenu(ent, "closing the console and", 0);
	addlinetomenu(ent, "use the abort option below.", 0);
	addlinetomenu(ent, " ", 0);
	addlinetomenu(ent, " ", 0);
	addlinetomenu(ent, " ", 0);
	addlinetomenu(ent, " ", 0);
	addlinetomenu(ent, "Abort", 6);

	setmenuhandler(ent, joinmenu_handler);
	ent->client->menustorage.currentline = 13;
	stuffcmd(ent, "messagemode\n");
	ent->enteringNAME = true;
	showmenu(ent);
}

void generalmenu_handler (edict_t *ent, int option)
{
	switch (option)
	{
	case 1: OpenUpgradeMenu(ent); break;
	case 2: OpenWeaponUpgradeMenu(ent, 0); break;
	case 3: ChooseTalentUpgradeMenu(ent); break;
	case 4: OpenRespawnWeapMenu(ent); break;
	case 5: OpenMasterPasswordMenu(ent); break;
	case 6: OpenMyinfoMenu(ent); break;
	case 7: OpenArmoryMenu(ent); break;
	case 8: ShowInventoryMenu(ent, 0, false); break;
	case 9: ShowAllyMenu(ent); break;
	case 10: ShowTradeMenu(ent); break;
	case 11: ShowVoteModeMenu(ent); break;
	case 12: ShowHelpMenu(ent, 0); break;
	case 13: SwitchCharacterMenu(ent); break;
	case 14: StopPrompt(ent); break;
	case 15: PasswordMenu(ent);break;
	default: closemenu(ent);
	}
	//OpenTalentUpgradeMenu(ent, 0); break;
}

void OpenGeneralMenu (edict_t *ent)
{
	if (!ShowMenu(ent))
        return;
	clearmenu(ent);

		//				xxxxxxxxxxxxxxxxxxxxxxxxxxx (max length 27 chars)
	addlinetomenu(ent, "Main menu", MENU_GREEN_CENTERED);
	addlinetomenu(ent, "What would you like to do?", MENU_GREEN_CENTERED);
	addlinetomenu(ent, " ", 0);
	addlinetomenu(ent, "Upgrade abilities", 1);
	addlinetomenu(ent, "Upgrade weapons", 2);
	addlinetomenu(ent, "Upgrade talents", 3);
	addlinetomenu(ent, "Set respawn weapon", 4);
	addlinetomenu(ent, "Set master password", 5);
	addlinetomenu(ent, "Show character info", 6);
	addlinetomenu(ent, "Access the armory", 7);
	addlinetomenu(ent, "Access your items", 8);
	addlinetomenu(ent, "Form alliance", 9);
	addlinetomenu(ent, "Trade items", 10);
	addlinetomenu(ent, "Vote for map/mode", 11);
	addlinetomenu(ent, "Help", 12);
	addlinetomenu(ent, " ", 0);
	addlinetomenu(ent, "Change password", 15);
	addlinetomenu(ent, "Switch characters", 13);

	setmenuhandler(ent, generalmenu_handler);
	ent->client->menustorage.currentline = 18;
	showmenu(ent);
}

char *G_GetTruncatedIP (edict_t *player);

char *GetTeamString (edict_t *ent)
{
	if (ent->teamnum == 1)
		return "Red";
	if (ent->teamnum == 2)
		return "Blue";
	if (numAllies(ent) > 0)
		return "Allied";
	return "None";
}

char *GetStatusString (edict_t *ent)
{
	if (ent->health < 1)
		return "Dead";
	if (ent->flags & FL_CHATPROTECT)
		return "Chat Protect";
	if (ent->client->idle_frames > 300)
		return "Idle";
	return "Normal";
}
	
void OpenWhoisMenu (edict_t *ent)
{
	edict_t *player;

	if (!ShowMenu(ent))
        return;
	clearmenu(ent);

	//decino: get the whois stats of the person you're spectating
/*	if (!(*gi.argv(1))) {
    if (ent->client->chase_target && !level.intermissiontime)

    gi.dprintf("Trying to whois %s\n", ent->client->chase_target->client->pers.netname);
	player = ent->client->chase_target;
	}*/

	if ((player = FindPlayerByName(gi.argv(1))) == NULL)
	{
		gi.cprintf(ent, PRINT_HIGH, "Couldn't find that player.\n");
		return;
	}

	if (G_IsSpectator(player))
		return;

	addlinetomenu(ent, "Whois Information", MENU_GREEN_CENTERED);
	addlinetomenu(ent, "", 0);

	// print name and class
	addlinetomenu(ent, va("%s (%s)", player->client->pers.netname, 
		GetClassString(player->myskills.class_num)), MENU_GREEN_CENTERED);

	// print IP address
	if (ent->myskills.administrator)
		addlinetomenu(ent, player->client->pers.current_ip, MENU_GREEN_CENTERED);
	else
		addlinetomenu(ent, G_GetTruncatedIP(player), MENU_GREEN_CENTERED);

	addlinetomenu(ent, "", 0);

	addlinetomenu(ent, va("Admin:        %s", player->myskills.administrator?"Yes":"No"), 0);
	addlinetomenu(ent, va("Owner:        %s", player->myskills.owner), 0);
	addlinetomenu(ent, va("Status:       %s", GetStatusString(player)), 0);
	addlinetomenu(ent, va("Team:         %s", GetTeamString(player)), 0);
	addlinetomenu(ent, va("Level:        %d", player->myskills.level), 0);
	addlinetomenu(ent, va("Experience:   %d", player->myskills.experience), 0);
	addlinetomenu(ent, va("Hit Percent:  %d%c", (int)(100*((float)player->myskills.shots_hit/player->myskills.shots)), '%'), 0);
	addlinetomenu(ent, va("Frag Percent: %d%c", (int)(100*((float)player->myskills.frags/player->myskills.fragged)), '%'), 0);
	addlinetomenu(ent, va("Played Hours: %.1f", (float)player->myskills.playingtime/3600), 0);


	addlinetomenu(ent, " ", 0);
	addlinetomenu(ent, "Exit", 1);

	setmenuhandler(ent, masterpw_handler);//FIXME: is this safe?
		ent->client->menustorage.currentline = 16;
	showmenu(ent);
}
