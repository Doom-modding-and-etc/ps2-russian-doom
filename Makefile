# _____     _____ ___  ______     _____  _____  __      __    
#      |   |         ||      \   |     ||     ||  \    /  |          
#  ____|   |    _____||       \  |     ||     ||   \  /   | 
# |        |   |      |        \ |     ||     ||    \/    |
# |     ___|   |____  |_________\|_____||_____||          |                                          
# Copyright 2021 Wolf3s and Wally modder
#
#

SRC_DIR = src/

PCSOUND_DIR = pcsound/

EE_LIB = russiandoom.a

EE_OBJS_DIR = obj

EE_LDFLAGS += -L. -L$(PS2SDK)/ports/lib
EE_INCS += -I./include -I./vu1 -I$(PS2SDK)/ports/include

# Disabling warnings
WARNING_FLAGS = -Wno-strict-aliasing -Wno-conversion-null 
# VU0 code is broken so disable for now
EE_CFLAGS   += $(WARNING_FLAGS) 

all: $(EE_LIB) $(SRC_DIR) $(PCSOUND_DIR)
	cp -rf RUSSIAN/ $(PS2SDK)/ee/include/
	cp -f  $(EE_LIB) $(PS2SDK)/ee/lib

clean: 
	@rm -f -r $(EE_LIB) $(SRC_OBJS) $(PCSOUND_DIR) 

install: all clean

include $(PS2SDK)/Defs.make
include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal


