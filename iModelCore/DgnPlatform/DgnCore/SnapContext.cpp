/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/SnapContext.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <GeomSerialization/GeomLibsSerialization.h>
#include <GeomSerialization/GeomLibsJsonSerialization.h>
#if defined (BENTLEYCONFIG_PARASOLID) 
#include <DgnPlatform/DgnBRep/PSolidUtil.h>
#endif

BEGIN_BENTLEY_DGN_NAMESPACE
/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct SnapGeometryHelper
{
private:

SnapMode                m_snapMode;
int                     m_snapDivisor;
double                  m_snapAperture;
DPoint3d                m_closePtWorld;
DMap4d                  m_worldToView;
CheckStop*              m_stopTester = nullptr;

GeometricPrimitivePtr   m_hitGeom;
Transform               m_hitLocalToWorld = Transform::FromIdentity();
GeometryParams          m_hitParams;
GeometryStreamEntryId   m_hitEntryId;
CurveLocationDetail     m_hitCurveDetail;
DVec3d                  m_hitNormalLocal = DVec3d::FromZero();
HitGeomType             m_hitGeomType = HitGeomType::None;
HitParentGeomType       m_hitParentGeomType = HitParentGeomType::None;

DPoint3d                m_hitClosePtLocal;
Transform               m_hitWorldToLocal;
DMap4d                  m_hitLocalToView;
double                  m_hitDistanceView;
ICurvePrimitivePtr      m_hitCurveDerived;

protected:

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   05/18
+---------------+---------------+---------------+---------------+---------------+------*/
bool EvaluateInterior()
    {
    if (!m_hitGeom.IsValid())
        return false;

    // NOTE: Nearest snap tracks surface when edge not within locate aperture...
    bool        interiorPt = (HitGeomType::Point != m_hitGeomType && SnapMode::Nearest == m_snapMode && m_hitDistanceView > m_snapAperture);
    DPoint3d    localPoint = interiorPt ? m_hitClosePtLocal : m_hitCurveDetail.point; // NOTE: m_hitClosePtLocal is only exact when using PickContext...

    switch (m_hitGeom->GetGeometryType())
        {
        case GeometricPrimitive::GeometryType::CurveVector:
            {
            CurveVectorCR curves = *m_hitGeom->GetAsCurveVector();

            if (!curves.IsAnyRegionType())
                return false;

            SolidLocationDetail solidDetail;
            DRay3d boresite = GetBoresite(localPoint, m_hitLocalToView.M1);

            CurveVector::InOutClassification result = curves.RayPierceInOnOut(boresite, solidDetail);
            if (CurveVector::InOutClassification::INOUT_Unknown == result)
                return false;

            if (interiorPt)
                m_hitCurveDetail.point = solidDetail.GetXYZ();
            m_hitNormalLocal.NormalizedCrossProduct(solidDetail.GetUDirection(), solidDetail.GetVDirection());
            break;
            }

        case GeometricPrimitive::GeometryType::SolidPrimitive:
            {
            bool                useCurvePoint = (HitGeomType::Point == m_hitGeomType || SnapMode::Center == m_snapMode);
            DPoint3d            testPointLocal = (useCurvePoint ? m_hitCurveDetail.point : m_hitClosePtLocal); // Prefer face identified by original hit location to disambiguate face for edge...
            SolidLocationDetail solidDetail;

            if (!m_hitGeom->GetAsISolidPrimitive()->ClosestPoint(testPointLocal, solidDetail))
                return false;

            if (useCurvePoint && testPointLocal.Distance(solidDetail.GetXYZ()) > 1.0e-5)
                return false;

            // NOTE: U/V directions don't ensure outward normals, so instead of trying to compare normals, we'll just look for a different face at point closer to eye...
            DRay3d boresite = GetBoresite(localPoint, m_hitLocalToView.M1);
            DPoint3d testPt;
            SolidLocationDetail offsetDetail;
                    
            testPt.SumOf(solidDetail.GetXYZ(), boresite.direction, -1.0e-3);

            if (m_hitGeom->GetAsISolidPrimitive()->ClosestPoint(testPt, offsetDetail) && !offsetDetail.GetFaceIndices().Is(solidDetail.GetFaceIndices().Index0(), solidDetail.GetFaceIndices().Index1(), solidDetail.GetFaceIndices().Index2()))
                solidDetail = offsetDetail;

            IGeometryPtr faceGeom = m_hitGeom->GetAsISolidPrimitive()->GetFace(solidDetail.GetFaceIndices());

            if (!faceGeom.IsValid())
                return false;

            switch (faceGeom->GetGeometryType())
                {
                case IGeometry::GeometryType::CurveVector:
                    {
                    CurveVector::InOutClassification result = faceGeom->GetAsCurveVector()->RayPierceInOnOut(boresite, solidDetail);
                    if (CurveVector::InOutClassification::INOUT_Unknown == result)
                        return false;

                    if (interiorPt)
                        m_hitCurveDetail.point = solidDetail.GetXYZ();
                    m_hitNormalLocal.NormalizedCrossProduct(solidDetail.GetUDirection(), solidDetail.GetVDirection());
                    break;
                    }

                case IGeometry::GeometryType::SolidPrimitive:
                    {
                    if (!faceGeom->GetAsISolidPrimitive()->ClosestPoint(localPoint, solidDetail))
                        return false;

                    DVec3d    uDir, vDir;
                    DPoint3d  point;

                    if (!faceGeom->GetAsISolidPrimitive()->TryUVFractionToXYZ(solidDetail.GetFaceIndices(), solidDetail.GetU(), solidDetail.GetV(), point, uDir, vDir))
                        return false;

                    if (interiorPt)
                        m_hitCurveDetail.point = point;
                    m_hitNormalLocal.NormalizedCrossProduct(uDir, vDir);
                    break;
                    }

                case IGeometry::GeometryType::BsplineSurface:
                    {
                    DPoint3d  surfacePoint;
                    DPoint2d  surfaceUV;
                    DVec3d    uDir, vDir, dPdUU, dPdVV, dPdUV;

                    faceGeom->GetAsMSBsplineSurface()->ClosestPoint(surfacePoint, surfaceUV, localPoint);

                    if (interiorPt)
                        m_hitCurveDetail.point = surfacePoint;
                    faceGeom->GetAsMSBsplineSurface()->EvaluateAllPartials(surfacePoint, uDir, vDir, dPdUU, dPdVV, dPdUV, m_hitNormalLocal, surfaceUV.x, surfaceUV.y);
                    m_hitNormalLocal.Normalize();
                    break;
                    }

                default:
                    return false;
                }
            break;
            }

        case GeometricPrimitive::GeometryType::BsplineSurface:
            {
            DPoint3d  surfacePoint;
            DPoint2d  surfaceUV;
            DVec3d    uDir, vDir, dPdUU, dPdVV, dPdUV;

            m_hitGeom->GetAsMSBsplineSurface()->ClosestPoint(surfacePoint, surfaceUV, localPoint);

            if (interiorPt)
                m_hitCurveDetail.point = surfacePoint;
            m_hitGeom->GetAsMSBsplineSurface()->EvaluateAllPartials(surfacePoint, uDir, vDir, dPdUU, dPdVV, dPdUV, m_hitNormalLocal, surfaceUV.x, surfaceUV.y);
            m_hitNormalLocal.Normalize();
            break;
            }

        case GeometricPrimitive::GeometryType::Polyface:
            {
            PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach(*m_hitGeom->GetAsPolyfaceHeader());
            bool found = false;

            visitor->SetNumWrap(1);

            for (; visitor->AdvanceToNextFace();)
                {
                DPoint3d thisPoint;

                if (!visitor->TryFindCloseFacetPoint(localPoint, 1.0e-5, thisPoint))
                    continue;

                FacetLocationDetail thisDetail;

                if (!visitor->LoadVertexData(thisDetail, 0))
                    continue;

                if (interiorPt)
                    m_hitCurveDetail.point = thisPoint;

                DVec3d thisNormal;

                if (thisDetail.TryGetNormal(thisNormal))
                    m_hitNormalLocal.Normalize(thisNormal);
                found = true;
                break;
                }

            if (!found)
                return false;
            break;
            }

#if defined (BENTLEYCONFIG_PARASOLID)
        case GeometricPrimitive::GeometryType::BRepEntity:
            {
            bool           useCurvePoint = (HitGeomType::Point == m_hitGeomType || SnapMode::Center == m_snapMode);
            DPoint3d       testPointLocal = (useCurvePoint ? m_hitCurveDetail.point : m_hitClosePtLocal); // Prefer face identified by original hit location to disambiguate face for edge...
            DRay3d         boresite = GetBoresite(testPointLocal, m_hitLocalToView.M1);
            DVec3d         viewZ = DVec3d::FromScale(boresite.direction, -1.0);
            IBRepEntityCR  entity = *m_hitGeom->GetAsIBRepEntity();
            ISubEntityPtr  closeEntity = BRepUtil::ClosestFace(entity, testPointLocal, &viewZ); // Prefer face identified by original hit location...

            if (!closeEntity.IsValid())
                return false;
                
            DVec3d    normal, uDir, vDir;
            DPoint2d  param;
            DPoint3d  point;

            if (!BRepUtil::ClosestPointToFace(*closeEntity, localPoint, point, param) || SUCCESS != BRepUtil::EvaluateFace(*closeEntity, point, normal, uDir, vDir, param))
                return false;

            if (useCurvePoint && testPointLocal.Distance(point) > 1.0e-5)
                return false; // reject point not on surface...

            if (interiorPt)
                m_hitCurveDetail.point = point;
            m_hitNormalLocal = normal;
            break;
            }
#endif

        default:
            {
            // Shouldn't be any surface type not covered above...
            return false;
            }
        }

    if (interiorPt)
        {
        m_hitGeomType = HitGeomType::Surface;
        m_hitCurveDetail.curve = nullptr; // Don't flash single curve primitive when snapping to interior...
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   05/18
+---------------+---------------+---------------+---------------+---------------+------*/
bool EvaluateArcCenter()
    {
    if (nullptr == m_hitCurveDetail.curve || ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc != m_hitCurveDetail.curve->GetCurvePrimitiveType())
        return false;

    DEllipse3dCP arc = m_hitCurveDetail.curve->GetArcCP();

    if (fabs(arc->sweep) < Angle::PiOver2())
        return false;

    DPoint3d localPts[2];
    DPoint4d viewPts[2];

    localPts[0] = m_hitClosePtLocal;
    localPts[1] = arc->center;

    // NOTE: For keypoint snap to interior, choose arc center if interior point is "closer" to it than to the edge...
    if (SnapMode::NearestKeypoint == m_snapMode && (HitParentGeomType::Sheet == m_hitParentGeomType || HitParentGeomType::Solid == m_hitParentGeomType))
        {
        double radius = DoubleOps::Min(arc->vector0.Magnitude(), arc->vector90.Magnitude());

        if (localPts[0].Distance(localPts[1]) > (radius * 0.25))
            return false;
        }
    else
        {
        m_hitLocalToView.M0.Multiply(viewPts, localPts, nullptr, 2);

        if (viewPts[0].RealDistance(viewPts[1]) > m_snapAperture)
            return false;
        }

    m_hitGeomType = HitGeomType::Point;
    m_hitCurveDetail.point = arc->center;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   05/18
+---------------+---------------+---------------+---------------+---------------+------*/
bool EvaluateCurve()
    {
    if (nullptr == m_hitCurveDetail.curve)
        return (SnapMode::Nearest == m_snapMode || HitGeomType::Point == m_hitGeomType);

    SnapMode effectiveSnapMode = m_snapMode;

    if (HitParentGeomType::Text == m_hitParentGeomType)
        {
        switch (effectiveSnapMode)
            {
            case SnapMode::Nearest:
                effectiveSnapMode = SnapMode::NearestKeypoint;
                break;

            case SnapMode::MidPoint:
            case SnapMode::Bisector:
                effectiveSnapMode = SnapMode::Center;
                break;

            case SnapMode::NearestKeypoint:
                effectiveSnapMode = SnapMode::Origin; // NOTE: Will use lower left of identified TextString. Ideally we'd want user origin for overall text block, but we can't get that from just the GeometryStream...
                break;
            }
        }

    switch (effectiveSnapMode)
        {
        case SnapMode::Nearest:
            {
            EvaluateArcCenter();
            return true;
            }

        case SnapMode::Origin:
            {
            DPoint3d hitPoint;

            if (!m_hitCurveDetail.curve->GetStartPoint(hitPoint))
                return false;

            m_hitCurveDetail.point = hitPoint;
            return true;
            }

        case SnapMode::MidPoint:
            {
            switch (m_hitCurveDetail.curve->GetCurvePrimitiveType())
                {
                case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line:
                    {
                    DSegment3dCP segment = m_hitCurveDetail.curve->GetLineCP();

                    m_hitCurveDetail.point.Interpolate(segment->point[0], 0.5, segment->point[1]);
                    return true;
                    }

                case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString:
                    {
                    DSegment3d segment;

                    if (!m_hitCurveDetail.curve->TryGetSegmentInLineString(segment, m_hitCurveDetail.componentIndex))
                        return false;

                    m_hitCurveDetail.point.Interpolate(segment.point[0], 0.5, segment.point[1]);
                    return true;
                    }

                case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc:
                    {
                    if (EvaluateArcCenter())
                        return true;

                    DEllipse3dCP ellipse = m_hitCurveDetail.curve->GetArcCP();

                    ellipse->FractionParameterToPoint(m_hitCurveDetail.point, 0.5);
                    return true;
                    }
                }

            // Fall through to bisector if not handled...
            }

        case SnapMode::Bisector:
            {
            double length;

            if (!m_hitCurveDetail.curve->Length(length))
                return false;

            CurveLocationDetail location;

            if (!m_hitCurveDetail.curve->PointAtSignedDistanceFromFraction(0.0, length * 0.5, false, location))
                return false;

            m_hitCurveDetail.point = location.point;
            return true;
            }

        case SnapMode::Center:
            {
            DPoint3d centroid;

            if (m_hitGeom.IsValid() && ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc != m_hitCurveDetail.curve->GetCurvePrimitiveType() && GeometricPrimitive::GeometryType::CurveVector == m_hitGeom->GetGeometryType())
                {
                CurveVectorCR curves = *m_hitGeom->GetAsCurveVector();

                if (curves.IsAnyRegionType() && GetAreaCentroid(centroid, curves))
                    {
                    m_hitCurveDetail.point = centroid;

                    if (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Invalid == curves.HasSingleCurvePrimitive())
                        {
                        m_hitGeomType = HitGeomType::Surface;
                        m_hitCurveDetail.curve = nullptr; // Don't flash single curve primitive when snapping to center of region...
                        }
                    
                    return true;
                    }
                }

            if (!GetCentroid(centroid, *m_hitCurveDetail.curve))
                return false;

            m_hitCurveDetail.point = centroid;
            return true;
            }

        case SnapMode::NearestKeypoint:
            {
            switch (m_hitCurveDetail.curve->GetCurvePrimitiveType())
                {
                case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line:
                    {
                    DSegment3dCP segment = m_hitCurveDetail.curve->GetLineCP();

                    SnapContext::GetSegmentKeypoint(m_hitCurveDetail.point, m_hitCurveDetail.componentFraction, m_snapDivisor, *segment);
                    return true;
                    }

                case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString:
                    {
                    DSegment3d segment;

                    if (!m_hitCurveDetail.curve->TryGetSegmentInLineString(segment, m_hitCurveDetail.componentIndex))
                        return false;

                    SnapContext::GetSegmentKeypoint(m_hitCurveDetail.point, m_hitCurveDetail.componentFraction, m_snapDivisor, segment);
                    return true;
                    }

                case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc:
                    {
                    if (EvaluateArcCenter())
                        return true;

                    // FALL THROUGH...
                    }

                default:
                    {
                    DPoint3d hitPoint;

                    if (!SnapContext::GetParameterKeypoint(*m_hitCurveDetail.curve, hitPoint, m_hitCurveDetail.fraction, m_snapDivisor))
                        return false;

                    m_hitCurveDetail.point = hitPoint;
                    return true;
                    }
                }
            }

        default:
            {
            // Should never be called with "exotic" snap modes...
            return false;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   05/18
+---------------+---------------+---------------+---------------+---------------+------*/
void SimplifyHitDetail()
    {
    if (nullptr == m_hitCurveDetail.curve)
        return;

    switch (m_hitCurveDetail.curve->GetCurvePrimitiveType())
        {
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line:
            {
            DSegment3dCP segment = m_hitCurveDetail.curve->GetLineCP();

            if (segment->point[0].IsEqual(segment->point[1])) // Check for zero length line and don't include redundant primitive...
                {
                m_hitCurveDetail.curve = nullptr;
                m_hitGeomType = HitGeomType::Point;
                }

            break;
            }

        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString:
            {
            bvector<DPoint3d> const* points = m_hitCurveDetail.curve->GetLineStringCP();

            if ((1 == points->size()) || (2 == points->size() && points->at(0).IsEqual(points->at(1))))
                {
                m_hitCurveDetail.curve = nullptr;
                m_hitGeomType = HitGeomType::Point;
                }

            break;
            }

        default:
            {
            MSBsplineCurveCP bcurve = m_hitCurveDetail.curve->GetProxyBsplineCurveCP();

            if (nullptr == bcurve || 2 != bcurve->params.order)
                break;
    
            // An order 2 bspline curve should be treated the same as a linestring for snapping...
            bvector<DPoint3d> poles;

            bcurve->GetUnWeightedPoles(poles);

            if (bcurve->params.closed)
                poles.push_back(poles.at(0));

            ICurvePrimitivePtr  curve = ICurvePrimitive::CreateLineString(poles);
            CurveLocationDetail detail;

            if (!curve->ClosestPointBounded(m_hitCurveDetail.point, detail))
                break;

            CurvePrimitiveIdCP curveId = m_hitCurveDetail.curve->GetId();

            if (nullptr != curveId)
                curve->SetId(curveId->Clone().get()); // Preserve curve topology id from source curve...

            detail.a = m_hitCurveDetail.a; // Preserve distance to original hit...

            m_hitCurveDerived = curve;
            m_hitCurveDetail = detail;
            break;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   05/18
+---------------+---------------+---------------+---------------+---------------+------*/
bool UpdateIfCloser(CurveLocationDetailCR curveDetail, HitGeomType geomType, HitParentGeomType parentGeomType)
    {
    if ((nullptr == m_hitCurveDetail.curve && HitGeomType::None == m_hitGeomType) || (curveDetail.a < m_hitCurveDetail.a))
        {
        m_hitCurveDerived   = const_cast<ICurvePrimitiveP>(curveDetail.curve); // Make sure m_hitCurveDetail.curve remains valid...
        m_hitCurveDetail    = curveDetail;
        m_hitGeomType       = geomType;
        m_hitParentGeomType = parentGeomType;

        return true;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   05/18
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint3d GetClosePointLocal(Transform worldToLocalTrans)
    {
    DPoint3d    closePointLocal = m_closePtWorld;

    worldToLocalTrans.Multiply(closePointLocal);

    return closePointLocal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   05/18
+---------------+---------------+---------------+---------------+---------------+------*/
DMatrix4d GetViewToLocal(Transform worldToLocalTrans)
    {
    DMatrix4d   worldToLocal = DMatrix4d::From(worldToLocalTrans);
    DMatrix4d   viewToWorld = m_worldToView.M1;
    DMatrix4d   viewToLocal;

    viewToLocal.InitProduct(worldToLocal, viewToWorld);

    return viewToLocal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   05/18
+---------------+---------------+---------------+---------------+---------------+------*/
DRay3d GetBoresite(DPoint3dCR localPoint, DMatrix4dCR viewToLocal)
    {
    double      aa;
    DRay3d      boresite;
    DPoint4d    eyePoint;

    viewToLocal.GetColumn(eyePoint, 2);
    boresite.direction.Init(eyePoint.x, eyePoint.y, eyePoint.z);
    boresite.origin = localPoint;

    if (DoubleOps::SafeDivide(aa, 1.0, eyePoint.w, 1.0))
        {
        DPoint3d  xyzEye;

        xyzEye.Scale(boresite.direction, aa);
        boresite.direction.DifferenceOf(xyzEye, boresite.origin);
        }

    boresite.direction.Normalize();
    boresite.direction.Negate();

    return boresite;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   05/18
+---------------+---------------+---------------+---------------+---------------+------*/
bool ProcessPointString(ICurvePrimitiveCR curve, DPoint3dCR localPoint)
    {
    // NOTE: When using PickContext, this code won't be used. When locating from tiles, localPoint is known to be a vertex, so we can just use ClosestPointBounded.
    CurveLocationDetail curveDetail;

    if (!curve.ClosestPointBounded(localPoint, curveDetail))
        return false;

    curveDetail.curve = nullptr; // Don't store curve for point string hit...

    return UpdateIfCloser(curveDetail, HitGeomType::Point, HitParentGeomType::None);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   05/18
+---------------+---------------+---------------+---------------+---------------+------*/
bool ProcessICurvePrimitive(ICurvePrimitiveCR curve, DPoint3dCR localPoint, HitGeomType geomType, HitParentGeomType parentGeomType)
    {
    CurveLocationDetail curveDetail;

    if (!curve.ClosestPointBounded(localPoint, curveDetail))
        return false;

    return UpdateIfCloser(curveDetail, geomType, parentGeomType);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   05/18
+---------------+---------------+---------------+---------------+---------------+------*/
bool ProcessCurveVector(CurveVectorCR curves, DPoint3dCR localPoint, HitGeomType geomType, HitParentGeomType parentGeomType)
    {
    CurveLocationDetail curveDetail;

    if (!curves.ClosestPointBounded(localPoint, curveDetail))
        return false;

    return UpdateIfCloser(curveDetail, geomType, parentGeomType);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   05/18
+---------------+---------------+---------------+---------------+---------------+------*/
bool ProcessBsplineSurface(MSBsplineSurfaceCR surface, DPoint3dCR localPoint, HitParentGeomType parentGeomType)
    {
    bool    foundCurve = false;

    if (!surface.uParams.closed || !surface.vParams.closed)
        {
        CurveVectorPtr curves = surface.GetUnstructuredBoundaryCurves(0.0, true, true);

        if (curves.IsValid() && ProcessCurveVector(*curves, localPoint, HitGeomType::None, parentGeomType))
            foundCurve = true;

        if (nullptr != m_stopTester && m_stopTester->_CheckStop())
            return false;
        }

    DPoint3d    surfacePoint;
    DPoint2d    surfaceUV;

    surface.ClosestPoint(surfacePoint, surfaceUV, localPoint);

    int divisorU = m_snapDivisor;
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
            ICurvePrimitivePtr  curve = ICurvePrimitive::CreateBsplineCurveSwapFromSource(*isoCurveU);

            if (ProcessICurvePrimitive(*curve, localPoint, HitGeomType::Surface, parentGeomType))
                foundCurve = true;

            if (nullptr != m_stopTester && m_stopTester->_CheckStop())
                return false;
            }
        }

    if (surface.vParams.closed || (!DoubleOps::AlmostEqual(keyParamV, 0.0) && !DoubleOps::AlmostEqual(keyParamV, 1.0)))
        {
        bvector<MSBsplineCurvePtr> segmentsV;

        surface.GetIsoVCurveSegments(keyParamV, segmentsV);

        for (MSBsplineCurvePtr& isoCurveV : segmentsV)
            {
            ICurvePrimitivePtr  curve = ICurvePrimitive::CreateBsplineCurveSwapFromSource(*isoCurveV);

            if (ProcessICurvePrimitive(*curve, localPoint, HitGeomType::Surface, parentGeomType))
                foundCurve = true;

            if (nullptr != m_stopTester && m_stopTester->_CheckStop())
                return false;
            }
        }

    return foundCurve;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   05/18
+---------------+---------------+---------------+---------------+---------------+------*/
bool ProcessSingleFaceSolidPrimitive(ISolidPrimitiveCR primitive, DPoint3dCR localPoint, TransformCR worldToLocal, HitParentGeomType parentGeomType)
    {
    int     divisorU = m_snapDivisor;
    int     divisorV = divisorU;
    bool    foundCurve = false;

    switch (primitive.GetSolidPrimitiveType())
        {
        case SolidPrimitiveType_DgnTorusPipe:
            {
            DgnTorusPipeDetail detail;

            if (!primitive.TryGetDgnTorusPipeDetail(detail))
                return false;

            divisorU = 4; // Don't think other arcs are very useful snap locations...avoid clutter and just output every 90 degrees...

            for (int uRule = 0; uRule < divisorU; ++uRule)
                {
                double              uFraction = (1.0 / divisorU) * uRule;
                ICurvePrimitivePtr  curve = ICurvePrimitive::CreateArc(detail.UFractionToVSectionDEllipse3d(uFraction));

                if (ProcessICurvePrimitive(*curve, localPoint, HitGeomType::Surface, parentGeomType))
                    foundCurve = true;
                }

            bool  fullTorus = Angle::IsFullCircle(detail.m_sweepAngle);

            if (fullTorus)
                divisorV *= 2;

            for (int vRule = 0; vRule <= divisorV; ++vRule)
                {
                double vFraction = (1.0 / divisorV) * vRule;

                if (fullTorus && DoubleOps::AlmostEqual(vFraction, 1.0))
                    continue; // Don't duplicate first arc for a full torus...

                HitGeomType hitGeomType = HitGeomType::Surface;

                if (!fullTorus && (DoubleOps::AlmostEqual(vFraction, 0.0) || DoubleOps::AlmostEqual(vFraction, 1.0)))
                    hitGeomType = HitGeomType::None;

                ICurvePrimitivePtr  curve = ICurvePrimitive::CreateArc(detail.VFractionToUSectionDEllipse3d(vFraction));

                if (ProcessICurvePrimitive(*curve, localPoint, hitGeomType, parentGeomType))
                    foundCurve = true;
                }

            return foundCurve;
            }

        case SolidPrimitiveType_DgnCone:
            {
            DgnConeDetail detail;

            if (!primitive.TryGetDgnConeDetail(detail))
                return false;

            DEllipse3d ellipse1;

            if (detail.FractionToSection(0.0, ellipse1))
                {
                ICurvePrimitivePtr  curve = ICurvePrimitive::CreateArc(ellipse1);

                if (ProcessICurvePrimitive(*curve, localPoint, HitGeomType::None, parentGeomType))
                    foundCurve = true;
                }

            DEllipse3d ellipse2;

            if (detail.FractionToSection(1.0, ellipse2))
                {
                ICurvePrimitivePtr  curve = ICurvePrimitive::CreateArc(ellipse2);

                if (ProcessICurvePrimitive(*curve, localPoint, HitGeomType::None, parentGeomType))
                    foundCurve = true;
                }

            divisorV *= 2;

            for (int vRule = 0; vRule < divisorV; ++vRule)
                {
                double      vFraction = (1.0 / divisorV) * vRule;
                DSegment3d  segment;

                if (!detail.FractionToRule(vFraction, segment))
                    continue;

                ICurvePrimitivePtr  curve = ICurvePrimitive::CreateLine(segment);

                if (ProcessICurvePrimitive(*curve, localPoint, HitGeomType::Surface, parentGeomType))
                    foundCurve = true;
                }
            
            DSegment3d  silhouette[2];
            DMatrix4d   viewToLocal = GetViewToLocal(worldToLocal);

            // NOTE: MicroStation's type 23 cone element always allowed it's silhouettes to be located/snapped to...continue doing so for historical reasons but with one caveat.
            //       The center of the cursor *must* be over the cone face to actually identify it and not just within locate tolerance. This behavior difference is because we
            //       no longer locate elements their silhouettes in PickContext (will work better when locating off mesh tiles)...
            if (detail.GetSilhouettes(silhouette[0], silhouette[1], viewToLocal))
                {
                for (int iSilhouette = 0; iSilhouette < 2; ++iSilhouette)
                    {
                    if (0.0 == silhouette[iSilhouette].Length())
                        continue;

                    ICurvePrimitivePtr  curve = ICurvePrimitive::CreateLine(silhouette[iSilhouette]);

                    if (ProcessICurvePrimitive(*curve, localPoint, HitGeomType::Surface, parentGeomType))
                        foundCurve = true;
                    }
                }

            return foundCurve;
            }

        case SolidPrimitiveType_DgnSphere:
            {
            DgnSphereDetail detail;

            if (!primitive.TryGetDgnSphereDetail(detail))
                return false;

            divisorU *= 2;

            for (int uRule = 0; uRule < divisorU; ++uRule)
                {
                double              uFraction = (1.0 / divisorU) * uRule;
                ICurvePrimitivePtr  curve = ICurvePrimitive::CreateArc(detail.UFractionToVSectionDEllipse3d(uFraction));

                if (ProcessICurvePrimitive(*curve, localPoint, HitGeomType::Surface, parentGeomType))
                    foundCurve = true;
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

                ICurvePrimitivePtr  curve = ICurvePrimitive::CreateArc(detail.VFractionToUSectionDEllipse3d(vFraction));

                if (ProcessICurvePrimitive(*curve, localPoint, HitGeomType::Surface, parentGeomType))
                    foundCurve = true;
                }

            return foundCurve;
            }

        case SolidPrimitiveType_DgnExtrusion:
            {
            DgnExtrusionDetail detail;

            if (!primitive.TryGetDgnExtrusionDetail(detail))
                return false;

            if (ProcessCurveVector(*detail.m_baseCurve, localPoint, HitGeomType::None, parentGeomType))
                foundCurve = true;

            CurveVectorPtr tmpCurve = detail.m_baseCurve->Clone(Transform::From(detail.m_extrusionVector));

            if (tmpCurve.IsValid() && ProcessCurveVector(*tmpCurve, localPoint, HitGeomType::None, parentGeomType))
                foundCurve = true;

            if (nullptr != m_stopTester && m_stopTester->_CheckStop())
                return false;

            bvector<DSegment3d> edges;

            WireframeGeomUtil::CollectLateralEdges(detail, edges);

            for (DSegment3d segment : edges)
                {
                ICurvePrimitivePtr  curve = ICurvePrimitive::CreateLine(segment);

                if (ProcessICurvePrimitive(*curve, localPoint, HitGeomType::None, parentGeomType))
                    foundCurve = true;

                if (nullptr != m_stopTester && m_stopTester->_CheckStop())
                    return false;
                }

            bvector<DPoint3d> pts;

            WireframeGeomUtil::CollectLateralRulePoints(*detail.m_baseCurve, pts, divisorU, divisorU * 2);

            for (DPoint3d pt : pts)
                {
                DSegment3d          segment = DSegment3d::FromOriginAndDirection(pt, detail.m_extrusionVector);
                ICurvePrimitivePtr  curve = ICurvePrimitive::CreateLine(segment);

                if (ProcessICurvePrimitive(*curve, localPoint, HitGeomType::Surface, parentGeomType))
                    foundCurve = true;

                if (nullptr != m_stopTester && m_stopTester->_CheckStop())
                    return false;
                }

            return foundCurve;
            }

        case SolidPrimitiveType_DgnRotationalSweep:
            {
            DgnRotationalSweepDetail detail;

            if (!primitive.TryGetDgnRotationalSweepDetail(detail))
                return false;

            if (!Angle::IsFullCircle(detail.m_sweepAngle))
                {
                if (ProcessCurveVector(*detail.m_baseCurve, localPoint, HitGeomType::None, parentGeomType))
                    foundCurve = true;

                DPoint3d  axisPoint;
                Transform transform;

                axisPoint.SumOf(detail.m_axisOfRotation.origin, detail.m_axisOfRotation.direction);
                transform.InitFromLineAndRotationAngle(detail.m_axisOfRotation.origin, axisPoint, detail.m_sweepAngle);

                CurveVectorPtr tmpCurve = detail.m_baseCurve->Clone(transform);

                if (tmpCurve.IsValid() && ProcessCurveVector(*tmpCurve, localPoint, HitGeomType::None, parentGeomType))
                    foundCurve = true;

                if (nullptr != m_stopTester && m_stopTester->_CheckStop())
                    return false;
                }

            bvector<DEllipse3d> edges;

            WireframeGeomUtil::CollectLateralEdges(detail, edges);

            for (DEllipse3d arc : edges)
                {
                ICurvePrimitivePtr  curve = ICurvePrimitive::CreateArc(arc);

                if (ProcessICurvePrimitive(*curve, localPoint, HitGeomType::None, parentGeomType))
                    foundCurve = true;

                if (nullptr != m_stopTester && m_stopTester->_CheckStop())
                    return false;
                }

            bvector<DPoint3d> pts;

            WireframeGeomUtil::CollectLateralRulePoints(*detail.m_baseCurve, pts, divisorU, 4); // Avoid clutter and just output every 90 degrees for closed face (i.e. rotational sweep is a torus)...

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

                if (!WireframeGeomUtil::ComputeRuleArc(ellipse, pts.at(iPt), detail.m_axisOfRotation.origin, detail.m_sweepAngle, transform, axes, invAxes, 1.0e-8))
                    continue;

                ICurvePrimitivePtr  curve = ICurvePrimitive::CreateArc(ellipse);

                if (ProcessICurvePrimitive(*curve, localPoint, HitGeomType::Surface, parentGeomType))
                    foundCurve = true;

                if (nullptr != m_stopTester && m_stopTester->_CheckStop())
                    return false;
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

                if (curve.IsValid() && ProcessCurveVector(*curve, localPoint, HitGeomType::Surface, parentGeomType))
                    foundCurve = true;

                if (nullptr != m_stopTester && m_stopTester->_CheckStop())
                    return false;
                }

            return foundCurve;
            }
            
        case SolidPrimitiveType_DgnRuledSweep:
            {
            DgnRuledSweepDetail detail;
    
            if (!primitive.TryGetDgnRuledSweepDetail(detail))
                return false;

            for (CurveVectorPtr curves : detail.m_sectionCurves)
                {
                if (ProcessCurveVector(*curves, localPoint, HitGeomType::None, parentGeomType))
                    foundCurve = true;
                }

            if (nullptr != m_stopTester && m_stopTester->_CheckStop())
                return false;

            bvector<DSegment3d> edges;

            WireframeGeomUtil::CollectLateralEdges(detail, edges);

            for (DSegment3d segment : edges)
                {
                ICurvePrimitivePtr  curve = ICurvePrimitive::CreateLine(segment);

                if (ProcessICurvePrimitive(*curve, localPoint, HitGeomType::None, parentGeomType))
                    foundCurve = true;

                if (nullptr != m_stopTester && m_stopTester->_CheckStop())
                    return false;
                }

            for (size_t iProfile = 0; iProfile < detail.m_sectionCurves.size()-1; ++iProfile)
                {
                bvector<DPoint3d> rulePts1;
                bvector<DPoint3d> rulePts2;

                WireframeGeomUtil::CollectLateralRulePoints(*detail.m_sectionCurves.at(iProfile), rulePts1, divisorU, divisorU * 2);
                WireframeGeomUtil::CollectLateralRulePoints(*detail.m_sectionCurves.at(iProfile+1), rulePts2, divisorU, divisorU * 2);

                if (rulePts1.size() != rulePts2.size())
                    {
                    if (1 == rulePts2.size()) // Special case to handle zero scale in both XY...
                        rulePts2.insert(rulePts2.end(), rulePts1.size()-1, rulePts2.front());
                    else
                        rulePts1.clear(); rulePts2.clear();;
                    }

                for (size_t iRule = 0; iRule < rulePts1.size(); ++iRule)
                    {
                    DSegment3d          segment = DSegment3d::From(rulePts1.at(iRule), rulePts2.at(iRule));
                    ICurvePrimitivePtr  curve = ICurvePrimitive::CreateLine(segment);

                    if (ProcessICurvePrimitive(*curve, localPoint, HitGeomType::Surface, parentGeomType))
                        foundCurve = true;

                    if (nullptr != m_stopTester && m_stopTester->_CheckStop())
                        return false;
                    }
                }

            return foundCurve;
            }

        default:
            {
            BeAssert(false);
            return false;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   05/18
+---------------+---------------+---------------+---------------+---------------+------*/
bool ProcessSolidPrimitive(ISolidPrimitiveCR primitive, DPoint3dCR localPoint, TransformCR worldToLocal)
    {
    HitParentGeomType parentGeomType = (primitive.GetCapped() ? HitParentGeomType::Solid : HitParentGeomType::Sheet);
    bvector<SolidLocationDetail::FaceIndices> faceIndices;

    primitive.GetFaceIndices(faceIndices);

    if (faceIndices.size() < 2)
        return ProcessSingleFaceSolidPrimitive(primitive, localPoint, worldToLocal, parentGeomType);

    SolidLocationDetail location;

    if (!primitive.ClosestPoint(localPoint, location))
        return false;

    IGeometryPtr faceGeom = primitive.GetFace(location.GetFaceIndices());

    if (!faceGeom.IsValid())
        return false;

    switch (faceGeom->GetGeometryType())
        {
        case IGeometry::GeometryType::CurveVector:
            {
            CurveVectorPtr faceCurves = faceGeom->GetAsCurveVector();

            return (faceCurves.IsValid() && ProcessCurveVector(*faceCurves, location.GetXYZ(), HitGeomType::None, parentGeomType));
            }

        case IGeometry::GeometryType::SolidPrimitive:
            {
            ISolidPrimitivePtr facePrimitive = faceGeom->GetAsISolidPrimitive();

            return (facePrimitive.IsValid() && ProcessSingleFaceSolidPrimitive(*facePrimitive, location.GetXYZ(), worldToLocal, parentGeomType));
            }

        case IGeometry::GeometryType::BsplineSurface:
            {
            MSBsplineSurfacePtr faceSurface = faceGeom->GetAsMSBsplineSurface();

            return (faceSurface.IsValid() && ProcessBsplineSurface(*faceSurface, location.GetXYZ(), parentGeomType));
            }

        default:
            {
            BeAssert(false);
            return false;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   05/18
+---------------+---------------+---------------+---------------+---------------+------*/
bool ProcessSingleFacePolyface(PolyfaceQueryCR meshData, DPoint3dCR localPoint)
    {
    int const*  vertIndex = meshData.GetPointIndexCP();

    if (!vertIndex)
        return false;

    size_t      numIndices = meshData.GetPointIndexCount();
    DPoint3dCP  verts = meshData.GetPointCP();
    int         polySize = meshData.GetNumPerFace();
    int         thisIndex, prevIndex=0, firstIndex=0;
    size_t      thisFaceSize = 0;
    bool        foundCurve = false;

    for (size_t readIndex = 0; readIndex < numIndices; readIndex++)
        {    
        // found face loop entry
        if (thisIndex = vertIndex[readIndex])
            {
            if (!thisFaceSize)
                {
                // remember first index in this face loop
                firstIndex = thisIndex;
                }
            else if (prevIndex > 0)
                {
                // draw visible edge (prevIndex, thisIndex)
                int closeVertexId = (abs(prevIndex) - 1);
                int segmentVertexId = (abs(thisIndex) - 1);
                ICurvePrimitivePtr curve = ICurvePrimitive::CreateLine(DSegment3d::From(verts[closeVertexId], verts[segmentVertexId]));

                if (ProcessICurvePrimitive(*curve, localPoint, HitGeomType::None, HitParentGeomType::Mesh))
                    foundCurve = true;
                }

            prevIndex = thisIndex;
            thisFaceSize++;
            }

        // found end of face loop (found first pad/terminator or last index in fixed block)
        if (thisFaceSize && (!thisIndex || (polySize > 1 && polySize == thisFaceSize)))
            {
            // draw last visible edge (prevIndex, firstIndex)
            if (prevIndex > 0)
                {
                int closeVertexId = (abs(prevIndex) - 1);
                int segmentVertexId = (abs(firstIndex) - 1);
                ICurvePrimitivePtr curve = ICurvePrimitive::CreateLine(DSegment3d::From(verts[closeVertexId], verts[segmentVertexId]));

                if (ProcessICurvePrimitive(*curve, localPoint, HitGeomType::None, HitParentGeomType::Mesh))
                    foundCurve = true;
                }

            thisFaceSize = 0;
            }

        if (0 == (readIndex % 100) && (nullptr != m_stopTester && m_stopTester->_CheckStop()))
            return false;
        }

    return foundCurve;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   05/18
+---------------+---------------+---------------+---------------+---------------+------*/
bool ProcessPolyface(PolyfaceQueryCR meshData, DPoint3dCR localPoint)
    {
    PolyfaceVisitorPtr  visitor = PolyfaceVisitor::Attach(meshData);
    double              tolerance = 1.0e-5;

    visitor->SetNumWrap(1);

    for (; visitor->AdvanceToNextFace();)
        {
        DPoint3d thisFacePoint;

        if (!visitor->TryFindCloseFacetPoint(localPoint, tolerance, thisFacePoint))
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
            return ProcessSingleFacePolyface(*perFacePolyfaces.front(), localPoint);
        break;
        }

    return false;
    }

#if defined (BENTLEYCONFIG_PARASOLID)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   05/18
+---------------+---------------+---------------+---------------+---------------+------*/
struct BRepFaceHatchProcessor : IParasolidWireOutput
    {
    SnapGeometryHelper&     m_processor;
    DPoint3d                m_localPoint;
    Transform               m_entityTransform;
    HitParentGeomType       m_parentGeomType;
    bool                    m_foundCurve = false;

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    BrienBastings   11/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    BentleyStatus _ProcessGoOutput(ICurvePrimitiveCR hatchCurve, PK_ENTITY_t entityTag)
        {
        CurveVectorPtr curve = CurveVector::Create(CurveVector::BoundaryType::BOUNDARY_TYPE_Open, hatchCurve.Clone());

        curve->TransformInPlace(m_entityTransform);

        if (m_processor.ProcessCurveVector(*curve, m_localPoint, HitGeomType::Surface, m_parentGeomType))
            m_foundCurve = true;

        return (nullptr != m_processor.m_stopTester && m_processor.m_stopTester->_CheckStop() ? ERROR : SUCCESS);
        }

    bool GetFoundCurve() {return m_foundCurve;}

    BRepFaceHatchProcessor(SnapGeometryHelper& processor, DPoint3dCR localPoint, TransformCR entityTransform, HitParentGeomType parentGeomType) : m_processor(processor), m_localPoint(localPoint), m_entityTransform(entityTransform), m_parentGeomType(parentGeomType) {}

    }; // BRepFaceHatchProcessor

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   05/18
+---------------+---------------+---------------+---------------+---------------+------*/
bool ProcessBody(IBRepEntityCR entity, DPoint3dCR localPoint)
    {
    ISubEntityPtr closeEntity = BRepUtil::ClosestSubEntity(entity, localPoint);

    if (!closeEntity.IsValid())
        return false;

    HitParentGeomType parentGeomType = HitParentGeomType::None;

    switch (entity.GetEntityType())
        {
        case IBRepEntity::EntityType::Wire:
            parentGeomType = HitParentGeomType::Wire;
            break;

        case IBRepEntity::EntityType::Sheet:
            parentGeomType = HitParentGeomType::Sheet;
            break;

        case IBRepEntity::EntityType::Solid:
            parentGeomType = HitParentGeomType::Solid;
            break;
        } 

    switch (closeEntity->GetSubEntityType())
        {
        case ISubEntity::SubEntityType::Edge:
        case ISubEntity::SubEntityType::Vertex:
            {
            GeometricPrimitiveCPtr geom = closeEntity->GetGeometry();

            if (geom.IsValid() && GeometricPrimitive::GeometryType::CurvePrimitive == geom->GetGeometryType())
                {
                ICurvePrimitivePtr curve = geom->GetAsICurvePrimitive();

                return ProcessICurvePrimitive(*curve, localPoint, HitGeomType::None, parentGeomType);
                }

            return false;
            }

        case ISubEntity::SubEntityType::Face:
            {
            CurveVectorPtr faceCurves = BRepUtil::Create::PlanarFaceToCurveVector(*closeEntity);

            if (faceCurves.IsValid())
                return ProcessCurveVector(*faceCurves, localPoint, HitGeomType::None, parentGeomType);

            break;
            }
        }

    BRepFaceHatchProcessor output(*this, localPoint, entity.GetEntityTransform(), parentGeomType);

    PSolidGoOutput::ProcessFaceHatching(output, m_snapDivisor, PSolidSubEntity::GetSubEntityTag(*closeEntity));

    if (nullptr != m_stopTester && m_stopTester->_CheckStop())
        return false;

    bool foundCurve = output.GetFoundCurve();

    bvector<ISubEntityPtr> faceEdges;

    if (SUCCESS != BRepUtil::GetFaceEdges(faceEdges, *closeEntity))
        return foundCurve;

    for (ISubEntityPtr& edge : faceEdges)
        {
        if (nullptr != m_stopTester && m_stopTester->_CheckStop())
            return false;

        GeometricPrimitiveCPtr geom = edge->GetGeometry();

        if (!geom.IsValid() || GeometricPrimitive::GeometryType::CurvePrimitive != geom->GetGeometryType())
            continue;

        ICurvePrimitivePtr curve = geom->GetAsICurvePrimitive();

        if (ProcessICurvePrimitive(*curve, localPoint, HitGeomType::None, parentGeomType))
            foundCurve = true;
        }

    return foundCurve;
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   05/18
+---------------+---------------+---------------+---------------+---------------+------*/
bool ProcessTextString(TextStringCR text, DPoint3dCR localPoint)
    {
    if (text.GetText().empty())
        return false;
        
    DPoint3d points[5];

    text.ComputeBoundingShape(points);
    text.ComputeTransform().Multiply(points, _countof(points));

    ICurvePrimitivePtr  curve = ICurvePrimitive::CreateLineString(points, 5);

    return ProcessICurvePrimitive(*curve, localPoint, HitGeomType::None, HitParentGeomType::Text);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   05/18
+---------------+---------------+---------------+---------------+---------------+------*/
bool ProcessGeometry(GeometricPrimitiveR geom, TransformCR localToWorld, bool checkRange)
    {
    Transform   worldToLocal;

    worldToLocal.InverseOf(localToWorld);

    DPoint3d    localPoint = GetClosePointLocal(worldToLocal);

    if (checkRange)
        {
        double      maxOutsideDist = (1.0e-2 * (1.0 + localPoint.Magnitude()));
        DRange3d    localRange;

        if (geom.GetRange(localRange) && localRange.DistanceOutside(localPoint) > maxOutsideDist)
            return false;
        }

    switch (geom.GetGeometryType())
        {
        case GeometricPrimitive::GeometryType::CurvePrimitive:
            {
            ICurvePrimitiveCR curve = *geom.GetAsICurvePrimitive();

            if (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_PointString == curve.GetCurvePrimitiveType())
                return ProcessPointString(curve, localPoint);

            return ProcessICurvePrimitive(curve, localPoint, HitGeomType::None, HitParentGeomType::Wire);
            }

        case GeometricPrimitive::GeometryType::CurveVector:
            {
            CurveVectorCR curves = *geom.GetAsCurveVector();

            return ProcessCurveVector(curves, localPoint, HitGeomType::None, curves.IsAnyRegionType() ? HitParentGeomType::Sheet : HitParentGeomType::Wire);
            }

        case GeometricPrimitive::GeometryType::SolidPrimitive:
            {
            ISolidPrimitiveCR primitive = *geom.GetAsISolidPrimitive();

            return ProcessSolidPrimitive(primitive, localPoint, worldToLocal);
            }

        case GeometricPrimitive::GeometryType::BsplineSurface:
            {
            MSBsplineSurfaceCR surface = *geom.GetAsMSBsplineSurface();

            return ProcessBsplineSurface(surface, localPoint, HitParentGeomType::Sheet);
            }

        case GeometricPrimitive::GeometryType::Polyface:
            {
            PolyfaceQueryCR mesh = *geom.GetAsPolyfaceHeader();

            return ProcessPolyface(mesh, localPoint);
            }

#if defined (BENTLEYCONFIG_PARASOLID)
        case GeometricPrimitive::GeometryType::BRepEntity:
            {
            IBRepEntityCR entity = *geom.GetAsIBRepEntity();

            return ProcessBody(entity, localPoint);
            }
#endif

        case GeometricPrimitive::GeometryType::TextString:
            {
            TextStringCR text = *geom.GetAsTextString();

            return ProcessTextString(text, localPoint);
            }

        default:
            {
            BeAssert(false);
            return false;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   05/18
+---------------+---------------+---------------+---------------+---------------+------*/
bool ProcessEntry(GeometryCollection::Iterator const& iter, bool preFiltered, DgnElementCP element)
    {
    GeometryStreamEntryId elemEntryId = iter.GetGeometryStreamEntryId();
    GeometricPrimitivePtr geom;

    if (nullptr != element && GeometryCollection::Iterator::EntryType::BRepEntity == iter.GetEntryType())
        {
        IBRepEntityPtr entity = BRepDataCache::FindCachedBRepEntity(*element, elemEntryId);

        if (entity.IsValid())
            {
            geom = GeometricPrimitive::Create(entity);
            }
        else
            {
            geom = iter.GetGeometryPtr();

            if (!geom.IsValid())
                return false;

            // Populate brep cache when not using PickContext and calling GeometryStreamIO::Collection::Draw...
            BRepDataCache::AddCachedBRepEntity(*element, elemEntryId, *geom->GetAsIBRepEntity());
            }
        }

    if (!geom.IsValid())
        {
        geom = iter.GetGeometryPtr();

        if (!geom.IsValid())
            return false;
        }

    Transform localToWorld = iter.GetGeometryToWorld();

    if (!ProcessGeometry(*geom, localToWorld, !preFiltered))
        return false; 

    m_hitGeom = geom;
    m_hitParams = iter.GetGeometryParams();
    m_hitEntryId = elemEntryId;
    m_hitLocalToWorld = localToWorld;

    OnHitChanged();

    if (!preFiltered)
        {
        // If we found an edge within locate tolerance, it's good enough and we can stop slogging through the geometry stream...
        if (m_hitDistanceView < m_snapAperture)
            return true;

        return false; // Keep going...
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   05/18
+---------------+---------------+---------------+---------------+---------------+------*/
bool SkipEntry(GeometryCollection::Iterator const& iter, GeometryStreamEntryIdCR snapElemEntryId, bool isPart, ViewFlagsCP viewFlags, DgnElementIdSet const* offSubCategories)
    {
    GeometryStreamEntryId elemEntryId = iter.GetGeometryStreamEntryId();

    if (snapElemEntryId.IsValid())
        return (snapElemEntryId != elemEntryId);

    if (nullptr != viewFlags || nullptr != offSubCategories)
        {
        GeometryParamsCR params = iter.GetGeometryParams();

        if (nullptr != viewFlags)
            {
            switch (params.GetGeometryClass())
                {
                case DgnGeometryClass::Construction:
                    if (!viewFlags->ShowConstructions())
                        return true;
                    break;

                case DgnGeometryClass::Dimension:
                    if (!viewFlags->ShowDimensions())
                        return true;
                    break;

                case DgnGeometryClass::Pattern:
                    if (!viewFlags->ShowPatterns())
                        return true;
                    break;
                }
            }

        if (nullptr != offSubCategories)
            {
            if (offSubCategories->end() != offSubCategories->find(params.GetSubCategoryId()))
                return true;
            }
        }

    DRange3d    localRange = iter.GetSubGraphicLocalRange();

    if (localRange.IsNull() && (!isPart || SUCCESS != DgnGeometryPart::QueryGeometryPartRange(localRange, iter.GetDgnDb(), iter.GetGeometryPartId())))
        return false;

    Transform   localToWorld = iter.GetGeometryToWorld();
    Transform   worldToLocal;

    worldToLocal.InverseOf(localToWorld);

    DPoint3d    localPoint = GetClosePointLocal(worldToLocal);

    return (localRange.DistanceOutside(localPoint) > 1.0e-5);
    }

public:

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   05/18
+---------------+---------------+---------------+---------------+---------------+------*/
SnapMode GetSnapMode() const {return m_snapMode;}
int GetSnapDivisor() const {return m_snapDivisor;}
double GetSnapAperture() const {return  m_snapAperture;}
DPoint3dCR GetClosePointWorld() const {return m_closePtWorld;}
DMap4dCR GetWorldToView() const {return m_worldToView;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   05/18
+---------------+---------------+---------------+---------------+---------------+------*/
void SetHitGeometryStreamEntryId(GeometryStreamEntryIdCR entryId) {m_hitEntryId = entryId;}
void SetHitCurveDetail(CurveLocationDetailCR detail) {m_hitCurveDetail = detail; OnHitChanged();}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   05/18
+---------------+---------------+---------------+---------------+---------------+------*/
void OnHitChanged()
    {
    m_hitWorldToLocal.InverseOf(m_hitLocalToWorld);
    m_hitClosePtLocal = GetClosePointLocal(m_hitWorldToLocal);

    DMatrix4d viewToLocal = GetViewToLocal(m_hitWorldToLocal);
    DMatrix4d localToView;

    localToView.QrInverseOf(viewToLocal);
    m_hitLocalToView.InitFrom(localToView, viewToLocal);

    DPoint3d localPts[2];
    DPoint4d viewPts[2];

    localPts[0] = m_hitClosePtLocal;
    localPts[1] = m_hitCurveDetail.point;

    m_hitLocalToView.M0.Multiply(viewPts, localPts, nullptr, 2);
    m_hitDistanceView = viewPts[0].RealDistance(viewPts[1]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   05/18
+---------------+---------------+---------------+---------------+---------------+------*/
GeometricPrimitivePtr GetHitGeometry() const {return m_hitGeom;}
TransformCR GetHitLocalToWorld() const {return m_hitLocalToWorld;}
GeometryParamsCR GetHitGeometryParams() const {return m_hitParams;}
GeometryStreamEntryIdCR GetHitGeometryStreamEntryId() const {return m_hitEntryId;}
CurveLocationDetailCR GetHitCurveDetail() const {return m_hitCurveDetail;}
ICurvePrimitivePtr& GetHitCurvePrimitivePtr() {return m_hitCurveDerived;} // Valid when m_hitCurveDetail.curve is not nullptr...
HitGeomType GetHitGeomType() const {return m_hitGeomType;}
HitParentGeomType GetHitParentGeomType() const {return m_hitParentGeomType;}
DPoint3d GetHitPointWorld() const {DPoint3d hitPtWorld = m_hitCurveDetail.point; m_hitLocalToWorld.Multiply(hitPtWorld); return hitPtWorld;}
bool IsHitNormalValid() const {return 0.0 != m_hitNormalLocal.Magnitude();}
DVec3d GetHitNormalWorld() const {DVec3d hitNormalWorld = m_hitNormalLocal; m_hitLocalToWorld.MultiplyMatrixOnly(hitNormalWorld); hitNormalWorld.Normalize(); return hitNormalWorld;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/12
+---------------+---------------+---------------+---------------+---------------+------*/
static bool IsPhysicallyClosed(ICurvePrimitiveCR primitive)
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
static bool GetAreaCentroid(DPoint3dR centroid, CurveVectorCR curve)
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
static bool GetCentroid(DPoint3dR centroid, ICurvePrimitiveCR primitive)
    {
    if (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc == primitive.GetCurvePrimitiveType())
        {
        // Center snap always uses arc center...
        centroid = primitive.GetArcCP ()->center;

        return true;
        }
    else if (IsPhysicallyClosed(primitive))
        {
        CurveVectorPtr  curve = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Outer);
        
        curve->push_back(primitive.Clone());

        // For physically closed/planar curve use area centroid instead of wire centroid...
        if (GetAreaCentroid(centroid, *curve))
            return true;
        }

    double  length;

    return primitive.WireCentroid(length, centroid);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/12
+---------------+---------------+---------------+---------------+---------------+------*/
static SnapHeat GetHeat(DPoint3dCR snapPoint, DPoint3dCR closePoint, DMatrix4dCR worldToView, double aperture, bool forceHot)
    {
    DPoint4d viewPts[2];
    worldToView.Multiply(&viewPts[0], &snapPoint, nullptr, 1);
    worldToView.Multiply(&viewPts[1], &closePoint, nullptr, 1);

    double viewDist = 0.0;
    if (!viewPts[0].RealDistanceXY(viewDist, viewPts[1]))
        return SNAP_HEAT_None;

    bool withinAperture = (viewDist <= aperture);
    return (withinAperture ? SNAP_HEAT_InRange : (forceHot ? SNAP_HEAT_NotInRange : SNAP_HEAT_None));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   05/18
+---------------+---------------+---------------+---------------+---------------+------*/
bool ComputeSnapLocation()
    {
    if (!EvaluateCurve())
        return false;

    EvaluateInterior();
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   05/18
+---------------+---------------+---------------+---------------+---------------+------*/
SnapStatus GetClosestCurve(GeometrySourceCR source, ViewFlagsCP viewFlags = nullptr, DgnElementIdSet const* offSubCategories = nullptr, CheckStop* stopTester = nullptr)
    {
    // NEEDSWORK: For imodel-js...
    //  DgnSubCategory::Appearance.GetDontLocate is a problem, geometry will get located and used by tools that aren't snapping...remove from imodel-js api...
    //  ViewController overrides for DgnSubCategory::Appearance.GetDontSnap is a problem...return hit subcategory to front end to let it check?
    //  Can we ignore snappable patterns/linestyles? These will locate currently, snap will just go to "base" geometry...
    GeometryCollection     collection(source);
    GeometryStreamEntryId  snapElemEntryId = m_hitEntryId;
    GeometryStreamEntryId  snapPartEntryId = m_hitEntryId;

    snapElemEntryId.SetPartIndex(0);
    m_stopTester = stopTester;

    for (auto iter : collection)
        {
        // Quick exclude of geometry that didn't generate the hit...
        if (SkipEntry(iter, snapElemEntryId, false, viewFlags, offSubCategories))
            continue;

        if (GeometryCollection::Iterator::EntryType::GeometryPart != iter.GetEntryType())
            {
            if (ProcessEntry(iter, snapElemEntryId.IsValid(), source.ToElement()))
                break;

            if (nullptr != m_stopTester && m_stopTester->_CheckStop())
                return SnapStatus::Aborted;

            continue;
            }

        DgnGeometryPartCPtr geomPart = iter.GetGeometryPartCPtr();

        if (!geomPart.IsValid())
            continue; // Shouldn't happen...

        GeometryCollection partCollection(geomPart->GetGeometryStream(), geomPart->GetDgnDb());

        partCollection.SetNestedIteratorContext(iter); // Iterate part GeomStream in context of parent...

        for (auto partIter : partCollection)
            {
            // Quick exclude of geometry that didn't generate the hit...
            if (SkipEntry(partIter, snapPartEntryId, true, viewFlags, offSubCategories))
                continue;

            if (ProcessEntry(partIter, snapPartEntryId.IsValid(), geomPart.get()))
                break;

            if (nullptr != m_stopTester && m_stopTester->_CheckStop())
                return SnapStatus::Aborted;
            }

        if (snapPartEntryId.IsValid())
            break; // Done with part...
        }

    SimplifyHitDetail();

    return (nullptr != m_hitCurveDetail.curve || HitGeomType::Point == m_hitGeomType ? SnapStatus::Success : SnapStatus::NoSnapPossible);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   05/18
+---------------+---------------+---------------+---------------+---------------+------*/
SnapGeometryHelper(SnapMode mode, int divisor, double aperture, DPoint3d closePtWorld, DMap4d worldToView) : m_snapMode(mode), m_snapDivisor(divisor), m_snapAperture(aperture), m_closePtWorld(closePtWorld), m_worldToView(worldToView) {}

}; // SnapGeometryHelper
END_BENTLEY_DGN_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/18
+---------------+---------------+---------------+---------------+---------------+------*/
SnapContext::Response SnapContext::DoSnap(SnapContext::Request const& input, DgnDbR db, struct CheckStop& checkstop)
    {
    SnapContext::Response output;
    output.SetStatus(SnapStatus::BadArg);

    // may have been aborted while it was in the queue. If so, don't even start
    if (checkstop.WasAborted() || !input.IsValid())
        return output;

    output.SetStatus(SnapStatus::NoElements);
    DgnElementId elementId = input.GetElementId();
    if (!elementId.IsValid())
        return output; // NOTE: Maybe to support snapable decorations the GeometryStream/Placement can be supplied in lieu of an element id?

    DgnElementCPtr element = db.Elements().GetElement(elementId);
    GeometrySourceCP source = element.IsValid() ? element->ToGeometrySource() : nullptr;

    if (nullptr == source)
        return output;

    DPoint3d closePoint = input.GetClosePoint();

    DMatrix4d worldToView = input.GetWorldToView();
    DMatrix4d viewToWorld;
    viewToWorld.QrInverseOf(worldToView);
    DMap4d worldToViewMap;
    worldToViewMap.InitFrom(worldToView, viewToWorld);

    ViewFlags viewFlags = input.GetViewFlags();
    SnapMode snapMode = input.GetSnapMode();

    // Hot distance in view coordinates (pixels). Locate aperture * hot distance factor...
    double snapAperture = input.GetSnapAperture();
    int snapDivisor = input.GetSnapDivisor();
    
    output.SetStatus(SnapStatus::Success);

    if (SnapMode::Origin != snapMode)
        {
        DgnElementIdSet offSubCategories;
        auto& subcat = input.GetOffSubCategories();

        if (!subcat.isNull() && subcat.isArray())
            {
            uint32_t nEntries = (uint32_t) subcat.size();
            for (uint32_t i=0; i < nEntries; i++)
                {
                DgnSubCategoryId subCategoryId;
                subCategoryId.FromJson(subcat[i]);
                offSubCategories.insert(subCategoryId);
                }
            }

        SnapGeometryHelper helper(snapMode, snapDivisor, snapAperture, closePoint, worldToViewMap);

        SnapStatus status = helper.GetClosestCurve(*source, &viewFlags, offSubCategories.empty() ? nullptr : &offSubCategories, &checkstop);
        if (SnapStatus::Aborted == status)
            {
            output.SetStatus(SnapStatus::Aborted);
            return output;
            }

        if (SnapStatus::Success == status && helper.ComputeSnapLocation())
            {
            DPoint3d snapPoint = helper.GetHitPointWorld(); 
            output.SetSnapPoint(snapPoint);
            output.SetHeat(SnapGeometryHelper::GetHeat(snapPoint, closePoint, worldToViewMap.M0, snapAperture, SnapMode::Center == snapMode));
            output.SetGeomType(helper.GetHitGeomType());
            output.SetParentGeomType(helper.GetHitParentGeomType());
            output.SetSubCategory(helper.GetHitGeometryParams().GetSubCategoryId().ToHexStr());

            if (!helper.GetHitGeometryParams().IsWeightFromSubCategoryAppearance())
                output.SetWeight(helper.GetHitGeometryParams().GetWeight());

            if (helper.IsHitNormalValid())
                output.SetNormal(helper.GetHitNormalWorld());

            if (nullptr != helper.GetHitCurveDetail().curve)
                {
                Json::Value  geomValue;
                IGeometryPtr geomPtr = IGeometry::Create(helper.GetHitCurvePrimitivePtr());

                if (geomPtr.IsValid() && IModelJson::TryGeometryToIModelJsonValue(geomValue, *geomPtr))
                    {
                    output.SetCurve(geomValue);

                    if (!helper.GetHitLocalToWorld().IsIdentity())
                        output.SetLocalToWorld(helper.GetHitLocalToWorld());
                    }
                }

            return output;
            }
        }

    DPoint3d snapPoint;
    source->GetPlacementTransform().GetTranslation(snapPoint);
    output.SetSnapPoint(snapPoint);
    output.SetHeat(SnapGeometryHelper::GetHeat(snapPoint, closePoint, worldToViewMap.M0, snapAperture, SnapMode::Center == snapMode));

    return output;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/06
+---------------+---------------+---------------+---------------+---------------+------*/
SnapStatus SnapContext::DoDefaultDisplayableSnap()
    {
    GeomDetailCR    detail = GetSnapDetail()->GetGeomDetail();

    // Don't require a curve primitive if hit geom is point or mode is nearest because current hit point is correct...
    if (SnapMode::Nearest == GetSnapMode() || HitGeomType::Point == detail.GetGeomType())
        {
        DPoint3d    hitPoint = GetSnapDetail()->GetHitPoint();

        SetSnapInfo(*GetSnapDetail(), GetSnapMode(), GetSnapSprite(GetSnapMode()), hitPoint, false, GetSnapAperture(), false);

        return SnapStatus::Success;
        }

    if (nullptr == detail.GetCurvePrimitive())
        {
        // Surface w/o curve is interior hit...only nearest should "track" surface...
        if (HitGeomType::Surface == detail.GetGeomType())
            {
            bool             usePlacement = true;
            DgnElementCPtr   element = GetSnapDetail()->GetElement();
            GeometrySourceCP source = (element.IsValid() ? element->ToGeometrySource() : nullptr);

            if (nullptr == source)
                {
                IElemTopologyCP elemTopo = GetSnapDetail()->GetElemTopology();

                if (nullptr == (source = (nullptr != elemTopo ? elemTopo->_ToGeometrySource() : nullptr)))
                    return SnapStatus::NotSnappable;

                // NOTE: Assume placement is not meaningful for non-element hits when it's identity...
                usePlacement = !source->GetPlacementTransform().IsIdentity();
                }

            if (SnapMode::Origin != GetSnapMode() || !usePlacement) // Snap to geometry origin for non-element hits without a meaningful placement...
                {
                SnapGeometryHelper helper(GetSnapMode(), GetSnapDivisor(), GetSnapAperture(), GetSnapDetail()->GetGeomDetail().GetClosestPoint(), GetWorldToView());

                helper.SetHitGeometryStreamEntryId(GetSnapDetail()->GetGeomDetail().GetGeometryStreamEntryId(true)); // Supply HitDetail entry id so that we can trivially reject GeometryStream entries...

                // NOTE: This can be fairly expensive...but we need something more meaningful than snapping to center of range...
                SnapStatus status = helper.GetClosestCurve(*source, &m_viewflags, nullptr, this);

                if (SnapStatus::Aborted == status)
                    return status;

                if (SnapStatus::Success == status && helper.ComputeSnapLocation())
                    {
                    GetSnapDetail()->GetGeomDetailW().SetCurvePrimitive(helper.GetHitCurveDetail().curve, &helper.GetHitLocalToWorld(), helper.GetHitGeomType());
                    GetSnapDetail()->GetGeomDetailW().SetSurfaceNormal(helper.IsHitNormalValid() ? helper.GetHitNormalWorld() : DVec3d::FromZero()); // Update or clear surface normal for snap location...

                    SetSnapInfo(*GetSnapDetail(), GetSnapMode(), GetSnapSprite(GetSnapMode()), helper.GetHitPointWorld(), SnapMode::Center == GetSnapMode(), GetSnapAperture(), false);

                    return status;
                    }

                if (!usePlacement)
                    return SnapStatus::NotSnappable;
                }

            DPoint3d hitPoint;

            source->GetPlacementTransform().GetTranslation(hitPoint);
            SetSnapInfo(*GetSnapDetail(), GetSnapMode(), GetSnapSprite(GetSnapMode()), hitPoint, false, GetSnapAperture(), false);

            return SnapStatus::Success;
            }

        return SnapStatus::NotSnappable;
        }

    return DoSnapUsingCurve(GetSnapMode());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/12
+---------------+---------------+---------------+---------------+---------------+------*/
SnapStatus SnapContext::DoSnapUsingCurve(SnapMode snapMode)
    {
    CurveLocationDetail detail;

    if (!GetSnapDetail()->GetGeomDetail().GetCloseDetail(detail))
        return SnapStatus::NotSnappable;

    SnapGeometryHelper helper(snapMode, GetSnapDivisor(), GetSnapAperture(), GetSnapDetail()->GetGeomDetail().GetClosestPoint(), GetWorldToView());

    helper.SetHitCurveDetail(detail);

    if (!helper.ComputeSnapLocation())
        return SnapStatus::NotSnappable;

    SetSnapInfo(*GetSnapDetail(), snapMode, GetSnapSprite(snapMode), helper.GetHitPointWorld(), SnapMode::Center == snapMode, GetSnapAperture(), false);

    return SnapStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/04
+---------------+---------------+---------------+---------------+---------------+------*/
SnapStatus SnapContext::DoTextSnap()
    {
    GeomDetailCR detail = GetSnapDetail()->GetGeomDetail();

    if (HitParentGeomType::Text != detail.GetParentGeomType())
        return DoDefaultDisplayableSnap(); // Use normal snap logic for a hit to leader/capsule geometry...

    switch (GetSnapMode())
        {
        case SnapMode::Origin:
        case SnapMode::NearestKeypoint:
            {
            DgnElementCPtr   element = GetSnapDetail()->GetElement();
            GeometrySourceCP source = (element.IsValid() ? element->ToGeometrySource() : nullptr);

            if (nullptr == source)
                return SnapStatus::NotSnappable;

            DPoint3d hitPoint = (nullptr != source->GetAsGeometrySource3d() ? source->GetAsGeometrySource3d()->GetPlacement().GetOrigin() : DPoint3d::From(source->GetAsGeometrySource2d()->GetPlacement().GetOrigin()));
            
            SetSnapInfo(*GetSnapDetail(), GetSnapMode(), GetSnapSprite(GetSnapMode()), hitPoint, false, GetSnapAperture(), false);

            return SnapStatus::Success;
            }

        case SnapMode::MidPoint:
        case SnapMode::Bisector:
        case SnapMode::Center:
            {
            if (nullptr == detail.GetCurvePrimitive())
                return DoDefaultDisplayableSnap(); // NOTE: Boundary shape unavailable for origin only...always have for edge/interior hit...

            DPoint3d    centroid;

            if (!SnapGeometryHelper::GetCentroid(centroid, *detail.GetCurvePrimitive()))
                return SnapStatus::NotSnappable;

            SetSnapInfo(*GetSnapDetail(), GetSnapMode(), GetSnapSprite(GetSnapMode()), centroid, false, GetSnapAperture(), false);

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
            SetSnapInfo(*GetSnapDetail(), GetSnapMode(), GetSnapSprite(GetSnapMode()), hitPoint, false, GetSnapAperture(), false);

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
* @bsimethod                                                    Keith.Bentley   05/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool SnapContext::GetParameterKeypoint(ICurvePrimitiveCR curve, DPoint3dR hitPoint, double& keyParam, int divisor)
    {
    double  length, subLength;

    if (!curve.Length(length) || !curve.SignedDistanceBetweenFractions(0.0, keyParam, subLength))
        return false;

    // NOTE: For a closed curve we want quadrant key points.
    if (SnapGeometryHelper::IsPhysicallyClosed(curve))
        divisor *= 4;

    int     keySeg = int ((subLength / length) * (double) divisor + 0.5);
    double  keyDist = (keySeg / (double) divisor) * length;

    CurveLocationDetail location;

    if (!curve.PointAtSignedDistanceFromFraction(0.0, keyDist, false, location))
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/04
+---------------+---------------+---------------+---------------+---------------+------*/
void SnapContext::SetSnapInfo(SnapDetailR snap, SnapMode snapMode, ISpriteP sprite, DPoint3dCR snapPoint, bool forceHot, double aperture, bool isAdjusted, int nBytes, Byte* customKeypointData)
    {
    snap.SetSnapMode(snapMode);
    snap.SetSprite(sprite);

    snap.SetSnapPoint(snapPoint, forceHot, aperture);
    snap.SetAllowAssociations(!isAdjusted);

    if (nBytes && customKeypointData)
        snap.SetCustomKeypoint(nBytes, customKeypointData);
    }

