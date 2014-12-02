/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnHandlers/AssocDependencyRoot.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

/*----------------------------------------------------------------------------------*//**
* "get" function for AssocPoint-based roots
*
* singleElm:
* roots[0] <= far root element
* roots[1] <= local ref attachmenet element
*
* twoElm:
* roots[0] <= far root element#1
* roots[1] <= far root element#2
* roots[2] <= local ref attachment element for far ref #1
* roots[3] <= local ref attachment element for far ref #2

* @bsimethod                    SamWilson                       03/01
+---------------+---------------+---------------+---------------+---------------+------*/
static UInt16    extractAssocPointRoots
(                               // <= # roots in AssocPoint
DependencyRoot      *roots,     // <= roots (max: 4)
DgnModelP            homeCache,//  => cache of dependent
const AssocGeom     *pPoint     //  => AssocPoint
)
    {
    UInt16             numRoots    = 0;

    switch (pPoint->type)
        {
        case INTERSECT_ASSOC:   //  associations with 2 targets:
        case INTERSECT2_ASSOC:
            {
            //  The targetted root elements
            DependencyManager::ResolveRefInSameFile (roots[numRoots++], homeCache, pPoint->twoElm.uniqueId1);
            DependencyManager::ResolveRefInSameFile (roots[numRoots++], homeCache, pPoint->twoElm.uniqueId2);
            roots[numRoots++].elemid = pPoint->twoElm.___legacyref1; // used only by importer -- NB: May be zero!
            roots[numRoots++].elemid = pPoint->twoElm.___legacyref2; // used only by importer
            break;
            }

        default:                // most associations have 1 tag:
            {
            DependencyManager::ResolveRefInSameFile (roots[numRoots++], homeCache, pPoint->singleElm.uniqueId);
            roots[numRoots++].elemid = pPoint->singleElm.___legacyref; // used only by importer -- NB: May be zero!
            break;
            }
        }

    return numRoots;
    }

/*----------------------------------------------------------------------------------*//**
* "set" function for AssocPoint-based roots
*
* Note: For a singleElm AssocPoint,
*       the reference attachment ID for the far root element is reported by get
*       in BOTH roots[0].refattid AND in roots[1].elemid. We'll assume that
*       the caller has modified it in roots[0].refattid, and we will take the new
*       value from there.
*       Similar for twoElm AssocPoints.
*
* @bsimethod                    SamWilson                       03/01
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt   replaceAssocPointRoots
(
AssocGeom           *pPoint,     // <=  AssocPoint
DgnModelP            homeCache, // => cache of dependent
const DependencyRoot*roots      //  => roots (max: 4)
)
    {
    switch (pPoint->type)
        {
        case INTERSECT_ASSOC:   //  associations with 2 targets:
        case INTERSECT2_ASSOC:
            {
            pPoint->twoElm.uniqueId1        = roots[0].elemid;
            pPoint->twoElm.uniqueId2        = roots[1].elemid;
            pPoint->twoElm.___legacyref1 = pPoint->twoElm.___legacyref2 = 0;
            break;
            }

        default:                // most associations have 1 tag:
            {
            pPoint->singleElm.uniqueId        = roots[0].elemid;
            pPoint->singleElm.___legacyref = 0;
            break;
            }
        }

    return SUCCESS;
    }

/*----------------------------------------------------------------------------------*//**
* "get" function for AssocPoint roots
* @bsimethod                    SamWilson                       03/01
+---------------+---------------+---------------+---------------+---------------+------*/
static UInt16 getAssocPointRoots
(                                   
DependencyRoot          *roots,     
DgnModelP                homeCache,
DependencyLinkage const& dl,
UInt16                   iRoot      
)
    {
    const AssocPoint    *pPoints = (const AssocPoint *) &dl.root;

    return extractAssocPointRoots (roots, homeCache, &((AssocGeom *) pPoints)[iRoot]);
    }

