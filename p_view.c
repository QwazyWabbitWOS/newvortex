#include "g_local.h"
#include "m_player.h"

static	edict_t		*current_player;
static	gclient_t	*current_client;

static	vec3_t	forward, right, up;
float	xyspeed;

float	bobmove;
int		bobcycle;		// odd cycles are right foot going forward
float	bobfracsin;		// sin(bobfrac*M_PI)

/*
===============
SV_CalcRoll

===============
*/
float SV_CalcRoll (vec3_t angles, vec3_t velocity)
{
	float	sign;
	float	side;
	float	value;
	
	side = DotProduct (velocity, right);
	sign = side < 0 ? -1 : 1;
	side = fabs(side);
	
	value = sv_rollangle->value;

	if (side < sv_rollspeed->value)
		side = side * value / sv_rollspeed->value;
	else
		side = value;
	
	return side*sign;
	
}


/*
===============
P_DamageFeedback

Handles color blends and view kicks
===============
*/
void P_DamageFeedback (edict_t *player)
{
	gclient_t	*client;
	float	side;
	float	realcount, count, kick;
	vec3_t	v;
	int		r, l;
	static	vec3_t	power_color = {0.0, 1.0, 0.0};
	static	vec3_t	acolor = {1.0, 1.0, 1.0};
	static	vec3_t	bcolor = {1.0, 0.0, 0.0};

	client = player->client;

	// flash the backgrounds behind the status numbers
	client->ps.stats[STAT_FLASHES] = 0;
	if (client->damage_blood)
		client->ps.stats[STAT_FLASHES] |= 1;
	if (client->damage_armor && !(player->flags & FL_GODMODE) && (client->invincible_framenum <= level.framenum))
		client->ps.stats[STAT_FLASHES] |= 2;

	// total points of damage shot at the player this frame
	count = (client->damage_blood + client->damage_armor + client->damage_parmor);
	if (count == 0)
		return;		// didn't take any damage

	// start a pain animation if still in the player model
	if (client->anim_priority < ANIM_PAIN && (player->s.modelindex == 255))
	{
		static int		i;

		client->anim_priority = ANIM_PAIN;
		if (client->ps.pmove.pm_flags & PMF_DUCKED)
		{
			player->s.frame = FRAME_crpain1-1;
			client->anim_end = FRAME_crpain4;
		}
		else
		{
			i = (i+1)%3;
			switch (i)
			{
			case 0:
				player->s.frame = FRAME_pain101-1;
				client->anim_end = FRAME_pain104;
				break;
			case 1:
				player->s.frame = FRAME_pain201-1;
				client->anim_end = FRAME_pain204;
				break;
			case 2:
				player->s.frame = FRAME_pain301-1;
				client->anim_end = FRAME_pain304;
				break;
			}
		}
	}

	realcount = count;
	if (count < 10)
		count = 10;	// always make a visible effect

	// play an apropriate pain sound
	if ((level.time > player->pain_debounce_time) && !(player->flags & FL_GODMODE) 
		&& (client->invincible_framenum <= level.framenum) && !player->mtype
		&& !(player->owner && player->owner->inuse)) // don't play sound if morphed
	{
		r = 1 + (randomMT()&1);
		player->pain_debounce_time = level.time + 0.7;
		if (player->health < 25)
			l = 25;
		else if (player->health < 50)
			l = 50;
		else if (player->health < 75)
			l = 75;
		else
			l = 100;

		gi.sound (player, CHAN_VOICE, gi.soundindex(va("*pain%i_%i.wav", l, r)), 1, ATTN_NORM, 0);
	}

	// the total alpha of the blend is always proportional to count
	if (client->damage_alpha < 0)
		client->damage_alpha = 0;
	client->damage_alpha += count*0.01;
	if (client->damage_alpha < 0.2)
		client->damage_alpha = 0.2;
	if (client->damage_alpha > 0.6)
		client->damage_alpha = 0.6;		// don't go too saturated

	// the color of the blend will vary based on how much was absorbed
	// by different armors
	VectorClear (v);
	if (client->damage_parmor)
		VectorMA (v, (float)client->damage_parmor/realcount, power_color, v);
	if (client->damage_armor)
		VectorMA (v, (float)client->damage_armor/realcount,  acolor, v);
	if (client->damage_blood)
		VectorMA (v, (float)client->damage_blood/realcount,  bcolor, v);
	VectorCopy (v, client->damage_blend);


	//
	// calculate view angle kicks
	//
	kick = abs(client->damage_knockback);
	if (kick && player->health > 0)	// kick of 0 means no view adjust at all
	{
		kick = kick * 100 / player->health;

		if (kick < count*0.5)
			kick = count*0.5;
		if (kick > 50)
			kick = 50;

		VectorSubtract (client->damage_from, player->s.origin, v);
		VectorNormalize (v);
		
		side = DotProduct (v, right);
		client->v_dmg_roll = kick*side*0.3;
		
		side = -DotProduct (v, forward);
		client->v_dmg_pitch = kick*side*0.3;

		client->v_dmg_time = level.time + DAMAGE_TIME;
	}

	//
	// clear totals
	//
	client->damage_blood = 0;
	client->damage_armor = 0;
	client->damage_parmor = 0;
	client->damage_knockback = 0;
}




/*
===============
SV_CalcViewOffset

Auto pitching on slopes?

  fall from 128: 400 = 160000
  fall from 256: 580 = 336400
  fall from 384: 720 = 518400
  fall from 512: 800 = 640000
  fall from 640: 960 = 

  damage = deltavelocity*deltavelocity  * 0.0001

===============
*/
void SV_CalcViewOffset (edict_t *ent)
{
	float		*angles;
	float		bob;
	float		ratio;
	float		delta;
	vec3_t		v;


//===================================

	// base angles
	angles = ent->client->ps.kick_angles;

	// if dead, fix the angle and don't add any kick
	if (ent->deadflag)
	{
		VectorClear (angles);

		ent->client->ps.viewangles[ROLL] = 40;
		ent->client->ps.viewangles[PITCH] = -15;
		ent->client->ps.viewangles[YAW] = ent->client->killer_yaw;
	}
	else
	{
		// add angles based on weapon kick

		VectorCopy (ent->client->kick_angles, angles);

		// add angles based on damage kick

		ratio = (ent->client->v_dmg_time - level.time) / DAMAGE_TIME;
		if (ratio < 0)
		{
			ratio = 0;
			ent->client->v_dmg_pitch = 0;
			ent->client->v_dmg_roll = 0;
		}
		angles[PITCH] += ratio * ent->client->v_dmg_pitch;
		angles[ROLL] += ratio * ent->client->v_dmg_roll;

		// add pitch based on fall kick

		ratio = (ent->client->fall_time - level.time) / FALL_TIME;
		if (ratio < 0)
			ratio = 0;
		angles[PITCH] += ratio * ent->client->fall_value;

		// add angles based on velocity

		delta = DotProduct (ent->velocity, forward);
		angles[PITCH] += delta*run_pitch->value;
		
		delta = DotProduct (ent->velocity, right);
		angles[ROLL] += delta*run_roll->value;

		// add angles based on bob

		delta = bobfracsin * bob_pitch->value * xyspeed;
		if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
			delta *= 6;		// crouching
		angles[PITCH] += delta;
		delta = bobfracsin * bob_roll->value * xyspeed;
		if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
			delta *= 6;		// crouching
		if (bobcycle & 1)
			delta = -delta;
		angles[ROLL] += delta;
	}

//===================================

	// base origin

	VectorClear (v);

	// add view height

	v[2] += ent->viewheight;

	// add fall height

	ratio = (ent->client->fall_time - level.time) / FALL_TIME;
	if (ratio < 0)
		ratio = 0;
	v[2] -= ratio * ent->client->fall_value * 0.4;

	// add bob height

	bob = bobfracsin * xyspeed * bob_up->value;
	if (bob > 6)
		bob = 6;
	//gi.DebugGraph (bob *2, 255);
	v[2] += bob;

	// add kick offset

	VectorAdd (v, ent->client->kick_origin, v);

	// absolutely bound offsets
	// so the view can never be outside the player box

	//gi.dprintf("before: v[2] %d\n", (int)v[2]);

	if (v[0] < -14)
		v[0] = -14;
	else if (v[0] > 14)
		v[0] = 14;
	if (v[1] < -14)
		v[1] = -14;
	else if (v[1] > 14)
		v[1] = 14;

	if (v[2] < -22)
		v[2] = -22;
	//	else if (v[2] > ent->maxs[2]-2)
	//		v[2] = ent->maxs[2]-2;
	else if (v[2] > 30)
		v[2] = 30;

	//gi.dprintf("after: v[2] %d\n", (int)v[2]);
	
	VectorCopy (v, ent->client->ps.viewoffset);
}

/*
==============
SV_CalcGunOffset
==============
*/
void SV_CalcGunOffset (edict_t *ent)
{
	int		i;
	float	delta;

	// gun angles from bobbing
	ent->client->ps.gunangles[ROLL] = xyspeed * bobfracsin * 0.005;
	ent->client->ps.gunangles[YAW] = xyspeed * bobfracsin * 0.01;
	if (bobcycle & 1)
	{
		ent->client->ps.gunangles[ROLL] = -ent->client->ps.gunangles[ROLL];
		ent->client->ps.gunangles[YAW] = -ent->client->ps.gunangles[YAW];
	}

	ent->client->ps.gunangles[PITCH] = xyspeed * bobfracsin * 0.005;

	// gun angles from delta movement
	for (i=0 ; i<3 ; i++)
	{
		delta = ent->client->oldviewangles[i] - ent->client->ps.viewangles[i];
		if (delta > 180)
			delta -= 360;
		if (delta < -180)
			delta += 360;
		if (delta > 45)
			delta = 45;
		if (delta < -45)
			delta = -45;
		if (i == YAW)
			ent->client->ps.gunangles[ROLL] += 0.1*delta;
		ent->client->ps.gunangles[i] += 0.2 * delta;
	}

	// gun height
	VectorClear (ent->client->ps.gunoffset);
//	ent->ps->gunorigin[2] += bob;

	// gun_x / gun_y / gun_z are development tools
	for (i=0 ; i<3 ; i++)
	{
		ent->client->ps.gunoffset[i] += forward[i]*(gun_y->value);
		ent->client->ps.gunoffset[i] += right[i]*gun_x->value;
		ent->client->ps.gunoffset[i] += up[i]* (-gun_z->value);
	}
}


/*
=============
SV_AddBlend
=============
*/
void SV_AddBlend (float r, float g, float b, float a, float *v_blend)
{
	float	a2, a3;

	if (a <= 0)
		return;
	a2 = v_blend[3] + (1-v_blend[3])*a;	// new total alpha
	a3 = v_blend[3]/a2;		// fraction of color from old

	v_blend[0] = v_blend[0]*a3 + r*(1-a3);
	v_blend[1] = v_blend[1]*a3 + g*(1-a3);
	v_blend[2] = v_blend[2]*a3 + b*(1-a3);
	v_blend[3] = a2;
}


