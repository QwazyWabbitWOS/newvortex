#include "g_local.h"

//************************************************************************************************
//			Indexing functions
//************************************************************************************************

//Takes the class string and returns the index
int getClassNum(char *newclass)
{
	if (Q_strcasecmp(newclass, "Soldier") == 0)
		return CLASS_SOLDIER;
	else if (Q_strcasecmp(newclass, "Mage") == 0)
		return CLASS_MAGE;
	else if (Q_strcasecmp(newclass, "Necromancer") == 0)
		return CLASS_NECROMANCER;
	else if (Q_strcasecmp(newclass, "Vampire") == 0)
		return CLASS_VAMPIRE;
	else if (Q_strcasecmp(newclass, "Engineer") == 0)
		return CLASS_ENGINEER;
	else if (Q_strcasecmp(newclass, "Poltergeist") == 0)
		return CLASS_POLTERGEIST;
	else if (Q_strcasecmp(newclass, "Knight") == 0)
		return CLASS_KNIGHT;
	else if (Q_strcasecmp(newclass, "Cleric") == 0)
		return CLASS_CLERIC;
	else if (Q_strcasecmp(newclass, "Shaman") == 0)
		return CLASS_SHAMAN;
	else if (Q_strcasecmp(newclass, "Alien") == 0)
		return CLASS_ALIEN;
	else if ((Q_strcasecmp(newclass, "Weapon Master") == 0) || (Q_strcasecmp(newclass, "WeaponMaster") == 0))
		return CLASS_WEAPONMASTER;
	return 0;
}

//************************************************************************************************
//			String functions
//************************************************************************************************

void padRight(char *String, int numChars)
{
	//Pads a string with spaces at the end, until the length of the string is 'numChars'
	int i;
	for (i = strlen(String); i < numChars; ++i)
		String[i] = ' ';
	String[numChars] = 0;
}

//************************************************************************************************

char GetRandomChar (void) 
{
	switch (GetRandom(1, 3))
	{
	case 1: return (GetRandom(48, 57));
	case 2: return (GetRandom(65, 90));
	case 3: return (GetRandom(97, 113));
	}
	return ' ';
}

//************************************************************************************************

char *GetRandomString (int len)
{
	int i;
	char *s;

	s = (char *)  gi.TagMalloc (len*sizeof(char), TAG_GAME);
	for (i=0; i<len-1; i++) {
		s[i] = GetRandomChar();
	}
	s[i] = '\0'; // terminating char
	return s;
}

//************************************************************************************************
//	Command list (help menu)
//************************************************************************************************

void PrintCommands(edict_t *ent)
{
    //Spam the user with a huge amount of text :)
	gi.cprintf(ent, PRINT_HIGH, "Vortex Command List:\n\n");
	gi.cprintf(ent, PRINT_HIGH, va("Version %s\n\n", VRX_VERSION));

	gi.cprintf(ent, PRINT_HIGH, "*** Skills ***\n\n");

	gi.cprintf(ent, PRINT_HIGH, "aura_holyfreeze\naura_shock\naura_salvation\nbless [self]\ncacodemon\ncripple\ncurse\nampdamage\nironmaiden\nweaken\namnesia\ndecoy\ndetpipes\nforcewall [solid]\ngravjump\nguardian\n+hook\nheal [self]\nlaser[s] [remove]\nmagicbolt\nmagmine\nminisentry [remove]\nmonster [help/hunt/remove/select/stand]\nnova\n+parasite\nsentry [remove/rotate]\nspell_boost\nspell_bomb [area/forward]\nspell_corpseexplode\nspell_stealammo\n+superspeed\nsupplystation\nteleport_fwd\n+thrust\nuse [power screen/holywater/potion/gravityboots]\n\n");
	
	gi.cprintf(ent, PRINT_HIGH, "*** Standard Commands ***\n\n");
	
	gi.cprintf(ent, PRINT_HIGH, "owner <name>\nwhois <playername>\nflashlight\nrune\nteam\ntrade\ntogglesecondary\nuse tball [self]\nupgrade_ability\nupgrade_weapon\nvote\nvrx_password\nvrxarmory\nvrxhelp\nvrxid\nvrxinfo\nvrxrespawn\n\n");
}

//************************************************************************************************
//	Armoury Strings (stuff you can buy at the armory)
//************************************************************************************************

char *GetArmoryItemString (int purchase_number)
{
	switch(purchase_number)
	{
	case 1:		return "Shotgun";
	case 2:		return "Super Shotgun";
	case 3:		return "Machinegun";
	case 4:		return "Chaingun";
	case 5:		return "Grenade Launcher";
	case 6:		return "Rocket Launcher";
	case 7:		return "Hyperblaster";
	case 8:		return "Railgun";
	case 9:		return "BFG10K";
	case 10:	return "20mm Cannon";
	case 11:	return "Bullets";
	case 12:	return "Shells";
	case 13:	return "Cells";
	case 14:	return "Grenades";
	case 15:	return "Rockets";
	case 16:	return "Slugs";
	case 17:	return "T-Balls";
	case 18:	return "Power Cubes";
	case 19:	return "Body Armor";
	case 20:	return "Health Potions";
	case 21:	return "Holy Water";
	case 22:	return "Anti-grav Boots";
	case 23:	return "Fire Resistant Clothing";
	case 24:	return "Auto-Tball";
	case 25:	return "Ability Rune";
	case 26:	return "Weapon Rune";
	case 27:    return "Combo Rune";
	case 28:	return "Reset Abilities/Weapons";

	default:	return "<BAD ITEM NUMBER>";
	}
}

//************************************************************************************************
//	Weapon Strings (mods and weapon names)
//************************************************************************************************

char *GetShortWeaponString (int weapon_number)
{
	//Returns a shorter form of a weapon, for the rune menu
	switch(weapon_number)
	{
	case WEAPON_BLASTER:			return "Blaster";
	case WEAPON_SHOTGUN:			return "SG";
	case WEAPON_SUPERSHOTGUN:		return "SSG";
	case WEAPON_MACHINEGUN:			return "MG";
	case WEAPON_CHAINGUN:			return "CG";
	case WEAPON_GRENADELAUNCHER:	return "GL";
	case WEAPON_ROCKETLAUNCHER:		return "RL";
	case WEAPON_HYPERBLASTER:		return "HB";
	case WEAPON_RAILGUN:			return "RG";
	case WEAPON_BFG10K:				return "BFG10K";
	case WEAPON_SWORD:				return "Sword";
	case WEAPON_20MM:				return "20mm";
	case WEAPON_HANDGRENADE:		return "HG";
	default:						return "<BAD WEAPON NUMBER>";
	}
}

//************************************************************************************************

char *GetWeaponString (int weapon_number)
{
	switch(weapon_number)
	{
	case WEAPON_BLASTER:			return "Blaster";
	case WEAPON_SHOTGUN:			return "Shotgun";
	case WEAPON_SUPERSHOTGUN:		return "Super Shotgun";
	case WEAPON_MACHINEGUN:			return "Machinegun";
	case WEAPON_CHAINGUN:			return "Chaingun";
	case WEAPON_GRENADELAUNCHER:	return "Grenade Launcher";
	case WEAPON_ROCKETLAUNCHER:		return "Rocket Launcher";
	case WEAPON_HYPERBLASTER:		return "Hyperblaster";
	case WEAPON_RAILGUN:			return "Railgun";
	case WEAPON_BFG10K:				return "BFG10K";
	case WEAPON_SWORD:				return "Sword";
	case WEAPON_20MM:				return "20mm Cannon";
	case WEAPON_HANDGRENADE:		return "Hand Grenade";
	default:						return "<BAD WEAPON NUMBER>";
	}
}

//************************************************************************************************

