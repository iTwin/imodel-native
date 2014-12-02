/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnHandlers/CurveHandler.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

#define AKIMA_POINT_COUNT(el) (el->ToLine_String_3d().numverts)
#define AKIMA_SEGMENT_COUNT(el) (el->ToLine_String_3d().numverts - 5)
#define AKIMA_BEZIER_ORDER (4)
#define AKIMA_BEZIER_DEGREE (3)

static int    s_minNumSamples = 2;
static int    s_minSampleLevel = 1;
static int    s_maxSampleLevel = 7;
static double s_defaultAkimaSampleTol = 1.0e-4;

#ifdef UNUSED_FUNCTION

static double s_minInterval = 1.0e-7;

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
        if (absTol <= (chordLength = bsiDPoint2d_distance (pPoints + i, pPoints + i + 1)))
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
        if (absTol <= (chordLength = bsiDPoint3d_distance (pPoints + i, pPoints + i + 1)))
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
        bsiDSegment3d_initFromDPoint3d (&akimaChord, &pAkimaPts3d[bezierIndex + 2], &pAkimaPts3d[bezierIndex + 3]);
    else
        bsiDSegment3d_initFromDPoint2d (&akimaChord, &pAkimaPts2d[bezierIndex + 2], &pAkimaPts2d[bezierIndex + 3]);

    bsiDSegment3d_evaluateEndPoints (&akimaChord, &akimaPts[2], &akimaPts[3]);
    bezierChordLength = bsiDPoint3d_computeNormal (&akimaUnitChords[2], &akimaPts[3], &akimaPts[2]);

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

                chordLength = bsiDPoint3d_computeNormal (&akimaUnitChords[iRelevant], &akimaPts[iRelevant + 1], &akimaPts[iRelevant]);
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

                chordLength = bsiDPoint3d_computeNormal (&akimaUnitChords[iRelevant - 1], &akimaPts[iRelevant], &akimaPts[iRelevant - 1]);
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
            bsiDPoint3d_addScaledDPoint3d (&pBezierPoles[1], &akimaPts[2], &startTangent, bezierChordLength / 3.0);
            bsiDPoint3d_addScaledDPoint3d (&pBezierPoles[2], &akimaPts[3], &endTangent,  -bezierChordLength / 3.0);
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
            bsiTrig_safeDivide (pBsplineStartParam, akimaVisiblePolygonLengthBefore, akimaVisiblePolygonLength, 0.0);

        if (pBsplineEndParam && bezierIndex < numAkimaPts - 6)
            bsiTrig_safeDivide (pBsplineEndParam, akimaVisiblePolygonLengthBefore + bezierChordLength, akimaVisiblePolygonLength, 1.0);
        }

    return status;
    }

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
DgnElementCP     pAkimaCurve,
int             bezierIndex,
double*         pAbsTol,
double          relTol
)
    {
    DPoint3dCP  pVertice3d = NULL;
    DPoint2dCP  pVertice2d = NULL;
    double      absTol;
    bool        is3d = pAkimaCurve->Is3d();
    int         numAkimaPts;

    if (!pAkimaCurve)
        return DGNHANDLERS_STATUS_BadArg;

    if (CURVE_ELM != pAkimaCurve->GetLegacyType())
        return ERROR; //MDLERR_BADTYPE;

    if (6 > (numAkimaPts = LineStringUtil::GetCount (*pAkimaCurve)))
        return DGNHANDLERS_STATUS_BadElement;

    if (is3d)
        pVertice3d = pAkimaCurve->ToLine_String_3d().vertice;
    else
        pVertice2d = pAkimaCurve->ToLine_String_2d().vertice;

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
DgnElementCP     pAkimaCurve,
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
                bsiTransform_multiplyDPoint3dArrayInPlace (pElementToWorld, xyz, AKIMA_BEZIER_ORDER);

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
DgnElementCP     pAkimaCurve,
TransformCP     pElementToWorld,
DPoint3dCP      pSpacePoint
)
    {
    DPoint3dCP  pVertice3d = NULL;
    DPoint2dCP  pVertice2d = NULL;
    bool        is3d = pAkimaCurve->Is3d();
    int         numAkimaPts;

    if (!pAkimaCurve || !pSpacePoint)
        return DGNHANDLERS_STATUS_BadArg;

    if (CURVE_ELM != pAkimaCurve->GetLegacyType())
        return ERROR; //MDLERR_BADTYPE;

    if (6 > (numAkimaPts = LineStringUtil::GetCount (*pAkimaCurve)))
        return DGNHANDLERS_STATUS_BadElement;

    if (is3d)
        pVertice3d = pAkimaCurve->ToLine_String_3d().vertice;
    else
        pVertice2d = pAkimaCurve->ToLine_String_2d().vertice;

    return closestPointFromAkimaPointsXYZ (pCurvePoint, pCurveParam, NULL, is3d ? pVertice3d : NULL, is3d ? NULL : pVertice2d, numAkimaPts,
                                           pElementToWorld, pSpacePoint);
    }

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
                bsiTransform_multiplyDPoint3dArrayInPlace (pElementToWorld, xyz, AKIMA_BEZIER_ORDER);

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

/*---------------------------------------------------------------------------------**//**
@description Compute the closest point on the Akima curve to the given point.
@remarks Both linestring and B-spline parameterizations of the Akima curve span the interval [0,1], but the Bezier segments
    in the former are distributed uniformly, while in the latter, they are distributed by relative cumulative chord length.
    Convert from one to the other with ~mbspAkima_linestringToBsplineParameter and ~mbspAkima_bsplineToLinestringParameter.
@param pCurvePoint OUT computed point on curve.
@param pCurveParam OUT computed curve parameter in [0,1] (linestring parameterization)
@param pXYDistance OUT distance to curve, in viewed coordinates
@param pAkimaCurve IN source element
@param pElementToWorld IN element placement transform
@param pSpacePoint IN point to project
@param pWorldToView IN transformation to xy space
@return SUCCESS if valid akima element
@bsimethod                                                      EarlinLutz      08/06
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt   bspAkima_closestPointXY
(
DPoint3dP       pCurvePoint,
double*         pCurveParam,
double*         pXYDistance,
DgnElementCP     pAkimaCurve,
TransformCP     pElementToWorld,
DPoint3dCP      pSpacePoint,
DMatrix4dCP     pWorldToView
)
    {
    DPoint3dCP  pVertice3d = NULL;
    DPoint2dCP  pVertice2d = NULL;
    bool        is3d = pAkimaCurve->Is3d();
    int         numAkimaPts;

    if (!pAkimaCurve || !pSpacePoint || !pWorldToView)
        return DGNHANDLERS_STATUS_BadArg;

    if (CURVE_ELM != pAkimaCurve->GetLegacyType())
        return ERROR; //MDLERR_BADTYPE;

    if (6 > (numAkimaPts = LineStringUtil::GetCount (*pAkimaCurve)))
        return DGNHANDLERS_STATUS_BadElement;

    if (is3d)
        pVertice3d = pAkimaCurve->ToLine_String_3d().vertice;
    else
        pVertice2d = pAkimaCurve->ToLine_String_2d().vertice;

    return closestPointFromAkimaPointsXY (pCurvePoint, pCurveParam, pXYDistance, 
                                          is3d ? pVertice3d : NULL, is3d ? NULL : pVertice2d, numAkimaPts,
                                          pElementToWorld, pSpacePoint, pWorldToView);
    }

/*---------------------------------------------------------------------------------**//**
@description Compute the distance along the Akima curve between two parameters in the linestring parameterization.
@remarks Negative parameter direction produces negative length.
@remarks Both linestring and B-spline parameterizations of the Akima curve span the interval [0,1], but the Bezier segments
    in the former are distributed uniformly, while in the latter, they are distributed by relative cumulative chord length.
    Convert from one to the other with ~mbspAkima_linestringToBsplineParameter and ~mbspAkima_bsplineToLinestringParameter.
@param pArcLength  OUT computed arc distance
@param pAkimaCurve IN subject curve
@param startParam  IN start parameter in [0,1] (linestring parameterization)
@param endParam    IN end parameter in [0,1] (linestring parameterization)
@return SUCCESS if valid Akima element
@bsimethod                                                      EarlinLutz      08/06
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt   bspAkima_signedDistanceAlong
(
double*         pArcLength,
DgnElementCP     pAkimaCurve,
double          startParam,
double          endParam
)
    {
    if (!pAkimaCurve)
        return DGNHANDLERS_STATUS_BadArg;

    if (CURVE_ELM != pAkimaCurve->GetLegacyType())
        return ERROR; //MDLERR_BADTYPE;

    if (startParam > endParam)
        {
        StatusInt status = bspAkima_signedDistanceAlong (pArcLength, pAkimaCurve, endParam, startParam);

        if (pArcLength)
            *pArcLength = - *pArcLength;

        return status;
        }

    int     numAkimaPts = LineStringUtil::GetCount (*pAkimaCurve);
    int     numSegment = numAkimaPts - 5;

    if (numSegment <= 0)
        return DGNHANDLERS_STATUS_BadElement;

    // expand linestring parameters into local segment index in [0,numSegment-1] and fraction parameter in [0,1]
    if (startParam < 0.0)
        startParam = 0.0;

    if (endParam > 1.0)
        endParam = 1.0;

    int     startIndex = (int) (startParam * numSegment);
    int     endIndex = (int) (endParam * numSegment);

    if (endIndex > numSegment - 1)
        endIndex = numSegment - 1;

    double      startBezierParam = startParam * numSegment - startIndex;
    double      endBezierParam = endParam * numSegment - endIndex;
    DPoint3d    xyz[AKIMA_BEZIER_ORDER];
    double      u0, u1, abstol =  0.0, reltol = -1.0, aTotal =  0.0;

    for (int segment = startIndex; segment <= endIndex; segment++)
        {
        if (SUCCESS == bspAkima_extractIndexedBezier (xyz, NULL, NULL, pAkimaCurve, segment, &abstol, reltol))
            {
            DPoint4d    xyzw[AKIMA_BEZIER_ORDER];

            for (int i = 0; i < AKIMA_BEZIER_ORDER; i++)
                bsiDPoint4d_initFromDPoint3dAndWeight (&xyzw[i], &xyz[i], 1.0);

            u0 = (segment == startIndex) ? startBezierParam : 0.0;
            u1 = (segment == endIndex)   ? endBezierParam   : 1.0;

            double aCurr = bsiBezierDPoint4d_arcLength (xyzw, AKIMA_BEZIER_ORDER, u0, u1);
            aTotal += aCurr;
            }
        }

    if (pArcLength)
        *pArcLength = aTotal;

    return SUCCESS;
    }

#endif

/*---------------------------------------------------------------------------------**//**
@description query the point count in the Akima element.
@param pAkimaElement IN source element
@bsimethod                                                      EarlinLutz      08/06
+---------------+---------------+---------------+---------------+---------------+------*/
static int     bspAkima_getPointCount (DgnElementCP pAkimaElement)
    {
    int count;

    // Akima curves, 2d linestring, and 3d linestrings have numverts in the same place.
    //    Standard practice is to look at the line_string_2d flavor of the union.

    if (!pAkimaElement || CURVE_ELM != pAkimaElement->GetLegacyType() || (int)pAkimaElement->ToLine_String_2d().numverts < 0)
        count = 0;
    else if (pAkimaElement->ToLine_String_2d().numverts > MAX_VERTICES)
        count = MAX_VERTICES;
    else
        count = pAkimaElement->ToLine_String_2d().numverts;

    return count;
    }

/*---------------------------------------------------------------------------------**//**
@description inplace reversal of point order in an akima element.
@param pAkimaCurve IN source element.
@returns ERROR if not an akima element.
@bsimethod                                                      EarlinLutz      08/06
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt   bspAkima_reverseInPlace (EditElementHandleR eeh)
    {
    int         numAkimaPts;

    if (6 > (numAkimaPts = bspAkima_getPointCount (eeh.GetElementCP ())))
        return DGNHANDLERS_STATUS_BadElement;

    if (eeh.GetElementCP ()->Is3d())
        {
        DPoint3dP   pBuffer = eeh.GetElementP ()->ToLine_String_3dR().vertice;
        DPoint3d    temp;

        for (int i = 0, j = numAkimaPts - 1; i < j; i++, j--)
            {
            temp = pBuffer[i];
            pBuffer[i] = pBuffer[j];
            pBuffer[j] = temp;
            }
        }
    else
        {
        DPoint2dP   pBuffer = eeh.GetElementP ()->ToLine_String_2dR().vertice;
        DPoint2d    temp;

        for (int i = 0, j = numAkimaPts - 1; i < j; i++, j--)
            {
            temp = pBuffer[i];
            pBuffer[i] = pBuffer[j];
            pBuffer[j] = temp;
            }
        }

    return SUCCESS;
    }

#ifdef UNUSED_FUNCTION

/*---------------------------------------------------------------------------------**//**
@description Access a single point from an akima.
@param pXYZ OUT returned point
@param pAkimaElement IN source element
@param index IN point index.   The two hidden points for start and end tangent control
    are indexed as 0,1 at start, n-1 and n-2 at end of Akima with n nominal points.
@bsimethod                                                      EarlinLutz      08/06
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt   bspAkima_getIndexedPoint
(
DPoint3dP       pXYZ,
DgnElementCP     pAkimaCurve,
int             index
)
    {
    int         numPoints = bspAkima_getPointCount (pAkimaCurve);

    if (index < 0 || index >= numPoints)
        return DGNHANDLERS_STATUS_BadElement;

    if (pAkimaCurve->Is3d())
        {
        *pXYZ = pAkimaCurve->ToLine_String_3d().vertice [index];
        }
    else
        {
        pXYZ->x = pAkimaCurve->ToLine_String_2d().vertice [index].x;
        pXYZ->y = pAkimaCurve->ToLine_String_2d().vertice [index].y;
        pXYZ->z = 0.0;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
@description return 3 points that can act as endpoint and two hidden points to control
   end tangent.
@param pXYZ0 OUT point on curve
@param pXYZ1 OUT first target point
@param pXYZ2 OUT second target point
@param pAkimaElement IN source element
@param segment IN segment index
@param segmentFraction IN fraction within indexed segment bezier
@param bReverse IN true to aim tangent backwards.
@return SUCCESS if valid akima element.
@bsimethod                                                      EarlinLutz    08/06
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt    bspAkima_getTangentPoints
(
DPoint3dP       pXYZ0,
DPoint3dP       pXYZ1,
DPoint3dP       pXYZ2,
DgnElementCP     pAkimaElement,
int             segmentIndex,
double          segmentFraction,
bool            bReverse,
double*         pAbsTol,
double          relTol
)
    {
    int         numAkimaPts = bspAkima_getPointCount (pAkimaElement);
    int         numSegment = numAkimaPts - 5;

    // Copy exisiting points if request is at an endpoint.
    if (bReverse && segmentIndex == 0 && segmentFraction < s_minInterval)
        {
        bspAkima_getIndexedPoint (pXYZ0, pAkimaElement, 2);
        bspAkima_getIndexedPoint (pXYZ1, pAkimaElement, 1);
        bspAkima_getIndexedPoint (pXYZ2, pAkimaElement, 0);
        }
    else if (!bReverse && segmentIndex == numSegment - 1 && segmentFraction >= 1.0 - s_minInterval)
        {
        if (bspAkima_getIndexedPoint (pXYZ0, pAkimaElement, segmentIndex + 3))
            return ERROR;

        if (bspAkima_getIndexedPoint (pXYZ1, pAkimaElement, segmentIndex + 4))
            return ERROR;

        if (bspAkima_getIndexedPoint (pXYZ2, pAkimaElement, segmentIndex + 5))
            return ERROR;
        }
    else
        {
        DPoint3d    cp[AKIMA_BEZIER_ORDER];
        DPoint3d    dxyz;
        double      s = bReverse ? -1.0 : 1.0;

        if (SUCCESS != bspAkima_extractIndexedBezier (cp, NULL, NULL, pAkimaElement, segmentIndex, pAbsTol, relTol))
            return ERROR;

        double dist = bsiDPoint3d_distance (&cp[0], &cp[AKIMA_BEZIER_ORDER - 1]);
        bsiBezierDPoint3d_evaluateDPoint3d (pXYZ0, &dxyz, cp, AKIMA_BEZIER_ORDER, segmentFraction);
        bsiDPoint3d_scaleToLength (&dxyz, &dxyz, dist);
        bsiDPoint3d_addScaledDPoint3d (pXYZ1, pXYZ0, &dxyz, 0.5 * s);
        bsiDPoint3d_addScaledDPoint3d (pXYZ2, pXYZ0, &dxyz, 1.0 * s);
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @description Convert the Akima parameter from linestring to B-spline parameterization.
* @remarks Both linestring and B-spline parameterizations of the Akima curve span the interval [0,1], but the Bezier segments
*   in the former are distributed uniformly, while in the latter, they are distributed by relative cumulative chord length.
* @param pBsplineParameter      OUT     parameter in B-spline parametrization of Akima curve
* @param linestringParameter    IN      parameter in linestring parametrization of Akima curve
* @param pAkimaCurve            IN      type 11 curve element
* @param pAbsTol                IN OUT  min length of a nontrivial Bezier segment.  If NULL, a value is computed using relTol; otherwise,
*                                       if negative, this computed tolerance is returned to facilitate looped execution.
* @param relTol                 IN      (if !pAbsTol or *pAbsTol < 0) fractional relative tolerance for computing pAbsTol.  If nonpositive,
*                                       the default value 1.0e-5 is used.
* @return SUCCESS if the parameter is converted,
*       DGNHANDLERS_STATUS_BadElement if there are less than 6 Akima points or there was a computation error,
*       DGNHANDLERS_STATUS_BadArg if required inputs are NULL or out of range, or
*       MDLERR_BADTYPE if the element is not an Akima curve.
* @see bspAkima_bsplineToLinestringParameter
* @bsimethod                                                    DavidAssaf      08/06
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt   bspAkima_linestringToBsplineParameter
(
double*         pBsplineParameter,
double          linestringParameter,
DgnElementCP     pAkimaCurve,
double*         pAbsTol,
double          relTol
)
    {
    if (!pAkimaCurve || !pBsplineParameter || linestringParameter < 0.0 || linestringParameter > 1.0)
        return DGNHANDLERS_STATUS_BadArg;

    if (CURVE_ELM != pAkimaCurve->GetLegacyType())
        return ERROR; //MDLERR_BADTYPE;

    if (linestringParameter == 0.0 || linestringParameter == 1.0)
        {
        *pBsplineParameter = linestringParameter;

        return SUCCESS;
        }

    int numAkimaPts = LineStringUtil::GetCount (*pAkimaCurve);
    int numBezierSegments = numAkimaPts - 5;

    if (numBezierSegments <= 0)
        return DGNHANDLERS_STATUS_BadElement;

    // expand linestring parameter into local segment index and fraction parameter
    double  unnormalizedLinestringParam = linestringParameter * numBezierSegments;
    int     bezierIndex = (int) unnormalizedLinestringParam;
    double  bezierParameter = unnormalizedLinestringParam - bezierIndex;

    // find the containing Bezier segment
    double      startParam, endParam;
    StatusInt   status = bspAkima_extractIndexedBezier (NULL, &startParam, &endParam, pAkimaCurve, bezierIndex, pAbsTol, relTol);

    if (SUCCESS != status && AKIMACURVE_STATUS_NullSolution != status)
        return DGNHANDLERS_STATUS_BadElement;

    *pBsplineParameter = bezierParameter * endParam + (1.0 - bezierParameter) * startParam;

    return SUCCESS;
    }

#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    DavidAssaf      04/06
+---------------+---------------+---------------+---------------+---------------+------*/
static void    akima_extractHiddenPoints
(
DPoint3dP       firstTwoHiddenPoints,
DPoint3dP       lastTwoHiddenPoints,
DgnElementCP     in
)
    {
    if (in->Is3d())
        {
        if (firstTwoHiddenPoints)
            memcpy (firstTwoHiddenPoints, in->ToLine_String_3d().vertice, 2 * sizeof (DPoint3d));

        if (lastTwoHiddenPoints)
            memcpy (lastTwoHiddenPoints, &in->ToLine_String_3d().vertice[in->ToLine_String_3d().numverts - 2], 2 * sizeof (DPoint3d));

        return;
        }

    if (firstTwoHiddenPoints)
        {
        firstTwoHiddenPoints[0].x = in->ToLine_String_2d().vertice[0].x;
        firstTwoHiddenPoints[0].y = in->ToLine_String_2d().vertice[0].y;
        firstTwoHiddenPoints[0].z = 0.0;
        firstTwoHiddenPoints[1].x = in->ToLine_String_2d().vertice[1].x;
        firstTwoHiddenPoints[1].y = in->ToLine_String_2d().vertice[1].y;
        firstTwoHiddenPoints[1].z = 0.0;
        }

    if (lastTwoHiddenPoints)
        {
        UInt32 nVerts = in->ToLine_String_2d().numverts;
        lastTwoHiddenPoints[0].x = in->ToLine_String_2d().vertice[nVerts - 2].x;
        lastTwoHiddenPoints[0].y = in->ToLine_String_2d().vertice[nVerts - 2].y;
        lastTwoHiddenPoints[0].z = 0.0;
        lastTwoHiddenPoints[1].x = in->ToLine_String_2d().vertice[nVerts - 1].x;
        lastTwoHiddenPoints[1].y = in->ToLine_String_2d().vertice[nVerts - 1].y;
        lastTwoHiddenPoints[1].z = 0.0;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @description  Extract pole-based curve and Akima points from type 11 curve element.
*
* @param baseCurve      OUT     pole-based curve
* @param akimaCurve     OUT     full array of Akima fit points (or NULL)
* @param numPts         OUT     number of Akima fit points (or NULL)
* @param in             IN      type 11 curve element
* @return SUCCESS if extractions successful
* @bsimethod                                                    DavidAssaf      03/06
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt   akima_extractCurvesFromElement
(
MSBsplineCurveP         baseCurve,
bvector<DPoint3d>*  akimaPoints,
int*                    numPts,
ElementHandleCR         akimaEhIn
)
    {
    // extract Akima point count from pole-based equivalent so that we don't get duplicates
    if (SUCCESS != BSplineCurveHandler::CurveFromElement (*baseCurve, akimaEhIn))
        return ERROR;

    int         nPts = (baseCurve->params.numPoles - 1) / 3 + 5;

    if (akimaPoints)
        {
        // take 2 starting targets out of the element
        // intermediate points from curve (at step of 3)
        // 2 final targets out of the element
        DPoint3d startTarget[2];
        DPoint3d endTarget[2];
        // copy visible Akima points from relevant poles, hidden points directly from element
        akima_extractHiddenPoints (startTarget, endTarget, akimaEhIn.GetElementCP ());
        akimaPoints->push_back (startTarget[0]);
        akimaPoints->push_back (startTarget[1]);

        for (int i = 0; i < nPts - 4; i++)
            akimaPoints->push_back (baseCurve->GetPole (3 * i));
        akimaPoints->push_back (endTarget[0]);
        akimaPoints->push_back (endTarget[1]);
        }

    if (numPts)
        *numPts = nPts;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @description Compute the discrete distance from a segment of a sampled base curve to the
*       given Akima segment approximating that base curve segment.
* @remarks Exactly 2^sampleLevel baseCurve samples are returned in the output array, but if a sample
*       does not vary by more than thresholdDistance from the Akima curve, a DISCONNECT point is
*       stored in its place.  The input array may contain DISCONNECTs.
* @remarks The sampleLevel determines the binary samples as follows:
*       <UL>
*       <LI>sampleLevel=0: sample at the middle parameter.
*       <LI>sampleLevel=1: sample at the 1/4 and 3/4 parameters.
*       <LI>sampleLevel=2: sample at the 1/8, 3/8, 5/8 and 7/8 parameters.
*       <LI>...
*       </UL>
* @param maxDistance            OUT     maximum distance of all computed samples to the given Akima curve
* @param baseSegmentSamplePts   OUT     2^sampleLevel samples from the base curve segment (may include DISCONNECT points)
* @param baseCurve              IN      pole-based curve (equivalent to original Akima curve) to sample
* @param param0                 IN      B-spline parameter at start of baseCurve segment to approximate
* @param param1                 IN      B-spline parameter at end of baseCurve segment to approximate
* @param akimaSegmentPts        IN      full array of fit points of the Akima curve approximating the baseCurve segment
* @param thresholdDistance      IN      samples that don't exceed this distance are stored as DISCONNECTs in output array
* @param sampleLevel            IN      binary subdivision level at which to sample baseCurve, in [0, s_maxSampleLevel]
* @return SUCCESS if sampling successful
* @bsimethod                                                    DavidAssaf      03/06
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt   akima_discreteDistanceFromCurveSegment
(
double*                     maxDistance,
bvector<DPoint3d>&      baseSegmentSamplePts,
const MSBsplineCurve*       baseCurve,
double                      param0,
double                      param1,
bvector<DPoint3d>&akimaSegmentPts,
double                      thresholdDistance,
int                         sampleLevel
)
    {
    if (!maxDistance || !baseCurve || thresholdDistance < 0.0 ||
        param0 >= param1 || param0 < 0.0 || param1 > 1.0 || sampleLevel < 0 || sampleLevel > s_maxSampleLevel)
        return DGNHANDLERS_STATUS_BadArg;

    baseSegmentSamplePts.clear ();

    MSBsplineCurve  akimaSegment;

    if (SUCCESS != akimaSegment.InitAkima (&akimaSegmentPts[0], (int) akimaSegmentPts.size ()))
        return ERROR;

    StatusInt   status = SUCCESS;
    int         numSamples = 1 << sampleLevel;
    double      knotSpan = param1 - param0, param, sampleDistance, maxDist = 0.0;
    DPoint3d    sample;

    // compute and store numSamples samples at odd parameter intervals that improve the last max distance
    for (int i = 1; i < 2 * numSamples; i += 2)
        {
        if (i >= s_minNumSamples && i <= 2 * numSamples - s_minNumSamples)
            {
            // limit samples to s_minSamples/2 at each end of Bezier segment where they tend to improve the fit
            bsiDPoint3d_initDisconnect (&sample);
            }
        else
            {
            param = param0 + (i / (2.0 * numSamples)) * knotSpan;
            baseCurve->FractionToPoint (sample, param);

            double      uor_res = akimaSegment.Resolution ();

            // distance from sample taken from original curve to the akima segment approximation
            if (SUCCESS != (status = bsprcurv_minDistToCurve (&sampleDistance, NULL, NULL, &sample, &akimaSegment, &uor_res, NULL)))
                break;

            if (maxDist < sampleDistance)
                maxDist = sampleDistance;

            // store a dummy point if this sample isn't necessary to improve our fit,
            // but always store real samples at all levels below minimum sample level (we'll remove them later if they don't improve fit)
            if (sampleDistance <= thresholdDistance && sampleLevel >= s_minSampleLevel)
                bsiDPoint3d_initDisconnect (&sample);
            }

        baseSegmentSamplePts.push_back (sample);
        }

    if (SUCCESS == status)
        *maxDistance = maxDist;

    akimaSegment.ReleaseMem ();

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @description Construct the Akima fit points that determine the best approximation of a
*       segment of an Akima curve, given various constraints.
* @param akimaSegment       OUT     Akima fit points of the approximation
* @param akimaCurve         IN      Akima fit points of the input curve
* @param baseCurve          IN      pole-based version of the input curve
* @param param0             IN      B-spline parameter at start of Akima segment to approximate
* @param param1             IN      B-spline parameter at end of Akima segment to approximate
* @param maxDistance        IN      maximum deviation of the returned approximation from the exact Akima segment
* @param maxApproxPts       IN      maximum points that the returned approximation can have
* @param maxSampleLevel     IN      compute 2^(maxSampleLevel+1) - 1 samples per Bezier interval; in [0,s_maxSampleLevel]
* @return SUCCESS if an approximation is computed
* @bsimethod                                                    DavidAssaf      03/06
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt   akima_approximateCurveSegment
(
bvector<DPoint3d>   &akimaSegment,
bvector<DPoint3d>   &akimaCurve,
const MSBsplineCurve*   baseCurve,
double                  param0,
double                  param1,
double                  maxDistance,
int                     maxApproxPts,
int                     maxSampleLevel
)
    {
    const DPoint3d*         akimaCurvePts = NULL;
    DPoint3d                endPts[3], sample, *akimaSegmentPts = NULL;
    double                  knot, lastKnot, testDist;
    StatusInt               status;
    int                     i, j, k, ptsToCopyIndex, numPtsToCopy, interiorKnotIndex, akimaSegmentInsertionIndex, numSamples, sampleLevel,
                            numAkimaPts, numAkimaPtsToAdd;
    bool                    bInitiallyWithinTolerance;

    if (!baseCurve || param0 >= param1 || param0 < 0.0 || param1 > 1.0 || maxDistance < 0.0)
        return DGNHANDLERS_STATUS_BadArg;

    if (maxSampleLevel < 0 || maxSampleLevel > s_maxSampleLevel)
        maxSampleLevel = s_maxSampleLevel;

    if (maxApproxPts < 6 || maxApproxPts > MAX_VERTICES)
        maxApproxPts = MAX_VERTICES;

    akimaSegment.clear ();

    akimaCurvePts = &akimaCurve[0];
    numAkimaPts =   (int)akimaCurve.size ();

    if (!akimaCurvePts || numAkimaPts < 6)
        return BSPLINE_STATUS_TooFewPoles;

    // set first two hidden and start Akima pts at same distance as in akimaCurve; compute indices to relevant interior Akima pts and knots
    endPts[0] = akimaCurve[0];
    endPts[1] = akimaCurve[1];
    endPts[2] = akimaCurve[2];
    ptsToCopyIndex = 3;
    interiorKnotIndex = 4;

    if (param0 >= KNOT_TOLERANCE_BASIS)
        {
        DVec3d    tangent;
        double      tangentLength = 1.0e-3 * bsiDPoint3d_magnitude (&endPts[2]);

        baseCurve->FractionToPoint (endPts[2], tangent, param0);
        bsiDPoint3d_normalizeInPlace (&tangent);

        if (tangentLength < 1.0)
            tangentLength = 1.0;

        endPts[1].SumOf (endPts[2], tangent, -tangentLength);
        endPts[0].SumOf (endPts[1], tangent, -tangentLength);

        int spanIndex = (int)baseCurve->FindKnotInterval (param0);
        ptsToCopyIndex = 2 + (spanIndex - 3) / 3 + 1;

        if (param0 == baseCurve->GetKnot (spanIndex + 1))
            {
            ptsToCopyIndex++;   // param0 is an Akima knot; don't add its Akima point twice
            spanIndex += 3;     // move to last multiple of param0 (Akima interior knots are mult 3)
            }

        interiorKnotIndex = spanIndex + 1;  // start indexing interior knots at end of this span
        }

    if (ptsToCopyIndex > numAkimaPts - 3)
        ptsToCopyIndex = numAkimaPts - 3;

    DPoint3dOps::Append (&akimaSegment, endPts, 3);

    // set end and last two hidden Akima pts at same distance as in akimaCurve; compute relevant interior Akima pt count
    endPts[0] = akimaCurve[numAkimaPts - 3];
    endPts[1] = akimaCurve[numAkimaPts - 2];
    endPts[2] = akimaCurve[numAkimaPts - 1];
    numPtsToCopy = numAkimaPts - 3 - ptsToCopyIndex;

    if (param1 <= 1.0 - KNOT_TOLERANCE_BASIS)
        {
        DVec3d      tangent;
        double      tangentLength = 1.0e-3 * bsiDPoint3d_magnitude (&endPts[0]);
        int         spanIndex;

        baseCurve->FractionToPoint (endPts[0], tangent, param1);
        bsiDPoint3d_normalizeInPlace (&tangent);

        if (tangentLength < 1.0)
            tangentLength = 1.0;

        endPts[1].SumOf (endPts[0], tangent, tangentLength);
        endPts[2].SumOf (endPts[1], tangent, tangentLength);

        spanIndex = (int)baseCurve->FindKnotInterval (param1);
        numPtsToCopy = (2 + (spanIndex - 3) / 3 + 1) - ptsToCopyIndex;
        }

    if (numPtsToCopy < 0)
        numPtsToCopy = 0;

    // copy relevant interior Akima pts
    BeAssert (ptsToCopyIndex >= 3);
    BeAssert (ptsToCopyIndex + numPtsToCopy <= numAkimaPts - 3);

    DPoint3dOps::Append (&akimaSegment, &akimaCurvePts[ptsToCopyIndex], numPtsToCopy);
    DPoint3dOps::Append (&akimaSegment, endPts, 3);

    bvector<DPoint3d> samplePts;

    // for each Bezier segment, oversample additional Akima points from baseCurve until evolving akimaSegment is close to baseCurve
    BeAssert (interiorKnotIndex >= 4);
    BeAssert (interiorKnotIndex + 3 * (numPtsToCopy - 1) <= baseCurve->params.numPoles - 3);
    lastKnot = param0;
    akimaSegmentInsertionIndex = 2;
    numAkimaPts = (int)akimaSegment.size ();

    for (i = 0; i <= numPtsToCopy; i++)
        {
        knot = (i < numPtsToCopy) ? baseCurve->GetKnot (interiorKnotIndex + 3 * i) : param1;
        bInitiallyWithinTolerance = true;
        sampleLevel = numSamples = 0;
        testDist = 0.0;

        // sample within this Bezier segment of baseCurve[lastKnot, knot]
        while (sampleLevel <= maxSampleLevel)
            {
            // compute numSamples samples at this level and compute dist to current akimaSegment
            numSamples = 1 << sampleLevel;

            if (SUCCESS != (status = akima_discreteDistanceFromCurveSegment (&testDist, samplePts, baseCurve, lastKnot, knot, akimaSegment, maxDistance, sampleLevel)))
                return status;

            if (testDist <= maxDistance)
                {
                // did not improve this Bezier segment after minimum iterations; restore exact segment and proceed to next
                if (sampleLevel == s_minSampleLevel && bInitiallyWithinTolerance)
                    {
                    for (k = 1; k < numSamples; k++)
                        {
                        sample = akimaSegment[akimaSegmentInsertionIndex + k];

                        if (!bsiDPoint3d_isDisconnect (&sample))
                            {
                            // mark previous levels' samples for removal
                            bsiDPoint3d_initDisconnect (&sample);
                            akimaSegment[akimaSegmentInsertionIndex + k] = sample;
                            numAkimaPts--;
                            }
                        }
                    break;
                    }

                // did not improve this Bezier segment; proceed to next
                if (sampleLevel >= s_minSampleLevel)
                    break;
                }
            else if (sampleLevel < s_minSampleLevel)
                {
                bInitiallyWithinTolerance = false;
                }

            // increment actual akima point count by number of relevant samples
            for (k = numAkimaPtsToAdd = 0; k < numSamples; k++)
                {
                sample = samplePts[k];

                if (!bsiDPoint3d_isDisconnect (&sample))
                    numAkimaPtsToAdd++;
                }

            // cannot improve this Bezier segment, but maybe we can squeeze a few more points into the next
            if (numAkimaPts + numAkimaPtsToAdd > maxApproxPts)
                break;

            // interleave samples (and DISCONNECTs) at this level with those of previous levels
            for (j = 1, k = 0; k < numSamples; j += 2, k++)
                {
                sample = samplePts[k];
                akimaSegment.insert (akimaSegment.begin () + akimaSegmentInsertionIndex + j, 1, sample);
                }

            // update # Akima points, # segment samples
            numAkimaPts += numAkimaPtsToAdd;
            numSamples *= 2;
            sampleLevel++;
            }

        akimaSegmentInsertionIndex += numSamples;
        lastKnot = knot;
        }

    // strip disconnects
    akimaSegmentPts = &akimaSegment[0];
    numAkimaPts = (int)akimaSegment.size ();
    mdlBspline_removeCoincidentPoints (akimaSegmentPts, NULL, &numAkimaPts, akimaSegmentPts, NULL, numAkimaPts, 0.0, true, false);
    akimaSegment.resize (numAkimaPts);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @description Return an approximation of the segment of an Akima (type 11) curve lying between the given parameters.
@remarks Both linestring and B-spline parameterizations of the Akima curve span the interval [0,1], but the Bezier segments
    in the former are distributed uniformly, while in the latter, they are distributed by relative cumulative chord length.
    Convert from one to the other with ~mbspAkima_linestringToBsplineParameter and ~mbspAkima_bsplineToLinestringParameter.
* @param out        OUT     Akima curve segment
* @param in         IN      Akima curve to segment
* @param pTransform IN      transform to apply to output
* @param param0     IN      start parameter of segment in [0,1] (B-spline parameterization)
* @param param1     IN      end parameter of segment in [0,1] (B-spline parameterization)
* @param tolerance  IN      absolute distance tolerance for the approximation (negative for default tolerance, zero maximizes iterations)
* @return SUCCESS if curve segment is returned
* @see bspAkima_generatePartial
* @bsimethod                                                    DavidAssaf      03/06
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt   bspAkima_segmentAkimaCurve
(
EditElementHandleR  eeh,
ElementHandleCR     akimaEhIn,
TransformCP         pTransform,
double              param0,
double              param1,
double              tolerance
)
    {
    bool        isIdentity = (!pTransform || bsiTransform_isIdentity (pTransform)), reverse;

    if (reverse = (param0 > param1))
        {
        double  tmpParam = param0;

        param0 = param1;
        param1 = tmpParam;
        }

    if (param0 < 0.0)
        param0 = 0.0;

    if (param1 > 1.0)
        param1 = 1.0;

    if (param1 - param0 < KNOT_TOLERANCE_BASIS)
        return AKIMACURVE_STATUS_NullSolution;

    if (param1 - param0 > 1.0 - KNOT_TOLERANCE_BASIS)
        {
        if (isIdentity)
            {
            eeh.Duplicate (akimaEhIn);

            return (reverse ? bspAkima_reverseInPlace (eeh) : SUCCESS);
            }

        param0 = 0.0;
        param1 = 1.0;
        }

    bvector<DPoint3d> akimaPoints;
    bvector<DPoint3d> akimaSegment;

    int             numPts;
    DPoint3dP       inPts;
    MSBsplineCurve  baseCurve;

    if (SUCCESS != akima_extractCurvesFromElement (&baseCurve, &akimaPoints, &numPts, akimaEhIn))
        {
        baseCurve.ReleaseMem ();
        return ERROR;
        }

    inPts = &akimaPoints[0];

    // base curve and Akima pts transform exactly, but the Akima curve may differ from the transformed base curve b/c it is not affine invariant
    if (!isIdentity)
        {
        baseCurve.TransformCurve (*pTransform);
        bsiTransform_multiplyDPoint3dArrayInPlace (pTransform, inPts, numPts);
        }

    // set default tol from range of visible Akima points
    if (tolerance < 0.0)
        tolerance = (s_defaultAkimaSampleTol * bsiDPoint3d_getLargestCoordinateDifference (inPts + 2, numPts - 4));

    if (SUCCESS != akima_approximateCurveSegment (akimaSegment, akimaPoints, &baseCurve, param0, param1, tolerance, MAX_VERTICES, -1))
        {
        baseCurve.ReleaseMem ();
        return ERROR;
        }

    if (reverse)
        DPoint3dOps::Reverse (akimaSegment);

    StatusInt   status = CurveHandler::CreateCurveElement (eeh, &akimaEhIn,
                    &akimaSegment[0], akimaSegment.size (), akimaEhIn.GetElementCP ()->Is3d(), *akimaEhIn.GetDgnModelP ());

    baseCurve.ReleaseMem ();

    return status;
    }

#ifdef UNUSED_FUNCTION

/*---------------------------------------------------------------------------------**//**
@description Compute a segment of the Akima curve.
@remarks Both linestring and B-spline parameterizations of the Akima curve span the interval [0,1], but the Bezier segments
    in the former are distributed uniformly, while in the latter, they are distributed by relative cumulative chord length.
    Convert from one to the other with ~mbspAkima_linestringToBsplineParameter and ~mbspAkima_bsplineToLinestringParameter.
@param pAkimaCurveOut OUT newly constructed element
@param pAkimaCurveIn IN source element
@param pTransform IN transform for output element
@param param0 IN start parameter of segment in [0,1] (linestring parameterization)
@param param1 IN end parameter of segment in [0,1] (linestring parameterization)
@param useFastMethod IN true to use a faster, coarse approximation (e.g., for dynamics); otherwise, use slower, fine approximation
@return SUCCESS if valid akima element
@see mdlBspline_segmentAkimaCurveExt
@bsimethod                                                      EarlinLutz      08/06
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt   bspAkima_generatePartial
(
EditElementHandleR eeh,
ElementHandleCR    akimaEhIn,
TransformCP     pTransform,
double          param0,
double          param1,
bool            useFastMethod
)
    {
    DgnElementCP pAkimaCurveIn = akimaEhIn.GetElementCP ();

    if (!useFastMethod)
        {
        double      bspParam0, bspParam1, tol = -1.0;
        StatusInt   status;

        if (SUCCESS == (status = bspAkima_linestringToBsplineParameter (&bspParam0, param0, pAkimaCurveIn, &tol, s_defaultAkimaSampleTol)) &&
            SUCCESS == (status = bspAkima_linestringToBsplineParameter (&bspParam1, param1, pAkimaCurveIn, &tol, s_defaultAkimaSampleTol)))
            return bspAkima_segmentAkimaCurve (eeh, akimaEhIn, pTransform, bspParam0, bspParam1, tol);
        }

    double      abstol = 0.0;
    double      reltol = -1.0;

    if (param0 > param1)
        {
        if (SUCCESS != bspAkima_generatePartial (eeh, akimaEhIn, pTransform, param1, param0, useFastMethod))
            return ERROR;

        return bspAkima_reverseInPlace (eeh);
        }

    if (param1 > 1.0)
        param1 = 1.0;

    if (param1 < 0.0)
        param1 = 0.0;

    if (param0 > 1.0)
        param0 = 1.0;

    if (param0 < 0.0)
        param0 = 0.0;

    if (param1 < param0 + s_minInterval)
        return ERROR;

    int         numAkimaPts = LineStringUtil::GetCount (*pAkimaCurveIn);
    int         numSegment = numAkimaPts - 5;

    DPoint3d xyzOut[MAX_VERTICES];
    int numOut;

    if (6 > numAkimaPts)
        return DGNHANDLERS_STATUS_BadElement;

    int         segment0 = (int)floor (param0 * numSegment);

    if (segment0 >= numSegment)
        segment0 = numSegment - 1;

    int         segment1 = (int)floor (param1 * numSegment);

    if (segment1 >= numSegment)
        segment1 = numSegment - 1;

    double segmentFraction0 = param0 * numSegment - segment0;
    double segmentFraction1 = param1 * numSegment - segment1;

    bspAkima_getTangentPoints (&xyzOut[2], &xyzOut[1], &xyzOut[0], pAkimaCurveIn, segment0, segmentFraction0, true, &abstol, reltol);
    numOut = 3;

    int firstCopy = segment0 + 1;
    int lastCopy  = segment1;

    // Prevent double points if start or end fraction is at interior point....
    if (segmentFraction0 >= 1.0 - s_minInterval)
        firstCopy++;

    if (segmentFraction1 <= s_minInterval)
        lastCopy--;

    for (int segment = firstCopy; segment <= lastCopy; segment++)
        {
        if (numOut >= MAX_VERTICES)
            return ERROR;

        bspAkima_getIndexedPoint (&xyzOut[numOut], pAkimaCurveIn, 2 + segment);
        numOut++;
        }

    if (numOut + 3 > MAX_VERTICES)
        return ERROR;

    bspAkima_getTangentPoints (&xyzOut[numOut], &xyzOut[numOut + 1], &xyzOut[numOut + 2], pAkimaCurveIn, segment1, segmentFraction1, false, &abstol, reltol);
    numOut += 3;

    // Ummm ... does it really transform pointwise??
    if (pTransform)
        bsiTransform_multiplyDPoint3dArrayInPlace (pTransform, xyzOut, numOut);

    return CurveHandler::CreateCurveElement (eeh, &akimaEhIn, xyzOut, numOut, pAkimaCurveIn->Is3d(), *akimaEhIn.GetDgnModelP ());
    }

/*---------------------------------------------------------------------------------**//**
* @description Compute the length of the Akima curve fit-point polyline.
* @remarks This function can be used for heuristics to detect stream curves (densely fit Akima curves), e.g., if avg &le; fraction * length.
* @remarks Just to be sure, this function filters out DISCONNECT points.
* @param pMaxChordLength    OUT     length of longest fit-point chord (or NULL)
* @param pAvgChordLength1   OUT     average chord length (or NULL)
* @param pAvgChordLength2   OUT     square root of average squared chord length (or NULL)
* @param pAkimaCurve        IN      akima curve
* @return length of polyline or zero if invalid curve
* @bsimethod                                                    DavidAssaf      09/06
+---------------+---------------+---------------+---------------+---------------+------*/
static double  bspAkima_computePolylineLength
(
double*         pMaxChordLength,
double*         pAvgChordLength1,
double*         pAvgChordLength2,
DgnElementCP     pAkimaCurve
)
    {
    DPoint3d    pt0, pt1;
    double      dist, normInfty = 0.0, norm1 = 0.0, norm2 = 0.0, totalChordLength = 0.0;
    int         i = 2, numSegments = 0, numAkimaPts;

    if (pMaxChordLength)
        *pMaxChordLength = 0.0;

    if (pAvgChordLength1)
        *pAvgChordLength1 = 0.0;

    if (pAvgChordLength2)
        *pAvgChordLength2 = 0.0;

    if (!pAkimaCurve || 6 > (numAkimaPts = bspAkima_getPointCount (pAkimaCurve)))
        return 0.0;

    // get first non-disconnect Akima fit point (skip first/last two hidden pts)
    while (SUCCESS == bspAkima_getIndexedPoint (&pt0, pAkimaCurve, i++) && bsiDPoint3d_isDisconnect (&pt0) && i < numAkimaPts - 2);

    while (i < numAkimaPts - 2)
        {
        // get next non-disconnect Akima fit point
        bspAkima_getIndexedPoint (&pt1, pAkimaCurve, i++);

        if (!bsiDPoint3d_isDisconnect (&pt1))
            {
            dist = bsiDPoint3d_distance (&pt0, &pt1);

            if (pMaxChordLength && normInfty < dist)
                normInfty = dist;

            if (pAvgChordLength1)
                norm1 += dist;

            if (pAvgChordLength2)
                norm2 += dist * dist;

            totalChordLength += dist;

            numSegments++;
            pt0 = pt1;
            }
        }

    if (pMaxChordLength)
        *pMaxChordLength = normInfty;

    if (pAvgChordLength1 && numSegments > 0)
        *pAvgChordLength1 = norm1 / numSegments;

    if (pAvgChordLength2 && numSegments > 0)
        *pAvgChordLength2 = sqrt (norm2 / numSegments);

    return totalChordLength;
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @description Return an approximation of the segment of an Akima (type 11) curve lying between the given parameters.
@remarks Both linestring and B-spline parameterizations of the Akima curve span the interval [0,1], but the Bezier segments
    in the former are distributed uniformly, while in the latter, they are distributed by relative cumulative chord length.
    Convert from one to the other with ~mbspAkima_linestringToBsplineParameter and ~mbspAkima_bsplineToLinestringParameter.
* @param out        OUT     Akima curve segment
* @param in         IN      Akima curve to segment
* @param pTransform IN      transform to apply to output
* @param param0     IN      start parameter of segment in [0,1] (B-spline parameterization)
* @param param1     IN      end parameter of segment in [0,1] (B-spline parameterization)
* @param tolerance  IN      absolute distance tolerance for the approximation (negative for default tolerance, zero maximizes iterations)
* @return SUCCESS if curve segment is returned
* @see bspAkima_generatePartial
* @bsimethod                                                    DavidAssaf      03/06
+---------------+---------------+---------------+---------------+---------------+------*/
AkimaCurveStatus    CurveHandler::SegmentAkimaCurve (EditElementHandleR eeh, ElementHandleCR eh, TransformCP transform, double param0, double param1, double tolerance)
    {
    return (AkimaCurveStatus) bspAkima_segmentAkimaCurve (eeh, eh, transform, param0, param1, tolerance);
    }

/*---------------------------------------------------------------------------------**//**
* @description Return an approximation of a segment of an Akima (type 11) curve starting at
*       the given point, ending at a later point and having no more than the given number of points.
* @remark The input/output indices refer to the <EM>compressed</EM> Akima curve point array (successive
*       duplicates omitted) employed by InitAkima.
*
* @param out            OUT     Akima curve segment
* @param endIndex       OUT     index into input curve point array of last visible point of output curve (in [3,n-3], or NULL)
* @param in             IN      Akima curve to segment
* @param startIndex     IN      index into input curve point array of first visible point of output curve (in [2,n-4])
* @param maxPoints      IN      maximum number of points in the returned segment in [6,MAX_PTS] (includes 4 hidden points)
* @param tolerance      IN      absolute distance tolerance for the approximation (negative for default tolerance, zero maximizes iterations)
* @return SUCCESS if segment is returned
* @bsimethod                                                    DavidAssaf      03/06
+---------------+---------------+---------------+---------------+---------------+------*/
AkimaCurveStatus    CurveHandler::SegmentAkimaCurveByPointCount (EditElementHandleR eeh, ElementHandleCR eh, int* endIndex, int startIndex, int maxPoints, double tolerance)
    {
    if (maxPoints < 6)
        maxPoints = 6;

    if (maxPoints > MAX_VERTICES)
        maxPoints = MAX_VERTICES;

    int             numPts;
    MSBsplineCurve  baseCurve;

    if (SUCCESS != akima_extractCurvesFromElement (&baseCurve, NULL, &numPts, eh))
        return AKIMACURVE_STATUS_BadElement;

    if (startIndex < 2)
        startIndex = 2;

    if (startIndex >= numPts - 3)
        {
        baseCurve.ReleaseMem ();

        return AKIMACURVE_STATUS_NullSolution;
        }

    int         knotIdx, endIdx, nextEndIdx;

    endIdx = startIndex + maxPoints - 5;

    if (endIdx > numPts - 3)
        endIdx = numPts - 3;

    if (startIndex >= endIdx)
        {
        baseCurve.ReleaseMem ();

        return AKIMACURVE_STATUS_NullSolution;
        }

    double      param0, param1;

    knotIdx = 3 * (startIndex - 2) + 1;
    BeAssert (knotIdx >= 1 && knotIdx <= baseCurve.params.numPoles - 3);
    param0 = baseCurve.GetKnot (knotIdx);

    nextEndIdx = endIdx;

    // search forward segments of decreasing length for the approximation with max original Akima points but maxPoints or less total points
    do
        {
        endIdx = nextEndIdx;

        knotIdx = 3 * (endIdx - 2) + 1;
        BeAssert (knotIdx >= 4 && knotIdx <= baseCurve.params.numPoles);
        param1 = baseCurve.GetKnot (knotIdx);

        if (AKIMACURVE_STATUS_Success != CurveHandler::SegmentAkimaCurve (eeh, eh, NULL, param0, param1, tolerance))
            {
            baseCurve.ReleaseMem ();

            return AKIMACURVE_STATUS_BadElement;
            }

        PointVector pointVector;
        DPoint3dP   pointsDirect = NULL;
        int         nPointsDirect = 0;

        LineStringUtil::GetLineStringTransformPoints (&pointVector, &pointsDirect, &nPointsDirect, (DgnElementP) eh.GetElementCP (), false);

        int         nPoints;
        DPoint3dP   points;

        if (pointsDirect)
            {
            points  = pointsDirect;
            nPoints = nPointsDirect;
            }
        else
            {
            points  = &pointVector.front ();
            nPoints = static_cast<int>(pointVector.size ());
            }

        if (0 == nPoints)
            {
            baseCurve.ReleaseMem ();

            return AKIMACURVE_STATUS_BadElement;
            }

        nextEndIdx = (startIndex + endIdx) / 2;

        } while (numPts > maxPoints && nextEndIdx > startIndex);

    if (endIndex)
        *endIndex = endIdx;

    baseCurve.ReleaseMem ();

    return AKIMACURVE_STATUS_Success;
    }

#ifdef UNUSED_FUNCTION
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz     10/03
+---------------+---------------+---------------+---------------+---------------+------*/
static bool     readToCurve
(
ElementHandleCR source,
MSBsplineCurveR curve
)
    {
    PointVector pointVector;
    DPoint3dP   pointsDirect = NULL;
    int         nPointsDirect = 0;

    LineStringUtil::GetLineStringTransformPoints (&pointVector, &pointsDirect, &nPointsDirect, (DgnElementP) source.GetElementCP (), false);

    int         nPoints;
    DPoint3dP   points;

    if (pointsDirect)
        {
        points  = pointsDirect;
        nPoints = nPointsDirect;
        }
    else
        {
        points  = &pointVector.front ();
        nPoints = static_cast<int>(pointVector.size ());
        }

    return (SUCCESS == curve.InitAkima (points, nPoints) ? true : false);
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/10
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   CurveHandler::_GetCurveVector (ElementHandleCR eh, CurveVectorPtr& curves)
    {
    PointVector pointVector;
    DPoint3dP   pointsDirect = NULL;
    int         nPointsDirect = 0;

    LineStringUtil::GetLineStringTransformPoints (&pointVector, &pointsDirect, &nPointsDirect, const_cast <DgnElementP> (eh.GetElementCP ()), false);

    int         nPoints;
    DPoint3dP   points;

    if (pointsDirect)
        {
        points  = pointsDirect;
        nPoints = nPointsDirect;
        }
    else
        {
        points  = &pointVector.front ();
        nPoints = static_cast <int> (pointVector.size ());
        }

    if (0 == nPoints)
        return ERROR;

    curves = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
    curves->push_back (ICurvePrimitive::CreateAkimaCurve (&points[0], nPoints));

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/10
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   CurveHandler::_SetCurveVector (EditElementHandleR eeh, CurveVectorCR path)
    {
    if (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_AkimaCurve != path.HasSingleCurvePrimitive ())
        return ERROR;

    bvector<DPoint3d> const* points = path.front ()->GetAkimaCurveCP ();

    if (points->size () > MAX_VERTICES)
        return ERROR;

    EditElementHandle   newEeh;

    // NOTE: In case eeh is component use ReplaceElement, Create methods uses SetElementDescr...
    if (SUCCESS != CurveHandler::CreateCurveElement (newEeh, &eeh, &points->front (), points->size (), eeh.GetElementCP ()->Is3d(), *eeh.GetDgnModelP ()))
        return ERROR;

    eeh.ReplaceElement (newEeh.GetElementCP ());

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Brien.Bastings                  07/05
+---------------+---------------+---------------+---------------+---------------+------*/
void            CurveHandler::_GetTypeName
(
WStringR        descr,
UInt32          desiredLength
)
    {
    descr.assign (DgnHandlersMessage::GetStringW(DgnHandlersMessage::IDS_TYPENAMES_CURVE_ELM));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/03
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       CurveHandler::_OnTransform
(
EditElementHandleR eeh,
TransformInfoCR trans
)
    {
    if (SUCCESS != T_Super::_OnTransform (eeh, trans))
        return ERROR;

    BeAssert (CURVE_ELM == eeh.GetLegacyType());

    return LineStringBaseHandler::TransformLineString (eeh, trans, false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/03
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       CurveHandler::_OnFenceStretch
(
EditElementHandleR  elemHandle,
TransformInfoCR     transform,
FenceParamsP        fp,
FenceStretchFlags   options
)
    {
    BeAssert (CURVE_ELM == elemHandle.GetLegacyType());

    return LineStringUtil::FenceStretch (elemHandle, transform, fp, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  10/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       CurveHandler::_OnFenceClip
(
ElementAgendaP  inside,
ElementAgendaP  outside,
ElementHandleCR elemHandle,
FenceParamsP    fp,
FenceClipFlags  options
)
    {
    fp->ParseAcceptedElement (inside, outside, elemHandle);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    john.gooding                    06/2009
+---------------+---------------+---------------+---------------+---------------+------*/
bool            CurveHandler::_IsSupportedOperation (ElementHandleCP eh, SupportOperation stype)
    {
    switch (stype)
        {
        case SupportOperation::LineStyle:
            return true;

        default:
            return T_Super::_IsSupportedOperation (eh, stype);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/12
+---------------+---------------+---------------+---------------+---------------+------*/
void            CurveHandler::_Draw (ElementHandleCR thisElm, ViewContextR context)
    {
    GeomRepresentations info = (GeomRepresentations) context.GetDisplayInfo (IsRenderable (thisElm));

    context.DrawCurveVector (thisElm, *this, info, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/08
+---------------+---------------+---------------+---------------+---------------+------*/
void            CurveHandler::_OnConvertTo3d (EditElementHandleR eeh, double elevation)
    {
    DgnV8ElementBlank   elm;

    eeh.GetElementCP ()->CopyTo (elm);

    int         numverts = elm.ToLine_String_2d().numverts;

    elm.SetSizeWordsNoAttributes((offsetof (Line_String_3d, vertice) + numverts * sizeof (DPoint3d)) / 2);
    elm.ToLine_String_3dR().numverts = numverts;
    DataConvert::Points2dTo3d (elm.ToLine_String_3dR().vertice, eeh.GetElementCP ()->ToLine_String_2d().vertice, numverts, elevation);
    ElementUtil::CopyAttributes (&elm, eeh.GetElementCP ());

    eeh.ReplaceElement (&elm);

    T_Super::_OnConvertTo3d (eeh, elevation);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/08
+---------------+---------------+---------------+---------------+---------------+------*/
void            CurveHandler::_OnConvertTo2d (EditElementHandleR eeh, TransformCR flattenTrans, DVec3dCR flattenDir)
    {
    // Pre-transform to "flatten" element and known linkages...
    TransformInfo   tInfo (flattenTrans);

    eeh.GetHandler().ApplyTransform (eeh, tInfo);

    DgnV8ElementBlank   elm;

    eeh.GetElementCP ()->CopyTo (elm);

    int         numverts = elm.ToLine_String_3d().numverts;

    elm.SetSizeWordsNoAttributes((offsetof (Line_String_2d, vertice) + numverts * sizeof (DPoint2d)) / 2);
    elm.ToLine_String_2dR().numverts = numverts;

    DataConvert::Points3dTo2d (elm.ToLine_String_2dR().vertice, eeh.GetElementCP()->ToLine_String_3d().vertice, numverts);
    ElementUtil::CopyAttributes (&elm, eeh.GetElementCP ());

    eeh.ReplaceElement (&elm);

    T_Super::_OnConvertTo2d (eeh, flattenTrans, flattenDir);
    }

/*---------------------------------------------------------------------------------**//**
* @description Scales first/last hidden tangents of an Akima curve to the length of the
*       first/last nontrivial visible chord.
* @bsimethod                                                    DavidAssaf      04/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            CurveHandler::ResetHiddenPoints (EditElementHandleR eeh)
    {
    if (!eeh.IsValid ())
        return;

    DgnElementP  el = eeh.GetElementP ();
    int         numVert = el->ToLine_String_2d().numverts;

    if (numVert < 6)
        return;

    int         i;
    double      tol = mgds_fc_nearZero, tangentLength;

    if (el->Is3d())
        {
        DPoint3d    tangent[2], *pts = el->ToLine_String_3dR().vertice;

        tol += mgds_fc_epsilon * bsiDPoint3d_getLargestCoordinate (pts, numVert);

        // seek forward from first visible point for the first nontrivial chord
        for (i = 3; i < numVert - 2; i++)
            if (!bsiDPoint3d_pointEqualTolerance (&pts[2], &pts[i], tol))
                break;

        // found a nontrivial start chord
        if (i < numVert - 2)
            {
            tangentLength = bsiDPoint3d_distance (&pts[2], &pts[i]);

            bsiDPoint3d_computeNormal (&tangent[1], &pts[1], &pts[2]);
            bsiDPoint3d_computeNormal (&tangent[0], &pts[0], &pts[1]);

            bsiDPoint3d_addScaledDPoint3d (&pts[1], &pts[2], &tangent[1], tangentLength);
            bsiDPoint3d_addScaledDPoint3d (&pts[0], &pts[1], &tangent[0], tangentLength);
            }

        // seek backward from the last visible point for the last nontrivial chord
        for (i = numVert - 4; i > 1; i--)
            if (!bsiDPoint3d_pointEqualTolerance (&pts[numVert - 3], &pts[i], tol))
                break;

        // found a nontrivial end chord
        if (i > 1)
            {
            tangentLength = bsiDPoint3d_distance (&pts[numVert - 3], &pts[i]);

            bsiDPoint3d_computeNormal (&tangent[1], &pts[numVert - 2], &pts[numVert - 3]);
            bsiDPoint3d_computeNormal (&tangent[0], &pts[numVert - 1], &pts[numVert - 2]);

            bsiDPoint3d_addScaledDPoint3d (&pts[numVert - 2], &pts[numVert - 3], &tangent[1], tangentLength);
            bsiDPoint3d_addScaledDPoint3d (&pts[numVert - 1], &pts[numVert - 2], &tangent[0], tangentLength);
            }
        }
    else
        {
        DPoint2d    tangent[2], *pts = el->ToLine_String_2dR().vertice;

        tol += mgds_fc_epsilon * bsiDPoint2dArray_getLargestCoordinate (pts, numVert);

        // seek forward from first visible point for the first nontrivial chord
        for (i = 3; i < numVert - 2; i++)
            if (!bsiDPoint2d_pointEqualTolerance (&pts[2], &pts[i], tol))
                break;

        // found a nontrivial start chord
        if (i < numVert - 2)
            {
            tangentLength = bsiDPoint2d_distance (&pts[2], &pts[i]);

            bsiDPoint2d_computeNormal (&tangent[1], &pts[1], &pts[2]);
            bsiDPoint2d_computeNormal (&tangent[0], &pts[0], &pts[1]);

            bsiDPoint2d_addScaledDPoint2d (&pts[1], &pts[2], &tangent[1], tangentLength);
            bsiDPoint2d_addScaledDPoint2d (&pts[0], &pts[1], &tangent[0], tangentLength);
            }

        // seek backward from the last visible point for the last nontrivial chord
        for (i = numVert - 4; i > 1; i--)
            if (!bsiDPoint2d_pointEqualTolerance (&pts[numVert - 3], &pts[i], tol))
                break;

        // found a nontrivial end chord
        if (i > 1)
            {
            tangentLength = bsiDPoint2d_distance (&pts[numVert - 3], &pts[i]);

            bsiDPoint2d_computeNormal (&tangent[1], &pts[numVert - 2], &pts[numVert - 3]);
            bsiDPoint2d_computeNormal (&tangent[0], &pts[numVert - 1], &pts[numVert - 2]);

            bsiDPoint2d_addScaledDPoint2d (&pts[numVert - 2], &pts[numVert - 3], &tangent[1], tangentLength);
            bsiDPoint2d_addScaledDPoint2d (&pts[numVert - 1], &pts[numVert - 2], &tangent[0], tangentLength);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley    01/86
+---------------+---------------+---------------+---------------+---------------+------*/
static void    get_transform (RotMatrixP pRMatrix, DPoint3d planepts[], size_t numpts)
    {
    DPoint3d    tmpPoints[3];

    tmpPoints[0] = planepts[0];
    tmpPoints[1] = planepts[1];

    size_t i;
    for (i=2; i < numpts; i++)
        {
        tmpPoints[2] = planepts[i];

        if (!LegacyMath::Vec::Colinear (tmpPoints))
            break;
        }

    // if all are linear, any point will be planar, give up
    if (i == numpts)
        pRMatrix->initIdentity ();
    else // get transformation matrix for plane
        pRMatrix->InitRotationFromOriginXY (planepts[0], planepts[1], planepts[i]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley    01/86
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   CurveHandler::GetStartTangentPoints (DPoint3d startpts[2], DPoint3d vert[], size_t numVerts)
    {
    RotMatrix   rMatrix;
    double      x[3], y[3], z[3];
    size_t      i, j;
    DPoint3d    *rvertP;

    rvertP = static_cast<DPoint3d *>(_alloca (numVerts * sizeof(DPoint3d)));

    get_transform (&rMatrix, vert, numVerts);

    for (i=0; i<numVerts; i++)
        {
        rvertP[i] = vert[i];
        rMatrix.MultiplyTranspose(rvertP[i]);
        }

    /* take the endpoint as the first of the three unique points */
    x[0] = rvertP[0].x;
    y[0] = rvertP[0].y;
    z[0] = rvertP[0].z;

    /* search through the array of verts to find two more unique points */
    for (i=1, j=0; i<numVerts; i++)
        {
        int unique_found = true;

        for (size_t k=0; k<=j; k++)
            if (LegacyMath::DUorEqual (rvertP[i].x, x[k]) && LegacyMath::DUorEqual (rvertP[i].y, y[k]))
                unique_found = false;

        if (unique_found)
            {
            j++;
            x[j] = rvertP[i].x;
            y[j] = rvertP[i].y;
            if (j==2) break;
            }
        }

    if (j<1)
        return ERROR; /* there are not 2 unique points*/

    startpts[1].x = 2.0 * x[0] - x[1];
    startpts[1].y = 2.0 * y[0] - y[1];
    startpts[0].x = 3.0 * x[0] - 2.0 * x[1];
    startpts[0].y = 3.0 * y[0] - 2.0 * y[1];
    startpts[0].z = startpts[1].z = rvertP[0].z;

    rMatrix.Multiply(startpts, startpts,  2);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley    01/86
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   CurveHandler::GetEndTangentPoints (DPoint3d endpts[2], DPoint3d vertice[], size_t numVerts)
    {
    BentleyStatus   status;
    DPoint3d        *tmpP, reverse_endpts[2];

    tmpP = static_cast<DPoint3d *>(_alloca (numVerts * sizeof(DPoint3d)));

    for (size_t i=0, j=numVerts-1; i<numVerts; i++, j--)
        tmpP[i] = vertice[j];

    status = CurveHandler::GetStartTangentPoints (reverse_endpts, tmpP, numVerts);

    endpts[0] = reverse_endpts[1];
    endpts[1] = reverse_endpts[0];

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    10/06
+---------------+---------------+---------------+---------------+---------------+------*/
ReprojectStatus CurveHandler::_OnGeoCoordinateReprojection (EditElementHandleR source, IGeoCoordinateReprojectionHelper& reprojectionHelper, bool inChain)
    {
    IGeoCoordinateReprojectionSettingsP settings = reprojectionHelper.GetSettings();
    if (reprojectionHelper.ShouldStroke (source, settings->StrokeCurves()))
        return GeoCoordinateReprojectionStrokeElement (source, reprojectionHelper, inChain);
    else
        {
        ReprojectStatus status      = REPROJECT_Success;
        DgnElementP      el          = source.GetElementP();
        bool            is3d        = Is3dElem (el);
        int             numPoints   = is3d ? el->ToLine_String_3d().numverts : el->ToLine_String_2d().numverts;

#if defined (INSERT_CURVE_POINTS)
        // NOTE: I tried this PostStrokeLinear code in here, but it is not an improvement. Better accuracy is obtained
        //       by stroking than by inserting more points. That's because we insert points assuming that the points are joined
        //       by a straight line segments, but they aren't - it's a curve. To add more points correctly, we'd need a way of
        //       calculating new points along the original curve (perhaps the _GetPointAtParameter method?) and inserting those.
        if (settings->PostStrokeLinear() && (numPoints > 5))
            {
            DPoint3dP   points;
            int         numOutPoints;
            int         iterations = 0;
            double      tolerance = reprojectionHelper.GetStrokeToleranceDestUors();

            for (;;)
                {
                // with our Akima curves, there are two points at the beginning and two at the end that are not displayed. We don't want to involve insert more
                // detail points in those two ares, so we start at point 2 and go to point n-2. (so if there are 6 points, we only want detail inserted between points 2 and 3.
                // We also never allow a complex to be created - we limit it to one curve with MAX_VERTICES or less. If the tolerance results in more points, keep doubling
                // the tolerance until it's under 5000.

                if (is3d)
                    status = reprojectionHelper.ReprojectPointsMoreDetail (&points, &numOutPoints, el->ToLine_String_3d().vertice, numPoints, 2, tolerance);
                else
                    status = reprojectionHelper.ReprojectPointsMoreDetail2D (&points, &numOutPoints, el->ToLine_String_2d().vertice, numPoints, 2, tolerance);

                if (SUCCESS != status)
                    break;

                if (numOutPoints < MAX_VERTICES)
                    break;

                // if it take more than 10 tries, forget it and just reproject the points.
                if (++iterations > 10)
                    break;

                tolerance = tolerance * 2.0;
                }

            if (SUCCESS == status && numOutPoints < MAX_VERTICES)
                {
                EditElementHandle  tmpEeh;

                // Can source be a complex complex component? If so, need to replace, passing source as output to create won't hook new edP up as it uses set...
                if (SUCCESS != CurveHandler::CreateCurveElement (tmpEeh, &source, points, numOutPoints, source.GetElementP()->Is3d(), source.GetDgnModelP()))
                    return ERROR;

                source.ReplaceElementDescr (tmpEeh.ExtractElementDescr ());

                return SUCCESS;
                }
            }
#endif

        if (is3d)
            status = reprojectionHelper.ReprojectPoints (el->ToLine_String_3dR().vertice, NULL, NULL, el->ToLine_String_3d().vertice, numPoints);
        else
            status = reprojectionHelper.ReprojectPoints2D (el->ToLine_String_2dR().vertice, NULL, NULL, el->ToLine_String_2d().vertice, numPoints);

        return status;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/2009
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   CurveHandler::CreateCurveElement
(
EditElementHandleR  eeh,
ElementHandleCP     templateEh,
DPoint3dCP          points,
size_t              numVerts,
bool                is3d,
DgnModelR        modelRef
)
    {
    if (numVerts > MAX_VERTICES)
        numVerts = MAX_VERTICES;
    else if (numVerts < 6) // Requires minimum of 6 points!
        return ERROR;

    size_t elmSize;

    if (is3d)
        elmSize = sizeof (Line_String_3d) + (numVerts-1) * sizeof (DPoint3d);
    else
        elmSize = sizeof (Line_String_2d) + (numVerts-1) * sizeof (DPoint2d);

    DgnElementCP in = (templateEh ? templateEh->GetElementCP () : NULL);
    DgnV8ElementBlank   out;

    if (in)
        {
        in->CopyTo (out);
        ElementUtil::SetRequiredFields (out, CURVE_ELM, LevelId(in->GetLevel()), false, (ElementUtil::ElemDim) is3d);
        }
    else
        {
        memset (&out, 0, elmSize);
        ElementUtil::SetRequiredFields (out, CURVE_ELM, LevelId(LEVEL_DEFAULT_LEVEL_ID), false, (ElementUtil::ElemDim) is3d);
        }

    out.ToLine_String_2dR().numverts = static_cast<UInt32>(numVerts);

    if (is3d)
        ElementUtil::PackLineWords3d (out.ToLine_String_2dR().vertice, points, numVerts);
    else
        ElementUtil::PackLineWords2d (out.ToLine_String_2dR().vertice, points, numVerts);

    out.SetSizeWordsNoAttributes(static_cast<UInt32>(elmSize/2));
    ElementUtil::CopyAttributes (&out, in);

    eeh.SetElementDescr (new MSElementDescr(out, modelRef), false);
    CurveHandler::ResetHiddenPoints (eeh);

    return eeh.GetDisplayHandler ()->ValidateElementRange (eeh);
    }

