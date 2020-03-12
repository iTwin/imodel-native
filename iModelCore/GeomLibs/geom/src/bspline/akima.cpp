/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE
#include "msbsplinemaster.h"


#define AKIMA_POINT_COUNT(el) (el->line_string_3d.numverts)
#define AKIMA_SEGMENT_COUNT(el) (el->line_string_3d.numverts - 5)
#define AKIMA_BEZIER_ORDER (4)
#define AKIMA_BEZIER_DEGREE (3)
#define MAX_VERTICES (5000)

#define DGNHANDLERS_STATUS_BadElement ERROR
#define DGNMODEL_STATUS_OutOfMemory ERROR
#define DGNHANDLERS_STATUS_BadArg ERROR
#define AKIMACURVE_Status_SUCCESS SUCCESS
#define AKIMACURVE_STATUS_BadElement ERROR
#define AKIMACURVE_STATUS_NullSolution (-752)

#ifdef AKIMA_ELEMENT
/*---------------------------------------------------------------------------------**//**
* @remarks Assumes either pAkimaPts3d or pAkimaPts2d have been given.
* @remarks Negative input results in default return.
* @bsimethod                                                    DavidAssaf      09/06
+---------------+---------------+---------------+---------------+---------------+------*/
static void    computeAkimaTolerance
(
double*         pAbsTol,
double*         pRelTol,
DPoint3dCP      pAkimaPts3d,
DPoint2dCP      pAkimaPts2d,
int             numAkimaPts
)
    {
    double relTol = pRelTol ? *pRelTol : -1.0;

    if (relTol <= 0.0)
        {
        relTol = mgds_fc_epsilon;

        if (pRelTol)
            *pRelTol = relTol;
        }

    double absTol = pAbsTol ? *pAbsTol : -1.0;

    if (absTol < 0.0)
        {
        double range = pAkimaPts3d
                     ? bsiDPoint3d_getLargestCoordinateDifference (pAkimaPts3d + 2, numAkimaPts - 4)
                     : bsiDPoint2d_getLargestCoordinateDifference (pAkimaPts2d + 2, numAkimaPts - 4);

        absTol = relTol * range;

        if (pAbsTol)
            *pAbsTol = absTol;
        }
    }
#endif
/*---------------------------------------------------------------------------------**//**
* @description Compute the unscaled forward tangent used in the Akima Bezier construction.
*
* @param pTangent           OUT     returned forward tangent
* @param pAkimaUnitChords   IN      array of 4 Akima unit chord vectors
* @param absoluteCoordTol   IN      tolerance for comparing coordinates of the unit chord vectors
* @bsimethod                                                    DavidAssaf      06/06
+---------------+---------------+---------------+---------------+---------------+------*/
static void    akima_computeForwardTangent
(
DPoint3dP       pTangent,
DPoint3dCP      pAkimaUnitChords,
double          absoluteCoordTol
)
    {
    double      fabs1, fabs2;

    fabs1 = fabs (pAkimaUnitChords[3].x - pAkimaUnitChords[2].x);
    fabs2 = fabs (pAkimaUnitChords[1].x - pAkimaUnitChords[0].x);

    if (fabs1 + fabs2 < absoluteCoordTol)
        pTangent->x = 0.5 * (pAkimaUnitChords[1].x + pAkimaUnitChords[2].x);
    else
        pTangent->x = (pAkimaUnitChords[1].x * fabs1 + pAkimaUnitChords[2].x * fabs2) / (fabs1 + fabs2);

    fabs1 = fabs (pAkimaUnitChords[3].y - pAkimaUnitChords[2].y);
    fabs2 = fabs (pAkimaUnitChords[1].y - pAkimaUnitChords[0].y);

    if (fabs1 + fabs2 < absoluteCoordTol)
        pTangent->y = 0.5 * (pAkimaUnitChords[1].y + pAkimaUnitChords[2].y);
    else
        pTangent->y = (pAkimaUnitChords[1].y * fabs1 + pAkimaUnitChords[2].y * fabs2) / (fabs1 + fabs2);

    fabs1 = fabs (pAkimaUnitChords[3].z - pAkimaUnitChords[2].z);
    fabs2 = fabs (pAkimaUnitChords[1].z - pAkimaUnitChords[0].z);

    if (fabs1 + fabs2 < absoluteCoordTol)
        pTangent->z = 0.5 * (pAkimaUnitChords[1].z + pAkimaUnitChords[2].z);
    else
        pTangent->z = (pAkimaUnitChords[1].z * fabs1 + pAkimaUnitChords[2].z * fabs2) / (fabs1 + fabs2);
    }
