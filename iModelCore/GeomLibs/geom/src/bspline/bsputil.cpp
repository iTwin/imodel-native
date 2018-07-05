/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/bspline/bsputil.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include "msbsplinemaster.h"

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

#define MDLERR_INSFMEMORY ERROR
#define MDLERR_NOKNOTS ERROR
#define MDLERR_NOPOLES ERROR
#define MDLERR_BADPARAMETER ERROR
#if !defined (FONTIMPORTER)
/*----------------------------------------------------------------------+
|                                                                       |
|   Local Defines                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
#define PARALLEL_TOLERANCE                  1.0e-12

/*----------------------------------------------------------------------+
|                                                                       |
|   Local type definitions                                              |
|                                                                       |
+----------------------------------------------------------------------*/

/*----------------------------------------------------------------------+
|                                                                       |
| name          sortUtil_returnMinimum                                  |
|                                                                       |
| author        BFP                                     1/91            |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int      sortUtil_returnMinimum
(
double          *minimum,
double          *list,
int             number
)
    {
    int         i, tag;
    double      *listPtr;

    *minimum = list[tag=0];
    for (i=1, listPtr=list+1; i < number; i++, listPtr++)
        if (*listPtr < *minimum)
            {
            *minimum = *listPtr;
            tag = i;
            }

    return (tag);
    }


/*----------------------------------------------------------------------+
|                                                                       |
|    B-Spline Utility Routines                                          |
|                                                                       |
+----------------------------------------------------------------------*/
#endif /* !FONTIMPORTER */

void MSBsplineCurve::KnotToBlendFunctions
(
double *pBlendFunctions,
double *pDerivatives,
size_t &knotIndex,
double knotValue
) const
    {
    int order = params.order;
    int numKnots = NumberAllocatedKnots ();
    double xMax = knots[numKnots - order];
    int index;
    bsputil_knotToBlendingFuncs (pBlendFunctions, pDerivatives, &index, knots, knotValue, xMax, order, params.closed);
    knotIndex = (size_t)index;
    }
    
void MSBsplineSurface::KnotToBlendFunctions
(
double *pBlendFunctions,
double *pDerivatives,
size_t &knotIndex,
double knotValue,
int direction
) const
    {
    if (direction == BSSURF_U)
        {
        int order = uParams.order;
        int numKnots = (int)GetNumUKnots ();
        double xMax = uKnots[numKnots - order];
        int index;
        bsputil_knotToBlendingFuncs (pBlendFunctions, pDerivatives, &index, uKnots, knotValue, xMax, order, uParams.closed);
        knotIndex = (size_t)index;
        }
    else
        {
        int order = vParams.order;
        int numKnots = (int)GetNumVKnots ();
        double xMax = vKnots[numKnots - order];
        int index;
        bsputil_knotToBlendingFuncs (pBlendFunctions, pDerivatives, &index, vKnots, knotValue, xMax, order, vParams.closed);
        knotIndex = (size_t)index;
        }
    }
    
/*----------------------------------------------------------------------+
|                                                                       |
| name          bsputil_knotToBlendingFuncs                            |
|                                                                       |
| author        RBB                                     6/89            |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void     bsputil_knotToBlendingFuncs
(
double          *coefs,
double          *dCoefs,
int             *left,
double const    *knots,
double          u,
double          uMax,
int             order,
int             closed
)
    {
    double      dMinus[MAX_ORDER * 2], dPlus[MAX_ORDER * 2];
    double      denom, m, n, nextC, nextDc;
    double const *tPtr;
    double      *coefsS, *dCoefsS, *dPlusS, *dMinusS;
    double const *leftKnot;
    double const *rghtKnot;
    double      *coefsR, *dCoefsR, *dPlusR, *dMinusR, *coefsEnd;
    int         iLeft, s;
    // umm.. we access knots[order-1] for uMin.  But expect uMax from the arg list.
    double uMin = knots[order-1];
    if (u < uMin)
        u = uMin;
    if (u > uMax)
        u = uMax;

    for (tPtr=knots+1; (u >= *tPtr && *tPtr < uMax); tPtr++);
    *left = iLeft = (int)(tPtr - knots);
    if (closed && mdlBspline_knotsShouldBeOpened (knots, 2*order, NULL, NULL, 0, order, closed))
        *left -= order/2;

    if (dCoefs) dCoefs[0] = 0.0;
    coefs [0] = 1.0;
    for (s=0, dPlusS=dPlus, dMinusS=dMinus, coefsS=coefs+1, dCoefsS=dCoefs+1,
         rghtKnot=knots+iLeft, leftKnot = rghtKnot-1;
         s < order-1;
         dPlusS++, dMinusS++, coefsS++, dCoefsS++, leftKnot--, rghtKnot++, s++)
        {
        *dPlusS = *rghtKnot - u;
        *dMinusS = u - *leftKnot;
        nextC = nextDc = 0.0;
        for(coefsEnd=coefs+s, coefsR=coefs, dCoefsR=dCoefs, dPlusR=dPlus, dMinusR=dMinus+s;
            coefsR <= coefsEnd;
            coefsR++, dCoefsR++, dPlusR++, dMinusR--)
            {
            denom = *dPlusR + *dMinusR;
            m = denom == 0.0 ? *coefsR : (*coefsR / denom);
            *coefsR = nextC + *dPlusR * m;
            nextC = *dMinusR * m;
            if (dCoefs)
                {
                n = denom == 0.0 ? *dCoefsR : (*dCoefsR / denom);
                *dCoefsR = nextDc + *dPlusR * n - m;
                nextDc = *dMinusR * n + m;
                }
            }
        *coefsS = nextC;
        if (dCoefs) *dCoefsS = nextDc;
        }
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          bsputil_computeBlendingFunctions                        |
|                                                                       |
| author        RBB                                     03/2016         |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void     bsputil_computeBlendingFunctions
(
double          *coefs,
double          *dCoefs,
int             *left,
double const    *knots,
double          u,
int             numPoles,
int             order,
int             closed
)
    {
    // Unlike the obsolete bsputil_computeBlendingFuncs - calculate a correct uMax from the knots.
    // not providing correct uMax can cause bsputil_knotToBlendingFuncs to go off the end of the knot
    // vector causing a memory access violation...   (Ray Bentley, 03/2016).
   double       uMax = closed ? knots[numPoles + order-1] : knots[numPoles];

   return bsputil_knotToBlendingFuncs (coefs, dCoefs, left, knots, u, uMax, order, closed);
   }

/*----------------------------------------------------------------------+
|                                                                       |
| name          bsputil_computeBlendingFuncs                            |
|                                                                       |
| author        RBB                                     6/89            |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void     bsputil_computeBlendingFuncs
(
double          *coefs,
double          *dCoefs,
int             *left,
double const    *knots,
double          fraction,
double          uMax,
int             order,
int             closed
)
    {
    /* make sure it is natural parameter for V8 -Lu Han */
    double u = fraction * uMax + knots[order-1] * (1.0 - fraction);
    return bsputil_knotToBlendingFuncs (coefs, dCoefs, left, knots, u, uMax, order, closed);
    }

#if !defined (FONTIMPORTER)
/*----------------------------------------------------------------------+
|                                                                       |
| name          bsputil_blendingsForSecondPars                          |
|                                                                       |
| author        LuHan                                   2/92            |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void     bsputil_blendingsForSecondPars
(
double          *coefs,
double          *dCoefs,
double          *ddCoefs,
int             *left,
double const    *knots,
double          u,
double          uMax,
int             order,
int             closed
)
    {
    double      dMinus[MAX_ORDER * 2], dPlus[MAX_ORDER * 2];
    double      denom, m, n, l, nextC, nextDc;
    double const *tPtr;
    double      *coefsS, *dCoefsS, *dPlusS, *dMinusS;
    double const *leftKnot;
    double const *rghtKnot;
    double      *coefsR, *dCoefsR, *dPlusR, *dMinusR, *coefsEnd;
    double      nextDDc, *ddCoefsS, *ddCoefsR;
    int         iLeft, s;

    if (u < 0.0) u = 0.0;
    if (u > uMax)    u = uMax;

    for (tPtr=knots+1; (u >= *tPtr && *tPtr < uMax); tPtr++);
    *left = iLeft = (int)(tPtr - knots);
    if (closed && mdlBspline_knotsShouldBeOpened (knots, 2*order, NULL, NULL, 0, order, closed))
        *left -= order/2;

    if (dCoefs) dCoefs[0] = 0.0;
    if (ddCoefs) ddCoefs[0] = 0.0;

    coefs [0] = 1.0;
    for (s=0, dPlusS=dPlus, dMinusS=dMinus, coefsS=coefs+1, dCoefsS=dCoefs+1,
         ddCoefsS=ddCoefs+1, rghtKnot=knots+iLeft, leftKnot = rghtKnot-1;
         s < order-1;
         dPlusS++, dMinusS++, coefsS++, dCoefsS++, ddCoefsS++,
         leftKnot--, rghtKnot++, s++)
        {
        *dPlusS = *rghtKnot - u;
        *dMinusS = u - *leftKnot;
        nextC = nextDc = nextDDc = 0.0;
        for(coefsEnd=coefs+s, coefsR=coefs, dCoefsR=dCoefs, ddCoefsR=ddCoefs,
            dPlusR=dPlus, dMinusR=dMinus+s;
            coefsR <= coefsEnd;
            coefsR++, dCoefsR++, ddCoefsR++, dPlusR++, dMinusR--)
            {
            denom = *dPlusR + *dMinusR;
            m = *coefsR / denom;
            *coefsR = nextC + *dPlusR * m;
            nextC = *dMinusR * m;
            n = 0;  // ??? what iv dDoefs is null but ddCoefs is not?
            if (dCoefs)
                {
                n = *dCoefsR / denom;
                *dCoefsR = nextDc + *dPlusR * n - m;
                nextDc = *dMinusR * n + m;
                }
            if (ddCoefs)
                {
                l = *ddCoefsR / denom;
                *ddCoefsR = nextDDc + *dPlusR * l - 2.0 * n;
                nextDDc = *dMinusR * l + 2.0 * n;
                }
            }
        *coefsS = nextC;
        if (dCoefs) *dCoefsS = nextDc;
        if (ddCoefs) *ddCoefsS = nextDDc;
        }
    }

static double sAbsTol = 1.0e-8;
static double sRelTol = 1.0e-8;
static double sWeightTol = 1.0e-10;

/*--------------------------------------------------------------------*//**
Return a tolerance appropriate to an array of points.
@param pXYZ IN array of points.
@param pWeights IN array of weights.  REMARK: library policy may choose to ignore weights
    and compute the tolerance using the raw xyz.
@return computed tolerance based on largest coordinate in poles.
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP double bsputil_pointTolerance
(
DPoint3dCP  pXYZ,
double const *pWeights,
int         numXYZ
)
    {
    double a = bsiDPoint3d_getLargestCoordinate (pXYZ, numXYZ);
    double tol = sAbsTol + sRelTol * a;
    // Hmm..... Do I really want this?
    if (tol > 1.0)
        tol = 1.0;
    return tol;
    }

/*--------------------------------------------------------------------*//**
@param curveP IN subject curve
@return computed tolerance based on largest coordinate in poles.
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP double bsputil_curveTolerance
(
MSBsplineCurveCP curve
)
    {
    return bsputil_pointTolerance (curve->poles, curve->weights, curve->params.numPoles);
    }

/*--------------------------------------------------------------------*//**
@param surface IN subject surface
@return computed tolerance based on largest coordinate in poles.
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP double bsputil_surfaceTolerance
(
MSBsplineSurfaceCP surface
)
    {
    int numPoles = surface->uParams.numPoles * surface->vParams.numPoles;
    return bsputil_pointTolerance (surface->poles, surface->weights, numPoles);
    }


/*--------------------------------------------------------------------*//**
Test if two weights are equal within system tolerance.
@param wA IN first weight
@param wB IN second weight
@return true if same within tolerance.
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool    bsputil_isSameWeight
(
double  wA,
double  wB
)
    {
    return fabs (wA-wB) < sWeightTol;
    }


/*--------------------------------------------------------------------*//**
Test if two points are equal, using caller's tolerance.
@param pPointA IN first point
@param pPointB IN second point
@return true if max absolute coordinate difference is within computed tolerance.
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool    bsputil_isSamePointTolerance
(
DPoint3dCP pPointA,
DPoint3dCP pPointB,
double     tolerance
)
    {
    if (tolerance <= 0.0)
        return DPoint3dOps::AlmostEqual (*pPointA, *pPointB);
    double maxDiff = pPointA->MaxDiff(*pPointB);
    return maxDiff < tolerance;
    }

/*--------------------------------------------------------------------*//**
Test if two points are equal, using global absolute and relative tolerances.
@param pPointA IN first point
@param pPointB IN second point
@return true if max absolute coordinate difference is within computed tolerance.
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool    bsputil_isSamePoint
(
DPoint3dCP pPointA,
DPoint3dCP pPointB
)
    {
    double maxDiff = pPointA->MaxDiff(*pPointB);
    if (maxDiff < sAbsTol)
        return true;
    double maxAbs = bsiTrig_maxAbsDPoint3dDPoint3d (pPointA, pPointB);
    return maxDiff < sAbsTol + sRelTol * maxAbs;
    }

/*--------------------------------------------------------------------*//**
Test if two points and weights are equal, using global absolute and relative tolerances on xyz parts.
Weights must match to absolute tolerance.
@param p0 IN first point
@param w0 IN first weight.
@param p1 IN second point
@param w1 IN second weight.
@return true if weights and xyz parts both match.
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool     bsputil_isSameRationalPoint
(
DPoint3dCP p0,
double     w0,
DPoint3dCP p1,
double     w1
)
    {
    return fabs (w0-w1) < sWeightTol && bsputil_isSamePoint (p0, p1);
    }

/*--------------------------------------------------------------------*//**
Test if two points and weights are equal, using caller's xyz tolerance
Weights must match to absolute tolerance.
@param p0 IN first point
@param w0 IN first weight.
@param p1 IN second point
@param w1 IN second weight.
@return true if weights and xyz parts both match.
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool     bsputil_isSameRationalPointTolerance
(
DPoint3dCP p0,
double     w0,
DPoint3dCP p1,
double     w1,
double     xyzTolerance
)
    {
    return bsputil_isSameWeight (w0, w1) && bsputil_isSamePointTolerance (p0, p1, xyzTolerance);
    }


/*--------------------------------------------------------------------*//**
+----------------------------------------------------------------------*/

bool MSBsplineCurve::AlmostEqual (MSBsplineCurveCR other) const
    {
    int         i;

    if (this->params.numPoles == other.params.numPoles &&
        this->params.numKnots == other.params.numKnots &&
        this->params.closed   == other.params.closed &&
        this->rational        == other.rational)
        {
        if (this->rational)
            {
            for (i=0; i<this->params.numPoles; i++)
                if (! bsputil_isSameRationalPoint (&this->poles[i], this->weights[i],
                                                   &other.poles[i], other.weights[i]))
                    return false;
            }
        else
            {
            for (i=0; i<this->params.numPoles; i++)
                if (! bsputil_isSamePoint (&this->poles[i], &other.poles[i]))
                    return false;
            }
        return true;
        }
    else
        {
        return false;
        }
    }

