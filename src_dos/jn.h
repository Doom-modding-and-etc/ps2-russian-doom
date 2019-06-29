//
// Copyright(C) 2018-2019 Julian Nechaevsky
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
//	Russian Doom specific variables.
//


#ifndef __JN_H__
#define __JN_H__


// -----------------------------------------------------------------------------
// Global variables
// -----------------------------------------------------------------------------

extern boolean vanilla;

// -----------------------------------------------------------------------------
// Controls
// -----------------------------------------------------------------------------

extern int mlook;


// -----------------------------------------------------------------------------
// Optional gameplay features
// -----------------------------------------------------------------------------

// Gameplay: Graphical
extern int brightmaps;
extern int fake_contrast;
extern int colored_hud;
extern int colored_blood;
extern int swirling_liquids;
extern int invul_sky;
extern int draw_shadowed_text;

// Gameplay: Audible
extern int play_exit_sfx;
extern int crushed_corpses_sfx;

// Gameplay: Tactical
extern int automap_stats;

// Gameplay: Crosshair
extern int crosshair_draw;


#endif