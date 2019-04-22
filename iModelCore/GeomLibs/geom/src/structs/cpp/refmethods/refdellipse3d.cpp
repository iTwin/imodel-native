/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

static double s_lineUnitCircleIntersectionTolerance = 1.0e-8;

#define FIX_MIN(value, min)          if (value < min) min = value
#define FIX_MAX(value, max)          if (value > max) max = value

// distanceSquared needed as plain functions in arg lists . . .
/*-----------------------------------------------------------------*//**
 @description Computes the squared distance between two points.
 @param pPoint1 => first point
 @param pPoint2 => second point
 @return squared distance between the points
@group "DPoint3d Distance"
 @bsimethod                                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDPoint3d_distanceSquared


(
DPoint3dCP pPoint1,
DPoint3dCP pPoint2
)
    {
    double      xdist, ydist, zdist;

    xdist = (pPoint2->x - pPoint1->x);
    ydist = (pPoint2->y - pPoint1->y);
    zdist = (pPoint2->z - pPoint1->z);

    return (xdist*xdist + ydist*ydist + zdist*zdist);
    }

/*-----------------------------------------------------------------*//**
 @description Computes the squared distance between two points, using only the xy parts.
 @param pPoint1 => first point
 @param pPoint2 => second point
 @return squared distance between the XY projections of the two points (i.e. any z difference is ignored)
@group "DPoint3d Distance"
 @bsimethod                                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDPoint3d_distanceSquaredXY


(
DPoint3dCP pPoint1,
DPoint3dCP pPoint2
)
    {
    double      xdist, ydist;

    xdist = pPoint2->x - pPoint1->x;
    ydist = pPoint2->y - pPoint1->y;

    return (xdist*xdist + ydist*ydist);
    }

/*----------------------------------------------------------------------+
|FUNC           bsiEllipse_componentRange       RK      06/96           |
| Conditional update range of one component of an ellipse.              |
|NORET                                                                  |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void    bsiEllipse_componentRange

(
double*         minP,           /* <=> min coordiante of range box */
double*         maxP,           /* <=> max coordinate of range box */
double          centComponent,  /* => center component */
double          cosComponent,   /* => basis vector component multiplied by cos */
double          sinComponent,   /* => basis vector component multiplied by sin */
double          theta0,         /* => start angle */
double          sweep           /* => sweep angle. */
)
    {
    double dr, r, c, s, theta1, theta2, extremeValue;
    r = sqrt(cosComponent * cosComponent + sinComponent*sinComponent);
    if (r > 0.0)
        {
        c = cosComponent / r;
        s = sinComponent / r;
        theta1 = Angle::Atan2 (s,c);
        theta2 = theta1 + msGeomConst_pi;
        dr = c * cosComponent + s * sinComponent;
        if (Angle::InSweepAllowPeriodShift (theta1, theta0, sweep))
            {
            extremeValue = centComponent + dr;
            FIX_MIN(extremeValue, *minP);
            FIX_MAX(extremeValue, *maxP);
            }

        if (Angle::InSweepAllowPeriodShift (theta2, theta0, sweep))
            {
            extremeValue =  centComponent - dr;
            FIX_MIN(extremeValue, *minP);
            FIX_MAX(extremeValue, *maxP);
            }
        }
    }

    /*-----------------------------------------------------------------*//**
* Return the intersection of a line with a cylinder, where the cylinder
* is defined by its (skewed) coordinate frame.  The cylinder is the
* set of all points that have x^2+y^2 = 1 in the skewed frame.
*
* @param pPointArray <= array of 0, 1, or 2 points
* @param pLineParam <= array of 0, 1, or 2 parameters along line
* @param pCylParam <= array of 0, 1, or 2 cylindrical coordinate
* @param pFrame => coordinate frame for the cylinder
* @param pStartPoint => line start point
* @param pEndPoint => line end point

* @return number of intersections
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
static int bsiCylinder_intersectLine

(
DPoint3d pointArray[2],
double   lineParam[2],
DPoint3d cylParam[2],
TransformCR frame,
DPoint3dCR startPoint,
DPoint3dCR endPoint
)
    {
    Transform inverseFrame;
    DPoint3d localStart, localEnd;
    double lambda0[2], lambda1[2];
    int numSolution = 0, i;
    double aa, bb, ab;

    if (inverseFrame.InverseOf (frame))
        {
        localStart = inverseFrame * startPoint;
        localEnd   = inverseFrame * endPoint;
        // EDL May 2018 Why was this using XYZ dot product?
        aa = localStart.DotProductXY (localStart) - 1.0; //localStart.DotProduct (localStart) - 1.0;
        ab = localStart.DotProductXY (localEnd) - 1.0; //localStart.DotProduct (localEnd) - 1.0;
        bb = localEnd.DotProductXY (localEnd) - 1.0; //localEnd.DotProduct (localEnd) - 1.0;
        numSolution = bsiMath_solveConvexQuadratic (lambda0, lambda1, aa, 2.0 * ab, bb);
        for (i = 0; i < numSolution; i++)
            {
            pointArray[i] = DPoint3d::FromSumOf (startPoint, lambda0[i], endPoint, lambda1[i]);
            lineParam[i] = lambda1[i];
            cylParam[i] = DPoint3d::FromSumOf (localStart, lambda0[i], localEnd, lambda1[i]);
            }
        }
    return numSolution;
    }

/*-----------------------------------------------------------------*//**
@param ellipse IN ellispe
@param localToGlobal OUT coordinate frame with origin at lower right of local range.
@param globalToLocal OUT transformation from world to local
@param range OUT ellipse range in the local coordinates.
@bsimethod                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
bool    bsiDEllipse3d_alignedRange
(
DEllipse3dCP ellipse,
TransformP   localToGlobal,
TransformP    globalToLocal,
DRange3dP     range
)
    {
    DEllipse3d mmEllipse, localEllipse;
    double r0, r1, start, sweep;
    DPoint3d center;
    RotMatrix axes;
    bool    stat = false;
    Transform worldToLocal, localToWorld, shiftedLocalToWorld;
    static double sSweepFactor = 1.00001;
    DVec3d diagonal;
    DPoint3d worldLowerLeft;
    DRange3d localRange;

    mmEllipse.InitMajorMinor (*ellipse);//  bsiDEllipse3d_initMajorMinor (&mmEllipse, ellipse);
    // Tentatively work in the centered major-minor axes ...
    mmEllipse.GetScaledRotMatrix (center, axes, r0, r1, start, sweep);

    if (fabs (sweep) < msGeomConst_pi * sSweepFactor)
        {
        // align to start-end chord ...
        DPoint3d startPoint, endPoint;
        ellipse->EvaluateEndPoints (startPoint, endPoint);
        DVec3d xVec, yVec, zVec;
        axes.GetColumn (zVec, 2);
        xVec.NormalizedDifference (endPoint, startPoint);
        yVec.NormalizedCrossProduct (zVec, xVec);
        localToWorld.InitFromOriginAndVectors (startPoint, xVec, yVec, zVec);
        }
    else
        {
        localToWorld.InitFrom (axes, center);
        }

    stat = worldToLocal.InverseOf(localToWorld);
    if (!stat)
        {
        worldToLocal.InitIdentity ();
        localEllipse = *ellipse;
        }

    worldToLocal.Multiply (localEllipse, *ellipse);
    localEllipse.GetRange (localRange);
    // Shift transform so origin is at lower left of range ...
    diagonal.DifferenceOf (localRange.high, localRange.low);
    localToWorld.Multiply(worldLowerLeft, localRange.low);

    shiftedLocalToWorld = localToWorld;
    shiftedLocalToWorld.SetTranslation(worldLowerLeft);

    if (NULL != range)
        {
        range->low.Zero ();
        range->high.Init (diagonal.x, diagonal.y, diagonal.z);
        }
    if (NULL != localToGlobal)
        *localToGlobal = shiftedLocalToWorld;

    if (NULL != globalToLocal)
        globalToLocal->InverseOf(*localToGlobal);

    return stat;
    }

/*-----------------------------------------------------------------*//**
@description Fill in ellipse data from data fields in DGN 3d ellipse element.
@param pEllipse OUT initialized DEllipse3d
@param pCenter IN center of ellipse.
@param pQuatWXYZ IN array of 4 doubles (ordered w,x,y,z) with quaternion for orthogonal frame.  If this is NULL,
           major and minor directions must be supplied as pDirection0 and pDirection90;
@param pDirectionX IN vector in the x axis direction.  This is scaled by rX. (It is NOT normalized before
                scaling.  In common use, it will be a unit vector.)
@param pDirectionY IN vector in the y axis direction.  This is scaled by rY. (It is NOT normalized before
                scaling.  In common use, it will be a unit vector.)
@param rX IN scale factor (usually a true distance) for x direction.
@param rY IN scale factor (usually a true distance) for y direction.
@param pStart IN optional start angle.  Defaults to zero
@param pSweep IN optional sweep angle.  Defaults to 2pi.
@group "DEllipse3d DGN Elements"
@bsimethod                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDEllipse3d_initFromDGNFields3d
(
DEllipse3dP     pEllipse,
DPoint3dCP      pCenter,
double const*   pQuatWXYZ,
DVec3dCP        pDirectionX,
DVec3dCP        pDirectionY,
double          rX,
double          rY,
double const*   pStart,
double const*   pSweep
)
    {
    DVec3d      vec0, vec90, zvec;

    if (NULL != pQuatWXYZ)
        {
        RotMatrix rot;
        rot.InitFromQuaternion (pQuatWXYZ);
        rot.GetRows(vec0, vec90, zvec);
        }
    else
        {
        vec0  = *pDirectionX;
        vec90 = *pDirectionY;
        }

    pEllipse->center = *pCenter;
    pEllipse->vector0.Scale (vec0, rX);
    pEllipse->vector90.Scale (vec90, rY);
    pEllipse->start = pStart ? *pStart : 0.0;
    pEllipse->sweep = pSweep ? *pSweep : msGeomConst_2pi;
    }


/*-----------------------------------------------------------------*//**
@description Fill in ellipse data from data fields in DGN 2d ellipse element.
@param pEllipse OUT initialized DEllipse3d
@param pCenter IN center of ellipse.
@param pXAngle IN optional angle from global x axis to local x axis.
@param pDirection0 IN optional vector form of ellipse x axis direction.
@param rX IN scale factor for ellipse x direction.
@param rY IN scale factor for ellipse y direction.
@param pStart IN optional start angle.  Defaults to zero
@param pSweep IN optional sweep angle.  Defaults to 2pi.
@param zDepth IN z value for ellipse.
@group "DEllipse3d DGN Elements"
@bsimethod                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDEllipse3d_initFromDGNFields2d
(
DEllipse3dP     pEllipse,
DPoint2dCP      pCenter,
double const*   pXAngle,
DVec2dCP        pDirection0,
double          rX,
double          rY,
double const*   pStart,
double const*   pSweep,
double          zDepth
)
    {
    DVec2d      vec0;

    if (pXAngle)
        {
        double theta = *pXAngle;
        vec0.x = cos (theta);
        vec0.y = sin (theta);
        }
    else if (pDirection0)
        {
        vec0 = *pDirection0;
        }
    else
        {
        vec0.Init (1,0);
        }

    pEllipse->center.Init (pCenter->x, pCenter->y, zDepth);
    pEllipse->vector0.Init (vec0.x * rX, vec0.y * rX, 0.0);
    pEllipse->vector90.Init (-vec0.y * rY, vec0.x * rY, 0.0);
    pEllipse->start = pStart ? *pStart : 0.0;
    pEllipse->sweep = pSweep ? *pSweep : msGeomConst_2pi;
    }

static double s_axisRatioTolerance = 1.0e-6;


/*-----------------------------------------------------------------*//**
@description Fill in ellipse data from data fields in DGN 3d ellipse element.
@param pEllipse IN initialized DEllipse3d
@param pCenter OUT center of ellipse.
@param pQuatWXYZ OUT quaternion for orthogonal frame.
            As per DGN convention, ordered WXYZ.
            If this is NULL,
           major and minor directions must be supplied as pDirection0 and pDirection90;
@param pDirectionX OUT unit vector in ellipse x direction.
@param pDirectionY OUT unit vector in ellipse y direction.
@param pRX OUT scale factor (usually a true distance) for x direction.
@param pRY OUT scale factor (usually a true distance) for y direction.
@param pStartAngle OUT optional start angle.  Defaults to zero
@param pSweepAngle OUT optional sweep angle.  Defaults to 2pi.
@group "DEllipse3d DGN Elements"
@bsimethod                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDEllipse3d_getDGNFields3d
(
DEllipse3dCP pEllipse,
DPoint3dP    pCenter,
double *    pQuatWXYZ,
DVec3dP     pDirectionX,
DVec3dP     pDirectionY,
double *    pRX,
double *    pRY,
double *    pStartAngle,
double *    pSweepAngle
)
    {
    DEllipse3d majorMinorEllipse;
    majorMinorEllipse.InitWithPerpendicularAxes (*pEllipse);

    DVec3d xAxis, yAxis, zAxis;
    double r0  = xAxis.Normalize (*((DVec3d const*) &majorMinorEllipse.vector0));
    double r90 = yAxis.Normalize (*((DVec3d const*) &majorMinorEllipse.vector90));

    if (r0 < s_axisRatioTolerance * r90)
        {
        // x axis is garbage, y is good ...
        yAxis.GetNormalizedTriad(zAxis, xAxis, yAxis);
        r0 = 0.0;
        }
    else if (r90 < s_axisRatioTolerance * r0)
        {
        // x axis is garbage, y is good ...
        xAxis.GetNormalizedTriad(yAxis, zAxis, xAxis);
        r90 = 0.0;
        }
    else
        {
        // The usual case.  x,y are perpendicular unit vectors ....
        zAxis.NormalizedCrossProduct (xAxis, yAxis);
        }

    RotMatrix matrix;
    matrix.InitFromColumnVectors (xAxis, yAxis, zAxis);
    matrix.SquareAndNormalizeColumns(matrix, 0, 1);

    if (pCenter)
        *pCenter = majorMinorEllipse.center;
    if (pQuatWXYZ)
        matrix.GetQuaternion (pQuatWXYZ, true);

    if (pDirectionX)
        *pDirectionX = xAxis;
    if (pDirectionY)
        *pDirectionY = yAxis;

    if (pRX)
        *pRX = r0;
    if (pRY)
        *pRY = r90;

    if (pStartAngle)
        *pStartAngle = majorMinorEllipse.start;
    if (pSweepAngle)
        *pSweepAngle = majorMinorEllipse.sweep;

    }


/*-----------------------------------------------------------------*//**
@param pEllipse OUT initialized DEllipse3d
@param pCenter IN center of ellipse.
@param pXAngle IN angle from global x axis to local x axis.
@param pDirection0 IN direction for x radius.
@param pRX IN scale factor for ellipse x direction.
@param pRY IN scale factor for ellipse y direction.
@param pStartAngle IN optional start angle.
@param pSweepAngle IN optional sweep angle.
@group "DEllipse3d DGN Elements"
@bsimethod                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDEllipse3d_getDGNFields2d
(
DEllipse3dCP    pEllipse,
DPoint2dP       pCenter,
double *        pXAngle,
DVec2dP         pDirection0,
double *        pRX,
double *        pRY,
double *        pStartAngle,
double *        pSweepAngle
)
    {
    DEllipse3d xyEllipse = *pEllipse;
    xyEllipse.center.z   = 0.0;
    xyEllipse.vector0.z  = 0.0;
    xyEllipse.vector90.z = 0.0;

    DEllipse3d xyMajorMinorEllipse;
    xyMajorMinorEllipse.InitWithPerpendicularAxes (xyEllipse);

    double r0  = xyMajorMinorEllipse.vector0.Magnitude ();
    double r90 = xyMajorMinorEllipse.vector90.Magnitude ();

    // Reverse left handed coordinate system
    if (xyMajorMinorEllipse.vector0.CrossProductXY (xyMajorMinorEllipse.vector90) < 0.0)
        {
        xyMajorMinorEllipse.vector90.Negate ();
        xyMajorMinorEllipse.start = -xyMajorMinorEllipse.start;
        xyMajorMinorEllipse.sweep = -xyMajorMinorEllipse.sweep;
        }


    // Use the longer axis for angle calculation
    double thetaX = r0 > r90 ? Angle::Atan2 (xyMajorMinorEllipse.vector0.y, xyMajorMinorEllipse.vector0.x)
                                : Angle::Atan2 (-xyMajorMinorEllipse.vector90.x, xyMajorMinorEllipse.vector90.y);

    if (pCenter)
        {
        pCenter->x = xyMajorMinorEllipse.center.x;
        pCenter->y = xyMajorMinorEllipse.center.y;
        }
    if (pXAngle)
        *pXAngle = thetaX;
    if (pDirection0)
        pDirection0->Init (cos (thetaX), sin(thetaX));
    if (pRX)
        *pRX = r0;
    if (pRY)
        *pRY = r90;
    if (pStartAngle)
        *pStartAngle = xyMajorMinorEllipse.start;
    if (pSweepAngle)
        *pSweepAngle = xyMajorMinorEllipse.sweep;
    }






/*---------------------------------------------------------------------------------**//**
@description Remove points not contained in the sweep ranges of (both of) two ellipses.
 @bsimethod                                                     EarlinLutz      01/99
+--------------------------------------------------------------------------------------*/
int filterDualSweeps

(
DPoint3dP pCartesianPoints,
DPoint3dP pEllipse0Coffs,
double        *pEllipse0Angle,
DPoint3dP pEllipse1Coffs,
double        *pEllipse1Angle,
DEllipse3dCP pEllipse0,
DEllipse3dCP pEllipse1,
DPoint3dCP pCartesianInPoint,
DPoint3dCP pEllipse0InCoffs,
DPoint3dCP pEllipse1InCoffs,
int           numUnBounded
)
    {
    int numBounded = 0;
    int i;
    double theta0, theta1;
    for (i = 0; i < numUnBounded ; i++)
        {
        theta0  = Angle::Atan2 (pEllipse0InCoffs[i].y, pEllipse0InCoffs[i].x);
        theta1  = Angle::Atan2 (pEllipse1InCoffs[i].y, pEllipse1InCoffs[i].x);

        if  (
                pEllipse0->IsAngleInSweep (theta0)
             && pEllipse1->IsAngleInSweep (theta1)
            )
            {
            if  (pCartesianPoints)
                pCartesianPoints[numBounded] = pCartesianInPoint[i];

            if  (pEllipse0Coffs)
                pEllipse0Coffs[numBounded]   = pEllipse0InCoffs[i];

            if  (pEllipse0Angle)
                pEllipse0Angle[numBounded]   = theta0;

            if  (pEllipse1Coffs)
                pEllipse1Coffs[numBounded]    = pEllipse1InCoffs[i];

            if  (pEllipse1Angle)
                pEllipse1Angle[numBounded]    = theta1;

            numBounded++;
            }
        }
    return numBounded;
    }



/*-----------------------------------------------------------------*//**
@description Find the intersections of xy projections of two ellipses defined by coordinate
 frames.   Both are unbounded.   Prefers inverse of first frame.
 @return the number of intersections.
 @bsimethod                                                     EarlinLutz      12/97
+----------------------------------------------------------------------*/
int intersectXYEllipseFrames
(
DPoint3dP pCartesianPoints,
DPoint3dP pEllipse0Params,
DPoint3dP pEllipse1Params,
TransformCP pFrame0,
TransformCP pFrame1
)
    {
    Transform inverseFrame;
    Transform localFrame1;
    double cosValue[4], sinValue[4], thetaValue[4];
    double c1, s1;
    double c0, s0;
    double ax, ux, vx;
    double ay, uy, vy;
    int numInt = 0;
    int i;

    if (inverseFrame.InverseOf (*pFrame0))
        {
        localFrame1.InitProduct (inverseFrame, *pFrame1);
        /* In the local coordinates of ellipse0, ellipse0 itself is a unit circle.
           Ellipse 1 is not.
        */
        
        ax = localFrame1.GetPointComponent (0);
        ux = localFrame1.GetFromMatrixByRowAndColumn (0, 0);
        vx = localFrame1.GetFromMatrixByRowAndColumn (0, 1);

        ay = localFrame1.GetPointComponent (1);
        uy = localFrame1.GetFromMatrixByRowAndColumn (1, 0);
        vy = localFrame1.GetFromMatrixByRowAndColumn (1, 1);
        RotMatrix B;
        B.InitFromRowValues
            (
             ux,  vx,  ax,
             uy,  vy,  ay,
            0.0, 0.0, 1.0   
            );

        bsiBezier_conicIntersectUnitCircle (cosValue, sinValue, thetaValue, &numInt,
                                            NULL, NULL, &B);

        for (i = 0; i < numInt; i++)
            {
            c1 = cosValue[i];
            s1 = sinValue[i];

            if  (pCartesianPoints)
                pCartesianPoints[i] = DPoint3d::FromProduct (*pFrame1, c1, s1, 0.0);

            if  (pEllipse0Params)
                {
                c0 = ax + ux * c1 + vx * s1;
                s0 = ay + uy * c1 + vy * s1;
                pEllipse0Params[i].Init (c0, s0, 0.0);
                }

            if  (pEllipse1Params)
                {
                pEllipse1Params[i].Init (c1, s1, 0.0);
                }
            }
        }
    return numInt;
    }


/*-----------------------------------------------------------------*//**
@description Compute area and swept angle as seen from given point.
@param pEllipse IN ellipse
@param pArea OUT swept area
@param pSweep OUT swept angle (in radians)
@param pPoint IN base point for sweep line.
@group "DEllipse3d Queries"
 @bsimethod                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDEllipse3d_xySweepProperties

(
DEllipse3dCP pEllipse,
double        *pArea,
double        *pSweep,
DPoint3dCP pPoint
)
    {
    Transform transform, inverse;
    DVec3d localVector, globalVector;
    DPoint3d centerToStart, centerToEnd, pointToStart, pointToEnd;
    DVec3d centerToMid, midToStart, midToEnd;
    double midCross;
    double c, s, c0, s0, c1, s1, cMid, sMid;
    double thetaMid;
    double theta0 = pEllipse->start;
    double theta1 = pEllipse->start + pEllipse->sweep;
    double angle;
    double localArea;
    bool    inside;

    double detJ = pEllipse->DeterminantJXY ();

    globalVector.DifferenceOf (*pPoint, pEllipse->center);
    pEllipse->GetXYLocalFrame (transform, inverse);

    localVector = inverse * globalVector;    // And we trust that uses only the matrix part of the multiply!!
    
    c = localVector.x;
    s = localVector.y;

    inside = c * c + s * s < 1.0;

    c0 = cos (theta0);
    s0 = sin (theta0);

    c1 = cos (theta1);
    s1 = sin (theta1);

    if (pArea)
        {
        localArea = 0.5 * ( pEllipse->sweep
                      - DoubleOps::DeterminantXYXY (c0, s0, c,  s)
                      - DoubleOps::DeterminantXYXY (c,  s,  c1, s1)
                        );
        *pArea = localArea * detJ;
        }

    if (pSweep)
        {
        centerToStart = pEllipse->vector0 * c0 + pEllipse->vector90 * s0;
        centerToEnd = pEllipse->vector0 * c1 + pEllipse->vector90 * s1;
        pointToStart.DifferenceOf (centerToStart, globalVector);
        pointToEnd.DifferenceOf (centerToEnd, globalVector);

        angle = Angle::Atan2 (
                        pointToStart.CrossProductXY (pointToEnd),
                        pointToStart.DotProductXY (pointToEnd)
                        );

        /* Imagine contracting the elliptical arc towards the chord between the start and end points.
            If the viewpoint is within the region swept by the contracting arc,
            we have to flip the angle by a period.
        */
        if (inside)
            {
            thetaMid = theta0 + 0.5 * pEllipse->sweep;

            cMid = cos (thetaMid);
            sMid = sin (thetaMid);
            centerToMid = pEllipse->vector0 * cMid + pEllipse->vector90 * sMid;

            midToStart.DifferenceOf (centerToStart, centerToMid);
            midToEnd.DifferenceOf (centerToEnd, centerToMid);
            midCross = midToStart.CrossProductXY (midToEnd);

            if (angle > 0.0)
                {
                if (midCross > 0.0)
                    angle = angle - msGeomConst_2pi;
                }
            else
                {
                if (midCross < 0.0)
                    angle = msGeomConst_2pi + angle;
                }
            }
        *pSweep = angle;
        }
    }


