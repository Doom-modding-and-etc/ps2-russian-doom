//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
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
//	Status bar code.
//	Does the face/direction indicator animatin.
//	Does palette indicators as well (red pain/berserk, bright pickup)
//

// Russian Doom (C) 2016-2018 Julian Nechaevsky


#include <stdio.h>

#include "i_swap.h" // [crispy] SHORT()
#include "i_system.h"
#include "i_video.h"
#include "z_zone.h"
#include "m_misc.h"
#include "m_random.h"
#include "w_wad.h"

#include "deh_main.h"
#include "deh_misc.h"
#include "doomdef.h"
#include "doomkeys.h"

#include "g_game.h"

#include "st_stuff.h"
#include "st_lib.h"
#include "r_local.h"

#include "p_local.h"
#include "p_inter.h"

#include "am_map.h"
#include "m_cheat.h"
#include "m_menu.h"

#include "s_sound.h"

// Needs access to LFB.
#include "v_video.h"

// State.
#include "doomstat.h"

// Data.
#include "dstrings.h"
#include "sounds.h"

#include "jn.h"


//
// STATUS BAR DATA
//


// Palette indices.
// For damage/bonus red-/gold-shifts
#define STARTREDPALS        1
#define STARTBONUSPALS      9
#define NUMREDPALS          8
#define NUMBONUSPALS        4
// Radiation suit, green shift.
#define RADIATIONPAL        13
// [JN] Atari Jaguar: cyan invulnerability palette
#define INVULNERABILITYPAL  14

// N/256*100% probability
//  that the normal face state will change
#define ST_FACEPROBABILITY  96

// For Responder
#define ST_TOGGLECHAT       KEY_ENTER

// Location of status bar
#define ST_X                0
#define ST_X2               104

#define ST_FX               143
#define ST_FY               169

// Should be set to patch width
//  for tall numbers later on
#define ST_TALLNUMWIDTH     (tallnum[0]->width)

// Number of status faces.
#define ST_NUMPAINFACES     5
#define ST_NUMSTRAIGHTFACES 3
#define ST_NUMTURNFACES     2
#define ST_NUMSPECIALFACES  3

#define ST_FACESTRIDE \
       (ST_NUMSTRAIGHTFACES+ST_NUMTURNFACES+ST_NUMSPECIALFACES)

#define ST_NUMEXTRAFACES    2

// [JN] Дополнительные циклы лиц:
// Atari Doom: +6 (разорванное лицо)
// PSX Doom: +1 (раздавленное лицо)
#define ST_NUMFACES \
       (ST_FACESTRIDE*ST_NUMPAINFACES+ST_NUMEXTRAFACES+7)

#define ST_TURNOFFSET       (ST_NUMSTRAIGHTFACES)
#define ST_OUCHOFFSET       (ST_TURNOFFSET + ST_NUMTURNFACES)
#define ST_EVILGRINOFFSET   (ST_OUCHOFFSET + 1)
#define ST_RAMPAGEOFFSET    (ST_EVILGRINOFFSET + 1)
#define ST_GODFACE          (ST_NUMPAINFACES*ST_FACESTRIDE)
#define ST_DEADFACE         (ST_GODFACE+1)
// [JN] Atari Doom - дополнительное лицо "разорванного" игрока
#define ST_EXPLFACE0        (ST_DEADFACE+1)
#define ST_EXPLFACE1        (ST_EXPLFACE0+1)
#define ST_EXPLFACE2        (ST_EXPLFACE1+1)
#define ST_EXPLFACE3        (ST_EXPLFACE2+1)
#define ST_EXPLFACE4        (ST_EXPLFACE3+1)
#define ST_EXPLFACE5        (ST_EXPLFACE4+1)
// [JN] PSX Doom - дополнительное лицо "раздавленного" игрока
#define ST_CRSHFACE0        (ST_EXPLFACE5+1)

#define ST_FACESX           143+ORIGWIDTH_DELTA
#define ST_FACESY           168

#define ST_EVILGRINCOUNT        (2*TICRATE)
#define ST_STRAIGHTFACECOUNT    (TICRATE/2)
#define ST_TURNCOUNT            (1*TICRATE)
#define ST_OUCHCOUNT            (1*TICRATE)
#define ST_RAMPAGEDELAY         (2*TICRATE)

#define ST_MUCHPAIN         20


// Location and size of statistics,
//  justified according to widget type.
// Problem is, within which space? STbar? Screen?
// Note: this could be read in by a lump.
//       Problem is, is the stuff rendered
//       into a buffer,
//       or into the frame buffer?

// AMMO number pos.
#define ST_AMMOWIDTH        3	
#define ST_AMMOX            44+ORIGWIDTH_DELTA
#define ST_AMMOY            171

// HEALTH number pos.
#define ST_HEALTHWIDTH      3	
#define ST_HEALTHX          90+ORIGWIDTH_DELTA
#define ST_HEALTHY          171

// Weapon pos.
#define ST_ARMSX            111+ORIGWIDTH_DELTA
#define ST_ARMSY            172
#define ST_ARMSBGX          104+ORIGWIDTH_DELTA
#define ST_ARMSBGY          168
#define ST_ARMSXSPACE       12
#define ST_ARMSYSPACE       10

// Frags pos.
#define ST_FRAGSX           138+ORIGWIDTH_DELTA
#define ST_FRAGSY           171	
#define ST_FRAGSWIDTH       2

// [JN] Press Beta: player's life pos.
#define ST_LIFESX           177+ORIGWIDTH_DELTA
#define ST_LIFESY           193
#define ST_LIFESWIDTH       1

// ARMOR number pos.
#define ST_ARMORWIDTH       3
#define ST_ARMORX           221+ORIGWIDTH_DELTA
#define ST_ARMORY           171

// Key icon positions.
#define ST_KEY0WIDTH        8
#define ST_KEY0HEIGHT       5
#define ST_KEY0X            239+ORIGWIDTH_DELTA
#define ST_KEY0Y            171
#define ST_KEY1WIDTH        ST_KEY0WIDTH
#define ST_KEY1X            239+ORIGWIDTH_DELTA
#define ST_KEY1Y            181
#define ST_KEY2WIDTH        ST_KEY0WIDTH
#define ST_KEY2X            239+ORIGWIDTH_DELTA
#define ST_KEY2Y            191

// Ammunition counter.
#define ST_AMMO0WIDTH       3
#define ST_AMMO0HEIGHT      6
#define ST_AMMO0X           288+ORIGWIDTH_DELTA
#define ST_AMMO0Y           173
#define ST_AMMO1WIDTH       ST_AMMO0WIDTH
#define ST_AMMO1X           288+ORIGWIDTH_DELTA
#define ST_AMMO1Y           179
#define ST_AMMO2WIDTH       ST_AMMO0WIDTH
#define ST_AMMO2X           288+ORIGWIDTH_DELTA
#define ST_AMMO2Y           191
#define ST_AMMO3WIDTH       ST_AMMO0WIDTH
#define ST_AMMO3X           288+ORIGWIDTH_DELTA
#define ST_AMMO3Y           185

// Indicate maximum ammunition.
// Only needed because backpack exists.
#define ST_MAXAMMO0WIDTH    3
#define ST_MAXAMMO0HEIGHT   5
#define ST_MAXAMMO0X        314+ORIGWIDTH_DELTA
#define ST_MAXAMMO0Y        173
#define ST_MAXAMMO1WIDTH    ST_MAXAMMO0WIDTH
#define ST_MAXAMMO1X        314+ORIGWIDTH_DELTA
#define ST_MAXAMMO1Y        179
#define ST_MAXAMMO2WIDTH    ST_MAXAMMO0WIDTH
#define ST_MAXAMMO2X        314+ORIGWIDTH_DELTA
#define ST_MAXAMMO2Y        191
#define ST_MAXAMMO3WIDTH    ST_MAXAMMO0WIDTH
#define ST_MAXAMMO3X        314+ORIGWIDTH_DELTA
#define ST_MAXAMMO3Y        185

// pistol
#define ST_WEAPON0X         110+ORIGWIDTH_DELTA
#define ST_WEAPON0Y         172

// shotgun
#define ST_WEAPON1X         122+ORIGWIDTH_DELTA
#define ST_WEAPON1Y         172

// chain gun
#define ST_WEAPON2X	        134+ORIGWIDTH_DELTA
#define ST_WEAPON2Y	        172

// missile launcher
#define ST_WEAPON3X         110+ORIGWIDTH_DELTA
#define ST_WEAPON3Y         181

// plasma gun
#define ST_WEAPON4X         122+ORIGWIDTH_DELTA
#define ST_WEAPON4Y         181

// bfg
#define ST_WEAPON5X	        134+ORIGWIDTH_DELTA
#define ST_WEAPON5Y	        181

// WPNS title
#define ST_WPNSX            109+ORIGWIDTH_DELTA
#define ST_WPNSY            191

// DETH title
#define ST_DETHX            109+ORIGWIDTH_DELTA
#define ST_DETHY            191


//Incoming messages window location
//UNUSED
// #define ST_MSGTEXTX	   (viewwindowx)
// #define ST_MSGTEXTY	   (viewwindowy+viewheight-18)
#define ST_MSGTEXTX         0
#define ST_MSGTEXTY         0

// Dimensions given in characters.
#define ST_MSGWIDTH         52
// Or shall I say, in lines?
#define ST_MSGHEIGHT        1

#define ST_OUTTEXTX         0
#define ST_OUTTEXTY         6

// Width, in characters again.
#define ST_OUTWIDTH         52 
 // Height, in lines. 
#define ST_OUTHEIGHT        1

#define ST_MAPTITLEX \
    (ORIGWIDTH - ST_MAPWIDTH * ST_CHATFONTWIDTH)

#define ST_MAPTITLEY        0
#define ST_MAPHEIGHT        1

// graphics are drawn to a backing screen and blitted to the real screen
byte *st_backing_screen;
	    
// main player in game
static player_t* plyr; 

// ST_Start() has just been called
static boolean st_firsttime;

// [JN] lump number for PALFIX (1) and PLAYPAL (2)
static int lu_palette1, lu_palette2;

// used for timing
static unsigned int	st_clock;

// used for making messages go away
static int st_msgcounter=0;

// used when in chat 
static st_chatstateenum_t   st_chatstate;

// whether in automap or first-person
static st_stateenum_t   st_gamestate;

// whether left-side main status bar is active
static boolean st_statusbaron;

// whether status bar chat is active
static boolean st_chat;

// value of st_chat before message popped up
static boolean st_oldchat;

// whether chat window has the cursor on
static boolean st_cursoron;

// !deathmatch
static boolean st_notdeathmatch; 

// !deathmatch && st_statusbaron
static boolean st_armson;

// !deathmatch
static boolean st_fragson; 

// [JN] only in Press Beta
static boolean st_artifactson;

// main bar left
static patch_t* sbar;

// 0-9, tall numbers
static patch_t* tallnum[10];

// tall % sign
static patch_t* tallpercent;

// 0-9, short, yellow (,different!) numbers
static patch_t* shortnum[10];

// 3 key-cards, 3 skulls
// jff 2/24/98 extend number of patches by three skull/card combos
static patch_t* keys[NUMCARDS+3]; 

// face status patches
// [JN] Массив удвоен, необходимо для бессмертия.
// Thanks Brad Harding for help!
static patch_t* faces[ST_NUMFACES * 2];
// [JN] If false, we can use an extra GOD faces.
extern boolean old_godface;

// face background
static patch_t* faceback;

 // main bar right
static patch_t* armsbg;

// weapon ownership patches
static patch_t* arms[6][2]; 

// ready-weapon widget
static st_number_t      w_ready;

 // in deathmatch only, summary of frags stats
static st_number_t      w_frags;

// [JN] Press Beta: artifacts widget
static st_number_t      w_artifacts;

// [JN] Press Beta: widget for player's lifes
static st_number_t      w_lifes;

// health widget
static st_percent_t     w_health;

// arms background
static st_binicon_t     w_armsbg; 

// weapon ownership widgets
static st_multicon_t    w_arms[6];

// face status widget
static st_multicon_t    w_faces; 

// keycard widgets
static st_multicon_t    w_keyboxes[3];

// armor widget
static st_percent_t     w_armor;

// ammo widgets
static st_number_t      w_ammo[4];

// max ammo widgets
static st_number_t      w_maxammo[4]; 



// number of frags so far in deathmatch
static int st_fragscount;

// [JN] Press Beta: number of picked up artifacts
static int st_artifactscount;

// used to use appopriately pained face
static int st_oldhealth = -1;

// used for evil grin
static boolean oldweaponsowned[NUMWEAPONS]; 

// count until face changes
static int st_facecount = 0;

// current face index, used by w_faces
static int st_faceindex = 0;

// holds key-type for each key box on bar
static int keyboxes[3]; 

// a random number per tick
static int st_randomnumber;  

cheatseq_t cheat_mus = CHEAT("idmus", 2);
cheatseq_t cheat_god = CHEAT("iddqd", 0);
cheatseq_t cheat_ammo = CHEAT("idkfa", 0);
cheatseq_t cheat_ammonokey = CHEAT("idfa", 0);
cheatseq_t cheat_keys = CHEAT("idka", 0);
cheatseq_t cheat_noclip = CHEAT("idspispopd", 0);
cheatseq_t cheat_commercial_noclip = CHEAT("idclip", 0);

cheatseq_t cheat_powerup[7] =
{
    CHEAT("idbeholdv", 0),
    CHEAT("idbeholds", 0),
    CHEAT("idbeholdi", 0),
    CHEAT("idbeholdr", 0),
    CHEAT("idbeholda", 0),
    CHEAT("idbeholdl", 0),
    CHEAT("idbehold",  0),
};

cheatseq_t cheat_choppers = CHEAT("idchoppers", 0);
cheatseq_t cheat_clev = CHEAT("idclev", 2);
cheatseq_t cheat_mypos = CHEAT("idmypos", 0);

// [crispy] new cheats
cheatseq_t cheat_massacre = CHEAT("tntem", 0);

// [JN] Press Beta cheat codes
cheatseq_t cheat_god_beta    = CHEAT("tst", 0); // iddqd
cheatseq_t cheat_ammo_beta   = CHEAT("amo", 0); // idkfa
cheatseq_t cheat_noclip_beta = CHEAT("nc", 0);  // idclip

// [JN] Чит-код отображения версии проекта
cheatseq_t cheat_version = CHEAT("version", 0);
static char msg[ST_MSGWIDTH];

//
// STATUS BAR CODE
//
void ST_Stop(void);

void ST_refreshBackground(void)
{
    if (screenblocks >= 11 && !automapactive)
    return;

    if (st_statusbaron)
    {
        V_UseBuffer(st_backing_screen);
        V_DrawPatch(ST_X+ORIGWIDTH_DELTA, 0, sbar);

        // [crispy] back up arms widget background
        if (!deathmatch && gamemode != pressbeta)
        V_DrawPatch(ST_ARMSBGX, 0, armsbg);

        if (netgame)
        V_DrawPatch(ST_FX+ORIGWIDTH_DELTA, 0, faceback);

        V_RestoreBuffer();

        V_CopyRect(ST_X, 0, st_backing_screen, ST_WIDTH, ST_HEIGHT, ST_X, ST_Y);
    }
}


// [crispy] adapted from boom202s/M_CHEAT.C:467-498

static int ST_cheat_massacre()
{
    int killcount = 0;
    thinker_t *th;
    extern int numbraintargets;
    extern void A_PainDie(mobj_t *);

    for (th = thinkercap.next; th != &thinkercap; th = th->next)
    {
        if (th->function.acp1 == (actionf_p1)P_MobjThinker)
        {
            mobj_t *mo = (mobj_t *)th;

            if (mo->flags & MF_COUNTKILL || mo->type == MT_SKULL)
            {
                if (mo->health > 0)
                {
                    P_DamageMobj(mo, NULL, NULL, 10000);
                    killcount++;
                }
                if (mo->type == MT_PAIN)
                {
                    A_PainDie(mo);
                    P_SetMobjState(mo, S_PAIN_DIE6);
                }
            }
        }
    }

    // [crispy] disable brain spitters
    numbraintargets = -1;

    return killcount;
} 


// Respond to keyboard input events,
//  intercept cheats.
boolean ST_Responder (event_t* ev)
{
    int i;

    // Filter automap on/off.
    if (ev->type == ev_keyup && ((ev->data1 & 0xffff0000) == AM_MSGHEADER))
    {
        switch(ev->data1)
        {
            case AM_MSGENTERED:
                st_gamestate = AutomapState;
                st_firsttime = true;
            break;

            case AM_MSGEXITED:
                // fprintf(stderr, "AM exited\n");
                st_gamestate = FirstPersonState;
            break;
        }
    }

    // if a user keypress...
    else if (ev->type == ev_keydown)
    {
        if (!netgame && (gameskill != sk_nightmare /* && gameskill != sk_ultranm */) && gamemode != pressbeta)
        {
            // 'dqd' cheat for toggleable god mode
            if (cht_CheckCheat(&cheat_god, ev->data2))
            {
                // [crispy] dead players are first respawned at the current position
                mapthing_t mt = {0};
                if (plyr->playerstate == PST_DEAD)
                {
                    signed int an;
                    extern void P_SpawnPlayer (mapthing_t* mthing);

                    mt.x = plyr->mo->x >> FRACBITS;
                    mt.y = plyr->mo->y >> FRACBITS;
                    mt.angle = (plyr->mo->angle + ANG45/2)*(uint64_t)45/ANG45;
                    mt.type = consoleplayer + 1;
                    P_SpawnPlayer(&mt);
 
                    // [crispy] spawn a teleport fog
                    an = plyr->mo->angle >> ANGLETOFINESHIFT;
                    P_SpawnMobj(plyr->mo->x+20*finecosine[an], plyr->mo->y+20*finesine[an], plyr->mo->z, MT_TFOG);
                    S_StartSound(plyr->mo, sfx_telept);
                }

                plyr->cheats ^= CF_GODMODE;

                if (plyr->cheats & CF_GODMODE)
                {
                    if (plyr->mo)
                    plyr->mo->health = 100;

                    plyr->health = deh_god_mode_health;
                    plyr->message = DEH_String(english_language ? STSTR_DQDON : STSTR_DQDON_RUS);
                }
                else 
                plyr->message = DEH_String(english_language ? STSTR_DQDOFF : STSTR_DQDOFF_RUS);
            }

            // 'fa' cheat for killer fucking arsenal
            else if (cht_CheckCheat(&cheat_ammonokey, ev->data2))
            {
                plyr->armorpoints = deh_idfa_armor;
                plyr->armortype = deh_idfa_armor_class;

                for (i=0;i<NUMWEAPONS;i++)
                    plyr->weaponowned[i] = true;

                // [JN] Проверяем, есть ли у игрока рюкзак. Если нет - выдаём.
                if (!plyr->backpack)
                {
                    for (i=0 ; i<NUMAMMO ; i++)
                    plyr->maxammo[i] *= 2;
                    plyr->backpack = true;
                }

                // [JN] ...и лишь затем пополняем боезапас.
                for (i=0;i<NUMAMMO;i++)
                    plyr->ammo[i] = plyr->maxammo[i];

                plyr->message = DEH_String(english_language ? STSTR_FAADDED : STSTR_FAADDED_RUS);
            }

            // 'kfa' cheat for key full ammo
            else if (cht_CheckCheat(&cheat_ammo, ev->data2))
            {
                plyr->armorpoints = deh_idkfa_armor;
                plyr->armortype = deh_idkfa_armor_class;

                for (i=0;i<NUMWEAPONS;i++)
                    plyr->weaponowned[i] = true;
	
                // [JN] Проверяем, есть ли у игрока рюкзак. Если нет - выдаём.
                if (!plyr->backpack)
                {
                    for (i=0 ; i<NUMAMMO ; i++)
                    plyr->maxammo[i] *= 2;
                    plyr->backpack = true;
                }

                // [JN] ...и лишь затем пополняем боезапас.
                for (i=0;i<NUMAMMO;i++)
                    plyr->ammo[i] = plyr->maxammo[i];

                for (i=0;i<NUMCARDS;i++)
                    plyr->cards[i] = true;

                plyr->message = DEH_String(english_language ? STSTR_KFAADDED : STSTR_KFAADDED_RUS);
            }

            // [JN] 'ka' чит для выдачи ключей
            else if (cht_CheckCheat(&cheat_keys, ev->data2))
            {
                for (i=0;i<NUMCARDS;i++)
                    plyr->cards[i] = true;

                plyr->message = DEH_String(english_language ? STSTR_KAADDED : STSTR_KAADDED_RUS);
            }

            // [crispy] implement Boom's "tntem" cheat
            // [JN] В несколько упрощенном варианте, без счетчика монстров
            else if (cht_CheckCheat(&cheat_massacre, ev->data2))
            {
                ST_cheat_massacre();
                plyr->message = DEH_String(english_language ? STSTR_MASSACRE : STSTR_MASSACRE_RUS);
            }

            // [JN] Отображение версии проекта
            else if (cht_CheckCheat(&cheat_version, ev->data2))
            {
                M_snprintf(msg, sizeof(msg), "%s%s - %s",
                english_language ? STSTR_VERSION : STSTR_VERSION_RUS,
                english_language ? STSRT_ARCH : STSRT_ARCH_RUS,
                english_language ? STSRT_DATE : STSRT_DATE_RUS);

                plyr->message = msg;
            }

            // 'mus' cheat for changing music
            else if (cht_CheckCheat(&cheat_mus, ev->data2))
            {
                char    buf[3];
                int     musnum;

                plyr->message = DEH_String(english_language ? STSTR_MUS : STSTR_MUS_RUS);
                cht_GetParam(&cheat_mus, buf);

                // Note: The original v1.9 had a bug that tried to play back
                // the Doom II music regardless of gamemode.  This was fixed
                // in the Ultimate Doom executable so that it would work for
                // the Doom 1 music as well.

                // [JN] Fixed: using a proper IDMUS selection for shareware 
                // and registered game versions.
                if (gamemode == commercial/* || gameversion < exe_ultimate*/)
                {
                    musnum = mus_runnin + (buf[0]-'0')*10 + buf[1]-'0' - 1;

                    // [crispy] prevent crash with IDMUS00
                    if ((((buf[0]-'0')*10 + buf[1]-'0') > 35 || musnum < mus_runnin) && gameversion >= exe_doom_1_8)
                        plyr->message = DEH_String(english_language ? STSTR_NOMUS : STSTR_NOMUS_RUS);
                    else
                        S_ChangeMusic(musnum, 1);
                }
                else
                {
                    musnum = mus_e1m1 + (buf[0]-'1')*9 + (buf[1]-'1');

                    // [crispy] prevent crash with IDMUS0x or IDMUSx0
                    if (((buf[0]-'1')*9 + buf[1]-'1') > 31 || buf[0] < '1' || buf[1] < '1')
                        plyr->message = DEH_String(english_language ? STSTR_NOMUS : STSTR_NOMUS_RUS);
                    else
                        S_ChangeMusic(musnum, 1);
                }
            }

            // [crispy] allow both idspispopd and idclip cheats in all gamemissions
            else if ( ( /* logical_gamemission == doom
            && */ cht_CheckCheat(&cheat_noclip, ev->data2))
            || ( /* logical_gamemission != doom
            && */ cht_CheckCheat(&cheat_commercial_noclip,ev->data2)))
            {	
                // Noclip cheat.
                // For Doom 1, use the idspipsopd cheat; for all others, use
                // idclip

                plyr->cheats ^= CF_NOCLIP;

                if (plyr->cheats & CF_NOCLIP)
                    plyr->message = DEH_String(english_language ? STSTR_NCON : STSTR_NCON_RUS);
                else
                    plyr->message = DEH_String(english_language ? STSTR_NCOFF : STSTR_NCOFF_RUS);
            }

            // 'behold?' power-up cheats
            for (i=0;i<6;i++)
            {
                if (cht_CheckCheat(&cheat_powerup[i], ev->data2))
                {
                    // [JN] Atari Jaguar: no invisibility sphere and light visor
                    if (gamemission == jaguar && (i == pw_invisibility || i == pw_infrared))
                    return false;

                    if (!plyr->powers[i])
                    {
                        P_GivePower( plyr, i);
                        plyr->message = DEH_String(english_language ? STSTR_BEHOLDX : STSTR_BEHOLDX_RUS); // [JN] Активирован
                    }
                    else if (i!=pw_strength)
                    {
                        plyr->powers[i] = 1;
                        plyr->message = DEH_String(english_language ? STSTR_BEHOLDZ : STSTR_BEHOLDZ_RUS); // [JN] Деактивирован
                    }
                    else
                    {
                        plyr->powers[i] = 0;
                        plyr->message = DEH_String(english_language ? STSTR_BEHOLDZ : STSTR_BEHOLDZ_RUS); // [JN] Деактивирован
                    }
                }
            }

            // 'behold' power-up menu
            if (cht_CheckCheat(&cheat_powerup[6], ev->data2))
            {
                if (gamemission == jaguar)  // [JN] Atari Jaguar: don't offer "i"nvisibility and "l"ight visor.
                plyr->message = DEH_String(english_language ? STSTR_BEHOLD_JAG : STSTR_BEHOLD_JAG_RUS);
                else
                plyr->message = DEH_String(english_language ? STSTR_BEHOLD : STSTR_BEHOLD_RUS);
            }

            // 'choppers' invulnerability & chainsaw
            else if (cht_CheckCheat(&cheat_choppers, ev->data2))
            {
                plyr->weaponowned[wp_chainsaw] = true;
                plyr->powers[pw_invulnerability] = true;
                plyr->message = DEH_String(english_language ? STSTR_CHOPPERS : STSTR_CHOPPERS_RUS);
            }

            // 'mypos' for player position
            else if (cht_CheckCheat(&cheat_mypos, ev->data2))
            {
                static char buf[ST_MSGWIDTH];

                M_snprintf(buf, sizeof(buf), english_language ?
                                             "ang=0x%x;x,y=(0x%x,0x%x)" : "eujk=%x / [<e=(%x<%x)",
                    players[consoleplayer].mo->angle,
                    players[consoleplayer].mo->x,
                    players[consoleplayer].mo->y);

                plyr->message = buf;
            }
        }

        // 'clev' change-level cheat
        if (!netgame && cht_CheckCheat(&cheat_clev, ev->data2) && gamemode != pressbeta)
        {
            char    buf[3];
            int     epsd;
            int     map;

            cht_GetParam(&cheat_clev, buf);

            if (gamemode == commercial)
            {
                epsd = 1;
                map = (buf[0] - '0')*10 + buf[1] - '0';
            }
            else
            {
                epsd = buf[0] - '0';
                map = buf[1] - '0';

                // Chex.exe always warps to episode 1.
                if (gameversion == exe_chex)
                {
                    if (epsd > 1)
                    {
                        epsd = 1;
                    }
                    if (map > 5)
                    {
                        map = 5;
                    }
                }
            }

            // Catch invalid maps.
            if (gamemode != commercial)
            {
                if (epsd < 1)
                {
                    return false;
                }
                if (epsd > 4)
                {
                    return false;
                }
                if (epsd == 4 && gameversion < exe_ultimate)
                {
                    return false;
                }
                if (map < 1)
                {
                    return false;
                }
                if (map > 9)
                {
                    return false;
                }
            }
            else
            {
                if (map < 1)
                {
                    return false;
                }
                if (map > 40)
                {
                    return false;
                }
                // [JN] В NRFTL не перемещаться на уровень 10 и выше
                if (map > 9 && gamemission == pack_nerve)
                {
                    return false;
                }
                // [JN] Atari Jaguar: dont warp to map 26 and higher
                if (map > 25 && gamemission == jaguar)
                {
                    return false;
                }
            }

            // So be it.
            plyr->message = DEH_String(english_language ? STSTR_CLEV : STSTR_CLEV_RUS);
            G_DeferedInitNew(gameskill, epsd, map);
        }

        // [JN] Finally, Press Beta cheats
        if (gamemode == pressbeta)
        {
            // 'TST' - god mode
            if (cht_CheckCheat(&cheat_god_beta, ev->data2))
            {
                plyr->cheats ^= CF_GODMODE;

                if (plyr->cheats & CF_GODMODE)
                {
                    if (plyr->mo)
                    plyr->mo->health = 100;

                    plyr->health = deh_god_mode_health;
                    plyr->message = DEH_String(english_language ? STSTR_DQDON : STSTR_DQDON_RUS);
                }
                else 
                plyr->message = DEH_String(english_language ? STSTR_DQDOFF : STSTR_DQDOFF_RUS);
            }

            // 'AMO' cheat for key full ammo
            else if (cht_CheckCheat(&cheat_ammo_beta, ev->data2))
            {
                plyr->armorpoints = deh_idkfa_armor;
                plyr->armortype = deh_idkfa_armor_class;

                for (i=0;i<NUMWEAPONS;i++)
                    plyr->weaponowned[i] = true;

                for (i=0;i<NUMAMMO;i++)
                    plyr->ammo[i] = plyr->maxammo[i];

                for (i=0;i<NUMCARDS;i++)
                    plyr->cards[i] = true;

                plyr->message = DEH_String(english_language ? STSTR_KFAADDED : STSTR_KFAADDED_RUS);
            }

            // 'NC' - noclipping mode
            else if (cht_CheckCheat(&cheat_noclip_beta, ev->data2))
            {	
                plyr->cheats ^= CF_NOCLIP;

                if (plyr->cheats & CF_NOCLIP)
                    plyr->message = DEH_String(english_language ? STSTR_NCON : STSTR_NCON_RUS);
                else
                    plyr->message = DEH_String(english_language ? STSTR_NCOFF : STSTR_NCOFF_RUS);
            }
        }
    }

    return false;
}


