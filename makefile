CC=gcc
COCKBAGS_MAKEFILE=makefile
SHARED_FLAGS:=
RELEASE_CFLAGS=-Isource/ -I./ -I../ $(SHARED_FLAGS) -O3
DEBUG_CFLAGS=-g -Isource/ -I./ -I../ $(SHARED_FLAGS) -DC_ONLY
LDFLAGS=-lm -shared
#DED_LDFLAGS=-ldl -lm -lz
#MODULE_LDFLAGS=-lm
#X11_LDFLAGS=-L/usr/X11R6/lib -lX11 -lXext

SHLIBCFLAGS=
SHLIBLDFLAGS=-shared

DO_SHLIB_CC=$(CC) $(CFLAGS) $(SHLIBCFLAGS) -o $@ -c $<

# this nice line comes from the linux kernel makefile
#ARCH := $(shell uname -m | sed -e s/i.86/i386/ -e s/sun4u/sparc/ -e s/sparc64/sparc/ -e s/arm.*/arm/ -e s/sa110/arm/ -e s/alpha/axp/)
ARCH =i386
SHLIBEXT =so

BUILD_DEBUG_DIR=debug$(ARCH)
BUILD_RELEASE_DIR=release$(ARCH)
TARGETS=$(BUILDDIR)/game.so

all: release

debug:
	@-mkdir -p $(BUILD_DEBUG_DIR)
	$(MAKE) -f $(COCKBAGS_MAKEFILE) targets BUILDDIR=$(BUILD_DEBUG_DIR) SOURCEDIR=. CFLAGS="$(DEBUG_CFLAGS) -DLINUX_VERSION='\"$(VERSION) Debug\"'"

release:
	@-mkdir -p $(BUILD_RELEASE_DIR)
	$(MAKE) -f $(COCKBAGS_MAKEFILE) targets BUILDDIR=$(BUILD_RELEASE_DIR) SOURCEDIR=. CFLAGS="$(RELEASE_CFLAGS) -g -fPIC -std=c99 -shared"

targets: $(TARGETS)