/*
=============
SV_CalcBlend
=============
*/
void SV_CalcBlend (edict_t *ent)
{
	int		contents;
	vec3_t	vieworg;
	int		remaining;

	ent->client->ps.blend[0] = ent->client->ps.blend[1] = 
		ent->client->ps.blend[2] = ent->client->ps.blend[3] = 0;

	// add for contents
	VectorAdd (ent->s.origin, ent->client->ps.viewoffset, vieworg);
	contents = gi.pointcontents (vieworg);
	if (contents & (CONTENTS_LAVA|CONTENTS_SLIME|CONTENTS_WATER) )
		ent->client->ps.rdflags |= RDF_UNDERWATER;
	else
		ent->client->ps.rdflags &= ~RDF_UNDERWATER;

	if (contents & (CONTENTS_SOLID|CONTENTS_LAVA))
		SV_AddBlend (1.0, 0.3, 0.0, 0.6, ent->client->ps.blend);
	else if (contents & CONTENTS_SLIME)
		SV_AddBlend (0.0, 0.1, 0.05, 0.6, ent->client->ps.blend);
	else if (contents & CONTENTS_WATER)
		SV_AddBlend (0.5, 0.3, 0.2, 0.4, ent->client->ps.blend);
	//GHz: lowlight vision effect
	//decino: this is annoying
	//if (ent->client->ps.rdflags & RDF_IRGOGGLES)
	//	SV_AddBlend (1, 0, 0, 0.2, ent->client->ps.blend);
	//K03 Begin
	if (ent->client->cloaking && (ent->svflags & SVF_NOCLIENT))
		SV_AddBlend (-1, -1, -1, 0.3, ent->client->ps.blend);
	if(ent->client->bfg_blend)
		SV_AddBlend (0, 1, 0, 0.3, ent->client->ps.blend);
	//K03 End

	// add for powerups
	if (ent->client->quad_framenum > level.framenum)
	{
		remaining = ent->client->quad_framenum - level.framenum;
		if (remaining == 30)	// beginning to fade
			gi.sound(ent, CHAN_ITEM, gi.soundindex("items/damage2.wav"), 1, ATTN_NORM, 0);
		if (remaining > 30 || (remaining & 4) )
			SV_AddBlend (0, 0, 1, 0.08, ent->client->ps.blend);
	}
	// RAFAEL
	else if (ent->client->quadfire_framenum > level.framenum)
	{
		remaining = ent->client->quadfire_framenum - level.framenum;
		if (remaining == 30)	// beginning to fade
			gi.sound(ent, CHAN_ITEM, gi.soundindex("items/quadfire2.wav"), 1, ATTN_NORM, 0);
		if (remaining > 30 || (remaining & 4) )
			SV_AddBlend (1, 0.2, 0.5, 0.08, ent->client->ps.blend);
	}
	else if (ent->client->invincible_framenum > level.framenum)
	{
		remaining = ent->client->invincible_framenum - level.framenum;
		if (remaining == 30)	// beginning to fade
			gi.sound(ent, CHAN_ITEM, gi.soundindex("items/protect2.wav"), 1, ATTN_NORM, 0);
		if (remaining > 30 || (remaining & 4) )
			SV_AddBlend (1, 1, 0, 0.08, ent->client->ps.blend);
	}
	else if (ent->client->enviro_framenum > level.framenum)
	{
		remaining = ent->client->enviro_framenum - level.framenum;
		if (remaining == 30)	// beginning to fade
			gi.sound(ent, CHAN_ITEM, gi.soundindex("items/airout.wav"), 1, ATTN_NORM, 0);
		if (remaining > 30 || (remaining & 4) )
			SV_AddBlend (0, 1, 0, 0.08, ent->client->ps.blend);
	}
	else if (ent->client->breather_framenum > level.framenum)
	{
		remaining = ent->client->breather_framenum - level.framenum;
		if (remaining == 30)	// beginning to fade
			gi.sound(ent, CHAN_ITEM, gi.soundindex("items/airout.wav"), 1, ATTN_NORM, 0);
		if (remaining > 30 || (remaining & 4) )
			SV_AddBlend (0.4, 1, 0.4, 0.04, ent->client->ps.blend);
	}

	// add for damage
	if (ent->client->damage_alpha > 0)
		SV_AddBlend (ent->client->damage_blend[0],ent->client->damage_blend[1]
		,ent->client->damage_blend[2], ent->client->damage_alpha, ent->client->ps.blend);

	if (ent->client->bonus_alpha > 0)
		SV_AddBlend (0.85, 0.7, 0.3, ent->client->bonus_alpha, ent->client->ps.blend);

	// drop the damage value
	ent->client->damage_alpha -= 0.06;
	if (ent->client->damage_alpha < 0)
		ent->client->damage_alpha = 0;

	// drop the bonus value
	ent->client->bonus_alpha -= 0.1;
	if (ent->client->bonus_alpha < 0)
		ent->client->bonus_alpha = 0;

	//decino: add fury effect
	if(ent->fury_blendtime >= level.time + 0.2){
	VectorSet(ent->client->damage_blend, 1, 0, 0);
	ent->client->damage_alpha = 0.35;}

	if(ent->fury_blendtime >= level.time + 0.1){
	VectorSet(ent->client->damage_blend, 1, 1, 0);
	ent->client->damage_alpha = 0.35;}

	//decino: add screen blends to indicate you're in the totem's radius
	if(ent->totem_effect_earth >= level.time){
	VectorSet(ent->client->damage_blend, 1, 1, 0);
	ent->client->damage_alpha = 0.2;}

	if(ent->totem_effect_air >= level.time){
	VectorSet(ent->client->damage_blend, 1, 1, 1);
	ent->client->damage_alpha = 0.2;}

	//decino: black screen for people with the "Player" name
	if(ent->fail_name && !ent->ghetto_pc){
	VectorSet(ent->client->damage_blend, 0, 0, 0);
	ent->client->damage_alpha = 0.2;}

	//decino: we morphed, show it
	if(ent->morph_blendtime >= level.time){
	VectorSet(ent->client->damage_blend, 0, 1, 0);
	ent->client->damage_alpha = 0.2;}
}

/*
=================
P_FallingDamage
=================
*/
void P_FallingDamage (edict_t *ent)
{
	float	delta;
	int		damage;
	vec3_t	dir;

	if (ent->s.modelindex != 255)
		return;		// not in the player model

	if (ent->movetype == MOVETYPE_NOCLIP)
		return;

	if ((ent->client->oldvelocity[2] < 0) && (ent->velocity[2] > ent->client->oldvelocity[2]) && (!ent->groundentity))
	{
		// if we are airborne and using double-jump, we shouldn't take damage
		//FIXME: why are we taking fall damage when we are in the air in the first place???
		if (ent->v_flags & SFLG_DOUBLEJUMP)
			return;

		delta = ent->client->oldvelocity[2];
	}
	else
	{
		if (!ent->groundentity)
			return;
		delta = ent->velocity[2] - ent->client->oldvelocity[2];
	}
	delta = delta*delta * 0.0001;

	//K03 Begin
	if (ent->client->thrusting)
		return;
	if (ent->client->hook_state == HOOK_ON)
        return;
	//K03 End

	// never take falling damage if completely underwater
	if (ent->waterlevel == 3)
		return;
	if (ent->waterlevel == 2)
		delta *= 0.25;
	if (ent->waterlevel == 1)
		delta *= 0.5;

	if (delta < 1)
		return;

	if (delta < 15)
	{
		if ((ent->client->pers.inventory[ITEM_INDEX(FindItem("Stealth Boots"))] < 1))
		{
			if((ent->myskills.abilities[CLOAK].disable) || (ent->myskills.abilities[CLOAK].current_level < 1))
			{
				// morphed players don't make footsteps
				if (!ent->mtype)
					ent->s.event = EV_FOOTSTEP;

				PlayerNoise(ent, ent->s.origin, PNOISE_SELF);	//ponko

				// if the player is not crouched, then alert monsters of footsteps
				if (!(ent->client->ps.pmove.pm_flags & PMF_DUCKED))
					ent->lastsound = level.framenum;
			}
		}
		return;
	}

	ent->client->fall_value = delta*0.5;
	if (ent->client->fall_value > 40)
		ent->client->fall_value = 40;
	ent->client->fall_time = level.time + FALL_TIME;

	if (delta > 30)
	{
		if (ent->health > 0)
		{
			if (delta >= 55)
				ent->s.event = EV_FALLFAR;
			else
				ent->s.event = EV_FALL;
			PlayerNoise(ent, ent->s.origin, PNOISE_SELF); //ponko
		}
		ent->pain_debounce_time = level.time;	// no normal pain sound
		damage = (delta-30)/2;
		if (damage < 1)
			damage = 1;
		VectorSet (dir, 0, 0, 1);

		if (!deathmatch->value || !((int)dmflags->value & DF_NO_FALLING) )
			T_Damage (ent, world, world, dir, ent->s.origin, vec3_origin, damage, 0, 0, MOD_FALLING);
	}
	else
	{
		ent->s.event = EV_FALLSHORT;
		PlayerNoise(ent, ent->s.origin, PNOISE_SELF);	//ponko
		return;
	}
}



