/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include "BoxFaces.h"
BEGIN_BENTLEY_GEOMETRY_NAMESPACE
bool FractionalizeInCircle(double x, double y, double r, double &u, double &v);

static void InitClosestPoint (SolidLocationDetailR pickData)
    {
    pickData.Init ();
    pickData.SetPickParameter (DBL_MAX);
    }

static void UpdatePick 
(
SolidLocationDetailR pickData,
DPoint3dCR spacePoint,
DPoint3dCR surfacePoint,
double u,
double v,
int index0,
int index1,
int index2 = -1
)
    {
    double d2 = spacePoint.DistanceSquared (surfacePoint);
    if (d2 < pickData.GetPickParameter ())
        {
        pickData.SetU (u);
        pickData.SetV (v);
        pickData.SetPickParameter (d2);
        pickData.SetXYZ (surfacePoint);
        pickData.SetFaceIndices (index0, index1, index2);
        }
    }

static void UpdatePick (SolidLocationDetailR pickData, SolidLocationDetailCR candidate)
    {
    if (candidate.GetPickParameter () < pickData.GetPickParameter ())
        pickData = candidate;
    }
    
static void UpdatePatchPick
(
SolidLocationDetailR pickData,
DPoint3dCR spacePoint,
DBilinearPatch3dCR patch,
int id0,
int id1,
int id2 = 0,
double u0 = 0.0,
double u1 = 1.0
)
    {
    bvector<DPoint2d> uv;
    patch.PerpendicularsOnBoundedPatch (spacePoint, uv);
    for (size_t i = 0; i < uv.size (); i++)
        {
        if (DoubleOps::IsIn01 (uv[i]))
            {
            DPoint3d xyz;
            DVec3d dXdu, dXdv;
            patch.Evaluate (uv[i].x, uv[i].y, xyz, dXdu, dXdv);
            UpdatePick (pickData, spacePoint, xyz, uv[i].x, uv[i].y, id0, id1, id2);
            }
        }
    }

static bool HasRealPoint (SolidLocationDetailR pickData)
    {
    return pickData.GetPickParameter () < DBL_MAX;
    }
   
// Update pick data with closest point on body of region bounded by transformed curves.Intersect a ray with a (possibly transformed) region bounded by a curve.
void UpdateByAreaPick
(
SolidLocationDetail &pickData,
CurveVectorPtr curve,
DPoint3dCR spacePoint,
TransformCP curveTransforms,    // array of transforms!!
int numTransform
)
    {
    if (!curve->IsAnyRegionType ())
        return;

    Transform regionToWorld, worldToRegion;
    DRange3d localRange;
    // We have to make a copy of the curve rotated to xy plane.
    // But instead of moving it to end position, we will move the point back into the xy system.
    // This saves one cuve copy. (And anyway the inout test only happens in xy space);
    CurveVectorPtr localCurve = curve->CloneInLocalCoordinates
            (
            LOCAL_COORDINATE_SCALE_01RangeBothAxes,
            regionToWorld, worldToRegion,
            localRange
            );
            
        
    for (int i = 0; i < numTransform; i++)
        {
        Transform inverseTransform;
        if (!inverseTransform.InverseOf (curveTransforms[i]))
            continue;
        // pull the space point back to the original curve.
        DPoint3d worldRetract, regionRetract;
        inverseTransform.Multiply (worldRetract, spacePoint);
        worldToRegion.Multiply (regionRetract, worldRetract);
        if (localRange.IsContainedXY (regionRetract))
            {
            CurveVector::InOutClassification inout = localCurve->PointInOnOutXY (regionRetract);
            if (  inout == CurveVector::INOUT_In
               || inout == CurveVector::INOUT_On
               )
                {
                DPoint3d xyzA, xyzB, xyzC;
                xyzA = regionRetract;
                xyzA.z = 0.0;
                regionToWorld.Multiply (xyzB, xyzA);
                curveTransforms[i].Multiply (xyzC, xyzB);
                UpdatePick (pickData, spacePoint, xyzC, regionRetract.x, regionRetract.y,
                            SolidLocationDetail::PrimaryIdCap, i, 0);                
                }
            }        
        }
    }

