/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnHandlers/HitDetail.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    05/01
+---------------+---------------+---------------+---------------+---------------+------*/
void GeomDetail::Init()
    {
    m_primitive     = nullptr;
    m_parentType    = HitParentGeomType::None;
    m_geomType      = HitGeomType::None;
    m_detailSource  = HitDetailSource::None;
    m_hitPriority   = HitPriority::Highest;
    m_nonSnappable  = false;
    m_viewDist      = 0.0;
    m_viewZ         = 0.0;

    m_geomId.Init();
    m_closePoint.Zero();
    m_normal.Zero();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/12
+---------------+---------------+---------------+---------------+---------------+------*/
ICurvePrimitiveCP GeomDetail::GetCurvePrimitive() const
    {
    return m_primitive.IsValid() ? m_primitive.get() : NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/13
+---------------+---------------+---------------+---------------+---------------+------*/
HitGeomType GeomDetail::GetCurvePrimitiveType() const
    {
    if (!m_primitive.IsValid())
        return HitGeomType::None;

    switch (m_primitive->GetCurvePrimitiveType())
        {
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line:
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString:
            return HitGeomType::Segment;

        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc:
            return HitGeomType::Arc;

        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_BsplineCurve:
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_InterpolationCurve:
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_AkimaCurve:
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Spiral:
            return HitGeomType::Curve;

        default:
            return HitGeomType::None;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/13
+---------------+---------------+---------------+---------------+---------------+------*/
HitGeomType GeomDetail::GetEffectiveHitGeomType() const
    {
    return (HitGeomType::Surface == m_geomType ? GetCurvePrimitiveType() : m_geomType);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/12
+---------------+---------------+---------------+---------------+---------------+------*/
void GeomDetail::SetCurvePrimitive(ICurvePrimitiveCP curve, TransformCP localToWorld, HitGeomType geomType)
    {
    m_primitive = nullptr; 
    m_geomType  = HitGeomType::None;

    if (!curve)
        {
        // Set geometry type override, HitGeomType::Point and HitGeomType::Surface are valid w/o curve...
        if (HitGeomType::Point == geomType || HitGeomType::Surface == geomType)
            m_geomType = geomType;
        return;
        }

    switch (curve->GetCurvePrimitiveType())
        {
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line:
            {
            DSegment3dCP  segment = curve->GetLineCP ();

            if (segment->point[0].IsEqual(segment->point[1])) // Check for zero length lines and don't store redundant primitive...
                {
                m_geomType = HitGeomType::Point;
                break;
                }

            m_primitive = curve->Clone();
            m_geomType  = HitGeomType::Segment;
            break;
            }

        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString:
            {
            bvector<DPoint3d> const* points = curve->GetLineStringCP ();

            if ((1 == points->size()) || (2 == points->size() && points->at(0).IsEqual(points->at(1))))
                {
                m_geomType = HitGeomType::Point;
                break;
                }

            m_primitive = curve->Clone();
            m_geomType  = HitGeomType::Segment;
            break;
            }

        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc:
            {
            m_primitive = curve->Clone();
            m_geomType  = HitGeomType::Arc;
            break;
            }

        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_BsplineCurve:
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_InterpolationCurve:
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_AkimaCurve:
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Spiral:
            {
            MSBsplineCurveCP bcurve = curve->GetProxyBsplineCurveCP();

            if (bcurve && 2 == bcurve->params.order) // An order 2 bspline curve should be treated as a linestring...
                {
                bvector<DPoint3d> poles;

                bcurve->GetUnWeightedPoles(poles);

                if (bcurve->params.closed)
                    poles.push_back(poles.at(0));

                m_primitive = ICurvePrimitive::CreateLineString(poles);
                m_geomType  = HitGeomType::Segment;

                CurvePrimitiveIdCP curveId = curve->GetId();

                if (nullptr != curveId)
                    m_primitive->SetId(curveId->Clone().get()); // Preserve curve topology id from source curve...
                break;
                }

            m_primitive = curve->Clone();
            m_geomType  = HitGeomType::Curve;
            break;
            }
        }

    if (m_primitive.IsValid() && nullptr != localToWorld && !localToWorld->IsIdentity())
        m_primitive->TransformInPlace(*localToWorld);

    // Set geometry type override...
    //  - HitGeomType::Point with CURVE_PRIMITIVE_TYPE_Arc denotes arc center...
    //  - HitGeomType::Surface with any curve primitive denotes an interior hit...
    if (HitGeomType::None != geomType)
        m_geomType = geomType;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/17
+---------------+---------------+---------------+---------------+---------------+------*/
static bool clipCurvePrimitive(bvector<ICurvePrimitivePtr>& partials, ICurvePrimitiveR curve, ClipVectorCR clip)
    {
    // NOTE: Modified from SimplifyCurveClipper which is more complicated as it needs clip a CurveVector...
    DRange3d curveRange;
                             
    if (!curve.GetRange(curveRange))
        return false;

    curveRange.Extend(DgnUnits::OneMillimeter());

    bvector<double> intersectParams;

    for (ClipPrimitivePtr const& thisClip : clip)
        {
        DRange3d clipRange;                                                                                             

        // Don't intersect with planes from clips that are disjoint from the current curve.
        if (thisClip->GetRange(clipRange, nullptr, true) && !curveRange.IntersectsWith(clipRange))
            continue;

        ClipPlaneSetCP clipPlaneSet;

        if (nullptr != (clipPlaneSet = thisClip->GetMaskOrClipPlanes())) // Use mask planes if they exist.
            {
            for (ConvexClipPlaneSetCR convexPlaneSet : *clipPlaneSet)
                {
                for (ClipPlaneCR plane : convexPlaneSet)
                    {
                    // NOTE: It would seem that it would not be necessary to calculate intersections with "interior" planes. However, we use 
                    //       that designation to mean any plane that should not generate cut geometry so "interior" is not really correct
                    //       and we need to intersect with these planes as well. Interior intersections do not really cause a problem as we 
                    //       discard them below if insidedness does not change. (RayB TR#244943)
                    bvector<CurveLocationDetailPair> intersections;
                
                    curve.AppendCurvePlaneIntersections(plane.GetDPlane3d(), intersections); // NOTE: Method calls clear in output vector!!!

                    // Get curve index for sorting, can disregard 2nd detail in pair as both should be identical...
                    for (CurveLocationDetailPair pair : intersections)
                        intersectParams.push_back(pair.detailA.fraction);
                    }
                }
            }
        }

    if (0 == intersectParams.size())
        {
        DPoint3d  testPoint;

        if (!curve.FractionToPoint(0.5, testPoint))
            return false;

        return !clip.PointInside(testPoint, 1.0e-5);
        }

    std::sort(intersectParams.begin(), intersectParams.end());
    intersectParams.push_back(1.0); // Add final param for end of curve...

    bool   lastInside = false;
    double insideStartParam = 0.0;
    double lastParam = 0.0;

    for (double thisParam : intersectParams)
        {
        if ((thisParam - lastParam) < 1.0e-4)
            continue;

        bool     thisInside = false;
        DPoint3d midPoint;

        if (curve.FractionToPoint((lastParam + thisParam) / 2.0, midPoint))
            thisInside = clip.PointInside(midPoint, 1.0e-5);

        if (thisInside)
            {
            if (!lastInside)
                insideStartParam = lastParam;
            }
        else
            {
            if (lastInside)
                {
                ICurvePrimitivePtr partialCurve = ICurvePrimitive::CreatePartialCurve(&curve, insideStartParam, lastParam);

                partials.push_back(partialCurve);
                }
            }

        lastParam  = thisParam;
        lastInside = thisInside;
        }

    if (lastInside)
        {
        ICurvePrimitivePtr partialCurve = ICurvePrimitive::CreatePartialCurve(&curve, insideStartParam, lastParam);

        partials.push_back(partialCurve);
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void GeomDetail::ClipCurvePrimitive(ClipVectorCP clip)
    {
    // NOTE: Snap point locations need to be computed from the clipped curve...
    if (nullptr == clip || !m_primitive.IsValid() || HitGeomType::Surface == m_geomType)
        return;

    double   fraction;
    DPoint3d closePoint;

    if (!m_primitive->ClosestPointBounded(m_closePoint, fraction, closePoint))
        return;

    bvector<ICurvePrimitivePtr> partials;

    if (!clipCurvePrimitive(partials, *m_primitive, *clip) || 0 == partials.size())
        return;

    int64_t tag;
    double fractionA, fractionB;
    ICurvePrimitivePtr parentCurve;

    for (ICurvePrimitivePtr& partialCurve : partials)
        {
        if (!partialCurve->TryGetPartialCurveData(fractionA, fractionB, parentCurve, tag))
            continue;

        if (fraction < fractionA || fraction > fractionB)
            continue;

        m_primitive = partialCurve->CloneDereferenced();
        break;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeomDetail::GetSegment(DSegment3dR segment) const
    {
    if (!m_primitive.IsValid())
        return false;

    switch (m_primitive->GetCurvePrimitiveType())
        {
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line:
            segment = *m_primitive->GetLineCP ();
            return true;

        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString:
            return m_primitive->TryGetSegmentInLineString(segment, GetSegmentNumber());

        default:
            return false;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeomDetail::GetArc(DEllipse3dR ellipse) const
    {
    if (!m_primitive.IsValid())
        return false;

    if (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc != m_primitive->GetCurvePrimitiveType())
        return false;

    ellipse = *m_primitive->GetArcCP ();

    return true;
    }
//! <ul>
//! <li>Return the current geometry as an ICurvePrimitive.
//! <li>If singleSegment is requested and the current geometry is a segment select within a linestring, only return that segment.
//! </ul>
DGNPLATFORM_EXPORT bool GeomDetail::GetCurvePrimitive (ICurvePrimitivePtr &curve, bool singleSegment) const
    {
    curve = nullptr;
    if (!m_primitive.IsValid())
        return false;

    DSegment3d  segment;

    if (singleSegment && GetSegment(segment))
        {
        curve = ICurvePrimitive::CreateLine (segment);
        }
    else
        {
        curve = m_primitive->Clone ();
        }
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/12
+---------------+---------------+---------------+---------------+---------------+------*/
size_t GeomDetail::GetSegmentNumber() const
    {
    if (!m_primitive.IsValid() || ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString != m_primitive->GetCurvePrimitiveType())
        return 0;

    bvector<DPoint3d> const* points = m_primitive->GetLineStringCP();

    if (points->size() < 3)
        return 0;
        
    double  fraction  = GetCloseParam();
    size_t  nSegments = (points->size()-1);
    double  uSegRange = (1.0 / nSegments);
    size_t  segmentNo = (size_t) (fraction / uSegRange);

    return (segmentNo >= nSegments ? nSegments-1 : segmentNo);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/12
+---------------+---------------+---------------+---------------+---------------+------*/
double GeomDetail::GetSegmentParam() const
    {
    if (!m_primitive.IsValid())
        return 0.0;
    
    if (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString != m_primitive->GetCurvePrimitiveType())
        return GetCloseParam();

    bvector<DPoint3d> const* points = m_primitive->GetLineStringCP ();

    if (points->size() < 3)
        return GetCloseParam();
        
    double  fraction  = GetCloseParam();
    size_t  nSegments = (points->size()-1);
    double  uSegRange = (1.0 / nSegments);
    size_t  segmentNo = (size_t) (fraction / uSegRange);
    double  segmParam = ((fraction - (uSegRange * (segmentNo >= nSegments ? nSegments-1 : segmentNo))) * nSegments);

    LIMIT_RANGE (0.0, 1.0, segmParam);

    return segmParam;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/12
+---------------+---------------+---------------+---------------+---------------+------*/
double GeomDetail::GetCloseParam() const
    {
    if (!m_primitive.IsValid())
        return 0.0;

    double      fraction;
    DPoint3d    curvePoint;

    if (!m_primitive->ClosestPointBounded(m_closePoint, fraction, curvePoint))
        return 0.0;

    return fraction;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/12
+---------------+---------------+---------------+---------------+---------------+------*/
size_t GeomDetail::GetCloseVertex() const
    {
    if (!m_primitive.IsValid())
        return 0;

    size_t  nSegments = 1;

    if (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString == m_primitive->GetCurvePrimitiveType())
        {
        bvector<DPoint3d> const* points = m_primitive->GetLineStringCP ();

        nSegments = ((points->size() < 3) ? 1 : (points->size()-1));
        }

    double  fraction  = GetCloseParam();
    double  uSegRange = (1.0 / nSegments);
    size_t  closeVertex = (size_t) ((fraction + (uSegRange*.5)) / uSegRange);

    return closeVertex;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/12
+---------------+---------------+---------------+---------------+---------------+------*/
size_t GeomDetail::GetPointCount() const
    {
    if (HitGeomType::Point == m_geomType)
        return 1;

    if (!m_primitive.IsValid())
        return 0;

    switch (m_primitive->GetCurvePrimitiveType())
        {
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line:
            return 2;

        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString:
            return m_primitive->GetLineStringCP ()->size();
        }

    return 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeomDetail::IsValidSurfaceHit() const
    {
    return 0.0 != GetSurfaceNormal().Magnitude();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeomDetail::IsValidEdgeHit() const
    {
    switch (GetGeomType())
        {
        case HitGeomType::Segment:
        case HitGeomType::Curve:
        case HitGeomType::Arc:
            return true;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/18
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeomDetail::GetCloseDetail(CurveLocationDetailR location) const
    {
    if (!m_primitive.IsValid())
        return false;

    size_t pointCount = GetPointCount();

    location.curve = GetCurvePrimitive();
    location.fraction = GetCloseParam();
    location.point = GetClosestPoint();
    location.componentIndex = GetSegmentNumber();
    location.numComponent = (pointCount > 2 ? pointCount-1 : 1);
    location.componentFraction = GetSegmentParam();
    location.a = GetScreenDist();

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/12
+---------------+---------------+---------------+---------------+---------------+------*/
CurvePrimitiveIdCP GeomDetail::GetCurvePrimitiveId() const
    {
    return m_primitive.IsValid() ? m_primitive->GetId() : nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/015
+---------------+---------------+---------------+---------------+---------------+------*/
HitDetail::HitDetail(DgnViewportR viewport, Sheet::Attachment::ViewportP sheetVp, GeometrySourceCP geomSource, DPoint3dCR testPoint, HitSource source, GeomDetailCR geomDetail) : m_viewport(viewport), m_sheetViewport(sheetVp)
    {
    m_elementId         = (nullptr != geomSource && nullptr != geomSource->ToElement() ? geomSource->ToElement()->GetElementId() : DgnElementId());
    m_locateSource      = source;
    m_testPoint         = testPoint;
    m_geomDetail        = geomDetail;
    m_subSelectionMode  = SubSelectionMode::None;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/04
+---------------+---------------+---------------+---------------+---------------+------*/
HitDetail::HitDetail(HitDetail const& from) : m_viewport(from.m_viewport), m_sheetViewport(from.m_sheetViewport) 
    {
    m_elementId         = from.m_elementId;
    m_locateSource      = from.m_locateSource;
    m_testPoint         = from.m_testPoint;
    m_geomDetail        = from.m_geomDetail;
    m_elemTopo          = from.m_elemTopo.IsValid() ? from.m_elemTopo->_Clone() : nullptr;
    m_subSelectionMode  = from.m_subSelectionMode;
    m_hitDescription    = from.m_hitDescription;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
HitDetail::~HitDetail() {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void HitDetail::_Draw(DecorateContextR context) const {context.DrawHit(*this);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void HitDetail::FlashGraphic(Render::GraphicR graphic, DecorateContextR context) const
    {
    ColorDef color = context.GetViewport()->GetHiliteColor();
    Render::OvrGraphicParams ovrParams;

    ovrParams.SetLineColor(color);
    ovrParams.SetFillColor(color);
    ovrParams.SetLineTransparency(0x40);
    ovrParams.SetFillTransparency(0x40);

    // NOTE: Pushing graphic towards eye was problematic depending on zoom (could end up outside project extents).
    //       Since we're mostly trying to optimize for "edge" hits here (flashing element is handled by Viewport::SetFlashed) we're
    //       always going to use a world overlay. Solids look ok drawn as overlay now, that wasn't the case with QVis...
    context.AddWorldOverlay(graphic, &ovrParams);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbR HitDetail::GetDgnDb() const
    {
    return m_viewport.GetViewController().GetDgnDb();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelP HitDetail::GetDgnModel() const
    {
    DgnElementCPtr element = GetElement();
    return element.IsValid() ? element->GetModel().get() : nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementCPtr HitDetail::GetElement() const
    {
    return GetDgnDb().Elements().GetElement(m_elementId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    06/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool HitDetail::_IsSameHit(HitDetailCP otherHit) const
    {
    if (nullptr == otherHit || m_elementId != otherHit->GetElementId())
        return false;

    if (!m_elemTopo.IsValid() && !otherHit->m_elemTopo.IsValid())
        return true;

    if (m_elemTopo.IsValid() != otherHit->m_elemTopo.IsValid())
        return false;

    if (!m_elemTopo->_IsEqual(*otherHit->m_elemTopo))
        return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String HitDetail::GetInfoString(Utf8CP delimiter) const
    {
    Utf8String out;
    if (m_hitDescription.IsValid())
        out = m_hitDescription->GetDescription() + delimiter;

    auto el = GetElement();
    if (el.IsValid())
        out += el->GetInfoString(delimiter);

    return out.Trim();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    05/01
+---------------+---------------+---------------+---------------+---------------+------*/
SnapDetail::SnapDetail(HitDetail const* from) : HitDetail(*from)
    {
    m_heat   = SNAP_HEAT_None;
    m_sprite = NULL;

    m_customKeypointSize = 0;
    m_customKeypointData = NULL;
    m_allowAssociations  = true;

    m_screenPt.x = m_screenPt.y = 0;
    m_snapPoint = m_geomDetail.GetClosestPoint();
    m_adjustedPt = m_snapPoint;
    m_snapMode = m_originalSnapMode = SnapMode::First;

    if (HitDetailType::Snap == from->GetHitType())
        {
        SnapDetailCP  fromSnap = (SnapDetailCP) from;

        m_minScreenDist = fromSnap->m_minScreenDist;
        }
    else
        {
        m_minScreenDist = m_geomDetail.GetScreenDist();
        m_geomDetail.SetScreenDist(0.0);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    05/01
+---------------+---------------+---------------+---------------+---------------+------*/
SnapDetail::SnapDetail(SnapDetail const& from) : HitDetail(from)
    {
    m_heat              = from.m_heat;
    m_screenPt          = from.m_screenPt;
    m_originalSnapMode  = from.m_originalSnapMode;
    m_divisor           = from.m_divisor;
    m_sprite            = from.m_sprite;
    m_snapMode          = from.m_snapMode;
    m_minScreenDist     = from.m_minScreenDist;
    m_snapPoint         = from.m_snapPoint;
    m_adjustedPt        = from.m_adjustedPt;
    m_allowAssociations = from.m_allowAssociations;

    if (m_sprite)
        m_sprite->AddRef();

    if (NULL == from.m_customKeypointData || 0 == from.m_customKeypointSize)
        {
        m_customKeypointSize = 0;
        m_customKeypointData = NULL;
        }
    else
        {
        m_customKeypointSize = from.m_customKeypointSize;
        m_customKeypointData = (Byte *) bentleyAllocator_malloc(m_customKeypointSize);

        memcpy(m_customKeypointData, from.m_customKeypointData, m_customKeypointSize);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    05/01
+---------------+---------------+---------------+---------------+---------------+------*/
SnapDetail::~SnapDetail()
    {
    if (m_sprite)
        m_sprite->Release();

    if (NULL != m_customKeypointData)
        bentleyAllocator_free(m_customKeypointData);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    ShaunSewall     12/11
+---------------+---------------+---------------+---------------+---------------+------*/
SnapDetail* SnapDetail::_Clone() const 
    {
    return new SnapDetail(*this);
    }

/*---------------------------------------------------------------------------------**//**
* @return  the "hit" point for this snapPath. If this path is "hot", this function will return
* the snapped point, otherwise it returns the closest point on the element.
* @bsimethod    HitDetail                                         KeithBentley    12/97
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint3dCR SnapDetail::_GetHitPoint() const
    {
    return IsHot() ? m_snapPoint : m_geomDetail.GetClosestPoint();
    }

/*---------------------------------------------------------------------------------**//**
* @return whether point has been adjusted
* @bsimethod    SnapDetail                                        BrienBastings   08/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool SnapDetail::PointWasAdjusted() const
    {
    return (!m_snapPoint.IsEqual(m_adjustedPt, 1.0e-10));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    05/01
+---------------+---------------+---------------+---------------+---------------+------*/
void SnapDetail::_SetHitPoint(DPoint3dCR hitPoint)
    {
    m_snapPoint  = hitPoint;
    m_adjustedPt = hitPoint;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    05/01
+---------------+---------------+---------------+---------------+---------------+------*/
static double   distSquaredXY(DPoint4dCR pVec1, DPoint4dCR pVec2)
    {
    DPoint3d    v1, v2;

    pVec1.GetProjectedXYZ(v1);
    pVec2.GetProjectedXYZ(v2);

    double dx = v1.x - v2.x;
    double dy = v1.y - v2.y;

    return dx * dx + dy * dy;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SamWilson       06/03
+---------------+---------------+---------------+---------------+---------------+------*/
static double   getDistanceFromSnap(SnapDetailCR snap)
    {
    DPoint3d    pts[2];
    DPoint4d    scrPts[2];

    pts[0] = snap.GetGeomDetail().GetClosestPoint();
    pts[1] = snap.GetSnapPoint();

    // NOTE: Use viewport to get active-to-view...
    snap.GetViewport().GetWorldToViewMap()->M0.Multiply(scrPts, pts, NULL, 2);

    return sqrt(distSquaredXY(scrPts[0], scrPts[1]));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/16
+---------------+---------------+---------------+---------------+---------------+------*/
void SnapDetail::SetSnapPoint(DPoint3dCR snapPt, bool forceHot, double hotDistance)
    {
    DPoint4d viewPt;

    m_viewport.GetWorldToViewMap()->M0.Multiply(&viewPt, &snapPt, NULL, 1);
    viewPt.NormalizeWeightInPlace();

    Point2d screenPt;
    screenPt.x = (long) viewPt.x;
    screenPt.y = (long) viewPt.y;

    SetScreenPoint(screenPt);
    SetHitPoint(snapPt);

    double screenDist = getDistanceFromSnap(*this);
    GetGeomDetailW().SetScreenDist(screenDist);

    bool withinAperture = (screenDist <= hotDistance);
    SetHeat(withinAperture ? SNAP_HEAT_InRange : (forceHot ? SNAP_HEAT_NotInRange : SNAP_HEAT_None));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    06/01
+---------------+---------------+---------------+---------------+---------------+------*/
IntersectDetail::IntersectDetail(HitDetailCP firstHit, HitDetailCP secondHit, DPoint3dCR pt) : SnapDetail(firstHit)
    {
    m_secondHit = const_cast<HitDetailP>(secondHit);

    if (m_secondHit)
        m_secondHit->AddRef();

    SetHitPoint(pt);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    06/01
+---------------+---------------+---------------+---------------+---------------+------*/
IntersectDetail::IntersectDetail(IntersectDetail const& i2) : SnapDetail(i2)
    {
    m_secondHit = i2.m_secondHit;
    m_secondHit->AddRef();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    06/01
+---------------+---------------+---------------+---------------+---------------+------*/
IntersectDetail::~IntersectDetail()
    {
    if (m_secondHit)
        m_secondHit->Release();

    m_secondHit = nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    ShaunSewall     12/11
+---------------+---------------+---------------+---------------+---------------+------*/
SnapDetail* IntersectDetail::_Clone() const 
    {
    return new IntersectDetail(*this);
    }

/*---------------------------------------------------------------------------------**//**
* determine whether this is the "same" hit as another one. For InteresectPaths, that means that
* both paths of both hits are the same.
* @bsimethod                                                    KeithBentley    06/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool IntersectDetail::_IsSameHit(HitDetailCP otherPath) const
    {
    // check base paths
    if (!T_Super::_IsSameHit(otherPath))
        return false;

    // for IntersectDetails, it can't be the same hit unless both are of type InteresctPath and BOTH paths match
    if (HitDetailType::Intersection != otherPath->GetHitType())
        return false;

    // now check the "second" paths
    HitDetailCP o2 = ((IntersectDetail const*) otherPath)->GetSecondHit();

    return GetSecondHit()->IsSameHit(o2);
    }

/*---------------------------------------------------------------------------------**//**
* IntersctPaths override the "DrawInView" method to hilite/unhilte BOTH paths that are part of the
* intersction. The "base" path is drawn using the drawmode of the call, but the "second" path
* is drawn using a dashed symbology.
* @bsimethod                                                    KeithBentley    06/01
+---------------+---------------+---------------+---------------+---------------+------*/
void IntersectDetail::_Draw(DecorateContextR context) const
    {
    // start by drawing the first path normally
    T_Super::_Draw(context);

    SnapDetail tmpSnapDetail(m_secondHit); // So display handlers know this is from a snap...

#if defined(WIP_HILITE)
    // NOTE: When we're flashing, the hilite flags are not necessarily set on the elements. So to get the second path
    //       drawn hilited, we need to turn on its hilited flag temporarily, and then restore it.
    bool currHilite = tmpSnapDetail.IsHilited();

    tmpSnapDetail.SetHilited(true);
#endif
    tmpSnapDetail.SetSubSelectionMode(GetSubSelectionMode()); // Set correct flash mode...
    tmpSnapDetail.Draw(context);

#if defined(WIP_HILITE)
    tmpSnapDetail.SetHilited(currHilite);
#endif
    }

/*=================================================================================**//**
* The result of a "locate" is a sorted list of objects that
* satisfied the search  criteria (a HitList). Earlier hits in the list
* are somehow "better" than those later on.
* @see          IHitDetail
* @bsiclass                                                     KeithBentley    12/97
+===============+===============+===============+===============+===============+======*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    05/01
+---------------+---------------+---------------+---------------+---------------+------*/
HitList::HitList() {m_currHit = -1;}
HitList::~HitList() {clear();}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      01/2008
+---------------+---------------+---------------+---------------+---------------+------*/
HitDetailP HitList::Get(int i)
    {
    if (i < 0)                  // ***NEEDS WORK: the old ObjectArray used to support -1 == END
        i = (int) size();

    if (i >= (int) GetCount())
        return NULL;

    return at(i).get();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      01/2008
+---------------+---------------+---------------+---------------+---------------+------*/
void HitList::Set(int i, HitDetailP p)
    {
    if (i < 0 || i >= (int) GetCount())
        {
        BeAssert(false);
        return;
        }
    at(i) = p;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      01/2008
+---------------+---------------+---------------+---------------+---------------+------*/
void HitList::Insert(int i, HitDetailP p)
    {
    if (i < 0 || i == (int)size())
        push_back(p);
    else
        insert(begin()+i, p);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      01/2008
+---------------+---------------+---------------+---------------+---------------+------*/
void HitList::DropNulls()
    {
    erase(std::remove(begin(), end(), (HitDetailP)NULL), end());
    }

/*---------------------------------------------------------------------------------**//**
* Drop all entries in the HitList. The reference count of all contained hits is decremented
* before they are dropped. Clears the "current hit" index.
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
void HitList::Empty()
    {
    // empty the list (also decrements ref counts of entries)
    clear();

    // we don't have a current hit.
    m_currHit = -1;
    }

/*---------------------------------------------------------------------------------**//**
* remove the first hit in the list.
* @bsimethod    Locate.Hitlist                                  KeithBentley    12/97
+---------------+---------------+---------------+---------------+---------------+------*/
void HitList::RemoveHit(int hitNum)
    {
    if (hitNum < 0)                     // *** NEEDS WORK: The old ObjectArray used to support -1 == END
        hitNum = (int) size() - 1;

    if (hitNum == m_currHit)
        m_currHit = -1;

    if (hitNum >= (int) size())        // Locate calls GetNextHit, which increments m_currHit, until it goes beyond the end of size of the array.
        return;                         // Then Reset call RemoteCurrentHit, which passes in m_currHit. When it's out of range, we do nothing.

    erase(begin() + hitNum);
    }

/*---------------------------------------------------------------------------------**//**
* get a hit from a particular index into a HitList
* @return       the requested hit from the HitList
* @bsimethod    Locate.Hitlist                                  KeithBentley    12/97
+---------------+---------------+---------------+---------------+---------------+------*/
HitDetailP HitList::GetHit(int hitNum) const
    {
    if (hitNum < 0)                     // *** NEEDS WORK: The old ObjectArray used to support -1 == END
        hitNum = (int) size() - 1;

    if (hitNum >= (int) size())
        return NULL;

    return  at(hitNum).get();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/07
+---------------+---------------+---------------+---------------+---------------+------*/
void HitList::RemoveCurrentHit()
    {
    RemoveHit(m_currHit);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/07
+---------------+---------------+---------------+---------------+---------------+------*/
void HitList::SetCurrentHit(HitDetailCP hit)
    {
    ResetCurrentHit();

    for (HitDetailCP thisHit; NULL != (thisHit=GetNextHit());)
        {
        if (thisHit == hit)
            return;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    06/02
+---------------+---------------+---------------+---------------+---------------+------*/
static inline double tenthOfPixel(double inValue) {return ((int) ((inValue * 10.0) + 0.5)) / 10.0;}

#define COMPARE_RELATIVE(a,b) {if (a<b) return -1; if (a>b) return 1;}

static const double s_tooCloseTolerance = 1.0e-10;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/12
+---------------+---------------+---------------+---------------+---------------+------*/
static int doZCompareOfSurfaceAndEdge(HitDetailCR oHitSurf, HitDetailCR oHitEdge)
    {
    DgnViewportR vp = oHitSurf.GetViewport();
    DPoint4d     homogeneousPlane;
    
    if (!homogeneousPlane.PlaneFromOriginAndNormal(oHitSurf.GetGeomDetail().GetClosestPoint(), oHitSurf.GetGeomDetail().GetSurfaceNormal()))
        return 0;

    DMap4d      worldToViewMap = *vp.GetWorldToViewMap();
    DPoint4d    eyePointWorld;

    worldToViewMap.M1.GetColumn(eyePointWorld, 2);

    DPoint3d    testPointWorld = oHitEdge.GetGeomDetail().GetClosestPoint();

    double  a0 = homogeneousPlane.DotProduct(eyePointWorld);
    double  a1 = homogeneousPlane.DotProduct(testPointWorld, 1.0);
    double  tol = s_tooCloseTolerance * (1.0 + fabs(a0) + fabs(a1) + fabs(homogeneousPlane.w));

#if defined (NOT_NOT_DUMP)
    if (fabs(a1) < tol)
        printf("Draw: %lf %lf (%lf)\n\n", a0, a1, tol);
    else if (a0 * a1 > 0)
        printf("Edge wins: %lf %lf (%lf)\n\n", a0, a1, tol);
    else
        printf("Surface wins: %lf %lf (%lf)\n\n", a0, a1, tol);
#endif

    if (fabs(a1) < tol)
        return 0;

    return ((a0 * a1 > 0) ? 1 : -1);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/12
+---------------+---------------+---------------+---------------+---------------+------*/
static int doZCompare(HitDetailCR oHit1, HitDetailCR oHit2)
    {
    double  z1 = oHit1.GetGeomDetail().GetZValue();
    double  z2 = oHit2.GetGeomDetail().GetZValue();

    // For 2d hits z reflects display priority which should be checked before locate priority, etc. when a fill/surface hit is involved...
    if (!oHit1.GetViewport().Is3dView())
        {
        // screen z values are sorted descending
        COMPARE_RELATIVE (z2, z1);

        return 0;
        }

    // Point clouds already output only a single best z for a screen location...only compare using screen distance, not z...
    if (HitDetailSource::PointCloud == oHit1.GetGeomDetail().GetDetailSource() && HitDetailSource::PointCloud == oHit2.GetGeomDetail().GetDetailSource())
        return 0;

    // Always prioritize sprites (ex. HUD markers) over surface hits...
    if (HitDetailSource::Sprite == oHit1.GetGeomDetail().GetDetailSource() || HitDetailSource::Sprite == oHit2.GetGeomDetail().GetDetailSource())
        return 0;

    DVec3d  normal1 = oHit1.GetGeomDetail().GetSurfaceNormal();
    DVec3d  normal2 = oHit2.GetGeomDetail().GetSurfaceNormal();

    // NOTE: Only surfaces display hidden edges...NEEDSWORK: Nothing is hidden by transparent display style (RenderMode::SmoothShade)...
    bool hiddenEdgesVisible = oHit1.GetViewport().GetViewFlags().HiddenEdgesVisible();
    bool isObscurableWireHit1 = (RenderMode::Wireframe != oHit1.GetViewport().GetViewFlags().GetRenderMode() && HitParentGeomType::Wire == oHit1.GetGeomDetail().GetParentGeomType());
    bool isObscurableWireHit2 = (RenderMode::Wireframe != oHit2.GetViewport().GetViewFlags().GetRenderMode() && HitParentGeomType::Wire == oHit2.GetGeomDetail().GetParentGeomType());

    if (0.0 != normal1.Magnitude() && 0.0 != normal2.Magnitude())
        {
        // Both surface hits...if close let other criteria determine order...
        if (DoubleOps::WithinTolerance(z1, z2, s_tooCloseTolerance))
            return 0;
        }
    else if (0.0 != normal1.Magnitude())
        {
        // 1st is surface hit...project 2nd hit into plane defined by surface normal...
        int compareResult = (hiddenEdgesVisible && !isObscurableWireHit2) ? 1 : doZCompareOfSurfaceAndEdge(oHit1, oHit2);

        return (0 == compareResult ? 0 : compareResult);
        }
    else if (0.0 != normal2.Magnitude())
        {
        // 2nd is surface hit...project 1st hit into plane defined by surface normal...
        int compareResult = (hiddenEdgesVisible && !isObscurableWireHit1) ? 1 : doZCompareOfSurfaceAndEdge(oHit2, oHit1);

        return (0 == compareResult ? 0 : -compareResult);
        }
    else
        {
        // NOTE: I don't believe this case currently exists...silhouette hits are only created for cones/spheres and always have a curve primitive...
        bool isSilhouetteHit1 = (HitGeomType::Surface == oHit1.GetGeomDetail().GetGeomType() && NULL == oHit1.GetGeomDetail().GetCurvePrimitive());
        bool isSilhouetteHit2 = (HitGeomType::Surface == oHit2.GetGeomDetail().GetGeomType() && NULL == oHit2.GetGeomDetail().GetCurvePrimitive());

        // NOTE: Likely silhouette hit, make sure it always loses to a real edge hit...
        if (isSilhouetteHit1 && !isSilhouetteHit2)
            return 1;
        if (isSilhouetteHit2 && !isSilhouetteHit1)
            return -1;
        if (DoubleOps::WithinTolerance(z1, z2, s_tooCloseTolerance))
            return 0; // Both silhouette or real edge hits...if close let other criteria determine order...
        }

    // screen z values are sorted descending
    COMPARE_RELATIVE (z2, z1);

    return 0;
    }

/*---------------------------------------------------------------------------------**//**
* compare two hits for insertion into list. Hits are compared by
* calling GetLocatePriority() and then GetLocateDistance() on each.
* @bsimethod    Locate.Hitlist                                  KeithBentley    12/97
+---------------+---------------+---------------+---------------+---------------+------*/
int HitList::Compare(HitDetailCP oHit1, HitDetailCP oHit2, bool comparePriority, bool compareZ) const
    {
    if (NULL == oHit1 || NULL == oHit2)
        return 0;

    if (compareZ)
        {
        int zCompareValue = doZCompare(*oHit1, *oHit2);

        if (0 != zCompareValue)
            return zCompareValue;
        }

    if (comparePriority)
        {
        int p1 = static_cast<int>(oHit1->GetGeomDetail().GetLocatePriority());
        int p2 = static_cast<int>(oHit2->GetGeomDetail().GetLocatePriority());

        COMPARE_RELATIVE (p1, p2);
        }

    double dist1 = tenthOfPixel(oHit1->GetGeomDetail().GetScreenDist());
    double dist2 = tenthOfPixel(oHit2->GetGeomDetail().GetScreenDist());

    COMPARE_RELATIVE (dist1, dist2);

    // Linestyle/pattern/thickness hits have lower priority...
    COMPARE_RELATIVE (oHit1->GetGeomDetail().GetDetailSource(), oHit2->GetGeomDetail().GetDetailSource());

    return 0;
    }

/*---------------------------------------------------------------------------------**//**
* Add a new hit to the list. Hits are sorted according to their priority and distance.
* @bsimethod    Locate.Hitlist                                  KeithBentley    12/97
+---------------+---------------+---------------+---------------+---------------+------*/
int HitList::AddHit (HitDetail* newHit, bool allowDuplicates, bool comparePriority)
    {
#if defined (NOT_NOW_USE_REVERSE_ITERATOR)
    HitList::iterator currHit = begin();

    HitDetail*  oldHit;
    int         count = GetCount();
    int         index = 0;
    int         comparison;

    while (index < count)
        {
        oldHit = (HitDetail*) currHit->get();

        comparison = Compare(newHit, oldHit, comparePriority, true);

        // Caller can establish a policy to only ever allow one hit for a given path. However, we want to make sure
        // that the one hit we do save is the "best" hit for that path. Therefore, every time we get another hit
        // for a path, we drop the one with the lower value based on the comparison, and save the other one.
        if (!allowDuplicates && newHit->IsSameHit(oldHit))
            {
            // replace with new hit if it's better (otherwise just ignore it).
            if (comparison < 0)
                *currHit = newHit;

            return count;
            }

        if (comparison < 0)
            break;

        currHit++;
        index++;
        }

    // this increments ref count of newHit
    insert(begin()+index, newHit);
#else
    // NOTE: Starting from the end ensures that all edge hits will get compared against surface hits to properly
    //       determine their visiblity. With a forward iterator, an edge hit could end up being chosen that is obscured
    //       if it is closer to the eye than an un-obscured edge hit.
    size_t index = 0;

    for (auto currHit = rbegin(); currHit != rend(); ++currHit)
        {
        int comparison = Compare(currHit->get(), newHit, comparePriority, true);
        if (comparison >= 0)
            continue;
        index = std::distance(begin(), currHit.base());
        break;
        }

    // this increments ref count of newHit
    insert(begin()+index, newHit);
#endif

#if defined (NOT_NOT_DUMP)
    printf("HIT LIST COUNT: %d\n", GetCount());

    HitDetailP    thisHit;

    for (int i=0; NULL != (thisHit = (HitDetailP) GetHit(i)); i++)
        printf("(%d) Elem: %llu, GeomType: %d Z: %lf Normal: (%lf %lf %lf)\n", i, (long long unsigned int) thisHit->GetElementId().GetValue(), thisHit->GetGeomDetail().GetGeomType(), thisHit->GetGeomDetail().GetZValue(), thisHit->GetGeomDetail().GetSurfaceNormal().x, thisHit->GetGeomDetail().GetSurfaceNormal().y, thisHit->GetGeomDetail().GetSurfaceNormal().z);

    printf("\n\n");
#endif

    return index;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    05/01
+---------------+---------------+---------------+---------------+---------------+------*/
HitDetailP HitList::GetCurrentHit() const
    {
    if (-1 == m_currHit)
        return NULL;

    return GetHit(m_currHit);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    05/01
+---------------+---------------+---------------+---------------+---------------+------*/
HitDetailP HitList::GetNextHit()
    {
    m_currHit++;

    return GetCurrentHit();
    }

/*---------------------------------------------------------------------------------**//**
* search through hitlist and remove any hits that match a specified path.
* @bsimethod                                                    KeithBentley    06/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool HitList::RemoveHitsFrom(HitDetailCR hit)
    {
    HitDetailP  thisHit;
    bool        removedOne = false;

    // walk backwards through list so we don't have to worry about what happens on remove
    for (int i=GetCount()-1; i>=0; i--)
        {
        if ((nullptr != (thisHit = GetHit(i))) && thisHit->IsSameHit(&hit))
            {
            removedOne = true;
            RemoveHit(i);
            }
        }
    
    return removedOne;
    }

/*---------------------------------------------------------------------------------**//**
* search through hitlist and remove any hits that contain a specified elementRef.
* @bsimethod                                                    KeithBentley    06/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool HitList::RemoveHitsFrom(DgnElementCR element)
    {
    HitDetailP  thisHit;
    bool        removedOne = false;

    // walk backwards through list so we don't have to worry about what happens on remove
    for (int i=GetCount()-1; i>=0; i--)
        {
        if ((nullptr != (thisHit = GetHit(i))) && thisHit->GetElementId() == element.GetElementId())
            {
            removedOne = true;
            RemoveHit(i);
            }
        }

    return removedOne;
    }

/*---------------------------------------------------------------------------------**//**
* search through hitlist and remove any hits from the specified model
* @bsimethod                                                    KeithBentley    06/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool HitList::RemoveHitsFrom(DgnModelR model)
    {
    HitDetailP  thisHit;
    bool        removedOne = false;

    // walk backwards through list so we don't have to worry about what happens on remove
    for (int i=GetCount()-1; i>=0; i--)
        {
        if ((NULL != (thisHit = GetHit(i))) && &model == thisHit->GetDgnModel())
            {
            removedOne = true;
            RemoveHit(i);
            }
        }

    return removedOne;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod    Locate.Hitlist                                  KeithBentley    12/97
+---------------+---------------+---------------+---------------+---------------+------*/
void HitList::Dump(WCharCP label) const
    {
    printf("%ls %d", label, GetCount());

    HitDetailP thisHit;

    for (int i=0; NULL != (thisHit = GetHit(i)); i++)
        printf("\n -> ElementId : %llu", (long long unsigned int) thisHit->GetElementId().GetValue());

    printf("\n");
    }
