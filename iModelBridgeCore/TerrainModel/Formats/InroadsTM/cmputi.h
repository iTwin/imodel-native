//---------------------------------------------------------------------------+
// $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+
/*----------------------------------------------------------------------------*/
/* cmputi.h                                              aec    26-Apr-2002   */
/*----------------------------------------------------------------------------*/
/* Function prototypes for DTM component functions.                           */
/*----------------------------------------------------------------------------*/

#pragma once


/*----------------------------------------------------------------------------*/
/* Dependent include files                                                    */
/*----------------------------------------------------------------------------*/
#include <portable.h>
#include <dtmstr.h>
#include <dtmedt.h>

typedef struct
{
    wchar_t name[DTM_C_NAMSIZ];
    wchar_t style[DTM_C_NAMSIZ];
    wchar_t desc[DTM_C_NAMSIZ];
    struct CIVdtmsrf *srfP;
    BeSQLite::BeGuid *guidsP;
    int numGuids;
    BOOL bStyleExists;
    BOOL bPassFilter;
} ComponentListItem;

/*----------------------------------------------------------------------------*/
/* Function prototypes                                                        */
/*----------------------------------------------------------------------------*/

struct CIVdtmcmp *aecDTM_findComponentByGuid /* <= pointer to component       */
(
    struct CIVdtmsrf *srfP,                 /* => DTM surface (or NULL)       */
    BeSQLite::BeGuid *guidP                             /* => component guid pointer      */
);

int aecDTM_deleteComponent /* <= TRUE if error                         */
(
    struct CIVdtmsrf *srfP,         /* => surface with feature (or NULL)          */
    struct CIVdtmcmp *cmpP          /* => feature to delete                       */
);

int aecDTM_sendAllComponents /* <= TRUE if error                    */
(
    void *mdlDescP,                    /* => mdl app desc. (or NULL)          */
    struct CIVdtmsrf *srfP,            /* => surface with components           */
    int opt,                           /* => options                          */
    int (*usrfncP)(void *,             /* => your function                    */
        struct CIVdtmsrf *,
        struct CIVdtmcmp *),
    void *datP                         /* => your data                        */
);

void aecDTM_setComponentDeletedFlag
(
    struct CIVdtmcmp *cmpP              /* => component to use              */
);

int aecDTM_isComponentDeletedFlagSet      /* <= TRUE if set      */
(
    struct CIVdtmcmp  *cmpP               /* => component to check          */
);

int aecDTM_indexComponentsByParent
(
    struct CIVdtmsrf *srfP                  /*  => surface              */
);

int aecDTM_indexComponentsByGuid
(
    struct CIVdtmsrf *srfP                  /*  => surface              */
);

int aecDTM_indexComponentsByStyle
(
    struct CIVdtmsrf *srfP                  /*  => surface              */
);

int aecDTM_destroyComponentsParentIndex
(
    struct CIVdtmsrf *srfP                  /*  => surface              */
);

int aecDTM_destroyComponentsGuidIndex
(
    struct CIVdtmsrf *srfP                  /*  => surface              */
);

int aecDTM_destroyComponentsStyleIndex
(
    struct CIVdtmsrf *srfP                  /*  => surface              */
);
