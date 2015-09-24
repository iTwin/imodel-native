//---------------------------------------------------------------------------+
// $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+
/*----------------------------------------------------------------------------*/
/* cmpmemuti.cpp                                twl    24-Apr-2002            */
/*----------------------------------------------------------------------------*/
/* Various utilities to add component members to surfaces and files.          */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* Include files                                                              */
/*----------------------------------------------------------------------------*/
#include "stdafx.h"
#include "cmpmemuti.h"

/*----------------------------------------------------------------------------*/
/* Local macros                                                               */
/*----------------------------------------------------------------------------*/
#define CMPMEM_IDXTYPE_PARENT  1

#define ALLOC_SIZE          100;

static CIVdtmcmpmem **ppLastCmpMem1 = NULL;
static CIVdtmcmpmem **ppLastCmpMem2 = NULL;

/*----------------------------------------------------------------------------*/
/* Local structures                                                           */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* Private static function prototypes                                         */
/*----------------------------------------------------------------------------*/
static int aecDTM_addToGuidIndexCallback ( void*, struct CIVdtmsrf*, struct CIVdtmcmpmem* );
static int aecDTM_addToParentIndexCallback ( void*, struct CIVdtmsrf*, struct CIVdtmcmpmem* );
static int aecDTM_insertComponentMemberIntoGuidIndex ( struct CIVdtmsrf*, struct CIVdtmcmpmem* );
static int aecDTM_insertComponentMemberIntoParentIndex ( struct CIVdtmsrf*, struct CIVdtmcmpmem* );
static int aecDTM_removeComponentMemberFromGuidIndex ( struct CIVdtmsrf *, struct CIVdtmcmpmem* );
static int aecDTM_removeComponentMemberFromParentIndex ( struct CIVdtmsrf *, struct CIVdtmcmpmem* );


/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_deleteComponentMember
 DESC: Deletes a component member from a surface.
 HIST: Original - twl 07-Mar-2005
 MISC:
 KEYW: DTM COMPONENT MEMBER DELETE
-----------------------------------------------------------------------------%*/

int aecDTM_deleteComponentMember    /* <= TRUE if error                           */
(
    struct CIVdtmsrf *srfP,         /* => surface with feature (or NULL)          */
    struct CIVdtmcmpmem *cmpMemP    /* => component member to delete              */
)
{
    int sts = SUCCESS;

    if ( !aecDTM_isComponentMemberDeletedFlagSet ( cmpMemP ) )
    {
        aecDTM_setComponentMemberDeletedFlag ( cmpMemP );
        srfP->cmpMemf->ndel++;

        if ( sts == SUCCESS )
        {
            aecDTM_removeComponentMemberFromGuidIndex ( srfP, cmpMemP );
            aecDTM_removeComponentMemberFromParentIndex ( srfP, cmpMemP );
        }

        aecDTM_setSurfaceModifiedFlag ( srfP );
    }

    return ( sts );
}

static int _sortCmpmemsByIdx( const void* a, const void *b )
{
    struct CIVdtmcmpmem *cmpmem1 = (CIVdtmcmpmem *)a;
    struct CIVdtmcmpmem *cmpmem2 = (CIVdtmcmpmem *)b;

	if (cmpmem1->index < cmpmem2->index)
		return -1;

	if ( cmpmem1->index > cmpmem2->index)
		return 1;

	return 0;
}


/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_getComponentMembersPtrs
 DESC: Returns a list of pointers to all component members that belong to a component.
 HIST: Original - twl 26-Jul-2002
 MISC:
 KEYW: DTM GET COMPONENT MEMBERS POINTERS
-----------------------------------------------------------------------------%*/