#ifdef AKIMA_ELEMENT
/*---------------------------------------------------------------------------------**//**
* @description Compute the polyline length, ignoring trivial chords.
* @param pPoints    IN  points in polyline
* @param numPoints  IN  number of points in polyline
* @param absTol     IN  minimum length of a nontrivial polyline chord
* @return Length of polyline.
* @bsimethod                                                    DavidAssaf      08/06
+---------------+---------------+---------------+---------------+---------------+------*/
static double  computeNontrivialPolylineLength2d
(
DPoint2dCP      pPoints,
int             numPoints,
double          absTol
)
    {
    double      totalLength = 0.0, chordLength = 0.0;

    for (int i = 0; i < numPoints - 1; i++)
        {
        if (absTol <= (chordLength = pPoints[ i].Distance (pPoints[ i + 1])))
            totalLength += chordLength;
        }

    return totalLength;
    }


/*---------------------------------------------------------------------------------**//**
* @description Compute the polyline length, ignoring trivial chords.
* @param pPoints    IN  points in polyline
* @param numPoints  IN  number of points in polyline
* @param absTol     IN  minimum length of a nontrivial polyline chord
* @return Length of polyline.
* @bsimethod                                                    DavidAssaf      08/06
+---------------+---------------+---------------+---------------+---------------+------*/
static double  computeNontrivialPolylineLength3d
(
DPoint3dCP      pPoints,
int             numPoints,
double          absTol
)
    {
    double      totalLength = 0.0, chordLength = 0.0;

    for (int i = 0; i < numPoints - 1; i++)
        {
        if (absTol <= (chordLength = pPoints[ i].Distance (pPoints[ i + 1])))
            totalLength += chordLength;
        }

    return totalLength;
    }

