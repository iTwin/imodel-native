//---------------------------------------------------------------------------+
// $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+
/*----------------------------------------------------------------------------*/
/* cmpmemuti.h                                           aec    26-Apr-2002   */
/*----------------------------------------------------------------------------*/
/* Function prototypes for DTM component member functions.                    */
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

int aecDTM_deleteComponentMember    /* <= TRUE if error                           */
(
    struct CIVdtmsrf *srfP,         /* => surface with feature (or NULL)          */
    struct CIVdtmcmpmem *cmpMemP    /* => component member to delete              */
);

int aecDTM_getComponentMembersPtrs
(
    struct CIVdtmcmpmem ***cmpMemPtrs,      /* <= FREE! - components members  */
    int *numCmpMemPtrs,                     /* <= number of component members */
    struct CIVdtmsrf *srfP,                 /*  => DTM surface (or NULL)      */
    struct CIVdtmcmp *cmpP                  /*  => component                  */
);

int aecDTM_getComponentMemberParentsPtrs
(
    struct CIVdtmcmp ***cmpPtrs,            /* <= FREE! - components          */
    int *numCmpPtrs,                        /* <= number of components        */
    struct CIVdtmsrf *srfP,                 /*  => DTM surface (or NULL)      */
    BeGuid *cmpMemGuidP                       /*  => guid of component member   */
);

void aecDTM_setComponentMemberDeletedFlag
(
    struct CIVdtmcmpmem *cmpMemP    /* => component member to use       */
);

int aecDTM_isComponentMemberDeletedFlagSet /* <= TRUE if set  */
(
    struct CIVdtmcmpmem  *cmpMemP     /* => component member to check    */
);

int aecDTM_sendAllComponentMembers     /* <= TRUE if error                    */
(
    void *mdlDescP,                    /* => mdl app desc. (or NULL)          */
    struct CIVdtmsrf *srfP,            /* => surface with component members   */
    int opt,                           /* => options                          */
    int (*usrfncP)(void *,             /* => your function                    */
        struct CIVdtmsrf *,
        struct CIVdtmcmpmem *),
    void *datP                         /* => your data                        */
);

int aecDTM_indexComponentMembersByGuid
(
    struct CIVdtmsrf *srfP                  /*  => surface              */
);

int aecDTM_indexComponentMembersByParent
(
    struct CIVdtmsrf *srfP                  /*  => surface              */
);

int aecDTM_destroyComponentMembersGuidIndex
(
    struct CIVdtmsrf *srfP                  /*  => surface              */
);

int aecDTM_destroyComponentMembersParentIndex
(
    struct CIVdtmsrf *srfP                  /*  => surface              */
);