/*
=============
P_WorldEffects
=============
*/
void P_WorldEffects (void)
{
	qboolean	breather;
	qboolean	envirosuit;
	int			waterlevel, old_waterlevel;

	if (current_player->movetype == MOVETYPE_NOCLIP)
	{
		current_player->air_finished = level.time + 12;	// don't need air
		return;
	}

	//K03 Begin
	//if (HasActiveCurse(current_player, CURSE_FROZEN))
	if (que_typeexists(current_player->curses, CURSE_FROZEN))
		current_player->air_finished = level.time + 6;
	//K03 End

	waterlevel = current_player->waterlevel;
	old_waterlevel = current_client->old_waterlevel;
	current_client->old_waterlevel = waterlevel;

	breather = current_client->breather_framenum > level.framenum;
	envirosuit = current_client->enviro_framenum > level.framenum;

	//
	// if just entered a water volume, play a sound
	//
	if (!old_waterlevel && waterlevel)
	{
		if ((current_player->client->pers.inventory[ITEM_INDEX(FindItem("Stealth Boots"))] < 1))
		{
			if((current_player->myskills.abilities[CLOAK].disable) || (current_player->myskills.abilities[CLOAK].current_level))
			{
				current_player->lastsound = level.framenum; // trigger monsters
				PlayerNoise(current_player, current_player->s.origin, PNOISE_SELF);
				if (current_player->watertype & CONTENTS_LAVA)
					gi.sound (current_player, CHAN_BODY, gi.soundindex("player/lava_in.wav"), 1, ATTN_NORM, 0);
				else if (current_player->watertype & CONTENTS_SLIME)
					gi.sound (current_player, CHAN_BODY, gi.soundindex("player/watr_in.wav"), 1, ATTN_NORM, 0);
				else if (current_player->watertype & CONTENTS_WATER)
					gi.sound (current_player, CHAN_BODY, gi.soundindex("player/watr_in.wav"), 1, ATTN_NORM, 0);
			}
		}
		current_player->flags |= FL_INWATER;

		// clear damage_debounce, so the pain sound will play immediately
		current_player->damage_debounce_time = level.time - 1;
	}

	//
	// if just completely exited a water volume, play a sound
	//
	if (old_waterlevel && ! waterlevel)
	{
		if ((current_player->client->pers.inventory[ITEM_INDEX(FindItem("Stealth Boots"))]<1) && !current_player->mtype)
		{
			if((current_player->myskills.abilities[CLOAK].disable) || (current_player->myskills.abilities[CLOAK].current_level < 1))
			{
				current_player->lastsound = level.framenum; // trigger monsters
				PlayerNoise(current_player, current_player->s.origin, PNOISE_SELF);
				gi.sound (current_player, CHAN_BODY, gi.soundindex("player/watr_out.wav"), 1, ATTN_NORM, 0);
			}
		}
		current_player->flags &= ~FL_INWATER;
	}

	//
	// check for head just going under water
	//
	if (old_waterlevel != 3 && waterlevel == 3)
	{
		if (current_player->client)//K03
			gi.sound (current_player, CHAN_BODY, gi.soundindex("player/watr_un.wav"), 1, ATTN_NORM, 0);
	}

	//
	// check for head just coming out of water
	//
	if (old_waterlevel == 3 && waterlevel != 3)
	{
		if (current_player->air_finished < level.time)
		{	// gasp for air
			if (current_player->client)//K03
			{
			gi.sound (current_player, CHAN_VOICE, gi.soundindex("player/gasp1.wav"), 1, ATTN_NORM, 0);
			PlayerNoise(current_player, current_player->s.origin, PNOISE_SELF);
			}
		}
		else  if (current_player->air_finished < level.time + 11)
		{	// just break surface
			if (current_player->client)//K03
				gi.sound (current_player, CHAN_VOICE, gi.soundindex("player/gasp2.wav"), 1, ATTN_NORM, 0);
		}
	}

	//
	// check for drowning
	//
	if (waterlevel == 3)
	{
		// breather or envirosuit give air
		if (breather || envirosuit)
		{
			current_player->air_finished = level.time + 10;

			if (((int)(current_client->breather_framenum - level.framenum) % 25) == 0)
			{
				if (current_player->client)//K03
				{
				if (!current_client->breather_sound)
					gi.sound (current_player, CHAN_AUTO, gi.soundindex("player/u_breath1.wav"), 1, ATTN_NORM, 0);
				else
					gi.sound (current_player, CHAN_AUTO, gi.soundindex("player/u_breath2.wav"), 1, ATTN_NORM, 0);
				current_client->breather_sound ^= 1;
				PlayerNoise(current_player, current_player->s.origin, PNOISE_SELF);
				}
				//FIXME: release a bubble?
			}
		}

		// if out of air, start drowning
		if ((current_player->air_finished < level.time) 
			&& !((current_player->myskills.class_num == CLASS_POLTERGEIST) && (current_player->mtype == 0))
			&& (current_player->myskills.abilities[WORLD_RESIST].current_level < 1 
									|| HasFlag(current_player)) 
			&& !(current_player->flags & FL_GODMODE))
		{	// drown!
			if (current_player->client->next_drown_time < level.time 
				&& current_player->health > 0)
			{
				current_player->client->next_drown_time = level.time + 1;

				// take more damage the longer underwater
				current_player->dmg += 2;
				if (current_player->dmg > 15)
					current_player->dmg = 15;

				// play a gurp sound instead of a normal pain sound
				if (current_player->client)
				{
					if (current_player->health <= current_player->dmg)
						gi.sound (current_player, CHAN_VOICE, gi.soundindex("player/drown1.wav"), 1, ATTN_NORM, 0);
					else if (randomMT()&1)
						gi.sound (current_player, CHAN_VOICE, gi.soundindex("*gurp1.wav"), 1, ATTN_NORM, 0);
					else
						gi.sound (current_player, CHAN_VOICE, gi.soundindex("*gurp2.wav"), 1, ATTN_NORM, 0);
				}

				current_player->pain_debounce_time = level.time;

				T_Damage (current_player, world, world, vec3_origin, current_player->s.origin, vec3_origin, current_player->dmg, 0, DAMAGE_NO_ARMOR, MOD_WATER);
			}
		}
	}
	else
	{
		current_player->air_finished = level.time + 12;
		current_player->dmg = 2;
	}

	//
	// check for sizzle damage
	//
	if (waterlevel && (current_player->watertype & (CONTENTS_LAVA|CONTENTS_SLIME)) 
		&& (current_player->myskills.abilities[WORLD_RESIST].current_level < 1 || HasFlag(current_player)) 
		&& !(current_player->flags & FL_GODMODE) 
		&& !((current_player->myskills.class_num == CLASS_POLTERGEIST) && (current_player->mtype == 0)))
	{
		if (current_player->watertype & CONTENTS_LAVA)
		{
			if (current_player->health > 0
				&& current_player->pain_debounce_time <= level.time
				&& current_client->invincible_framenum < level.framenum)
			{
				if (current_player->client)//K03
				{
				if (randomMT()&1)
					gi.sound (current_player, CHAN_VOICE, gi.soundindex("player/burn1.wav"), 1, ATTN_NORM, 0);
				else
					gi.sound (current_player, CHAN_VOICE, gi.soundindex("player/burn2.wav"), 1, ATTN_NORM, 0);
				}
				current_player->pain_debounce_time = level.time + 1;
			}

			if (envirosuit)	// take 1/3 damage with envirosuit
				T_Damage (current_player, world, world, vec3_origin, current_player->s.origin, vec3_origin, 1*waterlevel, 0, 0, MOD_LAVA);
			else
				T_Damage (current_player, world, world, vec3_origin, current_player->s.origin, vec3_origin, 3*waterlevel, 0, 0, MOD_LAVA);
		}

		if (current_player->watertype & CONTENTS_SLIME)
		{
			if (!envirosuit)
			{	// no damage from slime with envirosuit
				T_Damage (current_player, world, world, vec3_origin, current_player->s.origin, vec3_origin, 1*waterlevel, 0, 0, MOD_SLIME);
			}
		}
	}
}


