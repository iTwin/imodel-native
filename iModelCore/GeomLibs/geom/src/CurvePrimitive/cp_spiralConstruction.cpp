/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE
// Functions for newton search for curvature match.
struct CurvatureSearcher : FunctionRRToRR
{
ICurvePrimitivePtr m_spiral;
NewtonIterationsRRToRR m_newton;
CurvatureSearcher (ICurvePrimitivePtr &spiral) : m_spiral(spiral), m_newton (1.0e-10)
    {

    }
ValidatedDouble SearchForCurvatureFraction (double curvatureTarget, double startFraction)
    {
    m_curvatureA = m_curvatureB = curvatureTarget;
    double maxStep = 0.1;
    double uA = startFraction;
    double uB = startFraction;
    if (m_newton.RunApproximateNewton (uA, uB, *this, maxStep, maxStep))
        return ValidatedDouble (uA, true);
    return ValidatedDouble (startFraction, false);
    }

ValidatedDouble SearchForCurvatureFractions (double curvatureTargetA, double curvatureTargetB,
    double startFractionA,
    double startFractionB,
    double &fractionA,
    double &fractionB
    )
    {
    m_curvatureA = curvatureTargetA;
    m_curvatureB = curvatureTargetB;
    double maxStep = 0.1;
    double uA = startFractionA;
    double uB = startFractionB;
    if (m_newton.RunApproximateNewton (uA, uB, *this, maxStep, maxStep))
        {
        fractionA = uA;
        fractionB = uB;
        return true;
        }
    fractionA = startFractionA;
    fractionB = startFractionB;
    return false;
    }

double m_curvatureA;
double m_curvatureB;

double EvaluateCurvature(double fraction)
    {
    DVec3d dX, ddX;
    DPoint3d X;
    m_spiral->FractionToPoint (fraction, X, dX, ddX);
    DVec3d cross = DVec3d::FromCrossProduct (dX, ddX);
    double a = dX.Magnitude ();
    double aaa = a * a * a;
    double k = cross.Magnitude () / aaa;
    if (cross.z < 0.0)
        k = -k;
    return k;
    }
bool EvaluateRRToRR(double fractionA, double fractionB, double &functionA, double &functionB)
    {
    functionA = EvaluateCurvature (fractionA) - m_curvatureA;
    if (fractionB == fractionA)
        functionB = functionA;
    else
        {
        functionB = EvaluateCurvature (fractionB) - m_curvatureB;
        }
    return true;
    }

};

