/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/bspline/bsp_offset.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include "mxspline.h"
#include <Geom/MstnOnly/BspPrivateApi.h>
#include "msbsplinemaster.h"
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Peter.Yu                        03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
Public StatusInt bspcurv_interpolatedOffsetXY
(
MSBsplineCurveR offsetCurve, 
MSBsplineCurveCR sourceCurve, 
double offset0, 
double offset1, 
int numPoints
)
    {
    int i, status;
    double minKnot, maxKnot, d, dOffset, curvature, torsion, radiansA = 0, radiusA = 0, radiansB = 0, radiusB = 0;
    DPoint3d point;
    DVec3d   frame[3], xVec, normal0, normal1, tangent;
    bvector<DPoint3d> points;
    
    mdlBspline_getParameterRange (&minKnot, &maxKnot, &sourceCurve);
    d = (maxKnot - minKnot)/(numPoints - 1);
    dOffset = (offset1 - offset0)/(maxKnot - minKnot);
    xVec.Init ( 1.0, 0.0, 0.0);

    for (i=0; i<numPoints; i++)
        {
        if (SUCCESS != (status = sourceCurve.GetFrenetFrame (frame, point, curvature, torsion, minKnot + i*d)))
            return status;

        if (i == 0)
            {
            sourceCurve.FractionToPoint (point, tangent, minKnot + i*d);
            tangent.Scale (1.0+curvature*offset0);
            normal0 = frame[1];
            tangent.SumOf (tangent, normal0, -dOffset);
            radiansA = xVec.AngleToXY (tangent);
            radiusA = 1.0/curvature + offset0;
            }
        
        //reverse normal if it flips to the opposite direction at an inflection point.
        normal1 = frame[1];
        if (normal0.DotProduct (normal1) < 0)
            normal1.Negate (normal1);

        if (i == numPoints-1)
            {
            sourceCurve.FractionToPoint (point, tangent, minKnot + i*d);
            tangent.Scale (1.0+curvature*offset1);
            tangent.SumOf (tangent, normal1, -dOffset);
            radiansB = xVec.AngleToXY (tangent);
            radiusB = 1.0/curvature + offset1;
            }

        point.SumOf (point, normal1, -(offset0 + i*d*dOffset));
        points.push_back (point);
        normal0 = normal1;
        }

    return bspcurv_interpolateXYWithBearingAndRadius (&offsetCurve, &points[0], (int)points.size (), radiansA, radiusA, radiansB, radiusB);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Peter.Yu                        03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
Public StatusInt GEOMDLLIMPEXP bspcurv_approximateOffsetXY
(
MSBsplineCurveR offsetCurve, 
MSBsplineCurveCR sourceCurve, 
double offset0, 
double offset1,
int numPoles,
double geomTol
)
    {
    bvector<DPoint3d> P;
    bvector<double> up, uq;
    if (SUCCESS != MSBsplineCurve::SampleG1CurveByPoints (P, up, uq, &sourceCurve, 2, geomTol, 0.01*geomTol))
        return ERROR;
    
    int    status;
    size_t numPoints = P.size ();
    double dOffset, curvature, torsion;
    DPoint3d point;
    DVec3d   frame[3], normal0, normal1, sTangent, eTangent;
    bvector<DPoint3d> points;
    
    dOffset = (offset1 - offset0)/(up[numPoints-1] - up[0]);

    for (size_t i=0; i<numPoints; i++)
        {
        if (SUCCESS != (status = sourceCurve.GetFrenetFrame (frame, point, curvature, torsion, uq[i])))
            return status;

         if (i == 0)
            {
            sourceCurve.FractionToPoint (point, sTangent, uq[i]);
            sTangent.Scale (1.0+curvature*offset0);
            normal0 = frame[1];
            sTangent.SumOf (sTangent, normal0, -dOffset);
            }
        
        //reverse normal if it flips to the opposite direction at an inflection point.
        normal1 = frame[1];
        if (normal0.DotProduct (normal1) < 0)
            normal1.Negate (normal1);

        if (i == numPoints-1)
            {
            sourceCurve.FractionToPoint (point, eTangent, uq[i]);
            eTangent.Scale (1.0+curvature*offset1);
            eTangent.SumOf (eTangent, normal1, -dOffset);
            }

        point.SumOf (point, normal1, -(offset0 + (uq[i] - uq[0])*dOffset));
        points.push_back (point);
        normal0 = normal1;
        }

    return MSBsplineCurve::WeightedLeastSquaresFit (&offsetCurve, points, uq, true, &sTangent, &eTangent, numPoles, sourceCurve.params.order);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Peter.Yu                        03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
Public StatusInt GEOMDLLIMPEXP bspcurv_offsetEllipseXY
(
MSBsplineCurveR offsetCurve, 
DEllipse3dCR ellipse, 
double offset0, 
double offset1,
int numPoints
)
    {
    int i;
    double delta = ellipse.sweep/(numPoints-1), dOffset = (offset1 - offset0)/ellipse.sweep, curvature, radiansA = 0, radiusA = 0, radiansB = 0, radiusB = 0;
    DPoint3d point;
    DVec3d dX, ddX, normal, xVec, tmpVec;
    bvector<DPoint3d> points;
    
    xVec.Init ( 1.0, 0.0, 0.0);

    for (i=0; i<numPoints; i++)
        {
#ifdef CALL_ELLIPSE_METHODS
        ellipse.Evaluate (point, dX, ddX, ellipse.start+i*delta);
#else
        double theta = ellipse.start + i * delta;
        double c = cos (theta);
        double s = sin (theta);
        point.SumOf (ellipse.center, ellipse.vector0, c, ellipse.vector90, s);
        dX.SumOf (ellipse.vector0, -s, ellipse.vector90, c);
        ddX.SumOf (ellipse.vector0, -c, ellipse.vector90, -s);
        //dX.scale (1.0 / ellipse.sweep);
        //ddX.scale (1.0 / (ellipse.sweep * ellipse.sweep));
#endif
        
        normal.UnitPerpendicularXY (dX);

        if (i == 0)
            {
            curvature = tmpVec.NormalizedCrossProduct (dX, ddX)/pow (dX.Magnitude (), 3);
            dX.Scale (1.0+curvature*offset0);
            dX.SumOf (dX, normal, -dOffset);
            radiansA = xVec.AngleToXY (dX);
            radiusA = 1.0/curvature + offset0;
            }

        if (i == numPoints-1)
            {
            curvature = tmpVec.NormalizedCrossProduct (dX, ddX)/pow (dX.Magnitude (), 3);
            dX.Scale (1.0+curvature*offset1);
            dX.SumOf (dX, normal, -dOffset);
            radiansB = xVec.AngleToXY (dX);
            radiusB = 1.0/curvature + offset1;
            }

        point.SumOf (point, normal, -(offset0+i*delta*dOffset));
        points.push_back (point);
        }

    return bspcurv_interpolateXYWithBearingAndRadius (&offsetCurve, &points[0], (int)points.size (), radiansA, radiusA, radiansB, radiusB);
    }

static StatusInt getOffSetXYFractions
(
bvector<double>& params, 
MSBsplineCurveCR curve, 
int numPerKnotSpan
)
    {
    int         i, j, status, nKnots, numDistinct, *knotMultiplicityP = NULL;
    double      *distinctKnotP = NULL;

    nKnots = curve.NumberAllocatedKnots ();
    double knotTolerance = bspknot_knotTolerance (&curve);

    if (NULL == (distinctKnotP = (double*)BSIBaseGeom::Malloc (nKnots * sizeof(double))) ||
        NULL == (knotMultiplicityP = (int*)BSIBaseGeom::Malloc (nKnots * sizeof(int))))
        return ERROR;

    // get start/end knots too
    if (SUCCESS == (status = bspknot_getKnotMultiplicity (distinctKnotP, knotMultiplicityP, &numDistinct, curve.knots,
                                                                curve.params.numPoles, curve.params.order, curve.params.closed,
                                                                knotTolerance)))
        {
        for (i = 0; i < numDistinct-1; i++)
            for (j=0; j<=numPerKnotSpan; j++)
                {
                double h =  j/double (numPerKnotSpan+1);
                params.push_back (curve.KnotToFraction (distinctKnotP[i]*(1.0-h) + distinctKnotP[i+1]*h));
                }
        params.push_back (curve.KnotToFraction (distinctKnotP[numDistinct-1]));
        }

    BSIBaseGeom::Free (distinctKnotP);
    BSIBaseGeom::Free (knotMultiplicityP);
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Peter.Yu                        04/2010
+---------------+---------------+---------------+---------------+---------------+------*/
Public StatusInt GEOMDLLIMPEXP bspcuv_interpolatedOffSetXYSubdivide
(
MSBsplineCurveR offsetCurve, 
MSBsplineCurveCR sourceCurve, 
double offset0, 
double offset1, 
int numPerKnotSpan
)
    {
    offsetCurve.Zero ();
    bvector<double> values;
    if (SUCCESS != getOffSetXYFractions (values, sourceCurve, numPerKnotSpan))
        return ERROR;
    int  status;
    double dOffset, curvature, torsion, radiansA = 0, radiusA = 0, radiansB = 0, radiusB = 0;
    DPoint3d point;
    DVec3d   frame[3], xVec, normal0, normal1, tangent;
    bvector<DPoint3d> points;
    size_t numPoints = values.size ();
    dOffset = (offset1 - offset0)/(values[numPoints-1] - values[0]);

    xVec.Init ( 1.0, 0.0, 0.0);

    for (size_t i=0; i<numPoints; i++)
        {
        if (SUCCESS != (status = sourceCurve.GetFrenetFrame (frame, point, curvature, torsion, values[i])))
            return status;

        if (i == 0)
            {
            sourceCurve.FractionToPoint (point, tangent, values[i]);
            tangent.Scale (1.0+curvature*offset0);
            normal0 = frame[1];
            tangent.SumOf (tangent, normal0, -dOffset);
            radiansA = xVec.AngleToXY (tangent);
            radiusA = 1.0/curvature + offset0;
            }
        
        //reverse normal if it flips to the opposite direction at an inflection point.
        normal1 = frame[1];
        if (normal0.DotProduct (normal1) < 0)
            normal1.Negate (normal1);

        if (i == numPoints-1)
            {
            sourceCurve.FractionToPoint (point, tangent, values[i]);
            tangent.Scale (1.0+curvature*offset1);
            tangent.SumOf (tangent, normal1, -dOffset);
            radiansB = xVec.AngleToXY (tangent);
            radiusB = 1.0/curvature + offset1;
            }

        point.SumOf (point, normal1, -(offset0 + (values[i] - values[0])*dOffset));
        points.push_back (point);
        normal0 = normal1;
        }

    return bspcurv_interpolateXYWithBearingAndRadius (&offsetCurve, &points[0], (int)points.size (), radiansA, radiusA, radiansB, radiusB);
    }

// ASSUME .. curve to offset is pretty smooth
// ASSUME .. at least as many strokes as poles
// increase count by (totalTurn / angleRadians)
size_t OffsetStrokeCount (MSBsplineCurveCR curve, double angleRadians)
    {
    double angleSum = 0.0;
    bvector<DPoint3d> poles;
    curve.GetUnWeightedPoles (poles);
    size_t n = poles.size ();
    if (angleRadians > 0.0)
        {
        for (size_t i = 0; i + 2 < poles.size (); i++)
            {
            DVec3d vector0 = poles[i+1] - poles[i];
            DVec3d vector1 = poles[i+2] - poles[i+1];
            angleSum += vector0.AngleTo (vector1);
            }
        double xn;
        DoubleOps::SafeDivide (xn, angleSum, angleRadians, (double)n);
        xn = ceil (xn);
        if (xn > (double)n)
            n = (size_t)xn;
        }
    return n;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Earlin.Lutz                  01/17
+---------------+---------------+---------------+---------------+---------------+------*/
Public MSBsplineCurvePtr GEOMDLLIMPEXP bspcuv_interpolatedOffSetXYGreville
(
MSBsplineCurveCR sourceCurve, 
double offset0, 
double offset1
)
    {
    bvector<double> grevilleKnots;
    static double s_angleRadians = 0.1;
    int order = sourceCurve.GetIntOrder ();
#ifdef UseFullTolerancingStrokeCount
    double fastLength = sourceCurve.PolygonLength ();
    static double s_chordFraction = 1.0e-3;
    static double s_lengthFraction = 0.25;
    size_t numStroke = sourceCurve.GetStrokeCount (s_chordFraction * fastLength, s_angleRadians, s_lengthFraction * fastLength);
#else
    size_t numStroke = OffsetStrokeCount (sourceCurve, s_angleRadians);
#endif
    static size_t s_minStroke = 6;
    static size_t s_maxStroke = 100;
    if (numStroke < s_minStroke)
        numStroke = s_minStroke;
    if (numStroke > s_maxStroke)
        numStroke = s_maxStroke;
    int interiorIntervals = (int)numStroke - 3;     // greville conjures extras .. take some away
    MSBsplineCurve::ComputeUniformKnotGrevilleAbscissa (grevilleKnots, interiorIntervals, order);
    bvector<double> grevilleDistanceFraction;
    double u0 = 0.0;
    grevilleDistanceFraction.push_back (0.0);
    double baseLength = sourceCurve.Length ();
  
    // Get points on the true curve at true arc lengths controlled by the greville fractions.
    // These will be the points for offset and interpolation.
    for (size_t i = 0; i + 2 < grevilleKnots.size (); i++)
        {
        double distance = (grevilleKnots[i+1] - grevilleKnots[i]) * baseLength;
        double distance1, u1;
        if (!sourceCurve.FractionAtSignedDistance (u0, distance, u1, distance1))
            return nullptr;
        grevilleDistanceFraction.push_back (u1);
        u0 = u1;
        }
    grevilleDistanceFraction.push_back (1.0);

    bvector<DPoint3d> points;
    bvector<DVec3d> tangents;
    for (double g : grevilleDistanceFraction)
        {
        DPoint3d xyz;
        DVec3d dXYZ;
        double f = sourceCurve.KnotToFraction (g);
        sourceCurve.FractionToPoint (xyz, dXYZ, f);
        double offset = DoubleOps::Interpolate (offset0, f, offset1);
        DVec3d unitPerp;
        if (!unitPerp.UnitPerpendicularXY (dXYZ))
            return nullptr;
        points.push_back (xyz - offset * unitPerp);
        }
    int newCurveOrder = sourceCurve.GetIntOrder ();
    MSBsplineCurvePtr newCurve  = nullptr;
    if (newCurveOrder == 2)
        {
        newCurve = MSBsplineCurve::CreateFromPolesAndOrder (points, nullptr, nullptr, 2, false, true);
        }
    else
        newCurve = MSBsplineCurve::CreateFromInterpolationAtBasisFunctionPeaks (points, newCurveOrder, 0);
    return newCurve;
    }

MSBsplineCurvePtr MSBsplineCurve::CreateCopyOffsetXY (double offset0, double offset1, CurveOffsetOptionsCR options) const
    {
    int method = options.GetBCurveMethod ();
    if (method == 1)
        return bspcuv_interpolatedOffSetXYGreville (*this, offset0, offset1);


    static int s_maxPerKnot = 20;
    int order = GetIntOrder ();
    int numPerKnot = options.GetBCurvePointsPerKnot ();
    if (numPerKnot < 0)
        {
        numPerKnot = order;
        if (order <= 8)
            numPerKnot += order;
        else
            numPerKnot += 5;
        }
    if (numPerKnot > s_maxPerKnot)
        numPerKnot = s_maxPerKnot;

    MSBsplineCurvePtr offset = MSBsplineCurve::CreatePtr ();
    if (SUCCESS == bspcuv_interpolatedOffSetXYSubdivide (*offset, *this, offset0, offset1, numPerKnot))
        return offset;
    return NULL;
    }
#ifdef compile_bspcurv_offset
//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
//                CLASSIC bsprcurv OFFSET FUNCTION
//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BFP             11/91
+---------------+---------------+---------------+---------------+---------------+------*/
static int     bsprcurv_offsetWithSingleBezier
(
MSBsplineCurve  *offset,               /* <= offset curve */
double          *error,                /* <= error distance */
MSBsplineCurve  *curve,                /* => original curve */
double          distance,
int             continuity,            /* => geometric continuity desired */
RotMatrix       *rotMatrix             /* => of view, or NULL */
)
    {
    int         i, numPoints, status=SUCCESS, left, last;
    double      u, incr, matrix[2][2], rhs[2], bfuncs[4], totalError, dot;
    DPoint3d    tmp0, tmp1, delV0, delVn, frenet[3], di, delV0B, delVnB,
                *pP, *endP;
    DVec3d      zVec;

    memset (offset, 0, sizeof(*offset));
    memset (matrix, 0, sizeof(matrix));
    memset (rhs, 0, sizeof(rhs));
    /* Always make the offset order 4 with 4 knots at start and end .... */
    offset->params.numPoles = offset->params.order = 4;
    numPoints = curve->params.order > 3 ? 2 * curve->params.order : 6;

    /* Allocate memory */
    if (SUCCESS != (status = bspcurv_allocateCurve (offset)))
        return status;
    memset (offset->poles, 0, offset->params.numPoles * sizeof(DPoint3d));
    offset->knots[0] = offset->knots[1] =
    offset->knots[2] = offset->knots[3] = 0.0;
    offset->knots[4] = offset->knots[5] =
    offset->knots[6] = offset->knots[7] = 1.0;

    ScopedArray <DPoint3d>pointsArray (numPoints);    DPoint3d *points     = pointsArray.GetData ();

    last = curve->params.numPoles - 1;
    if (curve->rational)
        {
        tmp0.Scale (*(curve->poles), 1.0/curve->weights[0]);
        tmp1.Scale (*(curve->poles+1), 1.0/curve->weights[1]);
        delV0.DifferenceOf (tmp1, tmp0);
        offset->poles[0] = tmp0;
        tmp0.Scale (*(curve->poles+last-1), 1.0/curve->weights[last-1]);
        tmp1.Scale (*(curve->poles+last), 1.0/curve->weights[last]);
        delVn.DifferenceOf (tmp1, tmp0);
        offset->poles[3] = tmp1;
        }
    else
        {
        delV0.DifferenceOf (*(curve->poles+1), *(curve->poles));
        delVn.DifferenceOf (*(curve->poles+last), *(curve->poles+last-1));
        offset->poles[0] = curve->poles[0];
        offset->poles[3] = curve->poles[last];
        }
    
    if (delV0.Magnitude () < bsiTrig_smallAngle () * bspcurv_polygonLength (curve))
        bspcurv_evaluateCurvePoint (&tmp1, &delV0, curve, fc_epsilon);
    if (delVn.Magnitude () < bsiTrig_smallAngle () * bspcurv_polygonLength (curve))
        bspcurv_evaluateCurvePoint (&tmp1, &delVn, curve, 1.0 - fc_epsilon);

    bsiRotMatrix_getRow ( rotMatrix, &zVec,  2);
    bspcurv_frenetFrame (frenet, &tmp0, NULL, NULL, curve, 0.0, NULL);
    if (rotMatrix)
        frenet[1].CrossProduct (*frenet, zVec);
    offset->poles->SumOf (tmp0, frenet[1], distance);

    bspcurv_frenetFrame (frenet, &tmp0, NULL, NULL, curve, 1.0, NULL);
    if (rotMatrix)
        frenet[1].CrossProduct (*frenet, zVec);
    (offset->poles+3)->SumOf (tmp0, frenet[1], distance);

    /* To construct a cubic (4 point) Bezier approximating the offset ...
         First and last points are exact offsets of the base curve first and last.
         End tangents are parallel to base curve end tangents, i.e. vector from offset point0 to
            offset point 1 is some multiple of the base start tangent, and offset point 2
            is offset point 3 plus a multiple of the base end tangent.
        Treat the two multipliers as unknowns.
        At parameter u of the original curve, compute the exact offset point.   The (spatial)
            vector from this exact offset point to the offset bezier at the same parameter
            is linear in both of the multipliers.
        The square of this distance is a quadratic form in the two multipliers.
        Summing squared distances over various sample points and taking partials wrt the multipliers
            gives a linear least squares system.   Solve for the multipliers which minimize the
            sum of squared distances.
        Let Q0 and Q3 be start and end points (exact) of the offset.
        Let U0 and U3 be the known tangent directions at start and end.
        Then Q1 and Q2 are Q0+alpha*U0 and Q3+beta*U3
        At some parameter along the offest curve the blending function values are b0,b1,b2,b3 and
        the point on the approxmiate offset curve is
            B = b0*Q0 + b1*Q1 + b2*Q2 + b3*Q3
              = b0*Q0 + b1*(Q0+alpha*U0) + b2*(Q3+beta*U3) + b3*Q3
              = (b0+b1)Q0 + (b2+b3)Q3 + b1*alpha*U0 + b2*beta*U3
        If A is the true offset point from same parameter on base curve, B-A is the error vector.
        Sum (B-A)dot(B-A) over sample points, differentiate wrt alpha and beta, and you have
            a least squares system for best alpha, beta.
    */

    /* Construct system of equations to be solved */
    incr = 1.0 / (numPoints - 1);
    for (i=0, u=0.0; i < numPoints; i++, u += incr)
        {
        /* Calculate blending functions */
        bsputil_computeBlendingFunctions (bfuncs, NULL, &left, offset->knots, u, offset->params.numPoles,
                                      offset->params.order, offset->params.closed);

        /* Add to equations */
        bspcurv_frenetFrame (frenet, &tmp0, NULL, NULL, curve, u, NULL);
        if (rotMatrix)
            frenet[1].CrossProduct (*frenet, zVec);
        di.SumOf (tmp0, frenet[1], distance);
        /* Exact offset ... */
        points[i] = di;

        di.SumOf (di, *(offset->poles), -(bfuncs[0] + bfuncs[1]));
        di.SumOf (di, *(offset->poles+3), -(bfuncs[2] + bfuncs[3]));
        delV0B.Scale (delV0, bfuncs[1]);
        delVnB.Scale (delVn, bfuncs[2]);

        rhs[0] += di.DotProduct (delV0B);
        rhs[1] += di.DotProduct (delVnB);

        matrix[0][0] += delV0B.DotProduct (delV0B);
        dot = delV0B.DotProduct (delVnB);
        matrix[0][1] += dot;
        matrix[1][0] += dot;
        matrix[1][1] += delVnB.DotProduct (delVnB);
        }

    /* Solve system of equations */
    if (SUCCESS != (status = (bsiLinAlg_solveLinearGaussPartialPivot ((double*)matrix, 2, rhs, 1) ? SUCCESS : ERROR)))
        goto wrapup;

    (offset->poles+1)->SumOf (*(offset->poles), delV0, rhs[0]);
    (offset->poles+2)->SumOf (*(offset->poles+3), delVn, rhs[1]);

    /* Calculate error */
    for (pP=endP=points, endP+=numPoints, totalError=u=0.0; pP < endP; pP++, u += incr)
        {
        bspcurv_computeCurvePoint (&tmp0, NULL, NULL, u, offset->poles,
                offset->params.order, offset->params.numPoles, offset->knots,
                offset->weights, offset->rational, offset->params.closed);
        totalError += pP->Distance (tmp0);
        }

    *error = totalError / numPoints;
    offset->display.curveDisplay = true;

wrapup:
    if (status)
        bspcurv_freeCurve (offset);
    return status;
    }
#endif
/*----------------------------------------------------------------------+
|                                                                       |
|    Oriented Bounding Cylinder Routines                                |
|                                                                       |
+----------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brian.Peters    02/92
+---------------+---------------+---------------+---------------+---------------+------*/
Public  double   bound_cylCompute
(
BoundCyl        *cyl,
DPoint3d        *points,
int             numPoints
)
    {
    double      dot, dist;
    DPoint3d    *pP, *lastP, diff, cross;

    lastP = points + numPoints - 1;

    cyl->org = points[0];
    if ((cyl->length = cyl->dir.NormalizedDifference (*lastP, *points)) < fc_1em15)
        cyl->length = 1.0;
    cyl->radius = 0.0;

    for (pP = points + 1; pP < lastP; pP++)
        {
        diff.DifferenceOf (*pP, cyl->org);

        if ((dot = diff.DotProduct (cyl->dir)) < 0.0)
            {
            cyl->org.SumOf (cyl->org, cyl->dir, dot);
            cyl->length -= dot;
            }
        else if (dot > cyl->length)
            cyl->length = dot;

        cross.CrossProduct (diff, cyl->dir);
        if ((dist = cross.Magnitude ()) > cyl->radius)
            cyl->radius = dist;
        }
    return (cyl->length);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brian.Peters    02/92
+---------------+---------------+---------------+---------------+---------------+------*/
Public  double   bound_cylFromCurve
(
BoundCyl        *cyl,
MSBsplineCurve  *curve
)
    {
    if (curve->rational)
        {
        if (curve->params.numPoles > MAX_ORDER)
            {
            bsputil_unWeightPoles (curve->poles, curve->poles, curve->weights,
                                   curve->params.numPoles);
            bound_cylCompute (cyl, curve->poles, curve->params.numPoles);
            bsputil_weightPoles (curve->poles, curve->poles, curve->weights,
                                 curve->params.numPoles);
            return (cyl->length);
            }
        else
            {
            DPoint3d    local[MAX_ORDER];

            bsputil_unWeightPoles (local, curve->poles, curve->weights,
                                   curve->params.numPoles);
            return bound_cylCompute (cyl, local, curve->params.numPoles);
            }
        }
    else
        return bound_cylCompute (cyl, curve->poles, curve->params.numPoles);
    }

#ifdef compile_bspcurv_offset
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brian.Peters    12/91
+---------------+---------------+---------------+---------------+---------------+------*/
static int      bsprcurv_offsetStopBez
(
BezierInfo      *infoP,
MSBsplineCurve *curveP
)
    {
    int         code;
    BoundCyl    cyl;

    /* If bezier can be offset in one segment return true */
    code = infoP->off.error < infoP->off.tolerance;

    if (!code)
        {
        bound_cylFromCurve (&cyl, &infoP->off.curve);
        if (cyl.radius < infoP->off.tolerance)
            return true;

        bspcurv_freeCurve (&infoP->off.curve);
        }
    return code;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brian.Peters    12/91
+---------------+---------------+---------------+---------------+---------------+------*/
static int      bsprcurv_offsetGoBez
(
BezierInfo      *infoP,
MSBsplineCurve  *bezier,
int              id01
)
    {
    /* Offset using a single Bezier curve */
    memset (&infoP->off.curve, 0, sizeof(infoP->off.curve));
    bsprcurv_offsetWithSingleBezier (&infoP->off.curve, &infoP->off.error, bezier,
                                     *infoP->off.distance, infoP->off.continuity,
                                     infoP->off.matrix);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brian.Peters    12/91
+---------------+---------------+---------------+---------------+---------------+------*/
static int     bsprcurv_offsetSelectBez
(
BezierInfo      *infoP,
BezierInfo      *infoA,
BezierInfo      *infoB,
MSBsplineCurveP curveAP,
MSBsplineCurveP curveBP
)
    {
    int         status;
    status = bspcurv_appendCurves (&infoP->off.curve, &infoA->off.curve,
                                   &infoB->off.curve, true, true);
    bspcurv_freeCurve (&infoA->off.curve);
    bspcurv_freeCurve (&infoB->off.curve);
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brian.Peters    12/91
+---------------+---------------+---------------+---------------+---------------+------*/
static int     bsprcurv_offsetBezier
(
BezierInfo      *infoP,
MSBsplineCurve  *bezier
)
    {
    int         status;

    /* Offset using a single Bezier curve */
    memset (&infoP->off.curve, 0, sizeof(infoP->off.curve));
    if (SUCCESS != (status = bsprcurv_offsetWithSingleBezier (&infoP->off.curve,
                                      &infoP->off.error, bezier, *infoP->off.distance,
                                      infoP->off.continuity, infoP->off.matrix)))
        {
        bspcurv_freeCurve (&infoP->off.curve);
        return status;
        }

    return bspproc_processBezier (infoP, bezier,
                                  bsprcurv_offsetStopBez,
                                  NULLFUNC,
                                  bsprcurv_offsetGoBez,
                                  bsprcurv_offsetSelectBez);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brian.Peters    02/92
+---------------+---------------+---------------+---------------+---------------+------*/
static int     bsprcurv_offsetSort
(
int             *rank,
BezierInfo      *infoP,
int             *starts,
int             numSegments,
MSBsplineCurve  *curve
)
    {
    int         i, *iP;

    infoP->off.knots = curve->knots;
    for (i=0, iP=rank; i < numSegments; i++, iP++)
        *iP = i;
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
// Create a gap curve between the end of curve0 and start of curve1
* @bsimethod                                                    Lu.Han          11/93
+---------------+---------------+---------------+---------------+---------------+------*/
static int     bsprcurv_constructCuspCurve
(
MSBsplineCurve  *guspCurve,
MSBsplineCurve  *curve0,
MSBsplineCurve  *curve1,
int             cuspType
)
    {
    int         status = ERROR;
    DPoint3d    start, end, diff0, diff1;
    DPoint3d    approachPoint2;

    if (curve0->rational)
        bsputil_unWeightPoles (&start,
                 &curve0->poles[curve0->params.numPoles-1],
                 &curve0->weights[curve0->params.numPoles-1], 1);
    else
        start = curve0->poles[curve0->params.numPoles-1];

    if (curve1->rational)
        bsputil_unWeightPoles (&end, &curve1->poles[0], &curve1->weights[0], 1);
    else
        end = curve1->poles[0];

    /* If already connected, do not add the cusp */
    if (start.Distance (end) < fc_epsilon)
        return ERROR;

    switch (cuspType)
        {
        case OFFSET_CHAMFER_CUSP:   // straight line connection
            guspCurve->params.order = guspCurve->params.numPoles = 2;
            guspCurve->knots[0] = guspCurve->knots[1] = 0.0;
            guspCurve->knots[2] = guspCurve->knots[3] = 1.0;

            guspCurve->poles[0] = start;
            guspCurve->poles[1] = end;

            /* Do not need to set weights as end weights are always 1.0 */
            status = SUCCESS;
            break;

        case OFFSET_POINT_CUSP:     // straight lines from from the endponits to the closest approach of the tangents
            guspCurve->params.order = 2;
            guspCurve->params.numPoles = 3;
            guspCurve->knots[0] = guspCurve->knots[1] = 0.0;
            guspCurve->knots[2] = 0.5;
            guspCurve->knots[3] = guspCurve->knots[4] = 1.0;

            bsputil_polygonTangent (&diff0, curve0, 1);
            bsputil_polygonTangent (&diff1, curve1, 0);

            guspCurve->poles[0] = start;
            guspCurve->poles[2] = end;

            if (SUCCESS != (status = SegmentSegmentClosestApproachPoints (
                            guspCurve->poles[1], approachPoint2,
                            guspCurve->poles[0], diff0,
                            guspCurve->poles[2], diff1)))
                break;

            break;

        case OFFSET_PARABOLA_CUSP:  // parabola with start, closest approach of tangents, end as poles
            guspCurve->params.order = guspCurve->params.numPoles = 3;
            guspCurve->knots[0] = guspCurve->knots[1] = guspCurve->knots[2] = 0.0;
            guspCurve->knots[3] = guspCurve->knots[4] = guspCurve->knots[5] = 1.0;

            bsputil_polygonTangent (&diff0, curve0, 1);
            bsputil_polygonTangent (&diff1, curve1, 0);

            guspCurve->poles[0] = start;
            guspCurve->poles[2] = end;

            if (SUCCESS != (status = SegmentSegmentClosestApproachPoints (
                            guspCurve->poles[1], approachPoint2,
                            guspCurve->poles[0], diff0,
                            guspCurve->poles[2], diff1)))
                break;

            break;

        case OFFSET_ARC_CUSP:   // weighted bspline with the closest approach point as shoulder.
            guspCurve->rational = true;
            guspCurve->params.order = guspCurve->params.numPoles = 3;
            guspCurve->knots[0] = guspCurve->knots[1] = guspCurve->knots[2] = 0.0;
            guspCurve->knots[3] = guspCurve->knots[4] = guspCurve->knots[5] = 1.0;
            guspCurve->weights[0] = guspCurve->weights[2] = 1.0;

            bsputil_polygonTangent (&diff0, curve0, 1);
            bsputil_polygonTangent (&diff1, curve1, 0);

            guspCurve->poles[0] = start;
            guspCurve->poles[2] = end;

            if (SUCCESS != (status = SegmentSegmentClosestApproachPoints (
                            guspCurve->poles[1], approachPoint2,
                            guspCurve->poles[0], diff0,
                            guspCurve->poles[2], diff1)))
                break;

            diff1.NormalizedDifference (*(guspCurve->poles+2), *(guspCurve->poles));
            guspCurve->weights[1] = fabs (diff0.DotProduct (diff1));
            (guspCurve->poles+1)->Scale (*(guspCurve->poles+1), guspCurve->weights[1]);
            break;
        }

    return (status);
    }

/*---------------------------------------------------------------------------------**//**
// announce a new fragment of the offset curve.
// i.e.
//    append gap curve.
//    append new fragment.
* @bsimethod                                                    Brian.Peters    02/92
+---------------+---------------+---------------+---------------+---------------+------*/
static int     bsprcurv_appendOffsetSegment
(
MSBsplineCurve  *offset,
MSBsplineCurve  *segment,
BezierInfo      *infoP
)
    {
    int             status;
    double          wts[MAX_ARCPOLES], knots[MAX_ARCKNOTS];
    DPoint3d        poles[MAX_ARCPOLES];
    MSBsplineCurve  gap;

    if (infoP->off.cuspTreatment &&
        ! bspcurv_contiguousCurves (offset, segment))
        {
        memset (&gap, 0, sizeof(gap));
        gap.poles   = poles;
        gap.weights = wts;
        gap.knots   = knots;

        status = bsprcurv_constructCuspCurve (&gap, offset, segment, infoP->off.cuspTreatment);

        if (status == SUCCESS)
            bspcurv_appendCurves (offset, offset, &gap, true, true);
        }

    return bspcurv_appendCurves (offset, offset, segment, false, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brian.Peters    12/91
+---------------+---------------+---------------+---------------+---------------+------*/
static int     bsprcurv_offsetAssemble
(
BezierInfo      *infoP,
BezierInfo      *segP,
MSBsplineCurve  *bezierP,
int              index,                /* => index of segment */
int              start,                /* => knot offset of segment */
int              numberOfSegments
)
    {
    int             status;

    if (infoP->off.curve.params.numPoles)
        status = bsprcurv_appendOffsetSegment (&infoP->off.curve, &segP->off.curve, infoP);
    else
        status = bspcurv_copyCurve (&infoP->off.curve, &segP->off.curve);

#if defined (debug_offset)
    {
    ElementDescr    *edP=NULL;

    segP->off.curve.display.curveDisplay = true;
    bspcurv_createCurve (&edP, NULL, &segP->off.curve);
    mdlElmdscr_display (edP, 0, 0);
    mdlElmdscr_freeAll (&edP);
    }
#endif
    bspcurv_freeCurve (&segP->off.curve);
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* see J. Hoschek. "Spline approximation of offset curves." CAGD 5 (1988) 33-40.
* @bsimethod                                                    Brian.Peters    12/91
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bsprcurv_offset
(
MSBsplineCurve  *outCurve,             /* <= offset curve */
MSBsplineCurveCP inCurve,              /* => original curve */
double          distance,
int             cuspTreatment,
int             continuity,            /* => geometric continuity desired */
double          tolerance,
RotMatrix       *rotMatrix
)
    {
    int             status, numCusp, degree;
    double          *uCusp, *uP, *endU, knotTol, dist;
    double          wts[3], knots[6];
    RotMatrix       defaultRMatrix;
    DPoint3d        poles[3];
    BezierInfo      info;
    MSBsplineCurve  offset, gap;
    
    if (outCurve != inCurve)
        memset (outCurve, 0, sizeof (*outCurve));

    if (tolerance < fc_1em15)
        return ERROR;

    if (fabs (distance) < fc_1em15)
        {
        *outCurve = *inCurve;
        return SUCCESS;
        }

    /* Set tmp curve gap */
    memset (&gap, 0, sizeof(gap));
    gap.poles   = poles;
    gap.weights = wts;
    gap.knots   = knots;


    memset (&offset, 0, sizeof(offset));
    memset (&info.off.curve, 0, sizeof(info.off.curve));

    if (SUCCESS != (status = bspcurv_copyCurve (&offset, inCurve)))
        return status;

    if (cuspTreatment)
        bsprcurv_cuspPoints (NULL, &uCusp, &numCusp, &offset, tolerance, rotMatrix);
    else
        {
        uCusp = NULL;
        numCusp = 0;
        }

    dist = distance;
    info.off.distance       = &dist;
    info.off.tolerance      = tolerance;
    info.off.continuity     = continuity;
    info.off.cuspTreatment  = cuspTreatment;
    if (NULL == rotMatrix)
        {
        DVec3d  curveNormal;

        bspcurv_extractNormal (&curveNormal, NULL, NULL, inCurve);
        rotMatrix_orthogonalFromZRow (info.off.matrix = &defaultRMatrix, &curveNormal);
        }
    else
        {
        info.off.matrix     = rotMatrix;
        }

    knotTol = bspknot_knotTolerance (&offset);
    degree  = offset.params.order - 1;
    for (uP=endU=uCusp, endU += numCusp; uP < endU; uP++)
        if (SUCCESS != (status = bspknot_addKnot (&offset, *uP, knotTol, degree, false)))
            goto wrapup;
    if (SUCCESS != (status = bspproc_processBspline (&info, &offset,
                                                     bsprcurv_offsetSort,
                                                     NULLFUNC,
                                                     bsprcurv_offsetBezier,
                                                     bsprcurv_offsetAssemble,
                                                     NULLFUNC)))
        {
        bspcurv_freeCurve (&info.off.curve);
        }
    else
        {
        bspcurv_freeCurve (&offset);
        offset = info.off.curve;
        offset.display.curveDisplay = true;

        /* For order two closed curve, add arc to close it */
        if (inCurve->params.closed)
            {
            if (inCurve->params.order ==2)
                {
                if (SUCCESS == bsprcurv_constructCuspCurve (&gap, &offset, &offset, cuspTreatment))
                    bspcurv_appendCurves (&offset, &offset, &gap, true, true);
                }

            bspcurv_closeCurve (&offset, &offset);
            }

        if (outCurve == inCurve)
            bspcurv_freeCurve (outCurve);
        *outCurve = offset;
        }

wrapup:
    if (status)
        bspcurv_freeCurve (&offset);
    if (uCusp)
        BSIBaseGeom::Free (uCusp);
    return status;
    }
#endif




END_BENTLEY_GEOMETRY_NAMESPACE
