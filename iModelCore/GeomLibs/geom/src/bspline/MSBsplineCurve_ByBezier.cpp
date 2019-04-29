/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include <memory.h>
#include <stdlib.h>
#include <math.h>
#include "../DeprecatedFunctions.h"
#if defined (INCLUDE_PPL)
    #include <Bentley\Iota.h>
    #include <ppl.h>
    //#define USE_PPL
    #if !defined (USE_PPL)
        #include <algorithm>
    #endif
#endif
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

#include "msbsplinemaster.h"
#define MAX_STROKES_PER_BEZIER 100
//! Get poles for a single bezier poles from the curve.
//! @param [inout] poles On input, {order} poles over the knots, e.g. as extracted from MSBsplineCurve by GetSupport(..).
//!    On output, revised for clamped knots.
//! @param [inout] work vector for knots.
//! @return false if invalide bezierSelect.  Note that the bezierSelect for a high multiplicity knot returns true for the function
//!    but marks the interval as null.
bool MSBsplineCurve::GetBezier (BCurveSegment& segment, size_t bezierSelect) const
    {
    size_t order = (int)params.order;

    if (order > 1 && GetSupport (segment, bezierSelect))
        {
        segment.SaturateKnots ();
        return true;
        }

    return false;
    }

//! Move forward looking for a bezier segment of the curve.  Skip over nulls.
//! @param segment returned bezier
//! param [in,out] bezierSelect on input, first bezier to examine.
//!     On output, this is advanced to the "next" bezier to examine.
//! @return false if no bezier found.
bool MSBsplineCurve::AdvanceToBezier(BCurveSegment& segment, size_t &bezierSelect, bool saturateKnots) const
    {
    size_t order = (int)params.order;
    if (order > 1)
        {
        while (GetSupport (segment, bezierSelect))
            {
            bezierSelect++;
            if (!segment.IsNullU ())
                {
                if (saturateKnots)
                    segment.SaturateKnots ();
                return true;
                }
            }
        }
    return false;
    }
//! Move forward looking for a bezier segment that overlaps a fraction interval.  Skip over nulls and
//!  "earlier" knot intervals.
//! @param segment returned bezier, subdivided to match a portion of the knot interval.
//! @param [in,out] bezierSelect on input, first bezier to examine.
//!     On output, this is advanced to the "next" bezier to examine.
//! @param [in] interval live fraction interval.
//! @return false if no bezier found.
bool MSBsplineCurve::AdvanceToBezierInFractionInterval
(
BCurveSegment& segment,
size_t &bezierSelect,
DRange1dCR interval
) const
    {
    double knotA = FractionToKnot (interval.low);
    double knotB = FractionToKnot (interval.high);
    return AdvanceToBezierInKnotInterval (segment, bezierSelect,
                DRange1d::From (knotA, knotB));
    }

//! Move forward looking for a bezier segment that overlaps a knot interval.  Skip over nulls and
//!  "earlier" knot intervals.
//! @param segment returned bezier, subdivided to match a portion of the knot interval.
//! @param [in,out] bezierSelect on input, first bezier to examine.
//!     On output, this is advanced to the "next" bezier to examine.
//! @param [in] interval live knot interval.
//! @return false if no bezier found.
bool MSBsplineCurve::AdvanceToBezierInKnotInterval
(
BCurveSegment& segment,
size_t &bezierSelect,
DRange1dCR interval
) const
    {
    size_t order = (int)params.order;
    if (order > 1)
        {
        while (GetSupport (segment, bezierSelect))
            {
            bezierSelect++;
            if (segment.IsNullU ())
                {
                }
            else if (segment.UMax () <= interval.low)
                {
                }
            else if (segment.UMin () >= interval.high)
                {
                // We've gone by the interval.
                return false;
                }
            else
                {
                segment.SaturateKnots ();
                segment.SubdivideToIntersection (interval);
                return true;
                }
            }
        }
    return false;
    }


bool MSBsplineCurve::GetSupport
(
BCurveSegmentR segment,
size_t bezierSelect
) const
    {
    return bspcurv_getSupport (*this, segment, bezierSelect);
    }


// final bezier index plus 1...
size_t MSBsplineCurve::GetTailBezierSelect () const
    {
    int index = params.numPoles - params.order + 1;
    if (index < 0)
        index = 0;
    return (size_t)index;
    }

bool MSBsplineCurve::RetreatToBezierInKnotInterval (
BCurveSegment& segment,
size_t &bezierSelect,
DRange1dCR interval
) const
    {
    // predecrement .....
    size_t order = (int)params.order;
    if (order > 1)
        {
        while (bezierSelect > 0 && GetSupport (segment, --bezierSelect))
            {
            if (segment.IsNullU ())
                {
                }
            else if (segment.UMin () >= interval.high)
                {
                // skip this bezier.
                }
            else if (segment.UMax () <= interval.low)
                {
                // We've gone by the interval.
                return false;
                }
            else
                {
                segment.SaturateKnots ();
                segment.SubdivideToIntersection (interval);
                return true;
                }
            }
        }
    return false;
    }



#if defined (INCLUDE_PPL)
/*=================================================================================**//**
* @bsiclass                                     Sam.Wilson                      06/2010
+===============+===============+===============+===============+===============+======*/
struct          ClosestPointResults
{
    double      aMin;
    double      bestKnot;
    DPoint4d    bestXYZW;
    BCurveSegment segment;  // Very expensive to construct! 
    ClosestPointResults () : aMin(DBL_MAX) {;}

    static ClosestPointResults Combine (ClosestPointResults const& r1, ClosestPointResults const& r2)
        {
        return (r1.aMin < r2.aMin)? r1: r2;
        }
}; // ClosestPointResults

/*---------------------------------------------------------------------------------**//**
* Quick way to compute #Bezier segments will cover a BSpline.
* This is needed in order to set up a parallel for loop.
* @bsimethod                                    Sam.Wilson                      06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
inline size_t getBezierCount_only_for_open_curves (MSBsplineCurve const& curve)
    {
    // *** NEEDS WORK: Not correct for closed curves.
    return curve.params.numPoles - curve.params.order + 1;
    }

/*---------------------------------------------------------------------------------**//**
* This version of ClosestPoint can be built to use parallel_for.
* @bsimethod                                    Earlin.Lutz             08/07
+---------------+---------------+---------------+---------------+---------------+------*/
void MSBsplineCurve::ClosestPoint (DPoint3dR curvePoint, double &fraction, DPoint3dCR spacePoint) const
    {    
    Concurrency::combinable<ClosestPointResults> results;
    // *** NEEDS WORK: Not correct for closed curves.
    size_t spanCount = getBezierCount_only_for_open_curves (*this);
    Iota counter (spanCount);
#if defined (USE_PPL)
    // NB: Do not declare BCurveSegment outside the loop and then use it in the lambda. Each task/thread must have its own copy! That is why BCurveSegment is in combinable<ClosestPointResults>.
    Concurrency::parallel_for_each
#else
    std::for_each 
#endif
        (counter.begin(), counter.end(), [this,&spacePoint,&results](size_t spanIndex)
        {
        ClosestPointResults& result = results.local ();
        BCurveSegment& segment = result.segment;
        GetBezier (segment, spanIndex);
        if (!segment.IsNullU ())
            {
            double currFraction;
            double a;
            DPoint4d currXYZW;
            if (bsiBezierDPoint4d_closestPoint (&currXYZW, &currFraction, &a, segment.GetPoleP (), (int)segment.GetOrder (),
                        spacePoint.x, spacePoint.y, spacePoint.z, 0.0, 1.0)
                && a < result.aMin)
                {
                result.aMin = a;
                result.bestXYZW = currXYZW;
                result.bestKnot = segment.FractionToKnot (currFraction);
                }
            }
        }
        );

    ClosestPointResults best = results.combine (ClosestPointResults::Combine);      

    fraction = KnotToFraction(best.bestKnot);
    best.bestXYZW.GetProjectedXYZ (curvePoint);
    }
#else
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz             08/07
+---------------+---------------+---------------+---------------+---------------+------*/
void MSBsplineCurve::ClosestPoint (DPoint3dR curvePoint, double &fraction, DPoint3dCR spacePoint) const
    {    
    BCurveSegment segment;
    double currFraction;
    double bestKnot = 0.0;
    double a, aMin = DBL_MAX;
    DPoint4d currXYZW, bestXYZW;
    for (size_t spanIndex = 0; GetBezier (segment, spanIndex); spanIndex++)
        {
         if (!segment.IsNullU ())
            {
            if (bsiBezierDPoint4d_closestPoint (&currXYZW, &currFraction, &a, segment.GetPoleP (), (int)segment.GetOrder (),
                        spacePoint.x, spacePoint.y, spacePoint.z, 0.0, 1.0)
                && a < aMin)
                {
                aMin = a;
                bestXYZW = currXYZW;
                bestKnot = segment.FractionToKnot (currFraction);
                }
            }
        }
    fraction = KnotToFraction(bestKnot);
    bestXYZW.GetProjectedXYZ (curvePoint);
    }
#endif


