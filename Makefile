# _____     _____ ___  ______     _____  _____  __      __  _____ ______   
#      |   |         ||      \   |     ||     ||  \    /  ||     |      \         
#  ____|   |    _____||       \  |     ||     ||   \  /   ||____ |       \  
# |        |   |      |        \ |     ||     ||    \/    ||     |        \ 
# |     ___|   |____  |_________\|_____||_____||          ||_____|_________\                                               
# Copyright 2021 Wolf3s and Wally modder
#
#

EE_LIB = librussiandoom.a

EE_OBJS_DIR = aes_prng.o am_map.o d_event.o d_items.o\
d_iwad.o d_loop.o d_mode.o d_net.o\
deh_ammo.o deh_bexincl.o deh_cheat.o deh_doom.o\
deh_frame.o deh_io.o deh_main.o deh_mapping.o deh_misc.o\
deh_ptr.c deh_sound.o deh_str.o deh_text.o deh_thing.o deh_weapon.o\
doomstat.o f_finale.o f_wipe.o g_game.o gusconf.o hu_lib.o\
info.o m_argv.o m_bbox.o m_cheat.o m_fixed.o i_cdmus.o i_controller.o\
i_main.o i_pcsound.o i_sound.o i_system.o i_video.o i_videohr.o m_config.o\
memio.o net_client.o net_common.o net_dedicated.o net_io.o net_loop.o net_packet.o\
net_query.o net_server.o net_structrw.o i_input.o p_bexptr.o p_ceilng.o p_doors.o\
p_enemy.o p_fix.o p_floor.o p_inter.o p_lights.o p_map.o p_maputl.o p_mobj.o\
p_plats.o p_pspr.o p_saveg.o p_sight.o p_spec.o p_switch.o p_telept.o\
p_tick.o p_user.o r_bmaps.o r_bsp.o r_draw.o r_main.o r_plane.o r_segs.o\
rd_keybinds.o rd_lang.o rd_menu.o s_sound.o sound.o tables.o v_diskicon.o\
v_trans.o w_checksum.o w_file_posix.o w_file_stdc.o w_file_win32.o w_file.o\
w_main.o w_merge.o z_native.o z_zone.o



WARNING_FLAGS = -Wno-strict-aliasing -Wno-conversion-null 

EE_LDFLAGS  += -L. -L$(PS2SDK)/ports/lib
EE_INCS     += -I./include -I$(PS2SDK)/ports/include

ifeq ($(DEBUG), 1)
    EE_CFLAGS   += -D_DEBUG
    EE_CXXFLAGS += -D_DEBUG
endif

all: $(EE_LIB)

install: all
	mkdir -p $(PS2SDK)/ports/include
	mkdir -p $(PS2SDK)/ports/lib
	cp -rf include/ $(PS2SDK)/ports/include	
	cp -f  $(EE_LIB) $(PS2SDK)/ports/lib

clean:
	rm -f $(EE_OBJS_LIB) $(EE_OBJS) $(EE_BIN) $(EE_LIB)

include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal
