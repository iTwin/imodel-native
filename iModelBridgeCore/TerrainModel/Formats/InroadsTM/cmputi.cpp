//---------------------------------------------------------------------------+
// $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+
/*----------------------------------------------------------------------------*/
/* cmputi.cpp                                   twl    24-Apr-2002            */
/*----------------------------------------------------------------------------*/
/* Various utilities to add components to surfaces and files.                 */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* Include files                                                              */
/*----------------------------------------------------------------------------*/
#include "stdafx.h"
#include "cmputi.h"
#include "cmpmemuti.h"

/*----------------------------------------------------------------------------*/
/* Local macros                                                               */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* Local structures                                                           */
/*----------------------------------------------------------------------------*/
typedef struct
{
    wchar_t name[DTM_C_NAMSIZ];
    struct CIVdtmsrf *srfP;
    BeGuid guid;
} InExItem;

typedef struct
{
    struct CIVdtmsrf    *srfP;
    std::vector<InExItem> *inExNames;
    BOOL                includeNames;
    ComponentListItem   *listP;
    long                count;
    long                alc;
    int                 nStyleFlag;
    int                 (*pStyleFilterFunc)(BeGuid *, struct CIVdtmsrf *, int, BOOL *);
} ComponentListDat;

typedef struct
{
    struct CIVdtmsrf *dstSrfP;
    CMapStringToString *ftrGuidMapP;
    BeGuid oldCorGuid;
    BeGuid newCorGuid;
    BOOL bOneCmpAdded;
} TransferCompsDat;

/*----------------------------------------------------------------------------*/
/* Private static function prototypes                                         */
/*----------------------------------------------------------------------------*/
static int aecDTM_addToGuidMapCallback ( void *, struct CIVdtmsrf *, struct CIVdtmcmp *);
static int aecDTM_insertComponentIntoParentIndex ( struct CIVdtmsrf*, struct CIVdtmcmp* );
static int aecDTM_insertComponentIntoGuidIndex ( struct CIVdtmsrf*, struct CIVdtmcmp* );
static int aecDTM_insertComponentIntoStyleIndex ( struct CIVdtmsrf*, struct CIVdtmcmp* );
static int aecDTM_removeComponentfromGuidIndex ( struct CIVdtmsrf *, struct CIVdtmcmp *);
static int aecComponent_DoesComponentPassNameList ( struct CIVdtmcmp *, ComponentListDat *dat );
static int compareInExItems( const void *, const void * );

/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_findComponentByGuid
 DESC: Given a component BeGuid, this function returns a pointer to the
       corresponding component.
 HIST: Original - twl 21-Jul-2002
 MISC:
 KEYW: DTM FIND COMPONENT BY BeGuid
-----------------------------------------------------------------------------%*/

struct CIVdtmcmp *aecDTM_findComponentByGuid /* <= pointer to component       */
(
    struct CIVdtmsrf *srfP,                 /* => DTM surface (or NULL)       */
    BeGuid *guidP                             /* => component guid pointer      */
)
{
    struct CIVdtmcmp *cmpP = NULL;
    wchar_t guidString[GUID_STRING_MAX_SIZE];

    if ( srfP->cmpGuidMapP )
    {
        aecGuid_toString ( guidString, guidP );        
        srfP->cmpGuidMapP->Lookup ( guidString, (void *&)cmpP );
    }

    return ( cmpP );
}

/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_deleteComponent
 DESC: Gets component properties
 HIST: Original - twl 07-Mar-2005
 MISC:
 KEYW: DTM DELETE COMPONENT
-----------------------------------------------------------------------------%*/