/*
===============
G_SetClientEffects
===============
*/
void G_SetClientEffects (edict_t *ent)
{
	int		pa_type;
	int		remaining;
	qboolean addoneffects=false;//K03

	// FIXME: this whole function is pretty ugly, clean it up!

//	gi.dprintf("G_SetClientEffects()\n");

	ent->s.effects = 0;
	ent->s.renderfx = 0;

	if (ent->health <= 0 || level.intermissiontime/* || ent->movetype == MOVETYPE_NOCLIP*/)
		return;

	//GHz: Make sure everyone is visible to lowlight vision
	if (!(ent->s.renderfx & RF_IR_VISIBLE))
       ent->s.renderfx |= RF_IR_VISIBLE; 

	if (level.time > ent->shield_activate_time)
	{
		if (ent->shield == 1)
		{
			ent->s.effects |= EF_POWERSCREEN;
		}
		else if (ent->shield == 2)
		{
			ent->s.effects |= (EF_SPHERETRANS);
	//ponko			ent->s.effects |= EF_COLOR_SHELL;
				ent->s.renderfx |= RF_SHELL_GREEN;
		}
	}

	if (ent->powerarmor_time > level.time)
	{
		pa_type = PowerArmorType (ent);
		if (pa_type == POWER_ARMOR_SCREEN)
		{
			ent->s.effects |= EF_POWERSCREEN;
		}
		else if (pa_type == POWER_ARMOR_SHIELD)
		{
			ent->s.effects |= (EF_SPHERETRANS);
//ponko			ent->s.effects |= EF_COLOR_SHELL;
			ent->s.renderfx |= RF_SHELL_GREEN;
		}
	}

	if (ent->client->quad_framenum > level.framenum)
	{
		remaining = ent->client->quad_framenum - level.framenum;
		if (remaining > 30 || (remaining & 4) )
		{
			ent->s.effects |= EF_QUAD;
			addoneffects = true;//K03
		}
		if (remaining == 30 && (ent->svflags & SVF_MONSTER))	// beginning to fade
			gi.sound(ent, CHAN_ITEM, gi.soundindex("items/damage2.wav"), 1, ATTN_NORM, 0);
	}

	// RAFAEL
	if (ent->client->quadfire_framenum > level.framenum)
	{
		remaining = ent->client->quadfire_framenum - level.framenum;
		if (remaining > 30 || (remaining & 4) )
		{
			ent->s.effects |= EF_QUAD;
			addoneffects = true;
		}
		if (remaining == 30 && (ent->svflags & SVF_MONSTER))	// beginning to fade
			gi.sound(ent, CHAN_ITEM, gi.soundindex("items/quadfire2.wav"), 1, ATTN_NORM, 0);
	}

	if (ent->client->invincible_framenum > level.framenum
//ZOID
		&& (level.framenum & 8)
//ZOID
		)
	{
		remaining = ent->client->invincible_framenum - level.framenum;
		if (remaining > 30 || (remaining & 4) )
		{
			ent->s.effects |= EF_PENT;
			addoneffects = true;//K03
		}
		if (remaining == 30 && (ent->svflags & SVF_MONSTER))	// beginning to fade
			gi.sound(ent, CHAN_ITEM, gi.soundindex("items/protect2.wav"), 1, ATTN_NORM, 0);
	}

	if (domination->value && ent->client->pers.inventory[flag_index])
	{
		ent->s.effects |= EF_COLOR_SHELL;
		if (ent->teamnum == 1)
		{
			ent->s.effects |= EF_FLAG1;
			ent->s.renderfx |= RF_SHELL_RED;
		}
		else
		{
			ent->s.effects |= EF_FLAG2;
			ent->s.renderfx |= RF_SHELL_BLUE;
		}
		addoneffects = true;
	}

	// CTF player effects
	if (ctf->value)
	{
		if (ent->client->pers.inventory[red_flag_index])
		{
			ent->s.effects |= EF_COLOR_SHELL;
			ent->s.effects |= EF_FLAG1;
			ent->s.renderfx |= RF_SHELL_RED;

			addoneffects = true; // this effect overrides other effects
		}
		else if (ent->client->pers.inventory[blue_flag_index])
		{
			ent->s.effects |= EF_COLOR_SHELL;
			ent->s.effects |= EF_FLAG2;
			ent->s.renderfx |= RF_SHELL_BLUE;

			addoneffects = true; // this effect overrides other effects
		}
		else if (que_typeexists(ent->auras, 0))
		{
			ent->s.effects |= EF_COLOR_SHELL;
			if (ent->teamnum == RED_TEAM)
				ent->s.renderfx |= RF_SHELL_RED;
			if (ent->teamnum == BLUE_TEAM)
				ent->s.renderfx |= RF_SHELL_BLUE;

			addoneffects = true; // this effect overrides other effects
		}
	}


	if (ptr->value && (ent->incontrol_time > level.time) && !(level.framenum&6))
	{
		ent->s.effects |= EF_COLOR_SHELL;
		ent->s.renderfx |= (RF_SHELL_YELLOW);
	}

	//K03 Begin
	
	if (!addoneffects && !ptr->value && !domination->value 
		&& !pvm->value && ((ent->myskills.boss > 0) || IsNewbieBasher(ent)))//K03 added else
	{
		ent->s.effects |= EF_COLOR_SHELL;
		ent->s.renderfx |= (RF_SHELL_YELLOW);
	}
	else if (!addoneffects && SPREE_WAR == true && ent == SPREE_DUDE)//K03 added else
	{
		ent->s.effects |= EF_COLOR_SHELL;
		ent->s.renderfx |= (RF_SHELL_RED|RF_SHELL_GREEN|RF_SHELL_BLUE);
	}
	else if (!addoneffects && SPREE_WAR == true && ent != SPREE_DUDE)
	{
		ent->s.effects |= EF_COLOR_SHELL;
		ent->s.renderfx |= (RF_SHELL_RED);
	}
	else if (!addoneffects && (ent->myskills.streak >= 6) 
		&& !pvm->value && !ptr->value && !domination->value && !ctf->value)
	{
		ent->s.effects |= EF_COLOR_SHELL;
		ent->s.renderfx |= (RF_SHELL_GREEN);
	}
	// you glow white if you are partially frozen
	else if (!addoneffects && (que_typeexists(ent->curses, AURA_HOLYFREEZE)
		|| que_typeexists(ent->curses, CURSE_FROZEN) || (ent->chill_time>level.time)))
//	else if (!addoneffects && (HasActiveCurse(ent, AURA_HOLYFREEZE) || HasActiveCurse(ent, CURSE_FROZEN)))
	{
		ent->client->ps.rdflags |= RDF_UNDERWATER;
		ent->s.effects |= EF_COLOR_SHELL;
		ent->s.renderfx |= (RF_SHELL_RED|RF_SHELL_GREEN|RF_SHELL_BLUE);
		SV_AddBlend (0.75, 0.75, 0.75, 0.6, ent->client->ps.blend);
	}
	else if (!addoneffects && ent->flags & FL_DETECTED && ent->detected_time > level.time && ent->mtype != M_FORCEWALL)
	{
		ent->s.effects |= EF_COLOR_SHELL|EF_TAGTRAIL;
		ent->s.renderfx |= (RF_SHELL_YELLOW);
	}
	// an active aura makes you glow cyan
	//else if (!addoneffects && HasActiveAura(ent, 0))
	else if (!addoneffects && que_typeexists(ent->auras, 0))
	{
		ent->s.effects |= EF_COLOR_SHELL;
		ent->s.renderfx |= (RF_SHELL_CYAN);
	}
	else if (!addoneffects && (ent->myskills.abilities[GHOST].current_level > 0) && !ent->mtype)
	{
		// 3.6 high level ghosts are almost completely transparent
	//	if (ent->myskills.abilities[GHOST].current_level >= 10)
	//		ent->s.effects |= EF_SPHERETRANS;
	//	else
			ent->s.effects |= EF_PLASMA;
	}
	/*
	else if (!addoneffects && ent->client->cloaking)
	{
		if (level.time + 1.0 <= ent->client->cloaktime)
			ent->s.effects |= EF_PLASMA;
		else
			ent->s.effects |= EF_SPHERETRANS;
	}
	*/
	
	if (!addoneffects && ent->superspeed)
	{
		ent->s.effects |= EF_TAGTRAIL;
	}
/*
	if (!addoneffects && (ent->myskills.cloak_level > 0) && (level.time > ent->client->ability_delay)
		&& (ent->myskills.class_num == CLASS_VAMPIRE) && !que_typeexists(ent->auras, 0))//!HasActiveAura(ent, 0))
	{
		if ((ent->myskills.ghost_level > 5) || (ent->client->idle_frames >= 5))
			ent->s.effects |= EF_SPHERETRANS;
		else
			ent->s.effects |= EF_PLASMA;
	}
*/
	if (!addoneffects && (level.time > ent->client->ability_delay) && !que_typeexists(ent->auras, 0) 
		&& !ent->myskills.abilities[CLOAK].disable && (ent->myskills.abilities[CLOAK].current_level > 0))
	{
		if (/*(ent->myskills.abilities[GHOST].current_level >= 5) 
			|| */(ent->client->idle_frames >= 5))
			ent->s.effects |= EF_SPHERETRANS;
		else
			ent->s.effects |= EF_PLASMA;
	}

	if (!addoneffects && (ent->myskills.abilities[PLAGUE].current_level || que_typeexists(ent->curses, CURSE_PLAGUE)))
	{
		ent->s.effects |= EF_FLIES;
	}
	if (ent->flags & FL_CHATPROTECT)
	{
		ent->s.effects |= EF_COLOR_SHELL;
		ent->s.renderfx |= RF_SHELL_RED;
		SV_AddBlend(0.5, 0, 0, 0.3, ent->client->ps.blend);
	}

	if(ent->manashield)   
		ent->s.effects |= 1073741824 | 536870912;

	// in team modes, morphed players glow their team color
	if (domination->value || pvm->value || ptr->value || ctf->value)
	{
		if (ent->mtype)
		{
			ent->s.effects |= EF_COLOR_SHELL;
			if (pvm->value)
				ent->s.renderfx |= RF_SHELL_BLUE;
			else if (ent->teamnum == 1)
				ent->s.renderfx |= RF_SHELL_RED;
			else if (ent->teamnum == 2)
				ent->s.renderfx |= RF_SHELL_BLUE;
		}
	}
	//K03 End
}


