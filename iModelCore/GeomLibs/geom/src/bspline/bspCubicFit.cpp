/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include "msbsplinemaster.h"
#include <algorithm>

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @description Remove successive coincident points in an acyclic, ordered list.
* @remarks Parameters, if given, do not enter into the test for coincidence.
* @remarks Input may be same as matching output.
* @param outPts             OUT     preallocated, compressed point array
* @param outParams          OUT     (if inParams) preallocated, compressed parameter array (or NULL)
* @param numOut             OUT     size of output arrays
* @param inPts              IN      array of points to compress
* @param inParams           IN      array of parameters to compress in parallel with inPts (or NULL)
* @param numIn              IN      size of input arrays
* @param tol2               IN      minimum squared distance between different points (or negative to use default)
* @param stripDisconnects   IN      whether to strip DISCONNECT points
* @param returnAtLeastTwo   IN      whether to enforce numOut >= 2 if numOut would be 1 (return ERROR if numOut would be 0)
* @return SUCCESS if arguments are valid, unless otherwise noted above.
* @bsimethod                                                    DavidAssaf      04/06
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP  StatusInt    mdlBspline_removeCoincidentPoints
(
DPoint3d*       outPts,
double*         outParams,
int*            numOut,
const DPoint3d* inPts,
const double*   inParams,
int             numIn,
double          tol2,
bool            stripDisconnects,
bool            returnAtLeastTwo
)
    {
    int i, numCompressed;

    if (!outPts || !numOut || !inPts || numIn < 0)
        return MDLERR_BADARG;

    if (outParams && !inParams)
        outParams = NULL;

    if (stripDisconnects)
        {
        for (i = numCompressed = 0; i < numIn; i++)
            {
            if (inPts[i].x != DISCONNECT)
                {
                outPts[numCompressed] = inPts[i];
                if (outParams)
                    outParams[numCompressed] = inParams[i];
                numCompressed++;
                }
            }

        if (!numCompressed)
            return ERROR;

        // compress the disconnect-stripped arrays
        inPts = outPts;
        if (outParams)
            inParams = outParams;
        numIn = numCompressed;
        }

    if (tol2 < 0.0)
        {
        tol2 = fc_epsilon * bsiDPoint3d_getLargestCoordinateDifference (inPts, numIn);
        tol2 *= tol2;
        }
    if (tol2 < mgds_fc_nearZero * mgds_fc_nearZero)
        tol2 = mgds_fc_nearZero * mgds_fc_nearZero;

    outPts[0] = inPts[0];
    if (outParams)
        outParams[0] = inParams[0];
    for (i = numCompressed = 1; i < numIn; i++)
        {
        if (outPts[numCompressed - 1].DistanceSquared (inPts[i]) >= tol2)
            {
            outPts[numCompressed] = inPts[i];
            if (outParams)
                outParams[numCompressed] = inParams[i];
            numCompressed++;
            }
        }

    if (returnAtLeastTwo && 1 == numCompressed)
        {
        numCompressed = 2;
        outPts[1] = inPts[0];
        if (outParams)
            outParams[1] = inParams[0];
        }

    *numOut = numCompressed;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Lu.Han          03/93
+---------------+---------------+---------------+---------------+---------------+------*/
static int     normalizeKnots
(
double      *innerKnots,
int         numPoints
)
    {
    int     i;
    double  divisor, *pP;

    // shift knots to start at 0.0
    if (*innerKnots != 0.0)
        for (i = 0, pP = innerKnots; i < numPoints; i++, pP++)
            *pP -= *innerKnots;

    divisor = innerKnots[numPoints-1];
    if (divisor < fc_epsilon)
        return ERROR;

    // squash knots into range [0,1]
    divisor = 1.0 / divisor;
    for (i = 0, pP = innerKnots; i < numPoints; i++, pP++)
        *pP *= divisor;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* Adjust fit points/params for interpolation and copy into new buffers. Output buffers should hold one more entry than input buffers in the
* case of closed curves (with first != last point); given this proviso, corresponding input/output buffers may be same. Output params are
* normalized into the range [0,1].
* @bsimethod                                                  David.Assaf     10/2002
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP  int      mdlBspline_c2CubicInterpolatePrepareFitPoints
(
DPoint3d        *outPts,            /* <= new points */
double          *outParams,         /* <= new params (or NULL) */
int             *numPts,            /* <=> old/new number of points/params */
bool            *closed,            /* <=> periodic curve flag, flipped if invalid */
const DPoint3d  *inPts,             /* => points to be interpolated */
const double    *inParams,          /* => parameter at each point (or NULL) */
bool            remvData,           /* => true = remove coincident points */
double          tolerance           /* => max dist betw coincide pts or closed curve */
)
    {
    double  endGap;
    int     newNumPts;

    if (!outPts || !numPts || !closed)
        return ERROR;
    if (!inPts || *numPts < 2)
        return ERROR;

    /* remove coincident data points (and corresponding params) within tolerance */
    if (remvData)
        {
        mdlBspline_removeCoincidentPoints (outPts, outParams, &newNumPts, inPts, inParams, *numPts, tolerance * tolerance, false, true);
        }
    else
        {
        newNumPts = *numPts;
        memmove (outPts, inPts, newNumPts * sizeof (DPoint3d));
        if (outParams && inParams)
            memmove (outParams, inParams, newNumPts * sizeof (double));
        }

    endGap = outPts[0].Distance (outPts[newNumPts - 1]);

    /* if only 2 unique points, then must create open interpolant */
    if (newNumPts == 3 && endGap <= tolerance)
        {
        newNumPts--;
        endGap = outPts[0].Distance (outPts[newNumPts - 1]);
        }
    if (newNumPts <= 2)
        {
        if (endGap <= tolerance)
            return ERROR;

        *closed = false;
        }

    /* if missing, append first point and a new param (best guess = uniform) */
    if (*closed)
        {
        if (endGap > tolerance)
            {
            outPts[newNumPts] = outPts[0];
PUSH_STATIC_ANALYSIS_WARNING(6385)
// Not clear if compiler is unhappy about (a) newNumPts-1 being negative or (b) newNumPts being incremented.  This method apparently thinks caller knows to allocate for increment
            if (outParams && inParams && newNumPts > 1)
                outParams[newNumPts] = outParams[newNumPts - 1] + (outParams[newNumPts - 1] - outParams[0]) / (newNumPts - 1);
POP_STATIC_ANALYSIS_WARNING
            newNumPts++;
            }

        /* can't fit cubic closed curve to 3 unique pts */
        if (newNumPts == 4)
            *closed = false;
        }

    // reserve room for tangents in interpolation curve's pole element, or for (open) pole-based curves's numPoles = newNumPts + 2
    if (newNumPts > MAX_POLES - 2)
        return ERROR;

    if (outParams && inParams && SUCCESS != normalizeKnots(outParams, newNumPts))
        return ERROR;

    *numPts = newNumPts;
    return SUCCESS;
    }
#define BUFFER_SIZE     1024
/*---------------------------------------------------------------------------------**//**
* Note: See Farin's book 3rd edition page 137 for more info. Curves and Surfaces for CAGD--A Practical Guid
* |** *| |*** | | *** | | *** | * unknows = right hand side | .... | | *** | | ***| |* **|
* @bsimethod                                                    Lu.Han          03/95
+---------------+---------------+---------------+---------------+---------------+------*/
static int     solveNearTridiagonalSystem
(
DPoint3d        *outPtsP,   /* <= poles obtained from solving the system */
double          *outWtsP,   /* <= weights if not NULL */
DPoint3d        *dataPtsP,  /* => b left: the 1st and the last points are the same */
double          *dataWtsP,  /* => weights or NULL */
double          *knotsP,    /* => knot vector */
double          *alphaP,    /* => a */
double          *betaP,     /* => d  */
double          *gammaP,    /* => c  */
int             numPoints   /* => count last point which is the same as 1st point */
)
    {
    int         num, i;
    double      auxWts[BUFFER_SIZE], *leftWtsP = NULL, tmp;
    DPoint3d    auxPts[BUFFER_SIZE], *leftPtsP;

    num = numPoints - 1;
    if (num > BUFFER_SIZE)
        {
        if (NULL ==
            (leftPtsP = (DPoint3d*)msbspline_malloc (num * sizeof(DPoint3d), HEAPSIG_BCRV)))
            return ERROR;

        if (dataWtsP && outWtsP && NULL ==
            (leftWtsP = (double*)msbspline_malloc (num * sizeof(double),   HEAPSIG_BCRV)))
            return ERROR;
        }
    else
        {
        leftPtsP = auxPts;
        leftWtsP = auxWts;
        }

    memcpy (leftPtsP, dataPtsP, num*sizeof(DPoint3d));
    if (dataWtsP && outWtsP)
        memcpy (leftWtsP, dataWtsP, num*sizeof(double));

    /* First forward substitution */
    for (i = 1; i < num; i++)
        {
        tmp = -1.0 * alphaP[i] / betaP[i-1];
        betaP[i] = betaP[i] + tmp * gammaP[i-1];
        alphaP[i] = tmp * alphaP[i-1];
        leftPtsP[i].SumOf (leftPtsP[i], leftPtsP[i-1], tmp);
        if (dataWtsP && outWtsP)
            leftWtsP[i] = leftWtsP[i] + tmp * leftWtsP[i-1];
        }

    /* First backward substitution */
    tmp = 1.0 / (betaP[num-1] + alphaP[num-1]);
    gammaP[num-1] *= tmp;
    leftPtsP[num-1].Scale (leftPtsP[num-1], tmp);
    if (dataWtsP && outWtsP)
        leftWtsP[num-1] *= tmp;
    for (i = num - 2; i >= 0; i--)
        {
        leftPtsP[i].SumOf (leftPtsP[i], leftPtsP[i+1], -gammaP[i]);
        leftPtsP[i].SumOf (leftPtsP[i], leftPtsP[num-1], -alphaP[i]);
        leftPtsP[i].Scale (leftPtsP[i], 1.0 / betaP[i]);
        if (dataWtsP && outWtsP)
            leftWtsP[i] = (leftWtsP[i] - gammaP[i] * leftWtsP[i+1] - alphaP[i] *
                           leftWtsP[num-1]) / betaP[i];
        gammaP[i] = -1.0 * (gammaP[i] * gammaP[i+1] + alphaP[i] * gammaP[num-1]) / betaP[i];
        }

    /* Second forward substitution */
    tmp = 1.0 / (1.0 + gammaP[0]);
    outPtsP->Scale (*leftPtsP, tmp);
    if (dataWtsP && outWtsP)
        outWtsP[0] = leftWtsP[0] * tmp;
    for (i = 1; i < num; i++)
        {
        outPtsP[i].SumOf (leftPtsP[i], *outPtsP, -gammaP[i]);
        if (dataWtsP && outWtsP)
            outWtsP[i] = leftWtsP[i] - gammaP[i] * outWtsP[0];
        }

    if (leftPtsP != auxPts)
        msbspline_free (leftPtsP);

    if (outWtsP && dataWtsP && leftWtsP != auxWts)
        msbspline_free (leftWtsP);
    return (SUCCESS);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Lu.Han          03/93
+---------------+---------------+---------------+---------------+---------------+------*/
static int     solveTridiagonalSystem
(
DPoint3d        *outPts,
double          *outWts,
DPoint3d        *dataPts,
double          *dataWts,
double          *triUp,
double          *triLow,
double          *gamma,
int             numIntval
)
    {
    int         i;
    double      *auxWtP = NULL;
    DPoint3d    *auxPtP = NULL;

    if (NULL == (auxPtP = (DPoint3d*)BSIBaseGeom::Malloc ((numIntval+1) * sizeof(DPoint3d))))
        return ERROR;

    if (outWts && NULL == (auxWtP = (double*)BSIBaseGeom::Malloc ((numIntval+1) * sizeof(double))))
        return ERROR;

    /* first B-spline pole is not part of the system */
    outPts[0] = dataPts[0];
    if (outWts)
        outWts[0] = dataWts[0];

    /* Forward substitution */
    auxPtP[0] = dataPts[1];
    if (outWts)
        auxWtP[0] = dataWts[1];
    for (i=1; i<=numIntval; i++)
        {
        auxPtP[i].SumOf (dataPts[i+1], auxPtP[i-1], -1.0*triLow[i]);
        if (outWts)
            auxWtP[i] = dataWts[i+1] - triLow[i] * auxWtP[i-1];
        }

    /* Backward substitution */
    outPts[numIntval+1].Scale (auxPtP[numIntval], 1.0/triUp[numIntval]);
    if (outWts)
        outWts[numIntval+1] = auxWtP[numIntval]/triUp[numIntval];

    /* With natural end cnd, outPts[1] != dataPts[1], so find it here */
    for (i=numIntval-1; i>=0; i--)
        {
        outPts[i+1].SumOf (auxPtP[i], outPts[i+2], -1.0*gamma[i]);
        outPts[i+1].Scale (outPts[i+1], 1.0/triUp[i]);
        if (outWts)
            outWts[i+1]=(auxWtP[i]-gamma[i]*outWts[i+2])/triUp[i];
        }

    /* last B-spline pole is not part of the system */
    outPts[numIntval+2] = dataPts[numIntval+2];
    if (outWts)
        outWts[numIntval+2] = dataWts[numIntval+2];

    if (auxPtP)
        BSIBaseGeom::Free (auxPtP);

    if (outWts && auxWtP)
        BSIBaseGeom::Free (auxWtP);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* Note: As per Farin, dataPts contains the L+1 fit points in dataPts[0], [2]-[L], [L+2]. This routine sets either dataPts[1] or dataPts[L+1],
* depending on the value of beginning.
* @bsimethod                                                    Lu.Han          03/93
+---------------+---------------+---------------+---------------+---------------+------*/
static void    besselEnd
(
DPoint3d        *dataPts,   /* <=> fit pts + slots for end Bez pts */
double          *dataWts,   /* <=> weights on fit pts */
double          *knots,     /* => knot vector */
int             numIntval,  /* => #fit pts - 1 */
bool            beginning   /* => true/false = set dataPts[1]/[L+1] */
)
    {
    double      alpha, beta, alpSqrt, betSqrt, deno = 1.0/3.0;

    if (numIntval == 1)
        {
        /* collinear pts make a linear Bezier curve */
        if (beginning)
            {
            dataPts[1].SumOf (dataPts[3], *dataPts, 2.0);
            dataPts[1].Scale (dataPts[1], deno);
            if (dataWts)
                dataWts[1] = (2.0 * dataWts[0] + dataWts[3]) * deno;
            }
        else    /* end */
            {
            dataPts[2].SumOf (*dataPts, dataPts[3], 2.0);
            dataPts[2].Scale (dataPts[2], deno);
            if (dataWts)
                dataWts[2] = (2.0 * dataWts[3] + dataWts[0]) * deno;
            }
        }
    else if (numIntval == 2)
        {
        if (beginning)
            {
            alpha = (knots[2]-knots[1]) / (knots[2]-knots[0]);
            beta = 1.0 - alpha;
            alpSqrt = -1.0 * alpha * alpha;
            betSqrt = -1.0 * beta * beta;
            dataPts[1].SumOf (dataPts[2], *dataPts, alpSqrt);
            dataPts[1].SumOf (dataPts[1], dataPts[4], betSqrt);
            dataPts[1].Scale (dataPts[1], 1.0/(2.0*alpha));
            dataPts[1].SumOf (dataPts[1], *dataPts, alpha);
            dataPts[1].SumOf (*dataPts, dataPts[1], 2.0);
            dataPts[1].Scale (dataPts[1], deno);
            if (dataWts)
                {
                dataWts[1]=(dataWts[2]+alpSqrt*dataWts[0]+betSqrt*dataWts[4])
                           /(2.0*alpha);
                dataWts[1]=(2.0*(alpha*dataWts[0]+dataWts[1])+dataWts[0])*deno;
                }
            }
        else    /* end */
            {
            alpha = (knots[2]-knots[1]) / (knots[2]-knots[0]);
            beta = 1.0 - alpha;
            alpSqrt = -1.0 * alpha * alpha;
            betSqrt = -1.0 * beta * beta;
            dataPts[3].SumOf (dataPts[2], *dataPts, alpSqrt);
            dataPts[3].SumOf (dataPts[3], dataPts[4], betSqrt);
            dataPts[3].Scale (dataPts[3], 1.0/(2.0*beta));
            dataPts[3].SumOf (dataPts[3], dataPts[4], beta);
            dataPts[3].SumOf (dataPts[4], dataPts[3], 2.0);
            dataPts[3].Scale (dataPts[3], deno);
            if (dataWts)
                {
                dataWts[3]=(dataWts[2]+alpSqrt*dataWts[0]+betSqrt*dataWts[4])
                           /(2.0*beta);
                dataWts[3]=(2.0*(dataWts[3]+beta*dataWts[4])+dataWts[4])*deno;
                }
            }
        }
    else    /* numIntval > 2 */
        {
        if (beginning)
            {
            alpha= (knots[2]-knots[1])/(knots[2]-knots[0]);
            beta = 1.0 - alpha;
            alpSqrt = -1.0 * alpha * alpha;
            betSqrt = -1.0 * beta * beta;
            dataPts[1].SumOf (dataPts[2], *dataPts, alpSqrt);
            dataPts[1].SumOf (dataPts[1], dataPts[3], betSqrt);
            dataPts[1].Scale (dataPts[1], 1.0/(2.0*alpha));
            dataPts[1].SumOf (dataPts[1], *dataPts, alpha);
            dataPts[1].SumOf (*dataPts, dataPts[1], 2.0);
            dataPts[1].Scale (dataPts[1], deno);
            if (dataWts)
                {
                dataWts[1]=(dataWts[2]+alpSqrt*dataWts[0]+betSqrt*dataWts[3])
                           /(2.0*alpha);
                dataWts[1]=(2.0*(alpha*dataWts[0]+dataWts[1])+dataWts[0])*deno;
                }
            }
        else    /* end */
            {
            alpha = (knots[numIntval]-knots[numIntval-1]) /
                    (knots[numIntval]-knots[numIntval-2]);
            beta = 1.0 - alpha;
            alpSqrt = -1.0 * alpha * alpha;
            betSqrt = -1.0 * beta * beta;
            dataPts[numIntval+1].SumOf (dataPts[numIntval], dataPts[numIntval-1], alpSqrt);
            dataPts[numIntval+1].SumOf (dataPts[numIntval+1], dataPts[numIntval+2], betSqrt);
            dataPts[numIntval+1].Scale (dataPts[numIntval+1], 1.0/(2.0*beta));
            dataPts[numIntval+1].SumOf (dataPts[numIntval+1], dataPts[numIntval+2], beta);
            dataPts[numIntval+1].SumOf (dataPts[numIntval+2], dataPts[numIntval+1], 2.0);
            dataPts[numIntval+1].Scale (dataPts[numIntval+1], deno);
            if (dataWts)
                {
                dataWts[numIntval+1] = (dataWts[numIntval] +
                                        alpSqrt*dataWts[numIntval-1] +
                                        betSqrt*dataWts[numIntval+2])
                                        /(2.0*beta);
                dataWts[numIntval+1] = (2.0*(dataWts[numIntval+1] +
                                        beta*dataWts[numIntval+2]) +
                                        dataWts[numIntval+2])*deno;
                }
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* Note: As per Farin, dataPts contains the L+1 fit points in dataPts[0], [2]-[L], [L+2]. This routine sets either dataPts[1] or dataPts[L+1],
* depending on the value of beginning. This is the end condition used by ADSK for fit-splines with a given nonzero tangent.
* @bsimethod                                                  David.Assaf     09/2000
+---------------+---------------+---------------+---------------+---------------+------*/
static void    chordLengthEnd
(
DPoint3d        *dataPts,       /* <=> fit pts + slots for end Bez pts */
double          *dataWts,       /* <=> weights on fit pts (set like dataPts) */
DPoint3d        *endTangent,    /* => nonzero normalized end tangent vector */
int             numIntval,      /* => #fit pts - 1 */
bool            beginning       /* => true/false = set dataPts[1]/[L+1] */
)
    {
    int         iExt, iSet, iInt;   /* indices into dataPts, dataWts */
    DPoint3d    delta;
    double      mag2, scale;

    if (numIntval == 1)
        {
        /* Must special case, because there are no interior fit pts */
        if (beginning)
            {
            iExt = 0;   /* 1st fit pt */
            iSet = 1;   /* 2nd Bezier pt */
            iInt = 3;   /* other fit pt */
            }
        else
            {
            iExt = 3;   /* 2nd fit pt */
            iSet = 2;   /* 3rd Bezier pt */
            iInt = 0;   /* other fit pt */
            }
        }
    else
        {
        if (beginning)
            {
            iExt = 0;           /* 1st fit pt */
            iSet = 1;           /* 2nd Bezier pt in first Bezier segment */
            iInt = 2;           /* 2nd fit pt */
            }
        else
            {
            iExt = numIntval+2; /* last fit pt */
            iSet = numIntval+1; /* 3rd Bezier pt in last Bezier segment */
            iInt = numIntval;   /* penultimate fit pt */
            }
        }

    /* scale beg/end tangent by one third of dist betw 1st/last fit pt pair */
    delta.DifferenceOf (dataPts[iInt], dataPts[iExt]);
    mag2  = delta.x * delta.x + delta.y * delta.y + delta.z * delta.z;
    if (dataWts)
        {
        /* I'm guessing here! */
        double dist = dataWts[iInt] - dataWts[iExt];
        mag2 += dist * dist;
        }
    scale = sqrt (mag2) / 3.0;
    dataPts[iSet].SumOf (dataPts[iExt], *endTangent, scale);
    if (dataWts)
        {
        /* I'm guessing here! */
        dataWts[iSet] = scale;     /* assumes dataWts[iSet] = 1.0 initially */
        }
    }

/*---------------------------------------------------------------------------------**//**
* Note: As per Farin, dataPts contains the L+1 fit points in dataPts[0], [2]-[L], [L+2]. This routine sets either dataPts[1] or dataPts[L+1],
* depending on the value of beginning. This is the end condition used by ADSK for fit-splines with a given zero tangent.
* @bsimethod                                                  David.Assaf     09/2000
+---------------+---------------+---------------+---------------+---------------+------*/
static void    naturalEnd
(
DPoint3d        *dataPts,       /* <=> fit pts + slots for end Bez pts */
double          *dataWts,       /* <=> weights on fit pts (set like dataPts) */
int             numIntval,      /* => #fit pts - 1 */
bool            beginning       /* => true/false = set dataPts[1]/[L+1] */
)
    {
    // we use identity matrix for system matrix of 2-pt interpolant, so we must compute second/penultimate bezier points directly
    if (numIntval == 1)
        {
        double deno = 1.0/3.0;

        // same as bessel case for L=1
        if (beginning)
            {
            dataPts[1].SumOf (dataPts[3], *dataPts, 2.0);
            dataPts[1].Scale (dataPts[1], deno);
            if (dataWts)
                dataWts[1] = (2.0 * dataWts[0] + dataWts[3]) * deno;
            }
        else
            {
            dataPts[2].SumOf (*dataPts, dataPts[3], 2.0);
            dataPts[2].Scale (dataPts[2], deno);
            if (dataWts)
                dataWts[2] = (2.0 * dataWts[3] + dataWts[0]) * deno;
            }
        }
    else
        {
        if (beginning)
            {
            dataPts[1] = dataPts[0];
            if (dataWts)
                {
                /* I'm guessing here! */
                dataWts[1] = dataWts[0];
                }
            }
        else
            {
            dataPts[numIntval+1] = dataPts[numIntval+2];
            if (dataWts)
                {
                /* I'm guessing here! */
                dataWts[numIntval+1] = dataWts[numIntval+2];
                }
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* Note: Check each coordinate against tolerance.
* @bsimethod                                                  David.Assaf     05/2001
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    isNearZero
(
const DPoint3d  *pPoint,
double          absTol
)
    {
    return fabs(pPoint->x) <= absTol && fabs(pPoint->y) <= absTol && fabs(pPoint->z) <= absTol;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Lu.Han          03/93
+---------------+---------------+---------------+---------------+---------------+------*/
static void    initKnotsChordLength
(
double          *knots,          /* <=  knots by chord length */
DPoint3d        *dataPts,        /*  => points */
double          *dataWts,        /*  => weights or NULL */
int             num              /*  => number of dataPts */
)
    {
    int         i;
    double      delta, diffWts;
    DPoint3d    diff;

    knots[0] = 0.0;
    for (i = 1; i < num; i++)
        {
        diff.DifferenceOf (dataPts[i], dataPts[i-1]);
        delta  = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;

        if (dataWts)
            {
            diffWts = dataWts[i] - dataWts[i-1];
            delta += diffWts * diffWts;
            }
        knots[i] = knots[i-1] + sqrt(delta);
        }
    }

/*---------------------------------------------------------------------------------**//**
* Note: This routine sets the end cdx which don't figure into the system matrix for open cubic interpolating splines. This means setting the
* first and last entries in the right hand side of the system.
* @bsimethod                                                  David.Assaf     09/2000
+---------------+---------------+---------------+---------------+---------------+------*/
static void    setEndConditions
(
DPoint3d        *dataPts,           /* <=> entries [1]..[L+1] are rhs of system */
DPoint3d        *endTangents,
double          *dataWts,
double          *knots,             /* => param values corresp to fit pts */
int             numIntval,          /* => L = #fit pts - 1 */
bool            colinearTangents,   /* => T: ensure colinear computed end tangents (zero/NULL endTangent(s), geometrically closed spline) */
bool            chordLenTangents,   /* => T/F: scale endTangent by chordlen/bessel (nonzero endTangent, open spline) */
bool            naturalTangents     /* => T/F: compute natural/bessel endTangent (zero/NULL endTangent, open spline) */
)
    {
    bool    noTangent0, noTangent1; /* T/F: tangent must be computed/scaled */

    if (endTangents)
        {
        noTangent0 = isNearZero (&endTangents[0], fc_nearZero);
        noTangent1 = isNearZero (&endTangents[1], fc_nearZero);
        }
    else
        {
        noTangent0 = noTangent1 = true;
        }

    /* endTangents not given or both zero */
    if (noTangent0 && noTangent1)
        {
        /* compute natural end cdx */
        if (naturalTangents)
            {
            naturalEnd (dataPts, dataWts, numIntval, true);     /* beg */
            naturalEnd (dataPts, dataWts, numIntval, false);    /* end */
            }

        /* compute bessel end cdx */
        else
            {
            besselEnd (dataPts, dataWts, knots, numIntval, true);   /* beg */
            besselEnd (dataPts, dataWts, knots, numIntval, false);  /* end */
            }
        }

    /* at least one endTangent is nonzero */
    else
        {
        /* compute beginning end cdn */
        if (noTangent0)
            {
            if (naturalTangents)
                naturalEnd (dataPts, dataWts, numIntval, true);
            else
                besselEnd (dataPts, dataWts, knots, numIntval, true);
            }

        /* scale given beginning end tangent */
        else
            {
            if (chordLenTangents)
                chordLengthEnd
                    (dataPts, dataWts, &endTangents[0], numIntval, true);
            else
                {
                /* scale by length of bessel tangent */
                besselEnd (dataPts, dataWts, knots, numIntval, true);
                dataPts[1].SumOf (*dataPts, *endTangents, dataPts->Distance (dataPts[1]));
                }
            }

        /* compute terminating end cdn */
        if (noTangent1)
            {
            if (naturalTangents)
                naturalEnd (dataPts, dataWts, numIntval, false);
            else
                besselEnd (dataPts, dataWts, knots, numIntval, false);
            }

        /* scale given terminating end tangent */
        else
            {
            if (chordLenTangents)
                chordLengthEnd
                    (dataPts, dataWts, &endTangents[1], numIntval, false);
            else
                {
                /* scale by length of bessel tangent */
                besselEnd (dataPts, dataWts, knots, numIntval, false);
                dataPts[numIntval+1].SumOf (dataPts[numIntval+2], endTangents[1], dataPts[numIntval+1].Distance (dataPts[numIntval+2]));
                }
            }
        }

    /*
    Pivot computed Bessel tangent(s) at shared fit point to make them colinear.
    Scaling is preserved.
    */
    if (colinearTangents &&
        numIntval > 2 &&
        (noTangent0 || noTangent1) &&
        !naturalTangents &&
        dataPts->Distance (dataPts[numIntval+2]) < fc_epsilon)
        {
        DPoint3d coTangent;

        /* pivot computed end tangent colinear to given beginning tangent */
        if (!noTangent0)
            {
            coTangent.DifferenceOf (*dataPts, dataPts[1]);
            coTangent.Normalize ();
            dataPts[numIntval+1].SumOf (dataPts[numIntval+2], coTangent, dataPts[numIntval+2].Distance (dataPts[numIntval+1]));
            }

        /* pivot computed beginning tangent colinear to given end tangent */
        else if (!noTangent1)
            {
            coTangent.DifferenceOf (*dataPts, dataPts[numIntval+1]);
            coTangent.Normalize ();
            dataPts[1].SumOf (*dataPts, coTangent, dataPts->Distance (dataPts[1]));
            }

        /* pivot both computed tangents parallel to their difference vector */
        else
            {
            coTangent.DifferenceOf (dataPts[1], dataPts[numIntval+1]);
            coTangent.Normalize ();
            dataPts[1].SumOf (*dataPts, coTangent, dataPts->Distance (dataPts[1]));
            dataPts[numIntval+1].SumOf (dataPts[numIntval+2], coTangent, -1.0*dataPts[numIntval+2].Distance (dataPts[numIntval+1]));
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Lu.Han          03/93
+---------------+---------------+---------------+---------------+---------------+------*/
static void    systemLU
(
double      *triUp,
double      *triLow,
double      *alpha,
double      *beta,
double      *gamma,
int         numIntval
)
    {
    int     i;

    triUp[0] = beta[0];
    for (i = 1; i <= numIntval; i++)
        {
        triLow[i] = alpha[i] / triUp[i-1];
        triUp[i] = beta[i] - triLow[i] * gamma[i-1];
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Lu.Han          03/95
+---------------+---------------+---------------+---------------+---------------+------*/
static void    computeAlphaBetaGamma
(
double          *alpha,
double          *beta,
double          *gamma,
double          deltaIp1,
double          deltaI,
double          deltaIm1,
double          deltaIm2
)
    {
    double      deno;

    deno = deltaIm2 + deltaIm1 + deltaI;
    *alpha = deltaI * deltaI / deno;
    *beta = deltaI * (deltaIm2 + deltaIm1) / deno;
    deno = deltaIm1 + deltaI + deltaIp1;
    *beta += deltaIm1 * (deltaI + deltaIp1) / deno;
    *gamma = deltaIm1 * deltaIm1 / deno;

    deno = deltaIm1 + deltaI;
    *alpha /= deno;
    *beta /= deno;
    *gamma /= deno;
    }

/*---------------------------------------------------------------------------------**//**
* DavidAssaf 9/2000
* Note: The naturalTangent0(1) parameters determine the first(last) equations in the system for an open cubic interpolating spline.
* If naturalTangent0(1) = false, then the 1st(last) eqn sets the 2nd(penultimate) B-spline pole equal to the 2nd(3rd) Bezier pole of the
* 1st(last) Bezier segment. This Bezier pole can be set later by filling in the corresponding slot in the right hand side of the system (the
* end cdn won't figure in the system matrix). The fn setEndConditions() does this by computing the Bessel end tangent or by scaling a given
* end tangent by the length of the Bessel tangent or 1/3 the length of the corresponding fit point chord.
* If naturalTangent0(1) = true, then the 1st(last) row of the system matrix is the natural end condition (vanishing 2nd derivative). This end
* condition is used by ADSK open splines when a given end tangent is the zero vector.
* @bsimethod                                                    Lu.Han          03/93
+---------------+---------------+---------------+---------------+---------------+------*/
static void    setUpSystem
(
double      *alpha,
double      *beta,
double      *gamma,
double      *knots,         /* inner knots + 0,1 */
int         numIntval,      /* => L = #fit pts - 1 */
bool        isClosed,
bool        naturalTangent0,/* => T: natural end cond for 1st eqn (open spline) */
bool        naturalTangent1 /* => T: natural end cond for last eqn (open spline) */
)
    {
    int     i, num1;
    double  sum, deltaI, deltaIm1, deltaIm2, deltaIp1;

    num1 = numIntval-1;

    /* line segment */
    if (numIntval==1)
        {
        /* identity matrix */
        alpha[0] = alpha[1] = gamma[0] = gamma[1] = 0.0;
        beta[0] = beta[1]=1.0;
        }

    /* 3 fit pts => open spline */
    else if (numIntval==2)
        {
        /* i = 0 */
        if (naturalTangent0)
            {
            alpha[0] = 0.0;
            deltaI   = knots[1] - knots[0];
            deltaIp1 = knots[2] - knots[1];
            sum      = deltaI + deltaIp1;
            beta[0]  = deltaI + sum;
            gamma[0] = -deltaI;
            beta[0]  /= sum;
            gamma[0] /= sum;
            }
        else
            {
            alpha[0] = 0.0;
            beta[0]  = 1.0;
            gamma[0] = 0.0;
            }

        /* i = 1 */
        deltaIm1 = knots[1] - knots[0];
        deltaI   = knots[2] - knots[1];
        sum      = deltaIm1 + deltaI;
        alpha[1] = deltaI * deltaI / sum;
        beta[1]  = deltaI * deltaIm1 / sum + deltaIm1 * deltaI / sum;
        gamma[1] = deltaIm1 * deltaIm1 / sum;
        alpha[1] /= sum;
        beta[1]  /= sum;
        gamma[1] /= sum;

        /* i = L = 2 */
        if (naturalTangent1)
            {
            deltaIm1 = knots[2] - knots[1];
            deltaIm2 = knots[1] - knots[0];
            sum      = deltaIm2 + deltaIm1;
            alpha[2] = -deltaIm1;
            beta[2]  = deltaIm1 + sum;
            alpha[2] /= sum;
            beta[2]  /= sum;
            gamma[2] = 0.0;
            }
        else
            {
            alpha[2] = 0.0;
            beta[2]  = 1.0;
            gamma[2] = 0.0;
            }
        }

    /* 4 or more fit pts */
    else
        {
        if (isClosed)
            {
            /* The only end cdx for closed splines are in the system matrix */

            /* i = 0 */
            deltaI   = knots[1]      - knots[0];
            deltaIm2 = knots[num1]   - knots[num1-1];
            deltaIm1 = knots[num1+1] - knots[num1];
            deltaIp1 = knots[2]      - knots[1];
            computeAlphaBetaGamma (alpha, beta, gamma, deltaIp1, deltaI,
                                   deltaIm1, deltaIm2);

            /* i = 1 */
            deltaIm2 = deltaIm1;
            deltaIm1 = deltaI;
            deltaI   = knots[2] - knots[1];
            deltaIp1 = knots[3] - knots[2];
            computeAlphaBetaGamma (&alpha[1], &beta[1], &gamma[1], deltaIp1,
                                   deltaI, deltaIm1, deltaIm2);

            /* i = L-1 */
            deltaIp1 = deltaIm1;
            deltaI   = knots[num1+1] - knots[num1];
            deltaIm2 = knots[num1-1] - knots[num1-2];
            deltaIm1 = knots[num1]   - knots[num1-1];
            computeAlphaBetaGamma (&alpha[num1], &beta[num1], &gamma[num1],
                                   deltaIp1, deltaI, deltaIm1, deltaIm2);

            /* There is no L_th equation in the closed case */
            }
        else
            {
            /* i = 0 */
            if (naturalTangent0)
                {
                alpha[0] = 0.0;
                deltaI   = knots[1] - knots[0];
                deltaIp1 = knots[2] - knots[1];
                sum      = deltaI + deltaIp1;
                beta[0]  = deltaI + sum;
                gamma[0] = -deltaI;
                beta[0]  /= sum;
                gamma[0] /= sum;
                }
            else
                {
                alpha[0] = 0.0;
                beta[0]  = 1.0;
                gamma[0] = 0.0;
                }

            /* i = 1 */
            deltaI   = knots[2]-knots[1];
            deltaIm1 = knots[1]-knots[0];
            deltaIm2 = 0.0;
            deltaIp1 = knots[3]-knots[2];
            computeAlphaBetaGamma (&alpha[1], &beta[1], &gamma[1],
                                   deltaIp1, deltaI, deltaIm1, deltaIm2);

            /* i = L-1 = num1 */
            deltaI   = knots[num1+1] - knots[num1];
            deltaIm1 = knots[num1]   - knots[num1-1];
            deltaIm2 = knots[num1-1] - knots[num1-2];
            deltaIp1 = 0.0;
            computeAlphaBetaGamma (&alpha[num1], &beta[num1], &gamma[num1],
                                   deltaIp1, deltaI, deltaIm1, deltaIm2);

            /* i = L */
            if (naturalTangent1)
                {
                deltaIm1         = knots[numIntval]     - knots[numIntval - 1];
                deltaIm2         = knots[numIntval - 1] - knots[numIntval - 2];
                sum              = deltaIm2 + deltaIm1;
                alpha[numIntval] = -deltaIm1;
                beta[numIntval]  = deltaIm1 + sum;
                alpha[numIntval] /= sum;
                beta[numIntval]  /= sum;
                gamma[numIntval] = 0.0;
                }
            else
                {
                alpha[numIntval] = 0.0;
                beta[numIntval]  = 1.0;
                gamma[numIntval] = 0.0;
                }
            }

        /* Now for the main loop: */
        for (i = 2; i < num1; i++)
            {
            deltaI   = knots[i+1] - knots[i];
            deltaIm2 = knots[i-1] - knots[i-2];
            deltaIm1 = knots[i]   - knots[i-1];
            deltaIp1 = knots[i+2] - knots[i+1];
            computeAlphaBetaGamma (&alpha[i], &beta[i], &gamma[i],
                                   deltaIp1, deltaI, deltaIm1, deltaIm2);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* Note: Extra args for: 1. computing chord-length knot vector of CLOSED spline when no parameters given (false = uniform knots) 2. forcing
* computed end tangents of geometrically closed spline to be colinear (false = don't force) 3. scaling given nonzero tangent(s) of open spline
* by chord length (false = scale by bessel tang length) 4. computing natural end tangent(s) of open spline when given zero/null tangent(s)
* (false = bessel tangent)
* @bsimethod                                                  David.Assaf     09/2000
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP  int      bspcurv_c2CubicInterpolatePolesExt
(
DPoint3d        *outPts,            /* <= poles */
double          *outWts,            /* <= weights or NULL */
double          *knots,             /* <= knots or NULL */
double          *inParams,          /* => u parameters or NULL */
DPoint3d        *points,            /* => points to be interpolated (first = last if closed) */
DPoint3d        *endTangents,       /* => end tangents or NULL */
double          *weights,           /* => weights or NULL */
BsplineParam    *bsplineParams,     /* => B-Spline parameters */
int             numPoints,          /* => number of points (incl redundant endpt if closed) */
bool            chordLenKnots,      /* => T/F: chordlen/uniform knots (!inParams, closed spline) */
bool            colinearTangents,   /* => T: ensure colinear computed end tangents (zero/NULL endTangent(s), geometrically closed spline) */
bool            chordLenTangents,   /* => T/F: scale endTangent by chordlen/bessel (nonzero endTangent, open spline) */
bool            naturalTangents     /* => T/F: compute natural/bessel endTangent (zero/NULL endTangent, open spline) */
)
    {
    int             status=SUCCESS, numIntval, allocSize;
    double          *dataWts, *innerKnots, *alpha, *beta, *gamma,
                    *triLow, *triUp;
    DPoint3d        *dataPts = NULL;
    bool            naturalTangent0 = false, naturalTangent1 = false;

    numIntval = numPoints - 1;

    /* Normalize the non-zero end tangents */
    if (endTangents)
        {
        if (!isNearZero (&endTangents[0], fc_nearZero))
            endTangents[0].Normalize ();
        if (!isNearZero (&endTangents[1], fc_nearZero))
            endTangents[1].Normalize ();
        }

    /* Malloc memory for temp variables */
    innerKnots = alpha = beta = gamma = triLow = triUp = dataWts = NULL;
    allocSize = numPoints * sizeof(double);
    if (NULL == (dataPts    = (DPoint3d*)msbspline_malloc (bsplineParams->numPoles*sizeof(DPoint3d),
                                                            HEAPSIG_BCRV)) ||
        NULL == (innerKnots = (double*)msbspline_malloc (allocSize,    HEAPSIG_BCRV)) ||
        NULL == (alpha      = (double*)msbspline_malloc (allocSize,    HEAPSIG_BCRV)) ||
        NULL == (beta       = (double*)msbspline_malloc (allocSize,    HEAPSIG_BCRV)) ||
        NULL == (gamma      = (double*)msbspline_malloc (allocSize,    HEAPSIG_BCRV)) ||
        NULL == (triLow     = (double*)msbspline_malloc (allocSize,    HEAPSIG_BCRV)) ||
        NULL == (triUp      = (double*)msbspline_malloc (allocSize,    HEAPSIG_BCRV)))
        {
        status = ERROR;
        goto wrapup;
        }

    if (outWts && weights)
        if (NULL == (dataWts = (double*)msbspline_malloc (bsplineParams->numPoles *
                                               sizeof(double),  HEAPSIG_BCRV)))
            {
            status = ERROR;
            goto wrapup;
            }

    /* Initialize innerKnots */
    if (inParams)
        {
        memcpy (innerKnots, inParams, numPoints*sizeof(double));
        if (SUCCESS != (status = normalizeKnots (innerKnots, numPoints)))
            goto wrapup;
        if (knots)
            bspknot_computeKnotVector (knots, bsplineParams, innerKnots+1);
        }
    else
        {
        if (chordLenKnots || !bsplineParams->closed)
            {
            /* chord-length parametrization */
            initKnotsChordLength (innerKnots, points, weights, numPoints);
            if (SUCCESS != (status = normalizeKnots (innerKnots, numPoints)))
                goto wrapup;
            if (knots)
                bspknot_computeKnotVector (knots, bsplineParams, innerKnots+1);
            }
        else
            {
            /* uniform parametrization */
            double tmpKnots[MAX_KNOTS];

            bspknot_computeKnotVector (tmpKnots, bsplineParams, NULL);
            memcpy (innerKnots+1, tmpKnots+4, (numPoints-2)*sizeof(double));
            innerKnots[0] = 0.0;
            innerKnots[numPoints-1] = 1.0;
            if (knots)
                bspknot_computeKnotVector (knots, bsplineParams, NULL);
            }
        }

    /* Determine system matrix */
    if (naturalTangents && !bsplineParams->closed)
        {
        /* natural end cdx for open spline are part of system matrix */
        if (!endTangents || isNearZero (&endTangents[0], fc_nearZero))
            naturalTangent0 = true;
        if (!endTangents || isNearZero (&endTangents[1], fc_nearZero))
            naturalTangent1 = true;
        }

    /* OPTIMIZE: could always compute full knot vector to simplify abg compute */
    setUpSystem (alpha, beta, gamma, innerKnots, numIntval,
                    (0 != bsplineParams->closed), naturalTangent0, naturalTangent1);

    /* solve open interpolating cubic spline system */
    if (bsplineParams->closed == false)
        {
        dataPts[0] = points[0];
        dataPts[bsplineParams->numPoles-1] = points[numPoints-1];

        if (outWts && weights)
            {
            dataWts[0] = weights[0];
            dataWts[bsplineParams->numPoles-1] = weights[numPoints-1];
            }

        /* As per Farin, the second and penultimate dataPts are undetermined */
        memcpy (dataPts+2, points+1, (numPoints-2)*sizeof(DPoint3d));
        if (outWts && weights)
            memcpy (dataWts+2, weights+1, (numPoints-2)*sizeof(double));

        /* LU decomposition */
        systemLU (triUp, triLow, alpha, beta, gamma, numIntval);

        /* Set free Bezier points in rhs of system using end conditions */
        setEndConditions (dataPts, endTangents, dataWts, innerKnots, numIntval,
                            colinearTangents, chordLenTangents, naturalTangents);

        if (SUCCESS != (status = solveTridiagonalSystem (outPts, outWts,
            dataPts, dataWts, triUp, triLow, gamma, numIntval)))
            goto wrapup;
        }

    /* solve closed interpolating cubic spline system */
    else
        {
        DPoint3d tPole;

        if (SUCCESS != (status = solveNearTridiagonalSystem (outPts, outWts,
            points, weights, innerKnots, alpha, beta, gamma, numPoints)))
            goto wrapup;

        /*
        Farin's interpolation algorithm (or how we implement it) returns poles
        in the wrong order!  They must be shifted right by one position to line
        up with the knots.
        */
        tPole = outPts[bsplineParams->numPoles - 1];
        memmove (outPts + 1, outPts, (bsplineParams->numPoles - 1) * sizeof (DPoint3d));
        outPts[0] = tPole;
        if (outWts && weights)
            {
            double   tWt = weights[bsplineParams->numPoles - 1];
            memmove (outWts + 1, outWts, (bsplineParams->numPoles - 1) * sizeof (double));
            outWts[0] = tWt;
            }
        }

wrapup:

    if (dataPts)    msbspline_free (dataPts);
    if (dataWts)    msbspline_free (dataWts);
    if (innerKnots) msbspline_free (innerKnots);
    if (alpha)      msbspline_free (alpha);
    if (beta)       msbspline_free (beta);
    if (gamma)      msbspline_free (gamma);
    if (triUp)      msbspline_free (triUp);
    if (triLow)     msbspline_free (triLow);

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* Note: Extra args for: 1. computing chord-length knot vector of CLOSED spline when no parameters given (false = uniform knots) 2. forcing
* computed end tangents of geometrically closed spline to be colinear (false = don't force) 3. scaling given nonzero tangent(s) of open spline
* by chord length (false = scale by bessel tang length) 4. computing natural end tangent(s) of open spline when given zero/null tangent(s)
* (false = bessel tangent)
* @bsimethod                                                  David.Assaf     09/2000
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP  int      bspcurv_c2CubicInterpolateCurveExt
(
MSBsplineCurve  *curve,             /* <= cubic spline curve */
DPoint3d        *inPts,             /* => points to be interpolated */
double          *inParams,          /* => u parameters or NULL */
int             numPts,             /* => number of points */
bool            remvData,           /* => true = remove coincident points */
double          tolerance,          /* => max dist betw coincide pts or closed curve */
DPoint3d        *endTangents,       /* => end tangents or NULL */
bool            closedCurve,        /* => if true, closed Bspline is created */
bool            chordLenKnots,      /* => T/F: chordlen/uniform knots (!inParams, closed spline) */
bool            colinearTangents,   /* => T: ensure colinear computed end tangents (zero/NULL endTangent(s), geometrically closed spline) */
bool            chordLenTangents,   /* => T/F: scale endTangent by chordlen/bessel (nonzero endTangent, open spline) */
bool            naturalTangents     /* => T/F: compute natural/bessel endTangent (zero/NULL endTangent, open spline) */
)
    {
    int         status, numPoints = numPts;
    DPoint3d    *points = NULL;
    double      *params = NULL;

    /* Make a local copy */
    if (NULL == (points = (DPoint3d*)msbspline_malloc ((numPts+1)*sizeof(DPoint3d), HEAPSIG_BCRV)))
        {
        status = ERROR;
        goto wrapup;
        }
    if (inParams)
        {
        if (NULL == (params = (double*)msbspline_malloc ((numPts+1)*sizeof(double), HEAPSIG_BCRV)))
            {
            status = ERROR;
            goto wrapup;
            }
        }

    if (SUCCESS != (status = mdlBspline_c2CubicInterpolatePrepareFitPoints (points, params, &numPoints, &closedCurve, inPts, inParams, remvData, tolerance)))
        goto wrapup;

    curve->params.closed = closedCurve;
    curve->params.order = 4;
    curve->rational = false;
    curve->params.numPoles = curve->params.closed ? numPoints - 1 : numPoints + 2;

    if (chordLenKnots)
        {
        /* no curves are uniformly parametrized */
        curve->params.numKnots = numPoints - 2;
        }
    else
        {
        /* only closed curves without params are made uniform */
        curve->params.numKnots = (curve->params.closed && !inParams) ? 0 : numPoints - 2;
        }

    if (SUCCESS != (status = curve->Allocate ()))
        goto wrapup;

    if (SUCCESS != (status = bspcurv_c2CubicInterpolatePolesExt (curve->GetPoleP (), NULL, curve->knots, params, points, endTangents, NULL,
                                &curve->params, numPoints, chordLenKnots, colinearTangents, chordLenTangents, naturalTangents)))
        curve->ReleaseMem ();

wrapup:
    if (points) msbspline_free (points);
    if (params) msbspline_free (params);
    return status;
    }


END_BENTLEY_GEOMETRY_NAMESPACE
