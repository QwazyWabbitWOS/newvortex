#include "g_local.h"

void OpenGeneralTalentMenu (edict_t *ent, int lastline);
//Gives the player a new talent.
void addTalent(edict_t *ent, int talentID, int maxLevel)
{
	int nextEmptySlot = ent->myskills.talents.count;
	int i = 0;

	//Don't add too many talents.
	if(!(nextEmptySlot < MAX_TALENTS))
		return;

	//Don't add a talent more than once.
	for(i = 0; i < nextEmptySlot; ++i)
		if(ent->myskills.talents.talent[nextEmptySlot].id == talentID)
			return;

	ent->myskills.talents.talent[nextEmptySlot].id = talentID;
	ent->myskills.talents.talent[nextEmptySlot].maxLevel = maxLevel;
	ent->myskills.talents.talent[nextEmptySlot].upgradeLevel = 0;		//Just in case it's not already zero.

	ent->myskills.talents.count++;
}

//Adds all the required talents for said class.
void setTalents(edict_t *ent)
{
	switch(ent->myskills.class_num)
	{
		case CLASS_SOLDIER:
			addTalent(ent, TALENT_IMP_STRENGTH, 3);
			addTalent(ent, TALENT_IMP_RESIST, 3);
			addTalent(ent, TALENT_BLOOD_OF_ARES, 3);
//			addTalent(ent, TALENT_BASIC_HA, 5);
			addTalent(ent, TALENT_BOMBARDIER, 3);
			addTalent(ent, TALENT_RETALIATION, 3);
			return;
		case CLASS_POLTERGEIST:
			addTalent(ent, TALENT_MORPHING, 3);
			addTalent(ent, TALENT_MORE_AMMO, 3);
			addTalent(ent, TALENT_SUPERIORITY, 3);
//			addTalent(ent, TALENT_RETALIATION, 5);
			addTalent(ent, TALENT_SECOND_CHANCE, 3);
			addTalent(ent, TALENT_PACK_ANIMAL, 3);
			return;
		case CLASS_VAMPIRE:
			addTalent(ent, TALENT_IMP_CLOAK, 3);
			addTalent(ent, TALENT_ARMOR_VAMP, 3);
//			addTalent(ent, TALENT_SECOND_CHANCE, 4);
			addTalent(ent, TALENT_IMP_MINDABSORB, 3);
			addTalent(ent, TALENT_CANNIBALISM, 3);
			addTalent(ent, TALENT_FATAL_WOUND, 3);
			return;
		case CLASS_MAGE:
			addTalent(ent, TALENT_ICE_BOLT, 3);
			addTalent(ent, TALENT_FROST_NOVA, 3);
			addTalent(ent, TALENT_IMP_MAGICBOLT, 3);
			addTalent(ent, TALENT_MANASHIELD, 3);
			addTalent(ent, TALENT_MEDITATION, 3);
//			addTalent(ent, TALENT_OVERLOAD, 5);
			return;
		case CLASS_ENGINEER:
			addTalent(ent, TALENT_LASER_PLATFORM, 3);
            addTalent(ent, TALENT_ALARM, 3);
			addTalent(ent, TALENT_RAPID_ASSEMBLY, 3);
			addTalent(ent, TALENT_PRECISION_TUNING, 3);
			addTalent(ent, TALENT_STORAGE_UPGRADE, 3);
			return;
		case CLASS_KNIGHT:
			addTalent(ent, TALENT_REPEL, 3);
			addTalent(ent, TALENT_MAG_BOOTS, 3);
			addTalent(ent, TALENT_LEAP_ATTACK, 3);
			addTalent(ent, TALENT_MOBILITY, 3);
			addTalent(ent, TALENT_DURABILITY, 3);//lancing
			return;
		case CLASS_CLERIC:
			addTalent(ent, TALENT_BALANCESPIRIT, 3);
			addTalent(ent, TALENT_HOLY_GROUND, 3);
			addTalent(ent, TALENT_UNHOLY_GROUND, 3);
			addTalent(ent, TALENT_BOOMERANG, 3);
			addTalent(ent, TALENT_PURGE, 3);
			return;
		case CLASS_WEAPONMASTER:
			addTalent(ent, TALENT_BASIC_AMMO_REGEN, 3);
			addTalent(ent, TALENT_COMBAT_EXP, 3);
			addTalent(ent, TALENT_TACTICS, 3);//poison
			addTalent(ent, TALENT_SIDEARMS, 3);
			addTalent(ent, TALENT_OVERLOAD, 3);//ammo steal
			return;
		case CLASS_NECROMANCER:
			addTalent(ent, TALENT_CHEAPER_CURSES, 3);
			addTalent(ent, TALENT_CORPULENCE, 3);
			addTalent(ent, TALENT_LIFE_TAP, 3);
			addTalent(ent, TALENT_DIM_VISION, 3);
//			addTalent(ent, TALENT_EVIL_CURSE, 5);
			addTalent(ent, TALENT_FLIGHT, 3);
			return;
		case CLASS_SHAMAN://decino: all these talents need a replacement
			addTalent(ent, TALENT_ICE, 3);
			addTalent(ent, TALENT_WIND, 3);
			addTalent(ent, TALENT_SHADOW, 3);
			//addTalent(ent, TALENT_STONE, 3);
			addTalent(ent, TALENT_PEACE, 3);
			addTalent(ent, TALENT_TOTEM, 3);
			//addTalent(ent, TALENT_VOLCANIC, 3);
			return;
		case CLASS_ALIEN:
			addTalent(ent, TALENT_PHANTOM_OBSTACLE, 3);
			addTalent(ent, TALENT_SUPER_HEALER, 3);
			addTalent(ent, TALENT_PHANTOM_COCOON, 3);
			addTalent(ent, TALENT_SWARMING, 3);
			addTalent(ent, TALENT_EXPLODING_BODIES, 3);
			return;
	default: return;
	}
}

//Erases all talent information.
void eraseTalents(edict_t *ent)
{
	memset(&ent->myskills.talents, 0, sizeof(talentlist_t));
}

//Returns the talent slot with matching talentID.
//Returns -1 if there is no matching talent.
int getTalentSlot(edict_t *ent, int talentID)
{
	int i;

	//Make sure the ent is valid
	if(!ent)
	{
		WriteServerMsg(va("getTalentSlot() called with a NULL entity. talentID = %d", talentID), "CRITICAL ERROR", true, true);
		return -1;
	}

	//Make sure we are a player
	if(!ent->client)
	{
		//gi.dprintf(va("WARNING: getTalentSlot() called with a non-player entity! talentID = %d\n", talentID));
		return -1;
	}

	for(i = 0; i < ent->myskills.talents.count; ++i)
		if(ent->myskills.talents.talent[i].id == talentID)
			return i;
	return -1;
}

//Returns the talent upgrade level matching talentID.
//Returns -1 if there is no matching talent.
int getTalentLevel(edict_t *ent, int talentID)
{
	int slot = getTalentSlot(ent, talentID);
	
	if(slot < 0)	return 0;//-1;

	return ent->myskills.talents.talent[slot].upgradeLevel;	
}

//Upgrades the talent with a matching talentID
void upgradeTalent(edict_t *ent, int talentID)
{
	int slot = getTalentSlot(ent, talentID);
	talent_t *talent;

	if(slot == -1)
		return;

	talent = &ent->myskills.talents.talent[slot];

	// check for conflicting talents
	if (talentID == TALENT_CORPULENCE && getTalentLevel(ent, TALENT_LIFE_TAP) > 0)
	{
		gi.cprintf(ent, PRINT_HIGH, "Corpulence can't be combined with Life Tap.\n");
		return;
	}
	if (talentID == TALENT_LIFE_TAP && getTalentLevel(ent, TALENT_CORPULENCE) > 0)
	{
		gi.cprintf(ent, PRINT_HIGH, "Life Tap can't be combined with Corpulence.\n");
		return;
	}

	if(talent->upgradeLevel == talent->maxLevel)
	{
		gi.cprintf(ent, PRINT_HIGH, "You can not upgrade this talent any further.\n");
		return;
	}
	if(ent->myskills.talents.talentPoints < 1)
	{
		gi.cprintf(ent, PRINT_HIGH, "You do not have enough talent points.\n");
		return;
	}

    //We can upgrade.
	talent->upgradeLevel++;
	ent->myskills.talents.talentPoints--;
	gi.cprintf(ent, PRINT_HIGH, va("%s upgraded to level %d/%d.\n", GetTalentString(talent->id), talent->upgradeLevel, talent->maxLevel));
	gi.cprintf(ent, PRINT_HIGH, va("Talent points remaining: %d\n", ent->myskills.talents.talentPoints));
	savePlayer(ent);
}

//Upgrades the talent with a matching talentID
void upgradeGenTalent(edict_t *ent, int talentID)
{
	int slot = getTalentSlot(ent, talentID);
	talent_t *talent;

//	if(slot == -1)
//		return;

	talent = &ent->myskills.talents.talent[slot];

	if(talent->upgradeLevel == talent->maxLevel)
	{
		gi.cprintf(ent, PRINT_HIGH, "You can not upgrade this talent any further.\n");
		return;
	}
	if(ent->myskills.talents.talentPoints < 1)
	{
		gi.cprintf(ent, PRINT_HIGH, "You do not have enough talent points.\n");
		return;
	}

    //We can upgrade.
	talent->upgradeLevel++;
	ent->myskills.talents.talentPoints--;
	gi.cprintf(ent, PRINT_HIGH, va("%s upgraded to level %d/%d.\n", GetTalentString(talent->id), talent->upgradeLevel, talent->maxLevel));
	gi.cprintf(ent, PRINT_HIGH, va("Talent points remaining: %d\n", ent->myskills.talents.talentPoints));
	savePlayer(ent);
}

//****************************************
//************* Talent Menus *************
//****************************************

void TalentUpgradeMenu_handler(edict_t *ent, int option)
{
	//Not upgrading
	if(option > 0)
	{
		OpenTalentUpgradeMenu(ent, getTalentSlot(ent, option-1)+1);
	}
	else	//upgrading
	{
		upgradeTalent(ent, (option * -1)-1);
	}
}

void GenTalentUpgradeMenu_handler(edict_t *ent, int option)
{
	//Not upgrading
	if(option > 0)
	{
//		OpenGeneralTalentMenu(ent, getTalentSlot(ent, option-1)+1);
	}
	else	//upgrading
	{
		upgradeGenTalent(ent, (option * -1)-1);
	}
}

//Adds menu lines that describe the general use of the talent.
int writeTalentDescription(edict_t *ent, int talentID)
{
	switch(talentID)
	{
	//Soldier talents
	case TALENT_IMP_STRENGTH:
		addlinetomenu(ent, "Increases damage,", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "but reduces resist.", MENU_WHITE_CENTERED);
		return 2;
	case TALENT_IMP_RESIST:
		addlinetomenu(ent, "Increases resist,", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "but reduces damage.", MENU_WHITE_CENTERED);
		return 2;
	case TALENT_BLOOD_OF_ARES:
		addlinetomenu(ent, "Increases the damage you", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "give/take as you spree.", MENU_WHITE_CENTERED);
		return 2;
	case TALENT_BASIC_HA:
		addlinetomenu(ent, "Increases ammo pickups.", MENU_WHITE_CENTERED);
		return 1;
	case TALENT_BOMBARDIER:
		addlinetomenu(ent, "Reduces self-inflicted", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "grenade damage and", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "reduces cost.", MENU_WHITE_CENTERED);
		return 3;
	//Poltergeist talents
    case TALENT_MORPHING:
		addlinetomenu(ent, "Reduces the cost", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "of your morphs.", MENU_WHITE_CENTERED);
		return 2;
	case TALENT_MORE_AMMO:
		addlinetomenu(ent, "Increases maximum ammo", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "capacity for", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "tank/caco/flyer/medic.", MENU_WHITE_CENTERED);
		return 3;
	case TALENT_SUPERIORITY:
		addlinetomenu(ent, "Increases your damage", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "as you level up.", MENU_WHITE_CENTERED);
		return 2;
	case TALENT_RETALIATION:
		addlinetomenu(ent, "Damage increases as", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "health is reduced.", MENU_WHITE_CENTERED);
		return 2;
	case TALENT_PACK_ANIMAL:
	    addlinetomenu(ent, "Increased damage and", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "resistance when near", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "friendly morphed", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "players.", MENU_WHITE_CENTERED);
		return 4;
	//Vampire talents
	case TALENT_IMP_CLOAK:
		addlinetomenu(ent, "Move while cloaked", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "(must be crouching).", MENU_WHITE_CENTERED);
		return 2;
	case TALENT_ARMOR_VAMP:
		addlinetomenu(ent, "Also gain armor using", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "your vampire skill.", MENU_WHITE_CENTERED);
		return 2;
	case TALENT_FATAL_WOUND:
		addlinetomenu(ent, "Adds chance for flesh", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "eater to cause a fatal", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "wound.", MENU_WHITE_CENTERED);
		return 3;
	case TALENT_SECOND_CHANCE:
		addlinetomenu(ent, "Increases melee range of", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "berserker/parasite/mutant", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "and tank.", MENU_WHITE_CENTERED);
		return 3;
	case TALENT_IMP_MINDABSORB:
		addlinetomenu(ent, "Increases frequency of", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "mind absorb attacks.", MENU_WHITE_CENTERED);
		return 2;
	case TALENT_CANNIBALISM:
		addlinetomenu(ent, "Increases your maximum", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "health using corpse eater.", MENU_WHITE_CENTERED);
		return 2;
	//Mage talents
	case TALENT_ICE_BOLT:
		addlinetomenu(ent, "Special fireball spell", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "that chills players upon.", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "hit (cmd 'icebolt').", MENU_WHITE_CENTERED);
		return 3;
	case TALENT_MEDITATION:
		addlinetomenu(ent, "Meditate to build power", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "cubes (cmd 'meditate').", MENU_WHITE_CENTERED);
		return 2;
/*	case TALENT_OVERLOAD:
		addlinetomenu(ent, "Use extra power cubes", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "to overload spells,", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "increasing their", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "effectiveness", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "(cmd 'overload').", MENU_WHITE_CENTERED);
		return 5;*/
	case TALENT_FROST_NOVA:
		addlinetomenu(ent, "Special nova spell", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "that chills players.", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "(cmd 'frostnova')", MENU_WHITE_CENTERED);
		return 3;
	case TALENT_IMP_MAGICBOLT:
		addlinetomenu(ent, "Causes magic bolts to", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "explode.", MENU_WHITE_CENTERED);
		return 2;
	case TALENT_MANASHIELD:
		addlinetomenu(ent, "Reduces physical damage", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "by 50%%. All damage", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "absorbed consumes power", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "cubes. (cmd 'manashield')", MENU_WHITE_CENTERED);
		return 4;
	//Engineer talents
	case TALENT_LASER_PLATFORM:
		addlinetomenu(ent, "Create a laser platform", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "(cmd 'laserplatform').", MENU_WHITE_CENTERED);
		return 2;
	case TALENT_ALARM:
		addlinetomenu(ent, "Create a trap which attacks", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "enemies that get too close", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "(cmd 'lasertrap').", MENU_WHITE_CENTERED);
		return 3;
	case TALENT_RAPID_ASSEMBLY:
		addlinetomenu(ent, "Reduces build time.", MENU_WHITE_CENTERED);
		return 1;
	case TALENT_PRECISION_TUNING:
		addlinetomenu(ent, "Reduces the effect of curses", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "and other negative effects", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "on your devices.", MENU_WHITE_CENTERED);
		return 3;
	case TALENT_STORAGE_UPGRADE:
		addlinetomenu(ent, "Increases ammunition", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "capacity of SS/sentry/AC.", MENU_WHITE_CENTERED);
		return 2;
	//Knight talents
	case TALENT_REPEL:
		addlinetomenu(ent, "Adds chance for projectiles", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "to deflect from shield.", MENU_WHITE_CENTERED);
		return 2;
	case TALENT_MAG_BOOTS:
		addlinetomenu(ent, "Reduces effect of knockback.", MENU_WHITE_CENTERED);
		return 1;
	case TALENT_LEAP_ATTACK:
		addlinetomenu(ent, "Adds stun/knockback effect", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "to boost spell when landing.", MENU_WHITE_CENTERED);
		return 2;
	case TALENT_MOBILITY:
		addlinetomenu(ent, "Reduces boost cooldown.", MENU_WHITE_CENTERED);
		return 1;
	case TALENT_DURABILITY:
		addlinetomenu(ent, "Increases your sword length.", MENU_WHITE_CENTERED);
		return 1;
	//Cleric talents
	case TALENT_BALANCESPIRIT:
		addlinetomenu(ent, "New spirit that can", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "use the skills of both", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "yin and yang spirits.", MENU_WHITE_CENTERED);
		return 3;
	case TALENT_HOLY_GROUND:
		addlinetomenu(ent, "Designate an area as", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "holy ground to regenerate", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "teammates (cmd 'holyground').", MENU_WHITE_CENTERED);
		return 3;
	case TALENT_UNHOLY_GROUND:
		addlinetomenu(ent, "Designate an area as", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "unholy ground to damage", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "enemies (cmd 'unholyground').", MENU_WHITE_CENTERED);
		return 3;
	case TALENT_BOOMERANG:
		addlinetomenu(ent, "Turns blessed hammers", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "into boomerangs", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "(cmd 'boomerang').", MENU_WHITE_CENTERED);
		return 3;
	case TALENT_PURGE:
		addlinetomenu(ent, "Removes curses and grants", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "temporary invincibility", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "and immunity to curses", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "(cmd 'purge').", MENU_WHITE_CENTERED);
		return 4;
	//Weaponmaster talents
	case TALENT_BASIC_AMMO_REGEN:
		addlinetomenu(ent, "Basic ammo regeneration.", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "Regenerates one ammo pack", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "for the weapon in use.", MENU_WHITE_CENTERED);
		return 3;
	case TALENT_COMBAT_EXP:
	    addlinetomenu(ent, "Increases physical", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "damage, but reduces", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "resistance.", MENU_WHITE_CENTERED);
		return 3;
	case TALENT_TACTICS:
	    addlinetomenu(ent, "Adds a chance poisoning", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "players while dealing", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "damage.", MENU_WHITE_CENTERED);
		return 3;
	case TALENT_OVERLOAD:
	    addlinetomenu(ent, "Steal ammo from your", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "enemy while dealing damage", MENU_WHITE_CENTERED);
		return 2;
	case TALENT_SIDEARMS:
	    addlinetomenu(ent, "Gives you additional", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "respawn weapons. Weapon", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "choice is determined by", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "weapon upgrade level.", MENU_WHITE_CENTERED);
		return 4;
	//Necromancer talents
	case TALENT_EVIL_CURSE:
		addlinetomenu(ent, "Increases curse duration.", MENU_WHITE_CENTERED);
		return 1;
	case TALENT_CHEAPER_CURSES:
		addlinetomenu(ent, "Reduces curse cost.", MENU_WHITE_CENTERED);
		return 1;
	case TALENT_CORPULENCE:
		addlinetomenu(ent, "Increases hellspawn", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "and monster health/armor,", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "but reduces damage. Can't,", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "combine with Life Tap.", MENU_WHITE_CENTERED);
		return 4;
	case TALENT_LIFE_TAP:
		addlinetomenu(ent, "Increases hellspawn", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "and monster damage,", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "but slowly saps life.", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "Can't combine with", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "Corpulence.", MENU_WHITE_CENTERED);
		return 5;
	case TALENT_DIM_VISION:
		addlinetomenu(ent, "Adds chance to", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "automatically curse", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "enemies that shoot you.", MENU_WHITE_CENTERED);
		return 3;
	case TALENT_FLIGHT:
		addlinetomenu(ent, "Reduces jetpack cost.", MENU_WHITE_CENTERED);
		return 1;
	//Shaman talents
	case TALENT_TOTEM:
		addlinetomenu(ent, "Increases totem health.", MENU_WHITE_CENTERED);
		return 1;

	//decino: this is now the fury talent
	case TALENT_PEACE:
		addlinetomenu(ent, "Increases the chance", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "to activate fury.", MENU_WHITE_CENTERED);
		return 2;

    //decino: this talent is now water+fire
	case TALENT_ICE:
		addlinetomenu(ent, "Water totem will damage", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "nearby enemies.", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "Fire totem has a chance to", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "shoot a fireball.", MENU_WHITE_CENTERED);
		return 4;

    //decino: this talent is now air+earth
	case TALENT_WIND:
		addlinetomenu(ent, "Air totem has a chance", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "to ghost attacks for you.", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "Earth totem will increase", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "your resistance.", MENU_WHITE_CENTERED);
		return 4;

    //decino: this talent is now dark+nature
	case TALENT_SHADOW:
		addlinetomenu(ent, "Darkness totem will vamp", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "beyond your max health.", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "Nature totem will", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "regenerate power cubes.", MENU_WHITE_CENTERED);
		return 4;

	//decino: not used
	//case TALENT_STONE:
	//	addlinetomenu(ent, "Allows your earth totem to", MENU_WHITE_CENTERED);
	//	addlinetomenu(ent, "increase your resistance.", MENU_WHITE_CENTERED);
	//	return 2;

    //decino: not used
	//case TALENT_PEACE:
	//	addlinetomenu(ent, "Allows your nature totem to", MENU_WHITE_CENTERED);
	//	addlinetomenu(ent, "regenerate your power cubes.", MENU_WHITE_CENTERED);
	//	return 2;

    //decino: not used
	//case TALENT_VOLCANIC:
	//	addlinetomenu(ent, "Adds a chance for fire", MENU_WHITE_CENTERED);
	//	addlinetomenu(ent, "totem to shoot a fireball.", MENU_WHITE_CENTERED);
	//	return 2;


	//Alien talents
	case TALENT_PHANTOM_OBSTACLE:
		addlinetomenu(ent, "Reduces time to cloak.", MENU_WHITE_CENTERED);
		return 1;
	case TALENT_SUPER_HEALER:
		addlinetomenu(ent, "Adds armor regeneration", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "to your healer", MENU_WHITE_CENTERED);
		return 2;
	case TALENT_PHANTOM_COCOON:
		addlinetomenu(ent, "Allows cocoon to cloak.", MENU_WHITE_CENTERED);
		return 1;
	case TALENT_SWARMING:
		addlinetomenu(ent, "Increases spore count,", MENU_WHITE_CENTERED);
		return 1;
	case TALENT_EXPLODING_BODIES:
		addlinetomenu(ent, "Makes alien-summons'", MENU_WHITE_CENTERED);
		addlinetomenu(ent, "corpses explode.", MENU_WHITE_CENTERED);
		return 2;
	//General talents
	case TALENT_BARTERING:
		addlinetomenu(ent, "Reduces armory costs.", MENU_WHITE_CENTERED);
		return 1;
	default:	return 0;
	}
}