/*---------------------------------------------------------------------------------**//**
* @remarks Assumes either pAkimaPts3d or pAkimaPts2d are given, and that other inputs have been validated.
* @bsimethod                                                    DavidAssaf      09/06
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt   extractIndexedBezierFromAkimaPoints
(
DPoint3d*       pBezierPoles,
double*         pBsplineStartParam,
double*         pBsplineEndParam,
DPoint3dCP      pAkimaPts3d,
DPoint2dCP      pAkimaPts2d,
int             numAkimaPts,
int             bezierIndex,
double          absTol,
double          relTol
)
    {
    DSegment3d  akimaChord;
    DPoint3d    akimaPts[6], akimaUnitChords[5], startTangent, endTangent;
    double      chordLength, bezierChordLength;
    StatusInt   status = SUCCESS;
    int         i, iRelevant;

    // test the Bezier-spanning chord
    if (pAkimaPts3d)
        akimaChord.Init (pAkimaPts3d[bezierIndex + 2], pAkimaPts3d[bezierIndex + 3]);
    else
        bsiDSegment3d_initFromDPoint2d (&akimaChord, &pAkimaPts2d[bezierIndex + 2], &pAkimaPts2d[bezierIndex + 3]);

    bsiDSegment3d_evaluateEndPoints (&akimaChord, &akimaPts[2], &akimaPts[3]);
    bezierChordLength = akimaUnitChords[2].NormalizedDifference (akimaPts[3], akimaPts[2]);

    if (pBezierPoles)
        {
        if (bezierChordLength < absTol)
            {
            bsiDSegment3d_interpolateUniformDPoint3dArray (&akimaChord, pBezierPoles, 4, true);
            status = AKIMACURVE_STATUS_NullSolution;
            }

        if (SUCCESS == status)
            {
            // get preceding relevant akima pts/chords by searching for distinct points backward in the array
            for (i = bezierIndex + 1, iRelevant = 1; i >= 0; i--)
                {
                if (pAkimaPts3d)
                    akimaPts[iRelevant] = pAkimaPts3d[i];
                else
                    bsiDPoint3d_initFromDPoint2d (&akimaPts[iRelevant], &pAkimaPts2d[i]);

                chordLength = akimaUnitChords[iRelevant].NormalizedDifference (akimaPts[iRelevant + 1], akimaPts[iRelevant]);
                if (chordLength >= absTol && --iRelevant < 0)
                    break;
                }
            if (iRelevant >= 0)
                status = DGNHANDLERS_STATUS_BadElement;
            }

        if (SUCCESS == status)
            {
            // get succeeding relevant akima pts/chords by searching for distinct points forward in the array
            for (i = bezierIndex + 4, iRelevant = 4; i < numAkimaPts; i++)
                {
                if (pAkimaPts3d)
                    akimaPts[iRelevant] = pAkimaPts3d[i];
                else
                    bsiDPoint3d_initFromDPoint2d (&akimaPts[iRelevant], &pAkimaPts2d[i]);

                chordLength = akimaUnitChords[iRelevant - 1].NormalizedDifference (akimaPts[iRelevant], akimaPts[iRelevant - 1]);
                if (chordLength >= absTol && ++iRelevant > 5)
                    break;
                }
            if (iRelevant <= 5)
                status = DGNHANDLERS_STATUS_BadElement;
            }

        if (SUCCESS == status)
            {
            akima_computeForwardTangent (&startTangent, akimaUnitChords, relTol);
            akima_computeForwardTangent (&endTangent, akimaUnitChords + 1, relTol);

            pBezierPoles[0] = akimaPts[2];
            pBezierPoles[1].SumOf (akimaPts[2], startTangent, bezierChordLength / 3.0);
            pBezierPoles[2].SumOf (akimaPts[3], endTangent, -bezierChordLength / 3.0);
            pBezierPoles[3] = akimaPts[3];
            }
        }

    // at this point, return parameters even if we have a null solution or bad element

    // compute B-spline parameters bracketing this segment (correspond to triple knots of Akima B-spline!)
    if (pBsplineStartParam && bezierIndex == 0)
        *pBsplineStartParam = 0.0;

    if (pBsplineEndParam && bezierIndex == numAkimaPts - 6)
        *pBsplineEndParam = 1.0;

    if ((pBsplineStartParam && bezierIndex > 0) || (pBsplineEndParam && bezierIndex < numAkimaPts - 6))
        {
        double akimaVisiblePolygonLengthBefore, akimaVisiblePolygonLengthAfter, akimaVisiblePolygonLength;

        if (pAkimaPts3d)
            {
            akimaVisiblePolygonLengthBefore = computeNontrivialPolylineLength3d (pAkimaPts3d + 2, bezierIndex + 1, absTol);
            akimaVisiblePolygonLengthAfter  = computeNontrivialPolylineLength3d (pAkimaPts3d + 2 + (bezierIndex + 1), numAkimaPts - 4 - (bezierIndex + 1), absTol);
            }
        else
            {
            akimaVisiblePolygonLengthBefore = computeNontrivialPolylineLength2d (pAkimaPts2d + 2, bezierIndex + 1, absTol);
            akimaVisiblePolygonLengthAfter  = computeNontrivialPolylineLength2d (pAkimaPts2d + 2 + (bezierIndex + 1), numAkimaPts - 4 - (bezierIndex + 1), absTol);
            }

        akimaVisiblePolygonLength = akimaVisiblePolygonLengthBefore + bezierChordLength + akimaVisiblePolygonLengthAfter;

        if (pBsplineStartParam && bezierIndex > 0)
            DoubleOps::SafeDivide (*pBsplineStartParam, akimaVisiblePolygonLengthBefore, akimaVisiblePolygonLength, 0.0);

        if (pBsplineEndParam && bezierIndex < numAkimaPts - 6)
            DoubleOps::SafeDivide (*pBsplineEndParam, akimaVisiblePolygonLengthBefore + bezierChordLength, akimaVisiblePolygonLength, 1.0);
        }

    return status;
    }
#endif
#ifdef AKIMA_ELEMENT
/*---------------------------------------------------------------------------------**//**
* @description Return the four poles of the desired Bezier segment of the Akima curve.
*
* @remarks The i_th 0-based Bezier segment extends between the Akima points with 0-based indices i+2 and i+3.  If this chord has trivial
*       length (as determined by tolerance), then this function returns AKIMACURVE_STATUS_NullSolution and 4 colinear/coincident poles spanning the
*       trivial chord.
* @remarks If the other four Akima chords supporting a nontrivial Bezier segment contain a trivial chord, then that chord is extended
*       to the first neighboring unique point for the purpose of computing the Bezier segment.
* @remarks If there are less than 6 Akima points, or the three points (two hidden) at each end of the Akima point array are not distinct,
*       then DGNHANDLERS_STATUS_BadElement is returned.
* @code
        DPoint3d    poles[4];
        double      startParam, endParam, tol = -1.0;
        int         i = 0;
        do
            {
            if (SUCCESS == (status = bspAkima_extractIndexedBezier (poles, &startParam, &endParam, &edP->el, i++, &tol, -1.0)))
                {
                ...
                }
            } while (SUCCESS == status || AKIMACURVE_STATUS_NullSolution == status);

* @endcode
* @param pBezierPoles       OUT     four poles determining the indexed Bezier segment (or NULL)
* @param pBsplineStartParam OUT     B-spline parameter at start of Bezier segment (or NULL)
* @param pBsplineEndParam   OUT     B-spline parameter at end of Bezier segment (or NULL)
* @param pAkimaCurve        IN      type 11 curve element
* @param bezierIndex        IN      0-based index of Bezier segment to extract (bezierIndex &le; n-6, where n = # Akima pts, incl. 4 hidden)
* @param pAbsTol            IN OUT  min length of a nontrivial Bezier segment.  If NULL, a value is computed using relTol; otherwise, if
*                                   negative, this computed tolerance is returned to facilitate looped execution.
* @param relTol             IN      (if !pAbsTol or *pAbsTol < 0) fractional relative tolerance for computing pAbsTol.  If nonpositive, the
*                                   default value 1.0e-5 is used.
* @return SUCCESS if the Bezier poles are returned,
*       AKIMACURVE_STATUS_NullSolution or DGNHANDLERS_STATUS_BadElement in the situations described above,
*       DGNHANDLERS_STATUS_BadArg if required inputs are NULL or out of range, or
*       MDLERR_BADTYPE if the element is not an Akima curve.
* @bsimethod                                                    DavidAssaf      06/06
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt   bspAkima_extractIndexedBezier
(
DPoint3dP       pBezierPoles,
double*         pBsplineStartParam,
double*         pBsplineEndParam,
MSElementCP     pAkimaCurve,
int             bezierIndex,
double*         pAbsTol,
double          relTol
)
    {
    DPoint3dCP  pVertice3d = NULL;
    DPoint2dCP  pVertice2d = NULL;
    double      absTol;
    StatusInt   status = SUCCESS;
    bool        is3d = pAkimaCurve->hdr.dhdr.props.b.is3d;
    int         numAkimaPts;

    if (!pAkimaCurve)
        return DGNHANDLERS_STATUS_BadArg;

    if (CURVE_ELM != pAkimaCurve->ehdr.type)
        return ERROR; //MDLERR_BADTYPE;

    if (6 > (numAkimaPts = LineStringUtil::GetCount (*pAkimaCurve)))
        return DGNHANDLERS_STATUS_BadElement;

    if (is3d)
        pVertice3d = pAkimaCurve->line_string_3d.vertice;
    else
        pVertice2d = pAkimaCurve->line_string_2d.vertice;

    if (bezierIndex < 0 || bezierIndex > numAkimaPts - 6)
        return DGNHANDLERS_STATUS_BadArg;

    // compute and cache tolerance if necessary
    absTol = pAbsTol ? *pAbsTol : -1.0;
    computeAkimaTolerance (&absTol, &relTol, is3d ? pVertice3d : NULL, is3d ? NULL : pVertice2d, numAkimaPts);
    if (pAbsTol && *pAbsTol < 0.0)
        *pAbsTol = absTol;

    if (!pBezierPoles && !pBsplineStartParam && !pBsplineEndParam)
        return SUCCESS;

    return extractIndexedBezierFromAkimaPoints (pBezierPoles, pBsplineStartParam, pBsplineEndParam, is3d ? pVertice3d : NULL,
                                                is3d ? NULL : pVertice2d, numAkimaPts, bezierIndex, absTol, relTol);
    }

/*---------------------------------------------------------------------------------**//**
@description Evaluate an Akima curve at the given parameter.
@remarks Any parameter out of bounds is moved to an endpoint.
@remarks Both linestring and B-spline parameterizations of the Akima curve span the interval [0,1], but the Bezier segments
    in the former are distributed uniformly, while in the latter, they are distributed by relative cumulative chord length.
    Convert from one to the other with ~mbspAkima_linestringToBsplineParameter and ~mbspAkima_bsplineToLinestringParameter.
@param xyzP         OUT     point on curve (or NULL)
@param dxyzP        OUT     first deriviative (or NULL)
@param ddxyzP       OUT     second deriviative (or NULL)
@param dddxyzP      OUT     third deriviative (or NULL)
@param pAkimaCurve  IN      akima element
@param param        IN      parameter in [0,1] (linestring parameterization).
@return SUCCESS if valid akima element.
@bsimethod                                                      EarlinLutz    08/06
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt   bspAkima_evaluate
(
DPoint3dP       xyzP,
DVec3dP         dxyzP,
DVec3dP         ddxyzP,
DVec3dP         dddxyzP,
MSElementCP     pAkimaCurve,
double          param
)
    {
    int         numAkimaPts = LineStringUtil::GetCount (*pAkimaCurve);

    if (numAkimaPts < 6)
        return DGNHANDLERS_STATUS_BadElement;

    DPoint3d    bezier[AKIMA_BEZIER_ORDER];
    double      abstol = 0.0;
    double      reltol = -1.0;

    if (param < 0.0)
        param = 0.0;

    if (param > 1.0)
        param = 1.0;

    int     numSegment = numAkimaPts - 5;
    double  param1 = param * numSegment;
    int     segment = (int)floor (param1);

    // Parameter 1.0 pullback...
    if (segment > numSegment - 1)
        segment = numSegment - 1;

    double  bezierFraction = param1 - segment;

    if (SUCCESS != bspAkima_extractIndexedBezier (bezier, NULL, NULL, pAkimaCurve, segment, &abstol, reltol))
        return DGNHANDLERS_STATUS_BadElement;

    if (xyzP)
        bsiBezierDPoint3d_evaluateDPoint3d (xyzP, NULL, bezier, AKIMA_BEZIER_ORDER, bezierFraction);

    if (dxyzP || ddxyzP || dddxyzP)
        {
        // ugh.  bspBezier has evaluators for 2 derivatives on DPoint4d arrays.  We need 3.
        // So explicitly build up the differences for the first derivative and take *its* 2 derivatives.
        DPoint4d    derivPoles[AKIMA_BEZIER_ORDER];
        DPoint4d    dX4, ddX4, dddX4;
        int         derivOrder = AKIMA_BEZIER_ORDER - 1;

        for (int i = 0; i < derivOrder; i++)
            {
            derivPoles[i].x = 3.0 * (bezier[i+1].x - bezier[i].x);
            derivPoles[i].y = 3.0 * (bezier[i+1].y - bezier[i].y);
            derivPoles[i].z = 3.0 * (bezier[i+1].z - bezier[i].z);
            derivPoles[i].w = 0.0;
            }

        bsiBezierDPoint4d_evaluateDPoint4dArrayExt (&dX4, &ddX4, &dddX4, derivPoles, derivOrder, &bezierFraction, 1);

        if (dxyzP)
            dxyzP->xyzOf (&dX4);

        if (ddxyzP)
            ddxyzP->xyzOf (&ddX4);

        if (dddxyzP)
            dddxyzP->xyzOf (&ddX4);
        }

    return SUCCESS;
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BSI             03/90
+---------------+---------------+---------------+---------------+---------------+------*/
static int     bspconv_convertAkimaFromPoints
(
int             *type,                  /* <= curve type */
int             *rational,              /* <= rational (weights included) */
BsplineDisplay  *display,               /* <= display parameters */
BsplineParam    *params,                /* <= number of poles etc. */
DPoint3d        **poles,                /* <= pole coordinates */
double          **knots,                /* <= knot vector */
double          **weights,              /* <= weights (if (Rational) */
DPoint3dCP      points,                 /* => points defining Akima curve */
int             numPoints               /* => number of Akima points */
)
    {
PUSH_STATIC_ANALYSIS_WARNING(6385)
    int             status = SUCCESS, i, numVerts, numPoles, numIntrKnots, allocSize;
    double          d[5], polyLength, cumLength, *intrKnots=NULL, tol2;
    DPoint3d        p[6], m[5], t[2], *pnts = NULL, *poleArray = NULL;
    BsplineParam    tParams;

    if (numPoints > MAX_VERTICES || numPoints < 6 || !(pnts = (DPoint3dP) _alloca (numPoints * sizeof (DPoint3d))))
        return DGNHANDLERS_STATUS_BadElement;

    // compute tolerance from visible points as hidden points may be way far out, man
    tol2 = mgds_fc_epsilon * bsiDPoint3d_getLargestCoordinateDifference (points + 2, numPoints - 4);
    tol2 *= tol2;

    // get rid of redundant curve points that may result in bspline that can fail other systems, ACIS for example.
    mdlBspline_removeCoincidentPoints (pnts, NULL, &numVerts, points, NULL, numPoints, tol2, true, true);

    if (numVerts < 6)
        return DGNHANDLERS_STATUS_BadElement;

    numPoles = (numVerts - 5)*3 + 1;
    numIntrKnots = (numVerts - 6)*3;

    if (type)
        *type = BSCURVE_GENERAL;

    if (rational)
        *rational = false;

    if (display)
        {
        display->curveDisplay = true;
        display->polygonDisplay = false;
        }

    tParams.order    = 4;
    tParams.closed   = false;
    tParams.numPoles = numPoles;
    tParams.numKnots = numIntrKnots;

    if (params) *params = tParams;

    /* If neither poles or knots is required, return now */
    if (!poles && !knots)
        return SUCCESS;

    if (poles)
        {
        allocSize = numPoles*sizeof(DPoint3d);

//        if (NULL == (*poles = (DPoint3dP)dlmSystem_mdlMallocWithDescr (allocSize, (MdlDesc*)pHeapDescr)))
        if (NULL == (*poles = (DPoint3dP) BSIBaseGeom::Malloc (allocSize)))
            return DGNMODEL_STATUS_OutOfMemory;

        poleArray = *poles;
        }

    if (knots)
        {
        allocSize = bspknot_numberKnots (numPoles, 4, false) * sizeof (double);

//        if (NULL == (*knots = (double*)dlmSystem_mdlMallocWithDescr (allocSize, (MdlDesc*)pHeapDescr)))
        if (NULL == (*knots = (double*) BSIBaseGeom::Malloc (allocSize)))
            {
            status = DGNMODEL_STATUS_OutOfMemory;
            goto wrapup;
            }

        /* Interior knot used in calculations : notice extra space alloc'ed */
        allocSize = numPoles*sizeof(double);

//        if (NULL == (intrKnots = (double*)dlmSystem_mdlMallocWithDescr (allocSize, (MdlDesc*)pHeapDescr)))
        if (NULL == (intrKnots = (double*) BSIBaseGeom::Malloc (allocSize)))
            {
            status = DGNMODEL_STATUS_OutOfMemory;
            goto wrapup;
            }
        }

    if (weights)
        *weights = NULL;

    for (size_t k = 0; k < 6; k++)
        p[k] = pnts[k];
    //memcpy (p, pnts, 6*sizeof (DPoint3d));

    for (i=0; i < 4; i++)
        d[i] = m[i].NormalizedDifference (p[i+1], p[i]);

    /* needed below to ensure bspline has same parametrization as curve     */
    if (knots)
        {
        intrKnots[0] = d[2];
        intrKnots[3] = d[3];
        }

    akima_computeForwardTangent (t, m, mgds_fc_epsilon);

    /* first b-spline pole */
    if (poles)
        poleArray[0] = p[2];

    for (i=2; true; i++)
        {
        d[4] = m[4].NormalizedDifference (p[5], p[4]);

        if (knots && (i < numVerts-4))
            intrKnots[3*i] = d[4];

        akima_computeForwardTangent (t + 1, m + 1, mgds_fc_epsilon);

        /* Calculate poles of b-spline */
        /*  p[2] = point at start of segment; p[3] = point at end
            t[0] = tangent at start of segment; t[1] = tangent at end  */

        if (poles)
            {
            poleArray[3*(i-1)-2].SumOf (p[2], t[0], d[2] / 3.0);
            poleArray[3*(i-1)-1].SumOf (p[3], t[1], -d[2] / 3.0);

            poleArray[3*(i-1)] = p[3];
            }

        if (i < numVerts - 4)
            {
            //memcpy (p, &p[1], 5*sizeof(DPoint3d));
            for (size_t k = 0; k < 5; k++)
                p[k] = p[k+1];

            //memcpy (d, &d[1], 4*sizeof(double));
            for (size_t k = 0; k < 4; k++)
                d[k] = d[k+1];

            //memcpy (m, &m[1], 4*sizeof(DPoint3d));
            for (size_t k = 0; k < 4; k++)
                m[k] = m[k+1];

            t[0] = t[1];
            p[5]= pnts[i+4];
            }
        else
            {
            break;
            }
        }

    if (knots)
        {
        /* Knot vector to force Bezier curve segments with same parametrization */
        for (i=0, polyLength = 0.0; i < numVerts-5; i++)
            polyLength += intrKnots[3*i];

        if (0.0 != polyLength)
            {
            for (i=0, cumLength = 0.0; i < numVerts-6; i++)
                {
                cumLength += intrKnots[3*i];
                intrKnots[3*i+2] = intrKnots[3*i+1] = intrKnots[3*i] = cumLength/polyLength;
                }
            }

        if (SUCCESS != (status = bspknot_computeKnotVector (*knots, &tParams, intrKnots)))
            return status;
        }

wrapup:
    if (intrKnots)
        BSIBaseGeom::Free (intrKnots);

    if (status)
        {
        if (poles && *poles)
            BSIBaseGeom::Free (*poles);

        if (knots && *knots)
            BSIBaseGeom::Free (*knots);

        if (weights && *weights)
            BSIBaseGeom::Free (*weights);
        }

    return status;
POP_STATIC_ANALYSIS_WARNING
    }


