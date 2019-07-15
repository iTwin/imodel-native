/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
// This h file is to be included in only one c file (sp_curveIntersection.cpp)
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void DgnConeDetail::AddRayIntersections
(
bvector<SolidLocationDetail> &pickData,
DRay3dCR ray,
int parentId,
double minParameter
) const
    {
    static bool s_safeDivide = true;
    static bool s_transformSelect = true;
    Transform localToWorld, worldToLocal;
    double radiusA, radiusB;
    size_t baseSize = pickData.size ();
    if (GetTransforms (localToWorld, worldToLocal, radiusA, radiusB, s_transformSelect))
        {
        DRay3d localRay;
        worldToLocal.Multiply (localRay, ray);
        // Infinite cone intersections:
        // X^2 + Y^2 = R^2
        // X,Y are parameterized along ray.
        // R varies linearly with local Z, which is parameterized along ray.
        // (orgin.x + s*direction.x)^2 + (origin.y + s*direction.y)^2
        //     =  (rA + (origin.z + s*direction.z)*(rB-rA))^2
        // Quadratic in s.
        Polynomial::Power::Degree2 q = Polynomial::Power::Degree2 ();
        q.AddSquaredLinearTerm (localRay.origin.x, localRay.direction.x);
        q.AddSquaredLinearTerm (localRay.origin.y, localRay.direction.y);
        double dr = radiusB - radiusA;
        q.AddSquaredLinearTerm (radiusA + dr * localRay.origin.z,
                                dr * localRay.direction.z, -1.0);
        double ss[2];
        //int n1 = q.RealRoots (ss);
        //double ss1[2];
        int n;
        if (s_safeDivide)
            n = q.RealRootsWithSafeDivideCheck (ss);
        else
            n = q.RealRoots (ss);

        //assert (n == n1);
        for (int i = 0; i < n; i++)
            {
            if (ss[i] > minParameter)
                {
                DPoint3d localPiercePoint = localRay.FractionParameterToPoint (ss[i]);
                if (localPiercePoint.z >= 0.0
                    && localPiercePoint.z <= 1.0)
                    {
                    SolidLocationDetail detail (parentId, ss[i], ray.FractionParameterToPoint (ss[i]));
                    detail.SetFaceIndices (0,0, 0);
                    DgnConeDetail::SetDetailCoordinatesFromFractionalizedConeCoordinates
                            (detail, localPiercePoint, localToWorld, radiusA, radiusB);
                    pickData.push_back (detail);
                    }
                }
            }

        if (m_capped)
            {
            double zz [2] = {0.0, 1.0};
            double rr [2] = {radiusA, radiusB};
            for (int i = 0; i < 2; i++)
                {
                double pierceFraction;
                if (DoubleOps::SafeDivideParameter (pierceFraction,
                            zz[i] - localRay.origin.z,
                            localRay.direction.z, 0.0)
                    && pierceFraction > minParameter)
                    {
                    DPoint3d localPiercePoint = localRay.FractionParameterToPoint (pierceFraction);
                    if (localPiercePoint.MagnitudeSquaredXY () <= rr[i] * rr[i])
                        {
                        SolidLocationDetail detail (parentId, pierceFraction, ray.FractionParameterToPoint (pierceFraction));
                        detail.SetCapSelector (i);
                        double uFraction, vFraction;
                        double da = 2.0 * rr[i];
                        FractionalizeInCircle (localPiercePoint.x, localPiercePoint.y, rr[i], uFraction, vFraction);
                        DVec3d uVector, vVector;
                        localToWorld.GetMatrixColumn (uVector, 0); // unit !!
                        localToWorld.GetMatrixColumn (vVector, 1); // unit !!
                        uVector.Scale (da);
                        vVector.Scale (da);
                        detail.SetUV (uFraction, vFraction, uVector, vVector);
                        pickData.push_back (detail);
                        }
                    }
                }
            }
        }
    FilterMinParameter (pickData, baseSize, minParameter);
    SortTail (pickData, baseSize);
    }