// Update pick data with closest point on body of region.
void UpdateBySingleAreaPick
(
SolidLocationDetail &pickData,
CurveVectorPtr curve,
DPoint3dCR spacePoint,
int index1
)
    {
    if (!curve->IsAnyRegionType ())
        return;

    Transform regionToWorld, worldToRegion;
    DRange3d localRange;
    // We have to make a copy of the curve rotated to xy plane.
    // But instead of moving it to end position, we will move the point back into the xy system.
    // This saves one cuve copy. (And anyway the inout test only happens in xy space);
    CurveVectorPtr localCurve = curve->CloneInLocalCoordinates
            (
            LOCAL_COORDINATE_SCALE_01RangeBothAxes,
            regionToWorld, worldToRegion,
            localRange
            );
            
    DPoint3d worldRetract, regionRetract;
    worldRetract = spacePoint;
    worldToRegion.Multiply (regionRetract, worldRetract);
    if (localRange.IsContainedXY (regionRetract))
        {
        CurveVector::InOutClassification inout = localCurve->PointInOnOutXY (regionRetract);
        if (  inout == CurveVector::INOUT_In
            || inout == CurveVector::INOUT_On
            )
            {
            DPoint3d xyzA, xyzB;
            xyzA = regionRetract;
            xyzA.z = 0.0;
            regionToWorld.Multiply (xyzB, xyzA);
            UpdatePick (pickData, spacePoint, xyzB, regionRetract.x, regionRetract.y,
                        SolidLocationDetail::PrimaryIdCap, index1, 0);                
            }
        }        
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool DgnConeDetail::ClosestPoint
(
DPoint3dCR pointA,
SolidLocationDetailR pickData
) const
    {
    Transform localToWorld, worldToLocal;
    double radiusA, radiusB;
    InitClosestPoint (pickData);

    DEllipse3d ellipse[2];
    DPoint3d localPointA, pointB, localPointB, pointC;
    double u, v;
    if (GetTransforms (localToWorld, worldToLocal, radiusA, radiusB))
        {
        FractionToSection (0.0, ellipse[0]);
        FractionToSection (1.0, ellipse[1]);
        worldToLocal.Multiply (localPointA, pointA);
        double theta = Angle::Atan2 (localPointA.y, localPointA.x);
        u = theta / Angle::TwoPi ();
        DSegment3d worldSegment;
        if (FractionToRule (u, worldSegment))
            {
            worldSegment.ProjectPointBounded (pointB, v, pointA);
            UpdatePick (pickData, pointA, pointB, u, v, 0, 0, 0);
            }
        if (m_capped)
            {
            // Get localPointB back to closest point on circular disk ...
            localPointB = localPointA;
            double r = localPointB.MagnitudeXY ();
            double zPick, rCircle;
            int capIndex;
            if (localPointB.z > 0.5)
                {
                zPick = 1.0;
                rCircle = radiusB;
                capIndex = 1;
                }
            else
                {
                zPick = 0.0;
                rCircle = radiusA;
                capIndex = 0;
                }
            double mu = r > rCircle ? rCircle /r : 1.0;
            DPoint3d localPointC = DPoint3d::From
                    (localPointB.x * mu, localPointB.y * mu, zPick);
            FractionalizeInCircle (localPointC.x, localPointC.y, rCircle, u, v);
            localToWorld.Multiply (pointC, localPointC);
            UpdatePick (pickData, pointA, pointC, u, v, 
                        SolidLocationDetail::PrimaryIdCap, capIndex, 0);
            }
        }
    return HasRealPoint (pickData);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool DgnTorusPipeDetail::ClosestPoint
(
DPoint3dCR spacePoint,
SolidLocationDetailR pickData
) const
    {
    DPoint3d center;
    RotMatrix axes;
    Transform localToWorld, worldToLocal;
    double radiusA, radiusB;
    double sweepRadians;
    InitClosestPoint (pickData);
    if (TryGetFrame (center, axes, radiusA, radiusB, sweepRadians))
        {
        localToWorld = Transform::From (axes, center);
        worldToLocal.InverseOf (localToWorld);
        Polynomial::Implicit::Torus torus (radiusA, radiusB, GetReverseVector90 ());
        DPoint3d localSpacePoint, localTorusPoint, worldTorusPoint;
        worldToLocal.Multiply (localSpacePoint, spacePoint);
        double theta, phi, r, xyDist;
        torus.XYZToThetaPhiDistance (localSpacePoint, theta, phi, r, xyDist);
        double thetaTestFraction = Angle::NormalizeToSweep (theta, 0.0, m_sweepAngle, true, true);
        if (thetaTestFraction < 0.0)
            thetaTestFraction = 0.0;
        if (thetaTestFraction > 1.0)
            thetaTestFraction = 1.0;
        double thetaTest = thetaTestFraction * m_sweepAngle;
        localTorusPoint = torus.EvaluateThetaPhi (thetaTest, phi);
        localToWorld.Multiply (worldTorusPoint, localTorusPoint);
        double phi1 = Angle::NormalizeToSweep (phi, 0.0, Angle::TwoPi ());
        UpdatePick (pickData, spacePoint, worldTorusPoint, phi1, thetaTestFraction, 0, 0, 0);
        if (HasRealCaps ())
            {
            double u, v;
            // Project the point onto each cap disk.  Retract to unit disk and return to space.
            for (int capIndex = 0; capIndex < 2; capIndex++)
                {
                DEllipse3d disk = VFractionToUSectionDEllipse3d ((double)capIndex);
                Transform worldToDisk, diskToWorld;
                if (disk.GetLocalFrame (diskToWorld, worldToDisk))
                    {
                    DPoint3d diskLocal, diskWorld;
                    worldToDisk.Multiply (diskLocal, spacePoint);
                    diskLocal.z = 0.0;
                    double rho = diskLocal.MagnitudeXY ();
                    if (rho > 1.0)
                        diskLocal.Scale (1.0 / rho);
                    diskToWorld.Multiply (diskWorld, diskLocal);
                    FractionalizeInCircle (diskLocal.x, diskLocal.y, 1.0, u, v);
                    UpdatePick (pickData, spacePoint, diskWorld, u, v,
                                    SolidLocationDetail::PrimaryIdCap, capIndex, 0);
                    }
                }
            if (pickData.GetFaceIndices().IsCap0())
                ISolidPrimitive::ReverseFractionOrientation(pickData);
            }
        }
    return HasRealPoint (pickData);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool DgnSphereDetail::ClosestPoint
(
DPoint3dCR spacePoint,
SolidLocationDetailR pickData
) const
    {
    Transform localToWorld;
    Transform worldToLocal;
    InitClosestPoint (pickData);
    Polynomial::Implicit::Sphere sphere (1.0);    
    double theta, phi, r, u, v;
    // It's a true unit sphere in local coordinates ....
    if (GetNonUniformTransforms (localToWorld, worldToLocal))
        {
        double z[2], lat[2];
        DPoint3d spaceLocal, sphereLocal, sphereWorld;
        worldToLocal.Multiply (spaceLocal, spacePoint);
        sphereLocal.Normalize (spaceLocal);
        sphere.XYZToThetaPhiR (sphereLocal, theta, phi, r);
        GetSweepLimits (lat[0], lat[1], z[0], z[1], true);
        // Force spherical coordinates to bounding circle if beyond....
        if (phi > lat[1])
            phi = lat[1];
        if (phi < lat[0])
            phi = lat[0];
        sphereLocal = sphere.EvaluateThetaPhi (theta, phi);
        localToWorld.Multiply (sphereWorld, sphereLocal);        
        u = LongitudeToUFraction (theta);
        v = LatitudeToVFraction (phi);
        UpdatePick (pickData, spacePoint, sphereWorld, u, v, 0, 0, 0);
        if (m_capped)
            {
            DPoint3d capPointLocal, capPointWorld;
            GetSweepLimits (lat[0], lat[1], z[0], z[1], false);
            for (int capIndex = 0; capIndex < 2; capIndex++)
                {
                if (fabs (z[capIndex]) < 1.0) // z[i]==1 is a pole -- no cap surface 
                    {
                    double rLocal = cos (lat[capIndex]);
                    if (spaceLocal.MagnitudeXY () <= rLocal)
                        {
                        capPointLocal = spaceLocal;
                        capPointLocal.z = z[capIndex];
                        localToWorld.Multiply (capPointWorld, capPointLocal);
                        FractionalizeInCircle(capPointLocal.x, capPointLocal.y, rLocal, u, v);
                        UpdatePick (pickData, spacePoint, capPointWorld, u, v,
                                                        SolidLocationDetail::PrimaryIdCap, capIndex, 0);
                        }
                    }
                }
            }
        }
    return HasRealPoint (pickData);        
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool DgnBoxDetail::ClosestPoint
(
DPoint3dCR spacePoint,
SolidLocationDetailR pickData
) const
    {
    // clockwise around faces, from outside.  Bottom, top, 4 sides.
    BoxFaces cornerData;
    InitClosestPoint (pickData);
    cornerData.Load (*this);
    for (int faceIndex = 0; faceIndex < 6; faceIndex++)
        {
        if (!m_capped && BoxFaces::IsCapFace (faceIndex))
            continue;
        int id0 = BoxFaces::FaceIdToPrimarySelector (faceIndex);
        int id1 = BoxFaces::FaceIdToSecondarySelector (faceIndex);
        DBilinearPatch3d patch = cornerData.GetFace (faceIndex);
        UpdatePatchPick (pickData, spacePoint, patch, id0, id1);
        // Projection only considered interior to face.   Maybe we are close to an edge but
        //   not projection to a face directly.
        // Hmmm.. if there was a projection, it will certainly be closer than any edge point.
        //    So maybe we could skip looking for edge point.   But let's not be too tricky.
        // Only test along edges if faceId is higher number than neighbor.
        // Tricky tricky:  The caps are numbered 0 and 1.
        //    Side face is always higher than cap -- so no need to check if neighbor
        //    is a missing cap.
        for (int edgeId = 0; edgeId < 4; edgeId++)
            {
            int partnerFaceId = BoxFaces::FaceIdEdgeIdToPartnerFaceId (faceIndex,edgeId);
            if (partnerFaceId > faceIndex)
                continue;
            DSegment3d edge = patch.GetCCWEdge (edgeId);
            DPoint3d xyz;
            double edgeFraction;
            edge.ProjectPointBounded (xyz, edgeFraction, xyz);
            DPoint2d uv = DPoint2d::FromZero ();
            switch (edgeId)
                {
                case 0: uv = DPoint2d::From (edgeFraction, 0.0); break;
                case 1: uv = DPoint2d::From (1.0, edgeFraction); break;
                case 2: uv = DPoint2d::From (1.0 - edgeFraction, 1.0); break;
                case 3: uv = DPoint2d::From (0.0, 1.0 - edgeFraction); break;
                }
            UpdatePick (pickData, spacePoint, xyz, uv.x, uv.y, id0, id1, 0);
            }
        }
    return HasRealPoint (pickData);        
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool DgnExtrusionDetail::TryGetExtrusionFrame (TransformR localToWorld, TransformR worldToLocal) const
    {
    DPoint3d origin;
    if (m_baseCurve->GetStartPoint (origin))
        {
        RotMatrix axes;
        if (axes.InitFrom1Vector (m_extrusionVector, 2, true))
            {
            localToWorld.InitFrom (axes, origin);
            worldToLocal.InverseOf (localToWorld);
            return true;
            }
        }
    localToWorld.InitIdentity ();
    worldToLocal.InitIdentity ();
    return false;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool DgnExtrusionDetail::ClosestPoint
(
DPoint3dCR spacePoint,
SolidLocationDetailR pickData
) const
    {
    Transform worldToLocal, localToWorld;
    InitClosestPoint (pickData);
    if (TryGetExtrusionFrame (localToWorld, worldToLocal))
        {
        DMatrix4d worldToLocal4d;
        worldToLocal4d.InitFrom (worldToLocal);
        CurveLocationDetail detail;
        // Find the closest point on the curve when looking down the extrusion vector.
        // Make sure the space point projects to the body.
        if (m_baseCurve->ClosestPointBoundedXY (spacePoint, &worldToLocal4d, detail))
            {
            size_t leafIndex;
            DPoint3d surfacePoint;
            double v;
            double u = detail.componentFraction;
            if (m_baseCurve->LeafToIndex (detail.curve, leafIndex))
                {
                DRay3d ray = DRay3d::FromOriginAndVector (detail.point, m_extrusionVector);
                ray.ProjectPointBounded (surfacePoint, v, spacePoint);
                UpdatePick (pickData, spacePoint, surfacePoint, u, v, 0, (int)leafIndex, (int)detail.componentIndex);
                }
            }
        }
        
    if (m_capped)
        {
        Transform capShift[2];
        capShift[0].InitIdentity ();
        capShift[1].InitFrom (m_extrusionVector);
        UpdateByAreaPick (pickData, m_baseCurve, spacePoint, capShift, 2);
        if (pickData.GetFaceIndices().IsCap0())
            ISolidPrimitive::ReverseFractionOrientation(pickData);
        }
    return HasRealPoint (pickData);   
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
static void RuledSurfaceClosestPoint_primtivePair
(
SolidLocationDetail &pickData,
size_t index0,
size_t &primitiveCounter,
ICurvePrimitiveCR curveA,
ICurvePrimitiveCR curveB,
DPoint3dCR spacePoint,
int parentId
)
    {
    DSegment3d segmentA, segmentB;
    DEllipse3d arcA, arcB;
    ICurvePrimitive::CurvePrimitiveType typeA = curveA.GetCurvePrimitiveType ();
    ICurvePrimitive::CurvePrimitiveType typeB = curveB.GetCurvePrimitiveType ();
    bvector<DPoint3d> const *pointsA;
    bvector<DPoint3d> const *pointsB;
    MSBsplineCurveCP bcurveA;
    MSBsplineCurveCP bcurveB;
    if (typeA != typeB)
        {
        // hmm.. shouldn't happend
        }
    else if (typeA == ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line
        && curveA.TryGetLine (segmentA)
        && curveB.TryGetLine (segmentB)
        )
        {
        DBilinearPatch3d patch (segmentA, segmentB);
        UpdatePatchPick (pickData, spacePoint, patch, (int)index0, (int)primitiveCounter);
        // um..  edges??
        }
    else if (typeA == ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc
                && curveA.TryGetArc (arcA)
                && curveB.TryGetArc (arcB)
        )
        {
        SolidLocationDetail pickData1;
        if (bsiDEllipse3d_ruledPatchClosestPoint (pickData1, spacePoint, arcA, arcB))
            UpdatePick (pickData, pickData1);
        }
    else if (typeA == ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString
            && NULL != (pointsA = curveA.GetLineStringCP ())
            && NULL != (pointsB = curveA.GetLineStringCP ())
            )
        {
        size_t nA = pointsA->size ();
        size_t nB = pointsA->size ();
        if (nA != nB || nA < 2)
            return;
        double df = 1.0 / (double)(nA-1);
        for (size_t i = 0; curveA.TryGetSegmentInLineString (segmentA, i)
                        && curveB.TryGetSegmentInLineString (segmentB, i); i++)
            {
            DBilinearPatch3d patch (segmentA, segmentB);
            UpdatePatchPick (pickData, spacePoint, patch, (int)index0, (int)primitiveCounter, (int) i, i * df, (i+1) * df);
            }
        }
    else if (   NULL != (bcurveA = curveA.GetProxyBsplineCurveCP ())
             && NULL != (bcurveB = curveB.GetProxyBsplineCurveCP ())
            )
        {
        SolidLocationDetail pickData1;
        if (MSBsplineCurve::RuledSurfaceClosestPoint (pickData1, *bcurveA, *bcurveB, spacePoint))
           UpdatePick (pickData, pickData1);
        }
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
static void RuledSurfaceClosestPoint_recurseToPrimitives
(
SolidLocationDetail &pickData,
size_t index0,
size_t &primitiveCounter,
CurveVectorCR curveA,
CurveVectorCR curveB,
DPoint3dCR spacePoint,
int parentId
)
    {
    if (curveA.size () != curveB.size ())
        return;
    for (size_t i = 0, n = curveA.size (); i < n; i++)
        {
        CurveVectorCP childA = curveA[i]->GetChildCurveVectorCP ();
        CurveVectorCP childB = curveB[i]->GetChildCurveVectorCP ();
        if (!childA && !childB)
            {
            RuledSurfaceClosestPoint_primtivePair (pickData, index0, primitiveCounter, *curveA[i], *curveB[i], spacePoint, parentId);
            primitiveCounter++;
            }
        else if (childA && childB)
            {
            RuledSurfaceClosestPoint_recurseToPrimitives
                (
                pickData, index0,
                primitiveCounter,
                *childA, *childB,
                spacePoint, parentId
                );
            }
        }
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool DgnRuledSweepDetail::ClosestPoint
(
DPoint3dCR spacePoint,
SolidLocationDetailR pickData
) const
    {
    if (m_sectionCurves.size () == 0)
        return false;
    InitClosestPoint (pickData);
    int parentId = 0;// Don't know what this was in rayIntersect
    for (size_t i = 0, n = m_sectionCurves.size (); i +1 < n; i++)
        {
        size_t index1 = 0;
        RuledSurfaceClosestPoint_recurseToPrimitives (pickData,
                i, index1,
                *m_sectionCurves[i],
                *m_sectionCurves[i + 1],
                spacePoint, parentId);
        }

    if (m_capped)
        {
        UpdateBySingleAreaPick (pickData, m_sectionCurves.front (), spacePoint, 0);
        UpdateBySingleAreaPick (pickData, m_sectionCurves.back (), spacePoint, 1);
        if (pickData.GetFaceIndices().IsCap0())
            ISolidPrimitive::ReverseFractionOrientation(pickData);
        }
    return true;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool DgnRotationalSweepDetail::ClosestPoint
(
DPoint3dCR spacePoint,
SolidLocationDetailR pickData
) const
    {
    Transform worldToLocal, localToWorld;
    InitClosestPoint (pickData);
    if (GetTransforms (localToWorld, worldToLocal))
        {
        DPoint3d localSpacePoint;
        worldToLocal.Multiply (localSpacePoint, spacePoint);
        double thetaPick = atan2 (localSpacePoint.y, localSpacePoint.x);
        double rhoPick = localSpacePoint.MagnitudeXY ();
        // ASSUME ... base curve is in xz plane.
        // Rotate pick point back to xz plane.  Proximity to base curve is proximity to surface.
        DPoint3d localXZPoint = DPoint3d::From (rhoPick, 0.0, localSpacePoint.z);
        DPoint3d worldXZPoint;
        localToWorld.Multiply (worldXZPoint, localXZPoint);
        CurveLocationDetail xzLocation;

        //Transform localPickToBase = Transform::From (RotMatrix::FromAxisAndRotationAngle (2, -thetaPick));
        //Transform worldPickToBase = Transform::FromProduct (localToWorld, localPickToBase, worldToLocal);

        Transform localBaseToPick = Transform::From (RotMatrix::FromAxisAndRotationAngle (2, thetaPick));
        Transform worldBaseToPick = Transform::FromProduct (localToWorld, localBaseToPick, worldToLocal);


        Transform localEndRotation = Transform::From (RotMatrix::FromAxisAndRotationAngle (2, m_sweepAngle));
        // Find closest base curve point ....
        if (m_baseCurve->ClosestPointBounded (worldXZPoint, xzLocation))
            {
            // rotate the base curve point back out to pick point.
            double v = Angle::NormalizeToSweep (thetaPick, 0.0, m_sweepAngle);
            DPoint3d surfacePoint;
            worldBaseToPick.Multiply (surfacePoint, xzLocation.point);
            size_t leafIndex;
            if (m_baseCurve->LeafToIndex (xzLocation.curve, leafIndex))
                UpdatePick (pickData, spacePoint, surfacePoint, xzLocation.componentFraction, v, 0, (int)leafIndex, (int)xzLocation.componentIndex);
            }

        if (HasRealCaps ())
            {
            Transform capTransforms[2];
            capTransforms[0] = Transform::FromIdentity ();
            capTransforms[1] = Transform::FromProduct (localToWorld, localEndRotation, worldToLocal);
            UpdateByAreaPick (pickData, m_baseCurve, spacePoint, capTransforms, 2);
            if (pickData.GetFaceIndices().IsCap0())
                ISolidPrimitive::ReverseFractionOrientation(pickData);
            }
        }
    return true;
    }

END_BENTLEY_GEOMETRY_NAMESPACE


