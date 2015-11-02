/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnHandlers/RegionUtil.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  08/09
+---------------+---------------+---------------+---------------+---------------+------*/
RegionParams::RegionParams()
    {
    m_type              = RegionType::ExclusiveOr;
    m_regionLoops       = RegionLoops::Ignore;

    m_interiorText      = false;
    m_associative       = false;
    m_invisibleBoundary = false;
    m_forcePlanar       = true;
    m_dirty             = false;

    m_reservedFlags     = 0;

    m_gapTolerance      = 0.0;
    m_textMarginFactor  = 0.0;

    m_flatten.InitIdentity();
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  08/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            RegionParams::SetType(RegionType regionType)
    {
    m_type = regionType;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  08/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            RegionParams::SetFloodParams(RegionLoops regionLoops, double gapTolerance)
    {
    m_regionLoops  = regionLoops;
    m_gapTolerance = gapTolerance;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  08/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            RegionParams::SetInteriorText(bool interiorText, double textMarginFactor)
    {
    m_interiorText     = interiorText;
    m_textMarginFactor = textMarginFactor;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  08/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            RegionParams::SetAssociative(bool yesNo)
    {
    m_associative = yesNo;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  08/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            RegionParams::SetInvisibleBoundary(bool yesNo)
    {
    m_invisibleBoundary = yesNo;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  08/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            RegionParams::SetFlattenBoundary(bool yesNo, RotMatrixCP flatten)
    {
    m_forcePlanar = yesNo;
    
    if (flatten)
        m_flatten = *flatten;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  08/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            RegionParams::SetDirty(bool yesNo)
    {
    m_dirty = yesNo;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  08/09
+---------------+---------------+---------------+---------------+---------------+------*/
RegionType      RegionParams::GetType() const
    {
    return m_type;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  08/09
+---------------+---------------+---------------+---------------+---------------+------*/
RegionLoops     RegionParams::GetFloodParams(double* gapTolerance) const
    {
    if (gapTolerance)
        *gapTolerance = m_gapTolerance;

    return m_regionLoops;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  08/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool            RegionParams::GetInteriorText(double* textMarginFactor) const
    {
    if (textMarginFactor)
        *textMarginFactor = m_textMarginFactor;

    return m_interiorText;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  08/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool            RegionParams::GetAssociative() const
    {
    return m_associative;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  08/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool            RegionParams::GetInvisibleBoundary() const
    {
    return m_invisibleBoundary;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  08/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool            RegionParams::GetFlattenBoundary(RotMatrixP flatten) const
    {
    if (flatten)
        *flatten = m_flatten;

    return m_forcePlanar;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  08/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool            RegionParams::GetDirty() const
    {
    return m_dirty;
    }

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

#if defined (NEEDS_WORK_DGNITEM)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/09
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus getPathRoots(DependencyRoot& root, DisplayPathCP path, DgnModelP homeModel, bool allowFarElm)
    {
    AssocGeom   assoc;

    memset(&assoc, 0, sizeof (assoc));
    assoc.type = CUSTOM_ASSOC;

    // NOTE: Loop roots only for DWG (no far roots) so it's ok to add the dependency even in dynamics!
    if (SUCCESS != AssociativePoint::SetRoot((AssocPoint&) assoc, path, homeModel, allowFarElm))
        return ERROR;

    memset(&root, 0, sizeof (root));
    root.elemid = assoc.singleElm.uniqueId;
//    root.refattid = assoc.singleElm.refAttachmentId;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/13
+---------------+---------------+---------------+---------------+---------------+------*/
static void getDependencyRoots(bvector<DependencyRoot>& depRoots, bvector<DisplayPathCP>& regionRoots, DgnModelP homeModel, bool allowFarElm)
    {
    // NOTE: SetBoundaryRoots culls duplicates...but we don't want getPathRoots creating multiple far-elems for duplicate paths!
    DisplayPathCP   first = regionRoots.front();

    std::sort(regionRoots.begin(), regionRoots.end());
    regionRoots.erase(std::unique(regionRoots.begin(), regionRoots.end()), regionRoots.end());

    for (DisplayPathCP path: regionRoots)
        {
        DependencyRoot  root;

        // Create DependencyRoot from path (may be far root)...
        if (SUCCESS != getPathRoots(root, path, homeModel, allowFarElm))
            continue;

        // Preserve first root for boolean difference, std::sort may have changed order...
        if (path == first)
            depRoots.insert(depRoots.begin(), root);
        else
            depRoots.push_back(root);
        }
    }
#endif

/*=================================================================================**//**
* @bsiclass                                                     Brien.Bastings  09/13
+===============+===============+===============+===============+===============+======*/
struct TextBoxInfo : RefCounted <ICurvePrimitiveInfo>
{
GeometricElementCPtr  m_element;
bool                  m_used;

TextBoxInfo(GeometricElementCP element) {m_element = element; m_used = false;}

static ICurvePrimitiveInfoPtr Create(GeometricElementCP element) {return new TextBoxInfo(element);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/13
+---------------+---------------+---------------+---------------+---------------+------*/
static void ClearUsed(CurveVectorCR curves)
    {
    if (curves.IsUnionRegion() || curves.IsParityRegion())
        {
        for (ICurvePrimitivePtr curve: curves)
            {
            if (curve.IsNull() || ICurvePrimitive::CURVE_PRIMITIVE_TYPE_CurveVector != curve->GetCurvePrimitiveType())
                continue;

            ClearUsed(*curve->GetChildCurveVectorCP ());
            }
        }
    else
        {
        for (ICurvePrimitivePtr curve: curves)
            {
            if (!curve.IsValid())
                continue;

            TextBoxInfo* textInfo = dynamic_cast <TextBoxInfo*> (curve->GetCurvePrimitiveInfo().get());

            if (!textInfo)
                continue;

            textInfo->m_used = false;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/13
+---------------+---------------+---------------+---------------+---------------+------*/
static void GetUsed(CurveVectorCR curves, bvector<DgnElementId>& regionRoots)
    {
    if (curves.IsUnionRegion() || curves.IsParityRegion())
        {
        for (ICurvePrimitivePtr curve: curves)
            {
            if (curve.IsNull() || ICurvePrimitive::CURVE_PRIMITIVE_TYPE_CurveVector != curve->GetCurvePrimitiveType())
                continue;

            GetUsed(*curve->GetChildCurveVectorCP (), regionRoots);
            }
        }
    else
        {
        for (ICurvePrimitivePtr curve: curves)
            {
            if (!curve.IsValid())
                continue;

            TextBoxInfo* textInfo = dynamic_cast <TextBoxInfo*> (curve->GetCurvePrimitiveInfo().get());

            if (!textInfo || !textInfo->m_used || !textInfo->m_element.IsValid())
                continue;

            regionRoots.push_back(textInfo->m_element->GetElementId());
            }
        }
    }

}; // TextBoxInfo

/*=================================================================================**//**
* @bsiclass                                                     Brien.Bastings  09/09
+===============+===============+===============+===============+===============+======*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/09
+---------------+---------------+---------------+---------------+---------------+------*/
RegionGraphicsDrawGeom::RegionGraphicsDrawGeom()
    {
    m_pRG     = jmdlRG_new();
    m_pCurves = jmdlRIMSBS_newContext();

    jmdlRIMSBS_setupRGCallbacks(m_pCurves, m_pRG);
    jmdlRG_setFunctionContext(m_pRG, m_pCurves);
    jmdlRG_setDistanceTolerance(m_pRG, 1.0); // one uor...not so good...
    m_activeFaces.Attach(jmdlRG_getGraph(m_pRG), MTG_ScopeFace);

    m_currentGeomMarkerId = 0;
    m_textMarginFactor = 0.0;
    m_interiorText = false;

    m_forcePlanar = false;
    m_flattenTrans.InitIdentity();
    m_flattenDir.Zero();

    m_regionError = REGION_ERROR_None;
    m_isFlood = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/09
+---------------+---------------+---------------+---------------+---------------+------*/
RegionGraphicsDrawGeom::~RegionGraphicsDrawGeom()
    {
    if (m_pCurves)
        {
        int     iCurve = 0;

        do
            {
            void*   userDataP;

            if (!jmdlRIMSBS_getUserPointer(m_pCurves, &userDataP, iCurve++))
                break;

#if defined (NEEDS_WORK_DGNITEM)
            if (userDataP)
                ((DisplayPath*) userDataP)->Release();
#endif

            } while (true);
        }

    jmdlRIMSBS_freeContext(m_pCurves);

    // Must Attach to NULL graph BEFORE freeing m_pRG (so Attach can release the old mask back to m_pRG)
    m_activeFaces.Attach(NULL, MTG_ScopeFace);
    jmdlRG_free(m_pRG);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/09
+---------------+---------------+---------------+---------------+---------------+------*/
void RegionGraphicsDrawGeom::_SetDrawViewFlags(ViewFlags flags)
    {
    T_Super::_SetDrawViewFlags(flags);

    // Prefer "edge" geometry...
    m_viewFlags.SetRenderMode(DgnRenderMode::Wireframe);

    // Apply overrides to flags...
    m_viewFlags.styles = false;
    m_viewFlags.fill = false;
    m_viewFlags.patterns = false;

    // Only turn off text, otherwise find text based on current view attribute...
    if (m_interiorText)
        return;

    m_viewFlags.text = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool            RegionGraphicsDrawGeom::ComputePostFlattenTransform(CurveVectorCR curves)
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
void            RegionGraphicsDrawGeom::ResetPostFlattenTransform()
    {
    if (!m_forcePlanar || 0.0 == m_flattenDir.Magnitude())
        return;

    m_flattenTrans.InitIdentity();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   RegionGraphicsDrawGeom::GetMarkedRegions(CurveVectorPtr& regionOut, MTG_MarkSet* markSet)
    {
    CurveVectorPtr    baseRegion = jmdlRG_collectExtendedFaces(m_pRG, m_pCurves, markSet);
    TransformCP       localToWorldP = NULL;

    if (NULL != m_context)
        localToWorldP = m_context->GetCurrLocalToWorldTransformCP ();

    if (!baseRegion.IsValid())
        return ERROR;

    if (m_textBoundaries.IsValid())
        {
        CurvePrimitivePtrPairVector newToOld;

        TextBoxInfo::ClearUsed(*m_textBoundaries); // Clear flag that tells us whether this text loop becames part of result region boundary...

        if (NULL == localToWorldP)
            {
            regionOut = CurveVector::AreaDifference(*baseRegion, *m_textBoundaries, &newToOld);
            }
        else
            {
            CurveVectorPtr  localText = m_textBoundaries->Clone();
            Transform       worldToLocal;

            worldToLocal.InverseOf(*localToWorldP);
            localText->TransformInPlace(worldToLocal);
            regionOut = CurveVector::AreaDifference(*baseRegion, *localText, &newToOld);
            }

        if (regionOut.IsValid())
            {
            // transfer root information from old curves to new ...
            for (size_t i = 0; i < newToOld.size(); i++)
                {
                if (newToOld[i].curveB.IsValid() && newToOld[i].curveA.IsValid())
                    {
                    newToOld[i].curveA->SetCurvePrimitiveInfo(newToOld[i].curveB->GetCurvePrimitiveInfo());

                    // Text box appeared in region result...mark as used for DgnRegionElementTool call to GetRoots for process originals...
                    TextBoxInfo* textInfo = dynamic_cast <TextBoxInfo*> (newToOld[i].curveA->GetCurvePrimitiveInfo().get());

                    if (textInfo)
                        textInfo->m_used = true;
                    }
                }
            }
        else
            {
            regionOut = baseRegion; // Region w/o text is better than nothing...
            }
        }
    else
        {
        regionOut = baseRegion;
        }

    if (!regionOut.IsValid())
        return ERROR;

    if (localToWorldP)
        regionOut->TransformInPlace(*localToWorldP);

    // Flatten to plane defined by "average" geometry depth...
    if (ComputePostFlattenTransform(*regionOut))
        regionOut->TransformInPlace(m_flattenTrans);

    return regionOut.IsValid() ? SUCCESS : ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   RegionGraphicsDrawGeom::GetActiveRegions(CurveVectorPtr& regionOut)
    {
    EmbeddedIntArray  startArray;
    EmbeddedIntArray  sequenceArray;

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
BentleyStatus   RegionGraphicsDrawGeom::GetRoots(bvector<DgnElementId>& regionRoots, ICurvePrimitiveCR curvePrimitive)
    {
    CurveNodeInfo const* nodeInfo = dynamic_cast <CurveNodeInfo const*> (curvePrimitive.GetCurvePrimitiveInfo().get());

    if (!nodeInfo)
        {
        TextBoxInfo const* textInfo = dynamic_cast <TextBoxInfo const*> (curvePrimitive.GetCurvePrimitiveInfo().get());

        if (!textInfo)
            return ERROR;

        regionRoots.push_back(textInfo->m_element->GetElementId());

        return SUCCESS;
        }

    for (int nodeId: nodeInfo->m_nodeIds)
        {
        int     parentIndex;

        if (!jmdlRG_getParentCurveIndex(m_pRG, &parentIndex, nodeId) || parentIndex < 0)
            continue;

        void*   userDataP; // held by context...

        if (!jmdlRIMSBS_getUserPointer(m_pCurves, &userDataP, parentIndex) || !userDataP)
            continue;

#if defined (NEEDS_WORK_DGNITEM)
        int     userInt = 0;

        // NOTE: Dependency root order matters for assoc region re-evaluate of boolean difference...first root is geometry to subtract from!
        if (!m_isFlood && jmdlRIMSBS_getUserInt(m_pCurves, &userInt, parentIndex) && 1 == userInt)
            regionRoots.insert(regionRoots.begin(), (DisplayPathCP) userDataP); 
        else
            regionRoots.push_back((DisplayPathCP) userDataP);
#endif
        }

    return (regionRoots.empty() ? ERROR : SUCCESS);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/13
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   RegionGraphicsDrawGeom::GetRoots(bvector<DgnElementId>& regionRoots, CurveVectorCR region)
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
BentleyStatus   RegionGraphicsDrawGeom::GetRoots(bvector<DgnElementId>& regionRoots)
    {
    EmbeddedIntArray  startArray;
    EmbeddedIntArray  sequenceArray;

    jmdlRG_collectAndNumberExtendedFaceLoops(m_pRG, &startArray, &sequenceArray, &m_activeFaces);

    for (int iCmpn=0, nodeId=0; jmdlEmbeddedIntArray_getInt(&sequenceArray, &nodeId, iCmpn++); )
        {
        int     parentIndex;

        if (jmdlRG_getParentCurveIndex(m_pRG, &parentIndex, nodeId) && parentIndex >= 0)
            {
            void*   userDataP;

            if (jmdlRIMSBS_getUserPointer(m_pCurves, &userDataP, parentIndex) && userDataP)
#if defined (NEEDS_WORK_DGNITEM)
                regionRoots.push_back((DisplayPathCP) userDataP); // held by context...
#else
                regionRoots.push_back(DgnElementId()); // held by context...
#endif
            }
        }

    if (m_textBoundaries.IsValid())
        TextBoxInfo::GetUsed(*m_textBoundaries, regionRoots); // Add text box paths that have been marked as used in result region boundary...

    // NOTE: Cull duplicate entries. Used by DgnRegionElementTool for "process originals"...
    std::sort(regionRoots.begin(), regionRoots.end());
    regionRoots.erase(std::unique(regionRoots.begin(), regionRoots.end()), regionRoots.end());

    return (regionRoots.empty() ? ERROR : SUCCESS);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/09
+---------------+---------------+---------------+---------------+---------------+------*/
GeometricElementCP RegionGraphicsDrawGeom::GetCurrentElement()
    {
    return m_context->GetCurrentElement();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/12
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       RegionGraphicsDrawGeom::_ProcessCurvePrimitive(ICurvePrimitiveCR primitive, bool closed, bool filled)
    {
    TransformCP placementTrans = m_context->GetCurrLocalToWorldTransformCP ();
#if defined (NEEDS_WORK_DGNITEM)
    GeometricElementCP element = GetCurrentElement();

    if (path)
        path->AddRef();
#else
    void*       userDataP = nullptr;
#endif

    switch (primitive.GetCurvePrimitiveType())
        {
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line:
            {
            DSegment3d  segment = *primitive.GetLineCP ();

            if (placementTrans)
                placementTrans->Multiply(segment.point, 2);

            jmdlRG_addLinear(m_pRG, segment.point, 2, false, jmdlRIMSBS_addDataCarrier(m_pCurves, m_currentGeomMarkerId, userDataP));
            break;
            }

        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString:
            {
            bvector<DPoint3d> points = *primitive.GetLineStringCP ();

            if (placementTrans)
                placementTrans->Multiply(&points[0], (int) points.size());

            jmdlRG_addLinear(m_pRG, &points[0], (int) points.size(), false, jmdlRIMSBS_addDataCarrier(m_pCurves, m_currentGeomMarkerId, userDataP));
            break;
            }

        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc:
            {
            DEllipse3d  ellipse = *primitive.GetArcCP ();
            int         curveId;
            
            if (placementTrans)
                placementTrans->Multiply(ellipse);

            curveId = jmdlRIMSBS_addDEllipse3d(m_pCurves, m_currentGeomMarkerId, userDataP, &ellipse);
            jmdlRG_addCurve(m_pRG, curveId, curveId);
            break;
            }

        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_BsplineCurve:
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_InterpolationCurve:
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_AkimaCurve:
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Spiral:
            {
            MSBsplineCurveCR  bcurve = *primitive.GetProxyBsplineCurveCP ();
            MSBsplineCurve    tmpCurve; // Requires copy of curve...do NOT free!
            int               curveId;

            if (placementTrans)
                tmpCurve.CopyTransformed(bcurve, *placementTrans);
            else
                tmpCurve.CopyFrom(bcurve);

            curveId = jmdlRIMSBS_addMSBsplineCurve(m_pCurves, m_currentGeomMarkerId, userDataP, &tmpCurve);
            jmdlRG_addCurve(m_pRG, curveId, curveId);
            break;
            }
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/13
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       RegionGraphicsDrawGeom::_ProcessCurveVector(CurveVectorCR curves, bool filled)
    {
    if (m_inTextDraw)
        {
        if (curves.IsClosedPath() && ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Invalid != curves.HasSingleCurvePrimitive())
            {
            TransformCP     placementTransform = m_context->GetCurrLocalToWorldTransformCP ();
            CurveVectorPtr  textBox = curves.Clone();

            if (NULL != placementTransform)
                textBox->TransformInPlace(*placementTransform);

            GeometricElementCP element = GetCurrentElement();

            if (element)
                textBox->front()->SetCurvePrimitiveInfo(TextBoxInfo::Create(element));

            if (!m_textBoundaries.IsValid())
                m_textBoundaries = CurveVector::Create(CurveVector::BOUNDARY_TYPE_UnionRegion);

            m_textBoundaries->Add(textBox);
            }

        return SUCCESS;
        }

    jmdlRIMSBS_setCurrGroupId(m_pCurves, ++m_currentGeomMarkerId);

    return ERROR; // Output curve primitives...
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/05
+---------------+---------------+---------------+---------------+---------------+------*/
void            RegionGraphicsDrawGeom::_DrawTextString(TextStringCR text, double* zDepth)
    {
    if (!m_interiorText)
        return;

    if (text.GetText().empty())
        return;

    AutoRestore <bool> saveInTextDraw(&m_inTextDraw, true);

    double padding = (text.GetStyle().GetHeight() * m_textMarginFactor);
    DPoint3d    points[5];

    text.ComputeBoundingShape(points, padding);
    text.ComputeTransform().Multiply(points, _countof(points));

    CurveVectorPtr  curve = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Outer);

    curve->push_back(ICurvePrimitive::CreateLineString(points, 5));

    ClipAndProcessCurveVector(*curve, false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/09
+---------------+---------------+---------------+---------------+---------------+------*/
int             RegionGraphicsDrawGeom::GetCurrentGeomMarkerId()
    {
    return m_currentGeomMarkerId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   RegionGraphicsDrawGeom::SetupGraph(double gapTolerance, bool mergeHoles)
    {
    // Apply explicit flatten to plane transform to entire graph, not pushed on context since it's non-invertable...
    if (m_forcePlanar && !m_flattenTrans.IsIdentity())
        jmdlRG_multiplyByTransform(m_pRG, &m_flattenTrans);

    Transform   activeToViewTrans, viewToActiveTrans;

    if (m_context->GetViewport())
        {
        DMap4dCP    rootToViewMap = m_context->GetViewport()->GetWorldToViewMap();
//        DMap4dCP    activeToRootMap = m_context->GetViewport ()->GetActiveToRootMap ();
        DMap4d      activeToViewMap;

//        if (activeToRootMap)
//            activeToViewMap.InitProduct (*rootToViewMap, *activeToRootMap);
//        else
            activeToViewMap = *rootToViewMap;

        activeToViewTrans.InitFrom(activeToViewMap, false);
        viewToActiveTrans.InverseOf(activeToViewTrans);
        }
    else if (m_forcePlanar && m_isFlood)
        {
        // A non-planar graph isn't an error if we'll be post-flattening loops (flood only!)
        jmdlRG_getPlaneTransform(m_pRG, &viewToActiveTrans, NULL);
        activeToViewTrans.InverseOf(viewToActiveTrans);
        }
    else
        {
        if (!jmdlRG_getPlaneTransform(m_pRG, &viewToActiveTrans, NULL) || !activeToViewTrans.InverseOf(viewToActiveTrans))
            {
            m_regionError = REGION_ERROR_NonCoplanar;

            return ERROR;
            }
        }

    DVec3d      zAxis, zAxisWorld;

    zAxisWorld.Init(0.0, 0.0, 1.0);
    viewToActiveTrans.GetMatrixColumn(zAxis, 2);

    m_context->GetTransformClipStack().PopAll(*m_context);

    if (!zAxis.IsParallelTo(zAxisWorld) || m_context->GetViewport())
        {
        jmdlRG_multiplyByTransform(m_pRG, &activeToViewTrans);
        m_context->PushTransform(viewToActiveTrans);
        }

    TransformCP placementTransform = m_context->GetCurrLocalToWorldTransformCP ();

    if (NULL != placementTransform)
        {
        DVec3d  xDir;

        placementTransform->GetMatrixColumn(xDir, 0);
        gapTolerance /= xDir.Magnitude();
        }

    jmdlRG_mergeWithGapTolerance(m_pRG, gapTolerance, gapTolerance);
    jmdlRG_buildFaceRangeTree(m_pRG, 0.0, jmdlRG_getTolerance(m_pRG));

    if (mergeHoles)
        jmdlRG_buildFaceHoleArray(m_pRG);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            RegionGraphicsDrawGeom::GetFaceLoops(CurveVectorPtr& region, bvector<MTGNodeId>& faceNodeIds)
    {
    int         seedIndex;
    MTGNodeId   seedNodeId;

    jmdlMTGMarkSet_initIteratorIndex(&m_activeFaces, &seedIndex);

    if (!jmdlMTGMarkSet_getNextNode(&m_activeFaces, &seedIndex, &seedNodeId))
        ResetPostFlattenTransform(); // Compute new transform when there is no current active faces...

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
void            RegionGraphicsDrawGeom::AddFaceLoop(MTGNodeId faceNodeId)
    {
    jmdlMTGMarkSet_addNode(&m_activeFaces, faceNodeId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            RegionGraphicsDrawGeom::RemoveFaceLoop(MTGNodeId faceNodeId)
    {
    jmdlMTGMarkSet_removeNode(&m_activeFaces, faceNodeId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool            RegionGraphicsDrawGeom::ToggleFaceLoop(MTGNodeId faceNodeId)
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
bool            RegionGraphicsDrawGeom::IsFaceLoopSelected(MTGNodeId faceNodeId)
    {
    return TO_BOOL (jmdlMTGMarkSet_isNodeInSet(&m_activeFaces, faceNodeId));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            RegionGraphicsDrawGeom::CollectFaceLoopsAtPoint(bvector<MTGNodeId>* faceNodeIds, DPoint3dCR seedPoint, RegionLoops floodSelect, bool stepOutOfHoles)
    {
    DPoint3d    tmpPt = seedPoint;
    TransformCP placementTransP = m_context->GetCurrLocalToWorldTransformCP ();

    if (placementTransP)
        {
        Transform   inverseTrans;

        inverseTrans.InverseOf(*placementTransP);
        inverseTrans.Multiply(tmpPt);
        }

    MTGNodeId   seedNodeId;
    
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

        jmdlRG_getFaceSweepProperties(m_pRG, &areaToFace, NULL, NULL, seedNodeId);

        if (!(areaToFace > 0.0 || !jmdlRG_faceIsTrueExterior(m_pRG, seedNodeId)))
            return;
        }

    bvector<MTGNodeId> activeFaceNodes; 

    if (floodSelect == RegionLoops::Alternating)
        jmdlRG_collectAllNodesOnInwardParitySearch(m_pRG, &activeFaceNodes, NULL, seedNodeId);
    else
        jmdlRG_resolveHoleNodeId(m_pRG, &activeFaceNodes, seedNodeId);

    for (size_t i = 0; i < activeFaceNodes.size(); i++)
        faceNodeIds->push_back(jmdlRG_resolveFaceNodeId(m_pRG, activeFaceNodes[i]));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   RegionGraphicsDrawGeom::CollectBooleanFaces(RGBoolSelect boolOp, int highestOperandA, int highestOperandB)
    {
    if (!jmdlRG_collectBooleanFaces(m_pRG, boolOp, RGBoolSelect_Difference, highestOperandA, highestOperandB, &m_activeFaces))
        return ERROR;

    int         seedIndex;
    MTGNodeId   seedNodeId;

    jmdlMTGMarkSet_initIteratorIndex(&m_activeFaces, &seedIndex);

    return (jmdlMTGMarkSet_getNextNode(&m_activeFaces, &seedIndex, &seedNodeId) ? SUCCESS : ERROR);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            RegionGraphicsDrawGeom::CollectByInwardParitySearch(bool parityWithinComponent, bool vertexContactSufficient)
    {
    EmbeddedIntArray  holeArray;

    jmdlRG_collectAllNodesOnInwardParitySearchExt(m_pRG, &holeArray, NULL, MTG_NULL_NODEID, parityWithinComponent, vertexContactSufficient);

    MTGNodeId   faceNodeId = -1;

    for (int i = 0; jmdlEmbeddedIntArray_getInt(&holeArray, &faceNodeId, i); i++)
        jmdlMTGMarkSet_addNode(&m_activeFaces, faceNodeId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            RegionGraphicsDrawGeom::SetAbortFunction(RGC_AbortFunction abort)
    {
    jmdlRG_setAbortFunction(m_pRG, abort);
    }

/*=================================================================================**//**
* @bsiclass                                                     Brien.Bastings  09/09
+===============+===============+===============+===============+===============+======*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/09
+---------------+---------------+---------------+---------------+---------------+------*/
RegionGraphicsContext::RegionGraphicsContext()
    {
    m_purpose           = DrawPurpose::RegionFlood;
    m_targetModel       = NULL; // Destination for geometry when not using vp target...

    m_operation         = RegionType::Flood;
    m_regionLoops       = RegionLoops::Ignore;
    m_gapTolerance      = 0.0;

    m_stepOutOfHoles    = false;
    m_setLoopSymbology  = false;
    m_updateAssocRegion = false;
    m_cullRedundantLoop = false;
    }

#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void            RegionGraphicsContext::_SetupOutputs()
    {
    SetIViewDraw(m_output);

    m_output.SetViewContext(this);
    m_output.SetIsFlood(RegionType::Flood == m_operation);
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/13
+---------------+---------------+---------------+---------------+---------------+------*/
void            RegionGraphicsContext::_DrawTextString(TextStringCR text)
    {
    // Don't draw background shape and other adornments...
    text.GetGlyphSymbology(GetCurrentDisplayParams());
    CookDisplayParams();

    GetCurrentGraphicR().DrawTextString(text, NULL);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   RegionGraphicsContext::VisitFloodCandidate(GeometricElementCR element, TransformCP trans)
    {
    ViewContext::ContextMark mark(this);

    if (trans)
        _PushTransform(*trans);

    return (BentleyStatus) _VisitElement(element);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   RegionGraphicsContext::PushBooleanCandidate(GeometricElementCR element, TransformCP trans)
    {
    if (trans)
        _PushTransform(*trans);

    _SetCurrentElement(&element); // Push path entry since we aren't calling _VisitElement...

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   RegionGraphicsContext::VisitBooleanCandidate(GeometricElementCR element, TransformCP trans, bvector<DMatrix4d>* wireProducts, bool allowText)
    {
#if defined (V10_WIP_ELEMENTHANDLER)
    CurveVectorPtr  curves = ICurvePathQuery::ElementToCurveVector(eh);

    if (!curves.IsValid())
        {
        ITextQueryCP    textQuery;

        // Collect text boundaries for difference operand (For update of AssocRegions from DWG)...
        if (!allowText || !m_updateAssocRegion || !m_output.GetInteriorText() || NULL == (textQuery = eh.GetITextQuery()))
            return ERROR;

        T_ITextPartIdPtrVector  textParts;

        textQuery->GetTextPartIds(eh, *ITextQueryOptions::CreateDefault(), textParts);

        if (0 == textParts.size())
            return ERROR;

        ViewContext::ContextMark mark(this);

        if (SUCCESS != PushBooleanCandidate(eh, trans))
            return ERROR;

        for (ITextPartIdPtr& partId : textParts)
            {
            TextBlockPtr  textBlock = textQuery->GetTextPart(eh, *partId);

            if (!textBlock.IsValid())
                continue;

            DrawTextBlock(*textBlock); // Output text strings...
            }

        return SUCCESS;
        }
#else
    CurveVectorPtr  curves;

    if (!curves.IsValid())
        return SUCCESS;
#endif

    // Require closed (or phsically closed) for booleans (Ignore for update of AssocRegions from DWG can have open roots that form closed area)...
    if (!curves->IsAnyRegionType() && !m_updateAssocRegion)
        {
        DPoint3d    endPoints[2];

        if (!curves->IsOpenPath() || !curves->GetStartEnd(endPoints[0], endPoints[1]) || !endPoints[0].IsEqual(endPoints[1], 1.0e-6))
            return ERROR;
        }

    if (wireProducts)
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

    ViewContext::ContextMark mark(this);

    if (SUCCESS != PushBooleanCandidate(element, trans))
        return ERROR;

    m_output.ClipAndProcessCurveVector(*curves, false);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            RegionGraphicsContext::SetFloodParams(RegionLoops regionLoops, double gapTolerance, bool stepOutOfHoles)
    {
    m_regionLoops    = regionLoops;
    m_gapTolerance   = gapTolerance;
    m_stepOutOfHoles = stepOutOfHoles;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            RegionGraphicsContext::SetInteriorText(bool interiorText, double textMarginFactor)
    {
    m_output.SetTextMarginFactor(interiorText, textMarginFactor);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            RegionGraphicsContext::SetFlattenBoundary(TransformCR flattenTrans)
    {
    m_output.SetFlattenBoundary(flattenTrans);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            RegionGraphicsContext::SetFlattenBoundary(DVec3dCR flattenDir)
    {
    m_output.SetFlattenBoundary(flattenDir);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   RegionGraphicsContext::SetTargetModel(DgnModelR targetModel)
    {
#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
    _SetupOutputs();
#endif

    m_targetModel = &targetModel;
    SetDgnDb(targetModel.GetDgnDb());

#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
    SetViewFlags(GetViewFlags()); // Force _SetDrawViewFlags to be called on output...
#endif

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            RegionGraphicsContext::InitRegionParams(RegionParams& params)
    {
    params.SetType(m_operation);
    params.SetFloodParams(m_regionLoops, m_gapTolerance);
    params.SetInteriorText(GetViewFlags().text, m_output.GetTextMarginFactor());
    params.SetAssociative(true);

    if (NULL != m_output.GetFlattenBoundary())
        {
        RotMatrix   flatten;

        m_output.GetFlattenBoundary()->GetMatrix(flatten);
        params.SetFlattenBoundary(true, &flatten);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool            RegionGraphicsContext::GetAdjustedSeedPoints(bvector<DPoint3d>* seedPoints)
    {
    if (RegionType::Flood != m_operation)
        return false;

    TransformCP placementTransP = GetCurrLocalToWorldTransformCP ();

    for (size_t iSeed = 0; iSeed < m_floodSeeds.size(); iSeed++)
        {
        DPoint3d    tmpPt = m_floodSeeds[iSeed].m_pt;

        if (NULL != m_output.GetFlattenBoundary()) // project seeds into plane of region...
            {
            m_output.GetFlattenBoundary()->Multiply(tmpPt);

            if (placementTransP)
                {
                DVec3d      planeNormal;

                if (0.0 != planeNormal.NormalizedDifference(tmpPt, *(&m_floodSeeds[iSeed].m_pt)))
                    {
                    DVec3d      projectDir;

                    placementTransP->GetMatrixColumn(projectDir, 2);
                    LegacyMath::Vec::LinePlaneIntersect(&tmpPt, &m_floodSeeds[iSeed].m_pt, &projectDir, &tmpPt, &planeNormal, false);
                    }
                }
            }

        seedPoints->push_back(tmpPt);
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   RegionGraphicsContext::CreateRegionElement(DgnElementPtr& element, CurveVectorCR region, bvector<DgnElementId> const* regionRoots, bool is3d)
    {
#if defined (NEEDS_WORK_DGNITEM)
    switch (region.GetBoundaryType())
        {
        case CurveVector::BOUNDARY_TYPE_UnionRegion:
            {
            ElementAgenda   solidAgenda;

            for (ICurvePrimitivePtr curvePrimitive: region)
                {
                if (curvePrimitive.IsNull())
                    continue;
            
                if (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_CurveVector != curvePrimitive->GetCurvePrimitiveType())
                    return ERROR;

                EditElementHandle  tmpEeh;

                if (SUCCESS != CreateRegionElement(tmpEeh, *curvePrimitive->GetChildCurveVectorCP (), regionRoots, is3d))
                    return ERROR;

                solidAgenda.InsertElemDescr(tmpEeh.ExtractWriteableElement().get());
                }

            RegionParams    params;

            params.SetType(RegionType::Union);

            if (SUCCESS != AssocRegionCellHeaderHandler::CreateAssocRegionElement(eeh, solidAgenda, NULL, 0, NULL, 0, params, NULL))
                return ERROR;
            
            break;
            }

        case CurveVector::BOUNDARY_TYPE_ParityRegion:
            {
            EditElementHandle   solidEeh;
            ElementAgenda       holeAgenda;

            for (ICurvePrimitivePtr curvePrimitive: region)
                {
                if (curvePrimitive.IsNull())
                    continue;
            
                if (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_CurveVector != curvePrimitive->GetCurvePrimitiveType())
                    return ERROR;

                if (CurveVector::BOUNDARY_TYPE_Outer == curvePrimitive->GetChildCurveVectorCP ()->GetBoundaryType())
                    {
                    if (SUCCESS != CreateRegionElement(solidEeh, *curvePrimitive->GetChildCurveVectorCP (), regionRoots, is3d))
                        return ERROR;
                    }
                else
                    {
                    EditElementHandle  tmpEeh;

                    if (SUCCESS != CreateRegionElement(tmpEeh, *curvePrimitive->GetChildCurveVectorCP (), regionRoots, is3d))
                        return ERROR;

                    holeAgenda.InsertElemDescr(tmpEeh.ExtractWriteableElement().get());
                    }
                }

            if (SUCCESS != GroupedHoleHandler::CreateGroupedHoleElement(eeh, solidEeh, holeAgenda))
                return ERROR;

            break;
            }

        default:
            {
            if (SUCCESS != DraftingElementSchema::ToElement(eeh, region, NULL, is3d, *_GetViewTarget()))
                return ERROR;

            // Set symbology of entire loop based on primitive curve/segment root of greatest length (NOTE: For pseudo-legacy grouped hole tool behavior...)
            if (m_setLoopSymbology)
                {
                double              length = 0.0;
                size_t              segmentIndex = 0;
                ICurvePrimitiveCP   templateCurve = NULL;

                for (ICurvePrimitivePtr curvePrimitive: region)
                    {
                    if (curvePrimitive.IsNull())
                        continue;

                    double  thisLength = 0.0;
                    size_t  thisSegmentIndex = 0;

                    bvector<DPoint3d> const* points = curvePrimitive->GetLineStringCP ();

                    // NOTE: Linear segments were concatinated to a single linestring with curve roots for each segment. Multiple segments 
                    //       can refer to the same parent curve. Too much effort to get per-root lengths, segment length is hopefully good enough...
                    if (points)
                        {
                        for (size_t iPt = 0; iPt < points->size()-1; ++iPt)
                            {
                            double  thisSegmentLength = points->at(iPt).Distance(points->at(iPt+1));

                            if (thisSegmentLength <= thisLength)
                                continue;

                            thisLength = thisSegmentLength;
                            thisSegmentIndex = iPt;
                            }
                        }
                    else
                        {
                        curvePrimitive->Length(thisLength);
                        }

                    if (thisLength <= length)
                        continue;

                    length = thisLength;
                    segmentIndex = thisSegmentIndex;

                    templateCurve = curvePrimitive.get();
                    }

                if (templateCurve)
                    {
                    bvector<DisplayPathCP>  curveRoots;

                    if (SUCCESS == GetRoots(curveRoots, *templateCurve))
                        {
                        DisplayPathCP       templatePath = curveRoots.at(segmentIndex < curveRoots.size() ? segmentIndex : 0);
                        EditElementHandle   templateEeh(templatePath->GetCursorElem());

                        ElementPropertiesSetter::ApplyTemplate(eeh, templateEeh);
                        }
                    }
                }

            if (!regionRoots)
                break;

            bvector<DisplayPathCP>  loopRoots;

            if (SUCCESS != GetRoots(loopRoots, region)) // Get roots for this loop...
                break;

            regionRoots->insert(regionRoots->end(), loopRoots.begin(), loopRoots.end()); // Accumulate roots for all loops...

            int     loopOEDCode = (CurveVector::BOUNDARY_TYPE_Outer == region.GetBoundaryType() ? DWG_OUTERMOST_PATH : DWG_EXTERNAL_PATH);

            // NOTE: What's important for DWG is setting DWG_TEXTBOX_PATH...loop OED code is supposed to be a mask...
            for (DisplayPathCP path: loopRoots)
                {
                ElementHandle  pathEh(path->GetCursorElem());

                if (NULL != pathEh.GetITextQuery())
                    {
                    loopOEDCode = DWG_TEXTBOX_PATH;
                    break;
                    }
                }

            AssocRegionCellHeaderHandler::SetLoopOedCode(eeh, loopOEDCode);

            // NOTE: Loop roots are only for DWG which doesn't allow far roots, so it's ok to create the dependency even in dynamics...
            bvector<DependencyRoot>  depRoots;

            getDependencyRoots(depRoots, loopRoots, _GetViewTarget(), false); // Create DependencyRoot from path (may NOT be far root)...
            AssocRegionCellHeaderHandler::SetLoopRoots(eeh, &depRoots[0], depRoots.size());
            break;
            }
        }

    return SUCCESS;
#else
    return ERROR;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   RegionGraphicsContext::CreateRegionElements(DgnElementPtrVec& out, CurveVectorCR region, bvector<DgnElementId> const* regionRoots, bool is3d)
    {
    // Return agenda of solid areas instead of a single union region...
    if (CurveVector::BOUNDARY_TYPE_UnionRegion == region.GetBoundaryType())
        {
        for (ICurvePrimitivePtr curvePrimitive: region)
            {
            if (curvePrimitive.IsNull())
                continue;
            
            if (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_CurveVector != curvePrimitive->GetCurvePrimitiveType())
                return ERROR;

            DgnElementPtr element;

            if (SUCCESS != CreateRegionElement(element, *curvePrimitive->GetChildCurveVectorCP (), regionRoots, is3d))
                return ERROR;

            out.push_back(element);
            }
        }
    else
        {
        DgnElementPtr element;

        if (SUCCESS != CreateRegionElement(element, region, regionRoots, is3d))
            return ERROR;

        out.push_back(element);
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/13
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   RegionGraphicsContext::GetRegion(CurveVectorPtr& region)
    {
    return m_output.GetActiveRegions(region);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/13
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   RegionGraphicsContext::GetRegion(DgnElementPtr& element)
    {
    CurveVectorPtr  region;

    if (SUCCESS != m_output.GetActiveRegions(region))
        return ERROR;

    return CreateRegionElement(element, *region, NULL, GetViewTarget()->Is3d());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   RegionGraphicsContext::GetRegions(DgnElementPtrVec& out)
    {
    CurveVectorPtr  region;

    if (SUCCESS != m_output.GetActiveRegions(region))
        return ERROR;

    return CreateRegionElements(out, *region, NULL, GetViewTarget()->Is3d());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   RegionGraphicsContext::GetAssociativeRegion(DgnElementPtr& element, RegionParams const& params, WCharCP cellName)
    {
    if (params.GetType() != m_operation)
        return ERROR;

    if (!params.GetAssociative())
        return ERROR; // Should not create an un-associated assoc region except to represent a union region, which is handled by GetRegion...

    CurveVectorPtr  region;

    if (SUCCESS != m_output.GetActiveRegions(region))
        return ERROR;

    DgnElementPtrVec        out;
    bvector<DgnElementId>   regionRoots;

    if (SUCCESS != CreateRegionElements(out, *region, &regionRoots, GetViewTarget()->Is3d()))
        return ERROR;

#if defined (NEEDS_WORK_DGNITEM)
    bvector<DependencyRoot> depRoots;

    getDependencyRoots(depRoots, regionRoots, _GetViewTarget(), true); // Create DependencyRoot from path (may be far root)...

    if (0 == depRoots.size())
        return ERROR;

    bvector<DPoint3d> seedPoints;

    GetAdjustedSeedPoints(&seedPoints);

    return AssocRegionCellHeaderHandler::CreateAssocRegionElement(eeh, out, &depRoots[0], depRoots.size(), &seedPoints[0], seedPoints.size(), params, cellName);
#endif
    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   RegionGraphicsContext::UpdateAssociativeRegion(DgnElementPtr& element)
    {
    m_targetModel = element->GetModel().get(); // Model to use to create new geometry...

    CurveVectorPtr  region;

    if (SUCCESS != m_output.GetActiveRegions(region))
        return ERROR;

    DgnElementPtrVec        out;
    bvector<DgnElementId>   regionRoots;

    // NOTE: Can't use model dimension to update assoc region boundary in dictionary model...
    if (SUCCESS != CreateRegionElements(out, *region, &regionRoots, element->Is3d()))
        return ERROR;

#if defined (NEEDS_WORK_DGNITEM)
    if (NULL != m_output.GetFlattenBoundary())
        {
        bvector<DPoint3d> seedPoints;

        // update seeds points with seeds projected into plane of region...
        if (GetAdjustedSeedPoints(&seedPoints))
            AssocRegionCellHeaderHandler::SetFloodSeedPoints(eeh, &seedPoints[0], seedPoints.size());
        }

    return AssocRegionCellHeaderHandler::UpdateAssocRegionBoundary(eeh, out);
#else
    return ERROR;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/12
+---------------+---------------+---------------+---------------+---------------+------*/
int             RegionGraphicsContext::GetCurrentFaceNodeId()
    {
    return (0 != m_dynamicFaceSeed.m_faceNodeIds.size() ? m_dynamicFaceSeed.m_faceNodeIds[0] : 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool            RegionGraphicsContext::GetFaceAtPoint(CurveVectorPtr& region, DPoint3dCR seedPoint)
    {
    FloodSeed   floodSeed;

    floodSeed.m_pt = seedPoint;
    m_output.CollectFaceLoopsAtPoint(&floodSeed.m_faceNodeIds, floodSeed.m_pt, m_regionLoops, false);

    if (0 == floodSeed.m_faceNodeIds.size())
        {
        m_dynamicFaceSeed.m_faceNodeIds.clear();
        region = NULL;

        return false; // No closed loop found at point...
        }
    else if (!m_dynamicFaceSeed.m_faceNodeIds.empty() && m_dynamicFaceSeed.m_faceNodeIds[0] == floodSeed.m_faceNodeIds[0])
        {
        if (region.IsValid() && 0 != region->size())
            return false; // Same face loop as last time...
        }

    // New face hit...populate gpa with new face geometry...
    m_dynamicFaceSeed = floodSeed;

    m_output.GetFaceLoops(region, floodSeed.m_faceNodeIds);
    
    return region.IsValid(); // Return true when a new face is identified...
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool            RegionGraphicsContext::GetActiveFaces(CurveVectorPtr& region)
    {
    if (SUCCESS != m_output.GetActiveRegions(region))
        region = NULL;

    return region.IsValid();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool            RegionGraphicsContext::IsFaceAtPointSelected(DPoint3dCR seedPoint)
    {
    FloodSeed   floodSeed;

    floodSeed.m_pt = seedPoint;
    m_output.CollectFaceLoopsAtPoint(&floodSeed.m_faceNodeIds, floodSeed.m_pt, m_regionLoops, false);

    if (0 == floodSeed.m_faceNodeIds.size())
        return false;

    for (size_t iFace = 0; iFace < floodSeed.m_faceNodeIds.size(); iFace++)
        {
        if (m_output.IsFaceLoopSelected(floodSeed.m_faceNodeIds[iFace]))
            return true;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool            RegionGraphicsContext::ToggleFaceAtPoint(DPoint3dCR seedPoint)
    {
    FloodSeed   floodSeed;

    floodSeed.m_pt = seedPoint;
    m_output.CollectFaceLoopsAtPoint(&floodSeed.m_faceNodeIds, floodSeed.m_pt, m_regionLoops, false);

    if (0 == floodSeed.m_faceNodeIds.size())
        return false;

    bool        added = false;

    for (size_t iFace = 0; iFace < floodSeed.m_faceNodeIds.size(); iFace++)
        {
        if (m_output.ToggleFaceLoop(floodSeed.m_faceNodeIds[iFace]) && 0 == iFace)
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
BentleyStatus   RegionGraphicsContext::AddFaceLoopsAtPoints(DPoint3dCP seedPoints, size_t numSeed)
    {
    for (size_t iSeed = 0; iSeed < numSeed; iSeed++)
        {
        FloodSeed   floodSeed;

        floodSeed.m_pt = seedPoints[iSeed];
        m_output.CollectFaceLoopsAtPoint(&floodSeed.m_faceNodeIds, floodSeed.m_pt, m_regionLoops, m_stepOutOfHoles);

        if (0 == floodSeed.m_faceNodeIds.size())
            continue;

        for (size_t iFace = 0; iFace < floodSeed.m_faceNodeIds.size(); iFace++)
            m_output.AddFaceLoop(floodSeed.m_faceNodeIds[iFace]);

        m_floodSeeds.push_back(floodSeed);
        }

    return (m_floodSeeds.empty() ? ERROR : SUCCESS);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   RegionGraphicsContext::PopulateGraph(DgnViewportP vp, DgnElementCPtrVec const* in)
    {
    m_operation = RegionType::Flood;
    m_setupScan = true;
    m_ignoreViewRange = false;

    if (SUCCESS != Attach(vp, m_purpose))
        return ERROR;

    //DMap4dCP    activeToRootMap = vp->GetActiveToRootMap (); 
    //
    //if (activeToRootMap)
    //    {
    //    Transform   rootToActiveTrans;
    //
    //    // Collect geometry in active coords instead of root...
    //    rootToActiveTrans.InitFrom (*activeToRootMap, true);
    //    _PushTransform (rootToActiveTrans);
    //    }

    if (in)
        {
        for (DgnElementCPtr curr : *in)
            {
            GeometricElementCP geomElement = (curr.IsValid() ? curr->ToGeometricElement() : nullptr);

            if (nullptr == geomElement)
                continue;

            VisitFloodCandidate(*geomElement, NULL);
            }
        }
    else
        {
        VisitAllViewElements(false, NULL);
        }

    Detach();
    m_targetModel = vp->GetViewController().GetTargetModel(); // Detach clears path/Current Model...

    if (WasAborted())
        return ERROR;

    return m_output.SetupGraph(m_gapTolerance, RegionLoops::Ignore != m_regionLoops);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   RegionGraphicsContext::PopulateGraph(DgnModelR targetModel, DgnElementCPtrVec const& in, TransformCP inTrans)
    {
    m_operation = RegionType::Flood;

    if (SUCCESS != SetTargetModel(targetModel))
        return ERROR;

    for (DgnElementCPtr curr : in)
        {
        GeometricElementCP geomElement = (curr.IsValid() ? curr->ToGeometricElement() : nullptr);

        if (nullptr != geomElement)
            VisitFloodCandidate(*geomElement, inTrans);

        if (nullptr != inTrans)
            inTrans++;
        }

    return m_output.SetupGraph(m_gapTolerance, RegionLoops::Ignore != m_regionLoops);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   RegionGraphicsContext::Flood(DgnModelR targetModel, DgnElementCPtrVec const& in, TransformCP inTrans, DPoint3dCP seedPoints, size_t numSeed)
    {
    m_operation = RegionType::Flood;

    if (SUCCESS != SetTargetModel(targetModel))
        return ERROR;

    for (DgnElementCPtr curr : in)
        {
        GeometricElementCP geomElement = (curr.IsValid() ? curr->ToGeometricElement() : nullptr);

        if (nullptr != geomElement)
            VisitFloodCandidate(*geomElement, inTrans);

        if (nullptr != inTrans)
            inTrans++;
        }

    if (SUCCESS != m_output.SetupGraph(m_gapTolerance, RegionLoops::Ignore != m_regionLoops))
        return ERROR;

    return AddFaceLoopsAtPoints(seedPoints, numSeed);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   RegionGraphicsContext::Boolean(DgnModelR targetModel, bvector<CurveVectorPtr> const& in, RegionType operation)
    {
    if (RegionType::Flood == operation)
        return ERROR;

    m_operation = operation;

    if (SUCCESS != SetTargetModel(targetModel))
        return ERROR;

    for (CurveVectorPtr const& curve: in)
        {
        if (!curve.IsValid())
            continue;

        m_output.ClipAndProcessCurveVector(*curve, false);
        }

    if (SUCCESS != m_output.SetupGraph(0.0, true))
        return ERROR;

    if (RegionType::Intersection == operation || RegionType::ExclusiveOr == operation)
        return m_output.CollectBooleanFaces(getRGBoolSelect(operation, true), m_output.GetCurrentGeomMarkerId(), m_output.GetCurrentGeomMarkerId());

    return m_output.CollectBooleanFaces(getRGBoolSelect(operation), 1, m_output.GetCurrentGeomMarkerId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   RegionGraphicsContext::Boolean(DgnModelR targetModel, DgnElementCPtrVec const& in, TransformCP inTrans, RegionType operation)
    {
    if (RegionType::Flood == operation)
        return ERROR;

    m_operation = operation;

    if (SUCCESS != SetTargetModel(targetModel))
        return ERROR;

    for (DgnElementCPtr curr : in)
        {
        GeometricElementCP geomElement = (curr.IsValid() ? curr->ToGeometricElement() : nullptr);

        if (nullptr != geomElement)
            VisitBooleanCandidate(*geomElement, inTrans, nullptr, true);

        if (nullptr != inTrans)
            inTrans++;
        }

    int highestOperand = m_output.GetCurrentGeomMarkerId();

    if (SUCCESS != m_output.SetupGraph(0.0, true))
        return ERROR;

    if (RegionType::Intersection == operation || RegionType::ExclusiveOr == operation)
        return m_output.CollectBooleanFaces(getRGBoolSelect(operation, true), highestOperand, highestOperand);

    return m_output.CollectBooleanFaces(getRGBoolSelect(operation), 1, highestOperand);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   RegionGraphicsContext::Boolean(DgnModelR targetModel, DgnElementCPtrVec const& target, DgnElementCPtrVec const& tool, TransformCP targetTrans, TransformCP toolTrans, RegionType operation)
    {
    if (RegionType::Flood == operation)
        return ERROR;

    m_operation = operation;

    if (SUCCESS != SetTargetModel(targetModel))
        return ERROR;

    for (DgnElementCPtr curr : target)
        {
        GeometricElementCP geomElement = (curr.IsValid() ? curr->ToGeometricElement() : nullptr);

        if (nullptr != geomElement)
            VisitBooleanCandidate(*geomElement, targetTrans, nullptr, false);

        if (nullptr != targetTrans)
            targetTrans++;
        }

    int highestOperandA = m_output.GetCurrentGeomMarkerId();

    for (DgnElementCPtr curr : tool)
        {
        GeometricElementCP geomElement = (curr.IsValid() ? curr->ToGeometricElement() : nullptr);

        if (nullptr != geomElement)
            VisitBooleanCandidate(*geomElement, toolTrans, nullptr, true);

        if (nullptr != toolTrans)
            toolTrans++;
        }

    int highestOperandB = m_output.GetCurrentGeomMarkerId();

    if (SUCCESS != m_output.SetupGraph(0.0, true))
        return ERROR;

    return m_output.CollectBooleanFaces(getRGBoolSelect(operation), highestOperandA, highestOperandB);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   RegionGraphicsContext::BooleanWithHoles(DgnModelR targetModel, DgnElementCPtrVec const& in, DgnElementCPtrVec const& holes, TransformCP inTrans, TransformCP holeTrans, RegionType operation)
    {
    if (RegionType::Flood == operation)
        return ERROR;

    m_operation = operation;

    if (SUCCESS != SetTargetModel(targetModel))
        return ERROR;

    bvector<DMatrix4d> wireProducts;

    for (DgnElementCPtr curr : in)
        {
        GeometricElementCP geomElement = (curr.IsValid() ? curr->ToGeometricElement() : nullptr);

        if (nullptr != geomElement)
            VisitBooleanCandidate(*geomElement, inTrans, m_cullRedundantLoop ? &wireProducts : nullptr);

        if (nullptr != inTrans)
            inTrans++;
        }

    int highestOperand = m_output.GetCurrentGeomMarkerId();

    for (DgnElementCPtr curr : holes)
        {
        GeometricElementCP geomElement = (curr.IsValid() ? curr->ToGeometricElement() : nullptr);

        if (nullptr != geomElement)
            VisitBooleanCandidate(*geomElement, holeTrans);

        if (nullptr != holeTrans)
            holeTrans++;
        }

    if (SUCCESS != m_output.SetupGraph(0.0, true))
        return ERROR;

    if (RegionType::Intersection == operation || RegionType::ExclusiveOr == operation)
        return m_output.CollectBooleanFaces(getRGBoolSelect(operation, true), highestOperand, highestOperand);

    return m_output.CollectBooleanFaces(getRGBoolSelect(operation), 1, highestOperand);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            RegionGraphicsContext::SetAbortFunction(RGC_AbortFunction abort)
    {
    m_output.SetAbortFunction(abort);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/09
+---------------+---------------+---------------+---------------+---------------+------*/
RegionGraphicsContextPtr RegionGraphicsContext::Create()
    {
    return new RegionGraphicsContext();
    }

