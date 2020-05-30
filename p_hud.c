#include "g_local.h"

/*
======================================================================

INTERMISSION

======================================================================
*/

void MoveClientToIntermission (edict_t *ent)
{
	ent->client->showscores = true;
//	VectorCopy (level.intermission_origin, ent->s.origin);
	ent->client->ps.pmove.origin[0] = level.intermission_origin[0]*8;
	ent->client->ps.pmove.origin[1] = level.intermission_origin[1]*8;
	ent->client->ps.pmove.origin[2] = level.intermission_origin[2]*8;
	VectorCopy (level.intermission_angle, ent->client->ps.viewangles);
	ent->client->ps.pmove.pm_type = PM_FREEZE;
	ent->client->ps.gunindex = 0;
	ent->client->ps.blend[3] = 0;

	VectorCopy (level.intermission_origin, ent->s.origin);
	// clean up powerup info
	ent->client->quad_framenum = 0;
	ent->client->invincible_framenum = 0;
	ent->client->breather_framenum = 0;
	ent->client->enviro_framenum = 0;
	ent->client->grenade_blew_up = false;
	ent->client->grenade_time = 0;

//	RemoveAllAuras(ent);
//	RemoveAllCurses(ent);
	AuraRemove(ent, 0);
	CurseRemove(ent, 0);
	ent->client->is_poisoned = false;

	// RAFAEL
	ent->client->quadfire_framenum = 0;
	
	// RAFAEL
	//ent->client->trap_blew_up = false;
	//ent->client->trap_time = 0;

	ent->viewheight = 0;
	ent->s.modelindex = 0;
	ent->s.modelindex2 = 0;
	ent->s.modelindex3 = 0;
	ent->s.modelindex = 0;
	ent->s.effects = 0;
	ent->s.sound = 0;
	ent->solid = SOLID_NOT;

	// add the layout

	if (deathmatch->value && !(ent->svflags & SVF_MONSTER)) 
	{
		DeathmatchScoreboardMessage (ent, NULL);
		gi.unicast (ent, true);
	}

}
void SetLVChanged ( int i );
int GetLVChanged ( void );

//3.0 new intermission routine
void VortexBeginIntermission(char *nextmap)
{
	int		i;
	edict_t	*ent, *client;

	if (level.intermissiontime)
		return;		// allready activated

	level.intermissiontime = level.time;
	level.changemap = nextmap;
	level.exitintermission = 0;

	// find an intermission spot
	ent = G_Find (NULL, FOFS(classname), "info_player_intermission");
	if (!ent)
	{	// the map creator forgot to put in an intermission point...
		ent = G_Find (NULL, FOFS(classname), "info_player_start");
		if (!ent)
			ent = G_Find (NULL, FOFS(classname), "info_player_deathmatch");
	}
	else
	{	// chose one of four spots
		i = randomMT() & 3;
		while (i--)
		{
			ent = G_Find (ent, FOFS(classname), "info_player_intermission");
			if (!ent)	// wrap around the list
				ent = G_Find (ent, FOFS(classname), "info_player_intermission");
		}
	}

	VectorCopy (ent->s.origin, level.intermission_origin);
	VectorCopy (ent->s.angles, level.intermission_angle);

	// move all clients to the intermission point
	for (i=0 ; i<maxclients->value ; i++)
	{
		client = g_edicts + 1 + i;
		if (!client->inuse)
			continue;
		MoveClientToIntermission (client);
	}
}

//old intermission code. Should be obsolete, but left in just in case
void BeginIntermission (edict_t *targ)
{
	int		i;
	edict_t	*ent, *client;

	if (level.intermissiontime)
		return;		// allready activated

	gi.dprintf("WARNING: BeginIntermission() was called when we should be calling VortexBeginIntermission()!\n");

	level.intermissiontime = level.time;
	level.changemap = targ->map;

	// if on same unit, return immediately
	if (!deathmatch->value && (targ->map && targ->map[0] != '*') )
	{	// go immediately to the next level
		level.exitintermission = 1;
		return;
	}
	level.exitintermission = 0;

	// find an intermission spot
	ent = G_Find (NULL, FOFS(classname), "info_player_intermission");
	if (!ent)
	{	// the map creator forgot to put in an intermission point...
		ent = G_Find (NULL, FOFS(classname), "info_player_start");
		if (!ent)
			ent = G_Find (NULL, FOFS(classname), "info_player_deathmatch");
	}
	else
	{	// chose one of four spots
		i = randomMT() & 3;
		while (i--)
		{
			ent = G_Find (ent, FOFS(classname), "info_player_intermission");
			if (!ent)	// wrap around the list
				ent = G_Find (ent, FOFS(classname), "info_player_intermission");
		}
	}

	VectorCopy (ent->s.origin, level.intermission_origin);
	VectorCopy (ent->s.angles, level.intermission_angle);

	// move all clients to the intermission point
	for (i=0 ; i<maxclients->value ; i++)
	{
		client = g_edicts + 1 + i;
		if (!client->inuse)
			continue;
		MoveClientToIntermission (client);
	}
}

