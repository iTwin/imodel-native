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
    switch (primitive.GetCurvePrimitiveType ())
        {
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString:
            return (primitive.GetLineStringCP ()->size () > 3 && primitive.GetLineStringCP ()->front ().IsEqual (primitive.GetLineStringCP ()->back ()));

        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc:
            return primitive.GetArcCP ()->IsFullEllipse ();

        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_BsplineCurve:
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_InterpolationCurve:
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_AkimaCurve:
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Spiral:
            return (primitive.GetProxyBsplineCurveCP ()->IsClosed ());

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
    SnapPathCP         snap = GetSnapPath ();
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

            HitLocalToWorld (hitPoint);
            SetSnapInfo (snapMode, GetSnapSprite (snapMode), hitPoint, false, false);

            return SnapStatus::Success;
            }

        case SnapMode::MidPoint:
            {
            switch (detail.GetEffectiveHitGeomType ())
                {
                case HitGeomType::Point:
                    {
                    DPoint3d    hitPoint = detail.GetClosestPointLocal ();

                    HitLocalToWorld (hitPoint);
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
                    HitLocalToWorld (hitPoint);
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
                    HitLocalToWorld (hitPoint);
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

            HitLocalToWorld (location.point);
            SetSnapInfo (snapMode, GetSnapSprite (snapMode), location.point, false, false);

            return SnapStatus::Success;
            }

        case SnapMode::Center:
            {
            DPoint3d    centroid;

            if (!getCentroid (centroid, *curve))
                return SnapStatus::NotSnappable;

            HitLocalToWorld (centroid);
            SetSnapInfo (snapMode, GetSnapSprite (snapMode), centroid, true /* force hot */, false);

            return SnapStatus::Success;
            }

        case SnapMode::Nearest:
            {
            DPoint3d    hitPoint = detail.GetClosestPointLocal (); // Current snap info is for nearest...just need to set snap using current point.

            HitLocalToWorld (hitPoint);
            SetSnapInfo (snapMode, GetSnapSprite (snapMode), hitPoint, false, false);

            return SnapStatus::Success;
            }

        case SnapMode::NearestKeypoint:
            {
            switch (detail.GetEffectiveHitGeomType ())
                {
                case HitGeomType::Point:
                    {
                    DPoint3d    hitPoint = detail.GetClosestPointLocal ();

                    HitLocalToWorld (hitPoint);
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

                    GetSegmentKeypoint (hitPoint, keyparam, GetSnapDivisor (), segment);
                    HitLocalToWorld (hitPoint);
                    SetSnapInfo (snapMode, GetSnapSprite (snapMode), hitPoint, false, false);

                    return SnapStatus::Success;
                    }

                case HitGeomType::Arc:
                case HitGeomType::Curve:
                    {
                    double      keyparam = detail.GetCloseParam ();
                    DPoint3d    hitPoint;

                    if (!GetParameterKeypoint (hitPoint, keyparam, GetSnapDivisor ()))
                        return SnapStatus::NotSnappable;

                    HitLocalToWorld (hitPoint);
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
    SnapPathCP         snap = GetSnapPath ();
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
* @bsimethod                                                    Brien.Bastings  11/13
+---------------+---------------+---------------+---------------+---------------+------*/
void SnapContext::HitLocalToWorld (DPoint3dR point)
    {
    GetSnapPath ()->GetGeomDetail ().GetLocalToWorld ().MultiplyAndRenormalize (&point, &point, 1);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/13
+---------------+---------------+---------------+---------------+---------------+------*/
void SnapContext::ElmLocalToWorld (DPoint3dR point) // WIP_V10_NO_SHARED_CELLS - Remove with shared cells.
    {
    DMatrix4d   elmLocalToWorld;

    // NOTE: GeomDetail::LocalToWorld may include transforms pushed by _Draw method...need current element's local to world from context...
    if (SUCCESS == GetCurrLocalToWorldTrans (elmLocalToWorld))
        elmLocalToWorld.MultiplyAndRenormalize (&point, &point, 1);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/04
+---------------+---------------+---------------+---------------+---------------+------*/
void            SnapContext::SetSnapInfo
(
SnapMode        snapMode,
ISpriteP        sprite,
DPoint3dCR      snapPoint, // In active coords...
bool            forceHot,
bool            isAdjusted,
int             nBytes,
Byte*           customKeypointData
)
    {
    SnapPathP   snap = GetSnapPath ();

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

    pVec1.getProjectedXYZ (&v1);
    pVec2.getProjectedXYZ (&v2);

    double dx = v1.x - v2.x;
    double dy = v1.y - v2.y;

    return   dx * dx + dy * dy;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SamWilson       06/03
+---------------+---------------+---------------+---------------+---------------+------*/
static double   getDistanceFromSnap (SnapPathCR hit, ViewContextP context)
    {
    DPoint3d    pts[2];
    DPoint4d    scrPts[2];

    hit.GetGeomDetail ().GetClosestPoint (pts[0]);
    pts[1] = hit.GetSnapPoint();

    // NOTE: Use viewport to get active-to-view...
    context->GetViewport()->GetWorldToViewMap()->M0.multiply (scrPts, pts, NULL, 2);

    return sqrt (distSquaredXY (scrPts[0], scrPts[1]));
    }

/*---------------------------------------------------------------------------------**//**
* save the snap point into the snap path currently being generated.
* @bsimethod                                                    KeithBentley    05/01
+---------------+---------------+---------------+---------------+---------------+------*/
void            SnapContext::SetSnapPoint (DPoint3dCR snapPt, bool forceHot)
    {
    DPoint3d    rootPt;
    DPoint4d    viewPt;

    // NOTE: Use viewport to get active-to-view...
    m_viewport->GetWorldToViewMap()->M0.multiply (&viewPt, &rootPt, NULL, 1);

    viewPt.normalizeWeightInPlace ();
    Point2d screenPt;
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
virtual bool _ProcessAsFacets (bool isPolyface) const {return isPolyface;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   11/13
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _AnnounceContext (ViewContextR context) override
    {
    m_context = &context;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   11/13
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _AnnounceTransform (TransformCP trans) override
    {
    if (trans)
        m_currentTransform = *trans;
    else
        m_currentTransform.initIdentity ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   11/13
+---------------+---------------+---------------+---------------+---------------+------*/
void GetSpacePointAndCurveTransform (DPoint3dR spacePoint, TransformR curveToHitLocalTrans)
    {
    Transform   contextLocalToHitLocal;

    // NOTE: GeomDetail::LocalToWorld includes pushed transforms...account for difference between hit and context local coords...
    if (SUCCESS == m_snapContext.GetSnapPath ()->GetContextLocalToHitLocal (contextLocalToHitLocal, m_snapContext) && !contextLocalToHitLocal.IsIdentity ())
        curveToHitLocalTrans.InitProduct (contextLocalToHitLocal, m_currentTransform); // current is curveToContextLocalTrans, it's only transforms pushed by _Draw...
    else
        curveToHitLocalTrans = m_currentTransform; // curveToHitLocalTrans == curveToContextLocalTrans...

    Transform   hitLocalToCurveTrans;

    spacePoint = m_snapContext.GetSnapPath ()->GetGeomDetail ().GetClosestPointLocal ();
    hitLocalToCurveTrans.InverseOf (curveToHitLocalTrans);
    hitLocalToCurveTrans.Multiply (&spacePoint, &spacePoint, 1);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool IsEdgePointVisible (DPoint3dCR edgePoint, SnapPathCR snap)
    {
    // See HitList::Compare doZCompareOfSurfaceAndEdge...
    DgnViewportR vp = snap.GetViewport ();
    DPoint4d     homogeneousPlane;
    
    if (!homogeneousPlane.PlaneFromOriginAndNormal (snap.GetGeomDetail ().GetClosestPointLocal (), snap.GetGeomDetail ().GetSurfaceNormal ()))
        return true;

    DMap4d      worldToViewMap = *vp.GetWorldToViewMap ();
    DMap4d      localToWorldMap, localToViewMap;
    DMatrix4d   worldToLocal;

    worldToLocal.QrInverseOf (snap.GetGeomDetail ().GetLocalToWorld ());
    localToWorldMap.InitFrom (snap.GetGeomDetail ().GetLocalToWorld (), worldToLocal);
    localToViewMap.InitProduct (worldToViewMap, localToWorldMap);

    DPoint4d    eyePointLocal;

    localToViewMap.M1.GetColumn (eyePointLocal, 2);

    double  a0 = homogeneousPlane.DotProduct (eyePointLocal);
    double  a1 = homogeneousPlane.DotProduct (edgePoint, 1.0);
    double  tol = 1.0e-5 * (1.0 + fabs (a0) + fabs (a1) + fabs (homogeneousPlane.w));

    if (fabs (a1) < tol)
        return true;

    return ((a0 * a1 > 0) ? true : false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   11/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool TestCurveLocation (CurveVectorCR curves, DPoint3dCR spacePoint, TransformCR curveToHitLocal)
    {
    CurveLocationDetail location;

    if (!curves.ClosestPointBounded (spacePoint, location))
        return false;

    bool        isVisible;
    DPoint3d    edgePoint;

    // NOTE: Point visible check is problematic for curved surfaces as edge point is far away from surface normal location...
    curveToHitLocal.Multiply (&edgePoint, &location.point, 1);
    isVisible = IsEdgePointVisible (edgePoint, *m_snapContext.GetSnapPath ());

    // NOTE: m_location.curve becomes invalid when curves goes away...we only care about "a" and whether it's NULL, so that's fine...
    if (NULL == m_location.curve || (isVisible && !m_isVisible))
        m_location = location;
    else if ((m_isVisible && !isVisible) || !m_location.UpdateIfCloser (location))
        return false;

    m_isVisible = isVisible;

    if (curveToHitLocal.IsIdentity ())
        {
        m_snapContext.GetSnapPath ()->GetGeomDetailW ().SetCurvePrimitive (m_location.curve, HitGeomType::Surface);

        return true;
        }

    ICurvePrimitivePtr tmpCurve = m_location.curve->Clone ();

    tmpCurve->TransformInPlace (curveToHitLocal);
    m_snapContext.GetSnapPath ()->GetGeomDetailW ().SetCurvePrimitive (tmpCurve.get (), HitGeomType::Surface);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   11/13
+---------------+---------------+---------------+---------------+---------------+------*/
virtual BentleyStatus _ProcessCurveVector (CurveVectorCR curves, bool isFilled) override
    {
    DPoint3d    spacePoint;
    Transform   curveToHitLocal;

    GetSpacePointAndCurveTransform (spacePoint, curveToHitLocal);
    TestCurveLocation (curves, spacePoint, curveToHitLocal);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   11/13
+---------------+---------------+---------------+---------------+---------------+------*/
virtual BentleyStatus _ProcessFacets (PolyfaceQueryCR meshData, bool isFilled)
    {
    DPoint3d    spacePoint;
    Transform   curveToHitLocal;

    GetSpacePointAndCurveTransform (spacePoint, curveToHitLocal);

    PolyfaceVisitorPtr  visitor = PolyfaceVisitor::Attach (meshData);
    double              tolerance = 1e37; /*fc_hugeVal*/

    visitor->SetNumWrap (1);

    for (; visitor->AdvanceToNextFace (); )
        {
        DPoint3d    thisFacePoint;

        if (!visitor->TryFindCloseFacetPoint (spacePoint, tolerance, thisFacePoint))
            continue;

        CurveVectorPtr  curves = CurveVector::CreateLinear (visitor->Point ());

        TestCurveLocation (*curves, spacePoint, curveToHitLocal);
        tolerance = thisFacePoint.Distance (spacePoint); // Refine tolerance...
        }

    return SUCCESS;
    }

public:

SnapGraphicsProcessor (SnapContextR snapContext) : m_snapContext (snapContext) {m_isVisible = false;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   11/13
+---------------+---------------+---------------+---------------+---------------+------*/
static bool DoSnapUsingClosestCurve (GeometricElementCR element, SnapContextR snapContext)
    {
    SnapGraphicsProcessor processor (snapContext);

    ElementGraphicsOutput::Process (processor, element);

    if (NULL == snapContext.GetSnapPath ()->GetGeomDetail ().GetCurvePrimitive ())
        return false; // No edge found...

    return (SnapStatus::Success == snapContext.DoSnapUsingCurve (snapContext.GetSnapMode ()) ? true : false);
    }

}; // SnapEdgeProcessor

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/06
+---------------+---------------+---------------+---------------+---------------+------*/
SnapStatus      SnapContext::DoDefaultDisplayableSnap ()
    {
    SnapPathP       snap = GetSnapPath ();
    SnapMode        snapMode = GetSnapMode ();
    GeomDetailCR    detail = snap->GetGeomDetail ();

    // Don't require a gpa if hit geom is point or mode is nearest because current hit point is correct...
    if (SnapMode::Nearest == snapMode || HitGeomType::Point == detail.GetGeomType ())
        {
        DPoint3d    hitPoint;

        snap->GetHitPoint (hitPoint);
        SetSnapInfo (snapMode, GetSnapSprite (snapMode), hitPoint, false, false);

        return SnapStatus::Success;
        }

    if (NULL == detail.GetCurvePrimitive ())
        {
        // Surface w/o curve is interior hit...only nearest should "track" surface...
        if (HitGeomType::Surface == detail.GetGeomType ())
            {
            GeometricElementCPtr element = snap->GetElement();

            if (!element.IsValid())
                return SnapStatus::NotSnappable;

            if (SnapMode::Origin != snapMode)
                {
                // NOTE: This is a fairly expensive proposition...but snap to center of range is really useless, so...
                if (SnapGraphicsProcessor::DoSnapUsingClosestCurve (*element, *this))
                    return SnapStatus::Success;
                }

            DPoint3d hitPoint = (element->Is3d() ? element->ToElement3d()->GetPlacement().GetOrigin() : DPoint3d::From(element->ToElement2d()->GetPlacement().GetOrigin()));

            ElmLocalToWorld (hitPoint);
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
    SnapPathP       snap = GetSnapPath ();
    SnapMode        snapMode = GetSnapMode ();
    GeomDetailCR    detail = snap->GetGeomDetail ();

    switch (snapMode)
        {
        case SnapMode::Origin:
        case SnapMode::NearestKeypoint:
            {
            GeometricElementCPtr element = snap->GetElement();

            if (!element.IsValid())
                return SnapStatus::NotSnappable;

            DPoint3d hitPoint = (element->Is3d() ? element->ToElement3d()->GetPlacement().GetOrigin() : DPoint3d::From(element->ToElement2d()->GetPlacement().GetOrigin()));
            
            ElmLocalToWorld (hitPoint);
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

            HitLocalToWorld (centroid);
            SetSnapInfo (snapMode, GetSnapSprite (snapMode), centroid, false, false);

            return SnapStatus::Success;
            }

        case SnapMode::Nearest:
            {
            if (NULL == detail.GetCurvePrimitive ())
                return DoDefaultDisplayableSnap (); // NOTE: Boundary shape unavailable for origin only...always have for edge/interior hit...

            DSegment3d  segment;

            if (!detail.GetSegment (segment))
                return SnapStatus::NotSnappable;

            double      keyparam = detail.GetSegmentParam ();
            DPoint3d    hitPoint;

            GetSegmentKeypoint (hitPoint, keyparam, GetSnapDivisor (), segment);
            HitLocalToWorld (hitPoint);

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
void            SnapPath::SetCustomKeypoint (int nBytes, Byte* dataP)
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

