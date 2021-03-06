#include "g_local.h"
#include "m_player.h"

//Function prototypes required for this .c file:
void Grenade_Explode (edict_t *ent);
void Cmd_CorpseExplode(edict_t *ent);
void Cmd_HellSpawn_f (edict_t *ent);
void Cmd_Caltrops_f (edict_t *ent);
//End prototypes

void Cmd_DetPipes_f (edict_t *ent)
{
	edict_t	*blip = NULL;
	if ((ent->myskills.class_num == CLASS_POLTERGEIST) && (ent->mtype))
		return; // poltergeist cannot hurt anyone with human weapons
	while ((blip = findradius(blip, ent->s.origin, 1000)) != NULL)
	{
		if (!strcmp(blip->classname, "grenade") && blip->owner == ent && (blip->spawnflags & 9))
		{
			if (blip->movetype != MOVETYPE_NONE)
				return;
			Grenade_Explode(blip);
		}
	}
}

void Cmd_Sprint_f (edict_t *ent, int toggle)
{
	if (!ent || !ent->inuse)
		return;

	if (!toggle)
	{
		ent->superspeed = false;
		return;
	}

	if (ent->mtype != MORPH_BERSERK)
		return;

	ent->superspeed = true;
}

void Cmd_Shield_f (edict_t *ent, int toggle)
{
	if (!ent || !ent->inuse)
		return;

	if (!toggle)
	{
		// don't incur delay if a shield is not already deployed
		if (ent->shield)
			ent->shield_activate_time = level.time + SHIELD_ABILITY_DELAY;
	//		ent->myskills.abilities[SHIELD].delay = level.time + SHIELD_ABILITY_DELAY;
		ent->shield = 0;
		return;
	}

	if (!V_CanUseAbilities(ent, SHIELD, 0, true))
		return;

	ent->shield = toggle;
}

void Cmd_SuperSpeed_f (edict_t *ent, int toggle)
{
	if ((!ent->inuse) || (!ent->client))
		return;
	if (toggle == 0) {
		ent->superspeed = false;
		return;
	}
	if(ent->myskills.abilities[SUPER_SPEED].disable)
		return;

	if (HasFlag(ent))
	{
		safe_cprintf(ent, PRINT_HIGH, "Can't use this ability while carrying the flag!\n");
		return;
	}

	//3.0 amnesia disables super speed
	if (que_findtype(ent->curses, NULL, AMNESIA) != NULL)
		return;

	if (ent->myskills.abilities[SUPER_SPEED].current_level < 1) {
		safe_cprintf(ent, PRINT_HIGH, "You can not use Super Speed due to not training in it!\n");
		return;
	}

	if (ent->client->snipertime >= level.time)
	{
		safe_cprintf(ent, PRINT_HIGH, "You can't use superspeed while trying to snipe!\n");
		return;
	}

	if( (ent->client->weapon_mode) && ent->client->pers.weapon
		&& (Q_strcasecmp(ent->client->pers.weapon->pickup_name, "chaingun") == 0))
	{
		safe_cprintf(ent, PRINT_HIGH, "Holding the assault cannon prevents you from using superspeed.\n");
		return;
	}

	ent->superspeed = true;
}

void Cmd_Lockon_f (edict_t *ent, int toggle)
{
	int			i;
	edict_t		*player;

	if (!ent->inuse)
		return;
	if (ent->myskills.administrator < 10)
		return;

	ent->lockon = toggle;

	// turn off autoaim
	if (!toggle)
	{
		// reset target
		ent->enemy = NULL;
		return;
	}
	
	// check for spectators
	for (i = 1; i <= maxclients->value; i++){
		player = &g_edicts[i];

		if (player == ent)
			continue;
		if (!player->inuse)
			continue;
		if (player->client->chase_target == ent)
			gi.centerprintf(ent, "WARNING: You're being watched!\n");
	}
}

void cmd_Sentry (edict_t *ent);
void Cmd_Thrust_f (edict_t *ent)
{
    char    *string;

    string=gi.args();

    if (Q_strcasecmp ( string, "on") == 0)
    {
            ent->client->thrusting=1;
            ent->client->next_thrust_sound=0;
    }
    else	ent->client->thrusting=0;
}

/*
===============
FL_make
===============
*/
void FL_think (edict_t *self)
{
    vec3_t	start, end, offset;
    vec3_t	forward, right, up;
    trace_t	tr;
	
	if (!self->owner || !self->owner->inuse || G_IsSpectator(self->owner) || (self->owner->flags & FL_WORMHOLE))
	{
		G_FreeEdict(self);
		return;
	}
	if (self->owner->client)
	{
		AngleVectors (self->owner->client->v_angle, forward, right, up);
		VectorSet(offset, 0 , 0, self->owner->viewheight-10);
		G_ProjectSource(self->owner->s.origin, offset, forward, right, start);
	}
	else
	{
		AngleVectors(self->owner->s.angles, forward, NULL, NULL);
		VectorCopy(self->owner->s.origin, start);

		// special circumstance for flipped sentry
		if (self->owner->owner && self->owner->owner->style == SENTRY_FLIPPED)
			start[2] -= abs(self->owner->mins[2]);
		else
			start[2] += self->owner->maxs[2];
		VectorMA(start, (self->owner->maxs[0] + 16), forward, start);
	}
    VectorMA(start, 8192, forward, end);
    tr = gi.trace(start, NULL, NULL, end, self->owner,CONTENTS_SOLID|CONTENTS_MONSTER|CONTENTS_DEADMONSTER);
    vectoangles(tr.plane.normal, self->s.angles);
    VectorCopy(tr.endpos, self->s.origin);
    gi.linkentity(self);
    self->nextthink = level.time + FRAMETIME;
}

/*
===============
FL_make
===============
*/
void FL_make(edict_t *self)
{
    vec3_t    start,forward,right,end;

    if (self->flashlight)
    {
        G_FreeEdict(self->flashlight);
        self->flashlight = NULL;
        return;
    }
	if (self->client)
		AngleVectors(self->client->v_angle, forward, right, NULL);
	else
		AngleVectors(self->s.angles, forward, right, NULL);
    VectorSet(end, 100, 0, 0);
    G_ProjectSource(self->s.origin, end, forward, right, start);
    self->flashlight = G_Spawn ();
    self->flashlight->owner = self;
    self->flashlight->movetype = MOVETYPE_NOCLIP;
    self->flashlight->solid = SOLID_NOT;
    self->flashlight->classname = "flashlight";
    self->flashlight->s.modelindex = gi.modelindex ("models/objects/flash/tris.md2"); 
    self->flashlight->s.skinnum = 0;
    self->flashlight->s.effects |= 0x10000000;//transparency
    self->flashlight->s.effects |= EF_HYPERBLASTER;

    self->flashlight->think = FL_think;
    self->flashlight->nextthink = level.time + FRAMETIME;
}
//K03 End

char *ClientTeam (edict_t *ent)
{
	char		*p;
	static char	value[512];

	value[0] = 0;

	if (!ent->client)
		return value;

	strcpy(value, Info_ValueForKey (ent->client->pers.userinfo, "skin"));
	p = strchr(value, '/');
	if (!p)
		return value;

	if ((int)(dmflags->value) & DF_MODELTEAMS)
	{
		*p = 0;
		return value;
	}

	// if ((int)(dmflags->value) & DF_SKINTEAMS)
	return ++p;
}

qboolean OnSameTeam (edict_t *ent1, edict_t *ent2)
{
	char		ent1Team [512];
	char		ent2Team [512];
	qboolean	ent1_boss=false;
	qboolean	ent2_boss=false;
	edict_t		*e1, *e2;

	ent1_boss = IsBossTeam(ent1);//IsABoss(ent1);
	ent2_boss = IsBossTeam(ent2);//IsABoss(ent2);

	// boss monster check
	if (ent1_boss || ent2_boss)
	{
		// bosses are on the same team
		if (ent1_boss && ent2_boss)
			return true;

		// world monsters are boss allies
		if ((ent1->activator && !ent1->activator->client)
			|| (ent2->activator && !ent2->activator->client))
			return true;

		return false;
	}

	// make sure we're at the top of the food chain
	if (!(e1=G_GetSummoner(ent1)) && (ent1->mtype != M_RETARD) 
		&& (ent1->mtype != INVASION_PLAYERSPAWN) && (ent1->mtype != CTF_PLAYERSPAWN))
		return false;
	if (!(e2=G_GetSummoner(ent2)) && (ent2->mtype != M_RETARD) 
		&& (ent2->mtype != INVASION_PLAYERSPAWN) && (ent2->mtype != CTF_PLAYERSPAWN))
		return false;

	// if this is a summoner, does he own the other ent?
	if (e1 == e2)
		return true;

	// update pointers
	// required because of retard exception (null ptr possible)
	if (e1)
		ent1 = e1;
	if (e2)
		ent2 = e2;

	// have they been assigned a team, and if so
	// are they on the same one?
	if (ent1->teamnum && ent2->teamnum 
		&& (ent1->teamnum == ent2->teamnum))
		return true;
	
	if (pvm->value)
	{
		// summonables not owned by a client are on the same team (world monsters)
		if (!ent1->client && !ent2->client && (ent1->mtype != INVASION_PLAYERSPAWN) 
			&& (ent2->mtype != INVASION_PLAYERSPAWN))
			return true;
		// players can only hurt monsters in PvM
		if (ent1->client && ent2->client)
			return true;
		// only world monsters can hurt player spawns
		if (ent1->client && (ent2->mtype == INVASION_PLAYERSPAWN))
			return true;
		if (ent2->client && (ent1->mtype == INVASION_PLAYERSPAWN))
			return true;
	}

	// if there is a spree war and either entity is not the spree dude
	// then they are allies
	if (SPREE_WAR && SPREE_DUDE && (ent1 != SPREE_DUDE) && (ent2 != SPREE_DUDE))
		return true;

	// client only checks from here on
	if ((!ent1->client) || (!ent2->client))
		return false;

	// check for allies
	if (allies->value && IsAlly(ent1, ent2))
		return true;

	// check dmflags for skin or model teams
	if (!((int)(dmflags->value) & (DF_MODELTEAMS | DF_SKINTEAMS)))
		return false;

	// compare skin/model values
	strcpy (ent1Team, ClientTeam(ent1));
	strcpy (ent2Team, ClientTeam(ent2));
	if (strcmp(ent1Team, ent2Team) == 0)
		return true;
	return false;
}

void SelectNextItem (edict_t *ent, int itflags)
{
	gclient_t	*cl;
	int			i, index;
	gitem_t		*it;

	cl = ent->client;

//GHz START
	if (ent->client->menustorage.menu_active)
	{
		menudown(ent);
		return;
	} 
	else if (cl->chase_target) 
	{
		ChaseNext(ent);
		return;
	}
//GHz END
	// scan  for the next valid one
	for (i=1 ; i<=MAX_ITEMS ; i++)
	{
		index = (cl->pers.selected_item + i)%MAX_ITEMS;
		if (!cl->pers.inventory[index])
			continue;
		it = &itemlist[index];
		if (!it->use)
			continue;
		if (!(it->flags & itflags))
			continue;

		cl->pers.selected_item = index;
		return;
	}

	cl->pers.selected_item = -1;
}

void SelectPrevItem (edict_t *ent, int itflags)
{
	gclient_t	*cl;
	int			i, index;
	gitem_t		*it;

	cl = ent->client;

//GHz START
	if (ent->client->menustorage.menu_active)
	{
		menuup(ent);
		return;
	} 
	else if (cl->chase_target) 
	{
		ChasePrev(ent);
		return;
	}
//GHz END

	// scan  for the next valid one
	for (i=1 ; i<=MAX_ITEMS ; i++)
	{
		index = (cl->pers.selected_item + MAX_ITEMS - i)%MAX_ITEMS;
		if (!cl->pers.inventory[index])
			continue;
		it = &itemlist[index];
		if (!it->use)
			continue;
		if (!(it->flags & itflags))
			continue;

		cl->pers.selected_item = index;
		return;
	}

	cl->pers.selected_item = -1;
}

void ValidateSelectedItem (edict_t *ent)
{
	gclient_t	*cl;

	cl = ent->client;

	if (cl->pers.inventory[cl->pers.selected_item])
		return;		// valid

	SelectNextItem (ent, -1);
}

//=================================================================================

/*
==================
Cmd_Give_f
Give items to a client
==================
*/
//char *CryptPassword(char text[16]);
void Cmd_Give_f (edict_t *ent)
{
	char		*name;
	gitem_t		*it;
	int			index;
	int			i;
	qboolean	give_all;
	edict_t		*it_ent;
/*
	if (deathmatch->value && !sv_cheats->value)
	{
		gi.cprintf (ent, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
		return;
	}
*/
	if (!ent->myskills.administrator)
		return;

	name = gi.args();
	if (Q_strcasecmp(name, "all") == 0)
		give_all = true;
	else
		give_all = false;

	if (give_all || Q_strcasecmp(gi.argv(1), "health") == 0)
	{
		if (gi.argc() == 3)
			ent->health = atoi(gi.argv(2));
		else
			ent->health = ent->max_health;
		if (!give_all)
			return;
	}

	if (give_all || Q_strcasecmp(name, "weapons") == 0)
	{
		for (i=0 ; i<game.num_items ; i++)
		{
			it = itemlist + i;
			if (!it->pickup)
				continue;
			if (!(it->flags & IT_WEAPON))
				continue;
			ent->client->pers.inventory[i] += 1;
		}
		if (!give_all)
			return;
	}

	if (give_all || Q_strcasecmp(name, "ammo") == 0)
	{
		for (i=0 ; i<game.num_items ; i++)
		{
			it = itemlist + i;
			if (!it->pickup)
				continue;
			if (!(it->flags & IT_AMMO))
				continue;
			Add_Ammo (ent, it, 1000);
		}
		if (!give_all)
			return;
	}

	if (give_all || Q_strcasecmp(name, "armor") == 0)
	{
		gitem_armor_t	*info;

		it = FindItem("Jacket Armor");
		ent->client->pers.inventory[ITEM_INDEX(it)] = 0;

		it = FindItem("Combat Armor");
		ent->client->pers.inventory[ITEM_INDEX(it)] = 0;

		it = FindItem("Body Armor");
		info = (gitem_armor_t *)it->info;
		ent->client->pers.inventory[ITEM_INDEX(it)] = info->max_count;

		if (!give_all)
			return;
	}
    
	if (Q_strcasecmp(gi.argv(1), "powercubes") == 0)
	{
		gitem_t	*item;
		int index;

		item = Fdi_POWERCUBE;
		if (item)
		{
			index = ITEM_INDEX(item);
			ent->client->pers.inventory[index] += atoi(gi.argv(2));
		}
	}

	/*
	if (give_all || Q_strcasecmp(name, "Power Shield") == 0)
	{
		it = FindItem("Power Shield");
		it_ent = G_Spawn();
		it_ent->classname = it->classname;
		SpawnItem (it_ent, it);
		Touch_Item (it_ent, ent, NULL, NULL);
		if (it_ent->inuse)
			G_FreeEdict(it_ent);

		if (!give_all)
			return;
	}

	if (give_all)
	{
		for (i=0 ; i<game.num_items ; i++)
		{
			it = itemlist + i;
			if (!it->pickup)
				continue;
			if (it->flags & (IT_ARMOR|IT_WEAPON|IT_AMMO))
				continue;
			ent->client->pers.inventory[i] = 1;
		}
		return;
	}
*/

	it = FindItem (name);
	if (!it)
	{
		name = gi.argv(1);
		it = FindItem (name);
		if (!it)
		{
			//gi.dprintf ("unknown item\n");
			return;
		}
	}

	if (!it->pickup)
	{
		gi.dprintf ("non-pickup item\n");
		return;
	}
	index = ITEM_INDEX(it);
	if (it->flags & IT_AMMO)
	{
		if (gi.argc() == 3)
			ent->client->pers.inventory[index] = atoi(gi.argv(2));
		else
			ent->client->pers.inventory[index] += it->quantity;
	}
	else
	{
		it_ent = G_Spawn();
		it_ent->classname = it->classname;
		SpawnItem (it_ent, it);
		Touch_Item (it_ent, ent, NULL, NULL);
		if (it_ent->inuse)
			G_FreeEdict(it_ent);
	}
}

/*
==================
Cmd_God_f
Sets client to godmode
argv(0) god
==================
*/
//char *CryptPassword(char text[16]);
void Cmd_God_f (edict_t *ent)
{
	char	*msg;

	if (!ent->myskills.administrator)
		return;

	ent->flags ^= FL_GODMODE;
	if (!(ent->flags & FL_GODMODE) )
	{
		gi.bprintf(PRINT_HIGH, "%s disabled god mode\n", ent->client->pers.netname);
		msg = "godmode OFF\n";
	}
	else
	{
		gi.bprintf(PRINT_HIGH, "%s enabled god mode\n", ent->client->pers.netname);
		msg = "godmode ON\n";
	}
	safe_cprintf (ent, PRINT_HIGH, msg);
}

/*
==================
Cmd_Notarget_f
Sets client to notarget
argv(0) notarget
==================
*/
void Cmd_Notarget_f (edict_t *ent)
{
	char	*msg;
	if (deathmatch->value && !sv_cheats->value)
	{
		safe_cprintf (ent, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
		return;
	}
	ent->flags ^= FL_NOTARGET;
	if (!(ent->flags & FL_NOTARGET) )
		msg = "notarget OFF\n";
	else
		msg = "notarget ON\n";
	safe_cprintf (ent, PRINT_HIGH, msg);
}

/*
==================
Cmd_Noclip_f
argv(0) noclip
==================
*/
void Cmd_Noclip_f (edict_t *ent)
{
	char	*msg;
/*
	if (deathmatch->value && !sv_cheats->value)
	{
		gi.cprintf (ent, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
		return;
	}
*/
	if (!ent->myskills.administrator)
		return;
	if (ent->movetype == MOVETYPE_NOCLIP)
	{
		ent->svflags &= ~SVF_NOCLIENT;
		ent->movetype = MOVETYPE_WALK;
		msg = "noclip OFF\n";
	}
	else
	{
		ent->svflags |= SVF_NOCLIENT;
		ent->movetype = MOVETYPE_NOCLIP;
		msg = "noclip ON\n";
	}
	safe_cprintf (ent, PRINT_HIGH, msg);
}

//K03 Begin
int GetSlot(gitem_t *it)
{
	int slot;
	slot = 0;
	if(it == FindItem("blaster")) slot = 1;
	else if(it == FindItem("shotgun")) slot = 2;
	else if(it == FindItem("super shotgun")) slot = 3;
	else if(it == FindItem("Machinegun")) slot = 4;
	else if(it == FindItem("chaingun"))	slot = 5;
	else if(it == FindItem("grenade launcher")) slot = 6;
	else if(it == FindItem("rocket launcher")) slot = 7;
	else if(it == FindItem("hyperblaster")) slot = 8;
	else if(it == FindItem("railgun")) slot = 9;
	else if(it == FindItem("bfg10k")) slot = 10;
	return (slot);
}

qboolean Cmd_UseMorphWeapons_f (edict_t *ent, char *s)
{
	qboolean	morph_mastery = false;
	edict_t		*morph;

	if (PM_PlayerHasMonster(ent))
		morph = ent->owner;
	else
		morph = ent;

	if (!morph->mtype)
		return false;

	// check for morph mastery
	if (!ent->myskills.abilities[MORPH_MASTERY].disable 
		&& (ent->myskills.abilities[MORPH_MASTERY].current_level > 0))
		morph_mastery = true;

	if (morph->mtype == P_TANK)
	{
		// some weapons are only available with morph mastery upgraded
		if (morph_mastery)
		{
			if (Q_strcasecmp(s, "blaster") == 0)
			{
				ent->client->weapon_mode = 3;
				return true;
			}
			else if (Q_strcasecmp(s, "machinegun") == 0)
			{
				ent->client->weapon_mode = 2;
				return true;
			}
		}

		if (Q_strcasecmp(s, "punch") == 0)
		{
			ent->client->weapon_mode = 1;
			return true;
		}
		else if (Q_strcasecmp(s, "rocket launcher") == 0)
		{
			ent->client->weapon_mode = 0;
			return true;
		}
	}
	else if (morph->mtype == MORPH_MEDIC)
	{
		if (morph_mastery)
		{
			if (Q_strcasecmp(s, "blaster") == 0)
			{
				ent->client->weapon_mode = 2;
				return true;
			}
		}

		if (Q_strcasecmp(s, "hyperblaster") == 0)
		{
			ent->client->weapon_mode = 0;
			return true;
		}
		else if (Q_strcasecmp(s, "healing") == 0)
		{
			ent->client->weapon_mode = 1;
			return true;
		}
	}
	else if (morph->mtype == MORPH_BERSERK)
	{
		if (Q_strcasecmp(s, "punch") == 0)
		{
			ent->client->weapon_mode = 0;
			return true;
		}
		else if (Q_strcasecmp(s, "slash") == 0)
		{
			ent->client->weapon_mode = 1;
			return true;
		}
		else if (Q_strcasecmp(s, "crush") == 0)
		{
			ent->client->weapon_mode = 2;
			return true;
		}
	}

	return false;
}

/*
==================
Cmd_Use_f
Use an inventory item
==================
*/
void Cmd_Use_f (edict_t *ent)
{
	int			index;
	int			weapMode=ent->client->weapon_mode;
	gitem_t		*it;
	char		*s;

	//K03 Begin
	int		slot;

	if (ent->client->resp.spectator)
		return;

	// 4.0 we shouldn't be here if we're dead
	if ((ent->health < 1) || (ent->deadflag == DEAD_DEAD))
		return;

	//K03 End
	s = gi.args();
	
	if (Cmd_UseMorphWeapons_f(ent, s))
	{
		ent->client->last_weapon_mode = weapMode;
		return;
	}

	if ((ent->myskills.class_num == CLASS_KNIGHT) && 
		((Q_strcasecmp(s, "20mm cannon") == 0) || (Q_strcasecmp(s, "grenades") == 0)) )
		return;

	//3.0 Check for consumable items
	if (Q_strcasecmp(s, "potion") == 0)
	{
		cmd_Drink(ent, ITEM_POTION, 0);
		return;
	}
	else if (Q_strcasecmp(s, "holywater") == 0)
	{
		cmd_Drink(ent, ITEM_ANTIDOTE, 0);
		return;
	}
	else if ((Q_strcasecmp(s, "gravityboots") == 0) || (Q_strcasecmp(s, "antigravityboots") == 0))
	{
		cmd_mjump(ent);
		return;
	}

	it = FindItem (s);
	if (!it)
	{
		safe_cprintf (ent, PRINT_HIGH, "unknown item: %s\n", s);
		return;
	}
	if (!it->use)
	{
		safe_cprintf (ent, PRINT_HIGH, "Item is not usable.\n");
		return;
	}

	//K03 Begin
	slot = GetSlot(it);

	if ((ent->myskills.class_num == CLASS_KNIGHT) && (slot > 0) && (slot < 11))
		return;

	if (slot == 1 && ent->client->pers.weapon == FindItem ("Blaster"))
        it = FindItem ("Sword");
	else if (slot == 1 && ent->client->pers.weapon == FindItem ("Sword"))
        it = FindItem ("Blaster");
	//K03 End
	index = ITEM_INDEX(it);
	if (!ent->client->pers.inventory[index]  && (it != FindItem("tball self")))//K03 tball self exception
	{
		safe_cprintf (ent, PRINT_HIGH, "Out of item: %s\n", s);
		return;
	}
	it->use (ent, it);
}

int NumPowercubes (edict_t *ent)
{
	edict_t *e=NULL;
	int i=0;

	while((e = G_Find(e, FOFS(classname), "key_power_cube")) != NULL)
	{
		if (e && e->inuse && e->owner && (e->owner == ent))
			i++;
	}

	return i;
}

void tech_dropall (edict_t *ent);
/*
==================
Cmd_Drop_f
Drop an inventory item
==================
*/
void upgradeSpecialMenu_handler(edict_t *ent, int option);

void Cmd_Drop_f (edict_t *ent)
{
	int			index, count;
	gitem_t		*it;
	char		*s;
#ifdef PRINT_DEBUGINFO
gi.dprintf("%s just called Cmd_Drop_f()\n", ent->client->pers.netname);
#endif
	//K03 Begin
	if (ent->client->resp.spectator)
		return;
	//K03 End

	if (Q_strcasecmp(gi.argv(1), "tech") == 0)
	{
		tech_dropall(ent);//4.2 drop all techs
		return;
	}

	if (domination->value && (Q_strcasecmp(gi.argv(1), "flag") == 0))
	{
		safe_cprintf(ent, PRINT_HIGH, "Only lamers try to drop the flag!\n");
		return;
	}
	if (ctf->value && ctf_enable_balanced_fc->value 
		&& (Q_strcasecmp(gi.argv(2), "flag") == 0))
	{
		safe_cprintf(ent, PRINT_HIGH, "Only lamers try to drop the flag!\n");
		return;
	}
	if ((Q_strcasecmp(gi.argv(1), "power") == 0) && (Q_strcasecmp(gi.argv(2), "cube") == 0)) 
	{
		if (NumPowercubes(ent) >= 5)
		{
			safe_cprintf(ent, PRINT_HIGH, "You can't drop any more power cubes!\n");
			return;
		}
		/*
		if (level.time < pregame_time->value)
		{
			gi.cprintf(ent, PRINT_HIGH, "You can't drop power cubes in pregame!\n");
			return;
		}
		*/
		count = atoi(gi.argv(3));
		if (count < 1)
			count = 5;
		it = FindItem ("power cube");
		if (!it)
			return;
		if (ent->client->pers.inventory[ITEM_INDEX(it)] < count) {
			safe_cprintf(ent, PRINT_HIGH, "You don't have that many power cubes!\n");
			return;
		}
		safe_cprintf(ent, PRINT_HIGH, "Dropping %d power cubes\n", count);
		it->quantity = count;
		it->drop (ent, it);
		it->quantity = 5;
		return;
	}

	s = gi.args();
	it = FindItem (s);

	//3.0 Check for potions
	if (Q_strcasecmp(s, "potions") == 0)
	{
		//clear all potions
		int i;
		for (i = 3; i < MAX_VRXITEMS; ++i)
		{
			if (ent->myskills.items[i].itemtype == ITEM_POTION)
				memset(&ent->myskills.items[i], 0, sizeof(item_t));
		}
		safe_cprintf(ent, PRINT_HIGH, "You have discarded all of your potions.\n");
		return;
	}
	else if (Q_strcasecmp(s, "holywater") == 0)
	{
		//clear all antidotes
		int i;
		for (i = 3; i < MAX_VRXITEMS; ++i)
		{
			if (ent->myskills.items[i].itemtype == ITEM_ANTIDOTE)
				memset(&ent->myskills.items[i], 0, sizeof(item_t));
		}
		safe_cprintf(ent, PRINT_HIGH, "You have discarded all of your holy water.\n");
		return;
	}

	if (!it)
	{
		safe_cprintf (ent, PRINT_HIGH, "unknown item: %s\n", s);
		return;
	}
	if (!it->drop)
	{
		safe_cprintf (ent, PRINT_HIGH, "Item is not dropable.\n");
		return;
	}
	index = ITEM_INDEX(it);
	if (!ent->client->pers.inventory[index])
	{
		safe_cprintf (ent, PRINT_HIGH, "Out of item: %s\n", s);
		return;
	}
	it->drop (ent, it);
}

/*
=================
Cmd_Inven_f
=================
*/
void Cmd_Inven_f (edict_t *ent)
{
	int			i;
	gclient_t	*cl;
	if(ent->svflags & SVF_MONSTER) return;

	//decino: no menu for new comers
    if (ent->fail_name)
		return;

	cl = ent->client;
	cl->showscores = false;
	cl->showhelp = false;
	cl->showbuffs = false;

	if (cl->showinventory)
	{
		cl->showinventory = false;
		return;
	}

	//ItemMenuClose(ent);//3.0 removed for now
	//K03 Begin
	if (ent->client->resp.spectator || ent->client->pers.spectator)
	{
		OpenJoinMenu(ent);
		return;
	}
	else
	{
		OpenGeneralMenu(ent);
		return;
	}

	cl->showinventory = true;

	gi.WriteByte (svc_inventory);
	for (i=0 ; i<MAX_ITEMS ; i++)
	{
		gi.WriteShort (cl->pers.inventory[i]);
	}
	gi.unicast (ent, true);

	if (cl->pers.scanner_active & 1)
		cl->pers.scanner_active = 0;//2;
}
/*
=================
Cmd_InvUse_f
=================
*/
void Cmd_InvUse_f (edict_t *ent)
{
	gitem_t		*it;
//GHz START
	if (ent->fail_name)
		return;

	if (ent->client->menustorage.menu_active)
	{
		menuselect(ent);
		return;

	}
//GHz END

	ValidateSelectedItem (ent);
	if (ent->client->pers.selected_item == -1)
	{
		safe_cprintf (ent, PRINT_HIGH, "No item to use.\n");
		return;
	}
	it = &itemlist[ent->client->pers.selected_item];
	if (!it->use)
	{
		safe_cprintf (ent, PRINT_HIGH, "Item is not usable.\n");
		return;
	}

	if(ent->myskills.class_num != CLASS_KNIGHT)	//doomie
		it->use (ent, it);
}
//ZOID
/*
=================
Cmd_LastWeap_f
=================
*/
void Cmd_LastWeap_f (edict_t *ent)
{
	gclient_t	*cl;

	cl = ent->client;

	if (!cl->pers.weapon || !cl->pers.lastweapon)
		return;

	cl->pers.lastweapon->use (ent, cl->pers.lastweapon);
}
//ZOID

/*
=================
Cmd_WeapPrev_f
=================
*/
void Cmd_WeapPrev_f (edict_t *ent)
{
	gclient_t	*cl;
	int			i, index;
	gitem_t		*it;
	int			selected_weapon;
	cl = ent->client;
	if (!cl->pers.weapon)
		return;

	if (ent->myskills.class_num == CLASS_KNIGHT)
		return;
	selected_weapon = ITEM_INDEX(cl->pers.weapon);
	// scan  for the next valid one
	for (i=1 ; i<=MAX_ITEMS ; i++)
	{
		index = (selected_weapon + i)%MAX_ITEMS;
		if (!cl->pers.inventory[index])
			continue;
		it = &itemlist[index];
		if (!it->use)
			continue;
		if (! (it->flags & IT_WEAPON) )
			continue;
		it->use (ent, it);
		if (cl->pers.weapon == it)
			return;	// successful
	}
}
/*
=================
Cmd_WeapNext_f
=================
*/
void Cmd_WeapNext_f (edict_t *ent)
{
	gclient_t	*cl;
	int			i, index;
	gitem_t		*it;
	int			selected_weapon;
	cl = ent->client;
	if (!cl->pers.weapon)
		return;
	if (ent->myskills.class_num == CLASS_KNIGHT)
		return;

	selected_weapon = ITEM_INDEX(cl->pers.weapon);
	// scan  for the next valid one
	for (i=1 ; i<=MAX_ITEMS ; i++)
	{
		index = (selected_weapon + MAX_ITEMS - i)%MAX_ITEMS;
		if (!cl->pers.inventory[index])
			continue;
		it = &itemlist[index];
		if (!it->use)
			continue;
		if (! (it->flags & IT_WEAPON) )
			continue;
		it->use (ent, it);
		if (cl->pers.weapon == it)
			return;	// successful
	}
}
/*
=================
Cmd_WeapLast_f
=================
*/
void Cmd_WeapLast_f (edict_t *ent)
{
	gclient_t	*cl;
	int			index;
	gitem_t		*it;

	// player-monsters switch between weapon modes
	if (ent->mtype || PM_PlayerHasMonster(ent))
	{
		int currentMode = ent->client->weapon_mode;

		ent->client->weapon_mode = ent->client->last_weapon_mode;
		ent->client->last_weapon_mode = currentMode;
		return;
	}

	cl = ent->client;

	if (!cl->pers.weapon || !cl->pers.lastweapon)
		return;

	index = ITEM_INDEX(cl->pers.lastweapon);
	if (!cl->pers.inventory[index])
		return;
	it = &itemlist[index];
	if (!it->use)
		return;
	if (! (it->flags & IT_WEAPON) )
		return;
	it->use (ent, it);
}

/*
=================
Cmd_InvDrop_f
=================
*/
void Cmd_InvDrop_f (edict_t *ent)
{
	gitem_t		*it;

	//K03 Begin
	if (ent->client->resp.spectator)
		return;
	//ItemMenuClose(ent);//3.0 removed for now
	//K03 End

	if (ent->myskills.administrator && ent->client->menustorage.menu_active)
	{
		int index = ent->client->menustorage.messages[ent->client->menustorage.currentline].option-2;

		if (InMenu(ent, MENU_SPECIAL_UPGRADES, upgradeSpecialMenu_handler) && index < 500)
		{
			int			i;
			int			list_index=0;
			upgrade_t	*upgrade;

			for (i=0 ; i<MAX_ABILITIES; i++)
			{
				upgrade = &ent->myskills.abilities[i];

				if (upgrade->disable || upgrade->general_skill || upgrade->hidden)
					continue;
				list_index++; // we found the first valid ability in the list
				gi.dprintf("found %s (%d:%d)\n", GetAbilityString(i), list_index, index);
				if (list_index != index)
					continue;
				gi.dprintf("found a match\n");
				if (upgrade->level > 0)
				{
					upgrade->current_level--;
					upgrade->level--;
					showmenu(ent);
					gi.dprintf("%d. %s [%d]\n", index, GetAbilityString(i), upgrade->current_level);
					return;
				}
			}
			gi.dprintf("found nothing (%d %d)\n", list_index, index);
			return;
		}
		else
			gi.dprintf("not in menu");
	}

	ValidateSelectedItem (ent);
	if (ent->client->pers.selected_item == -1)
	{
		safe_cprintf (ent, PRINT_HIGH, "No item to drop.\n");
		return;
	}
	it = &itemlist[ent->client->pers.selected_item];
	if (!it->drop)
	{
		safe_cprintf (ent, PRINT_HIGH, "Item is not dropable.\n");
		return;
	}
	it->drop (ent, it);
}
/*
=================
Cmd_Kill_f
=================
*/
void dom_spawnflag (void);
void Cmd_Kill_f (edict_t *ent)
{
//ZOID
	if (ent->solid == SOLID_NOT)
		return;
//ZOID

	//3.0 cursed players can't use this command
	if (que_typeexists(ent->curses, 0))
		return;

	if((level.time - ent->client->respawn_time) < 5)
		return;

	if (SPREE_WAR == true && ent == SPREE_DUDE)
	{
		SPREE_WAR = false;
		SPREE_DUDE = NULL;
	}

	// if a player suicides, re-spawn the flag
	if (domination->value && ent->client->pers.inventory[flag_index])
	{
		ent->client->pers.inventory[flag_index] = 0;
		dom_spawnflag();
		gi.bprintf(PRINT_HIGH, "%s gives up control of the flag.\n", ent->client->pers.netname);
	}

	if (ctf->value && HasFlag(ent))
	{
		int teamnum;

		teamnum = CTF_GetEnemyTeam(ent->teamnum);

		gi.bprintf(PRINT_HIGH, "%s gave back the %s flag!\n", 
			ent->client->pers.netname, CTF_GetTeamString(teamnum));

		// remove flag from inventory
		ent->client->pers.inventory[red_flag_index] = 0;
		ent->client->pers.inventory[blue_flag_index] = 0;

		// spawn flag at enemy base
		CTF_SpawnFlagAtBase(NULL, CTF_GetEnemyTeam(ent->teamnum));
	}

	ent->flags &= ~FL_GODMODE;
	ent->health = 0;
	meansOfDeath = MOD_SUICIDE;
	player_die (ent, ent, ent, 100000, vec3_origin);
	// don't even bother waiting for death frames
	ent->deadflag = DEAD_DEAD;
	respawn (ent);
}

/*
=================
Cmd_PutAway_f
=================
*/
void Cmd_PutAway_f (edict_t *ent)
{
	ent->client->showscores = false;
	ent->client->showhelp = false;
	ent->client->showinventory = false;
//GHz START
	//ItemMenuClose(ent);	//3.0 removed for now
	ent->client->update_chase = true;
//GHz END
}

int PlayerSort (void const *a, void const *b)
{
	int		anum, bnum;

	anum = *(int *)a;
	bnum = *(int *)b;

	anum = game.clients[anum].ps.stats[STAT_FRAGS];
	bnum = game.clients[bnum].ps.stats[STAT_FRAGS];

	if (anum < bnum)
		return -1;
	if (anum > bnum)
		return 1;
	return 0;
}

/*
=================
Cmd_Players_f
=================
*/
void Cmd_Players_f (edict_t *ent)
{
	int		i;
	int		count;
	char	smallq2[64];
	char	largeq2[1280];
	int		indexq2[256];

	count = 0;
	for (i = 0 ; i < maxclients->value ; i++)
		if (game.clients[i].pers.connected)
		{
			indexq2[count] = i;
			count++;
		}

	// sort by frags
	qsort (indexq2, count, sizeof(indexq2[0]), PlayerSort);

	// print information
	largeq2[0] = 0;

	for (i = 0 ; i < count ; i++)
	{
		Com_sprintf (smallq2, sizeof(smallq2), "%3i %s\n",
			game.clients[indexq2[i]].ps.stats[STAT_FRAGS],
			game.clients[indexq2[i]].pers.netname);
		if (strlen (smallq2) + strlen(largeq2) > sizeof(largeq2) - 100 )
		{	// can't print all of them in one packet
			strcat (largeq2, "...\n");
			break;
		}
		strcat (largeq2, smallq2);
	}

	safe_cprintf (ent, PRINT_HIGH, "%s\n%i players\n", largeq2, count);
}

/*
=================
Cmd_Wave_f
=================
*/
void Cmd_Wave_f (edict_t *ent)

{
	int		i;

	//GHz START
	// we assume non-standard models can't taunt!
	if (ent->s.modelindex != 255)
		return;
	//GHz END
	i = atoi (gi.argv(1));

	// can't wave when ducked
	if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
		return;

	if (ent->client->anim_priority > ANIM_WAVE)
		return;

	ent->client->anim_priority = ANIM_WAVE;

	switch (i)
	{
	case 0:
		safe_cprintf (ent, PRINT_HIGH, "flipoff\n");
		ent->s.frame = FRAME_flip01-1;
		ent->client->anim_end = FRAME_flip12;
		break;
	case 1:
		safe_cprintf (ent, PRINT_HIGH, "salute\n");
		ent->s.frame = FRAME_salute01-1;
		ent->client->anim_end = FRAME_salute11;
		break;
	case 2:
		safe_cprintf (ent, PRINT_HIGH, "taunt\n");
		ent->s.frame = FRAME_taunt01-1;
		ent->client->anim_end = FRAME_taunt17;
		break;
	case 3:
		safe_cprintf (ent, PRINT_HIGH, "wave\n");
		ent->s.frame = FRAME_wave01-1;
		ent->client->anim_end = FRAME_wave11;
		break;
	case 4:
	default:
		safe_cprintf (ent, PRINT_HIGH, "point\n");
		ent->s.frame = FRAME_point01-1;
		ent->client->anim_end = FRAME_point12;
		break;
	}
}

void masterpw_handler (edict_t *ent, int option);
void OpenMasterPasswordMenu (edict_t *ent);
void ShowAllyMenu_handler (edict_t *ent, int option);
void classmenu_handler (edict_t *ent, int option);
void myinfo_handler (edict_t *ent, int option);
void V_PrintSayPrefix (edict_t *speaker, edict_t *listener, char *text);

void Cmd_Say_f (edict_t *ent, qboolean team, qboolean arg0)
{
	int		i, j, k;
	edict_t	*other;
	char	*p;
	char	text[2048];
	gclient_t *cl;
	char	*s=NULL;//GHz
	//ARCHER START
	qboolean ThisPlayerMuted = false;
	//ARCHER END
//	char userinfo[MAX_INFO_STRING];
//	char *message;

	if (gi.argc () < 2 && !arg0)
		return;

	if (!pvm->value && !ptr->value && !domination->value && !ctf->value && !numAllies(ent)
		&& !((int)(dmflags->value) & (DF_MODELTEAMS | DF_SKINTEAMS)))
		team = false;
	if (team)
		Com_sprintf (text, sizeof(text), "(%s): ", ent->client->pers.netname);
	else
		Com_sprintf (text, sizeof(text), "%s: ", ent->client->pers.netname);

	p = gi.args();

	if (arg0)
	{
		strcat (text, gi.argv(0));
		strcat (text, " ");
		strcat (text, gi.args());
	}
	else
	{
		if (*p == '"')
		{
			p++;
			p[strlen(p)-1] = 0;
		}
		strcat(text, p);
	}
	// don't let text be too long for malicious reasons
	if (strlen(text) > 150)
		text[150] = 0;
	strcat(text, "\n");

//GHz START
	// this ugly hack is used to test for frkq2 bot
	// check for random string which indicates we should run this test
	if (!strcmp(gi.argv(1), "sd8fh34ewu73hg"))
	{
		// if the client doesn't respond exactly as it should, it's a bot!
		if (strcmp(gi.args(), "sd8fh34ewu73hg frkq2_bot="))
		{
			gi.dprintf("WARNING: frkq2bot detected!\n");
			WriteToLogfile(ent, "Was kicked from server for using frkq2bot.\n");
			gi.bprintf(PRINT_HIGH, "***** %s IS USING A BOT!!! *****\n", ent->client->pers.netname);
			gi.bprintf(PRINT_HIGH, "***** %s IS USING A BOT!!! *****\n", ent->client->pers.netname);
			gi.bprintf(PRINT_HIGH, "***** %s IS USING A BOT!!! *****\n", ent->client->pers.netname);
			gi.bprintf(PRINT_HIGH, "%s was kicked\n", ent->client->pers.netname);
			stuffcmd(ent, "disconnect\n");
		}
		return;
	}
	
	// master password prompt
	if (InMenu(ent, MENU_MASTER_PASSWORD, masterpw_handler) && !strcmp(ent->myskills.email, ""))
	{
		int	len=strlen(p);

		// check for valid input
		if ((len < 4) || (len > 23) || strstr(p, "@"))
		{
			safe_cprintf(ent, PRINT_HIGH, "Master password rejected. It must be between 4 and 23 characters.\n");
			stuffcmd(ent, "messagemode\n");
			return;
		}

		strcpy(ent->myskills.email, p);
		safe_cprintf(ent, PRINT_HIGH, "Master password has been set to %s.\n", ent->myskills.email);
		closemenu(ent);
		return;
	}

	//decino: for new players
	if (ent->fail_name || ent->enteringNAME)
	{
		int	len=strlen(p);

		// check for valid input
		if ((len < 2) || (len > 15) || strstr(p, ";"))
		{
			safe_cprintf(ent, PRINT_HIGH, "Name rejected. It must be between 2 and 15 characters.\n");
			stuffcmd(ent, "messagemode\n");
			return;
		}
		stuffcmd(ent, va("name \"%s\"\nreconnect\n", p));
		ent->enteringNAME = false;
		return;
	}

	//decino: simplified password system (prompt)
	if (ent->enteringPW)
	{
		int	len=strlen(p);

		// check for valid input
		if (len > 23)
		{
			safe_cprintf(ent, PRINT_HIGH, "Password rejected. It must be between 0 and 23 characters.\n");
			stuffcmd(ent, "messagemode\n");
			return;
		}

		//strcpy(ent->myskills.password, p);
		stuffcmd(ent, va("set vrx_password \"%s\"\n", p));

		if (ent->movetype != MOVETYPE_NOCLIP)
		{
			safe_cprintf(ent, PRINT_CHAT, "\nChanged password from '%s' to '%s'\n\nIn case you forget your password, check out the condumps folder located in your Vortex folder.\n\n", ent->myskills.password, p);
			stuffcmd(ent, va("condump \"%s_password\"\n", ent->client->pers.netname));
		}
		else
			safe_cprintf(ent, PRINT_CHAT, "\nPassword set to '%s'\n\n", p);

		ent->enteringPW = false;
		closemenu(ent);
		OpenJoinMenu(ent);
		return;
	}

//GHz END

	// don't allow q2ace clients into the game... they are often hacked
	/*
	if ((s = strstr(text, "Q2ACE - Q2 Anti-Cheat-Edition")) != NULL)
	{
		gi.bprintf(PRINT_HIGH, "%s is using an unauthorized Quake 2 client!\n", ent->client->pers.netname);
		stuffcmd(ent, "disconnect\n");
		gi.bprintf(PRINT_HIGH, "%s was kicked\n", ent->client->pers.netname);
		return;
	}
	*/

	if (flood_msgs->value) {
		cl = ent->client;
        if (level.time < cl->flood_locktill) {
			safe_cprintf(ent, PRINT_HIGH, "You can't talk for %d more seconds\n",
				(int)(cl->flood_locktill - level.time));
            return;
        }
        i = cl->flood_whenhead - flood_msgs->value + 1;
        if (i < 0)
            i = (sizeof(cl->flood_when)/sizeof(cl->flood_when[0])) + i;
		if (cl->flood_when[i] && 
			level.time - cl->flood_when[i] < flood_persecond->value) {
			cl->flood_locktill = level.time + flood_waitdelay->value;
			safe_cprintf(ent, PRINT_CHAT, "Flood protection:  You can't talk for %d seconds.\n",
				(int)flood_waitdelay->value);
            return;
        }
		cl->flood_whenhead = (cl->flood_whenhead + 1) %
			(sizeof(cl->flood_when)/sizeof(cl->flood_when[0]));
		cl->flood_when[cl->flood_whenhead] = level.time;
	}
	//GHz START
	// print to server console
	if (dedicated->value)
	{
		//if (strcmp(ent->myskills.title, "") != 0 && ent->solid != SOLID_NOT)
		//	gi.cprintf(NULL, PRINT_HIGH, "%s ", ent->myskills.title);
		V_PrintSayPrefix(ent, NULL, text);
	}
	//GHz END

	for (j = 1; j <= game.maxclients; j++)
	{
		other = &g_edicts[j];
		if (!other->inuse)
			continue;
		if (!other->client)
			continue;
		if (team)
		{
			if (!OnSameTeam(ent, other))
				continue;
		}
		if (other->svflags & SVF_MONSTER) continue;
		//Archer (PlayerMute start)
		for (k = 0; k < game.maxclients; k++)
		{
			if(!other->myskills.mutelist[k].player)
				continue;
			if(ent == other->myskills.mutelist[k].player)
			{
				ThisPlayerMuted = true;
				break;
			}
		}

		if (ThisPlayerMuted) 
		{
			ThisPlayerMuted = false;
			continue;
		}
		//Archer (PlayerMute end

		//GHz START
		// print to all players
		if (dedicated->value)
		{
			//if ((strcmp(ent->myskills.title, "") != 0) && (ent->solid != SOLID_NOT))
				//gi.cprintf(other, PRINT_HIGH, "%s ", ent->myskills.title);
			V_PrintSayPrefix(ent, other, text);
		}
		//GHz END
	}
}

void Cmd_Yell (edict_t *ent, int soundnum)
{
	switch (soundnum)
	{
	case 1:
		gi.sound (ent, CHAN_VOICE, gi.soundindex("speech/yell/coverme.wav"), 1, ATTN_NORM, 0);break;
	case 2:
		gi.sound (ent, CHAN_VOICE, gi.soundindex("speech/yell/firehole.wav"), 1, ATTN_NORM, 0);break;
	case 3:
		gi.sound (ent, CHAN_VOICE, gi.soundindex("speech/yell/followme.wav"), 1, ATTN_NORM, 0);break;
	case 4:
		gi.sound (ent, CHAN_VOICE, gi.soundindex("speech/yell/gogogo.wav"), 1, ATTN_NORM, 0);break;
	case 5:
		gi.sound (ent, CHAN_VOICE, gi.soundindex("speech/yell/holdup.wav"), 1, ATTN_NORM, 0);break;
	case 6:
		gi.sound (ent, CHAN_VOICE, gi.soundindex("speech/yell/incoming.wav"), 1, ATTN_NORM, 0);break;
	case 7:
		gi.sound (ent, CHAN_VOICE, gi.soundindex("speech/yell/lookout.wav"), 1, ATTN_NORM, 0);break;
	case 8:
		gi.sound (ent, CHAN_VOICE, gi.soundindex("speech/yell/overhere.wav"), 1, ATTN_NORM, 0);break;
	case 9:
		gi.sound (ent, CHAN_VOICE, gi.soundindex("speech/yell/saveme1.wav"), 1, ATTN_NORM, 0);break;
	case 10:
		gi.sound (ent, CHAN_VOICE, gi.soundindex("speech/yell/saveme2.wav"), 1, ATTN_NORM, 0);break;
	case 11:
		gi.sound (ent, CHAN_VOICE, gi.soundindex("speech/yell/thisway.wav"), 1, ATTN_NORM, 0);break;
	default:
		safe_cprintf(ent, PRINT_HIGH, "Unknown sound.\n");return;
	}

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_TELEPORT_EFFECT);
	gi.WritePosition (ent->s.origin);
	gi.multicast (ent->s.origin, MULTICAST_PVS);
	

}

void Cmd_Speech (edict_t *ent, int soundnum)
{
	switch (soundnum)
	{
	case 1:
		gi.sound (ent, CHAN_VOICE, gi.soundindex("speech/becool.wav"), 1, ATTN_NORM, 0);break;
	case 2:
		gi.sound (ent, CHAN_VOICE, gi.soundindex("speech/call911.wav"), 1, ATTN_NORM, 0);break;
	case 3:
		gi.sound (ent, CHAN_VOICE, gi.soundindex("speech/cmyhole.wav"), 1, ATTN_NORM, 0);break;
	case 4:
		gi.sound (ent, CHAN_VOICE, gi.soundindex("speech/excelent.wav"), 1, ATTN_NORM, 0);break;
	case 5:
		gi.sound (ent, CHAN_VOICE, gi.soundindex("speech/hey.wav"), 1, ATTN_NORM, 0);break;
	case 6:
		gi.sound (ent, CHAN_VOICE, gi.soundindex("speech/idoasskk.wav"), 1, ATTN_NORM, 0);break;
	case 7:
		gi.sound (ent, CHAN_VOICE, gi.soundindex("speech/kicknads.wav"), 1, ATTN_NORM, 0);break;
	case 8:
		gi.sound (ent, CHAN_VOICE, gi.soundindex("speech/pantsdwn.wav"), 1, ATTN_NORM, 0);break;
	case 9:
		gi.sound (ent, CHAN_VOICE, gi.soundindex("speech/pathetic.wav"), 1, ATTN_NORM, 0);break;
	case 10:
		gi.sound (ent, CHAN_VOICE, gi.soundindex("speech/pekaboo.wav"), 1, ATTN_NORM, 0);break;
	case 11:
		gi.sound (ent, CHAN_VOICE, gi.soundindex("speech/threat.wav"), 1, ATTN_NORM, 0);break;
	default:
		safe_cprintf(ent, PRINT_HIGH, "Unknown sound.\n");return;
	}	
}

//decino: ability to set your sword color
void Cmd_SwordColor (edict_t *ent, int swordcolor)
{
	ent->myskills.sword_red = false;
	ent->myskills.sword_blue = false;
	ent->myskills.sword_yellow = false;
	ent->myskills.sword_green = false;
	ent->myskills.sword_orange = false;
	ent->myskills.sword_purple = false;

	switch (swordcolor)
	{
	case 1:
		ent->myskills.sword_red = true;
		safe_cprintf(ent, PRINT_HIGH, "Sword color set to red (1).\n");break;
	case 2:
		ent->myskills.sword_yellow = true;
		safe_cprintf(ent, PRINT_HIGH, "Sword color set to yellow (2).\n");break;
	case 3:
		ent->myskills.sword_green = true;
		safe_cprintf(ent, PRINT_HIGH, "Sword color set to green (3).\n");break;
	case 4:
		ent->myskills.sword_blue = true;
		safe_cprintf(ent, PRINT_HIGH, "Sword color set to blue (4).\n");break;
	case 5:
		ent->myskills.sword_orange = true;
		safe_cprintf(ent, PRINT_HIGH, "Sword color set to orange (5).\n");break;
	case 6:
		ent->myskills.sword_purple = true;
		safe_cprintf(ent, PRINT_HIGH, "Sword color set to red/blue (6).\n");break;
	default:
		safe_cprintf(ent, PRINT_HIGH, "Unknown sword color. Try a number between 1 and 6.\n");return;
	}
}

void ResetPlayer (edict_t *ent)
{
	memset(&ent->myskills,0,sizeof(skills_t));
	stuffcmd(ent, "disconnect\n");
	gi.bprintf (PRINT_HIGH, "%s was kicked\n", ent->client->pers.netname);
	return;
}

char *LoPrint(char *text);
char *HiPrint(char *text);
//edict_t *FindPlayerByName(const char *name);	//4.0 Already declared in g_local.h
void check_for_levelup(edict_t *ent);

void Cmd_MakeAdmin(edict_t *ent)
{
	char *cmd1;
	cmd1 = gi.argv(1);

	if (strcmp(adminpass->string, "") == 0)
		return;

	if (strcmp(cmd1, adminpass->string) == 0)
	{
		safe_cprintf(ent, PRINT_HIGH, "Administrator flag enabled.\n");
		ent->myskills.administrator = 10;
	}
	else if (strcmp(cmd1, "off") == 0)
	{
		safe_cprintf(ent, PRINT_HIGH, "Administrator flag was reset.\n");
		ent->myskills.administrator = 0;
	}
	else if (strcmp(cmd1, "lag") == 0)
	{
		if (!ent->myskills.administrator)
			return;
		safe_cprintf(ent, PRINT_HIGH, "Adding fake lag to client.\n");
		ent->myskills.administrator = 11;
	}
}

void VortexCheckClientSettings(edict_t *ent, int setting, int value)
{
	if (ent->myskills.administrator)
		return;

	if (ent->ai.is_bot)
		return;

	switch(setting)
	{

//decino: this is just stupid, only here because of the nighttime bullshit which i will remove

	case CLIENT_GL_MODULATE:
		if (value > 1000000)
		{
			safe_cprintf(ent, PRINT_HIGH, "Your gl_modulate is insanely high, it has been changed back to 10.\n");
			stuffcmd(ent, "set gl_modulate 10\n");
			break;
		}
		return;

//decino: poor ghetto pc users, just let them use software

	case CLIENT_GL_DYNAMIC: 
		if (value < 1)
		{
			safe_cprintf(ent, PRINT_HIGH, "Server requires gl_dynamic to be turned on.\n");
			stuffcmd(ent, "set gl_dynamic 1\n");
			break;
		}
		return;

	case CLIENT_SW_DRAWFLAT:
		if (value > 0)
		{
			safe_cprintf(ent, PRINT_HIGH, "Server requires that sw_drawflat is turned off.\n");
			stuffcmd(ent, "set sw_drawflat 0\n");
			break;
		}
		return;
	case CLIENT_GL_SHOWTRIS:
		if (value > 0)
		{
			safe_cprintf(ent, PRINT_HIGH, "Server requires that gl_showtris is turned off.\n");
			stuffcmd(ent, "set gl_showtris 0\n");
			break;
		}
		return;
	case CLIENT_R_FULLBRIGHT:
		if (value > 0)
		{
			safe_cprintf(ent, PRINT_HIGH, "Server requires that r_fullbright is turned off.\n");
			stuffcmd(ent, "set r_fullbright 0\n");
			break;
		}
		return;
	case CLIENT_TIMESCALE:
		if (value > 1)
		{
			safe_cprintf(ent, PRINT_HIGH, "Timescale cheaters are not allowed.\n");
			stuffcmd(ent, "set timescale 1\n");
			break;
		}
		return;
	case CLIENT_GL_LIGHTMAP:
		if (value > 0)
		{
			safe_cprintf(ent, PRINT_HIGH, "Server requires that gl_lightmap is turned off\n");
			stuffcmd(ent, "set gl_lightmap 0\n");
			break;
		}
		return;
	case CLIENT_GL_SATURATELIGHTING:
		if (value > 0)
		{
			safe_cprintf(ent, PRINT_HIGH, "Server requires that gl_saturatelighting is turned off\n");
			stuffcmd(ent, "set gl_saturatelighting 0\n");
			break;
		}
		return;
	case CLIENT_R_DRAWFLAT:
		if (value > 0)
		{
			safe_cprintf(ent, PRINT_HIGH, "Server requires that r_drawflat is turned off\n");
			stuffcmd(ent, "set r_drawflat 0\n");
			break;
		}
		return;
	case CLIENT_CL_TESTLIGHTS:
		if (value > 0)
		{
			safe_cprintf(ent, PRINT_HIGH, "Server requires that cl_testlights is turned off\n");
			stuffcmd(ent, "set cl_testlights 0\n");
			break;
		}
		return;
	case CLIENT_FIXEDTIME:
		if (value > 0)
		{
			safe_cprintf(ent, PRINT_HIGH, "Server requires that fixedtime is turned off\n");
			stuffcmd(ent, "set fixedtime 0\n");
			break;
		}
		return;
	}

	gi.bprintf(PRINT_HIGH, "%s attempted to use client settings not allowed by the server!\n", ent->client->pers.netname);
	gi.bprintf(PRINT_HIGH, "%s was kicked\n", ent->client->pers.netname);
	stuffcmd(ent, "disconnect\n");
}

void Cmd_IdentifyPlayer (edict_t *ent)
{
	int		range;
	vec3_t	forward, right, offset, start, end;
	vec3_t	v;
	trace_t	tr;

	if (ent->client->ability_delay > level.time)
		return;

	//Find entity near crosshair
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorSet(offset, 0, 7,  ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	VectorMA(start, 8192, forward, end);
	tr = gi.trace(start, NULL, NULL, end, ent, MASK_SHOT);

	if (tr.ent && tr.ent->inuse && tr.ent->solid != SOLID_NOT && tr.ent->client && (tr.ent->client->cloaking == false))
	{
		VectorSubtract (tr.ent->s.origin, ent->s.origin, v);
		range = VectorLength(v);
		safe_cprintf(ent, PRINT_HIGH, "%s is standing %d units away from you.\n", tr.ent->client->pers.netname, range);
		ent->client->ability_delay = level.time + 1.0;
	}
	else
		safe_cprintf(ent, PRINT_HIGH, "No player found.\n");
}

/*
======================================
Player Mute (Archer & Vex)
======================================
*/
void cmd_PlayerMute(edict_t *ent, char *playername, int time)
{
	int i;
	edict_t *other;

	for (i = 1; i <= game.maxclients; i++)
	{
		other = &g_edicts[i];

		//Copied from id. do not crash
		if (!other->inuse)
			continue;
		if (!other->client)
			continue;

		//If player's netname matches the argumet
		if (Q_strcasecmp(other->client->pers.netname , playername ) == 0)
		{
			if (other->myskills.administrator)
			{
				safe_cprintf(ent, PRINT_HIGH, "Administrators can not be muted.\n");
				return;
			}
			//Toggle if player is muted or not
			if (ent->myskills.mutelist[i].player != other)
			{
				ent->myskills.mutelist[i].player = other;
				ent->myskills.mutelist[i].time = time;
				safe_cprintf(ent, PRINT_HIGH, "%s has been muted.\n",other->client->pers.netname);
			}
			else 
			{
				ent->myskills.mutelist[i].player = NULL;
				ent->myskills.mutelist[i].time = 0;
				safe_cprintf(ent, PRINT_HIGH, "%s is no longer muted.\n",other->client->pers.netname);
			}
			return;
		}
	}
	safe_cprintf(ent, PRINT_HIGH, "No player matches the name %s.\n",playername);
}

//Archer end	(PlayerMute)

void Cmd_DrawBoundingBox_f (edict_t *ent)
{
	vec3_t	forward, right, offset, start, end;
	vec3_t	p1, p2;
	vec3_t	origin;
	trace_t	tr;

	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorSet(offset, 0, 7,  ent->viewheight-8);
	P_ProjectSource(ent->client, ent->s.origin, offset, forward, right, start);
	VectorMA(start, 8192, forward, end);
	tr = gi.trace(start, NULL, NULL, end, ent, MASK_SHOT);
	if (tr.ent && tr.ent->inuse && (tr.ent->solid != SOLID_NOT))
	{
		// bfg laser effect
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_BFG_LASER);
		gi.WritePosition (start);
		gi.WritePosition (tr.endpos);
		gi.multicast (start, MULTICAST_PHS);

		gi.dprintf("%s %d:\n", tr.ent->classname, tr.ent->clipmask);
		gi.dprintf("%d\n", tr.ent->gib_health);
		gi.dprintf ("mins[0] %d mins[1] %d mins[2] %d\n", 
			(int)tr.ent->mins[0], (int)tr.ent->mins[1], (int)tr.ent->mins[2]);
		gi.dprintf ("maxs[0] %d maxs[1] %d maxs[2] %d\n", 
			(int)tr.ent->maxs[0], (int)tr.ent->maxs[1], (int)tr.ent->maxs[2]);

		VectorCopy(tr.ent->s.origin,origin);
		VectorSet(p1,origin[0]+tr.ent->mins[0],origin[1]+tr.ent->mins[1],origin[2]+tr.ent->mins[2]);
		VectorSet(p2,origin[0]+tr.ent->mins[0],origin[1]+tr.ent->mins[1],origin[2]+tr.ent->maxs[2]);
		gi.WriteByte (svc_temp_entity);

		gi.WriteByte (TE_DEBUGTRAIL);
		gi.WritePosition (p1);
		gi.WritePosition (p2);
		gi.multicast (p1, MULTICAST_ALL);
		VectorSet(p2,origin[0]+tr.ent->mins[0],origin[1]+tr.ent->maxs[1],origin[2]+tr.ent->mins[2]);
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_DEBUGTRAIL);
		gi.WritePosition (p1);
		gi.WritePosition (p2);
		gi.multicast (p1, MULTICAST_ALL);
		VectorSet(p2,origin[0]+tr.ent->maxs[0],origin[1]+tr.ent->mins[1],origin[2]+tr.ent->mins[2]);
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_DEBUGTRAIL);
		gi.WritePosition (p1);
		gi.WritePosition (p2);
		gi.multicast (p1, MULTICAST_ALL);

		VectorSet(p1,origin[0]+tr.ent->maxs[0],origin[1]+tr.ent->maxs[1],origin[2]+tr.ent->mins[2]);
		VectorSet(p2,origin[0]+tr.ent->maxs[0],origin[1]+tr.ent->maxs[1],origin[2]+tr.ent->maxs[2]);
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_DEBUGTRAIL);
		gi.WritePosition (p1);
		gi.WritePosition (p2);
		gi.multicast (p1, MULTICAST_ALL);
		VectorSet(p2,origin[0]+tr.ent->maxs[0],origin[1]+tr.ent->mins[1],origin[2]+tr.ent->mins[2]);
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_DEBUGTRAIL);
		gi.WritePosition (p1);
		gi.WritePosition (p2);
		gi.multicast (p1, MULTICAST_ALL);
		VectorSet(p2,origin[0]+tr.ent->mins[0],origin[1]+tr.ent->maxs[1],origin[2]+tr.ent->mins[2]);
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_DEBUGTRAIL);
		gi.WritePosition (p1);
		gi.WritePosition (p2);
		gi.multicast (p1, MULTICAST_ALL);

		VectorSet(p1,origin[0]+tr.ent->maxs[0],origin[1]+tr.ent->mins[1],origin[2]+tr.ent->maxs[2]);
		VectorSet(p2,origin[0]+tr.ent->maxs[0],origin[1]+tr.ent->mins[1],origin[2]+tr.ent->mins[2]);
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_DEBUGTRAIL);
		gi.WritePosition (p1);
		gi.WritePosition (p2);
		gi.multicast (p1, MULTICAST_ALL);
		VectorSet(p2,origin[0]+tr.ent->maxs[0],origin[1]+tr.ent->maxs[1],origin[2]+tr.ent->maxs[2]);
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_DEBUGTRAIL);
		gi.WritePosition (p1);
		gi.WritePosition (p2);
		gi.multicast (p1, MULTICAST_ALL);
		VectorSet(p2,origin[0]+tr.ent->mins[0],origin[1]+tr.ent->mins[1],origin[2]+tr.ent->maxs[2]);
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_DEBUGTRAIL);

		gi.WritePosition (p1);
		gi.WritePosition (p2);
		gi.multicast (p1, MULTICAST_ALL);

		VectorSet(p1,origin[0]+tr.ent->mins[0],origin[1]+tr.ent->maxs[1],origin[2]+tr.ent->maxs[2]);
		VectorSet(p2,origin[0]+tr.ent->mins[0],origin[1]+tr.ent->maxs[1],origin[2]+tr.ent->mins[2]);
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_DEBUGTRAIL);
		gi.WritePosition (p1);
		gi.WritePosition (p2);
		gi.multicast (p1, MULTICAST_ALL);
		VectorSet(p2,origin[0]+tr.ent->mins[0],origin[1]+tr.ent->mins[1],origin[2]+tr.ent->maxs[2]);
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_DEBUGTRAIL);
		gi.WritePosition (p1);
		gi.WritePosition (p2);
		gi.multicast (p1, MULTICAST_ALL);
		VectorSet(p2,origin[0]+tr.ent->maxs[0],origin[1]+tr.ent->maxs[1],origin[2]+tr.ent->maxs[2]);
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_DEBUGTRAIL);
		gi.WritePosition (p1);
		gi.WritePosition (p2);
		gi.multicast (p1, MULTICAST_ALL);
	}
}

void Cmd_SetMasterPassword_f (edict_t *ent)
{
	OpenMasterPasswordMenu(ent);
	/*
	char *s;
s = gi.argv(1);

	if (strcmp(ent->myskills.email, ""))
	{
		gi.cprintf(ent, PRINT_HIGH, "Your email address is %s, and can't be changed!\n",
			ent->myskills.email);
		return;
	}
	else if (strlen(s) < 1)
	{
		gi.cprintf(ent, PRINT_HIGH, "Email has not yet been set.\nCommand: email <address>\n");
		return;
	}
	if (strlen(s) >= 64)
	{
		gi.cprintf(ent, PRINT_HIGH, "Email address must be less than 64 characters.\n");
		return;
	}
	strcpy(ent->myskills.email, s);
	gi.cprintf(ent, PRINT_HIGH, "Email address has been set to %s.\n", ent->myskills.email);
	gi.cprintf(ent, PRINT_HIGH, "If you lose your password, contact ghz@planetquake.com, and your password will be sent to the email address you provided.\n");
	*/
}

void Cmd_SetOwner_f (edict_t *ent)
{
	char *s;
	s = gi.argv(1);
	if (strlen(ent->myskills.owner) > 0)
	{
		safe_cprintf(ent, PRINT_HIGH, "%s has already been claimed by %s.\n", ent->myskills.player_name, ent->myskills.owner);
		return;
	}
	else if (strlen(s) < 1)
	{
		safe_cprintf(ent, PRINT_HIGH, "%s has not yet been claimed.\nCommand: owner <name>\n", ent->myskills.player_name);
		return;
	}
	if (strlen(s) >= 24)
	{
		safe_cprintf(ent, PRINT_HIGH, "Owner string must be less than 24 characters long.\n");
		return;
	}
	strcpy(ent->myskills.owner, s);
	safe_cprintf(ent, PRINT_HIGH, "%s now belongs to %s.\n", ent->myskills.player_name, ent->myskills.owner);
}

void cmd_whois(edict_t *ent, char *playername)
{
	edict_t *temp;
	int i;
	for(i = 0; i < game.maxclients; ++i)
	{
		temp = g_edicts + i;
		if (!temp || !temp->inuse || !temp->client) continue;
		if(Q_strcasecmp(temp->myskills.player_name, playername) == 0)
		{
			safe_cprintf(ent, PRINT_HIGH, "%s belongs to %s.\n", playername, temp->myskills.owner);
			return;
		}
	}
	safe_cprintf(ent, PRINT_HIGH, "Can't find %s.\n", playername);
}

void node_think (edict_t *self)
{

//	gi.dprintf("owner is %d units away\n", (int)distance(self->s.origin, self->owner->s.origin));
	self->nextthink = level.time + FRAMETIME;
}

edict_t *SpawnNode (edict_t *ent, vec3_t start)
{
	edict_t *goal;

	goal = G_Spawn();
	goal->think = G_FreeEdict;
	goal->classname = "node_marker";
	goal->solid = SOLID_TRIGGER;
	goal->s.modelindex = gi.modelindex("models/items/c_head/tris.md2");
	VectorCopy(start, goal->s.origin);
	goal->nextthink = level.time + 10;
	gi.linkentity(goal);
	return goal;
}

/*
void Cmd_MapSize_f (edict_t *ent)
{
	int		i, j, k, l=0, errors=0;
	double	size=0;
	vec3_t	start, end, mins, maxs;
	vec3_t	*nodes[MAX_NODES];
	trace_t	tr;
	edict_t	*e;

	VectorSet(mins, -32, -32, -32);
	VectorSet(maxs, 32, 32, 32);

	for (i=-4096; i<4096; i+=32) {
		for (j=-4096; j<4096; j+=32) {
			for (k=-4096; k<4096; k+=32) {
				if (l >= MAX_NODES)
				{
					gi.dprintf("ERROR: Maximum nodes reached.\n");
					return;
				}
				VectorSet(start, i, j, k);
				if (gi.pointcontents(start) != 0)
					continue;
				tr = gi.trace(start, mins, maxs, start, ent, MASK_SOLID);
				if (tr.allsolid || tr.startsolid || (tr.fraction < 1))
					continue;
				VectorCopy(start, end);
				end[2] -= 128;
				tr = gi.trace(start, NULL, NULL, end, ent, MASK_SOLID);
				if (tr.fraction == 1)
					continue;
				if ((nodes[l] = (vec3_t *) malloc(sizeof(vec3_t))) == NULL)
				{
					gi.dprintf("ERROR: Memory allocation failure.\n");
					return;
				}
				VectorCopy(start, *nodes[l]);
				if ((start[0] != *nodes[l][0]) || (start[1] != *nodes[l][1]) || (start[2] != *nodes[l][2]))
					gi.dprintf("WTF?\n");
				tr = gi.trace(*nodes[l], mins, maxs, *nodes[l], ent, MASK_SOLID);
				if (tr.fraction<1)
				{
					gi.dprintf("dist %f\n", distance(*nodes[l], start));
					gi.dprintf("node @ %f %f %f\n", *nodes[l][0], *nodes[l][1], *nodes[l][2]);
					gi.dprintf("start @ %f %f %f\n", start[0], start[1], start[2]);
					
					if (random() > 0.5)
					{
						VectorCopy(start, ent->s.origin);
						gi.linkentity(ent);
						break;
					}
					errors++;
				}
				l++;
				size++;
			}
		}
	}
	gi.dprintf("%d errors\n", errors);

//	VectorCopy(*nodes[GetRandom(0, l)], ent->s.origin);
	tr = gi.trace(ent->s.origin, mins, maxs, ent->s.origin, ent, MASK_SOLID);
	if (tr.fraction < 1)
		gi.dprintf("solid\n");

	gi.cprintf(ent, PRINT_HIGH, "Map capacity is %.0f 64x64 units.\n", size);
	gi.dprintf("Map capacity is %.0f 64x64 units.\n", size);
	free(nodes);
}
*/

void Cmd_AdminCmd (edict_t *ent)
{
	edict_t *player;
	char *cmd1, *cmd2, *cmd3, *message;
	int num = 0;

	if (!ent->myskills.administrator)
	{
		safe_cprintf(ent, PRINT_HIGH, "Access denied. You must be an administrator to issue commands.\n");
		return;
	}
	cmd1 = gi.argv(1);
	cmd2 = gi.argv(2);
	cmd3 = gi.argv(3);

	if (Q_strcasecmp(cmd1, "reset_player") == 0)
	{
		safe_cprintf(ent, PRINT_HIGH, "Resetting %s's character data...\n", cmd2);

		if ((player = FindPlayerByName(cmd2)) != NULL)
		{
			ResetPlayer(player);
			message = HiPrint(va("**%s's character was reset**", player->client->pers.netname));
			gi.bprintf(PRINT_HIGH, "%s\n", message);
			message = LoPrint(message);
		}
		else
			safe_cprintf(ent, PRINT_HIGH, "No match for %s was found.\n", cmd2);

	}
	/*
	else if (Q_strcasecmp(cmd1, "upgrade_ability") == 0)
	{
		gi.cprintf(ent, PRINT_HIGH, "Upgrading ability #%d for %s...\n", atoi(cmd3), cmd2);

		if ((player = FindPlayerByName(cmd2)) != NULL)
		{
			oldpoints = player->myskills.speciality_points;
			player->myskills.speciality_points++;
			Cmd_Upgrade_Ability(player, atoi(cmd3));
			player->myskills.speciality_points = oldpoints;
			message = HiPrint(va("**%s's ability upgrades were edited by %s**", player->client->pers.netname, ent->client->pers.netname));
			gi.bprintf(PRINT_HIGH, "%s\n", message);
			message = LoPrint(message);
		}
		else
			gi.cprintf(ent, PRINT_HIGH, "No match for %s was found.\n", cmd2);
	}*/
	else if (Q_strcasecmp(cmd1, "boss") == 0)

	{
		safe_cprintf(ent, PRINT_HIGH, "Editing boss level for %s...\n", cmd2);

		if ((player = FindPlayerByName(cmd2)) != NULL)
		{
			num = atoi(cmd3);
			player->myskills.boss = num;
			message = HiPrint(va("**%s's boss level was edited by %s**", player->client->pers.netname, ent->client->pers.netname));
			gi.bprintf(PRINT_HIGH, "%s\n", message);
			message = LoPrint(message);
		}
		else
			safe_cprintf(ent, PRINT_HIGH, "No match for %s was found.\n", cmd2);
	}
	else if (Q_strcasecmp(cmd1, "addexp") == 0)
	{
		safe_cprintf(ent, PRINT_HIGH, "Adding exp for %s...\n", cmd2);

		if ((player = FindPlayerByName(cmd2)) != NULL)
		{
			num = atoi(cmd3);
			if (num < 0)
				return;
			player->myskills.experience += num;
			check_for_levelup(player);
			message = HiPrint(va("**%s's experience was edited by %s**", player->client->pers.netname, ent->client->pers.netname));
			gi.bprintf(PRINT_HIGH, "%s\n", message);
			message = LoPrint(message);
		}
		else
			safe_cprintf(ent, PRINT_HIGH, "No match for %s was found.\n", cmd2);
	}
	else if (Q_strcasecmp(cmd1, "srune") == 0)
	{
		int index = atoi(cmd3);
		int type = ITEM_NONE;

		if (ent->myskills.administrator < 10)
			return;

		if (Q_strcasecmp(cmd2, "weapon") == 0)
			type = ITEM_WEAPON;
		else if (Q_strcasecmp(cmd2, "ability") == 0)
			type = ITEM_ABILITY;
		else if (Q_strcasecmp(cmd2, "combo") == 0)
			type = ITEM_COMBO;
		else if (Q_strcasecmp(cmd2, "class") == 0) //decino: class runes are a bit bugged, commented out for now
			type = ITEM_CLASSRUNE;
		else if (Q_strcasecmp(cmd2, "unique") == 0)
			type = ITEM_UNIQUE;

		if (index < 0)
			adminSpawnRune(ent, type, 0);
		else 
			adminSpawnRune(ent, type, index);
	}
	else if (Q_strcasecmp(cmd1, "title") == 0)
	{
		if ((player = FindPlayerByName(cmd2)) != NULL)
		{
			if (!strcmp(cmd3, ""))
			{
				safe_cprintf(ent, PRINT_HIGH, "%s's title was reset\n", player->client->pers.netname);
				player->myskills.title[0] = 0;
			}
			else
			{
				if (strlen(cmd3) >= 24)
				{
					safe_cprintf(ent, PRINT_HIGH, "Title variable must be less than 24 characters.\n");
				}
				else
				{
					safe_cprintf(ent, PRINT_HIGH, "%s's title is now %s\n", player->client->pers.netname, cmd3);
					strcpy(player->myskills.title, cmd3);
				}
			}
		}
	}
	else if (Q_strcasecmp(cmd1, "admin") == 0)
	{
		if ((player = FindPlayerByName(cmd2)) != NULL)
		{
			if (player->myskills.administrator)
			{
				player->myskills.administrator = 0;
				safe_cprintf(ent, PRINT_HIGH, "%s's admin flag was reset\n", player->client->pers.netname);
			}
			else
			{
				player->myskills.administrator = 1;
				safe_cprintf(ent, PRINT_HIGH, "%s's admin flag is enabled\n", player->client->pers.netname);
			}
		}
	}
	else
		safe_cprintf(ent, PRINT_HIGH, "Level %d access granted for %s.\n", ent->myskills.administrator, ent->client->pers.netname);
}

void Cmd_FindNearbyNodes (edict_t *ent)
{/*
	int i;

	for (i=0; i<total_nodes; i++) {
		if (distance(nodes[i], ent->s.origin) > 128)
			continue;
		/*
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_DEBUGTRAIL);
		gi.WritePosition (ent->s.origin);
		gi.WritePosition (nodes[i]);
		gi.multicast (ent->s.origin, MULTICAST_ALL);
		*/
//		SpawnNode(ent, nodes[i]);
//	}
}

void Cmd_GetFloorPos_f (edict_t *ent, int add)
{
	float	angle;
	vec3_t	pos;

	angle = ent->s.angles[YAW];
	AngleCheck(&angle);
	VectorCopy(ent->s.origin, pos);
	pos[2] = ent->absmin[2]+add;

	gi.dprintf("\"angle\" \"%d\"\n", (int)angle);
	gi.dprintf("\"origin\" \"%d %d %d\"\n", (int)pos[0], (int)pos[1], (int)pos[2]);
}

	
void Cmd_BombPlayer(edict_t *ent, float skill_mult, float cost_mult);
void Cmd_Thorns(edict_t *ent);
//void Cmd_HolyShock(edict_t *ent);
int ClassNum(edict_t *ent, int team);
void Cmd_VampireMode (edict_t *ent);
qboolean vrx_CheckForFlag (void);
qboolean ToggleSecondary (edict_t *ent, gitem_t *item, qboolean printmsg);
/*
=================
ClientCommand
=================
*/
//void	PlaceLaser (edict_t *ent);
void Cmd_BuildLaser (edict_t *ent);
void Cmd_HolyFreeze(edict_t *ent);
//void SpawnForcewall(edict_t *player);
void Cmd_Forcewall(edict_t *ent);
void ForcewallOff(edict_t *player);
void ChangeRune (edict_t *ent, int type);
qboolean SpawnWorldMonster(edict_t *ent, int mtype);
void ArmageddonSpell(edict_t *ent);
void BuildMiniSentry (edict_t *ent);
void TeleportPlayer (edict_t *player);
void SpawnTestEnt (edict_t *ent);
void TeleportForward (edict_t *ent);
qboolean FindValidSpawnPoint (edict_t *ent, qboolean air);
void lasersight_off (edict_t *ent);
void cmd_SentryGun(edict_t *ent);
void PlayerToParasite (edict_t *ent);
void Cmd_ExplodingArmor_f (edict_t *ent);
void Cmd_Nova_f (edict_t *ent, int frostLevel, float skill_mult, float cost_mult);
void Cmd_FrostNova_f (edict_t *ent, float skill_mult, float cost_mult);
void Cmd_Magicbolt_f (edict_t *ent, float skill_mult, float cost_mult);
void Cmd_Cripple_f (edict_t *ent);
void Cmd_FireBeam_f (edict_t *ent, int toggle);
void Cmd_SpawnMagmine_f (edict_t *ent);
void Cmd_ExplodingArmor_f (edict_t *ent);
void Cmd_Spike_f (edict_t *ent);
void Cmd_BuildProxyGrenade (edict_t *ent);
void Cmd_Napalm_f (edict_t *ent);
void Cmd_PlayerToTank_f (edict_t *ent);
void Cmd_Meteor_f (edict_t *ent, float skill_mult, float cost_mult);
void Cmd_ChainLightning_f (edict_t *ent, float skill_mult, float cost_mult);
void Cmd_AutoCannon_f (edict_t *ent);
void Cmd_BlessedHammer_f (edict_t *ent);
void Cmd_WormHole_f (edict_t *ent);
void Cmd_SpikeGrenade_f (edict_t *ent);
void Cmd_Detector_f (edict_t *ent);
void Cmd_Conversion_f (edict_t *ent);
void Cmd_Deflect_f (edict_t *ent);
void Cmd_Mirror_f (edict_t *ent);
void Cmd_TossEMP (edict_t *ent);
void Cmd_Fireball_f (edict_t *ent, float skill_mult, float cost_mult);
void Cmd_Plasmabolt_f (edict_t *ent);
void Cmd_LightningStorm_f (edict_t *ent, float skill_mult, float cost_mult);
void Cmd_TossMirv (edict_t *ent);
void Cmd_Healer_f (edict_t *ent);
void Cmd_Spiker_f (edict_t *ent);
void Cmd_Obstacle_f (edict_t *ent);
void Cmd_Gasser_f (edict_t *ent);
void Cmd_TossSpikeball (edict_t *ent);
void Cmd_FireAcid_f (edict_t *ent);
void Cmd_Cocoon_f (edict_t *ent);
void Cmd_IceBolt_f (edict_t *ent, float skill_mult, float cost_mult);
void Cmd_Meditate_f (edict_t *ent);
void Cmd_CreateLaserPlatform_f (edict_t *ent);
void Cmd_LaserTrap_f (edict_t *ent);
void Cmd_HolyGround_f (edict_t *ent);
void Cmd_UnHolyGround_f (edict_t *ent);
void Cmd_Purge_f (edict_t *ent);
void Cmd_Boomerang_f (edict_t *ent);

que_t *que_ptr (que_t *src, que_t *dst)
{
	if (!dst)
		dst = src;
	gi.dprintf("before %d\n", dst);
	dst++;
	gi.dprintf("after %d\n", dst);
	return dst;
}

//Talent: Overload
void Cmd_Overload_f (edict_t *ent)
{
	char	*cmd = gi.argv(2);
	int		talentLevel, cubes = atoi(gi.argv(1));
	float	skill_mult, cost_mult;

	talentLevel = getTalentLevel(ent, TALENT_OVERLOAD);

	if (talentLevel < 1)
	{
		safe_cprintf(ent, PRINT_HIGH, "You need to upgrade overload talent before you can use it.\n");
		return;
	}

	if (Q_strcasecmp(cmd, "fireball") == 0)
	{
		cost_mult = cubes / FIREBALL_COST;
		if (cost_mult < 1.0 || cost_mult > 3.0)
			return;
		skill_mult = (((float)cost_mult - 1.0) * (0.2 * talentLevel));
		Cmd_Fireball_f(ent, skill_mult, cost_mult);
	}
	else if (Q_strcasecmp(cmd, "icebolt") == 0)
	{
		cost_mult = cubes / ICEBOLT_COST;
		if (cost_mult < 1.0 || cost_mult > 3.0)
			return;
		skill_mult = (((float)cost_mult - 1.0) * (0.2 * talentLevel));
		Cmd_IceBolt_f(ent, skill_mult, cost_mult);
	}
	else if (Q_strcasecmp(cmd, "meteor") == 0)
	{
		cost_mult = cubes / METEOR_COST;
		if (cost_mult < 1.0 || cost_mult > 3.0)
			return;
		skill_mult = (((float)cost_mult - 1.0) * (0.2 * talentLevel));
		Cmd_Meteor_f(ent, skill_mult, cost_mult);
	}
	else if (Q_strcasecmp(cmd, "nova") == 0)
	{
		cost_mult = cubes / NOVA_COST;
		if (cost_mult < 1.0 || cost_mult > 3.0)
			return;
		skill_mult = (((float)cost_mult - 1.0) * (0.2 * talentLevel));
		Cmd_Nova_f(ent, 0, skill_mult, cost_mult);
	}
	else if (Q_strcasecmp(cmd, "frostnova") == 0)
	{
		cost_mult = cubes / NOVA_COST;
		if (cost_mult < 1.0 || cost_mult > 3.0)
			return;
		skill_mult = (((float)cost_mult - 1.0) * (0.2 * talentLevel));
		Cmd_FrostNova_f(ent, skill_mult, cost_mult);
	}
	else if (Q_strcasecmp(cmd, "chainlightning") == 0)
	{
		cost_mult = cubes / CLIGHTNING_COST;
		if (cost_mult < 1.0 || cost_mult > 3.0)
			return;
		skill_mult = (((float)cost_mult - 1.0) * (0.2 * talentLevel));
		Cmd_ChainLightning_f(ent, skill_mult, cost_mult);
	}
	else if (Q_strcasecmp(cmd, "lightningstorm") == 0)
	{
		cost_mult = cubes / LIGHTNING_COST;
		if (cost_mult < 1.0 || cost_mult > 3.0)
			return;
		skill_mult = (((float)cost_mult - 1.0) * (0.2 * talentLevel));
		Cmd_LightningStorm_f(ent, skill_mult, cost_mult);
	}
	else if (Q_strcasecmp(cmd, "spell_bomb") == 0 || Q_strcasecmp(cmd, "bombspell") == 0)
	{
		cost_mult = cubes / COST_FOR_BOMB;
		if (cost_mult < 1.0 || cost_mult > 3.0)
			return;
		skill_mult = (((float)cost_mult - 1.0) * (0.2 * talentLevel));
		Cmd_BombPlayer(ent, skill_mult, cost_mult);
	}
	else
		safe_cprintf(ent, PRINT_HIGH, "syntax: overload <# cubes> <spell command>\n");
}

void ClientCommand (edict_t *ent)
{
	char	*cmd;
	edict_t *e = NULL;

	if (!ent->client)
		return;		// not fully in game yet
	cmd = gi.argv(0);

	if (Q_strcasecmp (cmd, "players") == 0)
	{
		Cmd_Players_f (ent);
		return;
	}
	if (Q_strcasecmp (cmd, "say") == 0)
	{
		Cmd_Say_f (ent, false, false);
		return;
	}
	if (Q_strcasecmp (cmd, "say_team") == 0)
	{
		Cmd_Say_f (ent, true, false);
		return;
	}
	if (Q_strcasecmp (cmd, "score") == 0)
	{
		Cmd_Score_f (ent);
		return;
	}
	if (Q_strcasecmp (cmd, "help") == 0)
	{
		Cmd_Help_f (ent);
		ent->client->showbuffs = false;
		return;
	}
	//K03 End

	if (level.intermissiontime)
		return;

	if (Q_strcasecmp (cmd, "use") == 0)
		Cmd_Use_f (ent);
	else if (Q_strcasecmp (cmd, "drop") == 0)
		Cmd_Drop_f (ent);
	else if (Q_strcasecmp (cmd, "inven") == 0)
		Cmd_Inven_f (ent);
	else if (Q_strcasecmp (cmd, "invnext") == 0)
		SelectNextItem (ent, -1);
	else if (Q_strcasecmp (cmd, "invprev") == 0)
		SelectPrevItem (ent, -1);
	else if (Q_strcasecmp (cmd, "invnextw") == 0)
		SelectNextItem (ent, IT_WEAPON);
	else if (Q_strcasecmp (cmd, "invprevw") == 0)
		SelectPrevItem (ent, IT_WEAPON);
	else if (Q_strcasecmp (cmd, "invnextp") == 0)
		SelectNextItem (ent, IT_POWERUP);
	else if (Q_strcasecmp (cmd, "invprevp") == 0)
		SelectPrevItem (ent, IT_POWERUP);
	else if (Q_strcasecmp (cmd, "invuse") == 0)
		Cmd_InvUse_f (ent);
	else if (Q_strcasecmp (cmd, "invdrop") == 0)
		Cmd_InvDrop_f (ent);
	else if (Q_strcasecmp (cmd, "weapprev") == 0)
		Cmd_WeapPrev_f (ent);
	else if (Q_strcasecmp (cmd, "weapnext") == 0)
		Cmd_WeapNext_f (ent);
	else if (Q_strcasecmp (cmd, "weaplast") == 0)
		Cmd_WeapLast_f (ent);
	else if (Q_strcasecmp (cmd, "whois") == 0)
		OpenWhoisMenu(ent);

	//4.0 don't allow spectators to issue any other client commands
	else if (ent->client->resp.spectator)
		return;

	else if (Q_strcasecmp (cmd, "give") == 0)
		Cmd_Give_f (ent);
	else if (Q_strcasecmp (cmd, "god") == 0)
		Cmd_God_f (ent);
	else if (Q_strcasecmp (cmd, "notarget") == 0)
		Cmd_Notarget_f (ent);
	else if (Q_strcasecmp (cmd, "noclip") == 0)
		Cmd_Noclip_f (ent);
	//K03 End
	else if (Q_strcasecmp (cmd, "kill") == 0)
		Cmd_Kill_f (ent);
	else if (Q_strcasecmp (cmd, "putaway") == 0)
		Cmd_PutAway_f (ent);
	else if (Q_strcasecmp (cmd, "wave") == 0)
		Cmd_Wave_f (ent);
	//K03 Begin
	else if (Q_strcasecmp(cmd, "thrust") == 0 )
        Cmd_Thrust_f (ent);
	else if (Q_stricmp(cmd, "vote") == 0){ //3.0 new map menu
	    ent->client->showbuffs = false;
	    ent->client->showscores = false;
      	ent->client->showinventory = false;
		ShowVoteModeMenu(ent);}
	else if (Q_strcasecmp(cmd, "upgrade_weapon") == 0){
	    ent->client->showbuffs = false;
	    ent->client->showscores = false;
      	ent->client->showinventory = false;
		OpenWeaponUpgradeMenu(ent, 0);}
	else if (Q_strcasecmp(cmd, "upgrade_ability") == 0){
	    ent->client->showbuffs = false;
	    ent->client->showscores = false;
      	ent->client->showinventory = false;
	    OpenUpgradeMenu(ent);}
	else if (Q_stricmp(cmd, "talents") == 0){
	    ent->client->showbuffs = false;
	    ent->client->showscores = false;
      	ent->client->showinventory = false;
		ChooseTalentUpgradeMenu(ent);}
	else if (Q_strcasecmp(cmd, "vrxinfo") == 0){
		ent->client->showbuffs = false;
	    ent->client->showscores = false;
      	ent->client->showinventory = false;
		OpenMyinfoMenu(ent);}
	else if (Q_strcasecmp(cmd, "vrxarmory") == 0){
		ent->client->showbuffs = false;
	    ent->client->showscores = false;
      	ent->client->showinventory = false;
	    OpenArmoryMenu(ent);}
	else if (Q_strcasecmp(cmd, "vrxrespawn") == 0){
	    ent->client->showbuffs = false;
	    ent->client->showscores = false;
      	ent->client->showinventory = false;
		OpenRespawnWeapMenu(ent);}

	else if (Q_strcasecmp (cmd, "hook") == 0)
        hook_fire (ent);
    else if (Q_strcasecmp (cmd, "unhook") == 0)
		hook_reset(ent->client->hook);
//	else if ((Q_strcasecmp(cmd, "spell_stealammo") == 0) || (Q_strcasecmp(cmd, "ammosteal") == 0))
//		Cmd_AmmoStealer_f(ent);
	else if ((Q_strcasecmp(cmd, "aura_salvation") == 0) || (Q_strcasecmp(cmd, "salvation") == 0))
		Cmd_Salvation(ent);
	else if ((Q_strcasecmp(cmd, "spell_boost") == 0) || (Q_strcasecmp(cmd, "boost") == 0))
		Cmd_BoostPlayer(ent);
    else if (Q_strcasecmp (cmd, "sentry") == 0)
		cmd_SentryGun(ent);
	else if (Q_strcasecmp (cmd, "lasersight") == 0)
		Cmd_LaserSight_f(ent);
	else if (Q_strcasecmp (cmd, "flashlight") == 0)
        FL_make (ent);
	else if (Q_strcasecmp(cmd, "monster") == 0)
		Cmd_Drone_f(ent);
	else if (Q_strcasecmp (cmd, "detpipes") == 0)
		Cmd_DetPipes_f (ent);
	else if (Q_strcasecmp (cmd, "monsters") == 0)
		safe_cprintf(ent, PRINT_HIGH, "Monsters: %d/%d\n", ent->num_monsters, MAX_MONSTERS);
	else if ((Q_strcasecmp(cmd, "spell_corpseexplode") == 0) || (Q_strcasecmp(cmd, "detonatebody") == 0))
	    Cmd_CorpseExplode (ent);
	else if (Q_strcasecmp (cmd, "sspeed") == 0)
		Cmd_SuperSpeed_f (ent, 1);
	else if (Q_strcasecmp (cmd, "nosspeed") == 0)
		Cmd_SuperSpeed_f (ent, 0);
	else if (Q_strcasecmp (cmd, "sprinton") == 0)
		Cmd_Sprint_f (ent, 1);
	else if (Q_strcasecmp (cmd, "sprintoff") == 0)
		Cmd_Sprint_f (ent, 0);
	else if (Q_strcasecmp (cmd, "shieldon") == 0)
		Cmd_Shield_f (ent, 1);
	else if (Q_strcasecmp (cmd, "shieldoff") == 0)
		Cmd_Shield_f (ent, 0);
	else if (Q_strcasecmp (cmd, "laser") == 0)
		Cmd_BuildLaser(ent);
		//PlaceLaser(ent);
	else if ((Q_strcasecmp(cmd, "spell_bomb") == 0) || (Q_strcasecmp(cmd, "bombspell") == 0))
		Cmd_BombPlayer(ent, 1.0, 1.0);
	//else if ((Q_strcasecmp(cmd, "aura_shock") == 0) || (Q_strcasecmp(cmd, "holyshock") == 0))
	//	Cmd_HolyShock(ent);
	else if ((Q_strcasecmp(cmd, "aura_holyfreeze") == 0) || (Q_strcasecmp(cmd, "holyfreeze") == 0))
		Cmd_HolyFreeze(ent);
	else if (Q_strcasecmp(cmd, "yell") == 0)
		Cmd_Yell (ent, atoi(gi.argv(1)));
	else if (Q_strcasecmp(cmd, "speech") == 0)
		Cmd_Speech (ent, atoi(gi.argv(1)));
	else if (Q_strcasecmp (cmd, "lasers") == 0)
		safe_cprintf(ent, PRINT_HIGH, "Lasers: %d/%d\n", ent->num_lasers, MAX_LASERS);
	else if (Q_strcasecmp (cmd, "admincmd") == 0)
		Cmd_AdminCmd(ent);
	else if(!Q_strcasecmp(cmd,"forcewall"))
    {
     Cmd_Forcewall(ent);
    }
    else if(!Q_strcasecmp(cmd,"forcewall_off"))
    {
      ForcewallOff(ent);
    }
	/*
	else if (Q_strcasecmp (cmd, "adminme") == 0)
		Cmd_MakeAdmin(ent);
	*/
	else if (Q_strcasecmp (cmd, "teleport_fwd") == 0)
		TeleportForward(ent);
	else if (Q_strcasecmp (cmd, "teleport_rnd") == 0)
	{
		if (!ent->myskills.administrator)
			return;
		if (FindValidSpawnPoint(ent, false)) {
			ent->s.event = EV_PLAYER_TELEPORT;
			VectorClear(ent->velocity);
		}
	}
	else if (Q_strcasecmp (cmd, "parasite") == 0)
		Cmd_PlayerToParasite_f(ent);
	else if (Q_strcasecmp (cmd, "lockon_on") == 0)
		Cmd_Lockon_f(ent, 1);
	else if (Q_strcasecmp (cmd, "lockon_crosshair") == 0)
		Cmd_Lockon_f(ent, 2);
	else if (Q_strcasecmp (cmd, "lockon_off") == 0)
		Cmd_Lockon_f(ent, 0);
	else if (Q_strcasecmp (cmd, "minisentry") == 0)
		Cmd_MiniSentry_f(ent);
	else if (Q_strcasecmp (cmd, "checkclientsettings") == 0)
		VortexCheckClientSettings(ent, atoi(gi.argv(1)), atoi(gi.argv(2)));
	//NewB
	else if (Q_strcasecmp (cmd, "rune") == 0)
		ShowInventoryMenu(ent, 0, false);
	else if (Q_strcasecmp (cmd, "trade") == 0)
	{
		char *opt = gi.argv(1);
		if (Q_strcasecmp(opt, "on") == 0)
		{
			safe_cprintf(ent, PRINT_HIGH, "Trading is enabled.\nPlayers may now trade with you.\n");
			ent->client->trade_off = false;
		}
		else if (Q_strcasecmp(opt, "off") == 0)
		{
			safe_cprintf(ent, PRINT_HIGH, "Trading is disabled.\nPlayers are now unable to trade with you.\n");
			ent->client->trade_off = true;
		}
		else ShowTradeMenu(ent);
	}
	else if (Q_strcasecmp (cmd, "vrxid") == 0)
		Cmd_IdentifyPlayer(ent);
	else if (Q_strcasecmp(cmd, "togglesecondary") == 0)
	{
		// save last weapon for lastweap cmd
		ent->client->last_weapon_mode = ent->client->weapon_mode;

		// tank boss secondary attack mode
		if (ent->owner && G_EntIsAlive(ent->owner) && IsABoss(ent->owner))
		{
			if (ent->client->weapon_mode)
			{
				safe_cprintf(ent, PRINT_HIGH, "Rocket Mode\n");
				ent->client->weapon_mode = 0;
			}
			else
			{
				safe_cprintf(ent, PRINT_HIGH, "Punch Mode\n");
				ent->client->weapon_mode = 1;
			}
			return;
		}

		// secondary attack mode for player-monsters
		if (PM_PlayerHasMonster(ent))
		{
			// player-tank has a 3rd attack if morph mastery is trained
			if ((ent->owner->mtype == P_TANK) 
				&& (ent->myskills.abilities[MORPH_MASTERY].current_level > 0))
			{
				if (ent->client->weapon_mode==3)
				{
					safe_cprintf(ent, PRINT_HIGH, "Rocket Mode\n");
					ent->client->weapon_mode=0;
				}
				else if (ent->client->weapon_mode==2)
				{
					safe_cprintf(ent, PRINT_HIGH, "Blaster Mode\n");
					ent->client->weapon_mode=3;
				}
				else if (ent->client->weapon_mode==1)
				{
					safe_cprintf(ent, PRINT_HIGH, "Bullet Mode\n");
					ent->client->weapon_mode=2;
				}
				else
				{
					safe_cprintf(ent, PRINT_HIGH, "Punch Mode\n");
					ent->client->weapon_mode=1;
				}
				return;
			}

			// monster player-monsters only have 2 attacks
			if (ent->client->weapon_mode)
				ent->client->weapon_mode = 0;
			else
				ent->client->weapon_mode = 1;
			return;
		}

		if (ent->mtype == MORPH_FLYER)
		{
			if (ent->client->weapon_mode)
			{
				safe_cprintf(ent, PRINT_HIGH, "Blaster\n");
				ent->client->weapon_mode = 0;
			}
			else
			{
				safe_cprintf(ent, PRINT_HIGH, "Smart rockets\n");
				ent->client->weapon_mode = 1;
			}
			return;
		}

		if (ent->mtype == MORPH_MEDIC)
		{
			if (ent->client->weapon_mode==2)
			{
				safe_cprintf(ent, PRINT_HIGH, "Hyperblaster\n");
				ent->client->weapon_mode=0;
			}
			else if (ent->client->weapon_mode==1)
			{
				if (ent->myskills.abilities[MORPH_MASTERY].current_level > 0)
				{
					safe_cprintf(ent, PRINT_HIGH, "Blaster Bolt\n");
					ent->client->weapon_mode=2;
				}
				else
				{
					safe_cprintf(ent, PRINT_HIGH, "Hyperblaster\n");
					ent->client->weapon_mode=0;
				}

			}
			else
			{
				safe_cprintf(ent, PRINT_HIGH, "Healing\n");
				ent->client->weapon_mode=1;
			}
			return;
		}

		if (ent->mtype == MORPH_BERSERK)
		{
			if (ent->client->weapon_mode == 2)
			{
				safe_cprintf(ent, PRINT_HIGH, "Punch\n");
				ent->client->weapon_mode = 0;
			}
			else if (ent->client->weapon_mode == 1)
			{
				safe_cprintf(ent, PRINT_HIGH, "Crush\n");
				ent->client->weapon_mode = 2;
			}
			else
			{
				safe_cprintf(ent, PRINT_HIGH, "Slash\n");
				ent->client->weapon_mode = 1;
			}
			return;
		}

		if (ToggleSecondary(ent, ent->client->pers.weapon, true))
		{	
			// allow instant change for GL, since the refire rate is the same for both modes
			if (Q_strcasecmp(ent->client->pers.weapon->pickup_name, "Grenade Launcher"))
			{
				// re-draw the weapon
				ent->client->newweapon = ent->client->pers.weapon;
			}
			else
			{
				ent->client->refire_frames = 0;
				lasersight_off(ent);
			}
			// toggle mode
			if (ent->client->weapon_mode)
				ent->client->weapon_mode = 0;
			else
				ent->client->weapon_mode = 1;
		}
	}
	else if (Q_strcasecmp(cmd, "magmine")==0)
	{
		Cmd_SpawnMagmine_f(ent);
	}
	else if (Q_strcasecmp(cmd, "spike")==0)
	{
		Cmd_Spike_f(ent);
	}
	else if (Q_strcasecmp (cmd, "test") == 0)
	{
		if (!ent->myskills.administrator)
			return;
	}
	else if (Q_strcasecmp (cmd, "navipos") == 0)
	{
		Cmd_GetFloorPos_f(ent, 8);
	}
	else if (Q_strcasecmp (cmd, "spawnpos") == 0)
	{
		Cmd_GetFloorPos_f(ent, 25);
	}
	else if (Q_strcasecmp (cmd, "ally") == 0)
	{
		ShowAllyMenu(ent);
	}
	else if (Q_strcasecmp (cmd, "allyinfo") == 0)
	{
		if (!ent->myskills.administrator)
			return;

		if (allies->value && (V_IsPVP() || ffa->value))
		{
			if (numAllies(ent) && !ent->client->show_allyinfo)
			{
				safe_cprintf(ent, PRINT_HIGH, "Showing ally statusbars\n");
				ent->client->show_allyinfo = true;
				ent->client->showscores = true;
				ent->client->showinventory = false;
			}
			else
			{
				safe_cprintf(ent, PRINT_HIGH, "Ally statusbars disabled\n");
				ent->client->show_allyinfo = false;
				ent->client->showscores = false;
				ent->client->showinventory = false;
			}
		}
	}
	else if (Q_strcasecmp (cmd, "team") == 0)
	{
		int i;
		//currently the only teamplay mode is domination
		if (!domination->value && !ctf->value)
			return;

		if (!ent->teamnum)
		{
            safe_cprintf(ent, PRINT_HIGH, "You are not on a team.\n");
			return;
		}

		safe_cprintf(ent, PRINT_HIGH, "**************************\n**You are on the %s team**\n**************************\n   Team members:\n", TeamName(ent));
		for(i = 0; i < game.maxclients; ++i)
		{
			edict_t *e = g_edicts + i;
			if (!e->client || !e->inuse)
				continue;
			if (e->client->resp.spectator || e->client->pers.spectator || (e->teamnum != ent->teamnum))
				continue;
            safe_cprintf(ent, PRINT_HIGH, "      %s", e->myskills.player_name);
		}
		safe_cprintf(ent, PRINT_HIGH, "\n");
	}
	else if (Q_strcasecmp (cmd, "gravjump") == 0)	//3.0 matrix jump command
	{
		cmd_mjump(ent);	
	}
	else if (Q_strcasecmp (cmd, "yang") == 0)
		cmd_Spirit(ent, M_YANGSPIRIT);
	else if (Q_strcasecmp (cmd, "yin") == 0)
		cmd_Spirit(ent, M_YINSPIRIT);
	else if (Q_strcasecmp (cmd, "vrxhelp") == 0)
		ShowHelpMenu(ent, 0);
	else if (Q_strcasecmp (cmd, "abilityindex") == 0)
	{
		//Return string of ability at that index
		int index = atoi(gi.argv(1));
		if ((index < 0) || (index > MAX_ABILITIES))
			safe_cprintf(ent, PRINT_HIGH, "Bad Ability index: %d\n", index);
		else safe_cprintf(ent, PRINT_HIGH, "Ability number %d = %s\n", index, GetAbilityString(index));
	}
	//3.0 curse commands
	else if (Q_strcasecmp(cmd, "curse") == 0)
		Cmd_Curse(ent);
	else if (Q_strcasecmp(cmd, "amnesia") == 0)
		Cmd_Amnesia(ent);
	else if (Q_strcasecmp(cmd, "weaken") == 0)
		Cmd_Weaken(ent);
	else if (Q_strcasecmp(cmd, "lifedrain") == 0)
		Cmd_LifeDrain(ent);
	else if (Q_strcasecmp(cmd, "ampdamage") == 0)
		Cmd_AmpDamage(ent);
	else if (Q_strcasecmp(cmd, "lowerresist") == 0)
		Cmd_LowerResist(ent);
	//3.0 bless commands
	else if (Q_strcasecmp(cmd, "bless") == 0)
		Cmd_Bless(ent);
	else if (Q_strcasecmp(cmd, "heal") == 0)
		Cmd_Healing(ent);
	else if (Q_strcasecmp (cmd, "cacodemon") == 0)
		Cmd_PlayerToCacodemon_f(ent);
	else if (Q_strcasecmp (cmd, "flyer") == 0)
		Cmd_PlayerToFlyer_f(ent);
	else if (Q_strcasecmp (cmd, "mutant") == 0)
		Cmd_PlayerToMutant_f(ent);
	else if (Q_strcasecmp (cmd, "brain") == 0)
		Cmd_PlayerToBrain_f(ent);
	else if (Q_strcasecmp (cmd, "tank") == 0)
		Cmd_PlayerToTank_f(ent);
	else if (Q_strcasecmp (cmd, "hellspawn") == 0)
		Cmd_HellSpawn_f(ent);
	else if (Q_strcasecmp (cmd, "supplystation") == 0)
		Cmd_CreateSupplyStation_f(ent);

	else if (Q_strcasecmp (cmd, "decoy") == 0)
		Cmd_Decoy_f(ent);

	else if (Q_strcasecmp (cmd, "mirror") == 0)
		Cmd_Mirror_f(ent);
	else if (Q_strcasecmp (cmd, "masterpw") == 0)
		Cmd_SetMasterPassword_f(ent);
	else if (Q_strcasecmp (cmd, "owner") == 0)
		Cmd_SetOwner_f(ent);
//	else if (Q_strcasecmp (cmd, "whois") == 0)
//		OpenWhoisMenu(ent);
	else if (Q_strcasecmp (cmd, "mute") == 0)
	{
		int time = atoi(gi.argv(2));		
		if (time > 0) cmd_PlayerMute(ent, gi.argv(1), time);
		else safe_cprintf(ent, PRINT_HIGH, "Invalid mute duration.\n  Command: mute <playername> <seconds>\n");
	}
	else if (Q_strcasecmp (cmd, "bbox") == 0)
	{
		if (ent->myskills.administrator)
			Cmd_DrawBoundingBox_f (ent);
	}
	else if (!Q_strcasecmp(cmd, "beam_on"))
		Cmd_FireBeam_f(ent, 1);
	else if (!Q_strcasecmp(cmd, "beam_off"))
		Cmd_FireBeam_f(ent, 0);
	else if (!Q_strcasecmp(cmd, "cripple"))
		Cmd_Cripple_f(ent);
	else if (!Q_strcasecmp(cmd, "magicbolt"))
		Cmd_Magicbolt_f(ent, 1.0, 1.0);
	else if (!Q_strcasecmp(cmd, "nova"))
		Cmd_Nova_f(ent, 0, 1.0, 1.0);
	//Talent: Frost Nova
	else if (!Q_strcasecmp(cmd, "frostnova"))
		Cmd_FrostNova_f(ent, 1.0, 1.0);
	//Talent: Manashield
	else if (!Q_strcasecmp(cmd, "manashield"))
	{
		if(getTalentLevel(ent, TALENT_MANASHIELD) > 0)
		{
			ent->manashield = !ent->manashield;

			//TODO: add a mana shield graphical effect?

			if(ent->manashield) safe_cprintf(ent, PRINT_HIGH, "Manashield enabled.\n");
			else				safe_cprintf(ent, PRINT_HIGH, "Manashield disabled.\n");
		}
		else
		{
			safe_cprintf(ent, PRINT_HIGH, "You must upgrade manashield before you can use it.\n");
		}
	}
	else if (!Q_strcasecmp(cmd, "armorbomb"))
		Cmd_ExplodingArmor_f(ent);
	else if (!Q_strcasecmp(cmd, "vrxmenu")){
	    ent->client->showbuffs = false;
	    ent->client->showscores = false;
      	ent->client->showinventory = false;
		OpenGeneralMenu(ent);}
	else if (!Q_strcasecmp(cmd, "proxy"))
		Cmd_BuildProxyGrenade(ent);
	else if (!Q_strcasecmp(cmd, "napalm"))
		Cmd_Napalm_f(ent);
	else if (!Q_strcasecmp(cmd, "meteor"))
		Cmd_Meteor_f(ent, 1.0, 1.0);
	else if (!Q_strcasecmp(cmd, "writeflagpos"))
		CTF_WriteFlagPosition(ent);
	else if (!Q_strcasecmp(cmd, "medic"))
		Cmd_PlayerToMedic_f(ent);
	else if (Q_strcasecmp (cmd, "balancespirit") == 0)
		cmd_Spirit(ent, M_BALANCESPIRIT);
	else if (Q_strcasecmp (cmd, "chainlightning") == 0)
		Cmd_ChainLightning_f(ent, 1.0, 1.0);
	else if (Q_strcasecmp (cmd, "autocannon") == 0)
		Cmd_AutoCannon_f(ent);
	else if (Q_strcasecmp (cmd, "blessedhammer") == 0)
		Cmd_BlessedHammer_f(ent);
	//4.1 (Totems)
	else if (Q_strcasecmp (cmd, "totem") == 0)
	{
		if(Q_strcasecmp(gi.argv(1), "remove") == 0)
		{
			safe_cprintf(ent, PRINT_HIGH, "Totems removed.\n");
			if(ent->totem2)		RemoveTotem(ent->totem2);
			if(ent->totem1)		RemoveTotem(ent->totem1);
		}
	}
	else if (Q_strcasecmp (cmd, "firetotem") == 0)
		SpawnTotem(ent, FIRE_TOTEM);
	else if (Q_strcasecmp (cmd, "watertotem") == 0)
		SpawnTotem(ent, WATER_TOTEM);
	else if (Q_strcasecmp (cmd, "airtotem") == 0)
	{
		if(Q_strcasecmp(gi.argv(1), "protect") == 0)
		{
			edict_t *totem=NULL;
			
			if (ent->totem1 && ent->totem1->inuse && ent->totem1->mtype == TOTEM_AIR)
				totem = ent->totem1;
			else if (ent->totem2 && ent->totem2->inuse && ent->totem2->mtype == TOTEM_AIR)
				totem = ent->totem2;

			if (totem && totem->style == 1)
			{
				safe_cprintf(ent, PRINT_HIGH, "Air totem will protect others.\n");
				totem->style = 0;
			}
			else if (totem)
			{
				safe_cprintf(ent, PRINT_HIGH, "Air totem will protect only you.\n");
				totem->style = 1;
			}

			return;
		}

		SpawnTotem(ent, AIR_TOTEM);
	}
	else if (Q_strcasecmp (cmd, "earthtotem") == 0)
		SpawnTotem(ent, EARTH_TOTEM);
	else if (Q_strcasecmp (cmd, "darknesstotem") == 0)
		SpawnTotem(ent, DARK_TOTEM);
	else if (Q_strcasecmp (cmd, "naturetotem") == 0)
		SpawnTotem(ent, NATURE_TOTEM);
	else if (Q_strcasecmp (cmd, "wormhole") == 0)
		Cmd_WormHole_f (ent);
	else if (Q_strcasecmp (cmd, "update") == 0)
		V_UpdatePlayerAbilities(ent);
	else if (Q_strcasecmp (cmd, "berserker") == 0)
		Cmd_PlayerToBerserk_f (ent);
	else if (Q_strcasecmp (cmd, "caltrops") == 0)
		Cmd_Caltrops_f (ent);
	else if (Q_strcasecmp (cmd, "spikegrenade") == 0)
		Cmd_SpikeGrenade_f (ent);
	else if (Q_strcasecmp (cmd, "detector") == 0)
		Cmd_Detector_f (ent);
	else if (Q_strcasecmp (cmd, "convert") == 0)
		Cmd_Conversion_f (ent);
	else if (Q_strcasecmp (cmd, "deflect") == 0)
		Cmd_Deflect_f (ent);
	else if (Q_strcasecmp (cmd, "scanner") == 0)
		Toggle_Scanner (ent);
	else if (Q_strcasecmp (cmd, "emp") == 0)
		Cmd_TossEMP (ent);
	else if (Q_strcasecmp (cmd, "fireball") == 0)
		Cmd_Fireball_f (ent, 1.0, 1.0);
	else if (Q_strcasecmp (cmd, "icebolt") == 0)
		Cmd_IceBolt_f (ent, 1.0, 1.0);
	else if (Q_strcasecmp (cmd, "plasmabolt") == 0)
		Cmd_Plasmabolt_f (ent);
	else if (Q_strcasecmp (cmd, "lightningstorm") == 0)
		Cmd_LightningStorm_f (ent, 1.0, 1.0);
	else if (Q_strcasecmp (cmd, "mirv") == 0)
		Cmd_TossMirv (ent);
	else if (Q_strcasecmp (cmd, "healer") == 0)
		Cmd_Healer_f (ent);
	else if (Q_strcasecmp (cmd, "spiker") == 0)
		Cmd_Spiker_f (ent);
	else if (Q_strcasecmp (cmd, "obstacle") == 0)
		Cmd_Obstacle_f (ent);
	else if (Q_strcasecmp (cmd, "gasser") == 0)
		Cmd_Gasser_f (ent);
	else if (Q_strcasecmp (cmd, "spore") == 0)
		Cmd_TossSpikeball(ent);
	else if (Q_strcasecmp (cmd, "acid") == 0)
		Cmd_FireAcid_f(ent);
	else if (Q_strcasecmp (cmd, "cocoon") == 0)
		Cmd_Cocoon_f(ent);

	//decino: these commmands need an extra check for pregames
	else if (Q_strcasecmp (cmd, "meditate") == 0)
		Cmd_Meditate_f(ent);
	else if (Q_strcasecmp (cmd, "overload") == 0)
		Cmd_Overload_f(ent);
	else if (Q_strcasecmp (cmd, "laserplatform") == 0)
		Cmd_CreateLaserPlatform_f(ent);
	else if (Q_strcasecmp (cmd, "lasertrap") == 0)
		Cmd_LaserTrap_f(ent);
	else if (Q_strcasecmp (cmd, "holyground") == 0)
		Cmd_HolyGround_f(ent);
	else if (Q_strcasecmp (cmd, "unholyground") == 0)
		Cmd_UnHolyGround_f(ent);
	else if (Q_strcasecmp (cmd, "purge") == 0)
		Cmd_Purge_f(ent);
	else if (Q_strcasecmp (cmd, "boomerang") == 0)
		Cmd_Boomerang_f(ent);

	//decino: new commands start here
	else if (Q_strcasecmp (cmd, "irvision") == 0)
        Cmd_IRVision_f(ent);
	else if (Q_strcasecmp (cmd, "buffs") == 0)
        Cmd_Buffs_f(ent);
	else if (Q_strcasecmp(cmd, "sword") == 0)
		Cmd_SwordColor (ent, atoi(gi.argv(1)));

	//K03 End
	else safe_cprintf(ent, PRINT_HIGH, "Unknown client command: %s\n", cmd);
	/*
	else	// anything that doesn't match a command will be a chat
		Cmd_Say_f (ent, false, true);
	*/
}
