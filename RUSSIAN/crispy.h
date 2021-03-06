//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2014 Fabian Greffrath
// Copyright(C) 2016-2021 Julian Nechaevsky
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// DESCRIPTION:
//	Crispy Doom specific variables.
//



#ifndef __CRISPY_H__
#define __CRISPY_H__

#include "doomtype.h"

#ifndef MIN
#define MIN(a,b) (((a)<(b))?(a):(b))
#endif
#ifndef MAX
#define MAX(a,b) (((a)>(b))?(a):(b))
#endif
#ifndef BETWEEN
#define BETWEEN(l,u,x) (((l)>(x))?(l):((x)>(u))?(u):(x))
#endif

#define CRISPY_HUD 12
#define CRISPY_CROSSHAIR "STCFN043"
#define CRISPY_SLOPE(a) ((((a)->lookdir / MLOOKUNIT) << FRACBITS) / 173)

#define singleplayer (!demorecording && !demoplayback && !netgame)

extern boolean crispy_automapoverlay;
extern boolean crispy_cleanscreenshot;
extern boolean crispy_flashinghom;
extern boolean crispy_fliplevels;
extern boolean crispy_havee1m10;
extern boolean crispy_havemap33;
extern boolean crispy_havessg;
extern boolean crispy_nwtmerge;
extern boolean crispy_showfps;

#define crispy_stretchsky (crispy_freelook || crispy_mouselook || crispy_pitch)

extern int crispy_automapstats;
extern int crispy_centerweapon;
extern int crispy_coloredblood;
extern int crispy_coloredhud;
extern int crispy_crosshair;
extern int crispy_extsaveg;
extern int crispy_flipcorpses;
extern int crispy_freeaim;
extern int crispy_freelook;
extern int crispy_fullsounds;
extern int crispy_jump;
extern int crispy_mouselook;
extern int crispy_overunder;
extern int crispy_pitch;
extern int crispy_recoil;
extern int crispy_secretmessage;
extern int crispy_translucency;
extern int crispy_uncapped;

extern int crispy_demowarp;

enum
{
	REINIT_FRAMEBUFFERS = 1,
	REINIT_RENDERER = 2,
	REINIT_TEXTURES = 4,
	REINIT_ASPECTRATIO = 8,
};

enum
{
    COLOREDBLOOD_OFF,
    COLOREDBLOOD_BLOOD,
    COLOREDBLOOD_CORPSE,
    COLOREDBLOOD_BOTH,
    NUM_COLOREDBLOOD,
    COLOREDBLOOD_FIX = 0x10
};

enum
{
    COLOREDHUD_OFF,
    COLOREDHUD_BAR,
    COLOREDHUD_TEXT,
    COLOREDHUD_BOTH,
    NUM_COLOREDHUD
};

enum
{
    CROSSHAIR_OFF,
    CROSSHAIR_STATIC,
    CROSSHAIR_PROJECTED,
    NUM_CROSSHAIRS,
    CROSSHAIR_INTERCEPT = 0x10
};

enum
{
    FREEAIM_AUTO,
    FREEAIM_DIRECT,
    FREEAIM_BOTH,
    NUM_FREEAIMS
};

enum
{
    FREELOOK_OFF,
    FREELOOK_SPRING,
    FREELOOK_LOCK,
    NUM_FREELOOKS
};

enum
{
    JUMP_OFF,
    JUMP_LOW,
    JUMP_HIGH,
    NUM_JUMPS
};

enum
{
    TRANSLUCENCY_OFF,
    TRANSLUCENCY_MISSILE,
    TRANSLUCENCY_ITEM,
    TRANSLUCENCY_BOTH,
    NUM_TRANSLUCENCY
};

enum
{
    UNCAPPED_OFF,
    UNCAPPED_ON,
    UNCAPPED_60FPS,
    UNCAPPED_70FPS,
    NUM_UNCAPPED
};

void *crispy_realloc (void *ptr, size_t size);

#endif