char *GetModString (int weapon_number, int mod_number)
{
	switch(mod_number)
	{
	case 0:		return "Damage";
	case 1:
		switch(weapon_number)
		{
		case WEAPON_BLASTER:			return "Stun";
		case WEAPON_SHOTGUN:			return "Strike";
		case WEAPON_SUPERSHOTGUN:		return "Range";
		case WEAPON_MACHINEGUN:			return "Pierce";
		case WEAPON_CHAINGUN:			return "Spin";		
		case WEAPON_GRENADELAUNCHER:	return "Radius";
		case WEAPON_ROCKETLAUNCHER:		return "Radius";
		case WEAPON_HYPERBLASTER:		return "Stun";
		case WEAPON_RAILGUN:			return "Pierce";
		case WEAPON_BFG10K:				return "Duration";
		case WEAPON_SWORD:				return "Forging";
		case WEAPON_20MM:				return "Range";
		case WEAPON_HANDGRENADE:		return "Range";
		default:						return "<BAD WEAPON NUMBER>";
		}
	case 2:
		switch(weapon_number)
		{
		case WEAPON_BLASTER:			return "Speed";
		case WEAPON_SHOTGUN:			return "Pellets";
		case WEAPON_SUPERSHOTGUN:		return "Pellets";
		case WEAPON_MACHINEGUN:			return "Tracers";
		case WEAPON_CHAINGUN:			return "Tracers";
		case WEAPON_GRENADELAUNCHER:	return "Range";
		case WEAPON_ROCKETLAUNCHER:		return "Speed";
		case WEAPON_HYPERBLASTER:		return "Speed";
		case WEAPON_RAILGUN:			return "Burn";
		case WEAPON_BFG10K:				return "Speed";
		case WEAPON_SWORD:				return "Length";
		case WEAPON_20MM:				return "Recoil";
		case WEAPON_HANDGRENADE:		return "Radius";
		default:						return "<BAD WEAPON NUMBER>";
		}
	case 3:
		switch(weapon_number)
		{
		case WEAPON_BLASTER:			return "Trails";
		case WEAPON_SHOTGUN:			return "Spread";
		case WEAPON_SUPERSHOTGUN:		return "Spread";
		case WEAPON_MACHINEGUN:			return "Spread";
		case WEAPON_CHAINGUN:			return "Spread";
		case WEAPON_GRENADELAUNCHER:	return "Trails";
		case WEAPON_ROCKETLAUNCHER:		return "Trails";
		case WEAPON_HYPERBLASTER:		return "Light";
		case WEAPON_RAILGUN:			return "Trails";
		case WEAPON_BFG10K:				return "Slide";
		case WEAPON_SWORD:				return "Burn";
		case WEAPON_20MM:				return "Caliber";
		case WEAPON_HANDGRENADE:		return "Trails";
		default:						return "<BAD WEAPON NUMBER>";
		}
	case 4:		return "Noise/Flash";
	default:	return "<BAD WEAPON MOD NUMBER>";
	}
}

//************************************************************************************************
//	Class Strings
//************************************************************************************************

char *GetClassString (int class_num)
{
	switch (class_num)
	{
	case CLASS_SOLDIER:		return "Soldier";
	case CLASS_MAGE:		return "Mage";
	case CLASS_NECROMANCER:	return "Necromancer";
	case CLASS_VAMPIRE:		return "Vampire";
	case CLASS_ENGINEER:	return "Engineer";
	case CLASS_POLTERGEIST: return "Poltergeist";
	case CLASS_KNIGHT:		return "Knight";
	case CLASS_CLERIC:		return "Cleric";
	case CLASS_WEAPONMASTER:return "Weapon Master";
	case CLASS_SHAMAN:		return "Shaman";
	case CLASS_ALIEN:		return "Alien";
	default:				return "Unknown";
	}
}

//************************************************************************************************
//	"Talent Name" Strings
//************************************************************************************************

char *GeneralTalentString(int talent_ID)
{
	switch(talent_ID)
	{
	//Soldier Talents
	case TALENT_BARTERING:		    return "Bartering";
	case TALENT_TACTICS:			return "Imp. Vit";
	case TALENT_BASIC_HA:		    return "Ammo Efficiency";
	case TALENT_SUPERIORITY:		return "Superiority";
	case TALENT_RETALIATION:		return "Retaliation";
	default: return va("talent ID = %d", talent_ID);
	}
}

char *GetTalentString(int talent_ID)
{
	switch(talent_ID)
	{
	//Soldier Talents
	case TALENT_IMP_STRENGTH:		return "Imp. Strength";
	case TALENT_IMP_RESIST:			return "Imp. Resist";
	case TALENT_BLOOD_OF_ARES:		return "Blood of Ares";
	case TALENT_BOMBARDIER:			return "Bombardier";
	case TALENT_RETALIATION:        return "Retaliation";
	//Poltergeist Talents
	case TALENT_MORPHING:			return "Morphing";//decino: change to longer slash range for para/mutant/tank/berserker
	case TALENT_MORE_AMMO:			return "More Ammo";//decino: not sure if this is usefull with polt ammo regen
	case TALENT_SUPERIORITY:		return "Imp. Metabolism";//decino: don't get confused, this is now a different talent than orig. superiority
	case TALENT_PACK_ANIMAL:		return "Pack Animal";
	case TALENT_SECOND_CHANCE:		return "Imp. Appendage";
	//Vampire Talents
	case TALENT_IMP_CLOAK:			return "Imp. Cloak";//decino: sucks, needs a replacement or removal
	case TALENT_ARMOR_VAMP:			return "Armor Vampire";
	case TALENT_IMP_MINDABSORB:		return "Mind Control";
	case TALENT_CANNIBALISM:		return "Cannibalism";
	case TALENT_FATAL_WOUND:		return "Fatal Wound";
	//Mage Talents
	case TALENT_ICE_BOLT:			return "Ice Bolt";//decino: replace, add ice bolt to ability list
	case TALENT_MEDITATION:			return "Meditation";
	case TALENT_FROST_NOVA:			return "Frost Nova";
	case TALENT_IMP_MAGICBOLT:		return "Imp. Magicbolt";//decino: too simple
	case TALENT_MANASHIELD:			return "Mana Shield";
	//Engineer Talents
	case TALENT_LASER_PLATFORM:		return "Laser Platform";//decino: worthless, improve or replace
	case TALENT_ALARM:				return "Laser Trap";
	case TALENT_RAPID_ASSEMBLY:		return "Rapid Assembly";
	case TALENT_PRECISION_TUNING:	return "Fire Wall";
	case TALENT_STORAGE_UPGRADE:	return "Storage Upgrade";
	//Knight Talents
	case TALENT_REPEL:				return "Repel";
	case TALENT_MAG_BOOTS:			return "Mag Boots";
	case TALENT_LEAP_ATTACK:		return "Leap Attack";
	case TALENT_MOBILITY:			return "Mobility";
	case TALENT_DURABILITY:         return "Lancing";
	//Cleric Talents
	case TALENT_BALANCESPIRIT:		return "Balance Spirit";//decino: too shitty, make it combine spirits without having the opposite one upgraded; lv 3: same level as opposite spirit
	case TALENT_HOLY_GROUND:		return "Holy Ground";
	case TALENT_UNHOLY_GROUND:		return "Unholy Ground";
	case TALENT_PURGE:				return "Purge";
	case TALENT_BOOMERANG:			return "Boomerang";
	//Weapon Master Talents
	case TALENT_BASIC_AMMO_REGEN:	return "Ammo Regen";
	case TALENT_COMBAT_EXP:			return "Combat Exp.";
	case TALENT_SIDEARMS:			return "Sidearms";
	case TALENT_TACTICS:            return "Poison Rounds";
	case TALENT_OVERLOAD:           return "Ammo Thief";
	//Necromancer Talents
	case TALENT_CHEAPER_CURSES:		return "Cheaper Curses";
	case TALENT_CORPULENCE:			return "Corpulence";
	case TALENT_LIFE_TAP:			return "Life Tap";
	case TALENT_DIM_VISION:			return "Dim Vision";
	case TALENT_FLIGHT:				return "Flight";//decino: replace with something usefull?
	//Shaman Talents
	case TALENT_PEACE:              return "Short Temper";
	case TALENT_ICE:				return "Obsidian";//decino: replace with erosion talent (earth+air)
	case TALENT_WIND:				return "Erosion";//decino: replace with obsidian talent (fire+water)
	case TALENT_SHADOW:				return "Twilight";//decino: replace, fury talent?
	case TALENT_TOTEM:              return "Totemic Focus";
	//Alien Talents
	case TALENT_PHANTOM_OBSTACLE:	return "Hidden Obstacle";
	case TALENT_SUPER_HEALER:		return "Super Healer";//decino: should heal armor and healer should always heal 150% health w/o talent
	case TALENT_PHANTOM_COCOON:		return "Phantom Cocoon";
	case TALENT_SWARMING:			return "Swarming";//decino: shouldn't reduce damage, spores suck already
	case TALENT_EXPLODING_BODIES:	return "Exploding Body";
	//General talents
	case TALENT_BARTERING:          return "Bartering";
	default: return va("talent ID = %d", talent_ID);
	}
}

//************************************************************************************************
//	Ability Strings
//************************************************************************************************

char *GetAbilityString (int ability_number)
{
	switch (ability_number)
	{
	case VITALITY:			return	"Vitality";
	case REGENERATION:		return	"Regen";
	case RESISTANCE:		return	"Resist";
	case STRENGTH:			return	"Strength";
	case HASTE:				return	"Haste";
	case VAMPIRE:			return	"Life Steal";
	case JETPACK:			return	"Jetpack";
	case CLOAK:				return	"Cloak";
	case WEAPON_KNOCK:		return	"Knock Weapon";
	case ARMOR_UPGRADE:		return	"Armor Upgrade";
	case AMMO_STEAL:		return	"Aftermath";
	case ID:				return	"ID";
	case MAX_AMMO:			return	"Max Ammo";
	case GRAPPLE_HOOK:		return	"Grapple Hook";
	case SUPPLY_STATION:	return	"Supply Station";
	case CREATE_QUAD:		return	"Auto-Quad";
	case CREATE_INVIN:		return	"Auto-Invin";
	case POWER_SHIELD:		return	"Power Shield";
	case CORPSE_EXPLODE:	return	"Detonate Body";
	case GHOST:				return	"Ghost";
	case SALVATION:			return	"Salvation Aura";
	case FORCE_WALL:		return	"Force Wall";
	case AMMO_REGEN:		return	"Ammo Regen";
	case POWER_REGEN:		return	"PC Regen";
	case BUILD_LASER:		return	"Lasers";
	case HA_PICKUP:			return	"H/A Pickup";
	case BUILD_SENTRY:		return	"Sentry Guns";
	case BOOST_SPELL:		return	"Boost Spell";
	case BLOOD_SUCKER:		return	"Parasite";
	case PROXY:				return	"Proxy Grenade";
	case MONSTER_SUMMON:	return	"Monster Summon";
	case SUPER_SPEED:		return	"Sprint";
	case ARMOR_REGEN:		return	"Armor Regen";
	case BOMB_SPELL:		return	"Bomb Spell";
	case LIGHTNING:			return	"Chain Lightning";
	case DECOY:				return	"Mirror";
	case HOLY_FREEZE:		return	"HolyFreeze Aura";
	case WORLD_RESIST:		return	"World Resist";
	case BULLET_RESIST:		return	"Bullet Resist";
	case SHELL_RESIST:		return	"Shell Resist";
	case ENERGY_RESIST:		return	"Energy Resist";
	case PIERCING_RESIST:	return	"Piercing Resist";
	case SPLASH_RESIST:		return	"Splash Resist";
	case CACODEMON:			return	"Cacodemon";
	case PLAGUE:			return	"Plague";
	case FLESH_EATER:		return	"Corpse Eater";
	case HELLSPAWN:			return	"Hell Spawn";
	case BRAIN:				return	"Brain";
	case BEAM:				return	"Beam";
	case MAGMINE:			return	"Mag Mine";
	case CRIPPLE:			return	"Cripple";
	case MAGICBOLT:			return	"Magic Bolt";
	case TELEPORT:			return	"Teleport";
	case NOVA:				return	"Nova";
	case EXPLODING_ARMOR:	return	"Exploding Armor";
	case MIND_ABSORB:		return	"Mind Absorb";
	case LIFE_DRAIN:		return	"Life Drain";
	case AMP_DAMAGE:		return	"Amp Damage";
	case CURSE:				return	"Curse";
	case WEAKEN:			return	"Weaken";
	case AMNESIA:			return	"Amnesia";
	case BLESS:				return	"Bless";
	case HEALING:			return	"Healing";
	case AMMO_UPGRADE:		return	"Ammo Efficiency";
	case YIN:				return	"Yin Spirit";
	case YANG:				return	"Yang Spirit";
	case FLYER:				return	"Flyer";
	case SPIKE:				return  "Spike";
	case MUTANT:			return  "Mutant";
	case MORPH_MASTERY:		return	"Morph Mastery";
	case NAPALM:			return	"Napalm";
	case TANK:				return	"Tank";
	case MEDIC:				return	"Medic";
	case BERSERK:			return	"Berserker";
	case METEOR:			return	"Meteor";
	case AUTOCANNON:		return	"Auto Cannon";
	case HAMMER:			return	"Blessed Hammer";
	case BLACKHOLE:			return	"Wormhole";
	case FIRE_TOTEM:		return	"Fire Totem";
	case WATER_TOTEM:		return	"Water Totem";
	case AIR_TOTEM:			return	"Air Totem";
	case EARTH_TOTEM:		return	"Earth Totem";
	case DARK_TOTEM:		return	"Darkness Totem";
	case NATURE_TOTEM:		return	"Nature Totem";
	case FURY:				return	"Fury";
	case TOTEM_MASTERY:		return	"Totem Mastery";
	case SHIELD:			return	"Shield";
	case CALTROPS:			return	"Caltrops";
	case SPIKE_GRENADE:		return	"Spike Grenade";
	case DETECTOR:			return	"Detector";
	case CONVERSION:		return	"Conversion";
	case DEFLECT:			return	"Deflect";
	case SCANNER:			return	"Scanner";
	case EMP:				return	"EMP";
	case DOUBLE_JUMP:		return	"Double Jump";
	case LOWER_RESIST:		return	"Lower Resist";
	case FIREBALL:			return	"Fireball";
	case PLASMA_BOLT:		return	"Plasma Bolt";
	case LIGHTNING_STORM:	return	"Lightning Storm";
	case MIRV:				return	"Mirv Grenade";
	case SPIKER:			return	"Spiker";
	case OBSTACLE:			return	"Obstacle";
	case HEALER:			return	"Healer";
	case GASSER:			return	"Gasser";
	case SPORE:				return	"Spore";
	case ACID:				return	"Acid";
	case COCOON:			return	"Cocoon";
	default:				return	":)";
	}
}

//************************************************************************************************
//	Rune Strings
//************************************************************************************************

