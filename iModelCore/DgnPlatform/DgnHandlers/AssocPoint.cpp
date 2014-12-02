/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnHandlers/AssocPoint.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

#define LOADSTR(x) DgnHandlersMessage::GetStringW(x)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   10/07
+---------------+---------------+---------------+---------------+---------------+------*/
static ElementRefP findAssocElementById (UInt64 elemId, DgnModelP modelRef, bool allowDeleted)
    {
    EditElementHandle eh;
    if (eh.FindById (ElementId(elemId), modelRef, allowDeleted) != SUCCESS)
        return NULL;
    return eh.GetElementRef();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   10/07
+---------------+---------------+---------------+---------------+---------------+------*/
static void    pushAssocPathRoot
(
DisplayPath&    path,
ElementRefP      elemRef,
DgnModelP    modelRef
)
    {
    // Include non-complex header for a complex component in path...
    ElementRefP  parent;

    if (NULL != (parent = elementRef_getOutermostParent (elemRef)))
        path.PushPathElem (parent);

    path.PushPathElem (elemRef);

    if (modelRef)
        path.SetRoot (modelRef);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   10/07
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt assoc_buildCompletePathFromRootIds (DisplayPathP path, UInt64 rootElemId, DgnModelP modelRef)
    {
    ElementRefP  elemRef;

    path->SetRoot (modelRef);
    
    if (NULL == (elemRef = findAssocElementById (rootElemId, path->GetRoot (), false)))
        return ERROR;

    pushAssocPathRoot (*path, elemRef, NULL);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   10/07
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   AssociativePoint::GetRoot
(
DisplayPathP        path,               // <=> seed display path
int*                nRootsP,            // <=  intersect assoc will return 2, all others return 1
AssocPoint const&   assocPoint,         //  => assoc point to build a displaypath for
DgnModelP        modelRef,           //  => model ref of element this assoc is from
int                 rootIndex           //  => which root to get (Use 0 for single elem assoc)
)
    {
    AssocGeom&  assoc = (AssocGeom&) assocPoint;

    if (nRootsP)
        *nRootsP = 0;

    switch (assoc.type)
        {
        case LINEAR_ASSOC:
        case PROJECTION_ASSOC:
        case ARC_ASSOC:
        case MLINE_ASSOC:
        case ORIGIN_ASSOC:
        case BSURF_ASSOC:
        case BCURVE_ASSOC:
        case MESH_VERTEX_ASSOC:
        case MESH_EDGE_ASSOC:
        case CUSTOM_ASSOC:
            {
            if (nRootsP)
                *nRootsP = 1;

            if (rootIndex > 0)
                return ERROR;

            return (BentleyStatus) assoc_buildCompletePathFromRootIds (path, assoc.singleElm.uniqueId, modelRef);
            }

        case INTERSECT_ASSOC:
        case INTERSECT2_ASSOC:
            {
            if (nRootsP)
                *nRootsP = 2;

            if (rootIndex > 1)
                return ERROR;

            return (BentleyStatus) assoc_buildCompletePathFromRootIds (path, rootIndex ? assoc.twoElm.uniqueId2 : assoc.twoElm.uniqueId1, modelRef);
            }
        }

    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/07
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus AssociativePoint::GetTargetGeometry (EditElementHandleR eeh, TransformP pathTransP, DisplayPathCP path, DgnModelP modelRef_unused)
    {
    if (!path)
        return ERROR;

    eeh.SetElementRef (path->GetTailElem ());

    if (pathTransP)
		pathTransP->InitIdentity ();

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/07
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus AssociativePoint::GetTargetGeometry (EditElementHandleR eeh, TransformP pathTransP, AssocPoint& assoc, int iRoot, DgnModelP modelRef)
    {
    DisplayPath     path;

    if (SUCCESS != AssociativePoint::GetRoot (&path, NULL, assoc, modelRef, iRoot))
        return ERROR;

    return AssociativePoint::GetTargetGeometry (eeh, pathTransP, &path, modelRef);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BrienBastings   08/01
+---------------+---------------+---------------+---------------+---------------+------*/
static int      findMatchingAssocPointIndex
(
DPoint3dCP      oldPointP,
int             oldIndex,
DPoint3dP       pointsP,
int             numPoints
)
    {
    /* Check existing index first */
    if (oldIndex < numPoints && LegacyMath::RpntEqual (oldPointP, &pointsP[oldIndex]))
        {
        return oldIndex;
        }
    else
        {
        int     i;

        for (i = 0; i < numPoints; i++)
            {
            if (i != oldIndex && LegacyMath::RpntEqual (oldPointP, &pointsP[i]))
                return i;
            }
        }

    return -1;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    RBB             1/91
+---------------+---------------+---------------+---------------+---------------+------*/
static bool     intersectionAssocPossible (ElementHandleCR eh)
    {
    return (NULL != dynamic_cast <ICurvePathQuery*> (&eh.GetHandler ())); // includes mline...
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    RBB             10/90
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt   intersectionAssocGeometry
(
DPoint3dR       outPoint,       // <= output point  
AssocGeom&      assoc,          // => geometric info
ElementHandleCR    eh1,            // => first element 
ElementHandleCR    eh2,            // => element       
TransformCP     pathTrans1,
TransformCP     pathTrans2,
DgnModelP    modelRef        // => parent model
)
    {
    if (!intersectionAssocPossible (eh1) || !intersectionAssocPossible (eh2))
        return BAD_ASSOCIATION;

    if (INTERSECT2_ASSOC != assoc.type)
        return ElementUtil::GetIntersectionPointByIndex (outPoint, eh1, eh2, pathTrans1, pathTrans2, assoc.intersect.index);

    if (assoc.intersect2.seg1 >= 0 && assoc.intersect2.nSeg1 && assoc.intersect2.nSeg1 != LineStringUtil::GetApparentCount (eh1))
        return ASSOC_TOPO_CHANGE;

    if (assoc.intersect2.seg2 >= 0 && assoc.intersect2.nSeg2 && assoc.intersect2.nSeg2 != LineStringUtil::GetApparentCount (eh2))
        return ASSOC_TOPO_CHANGE;

    EditElementHandle  segEh1, segEh2;
    
    ElementUtil::GetSegment (segEh1, eh1, assoc.intersect2.seg1);
    ElementUtil::GetSegment (segEh2, eh2, assoc.intersect2.seg2);

    return ElementUtil::GetIntersectionPointByIndex (outPoint, segEh1, segEh2, pathTrans1, pathTrans2, assoc.intersect2.index);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    JVB             12/90
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   AssociativePoint::GetAssocPointSegment
(
DSegment3dR     segment,
AssocPoint&     assocPoint,
DgnModelP    parentModel
)
    {
    AssocGeom&  assoc = (AssocGeom&) assocPoint;
    int         pointNo;

    switch (assoc.type)
        {
        case LINEAR_ASSOC:
            pointNo = assoc.line.vertex;
            break;

        case MLINE_ASSOC:
            pointNo = assoc.mline.pointNo;
            break;

        case PROJECTION_ASSOC:
            pointNo = assoc.projection.vertex;
            break;

        default:
            return ERROR;
        }

    Transform           pathTrans;
    EditElementHandle   eeh;

    if (SUCCESS != AssociativePoint::GetTargetGeometry (eeh, &pathTrans, assocPoint, 0, parentModel))
        return ERROR;

    int         nPoints = LineStringUtil::GetApparentCount (eeh);

    if (pointNo == nPoints-1)
        pointNo --;
    else if (pointNo > nPoints-1)
        return ERROR;

    Handler*    handler = &eeh.GetHandler ();

    IMultilineQuery*    mlineQuery;

    if (NULL != (mlineQuery = dynamic_cast <IMultilineQuery*> (handler)))
        {
        size_t      nPoints;

        if (SUCCESS != mlineQuery->ExtractPoints (eeh, NULL, nPoints, 0))
            return ERROR;

        DPoint3dP   points = (DPoint3dP) _alloca (nPoints * sizeof (DPoint3d));

        mlineQuery->ExtractPoints (eeh, points, nPoints, nPoints);

        segment.init (points + pointNo, points + pointNo + 1);

        MultilineProfilePtr profile = mlineQuery->GetProfile (eeh, assoc.mline.b.lineNo);
        double      dist = profile->GetDistance ();

        if (0.0 != dist)
            {
            DVec3d  perpVec, parVec, zVec;

            eeh.GetDisplayHandler ()->IsPlanar (eeh, &zVec, NULL, NULL);

            parVec.DifferenceOf (segment.point[1], segment.point[0]);
            perpVec.CrossProduct (zVec, parVec);
            perpVec.Normalize ();

            segment.point[0].SumOf (segment.point[0], perpVec, dist);
            segment.point[1].SumOf (segment.point[1], perpVec, dist);
            }

        pathTrans.multiply (segment.point, 2);

        return SUCCESS;
        }

    CurveVectorPtr pathCurve = ICurvePathQuery::ElementToCurveVector (eeh);

    if (pathCurve.IsNull ())
        return ERROR;

    ICurvePrimitivePtr& pathMember = pathCurve->front ();

    switch (pathCurve->HasSingleCurvePrimitive ())
        {
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line:
            {
            segment = *pathMember->GetLineCP ();
            break;
            }

        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString:
            {
            bvector<DPoint3d> const* points = pathMember->GetLineStringCP ();

            segment.init (&points->at (pointNo), &points->at (pointNo + 1));
            break;
            }

        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_PointString:
            {
            bvector<DPoint3d> const* points = pathMember->GetPointStringCP ();

            segment.init (&points->at (pointNo), &points->at (pointNo + 1));
            break;
            }

        default:
            return ERROR;
        }

    pathTrans.multiply (segment.point, 2);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BrienBastings   01/01
+---------------+---------------+---------------+---------------+---------------+------*/
void            AssociativePoint::InValidateRoots (AssocPoint& assocPoint)
    {
    AssocGeom&  assoc = (AssocGeom&) assocPoint;

    switch (assoc.type)
        {
        case INTERSECT_ASSOC:
        case INTERSECT2_ASSOC:
            assoc.twoElm.uniqueId1 = INVALID_ELEMENTID;
            assoc.twoElm.uniqueId2 = INVALID_ELEMENTID;
            assoc.twoElm.___legacyref1 = 0;
            assoc.twoElm.___legacyref2 = 0;
            break;

        default:
            assoc.singleElm.uniqueId = INVALID_ELEMENTID;
            assoc.singleElm.___legacyref = 0;
            break;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BrienBastings   01/01
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   AssociativePoint::HandleTopologyChange
(
bool*           changedP,       // <= if assoc point was changed
DPoint3dP       newPointP,      // <= Point to look for         
DPoint3dCP      oldPointP,      // => Point to look for         
AssocPoint&     assocPoint,     // => Association information   
DgnModelP    modelRef        // => source of element         
)
    {
    BentleyStatus   status = ERROR;
    AssocGeom&      assoc = (AssocGeom&) assocPoint;

    if (changedP)
        *changedP = false;

    switch (assoc.type)
        {
        case LINEAR_ASSOC:
        case PROJECTION_ASSOC:
            {
            Transform           pathTrans;
            EditElementHandle   eeh;

            if (SUCCESS != AssociativePoint::GetTargetGeometry (eeh, &pathTrans, assocPoint, 0, modelRef))
                return ERROR;

            CurveVectorPtr pathCurve = ICurvePathQuery::ElementToCurveVector (eeh);

            if (pathCurve.IsNull ())
                return ERROR;

            bvector<DPoint3d> points;

            ICurvePrimitivePtr& pathMember = pathCurve->front ();

            switch (pathCurve->HasSingleCurvePrimitive ())
                {
                case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line:
                    {
                    DSegment3d  segment = *pathMember->GetLineCP ();

                    points.push_back (segment.point[0]);
                    points.push_back (segment.point[1]);
                    break;
                    }

                case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString:
                    {
                    points = *pathMember->GetLineStringCP ();
                    break;
                    }

                case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_PointString:
                    {
                    points = *pathMember->GetPointStringCP ();
                    break;
                    }

                default:
                    return ERROR;
                }

            UShort     nVertex = (UShort) points.size ();
            int        newVertex = -1;

            pathTrans.multiply (&points.front (), nVertex);

            if (0 <= (newVertex = findMatchingAssocPointIndex (oldPointP, assoc.type == LINEAR_ASSOC ? assoc.line.vertex : assoc.projection.vertex, &points.front (), nVertex)))
                {
                bool        isLastPoint = (newVertex && newVertex == nVertex-1);

                if (assoc.type == LINEAR_ASSOC)
                    {
                    assoc.line.nVertex        = nVertex;
                    assoc.line.vertex         = static_cast <UShort> (isLastPoint ? newVertex-1 : newVertex);
                    assoc.line.numerator      = isLastPoint ? 1 : 0;
                    assoc.line.divisor        = 1;
                    }
                else
                    {
                    assoc.projection.nVertex  = nVertex;
                    assoc.projection.vertex   = static_cast <UShort> (isLastPoint ? newVertex-1 : newVertex);
                    assoc.projection.ratioVal = isLastPoint ? 1.0 : 0.0;
                    }

                *newPointP = points.at (newVertex);

                status = SUCCESS;
                }
            else
                {
                AssociativePoint::InValidateRoots (assocPoint);

                status = ERROR;
                }

            if (changedP)
                *changedP = true;

            break;
            }

        case MLINE_ASSOC:
        case INTERSECT_ASSOC:
        case INTERSECT2_ASSOC:
            {
            AssociativePoint::InValidateRoots (assocPoint);

            status = ERROR;

            if (changedP)
                *changedP = true;
            break;
            }

        default:
            status = ERROR;
            break;
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    JVB             12/90
+---------------+---------------+---------------+---------------+---------------+------*/
Public StatusInt    AssociativePoint::RemoveAllAssociations (EditElementHandleR element)
    {
    StatusInt   status = ERROR;
    DgnElementP  elm = element.GetElementP ();

    switch (elm->GetLegacyType())
        {
        case DIMENSION_ELM:
            {
            for (int i=0; i<elm->ToDimensionElm().nPoints; i++)
                {
                if (elm->ToDimensionElm().GetDimTextCP(i)->flags.b.associative)
                    {
                    elm->ToDimensionElmR().GetDimTextP(i)->flags.b.associative = false;
                    status = SUCCESS;
                    }
                }
            break;
            }

        case MULTILINE_ELM:
            {
            MlinePoint* mPoint = NULL;

            mPoint = ((MlinePoint*)(((MlineElm *) elm)->profile + ((MlineElm *) elm)->nLines));

            for (int i=0; i<elm->ToMlineElm().nPoints; i++, mPoint++)
                {
                if (mPoint->flags.assoc)
                    {
                    mPoint->flags.assoc = false;
                    status = SUCCESS;
                    }
                }
            break;
            }

        case SHARED_CELL_ELM:
            {
            if (elm->ToSharedCell().m_override.assocPnt)
                {
                elm->ToSharedCellR().m_override.assocPnt = false;
                status = SUCCESS;
                }
            break;
            }

        #ifdef REMOVED_IN_GRAPHITE
        case ATTRIBUTE_ELM:
            {
            if (elm->ToAttributeElm().flags & ATTR_FLAG_ASSOC)
                {
                if (elm->ToAttributeElm().flags & ATTR_FLAG_OFFSET)
                    {
                    bsiDPoint3d_addDPoint3dDPoint3d (&elm->ToAttributeElmR().origin, &elm->ToAttributeElmR().origin, (DVec3dCP) &elm->ToAttributeElm().offset);
                    memset (&elm->ToAttributeElmR().offset, 0, sizeof (elm->ToAttributeElm().offset));

                    elm->ToAttributeElmR().flags &= ~ATTR_FLAG_OFFSET;
                    }

                elm->ToAttributeElmR().flags &= ~ATTR_FLAG_ASSOC;

                /* No point keeping reference to tag set...tag is now invalid */
                DependencyManagerLinkage::DeleteLinkage (element, DEPENDENCYAPPID_TagSetDef, 0);
                status = SUCCESS;
                }
            break;
            }
        #endif
        }

    if (0 != DependencyManagerLinkage::DeleteLinkage (element, DEPENDENCYAPPID_MicroStation, DEPENDENCYAPPVALUE_AssocPoint))
        status = SUCCESS;

    return status;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    JVB             8/90
+---------------+---------------+---------------+---------------+---------------+------*/
Public StatusInt    AssociativePoint::GetPointFromElement
(
DPoint3d        *outPoint, 
ElementHandleCR    element,
int             pointNum,
int             maxPoints,
DgnModelP    modelRef
)
    {
    AssocPoint      assocPt;

    if (SUCCESS == AssociativePoint::ExtractPoint (assocPt, element, pointNum, maxPoints))
        return AssociativePoint::GetPoint (outPoint, assocPt, modelRef);

    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    JoshSchifter    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
Public StatusInt    AssociativePoint::RemovePoint
(
EditElementHandleR element,
int             pointNum,
int             maxPoints
)
    {
    DependencyLinkageAccessor pOriginalLink;

    if (SUCCESS != DependencyManagerLinkage::GetLinkage (&pOriginalLink, element, DEPENDENCYAPPID_MicroStation, DEPENDENCYAPPVALUE_AssocPoint) ||
        DEPENDENCY_DATA_TYPE_ASSOC_POINT_I != pOriginalLink->u.f.rootDataType)
        return ERROR;

    int                 iRoot;
    bool                bFoundIndex = false;
    DependencyLinkage  *pReplaceLink;
    size_t              bufferSize = DependencyManagerLinkage::GetSizeofLinkage (*pOriginalLink, 0); 

    /* new linkage will always be smaller than original */
    if (NULL == (pReplaceLink = (DependencyLinkage *) _alloca (bufferSize)))
        return ERROR;

    memset (pReplaceLink, 0, bufferSize);

    pReplaceLink->appID      = pOriginalLink->appID;
    pReplaceLink->appValue   = pOriginalLink->appValue;
    pReplaceLink->u.flags    = pOriginalLink->u.flags;

    /* move each root to the new linkage, except the one being removed */
    for (iRoot=0; iRoot < pOriginalLink->nRoots; iRoot++)
        {
        if (pOriginalLink->root.a_i[iRoot].i == pointNum)
            {
            bFoundIndex = true;
            }
        else
            {
            pReplaceLink->root.a_i[pReplaceLink->nRoots].i     =  pOriginalLink->root.a_i[iRoot].i;
            pReplaceLink->root.a_i[pReplaceLink->nRoots].i2    =  pOriginalLink->root.a_i[iRoot].i2;
            pReplaceLink->root.a_i[pReplaceLink->nRoots].assoc =  pOriginalLink->root.a_i[iRoot].assoc;
            pReplaceLink->nRoots++;
            }
        }

    StatusInt status = ERROR;
    if (bFoundIndex)
        {
        if (0 < pReplaceLink->nRoots)
            status = DependencyManagerLinkage::UpdateLinkage (element, *pReplaceLink, 0);
        else
            status = (0 < DependencyManagerLinkage::DeleteLinkage (element,
                                                       DEPENDENCYAPPID_MicroStation,
                                                       DEPENDENCYAPPVALUE_AssocPoint)) ? SUCCESS : ERROR;
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BrienBastings   01/01
+---------------+---------------+---------------+---------------+---------------+------*/
Public StatusInt    AssociativePoint::InsertPoint
(
EditElementHandleR     element,
AssocPoint const&   assoc, 
int                 pointNum,
int                 maxPoints
)
    {
    DependencyLinkage   *depLinkageP;
    int const           bufferSize = offsetof (DependencyLinkage, root) + 7 + (sizeof (DependencyRootAssocPoint_I) * maxPoints);

    if (pointNum < 0 || pointNum > maxPoints)
        return ERROR;

    if (NULL != (depLinkageP = (DependencyLinkage *) _alloca (bufferSize)))
        {
        int     expectedType = DEPENDENCY_DATA_TYPE_ASSOC_POINT_I;

        if (0 == pointNum && 1 == maxPoints)
            {
            switch (element.GetLegacyType())
                {
        #ifdef REMOVED_IN_GRAPHITE
                case ATTRIBUTE_ELM:
        #endif
                case SHARED_CELL_ELM:
                case CELL_HEADER_ELM:
                    {
                    expectedType = DEPENDENCY_DATA_TYPE_ASSOC_POINT;
                    break;
                    }
                }
            }

        DependencyLinkageAccessor oldDepP;

        if (SUCCESS == DependencyManagerLinkage::GetLinkage (&oldDepP, element, DEPENDENCYAPPID_MicroStation, DEPENDENCYAPPVALUE_AssocPoint))
            {
            size_t depSz = DependencyManagerLinkage::GetSizeofLinkage(*oldDepP, 0) - sizeof(LinkageHeader);
            memcpy (depLinkageP, oldDepP, MIN(depSz,(size_t)bufferSize));

            DependencyManagerLinkage::DeleteLinkage (element, DEPENDENCYAPPID_MicroStation, DEPENDENCYAPPVALUE_AssocPoint);

            /* If not the right type this linkage is deleted and we add a new one */
            if (depLinkageP->u.f.rootDataType == expectedType)
                {
                if (expectedType == DEPENDENCY_DATA_TYPE_ASSOC_POINT_I)
                    {
                    int         rootIndex = 0;
                    bool        bFoundIndex = false;

                    /* Update existing root with this point number */
                    for (rootIndex=0; rootIndex<depLinkageP->nRoots; rootIndex++)
                        {
                        if (depLinkageP->root.a_i[rootIndex].i == pointNum)
                            {
                            depLinkageP->root.a_i[rootIndex].assoc = assoc;
                            bFoundIndex = true;
                            }
                        }

                    /* Not found insert as new point */
                    if (!bFoundIndex)
                        {
                        depLinkageP->root.a_i[depLinkageP->nRoots].assoc = assoc;
                        depLinkageP->root.a_i[depLinkageP->nRoots].i     = pointNum;
                        depLinkageP->root.a_i[depLinkageP->nRoots].i2    = 0;
                        depLinkageP->nRoots++;
                        }
                    }
                else
                    {
                    depLinkageP->root.assoc[0] = assoc;
                    }

                /* If the user set a new point, assume valid */
                depLinkageP->u.f.invalid = 0;
                return DependencyManagerLinkage::AppendLinkage (element, *depLinkageP, 0);
                }
            }

        DependencyManagerLinkage::InitLinkage (*depLinkageP, DEPENDENCYAPPID_MicroStation, expectedType, DEPENDENCY_ON_COPY_RemapRootsWithinSelection);

        depLinkageP->appValue           = DEPENDENCYAPPVALUE_AssocPoint;
        depLinkageP->nRoots             = 1;

        /* Make sure far ref root elm deleted with this element */
        depLinkageP->u.f.deleteRoots    = true;

        if (expectedType == DEPENDENCY_DATA_TYPE_ASSOC_POINT_I)
            {
            depLinkageP->root.a_i[0].assoc = assoc;
            depLinkageP->root.a_i[0].i     = pointNum;
            depLinkageP->root.a_i[0].i2    = 0;
            }
        else
            {
            depLinkageP->root.assoc[0] = assoc;
            }

        return DependencyManagerLinkage::AppendLinkage (element, *depLinkageP, 0);
        }

    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BrienBastings   07/01
+---------------+---------------+---------------+---------------+---------------+------*/
Public StatusInt    AssociativePoint::VertexAddedOrRemoved
(
EditElementHandleR element,
int             pointNum,
int             maxPoints,
bool            inserted
)
    {
    if (inserted)
        {
        /* Nothing to do if point being inserted at the end */
        if (pointNum >= maxPoints)
            return SUCCESS;
        }
    else
        {
        /* If this is the only assoc point left this will remove the linkage */
        AssociativePoint::RemovePoint (element, pointNum, maxPoints);
        }

    DependencyLinkageAccessor oldDepP;

    if (SUCCESS != DependencyManagerLinkage::GetLinkage (&oldDepP, element, DEPENDENCYAPPID_MicroStation, DEPENDENCYAPPVALUE_AssocPoint))
        return SUCCESS;

    /* If not the right type this linkage is deleted and we add a new one */
    if (oldDepP->u.f.rootDataType != DEPENDENCY_DATA_TYPE_ASSOC_POINT_I)
        return SUCCESS;

    DependencyLinkage   *depLinkageP;
    int const            bufferSize = offsetof (DependencyLinkage, root) + 7 + (sizeof (DependencyRootAssocPoint_I) * maxPoints);

    if (NULL == (depLinkageP = (DependencyLinkage *) _alloca (bufferSize)))
        return ERROR;

    size_t depSz = DependencyManagerLinkage::GetSizeofLinkage (*oldDepP, 0) - sizeof(LinkageHeader);
    memcpy (depLinkageP, oldDepP, MIN(depSz,(size_t)bufferSize));

    DependencyManagerLinkage::DeleteLinkage (element, DEPENDENCYAPPID_MicroStation, DEPENDENCYAPPVALUE_AssocPoint);

    /* Update existing root with point numbers greater than insertion point */
    for (int rootIndex=0; rootIndex<depLinkageP->nRoots; rootIndex++)
        {
        if (depLinkageP->root.a_i[rootIndex].i >= pointNum)
            depLinkageP->root.a_i[rootIndex].i += inserted ? 1 : -1;
        }

    return DependencyManagerLinkage::AppendLinkage (element, *depLinkageP, 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BrienBastings   07/01
+---------------+---------------+---------------+---------------+---------------+------*/
Public StatusInt    AssociativePoint::VertexAddedOrRemovedFromMSElement
(
DgnElementR     elem,
DgnModelR       model,
int             pointNum,
int             maxPoints,
bool            inserted
)
    {
    EditElementHandle eh (elem, model);
    if (AssociativePoint::VertexAddedOrRemoved (eh, pointNum, maxPoints, inserted) != SUCCESS)
        return ERROR;
    eh.GetElementCP()->CopyTo (elem);
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BrienBastings   01/01
+---------------+---------------+---------------+---------------+---------------+------*/
Public StatusInt    AssociativePoint::ExtractPoint
(
AssocPoint&     assoc,
ElementHandleCR    element,
int             pointNum,
int             maxPoints
)
    {
    DependencyLinkageAccessor depLinkageP;

    if (SUCCESS != DependencyManagerLinkage::GetLinkage (&depLinkageP, element, DEPENDENCYAPPID_MicroStation, DEPENDENCYAPPVALUE_AssocPoint))
        return ERROR;

    if (depLinkageP->u.f.invalid)
        return ERROR;

    switch (depLinkageP->u.f.rootDataType)
        {
        case DEPENDENCY_DATA_TYPE_ASSOC_POINT_I:
            {
            // Find root with this point number
            for (int rootIndex=0; rootIndex<depLinkageP->nRoots; rootIndex++)
                {
                if (depLinkageP->root.a_i[rootIndex].i == pointNum)
                    {
                    memcpy (&assoc, &depLinkageP->root.a_i[rootIndex].assoc, sizeof (assoc));

                    return SUCCESS;
                    }
                }
            
            break;
            }

        case DEPENDENCY_DATA_TYPE_ASSOC_POINT:
            {
            if (pointNum == 0 && pointNum < depLinkageP->nRoots)
                {
                memcpy (&assoc, &depLinkageP->root.assoc[pointNum], sizeof (assoc));

                return SUCCESS;
                }

            break;
            }
        }

    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BrienBastings   07/01
+---------------+---------------+---------------+---------------+---------------+------*/
static void     textAssocControlPointFromShape
(
DPoint3d        *outPoint,      /* <= output point */
DPoint3d        *points,
DPoint3d        *userOrigin,
int             controlPt
)
    {
    switch (controlPt)
        {
        case 1:     // Control points:  2 - 6 - 3
        case 2:     //                  5 - 9 - 7
        case 3:     //                  1 - 8 - 4
        case 4:
            *outPoint = points[controlPt-1];
            break;

        case 5:
            bsiDPoint3d_interpolate (outPoint, points,   0.5, points+1);
            break;

        case 6:
            bsiDPoint3d_interpolate (outPoint, points+1, 0.5, points+2);
            break;

        case 7:
            bsiDPoint3d_interpolate (outPoint, points+2, 0.5, points+3);
            break;

        case 8:
            bsiDPoint3d_interpolate (outPoint, points+3, 0.5, points);
            break;

        case 9:
            bsiDPoint3d_interpolate (outPoint, points,   0.5, points+2);
            break;

        default:
            *outPoint = *userOrigin;
            break;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   11/09
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt evaluateOriginAssoc (DPoint3dP outPoint, AssocGeom* assocP, ElementHandleCR eh)
    {
    if (assocP->origin.option)
        {
        ITextQuery*     textQuery;

        if (NULL != (textQuery = dynamic_cast <ITextQuery*> (&eh.GetHandler ())) && textQuery->IsTextElement (eh))
            {
            DPoint3d    pts[5], userOrigin;

            if (SUCCESS != ElementUtil::ExtractTextShape (eh, pts, userOrigin))
                return ERROR;

            textAssocControlPointFromShape (outPoint, pts, &userOrigin, assocP->origin.option);

            return SUCCESS;
            }
        }

    /* NOTE: The proper thing is to call GetSnapOrigin...but the default on DisplayHandler
             returns the range center instead of lower left however...could be ok; change
             only affects non-curve types...and those must not match the snapping code now?!? */
    DisplayHandlerP dHandler = eh.GetDisplayHandler ();

    if (!dHandler)
        return BAD_ASSOCIATION;

    dHandler->GetSnapOrigin (eh, *outPoint);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   11/09
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt evaluateArcAssoc (DPoint3dP outPoint, AssocGeom* assocP, ElementHandleCR eh)
    {
    CurveVectorPtr pathCurve = ICurvePathQuery::ElementToCurveVector (eh);

    if (pathCurve.IsNull () || ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc != pathCurve->HasSingleCurvePrimitive ())
        return BAD_ASSOCIATION;

    DEllipse3d   ellipse = *pathCurve->front ()->GetArcCP ();

    switch (assocP->arc.keyPoint)
        {
        case AssociativePoint::ARC_CENTER:
            {
            *outPoint = ellipse.center;
            break;
            }

        case AssociativePoint::ARC_START:
            {
            DPoint3d    endPt;

            ellipse.EvaluateEndPoints (*outPoint, endPt);
            break;
            }

        case AssociativePoint::ARC_END:
            {
            DPoint3d    startPt;

            ellipse.EvaluateEndPoints (startPt, *outPoint);
            break;
            }

        default:
            {
            ellipse.Evaluate (*outPoint, assocP->arc.angle);
            break;
            }
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   11/09
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt evaluateLinearAssoc (DPoint3dP outPoint, AssocGeom* assocP, ElementHandleCR eh)
    {
    int         vertex, nVertex;
    double      fraction;
    DPoint3d    point1, point2;

    vertex  = assocP->type == LINEAR_ASSOC ? assocP->line.vertex  : assocP->projection.vertex;
    nVertex = assocP->type == LINEAR_ASSOC ? assocP->line.nVertex : assocP->projection.nVertex;

    CurveVectorPtr pathCurve = ICurvePathQuery::ElementToCurveVector (eh);

    if (pathCurve.IsNull ())
        return ERROR;

    ICurvePrimitivePtr& pathMember = pathCurve->front ();

    switch (pathCurve->HasSingleCurvePrimitive ())
        {
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line:
            {
            DSegment3d  segment = *pathMember->GetLineCP ();

            point1 = segment.point[0];
            point2 = segment.point[1];

            if (nVertex && nVertex != (LegacyMath::RpntEqual (&segment.point[0], &segment.point[1]) ? 1 : 2))
                return ASSOC_TOPO_CHANGE;

            if (vertex)
                point1 = point2;

            break;
            }

        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString:
            {
            bvector<DPoint3d> const* points = pathMember->GetLineStringCP ();

            if (vertex >= 0 && nVertex && nVertex != points->size ())
                return ASSOC_TOPO_CHANGE;

            if (vertex >= (int) (points->size ()-1) || vertex < 0)
                return BAD_ASSOCIATION;

            point1 = points->at (vertex);
            point2 = points->at (vertex + 1);
            break;
            }

        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_PointString:
            {
            bvector<DPoint3d> const* points = pathMember->GetPointStringCP ();

            if (vertex >= 0 && nVertex && nVertex != points->size ())
                return ASSOC_TOPO_CHANGE;

            if (vertex >= (int) (points->size ()-1) || vertex < 0)
                return BAD_ASSOCIATION;

            point1 = points->at (vertex);
            point2 = points->at (vertex + 1);
            break;
            }

        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_BsplineCurve:
            {
            // NOTE: Linear assoc used to associative to control poly, NEVER the curve...
            MSBsplineCurveCP curve = pathMember->GetBsplineCurveCP ();
        
            int     numPoles = curve->params.numPoles;

            point1 = curve->GetPole (vertex);
            point2 = curve->GetPole(vertex + 1);

            if (vertex >= 0 && nVertex && nVertex != numPoles)
                return ASSOC_TOPO_CHANGE;

            if (vertex >= numPoles-1 || vertex < 0)
                return BAD_ASSOCIATION;

            break;
            }

        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_InterpolationCurve:
            {
            // NOTE: Linear assoc used to associative to control poly, NEVER the curve...
            MSInterpolationCurveCP  fitCurve = pathMember->GetInterpolationCurveCP ();
        
            int     numPoints = fitCurve->params.numPoints;

            point1 = fitCurve->fitPoints[vertex];
            point2 = fitCurve->fitPoints[vertex];

            if (vertex >= 0 && nVertex && nVertex != numPoints+2)
                return ASSOC_TOPO_CHANGE;

            if (vertex >= numPoints || vertex < 0)
                return BAD_ASSOCIATION;

            break;
            }

        default:
            return BAD_ASSOCIATION;
        }

    if (LINEAR_ASSOC == assocP->type)
        fraction = (double) assocP->line.numerator / (double) assocP->line.divisor;
    else
        fraction = assocP->projection.ratioVal;

    // output is a linear combination of the points
    outPoint->x = point1.x + fraction * (point2.x - point1.x);
    outPoint->y = point1.y + fraction * (point2.y - point1.y);
    outPoint->z = point1.z + fraction * (point2.z - point1.z);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   02/10
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt evaluateBCurveAssoc (DPoint3dR outPoint, AssocGeom* assocP, ElementHandleCR eh)
    {
    CurveVectorPtr pathCurve = ICurvePathQuery::ElementToCurveVector (eh);

    if (pathCurve.IsNull ())
        return ERROR;

    ICurvePrimitivePtr& pathMember = pathCurve->front ();

    switch (pathCurve->HasSingleCurvePrimitive ())
        {
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_BsplineCurve:
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_InterpolationCurve:
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_AkimaCurve:
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Spiral:
            pathMember->FractionToPoint (assocP->bCurve.uParam, outPoint);
            return SUCCESS;

        default:
            return ERROR;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BrienBastings   01/01
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt getAssocPoint
(
DPoint3dP       outPointP,      /* <= Point extracted from assoc.      */
AssocPoint*     assocPointP,    /* => Association information          */
DgnModelP    modelRef,       /* => source of element                */
DisplayPathCP   target1,
DisplayPathCP   target2
)
    {
    Transform       pathTrans;
    EditElementHandle  eeh;

    if (SUCCESS != AssociativePoint::GetTargetGeometry (eeh, &pathTrans, target1, modelRef))
        return ERROR;

    StatusInt   status = ERROR;
    DPoint3d    point = {0.0, 0.0, 0.0};
    AssocGeom*  assocP = (AssocGeom *) assocPointP;

    switch (assocP->type)
        {
        case LINEAR_ASSOC:
        case PROJECTION_ASSOC:
            {
            if (SUCCESS != (status = evaluateLinearAssoc (&point, assocP, eeh)))
                return status;

            pathTrans.multiply (&point);
            break;
            }

        case ARC_ASSOC:
            {
            if (SUCCESS != (status = evaluateArcAssoc (&point, assocP, eeh)))
                return status;

            pathTrans.multiply (&point);
            break;
            }

        case ORIGIN_ASSOC:
            {
            if (SUCCESS != (status = evaluateOriginAssoc (&point, assocP, eeh)))
                return status;

            pathTrans.multiply (&point);
            break;
            }

        case MLINE_ASSOC:
            {
            if (SUCCESS != (status = MultilineHandler::EvaluateMlineAssoc (eeh, *((AssocPoint*) assocP), point)))
                return status;

            pathTrans.multiply (&point);
            break;
            }

        case BCURVE_ASSOC:
            {
            if (SUCCESS != (status = evaluateBCurveAssoc (point, assocP, eeh)))
                return status;

            pathTrans.multiply (&point);
            break;
            }

        case BSURF_ASSOC:
            {
            IBsplineSurfaceQuery*   bsurfQuery;

            if (NULL == (bsurfQuery = dynamic_cast <IBsplineSurfaceQuery*> (&eeh.GetHandler ())))
                return BAD_ASSOCIATION;

            MSBsplineSurfacePtr surface;

            if (SUCCESS != bsurfQuery->GetBsplineSurface (eeh, surface))
                return BAD_ASSOCIATION;

            bspsurf_evaluateSurfacePoint (&point, NULL, NULL, NULL, assocP->bSurf.uParam, assocP->bSurf.vParam, surface.get ());
            pathTrans.multiply (&point, &point);
            break;
            }

        case MESH_VERTEX_ASSOC:
        case MESH_EDGE_ASSOC:
            {
            if (SUCCESS != (status = MeshHeaderHandler::EvaluateMeshAssoc (eeh, *assocPointP, point)))
                return BAD_ASSOCIATION;

            pathTrans.multiply (&point);
            break;
            }

        case INTERSECT_ASSOC:
        case INTERSECT2_ASSOC:
            {
            Transform       pathTrans2;
            EditElementHandle  eeh2;

            if (SUCCESS != AssociativePoint::GetTargetGeometry (eeh2, &pathTrans2, target2, modelRef))
                return ERROR;

            status = intersectionAssocGeometry (point, *assocP, eeh, eeh2, &pathTrans, &pathTrans2, modelRef);
            break;
            }

        case CUSTOM_ASSOC:
            {
            DisplayHandlerP dispHandler;

            if (NULL == (dispHandler = eeh.GetDisplayHandler ()))
                return ERROR;

            ElementHandle  pathEh (ElementId(assocP->customKeypoint.pathElementId), *modelRef);

            if (!pathEh.IsValid ())
                return ERROR;

            byte*       data;
            UInt32      size;

            // Read the custom keypoint's state data from the path element. See createCustomKeypointElement
            if (SUCCESS != AssociativePoint::CustomGetDataFromPathElement (data, size, pathEh, *assocPointP))
                return ERROR;

            if (SUCCESS != dispHandler->EvaluateCustomKeypoint (eeh, &point, data))
                return ERROR;

            pathTrans.multiply (&point);
            status = SUCCESS;
            break;
            }

        default:
            {
            status = BAD_ASSOCIATION;
            break;
            }
        }

    if (SUCCESS == status)
        {
        if (outPointP)
            *outPointP = point;
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/2007
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt getCustomAssocPoint (DisplayHandler& dh, ElementHandleCR eh, DPoint3dR outPoint, DisplayPathCP dp1, PersistentSnapPathCR snapPath)
    {
    UInt32      nbytes;
    byte const* data;

    if (SUCCESS != snapPath.GetCustomKeypointData (data, nbytes) || 0 == nbytes || NULL == data)
        return ERROR;

    if (SUCCESS != dh.EvaluateCustomKeypoint (eh, &outPoint, const_cast<byte*>(data)))
        return ERROR;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      03/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       DisplayHandler::EvaluateDefaultSnap (ElementHandleCR elHandle, DPoint3dR outPoint, DisplayPathCP dp1, DisplayPathCP dp2, AssocPoint const& assoc, PersistentSnapPathCR snapPath)
    {
    if (CUSTOM_ASSOC == ((AssocGeom&)assoc).type && 0 == ((AssocGeom&)assoc).customKeypoint.pathElementId)
        return getCustomAssocPoint (*this, elHandle, outPoint, dp1, snapPath);

    return getAssocPoint (&outPoint, const_cast<AssocPoint*>(&assoc), snapPath.GetHomeDgnModelP(), dp1, dp2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      03/07
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   AssociativePoint::GetPoint
(
DPoint3dP           outPointP,      /* <= Point extracted from assoc.      */
AssocPoint const&   assocPoint,     /* => Association information          */
DgnModelP        modelRef        /* => source of element                */
)
    {
    DPoint3d    pt;
    if (NULL == outPointP)
        outPointP = &pt;

    return (BentleyStatus) PersistentSnapPath::EvaluateAssocPoint (*outPointP, assocPoint, modelRef); // >> DisplayHandler::EvaluateSnap >> _EvaluateSnap
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BrienBastings   08/01
+---------------+---------------+---------------+---------------+---------------+------*/
Public StatusInt    AssociativePoint::GetRoot
(
ElementRefP      *elemRefP,          /* <= root elem ref */
DgnModelP    *elemDgnModelP,     /* <= root model ref */
Transform       *elemTransformP,    /* <= transform to root */
int             *nRootsP,           /* <= intersect assoc will return 2, all others return 1 */
AssocPoint&     assocPoint,         /* => assoc point to get the roots for */
DgnModelP    modelRef,           /* => model ref of element this assoc is from */
int             rootIndex           /* => which root to get (Use 0 for single elem assoc) */
)
    {
    if (elemRefP)
        *elemRefP = NULL;

    if (elemDgnModelP)
        *elemDgnModelP = NULL;

    if (elemTransformP)
        elemTransformP->InitIdentity ();

    DisplayPath     path;

    if (SUCCESS != AssociativePoint::GetRoot (&path, nRootsP, assocPoint, modelRef, rootIndex))
        return ERROR;

    if (elemRefP)
        *elemRefP = path.GetTailElem ();

    if (elemDgnModelP)
        *elemDgnModelP = path.GetRoot ();

    if (elemTransformP)
        elemTransformP->InitIdentity();

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      1/00
+---------------+---------------+---------------+---------------+---------------+------*/
WString         AssociativePoint::FormatAssocPointString
(
AssocPoint const&   assocPoint,
bool                appendIds
)
    {
    AssocGeom&  assoc = (AssocGeom&) assocPoint;
    WChar       fmt[128], tmpBuf[128];
    bool        isTwoElm = false;

    switch (assoc.type)
        {
        case LINEAR_ASSOC:
            BeStringUtilities::Snwprintf (tmpBuf, _countof(tmpBuf), LOADSTR(DgnHandlersMessage::IDS_DEPENDENCY_FMTS_LINEAR_FMT).c_str(), fmt, assoc.line.vertex,
                                         assoc.line.numerator,
                                         assoc.line.divisor);
            break;

        case PROJECTION_ASSOC:
            BeStringUtilities::Snwprintf (tmpBuf, _countof(tmpBuf), LOADSTR(DgnHandlersMessage::IDS_DEPENDENCY_FMTS_PROJECTION_FMT).c_str(), assoc.projection.vertex,
                                         assoc.projection.ratioVal);
            break;

        case ARC_ASSOC:
            if (ARC_ANGLE == assoc.arc.keyPoint)
                {
                BeStringUtilities::Snwprintf (tmpBuf, _countof(tmpBuf), LOADSTR(DgnHandlersMessage::IDS_DEPENDENCY_FMTS_ARC_ANGLE_FMT).c_str(), assoc.arc.angle);
                }
            else
                {
                DgnHandlersMessage::Number arcTypeMsg = (DgnHandlersMessage::Number)-1;

                if (ARC_CENTER == assoc.arc.keyPoint)
                    arcTypeMsg = DgnHandlersMessage::IDS_DEPENDENCY_FMTS_ARC_TYPE_CENTER;
                else
                if (ARC_START == assoc.arc.keyPoint)
                    arcTypeMsg = DgnHandlersMessage::IDS_DEPENDENCY_FMTS_ARC_TYPE_START;
                else
                if (ARC_END == assoc.arc.keyPoint)
                    arcTypeMsg = DgnHandlersMessage::IDS_DEPENDENCY_FMTS_ARC_TYPE_END;

                DgnHandlersMessage::Number msgid = DgnHandlersMessage::IDS_DEPENDENCY_FMTS_ARC_TYPE_FMT;

                if ((DgnHandlersMessage::Number)-1 != arcTypeMsg)
                    msgid = arcTypeMsg;
                else
                    tmpBuf[0] = '\0';

                BeStringUtilities::Snwprintf (tmpBuf, _countof(tmpBuf), LOADSTR(msgid).c_str(), tmpBuf);
                }

            break;

        case MLINE_ASSOC:
            BeStringUtilities::Snwprintf (tmpBuf, _countof(tmpBuf), LOADSTR(DgnHandlersMessage::IDS_DEPENDENCY_FMTS_MLINE_FMT).c_str(), assoc.mline.pointNo,
                                         assoc.mline.b.lineNo,
                                         assoc.mline.offsetVal);
            break;

        case MESH_VERTEX_ASSOC:
            BeStringUtilities::Snwprintf (tmpBuf, _countof(tmpBuf), LOADSTR(DgnHandlersMessage::IDS_DEPENDENCY_FMTS_MESH_VERTEX_FMT).c_str(), assoc.meshVertex.vertexIndex);
            break;

        case MESH_EDGE_ASSOC:
            BeStringUtilities::Snwprintf (tmpBuf, _countof(tmpBuf), LOADSTR(DgnHandlersMessage::IDS_DEPENDENCY_FMTS_MESH_EDGE_FMT).c_str(), assoc.meshEdge.edgeIndex,
                                         assoc.meshEdge.uParam);
            break;

        case ORIGIN_ASSOC:
            BeStringUtilities::Snwprintf (tmpBuf, _countof(tmpBuf), LOADSTR(DgnHandlersMessage::IDS_DEPENDENCY_FMTS_ORIGIN_FMT).c_str());
            break;

        case BCURVE_ASSOC:
            BeStringUtilities::Snwprintf (tmpBuf, _countof(tmpBuf), LOADSTR(DgnHandlersMessage::IDS_DEPENDENCY_FMTS_BCURVE_FMT).c_str(), assoc.bCurve.uParam);
            break;

        case BSURF_ASSOC:
            BeStringUtilities::Snwprintf (tmpBuf, _countof(tmpBuf), LOADSTR(DgnHandlersMessage::IDS_DEPENDENCY_FMTS_BSURF_FMT).c_str(), assoc.bSurf.uParam,
                                         assoc.bSurf.vParam);
            break;

        case INTERSECT_ASSOC:
            isTwoElm = true;
            BeStringUtilities::Snwprintf (tmpBuf, _countof(tmpBuf), LOADSTR(DgnHandlersMessage::IDS_DEPENDENCY_FMTS_INTERSECT_FMT).c_str(), assoc.intersect.index);
            break;

        case INTERSECT2_ASSOC:
            isTwoElm = true;
            BeStringUtilities::Snwprintf (tmpBuf, _countof(tmpBuf), LOADSTR(DgnHandlersMessage::IDS_DEPENDENCY_FMTS_INTERSECT2_FMT).c_str(), assoc.intersect2.index,
                                         assoc.intersect2.seg1,
                                         assoc.intersect2.seg2);
            break;

        default:
            tmpBuf[0] = '\0';
            break;
        }

    if (!appendIds)
        return tmpBuf;

    WString res (tmpBuf);

    if (isTwoElm)
        {
        BeStringUtilities::Snwprintf (tmpBuf, _countof(tmpBuf), LOADSTR(DgnHandlersMessage::IDS_DEPENDENCY_FMTS_TWO_ELM_FMT).c_str(), assoc.twoElm.uniqueId1,
                                                assoc.twoElm.uniqueId2);
        }
    else
        {
        BeStringUtilities::Snwprintf (tmpBuf, _countof(tmpBuf), LOADSTR(DgnHandlersMessage::IDS_DEPENDENCY_FMTS_SINGLE_ELM_FMT).c_str(), assoc.singleElm.uniqueId);
        }

    res.append (L" ");
    res.append (tmpBuf);

    return res;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2007
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   AssociativePoint::CustomGetDataFromPathElement
(
byte*&              data,
UInt32&             size,
ElementHandleCR        pathEh,
AssocPoint const&   assoc
)
    {
    if (!pathEh.IsValid ())
        {
        BeAssert (false);

        return ERROR;
        }

    LinkageHeader*  linkageP = (LinkageHeader*) elemUtil_extractLinkage (NULL, NULL, pathEh.GetElementCP(), LINKAGEID_CustomKeypoint);

    if (NULL == linkageP)
        return ERROR;

    data = (byte*)(linkageP + 1);
    size = LinkageUtil::GetWords(linkageP) * sizeof(UShort) - sizeof(*linkageP);

    return SUCCESS;
    }