#
# Game object files
#
OBJS_GAME=\
	$(BUILDDIR)/ally.o \
	$(BUILDDIR)/armory.o \
	$(BUILDDIR)/auras.o \
	$(BUILDDIR)/backpack.o \
	$(BUILDDIR)/bombspell.o \
	$(BUILDDIR)/boss_general.o \
	$(BUILDDIR)/boss_makron.o \
	$(BUILDDIR)/boss_tank.o \
	$(BUILDDIR)/class_brain.o \
	$(BUILDDIR)/class_demon.o \
	$(BUILDDIR)/cloak.o \
	$(BUILDDIR)/ctf.o \
	$(BUILDDIR)/damage.o \
	$(BUILDDIR)/domination.o \
	$(BUILDDIR)/drone_ai.o \
	$(BUILDDIR)/drone_bitch.o \
	$(BUILDDIR)/drone_brain.o \
	$(BUILDDIR)/drone_decoy.o \
	$(BUILDDIR)/drone_gladiator.o \
	$(BUILDDIR)/drone_gunner.o \
	$(BUILDDIR)/drone_medic.o \
	$(BUILDDIR)/drone_misc.o \
	$(BUILDDIR)/drone_move.o \
	$(BUILDDIR)/drone_mutant.o \
	$(BUILDDIR)/drone_parasite.o \
	$(BUILDDIR)/drone_retard.o \
	$(BUILDDIR)/drone_soldier.o \
	$(BUILDDIR)/drone_tank.o \
	$(BUILDDIR)/ents.o \
	$(BUILDDIR)/file_output.o \
	$(BUILDDIR)/flying_skull.o \
	$(BUILDDIR)/forcewall.o \
	$(BUILDDIR)/g_chase.o \
	$(BUILDDIR)/g_cmds.o \
	$(BUILDDIR)/g_combat.o \
	$(BUILDDIR)/gds.o \
	$(BUILDDIR)/g_flame.o \
	$(BUILDDIR)/g_func.o \
	$(BUILDDIR)/g_items.o \
	$(BUILDDIR)/g_lasers.o \
	$(BUILDDIR)/g_main.o \
	$(BUILDDIR)/g_misc.o \
	$(BUILDDIR)/g_monster.o \
	$(BUILDDIR)/g_phys.o \
	$(BUILDDIR)/g_save.o \
	$(BUILDDIR)/g_spawn.o \
	$(BUILDDIR)/g_svcmds.o \
	$(BUILDDIR)/g_sword.o \
	$(BUILDDIR)/g_target.o \
	$(BUILDDIR)/g_trigger.o \
	$(BUILDDIR)/g_utils.o \
	$(BUILDDIR)/g_weapon.o \
	$(BUILDDIR)/help.o \
	$(BUILDDIR)/invasion.o \
	$(BUILDDIR)/item_menu.o \
	$(BUILDDIR)/jetpack.o \
	$(BUILDDIR)/lasers.o \
	$(BUILDDIR)/lasersight.o \
	$(BUILDDIR)/laserstuff.o \
	$(BUILDDIR)/magic.o \
	$(BUILDDIR)/maplist.o \
	$(BUILDDIR)/menu.o \
	$(BUILDDIR)/mersennetwister.o \
	$(BUILDDIR)/m_flash.o \
	$(BUILDDIR)/minisentry.o \
	$(BUILDDIR)/misc_stuff.o \
	$(BUILDDIR)/p_client.o \
	$(BUILDDIR)/p_hook.o \
	$(BUILDDIR)/p_hud.o \
	$(BUILDDIR)/player.o \
	$(BUILDDIR)/playerlog.o \
	$(BUILDDIR)/player_points.o \
	$(BUILDDIR)/playertoberserk.o \
	$(BUILDDIR)/playertoflyer.o \
	$(BUILDDIR)/playertomedic.o \
	$(BUILDDIR)/playertomutant.o \
	$(BUILDDIR)/playertoparasite.o \
	$(BUILDDIR)/playertotank.o \
	$(BUILDDIR)/p_menu.o \
	$(BUILDDIR)/p_parasite.o \
	$(BUILDDIR)/p_trail.o \
	$(BUILDDIR)/pvb.o \
	$(BUILDDIR)/p_view.o \
	$(BUILDDIR)/p_weapon.o \
	$(BUILDDIR)/q_shared.o \
	$(BUILDDIR)/repairstation.o \
	$(BUILDDIR)/runes.o \
	$(BUILDDIR)/scanner.o \
	$(BUILDDIR)/sentrygun2.o \
	$(BUILDDIR)/shaman.o \
	$(BUILDDIR)/special_items.o \
	$(BUILDDIR)/spirit.o \
	$(BUILDDIR)/supplystation.o \
	$(BUILDDIR)/talents.o \
	$(BUILDDIR)/teamplay.o \
	$(BUILDDIR)/totems.o \
	$(BUILDDIR)/trade.o \
	$(BUILDDIR)/upgrades.o \
	$(BUILDDIR)/v_file_io.o \
	$(BUILDDIR)/v_items.o \
	$(BUILDDIR)/v_maplist.o \
	$(BUILDDIR)/vote.o \
	$(BUILDDIR)/v_utils.o \
	$(BUILDDIR)/weapons.o \
	$(BUILDDIR)/weapon_upgrades.o

$(BUILDDIR)/game.so: $(OBJS_GAME)
	@echo Linking...;
	$(CC) $(CFLAGS) -o $@ $(OBJS_GAME) $(LDFLAGS) $(MODULE_LDFLAGS)