/*-----------------------------------------------------------------*//**
@description Find "intersections" of a DSegment3d and a DEllipse3d.
@remarks Curves in space can pass very close to
 each other without intersecting, so some logic must be applied to define intersection
 more cleanly. The logic applied is to compute the intersection of the line with
 the cylinder swept by the ellipse along its plane normal.

 @param pEllipse      => base ellipse for the cylinder.
 @param pPointArray   <= cartesian intersection points.
 @param pEllipseParams     <= array of coordinates relative to the instance
                            For each point, (xy) are the cosine and sine of the
                            ellipse parameter, (z) is z distance from the plane of
                            of the ellipse.
 @param pLineParams     <= array of parametric coordinates on the line.
 @param pSegment       => the line segment
 @return the number of intersections.
  @group "DEllipse3d Intersection"
 @bsimethod                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int bsiDEllipse3d_intersectSweptDSegment3d

(
DEllipse3dCP pEllipse,
DPoint3dP pPointArray,
DPoint3dP pEllipseParams,
double        *pLineParams,
DSegment3dCP pSegment
)
    {
    int             i, numUnbounded = 0;
    double          ellipseCondition, lambda0[2], lambda1[2];
    DSegment3d      localSegment;
    Transform ellipseFrame, ellipseInverse;
    static double   s_conditionTol = 1.0e-8;

    pEllipse->GetLocalFrame (ellipseFrame, ellipseInverse);
    ellipseCondition = RotMatrix::From (ellipseFrame).ConditionNumber ();

    if (ellipseCondition > s_conditionTol)
        {
        double aa, ab, bb;
        localSegment.InitProduct (ellipseInverse, *pSegment);//bsiDSegment3d_multiplyTransformDSegment3d (&localSegment, &ellipseInverse, pSegment);
        aa = localSegment.point[0].DotProductXY (*(&localSegment.point[0])) - 1.0;
        ab = localSegment.point[0].DotProductXY (*(&localSegment.point[1])) - 1.0;
        bb = localSegment.point[1].DotProductXY (*(&localSegment.point[1])) - 1.0;

        numUnbounded = bsiMath_solveConvexQuadratic (lambda0, lambda1, aa, 2.0 * ab, bb);

        for (i = 0; i < numUnbounded; i++)
            {
            if (pLineParams)
                pLineParams[i] = lambda1[i];

            if (pEllipseParams)
                pEllipseParams[i].SumOf (
                                        localSegment.point[0], lambda0[i],
                                        localSegment.point[1], lambda1[i]
                                        );
            if (pPointArray)
                pPointArray[i].SumOf (
                                     pSegment->point[0], lambda0[i],
                                     pSegment->point[1], lambda1[i]
                                     );
            }
        }
    return numUnbounded;
    }


/*-----------------------------------------------------------------*//**
@description Intersect an ellipse and a segment as described in ~mbsiDEllipse3d_intersectSweptDSegment3d, and
 filter out results not part of both ranges.

 @param pEllipse      => base ellipse for the cylinder.
 @param pPointArray   <= cartesian intersection points.
 @param pEllipseParams     <= array of coordinates relative to the instance
                            For each point, (xy) are the cosine and sine of the
                            ellipse parameter, (z) is z distance from the plane of
                            of the ellipse.
 @param pLineParams     <= array of parametric coordinates on the line.
 @param pSegment       => the line segment
 @return the number of intersections.
@group "DEllipse3d Intersection"
 @bsimethod                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int bsiDEllipse3d_intersectSweptDSegment3dBounded

(
DEllipse3dCP pEllipse,
DPoint3dP pPointArray,
DPoint3dP pEllipseParams,
double        *pLineParams,
DSegment3dCP pSegment
)
    {
    DPoint3d pointArray[2];
    double lineParam[2];
    DPoint3d ellipseParam[2];
    int numBounded = 0;
    int i;
    static double s_lineTol = 1.0e-12;
    int numUnbounded = bsiDEllipse3d_intersectSweptDSegment3d
                                (
                                pEllipse,
                                pointArray,
                                ellipseParam,
                                lineParam,
                                pSegment
                                );

    for (i = 0; i < numUnbounded; i++)
        {
        if (lineParam[i] >= - s_lineTol && lineParam[i] <= 1.0 + s_lineTol)
            {
            double theta = Angle::Atan2 (ellipseParam[i].y, ellipseParam[i].x);
            if (pEllipse->IsAngleInSweep (theta))
                {
                if (pPointArray)
                    pPointArray[numBounded] = pointArray[i];

                if (pLineParams)
                    pLineParams[numBounded] = lineParam[i];

                if (pEllipseParams)
                    pEllipseParams[numBounded] = ellipseParam[i];

                numBounded++;
                }
            }
        }

    return numBounded;
    }


/*-----------------------------------------------------------------*//**
@description Project a point onto the (unbounded) ellipse.
@remarks May return up to 4 points.
@param pEllipse IN ellipse to search
@param pCartesianPoints OUT array (allocated by caller) of points on the ellipse.
@param pEllipseAngle OUT array (allocated by caller) of ellipse angles.
@param pPoint IN space point
 @return the number of projection points
@group "DEllipse3d Closest Point"
 @bsimethod                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int bsiDEllipse3d_projectPoint

(
DEllipse3dCP pEllipse,
DPoint3dP pCartesianPoints,
double        *pEllipseAngle,
DPoint3dCP pPoint
)
    {
    int numProjection = 0;
    if (pEllipse->IsCircular ())
        {
        DPoint3d vectorA;
        double cc, ss, rc, rs, theta0, theta1, r;
        vectorA.DifferenceOf (*pPoint, pEllipse->center);
        rc = vectorA.DotProduct (pEllipse->vector0);
        rs = vectorA.DotProduct (pEllipse->vector90);
        r = sqrt (rc * rc + rs * rs);
        if (r != 0.0)
            {
            theta0 = Angle::Atan2 ( rs,  rc);
            theta1 = Angle::Atan2 (-rs, -rc);
            r = sqrt (rc * rc + rs * rs);

            cc =  rc / r;
            ss =  rs / r;

            numProjection = 2;
            if (pEllipseAngle)
                {
                pEllipseAngle[0] = theta0;
                pEllipseAngle[1] = theta1;
                }
            if (pCartesianPoints)
                {
                pCartesianPoints[0].SumOf(pEllipse->center, pEllipse->vector0, cc, pEllipse->vector90, ss);
                pCartesianPoints[1].SumOf(pEllipse->center, pEllipse->vector0, -cc, pEllipse->vector90, -ss);
                }
            }
        else
            {
            numProjection = 4;
            // the ellipse is confirmed as a circle, and the space point is
            // at its center, hence equidistant from all points.
            // Return the 4 quadrant points ...
            if (pEllipseAngle)
                {
                pEllipseAngle[0] = 0.0;
                pEllipseAngle[1] = Angle::PiOver2 ();
                pEllipseAngle[2] = Angle::Pi ();
                pEllipseAngle[3] = -Angle::PiOver2 ();
                }
            if (pCartesianPoints)
                {
                pCartesianPoints[0].SumOf (pEllipse->center, pEllipse->vector0);
                pCartesianPoints[1].SumOf (pEllipse->center, pEllipse->vector90);
                pCartesianPoints[2].SumOf (pEllipse->center, pEllipse->vector0, -1.0);
                pCartesianPoints[3].SumOf (pEllipse->center, pEllipse->vector90, -1.0);
                }
            }
        }
    else
        {
        RotMatrix matrix;
        DPoint3d vectorA;
        double theta;
        double dotUU, dotUV, dotVV, dotUA, dotVA, alpha, beta, gamma;
        double cosValue[4], sinValue[4], angleValue[4];
        int numSolution, i;
        vectorA.DifferenceOf (*pPoint, pEllipse->center);

        dotUU = pEllipse->vector0.DotProduct (pEllipse->vector0);
        dotUV = pEllipse->vector0.DotProduct (pEllipse->vector90);
        dotVV = pEllipse->vector90.DotProduct (pEllipse->vector90);
        dotUA = pEllipse->vector0.DotProduct (vectorA);
        dotVA = pEllipse->vector90.DotProduct (vectorA);
        alpha = 0.5 * (dotUU - dotVV);
        beta = 0.5 * dotVA;
        gamma = 0.5 * dotUA;
        matrix.InitFromRowValues (
                                -dotUV, alpha, beta,
                                alpha, dotUV,  -gamma,
                                beta, -gamma,  0.0);
        bsiBezier_implicitConicIntersectUnitCircle (cosValue, sinValue, angleValue, &numSolution, NULL, NULL, &matrix);
        for (i = 0; i < numSolution; i++)
            {
            theta = angleValue [i];
            if (pCartesianPoints)
                pEllipse->Evaluate (pCartesianPoints[i], theta);
            if (pEllipseAngle)
                pEllipseAngle[i] = theta;
            }
        numProjection = numSolution;
        }
    return numProjection;
    }


/*-----------------------------------------------------------------*//**
@description Project a point onto the xy projection of the (unbounded) ellipse.
@remarks May return up to 4 points.
@param pEllipse IN ellipse
@param pCartesianPoints OUT array (allocated by caller) of points on the ellipse.
@param pEllipseAngle OUT array (allocated by caller) of ellipse angles.
@param pPoint IN space point
 @return the number of projection points
@group "DEllipse3d Closest Point"
 @bsimethod                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int bsiDEllipse3d_projectPointXY

(
DEllipse3dCP pEllipse,
DPoint3dP pCartesianPoints,
double        *pEllipseAngle,
DPoint3dCP pPoint
)
    {
    int numProjection = 0;
    if (pEllipse->IsCircularXY ())
        {
        DPoint3d vectorToPoint;
        double rCircle = sqrt (pEllipse->vector0.DotProductXY (pEllipse->vector0));
        double rPoint;
        double scale;
        vectorToPoint.DifferenceOf (*pPoint, pEllipse->center);
        rPoint = sqrt (vectorToPoint.DotProductXY (vectorToPoint));
        if (DoubleOps::SafeDivide (scale, rCircle, rPoint, 0.0))
            {
            numProjection = 2;
            if (pCartesianPoints)
                {
                pCartesianPoints[0].SumOf (pEllipse->center, vectorToPoint, scale);
                pCartesianPoints[1].SumOf (pEllipse->center, vectorToPoint, -scale);
                }

            if (pEllipseAngle)
                {
                double dotU = vectorToPoint.DotProductXY (pEllipse->vector0);
                double dotV = vectorToPoint.DotProductXY (pEllipse->vector90);
                pEllipseAngle[0] = Angle::Atan2 (dotV, dotU);
                pEllipseAngle[1] = Angle::Atan2 (-dotV, -dotU);
                }
            }
        }
    else
        {
        RotMatrix matrix;
        DVec3d vectorA;
        double theta;
        double dotUU, dotUV, dotVV, dotUA, dotVA, alpha, beta, gamma;
        double cosValue[4], sinValue[4], angleValue[4];
        int numSolution, i;
        vectorA.DifferenceOf (*pPoint, pEllipse->center);

        dotUU = pEllipse->vector0.DotProductXY (pEllipse->vector0);
        dotUV = pEllipse->vector0.DotProductXY (pEllipse->vector90);
        dotVV = pEllipse->vector90.DotProductXY (pEllipse->vector90);
        dotUA = pEllipse->vector0.DotProductXY (vectorA);
        dotVA = pEllipse->vector90.DotProductXY (vectorA);
        alpha = 0.5 * (dotUU - dotVV);
        beta = 0.5 * dotVA;
        gamma = 0.5 * dotUA;
        matrix.InitFromRowValues (
                                -dotUV, alpha, beta,
                                alpha, dotUV,  -gamma,
                                beta, -gamma,  0.0);
        bsiBezier_implicitConicIntersectUnitCircle (cosValue, sinValue, angleValue, &numSolution, NULL, NULL, &matrix);
        for (i = 0; i < numSolution; i++)
            {
            theta = angleValue [i];
            pEllipse->Evaluate (pCartesianPoints[i], theta);
            pEllipseAngle[i] = theta;
            }
        numProjection = numSolution;
        }

    return numProjection;
    }

/*-----------------------------------------------------------------*//**
@description Project a point to the ellipse, and apply sector bounds.  Projection done
   by callback.
@param pCartesianPoints OUT array (allocated by caller) of points on the ellipse.
@param pEllipseAngle OUT array (allocated by caller) of ellipse angles.
@param pPoint IN space point
 @return the number of unbounded projection points
 @bsimethod                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
static int projectPointBounded_go

(
DEllipse3dCP pEllipse,
DPoint3dP pCartesianPoints,
double        *pEllipseAngle,
DPoint3dCP pPoint,
int (*projectionFunc)
    (
    const DEllipse3d *,
    DPoint3d *,
    double *,
    const DPoint3d *
    )
)
    {
    DPoint3d cartesianPoint[4];
    double   angle[4];
    int numBounded = 0;
    int i;
    int numUnBounded = projectionFunc (pEllipse, cartesianPoint, angle, pPoint);

    for (i = 0; i < numUnBounded; i++)
        {
        if (Angle::InSweepAllowPeriodShift (angle[i], pEllipse->start, pEllipse->sweep))
            {
            if (pCartesianPoints)
                pCartesianPoints[numBounded] = cartesianPoint[i];
            if (pEllipseAngle)
                pEllipseAngle[numBounded] = angle[i];
            numBounded++;
            }
        }
    return numBounded;
    }


/*-----------------------------------------------------------------*//**
@description Project a point to the xy projection of the ellipse, and apply sector bounds.
@remarks May return up to 4 points.
@param pEllipse IN ellipse
@param pCartesianPoints OUT array (allocated by caller) of points on the ellipse.
@param pEllipseAngle OUT array (allocated by caller) of ellipse angles.
@param pPoint IN space point
 @return the number of projection points
@group "DEllipse3d Closest Point"
 @bsimethod                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int bsiDEllipse3d_projectPointXYBounded

(
DEllipse3dCP pEllipse,
DPoint3dP pCartesianPoints,
double        *pEllipseAngle,
DPoint3dCP pPoint
)
    {
    return projectPointBounded_go
            (pEllipse, pCartesianPoints, pEllipseAngle, pPoint, bsiDEllipse3d_projectPointXY);
    }


/*-----------------------------------------------------------------*//**
@description Project a point to the xy projection of the ellipse, and apply sector bounds.
@remarks May return up to 4 points.
@param pEllipse IN ellipse
@param pCartesianPoints OUT array (allocated by caller) of points on the ellipse.
@param pEllipseAngle OUT array (allocated by caller) of ellipse angles.
@param pPoint IN space point
 @return the number of projection points
@group "DEllipse3d Closest Point"
 @bsimethod                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int bsiDEllipse3d_projectPointBounded

(
DEllipse3dCP pEllipse,
DPoint3dP pCartesianPoints,
double        *pEllipseAngle,
DPoint3dCP pPoint
)
    {
    return projectPointBounded_go
            (pEllipse, pCartesianPoints, pEllipseAngle, pPoint, bsiDEllipse3d_projectPoint);
    }

/*-----------------------------------------------------------------*//**
@description Find the closest point on a bounded ellipse, considering both endpoints and perpendicular
 projections.
 @bsimethod                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
static bool        closestPoint_go

(
DEllipse3dCP pEllipse,
double        *pMinAngle,         /* => parameter at closest approach point */
double        *pMinDistSquared,   /* => squard distance to closest approach point */
DPoint3dP pMinPoint,         /* => closest approach point */
DPoint3dCP pPoint,
double (*distanceFunc)(const DPoint3d*, const DPoint3d*),
int (*projectPointFunc)
    (
    const DEllipse3d    *pEllipse,
          DPoint3d      *pCartesianPoints,
          double        *pEllipseAngle,
    const DPoint3d      *pPoint
    )
)
    {
    DPoint3d cartesianPoint[6];
    double   angle[6];
    double theta;
    double distSquared, minDistSquared;
    int iMin;
    int numCandidate = 0;
    int i;

    numCandidate = projectPointFunc (pEllipse, cartesianPoint, angle, pPoint);

    if (!pEllipse->IsFullEllipse ())
        {
        theta = angle[numCandidate] = pEllipse->start;
        pEllipse->Evaluate (cartesianPoint[numCandidate], theta);
        angle[numCandidate] = theta;
        numCandidate++;

        theta = angle[numCandidate] = pEllipse->start + pEllipse->sweep;
        pEllipse->Evaluate (cartesianPoint[numCandidate], theta);
        angle[numCandidate] = theta;
        numCandidate++;
        }

    if (numCandidate > 0)
        {
        minDistSquared  = distanceFunc (pPoint, &cartesianPoint[0]);
        iMin = 0;
        for (i = 1; i < numCandidate; i++)
            {
            distSquared = distanceFunc (pPoint, &cartesianPoint[i]);
            if (distSquared < minDistSquared)
                {
                minDistSquared = distSquared;
                iMin = i;
                }
            }
        if (pMinDistSquared)
            *pMinDistSquared    = minDistSquared;
        if (pMinPoint)
            *pMinPoint          = cartesianPoint[iMin];
        if (pMinAngle)
            *pMinAngle          = angle[iMin];
        return true;
        }
    return false;
    }



/*-----------------------------------------------------------------*//**
@description Find the closest point on a bounded ellipse, considering both endpoints and perpendicular
 projections, and ignoring z of both the ellipse and space point.
@param pEllipse IN ellipse to search
@param pMinAngle OUT angular parameter at closest point
@param pMinDistSquared OUT squared distance to closest point
@param pMinPoint OUT closest point
@param pPoint IN space point
@return always true
@group "DEllipse3d Closest Point"
@bsimethod                                                      EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool        bsiDEllipse3d_closestPointXYBounded

(
DEllipse3dCP pEllipse,
double        *pMinAngle,
double        *pMinDistSquared,
DPoint3dP pMinPoint,
DPoint3dCP pPoint
)
    {
    return closestPoint_go (pEllipse, pMinAngle, pMinDistSquared,
                    pMinPoint, pPoint,
                    bsiDPoint3d_distanceSquaredXY,
                    bsiDEllipse3d_projectPointXYBounded
                    );
    }


/*-----------------------------------------------------------------*//**
@description Find the closest point on a bounded ellipse, considering both endpoints and perpendicular
 projections.
@param pEllipse IN ellipse to search
@param pMinAngle OUT angular parameter at closest point
@param pMinDistSquared OUT squared distance to closest point
@param pMinPoint OUT closest point
@param pPoint IN space point
@return always true
@group "DEllipse3d Closest Point"
 @bsimethod                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool        bsiDEllipse3d_closestPointBounded
(
DEllipse3dCP pEllipse,
double        *pMinAngle,
double        *pMinDistSquared,
DPoint3dP pMinPoint,
DPoint3dCP pPoint
)
    {
    return closestPoint_go (pEllipse, pMinAngle, pMinDistSquared,
                    pMinPoint, pPoint,
                    bsiDPoint3d_distanceSquared,
                    bsiDEllipse3d_projectPointBounded
                    );
    }

/*-----------------------------------------------------------------*//**
@description Find the intersections of xy projections of an ellipse and line, applying both ellipse and line parameter bounds.
 @param pEllipse      => ellipse to intersect with line.
 @param pCartesianPoints   <= cartesian intersection points.
 @param pLineParams         <= array of line parameters (0=start, 1=end)
 @param pEllipseCoffs      <= array of intersection coordinates in ellipse
                                frame.   xy are cosine and sine of angles.
                                z is z distance from plane of ellipse.
 @param pEllipseAngle      <= array of angles in ellipse parameter space.
 @param pIsTangency    <= true if the returned intersection is a tangency.
 @param pStartPoint    => line start
 @param pEndPoint      => line end
 @return the number of intersections after applying ellipse and line parameter limits.
@group "DEllipse3d Intersection"
 @bsimethod                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int bsiDEllipse3d_intersectXYLineBounded

(
DEllipse3dCP pEllipse,
DPoint3dP pCartesianPoints,
double        *pLineParams,
DPoint3dP pEllipseCoffs,
double        *pEllipseAngle,
bool          *pIsTangency,
DPoint3dCP pStartPoint,
DPoint3dCP pEndPoint
)
    {
    int numIntersection;
    int numBounded;
    int i;
    DPoint3d cartesianPoint[2], ellipseCoff[2];
    double   ellipseAngle[2];
    double   lineParam[2];
    double angle;
    numIntersection = pEllipse->IntersectXYLine (cartesianPoint, lineParam, ellipseCoff, ellipseAngle, *pStartPoint, *pEndPoint);

    if (pIsTangency)
        *pIsTangency = numIntersection == 1;

    numBounded = 0;
    for (i = 0; i < numIntersection ; i++)
        {
        if  (lineParam[i] >= 0.0 && lineParam[i] <= 1.0)
            {
            angle = ellipseAngle[i];
            if  (Angle::InSweepAllowPeriodShift (angle, pEllipse->start, pEllipse->sweep))
                {
                if  (pCartesianPoints)
                    pCartesianPoints[numBounded] = cartesianPoint[i];
                if  (pLineParams)
                    pLineParams[numBounded] = lineParam[i];
                if  (pEllipseCoffs)
                    pEllipseCoffs[numBounded] = ellipseCoff[i];
                if  (pEllipseAngle)
                    pEllipseAngle[numBounded] = angle;
                numBounded++;
                }
            }
        }
    return numBounded;
    }


/*-----------------------------------------------------------------*//**
* @description Returns a DEllipse3d with given fields.

+----------------------------------------------------------------------*/
DEllipse3d DEllipse3d::From (double cx, double cy, double cz, double ux, double uy, double uz, double vx, double vy, double vz, double startRadiansIn, double sweepIn)
    { 
    DEllipse3d ellipse;
    ellipse.Init(cx, cy, cz, ux, uy, uz, vx, vy, vz, startRadiansIn, sweepIn);
    
    return ellipse;
    }
 

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
DEllipse3d DEllipse3d::FromFractionInterval (DEllipse3dCR parent, double startFraction, double endFraction)
    { 
    if (DoubleOps::IsExact01 (startFraction, endFraction))
        return parent;
    DEllipse3d ellipse = parent;
    ellipse.start = parent.start + startFraction * parent.sweep;
    ellipse.sweep = (endFraction - startFraction) * parent.sweep;
    return ellipse;
    }

/*-----------------------------------------------------------------*//**
* @description Returns a DEllipse3d with given fields.

+----------------------------------------------------------------------*/
 

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
DEllipse3d DEllipse3d::FromXYMajorMinor (double cx, double cy, double cz, double rx, double ry, double thetaX, double startRadiansIn, double sweepIn)
    { 
    DEllipse3d ellipse;
    ellipse.InitFromXYMajorMinor (cx, cy, cz, rx, ry, thetaX, startRadiansIn, sweepIn);

    return ellipse;
    }
 
/*-----------------------------------------------------------------*//**
* @description Returns a DEllipse3d with given fields.

+----------------------------------------------------------------------*/


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
DEllipse3d DEllipse3d::FromPoints (DPoint3dCR centerIN, DPoint3dCR point0, DPoint3dCR point90, double startRadiansIn, double sweepIn)
    { 
    DEllipse3d ellipse;
    ellipse.InitFromPoints (centerIN, point0, point90, startRadiansIn, sweepIn);

    return ellipse;
    }
 
/*-----------------------------------------------------------------*//**
* @description Returns a DEllipse3d with given fields.

+----------------------------------------------------------------------*/


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
DEllipse3d DEllipse3d::FromPointsOnArc (DPoint3dCR start, DPoint3dCR middle, DPoint3dCR end)
    { 
    DEllipse3d ellipse;
    ellipse.InitFromPointsOnArc (start, middle, end);

    return ellipse;
    }
 