// Construct a spiral with given curvature limits (both nonzero) and true length.
// The inflection is always at the origin.
// Start and endpoints are along the spiral as determined for the curvature targets.
ICurvePrimitivePtr CreateBasePartialSpiralWithPreciseBearingCurvatureLengthCurvature (int spiralType, double curvatureA, double curvatureB, double targetPartialLength)
    {
    double distanceTolBig = 1.0e-7 * targetPartialLength;
    double distanceTolSmall = 1.0e-12 * targetPartialLength;
    double maxLocalFraction = 2.0;      // when extrapolating from prior iterates, allow modest amount of out-of-interval step.   (default to length ratio if this fails)
    double curvature1, radius1, length1, fraction0;
    double bearing0 = 0.0;
    // (fraction0) is the approximate fraction for curvatureA
    // (fraction1, radius1, length1) are the construction parameters for the full spiral
    // radius1 is the smaller radius of the two requests.
    // The actual fractions for the partial spiral are determined by search for precise requested curvatures.
    if (fabs (curvatureA) > fabs (curvatureB))
        {
        fraction0 = curvatureB / curvatureA;
        curvature1 = curvatureA;
        radius1    = 1.0 / curvatureA;
        length1    = targetPartialLength * curvatureA / (curvatureA - curvatureB);
        }
    else
        {
        fraction0 = curvatureA / curvatureB;
        curvature1 = curvatureB;
        radius1    = 1.0 / curvatureB;
        length1    = targetPartialLength * curvatureB / (curvatureB - curvatureA);
        }
    if (!DSpiral2dBase::IsValidRLCombination (length1, radius1, spiralType))
        return nullptr;
    DPoint3d origin = DPoint3d::From (0,0,0);
    int numConverged = 0;
    bvector<DPoint2d> lengthHistory;  // tuple:  (length1, actualPartialLength-targetLength, length1-targetLength)
    // "A and B" are the fractional end positions.
    double fractionA = curvatureA / curvature1;
    double fractionB = curvatureB / curvature1;
    for (int count = 0 ;count < 15;count++)
        {
        if (fabs (length1) > fabs (2.0 * radius1))
            return nullptr;
        auto spiral = ICurvePrimitive::CreatePseudoSpiralPointBearingRadiusLengthRadius (spiralType, origin, bearing0, 0.0, length1, radius1, 0.0, 1.0);
        CurvatureSearcher searcher (spiral);
        if (!searcher.SearchForCurvatureFractions (curvatureA, curvatureB, curvatureA/curvature1, curvatureB/curvature1, fractionA, fractionB))
            return nullptr;
        spiral = ICurvePrimitive::CreatePseudoSpiralPointBearingRadiusLengthRadius (spiralType, origin, bearing0, 0.0, length1, radius1, fractionA, fractionB);
        double actualPartialLength;
        spiral->Length (actualPartialLength);
        lengthHistory.push_back (DPoint2d::From (length1, actualPartialLength - targetPartialLength));
        double error = actualPartialLength - targetPartialLength;
        if (fabs (error) < distanceTolBig)
            {
            numConverged++;
            if (numConverged > 1 || fabs (error) < distanceTolSmall)
                return spiral;
            }
        else
            numConverged = 0;
        bool stepComputed = false;
        if (lengthHistory.size () > 1)
            {
            size_t k = lengthHistory.size ();
            double f0 = lengthHistory[k-2].y;
            double f1 = lengthHistory[k-1].y;
            ValidatedDouble localFraction = DoubleOps::InverseInterpolate (f0, 0.0, f1);
            if (localFraction.IsValid () && fabs (localFraction - 0.5) < maxLocalFraction)
                {
                stepComputed = true;
                length1 = DoubleOps::Interpolate (lengthHistory[k-2].x, localFraction, lengthHistory[k-1].x);
                }
            }
        if (!stepComputed)
            length1 = length1 *(targetPartialLength/ actualPartialLength);
        }
    return nullptr;
    }
// Construct a spiral with given curvature limits (both nonzero) and true length.
// The inflection is always at the origin.
// Start and endpoints are along the spiral as determined for the curvature targets.
// Same parameters as CreateSpiralBearingRadiusLengthRadius (except no start and end fraction, since they are computed in the iteration)
ICurvePrimitivePtr ICurvePrimitive::CreatePseudolSpiralWithTrueRadiusLengthRadius (
      int spiralType,
      DPoint3dCR startPoint,
      double startRadians,
      double startRadius,
      double targetLength,
      double endRadius
      )
    {
    // RailCorp has its own special iteration.
    // This is a really good thing, because this generic iteration fails at the smooth transition at the endpoint !!!
    if (spiralType == DSpiral2dBase::TransitionType_AustralianRailCorp)
        return CreateAustralianRailCorpBearingRadiusLengthRadius (startPoint, startRadians, startRadius, targetLength, endRadius);

    double startCurvature, endCurvature;
    DoubleOps::SafeDivide (startCurvature, 1.0, startRadius, 0.0);
    DoubleOps::SafeDivide (endCurvature, 1.0, endRadius, 0.0);
    // This creates a spiral in xy plane, with inflection at origin.  Actual start point is "somewhere out in the first or forth quadrant" --
    //    we pull it back to the requested start and orientation.
    auto spiral = CreateBasePartialSpiralWithPreciseBearingCurvatureLengthCurvature (spiralType, startCurvature, endCurvature, targetLength);
    if (!spiral.IsValid ())
        return nullptr;
    auto placement = spiral->GetSpiralPlacementCP ();
    double signYZ = 1.0;
    if (placement->fractionA > placement->fractionB)
        signYZ = -1.0;
    Transform putdownFrame = Transform::From (RotMatrix::FromAxisAndRotationAngle (2, startRadians), startPoint);
    DPoint3d pointA, pointB;
    DVec3d unitA, unitB;
    if (!spiral->GetStartEnd (pointA, pointB, unitA, unitB))
        return nullptr;
    auto perpA = DVec3d::FromCCWPerpendicularXY (unitA);
    Transform pickupFrame = Transform::FromOriginAndVectors (pointA, unitA, signYZ * perpA, DVec3d::From (0,0,signYZ));
    auto pickupInverse = pickupFrame.ValidatedInverse ();
    Transform shift = putdownFrame * pickupInverse;
    if (spiral->TransformInPlace (shift))
        return spiral;
    return nullptr;
    }

struct NSWCompoundSolver
{
double m_R0;
double m_R1;
double m_s01;

// We are on an NSW spiral with compound radius transition.
// We know m_R0 = radius at start point.  We do NOT know its x0 value.
// We know m_R1 = radius at end point.  We do NOT know its x1 value.
// We know m_s01 = distance between, along the spiral
// We do not know s0 = distance from inflection to start.
//
// for an estimated s1 -- the total length of spiral form inflection to smallest radius -- 
// the NSW (aka AustralianRailCorp) spiral pings the final radius and length.
//
// But in the usual construction the distance from the larger radius back the inflection is unknown.
// So that additional distance (here, m_s0) is to be solved.
//
// Because of the remarkably good behavior at R1,L, the iteration involves only the one variable m_s0.
// For suggested m_s0, set up the entire spiral (from inflection to m_s0+m_s01) and compute the local radius
//  of curvature at distance m_s0.
//
// This is done using the RailCorp standard formula for R0 at distance m_s0.   This bypasses calculating the 
// various derivatives of the formulas for x and y, but seems very accurate.
//
// So the target function is    f(m_s0) = targetR0 - computedR0.
// The Newton iteration with approximate derivative converges nicely -- 3 digits at a time.
//
// NOTE: To make this iteration self contained, the various equations for RailCorp spiral are copied here.
//  they should match those in the TransitionSpiral_AustralianRailCorp class.
//
NSWCompoundSolver (double R0, double R1, double s01) : m_R0 (R0), m_R1 (R1), m_s01 (s01)
    {
    }
static double phiFromRX( double R, double xc )
{
    double phi, expr1, expr2, expr3;

    expr1 = ( 2. / sqrt( 3. ));
    expr2 = ( - ( 3. / 4. ) * sqrt( 3. ) * xc / R );
    if (expr2 < -1.0)
        expr2 = -1.0;
    if (expr2 > 1.0)
        expr2 = 1.0;
    expr3 = Angle::DegreesToRadians( 240. );

    phi = asin( expr1 * cos( acos( expr2 ) / 3. + expr3 ));

    return( phi );
}

static double radiusFromMX (double m, double x)
    {
    double xx = x * x;
    double q = 1.0 + 9.0 * m * m * xx * xx;
    return q * sqrt (q) / (6.0 * m * x);
    }
static double xFromMS (double m, double s)
    {
    double a1 = 0.9000;
    double a2 = 5.1750;
    double a3 = 43.1948;
    double a4 = 426.0564;
    double m2s4 =  m * m * s * s * s * s;
    double x = s * (1.0 - m2s4 * (a1 - m2s4 * (a2 - m2s4 * (a3 - m2s4 * a4))));
    return x;
    }

double x1FromLR( double L, double R )
{
    int idx = 0;
    double xc, l, m, phi;
    static double tolerance = 1.0e-5;

    xc = .7 * L;

    for( idx = 0; idx < 100; ++idx )
    {
        phi = phiFromRX( R, xc );
        double xc2 = xc * xc;
        m = tan( phi ) / ( 3.0 * xc2);
        double m2x4 = m * m * xc2 * xc2;
        double correction = xc * m2x4 * (
                 (     9. /   10. ) + m2x4 * (
               - (     9. /    8. ) + m2x4 * (
               + (   729. /  208. ) + m2x4 *
               - ( 32805. / 2176. ))));
        l = xc + correction;
        xc = ( L / l ) * xc;

        if( fabs( L - l ) < tolerance )
            break;
    }

    return( xc );
}

void ComputeCompoundFunction (double s0, double &fR0)
    {
    double s1 = s0 + m_s01; // distance from inflection to end.
    double x1 = x1FromLR (s1, m_R1);
    double phi = phiFromRX( m_R1, x1);
    double m = tan( phi ) / ( 3.0 * x1 * x1);
    double x0 = xFromMS (m, s0);
    double newR0 = radiusFromMX (m, x0);
    fR0 = newR0 - m_R0;
    }

double m_derivativeDelta;
double m_s0;
bool IsAtTrivialSolution ()
    {
    double a = DoubleOps::SmallCoordinateRelTol () * m_R1;
    return fabs (m_R0) <= a && fabs (m_s0) <= a;
    }
bool RunIteration ()
    {
    uint32_t numConverged = 0;
    double typicalLength = 100.0;
    double tightTol = 1.0e-14 * typicalLength;
    double looseTol = tightTol * 1000.0;
    for (uint32_t iteration = 0; iteration < 20; iteration++)
        {
        double fA, fB;
        ComputeCompoundFunction (m_s0, fA);
        ComputeCompoundFunction (m_s0 + m_derivativeDelta, fB);
        double deltaS0;
        if (!DoubleOps::SafeDivide (deltaS0, m_derivativeDelta * fA, fB - fA, 0.0))
            {
            return false;
            }
        else
            {
            //printf ("         (deltaS0 %10.3le)\n", deltaS0);
            double a = fabs (deltaS0);
            m_s0 -= deltaS0;
            if (a < tightTol)
                return true;
            if (a < looseTol)
                {
                numConverged++;
                if (numConverged > 1)
                    return true;
                }
            else
                {
                numConverged = 0;
                }
            }
        }
    return false;
    }

bool InitializeFreeVariables ()
    {
    double curvature0, curvature1;
    DoubleOps::SafeDivide (curvature0, 1.0, m_R0, 0.0);
    DoubleOps::SafeDivide (curvature1, 1.0, m_R1, 0.0);
    m_s0 = m_s01 * curvature0 / (curvature1 - curvature0);
    m_derivativeDelta = 1.0e-6 * ( fabs (m_s0) + fabs (m_s01));
    return true;
    }
bool GetCurrentSolution (double &s0)
    {
    s0 = m_s0;
    return true;
    }
};
/* Caller responsible for shuffling so startRadius > endRadius
* 1) startRadius is not infinite.
* 2) endRadius < startRadius
* 3) both radii positive
*/
static ICurvePrimitivePtr CreateAustralianRailCorpBearingRadiusLengthRadius_go
(
DPoint3d startPoint,
double bearingRadians0,
double startRadius,     // radius closer to inflection
double requestedLength, // length of active section
double endRadius,     // radius farther from inflection
bool   constructReversedSense,
bool reversedCurvature
)
    {
    if (endRadius >= startRadius && startRadius != 0.0)
        return nullptr;
    ICurvePrimitivePtr spiralA;
    NSWCompoundSolver solver (startRadius, endRadius, requestedLength);
    if (solver.InitializeFreeVariables ())
        {
        double s0;
        if (solver.IsAtTrivialSolution () || solver.RunIteration ())
            {
            solver.GetCurrentSolution (s0);
            double totalLength = s0 + requestedLength;
            double fraction0 = s0 / totalLength;
            double fraction1 = 1.0;
            if (constructReversedSense)
                {
                spiralA = ICurvePrimitive::CreateSpiralBearingRadiusLengthRadius (
                        DSpiral2dBase::TransitionType_AustralianRailCorp,
                        0.0,    // start bearing
                        0.0,    // start radius (inflection)
                        totalLength,
                        endRadius,
                        Transform::FromIdentity (),
                        fraction1,
                        fraction0);
                }
            else
                {
                spiralA = ICurvePrimitive::CreateSpiralBearingRadiusLengthRadius (
                        DSpiral2dBase::TransitionType_AustralianRailCorp,
                        0.0,    // start bearing
                        0.0,    // start radius (inflection)
                        totalLength,
                        endRadius,
                        Transform::FromIdentity (),
                        fraction0,
                        fraction1);
                }
            if (spiralA.IsValid ())
                {
                DPoint3d startA, endA;
                DVec3d startTangentA, endTangentA;
                spiralA->GetStartEnd (startA, endA, startTangentA, endTangentA);
                auto startPerp = DVec3d::FromUnitPerpendicularXY (startTangentA);
                auto zVector = DVec3d::From (0,0,1);
                if (reversedCurvature)
                    {
                    zVector = zVector * -1.0;
                    startPerp = startPerp * -1.0;
                    }
                auto frameA = Transform::FromOriginAndVectors (startA, startTangentA, startPerp, zVector);
                auto inverseA = frameA.ValidatedInverse ();
                auto frameB = Transform::From (RotMatrix::FromAxisAndRotationAngle (2, bearingRadians0), startPoint);
                if (!spiralA->TransformInPlace (frameB * inverseA.Value ()))
                    return nullptr;
                }
            }
        }
    return spiralA;
    }

// Construct a spiral with given curvature limits and radius, using the AustralianRailCorp (aka NewSouthWales) series expansions and radius expressions.
ICurvePrimitivePtr ICurvePrimitive::CreateAustralianRailCorpBearingRadiusLengthRadius (
      DPoint3dCR startPoint,
      double startRadians,
      double startRadius,
      double targetLength,
      double endRadius
      )
    {
    double absStartRadius = fabs (startRadius);
    double absEndRadius = fabs (endRadius);
    ICurvePrimitivePtr spiral;
    double k0, k1;
    DoubleOps::SafeDivide (k0, 1.0, absStartRadius, 0.0);
    DoubleOps::SafeDivide (k1, 1.0, absEndRadius, 0.0);
    double startSign = k0 * k1 >= 0.0 ? 1.0 : -1.0;
    if (k1 > k0)
        {
        spiral = CreateAustralianRailCorpBearingRadiusLengthRadius_go (startPoint, startRadians,
                startSign * absStartRadius,
                targetLength,
                absEndRadius,
                false,
                endRadius < 0.0
                );
        }
    else if (k0 > k1)
        {
        spiral = CreateAustralianRailCorpBearingRadiusLengthRadius_go (startPoint, startRadians,
                startSign * absEndRadius,
                targetLength,
                absStartRadius,
                true,
                startRadius > 0.0
                );
        }
    return spiral;
    }
bool ICurvePrimitive::CreateSpiralsStartShoulderTarget
(
    int transitionType,
    DPoint3dCR pointA,
    DPoint3dCR pointB,
    DPoint3dCR pointC,
    ICurvePrimitivePtr &primitiveA,
    ICurvePrimitivePtr &primitiveB
)
    {
    bool result = false;
    Transform localToWorld, worldToLocal;
    primitiveA = primitiveB = nullptr;
    if (localToWorld.InitNormalizedFrameFromOriginXPointYPoint(pointA, pointB, pointC))
        {
        worldToLocal.InvertRigidBodyTransformation(localToWorld);
        DPoint3d localA = DPoint3d::From(0, 0, 0);
        DPoint3d localB = worldToLocal * pointB;
        DPoint3d localC = worldToLocal * pointC;
        DPoint2d localP, localQ;
        DSpiral2dBase *spiralA = DSpiral2dBase::Create(transitionType);
        DSpiral2dBase *spiralB = DSpiral2dBase::Create(transitionType);
        if (DSpiral2dBase::SymmetricPointShoulderTargetTransition(
            DPoint2d::From(localA), DPoint2d::From(localB), DPoint2d::From(localC),
            *spiralA,
            *spiralB,
            localP, localQ))
            {
            DVec2d xVec = DVec2d::From(1.0, 0.0);

            Transform frameA = localToWorld * Transform::FromOriginAndXVector(DPoint2d::From(0, 0), xVec);
            Transform frameB = localToWorld * Transform::FromOriginAndXVector(localP, xVec);

            primitiveA = ICurvePrimitive::CreateSpiral(*spiralA, frameA, 0.0, 1.0);
            primitiveB = ICurvePrimitive::CreateSpiral(*spiralB, frameB, 0.0, 1.0);
            result = true;
            }
        delete spiralA;
        delete spiralB;
        }
    return result;
    }

CurveVectorPtr CurveVector::CreateSpiralLineToLineShift
(
    int transitionType,
    DPoint3dCR pointA,
    DPoint3dCR shoulderB,
    DPoint3dCR shoulderC,
    DPoint3dCR pointD
)
    {
    ICurvePrimitivePtr spiralA, spiralB, spiralC, spiralD;
    ICurvePrimitive::CreateSpiralsStartShoulderTarget(DSpiral2dBase::TransitionType_Clothoid, pointA, shoulderB, shoulderC, spiralA, spiralB);

    ICurvePrimitive::CreateSpiralsStartShoulderTarget(DSpiral2dBase::TransitionType_Clothoid, pointD, shoulderC, shoulderB, spiralD, spiralC);

    auto cv = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open);
    DPoint3d xyzB1 = pointA;
    if (spiralA.IsValid() && spiralB.IsValid())
        {
        cv->push_back(spiralA);
        cv->push_back(spiralB);
        spiralB->FractionToPoint(1.0, xyzB1);
        }


    DPoint3d xyzC0 = pointD;
    if (spiralC.IsValid() && spiralD.IsValid())
        {
        spiralC->ReverseCurvesInPlace();
        spiralD->ReverseCurvesInPlace();
        spiralC->FractionToPoint(0.0, xyzC0);
        }

    if (!xyzB1.AlmostEqual(xyzC0))
        cv->push_back(ICurvePrimitive::CreateLine(DSegment3d::From(xyzB1, xyzC0)));

    if (spiralC.IsValid() && spiralD.IsValid())
        {
        cv->push_back(spiralC);
        cv->push_back(spiralD);
        }

    return cv;
    }


CurveVectorPtr CurveVector::ConstructSpiralArcSpiralTransition
(
    DPoint3dCR xyz0,
    DPoint3dCR xyz1,
    DPoint3dCR xyz2,
    double arcRadius,
    double spiralLength
)
    {
    DSpiral2dBaseP spiralA = DSpiral2dBase::Create(DSpiral2dBase::TransitionType_Clothoid);
    DSpiral2dBaseP spiralB = DSpiral2dBase::Create(DSpiral2dBase::TransitionType_Clothoid);
    DPoint3d xyzA, xyzB, xyzC, xyzD;
    DEllipse3d arc;
    auto cv = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open);
    if (DSpiral2dBase::LineSpiralArcSpiralLineTransition(xyz0, xyz2, xyz1,
        arcRadius, spiralLength, spiralLength, *spiralA, *spiralB,
        xyzA, xyzB, xyzC, xyzD, arc
    ))
        {
        Transform frameA = Transform::From(xyzA);
        Transform frameB = Transform::From(xyzB);
        auto cpA = ICurvePrimitive::CreateSpiral(*spiralA, frameA, 0.0, 1.0);
        auto cpB = ICurvePrimitive::CreateSpiral(*spiralB, frameB, 1.0, 0.0);
        cv->push_back(cpA);
        cv->push_back(ICurvePrimitive::CreateArc(arc));
        cv->push_back(cpB);
        }
    delete spiralA;
    delete spiralB;
    return cv;
    }