void MSBsplineCurve::AddPlaneIntersections (
bvector<DPoint3d>* outputPoints,
bvector<double> *outputFractions,
DPlane3dCR plane
) const
    {
    DPoint4d planeCoffs;
    plane.GetDPoint4d (planeCoffs);
    AddPlaneIntersections (outputPoints, outputFractions, planeCoffs);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz             08/07
+---------------+---------------+---------------+---------------+---------------+------*/
void MSBsplineCurve::AddPlaneIntersections (
bvector<DPoint3d>* outputPoints,
bvector<double> *outputFractions,
DPoint4dCR planeCoffs
) const
    {    
    BCurveSegment segment;
    DPoint4d intersectionPoints[MAX_ORDER];
    double  intersectionFractions[MAX_ORDER];
    int numIntersection;

    for (size_t spanIndex = 0; GetBezier (segment, spanIndex); spanIndex++)
        {
         if (!segment.IsNullU ())
            {
            bsiBezierDPoint4d_allDPlane4dIntersections (intersectionFractions, intersectionPoints, &numIntersection, MAX_ORDER,
                    segment.GetPoleP (), params.order, &planeCoffs, 3, false);
            for (size_t i = 0; i < (size_t)numIntersection; i++)
                {
                if (NULL != outputFractions)
                    outputFractions->push_back (KnotToFraction (segment.FractionToKnot (intersectionFractions[i])));

                if (NULL != outputPoints)
                    {
                    DPoint3d xyz;
                    intersectionPoints[i].GetProjectedXYZ (xyz);
                    outputPoints->push_back (xyz);
                    }
                }
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz             08/07
+---------------+---------------+---------------+---------------+---------------+------*/
void MSBsplineCurve::AddArcIntersectionsXY
(
bvector<DPoint3d> *curvePoints,
bvector<double> *curveFractions,
bvector<DPoint3d> *ellipsePoints,
bvector<double> *ellipseFractions,
DEllipse3dCR ellipse,
bool extendConic,
DMatrix4dCP matrix
) const
    {    
    BCurveSegment segment;
    BCurveSegment segmentXY;
    double  conicAngles[2 * MAX_ORDER];
    double  bezierFractions[2 * MAX_ORDER];
    int numIntersection;
    DConic4d conic, conicA;
    bsiDConic4d_initFromDEllipse3d (&conic, &ellipse);
    conicA = conic;
    if (matrix)
        bsiDConic4d_applyDMatrix4d (&conicA, matrix, &conic);
    for (size_t spanIndex = 0; GetBezier (segment, spanIndex); spanIndex++)
        {
         if (!segment.IsNullU ())
            {
            if (matrix)
                segmentXY.CopyFrom (segment, matrix);
            bsiBezierDPoint4d_intersectDConic4dXYExt (NULL, bezierFractions, 
                    NULL, conicAngles, &numIntersection, MAX_ORDER,
                    matrix == NULL ? segment.GetPoleP () : segmentXY.GetPoleP (),
                    params.order,
                    &conicA, extendConic ? true : false);
            for (size_t i = 0; i < (size_t)numIntersection; i++)
                {
                DPoint3d xyz;
                if (NULL != curveFractions)
                    curveFractions->push_back (KnotToFraction (segment.FractionToKnot (bezierFractions[i])));
                if (NULL != curvePoints)
                    curvePoints->push_back (segment.FractionToPoint (bezierFractions[i]));

                if (NULL != ellipseFractions)
                    ellipseFractions->push_back (bsiDConic4d_angleParameterToFraction (&conic, conicAngles[i]));
                if (NULL != ellipsePoints)
                    {
                    bsiDConic4d_angleParameterToDPoint3d (&conic, &xyz, conicAngles[i]);
                    ellipsePoints->push_back (xyz);
                    }
                }
            }
        }
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
//static double s_angleTol = 0.10;
static double s_paramTol = 1.0e-10;

static bool ImproveCloseApproach
(
MSBsplineCurveCR curveA,
MSBsplineCurveCR curveB,
CurveLocationDetailR locationA,
CurveLocationDetailR locationB,
double maxDist
)
    {
    BCurveApproachFunction func (&curveA, &curveB);
    NewtonIterationsRRToRR newton (s_paramTol);
    double fractionA = locationA.fraction;
    double fractionB = locationB.fraction;
    if (newton.RunNewton (fractionA, fractionB, func))
        {
        locationA.fraction = fractionA;
        locationB.fraction = fractionB;
        curveA.FractionToPoint (locationA.point, fractionA);
        curveB.FractionToPoint (locationB.point, fractionB);
        if (locationA.point.Distance (locationB.point) <= maxDist)
            return true;
        }
    return false;
    }


void MSBsplineCurve::AddCurveIntersectionsXY
(
bvector<DPoint3d> *curveAPoints,
bvector<double> *curveAFractions,
bvector<DPoint3d> *curveBPoints,
bvector<double> *curveBFractions,
MSBsplineCurveCR curveB,
DMatrix4dCP matrix
) const
    {
    AddCurveIntersectionsXY (
            curveAPoints, curveAFractions, nullptr,
            curveBPoints, curveBFractions, nullptr,
            curveB, matrix);
    }

#define MAX_BEZIER_BEZIER_INTERSECTION 100
void MSBsplineCurve::AddCurveIntersectionsXY
(
bvector<DPoint3d> *curveAPoints,
bvector<double> *curveAFractions,
bvector<DSegment1d> *curveAOverlapFractions,
bvector<DPoint3d> *curveBPoints,
bvector<double> *curveBFractions,
bvector<DSegment1d> *curveBOverlapFractions,
MSBsplineCurveCR curveB,
DMatrix4dCP matrix
) const
    {
    static bool s_useNullMatrixBranch = false;
    if (s_useNullMatrixBranch && NULL == matrix)
        {
        bvector<DPoint3d> strokeA, strokeB;
        bvector<double>    fractionA, fractionB;
        static double s_approachRelTol = 1.0e-3;
        static double s_approachRelTol2 = 1.0e-4;
        this->AddStrokes (strokeA, NULL, &fractionA, 0.0, 0.15, 0.0, true, CURVE_PARAMETER_MAPPING_CurveFraction);
        curveB.AddStrokes (strokeB, NULL, &fractionB, 0.0, 0.15, 0.0, true, CURVE_PARAMETER_MAPPING_CurveFraction);
        DRange3d rangeA = DPoint3dOps::Range (&strokeA);
        DRange3d rangeB = DPoint3dOps::Range (&strokeB);
        DRange3d rangeAB;
        rangeAB.UnionOf (rangeA, rangeB);
        
        double approachFilterTol = s_approachRelTol * rangeAB.low.Distance (rangeAB.high);
        double approachTol = s_approachRelTol2 * approachFilterTol;
        bvector <CurveLocationDetail> locationA, locationB;
        for (size_t i = 0; i < strokeA.size (); i++)
            strokeA[i].z = 0.0;
        for (size_t i = 0; i < strokeB.size (); i++)
            strokeB[i].z = 0.0;
        PolylineOps::AddCloseApproaches (strokeA, &fractionA, strokeB, &fractionB, locationA, locationB, approachFilterTol);
        for (size_t i = 0; i < locationA.size (); i++)
            {
            if (ImproveCloseApproach (*this, curveB, locationA[i], locationB[i], approachTol))
                {
                // Don't enter the same knot values twice ..
                // We'd like to look in curveAFractions and curveBFractions but they might be NULL pointers.
                bool skip = false;
                for (size_t k = 0; k < i && !skip; k++)
                    {
                    if (   MSBsplineCurve::AreSameKnots (locationA[k].fraction, locationA[i].fraction)
                        && MSBsplineCurve::AreSameKnots (locationB[k].fraction, locationB[i].fraction)
                        )
                        skip = true;
                    }
                if (!skip)
                    {
                    DPoint3dOps::Append (curveAPoints, locationA[i].point);
                    DPoint3dOps::Append (curveBPoints, locationB[i].point);
                    DoubleOps::Append (curveAFractions, locationA[i].fraction);
                    DoubleOps::Append (curveBFractions, locationB[i].fraction);
                    }
                }
            }
        }
    else
        {
        BCurveSegment segmentA, segmentB;
        BCurveSegment segmentAXY, segmentBXY;
        double  fractionA[MAX_BEZIER_BEZIER_INTERSECTION];
        double  fractionB[MAX_BEZIER_BEZIER_INTERSECTION];
        int numIntersection, numExtraIntersection;
        double xyzTol =
            DoubleOps::Max (
                    DPoint3dOps::Tolerance (poles, params.numPoles, Angle::SmallAngle (), Angle::SmallAngle ()),
                    DPoint3dOps::Tolerance (poles, params.numPoles, Angle::SmallAngle (), Angle::SmallAngle ())
                    );
        double weightTol = Angle::SmallAngle ();

        for (size_t spanIndexA = 0; GetBezier (segmentA, spanIndexA); spanIndexA++)
            {
            if (segmentA.IsNullU ())
                continue;
            segmentAXY.CopyFrom (segmentA, matrix);
            for (size_t spanIndexB = 0; curveB.GetBezier (segmentB, spanIndexB); spanIndexB++)
                {
                if (segmentB.IsNullU ())
                    continue;
                segmentBXY.CopyFrom (segmentB, matrix);
                if (segmentA.GetOrder () == segmentB.GetOrder ()
                    && DPoint4d::AlmostEqual (
                        segmentAXY.GetPoleP (),
                        segmentBXY.GetPoleP (), (int)segmentB.GetOrder (),
                        xyzTol, weightTol)
                        )
                        {
                        if (nullptr != curveAOverlapFractions)
                            curveAOverlapFractions->push_back (
                                    DSegment1d (KnotToFraction (segmentA.FractionToKnot (0.0)),
                                                KnotToFraction (segmentA.FractionToKnot (1.0)))
                                    );
                        if (nullptr != curveBOverlapFractions)
                            curveBOverlapFractions->push_back (
                                    DSegment1d (curveB.KnotToFraction (segmentB.FractionToKnot (0.0)),
                                                curveB.KnotToFraction (segmentB.FractionToKnot (1.0)))
                                    );
                        }
                else if (segmentA.GetOrder () == segmentB.GetOrder ()
                    && DPoint4d::AlmostEqualReversed (
                        segmentAXY.GetPoleP (),
                        segmentBXY.GetPoleP (), (int)segmentB.GetOrder (),
                        xyzTol, weightTol)
                        )
                        {
                        if (nullptr != curveAOverlapFractions)
                            curveAOverlapFractions->push_back (
                                    DSegment1d (KnotToFraction (segmentA.FractionToKnot (0.0)),
                                                KnotToFraction (segmentA.FractionToKnot (1.0)))
                                    );
                        if (nullptr != curveBOverlapFractions)
                            curveBOverlapFractions->push_back (
                                    DSegment1d (curveB.KnotToFraction (segmentB.FractionToKnot (1.0)),
                                                curveB.KnotToFraction (segmentB.FractionToKnot (0.0)))
                                    );
                        }
                else if (bsiBezierDPoint4d_intersectXY_chordal (NULL, fractionA, NULL, fractionB,
                        &numIntersection, &numExtraIntersection, MAX_BEZIER_BEZIER_INTERSECTION,
                        segmentAXY.GetPoleP (), (int)segmentA.GetOrder (),
                        segmentBXY.GetPoleP (), (int)segmentB.GetOrder ()
                        ))
                    {
                    for (size_t i = 0; i < (size_t)numIntersection; i++)
                        {
                        if (NULL != curveAPoints)
                            curveAPoints->push_back (segmentA.FractionToPoint (fractionA[i]));
                        if (NULL != curveBPoints)
                            curveBPoints->push_back (segmentB.FractionToPoint (fractionB[i]));
                        if (NULL != curveAFractions)
                            curveAFractions->push_back (
                                KnotToFraction (segmentA.FractionToKnot (fractionA[i])));
                        if (NULL != curveBFractions)
                            curveBFractions->push_back (
                                curveB.KnotToFraction (segmentB.FractionToKnot (fractionB[i])));
                        }

                    }
                }
            }
        }
    }
void MSBsplineCurve::AddLinestringIntersectionsXY
(
bvector<DPoint3d> *curveAPoints,
bvector<double> *curveAFractions,
bvector<DPoint3d> *linestringPoints,
bvector<double> *linestringFractions,
bvector<DPoint3d> const &linestring,
DMatrix4dCP matrix
) const
    {
    return AddLinestringIntersectionsXY (
          curveAPoints, curveAFractions,
          linestringPoints, linestringFractions,
          linestring,
          false,
          matrix
          );
    }

void MSBsplineCurve::AddLinestringIntersectionsXY
(
bvector<DPoint3d> *curveAPoints,
bvector<double> *curveAFractions,
bvector<DPoint3d> *linestringPoints,
bvector<double> *linestringFractions,
bvector<DPoint3d> const &linestring,
bool extendLinestring,
DMatrix4dCP matrix
) const
    {
    BCurveSegment segmentA, segmentAXY;
    double  fractionA[MAX_BEZIER_CURVE_ORDER];
    double  fractionB[MAX_BEZIER_CURVE_ORDER];
    DPoint4d pointA[MAX_BEZIER_CURVE_ORDER];

    size_t numIntersection;
    DSegment3d segmentB;
    DSegment4d segmentBH;
    size_t numB = linestring.size ();

    for (size_t spanIndex = 0; GetBezier (segmentA, spanIndex); spanIndex++)
        {
        if (segmentA.IsNullU ())
            continue;
        DPoint4dP poles = segmentA.GetPoleP ();
        size_t order = (int)GetOrder ();
        if (NULL != matrix)
            {
            segmentAXY.CopyFrom (segmentA, matrix);
            poles = segmentAXY.GetPoleP ();
            }

        for (size_t iB = 1; iB < numB; iB++)
            {
            segmentB.point[0] = linestring[iB-1];
            segmentB.point[1] = linestring[iB];
            segmentBH.Init (segmentB.point[0], segmentB.point[1]);
            if (NULL != matrix)
                segmentBH.InitProduct (*matrix, segmentBH);
                
            bsiBezierDPoint4d_intersectDSegment4dXY (
                    pointA, fractionA, NULL, fractionB,
                    numIntersection, MAX_BEZIER_CURVE_ORDER,
                    NULL == matrix ? segmentA.GetPoleP () : segmentAXY.GetPoleP (),
                    order, segmentBH,
                    extendLinestring && iB == 1,      // extend backwards from erste segment.
                    extendLinestring && iB + 1 == numB  // extend forwards from last segment.
                    );

            // overwrite transformed evaluations if needed ..
            if (NULL != matrix && NULL != curveAPoints)
                bsiBezierDPoint4d_evaluateDPoint4dArrayExt (pointA, NULL, NULL,
                                segmentA.GetPoleP (), (int)order, fractionA, (int)numIntersection);

            for (size_t i = 0; i < numIntersection; i++)
                {
                DPoint3d xyzA, xyzB;
                if (!pointA[i].GetProjectedXYZ (xyzA))
                    continue;
                if (NULL != curveAPoints)
                    curveAPoints->push_back (xyzA);
                if (NULL != linestringPoints)
                    {
                    segmentB.FractionParameterToPoint (xyzB, fractionB[i]);
                    linestringPoints->push_back (xyzB);
                    }
                if (NULL != curveAFractions)
                    curveAFractions->push_back (
                        KnotToFraction (segmentA.FractionToKnot (fractionA[i])));
                if (NULL != linestringFractions)
                    linestringFractions->push_back (
                            PolylineOps::SegmentFractionToPolylineFraction (iB - 1, numB - 1, fractionB[i]));
                }
            }
        }
    }



void MSBsplineCurve::ClosestPointXY (DPoint3dR curvePoint, double &fraction, double &xyDistance, DPoint3dCR spacePoint, DMatrix4dCP worldToView) const
    {    
    BCurveSegment segment;
    double currFraction;
    double bestKnot = 0.0;
    double a, aMin = DBL_MAX;
    DPoint3d xyzView;
    DPoint4d currXYZWLocal;
    DPoint3d closePointOnBezier;
    closePointOnBezier.Zero ();
    if (NULL == worldToView)
        xyzView = spacePoint;
    else
        bsiDMatrix4d_multiplyAndRenormalizeDPoint3dArray (worldToView, &xyzView, &spacePoint, 1);

    for (size_t spanIndex = 0; GetBezier (segment, spanIndex); spanIndex++)
        {
         if (!segment.IsNullU ())
            {
            if (NULL != worldToView)
                bsiDMatrix4d_multiply4dPoints (worldToView, segment.GetWorkPoleP (0), segment.GetPoleP (), (int)segment.GetOrder ());
            if (bsiBezierDPoint4d_closestXYPoint (&currXYZWLocal, &currFraction, &a,
                            nullptr == worldToView ? segment.GetPoleP (0) : segment.GetWorkPoleP (0),
                            (int)segment.GetOrder (),
                            xyzView.x, xyzView.y, 0.0, 1.0)
                && a < aMin
                )
                {
                aMin = a;
                segment.FractionToPoint (closePointOnBezier, currFraction);
                bestKnot = segment.FractionToKnot (currFraction);
                }
            }
        }
    fraction = KnotToFraction(bestKnot);
    curvePoint = closePointOnBezier;
    xyDistance = sqrt (aMin);
    }

struct BCurveSegmentLengthIntegrator
{
BSIQuadraturePoints m_gaussRule;
static const uint32_t s_maxGauss = 20;
static const uint32_t s_maxPreSets = 7;
// 0,1,2 are full, left half, right half, then 4 quarters.
double m_u[10][s_maxGauss], m_w[10][s_maxGauss];
int m_numGauss;
int m_maxDepth;
UsageSums m_recursionDepth;
void BuildGaussWeights (uint32_t intervalSelect, double a, double b)
    {
    BeAssert (intervalSelect < s_maxPreSets);
    if (intervalSelect > s_maxPreSets)
        intervalSelect = 0;      // should never happen
    for (int i = 0; i < m_numGauss; i++)
        m_gaussRule.GetEval (i, a, b, m_u[intervalSelect][i], m_w[intervalSelect][i]);
    }

BCurveSegmentLengthIntegrator (int numGauss, int maxDepth)
    {
    m_maxDepth = maxDepth;
    if (m_maxDepth > 10)
        m_maxDepth = 10;
    m_gaussRule.InitGauss (numGauss > 0 ? numGauss : 3);
    m_numGauss = m_gaussRule.GetNumEval ();
    BuildGaussWeights (0, 0.0, 1.0);
    BuildGaussWeights (1, 0.0, 0.5);
    BuildGaussWeights (2, 0.5, 1.0);
    for (uint32_t i = 0; i < 4; i++)
        BuildGaussWeights (3 + i, i * 0.25, (i+1) * 0.25);
    }


double SumGaussRule (double a0, double a1, BCurveSegment &segment, RotMatrixCP matrix)
    {
    DVec3d curveTangent;
    DPoint3d xyzCurve;
    double a = 0.0;
    for (int i = 0; i < m_numGauss; i++)
        {
        double u, w;
        m_gaussRule.GetEval (i, a0, a1, u, w);
        segment.FractionToPoint (xyzCurve, curveTangent, u, false);
        if (nullptr != matrix)
            matrix->Multiply (curveTangent);
        a += w * curveTangent.Magnitude ();
        }
    return a;
    }


double SumGaussRule (uint32_t intervalSelect, BCurveSegment &segment, RotMatrixCP matrix)
    {
    if (intervalSelect > s_maxPreSets)
        intervalSelect = 0;      // should never happen
    DVec3d curveTangent;
    DPoint3d xyzCurve;
    double a = 0.0;
    for (int i = 0; i < m_numGauss; i++)
        {
        segment.FractionToPoint (xyzCurve, curveTangent, m_u[intervalSelect][i], false);
        if (nullptr != matrix)
            matrix->Multiply (curveTangent);
        a += m_w[intervalSelect][i] * curveTangent.Magnitude ();
        }
    return a;
    }
double SumGaussRules (uint32_t intervalSelectBegin, uint32_t intervalSelectEnd, BCurveSegment &segment, RotMatrixCP matrix)
    {
    double a = 0.0;
    for (uint32_t intervalSelect = intervalSelectBegin; intervalSelect < intervalSelectEnd; intervalSelect++)
        a += SumGaussRule (intervalSelect, segment, matrix);
    return a;
    }
double RecursiveSum (double a0, double a1, BCurveSegment segment, RotMatrixCP matrix, int depth, double parentSum, double absTol)
    {
    double aMid = 0.5 * (a0 + a1);
    double left = SumGaussRule (a0, aMid, segment, matrix);
    double right = SumGaussRule (aMid, a1, segment, matrix);
    double sum = left + right;
    double e = fabs (sum - parentSum);
    if (depth >= m_maxDepth || e <= absTol)
        {
        m_recursionDepth.Accumulate ((double)depth);
        return sum;
        }
    return  RecursiveSum (a0, aMid, segment, matrix, depth + 1, left, absTol)
          + RecursiveSum (aMid, a1, segment, matrix, depth + 1, right, absTol);
    }
};


#ifdef IntermediateVersion_doBCurveLength
static double doBCurveLength (MSBsplineCurveCR curve, RotMatrixCP matrix)
    {
    static double s_relTol = 1.0e-10;
    static int s_gaussPoints = 5;
    BSIQuadraturePoints gaussRule;
    gaussRule.InitGauss (s_gaussPoints);
    int numQuadraturePoints = gaussRule.GetNumEval ();
    DPoint3d derivativePoles[MAX_BEZIER_ORDER];
    BCurveSegment segment;
    BCurveSegmentLengthIntegrator quadrature(7);
    double a = 0.0;
    double sum = 0.0;
    UsageSums eSum;
    for (size_t spanIndex = 0; curve.GetBezier (segment, spanIndex); spanIndex++)
        {
         if (!segment.IsNullU ())
            {
            double sum0 = quadrature.SumGaussRule (0, segment, matrix);
            double sum1 = quadrature.SumGaussRules (1, 3,segment, matrix);
            double e = fabs (sum1 - sum0);
            if (e <= s_relTol * fabs (sum1))
                {
                sum += sum1;
                eSum.Accumulate (e);
                }
            else
                {
                double sum2 = quadrature.SumGaussRules (3, 7, segment, matrix);
                sum += sum2;
                eSum.Accumulate (fabs (sum2 - sum1));
                }
            if (s_gaussPoints > 0)
                {
                if (curve.rational)
                    {
                    DPoint3d xyzCurve;
                    double u, w;
                    DVec3d curveTangent;
                    for (int k = 0; k < numQuadraturePoints; k++)
                        {
                        gaussRule.GetEval (k, 0.0, 1.0, u, w);
                        segment.FractionToPoint (xyzCurve, curveTangent, u, false);
                        if (nullptr != matrix)
                            matrix->Multiply (curveTangent);
                        a += w * curveTangent.Magnitude ();
                        }
                    }
                else
                    {
                    // form simple derivative poles ...
                    int tangentOrder = curve.params.order - 1;
                    double orderFactor = curve.params.order - 1;
                    DPoint4dP poleP = segment.GetPoleP ();
                    for (int i = 0; i < tangentOrder; i++)
                        {
                        derivativePoles[i].x = poleP[i+1].x - poleP[i].x;
                        derivativePoles[i].y = poleP[i+1].y - poleP[i].y;
                        derivativePoles[i].z = poleP[i+1].z - poleP[i].z;
                        }
                    DVec3d curveTangent;
                    double u, w;
                    double a1 = 0.0;
                    for (int k = 0; k < numQuadraturePoints; k++)
                        {
                        gaussRule.GetEval (k, 0.0, 1.0, u, w);
                        bsiBezierDPoint3d_evaluateDPoint3d (&curveTangent, nullptr, derivativePoles, tangentOrder, u);
                        if (nullptr != matrix)
                            matrix->Multiply (curveTangent);
                        a1 += w * curveTangent.Magnitude ();
                        }
                    a += orderFactor * a1;
                    }
                }
            else
                {
                if (nullptr != matrix)
                    a += bsiBezierDPoint4d_arcLength (matrix, segment.GetPoleP (), curve.params.order, 0.0, 1.0);
                else
                    a += bsiBezierDPoint4d_arcLength (segment.GetPoleP (), curve.params.order, 0.0, 1.0);
                }
            }
        }

    return sum;
    }
#else
static double doBCurveLength (MSBsplineCurveCR curve, RotMatrixCP matrix)
    {
    static double s_relTol = 1.0e-10;
    static int s_gaussPoints = 7;
    static int s_maxDepth = 4;
    BCurveSegment segment;
    BCurveSegmentLengthIntegrator quadrature(s_gaussPoints, s_maxDepth);
    double sumQ = 0.0;
    UsageSums eSum;
    for (size_t spanIndex = 0; curve.GetBezier (segment, spanIndex); spanIndex++)
        {
         if (!segment.IsNullU ())
            {
            double sum0 = quadrature.SumGaussRule (0, segment, matrix);
            double q = quadrature.RecursiveSum (0.0, 1.0, segment, matrix, 0, sum0, fabs (s_relTol * sum0));
            sumQ += q;
#ifdef TwoLevelExplicit
            double sum0 = quadrature.SumGaussRule (0, segment, matrix);
            double sum1 = quadrature.SumGaussRules (1, 3,segment, matrix);
            double e = fabs (sum1 - sum0);
            if (e <= s_relTol * fabs (sum1))
                {
                sum += sum1;
                eSum.Accumulate (e);
                }
            else
                {
                double sum2 = quadrature.SumGaussRules (3, 7, segment, matrix);
                sum += sum2;
                eSum.Accumulate (fabs (sum2 - sum1));
                }
#endif
            }
        }

    return sumQ;
    }
#endif


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz             08/07
+---------------+---------------+---------------+---------------+---------------+------*/
double MSBsplineCurve::Length () const
    {
    return doBCurveLength (*this, nullptr);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz             08/07
+---------------+---------------+---------------+---------------+---------------+------*/
double MSBsplineCurve::Length (RotMatrixCP worldToLocal) const
    {
    return doBCurveLength (*this, worldToLocal);
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Lu.Han          03/95
+---------------+---------------+---------------+---------------+---------------+------*/
static bool     refineSegmentArcLengthFraction
(
RotMatrixCP     worldToLocal,
BCurveSegmentR  segment,
double          &fraction,      // on input, estimated fractional position of length.
                                // on output, improved estimate
double          &absLengthError,
double          targetLength,
double          totalLength,     // precomputed total length of this bezier.
double          lengthTol,
double          fractionTol
)
    {
    absLengthError = 0.0;
    if (targetLength < 0.0)
        return false;
    if (targetLength == 0.0)
        {
        fraction = 0.0;
        return true;
        }

    static size_t maxStep = 20;
    int numConverged = 0;
    static int sNumConvergedNeeded = 2;

    for (size_t numStep = 0; numStep < maxStep; numStep++)
        {
        double newLength;
        DPoint3d point;
        DVec3d tangent;
        segment.Length (worldToLocal, newLength, 0.0, fraction);
        segment.FractionToPoint (point, tangent, fraction, false);
        if (nullptr != worldToLocal)
            worldToLocal->Multiply (tangent);
        double a = tangent.Magnitude ();
        double du;
        double signedLengthError = targetLength - newLength;
        absLengthError = fabs (signedLengthError);
        if (!DoubleOps::SafeDivideParameter (du, signedLengthError, a, 0.0))
            return false;
        if (absLengthError < lengthTol)
            {
            if (++numConverged >= sNumConvergedNeeded)
                return true;
            }
        else if (fabs (du) < fractionTol)
            {
            if (++numConverged >= sNumConvergedNeeded)
                return true;
            }
        else
            numConverged = 0;
        fraction += du;
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    earlin.lutz             10/14
+---------------+---------------+---------------+---------------+---------------+------*/
void MSBsplineCurve::PointsAtUniformFractions
(
bvector <DPoint3d> &points,
bvector<double> &fractions,
size_t numPoints
) const
    {
    points.clear ();
    fractions.clear ();

    fractions.push_back (0.0);
    if (numPoints > 1)
        {
        double step = 1.0 / (double)(numPoints - 1);
        for (size_t i = 1; i +1 < numPoints; i++)
            fractions.push_back (i * step);
        fractions.push_back (1.0);
        }
    for (double f : fractions)
        {
        DPoint3d xyz;
        FractionToPoint (xyz, f);
        points.push_back (xyz);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    earlin.lutz             10/14
+---------------+---------------+---------------+---------------+---------------+------*/
bool MSBsplineCurve::PointsAtUniformArcLength
(
bvector <DPoint3d> &points,
bvector<double> &fractions,
size_t numPoints
) const
    {
    points.clear ();
    fractions.clear ();
    bool stat = true;
    fractions.push_back (0.0);
    if (numPoints > 1)
        {
        double step = Length () / (double)(numPoints - 1);
        double fraction, error;
        for (size_t i = 1; i + 1 < numPoints; i++)
            {
            if (!FractionAtSignedDistance (fractions.back (), step, fraction, error))
                {
                stat = false;
                break;
                }
            fractions.push_back (fraction);
            }
        fractions.push_back (1.0);
        }
    for (double f : fractions)
        {
        DPoint3d xyz;
        FractionToPoint (xyz, f);
        points.push_back (xyz);
        }
    return stat;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    earlin.lutz             11/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool MSBsplineCurve::FractionAtSignedDistance
(
double fractionA,
double requestedSignedDistance,
double &fractionB,
double &actualSignedDistance
) const
    {
    return FractionAtSignedDistance (nullptr, fractionA, requestedSignedDistance, fractionB, actualSignedDistance);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    earlin.lutz             11/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool MSBsplineCurve::FractionAtSignedDistance
(
RotMatrixCP worldToLocal,
double fractionA,
double requestedSignedDistance,
double &fractionB,
double &actualSignedDistance
) const
    {
    DRange1d knotRange = DRange1d::From (FractionToKnot (fractionA));
    BCurveSegment segment;
    static double s_bezierFractionTol = 1.0e-10;    
    double segmentLength;
    double lengthError;
    double remainingLength = fabs (requestedSignedDistance);
    double lengthTol = s_bezierFractionTol * fabs (requestedSignedDistance);    // ugh. no good reference value without inspecting entire curve.
    if (requestedSignedDistance > 0.0)
        {
        knotRange.Extend (FractionToKnot (1.0));
        for (size_t i = 0; AdvanceToBezierInKnotInterval (segment, i, knotRange);)
            {
            if (nullptr == worldToLocal)
                segment.Length (segmentLength, 0.0, 1.0);
            else
                segmentLength = bsiBezierDPoint4d_arcLength (worldToLocal, segment.GetPoleP (), params.order, 0.0, 1.0);

            if (segmentLength < remainingLength)
                {
                actualSignedDistance += segmentLength;
                remainingLength -= segmentLength;
                }
            else if (segmentLength == remainingLength)
                {
                double finalKnot = segment.UMax ();
                actualSignedDistance = requestedSignedDistance;
                fractionB = KnotToFraction (finalKnot);
                return true;
                }
            else
                {
                double segmentFraction = remainingLength / segmentLength;
                if (refineSegmentArcLengthFraction (worldToLocal, segment, segmentFraction, lengthError, remainingLength, segmentLength,
                        lengthTol, s_bezierFractionTol))
                    {
                    double finalKnot = segment.FractionToKnot (segmentFraction);
                    fractionB = KnotToFraction (finalKnot);
                    actualSignedDistance = requestedSignedDistance;
                    return true;
                    }
                else
                    {
                    // um... how can this fail??
                    actualSignedDistance += segmentLength;
                    fractionB = segment.UMax ();
                    return false;
                    }
                }         
            }
        // ran off end of curve ....
        fractionB = 1.0;
        return fabs (remainingLength) <= lengthTol;
        }
    else if (requestedSignedDistance < 0.0)
        {
        knotRange.Extend (FractionToKnot (0.0));
        for (size_t i = GetTailBezierSelect (); RetreatToBezierInKnotInterval (segment, i, knotRange);)
            {
            if (nullptr == worldToLocal)
                segment.Length (segmentLength, 0.0, 1.0);
            else
                segmentLength = bsiBezierDPoint4d_arcLength (worldToLocal, segment.GetPoleP (), params.order, 0.0, 1.0);

            if (segmentLength < remainingLength)
                {
                remainingLength -= segmentLength;
                actualSignedDistance -= segmentLength;
                }
            else if (segmentLength == remainingLength)
                {
                double finalKnot = segment.UMin ();
                fractionB = KnotToFraction (finalKnot);
                actualSignedDistance = requestedSignedDistance;
                return true;
                }
            else
                {
                double leftPortion = segmentLength - remainingLength;
                double segmentFraction = leftPortion / segmentLength;
                if (refineSegmentArcLengthFraction (worldToLocal, segment, segmentFraction, lengthError, leftPortion, segmentLength,
                        lengthTol, s_bezierFractionTol))
                    {
                    double finalKnot = segment.FractionToKnot (segmentFraction);
                    fractionB = KnotToFraction (finalKnot);
                    actualSignedDistance = requestedSignedDistance;
                    return true;
                    }
                else
                    {
                    // um... how can this fail??
                    actualSignedDistance += segmentLength;
                    fractionB = segment.UMax ();
                    return false;
                    }
                }         
            }
        fractionB = 0.0;
        return fabs (remainingLength) <= lengthTol;    
        }
    // requested zero ...
    fractionB = fractionA;
    actualSignedDistance = 0.0;
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    earlin.lutz             11/12
+---------------+---------------+---------------+---------------+---------------+------*/
double MSBsplineCurve::LengthBetweenKnots (double startKnot, double endKnot) const
    {
    double sum = 0.0;
    DRange1d range = DRange1d::From (startKnot);
    range.Extend (endKnot);     // range is low to high!!!
    BCurveSegment segment;
    double segmentLength;
    for (size_t i = 0; AdvanceToBezierInKnotInterval (segment, i, range);)
        {
        segment.Length (segmentLength, 0.0, 1.0);
        sum += segmentLength;
        }

    return sum;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    earlin.lutz             11/12
+---------------+---------------+---------------+---------------+---------------+------*/
double MSBsplineCurve::LengthBetweenKnots (RotMatrixCP worldToLocal, double startKnot, double endKnot) const
    {
    double sum = 0.0;
    DRange1d range = DRange1d::From (startKnot);
    range.Extend (endKnot);     // range is low to high!!!
    BCurveSegment segment;
    for (size_t i = 0; AdvanceToBezierInKnotInterval (segment, i, range);)
        {
        sum += bsiBezierDPoint4d_arcLength (worldToLocal, segment.GetPoleP (), params.order, 0.0, 1.0);
        }

    return sum;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    peter.yu                        02/2009
+---------------+---------------+---------------+---------------+---------------+------*/
double MSBsplineCurve::LengthBetweenFractions (double startFraction, double endFraction) const
    {
    return LengthBetweenKnots (FractionToKnot (startFraction), FractionToKnot (endFraction));
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    peter.yu                        02/2009
+---------------+---------------+---------------+---------------+---------------+------*/
double MSBsplineCurve::LengthBetweenFractions (RotMatrixCP worldToLocal, double startFraction, double endFraction) const
    {
    return LengthBetweenKnots (worldToLocal, FractionToKnot (startFraction), FractionToKnot (endFraction));
    }








void MSBsplineCurve::AddStrokes (bvector <DPoint3d> &points,
                double chordTol, double angleTol, double maxEdgeLenth, bool includeStartPoint) const
    {
    BCurveSegment segment;
    size_t numRealSpan = 0;
    for (size_t spanIndex = 0; GetBezier (segment, spanIndex); spanIndex++)
        {
        if (!segment.IsNullU ())
            {
            int numEdge = bsiBezierDPoint4d_estimateEdgeCount (segment.GetPoleP (), params.order, chordTol, angleTol, maxEdgeLenth, rational? false : true);
            if (numEdge > MAX_STROKES_PER_BEZIER)
                numEdge = MAX_STROKES_PER_BEZIER;
            if (numRealSpan == 0)
                {
                DPoint3d xyz;
                if (segment.TryGetPoleXYZ (0, xyz))
                    PolylineOps::AddContinuationStartPoint (points, xyz, includeStartPoint);
                }
            bsiBezierDPoint4d_addStrokes (segment.GetPoleP (), params.order, points, NULL, NULL, numEdge, false, 0.0, 1.0);
            numRealSpan++;
            }
        }
    }


//! Compute stroke approximation
//! @param [in] options tolerance options.
//! @param [out] points array to receive points.
//! @param [out] derivatives optional array to receive derivative vectors
//! @param [out] params optional array to receive parameters
GEOMDLLIMPEXP void MSBsplineCurve::AddStrokes
(
IFacetOptionsCR options,
bvector <DPoint3d> &points,
bvector <DVec3d> *derivatives,
bvector <double> *params,
bool includeStart
) const
    {
    AddStrokes (points, derivatives, params,
                options.GetChordTolerance (),
                options.GetAngleTolerance (),
                options.GetMaxEdgeLength (),
                includeStart,
                options.GetCurveParameterMapping ()
                );
    }



void MSBsplineCurve::AddStrokes (bvector <DPoint3d> &points,
                     bvector <DVec3d> *derivatives,
                     bvector <double> *parameters,
                     double chordTol, double angleTol, double maxEdgeLenth, bool includeStartPoint,
                    CurveParameterMapping parameterSelect) const
    {
    BCurveSegment segment;
    bool thisIsTheFirstRealSegment = true;

    for (size_t spanIndex = 0; GetBezier (segment, spanIndex); spanIndex++)
        {
        size_t index0 = points.size ();
        if (!segment.IsNullU ())
            {
            int numEdge = bsiBezierDPoint4d_estimateEdgeCount (segment.GetPoleP (), params.order, chordTol, angleTol, maxEdgeLenth, rational? false : true);
            if (thisIsTheFirstRealSegment )
                {
                bsiBezierDPoint4d_addStrokes (segment.GetPoleP (), params.order, points, parameters, derivatives, numEdge, includeStartPoint, 0.0, 1.0);
                thisIsTheFirstRealSegment = false;
                }
            else
                bsiBezierDPoint4d_addStrokes (segment.GetPoleP (), params.order, points, parameters, derivatives, numEdge, false, 0.0, 1.0);
            MapFractions (parameters, derivatives, index0, segment.UMin (), segment.UMax (), parameterSelect, this);
           }
        }
    }

void MSBsplineCurve::AddStrokes (
                    size_t numPoints,
                    bvector <DPoint3d> &points,
                     bvector <DVec3d> *derivatives,
                     bvector <double> *parameters,
                     bool includeStartPoint,
                     CurveParameterMapping parameterSelect) const
    {
    if (numPoints == 0)
        return;
    double df = 0;
    size_t iStart = 0;
    if (!includeStartPoint)
        iStart = 1;

    if (numPoints > 1)
        df = 1.0 / (double)(numPoints - 1);
    // TODO: Optimize as a single sweep through the beziers
    DPoint3d xyz;
    DVec3d dxyz;
    
    for (size_t i = iStart; i < numPoints; i++)
        {
        double f = i * df;
        if (NULL == derivatives)
            {
            FractionToPoint (xyz, f);
            points.push_back (xyz);
            }
        else
            {
            FractionToPoint (xyz, dxyz, f);
            points.push_back (xyz);
            derivatives->push_back (dxyz);
            }
        if (NULL != parameters)
            parameters->push_back (f);
        }

    double uMin, uMax;
    GetKnotRange (uMin, uMax);

    if (NULL != parameters && parameterSelect != CURVE_PARAMETER_MAPPING_CurveFraction)
        MapFractions (parameters, derivatives, 0, uMin, uMax, parameterSelect, this);
    }





size_t MSBsplineCurve::GetStrokeCount (double chordTol, double angleTol, double maxEdgeLength) const
    {
    BCurveSegment segment;
    size_t count = 0;
    size_t realSegmentCount = 0;
    for (size_t spanIndex = 0; GetBezier (segment, spanIndex); spanIndex++)
        {
        if (!segment.IsNullU ())
            {
            realSegmentCount++;
            int numEdge = bsiBezierDPoint4d_estimateEdgeCount (segment.GetPoleP (), params.order, chordTol, angleTol, maxEdgeLength, rational? false : true);

            if (realSegmentCount == 1)
                count += numEdge;
            else
                count += numEdge - 1;
            }
        }

    return count;
    }

void MSBsplineCurve::AllTangentsXY (bvector<DPoint3d>& points, bvector<double>& fractions, 
                                DPoint3dCR spacePoint, DMatrix4dCP matrix) const
    {
    BCurveSegment segment, segmentXY;

    DPoint4d    homogeneousPoint;
    double      tangentParam[MAX_BEZIER_CURVE_ORDER];
    DPoint4d    tangentPoint[MAX_BEZIER_CURVE_ORDER];

    homogeneousPoint.Init (spacePoint, 1.0);
    if (matrix != NULL)
        matrix->Multiply (homogeneousPoint, homogeneousPoint);

    for (size_t spanIndex = 0; GetBezier (segment, spanIndex); spanIndex++)
        {
        if (!segment.IsNullU ())
            {
            int numTangent;
            if (matrix != NULL)
                segmentXY.CopyFrom (segment, matrix);

            bsiBezierDPoint4d_allTangentsFromDPoint4dExt (tangentParam, tangentPoint, &numTangent, MAX_BEZIER_CURVE_ORDER,
                matrix != NULL ? segmentXY.GetPoleP () : segment.GetPoleP (), (int)segment.GetOrder (),
                &homogeneousPoint,
                2,
                false);

            for (int i = 0; i < numTangent; i++)
                {
                double knot = segment.FractionToKnot (tangentParam[i]);
                double fraction = KnotToFraction (knot);
                fractions.push_back (fraction);
                
                DPoint3d xyz;
                segment.FractionToPoint (xyz, tangentParam[i]);
                points.push_back (xyz);
                }
            }
        }
    }

void MSBsplineCurve::AllTangents (bvector<DPoint3d>& points, bvector<double>& fractions, DPoint3dCR spacePoint) const
    {
    BCurveSegment segment;

    DPoint4d    homogeneousPoint;
    double      tangentParam[MAX_BEZIER_CURVE_ORDER];
    DPoint4d    tangentPoint[MAX_BEZIER_CURVE_ORDER];

    homogeneousPoint.Init (spacePoint, 1.0);

    for (size_t spanIndex = 0; GetBezier (segment, spanIndex); spanIndex++)
        {
        if (!segment.IsNullU ())
            {
            int numTangent;
            bsiBezierDPoint4d_allTangentsFromDPoint4dExt (tangentParam, tangentPoint, &numTangent, MAX_BEZIER_CURVE_ORDER,
                segment.GetPoleP (), (int)segment.GetOrder (),
                &homogeneousPoint,
                3,
                false);

            for (int i = 0; i < numTangent; i++)
                {
                double knot = segment.FractionToKnot (tangentParam[i]);
                double fraction = KnotToFraction (knot);
                fractions.push_back (fraction);
                
                DPoint3d xyz;
                segment.FractionToPoint (xyz, tangentParam[i]);
                points.push_back (xyz);
                }
            }
        }
    }

bool MSBsplineCurve::ClosestTangentXY (DPoint3dR curvePoint, double &curveFraction, 
                                    DPoint3dCR spacePoint, DPoint3dCR biasPoint, DMatrix4dCP matrix) const
    {
    bvector<DPoint3d> points;
    bvector<double> fractions;
    AllTangentsXY (points, fractions, spacePoint, matrix);
    
    if (fractions.size () > 0)
        {
        size_t i;
        double distance, minDistance = DBL_MAX;
        double fraction;
        size_t numTest = 0;
        for (i = 0; i < fractions.size (); i++)
            {
            fraction = fractions[i];
            if (biasPoint.DistanceXY (points[i], matrix, distance)
                && (numTest == 0 || distance < minDistance))
                {
                numTest++;
                minDistance = distance;
                curvePoint = points[i];
                curveFraction = fraction;
                }
            }

        return true;
        }

    return false;
    }

bool MSBsplineCurve::ClosestTangent (DPoint3dR curvePoint, double &curveFraction, DPoint3dCR spacePoint, DPoint3dCR biasPoint) const
    {
    bvector<DPoint3d> points;
    bvector<double> fractions;
    AllTangents (points, fractions, spacePoint);

    if (fractions.size () > 0)
        {
        size_t i;
        double distance, minDistance = DBL_MAX;
        double fraction;
        DPoint3d testPoint;

        for (i = 0; i < fractions.size (); i++)
            {
            fraction = fractions[i];
            testPoint = points[i];
            distance = testPoint.Distance (biasPoint);
            if (i == 0 || distance < minDistance)
                {
                minDistance = distance;
                curvePoint = testPoint;
                curveFraction = fraction;
                }
            }

        return true;
        }

    return false;
    }

void MSBsplineCurve::AllParallellTangentsXY (bvector<DPoint3d>& points, bvector<double>& fractions, DVec3d vector) const
    {
    BCurveSegment segment;

    DPoint4d    homogeneousPoint;
    double      tangentParam[MAX_BEZIER_CURVE_ORDER];
    DPoint4d    tangentPoint[MAX_BEZIER_CURVE_ORDER];

    homogeneousPoint.Init (vector, 0.0);

    for (size_t spanIndex = 0; GetBezier (segment, spanIndex); spanIndex++)
        {
        if (!segment.IsNullU ())
            {
            int numTangent;
            bsiBezierDPoint4d_allTangentsFromDPoint4dExt (tangentParam, tangentPoint, &numTangent, MAX_BEZIER_CURVE_ORDER,
                segment.GetPoleP (), GetIntOrder (),
                &homogeneousPoint,
                3,
                false);

            for (int i = 0; i < numTangent; i++)
                {
                double knot = FractionToKnot (tangentParam[i]);
                double fraction = KnotToFraction (knot);
                fractions.push_back (fraction);
                
                DPoint3d xyz;
                segment.FractionToPoint (xyz, tangentParam[i]);
                points.push_back (xyz);
                }
            }
        }
    }

static double sCuspRelTol = 1.0e-8;

void MSBsplineCurve::AddCuspsXY (bvector<DPoint3d> *points, bvector<double> *fractionParameters, DMatrix4dCP matrix) const
    {
    BCurveSegment segment, segmentXY;

    double values[MAX_ORDER];
    DPoint3d point;
    DVec3d   derivA;
    DPoint4d cusps[MAX_ORDER];
    double lastParam = -1.0;

    derivA.Zero ();
    for (size_t spanIndex = 0; GetBezier (segment, spanIndex); spanIndex++)
        {
        double u0 = 0.0;
        double u1 = 1.0;

        if (!segment.IsNullU ())
            {
            if (matrix != NULL)
                segmentXY.CopyFrom (segment, matrix);

            if (spanIndex > 0)
                {
                DVec3d derivB;

                if (matrix != NULL)
                    segmentXY.FractionToPoint (point, derivB, u0);
                else
                    segment.FractionToPoint (point, derivB, u0);

                derivB.Normalize ();
                if (derivB.DotProduct (derivA) < COSINE_TOLERANCE && !DoubleOps::AlmostEqual (segment.UMin (), lastParam))
                    {
                    if (fractionParameters)
                        fractionParameters->push_back (segment.UMin ());
                    if (points)
                        {
                        segment.FractionToPoint (point, u0);
                        points->push_back (point);
                        }
                    lastParam = segment.UMin ();
                    }
                }

            if (matrix != NULL)
                segmentXY.FractionToPoint (point, derivA, u1);
            else
                segment.FractionToPoint (point, derivA, u1);

            derivA.Normalize ();
            int numInBezier;
            bsiBezierDPoint4d_allNearCusps (values, cusps, &numInBezier, (int)segment.GetOrder (),
                    matrix != NULL ? segmentXY.GetPoleP () : segment.GetPoleP (), 
                    (int)segment.GetOrder (), 2, sCuspRelTol);

            for (size_t j = 0; j < (size_t)numInBezier; j++)
                {
                double param = segment.FractionToKnot (values[j]);
                
                if (j==0 && DoubleOps::AlmostEqual (param, lastParam))
                    continue;

                if (fractionParameters)
                    fractionParameters->push_back (param);
                if (points)
                    {
                    segment.FractionToPoint (point, values[j]);
                    points->push_back (point);
                    }
                
                lastParam = param;
                }
            }
        }
    }

void MSBsplineCurve::AddCusps (bvector<DPoint3d> *points, bvector<double> *fractionParameters) const
    {
    double values[MAX_ORDER];
    DPoint3d point;
    DVec3d   derivA;
    DPoint4d cusps[MAX_ORDER];
    BCurveSegment segment;
    double lastParam = -1.0;

    derivA.Zero ();
    for (size_t spanIndex = 0; GetBezier (segment, spanIndex); spanIndex++)
        {
        double u0 = 0.0;    /* Put in variable form so we can pass an address */
        double u1 = 1.0;

        if (!segment.IsNullU ())
            {
            if (spanIndex > 0)
                {
                DVec3d derivB;

                segment.FractionToPoint (point, derivB, u0);
                derivB.Normalize ();
                if (derivB.DotProduct (derivA) < COSINE_TOLERANCE && !DoubleOps::AlmostEqual (segment.UMin (), lastParam))
                    {
                    if (fractionParameters)
                        fractionParameters->push_back (segment.UMin ());
                    if (points)
                        points->push_back (point);
                    lastParam = segment.UMin ();
                    }
                }

            segment.FractionToPoint (point, derivA, u1);
            derivA.Normalize ();
            int numInBezier;
            bsiBezierDPoint4d_allNearCusps (values, cusps, &numInBezier,
                    (int)segment.GetOrder (), segment.GetPoleP (),
                    (int)segment.GetOrder (), 3, sCuspRelTol);

            for (size_t j = 0; j < (size_t)numInBezier; j++)
                {
                double param = FractionToKnot (values[j]);
                
                if (j==0 && DoubleOps::AlmostEqual (param, lastParam))
                    continue;

                if (fractionParameters)
                    fractionParameters->push_back (param);
                if (points)
                    {
                    segment.FractionToPoint (point, values[j]);
                    points->push_back (point);
                    }
                
                lastParam = param;
                }
            }
        }
    }

void MSBsplineCurve::AddLineIntersectionsXY (bvector<DPoint3d> *curvePoints, bvector<double> *curveFractions,
                                             bvector<DPoint3d> *linePoints, bvector<double> *lineFractions,
                                        DSegment3dCR segment, bool extendSegment, DMatrix4dCP matrix) const
    {
    bvector<DPoint3d>curveWorkPoint;
    bvector<double>curveWorkFraction;
    DPoint3d lineWorkPoint;
    double lineWorkFraction;
    if (NULL == matrix)
        {
        DVec3d zVector = DVec3d::From (0.0, 0.0, 1.0);
        DVec3d lineVector = DVec3d::FromStartEnd (segment.point[0], segment.point[1]);
        DVec3d planeNormal;
        planeNormal.NormalizedCrossProduct (lineVector, zVector);
        DPlane3d plane = DPlane3d::FromOriginAndNormal (segment.point[0], planeNormal);
        AddPlaneIntersections (&curveWorkPoint, &curveWorkFraction, plane);
        for (size_t i = 0; i < curveWorkPoint.size (); i++)
            {
            if (segment.ProjectPointXY (lineWorkPoint, lineWorkFraction, curveWorkPoint[i]))
                {
                if (extendSegment || (lineWorkFraction >= 0.0 && lineWorkFraction <= 1.0))
                    {
                    if (NULL != curvePoints)
                        curvePoints->push_back (curveWorkPoint[i]);
                    if (NULL != curveFractions)
                        curveFractions->push_back (curveWorkFraction[i]);
                    if (NULL != linePoints)
                        linePoints->push_back (lineWorkPoint);
                    if (NULL != lineFractions)
                        lineFractions->push_back (lineWorkFraction);
                    }
                }
            }
        }
    else
        {
        DSegment4d segmentXYZW;
        segmentXYZW.Init (segment.point[0], segment.point[1]);
        matrix->Multiply (segmentXYZW.point, segmentXYZW.point, 2);        
        // Perspective time.
        // We want planeCoffs such that:
        // 1) xyzwPlaneCoffs DOT segmentXYZW.point[0] = 0 --- segment start is on plane.
        // 2) xyzwPlaneCoffs DOT segmentXYZW.point[1] = 0 --- segment end is on plane.
        // 3) xyzwPlaneCoffs DOT (0,0,1,0) = 0 --- post-transform eyepoint is on plane.
        // 4) xyzwPlaneCoffs DOT (1,1,0,1) = 1 --- xyw parts are nontrivial
        // This is in post-transform space -- for a point X "on the curve" we want
        //             xyzwPlaneCoffs DOT (matrix * X) = 0
        // i.e.       (xyzwPlaneCoffs ^ * matrix) DOT X = 0
        DMatrix4d planeMatrix, planeMatrixInverse;
        planeMatrix.SetRow (0, segmentXYZW.point[0]);
        planeMatrix.SetRow (1, segmentXYZW.point[1]);
        planeMatrix.SetRow (2, 0.0, 0.0, 1.0, 0.0);
        planeMatrix.SetRow (3, 1.0, 1.0, 0.0, 1.0);
        DPoint4d rightHandSide = DPoint4d::From (0.0, 0.0, 0.0, 1.0);
        DPoint4d xyPlaneCoffs, xyzPlaneCoffs;
        if (planeMatrixInverse.QrInverseOf (planeMatrix))
            {
            planeMatrixInverse.Multiply (&xyPlaneCoffs, &rightHandSide, 1);
            matrix->MultiplyTranspose (&xyzPlaneCoffs, &xyPlaneCoffs, 1);
            AddPlaneIntersections (&curveWorkPoint, &curveWorkFraction, xyzPlaneCoffs);
            for (size_t i = 0; i < curveWorkPoint.size (); i++)
                {
                DPoint4d transformedWorkPoint, closestLinePoint;
                matrix->Multiply (&transformedWorkPoint, &curveWorkPoint[i], NULL, 1);
                if (segmentXYZW.ProjectPointUnboundedCartesianXYW (closestLinePoint, lineWorkFraction, transformedWorkPoint))
                    {
                    if (extendSegment || (lineWorkFraction >= 0.0 && lineWorkFraction <= 1.0))
                        {
                        segment.FractionParameterToPoint (lineWorkPoint, lineWorkFraction);
                        if (NULL != curvePoints)
                            curvePoints->push_back (curveWorkPoint[i]);
                        if (NULL != curveFractions)
                            curveFractions->push_back (curveWorkFraction[i]);
                        if (NULL != linePoints)
                            linePoints->push_back (lineWorkPoint);
                        if (NULL != lineFractions)
                            lineFractions->push_back (lineWorkFraction);
                        }
                    }
                }
            }
        }
    }

DRange1d MSBsplineCurve::GetRangeOfProjectionOnRay (DRay3dCR ray, double fraction0, double fraction1) const
    {
    if (!DoubleOps::IsExact01 (fraction0, fraction1))
        {
        MSBsplineCurve partialCurve;
        partialCurve.CopyFractionSegment (*this, fraction0, fraction1);
        DRange1d projectedRange = partialCurve.GetRangeOfProjectionOnRay (ray, 0.0, 1.0);
        partialCurve.ReleaseMem ();
        return projectedRange;
        }

    DPoint4d planeCoffs;
    planeCoffs.PlaneFromOriginAndNormal (ray.origin, ray.direction);
    planeCoffs.Scale (1.0/ray.direction.Magnitude());
    
    DRange1d range = DRange1d ();
    BCurveSegment1d segment;
    double roots[MAX_ORDER], ders[MAX_ORDER-1];
    size_t order = GetOrder ();
    int numRoot;

    Bspline1d spline;
    spline.GetKnotsR().assign (knots, knots + NumberAllocatedKnots ());
    bvector<double> & poles1D = spline.GetPolesR ();
    for (int k = 0; k < NumberAllocatedPoles (); k++)
        poles1D.push_back (planeCoffs.DotProduct (GetPoleDPoint4d (k)));

    for (size_t spanIndex = 0; spline.GetBezier (segment, spanIndex); spanIndex++)
        {
        if (!segment.isNullU)
            {
            range.Extend (segment.poles[0]);
            range.Extend (segment.poles[order-1]);

            for (size_t i = 0; i < order-1; i++)
                ders[i] = order*(segment.poles[i+1] - segment.poles[i]);

            if (bsiBezier_univariateRoots (roots, &numRoot, ders, (int)order-1) && numRoot > 0)
                {
                for (size_t j = 0; j < (size_t)numRoot; j++)
                    range.Extend (segment.FractionToValue (roots[j]));
                }
                
            }
        }

    return range;
    }

MSBsplineStatus MSBsplineCurve::ComputeInflectionPoints (bvector<DPoint3d>& points, bvector<double>& inflectionParams)
    {
    DPoint3d derivA1, derivA2, point;
    bvector<GraphicsPoint> inflections;
    bvector<DPoint4d> segmentPoles;
    BCurveSegment segment;
    derivA1.Zero ();
    derivA2.Zero ();
    size_t numRealSpan = 0;
    for (size_t spanIndex = 0; GetBezier (segment, spanIndex); spanIndex++)
        {
        double u0 = 0.0;    /* Put in variable form so we can pass an address */
        double u1 = 1.0;
        
        if (!segment.IsNullU ())
            {
            numRealSpan++;
            if (numRealSpan > 1)
                {
                //bool    bBreakAtStart = false;
                DPoint3d derivB1, derivB2;
                DPoint3d normalA, normalB;
                bsiBezierDPoint4d_evaluateDPoint3dArrayExt (NULL, &derivB1, &derivB2,
                                    segment.GetPoleP (), params.order, &u0, 1);
                normalA.CrossProduct (derivA1, derivA2);
                normalB.CrossProduct (derivB1, derivB2);
                double zero = -1.0E-8;
                if (normalA.DotProduct (normalB) < zero)
                    {
                    inflectionParams.push_back (segment.UMin ());
                    }
                }

            bsiBezierDPoint4d_evaluateDPoint3dArrayExt (NULL, &derivA1, &derivA2, segment.GetPoleP (), params.order, &u1, 1);
            segment.GetPoles (segmentPoles);
            bsiBezierDPoint4d_inflectionPoints (inflections, segmentPoles);
            for (auto & gp : inflections)
                {
                double  param = segment.FractionToKnot (gp.a);
                if(fabs(param) < 1e-8 || fabs(param-1.0) < 1e-8)  //from SS3
                    continue;
                inflectionParams.push_back (param);
                }
            }
        }

    DoubleOps::Sort (inflectionParams);

    size_t i = 0, numInflections = inflectionParams.size ();

    if (numInflections > 0)
        {
        while (i < numInflections)
            {
            FractionToPoint (point, inflectionParams[i]);
            points.push_back (point);
            i++;
            }
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Peter.Yu                        04/2010
+---------------+---------------+---------------+---------------+---------------+------*/
MSBsplineStatus MSBsplineCurve::ComputeInflectionPointsXY (bvector<DPoint3d>& points, bvector<double>& inflectionParams, RotMatrixCP transform)
    {
    double values[MAX_ORDER];
    DPoint3d derivA1, derivA2, point;
    bool derivAValid = false;
    DPoint4d inflections[MAX_ORDER];
    BCurveSegment segment;
    derivA1.Zero ();
    derivA2.Zero ();

    for (size_t spanIndex = 0; GetBezier (segment, spanIndex); spanIndex++)
        {
        double u0 = 0.0;    /* Put in variable form so we can pass an address */
        double u1 = 1.0;
        
        if (!segment.IsNullU ())
            {
            if (transform)
                transform->Multiply (segment.GetPoleP (), segment.GetPoleP (), (int)segment.GetOrder ());
            if (derivAValid)
                {
                //bool    bBreakAtStart = false;
                DPoint3d derivB1, derivB2;
                DPoint3d normalA, normalB;
                bsiBezierDPoint4d_evaluateDPoint3dArrayExt (NULL, &derivB1, &derivB2,
                                    segment.GetPoleP (), params.order, &u0, 1);
                normalA.CrossProduct (derivA1, derivA2);
                normalB.CrossProduct (derivB1, derivB2);
                if (normalA.DotProduct (normalB) <=0.0)
                    {
                    inflectionParams.push_back (segment.UMin ());
                    }
                }

            bsiBezierDPoint4d_evaluateDPoint3dArrayExt (NULL, &derivA1, &derivA2, segment.GetPoleP (), params.order, &u1, 1);
            derivAValid = true;
            size_t numInBezier = bsiBezierDPoint4d_inflectionPointsXY (inflections, values, segment.GetPoleP (), params.order);
            for (size_t j = 0; j < numInBezier; j++)
                {
                double  param = segment.FractionToKnot (values[j]);
                inflectionParams.push_back (param);
                }
            }
        }

    DoubleOps::Sort (inflectionParams);

    size_t i = 0, numInflections = inflectionParams.size ();

    if (numInflections > 0)
        {
        while (i < numInflections)
            {
            FractionToPoint (point, inflectionParams[i]);
            points.push_back (point);
            i++;
            }
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Peter.Yu                        03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
DRange3d MSBsplineCurve::GetRange () const
    {
    DRange3d range;
    range.Init ();
    BCurveSegment segment;

    for (size_t spanIndex = 0; GetBezier (segment, spanIndex); spanIndex++)
        {
         if (!segment.IsNullU ())
            {
            DRange3d curveRange;
            bsiBezierDPoint4d_getDRange3d (&curveRange, segment.GetPoleP (), (int)segment.GetOrder ());
            range.Extend (curveRange);
            }
        }
    
    return range;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz             08/07
+---------------+---------------+---------------+---------------+---------------+------*/
void MSBsplineCurve::WireCentroid
(
double &length,
DPoint3dR centroid,
double fraction0,
double fraction1
) const
    {
    // range initialization shuffles if in reverse order.
    // That's good -- these integrals are direction independent.
    DRange1d segmentKnots, requestedKnots, segmentFractions;

    requestedKnots = DRange1d::From (
                        FractionToKnot (fraction0),
                        FractionToKnot (fraction1)
                        );
    BCurveSegment segment;
    length = 0.0;
    centroid.Zero ();
    DVec3d sum;
    sum.Zero ();
    for (size_t spanIndex = 0; GetBezier (segment, spanIndex); spanIndex++)
        {
         if (!segment.IsNullU ()
            && segment.KnotRange ().StrictlyNonEmptyFractionalIntersection (requestedKnots, segmentFractions))
            {
            double segmentLength;
            DVec3d segmentCentroid;
            segment.WireCentroid (segmentLength, segmentCentroid, segmentFractions.low, segmentFractions.high);
            length += segmentLength;
            sum.SumOf (sum, segmentCentroid, segmentLength);
            }
        }
    centroid.SafeDivide (sum, length);
    }

size_t MSBsplineCurve::CountDistinctBeziers () const
    {
    // Should be able to do this from the knots alone, but going through the supports is definitive...
    // (And it just copies poles around -- we suppress the knot insertions)
    BCurveSegment segment;
    size_t n = 0;
    for (size_t i = 0; AdvanceToBezier (segment, i, false);)
        {
        n++;
        }
    return n;
    }

// REMARK: This is a plain C function (instead of method) in order to allow a targeted "friend struct" declaration
//  in BCurveSegment.
bool bspcurv_getSupport
(
MSBsplineCurveCR curve,
BCurveSegmentR   segment,
size_t bezierSelect
)
    {
    int numPoles = (int)curve.params.numPoles;
    int order    = (int)curve.params.order;

    if (order < 2)
        return false;
    if (numPoles < 2)
        return false;

    bool closed  = curve.params.closed != 0 ? true : false;
    segment.m_order = 0;
    segment.m_numKnots = 0;
    segment.m_index = 0;
    int numBezier = closed ? numPoles : numPoles - order + 1;
    int basePole = (int)bezierSelect;
    if (closed)
        {
        if (mdlBspline_knotsShouldBeOpened (curve.knots, (int)(2*order), NULL, NULL, 0, (int)order, (int)closed))
            {
            basePole += numPoles;
            basePole -= order / 2;
            }        
        while (basePole >= numPoles)
            basePole -= numPoles;
        }
    //basePole = bezierSelect;    // nope, do the shift in the knots.
    if ((size_t)order <= BCurveSegment::MaxOrder && (int)bezierSelect < numBezier)
        {
        segment.m_order = order;
        segment.m_index = bezierSelect;

        if (curve.weights != NULL)
            {
            DPoint4d xyzw;
            for (int i = 0; i < order; i++)
                {
                int j = basePole + i;
                if (closed && j >= numPoles)
                    j -= numPoles;
                xyzw.x = curve.poles[j].x;
                xyzw.y = curve.poles[j].y;
                xyzw.z = curve.poles[j].z;
                xyzw.w = curve.weights[j];
                segment.m_poles[i] = xyzw;
                }
            }
        else
            {
            DPoint4d xyzw;
            for (int i = 0; i < order; i++)
                {
                int j = basePole + i;
                if (closed && j >= numPoles)
                    j -= numPoles;
                xyzw.x = curve.poles[j].x;
                xyzw.y = curve.poles[j].y;
                xyzw.z = curve.poles[j].z;
                xyzw.w = 1.0;
                segment.m_poles[i] = xyzw;
                }
            }

        segment.m_numKnots = 2 * order - 2;
        //size_t j0 = bezierSelect + order - 1;
        //size_t j0 = bezierSelect + 1;
        //size_t j0 = closed ? bezierSelect + 2 : bezierSelect + 1;
        int j0 = (int)bezierSelect + 1;   // BCurveSegment does not carry the legacy extra knot !!!!
        for (int i = 0, j = j0; i < (int)segment.m_numKnots; i++, j++)
            segment.m_knots[i] = curve.knots[j];
        segment.m_uMin = segment.m_knots[order - 2];
        segment.m_uMax = segment.m_knots[order - 1];
        segment.m_isNullU = bsiBezier_isNullKnotInterval (segment.m_uMin, segment.m_uMax);
        segment.m_index = bezierSelect;
        return true;
        }
    return false;
    }


struct EqualChordsByLengthContext
{

    bvector<DPoint3d> m_pointArray;
    bvector<double> m_paramArray;

    DPoint3d mXYZOld;
    double   mParamOld;
    double   mChordLength;

    MSBsplineCurve *mpCurve;
EqualChordsByLengthContext (MSBsplineCurve *curveP, double chordLength)
    {
    mpCurve = curveP;
    mChordLength = chordLength;
    
    mParamOld = 0.0;
    mpCurve->FractionToPoint (mXYZOld, mParamOld);
    m_pointArray.push_back (mXYZOld);
    m_paramArray.push_back (mParamOld);
    }


bool ProcessBezier (DPoint4d *pPoles, int order, double s0, double s1)
    {
    DVec3d U[MAX_BEZIER_CURVE_ORDER];
    double   dw[MAX_BEZIER_CURVE_ORDER];
    double   gPoles[2 * MAX_BEZIER_CURVE_ORDER];
    double   hPoles[2 * MAX_BEZIER_CURVE_ORDER];
    double   fPoles[2 * MAX_BEZIER_CURVE_ORDER];
    double   roots[ 2 * MAX_BEZIER_CURVE_ORDER];
    int numRoots;
    int      fOrder, maxOrder = 2 * MAX_BEZIER_CURVE_ORDER;
    double uBase = 0.0; // To find: u > uBase with chord condition.
    for (bool bCurveAlive = true; bCurveAlive;)
        {
        // Homogeneous distance condition is
        // (X/w - Q) . (X/w - Q) = d^2
        //  (X - Qw).(X-Qw) = (d*w)^2
        //  U = X-Qw is a pure vector.
        //  Result is a scalar of double degree.
        for (int i = 0; i < order; i++)
            {
            U[i].x = pPoles[i].x - mXYZOld.x * pPoles[i].w;
            U[i].y = pPoles[i].y - mXYZOld.y * pPoles[i].w;
            U[i].z = pPoles[i].z - mXYZOld.z * pPoles[i].w;
            dw[i]  = pPoles[i].w * mChordLength;
            }

        bsiBezier_dotProduct (gPoles, &fOrder, maxOrder,
                        (double*)U, order, 0, 3,
                        (double*)U, order, 0, 3,
                        3);
        bsiBezier_univariateProduct (hPoles, 0, 1,
                        dw, order, 0, 1,
                        dw, order, 0, 1);

        for (int i = 0; i < fOrder; i++)
            fPoles[i] = gPoles[i] - hPoles[i];

        bsiBezier_univariateRoots (roots, &numRoots, fPoles, fOrder);
        bool bNextPointFound = false;
        // take the first root with u > uBase as the "next" base point ...
        for (int i = 0; i < numRoots && !bNextPointFound; i++)
            {
            double u = roots[i];
            if (u > uBase)
                {
                double s = s0 +  u * (s1 - s0);
                DPoint3d xyz;
                bsiBezierDPoint4d_evaluateDPoint3dArray (
                        &xyz, NULL, pPoles, order, &u, 1);
                m_pointArray.push_back (xyz);
                m_paramArray.push_back (s);
                bNextPointFound = true;
                // Shift base point forward ...
                mXYZOld = xyz;
                mParamOld = s;
                uBase = u;
                }
            }

        if (!bNextPointFound)
            bCurveAlive = false;
        }
    return true;
    }
};
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     08/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool MSBsplineCurve::StrokeWithFixedChordLength
(
bvector<DPoint3d> &points,
bvector<double> &params,
double          chordLength   /* => chord length */
)
    {
    EqualChordsByLengthContext context (this, chordLength);
    points.clear  ();
    params.clear ();
    BCurveSegment segment;
    bool handlerOK = true;
    for (size_t spanIndex = 0; handlerOK && GetBezier (segment, spanIndex); spanIndex++)
        {
        if (!segment.IsNullU ())
            handlerOK = context.ProcessBezier (
                    segment.GetPoleP (), (int)segment.GetOrder (),
                    segment.UMin (), segment.UMax ());
        }

    if (handlerOK)
        {
        points.swap (context.m_pointArray);
        params.swap (context.m_paramArray);
        return true;
        }
    return false;
    }


END_BENTLEY_GEOMETRY_NAMESPACE
