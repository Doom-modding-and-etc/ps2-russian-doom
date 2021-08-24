# _____     ___ ____  _______    _____  _____  ___     __   
#  ____|   |    ____||       \  |     ||     ||   \  /   | 
# |     ___|   |____ |________\ |_____||_____||    \/    | 
# Copyright 2021 Wolf3s and Wally modder                                          
# 
#
#
VERSION = V0.ACHO QUE DESMAEI 
NAME = DOOM

EE_DIR = obj 

EE_BIN = $(NAME) RUSSO.ELF

EE_OBJS = aes_prng.o am_map.o 

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