/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/SnapContext.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#if defined (BENTLEYCONFIG_PARASOLID) 
#include <DgnPlatform/DgnBRep/PSolidUtil.h>
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/12
+---------------+---------------+---------------+---------------+---------------+------*/
static bool isPhysicallyClosed(ICurvePrimitiveCR primitive)
    {
    switch (primitive.GetCurvePrimitiveType())
        {
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString:
            return (primitive.GetLineStringCP()->size() > 3 && primitive.GetLineStringCP()->front().IsEqual(primitive.GetLineStringCP()->back()));

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
static bool getAreaCentroid(DPoint3dR centroid, CurveVectorCR curve)
    {
    Transform   localToWorld, worldToLocal;
    DRange3d    localRange;

    if (!curve.IsPlanar(localToWorld, worldToLocal, localRange))
        return false;

    DVec3d      normal;
    double      area;

    return curve.CentroidNormalArea(centroid, normal, area);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/12
+---------------+---------------+---------------+---------------+---------------+------*/
static bool getCentroid(DPoint3dR centroid, ICurvePrimitiveCR primitive)
    {
    if (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc == primitive.GetCurvePrimitiveType())
        {
        // Center snap always uses arc center...
        centroid = primitive.GetArcCP ()->center;

        return true;
        }
    else if (isPhysicallyClosed(primitive))
        {
        CurveVectorPtr  curve = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Outer);
        
        curve->push_back(primitive.Clone());

        // For physically closed/planar curve use area centroid instead of wire centroid...
        if (getAreaCentroid(centroid, *curve))
            return true;
        }

    double  length;

    return primitive.WireCentroid(length, centroid);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/12
+---------------+---------------+---------------+---------------+---------------+------*/
SnapStatus SnapContext::DoSnapUsingCurve(SnapMode snapMode)
    {
    SnapDetailCP       snap = GetSnapDetail();
    GeomDetailCR       detail = snap->GetGeomDetail();
    ICurvePrimitiveCP  curve;

    if (NULL == (curve = detail.GetCurvePrimitive()))
        return SnapStatus::NotSnappable;

    switch (snapMode)
        {
        case SnapMode::Origin:
            {
            DPoint3d    hitPoint;

            if (!curve->GetStartPoint(hitPoint))
                return SnapStatus::NotSnappable;

            SetSnapInfo(snapMode, GetSnapSprite(snapMode), hitPoint, false, false);

            return SnapStatus::Success;
            }

        case SnapMode::MidPoint:
            {
            switch (detail.GetEffectiveHitGeomType())
                {
                case HitGeomType::Point:
                    {
                    DPoint3d    hitPoint = detail.GetClosestPoint();
                    DEllipse3d  arc;

                    // Check for surface interior hit that was closest to arc center...
                    if (detail.GetArc(arc))
                        hitPoint = arc.center;

                    SetSnapInfo(snapMode, GetSnapSprite(snapMode), hitPoint, false, false);

                    return SnapStatus::Success;
                    }

                case HitGeomType::Segment:
                    {
                    DSegment3d  segment;

                    if (!detail.GetSegment(segment))
                        return SnapStatus::NotSnappable;

                    DPoint3d    hitPoint;

                    hitPoint.Interpolate(segment.point[0], 0.5, segment.point[1]);
                    SetSnapInfo(snapMode, GetSnapSprite(snapMode), hitPoint, false, false);

                    return SnapStatus::Success;
                    }

                case HitGeomType::Arc:
                    {
                    DEllipse3dCP  ellipse = curve->GetArcCP ();

                    if (!ellipse)
                        return SnapStatus::NotSnappable;

                    DPoint3d    hitPoint;

                    ellipse->FractionParameterToPoint(hitPoint, 0.5);
                    SetSnapInfo(snapMode, GetSnapSprite(snapMode), hitPoint, false, false);

                    return SnapStatus::Success;
                    }
                }

            // Fall through to bisector if not handled...
            }

        case SnapMode::Bisector:
            {
            double  length;

            if (!curve->Length(length))
                return SnapStatus::NotSnappable;

            CurveLocationDetail location;

            if (!curve->PointAtSignedDistanceFromFraction(0.0, length * 0.5, false, location))
                return SnapStatus::NotSnappable;

            SetSnapInfo(snapMode, GetSnapSprite(snapMode), location.point, false, false);

            return SnapStatus::Success;
            }

        case SnapMode::Center:
            {
            DPoint3d    centroid;

            if (!getCentroid(centroid, *curve))
                return SnapStatus::NotSnappable;

            SetSnapInfo(snapMode, GetSnapSprite(snapMode), centroid, true /* force hot */, false);

            return SnapStatus::Success;
            }

        case SnapMode::Nearest:
            {
            DPoint3d    hitPoint = detail.GetClosestPoint(); // Current snap info is for nearest...just need to set snap using current point.

            SetSnapInfo(snapMode, GetSnapSprite(snapMode), hitPoint, false, false);

            return SnapStatus::Success;
            }

        case SnapMode::NearestKeypoint:
            {
            switch (detail.GetEffectiveHitGeomType())
                {
                case HitGeomType::Point:
                    {
                    DPoint3d    hitPoint = detail.GetClosestPoint();
                    DEllipse3d  arc;

                    // Check for surface interior hit that was closest to arc center...
                    if (detail.GetArc(arc))
                        hitPoint = arc.center;
                    
                    SetSnapInfo(snapMode, GetSnapSprite(snapMode), hitPoint, false, false);

                    return SnapStatus::Success;
                    }

                case HitGeomType::Segment:
                    {
                    DSegment3d  segment;

                    if (!detail.GetSegment(segment))
                        return SnapStatus::NotSnappable;

                    double      keyparam = detail.GetSegmentParam();
                    DPoint3d    hitPoint;

                    GetSegmentKeypoint(hitPoint, keyparam, GetSnapDivisor(), segment);
                    SetSnapInfo(snapMode, GetSnapSprite(snapMode), hitPoint, false, false);

                    return SnapStatus::Success;
                    }

                case HitGeomType::Arc:
                case HitGeomType::Curve:
                    {
                    double      keyparam = detail.GetCloseParam();
                    DPoint3d    hitPoint;

                    if (!GetParameterKeypoint(hitPoint, keyparam, GetSnapDivisor()))
                        return SnapStatus::NotSnappable;

                    SetSnapInfo(snapMode, GetSnapSprite(snapMode), hitPoint, false, false);

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
bool SnapContext::GetParameterKeypoint(DPoint3dR hitPoint, double& keyParam, int divisor)
    {
    SnapDetailCP       snap = GetSnapDetail();
    GeomDetailCR       detail = snap->GetGeomDetail();
    ICurvePrimitiveCP  curve;

    if (NULL == (curve = detail.GetCurvePrimitive()))
        return false;

    double  length, subLength;

    if (!curve->Length(length) || !curve->SignedDistanceBetweenFractions(0.0, keyParam, subLength))
        return false;

    // NOTE: For a closed curve we want quadrant key points.
    if (isPhysicallyClosed(*curve))
        divisor *= 4;

    int     keySeg = int ((subLength / length) * (double) divisor + 0.5);
    double  keyDist = (keySeg / (double) divisor) * length;

    CurveLocationDetail location;

    if (!curve->PointAtSignedDistanceFromFraction(0.0, keyDist, false, location))
        return false;

    hitPoint = location.point;
    keyParam = location.fraction;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/04
+---------------+---------------+---------------+---------------+---------------+------*/
void SnapContext::GetSegmentKeypoint(DPoint3dR hitPoint, double& keyParam, int divisor, DSegment3dCR segment)
    {
    int numerator = int ((keyParam * (double) divisor) + 0.5);
    
    keyParam = (numerator / (double) divisor);
    hitPoint.Interpolate(segment.point[0], keyParam, segment.point[1]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/04
+---------------+---------------+---------------+---------------+---------------+------*/
void SnapContext::SetSnapInfo(SnapMode snapMode, ISpriteP sprite, DPoint3dCR snapPoint, bool forceHot, bool isAdjusted, int nBytes, Byte* customKeypointData)
    {
    SnapDetailP snap = GetSnapDetail();

    snap->SetSnapMode(snapMode);
    snap->SetSprite(sprite);

    snap->SetSnapPoint(snapPoint, forceHot, m_snapAperture);
    snap->SetAllowAssociations(!isAdjusted);

    if (nBytes && customKeypointData)
        snap->SetCustomKeypoint(nBytes, customKeypointData);
    }

BEGIN_BENTLEY_DGN_NAMESPACE
/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct SnapGraphicsProcessor : IGeometryProcessor
{
private:

    SnapContextR        m_snapContext;
    CurveLocationDetail m_location;
    bool                m_inProcessFace = false;

protected:

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/16
+---------------+---------------+---------------+---------------+---------------+------*/
IGeometryProcessor::UnhandledPreference _GetUnhandledPreference(ISolidPrimitiveCR, SimplifyGraphic&) const override {return IGeometryProcessor::UnhandledPreference::Curve;}
IGeometryProcessor::UnhandledPreference _GetUnhandledPreference(MSBsplineSurfaceCR, SimplifyGraphic&) const override {return IGeometryProcessor::UnhandledPreference::Curve;}
IGeometryProcessor::UnhandledPreference _GetUnhandledPreference(PolyfaceQueryCR, SimplifyGraphic&) const override {return IGeometryProcessor::UnhandledPreference::Curve;}
IGeometryProcessor::UnhandledPreference _GetUnhandledPreference(IBRepEntityCR, SimplifyGraphic&) const override {return IGeometryProcessor::UnhandledPreference::Curve;}
IGeometryProcessor::UnhandledPreference _GetUnhandledPreference(TextStringCR, SimplifyGraphic&) const override {return IGeometryProcessor::UnhandledPreference::Box;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/12
+---------------+---------------+---------------+---------------+---------------+------*/
void CollectLateralRulePoints(CurveVectorCR curves, bvector<DPoint3d>& pts, int divisor, int closedDivisor)
    {
    switch (curves.HasSingleCurvePrimitive())
        {
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line:
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString:
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_PointString:
            break;

        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc:
            {
            DEllipse3dCP  ellipse = curves.front()->GetArcCP();
            bool          fullArc = ellipse->IsFullEllipse();

            if (fullArc)
                divisor = closedDivisor;

            for (int iRule = 0; iRule < divisor; ++iRule)
                {
                double    fraction = (1.0 / divisor) * iRule;
                DPoint3d  point;

                if (!fullArc && DoubleOps::AlmostEqual(fraction, 0.0))
                    continue;

                ellipse->FractionParameterToPoint(point, fraction);
                pts.push_back(point);
                }
            break;
            }

        default:
            {
            MSBsplineCurveCP  bcurve = curves.front()->GetProxyBsplineCurveCP();

            if (nullptr == bcurve)
                break;

            if (bcurve->IsClosed())
                divisor = closedDivisor;

            for (int iRule = 0; iRule < divisor; ++iRule)
                {
                double    fraction = (1.0 / divisor) * iRule;
                DPoint3d  point;

                if (!bcurve->IsClosed() && DoubleOps::AlmostEqual(fraction, 0.0))
                    continue;

                bcurve->FractionToPoint(point, fraction);
                pts.push_back(point);
                }
            break;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool ComputeRuleArc(DEllipse3dR ellipse, DPoint3dCR startPt, DPoint3dCR originPt, double sweepAngle, TransformCR transform, RotMatrixCR axes, RotMatrixCR invAxes, double ruleTolerance)
    {
    DPoint3d    endPt, centerPt, tmpPt;

    transform.Multiply(&endPt, &startPt, 1);
    centerPt = originPt;

    tmpPt = startPt;
    axes.Multiply(tmpPt);
    axes.Multiply(centerPt);
    centerPt.z = tmpPt.z;

    DVec3d      xVec, yVec, zVec;
    RotMatrix   rMatrix;

    zVec.Init(0.0, 0.0, 1.0);
    xVec.NormalizedDifference(tmpPt, centerPt);
    yVec.CrossProduct(zVec, xVec);
    rMatrix.InitFromColumnVectors(xVec, yVec, zVec);
    rMatrix.InitProduct(invAxes, rMatrix);
    axes.MultiplyTranspose(centerPt);

    double  radius = centerPt.Distance(startPt);

    if (radius < ruleTolerance)
        return false;

    ellipse.InitFromScaledRotMatrix(centerPt, rMatrix, radius, radius, 0.0, sweepAngle);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   11/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool TestArcCenter(ICurvePrimitiveCR primitive, SimplifyGraphic& graphic)
    {
    if (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc != primitive.GetCurvePrimitiveType())
        return false;

    DEllipse3dCP arc = primitive.GetArcCP();

    if (fabs(arc->sweep) < Angle::PiOver2())
        return false;

    Transform   localToWorld = graphic.GetLocalToWorldTransform();
    Transform   worldToLocal;
    DPoint3d    spacePointLocal;

    worldToLocal.InverseOf(localToWorld);
    worldToLocal.Multiply(&spacePointLocal, &m_snapContext.GetSnapDetail()->GetGeomDetail().GetClosestPoint(), 1);

    CurveLocationDetail location(&primitive, 0.0, arc->center, 0, 1, 0.0, spacePointLocal.Distance(arc->center));

    // NOTE: m_location.curve becomes invalid when curvesLocal goes away...we only care about "a" and whether it's nullptr, so that's fine...
    if (nullptr == m_location.curve)
        m_location = location;
    else if (!m_location.UpdateIfCloser(location))
        return false;

    // NOTE: Set curve primitive to arc with geometry type as point to denote arc center...
    m_snapContext.GetSnapDetail()->GetGeomDetailW().SetCurvePrimitive(m_location.curve, &localToWorld, HitGeomType::Point);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   11/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool TestCurveLocation(CurveVectorCR curvesLocal, SimplifyGraphic& graphic)
    {
    Transform   localToWorld = graphic.GetLocalToWorldTransform();
    Transform   worldToLocal;
    DPoint3d    spacePointLocal;

    worldToLocal.InverseOf(localToWorld);
    worldToLocal.Multiply(&spacePointLocal, &m_snapContext.GetSnapDetail()->GetGeomDetail().GetClosestPoint(), 1);

    CurveLocationDetail location;

    if (!curvesLocal.ClosestPointBounded(spacePointLocal, location))
        return false;

    // Process components for arc centers...
    graphic.ProcessAsCurvePrimitives(curvesLocal, false);

    // NOTE: m_location.curve becomes invalid when curvesLocal goes away...we only care about "a" and whether it's nullptr, so that's fine...
    if (nullptr == m_location.curve)
        m_location = location;
    else if (!m_location.UpdateIfCloser(location))
        return false;

    m_snapContext.GetSnapDetail()->GetGeomDetailW().SetCurvePrimitive(m_location.curve, &localToWorld, HitGeomType::Surface);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   05/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool _ProcessCurvePrimitive(ICurvePrimitiveCR primitive, bool closed, bool filled, SimplifyGraphic& graphic) override
    {
    TestArcCenter(primitive, graphic);

    return (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_CurveVector != primitive.GetCurvePrimitiveType());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   11/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool _ProcessCurveVector(CurveVectorCR curves, bool isFilled, SimplifyGraphic& graphic) override
    {
    TestCurveLocation(curves, graphic);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   11/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool _ProcessSolidPrimitive(ISolidPrimitiveCR primitive, SimplifyGraphic& graphic) override
    {
    int divisorU = m_snapContext.GetSnapDivisor();
    int divisorV = divisorU;

    switch (primitive.GetSolidPrimitiveType())
        {
        case SolidPrimitiveType_DgnTorusPipe:
            {
            DgnTorusPipeDetail  detail;

            if (!primitive.TryGetDgnTorusPipeDetail(detail))
                return true;

            divisorU = 4; // Don't think other arcs are very useful snap locations...avoid clutter and just output every 90 degrees...

            for (int uRule = 0; uRule < divisorU; ++uRule)
                {
                double          uFraction = (1.0 / divisorU) * uRule;
                CurveVectorPtr  curve = CurveVector::Create(CurveVector::BoundaryType::BOUNDARY_TYPE_Open, ICurvePrimitive::CreateArc(detail.UFractionToVSectionDEllipse3d(uFraction)));

                TestCurveLocation(*curve, graphic);
                }

            bool  fullTorus = Angle::IsFullCircle(detail.m_sweepAngle);

            if (fullTorus)
                divisorV *= 2;

            for (int vRule = 0; vRule <= divisorV; ++vRule)
                {
                double vFraction = (1.0 / divisorV) * vRule;

                if (fullTorus && DoubleOps::AlmostEqual(vFraction, 1.0))
                    continue; // Don't duplicate first arc for a full torus...

                CurveVectorPtr  curve = CurveVector::Create(CurveVector::BoundaryType::BOUNDARY_TYPE_Open, ICurvePrimitive::CreateArc(detail.VFractionToUSectionDEllipse3d(vFraction)));

                TestCurveLocation(*curve, graphic);
                }

            return true;
            }

        case SolidPrimitiveType_DgnCone:
            {
            DgnConeDetail  detail;

            if (!primitive.TryGetDgnConeDetail(detail))
                return true;

            DEllipse3d ellipse1;

            if (detail.FractionToSection(0.0, ellipse1))
                {
                CurveVectorPtr  curve = CurveVector::Create(CurveVector::BoundaryType::BOUNDARY_TYPE_Open, ICurvePrimitive::CreateArc(ellipse1));

                TestCurveLocation(*curve, graphic);
                }

            DEllipse3d ellipse2;

            if (detail.FractionToSection(1.0, ellipse2))
                {
                CurveVectorPtr  curve = CurveVector::Create(CurveVector::BoundaryType::BOUNDARY_TYPE_Open, ICurvePrimitive::CreateArc(ellipse2));

                TestCurveLocation(*curve, graphic);
                }

            divisorV *= 2;

            for (int vRule = 0; vRule < divisorV; ++vRule)
                {
                double      vFraction = (1.0 / divisorV) * vRule;
                DSegment3d  segment;

                if (!detail.FractionToRule(vFraction, segment))
                    continue;

                CurveVectorPtr  curve = CurveVector::Create(CurveVector::BoundaryType::BOUNDARY_TYPE_Open, ICurvePrimitive::CreateLine(segment));

                TestCurveLocation(*curve, graphic);
                }
            
            DSegment3d  silhouette[2];

            // NOTE: MicroStation's type 23 cone element always allowed it's silhouettes to be located/snapped to...continue doing so for historical reasons but with one caveat.
            //       The center of the cursor *must* be over the cone face to actually identify it and not just within locate tolerance. This behavior difference is because we
            //       no longer locate elements their silhouettes in PickContext (would need to do this using the mesh tiles or something in order to support all geometry types)...
            if (detail.GetSilhouettes(silhouette[0], silhouette[1], graphic.GetViewToLocal()))
                {
                for (int iSilhouette = 0; iSilhouette < 2; ++iSilhouette)
                    {
                    if (0.0 == silhouette[iSilhouette].Length())
                        continue;

                    CurveVectorPtr  curve = CurveVector::Create(CurveVector::BoundaryType::BOUNDARY_TYPE_Open, ICurvePrimitive::CreateLine(silhouette[iSilhouette]));

                    TestCurveLocation(*curve, graphic);
                    }
                }

            return true;
            }

        case SolidPrimitiveType_DgnSphere:
            {
            DgnSphereDetail  detail;

            if (!primitive.TryGetDgnSphereDetail(detail))
                return true;

            divisorU *= 2;

            for (int uRule = 0; uRule < divisorU; ++uRule)
                {
                double          uFraction = (1.0 / divisorU) * uRule;
                CurveVectorPtr  curve = CurveVector::Create(CurveVector::BoundaryType::BOUNDARY_TYPE_Open, ICurvePrimitive::CreateArc(detail.UFractionToVSectionDEllipse3d(uFraction)));

                TestCurveLocation(*curve, graphic);
                }

            double latitude0, latitude1, z0, z1;
            bool   fullSphere = !detail.GetSweepLimits(latitude0, latitude1, z0, z1);

            if (fullSphere)
                divisorV = 2; // Don't think other arcs are very useful snap locations...avoid clutter and just output equator...

            double vFraction0 = detail.LatitudeToVFraction(latitude0);
            double vFraction1 = detail.LatitudeToVFraction(latitude1);

            for (int vRule = 0; vRule <= divisorV; ++vRule)
                {
                double vFraction = vFraction0 + (((vFraction1 - vFraction0) / divisorV) * vRule);

                if (fullSphere && (DoubleOps::AlmostEqual(vFraction, 0.0) || DoubleOps::AlmostEqual(vFraction, 1.0)))
                    continue; // Don't generate 0 radius arcs at top/bottom of full sphere...

                CurveVectorPtr  curve = CurveVector::Create(CurveVector::BoundaryType::BOUNDARY_TYPE_Open, ICurvePrimitive::CreateArc(detail.VFractionToUSectionDEllipse3d(vFraction)));

                TestCurveLocation(*curve, graphic);
                }

            return true;
            }

        case SolidPrimitiveType_DgnExtrusion:
            {
            if (!m_inProcessFace)
                break;

            DgnExtrusionDetail  detail;

            if (!primitive.TryGetDgnExtrusionDetail(detail))
                return true;

            bvector<DPoint3d> pts;

            CollectLateralRulePoints(*detail.m_baseCurve, pts, divisorU, divisorU * 2);

            for (size_t iPt = 0; iPt < pts.size(); ++iPt)
                {
                DSegment3d      segment = DSegment3d::FromOriginAndDirection(pts[iPt], detail.m_extrusionVector);
                CurveVectorPtr  curve = CurveVector::Create(CurveVector::BoundaryType::BOUNDARY_TYPE_Open, ICurvePrimitive::CreateLine(segment));

                TestCurveLocation(*curve, graphic);
                }

            return false; // Output face edges...
            }

        case SolidPrimitiveType_DgnRotationalSweep:
            {
            if (!m_inProcessFace)
                break;

            DgnRotationalSweepDetail  detail;

            if (!primitive.TryGetDgnRotationalSweepDetail(detail))
                return true;

            bvector<DPoint3d> pts;

            CollectLateralRulePoints(*detail.m_baseCurve, pts, divisorU, 4); // Avoid clutter and just output every 90 degrees for closed face (i.e. rotational sweep is a torus)...

            RotMatrix   axes, invAxes, tmpRMatrix;
            Transform   transform;

            invAxes.InitFrom1Vector(detail.m_axisOfRotation.direction, 2, true);
            axes.TransposeOf(invAxes);

            tmpRMatrix.InitFromPrincipleAxisRotations(axes, 0.0, 0.0, detail.m_sweepAngle);
            tmpRMatrix.InitProduct(invAxes, tmpRMatrix);
            transform.From(tmpRMatrix, detail.m_axisOfRotation.origin);

            for (size_t iPt = 0; iPt < pts.size(); ++iPt)
                {
                DEllipse3d  ellipse;

                if (!ComputeRuleArc(ellipse, pts.at(iPt), detail.m_axisOfRotation.origin, detail.m_sweepAngle, transform, axes, invAxes, 1.0e-8))
                    continue;

                CurveVectorPtr  curve = CurveVector::Create(CurveVector::BoundaryType::BOUNDARY_TYPE_Open, ICurvePrimitive::CreateArc(ellipse));

                TestCurveLocation(*curve, graphic);
                }

            bool  fullRevolve = Angle::IsFullCircle(detail.m_sweepAngle);

            if (fullRevolve)
                divisorV *= 2;

            for (int vRule = 0; vRule < divisorV; ++vRule)
                {
                double vFraction = (1.0 / divisorV) * vRule;

                if (!fullRevolve && DoubleOps::AlmostEqual(vFraction, 0.0))
                    continue;

                CurveVectorPtr  curve = detail.VFractionToProfile(vFraction);

                if (curve.IsValid())    
                    TestCurveLocation(*curve, graphic);
                }

            return false; // Output face edges...
            }
            
        case SolidPrimitiveType_DgnRuledSweep:
            {
            if (!m_inProcessFace)
                break;

            DgnRuledSweepDetail detail;
    
            if (!primitive.TryGetDgnRuledSweepDetail(detail))
                return true;

            for (size_t iProfile = 0; iProfile < detail.m_sectionCurves.size()-1; ++iProfile)
                {
                bvector<DPoint3d> rulePts1;
                bvector<DPoint3d> rulePts2;

                CollectLateralRulePoints(*detail.m_sectionCurves.at(iProfile), rulePts1, divisorU, divisorU * 2);
                CollectLateralRulePoints(*detail.m_sectionCurves.at(iProfile+1), rulePts2, divisorU, divisorU * 2);

                if (rulePts1.size() != rulePts2.size())
                    {
                    if (1 == rulePts2.size()) // Special case to handle zero scale in both XY...
                        rulePts2.insert(rulePts2.end(), rulePts1.size()-1, rulePts2.front());
                    else
                        rulePts1.clear(); rulePts2.clear();;
                    }

                for (size_t iRule = 0; iRule < rulePts1.size(); ++iRule)
                    {
                    DSegment3d      segment = DSegment3d::From(rulePts1.at(iRule), rulePts2.at(iRule));
                    CurveVectorPtr  curve = CurveVector::Create(CurveVector::BoundaryType::BOUNDARY_TYPE_Open, ICurvePrimitive::CreateLine(segment));

                    TestCurveLocation(*curve, graphic);
                    }
                }

            return false; // Output face edges...
            }
        }

    // Find selected face and output the u/v rules just for that face...
    Transform   worldToLocal;
    DPoint3d    spacePointLocal;

    worldToLocal.InverseOf(graphic.GetLocalToWorldTransform());
    worldToLocal.Multiply(&spacePointLocal, &m_snapContext.GetSnapDetail()->GetGeomDetail().GetClosestPoint(), 1);

    SolidLocationDetail location;

    if (!primitive.ClosestPoint(spacePointLocal, location))
        return true;

    IGeometryPtr faceGeom = primitive.GetFace(location.GetFaceIndices());

    if (!faceGeom.IsValid())
        return true;

    switch (faceGeom->GetGeometryType())
        {
        case IGeometry::GeometryType::CurveVector:
            {
            CurveVectorPtr  faceCurves = faceGeom->GetAsCurveVector();

            if (faceCurves.IsValid())
                TestCurveLocation(*faceCurves, graphic);
            break;
            }

        case IGeometry::GeometryType::SolidPrimitive:
            {
            ISolidPrimitivePtr facePrimitive = faceGeom->GetAsISolidPrimitive();

            if (!facePrimitive.IsValid())
                break;

            AutoRestore<bool> saveInProcessFace(&m_inProcessFace, true);
            graphic.AddSolidPrimitive(*facePrimitive);
            break;
            }

        case IGeometry::GeometryType::BsplineSurface:
            {
            MSBsplineSurfacePtr faceSurface = faceGeom->GetAsMSBsplineSurface();

            if (!faceSurface.IsValid())
                break;

            AutoRestore<bool> saveInProcessFace(&m_inProcessFace, true);
            graphic.AddBSplineSurface(*faceSurface);
            break;
            }

        default:
            {
            BeAssert(false);
            break;
            }
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   11/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool _ProcessSurface(MSBsplineSurfaceCR surface, SimplifyGraphic& graphic) override
    {
    if (!surface.uParams.closed || !surface.vParams.closed)
        {
        CurveVectorPtr curves = surface.GetUnstructuredBoundaryCurves(0.0, true, true);

        if (curves.IsValid())
            TestCurveLocation(*curves, graphic);
        }

    // NOTE: Use current keypoint snap divisor to produce uv rule curves to evaluate snap mode on...
    Transform   worldToLocal;
    DPoint3d    spacePointLocal;

    worldToLocal.InverseOf(graphic.GetLocalToWorldTransform());
    worldToLocal.Multiply(&spacePointLocal, &m_snapContext.GetSnapDetail()->GetGeomDetail().GetClosestPoint(), 1);

    DPoint3d    surfacePoint;
    DPoint2d    surfaceUV;

    surface.ClosestPoint(surfacePoint, surfaceUV, spacePointLocal);

    int divisorU = m_snapContext.GetSnapDivisor();
    int divisorV = divisorU;

    // NOTE: For closed directions we want more key points...
    if (surface.uParams.closed)
        divisorU *= 2;

    if (surface.vParams.closed)
        divisorV *= 2;
        
    int     keyFactorU = int ((surfaceUV.x / 1.0) * (double) divisorU + 0.5);
    int     keyFactorV = int ((surfaceUV.y / 1.0) * (double) divisorV + 0.5);
    double  keyParamU = (keyFactorU / (double) divisorU);
    double  keyParamV = (keyFactorV / (double) divisorV);

    if (surface.uParams.closed || (!DoubleOps::AlmostEqual(keyParamU, 0.0) && !DoubleOps::AlmostEqual(keyParamU, 1.0)))
        {
        bvector<MSBsplineCurvePtr> segmentsU;

        surface.GetIsoUCurveSegments(keyParamU, segmentsU);

        for (MSBsplineCurvePtr& isoCurveU : segmentsU)
            {
            CurveVectorPtr curve = CurveVector::Create(CurveVector::BoundaryType::BOUNDARY_TYPE_Open, ICurvePrimitive::CreateBsplineCurveSwapFromSource(*isoCurveU));

            TestCurveLocation(*curve, graphic);
            }
        }

    if (surface.vParams.closed || (!DoubleOps::AlmostEqual(keyParamV, 0.0) && !DoubleOps::AlmostEqual(keyParamV, 1.0)))
        {
        bvector<MSBsplineCurvePtr> segmentsV;

        surface.GetIsoVCurveSegments(keyParamV, segmentsV);

        for (MSBsplineCurvePtr& isoCurveV : segmentsV)
            {
            CurveVectorPtr curve = CurveVector::Create(CurveVector::BoundaryType::BOUNDARY_TYPE_Open, ICurvePrimitive::CreateBsplineCurveSwapFromSource(*isoCurveV));

            TestCurveLocation(*curve, graphic);
            }
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   11/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool _ProcessPolyface(PolyfaceQueryCR meshData, bool isFilled, SimplifyGraphic& graphic) override
    {
    if (m_inProcessFace)
        return false; // Process single face using unhandled preference...

    PolyfaceVisitorPtr  visitor = PolyfaceVisitor::Attach(meshData);
    double              tolerance = 1.0e-5;

    visitor->SetNumWrap(1);

    DPoint3d    spacePointLocal;
    Transform   worldToLocal;

    worldToLocal.InverseOf(graphic.GetLocalToWorldTransform());
    worldToLocal.Multiply(&spacePointLocal, &m_snapContext.GetSnapDetail()->GetGeomDetail().GetClosestPoint(), 1);

    for (; visitor->AdvanceToNextFace();)
        {
        DPoint3d thisFacePoint;

        if (!visitor->TryFindCloseFacetPoint(spacePointLocal, tolerance, thisFacePoint))
            continue;

        // Get a "face" containing this facet, a single facet when there are hidden edges isn't what someone would consider a face...
        bvector<ptrdiff_t> seedReadIndices;
        bvector<ptrdiff_t> allFaceBlocks;
        bvector<ptrdiff_t> activeReadIndexBlocks;
    
        meshData.PartitionByConnectivity(2, allFaceBlocks);
        seedReadIndices.push_back(visitor->GetReadIndex());
        PolyfaceHeader::SelectBlockedIndices(allFaceBlocks, seedReadIndices, true, activeReadIndexBlocks);

        bvector<PolyfaceHeaderPtr> perFacePolyfaces;

        meshData.CopyPartitions(activeReadIndexBlocks, perFacePolyfaces);

        if (0 != perFacePolyfaces.size())
            {
            AutoRestore<bool> saveInProcessFace(&m_inProcessFace, true);
            graphic.AddPolyface(*perFacePolyfaces.front());
            }

        break;
        }

    return true;
    }

#if defined (BENTLEYCONFIG_PARASOLID)
/*=================================================================================**//**
* @bsiclass                                                     Brien.Bastings  11/17
+===============+===============+===============+===============+===============+======*/
struct BRepFaceHatchProcessor : IParasolidWireOutput
    {
    SnapGraphicsProcessor&  m_processor;
    SimplifyGraphic&        m_graphic;
    Transform               m_entityTransform;

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    BrienBastings   11/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    BentleyStatus _ProcessGoOutput(ICurvePrimitiveCR hatchCurve, PK_ENTITY_t entityTag)
        {
        CurveVectorPtr curve = CurveVector::Create(CurveVector::BoundaryType::BOUNDARY_TYPE_Open, hatchCurve.Clone());

        curve->TransformInPlace(m_entityTransform);
        m_processor.TestCurveLocation(*curve, m_graphic);

        return (m_processor.m_snapContext.CheckStop() ? ERROR : SUCCESS);
        }

    BRepFaceHatchProcessor(SnapGraphicsProcessor& processor, SimplifyGraphic& graphic, TransformCR entityTransform) : m_processor(processor), m_graphic(graphic), m_entityTransform(entityTransform) {}

    }; // BRepFaceHatchProcessor

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   11/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool _ProcessBody(IBRepEntityCR entity, SimplifyGraphic& graphic) override
    {
    if (IBRepEntity::EntityType::Wire == entity.GetEntityType())
        return false; // Output edge geometry...

    Transform   worldToLocal;
    DPoint3d    spacePointLocal;

    worldToLocal.InverseOf(graphic.GetLocalToWorldTransform());
    worldToLocal.Multiply(&spacePointLocal, &m_snapContext.GetSnapDetail()->GetGeomDetail().GetClosestPoint(), 1);

    ISubEntityPtr closeEntity = BRepUtil::ClosestSubEntity(entity, spacePointLocal);

    if (!closeEntity.IsValid())
        return true;

    switch (closeEntity->GetSubEntityType())
        {
        case ISubEntity::SubEntityType::Edge:
        case ISubEntity::SubEntityType::Vertex:
            {
            GeometricPrimitiveCPtr geom = closeEntity->GetGeometry();

            if (geom.IsValid() && GeometricPrimitive::GeometryType::CurvePrimitive == geom->GetGeometryType())
                {
                CurveVectorPtr curve = CurveVector::Create(CurveVector::BoundaryType::BOUNDARY_TYPE_Open, geom->GetAsICurvePrimitive());

                TestCurveLocation(*curve, graphic);
                }

            return true;
            }

        case ISubEntity::SubEntityType::Face:
            {
            CurveVectorPtr faceCurves = BRepUtil::Create::PlanarFaceToCurveVector(*closeEntity);

            if (faceCurves.IsValid())
                {
                TestCurveLocation(*faceCurves, graphic);

                return true;
                }

            break;
            }
        }

    int divisor = m_snapContext.GetSnapDivisor();

    BRepFaceHatchProcessor output(*this, graphic, entity.GetEntityTransform());

    PSolidGoOutput::ProcessFaceHatching(output, divisor, PSolidSubEntity::GetSubEntityTag(*closeEntity));

    bvector<ISubEntityPtr> faceEdges;

    if (SUCCESS != BRepUtil::GetFaceEdges(faceEdges, *closeEntity))
        return true;

    for (ISubEntityPtr& edge : faceEdges)
        {
        if (m_snapContext.CheckStop())
            return true;

        GeometricPrimitiveCPtr geom = edge->GetGeometry();

        if (!geom.IsValid() || GeometricPrimitive::GeometryType::CurvePrimitive != geom->GetGeometryType())
            continue;

        CurveVectorPtr curve = CurveVector::Create(CurveVector::BoundaryType::BOUNDARY_TYPE_Open, geom->GetAsICurvePrimitive());

        TestCurveLocation(*curve, graphic);
        }

    return true;
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   08/15
+---------------+---------------+---------------+---------------+---------------+------*/
void _OutputGraphics(ViewContextR context) override
    {
    SnapDetailP      snap = m_snapContext.GetSnapDetail();
    DgnElementCPtr   element = snap->GetElement();
    GeometrySourceCP source = (element.IsValid() ? element->ToGeometrySource() : nullptr);

    if (nullptr == source)
        {
        IElemTopologyCP elemTopo = snap->GetElemTopology();
        if (nullptr == (source = (nullptr != elemTopo ? elemTopo->_ToGeometrySource() : nullptr)))
            return;
        }

    // Attach viewport from snap context for cone silhouettes...
    context.Attach(m_snapContext.GetViewport(), _GetProcessPurpose());

    // Get the geometry for this hit from the GeometryStream...
    GeometryCollection collection(*source);
    Render::GraphicBuilderPtr graphic;

    GeometryStreamEntryId snapElemEntryId = snap->GetGeomDetail().GetGeometryStreamEntryId();
    GeometryStreamEntryId snapPartEntryId = snap->GetGeomDetail().GetGeometryStreamEntryId(true);

    for (auto iter : collection)
        {
        // Quick exclude of geometry that didn't generate the hit...
        if (snapElemEntryId != iter.GetGeometryStreamEntryId())
            continue;

        GeometricPrimitivePtr geom;

        if (nullptr != source && GeometryCollection::Iterator::EntryType::BRepEntity == iter.GetEntryType())
            {
            IBRepEntityPtr entity = BRepDataCache::FindCachedBRepEntity(*element, snapElemEntryId);

            if (entity.IsValid())
                geom = GeometricPrimitive::Create(entity);
            }

        if (!geom.IsValid())
            geom = iter.GetGeometryPtr();

        if (geom.IsValid())
            {
            if (!graphic.IsValid())
                graphic = context.CreateSceneGraphic(iter.GetGeometryToWorld());

            geom->AddToGraphic(*graphic);

            if (iter.IsBRepPolyface())
                continue; // Keep going, want to draw all matching geometry (multi-symb BRep is Polyface per-symbology)...
            break;
            }

        DgnGeometryPartCPtr geomPart = iter.GetGeometryPartCPtr();

        if (!geomPart.IsValid())
            return; // Shouldn't happen...

        GeometryCollection partCollection(geomPart->GetGeometryStream(), context.GetDgnDb());

        partCollection.SetNestedIteratorContext(iter); // Iterate part GeomStream in context of parent...

        for (auto partIter : partCollection)
            {
            // Quick exclude of part geometry that didn't generate the hit...pass true to compare part geometry index...
            if (snapPartEntryId != partIter.GetGeometryStreamEntryId())
                continue;

            GeometricPrimitivePtr partGeom;

            if (GeometryCollection::Iterator::EntryType::BRepEntity == partIter.GetEntryType())
                {
                IBRepEntityPtr entity = BRepDataCache::FindCachedBRepEntity(*geomPart, snapPartEntryId);

                if (entity.IsValid())
                    partGeom = GeometricPrimitive::Create(entity);
                }

            if (!partGeom.IsValid())
                partGeom = partIter.GetGeometryPtr();                

            if (!partGeom.IsValid())
                continue;

            if (!graphic.IsValid())
                graphic = context.CreateSceneGraphic(partIter.GetGeometryToWorld());

            partGeom->AddToGraphic(*graphic);

            if (partIter.IsBRepPolyface())
                continue; // Keep going, want to draw all matching geometry (multi-symb BRep is Polyface per-symbology)...
            break;
            }

        break; // Done with part...
        }
    }

public:

SnapGraphicsProcessor(SnapContextR snapContext) : m_snapContext(snapContext) {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   08/15
+---------------+---------------+---------------+---------------+---------------+------*/
static bool DoSnapUsingClosestCurve(SnapContextR snapContext)
    {
    SnapGraphicsProcessor processor(snapContext);

    GeometryProcessor::Process(processor, snapContext.GetDgnDb());

    if (nullptr == snapContext.GetSnapDetail()->GetGeomDetail().GetCurvePrimitive())
        return false; // No edge found...

    return (SnapStatus::Success == snapContext.DoSnapUsingCurve(snapContext.GetSnapMode()) ? true : false);
    }

}; // SnapEdgeProcessor
END_BENTLEY_DGN_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/06
+---------------+---------------+---------------+---------------+---------------+------*/
SnapStatus SnapContext::DoDefaultDisplayableSnap()
    {
    SnapDetailP     snap = GetSnapDetail();
    SnapMode        snapMode = GetSnapMode();
    GeomDetailCR    detail = snap->GetGeomDetail();

    // Don't require a curve primitive if hit geom is point or mode is nearest because current hit point is correct...
    if (SnapMode::Nearest == snapMode || HitGeomType::Point == detail.GetGeomType())
        {
        DPoint3d    hitPoint = snap->GetHitPoint();

        SetSnapInfo(snapMode, GetSnapSprite(snapMode), hitPoint, false, false);

        return SnapStatus::Success;
        }

    if (nullptr == detail.GetCurvePrimitive())
        {
        // Surface w/o curve is interior hit...only nearest should "track" surface...
        if (HitGeomType::Surface == detail.GetGeomType())
            {
            DgnElementCPtr   element = snap->GetElement();
            GeometrySourceCP source = (element.IsValid() ? element->ToGeometrySource() : nullptr);

            if (SnapMode::Origin != snapMode || nullptr == source)
                {
                // NOTE: This is a fairly expensive proposition...but we need something more meaningful than snap to center of range...
                if (SnapGraphicsProcessor::DoSnapUsingClosestCurve(*this))
                    return SnapStatus::Success;
                else if (nullptr == source)
                    return SnapStatus::NotSnappable; // NOTE: Don't assume placement is meaningful for non-element hits (probably identity)...
                }

            DPoint3d hitPoint;

            source->GetPlacementTransform().GetTranslation(hitPoint);
            SetSnapInfo(snapMode, GetSnapSprite(snapMode), hitPoint, false, false);

            return SnapStatus::Success;
            }

        return SnapStatus::NotSnappable;
        }

    return DoSnapUsingCurve(snapMode);
    }

/*---------------------------------------------------------------------------------**//**
* Call this routine if the point to which this snap should be placed is different from the
* point that actually generated the snap.
* @bsimethod                                                    KeithBentley    05/01
+---------------+---------------+---------------+---------------+---------------+------*/
void SnapContext::SetAdjustedSnapPoint(DPoint3dCR adjustedPt)
    {
    m_snapPath->SetAdjustedPoint(adjustedPt);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/04
+---------------+---------------+---------------+---------------+---------------+------*/
KeypointType SnapContext::GetSnapKeypointType(SnapMode snapMode)
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
SnapStatus SnapContext::DoTextSnap()
    {
    SnapDetailP     snap = GetSnapDetail();
    SnapMode        snapMode = GetSnapMode();
    GeomDetailCR    detail = snap->GetGeomDetail();

    switch (snapMode)
        {
        case SnapMode::Origin:
        case SnapMode::NearestKeypoint:
            {
            DgnElementCPtr   element = snap->GetElement();
            GeometrySourceCP source = (element.IsValid() ? element->ToGeometrySource() : nullptr);

            if (nullptr == source)
                return SnapStatus::NotSnappable;

            DPoint3d hitPoint = (nullptr != source->GetAsGeometrySource3d() ? source->GetAsGeometrySource3d()->GetPlacement().GetOrigin() : DPoint3d::From(source->GetAsGeometrySource2d()->GetPlacement().GetOrigin()));
            
            SetSnapInfo(snapMode, GetSnapSprite(snapMode), hitPoint, false, false);

            return SnapStatus::Success;
            }

        case SnapMode::MidPoint:
        case SnapMode::Bisector:
        case SnapMode::Center:
            {
            if (NULL == detail.GetCurvePrimitive())
                return DoDefaultDisplayableSnap(); // NOTE: Boundary shape unavailable for origin only...always have for edge/interior hit...

            DPoint3d    centroid;

            if (!getCentroid(centroid, *detail.GetCurvePrimitive()))
                return SnapStatus::NotSnappable;

            SetSnapInfo(snapMode, GetSnapSprite(snapMode), centroid, false, false);

            return SnapStatus::Success;
            }

        case SnapMode::Nearest:
            {
            if (nullptr == detail.GetCurvePrimitive())
                return DoDefaultDisplayableSnap(); // NOTE: Boundary shape unavailable for origin only...always have for edge/interior hit...

            DSegment3d  segment;

            if (!detail.GetSegment(segment))
                return SnapStatus::NotSnappable;

            double      keyparam = detail.GetSegmentParam();
            DPoint3d    hitPoint;

            GetSegmentKeypoint(hitPoint, keyparam, GetSnapDivisor(), segment);

            SetSnapInfo(snapMode, GetSnapSprite(snapMode), hitPoint, false, false);

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
void SnapDetail::SetCustomKeypoint(int nBytes, Byte* dataP)
    {
    if (nBytes && NULL != (m_customKeypointData = (Byte *) bentleyAllocator_malloc(nBytes)))
        {
        m_customKeypointSize = nBytes;
        memcpy(m_customKeypointData, dataP, nBytes);

        // NOTE: Clear curve topo id so we don't try to create TopologyCurveAssociation instead of handler's custom assoc!
        ICurvePrimitiveCP hitPrimitive = m_geomDetail.GetCurvePrimitive();

        if (hitPrimitive)
            hitPrimitive->SetId(NULL); 
        }
    }
