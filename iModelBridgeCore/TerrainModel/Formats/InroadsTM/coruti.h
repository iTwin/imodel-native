//---------------------------------------------------------------------------+
// $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+
/*----------------------------------------------------------------------------*/
/* coruti.h                                              aec    26-Apr-2002   */
/*----------------------------------------------------------------------------*/
/* Function prototypes for DTM corridors functions.                           */
/*----------------------------------------------------------------------------*/

#pragma once

/*----------------------------------------------------------------------------*/
/* Dependent include files                                                    */
/*----------------------------------------------------------------------------*/
#include <portable.h>
#include <dtmstr.h>
#include <dtmedt.h>

/*----------------------------------------------------------------------------*/
/* Function prototypes                                                        */
/*----------------------------------------------------------------------------*/

int aecDTM_sendAllCorridors /* <= TRUE if error                    */
(
    void *mdlDescP,                    /* => mdl app desc. (or NULL)          */
    struct CIVdtmsrf *srfP,            /* => surface with corridors           */
    int opt,                           /* => options                          */
    int (*usrfncP)(void *,             /* => your function                    */
        struct CIVdtmsrf *,
        struct CIVdtmcor *),
    void *datP                         /* => your data                        */
);

int aecDTM_isCorridorDeletedFlagSet   /* <= TRUE if set      */
(
    struct CIVdtmcor  *corP            /* => corridor to check          */
);

int aecDTM_indexCorridorsByGuid
(
    struct CIVdtmsrf *srfP                  /*  => surface              */
);

int aecDTM_indexCorridorsByName
(
    struct CIVdtmsrf *srfP                  /*  => surface              */
);

int aecDTM_indexCorridorsComponentsMembers
(
    struct CIVdtmsrf *srfP                  /*  => surface              */
);

int aecDTM_destroyCorridorsComponentsMembersIndexes
(
    struct CIVdtmsrf *srfP                  /*  => surface              */
);

int aecDTM_destroyCorridorsGuidIndex
(
    struct CIVdtmsrf *srfP                  /*  => surface              */
);

int aecDTM_destroyCorridorsNameIndex
(
    struct CIVdtmsrf *srfP                  /*  => surface              */
);
