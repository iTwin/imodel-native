/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
// This .h file is to be included directly into .cpp file of caller code.
// Put the include AFTER the main includes but outside any namespace blocks.
USING_NAMESPACE_BENTLEY_GEOMETRY_INTERNAL

//
// Static class with methods for constructing transition curves based on
// (point, tangent, radius) at start and end.
//
struct ConstructTransition_PointTangentRadius_PointTangentRadius
    {
    /*
    * Construct an XY plane curve that matches (pointA,tangentA,radiusA) and
    * (pointB,tangentB,radiusB)
    *
    * Specifically, there are 3 parts to the curve:
    *   1) an arc that starts at pointA with direction tangentA and radius radiusA
    *   2) a line (tangent to both arcs
    *   3) an arc that starts at pointB with direction tangentB and radius radiusB
    *
    * Z parts of all inputs are ignored.
    *
    * Note that both tangents are "foward" on the curve -- tangentA is "inbound" and tangentB is "outbound"
    */
    static CurveVectorPtr ConstructOnXYPlane
    (
        DPoint3dCR pointA,
        DVec3dCR directionA, // Depart from pointA in directionA
        double radiusA,
        DPoint3dCR pointB,
        DVec3dCR directionB, // Depart out of the curve) from pointB in directionB
        double radiusB
    )
        {
        DVec3d radiusVectorA, radiusVectorB;
        radiusVectorA.UnitPerpendicularXY(directionA);
        radiusVectorB.UnitPerpendicularXY(directionB);
        radiusVectorA.Scale(radiusA);
        radiusVectorB.Scale(radiusB);
        DPoint3d centerA = pointA - radiusVectorA;
        DPoint3d centerB = pointB - radiusVectorB;
        centerB.z = centerA.z;  // enforce single plane data !!
        double dAB = centerA.Distance(centerB);
        DVec3d vector0A = DVec3d::FromStartEndNormalize(centerA, centerB) * fabs(radiusA);
        DVec3d vector90A = DVec3d::FromCCWPerpendicularXY(vector0A);
        DVec3d vector0B = DVec3d::FromStartEndNormalize(centerA, centerB) * fabs(radiusB);
        DVec3d vector90B = DVec3d::FromCCWPerpendicularXY(vector0B);
        double radiansA = vector0A.AngleToXY(radiusVectorA);
        double radiansB = vector0B.AngleToXY(radiusVectorB);
        double q = Angle::PiOver2();
        double omegaA = q;
        double omegaB = q;
        // TODO:
        //   Let the radius signs control the vector directions -- each vector0 perpendicular to center-to-center
        //   Collapse these to alpha = asin ((radiusA-radiusB)/dAB)
        //   and all cases are just alpha
        if (radiusA > 0.0)
            {
            if (radiusB > 0.0)
                {
                double alpha = asin((fabs(radiusA) - fabs(radiusB)) / dAB);
                omegaA = q - alpha;
                omegaB = q - alpha;
                }
            else
                {
                double alpha = asin((fabs(radiusA) + fabs(radiusB)) / dAB);
                omegaA = q - alpha;
                omegaB = -q - alpha;
                }
            }
        else
            {
            if (radiusB > 0.0)
                {
                double alpha = asin((fabs(radiusA) + fabs(radiusB)) / dAB);
                omegaA = -q + alpha;
                omegaB = q + alpha;
                }
            else
                {
                double alpha = asin((fabs(radiusA) - fabs(radiusB)) / dAB);
                omegaA = -q + alpha;
                omegaB = -q + alpha;
                }
            }
        auto arcA = DEllipse3d::FromVectors(centerA, vector0A, vector90A, radiansA, omegaA - radiansA);
        auto arcB = DEllipse3d::FromVectors(centerB, vector0B, vector90B, omegaB, radiansB - omegaB);
        DPoint3d pointA1, pointB0;
        DVec3d   tangentA1, tangentB0, curlA1, curlB0;
        arcA.FractionParameterToDerivatives(pointA1, tangentA1, curlA1, 1.0);
        arcB.FractionParameterToDerivatives(pointB0, tangentB0, curlB0, 0.0);

        auto segment = DSegment3d::From(pointA1, pointB0);
        DVec3d segmentVector = DVec3d::FromStartEnd(pointA1, pointB0);
        if (tangentA1.DotProduct(segmentVector) < 0.0)
            arcA.ComplementSweep();
        if (tangentB0.DotProduct(segmentVector) < 0.0)
            arcB.ComplementSweep();
        auto path = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open);
        path->push_back(ICurvePrimitive::CreateArc(arcA));
        path->push_back(ICurvePrimitive::CreateLine(segment));
        path->push_back(ICurvePrimitive::CreateArc(arcB));
        return path;
#ifdef CreateAnnotation
        auto annotation = CurveVector::Create(CurveVector::BOUNDARY_TYPE_None);
        annotation->push_back(ICurvePrimitive::CreateLine(DSegment3d::From(centerA, centerB)));
        annotation->push_back(ICurvePrimitive::CreateLine(DSegment3d::From(centerA, centerA + radiusVectorA)));
        annotation->push_back(ICurvePrimitive::CreateLine(DSegment3d::From(centerB, centerB + radiusVectorB)));
        arcA.MakeFullSweep();
        arcB.MakeFullSweep();
        annotation->push_back(ICurvePrimitive::CreateArc(arcA));
        annotation->push_back(ICurvePrimitive::CreateArc(arcB));
        curves.push_back(annotation);
#endif
        }
    /*
    * Construct a planar curve (on a plane determined here) that that matches (pointA,tangentA,radiusA) and
    * (pointB,projectedTangentB,radiusB)
    * where projectedTangentB is the computed projection of tangentB on the plane of
    * pointA, pointB, and tangentA.
    *
    * All inputs are full 3D.
    *
    * The intended use is that this method is called twice once with (pointA, pointB) data order,
    * and then with the reverse order.
    * Each call matches 5 of the 6 input conditions exactly, and is close to the tangentB except for plane.
    */
    static CurveVectorPtr  ConstructOnPlaneFavoringFirstInput
    (
        DPoint3dCR pointA,
        DVec3dCR directionA, // Depart from pointA in directionA
        double radiusA,
        DPoint3dCR pointB,
        DVec3dCR directionB, // Depart (out of the curve) from pointB in directionB
        double radiusB
    )
        {
        // vector perpendicular to both directions
        DVec3d vectorAB = DVec3d::FromStartEnd(pointA, pointB);
        auto axes = RotMatrix::From2Vectors(directionA, vectorAB);
        if (!axes.SquareAndNormalizeColumns(axes, 0, 1))
            return nullptr;
        auto localToWorld = Transform::From(axes, pointA);
        Transform worldToLocal;
        worldToLocal.InvertRigidBodyTransformation(localToWorld);
        DPoint3d localPointA = worldToLocal * pointA;
        DPoint3d localPointB = worldToLocal * pointB;
        DVec3d localDirectionA = directionA * axes;  // vector on left is inverse
        DVec3d localDirectionB = directionB * axes;
        localDirectionB.z = 0.0;
        auto localCurves = ConstructOnXYPlane(
            localPointA, localDirectionA, radiusA,
            localPointB, localDirectionB, radiusB);
        if (localCurves.IsValid())
            {
            localCurves->TransformInPlace(localToWorld);
            return localCurves;
            }
        return nullptr;
        }

    /*---------------------------------------------------------------------------------**//**
    * Construct a linestring that has start point and tangent at start of curveA
    * and end point and tangent at end of curveB.
    * The blend is via the cubic bezier polynomials with control values (1,1,0,0) for curveA
    * and (0,0,1,1) for curveB.
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    static ICurvePrimitivePtr CubicBlendLineString(ICurvePrimitiveCR curveA, ICurvePrimitiveCR curveB, int numPoints)
        {
        DPoint3d pointA, pointB;
        bvector<DPoint3d> blendPoints;
        for (int i = 0; i <= numPoints; i++)
            {
            double f = (double)i / (double)numPoints;
            curveA.FractionToPoint(f, pointA);
            curveB.FractionToPoint(f, pointB);
            double g = 1.0 - f;
            // bezier cubic (1,1,0,0)
            double fA = g * g * (g + 3.0 *f);
            double fB = 1.0 - fA;
            blendPoints.push_back(DPoint3d::FromSumOf(pointA, fA, pointB, fB));
            }
        return ICurvePrimitive::CreateLineString(blendPoints);
        }
    /*---------------------------------------------------------------------------------**//**
    * Construct a linestring that has start point and tangent at start of curveA
    * and end point and tangent at end of curveB.
    * The blend is via the cubic bezier polynomials with control values (1,1,0,0) for curveA
    * and (0,0,1,1) for curveB.
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    static CurveVectorPtr ConstructMiddleCurveBlend
    (
        CurveVectorCR curveA,   // Curve with 3 curve primitives.
        CurveVectorCR curveB,   // path with 3 curve primitives.
        int numPoints)
        {
        auto blend = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open);
        if (curveA.size() == 3 && curveB.size() == 3)
            {
            blend->push_back(curveA[0]->Clone());
            blend->push_back(CubicBlendLineString(*curveA[1], *curveB[1], numPoints));
            blend->push_back(curveB[2]->Clone());
            }
        return blend;
        }

    /*---------------------------------------------------------------------------------**//**
    * Construct a curve which starts at pointA with directionA and radiusA,
    * and arrives at pointB with directionB and radiusB.
    * Both points and directions are full 3D -- not required to be coplanar.
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    static CurveVectorPtr ConstructFull3DBlend
    (
        DPoint3dCR pointA,    // start point
        DVec3dCR directionA, // Forward tangent (into curve) from pointA.
        double radiusA,      // start radius
        DPoint3dCR pointB,   // end point
        DVec3dCR directionB, // forward tangent (out of curve) at pointB.
        double radiusB
    )
        {
        CurveVectorPtr shortestCA = nullptr;
        double minLengthA = 1.0e20;
        CurveVectorPtr shortestCB = nullptr;
        double minLengthB = 1.0e20;
        for (DPoint2d signs : {
            DPoint2d::From(1, 1),
                DPoint2d::From(1, -1),
                DPoint2d::From(-1, 1),
                DPoint2d::From(-1, -1),
            })
            {
            auto cA = ConstructOnPlaneFavoringFirstInput(
                pointA, directionA, signs.x * radiusA,
                pointB, directionB, signs.y * radiusB);
            double lengthA = cA->Length();
            if (lengthA < minLengthA)
                {
                minLengthA = lengthA;
                shortestCA = cA;
                }

            auto cB = ConstructOnPlaneFavoringFirstInput(
                pointB, -1.0 * directionB, signs.x * radiusB,
                pointA, -1.0 * directionA, signs.y * radiusA);
            double lengthB = cB->Length();
            if (lengthB < minLengthB)
                {
                minLengthB = lengthB;
                shortestCB = cB;
                }
            }

            if (shortestCA.IsValid() && shortestCB.IsValid())
                {
                shortestCB->ReverseCurvesInPlace();
                auto cAB = ConstructMiddleCurveBlend(*shortestCA, *shortestCB, 17);
                return cAB;
                }
            return nullptr;
        }
    };