char *GetRuneValString(item_t *rune)
{
	int level = rune->itemLevel;
    
	switch(rune->itemtype)
	{
	case ITEM_WEAPON:
		{
            switch(level / 2)
			{
			case 0:	 return "Worthless";
			case 1:  return "Cracked";
			case 2:  return "Chipped";
			case 3:  return "Flawed";
			case 4:  return "Normal";
			case 5:  return "Good";
			case 6:  return "Great";
			case 7:  return "Exceptional";
			case 8:  return "Flawless";
			case 9:  return "Perfect";
			case 10: return "Godly";
			default: return "Cheater's";
			}
		}
		break;
	case ITEM_ABILITY:	
		{
			switch(level / 3)
			{
			case 0:	 return "Worthless";
			case 1:  return "Cracked";
			case 2:  return "Normal";
			case 3:  return "Good";
			case 4:  return "Flawless";
			case 5:  return "Perfect";
			case 6:  return "Godly";
			default: return "Cheater's";
			}
		}
		break;
	case ITEM_COMBO:	
		{
			switch(level / 2)
			{
			case 0:	 return "Worthless";
			case 1:  return "Cracked";
			case 2:  return "Normal";
			case 3:  return "Good";
			case 4:  return "Flawless";
			case 5:  return "Perfect";
			case 6:  return "Godly";
			default: return "Cheater's";
			}
		}
		break;
	case ITEM_CLASSRUNE:
		{
			switch (rune->classNum)
			{
			case CLASS_ENGINEER:
				{
					switch(level / 2)
					{
					case 0:	 return "Student's";
					case 1:  return "Assistant's";
					case 2:  return "Technician's";
					case 3:  return "Mechanic's";
					case 4:  return "Scientist's";
					case 5:  return "Physicist's";
					case 6:  return "Engineer's";
					default: return "Cheater's";
					}
				}
				break;
			case CLASS_SOLDIER:
				{
					switch(level / 2)
					{
					case 0:	 return "Newbie's";
					case 1:  return "Greenhorn's";
					case 2:  return "Sergent's";
					case 3:  return "Soldier's";
					case 4:  return "Warrior's";
					case 5:  return "Veteran's";
					case 6:  return "Master's";
					default: return "Cheater's";
					}
				}
				break;
			case CLASS_MAGE:
				{
					switch(level / 2)
					{
					case 0:	 return "Apprentice's";
					case 1:  return "Illusionist's";
					case 2:  return "Sage's";
					case 3:  return "Mage's";
					case 4:  return "Wizard's";
					case 5:  return "Sorcerer's";
					case 6:  return "Archimage's";
					default: return "Cheater's";
					}
				}
				break;
			case CLASS_NECROMANCER:
				{
					switch(level / 2)
					{
					case 0:	 return "Excorcist's";
					case 1:  return "Theurgist's";
					case 2:  return "Curser's";
					case 3:  return "Necromancer's";
					case 4:  return "Warlock's";
					case 5:  return "Demi-Lich's";
					case 6:  return "Lich's";
					default: return "Cheater's";
					}
				}
				break;
			case CLASS_VAMPIRE:
				{
					switch(level / 2)
					{
					case 0:	 return "Ghoul's";
					case 1:  return "Geist's";
					case 2:  return "Wraith's";
					case 3:  return "Vampire's";
					case 4:  return "Revenant's";
					case 5:  return "Nosferatu's";
					case 6:  return "Dracula's";
					default: return "Cheater's";
					}
				}
				break;
			case CLASS_POLTERGEIST:
				{
					switch(level / 2)
					{
					case 0:	 return "Spook's";
					case 1:  return "Spirit's";
					case 2:  return "Phantom's";
					case 3:  return "Poltergeist's";
					case 4:  return "Apparition's";
					case 5:  return "Ghost's";
					case 6:  return "Monster's";
					default: return "Cheater's";
					}
				}
				break;
			case CLASS_KNIGHT:
				{
					switch(level / 2)
					{
					case 0:	 return "Commoner's";
					case 1:  return "Squire's";
					case 2:  return "Guard's";
					case 3:  return "Knight's";
					case 4:  return "Baron's";
					case 5:  return "Lord's";
					case 6:  return "King's";
					default: return "Cheater's";
					}
				}
				break;
			case CLASS_CLERIC:
				{
					switch(level / 2)
					{
					case 0:	 return "Follower's";
					case 1:  return "Acolyte's";
					case 2:  return "Preacher's";
					case 3:  return "Cleric's";
					case 4:  return "Pastor's";
					case 5:  return "Bishop's";
					case 6:  return "Pope's";
					default: return "Cheater's";
					}
				}
				break;
			case CLASS_WEAPONMASTER:
				{
					switch(level / 2)
					{
					case 0:  return "Amateur's";
					case 1:	 return "Neophyte's";
					case 2:  return "Novice's";
					case 3:  return "Weapon Master's";
					case 4:  return "Guru's";
					case 5:  return "Expert's";
					case 6:  return "Elite's'";
					default: return "Cheater's";
					}
				}
				break;
			case CLASS_ALIEN: //decino: no more unknown class shit
				{
					switch(level / 2)
					{
					case 0:  return "Breeder's";
					case 1:	 return "Hatchling's";
					case 2:  return "Drone's";
					case 3:  return "Alien's";
					case 4:  return "Stinger's";
					case 5:  return "Guardian's";
					case 6:  return "Stalker's'";
					default: return "Cheater's";
					}
				}
				break;
			case CLASS_SHAMAN: //decino: ditto
				{
					switch(level / 2)
					{
					case 0:  return "Guide's";
					case 1:	 return "Mediator's";
					case 2:  return "Healer's";
					case 3:  return "Shaman's";
					case 4:  return "Machi's";
					case 5:  return "Shi Yi's";
					case 6:  return "Yaskomo's'";
					default: return "Cheater's";
					}
				}
				break;
			default: return "<Unknown Class>";
			}
			break;
		default: return "Strange";
		}
	}
}

//************************************************************************************************
//	CLASS RUNE ARRAYS
//************************************************************************************************

int getClassRuneStat(int cIndex)
{
	int index;

	switch (cIndex)
	{
	case CLASS_SOLDIER:
		{
			index = GetRandom(0, 6);
			switch(index)
			{
			case 0: return STRENGTH;
			case 1: return RESISTANCE;
			case 2: return HA_PICKUP;
			case 3: return NAPALM;
			case 4: return SPIKE_GRENADE;
			case 5: return EMP;
			case 6: return MIRV;
			}
		}
		break;
	case CLASS_KNIGHT:
		{
			index = GetRandom(0, 6);
			switch(index)
			{
			case 0: return ARMOR_UPGRADE;
			case 1: return REGENERATION;
			case 2: return POWER_SHIELD;
			case 3: return ARMOR_REGEN;
			case 4: return EXPLODING_ARMOR;
			case 5: return BEAM;
			case 6: return PLASMA_BOLT;
			}
		}
		break;
	case CLASS_VAMPIRE:
		{
			index = GetRandom(0, 7);
			switch(index)
			{
			case 0: return VAMPIRE;
			case 1: return GHOST;
			case 2: return LIFE_DRAIN;
			case 3: return FLESH_EATER;
			case 4: return CORPSE_EXPLODE;
			case 5: return MIND_ABSORB;
			case 6: return AMMO_STEAL;
			case 7: return CONVERSION;
			}
		}
		break;
	case CLASS_NECROMANCER:
		{
			index = GetRandom(0, 7);
			switch(index)
			{
			case 0: return MONSTER_SUMMON;
			case 1: return HELLSPAWN;
			case 2: return PLAGUE;
			case 3: return AMP_DAMAGE;
			case 4: return CRIPPLE;
			case 5: return CURSE;
			case 6: return WEAKEN;
			case 7: return LOWER_RESIST;
			}
		}
		break;
	case CLASS_ENGINEER:
		{
			index = GetRandom(0, 7);
			switch(index)
			{
			case 0: return PROXY;
			case 1: return BUILD_SENTRY;
			case 2: return SUPPLY_STATION;
			case 3: return BUILD_LASER;
			case 4: return MAGMINE;
			case 5: return CALTROPS;
			case 6: return AUTOCANNON;
			case 7: return DETECTOR;
			}
		}
		break;
	case CLASS_MAGE:
		{
			index = GetRandom(0, 7);
			switch(index)
			{
			case 0: return MAGICBOLT;
			case 1: return NOVA;
			case 2: return BOMB_SPELL;
			case 3: return FIREBALL;
			case 4: return FORCE_WALL;
			case 5: return LIGHTNING;
			case 6: return METEOR;
			case 7: return LIGHTNING_STORM;
			}
		}
		break;
	case CLASS_CLERIC:
		{
			index = GetRandom(0, 7);
			switch(index)
			{
			case 0: return SALVATION;
			case 1: return HOLY_FREEZE;
			case 2: return HEALING;
			case 3: return BLESS;
			case 4: return YIN;
			case 5: return YANG;
			case 6: return HAMMER;
			case 7: return DEFLECT;
			}
		}
		break;
	case CLASS_POLTERGEIST:
		{
			index = GetRandom(0, 8);
			switch(index)
			{
			case 0: return CACODEMON;
			case 1: return BLOOD_SUCKER;
			case 2: return BRAIN;
			case 3: return FLYER;
			case 4: return MUTANT;
			case 5: return TANK;
			case 6: return MEDIC;
			case 7: return MORPH_MASTERY;
			case 8: return BERSERK;
			}
		}
		break;
	case CLASS_SHAMAN:
		{
			index = GetRandom(0, 8);
			switch(index)
			{
			case 0: return FIRE_TOTEM;
			case 1: return WATER_TOTEM;
			case 2: return AIR_TOTEM;
			case 3: return EARTH_TOTEM;
			case 4: return DARK_TOTEM;
			case 5: return NATURE_TOTEM;
			case 6: return FURY;
			case 7: return TOTEM_MASTERY;
			case 8: return HASTE;
			}
		}
		break;
	case CLASS_WEAPONMASTER:
		{
			index = GetRandom(0, 7);
			switch(index)
			{
			case 0: return SUPER_SPEED;
			case 1: return CREATE_QUAD;
			case 2: return CREATE_INVIN;
			case 3: return AMMO_REGEN;
			case 4: return WEAPON_KNOCK;
			case 5: return VITALITY;
			case 6: return POWER_REGEN;
			case 7: return WORLD_RESIST;
			}
		}
		break;
	case CLASS_ALIEN:
		{
			index = GetRandom(0, 7);
			switch(index)
			{
			case 0: return SPIKER;
			case 1: return OBSTACLE;
			case 2: return GASSER;
			case 3: return HEALER;
			case 4: return SPORE;
			case 5: return ACID;
			case 6: return SPIKE;
			case 7: return COCOON;
			}
		}
		break;
	}
	return -1;
}

