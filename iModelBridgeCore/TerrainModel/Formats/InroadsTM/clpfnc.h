//---------------------------------------------------------------------------+
// $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+
#pragma once

/*----------------------------------------------------------------------------*/
/* Data structures                                                            */
/*----------------------------------------------------------------------------*/

struct AECclip
    {
    DPoint3d p, nrm;
    struct AECclip *nxt, *nst;
    short nestLevel;                     /* 1: odd nest level, 0: even          */
    byte prvPar;                      /* previous parallel state             */
    byte noPar;                       /* no parallel sides allowed           */
    byte pad[2];                      /* pad structure to 8-byte boundary    */
    };

/*----------------------------------------------------------------------------*/
/* Prototypes.                                                                */
/*----------------------------------------------------------------------------*/

struct AECclip *aecClip_create /* <= NULL if error                 */
    (
    size_t numVrts,                        /* => number of polygon pnts           */
    DPoint3d *vrtsP,                     /* => polygon coordinates              */
    int dontAllowParallelSides           /* => TRUE: no par. sides              */
    );

void aecClip_free
    (
    struct AECclip *clpP                /* => clipping descriptor              */
    );

void aecClip_string
    (
    void *mdlDescP,                      /* => mdl desc. (or NULL)              */
    struct AECclip *clpP,                /* => clipping descriptor              */
    size_t numVrt,                         /* => # points in string               */
    DPoint3d *vrtsP,                     /* => coordinates of string            */
    int type,                            /* => 0: open, 1: closed               */
    void *insideUserDataP,               /* => user data for inside             */
    void *outsideUserDataP,              /* => user data for outside            */
    void (*insideFunctionP)(),           /* => function for inside              */
    void (*outsideFunctionP)()           /* => function for outside             */
    );
