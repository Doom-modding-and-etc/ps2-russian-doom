//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
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
//	Here is a core component: drawing the floors and ceilings,
//	 while maintaining a per column clipping list only.
//	Moreover, the sky areas have to be determined.
//


#include <stdio.h>
#include <stdlib.h>
#include "include/i_system.h"
#include "include/z_zone.h"
#include "include/w_wad.h"
#include "include/doomdef.h"
#include "include/doomstat.h"
#include "include/r_data.h"
#include "include/r_local.h"
#include "include/r_swirl.h"
#include "include/r_bmaps.h"
#include "include/jn.h"


// -----------------------------------------------------------------------------
// MAXVISPLANES is no longer a limit on the number of visplanes,
// but a limit on the number of hash slots; larger numbers mean
// better performance usually but after a point they are wasted,
// and memory and time overheads creep in.
//
// Lee Killough
// -----------------------------------------------------------------------------

#define MAXVISPLANES	128                  // must be a power of 2

static visplane_t *visplanes[MAXVISPLANES];  // [JN] killough
static visplane_t *freetail;                 // [JN] killough
static visplane_t **freehead = &freetail;    // [JN] killough
visplane_t *floorplane, *ceilingplane;

// [JN] killough -- hash function for visplanes
// Empirically verified to be fairly uniform:

#define visplane_hash(picnum, lightlevel, height) \
    ((unsigned int)((picnum) * 3 + (lightlevel) + (height) * 7) & (MAXVISPLANES - 1))

// [JN] killough 8/1/98: set static number of openings to be large enough
// (a static limit is okay in this case and avoids difficulties in r_segs.c)

#define MAXOPENINGS (WIDESCREENWIDTH * SCREENHEIGHT)
int openings[MAXOPENINGS], *lastopening; // [crispy] 32-bit integer math

//
// Clip values are the solid pixel bounding the range.
//  floorclip starts out SCREENHEIGHT
//  ceilingclip starts out -1
//

int floorclip[WIDESCREENWIDTH];   // [crispy] 32-bit integer math
int ceilingclip[WIDESCREENWIDTH]; // [crispy] 32-bit integer math

//
// spanstart holds the start of a plane span
// initialized to 0 at start
//

static int spanstart[SCREENHEIGHT];

//
// texture mapping
//

static lighttable_t **planezlight;
static fixed_t planeheight;
static fixed_t cachedheight[SCREENHEIGHT];
static fixed_t cacheddistance[SCREENHEIGHT];
static fixed_t cachedxstep[SCREENHEIGHT];
static fixed_t cachedystep[SCREENHEIGHT];

fixed_t *yslope;
fixed_t  yslopes[LOOKDIRS][SCREENHEIGHT];
fixed_t  distscale[WIDESCREENWIDTH];


// -----------------------------------------------------------------------------
// R_MapPlane
//
// Uses global vars:
//  - planeheight
//  - ds_source
//  - viewx
//  - viewy
//
// BASIC PRIMITIVE
// -----------------------------------------------------------------------------

static void R_MapPlane (int y, int x1, int x2)
{
    unsigned index;
    int      dx, dy;
    fixed_t	 distance;

#ifdef RANGECHECK
    if (x2 < x1 || x1 < 0 || x2 >= viewwidth || y > viewheight)
    {
        I_Error (english_language ? 
                 "R_MapPlane: %i, %i at %i" :
                 "R_MapPlane: %i, %i у %i", x1, x2, y);
    }
#endif

    // [crispy] visplanes with the same flats now match up far better than before
    // adapted from prboom-plus/src/r_plane.c:191-239, translated to fixed-point math
    if (!(dy = abs(centery - y)))
    {
        return;
    }

    if (planeheight != cachedheight[y])
    {
        cachedheight[y] = planeheight;
        distance = cacheddistance[y] = FixedMul (planeheight, yslope[y]);
        ds_xstep = cachedxstep[y] = FixedMul (viewsin, planeheight) / dy;
        ds_ystep = cachedystep[y] = FixedMul (viewcos, planeheight) / dy;
    }
    else
    {
        distance = cacheddistance[y];
        ds_xstep = cachedxstep[y];
        ds_ystep = cachedystep[y];
    }

    dx = x1 - centerx;

    ds_xfrac = viewx + FixedMul(viewcos, distance) + dx * ds_xstep;
    ds_yfrac = -viewy - FixedMul(viewsin, distance) + dx * ds_ystep;

    if (fixedcolormap)
    {
        ds_colormap = fixedcolormap;
    }
    else
    {
        // [JN] Note: no smoother diminished lighting in -vanilla mode
        index = distance >> lightzshift;

        if (index >= maxlightz)
            index = maxlightz-1;

        ds_colormap = planezlight[index];
    }

    ds_y = y;
    ds_x1 = x1;
    ds_x2 = x2;

    // high or low detail
    spanfunc ();	
}