//************************************************************************************************

int CountRuneMods(item_t *rune)
{
    int i;
	int count = 0;
	for(i = 0; i < MAX_VRXITEMMODS; ++i)
		if(rune->modifiers[i].type != TYPE_NONE && rune->modifiers[i].value > 0)
			++count;
	return count;    
}

//************************************************************************************************
//			Physical functions
//************************************************************************************************

float V_EntDistance(edict_t *ent1, edict_t *ent2)
//Gets the absolute vector distance between 2 entities
{
	vec3_t dist;

	dist[0] = ent1->s.origin[0] - ent2->s.origin[0];
	dist[1] = ent1->s.origin[1] - ent2->s.origin[1];
	dist[2] = ent1->s.origin[2] - ent2->s.origin[2];

    return sqrt((dist[0] * dist[0]) + (dist[1] * dist[1]) + (dist[2] * dist[2]));
}

//************************************************************************************************

void V_ResetAbilityDelays(edict_t *ent)
//Resets all of the selected ent's ability delays
{
	int i;

	//Reset ability cooldowns, even though almost none of them are used.
	for (i = 0; i < MAX_ABILITIES; ++i)
        ent->myskills.abilities[i].delay = 0.0f;

	//Reset talent cooldowns as well.
	for (i = 0; i < ent->myskills.talents.count; ++i)
		ent->myskills.talents.talent[i].delay = 0.0f;
}

//************************************************************************************************

qboolean V_CanUseAbilities (edict_t *ent, int ability_index, int ability_cost, qboolean print_msg)
{
	if (!ent->client)
		return false;
	if (!G_EntIsAlive(ent))
		return false;

	if (ent->myskills.abilities[ability_index].disable)
		return false;

	if (ent->owner && ent->owner->mtype == BOSS_TANK)
		return false;

	if (ent->owner && G_EntIsAlive(ent->owner) && IsABoss(ent->owner))
		return false;
	
	//4.2 can't use abilities in wormhole/noclip
	if (ent->flags & FL_WORMHOLE)
		return false;

	// poltergeist cannot use abilities in human form
	if (ent->myskills.class_num == CLASS_POLTERGEIST && !ent->mtype)
	{
		// allow them to morph
		if (!PM_PlayerHasMonster(ent) && (ability_index != CACODEMON) && (ability_index != MUTANT) && (ability_index != BRAIN) && (ability_index != FLYER)
			&& (ability_index != MEDIC) && (ability_index != BLOOD_SUCKER) && (ability_index != TANK) && (ability_index != BERSERK))
		{
			if (print_msg && !ent->ai.is_bot)
				safe_cprintf(ent, PRINT_HIGH, "You can't use abilities in human form!\n");
			return false;
		}
	}

	if (ent->myskills.abilities[ability_index].current_level < 1)
	{
		if (print_msg && !ent->ai.is_bot)
			safe_cprintf(ent, PRINT_HIGH, "You have to upgrade to use this ability!\n");
		return false;
	}

	// enforce special rules on flag carrier in CTF mode
	if (ctf->value && ctf_enable_balanced_fc->value && HasFlag(ent))
	{
		if (print_msg && !ent->ai.is_bot)
			safe_cprintf(ent, PRINT_HIGH, "Flag carrier cannot use abilities.\n");
		return false;
	}

	if (que_typeexists(ent->curses, CURSE_FROZEN))
		return false;

	if (level.time < pregame_time->value)
	{
//		gi.cprintf(ent, PRINT_HIGH, "You can't use abilities during pre-game.\n");
		return false;
	}
	
	// can't use abilities immediately after a re-spawn
	if (ent->client->respawn_time > level.time)
	{
		if (print_msg && !ent->ai.is_bot)
			safe_cprintf (ent, PRINT_HIGH, "You can't use abilities for another %2.1f seconds.\n", 
				ent->client->respawn_time-level.time);
		return false;
	}

	if (ent->client->ability_delay > level.time) 
	{
		if (print_msg && !ent->ai.is_bot)
			safe_cprintf (ent, PRINT_HIGH, "You can't use abilities for another %2.1f seconds.\n", 
				ent->client->ability_delay-level.time);
		return false;
	}

	if (ent->myskills.abilities[ability_index].delay > level.time)
	{
		if (print_msg && !ent->ai.is_bot)
			safe_cprintf(ent, PRINT_HIGH, "You can't use this ability for another %2.1f seconds.\n",
				ent->myskills.abilities[ability_index].delay-level.time);
		return false;
	}

	// can't use abilities if you don't have enough power cubes
	if (ability_cost && (ent->client->pers.inventory[power_cube_index] < ability_cost))
	{
		if (print_msg && !ent->ai.is_bot)
			safe_cprintf(ent, PRINT_HIGH, "You need more %d power cubes to use this ability.\n",
				ability_cost-ent->client->pers.inventory[power_cube_index]);
		return false;
	}

	// players cursed by amnesia can't use abilities
	if (que_findtype(ent->curses, NULL, AMNESIA) != NULL)
	{
		if (print_msg)
			safe_cprintf(ent, PRINT_HIGH, "You have been cursed with amnesia and can't use any abilities!!\n");
		return false;
	}
	return true;
}

//************************************************************************************************

//Gives the player packs of ammo
qboolean V_GiveAmmoClip(edict_t *ent, float qty, int ammotype)
{
	int amount;
	int *max;
	int *current;

	switch(ammotype)
	{
	case AMMO_SHELLS:
		amount = SHELLS_PICKUP;
		current = &ent->client->pers.inventory[shell_index];
		max = &ent->client->pers.max_shells;
		break;
	case AMMO_BULLETS:
		amount = BULLETS_PICKUP;
		current = &ent->client->pers.inventory[bullet_index];
		max = &ent->client->pers.max_bullets;
		break;
	case AMMO_GRENADES:
		amount = GRENADES_PICKUP;
		current = &ent->client->pers.inventory[grenade_index];
		max = &ent->client->pers.max_grenades;
		break;
	case AMMO_ROCKETS:
		amount = ROCKETS_PICKUP;
		current = &ent->client->pers.inventory[rocket_index];
		max = &ent->client->pers.max_rockets;
		break;
	case AMMO_SLUGS:
		amount = SLUGS_PICKUP;
		current = &ent->client->pers.inventory[slug_index];
		max = &ent->client->pers.max_slugs;
		break;
	case AMMO_CELLS:
		amount = CELLS_PICKUP;
		current = &ent->client->pers.inventory[cell_index];
		max = &ent->client->pers.max_cells;
		break;
	default: return false;
	}

	//don't pick up ammo if you are full already
	//if (*current >= *max) return false;
	if (*current >= *max && qty > 0.0) return false;

	//Multiply by the item quantity
	amount *= qty;

	//Multiply by the ammo efficiency skill
	//if (!ent->myskills.abilities[AMMO_UPGRADE].disable)
	//	amount *= AMMO_UP_BASE + (AMMO_UP_MULT * ent->myskills.abilities[AMMO_UPGRADE].current_level);

	//Talent: Improved HA Pickup
    // if (getTalentSlot(ent, TALENT_BASIC_HA) != -1)  
    //      amount *= 1.0 + (0.2 * getTalentLevel(ent, TALENT_BASIC_HA)); 

	if(ent->myskills.talents.AmmoLevel > 0)
	amount *= 1.0 + (0.33 * ent->myskills.talents.AmmoLevel); 

	//Give them the ammo
	*current += amount;
	if (*current > *max)
		*current = *max;
	if (*current < 0)
		*current = 0;

	return true;
}

