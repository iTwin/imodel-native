/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/dependency/DependencyManagerLinkage.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

#define RETURN_IF_NOT_SUCCESS(s)        if (SUCCESS != (s)) {return s;}
#define RETURN_IF_INVALID_DTYPE(d,e)    if ((d) < 0 || DEPENDENCY_DATA_TYPE_MAX < (d)) {BeAssert (false); return e;}
#define EXTRACT_LINKAGE(p)              ((DependencyLinkage *) ((LinkageHeader *)(p) + 1))
#define DEPENDENCY_HEADER_SIZE          offsetof (DependencyLinkage, root)

extern int                                      dependencyLinkageSize (DependencyLinkage const*);
extern DependencyManagerLinkage::RootGetFunc    g_getRoots[DEPENDENCY_DATA_TYPE_MAX+1];

/*----------------------------------------------------------------------+
|                                                                       |
|   Local type definitions                                              |
|                                                                       |
+----------------------------------------------------------------------*/
struct LinkageMatchParms /* (don't need an PendingChanges for this) */
    {
    DgnElementCP     pEl;        // element to examine
    UShort          appID;      // appID to match
    UShort          appValue;   // appValue value to match
    int             nFound;     // # found
    bool            findFirst;
    LinkageHeader const*  linkage;     // <= first found

    LinkageMatchParms (DgnElementCP pEl, UShort appID, UShort appValue, bool findFirst = false)
        { this->pEl = pEl; this->appID = appID; this->appValue = appValue; this->nFound = 0; this->findFirst = findFirst; linkage = NULL;}
    };

struct LinkageUpdateParms
:       LinkageMatchParms
    {
    DependencyLinkage const *pNew;
    StatusInt           result;
    int                 xtrabytes;

    LinkageUpdateParms (DgnElement*pEl, DependencyLinkage const *pNew, int xtrabytes, bool findFirst = false)
        : LinkageMatchParms (pEl, pNew->appID, pNew->appValue, findFirst)
        { this->pNew = pNew; this->result = ERROR; this->xtrabytes = xtrabytes; }
    };