int V_HighestFragScore (void)
{
	int i, highScore=0;
	edict_t *cl_ent;

	for (i=0 ; i<game.maxclients ; i++)
	{
		cl_ent = g_edicts + 1 + i;
		if (!cl_ent->inuse || G_IsSpectator(cl_ent))
			continue;
		if (!highScore || (cl_ent->client->resp.frags > highScore))
			highScore = cl_ent->client->resp.frags;
	}
	return highScore;
}


/* 
================== 
DeathmatchScoreboardMessage *Improved!* 
================== 
*/ 

#define MAX_ENTRY_SIZE		1024
#define MAX_STRING_SIZE		1400

void DeathmatchScoreboardMessage (edict_t *ent, edict_t *killer) 
{ 

     char entry[MAX_ENTRY_SIZE]; 
     char string[MAX_STRING_SIZE];
	 char *name, classname[20];//3.78
	 char newstring[512];
     int stringlength; 
     int i, j, k; 
     int sorted[MAX_CLIENTS];
     int sortedscores[MAX_CLIENTS]; 
     int score, total, highscore=0; 
     int y;
	 //float accuracy;
	 int time_left=999, frag_left=999;
     gclient_t *cl; 
     edict_t *cl_ent; 

	 if (ent->fail_name)
		 return;

	 // if we are looking at the scoreboard or inventory, deactivate the scanner
	 if (ent->client->showscores || ent->client->showinventory)
	 {
		if (ent->client->pers.scanner_active)
			ent->client->pers.scanner_active = 2;
	 }
	 else
	 {
		 *string = 0;

		 // Scanner active ?
		if (ent->client->pers.scanner_active & 1)
			ShowScanner(ent,string);

		// normal quake code ...	
		gi.WriteByte (svc_layout);
		gi.WriteString (string);
		return;
	 }
	
     // sort the clients by score 
     total = 0; 
     for (i=0 ; i<game.maxclients ; i++)
	{
		cl_ent = g_edicts + 1 + i;

        //3.0 scoreboard code fix
		if (!cl_ent->client || !cl_ent->inuse || cl_ent->client->resp.spectator)
			continue;

		score = game.clients[i].resp.score;
		for (j=0 ; j<total ; j++)
		{
			if (score > sortedscores[j])
				break;
		}
		for (k=total ; k>j ; k--)
		{
			sorted[k] = sorted[k-1];
			sortedscores[k] = sortedscores[k-1];
		}
		sorted[j] = i;
		sortedscores[j] = score;
		total++;
	}
	// print level name and exit rules
	string[0] = 0;
	stringlength = strlen(string);

     // make a header for the data 
	//K03 Begin
	if (timelimit->value)
		time_left = (timelimit->value*60 - level.time);
	else
		time_left = 60*99;
	if (fraglimit->value)
		frag_left = (fraglimit->value - V_HighestFragScore());

	if (time_left < 0)
		time_left = 0;

	Com_sprintf(entry, sizeof(entry), 
		"xv 0 yv 16 string2 \"Time:%2im %2is Frags:%3i Players:%3i\" "
		"xv 0 yv 24 string2 \"Name       Lv Cl Score Frg Spr Png\" ",
		(int)(time_left/60), (int)(time_left-(int)((time_left/60)*60)),
		frag_left, total_players()); 

	 //K03 End
     j = strlen(entry); 

	 //gi.dprintf("header string length=%d\n", j);

     if (stringlength + j < MAX_ENTRY_SIZE) 
     { 
          strcpy (string + stringlength, entry); 
          stringlength += j; 
     } 

     // add the clients in sorted order 
     if (total > 24) 
          total = 24; 
     /* The screen is only so big :( */ 
	 
     for (i=0 ; i<total ; i++)
	 {
          cl = &game.clients[sorted[i]]; 
          cl_ent = g_edicts + 1 + sorted[i];

		  if (!cl_ent)
			  continue; 

          y = 34 + 8 * i; 
		
		// 3.78 truncate client's name and class string
		name = strdup(cl->pers.netname);
		V_TruncateString(name, 11, (char *)newstring);
		V_TruncateString(GetClassString(cl_ent->myskills.class_num), 2, (char *)newstring);
		strncpy((char *)classname, (char *)newstring, 3);
		padRight(name, 10);

		Com_sprintf(entry, sizeof(entry),
			"xv 0 yv %i string \"%s %2i %s %5i %3i %3i %3i\" ",
			y, name, cl_ent->myskills.level, classname, cl->resp.score, 
			cl->resp.frags, cl_ent->myskills.streak, cl->ping); 

		j = strlen(entry);

		//gi.dprintf("player string length=%d\n", j);

		if (stringlength + j > MAX_ENTRY_SIZE)
			break; 

		strcpy (string + stringlength, entry); 
		stringlength += j; 
     } 

     gi.WriteByte (svc_layout); 
     gi.WriteString (string); 

} 
//K03 End


/*
==================
DeathmatchScoreboard

Draw instead of help message.
Note that it isn't that hard to overflow the 1400 byte message limit!
==================
*/
void DeathmatchScoreboard (edict_t *ent)
{
//GHz START
	if (ent->client->menustorage.menu_active)
		closemenu(ent);
//GHz END
	DeathmatchScoreboardMessage (ent, ent->enemy);
	gi.unicast (ent, true);
}


/*
==================
Cmd_Score_f

Display the scoreboard
==================
*/
void Cmd_Score_f (edict_t *ent)
{
	
	ent->client->showinventory = false;
	ent->client->showhelp = false;

	if (ent->client->pers.scanner_active & 1)
		ent->client->pers.scanner_active = 0;//2;

	if (!deathmatch->value && !coop->value)
		return;

	if (ent->client->showscores)
	{
		ent->client->showscores = false;
		return;
	}
	
	ent->client->showscores = true;
	DeathmatchScoreboard (ent);
}


/*
==================
HelpComputer

Draw help computer.
==================
*/
void HelpComputer (edict_t *ent)
{
	char	string[1024];
	char	*sk;

	sk = "hard+";

	// send the layout
	Com_sprintf (string, sizeof(string),
		"xv 32 yv 8 picn help "			// background
		"xv 202 yv 12 string2 \"%s\" "		// skill
		"xv 0 yv 24 cstring2 \"%s\" "		// level name
		"xv 0 yv 54 cstring2 \"%s\" "		// help 1
		"xv 0 yv 110 cstring2 \"%s\" "		// help 2
		"xv 50 yv 164 string2 \" kills     goals    secrets\" "
		"xv 50 yv 172 string2 \"%3i/%3i     %i/%i       %i/%i\" ", 
		sk,
		level.level_name,
		game.helpmessage1,
		game.helpmessage2,
		level.killed_monsters, level.total_monsters, 
		level.found_goals, level.total_goals,
		level.found_secrets, level.total_secrets);

	gi.WriteByte (svc_layout);
	gi.WriteString (string);
	gi.unicast (ent, true);
}

//decino: since i can't add more shit to the HUD we'll do it the ugly way
void BuffIndicator (edict_t *ent)
{
	char	string[1024];

	if (ent->cocoon_time >= level.time)
	Com_sprintf (string, sizeof(string), "xv 350 yb -84 string2 \"COCOONED\" ");

	if (ent->fury_time >= level.time)
	Com_sprintf (string, sizeof(string), "xv 350 yb -84 string2 \"FURIED\" ");

	if (ent->bless_time >= level.time)
	Com_sprintf (string, sizeof(string), "xv 350 yb -84 string2 \"BLESSED\" ");

	if (ent->heal_time >= level.time)
	Com_sprintf (string, sizeof(string), "xv 350 yb -84 string2 \"HEALING\" ");	

	if (ent->deflect_time >= level.time)
	Com_sprintf (string, sizeof(string), "xv 350 yb -84 string2 \"DEFLECTING\" ");

    if (ent->fury_time <= level.time && ent->cocoon_time <= level.time && ent->bless_time <= level.time && ent->heal_time <= level.time && ent->deflect_time <= level.time)
	Com_sprintf (string, sizeof(string), "xv 350 yb -84 string \"Awaiting Buffs:\" ");

	gi.WriteByte (svc_layout);
	gi.WriteString (string);
	gi.unicast (ent, true);
}

/*
==================
Cmd_Help_f

Display the current help message
==================
*/
void Cmd_Help_f (edict_t *ent)
{
	// this is for backwards compatability
	if (deathmatch->value)
	{
		Cmd_Score_f (ent);
		return;
	}

	ent->client->showinventory = false;
	ent->client->showscores = false;

	if (ent->client->showhelp && (ent->client->resp.game_helpchanged == game.helpchanged))
	{
		ent->client->showhelp = false;
		return;
	}

	ent->client->showhelp = true;
	ent->client->resp.helpchanged = 0;
	HelpComputer (ent);
}

//=======================================================================

/*
===============
G_SetStats
===============
*/
void Weapon_Sword (edict_t *ent);
void G_SetStats (edict_t *ent)
{
	gitem_t		*item;
//	int			num;
	int			index, cells;
	int			power_armor_type = 0;
	trace_t		tr;
	vec3_t		forward, right, start, end, offset;
	edict_t		*scan=NULL;
//	int			time_left;//K03

	//
	// health
	//

    if (ent->fail_name)
		return;

	ent->client->ps.stats[STAT_HEALTH_ICON] = level.pic_health;

	ent->client->ps.stats[STAT_HEALTH] = ent->health;

//GHz START
	// 3.5 show percent ability is charged
	if (ent->client->charge_index)
		ent->client->ps.stats[STAT_CHARGE_LEVEL] = ent->myskills.abilities[ent->client->charge_index-1].charge;
	else
		ent->client->ps.stats[STAT_CHARGE_LEVEL] = 0;

	if (G_EntExists(ent->supplystation) && (ent->supplystation->wait >= level.time))
	{
		ent->client->ps.stats[STAT_STATION_ICON] = gi.imageindex("i_tele");
		ent->client->ps.stats[STAT_STATION_TIME] = ent->supplystation->wait-level.time;
	}
	else
	{
		ent->client->ps.stats[STAT_STATION_ICON] = 0;
		ent->client->ps.stats[STAT_STATION_TIME] = 0;
	}

	if (ptr->value || domination->value)
	{
		//if (G_EntExists(ent))
		if (ent->inuse && ent->teamnum)
		{
			index = 0;
			if (ent->teamnum == 1)
			{
				if (DEFENSE_TEAM == ent->teamnum)
					index =  gi.imageindex("i_ctf1"); // show team color pic
				else
					index = gi.imageindex("i_ctf1d"); // not in control
			}
			else if (ent->teamnum == 2)
			{
				if (DEFENSE_TEAM == ent->teamnum)
					index = gi.imageindex("i_ctf2");
				else
					index = gi.imageindex("i_ctf2d");
			}
			ent->client->ps.stats[STAT_TEAM_ICON] = index;
		}
		else
		{
			ent->client->ps.stats[STAT_TEAM_ICON] = 0;
		}

	}

	if (ctf->value)
	{
		if (ent->inuse && ent->teamnum)
		{
			edict_t *base;
			index = 0;

			if ((base = CTF_GetFlagBaseEnt(ent->teamnum)) != NULL)
			{
				if (ent->teamnum == RED_TEAM)
				{
					if (base->count == BASE_FLAG_SECURE)
						index =  gi.imageindex("i_ctf1"); // show team color pic
					else
					{
						if (level.framenum&8)
							index = gi.imageindex("i_ctf1d"); // flag taken
						else
							index = gi.imageindex("i_ctf1");
					}
				}
				else if (ent->teamnum == BLUE_TEAM)
				{
					if (base->count == BASE_FLAG_SECURE)
						index = gi.imageindex("i_ctf2");
					else
					{
						if (level.framenum&8)
							index = gi.imageindex("i_ctf2d");
						else
							index = gi.imageindex("i_ctf2");
					}
				}
			}
			ent->client->ps.stats[STAT_TEAM_ICON] = index;
		}
		else
		{
			ent->client->ps.stats[STAT_TEAM_ICON] = 0;
		}
	}
//GHz END

	// player boss
	if (G_EntExists(ent->owner))
		ent->client->ps.stats[STAT_HEALTH] = ent->owner->health;

	//
	// ammo
	//
	if (!ent->client->ammo_index /* || !ent->client->pers.inventory[ent->client->ammo_index] */)
	{
		ent->client->ps.stats[STAT_AMMO_ICON] = 0;
		ent->client->ps.stats[STAT_AMMO] = 0;
	}
	else
	{
		item = &itemlist[ent->client->ammo_index];
		ent->client->ps.stats[STAT_AMMO_ICON] = gi.imageindex (item->icon);
		ent->client->ps.stats[STAT_AMMO] = ent->client->pers.inventory[ent->client->ammo_index];
	}
//GHz START
	// blaster weapon ammo
	if (ent->client->pers.weapon == Fdi_BLASTER)
	{
		ent->client->ps.stats[STAT_AMMO_ICON] = gi.imageindex("a_cells");
		ent->client->ps.stats[STAT_AMMO] = ent->monsterinfo.lefty;
	}
//GHz END

	//
	// armor
	//

	power_armor_type = PowerArmorType (ent);

	if (power_armor_type)
	{
		cells = ent->client->pers.inventory[ITEM_INDEX(Fdi_CELLS)];

//		if (cells == 0)
//		{	// ran out of cells for power armor
//			ent->flags &= ~FL_POWER_ARMOR;
//			gi.sound(ent, CHAN_ITEM, gi.soundindex("misc/power2.wav"), 1, ATTN_NORM, 0);
//			power_armor_type = 0;
//		}
	}

	index = ArmorIndex (ent);
	if (power_armor_type && (!index || ent->mtype || (level.framenum & 8) ) )//4.2 morphed players only have powered armor
	{	// flash between power armor and other armor icon
		ent->client->ps.stats[STAT_ARMOR_ICON] = gi.imageindex ("i_powershield");
		ent->client->ps.stats[STAT_ARMOR] = cells;
	}
	else if (index && !ent->mtype)
	{
		item = GetItemByIndex (index);
		ent->client->ps.stats[STAT_ARMOR_ICON] = gi.imageindex (item->icon);
		ent->client->ps.stats[STAT_ARMOR] = ent->client->pers.inventory[index];
	}
	else
	{
		ent->client->ps.stats[STAT_ARMOR_ICON] = 0;
		ent->client->ps.stats[STAT_ARMOR] = 0;
	}

	// morphed players
	if (ent->mtype)
	{
		if ((ent->mtype == MORPH_MEDIC) && (ent->client->weapon_mode == 0 || ent->client->weapon_mode == 2))
		{
			ent->client->ps.stats[STAT_AMMO_ICON] = gi.imageindex("a_cells");
			ent->client->ps.stats[STAT_AMMO] = ent->myskills.abilities[MEDIC].ammo;
		}
		else if (ent->mtype == MORPH_FLYER)
		{
			ent->client->ps.stats[STAT_AMMO_ICON] = gi.imageindex("a_cells");
			ent->client->ps.stats[STAT_AMMO] = ent->myskills.abilities[FLYER].ammo;
		}
		else if (ent->mtype == MORPH_CACODEMON)
		{
			ent->client->ps.stats[STAT_AMMO_ICON] = gi.imageindex("a_rockets");
			ent->client->ps.stats[STAT_AMMO] = ent->myskills.abilities[CACODEMON].ammo;
		}
		else
		{
			ent->client->ps.stats[STAT_AMMO_ICON] = 0;
			ent->client->ps.stats[STAT_AMMO] = 0;
		}

	}
	
	// player-monsters
	if (PM_PlayerHasMonster(ent))
	{
		if (ent->owner->mtype == P_TANK)
		{
			if (ent->client->weapon_mode == 0)
			{
				ent->client->ps.stats[STAT_AMMO_ICON] = gi.imageindex("a_rockets");
				ent->client->ps.stats[STAT_AMMO] = ent->owner->monsterinfo.jumpup;
			}
			else if (ent->client->weapon_mode == 2)
			{
				ent->client->ps.stats[STAT_AMMO_ICON] = gi.imageindex("a_bullets");
				ent->client->ps.stats[STAT_AMMO] = ent->owner->monsterinfo.lefty;
			}
			else if (ent->client->weapon_mode == 3)
			{
				ent->client->ps.stats[STAT_AMMO_ICON] = gi.imageindex("a_cells");
				ent->client->ps.stats[STAT_AMMO] = ent->owner->monsterinfo.radius;
			}
			else
			{
				ent->client->ps.stats[STAT_AMMO_ICON] = 0;
				ent->client->ps.stats[STAT_AMMO] = 0;
			}
			
			if (power_armor_type)
			{
				ent->client->ps.stats[STAT_ARMOR_ICON] = gi.imageindex ("i_powershield");
				ent->client->ps.stats[STAT_ARMOR] = cells;
			}
			else
			{
				ent->client->ps.stats[STAT_ARMOR_ICON] = 0;
				ent->client->ps.stats[STAT_ARMOR] = 0;
			}
			
		}
		else
		{
			ent->client->ps.stats[STAT_AMMO_ICON] = 0;
			ent->client->ps.stats[STAT_AMMO] = 0;
		}
	}

	//
	// pickup message
	//
	if (level.time > ent->client->pickup_msg_time)
	{
		ent->client->ps.stats[STAT_PICKUP_ICON] = 0;
		ent->client->ps.stats[STAT_PICKUP_STRING] = 0;
	}

	//
	// timers
	//
	if (ent->client->quad_framenum > level.framenum)
	{
		ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex ("p_quad");
		ent->client->ps.stats[STAT_TIMER] = (ent->client->quad_framenum - level.framenum)/10;
	}
	// RAFAEL
	else if (ent->client->quadfire_framenum > level.framenum)
	{
		// note to self
		// need to change imageindex
		ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex ("p_quadfire");
		ent->client->ps.stats[STAT_TIMER] = (ent->client->quadfire_framenum - level.framenum)/10;
	}
	else if (ent->client->invincible_framenum > level.framenum)
	{
		ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex ("p_invulnerability");
		ent->client->ps.stats[STAT_TIMER] = (ent->client->invincible_framenum - level.framenum)/10;
	}
	else if (ent->client->enviro_framenum > level.framenum)
	{
		ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex ("p_envirosuit");
		ent->client->ps.stats[STAT_TIMER] = (ent->client->enviro_framenum - level.framenum)/10;
	}
	else if (ent->client->breather_framenum > level.framenum)
	{
		ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex ("p_rebreather");
		ent->client->ps.stats[STAT_TIMER] = (ent->client->breather_framenum - level.framenum)/10;
	}
	//K03 Begin
	else if (ent->client->thrusting == 1 ||
		ent->client->cloakable ||
		ent->client->hook_state)
	{
		ent->client->ps.stats[STAT_TIMER_ICON] = gi.imageindex ("k_powercube");
		ent->client->ps.stats[STAT_TIMER] = ent->client->pers.inventory[power_cube_index];
	}
	//K03 End
	else
	{
		ent->client->ps.stats[STAT_TIMER_ICON] = 0;
		ent->client->ps.stats[STAT_TIMER] = 0;
	}

	//
	// selected item
	//
	if (ent->client->pers.selected_item == -1)
		ent->client->ps.stats[STAT_SELECTED_ICON] = 0;
	else
		ent->client->ps.stats[STAT_SELECTED_ICON] = gi.imageindex (itemlist[ent->client->pers.selected_item].icon);

	ent->client->ps.stats[STAT_SELECTED_ITEM] = ent->client->pers.selected_item;

	//
	// layouts
	//
	ent->client->ps.stats[STAT_LAYOUTS] = 0;

	if (deathmatch->value)
	{
		if (ent->client->pers.health <= 0 || level.intermissiontime
			|| ent->client->showscores || ent->client->pers.scanner_active)
			ent->client->ps.stats[STAT_LAYOUTS] |= 1;
		if (ent->client->showinventory && ent->client->pers.health > 0)
			ent->client->ps.stats[STAT_LAYOUTS] |= 2;
	}
	else
	{
		if (ent->client->showscores || ent->client->showhelp)
			ent->client->ps.stats[STAT_LAYOUTS] |= 1;
		if (ent->client->showinventory && ent->client->pers.health > 0)
			ent->client->ps.stats[STAT_LAYOUTS] |= 2;
	}

	//
	// frags
	//
	ent->client->ps.stats[STAT_FRAGS] = ent->client->resp.score;

	//
	// help icon / current weapon if not shown
	//
	if (!ent->fail_name){
	if (ent->client->resp.helpchanged && (level.framenum&8) )
		ent->client->ps.stats[STAT_HELPICON] = gi.imageindex ("i_help");
	else if ( (ent->client->pers.hand == CENTER_HANDED || ent->client->ps.fov > 91)
		&& ent->client->pers.weapon)
		ent->client->ps.stats[STAT_HELPICON] = gi.imageindex (ent->client->pers.weapon->icon);
	else
		ent->client->ps.stats[STAT_HELPICON] = 0;}

//ponpoko
	/*K03 if(ent->client->zc.aiming == 1)
	{
		ent->client->ps.stats[STAT_SIGHT_PIC] = gi.imageindex ("zsight");
	}
	else if(ent->client->zc.aiming == 3)
	{
		if(ent->client->zc.lockon) ent->client->ps.stats[STAT_SIGHT_PIC] = gi.imageindex ("zsight_l1");
		else ent->client->ps.stats[STAT_SIGHT_PIC] = gi.imageindex ("zsight_l0");
	}
	else ent->client->ps.stats[STAT_SIGHT_PIC] = 0;*/
//ponpoko

	//K03 Begin
	//ent->client->ps.stats[STAT_LEVEL] = ent->myskills.level;
	ent->client->ps.stats[STAT_STREAK] = ent->myskills.streak;

	/*if (timelimit->value)
		time_left = (timelimit->value*60 - level.time);
	else
		time_left = 60*99;

	ent->client->ps.stats[STAT_TIMEMIN] = (int)(time_left/60);*/
	ent->client->ps.stats[STAT_TIMEMIN] = ent->client->pers.inventory[power_cube_index];
	if (ent->client->ps.stats[STAT_TIMEMIN] < 0)
		ent->client->ps.stats[STAT_TIMEMIN] = 0;

	if (ent->totem1 || ent->totem2)
	ent->client->ps.stats[STAT_ACTIVE_TOTEM] = gi.imageindex ("p_megahealth");
	else
	ent->client->ps.stats[STAT_ACTIVE_TOTEM] = 0;

	//decino: buff indicator timer, only 1 at a time
	if (ent->client->showbuffs){// && (ent->fury_time >= level.time || ent->cocoon_time >= level.time || ent->bless_time >= level.time || ent->heal_time >= level.time || ent->deflect_time >= level.time)){
	if (ent->cocoon_time >= level.time)
	ent->client->ps.stats[STAT_BUFF_TIMER] = 0.9+ent->cocoon_time - level.time;

	if (ent->fury_time >= level.time)
	ent->client->ps.stats[STAT_BUFF_TIMER] = 0.9+ent->fury_time - level.time;

	if (ent->bless_time >= level.time)
	ent->client->ps.stats[STAT_BUFF_TIMER] = 0.9+ent->bless_time - level.time;

	if (ent->heal_time >= level.time)
	ent->client->ps.stats[STAT_BUFF_TIMER] = 0.9+ent->heal_time - level.time;

	if (ent->deflect_time >= level.time)
	ent->client->ps.stats[STAT_BUFF_TIMER] = 0.9+ent->deflect_time - level.time;}

	if (!ent->client->showbuffs)
	ent->client->ps.stats[STAT_BUFF_TIMER] = 0;
	//decino: indicator end

	if (ent->client->forcewall_health > 0)
	ent->client->ps.stats[STAT_ID_HEALTH] = ent->client->forcewall_health;
	else
	ent->client->ps.stats[STAT_ID_HEALTH] = 0;

	if (ent->client->pers.selected_item == -1)
		ent->client->ps.stats[STAT_SELECTED_NUM] = 0;
	else
		ent->client->ps.stats[STAT_SELECTED_NUM] = ent->client->pers.inventory[ent->client->pers.selected_item];
	//K03 End

	//GHz Begin

	// id code
	if  (/*(ent->lockon) ||*/ ((!ent->myskills.abilities[ID].disable) && (ent->myskills.abilities[ID].current_level >= 0)))
	{
		// find entity near crosshair
		AngleVectors (ent->client->v_angle, forward, right, NULL);
		VectorSet(offset, 0, 7,  ent->viewheight-8);
		P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
		VectorMA(start, 8192, forward, end);
		tr = gi.trace(start, NULL, NULL, end, ent, MASK_SHOT);
		if (G_EntExists(tr.ent) && (tr.ent->health>0))
		{
			if (tr.ent->mtype == M_MIRROR)
				ent->client->idtarget = tr.ent->owner;
			else
				ent->client->idtarget = tr.ent;
			//ent->enemy = tr.ent;
		}
		
		if (G_EntExists(ent->client->idtarget) && (ent->client->idtarget->health>0) 
			&& visible(ent, ent->client->idtarget))
		{
			edict_t *e = ent->client->idtarget;
			float	dist = entdist(ent, e);

			// set a configstring
			//FIXME: we should truncate this string if it is longer than MAX_QPATH (maximum configstring length)
			if (e->client)
				gi.configstring (CS_GENERAL+(ent-g_edicts-1), va("%s (%d) @ %.0f: %dh/%da/%d", 
					e->client->pers.netname, e->myskills.level, dist, e->health, e->client->pers.inventory[body_armor_index], e->client->pers.inventory[e->client->ammo_index]));
			else if (PM_MonsterHasPilot(e))
				gi.configstring (CS_GENERAL+(ent-g_edicts-1), va("%s (%d) @ %.0f: %dh", e->owner->client->pers.netname, e->owner->myskills.level, dist, e->health));
			else if (e->mtype == M_SENTRY || e->mtype == M_MINISENTRY)
				gi.configstring (CS_GENERAL+(ent-g_edicts-1), va("%s (%d) @ %.0f: %dh/%da/%d", 
					V_GetMonsterName(e->mtype), e->monsterinfo.level, dist, e->health, e->monsterinfo.power_armor_power, e->light_level));
			else if (e->mtype)
				gi.configstring (CS_GENERAL+(ent-g_edicts-1), va("%s (%d) @ %.0f: %dh/%da", 
					V_GetMonsterName(e->mtype), e->monsterinfo.level, dist, e->health, e->monsterinfo.power_armor_power));
			else
				gi.configstring (CS_GENERAL+(ent-g_edicts-1), va("%s @ %.0f", e->classname, dist));

			// set stat to the configstring's index
			ent->client->ps.stats[STAT_CHASE] = CS_GENERAL+(ent-g_edicts-1);
			/*
			// id health
			if (ent->client->idtarget->health)
				ent->client->ps.stats[STAT_ID_HEALTH] = ent->client->idtarget->health;
			else
				ent->client->ps.stats[STAT_ID_HEALTH] = 0;
			// id armor
			if (ent->client->idtarget->client)
				num = ent->client->idtarget->client->pers.inventory[body_armor_index];
			else
				num = ent->client->idtarget->monsterinfo.power_armor_power;
			if (num)
				ent->client->ps.stats[STAT_ID_ARMOR] = num;
			else
				ent->client->ps.stats[STAT_ID_ARMOR] = 0;
			// id ammo
			if (ent->client->idtarget->client)
				num = ent->client->idtarget->client->pers.inventory[ent->client->ammo_index];
			else if (ent->client->idtarget->creator) // for sentries
				num = ent->client->idtarget->light_level;
			else
				num = 0;
			if (num)
				ent->client->ps.stats[STAT_ID_AMMO] = num;
			else
				ent->client->ps.stats[STAT_ID_AMMO] = 0;
			*/
		}
		else
		{
			ent->client->idtarget = NULL;
//			ent->client->ps.stats[STAT_ID_HEALTH] = 0;
//			ent->client->ps.stats[STAT_ID_ARMOR] = 0;
//			ent->client->ps.stats[STAT_ID_AMMO] = 0;
			ent->client->ps.stats[STAT_CHASE] = 0;
		}
	}
	else
	{
//		ent->client->ps.stats[STAT_ID_HEALTH] = 0;
//		ent->client->ps.stats[STAT_ID_ARMOR] = 0;
//		ent->client->ps.stats[STAT_ID_AMMO] = 0;
		ent->client->ps.stats[STAT_CHASE] = 0;
	}
	//GHz End
//GHz START
	if ((ent->myskills.abilities[ID].disable) || (!ent->myskills.abilities[ID].current_level) || (level.time > ent->lastdmg+2))
		ent->client->ps.stats[STAT_ID_DAMAGE] = 0;
//GHz END
}