//************************************************************************************************

//Returns an ammo type based on the player's respawn weapon.
int V_GetRespawnAmmoType(edict_t *ent)
{
	switch (ent->myskills.respawn_weapon)
	{
	case 2:		//sg
	case 3:		//ssg
	case 12:	//20mm
		return AMMO_SHELLS;
	case 4:		//mg
	case 5:		//cg
		return AMMO_BULLETS;
	case 6:		//gl
	case 11:	//hg
		return AMMO_GRENADES;
	case 7:		//rl
		return AMMO_ROCKETS;
	case 9:		//rg
		return AMMO_SLUGS;
	case 8:		//hb
	case 10:	//bfg
		return AMMO_CELLS;
	default:	//blaster/sword
		return 0;	//nothing
	}
}

//************************************************************************************************

int GetClientNumber(edict_t *ent)
//Returns zero if a client was not found
{
	edict_t *temp;
	int i;

	for (i = 1; i <= game.maxclients; i++)
	{
		temp = &g_edicts[i];

		//Copied from id. do not crash
		if (!temp->inuse)
			continue;
		if (!temp->client)
			continue;

		//More checking
		if (strlen(temp->myskills.player_name) < 1)
			continue;

		if (Q_strcasecmp(ent->myskills.player_name, temp->myskills.player_name) == 0)	//same name
			return i+1;
	}
	return 0;
}

//************************************************************************************************

edict_t *V_getClientByNumber(int index)
//Gets client through an index number used in GetClientNumber()
{
	return &g_edicts[index];
}

//************************************************************************************************
//************************************************************************************************
//			New File I/O functions
//************************************************************************************************
//************************************************************************************************

char ReadChar(FILE *fptr)
{
	char Value;
	fread(&Value, sizeof(char), 1, fptr);
	return Value;
}

//************************************************************************************************

void WriteChar(FILE *fptr, char Value)
{
	fwrite(&Value, sizeof(char), 1, fptr);
}

//************************************************************************************************

void ReadString(char *buf, FILE *fptr)
{
	int Length;

	//get the string length
	Length = ReadChar(fptr);

	//If string is empty, abort
	if (Length == 0)
	{
		buf[0] = 0;
		//return buf;
	}

	fread(buf, Length, 1, fptr);

	//Null terminate the string just read
	buf[Length] = 0;
}

//************************************************************************************************

void WriteString(FILE *fptr, char *String)
{
	int Length = strlen(String);
	WriteChar(fptr, (char)Length);
	fwrite(String, Length, 1, fptr);	
}

//************************************************************************************************

int ReadInteger(FILE *fptr)
{
	int Value;
	fread(&Value, sizeof(int), 1, fptr);
	return Value;
}

//************************************************************************************************

void WriteInteger(FILE *fptr, int Value)
{
	fwrite(&Value, sizeof(int), 1, fptr);
}

//************************************************************************************************

long ReadLong(FILE *fptr)
{
	long Value;
	fread(&Value, sizeof(long), 1, fptr);
	return Value;
}

//************************************************************************************************

void WriteLong(FILE *fptr, long Value)
{
	fwrite(&Value, sizeof(long), 1, fptr);
}

//************************************************************************************************

char *V_FormatFileName(char *name)
{
	char filename[64];
	char buffer[64];
	int i, j = 0;

	//This bit of code formats invalid filename chars, like the '?' and 
	//reformats them into a saveable format.
	Q_strncpy (buffer, name, sizeof(buffer)-1);
	
	for (i = 0; i < strlen(buffer); ++i)
	{
		char tmp;
		switch (buffer[i])
		{
		case '\\':		tmp = '1';		break;
		case '/':		tmp = '2';		break;
		case '?':		tmp = '3';		break;
		case '|':		tmp = '4';		break;
		case '"':		tmp = '5';		break;
		case ':':		tmp = '6';		break;
		case '*':		tmp = '7';		break;
		case '<':		tmp = '8';		break;
		case '>':		tmp = '9';		break;
		case '_':		tmp = '_';		break;
		default:		tmp = '0';		break;
		}

		if (tmp != '0')
		{
			filename[j++] = '_';
			filename[j++] = tmp;
		}
		else
		{
			filename[j++] = buffer[i];
		}		
	}

	//make sure string is null-terminated
	filename[j] = 0;

	return va("%s", filename);
}

//************************************************************************************************
//************************************************************************************************
//			New File I/O functions (for parsing text files)
//************************************************************************************************
//************************************************************************************************

int V_tFileCountLines(FILE *fptr, long size)
{
	int count = 0;
	char temp;
	int i = 0;
	
	do
	{
		temp = getc(fptr);
		if (temp == '\n')
			count++;
	}
	while (++i < size);

	rewind(fptr);
	return count - 1;	//Last line is always empty
}

//************************************************************************************************

void V_tFileGotoLine(FILE *fptr, int linenumber, long size)
{
	int count = 0;
	char temp;
	int i = 0;
	
	do
	{
		temp = fgetc(fptr);
		if (temp == '\n')
			count++;
		if (count == linenumber)
			return;
	}
	while (++i < size);
		return;
}

//************************************************************************************************

// this needs to match NewLevel_Addons() in player_points.c
// NOTE: this function can't/won't award refunds if the ability was already upgraded
void UpdateFreeAbilities (edict_t *ent)
{
	if (ent->myskills.level >= 0)
	{
		// free ID at level 0
		if (!ent->myskills.abilities[ID].level)
		{
			ent->myskills.abilities[ID].level++;
			ent->myskills.abilities[ID].current_level++;
		}

		// free scanner at level 10
		if (ent->myskills.level >= 10)
		{
			if (!ent->myskills.abilities[SCANNER].level)
			{
				ent->myskills.abilities[SCANNER].level++;
				ent->myskills.abilities[SCANNER].current_level++;
			}
		}
	}

	// free max ammo and vitality upgrade every 5 levels
	ent->myskills.abilities[MAX_AMMO].level = ent->myskills.abilities[VITALITY].level = ent->myskills.level / 5;
}

#define CHANGECLASS_MSG_CHANGE	1
#define CHANGECLASS_MSG_RESET	2