int ST_calcPainOffset(void)
{
    int         health;
    static int  lastcalc;
    static int  oldhealth = -1;

    health = plyr->health > 100 ? 100 : plyr->health;

    if (health != oldhealth)
    {
        lastcalc = ST_FACESTRIDE * (((100 - health) * ST_NUMPAINFACES) / 101);
        oldhealth = health;
    }

    return lastcalc;
}


//
// This is a not-very-pretty routine which handles
//  the face states and their timing.
// the precedence of expressions is:
//  dead > evil grin > turned head > straight ahead
//
void ST_updateFaceWidget(void)
{
    int         i;
    angle_t     badguyangle;
    angle_t     diffang;
    static int  lastattackdown = -1;
    static int  priority = 0;
    boolean     doevilgrin;

    int         painoffset;
    static int  faceindex;

    painoffset = ST_calcPainOffset();

    if (priority < 10)
    {
        // dead
        // [crispy] theoretically allow for negative player health values
        if (plyr->health <= 0)
        {
            priority = 9;
            painoffset = 0;
            faceindex = ST_DEADFACE;
            st_facecount = 1;
        }
        // [JN] Atari и PSX Doom - дополнительные лице "разорванного" и "раздавленного" игрока.
        // Proper checking for xdeath state has been taken from Crispy Doom, thanks to Fabian Greffrath!
        // Don't use if for possible custom HUD faces.
        if (!old_godface && plyr->health <= 0 && plyr->mo->state - states >= mobjinfo[plyr->mo->type].xdeathstate && !vanillaparm)
        {
            priority = 9;
            painoffset = 0;

            // [JN] Синхронизация лица с состоянием игрока
            if (plyr->mo->state == &states[S_PLAY_XDIE1])
                faceindex = ST_EXPLFACE0;
            if (plyr->mo->state == &states[S_PLAY_XDIE2])
                faceindex = ST_EXPLFACE1;
            if (plyr->mo->state == &states[S_PLAY_XDIE3])
                faceindex = ST_EXPLFACE2;
            if (plyr->mo->state == &states[S_PLAY_XDIE4])
                faceindex = ST_EXPLFACE3;
            if (plyr->mo->state == &states[S_PLAY_XDIE5])
                faceindex = ST_EXPLFACE4;
            if ((plyr->mo->state == &states[S_PLAY_XDIE6])
            || (plyr->mo->state == &states[S_PLAY_XDIE7])
            || (plyr->mo->state == &states[S_PLAY_XDIE8])
            || (plyr->mo->state == &states[S_PLAY_XDIE9]))
                faceindex = ST_EXPLFACE5;

            if (plyr->mo->state == &states[S_GIBS])
                faceindex = ST_CRSHFACE0;
        }
    }

    if (priority < 9)
    {
        if (plyr->bonuscount)
        {
            // picking up bonus
            doevilgrin = false;

            for (i=0;i<NUMWEAPONS;i++)
            {
                if (oldweaponsowned[i] != plyr->weaponowned[i])
                {
                    // [BH] no evil grin when invulnerable
                    // [JN] extra god faces have grin, use them in god mode
                    if ((old_godface && !(plyr->cheats & CF_GODMODE) && !plyr->powers[pw_invulnerability])
                    || !old_godface)
                    {
                        doevilgrin = true;
                        oldweaponsowned[i] = plyr->weaponowned[i];
                    }
                }
            }

            if (doevilgrin) 
            {
                // evil grin if just picked up weapon
                priority = 8;
                st_facecount = ST_EVILGRINCOUNT;
                faceindex = ST_EVILGRINOFFSET;
            }
        }
    }

    if (priority < 8) 
    { 	    // being attacked
        if (plyr->damagecount && plyr->attacker && plyr->attacker != plyr->mo)
        {
            // [JN] Исправление бага с отсутствующим Ouch Face.
            // По методу Brad Harding (Doom Retro).

            // [JN] Корректная формула "Ouch face"
            if (!vanillaparm && gamemode != pressbeta)
            {
                // [BH] fix ouch-face when damage > 20
                if (st_oldhealth - plyr->health > ST_MUCHPAIN)
                {
                    st_facecount = ST_TURNCOUNT;
                    faceindex = ST_OUCHOFFSET;
                    priority = 8;   // [BH] keep ouch-face visible
                }
                else
                {
                    angle_t badguyangle = R_PointToAngle2(plyr->mo->x, plyr->mo->y, plyr->attacker->x,plyr->attacker->y);
                    angle_t diffang;

                    if (badguyangle > plyr->mo->angle)
                    {
                        // whether right or left
                        diffang = badguyangle - plyr->mo->angle;
                        i = (diffang > ANG180);
                    }
                    else
                    {
                        // whether left or right
                        diffang = plyr->mo->angle - badguyangle;
                        i = (diffang <= ANG180);
                    }       // confusing, ain't it?

                    st_facecount = ST_TURNCOUNT;

                    if (diffang < ANG45)
                    {
                        // head-on
                        faceindex = ST_RAMPAGEOFFSET;
                    }
                    else if (i)
                    {
                        // turn face right
                        faceindex = ST_TURNOFFSET;
                    }
                    else
                    {
                        // turn face left
                        faceindex = ST_TURNOFFSET+1;
                    }
                }
            }

            // [JN] Традиционная формула
            else
            {
                if (plyr->health - st_oldhealth > ST_MUCHPAIN)
                {
                    st_facecount = ST_TURNCOUNT;
                    faceindex = ST_OUCHOFFSET;
                    priority = 7; // [JN] Традиционный приоритет
                }
                else
                {
                    badguyangle = R_PointToAngle2(plyr->mo->x, plyr->mo->y, plyr->attacker->x, plyr->attacker->y);

                    if (badguyangle > plyr->mo->angle)
                    {
                        // whether right or left
                        diffang = badguyangle - plyr->mo->angle;
                        i = diffang > ANG180; 
                    }
                    else
                    {
                        // whether left or right
                        diffang = plyr->mo->angle - badguyangle;
                        i = diffang <= ANG180; 
                    } // confusing, aint it?

                    st_facecount = ST_TURNCOUNT;

                    if (diffang < ANG45)
                    {
                        // head-on    
                        faceindex = ST_RAMPAGEOFFSET;
                    }
                    else if (i)
                    {
                        // turn face right
                        faceindex = ST_TURNOFFSET;
                    }
                    else
                    {
                        // turn face left
                        faceindex = ST_TURNOFFSET+1;
                    }
                }
            }
        }	
    }

    // [JN] Исправление бага с отсутствующим Ouch Face.
    // По методу Brad Harding (Doom Retro).

    if (priority < 7)
    {
        // getting hurt because of your own damn stupidity
        if (plyr->damagecount)
        {
            if (gamemode != pressbeta)
            {
            if (extra_player_faces && !vanillaparm)
            {
                if (st_oldhealth - plyr->health > ST_MUCHPAIN)
                {
                priority = 7;
                st_facecount = ST_TURNCOUNT;
                faceindex = ST_OUCHOFFSET;
                }
                else
                {
                priority = 6;
                st_facecount = ST_TURNCOUNT;
                faceindex = ST_RAMPAGEOFFSET;
                }
            }

            else
            {
                if (plyr->health - st_oldhealth > ST_MUCHPAIN)
                {
                    priority = 7;
                    st_facecount = ST_TURNCOUNT;
                    faceindex = ST_OUCHOFFSET;
                }
                else
                {
                    priority = 6;
                    st_facecount = ST_TURNCOUNT;
                    faceindex = ST_RAMPAGEOFFSET;
                }
            }
            }

            // [JN] Press Beta: OUCH face is appearing only after getting
            // hurt in special sectors (slime, blood, etc). Emulate this.
            else
            {
                priority = 7;
                st_facecount = ST_TURNCOUNT;
                faceindex = ST_RAMPAGEOFFSET;
                
                if (plyr->mo->subsector->sector->special)
                {
                    priority = 6;
                    faceindex = ST_OUCHOFFSET;
                }
            }
        }
    }

    if (priority < 6)
    {
        // rapid firing
        if (plyr->attackdown)
        {
            // [BH] no rampage face when invulnerable
            // [JN] extra god faces have rampage, use them in god mode
            if ((old_godface && !(plyr->cheats & CF_GODMODE) && !plyr->powers[pw_invulnerability])
            || !old_godface)
            {
                if (lastattackdown==-1)
                    lastattackdown = ST_RAMPAGEDELAY;
                else if (!--lastattackdown)
                {
                    priority = 5;
                    faceindex = ST_RAMPAGEOFFSET;
                    st_facecount = 1;
                    lastattackdown = 1;
                }
            }
        }
        else
        {
            lastattackdown = -1;
        }
    }

    if (priority < 5)
    {
        // invulnerability
        if ((plyr->cheats & CF_GODMODE) || plyr->powers[pw_invulnerability])
        {
            priority = 4;
            faceindex = ST_GODFACE;
            if (old_godface)
            {
                // [JN] Standard god mode face behaviour
                st_facecount = 1;
                painoffset = 0;
            }
            else
            {
                // [JN] Activate extra bloody god mode faces
                st_facecount = 0;
            }
        }
    }

    // look left or look right if the facecount has timed out
    if (!st_facecount)
    {
        faceindex = st_randomnumber % 3;
        st_facecount = ST_STRAIGHTFACECOUNT;
        priority = 0;
    }

    st_facecount--;
    st_faceindex = painoffset + faceindex;

    // [JN] При наличии бессмертия активируется дополнительный цикл.
    if (!old_godface && ((plyr->powers[pw_invulnerability]) || (plyr->cheats & CF_GODMODE)))
    {
        st_faceindex = painoffset + faceindex + ST_NUMFACES;
    }
}