ValidatedDEllipse3d DEllipse3d::FromStartTangentNormalRadiusSweep
(
DPoint3dCR pointA,
DVec3dCR tangent,
DVec3dCR planeNormal,
double radius,
double sweepRadians
)
    {
    DVec3d vector0 = DVec3d::FromCrossProduct (tangent, planeNormal);
    if (!vector0.ScaleToLength (radius))
        return ValidatedDEllipse3d ();
    DVec3d vector90 = DVec3d::FromCrossProduct (planeNormal, vector0);
    if (!vector90.ScaleToLength (radius))
        return ValidatedDEllipse3d ();
    DPoint3d center = DPoint3d::FromSumOf (pointA, vector0, -1.0);
    return ValidatedDEllipse3d (
        DEllipse3d::FromVectors (center, vector0, vector90, 0.0, sweepRadians), true);

    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      05/2016
+--------------------------------------------------------------------------------------*/
DVec3d DEllipse3d::CrossProductOfBasisVectors () const
    {
    return DVec3d::FromCrossProduct (vector0, vector90);
    }

/*-----------------------------------------------------------------*//**
* @description Returns a DEllipse3d with given fields.

+----------------------------------------------------------------------*/


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
DEllipse3d DEllipse3d::FromArcCenterStartEnd (DPoint3dCR centerIN, DPoint3dCR start, DPoint3dCR end)
    { 
    DEllipse3d ellipse;
    ellipse.InitFromArcCenterStartEnd (centerIN, start, end);

    return ellipse;
    }
 
/*-----------------------------------------------------------------*//**
* @description Returns a DEllipse3d with given fields.

+----------------------------------------------------------------------*/


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
DEllipse3d DEllipse3d::FromVectors (DPoint3dCR centerIN, DVec3dCR vector0IN, DVec3dCR vector90IN, double startRadiansIn, double sweepIn)
    { 
    DEllipse3d ellipse;
    ellipse.InitFromVectors (centerIN, vector0IN, vector90IN, startRadiansIn, sweepIn);

    return ellipse;
    }
 
 
/*-----------------------------------------------------------------*//**
* @description Returns a DEllipse3d with given fields.

+----------------------------------------------------------------------*/


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
DEllipse3d DEllipse3d::FromScaledRotMatrix (DPoint3dCR centerIN, RotMatrixCR matrix, double r0, double r1, double startRadiansIn, double sweepIn)
    { 
    DEllipse3d ellipse;
    ellipse.InitFromScaledRotMatrix (centerIN, matrix, r0, r1, startRadiansIn, sweepIn);

    return ellipse;
    }
 
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
DEllipse3d DEllipse3d::FromScaledVectors (DEllipse3dCR source, double factor)
    { 
    DEllipse3d ellipse = source;
    ellipse.vector0.Scale (factor);
    ellipse.vector90.Scale (factor);
    return ellipse;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
DEllipse3d DEllipse3d::FromScaledVectors (DPoint3dCR centerIN, DVec3dCR vector0IN, DVec3dCR vector90IN, double r0, double r1, double startRadiansIn, double sweepIn)
    { 
    DEllipse3d ellipse;
    ellipse.InitFromScaledVectors (centerIN, vector0IN, vector90IN, r0, r1, startRadiansIn, sweepIn);

    return ellipse;
    }

/*-----------------------------------------------------------------*//**
* @description Returns a DEllipse3d with given fields.

+----------------------------------------------------------------------*/
;

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
DEllipse3d DEllipse3d::FromMajorMinor (DEllipse3dCR source)
    {
    DEllipse3d ellipse;
    ellipse.InitMajorMinor (source);

    return ellipse;
    }
 
/*-----------------------------------------------------------------*//**
* @description Returns a DEllipse3d with given fields.

+----------------------------------------------------------------------*/


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
DEllipse3d DEllipse3d::FromCenterNormalRadius (DPoint3dCR centerIN, DVec3dCR normal, double radius)
    { 
    DEllipse3d ellipse;
    ellipse.InitFromCenterNormalRadius (centerIN, normal, radius);

    return ellipse;
    }
 
/*-----------------------------------------------------------------*//**
* @description Returns a DEllipse3d with given fields.

+----------------------------------------------------------------------*/


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
DEllipse3d DEllipse3d::FromPerpendicularAxes (DEllipse3dCR source)
    { 
    DEllipse3d ellipse;
    ellipse.InitWithPerpendicularAxes  (source);

    return ellipse;
    }
 

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
DEllipse3d DEllipse3d::FromReversed (DEllipse3dCR source)
    { 
    DEllipse3d ellipse;
    ellipse.InitReversed (source);

    return ellipse;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
DEllipse3d DEllipse3d::FromNegateVector90 (DEllipse3dCR source)
    { 
    DEllipse3d ellipse = source;
    ellipse.vector90.Negate ();
    ellipse.start *= -1.0;
    ellipse.sweep *= -1.0;
    return ellipse;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      09/2013
+--------------------------------------------------------------------------------------*/
DEllipse3d DEllipse3d::FromCopyWithPositiveSweep (DEllipse3dCR source)
    { 
    if (source.sweep >= 0.0)
        return source;
    DEllipse3d result = source;
    result.sweep = -source.sweep;
    result.start = -source.start;
    result.vector90.Negate ();
    return result;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
DEllipse3d DEllipse3d::FromRotatedAxes (DEllipse3dCR source, double newStart)
    { 
    DEllipse3d ellipse = source;
    double theta = source.start - newStart;
    if (theta != 0.0)
        {
        double c = cos (theta);
        double s = sin (theta);
        ellipse.vector0.SumOf (source.vector0, c, source.vector90, s);
        ellipse.vector90.SumOf (source.vector0, -s, source.vector90, c);
        ellipse.start = newStart;
        }
    return ellipse;
    }
 
 

/*-----------------------------------------------------------------*//**
@description Fill in ellipse data.

 @param [out] pEllipse initialized ellipse
 @param [in] cx  center x coordinate
 @param [in] cy  center y coordinate
 @param [in] cz  center z coordinate
 @param [in] ux  x part of 0 degree vector
 @param [in] uy  y part of 0 degree vector
 @param [in] uz  z part of 0 degree vector
 @param [in] vx  x part of 90 degree vector
 @param [in] vy  y part of 90 degree vector
 @param [in] vz  z part of 90 degree vector
 @param [in] startRadiansIn  start angle in parameter space
 @param [in] sweepIn  sweep angle
 @group "DEllipse3d Initialization"
 @bsimethod                                                     EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DEllipse3d::Init
(

double        cx,
double        cy,
double        cz,
double        ux,
double        uy,
double        uz,
double        vx,
double        vy,
double        vz,
double        startRadiansIn,
double        sweepIn

)
    {
    center.Init (cx, cy, cz);
    vector0.Init (ux, uy, uz);
    vector90.Init (vx, vy, vz);

    start = startRadiansIn;
    sweep = sweepIn;
    }


/*-----------------------------------------------------------------*//**
@description Fill in ellipse data from 2D major and minor axis lengths and the angle
   from the global to the local x-axis.

 @param [out] pEllipse initialized ellipse
 @param [in] cx center x coordinate
 @param [in] cy center y coordinate
 @param [in] cz z coordinate of all points on the ellipse
 @param [in] rx radius along local x axis
 @param [in] ry radius along local y axis
 @param [in] thetaX angle from global x to local x
 @param [in] startRadiansIn start angle in parameter space
 @param [in] sweepIn sweep angle
 @group "DEllipse3d Initialization"
 @bsimethod                                                     EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DEllipse3d::InitFromXYMajorMinor
(

double          cx,
double          cy,
double          cz,
double          rx,
double          ry,
double          thetaX,
double          startRadiansIn,
double          sweepIn

)
    {
    double ux = cos (thetaX);
    double uy = sin (thetaX);

    center.Init (cx, cy, cz);
    vector0.Init (rx * ux, rx * uy, 0.0);
    vector90.Init (-ry * uy, ry * ux, 0.0);

    start = startRadiansIn;
    sweep = sweepIn;
    }


/*-----------------------------------------------------------------*//**
@description Fill in ellipse data from center, 0 degree, and 90 degree points.

 @param [out] pEllipse initialized ellipse
 @param [in] center0 ellipse center
 @param [in] point0 0 degree point
 @param [in] point90 90 degree point
 @param [in] startRadiansIn start angle
 @param [in] sweepIn sweep angle
 @group "DEllipse3d Initialization"
 @bsimethod                                                     EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DEllipse3d::InitFromPoints
(

DPoint3dCR center0,
DPoint3dCR point0,
DPoint3dCR point90,
double          startRadiansIn,
double          sweepIn

)
    {
    center = center0;
    vector0.DifferenceOf (point0, center0);
    vector90.DifferenceOf (point90, center0);
    start = startRadiansIn;
    sweep = sweepIn;
    }



/*-----------------------------------------------------------------*//**
@description Initialize an elliptical arc from 3 points.

 @param [out] pEllipse initialized ellipse
 @param [in] start start point
 @param [in] middle mid point
 @param [in] end end point
 @return true if the three points are valid, false if colinear.
 @group "DEllipse3d Initialization"
 @bsimethod                                                     EarlinLutz      12/97
+----------------------------------------------------------------------*/
bool DEllipse3d::InitFromPointsOnArc
(

DPoint3dCR start,
DPoint3dCR middle,
DPoint3dCR end

)
    {
    DVec3d normal;
    DVec3d vector01, vector12;
    double param0, param1;
    DPoint3d center0, center1;
    DRay3d perpendicularRay01, perpendicularRay12;

    DVec3d endVector;
    vector01.DifferenceOf (middle, start);
    vector12.DifferenceOf (end, middle);
    normal.NormalizedCrossProduct (vector01, vector12);
    perpendicularRay01.direction.CrossProduct (vector01, normal);
    perpendicularRay12.direction.CrossProduct (vector12, normal);
    perpendicularRay01.origin.SumOf (start, vector01, 0.5);
    perpendicularRay12.origin.SumOf (middle, vector12, 0.5);

    
    if (DRay3d::ClosestApproachUnboundedRayUnboundedRay
                (param0, param1, center0, center1, perpendicularRay01, perpendicularRay12))
        {
        center = center0;
        vector0.DifferenceOf (start, center0);
        vector90.CrossProduct (normal, vector0);
        this->start = 0.0;
        endVector.DifferenceOf (end, center0);
        sweep = vector0.SignedAngleTo (endVector, normal);
        if (sweep < 0.0)
            sweep = Angle::ReverseComplement (sweep);
        return true;
        }
    return false;
    }



/*-----------------------------------------------------------------*//**
@description Initialize a circlular arc from start point, end point, another vector which
  determines the plane, and the arc length.

 @param [out] pEllipse initialized ellipse
 @param [in] startIN start point
 @param [in] end end point
 @param [in] arcLength required arc length
 @param [in] planeVector vector to be used to determine the plane of the
                    arc.  The plane is chosen so that it contains both the
                    start-to-end vector and the plane vector, and the arc bulge
                    is in the direction of the plane vector (rather than opposite).
 @return true if the arc length exceeds the chord length and the 2 points and plane vector
                determine a clear plane.
 @group "DEllipse3d Initialization"
 @bsimethod                                                     EarlinLutz      01/01
+----------------------------------------------------------------------*/
bool DEllipse3d::InitArcFromPointPointArcLength
(
DPoint3dCR startIn,
DPoint3dCR endIn,
double  arcLength,
DVec3dCR planeVector
)
    {
    bool    boolstat = false;
    double lambda;
    static double s_minPhi = 1.0e-3;
    double chordLength = startIn.Distance (endIn);
    double s_tol = Angle::SmallAngle ();
    static double s_formulaSplit = 0.95;

    if (chordLength < arcLength)
        {
        int count;
        static int maxCount = 20;
        double phi, dPhi;
        lambda = chordLength / arcLength;
        /* Newton-Raphson iteration for
                f(phi)  = sin(phi)-lambda * phi = 0
                f'(phi) = cos(phi)-lambda
            The line lambda*phi and the sine wave both pass
                through (0,0).   For 0 < lambda < 1, they
                have one additional root between 0 and pi.
        To get a starting guess:
            Approximate sin(theta) by a parabola
                q(theta) = -4 theta (theta-pi)/pi^2
            solve q(theta) - lambda theta = 0
            This has a root at 0 and one at
                theta = pi (1 - pi lambda / 4)
        For lambda safely away from one, this converges in typical NR fashion --
            half dozen iterations.   As lambda approaches one,
            the equation's two roots come together, and it is easily seen
            (experimentally) that the NR still converges, albeit slowly.
            Examples:
                lambda  count
                .99999  17
                .9797    8
                .9213    6
                .519     4
        For the small values, a better estimate comes from the small angle formula
         sin(x) = x - x^3/6; hence
            (1-lambda)x - x^3/6 = 0

        */
        if (lambda < s_formulaSplit)
            {
            phi = msGeomConst_pi * (1.0 - msGeomConst_pi * lambda * 0.25);
            }
        else
            phi = sqrt (6.0 * (1.0 - lambda));


        dPhi = 1.0;
        for (count = 0; count < maxCount && fabs (dPhi) > s_tol; count++)
            {
            if (phi > msGeomConst_2pi)
                phi = msGeomConst_2pi;
            if (phi < s_minPhi)
                phi = msGeomConst_piOver2;
            dPhi = (sin (phi) - lambda * phi) / ( cos(phi) - lambda);
            phi -= dPhi;
            }

        if (fabs (dPhi) <= s_tol)
            {
            DVec3d chord, normal, binormal;
            double b;
            chord.DifferenceOf (endIn, startIn);
            if (!chord.IsParallelTo (planeVector)
                && DoubleOps::SafeDivide
                    (
                    b,
                    chordLength * cos (phi),
                    2.0 * sin (phi),
                    1.0))
                {
                normal.NormalizedCrossProduct (chord, planeVector);
                binormal.NormalizedCrossProduct (normal, chord);
                center.SumOf (startIn, chord, 0.5, binormal, -b);
                vector0.DifferenceOf (startIn, center);
                vector90.CrossProduct (vector0, normal);
                start = 0.0;
                sweep = 2.0 * phi;
                boolstat = true;
                }
            }
        }
    return boolstat;
    }


bool DEllipse3d::IntersectionOfStartAndEndTangents(DPoint3dR xyz) const
    {
    // Compute the intersection of the end tangents of the unit circle.  Map that to the (possibly skewed) plane of the ellipse.
    // (The skewed coordinate ellipse plane is a linear transform of unit circle.  Linear transform preserves tangents and intersections)
    double alpha = sweep * 0.5;
    double thetaMid = start + alpha;
    double cosine = cos(alpha);
    double secant;
    bool stat = DoubleOps::SafeDivide(secant, 1.0, cosine, 1.0);    // For parallel case, return mid-arc
    double u = secant * cos (thetaMid);   // hm . .  actually 1.0
    double v = secant * sin (thetaMid);   // hm . . actual tan (thetaMid)
    xyz.SumOf(center, vector0, u, vector90, v);
    return stat;
    }

/*-----------------------------------------------------------------*//**
@description Initialize a circular arc from start point, start tangent, and end point.

 @param [out] pEllipse initialized ellipse
 @param [in] startIN start point
 @param [in] tangent start tangent
 @param [in] end end point
 @return true if circular arc computed.   false if start, end and tangent are colinear.
 @group "DEllipse3d Initialization"
 @bsimethod                                                     EarlinLutz      12/97
+----------------------------------------------------------------------*/
bool DEllipse3d::InitArcFromPointTangentPoint
(

DPoint3dCR startIN,
DVec3d tangent,
DPoint3dCR end

)
    {
    DVec3d normal;
    DVec3d vector01;
    DPoint3d midPoint;
    DVec3d perp0, perp01;
    double param0, param1;
    DPoint3d center0, center1;
    DVec3d endVector;

    vector01.DifferenceOf (end, startIN);
    normal.NormalizedCrossProduct (tangent, vector01);
    perp01.CrossProduct (normal, vector01);
    perp0.CrossProduct (normal, tangent);
    midPoint.SumOf (startIN, vector01, 0.5);

    if (bsiGeom_closestApproachOfRays
                (&param0, &param1, &center0, &center1, &midPoint, &perp01, &startIN, &perp0))
        {
        center = center0;
        vector0.DifferenceOf (startIN, center0);
        vector90.CrossProduct (normal, vector0);
        start = 0.0;
        endVector.DifferenceOf (end, center0);
        sweep = vector0.SignedAngleTo (endVector, normal);
        if (sweep < 0.0)
            sweep = Angle::ReverseComplement (sweep);
        return true;
        }
    return false;
    }

/*-----------------------------------------------------------------*//**
@description Return an array of up to 4 points where a ray has closest approach to an ellipse.
@remarks Both ellipse and ray are unbounded.
@param pEllipse IN ellipse to search
@param pEllipseAngleBuffer OUT array (allocated by caller) to hold 4 angles on ellipse
@param pRayFractionBuffer OUT array (allocated by caller) to hold 4 fractions on ray
@param pEllipsePointBuffer OUT array (allocated by caller) to hold 4 ellipse points
@param pRayPointBuffer OUT array (allocated by caller) to hold 4 ray points
@param pRay IN ray to search
@return number of approach points computed.
@group "DEllipse3d Closest Point"
@bsimethod                                                      EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int             bsiDEllipse3d_closestApproachDRay3d

(
DEllipse3dCP pEllipse,
double          *pEllipseAngleBuffer,
double          *pRayFractionBuffer,
DPoint3dP pEllipsePointBuffer,
DPoint3dP pRayPointBuffer,
DRay3dCP pRay
)
    {
    DPoint3d vectorR; /* Ellipse center to ray origin */
    DPoint3d vectorT; /* Extent of ray */
    DPoint3d vectorWC, vectorW0, vectorW90; /* Intermediate pseudo ellipse */
    DPoint3d vectorS0, vectorS90; /* Tangential ellipse */
    double sinArray[4];
    double cosArray[4];
    double angArray[4];
    int numSolution;
    double a0T, a90T, aRT;
    int i;
    double tangentLength;
    RotMatrix coefficients;

    vectorR.DifferenceOf (pRay->origin, pEllipse->center);

    tangentLength = vectorT.Normalize (pRay->direction);

    if (tangentLength == 0.0)
        return 0;

    /* From the ellipse center, the vector from the ellipse point at theta to the
        ray point at lambda is
         vectorR + lambda * T - vector0 * cos(theta) - vector90 * sin(theta)
      Eliminate lambda by making this vector perpendiculat to T,
        lambda = (a0T*cos + a90T*sin - aRT)/ aTT
      vectorT is normalized, so aTT==1, and division drops out for
      lambda = a0T * cos + a90T * sin - aRT
    */
    a0T  = pEllipse->vector0.DotProduct (vectorT);
    a90T = pEllipse->vector90.DotProduct (vectorT);
    aRT  = vectorR.DotProduct (vectorT);
    /* Now make the vector perpendicular to the ellipse tangent S=-vector0*sin + vector90 * cos
        (vectorR + lambda * T - vector0 * cos(theta) - vector90 * sin(theta)) dot S = 0
        ([vectorR - aRT * T] + [a0T * T - vector0] * cos + [a90T * T - vector90] * sin] dot S = 0
        (vectorWC + vectorW0 * cos + vectorW90 * sin] dot S = 0
    */
    vectorWC.SumOf (vectorR, vectorT, -aRT);
    vectorW0.SumOf(vectorT, a0T, pEllipse->vector0, -1.0);
    vectorW90.SumOf(vectorT, a90T, pEllipse->vector90, -1.0);

    vectorS0.Scale (pEllipse->vector90, -1.0);
    vectorS90 = pEllipse->vector0;

    /* In outerproduct matrix form:
       [cos sin 1][W0 ][S0 S90 0][cos] = 0
                  [W90]          [sin]
                  [WC]           [1  ]
    */
    
    coefficients.form3d[0][0] = vectorW0.DotProduct (vectorS0);
    coefficients.form3d[0][1] = vectorW0.DotProduct (vectorS90);
    coefficients.form3d[0][2] = 0.0;

    coefficients.form3d[1][0] = vectorW90.DotProduct (vectorS0);
    coefficients.form3d[1][1] = vectorW90.DotProduct (vectorS90);
    coefficients.form3d[1][2] = 0.0;

    coefficients.form3d[2][0] = vectorWC.DotProduct (vectorS0);
    coefficients.form3d[2][1] = vectorWC.DotProduct (vectorS90);
    coefficients.form3d[2][2] = 0.0;    

    if (bsiBezier_implicitConicIntersectUnitCircle
                    (
                    cosArray,
                    sinArray,
                    angArray,
                    &numSolution,
                    NULL, NULL,
                    &coefficients))
        {
        /* Back out the parameters */
        for (i = 0; i < numSolution; i++)
            {
            double lambda = a0T * cosArray[i] + a90T * sinArray[i] - aRT;
            double fraction = lambda / tangentLength;
            if (pRayFractionBuffer)
                pRayFractionBuffer[i] = fraction;
            if (pRayPointBuffer)
                pRayPointBuffer[i].SumOf (pRay->origin, pRay->direction, fraction);
            if (pEllipseAngleBuffer)
                pEllipseAngleBuffer[i] = angArray[i];
            if (pEllipsePointBuffer)
                pEllipsePointBuffer[i].SumOf(pEllipse->center, pEllipse->vector0, cosArray[i], pEllipse->vector90, sinArray[i]);
            }
        return numSolution;
        }
    return 0;
    }

Public GEOMDLLIMPEXP void     bsiDConic4d_getQuadricBezierPoles

(
DConic4dCP pConic,
DPoint4dP pPoleArray,
DPoint3dP pCirclePoleArray,
int       *pNumPole,
int       *pNumSpan,
int       maxPole
)
    {
    bsiDConic4d_getQuadricBezierPoles (pConic, pPoleArray, pCirclePoleArray, nullptr, pNumPole, pNumSpan, maxPole);
    }
/*-----------------------------------------------------------------*//**
* @description Return the (weighted) control points of quadratic beziers which
*   combine to represent the full conic section.
*
* @param pConic => conic to evaluate
* @param pPoleArray <= array of poles of composite quadratic Bezier curve.  Array count depends on the angular extent.
* @param pCirclePoleArray <= array of corresponding poles which
*           give the bezier polynomials back to the unit circle points (x,y,w)
*           where x^2 + y^2 = w^2, but beware that w is not identically 1!  (Use this to map from bezier parameter back to angles.)
* @param pNumPole <= number of poles returned
* @param pNumSpan <= number of spans returned.  The pole indices for the first span are (always) 0,1,2; for the second span (if present),
*                    2,3,4, and so on.
* @param maxPole => maximum number of poles desired.  maxPole must be at least
*               5.  The circle is split into at most (maxPole - 1) / 2 spans.
*               Beware that for 5 poles the circle is split into at most
*               two spans, and there may be zero weights.   For 7 or more poles
*               all weights can be positive.   The function may return fewer
*               poles.
* @group "DConic4d Bezier"
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiDConic4d_getQuadricBezierPoles

(
DConic4dCP pConic,
DPoint4dP pPoleArray,
DPoint3dP pCirclePoleArray,
double*   pAngleArray,
int       *pNumPole,
int       *pNumSpan,
int       maxPole
)
    {
    int i;
#define MAX_CIRCLE_POLE 35
    DPoint3d circlePole[MAX_CIRCLE_POLE];
    int numPole;
    double sweep;
    int numSpan;
    int maxSpan;
    bool    fullCircle = Angle::IsFullCircle (pConic->sweep);
    double step, cc, theta;

    if (maxPole > MAX_CIRCLE_POLE)
        maxPole = MAX_CIRCLE_POLE;

    if (maxPole < 5)
        {
        /* Nothing to do! */
        numPole = 0;
        numSpan = 0;
        }
    else
        {
        maxSpan = (maxPole - 1) / 2;
        if (fullCircle)
            {
            numSpan = maxSpan;
            sweep = pConic->sweep > 0.0 ? msGeomConst_2pi : -msGeomConst_2pi;
            }
        else
            {
            sweep = pConic->sweep;
            numSpan = 1 + (int) ((fabs (pConic->sweep) / msGeomConst_2pi) * maxSpan);
            }

        if (numSpan > maxSpan)
            numSpan = maxSpan;

        step = 0.5 * sweep / numSpan;
        numPole = 1 + 2 * numSpan;
        for (i = 0; i < numPole; i++)
            {
            theta = pConic->start + i * step;
            circlePole[i].x = cos (theta);
            circlePole[i].y = sin (theta);
            circlePole[i].z = 1.0;
            }

        if (pAngleArray != nullptr)
            {
            pAngleArray[0]=pConic->start;
            for (i = 1; i <= numSpan; i++)
                pAngleArray[i] = pConic->start + 2.0 * i * step;
            }

        cc = cos (step);
        if (fullCircle && fabs (cc) < 1.0e-15)
            cc = 0.0;   // eliminate numerical fuzz at 90 degrees.
        for (i = 0; i < numSpan; i++)
            {
            circlePole[1 + 2 * i].z = cc;
            }

        /* Force exact closure on full circle case */
        if (fullCircle)
            circlePole[numPole - 1] = circlePole[0];

        }

    if (pPoleArray)
        {
        for (i = 0; i < numPole; i++)
            {
            pPoleArray[i].SumOf (
                            pConic->vector0,  circlePole[i].x,
                            pConic->vector90, circlePole[i].y,
                            pConic->center,   circlePole[i].z);
            }
        }

    if (pCirclePoleArray)
        {
        for (i = 0; i < numPole; i++)
            {
            pCirclePoleArray[i] = circlePole[i];
            }
        }

    if (pNumPole)
        *pNumPole = numPole;
    if (pNumSpan)
        *pNumSpan = numSpan;

    }



/*-----------------------------------------------------------------*//**
@description Initialize a circular arc with given center and start, and with
 sweep so that the end point is near the given end. (Note that the circle
 will NOT pass directly through the endpoint itself if it is at a different
 distance from the center.)  The arc is always the smaller of the two
 possible parts of the full circle.

 @param [out] pEllipse initialized ellipse
 @param [in] centerIN ellipse center
 @param [in] startIN start point
 @param [in] end nominal end point
 @return false if the the three points are colinear.
 @group "DEllipse3d Initialization"
 @bsimethod                                                     EarlinLutz      12/97
+----------------------------------------------------------------------*/
bool DEllipse3d::InitFromArcCenterStartEnd
(

DPoint3dCR centerIN,
DPoint3dCR startIN,
DPoint3dCR end

)
    {
    DPoint3d normal;
    DVec3d vector0IN, vector90IN, vectorEnd;
    double cc, ss;
    double sweepIn;
    double cotangent;
    vector0IN.DifferenceOf (startIN, centerIN);
    vectorEnd.DifferenceOf (end, centerIN);
    normal.NormalizedCrossProduct (vector0IN, vectorEnd);
    vector90IN.CrossProduct (normal, vector0IN);
    cc = vectorEnd.DotProduct (vector0IN);
    ss = vectorEnd.DotProduct (vector90IN);
    sweepIn = Angle::Atan2 (ss, cc);
    this->InitFromVectors (centerIN, vector0IN, vector90IN, 0.0, sweepIn);
    /* If the input was degenerate, the normal vector will be zero, hence
        the sine will be zero, hence we can't divide. */
    return DoubleOps::SafeDivideParameter (cotangent, cc, ss, 1.0);
    }

/*-----------------------------------------------------------------*//**
* @description Initalize a conic from its center, vector to 0 degree point, and vector to 90 degree point.
*
* @param pConic <= initialized conic
* @param pCenter => conic center
* @param pVector0 => 0 degree vector
* @param pVector90 => 90 degree vector
* @param theta0 => start angle
* @param sweep => sweep angle
* @group "DConic4d Initialization"
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDConic4d_initFrom3dVectors

(
DConic4dP pConic,
DPoint3dCP pCenter,
DPoint3dCP pVector0,
DPoint3dCP pVector90,
double          theta0,
double          sweep
)

    {
    pConic->center.InitFrom (*pCenter, 1.0);
    pConic->vector0.InitFrom (*pVector0, 0.0);
    pConic->vector90.InitFrom (*pVector90, 0.0);
    pConic->start        = theta0;
    pConic->sweep        = sweep;
    }

/*-----------------------------------------------------------------*//**
* @description Convert a cartesian ellipse to a homogeneous conic.
* @param pConic <= initialized conic
* @param pSource => cartesian ellipse
* @group "DConic4d Initialization"
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDConic4d_initFromDEllipse3d

(
DConic4dP pConic,
DEllipse3dCP pSource
)
    {
    bsiDConic4d_initFrom3dVectors (pConic,
                    &pSource->center,
                    &pSource->vector0,
                    &pSource->vector90,
                    pSource->start,
                    pSource->sweep);
    }

/*-----------------------------------------------------------------*//**
@description Fill in ellipse data from center and two basis vectors.

 @param [out] pEllipse initialized ellipse
 @param [in] centerIN ellipse center
 @param [in] vector0IN 0 degree vector
 @param [in] vector90IN 90 degree vector
 @param [in] startRadiansIn start angle
 @param [in] sweepIn sweep angle
 @group "DEllipse3d Initialization"
 @bsimethod                                                     EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DEllipse3d::InitFromVectors
(

DPoint3dCR centerIN,
DVec3dCR vector0IN,
DVec3dCR vector90IN,
double          startRadiansIn,
double          sweepIn

)
    {
    center = centerIN;
    vector0 = vector0IN,
    vector90 = vector90IN;
    start = startRadiansIn;
    sweep = sweepIn;
    }



/*-----------------------------------------------------------------*//**
@description Set angular parameters to have given start and end points.
@remarks If the given points are really on the ellipse, this does the expected thing.
@remarks If the given points are not on the ellipse, here's exactly what happens.
    The start/end points are placed on the original ellipse at the point where the ellipse intersects
    the plane formed by the ellipse axis and the given point.  This leaves the problem that the ellipse
    defines two paths from the given start to end. This is resolved as follows.  The ellipse's existing
    0 and 90 degree vectors define a coordinate system.  In that system, the short sweep from the 0
    degree vector to the 90 degree vector is considered "counterclockwise".
@remarks Beware that the relation of supposed start/end points to the ellipse is ambiguous.

 @param [in] pEllipse  ellipse to update
 @param [in] startPoint  new start point
 @param [in] endPoint  new end point
 @param [in] ccw true to force counterclockwise direction, false for clockwise.
 @return true if the ellipse axes are independent.  false if the ellipse is degenerate.
 @group "DEllipse3d Angles"
 @bsimethod                                                     EarlinLutz      12/97
+----------------------------------------------------------------------*/
bool DEllipse3d::SetStartEnd
(

DPoint3dCR startPoint,
DPoint3dCR endPoint,
bool ccw

)
    {
    Transform inverseFrame, frame;
    DPoint3d point[2], paramPoint[2];
    double theta0, theta1, computedSweep;
    bool isInvertible;

    isInvertible = this->GetLocalFrame (frame, inverseFrame);

    if (isInvertible)
        {
        point[0] = startPoint;
        point[1] = endPoint;

        inverseFrame.Multiply (paramPoint, point, 2);
        theta0 = Angle::Atan2 (paramPoint[0].y, paramPoint[0].x);
        theta1 = Angle::Atan2 (paramPoint[1].y, paramPoint[1].x);
        computedSweep  = theta1 - theta0;
        if (computedSweep < 0.0)
            {
            if (ccw)
                computedSweep += Angle::TwoPi ();
            }
        else
            {
            if (!ccw)
                computedSweep -= Angle::TwoPi ();
            }
        start = theta0;
        sweep = computedSweep;
        }
    return isInvertible;
    }


/*-----------------------------------------------------------------*//**
@description Fill in ellipse data from center, x and y directions from columns
 0 and 1 of a RotMatrix, and scale factors to apply to x and and y directions.

 @param [out] pEllipse initialized ellipse
 @param [in] centerIN ellipse center
 @param [in] matrix columns 0, 1 are ellipse directions (to be scaled by r0, r1)
 @param [in] r0 scale factor for column 0
 @param [in] r1 scale factor for column 1
 @param [in] startRadiansIn start angle
 @param [in] sweepIn sweep angle
 @group "DEllipse3d Initialization"
 @bsimethod                                                     EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DEllipse3d::InitFromScaledRotMatrix
(

DPoint3dCR centerIN,
RotMatrixCR matrix,
double      r0,
double      r1,
double              startRadiansIn,
double              sweepIn

)
    {
    DVec3d vectorU, vectorV, nullVector;
    matrix.GetColumns (vectorU, vectorV, nullVector);

    vectorU.Scale (vectorU, r0);
    vectorV.Scale (vectorV, r1);
    this->InitFromVectors (centerIN, vectorU, vectorV, startRadiansIn, sweepIn);
    }


/*-----------------------------------------------------------------*//**
@description Fill in ellipse data from center and x and y directions as vectors with scale factors.

 @param [out] pEllipse initialized ellipse
 @param [in] centerIN ellipse center
 @param [in] vector0IN 0 degree vector (e.g. major axis)
 @param [in] vector90IN 90 degree vector (e.g. minor axis)
 @param [in] r0 scale factor for vector 0
 @param [in] r1 scale factor for vector 90
 @param [in] startRadiansIn start angle
 @param [in] sweepIn sweep angle
 @group "DEllipse3d Initialization"
 @bsimethod                                                     EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DEllipse3d::InitFromScaledVectors
(

DPoint3dCR centerIN,
DVec3dCR vector0IN,
DVec3dCR vector90IN,
double      r0,
double      r1,
double              startRadiansIn,
double              sweepIn

)
    {
    DVec3d vectorU, vectorV;

    vectorU.Scale (vector0IN, r0);
    vectorV.Scale (vector90IN, r1);
    this->InitFromVectors (centerIN,
                vectorU,
                vectorV,
                startRadiansIn,
                sweepIn);
    }


/*-----------------------------------------------------------------*//**
@description Adjust axis vectors so 0-degree vector is along true major axis.
@remarks This is similar to ~mbsiDEllipse3d_initWithPerpendicularAxes, which
       chooses the 0 degree vector <EM>closest</EM> to current 0 degrees, even if
       that is really the "minor" axis.  This function makes the further adjustment
       of bringing the larger axis to 0 degrees in the parameterization.
 @param [out] majorMinorEllipse modified ellipse
 @param [in] source source ellipse.
 @group "DEllipse3d Initialization"
 @bsimethod                                                     EarlinLutz      07/03
+----------------------------------------------------------------------*/
void DEllipse3d::InitMajorMinor
(

DEllipse3dCR source

)
    {
    DVec3d vector180;
    double rr0, rr90;

    this->InitWithPerpendicularAxes (source);
    rr0 = vector0.MagnitudeSquared ();
    rr90 = vector90.MagnitudeSquared ();
    if (rr90 > rr0)
        {
        /* Save the current 180 direction.  Move 0 to 90, 90 to the saved 180.   Adjust start angle. */
        vector180.Negate (vector0);
        vector0 = vector90;
        vector90 = vector180;
        start -= Angle::PiOver2 ();
        }
    }


/*-----------------------------------------------------------------*//**
 @bsimethod                                                     EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DEllipse3d::GetScaledRotMatrix
(

DPoint3dR centerOUT,
RotMatrixR matrix,
double      &r0,
double      &r1,
double      &startRadiansIn,
double      &sweepOUT

) const
    {
    DVec3d xAxis, yAxis, zAxis;
    double r2, r3;
    DEllipse3d majorMinorEllipse;
    majorMinorEllipse.InitWithPerpendicularAxes (*this);
    r2 = xAxis.Normalize (majorMinorEllipse.vector0);
    r3 = yAxis.Normalize (majorMinorEllipse.vector90);
    zAxis.CrossProduct (xAxis, yAxis);

    centerOUT = majorMinorEllipse.center;
    matrix.InitFromColumnVectors (xAxis, yAxis, zAxis);

    r0 = r2;
    r1 = r3;

    startRadiansIn = majorMinorEllipse.start;
    sweepOUT = majorMinorEllipse.sweep;
    }

/*-----------------------------------------------------------------*//**
 @bsimethod                                                     EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DEllipse3d::GetScaledTransforms
(
TransformR localToWorld,
double &r0,
double &r1,
double &theta0,
double &sweepOut,
TransformR worldToLocal
) const
    {
    DPoint3d centerOut;
    RotMatrix axes;
    GetScaledRotMatrix (centerOut, axes, r0, r1, theta0, sweepOut);
    localToWorld.InitFrom (axes, centerOut);
    worldToLocal.InverseOf (localToWorld);
    }
/*-----------------------------------------------------------------*//**
@description Initialize a circle from center, normal and radius.
 @param [out] pEllipse initialized ellipse
 @param [in] centerIN circle center
 @param [in] normal plane normal (NULL for 001)
 @param [in] radius circle radius
 @group "DEllipse3d Initialization"
 @bsimethod                                                     EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DEllipse3d::InitFromCenterNormalRadius
(

DPoint3dCR centerIN,
DVec3dCR normal,
double          radius

)
    {
    DVec3d uVector, vVector, wVector;
    
    normal.GetNormalizedTriad(uVector, vVector, wVector);
    uVector.Scale (uVector, radius);
    vVector.Scale (vVector, radius);
    
    this->InitFromVectors (centerIN, uVector, vVector, 0.0, Angle::TwoPi ());
    }


/*-----------------------------------------------------------------*//**
@description Test whether the ellipse is complete (2pi range).
 @param [in] pEllipse ellipse to query
 @return true if the ellipse is complete
 @group "DEllipse3d Queries"
 @bsimethod                                                     EarlinLutz      12/97
+----------------------------------------------------------------------*/
bool DEllipse3d::IsFullEllipse
(

) const
    {
    return Angle::IsFullCircle (this->sweep);
    }

/*-----------------------------------------------------------------*//**
@description Test whether the ellipse is near zero radius
 @bsimethod                                                     EarlinLutz      11/13
+----------------------------------------------------------------------*/
bool DEllipse3d::IsNearZeroRadius () const
    {
    double r = vector0.Magnitude () + vector90.Magnitude ();
    double s = 1.0 + center.Magnitude ();
    return r < Angle::SmallAngle () * s;
    }


/*-----------------------------------------------------------------*//**
@description Set the ellipse sweep to a full 360 degrees (2pi radians).
@remarks Start angle is left unchanged.
 @param [in,out] pEllipse ellipse to change
 @group "DEllipse3d Angles"
 @bsimethod                                                     EarldinLutz     12/97
+----------------------------------------------------------------------*/
void DEllipse3d::MakeFullSweep
(

)
    {
    sweep = sweep >= 0.0 ? msGeomConst_2pi : -msGeomConst_2pi;
    }


/*-----------------------------------------------------------------*//**
@description Set the ellipse sweep to the complement of its current angular range.
@remarks Full ellipse is left unchanged.
 @param [in,out] pEllipse ellipse to change
 @group "DEllipse3d Angles"
 @bsimethod                                                     EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DEllipse3d::ComplementSweep
(

)
    {
    if (!this->IsFullEllipse ())
        sweep = Angle::ReverseComplement (sweep);
    }

/*-----------------------------------------------------------------*//**
@description Set the ellipse sweep to the smaller of its current or complement sweep.
@remarks Full ellipse is left unchanged.

 @bsimethod                                                     EarlinLutz      01/17
+----------------------------------------------------------------------*/
void DEllipse3d::SelectSmallerSweep ()
    {
    if (!this->IsFullEllipse ())
        {
        double complementSweep = Angle::ReverseComplement (sweep);
        if (fabs (complementSweep) < fabs (sweep))
            sweep = complementSweep;
        }
    }

ValidatedDEllipse3d DEllipse3d::ClosestEllipse (bvector<DEllipse3d> const &ellipses, double fraction, DPoint3dCR spacePoint)
    {
    double dMin = DBL_MAX;
    ValidatedDEllipse3d result = ValidatedDEllipse3d ();
    for (auto &e : ellipses)
        {
        double d = spacePoint.Distance (e.FractionToPoint (fraction));
        if (d < dMin)
            {
            result = e;
            dMin = d;
            }
        }
    return result;
    }


/*-----------------------------------------------------------------*//**
@description Compute the ellipse xyz point at a given parametric (angular) coordinate.
 @param [in] pEllipse ellipse to evaluate
 @param [out] point evaluated point
 @param [in] theta angle
 @group "DEllipse3d Evaluation"
 @bsimethod                                                     EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DEllipse3d::Evaluate
(

DPoint3dR point,
double      theta

) const
    {
    double cosTheta, sinTheta;
    cosTheta = cos(theta);
    sinTheta = sin(theta);
    point.SumOf (center, vector0, cosTheta, vector90, sinTheta);
    }

/*-----------------------------------------------------------------*//**
@bsimethod                                  EarlinLutz      10/15
+----------------------------------------------------------------------*/
DPoint3d DEllipse3d::RadiansToPoint (double radians) const
    {
    return DPoint3d::FromSumOf (center, vector0, cos (radians), vector90, sin (radians));
    }

/*-----------------------------------------------------------------*//**
@bsimethod                                  EarlinLutz      10/15
+----------------------------------------------------------------------*/
DPoint3d DEllipse3d::FractionToPoint (double fraction) const
    {
    return RadiansToPoint (FractionToAngle (fraction));
    }




/*-----------------------------------------------------------------*//**
@description Compute the ellipse xyz point at a given parametric (xy) coordinate.
 @param [in] pEllipse ellipse to evaluate
 @param [out] point evaluated point
 @param [in] xx local x coordinate: cos(theta)
 @param [in] yy local y coordinate: sin(theta)
 @group "DEllipse3d Evaluation"
 @bsimethod                                                     EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DEllipse3d::Evaluate
(

DPoint3dR point,
double      xx,
double      yy

) const
    {
    point.SumOf (center, vector0, xx, vector90, yy);
    }


/*-----------------------------------------------------------------*//**
@description Compute the ellipse xyz point at a given parametric (angular) coordinate.
 @param [in] pEllipse ellipse to evaluate
 @param [out] point evaluated point (unit weight)
 @param [in] theta angle
 @group "DEllipse3d Evaluation"
 @bsimethod                                                     EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DEllipse3d::Evaluate
(

DPoint4dR point,
double      theta

) const
    {
    double cosTheta, sinTheta;
    DPoint3d point0;
    cosTheta = cos(theta);
    sinTheta = sin(theta);
    point0.SumOf (center, vector0, cosTheta,vector90, sinTheta);
    point.x = point0.x;
    point.y = point0.y;
    point.z = point0.z;
    point.w = 1.0;
    }


/*-----------------------------------------------------------------*//**
@description Compute the ellipse start and end points.
 @param [in] pEllipse ellipse to evaluate
 @param [out] startPoint start point of ellipse
 @param [out] endPoint end point of ellipse
 @group "DEllipse3d Evaluation"
 @bsimethod                                                     EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DEllipse3d::EvaluateEndPoints
(

DPoint3dR startPoint,
DPoint3dR endPoint

) const
    {
    this->Evaluate (startPoint, start);
    this->Evaluate (endPoint, start + sweep);
    }


/*-----------------------------------------------------------------*//**
@description  Compute the ellipse xyz point and derivatives at a given parametric (angular) coordinate.
 @param [in] pEllipse ellipse to evaluate
 @param [out] point3dX (optional) point on ellipse
 @param [out] dX (optional) first derivative vector
 @param [out] ddX (optional) second derivative vector
 @param [in] theta angle for evaluation
 @group "DEllipse3d Evaluation"
 @bsimethod                                                     EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DEllipse3d::Evaluate
(

DPoint3dR point3dX,
DVec3dR dX,
DVec3dR ddX,
double      theta

) const
    {
    double cosTheta, sinTheta;
    DVec3d vector;

    cosTheta = cos(theta);
    sinTheta = sin(theta);

    vector.SumOf (vector0, cosTheta, vector90, sinTheta);
    point3dX.SumOf (center, vector);

    ddX.Negate (vector);

    dX.SumOf (vector0, -sinTheta, vector90, cosTheta);
    }


/*-----------------------------------------------------------------*//**
@description Compute the ellipse xyz point at a given fraction of the angular parametric range.
 @param [in] pEllipse ellipse to evaluate
 @param [out] point3dX point on ellipse
 @param [in] fraction fractional parameter for evaluation
 @group "DEllipse3d Evaluation"
 @bsimethod                                                     EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DEllipse3d::FractionParameterToPoint
(

DPoint3dR point3dX,
double      fraction

) const
    {
    double theta = start + fraction * sweep;
    double cosTheta, sinTheta;

    cosTheta = cos(theta);
    sinTheta = sin(theta);
    point3dX.SumOf (center, vector0, cosTheta, vector90, sinTheta);
    }


/*-----------------------------------------------------------------*//**
@description Compute the ellipse xyz point and derivatives at a given fraction of the angular parametric range.
 @param [in] pEllipse ellipse to evaluate
 @param [out] point3dX (optional) point on ellipse
 @param [out] dX (optional) second derivative vector
 @param [out] ddX (optional) second derivative vector
 @param [in] fraction fractional parameter for evaluation
 @group "DEllipse3d Evaluation"
 @bsimethod                                                     EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DEllipse3d::FractionParameterToDerivatives
(

DPoint3dR point3dX,
DVec3dR dX,
DVec3dR ddX,
double      fraction

) const
    {
    double theta = start + fraction * sweep;
    double a = sweep;

    this->Evaluate (point3dX, dX, ddX, theta);
    dX.Scale (dX, a);
    ddX.Scale (ddX, a * a);
    }


/*-----------------------------------------------------------------*//**
@description Compute ellipse xyz point and derivatives, returned as an array.
 @param [out] point3dX Array of ellipse point, first derivative, etc.  Must contain room for numDerivatives+1 points.  point3dX[i] = i_th derivative.
 @param [in] numDerivative number of derivatives (0 to compute just the xyz point)
 @param [in] theta angle for evaluation
 @group "DEllipse3d Evaluation"
 @bsimethod                                                     EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DEllipse3d::Evaluate
(

DPoint3dP        point3dX,
int              numDerivative,
double           theta

) const
    {
    double cosTheta, sinTheta;
    DVec3d vector;
    int i;

    cosTheta = cos(theta);
    sinTheta = sin(theta);

    vector.SumOf (vector0, cosTheta, vector90, sinTheta);

    point3dX[0].SumOf (center, vector);

    for (i = 2; i <= numDerivative ; i += 2)
        {
        vector.Negate ();
        point3dX[i] = vector;
        }

    if (numDerivative >= 1)
        {
        vector.SumOf (vector0, -sinTheta, vector90,  cosTheta);
        point3dX[1] = vector;

        for (i = 3; i <= numDerivative ; i += 2 )
            {
            vector.Negate ();
            point3dX[i] = vector;
            }
        }
    }


/*-----------------------------------------------------------------*//**
@description Convert a fractional parameter to ellipse parameterization angle.
 @param [in] pEllipse ellipse to evaluate
 @param [in] fraction fraction of angular range
 @return angular parameter
 @group "DEllipse3d Evaluation"
 @bsimethod                                                     EarlinLutz      12/97
+----------------------------------------------------------------------*/
double DEllipse3d::FractionToAngle
(

double      fraction

) const
    {
    return start + fraction * sweep;
    }


/*-----------------------------------------------------------------*//**
@description Compute the determinant of the Jacobian matrix for the transformation from local coordinates (cosine, sine) to global xy-coordinates.
 @param [in] pEllipse ellipse to query
 @return determinant of Jacobian.
 @group "DEllipse3d Queries"
 @bsimethod                                                     EarlinLutz      12/97
+----------------------------------------------------------------------*/
double DEllipse3d::DeterminantJXY
(

) const
    {
    return vector0.x * vector90.y - vector0.y * vector90.x;
    }


/*-----------------------------------------------------------------*//**
@description Get the coordinate frame for an ellipse.  X,Y axes are at 0 and 90 degrees.
 Z axis is perpendicular with magnitude equal to the geometric mean of the other two.
 @param [in] pEllipse ellipse whose frame is computed.
 @param [out] frame transformation from (cosine, sine, z) coordinates to global xyz.  May be NULL.
 @param [out] inverse inverse of frame.  May be NULL.
 @return true if the requested frames were returned.
 @group "DEllipse3d Queries"
 @bsimethod                                                     EarlinLutz      12/97
+----------------------------------------------------------------------*/
bool DEllipse3d::GetLocalFrame
(

TransformR frame,
TransformR inverse

) const
    {
    Transform frame1;
    bool myStat = true;
    DVec3d zVector;
    zVector.GeometricMeanCrossProduct (vector0, vector90);

    frame1.InitFromOriginAndVectors (center, vector0, vector90, zVector);

    frame = frame1;

    myStat = inverse.InverseOf (frame1);

    return myStat;
    }

/*-----------------------------------------------------------------*//**
 @bsimethod                                                     EarlinLutz      12/97
+----------------------------------------------------------------------*/
ValidatedTransform DEllipse3d::FractionToFrenetFrame (double fraction) const
    {
    DPoint3d xyz;
    DVec3d dX, ddX;
    FractionParameterToDerivatives(xyz, dX, ddX, fraction);
    DPoint3dDVec3dDVec3d basis (xyz, dX, ddX);
    return basis.NormalizedLocalToWorldTransform();
    }

/*-----------------------------------------------------------------*//**
 @bsimethod                                                     EarlinLutz      12/97
+----------------------------------------------------------------------*/
ValidatedTransform DEllipse3d::FractionToCenterFrame (double fraction) const
    {
    double c = cos (start);
    double s = sin(start);
    DVec3d vectorX = DVec3d::FromSumOf (vector0, c, vector90, s);
    DVec3d vectorY = DVec3d::FromSumOf (vector0, -s, vector90, c);
    DPoint3dDVec3dDVec3d basis(center, vectorX, vectorY);
    return basis.NormalizedLocalToWorldTransform();
    }


/*-----------------------------------------------------------------*//**
@description Get the coordinate frame and inverse of an ellipse as viewed along the global z axis.
 @param [in] pEllipse ellipse whose frame is computed.
 @param [out] frame transformation from (cosine, sine, z) coordinates to global xyz.  May be NULL.
 @param [out] inverse inverse of frame.  May be NULL.
 @return true if the requested frames were returned.
 @group "DEllipse3d Queries"
 @bsimethod                                                     EarlinLutz      12/97
+----------------------------------------------------------------------*/
bool DEllipse3d::GetXYLocalFrame
(

TransformR frame,
TransformR inverse

) const
    {
    Transform frame1;
    bool myStat = true;
    frame1.InitFromOriginAndVectors (center, vector0, vector90, DVec3d::From (0.0, 0.0, 1.0));

    frame = frame1;

    myStat = inverse.InverseOf (frame1);

    return myStat;
    }



/*-----------------------------------------------------------------*//**
@description Compute the local coordinates of a point in the skewed coordinates of the ellipse, using
 only xy parts of both the ellipse and starting point.
@remarks This is equivalent to computing the intersection of the ellipse plane with a line through the point and
 parallel to the z axis, and returning the coordinates of the intersection relative to the
 skewed axes of the ellipse.
 @param [in] pEllipse ellipse to evaluate
 @param [out] localPoint evaluated point.  Coordinates x,y are multipliers for the ellipse axes.
                        Coordinate z is height of the initial point from the plane of the ellipse.
 @param [in] point point to convert to local coordinates
 @return true if ellipse axes are independent.
 @group "DEllipse3d Closest Point"
 @bsimethod                                                     EarlinLutz      12/97
+----------------------------------------------------------------------*/
bool DEllipse3d::PointToXYLocal
(

DPoint3dR localPoint,
DPoint3dCR point

) const
    {
    Transform frame, inverse;
    bool myStat;
    myStat = this->GetXYLocalFrame (frame, inverse);
    if (myStat)
        {
        inverse.Multiply (&localPoint, &point, 1);
        }
    return myStat;
    }


/*-----------------------------------------------------------------*//**
@description Compute the angular position of the point relative to the ellipse's local coordinates.
@remarks If the point is on the ellipse, this is the inverse of evaluating the ellipse at the angle.
 @param [in] pEllipse ellipse definining angular space
 @param [in] point point to evaluate
 @return angle in ellipse parameterization
 @group "DEllipse3d Closest Point"
 @bsimethod                                                     EarlinLutz      12/97
+----------------------------------------------------------------------*/
double DEllipse3d::PointToAngle
(

DPoint3dCR point

) const
    {
    Transform frame;
    Transform inverse;
    DPoint3d localPoint;
    double theta = 0.0;

    if (this->GetLocalFrame (frame, inverse))
        {
        inverse.Multiply (&localPoint, &point, 1);
        theta = Angle::Atan2 (localPoint.y, localPoint.x);
        }
    return theta;
    }

/*-----------------------------------------------------------------*//**
@description Project a point onto the plane of the ellipse.

 @param [in] pEllipse ellipse whose axes become 3d plane directions.
 @param [out] xYZNear projection of point onto ellipse plane
 @param [out] coff0 coefficient on vector towards 0 degree point
 @param [out] coff90 coefficient on vector towards 90 degree point
 @param [in] xYZ point to project onto plane
 @return true if the plane is well defined.
@group "DEllipse3d Closest Point"
 @bsimethod                                                     EarlinLutz      12/97
+----------------------------------------------------------------------*/
bool DEllipse3d::ProjectPointToPlane
(

DPoint3dR planePoint,
double      &coff0,
double      &coff90,
DPoint3dCR spacePoint

) const
    {
    DPoint3dDVec3dDVec3d basis (center, vector0, vector90);
    ValidatedDPoint2d uv = basis.ProjectPointToUV (spacePoint);
    if (uv.IsValid ())
        {
        coff0 = uv.Value ().x;
        coff90 = uv.Value ().y;
        planePoint = basis.Evaluate (coff0, coff90);
        return true;
        }
    else
        {
        coff0 = coff90 = 0.0;
        planePoint = center;
        return false;
        }
    }



/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
int DEllipse3d::GetStrokeCount
(
int             nDefault,
int             nMax,
double          chordTol,
double          angleTol
) const
    {
    double r0squared, r90squared, rmax;
    double targetAngle = msGeomConst_piOver2;
    if (angleTol > 0 && angleTol < targetAngle)
        targetAngle = angleTol;
    if (chordTol > 0.0)
        {
        r0squared = vector0.DotProduct (vector0);
        r90squared = vector90.DotProduct (vector90);
        rmax = sqrt (DoubleOps::Max (r0squared, r90squared));

        if (rmax > chordTol)
            {
            double chordAngle = 2.0 * acos (1.0 - chordTol / rmax);
            if (chordAngle < targetAngle)
                targetAngle = chordAngle;
            }
        }

    int n = (int)(0.99999 + msGeomConst_2pi / targetAngle);
    if (n < nDefault)
        n = nDefault;
    // restrict return value to [4,nMax]
    if (n < 4)
        n = 4;
    else if (n > nMax)
        n = nMax;

    return n;
    }


/*-----------------------------------------------------------------*//**
@description Evaluate an ellipse using given coefficients for the axes.
@remarks If the x,y components of the coefficients define a unit vector, the point is "on" the ellipse.
 @param [out] point array of cartesian points
 @param [in] trig array of local coords (e.g., (cos, sin)).
 @param [in] numPoint number of pairs
 @group "DEllipse3d Evaluation"
 @bsimethod                                                     EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DEllipse3d::EvaluateTrigPairs
(

DPoint3dP       point,
DPoint2dCP      trig,
int             numPoint

) const
    {
    int i;
    for (i = 0; i < numPoint; i++)
        {
        point[i].SumOf (center, vector0, trig[i].x, vector90, trig[i].y);
        }
    return;
    }


/*-----------------------------------------------------------------*//**
@description Evaluate an ellipse at a number of (cosine, sine) pairs, removing
 pairs whose corresponding angle is not in range.

 @param [out] point array of cartesian points
 @param [in] trig array of local coords
 @param [in] numPoint number of pairs
 @return number of points found to be in the angular range of the ellipse.
 @group "DEllipse3d Evaluation"
 @bsimethod                                                     EarlinLutz      12/97
+----------------------------------------------------------------------*/
int DEllipse3d::TestAndEvaluateTrigPairs
(

DPoint3dP       point,
DPoint2dCP      trig,
int             numPoint

) const
    {
    int n = 0;
    int i;

    for (i = 0; i < numPoint; i++)
        {
        double theta = Angle::Atan2 (trig[i].y, trig[i].x);
        if (Angle::InSweepAllowPeriodShift (theta, start, sweep))
            {
            this->EvaluateTrigPairs (&point[n++], &trig[i], 1);
            }
        }
    return n;
    }


/*-----------------------------------------------------------------*//**
@description Test if a specified angle is within the sweep of the ellipse.
 @param [in] pEllipse ellipse whose angular range is queried
 @param [in] angle angle (radians) to test
 @return true if angle is within the sweep angle of the elliptical arc.
@group "DEllipse3d Angles"
 @bsimethod                                                     EarlinLutz      12/97
+----------------------------------------------------------------------*/
bool DEllipse3d::IsAngleInSweep
(

double      angle

) const
    {
    return Angle::InSweepAllowPeriodShift (angle, start, sweep);
    }


/*-----------------------------------------------------------------*//**
@description Convert an angular parameter to a fraction of bounded arc length.
 @param [in] pEllipse ellipse whose angular range is queried
 @param [in] angle angle (radians) to convert
 @return fractional parameter
 @group "DEllipse3d Evaluation"
 @bsimethod                                                     EarlinLutz      12/97
+----------------------------------------------------------------------*/
double DEllipse3d::AngleToFraction
(

double      angle

) const
    {
    return Angle::NormalizeToSweep (angle, start, sweep);
    }


/*-----------------------------------------------------------------*//**
@description Get the start and end angles of the ellipse.
 @param [in] pEllipse ellipse whose angular range is queried
 @param [out] startAngle start angle
 @param [out] endAngle end angle
 @group "DEllipse3d Angles"
 @bsimethod                                                     EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DEllipse3d::GetLimits
(

double    &startAngle,
double    &endAngle

) const
    {
    startAngle = start;
    endAngle = start + sweep;
    }


/*-----------------------------------------------------------------*//**
@description Get the start and sweep angles of the ellipse.
 @param [in] pEllipse ellipse whose angular range is queried.
 @param [out] startAngle start angle
 @param [out] sweepAngle sweep angle
 @group "DEllipse3d Angles"
 @bsimethod                                                     EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DEllipse3d::GetSweep
(

double    &startAngle,
double    &sweepAngle

) const
    {
    startAngle = start;
    sweepAngle = sweep;
    }


/*-----------------------------------------------------------------*//**
@description Set the start and end angles of the ellipse.
 @param [out] pEllipse ellipse whose angular range is changed
 @param [in] startAngle start angle
 @param [in] endAngle end angle
 @group "DEllipse3d Angles"
 @bsimethod                                                     EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DEllipse3d::SetLimits
(

double    startAngle,
double    endAngle

)
    {
    start = startAngle;
    sweep = endAngle - startAngle;
    }


/*-----------------------------------------------------------------*//**
@description Set the start and sweep angles of the ellipse.
 @param [out] pEllipse ellipse whose angular range is changed
 @param [in] startAngle start angle
 @param [in] sweep sweep angle
 @group "DEllipse3d Angles"
 @bsimethod                                                     EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DEllipse3d::SetSweep
(

double    startAngle,
double    sweepIn

)
    {
    start = startAngle;
    sweep = sweepIn;
    }


/*-----------------------------------------------------------------*//**
@description Make a copy of the source ellipse, altering the axis vectors and angular limits so that
 the revised ellipse has perpendicular axes in the conventional major/minor axis form.
@remarks Inputs may be the same.
 @param [out] pEllipse ellipse with perpendicular axes
 @param [in] source ellipse with unconstrained axes
 @group "DEllipse3d Initialization"
 @bsimethod                                                     EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DEllipse3d::InitWithPerpendicularAxes
(

DEllipse3dCR source

)
    {
    double   dotUV, dotUU, dotVV;
    double   c, s, theta;
    double   ax, ay, a, tol;
    DVec3d vector0IN, vector90IN;

    dotUV = source.vector0.DotProduct (source.vector90);
    dotUU = source.vector0.DotProduct (source.vector0);
    dotVV = source.vector90.DotProduct (source.vector90);

    ay = dotUU - dotVV;
    ax = 2.0 * dotUV;
    a = dotUU + dotVV;
    tol = Angle::SmallAngle () * a;
    if (fabs (ax) < tol)
        {
        *this = source;
        }
    else
        {
        Angle::HalfAngleFuctions (c, s, ay, ax);
        Angle::Rotate90UntilSmallAngle (c, s, c, s);
        *this = source;
        /* Save the given axes in locals because the originals will be overwritten
            if the same ellipse is being used for input and output. */
        vector0IN = source.vector0;
        vector90IN = source.vector90;
        vector0.SumOf (vector0IN, c, vector90IN, s);
        vector90.SumOf (vector0IN, -s, vector90IN, c);
        theta = Angle::Atan2 (s,c);
        start -= theta;
        }
    }