int aecDTM_getComponentMembersPtrs
(
    struct CIVdtmcmpmem ***cmpMemPtrs,      /* <= FREE! - components members  */
    int *numCmpMemPtrs,                     /* <= number of component members */
    struct CIVdtmsrf *srfP,                 /*  => DTM surface (or NULL)      */
    struct CIVdtmcmp *cmpP                  /*  => component                  */
)
{
    CPtrArray *ptrArrayP = NULL;
    wchar_t guidString[GUID_STRING_MAX_SIZE];
    int sts = SUCCESS;

    *cmpMemPtrs  = NULL;
    *numCmpMemPtrs = 0;

    if ( srfP->cmpMemParentMapP )
    {
        aecGuid_toString ( guidString, &cmpP->guid ); 

        if ( srfP->cmpMemParentMapP->Lookup ( guidString, (void*&)ptrArrayP ) && ptrArrayP && ptrArrayP->GetCount() > 0 )
        {
            *cmpMemPtrs = (struct CIVdtmcmpmem **) calloc ( ptrArrayP->GetCount(), sizeof ( struct CIVdtmcmpmem *) );
        
            for ( int i = 0; i < ptrArrayP->GetCount(); i++ )
            {
                if ( !aecDTM_isComponentMemberDeletedFlagSet ( (struct CIVdtmcmpmem *)ptrArrayP->GetAt ( i ) ) )
                {
                    (*cmpMemPtrs)[*numCmpMemPtrs] = (struct CIVdtmcmpmem *)ptrArrayP->GetAt ( i );
                    (*numCmpMemPtrs)++;
                }
            }
        }
    }

    return sts;
}


/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_getComponentMemberParentsPtrs
 DESC: Returns a list of pointers to all components that contain a component
       member with the specified guid.
 HIST: Original - twl 26-Jul-2002
 MISC:
 KEYW: DTM GET COMPONENT MEMBER PARENT POINTERS
-----------------------------------------------------------------------------%*/

int aecDTM_getComponentMemberParentsPtrs
(
    struct CIVdtmcmp ***cmpPtrs,            /* <= FREE! - components          */
    int *numCmpPtrs,                        /* <= number of components        */
    struct CIVdtmsrf *srfP,                 /*  => DTM surface (or NULL)      */
    BeGuid *cmpMemGuidP                       /*  => guid of component member   */
)
{
    CPtrArray *ptrArrayP = NULL;
    wchar_t guidString[GUID_STRING_MAX_SIZE];
    int sts = SUCCESS;

    *cmpPtrs  = NULL;
    *numCmpPtrs = 0;

    if ( srfP->cmpMemGuidMapP )
    {
        aecGuid_toString ( guidString, cmpMemGuidP ); 

        if ( srfP->cmpMemGuidMapP->Lookup ( guidString, (void*&)ptrArrayP ) && ptrArrayP && ptrArrayP->GetCount() > 0 )
        {
            *cmpPtrs = (struct CIVdtmcmp **) calloc ( ptrArrayP->GetCount(), sizeof ( struct CIVdtmcmp *) );
        
            for ( int i = 0; i < ptrArrayP->GetCount(); i++ )
            {
                struct CIVdtmcmpmem *pCmpMem = (struct CIVdtmcmpmem *)ptrArrayP->GetAt ( i );
                if ( !aecDTM_isComponentMemberDeletedFlagSet ( pCmpMem ) )
                {
                    (*cmpPtrs)[*numCmpPtrs] = aecDTM_findComponentByGuid ( srfP, &pCmpMem->cmpGuid );
                    (*numCmpPtrs)++;
                }
            }
        }
    }

    return sts;
}


/*%-----------------------------------------------------------------------
 FUNC: aecDTM_setComponentMemberDeletedFlag
 DESC: It sets the shape Deleted flag for a single shape.
 HIST: Original - twl 26-Apr-2002
 MISC:
 KEYW: DTM COMPONENT MEMBER FLAG DELETED SET
-----------------------------------------------------------------------%*/

void aecDTM_setComponentMemberDeletedFlag
(
    struct CIVdtmcmpmem *cmpMemP    /* => component member to use       */
)
{
    if ( cmpMemP != (struct CIVdtmcmpmem *)0 ) cmpMemP->flg |= DTM_C_CMPMEMDEL;
}


/*%-----------------------------------------------------------------------
 FUNC: aecDTM_isComponentMemberDeletedFlagSet
 DESC: Returns TRUE if input component member's deleted bit is set.
 HIST: Original - twl 26-Apr-2002
 MISC:
 KEYW: DTM COMPONENT MEMBER FLAG DELETED
-----------------------------------------------------------------------%*/