/*----------------------------------------------------------------------+
|                                                                       |
|   Private Global variables                                            |
|                                                                       |
+----------------------------------------------------------------------*/

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/2006
+---------------+---------------+---------------+---------------+---------------+------*/
bool     deplink_hasLinkage (DgnElementCP pEl)
    {
    DependencyLinkageIterator il (pEl);
    return il.IsValid ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/2007
+---------------+---------------+---------------+---------------+---------------+------*/
SemiPrivate bool    deplink_parentHasLinkageOrDependents (ElementRefP child, bool findMovedParents)
    {
#ifdef DGNPROJECT_MODELS_CHANGES_WIP
    ElementRefP parent;
    while ((parent = getLiveParent (child, findMovedParents)) != NULL)
        {
        if (elementRef_hasDependents (parent))
            return true;

        ElementHandle eh (parent, NULL);
        if (deplink_hasLinkage (eh.GetElementCP()))
            return true;

        child = parent;
        }
#endif

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      09/2008
+---------------+---------------+---------------+---------------+---------------+------*/
static int processLinkage_detectMatching
(
LinkageHeader const* pHeader,
void*                pParmsPtr
)
    {
    if (!pHeader->user || LINKAGEID_Dependency != pHeader->primaryID)
        return 0;

    DependencyLinkage *pDep = EXTRACT_LINKAGE (pHeader);
    LinkageMatchParms *pParms = (LinkageMatchParms*) (pParmsPtr);

    if (pDep->appID != pParms->appID || pDep->appValue != pParms->appValue)
        return 0;
        
    pParms->linkage = pHeader;
    ++pParms->nFound;
    return 1;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      09/2008
+---------------+---------------+---------------+---------------+---------------+------*/
static int    processLinkage_deleteMatching
(
LinkageHeader   *pOutLink,
void            *pParmsPtr,
LinkageHeader   *pInLink,
DgnElementP      pElem
)
    {
    LinkageMatchParms *pParms = (LinkageMatchParms*) (pParmsPtr);
    
    if (0 == processLinkage_detectMatching (pInLink, pParms))
        return PROCESS_ATTRIB_STATUS_NOCHANGE;

    int r = PROCESS_ATTRIB_STATUS_DELETE;

    if (pParms->findFirst)
        r |= PROCESS_ATTRIB_STATUS_ABORT;

    return r;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          processLinkage_updateInPlace                            |
|                                                                       |
| author        BrienBastings                           08/01           |
|                                                                       |
+----------------------------------------------------------------------*/
static int     processLinkage_updateInPlace
(
LinkageHeader       *outLinkP,      /* <=  replacement linkage */
void                *pParms,        /*  => remap parameters    */
LinkageHeader       *inLinkP,       /* <=> linkage to process  */
DgnElementP           elemP          /* <=> owner element       */
)
    {
    LinkageUpdateParms  *pUpdate = (LinkageUpdateParms *) pParms;

    if (!inLinkP->user || LINKAGEID_Dependency != inLinkP->primaryID)
        return PROCESS_ATTRIB_STATUS_NOCHANGE;

    if (!processLinkage_detectMatching (inLinkP, pUpdate))
        return PROCESS_ATTRIB_STATUS_NOCHANGE;

    DependencyLinkage const *pOld = EXTRACT_LINKAGE (inLinkP);    // => converts to native
    DependencyLinkage const *pNew = pUpdate->pNew;

    int oldSize = dependencyLinkageSize (pOld) + pUpdate->xtrabytes;
    int newSize = dependencyLinkageSize (pNew) + pUpdate->xtrabytes;

    if (oldSize != newSize)
        {
        pUpdate->result = ERROR;

        return PROCESS_ATTRIB_STATUS_ABORT;
        }
    else
        {
        memcpy (outLinkP, inLinkP, sizeof(LinkageHeader));
        memcpy (outLinkP+1, pNew, oldSize);
        pUpdate->result = SUCCESS;

        return PROCESS_ATTRIB_STATUS_REPLACE | PROCESS_ATTRIB_STATUS_ABORT;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2007
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt processDependencyRoot
(
T_StdElementRefSet*     rootsOut,
DependencyRoot*         roots,
DependencyManagerLinkage::ProcessRootsOfProcessor proc,
void*                   arg2,
DependencyLinkage const* dep,
unsigned                n
)
    {
    for (unsigned i=0; i<n; ++i)
        {
        if (NULL != roots[i].ref)
            {
            if (NULL != proc)
                {
                StatusInt status = proc (&roots[i], dep, arg2);
/*<*/           RETURN_IF_NOT_SUCCESS(status);
                }

            if (NULL != rootsOut)
                rootsOut->insert (roots[i].ref);
            }
        }
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SamWilson       07/02
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt       processRootsOfHelper
(
T_StdElementRefSet*     rootsOut,
DependencyManagerLinkage::ProcessRootsOfProcessor proc,
void*                   arg2,
DgnModelP               cache,
DependencyLinkage const* dep
)
    {
    unsigned    dtype  = dep->u.f.rootDataType;
    unsigned    nRoots = dep->nRoots;

    RETURN_IF_INVALID_DTYPE (dtype, ERROR);

    if (dep->u.f.invalid)   // linkage is invalid?
        return SUCCESS;     //  element IDs are bogus, so don't even try to resolve elementrefs!

    for (UInt16 iRoot=0; iRoot<nRoots; ++iRoot)
        {
        DependencyRoot  roots[DEPENDENCY_MAX_TARGETS_PER_ROOT];
        UInt16        n = g_getRoots[dtype] (roots, cache, *dep, iRoot);
        StatusInt status = processDependencyRoot (rootsOut, roots, proc, arg2, dep, n);
/*<*/   RETURN_IF_NOT_SUCCESS(status);
        }

    return  SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SamWilson       07/02
+---------------+---------------+---------------+---------------+---------------+------*/
void            DependencyManagerLinkage::ProcessRootsOf
(
ElementRefP              ref,
DgnElementCP             el,
DependencyManagerLinkage::ProcessRootsOfProcessor proc,
void*                   arg2,
T_StdElementRefSet*   roots
)
    {
    ElementHandle eh (ref);
    if (NULL==el)
        el = eh.GetElementCP ();

    for (DependencyLinkageIterator il (el); il.IsValid(); il.ToNext())
        {
        if (processRootsOfHelper (roots, proc, arg2, ref->GetDgnModelP(), il.GetLinkage ()) != SUCCESS)
            break;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      09/2008
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   DependencyManagerLinkage::InitLinkage
(                               
DependencyLinkage& linkage,    
UShort             appID,       
int                dataType,    
int                copyOptions  
)
    {
//    RETURN_IF_INVALID_DTYPE (dataType, ERROR); *** can't assume that caller has set up type yet

/*
    if (appID < DEPENDENCY_APPID_RESERVED)
        return ERROR;
*/

    linkage.appID         = appID;
    linkage.appValue      = 0;
    linkage.u.flags       = 0;
    linkage.u.f.rootDataType = dataType;
    linkage.u.f.copyOptions = copyOptions;
    linkage.nRoots        = 0;
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/09
+---------------+---------------+---------------+---------------+---------------+------*/
void    DependencyManagerLinkage::DefineElementIDDependency
(
DependencyLinkage&  depLinkage,
UShort              appID,
UShort              appValue,
int                 copyOptions,
UInt64           elementID
)
    {
    DependencyManagerLinkage::InitLinkage (depLinkage, appID, DEPENDENCY_DATA_TYPE_ELEM_ID, copyOptions);

    depLinkage.appValue         = appValue;
    depLinkage.nRoots           = 1;
    depLinkage.root.elemid[0]   = elementID;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    12/08
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt DependencyManagerLinkage::AppendSimpleLinkageToMSElement
(                               
DgnElementP          elemP,
UShort              appId,
UShort              appValue,
int                 copyOption,
UInt64           elementID
)
    {
    DependencyLinkage   depLinkage;

    DefineElementIDDependency (depLinkage, appId, appValue, copyOption, elementID);

    /* Only pass fileObj now if uniqueId is valid, others setup dependency in post-process */
    return DependencyManagerLinkage::AppendLinkageToMSElement (elemP, depLinkage, 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      09/2008
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt DependencyManagerLinkage::AppendLinkageToMSElement (DgnElementP el, DependencyLinkage const& dependencyData, size_t xtrabytes)
    {
    RETURN_IF_INVALID_DTYPE (dependencyData.u.f.rootDataType, ERROR);

    //  Compute size of linkage, including header and dependencyData
    size_t fullsize = dependencyLinkageSize (&dependencyData) + xtrabytes;

    if (fullsize > DEPENDENCY_HEADER_SIZE + DEPENDENCY_MAX_DATA)
        {
        #if defined (ENABLE_TRACING)
            if (s_trace)
                {
                toolSubsystem_fwprintf (g_traceFP, L"           ***ERROR DependencyManagerLinkage::AppendLinkageEx failed -- size = %d bytes\n", fullsize);
                }
        #endif
/*<=*/  return  ERROR;
        }

    size_t linkagesize  = fullsize + sizeof(LinkageHeader);      // in bytes
    size_t rawlinkagesize = linkagesize;                        //  "   "
    linkagesize         = (linkagesize + 7) & ~7;   // pad to 8-byte multiple

    //  Allocate header + data
    LinkageHeader* hdr = (LinkageHeader*) _alloca (linkagesize);

    //  Set up header
    memset (hdr, 0, sizeof (*hdr));
    hdr->primaryID       = LINKAGEID_Dependency;
    hdr->user            = true;

    if (LinkageUtil::SetWords (hdr, static_cast<int>(linkagesize/2)) != SUCCESS)
        {
        #if defined (ENABLE_TRACING)
            if (s_trace)
                {
                toolSubsystem_fwprintf (g_traceFP, L"           ***ERROR DependencyManagerLinkage::AppendLinkageEx: LinkageUtil::SetWords failed -- linkageWords=%d\n", linkagesize/2);
                }
        #endif

/*<=*/  return ERROR;   // (probably too big)
        }

    //  Copy dependency data into place
    memcpy (EXTRACT_LINKAGE(hdr), (void*)&dependencyData, fullsize);

    EXTRACT_LINKAGE(hdr)->u.f.__unused__ = 0;

    size_t padding = linkagesize - rawlinkagesize;
    memset ((char*)hdr + rawlinkagesize, 0, padding);

    BeAssert (2*LinkageUtil::GetWords (hdr) == DependencyManagerLinkage::GetSizeofLinkage (dependencyData, xtrabytes)); // check that this function and DependencyManagerLinkage::GetSizeofLinkage are consistent

    //  Append to element
    StatusInt s = elemUtil_appendLinkage (el, hdr);

    #if defined (ENABLE_TRACING)
        if (s_trace)
            {
            if (s != SUCCESS)
                toolSubsystem_fwprintf (g_traceFP, L"        ***ERROR DependencyManagerLinkage::AppendLinkage: mdlLinkage_appendToElement failed\n");
            else if (s_trace>1)
                {
                WString buf = DependencyManagerLinkage::FmtRoot (NULL, dependencyData, 0);
                toolSubsystem_fwprintf (g_traceFP, L"\t\t\t\t<<DependencyManagerLinkage::AppendLinkage>> %llu linkage {appID:%d, appValue:%d, nRoots:%d root:{%ls...}\n",
                                el->ehdr.uniqueId,
                                dependencyData.appID, dependencyData.appValue, dependencyData.nRoots, buf.c_str());
                }
            }
    #endif

    return  s;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      09/2008
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt DependencyManagerLinkage::AppendLinkage (EditElementHandleR eh, DependencyLinkage const& linkage, size_t xtrabytes)
    {
    MSElementDescrP elDscr =eh.GetElementDescrP();
    if (NULL == elDscr)
        return ERROR;

    size_t depsz = DependencyManagerLinkage::GetSizeofLinkage (linkage, 0) + xtrabytes;
    size_t elsz  = elDscr->Element().Size();
    elDscr->ReserveMemory(elsz + depsz);
    AppendLinkageToMSElement (&elDscr->ElementR(), linkage, xtrabytes);
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      09/2008
+---------------+---------------+---------------+---------------+---------------+------*/
int DependencyManagerLinkage::DeleteLinkageFromMSElement (DgnElement *pEl, UShort appID, UShort appValue)
    {
    //  Remove all DependencyLinkages w/ matching appID and type
    LinkageMatchParms dp (pEl, appID, appValue);
    mdlElement_processLinkagesDirect (processLinkage_deleteMatching, &dp, pEl);

    #if defined (ENABLE_TRACING)
        if (s_trace>1 && dp.nFound>0 || s_trace>4)
            {
            toolSubsystem_fwprintf (g_traceFP, L"\t\t\t\t<<DependencyManagerLinkage::DeleteLinkage>> %llu linkage {appID:%d, appValue:%d} ",
                        pEl->ehdr.uniqueId,
                        appID, appValue);
            toolSubsystem_fwprintf (g_traceFP, L"%ls\n", (dp.nFound>0)? L" DELETED": L" *** NOT FOUND ***");
            }
    #endif

    return  dp.nFound;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      09/2008
+---------------+---------------+---------------+---------------+---------------+------*/
int      DependencyManagerLinkage::DeleteLinkage
(                               
EditElementHandleR    eh,          
UShort           appID,         
UShort           appValue       
)
    {
    return DeleteLinkageFromMSElement (eh.GetElementP(), appID, appValue);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      09/2008
+---------------+---------------+---------------+---------------+---------------+------*/
struct DeleteAllParms
{
EditElementHandleR          m_eh;
DependencyManagerLinkage::DependencySelector const*   m_selector;

DeleteAllParms (EditElementHandleR eh, DependencyManagerLinkage::DependencySelector const* s) : m_eh(eh), m_selector(s) {;}
bool    ShouldDelete (DependencyLinkage const& linkage) const
    {
    return NULL==m_selector || m_selector->ShouldProcessDependency (linkage, m_eh);
    }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      09/2008
+---------------+---------------+---------------+---------------+---------------+------*/
static int    processLinkage_deleteAll
(
LinkageHeader   *pOutLink,
void            *pParmsPtr,
LinkageHeader   *pInLink,
DgnElementP      pElem
)
    {
    if (!pInLink->user || LINKAGEID_Dependency != pInLink->primaryID)
        return 0;

    DependencyLinkage *pDep = EXTRACT_LINKAGE (pInLink);

    DeleteAllParms* parms = (DeleteAllParms*) pParmsPtr;

    return parms->ShouldDelete (*pDep)? PROCESS_ATTRIB_STATUS_DELETE: PROCESS_ATTRIB_STATUS_NOCHANGE;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      09/2008
+---------------+---------------+---------------+---------------+---------------+------*/
void DependencyManagerLinkage::DeleteAll (EditElementHandleR eh, bool includingComponents, DependencySelector const* selector)
    {
    DeleteAllParms parms (eh, selector);
    mdlElement_processLinkagesDirect (processLinkage_deleteAll, (void*)&parms, eh.GetElementP());

    if (includingComponents)
        {
        for (ChildEditElemIter ci (eh, ExposeChildrenReason::Count); ci.IsValid(); ci = ci.ToNext ())
            {
            DeleteAll (ci, true);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      09/2008
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt DependencyManagerLinkage::UpdateLinkage
(                               
EditElementHandleR             eh,          
DependencyLinkage const&    dep,      
size_t                      xtrabytes  
)
    {
    RETURN_IF_INVALID_DTYPE (dep.u.f.rootDataType, ERROR);

    #if defined (ENABLE_TRACING)
        if (s_trace>1)
            toolSubsystem_fwprintf (g_traceFP, L"\t\t\t\t<<DependencyManagerLinkage::UpdateLinkage>> %llu linkage {appID:%d, appValue:%d, xtrabytes=%d}\n",
                            eh.GetElementCP()->ehdr.uniqueId,
                            dep.appID, dep.appValue, xtrabytes);
    #endif

    // Try to update in place
    LinkageUpdateParms update (eh.GetElementP(), &dep, static_cast<int>(xtrabytes), /*findFirst*/true);

    update.result = ERROR;
    mdlElement_processLinkagesDirect (processLinkage_updateInPlace, &update, eh.GetElementP());

    // Couldn't update in place? delete+append
    if (update.result != SUCCESS)
        {
        update.nFound = 0;
        update.findFirst = true;
        mdlElement_processLinkagesDirect (processLinkage_deleteMatching, &update, eh.GetElementP());
        if (update.nFound)
            update.result = DependencyManagerLinkage::AppendLinkage (eh, dep, xtrabytes);
        }

    return update.result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      09/2008
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt DependencyManagerLinkage::GetLinkageFromMSElement
(
DependencyLinkageAccessor* ppDependencyLinkageInplace,
DgnElementCP         element,
UShort              appID,
UShort              appValue
)
    {
    //  Scan for matching linkage
    LinkageMatchParms match (element, appID, appValue, /*findFirst*/true);
    mdlElement_visitLinkages (processLinkage_detectMatching, &match, element);

    //  Not found? => ERROR
    if (0 == match.nFound)
        return ERROR;

    //  Optional: return pointer to in-place linkage
    if (NULL != ppDependencyLinkageInplace)
        ppDependencyLinkageInplace->SetUnalignedData ((DependencyLinkage*)(match.linkage+1));

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      09/2008
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt DependencyManagerLinkage::GetLinkage
(
DependencyLinkageAccessor* ppDependencyLinkageInplace,
ElementHandleCR        eh,
UShort              appID,
UShort              appValue
)
    {
    return DependencyManagerLinkage::GetLinkageFromMSElement (ppDependencyLinkageInplace, eh.GetElementCP(), appID, appValue);
    }

/*----------------------------------------------------------------------------------*//**
* Get the stored size of the specified dependency linkage.
*<p>
* This can be useful in order to predict whether adding a linkage to an element will
* exceed the maximum element size.
*<p>
* The returned size includes the standard LinkageHeader size. The returned size is padded
* and rounded as required for stored linkage sizes.
*<p>
* The returned size is in in bytes.
* @bsimethod                    SamWilson                       05/01
+---------------+---------------+---------------+---------------+---------------+------*/
size_t   DependencyManagerLinkage::GetSizeofLinkage
(
DependencyLinkage const& dep, 
size_t              xtrabytes 
)
    {
    size_t fullsize  = dependencyLinkageSize (&dep) + xtrabytes;
    fullsize        += sizeof(LinkageHeader);    // account for std. linkage header
    fullsize         = (fullsize + 7) & ~7;     // pad to 8-byte multiple

    // account for round-up effects
    LinkageHeader dummy;
    memset (&dummy, 0, sizeof (dummy));
    dummy.primaryID  = LINKAGEID_Dependency;
    dummy.user       = true;
    LinkageUtil::SetWords (&dummy, static_cast<int>(fullsize/2));
    return LinkageUtil::GetWords (&dummy) * 2;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      01/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void DependencyManagerLinkage::GetRootElementIds (bvector<UInt64>& ids, ElementHandleCR eh, UShort applicationID, UShort applicationValue)
    {
    GetRootElementIdsInMSElement (ids, eh.GetElementCP(), applicationID, applicationValue);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      01/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void            DependencyManagerLinkage::GetRootElementIdsInMSElement
(
bvector<UInt64>& ids,
DgnElementCP     elmP,            
UShort          applicationID,   
UShort          applicationValue 
)
    {
    DependencyLinkageAccessor depLinkageP;

    if (SUCCESS != GetLinkageFromMSElement (&depLinkageP, elmP, applicationID, applicationValue))
        return;

    for (UInt16 i=0; i<depLinkageP->nRoots; i++)
        {
        DependencyRoot roots[DEPENDENCY_MAX_TARGETS_PER_ROOT];
        UInt16 n = GetRoots (roots, NULL, *depLinkageP, i);
        for (UInt16 j = 0; j<n; ++j)
            {
            if (roots[j].elemid != 0)
                ids.push_back (roots[j].elemid);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void DependencyLinkageAccessor::SetUnalignedData (DependencyLinkage const* dl)
    {
    m_alignedArray.Clear();
    m_alignedLinkage = (DependencyLinkage*) m_alignedArray.GetAlignedData ((byte*)dl, DependencyManagerLinkage::GetSizeofLinkage (*dl,0));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
size_t DependencyLinkageAccessor::GetSizeofLinkage () const 
    {
    return DependencyManagerLinkage::GetSizeofLinkage (*GetDependencyLinkage(), m_extraBytes);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void DependencyLinkageAccessor::CopyTo (DependencyLinkage* target) const 
    {
    if (target == GetDependencyLinkage())
        return;
    memcpy (target, GetDependencyLinkage(), GetSizeofLinkage());
    }