/*----------------------------------------------------------------------------------*//**
* "set" function for AssocPoint roots
* @bsimethod                    SamWilson                       03/01
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt       setAssocPointRoots
(
DependencyLinkage&  dl,
DgnModelP           homeCache,
DependencyRoot const *roots,
UInt16              iRoot      
)
    {
    AssocPoint      *pPoints = (AssocPoint *) &dl.root;

    return replaceAssocPointRoots (&((AssocGeom *) pPoints)[iRoot], homeCache, roots);
    }

/*----------------------------------------------------------------------------------*//**
* "get" function for DependencyRootAssocPoint_I roots
* @bsimethod                    SamWilson                       03/01
+---------------+---------------+---------------+---------------+---------------+------*/
static UInt16 getAssocPointIRoots
(                                   
DependencyRoot          *roots,     
DgnModelP                homeCache,
DependencyLinkage const& dl,
UInt16                   iRoot      
)
    {
    const DependencyRootAssocPoint_I  *pPoints = (const DependencyRootAssocPoint_I *) &dl.root;

    roots[0].data = pPoints[iRoot].i;
    roots[0].data2 = pPoints[iRoot].i2;

    return extractAssocPointRoots (roots, homeCache, (AssocGeom *) &((DependencyRootAssocPoint_I *) pPoints)[iRoot].assoc);
    }

/*----------------------------------------------------------------------------------*//**
* "set" function for DependencyRootAssocPoint_I roots
* @bsimethod                    SamWilson                       03/01
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt       setAssocPointIRoots
(
DependencyLinkage&  dl,
DgnModelP           homeCache,
DependencyRoot const *roots,
UInt16              iRoot      
)
    {
    DependencyRootAssocPoint_I    *pPoints = (DependencyRootAssocPoint_I *) &dl.root;

    return replaceAssocPointRoots ((AssocGeom *) &((DependencyRootAssocPoint_I *) pPoints)[iRoot].assoc, homeCache, roots);
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                    SamWilson                       03/01
+---------------+---------------+---------------+---------------+---------------+------*/
void            DependencyRootAssoc1_I::Header::SetUniqueID
(
UInt64 uniqueId
)
    {
    uniqueIdHi = (UInt32)(uniqueId>>32);
    uniqueIdLo = (UInt32)(uniqueId);
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                    SamWilson                       03/01
+---------------+---------------+---------------+---------------+---------------+------*/
ElementId       DependencyRootAssoc1_I::Header::GetUniqueID () const
    {
    return ElementId(((UInt64)uniqueIdHi<<32) | uniqueIdLo);
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                    SamWilson                       03/01
+---------------+---------------+---------------+---------------+---------------+------*/
void            DependencyRootAssoc1_I::Header::Init
(
UInt8              subtype,
UInt64          uniqueId
)
    {
    SetUniqueID (uniqueId);
    this->subtype  = subtype;
    this->userValue8 = 0;
    this->userValue16 = 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/2004
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       DependencyRootAssoc1_I::Linear::FromAssoc (AssocGeom const& assoc)
    {
    if (LINEAR_ASSOC != assoc.type)
        return ERROR;

    Init (DEPENDENCY_ASSOC1_SUBTYPE_LINEAR, assoc.line.uniqueId);
    vertex       = assoc.line.vertex;
    nVertex      = assoc.line.nVertex;
    numerator    = assoc.line.numerator;
    divisor      = assoc.line.divisor;
    memset (pad, 0, _countof (pad));
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/2004
+---------------+---------------+---------------+---------------+---------------+------*/
void            DependencyRootAssoc1_I::Linear::ToAssoc (AssocGeom& assoc) const
    {
    BeAssert (DEPENDENCY_ASSOC1_SUBTYPE_LINEAR==subtype);
    AssociativePoint::InitKeypoint ((AssocPoint&) assoc, vertex, nVertex, numerator, divisor);
    AssociativePoint::SetRoot ((AssocPoint&) assoc, GetUniqueID(), 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/2004
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       DependencyRootAssoc1_I::Projection::FromAssoc (AssocGeom const& assoc)
    {
    if (PROJECTION_ASSOC != assoc.type)
        return ERROR;

    Init (DEPENDENCY_ASSOC1_SUBTYPE_PROJECTION, assoc.projection.uniqueId);
    vertex       = assoc.projection.vertex;
    nVertex      = assoc.projection.nVertex;
    ratioVal     = assoc.projection.ratioVal;
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/2004
+---------------+---------------+---------------+---------------+---------------+------*/
void            DependencyRootAssoc1_I::Projection::ToAssoc (AssocGeom& assoc) const
    {
    BeAssert (DEPENDENCY_ASSOC1_SUBTYPE_PROJECTION==subtype);
    AssociativePoint::InitProjection ((AssocPoint&) assoc, vertex, nVertex, ratioVal);
    AssociativePoint::SetRoot ((AssocPoint&) assoc, GetUniqueID(), 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/2004
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       DependencyRootAssoc1_I::Arc::FromAssoc (AssocGeom const& assoc)
    {
    if (ARC_ASSOC != assoc.type)
        return ERROR;

    Init (DEPENDENCY_ASSOC1_SUBTYPE_ARC, assoc.arc.uniqueId);
    angle        = assoc.arc.angle;
    keyPoint     = assoc.arc.keyPoint;
    memset (pad, 0, _countof (pad));
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/2004
+---------------+---------------+---------------+---------------+---------------+------*/
void            DependencyRootAssoc1_I::Arc::ToAssoc (AssocGeom& assoc) const
    {
    BeAssert (DEPENDENCY_ASSOC1_SUBTYPE_ARC==subtype);
    AssociativePoint::InitArc ((AssocPoint&) assoc, (AssociativePoint::ArcLocation) keyPoint, angle);
    AssociativePoint::SetRoot ((AssocPoint&) assoc, GetUniqueID(), 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/2004
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       DependencyRootAssoc1_I::Origin::FromAssoc (AssocGeom const& assoc)
    {
    if (ORIGIN_ASSOC != assoc.type)
        return ERROR;

    Init (DEPENDENCY_ASSOC1_SUBTYPE_ORIGIN, assoc.origin.uniqueId);
    option       = assoc.origin.option;
    memset (pad, 0, _countof (pad));
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/2004
+---------------+---------------+---------------+---------------+---------------+------*/
void            DependencyRootAssoc1_I::Origin::ToAssoc (AssocGeom& assoc) const
    {
    BeAssert (DEPENDENCY_ASSOC1_SUBTYPE_ORIGIN==subtype);
    AssociativePoint::InitOrigin ((AssocPoint&) assoc, option);
    AssociativePoint::SetRoot ((AssocPoint&) assoc, GetUniqueID(), 0);
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                    SamWilson                       03/01
+---------------+---------------+---------------+---------------+---------------+------*/
static UInt16 assoc1_getRoots
(                                   
DependencyRoot          *roots,     
DgnModelP                homeCache,
DependencyLinkage const& dl,
UInt16                   iRoot      
)
    {
    AssocPoint assoc;
    AssociativePoint::GetAssocFromAssoc1 (assoc, dl, iRoot);
    // ***TBD roots[0].data = ...
    BeAssert (false);
    return extractAssocPointRoots (roots, homeCache, (AssocGeom*)&assoc);
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                    SamWilson                       03/01
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt       assoc1_setRoots
(
DependencyLinkage&  dl,
DgnModelP           homeCache,
DependencyRoot const *roots,
UInt16              iRoot      
)
    {
    DependencyRoot const& root = roots[0];    // only one root per item

    if (homeCache != NULL && root.ref != NULL && root.ref->GetDgnModelP() != homeCache && !homeCache->IsDictionaryModel())
        return ERROR;   // inter-cache refs not supported (except NMC->model)

    dl.root.assoc1_i[iRoot].h.SetUniqueID (root.elemid);
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/2004
+---------------+---------------+---------------+---------------+---------------+------*/
/*static*/ StatusInt AssociativePoint::DefineAssoc1FromAssoc
(
DependencyLinkage&  dl,         // <=
int                 iRoot,
AssocPoint const&   assocPoint
)
    {
    AssocGeom const& assoc = (AssocGeom&) assocPoint;

    DependencyRootAssoc1_I* r = &dl.root.assoc1_i[iRoot];

    switch (assoc.type)
        {
        case LINEAR_ASSOC:      return r->linear.FromAssoc (assoc);
        case ARC_ASSOC:         return r->arc.FromAssoc (assoc);
        case ORIGIN_ASSOC:      return r->origin.FromAssoc (assoc);
        case PROJECTION_ASSOC:  return r->projection.FromAssoc (assoc);
        }
    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @Description Define an DEPENDENCY_DATA_TYPE_ASSOC1_I dependency linkage root to
*               capture the state of the specified AssocPoint.
* @returns non-zero error code if the assocPoint cannot be captured by an DEPENDENCY_DATA_TYPE_ASSOC1_I root.
* @bsimethod                                                    BSI             07/2004
+---------------+---------------+---------------+---------------+---------------+------*/
/*static*/ StatusInt AssociativePoint::DefineAssoc1FromAssocAndSubtype
(
DependencyLinkage&  dl,         // <=
int                 iRoot,
AssocPoint const&   assocPoint,
int                 assoc1Subtype
)
    {
    DependencyRootAssoc1_I* r = &dl.root.assoc1_i[iRoot];

    AssocGeom const& assoc = (AssocGeom&) assocPoint;

    switch (assoc1Subtype)
        {
        case DEPENDENCY_ASSOC1_SUBTYPE_LINEAR:    r->linear.FromAssoc (assoc);      return SUCCESS;
        case DEPENDENCY_ASSOC1_SUBTYPE_PROJECTION:r->projection.FromAssoc (assoc);  return SUCCESS;
        case DEPENDENCY_ASSOC1_SUBTYPE_ARC:       r->arc.FromAssoc (assoc);         return SUCCESS;
        case DEPENDENCY_ASSOC1_SUBTYPE_ORIGIN:    r->origin.FromAssoc (assoc);      return SUCCESS;
        }

    r->h.SetUniqueID (0);
    r->h.subtype = 0xff;

    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @Description
* @bsimethod                                                    BSI             07/2004
+---------------+---------------+---------------+---------------+---------------+------*/
Public StatusInt AssociativePoint::GetAssocFromAssoc1
(
AssocPoint&                 assocPoint,     // <=
DependencyLinkage const&    dl,
int                         iRoot
)
    {
    AssocGeom& assoc = (AssocGeom&) assocPoint;

    if (DEPENDENCY_DATA_TYPE_ASSOC1_I == dl.u.f.rootDataType)
        {
        DependencyRootAssoc1_I const& r = dl.root.assoc1_i[iRoot];

        switch (dl.root.assoc1_i[iRoot].h.subtype)
            {
            case DEPENDENCY_ASSOC1_SUBTYPE_LINEAR:      r.linear.ToAssoc (assoc);   return SUCCESS;
            case DEPENDENCY_ASSOC1_SUBTYPE_PROJECTION:  r.projection.ToAssoc (assoc); return SUCCESS;
            case DEPENDENCY_ASSOC1_SUBTYPE_ARC:         r.arc.ToAssoc (assoc);       return SUCCESS;
            case DEPENDENCY_ASSOC1_SUBTYPE_ORIGIN:      r.origin.ToAssoc (assoc);    return SUCCESS;
            }
        }

    assoc.type = 0xff;
    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/2008
+---------------+---------------+---------------+---------------+---------------+------*/
static WString fmtAssocPointDependency
(
UInt16                  rootType,
DependencyLinkage const& dl,
UInt16                  iRoot,
DgnModelP               cache
)
    {
    WChar buf[256];
    size_t maxBuf = _countof (buf);
    switch (dl.u.f.rootDataType)
        {
        default:
            return L"";

        case DEPENDENCY_DATA_TYPE_ASSOC1_I:
            {
            AssocPoint assoc;
            AssociativePoint::GetAssocFromAssoc1 (assoc, dl, iRoot);
            BeStringUtilities::Snwprintf (buf, maxBuf, L"{%d,%d} ", dl.root.assoc1_i[iRoot].h.userValue16, dl.root.assoc1_i[iRoot].h.userValue8);
            return WString(buf) + AssociativePoint::FormatAssocPointString (assoc);
            }

        case DEPENDENCY_DATA_TYPE_ASSOC_POINT:
            return AssociativePoint::FormatAssocPointString (dl.root.assoc[iRoot]);

        case DEPENDENCY_DATA_TYPE_ASSOC_POINT_I:
            {
            BeStringUtilities::Snwprintf (buf, maxBuf, L"{%d,%d} ", dl.root.a_i[iRoot].i, dl.root.a_i[iRoot].i2);
            return WString(buf) + AssociativePoint::FormatAssocPointString (dl.root.a_i[iRoot].assoc);
            }
        }
    // Unreachable Code
    // return WString ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      01/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void            assocDependencyRoot_staticInitialize ()
    {
    DependencyManagerLinkage::DefineRootType (DEPENDENCY_DATA_TYPE_ASSOC_POINT,    sizeof(AssocPoint),                  getAssocPointRoots, setAssocPointRoots,     NULL, fmtAssocPointDependency);
    DependencyManagerLinkage::DefineRootType (DEPENDENCY_DATA_TYPE_ASSOC_POINT_I,  sizeof(DependencyRootAssocPoint_I),  getAssocPointIRoots, setAssocPointIRoots,   NULL, fmtAssocPointDependency);
    DependencyManagerLinkage::DefineRootType (DEPENDENCY_DATA_TYPE_ASSOC1_I,       sizeof(DependencyRootAssoc1_I),      assoc1_getRoots,     assoc1_setRoots,       NULL, fmtAssocPointDependency);

    BeAssert (DEPENDENCY_SOME_DATA/sizeof(AssocPoint)                   == DEPENDENCY_SOME_ASSOCPOINTS);
    BeAssert (DEPENDENCY_SOME_DATA/sizeof(DependencyRootAssocPoint_I)   == DEPENDENCY_SOME_ASSOCPOINTIS);
    BeAssert (DEPENDENCY_SOME_DATA/sizeof(DependencyRootAssoc1_I)       == DEPENDENCY_SOME_ASSOC1IS);

    BeAssert (DEPENDENCY_MAX_DATA/sizeof(AssocPoint)      == DEPENDENCY_MAX_ASSOCPOINTS);
    BeAssert (DEPENDENCY_MAX_DATA/sizeof(DependencyRootAssocPoint_I)    == DEPENDENCY_MAX_ASSOCPOINTIS);
    BeAssert (DEPENDENCY_MAX_DATA/sizeof(DependencyRootAssoc1_I)        == DEPENDENCY_MAX_ASSOC1IS);

    BeAssert ((sizeof(DependencyRootAssoc1_I::Linear) & 7) == 0);    // 8-byte multiple
    BeAssert (sizeof(DependencyRootAssoc1_I::Linear) == sizeof(DependencyRootAssoc1_I::Arc));
    BeAssert (sizeof(DependencyRootAssoc1_I::Linear) == sizeof(DependencyRootAssoc1_I::Origin));
    BeAssert (sizeof(DependencyRootAssoc1_I::Linear) == sizeof(DependencyRootAssoc1_I::Projection));
    }