int aecDTM_deleteComponent /* <= TRUE if error                         */
(
    struct CIVdtmsrf *srfP,         /* => surface with feature (or NULL)          */
    struct CIVdtmcmp *cmpP          /* => feature to delete                       */
)
{
    int sts = SUCCESS;

    if ( !aecDTM_isComponentDeletedFlagSet ( cmpP ) )
    {
        struct CIVdtmcmpmem **cmpMemsPP = NULL;
        int numCmpMems = 0;

        aecDTM_getComponentMembersPtrs ( &cmpMemsPP, &numCmpMems, srfP, cmpP );

        for ( int i = 0; i < numCmpMems; i++ )
            aecDTM_deleteComponentMember ( srfP, cmpMemsPP[i] );

        if ( cmpMemsPP )
            free ( cmpMemsPP );

        aecDTM_setComponentDeletedFlag ( cmpP );
        srfP->cmpf->ndel++;

        if ( sts == SUCCESS )
        {
            aecDTM_removeComponentfromGuidIndex ( srfP, cmpP );
        }

        aecDTM_setSurfaceModifiedFlag ( srfP );
    }

    return ( sts );
}


/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_sendAllComponents
 DESC: Loops though all components in a surface
 HIST: Original - twl 26-Apr-2002
 MISC:
 KEYW: DTM COMPONENTS SEND ALL
-----------------------------------------------------------------------------%*/

int aecDTM_sendAllComponents           /* <= TRUE if error                    */
(
    void *mdlDescP,                    /* => mdl app desc. (or NULL)          */
    struct CIVdtmsrf *srfP,            /* => surface with components          */
    int opt,                           /* => options                          */
    int (*usrfncP)(void *,             /* => your function                    */
        struct CIVdtmsrf *,
        struct CIVdtmcmp *),
    void *datP                         /* => your data                        */
)
{
    struct CIVdtmblk *blkP;
    struct CIVdtmcmp *cP;
    int sts = SUCCESS;

    for ( blkP = srfP->cmpf->blk; blkP  &&  sts == SUCCESS; blkP = blkP->nxt )
    {
        for ( cP = blkP->rec.cmp; cP < blkP->rec.cmp + blkP->use  &&  sts == SUCCESS; cP++ )
        {
            if ( !(opt & DTM_C_NOBREK)  &&  aecInterrupt_check() )
                sts = DTM_M_PRCINT;
            else if ( opt & DTM_C_DELETE  ||  !aecDTM_isComponentDeletedFlagSet ( cP ) )
            {
                //aecTicker_show();
                sts = (*usrfncP)( datP, srfP, cP);
            }
        }
    }

    return ( sts );
}


/*%-----------------------------------------------------------------------
 FUNC: aecDTM_setComponentDeletedFlag
 DESC: It sets the component Deleted flag for a single component.
 HIST: Original - twl 26-Apr-2002
 MISC:
 KEYW: DTM COMPONENT FLAG DELETED SET
-----------------------------------------------------------------------%*/

void aecDTM_setComponentDeletedFlag
(
    struct CIVdtmcmp *cmpP              /* => component to use          */
)
{
    if ( cmpP != (struct CIVdtmcmp *)0 ) cmpP->flg |= DTM_C_CMPDEL;
}


/*%-----------------------------------------------------------------------
 FUNC: aecDTM_isComponentDeletedFlagSet
 DESC: Returns TRUE if input component's deleted bit is set.
 HIST: Original - twl 26-Apr-2002
 MISC:
 KEYW: DTM COMPONENT FLAG DELETED
-----------------------------------------------------------------------%*/

int aecDTM_isComponentDeletedFlagSet      /* <= TRUE if set             */
(
    struct CIVdtmcmp  *cmpP               /* => component to check      */
)
{
    int sts = FALSE;
    if ( cmpP != (struct CIVdtmcmp *)0  &&  cmpP->flg & DTM_C_CMPDEL ) sts = TRUE;
    return ( sts );
}

/*%-----------------------------------------------------------------------
 FUNC: aecDTM_indexComponentsByParent
 DESC: Indexes components by parent
 HIST: Original - twl 26-Apr-2002
 MISC:
 KEYW: DTM INDEX COMPONENTS BY PARENT
-----------------------------------------------------------------------%*/

