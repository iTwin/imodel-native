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