int aecDTM_isComponentMemberDeletedFlagSet /* <= TRUE if set  */
(
    struct CIVdtmcmpmem  *cmpMemP     /* => component member to check    */
)
{
    int sts = FALSE;
    if ( cmpMemP != (struct CIVdtmcmpmem *)0  &&  cmpMemP->flg & DTM_C_CMPMEMDEL ) sts = TRUE;
    return ( sts );
}


/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_sendAllComponentMembers
 DESC: Loops though all component members in a surface
 HIST: Original - twl 26-Apr-2002
 MISC:
 KEYW: DTM SHAPES SEND ALL
-----------------------------------------------------------------------------%*/

int aecDTM_sendAllComponentMembers     /* <= TRUE if error                    */
(
    void *mdlDescP,                    /* => mdl app desc. (or NULL)          */
    struct CIVdtmsrf *srfP,            /* => surface with component members   */
    int opt,                           /* => options                          */
    int (*usrfncP)(void *,             /* => your function                    */
        struct CIVdtmsrf *,
        struct CIVdtmcmpmem *),
    void *datP                         /* => your data                        */
)
{
    struct CIVdtmblk *blkP;
    struct CIVdtmcmpmem *cP;
    int sts = SUCCESS;

    for ( blkP = srfP->cmpMemf->blk; blkP  &&  sts == SUCCESS; blkP = blkP->nxt )
    {
        for ( cP = blkP->rec.cmpMem; cP < blkP->rec.cmpMem + blkP->use  &&  sts == SUCCESS; cP++ )
        {
            if ( !(opt & DTM_C_NOBREK)  &&  aecInterrupt_check() )
                sts = DTM_M_PRCINT;
            else if ( opt & DTM_C_DELETE  ||  !aecDTM_isComponentMemberDeletedFlagSet ( cP ) )
            {
                aecTicker_show();
                sts = (*usrfncP)( datP, srfP, cP);
            }
        }
    }

    return ( sts );
}


/*%-----------------------------------------------------------------------
 FUNC: aecDTM_indexComponentMembersByGuid
 DESC: Indexes component members by guid
 HIST: Original - twl 26-Apr-2002
 MISC:
 KEYW: DTM INDEX COMPONENT MEMBERS BY BeGuid
-----------------------------------------------------------------------%*/

int aecDTM_indexComponentMembersByGuid
(
    struct CIVdtmsrf *srfP                  /*  => surface              */
)
{
    int sts = SUCCESS;

    aecDTM_destroyComponentMembersGuidIndex ( srfP );

    if ( sts == SUCCESS )
    {
        srfP->cmpMemGuidMapP = new CMapStringToPtr;
        srfP->cmpMemGuidMapP->InitHashTable ( DTM_C_CMPMEMHSHSIZ );
        sts = aecDTM_sendAllComponentMembers ( (void *)0, srfP, DTM_C_NOBREK, aecDTM_addToGuidIndexCallback, NULL );
    }

    return sts;
}


/*%-----------------------------------------------------------------------
 FUNC: aecDTM_indexComponentMembersByParent
 DESC: Indexes component members by parent
 HIST: Original - twl 26-Apr-2002
 MISC:
 KEYW: DTM INDEX COMPONENT MEMBERS BY PARENT
-----------------------------------------------------------------------%*/

int aecDTM_indexComponentMembersByParent
(
    struct CIVdtmsrf *srfP                  /*  => surface              */
)
{
    int sts = SUCCESS;

    aecDTM_destroyComponentMembersParentIndex ( srfP );

    if ( sts == SUCCESS )
    {
        srfP->cmpMemParentMapP = new CMapStringToPtr;
        srfP->cmpMemParentMapP->InitHashTable ( DTM_C_CMPHSHSIZ );
        sts = aecDTM_sendAllComponentMembers ( (void *)0, srfP, DTM_C_NOBREK, aecDTM_addToParentIndexCallback, NULL );
    }

    return sts;
}


/*%-----------------------------------------------------------------------
 FUNC: aecDTM_destroyComponentMembersGuidIndex
 DESC: Destorys the guid index;
 HIST: Original - twl 26-Apr-2002
 MISC:
 KEYW: DTM DESTROY INDEX COMPONENT MEMBERS BeGuid
-----------------------------------------------------------------------%*/

