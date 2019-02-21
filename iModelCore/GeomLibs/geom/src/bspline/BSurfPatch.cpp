/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/bspline/BSurfPatch.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include <memory.h>
#include <stdlib.h>
#include <math.h>
#include "msbsplinemaster.h"

#include "GridArrays.cpp"



BEGIN_BENTLEY_GEOMETRY_NAMESPACE

// internal controls to force selected steps to be repeated to demonstrate timing effects.
static int s_timerSelect = 0;
static int s_timerCount = 0;
void bspsurf_setTimerControl (int select, int count)
    {
    s_timerSelect = select;
    s_timerCount = count;
    }

DPoint2d BSurfPatch::PatchUVToKnotUV (DPoint2dCR patchUV) const
    {
    return DPoint2d::From (
            DoubleOps::Interpolate (uMin, patchUV.x, uMax),
            DoubleOps::Interpolate (vMin, patchUV.y, vMax)
            );
    }


void GetBasePole (size_t select, bool closed, double *knots, size_t order, size_t numPoles, size_t &basePole, size_t &maxBasePole)
    {
    basePole = select;
    maxBasePole = numPoles - order;
    if (closed)
        {
        maxBasePole = numPoles - 1;
        if (mdlBspline_knotsShouldBeOpened (knots, (int)(2*order), NULL, NULL, 0, (int)order, (int)closed))
            {
            basePole += numPoles;
            basePole -= order / 2;
            }        
        while (basePole >= numPoles)
            basePole -= numPoles;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Peter.Yu                        04/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool MSBsplineSurface::GetSupport 
(
bvector<DPoint4d>& outPoles, 
bvector<double>& outUKnots, 
bvector<double>& outVKnots, 
size_t uIndex,
size_t vIndex
) const
    {
    size_t numUPoles = (int)uParams.numPoles;
    size_t numVPoles = (int)vParams.numPoles;
    size_t uOrder    = (int)uParams.order;
    size_t vOrder    = (int)vParams.order;
    bool uClosed  = uParams.closed != 0 ? true : false;
    bool vClosed  = vParams.closed != 0 ? true : false;
    outPoles.clear ();
    outUKnots.clear ();
    outVKnots.clear ();
    size_t uBasePole, vBasePole, uMaxBasePole, vMaxBasePole;
    GetBasePole (uIndex, uClosed, uKnots, uOrder, numUPoles, uBasePole, uMaxBasePole);
    GetBasePole (vIndex, vClosed, vKnots, vOrder, numVPoles, vBasePole, vMaxBasePole);
    if (uIndex <= uMaxBasePole && vIndex <= vMaxBasePole)
        {
        if (weights != NULL)
            {
            DPoint4d xyzw;
            for (size_t i = 0; i < vOrder; i++)
                {
                for (size_t j = 0; j < uOrder; j++)
                    {
                    size_t k = vBasePole + i;
                    size_t l = uBasePole + j;
                    if (vClosed && k >= numVPoles)
                        k -= numVPoles;
                    if (uClosed && l >= numUPoles)
                        l -= numUPoles;
                    xyzw.x = poles[k*numUPoles+l].x;
                    xyzw.y = poles[k*numUPoles+l].y;
                    xyzw.z = poles[k*numUPoles+l].z;
                    xyzw.w = weights[k*numUPoles+l];
                    outPoles.push_back (xyzw);
                    }
                }
            }
        else
            {
            DPoint4d xyzw;
            for (size_t i = 0; i < vOrder; i++)
                {
                for (size_t j = 0; j < uOrder; j++)
                    {
                    size_t k = vBasePole + i;
                    size_t l = uBasePole + j;
                    if (vClosed && k >= numVPoles)
                        k -= numVPoles;
                    if (uClosed && l >= numUPoles)
                        l -= numUPoles;
                    xyzw.x = poles[k*numUPoles+l].x;
                    xyzw.y = poles[k*numUPoles+l].y;
                    xyzw.z = poles[k*numUPoles+l].z;
                    xyzw.w = 1.0;
                    outPoles.push_back (xyzw);
                    }
                }
            }

        size_t numUKnotsOut = 2 * uOrder - 2, numVKnotsOut = 2 * vOrder - 2;
        for (size_t i = 0, j = uIndex + 1; i < numUKnotsOut; i++, j++)
            outUKnots.push_back (uKnots[j]);
        for (size_t i = 0, j = vIndex + 1; i < numVKnotsOut; i++, j++)
            outVKnots.push_back (vKnots[j]);

        return true;
        }

    return false;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Peter.Yu                        04/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool MSBsplineSurface::GetPatchSupport (BSurfPatch& patchData, size_t uIndex, size_t vIndex) const
    {
    size_t uOrder    = (size_t)uParams.order;
    size_t vOrder    = (size_t)vParams.order;

    if (uOrder > 1 && vOrder > 1 && GetSupport (patchData.xyzw, patchData.uKnots, patchData.vKnots, uIndex, vIndex))
        {
        patchData.uMin = patchData.uKnots[uOrder - 2];
        patchData.uMax = patchData.uKnots[uOrder - 1];
        patchData.vMin = patchData.vKnots[vOrder - 2];
        patchData.vMax = patchData.vKnots[vOrder - 1];
        patchData.uOrder = uOrder;
        patchData.vOrder = vOrder;
        patchData.uIndex = uIndex;
        patchData.vIndex = vIndex;

        return true;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Peter.Yu                        04/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool BSurfPatch::DoSaturate () 
    {
    DPoint4d    cps[MAX_BEZIER_CURVE_ORDER];

    for (size_t col = 0; col < uOrder; col++)
        {
        for (size_t row = 0; row < vOrder; row++)
            cps[row] = xyzw[col + row*uOrder];

        bsiBezier_saturateKnotsInInterval ((double*)cps, 4, &vKnots[0], (int)vOrder, isNullV);

        if (!isNullV)
            {
            for (size_t row = 0; row < vOrder; row++)
                xyzw[col + row*uOrder] = cps[row];
            }
        }

    for (size_t row = 0; row < vOrder; row++)
        {
        for (size_t col  =0; col < uOrder; col++)
            cps[col] = xyzw[col + row*uOrder];

        bsiBezier_saturateKnotsInInterval ((double*)cps, 4, &uKnots[0], (int)uOrder, isNullU);

        if (!isNullU)
            {
            for (size_t col = 0; col < uOrder; col++)
                xyzw[col + row*uOrder] = cps[col];
            }
        }

    uMin = uKnots[uOrder - 2];
    uMax = uKnots[uOrder - 1];
    vMin = vKnots[vOrder - 2];
    vMax = vKnots[vOrder - 1];

    return true;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Peter.Yu                        04/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool MSBsplineSurface::GetPatch (BSurfPatch& patchData, size_t uIndex, size_t vIndex) const
    {
    if (!MSBsplineSurface::GetPatchSupport (patchData, uIndex, vIndex))
        return false;

    patchData.DoSaturate ();
    return true;
    }


struct PatchProjectionFunction : FunctionRRToRR
{
BSurfPatchCR m_patch;
DPoint3d m_spacePoint;
double m_uScale;
double m_vScale;
PatchProjectionFunction (BSurfPatchCR patch, DPoint3dCR spacePoint) : m_patch (patch), m_spacePoint (spacePoint)
    {
    m_uScale = m_vScale = 1.0;
    }

// Record the magnitude of the tangent vectors as scale factors.  This can prevent huge number issues when the tangents are dotted with large deltas.
// (And if U,V are zero at the reference point, the factors stay 1.0.    That would be a really bad place for the iteration with or without this scaling.)
void SetScales (double u, double v)
    {
    DPoint3d xyz;
    DVec3d U, V;
    m_patch.Evaluate (DPoint2d::From (u, v), xyz, U, V);
    DoubleOps::SafeDivide (m_uScale, 1.0, U.Magnitude (), 1.0);
    DoubleOps::SafeDivide (m_vScale, 1.0, V.Magnitude (), 1.0);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod    
+---------------+---------------+---------------+---------------+---------------+------*/
bool EvaluateRRToRR
(
double u,
double v,
double &f,
double &g
) override
    {
    DPoint3d xyz;
    DVec3d U, V, W;
    m_patch.Evaluate (DPoint2d::From (u, v), xyz, U, V);
    W.DifferenceOf (m_spacePoint, xyz);
    f = W.DotProduct (U) * m_uScale;
    g = W.DotProduct (V) * m_vScale;
    return true;
    }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                   
+---------------+---------------+---------------+---------------+---------------+------*/
static bool RefinePatchClosestPoint_interiorOnly (BSurfPatchCR patch, DPoint3dCR spacePoint, double maxDelta01, SolidLocationDetailR detail)
    {
    static int s_skip = 0;
    if (s_skip)
        return false;
    static double s_abstol = 1.0e-12;
    static double s_maxDelta = 0.25;
    PatchProjectionFunction F (patch, spacePoint);
    NewtonIterationsRRToRR newton (s_abstol);
    double u = detail.GetU ();
    double v = detail.GetV ();
    F.SetScales (u, v);
    DPoint3d xyzA, xyzB;
        patch.Evaluate (DPoint2d::From (u,v), xyzA);
    if (newton.RunApproximateNewton (u, v, F, s_maxDelta, s_maxDelta))
        {
        patch.Evaluate (DPoint2d::From (u,v), xyzB);
        double dA = spacePoint.Distance (xyzA);
        double dB = spacePoint.Distance (xyzB);
        bool accept = false;
        if (DoubleOps::IsIn01 (u, v))
            {
            accept = dB < dA;
            }
        else  // point outside 01.  Try clamped point.
            {
            double u1 = DoubleOps::ClampFraction (u);
            double v1 = DoubleOps::ClampFraction (v);
            patch.Evaluate (DPoint2d::From (u,v), xyzB);
            dB = spacePoint.Distance (xyzB);
            if (dB < dA && fabs (u1 - u) < s_abstol && fabs (v1 - v) < s_abstol)
                {
                u = u1;
                v = v1;
                accept = true;
                }
            }
        if (accept)
            {
            detail.SetU (u);
            detail.SetV (v);
            detail.SetA (xyzB.Distance (spacePoint));
            detail.SetXYZ (xyzB);
            return true;
            }
        }

    return false;
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod                       
+---------------+---------------+---------------+---------------+---------------+------*/
static bool AcceptTriangleUVWForRefinement (DPoint3dCR uvw)
    {
    static double s_acceptSize = 1.5;
    return    fabs (uvw.x-0.5) <= s_acceptSize
           && fabs (uvw.y-0.5) <= s_acceptSize
           && fabs (uvw.z-0.5) <= s_acceptSize;
    }
/**************
// Timing tests Sept 27 2013
// 13x18 poles.  moderate sinusoidal variation.
// order 3x4, orderFactor 2.
// BEFORE testing triangle uv within(-1,2) -- i.e. newton runs (and flails) on a lot of patches:
//  30% per-triangle search, 30% newton.
// AFTER testing (-1,2) 65% of time is in per-triangle search -- the newton time vanishes.
// i.e. the following are insignificant:
    patch extraction (saturate knots) 2%
    grid evaluation 6%
    Newton on promising patches 2%

// Same distribution for orderFactor=3 -- newton shows even less (better starting guesses==>shorter iteration.  Smaller Triangle==>fewer patches "live"
    IDEA 1:  Need fast preselect of good triangles (or patches?)  Doing the extraction twice doesn't cost much!!!!
    Maxsurf experience says range tree over triangles does it.  But that depends on amortizing the range tree construction over many searches.
    Can't to that here.
    Bezier extraction is cheap -- need to range tests on those.
    Pass 1: find bezier "closest" bezier patch by range containment.
    Pass 2: scan all beziers -- only do pertriangle on beziers that pass range+minDist test.

    IDEA 2: Time spent choosing triangle density is worthwhile.  (Fixed order factor is zero time -- put some effort here!!!)

    FOLLOWUP ON 2: Install ComputeDensity function based on deviation of poles from the midpoints of chords.
        Compared to prior fixed order factor:
          1) typically improve to 40 to 60% of prior time.
          2) When order is very close to pole count, END INTERVALS have significantly non-uniform parameterizations
                 due to clamping.  This (correctly!!) triggers higher counts and impairs performance.
**************/
size_t DefaultClosestPointGridDensity (size_t order)
    {
    static size_t s_orderFactor = 3;
    //static size_t s_baseCount = 4;
    if (order == 2)
        return 2;
    return s_orderFactor * order;
    }

static size_t s_num1 = 0;
static size_t s_num2 = 0;
static size_t s_num3 = 0;
static size_t s_num4 = 0;

/*---------------------------------------------------------------------------------**//**
* @bsimethod       
+---------------+---------------+---------------+---------------+---------------+------*/
size_t ComputeDensity (size_t defaultCount, size_t order, double linearError, double twistError)
    {
    double e = DoubleOps::Max (linearError, twistError);
    if (e < 0.15)
      return s_num1++,     order;
    if (e < 0.60)
      return s_num2++, 2 * order;
    if (e < 1.0)
      return s_num3++, 3 * order;
    return s_num4++,   4 * order;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                       
+---------------+---------------+---------------+---------------+---------------+------*/
static bool ClosestPointOnSurface (MSBsplineSurfaceCR surface, DPoint3dCR spacePoint, SolidLocationDetailR detail)
    {
    BSurfPatch patch;
    size_t numU, numV;
    surface.GetIntervalCounts (numU, numV);
    detail.Init ();
    detail.SetA (DBL_MAX);
    size_t uOrder = surface.GetUOrder ();
    size_t vOrder = surface.GetVOrder ();
    GridArrays grid;
    size_t numUGrid0 = DefaultClosestPointGridDensity (uOrder);
    size_t numVGrid0 = DefaultClosestPointGridDensity (vOrder);
    DPoint2d quadParam;
    DPoint3d quadXYZ;
    // final step of newton raphson might jiggle but not show distance improvement ... allow a little of that
    static double s_finalImprovementWindow = 1.00001;
    for (size_t i = 0; i < numU; i++)
        {
        for (size_t j = 0; j < numV; j++)
            {
            if (surface.GetPatch (patch, i, j)
              && !patch.isNullU
              && !patch.isNullV
              )
                {
                double uErrorFraction, vErrorFraction, twistErrorFraction;
                size_t numUGrid = numUGrid0;
                size_t numVGrid = numVGrid0;
                if (s_timerSelect != 5)
                    {
                    patch.MidpointDeviations (uErrorFraction, vErrorFraction, twistErrorFraction);
                    numUGrid = ComputeDensity (numUGrid0, uOrder, uErrorFraction, twistErrorFraction);
                    numVGrid = ComputeDensity (numVGrid0, vOrder, vErrorFraction, twistErrorFraction);
                    }
                DPoint3d triangleUVW, triangleUVW1;
                triangleUVW1.Init (DBL_MAX,DBL_MAX, DBL_MAX);
                if (s_timerSelect == 1)
                    surface.GetPatch (patch, i, j);
                SolidLocationDetail detail1;
                detail1.Init ();
                detail1.SetA (DBL_MAX);
                grid.EvaluateBezier (0.0, 1.0, numUGrid, 0.0, 1.0, numVGrid, patch, false, 0);
                if (s_timerSelect == 2)
                    grid.EvaluateBezier (0.0, 1.0, numUGrid, 0.0, 1.0, numVGrid, patch, false, 0);
                for (size_t i = 0; i + 1 < numUGrid; i++)
                    {
                    for (size_t j = 0; j + 1 < numVGrid; j++)
                        {
                        for (size_t t = 0; t <= 3; t += 2)
                            {
                            DPoint3d planeUVW;
                            if (grid.TryClosestPointByTriangle (i, j, t, spacePoint, quadParam, quadXYZ, triangleUVW, planeUVW))
                                {
                                if (s_timerSelect == 3)
                                    grid.TryClosestPointByTriangle (i, j, t, spacePoint, quadParam, quadXYZ, triangleUVW, planeUVW);
                                // EDL March 2016 After triangle search, the saved uv was the off-triangle projection to plane rather than the triangle edge.
                                //  The subsequent AcceptTriangleForUVRefinement test says that was intentional.
                                //  But an example case had fairly large out-of-triangle uvw and was rejected.
                                //  Changing to save the 0..1 triangleUVW instead of planeUVW.
                                DPoint3d patchXYZ;
                                patch.Evaluate (quadParam, patchXYZ);
                                double d = spacePoint.Distance (patchXYZ);
                                if (d < detail1.GetA ())
                                    {
                                    triangleUVW1 = triangleUVW;
                                    detail1.SetA (d);
                                    detail1.SetUV (quadParam);
                                    detail1.SetXYZ (patchXYZ);
                                    }
                                }
                            }
                        }
                    }
                if (detail1.GetA () < DBL_MAX)
                    {

                    // TODO only do patch refinement on patches that are "close" -- but the triangle proximity
                    // doesn't tell us if we are close to the interior.  So we do it (and probably wander around in space) for all patches..
                    // Cheap substitute is that AcceptTriangleUVWForRefinement rejects candidates whose projection onto the triangle is "way out there"
                    if (detail1.GetA () < detail.GetA ())
                          {
                          DPoint2d uvPatch = detail1.GetUV ();
                          detail = detail1;
                          detail.SetUV (patch.PatchUVToKnotUV (uvPatch));
                          }

                    if (AcceptTriangleUVWForRefinement (triangleUVW1))
                        {
                        if (s_timerSelect == 4)
                            {
                            SolidLocationDetail detail2 = detail1;
                            RefinePatchClosestPoint_interiorOnly (patch, spacePoint, 0.1, detail2);
                            }
                        if (RefinePatchClosestPoint_interiorOnly (patch, spacePoint, 0.1, detail1)
                            && detail1.GetA () <= s_finalImprovementWindow * detail.GetA ())
                            {
                            DPoint2d uvPatch = detail1.GetUV ();
                            detail = detail1;
                            detail.SetUV (patch.PatchUVToKnotUV (uvPatch));
                            }
                        }
                    }
                }
            }
        }
    return detail.GetA () < DBL_MAX;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                       
+---------------+---------------+---------------+---------------+---------------+------*/
size_t DefaultRayPierceGridDensity (size_t order)
    {
    if (order == 2)
        return 2;
    return 2 * order;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                       
+---------------+---------------+---------------+---------------+---------------+------*/
static bool RefinePatchRayIntersection (BSurfPatchCR patch, DRay3dCR ray, DPoint2dR patchParam, double &rayParam, DPoint3dR xyz)
    {
    // ray.origin + lambda * ray.Direction - patch(u,v) = 0
    // 3 equations.
    // jacobian
    //  [ray.Direction,  -patch.dXdu, -patch.dX.dv)
    size_t numConverged = 0;
    static double s_uvTol = 1.0e-10;
    static size_t s_targetNumConverged = 3;   /// 2 is good with exact derivatives -- seems to need an extra with approximate !!!!
    static size_t s_maxIterations = 15;
    RotMatrix jacobian;
    DPoint3d X;   // surface
    DPoint3d xyzRay;
    DVec3d errorVector;
    DVec3d updateVector;
    DVec3d   dXdu, dXdv;
    double f = rayParam;
    DPoint2d uv = patchParam;
    for (size_t iterations = 0; iterations < s_maxIterations; iterations++)
        {
        xyzRay = ray.FractionParameterToPoint (f);
        patch.Evaluate (uv, X, dXdu, dXdv);
        dXdu.Negate ();
        dXdv.Negate ();
        jacobian.InitFromColumnVectors (ray.direction, dXdu, dXdv);
        errorVector.DifferenceOf (xyzRay, X);
        if (!jacobian.Solve (updateVector, errorVector))
            return false;
        f -= updateVector.x;
        uv.x -= updateVector.y;
        uv.y -= updateVector.z;
        double e = sqrt (updateVector.y * updateVector.y + updateVector.z * updateVector.z);
        if (e < s_uvTol)
            {
            numConverged++;
            if (numConverged >= s_targetNumConverged)
                {
                patchParam = uv;
                rayParam = f;
                xyz = X;
                return true;
                }
            }
        else
            numConverged = 0;
        }
    return false;
    }

static bool RefinePatchCurveIntersection (BSurfPatchCR patch, ICurvePrimitiveCR curve, DPoint2dR patchParam, double &curveParam, DPoint3dR xyz)
    {
    // curve(f) - patch(u,v) = 0
    // 3 equations.
    // jacobian
    //  [curve.Direction,  -patch.dXdu, -patch.dX.dv]
    size_t numConverged = 0;
    static double s_uvTol = 1.0e-10;
    static size_t s_targetNumConverged = 3;   /// 2 is good with exact derivatives -- seems to need an extra with approximate !!!!
    static size_t s_maxIterations = 15;
    RotMatrix jacobian;
    DPoint3d X, Y;   // surface, curve
    DVec3d errorVector;
    DVec3d updateVector;
    DVec3d   dXdu, dXdv, dYdf;
    double f = curveParam;
    DPoint2d uv = patchParam;
    for (size_t iterations = 0; iterations < s_maxIterations; iterations++)
        {
        curve.FractionToPoint (f, Y, dYdf);
        patch.Evaluate (uv, X, dXdu, dXdv);
        dXdu.Negate ();
        dXdv.Negate ();
        jacobian.InitFromColumnVectors (dYdf, dXdu, dXdv);
        errorVector.DifferenceOf (Y, X);
        if (!jacobian.Solve (updateVector, errorVector))
            return false;
        f -= updateVector.x;
        uv.x -= updateVector.y;
        uv.y -= updateVector.z;
        double e = sqrt (updateVector.y * updateVector.y + updateVector.z * updateVector.z);
        if (e < s_uvTol)
            {
            numConverged++;
            if (numConverged >= s_targetNumConverged)
                {
                patchParam = uv;
                curveParam = f;
                xyz = X;
                return true;
                }
            }
        else
            numConverged = 0;
        }
    return false;
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod                       
+---------------+---------------+---------------+---------------+---------------+------*/
static double MidpointDeviation (DPoint4dCR point0, DPoint4dCR point1, DPoint4dCR point2, double cap = 10.0)
    {
    DPoint4d midpoint = DPoint4d::FromInterpolate (point0, 0.5, point2);
    double a, d01, d12;
    point1.RealDistanceSquared (&a, midpoint);
    point0.RealDistanceSquared (&d01, point1);
    point1.RealDistanceSquared (&d12, point2);
    if (a > d01 * cap)
        return cap;
    if (a > d12 * cap)
        return cap;
    return DoubleOps::Max (a / d01, a / d12);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                       
+---------------+---------------+---------------+---------------+---------------+------*/
static double TwistDeviation (DPoint4dCR point00, DPoint4dCR point01, DPoint4dCR point10, DPoint4d point11, double cap = 10.0)
    {
    DPoint4d pointA = DPoint4d::FromInterpolate (point00,0.5,  point11);
    DPoint4d pointB = DPoint4d::FromInterpolate (point10, 0.5, point01);
    double a, dA, dB;
    point00.RealDistanceSquared (&dA, point11);
    point10.RealDistanceSquared (&dB, point01);
    pointA.RealDistanceSquared (&a, pointB);
    if (a > dA * cap)
        return cap;
    if (a > dB * cap)
        return cap;
    return DoubleOps::Max (a / dA, a / dB);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                       
+---------------+---------------+---------------+---------------+---------------+------*/
bool BSurfPatch::MidpointDeviations (double &uFraction, double &vFraction, double &twistFraction) const
    {
    static double s_cap = 10.0;
    uFraction = vFraction = twistFraction = 0.0;
    if (uOrder < 2 && vOrder < 2)
        return false;
    // u stringers ...
    for (size_t j = 0; j < vOrder; j++)
        {
        size_t i0 = j * uOrder;
        for (size_t i = 0; i + 2 < uOrder; i++)
            {
            uFraction = DoubleOps::Max (uFraction, MidpointDeviation (
                  xyzw[i0 + i],
                  xyzw[i0 + i + 1],
                  xyzw[i0 + i + 2],
                  s_cap));
            }
        }

    // v stringers ...
    for (size_t i = 0; i < uOrder; i++)
        {
        size_t j0 = i;
        for (size_t j = 0; j + 2 < vOrder; j++)
            {
            vFraction = DoubleOps::Max (uFraction, MidpointDeviation (
                  xyzw[j0 + j],
                  xyzw[ j0 + j + uOrder],
                  xyzw[ j0 + j + 2 * uOrder],
                  s_cap));
            }
        }

    // quads ...
    for (size_t j = 0; j + 1 < vOrder; j++)
        {
        size_t i0 = j * uOrder;
        size_t i1 = (j + 1) * uOrder;
        for (size_t i = 0; i + 2 < uOrder; i++)
            {
            twistFraction = DoubleOps::Max (twistFraction,
                TwistDeviation (
                  xyzw[i0 + i], xyzw[i0 + i + 1],
                  xyzw[i1 + i], xyzw[i1 + i + 1],
                  s_cap));
            }
        }

    uFraction = sqrt (uFraction);
    vFraction = sqrt (vFraction);
    twistFraction = sqrt (twistFraction);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                       
+---------------+---------------+---------------+---------------+---------------+------*/
bool BSurfPatch::Evaluate (DPoint2dCR uv, DPoint3dR xyz) const
    {
    DPoint4d Q;
    TensorProducts::Evaluate (Q, uv, &xyzw[0], uOrder, vOrder);
    double divW;
    if (DoubleOps::SafeDivide (divW, 1.0, Q.w, 0.0))
        {
        Q.GetXYZ (xyz);
        xyz.Scale (divW);
        return true;
        }
    xyz.Zero ();
    return false;
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod                       
+---------------+---------------+---------------+---------------+---------------+------*/
bool BSurfPatch::Evaluate (DPoint2dCR uv, DPoint3dR xyz, DVec3dR dXdu, DVec3dR dXdv) const
    {
    DPoint4d Q, dQdu, dQdv;
    TensorProducts::Evaluate (Q, dQdu, dQdv, uv, &xyzw[0], uOrder, vOrder);
    double divW;
    if (DoubleOps::SafeDivide (divW, 1.0, Q.w, 0.0))
        {
        Q.GetXYZ (xyz);
        xyz.Scale (divW);
        dXdu.WeightedDifferenceOf (dQdu, Q);
        dXdu.Scale (divW * divW);
        dXdv.WeightedDifferenceOf (dQdv, Q);
        dXdv.Scale (divW * divW);
        return true;
        }
    xyz.Zero ();
    dXdu.Zero ();
    dXdv.Zero ();
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley 11/2015
+---------------+---------------+---------------+---------------+---------------+------*/
static bool PatchOverlapsRay (TransformCR rayTransform, DPoint3dCR rayOrigin, BSurfPatchCR patch)
    {
    DRange3d        patchRange = DRange3d::NullRange();

    for (auto& patchPoint : patch.xyzw)
        {
        DPoint4d        rayPatchPoint;

        rayTransform.Multiply (&rayPatchPoint, &patchPoint, 1);

        patchRange.Extend (rayPatchPoint);
        }

    static      double      s_overlapTolerance = 1.0E-8;

    return  patchRange.high.x > rayOrigin.x - s_overlapTolerance &&
            patchRange.high.y > rayOrigin.y - s_overlapTolerance &&
            patchRange.low.x  < rayOrigin.x + s_overlapTolerance &&
            patchRange.low.y  < rayOrigin.y + s_overlapTolerance;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod              
+---------------+---------------+---------------+---------------+---------------+------*/
static bool SurfaceIntersectRay (MSBsplineSurfaceCR surface, DRay3dCR ray, bvector<double> &rayParameters, bvector<DPoint2d> &surfaceParameters, bvector<DPoint3d> &surfaceXYZ)
    {
    rayParameters.clear ();
    surfaceParameters.clear ();
    surfaceXYZ.clear ();
    
    BSurfPatch patch;
    size_t numU, numV;
    surface.GetIntervalCounts (numU, numV);
    GridArrays grid;
    size_t uOrder = surface.GetUOrder ();
    size_t vOrder = surface.GetVOrder ();
    double rayParam[2];
    DPoint2d patchParam[2];
    DPoint3d patchXYZ[2];    
    size_t numUGrid = DefaultRayPierceGridDensity (uOrder);
    size_t numVGrid = DefaultRayPierceGridDensity (vOrder);
    RotMatrix       rayMatrix = RotMatrix::From1Vector (ray.direction, 2, true);

    rayMatrix.InverseOf (rayMatrix);
    Transform       rayTransform = Transform::FromMatrixAndFixedPoint (rayMatrix, ray.origin);


    // TODO -- optimize on 2x2 -- just use the poles directly.
    // TODO -- prefilter polygon pierce ????
    // TODO -- apply chord/angle tolerance logic to adjust grid density
    for (size_t iPatch = 0; iPatch < numU; iPatch++)
        {
        for (size_t jPatch = 0; jPatch < numV; jPatch++)
            {
            if (surface.GetPatchSupport (patch, iPatch, jPatch)
                && PatchOverlapsRay (rayTransform, ray.origin, patch)          // First test unsaturated...
                && patch.DoSaturate()       
                && !patch.isNullU 
                && !patch.isNullV
                && PatchOverlapsRay (rayTransform, ray.origin, patch)          // Test again after saturation...
                )
                {
                grid.EvaluateBezier (0.0, 1.0, numUGrid, 0.0, 1.0, numVGrid, patch, false, 0);
                for (size_t i = 0; i + 1 < numUGrid; i++)
                    {
                    for (size_t j = 0; j + 1 < numVGrid; j++)
                        {
                        size_t numK;
                        if (grid.TryRayPierce (i, j, ray, patchParam, rayParam, patchXYZ, numK))
                            {
                            for (size_t k = 0; k < numK; k++)
                                {
                                if (RefinePatchRayIntersection (patch, ray, patchParam[k], rayParam[k], patchXYZ[k]))
                                    {
                                    rayParameters.push_back (rayParam[k]);
                                    surfaceParameters.push_back (patch.PatchUVToKnotUV (patchParam[k]));
                                    surfaceXYZ.push_back (patchXYZ[k]);                                                                   
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    return true;
    }





/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2015
+---------------+---------------+---------------+---------------+---------------+------*/
PatchRangeDataPtr MSBsplineSurface::ComputePatchRangeData() const
    {
    size_t                  numU, numV;
    PatchRangeDataPtr     patchRangeData = PatchRangeData::Create();

    bvector<DPoint3d>   patchPoles;

    GetIntervalCounts (numU, numV);

    for (size_t uIndex = 0; uIndex < numU; uIndex++)
        {
        for (size_t vIndex = 0; vIndex < numV; vIndex++)
            {
            BSurfPatch      patch;

            if (GetPatch (patch, uIndex, vIndex) && !patch.isNullU && !patch.isNullV)
                {
                patchPoles.clear();
                for (auto& xyzw :patch.xyzw)
                    {
                    DPoint3d    xyz;
                    
                    if (xyzw.GetProjectedXYZ (xyz))
                        patchPoles.push_back (xyz);
                    }

                TaggedLocalRange localRange (uIndex, vIndex, 0.0);

                localRange.InitFromPrincipalAxesOfPoints (patchPoles);
                patchRangeData->m_ranges.push_back (localRange);
                }
            }
        }
    return patchRangeData;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void MSBsplineSurface::FastIntersectRay (DRay3dCR ray, PatchRangeDataPtr& patchRangeData, bvector<double> &rayParameters, bvector<DPoint2d> &surfaceParameters, bvector<DPoint3d> &surfaceXYZ) const
    {
    if (!patchRangeData.IsValid())
        patchRangeData = ComputePatchRangeData();

    GridArrays      grid;
    size_t          numUGrid = DefaultRayPierceGridDensity (GetUOrder());
    size_t          numVGrid = DefaultRayPierceGridDensity (GetVOrder());


    for (auto& localRange : patchRangeData->m_ranges)
        {
        DRay3d      localRay;
        double      param0, param1;
        DPoint3d    point0, point1;
        BSurfPatch  patch;

        localRange.m_worldToLocal.Multiply (localRay, ray);

        if (localRange.m_localRange.IntersectRay (param0, param1, point0, point1, localRay.origin, localRay.direction) &&
            (param0 > 0.0 || param1 > 0.0) &&
            GetPatch (patch, localRange.m_indexA, localRange.m_indexB))
            {
            grid.EvaluateBezier (0.0, 1.0, numUGrid, 0.0, 1.0, numVGrid, patch, false, 0);
            for (size_t i = 0; i + 1 < numUGrid; i++)
                {
                for (size_t j = 0; j + 1 < numVGrid; j++)
                    {
                    double      rayParam[2];
                    DPoint2d    patchParam[2];
                    DPoint3d    patchXYZ[2];
                    size_t      numK;

                    if (grid.TryRayPierce (i, j, ray, patchParam, rayParam, patchXYZ, numK))
                        {
                        for (size_t k = 0; k < numK; k++)
                            {
                            if (RefinePatchRayIntersection (patch, ray, patchParam[k], rayParam[k], patchXYZ[k]) && rayParam[k] > 0.0)
                                {
                                rayParameters.push_back (rayParam[k]);
                                surfaceParameters.push_back (patch.PatchUVToKnotUV (patchParam[k]));
                                surfaceXYZ.push_back (patchXYZ[k]);                                                                   
                                }
                            }
                        }
                    }
                }
            }
        }
    }

 
/*---------------------------------------------------------------------------------**//**
* @bsimethod              
+---------------+---------------+---------------+---------------+---------------+------*/
void MSBsplineSurface::IntersectRay
(
bvector<DPoint3d> &intersectionPoints,  //!< returned intersection points
bvector<double>   &rayParameters,       //!< returned parameters on the ray
bvector<DPoint2d> &surfaceParameters,   //!< returned parameters on the surface
DRay3dCR ray
) const
    {
    SurfaceIntersectRay (*this, ray, rayParameters, surfaceParameters, intersectionPoints);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod              
+---------------+---------------+---------------+---------------+---------------+------*/
void MSBsplineSurface::IntersectRay
(
bvector<DPoint3d> &intersectionPoints,  //!< returned intersection points
bvector<double>   &rayParameters,       //!< returned parameters on the ray
bvector<DPoint2d> &surfaceParameters,   //!< returned parameters on the surface
DRay3dCR ray,
DRange1dCR rayParameterInterval
) const
    {
    SurfaceIntersectRay (*this, ray, rayParameters, surfaceParameters, intersectionPoints);

    size_t numAccept = 0;
    for (size_t i = 0; i < rayParameters.size (); i++)
        {
        if (rayParameterInterval.Contains (rayParameters[i]))
            {
            rayParameters[numAccept] = rayParameters[i];
            intersectionPoints[numAccept] = intersectionPoints[i];
            surfaceParameters[numAccept] = surfaceParameters[i];
            numAccept++;
            }
        }
    rayParameters.resize (numAccept);
    intersectionPoints.resize (numAccept);
    surfaceParameters.resize (numAccept);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod              
+---------------+---------------+---------------+---------------+---------------+------*/
//! Find closest point on surface
void MSBsplineSurface::ClosestPoint (DPoint3dR surfacePoint, DPoint2dR surfaceUV, DPoint3dCR spacePoint) const
    {
    SolidLocationDetail detail;
    ClosestPointOnSurface (*this, spacePoint, detail);
    surfaceUV = detail.GetUV ();
    surfacePoint = detail.GetXYZ ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                            12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void MSBsplineSurface::FastIntersectCurve
(
ICurvePrimitiveCR curve,                    //!< [in] curve primitive.
PatchRangeDataPtr& patchRangeData,          //!< [in,out] cached mesh details
bvector<CurveAndSolidLocationDetail> &intersectionPoints     //!< [out] hit points on curve
) const
    {
    BSurfPatch patch;
    if (!patchRangeData.IsValid())
        {
        patchRangeData = ComputePatchRangeData();
        // um... don't allow planar surfaces to have 0 thickness in local range ...
        double extension = 1.0e-8;
        for (auto &localRange : patchRangeData->m_ranges)
            localRange.m_localRange.Extend (extension);
        }
    intersectionPoints.clear ();
    bvector<PartialCurveDetail> boxIntersections;
    GridArrays      grid;
    size_t          numUGrid = DefaultRayPierceGridDensity (GetUOrder());
    size_t          numVGrid = DefaultRayPierceGridDensity (GetVOrder());
    bvector<CurveAndSolidLocationDetail> gridHits;
    DRange3d curveWorldRange;
    curve.GetRange (curveWorldRange);
    for (auto& localRange : patchRangeData->m_ranges)
        {
        if (curveWorldRange.IntersectsWith (localRange.m_worldRange))
            {
            boxIntersections.clear ();
            curve.AppendCurveRangeIntersections (localRange, boxIntersections);
            for (auto &partialCurveDetail : boxIntersections)
                {
                if (GetPatch (patch, localRange.m_indexA, localRange.m_indexB))
                    {
                    grid.EvaluateBezier (0.0, 1.0, numUGrid, 0.0, 1.0, numVGrid, patch, false, 0);
                    DRange3d partialCurveRange;
                    ICurvePrimitivePtr partialCurve = partialCurveDetail.parentCurve->CloneBetweenFractions (partialCurveDetail.fraction0, partialCurveDetail.fraction1, false);
                    if (partialCurve.IsValid ())
                        {
                        partialCurve->GetRange (partialCurveRange);
                        for (size_t i = 0; i + 1 < numUGrid; i++)
                            {
                            for (size_t j = 0; j + 1 < numVGrid; j++)
                                {
                                grid.AppendCurvePierce (i, j, *partialCurve, partialCurveRange, gridHits);
                                for (auto & hit : gridHits)
                                    {
                                    // iterate within the partial curve, map to parent?
                                    DPoint2d uvPatch = hit.m_solidDetail.GetUV ();
                                    double f = hit.m_curveDetail.fraction;
                                    DPoint3d X;
                                    DVec3d dXdu, dXdv;
                                    if (RefinePatchCurveIntersection (patch, *partialCurve, uvPatch, f, X))
                                        {
                                        DPoint2d uvSurface = patch.PatchUVToKnotUV (uvPatch);
                                        double parentFraction = partialCurveDetail.ChildFractionToParentFraction (f);
                                        // reevaluate with improved curve and surface points
                                        EvaluatePoint (X, dXdu, dXdv, uvSurface.x, uvSurface.y);
                                        CurveAndSolidLocationDetail detail = hit;
                                        detail.m_solidDetail.SetUV (uvSurface.x, uvSurface.y, dXdu, dXdv);
                                        detail.m_solidDetail.SetXYZ (X);
                                        detail.m_solidDetail.SetPickParameter (parentFraction);
                                        if (partialCurveDetail.parentCurve->FractionToPoint (parentFraction, detail.m_curveDetail))
                                            {
                                            intersectionPoints.push_back (detail);
                                            }
                                        }
                                    }
                                gridHits.clear ();
                                }
                            }
                        }
                    }
                }
            }
        }
    }



#define EmulateBsprsurf_minDistToSurface
#ifdef EmulateBsprsurf_minDistToSurface
/*---------------------------------------------------------------------------------**//**
Known callers 130919 EDL
1) bsprsruf_blendRails
2) bspsurf_imposeBoundaryBySweptCurve -- used only after failure by local newton.
3) bspsurf_trimmedPlaneFromCurves -- planar surface !?!!?
        (3 call sites)
4) bspplank -- initial iterates from starter segment
* @bsimethod                                                    BFP             12/90
+---------------+---------------+---------------+---------------+---------------+------*/

Public GEOMDLLIMPEXP int      bsprsurf_minDistToSurface
(
double*             distance,           /* <= distance to closest point on curve */
DPoint3dP           minPt,              /* <= closest point on curve */
DPoint2dP           param,              /* <= parameter of closest point */
DPoint3dCP          testPt,             /* => point to calculate dist from */
MSBsplineSurface*   surface,            /* => input surface */
double*             tolerance           /* => tolerance to use in calculation */
)
    {
    DPoint3d xyz;
    DPoint2d uv;
    surface->ClosestPoint (xyz, uv, *testPt);
    if (NULL != distance)
        *distance = xyz.Distance (*testPt);
    if (NULL != minPt)
        *minPt = xyz;
    if (NULL != param)
        *param = uv;
    return SUCCESS;
    }
#endif    

struct GridProcessor
{
MSBsplineSurfaceCR m_surface;
size_t m_numUGrid, m_numVGrid;
bool m_reverseU;
bool m_evaluateDerivatives;
BSurfPatch      patch;
GridArrays      grid;

GridProcessor
(
MSBsplineSurfaceCR surface,
bool reverseU = false,
bool evaluateDerivatives = false
)
    : m_surface (surface),
      m_reverseU (reverseU),
      m_evaluateDerivatives (evaluateDerivatives)
      {
      }
// virtual method for a patch processor
struct IGridPatchProcessor
{
virtual void Process (MSBsplineSurfaceCR surface, BSurfPatch &patch, size_t uIndex, size_t vIndex,
    GridArrays &grid, size_t numU, size_t numV) = 0;
};
void Process (IGridPatchProcessor &patchProcessor)
    {
    size_t                  numU, numV;
    m_surface.GetIntervalCounts (numU, numV); // number of bezier patches in each direction
    GridArrays      grid;
    size_t          numUGrid = DefaultRayPierceGridDensity (m_surface.GetUOrder());
    size_t          numVGrid = DefaultRayPierceGridDensity (m_surface.GetVOrder());

    for (size_t uIndex = 0; uIndex < numU; uIndex++)
        {
        for (size_t vIndex = 0; vIndex < numV; vIndex++)
            {
            if (m_surface.GetPatch (patch, uIndex, vIndex) && !patch.isNullU && !patch.isNullV)
                {
                grid.EvaluateBezier (0.0, 1.0, numUGrid, 0.0, 1.0, numVGrid, patch, m_reverseU, m_evaluateDerivatives);
                patchProcessor.Process (m_surface, patch, uIndex, vIndex, grid, numUGrid, numVGrid);
                }
            }
        }

    }
};

struct PatchMomentSums : GridProcessor::IGridPatchProcessor
{
private:
DMatrix4d m_sums;
DPoint3d m_origin;
public:
PatchMomentSums (DPoint3dCR origin) {Clear (); m_origin = origin;}
void Clear (){ m_sums = DMatrix4d::FromZero ();}

virtual void Process (MSBsplineSurfaceCR surface, BSurfPatch &patch, size_t uIndex, size_t vIndex,
    GridArrays &grid, size_t numU, size_t numV) override
    {
    grid.ResolveCoordinatesAndNormals (&surface, false);
    for (auto &xyz : grid.xyz)
        DPoint3dOps::AccumulateToMomentSumUpperTriangle (m_sums, m_origin, xyz);
    }
DMatrix4d GetSymmetricSums ()
    {
    DMatrix4d sums = m_sums;
    sums.CopyUpperTriangleToLower ();
    return sums;
    }
};

struct PatchRangeAccumulator: GridProcessor::IGridPatchProcessor
{
private:
DRange3d m_range;
Transform m_transform;
public:
PatchRangeAccumulator (TransformCR transform) : m_transform (transform) {Clear ();}
void Clear (){ m_range.Init ();}

virtual void Process (MSBsplineSurfaceCR surface, BSurfPatch &patch, size_t uIndex, size_t vIndex,
    GridArrays &grid, size_t numU, size_t numV) override
    {
    grid.ResolveCoordinatesAndNormals (&surface, false);
    m_range.Extend (m_transform, grid.xyz);
    }
DRange3d GetRange () {return m_range;}
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz 02/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool MSBsplineSurface::TightPrincipalExtents (TransformR originWithExtentVectors, TransformR centroidAlLocalToWorld, TransformR centroidalWorldToLocal, DRange3dR centroidalRange) const
    {
    DPoint3d pole0 = GetUnWeightedPole (0);
    PatchMomentSums summer (pole0);
    GridProcessor surfaceProcessor (*this);
    surfaceProcessor.Process (summer);
    DMatrix4d sums = summer.GetSymmetricSums ();
    centroidAlLocalToWorld.InitIdentity ();
    centroidalWorldToLocal.InitIdentity ();
    originWithExtentVectors.InitIdentity ();
    centroidalRange.Init ();
    DVec3d centroid;
    RotMatrix axes;
    DVec3d moments;
    double volume;
    if (sums.ConvertInertiaProductsToPrincipalMoments (volume, centroid, axes, moments))
        {
        Transform localToWorld = Transform::From (axes, pole0 + centroid);
        Transform worldToLocal;
        worldToLocal.InverseOf (localToWorld);
        PatchRangeAccumulator localRangeAccumulator (worldToLocal);
        surfaceProcessor.Process (localRangeAccumulator);
        DRange3d localRange = localRangeAccumulator.GetRange ();
        return DPoint3dOps::LocalRangeToOrderedExtents (localToWorld, localRange, originWithExtentVectors, centroidAlLocalToWorld, centroidalWorldToLocal, centroidalRange);
        }
    return false;
    }


struct PatchPointAccumulator: GridProcessor::IGridPatchProcessor
{
private:
DPoint3dDoubleUVArrays &m_data;
double m_patchCount;
public:
PatchPointAccumulator (DPoint3dDoubleUVArrays &data) : m_data(data), m_patchCount (0) {}

virtual void Process (MSBsplineSurfaceCR surface, BSurfPatch &patch, size_t uIndex, size_t vIndex,
    GridArrays &grid, size_t numU, size_t numV) override
    {
    grid.ResolveCoordinatesAndNormals (&surface, false);
    for (size_t i = 0; i < grid.xyz.size(); i++)
        {
        m_data.m_xyz.push_back (grid.xyz[i]);
        m_data.m_f.push_back (m_patchCount);
        m_data.m_uv.push_back (grid.uv[i]);   // need to map up to surface !!!
        }
    m_patchCount += 1.0;
    }
};

    //! Return array of points on the surface.
    //! The points data is:
    //!<ul>
    //!<li>m_xyz = coordinates
    //!<li>m_f = index of bezier patch.
    //!<li>m_uv = surface parametric coordinate
    //!</ul>
void MSBsplineSurface::EvaluatePoints (DPoint3dDoubleUVArrays &data, IFacetOptionsCP options) const
    {
    GridProcessor surfaceProcessor (*this);
    data.m_xyz.clear ();
    data.m_f.clear ();
    data.m_uv.clear ();
    PatchPointAccumulator accumulator (data);
    surfaceProcessor.Process (accumulator);
    }

END_BENTLEY_GEOMETRY_NAMESPACE    