/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void DgnTorusPipeDetail::AddRayIntersections
(
bvector<SolidLocationDetail> &pickData,
DRay3dCR ray,
int parentId,
double minParameter
) const
    {
    size_t baseSize = pickData.size ();
    DPoint3d center;
    RotMatrix axes;
    Transform localToWorld, worldToLocal;
    double radiusA, radiusB;
    double sweepRadians;
    if (TryGetFrame (center, axes, radiusA, radiusB, sweepRadians))
        {
        localToWorld = Transform::From (axes, center);
        worldToLocal.InverseOf (localToWorld);
        Polynomial::Implicit::Torus torus (radiusA, radiusB, GetReverseVector90 ());
        DRay3d localRay;
        worldToLocal.Multiply (localRay, ray);
        double rayFraction[5];
        DPoint3d localXYZ[5];
        DPoint3d worldRayPoint;
        int numHit = torus.IntersectRay (localRay, rayFraction, localXYZ, 5);
        for (int i = 0; i < numHit; i++)
            {
            double theta, phi, majorCircleDistance;
            DPoint3d uvw = localXYZ[i];
            double rho;
            torus.XYZToThetaPhiDistance (uvw, theta, phi, majorCircleDistance, rho);
            double vFraction = Angle::NormalizeToSweep (theta, 0.0, m_sweepAngle);
            double uFraction = Angle::NormalizeToSweep (phi, 0.0, Angle::TwoPi ());
            if (DoubleOps::IsIn01 (vFraction))
                {
                double s = rayFraction[i];
                worldRayPoint = ray.FractionParameterToPoint(s);
                pickData.push_back (SolidLocationDetail (parentId, s, worldRayPoint));
                pickData.back ().SetFaceIndices (0,0,0);
                DVec3d uDirectionLocal, vDirectionLocal, uDirection, vDirection;
                torus.EvaluateDerivativesThetaPhi (theta, phi, vDirectionLocal, uDirectionLocal);
                uDirectionLocal.Scale (Angle::TwoPi ());
                vDirectionLocal.Scale (m_sweepAngle);
                localToWorld.MultiplyMatrixOnly (uDirection, uDirectionLocal);
                localToWorld.MultiplyMatrixOnly (vDirection, vDirectionLocal);
                pickData.back ().SetUV (uFraction, vFraction, uDirection, vDirection);
                }
            }

        if (m_capped && !Angle::IsFullCircle (m_sweepAngle))
            {
            DEllipse3d ellipse0 = torus.MinorCircle (0.0);
            DEllipse3d ellipse1 = torus.MinorCircle (m_sweepAngle);
            DPoint2d uv0, uv1;
            DPoint3d xyz0, xyz1;
            double   fraction0, fraction1;
            double uFraction, vFraction;
            DVec3d uDirection, vDirection;
            if (localRay.IntersectPlane (ellipse0, xyz0, fraction0, uv0)
                && uv0.MagnitudeSquared () <= 1.0)
                {
                worldRayPoint = ray.FractionParameterToPoint(fraction0);
                pickData.push_back (SolidLocationDetail (parentId, fraction0, worldRayPoint));
                FractionalizeInCircle (uv0.x, uv0.y, 1.0, uFraction, vFraction);
                uDirection.Scale (ellipse0.vector0, 2.0);
                vDirection.Scale (ellipse0.vector90, 2.0);
                localToWorld.MultiplyMatrixOnly (uDirection);
                localToWorld.MultiplyMatrixOnly (vDirection);
                // Those calculations are on ellipse facing inwards ... map u==>1-u
                ISolidPrimitive::ReverseFractionOrientation (uFraction, vFraction);
                ISolidPrimitive::ReverseFractionOrientation (uDirection, vDirection);
                pickData.back ().SetUV (uFraction, vFraction, uDirection, vDirection);
                pickData.back ().SetCapSelector (0);
                }
            if (localRay.IntersectPlane (ellipse1, xyz1, fraction1, uv1)
                && uv1.MagnitudeSquared () <= 1.0)
                {
                worldRayPoint = ray.FractionParameterToPoint(fraction1);
                pickData.push_back (SolidLocationDetail (parentId, fraction1, worldRayPoint));
                FractionalizeInCircle (uv1.x, uv1.y, 1.0, uFraction, vFraction);
                uDirection.Scale (ellipse1.vector0, 2.0);
                vDirection.Scale (ellipse1.vector90, 2.0);
                localToWorld.MultiplyMatrixOnly (uDirection);
                localToWorld.MultiplyMatrixOnly (vDirection);
                pickData.back ().SetUV (uFraction, vFraction, uDirection, vDirection);
                pickData.back ().SetCapSelector (1); 
                }
            }
        }
    FilterMinParameter (pickData, baseSize, minParameter);
    SortTail (pickData, baseSize);
    }