// -----------------------------------------------------------------------------
// R_ClearPlanes
// At begining of frame.
// -----------------------------------------------------------------------------

void R_ClearPlanes (void)
{
    int i;

    // opening / clipping determination
    for (i = 0 ; i < viewwidth ; i++)
    {
        floorclip[i] = viewheight;
        ceilingclip[i] = -1;
    }

    for (i = 0; i < MAXVISPLANES; i++)  // [JN] new code -- killough
        for (*freehead = visplanes[i], visplanes[i] = NULL ; *freehead ; )
            freehead = &(*freehead)->next;

    lastopening = openings;

    // texture calculation
    memset (cachedheight, 0, sizeof(cachedheight));
}

// -----------------------------------------------------------------------------
// [crispy] remove MAXVISPLANES Vanilla limit
// New function, by Lee Killough
// -----------------------------------------------------------------------------

static visplane_t *new_visplane (unsigned int hash)
{
    visplane_t *check = freetail;

    if (!check)
    {
        check = calloc(1, sizeof(*check));
    }
    else if (!(freetail = freetail->next))
    {
        freehead = &freetail;
    }

    check->next = visplanes[hash];
    visplanes[hash] = check;

    return check;
}

// -----------------------------------------------------------------------------
// R_FindPlane
// -----------------------------------------------------------------------------

visplane_t *R_FindPlane (fixed_t height, int picnum, int lightlevel)
{
    unsigned int  hash;
    visplane_t   *check;

    if (picnum == skyflatnum)
    {
        height = 0; // all skys map together
        lightlevel = 0;
    }

    // New visplane algorithm uses hash table -- killough
    hash = visplane_hash(picnum, lightlevel, height);
    
    for (check = visplanes[hash]; check; check = check->next)
        if (height == check->height && picnum == check->picnum && lightlevel == check->lightlevel)
            return check;

    check = new_visplane(hash);

    check->height = height;
    check->picnum = picnum;
    check->lightlevel = lightlevel;
    check->minx = WIDESCREENWIDTH;
    check->maxx = -1;

    memset(check->top, UINT_MAX, sizeof(check->top));

    return check;
}

// -----------------------------------------------------------------------------
// R_DupPlane
// -----------------------------------------------------------------------------

visplane_t *R_DupPlane(const visplane_t *pl, int start, int stop)
{
    visplane_t  *new_pl = new_visplane(visplane_hash(pl->picnum, pl->lightlevel, pl->height));

    new_pl->height = pl->height;
    new_pl->picnum = pl->picnum;
    new_pl->lightlevel = pl->lightlevel;
    new_pl->minx = start;
    new_pl->maxx = stop;

    memset(new_pl->top, UINT_MAX, sizeof(new_pl->top));

    return new_pl;
}

// -----------------------------------------------------------------------------
// R_CheckPlane
// -----------------------------------------------------------------------------

visplane_t *R_CheckPlane (visplane_t *pl, int start, int stop)
{
    int intrl, intrh, unionl, unionh, x;

    if (start < pl->minx)
    {
        intrl = pl->minx, unionl = start;
    }
    else
    {
        unionl = pl->minx, intrl = start;
    }

    if (stop  > pl->maxx)
    {
        intrh = pl->maxx, unionh = stop;
    }
    else
    {
        unionh = pl->maxx, intrh  = stop;
    }

    for (x=intrl ; x <= intrh && pl->top[x] == UINT_MAX; x++); // [crispy] hires / 32-bit integer math
    // [crispy] fix HOM if ceilingplane and floorplane are the same visplane (e.g. both are skies)
    if (!(pl == floorplane && markceiling && floorplane == ceilingplane) && x > intrh)
    {
        // Can use existing plane; extend range
        pl->minx = unionl, pl->maxx = unionh;
        return pl;
    }
    else
    {
        // Cannot use existing plane; create a new one
        return R_DupPlane(pl, start, stop);
    }
}

// -----------------------------------------------------------------------------
// R_MakeSpans
// -----------------------------------------------------------------------------