void ChangeClass (char *playername, int newclass, int msgtype)
{
	int		i = 0;
	edict_t *player;
	float tempTalentPoints = 0;

	if (!newclass)
		return;

	for (i = 1; i <= maxclients->value; i++)
	{
		player = &g_edicts[i];
		if (!player->inuse)
			continue;
		if (Q_strcasecmp(playername, player->myskills.player_name) != 0)
			continue;

		if (newclass == CLASS_KNIGHT)
			player->myskills.respawn_weapon = 1; //sword only

		//Reset player's skills and change their class
		memset(player->myskills.abilities, 0, sizeof(upgrade_t) * MAX_ABILITIES);
		memset(player->myskills.weapons, 0, sizeof(weapon_t) * MAX_WEAPONS);
		player->myskills.class_num = newclass;

		//Give them some upgrade points.
		player->myskills.speciality_points = 2 * player->myskills.level;

		//if(player->myskills.class_num == CLASS_WEAPONMASTER)
		//	player->myskills.weapon_points = 6 * player->myskills.level;
		//else
		player->myskills.weapon_points = 4 * player->myskills.level;

		//Weapon masters get double the weapon points of other classes.
		//if(player->myskills.class_num == CLASS_WEAPONMASTER)
		//	player->myskills.weapon_points *= 2;
		
		//Initialize their upgrades
		disableAbilities(player);
		setGeneralAbilities(player);
		setClassAbilities(player);
        resetWeapons(player);

		//Check for special level-up bonuses
		//max ammo and vit
		UpdateFreeAbilities(player);
		//player->myskills.abilities[MAX_AMMO].level = player->myskills.abilities[VITALITY].level = player->myskills.level / 5;
		//if (player->myskills.level > 9)
		//	player->myskills.abilities[ID].level = 1;
		

		//Reset talents, and give them their talent points
		eraseTalents(player);
		setTalents(player);
		/*if(player->myskills.level < TALENT_MAX_LEVEL)
			player->myskills.talents.talentPoints = player->myskills.level - TALENT_MIN_LEVEL + 1;
		else player->myskills.talents.talentPoints = TALENT_MAX_LEVEL - TALENT_MIN_LEVEL + 1;*/

		if (player->myskills.level < TALENT_MAX_LEVEL) 
			{
 				tempTalentPoints = player->myskills.level - TALENT_MIN_LEVEL + 1;
 				tempTalentPoints = ceil(tempTalentPoints/2.f);
 				player->myskills.talents.talentPoints = (int)tempTalentPoints;
 			}
 			else 
			{
 				tempTalentPoints = TALENT_MAX_LEVEL - TALENT_MIN_LEVEL + 1;
 				tempTalentPoints = ceil(tempTalentPoints/2.f);
 				player->myskills.talents.talentPoints = (int)tempTalentPoints;
 			}

		if(player->myskills.talents.talentPoints < 0)
			player->myskills.talents.talentPoints = 0;

		//Re-apply equipment
		V_ResetAllStats(player);
		for (i = 0; i < 3; ++i)
			V_ApplyRune(player, &player->myskills.items[i]);

		if (msgtype == CHANGECLASS_MSG_CHANGE)
		{
			//Notify everyone
			gi.cprintf(NULL, PRINT_HIGH, "%s's class was changed to %s (%d).\n", playername,  GetClassString(newclass), newclass);
			WriteToLogfile(player, va("Class was changed to %s (%d).\n", gi.argv(3), newclass));
			gi.dprintf("%s's class was changed to %s (%d).\n", playername, gi.argv(3), newclass);
		}
		else if (msgtype == CHANGECLASS_MSG_RESET)
		{
			gi.cprintf(player, PRINT_HIGH, "Your ability and weapon upgrades have been reset!\n");
			WriteToLogfile(player, "Character data was reset.\n");
			gi.dprintf("%s's character data was reset.\n", playername);
		}

		//Reset max health/armor/ammo
		modify_max(player);

		//Save the player.
		savePlayer(player);
		return;
	}
	gi.dprintf("Can't find player: %s\n", playername);
}

void V_TruncateString (char *string, int newStringLength, char *dest)
{
//	char buf[512];

	if (!newStringLength || (newStringLength > 512))
		return;

	// it's already short enough
	if (strlen(string) <= newStringLength)
		return;

 		strncpy(dest,string,newStringLength);
 		dest[newStringLength] = '\0';
}

void V_RegenAbilityAmmo (edict_t *ent, int ability_index, int regen_frames, int regen_delay)
{
	int ammo;
	int max = ent->myskills.abilities[ability_index].max_ammo;
	int *current = &ent->myskills.abilities[ability_index].ammo;
	int *delay = &ent->myskills.abilities[ability_index].ammo_regenframe;

	if (*current > max)
		return;

	// don't regenerate ammo if player is firing
//	if (ent->client->buttons & BUTTON_ATTACK) decino: uhh, no :)
//		return;

	if (regen_delay > 0)
	{
		if (level.framenum < *delay)
			return;

		*delay = level.framenum + regen_delay;
	}
	else
		regen_delay = 1;

	ammo = floattoint((float)max / ((float)regen_frames / regen_delay));

	//gi.dprintf("ammo=%d, max=%d, frames=%d, delay=%d\n", ammo, max, regen_frames, regen_delay);

	if (ammo < 1)
		ammo = 1;

	//decino: only regenerate 20% of the ammo if we're firing
	if (ent->client->buttons & BUTTON_ATTACK)
		ammo *= 0.2;

	*current += ammo;
	if (*current > max)
		*current = max;
}

void V_RestoreMorphed (edict_t *ent, int refund)
{
	//gi.dprintf("V_RestoreMorphed()\n");

	if (ent->myskills.class_num == CLASS_POLTERGEIST)
		VortexRemovePlayerSummonables(ent);

	if (PM_PlayerHasMonster(ent))
	{
		if (refund > 0)
			ent->client->pers.inventory[power_cube_index] += refund;

		// player's chasing this monster should now chase the player instead
		PM_UpdateChasePlayers(ent->owner);

		// remove the tank entity
		M_Remove(ent->owner, false, false);
		PM_RestorePlayer(ent);
		return;
	}

	if (ent->mtype && (refund > 0))
		ent->client->pers.inventory[power_cube_index] += refund;

	ent->viewheight = 22;
	ent->mtype = 0;
	ent->s.modelindex = 255;
	ent->s.skinnum = ent-g_edicts-1;
	ShowGun(ent);

	//decino: show cool morph fx!!!1!1!
	ent->morph_blendtime = level.time + 0.1;
	gi.sound(ent, CHAN_ITEM, gi.soundindex("misc/itembreak.wav"), 1, ATTN_NORM, 0);
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_BFG_BIGEXPLOSION);
	gi.WritePosition (ent->s.origin);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->lock_frames = 0;//4.2 reset smart-rocket lock-on counter
}

void mutant_checkattack (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf);
void V_Touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	// made a sound
	if (other && other->inuse)
		other->lastsound = level.framenum;

	mutant_checkattack(self, other, plane, surf);
}

//FIXME: this doesn't work with 2-4 point abilities
void V_UpdatePlayerAbilities (edict_t *ent)
{
	int			i, refunded = 0;
	upgrade_t	old_abilities[MAX_ABILITIES];
	qboolean	points_refunded = false;

	UpdateFreeAbilities(ent);

	// make a copy of old abilities that we can use as a reference for comparison
	for (i=0; i<MAX_ABILITIES; ++i)	
		memcpy(&old_abilities, ent->myskills.abilities, sizeof(upgrade_t) * MAX_ABILITIES);

	// reset all ability upgrades
	memset(ent->myskills.abilities, 0, sizeof(upgrade_t) * MAX_ABILITIES);
	disableAbilities(ent);

	// set general abilities
	setGeneralAbilities(ent);
	// set class-specific abilities
	setClassAbilities(ent);

	for (i=0; i<MAX_ABILITIES; ++i)	
	{
		// if this ability was previously enabled, restore the upgrade level
		if ((ent->myskills.abilities[i].disable == false) && (old_abilities[i].disable == false))
		{
			// restore previous upgrade level if our new max_level is greater or equal to the old one
			if (ent->myskills.abilities[i].max_level >= old_abilities[i].max_level)
				ent->myskills.abilities[i].level = old_abilities[i].level;
			// otherwise set new upgrade level to soft max and refund unused points
			else
			{
				refunded = old_abilities[i].level - ent->myskills.abilities[i].max_level;
				ent->myskills.abilities[i].level = ent->myskills.abilities[i].max_level;
			}
		}
		// if this upgrade was previously disabled, refund them points
		else if ((ent->myskills.abilities[i].disable == true) && (old_abilities[i].disable == false))
			refunded = old_abilities[i].level;
	}

	// re-apply equipment
	V_ResetAllStats(ent);
	for (i = 0; i < 3; ++i)
		V_ApplyRune(ent, &ent->myskills.items[i]);

	gi.cprintf(ent, PRINT_HIGH, "Your abilities have been updated.\n");
	
	// have points been refunded?
	if (refunded > 0)
	{
		ent->myskills.speciality_points += old_abilities[i].level;
		gi.cprintf(ent, PRINT_HIGH, "%d ability points have been refunded.\n", refunded);
	}
}

char *V_GetMonsterName (int mtype)
{
	switch (mtype)
	{
	case M_SOLDIER:
	case M_SOLDIERLT:
	case M_SOLDIERSS:
		return "soldier";
	case M_FLIPPER:
		return "flipper";
	case M_FLYER:
		return "flyer";
	case M_INFANTRY:
		return "infantry";
	case M_INSANE:
	case M_RETARD:
		return "retard";
	case M_GUNNER:
		return "gunner";
	case M_CHICK:
		return "bitch";
	case M_PARASITE:
		return "parasite";
	case M_FLOATER:
		return "floater";
	case M_HOVER:
		return "hover";
	case M_BERSERK:
		return "berserker";
	case M_MEDIC:
		return "medic";
	case M_MUTANT:
		return "mutant";
	case M_BRAIN:
		return "brain";
	case M_GLADIATOR:
		return "gladiator";
	case M_TANK:
	case P_TANK:
		return "tank";
	case M_SUPERTANK:
		return "supertank";
	case M_JORG:
		return "jorg";
	case M_MAKRON:
		return "makron";
	case M_COMMANDER:
		return "commander";
	case M_MINISENTRY:
		return "mini-sentry";
	case M_SENTRY:
		return "sentry";
	case M_FORCEWALL:
		return "force wall";
	case M_DECOY:
		return "admin clone";
	case M_SKULL:
		return "hellspawn";
	case M_YINSPIRIT:
		return "yin spirit";
	case M_YANGSPIRIT:
		return "yang spirit";
	case M_BALANCESPIRIT:
		return "balance spirit";
	case M_AUTOCANNON:
		return "autocannon";
	case M_DETECTOR:
		return "detector";
	case TOTEM_FIRE:
		return "fire totem";
	case TOTEM_WATER:
		return "water totem";
	case TOTEM_AIR:
		return "air totem";
	case TOTEM_EARTH:
		return "earth totem";
	case TOTEM_NATURE:
		return "nature totem";
	case TOTEM_DARKNESS:
		return "darkness totem";
	case M_SPIKER:
		return "spiker";
	case M_OBSTACLE:
		return "obstacle";
	case M_GASSER:
		return "gasser";
	case M_SPIKEBALL:
		return "spore";
	case M_HEALER:
		return "healer";
	case M_COCOON:
		return "cocoon";
	case M_LASERPLATFORM:
		return "laser platform";
	case INVASION_PLAYERSPAWN: 
	case CTF_PLAYERSPAWN:
		return "spawn";
	default:
		return "<unknown>";
	}
}

qboolean V_IsPVP (void)
{
	return (!pvm->value && !ctf->value && !ffa->value && !invasion->value && !ctf->value && !ptr->value);
}

qboolean V_HealthCache (edict_t *ent, int max_per_second, int update_frequency)
{
	int heal, delta, max;

	if (ent->health_cache_nextframe > level.framenum)
		return false;

	ent->health_cache_nextframe = level.framenum + update_frequency;

	if (ent->health_cache > 0 && ent->health < ent->max_health)
	{
		if (update_frequency <= 10)
			max = max_per_second / (10 / update_frequency);
		else
			max = max_per_second;
		if (max > ent->health_cache)
			max = ent->health_cache;

		delta = ent->max_health - ent->health;

		if (delta > max)
			heal = max;
		else
			heal = delta;

		ent->health += heal;
		ent->health_cache -= heal;

	//	gi.dprintf("healed %d / %d (max %d) at %d\n", heal, ent->health_cache, max, level.framenum);

		return true;
	}

	ent->health_cache = 0;

	return false;
}
	
qboolean V_ArmorCache (edict_t *ent, int max_per_second, int update_frequency)
{
	int heal, delta, max, max_armor;
	int	*armor;

	if (ent->armor_cache_nextframe > level.framenum)
		return false;

	if (ent->client)
	{
		armor = &ent->client->pers.inventory[body_armor_index];
		max_armor = MAX_ARMOR(ent);
	}
	else
	{
		armor = &ent->monsterinfo.power_armor_power;
		max_armor = ent->monsterinfo.max_armor;
	}

	ent->armor_cache_nextframe = level.framenum + update_frequency;

	if (ent->armor_cache > 0 && *armor < max_armor)
	{
		if (update_frequency <= 10)
			max = max_per_second / (10 / update_frequency);
		else
			max = max_per_second;
		if (max > ent->armor_cache)
			max = ent->armor_cache;

		delta = max_armor - *armor;

		if (delta > max)
			heal = max;
		else
			heal = delta;

		*armor += heal;
		ent->armor_cache -= heal;

		return true;
	}

	ent->armor_cache = 0;

	return false;
}		

void V_ResetPlayerState (edict_t *ent)
{
	if (PM_PlayerHasMonster(ent))
	{
		// player's chasing this monster should now chase the player instead
		PM_UpdateChasePlayers(ent->owner);
		// remove the monster entity
		M_Remove(ent->owner, true, false);
		// restore the player to original state
		PM_RestorePlayer(ent);
	}

	// restore morphed player state
	if (ent->mtype)
	{
		ent->viewheight = 22;
		ent->mtype = 0;
		ent->s.modelindex = 255;
		ent->s.skinnum = ent-g_edicts-1;
		ShowGun(ent);

		// reset flyer rocket lock-on frames
		ent->client->lock_frames = 0;
	}
	
	// reset railgun sniper frames
	ent->client->refire_frames = 0;

	// reset h/a cache
	ent->health_cache = 0;
	ent->health_cache_nextframe = 0;
	ent->armor_cache = 0;
	ent->armor_cache_nextframe = 0;

	// boot them out of a wormhole, but don't forget to teleport them to a valid location!
	ent->flags &= ~FL_WORMHOLE;
	ent->svflags &= ~SVF_NOCLIENT;
	ent->movetype = MOVETYPE_WALK;

	// reset detected state
	ent->flags &= ~FL_DETECTED;
	ent->detected_time = 0;

	// reset cocooned state
	ent->cocoon_time = 0;
	ent->cocoon_factor = 0;

	// disble fury
	ent->fury_time = 0;

	// disable movement abilities
	//jetpack
	ent->client->thrusting = 0;
	//grapple hook
	ent->client->hook_state = HOOK_READY;
	// super speed
	ent->superspeed = false;
	// mana shield
	ent->manashield = false;
	// reset holdtime
	ent->holdtime = 0;

	// powerups
	ent->client->invincible_framenum = 0;
	ent->client->quad_framenum = 0;

	// reset ability delay
	ent->client->ability_delay = 0;
	V_ResetAbilityDelays(ent);

	// reset their velocity
	VectorClear(ent->velocity);

	// remove curses
	CurseRemove(ent, 0);

	if (ent->client)
	ent->client->is_poisoned = false;
	// remove auras
	AuraRemove(ent, 0);

	// remove movement penalty
	ent->Slower = 0;
	ent->slowed_factor = 1.0;
	ent->slowed_time = 0;
	ent->chill_level = 0;
	ent->chill_time = 0;

	// remove all summonables
	VortexRemovePlayerSummonables(ent);

	// if teamplay mode, set team skin
	if (ctf->value || domination->value)
	{
		char *s = Info_ValueForKey(ent->client->pers.userinfo, "skin");
		AssignTeamSkin(ent, s);
		
		// drop the flag
		dom_dropflag(ent, FindItem("Flag"));
		CTF_DropFlag(ent, FindItem("Red Flag"));
		CTF_DropFlag(ent, FindItem("Blue Flag"));	
	}

	// drop all techs
	tech_dropall(ent);

	// disable scanner
	ClearScanner(ent->client);

	// stop trading
	EndTrade(ent);
}

/*
============
V_TouchSolids
A modified version of G_TouchTriggers that will call the touch function
of any valid intersecting entities
============
*/
void V_TouchSolids (edict_t *ent)
{
	int			i, num;
	edict_t		*touch[MAX_EDICTS], *hit;

	// sanity check
	if (!ent || !ent->inuse || !ent->takedamage)
		return;

	num = gi.BoxEdicts (ent->absmin, ent->absmax, touch, MAX_EDICTS, AREA_SOLID);

	// be careful, it is possible to have an entity in this
	// list removed before we get to it (killtriggered)
	for (i=0 ; i<num ; i++)
	{
		hit = touch[i];
		if (!hit->inuse || !hit->touch || !hit->takedamage || hit == ent)
			continue;
		hit->touch (hit, ent, NULL, NULL);
	}
}

// returns true if an entity has any offensive
// (meaning they can do damage) deployed
qboolean V_HasSummons (edict_t *ent)
{
	return (ent->num_sentries || ent->num_monsters || ent->skull || ent->num_spikers 
		|| ent->num_autocannon || ent->num_gasser || ent->num_caltrops || ent->num_proxy 
		|| ent->num_obstacle || ent->num_armor || ent->num_spikeball || ent->num_lasers);
}