int aecDTM_destroyComponentMembersGuidIndex
(
    struct CIVdtmsrf *srfP                  /*  => surface              */
)
{
    if ( srfP->cmpMemGuidMapP )
    {
        POSITION pos;
        for( pos = srfP->cmpMemGuidMapP->GetStartPosition(); pos != NULL; )
        {
            CPtrArray *ptrArrayP = NULL;
            CString key;
            srfP->cmpMemGuidMapP->GetNextAssoc( pos, key, (void*&)ptrArrayP );

            if ( ptrArrayP )
                delete ptrArrayP;
        }

        srfP->cmpMemGuidMapP->RemoveAll();
        delete srfP->cmpMemGuidMapP;
        srfP->cmpMemGuidMapP = NULL;
    }

    return SUCCESS;
}


/*%-----------------------------------------------------------------------
 FUNC: aecDTM_destroyComponentMembersParentIndex
 DESC: Destorys the parent index;
 HIST: Original - twl 26-Apr-2002
 MISC:
 KEYW: DTM DESTROY INDEX COMPONENT MEMBERS PARENT
-----------------------------------------------------------------------%*/

int aecDTM_destroyComponentMembersParentIndex
(
    struct CIVdtmsrf *srfP                  /*  => surface              */
)
{
    if ( srfP->cmpMemParentMapP )
    {
        POSITION pos;
        for( pos = srfP->cmpMemParentMapP->GetStartPosition(); pos != NULL; )
        {
            CPtrArray *ptrArrayP = NULL;
            CString key;
            srfP->cmpMemParentMapP->GetNextAssoc( pos, key, (void*&)ptrArrayP );

            if ( ptrArrayP )
                delete ptrArrayP;
        }

        srfP->cmpMemParentMapP->RemoveAll();
        delete srfP->cmpMemParentMapP;
        srfP->cmpMemParentMapP = NULL;
    }

    return SUCCESS;
}


/*%-----------------------------------------------------------------------
 FUNC: aecDTM_insertComponentMemberIntoGuidIndex
 DESC: Inserts a shape into the shape guid index.
 HIST: Original - twl 26-Apr-2002
 MISC:
 KEYW: DTM INSERT INDEX COMPONENT MEMBER BY BeGuid
-----------------------------------------------------------------------%*/

static int aecDTM_insertComponentMemberIntoGuidIndex
(
    struct CIVdtmsrf *srfP,                 /*  => surface              */
    struct CIVdtmcmpmem *cmpMemP            /*  => component member     */
)
{
    int sts;

    if ( !srfP->cmpMemParentMapP )
        sts = aecDTM_indexComponentMembersByGuid ( srfP );
    else
        sts = aecDTM_addToGuidIndexCallback ( NULL, srfP, cmpMemP );

    return sts;
}


/*%-----------------------------------------------------------------------
 FUNC: aecDTM_insertComponentMemberIntoParentIndex
 DESC: Inserts a shape into the shape name index.
 HIST: Original - twl 26-Apr-2002
 MISC:
 KEYW: DTM INSERT INDEX COMPONENT MEMBER BY NAME
-----------------------------------------------------------------------%*/

static int aecDTM_insertComponentMemberIntoParentIndex
(
    struct CIVdtmsrf *srfP,                 /*  => surface              */
    struct CIVdtmcmpmem *cmpMemP            /*  => component member     */
)
{
    int sts;

    if ( !srfP->cmpMemParentMapP )
        sts = aecDTM_indexComponentMembersByParent ( srfP );
    else
        sts = aecDTM_addToParentIndexCallback ( NULL, srfP, cmpMemP );

    return sts;
}


/*%-----------------------------------------------------------------------
 FUNC: aecDTM_removeComponentMemberFromGuidIndex
 DESC: Removes a component member from the guid index.
 HIST: Original - twl 26-Apr-2002
 MISC:
 KEYW: DTM REMOVE INDEX COMPONENT MEMBER BY BeGuid
-----------------------------------------------------------------------%*/