void ST_updateWidgets(void)
{
    static int largeammo = 1994; // means "n/a"
    int        i;

    // must redirect the pointer if the ready weapon has changed.
    //  if (w_ready.data != plyr->readyweapon)
    //  {
    if (weaponinfo[plyr->readyweapon].ammo == am_noammo)
        w_ready.num = &largeammo;
    else
        w_ready.num = &plyr->ammo[weaponinfo[plyr->readyweapon].ammo];
    //{
    // static int tic=0;
    // static int dir=-1;
    // if (!(tic&15))
    //   plyr->ammo[weaponinfo[plyr->readyweapon].ammo]+=dir;
    // if (plyr->ammo[weaponinfo[plyr->readyweapon].ammo] == -100)
    //   dir = 1;
    // tic++;
    // }
    w_ready.data = plyr->readyweapon;

    // if (*w_ready.on)
    //  STlib_updateNum(&w_ready, true);
    // refresh weapon change
    //  }

    // update keycard multiple widgets
    for (i=0;i<3;i++)
    {
        keyboxes[i] = plyr->cards[i] ? i : -1;

        //jff 2/24/98 select double key
        // [JN] Press Beta have a bug with missing skull keys on HUD.
        // To emulate this, following condition must be commented out:

        if (plyr->cards[i+3])
        keyboxes[i] = (keyboxes[i]==-1) ? i+3 : i+6;
    }

    // refresh everything if this is him coming back to life
    ST_updateFaceWidget();

    // used by the w_armsbg widget
    st_notdeathmatch = !deathmatch;

    // used by w_arms[] widgets
    st_armson = st_statusbaron && !deathmatch; 

    // used by w_frags widget
    st_fragson = deathmatch && st_statusbaron; 
    st_fragscount = 0;

    for (i=0 ; i<MAXPLAYERS ; i++)
    {
        if (i != consoleplayer)
            st_fragscount += plyr->frags[i];
        else
            st_fragscount -= plyr->frags[i];
    }

    // [JN] Press Beta: artifacts counter routines
    if (gamemode == pressbeta)
    {   
        st_artifactson = !deathmatch && st_statusbaron; 
        st_artifactscount = 0;

        // [JN] Overflow guard, just for reliability
        if (st_artifactscount < 0)
            st_artifactscount = 0;
        if (st_artifactscount > 99)
            st_artifactscount = 99;

        st_artifactscount += artifactcount;
    }

    // get rid of chat window if up because of message
    if (!--st_msgcounter)
    st_chat = st_oldchat;
}