/*-----------------------------------------------------------------*//**
@description Compute the range box of the ellipse in its major-minor axis coordinate system.
 Compute line segments that are the horizontal and vertical midlines in that system.
 Return those line segments ordered with the longest first, and return the shorter length.

@remarks The typical use of this is that if the shorter length is less than some tolerance the
 points swept out by the ellipse are the longer segment.  (But beware that the start and
 end points of the segment can be other than the start and end points of the ellipse.)

 @param [in] pEllipse ellipse to analyze
 @param [out] longSegment longer axis of local conic range box
 @param [out] shortSegment shorter axis of local conic range box
 @return size of the shorter dimension
 @group "DEllipse3d Queries"
 @bsimethod                                                     EarlinLutz      12/97
+----------------------------------------------------------------------*/
double DEllipse3d::GetMajorMinorRangeMidlines
(

DSegment3dR longSegment,
DSegment3dR shortSegment

) const
    {
    DEllipse3d majorMinorEllipse;
    DRange2d localRange;
    DSegment3d segment[2];
    double xBar, yBar;
    int iLong, iShort;
    double lengthSquared[2];

    majorMinorEllipse.InitWithPerpendicularAxes (*this);
    majorMinorEllipse.GetLocalRange (localRange);

    xBar = 0.5 * (localRange.low.x + localRange.high.x);
    yBar = 0.5 * (localRange.low.y + localRange.high.y);

    majorMinorEllipse.Evaluate (segment[0].point[0], localRange.low.x, yBar);
    majorMinorEllipse.Evaluate (segment[0].point[1], localRange.high.x, yBar);

    majorMinorEllipse.Evaluate (segment[1].point[0], xBar, localRange.low.y);
    majorMinorEllipse.Evaluate (segment[1].point[1], xBar, localRange.high.y);

    lengthSquared[0] = segment[0].LengthSquared ();
    lengthSquared[1] = segment[1].LengthSquared ();

    if (lengthSquared[0] >= lengthSquared[1])
        {
        iLong = 0;
        iShort = 1;
        }
    else
        {
        iLong = 1;
        iShort = 0;
        }

    longSegment = segment[iLong];

    shortSegment = segment[iShort];

    return sqrt (lengthSquared[iShort]);
    }


/*-----------------------------------------------------------------*//**
@description Make a copy of the source ellipse, reversing the start and end angles.
@remarks Inputs may be the same.
 @param [out] pEllipse copied and reversed ellipse
 @param [in] source source ellipse
 @group "DEllipse3d Initialization"
 @bsimethod                                                     EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DEllipse3d::InitReversed
(

DEllipse3dCR source

)
    {
    if (this != &source)
        *this = source;
    start = start + sweep;
    sweep = (-this->sweep);
    }


/*-----------------------------------------------------------------*//**
* @description Compute the magnitude of the tangent vector to the ellipse at the specified angle.
 @param [in] pEllipse ellipse to evaluate
 @param [in] theta angular parameter
 @return tangent magnitude
 @group "DEllipse3d Evaluation"
 @bsimethod                                                     EarlinLutz      12/97
+----------------------------------------------------------------------*/
double DEllipse3d::TangentMagnitude
(

double      theta

) const
    {
    DVec3d tangent;
    double s, c;

    s =  sin(theta);
    c =  cos(theta);

    tangent.SumOf (vector0, -s, vector90, c);

    return tangent.Magnitude ();
    }

#ifndef MinimalRefMethods


/*---------------------------------------------------------------------------------**//**
@description Return arc length of ellipse.
@param [in] pEllipse  ellipse to integrate
@return arc length of ellipse.
@group "DEllipse3d Queries"
@bsimethod                                    Earlin.Lutz                     05/2006
+--------------------------------------------------------------------------------------*/
double DEllipse3d::ArcLength
(

) const
    {
    DEllipse3d ellipseMM;
    double a, b;
    ellipseMM.InitMajorMinor (*this);
    a = ellipseMM.vector0.Magnitude ();
    b = ellipseMM.vector90.Magnitude ();
    return bsiGeom_ellipseArcLength (a, b, ellipseMM.start, ellipseMM.sweep);
    }


/*---------------------------------------------------------------------------------**//**
@description Return the sweep angle corresponding to an arc length.
@remarks Negative returned sweep angle corresponds to arclength traversed in the opposite direction of the ellipse sweep.
@param [in] pEllipse  ellipse to integrate
@param [in] arcLength  arc length to invert
@return sweep angle
@group "DEllipse3d Queries"
@bsimethod                                    Earlin.Lutz                     06/2006
+--------------------------------------------------------------------------------------*/
double DEllipse3d::InverseArcLength
(

double arcLength

) const
    {
    DEllipse3d  ellipseMM;
    double a, b, startRadians, sweepScale = 1.0;

    ellipseMM.InitMajorMinor (*this);
    a = ellipseMM.vector0.Magnitude ();
    b = ellipseMM.vector90.Magnitude ();
    startRadians = ellipseMM.start;

    if (arcLength < 0.0)
        {
        sweepScale = -1.0;
        arcLength = -arcLength;
        startRadians = -startRadians;
        }

    if (sweep < 0.0)
        {
        startRadians = -startRadians;
        }

    return sweepScale * bsiGeom_ellipseInverseArcLength (a, b, startRadians, arcLength);
    }


/*---------------------------------------------------------------------------------**//**
@description Compute the (signed) arc length between specified fractional parameters.
@remarks Fractions outside [0,1] return error.
 @param [in] pEllipse ellipse to measure.
 @param [out] arcLength computed arc length.  Negative if fraction1 < fraction0.
 @param [in] fraction0 start fraction for interval to measure
 @param [in] fraction1 end fraction for interval to measure
 @return true if the arc length was computed.
 @group "DEllipse3d Queries"
 @bsimethod                                                     EarlinLutz      03/99
+--------------------------------------------------------------------------------------*/
bool DEllipse3d::FractionToLength
(

double      &arcLength,
double      fraction0,
double      fraction1

) const
    {
    DEllipse3d partialEllipse = *this;
    partialEllipse.start = start + fraction0 * sweep;
    partialEllipse.sweep = sweep * (fraction1 - fraction0);
    arcLength = partialEllipse.ArcLength ();
    if (fraction1 < fraction0)
        arcLength = -arcLength;
    return true;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      11/12
+--------------------------------------------------------------------------------------*/
DRange1d DEllipse3d::ProjectedParameterRange (DRay3dCR ray) const
    {
    double divAA = ray.direction.SafeOneOverMagnitudeSquared (0.0);
    double c = ray.DirectionDotVectorToTarget (center) * divAA;
    double u = ray.direction.DotProduct (vector0) * divAA;
    double v = ray.direction.DotProduct (vector90) * divAA;
    DRange1d range = DRange1d::From (c);
    if (!Angle::IsFullCircle (sweep))
        {
        double endRadians = start + sweep;
        range.Extend (c + u * cos (start) + v * sin(start));
        range.Extend (c + u * cos (endRadians) + v * sin(endRadians));
        }
    bsiEllipse_componentRange (&range.low, &range.high, c, u, v, start, sweep);
    return range;
    }


/*-----------------------------------------------------------------*//**
@description Compute the xyz range limits of a 3D ellipse.
 @param [in] pEllipse ellipse whose range is determined
 @param [out] range computed range
 @group "DEllipse3d Queries"
 @bsimethod                                                     EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DEllipse3d::GetRange (DRange3dR range) const
    {
    DPoint3d startPoint, endPoint;
    double theta0 = start;
    double sweepIn  = sweep;

    if (!Angle::IsFullCircle (sweep))
        {
        this->Evaluate (startPoint, theta0);
        this->Evaluate (endPoint, theta0 + sweepIn);

        range.low = range.high = startPoint;

        FIX_MIN (endPoint.x, range.low.x);
        FIX_MIN (endPoint.y, range.low.y);
        FIX_MIN (endPoint.z, range.low.z);

        FIX_MAX (endPoint.x, range.high.x);
        FIX_MAX (endPoint.y, range.high.y);
        FIX_MAX (endPoint.z, range.high.z);
        }
    else
        {
        range.low = range.high = center;
        }

    bsiEllipse_componentRange (&range.low.x, &range.high.x,
                        center.x, vector0.x, vector90.x,
                        theta0, sweepIn);

    bsiEllipse_componentRange (&range.low.y, &range.high.y,
                        center.y, vector0.y, vector90.y,
                        theta0, sweepIn);

    bsiEllipse_componentRange (&range.low.z, &range.high.z,
                        center.z, vector0.z, vector90.z,
                        theta0, sweepIn);
    }

#endif

/*-----------------------------------------------------------------*//**
@description Compute the range of the ellipse in its own coordinate system..
@remarks This depends on the start and sweep angles but not the center or axis coordinates.
 @param [in] pEllipse ellipse whose range is determined
 @param [out] range computed range
 @group "DEllipse3d Queries"
 @bsimethod                                                     EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DEllipse3d::GetLocalRange (DRange2dR range) const
    {
    range.InitFromUnitArcSweep (start, sweep);
    }


#ifndef MinimalRefMethods

/*-----------------------------------------------------------------*//**
@description Find intersections of a (full) ellipse with a plane.
@remarks Return value n=1 is a single tangency point returned in trigPoints[0];
        n=2 is two simple intersections returned in trigPoints[0..1]
@remarks The three component values in trigPoints are:
<UL>
<LI>x == cosine of angle
<LI>y == sine of angle
<LI>z == angle in radians
</UL>
 @param [in] pEllipse ellipse to intersect with plane
 @param [out] trigPoints 2 points: cosine, sine, theta values of plane intersection
 @param [in] plane homogeneous plane equation
 @return The number of intersections, i.e. 0, 1, or 2
 @group "DEllipse3d Intersection"
 @bsimethod                                                     EarlinLutz      12/97
+----------------------------------------------------------------------*/
int DEllipse3d::IntersectPlane
(

DPoint3dP trigPoints,
DPoint4dCR plane

) const
    {
    double alpha = plane.DotProduct (center, 1.0);
    double beta  = plane.DotProduct (vector0, 0.0);
    double gamma = plane.DotProduct (vector90, 0.0);

    int n = bsiMath_solveApproximateUnitQuadratic (
                &trigPoints[0].x, &trigPoints[0].y,
                &trigPoints[1].x, &trigPoints[1].y,
                alpha, beta, gamma, s_lineUnitCircleIntersectionTolerance);
    if ( n == 1 )
        {
        /* Take the on-circle solution */
        trigPoints[0].x = trigPoints[1].x;
        trigPoints[0].y = trigPoints[1].y;
        trigPoints[0].z = Angle::Atan2 (trigPoints[0].y, trigPoints[0].x);
        }
    else if ( n == 2 )
        {
        trigPoints[0].z = Angle::Atan2 (trigPoints[0].y, trigPoints[0].x);
        trigPoints[1].z = Angle::Atan2 (trigPoints[1].y, trigPoints[1].x);
        }
    else
        {
        /* suppress the degenerate cases */
        n = 0;
        }
    return n;
    }



/*-----------------------------------------------------------------*//**
@description Find angles at which the ellipse tangent vector is perpendicular to given vector.
 @param [out] angles 0,1, or 2 angles.   This is an array that must be allocated by the caller.
 @param [in] vector perpendicular vector.
 @return The number of solutions, i.e. 0, 1, or 2
 @group "DEllipse3d Intersection"
 @bsimethod                                                     EarlinLutz      12/97
+----------------------------------------------------------------------*/
int DEllipse3d::SolveTangentsPerpendicularToVector
(
double  *angles,   
DVec3dR vector
) const
    {
    DPoint3d trigPoints[2];
    double alpha = 0.0;
    double beta  = vector0.DotProduct (vector);
    double gamma = vector90.DotProduct (vector);

    int n = bsiMath_solveApproximateUnitQuadratic (
                &trigPoints[0].x, &trigPoints[0].y,
                &trigPoints[1].x, &trigPoints[1].y,
                alpha, beta, gamma, s_lineUnitCircleIntersectionTolerance);
    if ( n == 1 )
        {
        /* Take the on-circle solution */
        angles[0] = Angle::Atan2 (trigPoints[0].y, trigPoints[0].x);
        }
    else if ( n == 2 )
        {
        angles[0] = Angle::Atan2 (trigPoints[0].y, trigPoints[0].x);
        angles[1] = Angle::Atan2 (trigPoints[1].y, trigPoints[1].x);
        }
    else
        {
        /* suppress the degenerate cases */
        n = 0;
        }
    return n;
    }




/*-----------------------------------------------------------------*//**
@description Find the intersections of xy projections of an ellipse and line.
@remarks May return 0, 1, or 2 points.  Both ellipse and line are unbounded.
 @param [out] cartesianPoints cartesian intersection points
 @param [out] pLineParams array of line parameters (0=start, 1=end)
 @param [out] ellipseCoffs array of coordinates relative to the ellipse.
                              For each point, (xy) are the cosine and sine of the
                              ellipse parameter, (z) is z distance from the plane of
                              of the ellipse.
 @param [out] pEllipseAngle array of angles on the ellipse
 @param [in] startPoint line start
 @param [in] endPoint line end
 @return the number of intersections.
 @group "DEllipse3d Intersection"
 @bsimethod                                                     EarlinLutz      12/97
+----------------------------------------------------------------------*/
int DEllipse3d::IntersectXYLine
(

DPoint3dP       cartesianPoints,
double          *pLineParams,
DPoint3dP       ellipseCoffs,
double          *pEllipseAngle,
DPoint3dCR      startPoint,
DPoint3dCR      endPoint

) const
    {
    Transform frame, inverse;
    DPoint3d ellipseCoffs0[2];
    int numIntersection;
    int i;

    this->GetXYLocalFrame (frame, inverse);

    numIntersection  = bsiCylinder_intersectLine (cartesianPoints, pLineParams, ellipseCoffs0,
                                frame, startPoint, endPoint);
    for (i = 0; i < numIntersection; i++)
        {
        ellipseCoffs[i] = ellipseCoffs0[i];

        pEllipseAngle[i] = Angle::Atan2 (ellipseCoffs0[i].y, ellipseCoffs0[i].x);
        }

    return numIntersection;
    }

#endif

/*-----------------------------------------------------------------*//**
@bsimethod                                                      EarlinLutz      12/97
+----------------------------------------------------------------------*/
bool DEllipse3d::IsCircular () const {double r; return IsCircular (r);}


/*-----------------------------------------------------------------*//**
@bsimethod                                                      EarlinLutz      12/12
+----------------------------------------------------------------------*/
bool DEllipse3d::IsCircular (double &r) const
    {
    double relTol = Angle::SmallAngle ();
    double  dotUV, magU, magV;

    dotUV = vector0.DotProduct (vector90);
    r = magU = vector0.Magnitude ();
    magV = vector90.Magnitude ();

    /* Circular if the axes have the same magnitude and are perpendicular */
    if (fabs (magU - magV) < (magU + magV) * relTol)
        {
        if (fabs (dotUV) < magU * magV * relTol)
            return true;
        }
    return false;
    }

/*-----------------------------------------------------------------*//**
@bsimethod                                                      EarlinLutz      6/15
+----------------------------------------------------------------------*/
bool DEllipse3d::IsCCWSweepXY () const
    {
    return sweep * vector0.CrossProductXY (vector90) > 0.0;
    }



/*-----------------------------------------------------------------*//**
@bsimethod                                                      EarlinLutz      12/12
+----------------------------------------------------------------------*/
bool DEllipse3d::IsCircularXY () const { double r; return IsCircular (r);}

/*-----------------------------------------------------------------*//**
@bsimethod                                                      EarlinLutz      12/12
+----------------------------------------------------------------------*/
bool DEllipse3d::IsCircularXY (double &r) const
    {
    double uu = vector0.DotProductXY (vector0);
    double uv = vector90.DotProductXY (vector0);
    double vv = vector90.DotProductXY (vector90);

    static double s_reltol = 1.0e-12;
    r = sqrt (uu);
    return    fabs (uv) < s_reltol * uu
            && fabs (uu - vv) < s_reltol * uu;
    }


/*-----------------------------------------------------------------*//**
@description Find the intersections of xy projections of two ellipses.
@remarks May return 0, 1, 2, 3 or 4 points.  Both ellipses are unbounded.
 @param [out] cartesianPoints cartesian intersection points.
 @param [out] ellipse0Params array of coordinates relative to the first ellipse.
                            For each point, (xy) are the cosine and sine of the
                            ellipse parameter, (z) is z distance from the plane of
                            of the ellipse.
 @param [out] ellipse1Params array of coordinates relative to the second ellipse.
                            For each point, (xy) are the cosine and sine of the
                            ellipse parameter, (z) is z distance from the plane of
                            of the ellipse.
 @param [in] ellipse1 the other ellipse.
 @return the number of intersections.
 @group "DEllipse3d Intersection"
 @bsimethod                                                     EarlinLutz      12/97
+----------------------------------------------------------------------*/
int DEllipse3d::IntersectXYDEllipse3d
(

DPoint3dP       cartesianPoints,
DPoint3dP       ellipse0Params,
DPoint3dP       ellipse1Params,
DEllipse3dCR    ellipse1

) const
    {
    Transform ellipse0Frame,  ellipse1Frame, inverse0, inverse1;
    RotMatrix ellipse0Matrix, ellipse1Matrix;
    double ellipse0Condition, ellipse1Condition;
    int numUnbounded = 0;

    /* Condition number from dmatrix3d tells about skewing.
       We multiply by SIZE factor so that we favor using
       smaller ellipse as reference frame.
    */
    this->GetXYLocalFrame (ellipse0Frame, inverse0);
    ellipse0Matrix.InitFrom (ellipse0Frame);
    ellipse0Condition = ellipse0Matrix.ConditionNumber ();
    ellipse0Condition *= sqrt (fabs (
            DVec3d::FromMatrixColumn (ellipse0Frame, 0).CrossProductXY (DVec3d::FromMatrixColumn (ellipse0Frame, 0))));

    ellipse1.GetXYLocalFrame (ellipse1Frame, inverse1);
    ellipse1Matrix.InitFrom (ellipse1Frame);
    ellipse1Condition = ellipse1Matrix.ConditionNumber ();
    ellipse1Condition *= sqrt (fabs (
            DVec3d::FromMatrixColumn (ellipse1Frame, 0).CrossProductXY (DVec3d::FromMatrixColumn (ellipse1Frame, 0))));

    if (ellipse0Condition > ellipse1Condition)
        {
        numUnbounded = intersectXYEllipseFrames (
                                cartesianPoints,
                                ellipse0Params, ellipse1Params,
                                &ellipse0Frame, &ellipse1Frame);
        }
    else
        {
        numUnbounded = intersectXYEllipseFrames (
                                cartesianPoints,
                                ellipse1Params,   ellipse0Params,
                                &ellipse1Frame,    &ellipse0Frame);
        }
    return numUnbounded;
    }


/*-----------------------------------------------------------------*//**
@description Find the intersections of xy projections of two ellipses, with bounds applied.
@remarks May return 0, 1, 2, 3 or 4 points.
 @param [out] cartesianPoints cartesian intersection points.
 @param [out] ellipse0Coffs array of coordinates relative to the first ellipse
                            For each point, (xy) are the cosine and sine of the
                            ellipse parameter, (z) is z distance from the plane of
                            of the ellipse.
 @param [out] pEllipse0Angle array of angles on the first ellipse
 @param [out] ellipse1Coffs array of coordinates relative to the second ellipse.
                            For each point, (xy) are the cosine and sine of the
                            ellipse parameter, (z) is z distance from the plane of
                            of the ellipse.
 @param [out] pEllipse1Angle array of angles on the other ellipse
 @param [in] ellipse1 the other ellipse.
 @return the number of intersections.
 @group "DEllipse3d Intersection"
 @bsimethod                                                     EarlinLutz      12/97
+----------------------------------------------------------------------*/
int DEllipse3d::IntersectXYDEllipse3dBounded
(

DPoint3dP       cartesianPoints,
DPoint3dP       ellipse0Coffs,
double          *pEllipse0Angle,
DPoint3dP       ellipse1Coffs,
double          *pEllipse1Angle,
DEllipse3dCR    ellipse1

) const
    {
    DPoint3d cartesianPoint[4], ellipse0Coffs0[4], ellipse1Coffs0[4];
    int numUnBounded;
    int numBounded;

    numUnBounded = this->IntersectXYDEllipse3d (cartesianPoint, ellipse0Coffs0, ellipse1Coffs0, ellipse1);

    numBounded = filterDualSweeps (
                        cartesianPoints,
                        ellipse0Coffs, pEllipse0Angle,
                        ellipse1Coffs, pEllipse1Angle,
                        this,
                        &ellipse1,
                        cartesianPoint, ellipse0Coffs0, ellipse1Coffs0, numUnBounded);
    return numBounded;
    }


/*-----------------------------------------------------------------*//**
@description Find "intersections" of two DEllipse3d.
@remarks Ellipses in space can pass very close to
 each other without intersecting, so some logic must be applied to define intersection
 more cleanly. The logic applied is to choose the more circular ellipse and apply the
 transformation which makes that one a unit circle, then intersect the xy projections of the
 transformations.

 @param [out] cartesianPoints cartesian intersection points.
 @param [out] ellipse0Params array of coordinates relative to the first ellipse
                                For each point, (xy) are the cosine and sine of the
                                ellipse parameter, (z) is z distance from the plane of
                                of the ellipse.
 @param [out] ellipse1Params array of coordinates relative to the second ellipse.
                                For each point, (xy) are the cosine and sine of the
                                ellipse parameter, (z) is z distance from the plane of
                                of the ellipse.
 @param [in] ellipse1 the other ellipse.
 @return the number of intersections.
 @group "DEllipse3d Intersection"
 @bsimethod                                                     EarlinLutz      12/97
+----------------------------------------------------------------------*/
int DEllipse3d::IntersectSweptDEllipse3d
(

DPoint3dP cartesianPoints,
DPoint3dP ellipse0Params,
DPoint3dP ellipse1Params,
DEllipse3dCR ellipse1

) const
    {
    Transform ellipse0Frame, ellipse1Frame, inverse0, inverse1;
    double ellipse0Condition, ellipse1Condition;
    int numUnbounded = 0;

    this->GetLocalFrame (ellipse0Frame, inverse0);
    ellipse0Condition = RotMatrix::From (ellipse0Frame).ConditionNumber ();

    ellipse1.GetLocalFrame (ellipse1Frame, inverse1);
    ellipse1Condition = RotMatrix::From (ellipse1Frame).ConditionNumber ();

    if (ellipse0Condition > ellipse1Condition)
        {
        numUnbounded = intersectXYEllipseFrames (cartesianPoints, ellipse0Params, ellipse1Params, &ellipse0Frame, &ellipse1Frame);
        }
    else
        {
        numUnbounded = intersectXYEllipseFrames (cartesianPoints, ellipse1Params, ellipse0Params, &ellipse1Frame, &ellipse0Frame);
        }
    return numUnbounded;
    }


/*-----------------------------------------------------------------*//**
@description Intersect two ellipses as described in ~mbsiDEllipse3d_intersectSweptDEllipse3d, and
 filter out results not part of both ranges.

 @param [in] pEllipse0 ellipse to intersect with cylinder of second ellipse.
 @param [out] cartesianPoints cartesian intersection points.
 @param [out] ellipse0Coffs array of coordinates relative to the first ellipse
                            For each point, (xy) are the cosine and sine of the
                            ellipse parameter, (z) is z distance from the plane of
                            of the ellipse.
 @param [out] pEllipse0Angle array of angles on the first ellipse.
 @param [out] ellipse1Coffs array of coordinates relative to the second ellipse.
                            For each point, (xy) are the cosine and sine of the
                            ellipse parameter, (z) is z distance from the plane of
                            of the ellipse.
 @param [out] pEllipse1Angle array of angles on the other ellipse.
 @param [in] ellipse1 the other ellipse.
 @return the number of intersections.
 @group "DEllipse3d Intersection"
 @bsimethod                                                     EarlinLutz      12/97
+----------------------------------------------------------------------*/
int DEllipse3d::IntersectSweptDEllipse3dBounded
(

DPoint3dP cartesianPoints,
DPoint3dP ellipse0Coffs,
double        *pEllipse0Angle,
DPoint3dP ellipse1Coffs,
double        *pEllipse1Angle,
DEllipse3dCR ellipse1

) const
    {
    DPoint3d cartesianPoint[4], ellipse0Coffs0[4], ellipse1Coffs0[4];
    int numUnBounded;
    int numBounded;

    numUnBounded = this->IntersectSweptDEllipse3d (cartesianPoint, ellipse0Coffs0, ellipse1Coffs0, ellipse1);

    numBounded = filterDualSweeps (
                                    cartesianPoints,
                                    ellipse0Coffs, pEllipse0Angle,
                                    ellipse1Coffs, pEllipse1Angle,
                                    this, &ellipse1,
                                    cartesianPoint,
                                    ellipse0Coffs0,
                                    ellipse1Coffs0,
                                    numUnBounded);
    return numBounded;
    }

#ifndef MinimalRefMethods

/*-----------------------------------------------------------------*//**
@description Find "intersections" of a DSegment3d and a DEllipse3d.
@remarks Curves in space can pass very close to
 each other without intersecting, so some logic must be applied to define intersection
 more cleanly. The logic applied is to compute the intersection of the line with
 the cylinder swept by the ellipse along its plane normal.

 @param [in] pEllipse base ellipse for the cylinder.
 @param [out] pointArray cartesian intersection points.
 @param [out] ellipseParams array of coordinates relative to the instance
                            For each point, (xy) are the cosine and sine of the
                            ellipse parameter, (z) is z distance from the plane of
                            of the ellipse.
 @param [out] pLineParams array of parametric coordinates on the line.
 @param [in] segment the line segment
 @return the number of intersections.
  @group "DEllipse3d Intersection"
 @bsimethod                                                     EarlinLutz      12/97
+----------------------------------------------------------------------*/
int DEllipse3d::IntersectSweptDSegment3d
(

DPoint3dP pointArray,
DPoint3dP ellipseParams,
double        *pLineParams,
DSegment3dCR segment

) const
    {
    return bsiDEllipse3d_intersectSweptDSegment3d (this, pointArray, ellipseParams, pLineParams, (&segment));
    }


/*-----------------------------------------------------------------*//**
@description Intersect an ellipse and a segment as described in ~mbsiDEllipse3d_intersectSweptDSegment3d, and
 filter out results not part of both ranges.

 @param [in] pEllipse base ellipse for the cylinder.
 @param [out] pointArray cartesian intersection points.
 @param [out] ellipseParams array of coordinates relative to the instance
                            For each point, (xy) are the cosine and sine of the
                            ellipse parameter, (z) is z distance from the plane of
                            of the ellipse.
 @param [out] pLineParams array of parametric coordinates on the line.
 @param [in] segment the line segment
 @return the number of intersections.
@group "DEllipse3d Intersection"
 @bsimethod                                                     EarlinLutz      12/97
+----------------------------------------------------------------------*/
int DEllipse3d::IntersectSweptDSegment3dBounded
(

DPoint3dP pointArray,
DPoint3dP ellipseParams,
double        *pLineParams,
DSegment3dCR segment

) const
    {
    return bsiDEllipse3d_intersectSweptDSegment3dBounded (this, pointArray, ellipseParams, pLineParams, (&segment));
    }


/*-----------------------------------------------------------------*//**
@description Project a point onto the (unbounded) ellipse.
@remarks May return up to 4 points.
@param [out]cartesianPoints  array (allocated by caller) of points on the ellipse.
@param [out]pEllipseAngle  array (allocated by caller) of ellipse angles.
@param [in] point  space point
 @return the number of projection points
@group "DEllipse3d Closest Point"
 @bsimethod                                                     EarlinLutz      12/97
+----------------------------------------------------------------------*/
int DEllipse3d::ProjectPoint
(

DPoint3dP cartesianPoints,
double        *pEllipseAngle,
DPoint3dCR point

) const
    {
    return bsiDEllipse3d_projectPoint (this, cartesianPoints, pEllipseAngle, (&point));
    }


/*-----------------------------------------------------------------*//**
@description Project a point onto the xy projection of the (unbounded) ellipse.
@remarks May return up to 4 points.
@param [out]cartesianPoints  array (allocated by caller) of points on the ellipse.
@param [out]pEllipseAngle  array (allocated by caller) of ellipse angles.
@param [in] point  space point
 @return the number of projection points
@group "DEllipse3d Closest Point"
 @bsimethod                                                     EarlinLutz      12/97
+----------------------------------------------------------------------*/
int DEllipse3d::ProjectPointXY
(

DPoint3dP   cartesianPoints,
double        *pEllipseAngle,
DPoint3dCR point

) const
    {
    return bsiDEllipse3d_projectPointXY (this, cartesianPoints, pEllipseAngle, (&point));
    }


/*-----------------------------------------------------------------*//**
@description Project a point to the xy projection of the ellipse, and apply sector bounds.
@remarks May return up to 4 points.
@param [out]cartesianPoints  array (allocated by caller) of points on the ellipse.
@param [out]pEllipseAngle  array (allocated by caller) of ellipse angles.
@param [in] point  space point
 @return the number of projection points
@group "DEllipse3d Closest Point"
 @bsimethod                                                     EarlinLutz      12/97
+----------------------------------------------------------------------*/
int DEllipse3d::ProjectPointXYBounded
(

DPoint3dP cartesianPoints,
double        *pEllipseAngle,
DPoint3dCR point

) const
    {
    return bsiDEllipse3d_projectPointXYBounded (this, cartesianPoints, pEllipseAngle, (&point));
    }


/*-----------------------------------------------------------------*//**
@description Project a point to the xy projection of the ellipse, and apply sector bounds.
@remarks May return up to 4 points.
@param [out]cartesianPoints  array (allocated by caller) of points on the ellipse.
@param [out]pEllipseAngle  array (allocated by caller) of ellipse angles.
@param [in] point  space point
 @return the number of projection points
@group "DEllipse3d Closest Point"
 @bsimethod                                                     EarlinLutz      12/97
+----------------------------------------------------------------------*/
int DEllipse3d::ProjectPointBounded
(

DPoint3dP cartesianPoints,
double        *pEllipseAngle,
DPoint3dCR point

) const
    {
    return bsiDEllipse3d_projectPointBounded (this, cartesianPoints, pEllipseAngle, (&point));
    }


/*-----------------------------------------------------------------*//**
@description Find the closest point on a bounded ellipse, considering both endpoints and perpendicular
 projections, and ignoring z of both the ellipse and space point.
@param [in] pEllipse  ellipse to search
@param [out] minAngle  angular parameter at closest point
@param [out] minDistSquared  squared distance to closest point
@param [out]minPoint  closest point
@param [in] point  space point
@return always true
@group "DEllipse3d Closest Point"
@bsimethod                                                      EarlinLutz      12/97
+----------------------------------------------------------------------*/
bool DEllipse3d::ClosestPointXYBounded
(

double        &minAngle,
double        &minDistSquared,
DPoint3dR minPoint,
DPoint3dCR point

) const
    {
    return bsiDEllipse3d_closestPointXYBounded (this, &minAngle,&minDistSquared,(&minPoint),(&point)) ? true : false;
    }


/*-----------------------------------------------------------------*//**
@description Find the closest point on a bounded ellipse, considering both endpoints and perpendicular
 projections.
@param [in] pEllipse  ellipse to search
@param [out] minAngle angular parameter at closest point
@param [out] minDistSquared  squared distance to closest point
@param [out]minPoint  closest point
@param [in] point  space point
@return always true
@group "DEllipse3d Closest Point"
 @bsimethod                                                     EarlinLutz      12/97
+----------------------------------------------------------------------*/
bool DEllipse3d::ClosestPointBounded
(

double        &minAngle,
double        &minDistSquared,
DPoint3dR minPoint,
DPoint3dCR point

) const
    {
    return bsiDEllipse3d_closestPointBounded (this, &minAngle,&minDistSquared,(&minPoint),(&point)) ? true : false;
    }


/*-----------------------------------------------------------------*//**
@description Find the intersections of xy projections of an ellipse and line, applying both ellipse and line parameter bounds.
 @param [out] cartesianPoints cartesian intersection points.
 @param [out] pLineParams array of line parameters (0=start, 1=end)
 @param [out] ellipseCoffs array of intersection coordinates in ellipse
                                frame.   xy are cosine and sine of angles.
                                z is z distance from plane of ellipse.
 @param [out] pEllipseAngle array of angles in ellipse parameter space.
 @param [out] pIsTangency true if the returned intersection is a tangency.
 @param [in] startPoint line start
 @param [in] endPoint line end
 @return the number of intersections after applying ellipse and line parameter limits.
@group "DEllipse3d Intersection"
 @bsimethod                                                     EarlinLutz      12/97
+----------------------------------------------------------------------*/
int DEllipse3d::IntersectXYLineBounded
(

DPoint3dP cartesianPoints,
double        *pLineParams,
DPoint3dP ellipseCoffs,
double        *pEllipseAngle,
bool          *pIsTangency,
DPoint3dCR startPoint,
DPoint3dCR endPoint

) const
    {
    return bsiDEllipse3d_intersectXYLineBounded (this, cartesianPoints, pLineParams, ellipseCoffs, pEllipseAngle, pIsTangency, (&startPoint), (&endPoint));
    }


/*-----------------------------------------------------------------*//**
@description Compute area and swept angle as seen from given point.
@param [in] pEllipse  ellipse
@param [out] area  swept area
@param [out] sweepOUT  swept angle (in radians)
@param [in] point  base point for sweep line.
@group "DEllipse3d Queries"
 @bsimethod                                                     EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DEllipse3d::XySweepProperties
(

double        &area,
double        &sweepOUT,
DPoint3dCR point

) const
    {
    bsiDEllipse3d_xySweepProperties (this, &area, &sweepOUT,(&point));
    }

#ifdef abc
/*-----------------------------------------------------------------*//**
@description Apply a transformation to the source ellipse.
 @param [out] pDest transformed ellipse
 @param [in] transform transformation to apply
 @param [in] source source ellipse
@group "DEllipse3d Transform"
 @bsimethod                                                     EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DEllipse3d::InitProduct
(

TransformCR transform,
DEllipse3dCR source

)
    {
    bsiDEllipse3d_multiplyTransformDEllipse3d (this, (&transform),(&source));
    }
#endif
/*-----------------------------------------------------------------*//**
@description Return the (weighted) control points of quadratic beziers which
   combine to represent the full conic section.

 @param [out] poleArray array of poles of composite quadratic Bezier curve.  Array count depends on the angular extent.
 @param [out] circlePoleArray array of corresponding poles which
            map the bezier polynomials back to the unit circle points (x,y,w)
            where x^2 + y^2 = w^2, but beware that w is not identically 1!  (Use this to map from bezier parameter back to angles.)
 @param [out] pNumPole number of poles returned
 @param [out] pNumSpan number of spans returned.  The pole indices for the first span are (always) 0,1,2; for the second span (if present),
                    2,3,4, and so on.
 @param [in] maxPole maximum number of poles desired.  maxPole must be at least
                5.  The circle is split into (maxPole - 1) / 2 spans.
                Beware that for 5 poles the circle is split into at most
                two spans, and there may be zero weights.   For 7 or more poles
                all weights can be positive.  The function may return fewer
                poles.
 @group "DEllipse3d Queries"
 @bsimethod                                                     EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DEllipse3d::QuadricBezierPoles
(

DPoint4dP     poleArray,
DPoint3dP     circlePoleArray,
double*       angleArray,
int           *pNumPole,
int           *pNumSpan,
int           maxPole
) const
    {
    DConic4d conic;
    bsiDConic4d_initFromDEllipse3d (&conic, this);
    bsiDConic4d_getQuadricBezierPoles (&conic, poleArray, circlePoleArray, angleArray, pNumPole, pNumSpan, maxPole);
    }


void DEllipse3d::QuadricBezierPoles
(
DPoint4dP     poleArray,
DPoint3dP     circlePoleArray,
int           *pNumPole,
int           *pNumSpan,
int           maxPole
) const
    {
    QuadricBezierPoles (poleArray, circlePoleArray, nullptr, pNumPole, pNumSpan, maxPole);
    }

/*-----------------------------------------------------------------*//**
@description Initialize an ellipse from center, primary axis point, and additional pass-though point.
@param [out]pEllipse  initialized ellipse
@param [in] centerIN center point of ellipse.
@param [in] point0 point to appear at the zero degree point.   The ellipse must pass through
                this point as a major or minor axis point, i.e. its tangent must be perpendicular
                to the vector from the center to this point.
@param [in] point1 additional pass-through point.
@return false if center, point0 and point1 are not independent, or if
    point1 is too far away from center to allow ellipse constrution.
@group "DEllipse3d Initialization"
@bsimethod                                                      EarlinLutz      12/97
+----------------------------------------------------------------------*/
bool DEllipse3d::InitFromCenterMajorAxisPointAndThirdPoint
(

DPoint3dCR centerIN,
DPoint3dCR point0,
DPoint3dCR point1

)
    {
    double dot00, dot01, disc, root;
    double cc, ss, dss, axisRatio;
    double alpha0, alpha1;
    center = centerIN;
    vector0 = DVec3d::FromStartEnd (centerIN, point0);
    DVec3d vector1 = DVec3d::FromStartEnd (center, point1);


    dot00 = vector0.DotProduct (vector0);//vector0.DotProduct (vector0);
    dot01 = vector0.DotProduct (vector1);

    disc = dot00 * dot00 - dot01 * dot01;
    if (disc <= 0.0)
        return false;
    root = sqrt (disc);
    /* dot00 must be strictly nonzero, and this division and acos are quite safe: */
    if (    !DoubleOps::SafeDivide (cc, dot01, dot00, 1.0)
        ||  !DoubleOps::SafeDivide (alpha0, dot00, root, 1.0)
        ||  !DoubleOps::SafeDivide (alpha1, dot01, root, 1.0))
        return false;

    start = 0.0;
    sweep = acos (cc);
    /* sine could be calculated as sqrt (1-cc).  */
    ss = sin (sweep);
    if (!DoubleOps::SafeDivide (dss, 1.0, ss, 1.0))
        return false;

    /* vector0, vector1, and the 90 degree vector are related by the ellipse definition
        vector1 = vector0 * cos(sweep) + vector90 * sin(sweep).
        Rearrange for vector90 in terms of vector0 and vector1:
            vector90 = (vector1 - cos(sweep) * vector0) / sin(sweep)
        We know how to compute cos(sweep) = vector0.vector1 / vector0.vector0,
            and sin^2 = 1 - cos^2.  Inserting these dot products into
                 the vector90 expression and simplifying, we get
            vector90 =
                       (vector0.vector0 * vector1 - vector0.vector1 * vector0 )
                    /  sqrt (vector0.vector0 - vector0.vector1)
    */
    vector90 = vector1 * alpha0 - vector0 * alpha1;

    return DoubleOps::SafeDivide (axisRatio,
                vector0.Magnitude (),
                vector90.Magnitude (),
                1.0);

    }

/*-----------------------------------------------------------------*//**
@description Return an array of up to 4 points where a ray has closest approach to an ellipse.
@param [in] pEllipse  ellipse to search
@param [out]pEllipseAngleBuffer  array (allocated by caller) to hold 4 angles on ellipse
@param [out]pRayFractionBuffer  array (allocated by caller) to hold 4 fractions on ray
@param [out]ellipsePointBuffer  array (allocated by caller) to hold 4 ellipse points
@param [out]rayPointBuffer  array (allocated by caller) to hold 4 ray points
@param [in] ray  ray to search
@return number of approach points computed.
@group "DEllipse3d Closest Point"
@bsimethod                                                      EarlinLutz      12/97
+----------------------------------------------------------------------*/
int DEllipse3d::ClosestApproach
(

double          *pEllipseAngleBuffer,
double          *pRayFractionBuffer,
DPoint3dP ellipsePointBuffer,
DPoint3dP rayPointBuffer,
DRay3dCR ray

) const
    {
    return bsiDEllipse3d_closestApproachDRay3d (this, pEllipseAngleBuffer, pRayFractionBuffer, ellipsePointBuffer, rayPointBuffer, (&ray));
    }


/*-----------------------------------------------------------------*//**
@description Fill in ellipse data from data fields in DGN 3d ellipse element.
@param [out]pEllipse  initialized DEllipse3d
@param [in] centerIN  center of ellipse.
@param [in] directionX  vector in the x axis direction.  This is scaled by rX. (It is NOT normalized before
                scaling.  In common use, it will be a unit vector.)
@param [in] directionY  vector in the y axis direction.  This is scaled by rY. (It is NOT normalized before
                scaling.  In common use, it will be a unit vector.)
@param [in] rX  scale factor (usually a true distance) for x direction.
@param [in] rY  scale factor (usually a true distance) for y direction.
@param [in] startAngle start angle
@param [in] sweepAngle sweep angle
@group "DEllipse3d Initialization"
@bsimethod                                                     EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DEllipse3d::InitFromDGNFields3d
(
DPoint3dCR centerIN,
DVec3dCR directionX,
DVec3dCR directionY,
double          rX,
double          rY,
double          startAngle,
double          sweepAngle
)
    {
    bsiDEllipse3d_initFromDGNFields3d (this, (&centerIN),NULL,(&directionX),(&directionY),rX,rY,&startAngle, &sweepAngle);
    }



/*-----------------------------------------------------------------*//**
@description Fill in ellipse data from data fields in DGN 3d ellipse element.
@param [out]pEllipse  initialized DEllipse3d
@param [in] centerIN  center of ellipse.
@param [in] pQuatWXYZ  array of 4 doubles (ordered w,x,y,z) with quaternion for orthogonal frame.
@param [in] rX  scale factor (usually a true distance) for x direction.
@param [in] rY  scale factor (usually a true distance) for y direction.
@param [in] startAngle start angle
@param [in] sweepAngle sweep angle
@group "DEllipse3d Initialization"
@bsimethod                                                     EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DEllipse3d::InitFromDGNFields3d
(
DPoint3dCR centerIN,
double const*   pQuatWXYZ,
double          rX,
double          rY,
double          startAngle,
double          sweepAngle
)
    {
    bsiDEllipse3d_initFromDGNFields3d (this, (&centerIN),pQuatWXYZ,NULL,NULL,rX,rY,&startAngle, &sweepAngle);
    }


/*-----------------------------------------------------------------*//**
@description Fill in ellipse data from data fields in DGN 2d ellipse element.
@param [out]pEllipse  initialized DEllipse3d
@param [in] centerIN  center of ellipse.
@param [in] direction0  ellipse x axis direction.
@param [in] rX  scale factor for ellipse x direction.
@param [in] rY  scale factor for ellipse y direction.
@param [in] startAngle  optional start angle.
@param [in] sweepAngle  optional sweep angle.
@param [in] zDepth  z value for ellipse.
@group "DEllipse3d Initialization"
@bsimethod                                                     EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DEllipse3d::InitFromDGNFields2d
(

DPoint2dCR centerIN,
DVec2dCR direction0,
double          rX,
double          rY,
double          startAngle,
double          sweepAngle,
double          zDepth

)
    {
    bsiDEllipse3d_initFromDGNFields2d (this, (&centerIN),NULL,(&direction0),rX,rY, &startAngle, &sweepAngle, zDepth);
    }


/*-----------------------------------------------------------------*//**
@description Fill in ellipse data from data fields in DGN 2d ellipse element.
@param [out]pEllipse  initialized DEllipse3d
@param [in] centerIN  center of ellipse.
@param [in] xAngle  angle from global x axis to local x axis.
@param [in] rX  scale factor for ellipse x direction.
@param [in] rY  scale factor for ellipse y direction.
@param [in] startAngle  optional start angle.
@param [in] sweepAngle  optional sweep angle.
@param [in] zDepth  z value for ellipse.
@group "DEllipse3d Initialization"
@bsimethod                                                     EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DEllipse3d::InitFromDGNFields2d
(

DPoint2dCR centerIN,
double          xAngle,
double          rX,
double          rY,
double          startAngle,
double          sweepAngle,
double          zDepth

)
    {
    bsiDEllipse3d_initFromDGNFields2d (this, (&centerIN),&xAngle,NULL,rX,rY, &startAngle, &sweepAngle, zDepth);
    }


/*-----------------------------------------------------------------*//**
@description Fill in ellipse data from data fields in DGN 3d ellipse element.
@param [in] pEllipse  initialized DEllipse3d
@param [out] centerOUT  center of ellipse.
@param [out] pQuatWXYZ  quaternion for orthogonal frame.
            As per DGN convention, ordered WXYZ.
            If this is NULL,
           major and minor directions must be supplied as pDirection0 and pDirection90;
@param [out] directionX  unit vector in ellipse x direction.
@param [out] directionY  unit vector in ellipse y direction.
@param [out] rx  scale factor (usually a true distance) for x direction.
@param [out] ry  scale factor (usually a true distance) for y direction.
@param [out] startAngle  start angle.
@param [out] sweepAngle sweep angle.
@group "DEllipse3d Initialization"
@bsimethod                                                     EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DEllipse3d::GetDGNFields3d
(

DPoint3dR centerOUT,
double *    pQuatWXYZ,
DVec3dR     directionX,
DVec3dR     directionY,
double      &rx,
double      &ry,
double      &startAngle,
double      &sweepAngle

) const
    {
    bsiDEllipse3d_getDGNFields3d (this, (&centerOUT),pQuatWXYZ,(&directionX),(&directionY),&rx,&ry,&startAngle,&sweepAngle);
    }


/*-----------------------------------------------------------------*//**
@group "DEllipse3d Initialization"
@bsimethod                                                     EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DEllipse3d::GetDGNFields2d
(

DPoint2dR centerIN,
double          &xAngle,
DVec2dR         direction0,
double          &rx,
double          &ry,
double          &startAngle,
double          &sweepAngle

) const
    {
    bsiDEllipse3d_getDGNFields2d (this, (&centerIN),&xAngle,(&direction0),&rx,&ry,&startAngle,&sweepAngle);
    }


/*-----------------------------------------------------------------*//**
@param [in] ellipse  ellispe
@param [out]localToGlobal  coordinate frame with origin at lower right of local range.
@param [out]globalToLocal  transformation from world to local
@param [out]range  ellipse range in the local coordinates.
@group "DEllipse3d Queries"
@bsimethod                                                     EarlinLutz      03/09
+----------------------------------------------------------------------*/
bool DEllipse3d::AlignedRange
(
TransformR localToGlobal,
TransformR globalToLocal,
DRange3dR range
) const
    {
    return bsiDEllipse3d_alignedRange (this, (&localToGlobal), (&globalToLocal), (&range)) ? true : false;
    }


/*-----------------------------------------------------------------*//**
@param [in] ellipse  ellipse
@return largest (absolute) coordinate or vector component.
@bsimethod                                     BentleySystems 08/09
+----------------------------------------------------------------------*/
double DEllipse3d::MaxAbs
(
) const
    {
    double a = fabs (center.x);
    double b;
    if ((b  = fabs (center.y)) > a)
        a = b;
    if ((b  = fabs (center.z)) > a)
        a = b;

    if ((b  = fabs (vector0.z)) > a)
        a = b;
    if ((b  = fabs (vector0.z)) > a)
        a = b;
    if ((b  = fabs (vector0.z)) > a)
        a = b;

    if ((b  = fabs (vector90.z)) > a)
        a = b;
    if ((b  = fabs (vector90.z)) > a)
        a = b;
    if ((b  = fabs (vector90.z)) > a)
        a = b;
    return a;
    }



#ifndef SmallGeomLib
/*---------------------------------------------------------------------------------**//**
@description Compute projection
+--------------------------------------------------------------------------------------*/
bool DEllipse3d::ClosestPointBoundedXY
(
DPoint3dR   closePoint,
double&     closeFraction,
double&     distanceXY,
DPoint3dCR  spacePoint,
DMatrix4dCP worldToLocal
) const
    {
    return ClosestPointBoundedXY (closePoint, closeFraction, distanceXY, spacePoint, worldToLocal, false, false);
    }



/*---------------------------------------------------------------------------------**//**
@description Compute projection
+--------------------------------------------------------------------------------------*/
bool DEllipse3d::ClosestPointBoundedXY
(
DPoint3dR   closePoint,
double&     closeFraction,
double&     distanceXY,
DPoint3dCR  spacePoint,
DMatrix4dCP worldToLocal,
bool extend0,
bool extend1
) const
    {
    DConic4d conic;
    DPoint4d spacePoint4dLocal;
    DPoint3d spacePointLocal3d;
    DPoint3d curvePointLocal3d;

    bsiDConic4d_initFromDEllipse3d (&conic, this);
    if (worldToLocal != NULL)
        {
        bsiDConic4d_applyDMatrix4d (&conic, worldToLocal, &conic);
        spacePoint4dLocal = DPoint4d::FromMultiply (worldToLocal, spacePoint);
        }
    else
        spacePoint4dLocal.Init (spacePoint, 1.0);

    double radians, distanceSquaredXY;
    if (spacePoint4dLocal.GetProjectedXYZ (spacePointLocal3d)
        && bsiDConic4d_closestPointXYBounded (&conic, &radians, &distanceSquaredXY, &curvePointLocal3d, &spacePointLocal3d, extend0 || extend1))
        {
        Evaluate (closePoint, radians);
        closeFraction = Angle::NormalizeToSweep (radians, start, sweep, extend0, extend1);
        distanceXY = sqrt (distanceSquaredXY);
        return true;
        }

    closeFraction = 0.0;
    this->FractionParameterToPoint (closePoint, 0.0);
    spacePoint.DistanceXY (closePoint, worldToLocal, distanceXY);
    return false;
    }
#endif


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void DEllipse3d::WireCentroid
(
double     &length,
DPoint3dR   centroid,
double fraction0,
double fraction1
) const
    {
    DEllipse3d partialEllipse = *this;
    partialEllipse.start = start + fraction0 * sweep;
    partialEllipse.sweep = sweep * (fraction1 - fraction0);
    length = partialEllipse.ArcLength ();
    double thetaMid = partialEllipse.start + 0.5 * partialEllipse.sweep;
    double alpha = 0.5 * fabs (partialEllipse.sweep);
    double c = cos (thetaMid);
    double s = sin (thetaMid);
    double centroidFraction;
    DoubleOps::SafeDivide (centroidFraction, sin (alpha), alpha);
    centroid.SumOf (center, vector0, c * centroidFraction, vector90, s * centroidFraction);
    }

#endif

#ifdef TrigCoffUtilities

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void SquaredTrigBezierCoffs(
double y,       // +1 for positive half circle, -1 for negative
double CC[5],
double CS[5],
double SS[5],
double CW[5],
double SW[5],
double WW[5]
)
    {
    double C[3] = {1,0,-1};
    double S[3] = {0,y,0};
    double W[3] = {1,0,1};
    bsiBezier_univariateProduct (CC, 1, 1,
                C, 3, 1, 1,
                C, 3, 1, 1);
    bsiBezier_univariateProduct (CS, 1, 1,
                C, 3, 1, 1,
                S, 3, 1, 1);
    bsiBezier_univariateProduct (SS, 1, 1,
                S, 3, 1, 1,
                S, 3, 1, 1);
    bsiBezier_univariateProduct (CW, 1, 1,
                C, 3, 1, 1,
                W, 3, 1, 1);
    bsiBezier_univariateProduct (SW, 1, 1,
                S, 3, 1, 1,
                W, 3, 1, 1);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void EvaluateQuadraticBezierAngle(double t, double y, double &cosine, double &sine, double &theta)
    {
    double t0 = 1.0 - t;
    double B0 = t0 * t0;
    double B1 = 2.0 * t * t0;
    double B2 = t * t;
    double C = B0 - B2;
    double S = y * B1;
    double W = B0 + B2;
    cosine = C / W;
    sine   = S / W;
    theta = atan2 (S, C);
    }
// Return quartic bezier coffs for
//  coff_CC * cos^2 + coff_SS * sin^2
void QuarticBezierTrigExpansion
(
double y,       // +1 for positive half circle, -1 for negative
double coff_CC,
double coff_CS,
double coff_SS,
double coff_C,
double coff_S,
double coff_1,
double coff[5]
)
    {
    double CC[5], SS[5], WW[5], CS[5], SW[5], CW[5];
    SquaredTrigBezierCoffs (y, CC, SS, WW, CS, SW, CW);
    for (int i = 0; i < 5; i++)
        {
        coff[i] =
                CC[i] * coff_CC
              + SS[i] * coff_SS
              + CS[i] * coff_CS
              + SW[i] * coff_S
              + CW[i] * coff_C
              + WW[i] * coff_1;
        }    
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
int SolveTrigQuartic(
double coff_CC,
double coff_CS,
double coff_SS,
double coff_C,
double coff_S,
double coff_1,
double angles[4]
)
    {
    double bezierCoffs[5];
    double params[5];
    double yy[2] = {1.0, -1.0};
    int numRoot = 0;
    for (int k = 0; k < 2; k++)
        {
        QuarticBezierTrigExpansion (yy[k], coff_CC, coff_CS, coff_SS, coff_C, coff_S, coff_1, bezierCoffs);
        int numRootThisHalf;
        if (bsiBezier_univariateRoots (params, &numRootThisHalf, bezierCoffs, 5))
            {
            for (int i = 0; i < numRootThisHalf; i++)
                {
                double c, s, theta;
                EvaluateQuadraticBezierAngle (params[i], yy[k], c, s, theta);
                // ugly ... to avoid adding repeated endpoints, check for duplicates
                int numMatch = 0;
                for (int j = 0; j < numRoot; j++)
                    {
                    if (Angle::NearlyEqualAllowPeriodShift (theta, angles[j]))
                        numMatch++;
                    }
                if (numMatch == 0 && numRoot < 4)
                    angles[numRoot++] = theta;
                }
            }
        }
    return numRoot;
    }
#endif


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void MakeEllipseFromXPointRadiiTrigs(
bvector<DEllipse3d>&ellipses,
double a,
double b,
DPoint3dCR xPoint,
DPoint3dCR edgePoint,
double c,
double s
)
    {
    // THE TRIANGLE ... hypotenuse is xPoint to edgePoint.
    //                  horizontal leg on local x axis
    //                  vertical leg perpendicular up to edge point.
    double u = a * (1.0 - c);
    double v = b * s;
    // Ellipse space angle
    double theta = atan2 (s, c);
    // angle from hypotenuse to u axis
    double phi = atan2 (v, u);
    double dx = edgePoint.x - xPoint.x;
    double dy = edgePoint.y - xPoint.y;
    double phiEdge = atan2 (dy, dx);
    double phiAxis = phiEdge - phi;
    double c1 = cos (phiAxis);
    double s1 = sin (phiAxis);
    DEllipse3d ellipse =
            DEllipse3d::From (
                xPoint.x + a * c1, xPoint.y + a * s1, xPoint.z,
                -a * c1, -a * s1, 0.0,
                b * s1,  -b * c1, 0.0,
                0.0, -theta
                );
    ellipses.push_back (ellipse);
    }

// static !!
void DEllipse3d::Construct_XRadius_YRadius_XPoint_EdgePoint
(
bvector<DEllipse3d>&ellipses,
double a,
double b,
DPoint3dCR xPoint,
DPoint3dCR edgePoint
)
    {
    // In some rotated coordinate system, the u and v coordinates of the x point andedge point are
    // xPoint:    (a,0)
    // edgePoint: (a cos(theta), b sin (theta))
    // and the x coordinate of the x axis point "under the edge point" is a(1-cos(theta))
    // In the right triangle (with d = distance from x point to edge point)
    //    (a(1-cos))^2 + (b sin)^2 = d^2
    //    (aa (1-cos)^2 + bb (1 - cos^2) = dd
    //     aa + bb  - dd - 2 aa cos + (aa-bb) cos^2 = 0
    //   Solve for cos (2 values)
    //   Test each sine (positive, negative)
    double dd = xPoint.DistanceSquaredXY (edgePoint);
    double aa = a * a;
    double bb = b * b;
    double c0 = aa + bb - dd;
    double c1 = - aa;
    double c2 = aa - bb;
    double discriminant = c1 * c1 - c0 * c2;
    ellipses.clear ();
    double cosines[2];
    double c;
    int numCosine = 0;
    if (DoubleOps::AlmostEqual (aa,bb)
            || DoubleOps::AlmostEqual (c1 * c1, c0 * c2))
        {
        if (DoubleOps::SafeDivide (c, c0, -2.0 * c1, 0.0))
            cosines[numCosine++] = c;
        }
    else if (discriminant > 0.0)
        {
        double q = sqrt (discriminant);
        cosines[numCosine++] = (-c1 - q) / c2;
        cosines[numCosine++] = (-c1 + q) / c2;
        }

    for (int i = 0; i < numCosine; i++)
        {
        double c = cosines[i];
        if (fabs (c) <= 1.0)
            {
            double s = sqrt (1.0 - c * c);
            MakeEllipseFromXPointRadiiTrigs (ellipses, a, b, xPoint, edgePoint, c, s);
            MakeEllipseFromXPointRadiiTrigs (ellipses, a, b, xPoint, edgePoint, c, -s);
            }
        }
    }

//static 
static bool TryConstruct_EdgePoint_XPoint_EdgePoint_XAngle_go
(
DEllipse3dR result,
DPoint3dCR edgePoint0,
DPoint3dCR xPoint,
DPoint3dCR edgePoint1,
double xAngle,
bool reverseAngle
)
    {
    result.Init (0,0,0,
                  1,0,0,
                  0,1,0,
                  0.0, Angle::TwoPi ());
    double c = cos (xAngle);
    double s = sin (xAngle);
    DVec3d vectorU = DVec3d::From ( c, s, 0);
    DVec3d vectorV = DVec3d::From (-s, c, 0);
    if (reverseAngle)
        {
        vectorU.Negate ();
        vectorV.Negate ();
        }
    
    DVec3d globalVector0 = DVec3d::FromStartEnd (xPoint, edgePoint0);
    DVec3d globalVector1 = DVec3d::FromStartEnd (xPoint, edgePoint1);
    DVec3d vector0 = DVec3d::From (globalVector0.DotProduct (vectorU), globalVector0.DotProduct (vectorV), 0.0);
    DVec3d vector1 = DVec3d::From (globalVector1.DotProduct (vectorU), globalVector1.DotProduct (vectorV), 0.0);
// Local coordinates have origin at xPoint.
// a and b are to be determined.
// Center is at (-a, 0)
// At (local) point0  ((x0 + a)^2/aa + y0^2/bb = 1  ===> x0^2 + 2 x0 a + aa + y0^2 (aa/bb) = aa ===> 2 x0 a + y0^2 (aa/bb) = - x0^2
// At (local) point1  ((x1 + a)^2/aa + y1^2/bb = 1  ===> x1^2 + 2 x1 a + aa + y1^2 (aa/bb) = aa ===> 2 x1 a + y1^2 (aa/bb) = - x1^2
// This is two linear equations in the two unknowns (a, lambdaSq) where lambdaSq = aa/bb.
    double a, b, lambdaSq;
    double x0sq = vector0.x * vector0.x;
    double y0sq = vector0.y * vector0.y;
    double x1sq = vector1.x * vector1.x;
    double y1sq = vector1.y * vector1.y;

    if (bsiSVD_solve2x2 (&a, &lambdaSq,
            2.0 * vector0.x,  y0sq,
            2.0 * vector1.x,  y1sq,
            -x0sq, -x1sq)
        && lambdaSq > 0.0
        )
        {
        b = a / sqrt (lambdaSq);
        double c0, s0, c1, s1;
        if (   DoubleOps::SafeDivideParameter (c0, vector0.x + a, a, 0.0)
            && DoubleOps::SafeDivideParameter (s0, vector0.y, b, 0.0)
            && DoubleOps::SafeDivideParameter (c1, vector1.x + a, a, 0.0)
            && DoubleOps::SafeDivideParameter (s1, vector1.y, b, 0.0))
            {
            result.center = DPoint3d::FromSumOf (xPoint, vectorU, -a);
            result.vector0 = DVec3d::FromScale (vectorU, a);
            result.vector90 = DVec3d::FromScale (vectorV, b);
            double theta0 = atan2 (s0, c0);
            double theta1 = atan2 (s1, c1);
            result.start = theta0;
            result.sweep = theta1 - theta0;
            if (theta0 * theta1 > 0.0)
                result.sweep = Angle::ReverseComplement (result.sweep);
            return true;
            }
        }
    return false;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool DEllipse3d::TryConstruct_EdgePoint_XPoint_EdgePoint_XAngle
(
DEllipse3dR result,
DPoint3dCR edgePoint0,
DPoint3dCR xPoint,
DPoint3dCR edgePoint1,
double xAngle
)
    {
    return TryConstruct_EdgePoint_XPoint_EdgePoint_XAngle_go (result, edgePoint0, xPoint, edgePoint1, xAngle, false)
        || TryConstruct_EdgePoint_XPoint_EdgePoint_XAngle_go (result, edgePoint0, xPoint, edgePoint1, xAngle, true);
    }



/*--------------------------------------------------------------------------------**//**
* @bsistruct                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
struct Function_xeea : FunctionRRToRR
{
DVec3d m_vector0;
DVec3d m_vector1;
double m_a;
double m_d0sq;
double m_d1sq;
double m_scaleFactor;
Function_xeea (DPoint3dCR xPoint, DPoint3dCR edgePoint0, DPoint3dCR edgePoint1, double a)
    {
    m_vector0 = DVec3d::FromStartEnd (xPoint, edgePoint0);
    m_vector1 = DVec3d::FromStartEnd (xPoint, edgePoint1);
    m_a = a;
    m_d0sq = m_vector0.MagnitudeSquaredXY ();
    m_d1sq = m_vector1.MagnitudeSquaredXY ();
    m_scaleFactor = 1.0 / (a * a * a * a);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void EvaluateX0X1(double theta, double &x0, double &x1, double &x0sq, double &x1sq)
    {
    DVec3d xVector = DVec3d::From (cos(theta), sin(theta), 0.0);
    x0 = m_vector0.DotProduct (xVector);
    x1 = m_vector1.DotProduct (xVector);
    x0sq = x0 * x0;
    x1sq = x1 * x1;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
double Evaluate_RtoR(double theta)
    {
    double x0, x1, x0sq, x1sq;
    EvaluateX0X1 (theta, x0, x1, x0sq, x1sq);
    //double lambda0SqNeg = (x0sq + 2.0 * m_a * x0) /(m_d0sq - x0sq);
    //double lambda1SqNeg = (x1sq + 2.0 * m_a * x1) /(m_d1sq - x1sq);
    return m_scaleFactor * (
            (x0sq + 2.0 * m_a * x0) * (m_d1sq - x1sq)
        -   (x1sq + 2.0 * m_a * x1) * (m_d0sq - x0sq)
            );
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool EvaluateRRToRR(double theta0, double theta1, double &f0, double &f1) override 
    {
    f0 = Evaluate_RtoR (theta0);
    double b;
    TryEvaluateB (theta0, b);
    f1 = theta1;
    return true;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool TryEvaluateB(double theta, double &b)
    {
    double x0, x1, x0sq, x1sq;
    EvaluateX0X1 (theta, x0, x1, x0sq, x1sq);
    double lambdaSq0 = (x0sq + 2.0 * m_a * x0) / (m_d0sq - x0sq);
    //double lambdaSq1 = (x1sq + 2.0 * m_a * x1) / (m_d1sq - x1sq);
    if (lambdaSq0 < 0.0)
        {
        b = sqrt (- m_a * m_a / lambdaSq0);
        return true;
        }
    return false;
    }
};

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void DEllipse3d::Construct_XPoint_EdgePoint_EdgePoint_XRadius (
bvector<DEllipse3d> &ellipse,
DPoint3dCR xPoint,
DPoint3dCR edgePoint0,
DPoint3dCR edgePoint1,
double a
)
    {
    double abstol = Angle::SmallAngle ();
    ellipse.clear ();
    Function_xeea F (xPoint, edgePoint0, edgePoint1, a);
    bvector<double> function;
    bvector<double> theta;
    int numSample = 180;
    double dtheta = Angle::TwoPi () / numSample;
    for (int i = 0; i <= numSample; i++)
        {
        theta.push_back (i * dtheta);
        function.push_back (F.Evaluate_RtoR(theta.back ()));
        }
    for (int i = 0; i < numSample; i++)
        {
        if (function[i+1] * function[i] <= 0.0)
            {
            NewtonIterationsRRToRR newton (abstol);
            double theta0 = 0.5 * (theta[i] + theta[i+1]);
            double theta1 = 0.0;
            double b;
            if (newton.RunApproximateNewton (theta0, theta1, F, dtheta, 1000.0)
                && F.TryEvaluateB (theta0, b))
                {
                DVec3d vectorU = DVec3d::From (cos(theta0), sin(theta0), 0.0);
                DVec3d vectorV = DVec3d::From (-sin(theta0), cos(theta0), 0.0);
                DPoint3d center = DPoint3d::FromSumOf (xPoint, vectorU, -a);
                DVec3d vector0, vector90;
                vector0.Scale (vectorU, a);
                vector90.Scale (vectorV, b);
                ellipse.push_back (DEllipse3d::FromVectors (
                        center, vector0, vector90, 0.0, Angle::TwoPi ()));
                        
                //printf ("root angle %.16g\n", theta0);
                }
            }
        }
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      01/2014
+--------------------------------------------------------------------------------------*/
void DEllipse3d::Construct_Point_Direction_TangentXY
(
bvector<DEllipse3d> &ellipse,
bvector<double> &fractionB,
DPoint3dCR pointA,
DVec3dCR   directionA,
DRay3d     rayB
)
    {
    ellipse.clear ();
    DVec3d unitU, unitV, vectorW;
    vectorW.DifferenceOf (rayB.origin, pointA);
    unitU.UnitPerpendicularXY (directionA);
    unitV.UnitPerpendicularXY (rayB.direction);    
    vectorW.z = unitU.z = unitV.z = 0.0;
//   center is    C = A + s Uo
//   center distance from line is (C-B).V
//   all are unit vectors, so equate (+/-)s and the center distance:
//      (+/-)s = (A + sU -B).V
//       (B-A).V = s(U.V -+ 1)
    double e = vectorW.DotProductXY (unitV);
    double dotUV = unitU.DotProductXY (unitV);
    double d[2] = {dotUV + 1.0, dotUV - 1.0};
    for (int i = 0; i < 2; i++)
        {
        double s;
        if (DoubleOps::SafeDivide (s, e, d[i], 0.0))
            {
            DPoint3d center = DPoint3d::FromSumOf (pointA, unitU, s);
            DVec3d   vector0 = DVec3d::FromScale (unitU, -s);
            DVec3d   vector90 = DVec3d::From (-vector0.y, vector0.x, 0.0);
            DPoint3d pointD;
            double fractionD;
            if (rayB.ProjectPointUnbounded (pointD, fractionD, center))
                {
                DVec3d vectorR = DVec3d::FromStartEnd (center, pointD);
                vectorR.z = 0.0;
                double sweep = atan2 (vector90.DotProduct (vectorR), vector0.DotProduct (vectorR));
                ellipse.push_back (DEllipse3d::FromVectors (center, vector0, vector90, 0.0, sweep));
                fractionB.push_back (fractionD);
                }
            }
        }
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      06/2014
+--------------------------------------------------------------------------------------*/
bool DEllipse3d::IsAlmostEqual (DEllipse3dCR other, double tolerance) const
    {
    return DPoint3dOps::AlmostEqual (center, other.center, tolerance)
        && DVec3dOps::AlmostEqual (vector0, other.vector0, tolerance)
        && DVec3dOps::AlmostEqual (vector90, other.vector90, tolerance)
        && Angle::NearlyEqualAllowPeriodShift (start, other.start)
        && Angle::NearlyEqualAllowPeriodShift (sweep, other.sweep)
        ;
    }

//===========================================================================
DEllipse3d DEllipse3d::FromCenterRadiusXY (DPoint3dCR center, double radius)
    {
    return DEllipse3d::From (center.x, center.y, center.z,
                radius,0,0,
                0,radius,0,
                0.0,
                Angle::TwoPi ());
    }

#define verifyTrig_not
#ifdef verifyTrig
double check (double a, double b)
    {
    return a - b;
    }
#endif

//===========================================================================
ValidatedDEllipse3d DEllipse3d::FromFilletInCorner
(
DPoint3dCR pointA,  //!< [in] point "before" the filleted corner
DPoint3dCR pointB,  //!< [in] corner to be filleted away
DPoint3dCR pointC,  //!< [in] point "after" the filleted corner.
double radius
)
    {
    return DEllipse3d::FromFilletCommonPointAndRaysToTangency (
            pointB,
            pointA - pointB,
            pointC - pointB,
            radius
            );
    }

//===========================================================================
ValidatedDEllipse3d DEllipse3d::FromFilletInBoundedCorner
(
DPoint3dCR pointA,  //!< [in] point "before" the filleted corner
DPoint3dCR pointB,  //!< [in] corner to be filleted away
DPoint3dCR pointC  //!< [in] point "after" the filleted corner.
)
    {
    // get a reference radius ....
    double dA = pointB.Distance (pointA);
    double dB = pointB.Distance(pointC);
    double radius0 = DoubleOps::Min(dA, dB);
    auto fillet0 = DEllipse3d::FromFilletCommonPointAndRaysToTangency(
        pointB,
        pointA - pointB,
        pointC - pointB,
        radius0
        );
    auto tangencyA = fillet0.Value().FractionToPoint(0.0);
    auto tangencyC = fillet0.Value().FractionToPoint(1.0);
    auto fractionA = DoubleOps::ValidatedDivide(pointB.Distance(tangencyA), dA);
    auto fractionC = DoubleOps::ValidatedDivide(pointB.Distance(tangencyC), dB);
    if (fractionA.IsValid() && fractionC.IsValid())
        {
        // EDL May 2018 This was multiply, wrong final radius
        double radius = radius0 / DoubleOps::Min(fractionA.Value(), fractionC.Value());
        return DEllipse3d::FromFilletCommonPointAndRaysToTangency
            (
            pointB,
            pointA - pointB,
            pointC - pointB,
            radius
            );
        }
    fillet0.SetIsValid (false);
    return fillet0;
    }

//===========================================================================
ValidatedDEllipse3d DEllipse3d::FromFilletCommonPointAndRaysToTangency
(
DPoint3dCR commonPoint, //!< [in] intersection point of the filleted rays.
DVec3dCR vectorA, //!< [in]  vector from intersection towards tanency point on first ray.
DVec3dCR vectorB, //!< [in]  vector from intersection towards tanency point on first ray.
double radius     //! [in] circle radius.
)
    {
    DEllipse3d ellipse = DEllipse3d::FromCenterRadiusXY (commonPoint, radius);  // default for error return ...

    auto unitA = vectorA.ValidatedNormalize ();
    auto unitC = vectorB.ValidatedNormalize ();
    if (!unitA.IsValid () || !unitC.IsValid ())
        return ValidatedDEllipse3d (ellipse, false);
    auto unitNormal = DVec3d::FromCrossProduct (unitA, unitC).ValidatedNormalize ();
    if (!unitNormal.IsValid ())
        return ValidatedDEllipse3d (ellipse, false);

    DVec3d vectorBD = unitA + unitC;
    DVec3d unitToCenter = vectorBD;
    unitToCenter.Normalize ();

    double sine = unitA.Value().CrossProductMagnitude (unitToCenter);
    double cosine = unitA.Value().DotProduct (unitToCenter);
    double sweep = 2.0 * atan2 (cosine, sine);    // Yes, cosine in numerator.  atan (co/si) = 90 - atan (si/co)
#ifdef verifyTrig
    double thetaAC = unitA.Value().AngleTo (unitC);
    double alpha = 0.5 * thetaAC;
    check (cosine, cos (alpha));
    check (sine, sin (alpha));
    check (Angle::Pi () - thetaAC, sweep);
#endif

    double centerDistance = radius / sine;
    double armDistance = centerDistance * cosine;

    ellipse.center = commonPoint + unitToCenter * centerDistance;
    DPoint3d tangentA = commonPoint + armDistance * unitA;
    //DPoint3d tangentC = commonPoint + armDistance * unitC;
    ellipse.vector0 = DVec3d::FromStartEnd (ellipse.center, tangentA);
    ellipse.vector90 = DVec3d::FromCrossProduct (ellipse.vector0, unitNormal);
    ellipse.start = 0.0;
    ellipse.sweep = sweep;
    return ValidatedDEllipse3d (ellipse, true);
    }

//===========================================================================
ValidatedDEllipse3d DEllipse3d::FromFilletToOutboundTangencySegments
(
DSegment3dCR segmentA,  //!< [in] outward directed segment.
DSegment3dCR segmentB,  //!< [in] outward directed segment.
double radius     //! [in] circle radius.
)
    {
    double fractionA, fractionB;
    DPoint3d pointA, pointB;
    if (!DSegment3d::IntersectXY (fractionA, fractionB, pointA, pointB, segmentA, segmentB))
        return ValidatedDEllipse3d (DEllipse3d::FromCenterRadiusXY (segmentA.point[0], radius), false);

    return FromFilletCommonPointAndRaysToTangency (pointA,
                    DVec3d::FromStartEnd (segmentA.point[0], segmentA.point[1]),
                    DVec3d::FromStartEnd (segmentB.point[0], segmentB.point[1]),
                    radius);
    }

ValidatedDEllipse3d DEllipse3d::FromStartTangentSweepEndTangentXY
(
DPoint3dCR pointA,      //!< [in] Constraint: Start at this point.
DVec3dCR tangentA,      //!< [in] Constraint: Start tangent is parallel to this
DPoint3dCR pointB,      //!< [in] Constraint: End at this point.
DVec3dCR tangentB,     //!< [in] Constratin: End tangent is parallel to this.
double sweepRadians    //!< [in] Constraint: sweep angle for output ellipse
)
    {
// New ellipse is
//       X = C + cU + sV
// where C, U, V are to be determined.
// Start (at angle 0) has c=1, s=0
// End (at sweepRadians has (known) c1, s1
// Result will parameterize from angle 0, so C + U = A.  (Apply this a bit later to elimiante one or the other)
// Tangent at angle 0 is V.   So V is some factor times tangentA, V = alpha * tangentA
// This leaves three unknowns:  Cx, Cy, alpha.
//
// Passthrough condition at B is:    B = (A-U) + c1 U + s1 *alpha *tangentA
//                                    B-A = (c1-1) U + s1 * alpha * tangentA
//   (and that has x,y parts)
// tangent condtion at B is
//         (-s1 * U + c1 * alpha * tangentA) CROSS tangentB = 0
//         (-s1 * U CROSS tangentB + alpha * (c1 * tangentA CROSS tangentB) = 0
//         (-s1 * (U.x * tangentB.y - U.y * tangentB.x) + alpha * c1 * tangentA CROSS tangentB) = 0
// 
// 
// BINGO!!! 3 linear equations.
    double c1 = cos (sweepRadians);
    double s1 = sin (sweepRadians);
    double v1 = c1 - 1.0;
    auto matrix = RotMatrix::FromRowValues
        //   Ux               Uy                   alpha
        (   v1,               0.0,                 tangentA.x * s1,
            0.0,              v1,                  tangentA.y * s1,
            -s1 * tangentB.y, s1 * tangentB.x,     c1 * tangentA.CrossProductXY (tangentB)
        );

    auto  rightHandSide = DPoint3d::From (
                pointB.x - pointA.x,
                pointB.y - pointA.y,
                0.0
                );

    DPoint3d solution;
    if (matrix.Solve (solution, rightHandSide))
        {
        auto U = DVec3d::From (solution.x, solution.y, 0.0);
        double alpha = solution.z;
        auto V = DVec3d::From (alpha * tangentA.x, alpha * tangentA.y, 0.0);
        auto C = pointA - U;
        return ValidatedDEllipse3d (DEllipse3d::FromVectors (C, U, V, 0.0, sweepRadians), true);
        }
    return ValidatedDEllipse3d ();
    }



void DEllipse3d::Construct_Point_Direction_TangentToCircleXY
(
bvector<DEllipse3d> &allCircles,
DPoint3dCR pointA,
DVec3dCR   directionA,
DPoint3dCR circleBCenter,
double     circleBRadius
)
    {
#define DirectSolution
#ifdef DirectSolution
// New ellipse is
//       X = C + cos(theta) U + sin(theta) V, with U and V perpendicular and of same length.
// where C, U, V are to be determined.
// start point (theta==0) is at pointA.  V is in the direction of the start tangent.
// Construct unitU perpendicular to V.
// move by distance r along U until distance from pointA is (r+rB) where rB = radius of circle centered at B
// C = A + r * unitU
// (C-B) DOT (C-B) = (r +- rB)^2
// (A + r * unitU - B) DOT (A + r * unitU -B) = (r +- rB) ^2
// Let E = A-B
// (E + r * unitU) DOT (E + r * unitU) = (r + rB) ^2
//  E DOT E + 2 r unitU DOT E + r^2 = r^2 +- 2 r rB + rB ^2
//  E DOT E + 2 r unitU DOT E = +- 2r rB + rB^2
//    r = (EE - rB^2) / (2  ( unitU DOT E -+ rB)
// 
    allCircles.clear ();
    DVec3d perpVector = DVec3d::From (directionA.y, -directionA.x, 0.0);
    auto unitU = perpVector.ValidatedNormalize ();
    if (unitU.IsValid ())
        {
        DVec3d vectorE = circleBCenter - pointA;
        double ee = vectorE.DotProduct (vectorE);
        double eu = unitU.Value ().DotProduct (vectorE);
        double numerator = ee - circleBRadius * circleBRadius;

        double denominator0 = 2.0 * (eu + circleBRadius);
        double denominator1 = 2.0 * (eu - circleBRadius);
        ValidatedDouble radii[2];
        radii[0] = DoubleOps::ValidatedDivide (numerator, denominator0);
        radii[1] = DoubleOps::ValidatedDivide (numerator, denominator1);
        for (int i = 0; i < 2; i++)
            {
            if (radii[i].IsValid ())
                {
                DEllipse3d ellipse = FromCenter_StartPoint_EndTarget_StartTangentBias_CircularXY (
                        pointA + radii[i] * unitU.Value (),
                        pointA,
                        circleBCenter,
                        directionA);
                allCircles.push_back (ellipse);
                }
            }        
        }
#else

#define MAX_CIRCLE_CCL 8
    DPoint3d centers[MAX_CIRCLE_CCL];
    double radii[MAX_CIRCLE_CCL];
    int numCandidate;
    DPoint3d centerIn[2] = {pointA, circleBCenter};
    double   radiusIn[2] = {0.0, circleBRadius};
    // ugh -- circleTTT doesn't use const pointers ...
    DPoint3d myPointA = pointA;
    DVec3d   myVectorA = directionA;
    bsiGeom_circleTTTCircleCircleLineConstruction
        (
        centers, radii, &numCandidate, MAX_CIRCLE_CCL,
        centerIn, radiusIn,
        &myPointA, &myVectorA);
    for (int i = 0; i < numCandidate; i++)
        {
        DEllipse3d ellipse = FromCenter_StartPoint_EndTarget_StartTangentBias_CircularXY (centers[i], pointA, circleBCenter, directionA);
        allCircles.push_back (ellipse);
        }
#endif
    }

DEllipse3d DEllipse3d::FromCenter_StartPoint_EndTarget_StartTangentBias_CircularXY (DPoint3dCR center, DPoint3d startPoint, DPoint3dCR endTarget, DVec3dCR startTangentBias)
    {
    DEllipse3d ellipse;
    ellipse.center = center;
    ellipse.vector0 = startPoint- ellipse.center;
    ellipse.vector90 = DVec3d::FromCCWPerpendicularXY (ellipse.vector0);
    ellipse.start = 0.0;
    auto vectorW = endTarget - ellipse.center;
    ellipse.sweep = ellipse.vector0.AngleToXY (vectorW);
    // The ellipse is complete, but possibly the wrong part of the circle ..
    if (ellipse.sweep * ellipse.vector90.DotProductXY (startTangentBias) < 0.0)
        ellipse.ComplementSweep ();
    return ellipse;
    }

ValidatedDEllipse3d DEllipse3d::FromEllipseSweptToPlane
(
DEllipse3dCR spaceEllipse,
DPlane3dCR targetPlane,
DVec3dCR sweepDirection
)
    {
    auto sweepTransform = Transform::FromSweepToPlane (sweepDirection, targetPlane);
    if (sweepTransform.IsValid ())
        {
        DEllipse3d targetEllipse;
        sweepTransform.Value ().Multiply (targetEllipse, spaceEllipse);
        return ValidatedDEllipse3d (targetEllipse, true);
        }
    return ValidatedDEllipse3d (spaceEllipse, false);
    }
//================================================================================================================================
// construct fractional coordinates of lines tangent to two circles.
bool DEllipse3d::ConstructTangentLineRatios
(
double centerToCenterDistance,  //!< [in] distance between centers
double radiusA,                 //!< [in] radius of first circle.
double radiusB,                 //!< [in] radius of second circle
bool outerTangents,             //!< [in] true for tangents from outside to ouside, false for tangents that cross between centers
DPoint2dR uvA,                  //!< [in] fractional coordinates of tangency point on circle A, for use in DPoint3d::FromInterpolateAndPerpendicularXY 
DPoint2dR uvB                  //!< [in] fractional coordinates of tangency point on circle B, for use in DPoint3d::FromInterpolateAndPerpendicularXY 
)
    {
    if (radiusA == 0.0 && radiusB == 0.0)
        {
        uvA = DPoint2d::From (0.0, 0.0);
        uvB = DPoint2d::From (1.0, 0.0);
        return true;
        }
    if (outerTangents)
        {
        auto fraction = DoubleOps::ValidatedDivideParameter (radiusB, radiusA - radiusB);
        if (fraction.IsValid ())
            {
            double distanceA = (1.0 + fraction) * centerToCenterDistance;
            double distanceB = fraction * centerToCenterDistance;
            double s = fabs (distanceA) > fabs (distanceB)
                ? DoubleOps::ValidatedDivideDistance (radiusA, distanceA, 0.0).Value ()
                : DoubleOps::ValidatedDivideDistance (radiusB, distanceB, 0.0).Value ();
            auto divD = DoubleOps::ValidatedDivideDistance (1.0, centerToCenterDistance);
            double eA = radiusA * s;    // distance centerA to projection of its tangency point
            double eB = radiusB * s;
            double qA = radiusA * radiusA - eA * eA;
            double qB = radiusB * radiusB - eB * eB;
            if (qA >= 0.0 && qB >= 0.0 && divD.IsValid () && fabs (s) <= 1.0)
                {
                double hA = sqrt (qA);
                double hB = sqrt (qB);
                uvA = DPoint2d::From (eA * divD, hA * divD);
                uvB = DPoint2d::From (1.0 + eB * divD, hB * divD);
                return true;
                }
            }
        else
            {
            // same radii ..
            auto radiusFraction = DoubleOps::ValidatedDivideParameter (radiusA, centerToCenterDistance);
            if (radiusFraction.IsValid ())
                {
                uvA = DPoint2d::From (0.0, radiusFraction);
                uvB = DPoint2d::From (1.0, radiusFraction);
                return true;
                }
            }
        }
    else
        {
        // Tangent line crosses the x axis at this fractional coordinate from centerA to centerB
        auto fraction = DoubleOps::ValidatedDivideParameter (radiusA, radiusA + radiusB);
        if (fraction.IsValid ())
            {
            double distanceA = fraction * centerToCenterDistance;
            double distanceB = centerToCenterDistance - distanceA;
            // cosecant of the angle -- divide by larger of two choices
            double s = fabs (distanceA) > fabs (distanceB)
                ? DoubleOps::ValidatedDivideDistance (radiusA, distanceA, 0.0).Value ()
                : DoubleOps::ValidatedDivideDistance (radiusB, distanceB, 0.0).Value ();
            auto divD = DoubleOps::ValidatedDivideDistance (1.0, centerToCenterDistance);
            double eA = radiusA * s;    // distance centerA to projection of its tangency point
            double eB = radiusB * s;
            double qA = radiusA * radiusA - eA * eA;
            double qB = radiusB * radiusB - eB * eB;
            if (qA >= 0.0 && qB >= 0.0 && divD.IsValid () && fabs (s) <= 1.0)
                {
                double hA = sqrt (qA);
                double hB = sqrt (qB);
                uvA = DPoint2d::From (eA * divD, hA * divD);
                uvB = DPoint2d::From (1.0 - eB * divD, - hB * divD);
                return true;
                }
            }
        }
    return false;
    }

bool DEllipse3d::Construct_ArcLineArc_PointTangentRadius_PointTangentRadiusXY
(
DPoint3dCR pointA,
DVec3dCR   tangentAin,
double     radiusA,
DPoint3dCR pointB,
DVec3dCR   tangentBin,
double     radiusB,
DEllipse3dR arcA,
DSegment3dR tangentSegment,
DEllipse3dR arcB
)
    {
    auto tangentA = tangentAin; tangentA.z = 0.0;
    auto tangentB = tangentBin; tangentB.z = 0.0;
    DVec3d unitA, unitB;
    double magA, magB;
    if (unitA.TryNormalize (tangentA, magA) && unitB.TryNormalize (tangentB, magB))
        {
        auto perpA = DVec3d::FromUnitPerpendicularXY (unitA);
        auto perpB  = DVec3d::FromUnitPerpendicularXY (unitB);
        auto centerA = pointA + radiusA * perpA;
        auto centerB = pointB + radiusB * perpB;
        centerB.z = centerA.z;
        DPoint2d uvA[4], uvB[4];
        bool ok[4];
        ok[0] = ok[1] = DEllipse3d::ConstructTangentLineRatios (centerA.Distance (centerB), fabs (radiusA), fabs (radiusB), false, uvA[0], uvB[0]);
        uvA[1] = DPoint2d::From (uvA[0].x, - uvA[0].y);
        uvB[1] = DPoint2d::From (uvB[0].x, - uvB[0].y);
        ok[2] = ok[3] = DEllipse3d::ConstructTangentLineRatios (centerA.Distance (centerB), fabs (radiusA), fabs (radiusB), true,  uvA[2], uvB[2]);
        uvA[3] = DPoint2d::From (uvA[2].x, - uvA[2].y);
        uvB[3] = DPoint2d::From (uvB[2].x, - uvB[2].y);
        // Look for a solution that has good tangent directions ...
        DVec3d segmentTangent;
        DVec3d tangentA0, tangentB0, tangentA1, tangentB1, d2A, d2B;
        DPoint3d xyzA, xyzB;
        for (int i = 0; i < 4; i++)
            {
            if (ok[i])
                {

                tangentSegment = DSegment3d::From (
                    DPoint3d::FromInterpolateAndPerpendicularXY (centerA, uvA[i].x, centerB, uvA[i].y),
                    DPoint3d::FromInterpolateAndPerpendicularXY (centerA, uvB[i].x, centerB, uvB[i].y)
                    );
                arcA = DEllipse3d::FromCenter_StartPoint_EndTarget_StartTangentBias_CircularXY (
                        centerA, pointA, tangentSegment.point[0], unitA);
                arcB = DEllipse3d::FromCenter_StartPoint_EndTarget_StartTangentBias_CircularXY (
                        centerB, pointB, tangentSegment.point[1], unitB);
                arcB.ComplementSweep ();

                segmentTangent = tangentSegment.point[1] - tangentSegment.point[0];

                arcA.FractionParameterToDerivatives (xyzA, tangentA0, d2A, 0.0);
                if (tangentA0.DotProduct (tangentA) < 0.0)
                    arcA = DEllipse3d::FromReversed (arcA);

                arcB.FractionParameterToDerivatives (xyzB, tangentB1, d2B, 1.0);
                if (tangentB1.DotProduct (tangentB) < 0.0)
                    arcB = DEllipse3d::FromReversed (arcB);

                arcA.FractionParameterToDerivatives (xyzA, tangentA1, d2A, 1.0);
                arcB.FractionParameterToDerivatives (xyzB, tangentB0, d2B, 0.0);
                double dotA = segmentTangent.DotProduct (tangentA1);
                double dotB = segmentTangent.DotProduct (tangentB0);
                if (dotA > 0.0 && dotB > 0.0)
                    return true;
                }
            }
        }
    return false;
    }

// Modify an ellipse so that
//  1) one endpoint and its tangent are preserved.
//  2) sweep angle is preserved
//  3) the other endpoint moves a specified vector distance.
// @param [out] result transformed ellipse
// @param [in] source original ellipse
// @param [in] translation translation vector to apply.
// @param [in] movingEndIndex 0 to move startpoint, 1 to move endpoint
ValidatedDEllipse3d DEllipse3d::FromEndPointTranslation(DEllipse3dCR source, DVec3dCR translation, int movingEndIndex)
    {
    DEllipse3d result = source;
    DVec3d vectorU0, vectorV0, vectorW0;
    DVec3d vectorU1, vectorV1, vectorW1;
    DPoint3d fixedPoint, movingPoint;
    double theta0 = source.start;
    double theta1 = source.start + source.sweep;
    DVec3d vector;
    if (movingEndIndex == 0)
        {
        source.Evaluate (fixedPoint, vectorU0, vector, theta1);
        source.Evaluate (movingPoint, vector, vector, theta0);
        }
    else
        {
        source.Evaluate (fixedPoint, vectorU0, vector, theta0);
        source.Evaluate (movingPoint, vector, vector, theta1);
        }
    vectorV0.DifferenceOf (movingPoint, fixedPoint);
    vectorW0.GeometricMeanCrossProduct (vectorU0, vectorV0);

    vectorU1 = vectorU0;
    vectorV1.SumOf (vectorV0, translation);
    vectorW1.GeometricMeanCrossProduct (vectorU1, vectorV1);

    Transform frame0, frame1;
    frame0.InitFromOriginAndVectors (fixedPoint, vectorU0, vectorV0, vectorW0);
    frame1.InitFromOriginAndVectors (fixedPoint, vectorU1, vectorV1, vectorW1);
    Transform inverse0;
    if (inverse0.InverseOf(frame0))
        {
        Transform product;
        product.InitProduct (frame1, inverse0);
        product.Multiply (result, source);
        return ValidatedDEllipse3d (result, true);
        }
    return ValidatedDEllipse3d (result, false);
    }


END_BENTLEY_GEOMETRY_NAMESPACE
