# _____     _____ ___  ______     _____  _____  __      __    
#      |   |         ||      \   |     ||     ||  \    /  |          
#  ____|   |    _____||       \  |     ||     ||   \  /   | 
# |        |   |      |        \ |     ||     ||    \/    |
# |     ___|   |____  |_________\|_____||_____||          |                                          
# Copyright 2021 Wolf3s and Wally modder
#
#


EE_LIB = russiandoom.a

SRC_OBJS = src/aes_prng.o src/am_map.o src/d_event.o src/d_items.o\
src/d_iwad.o src/d_loop.o src/d_mode.o src/d_net.o\
src/deh_ammo.o src/deh_bexincl.o src/deh_cheat.o src/deh_doom.o\
src/deh_frame.o src/deh_io.o src/deh_main.o src/deh_mapping.o src/deh_misc.o\
src/deh_ptr.c src/deh_sound.o src/deh_str.o src/deh_text.o src/deh_thing.o src/deh_weapon.o\
doomstat.o f_finale.o f_wipe.o g_game.o gusconf.o hu_lib.o\
src/info.o src/m_argv.o src/m_bbox.o src/m_cheat.o src/m_fixed.o src/i_cdmus.o src/i_controller.o\
src/i_main.o src/i_pcsound.o src/i_sound.o src/i_system.o src/i_video.o src/i_videohr.o src/m_config.o\
src/memio.o src/net_client.o src/net_common.o src/net_dedicated.o src/net_io.o src/net_loop.o src/net_packet.o\
src/net_query.o src/net_server.o src/net_structrw.o src/i_input.o src/p_bexptr.o src/p_ceilng.o src/p_doors.o\
src/p_enemy.o src/p_fix.o src/p_floor.o src/p_inter.o src/p_lights.o src/p_map.o src/p_maputl.o src/p_mobj.o\
src/p_plats.o src/p_pspr.o src/p_saveg.o src/p_sight.o src/p_spec.o src/p_switch.o src/p_telept.o\
src/p_tick.o src/p_user.o src/r_bmaps.o src/r_bsp.o src/r_draw.o src/r_main.o src/r_plane.o src/r_segs.o\
src/rd_keybinds.o src/rd_lang.o src/rd_menu.o src/s_sound.o src/sound.o src/tables.o src/v_diskicon.o\
src/v_trans.o src/w_checksum.o src/w_file_posix.o src/w_file_stdc.o src/w_file_win32.o src/w_file.o\
src/w_main.o src/w_merge.o src/z_native.o src/z_zone.o

EE_LDFLAGS += -L. -L$(PS2SDK)/ports/lib
EE_INCS += -I./include -I./vu1 -I$(PS2SDK)/ports/include

# Disabling warnings
WARNING_FLAGS = -Wno-strict-aliasing -Wno-conversion-null 
# VU0 code is broken so disable for now
EE_CFLAGS   += $(WARNING_FLAGS) 

all: $(EE_LIB) $(SRC_OBJS)
	cp -rf RUSSIAN/ $(PS2SDK)/ee/include/
	cp -f  $(EE_LIB) $(SRC_DIR) $(PS2SDK)/ee/lib

clean: 
	@rm -f -r $(EE_LIB) $(SRC_OBJS) $(PCSOUND_DIR) 

install: all clean

include $(PS2SDK)/Defs.make
include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal


