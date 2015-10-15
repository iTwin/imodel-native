//---------------------------------------------------------------------------+
// $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+
/*----------------------------------------------------------------------------*/
/* coruti.cpp                                   twl    24-Apr-2002            */
/*----------------------------------------------------------------------------*/
/* Corridor utilites                                                          */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* Include files                                                              */
/*----------------------------------------------------------------------------*/
#include "stdafx.h"
#include "coruti.h"
#include "cmputi.h"
#include "cmpmemuti.h"

/*----------------------------------------------------------------------------*/
/* Local macros                                                               */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* Local structures                                                           */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* Private static function prototypes                                         */
/*----------------------------------------------------------------------------*/
static int aecDTM_countCallback ( void*, struct CIVdtmsrf*, struct CIVdtmcor* );
static int aecDTM_addToGuidMapCallback ( void*, struct CIVdtmsrf*, struct CIVdtmcor* );
static int aecDTM_addToNameMapCallback ( void*, struct CIVdtmsrf*, struct CIVdtmcor* );
static int aecDTM_insertCorridorIntoGuidIndex ( struct CIVdtmsrf*, struct CIVdtmcor* );
static int aecDTM_insertCorridorIntoNameIndex ( struct CIVdtmsrf*, struct CIVdtmcor* );

/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_sendAllCorridors
 DESC: Loops though all corridors in a surface
 HIST: Original - twl 26-Apr-2002
 MISC:
 KEYW: DTM CORRIDORS SEND ALL
-----------------------------------------------------------------------------%*/

int aecDTM_sendAllCorridors /* <= TRUE if error                    */
(
    void *mdlDescP,                    /* => mdl app desc. (or NULL)          */
    struct CIVdtmsrf *srfP,            /* => surface with corridors           */
    int opt,                           /* => options                          */
    int (*usrfncP)(void *,             /* => your function                    */
        struct CIVdtmsrf *,
        struct CIVdtmcor *),
    void *datP                         /* => your data                        */
)
{
    struct CIVdtmblk *blkP;
    struct CIVdtmcor *cP;
    int sts = SUCCESS;

    for ( blkP = srfP->corf->blk; blkP  &&  sts == SUCCESS; blkP = blkP->nxt )
    {
        for ( cP = blkP->rec.cor; cP < blkP->rec.cor + blkP->use  &&  sts == SUCCESS; cP++ )
        {
            if ( !(opt & DTM_C_NOBREK)  &&  aecInterrupt_check() )
                sts = DTM_M_PRCINT;
            else if ( opt & DTM_C_DELETE  ||  !aecDTM_isCorridorDeletedFlagSet ( cP ) )
            {
                aecTicker_show();
                sts = (*usrfncP)( datP, srfP, cP);
            }
        }
    }

    return ( sts );
}

/*%-----------------------------------------------------------------------
 FUNC: aecDTM_isCorridorDeletedFlagSet
 DESC: Returns TRUE if input corridor's deleted bit is set.
 HIST: Original - twl 26-Apr-2002
 MISC:
 KEYW: DTM CORRIDOR FLAG DELETED
-----------------------------------------------------------------------%*/

int aecDTM_isCorridorDeletedFlagSet    /* <= TRUE if set                */
(
    struct CIVdtmcor  *corP            /* => corridor to check          */
)
{
    int sts = FALSE;
    if ( corP != (struct CIVdtmcor *)0  &&  corP->flg & DTM_C_CORDEL ) sts = TRUE;
    return ( sts );
}


/*%-----------------------------------------------------------------------
 FUNC: aecDTM_indexCorridorsByGuid
 DESC: Indexes corridors by guid
 HIST: Original - twl 26-Apr-2002
 MISC:
 KEYW: DTM INDEX CORRIDOR BY BeSQLite::BeGuid
-----------------------------------------------------------------------%*/

int aecDTM_indexCorridorsByGuid
(
    struct CIVdtmsrf *srfP                  /*  => surface              */
)
{
    int sts = SUCCESS;

    if ( srfP->corGuidMapP )
        delete srfP->corGuidMapP;

    if ( sts == SUCCESS )
    {
        srfP->corGuidMapP = new CMapStringToPtr;
        srfP->corGuidMapP->InitHashTable ( DTM_C_CORHSHSIZ );
        sts = aecDTM_sendAllCorridors ( (void *)0, srfP, DTM_C_NOBREK, aecDTM_addToGuidMapCallback, srfP->corGuidMapP );
    }

    return sts;
}


/*%-----------------------------------------------------------------------
 FUNC: aecDTM_indexCorridorsByName
 DESC: Indexes corridors by name
 HIST: Original - twl 26-Apr-2002
 MISC:
 KEYW: DTM INDEX CORRIDOR BY NAME
-----------------------------------------------------------------------%*/

int aecDTM_indexCorridorsByName
(
    struct CIVdtmsrf *srfP                  /*  => surface              */
)
{
    int sts = SUCCESS;

    if ( srfP->corNameMapP )
        delete srfP->corNameMapP;

    if ( sts == SUCCESS )
    {
        srfP->corNameMapP = new CMapStringToPtr;
        srfP->corNameMapP->InitHashTable ( DTM_C_CORHSHSIZ );
        sts = aecDTM_sendAllCorridors ( (void *)0, srfP, DTM_C_NOBREK, aecDTM_addToNameMapCallback, srfP->corNameMapP );
    }

    return sts;
}


/*%-----------------------------------------------------------------------
 FUNC: aecDTM_indexCorridorsComponentsMembers
 DESC: Indexes corridors, shapes, and components
 HIST: Original - twl 26-Apr-2002
 MISC:
 KEYW: DTM INDEX CORRIDORS SHAPES COMPONENTS
-----------------------------------------------------------------------%*/

int aecDTM_indexCorridorsComponentsMembers
(
    struct CIVdtmsrf *srfP                  /*  => surface              */
)
{
    int sts;

    if ( ( sts = aecDTM_indexCorridorsByGuid ( srfP ) ) == SUCCESS )
      if ( ( sts = aecDTM_indexCorridorsByName ( srfP ) ) == SUCCESS )
        if ( ( sts = aecDTM_indexComponentsByParent ( srfP ) ) == SUCCESS )
          if ( ( sts = aecDTM_indexComponentsByGuid ( srfP ) ) == SUCCESS )
            if ( ( sts = aecDTM_indexComponentsByStyle ( srfP ) ) == SUCCESS )
              if ( ( sts = aecDTM_indexComponentMembersByGuid ( srfP ) ) == SUCCESS )
                sts = aecDTM_indexComponentMembersByParent ( srfP );

    return sts;
}


/*%-----------------------------------------------------------------------
 FUNC: aecDTM_destroyCorridorsComponentsMembersIndexes
 DESC: Destorys corridors, shapes, and components indexes.
 HIST: Original - twl 26-Apr-2002
 MISC:
 KEYW: DTM INDEX CORRIDORS SHAPES COMPONENTS
-----------------------------------------------------------------------%*/

int aecDTM_destroyCorridorsComponentsMembersIndexes
(
    struct CIVdtmsrf *srfP                  /*  => surface              */
)
{
    int sts;

    if ( ( sts = aecDTM_destroyCorridorsGuidIndex ( srfP ) ) == SUCCESS )
      if ( ( sts = aecDTM_destroyCorridorsNameIndex ( srfP ) ) == SUCCESS )
        if ( ( sts = aecDTM_destroyComponentsParentIndex ( srfP ) ) == SUCCESS )
          if ( ( sts = aecDTM_destroyComponentsGuidIndex ( srfP ) ) == SUCCESS )
            if ( ( sts = aecDTM_destroyComponentsStyleIndex ( srfP ) ) == SUCCESS )
              if ( ( sts = aecDTM_destroyComponentMembersGuidIndex ( srfP ) ) == SUCCESS )
                sts = aecDTM_destroyComponentMembersParentIndex ( srfP );

    return sts;
}