#ifdef AKIMA_ELEMENT
/*---------------------------------------------------------------------------------**//**
* @remarks Assumes numAkimaPts and pSpacePoint have been validated.
* @bsimethod                                                    DavidAssaf      09/06
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt   closestPointFromAkimaPointsXYZ
(
DPoint3dP       pCurvePoint,
double*         pCurveParam,
double*         pDistance,
DPoint3dCP      pAkimaPts3d,
DPoint2dCP      pAkimaPts2d,
int             numAkimaPts,
TransformCP     pElementToWorld,
DPoint3dCP      pSpacePoint

)
    {
    DPoint3d    xyz[AKIMA_BEZIER_ORDER], segmentPoint, curvePoint = *pSpacePoint;
    double      curveDist2 = DBL_MAX, curveParam = 0.0, segmentDist2, segmentParam;
    double      absTol = -1.0, relTol = -1.0;
    int         numSegment = numAkimaPts - 5;
    double      df = 1.0 / numSegment;

    computeAkimaTolerance (&absTol, &relTol, pAkimaPts3d, pAkimaPts2d, numAkimaPts);

    for (int segment = 0; segment < numSegment; segment++)
        {
        if (SUCCESS == extractIndexedBezierFromAkimaPoints (xyz, NULL, NULL, pAkimaPts3d, pAkimaPts2d, numAkimaPts, segment, absTol, relTol))
            {
            if (pElementToWorld)
                pElementToWorld->Multiply (xyz, AKIMA_BEZIER_ORDER);

            if (bsiBezierDPoint3d_closestPoint (&segmentPoint, &segmentParam, &segmentDist2, xyz, NULL, AKIMA_BEZIER_ORDER,
                                                 pSpacePoint->x, pSpacePoint->y, pSpacePoint->z, 0.0, 1.0))
                {
                if (segment == 0 || segmentDist2 < curveDist2)
                    {
                    curvePoint = segmentPoint;
                    curveParam = (segment + segmentParam) * df;
                    curveDist2 = segmentDist2;
                    }
                }
            }
        }

    if (pCurvePoint)
        *pCurvePoint = curvePoint;

    if (pCurveParam)
        *pCurveParam = curveParam;

    if (pDistance)
        *pDistance = sqrt (segmentDist2);

    return SUCCESS;

    }
#endif

#ifdef AKIMA_ELEMENT
/*---------------------------------------------------------------------------------**//**
@description Compute the closest point on the Akima curve to the given point.
@remarks Both linestring and B-spline parameterizations of the Akima curve span the interval [0,1], but the Bezier segments
    in the former are distributed uniformly, while in the latter, they are distributed by relative cumulative chord length.
    Convert from one to the other with ~mbspAkima_linestringToBsplineParameter and ~mbspAkima_bsplineToLinestringParameter.
@param pCurvePoint OUT computed point on curve.
@param pCurveParam OUT computed curve parameter in [0,1] (linestring parameterization)
@param pAkimaCurve IN source element
@param pElementToWorld IN element placement transform
@param pSpacePoint IN point to project
@return SUCCESS if valid akima element
@bsimethod                                                      EarlinLutz      08/06
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt   bspAkima_closestPointXYZ
(
DPoint3dP       pCurvePoint,
double*         pCurveParam,
MSElementCP     pAkimaCurve,
TransformCP     pElementToWorld,
DPoint3dCP      pSpacePoint
)
    {
    DPoint3dCP  pVertice3d = NULL;
    DPoint2dCP  pVertice2d = NULL;
    bool        is3d = pAkimaCurve->hdr.dhdr.props.b.is3d;
    int         numAkimaPts;

    if (!pAkimaCurve || !pSpacePoint)
        return DGNHANDLERS_STATUS_BadArg;

    if (CURVE_ELM != pAkimaCurve->ehdr.type)
        return ERROR; //MDLERR_BADTYPE;

    if (6 > (numAkimaPts = LineStringUtil::GetCount (*pAkimaCurve)))
        return DGNHANDLERS_STATUS_BadElement;

    if (is3d)
        pVertice3d = pAkimaCurve->line_string_3d.vertice;
    else
        pVertice2d = pAkimaCurve->line_string_2d.vertice;

    return closestPointFromAkimaPointsXYZ (pCurvePoint, pCurveParam, NULL, is3d ? pVertice3d : NULL, is3d ? NULL : pVertice2d, numAkimaPts,
                                           pElementToWorld, pSpacePoint);
    }
#endif
#ifdef AKIMA_ELEMENT
/*---------------------------------------------------------------------------------**//**
* @remarks Assumes numAkimaPts, pSpacePoint and pWorldToView have been validated.
* @bsimethod                                                    DavidAssaf      09/06
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt   closestPointFromAkimaPointsXY
(
DPoint3dP       pCurvePoint,
double*         pCurveParam,
double*         pDistanceXY,
DPoint3dCP      pAkimaPts3d,
DPoint2dCP      pAkimaPts2d,
int             numAkimaPts,
TransformCP     pElementToWorld,
DPoint3dCP      pSpacePoint,
DMatrix4dCP     pWorldToView
)
    {
    DPoint4d    xyzView[AKIMA_BEZIER_ORDER], segmentPointView;
    DPoint3d    xyz[AKIMA_BEZIER_ORDER], curvePoint = *pSpacePoint, spacePointView;
    double      curveDist2 = DBL_MAX, curveParam = 0.0, segmentDist2, segmentParam;
    double      absTol = -1.0, relTol = -1.0;
    int         numSegment = numAkimaPts - 5;
    double      df = 1.0 / numSegment;

    computeAkimaTolerance (&absTol, &relTol, pAkimaPts3d, pAkimaPts2d, numAkimaPts);

    bsiDMatrix4d_multiplyAndRenormalizeDPoint3dArray (pWorldToView, &spacePointView, pSpacePoint, 1);

    for (int segment = 0; segment < numSegment; segment++)
        {
        if (SUCCESS == extractIndexedBezierFromAkimaPoints (xyz, NULL, NULL, pAkimaPts3d, pAkimaPts2d, numAkimaPts, segment, absTol, relTol))
            {
            if (pElementToWorld)
                pElementToWorld->Multiply (xyz, AKIMA_BEZIER_ORDER);

            bsiDMatrix4d_multiplyWeightedDPoint3dArray (pWorldToView, xyzView, xyz, NULL, AKIMA_BEZIER_ORDER);

            if (bsiBezierDPoint4d_closestXYPoint (&segmentPointView, &segmentParam, &segmentDist2, xyzView, AKIMA_BEZIER_ORDER,
                                                   spacePointView.x, spacePointView.y, 0.0, 1.0))
                {
                if (segment == 0 || segmentDist2 < curveDist2)
                    {
                    bsiBezierDPoint3d_evaluateDPoint3d (&curvePoint, NULL, xyz, AKIMA_BEZIER_ORDER, segmentParam);
                    curveParam = (segment + segmentParam) * df;
                    curveDist2 = segmentDist2;
                    }
                }
            }
        }

    if (pCurvePoint)
        *pCurvePoint = curvePoint;

    if (pCurveParam)
        *pCurveParam = curveParam;

    if (pDistanceXY)
        *pDistanceXY = sqrt (curveDist2);

    return SUCCESS;
    }
#endif

MSBsplineStatus MSBsplineCurve::InitAkima
(
DPoint3dCP points,
size_t count
)
    {
    return bspconv_convertAkimaFromPoints (
                            &type, &rational, &display, &params,
                            &poles, &knots, &weights,
                            points, (int)count);
    }

MSBsplineStatus MSBsplineCurve::InitAkima
(
bvector<DPoint3d> const &points
)
    {
    return InitAkima (&points[0], points.size ());
    }

END_BENTLEY_GEOMETRY_NAMESPACE
