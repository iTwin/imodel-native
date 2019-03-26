/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnHandlers/RegionUtil.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <Regions/regionsAPI.h>
#include <Regions/rimsbsAPI.h>

BEGIN_BENTLEY_DGN_NAMESPACE

/*=================================================================================**//**
* @bsiclass                                                     Brien.Bastings  05/17
+===============+===============+===============+===============+===============+======*/
struct RegionData : RefCounted<IRegionData>
{
RegionGraphicsContext&  m_context;
RG_Header*              m_pRG;
RIMSBS_Context*         m_pCurves;
MTG_MarkSet             m_activeFaces;
Transform               m_worldToViewTrans = Transform::FromIdentity();
Transform               m_viewToWorldTrans = Transform::FromIdentity();
bool                    m_graphIsValid = false;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/09
+---------------+---------------+---------------+---------------+---------------+------*/
RegionData(RegionGraphicsContext& context) : m_context(context)
    {
    m_pRG = jmdlRG_new();
    m_pCurves = jmdlRIMSBS_newContext();

    jmdlRIMSBS_setupRGCallbacks(m_pCurves, m_pRG);
    jmdlRG_setFunctionContext(m_pRG, m_pCurves);
    jmdlRG_setDistanceTolerance(m_pRG, DgnUnits::OneMillimeter()); // Good tolerance???
    m_activeFaces.Attach(jmdlRG_getGraph(m_pRG), MTG_ScopeFace);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/09
+---------------+---------------+---------------+---------------+---------------+------*/
~RegionData()
    {
    if (m_pCurves)
        {
        int iCurve = 0;

        do
            {
            void* userDataP = nullptr;

            if (!jmdlRIMSBS_getUserPointer(m_pCurves, &userDataP, iCurve++))
                break;

            if (userDataP)
                free(userDataP);

            } while (true);
        }

    jmdlRIMSBS_freeContext(m_pCurves);

    // Must Attach to nullptr graph BEFORE freeing m_pRG (so Attach can release the old mask back to m_pRG)
    m_activeFaces.Attach(nullptr, MTG_ScopeFace);
    jmdlRG_free(m_pRG);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/09
+---------------+---------------+---------------+---------------+---------------+------*/
static IRegionDataPtr Create(RegionGraphicsContext& context) {return new RegionData(context);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus SetupGraph(double gapTolerance, bool mergeHoles)
    {
    m_graphIsValid = false;

    // Apply explicit flatten to plane transform to entire graph, not pushed on context since it's non-invertable...
    if (m_context.m_forcePlanar && !m_context.m_flattenTrans.IsIdentity())
        jmdlRG_multiplyByTransform(m_pRG, &m_context.m_flattenTrans);

    if (m_context.GetViewport())
        {
        m_worldToViewTrans.InitFrom(*m_context.GetViewport()->GetWorldToViewMap(), false);
        m_viewToWorldTrans.InverseOf(m_worldToViewTrans);
        }
    else if (m_context.m_forcePlanar && RegionType::Flood == m_context.m_operation)
        {
        // A non-planar graph isn't an error if we'll be post-flattening loops (flood only!)
        jmdlRG_getPlaneTransform(m_pRG, &m_viewToWorldTrans, nullptr);
        m_worldToViewTrans.InverseOf(m_viewToWorldTrans);
        }
    else
        {
        if (!jmdlRG_getPlaneTransform(m_pRG, &m_viewToWorldTrans, nullptr) || !m_worldToViewTrans.InverseOf(m_viewToWorldTrans))
            {
            m_context.m_regionError = REGION_ERROR_NonCoplanar;

            return ERROR;
            }
        }

    jmdlRG_multiplyByTransform(m_pRG, &m_worldToViewTrans);
    jmdlRG_mergeWithGapTolerance(m_pRG, gapTolerance, gapTolerance);
    jmdlRG_buildFaceRangeTree(m_pRG, 0.0, jmdlRG_getTolerance(m_pRG));

    if (mergeHoles)
        jmdlRG_buildFaceHoleArray(m_pRG);

    m_graphIsValid = true;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/09
+---------------+---------------+---------------+---------------+---------------+------*/
void GetFaceLoops(CurveVectorPtr& region, bvector<MTGNodeId>& faceNodeIds)
    {
    int         seedIndex;
    MTGNodeId   seedNodeId;

    jmdlMTGMarkSet_initIteratorIndex(&m_activeFaces, &seedIndex);

    if (!jmdlMTGMarkSet_getNextNode(&m_activeFaces, &seedIndex, &seedNodeId))
        m_context.ResetPostFlattenTransform(); // Compute new transform when there is no current active faces...

    MTG_MarkSet markSet;

    markSet.Attach(jmdlRG_getGraph(m_pRG), MTG_ScopeFace);

    // Add faces to temporary mark set so that we can share the same code for dynamic faces and accepted faces...
    for (int nodeId: faceNodeIds)
        markSet.AddNode(nodeId);

    GetMarkedRegions(region, &markSet);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/09
+---------------+---------------+---------------+---------------+---------------+------*/
void AddFaceLoop(MTGNodeId faceNodeId)
    {
    jmdlMTGMarkSet_addNode(&m_activeFaces, faceNodeId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/09
+---------------+---------------+---------------+---------------+---------------+------*/
void RemoveFaceLoop(MTGNodeId faceNodeId)
    {
    jmdlMTGMarkSet_removeNode(&m_activeFaces, faceNodeId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool ToggleFaceLoop(MTGNodeId faceNodeId)
    {
    if (jmdlMTGMarkSet_isNodeInSet(&m_activeFaces, faceNodeId))
        {
        RemoveFaceLoop(faceNodeId);

        return false;
        }

    AddFaceLoop(faceNodeId);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool IsFaceLoopSelected(MTGNodeId faceNodeId)
    {
    return TO_BOOL (jmdlMTGMarkSet_isNodeInSet(&m_activeFaces, faceNodeId));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/09
+---------------+---------------+---------------+---------------+---------------+------*/
void CollectFaceLoopsAtPoint(bvector<MTGNodeId>* faceNodeIds, DPoint3dCR seedPoint, RegionLoops floodSelect, bool stepOutOfHoles)
    {
    DPoint3d    tmpPt = seedPoint;
    MTGNodeId   seedNodeId;
    
    m_worldToViewTrans.Multiply(tmpPt);
    jmdlRG_smallestContainingFace(m_pRG, &seedNodeId, &tmpPt);

    if (stepOutOfHoles)
        {
        MTGNodeId   adjacentFaceNodeId = jmdlRG_stepOutOfHole(m_pRG, seedNodeId);

        if (MTG_NULL_NODEID != adjacentFaceNodeId)
            seedNodeId = jmdlRG_resolveFaceNodeId(m_pRG, adjacentFaceNodeId);
        }

    if (MTG_NULL_NODEID == seedNodeId)
        return;

    if (stepOutOfHoles)
        {
        double  areaToFace = 0.0;

        jmdlRG_getFaceSweepProperties(m_pRG, &areaToFace, nullptr, nullptr, seedNodeId);

        if (!(areaToFace > 0.0 || !jmdlRG_faceIsTrueExterior(m_pRG, seedNodeId)))
            return;
        }

    bvector<MTGNodeId> activeFaceNodes; 

    if (floodSelect == RegionLoops::Alternating)
        jmdlRG_collectAllNodesOnInwardParitySearch(m_pRG, &activeFaceNodes, nullptr, seedNodeId);
    else
        jmdlRG_resolveHoleNodeId(m_pRG, &activeFaceNodes, seedNodeId);

    for (size_t i = 0; i < activeFaceNodes.size(); i++)
        faceNodeIds->push_back(jmdlRG_resolveFaceNodeId(m_pRG, activeFaceNodes[i]));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus CollectBooleanFaces(RGBoolSelect boolOp, int highestOperandA, int highestOperandB)
    {
    if (!jmdlRG_collectBooleanFaces(m_pRG, boolOp, RGBoolSelect_Difference, highestOperandA, highestOperandB, &m_activeFaces))
        return ERROR;

    int       seedIndex;
    MTGNodeId seedNodeId;

    jmdlMTGMarkSet_initIteratorIndex(&m_activeFaces, &seedIndex);

    return (jmdlMTGMarkSet_getNextNode(&m_activeFaces, &seedIndex, &seedNodeId) ? SUCCESS : ERROR);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/09
+---------------+---------------+---------------+---------------+---------------+------*/
void CollectByInwardParitySearch(bool parityWithinComponent, bool vertexContactSufficient)
    {
    EmbeddedIntArray holeArray;

    jmdlRG_collectAllNodesOnInwardParitySearchExt(m_pRG, &holeArray, nullptr, MTG_NULL_NODEID, parityWithinComponent, vertexContactSufficient);

    MTGNodeId faceNodeId = -1;

    for (int i = 0; jmdlEmbeddedIntArray_getInt(&holeArray, &faceNodeId, i); i++)
        jmdlMTGMarkSet_addNode(&m_activeFaces, faceNodeId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus GetMarkedRegions(CurveVectorPtr& regionOut, MTG_MarkSet* markSet)
    {
    CurveVectorPtr baseRegion = jmdlRG_collectExtendedFaces(m_pRG, m_pCurves, markSet);

    if (!baseRegion.IsValid())
        return ERROR;

    regionOut = baseRegion;
    regionOut->TransformInPlace(m_viewToWorldTrans);

    // Flatten to plane defined by "average" geometry depth...
    if (m_context.ComputePostFlattenTransform(*regionOut))
        regionOut->TransformInPlace(m_context.m_flattenTrans);

    return regionOut.IsValid() ? SUCCESS : ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus GetActiveRegions(CurveVectorPtr& regionOut)
    {
    EmbeddedIntArray startArray;
    EmbeddedIntArray sequenceArray;

    jmdlRG_collectAndNumberExtendedFaceLoops(m_pRG, &startArray, &sequenceArray, &m_activeFaces);

    if (SUCCESS != GetMarkedRegions(regionOut, &m_activeFaces))
        return ERROR;

    // This won't affect our info ptr as it modifies linestring primitives in-place and the simplified boundary is always desirable...
    regionOut->SimplifyLinestrings(-1.0, true, true);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/13
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus GetRoots(bvector<DgnElementId>& regionRoots, ICurvePrimitiveCR curvePrimitive)
    {
    if (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_CurveVector == curvePrimitive.GetCurvePrimitiveType())
        return GetRoots(regionRoots, *curvePrimitive.GetChildCurveVectorCP());

    CurveNodeInfo const* nodeInfo = dynamic_cast <CurveNodeInfo const*> (curvePrimitive.GetCurvePrimitiveInfo().get());

    if (!nodeInfo)
        return SUCCESS;

    for (int nodeId: nodeInfo->m_nodeIds)
        {
        int     parentIndex;

        if (!jmdlRG_getParentCurveIndex(m_pRG, &parentIndex, nodeId) || parentIndex < 0)
            continue;

        void*   userDataP = nullptr; // held by context...

        if (!jmdlRIMSBS_getUserPointer(m_pCurves, &userDataP, parentIndex) || !userDataP)
            continue;

        int     userInt = 0;

        // NOTE: Dependency root order matters for assoc region re-evaluate of boolean difference...first root is geometry to subtract from!
        if (RegionType::Flood != m_context.m_operation && jmdlRIMSBS_getUserInt(m_pCurves, &userInt, parentIndex) && 1 == userInt)
            regionRoots.insert(regionRoots.begin(), *((DgnElementId*) userDataP)); 
        else
            regionRoots.push_back(*((DgnElementId*) userDataP));
        }

    return (regionRoots.empty() ? ERROR : SUCCESS);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/13
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus GetRoots(bvector<DgnElementId>& regionRoots, CurveVectorCR region)
    {
    for (ICurvePrimitivePtr curvePrimitive: region)
        {
        if (curvePrimitive.IsNull())
            continue;

        GetRoots(regionRoots, *curvePrimitive);
        }

    return (regionRoots.empty() ? ERROR : SUCCESS);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus GetRoots(bvector<DgnElementId>& regionRoots)
    {
    EmbeddedIntArray startArray;
    EmbeddedIntArray sequenceArray;

    jmdlRG_collectAndNumberExtendedFaceLoops(m_pRG, &startArray, &sequenceArray, &m_activeFaces);

    for (int iCmpn=0, nodeId=0; jmdlEmbeddedIntArray_getInt(&sequenceArray, &nodeId, iCmpn++); )
        {
        int     parentIndex;

        if (jmdlRG_getParentCurveIndex(m_pRG, &parentIndex, nodeId) && parentIndex >= 0)
            {
            void*   userDataP = nullptr;

            if (jmdlRIMSBS_getUserPointer(m_pCurves, &userDataP, parentIndex) && userDataP)
                regionRoots.push_back(*((DgnElementId*) userDataP)); // held by context...
            }
        }

    // NOTE: Cull duplicate entries. Used by DgnRegionElementTool for "process originals"...
    std::sort(regionRoots.begin(), regionRoots.end());
    regionRoots.erase(std::unique(regionRoots.begin(), regionRoots.end()), regionRoots.end());

    return (regionRoots.empty() ? ERROR : SUCCESS);
    }

}; // RegionData

END_BENTLEY_DGN_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/09
+---------------+---------------+---------------+---------------+---------------+------*/
static RGBoolSelect getRGBoolSelect(RegionType operation, bool counted = false)
    {
    switch (operation)
        {
        case RegionType::Union:
            return RGBoolSelect_Union;

        case RegionType::Intersection:
            return counted ? RGBoolSelect_CountedIntersection : RGBoolSelect_Intersection;

        case RegionType::Difference:
            return RGBoolSelect_Difference;

        default:
            return counted ? RGBoolSelect_GlobalParity : RGBoolSelect_Parity;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool RegionGraphicsContext::ComputePostFlattenTransform(CurveVectorCR curves)
    {
    if (!m_forcePlanar || 0.0 == m_flattenDir.Magnitude())
        return false;

    if (!m_flattenTrans.IsIdentity()) // Once computed same transform must be applied to all loops!
        return true;

    Transform   localToWorld, worldToLocal;
    DRange3d    localRange;
    DPoint3d    planePt = DPoint3d::From(0.0, 0.0, 0.0);
    DVec3d      planeDir = m_flattenDir;

    // If already a planar loop ignore supplied flatten direction...
    if (curves.IsPlanarWithDefaultNormal(localToWorld, worldToLocal, localRange, &m_flattenDir))
        {
        planeDir.Init(0.0, 0.0, 1.0);
        localToWorld.MultiplyMatrixOnly(planeDir);
        planeDir.Normalize();
        localToWorld.Multiply(planePt);
        }
    else
        {
        curves.GetStartPoint(planePt);
        }
            
    m_flattenTrans.InitFromProjectionToPlane(planePt, planeDir);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/09
+---------------+---------------+---------------+---------------+---------------+------*/
void RegionGraphicsContext::ResetPostFlattenTransform()
    {
    if (!m_forcePlanar || 0.0 == m_flattenDir.Magnitude())
        return;

    m_flattenTrans.InitIdentity();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/13
+---------------+---------------+---------------+---------------+---------------+------*/
void RegionGraphicsContext::AddFaceLoopsByInwardParitySearch(bool parityWithinComponent, bool vertexContactSufficient)
    {
    RegionData& data = static_cast<RegionData&> (*m_regionData);

    data.CollectByInwardParitySearch(parityWithinComponent, vertexContactSufficient);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/13
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus RegionGraphicsContext::GetRoots(bvector<DgnElementId>& regionRoots, ICurvePrimitiveCR curvePrimitive)
    {
    RegionData& data = static_cast<RegionData&> (*m_regionData);

    return data.GetRoots(regionRoots, curvePrimitive);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/13
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus RegionGraphicsContext::GetRoots(bvector<DgnElementId>& regionRoots, CurveVectorCR region)
    {
    RegionData& data = static_cast<RegionData&> (*m_regionData);

    return data.GetRoots(regionRoots, region);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus RegionGraphicsContext::GetRoots(bvector<DgnElementId>& regionRoots)
    {
    RegionData& data = static_cast<RegionData&> (*m_regionData);

    return data.GetRoots(regionRoots);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool RegionGraphicsContext::_ProcessCurvePrimitive(ICurvePrimitiveCR primitive, bool closed, bool filled, SimplifyGraphic& graphic)
    {
    Transform     placementTrans = graphic.GetLocalToWorldTransform();
    DgnElementCP  elem = (m_currentGeomSource ? m_currentGeomSource->ToElement() : nullptr);
    DgnElementId* userDataP = nullptr;

    if (nullptr != elem)
        {
        userDataP = (DgnElementId*) malloc(sizeof(DgnElementId));
        *userDataP = elem->GetElementId();
        }

    RegionData& data = static_cast<RegionData&> (*m_regionData);

    if (IsPerspectiveRegion())
        {
        bvector<DPoint3d> points;
        IFacetOptionsPtr options = IFacetOptions::CreateForCurves();

        if (!primitive.AddStrokes(points, *options))
            return true;

        placementTrans.Multiply(&points[0], (int) points.size());
        GetViewport()->GetWorldToViewMap()->M0.MultiplyAndRenormalize(points);

        for (DPoint3dR pt : points)
            pt.z = 0.0;

        graphic.GetViewContext().GetViewport()->GetWorldToViewMap()->M1.MultiplyAndRenormalize(points);

        jmdlRG_addLinear(data.m_pRG, &points[0], (int) points.size(), false, jmdlRIMSBS_addDataCarrier(data.m_pCurves, m_currentGeomMarkerId, userDataP));
        return true;
        }

    switch (primitive.GetCurvePrimitiveType())
        {
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line:
            {
            DSegment3d  segment = *primitive.GetLineCP ();

            placementTrans.Multiply(segment.point, 2);

            jmdlRG_addLinear(data.m_pRG, segment.point, 2, false, jmdlRIMSBS_addDataCarrier(data.m_pCurves, m_currentGeomMarkerId, userDataP));
            return true;
            }

        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString:
            {
            bvector<DPoint3d> points = *primitive.GetLineStringCP ();

            placementTrans.Multiply(&points[0], (int) points.size());

            jmdlRG_addLinear(data.m_pRG, &points[0], (int) points.size(), false, jmdlRIMSBS_addDataCarrier(data.m_pCurves, m_currentGeomMarkerId, userDataP));
            return true;
            }

        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc:
            {
            DEllipse3d  ellipse = *primitive.GetArcCP ();
            int         curveId;
            
            placementTrans.Multiply(ellipse);

            curveId = jmdlRIMSBS_addDEllipse3d(data.m_pCurves, m_currentGeomMarkerId, userDataP, &ellipse);
            jmdlRG_addCurve(data.m_pRG, curveId, curveId);
            return true;
            }

        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_BsplineCurve:
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_InterpolationCurve:
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_AkimaCurve:
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Spiral:
            {
            MSBsplineCurveCR  bcurve = *primitive.GetProxyBsplineCurveCP ();
            MSBsplineCurve    tmpCurve; // Requires copy of curve...do NOT free!
            int               curveId;

            tmpCurve.CopyTransformed(bcurve, placementTrans);

            curveId = jmdlRIMSBS_addMSBsplineCurve(data.m_pCurves, m_currentGeomMarkerId, userDataP, &tmpCurve);
            jmdlRG_addCurve(data.m_pRG, curveId, curveId);
            return true;
            }

        default:
            {
            if (userDataP)
                free(userDataP);

            return false;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool RegionGraphicsContext::_ProcessCurveVector(CurveVectorCR curves, bool filled, SimplifyGraphic& graphic)
    {
    RegionData& data = static_cast<RegionData&> (*m_regionData);

    if (m_restrictToPlane)
        {
        DRange3d    localRange;
        Transform   localToWorld, worldToLocal, fwdPlacementTrans, invPlacementTrans;
        DVec3d      defaultNormal = m_plane.normal;

        fwdPlacementTrans = graphic.GetLocalToWorldTransform();
        invPlacementTrans.InverseOf(fwdPlacementTrans);
        invPlacementTrans.MultiplyMatrixOnly(defaultNormal);

        if (!curves.IsPlanarWithDefaultNormal(localToWorld, worldToLocal, localRange, &defaultNormal))
            return true;

        DVec3d      planeDir = DVec3d::UnitZ();
        DPoint3d    planePt = DPoint3d::FromZero();
        Transform   planeTrans = Transform::FromProduct(fwdPlacementTrans, localToWorld);

        planeTrans.Multiply(planePt);
        planeTrans.MultiplyMatrixOnly(planeDir);
        planeDir.Normalize();

        if (fabs(planeDir.DotProduct(m_plane.normal)) < 0.99999 || fabs(m_plane.Evaluate(planePt)) > (1.0e-10 * (1.0 + m_plane.origin.Magnitude())))
            return true;
        }

    jmdlRIMSBS_setCurrGroupId(data.m_pCurves, ++m_currentGeomMarkerId);

    graphic.ProcessAsCurvePrimitives(curves, filled);

    return true;
    }

/*=================================================================================**//**
* @bsiclass                                                     Brien.Bastings  09/09
+===============+===============+===============+===============+===============+======*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  08/17
+---------------+---------------+---------------+---------------+---------------+------*/
static bool hasValidCurveGeometry(GeometrySourceCR source)
    {
    bool hasCurveGeometry = false;
    GeometryCollection collection(source);

    for (auto iter : collection)
        {
        switch (iter.GetEntryType())
            {
            case GeometryCollection::Iterator::EntryType::TextString:
                {
                // NOTE: Ignore regions that are probably used as a text frame by skipping any element that contains text.
                //       A text boundary/frame would always need to be treated as a hole, this is a bit of a pain and more 
                //       difficult since the frame is just stored as loose geometry in the GeometryStream. Going forward it's 
                //       probably better if the user makes use of display priority/z with a text background anyway.
                return false;
                }

            case GeometryCollection::Iterator::EntryType::GeometryPart:
                {
                if (hasCurveGeometry)
                    break;

                DgnGeometryPartCPtr partGeom = iter.GetGeometryPartCPtr();

                if (!partGeom.IsValid())
                    break;

                GeometryCollection partCollection(partGeom->GetGeometryStream(), partGeom->GetDgnDb());

                for (auto partIter : partCollection)
                    {
                    switch (partIter.GetEntryType())
                        {
                        case GeometryCollection::Iterator::EntryType::CurvePrimitive:
                        case GeometryCollection::Iterator::EntryType::CurveVector:
                            {
                            hasCurveGeometry = true;
                            break;
                            }
                        }

                    if (hasCurveGeometry)
                        break;
                    }
                break;
                }

            case GeometryCollection::Iterator::EntryType::CurvePrimitive:
            case GeometryCollection::Iterator::EntryType::CurveVector:
                {
                hasCurveGeometry = true;
                break;
                }
            }
        }

    return hasCurveGeometry;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/09
+---------------+---------------+---------------+---------------+---------------+------*/
RegionGraphicsContext::RegionGraphicsContext()
    {
    m_purpose = DrawPurpose::RegionFlood;
    m_regionData = RegionData::Create(*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/09
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt RegionGraphicsContext::_OutputGeometry(GeometrySourceCR source)
    {
    if (!hasValidCurveGeometry(source))
        return SUCCESS; // Avoid de-serializing BReps and other expensive geometry that will just be ignored...

    if (m_regionFilter.IsValid() && m_regionFilter->_FilterSource(source))
        return SUCCESS; // Source filtered

    m_currentGeomSource = &source;
    StatusInt status = T_Super::_OutputGeometry(source);
    m_currentGeomSource = nullptr;
    
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus RegionGraphicsContext::VisitBooleanCandidate(GeometrySourceCR element, bvector<DMatrix4d>* wireProducts, bool allowText)
    {
    CurveVectorPtr curves;
    GeometryCollection collection(element);

    for (auto iter : collection)
        {
        if (curves.IsValid())
            return ERROR; // Multiple geometric primitives...

        GeometryCollection::Iterator::EntryType entryType = iter.GetEntryType();

        switch (entryType)
            {
            case GeometryCollection::Iterator::EntryType::CurvePrimitive:
                {
                GeometricPrimitivePtr geom = iter.GetGeometryPtr();

                if (!geom.IsValid())
                    return ERROR;

                if (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_PointString == geom->GetAsICurvePrimitive()->GetCurvePrimitiveType())
                    return ERROR;

                double length = 0.0;
                DPoint3d pointS, pointE;

                if (!geom->GetAsICurvePrimitive()->GetStartEnd(pointS, pointE) || !pointS.AlmostEqual(pointE) || !geom->GetAsICurvePrimitive()->FastLength(length) || length < DoubleOps::SmallCoordinateRelTol())
                    return ERROR; // Reject zero-length lines and paths that aren'tphysically closed.

                curves = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open, geom->GetAsICurvePrimitive());
                curves->TransformInPlace(iter.GetGeometryToWorld());
                break;
                }

            case GeometryCollection::Iterator::EntryType::CurveVector:
                {
                GeometricPrimitivePtr geom = iter.GetGeometryPtr();

                if (!geom.IsValid())
                    return ERROR;

                if (!geom->GetAsCurveVector()->IsAnyRegionType() && (!geom->GetAsCurveVector()->IsPhysicallyClosedPath() || geom->GetAsCurveVector()->FastLength() < DoubleOps::SmallCoordinateRelTol()))
                    return ERROR;

                curves = geom->GetAsCurveVector()->Clone();
                curves->TransformInPlace(iter.GetGeometryToWorld());
                break;
                }

            default:
                return ERROR;
            }
        }

    if (!curves.IsValid())
        return ERROR;

    if (wireProducts) // This logic was originally added to cull redundant loops for DWG hatch creation...
        {
        DMatrix4d   productB;

        curves->ComputeSecondMomentWireProducts(productB);

        for (DMatrix4d productA: *wireProducts)
            {
            double  matrixDiff, colDiff, rowDiff, lengthDiff;

            productA.MaxAbsDiff(productB, matrixDiff, colDiff, rowDiff, lengthDiff);

            if (DoubleOps::WithinTolerance(matrixDiff, 0.0, 1.0e-12) &&
                DoubleOps::WithinTolerance(colDiff, 0.0, 1.0e-12) &&
                DoubleOps::WithinTolerance(lengthDiff, 0.0, 1.0e-12))
                return ERROR; // Duplicate found...
            }

        wireProducts->push_back(productB);
        }

    Render::GraphicBuilderPtr builder = CreateSceneGraphic();
    SimplifyGraphic* graphic = static_cast<SimplifyGraphic*> (builder.get());

    m_currentGeomSource = &element;
    _ProcessCurveVector(*curves, false, *graphic);
    m_currentGeomSource = nullptr;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/09
+---------------+---------------+---------------+---------------+---------------+------*/
void RegionGraphicsContext::SetFloodParams(RegionLoops regionLoops, double gapTolerance, bool stepOutOfHoles)
    {
    m_regionLoops = regionLoops;
    m_gapTolerance = gapTolerance;
    m_stepOutOfHoles = stepOutOfHoles;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/09
+---------------+---------------+---------------+---------------+---------------+------*/
void RegionGraphicsContext::SetFlattenBoundary(TransformCR flattenTrans)
    {
    m_forcePlanar = true;
    m_applyPerspective = false;
    m_flattenTrans = flattenTrans;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/09
+---------------+---------------+---------------+---------------+---------------+------*/
void RegionGraphicsContext::SetFlattenBoundary(DVec3dCR flattenDir)
    {
    m_forcePlanar = true;
    m_applyPerspective = false;
    m_flattenDir = flattenDir;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/17
+---------------+---------------+---------------+---------------+---------------+------*/
void RegionGraphicsContext::SetPerspectiveFlatten(bool applyPerspective)
    {
    m_applyPerspective = applyPerspective;

    if (applyPerspective)
        m_forcePlanar = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool RegionGraphicsContext::IsPerspectiveRegion()
    {
    return (m_applyPerspective && GetViewport() && GetViewport()->IsCameraOn());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/09
+---------------+---------------+---------------+---------------+---------------+------*/
void RegionGraphicsContext::InitRegionParams(RegionParams& params)
    {
    params.SetType(m_operation);
    params.SetFloodParams(m_regionLoops, m_gapTolerance);
    params.SetAssociative(true);

    if (m_forcePlanar)
        {
        RotMatrix flatten;

        m_flattenTrans.GetMatrix(flatten);
        params.SetFlattenBoundary(true, &flatten);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool RegionGraphicsContext::GetAdjustedSeedPoints(bvector<DPoint3d>* seedPoints)
    {
    if (RegionType::Flood != m_operation)
        return false;

    for (size_t iSeed = 0; iSeed < m_floodSeeds.size(); iSeed++)
        {
        DPoint3d tmpPt = m_floodSeeds[iSeed].m_pt;

        if (m_forcePlanar) // project seeds into plane of region...
            m_flattenTrans.Multiply(tmpPt);

        seedPoints->push_back(tmpPt);
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/13
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus RegionGraphicsContext::GetRegion(CurveVectorPtr& region)
    {
    RegionData& data = static_cast<RegionData&> (*m_regionData);
    
    return data.GetActiveRegions(region);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/12
+---------------+---------------+---------------+---------------+---------------+------*/
int RegionGraphicsContext::GetCurrentFaceNodeId()
    {
    return (0 != m_dynamicFaceSeed.m_faceNodeIds.size() ? m_dynamicFaceSeed.m_faceNodeIds[0] : 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool RegionGraphicsContext::GetFaceAtPoint(CurveVectorPtr& region, DPoint3dCR seedPoint)
    {
    RegionData& data = static_cast<RegionData&> (*m_regionData);
    FloodSeed   floodSeed;

    floodSeed.m_pt = seedPoint;
    data.CollectFaceLoopsAtPoint(&floodSeed.m_faceNodeIds, floodSeed.m_pt, m_regionLoops, m_stepOutOfHoles);

    if (0 == floodSeed.m_faceNodeIds.size())
        {
        m_dynamicFaceSeed.m_faceNodeIds.clear();
        region = nullptr;

        return false; // No closed loop found at point...
        }
    else if (!m_dynamicFaceSeed.m_faceNodeIds.empty() && m_dynamicFaceSeed.m_faceNodeIds[0] == floodSeed.m_faceNodeIds[0])
        {
        if (region.IsValid() && 0 != region->size())
            return false; // Same face loop as last time...
        }

    // New face hit...populate gpa with new face geometry...
    m_dynamicFaceSeed = floodSeed;

    data.GetFaceLoops(region, floodSeed.m_faceNodeIds);

    return region.IsValid(); // Return true when a new face is identified...
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool RegionGraphicsContext::GetActiveFaces(CurveVectorPtr& region)
    {
    if (SUCCESS != GetRegion(region))
        region = nullptr;

    return region.IsValid();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/17
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr RegionGraphicsContext::GetFromPerspectiveRegion(CurveVectorCR region, bool isDynamic)
    {
    if (!IsPerspectiveRegion())
        return nullptr;

    if (!isDynamic && m_floodSeeds.empty())
        return nullptr;

    bvector<Dgn::DgnElementId> roots;

    if (SUCCESS != GetRoots(roots, region))
        return nullptr;

    DgnElementCPtrVec elements;

    for (DgnElementId elemId : roots)
        {
        DgnElementCPtr element = GetDgnDb().Elements().GetElement(elemId);

        if (!element.IsValid())
            continue;

        elements.push_back(element);
        }

    if (elements.empty())
        return nullptr;

    RegionGraphicsContextPtr tmpRegionContext = RegionGraphicsContext::Create();

    tmpRegionContext->SetFloodParams(m_regionLoops, m_gapTolerance, m_stepOutOfHoles);

    if (SUCCESS != tmpRegionContext->PopulateGraph(elements))
        return nullptr;

    RegionData& tmpData = static_cast<RegionData&> (*tmpRegionContext->m_regionData);
    DPoint3d    facePoint = DPoint3d::FromZero();
    DVec3d      faceNormal = DVec3d::UnitZ();

    tmpData.m_viewToWorldTrans.Multiply(facePoint);
    tmpData.m_viewToWorldTrans.MultiplyMatrixOnly(faceNormal);

    CurveVectorPtr resultRegion;

    if (isDynamic)
        {
        DPoint3d tmpPt = m_dynamicFaceSeed.m_pt;
        DVec3d   viewZRoot;

        viewZRoot.DifferenceOf(tmpPt, *(&GetViewport()->GetCamera().GetEyePoint()));
        LegacyMath::Vec::LinePlaneIntersect(&tmpPt, &tmpPt, &viewZRoot, &facePoint, &faceNormal, false);

        if (!tmpRegionContext->GetFaceAtPoint(resultRegion, tmpPt))
            return nullptr;

        return resultRegion;
        }

    bvector<DPoint3d> seedPoints;

    for (size_t iSeed = 0; iSeed < m_floodSeeds.size(); iSeed++)
        {
        DPoint3d tmpPt = m_floodSeeds[iSeed].m_pt;
        DVec3d   viewZRoot;

        viewZRoot.DifferenceOf(tmpPt, *(&GetViewport()->GetCamera().GetEyePoint()));
        LegacyMath::Vec::LinePlaneIntersect(&tmpPt, &tmpPt, &viewZRoot, &facePoint, &faceNormal, false);
        seedPoints.push_back(tmpPt);
        }

    tmpRegionContext->AddFaceLoopsAtPoints(&seedPoints[0], seedPoints.size());

    if (!tmpRegionContext->GetActiveFaces(resultRegion))
        return nullptr;

    return resultRegion;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool RegionGraphicsContext::IsFaceAtPointSelected(DPoint3dCR seedPoint)
    {
    RegionData& data = static_cast<RegionData&> (*m_regionData);
    FloodSeed   floodSeed;

    floodSeed.m_pt = seedPoint;
    data.CollectFaceLoopsAtPoint(&floodSeed.m_faceNodeIds, floodSeed.m_pt, m_regionLoops, m_stepOutOfHoles);

    if (0 == floodSeed.m_faceNodeIds.size())
        return false;

    for (size_t iFace = 0; iFace < floodSeed.m_faceNodeIds.size(); iFace++)
        {
        if (data.IsFaceLoopSelected(floodSeed.m_faceNodeIds[iFace]))
            return true;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool RegionGraphicsContext::ToggleFaceAtPoint(DPoint3dCR seedPoint)
    {
    RegionData& data = static_cast<RegionData&> (*m_regionData);
    FloodSeed   floodSeed;

    floodSeed.m_pt = seedPoint;
    data.CollectFaceLoopsAtPoint(&floodSeed.m_faceNodeIds, floodSeed.m_pt, m_regionLoops, m_stepOutOfHoles);

    if (0 == floodSeed.m_faceNodeIds.size())
        return false;

    bool added = false;

    for (size_t iFace = 0; iFace < floodSeed.m_faceNodeIds.size(); iFace++)
        {
        if (data.ToggleFaceLoop(floodSeed.m_faceNodeIds[iFace]) && 0 == iFace)
            added = true;
        }

    if (added)
        {
        m_floodSeeds.push_back(floodSeed);
        }
    else if (!m_floodSeeds.empty())
        {
        // Remove seed points for this face by invalidating node list...
        for (size_t iSeed = 0; iSeed < m_floodSeeds.size(); iSeed++)
            {
            // first face id is always outer loop...
            if (m_floodSeeds[iSeed].m_faceNodeIds[0] != floodSeed.m_faceNodeIds[0])
                continue;

            m_floodSeeds[iSeed].m_faceNodeIds.clear();
            break;
            }

        bvector<FloodSeed>::iterator curr = m_floodSeeds.begin();  // first entry
        bvector<FloodSeed>::iterator endIt = m_floodSeeds.end();   // one past the last entry
        bvector<FloodSeed>::iterator nextValid = curr;              // where next valid entry should go

        for (; curr != endIt; ++curr)
            {
            if (!curr->m_faceNodeIds.empty())
                {
                if (nextValid != curr)
                    *nextValid = *curr;

                ++nextValid;
                }
            }

        if (nextValid != endIt)
            m_floodSeeds.erase(nextValid, endIt);
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus RegionGraphicsContext::AddFaceLoopsAtPoints(DPoint3dCP seedPoints, size_t numSeed)
    {
    RegionData& data = static_cast<RegionData&> (*m_regionData);

    for (size_t iSeed = 0; iSeed < numSeed; iSeed++)
        {
        FloodSeed   floodSeed;

        floodSeed.m_pt = seedPoints[iSeed];
        data.CollectFaceLoopsAtPoint(&floodSeed.m_faceNodeIds, floodSeed.m_pt, m_regionLoops, m_stepOutOfHoles);

        if (0 == floodSeed.m_faceNodeIds.size())
            continue;

        for (size_t iFace = 0; iFace < floodSeed.m_faceNodeIds.size(); iFace++)
            data.AddFaceLoop(floodSeed.m_faceNodeIds[iFace]);

        m_floodSeeds.push_back(floodSeed);
        }

    return (m_floodSeeds.empty() ? ERROR : SUCCESS);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus RegionGraphicsContext::PopulateGraph(DgnViewportP vp, DgnElementCPtrVec const* in, DRange3dCP range, DPlane3dCP plane, CurveVectorCP boundaryEdges)
    {
    if (nullptr == vp)
        return ERROR;

    m_operation = RegionType::Flood;
    m_ignoreViewRange = false;

    if (nullptr != range)
        {
        Frustum    frustum;
        DPoint3dP  frustPts = frustum.GetPtsP();

        range->Get8Corners(frustPts);
        vp->GetWorldToNpcMap()->M0.MultiplyAndRenormalize(frustPts, frustPts, NPC_CORNER_COUNT);
        m_npcSubRange = frustum.ToRange();
        m_useNpcSubRange = true;
        }

    if (SUCCESS != Attach(vp, m_purpose))
        return ERROR;

    if (nullptr != plane)
        {
        m_plane = *plane;
        m_restrictToPlane = true;
        }

    if (in)
        {
        for (DgnElementCPtr curr : *in)
            {
            GeometrySourceCP geomElement = (curr.IsValid() ? curr->ToGeometrySource() : nullptr);

            if (nullptr == geomElement)
                continue;

            _VisitGeometry(*geomElement);
            }
        }
    else
        {
        VisitAllViewElements();
        }

    if (WasAborted())
        return ERROR;

    if (nullptr != boundaryEdges)
        {
        if (0 == m_currentGeomMarkerId)
            return ERROR; // No other geometry was added...

        Render::GraphicBuilderPtr builder = CreateSceneGraphic();
        SimplifyGraphic* graphic = static_cast<SimplifyGraphic*> (builder.get());

        _ProcessCurveVector(*boundaryEdges, false, *graphic);
        }

    RegionData& data = static_cast<RegionData&> (*m_regionData);

    return data.SetupGraph(m_gapTolerance, RegionLoops::Ignore != m_regionLoops);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus RegionGraphicsContext::PopulateGraph(DgnElementCPtrVec const& in)
    {
    if (in.empty())
        return ERROR;

    SetDgnDb(in.front()->GetDgnDb());
    m_operation = RegionType::Flood;

    for (DgnElementCPtr curr : in)
        {
        GeometrySourceCP geomElement = (curr.IsValid() ? curr->ToGeometrySource() : nullptr);

        if (nullptr == geomElement)
            continue;

        _VisitGeometry(*geomElement);
        }

    RegionData& data = static_cast<RegionData&> (*m_regionData);

    return data.SetupGraph(m_gapTolerance, RegionLoops::Ignore != m_regionLoops);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus RegionGraphicsContext::Flood(DgnElementCPtrVec const& in, DPoint3dCP seedPoints, size_t numSeed)
    {
    if (in.empty())
        return ERROR;

    SetDgnDb(in.front()->GetDgnDb());
    m_operation = RegionType::Flood;

    for (DgnElementCPtr curr : in)
        {
        GeometrySourceCP geomElement = (curr.IsValid() ? curr->ToGeometrySource() : nullptr);

        if (nullptr == geomElement)
            continue;

        _VisitGeometry(*geomElement);
        }

    RegionData& data = static_cast<RegionData&> (*m_regionData);

    if (SUCCESS != data.SetupGraph(m_gapTolerance, RegionLoops::Ignore != m_regionLoops))
        return ERROR;

    return AddFaceLoopsAtPoints(seedPoints, numSeed);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus RegionGraphicsContext::Boolean(DgnDbR db, bvector<CurveVectorPtr> const& in, RegionType operation)
    {
    if (RegionType::Flood == operation)
        return ERROR;

    SetDgnDb(db);
    m_operation = operation;

    Render::GraphicBuilderPtr builder = CreateSceneGraphic();
    SimplifyGraphic* graphic = static_cast<SimplifyGraphic*> (builder.get());

    for (CurveVectorPtr const& curve: in)
        {
        if (!curve.IsValid())
            continue;

        _ProcessCurveVector(*curve, false, *graphic);
        }

    RegionData& data = static_cast<RegionData&> (*m_regionData);

    if (SUCCESS != data.SetupGraph(0.0, true))
        return ERROR;

    if (RegionType::Intersection == operation || RegionType::ExclusiveOr == operation)
        return data.CollectBooleanFaces(getRGBoolSelect(operation, true), m_currentGeomMarkerId, m_currentGeomMarkerId);

    return data.CollectBooleanFaces(getRGBoolSelect(operation), 1, m_currentGeomMarkerId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus RegionGraphicsContext::Boolean(DgnElementCPtrVec const& in, RegionType operation)
    {
    if (RegionType::Flood == operation)
        return ERROR;

    if (in.empty())
        return ERROR;

    SetDgnDb(in.front()->GetDgnDb());
    m_operation = operation;

    for (DgnElementCPtr curr : in)
        {
        GeometrySourceCP geomElement = (curr.IsValid() ? curr->ToGeometrySource() : nullptr);

        if (nullptr != geomElement)
            VisitBooleanCandidate(*geomElement, nullptr, true);
        }

    int highestOperand = m_currentGeomMarkerId;

    RegionData& data = static_cast<RegionData&> (*m_regionData);

    if (SUCCESS != data.SetupGraph(0.0, true))
        return ERROR;

    if (RegionType::Intersection == operation || RegionType::ExclusiveOr == operation)
        return data.CollectBooleanFaces(getRGBoolSelect(operation, true), highestOperand, highestOperand);

    return data.CollectBooleanFaces(getRGBoolSelect(operation), 1, highestOperand);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus RegionGraphicsContext::Boolean(DgnElementCPtrVec const& target, DgnElementCPtrVec const& tool, RegionType operation)
    {
    if (RegionType::Flood == operation)
        return ERROR;

    if (target.empty())
        return ERROR;

    SetDgnDb(target.front()->GetDgnDb());
    m_operation = operation;

    for (DgnElementCPtr curr : target)
        {
        GeometrySourceCP geomElement = (curr.IsValid() ? curr->ToGeometrySource() : nullptr);

        if (nullptr != geomElement)
            VisitBooleanCandidate(*geomElement, nullptr, false);
        }

    int highestOperandA = m_currentGeomMarkerId;

    for (DgnElementCPtr curr : tool)
        {
        GeometrySourceCP geomElement = (curr.IsValid() ? curr->ToGeometrySource() : nullptr);

        if (nullptr != geomElement)
            VisitBooleanCandidate(*geomElement, nullptr, true);
        }

    int highestOperandB = m_currentGeomMarkerId;

    RegionData& data = static_cast<RegionData&> (*m_regionData);

    if (SUCCESS != data.SetupGraph(0.0, true))
        return ERROR;

    return data.CollectBooleanFaces(getRGBoolSelect(operation), highestOperandA, highestOperandB);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus RegionGraphicsContext::BooleanWithHoles(DgnElementCPtrVec const& in, DgnElementCPtrVec const& holes, RegionType operation)
    {
    if (RegionType::Flood == operation)
        return ERROR;

    if (in.empty())
        return ERROR;

    SetDgnDb(in.front()->GetDgnDb());
    m_operation = operation;

    bvector<DMatrix4d> wireProducts;

    for (DgnElementCPtr curr : in)
        {
        GeometrySourceCP geomElement = (curr.IsValid() ? curr->ToGeometrySource() : nullptr);

        if (nullptr != geomElement)
            VisitBooleanCandidate(*geomElement, m_cullRedundantLoop ? &wireProducts : nullptr);
        }

    int highestOperand = m_currentGeomMarkerId;

    for (DgnElementCPtr curr : holes)
        {
        GeometrySourceCP geomElement = (curr.IsValid() ? curr->ToGeometrySource() : nullptr);

        if (nullptr != geomElement)
            VisitBooleanCandidate(*geomElement);
        }

    RegionData& data = static_cast<RegionData&> (*m_regionData);

    if (SUCCESS != data.SetupGraph(0.0, true))
        return ERROR;

    if (RegionType::Intersection == operation || RegionType::ExclusiveOr == operation)
        return data.CollectBooleanFaces(getRGBoolSelect(operation, true), highestOperand, highestOperand);

    return data.CollectBooleanFaces(getRGBoolSelect(operation), 1, highestOperand);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/09
+---------------+---------------+---------------+---------------+---------------+------*/
void RegionGraphicsContext::SetAbortFunction(RegionGraphicsContext_AbortFunction abort)
    {
    RegionData& data = static_cast<RegionData&> (*m_regionData);

    jmdlRG_setAbortFunction(data.m_pRG, (RGC_AbortFunction) abort);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool RegionGraphicsContext::IsGraphInitialized()
    {
    RegionData& data = static_cast<RegionData&> (*m_regionData);

    return data.m_graphIsValid;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/09
+---------------+---------------+---------------+---------------+---------------+------*/
RegionGraphicsContextPtr RegionGraphicsContext::Create()
    {
    return new RegionGraphicsContext();
    }
