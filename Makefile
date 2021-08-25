# _____     _____ ___  ______     _____  _____  __      __  _____ ______     _____           ____  ____       _      ___
#      |   |         ||      \   |     ||     ||  \    /  ||     |      \   |     | |     | |     |     |    / \    |   \    |      
#  ____|   |    _____||       \  |     ||     ||   \  /   ||____ |       \  |   __| |     | |____ |____ |   /___\   |    \   |  
# |        |   |      |        \ |     ||     ||    \/    ||     |        \ |   \   |     |     /     / |  /     \  |     \  | 
# |     ___|   |____  |_________\|_____||_____||          ||_____|_________\|    \   \___/  ___/  ___/  | /       \ |      \_|
# Copyright 2021 Wolf3s and Wally modder
#
#
VERSION = 0
SUBVERSION = 0
PATCHLEVEL = 1
EXTRAVERSION = LENO BREGA

EE_BIN = $(NAME) RUSSO.ELF

EE_OBJS_DIR = aes_prng.o am_map.o d_event.o d_items.o\
d_iwad.o d_loop.o d_mode.o d_net.o\
deh_ammo.o deh_bexincl.o deh_cheat.o deh_doom.o\
deh_frame.o deh_io.o deh_main.o deh_mapping.o deh_misc.o\
deh_ptr.c deh_sound.o deh_str.o deh_text.o deh_thing.o deh_weapon.o\
doomstat.o f_finale.o f_wipe.o\


EE_LIBS = -lkernel 
all: $(EE_BIN)

clean:
	rm -f $(EE_OBJS) $(EE_BIN)

run:
	ps2client execee host:$(EE_BIN)

reset:
	ps2client reset

include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal