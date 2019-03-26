/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/CurvePrimitive/CurvePrimitivePair.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include "../SolidPrimitive/BoxFaces.h"
BEGIN_BENTLEY_GEOMETRY_NAMESPACE



/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
ICurvePrimitivePtr ICurvePrimitive::CreateInterpolationBetweenCurves
(
ICurvePrimitiveCR curveA,
double fraction,
ICurvePrimitiveCR curveB
)
    {
    DSegment3d segmentA, segmentB;
    if (curveA.TryGetLine (segmentA)
        && curveB.TryGetLine (segmentB))
        {
        DSegment3d segmentC = DSegment3d::From
            (
            DPoint3d::FromInterpolate (segmentA.point[0], fraction, segmentB.point[0]),
            DPoint3d::FromInterpolate (segmentA.point[1], fraction, segmentB.point[1])
            );
        return ICurvePrimitive::CreateLine (segmentC);
        }

    bvector<DPoint3d> const *pointsA;
    bvector<DPoint3d> const *pointsB;
    if (   NULL != (pointsA = curveA.GetLineStringCP ())
        && NULL != (pointsB = curveB.GetLineStringCP ())
        && pointsA->size () == pointsB->size ()
        )
        {
        bvector<DPoint3d>points;
        for (size_t i = 0; i < pointsA->size (); i++)
            points.push_back (DPoint3d::FromInterpolate (pointsA->at(i), fraction, pointsB->at(i)));
        return ICurvePrimitive::CreateLineString (points);
        }

    DEllipse3d ellipseA, ellipseB;
    if (curveA.TryGetArc (ellipseA)
        && curveB.TryGetArc (ellipseB))
        {
        if (Angle::NearlyEqual (ellipseA.sweep, ellipseB.sweep))
            {
            DEllipse3d ellipseB1 = ellipseB;
            if (!Angle::NearlyEqualAllowPeriodShift (ellipseA.start, ellipseB.start))
                ellipseB1 = DEllipse3d::FromRotatedAxes (ellipseB, ellipseA.start);
            DEllipse3d ellipseC = ellipseA;    // get the angles
            ellipseC.center   = DPoint3d::FromInterpolate (ellipseA.center, fraction, ellipseB1.center);
            ellipseC.vector0  = DVec3d::FromInterpolate (ellipseA.vector0,  fraction, ellipseB1.vector0);
            ellipseC.vector90 = DVec3d::FromInterpolate (ellipseA.vector90, fraction, ellipseB1.vector90);                
            return ICurvePrimitive::CreateArc (ellipseC);
            }
        }

    MSBsplineCurveCP bcurveA = curveA.GetBsplineCurveCP ();
    MSBsplineCurveCP bcurveB = curveB.GetBsplineCurveCP ();
    if (NULL != bcurveA && NULL != bcurveB)
        {
        MSBsplineCurvePtr curveC = MSBsplineCurve::CreateInterpolationBetweenCurves (*bcurveA, fraction, *bcurveB);
        if (curveC.IsValid ())
            {    
            return ICurvePrimitive::CreateBsplineCurve (*curveC);
            }
        }
    return ICurvePrimitivePtr ();
    }

#ifdef CompileAllCode

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
static bool TryUVFractionToXYZ_RuledSurface
(
ICurvePrimitiveCR curveA,
ICurvePrimitiveCR curveB,
double uFraction,
double vFraction,
bool extrapolateV,
DPoint3dR xyz,
DVec3dR dXdu,
DVec3dR dXdv
)
    {
    if (   DoubleOps::IsIn01 (uFraction)
        && (extrapolateV || DoubleOps::IsIn01 (vFraction)))
        {
        DPoint3d xyz0, xyz1;
        DVec3d dX0du, dX1du;
        curveA.FractionToPoint (uFraction, xyz0, dX0du);
        curveB.FractionToPoint (uFraction, xyz1, dX1du);
        xyz.Interpolate (xyz0, vFraction, xyz1);
        dXdu.Interpolate (dX0du, vFraction, dX1du);
        dXdv.DifferenceOf (xyz1, xyz0);
        return true;        
        }
    return false;
    }