void ST_Ticker (void)
{
    st_clock++;
    st_randomnumber = M_Random();
    ST_updateWidgets();
    st_oldhealth = plyr->health;
}

int st_palette = 0;


void ST_doPaletteStuff(void)
{
    int     palette;
    byte*   pal;
    int     cnt;
    int     bzc;

    cnt = plyr->damagecount;

    if (plyr->powers[pw_strength])
    {
        // slowly fade the berzerk out
        bzc = 12 - (plyr->powers[pw_strength]>>6);

        if (bzc > cnt)
        cnt = bzc;
    }

    if (cnt)
    {
        palette = (cnt+7)>>3;

        if (palette >= NUMREDPALS)
        palette = NUMREDPALS-1;

        palette += STARTREDPALS;
    }

    // [JN] Изменение палитры при получении бонусов
    else if (plyr->bonuscount)
    {
        palette = (plyr->bonuscount+7)>>3;
        
        // [JN] Дополнительный фрейм палитры для более плавного
        // появления/угасания жёлтого экрана при подборе предметов.
        // https://doomwiki.org/wiki/PLAYPAL
        // [JN] 11.09.2018 - не применять фикс в режиме -vanilla и для Atari Jaguar.

        if (palette >= NUMBONUSPALS)
        palette = (vanillaparm || gamemission == jaguar) ?
                   NUMBONUSPALS-1 : NUMBONUSPALS;

        palette += (vanillaparm || gamemission == jaguar) ?
                    STARTBONUSPALS : STARTBONUSPALS-1;
    }

    // [JN] Don't replace CYAN palette with GREEN palette in Atari Jaguar
    else if ( plyr->powers[pw_ironfeet] > 4*32 || plyr->powers[pw_ironfeet]&8)
    {
        palette = RADIATIONPAL;
    }
    else
    {
        palette = 0;
    }
    
    // [JN] Use CYAN invulnerability palette in Atari Jaguar,
    // unbreakable by other palettes
    if (gamemission == jaguar &&
    (plyr->powers[pw_invulnerability] > 4*32 || (plyr->powers[pw_invulnerability]&8)))
    {
        palette = INVULNERABILITYPAL;
    }

    // In Chex Quest, the player never sees red.  Instead, the
    // radiation suit palette is used to tint the screen green,
    // as though the player is being covered in goo by an
    // attacking flemoid.

    if (gameversion == exe_chex && palette >= STARTREDPALS && palette < STARTREDPALS + NUMREDPALS)
    {
        palette = RADIATIONPAL;
    }

    if (palette != st_palette)
    {
        st_palette = palette;
        pal = (byte *) W_CacheLumpNum ((usegamma <= 8 ? 
                                        lu_palette1 : 
                                        lu_palette2), 
                                        PU_CACHE) + palette * 768;
        I_SetPalette (pal);
    }
}


void ST_drawWidgets(boolean refresh)
{
    int     i;

    // used by w_arms[] widgets
    st_armson = st_statusbaron && !deathmatch;

    // used by w_frags widget
    st_fragson = deathmatch && st_statusbaron; 

    // [JN] used by w_artifacts widget
    st_artifactson = !deathmatch && st_statusbaron; 

    STlib_updateNum(&w_ready, refresh);

    // [crispy] draw "special widgets" in the Crispy HUD
    if ((screenblocks == 11 || screenblocks == 12 || screenblocks == 13) && !automapactive)
    {
        // [crispy] draw berserk pack instead of no ammo if appropriate
        if (plyr->readyweapon == wp_fist && plyr->powers[pw_strength])
        {
            static patch_t *patch;

            if (!patch)
            {
                const int lump = W_CheckNumForName(DEH_String("PSTRA0"));

                if (lump >= 0)
                patch = W_CacheLumpNum(lump, PU_STATIC);
            }

            if (patch)
            {
                // [crispy] (23,179) is the center of the Ammo widget
                V_DrawPatch(23 - SHORT(patch->width)/2 + SHORT(patch->leftoffset),
                            179 - SHORT(patch->height)/2 + SHORT(patch->topoffset),
                            patch);
            }
        }
    }

    for (i=0;i<4;i++)
    {
        STlib_updateNum(&w_ammo[i], refresh);
        STlib_updateNum(&w_maxammo[i], refresh);
    }

    // [JN] Signed Crispy HUD: no STBAR backbround, with player's face/background
    if (screenblocks == 11 && !automapactive)
    {
        if (netgame)    // [JN] Account player's color in network game
        V_DrawPatch(ST_FX+ORIGWIDTH_DELTA, ST_FY, faceback);
        else            // [JN] Use only gray color in single player
        V_DrawPatch(ST_FX+ORIGWIDTH_DELTA, ST_FY, W_CacheLumpName(DEH_String("STFB1"), PU_CACHE));
    }
    
    // [JN] Signed Crispy HUD: no STBAR backbround, without player's face/background
    if (screenblocks == 11 || screenblocks == 12)
    {
        if (!automapactive) // [JN] Don't draw signs in automap
        {
            // [JN] Don't draw ammo for fist and chainsaw
            if (plyr->readyweapon == wp_pistol
             || plyr->readyweapon == wp_shotgun
             || plyr->readyweapon == wp_supershotgun
             || plyr->readyweapon == wp_chaingun
             || plyr->readyweapon == wp_missile
             || plyr->readyweapon == wp_plasma
             || plyr->readyweapon == wp_bfg)
                V_DrawPatch(2+ORIGWIDTH_DELTA, 191, W_CacheLumpName(DEH_String("STCHAMMO"), PU_CACHE));

            if (deathmatch) // [JN] Frags
            V_DrawPatch(108+ORIGWIDTH_DELTA, 191, W_CacheLumpName(DEH_String("STCHFRGS"), PU_CACHE));
            else            // [JN] Arms
            V_DrawPatch(108+ORIGWIDTH_DELTA, 191, W_CacheLumpName(DEH_String("STCHARMS"), PU_CACHE));

            // [JN] Health, armor, ammo
            V_DrawPatch(52+ORIGWIDTH_DELTA, 173, W_CacheLumpName(DEH_String("STCHNAMS"), PU_CACHE));
        }

        V_DrawPatch(292+ORIGWIDTH_DELTA, 173, W_CacheLumpName(DEH_String("STYSSLSH"), PU_CACHE));
    }

    // [JN] Traditional Crispy HUD
    if (screenblocks == 13)
    V_DrawPatch(292+ORIGWIDTH_DELTA, 173, W_CacheLumpName(DEH_String("STYSSLSH"), PU_CACHE));

    STlib_updatePercent(&w_health, refresh || screenblocks == 11 || screenblocks == 12 || screenblocks == 13);
    STlib_updatePercent(&w_armor, refresh || screenblocks == 11 || screenblocks == 12 || screenblocks == 13);

    // [JN] Don't update/draw ARMS background in Press Beta
    if ((screenblocks < 11 || automapactive) && gamemode != pressbeta)
    STlib_updateBinIcon(&w_armsbg, refresh);

    // [JN] Don't update/draw ARMS section in Press Beta
    if (gamemode != pressbeta)
    {
    for (i=0;i<6;i++)
    STlib_updateMultIcon(&w_arms[i], refresh || screenblocks == 11 || screenblocks == 12 || screenblocks == 13);
    }

    if (screenblocks < 12 || automapactive)
    STlib_updateMultIcon(&w_faces, refresh || screenblocks == 11);

    for (i=0;i<3;i++)
    STlib_updateMultIcon(&w_keyboxes[i], refresh || screenblocks == 11 || screenblocks == 12 || screenblocks == 13);

    STlib_updateNum(&w_frags, refresh || screenblocks == 11 || screenblocks == 12 || screenblocks == 13);
    
    // [JN] Press Beta: some special routine. I need to draw Artifacts widet while not in
    // automap and Arms widget while in automap. Plus, background must be redrawn immediately.
    // Also see AM_Stop at am_map.c
    if (gamemode == pressbeta)
    {
        if (!automapactive)
        {
            // [JN] Draw Artifacts widet
            STlib_updateNum(&w_artifacts, refresh || screenblocks == 11 || screenblocks == 12 || screenblocks == 13);
        }
        else
        {
            // [JN] Draw Arms widet. Background (w_armsbg) and numbers (w_arms)
            STlib_updateBinIcon(&w_armsbg, refresh || screenblocks == 11 || screenblocks == 12 || screenblocks == 13);

            for (i=0;i<6;i++)
            STlib_updateMultIcon(&w_arms[i], refresh || screenblocks == 11 || screenblocks == 12 || screenblocks == 13);
        }

        // [JN] Draw player's life widget only in traditional HUD, Crispy HUD with player's face and automap
        if (screenblocks <= 11 || automapactive)
        STlib_updateNum(&w_lifes, refresh);
    }
}


void ST_doRefresh(void)
{
    st_firsttime = false;

    // draw status bar background to off-screen buff
    ST_refreshBackground();

    // and refresh all widgets
    ST_drawWidgets(true);
}


void ST_diffDraw(void)
{
    // update all widgets
    ST_drawWidgets(false);
}


void ST_Drawer (boolean fullscreen, boolean refresh)
{
    // [JN] Redraw whole status bar while in HELP screens.
    // Fixes a notable delay of HUD redraw after closing HELP screen.
    extern boolean inhelpscreens;
    
    st_statusbaron = (!fullscreen) || automapactive || screenblocks == 11 || screenblocks == 12;
    st_firsttime = st_firsttime || refresh || inhelpscreens;

    // Do red-/gold-shifts from damage/items
    ST_doPaletteStuff();

    // If just after ST_Start(), refresh all
    if (st_firsttime)
        ST_doRefresh();
    // Otherwise, update as little as possible
    else 
        ST_diffDraw();
}

typedef void (*load_callback_t)(char *lumpname, patch_t **variable);


// Iterates through all graphics to be loaded or unloaded, along with
// the variable they use, invoking the specified callback function.

static void ST_loadUnloadGraphics(load_callback_t callback)
{
    int     i;
    int     j;
    int     facenum;

    char    namebuf[9];

    // Load the numbers, tall and short
    for (i=0;i<10;i++)
    {
        DEH_snprintf(namebuf, 9, "STTNUM%d", i);
        callback(namebuf, &tallnum[i]);

        DEH_snprintf(namebuf, 9, "STYSNUM%d", i);
        callback(namebuf, &shortnum[i]);
    }

    // Load percent key.
    //Note: why not load STMINUS here, too?

    callback(DEH_String("STTPRCNT"), &tallpercent);

    // key cards
    for (i=0;i<NUMCARDS+3;i++)  //jff 2/23/98 show both keys too
    {
        DEH_snprintf(namebuf, 9, "STKEYS%d", i);
        callback(namebuf, &keys[i]);
    }

    // arms background
    callback(DEH_String("STARMS"), &armsbg);

    // arms ownership widgets
    for (i=0; i<6; i++)
    {
        DEH_snprintf(namebuf, 9, "STGNUM%d", i+2);

        // gray #
        callback(namebuf, &arms[i][0]);

        // yellow #
        arms[i][1] = shortnum[i+2]; 
    }

    // face backgrounds for different color players
    DEH_snprintf(namebuf, 9, "STFB%d", consoleplayer);
    callback(namebuf, &faceback);

    // status bar background bits
    callback(DEH_String("STBAR"), &sbar);

    // face states
    facenum = 0;
    for (i=0; i<ST_NUMPAINFACES; i++)
    {
        for (j=0; j<ST_NUMSTRAIGHTFACES; j++)
        {
            DEH_snprintf(namebuf, 9, "STFST%d%d", i, j);
            callback(namebuf, &faces[facenum]);
            ++facenum;
        }

        DEH_snprintf(namebuf, 9, "STFTR%d0", i);	// turn right
        callback(namebuf, &faces[facenum]);
        ++facenum;

        DEH_snprintf(namebuf, 9, "STFTL%d0", i);	// turn left
        callback(namebuf, &faces[facenum]);
        ++facenum;

        DEH_snprintf(namebuf, 9, "STFOUCH%d", i);	// ouch!
        callback(namebuf, &faces[facenum]);
        ++facenum;

        DEH_snprintf(namebuf, 9, "STFEVL%d", i);	// evil grin ;)
        callback(namebuf, &faces[facenum]);
        ++facenum;

        DEH_snprintf(namebuf, 9, "STFKILL%d", i);	// pissed off
        callback(namebuf, &faces[facenum]);
        ++facenum;
    }

    callback(DEH_String("STFGOD0"), &faces[facenum]);
    ++facenum;
    callback(DEH_String("STFDEAD0"), &faces[facenum]);
    ++facenum;
    callback(DEH_String("STFEXPL0"), &faces[facenum]);
    ++facenum;
    callback(DEH_String("STFEXPL1"), &faces[facenum]);
    ++facenum;
    callback(DEH_String("STFEXPL2"), &faces[facenum]);
    ++facenum;
    callback(DEH_String("STFEXPL3"), &faces[facenum]);
    ++facenum;
    callback(DEH_String("STFEXPL4"), &faces[facenum]);
    ++facenum;
    callback(DEH_String("STFEXPL5"), &faces[facenum]);
    ++facenum;
    callback(DEH_String("STFCRSH0"), &faces[facenum]);
    ++facenum;

    // [JN] Удвоение массива спрайтов лиц, необходимое для бессмертия.
    if (!old_godface)
    {
        for (i = 0; i < ST_NUMPAINFACES; i++)
        {
            for (j = 0; j < ST_NUMSTRAIGHTFACES; j++)
            {
                M_snprintf(namebuf, 9, "STFST%i%iG", i, j);
                callback(namebuf, &faces[facenum++]);
            }

            M_snprintf(namebuf, 9, "STFTR%i0G", i);          // turn right
            callback(namebuf, &faces[facenum++]);

            M_snprintf(namebuf, 9, "STFTL%i0G", i);          // turn left
            callback(namebuf, &faces[facenum++]);

            M_snprintf(namebuf, 9, "STFOUC%iG", i);         // ouch!
            callback(namebuf, &faces[facenum++]);

            M_snprintf(namebuf, 9, "STFEVL%iG", i);          // evil grin ;)
            callback(namebuf, &faces[facenum++]);

            M_snprintf(namebuf, 9, "STFKIL%iG", i);         // pissed off
            callback(namebuf, &faces[facenum++]);
        }

        callback("STFGOD0G", &faces[facenum++]);
        callback("STFDEA0G", &faces[facenum++]);
        callback("STFEXP0G", &faces[facenum++]);
        callback("STFEXP1G", &faces[facenum++]);
        callback("STFEXP2G", &faces[facenum++]);
        callback("STFEXP3G", &faces[facenum++]);
        callback("STFEXP4G", &faces[facenum++]);
        callback("STFEXP5G", &faces[facenum++]);
        callback("STFCRS0G", &faces[facenum++]);
    }
}