//! set point, uv coordinates, and uv derivatives vectors 
void DgnSphereDetail::SetDetailUVFromUnitSphereCoordinates
(
SolidLocationDetail &detail,//!< [in,out] detail to update
DPoint3dCR  uvw,       //!< [in] coordinates in local unit sphere space 
TransformCR localToWorld,    //! [in] transform to world coordinates
double startLatitude,
double latitudeSweep
)
    {
    double theta, phi, r;   
    Polynomial::Implicit::Sphere sphere (1.0);
    DVec3d dXdTheta, dXdPhi, dXdu, dXdv;
    sphere.XYZToThetaPhiR (uvw, theta, phi, r);
    sphere.EvaluateDerivativesThetaPhi (theta, phi, dXdTheta, dXdPhi);
    localToWorld.MultiplyMatrixOnly (dXdTheta);
    localToWorld.MultiplyMatrixOnly(dXdPhi);
    double uFraction = theta / Angle::TwoPi ();
    double vFraction;
    DoubleOps::SafeDivideParameter (vFraction, phi - startLatitude, latitudeSweep, 0.0);
    dXdu.Scale (dXdTheta, Angle::TwoPi ());
    dXdv.Scale (dXdPhi, latitudeSweep);
    detail.SetUV (uFraction, vFraction, dXdu, dXdv);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void DgnSphereDetail::AddRayIntersections
(
bvector<SolidLocationDetail> &pickData,
DRay3dCR ray,
int parentId,
double minParameter
) const
    {
    // Anything this close to the north pole in z is considered "at" the north pole.
    static double s_zTolerance = 1.0e-12;
    Transform localToWorld;
    Transform worldToLocal;
    size_t baseSize = pickData.size ();
    // It's a true unit sphere in local coordinates ....
    if (GetNonUniformTransforms (localToWorld, worldToLocal))
        {
        DRay3d localRay;
        worldToLocal.Multiply (localRay, ray);
        double ss[2];
        DPoint3d uvw[2];
        Polynomial::Implicit::Sphere sphere (1.0);
        int n = sphere.IntersectRay (localRay, ss, uvw, 2);
        double lat0, lat1, z[2];
        GetSweepLimits (lat0, lat1, z[0], z[1], true);
        if (z[1] + s_zTolerance > 1.0)
            z[1] = 1.0;
        if (z[0] - s_zTolerance < -1.0)
            z[0] = -1.0;
        for (int i = 0; i < n; i++)
            {

            if (uvw[i].z >= z[0] && uvw[i].z <= z[1])
                {
                SolidLocationDetail detail (parentId, ss[i],ray.FractionParameterToPoint (ss[i]));
                SetDetailUVFromUnitSphereCoordinates (detail, uvw[i], localToWorld, m_startLatitude, m_latitudeSweep);
                detail.SetFaceIndices (0,0,0);
                pickData.push_back (detail);
                }
            }
        if (m_capped)
            {
            GetSweepLimits (lat0, lat1, z[0], z[1], false);
            for (int i = 0; i < 2; i++)
                {
                if (fabs (z[i]) < 1.0) // z[i]==1 is a pole -- no cap surface 
                    {
                    double r = sqrt (1.0 - z[i] * z[i]);
                    // r is radius of cap circle.
                    // intersect ray with plane ...
                    // z[i] = origin.z + s * direction.z
                    double pierceFraction;
                    if (DoubleOps::SafeDivide (pierceFraction, z[i] - localRay.origin.z, localRay.direction.z, 0.0))
                        {
                        DPoint3d uvw = localRay.FractionParameterToPoint (pierceFraction);
                        if (uvw.MagnitudeSquaredXY () <= r)
                            {
                            SolidLocationDetail detail (parentId, pierceFraction, ray.FractionParameterToPoint (pierceFraction));
                            detail.SetCapSelector (i);
                            double uFraction, vFraction;
                            double da = 2.0 * r;
                            FractionalizeInCircle (uvw.x, uvw.y, r, uFraction, vFraction);
                            DVec3d uVector, vVector;
                            localToWorld.GetMatrixColumn (uVector, 0);
                            localToWorld.GetMatrixColumn (vVector, 1);
                            uVector.Scale (da);
                            vVector.Scale (da);
                            detail.SetUV (uFraction, vFraction, uVector, vVector);
                            pickData.push_back (detail);
                            }
                        }
                    }
                }
            }
        FilterMinParameter (pickData, baseSize, minParameter);
        SortTail (pickData, baseSize);
        }
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool DgnSphereDetail::GetSweepLimits
(
double &latitude0,
double &latitude1,
double &z0,
double &z1,
bool forceSweepNorth
) const
    {
    static double s_fullSphereTol = 1.0e-10;
    double a = Angle::Pi () * 0.5;
    latitude0 = DoubleOps::Clamp (m_startLatitude, -a, a);
    latitude1 = DoubleOps::Clamp (m_startLatitude + m_latitudeSweep, -a , a);
    if (forceSweepNorth && latitude0 > latitude1)
        std::swap (latitude0, latitude1);
    z0 = sin (latitude0);
    z1 = sin (latitude1);
    return fabs (z1 - z0) < 2.0 - s_fullSphereTol;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void DgnBoxDetail::AddRayIntersections
(
bvector<SolidLocationDetail> &pickData,
DRay3dCR ray,
int parentId,
double minParameter
) const
    {
    size_t baseSize = pickData.size ();
    // clockwise around faces, from outside.  Bottom, top, 4 sides.
    BoxFaces cornerData;
    cornerData.Load (*this);
    double hitParam[2];
    DPoint3d hitXYZ[2];
    DPoint2d hitUV[2];
    for (int faceIndex = 0; faceIndex < 6; faceIndex++)
        {
        if (!m_capped && BoxFaces::IsCapFace (faceIndex))
            continue;
        DBilinearPatch3d patch = cornerData.GetFace (faceIndex);
        int numHit = ray.IntersectHyperbolicParaboloid (
                hitXYZ, hitParam, hitUV,
                patch.point[0][0], patch.point[1][0], patch.point[0][1], patch.point[1][1]
                );
        for (int hit = 0; hit < numHit; hit++)
            {
            if (DoubleOps::IsIn01 (hitUV[hit]))
                {
                DPoint3d xyz;
                DVec3d dXdu, dXdv;
                patch.Evaluate (hitUV[hit].x, hitUV[hit].y, xyz, dXdu, dXdv);
                SolidLocationDetail detail (parentId, hitParam[hit], hitXYZ[hit]);
                BoxFaces::ApplyIds (faceIndex, detail);
                detail.SetUV (hitUV[hit].x, hitUV[hit].y, dXdu, dXdv);
                pickData.push_back (detail);
                }
            }
        }
    FilterMinParameter (pickData, baseSize, minParameter);
    SortTail (pickData, baseSize);
    }

// Intersect a ray with a (possibly transformed) region bounded by a curve.
void AddAreaHits
(
CurveVectorPtr curve,
bvector<SolidLocationDetail> &pickData,
DRay3dCR ray,
int parentId,
double minParameter,
Transform baseToTarget,
bool activeA,
bool activeB
)
    {
    if (!activeA && !activeB)
        return;
    Transform localToWorld[2], worldToLocal;
    bool active[2] = {activeA, activeB};
    DRange3d localRange;
    CurveVectorPtr localCurve = curve->CloneInLocalCoordinates
            (
            LOCAL_COORDINATE_SCALE_01RangeBothAxes,
            localToWorld[0], worldToLocal,
            localRange
            );

    if (localCurve.IsValid ())
        {        
        localToWorld[1] = Transform::FromProduct (baseToTarget, localToWorld[0]);
        double rayFraction;
        DPoint3d uvw;
        
        for (int i = 0; i < 2; i++)
            {
            if (active[i]
                && ray.IntersectZPlane (localToWorld[i], 0.0, uvw, rayFraction)
                && localRange.IsContainedXY (uvw))
                {
                CurveVector::InOutClassification inout = localCurve->PointInOnOutXY (uvw);
                if (  inout == CurveVector::INOUT_In
                   || inout == CurveVector::INOUT_On
                   )
                    {
                    DVec3d uVector, vVector;
                    localToWorld[i].GetMatrixColumn (uVector, 0);
                    localToWorld[i].GetMatrixColumn (vVector, 1);
                    pickData.push_back (SolidLocationDetail (parentId,
                        rayFraction,
                        ray.FractionParameterToPoint (rayFraction),
                        uvw.x, uvw.y,
                        uVector, vVector)
                        );
                    SolidLocationDetail::FaceIndices indices;
                    if (i == 0)
                        indices.SetCap0 ();
                    else
                        indices.SetCap1 ();
                    pickData.back ().SetFaceIndices (indices);
                    }     
                }
            }
        }
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void DgnExtrusionDetail::AddRayIntersections
(
bvector<SolidLocationDetail> &pickData,
DRay3dCR ray,
int parentId,
double minParameter
) const
    {
    size_t baseSize = pickData.size ();

    if (!ray.direction.IsParallelTo (m_extrusionVector))
        {
        DPlane3d plane;
        plane.origin = ray.origin;
        plane.normal = DVec3d::FromNormalizedCrossProduct (m_extrusionVector, ray.direction);
        bvector<CurveLocationDetailPair>planeIntersections;
        m_baseCurve->AppendCurvePlaneIntersections (plane, planeIntersections);
        for (size_t i = 0; i < planeIntersections.size (); i++)
            {
            CurveLocationDetailPair &planeIntersection = planeIntersections.at (i);
            DRay3d extrusionRay;
            extrusionRay.origin = planeIntersection.detailA.point;
            extrusionRay.direction = m_extrusionVector;
            double rayFraction, extrusionFraction;
            DPoint3d rayPoint, extrusionPoint;
            size_t leafIndex;
            DPoint3d curvePoint;
            DVec3d curveTangent;
            if (!planeIntersection.SameCurveAndFraction ())
                {
                }
            else if (!m_baseCurve->LeafToIndex (planeIntersection.detailA.curve, leafIndex))
                {
                }
            else if (!planeIntersection.detailA.TryComponentFractionToPoint (curvePoint, curveTangent))
                {
                }
            else if (!DRay3d::ClosestApproachUnboundedRayUnboundedRay (
                        rayFraction, extrusionFraction, rayPoint, extrusionPoint,
                        ray, extrusionRay))
                {
                }
            else if (DoubleOps::IsIn01 (extrusionFraction))
                {
                SolidLocationDetail detail (parentId, rayFraction, rayPoint);
                detail.SetUV (planeIntersection.detailA.componentFraction, extrusionFraction,
                                curveTangent, m_extrusionVector);
                detail.SetFaceIndices (0, (int)leafIndex, (int)planeIntersection.detailA.componentIndex);
                pickData.push_back (detail);
                }
            }
        }

    if (m_capped && m_baseCurve->IsAnyRegionType ())
        {
        Transform extrusionTransform = Transform::From (m_extrusionVector);
        AddAreaHits (m_baseCurve,
                pickData, ray, parentId, minParameter,
                extrusionTransform, true, true);
        for (auto &pd : pickData)
            {
            if (pd.GetFaceIndices ().IsCap0 ())
                ISolidPrimitive::ReverseFractionOrientation (pd);
            }
        }
    FilterMinParameter (pickData, baseSize, minParameter);
    SortTail (pickData, baseSize);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
static bool AddRuledSurfaceRayIntersections_recurseToPrimitives
(
bvector<struct SolidLocationDetail> &pickData,
size_t &index0,
size_t &primitiveCounter,
CurveVectorCR curveA,
CurveVectorCR curveB,
DRay3dCR ray,
int parentId
)
    {
    if (curveA.size () != curveB.size ())
        return false;
    for (size_t i = 0, n = curveA.size (); i < n; i++)
        {
        CurveVectorCP childA = curveA[i]->GetChildCurveVectorCP ();
        CurveVectorCP childB = curveB[i]->GetChildCurveVectorCP ();
        if (!childA && !childB)
            {
            size_t index0 = pickData.size ();
            ICurvePrimitive::AddRuledSurfaceRayIntersections
                (
                pickData,
                *curveA[i], *curveB[i], ray
                );
            // pickData comes back with Index2 set but others 0 ...
            for (size_t i = index0; i < pickData.size (); i++)
                {
                pickData[i].SetParentId (parentId);
                pickData[i].SetFaceIndices01 ((ptrdiff_t)index0, (ptrdiff_t)primitiveCounter);
                }
            primitiveCounter++;
            }
        else if (childA && childB)
            {
            AddRuledSurfaceRayIntersections_recurseToPrimitives
                (
                pickData,
                index0, primitiveCounter, *childA, *childB,
                ray, parentId
                );
            }
        }
    return false;
    }



/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void DgnRuledSweepDetail::AddRayIntersections
(
bvector<SolidLocationDetail> &pickData,
DRay3dCR ray,
int parentId,
double minParameter
) const
    {
    if (m_sectionCurves.size () == 0)
        return;
    
    size_t baseSize = pickData.size ();
    for (size_t i = 0, n = m_sectionCurves.size (); i +1 < n; i++)
        {
        size_t index1 = 0;
        AddRuledSurfaceRayIntersections_recurseToPrimitives (pickData,
                i, index1,
                *m_sectionCurves[i],
                *m_sectionCurves[i + 1],
                ray, parentId);
        }

    if (m_capped)
        {
        Transform identity = Transform::FromIdentity ();        
        AddAreaHits (m_sectionCurves.front (),
                pickData, ray, parentId, minParameter,
                identity, true, false);
        AddAreaHits (m_sectionCurves.back (),
                pickData, ray, parentId, minParameter,
                identity, false, true);
        for (auto &pd : pickData)
            {
            if (pd.GetFaceIndices ().IsCap0 ())
                ISolidPrimitive::ReverseFractionOrientation (pd);
            }
        }
    FilterMinParameter (pickData, baseSize, minParameter);
    SortTail (pickData, baseSize);
    }



namespace CurveLocationDetailFilter
{
// Interface for testing a detail pair.
struct PairTester
    {

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
GEOMAPI_VIRTUAL bool AcceptPair(CurveLocationDetailCR curveData, CurveLocationDetailCR rayData,
            SolidLocationDetail &detail) = 0;
    };

// Tester to check if the "a" field of the curveData is in a sweep range.
struct AngleInSweep : PairTester
    {
    double m_theta0;
    double m_sweep;
    double m_minRayParameter;
    AngleInSweep (double theta0, double sweep, double minRayParameter)
        : m_theta0 (theta0),
          m_sweep (sweep),
          m_minRayParameter (minRayParameter)
        {
        }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool AcceptPair(
    CurveLocationDetailCR curveData,
    CurveLocationDetailCR rayData,
    SolidLocationDetail &detail
    ) override
        {
        double theta = curveData.a;
        double fraction = rayData.fraction;
        if (Angle::InSweepAllowPeriodShift (theta, m_theta0, m_sweep)
                && fraction >= m_minRayParameter)
            {
            // funny coordinate mix.
            // XYZ is local ray point
            // dXdu is (world) derivative of XYZ wrt curve fraction.
            // dXdu is local derivative of pierce point wrt sweep fraction
            double u = curveData.componentFraction;
            double v = Angle::NormalizeToSweep (theta, m_theta0, m_sweep);
            DVec3d curveTangent, localSweepTangent;
            DPoint3d curveXYZ;
            curveData.curve->ComponentFractionToPoint ((int)curveData.componentIndex, u, curveXYZ, curveTangent);
            localSweepTangent.Init (-rayData.point.y, rayData.point.x, 0.0);
            localSweepTangent.Scale (m_sweep);
            detail.SetPickParameter (rayData.fraction);
            detail.SetUV (u, v, curveTangent, localSweepTangent);
            detail.SetXYZ (curveXYZ);
            detail.SetFaceIndices (0, (int)rayData.componentIndex, (int)curveData.componentIndex);   // (only one sweep step, leafIndex varies)
            return true;
            }
        return false;
        }
    };

// Tester to check if the "a" field of the curveData is in a simple range.
struct DistanceInRange : PairTester
    {
    double m_a0;
    double m_a1;
    double m_minRayParameter;
    DistanceInRange (double a0, double a1, double minRayParameter)
        {
        m_a0 = DoubleOps::Min (a0, a1);
        m_a1 = DoubleOps::Max (a0, a1);
        m_minRayParameter = minRayParameter;
        }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool AcceptPair(
    CurveLocationDetailCR curveData,
    CurveLocationDetailCR rayData,
    SolidLocationDetail &detail
    ) override
        {
        double a = curveData.a;
        double fraction = rayData.a;
        detail = SolidLocationDetail (0, curveData.fraction, curveData.point);
        if((a - m_a0) * (a - m_a1) <= 0.0
              && fraction >= m_minRayParameter)
            {
            double u = curveData.fraction;
            double v = (a - m_a0) / (m_a1 - m_a0);
            DVec3d uDirection, vDirection;
            uDirection.Zero ();
            vDirection.Zero ();
            detail.SetUV (u, v, uDirection, vDirection);
            return true;
            }
        return false;
        }
    };
};


/*--------------------------------------------------------------------------------**//**
* @bsistruct                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
struct IntersectionLists
{
bvector<CurveLocationDetail> m_curveDetail;
bvector<CurveLocationDetail> m_rayDetail;   // pick ray position


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void BuildPickData(bvector<SolidLocationDetail> &pickData, int parentId,
    CurveLocationDetailFilter::PairTester *tester)
    {
    assert (m_curveDetail.size () == m_rayDetail.size ());

    for (size_t i = 0; i < m_curveDetail.size (); i++)
        {
        SolidLocationDetail detail;
        if (NULL != tester
            && tester->AcceptPair (m_curveDetail[i], m_rayDetail[i], detail))
            {
            detail.SetParentId (parentId);
            pickData.push_back (detail);
            }
        }
    }

};

// Find intersections of geometry primitives with the rotation of a line around an axis
// (line may not be perpendicular to axis)
// Stored intersection is recored in a curveDetail and rayDetail.
// curveDetail.fraction = fraction along curve primitive.
// curveDetail.a = rotation angle to intersection.
// curveDetail.point = global coordinates of hit point. (not primary point on curve)
// rayDetail.fraction = fraction along ray.
// rayDetail.componentIndex = recursive leaf counter.
// rayDetail.point = local coordinates on ray.
struct RotatedNonPerpendicularLineIntersectionProcessor: public ICurvePrimitiveProcessor, IntersectionLists
{
Transform m_worldToLocal;
Transform m_localToWorld;
DRay3d    m_worldRay;
DRay3d    m_localRay;
DRay3d    m_normalizedRay;  // origin is at closest approach to z axis.  direction.z is +1
double    m_rayApproachFraction; // (local) ray fraction at close approach.
double    m_r0;     // distance from local z axis to closest point on ray.  Seems ok to be zero.
double    m_z0;     // z origin for central quadric.  (assert: matches m_rayApproachPoint.z)
double    m_dtdz;   // parameter change on m_localRay per z change on normalized ray (which has parameter equal z)
double    m_alphaPerp;      // part of ray vector perpendicular to local z. (assert:  local direction vector xy part magnitude)

DPoint4d  m_quadricDiagonal;

size_t m_leafCounter;


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void ResetLeafCounter(){m_leafCounter = 0;}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void IncrementLeafCounter(){m_leafCounter++;}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
size_t GetLeafCounter(){ return m_leafCounter;}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void StartLeaf(){}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void FinishLeaf() {IncrementLeafCounter ();}

RotatedNonPerpendicularLineIntersectionProcessor (TransformCR localToWorld, TransformCR worldToLocal) :
    m_worldToLocal (worldToLocal),
    m_localToWorld (localToWorld)
    {
    }
GEOMAPI_VIRTUAL ~RotatedNonPerpendicularLineIntersectionProcessor (){}



/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool SetupQuadraticForm(DRay3dCR worldRay)
    {
    m_worldRay = worldRay;
    m_worldToLocal.Multiply (m_localRay, worldRay);
    DVec3d localZ = DVec3d::From (0,0,1);
    DRay3d zRay = DRay3d::FromOriginAndVector (DPoint3d::From(0,0,0), localZ);
    if (m_localRay.direction.IsPerpendicularTo (localZ))
        return false;
    double fractionZ;
    DPoint3d zPoint;
    DPoint3d rayApproachPoint;
    DRay3d::ClosestApproachUnboundedRayUnboundedRay (
            m_rayApproachFraction, fractionZ,
            rayApproachPoint, zPoint,
            m_localRay, zRay
            );
    m_r0 = rayApproachPoint.MagnitudeXY ();
    m_normalizedRay.origin = rayApproachPoint;
    m_z0 = rayApproachPoint.z;
    if (!DoubleOps::SafeDivide (m_dtdz, 1.0, m_localRay.direction.z, 0.0))
        return false;
    m_normalizedRay.direction = DVec3d::FromScale (m_localRay.direction, m_dtdz);
    m_alphaPerp     = m_normalizedRay.direction.MagnitudeXY ();
    m_quadricDiagonal = DPoint4d::From (1,1, - m_alphaPerp * m_alphaPerp, -m_r0 * m_r0);
    // The quadric is m_axx * x^2 + m_ayy * y^2 + m_axx * z^2 + m_aww * w^2 = 0 !!!!
    // (for coordinates in the quadric's local frame, which has z=0 at closest approach !!!)
    // (Subtract rayApproachPoint.z from z coordinate of curves!!!)
    // If (x,y,z,w) are parametric degree d, expand into the quadric, intersections are
    //     parametric degree 2*d. 
    // If ray passes through axis, m_r0 is zero and it is a cone.
    // If ray is parallel to axis, m_alphaPerp is zero and it is a cylinder
    //    (Closest approach gave us an appropriate z point for this.)
    // Otherwise both are nonzero and it is a hyperbolic paraboloid.
    return true;
    }

//
bool FindRayPoint (
//! world point being rotated to ray
DPoint3dCR worldXYZ,
//! normalized ray fraction at contact
double &rayFraction,
//! point on ray
DPoint3dR rayPoint,
//! rotation from world point to ray
double &rotationRadians
)
    {
    DPoint3d localXYZ;
    m_worldToLocal.Multiply (localXYZ, worldXYZ);
    rayFraction = m_rayApproachFraction + m_dtdz * (localXYZ.z - m_z0);
    rayPoint = m_localRay.FractionParameterToPoint (rayFraction);
    rotationRadians = localXYZ.AngleToXY (rayPoint);
    return true;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void ProcessMappedLine
(
ICurvePrimitiveCR curve,
DSegment3dCR segment,
size_t componentIndex, 
size_t numComponent,
DSegment1dCP interval,
DSegment1dCR mappingInterval    // remap local fraction to this parent.
)
    {
    double bezCoffs[10];    // only need 3 !!
    double bezRoots[10];
    DSegment3d localSegment;
    DPoint4d localSegmentAsBezier[10];
    m_worldToLocal.Multiply (localSegment, segment);
    localSegmentAsBezier[0] = DPoint4d::From (localSegment.point[0].x, localSegment.point[0].y,
                    localSegment.point[0].z - m_z0, 1.0);
    localSegmentAsBezier[1] = DPoint4d::From (localSegment.point[1].x, localSegment.point[1].y,
                    localSegment.point[1].z - m_z0, 1.0);
    int curveOrder = 2;
    int productOrder;
    int numRoot;
    bsiBezierDPoint4d_weightedInnerProduct (bezCoffs, productOrder,
                m_quadricDiagonal,
                localSegmentAsBezier, localSegmentAsBezier,
                curveOrder);
    if (bsiBezier_univariateRoots (bezRoots, &numRoot, bezCoffs, productOrder)
        && numRoot > 0)
        {
        for (int i = 0; i < numRoot; i++)
            {
            double segmentFraction = bezRoots[i];
            DPoint3d segmentPoint, rayPoint;
            double rayFraction, theta;
            segment.FractionParameterToPoint (segmentPoint, segmentFraction);
            if (FindRayPoint (segmentPoint, rayFraction, rayPoint, theta))
                {
                CurveLocationDetail curveDetail (&curve,
                        mappingInterval.FractionToPoint(segmentFraction), segmentPoint,
                        componentIndex, numComponent, segmentFraction);
                curveDetail.SetDistance (theta);
                m_curveDetail.push_back (curveDetail);
                CurveLocationDetail rayDetail (NULL, rayFraction, rayPoint);
                rayDetail.componentIndex = GetLeafCounter ();
                m_rayDetail.push_back (rayDetail);
                }
            } 
        }

    }

void _ProcessLine
(
ICurvePrimitiveCR curve,
DSegment3dCR segment,
DSegment1dCP interval
)
override
    {
    StartLeaf ();
    DSegment1d mappingInterval (0.0,1.0);
    ProcessMappedLine (curve, segment, 0, 1, interval, mappingInterval);
    FinishLeaf ();
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void _ProcessBsplineCurve (ICurvePrimitiveCR curve, MSBsplineCurveCR bcurve, DSegment1dCP interval) override
    {
    StartLeaf ();
    double bezCoffs[MAX_BEZIER_ORDER];
    double bezRoots[MAX_BEZIER_ORDER];
    BCurveSegment segment;
    for (size_t segmentIndex = 0; bcurve.AdvanceToBezier (segment, segmentIndex, true);)
        {
        DPoint4dP workPoles = segment.GetWorkPoleP (0);
        int segmentOrder = (int)segment.GetOrder ();
        int productOrder;
        m_worldToLocal.Multiply (workPoles, segment.GetPoleP (), segmentOrder);
        double bigNum = 0.0;
        for (int i = 0; i < segmentOrder; i++)
            bigNum = DoubleOps::MaxAbs (bigNum, workPoles[i].MaxAbs ());
        double scale;
        DoubleOps::SafeDivide (scale, 1.0, bigNum, 1.0);
        for (int i = 0; i < segmentOrder; i++)
            workPoles[i].z -= m_z0 * workPoles[i].w;
        for (int i = 0; i < segmentOrder; i++)
            workPoles[i].Scale (scale);
        bsiBezierDPoint4d_weightedInnerProduct (bezCoffs, productOrder,
                    m_quadricDiagonal,
                    workPoles, workPoles,
                    segmentOrder);
        int numRoot = 0;
        if (bsiBezier_univariateRoots (bezRoots, &numRoot, bezCoffs, productOrder)
            && numRoot > 0)
            {
            for (int i = 0; i < numRoot; i++)
                {
                double segmentFraction = bezRoots[i];
                double knot = segment.FractionToKnot (bezRoots[i]);
                double curveFraction = bcurve.KnotToFraction (knot);
                // ellipse angles map unchanged through world to local ...
                DPoint3d worldCurvePoint = segment.FractionToPoint (segmentFraction);
                double rayFraction;
                DPoint3d rayPoint;
                double curveRotationAngle;
                if (FindRayPoint (worldCurvePoint, rayFraction, rayPoint, curveRotationAngle))
                    {
                    CurveLocationDetail curveDetail (&curve, curveFraction, worldCurvePoint);
                    curveDetail.SetDistance (curveRotationAngle);
                    CurveLocationDetail rayDetail (NULL, rayFraction, rayPoint);
                    rayDetail.componentIndex = GetLeafCounter ();
                    m_curveDetail.push_back (curveDetail);
                    m_rayDetail.push_back (rayDetail);
                    }
                }
            }        
        }
    }

void _ProcessArc
(
ICurvePrimitiveCR curve,
DEllipse3dCR worldArc,
DSegment1dCP interval
)
override
    {
    StartLeaf ();
    DEllipse3d localArc;
    m_worldToLocal.Multiply (localArc, worldArc);
    QuadraticArc solver (localArc, 0.0, 0.0, m_z0);
    double arcAngles[10];
    int numAngle = solver.SolveDiagonalQuadric (m_quadricDiagonal, arcAngles, 10);
    for (int i = 0; i < numAngle; i++)
        {
        // ellipse angles map unchanged through world to local ...
        DPoint3d worldArcPoint;
        double   arcFraction = worldArc.AngleToFraction (arcAngles[i]);
        worldArc.Evaluate (worldArcPoint, arcAngles[i]);
        double rayFraction;
        DPoint3d rayPoint;
        double curveRotationAngle;
        if (FindRayPoint (worldArcPoint, rayFraction, rayPoint, curveRotationAngle))
            {
            CurveLocationDetail curveDetail (&curve, arcFraction, worldArcPoint);
            curveDetail.SetDistance (curveRotationAngle);
            CurveLocationDetail rayDetail (NULL, rayFraction, rayPoint);
            rayDetail.componentIndex = GetLeafCounter ();
            m_curveDetail.push_back (curveDetail);
            m_rayDetail.push_back (rayDetail);
            }
        }
    FinishLeaf ();
    }
    
void _ProcessLineString (ICurvePrimitiveCR curve, bvector<DPoint3d> const &points, DSegment1dCP interval) override
    {
    StartLeaf ();
    size_t n = points.size ();
    if (n < 1)
        return;
    double df = 1.0 / (double) (n-1);
    for (size_t i = 0; i + 1 < n; i++)
        {
        DSegment3d segment;
        DSegment1d mappingInterval (i*df, (i+1)*df);
        segment.point[0] = points[i];
        segment.point[1] = points[i+1];
        ProcessMappedLine (curve, segment, i, n - 1, interval, mappingInterval);
        }
    FinishLeaf ();
    }
    
};

// Find intersections of geometry primitives with the rotation of a line around an axis
// (line may not be perpendicular to axis)
struct RotatedPerpendicularLineIntersectionProcessor: public IntersectionLists
{
Transform m_worldToLocal;
Transform m_localToWorld;
DRay3d    m_worldRay;
DRay3d    m_localRay;
DPlane3d  m_localSweptPlane;  // local z coordinate of the plane swept as the ray rotates around the z axis.
DPlane3d  m_globalSweptPlane;

RotatedPerpendicularLineIntersectionProcessor (TransformCR localToWorld, TransformCR worldToLocal) :
    m_worldToLocal (worldToLocal),
    m_localToWorld (localToWorld)
    {
    }
GEOMAPI_VIRTUAL ~RotatedPerpendicularLineIntersectionProcessor (){}



/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool SetupPlane(DRay3dCR worldRay)
    {
    m_worldRay = worldRay;
    m_worldToLocal.Multiply (m_localRay, worldRay);
    DVec3d localZ = DVec3d::From (0,0,1);
    if (!m_localRay.direction.IsPerpendicularTo (localZ))
        return false;
    m_localSweptPlane = DPlane3d::FromOriginAndNormal (DPoint3d::From (0,0, m_localRay.origin.z), localZ);
    m_localToWorld.Multiply (m_globalSweptPlane, m_localSweptPlane);
    return true;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void ProcessCurveVector(
DgnRotationalSweepDetail const &sweptSurface,
CurveVectorCR curves,
bvector<SolidLocationDetail> &pickData,
int parentId,
double sweepAngle,
double minRayParameter
)
    {
    bvector<CurveLocationDetailPair>planeIntersections;
    curves.AppendCurvePlaneIntersections (m_globalSweptPlane, planeIntersections);
    // We have (world) intersections with the plane.
    // But we need to know what point of the ray hits those points.
    // Each plane point generally leads to two ray points.
    for (size_t k = 0; k < planeIntersections.size (); k++)
        {
        size_t leafIndex = 0;
        CurveLocationDetailPair &planeIntersection = planeIntersections.at (k);
        DPoint3d worldCurvePoint;
        DVec3d   worldCurveTangent;
        double componentFraction = planeIntersection.detailA.componentFraction;
        ptrdiff_t curveComponent = (ptrdiff_t)planeIntersection.detailA.componentIndex;
        if (!planeIntersection.SameCurveAndFraction ())
            {
            }
        else if (!curves.LeafToIndex (planeIntersections[k].detailA.curve, leafIndex))
            {
            }
        else if (!planeIntersection.detailA.TryComponentFractionToPoint (worldCurvePoint, worldCurveTangent))
            {
            }
        else 
            {
            DPoint3d worldXYZOnCurve = planeIntersections[k].detailA.point;
            DPoint3d localXYZOnCurve;
            m_worldToLocal.Multiply (localXYZOnCurve, worldXYZOnCurve);
            double rSquared = localXYZOnCurve.MagnitudeSquaredXY ();
            // As the curve point rotates around it stays at rSquared from the 
            // axis.  Find the ray fractions where the ray is the same squared distance
            // from the axis.
            Polynomial::Power::Degree2 quadratic;
            quadratic.AddSquaredLinearTerm (m_localRay.origin.x, m_localRay.direction.x);
            quadratic.AddSquaredLinearTerm (m_localRay.origin.y, m_localRay.direction.y);
            quadratic.AddConstant (-rSquared);
            double ss[2];   // ray parameters of real roots.
            int numRoot = quadratic.RealRoots (ss);
            for (int i = 0; i < numRoot; i++)
                {
                DPoint3d localXYZOnRay = m_localRay.FractionParameterToPoint (ss[i]);
                double theta = localXYZOnCurve.AngleToXY (localXYZOnRay);
                DPoint3d worldXYZOnRay = m_worldRay.FractionParameterToPoint (ss[i]);
                m_localToWorld.Multiply (worldXYZOnRay, localXYZOnRay);
                if (Angle::InSweepAllowPeriodShift (theta, 0.0, sweepAngle)
                    && ss[i] > minRayParameter
                    )
                    {
                    double vFraction = Angle::NormalizeToSweep (theta, 0.0, sweepAngle);
                    Transform curveToPointSurfacePoint, curvePointToSurfaceTangent;
                    if (sweptSurface.GetVFractionTransform (vFraction, curveToPointSurfacePoint, curvePointToSurfaceTangent))
                        {
                        DVec3d uVector, vVector;
                        curveToPointSurfacePoint.MultiplyMatrixOnly (uVector, worldCurveTangent);
                        curvePointToSurfaceTangent.Multiply (vVector, worldCurvePoint);
                        SolidLocationDetail detail (parentId, ss[i], worldXYZOnRay);
                        detail.SetUV (componentFraction, vFraction, uVector, vVector);
                        detail.SetFaceIndices (0, (ptrdiff_t)leafIndex, curveComponent);
                        pickData.push_back (detail);
                        }
                    }
                }
            }
        }
    }
};





/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void DgnRotationalSweepDetail::AddRayIntersections
(
bvector<SolidLocationDetail> &pickData,
DRay3dCR ray,
int parentId,
double minParameter
) const
    {
    Transform worldToLocal, localToWorld;
    if (GetTransforms (localToWorld, worldToLocal))
        {
        RotatedNonPerpendicularLineIntersectionProcessor intersector (localToWorld, worldToLocal);
        if (intersector.SetupQuadraticForm (ray))
            {
            intersector.ResetLeafCounter ();
            intersector._ProcessCurveVector (*m_baseCurve, NULL);
            CurveLocationDetailFilter::AngleInSweep angleTester(0.0, m_sweepAngle, minParameter);
            intersector.BuildPickData (pickData, parentId, &angleTester);
            // pick data world dXdu is at the curve.  rotate it to the pick point.
            for (size_t i = 0; i < pickData.size (); i++)
                {
                SolidLocationDetail & pick = pickData.at (i);
                DVec3d dXdu = pick.GetUDirection ();    // world coordinates at curve !!!
                DVec3d dXdv = pick.GetVDirection ();    // local coordinates at pick !!!
                worldToLocal.MultiplyMatrixOnly (dXdu);
                DPoint2d uvFractions = pick.GetUV ();
                double theta = uvFractions.y * m_sweepAngle;
                dXdu.RotateXY (theta);
                localToWorld.MultiplyMatrixOnly (dXdu);
                localToWorld.MultiplyMatrixOnly (dXdv);
                pick.SetUDirection (dXdu);
                pick.SetVDirection (dXdv);
                pick.SetXYZ (ray.FractionParameterToPoint (pickData[i].GetPickParameter ()));
                }
            }
        else
            {
            RotatedPerpendicularLineIntersectionProcessor intersector1 (localToWorld, worldToLocal);
            if (intersector1.SetupPlane (ray))
                {
                intersector1.ProcessCurveVector (*this, *m_baseCurve, pickData, parentId, m_sweepAngle, minParameter);
                }
            }
        }

    Transform sweepTransform, sweepDerivative;
    if (   m_capped && m_baseCurve->IsAnyRegionType ()
        && GetVFractionTransform (1.0, sweepTransform, sweepDerivative)
      )
        {
        AddAreaHits (m_baseCurve,
                pickData, ray, parentId, minParameter,
                sweepTransform, true, true);
        for (auto &pd : pickData)
            {
            if (pd.GetFaceIndices ().IsCap0 ())
                ISolidPrimitive::ReverseFractionOrientation (pd);
            }
        }
    }

END_BENTLEY_GEOMETRY_NAMESPACE