struct Newton_TransitionTangentFunction : FunctionRToRD
    {
    DPoint3dCR m_xyz0;
    DPoint3dCR m_xyz1;
    DPoint3dCR m_xyz2;
    double m_radius;
    DPlane3d m_plane;

    Newton_TransitionTangentFunction(
        DPoint3dCR xyz0,      // prior point (possibly PI)
        DPoint3dCR xyz1,      // shoulder (PI) point
        DPoint3dCR xyz2,      // following point (possibly PI)
        double radius,              // arc radius
        DPlane3d plane    // plane that must contain start point.
    ) : m_xyz0(xyz0), m_xyz1(xyz1), m_xyz2(xyz2), m_radius(radius), m_plane(plane)
        {
        }

    ValidatedDouble EvaluateRToR(double spiralLength)
        {
        auto cv = CurveVector::ConstructSpiralArcSpiralTransition(m_xyz0, m_xyz1, m_xyz2, m_radius, spiralLength);
        if (cv.IsValid())
            {
            DPoint3d xyz0, xyz1;
            cv->GetStartEnd(xyz0, xyz1);
            return ValidatedDouble(m_plane.Evaluate(xyz0), true);
            }
        return ValidatedDouble(0.0, false);
        }
    bool EvaluateRToRD(double spiralLength, double &f, double &df) override
        {
        double step = 1.0e-4;
        auto f0 = EvaluateRToR(spiralLength);
        auto f1 = EvaluateRToR(spiralLength + step);
        if (f0.IsValid() && f1.IsValid())
            {
            f = f0.Value();
            df = (f1 - f0) / step;
            return true;
            }
        return false;
        }
    };

CurveVectorPtr CurveVector::ConstructSpiralArcSpiralTransitionPseudoOffset
(
    DPoint3dCR primaryPoint0,
    DPoint3dCR primaryPoint1,
    DPoint3dCR primaryPoint2,
    double primaryRadius,
    double primarySpiralLength,
    double offsetDistance
)
    {
    auto primaryCV = CurveVector::ConstructSpiralArcSpiralTransition(primaryPoint0, primaryPoint1, primaryPoint2,
        primaryRadius, primarySpiralLength);
    if (primaryCV.IsValid())
        {
        DPoint3d xyz0, xyz2;
        DVec3d tangent0, tangent2;
        primaryCV->GetStartEnd(xyz0, xyz2, tangent0, tangent2);
        bvector<DPoint3d> pi0, pi1;
        pi0.push_back(primaryPoint0);
        pi0.push_back(primaryPoint1);
        pi0.push_back(primaryPoint2);
        double offsetRadius = tangent0.CrossProductXY(tangent2) < 0.0 ? primaryRadius - offsetDistance : primaryRadius + offsetDistance;
        if (offsetRadius > 0.0)
            {
            PolylineOps::OffsetLineString(pi1, pi0, offsetDistance, DVec3d::UnitZ(), false, 2.0);
            if (pi1.size() == 3)
                {
                double offsetSpiralLength = primarySpiralLength * offsetRadius / primaryRadius;
                Newton_TransitionTangentFunction function(pi1[0], pi1[1], pi1[2], offsetRadius, DPlane3d::FromOriginAndNormal(xyz0, tangent0));
                NewtonIterationsRRToRR newton(1.0e-10);
                if (newton.RunNewton(offsetSpiralLength, function))
                    {
                    return ConstructSpiralArcSpiralTransition(pi1[0], pi1[1], pi1[2], offsetRadius, offsetSpiralLength);
                    }
                }
            }
        }
    return nullptr;
    }

END_BENTLEY_GEOMETRY_NAMESPACE
