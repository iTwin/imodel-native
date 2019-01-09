/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/bspline/MSBsplineCurve.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include <memory.h>
#include <stdlib.h>
#include <math.h>
#if defined (INCLUDE_PPL)
    #include <Bentley/Iota.h>
    #include <ppl.h>
    //#define USE_PPL
    #if !defined (USE_PPL)
        #include <algorithm>
    #endif
#endif
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

#include "msbsplinemaster.h"

#define MDLERR_NOPOLES ERROR
#define MDLERR_INSFMEMORY ERROR

// msbspline had various weight tolerances:
//      bspcurv_contiguousCurves uses fc_epsilon (0.00001)
static double sWeightTolerance = 1.0e-8;


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz             03/13
+---------------+---------------+---------------+---------------+---------------+------*/
void MSBsplineCurve::ExtractTo (MSBsplineCurveR dest)
    {
    dest = *this;
    Zero ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz             08/07
+---------------+---------------+---------------+---------------+---------------+------*/
void MSBsplineCurve::FractionToPoint (DPoint3dR xyz, double u) const
    {
    bspcurv_computeCurvePoint (&xyz, NULL, NULL, u,
                               poles,
                               params.order,
                               params.numPoles,
                               knots,
                               weights,
                               rational,
                               params.closed);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Peter.Yu                        04/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void MSBsplineCurve::FractionToPoints (bvector<DPoint3d>& points, size_t numPoints)
    {
    double knotA, knotB, h;
    GetKnotRange (knotA, knotB);
    
    
    points.resize (numPoints);
    if (numPoints == 1)
        FractionToPoint (points[0], knotA);
    else
        {
        h = (knotB - knotA)/(numPoints - 1);
        for (size_t i=0; i<numPoints; i++)
            FractionToPoint (points[i], knotA + i*h);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Peter.Yu                        04/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void MSBsplineCurve::FractionToPoints (bvector<DPoint3d>& points, bvector<double>& fractions)
    {
    size_t numPts = fractions.size ();
    points.resize (numPts);

    for (size_t i=0; i<numPts; i++)
        FractionToPoint (points[i], fractions[i]);
    }


// Newton iteration function for "closest approach between smooth curves".
struct BCurveApproachFunction : FunctionRRToRRD
{
MSBsplineCurveCP m_curveA;
MSBsplineCurveCP m_curveB;

BCurveApproachFunction (MSBsplineCurveCP curveA, MSBsplineCurveCP curveB)
    : m_curveA(curveA), m_curveB (curveB) {}

public:
// Virtual function
// @param [in] u  first variable
// @param [in] v  second variable
// @param [out]f  first function value
// @param [out]g  second function value
// @param [out]dfdu  derivative of f wrt u
// @param [out]dfdv  derivative of f wrt v
// @param [out]dgdu  derivative of g wrt u
// @param [out]dgdv  derivative of g wrt v
// @return true if function was evaluated.
bool EvaluateRRToRRD
(
double uA,
double uB,
double &f,
double &g,
double &dfdu,
double &dfdv,
double &dgdu,
double &dgdv
) override
    {
    DPoint3d pointA, pointB;
    DVec3d d1A, d1B, d2A, d2B;
    DVec3d dA[3], dB[3];
    if (    SUCCESS == m_curveA->ComputeDerivatives (dA, 2, uA)
        &&  SUCCESS == m_curveB->ComputeDerivatives (dB, 2, uB))
        {
        pointA = dA[0];
        pointB = dB[0];
        DVec3d chord = DVec3d::FromStartEnd (pointA, pointB);
        d1A = dA[1];
        d1B = dB[1];
        d2A = dA[2];
        d2B = dB[2];
        f = d1A.DotProduct (chord);
        dfdu = d2A.DotProduct (chord) - d1A.DotProduct (d1A);
        dfdv = d1A.DotProduct (d1B);
        g = d1B.DotProduct (chord);
        dgdu = - d1B.DotProduct (d1A);
        dgdv = d2B.DotProduct (chord) + d1B.DotProduct (d1B);
        return true;
        }
    return false;
    }

};
static double s_angleTol = 0.10;
static double s_paramTol = 1.0e-10;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BFP             06/90
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bsprcurv_extClosestBtwCurves
(
double          *distance,             /* <= closest distance between curves */
DPoint3d        *minPt0,               /* <= closest point on curve0 */
DPoint3d        *minPt1,               /* <= closest point on curve1 */
double          *param0,               /* <= fraction of point on curve0 */
double          *param1,               /* <= fraction of point on curve1 */
MSBsplineCurveCP curve0,
MSBsplineCurveCP curve1,
double          *tolerance,
double          maxDistance
)
    {
    bvector <DPoint3d> pointA, pointB;
    bvector <double>   paramA, paramB;
    curve0->AddStrokes (pointA, NULL, &paramA, CURVE_PARAMETER_MAPPING_CurveFraction, 0.0, s_angleTol, 0.0);
    curve1->AddStrokes (pointB, NULL, &paramB, CURVE_PARAMETER_MAPPING_CurveFraction, 0.0, s_angleTol, 0.0);
    CurveLocationDetail locationA, locationB;
    double uA, uB;
    // ASSUME closest approach of strokes is starting place for newton iteration ....
    if (PolylineOps::ClosestApproach (pointA, pointB, locationA, locationB)
        && locationA.Interpolate (paramA, uA)
        && locationB.Interpolate (paramB, uB))
        {
        BCurveApproachFunction newtonFunction (curve0, curve1);
        NewtonIterationsRRToRR newton (s_paramTol);
        newton.RunNewton (uA, uB, newtonFunction);
        DPoint3d outA, outB;

        curve0->FractionToPoint (outA, uA);
        curve1->FractionToPoint (outB, uB);

        if (NULL != distance)
            *distance = outA.Distance (outB);
        if (NULL != param0)
            *param0 = uA;
        if (NULL != param1)
            *param1 = uB;

        if (NULL != minPt0)
            *minPt0 = outA;
        if (NULL != minPt1)
            *minPt1 = outB;

        return SUCCESS;
        }
    return ERROR;
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz             08/07
+---------------+---------------+---------------+---------------+---------------+------*/
void MSBsplineCurve::FractionToPoint (DPoint3dR xyz, DVec3dR tangent, double u) const
    {
    bspcurv_computeCurvePoint (&xyz, &tangent, NULL, u,
                               poles,
                               params.order,
                               params.numPoles,
                               knots,
                               weights,
                               rational,
                               params.closed);
    double uMax, uMin;
    GetKnotRange (uMin, uMax);
    tangent.Scale (uMax - uMin);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz             08/07
+---------------+---------------+---------------+---------------+---------------+------*/
void MSBsplineCurve::FractionToPoint (DPoint3dR xyz, double &weight, DVec3dR tangent, double u) const
    {
    bspcurv_computeCurvePoint (&xyz, &tangent, &weight, u,
                               poles,
                               params.order,
                               params.numPoles,
                               knots,
                               weights,
                               rational,
                               params.closed);
    double uMax, uMin;
    GetKnotRange (uMin, uMax);
    tangent.Scale (uMax - uMin);
    }

void MSBsplineCurve::FractionToPoint (DPoint3dR xyz, DVec3dR dXYZ, DVec3dR ddXYZ, double u) const
    {
    DVec3d derivativeArray[3];
    ComputeDerivatives (derivativeArray, 2, u);
    xyz = derivativeArray[0];
    dXYZ = derivativeArray[1];
    ddXYZ = derivativeArray[2];
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz             08/07
+---------------+---------------+---------------+---------------+---------------+------*/
double MSBsplineCurve::FractionToKnot (double f) const
    {
    double knot0, knot1;
    GetKnotRange (knot0, knot1);
    return knot0 + f * (knot1 - knot0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz             08/07
+---------------+---------------+---------------+---------------+---------------+------*/
double MSBsplineCurve::KnotToFraction (double knot) const
    {
    double knot0, knot1;
    GetKnotRange (knot0, knot1);
    return (knot - knot0) / (knot1 - knot0);
    }


void MSBsplineCurve::MapFractions (bvector<double> *params, bvector<DVec3d> *derivatives, size_t i0, double knot0, double knot1, CurveParameterMapping select, MSBsplineCurveCP curve)
    {
    if (params == NULL && derivatives == NULL)
        return;
    // params start now in bezier fractions.
    double a0 = 0.0;
    double da = 1.0;

    if (select == CURVE_PARAMETER_MAPPING_CurveKnot)
        {
        a0 = knot0;
        da = knot1 - knot0;
        }
    else if (select == CURVE_PARAMETER_MAPPING_CurveFraction)
        {
        a0 = curve->KnotToFraction (knot0);
        da = curve->KnotToFraction (knot1) - a0;
        }

    if (params != NULL && (a0 != 0.0 || da != 1.0))
        {
        size_t i1 = params->size ();
        for (size_t i = i0; i < i1; i++)
            params->at (i) = a0 + params->at (i) * da;
        }

    if (derivatives != NULL && da != 1.0)
        {
        size_t i1 = derivatives->size ();
        double b = 1.0 / da;
        for (size_t i = i0; i < i1; i++)
            derivatives->at(i).Scale (b);
        }
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz             08/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool MSBsplineCurve::MapKnots
(
double a,
double b
)
    {
    double      start, end;
    GetKnotRange (start, end);
    double s;
    double tol = 1.0e-10;
    if (!DoubleOps::SafeDivide (s, b - a, end - start, 0.0))
        return false;
    int         numKnots = params.NumberAllocatedKnots ();
    for (int i = 0; i < numKnots; i++)
        {
        knots[i] = a + (knots[i] - start) * s;
        if (fabs (knots[i]) < tol)
            knots[i] = 0.0;
        else if (fabs (knots[i] - 1.0) < tol)
            knots[i] = 1.0;
        }
    return true;
    }


MSBsplineStatus  MSBsplineCurve::AddKnot (double unnormalizedKnotValue, int newMultiplicity)
    {
    double uA, uB;
    int    kA, kB;
    double tolerance;
    GetKnotRange (uA, uB, kA, kB, tolerance);
#ifdef bspknot_addKnot_takesFraction
    double fraction = KnotToFraction (unnormalizedKnotValue);
    return bspknot_addKnot (this, fraction, tolerance, newMultiplicity, false);
#else
    return bspknot_addKnot (this, unnormalizedKnotValue, tolerance, newMultiplicity, false);
#endif
    //return addKnot (*this, unnormalizedKnotValue, tolerance, newMultiplicity, false);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    earlin.lutz                     03/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void MSBsplineCurve::NormalizeKnots ()
    {
    bspknot_normalizeKnotVector (knots, params.numPoles, params.order, params.closed);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    peter.yu                        02/2009
+---------------+---------------+---------------+---------------+---------------+------*/
bool    MSBsplineCurve::IsPhysicallyClosed (double tolerance) const
    {
    return bspcurv_isPhysicallyClosedBCurve (this, tolerance);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    peter.yu                        02/2009
+---------------+---------------+---------------+---------------+---------------+------*/
bool    MSBsplineCurve::IsClosed () const
    {
    return (0 != params.closed);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    peter.yu                        02/2009
+---------------+---------------+---------------+---------------+---------------+------*/
MSBsplineStatus MSBsplineCurve::GetFrenetFrame (DVec3dP frame, DPoint3dR point, double& curvature, double& torsion, double u) const
    {
    return bspcurv_frenetFrame (frame, &point, &curvature, &torsion, this, u, NULL);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    peter.yu                        02/2009
+---------------+---------------+---------------+---------------+---------------+------*/
MSBsplineStatus MSBsplineCurve::GetFrenetFrame (TransformR frame, double u) const
    {
    DVec3d axes[3];
    DPoint3d point;
    double curvature, torsion;
    MSBsplineStatus stat = bspcurv_frenetFrame (axes, &point, &curvature, &torsion, this, u, NULL);
    frame.InitFromOriginAndVectors (point, axes[0], axes[1], axes[2]);
    return stat;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    peter.yu                        02/2009
+---------------+---------------+---------------+---------------+---------------+------*/
MSBsplineStatus MSBsplineCurve::ComputeDerivatives (DVec3dP dervs, int numDervs, double u) const
    {
    if(numDervs >= MAX_ORDER - 1)
        return ERROR;

    int     i, k, status;
    double  dw[MAX_ORDER]; /* must contain numDervs+1 doubles */

    if (SUCCESS != (status = bspcurv_computeDerivatives (dervs, dw, this, numDervs, u, false)))
        return status;

    if (rational)
        {
        DVec3d  pt;
        double a = 1.0 /dw[0];
        dervs[0].Scale (a);
        for (k=1; k<=numDervs; k++)
            {
            pt.Zero ();
            for (i=1; i<=k; i++)
                pt.SumOf (pt, dervs[k-i], dw[i] * bsiBezier_getBinomialCoefficient (i, k));
            dervs[k].DifferenceOf (dervs[k], pt);
            dervs[k].Scale (a);
            }
        }

    return SUCCESS;
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    peter.yu                        02/2009
+---------------+---------------+---------------+---------------+---------------+------*/
MSBsplineStatus MSBsplineCurve::ExtractSegmentBetweenKnots (MSBsplineCurveR target, double unnormalizedKnotA, double unnormalizedKnotB)
    {
    return bspcurv_segmentCurve (&target, this, KnotToFraction (unnormalizedKnotA), KnotToFraction (unnormalizedKnotB));
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    peter.yu                        02/2009
+---------------+---------------+---------------+---------------+---------------+------*/
MSBsplineStatus MSBsplineCurve::AppendCurve (MSBsplineCurveCR inCurve)
    {
    return bspcurv_appendCurves (this, this, const_cast<MSBsplineCurveP>(&inCurve), false, false);
    }

bool MSBsplineCurve::IsSameGeometry (MSBsplineCurveCR other) const
    {
    if (rational != other.rational)
        return false;
    if (params.numPoles != other.params.numPoles)
        return false;
    if (params.closed != other.params.closed)
        return false;
    if (params.order != other.params.order)
        return false;
    for (int i = 0; i < params.numPoles; i++)
        if (!poles[i].IsEqual (other.poles[i]))
            return false;
    int numKnots = NumberAllocatedKnots();
    for (int i = 0; i < numKnots; i++)
        if (knots[i] != other.knots[i])
            return false;

    if (rational)
        for (int i = 0; i < params.numPoles; i++)
            if (weights[i] != other.weights[i])
                return false;

    return true;
    }




/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    peter.yu                        02/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void MSBsplineCurve::ExtractEndPoints (DPoint3dR start, DPoint3dR end) const
    {
    bspcurv_extractEndPoints (&start, &end, this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    peter.yu                        02/2009
+---------------+---------------+---------------+---------------+---------------+------*/
MSBsplineStatus MSBsplineCurve::ExtractCurveNormal (DVec3dR normal, DPoint3dR position, double &planarDeviation) const
    {
    return bspcurv_extractNormal (&normal, &position, &planarDeviation, this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz                        02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void MSBsplineCurve::GetC1DiscontinuousFractions (bvector<double>& fractions) const
    {
    static double s_cuspRelTol = 1.0e-8;
    BCurveSegment segment;
    fractions.clear ();
    DPoint3d xyzA, xyzB;
    DVec3d tangentA, tangentB;
    xyzA.Zero ();
    tangentA.Zero ();
    double angleTol = Angle::SmallAngle ();
    double uA = 0.0;
    size_t numSegment = 0;
    // "fractions" is loaded with knot values and renormalized afterwards ..
    for (size_t i = 0; AdvanceToBezier (segment, i, true);)
        {
        bool isD1AtStart = false;
        double tangentZeroTol = s_cuspRelTol * segment.PolygonLength ();
        if (numSegment == 0)
            {
            isD1AtStart = true;
            }
        else
            {
            segment.FractionToPoint (xyzB, tangentB, 0.0, false);

            if (!xyzA.AlmostEqual (xyzB))
                isD1AtStart = true;
            else if (tangentA.AngleTo (tangentB) > angleTol)
                isD1AtStart = true;
            else if (tangentA.Magnitude () < tangentZeroTol
                  || tangentB.Magnitude () < tangentZeroTol
                  )
                isD1AtStart = true;

            }
        if (isD1AtStart)
            fractions.push_back (segment.FractionToKnot (0.0));
        uA = segment.FractionToKnot (1.0);
        segment.FractionToPoint (xyzA, tangentA, 1.0, false);
        numSegment++;
        }
        
    if (fractions.size () > 0)
        fractions.push_back (uA);
    double knotA, knotB;        
    GetKnotRange (knotA, knotB);
    double knotFactor;
    DoubleOps::SafeDivide (knotFactor, 1.0, knotB - knotA, 1.0);
    for (size_t i = 0; i < fractions.size (); i++)
        {
        fractions[i] = knotA + knotFactor * (fractions[i] - knotA);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz                        02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void MSBsplineCurve::GetC1DiscontinuousCurves (bvector<double>& fractions, bvector<MSBsplineCurvePtr> &curves) const
    {
    GetC1DiscontinuousFractions (fractions);
    for (size_t i = 0; i + 1 < fractions.size (); i++)
        {
        MSBsplineCurve segment;
        segment.CopyFractionSegment (*this, fractions[i], fractions[i+1]);
        curves.push_back (segment.MSBsplineCurve::CreateCapture ());
        }
    }




/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    peter.yu                        02/2009
+---------------+---------------+---------------+---------------+---------------+------*/
double MSBsplineCurve::PolygonLength () const
    {
    return PolylineOps::Length (poles,
                    rational ? weights : NULL,
                    1,
                    (size_t)params.numPoles,
                    params.closed != 0
                    );
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    earlin.lutz                     03/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void MSBsplineCurve::GetPoleRange (DRange3dR range) const
    {
    range.InitFrom (poles, weights, params.numPoles);
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz             08/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool MSBsplineCurve::AreSameWeights (double wA, double wB)
    {
    return fabs (wA - wB) < sWeightTolerance;
    }

bool MSBsplineCurve::AreSameKnots (double wA, double wB)
    {
    return bsiBezier_isNullKnotInterval (wA, wB);
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz             08/07
+---------------+---------------+---------------+---------------+---------------+------*/
MSBsplineCurve::KnotPosition MSBsplineCurve::SearchKnot
(
double unnormalizedKnotValue,
int &k0,
int &k1,
double &correctedKnotValue
) const
    {
    int order = params.order;
    int numKnots = params.NumberAllocatedKnots ();
    double uMin, uMax, uTolerance;
    double u = unnormalizedKnotValue;
    double u0, u1;
    int    indexMin, indexMax;
    GetKnotRange (uMin, uMax, indexMin, indexMax, uTolerance);

    // This should be the same assignments, but do it to be convincing ..
    uMin = knots[indexMin];
    uMax = knots[indexMax];

    if (u <= uMin + uTolerance)
        {
        correctedKnotValue = uMin;
        // Find the multiplicity of the start knot ...
        k0 = k1 = order - 1;
        u0 = uMin - uTolerance;
        while (k0 - 1 >= 0 && knots[k0] >= u0)
            k0--;
        return (u < u0) ?
            KNOTPOS_BEFORE_START : KNOTPOS_START;
        }

   if (u > uMax - uTolerance)
        {
        correctedKnotValue = uMax;
        k0 = k1 = numKnots - order;
        u1 = uMax + uTolerance;
        while (k1 + 1 < numKnots && knots[k1] < u1)
            k1++;
        return (u > u1) ?
            KNOTPOS_AFTER_FINAL : KNOTPOS_FINAL;
        }

   /* find interval or match */
    u0 = u - uTolerance;
    u1 = u + uTolerance;
    int kStart = order - 1;
    int kFinal = numKnots - order;
    /*for (k0 = kFinal; k0 > kStart && knots[k0] > u1; k0--)
        {}*/
    k0 = kStart;
    while (k0 < kFinal)
        {
        if (u0 > knots[k0] && u1 < knots[k0+1])
            break;
        if (knots[k0] > u0 && knots[k0] < u1)
            break;
        k0++;
        }

    if (k0 == kFinal)
        {
        // Can't happen unless knots are scrambled?
        k1 = numKnots - 1;
        correctedKnotValue = uMax;
        return KNOTPOS_AFTER_FINAL;
        }

    if (knots[k0] >= u0)
        {
        // Match at k0.
        correctedKnotValue = knots[k0];
        u1 = knots[k0] + uTolerance;
        for (k1 = k0; k1 + 1 < kFinal && knots[k1 + 1] <= u1; k1++)
            {}
        return KNOTPOS_INTERIOR;
        }

    // Walk over multiple knots at k0, look if u is just above ...
    k1 = k0;
    double u2 = knots[k0] + uTolerance;
    while (k1 + 1 < kFinal && knots[k1 + 1] <= u2)
        k1++;
    if (knots[k1] > u0)
        {
        correctedKnotValue = knots[k0];
        return KNOTPOS_INTERIOR;
        }

    if (knots[k1+1] > u1)
        {
        // Strictly between k1, k1+1
        k0 = k1;
        k1++;
        correctedKnotValue = u;
        return KNOTPOS_INTERVAL;
        }

    // u is within tolerance of knots[k1+1]
    k0 = k1;
    u2 = knots[k0] + uTolerance;
    while (k1 + 1 < kFinal && knots[k1 + 1] < uTolerance)
        k1++;
    correctedKnotValue = knots[k0];
    return KNOTPOS_INTERIOR;
    }

bool MSBsplineCurve::IsParabola
(
TransformR localToWorld,
TransformR worldToLocal,
double &vertexFraction,
DPoint3dR localStart,
DPoint3dR localEnd,
double &xSquaredCoefficient
) const
    {
    worldToLocal.InitIdentity ();
    localToWorld.InitIdentity ();
    vertexFraction = xSquaredCoefficient = 0.0;
    localStart.Zero ();
    localEnd.Zero ();
    if (GetOrder () != 3)
        return false;
    if (GetNumPoles () != 3)
        return false;
    DRange1d weightRange = GetWeightRange ();
    if (!AreSameWeights (weightRange.Low (), weightRange.High ()))
        return false;
    DPoint3d point0 = GetUnWeightedPole (0);
    DPoint3d point1 = GetUnWeightedPole (1);
    DPoint3d point2 = GetUnWeightedPole (2);
    DVec3d   delta01 = DVec3d::FromStartEnd (point0, point1);
    DVec3d   delta12 = DVec3d::FromStartEnd (point1, point2);
    DVec3d   delta012 = DVec3d::FromStartEnd (delta01, delta12);

    // The curve's tangent vector is a linear function of fraction parameter and the two first differences:
    // tangent = (1-u) * delta01 + u * delta12;
    // The tangent vector is perpendicular to the next difference at the vertex:
    // tangent.delta012 = (1-u)*delta01.DotProduct (delta012) + u*delta12.DotProduct (delta012) = 0
    // Solve for the fraction u:
    double  a01 = delta01.DotProduct (delta012);
    double  a12 = delta12.DotProduct (delta012);
    if (!DoubleOps::SafeDivide (vertexFraction, -a01, a12 - a01, 0.0))
        return false;
    double u0 = 1.0 - vertexFraction;
    double u1 = vertexFraction;        
    DPoint3d origin = DPoint3d::FromSumOf (point0, u0 * u0, point1, 2.0 * u0 * u1, point2, u1 * u1);
    DVec3d   tangent = DVec3d::FromSumOf (delta01, u0, delta12, u1);
    // The coordinate system at the vertex has x in the tangent direction.
    // The delta012 vector is in the positive-y half plane.
    // (Actually perpendicular, because of the vertex condition.)
    if (!localToWorld.InitFromOriginXVectorYVectorSquareAndNormalize (origin, tangent, delta012))
        return false;
    worldToLocal.InvertRigidBodyTransformation (localToWorld);
    worldToLocal.Multiply (localStart, point0);
    worldToLocal.Multiply (localEnd, point2);
    if (fabs (localStart.x) > fabs (localEnd.x))
        return DoubleOps::SafeDivide (xSquaredCoefficient, localStart.y, localStart.x * localStart.x, 0.0);
    else 
        return DoubleOps::SafeDivide (xSquaredCoefficient, localEnd.y, localEnd.x * localEnd.x, 0.0);
    }

END_BENTLEY_GEOMETRY_NAMESPACE