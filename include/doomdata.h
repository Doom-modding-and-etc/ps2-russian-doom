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
//  all external data is defined here
//  most of the data is loaded into different structures at run time
//  some internal structures shared by many modules are here
//


#ifndef __DOOMDATA__
#define __DOOMDATA__


#include "doomtype.h"
#include "doomdef.h"


//
// Map level types.
// The following data structures define the persistent format
// used in the lumps of the WAD files.
//

// Lump order in a map WAD: each map needs a couple of lumps
// to provide a complete scene geometry description.
enum
{
    ML_LABEL,       // A separator, name, ExMx or MAPxx
    ML_THINGS,      // Monsters, items..
    ML_LINEDEFS,    // LineDefs, from editing
    ML_SIDEDEFS,    // SideDefs, from editing
    ML_VERTEXES,    // Vertices, edited and BSP splits generated
    ML_SEGS,        // LineSegs, from LineDefs split by BSP
    ML_SSECTORS,    // SubSectors, list of LineSegs
    ML_NODES,       // BSP nodes
    ML_SECTORS,     // Sectors, from editing
    ML_REJECT,      // LUT, sector-sector visibility	
    ML_BLOCKMAP     // LUT, motion clipping, walls/grid element
};


// A single Vertex.
typedef struct
{
    short x;
    short y;
} PACKEDATTR mapvertex_t;


// A SideDef, defining the visual appearance of a wall,
// by setting textures and offsets.
typedef struct
{
    short textureoffset;
    short rowoffset;
    char  toptexture[8];
    char  bottomtexture[8];
    char  midtexture[8];
    // Front sector, towards viewer.
    short sector;
} PACKEDATTR mapsidedef_t;


// A LineDef, as used for editing, and as input
// to the BSP builder.
typedef struct
{
    unsigned short v1;
    unsigned short v2;
    unsigned short flags;
    short special;
    short tag;
    // sidenum[1] will be -1 if one sided
    unsigned short sidenum[2];
} PACKEDATTR maplinedef_t;

// [crispy] allow loading of Hexen-format maps
// taken from chocolate-doom/src/hexen/xddefs.h:63-75
typedef struct
{
    short v1;
    short v2;
    short flags;
    byte special;
    byte arg1;
    byte arg2;
    byte arg3;
    byte arg4;
    byte arg5;
    short sidenum[2];
} PACKEDATTR maplinedef_hexen_t;

//
// LineDef attributes.
//

// Solid, is an obstacle.
#define ML_BLOCKING 1

// Blocks monsters only.
#define ML_BLOCKMONSTERS 2

// Backside will not be present at all if not two sided.
#define ML_TWOSIDED 4

// If a texture is pegged, the texture will have
// the end exposed to air held constant at the
// top or bottom of the texture (stairs or pulled
// down things) and will move with a height change
// of one of the neighbor sectors.
// Unpegged textures allways have the first row of
// the texture at the top pixel of the line for both
// top and bottom textures (use next to windows).

// upper texture unpegged
#define ML_DONTPEGTOP 8

// lower texture unpegged
#define ML_DONTPEGBOTTOM 16

// In AutoMap: don't map as two sided: IT'S A SECRET!
#define ML_SECRET 32

// Sound rendering: don't let sound cross two of these.
#define ML_SOUNDBLOCK 64

// Don't draw on the automap at all.
#define ML_DONTDRAW 128

// Set if already seen, thus drawn in automap.
#define ML_MAPPED 256


// Sector definition, from editing.
typedef	struct
{
    short floorheight;
    short ceilingheight;
    char  floorpic[8];
    char  ceilingpic[8];
    short lightlevel;
    short special;
    short tag;
} PACKEDATTR mapsector_t;


// SubSector, as generated by BSP.
typedef struct
{
    unsigned short numsegs;
    // Index of first one, segs are stored sequentially.
    unsigned short firstseg;
} PACKEDATTR mapsubsector_t;

// [crispy] allow loading of maps with DeePBSP nodes
// taken from prboom-plus/src/doomdata.h:163-166
typedef struct
{
    unsigned short numsegs;
    int firstseg;
} PACKEDATTR mapsubsector_deepbsp_t;

// [crispy] allow loading of maps with ZDBSP nodes
// taken from prboom-plus/src/doomdata.h:168-170
typedef struct {
    unsigned int numsegs;
} PACKEDATTR mapsubsector_zdbsp_t;


// LineSeg, generated by splitting LineDefs
// using partition lines selected by BSP builder.
typedef struct
{
    unsigned short v1;
    unsigned short v2;
    short angle;		
    unsigned short linedef;
    short side;
    short offset;
} PACKEDATTR mapseg_t;

// [crispy] allow loading of maps with DeePBSP nodes
// taken from prboom-plus/src/doomdata.h:183-190
typedef struct
{
    int v1;
    int v2;
    unsigned short angle;
    unsigned short linedef;
    short side;
    unsigned short offset;
} PACKEDATTR mapseg_deepbsp_t;

// [crispy] allow loading of maps with ZDBSP nodes
// taken from prboom-plus/src/doomdata.h:192-196
typedef struct {
    unsigned int v1, v2;
    unsigned short linedef;
    unsigned char side;
} PACKEDATTR mapseg_zdbsp_t;

// BSP node structure.

// Indicate a leaf.
#define NF_SUBSECTOR    0x80000000
#define NO_INDEX        ((unsigned short)-1)

typedef struct
{
    // Partition line from (x,y) to x+dx,y+dy)
    short x;
    short y;
    short dx;
    short dy;

    // Bounding box for each child,
    // clip against view frustum.
    short bbox[2][4];

    // If NF_SUBSECTOR its a subsector,
    // else it's a node of another subtree.
    unsigned short children[2];
} PACKEDATTR mapnode_t;

// [crispy] allow loading of maps with DeePBSP nodes
// taken from prboom-plus/src/doomdata.h:216-225
typedef struct
{
    short x;
    short y;
    short dx;
    short dy;
    short bbox[2][4];
    int children[2];
} PACKEDATTR mapnode_deepbsp_t;

// [crispy] allow loading of maps with ZDBSP nodes
// taken from prboom-plus/src/doomdata.h:227-136
typedef struct {
    short x;
    short y;
    short dx;
    short dy;
    short bbox[2][4];
    int children[2];
} PACKEDATTR mapnode_zdbsp_t;

// Thing definition, position, orientation and type,
// plus skill/visibility flags and attributes.
typedef struct
{
    short x;
    short y;
    short angle;
    short type;
    short options;
} PACKEDATTR mapthing_t;

// [crispy] allow loading of Hexen-format maps
// taken from chocolate-doom/src/hexen/xddefs.h:134-149
typedef struct
{
    short tid;
    short x;
    short y;
    short height;
    short angle;
    short type;
    short options;
    byte special;
    byte arg1;
    byte arg2;
    byte arg3;
    byte arg4;
    byte arg5;
} PACKEDATTR mapthing_hexen_t;

#endif			// __DOOMDATA__