/*%-----------------------------------------------------------------------
 FUNC: aecDTM_insertCorridorIntoGuidIndex
 DESC: Inserts a corridor into the corridor guid index.
 HIST: Original - twl 26-Apr-2002
 MISC:
 KEYW: DTM INSERT INDEX CORRIDOR BY BeSQLite::BeGuid
-----------------------------------------------------------------------%*/

static int aecDTM_insertCorridorIntoGuidIndex
(
    struct CIVdtmsrf *srfP,                 /*  => surface              */
    struct CIVdtmcor *corP                  /*  => corridor             */
)
{
    wchar_t guidString[GUID_STRING_MAX_SIZE];
    int sts = SUCCESS;

    if ( !srfP->corGuidMapP )
    {
        aecDTM_indexCorridorsByGuid ( srfP );
    }
    else
    {
        aecGuid_toString  ( guidString, &corP->guid );
        srfP->corGuidMapP->SetAt ( guidString, corP );
    }
    return sts;
}


/*%-----------------------------------------------------------------------
 FUNC: aecDTM_insertCorridorIntoNameIndex
 DESC: Inserts a corridor into the corridor name index.
 HIST: Original - twl 26-Apr-2002
 MISC:
 KEYW: DTM INSERT INDEX CORRIDOR BY NAME
-----------------------------------------------------------------------%*/

static int aecDTM_insertCorridorIntoNameIndex
(
    struct CIVdtmsrf *srfP,                 /*  => surface              */
    struct CIVdtmcor *corP                  /*  => corridor             */
)
{
    int sts = SUCCESS;

    if ( !srfP->corNameMapP )
    {
        aecDTM_indexCorridorsByName ( srfP );
    }
    else
    {
        srfP->corNameMapP->SetAt ( corP->nam, corP );
    }

    return sts;
}


/*%-----------------------------------------------------------------------
 FUNC: aecDTM_destroyCorridorsGuidIndex
 DESC: Destorys the corridors guid index;
 HIST: Original - twl 26-Apr-2002
 MISC:
 KEYW: DTM DESTROY INDEX CORRIDOR BeSQLite::BeGuid
-----------------------------------------------------------------------%*/

int aecDTM_destroyCorridorsGuidIndex
(
    struct CIVdtmsrf *srfP                  /*  => surface              */
)
{
    if ( srfP->corGuidMapP )
    {
        srfP->corGuidMapP->RemoveAll();
        delete srfP->corGuidMapP;
        srfP->corGuidMapP = NULL;
    }

    return SUCCESS;
}


/*%-----------------------------------------------------------------------
 FUNC: aecDTM_destroyCorridorsNameIndex
 DESC: Destorys the corridors name index;
 HIST: Original - twl 26-Apr-2002
 MISC:
 KEYW: DTM DESTROY INDEX CORRIDOR NAME
-----------------------------------------------------------------------%*/

int aecDTM_destroyCorridorsNameIndex
(
    struct CIVdtmsrf *srfP                  /*  => surface              */
)
{
    if ( srfP->corNameMapP )
    {
        srfP->corNameMapP->RemoveAll();
        delete srfP->corNameMapP;
        srfP->corNameMapP = NULL;
    }

    return SUCCESS;
}



static int aecDTM_countCallback
(
    void *dat,
    struct CIVdtmsrf *srfP,
    struct CIVdtmcor *corP
)
{
    unsigned int *countP = ( unsigned int * ) dat;

    (*countP)++;

    return SUCCESS;
}

static int aecDTM_addToGuidMapCallback
(
    void *dat,
    struct CIVdtmsrf *srfP,
    struct CIVdtmcor *corP
)
{
    CMapStringToPtr *mapP = ( CMapStringToPtr * ) dat;
    wchar_t guidString[GUID_STRING_MAX_SIZE];

    aecGuid_toString  ( guidString, &corP->guid );
    mapP->SetAt ( guidString, corP );

    return SUCCESS;
}

static int aecDTM_addToNameMapCallback
(
    void *dat,
    struct CIVdtmsrf *srfP,
    struct CIVdtmcor *corP
)
{
    CMapStringToPtr *mapP = ( CMapStringToPtr * ) dat;
    mapP->SetAt ( corP->nam, corP );

    return SUCCESS;
}
