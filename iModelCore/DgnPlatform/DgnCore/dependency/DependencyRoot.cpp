/*----------------------------------------------------------------------+
|
|     $Source: DgnCore/dependency/DependencyRoot.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+----------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

/*  Size of a dependency linkage */
#define DEPENDENCY_HEADER_SIZE  offsetof (DependencyLinkage, root)

#define RETURN_IF_INVALID_DTYPE(d,e)  if ((d) < 0 || DEPENDENCY_DATA_TYPE_MAX < (d)) {BeAssert (false); return e;}

#define ID_OF(elemRef)          ((elemRef) ? elemRef->GetElementId().GetValue(): 0)

DependencyManagerLinkage::RootGetFunc          g_getRoots[DEPENDENCY_DATA_TYPE_MAX+1];              // MT OK

static DependencyManagerLinkage::RootSetFunc  s_setRoots[DEPENDENCY_DATA_TYPE_MAX+1];       // static OK for MT
static unsigned                               s_sizeofData[DEPENDENCY_DATA_TYPE_MAX+1];     // static OK for MT
static DependencyManagerLinkage::RootFmtFunc  s_fmtRoots[DEPENDENCY_DATA_TYPE_MAX+1];       // static OK for MT

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      01/2009
+---------------+---------------+---------------+---------------+---------------+------*/
static ElementRefP getElementByID (DgnModelP model, ElementId id)
    {
    ElementRefP ref = model->FindElementById (id);
    if (NULL != ref)
        return ref;

    if (DgnModelSections::None == model->IsFilled (DgnModelSections::Model))
        model->FillSections (DgnModelSections::Model);

    return model->FindElementById (id);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/2008
+---------------+---------------+---------------+---------------+---------------+------*/
static UInt16  getFarPathRoots
(
DependencyRoot          *roots,     
DgnModelP                homeModel,
DependencyLinkage const& dl,       
UInt16                   iRoot     
)
    {
    BeAssert (iRoot < dl.nRoots);

    if (iRoot >= dl.nRoots)       // 0 roots or iRoot is out of range.
        return 1;

    BeAssert (dl.nRoots > 0);

    DependencyRoot&     root = roots[0];
    UInt64 const*    pElemID = dl.root.elemid;

    root.elemid     = ElementId().GetValue();
    root.ref        = NULL;

    if (dl.u.f.invalid || NULL == homeModel)
        {   // homeModel==NULL means that caller is remapping IDs and he wants me to return the remap key
        roots->elemid = *pElemID;
        return 1;
        }

    for (int i=dl.nRoots-1; i>=0; --i)
        {
        if (i == iRoot)
            {
            root.elemid     = pElemID[i];
            root.ref        = getElementByID (homeModel, ElementId(pElemID[i]));
            return 1;
            }
        }

    BeAssert (false && "we know that iRoot was in the range 0..nRoots-1, so we must have found it in the loop above");
    return 1;
    }

/*----------------------------------------------------------------------------------*//**
* "get" function for ElementId roots
* @bsimethod                    SamWilson                       03/01
+---------------+---------------+---------------+---------------+---------------+------*/
static UInt16  getElementID
(                                   
DependencyRoot          *roots,     
DgnModelP                homeModel,
DependencyLinkage const& dl,
UInt16                   iRoot      
)
    {
    if (DEPENDENCYAPPID_MicroStation == dl.appID && DEPENDENCYAPPVALUE_FarReference == dl.appValue)
        return getFarPathRoots (roots, homeModel, dl, iRoot);

    DependencyRoot      &root = roots[0]; // we only support one root per ElementId item
    const UInt64     *pElemID = (const UInt64 *) &dl.root;

    DependencyManager::ResolveRefInSameFile (root, homeModel, pElemID[iRoot]);
    return 1;
    }

/*----------------------------------------------------------------------------------*//**
* "set" function for ElementId roots
* @bsimethod                    SamWilson                       03/01
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt       setElementID
(
DependencyLinkage&  dl,
DgnModelP           homeModel,
DependencyRoot const *roots,
UInt16              iRoot      
)
    {
    UInt64 *pElemID = dl.root.elemid;
    const DependencyRoot& root = roots[0]; // we only support one root per ElementId item

    if (homeModel != NULL && root.ref != NULL && root.ref->GetDgnModelP() != homeModel && !homeModel->IsDictionaryModel())
        return ERROR;   // inter-model refs not supported (except NMC->model)

    pElemID[iRoot] = root.elemid;

    return SUCCESS;
    }

/*----------------------------------------------------------------------------------*//**
* "get" function for FarElementId roots
* roots[0] <= the far root element
* roots[1] <= the local ref attachment element
*
* Note: (roots[0].refattid == roots[1].elemid);
*
* @bsimethod                    SamWilson                       03/01
+---------------+---------------+---------------+---------------+---------------+------*/
static UInt16        getFarElementID
(                                   
DependencyRoot          *roots,     
DgnModelP                homeModel,
DependencyLinkage const& dl,
UInt16                   iRoot      
)
    {
    DependencyRootFarElementID const *pElemID = (const DependencyRootFarElementID *) &dl.root;

    DependencyManager::ResolveRefInSameFile (roots[0], homeModel,  pElemID[iRoot].elemid);

    roots[1].elemid = pElemID[iRoot].unusedref; // accessed only by the importer

    return 2;
    }

/*----------------------------------------------------------------------------------*//**
* "set" function for ElementId roots
* roots[0] == the far root element
* roots[1] == the local ref attachment element
*
* Note: the reference attachment ID for the far root element is reported by get
*       in BOTH roots[0].refattid AND in roots[1].elemid. We'll assume that
*       the caller has modified it in roots[0].refattid, and we will take the new
*       value from there.
*
* @bsimethod                    SamWilson                       03/01
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt       setFarElementID
(
DependencyLinkage&  dl,
DgnModelP           homeModel,
DependencyRoot const *roots,
UInt16              iRoot      
)
    {
    DependencyRootFarElementID *pElemID = (DependencyRootFarElementID *) &dl.root;

    pElemID[iRoot].elemid   = roots[0].elemid;    // the far root element
    pElemID[iRoot].unusedref = 0;

    return SUCCESS;
    }

/*----------------------------------------------------------------------------------*//**
* "get" function for DependencyRootElementID_V roots
* @bsimethod                    SamWilson                       03/01
+---------------+---------------+---------------+---------------+---------------+------*/
static UInt16        getElementID_V
(                                   
DependencyRoot          *roots,     
DgnModelP                homeModel,
DependencyLinkage const& dl,
UInt16                   iRoot      
)
    {
    DependencyRoot& root = roots[0];    // only one root per DependencyRootElementID_V item
    DependencyRootElementID_V *pElementID_V = (DependencyRootElementID_V *) &dl.root;
    DependencyManager::ResolveRefInSameFile (root, homeModel, pElementID_V[iRoot].elemid);
    root.data = pElementID_V[iRoot].value;
    return 1;
    }

/*----------------------------------------------------------------------------------*//**
* "set" function for DependencyRootElementID_V roots
* @bsimethod                    SamWilson                       03/01
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt       setElementID_V
(
DependencyLinkage&  dl,
DgnModelP           homeModel,
DependencyRoot const *roots,
UInt16              iRoot      
)
    {
    DependencyRoot const& root = roots[0];    // only one root per DependencyRootElementID_V item
    DependencyRootElementID_V *p = (DependencyRootElementID_V *) &dl.root;

    if (homeModel != NULL && root.ref != NULL && root.ref->GetDgnModelP() != homeModel && !homeModel->IsDictionaryModel())
        return ERROR;   // inter-model refs not supported (except NMC->model)

    p[iRoot].elemid = root.elemid;

    return SUCCESS;
    }

/*----------------------------------------------------------------------------------*//**
* "get" function for DependencyRootFarElementID_V roots
* roots[0] <= the far root element
* roots[1] <= the local ref attachment element
*
* Note: (roots[0].refattid == roots[1].elemid);
*
* @bsimethod                    SamWilson                       03/01
+---------------+---------------+---------------+---------------+---------------+------*/
static UInt16        getFarElementID_V
(                                   
DependencyRoot          *roots,     
DgnModelP                homeModel,
DependencyLinkage const& dl,
UInt16                   iRoot      
)
    {
    const DependencyRootFarElementID_V  *pElemID_V = (const DependencyRootFarElementID_V *) &dl.root;

    DependencyManager::ResolveRefInSameFile (roots[0], homeModel, pElemID_V[iRoot].s.elemid);

    roots[0].data = pElemID_V[iRoot].s.value;

    roots[1].elemid = pElemID_V[iRoot].unusedref;   // return the attachment id. Used only during importing.

    return 2;
    }

/*----------------------------------------------------------------------------------*//**
* "set" function for DependencyRootFarElementID_V roots
* roots[0] == the far root element
* roots[1] == the local ref attachment element
*
* Note: the reference attachment ID for the far root element is reported by get
*       in BOTH roots[0].refattid AND in roots[1].elemid. We'll assume that
*       the caller has modified it in roots[0].refattid, and we will take the new
*       value from there.
*
* @bsimethod                    SamWilson                       03/01
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt       setFarElementID_V
(
DependencyLinkage&  dl,
DgnModelP           homeModel,
DependencyRoot const *roots,
UInt16              iRoot      
)
    {
    DependencyRootFarElementID_V *p = (DependencyRootFarElementID_V *) &dl.root;

//    BeAssert (roots[0].refattid == roots[1].elemid);

    p[iRoot].s.elemid = roots[0].elemid;
    p[iRoot].unusedref = 0;

    return SUCCESS;
    }

/*----------------------------------------------------------------------------------*//**
* "get" function for PATH_V roots
* @bsimethod                    BarryBentley                    08/02
+---------------+---------------+---------------+---------------+---------------+------*/
static UInt16        getPath_V
(                                   
DependencyRoot          *roots,     
DgnModelP                homeModel,
DependencyLinkage const& dl,
UInt16                   iRoot      
)
    {
    // This type of dependency linkage resolves to make an element in the master file dependent
    //  on one element in a nested reference file. It is used for named groups and is not of
    //  much use in any other situation. It is always called with iRoot of 0.
    roots[0].elemid     = INVALID_ELEMENTID;
    roots[0].ref        = NULL;

    if (dl.u.f.invalid || NULL == homeModel)
        {
        roots[0].elemid     = dl.root.path_v.path[0];
        }
    else if ((0 != dl.root.path_v.numElemsInPath) && iRoot == 0)
        {
        roots[0].elemid     = dl.root.path_v.path[0];
        roots[0].ref        = homeModel? getElementByID (homeModel, ElementId(dl.root.path_v.path[0])) : NULL;
        }

    roots[0].data = dl.root.path_v.value;

    return 1;
    }

/*----------------------------------------------------------------------------------*//**
* "set" function for ElementId roots
* @bsimethod                    BarryBentley                    08/02
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt       setPath_V
(
DependencyLinkage&  dl,
DgnModelP           homeModel,
DependencyRoot const *roots,
UInt16              iRoot      
)
    {
    // the roots have been remapped to the two values in *roots, put them into the array.
    dl.root.path_v.path[0] = roots[0].elemid;

    return SUCCESS;
    }

/*======================================================================+
|                                                                       |
|    Public Code Section                                                |
|                                                                       |
|                                                                       |
+======================================================================*/
/*----------------------------------------------------------------------+
|                                                                       |
| name          dependency_getRoots                                     |
|                                                                       |
| author        SamWilson                               01/01           |
|                                                                       |
+----------------------------------------------------------------------*/
static UInt16 dependency_getRoots
(                                   // <= # roots written to pRoots
DependencyRoot    *pRoots,          // <= elements referenced by the i'th root (max: DEPENDENCY_MAX_TARGETS_PER_ROOT)
DgnModelP          homeModel,      //  => model of dependent
DependencyLinkage *pDep,            //  => dependency linkage to query
UInt16            iRoot            //  => root in the linkage to query
)
    {
    RETURN_IF_INVALID_DTYPE (pDep->u.f.rootDataType, 0);

    if (iRoot < 0 || iRoot >= pDep->nRoots)
        return 0;

    return g_getRoots[pDep->u.f.rootDataType] (pRoots, homeModel, *pDep, iRoot);
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          dependency_setRoots                                     |
|                                                                       |
| author        SamWilson                               01/01           |
|                                                                       |
+----------------------------------------------------------------------*/
static StatusInt dependency_setRoots
(                                   // <=  ERROR if invalid root encountered
DependencyLinkage *pDep,            // <=> dependency linkage to update
DgnModelP          homeModel,      //  => model of dependent
const DependencyRoot *pRoots,       //  => elements referenced by the i'th root
UInt16             iRoot            //  => root in the linkage to query
)
    {
    RETURN_IF_INVALID_DTYPE (pDep->u.f.rootDataType, ERROR);

    if (iRoot < 0 || iRoot >= pDep->nRoots)
        return ERROR;

    return s_setRoots[pDep->u.f.rootDataType] (*pDep, homeModel, pRoots, iRoot);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      09/2008
+---------------+---------------+---------------+---------------+---------------+------*/
UInt16   DependencyManagerLinkage::GetRoots
(                                   // <= # roots written to pRoots
DependencyRoot    *pRoots,          // <= elements referenced by the i'th root (max: DEPENDENCY_MAX_TARGETS_PER_ROOT)
DgnModelP          homeModel,       //  => model of dependent element
DependencyLinkage const& dep,       //  => dependency linkage to query
UInt16             iRoot            //  => root in the linkage to query
)
    {
    return dependency_getRoots (pRoots, homeModel, const_cast<DependencyLinkage*>(&dep), iRoot);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      09/2008
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt  DependencyManagerLinkage::SetRoots
(                                   // <=  ERROR if invalid root encountered
DependencyLinkage& dep,            // <=> dependency linkage to update
DgnModelP          homeModel,       //  => model of dependent element
const DependencyRoot*pRoots,        //  => elements referenced by the i'th root
UInt16             iRoot            //  => root in the linkage to query
)
    {
    return dependency_setRoots (&dep, homeModel, pRoots, iRoot);
    }

/*-----------------------------------------------------------------------
   _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
  _/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 _/_/_/_/_/_/_/_/_/_/ #$-debugging utilities-$#_/_/_/_/_/_/_/_/_/_/_/_/
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
-----------------------------------------------------------------------*/
/*----------------------------------------------------------------------------------*//**
* Identify an element, using filename:filenum:type:element notation
* @bsimethod                    SamWilson                       03/01
+---------------+---------------+---------------+---------------+---------------+------*/
WString DependencyManagerLinkage::FmtDependencyRoot (DependencyRoot const& root)
    {
#ifdef DGNV10FORMAT_CHANGES_WIP
    WChar buf[256];
    if (root.ref == NULL)
        {
        BeStringUtilities::Snwprintf (buf, _countof(buf), L"(R%llu:?:M?:T?:E%llu)", root.refattid, root.elemid);
/*<*/   return buf;
        }

    int         etype   = elementRef_getElemType (root.ref);

    DgnModelP   model   = root.ref->GetDgnModelP();
    ModelId     mid     = model->GetModelId();
    DgnProjectP    file    = model->GetDgnProject();

    WChar name[256];
    mdlFile_parseName (file->GetFileName().c_str(), NULL, NULL, name, NULL);
    if (dgnFileObj_isReadOnly(file))
        wcscat (name, L"(RO)");

    BeStringUtilities::Snwprintf (buf, _countof(buf), L"(R%llu:F\"%ls\":M%d:T%d:E%llu)", root.refattid, name, mid, etype, root.elemid);

/*
#ifndef NDEBUG
    WChar addrs[128];
    BeStringUtilities::Snwprintf (addrs, _countof(addrs), L" (%p:%p:%p)", file, model, root.ref);
    wcscat (buf, addrs);
#endif
*/

    if (root.ref->IsDeletedOrMoved())
        wcscat (buf, L"<DE>");

    if (root.ref->IsDeleted())
        wcscat (buf, L"<DR>");

    if (root.ref->InDeletedCache())
        wcscat (buf, L"<DC>");

    return buf;
#endif
    return  NULL;
    }

/*----------------------------------------------------------------------------------*//**
* Identify an element, using filename:filenum:type:element notation
* @bsimethod                    SamWilson                       03/01
+---------------+---------------+---------------+---------------+---------------+------*/
SemiPrivate WCharCP     fmtDependencyRoot (DependencyRoot const& root)
    {
    static WChar buf[256];
    WString s = DependencyManagerLinkage::FmtDependencyRoot (root);
    BeStringUtilities::Wcsncpy (buf, _countof (buf), s.c_str());
    return buf;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2006
+---------------+---------------+---------------+---------------+---------------+------*/
SemiPrivate WCharCP fmtElementRef
(
ElementRefP      ref
)
    {
    DependencyRoot root;
    root.ref       = ref;
    root.elemid    = ID_OF (ref);
    return fmtDependencyRoot (root);
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                    SamWilson                       03/01
+---------------+---------------+---------------+---------------+---------------+------*/
SemiPrivate WCharCP     fmtCache
(
DgnModelP       model
)
    {
    static WChar    buf[1024];
    BeStringUtilities::Snwprintf (buf, L"%ls (modelID %lld) model:%p file:%p", model->GetDgnProject().GetFileName().c_str(), model->GetModelId().GetValue(), model, &model->GetDgnProject());
    return buf;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      01/2009
+---------------+---------------+---------------+---------------+---------------+------*/
static WString fmtRoot0
(
UInt16                  rootType,
DependencyLinkage const& dl,
UInt16                  iRoot,
DgnModelP               model
)
    {
    char asc[256];
    size_t maxAsc = sizeof (asc);
    switch (dl.u.f.rootDataType)
        {
        default:
            BeStringUtilities::Snprintf (asc, maxAsc, "<<UNKNOWN DATA TYPE %d?? >>", dl.u.f.rootDataType);
            break;

        case DEPENDENCY_DATA_TYPE_ELEM_ID:
            BeStringUtilities::Snprintf (asc, maxAsc, "%hs", "ElemID{");
            break;

        case DEPENDENCY_DATA_TYPE_ELEM_ID_V:
            BeStringUtilities::Snprintf (asc, maxAsc, "ElemIdValue(%f){", dl.root.e_v[iRoot].value);
            break;

        case DEPENDENCY_DATA_TYPE_ELEM_ID_IN_MODEL:
            BeStringUtilities::Snprintf (asc, maxAsc, "ElemIDInModel{%d, ", dl.root.elemidInModel[iRoot].v8modelId);
            break;

        case DEPENDENCY_DATA_TYPE_FAR_ELEM_ID:
            BeStringUtilities::Snprintf (asc, maxAsc, "%hs", "FarElemID{");
            break;

        case DEPENDENCY_DATA_TYPE_FAR_ELEM_ID_V:
            BeStringUtilities::Snprintf (asc, maxAsc, "FarElemIDValue(%f){", dl.root.far_e_v[iRoot].s.value);
            break;

        case DEPENDENCY_DATA_TYPE_PATH_V:
            {
            BeStringUtilities::Snprintf (asc, maxAsc, "Path{%f){", dl.root.path_v.value);
            unsigned n = dl.root.path_v.numElemsInPath;
            for (unsigned i=0; i<n; ++i)
                {
                char root[64];
                sprintf (root, " %llu", dl.root.path_v.path[i]);
                strncat (asc, root, maxAsc - strlen (asc) - 1);
                }
            strncat (asc, "}{", maxAsc - strlen (asc) - 1);
            break;
            }
        }

    return WString (asc, false);
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          fmtRoot -- display root for debugging purposes          |
|                                                                       |
| author        SamWilson                               10/00           |
|                                                                       |
+----------------------------------------------------------------------*/
SemiPrivate WCharCP     fmtRoot
(
WCharP             pBufOut,        // <=
DgnModelP          model,
DependencyLinkage const* pLinkage,
UInt16             iRoot,
int                maxBuf
)
    {
    WString buf = s_fmtRoots[pLinkage->u.f.rootDataType] (pLinkage->u.f.rootDataType, *pLinkage, iRoot, model);

    DependencyRoot  roots[DEPENDENCY_MAX_TARGETS_PER_ROOT];
    unsigned n = g_getRoots[pLinkage->u.f.rootDataType] (roots, model, *pLinkage, iRoot);

    for (unsigned i=0; i<n; ++i)
        {
        buf.append (L" ");
        buf.append (DependencyManagerLinkage::FmtDependencyRoot (roots[i]));
        }
    buf.append (L"}");

    BeStringUtilities::Wcsncpy (pBufOut, maxBuf, buf.c_str());
    return pBufOut;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      01/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void    DependencyManagerLinkage::DefineRootType (UInt16 i, size_t sz, RootGetFunc g, RootSetFunc s, void* m, RootFmtFunc f)
    {
    BeAssert (DgnPlatformLib::InStaticInitialization());

    if (i > DEPENDENCY_DATA_TYPE_MAX)
        {BeAssert (false); return;}

    g_getRoots[i] = g;
    s_setRoots[i] = s;
    s_sizeofData[i] = static_cast<unsigned int>(sz);
    s_fmtRoots[i] = f;
    }

static UInt16      getElementIdInModel_notSupportedInV10 (DependencyRoot[DEPENDENCY_MAX_TARGETS_PER_ROOT], DgnModelP, DependencyLinkage const&, UInt16 ) {BeDataAssert (false && "DEPENDENCY_DATA_TYPE_ELEM_ID_IN_MODEL not supported in DgnDb"); return 0;}
static StatusInt   setElementIdInModel_notSupportedInV10 (DependencyLinkage&, DgnModelP, DependencyRoot const*, UInt16) {BeDataAssert (false && "DEPENDENCY_DATA_TYPE_ELEM_ID_IN_MODEL not supported in DgnDb"); return ERROR;}


/*----------------------------------------------------------------------+
| author        SamWilson                               10/00           |
|                                                                       |
+----------------------------------------------------------------------*/
SemiPrivate void dependencyRoot_staticInitialize ()
    {
    BeAssert (DgnPlatformLib::InStaticInitialization());
    BeAssert (offsetof (DependencyLinkage, root) % 8 == 0); /* ***NB! start of union must be 8-byte aligned! */

    DependencyManagerLinkage::DefineRootType (DEPENDENCY_DATA_TYPE_ELEM_ID,          sizeof(ElementId),                        getElementID,           setElementID,        NULL, fmtRoot0);
#ifdef DGNV10FORMAT_CHANGES_WIP
    DependencyManagerLinkage::DefineRootType (DEPENDENCY_DATA_TYPE_ELEM_ID_IN_MODEL, sizeof(DependencyRootElementIdInModel),   getElementIdInModel,    setElementIdInModel, NULL, fmtRoot0);
#else
    DependencyManagerLinkage::DefineRootType (DEPENDENCY_DATA_TYPE_ELEM_ID_IN_MODEL, sizeof(DependencyRootElementIdInModel),   getElementIdInModel_notSupportedInV10,    setElementIdInModel_notSupportedInV10, NULL, fmtRoot0);
#endif
    DependencyManagerLinkage::DefineRootType (DEPENDENCY_DATA_TYPE_FAR_ELEM_ID,      sizeof(DependencyRootFarElementID),       getFarElementID,        setFarElementID,     NULL, fmtRoot0);
    DependencyManagerLinkage::DefineRootType (DEPENDENCY_DATA_TYPE_ELEM_ID_V,        sizeof(DependencyRootElementID_V),        getElementID_V,         setElementID_V,      NULL, fmtRoot0);
    DependencyManagerLinkage::DefineRootType (DEPENDENCY_DATA_TYPE_FAR_ELEM_ID_V,    sizeof(DependencyRootFarElementID_V),     getFarElementID_V,      setFarElementID_V,   NULL, fmtRoot0);
    DependencyManagerLinkage::DefineRootType (DEPENDENCY_DATA_TYPE_PATH_V,           0,                                        getPath_V,              setPath_V,           NULL, fmtRoot0);

    BeAssert (DEPENDENCY_MAX_DATA/sizeof(ElementId)                     == DEPENDENCY_MAX_ELEMIDS);
    BeAssert (DEPENDENCY_MAX_DATA/sizeof(DependencyRootFarElementID)    == DEPENDENCY_MAX_FARELEMIDS);
    BeAssert (DEPENDENCY_MAX_DATA/sizeof(DependencyRootElementID_V)     == DEPENDENCY_MAX_ELEMIDVS);
    BeAssert (DEPENDENCY_MAX_DATA/sizeof(DependencyRootFarElementID_V)  == DEPENDENCY_MAX_FARELEMIDVS);
    BeAssert (DEPENDENCY_SOME_DATA/sizeof(ElementId)                    == DEPENDENCY_SOME_ELEMIDS);
    BeAssert (DEPENDENCY_SOME_DATA/sizeof(DependencyRootFarElementID)   == DEPENDENCY_SOME_FARELEMIDS);
    BeAssert (DEPENDENCY_SOME_DATA/sizeof(DependencyRootElementID_V)    == DEPENDENCY_SOME_ELEMIDVS);
    BeAssert (DEPENDENCY_SOME_DATA/sizeof(DependencyRootFarElementID_V) == DEPENDENCY_SOME_FARELEMIDVS);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      09/2008
+---------------+---------------+---------------+---------------+---------------+------*/
/*static*/ WString DependencyManagerLinkage::FmtRoot
(
DgnModelP           model,     
DependencyLinkage const& linkage, 
UInt16              iRoot
)
    {
    WChar buf[128];
    fmtRoot (buf, model, &linkage, iRoot, _countof(buf));
    return WString(buf);
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          dependencyLinkageSize                                   |
|                                                                       |
| author        BarryBentley                            08/02           |
|                                                                       |
+----------------------------------------------------------------------*/
SemiPrivate int dependencyLinkageSize (DependencyLinkage const*pLinkage)
    {
    if (pLinkage->u.f.rootDataType != DEPENDENCY_DATA_TYPE_PATH_V)
        return (DEPENDENCY_HEADER_SIZE + pLinkage->nRoots * s_sizeofData[pLinkage->u.f.rootDataType]);
    else
        return DEPENDENCY_HEADER_SIZE  + sizeof(DependencyRootPath_V) + ((pLinkage->root.path_v.numElemsInPath - 1) * sizeof(ElementId));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/2004
+---------------+---------------+---------------+---------------+---------------+------*/
/*static*/ size_t DependencyManagerLinkage::GetSizeofRootDataType (int dt)
    {
    if (dt < 0 || dt > _countof(s_sizeofData))
        return 0;
    return s_sizeofData[dt];
    }

/*---------------------------------------------------------------------------------**//**
* Resolve a reference to a target element that is in the same file as the reference-holder
* element. Normally, both will be in the same model. The one special case we support
* is where the target is in a model and the ref-holder is in the NMC.
* @bsimethod                                                    SamWilson       08/02
+---------------+---------------+---------------+---------------+---------------+------*/
void DependencyManager::ResolveRefInSameFile
(
DependencyRoot& root,           // <=
DgnModelP       homeModel,
UInt64       eid
)
    {
    root.elemid    = eid;

    if (NULL == homeModel)
        root.ref    = NULL; // homeModel==NULL means that caller is remapping IDs and he wants me to return the remap key (eid)
    else
        {
        root.ref    = getElementByID (homeModel, ElementId(eid));

        if (root.ref == NULL && homeModel->IsDictionaryModel()) // (support NMC->model in same file)
            root.ref = homeModel->GetDgnProject().Models().GetElementById(ElementId(eid)).get();
        }
    }