//Adds a menu line that tells the player what their current talent upgrade does, and
//what their next upgrade will do, if they choose to get it.
/*
void writeTalentUpgrade(edict_t *ent, int talentID, int level)
{
	if(level == 0)
	{
		addlinetomenu(ent, "-------", MENU_WHITE_CENTERED);
		return;
	}
	switch(talentID)
	{
	//Soldier talents
	case TALENT_IMP_STRENGTH:
		addlinetomenu(ent, va("+%d%%, -%0.2fx resist mult", level*10, (float)level*0.01f ), MENU_WHITE_CENTERED);
		return;
	case TALENT_IMP_RESIST:
		addlinetomenu(ent, va("+%0.2fx, -%d%% damage", (float)level*0.02f, level*10 ), MENU_WHITE_CENTERED);
		return;
	case TALENT_BLOOD_OF_ARES:
		addlinetomenu(ent, va("+%d%% per kill (max +200%%)", level*2 ), MENU_WHITE_CENTERED);
		return;
	case TALENT_BASIC_HA:
		addlinetomenu(ent, va("+%d%%", level*20 ), MENU_WHITE_CENTERED);
		return;
	case TALENT_IMP_TRAPS:
		addlinetomenu(ent, va("+%0.1f%%", level*6.6), MENU_WHITE_CENTERED);
		return;
	
	case TALENT_LONG_POWERUPS:
		switch(level)
		{
		case 0:	addlinetomenu(ent, " +0 seconds", MENU_WHITE_CENTERED);	break;
		case 1:	addlinetomenu(ent, " +3 seconds", MENU_WHITE_CENTERED);	break;
		case 2:	addlinetomenu(ent, " +5 seconds", MENU_WHITE_CENTERED);	break;
		case 3:	addlinetomenu(ent, " +8 sec, +1sec per kill!", MENU_WHITE_CENTERED);	break;
		}
		return;
	
	//Poltergeist talents
    case TALENT_MORPHING:
		addlinetomenu(ent, va("-%d%%", level*25), MENU_WHITE_CENTERED);
		return;
	case TALENT_CACO_MAST:
		addlinetomenu(ent, va("+%d%%", level*10), MENU_WHITE_CENTERED);
		return;
	case TALENT_PARA_MAST:
		addlinetomenu(ent, va("+%d units", level*10), MENU_WHITE_CENTERED);
		return;
	case TALENT_BRAIN_MAST:
		addlinetomenu(ent, va("+%d%%", level*10), MENU_WHITE_CENTERED);
		return;
	case TALENT_FLYER_MAST:
		addlinetomenu(ent, va("+%d%%, -%d%%", level*10, level*25), MENU_WHITE_CENTERED);
		return;
	case TALENT_MUTANT_MAST:
		addlinetomenu(ent, va("+%d%%", level*10), MENU_WHITE_CENTERED);
		return;
	case TALENT_MEDIC_MAST:
		addlinetomenu(ent, va("+%d%%", level*5), MENU_WHITE_CENTERED);
		return;
	case TALENT_TANK_MAST:
		addlinetomenu(ent, va("+%d%%", level*10), MENU_WHITE_CENTERED);
		return;
	case TALENT_BERSERK_MAST:
		addlinetomenu(ent, va("+%d%%", level*11), MENU_WHITE_CENTERED);
		return;
	//Vampire talents
	case TALENT_IMP_CLOAK:
		addlinetomenu(ent, va("%dpc/frame", 6-level), MENU_WHITE_CENTERED);	
		return;
	case TALENT_ARMOR_VAMP:
		addlinetomenu(ent, va("+%d%%", level*10), MENU_WHITE_CENTERED);
		return;
	case TALENT_EVIL_CURSE:
		addlinetomenu(ent, va("+%d%%", level*25), MENU_WHITE_CENTERED);
		return;
	case TALENT_SECOND_CHANCE:
		addlinetomenu(ent, va("every %0.1f minutes", 3.0-level*0.5), MENU_WHITE_CENTERED);
		return;
	case TALENT_IMP_MINDABSORB:
		addlinetomenu(ent, va("-%0.1f seconds", 0.5f * level), MENU_WHITE_CENTERED);
		return;
	//Mage talents
	case TALENT_IMP_PCR:
		addlinetomenu(ent, va(" +%d", level * 5), MENU_WHITE_CENTERED);
		return;
	case TALENT_ELEMENTAL_MASTERY:
		addlinetomenu(ent, va("+%d%% damage", level*10), MENU_WHITE_CENTERED);
		return;
	case TALENT_FROST_NOVA:
		addlinetomenu(ent, va("-%d%% movement", level * 5), MENU_WHITE_CENTERED);	
		return;
	case TALENT_IMP_MAGICBOLT:
		addlinetomenu(ent, va("+%d%% damage, %0.1fx cost", level * 4, 1.0 + (float)level * 0.2), MENU_WHITE_CENTERED);
		return;
	case TALENT_MANASHIELD:
		addlinetomenu(ent, va("%0.1f PC/damage", 4.0 - (float)level * 0.5), MENU_WHITE_CENTERED);
		return;
	//Engineer talents
	case TALENT_IMP_SUPPLY:
		addlinetomenu(ent, va("+%d/%d/%d%%", level * 5, level * 10, level * 20), MENU_WHITE_CENTERED);
		return;
	case TALENT_IMP_SENTRY:
		addlinetomenu(ent, va("+%d%% resist, -%d%% damage", level * 5, level * 2), MENU_WHITE_CENTERED);
		return;
	case TALENT_ATOMIC_ARMOR:
	addlinetomenu(ent, va("+%d%%", 25*level), MENU_WHITE_CENTERED);
		return;
	//Knight talents
	case TALENT_IMP_POWERSCREEN:
		addlinetomenu(ent, va("-%d%%", 5*level), MENU_WHITE_CENTERED);
		return;
	case TALENT_IMP_REGEN:
		addlinetomenu(ent, va("-%0.2f seconds", 0.05 * level), MENU_WHITE_CENTERED);
		return;
	case TALENT_MOBILITY:
		switch(level)
		{
		case 3:		addlinetomenu(ent, "-%0.5 seconds", MENU_WHITE_CENTERED);	break;	
		default:	addlinetomenu(ent, va("-%0.2f seconds", 0.15 * level), MENU_WHITE_CENTERED);	break;
		}
        return;
	case TALENT_DURABILITY:
		addlinetomenu(ent, va("+%d", level), MENU_WHITE_CENTERED);
		return;
	//Cleric talents
	case TALENT_BALANCESPIRIT:
		addlinetomenu(ent, va("%d%% effectiveness", level*20), MENU_WHITE_CENTERED);
		return;
	case TALENT_SUPPORT_SKILLS:
		addlinetomenu(ent, va("%d%% / %d%%", level*5, level*10), MENU_WHITE_CENTERED);
		return;
	case TALENT_ICY_CHILL:
		addlinetomenu(ent, va("%d/clvl damage, +%d%% cost", level, level * 40), MENU_WHITE_CENTERED);
		return;
	case TALENT_MEDITATION:
		switch(level)
		{
		case 1: addlinetomenu(ent, "-10%", MENU_WHITE_CENTERED);		break;
		case 2: addlinetomenu(ent, "-20%", MENU_WHITE_CENTERED);		break;
		case 3: addlinetomenu(ent, "-50%", MENU_WHITE_CENTERED);		break;
		default: addlinetomenu(ent, "no bonus", MENU_WHITE_CENTERED);	break;
		}
		return;
	case TALENT_HOLY_POWER:
		switch(level)
		{
		case 1: addlinetomenu(ent, "+15%", MENU_WHITE_CENTERED);		break;
		case 2: addlinetomenu(ent, "+30%", MENU_WHITE_CENTERED);		break;
		case 3: addlinetomenu(ent, "+50%", MENU_WHITE_CENTERED);		break;
		}
        return;
	//Weaponmaster talents
	case TALENT_BASIC_AMMO_REGEN:
		addlinetomenu(ent, va("every %d seconds", 30 - 5 * level), MENU_WHITE_CENTERED);
		return;
	case TALENT_COMBAT_EXP:
		addlinetomenu(ent, va("+%d%% / -%d%%",5 + level*5, 55-level*5), MENU_WHITE_CENTERED);
		return;
	case TALENT_IMP_HASTE:
		addlinetomenu(ent, va("+%d%%", level * 25), MENU_WHITE_CENTERED);
		return;
	case TALENT_TACTICS:
		addlinetomenu(ent, va("+%dh / +%da", level, level), MENU_WHITE_CENTERED);
		return;
	case TALENT_SIDEARMS:
		addlinetomenu(ent, va("Addtional weapons: %d", level), MENU_WHITE_CENTERED);
		return;
	//Necromancer talents
	case TALENT_DRONE_MASTERY:
		addlinetomenu(ent, va("+%d%%", level * 4), MENU_WHITE_CENTERED);
		return;
	case TALENT_DRONE_POWER:
		addlinetomenu(ent, va("+%d%% damage, -%d%% durability", level*3, level*3), MENU_WHITE_CENTERED);
		return;
	case TALENT_IMP_PLAGUE:
		addlinetomenu(ent, va("+%d%%", level*10), MENU_WHITE_CENTERED);
		return;
	case TALENT_CANNIBALISM:
		addlinetomenu(ent, va("+%0.1fx", (float)level * 0.1f), MENU_WHITE_CENTERED);
		return;
	//Shaman talents
	//case TALENT_TOTEM:
	//	addlinetomenu(ent, va("%d cubes every 5 seconds", 30-level*5), MENU_WHITE_CENTERED);
	//	return;
	case TALENT_FLAME:
		addlinetomenu(ent, va("%d%% chance", level*5), MENU_WHITE_CENTERED);
		return;
	case TALENT_ICE:
		addlinetomenu(ent, va("%d-%d damage", 10*level, 20*level), MENU_WHITE_CENTERED);		
		return;
	case TALENT_WIND:
		addlinetomenu(ent, va("%d%% chance", level*5), MENU_WHITE_CENTERED);		
		return;
	case TALENT_STONE:
        addlinetomenu(ent, va("%+d%% resist", level*5), MENU_WHITE_CENTERED);
		return;
	case TALENT_SHADOW:
		addlinetomenu(ent, va("%+d%%", level*10), MENU_WHITE_CENTERED);
		return;
	case TALENT_PEACE:
		addlinetomenu(ent, va("+%d% power cubes", level*10), MENU_WHITE_CENTERED);
		return;
	default:	return;
	}
}
*/
void openTalentMenu(edict_t *ent, int talentID)
{
	talent_t *talent	= &ent->myskills.talents.talent[getTalentSlot(ent, talentID)];
	int level			= talent->upgradeLevel;
	int lineCount = 7;//12;

	if (!ShowMenu(ent))
       return;
	clearmenu(ent);

	addlinetomenu(ent, "Talent", MENU_GREEN_CENTERED);
	addlinetomenu(ent, GetTalentString(talentID), MENU_WHITE_CENTERED);
	addlinetomenu(ent, " ", 0);

	lineCount += writeTalentDescription(ent, talentID);

	addlinetomenu(ent, " ", 0);
	//addlinetomenu(ent, "Current", MENU_GREEN_CENTERED);
	//writeTalentUpgrade(ent, talentID, level);
	addlinetomenu(ent, " ", 0);

	/*
    if(talent->upgradeLevel < talent->maxLevel)
	{
		level++;
		addlinetomenu(ent, "Next Level", MENU_GREEN_CENTERED);
		writeTalentUpgrade(ent, talentID, level);
	}
	else
	{
		addlinetomenu(ent, " ", 0);
		addlinetomenu(ent, " ", 0);
	}
	addlinetomenu(ent, " ", 0);
	*/
	
	if(talent->upgradeLevel < talent->maxLevel)
		addlinetomenu(ent, "Upgrade this talent.", -1*(talentID+1));
	else addlinetomenu(ent, " ", 0);

	addlinetomenu(ent, "Previous menu.", talentID+1);

	setmenuhandler(ent, TalentUpgradeMenu_handler);
	ent->client->menustorage.currentline = lineCount;
	showmenu(ent);
}

void generalTalentMenu(edict_t *ent, int talentID)
{
	talent_t *talent	= &ent->myskills.talents.talent[getTalentSlot(ent, talentID)];
	int level			= talent->upgradeLevel;
	int lineCount = 7;//12;

	if (!ShowMenu(ent))
       return;
	clearmenu(ent);

	addlinetomenu(ent, "Talent", MENU_GREEN_CENTERED);
	addlinetomenu(ent, GetTalentString(talentID), MENU_WHITE_CENTERED);
	addlinetomenu(ent, " ", 0);

	lineCount += writeTalentDescription(ent, talentID);

	addlinetomenu(ent, " ", 0);
	//addlinetomenu(ent, "Current", MENU_GREEN_CENTERED);
	//writeTalentUpgrade(ent, talentID, level);
	addlinetomenu(ent, " ", 0);

	/*
    if(talent->upgradeLevel < talent->maxLevel)
	{
		level++;
		addlinetomenu(ent, "Next Level", MENU_GREEN_CENTERED);
		writeTalentUpgrade(ent, talentID, level);
	}
	else
	{
		addlinetomenu(ent, " ", 0);
		addlinetomenu(ent, " ", 0);
	}
	addlinetomenu(ent, " ", 0);
	*/
	
	if(talent->upgradeLevel < talent->maxLevel)
		addlinetomenu(ent, "Upgrade this talent.", -1*(talentID+1));
	else addlinetomenu(ent, " ", 0);

	addlinetomenu(ent, "Previous menu.", talentID+1);

	setmenuhandler(ent, GenTalentUpgradeMenu_handler);
	ent->client->menustorage.currentline = lineCount;
	showmenu(ent);
}