#endif

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
static bool IsVectorMultiple(DVec3dCR vectorA, DVec3dCR vectorB, double scale)
    {
    DVec3d vectorB1 = vectorB;
    vectorB1.Scale (scale);
    return DVec3dOps::AlmostEqual (vectorA, vectorB1);
    }
//! @param [in] ellipse0 ellipse at v = 0
//! @param [in] ellipse1 ellipse at v = 1
//! @param [out] ellipseA larger ellipse
//! @param [out] vA v coordiante at larger ellipse.
//! @param [out] ellipseB smaller ellipse
//! @param [out] vB v coordinate at smaller ellipse.
//! @param [out] fractionB smaller ellipse as fraction of larger.
static bool OrientAsCone
(
DEllipse3dCR ellipse0,
DEllipse3dCR ellipse1,
DEllipse3dR ellipseA,
double      &vA,
DEllipse3dR ellipseB,
double      &vB,
double      &fractionB
)
    {
    static double s_coneFraction = 1.0e-8;
    DVec3d crossA, crossB;
    double mag0 = sqrt (crossA.GeometricMeanCrossProduct (ellipse0.vector0, ellipse0.vector90));
    double mag1 = sqrt (crossB.GeometricMeanCrossProduct (ellipse1.vector0, ellipse1.vector90));
    if (mag0 == 0.0 && mag1 == 0.0) // tolerance?
        return false;
    if (mag0 > mag1 && DoubleOps::SafeDivideParameter (fractionB, mag1, mag0, 0.0))
        {
        ellipseA = ellipse0;
        vA = 0.0;
        ellipseB = ellipse1;
        vB = 1.0;
        return mag1 < s_coneFraction * mag0
            || (  IsVectorMultiple (ellipseB.vector0, ellipse0.vector0, fractionB)
               && IsVectorMultiple (ellipseB.vector90, ellipse0.vector90, fractionB)
               );
        }
    else if (mag1 > mag0 && DoubleOps::SafeDivideParameter (fractionB, mag0, mag1, 0.0))
        {
        ellipseA = ellipse1;
        vA = 1.0;
        ellipseB = ellipse0;
        vB = 0.0;
        return mag1 < s_coneFraction * mag0
            || (  IsVectorMultiple (ellipseB.vector0,  ellipseA.vector0, fractionB)
               && IsVectorMultiple (ellipseB.vector90, ellipseA.vector90, fractionB)
               );
        }
    else    // exactly equal?  What if both are really small?
        {
        fractionB = 1.0;
        ellipseA = ellipse0;
        vA = 0.0;
        ellipseB = ellipse1;
        vB = 1.0;
        return    IsVectorMultiple (ellipse1.vector0,  ellipse0.vector0, fractionB)
               && IsVectorMultiple (ellipse1.vector90, ellipse0.vector90, fractionB)
               ;
        }
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
static bool GetConeFrame
(
DEllipse3dCR ellipseA,
DPoint3dCR pointB,
TransformR localToWorld,
TransformR worldToLocal
)
    {
    DVec3d zVector = DVec3d::FromStartEnd (ellipseA.center, pointB);
    localToWorld.InitFromOriginAndVectors (ellipseA.center, ellipseA.vector0, ellipseA.vector90, zVector);
    return worldToLocal.InverseOf (localToWorld);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
static void CheckSamePoint(DPoint3d xyz0, DPoint3d xyz1)
    {
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
static bool AddRayIntersections_RuledSurface
(
bvector<SolidLocationDetail> &pickData,
DEllipse3dCR ellipse0,
DEllipse3dCR ellipse1,
DRay3dCR worldRay
)
    {
    DEllipse3d ellipseA, ellipseB;
    double fractionB;
    double vA, vB;
    Transform localToWorld, worldToLocal;

    if (OrientAsCone (ellipse0, ellipse1, ellipseA, vA, ellipseB, vB, fractionB)
        && GetConeFrame (ellipseA, ellipseB.center, localToWorld, worldToLocal)
        )
        {
        DRay3d localRay;
        worldToLocal.Multiply (localRay, worldRay);
        // cone is u^2 + v^2 = (1 + z (fractionB - 1))^2
        //                   = (1 + (az + uz s)db)^2
        //                   = (1 + az db + s uz db)^2
        double db = fractionB - 1.0;
        Polynomial::Power::Degree2 quadratic;
        quadratic.AddSquaredLinearTerm (localRay.origin.x, localRay.direction.x);
        quadratic.AddSquaredLinearTerm (localRay.origin.y, localRay.direction.y);
        quadratic.AddSquaredLinearTerm (
                        1.0 + localRay.origin.z  * db,
                        localRay.direction.z * db,
                        -1.0);
        double ss[2];   // ray parameters of real roots.
        int numRoot = quadratic.RealRoots (ss);
        for (int i = 0; i < numRoot; i++)
            {
            DPoint3d localXYZOnRay = localRay.FractionParameterToPoint (ss[i]);
            double theta = bsiTrig_atan2 (localXYZOnRay.y, localXYZOnRay.x);
            DPoint3d worldXYZOnRay = worldRay.FractionParameterToPoint (ss[i]);
            if (Angle::InSweepAllowPeriodShift (theta, ellipseA.start, ellipseA.sweep)
                )
                {
                double uFraction = Angle::NormalizeToSweep (theta, ellipseA.start, ellipseA.sweep, false, true);
                double vFraction = DoubleOps::Interpolate (vA, localXYZOnRay.z, vB);
                DVec3d du0, du1, ddu;
                DPoint3d xyz0, xyz1;
                ellipse0.FractionParameterToDerivatives (xyz0, du0, ddu, uFraction);
                ellipse1.FractionParameterToDerivatives (xyz1, du1, ddu, uFraction);
                DVec3d uVector = DVec3d::FromInterpolate (du0, vFraction, du1);
                DVec3d vVector = DVec3d::FromStartEnd (xyz0, xyz1);
                DPoint3d xyzCheck = DPoint3d::FromInterpolate (xyz0, vFraction, xyz1);
                CheckSamePoint (xyzCheck, worldXYZOnRay);
                if (DoubleOps::IsIn01 (uFraction, vFraction))
                    {
                    SolidLocationDetail detail;
                    detail.SetPickParameter (ss[i]);
                    detail.SetXYZ (worldXYZOnRay);
                    detail.SetUV (uFraction, vFraction, uVector, vVector);
                    pickData.push_back (detail);
                    }
                }
            }
        return true;
        }
    return false;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
static bool AddRayIntersections_RuledSurface
(
bvector<SolidLocationDetail> &pickData,
MSBsplineCurveCR curveA,
MSBsplineCurveCR curveB,
DRay3dCR worldRay
)
    {
    return MSBsplineCurve::AddRuleSurfaceRayIntersections (pickData, curveA, curveB, worldRay);
    }


void AddHyperbolicParabolidRayIntersections
(
bvector<SolidLocationDetail> &pickData,
DPoint3dCR pointA0,
DPoint3dCR pointA1,
DPoint3dCR pointB0,
DPoint3dCR pointB1,
DRay3dCR   ray,
double     patchU0,
double     patchUStep,
ptrdiff_t  index2
)
    {
    DBilinearPatch3d patch (pointA0, pointA1, pointB0, pointB1);
    double hitParam[2];
    DPoint3d hitXYZ[2];
    DPoint2d hitUV[2];
    int numHit = ray.IntersectHyperbolicParaboloid (
            hitXYZ, hitParam, hitUV,
            pointA0, pointA1, pointB0, pointB1);
    for (int hit = 0; hit < numHit; hit++)
        {
        if (DoubleOps::IsIn01 (hitUV[hit]))
            {
            DPoint3d xyz;
            DVec3d dXdu, dXdv;
            patch.Evaluate (hitUV[hit].x, hitUV[hit].y, xyz, dXdu, dXdv);
            dXdu.Scale (1.0 / patchUStep);
            SolidLocationDetail detail;
            detail.SetPickParameter (hitParam[hit]);
            detail.SetXYZ (hitXYZ[hit]);
            detail.SetUV (patchU0 + patchUStep * hitUV[hit].x, hitUV[hit].y, dXdu, dXdv);
            detail.SetFaceIndices (0, 0, index2);
            pickData.push_back (detail);                
            }
        }
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool ICurvePrimitive::AddRuledSurfaceRayIntersections
    (
    bvector<SolidLocationDetail> &pickData,
    ICurvePrimitiveCR curveA,
    ICurvePrimitiveCR curveB,
    DRay3dCR ray
    )
    {

    DSegment3d segmentA, segmentB;
    if (curveA.TryGetLine (segmentA)
        && curveB.TryGetLine (segmentB))
        {
        AddHyperbolicParabolidRayIntersections (pickData,
              segmentA.point[0], segmentA.point[1], segmentB.point[0], segmentB.point[1],
              ray, 0.0, 1.0, 0);
        return true;
        }
    bvector<DPoint3d> const *pointsA;
    bvector<DPoint3d> const *pointsB;
    if (   NULL != (pointsA = curveA.GetLineStringCP ())
        && NULL != (pointsB = curveB.GetLineStringCP ())
        && pointsA->size () == pointsB->size ()
        && pointsA->size () > 1
       )
        {
        size_t numSegment = pointsA->size () - 1;
        //double delta = 1.0 / (double) numSegment;
        for (size_t i = 0; i < numSegment; i++)
            {
            AddHyperbolicParabolidRayIntersections (pickData,
                  pointsA->at(i), pointsA->at(i+1),
                  pointsB->at(i), pointsB->at(i+1),
                  ray,
                  0.0, 1.0, i);
                    // i * delta, delta // this is the "full linestring" parameter space ... but SolidLocationDetail wants by component.
            }
        return true;
        }


    DEllipse3d ellipseA, ellipseB;
    if (curveA.TryGetArc (ellipseA)
        && curveB.TryGetArc (ellipseB)
        && AddRayIntersections_RuledSurface (pickData, ellipseA, ellipseB, ray)
        )
        return true;


    // Can we use the curve's bspline directly, and not worry about parameterization?
    {
    MSBsplineCurveCP pbcurveA = curveA.GetBsplineCurveCP ();
    MSBsplineCurveCP pbcurveB = curveB.GetBsplineCurveCP ();
    if (NULL != pbcurveA && NULL != pbcurveB)
        {
        return AddRayIntersections_RuledSurface (pickData, *pbcurveA, *pbcurveB, ray);
        }
    }

    {
    // degenerate to bspline pair -- remap u parameters and derivatives in original curve parameterization
    size_t baseCount = pickData.size ();
    MSBsplineCurve bcurveA, bcurveB;
    bcurveA.Zero ();
    bcurveB.Zero ();
    if (   curveA.GetMSBsplineCurve (bcurveA, 0.0, 1.0)
        && curveB.GetMSBsplineCurve (bcurveB, 0.0, 1.0)
       )
        {
        if (AddRayIntersections_RuledSurface (pickData, bcurveA, bcurveB, ray))
            {
            for (size_t i = baseCount; i < pickData.size (); i++)
                {
                DPoint3d xyzA0, xyzB0;    // on splines A and B as evaluted at the spline fraction ...
                DPoint3d xyzA1, xyzB1;    // on original curves by projection of xyzA0, xyzB0
                DRay3d tangentA1, tangentB1;  // on primitives A and B at primitive fraction uA, uB;
                double uA1, uB1;        // fractions when xyzA0, xyzB0 are projected to their primitives.
                                        // !!! If these are not the same the curves are not really compatible for ruling
                double splineFraction = pickData[i].GetU ();
                double v = pickData[i].GetV ();
                bcurveA.FractionToPoint (xyzA0, splineFraction);
                bcurveB.FractionToPoint (xyzB0, splineFraction); 
                curveA.ClosestPointBounded (xyzA0, uA1, xyzA1);
                curveB.ClosestPointBounded (xyzB0, uB1, xyzB1);
                curveA.FractionToPoint (uA1, tangentA1);
                curveB.FractionToPoint (uB1, tangentB1);
                double u = DoubleOps::Interpolate (uA1, v, uB1);  // if uA1!=uA2 the parameterization is goofy...
                DVec3d dXdu = DVec3d::FromInterpolate (tangentA1.direction, v, tangentB1.direction);
                pickData[i].SetU (u);
                pickData[i].SetUDirection (dXdu);
                }
            }
        }

    bcurveA.ReleaseMem ();
    bcurveB.ReleaseMem ();
    }
    return false;
    }

END_BENTLEY_GEOMETRY_NAMESPACE