static void ST_loadCallback(char *lumpname, patch_t **variable)
{
    *variable = W_CacheLumpName(lumpname, PU_STATIC);
}


void ST_loadGraphics(void)
{
    ST_loadUnloadGraphics(ST_loadCallback);
}


void ST_loadData(void)
{
    lu_palette1 = W_GetNumForName (DEH_String("PALFIX"));
    lu_palette2 = W_GetNumForName (DEH_String("PLAYPAL"));

    ST_loadGraphics();
}


static void ST_unloadCallback(char *lumpname, patch_t **variable)
{
    W_ReleaseLumpName(lumpname);
    *variable = NULL;
}


void ST_unloadGraphics(void)
{
    ST_loadUnloadGraphics(ST_unloadCallback);
}


void ST_unloadData(void)
{
    ST_unloadGraphics();
}


void ST_initData(void)
{
    int	    i;

    st_firsttime = true;
    plyr = &players[consoleplayer];

    st_clock = 0;
    st_chatstate = StartChatState;
    st_gamestate = FirstPersonState;

    st_statusbaron = true;
    st_oldchat = st_chat = false;
    st_cursoron = false;

    st_faceindex = 0;
    st_palette = -1;

    st_oldhealth = -1;

    for (i=0;i<NUMWEAPONS;i++)
    oldweaponsowned[i] = plyr->weaponowned[i];

    for (i=0;i<3;i++)
    keyboxes[i] = -1;

    STlib_init();
}


void ST_createWidgets(void)
{
    int i;

    // ready weapon ammo
    STlib_initNum(&w_ready,
        ST_AMMOX,
        ST_AMMOY,
        tallnum,
        &plyr->ammo[weaponinfo[plyr->readyweapon].ammo],
        &st_statusbaron,
        ST_AMMOWIDTH );

    // the last weapon type
    w_ready.data = plyr->readyweapon; 

    // health percentage
    STlib_initPercent(&w_health,
        ST_HEALTHX,
        ST_HEALTHY,
        tallnum,
        &plyr->health,
        &st_statusbaron,
        tallpercent);

    // arms background
    STlib_initBinIcon(&w_armsbg,
        ST_ARMSBGX,
        ST_ARMSBGY,
        armsbg,
        &st_notdeathmatch,
        &st_statusbaron);

    // weapons owned
    for(i=0;i<6;i++)
    {
        STlib_initMultIcon(&w_arms[i],
            ST_ARMSX+(i%3)*ST_ARMSXSPACE,
            ST_ARMSY+(i/3)*ST_ARMSYSPACE,
            arms[i],
            &plyr->weaponowned[i+1],
            &st_armson);
    }

    // frags sum
    STlib_initNum(&w_frags,
        ST_FRAGSX,
        ST_FRAGSY,
        tallnum,
        &st_fragscount,
        &st_fragson,
        ST_FRAGSWIDTH);

    // [JN] Press Beta artifacts sum
    STlib_initNum(&w_artifacts,
        ST_FRAGSX+1,
        ST_FRAGSY,
        tallnum,
        &st_artifactscount,
        &st_artifactson,
        ST_FRAGSWIDTH);

    // [JN] Press Beta: player's lifes sum
    STlib_initNum(&w_lifes,
        ST_LIFESX,
        ST_LIFESY,
        shortnum,
        &lifecount,
        &st_statusbaron,
        ST_LIFESWIDTH);

    // faces
    STlib_initMultIcon(&w_faces,
        ST_FACESX,
        ST_FACESY,
        faces,
        &st_faceindex,
        &st_statusbaron);

    // armor percentage - should be colored later
    STlib_initPercent(&w_armor,
        ST_ARMORX,
        ST_ARMORY,
        tallnum,
        &plyr->armorpoints,
        &st_statusbaron, tallpercent);

    // keyboxes 0-2
    STlib_initMultIcon(&w_keyboxes[0],
        ST_KEY0X,
        ST_KEY0Y,
        keys,
        &keyboxes[0],
        &st_statusbaron);

    STlib_initMultIcon(&w_keyboxes[1],
        ST_KEY1X,
        ST_KEY1Y,
        keys,
        &keyboxes[1],
        &st_statusbaron);

    STlib_initMultIcon(&w_keyboxes[2],
        ST_KEY2X,
        ST_KEY2Y,
        keys,
        &keyboxes[2],
        &st_statusbaron);

    // ammo count (all four kinds)
    STlib_initNum(&w_ammo[0],
        ST_AMMO0X,
        ST_AMMO0Y,
        shortnum,
        &plyr->ammo[0],
        &st_statusbaron,
        ST_AMMO0WIDTH);

    STlib_initNum(&w_ammo[1],
        ST_AMMO1X,
        ST_AMMO1Y,
        shortnum,
        &plyr->ammo[1],
        &st_statusbaron,
        ST_AMMO1WIDTH);

    STlib_initNum(&w_ammo[2],
        ST_AMMO2X,
        ST_AMMO2Y,
        shortnum,
        &plyr->ammo[2],
        &st_statusbaron,
        ST_AMMO2WIDTH);

    STlib_initNum(&w_ammo[3],
        ST_AMMO3X,
        ST_AMMO3Y,
        shortnum,
        &plyr->ammo[3],
        &st_statusbaron,
        ST_AMMO3WIDTH);

    // max ammo count (all four kinds)
    STlib_initNum(&w_maxammo[0],
        ST_MAXAMMO0X,
        ST_MAXAMMO0Y,
        shortnum,
        &plyr->maxammo[0],
        &st_statusbaron,
        ST_MAXAMMO0WIDTH);

    STlib_initNum(&w_maxammo[1],
        ST_MAXAMMO1X,
        ST_MAXAMMO1Y,
        shortnum,
        &plyr->maxammo[1],
        &st_statusbaron,
        ST_MAXAMMO1WIDTH);

    STlib_initNum(&w_maxammo[2],
        ST_MAXAMMO2X,
        ST_MAXAMMO2Y,
        shortnum,
        &plyr->maxammo[2],
        &st_statusbaron,
        ST_MAXAMMO2WIDTH);

    STlib_initNum(&w_maxammo[3],
        ST_MAXAMMO3X,
        ST_MAXAMMO3Y,
        shortnum,
        &plyr->maxammo[3],
        &st_statusbaron,
        ST_MAXAMMO3WIDTH);
}

static boolean	st_stopped = true;


void ST_Start (void)
{
    if (!st_stopped)
    ST_Stop();

    ST_initData();
    ST_createWidgets();
    st_stopped = false;
}


void ST_Stop (void)
{
    if (st_stopped)
    return;

    I_SetPalette (W_CacheLumpNum ((usegamma <= 8 ? 
                                   lu_palette1 : 
                                   lu_palette2), 
                                   PU_CACHE));

    st_stopped = true;
}


void ST_Init (void)
{
    ST_loadData();
    st_backing_screen = (byte *) Z_Malloc((ST_WIDTH << hires) * (ST_HEIGHT << hires), PU_STATIC, 0);
}