/*
===============
G_SetClientEvent
===============
*/
void G_SetClientEvent (edict_t *ent)
{
	if (ent->s.event)
		return;

	if ( ent->groundentity && xyspeed > 225)
	{
		// called with each player step
		if ( (int)(current_client->bobtime+bobmove) != bobcycle 
			&& (ent->client->pers.inventory[ITEM_INDEX(FindItem("Stealth Boots"))] < 1)	&& !ent->mtype)
		{
			if ((ent->myskills.abilities[CLOAK].disable) || (ent->myskills.abilities[CLOAK].current_level < 1))
			ent->s.event = EV_FOOTSTEP;
			PlayerNoise(ent, ent->s.origin, PNOISE_SELF); //ponko

			// if the player is not crouched, then alert monsters of footsteps
			if (ent->client && !(ent->client->ps.pmove.pm_flags & PMF_DUCKED))
				ent->lastsound = level.framenum;
		}
	}
}

/*
===============
G_SetClientSound
===============
*/
void G_SetClientSound (edict_t *ent)
{
	char	*weap;

	if (ent->client->resp.game_helpchanged != game.helpchanged)
	{
		ent->client->resp.game_helpchanged = game.helpchanged;
		ent->client->resp.helpchanged = 1;
	}

	// help beep (no more than three times)
	if (ent->client->resp.helpchanged && ent->client->resp.helpchanged <= 3 && !(level.framenum&63) )
	{
		ent->client->resp.helpchanged++;
		gi.sound (ent, CHAN_VOICE, gi.soundindex ("misc/pc_up.wav"), 1, ATTN_STATIC, 0);
	}


	if (ent->client->pers.weapon)
		weap = ent->client->pers.weapon->classname;
	else
		weap = "";

	if (ent->waterlevel && (ent->watertype&(CONTENTS_LAVA|CONTENTS_SLIME)) )
		ent->s.sound = snd_fry;
	else if (strcmp(weap, "weapon_railgun") == 0)
		ent->s.sound = gi.soundindex("weapons/rg_hum.wav");
	else if (strcmp(weap, "weapon_bfg") == 0)
		ent->s.sound = gi.soundindex("weapons/bfg_hum.wav");
	else if (ent->client->weapon_sound)
		ent->s.sound = ent->client->weapon_sound;
	else
		ent->s.sound = 0;
}

/*
===============
G_SetClientFrame
===============
*/
void G_SetClientFrame (edict_t *ent)
{
	gclient_t	*client;
	qboolean	duck, run;

	if (ent->s.modelindex != 255)
		return; // not in the player model

	client = ent->client;

	if (client->ps.pmove.pm_flags & PMF_DUCKED)
		duck = true;
	else
		duck = false;
	if (xyspeed)
		run = true;
	else
		run = false;

	// check for stand/duck and stop/go transitions
	if (duck != client->anim_duck && client->anim_priority < ANIM_DEATH)
		goto newanim;
	if (run != client->anim_run && client->anim_priority == ANIM_BASIC)
		goto newanim;
	if (!ent->groundentity && client->anim_priority <= ANIM_WAVE)
		goto newanim;

	// ### Hentai ### BEGIN
	if(client->anim_priority == ANIM_REVERSE)
	{
		if(ent->s.frame > client->anim_end)
		{
			ent->s.frame--;
			return;
		}
	}
	else if (ent->s.frame < client->anim_end) // ### Hentai ### END
	{	// continue an animation
		ent->s.frame++;
		return;
	}

	if (client->anim_priority == ANIM_DEATH)
		return;		// stay there
	if (client->anim_priority == ANIM_JUMP)
	{
		if (!ent->groundentity)
			return;		// stay there
		ent->client->anim_priority = ANIM_WAVE;
		ent->s.frame = FRAME_jump3;
		ent->client->anim_end = FRAME_jump6;
		return;
	}

newanim:
	// return to either a running or standing frame
	client->anim_priority = ANIM_BASIC;
	client->anim_duck = duck;
	client->anim_run = run;

	if (!ent->groundentity)
	{
		client->anim_priority = ANIM_JUMP;
		if (ent->s.frame != FRAME_jump2)
			ent->s.frame = FRAME_jump1;
		client->anim_end = FRAME_jump2;
		
	}
	else if (run)
	{	// running
		if (duck)
		{
			ent->s.frame = FRAME_crwalk1;
			client->anim_end = FRAME_crwalk6;
		}
		else
		{
			ent->s.frame = FRAME_run1;
			client->anim_end = FRAME_run6;
		}
	}
	else
	{	// standing
		if (duck)
		{
			ent->s.frame = FRAME_crstnd01;
			client->anim_end = FRAME_crstnd19;
		}
		else
		{
			ent->s.frame = FRAME_stand01;
			client->anim_end = FRAME_stand40;
		}
	}
}

void P_FollowWall (edict_t *ent)
{
	int		turns=0, num=90;
	vec3_t	forward, right, start, end;
	vec3_t	offset, angles;
	trace_t	tr;
	
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorSet(offset, 0, 7,  ent->viewheight-8);
	P_ProjectSource(ent->client, ent->s.origin, offset, forward, right, start);
	VectorMA(start, 8192, forward, end);
	tr = gi.trace(start, NULL, NULL, end, ent, MASK_SOLID);
	VectorCopy(tr.endpos, end);
	vectoangles(tr.plane.normal, angles);
	ValidateAngles(angles);
	VectorCopy(tr.endpos, start);

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_BFG_LASER);
	gi.WritePosition (ent->s.origin);
	gi.WritePosition (tr.endpos);
	gi.multicast (ent->s.origin, MULTICAST_PHS);

	while (turns < 10)
	{
		angles[YAW] += num;	
		forward[0] = cos(DEG2RAD(angles[YAW]));
		forward[1] = sin(DEG2RAD(angles[YAW]));
		forward[2] = 0;
		VectorMA(start, 8192, forward, end);
		tr = gi.trace(start, NULL, NULL, end, ent, MASK_SOLID);

		if (tr.startsolid || tr.allsolid)
		{
			num = -90;
			gi.dprintf("fail\n");
			continue;
		}

		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_BFG_LASER);
		gi.WritePosition (start);
		gi.WritePosition (tr.endpos);
		gi.multicast (start, MULTICAST_PHS);

		VectorCopy(tr.endpos, start);
		vectoangles(tr.plane.normal, angles);
		ValidateAngles(angles);
		turns++;
		//gi.dprintf("turn %d %d degrees\n", turns, (int)angles[YAW]);
	}
/*
	while (1)
	{
		angles[YAW] += num;	
		forward[0] = cos(DEG2RAD(angles[YAW]));
		forward[1] = sin(DEG2RAD(angles[YAW]));
		forward[2] = 0;
		VectorMA(start, 48, forward, end);
		tr = gi.trace(start, NULL, NULL, end, ent, MASK_SOLID);
		if (tr.fraction < 1)
		{
			if (num == -180)
				break;
			num = -180;
			continue;
		}
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_BFG_LASER);
		gi.WritePosition (start);
		gi.WritePosition (tr.endpos);
		gi.multicast (start, MULTICAST_PHS);
		break;
	}
*/
}
	

void ShowAllyInfo (edict_t *ent);

/*
=================
ClientEndServerFrame

Called for each player at the end of the server frame
and right after spawning
=================
*/
void ClientEndServerFrame (edict_t *ent)
{
	float	bobtime;
	int		i, updateFrame;
//	char	string[1024];

	current_player = ent;
	current_client = ent->client;

	//
	// If the origin or velocity have changed since ClientThink(),
	// update the pmove values.  This will happen when the client
	// is pushed by a bmodel or kicked by an explosion.
	// 
	// If it wasn't updated here, the view position would lag a frame
	// behind the body position when pushed -- "sinking into plats"
	//
	for (i=0 ; i<3 ; i++)
	{
		current_client->ps.pmove.origin[i] = ent->s.origin[i]*8.0;
		current_client->ps.pmove.velocity[i] = ent->velocity[i]*8.0;
	}

	//
	// If the end of unit layout is displayed, don't give
	// the player any normal movement attributes
	//
	if (level.intermissiontime)
	{
		// FIXME: add view drifting here?
		current_client->ps.blend[3] = 0;
		current_client->ps.fov = 90;
		G_SetStats (ent);
		return;
	}

	AngleVectors (ent->client->v_angle, forward, right, up);

	// burn from lava, etc
	P_WorldEffects ();

	//
	// set model angles from view angles so other things in
	// the world can tell which direction you are looking
	//
	if (ent->client->v_angle[PITCH] > 180)
		ent->s.angles[PITCH] = (-360 + ent->client->v_angle[PITCH])/3;
	else
		ent->s.angles[PITCH] = ent->client->v_angle[PITCH]/3;
	//GHz START
	// monsters don't use pitch
	if (ent->mtype)
		ent->s.angles[PITCH] = 0;
	//GHz END
	ent->s.angles[YAW] = ent->client->v_angle[YAW];
	ent->s.angles[ROLL] = 0;
	ent->s.angles[ROLL] = SV_CalcRoll (ent->s.angles, ent->velocity)*4;

	//
	// calculate speed and cycle to be used for
	// all cyclic walking effects
	//
	xyspeed = sqrt(ent->velocity[0]*ent->velocity[0] + ent->velocity[1]*ent->velocity[1]);

	if (xyspeed < 5)
	{
		bobmove = 0;
		current_client->bobtime = 0;	// start at beginning of cycle again
	}
	else if (ent->groundentity)
	{	// so bobbing only cycles when on ground
		if (xyspeed > 210)
			bobmove = 0.25;
		else if (xyspeed > 100)
			bobmove = 0.125;
		else
			bobmove = 0.0625;
	}
	
	bobtime = (current_client->bobtime += bobmove);

	if (current_client->ps.pmove.pm_flags & PMF_DUCKED)
		bobtime *= 4;

	bobcycle = (int)bobtime;
	bobfracsin = fabs(sin(bobtime*M_PI));

	// detect hitting the floor
	P_FallingDamage (ent);

	// apply all the damage taken this frame
	P_DamageFeedback (ent);

	// determine the view offsets
	SV_CalcViewOffset (ent);

	// determine the gun offsets
	SV_CalcGunOffset (ent);

	// determine the full screen color blend
	// must be after viewoffset, so eye contents can be
	// accurately determined
	// FIXME: with client prediction, the contents
	// should be determined by the client
	SV_CalcBlend (ent);

	// chase cam stuff
	if (ent->client->resp.spectator)
		G_SetSpectatorStats(ent);
	else
		G_SetStats (ent);

	G_CheckChaseStats(ent);

	G_SetClientEvent (ent);

	G_SetClientEffects (ent);

	G_SetClientSound (ent);

	G_SetClientFrame (ent);

	if (ent->client->showbuffs && !ent->client->showinventory && !ent->client->showscores){
	BuffIndicator(ent);}

	VectorCopy (ent->velocity, ent->client->oldvelocity);
	VectorCopy (ent->client->ps.viewangles, ent->client->oldviewangles);

	// clear weapon kicks
	VectorClear (ent->client->kick_origin);
	VectorClear (ent->client->kick_angles);

	if (ent->client->pers.scanner_active & 1)
		updateFrame = SCANNER_UPDATE_FREQ;
	else
		updateFrame = 31;

	if (!ent->client->showbuffs){
	// if the scoreboard is up, update it
	//if (ent->client->showscores && !(level.framenum & 31) && !ent->client->menustorage.menu_active)
	if (((ent->client->showscores || ent->client->pers.scanner_active) && !ent->client->menustorage.menu_active && !ent->client->show_allyinfo
		&& !(level.framenum % updateFrame)) || (ent->client->pers.scanner_active & 2))
	{
		DeathmatchScoreboardMessage (ent, ent->enemy);
		gi.unicast (ent, false);
	}
	else if (ent->client->show_allyinfo && !(level.framenum%10))
	{
		ShowAllyInfo(ent);
	}
	//GHz START
	else if (!ent->client->showscores && ent->client->menustorage.menu_active)
	{
		closemenu(ent);
	}
}
	//GHz END
	else{
	BuffIndicator (ent);
	gi.unicast (ent, false);}

//	P_FollowWall(ent);
}