/*--------------------------------------------------------------------*//**
+----------------------------------------------------------------------*/
bool MSBsplineCurve::AlmostEqual (MSBsplineCurveCR other, double tolerance) const
    {
    int         i;

    if (   this->GetNumPoles () == other.GetNumPoles ()
        && this->GetNumKnots () == other.GetNumKnots ()
        && this->GetOrder () == other.GetOrder ()
        && this->IsClosed ()   == other.IsClosed ()
        && this->HasWeights ()        == other.HasWeights ()
        )
        {
        if (this->rational)
            {
            for (i=0; i<this->params.numPoles; i++)
                if (! bsputil_isSameRationalPointTolerance (&this->poles[i], this->weights[i],
                                                   &other.poles[i], other.weights[i], tolerance))
                    return false;
            }
        else
            {
            for (i=0; i<this->params.numPoles; i++)
                if (! bsputil_isSamePointTolerance (&this->poles[i], &other.poles[i], tolerance))
                    return false;
            }
        return true;
        }
    else
        {
        return false;
        }
    }


/*----------------------------------------------------------------------+
|                                                                       |
| name          bsputil_edgeCode                                        |
|                                                                       |
| author        BrianPeters                             4/93            |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int      bsputil_edgeCode
(
DPoint2dCP      uv,
double          tolerance
)
    {
    int         code;
    double      near1;

    near1 = 1.0 - tolerance;
    code = NO_EDGE;

    if (uv->x <= tolerance)
        code |= U0_EDGE;
    else if (uv->x >= near1)
        code |= U1_EDGE;

    if (uv->y <= tolerance)
        code |= V0_EDGE;
    else if (uv->y >= near1)
        code |= V1_EDGE;

    return code;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          bsputil_edgeCode                                        |
|                                                                       |
| author        BrianPeters                             4/93            |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int      bsputil_edgeCode
(
DPoint2dCR uv0,
DPoint2dCR uv1,
double          tolerance
)
    {
    double a0 = tolerance;
    double a1 = 1.0 - tolerance;
    if (uv0.x <= a0 && uv1.x <= a0)
        return U0_EDGE;
    if (uv0.x >= a1 && uv1.x >= a1)
        return U1_EDGE;
    if (uv0.y <= a0 && uv1.y <= a0)
        return V0_EDGE;
    if (uv0.y >= a1 && uv1.y >= a1)
        return V1_EDGE;
    return NO_EDGE;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          bsputil_onEdge                                          |
|                                                                       |
| author        BFP                                     4/91            |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool     bsputil_onEdge
(
DPoint2dCP      uv,
double          tolerance
)
    {
    double    near1 = 1.0 - tolerance;

    return (uv->x < tolerance || uv->x > near1 ||
            uv->y < tolerance || uv->y > near1);
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          bsputil_onEdge                                          |
|                                                                       |
| author        BFP                                     4/91            |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool     bsputil_countPointsToEdgeBreak
(
DPoint2dCP   uv,
int i0,
int numPoint,
double tolerance,
int    &edgeCode,
int    &numOnEdge
)
    {
    edgeCode = NO_EDGE;
    double a0 = tolerance;
    double a1 = 1.0 - tolerance;

    if (uv[i0].x <= a0)
        {
        numOnEdge = 1;
        int i1 = i0 + 1;
        while (i1 < numPoint && uv[i1].x < a0)
            {
            i1++;
            }
        if (i1 - i0 > 1)
            {
            numOnEdge = i1 - i0;
            edgeCode = U0_EDGE;
            return true;
            }
        }


    if (uv[i0].x >= a1)
        {
        numOnEdge = 1;
        int i1 = i0 + 1;
        while (i1 < numPoint && uv[i1].x >= a1)
            {
            i1++;
            }
        if (i1 - i0 > 1)
            {
            numOnEdge = i1 - i0;
            edgeCode = U1_EDGE;
            return true;
            }
        }

    if (uv[i0].y <= a0)
        {
        numOnEdge = 1;
        int i1 = i0 + 1;
        while (i1 < numPoint && uv[i1].y < a0)
            {
            i1++;
            }
        if (i1 - i0 > 1)
            {
            numOnEdge = i1 - i0;
            edgeCode = V0_EDGE;
            return true;
            }
        }


    if (uv[i0].y >= a1)
        {
        numOnEdge = 1;
        int i1 = i0 + 1;
        while (i1 < numPoint && uv[i1].y >= a1)
            {
            i1++;
            }
        if (i1 - i0 > 1)
            {
            numOnEdge = i1 - i0;
            edgeCode = V1_EDGE;
            return true;
            }
        }
    edgeCode = NO_EDGE;
    numOnEdge = 1;
    int i1 = i0 + 1;
    while (i1 < numPoint)
        {
        if (NO_EDGE != bsputil_edgeCode (&uv[i1], tolerance))
            {
            i1++;
            break;
            }
        i1++;
        }
    if (i1 - i0 > 1)
        {
        numOnEdge = i1 - i0;
        edgeCode = NO_EDGE;
        return true;
        }
    edgeCode = NO_EDGE;
    numOnEdge = 0;
    return false;
    }
/*----------------------------------------------------------------------+
|                                                                       |
| name          bsputil_closestEdge                                     |
|               Go to closest edge in UV space                          |
| author        BFP                                     2/91            |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void     bsputil_closestEdge
(
DPoint2d        *edgePt,
DPoint2d        *testPt
)
    {
    double      tmp, deltaX, deltaY;

    deltaX = testPt->x < (tmp = 1.0 - testPt->x) ? testPt->x : tmp;
    deltaY = testPt->y < (tmp = 1.0 - testPt->y) ? testPt->y : tmp;

    if (deltaX < deltaY)
        {
        edgePt->x = testPt->x < 0.5 ? 0.0 : 1.0;
        edgePt->y = testPt->y;
        }
    else
        {
        edgePt->x = testPt->x;
        edgePt->y = testPt->y < 0.5 ? 0.0 : 1.0;
        }
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          bsputil_extractScaledValues                             |
|                                                                       |
| author        RBB                                     6/89            |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void     bsputil_extractScaledValues
(
double          *weights,
double          *igdsWeights,
int             numPoles
)
    {
    double      *lP;
    double      *wP, *endP;

    for (lP=igdsWeights, wP=endP=weights, endP += numPoles; wP < endP; lP++, wP++)
        *wP = *lP;
    }

#endif /* !FONTIMPORTER */
/*----------------------------------------------------------------------+
|                                                                       |
| name          bsputil_loadPoles                                       |
|                                                                       |
| author        BFP                                     11/90           |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void     bsputil_loadPoles
(
DPoint3d        *poles,                /* <= poles that define curve at u */
double          *weights,              /* <= weights (if rational) */
DPoint3d        *fullPoles,            /* => all poles of curve */
double          *fullWeights,          /* => all  weights (if rational) */
int             start,                 /* => index of first pole */
int             numPoles,
int             order,
int             closed,
int             rational
)
    {
    int          shift;

    if (start < 0)
        {
        shift = numPoles + start;
        memcpy (poles, fullPoles + shift, -start * sizeof(DPoint3d));
        memcpy (poles - start, fullPoles, (order + start) * sizeof(DPoint3d));
        if (rational)
            {
            memcpy (weights, fullWeights+shift, -start * sizeof(double));
            memcpy (weights - start, fullWeights, (order + start) * sizeof(double));
            }
        }
    else if (start > (numPoles - order))
        {
        shift = numPoles - start;
        memcpy (poles, fullPoles + start,  shift * sizeof(DPoint3d));
        memcpy (poles+shift, fullPoles, (order - shift) * sizeof(DPoint3d));
        if (rational)
            {
            memcpy (weights, fullWeights + start,  shift * sizeof(double));
            memcpy (weights + shift, fullWeights, (order-shift) * sizeof(double));
            }
        }
    else
        {
        memcpy (poles, fullPoles + start, order * sizeof(DPoint3d));
        if (rational)
            memcpy (weights, fullWeights + start, order * sizeof(double));
        }
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          bsputil_relevantPoles                                   |
|                                                                       |
| author        BFP                                     11/90           |
|                                                                       |
+----------------------------------------------------------------------*/
static void    bsputil_relevantPoles
(
DPoint3d        *poles,                /* <= poles that define curve at u */
double          *weights,              /* <= weights (if rational) */
int             *rght,                 /* <= kV[rght-1] <= u < kV[rght] */
int             *start,                /* <= index of first pole */
DPoint3d        *fullPoles,            /* => all poles of curve */
double          *fullWeights,          /* => all  weights (if rational) */
double          u,                     /* => parameter to find */
double          *knotVector,           /* => full knot vector (kV) */
int             numPoles,
int             order,
int             closed,
int             rational,
bool            bCorrect               /* => to compensate for fake periodic knots */
)
    {
    double      *kP, *lastKnotP;

    lastKnotP = knotVector + bspknot_numberKnots (numPoles, order, closed);
    for (kP=knotVector; u >= *kP && kP < lastKnotP; kP++)
        ;

    *rght = *start = (int)(kP - knotVector);
    if (closed && bCorrect)
        *start -= order/2;
    *start -= order;

    bsputil_loadPoles (poles, weights, fullPoles, fullWeights, *start,
                       numPoles, order, closed, rational);
    }

#if !defined (FONTIMPORTER)
/*----------------------------------------------------------------------+
|                                                                       |
| name          bsputil_swap                                            |
|                                                                       |
| author        RBB                                     3/90            |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void     bsputil_swap
(
MSBsplineCurve  *curve1,
MSBsplineCurve  *curve2
)
    {
    MSBsplineCurve  tmp;

    tmp     = *curve1;
    *curve1 = *curve2;
    *curve2 = tmp;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          bsputil_flushStrokes                                    |
|                                                                       |
| author        BFP                                     10/90           |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int      bsputil_flushStrokes
(
DPoint3d        *out,
int             strokes,
DPoint3d        **buffer,
int             *bufSize
)
    {
    int         allocSize;
    DPoint3d    *bPtr;

    if (strokes > 1)
        {
        if (*bufSize == 0)
            {
            allocSize = strokes * sizeof (DPoint3d);
            if (NULL == (bPtr = (DPoint3dP)dlmSystem_mdlMalloc (allocSize)))
                return MDLERR_INSFMEMORY;
            *buffer = bPtr;
            *bufSize = strokes;
            memcpy (*buffer, out, allocSize);
            }
        else
            {
            allocSize = (*bufSize + strokes) * sizeof (DPoint3d);
            if (NULL == (bPtr = (DPoint3dP)dlmSystem_mdlRealloc (*buffer, allocSize)))
                return MDLERR_INSFMEMORY;
            else
                {
                *buffer = bPtr;
                memcpy ((*buffer) + *bufSize, out, strokes*sizeof(DPoint3d));
                *bufSize += strokes;
                }
            }
        }

    return SUCCESS;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          bsputil_returnStrokes                                   |
|                                                                       |
| author        BFP                                     10/90           |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int      bsputil_returnStrokes
(
DSegment3d*      rvec,
DPoint3d        out[],
int             *strokes,
DPoint3d        **destination,
int             *destSize
)
    {
    int         status;

    if (*strokes && !bsputil_isSamePoint (&rvec->point[0], out + *strokes - 1))
        {
        if (SUCCESS != (status = bsputil_flushStrokes (out, *strokes, destination, destSize)))
            return status;
        *strokes = 0;
        }

    if (! *strokes)
        {
        out[0] = rvec->point[0];
        *strokes = 1;
        }

    out[*strokes] = rvec->point[1];
    (*strokes)++;
    if (*strokes >= MAX_CLIPBATCH - 1)
        {
        if (SUCCESS != (status = bsputil_flushStrokes (out, *strokes, destination, destSize)))
            return status;
        out[0] = out[*strokes-1];
        *strokes = 1;
        }

    return SUCCESS;
    }

#endif /* !FONTIMPORTER */
/*----------------------------------------------------------------------+
|                                                                       |
| name          bsputil_weightPoles                                     |
|                                                                       |
| author        RBB                                     3/90            |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void     bsputil_weightPoles
(
DPoint3d        *weightedPoles,
DPoint3d        *poles,
double          *weights,
int             numPoles
)
    {
    double      *wtP, *endP;
    DPoint3d    *poleP, *wtPoleP;

    if (NULL == weights)
        {
        memcpy (weightedPoles, poles, numPoles * sizeof(DPoint3d));
        }
    else
        {
        for (wtP=endP=weights, poleP=poles, wtPoleP=weightedPoles, endP += numPoles;
             wtP < endP; wtP++, poleP++, wtPoleP++)
            wtPoleP->Scale (*poleP, *wtP);
        }
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          bsputil_unWeightPoles                                   |
|                                                                       |
| author        RBB                                     3/90            |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void     bsputil_unWeightPoles
(
DPoint3d        *poles,
DPoint3d        *weightedPoles,
double          *weights,
int             numPoles
)
    {
    double      *wtP, *endP;
    DPoint3d    *poleP, *wtPoleP;

    if (NULL == weights)
        {
        memcpy (poles, weightedPoles, numPoles * sizeof(DPoint3d));
        }
    else
        {
        for (wtP=endP=weights, poleP=poles, wtPoleP=weightedPoles, endP += numPoles;
             wtP < endP; wtP++, poleP++, wtPoleP++)
            poleP->Scale (*wtPoleP, 1.0 / *wtP);
        }
    }


#if !defined (FONTIMPORTER)
/*----------------------------------------------------------------------+
|                                                                       |
| name          bsputil_regularizeWeights                               |
|               sets end weights both to the value 1.0                  |
|               WARNING: interior weights may exceed the value 1.0      |
|               See Farin, 3rd edition, page 236.                       |
| author        BrianPeters                             3/93            |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void     bsputil_regularizeWeights
(
MSBsplineCurve  *curve
)
    {
    int         last, i;
    double      pHat, factor, lastWt, *wP;

    if (! curve->rational)
        return ;

    bsputil_unWeightPoles (curve->poles, curve->poles, curve->weights,
                           curve->params.numPoles);

    last = curve->params.numPoles - 1;
    lastWt = curve->weights[last];
    pHat = pow (lastWt / curve->weights[0], 1.0 / last);

    for (i = 0, wP = curve->weights + last, factor = 1.0;
         i < curve->params.numPoles; i++, wP--, factor *= pHat)
        *wP *= factor / lastWt;

    bsputil_weightPoles (curve->poles, curve->poles, curve->weights,
                         curve->params.numPoles);
    }

#endif /* !FONTIMPORTER */
/*----------------------------------------------------------------------+
|                                                                       |
| name          bsputil_cumulativeDistance                              |
|                                                                       |
| author        RBB                                     3/90            |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP double   bsputil_cumulativeDistance
(
DPoint3d        *points,
double          *weights,
int             rational,
int             numPoints
)
    {
    int         i;
    double      sum = 0.0;
    DPoint3d    *pPtr;

    if (rational)
        bsputil_unWeightPoles (points, points, weights, numPoints);

    for (i=1, pPtr=points; i<numPoints; i++, pPtr++)
        sum += pPtr->Distance (pPtr[1]);

    if (rational)
        bsputil_weightPoles (points, points, weights, numPoints);

    return (sum);
    }


#if !defined (FONTIMPORTER)
/*----------------------------------------------------------------------+
|                                                                       |
| name          computeNormalAllowDegenerate                            |
|                                                                       |
| author        BrianPeters                             1/92            |
|                                                                       |
+----------------------------------------------------------------------*/
static double  computeNormalAllowDegenerate
(
DPoint3d        *norm,
DPoint3d        *p0,
DPoint3d        *p1
)
    {
    double      mag = norm->NormalizedDifference (*p0, *p1);

    if (mag < fc_1em15)
        norm->x = 0.0;

    return (mag);
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          isLinearArray                                           |
|                                                                       |
| author        BrianPeters                             1/92            |
|                                                                       |
+----------------------------------------------------------------------*/
static bool    isLinearArray
(
DPoint3d        *points,
int             numPoints,
double          cosineTol
)
    {
    DPoint3d    *p0, *p1, *endP, diff0, diff1;

    for (p0=p1=endP=points, p1 += 1, endP += numPoints; p1 < endP; p0++, p1++)
        {
        if (diff0.NormalizedDifference (*p1, *p0) > fc_1em15)
            break;
        }

    /* If all points are degerate return true */
    if (p1 == endP)
        return (true);

    for (p0++, p1++; p1 < endP; p0++, p1++)
        {
        if (diff1.NormalizedDifference (*p1, *p0) > fc_1em15)
            {
            if (diff0.DotProduct (diff1) < cosineTol)
                return (false);
            diff0 = diff1;
            }
        }

    return (true);
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          containsInflection                                      |
|                                                                       |
| author        BrianPeters                             1/92            |
|                                                                       |
+----------------------------------------------------------------------*/
static bool     containsInflection
(
DPoint3d        *points,
int             numPoints
)
    {
    int         previous=false;
    DPoint3d    *p0, *p1, *endP, diff0, diff1, thisB, lastB;

    for (p0=p1=endP=points, p1 += 1, endP += numPoints; p1 < endP; p0++, p1++)
        {
        if (diff0.NormalizedDifference (*p1, *p0) > fc_1em15)
            break;
        }

    /* If all points are degerate return false */
    if (p1 >= endP)
        return (false);

    for (p0++, p1++; p1 < endP; p0++, p1++)
        {
        if (diff1.NormalizedDifference (*p1, *p0) > fc_1em15)
            {
            thisB.CrossProduct (diff0, diff1);
            if (previous &&
                thisB.DotProduct (lastB) < 0.0)
                return (true);
            else
                previous = true;

            lastB = thisB;
            diff0 = diff1;
            }
        }
    return (false);
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          bsputil_isLinearArray                                   |
|                                                                       |
| author        BrianPeters                             1/92            |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool     bsputil_isLinearArray
(
DPoint3d        *poles,
double          *weights,
int             rational,
int             numPoles,
double          cosineTol
)
    {
    bool        code;
    DPoint3d    local[MAX_ORDER];

    if (rational)
        {
        if (numPoles < MAX_ORDER)
            {
            bsputil_unWeightPoles (local, poles, weights, numPoles);
            code = isLinearArray (local, numPoles, cosineTol);
            }
        else
            {
            bsputil_unWeightPoles (poles, poles, weights, numPoles);
            code = isLinearArray (poles, numPoles, cosineTol);
            bsputil_weightPoles (poles, poles, weights, numPoles);
            }
        }
    else
        code = isLinearArray (poles, numPoles, cosineTol);

    return (code);
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          bsputil_containsInflection                              |
|                                                                       |
| author        BrianPeters                             1/92            |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool     bsputil_containsInflection
(
DPoint3d        *poles,
double          *weights,
int             rational,
int             numPoles
)
    {
    bool        code;
    DPoint3d    local[MAX_ORDER];

    if (rational)
        {
        if (numPoles < MAX_ORDER)
            {
            bsputil_unWeightPoles (local, poles, weights, numPoles);
            code = containsInflection (local, numPoles);
            }
        else
            {
            bsputil_unWeightPoles (poles, poles, weights, numPoles);
            code = containsInflection (poles, numPoles);
            bsputil_weightPoles (poles, poles, weights, numPoles);
            }
        }
    else
        code = containsInflection (poles, numPoles);

    return (code);
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          bsputil_polygonTangent                                  |
|                                                                       |
| author        BrianPeters                             1/92            |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void     bsputil_polygonTangent
(
DPoint3d        *tangent,
MSBsplineCurve  *bezier,
int             endFlag
)
    {
    int         last, lastM1;
    DPoint3d    tmp0, tmp1;

    if (endFlag)
        lastM1 = (last = bezier->params.numPoles - 1) - 1;
    else
        last = (lastM1 = 0) + 1;

    if (bezier->rational)
        {
        tmp0.Scale (*(bezier->poles + lastM1), 1.0 / bezier->weights[lastM1]);
        tmp1.Scale (*(bezier->poles + last), 1.0 / bezier->weights[last]);
        computeNormalAllowDegenerate (tangent, &tmp1, &tmp0);
        }
    else
        computeNormalAllowDegenerate (tangent, bezier->poles + last, bezier->poles + lastM1);
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          bsputil_polygonBiNormal                                 |
|               Returns the first non-degenerate calculated bi-normal   |
|               by getting the cross product of succesive polygon       |
|               legs. Starts from the specified end of the control      |
|               polygon.                                                |
| author        BrianPeters                             1/92            |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void     bsputil_polygonBiNormal
(
DPoint3d        *normal,
MSBsplineCurve  *curve,
int             endFlag
)
    {
    int         incr;
    DPoint3d    diff0, diff1, local[MAX_ORDER], *poleP, *p0, *p1, *endP;

    normal->x = normal->y = normal->z = 0.0;

    if (curve->rational)
        {
        if (curve->params.numPoles <= MAX_ORDER)
            {
            bsputil_unWeightPoles (local, curve->poles, curve->weights, curve->params.order);
            poleP = local;
            }
        else
            {
            bsputil_unWeightPoles (curve->poles, curve->poles, curve->weights, curve->params.order);
            poleP = curve->poles;
            }
        }
    else
        poleP = curve->poles;

    if (endFlag)
        {
        incr = -1;
        poleP += curve->params.numPoles - 1;
        }
    else
        incr = 1;

    for (p0=p1=endP=poleP, p1 += incr, endP += incr * curve->params.numPoles;
         p1 != endP; p0 += incr, p1 += incr)
        {
        if (diff0.NormalizedDifference (*p1, *p0) > fc_1em15)
            break;
        }

    /* If all points are degerate return zero vector */
    if (p1 == endP)
        return;

    for (p0 += incr, p1 += incr; p1 != endP; p0 += incr, p1 += incr)
        {
        if (diff1.NormalizedDifference (*p1, *p0) > fc_1em15)
            {
            normal->CrossProduct (diff0, diff1);
            break;
            }
        }

    if (endFlag)
        normal->Scale (*normal, -1.0);

    if (curve->rational && curve->params.numPoles > MAX_ORDER)
        bsputil_weightPoles (curve->poles, curve->poles, curve->weights,
                             curve->params.numPoles);
    }

#endif /* !FONTIMPORTER */
/*----------------------------------------------------------------------+
|                                                                       |
| name          bsputil_extractNormal                                   |
|               from extract_elemDescrNormal                            |
| author        RBB                                      6/91           |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void     bsputil_extractNormal
(
DPoint3d        *normalP,               /* <= curve normal */
DPoint3d        *positionP,             /* <= point on plane (taken directly from points) */
double          *planarDeviationP,      /* <= deviation from planar */
DPoint3d        *points,
int             numPoints,
DPoint3d        *defaultNormalP
)
    {
    int         i;
    DPoint3d    *poleP, *nextP, position;
    DVec3d      normal, areaNormal;
    /*
    This code uses the average area vector to calculate the areanormal.
    This produces very tiny errors that appear to be due to
    some sort of roundoff..... The bsiGeom_planeThroughPoints
    function seems to behave more reasonably, so it is used to calclulate
    the actual value and the area normal is used only to get direction.  RBB 11/00.
    */
    areaNormal.x = areaNormal.y = areaNormal.z = 0.0;

    for (i=0, poleP=points; i<numPoints; i++, poleP++)
        {
        nextP = (i==numPoints-1) ? points : poleP + 1;

        areaNormal.x += (poleP->y - nextP->y) * (poleP->z + nextP->z);
        areaNormal.y += (poleP->z - nextP->z) * (poleP->x + nextP->x);
        areaNormal.z += (poleP->x - nextP->x) * (poleP->y + nextP->y);
        }

    // position is one of the widely separated points---it is NOT an average point!
    if (bsiGeom_planeThroughPoints (&normal, &position, points, numPoints))
        {
        if (normal.DotProduct (areaNormal) < 0.0)
            normal.Scale (normal, -1.0);

        normal.Normalize ();
        }
    else
        {
        RotMatrix   matrix;
        DPoint3d    pt0 = {0,0,0}, pt1;
        DVec3d direction;
        double      mag = 0.0;
        int         index0, index1;

        if (numPoints > 0)
            pt0 = points[0];

        // points are colinear; find their direction
        if (bsiGeom_findWidelySeparatedPoints (&pt0, &index0, &pt1, &index1, points, numPoints))
            mag = index0 < index1 ? direction.NormalizedDifference (pt1, pt0) : direction.NormalizedDifference (pt0, pt1);

        if (defaultNormalP)
            normal = *(DVec3dP)defaultNormalP;
        else
            {
            normal.x = normal.y = 0.0;
            normal.z = 1.0;
            }

        // use default normal if it is perpendicular to line, or if points are coincident
        if (mag > 0.0 && !normal.IsPerpendicularTo (direction))
            {
            if (normal.IsParallelTo (direction))
                {
                matrix.InitFrom1Vector (direction, 0, true);
                matrix.GetColumn (normal, 2);
                }
            else
                {
                matrix.InitFrom2Vectors (direction, normal);
                matrix.SquareAndNormalizeColumns (matrix, 0, 1);
                matrix.GetColumn (normal, 1);
                }
            }

        // TR #123809: define position even if !positionP (needed for planarDeviation calculation)
        position = pt0;
        }

    if (normalP)
        *normalP = normal;

    if (positionP)
        *positionP = position;

    if (planarDeviationP)
        {
        double deviation, positionZ = normal.DotProduct (position);

        for (i=0, poleP=points; i<numPoints; i++, poleP++)
            {
            deviation = fabs (normal.DotProduct (*poleP) - positionZ);

            if (i==0 || deviation > *planarDeviationP)
                *planarDeviationP = deviation;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @description Compute a normal from a cloud of possibly weighted points.
*
* @remarks If the point cloud is determined to be nonplanar under the specified tolerance,
*       the returned normal is for an average plane.
*
* @param pNormal            OUT     normal from poles (or NULL)
* @param pOrigin            OUT     average of unweighted poles, projected to computed plane (or NULL)
* @param pIsPlanar          OUT     whether poles' maximum planar deviation <= absoluteTolerance + relativeTolerance * rangeDiagonal (or NULL)
* @param pPoles             IN      point cloud.  If NULL != pWeights, computations are performed on unweighted poles, which are restored on return.
* @param pWeights           IN      positive weights, if poles are weighted (or NULL)
* @param numPoles           IN      number of poles (and weights)
* @param pDefaultNormal     IN      default normal to use in computing normal if poles are colinear/coincident (or NULL to use 001)
* @param absoluteTolerance  IN      unscaled component of planarity tolerance (or -1 for default)
* @param relativeTolerance  IN      range fraction for scaled component of planarity tolerance (or -1 for default)
* @return SUCCESS if at least one pole is given.
* @bsimethod                                                    DavidAssaf      02/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       mdlBspline_computeNormal
(
DPoint3d*       pNormal,
DPoint3d*       pOrigin,
bool*           pIsPlanar,
DPoint3d*       pPoles,
const double*   pWeights,
int             numPoles,
const DPoint3d* pDefaultNormal,
double          absoluteTolerance,
double          relativeTolerance
)
    {
    DPoint3d    defaultNormal = {0,0,1};
    DPoint3d    defaultOrigin = {0,0,0};
    DPoint3d    normal, origin;
    double      planarDeviation;

    if (!pPoles || numPoles <= 0)
        return ERROR;

    // init outputs
    if (pNormal)
        *pNormal = pDefaultNormal ? *pDefaultNormal : defaultNormal;
    if (pOrigin)
        {
        *pOrigin = pPoles ? pPoles[0] : defaultOrigin;
        if (pPoles && pWeights)
            pOrigin->Scale (1.0 / pWeights[0]);
        }
    if (pIsPlanar)
        *pIsPlanar = false;

    if (absoluteTolerance < 0.0)
        absoluteTolerance = 1.0e-14;
    if (relativeTolerance < 0.0)
        relativeTolerance = 1.0e-8;

    if (pWeights)
        bsputil_unWeightPoles (pPoles, pPoles, const_cast<double*>(pWeights), numPoles);

    bsputil_extractNormal (&normal, &origin, &planarDeviation, pPoles, numPoles, const_cast<DPoint3d*>(pDefaultNormal));

    if (pNormal)
        *pNormal = normal;

    if (pOrigin || pIsPlanar)
        {
        double tol = absoluteTolerance;

        if (relativeTolerance)
            tol += relativeTolerance * bsiDPoint3d_getLargestCoordinateDifference (pPoles, numPoles);

        // compute average pole and project to plane
        if (pOrigin)
            {
            DPoint3d    averageUnweightedPole = {0,0,0};
            int         i, numPts = numPoles;

            // common case of duplicate endpts skews average
            if (numPoles > 2 && pPoles[0].IsEqual (pPoles[numPoles - 1], tol))
                numPts--;

            for (i = 0; i < numPts; i++)
                averageUnweightedPole.Add (pPoles[i]);

            averageUnweightedPole.Scale (1.0 / numPts);

            bsiDPoint3d_distancePointToPlane (pOrigin, &averageUnweightedPole, &normal, &origin);
            }

        if (pIsPlanar)
            *pIsPlanar = planarDeviation <= tol;
        }

    if (pWeights)
        bsputil_weightPoles (pPoles, pPoles, const_cast<double*>(pWeights), numPoles);

    return SUCCESS;
    }


/*----------------------------------------------------------------------+
|                                                                       |
|   B-Spline Knot Vector Routines                                       |
|                                                                       |
+----------------------------------------------------------------------*/
/*----------------------------------------------------------------------+
|                                                                       |
| name          bspknot_bezierKnotVector                                |
|                                                                       |
| author        LuHan                                   5/93            |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP double   *bspknot_bezierKnotVector
(
int             order
)
    {
    static double bezierKnotVector[2*MAX_ORDER] =
        {
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
        1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0
        };

    return bezierKnotVector + MAX_ORDER - order;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          bspknot_numberKnots                                     |
|                                                                       |
| author        BFP                                     10/90           |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int      bspknot_numberKnots
(
int             numPoles,
int             order,
int             closed
)
    {
    return (closed ? numPoles + 2*order - 1 : numPoles + order);
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          bspknot_sameKnot                                        |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool    bspknot_sameKnot
(
double knotA,
double knotB
)
    {
    double e = fabs (knotB - knotA);
    double max = 1.0;
    if (fabs (knotA) > max)
        max = fabs (knotA);
    if (fabs (knotB) > max)
        max = fabs (knotB);
    return e < KNOT_TOLERANCE_BASIS * max;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          bspknot_knotTolerance                                   |
|                                                                       |
| author        RBB                                     3/90            |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP double   bspknot_knotTolerance
(
MSBsplineCurveCP curve
)
    {
    int         i, numKnots;
    double      dist, *knots, knotTolerance, minKnot, maxKnot;

    if (!curve->knots)     return MDLERR_NOKNOTS;
    if (!curve->poles)     return MDLERR_NOPOLES;

    knotTolerance = KNOT_TOLERANCE_BASIS;
    knots = curve->knots;
    numKnots = bspknot_numberKnots (curve->params.numPoles, curve->params.order,
                                    curve->params.closed);

    /* Adjust the knot tolerance by the inverse of the control poly length */
    dist = bsputil_cumulativeDistance (curve->poles, curve->weights, curve->rational,
                                       curve->params.numPoles);
    if (dist < 1.0) dist = 1.0;
    // WOW -- divde by real polygon distance?   ARGHHHHHHHHHHHHHHHHHHHHHHHHHBFP
    knotTolerance /= dist;

    /* Increase knot tolerance by delta between interior knots */
    minKnot = knots[curve->params.order-1];
    maxKnot = knots[numKnots - curve->params.order];
    knotTolerance *= fabs (maxKnot - minKnot);

    /* loop over knots to make sure tolerance < minimum delta between knots */
    for (i=curve->params.order; i <= numKnots - curve->params.order; i++)
        {
        if (fabs (knots[i] - knots[i-1]) < knotTolerance)
            {
            if (fabs (knots[i] - knots[i-1]) > fc_epsilon)
                {
                /* make sure it's not a multiple knot */
                knotTolerance = fabs (knots[i] - knots[i-1]) / 10.0;
                }
            }
        }

    // TR #172772: don't return a knotTolerance so small that barely distinct knots coalesce upon normalization
    return knotTolerance < mgds_fc_nearZero ? mgds_fc_nearZero : knotTolerance;
    }

/*----------------------------------------------------------------------+
| author        EDL                                             04/13
+----------------------------------------------------------------------*/
bool MSBsplineCurve::ComputeUniformKnots ()
    {
    int numKnots = NumberAllocatedKnots ();
    if (knots == NULL)
        AllocateKnots (numKnots);
    return SUCCESS == bspknot_computeKnotVector (knots, &params, NULL);
    }
    
/*----------------------------------------------------------------------+
| author        EDL                                             04/13
+----------------------------------------------------------------------*/
bool MSBsplineSurface::ComputeUniformUKnots ()
    {
    if (uKnots == NULL)
        AllocateUKnots ();
    return SUCCESS == bspknot_computeKnotVector (uKnots, &uParams, NULL);
    }

/*----------------------------------------------------------------------+
| author        EDL                                             04/13
+----------------------------------------------------------------------*/
bool MSBsplineSurface::ComputeUniformVKnots ()
    {
    if (vKnots == NULL)
        AllocateVKnots ();
    return SUCCESS == bspknot_computeKnotVector (vKnots, &vParams, NULL);
    }

/*----------------------------------------------------------------------+
| author        EDL                                             04/13
+----------------------------------------------------------------------*/
bool MSBsplineSurface::SetUParamsWithUniformKnots (size_t numPoles, size_t order, bool closed)
    {
    if (order <= numPoles && order <= MAX_ORDER)
        {
        uParams.order = (int)order;
        uParams.numPoles = (int)numPoles;
        uParams.closed = closed;
        if (SUCCESS == AllocateUKnots ())
            return SUCCESS == bspknot_computeKnotVector (uKnots, &uParams, NULL);
        }
    return false;
    }

/*----------------------------------------------------------------------+
| author        EDL                                             04/13
+----------------------------------------------------------------------*/
bool MSBsplineSurface::SetVParamsWithUniformKnots (size_t numPoles, size_t order, bool closed)
    {
    if (order <= numPoles && order <= MAX_ORDER)
        {
        vParams.order = (int)order;
        vParams.numPoles = (int)numPoles;
        vParams.closed = closed;
        if (SUCCESS == AllocateVKnots ())
            return SUCCESS == bspknot_computeKnotVector (vKnots, &vParams, NULL);
        }
    return false;
    }



/*----------------------------------------------------------------------+
|                                                                       |
| name          bspknot_computeKnotVector                               |
|                                                                       |
| author        BFP                                     3/90            |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int      bspknot_computeKnotVector
(
double          *knotVector,        /* <= Full knot vector */
BsplineParam    *params,            /* => B-Spline parameters */
double          *interiorKnots      /* => interior knots (if nonuniform) */
)
    {
    /* THIS FUNCTION ASSUMES THE INTERIOR KNOTS ARE NORMALIZED!!! */

    int         i, numIntrKnots;
    double      currKnot, delta, *kP0, *kP1, *kP2, *kP3;

    if (knotVector == NULL)
        return MDLERR_NOKNOTS;

    knotVector[params->order-1] = 0.0;
    numIntrKnots = bspknot_numberKnots (params->numPoles, params->order, params->closed)
                 - 2*params->order;

    if (params->numKnots && NULL != interiorKnots)
        {
        memcpy (knotVector+params->order, interiorKnots, numIntrKnots * sizeof (double));
        }
    else    /* uniform case */
        {
        delta = 1.0/(double) (numIntrKnots + 1);
        currKnot = 0.0;
        for (i=0, kP1=knotVector+params->order; i < numIntrKnots; i++, kP1++)
             *kP1 = (currKnot += delta);
        }

    if (params->closed)
        {
        for (i=0,
             kP0=knotVector,
             kP1=knotVector+params->order-1,
             kP2=knotVector+numIntrKnots+1,
             kP3=knotVector+numIntrKnots+params->order;
             i < params->order;
             i++, kP0++, kP1++, kP2++, kP3++)
                {
                *kP0 = *kP2 - 1.0;
                *kP3 = *kP1 + 1.0;
                }
        }
    else
        {
        for (i=0, kP0=knotVector, kP3=knotVector+numIntrKnots+params->order;
             i < params->order;
             i++, kP0++, kP3++)
                {
                *kP0 = 0.0;
                *kP3 = 1.0;
                }
        }

    return SUCCESS;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          bspknot_computeKnotVectorNotNormalized                  |
|                                                                       |
| author        LuHan                                   03/01           |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int      bspknot_computeKnotVectorNotNormalized
(
double          *knotVector,        /* <= Full knot vector */
BsplineParam    *params,            /* => B-Spline parameters */
double          *interiorKnots      /* => interior knots (if nonuniform) */
)
    {
    memcpy (knotVector, interiorKnots, (params->numPoles + params->order) * sizeof (double));
    return SUCCESS;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          bspknot_normalizeKnotVector                             |
|                                                                       |
| author        BFP                                     3/90            |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int      bspknot_normalizeKnotVector
(
double          *knotVector,
int             numPoles,
int             order,
int             closed
)
    {
    int         i, numKnots;
    double      root, divisor, *kP0, *kP1, *kP2, *kP3;

    if (knotVector == NULL)    return MDLERR_NOKNOTS;

    numKnots = bspknot_numberKnots (numPoles, order, closed);
    root = knotVector[order-1];

    divisor = (closed ? knotVector[numPoles+order-1] : knotVector[numPoles]) - root;
    if (fabs (divisor) < KNOT_TOLERANCE_BASIS)
        return MDLERR_BADPARAMETER;

    for (i=0, kP0=knotVector; i < numKnots; i++, kP0++)
        *kP0 = (*kP0 - root) / divisor;

    /* Note...This shouldn't be necessary, but PC cant seem to divide a number by
       itself (due to different precision of double on FPU stack) and get 1.0 */
    if (closed)
        knotVector[numPoles + order - 1] = 1.0;
    else
        for (i=0, kP0 = knotVector + numPoles; i<order; i++)
            *kP0++ = 1.0;

    if (closed)
        {
        for (i=0,
             kP0=knotVector,
             kP1=knotVector+order-1,
             kP2=knotVector+numKnots-2*order+1,
             kP3=knotVector+numKnots-order;
             i < order;
             i++, kP0++, kP1++, kP2++, kP3++)
            {
            *kP0 = *kP2 - 1.0;
            *kP3 = *kP1 + 1.0;
            }
        }

    return SUCCESS;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          bspknot_findSpan                                        |
|                                                                       |
| author        RBB                                     3/90            |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int      bspknot_findSpan
(
int             *spanIndex,
double          *knotVector,
int             numPoles,
int             order,
int             closed,
double          t
)
    {
    int         i, left, right, status=false, minIndex, maxIndex;
    double      minKnot, maxKnot;

   /* Binary search to find the proper interval.  If t is out of the
      range of knot vector,  status remains false and spanIndex is undefined.
      If t equal to the left endpoint of the knot vector, spanIndex gets the
      lowest possible return value.  Otherwise spanIndex gets the index i
      which satisfies the following condition: t[i] < t <= t[i+1]. */

    minIndex = order - 1;
    maxIndex = closed ? numPoles + order - 1 : numPoles;
    minKnot = knotVector[minIndex];
    maxKnot = knotVector[maxIndex];
    if (t < minKnot)
        {
        *spanIndex = minIndex;
        }
    else if (t > maxKnot)
        {
        *spanIndex = maxIndex;
        }
    else
        {
        if (t == minKnot)
            {
            *spanIndex = order-1;
            return SUCCESS;
            }
        else
            {
            left = order;
            right = closed ? numPoles + order - 1 : numPoles;
            while (!status)
                {
                i = (left + right) / 2;
                if (t <= knotVector[i])
                    {
                    if (t > knotVector[i-1])
                        {
                        *spanIndex = i-1;
                        return SUCCESS;
                        }
                    else
                        {
                        right = i-1;
                        }
                    }
                else
                    {
                    if (t <= knotVector[i+1])
                        {
                        *spanIndex = i;
                        return SUCCESS;
                        }
                    else
                        {
                        left = i+1;
                        }
                    }
                }
            }
        }
    return (true);
    }

size_t MSBsplineCurve::FindKnotInterval (double knotValue) const
    {
    int index;
    bspknot_findSpan (&index, knots, params.numPoles, params.order, params.closed, knotValue);
    return (size_t)index;
    }

size_t MSBsplineSurface::FindKnotInterval (double knotValue, int direction) const
    {
    int index;
    if (direction == BSSURF_U)
        bspknot_findSpan (&index, uKnots, uParams.numPoles, uParams.order, uParams.closed, knotValue);
    else
        bspknot_findSpan (&index, vKnots, vParams.numPoles, vParams.order, vParams.closed, knotValue);
    return (size_t)index;
    }
void MSBsplineCurve::ComputeUniformKnotGrevilleAbscissa (bvector<double> &averageKnots, size_t numInterval, int order)
    {
    averageKnots.clear ();
    if (numInterval < 1 || order < 2)
        return;
    int numAverage = order - 1;
    // Construct uniform knots with step 1 and (numAverage) leading and trailing dups ...
    for (int i = 0; i < numAverage; i++)
        averageKnots.push_back (0.0);
    for (size_t i = 1; i < numInterval; i++)
        averageKnots.push_back ((double)i);
    for (int i = 0; i < numAverage; i++)
        averageKnots.push_back ((double)numInterval);
    // save moving averages in place .
    size_t n = averageKnots.size ();
    size_t n1 = 0;
    for (size_t i0 = 0; i0 + numAverage <= n; i0++)
        {
        double s = averageKnots[i0];
        for (size_t i = i0 + 1; i < i0 + numAverage; i++)
            s += averageKnots[i];
        averageKnots[i0] = s;
        n1++;
        }
    averageKnots.resize (n1);
    double f = 1.0 / averageKnots.back ();
    for (double &u : averageKnots)
        u *= f;
    // enforce exact 1.0 at end ...
    averageKnots.back () = 1.0;
    }
/*----------------------------------------------------------------------+
|                                                                       |
| name          bspknot_computeGrevilleAbscissa                         |
|                                                                       |
| author        BFP                                     3/90            |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int      bspknot_computeGrevilleAbscissa
(
double          *nodeValues,
double          *knotVector,
int             numPoles,
int             order,
int             closed,
double          tolerance
)
    {
    int         i, j, degree;
    double      knotSum, minKnot, maxKnot, *kP0, *kP1, *nP;

    degree = order - 1;

    // NEEDS_WORK:
    // There is a possibility that the condition below should be:
    // closed && mdlBspline_knotsShouldBeOpened (knotVector, bspknot_numberKnots (numPoles, order, closed), NULL, NULL, 0, order, closed)
    kP0 = closed ? knotVector + 2 : knotVector + 1;

    for (i=0, nP=nodeValues; i < numPoles; i++, nP++, kP0++)
        {
        knotSum = 0.0;
        for (j=0, kP1=kP0; j < degree; j++, kP1++)
            knotSum += *kP1;
        *nP = knotSum / (double) degree;
        }

    minKnot = knotVector[order-1];
    maxKnot = closed ? knotVector[numPoles+order-1] : knotVector[numPoles];

    if (fabs (nodeValues[0] - minKnot) < tolerance)
        nodeValues[0] = minKnot;

    if (fabs (nodeValues[numPoles - 1] - maxKnot) < tolerance)
        nodeValues[numPoles - 1] = maxKnot;

    return SUCCESS;
    }
/*----------------------------------------------------------------------+
+----------------------------------------------------------------------*/
void MSBsplineCurve::ComputeGrevilleAbscissa(bvector<double> &averageKnots)  const
    {
    averageKnots.clear ();
    size_t numPoles = GetNumPoles ();
    auto knotRange = GetKnotRange ();
    double tolerance = Angle::SmallAngle () * knotRange.Length ();
    // NEEDS_WORK:
    // There is a possibility that the condition below should be:
    // closed && mdlBspline_knotsShouldBeOpened (knotVector, bspknot_numberKnots (numPoles, order, closed), NULL, NULL, 0, order, closed)
    size_t k0 = params.closed ? 2 : 1;
    int order = params.order;
    double degree = order - 1;
    for (size_t i = 0, k = k0; i < numPoles; i++, k++)
        {
        double knotSum = 0.0;
        for (size_t j=k; j < k + degree; j++)
            knotSum += knots[j];
        averageKnots.push_back (knotSum / degree);
        }

    if (fabs (averageKnots[0] - knotRange.GetStart ()) < tolerance)
        averageKnots[0] = knotRange.GetStart();

    if (fabs (averageKnots[numPoles - 1] - knotRange.GetEnd ()) < tolerance)
        averageKnots[numPoles - 1] = knotRange.GetEnd ();
    }

/*----------------------------------------------------------------------+
+----------------------------------------------------------------------*/
void MSBsplineCurve::CompressKnots
(
bvector<double> const &knots,
int order,
bvector<double> &outKnots,
bvector<size_t>&outMultiplicity,
size_t &leftActiveIndex,
size_t &rightActiveIndex
)
    {
    outKnots.clear ();
    outMultiplicity.empty ();
    size_t n = knots.size ();
    leftActiveIndex = rightActiveIndex = 0;
    if (n == 0)
        return;
    outMultiplicity.push_back (1);
    outKnots.push_back (knots[0]);


    for (size_t i = 1; i < n; i++)
        {
        double u = knots[i];
        if (AreSameKnots (outKnots.back (), u))
            outMultiplicity.back ()++;
        else
            {
            outMultiplicity.push_back (1);
            outKnots.push_back (u);
            }
        // force start and end knots to their principal source values ...
        if (i + 1 == order)
            {
            leftActiveIndex = outKnots.size () - 1;
            outKnots.back () = u;
            }
        if (i + (size_t)order == n)
            {
            rightActiveIndex = outKnots.size () - 1;
            outKnots.back () = u;
            }
        }
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz             08/07
+---------------+---------------+---------------+---------------+---------------+------*/
void KnotData::Clear ()
    {
    allKnots.clear ();
    compressedKnots.clear ();
    multiplicities.clear ();
    order = 0;
    closed = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz             08/14
+---------------+---------------+---------------+---------------+---------------+------*/
void KnotData::CollectHighMultiplicityActiveKnots (size_t targetMultiplicity, bool includeEnds, bvector<double> &knots)
    {
    knots.clear ();
    size_t numActive = GetNumActiveKnots ();
    double u;
    size_t n;
    for (size_t i = 0; GetKnotByActiveIndex (i, u, n); i++)
        {
        if (includeEnds && (i == 0 || i + 1 == numActive))
            knots.push_back (u);
        else if (n >= targetMultiplicity)
            knots.push_back (u);
        }
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz             08/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool KnotData::IsWellOrdered () const
    {
    if (order == 0)
        return false;
    if (allKnots.size () < 2 * order)
        return false;
    for (size_t i = 0; i + 1 < allKnots.size (); i++)
        {
        if (allKnots[i] > allKnots[i+1])
            return false;
        }

    if (compressedKnots.size () != multiplicities.size ())
        return false;
    size_t m = compressedKnots.size ();
    size_t sum = 0;
    for (size_t i = 0; i < m; i++)
        sum += multiplicities[i];
    if (sum != allKnots.size ())
        return false;
    if (leftIndex >= rightIndex)
        return false;
    if (leftIndex >= m)
        return false;
    if (rightIndex >= m )
        return false;
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz             08/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool KnotData::LoadCurveKnots (MSBsplineCurveCR curve)
    {
    Clear ();
    curve.GetKnots (allKnots);
    MSBsplineCurve::CompressKnots (allKnots, curve.GetIntOrder (), compressedKnots, multiplicities, leftIndex, rightIndex);
    order = (size_t)curve.GetIntOrder ();
    closed = curve.IsClosed ();
    return IsWellOrdered ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz             08/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool KnotData::LoadSurfaceUKnots (MSBsplineSurfaceCR surface)
    {
    Clear ();
    surface.GetUKnots (allKnots);
    MSBsplineCurve::CompressKnots (allKnots,
            surface.uParams.order,
            compressedKnots, multiplicities, leftIndex, rightIndex);
    order = (size_t)surface.uParams.order;
    closed = surface.uParams.closed != 0;
    return IsWellOrdered ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz             08/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool KnotData::LoadSurfaceVKnots (MSBsplineSurfaceCR surface)
    {
    Clear ();
    surface.GetVKnots (allKnots);
    MSBsplineCurve::CompressKnots (allKnots,
            surface.vParams.order,
            compressedKnots, multiplicities, leftIndex, rightIndex);
    order = (size_t)surface.vParams.order;
    closed = surface.vParams.closed != 0;
    return IsWellOrdered ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz             08/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool KnotData::IsStrictInteriorKnot (double u) const
    {
    return compressedKnots.size () > 0
        && rightIndex < compressedKnots.size ()
        && leftIndex < compressedKnots.size ()
        && leftIndex < rightIndex
        && compressedKnots[leftIndex] < u
        && compressedKnots[rightIndex] > u
        ;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz             08/07
+---------------+---------------+---------------+---------------+---------------+------*/
void KnotData::FindKnotMultiplicity (double knotValue, double &correctedKnotValue, size_t &multiplicityOut) const
    {
    for (size_t i = leftIndex; i <= rightIndex; i++)
        {
        if (MSBsplineCurve::AreSameKnots (compressedKnots[i], knotValue))
            {
            multiplicityOut = multiplicities[i];
            correctedKnotValue = compressedKnots[i];
            return;
            }
        }
    multiplicityOut = 0;
    correctedKnotValue = knotValue;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz             08/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool KnotData::GetKnotByActiveIndex (size_t i, double &a, size_t &multiplicityA) const
    {
    if (leftIndex + i <= rightIndex)
        {
        a = compressedKnots[leftIndex + i];
        multiplicityA = multiplicities[leftIndex + i];
        return true;
        }
    a = 0.0;
    multiplicityA = 0;
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz             08/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool KnotData::GetActiveKnotRange (double &knot0, double &knot1) const
    {
    if (leftIndex <= rightIndex && rightIndex < compressedKnots.size())
        {
        knot0 = compressedKnots[leftIndex];
        knot1 = compressedKnots[rightIndex];
        return true;
        }
    knot0 = 0.0;
    knot1 = 1.0;
    return false;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz             08/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool KnotData::GetKnotIntervalByActiveIndex (size_t i, double &a0, double &a1) const
    {
    if (leftIndex + i + 1 <= rightIndex)
        {
        a0 = compressedKnots[leftIndex + i];
        a1 = compressedKnots[leftIndex + i + 1];
        return true;
        }
    a0 = a1 = 0.0;
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz             08/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool KnotData::GetLongestActiveKnotInterval (size_t &i0, double &a0, double &a1) const
    {
    a0 = a1 = 0.0;
    i0 = 0;
    if (leftIndex + 1 <= rightIndex)
        {
        double a = 0.0;
        for (size_t i = leftIndex + 1; i <= rightIndex; i++)
            {
            double b = compressedKnots[i] - compressedKnots[i-1];
            if (b > a)
                {
                a0 = compressedKnots[i-1];
                a1 = compressedKnots[i];
                a = b;
                i0 = i;
                }
            }
        return a > 0.0; 
        }
    return false;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz             08/07
+---------------+---------------+---------------+---------------+---------------+------*/
size_t KnotData::GetNumActiveKnots() const
    {
    if (rightIndex < leftIndex)
        return 0;
    return rightIndex + 1 - leftIndex;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          bspknot_getKnotMultiplicity                             |
|                                                                       |
| author        RBB                                     3/90            |
|                                                                       |
| Note: bsputil_getKnotMultiplicityExt does not decrement count         |
|       for a closed curve.                                             |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int      bspknot_getKnotMultiplicity
(
double          *distinctKnots,
int             *knotMultiplicity,
int             *numDistinct,
double          *knotVector,
int             numPoles,
int             order,
int             closed,
double          knotTolerance
)
    {
    int         i, numKnots;
    double      minKnot, maxKnot, *kP0, *kP1;

    numKnots = bspknot_numberKnots (numPoles, order, closed);
    minKnot  = knotVector[order-1];
    maxKnot  = knotVector[numKnots-order];

    /* Ignore knots less than minKnot */
    for (kP0=knotVector; *kP0 < minKnot; kP0++)
        ;

    *numDistinct = 1;
    distinctKnots[0] = *kP0;
    knotMultiplicity[0] = 1;

    for (i=(int)((kP0-knotVector)+1), kP1=kP0+1;
         i < numKnots && *kP1 <= maxKnot;
         i++, kP0++, kP1++)
        {
        if (fabs (*kP1 - *kP0) > knotTolerance)
            {
            *(distinctKnots + *numDistinct) = *kP1;
            *(knotMultiplicity + *numDistinct) = 1;
            *numDistinct += 1;
            }
        else
            {
            *(knotMultiplicity + *numDistinct - 1) += 1;
            }
        }

    if (closed)
        {
        if (fabs (distinctKnots[*numDistinct-1] - maxKnot) < knotTolerance)
            *numDistinct -= 1;
        }

    if (!closed)
        distinctKnots[*numDistinct-1] = maxKnot;

    return SUCCESS;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          bspknot_getKnotMultiplicity                             |
|                                                                       |
| author        RBB                                     3/90            |
|                                                                       |
| Note: bsputil_getKnotMultiplicityExt does not decrement count         |
|       for a closed curve.                                             |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int      bspknot_getKnotMultiplicity
(
bvector<double> &distinctKnots,
bvector<size_t> &multiplicities,
double          *knotVector,
int             numPoles,
int             order,
int             closed,
double          knotTolerance
)
    {
    distinctKnots.clear ();
    multiplicities.clear ();

    int         i, numKnots;
    double      minKnot, maxKnot, *kP0, *kP1;

    numKnots = bspknot_numberKnots (numPoles, order, closed);
    minKnot  = knotVector[order-1];
    maxKnot  = knotVector[numKnots-order];

    /* Ignore knots less than minKnot */
    for (kP0=knotVector; *kP0 < minKnot; kP0++)
        ;

    distinctKnots.push_back (*kP0);
    multiplicities.push_back (1);

    for (i=(int)((kP0-knotVector)+1), kP1=kP0+1;
         i < numKnots && *kP1 <= maxKnot;
         i++, kP0++, kP1++)
        {
        if (fabs (*kP1 - *kP0) > knotTolerance)
            {
            distinctKnots.push_back (*kP1);
            multiplicities.push_back (1);
            }
        else
            {
            multiplicities.back () += 1;
            }
        }

    if (closed)
        {
        if (fabs (distinctKnots.back () - maxKnot) < knotTolerance)
            multiplicities.back ()--;
        }
    else  // enforce exact max??
        distinctKnots.back () = maxKnot;

    return SUCCESS;
    }







/*----------------------------------------------------------------------+
|                                                                       |
| name          bspknot_increaseKnotDegree                              |
|                                                                       |
| author        RBB                                     3/90            |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int      bspknot_increaseKnotDegree
(
double          **newKnots,
int             *newNumPolesM1,
int             newDegree,
double          *oldKnots,
int             oldNumPoles,
int             oldDegree,
int             closed,
double          knotTolerance
)
    {
    int         order, i, j, k, multiplicityDelta, numDist, newNumKnots,
                status=SUCCESS, *knotMult;
    double      *distKnot;

    order    = oldDegree + 1;
    distKnot = NULL;
    knotMult = NULL;
    numDist  = bspknot_numberKnots (oldNumPoles, order, closed);
    if (numDist < 1)
        return ERROR;
    if (NULL == (knotMult = (int*)_alloca (numDist * sizeof(int))) ||
        NULL == (distKnot = (double*)_alloca (numDist * sizeof(double))))
        {
        status = MDLERR_INSFMEMORY;
        goto wrapup;
        }
    bspknot_getKnotMultiplicity (distKnot, knotMult, &numDist, oldKnots, oldNumPoles,
                                 order, closed, knotTolerance);

    multiplicityDelta = newDegree - oldDegree;
    newNumKnots = bspknot_numberKnots (oldNumPoles, order, closed)
                + numDist * multiplicityDelta;
    if (NULL == (*newKnots = (double*)msbspline_malloc (newNumKnots * sizeof(double), HEAPSIG_BUTI)))
        {
        status = MDLERR_INSFMEMORY;
        goto wrapup;
        }

    for (i=k=0; i < numDist; i++)
        {
        for (j= 0; j < knotMult[i] + multiplicityDelta; j++)
            {
            (*newKnots)[k] = distKnot[i];
            k++;
            }
        }

    *newNumPolesM1 = newNumKnots - newDegree - 2;

wrapup:
    return status;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          bspknot_insertKnot                                      |
|                                                                       |
| author        BFP                                     3/90            |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int      bspknot_insertKnot
(
double          u,
int             addMult,
int             currentMult,
DPoint3d        *oldPoles,
double          *oldKnots,
double          *oldWeights,
DPoint3d        *newPoles,
double          *newKnots,
double          *newWeights,
int             numPoles,
int             order,
int             closed,
int             rational
)
{
    int         i, j, degree, rght, start, numKnots, shift, end;
    double      *c, *left, *right, *wP0, *wP1, *wts,
                wtsBuffer[2*MAX_BSORDER], coef[MAX_BSORDER];
    DPoint3d    *pP0, *pP1, *pole, poleBuffer[2*MAX_BSORDER];
    bool        bCorrect;

    numKnots = bspknot_numberKnots (numPoles, order, closed);
    bCorrect = mdlBspline_knotsShouldBeOpened (oldKnots, numKnots, oldPoles, rational ? oldWeights : NULL, numPoles, order, closed);

    degree = order - 1;
    pole   = poleBuffer + addMult;
    wts    = wtsBuffer + addMult;

    bsputil_relevantPoles (pole, wts, &rght, &start, oldPoles, oldWeights, u,
                           oldKnots, numPoles, order, closed, rational, bCorrect);

    for (i=addMult, end=degree; i > 0; i--, end--)
        {
        *(pole-i) = *pole;
        right = oldKnots + rght;
        left = right - end;
        for (j=0, pP0=pole, pP1=pP0+1, c=coef; j < end;
             j++, right++, left++, pP0++, pP1++, c++)
            {
            *c = (u - *left)/(*right - *left);
            pP0->x = *c * ((pP1->x) - (pP0->x)) + (pP0->x);
            pP0->y = *c * ((pP1->y) - (pP0->y)) + (pP0->y);
            pP0->z = *c * ((pP1->z) - (pP0->z)) + (pP0->z);
            }
        if (rational)
            {
            *(wts-i) = *wts;
            for (j=0, wP0=wts, wP1=wP0+1, c=coef;
                 j < end; j++, wP0++, wP1++, c++)
                {
                *wP0 = *c * (*wP1 - *wP0) + *wP0;
                }
            }
        }

    /* Copy to newPoles & newWeights */
    if (start < 0)
        {
        /* new poles wrap around old poles at left */
        shift = order + start;
        memcpy (newPoles, poleBuffer - start, (shift + addMult) * sizeof(DPoint3d));
        memcpy (newPoles + shift + addMult, oldPoles + shift, (numPoles - order) * sizeof(DPoint3d));
        memcpy (newPoles + numPoles + addMult + start, poleBuffer, (-start) * sizeof(DPoint3d));
        if (rational)
            {
            memcpy (newWeights, wtsBuffer - start, (shift + addMult) * sizeof(double));
            memcpy (newWeights + shift + addMult, oldWeights + shift, (numPoles - order) * sizeof(double));
            memcpy (newWeights + numPoles + addMult + start, wtsBuffer, (-start) * sizeof(double));
            }
        }
    else if (start > (numPoles - order))
        {
        /* new poles wrap around old poles at right */
        shift = start + order - numPoles;
        memcpy (newPoles, poleBuffer + order + addMult - shift, shift * sizeof(DPoint3d));
        memcpy (newPoles + shift, oldPoles + shift, (numPoles - order) * sizeof(DPoint3d));
        memcpy (newPoles + start, poleBuffer, (numPoles + addMult - start) * sizeof(DPoint3d));
        if (rational)
            {
            memcpy (newWeights, wtsBuffer + order + addMult - shift, shift * sizeof(double));
            memcpy (newWeights + shift, oldWeights + shift, (numPoles - order) * sizeof(double));
            memcpy (newWeights + start, wtsBuffer, (numPoles + addMult - start) * sizeof(double));
            }
        }
    else
        {
        /* new poles fit inside old poles */
        shift = start + order;
        memcpy (newPoles, oldPoles, start * sizeof(DPoint3d));
        memcpy (newPoles + start, poleBuffer, (order + addMult) * sizeof(DPoint3d));
        memcpy (newPoles + shift + addMult, oldPoles + start + order, (numPoles - shift) * sizeof(DPoint3d));
        if (rational)
            {
            memcpy (newWeights, oldWeights, start * sizeof(double));
            memcpy (newWeights + start, wtsBuffer, (order + addMult) * sizeof(double));
            memcpy (newWeights + shift + addMult, oldWeights + start + order, (numPoles - shift) * sizeof(double));
            }
        }

#if defined (INSERT_KNOT_AT_LEFT_OF_CLUSTER)
    /* Including the following line causes the new knot to be inserted at the left
       of a cluster of knots within knot tolerance.  This can cause a non-monotonic
       knot vector.  */

    rght -= currentMult;
#endif
    memcpy (newKnots, oldKnots, rght * sizeof(double));
    for (i=0, right=newKnots + rght; i < addMult; i++, right++)
        *right = u;
    memcpy (newKnots + rght + addMult, oldKnots + rght, (numKnots - rght) * sizeof(double));

#if defined (USE_NORMALIZED_KNOTS)
    bspknot_normalizeKnotVector (newKnots, numPoles + addMult, order, closed);
#endif

    return SUCCESS;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          bspknot_addKnot                                         |
|                                                                       |
| author        BFP                                     3/90            |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int      bspknot_addKnot
(
MSBsplineCurve  *curve,
double          u,
double          knotTolerance,
int             newMult,
int             addToCurrent
)
    {
    int         i, status, degree, newNumPoles, numKnots, currentMult, addMult, allocSize;
    double      tempU, minKnot, maxKnot, *knots, *weights, *kP;
    DPoint3d    *poles;
// Since use of this is within assert, we must guard the allocation of 'e' the same way to avoid unused variable warnings.
#if !defined (NDEBUG)
    bool polygonVisible = curve->GetPolygonDisplay ();
    bool curveVisible   = curve->GetCurveDisplay ();
#endif

    degree = curve->params.order - 1;

    numKnots = bspknot_numberKnots (curve->params.numPoles, curve->params.order,
                                    curve->params.closed);
    minKnot = curve->knots[degree];
    maxKnot = curve->knots[numKnots - curve->params.order];

    if (u < minKnot || u > maxKnot)
        return MDLERR_BADPARAMETER;

   /* determine the the multiplicity of u already in the knot sequence */

    currentMult = 0;
    tempU = u;
    for (i=0, kP=curve->knots; i < numKnots; i++, kP++)
        {
        if (fabs (*kP - tempU) <= knotTolerance)
            {
            tempU = *kP;
            currentMult += 1;
            }
        else
            {
            if (currentMult > 0) break;
            }
        }

    addMult = addToCurrent ? newMult : newMult - currentMult;

    /* done if addMult<=0. newMult = degree & currentMult = degree or order */
    if (addMult <= 0) return SUCCESS;
    if (addMult > curve->params.order)
        return MDLERR_BADPARAMETER;

    /* Malloc memory for the resulting curve structure */
    poles = NULL; knots = weights = NULL;
    newNumPoles = curve->params.numPoles + addMult;
    allocSize = newNumPoles * sizeof (DPoint3d);
    if (NULL == (poles = (DPoint3dP)dlmSystem_mdlMallocWithDescr (allocSize, (void*)pHeapDescr)))
        {
        status = MDLERR_INSFMEMORY;
        goto wrapup;
        }

    if (curve->rational)
        {
        allocSize = newNumPoles * sizeof (double);
        if (NULL == (weights = (double*)dlmSystem_mdlMallocWithDescr (allocSize, (void*)pHeapDescr)))
            {
            status = MDLERR_INSFMEMORY;
            goto wrapup;
            }
        }

    numKnots += addMult;
    allocSize = numKnots * sizeof (double);
    if (NULL == (knots = (double*)dlmSystem_mdlMallocWithDescr (allocSize, (void*)pHeapDescr)))
        {
        status = MDLERR_INSFMEMORY;
        goto wrapup;
        }

    /* Calculate new poles, knots, and weights from old */
    if (SUCCESS !=
        (status = bspknot_insertKnot (tempU, addMult, currentMult, curve->poles,
                                      curve->knots, curve->weights, poles, knots,
                                      weights, curve->params.numPoles, curve->params.order,
                                      curve->params.closed, curve->rational)))
        goto wrapup;

    curve->params.numPoles += addMult;
    dlmSystem_mdlFree (curve->poles);
    curve->poles = poles;

    curve->params.numKnots = bspknot_numberKnots (curve->params.numPoles,
                                                  curve->params.order, curve->params.closed)
                             - 2 * curve->params.order;
    dlmSystem_mdlFreeWithDescr (curve->knots, (void*)pHeapDescr);
    curve->knots = knots;

    if (curve->rational)
        {
        dlmSystem_mdlFreeWithDescr (curve->weights, (void*)pHeapDescr);
        curve->weights = weights;
        }
    poles = NULL; knots = weights = NULL;

wrapup:
    if (poles)      dlmSystem_mdlFreeWithDescr  (poles, (void*)pHeapDescr);
    if (knots)      dlmSystem_mdlFreeWithDescr  (knots, (void*)pHeapDescr);
    if (weights)    dlmSystem_mdlFreeWithDescr  (weights, (void*)pHeapDescr);

    assert (polygonVisible == curve->GetPolygonDisplay ());
    assert (curveVisible == curve->GetCurveDisplay ());

    return status;
    }


#if !defined (FONTIMPORTER)


/*----------------------------------------------------------------------+
|                                                                       |
| name          bspknot_scaleKnotVector                                 |
|                                                                       |
| author        RayBentley                               3/93           |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void bspknot_scaleCurveKnots
(
MSBsplineCurve  *curveP,
double          scale
)
    {
    int         i, numKnots;

    numKnots = bspknot_numberKnots (curveP->params.numPoles,
                                    curveP->params.order,
                                    curveP->params.closed);

    for (i=0; i<numKnots; i++)
        curveP->knots[i] *= scale;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          bspknot_scaleKnotVector                                 |
|                                                                       |
| author        BrianPeters                              3/93           |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void bspknot_scaleKnotVector
(
double          *knots,
BsplineParam    *params,
double          scale
)
    {
    int         numKnots;
    double      *kP, *endP;

    numKnots = bspknot_numberKnots (params->numPoles, params->order, params->closed);

    for (kP = endP = knots, endP += numKnots; kP < endP; kP++)
        *kP *= scale;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          bsputil_validParameter                                  |
|                                                                       |
| author        BrianPeters                             2/92            |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool     bsputil_validParameter
(
double          u,
double const    *knots,
BsplineParam const   *params
)
    {
    int         numKnots;
    double      min, max;

    numKnots = bspknot_numberKnots (params->numPoles, params->order, params->closed);
    min = knots[params->order - 1];
    max = knots[numKnots - params->order];

    return (min <= u && u <= max);
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          bsputil_validParameterSurface                           |
|                                                                       |
| author        BrianPeters                             2/92            |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool     bsputil_validParameterSurface
(
DPoint2d        *uv,
double const    *uKnots,
BsplineParam const    *uParams,
double const *vKnots,
BsplineParam const   *vParams
)
    {
    return (bsputil_validParameter (uv->x, uKnots, uParams) &&
            bsputil_validParameter (uv->y, vKnots, vParams));

    }


/*----------------------------------------------------------------------+
|                                                                       |
| name          bsputil_segmentIntersectCode                            |
|                                                                       |
| author        RBB                                      9/91           |
|                                                                       |
+----------------------------------------------------------------------*/
static int     bsputil_segmentIntersectCode
(
double          intersectDistance,
double          segmentLength,
double          tolerance
)
    {
    if (intersectDistance < - tolerance || intersectDistance > segmentLength + tolerance)
        return INTERSECT_NONE;
    else
        {
        if (intersectDistance < tolerance)
            return INTERSECT_ORIGIN;
        else if (intersectDistance < segmentLength - tolerance)
            return INTERSECT_SPAN;
        else
            return INTERSECT_END;
        }
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          bsputil_xySegmentIntersection                           |
|                                                                       |
| author        RBB                                      6/91           |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void     bsputil_xySegmentIntersection
(
int             *intersect0,            /* <= intersection code for segment 0 */
int             *intersect1,            /* <= intersection code for segment 1 */
double          *distance0,             /* <= distance (0-1.0) along segment 0 */
double          *distance1,             /* <= distance (0-1.0) along segment 1 */
DRange2dP       seg0P,                  /* <= segment 0 */
DRange2dP       seg1P,                  /* <= segment 1 */
double          tolerance               /* => tolerance for endpoint checks */
)
    {
    double      length0, length1, denom;
    DPoint2d    delta0, delta1, delta10;

    *intersect0 = *intersect1 = INTERSECT_NONE;

    delta0.x = seg0P->high.x - seg0P->low.x;
    delta0.y = seg0P->high.y - seg0P->low.y;
    length0  = sqrt (delta0.x * delta0.x + delta0.y * delta0.y);

    if (length0 < 1.0E-12)
        return ;
    delta0.x /= length0;
    delta0.y /= length0;

    delta1.x = seg1P->high.x - seg1P->low.x;
    delta1.y = seg1P->high.y - seg1P->low.y;
    length1 = sqrt (delta1.x * delta1.x + delta1.y * delta1.y);

    if (length1 < 1.0E-12)
        return ;
    delta1.x /= length1;
    delta1.y /= length1;

    denom = delta0.y * delta1.x - delta0.x * delta1.y;

    if (fabs (denom) > PARALLEL_TOLERANCE)
        {
        delta10.x = seg1P->low.x - seg0P->low.x;
        delta10.y = seg1P->low.y - seg0P->low.y;

        *distance1 = (delta0.x * delta10.y - delta0.y * delta10.x)/denom;

        if (INTERSECT_NONE !=
            (*intersect1 = bsputil_segmentIntersectCode (*distance1, length1, tolerance)))
            {
            *distance0 = fabs (delta0.x) > fabs (delta0.y) ?
                 (delta1.x * *distance1 + delta10.x)/delta0.x :
                 (delta1.y * *distance1 + delta10.y)/delta0.y;

            if (INTERSECT_NONE == (*intersect0 = bsputil_segmentIntersectCode (*distance0,
                                                         length0, tolerance)))
                *intersect1 = INTERSECT_NONE;
            }
        }
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          bsputil_segmentIntersection                             |
|                                                                       |
| author        RayBentley                               1/93           |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt    bsputil_segmentIntersection
(
int         *code0P,
int         *code1P,
DPoint3d    *pointP0,
DPoint3d    *pointP1,
double      *param0P,
double      *param1P,
DPoint3d    *segment0P,
DPoint3d    *segment1P,
double      endTol
)
    {
    DPoint3d    aa, bb, cc;
    double      a, b, c, d, f, denominator;

    *code0P = *code1P = INTERSECT_NONE;

    aa.DifferenceOf (segment0P[1], *segment0P);
    bb.DifferenceOf (segment1P[1], *segment1P);
    cc.DifferenceOf (*segment1P, *segment0P);

    a = aa.DotProduct (cc);
    b = aa.DotProduct (aa);
    c = aa.DotProduct (bb);
    d = bb.DotProduct (cc);
    f = bb.DotProduct (bb);

    denominator = (b*f - c*c);
    if (fabs (denominator) > PARALLEL_TOLERANCE)
        {
        *param0P = (a*f - c*d)/denominator;
        *param1P = (c*a - b*d)/denominator;
        pointP0->SumOf (*segment0P, aa, *param0P);
        pointP1->SumOf (*segment1P, bb, *param1P);

        *code0P = bsputil_segmentIntersectCode (*param0P, 1.0, endTol / sqrt (b));
        *code1P = bsputil_segmentIntersectCode (*param1P, 1.0, endTol / sqrt (f));
        return SUCCESS;
        }

    return ERROR;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          bsputil_lineStringIntersect                             |
|                                                                       |
| author        RayBentley                               3/93           |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int bsputil_lineStringIntersect
(
DPoint3d    *pointP,
double      *u0P,
double      *u1P,
DPoint3d    *stringP0,
int         nPoints0,
DPoint3d    *stringP1,
int         nPoints1,
double      tolerance
)
    {
    int         nIntersects = 0, code0, code1, acceptMask0, acceptMask1;
    double      incr0, incr1, param0, param1, distance0, distance1;
    DPoint3d    *p0, *p1, *end0, *end1, point0, point1;

    incr0 = 1.0 / (double) (nPoints0 - 1);
    incr1 = 1.0 / (double) (nPoints1 - 1);

    acceptMask0 = INTERSECT_ORIGIN | INTERSECT_SPAN | INTERSECT_END;
    for (p0 = stringP0, end0 = p0 + nPoints0 - 1, param0 = 0.0;
            p0 < end0;
                p0++, param0 += incr0)
        {
        acceptMask1 = INTERSECT_ORIGIN | INTERSECT_SPAN | INTERSECT_END;
        for (p1 = stringP1, end1 = p1 + nPoints1 - 1, param1 = 0.0;
                p1 < end1;
                    p1++, param1 += incr1)
            {
            bsputil_segmentIntersection (&code0, &code1,
                                         &point0, &point1,
                                         &distance0, &distance1, p0, p1, tolerance);

            if ((code0 & acceptMask0) && (code1 && acceptMask1) &&
                point0.Distance (point1) <= tolerance)
                {
                if (u0P)    u0P[nIntersects]    = param0 + incr0 * distance0;
                if (u1P)    u1P[nIntersects]    = param1 + incr1 * distance1;
                if (pointP) pointP[nIntersects].Interpolate (point0, 0.5, point1);

                nIntersects++;
                }
            acceptMask1 = INTERSECT_SPAN | INTERSECT_END;
            }
        acceptMask0 = INTERSECT_SPAN | INTERSECT_END;
        }
    return nIntersects;
    }

#endif /* !FONTIMPORTER */

/*----------------------------------------------------------------------+
|                                                                       |
|    B-Spline Boundary Utility Routines                                 |
|                                                                       |
+----------------------------------------------------------------------*/
/*----------------------------------------------------------------------+
|                                                                       |
| name          bsputil_freeBoundary                                    |
|                                                                       |
| author        BrianPeters                             8/93            |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void     bsputil_freeBoundary
(
BsurfBoundary   **bounds,
int             numBounds
)
    {
    BsurfBoundary       *bndP, *endP;

    for (bndP = endP = *bounds, endP += numBounds; bndP < endP; bndP++)
        dlmSystem_mdlFree (bndP->points);

    dlmSystem_mdlFree (*bounds);
    *bounds = NULL;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          bsputil_free                                            |
|                                                                       |
| author        EarlinLutz                              08/97           |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void     bsputil_free
(
void            *pMem
)
    {
    dlmSystem_mdlFree (pMem);
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          bsputil_freePointList                                   |
|                                                                       |
| author        BrianPeters                             8/93            |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void     bsputil_freePointList
(
PointList       **list,
int             numLists
)
    {
    PointList       *bndP, *endP;

    for (bndP = endP = *list, endP += numLists; bndP < endP; bndP++)
        dlmSystem_mdlFree (bndP->points);

    dlmSystem_mdlFree (*list);
    *list = NULL;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          bsputil_getKnotMultiplicityExt                          |
|                                                                       |
| author        RBB                                     3/90            |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int      bsputil_getKnotMultiplicityExt
(
double          *distinctKnots,
int             *knotMultiplicity,
int             *numDistinct,
double          *knotVector,
int             numPoles,
int             order,
int             closed,
double          knotTolerance
)
    {
    int         i, numKnots;
    double      minKnot, maxKnot, *kP0, *kP1;

    numKnots = bspknot_numberKnots (numPoles, order, closed);
    minKnot  = closed ? knotVector[0] : knotVector[order-1];
    maxKnot  = closed ? knotVector[numKnots-1] : knotVector[numKnots-order];

    /* Ignore knots less than minKnot */
    for (kP0=knotVector; *kP0 < minKnot; kP0++)
        ;

    *numDistinct = 1;
    distinctKnots[0] = *kP0;
    knotMultiplicity[0] = 1;

    for (i=(int)((kP0-knotVector)+1), kP1=kP0+1;
         i < numKnots && *kP1 <= maxKnot;
         i++, kP0++, kP1++)
        {
        if (fabs (*kP1 - *kP0) > knotTolerance)
            {
            *(distinctKnots + *numDistinct) = *kP1;
            *(knotMultiplicity + *numDistinct) = 1;
            *numDistinct += 1;
            }
        else
            {
            *(knotMultiplicity + *numDistinct - 1) += 1;
            }
        }

    return SUCCESS;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          bsputil_extractParameterRange                           |
|                                                                       |
| author        RayBentley                                1/94          |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void     bsputil_extractParameterRange
(
double          *minP,
double          *maxP,
double          *knotP,
BsplineParam    *paramP
)
    {
    if (knotP && paramP)
        {
        if (minP)
            *minP = knotP[paramP->order - 1];
        if (maxP)
            *maxP = paramP->closed ?
                knotP[paramP->numPoles + paramP->order-1] : knotP[paramP->numPoles];
        }
    else
        {
        if (minP)
            *minP = 0;
        if (maxP)
            *maxP = 1;
        }
    }




/*----------------------------------------------------------------------+
|                                                                       |
| name          bsputil_calculateNumRulesFromCurve                      |
|                                                                       |
| author        RayBentley                              05/98           |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int bsputil_calculateNumRulesFromCurve
(
MSBsplineCurve          *curveP,
int                     nFullCircleIsoparametrics
)
    {
    int                 nPoles = curveP->params.numPoles;
    double              angle;
    DPoint3d            *poleP, *endP, *firstPoleP, *lastPoleP;
    DVec3d              normal, lastNormal;

    if (curveP->rational)
        bsputil_unWeightPoles (curveP->poles, curveP->poles, curveP->weights, nPoles);

    angle = 0.0;

    firstPoleP = lastPoleP = curveP->poles;
    for (poleP = lastPoleP + 1, endP = lastPoleP + nPoles; poleP < endP; poleP++)
       {
       normal.NormalizedDifference (*lastPoleP, *poleP);
       if (lastPoleP > curveP->poles)
            angle += fabs (normal.AngleTo (lastNormal));

        lastPoleP = poleP;
        lastNormal = normal;
        }

    if (curveP->rational)
        bsputil_weightPoles (curveP->poles, curveP->poles, curveP->weights, nPoles);

    return  bsputil_nIsoParamsFromAngle (angle, nFullCircleIsoparametrics);
    }


/*----------------------------------------------------------------------+
|                                                                       |
| name          mdlBspline_getNaturalParameterRange                     |
|                                                                       |
| author        LuHan                                    03/01          |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void     mdlBspline_getNaturalParameterRange
(
double          *pStart,
double          *pEnd,
MSBsplineCurveCP curveP
)
    {
    if (pStart)
        {
        *pStart = curveP->knots[curveP->params.order-1];
        }

    if (pEnd)
        {
        int numKnots = bspknot_numberKnots (curveP->params.numPoles, curveP->params.order, curveP->params.closed);
        *pEnd = curveP->knots[numKnots - curveP->params.order];
        }
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          mdlBspline_normalizeCurveKnots                          |
|                                                                       |
| author        LuHan                                    03/01          |
| Return true if normalization is performed                             |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool     mdlBspline_normalizeCurveKnots
(
double          *pStartNaturalParam,
double          *pEndNaturalParam,
MSBsplineCurve  *curveP
)
    {
    if (!mdlBspline_curveHasNormalizedKnots (curveP))
        {
        int         numKnots = bspknot_numberKnots (curveP->params.numPoles, curveP->params.order, curveP->params.closed);
        double      start, end;

        mdlBspline_getNaturalParameterRange (&start, &end, curveP);
        mdlBspline_toFractionParams (curveP->knots, curveP->knots, numKnots, start, end);
        if (pStartNaturalParam) *pStartNaturalParam = start;
        if (pEndNaturalParam)   *pEndNaturalParam = end;
        return  true;
        }
    else
        {
        if (pStartNaturalParam) *pStartNaturalParam = 0.0;
        if (pEndNaturalParam)   *pEndNaturalParam = 1.0;
        return  false;
        }
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          mdlBspline_unNormalizeCurveKnots                        |
|                                                                       |
| author        LuHan                                    03/01          |
| Return true if non-normalization is preformed                         |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool     mdlBspline_unNormalizeCurveKnots
(
MSBsplineCurve  *curveP,        /* <=> curve with normalized knots */
double          startNaturalParam,
double          endNaturalParam
)
    {
    if (!mdlBspline_curveHasNormalizedKnots (curveP))
        return  false;

    if (fabs(startNaturalParam) > mgds_fc_nearZero  ||  fabs(endNaturalParam-1.0) > mgds_fc_nearZero)
        {
        int     numKnots = bspknot_numberKnots (curveP->params.numPoles, curveP->params.order, curveP->params.closed);
        mdlBspline_toNaturalParams (curveP->knots, curveP->knots, numKnots, startNaturalParam, endNaturalParam);
        return  true;
        }
    return  false;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          mdlBspline_getParameterRange                            |
|                                                                       |
| author        LuHan                                    03/01          |
|                                                                       |
| DEPRECATED -- Use getNaturalParameterRange                            |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void     mdlBspline_getParameterRange
(
double          *pStart,
double          *pEnd,
MSBsplineCurveCP curveP
)
    {
    mdlBspline_getNaturalParameterRange (pStart, pEnd, curveP);
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          mdlBspline_curveHasNormalizedKnots                      |
|                                                                       |
| author        LuHan                                    03/01          |
| This function tells if a curve has parameter range between [0.0, 1.0] |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool     mdlBspline_curveHasNormalizedKnots
(
MSBsplineCurveCP curveP
)
    {
    bool        isNormalized = true;
    int         numKnots = bspknot_numberKnots (curveP->params.numPoles, curveP->params.order, curveP->params.closed);

    if (fabs(curveP->knots[curveP->params.order-1]) > mgds_fc_nearZero  ||
        fabs(curveP->knots[numKnots-curveP->params.order]-1.0) > mgds_fc_nearZero)
        {
        isNormalized = false;
        }

    return isNormalized;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          mdlBspline_toFractionParams                             |
|                                                                       |
| author        LuHan                                    03/01          |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void     mdlBspline_toFractionParams
(
double          *pOutParams,    /* <= fraction params between 0.0 and 1.0 */
double          *pInParams,     /* => natural params */
int             num,            /* => size of array */
double          start,          /* => start of natural range */
double          end             /* => end of natural range */
)
    {
    double      *pIn, *pOut, *pEnd, delta, tol = 1.0e-12;

    delta = end - start;
    for (pIn = pInParams, pOut = pOutParams, pEnd = pInParams+num; pIn < pEnd; pIn++, pOut++)
        {
        *pOut = (*pIn - start) / delta;

        /* Note...The following shouldn't be necessary, but PC cant seem to divide a number by
        itself (due to different precision of double on FPU stack) and get 1.0 */
        if (fabs(*pOut) < tol)
            {
            *pOut = 0.0;
            }
        else if (fabs (*pOut - 1.0) < tol)
            {
            *pOut = 1.0;
            }
        }
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          mdlBspline_toNaturalParams                              |
|                                                                       |
| author        LuHan                                    03/01          |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void     mdlBspline_toNaturalParams
(
double          *pOutParams,    /* <= natural params between start and end */
double          *pInParams,     /* => fraction params between 0.0 and 1.0 */
int             num,            /* => size of array */
double          start,          /* => start of natural range */
double          end             /* => end of natural range */
)
    {
    double      *pIn, *pOut, *pEnd, delta;

    delta = end - start;
    for (pIn = pInParams, pOut = pOutParams, pEnd = pInParams+num; pIn < pEnd; pIn++, pOut++)
        {
        *pOut = *pIn * delta + start;
        }
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          mdlBspline_naturalParameterToFractionParameter          |
|                                                                       |
| author        EarlinLutz                               03/01          |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP double mdlBspline_naturalParameterToFractionParameter
(
MSBsplineCurveCP pCurve,
double          naturalParam
)
    {
    double a, b, delta;
    mdlBspline_getNaturalParameterRange (&a, &b, pCurve);
    delta = b - a;
    if (delta != 0)
        return  (naturalParam - a) / delta;
    else
        return 0.0;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          mdlBspline_fractionParameterToNaturalParameter          |
|                                                                       |
| author        EarlinLutz                               03/01          |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP double mdlBspline_fractionParameterToNaturalParameter
(
MSBsplineCurveCP pCurve,
double          fractionalParam
)
    {
    double a, b;
    mdlBspline_getNaturalParameterRange (&a, &b, pCurve);
    return a + (b - a) * fractionalParam;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          clampInitcp                                             |
|                                                                       |
| author        LuHan                                    03/01          |
|                                                                       |
+----------------------------------------------------------------------*/
static void    clampInitcp
(
DPoint3d Aw,
DPoint3d *Bw
)
    {
    Bw->x = Aw.x;
    Bw->y = Aw.y;
    Bw->z = Aw.z;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          clampInitw                                              |
|                                                                       |
| author        LuHan                                    03/01          |
|                                                                       |
+----------------------------------------------------------------------*/
static void    clampInitw
(
double Aw,
double *Bw
)
    {
    *Bw = Aw;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          clampComcpt                                             |
|                                                                       |
| author        LuHan                                    03/01          |
|                                                                       |
+----------------------------------------------------------------------*/
static void    clampComcpt
(
double      alpha,
DPoint3d    Aw,
double      beta,
DPoint3d    Bw,
DPoint3d    *Cw
)
    {
    Cw->x = alpha*Aw.x+beta*Bw.x;
    Cw->y = alpha*Aw.y+beta*Bw.y;
    Cw->z = alpha*Aw.z+beta*Bw.z;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          clampComcw                                              |
|                                                                       |
| author        LuHan                                    03/01          |
|                                                                       |
+----------------------------------------------------------------------*/
static void    clampComcw
(
double      alpha,
double      Aw,
double      beta,
double      Bw,
double      *Pw
)
    {
    *Pw = alpha*Aw+beta*Bw;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          mdlBspline_clampCurveKnots                              |
|                                                                       |
| author        LuHan                                    03/01          |
| This function will clamp periodic V7 or V8 knots                      |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int      mdlBspline_clampCurveKnots
(
MSBsplineCurve  *curOutP,
MSBsplineCurve  *curInP
)
    {
    int         i, j, k, ll, lk, lr, n, m, span1, mult1, span2, mult2,
                is, ie, p;
    double      *UP, *UQ, *Pw = NULL, *Qw = NULL, alf, oma, left, v[2];
    DPoint3d    *Pp, *Qp;
    MSBsplineCurve  curve;
    span1 = span2 = 0;    // ?? uninitialized possible?
    if (curInP->params.closed)
        mdlBspline_convertPeriodicKnots (&curve, curInP);
    else
        bspcurv_copyCurve (&curve, curInP);

    /* Get local notation */
    n = curve.params.numPoles-1;
    Pp = curve.poles;
    if (curve.rational) Pw = curve.weights;
    p = curve.params.order-1;
    m = curve.params.order + curve.params.numPoles - 1;
    UP = curve.knots;

    v[0] = UP[p]; v[1] = UP[m-p];

    /* Find knot spans and set multiplicities of bounds (as knots) */
    mult1 = 0;
    for ( i=0; i<=m; i++ )
        {
        if ( UP[i] == v[0] )  mult1 += 1;
        if ( UP[i] > v[0] )
            {
            span1 = i-1;
            break;
            }
        }

    mult2 = 0;
    for ( i=m; i>=0; i-- )
        {
        if ( UP[i] == v[1] )  mult2 += 1;
        if ( UP[i] < v[1] )
            {
            span2 = i+mult2;
            break;
            }
        }

    /* Handle case that junk is in first and last knots */
    if ( span1 >= p && mult1 == span1 )
        {
        UP[0] = v[0];
        mult1 += 1;
        }
    if ( span2 == m-1 && mult2 >= p )
        {
        UP[m] = v[1];
        mult2 += 1;
        span2 += 1;
        }

    /* Check if it is already properly clamped (at ends) */
    if ( span1 >= p && mult1 == span1+1 && span2 == m && mult2 >= p+1 )
        {
        bspcurv_freeCurve (&curve);
        return bspcurv_copyCurve (curOutP, curInP);
        }

    /* Get new indices */
    is = span1-p;
    ie = span2-mult2;

    n = ie-is;
    m = span2-span1-mult2+2*p+1;

    /* Get new memory for control points and knots */
    if (curOutP == curInP)
        {
        bspcurv_freeCurve (curInP);
        }

    curOutP->rational = curve.rational;
    curOutP->params.closed = false;
    curOutP->params.order = p + 1;
    curOutP->params.numPoles = n + 1;
    curOutP->params.numKnots = curve.params.numPoles - curve.params.order;

    bspcurv_allocateCurve (curOutP);
    Qp = curOutP->poles;
    if (curOutP->rational) Qw = curOutP->weights;
    UQ = curOutP->knots;

    /* Get initial control points */
    for ( i=is; i<=ie; i++ )
        {
        clampInitcp(Pp[i],&Qp[i-is]);
        if (curOutP->rational)
            clampInitw(Pw[i],&Qw[i-is]);
        }

    /* Insert the left knot */
    ll = span1-p;
    for ( i=1; i<=p-mult1; i++ )
        {
        for ( j=0; j<=p-i-mult1; j++ )
            {
            left = UP[ll+i+j];
            alf  = (v[0]-left)/(UP[span1+j+1]-left);
            oma  = 1.0-alf;
            clampComcpt(alf,Qp[j+1],oma,Qp[j],&Qp[j]);
            if (curOutP->rational)
                clampComcw(alf,Qw[j+1],oma,Qw[j],&Qw[j]);
            }
        }

    /* Insert the right knot */
    lr = span2-p;   lk = n-p+mult2;
    for ( i=1; i<=p-mult2; i++ )
        {
        for ( j=p-i-mult2; j>=0; j-- )
            {
            k    = lk+i+j;
            left = UP[lr+i+j];
            if ( left < v[0] )  left = v[0];

            alf = (v[1]-left)/(UP[span2+j+1]-left);
            oma = 1.0-alf;
            clampComcpt(alf,Qp[k],oma,Qp[k-1],&Qp[k]);
            if (curOutP->rational)
                clampComcw(alf,Qw[k],oma,Qw[k-1],&Qw[k]);
            }
        }

    /* Load the knot vector */
    j = -1;
    for ( i=0; i<=p; i++ )                  UQ[++j] = v[0];
    for ( i=span1+1; i<=span2-mult2; i++ )  UQ[++j] = UP[i];
    for ( i=0; i<=p; i++ )                  UQ[++j] = v[1];

    bspcurv_freeCurve (&curve);
    return  SUCCESS;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          mdlBspline_convertPeriodicKnots                         |
|                                                                       |
| author        LuHan                                    03/01          |
| This function converts V7.0 periodic knots to V8.0 format             |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int      mdlBspline_convertPeriodicKnots
(
MSBsplineCurve  *curveOut,              /* <= expanded periodci knots, but curve.params.closed == false */
MSBsplineCurve  *curveIn                /* => curve.params.closed == true */
)
    {
    int             numKnots, status, i;
    MSBsplineCurve  curve;

    if (!curveIn->params.closed || !curveIn->knots)
        return bspcurv_copyCurve (curveOut, curveIn);

    curve.type = curveIn->type;
    curve.display = curveIn->display;
    curve.rational = curveIn->rational;
    curve.params = curveIn->params;
    curve.params.closed = false;
    numKnots = bspknot_numberKnots (curveIn->params.numPoles, curveIn->params.order, curveIn->params.closed);
    curve.params.numPoles = numKnots - curveIn->params.order;
    if (SUCCESS != (status = bspcurv_allocateCurve (&curve)))
        return  status;

    memcpy (curve.knots, curveIn->knots, numKnots * sizeof (double));
    memcpy (curve.poles, curveIn->poles, curveIn->params.numPoles * sizeof (DPoint3d));
    if (curveIn->rational)
        memcpy (curve.weights, curveIn->weights, curveIn->params.numPoles * sizeof (double));
    for (i = 0; i < curve.params.order - 1; i++)
        {
        curve.poles[i+curveIn->params.numPoles] = curveIn->poles[i];
        if (curveIn->rational)
            curve.weights[i+curveIn->params.numPoles] = curveIn->weights[i];
        }

    if (curveOut == curveIn)
        bspcurv_freeCurve (curveOut);

    status = bspcurv_copyCurve (curveOut, &curve);
    bspcurv_freeCurve (&curve);
    return status;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          mdlBspline_curveHasClampedKnots                         |
|                                                                       |
| author        LuHan                                    03/01          |
| This function tells if a curve has the clamped knots in V8.0 or V7.0  |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool     mdlBspline_curveHasClampedKnots
(
MSBsplineCurveCP curveP
)
    {
    bool        isClamped = true;

    if (!curveP->params.closed)
        {
        int     i, numKnots = bspknot_numberKnots (curveP->params.numPoles, curveP->params.order, curveP->params.closed),
                order = curveP->params.order;
        double  *sK, *eK;

        sK = curveP->knots + order - 1;
        eK = curveP->knots + numKnots - order;
        for (i = 0; i < order-1; i++)
            {
            if (fabs (curveP->knots[i] - *sK) > mgds_fc_nearZero || fabs (curveP->knots[numKnots-order+i] - *eK) > mgds_fc_nearZero)
                {
                isClamped = false;
                break;
                }
            }
        }
    else
        isClamped = false;

    return isClamped;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          mdlBspline_curveHasPeriodicKnots                        |
|                                                                       |
| author        LuHan                                    03/01          |
| This function tells if a curve has the periodic knots in V8.0 or V7.0 |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool     mdlBspline_curveHasPeriodicKnots
(
MSBsplineCurveCP curveP
)
    {
    bool        isPeriodic = true;

    if (curveP->params.closed)
        {
        return  true;
        }
    else if (mdlBspline_curveHasClampedKnots (curveP))
        {
        return  false;
        }
    else
        {
        double      delta0a, delta0b, delta1a, delta1b, *knots = NULL;
        int         i, order = curveP->params.order,
                    numKnots = bspknot_numberKnots (curveP->params.numPoles, curveP->params.order, curveP->params.closed);

        for (i = 1, knots = curveP->knots; i < curveP->params.order; i++)
            {
            delta0a = knots[i] - knots[i-1];
            delta0b = knots[numKnots-2*order+1+i] - knots[numKnots-2*order+i];
            delta1a = knots[order-1+i] - knots[order+i-2];
            delta1b = knots[numKnots-order+i] - knots[numKnots-order+i-1];

            if (fabs (fabs(delta0a) - fabs(delta0b)) > mgds_fc_nearZero ||
                fabs (fabs(delta1a) - fabs(delta1b)) > mgds_fc_nearZero)
                {
                isPeriodic = false;
                break;
                }
            }

        if (isPeriodic)
            {
            /* further check the control points */
            if (curveP->rational)
                bsputil_unWeightPoles (curveP->poles, curveP->poles, curveP->weights, curveP->params.order);

            for (i = 0; i < curveP->params.order - 1; i++)
                {
                if (curveP->poles[i].Distance (*(&curveP->poles[curveP->params.numPoles-order+1+i])) > fc_epsilon)
                    {
                    isPeriodic  = false;
                    break;
                    }
                }

            if (curveP->rational)
                bsputil_weightPoles (curveP->poles, curveP->poles, curveP->weights, curveP->params.order);
            }
        }

    return isPeriodic;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          mdlBspline_curveStoreFullKnots                          |
|                                                                       |
| author        LuHan                                    03/01          |
| This function tells if a curve has the new knot format in V8.0        |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool     mdlBspline_curveStoreFullKnots
(
MSBsplineCurveCP curveP
)
    {
    if (mdlBspline_curveHasNormalizedKnots (curveP))
        {
        if (curveP->params.closed || mdlBspline_curveHasClampedKnots (curveP))
            {
            return  false;
            }
        }
    return  true;
    }

/*---------------------------------------------------------------------------------**//**
* @description Returns true if the associated B-spline curve is "closed" in
*       the nonperiodic V7 sense (cf. bspcurv_closeCurve, bspconv_computeCurveFromArc).
*       Such knots are problematic when the B-spline curve is processed as
*       periodic.  The remedy is usually a call to bspcurv_openCurve.
* @remarks If pPoles is not NULL and this function returns true, then the first/last
*       poles are equal, e.g., the B-spline curve is geometrically closed, but
*       not periodically defined.  If pPoles is NULL and this function returns true,
*       then the first/last poles are *usually* equal, but not always.
* @param pKnots     IN      full knot vector (or at least the first 2*order knots)
* @param numKnots   IN      number of knots given
* @param pPoles     IN      full pole vector (optional)
* @param pWeights   IN      full weight vector if rational (optional)
* @param numPoles   IN      (if pPoles) number of poles
* @param order      IN      order of B-spline curve
* @param closed     IN      closure of B-spline curve
* @see mdlBspline_knotsShouldBeOpenedInU, mdlBspline_knotsShouldBeOpenedInV
* @bsimethod                                                    DavidAssaf      08/01
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     mdlBspline_knotsShouldBeOpened
(
const double*   pKnots,
int             numKnots,
const DPoint3d* pPoles,
const double*   pWeights,
int             numPoles,
int             order,
int             closed
)
    {
    // If closed, look for start knot = 0 with multiplicity order (using small tolerance)
    // and containing order / 2 interior knots, and verify equality of first/last poles.
    // Assume nondecreasing knot vector.
    int start = order - 1;

    if (pKnots && numKnots >= start && closed)
        {
        int i, numInteriorStartKnots, mult;

        // V7 nonperiodic closure only occurs on normalized knots
        if (pKnots[start] != 0.0)
            return false;

        // check knots after start
        for (i = start + 1, mult = 1; i < numKnots; i++, mult++)
            if (fabs (pKnots[i]) > RELATIVE_BSPLINE_EXT_KNOT_TOLERANCE)
                break;

        numInteriorStartKnots = mult - 1;
        if (numInteriorStartKnots != order / 2)
            return false;

        // check knots before start
        for (i = start - 1; i >= 0; i--, mult++)
            if (fabs (pKnots[i]) > RELATIVE_BSPLINE_EXT_KNOT_TOLERANCE)
                break;

        if (mult != order)
            return false;

        // check first/last poles as in bspcurv_closeCurve
        if (pPoles && numPoles > 0)
            {
            // New in v8.9: tighten tolerance for smaller coordinates (for callers passing in master units)
            double tol = bsiDPoint3d_getLargestWeightedCoordinateDifference (pPoles, pWeights, numPoles) * fc_epsilon;
            if (tol > 1.0)
                tol = 1.0;  // v8.5 tolerance

            if (pPoles->IsEqual (pPoles[ numPoles - 1], tol) &&
                (!pWeights || bsputil_isSameWeight (pWeights[0], pWeights[numPoles - 1])))
                return true;
            }
        else
           {
           // if poles aren't given, assume first/last poles are equal
           return true;
           }
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @description Returns true if the associated B-spline curve is "closed" in the
*       nonperiodic V7 sense (cf. bspcurv_closeCurve and bspconv_computeCurveFromArc).
*       Such knots are problematic when the B-spline curve is processed as periodic.
*       The remedy is usually a call to bspcurv_openCurve.
* @remarks If this function returns true, then the first/last poles are equal, e.g.,
*       the curve is geometrically closed, but not periodically defined.
* @see mdlBspline_surfaceShouldBeOpenedInU, mdlBspline_surfaceShouldBeOpenedInV
* @bsimethod                                                    DavidAssaf      08/01
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     mdlBspline_curveShouldBeOpened
(
const MSBsplineCurve    *pCurve
)
    {
    if (pCurve && pCurve->params.closed)
        {
        int nKnots = bspknot_numberKnots (pCurve->params.numPoles, pCurve->params.order, true);

        return mdlBspline_knotsShouldBeOpened (pCurve->knots, nKnots, pCurve->poles, pCurve->rational ? pCurve->weights : NULL,
                                               pCurve->params.numPoles, pCurve->params.order, true);
        }

    return false;
    }


bool MSBsplineCurve::AreKnotsValid (bool clampingRequired) const
    {
    return mdlBspline_areKnotsValid(knots, &params, clampingRequired);
    }

bool MSBsplineSurface::AreUKnotsValid (bool clampingRequired) const
    {
    return mdlBspline_areKnotsValid(uKnots, &uParams, clampingRequired);
    }

bool MSBsplineSurface::AreVKnotsValid (bool clampingRequired) const
    {
    return mdlBspline_areKnotsValid(vKnots, &vParams, clampingRequired);
    }


/*----------------------------------------------------------------------+
|                                                                       |
| Name:     mdlBspline_knotsAreValid                                    |
|                                                                       |
| Author:   DavidAssaf                                  03/04           |
|                                                                       |
| Always looks for nontrivial range and nondecreasing knots.            |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool     mdlBspline_areKnotsValid
(
const   double*         pKnots,             // full knot vector
const   BsplineParam*   pParams,
        bool            bCheckExteriorKnots // clamped if open, periodically extended if closed
)
    {
    double  start, end;
    int     i, nKnot, order;

    if (!pKnots || !pParams)
        return false;

    order = pParams->order;
    nKnot = bspknot_numberKnots (pParams->numPoles, order, pParams->closed);
    start = pKnots[order - 1];
    end = pKnots[nKnot - order];

    // check nontrivial range
    if (start >= end)
        return false;

    // check nondecreasing sequence
    for (i = 0; i < nKnot - 1; i++)
        if (pKnots[i] > pKnots[i + 1])
            return false;

    if (bCheckExteriorKnots)
        {
        if (pParams->closed)
            {
            // exterior knots must be periodically extended
            for (i = 0; i < order - 1; i++)
                {
                if (pKnots[i] != start - (end - pKnots[nKnot - order - (order - 1 - i)]))
                    return false;
                if (pKnots[nKnot - 1 - i] != end + (pKnots[order - 1 + (order - 1 - i)] - start))
                    return false;
                }
            }
        else
            {
            // exterior knots must be clamped
            for (i = 0; i < order - 1; i++)
                {
                if (pKnots[i] != start)
                    return false;
                if (pKnots[nKnot - 1 - i] != end)
                    return false;
                }
            }
        }

    return true;
    }

    
END_BENTLEY_GEOMETRY_NAMESPACE