static int aecDTM_removeComponentMemberFromGuidIndex
(
    struct CIVdtmsrf *srfP,                 /*  => surface              */
    struct CIVdtmcmpmem *cmpMemP            /*  => component member     */
)
{
    int sts = SUCCESS;

    if ( srfP->cmpMemGuidMapP )
    {
        wchar_t guidString[GUID_STRING_MAX_SIZE];
        CPtrArray *ptrArrayP = NULL;
        BOOL found = FALSE;

        aecGuid_toString ( guidString, &cmpMemP->cmpMemGuid ); 
        srfP->cmpMemGuidMapP->Lookup ( guidString, (void*&)ptrArrayP );

        for ( int i = 0; i < ptrArrayP->GetCount( ) && !found; i++ )
        {
             struct CIVdtmcmpmem *tmpCmpMemP = (struct CIVdtmcmpmem *)ptrArrayP->GetAt ( i );

             if ( !aecGuid_compare ( &cmpMemP->cmpGuid, &tmpCmpMemP->cmpGuid ) )
             {
                ptrArrayP->RemoveAt ( i );
                found = TRUE;
             }
        }

        if ( ptrArrayP->GetCount() <= 0 )
        {
            delete ptrArrayP;
            srfP->cmpMemGuidMapP->RemoveKey ( guidString );
        }
    }

    return sts;
}


/*%-----------------------------------------------------------------------
 FUNC: aecDTM_removeComponentMemberFromParentIndex
 DESC: Removes a component member from the parent index.
 HIST: Original - twl 26-Apr-2002
 MISC:
 KEYW: DTM REMOVE INDEX COMPONENT MEMBER BY PARENT
-----------------------------------------------------------------------%*/

static int aecDTM_removeComponentMemberFromParentIndex
(
    struct CIVdtmsrf *srfP,                 /*  => surface              */
    struct CIVdtmcmpmem *cmpMemP            /*  => component member     */
)
{
    int sts = SUCCESS;

    if ( srfP->cmpMemParentMapP )
    {
        wchar_t guidString[GUID_STRING_MAX_SIZE];
        CPtrArray *ptrArrayP = NULL;
        BOOL found = FALSE;

        aecGuid_toString ( guidString, &cmpMemP->cmpGuid ); 
        srfP->cmpMemParentMapP->Lookup ( guidString, (void*&)ptrArrayP );

        for ( int i = 0; i < ptrArrayP->GetCount( ) && !found; i++ )
        {
             struct CIVdtmcmpmem *tmpCmpMemP = (struct CIVdtmcmpmem *)ptrArrayP->GetAt ( i );

             if ( !aecGuid_compare ( &cmpMemP->cmpMemGuid, &tmpCmpMemP->cmpMemGuid ) )
             {
                ptrArrayP->RemoveAt ( i );
                found = TRUE;
             }
        }

        if ( ptrArrayP->GetCount() <= 0 )
        {
            delete ptrArrayP;
            srfP->cmpMemParentMapP->RemoveKey ( guidString );
        }
    }

    return sts;
}

static int aecDTM_addToGuidIndexCallback
(
    void *dat,
    struct CIVdtmsrf *srfP,
    struct CIVdtmcmpmem *cmpMemP
)
{
    CPtrArray *ptrArrayP = NULL;
    wchar_t guidString[GUID_STRING_MAX_SIZE];

    aecGuid_toString ( guidString, &cmpMemP->cmpMemGuid ); 

    if ( !srfP->cmpMemGuidMapP->Lookup ( guidString, (void*&)ptrArrayP ) || !ptrArrayP )
    {
        ptrArrayP = new CPtrArray;
        srfP->cmpMemGuidMapP->SetAt ( guidString, ptrArrayP );
    }

    ptrArrayP->Add ( cmpMemP );

    return SUCCESS;
}

static int aecDTM_addToParentIndexCallback
(
    void *dat,
    struct CIVdtmsrf *srfP,
    struct CIVdtmcmpmem *cmpMemP
)
{
    CPtrArray *ptrArrayP = NULL;
    wchar_t guidString[GUID_STRING_MAX_SIZE];

    aecGuid_toString ( guidString, &cmpMemP->cmpGuid ); 

    if ( !srfP->cmpMemParentMapP->Lookup ( guidString, (void*&)ptrArrayP ) || !ptrArrayP )
    {
        ptrArrayP = new CPtrArray;
        srfP->cmpMemParentMapP->SetAt ( guidString, ptrArrayP );
    }

    ptrArrayP->Add ( cmpMemP );

    return SUCCESS;
}