/*
===============
G_CheckChaseStats
===============
*/
void G_CheckChaseStats (edict_t *ent)
{
	int i;
	gclient_t *cl;

	if (ent->fail_name)
		return;

	//GHz START
	// set stats for non-client targets
	if (ent->client->chase_target && !ent->client->chase_target->client)
	{
		// player-monster chase stats
		if (PM_MonsterHasPilot(ent->client->chase_target))
		{
			// use stats of the monster's owner
			memcpy(ent->client->ps.stats, ent->client->chase_target->owner->client->ps.stats, 
				sizeof(ent->client->ps.stats));
			G_SetSpectatorStats(ent);

			// set a configstring to the player's name
			gi.configstring (CS_GENERAL+(ent-g_edicts-1), 
				va("Chasing %s", ent->client->chase_target->owner->client->pers.netname));
			// set stat to the configstring's index
			ent->client->ps.stats[STAT_CHASE] = CS_GENERAL+(ent-g_edicts-1);
		}
		else
		{
			ent->client->ps.stats[STAT_HEALTH] = ent->client->chase_target->health;
			if (ent->client->chase_target->monsterinfo.power_armor_power)
			{
				ent->client->ps.stats[STAT_ARMOR_ICON] = gi.imageindex("i_powershield");
				ent->client->ps.stats[STAT_ARMOR] = ent->client->chase_target->monsterinfo.power_armor_power;
			}
			else
			{
				ent->client->ps.stats[STAT_ARMOR_ICON] = 0;
				ent->client->ps.stats[STAT_ARMOR] = 0;
			}
			// monsters don't have ammo
			ent->client->ps.stats[STAT_AMMO_ICON] = 0;
			ent->client->ps.stats[STAT_AMMO] = 0;
			// set a configstring to the monster's classname
			if (ent->client->chase_target->mtype)
				gi.configstring (CS_GENERAL+(ent-g_edicts-1), va("Chasing %s (%d)", 
					V_GetMonsterName(ent->client->chase_target->mtype), ent->client->chase_target->monsterinfo.level));
			else
				gi.configstring (CS_GENERAL+(ent-g_edicts-1), va("Chasing %s", ent->client->chase_target->classname));
			// set stat to the configstring's index
			ent->client->ps.stats[STAT_CHASE] = CS_GENERAL+(ent-g_edicts-1);
		}
		return; // since this player has a chase target, it must be a spectator which can't be chased!
	}
	//GHz END
	for (i = 1; i <= maxclients->value; i++) {
		cl = g_edicts[i].client;
		if (!g_edicts[i].inuse || cl->chase_target != ent)
			continue;
		if (ent->client->pers.weapon)
			ent->client->ps.gunindex = gi.modelindex(ent->client->pers.weapon->view_model);
		memcpy(cl->ps.stats, ent->client->ps.stats, sizeof(cl->ps.stats));
		G_SetSpectatorStats(g_edicts + i);
	}
}

