# _____     _____ ___  ______     _____  _____  __      __  _____ ______   
#      |   |         ||      \   |     ||     ||  \    /  ||     |      \         
#  ____|   |    _____||       \  |     ||     ||   \  /   ||____ |       \  
# |        |   |      |        \ |     ||     ||    \/    ||     |        \ 
# |     ___|   |____  |_________\|_____||_____||          ||_____|_________\                                               
# Copyright 2021 Wolf3s and Wally modder
#
#

EE_LIB = russiandoom.a

EE_OBJS_DIR = obj



EE_LDFLAGS  += -L. -L$(PS2SDK)/ports/lib
EE_INCS     += -I./include -I./vu1 -I$(PS2SDK)/ports/include

ifeq ($(DEBUG), 1)
    EE_CFLAGS   += -D_DEBUG
    EE_CXXFLAGS += -D_DEBUG
endif

# Disabling warnings
WARNING_FLAGS = -Wno-strict-aliasing -Wno-conversion-null 

# VU0 code is broken so disable for now
EE_CFLAGS   += $(WARNING_FLAGS) -DNO_VU0_VECTORS -DNO_ASM
EE_CXXFLAGS += $(WARNING_FLAGS) -DNO_VU0_VECTORS -DNO_ASM

all: $(EE_LIB)

install: all
	mkdir -p $(PS2SDK)/ports/include
	mkdir -p $(PS2SDK)/ports/lib
	cp -rf include/    $(PS2SDK)/ports/include
	cp -f  $(EE_LIB) $(PS2SDK)/ports/lib

clean:
	rm -f $(EE_OBJS_LIB) $(EE_OBJS) $(EE_BIN) $(EE_LIB)


include $(PS2SDK)/Defs.make
include $(PS2SDK)/samples/Makefile.eeglobal

#%.vsm: %.pp.vcl
#	openvcl -o $@ $<

#%.pp.vcl: %.vcl
#	vclpp $< $@ -j

%.vo: %_vcl.vsm
	dvp-as -o $@ $<