//****************************************
//*********** Main Talent Menu ***********
//****************************************

void openTalentMenu_handler(edict_t *ent, int option)
{
	switch(option)
	{
	case 9999:	//Exit
		{
			closemenu(ent);
			return;
		}
	default: openTalentMenu(ent, option);
	}
}

void upgradingBartering_handler (edict_t *ent, int option)
{
	switch(option)
	{
	case 1:
	{
	if (ent->myskills.talents.talentPoints < 1){
    gi.cprintf(ent, PRINT_HIGH, "You do not have enough talent points.\n");
	return;}

	if (ent->myskills.talents.BarteringLevel > 2){
	gi.cprintf(ent, PRINT_HIGH, "You can not upgrade this talent any further.\n");
	return;}

    ent->myskills.talents.BarteringLevel++;
	ent->myskills.talents.talentPoints--;
	gi.cprintf(ent, PRINT_HIGH, va("Bartering upgraded to level %d/3.\n", ent->myskills.talents.BarteringLevel));
	gi.cprintf(ent, PRINT_HIGH, va("Talent points remaining: %d\n", ent->myskills.talents.talentPoints));
	savePlayer(ent);
	return;
	}
	default: OpenGeneralTalentMenu(ent, option);
	}
}

void upgradingVital_handler (edict_t *ent, int option)
{
	switch(option)
	{
	case 1:
	{
	if (ent->myskills.talents.talentPoints < 1){
    gi.cprintf(ent, PRINT_HIGH, "You do not have enough talent points.\n");
	return;}

	if (ent->myskills.talents.VitLevel > 2){
	gi.cprintf(ent, PRINT_HIGH, "You can not upgrade this talent any further.\n");
	return;}

    ent->myskills.talents.VitLevel++;
	ent->myskills.talents.talentPoints--;
	gi.cprintf(ent, PRINT_HIGH, va("Imp. Vitality upgraded to level %d/3.\n", ent->myskills.talents.VitLevel));
	gi.cprintf(ent, PRINT_HIGH, va("Talent points remaining: %d\n", ent->myskills.talents.talentPoints));
	savePlayer(ent);
	return;
	}
	default: OpenGeneralTalentMenu(ent, option);
	}
}

void upgradingAmmo_handler (edict_t *ent, int option)
{
	switch(option)
	{
	case 1:
	{
	if (ent->myskills.talents.talentPoints < 1){
    gi.cprintf(ent, PRINT_HIGH, "You do not have enough talent points.\n");
	return;}

	if (ent->myskills.talents.AmmoLevel > 2){
	gi.cprintf(ent, PRINT_HIGH, "You can not upgrade this talent any further.\n");
	return;}

    ent->myskills.talents.AmmoLevel++;
	ent->myskills.talents.talentPoints--;
	gi.cprintf(ent, PRINT_HIGH, va("Ammo Pickups upgraded to level %d/3.\n", ent->myskills.talents.AmmoLevel));
	gi.cprintf(ent, PRINT_HIGH, va("Talent points remaining: %d\n", ent->myskills.talents.talentPoints));
	savePlayer(ent);
	return;
	}
	default: OpenGeneralTalentMenu(ent, option);
	}
}

void upgradingSuper_handler (edict_t *ent, int option)
{
	switch(option)
	{
	case 1:
	{
	if (ent->myskills.talents.talentPoints < 1){
    gi.cprintf(ent, PRINT_HIGH, "You do not have enough talent points.\n");
	return;}

	if (ent->myskills.talents.SuperLevel > 2){
	gi.cprintf(ent, PRINT_HIGH, "You can not upgrade this talent any further.\n");
	return;}

    ent->myskills.talents.SuperLevel++;
	ent->myskills.talents.talentPoints--;
	gi.cprintf(ent, PRINT_HIGH, va("Superiority upgraded to level %d/3.\n", ent->myskills.talents.SuperLevel));
	gi.cprintf(ent, PRINT_HIGH, va("Talent points remaining: %d\n", ent->myskills.talents.talentPoints));
	savePlayer(ent);
	return;
	}
	default: OpenGeneralTalentMenu(ent, option);
	}
}

void upgradingGreed_handler (edict_t *ent, int option)
{
	switch(option)
	{
	case 1:
	{
	if (ent->myskills.talents.talentPoints < 1){
    gi.cprintf(ent, PRINT_HIGH, "You do not have enough talent points.\n");
	return;}

	if (ent->myskills.talents.GreedLevel > 2){
	gi.cprintf(ent, PRINT_HIGH, "You can not upgrade this talent any further.\n");
	return;}

    ent->myskills.talents.GreedLevel++;
	ent->myskills.talents.talentPoints--;
	gi.cprintf(ent, PRINT_HIGH, va("Greed upgraded to level %d/3.\n", ent->myskills.talents.GreedLevel));
	gi.cprintf(ent, PRINT_HIGH, va("Talent points remaining: %d\n", ent->myskills.talents.talentPoints));
	savePlayer(ent);
	return;
	}
	default: OpenGeneralTalentMenu(ent, option);
	}
}

void talentDESC_Bartering (edict_t *ent)
{
//	talent_t	*talent;
//	char		buffer[30];
//	int			i;
//	int         talentLevel;
    
	if (!ShowMenu(ent))
       return;
	clearmenu(ent);

	// menu header
	addlinetomenu(ent, "Talent", MENU_GREEN_CENTERED);
	addlinetomenu(ent, "Bartering", MENU_WHITE_CENTERED);
	addlinetomenu(ent, " ", 0);

	addlinetomenu(ent, "Reduces the armory cost.", MENU_WHITE_CENTERED);

	addlinetomenu(ent, "", 0);
	addlinetomenu(ent, "", 0);

	addlinetomenu(ent, "Upgrade this talent.", 1);
	addlinetomenu(ent, "Previous Menu.", 2);

	setmenuhandler(ent, upgradingBartering_handler);

	ent->client->menustorage.currentline = 8;

	showmenu(ent);
}

void talentDESC_Vital (edict_t *ent)
{
//	talent_t	*talent;
//	char		buffer[30];
//	int			i;
//	int         talentLevel;
    
	if (!ShowMenu(ent))
       return;
	clearmenu(ent);

	// menu header
	addlinetomenu(ent, "Talent", MENU_GREEN_CENTERED);
	addlinetomenu(ent, "Imp. Vitality", MENU_WHITE_CENTERED);
	addlinetomenu(ent, " ", 0);

    addlinetomenu(ent, "Increases your levelup", MENU_WHITE_CENTERED);
	addlinetomenu(ent, "health and armor bonus.", MENU_WHITE_CENTERED);

	addlinetomenu(ent, "", 0);
	addlinetomenu(ent, "", 0);

	addlinetomenu(ent, "Upgrade this talent.", 1);
	addlinetomenu(ent, "Previous Menu.", 2);

	setmenuhandler(ent, upgradingVital_handler);

	ent->client->menustorage.currentline = 9;

	showmenu(ent);
}

void talentDESC_Ammo (edict_t *ent)
{
//	talent_t	*talent;
//	char		buffer[30];
//	int			i;
//	int         talentLevel;
    
	if (!ShowMenu(ent))
       return;
	clearmenu(ent);

	// menu header
	addlinetomenu(ent, "Talent", MENU_GREEN_CENTERED);
	addlinetomenu(ent, "Ammo Pickups", MENU_WHITE_CENTERED);
	addlinetomenu(ent, " ", 0);

    addlinetomenu(ent, "Increases the value", MENU_WHITE_CENTERED);
	addlinetomenu(ent, "of ammo you pick up.", MENU_WHITE_CENTERED);

	addlinetomenu(ent, "", 0);
	addlinetomenu(ent, "", 0);

	addlinetomenu(ent, "Upgrade this talent.", 1);
	addlinetomenu(ent, "Previous Menu.", 2);

	setmenuhandler(ent, upgradingAmmo_handler);

	ent->client->menustorage.currentline = 9;

	showmenu(ent);
}

void talentDESC_Super (edict_t *ent)
{
//	talent_t	*talent;
//	char		buffer[30];
//	int			i;
//	int         talentLevel;
    
	if (!ShowMenu(ent))
       return;
	clearmenu(ent);

	// menu header
	addlinetomenu(ent, "Talent", MENU_GREEN_CENTERED);
	addlinetomenu(ent, "Superiority", MENU_WHITE_CENTERED);
	addlinetomenu(ent, " ", 0);

	addlinetomenu(ent, "Increases damage and", MENU_WHITE_CENTERED);
	addlinetomenu(ent, "resistance to monsters.", MENU_WHITE_CENTERED);

	addlinetomenu(ent, "", 0);
	addlinetomenu(ent, "", 0);

	addlinetomenu(ent, "Upgrade this talent.", 1);
	addlinetomenu(ent, "Previous Menu.", 2);

	setmenuhandler(ent, upgradingSuper_handler);

	ent->client->menustorage.currentline = 9;

	showmenu(ent);
}

void talentDESC_Greed (edict_t *ent)
{
//	talent_t	*talent;
//	char		buffer[30];
//	int			i;
//	int         talentLevel;
    
	if (!ShowMenu(ent))
       return;
	clearmenu(ent);

	// menu header
	addlinetomenu(ent, "Talent", MENU_GREEN_CENTERED);
	addlinetomenu(ent, "Greed", MENU_WHITE_CENTERED);
	addlinetomenu(ent, " ", 0);

	addlinetomenu(ent, "Increases the chance that", MENU_WHITE_CENTERED);
	addlinetomenu(ent, "the enemy you kill will.", MENU_WHITE_CENTERED);
	addlinetomenu(ent, "drop a rune.", MENU_WHITE_CENTERED);

	addlinetomenu(ent, "", 0);
	addlinetomenu(ent, "", 0);

	addlinetomenu(ent, "Upgrade this talent.", 1);
	addlinetomenu(ent, "Previous Menu.", 2);

	setmenuhandler(ent, upgradingGreed_handler);

	ent->client->menustorage.currentline = 10;

	showmenu(ent);
}

void generalTalentMenu_handler(edict_t *ent, int option)
{
	switch(option)
	{
	case 1://bartering
		{
		    talentDESC_Bartering(ent);
			return;
		}
	case 2://imp vital
		{
		    talentDESC_Vital(ent);
			return;
		}
	case 3://ammo pickups
		{
		    talentDESC_Ammo(ent);
			return;
		}
	case 4://superior
		{
		    talentDESC_Super(ent);
			return;
		}
	case 5://greed
		{
		    talentDESC_Greed(ent);
			return;
		}
	case 9999:	//Exit
		{
			closemenu(ent);
			return;
		}
	default: generalTalentMenu(ent, option);
	}
}

void OpenTalentUpgradeMenu (edict_t *ent, int lastline)
{
	talent_t	*talent;
	char		buffer[30];
	int			i;
    
	if (!ShowMenu(ent))
       return;
	clearmenu(ent);

	// menu header
	addlinetomenu(ent, "Class Talents", MENU_GREEN_CENTERED);
	addlinetomenu(ent, " ", 0);

	for (i = 0; i < ent->myskills.talents.count; i++)
	{
		talent = &ent->myskills.talents.talent[i];

		//create menu string
		strcpy(buffer, GetTalentString(talent->id));
		strcat(buffer, ":");
		padRight(buffer, 15);

		addlinetomenu(ent, va("%d. %s %d/%d", i+1, buffer, talent->upgradeLevel, talent->maxLevel), talent->id);
	}

	// menu footer
	addlinetomenu(ent, " ", 0);
	addlinetomenu(ent, va("You have %d talent points.", ent->myskills.talents.talentPoints), 0);
	addlinetomenu(ent, " ", 0);

	addlinetomenu(ent, "Exit", 9999);
	setmenuhandler(ent, openTalentMenu_handler);

	if (!lastline)	ent->client->menustorage.currentline = ent->myskills.talents.count + 6;
	else			ent->client->menustorage.currentline = lastline + 2;

	showmenu(ent);

	// try to shortcut to chat-protect mode
	if (ent->client->idle_frames < CHAT_PROTECT_FRAMES-51)
		ent->client->idle_frames = CHAT_PROTECT_FRAMES-51;
}

void OpenGeneralTalentMenu (edict_t *ent, int lastline)
{
	talent_t	*talent;
	char		buffer[30];
	int			i;
//	int         talentLevel;
    
	if (!ShowMenu(ent))
       return;
	clearmenu(ent);

	// menu header
	addlinetomenu(ent, "General Talents", MENU_GREEN_CENTERED);
	addlinetomenu(ent, " ", 0);

	for (i = 0; i < ent->myskills.talents.count; i++)
	{
		talent = &ent->myskills.talents.talent[i];

		//create menu string
		strcpy(buffer, GetTalentString(talent->id));
		strcat(buffer, ":");
		padRight(buffer, 14);
	}

	addlinetomenu(ent, va("1. Bartering:      %d/3", ent->myskills.talents.BarteringLevel), 1);
	addlinetomenu(ent, va("2. Imp. Vitality:  %d/3", ent->myskills.talents.VitLevel), 2);
	addlinetomenu(ent, va("3. Ammo Pickups:   %d/3", ent->myskills.talents.AmmoLevel), 3);
	addlinetomenu(ent, va("4. Superiority:    %d/3", ent->myskills.talents.SuperLevel), 4);
	addlinetomenu(ent, va("5. Greed:          %d/3", ent->myskills.talents.GreedLevel), 5);

	// menu footer
	addlinetomenu(ent, " ", 0);
	addlinetomenu(ent, va("You have %d talent points.", ent->myskills.talents.talentPoints), 0);
	addlinetomenu(ent, " ", 0);

	addlinetomenu(ent, "Exit", 9999);
	setmenuhandler(ent, generalTalentMenu_handler);

//	if (!lastline)	ent->client->menustorage.currentline = ent->myskills.talents.count + 6;
	ent->client->menustorage.currentline = 11;

	showmenu(ent);

	// try to shortcut to chat-protect mode
	if (ent->client->idle_frames < CHAT_PROTECT_FRAMES-51)
		ent->client->idle_frames = CHAT_PROTECT_FRAMES-51;
}