/*
===============
G_SetSpectatorStats
===============
*/
void G_SetSpectatorStats (edict_t *ent)
{
	gclient_t *cl = ent->client;

	if (!cl->chase_target)
		G_SetStats (ent);

	if (ent->fail_name)
		return;

	cl->ps.stats[STAT_SPECTATOR] = 1;

	// layouts are independant in spectator
	cl->ps.stats[STAT_LAYOUTS] = 0;
	if (cl->pers.health <= 0 || level.intermissiontime || cl->showscores)
		cl->ps.stats[STAT_LAYOUTS] |= 1;
	if (cl->showinventory && cl->pers.health > 0)
		cl->ps.stats[STAT_LAYOUTS] |= 2;

	if (cl->chase_target && cl->chase_target->inuse && cl->chase_target->client)
	{
		// set a configstring to the player's name
		gi.configstring (CS_GENERAL+(ent-g_edicts-1), 
			va("Chasing %s (%d)", cl->chase_target->client->pers.netname, cl->chase_target->myskills.level));
		// set stat to the configstring's index
		cl->ps.stats[STAT_CHASE] = CS_GENERAL+(ent-g_edicts-1);

		//cl->ps.stats[STAT_CHASE] = CS_PLAYERSKINS + 
		//	(cl->chase_target - g_edicts) - 1;
	}
	else
		cl->ps.stats[STAT_CHASE] = 0;
}