static void
R_MakeSpans (int x, unsigned int t1, unsigned int b1, // [crispy] 32-bit integer math
                    unsigned int t2, unsigned int b2) // [crispy] 32-bit integer math
{
    for ( ; t1 < t2 && t1 <= b1 ; t1++)
    {
        R_MapPlane(t1, spanstart[t1], x-1);
    }
    for ( ; b1 > b2 && b1 >= t1 ; b1--)
    {
        R_MapPlane(b1, spanstart[b1], x-1);
    }
    while (t2 < t1 && t2 <= b2)
    {
        spanstart[t2++] = x;
    }
    while (b2 > b1 && b2 >= t2)
    {
        spanstart[b2--] = x;
    }
}

// -----------------------------------------------------------------------------
// R_DrawPlanes
// At the end of each frame.
// -----------------------------------------------------------------------------

void R_DrawPlanes (void) 
{
    int i, x;
    visplane_t *pl;

    for (i = 0 ; i < MAXVISPLANES ; i++)
    for (pl = visplanes[i] ; pl ; pl = pl->next)
    if (pl->minx <= pl->maxx)
    {
        // sky flat
        if (pl->picnum == skyflatnum)
        {
            dc_iscale = pspriteiscale>>(detailshift && !hires);
            
            // [JN] Scale sky texture if appropriate.
            if (mlook && scaled_sky)
            {
                dc_iscale = dc_iscale / 2;
            }

            // Sky is allways drawn full bright, 
            //  i.e. colormaps[0] is used.
            // Because of this hack, sky is not affected
            //  by INVUL inverse mapping.
            //
            // [JN] Make optional, "Invulnerability affects sky" feature.
            
            if (invul_sky && !vanillaparm)
            {
                dc_colormap = (fixedcolormap ? fixedcolormap : colormaps);
            }
            else
            {
                dc_colormap = colormaps;
            }

            dc_texturemid = skytexturemid;
            dc_texheight = textureheight[skytexture]>>FRACBITS;

            for (x=pl->minx ; x <= pl->maxx ; x++)
            {
                if ((dc_yl = pl->top[x]) != UINT_MAX && dc_yl <= (dc_yh = pl->bottom[x])) // [crispy] 32-bit integer math
                {
                    // [crispy] Optionally draw skies horizontally linear.
                    int angle = ((viewangle + (linear_sky && !vanillaparm ? linearskyangle[x] : 
                                               xtoviewangle[x]))^flip_levels)>>ANGLETOSKYSHIFT;
                    dc_x = x;
                    dc_source = R_GetColumn(skytexture, angle);
                    colfunc ();
                }
            }
        }
        else  // regular flat
        {
            int stop, light;
            const int lumpnum = firstflat + flattranslation[pl->picnum];

            // [crispy] add support for SMMU swirling flats
            ds_source = (flattranslation[pl->picnum] == -1) ?
                        R_DistortedFlat(pl->picnum) :
                        W_CacheLumpNum(lumpnum, PU_STATIC);

            planeheight = abs(pl->height-viewz);
            light = ((pl->lightlevel + level_brightness) >> LIGHTSEGSHIFT) + extralight;

            if (light >= LIGHTLEVELS)
            {
                light = LIGHTLEVELS-1;
            }
            if (light < 0)
            {
                light = 0;
            }

            stop = pl->maxx + 1;
            planezlight = zlight[light];
            pl->top[pl->minx-1] = pl->top[stop] = UINT_MAX; // [crispy] 32-bit integer math

            // [JN] Apply brightmaps to floor/ceiling...
            if (brightmaps && brightmaps_allowed)
            {
                if (pl->picnum == bmapflatnum1  // CONS1_1
                ||  pl->picnum == bmapflatnum2  // CONS1_5
                ||  pl->picnum == bmapflatnum3) // CONS1_7
                {
                    planezlight = fullbright_notgrayorbrown_floor[light];
                }
                if (pl->picnum == bmapflatnum4) // GATE6
                {
                    planezlight = fullbright_orangeyellow_floor[light];
                }
            }

            for (x = pl->minx ; x <= stop ; x++)
            {
                R_MakeSpans(x,pl->top[x-1], pl->bottom[x-1], pl->top[x], pl->bottom[x]);
            }

            // [crispy] add support for SMMU swirling flats
            if (flattranslation[pl->picnum] != -1)
            {
                W_ReleaseLumpNum(lumpnum);
            }
        }
    }
}