$(BUILDDIR)/ally.o: $(SOURCEDIR)/ally.c; $(DO_SHLIB_CC)
$(BUILDDIR)/armory.o: $(SOURCEDIR)/armory.c; $(DO_SHLIB_CC)
$(BUILDDIR)/auras.o: $(SOURCEDIR)/auras.c; $(DO_SHLIB_CC)
$(BUILDDIR)/backpack.o: $(SOURCEDIR)/backpack.c; $(DO_SHLIB_CC)
$(BUILDDIR)/bombspell.o: $(SOURCEDIR)/bombspell.c; $(DO_SHLIB_CC)
$(BUILDDIR)/boss_general.o: $(SOURCEDIR)/boss_general.c; $(DO_SHLIB_CC)
$(BUILDDIR)/boss_makron.o: $(SOURCEDIR)/boss_makron.c; $(DO_SHLIB_CC)
$(BUILDDIR)/boss_tank.o: $(SOURCEDIR)/boss_tank.c; $(DO_SHLIB_CC)
$(BUILDDIR)/class_brain.o: $(SOURCEDIR)/class_brain.c; $(DO_SHLIB_CC)
$(BUILDDIR)/class_demon.o: $(SOURCEDIR)/class_demon.c; $(DO_SHLIB_CC)
$(BUILDDIR)/cloak.o: $(SOURCEDIR)/cloak.c; $(DO_SHLIB_CC)
$(BUILDDIR)/ctf.o: $(SOURCEDIR)/ctf.c; $(DO_SHLIB_CC)
$(BUILDDIR)/damage.o: $(SOURCEDIR)/damage.c; $(DO_SHLIB_CC)
$(BUILDDIR)/domination.o: $(SOURCEDIR)/domination.c; $(DO_SHLIB_CC)
$(BUILDDIR)/drone_ai.o: $(SOURCEDIR)/drone_ai.c; $(DO_SHLIB_CC)
$(BUILDDIR)/drone_bitch.o: $(SOURCEDIR)/drone_bitch.c; $(DO_SHLIB_CC)
$(BUILDDIR)/drone_brain.o: $(SOURCEDIR)/drone_brain.c; $(DO_SHLIB_CC)
$(BUILDDIR)/drone_decoy.o: $(SOURCEDIR)/drone_decoy.c; $(DO_SHLIB_CC)
$(BUILDDIR)/drone_gladiator.o: $(SOURCEDIR)/drone_gladiator.c; $(DO_SHLIB_CC)
$(BUILDDIR)/drone_gunner.o: $(SOURCEDIR)/drone_gunner.c; $(DO_SHLIB_CC)
$(BUILDDIR)/drone_medic.o: $(SOURCEDIR)/drone_medic.c; $(DO_SHLIB_CC)
$(BUILDDIR)/drone_misc.o: $(SOURCEDIR)/drone_misc.c; $(DO_SHLIB_CC)
$(BUILDDIR)/drone_move.o: $(SOURCEDIR)/drone_move.c; $(DO_SHLIB_CC)
$(BUILDDIR)/drone_mutant.o: $(SOURCEDIR)/drone_mutant.c; $(DO_SHLIB_CC)
$(BUILDDIR)/drone_parasite.o: $(SOURCEDIR)/drone_parasite.c; $(DO_SHLIB_CC)
$(BUILDDIR)/drone_retard.o: $(SOURCEDIR)/drone_retard.c; $(DO_SHLIB_CC)
$(BUILDDIR)/drone_soldier.o: $(SOURCEDIR)/drone_soldier.c; $(DO_SHLIB_CC)
$(BUILDDIR)/drone_tank.o: $(SOURCEDIR)/drone_tank.c; $(DO_SHLIB_CC)
$(BUILDDIR)/ents.o: $(SOURCEDIR)/ents.c; $(DO_SHLIB_CC)
$(BUILDDIR)/file_output.o: $(SOURCEDIR)/file_output.c; $(DO_SHLIB_CC)
$(BUILDDIR)/flying_skull.o: $(SOURCEDIR)/flying_skull.c; $(DO_SHLIB_CC)
$(BUILDDIR)/forcewall.o: $(SOURCEDIR)/forcewall.c; $(DO_SHLIB_CC)
$(BUILDDIR)/g_chase.o: $(SOURCEDIR)/g_chase.c; $(DO_SHLIB_CC)
$(BUILDDIR)/g_cmds.o: $(SOURCEDIR)/g_cmds.c; $(DO_SHLIB_CC)
$(BUILDDIR)/g_combat.o: $(SOURCEDIR)/g_combat.c; $(DO_SHLIB_CC)
$(BUILDDIR)/gds.o: $(SOURCEDIR)/gds.c; $(DO_SHLIB_CC)
$(BUILDDIR)/g_flame.o: $(SOURCEDIR)/g_flame.c; $(DO_SHLIB_CC)
$(BUILDDIR)/g_func.o: $(SOURCEDIR)/g_func.c; $(DO_SHLIB_CC)
$(BUILDDIR)/g_items.o: $(SOURCEDIR)/g_items.c; $(DO_SHLIB_CC)
$(BUILDDIR)/g_lasers.o: $(SOURCEDIR)/g_lasers.c; $(DO_SHLIB_CC)
$(BUILDDIR)/g_main.o: $(SOURCEDIR)/g_main.c; $(DO_SHLIB_CC)
$(BUILDDIR)/g_misc.o: $(SOURCEDIR)/g_misc.c; $(DO_SHLIB_CC)
$(BUILDDIR)/g_monster.o: $(SOURCEDIR)/g_monster.c; $(DO_SHLIB_CC)
$(BUILDDIR)/g_phys.o: $(SOURCEDIR)/g_phys.c; $(DO_SHLIB_CC)
$(BUILDDIR)/g_save.o: $(SOURCEDIR)/g_save.c; $(DO_SHLIB_CC)
$(BUILDDIR)/g_spawn.o: $(SOURCEDIR)/g_spawn.c; $(DO_SHLIB_CC)
$(BUILDDIR)/g_svcmds.o: $(SOURCEDIR)/g_svcmds.c; $(DO_SHLIB_CC)
$(BUILDDIR)/g_sword.o: $(SOURCEDIR)/g_sword.c; $(DO_SHLIB_CC)
$(BUILDDIR)/g_target.o: $(SOURCEDIR)/g_target.c; $(DO_SHLIB_CC)
$(BUILDDIR)/g_trigger.o: $(SOURCEDIR)/g_trigger.c; $(DO_SHLIB_CC)
$(BUILDDIR)/g_utils.o: $(SOURCEDIR)/g_utils.c; $(DO_SHLIB_CC)
$(BUILDDIR)/g_weapon.o: $(SOURCEDIR)/g_weapon.c; $(DO_SHLIB_CC)
$(BUILDDIR)/help.o: $(SOURCEDIR)/help.c; $(DO_SHLIB_CC)
$(BUILDDIR)/invasion.o: $(SOURCEDIR)/invasion.c; $(DO_SHLIB_CC)
$(BUILDDIR)/item_menu.o: $(SOURCEDIR)/item_menu.c; $(DO_SHLIB_CC)
$(BUILDDIR)/jetpack.o: $(SOURCEDIR)/jetpack.c; $(DO_SHLIB_CC)
$(BUILDDIR)/lasers.o: $(SOURCEDIR)/lasers.c; $(DO_SHLIB_CC)
$(BUILDDIR)/lasersight.o: $(SOURCEDIR)/lasersight.c; $(DO_SHLIB_CC)
$(BUILDDIR)/laserstuff.o: $(SOURCEDIR)/laserstuff.c; $(DO_SHLIB_CC)
$(BUILDDIR)/magic.o: $(SOURCEDIR)/magic.c; $(DO_SHLIB_CC)
$(BUILDDIR)/maplist.o: $(SOURCEDIR)/maplist.c; $(DO_SHLIB_CC)
$(BUILDDIR)/menu.o: $(SOURCEDIR)/menu.c; $(DO_SHLIB_CC)
$(BUILDDIR)/mersennetwister.o: $(SOURCEDIR)/mersennetwister.c; $(DO_SHLIB_CC)
$(BUILDDIR)/m_flash.o: $(SOURCEDIR)/m_flash.c; $(DO_SHLIB_CC)
$(BUILDDIR)/minisentry.o: $(SOURCEDIR)/minisentry.c; $(DO_SHLIB_CC)
$(BUILDDIR)/misc_stuff.o: $(SOURCEDIR)/misc_stuff.c; $(DO_SHLIB_CC)
$(BUILDDIR)/p_client.o: $(SOURCEDIR)/p_client.c; $(DO_SHLIB_CC)
$(BUILDDIR)/p_hook.o: $(SOURCEDIR)/p_hook.c; $(DO_SHLIB_CC)
$(BUILDDIR)/p_hud.o: $(SOURCEDIR)/p_hud.c; $(DO_SHLIB_CC)
$(BUILDDIR)/player.o: $(SOURCEDIR)/player.c; $(DO_SHLIB_CC)
$(BUILDDIR)/playerlog.o: $(SOURCEDIR)/playerlog.c; $(DO_SHLIB_CC)
$(BUILDDIR)/player_points.o: $(SOURCEDIR)/player_points.c; $(DO_SHLIB_CC)
$(BUILDDIR)/playertoberserk.o: $(SOURCEDIR)/playertoberserk.c; $(DO_SHLIB_CC)
$(BUILDDIR)/playertoflyer.o: $(SOURCEDIR)/playertoflyer.c; $(DO_SHLIB_CC)
$(BUILDDIR)/playertomedic.o: $(SOURCEDIR)/playertomedic.c; $(DO_SHLIB_CC)
$(BUILDDIR)/playertomutant.o: $(SOURCEDIR)/playertomutant.c; $(DO_SHLIB_CC)
$(BUILDDIR)/playertoparasite.o: $(SOURCEDIR)/playertoparasite.c; $(DO_SHLIB_CC)
$(BUILDDIR)/playertotank.o: $(SOURCEDIR)/playertotank.c; $(DO_SHLIB_CC)
$(BUILDDIR)/p_menu.o: $(SOURCEDIR)/p_menu.c; $(DO_SHLIB_CC)
$(BUILDDIR)/p_parasite.o: $(SOURCEDIR)/p_parasite.c; $(DO_SHLIB_CC)
$(BUILDDIR)/p_trail.o: $(SOURCEDIR)/p_trail.c; $(DO_SHLIB_CC)
$(BUILDDIR)/pvb.o: $(SOURCEDIR)/pvb.c; $(DO_SHLIB_CC)
$(BUILDDIR)/p_view.o: $(SOURCEDIR)/p_view.c; $(DO_SHLIB_CC)
$(BUILDDIR)/p_weapon.o: $(SOURCEDIR)/p_weapon.c; $(DO_SHLIB_CC)
$(BUILDDIR)/q_shared.o: $(SOURCEDIR)/q_shared.c; $(DO_SHLIB_CC)
$(BUILDDIR)/repairstation.o: $(SOURCEDIR)/repairstation.c; $(DO_SHLIB_CC)
$(BUILDDIR)/runes.o: $(SOURCEDIR)/runes.c; $(DO_SHLIB_CC)
$(BUILDDIR)/scanner.o: $(SOURCEDIR)/scanner.c; $(DO_SHLIB_CC)
$(BUILDDIR)/sentrygun2.o: $(SOURCEDIR)/sentrygun2.c; $(DO_SHLIB_CC)
$(BUILDDIR)/shaman.o: $(SOURCEDIR)/shaman.c; $(DO_SHLIB_CC)
$(BUILDDIR)/special_items.o: $(SOURCEDIR)/special_items.c; $(DO_SHLIB_CC)
$(BUILDDIR)/spirit.o: $(SOURCEDIR)/spirit.c; $(DO_SHLIB_CC)
$(BUILDDIR)/supplystation.o: $(SOURCEDIR)/supplystation.c; $(DO_SHLIB_CC)
$(BUILDDIR)/talents.o: $(SOURCEDIR)/talents.c; $(DO_SHLIB_CC)
$(BUILDDIR)/teamplay.o: $(SOURCEDIR)/teamplay.c; $(DO_SHLIB_CC)
$(BUILDDIR)/totems.o: $(SOURCEDIR)/totems.c; $(DO_SHLIB_CC)
$(BUILDDIR)/trade.o: $(SOURCEDIR)/trade.c; $(DO_SHLIB_CC)
$(BUILDDIR)/upgrades.o: $(SOURCEDIR)/upgrades.c; $(DO_SHLIB_CC)
$(BUILDDIR)/v_file_io.o: $(SOURCEDIR)/v_file_io.c; $(DO_SHLIB_CC)
$(BUILDDIR)/v_items.o: $(SOURCEDIR)/v_items.c; $(DO_SHLIB_CC)
$(BUILDDIR)/v_maplist.o: $(SOURCEDIR)/v_maplist.c; $(DO_SHLIB_CC)
$(BUILDDIR)/vote.o: $(SOURCEDIR)/vote.c; $(DO_SHLIB_CC)
$(BUILDDIR)/v_utils.o: $(SOURCEDIR)/v_utils.c; $(DO_SHLIB_CC)
$(BUILDDIR)/weapons.o: $(SOURCEDIR)/weapons.c; $(DO_SHLIB_CC)
$(BUILDDIR)/weapon_upgrades.o: $(SOURCEDIR)/weapon_upgrades.c; $(DO_SHLIB_CC)

#
# Parameters
#
clean: clean-debug clean-release

clean-debug:
	$(MAKE) -f $(COCKBAGS_MAKEFILE) clean2 BUILDDIR=$(BUILD_DEBUG_DIR)

clean-release:
	$(MAKE) -f $(COCKBAGS_MAKEFILE) clean2 BUILDDIR=$(BUILD_RELEASE_DIR)

clean2:
	@-rm -f \
	$(OBJS_GAME)

distclean:
	@-rm -rf $(BUILD_DEBUG_DIR) $(BUILD_RELEASE_DIR)