int aecDTM_indexComponentsByParent
(
    struct CIVdtmsrf *srfP                  /*  => surface              */
)
{
    int sts = SUCCESS;
    return sts;
}


/*%-----------------------------------------------------------------------
 FUNC: aecDTM_indexComponentsByGuid
 DESC: Indexes components by guid
 HIST: Original - twl 26-Apr-2002
 MISC:
 KEYW: DTM INDEX COMPONENTS BY BeGuid
-----------------------------------------------------------------------%*/

int aecDTM_indexComponentsByGuid
(
    struct CIVdtmsrf *srfP                  /*  => surface              */
)
{
    int sts = SUCCESS;

    if ( srfP->cmpGuidMapP )
        delete srfP->cmpGuidMapP;

    if ( sts == SUCCESS )
    {
        srfP->cmpGuidMapP = new CMapStringToPtr;
        srfP->cmpGuidMapP->InitHashTable ( DTM_C_CMPHSHSIZ );
        sts = aecDTM_sendAllComponents ( (void *)0, srfP, DTM_C_NOBREK, aecDTM_addToGuidMapCallback, srfP->cmpGuidMapP );
    }

    return sts;
}


/*%-----------------------------------------------------------------------
 FUNC: aecDTM_indexComponentsByStyle
 DESC: Indexes components by style
 HIST: Original - twl 26-Apr-2002
 MISC:
 KEYW: DTM INDEX COMPONENTS BY STYLE
-----------------------------------------------------------------------%*/

int aecDTM_indexComponentsByStyle
(
    struct CIVdtmsrf *srfP                  /*  => surface              */
)
{
    return SUCCESS;
}


/*%-----------------------------------------------------------------------
 FUNC: aecDTM_destroyComponentsParentIndex
 DESC: Destorys the parent index;
 HIST: Original - twl 26-Apr-2002
 MISC:
 KEYW: DTM DESTROY INDEX COMPONENT PARENT
-----------------------------------------------------------------------%*/

int aecDTM_destroyComponentsParentIndex
(
    struct CIVdtmsrf *srfP                  /*  => surface              */
)
{
    return SUCCESS;
}


/*%-----------------------------------------------------------------------
 FUNC: aecDTM_destroyComponentsGuidIndex
 DESC: Destorys the components guid index;
 HIST: Original - twl 26-Apr-2002
 MISC:
 KEYW: DTM DESTROY INDEX COMPONENT BeGuid
-----------------------------------------------------------------------%*/

int aecDTM_destroyComponentsGuidIndex
(
    struct CIVdtmsrf *srfP                  /*  => surface              */
)
{
    if ( srfP->cmpGuidMapP )
    {
        srfP->cmpGuidMapP->RemoveAll();
        delete srfP->cmpGuidMapP;
        srfP->cmpGuidMapP = NULL;
    }

    return SUCCESS;
}


/*%-----------------------------------------------------------------------
 FUNC: aecDTM_destroyComponentsStyleIndex
 DESC: Destorys the components style index;
 HIST: Original - twl 26-Apr-2002
 MISC:
 KEYW: DTM DESTROY INDEX COMPONENT STYLE
-----------------------------------------------------------------------%*/

int aecDTM_destroyComponentsStyleIndex
(
    struct CIVdtmsrf *srfP                  /*  => surface              */
)
{
    return SUCCESS;
}


/*%-----------------------------------------------------------------------
 FUNC: aecDTM_insertComponentIntoParentIndex
 DESC: Inserts a component into the component name index.
 HIST: Original - twl 26-Apr-2002
 MISC:
 KEYW: DTM INSERT INDEX COMPONENT BY NAME
-----------------------------------------------------------------------%*/

static int aecDTM_insertComponentIntoParentIndex
(
    struct CIVdtmsrf *srfP,                 /*  => surface              */
    struct CIVdtmcmp *cmpP                  /*  => component            */
)
{
    return SUCCESS;
}


