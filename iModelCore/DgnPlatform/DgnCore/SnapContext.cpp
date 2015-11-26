/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/SnapContext.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/12
+---------------+---------------+---------------+---------------+---------------+------*/
static bool isPhysicallyClosed (ICurvePrimitiveCR primitive)
    {
    switch (primitive.GetCurvePrimitiveType())
        {
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString:
            return (primitive.GetLineStringCP()->size () > 3 && primitive.GetLineStringCP()->front().IsEqual (primitive.GetLineStringCP()->back()));

        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc:
            return primitive.GetArcCP()->IsFullEllipse();

        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_BsplineCurve:
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_InterpolationCurve:
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_AkimaCurve:
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Spiral:
            return (primitive.GetProxyBsplineCurveCP()->IsClosed());

        default:
            return false;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/12
+---------------+---------------+---------------+---------------+---------------+------*/
static bool getAreaCentroid (DPoint3dR centroid, CurveVectorCR curve)
    {
    Transform   localToWorld, worldToLocal;
    DRange3d    localRange;

    if (!curve.IsPlanar (localToWorld, worldToLocal, localRange))
        return false;

    DVec3d      normal;
    double      area;

    return curve.CentroidNormalArea (centroid, normal, area);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/12
+---------------+---------------+---------------+---------------+---------------+------*/
static bool getCentroid (DPoint3dR centroid, ICurvePrimitiveCR primitive)
    {
    if (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc == primitive.GetCurvePrimitiveType ())
        {
        // Center snap always uses arc center...
        centroid = primitive.GetArcCP ()->center;

        return true;
        }
    else if (isPhysicallyClosed (primitive))
        {
        CurveVectorPtr  curve = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Outer);
        
        curve->push_back (primitive.Clone ());

        // For physically closed/planar curve use area centroid instead of wire centroid...
        if (getAreaCentroid (centroid, *curve))
            return true;
        }

    double  length;

    return primitive.WireCentroid (length, centroid);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/12
+---------------+---------------+---------------+---------------+---------------+------*/
SnapStatus      SnapContext::DoSnapUsingCurve (SnapMode snapMode)
    {
    SnapDetailCP       snap = GetSnapDetail ();
    GeomDetailCR       detail = snap->GetGeomDetail ();
    ICurvePrimitiveCP  curve;

    if (NULL == (curve = detail.GetCurvePrimitive ()))
        return SnapStatus::NotSnappable;

    switch (snapMode)
        {
        case SnapMode::Origin:
            {
            DPoint3d    hitPoint;

            if (!curve->GetStartPoint (hitPoint))
                return SnapStatus::NotSnappable;

            SetSnapInfo (snapMode, GetSnapSprite (snapMode), hitPoint, false, false);

            return SnapStatus::Success;
            }

        case SnapMode::MidPoint:
            {
            switch (detail.GetEffectiveHitGeomType ())
                {
                case HitGeomType::Point:
                    {
                    DPoint3d    hitPoint = detail.GetClosestPoint();

                    SetSnapInfo (snapMode, GetSnapSprite (snapMode), hitPoint, false, false);

                    return SnapStatus::Success;
                    }

                case HitGeomType::Segment:
                    {
                    DSegment3d  segment;

                    if (!detail.GetSegment (segment))
                        return SnapStatus::NotSnappable;

                    DPoint3d    hitPoint;

                    hitPoint.Interpolate (segment.point[0], 0.5, segment.point[1]);
                    SetSnapInfo (snapMode, GetSnapSprite (snapMode), hitPoint, false, false);

                    return SnapStatus::Success;
                    }

                case HitGeomType::Arc:
                    {
                    DEllipse3dCP  ellipse = curve->GetArcCP ();

                    if (!ellipse)
                        return SnapStatus::NotSnappable;

                    DPoint3d    hitPoint;

                    ellipse->FractionParameterToPoint (hitPoint, 0.5);
                    SetSnapInfo (snapMode, GetSnapSprite (snapMode), hitPoint, false, false);

                    return SnapStatus::Success;
                    }
                }

            // Fall through to bisector if not handled...
            }

        case SnapMode::Bisector:
            {
            double  length;

            if (!curve->Length (length))
                return SnapStatus::NotSnappable;

            CurveLocationDetail location;

            if (!curve->PointAtSignedDistanceFromFraction (0.0, length * 0.5, false, location))
                return SnapStatus::NotSnappable;

            SetSnapInfo (snapMode, GetSnapSprite (snapMode), location.point, false, false);

            return SnapStatus::Success;
            }

        case SnapMode::Center:
            {
            DPoint3d    centroid;

            if (!getCentroid (centroid, *curve))
                return SnapStatus::NotSnappable;

            SetSnapInfo (snapMode, GetSnapSprite (snapMode), centroid, true /* force hot */, false);

            return SnapStatus::Success;
            }

        case SnapMode::Nearest:
            {
            DPoint3d    hitPoint = detail.GetClosestPoint(); // Current snap info is for nearest...just need to set snap using current point.

            SetSnapInfo (snapMode, GetSnapSprite (snapMode), hitPoint, false, false);

            return SnapStatus::Success;
            }

        case SnapMode::NearestKeypoint:
            {
            switch (detail.GetEffectiveHitGeomType ())
                {
                case HitGeomType::Point:
                    {
                    DPoint3d    hitPoint = detail.GetClosestPoint();

                    SetSnapInfo (snapMode, GetSnapSprite (snapMode), hitPoint, false, false);

                    return SnapStatus::Success;
                    }

                case HitGeomType::Segment:
                    {
                    DSegment3d  segment;

                    if (!detail.GetSegment (segment))
                        return SnapStatus::NotSnappable;

                    double      keyparam = detail.GetSegmentParam ();
                    DPoint3d    hitPoint;

                    GetSegmentKeypoint (hitPoint, keyparam, GetSnapDivisor(), segment);
                    SetSnapInfo (snapMode, GetSnapSprite (snapMode), hitPoint, false, false);

                    return SnapStatus::Success;
                    }

                case HitGeomType::Arc:
                case HitGeomType::Curve:
                    {
                    double      keyparam = detail.GetCloseParam ();
                    DPoint3d    hitPoint;

                    if (!GetParameterKeypoint (hitPoint, keyparam, GetSnapDivisor()))
                        return SnapStatus::NotSnappable;

                    SetSnapInfo (snapMode, GetSnapSprite (snapMode), hitPoint, false, false);

                    return SnapStatus::Success;
                    }

                default:
                    return SnapStatus::NotSnappable;
                }
            }

        default:
            {
            // Should never be called with "exotic" snap modes...
            return SnapStatus::NotSnappable;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   05/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool            SnapContext::GetParameterKeypoint (DPoint3dR hitPoint, double& keyParam, int divisor)
    {
    SnapDetailCP       snap = GetSnapDetail ();
    GeomDetailCR       detail = snap->GetGeomDetail ();
    ICurvePrimitiveCP  curve;

    if (NULL == (curve = detail.GetCurvePrimitive ()))
        return false;

    double  length, subLength;

    if (!curve->Length (length) || !curve->SignedDistanceBetweenFractions (0.0, keyParam, subLength))
        return false;

    // NOTE: For a closed curve we want quadrant key points.
    if (isPhysicallyClosed (*curve))
        divisor *= 4;

    int     keySeg = int ((subLength / length) * (double) divisor + 0.5);
    double  keyDist = (keySeg / (double) divisor) * length;

    CurveLocationDetail location;

    if (!curve->PointAtSignedDistanceFromFraction (0.0, keyDist, false, location))
        return false;

    hitPoint = location.point;
    keyParam = location.fraction;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/04
+---------------+---------------+---------------+---------------+---------------+------*/
void            SnapContext::GetSegmentKeypoint (DPoint3dR hitPoint, double& keyParam, int divisor, DSegment3dCR segment)
    {
    int         numerator = int ((keyParam * (double) divisor) + 0.5);
    
    keyParam = (numerator / (double) divisor);
    hitPoint.Interpolate (segment.point[0], keyParam, segment.point[1]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/04
+---------------+---------------+---------------+---------------+---------------+------*/
void            SnapContext::SetSnapInfo
(
SnapMode        snapMode,
ISpriteP        sprite,
DPoint3dCR      snapPoint, // In world coords...
bool            forceHot,
bool            isAdjusted,
int             nBytes,
Byte*           customKeypointData
)
    {
    SnapDetailP snap = GetSnapDetail ();

    snap->SetSnapMode (snapMode);
    snap->SetSprite (sprite);

    SetSnapPoint (snapPoint, forceHot);
    snap->SetAllowAssociations (!isAdjusted);

    if (nBytes && customKeypointData)
        snap->SetCustomKeypoint (nBytes, customKeypointData);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    05/01
+---------------+---------------+---------------+---------------+---------------+------*/
static double   distSquaredXY (DPoint4dCR pVec1, DPoint4dCR pVec2)
    {
    DPoint3d    v1, v2;

    pVec1.GetProjectedXYZ (v1);
    pVec2.GetProjectedXYZ (v2);

    double dx = v1.x - v2.x;
    double dy = v1.y - v2.y;

    return dx * dx + dy * dy;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SamWilson       06/03
+---------------+---------------+---------------+---------------+---------------+------*/
static double   getDistanceFromSnap (SnapDetailCR hit, ViewContextP context)
    {
    DPoint3d    pts[2];
    DPoint4d    scrPts[2];

    pts[0] = hit.GetGeomDetail().GetClosestPoint();
    pts[1] = hit.GetSnapPoint();

    // NOTE: Use viewport to get active-to-view...
    context->GetViewport()->GetWorldToViewMap()->M0.Multiply (scrPts, pts, NULL, 2);

    return sqrt (distSquaredXY (scrPts[0], scrPts[1]));
    }

/*---------------------------------------------------------------------------------**//**
* save the snap point into the snap path currently being generated.
* @bsimethod                                                    KeithBentley    05/01
+---------------+---------------+---------------+---------------+---------------+------*/
void            SnapContext::SetSnapPoint (DPoint3dCR snapPt, bool forceHot)
    {
    DPoint4d    viewPt;

    m_viewport->GetWorldToViewMap()->M0.Multiply (&viewPt, &snapPt, NULL, 1);

    viewPt.NormalizeWeightInPlace ();

    Point2d     screenPt;
    screenPt.x = (long) viewPt.x;
    screenPt.y = (long) viewPt.y;

    m_snapPath->SetScreenPoint (screenPt);
    m_snapPath->SetHitPoint (snapPt);

    double  screenDist = getDistanceFromSnap (*m_snapPath, this);
    m_snapPath->GetGeomDetailW ().SetScreenDist (screenDist);

    bool    withinAperture = (screenDist <= m_snapAperture);
    m_snapPath->SetHeat (withinAperture ? SNAP_HEAT_InRange : (forceHot ? SNAP_HEAT_NotInRange : SNAP_HEAT_None));
    }

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct SnapGraphicsProcessor : IElementGraphicsProcessor
{
private:

SnapContextR        m_snapContext;
CurveLocationDetail m_location;
bool                m_isVisible;
ViewContextP        m_context;
Transform           m_currentTransform;

protected:

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   11/13
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool _ProcessAsBody (bool isCurved) const override {return false;}
virtual bool _ProcessAsFacets (bool isPolyface) const override {return isPolyface;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   11/13
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _AnnounceContext (ViewContextR context) override {m_context = &context;}
virtual void _AnnounceTransform (TransformCP trans) override {if (trans) m_currentTransform = *trans; else m_currentTransform.InitIdentity ();}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool IsEdgePointVisible (DPoint3dCR edgePointWorld, SnapDetailCR snap)
    {
    // See HitList::Compare doZCompareOfSurfaceAndEdge...
    DgnViewportR vp = snap.GetViewport();
    DPoint4d     homogeneousPlane;
    
    if (!homogeneousPlane.PlaneFromOriginAndNormal(snap.GetGeomDetail().GetClosestPoint(), snap.GetGeomDetail().GetSurfaceNormal()))
        return true;

    DMap4d      worldToViewMap = *vp.GetWorldToViewMap();
    DPoint4d    eyePointWorld;

    worldToViewMap.M1.GetColumn (eyePointWorld, 2);

    double  a0 = homogeneousPlane.DotProduct (eyePointWorld);
    double  a1 = homogeneousPlane.DotProduct (edgePointWorld, 1.0);
    double  tol = 1.0e-5 * (1.0 + fabs (a0) + fabs (a1) + fabs (homogeneousPlane.w));

    if (fabs (a1) < tol)
        return true;

    return ((a0 * a1 > 0) ? true : false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   11/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool TestCurveLocation (CurveVectorCR curvesLocal)
    {
    DPoint3d    spacePointLocal;
    Transform   worldToLocal;

    worldToLocal.InverseOf(m_currentTransform);
    worldToLocal.Multiply(&spacePointLocal, &m_snapContext.GetSnapDetail()->GetGeomDetail().GetClosestPoint(), 1);

    CurveLocationDetail location;

    if (!curvesLocal.ClosestPointBounded (spacePointLocal, location))
        return false;

    DPoint3d    locatePointWorld;

    m_currentTransform.Multiply(&locatePointWorld, &location.point, 1);

    // NOTE: Point visible check is problematic for curved surfaces as edge point is far away from surface normal location...
    bool isVisible = IsEdgePointVisible (locatePointWorld, *m_snapContext.GetSnapDetail());

    // NOTE: m_location.curve becomes invalid when curves goes away...we only care about "a" and whether it's NULL, so that's fine...
    if (nullptr == m_location.curve || (isVisible && !m_isVisible))
        m_location = location;
    else if ((m_isVisible && !isVisible) || !m_location.UpdateIfCloser(location))
        return false;

    m_isVisible = isVisible;
    m_snapContext.GetSnapDetail()->GetGeomDetailW().SetCurvePrimitive(m_location.curve, m_currentTransform.IsIdentity() ? nullptr : &m_currentTransform, HitGeomType::Surface);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   11/13
+---------------+---------------+---------------+---------------+---------------+------*/
virtual BentleyStatus _ProcessCurveVector (CurveVectorCR curves, bool isFilled) override
    {
    // Quick exclude of geometry that didn't generate the hit...
    if (m_snapContext.GetSnapDetail()->GetGeomDetail().GetGeomStreamEntryId() != m_context->GetGeomStreamEntryId())
        return SUCCESS;

    TestCurveLocation(curves);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   11/13
+---------------+---------------+---------------+---------------+---------------+------*/
virtual BentleyStatus _ProcessFacets (PolyfaceQueryCR meshData, bool isFilled) override
    {
    // Quick exclude of geometry that didn't generate the hit...
    if (m_snapContext.GetSnapDetail()->GetGeomDetail().GetGeomStreamEntryId() != m_context->GetGeomStreamEntryId())
        return SUCCESS;

    PolyfaceVisitorPtr  visitor = PolyfaceVisitor::Attach(meshData);
    double              tolerance = 1e37; /*fc_hugeVal*/

    visitor->SetNumWrap(1);

    DPoint3d    spacePointLocal;
    Transform   worldToLocal;

    worldToLocal.InverseOf(m_currentTransform);
    worldToLocal.Multiply(&spacePointLocal, &m_snapContext.GetSnapDetail()->GetGeomDetail().GetClosestPoint(), 1);

    for (; visitor->AdvanceToNextFace(); )
        {
        DPoint3d    thisFacePoint;

        if (!visitor->TryFindCloseFacetPoint(spacePointLocal, tolerance, thisFacePoint))
            continue;

        CurveVectorPtr  curves = CurveVector::CreateLinear(visitor->Point());

        TestCurveLocation(*curves);
        tolerance = thisFacePoint.Distance(spacePointLocal); // Refine tolerance...
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   08/15
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _OutputGraphics (ViewContextR context) override
    {
    SnapDetailP snap = m_snapContext.GetSnapDetail();
    IElemTopologyCP elemTopo = snap->GetElemTopology();
    ITransientGeometryHandlerP transientHandler = (nullptr != elemTopo ? elemTopo->_GetTransientGeometryHandler() : nullptr);

    if (nullptr == transientHandler)
        return;

    transientHandler->_DrawTransient(*snap, m_snapContext);
    }

public:

SnapGraphicsProcessor (SnapContextR snapContext) : m_snapContext(snapContext) {m_isVisible = false;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   08/15
+---------------+---------------+---------------+---------------+---------------+------*/
static bool DoSnapUsingClosestCurve (SnapContextR snapContext)
    {
    SnapGraphicsProcessor processor(snapContext);

    ElementGraphicsOutput::Process(processor, snapContext.GetDgnDb());

    if (nullptr == snapContext.GetSnapDetail()->GetGeomDetail().GetCurvePrimitive())
        return false; // No edge found...

    return (SnapStatus::Success == snapContext.DoSnapUsingCurve(snapContext.GetSnapMode()) ? true : false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   11/13
+---------------+---------------+---------------+---------------+---------------+------*/
static bool DoSnapUsingClosestCurve (GeometrySourceCR source, SnapContextR snapContext)
    {
    SnapGraphicsProcessor processor(snapContext);

    ElementGraphicsOutput::Process(processor, source);

    if (nullptr == snapContext.GetSnapDetail()->GetGeomDetail().GetCurvePrimitive())
        return false; // No edge found...

    return (SnapStatus::Success == snapContext.DoSnapUsingCurve(snapContext.GetSnapMode()) ? true : false);
    }

}; // SnapEdgeProcessor

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/06
+---------------+---------------+---------------+---------------+---------------+------*/
SnapStatus      SnapContext::DoDefaultDisplayableSnap ()
    {
    SnapDetailP     snap = GetSnapDetail();
    SnapMode        snapMode = GetSnapMode();
    GeomDetailCR    detail = snap->GetGeomDetail();

    // Don't require a curve primitive if hit geom is point or mode is nearest because current hit point is correct...
    if (SnapMode::Nearest == snapMode || HitGeomType::Point == detail.GetGeomType())
        {
        DPoint3d    hitPoint = snap->GetHitPoint();

        SetSnapInfo (snapMode, GetSnapSprite (snapMode), hitPoint, false, false);

        return SnapStatus::Success;
        }

    if (nullptr == detail.GetCurvePrimitive ())
        {
        // Surface w/o curve is interior hit...only nearest should "track" surface...
        if (HitGeomType::Surface == detail.GetGeomType ())
            {
            DgnElementCPtr   element = snap->GetElement();
            GeometrySourceCP source = (element.IsValid() ? element->ToGeometrySource() : nullptr);

            if (nullptr == source)
                return SnapGraphicsProcessor::DoSnapUsingClosestCurve(*this) ? SnapStatus::Success : SnapStatus::NotSnappable;

            if (SnapMode::Origin != snapMode)
                {
                // NOTE: This is a fairly expensive proposition...but snap to center of range is really useless, so...
                if (SnapGraphicsProcessor::DoSnapUsingClosestCurve (*source, *this))
                    return SnapStatus::Success;
                }

            DPoint3d hitPoint = (nullptr != source->ToGeometrySource3d() ? source->ToGeometrySource3d()->GetPlacement().GetOrigin() : DPoint3d::From(source->ToGeometrySource2d()->GetPlacement().GetOrigin()));

            SetSnapInfo (snapMode, GetSnapSprite (snapMode), hitPoint, false, false);

            return SnapStatus::Success;
            }

        return SnapStatus::NotSnappable;
        }

    return DoSnapUsingCurve (snapMode);
    }

/*---------------------------------------------------------------------------------**//**
* Call this routine if the point to which this snap should be placed is different from the
* point that actually generated the snap.
* @bsimethod                                                    KeithBentley    05/01
+---------------+---------------+---------------+---------------+---------------+------*/
void    SnapContext::SetAdjustedSnapPoint (DPoint3dCR adjustedPt)
    {
    m_snapPath->SetAdjustedPoint (adjustedPt);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/04
+---------------+---------------+---------------+---------------+---------------+------*/
KeypointType    SnapContext::GetSnapKeypointType (SnapMode snapMode)
    {
    KeypointType    keypointType = KEYPOINT_TYPE_Unknown;

    switch (snapMode)
        {
        case SnapMode::Nearest:
            keypointType = KEYPOINT_TYPE_Nearest;
            break;
        case SnapMode::NearestKeypoint:
            keypointType = KEYPOINT_TYPE_Keypoint;
            break;
        case SnapMode::MidPoint:
            keypointType = KEYPOINT_TYPE_Midpoint;
            break;
        case SnapMode::Center:
            keypointType = KEYPOINT_TYPE_Center;
            break;
        case SnapMode::Origin:
            keypointType = KEYPOINT_TYPE_Origin;
            break;
        case SnapMode::Bisector:
            keypointType = KEYPOINT_TYPE_Bisector;
            break;
        case SnapMode::Intersection:
            keypointType = KEYPOINT_TYPE_Intersection;
            break;
        case SnapMode::Tangency:
            keypointType = KEYPOINT_TYPE_Tangent;
            break;
        case SnapMode::TangentPoint:
            keypointType = KEYPOINT_TYPE_Tangentpoint;
            break;
        case SnapMode::Perpendicular:
            keypointType = KEYPOINT_TYPE_Perpendicular;
            break;
        case SnapMode::PerpendicularPoint:
            keypointType = KEYPOINT_TYPE_Perpendicularpt;
            break;
        case SnapMode::Parallel:
            keypointType = KEYPOINT_TYPE_Parallel;
            break;
        case SnapMode::PointOn:
            keypointType = KEYPOINT_TYPE_PointOn;
            break;
        }

    return keypointType;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/04
+---------------+---------------+---------------+---------------+---------------+------*/
SnapStatus      SnapContext::DoTextSnap ()
    {
    SnapDetailP     snap = GetSnapDetail ();
    SnapMode        snapMode = GetSnapMode ();
    GeomDetailCR    detail = snap->GetGeomDetail ();

    switch (snapMode)
        {
        case SnapMode::Origin:
        case SnapMode::NearestKeypoint:
            {
            DgnElementCPtr   element = snap->GetElement();
            GeometrySourceCP source = (element.IsValid() ? element->ToGeometrySource() : nullptr);

            if (nullptr == source)
                return SnapStatus::NotSnappable;

            DPoint3d hitPoint = (nullptr != source->ToGeometrySource3d() ? source->ToGeometrySource3d()->GetPlacement().GetOrigin() : DPoint3d::From(source->ToGeometrySource2d()->GetPlacement().GetOrigin()));
            
            SetSnapInfo (snapMode, GetSnapSprite (snapMode), hitPoint, false, false);

            return SnapStatus::Success;
            }

        case SnapMode::MidPoint:
        case SnapMode::Bisector:
        case SnapMode::Center:
            {
            if (NULL == detail.GetCurvePrimitive ())
                return DoDefaultDisplayableSnap (); // NOTE: Boundary shape unavailable for origin only...always have for edge/interior hit...

            DPoint3d    centroid;

            if (!getCentroid (centroid, *detail.GetCurvePrimitive ()))
                return SnapStatus::NotSnappable;

            SetSnapInfo (snapMode, GetSnapSprite (snapMode), centroid, false, false);

            return SnapStatus::Success;
            }

        case SnapMode::Nearest:
            {
            if (nullptr == detail.GetCurvePrimitive ())
                return DoDefaultDisplayableSnap (); // NOTE: Boundary shape unavailable for origin only...always have for edge/interior hit...

            DSegment3d  segment;

            if (!detail.GetSegment (segment))
                return SnapStatus::NotSnappable;

            double      keyparam = detail.GetSegmentParam ();
            DPoint3d    hitPoint;

            GetSegmentKeypoint (hitPoint, keyparam, GetSnapDivisor (), segment);

            SetSnapInfo (snapMode, GetSnapSprite (snapMode), hitPoint, false, false);

            return SnapStatus::Success;
            }

        default:
            {
            // Should never be called with "exotic" snap modes...
            return SnapStatus::NotSnappable;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/05
+---------------+---------------+---------------+---------------+---------------+------*/
void            SnapDetail::SetCustomKeypoint (int nBytes, Byte* dataP)
    {
    if (nBytes && NULL != (m_customKeypointData = (Byte *) bentleyAllocator_malloc(nBytes)))
        {
        m_customKeypointSize = nBytes;
        memcpy (m_customKeypointData, dataP, nBytes);

        // NOTE: Clear curve topo id so we don't try to create TopologyCurveAssociation instead of handler's custom assoc!
        ICurvePrimitiveCP hitPrimitive = m_geomDetail.GetCurvePrimitive ();

        if (hitPrimitive)
            hitPrimitive->SetId (NULL); 
        }
    }