/*%-----------------------------------------------------------------------
 FUNC: aecDTM_insertComponentIntoGuidIndex
 DESC: Inserts a component into the component guid index.
 HIST: Original - twl 26-Apr-2002
 MISC:
 KEYW: DTM INSERT INDEX COMPONENT BY BeGuid
-----------------------------------------------------------------------%*/

static int aecDTM_insertComponentIntoGuidIndex
(
    struct CIVdtmsrf *srfP,                 /*  => surface              */
    struct CIVdtmcmp *cmpP                  /*  => component            */
)
{
    wchar_t guidString[GUID_STRING_MAX_SIZE];
    int sts = SUCCESS;

    if ( !srfP->cmpGuidMapP )
    {
        aecDTM_indexComponentsByGuid ( srfP );
    }
    else
    {
        aecGuid_toString  ( guidString, &cmpP->guid );
        srfP->cmpGuidMapP->SetAt ( guidString, cmpP );
    }
    return sts;
}


/*%-----------------------------------------------------------------------
 FUNC: aecDTM_insertComponentIntoStyleIndex
 DESC: Inserts a component into the component style index.
 HIST: Original - twl 26-Apr-2002
 MISC:
 KEYW: DTM INSERT INDEX COMPONENT BY STYLE
-----------------------------------------------------------------------%*/

static int aecDTM_insertComponentIntoStyleIndex
(
    struct CIVdtmsrf *srfP,                 /*  => surface              */
    struct CIVdtmcmp *cmpP                  /*  => component            */
)
{
    return SUCCESS;
}


/*%-----------------------------------------------------------------------
 FUNC: aecDTM_removeComponentfromGuidIndex
 DESC: Removes a component from the component guid index.
 HIST: Original - twl 26-Apr-2002
 MISC:
 KEYW: DTM REMOVE INDEX COMPONENT BY BeGuid
-----------------------------------------------------------------------%*/

static int aecDTM_removeComponentfromGuidIndex
(
    struct CIVdtmsrf *srfP,                 /*  => surface              */
    struct CIVdtmcmp *cmpP                  /*  => component            */
)
{
    wchar_t guidString[GUID_STRING_MAX_SIZE];
    int sts = SUCCESS;

    aecGuid_toString  ( guidString, &cmpP->guid );

    if ( srfP->cmpGuidMapP )
        srfP->cmpGuidMapP->RemoveKey ( guidString );

    return sts;
}

static int aecDTM_addToGuidMapCallback
(
    void *dat,
    struct CIVdtmsrf *srfP,
    struct CIVdtmcmp *cmpP
)
{
    CMapStringToPtr *mapP = ( CMapStringToPtr * ) dat;
    wchar_t guidString[GUID_STRING_MAX_SIZE];

    aecGuid_toString  ( guidString, &cmpP->guid );
    mapP->SetAt ( guidString, cmpP );

    return SUCCESS;
}

static int aecComponent_DoesComponentPassNameList
(
    struct CIVdtmcmp *cmpP,
    ComponentListDat *dat
)
{
    InExItem inExName;
    BOOL bFound = FALSE;

    memset ( &inExName, 0, sizeof ( inExName ) );
    memcpy ( inExName.name, cmpP->nam, sizeof ( inExName.name ) );
    inExName.srfP = dat->srfP;

    if ( bsearch ( &inExName, &(*dat->inExNames)[0], dat->inExNames->size(), sizeof ( InExItem ), compareInExItems ) )
        bFound = TRUE;

    if ( !dat->includeNames )
        bFound = !bFound;        

    return ( (int)bFound );    
}

static int compareInExItems( const void *pParm1, const void *pParm2 )
{
    InExItem *item1 = (InExItem *)pParm1;
    InExItem *item2 = (InExItem *)pParm2;

    int sts = 0;
    
    if (item1 && item1->srfP && item2 && item2->srfP )
    {
        sts = aecGuid_compare ( &item1->srfP->guid, &item2->srfP->guid );

        if ( sts == 0 )
            sts = wcscmp ( item1->name, item2->name );
    }
    return sts;
}

