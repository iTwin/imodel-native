/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/funcs/rotconic.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include <Geom/MstnOnly/GeomPrivateApi.h>
#include <math.h>
#include <stdlib.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

extern void bsiDoubleArray_sort (double *, int, int);

/*----------------------------------------------------------------*//**
*    macro definitions
*
+----------------------------------------------------------------------*/
/*----------------------------------------------------------------*//**
*    Local defines
*
+----------------------------------------------------------------------*/
#define MAX_TOROIDAL_POINT 400
#define MINIMUM_ANGLE_STEP 0.05
/*----------------------------------------------------------------*//**
*    Local type definitions
*
+----------------------------------------------------------------------*/
typedef struct
    {
    const RotatedConic      *pSurface;
    SilhouetteArrayHandler  handlerFunc;
    void                    *pUserData;
    } ToroidalOutputParams;

enum
    {
    TP_Break,
    TP_PhiLimit,
    TP_ThetaLimit,
    TP_ThetaStep,
    TP_PhiStep
    };

#define TP_BOUNDARY_POINT 0x00000001

typedef struct
    {
    DPoint3d thetaPoint;
    DPoint3d phiPoint;
    double sortCoordinate;
    int mask;
    } ToroidalPoint;

typedef struct _toroidalPointArray ToroidalPointArray;

typedef StatusInt (*ToroidalOutputFunction)(ToroidalPointArray *, void *);
struct _toroidalPointArray
    {
    int numPoint;
    ToroidalPoint pointPair[MAX_TOROIDAL_POINT];
    ToroidalOutputFunction outputFunction;
    void                *pOutputData;
    } ;

typedef enum
    {
    PR_EmptyAngle,
    PR_SweepAngle,
    PR_FullCircle
    } ParamIntervalType;

typedef struct
    {
    ParamIntervalType type;
    double          start;
    double          delta;
    } ParamInterval;
typedef struct
    {
    ParamInterval param[2];
    } ParamRange;


/*----------------------------------------------------------------*//**
*    Private Global variables
*
+----------------------------------------------------------------------*/

static const DPoint4d s_point0001_4d = { 0.0, 0.0, 0.0, 1.0};

static const DPoint4d s_point1001_4d = { 1.0, 0.0, 0.0, 1.0};
static const DPoint4d s_point0101_4d = { 0.0, 1.0, 0.0, 1.0};
//static const DPoint4d s_point0011_4d = { 0.0, 0.0, 1.0, 1.0};
static const DPoint4d s_point1101_4d = { 1.0, 1.0, 0.0, 1.0};

static const DPoint4d s_point1000_4d = { 1.0, 0.0, 0.0, 0.0};
static const DPoint4d s_point0100_4d = { 0.0, 1.0, 0.0, 0.0};
static const DPoint4d s_point0010_4d = { 0.0, 0.0, 1.0, 0.0};

//static const DPoint4d s_point1011_4d = { 1.0, 0.0, 1.0, 1.0};

static const double s_toroidalQuadTol = 1.0e-12;
static const double s_toroidalFilterFraction = 0.1;  /* Filter away points closer than this fraction of angular tolerance */
static const double s_lineUnitCircleIntersectionTolerance = 1.0e-12;

/*----------------------------------------------------------------*//**
*    Public Global variables
*
+----------------------------------------------------------------------*/
/*----------------------------------------------------------------*//**
*    External variables
*
+----------------------------------------------------------------------*/

/*======================================================================+
*
*    Private Utility Routines
*
+======================================================================*/

/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            07/97
*
+----------------------------------------------------------------------*/
static void jmdlParamInterval_initFromAngleLimits

(
ParamInterval   *pParam,
double          theta0,
double          theta1
)
    {
    double sweep = theta1 - theta0;
    if (bsiTrig_isAngleFullCircle (sweep))
        {
        pParam->type = PR_FullCircle;
        pParam->start = theta0;
        pParam->delta = sweep > 0.0 ? msGeomConst_2pi : -msGeomConst_2pi;
        }
    else
        {
        pParam->type = PR_SweepAngle;
        pParam->start = theta0;
        pParam->delta = sweep;
        }
    }

/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            07/97
*
+----------------------------------------------------------------------*/
static void jmdlParamRange_initFromAngleLimits

(
ParamRange  *pRange,
double      theta0,
double      theta1,
double      phi0,
double      phi1
)
    {
    jmdlParamInterval_initFromAngleLimits (&pRange->param[0], theta0, theta1);
    jmdlParamInterval_initFromAngleLimits (&pRange->param[1], phi0, phi1);
    }

/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            07/97
*
+----------------------------------------------------------------------*/
static bool    jmdlParamInterval_containsPoint

(
const ParamInterval   *pParam,
double          value
)
    {
    switch (pParam->type)
        {
        case PR_FullCircle:
            return true;
        case PR_SweepAngle:
            return bsiTrig_angleInSweep (value, pParam->start, pParam->delta);
        case PR_EmptyAngle:
            return false;
        }
    return  false;
    }

/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            07/97
*
+----------------------------------------------------------------------*/
static double jmdlParamInterval_normalizePoint

(
const ParamInterval   *pParam,
double          value
)
    {
    switch (pParam->type)
        {
        case PR_FullCircle:
        case PR_SweepAngle:
            return pParam->start + pParam->delta * bsiTrig_normalizeAngleToSweep (value, pParam->start, pParam->delta);
        case PR_EmptyAngle:
            return value;
        }
    return  0.0;
    }
/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            07/97
*
+----------------------------------------------------------------------*/
static bool    jmdlParamRange_containsPoint

(
const ParamRange  *pRange,
double      value0,
double      value1
)
    {
    return    jmdlParamInterval_containsPoint (&pRange->param[0], value0)
           && jmdlParamInterval_containsPoint (&pRange->param[1], value1);
    }
#ifdef CompileAll
/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            07/97
*
+----------------------------------------------------------------------*/
static bool    jmdlParamRange_intervalContainsPoint

(
const ParamRange  *pRange,
int         index,
double      value
)
    {
    if (index == 0)
        return jmdlParamInterval_containsPoint (&pRange->param[0], value);
    if (index == 1)
        return jmdlParamInterval_containsPoint (&pRange->param[1], value);
    return false;
    }
#endif
/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            07/97
*
+----------------------------------------------------------------------*/
static void jmdlParamRange_normalizePoint

(
double      *pValue0,
double      *pValue1,
const ParamRange  *pRange
)
    {
    *pValue0 = jmdlParamInterval_normalizePoint (&pRange->param[0], *pValue0);
    *pValue1 = jmdlParamInterval_normalizePoint (&pRange->param[1], *pValue1);
    }

/*======================================================================+
*
*    Major Public Code Section
*
+======================================================================*/
#ifdef CompileAll
/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            08/97
*
*  true if angle is 2pi or more, or if angle is within a very large
*  tolerance (0.01) of 2pi and rotation with this radius produces a
*  point within tolerance of start point.
+----------------------------------------------------------------------*/
static bool        jmdlTrig_isSweepNearFullCircle

(
double      sweep,
double      radius,
double      distanceTolerance
)
    {
    double delta;
    static double s_grossAngleTolerance = 0.01;
    double dist;
    if (bsiTrig_isAngleFullCircle (sweep))
        return true;
    delta = fabs (sweep - msGeomConst_2pi);
    if (delta < s_grossAngleTolerance)
        {
        dist = 2.0 * radius * sin (delta * 0.5);
        if (dist <= distanceTolerance)
            return true;
        }
    return false;
    }
#endif

#ifdef CompileAll
/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            07/97
*
*  Swap the z and w components of a DPoint4d.
+----------------------------------------------------------------------*/
static void jmdlDPoint4d_swapzw

(
DPoint4dP pPoint
)
    {
    double temp = pPoint->z;
    pPoint->z = pPoint->w;
    pPoint->w = temp;
    }
#endif

#ifdef CompileAll
/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            07/97
*
*  Swap the z and w components of the conic.
+----------------------------------------------------------------------*/
static void jmdlHConic_swapzw

(
HConic  *pHConic
)
    {
    jmdlDPoint4d_swapzw (&pHConic->coordinates.center  );
    jmdlDPoint4d_swapzw (&pHConic->coordinates.vector0 );
    jmdlDPoint4d_swapzw (&pHConic->coordinates.vector90);
    }
#endif

/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            07/97
*
*  Multiply a 4d point by the matrix
*  [ c -s  0  0]
*  [ s  c  0  0]
*  [ 0  0  1  0]
*  [ 0  0  0  1]
+----------------------------------------------------------------------*/
static   void jmdlDPoint4d_rotateXY

(
DPoint4dP pOutPoint,     /* => result point */
DPoint4dCP pInPoint,      /* => original point */
double              c,              /* => cosine of angle */
double              s               /* => sine of angle */
)
    {
    double x =  c * pInPoint->x - s * pInPoint->y;
    double y =  s * pInPoint->x + c * pInPoint->y;
    pOutPoint->x = x;
    pOutPoint->y = y;
    pOutPoint->z = pInPoint->z;
    pOutPoint->w = pInPoint->w;
    }

#ifdef CompileAll
/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            07/97
*
*  Multiply a 4d point by the matrix
*  [ c -s  0]
*  [ s  c  0]
*  [ 0  0  1]
+----------------------------------------------------------------------*/
static   void jmdlVector_rotateXY

(
DPoint4dP pOutPoint,     /* => result point */
DPoint4dCP pInPoint,      /* => original point */
double              c,              /* => cosine of angle */
double              s               /* => sine of angle */
)
    {
    double x =  c * pInPoint->x - s * pInPoint->y;
    double y =  s * pInPoint->x + c * pInPoint->y;
    pOutPoint->x = x;
    pOutPoint->y = y;
    pOutPoint->z = pInPoint->z;
    }
#endif

/*------------------------------------------------------------------*//**
* Zero out a rotated conic.
* @bsihdr                               EarlinLutz          08/97
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void bsiRotatedConic_clear

(
RotatedConic        *pSurface
)
    {
    memset (pSurface, 0, sizeof (RotatedConic));
    }

/*------------------------------------------------------------------*//**
* Set the sweep angle. Used only in pre-coordinate system mode.
* @bsihdr                               EarlinLutz          08/97
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void bsiRotatedConic_setSweep

(
RotatedConic        *pSurface,
double              sweep
)
    {
    pSurface->sweep = sweep;
    }

/*------------------------------------------------------------------*//**
* Set the hoop radius.  Used only for torus type.
* @bsihdr                               EarlinLutz          08/97
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void bsiRotatedConic_setHoopRadius

(
RotatedConic        *pSurface,
double              radius
)
    {
    pSurface->hoopRadius = radius;
    }

/*------------------------------------------------------------------*//**
* (Pre)multiply the conic's coordinates by a 4x4 matrix.
* @bsihdr                               EarlinLutz          08/97
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void       bsiHConic_multiplyHMatrix

(
HConic  *pOutConic,       /* <= transformed conic */
const HConic  *pInConic,        /* => original conic */
DMatrix4dCP pMatrix             /* => matrix */
)
    {
    if (pInConic)
        bsiDEllipse4d_multiplyByHMatrix (&pOutConic->coordinates, &pInConic->coordinates, pMatrix);
    }

/*------------------------------------------------------------------*//**
* Initialize an untyped conic.
* @bsihdr                               EarlinLutz          08/97
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void bsiHConic_initNull

(
HConic          *pHConic       /* <= conic to initialize */
)
    {
    if (pHConic)
        {
        memset (&pHConic->coordinates, 0, sizeof (DEllipse4d));
        pHConic->type = HConic_Null;
        }
    }

/*------------------------------------------------------------------*//**
* Compute the range of the conic.
* @bsihdr                               EarlinLutz          08/97
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt bsiHConic_getRange

(
DRange3dP pRange,           /* <= range to fill */
const HConic        *pHConic       /* => conic to evaluate */
)
    {
    StatusInt status = ERROR;
    if (pHConic)
        {
        switch (pHConic->type)
            {
            case HConic_Line:
                {
                DPoint3d startPoint, endPoint;
                pHConic->coordinates.vector0.GetProjectedXYZ (startPoint);
                pHConic->coordinates.vector90.GetProjectedXYZ (endPoint);
                bsiDRange3d_initFrom2Points (pRange, &startPoint, &endPoint);
                status = SUCCESS;
                break;
                }
            case HConic_Ellipse:
                bsiDRange3d_init (pRange);
                bsiDEllipse4d_extendRange (pRange, &pHConic->coordinates);
                status = SUCCESS;
                break;
            case HConic_Point:
                {
                DPoint3d point;
                pHConic->coordinates.center.GetProjectedXYZ (point);
                bsiDRange3d_initFromPoint (pRange, &point);
                status = SUCCESS;
                break;
                }
            }
        }
    if (SUCCESS != status)
        bsiDRange3d_init (pRange);
    return status;
    }

/*------------------------------------------------------------------*//**
* Build planes that surround 2 concentric arcs and given altitudes.  Planes can be
* tilted.
*
* To be a positive volume, radii must be positive.
* (z0, z1 may be in either order. theta0 and theta1 may be in either
* order.
* @param pPoint <= 8 points of bounding volume
* @param pPlane <= 6 planes of bounding volume
* @param theta0 => arc start angle.
* @param theta1 => arc end angle.
* @param radius0 => first arc radius.
* @param radius1 => second arc radius.
* @param z => height of arc plane above xy plane.
* @param internalSlope => slope dz/ds for outward normal at internal points
* @param endSlope => => slope dz/ds fo outward normal at secant edge
* @param alpha0 => z parameter limit
* @param alpha1 => z parameter limit
* @param pMap   => placement transformation
* @bsihdr                               EarlinLutz          08/97
+----------------------------------------------------------------------*/
static StatusInt       jmdlRotatedConic_arcHull

(
DPoint3dP pPoint,
DPoint4dP pPlane,
double      theta0,
double      theta1,
double      radius0,
double      radius1,
double      z,
double      internalSlope,
double      endSlope,
double      alpha0,
double      alpha1,
DMap4dCP pMap
)
    {
    double theta, sweep, thetaMid, alpha;
    double c, s, c0, s0, c1, s1, cMid, sMid;
    int numPlane = 0;
    bool    computePlaneIntersections = false;
    StatusInt status = SUCCESS;
    DPoint4d testPoint;             /* Point the should be in the volume */

    static double s_radiusFactor = 0.6;
    static double s_lowSweepFactor = 0.3;
    static double s_highSweepFactor = 0.7;

    if (radius0 > radius1)
        {
        double temp = radius0;
        radius0 = radius1;
        radius1 = temp;
        }

    if (alpha0 > alpha1)
        {
        alpha = alpha0;
        alpha0 = alpha1;
        alpha1 = alpha;
        }

    if (theta0 > theta1)
        {
        theta = theta0;
        theta0 = theta1;
        theta1 = theta;
        }

    sweep = theta1 - theta0;

    bsiDPoint4d_setComponents (&pPlane[0], 0.0, 0.0,  -1.0,  alpha0);
    bsiDPoint4d_setComponents (&pPlane[1], 0.0, 0.0,   1.0, -alpha1);

    numPlane = 2;


    if (bsiTrig_isAngleFullCircle (sweep))
        {
        bsiDPoint4d_setComponents (&pPlane[5],  1.0,  0.0, 0.0, -radius1);
        bsiDPoint4d_setComponents (&pPlane[4], -1.0,  0.0, 0.0, -radius1);
        bsiDPoint4d_setComponents (&pPlane[3],  0.0,  1.0, 0.0, -radius1);
        bsiDPoint4d_setComponents (&pPlane[2],  0.0, -1.0, 0.0, -radius1);
        numPlane = 6;

        bsiDPoint4d_setComponents (&testPoint, 0.0, 0.0, 0.5 * (alpha0 + alpha1), 1.0);

        if (internalSlope == 0.0)
            {
            pPoint[0].Init ( -radius1, -radius1, alpha0);
            pPoint[1].Init (  radius1, -radius1, alpha0);
            pPoint[2].Init ( -radius1,  radius1, alpha0);
            pPoint[3].Init (  radius1,  radius1, alpha0);

            pPoint[4].Init ( -radius1, -radius1, alpha1);
            pPoint[5].Init (  radius1, -radius1, alpha1);
            pPoint[6].Init ( -radius1,  radius1, alpha1);
            pPoint[7].Init (  radius1,  radius1, alpha1);
            }
        else
            {
            computePlaneIntersections = true;
            }

        }
    else if (   radius0 < s_radiusFactor * radius1
             && fabs (sweep) > s_lowSweepFactor * msGeomConst_pi
             && fabs (sweep) < s_highSweepFactor * msGeomConst_pi
            )
        {
        /* Use end faces and two tangents to the larger arc */
        double c2, s2, c3, s3;
        double theta2 = theta0 + 0.25 * sweep;
        double theta3 = theta0 + 0.75 * sweep;
        thetaMid = 0.5 *(theta0 + theta1);
        cMid = cos (thetaMid);
        sMid = sin (thetaMid);

        /* These c,s values are all outward normals for planes. */
        c0 = sin (theta0);      /* end tangent rotated 90 CW */
        s0 = -cos (theta0);
        c1 = -sin (theta1);
        s1 =  cos (theta1);
        c2 =  cos (theta2);
        s2 =  sin (theta2);
        c3 =  cos (theta3);
        s3 =  sin (theta3);

        bsiDPoint4d_setComponents (&pPlane[2], c0, s0, 0.0, 0.0);
        bsiDPoint4d_setComponents (&pPlane[3], c3, s3, 0.0, -radius1);

        bsiDPoint4d_setComponents (&pPlane[4], c1, s1, 0.0, 0.0);
        bsiDPoint4d_setComponents (&pPlane[5], c2, s2, 0.0, -radius1);

        bsiDPoint4d_setComponents (&testPoint,
                        0.5 * radius1 * cMid,
                        0.5 * radius1 * sMid,
                        0.5 * (alpha0 + alpha1),
                        1.0);

        computePlaneIntersections = true;
        }
    else
        {
        /* Confusing combination of radii and sweep */
        /* Get simple bounding boxes of the two ellipses in system with axis
            toward middle of sweep */
        DRange3d range;
        double halfSweep = 0.5 * (theta1 - theta0);
        double rCentroid;
        bsiDRange3d_init(&range);

        thetaMid = 0.5 * (theta0 + theta1);

        c0 = cos(thetaMid);
        s0 = sin(thetaMid);
        c1 = -s0;
        s1 = c0;

        bsiDRange3d_extendByComponents (&range, radius0, 0.0, 0.0);
        bsiDRange3d_extendByComponents (&range, radius1, 0.0, 0.0);

        /* Add true endpoints to the range */
        c = cos (halfSweep);
        s = sin (halfSweep);
        bsiDRange3d_extendByComponents (&range, radius0 * c,  radius0 * s, 0.0);
        bsiDRange3d_extendByComponents (&range, radius0 * c, -radius0 * s, 0.0);

        bsiDRange3d_extendByComponents (&range, radius1 * c,  radius1 * s, 0.0);
        bsiDRange3d_extendByComponents (&range, radius1 * c, -radius1 * s, 0.0);

        if (fabs (halfSweep) > msGeomConst_piOver2)
            {
            bsiDRange3d_extendByComponents (&range, 0.0,  radius1, 0.0);
            bsiDRange3d_extendByComponents (&range, 0.0, -radius1, 0.0);
            }

        bsiDPoint4d_setComponents (&pPlane[2], -c1, -s1, 0.0,  range.low.y );
        bsiDPoint4d_setComponents (&pPlane[3],  c1,  s1, 0.0, -range.high.y);

        bsiDPoint4d_setComponents (&pPlane[4], -c0, -s0, 0.0,  range.low.x );
        bsiDPoint4d_setComponents (&pPlane[5],  c0,  s0, 0.0, -range.high.x);

        rCentroid = 0.5 * (range.low.x + range.high.x);
        bsiDPoint4d_setComponents (&testPoint,
                        c0 * rCentroid,
                        s0 * rCentroid,
                        0.5 * (alpha0 + alpha1), 1.0);

        computePlaneIntersections = true;
        }

    if (computePlaneIntersections)
        {
        int i;
        DPoint4d *pPlane0, *pPlane1, *pPlane2;
        RotMatrix A, Ainv;
        static int planeId[8][3] =
            {
                { 0, 4, 2},
                { 0, 5, 2},
                { 0, 4, 3},
                { 0, 3, 5},
                { 1, 4, 2},
                { 1, 5, 2},
                { 1, 4, 3},
                { 1, 5, 3}
            };
        for (i = 0; i < 8 && SUCCESS == status; i++)
            {
            pPlane0 = pPlane + planeId[i][0];
            pPlane1 = pPlane + planeId[i][1];
            pPlane2 = pPlane + planeId[i][2];
            A.SetColumn (DVec3d::From (pPlane0->x, pPlane1->x, pPlane2->x), 0);
            A.SetColumn (DVec3d::From (pPlane0->y, pPlane1->y, pPlane2->y), 1);
            A.SetColumn (DVec3d::From (pPlane0->z, pPlane1->z, pPlane2->z), 2);
            if (Ainv.InverseOf (A))
                {
                status = SUCCESS;
                bsiRotMatrix_multiplyComponents (&Ainv, &pPoint[i], -pPlane0->w, -pPlane1->w, -pPlane2->w);
                }
            else
                {
                status = ERROR;
                }
            }
        }


    if (SUCCESS == status && pMap)
        {
        int i;
        double dot;
        bsiDMatrix4d_multiplyAndRenormalizeDPoint3dArray (&pMap->M0, pPoint, pPoint, 8);
        bsiDMatrix4d_multiplyMatrixPoint (&pMap->M0, &testPoint, &testPoint);
        bsiDMatrix4d_multiplyTransposePoints (&pMap->M1, pPlane, pPlane, 6);

        for (i = 0; i < 6; i++)
            {
            bsiDPoint4d_normalizePlane (&pPlane[i], &pPlane[i]);
            }

        /* Reverse normal of any plane that does not have the test point below */
        for (i = 0; i < 6; i++)
            {
            dot = bsiDPoint4d_dotProduct (&pPlane[i], &testPoint);
            if (dot > 0.0)
                bsiDPoint4d_negate (&pPlane[i], &pPlane[i]);
            }
        }
    return status;
    }
/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            09/97
*
+----------------------------------------------------------------------*/
static void    jmdlDRange3d_extendUnitAngular

(
DRange3dP pAngleRange,  /* <= ANGLES that cause extrema */
DRange3dP pXYRange,             /* <= COORDINATES of extrema */
double phi                      /* => angle whose cos, sin are compared to xy range */
)
    {
    double c = cos(phi);
    double s = sin(phi);

    if (c > pXYRange->high.x)
        {
        pXYRange->high.x = c;
        pAngleRange->high.x = phi;
        }

    if (s > pXYRange->high.y)
        {
        pXYRange->high.y = s;
        pAngleRange->high.y = phi;
        }

    if (c < pXYRange->low.x)
        {
        pXYRange->low.x = c;
        pAngleRange->low.x = phi;
        }

    if (s < pXYRange->low.y)
        {
        pXYRange->low.y = s;
        pAngleRange->low.y = phi;
        }
    }


/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            09/97
*
+----------------------------------------------------------------------*/
static void    jmdlDRange3d_extendUnitAngularInSweep

(
DRange3dP pAngleRange,  /* <= ANGLES that cause extrema */
DRange3dP pXYRange,             /* <= COORDINATES of extrema */
double      phi,                /* => angle to test */
double      phi0,               /* => start angle of range */
double      phi1                /* => end angle of range */
)
    {
    if (bsiTrig_angleInSweep (phi, phi0, phi1 - phi0))
        jmdlDRange3d_extendUnitAngular (pAngleRange, pXYRange, phi);
    }

/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            09/97
*
*  Find angles at which unit xy arc has extrema.
+----------------------------------------------------------------------*/
static void    jmdlRotatedConic_unitArcExtrema

(
DRange3dP pAngleRange,  /* <= ANGLES that cause extrema */
DRange3dP pXYRange,             /* <= COORDINATES of extrema */
double phi0,
double phi1
)
    {
    double sweep = phi1 - phi0;

    bsiDRange3d_init (pXYRange);
    jmdlDRange3d_extendUnitAngular              (pAngleRange, pXYRange, phi0);
    jmdlDRange3d_extendUnitAngular              (pAngleRange, pXYRange, phi1);

    jmdlDRange3d_extendUnitAngularInSweep       (pAngleRange, pXYRange, 0.0, phi0, sweep);
    jmdlDRange3d_extendUnitAngularInSweep       (pAngleRange, pXYRange, msGeomConst_piOver2, phi0, sweep);

    jmdlDRange3d_extendUnitAngularInSweep       (pAngleRange, pXYRange, -msGeomConst_piOver2, phi0, sweep);
    jmdlDRange3d_extendUnitAngularInSweep       (pAngleRange, pXYRange, msGeomConst_pi, phi0, sweep);
    }


/*------------------------------------------------------------------*//**
* Compute the range of the surface.
* @param pPoint <= 8 corners of bounding volume. (Some may be coincident)
* @param pPlane <= 6 planes of bounding volume.
* @bsihdr                               EarlinLutz          08/97
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt bsiRotatedConic_getBoundingVolume

(
DPoint3dP pPoint,
DPoint4dP pPlane,
const RotatedConic  *pSurface
)
    {
    StatusInt status = ERROR;
    double phi0, phi1, phiStar, radius, z, slope;
    double nearRadius;
    double z0, z1;
    switch (pSurface->type)
        {
        case RC_Cylinder:
            status = jmdlRotatedConic_arcHull (
                                pPoint,
                                pPlane,
                                pSurface->parameterRange.low.x,
                                pSurface->parameterRange.high.x,
                                1.0,
                                1.0,
                                0.0,
                                0.0,
                                0.0,
                                pSurface->parameterRange.low.y,
                                pSurface->parameterRange.high.y,
                                &pSurface->rotationMap
                                );
            status = SUCCESS;
            break;

        case RC_Disk:
            status = jmdlRotatedConic_arcHull (
                                pPoint,
                                pPlane,
                                pSurface->parameterRange.low.x, /* angle */
                                pSurface->parameterRange.high.x,
                                pSurface->parameterRange.low.y, /* radius */
                                pSurface->parameterRange.high.y,
                                0.0,    /* z = 0 */
                                0.0,    /* tangent slope */
                                0.0,    /* tangent slope */
                                0.0,    /* z0 = 0 */
                                0.0,    /* z1 = 0 */
                                &pSurface->rotationMap
                                );
            break;
        case RC_Cone:
            status = jmdlRotatedConic_arcHull (
                                pPoint,
                                pPlane,
                                pSurface->parameterRange.low.x,
                                pSurface->parameterRange.high.x,
                                1.0,
                                1.0,
                                -1.0,
                                -1.0,   /* Should be able to do better here */
                                0.0,
                                pSurface->parameterRange.low.y,
                                pSurface->parameterRange.high.y,
                                &pSurface->rotationMap

                                );
            break;

        case RC_Torus:
            {
            double r0, r1, b;
            DRange3d angleRange, xyRange;
            b = pSurface->hoopRadius;
            phi0    = pSurface->parameterRange.low.y;
            phi1    = pSurface->parameterRange.high.y;
            jmdlRotatedConic_unitArcExtrema (&angleRange, &xyRange, phi0, phi1);
            phiStar = angleRange.high.x;
            r0     = 1.0 + b * xyRange.low.x;
            r1     = 1.0 + b * xyRange.high.x;
            z       = b * sin(phiStar);
            z0      = b * xyRange.low.y;
            z1      = b * xyRange.high.y;
            slope = 0.0;
            if (fabs (phiStar) < msGeomConst_pi * 0.4)
                slope = tan (phiStar);

            status = jmdlRotatedConic_arcHull (
                                pPoint,
                                pPlane,
                                pSurface->parameterRange.low.x,
                                pSurface->parameterRange.high.x,
                                r0,
                                r1,
                                z,
                                0.0,
                                0.0,
                                z0,
                                z1,
                                &pSurface->rotationMap
                                );
            }
            break;

        case RC_Sphere:
            {
            double r0, r1;
            DRange3d angleRange, xyRange;
            phi0    = pSurface->parameterRange.low.y;
            phi1    = pSurface->parameterRange.high.y;
            jmdlRotatedConic_unitArcExtrema (&angleRange, &xyRange, phi0, phi1);
            phiStar = angleRange.high.x;
            radius  = fabs (cos (phiStar));
            z       = sin(phiStar);
            z0      = xyRange.low.y;
            z1      = xyRange.high.y;

            slope = 0.0;
            r0 = fabs (cos(phi0));
            r1 = fabs (cos (phi1));
            nearRadius = r0 < r1 ? r0 : r1;

            if (fabs (phiStar) < msGeomConst_pi * 0.4)
                slope = tan (phiStar);

            status = jmdlRotatedConic_arcHull (
                                pPoint,
                                pPlane,
                                pSurface->parameterRange.low.x,
                                pSurface->parameterRange.high.x,
                                radius,
                                nearRadius,
                                z,
                                tan(phiStar),
                                0.0,
                                z0,
                                z1,
                                &pSurface->rotationMap

                                );
            }
            break;

        }

    return status;
    }

/*------------------------------------------------------------------*//**
* Extend a range by the ranges of an array of conics.
* @bsihdr                               EarlinLutz          08/97
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void bsiDRange3d_extendByHConicArray

(
DRange3dP pRange,       /* <= range to extend */
const HConic    *pHConic,       /* => array of conics for range */
int     numConic        /* => number of conics */
)
    {
    int i;
    DRange3d conicRange;
    for (i = 0; i < numConic; i++)
        {
        bsiHConic_getRange (&conicRange, pHConic + i);
        bsiDRange3d_extendByRange (pRange, &conicRange);
        }
    }

/*------------------------------------------------------------------*//**
* Initialize a conic degenerated as a line. (3d coordinates
* @bsihdr                               EarlinLutz          08/97
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void bsiHConic_initLine

(
HConic          *pHConic,       /* <= conic to initialize */
DPoint3dCP pStart,      /* => start point */
DPoint3dCP pEnd         /* => end point */
)
    {
    if (pHConic)
        {
        DPoint4d start, end;
        bsiDPoint4d_copyAndWeight (&start, pStart, 1.0);
        bsiDPoint4d_copyAndWeight (&end,   pEnd,   1.0);
        bsiDEllipse4d_initFrom4dVectors (&pHConic->coordinates, &s_point0001_4d, &start, &end, 0.0, 1.0);
        pHConic->type = HConic_Line;
        }
    }

/*------------------------------------------------------------------*//**
* Initialize a conic degenerated as a line. (4d coordinates)
* @bsihdr                               EarlinLutz          08/97
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void bsiHConic_initLine4d

(
HConic          *pHConic,       /* <= conic to initialize */
DPoint4dCP pStart,      /* => start point */
DPoint4dCP pEnd         /* => end point */
)
    {
    if (pHConic)
        {
        bsiDEllipse4d_initFrom4dVectors (&pHConic->coordinates, &s_point0001_4d, pStart, pEnd, 0.0, 1.0);
        pHConic->type = HConic_Line;
        }
    }

/*------------------------------------------------------------------*//**
* Initialize a conic degenerated as a pair of lines. (4d coordinates)
* @bsihdr                               EarlinLutz          08/97
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void bsiHConic_initLinePair4d

(
HConic          *pHConic,       /* <= conic to initialize */
DPoint4dCP pIntersection,       /* => common start point */
DPoint4dCP pPoint0,       /* => first branch target */
DPoint4dCP pPoint1      /* => second branch target */
)
    {
    if (pHConic)
        {
        bsiDEllipse4d_initFrom4dVectors (&pHConic->coordinates, pIntersection, pPoint0, pPoint1, 0.0, 1.0);
        pHConic->type = HConic_LinePair;
        }
    }

/*------------------------------------------------------------------*//**
* Initialize a conic degenerated as a point.
* @bsihdr                               EarlinLutz          08/97
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void bsiHConic_initPoint

(
HConic          *pHConic,       /* <= conic to initialize */
DPoint3dCP pPoint       /* => point */
)
    {
    static DPoint3d zeroVector = {0.0, 0.0, 0.0};
    if (pHConic)
        {
        bsiDEllipse4d_initFrom3dVectors (&pHConic->coordinates, pPoint, &zeroVector, &zeroVector, 0.0, 1.0);
        pHConic->type = HConic_Point;
        }
    }

/*------------------------------------------------------------------*//**
* Initialize a conic as an ellipse (homogeneous, possilbly mapped to
*   parabola or hyperbola)
* @bsihdr                               EarlinLutz          08/97
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void bsiHConic_initHEllipse

(
HConic          *pHConic,       /* <= conic to initialize */
DEllipse4dCP pHEllipse  /* => ellipse */
)
    {
    if (pHConic)
        {
        pHConic->coordinates = *pHEllipse;
        pHConic->type = HConic_Ellipse;
        }
    }

/*------------------------------------------------------------------*//**
* Initialize a conic as an ellipse (homogeneous, possilbly mapped to
*   parabola or hyperbola)
* @bsihdr                               EarlinLutz          08/97
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void bsiHConic_init4dEllipseVectors

(
HConic          *pHConic,       /* <= conic to initialize */
DPoint4dCP pOrigin,       /* => origin vector */
DPoint4dCP pVector0,      /* => 0 degree vector */
DPoint4dCP pVector90,     /* => 90 degree vector */
double    theta0,               /* start angle */
double    sweep         /* end angle  */
)
    {
    if (pHConic)
        {
        bsiDEllipse4d_initFrom4dVectors (&pHConic->coordinates, pOrigin, pVector0, pVector90, theta0, sweep);
        pHConic->type = HConic_Ellipse;
        }
    }

/*------------------------------------------------------------------*//**
* Initialize a conic as an ellipse.
*   parabola or hyperbola)
* @bsihdr                               EarlinLutz          08/97
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void bsiHConic_init3dEllipsePoints

(
HConic          *pHConic,       /* <= conic to initialize */
DPoint3dCP pOrigin,       /* => origin vector */
DPoint3dCP pPoint0,      /* => 0 degree vector */
DPoint3dCP pPoint90,     /* => 90 degree vector */
double    theta0,               /* start angle */
double    sweep         /* end angle  */
)
    {

    DPoint3d point[3];
    point[0] = *pOrigin;
    point[1] = *pPoint0;
    point[2] = *pPoint90;

    if (pHConic)
        {
        bsiDEllipse4d_initFrom3dPoints (&pHConic->coordinates, point, theta0, sweep);
        pHConic->type = HConic_Ellipse;
        }
    }

/*---------------------------------------------------------------------------------**//**
Combine adjacent intervals of ellipses.
* @bsimethod    ICurveQuery
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiHConic_consolidateSectors
(
HConic *pConic
)
    {
    if (pConic->type == HConic_Ellipse)
        {
        double w0 = pConic->coordinates.vector0.w;
        double w90 = pConic->coordinates.vector90.w;
        double wc = pConic->coordinates.center.w;
        double q = w0 * w0 + w90 * w90;
        if (q < wc)
            {
            //// OK, it really is an ellipse, no fear of joining anything across an asymptote ...
            //double tol = bsiTrig_smallAngle ();
            bsiRange1d_consolidatePeriodic (&pConic->coordinates.sectors, msGeomConst_2pi);
            }
        }
    }

/*------------------------------------------------------------------*//**
* Compute 0, 1, or 2 conic parts of the intersection of unit cone patch
* and a plane.
* @bsihdr                               EarlinLutz          08/97
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void omdHConic_initUnitConePlaneIntersection

(
HConic    *pHConic,       /* <= 0, 1, or 2 intersection conics */
int       *pNumConic,     /* <= number of conics (0, 1, or 2) */
const DPoint4d  *pPlane,        /* <= plane coefficients */
const DRange3d  *pRange         /* <= parameter range to apply */
)
    {
    double tol = bsiTrig_smallAngle ();
    double directionScale = sqrt (pPlane->x * pPlane->x + pPlane->y * pPlane->y + pPlane->z * pPlane->z);
    double hx, hy, hz, hw;
    double c0, s0, c1,s1;
    DPoint4d point00, point01, point10, point11;
    int numSolution;
    double theta0, theta1, dTheta;
    double alpha0, alpha1;
    *pNumConic = 0;

    if (pRange)
        {
        theta0 = pRange->low.x;
        theta1 = pRange->high.x;
        alpha0 = pRange->low.y;
        alpha1 = pRange->high.y;
        }
    else
        {
        theta0 = 0.0;
        theta1 = msGeomConst_2pi;
        alpha0 = 0.0;
        alpha1 = 1.0;

        }

    dTheta = theta1 - theta0;

    if (directionScale == 0.0
        || fabs (directionScale) < tol * fabs (pPlane->z))
        {
        /* The plane is 'at infinity'.  Call it an empty intersection set */
        }
    else
        {
        hx = pPlane->x / directionScale;
        hy = pPlane->y / directionScale;
        hz = pPlane->z / directionScale;
        hw = pPlane->w / directionScale;

        if (fabs (hw) < tol)
            {
            /* Plane passes through origin. It may also pass through 1 or to rulings of the cone */
            numSolution = bsiMath_solveApproximateUnitQuadratic (
                &c0, &s0, &c1, &s1,
                hz, hx, hy,
                s_lineUnitCircleIntersectionTolerance
                );

            if (numSolution == 1)
                {
                /* The plane is tangent to the cone at the given angle */
                bsiDPoint4d_setComponents (&point00, alpha0 * c0, alpha0 * s0, alpha0, 1.0);
                bsiDPoint4d_setComponents (&point01, alpha1 * c0, alpha1 * s0, alpha1, 1.0);
                if (!pRange || bsiTrig_angleInSweep (bsiTrig_atan2 (s0, c0) , theta0, dTheta))
                    {
                    bsiHConic_initLine4d (pHConic, &point00, &point01);
                    *pNumConic += 1;
                    }

                }
            else if (numSolution == 2)
                {

                /* The plane slices the cone in two distinct lines */
                bsiDPoint4d_setComponents (&point00, alpha0 * c0, alpha0 * s0, alpha0, 1.0);
                bsiDPoint4d_setComponents (&point01, alpha1 * c0, alpha1 * s0, alpha1, 1.0);

                bsiDPoint4d_setComponents (&point10, alpha0 * c1, alpha0 * s1, alpha0, 1.0);
                bsiDPoint4d_setComponents (&point11, alpha1 * c1, alpha1 * s1, alpha1, 1.0);

                if (!pRange || bsiTrig_angleInSweep (bsiTrig_atan2 (s0, c0) , theta0, dTheta))
                    {
                    bsiHConic_initLine4d (pHConic + *pNumConic, &point00, &point01);
                    *pNumConic += 1;
                    }

                if (!pRange || bsiTrig_angleInSweep (bsiTrig_atan2 (s1, c1) , theta0, dTheta))
                    {
                    bsiHConic_initLine4d (pHConic + *pNumConic, &point10, &point11);
                    *pNumConic += 1;
                    }
                }
            }
        else
            {
            /* plane does not pass through origin, so intersection is always a full curve.
                (Which may have asymptotes !!!) */
            DPoint4d origin;
            DPoint4d vector0;
            DPoint4d vector90;
            bsiDPoint4d_setComponents (&vector0,  -hw,  0.0, 0.0, hx);
            bsiDPoint4d_setComponents (&vector90, 0.0, -hw,  0.0, hy);
            bsiDPoint4d_setComponents (&origin,   0.0, 0.0, -hw,  hz);
            bsiHConic_init4dEllipseVectors (pHConic, &origin, &vector0, &vector90, theta0, dTheta); bsiDEllipse4d_absCenterWeight(&pHConic->coordinates, &pHConic->coordinates) ; /* THISWAS a bool thrown away as a statement */

            if (pRange)
                {
                DPoint4d clipPlane[2];
                bool    clipped;
                bsiDPoint4d_setComponents (clipPlane,     0.0, 0.0, 1.0,  -alpha1); /* top clip plane */
                bsiDPoint4d_setComponents (clipPlane + 1, 0.0, 0.0, -1.0, -alpha0); /* bottom clip plane */
                bsiDEllipse4d_clipToPlanes (&pHConic->coordinates, &clipped, clipPlane, 2, 0);
                }
            if (pHConic->coordinates.sectors.n <= 0)
                {
                *pNumConic = 0;
                }
            else
                {
                *pNumConic = 1;
                }
            }
        }
    }

/*------------------------------------------------------------------*//**
* Compute intersections of a (homogeneous) ellipse with a plane.
* Append the intersection parameters to an array.
* @bsihdr                               EarlinLutz          08/97
+----------------------------------------------------------------------*/
static void    jmdlDEllipse4d_addPlaneIntersectionParams

(
DPoint3dP pParam,       /* <=> growing array of intersection parameters */
int             *pNumPoint,     /* <=> growing count */
int             maxPoint,       /* => count limit */
DEllipse4dCP pEllipse,      /* => ellipse to be intersected with plane */
DPoint4dCP pPlane               /* => homogeneous plane array */
)
    {
    int n, i, k;
    DPoint3d param[2];
    n = bsiDEllipse4d_intersectPlane (
                        param,
                        &pEllipse->center,
                        &pEllipse->vector0,
                        &pEllipse->vector90,
                        pPlane
                        );

    for (i = 0; i < n && *pNumPoint < maxPoint; i++)
        {
        k = *pNumPoint;
        *pNumPoint += 1;
        pParam[k] = param[i];
        }
    }

/*------------------------------------------------------------------*//**
* Test if (phi, theta) is in the (possibly signed, possibly wrapped)
* spherical coordinates rectangle, allowing for the fact that a full
* range of +- pi in both theta and phi covers the sphere twice.
* @bsihdr                               EarlinLutz          08/97
+----------------------------------------------------------------------*/
static bool            jmdlRotatedConic_pointInSphericalSweep

(
double  theta,          /* => longitude of test point */
double  phi,            /* => latitude of test point */
double  theta0,         /* => start longitude of range */
double  dTheta,         /* => sweep longitude of range */
double  phi0,           /* => start latitude of range */
double  dPhi            /* => sweep latitude of range */
)
    {
    return
           (   bsiTrig_angleInSweep (theta, theta0, dTheta)
            && bsiTrig_angleInSweep (phi, phi0, dPhi))
        || (   bsiTrig_angleInSweep (theta + msGeomConst_pi, theta0, dTheta)
            && bsiTrig_angleInSweep (msGeomConst_pi - phi, phi0, dPhi));
    }

/*------------------------------------------------------------------*//**
* Test if (phi, theta) is in the (possibly signed, possibly wrapped)
* spherical coordinates rectangle, allowing for the fact that a full
* range of +- pi in both theta and phi covers the sphere twice.
* @bsihdr                               EarlinLutz          08/97
+----------------------------------------------------------------------*/
static bool            jmdlRotatedConic_pointInSphericalRange

(
double  theta,          /* => longitude of test point */
double  phi,            /* => latitude of test point */
DRange3dCP pRange    /* => spherical coordinates range */
)
    {
    return jmdlRotatedConic_pointInSphericalSweep (theta, phi,
                        pRange->low.x,
                        pRange->high.x - pRange->low.x,
                        pRange->low.y,
                        pRange->high.y - pRange->low.y
                        );
    }
/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            08/97
*
+----------------------------------------------------------------------*/
static int     jmdlDEllipse4d_sphericalClip

(
DEllipse4dP pClippedEllipse,
DEllipse4dCP pEllipse,
DRange3dCP pRange
)
    {
#define MAX_INTERSECTION_PARAM 10
    double theta0 = pRange->low.x;
    double theta1 = pRange->high.x;
    double phi0   = pRange->low.y;
    double phi1   = pRange->high.y;
    double dTheta = theta1 - theta0;
    double dPhi   = phi1 - phi0;
    double theta, phi;
    int    intervalSaved;
    double beta0, beta1;

    double alpha[MAX_INTERSECTION_PARAM];
    double alpha0, alpha1, alphaMid;
    DPoint4d midPoint, midPointSpherical;
    int i;

    DPoint3d param[MAX_INTERSECTION_PARAM];
    DPoint4d plane;
    int numParam = 0;
    int maxAddParam = MAX_INTERSECTION_PARAM - 2;

    bsiDPoint4d_setComponents (&plane, 0.0, 0.0, 1.0, -sin (phi0));
    jmdlDEllipse4d_addPlaneIntersectionParams (param, &numParam, maxAddParam, pEllipse, &plane);

    bsiDPoint4d_setComponents (&plane, 0.0, 0.0, 1.0, -sin (phi1));
    jmdlDEllipse4d_addPlaneIntersectionParams (param, &numParam, maxAddParam, pEllipse, &plane);

    bsiDPoint4d_setComponents (&plane, -sin (theta0), cos(theta0), 0.0, 0.0);
    jmdlDEllipse4d_addPlaneIntersectionParams (param, &numParam, maxAddParam, pEllipse, &plane);

    bsiDPoint4d_setComponents (&plane, -sin (theta1), cos(theta1), 0.0, 0.0);
    jmdlDEllipse4d_addPlaneIntersectionParams (param, &numParam, maxAddParam, pEllipse, &plane);


    /* Get raw angles for each intersection point */
    for (i = 0; i < numParam; i++)
        {
        alpha[i] = bsiTrig_atan2 (param[i].y, param[i].x);
        }

    /* Force in endpoints to get cyclic coverage */
    alpha[numParam++] = -msGeomConst_pi;
    alpha[numParam++] =  msGeomConst_pi;

    bsiDoubleArray_sort (alpha, numParam, true);

    *pClippedEllipse = *pEllipse;
    bsiDEllipse4d_clearSectors (pClippedEllipse);
    intervalSaved = false;
    beta0 = beta1 = 0.0;
    for (i = 1; i < numParam; i++)
        {
        alpha0 = alpha[i - 1];
        alpha1 = alpha[i];
        alphaMid = 0.5 * (alpha0 + alpha1);
        bsiDEllipse4d_evaluateDPoint4d (pEllipse, &midPoint, alphaMid);
        bsiQuadric_cartesianToSphere (&midPointSpherical, &midPoint);
        theta = midPointSpherical.x;
        phi   = midPointSpherical.y;

        if (   jmdlRotatedConic_pointInSphericalSweep (theta, phi,theta0, dTheta, phi0, dPhi ))
            {
            if (intervalSaved)
                {
                beta1 = alpha1;
                }
            else
                {
                beta0 = alpha0;
                beta1 = alpha1;
                }
            intervalSaved = true;
            }
        else
            {
            if (intervalSaved)
                bsiRange1d_addArcSweep(&pClippedEllipse->sectors, beta0, beta1 - beta0);
            intervalSaved = false;
            }
        }

        if (intervalSaved)
            bsiRange1d_addArcSweep(&pClippedEllipse->sectors, beta0, beta1 - beta0);

    return SUCCESS;
    }

/*------------------------------------------------------------------*//**
* Compute a conic which is the intersection of a unit sphere and a plane.
*
* Various tolerance tests are applied assuming working range 0..1
* (Because the sphere is unit sized..)
* @bsihdr                               EarlinLutz          08/97
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void omdHConic_initUnitSpherePlaneIntersection

(
HConic    *pHConic,       /* <= 0, 1, or 2 intersection conics */
int       *pNumConic,     /* <= number of conics (0, 1, or 2) */
const DPoint4d  *pPlane,        /* => plane coefficients */
const DRange3d  *pRange         /* => parameter range to apply */
)
    {
    DPoint3d center, point0, point90;
    *pNumConic = 0;


    if (2 == bsiGeom_intersectPlaneUnitSphere (&center, &point0, &point90, pPlane))
        {
        bsiHConic_init3dEllipsePoints (pHConic, &center, &point0, &point90, 0.0, msGeomConst_2pi);
        *pNumConic = 1;
        if (pRange)
            jmdlDEllipse4d_sphericalClip (&pHConic->coordinates, &pHConic->coordinates, pRange);
        }
    }

/*------------------------------------------------------------------*//**
* If possible, init conics which are the intersection of a torus and
* a plane.  Return ERROR if intersection is non-conic.
* @bsihdr                               EarlinLutz          08/97
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt omdHConic_initTorusPlaneIntersection

(
HConic    *pHConic,       /* <= 0, 1, or 2 intersection conics */
int       *pNumConic,     /* <= number of conics (0, 1, or 2) */
      double    b,              /* => hoop radius of torus */
const DPoint4d  *pPlane,        /* => plane coefficients */
const DRange3d  *pRange         /* => parameter range to apply */
)
    {
    double rxy2  = pPlane->x * pPlane->x + pPlane->y * pPlane->y;
    double rxy   = sqrt (rxy2);
    double rz    = fabs (pPlane->z);
    double rw    = fabs (pPlane->w);
    double rxyz = sqrt (rxy2 + rz * rz);
    double thetaA, thetaB, phi;
    double theta0, theta1, phi0, phi1;
    static double relTol = 1.0e-12;
    int numIntersection = 0;
    StatusInt status = ERROR;

    if (pRange)
        {
        theta0 = pRange->low.x;
        theta1 = pRange->high.x;
        phi0   = pRange->low.y;
        phi1   = pRange->high.y;
        }
    else
        {
        theta0 = phi0 = 0.0;
        theta1 = phi1 = msGeomConst_2pi;
        }

    if (rxy < relTol * rz)
        {
        /* Horizontal plane */
        double alpha = pPlane->w;
        double beta  = 0.0;
        double gamma = pPlane->z * b;
        int i;
        double cosPhi[2], sinPhi[2];
        double phiSweep = phi1 - phi0;

        /* In normalized circle space:
                Intersection of line s*b = zPlane with circle c^2 + s^2 = 1
        */
        int numSolution = bsiMath_solveApproximateUnitQuadratic (
                &cosPhi[0], &sinPhi[0],
                &cosPhi[1], &sinPhi[1],
                alpha, beta, gamma,
                s_lineUnitCircleIntersectionTolerance
                );
        for (i = 0; i < numSolution; i++)
            {
            phi = bsiTrig_atan2 (sinPhi[i], cosPhi[i]);
            if (bsiTrig_angleInSweep (phi, phi0, phiSweep))
                {
                bsiHConic_initZEllipse (pHConic + numIntersection++, theta0, theta1, 1.0 + b * cosPhi[i], b * sinPhi[i]);
                }
            }
        status = SUCCESS;
        }
    else if (rw < relTol * rxyz && rz < relTol * rxy)
        {
        /* Vertical Plane through origin */
        thetaA = bsiTrig_atan2 (-pPlane->x, pPlane->y);
        thetaB = bsiTrig_atan2 (pPlane->x, -pPlane->y);

        if (bsiTrig_angleInSweep (thetaA, theta0, theta1 - theta0))
            bsiHConic_initMeridian (pHConic + numIntersection++, phi0, phi1, thetaA, b, 1.0);

        if (bsiTrig_angleInSweep (thetaB, theta0, theta1 - theta0))
            bsiHConic_initMeridian (pHConic + numIntersection++, phi0, phi1, thetaB, b, 1.0);

        status = SUCCESS;
        }
    *pNumConic = numIntersection;
    return status;
    }

/*------------------------------------------------------------------*//**
*  Compute a conic which is the intersection of a unit cone and
*  a plane.
*
*  Various tolerance tests are applied assuming working range 0..1
*  (Because the cone is unit sized..)
* @bsihdr                               EarlinLutz          08/97
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void omdHConic_initUnitCylinderPlaneIntersection

(
HConic    *pHConic,       /* <= 0, 1, or 2 intersection conics */
int       *pNumConic,     /* <= number of conics (0, 1, or 2) */
const DPoint4d  *pPlane,        /* <= plane coefficients */
const DRange3d  *pRange         /* <= parameter range to apply */
)
    {
    static double tol = 1.0e-8;
    double directionScale = sqrt (pPlane->x * pPlane->x + pPlane->y * pPlane->y + pPlane->z * pPlane->z);
    double hx, hy, hz, hw;
    double c0, s0, c1,s1;
    DPoint4d point00, point01, point10, point11;
    int numSolution;
    double theta0, theta1, dTheta;
    double alpha0, alpha1;
    *pNumConic = 0;

    if (pRange)
        {
        theta0 = pRange->low.x;
        theta1 = pRange->high.x;
        alpha0 = pRange->low.y;
        alpha1 = pRange->high.y;
        }
    else
        {
        theta0 = 0.0;
        theta1 = msGeomConst_2pi;
        alpha0 = 0.0;
        alpha1 = 1.0;

        }

    dTheta = theta1 - theta0;

    if (directionScale == 0.0
        || fabs (directionScale) < tol * fabs (pPlane->z))
        {
        /* The plane is 'at infinity'.  Call it an empty intersection set */
        }
    else
        {
        hx = pPlane->x / directionScale;
        hy = pPlane->y / directionScale;
        hz = pPlane->z / directionScale;
        hw = pPlane->w / directionScale;

        if (fabs (hz) < tol)
            {
            /* Plane is vertical.  It may pass through 0, 1, or 2 rulings of the cylinder */
            numSolution = bsiMath_solveApproximateUnitQuadratic (
                &c0, &s0, &c1, &s1,
                hw, hx, hy,
                s_lineUnitCircleIntersectionTolerance
                );

            if (numSolution == 1)
                {
                /* The plane is tangent to the cone at the given angle */
                bsiDPoint4d_setComponents (&point00, c0, s0, alpha0, 1.0);
                bsiDPoint4d_setComponents (&point01, c0, s0, alpha1, 1.0);
                if (!pRange || bsiTrig_angleInSweep (bsiTrig_atan2 (s0, c0) , theta0, dTheta))
                    {
                    bsiHConic_initLine4d (pHConic, &point00, &point01);
                    *pNumConic += 1;
                    }

                }
            else if (numSolution == 2)
                {

                /* The plane slices the cone in two distinct lines */
                bsiDPoint4d_setComponents (&point00, c0, s0, alpha0, 1.0);
                bsiDPoint4d_setComponents (&point01, c0, s0, alpha1, 1.0);

                bsiDPoint4d_setComponents (&point10, c1, s1, alpha0, 1.0);
                bsiDPoint4d_setComponents (&point11, c1, s1, alpha1, 1.0);

                if (!pRange || bsiTrig_angleInSweep (bsiTrig_atan2 (s0, c0) , theta0, dTheta))
                    {
                    bsiHConic_initLine4d (pHConic + *pNumConic, &point00, &point01);
                    *pNumConic += 1;
                    }

                if (!pRange || bsiTrig_angleInSweep (bsiTrig_atan2 (s1, c1) , theta0, dTheta))
                    {
                    bsiHConic_initLine4d (pHConic + *pNumConic, &point10, &point11);
                    *pNumConic += 1;
                    }
                }
            }
        else
            {
            /* Plane is not vertical.   Intersection is ellipse */
            DPoint4d origin;
            DPoint4d vector0;
            DPoint4d vector90;

            bsiDPoint4d_setComponents (&vector0,  -hz,  0.0, hx,  0.0);
            bsiDPoint4d_setComponents (&vector90, 0.0, -hz,  hy,  0.0);
            bsiDPoint4d_setComponents (&origin,   0.0, 0.0,  hw,  -hz);

            bsiHConic_init4dEllipseVectors (pHConic, &origin, &vector0, &vector90, theta0, dTheta); bsiDEllipse4d_absCenterWeight(&pHConic->coordinates, &pHConic->coordinates) ; /* THISWAS a bool thrown away as a statement */

            if (pRange)
                {
                DPoint4d clipPlane[2];
                bool    clipped;
                bsiDPoint4d_setComponents (clipPlane,     0.0, 0.0, 1.0,  -alpha1); /* top clip plane */
                bsiDPoint4d_setComponents (clipPlane + 1, 0.0, 0.0, -1.0,  alpha0); /* bottom clip plane */
                bsiDEllipse4d_clipToPlanes (&pHConic->coordinates, &clipped, clipPlane, 2, 0);
                }
            if (pHConic->coordinates.sectors.n <= 0)
                {
                *pNumConic = 0;
                }
            else
                {
                *pNumConic = 1;
                }
            }
        }
    }

/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            07/97
*
*  Compute a conic which is the intersection of a unit disk and
*  a plane.
*
*  Various tolerance tests are applied assuming working range 0..1
*  (Because the disk is unit sized ... )
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void omdHConic_initUnitDiskPlaneIntersection

(
HConic    *pHConic,       /* <= 0, 1, or 2 intersection conics */
int       *pNumConic,     /* <= number of conics (0, 1, or 2) */
const DPoint4d  *pPlane,        /* <= plane coefficients */
const DRange3d  *pRange         /* <= parameter range to apply */
)
    {
    double tol = bsiTrig_smallAngle ();
    //double almostOne = 1.0 - tol;
    double rxy = sqrt (pPlane->x * pPlane->x + pPlane->y * pPlane->y);
    double c0, s0, c1,s1, e0, e1;
    DPoint4d point0, point1;
    double theta0, theta1, dTheta;
    double r0, r1;
    *pNumConic = 0;

    if (pRange)
        {
        theta0 = pRange->low.x;
        theta1 = pRange->high.x;
        r0 = pRange->low.y;
        r1 = pRange->high.y;
        }
    else
        {
        theta0 = 0.0;
        theta1 = msGeomConst_2pi;
        r0 = 0.0;
        r1 = 1.0;
        }

    dTheta = theta1 - theta0;

    /* Work with the xy plane line where the plane intersects xy plane */
    if (rxy <= fabs (tol * pPlane->w))
        {
        /* closest approach of the line to the origin is at infinity */
        }
    else
        {
        double inverse = 1.0 / rxy;
        double rMin = - pPlane->w * inverse;    /* SIGNED radius */
        //double rMin2 = rMin * rMin;
        //double qMin = fabs (rMin);

        double ux = pPlane->x * inverse;    /* Unit vector perpendicular to line */
        double uy = pPlane->y * inverse;
        double vx, vy;                      /* Unit vector tangent to line */
        double ax, ay;                      /* Base Point of line */
        double bb;
        double q0, q1;
        double mu0, mu1, dMu;

        double mu[6];   /* Up to 6 hits of 2 circles and 2 end lines */
        int numMu = 0;

        if (rMin < 0.0)
            {
            rMin = -rMin;
            ux = -ux;
            uy = -uy;
            }

        ax = rMin * ux;
        ay = rMin * uy;
        vx = -uy;
        vy =  ux;

        q0 = fabs (r0);
        q1 = fabs (r1);

        if (q1 < q0)
            {
            q0 = fabs (r1);
            q1 = fabs (r0);
            }

        if (q0 < tol)
            {
            /* Ignore singular point within the sector */
            }
        else if (rMin < q0 - tol)
            {
            bb = sqrt (q0 * q0 - rMin * rMin);
            mu[numMu++] =  bb;
            mu[numMu++] = -bb;
            }
        else if (rMin < q0 + tol)
            {
            mu[numMu++] = 0.0;
            }


        if (q1 < tol)
            {
            /* Shouldn't happen */
            numMu = 0;
            }
        else if (rMin < q1 - tol)
            {
            bb = sqrt (q1 * q1 - rMin * rMin);
            mu[numMu++] =  bb;
            mu[numMu++] = -bb;
            }
        else
            {
            /* rxy is outside both circles */
            numMu = 0;
            }


        if (!bsiTrig_isAngleFullCircle (theta1 - theta0))
            {
            /* Intersect with rays at start and end angles  */
            double rbar;
            c0 = cos (theta0);
            s0 = sin (theta0);
            c1 = cos (theta1);
            s1 = sin (theta1);
            e0 = ux * c0 + uy * s0;
            e1 = ux * c1 + uy * s1;
            /* If the boundary ray is on the 'far side' of the origin from the line, the negative e0
                bypasses this block */
            if (q0 * e0 + tol < rMin && rMin < q1 * e0 - tol)
                {
                rbar = rMin / e0;
                mu[numMu++] = rbar * (vx * c0 + vy * s0);
                }

            if (q0 * e1 + tol < rMin && rMin < q1 * e1 - tol)
                {
                rbar = rMin / e1;
                mu[numMu++] = rbar * (vx * c1 + vy * s1);
                }
            }

        if (numMu > 1)
            {
            int numKeep = 0;
            int i;
            bsiDoubleArray_sort (mu, numMu, true);

            mu0 = mu[0];
            mu1 = mu[numMu - 1];
            dMu = mu1 - mu0;

            bsiDPoint4d_setComponents (&point0, ax + mu0 * vx, ay + mu0 * vy, 0.0, 1.0);
            bsiDPoint4d_setComponents (&point1, ax + mu1 * vx, ay + mu1 * vy, 0.0, 1.0);
            bsiHConic_initLine4d (pHConic, &point0, &point1);
            bsiDEllipse4d_clearSectors (&pHConic->coordinates);

            for (i = 1; i < numMu; i++)
                {
                double delta = mu[i] - mu[i-1];
                double alpha = 0.5 * (mu[i-1] + mu[i]);
                double xx = ax + vx * alpha;
                double yy = ay + vy * alpha;
                double theta = atan2 (yy, xx);
                double rr = sqrt (xx * xx + yy * yy);
                if (delta > tol && bsiTrig_angleInSweep (theta, theta0, dTheta)
                        && rr >= q0 && rr <= q1)
                    {
                    double s0 = (mu[i-1] - mu0) / dMu;
                    double s1 = (mu[i] - mu0) / dMu;
                    bsiRange1d_addUnordered (&pHConic->coordinates.sectors, s0, s1);
                    numKeep++;
                    }
                }
            if (numKeep > 0)
                *pNumConic = 1;
            }
        }
    }

/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            07/97
*
+----------------------------------------------------------------------*/
static void updateInterval

(
double *px0,
double *px1,
bool   *pDefined,
double y0,
double y1
)
    {
    if (y1 < y0)
        {
        double temp = y0;
        y0 = y1;
        y1 = temp;
        }

    if (!*pDefined)
        {
        *pDefined = true;
        *px0 = y0;
        *px1 = y1;
        }
    else
        {
        if (y0 > *px0)
            *px0 = y0;
        if (y1 < *px1)
            *px0 = y0;
        }
    }

/*----------------------------------------------------------------*//**
*  Compute a conic which is the intersection of a unit cone and
*  a plane.
*
*  Various tolerance tests are applied assuming working range 0..1
*  (Because the cone is unit sized..)
* @bsihdr EarlinLutz                            07/97
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void bsiHConic_initUnitSquarePlaneIntersection

(
HConic    *pHConic,       /* <= 0, 1, or 2 intersection conics */
int       *pNumConic,     /* <= number of conics (0, 1, or 2) */
const DPoint4d  *pPlane,        /* <= plane coefficients */
const DRange3d  *pRange         /* <= parameter range to apply */
)
    {
    double tol = bsiTrig_smallAngle ();
    double directionScale = sqrt (pPlane->x * pPlane->x + pPlane->y * pPlane->y);
    double hx, hy, hw, ax, ay;
    double x0, y0, x1, y1;
    *pNumConic = 0;

    if (pRange)
        {
        x0 = pRange->low.x;
        x1 = pRange->high.x;
        y0 = pRange->low.y;
        y1 = pRange->high.y;
        }
    else
        {
        x0 = 0.0;
        x1 = 1.0;
        y0 = 0.0;
        y1 = 1.0;
        }

    if (directionScale == 0.0
        || fabs (directionScale) < tol * fabs (pPlane->w))
        {
        /* The plane is 'at infinity'.  Call it an empty intersection set */
        }
    else
        {
        double lambda0;
        double lambda1;
        double lambdaIn = 0, lambdaOut = 0;
        bool    intervalDefined = false;
        hx = pPlane->x / directionScale;
        hy = pPlane->y / directionScale;
        hw = pPlane->w / directionScale;

        ax = hx * hw;
        ay = hy * hw;

        if (fabs (hx) > tol)
            {
            lambda0 = -(y0 + ax) / hx;
            lambda1 = -(y1 + ay) / hx;
            updateInterval (&lambdaIn, &lambdaOut, &intervalDefined, lambda0, lambda1);
            }

        if (fabs (hy) > tol)
            {
            lambda0 =  (y0 + ax) / hy;
            lambda1 =  (y1 + ay) / hy;
            updateInterval (&lambdaIn, &lambdaOut, &intervalDefined, lambda0, lambda1);
            }

        if (intervalDefined && lambdaIn < lambdaOut)
            {
            DPoint4d start, end;
            bsiDPoint4d_setComponents (&start,
                                    - ax - lambdaIn * hy,
                                    - ay + lambdaIn * hx,
                                    0.0,
                                    1.0);
            bsiDPoint4d_setComponents (&end,
                                    - ax - lambdaOut * hy,
                                    - ay + lambdaOut * hx,
                                    0.0,
                                    1.0);
            bsiHConic_initLine4d (pHConic, &start, &end);
            *pNumConic = 1;
            }
        }
    }

/*----------------------------------------------------------------*//**
* Initialize a conic as a circular arc of given radius around the z axis.
* @bsihdr EarlinLutz                            07/97
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void bsiHConic_initZEllipse

(
HConic          *pHConic,       /* <= conic to initialize */
double          theta0,         /* => start angle */
double          theta1,         /* => end angle */
double          radius,         /* => circle radius */
double          z               /* => altitude */
)
    {
    if (pHConic)
        {
        if (fabs (radius) > bsiTrig_smallAngle())
            {
            DPoint3d center, vector0, vector90;
            center.Init ( 0.0, 0.0, z);
            vector0.Init ( radius, 0.0, 0.0);
            vector90.Init ( 0.0, radius, 0.0);
            bsiDEllipse4d_initFrom3dVectors
                            (
                            &pHConic->coordinates,
                            &center,
                            &vector0,
                            &vector90,
                            theta0,
                            theta1 - theta0
                            );
            pHConic->type = HConic_Ellipse;
            }
        else
            {
            DPoint3d point;
            point.Init ( 0.0, 0.0, z);
            bsiHConic_initPoint (pHConic, &point);
            }
        }
    }

/*----------------------------------------------------------------*//**
* Initialize a meridian of longitude.
* @bsihdr EarlinLutz                            07/97
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void bsiHConic_initMeridian

(
HConic          *pHConic,       /* <= conic to initialize */
double          phi0,           /* => start angle (latitude) */
double          phi1,           /* => end angle (latitude) */
double          theta,          /* => meridian angle */
double          radius,         /* => meridian radius */
double          offset          /* => offset of center from z axis in theta direction */
)
    {
    double dPhi = phi1 - phi0;
    double c = cos (theta);
    double s = sin (theta);
    if (pHConic)
        {
        if (fabs (dPhi) > bsiTrig_smallAngle())
            {
            DPoint3d vector0, vector90, center;
            vector0.Init (  c * radius,   s * radius,   0.0);
            vector90.Init ( 0.0, 0.0, radius);
            center.Init (   c * offset, s * offset, 0.0);
            bsiDEllipse4d_initFrom3dVectors
                            (
                            &pHConic->coordinates,
                            &center,
                            &vector0,
                            &vector90,
                            phi0,
                            dPhi
                            );
            pHConic->type = HConic_Ellipse;
            }
        else
            {
            DPoint3d point;
            double cosPhi = cos (phi0);
            double sinPhi = sin (phi0);
            double a = offset + radius * cosPhi;
            point.Init ( c * a, s * a, radius * sinPhi);
            bsiHConic_initPoint (pHConic, &point);
            }
        }
    }

/*----------------------------------------------------------------*//**
* Extract start and end parameters of a specified segment of a conic.
* @bsihdr EarlinLutz                            07/97
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt bsiHConic_getSegment

(
double    *pStart,      /* <= segement start parameter */
double    *pEnd,                /* <= segment end parameter */
const HConic    *pHConic,       /* => conic to initialize */
int       i             /* => segment index */
)
    {
    StatusInt status;
    if (pHConic && i >= 0 && i < pHConic->coordinates.sectors.n)
        {
        *pStart = pHConic->coordinates.sectors.interval[i].minValue;
        *pEnd   = pHConic->coordinates.sectors.interval[i].maxValue;
        status = SUCCESS;
        }
    else
        {
        *pStart = 0.0;
        *pEnd   = 0.0;
        status = ERROR;
        }
    return status;
    }

/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            07/97
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt bsiDEllipse4d_getSegment

(
double    *pStart,      /* <= segement start parameter */
double    *pEnd,                /* <= segment end parameter */
DEllipse4dCP pHEllipse,     /* => conic to query */
int       i             /* => segment index */
)
    {
    StatusInt status;
    if (pHEllipse && i >= 0 && i < pHEllipse->sectors.n)
        {
        *pStart = pHEllipse->sectors.interval[i].minValue;
        *pEnd   = pHEllipse->sectors.interval[i].maxValue;
        status = SUCCESS;
        }
    else
        {
        *pStart = 0.0;
        *pEnd   = 0.0;
        status = ERROR;
        }
    return status;
    }

/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            07/97
*
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void bsiRotatedConic_setParameterRange

(
RotatedConic        *pSurface,
double              theta0,
double              deltaTheta,
double              phi0,
double              deltaPhi
)
    {
    pSurface->parameterRange.low.x  = theta0;
    pSurface->parameterRange.high.x = theta0 + deltaTheta;
    pSurface->parameterRange.low.y  = phi0;
    pSurface->parameterRange.high.y = phi0 + deltaPhi;
    pSurface->parameterRange.low.z = pSurface->parameterRange.high.z = 1.0;
    }

/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            07/97
*
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void bsiRotatedConic_setTolerance

(
RotatedConic        *pSurface,
double              tolerance
)
    {
    pSurface->tolerance = tolerance;
    }

#ifdef CompileAll
static void jmdlRotMatrix_initXYFromDPoint4d

(
RotMatrixP pMxy,            /* => matrix whose XY submatrix is filled from xy parts of points*/
RotMatrixP pMzw,            /* => matrix whose XY submatrix is filled from zw parts of points*/
DPoint4dP pPoint0,       /* => point whose zw coordinates go in column 0*/
DPoint4dP pPoint1           /* => point whose zw coordinates go in column 1*/
)
    {
    if (pMxy)
        {
        pMxy->form3d[0][0] = pPoint0->x;
        pMxy->form3d[0][1] = pPoint1->x;
        pMxy->form3d[1][0] = pPoint0->y;
        pMxy->form3d[1][1] = pPoint1->y;
        }

    if (pMzw)
        {
        pMxy->form3d[0][0] = pPoint0->z;
        pMxy->form3d[0][1] = pPoint1->z;
        pMxy->form3d[1][0] = pPoint0->w;
        pMxy->form3d[1][1] = pPoint1->w;
        }
    }
#endif

#define DET_COLXY(_A, _iA, _B, _iB) (_A->form3d[0][_iA] * _B->form3d[1][_iB] - _A->form3d[1][_iA] * _B->form3d[0][_iB])
#define DET_ROWXY(_A, _iA, _B, _iB) (_A->column[0]._iA * _B->column[1]._iB - _A->column[1]._iA * _B->column[0]._iB)
#ifdef goryIntersector
/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            07/97
*
*  Find c,s values such that the 2x2 rotation matrix
*        K = [ c s]
*           [-s c]
*  makes the product
*                   K*A + B
*  singular.
+----------------------------------------------------------------------*/
xPublic      int       jmdlRotatedConic_computeSingularRotations    /* number of rotations */

(
DPoint3dP pTrig,     /* <= [cos,sin,1] for 0, 1, or 2 singular rotations */
RotMatrixCP pA, /* => first matrix is leading 2x2  */
RotMatrixCP pB          /* => second matrix is leading 2x2 */
)
    {
    double beta  = - DET_COLXY (pA, 0, pB, 1) - DET_COLXY (pB, 0, pA, 1);
    double gamma = - DET_ROWXY (pA, x, pB, x) + DET_ROWXY (pB, y, pA, y);
    double alpha =   DET_COLXY (pA, 0, pA, 1) + DET_COLXY (pB, 0, pB, 1);

    int numSolution = bsiMath_solveApproximateUnitQuadratic (
                &pTrig[0].x, &pTrig[0].y,
                &pTrig[1].x, &pTrig[1].y,
                alpha, beta, gamma,
                s_lineUnitCircleIntersectionTolerance
                );
    pTrig[0].z = pTrig[1].z = 1.0;
    return numSolution;
    }
#endif

#ifdef CompileAll
/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            08/97
*
+----------------------------------------------------------------------*/
static StatusInt       jmdlRotatedConic_torusSilhouetteLimits

(
DPoint3dP pPointArray,       /* <= array of 0 to 4 singular angles, as cosine,sine, theta */
int             *pNumAngle,         /* <= number of singular points */
DPoint4dCP pEyePoint,       /* => local eyepoint coordinates */
double    a,                /* => primary axis radius */
double    b                 /* => minor radius */
)
    {
    double theta0, R0, R0sq;
    StatusInt status = ERROR;
    double aa, bb, cc;
    int numRoot, i, numTheta;
    double cosPhiRoot[2];
    double thetaRoot[4];
    double thetaQ[4];
    double cosQ[4];
    double sinQ[4];
    int numQ;

    /* At fixed theta, the torus silhouette's phi angle has cosine and sine as intersections of
       the unit circle cs with a line
            b*ew + (-ex*cos(theta) - ey*sin(theta) + a * ew) * c - ez * s
            alpha + beta * c + gamma * s = 0
       At each theta this may have 0, 1, or 2 solutions.   We want to find the single-solution cases.
       These occur where beta^2 + gamma^2 = alpha^2.
       Expanding directly gives a quadric section in the C,S plane.  Intersecting this with C^2+s^2=1 gives
        up to 4 points.   Nasty business.
       If ey happens to be zero, roots appear in nice pairs.  We can force this to happen always be
        rotating appropriately...
    */
    R0sq = pEyePoint->x * pEyePoint->x + pEyePoint->y * pEyePoint->y;
    R0   = sqrt (R0sq);
    if (pEyePoint->w == 0.0)
        {
        *pNumAngle = 0;
        status = SUCCESS;
        }
    else if (R0 > 0.0)
        {
        theta0 = bsiTrig_atan2 (pEyePoint->y, pEyePoint->x);
        /* Now theta = theta0 + phi, and ex*cos(theta) + ey*sin(theta) = R0 * cos(phi) */
        aa = R0sq;
        bb = -2.0 * a * pEyePoint->w * R0;
        cc = pEyePoint->w * pEyePoint->w * (a * a - b * b) + pEyePoint->z * pEyePoint->z;
        numRoot = bsiMath_solveQuadratic (cosPhiRoot, aa, bb, cc);
        for (i = 0; i < numRoot; i++)
            {
            if (fabs (cosPhiRoot[i]) < 1.0)
                {
                double phi = acos (cosPhiRoot[i]);
                thetaRoot[numTheta++] = theta0 + phi;
                thetaRoot[numTheta++] = theta0 - phi;;
                }
            }

        /* On the other hand, Lu Han just wrote the quartic solver: */
        bsiMath_conicIntersectUnitCircle (cosQ, sinQ, thetaQ, &numQ,
                                    a * pEyePoint->w, -pEyePoint->x, -pEyePoint->y,
                                    -pEyePoint->z, 0.0, 0.0,
                                    b * pEyePoint->w, 0.0, 0.0);

        }

    return status;
    }
#endif

/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            08/97
*
+----------------------------------------------------------------------*/
static StatusInt       bsiRotatedConic_toroidalFormLimits

(
DPoint3dP pPointArray,       /* <= array of 0 to 4 singular angles, as cosine,sine, theta */
int             *pNumAngle,         /* <= number of singular points */
RotMatrixCP pCoff,                  /* => coefficient matrix */
bool    transpose           /* => true to transpose matrix */
)
    {
    StatusInt status = ERROR;
    double thetaQ[4];
    double cosQ[4];
    double sinQ[4];
    int numQ, i;

    int solutionStatus;

    if (!transpose)
        {
        solutionStatus = bsiMath_conicIntersectUnitCircle (cosQ, sinQ, thetaQ, &numQ,
                                    pCoff->form3d[0][2], pCoff->form3d[0][0], pCoff->form3d[0][1],
                                    pCoff->form3d[1][2], pCoff->form3d[1][0], pCoff->form3d[1][1],
                                    pCoff->form3d[2][2], pCoff->form3d[2][0], pCoff->form3d[2][1]
                                    );
        }
    else
        {
        solutionStatus = bsiMath_conicIntersectUnitCircle (cosQ, sinQ, thetaQ, &numQ,
                                    pCoff->form3d[2][0], pCoff->form3d[0][0], pCoff->form3d[1][0],
                                    pCoff->form3d[2][1], pCoff->form3d[0][1], pCoff->form3d[1][1],
                                    pCoff->form3d[2][2], pCoff->form3d[0][2], pCoff->form3d[1][2]
                                    );
        }

    if (solutionStatus >= 0)
        {
        *pNumAngle = numQ;
        for (i = 0; i < numQ; i++)
            {
            pPointArray[i].x = cosQ[i];
            pPointArray[i].y = sinQ[i];
            pPointArray[i].z = thetaQ[i];
            }
        status = SUCCESS;
        }
    else
        {
        *pNumAngle = 0;
        }

    return status;
    }


/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            07/97
*
+----------------------------------------------------------------------*/
static StatusInt transformAndFlush /* Number of points left in local buffer at end (0 or 1) */

(
DPoint3dP pWorldBuffer, /* <=> world buffer */
DPoint4dP pLocalBuffer, /* <=> local buffer.  DATA IS OVERWRITTEN  */
int             *pNumPoint,     /* <=> point count */
int             recycleLastPoint,   /* => true if last point is to be recycled as 0'th */
const RotatedConic  *pSurface,      /* => the rotated conic surface */
SilhouetteArrayHandler handlerFunc, /* => callback for points */
void                *pUserData      /* => arbitrary pointer */
)
    {
    DPoint4d savedPoint;
    StatusInt status = ERROR;
    int numPoint = *pNumPoint;

    if (numPoint > 0)
        {
        savedPoint = pLocalBuffer[numPoint-1];

        bsiRotatedConic_transformDPoint4dArray (pLocalBuffer, pLocalBuffer, numPoint,
                                    pSurface, RC_COORDSYS_local, RC_COORDSYS_world);
        bsiDPoint4d_normalizeArray (pWorldBuffer, pLocalBuffer, numPoint);
        status = handlerFunc (NULL, pWorldBuffer, numPoint, RC_CURVEMASK_SMOOTH, pSurface, pUserData);

        if (recycleLastPoint)
            {
            pLocalBuffer[0] = savedPoint;
            numPoint = 1;
            }
        else
            {
            numPoint = 0;
            }
        }

    *pNumPoint = numPoint;
    return status;
    }

/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            08/97
*
+----------------------------------------------------------------------*/
static void    tpa_clear

(
ToroidalPointArray    *pArray
)
    {
    pArray->numPoint = 0;
    }

/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            08/97
*
+----------------------------------------------------------------------*/
static int tp_compareSortCoordinate

(
const void *vpPoint0,
const void *vpPoint1
)
    {
    const ToroidalPoint *pPoint0 = (const ToroidalPoint*)vpPoint0;
    const ToroidalPoint *pPoint1 = (const ToroidalPoint*)vpPoint1;
    if (pPoint0->sortCoordinate < pPoint1->sortCoordinate)
        return -1;
    if (pPoint0->sortCoordinate > pPoint1->sortCoordinate)
        return 1;
    return 0;
    }

/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            08/97
*
+----------------------------------------------------------------------*/
static void    tpa_centralSort

(
ToroidalPointArray    *pArray,
double                  thetaMid,
double                  phiMid
)
    {
    int numPoint = pArray->numPoint;
    double phi, theta;
    int i;
    for (i = 0; i < numPoint; i++)
        {
        theta = pArray->pointPair[i].thetaPoint.z;
        phi   = pArray->pointPair[i].phiPoint.z;

        pArray->pointPair[i].sortCoordinate =
                    bsiTrig_atan2 (phi - phiMid, theta - thetaMid);
        }
    qsort (pArray->pointPair, numPoint, sizeof (ToroidalPoint),
        (int (*)(const void *, const void *))tp_compareSortCoordinate);
    }
/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            08/97
*
+----------------------------------------------------------------------*/
static void    tpa_close

(
ToroidalPointArray    *pArray
)
    {
    ToroidalPoint point;
    if (pArray->numPoint > 0 && pArray->numPoint < MAX_TOROIDAL_POINT)
        {
        point = pArray->pointPair[0];
        pArray->pointPair[pArray->numPoint++] = point;
        }
    }


/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            08/97
*
+----------------------------------------------------------------------*/
static void    tpa_setOutputFunction

(
ToroidalPointArray    *pArray,
ToroidalOutputFunction outputFunction,
void                    *pOutputData
)
    {
    pArray->outputFunction = outputFunction;
    pArray->pOutputData    = pOutputData;
    }

/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            08/97
*
+----------------------------------------------------------------------*/
static StatusInt    tpa_output

(
ToroidalPointArray    *pArray
)
    {
    return pArray->outputFunction (pArray, pArray->pOutputData);
    }

/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            08/97
*
+----------------------------------------------------------------------*/
static bool                tp_inRanges /* true if point is contained in both ranges */

(
ToroidalPoint         *pPoint,
const ParamRange            *pRange0,
const ParamRange            *pRange1
)
    {
    double theta = pPoint->thetaPoint.z;
    double phi   = pPoint->phiPoint.z;

    if (pRange0 && !jmdlParamRange_containsPoint (pRange0, theta, phi))
        return false;
    if (pRange1 && !jmdlParamRange_containsPoint (pRange1, theta, phi))
        return false;
    return true;
    }

/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            08/97
*
+----------------------------------------------------------------------*/
static StatusInt tpa_addTPoint

(
ToroidalPointArray    *pArray,
const ToroidalPoint         *pTPoint
)
    {
    if (pArray->numPoint < MAX_TOROIDAL_POINT)
        {
        pArray->pointPair[pArray->numPoint] = *pTPoint;
        pArray->numPoint++;
        return SUCCESS;
        }
    return ERROR;
    }
/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            08/97
*
+----------------------------------------------------------------------*/
static int tp_compareTheta

(
const ToroidalPoint *pPoint0,
const ToroidalPoint *pPoint1
)
    {
    if (pPoint0->thetaPoint.z < pPoint1->thetaPoint.z)
        return -1;
    if (pPoint0->thetaPoint.z > pPoint1->thetaPoint.z)
        return 1;
    return 0;
    }

/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            08/97
*
+----------------------------------------------------------------------*/
static void  tpa_filter

(
ToroidalPointArray  *pArray,
double          dTheta,         /* => filter tolerance for theta */
double          dPhi            /* => filter tolerance for phi */
)
    {
    int i, numKeep, numPoint;
    double ePhi, eTheta;
    double phi0, theta0;
    double phi1, theta1;

    numPoint = pArray->numPoint;

    if (dPhi > 0.0 || dTheta > 0.0 && numPoint > 2)
        {
        int lastI = numPoint - 1;
        numKeep = 1;

        theta0 = pArray->pointPair[0].thetaPoint.z;
        phi0   = pArray->pointPair[0].phiPoint.z;

        for (i = 1; i <= lastI;i++)
            {
            theta1 = pArray->pointPair[i].thetaPoint.z;
            phi1   = pArray->pointPair[i].phiPoint.z;
            eTheta = fabs (theta1 - theta0);
            ePhi = fabs (phi1 - phi0);
            if (  (pArray->pointPair[i].mask & TP_BOUNDARY_POINT)
                || eTheta >= dTheta
                || ePhi >= dPhi
                || i == lastI
                )
                {
                pArray->pointPair[numKeep] = pArray->pointPair[i];
                numKeep++;
                }
            theta0 = theta1;
            phi0   = phi1;
            }
        pArray->numPoint = numKeep;
        }
    }

/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            08/97
*
+----------------------------------------------------------------------*/
static void  tpa_normalizeAndSortTheta

(
ToroidalPointArray  *pArray,
const ParamRange    *pRange,
double          dTheta,         /* => filter tolerance for theta */
double          dPhi            /* => filter tolerance for phi */
)
    {
    int i, numPoint;
    numPoint = pArray->numPoint;

    if (pRange)
        {
        for (i = 0; i < numPoint; i++)
            {
            jmdlParamRange_normalizePoint (
                            &pArray->pointPair[i].thetaPoint.z,
                            &pArray->pointPair[i].phiPoint.z,
                            pRange);
            }
        }
    qsort (pArray->pointPair, numPoint, sizeof(ToroidalPoint),
            (int (*)(const void*, const void*))tp_compareTheta
            );

    tpa_filter (pArray, dTheta, dPhi);
    }

/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            08/97
*
+----------------------------------------------------------------------*/
static void  tpa_normalizeToAngleLimits

(
ToroidalPointArray  *pArray,
double              theta0,
double              theta1,
double              phi0,
double              phi1
)
    {
    int i;
    int numPoint = pArray->numPoint;
    ParamRange range;

    jmdlParamRange_initFromAngleLimits (&range, theta0, theta1, phi0, phi1);

    for (i = 0; i < numPoint; i++)
        {
        jmdlParamRange_normalizePoint (
                        &pArray->pointPair[i].thetaPoint.z,
                        &pArray->pointPair[i].phiPoint.z,
                        &range);
        }
    }


/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            08/97
*
+----------------------------------------------------------------------*/
static int         compareZ

(
DPoint3dCP pPoint0,
DPoint3dCP pPoint1
)
    {
    if (pPoint0->z < pPoint1->z)
        return -1;
    if (pPoint0->z > pPoint1->z)
        return 1;
    return 0;
    }

/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            08/97
*
+----------------------------------------------------------------------*/
static int     outputThetaPhi

(
ToroidalPointArray *pArray,
ToroidalOutputParams *pParams
)
    {
    const RotatedConic *pSurface = pParams->pSurface;
    //StatusInt status = SUCCESS;
    void  *pUserData  = pParams->pUserData;
    double sinPhi, cosPhi, sinTheta, cosTheta;
    ToroidalPoint *pPoint;
    int numPoint = pArray->numPoint;
    int i;
    double a = 1.0;
    double b = pSurface->hoopRadius;
    double rho;
    double thetaMin, thetaMax, phiMin, phiMax, phi1, theta1, phi0, theta0, thetaMid, phiMid;
    ParamRange surfaceRange;
    int numOut;

    DPoint4d point4d[MAX_TOROIDAL_POINT];
    DPoint3d point3d[MAX_TOROIDAL_POINT];

    jmdlParamRange_initFromAngleLimits
                    (
                    &surfaceRange,
                    pSurface->parameterRange.low.x,
                    pSurface->parameterRange.high.x,
                    pSurface->parameterRange.low.y,
                    pSurface->parameterRange.high.y
                    );

    phiMin = thetaMin = 1000.0;
    phiMax = thetaMax = -1000.0;

    numOut = 0;
    theta0 = thetaMid = phi0 = phiMid = 0.0;
    for (i = 0; i < numPoint; i++)
        {
        pPoint = &pArray->pointPair[i];
        cosTheta = pPoint->thetaPoint.x;
        sinTheta = pPoint->thetaPoint.y;
        cosPhi = pPoint->phiPoint.x;
        sinPhi = pPoint->phiPoint.y;
        rho = a + b * cosPhi;
        phi1 = pArray->pointPair[i].phiPoint.z;
        theta1 = pArray->pointPair[i].thetaPoint.z;

        if (i > 0)
            {
            thetaMid = 0.5 * (theta0 + theta1);
            phiMid   = 0.5 * (phi0 + phi1);
            }

        theta0 = theta1;
        phi0   = phi1;

        if (phi1 < phiMin)
            phiMin = phi1;
        if (phi1 > phiMax)
            phiMax = phi1;
        if (theta1 < thetaMin)
            thetaMin = theta1;
        if (theta1 > thetaMax)
            thetaMax = theta1;

        if (numOut > 0)
            {
            if (!jmdlParamRange_containsPoint (&surfaceRange, thetaMid, phiMid))
                {
                transformAndFlush (point3d, point4d, &numOut,
                                false,
                                pSurface,
                                pParams->handlerFunc,
                                pUserData);
                numOut = 0;
                }
            }


        point4d[numOut].x = cosTheta * rho;
        point4d[numOut].y = sinTheta * rho;
        point4d[numOut].z = b * sinPhi;
        point4d[numOut].w = 1.0;
        numOut++;
        }

    if (numOut > 0)
        transformAndFlush (point3d, point4d, &numOut,
                        false,
                        pSurface,
                        pParams->handlerFunc,
                        pUserData);


    return SUCCESS;
    }

/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            08/97
*
*  Output function for points that have been computed with phi
*  and theta swapped.  Swap them back and call the normal function.
+----------------------------------------------------------------------*/
static int     outputPhiTheta

(
ToroidalPointArray *pArray,
ToroidalOutputParams *pParams
)
    {
    int numPoint = pArray->numPoint;
    int i;
    DPoint3d tempPoint;


    for (i = 0; i < numPoint; i++)
        {
        tempPoint = pArray->pointPair[i].thetaPoint;
        pArray->pointPair[i].thetaPoint = pArray->pointPair[i].phiPoint;
        pArray->pointPair[i].phiPoint = tempPoint;
        }

    return outputThetaPhi (pArray, pParams);
    }

/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            08/97
*
*  Unbounded theta slice.
+----------------------------------------------------------------------*/
static int     jmdlRotatedConic_sliceToroidalFormAtTheta

(
ToroidalPoint   *pPoint,        /* <= 0, 1, or 2 toroidal point */
int             *pNumPoint,     /* <= number of points hit by the slice */
RotMatrixCP pMatrix,       /* => coefficient matrix */
double          theta
)
    {
    DPoint3d thetaPoint, coffPoint, phiPoint;
    double cosPhi[2], sinPhi[2];
    double phi;
    int i, numRoot;

    thetaPoint.x = coffPoint.x = cos (theta);
    thetaPoint.y = coffPoint.y = sin (theta);
    thetaPoint.z = theta;
    coffPoint.z = 1.0;
    pMatrix->Multiply (coffPoint);
    numRoot   = bsiMath_solveApproximateUnitQuadratic (
                                    &cosPhi[0], &sinPhi[0],
                                    &cosPhi[1], &sinPhi[1],
                                    coffPoint.z, coffPoint.x, coffPoint.y, s_toroidalQuadTol);
    *pNumPoint = 0;
    for (i = 0; i < numRoot; i++)
        {
        phi = bsiTrig_atan2 (sinPhi[i], cosPhi[i]);
        phiPoint.x = cosPhi[i];
        phiPoint.y = sinPhi[i];
        phiPoint.z = phi;
        pPoint[i].phiPoint = phiPoint;
        pPoint[i].thetaPoint = thetaPoint;
        *pNumPoint += 1;
        }
    return SUCCESS;
    }

/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            08/97
*
*  Unbounded phi slice.
+----------------------------------------------------------------------*/
static int     jmdlRotatedConic_sliceToroidalFormAtPhi

(
ToroidalPoint   *pPoint,        /* <= 0, 1, or 2 toroidal point */
int             *pNumPoint,     /* <= number of points hit by the slice */
RotMatrixCP pMatrix,       /* => coefficient matrix */
double          phi
)
    {
    DPoint3d thetaPoint, coffPoint, phiPoint;
    double cosTheta[2], sinTheta[2];
    double theta;
    int i, numRoot;

    phiPoint.x = coffPoint.x = cos (phi);
    phiPoint.y = coffPoint.y = sin (phi);
    phiPoint.z = phi;
    coffPoint.z = 1.0;
    pMatrix->MultiplyTranspose (coffPoint);
    numRoot   = bsiMath_solveApproximateUnitQuadratic (
                                    &cosTheta[0], &sinTheta[0],
                                    &cosTheta[1], &sinTheta[1],
                                    coffPoint.z, coffPoint.x, coffPoint.y, s_toroidalQuadTol);
    *pNumPoint = 0;
    for (i = 0; i < numRoot; i++)
        {
        theta = bsiTrig_atan2 (sinTheta[i], cosTheta[i]);
        thetaPoint.x = cosTheta[i];
        thetaPoint.y = sinTheta[i];
        thetaPoint.z = theta;
        pPoint[i].phiPoint = phiPoint;
        pPoint[i].thetaPoint = thetaPoint;
        *pNumPoint += 1;
        }
    return SUCCESS;
    }

/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            08/97
*
*  Find the 0,1, or 2 points where an isoline crosses the contour.
+----------------------------------------------------------------------*/
static int     tpa_addSliceAtPhi

(
ToroidalPointArray *pArray,
RotMatrixCP pMatrix,
double           phi,
ParamRange       *pRange0,
ParamRange       *pRange1,
int              mask
)
    {
    ToroidalPoint tPoint[2];
    int numPoint;
    int i;
    jmdlRotatedConic_sliceToroidalFormAtPhi (tPoint, &numPoint, pMatrix, phi);

    for (i = 0; i < numPoint; i++)
        {
        if (tp_inRanges (&tPoint[i], pRange0, pRange1))
            {
            tPoint[i].mask = mask;
            tpa_addTPoint (pArray, &tPoint[i]);
            }
        }
    return numPoint;
    }

/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            08/97
*
*  Find the 0,1, or 2 points where an isoline crosses the contour.
+----------------------------------------------------------------------*/
static int     tpa_addSliceAtTheta

(
ToroidalPointArray *pArray,
RotMatrixCP pMatrix,
double           theta,
ParamRange       *pRange0,
ParamRange       *pRange1,
int              mask
)
    {
    ToroidalPoint tPoint[2];
    int numPoint;
    int i;
    jmdlRotatedConic_sliceToroidalFormAtTheta (tPoint, &numPoint, pMatrix, theta);

    for (i = 0; i < numPoint; i++)
        {
        if (tp_inRanges (&tPoint[i], pRange0, pRange1))
            {
            tPoint[i].mask = mask;
            tpa_addTPoint (pArray, &tPoint[i]);
            }
        }
    return numPoint;
    }

/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            08/97
*
*  Interval is to be subdivided into steps numbered 0..lastIndex.
| If interval is tiny, lastIndex=0.                                     |
+----------------------------------------------------------------------*/
static void    gridSteps

(
double *pStep,
int    *pLastIndex,
double x0,
double x1,
double baseSize         /* Assumed nonzero */
)
    {
    static double s_singlePointRelTol = 1.0e-8;
    double delta = x1 - x0;

    if (baseSize < 0.0)
        baseSize = fabs (baseSize);

    if (fabs (delta) < s_singlePointRelTol)
        {
        *pLastIndex = 0;
        *pStep = baseSize;
        }
    else if (fabs (delta) <= baseSize)
        {
        *pLastIndex = 1;
        *pStep = delta;
        }
    else
        {
        *pLastIndex = 1 + (int) fabs(delta / baseSize);
        *pStep = delta / *pLastIndex;
        }
    }

/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            08/97
*
+----------------------------------------------------------------------*/
static double evalGridStep

(
double x0,
double x1,
int     index,
int    lastIndex,
double step
)
    {
    double x;
    if (index == 0)
        {
        x = x0;
        }
    else if (index == lastIndex)
        {
        x = x1;
        }
    else
        {
        x = x0 + index * step;
        }
    return x;
    }

/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            07/97
*
*  Find zeros of a toroidal form within a band in which:
*  1) theta varies freely from theta0 to theta1
*  2) phi is single valued within the band limits phiA to phiB
*  3) phi values from phi0 to phi1 are kept
+----------------------------------------------------------------------*/
static StatusInt jmdlRotatedConic_toroidalBlock

(
RotMatrixCP pMatrix,
double      theta0,     /* Overall theta limit */
double      theta1,     /* Overall theta limit */
double      phi0,       /* Overall phi limit */
double      phi1,       /* Overall phi limit */
double      phiA,       /* Band phi limit */
double      phiB,       /* Band phi limit. ASSUMED phiB > phiA */
double      maxDTheta,
double      maxDPhi,
ToroidalOutputFunction outputFunc,
void        *pOutputData
)
    {

    StatusInt status = SUCCESS;
    ToroidalPointArray tPoints;

    ParamRange  patchRange;
    ParamRange  bandRange;
    //double      thetaSweep = theta1 - theta0;
    //double      phiSweep   = phi1   - phi0;
    double      theta, phi;
    double      step;

    int i, numInterval;


    if (maxDTheta <= MINIMUM_ANGLE_STEP)
        maxDTheta = MINIMUM_ANGLE_STEP;
    if (maxDPhi <= MINIMUM_ANGLE_STEP)
        maxDPhi = MINIMUM_ANGLE_STEP;

    jmdlParamRange_initFromAngleLimits (&patchRange, theta0, theta1, phi0, phi1);
    jmdlParamRange_initFromAngleLimits (&bandRange, theta0, theta1, phiA, phiB);

    tpa_clear (&tPoints);
    tpa_setOutputFunction (&tPoints, outputFunc, pOutputData);

    gridSteps (&step, &numInterval, theta0, theta1, maxDTheta);
    for (i = 0; i <= numInterval; i++)
        {
        theta = theta0 + i * step;
        if (i == numInterval)
            theta = theta1;             /* prevent roundoff problems */
        tpa_addSliceAtTheta (&tPoints, pMatrix, theta, &patchRange, &bandRange, 0);
        }

    gridSteps (&step, &numInterval, phiA, phiB, maxDPhi);
    for (i = 0; i <= numInterval; i++)
        {
        phi = phiA + i * step;
        if (i == numInterval)
            phi = phiB;         /* prevent roundoff problems */
        tpa_addSliceAtPhi (&tPoints, pMatrix, phi, &patchRange, &bandRange, 0);
        }

    tpa_normalizeAndSortTheta
                    (
                    &tPoints,
                    &patchRange,
                    s_toroidalFilterFraction * maxDTheta,
                    s_toroidalFilterFraction * maxDPhi
                    );

    status = tpa_output (&tPoints);
    return status;
    }

/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            07/97
*
*  Find zeros of a toroidal form within a band in which:
*  1) theta varies freely from theta0 to theta1
*  2) phi is single valued within the band limits phiA to phiB
*  3) phi values from phi0 to phi1 are kept
+----------------------------------------------------------------------*/
static StatusInt jmdlRotatedConic_toroidalBand

(
RotMatrixCP pMatrix,
double      theta0,     /* Overall theta limit */
double      theta1,     /* Overall theta limit */
double      phi0,       /* Overall phi limit */
double      phi1,       /* Overall phi limit */
double      phiA,       /* Band phi limit */
double      phiB,       /* Band phi limit. ASSUMED phiB > phiA */
double      maxDTheta,
double      maxDPhi,
ToroidalOutputFunction outputFunc,
void        *pOutputData
)
    {

    StatusInt status = SUCCESS;
    ToroidalPoint breakPointM[2];
    ToroidalPoint breakPointA[2];
    ToroidalPoint breakPointB[2];
    double thetaBreak[6];
    int numBreakA, numBreakB, numBreak, numBreakM;
    double      thetaSweep = theta1 - theta0;
    //double      phiSweep   = phi1   - phi0;
    double      phiAB = phiB - phiA;
    double      theta, phiM0, phiM1;
    double      thetaC, thetaD, thetaM;
    double      smallAngle = bsiTrig_smallAngle ();

    int i;

    /* We know ...
    On a full torus with 'banded' contours: in each band the contours wander around theta with phi varying
    in the general shape of a sine curve --- one crossing of any constant theta line, two if any constant
    phi line.

    When dealing with a partial band, the partial bounds phiA and phiB can cut the contour in 4 points.
    Draw vertical (constant theta) lines at each of these points.
    Also find the crossings of
    */

    if (   SUCCESS == jmdlRotatedConic_sliceToroidalFormAtPhi (breakPointM, &numBreakM, pMatrix, phiA + 0.5 * phiAB)
        && numBreakM == 2
        && SUCCESS == jmdlRotatedConic_sliceToroidalFormAtPhi (breakPointA, &numBreakA, pMatrix, phiA)
        && SUCCESS == jmdlRotatedConic_sliceToroidalFormAtPhi (breakPointB, &numBreakB, pMatrix, phiB)
        )
        {
        /* The middle phi value has crossings. */
        numBreak = 0;
        thetaBreak[numBreak++] = theta0;
        thetaBreak[numBreak++] = theta1;
        for (i = 0; i < numBreakA; i++)
            {
            theta = breakPointA[i].thetaPoint.z;
            if (bsiTrig_angleInSweep (theta, theta0, thetaSweep))
                thetaBreak[numBreak++] = bsiTrig_adjustAngleToSweep (theta, theta0, thetaSweep);
            }

        for (i = 0; i < numBreakB; i++)
            {
            theta = breakPointB[i].thetaPoint.z;
            if (bsiTrig_angleInSweep (theta, theta0, thetaSweep))
                thetaBreak[numBreak++] = bsiTrig_adjustAngleToSweep (theta, theta0, thetaSweep);
            }


        bsiDoubleArray_sort (thetaBreak, numBreak, true);
        for (i = 1; i < numBreak; i++)
            {
            thetaC = thetaBreak[i-1];
            thetaD = thetaBreak[i];
            thetaM = 0.5 * (thetaC + thetaD);
            if (   fabs (thetaC - thetaD) > smallAngle
                && SUCCESS == jmdlRotatedConic_sliceToroidalFormAtTheta
                            (breakPointM, &numBreakM, pMatrix, thetaM)
                && numBreakM == 2
                )
                {
                phiM0 = breakPointM[0].phiPoint.z;
                phiM1 = breakPointM[1].phiPoint.z;
                if  (  bsiTrig_angleInSweep (phiM0, phiA, phiAB)
                    || bsiTrig_angleInSweep (phiM1, phiA, phiAB)
                    )
                    jmdlRotatedConic_toroidalBlock (
                            pMatrix,
                            thetaC,
                            thetaD,
                            phi0,
                            phi1,
                            phiA,
                            phiB,
                            maxDTheta,
                            maxDPhi,
                            outputFunc,
                            pOutputData);
                }
            }
        }
    return status;
    }

/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            09/97
*
+----------------------------------------------------------------------*/
static int     jmdlRotatedConic_processThetaBands

(
const RotatedConic    *pSurface,
RotMatrixCP pMatrix,       /* => toroidal form coefficients */
double                theta0,
double                theta1,
double                phi0,
double                phi1,
DPoint3dP pPhiLimit,     /* => phi limits. Must be dimensioned at least 3 larger to accomodate
surface and wraparound limits */
int                     numPhiLimit,    /* => number of limits */
ToroidalOutputFunction  outputFunc,      /* => output handler */
void                    *pOutputParams
)
    {
    static double bandTolerance = 0.05;
    double phiMid, phiA, phiB;
    int band;
    StatusInt bandStatus;

    double phiSweep, thetaSweep;

    thetaSweep  = theta1 - theta0;

    phiSweep    = phi1 - phi0;

    /* Two complete theta loops, each with restricted phi range */
    if (!bsiTrig_isAngleFullCircle (phiSweep))
        {
        /* Only the internal band breaks matter */
        int k = 0;
        for (band = 0; band < numPhiLimit; band++)
            {
            double phiBand = bsiTrig_adjustAngleToSweep (pPhiLimit[band].z, phi0, phiSweep);
            if (bsiTrig_angleInSweep (phiBand, phi0, phiSweep))
                pPhiLimit[k++].z = bsiTrig_adjustAngleToSweep (pPhiLimit[band].z, phi0, phiSweep);
            }

        pPhiLimit[k++].z = phi0;
        pPhiLimit[k++].z = phi1;
        numPhiLimit = k;
        qsort (pPhiLimit, numPhiLimit, sizeof (DPoint3d),
                (int (*)(const void *, const void *))compareZ);
        }
    else
        {
        qsort (pPhiLimit, numPhiLimit, sizeof (DPoint3d),
                (int (*)(const void *, const void *))compareZ);
        pPhiLimit[numPhiLimit++].z = pPhiLimit[0].z + msGeomConst_2pi;
        }

    bandStatus = SUCCESS;
    for (band = 1; SUCCESS == bandStatus && band < numPhiLimit; band++)
        {
        phiA = pPhiLimit[band-1].z;
        phiB = pPhiLimit[band].z;
        phiMid = 0.5 * (phiA + phiB);
        if (bsiTrig_angleInSweep (phiMid, phi0, phiSweep))
            bandStatus = jmdlRotatedConic_toroidalBand
                            (
                            pMatrix,
                            theta0,
                            theta1,
                            phi0,
                            phi1,
                            phiA,
                            phiB,
                            bandTolerance, bandTolerance,
                            outputFunc,
                            pOutputParams
                            );
        }
    return SUCCESS;
    }

/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            09/97
*
+----------------------------------------------------------------------*/
static void fillCriticalAngles

(
double      *pAngle,
int         *pNumAngle,
DPoint3dP pPoint,           /* => candidate angles as cos,sin,theta */
int         numPoint,
double      angle0,         /* => patch limit */
double      angle1          /* => patch limit */
)
    {
    double sweep = angle1 - angle0;
    int i, numAngle;

    numAngle = 0;
    if (numPoint == 0)
        {
        /* Just fill in the patch limits */
        pAngle[numAngle++] = angle0;
        pAngle[numAngle++] = angle1;

        }
    else if (bsiTrig_isAngleFullCircle (sweep))
        {
        /* Sort all limit angles, and add a wraparound */

        for (i = 0; i < numPoint; i++)
            pAngle[numAngle++] = pPoint[i].z;

        bsiDoubleArray_sort (pAngle, numAngle, true);
        pAngle[numAngle++] = pAngle[0] + msGeomConst_2pi;
        }
    else
        {
        /* Sort limit angles within patch limits */
        pAngle[numAngle++] = angle0;
        pAngle[numAngle++] = angle1;

        for (i = 0; i < numPoint; i++)
            {
            if (bsiTrig_angleInSweep (pPoint[i].z, angle0, sweep))
                pAngle[numAngle++] = pPoint[i].z;
            }

        bsiDoubleArray_sort (pAngle, numAngle, true);
        }
    *pNumAngle = numAngle;
    }

/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            09/97
*
+----------------------------------------------------------------------*/
static void tpa_addGrid

(
ToroidalPointArray  *pPointArray,
RotMatrixCP pMatrix,
double              startAngle,
double              endAngle,
int (*gridFunc)(ToroidalPointArray *, const RotMatrix *, double, ParamRange *, ParamRange *, int),
double              maxStepSize,
int                 mask
)
    {
    int i, numInterval;
    double step, angle;

    gridSteps (&step, &numInterval, startAngle, endAngle, maxStepSize);
    for (i = 0; i <= numInterval; i++)
        {
        angle = evalGridStep (startAngle, endAngle, i, numInterval, step);
        gridFunc (pPointArray, pMatrix, angle, NULL, NULL, mask);
        }
    }

/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            09/97
*
+----------------------------------------------------------------------*/
static int tpa_locateThetaBand /* Index of band containing ALL points */

(
ToroidalPointArray  *pPointArray,
double              *pThetaLimit,       /* => Limits of theta bands */
int                 numLimit            /* => one more than number of bands */
)
    {
    int band, i;
    int numPoint = pPointArray->numPoint;
    bool    allInBand;
    double thetaStart, thetaSweep, theta;

    for (band = 1; band < numLimit; band++)
        {
        thetaStart = pThetaLimit[band-1];
        thetaSweep = pThetaLimit[band] - thetaStart;
        allInBand = true;
        for (i = 0; i < numPoint && allInBand; i++)
            {
            theta = pPointArray->pointPair[i].thetaPoint.z;
            if (!bsiTrig_angleInSweep (theta, thetaStart, thetaSweep))
                {
                allInBand = false;
                }
            }
        if (allInBand)
            return band - 1;
        }
    return -1;
    }

/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            09/97
*
+----------------------------------------------------------------------*/
static int     jmdlRotatedConic_processBubbles

(
const RotatedConic    *pSurface,
RotMatrixCP pMatrix,       /* => toroidal form coefficients */
double                theta0,
double                theta1,
double                phi0,
double                phi1,
DPoint3dP pThetaLimit,     /* => phi limits. Must be dimensioned at least 3 larger to accomodate
surface and wraparound limits */
int                   numThetaLimit,    /* => number of limits */
DPoint3dP pPhiLimit,       /* => phi limits. Must be dimensioned at least 3 larger to accomodate
surface and wraparound limits */
int                     numPhiLimit,    /* => number of limits */
ToroidalOutputFunction  outputFunc,     /* => output handler */
void                    *pOutputData
)
    {
    static double bandTolerance = 0.10;
    double phiCrit  [10];
    double thetaCrit[10];
    int numPhiCrit, numThetaCrit, j;
    double phiMid, thetaMid;
    ToroidalPointArray tPoints;
    ToroidalPoint breakPoint[4];
    int numBreak, thetaBand;
    double phiStart, phiEnd, thetaStart, thetaEnd;
    double smallAngle = bsiTrig_smallAngle ();

    double phiSweep, thetaSweep;
    double thetaBandSweep;
    double phiBandSweep;

    double maxDTheta = bandTolerance;
    double maxDPhi   = bandTolerance;
    thetaSweep  = theta1 - theta0;

    phiSweep    = phi1 - phi0;

    /* Get the raw bubble limits */
    fillCriticalAngles (thetaCrit, &numThetaCrit, pThetaLimit, numThetaLimit, 0.0, msGeomConst_2pi);
    fillCriticalAngles (phiCrit, &numPhiCrit, pPhiLimit, numPhiLimit, 0.0, msGeomConst_2pi);

    for (j = 1; j < numPhiCrit; j++)
        {
        phiStart = phiCrit[j-1];
        phiEnd   = phiCrit[j];
        phiBandSweep = phiEnd - phiStart;
        phiMid = 0.5 * (phiStart + phiEnd);
        jmdlRotatedConic_sliceToroidalFormAtPhi (breakPoint, &numBreak, pMatrix, phiMid);
        if (numBreak > 0 && fabs (phiStart - phiEnd) > smallAngle)
            {
            tpa_clear (&tPoints);
            tpa_setOutputFunction (&tPoints, outputFunc, pOutputData);

            tpa_addGrid (&tPoints, pMatrix, phiStart, phiEnd, tpa_addSliceAtPhi, maxDPhi, 0);

            if (0 <= (thetaBand = tpa_locateThetaBand (&tPoints, thetaCrit, numThetaCrit)))
                {
                thetaStart = thetaCrit[thetaBand];
                thetaEnd   = thetaCrit[thetaBand+1];
                thetaBandSweep = thetaEnd - thetaStart;
                thetaMid = 0.5 * (thetaStart + thetaEnd);

                tpa_addGrid (&tPoints, pMatrix,
                                    thetaStart, thetaEnd,
                                    tpa_addSliceAtTheta,
                                    maxDPhi,
                                    0);

                if (bsiTrig_angleInSweep (theta0, thetaStart, thetaBandSweep))
                    tpa_addSliceAtTheta (&tPoints, pMatrix, theta0, NULL, NULL, TP_BOUNDARY_POINT);
                if (bsiTrig_angleInSweep (theta1, thetaStart, thetaBandSweep))
                    tpa_addSliceAtTheta (&tPoints, pMatrix, theta1, NULL, NULL, TP_BOUNDARY_POINT);

                if (bsiTrig_angleInSweep (phi0, phiStart, phiBandSweep))
                    tpa_addSliceAtPhi (&tPoints, pMatrix, phi0, NULL, NULL, TP_BOUNDARY_POINT);
                if (bsiTrig_angleInSweep (phi1, phiStart, phiBandSweep))
                    tpa_addSliceAtPhi (&tPoints, pMatrix, phi1, NULL, NULL, TP_BOUNDARY_POINT);

                tpa_normalizeToAngleLimits (&tPoints, thetaStart, thetaEnd, phiStart, phiEnd);
                tpa_centralSort (&tPoints, thetaMid, phiMid);

                tpa_filter  (
                            &tPoints,
                            s_toroidalFilterFraction * maxDTheta,
                            s_toroidalFilterFraction * maxDPhi
                            );
                tpa_close (&tPoints);
                tpa_output (&tPoints);
                }
            }
        }



    return SUCCESS;
    }

#define MAX_SILHOUETTE_BUFFER 200
/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            07/97
*
*  Trace solution contours of the commonly occurring equation
*    (c s 1)[ acC acS ac1 ] (C) = 0
*           [ asC asS as1 ] (S)
*           [ a1C a1S a11 ] (1)
*  for c=cos(phi), s=sin(phi), C=cos(theta), S=sin(theta)
*  within the limits of the surface.
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt    bsiRotatedConic_toroidalForm

(
const RotatedConic  *pSurface,      /* => the rotated conic surface */
RotMatrixCP pMatrix,       /* => coefficient matrix */
SilhouetteArrayHandler handlerFunc, /* => callback for points */
double              tolerance,      /* => distance from surface */
void                *pUserData      /* => arbitrary pointer */
)
    {
    StatusInt status = ERROR;
    double a,b;
    double theta0, theta1, thetaSweep, phi0, phi1, phiSweep;

    DPoint3d thetaLimit[8], phiLimit[8];
    int numThetaLimit, numPhiLimit;
    ToroidalOutputParams outputParams;

    outputParams.pSurface   = pSurface;
    outputParams.handlerFunc = handlerFunc;
    outputParams.pUserData  = pUserData;

    if (!pSurface || RC_Torus != pSurface->type)
        return ERROR;

    /* Torus sizes, correspond to analysis on paper */
    a = 1.0;
    b = pSurface->hoopRadius;

    theta0 = pSurface->parameterRange.low.x;
    theta1 = pSurface->parameterRange.high.x;
    thetaSweep = theta1 - theta0;

    phi0 = pSurface->parameterRange.low.y;
    phi1 = pSurface->parameterRange.high.y;
    phiSweep = phi1 - phi0;


    bsiRotatedConic_toroidalFormLimits (thetaLimit, &numThetaLimit, pMatrix, false);
    bsiRotatedConic_toroidalFormLimits (phiLimit, &numPhiLimit, pMatrix, true);

    if (numThetaLimit == 0 && numPhiLimit == 4)
        {
        jmdlRotatedConic_processThetaBands (pSurface, pMatrix,
                                        theta0, theta1, phi0, phi1,
                                        phiLimit, numPhiLimit,
                                        (ToroidalOutputFunction)outputThetaPhi, &outputParams);
        }
    else if (numPhiLimit == 0 && numThetaLimit == 4)
        {
        RotMatrix transpose;
        transpose.TransposeOf (*pMatrix);
        /* Use the logic for theta bands, but with transposed matrix and all phi-theta references
            exchanged, and an output function that undoes the exchange. */
        jmdlRotatedConic_processThetaBands (pSurface, &transpose,
                                        phi0, phi1, theta0, theta1,
                                        thetaLimit, numThetaLimit,
                                        (ToroidalOutputFunction)outputPhiTheta, &outputParams);
        }
    else
        {
        jmdlRotatedConic_processBubbles (pSurface, pMatrix,
                                                theta0, theta1,
                                                phi0,   phi1,
                                                thetaLimit, numThetaLimit,
                                                phiLimit, numPhiLimit,
                                                (ToroidalOutputFunction)outputThetaPhi, &outputParams);
        }

    return status;
    }


/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            07/97
*
*  Compute general silhouette curve of a torus.
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt    bsiRotatedConic_torusGeneralSilhouette

(
const RotatedConic  *pSurface,      /* => the rotated conic surface */
DPoint4dCP pEyePoint,     /* => eyepoint (nonzero weight) or view vector (zero weight) */
SilhouetteArrayHandler handlerFunc, /* => callback for points */
double              tolerance,      /* => distance from surface */
void                *pUserData      /* => arbitrary pointer */
)
    {
    DPoint4d localEyePoint;
    double a,b;
    RotMatrix coffMatrix;

    if (!pSurface || RC_Torus != pSurface->type)
        return ERROR;

    bsiRotatedConic_transformDPoint4dArray (&localEyePoint, pEyePoint, 1, pSurface, RC_COORDSYS_world, RC_COORDSYS_local);

    /* Torus sizes, correspond to analysis on paper */
    a = 1.0;
    b = pSurface->hoopRadius;

    coffMatrix.SetColumn (DVec3d::From (-localEyePoint.x, 0.0, 0.0), 0);
    coffMatrix.SetColumn (DVec3d::From (-localEyePoint.y, 0.0, 0.0), 1);
    coffMatrix.SetColumn (DVec3d::From (a * localEyePoint.w, -localEyePoint.z, b * localEyePoint.w), 2);

    return bsiRotatedConic_toroidalForm (pSurface, &coffMatrix,
                                handlerFunc, tolerance, pUserData);
    }

/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            07/97
*
*  Compute general plane intersection with a torus.
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt    bsiRotatedConic_torusGeneralIntersectPlane

(
const RotatedConic  *pSurface,      /* => the rotated conic surface */
DPoint4dCP pPlane,        /* => plane coordinates */
SilhouetteArrayHandler handlerFunc, /* => callback for points */
double              tolerance,      /* => distance from surface */
void                *pUserData      /* => arbitrary pointer */
)
    {
    DPoint4d h;
    double a,b;
    RotMatrix coffMatrix;

    if (!pSurface || RC_Torus != pSurface->type)
        return ERROR;

    bsiDMatrix4d_multiplyTransposePoints (&pSurface->rotationMap.M0, &h, pPlane, 1);

    /* Torus sizes, correspond to analysis on paper */
    a = 1.0;
    b = pSurface->hoopRadius;

    coffMatrix.SetColumn (DVec3d::From (b * h.x, 0.0, a * h.x), 0);
    coffMatrix.SetColumn (DVec3d::From (b * h.y, 0.0, a * h.y), 1);
    coffMatrix.SetColumn (DVec3d::From (0.0, b * h.z, h.w), 2);

    return bsiRotatedConic_toroidalForm (pSurface, &coffMatrix,
                                handlerFunc, tolerance, pUserData);
    }

/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            07/97
*
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt       bsiRotatedConic_extendRangeByPoints

(
HConic              *pConic,
DPoint3dP pPointArray,
int                 numPoint,
unsigned int        curveMask,
const RotatedConic  *pSurface,      /* => the rotated conic surface */
DRange3dP pRange            /* <= surface range */
)
    {
    bsiDRange3d_extendByDPoint3dArray (pRange, pPointArray, numPoint);
    return SUCCESS;
    }

/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            07/97
*
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt       bsiRotatedConic_getRange

(
DRange3dP pRange,           /* <= surface range */
const RotatedConic  *pSurface       /* => the rotated conic surface */
)
    {
    StatusInt status = ERROR;
    switch (pSurface->type)
        {
        case RC_RotatedLine:
        case RC_Cylinder:
        case RC_Cone:
            {
            HConic top, bottom;
            DRange3d topRange, bottomRange;
            bsiRotatedConic_boundaryComponents (&bottom, &top, NULL, NULL, pSurface);
            if (   SUCCESS == bsiHConic_getRange (&bottomRange, &bottom)
                && SUCCESS == bsiHConic_getRange (&topRange, &top)
               )
                {
                *pRange = bottomRange;
                bsiDRange3d_extendByRange (pRange, &topRange);
                status = SUCCESS;
                }
            }
            break;
        case RC_Disk:
            {
            HConic arc0, arc1;
            DRange3d range0, range1;
            bsiRotatedConic_boundaryComponents (&arc0, &arc1, NULL, NULL, pSurface);
            bsiDRange3d_init (pRange);

            if (   SUCCESS == bsiHConic_getRange (&range0, &arc0))
                {
                bsiDRange3d_extendByRange (pRange, &range0);
                status = SUCCESS;
                }

            if (   SUCCESS == bsiHConic_getRange (&range1, &arc1))
                {
                bsiDRange3d_extendByRange (pRange, &range1);
                status = SUCCESS;
                }

            }
            break;
        case RC_Sphere:
            {
            HConic boundary[4];
            HConic silhouette[4];
            int numSilhouette;

            bsiRotatedConic_boundaryComponents (boundary, boundary + 1, boundary + 2, boundary + 3, pSurface);

            bsiDRange3d_init (pRange);
            bsiDRange3d_extendByHConicArray (pRange, boundary, 4);

            bsiRotatedConic_interiorConicSilhouette (silhouette, &numSilhouette, pSurface, &s_point1000_4d);
            bsiDRange3d_extendByHConicArray (pRange, silhouette, numSilhouette);
            bsiRotatedConic_interiorConicSilhouette (silhouette, &numSilhouette, pSurface, &s_point0100_4d);
            bsiDRange3d_extendByHConicArray (pRange, silhouette, numSilhouette);
            bsiRotatedConic_interiorConicSilhouette (silhouette, &numSilhouette, pSurface, &s_point0010_4d);
            bsiDRange3d_extendByHConicArray (pRange, silhouette, numSilhouette);
            }
            break;
        case RC_Plane:
            {
            static DPoint3d localCorner[4] =
                {
                    { 0.0, 0.0, 0.0 },
                    { 1.0, 0.0, 0.0 },
                    { 0.0, 1.0, 0.0 },
                    { 1.0, 1.0, 0.0 }
                };
            DPoint3d worldCorner[4];
            bsiDMatrix4d_multiplyAndRenormalizeDPoint3dArray (&pSurface->rotationMap.M0, worldCorner, localCorner, 4);
            bsiDRange3d_initFromArray (pRange, worldCorner, 4);
            status = SUCCESS;
            }
            break;
        case RC_Torus:
            {
            HConic boundary[4];

            bsiRotatedConic_boundaryComponents (boundary, boundary + 1, boundary + 2, boundary + 3, pSurface);

            bsiDRange3d_init (pRange);
            bsiDRange3d_extendByHConicArray (pRange, boundary, 4);

            bsiRotatedConic_torusGeneralSilhouette (pSurface, &s_point1000_4d, (SilhouetteArrayHandler)bsiRotatedConic_extendRangeByPoints, 0.0, pRange);
            bsiRotatedConic_torusGeneralSilhouette (pSurface, &s_point0100_4d, (SilhouetteArrayHandler)bsiRotatedConic_extendRangeByPoints, 0.0, pRange);
            bsiRotatedConic_torusGeneralSilhouette (pSurface, &s_point0010_4d, (SilhouetteArrayHandler)bsiRotatedConic_extendRangeByPoints, 0.0, pRange);
            }
            break;
        }
    return status;
    }

/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            07/97
*
*  Compute products of the form
*           XT*A*Y
*  where X,Y are homogeneous vectors
*  and A is the homogeneous matrix of the surface.
*  Return ERROR if the surface is not (pre)classified as a quadrtic
*
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt       bsiRotatedConic_quadraticProducts

(
double              *pXTAX,         /* <= */
double              *pXTAY,         /* <= */
double              *pYTAY,         /* <= */
DPoint4dP pBinvX,           /* <= X transformed to local system */
DPoint4dP pBinvY,           /* <= Y transformed to local system */
const RotatedConic  *pSurface,      /* => the rotated conic surface */
DPoint4dCP pX,      /* => X point */
DPoint4dCP pY               /* => Y point */
)
    {
    StatusInt status = ERROR;


    switch (pSurface->type)
        {
        case RC_Sphere:
            bsiDMatrix4d_multiply4dPoints (&pSurface->rotationMap.M1, pBinvX, pX, 1);
            bsiDMatrix4d_multiply4dPoints (&pSurface->rotationMap.M1, pBinvY, pY, 1);
            *pXTAX = bsiQuadric_sphereProduct (pBinvX, pBinvX);
            *pXTAY = bsiQuadric_sphereProduct (pBinvX, pBinvY);
            *pYTAY = bsiQuadric_sphereProduct (pBinvY, pBinvY);
            status = SUCCESS;
            break;
        case RC_Cylinder:
            bsiDMatrix4d_multiply4dPoints (&pSurface->rotationMap.M1, pBinvX, pX, 1);
            bsiDMatrix4d_multiply4dPoints (&pSurface->rotationMap.M1, pBinvY, pY, 1);
            *pXTAX = bsiQuadric_cylinderProduct (pBinvX, pBinvX);
            *pXTAY = bsiQuadric_cylinderProduct (pBinvX, pBinvY);
            *pYTAY = bsiQuadric_cylinderProduct (pBinvY, pBinvY);
            status = SUCCESS;
            break;
        case RC_Cone:
            bsiDMatrix4d_multiply4dPoints (&pSurface->rotationMap.M1, pBinvX, pX, 1);
            bsiDMatrix4d_multiply4dPoints (&pSurface->rotationMap.M1, pBinvY, pY, 1);
            *pXTAX = bsiQuadric_coneProduct (pBinvX, pBinvX);
            *pXTAY = bsiQuadric_coneProduct (pBinvX, pBinvY);
            *pYTAY = bsiQuadric_coneProduct (pBinvY, pBinvY);
            status = SUCCESS;
            break;
        }
    return status;
    }

/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            07/97
*
|
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool       bsiRotatedConic_isQuadric   /* false for non-quadric */

(
DPoint4dP pSigma,           /* <= vector of 0,1,-1 values on diagonal of characteristic matrix. */
const RotatedConic  *pSurface       /* => the rotated conic surface */
)
    {
    bool    isQuadric = false;
    switch (pSurface->type)
        {
        case RC_Sphere:
            if (pSigma)
                bsiDPoint4d_setComponents (pSigma, 1.0, 1.0, 1.0, -1.0);
            isQuadric = true;
            break;
        case RC_Cylinder:
            if (pSigma)
                bsiDPoint4d_setComponents (pSigma, 1.0, 1.0, 0.0, -1.0);
            isQuadric = true;
            break;
        case RC_Cone:
            if (pSigma)
                bsiDPoint4d_setComponents (pSigma, 1.0, 1.0, -1.0,  0.0);
            isQuadric = true;
            break;
        }
    return isQuadric;
    }
/* REMARK: Move to hmatrix file after MSJ merge. */
/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                                7/96
*
*  Copy data from a matrix row to a DPoint4d structure.
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void bsiDMatrix4d_getRowDPoint4d

(
DPoint4dP pVec,      /* <= point copied from column */
DMatrix4dCP pMatrix,   /* => matrix whose column is retrieved */
int i                       /* => index of column (0 <= i < 4 ) whose values are to be set */
)
    {
    if ( i >= 0 && i < 4 )
        {
        pVec->x = pMatrix->coff[i][0];
        pVec->y = pMatrix->coff[i][1];
        pVec->z = pMatrix->coff[i][2];
        pVec->w = pMatrix->coff[i][3];
        }
    }


/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            07/97
*
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool       bsiRotatedConic_isPlanar   /* false for non-planar */

(
DPoint4dP pPlaneCoffs,   /* => coefficients of plane normal vector */
const RotatedConic  *pSurface       /* => the rotated conic surface */
)
    {
    bool    isPlanar = false;
    switch (pSurface->type)
        {
        case RC_Disk:
        case RC_Plane:
            if (pPlaneCoffs)
                bsiDMatrix4d_getRowDPoint4d (pPlaneCoffs, &pSurface->rotationMap.M1, 2);
            isPlanar = true;
            break;
        }
    return isPlanar;
    }

/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            07/97
*
|
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt       bsiRotatedConic_mappedBasis

(
DMap4dP pHMap,      /* <= Mapping from pSurface0 local coordinates to
pSurface1 local coordinates */
const RotatedConic  *pSurface0,     /* => foreign surface */
const RotatedConic  *pSurface1      /* => rotated conic whose local system is being used
for calculations */
)
    {
    bsiDMap4d_multiplyInverted (pHMap,
                        &pSurface1->rotationMap, true,
                        &pSurface0->rotationMap, false);
    return SUCCESS;
    }


/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            07/97
*
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int bsiRotatedConic_uFlags  /* bitwise OR of bits for curvature properties */

(
const RotatedConic  *pSurface       /* => the rotated conic surface */
)
    {
    int result = 0;
    /* BRANCH_ON_SURFACE_TYPE */
    switch (pSurface->type)
        {
        case RC_RotatedLine:
        case RC_RotatedEllipse:
            result = RC_PeriodicFunctionMask | RC_CurvedMask;
            if (bsiTrig_isAngleFullCircle (pSurface->sweep))
                result |= RC_FullParamterRangeMask;
            break;

        case RC_Sphere:
        case RC_Torus:
        case RC_Cone:
        case RC_Disk:
        case RC_Cylinder:
            result = RC_PeriodicFunctionMask | RC_CurvedMask;
            if (bsiTrig_isAngleFullCircle (pSurface->parameterRange.high.x - pSurface->parameterRange.low.x))
                result |= RC_FullParamterRangeMask;
            break;
        }
    return result;
    }

/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            07/97
*
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int bsiRotatedConic_vFlags  /* bitwise OR of bits for curvature properties */

(
const RotatedConic  *pSurface       /* => the rotated conic surface */
)
    {
    int result = 0;
    /* BRANCH_ON_SURFACE_TYPE */
    switch (pSurface->type)
        {
        case RC_RotatedEllipse:
            result = RC_PeriodicFunctionMask | RC_CurvedMask;
            if (bsiDEllipse4d_isFullEllipse(&pSurface->conic))
                result |= RC_FullParamterRangeMask;
            break;

        case RC_Sphere:
        case RC_Torus:
            result = RC_PeriodicFunctionMask | RC_CurvedMask;
            if (bsiTrig_isAngleFullCircle (pSurface->parameterRange.high.y - pSurface->parameterRange.low.y))
                result |= RC_FullParamterRangeMask;
            break;
        }
    return result;
    }

/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            07/97
*
*  Return boundary components in local coordinates.
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt       bsiRotatedConic_localBoundaryComponents

(
HConic      *pBottom,               /* <= bottom conic.  NULL if not required */
HConic      *pTop,                  /* <= top conic      NULL if not required */
HConic      *pLeft,                 /* <= left conic     NULL if not required */
HConic      *pRight,                /* <= right conic    NULL if not required */
const RotatedConic  *pSurface,      /* => the rotated conic surface */
bool        orientForSweep          /* => if true, orient opposite sides with parameterizations for sweeping.
If false, orient parameterizations for a CCW boundary traversal */
)
    {
    StatusInt status = ERROR;
    DPoint3d point0A, point0B, point1A, point1B;
    double c0, s0, c1, s1, theta0, theta1, zA, zB, rA, rB;
    double phi0, phi1, cPhi0, sPhi0, cPhi1, sPhi1;
    //double tol = bsiTrig_smallAngle ();

    /* BRANCH_ON_SURFACE_TYPE */
    switch (pSurface->type)
        {
        case RC_Plane:
            {
            if (orientForSweep)
                {
                bsiHConic_initLine4d (pBottom, &s_point0001_4d, &s_point1001_4d);
                bsiHConic_initLine4d (pTop,    &s_point0101_4d, &s_point1101_4d);
                bsiHConic_initLine4d (pRight,   &s_point1001_4d, &s_point1101_4d);
                bsiHConic_initLine4d (pLeft,   &s_point0001_4d, &s_point0101_4d);
                }
            else
                {
                bsiHConic_initLine4d (pBottom, &s_point0001_4d, &s_point1001_4d);
                bsiHConic_initLine4d (pRight,   &s_point1001_4d, &s_point1101_4d);
                bsiHConic_initLine4d (pTop,     &s_point1101_4d, &s_point0101_4d);
                bsiHConic_initLine4d (pLeft,    &s_point0101_4d, &s_point0001_4d);
                }
            status = SUCCESS;
            }
            break;
        case RC_RotatedLine:
            {
            DPoint4d h0A, h0B, h1A, h1B;
            DPoint4d h2A, h2B, h3A, h3B;
            DEllipse4d ellipseA, ellipseB;

            h0A = pSurface->conic.vector0;
            h0B = pSurface->conic.vector90;
            c1 = cos (pSurface->sweep);
            s1 = sin (pSurface->sweep);
            jmdlDPoint4d_rotateXY (&h1A, &h0A, c1, s1);
            jmdlDPoint4d_rotateXY (&h1B, &h0B, c1, s1);


            h3A = h0A;
            h3B = h0B;
            h3A.x = h3A.y = 0.0;
            h3B.x = h3B.y = 0.0;
            h0A.w = h0B.w = h0A.z = h0B.z = 0.0;

            jmdlDPoint4d_rotateXY (&h2A, &h0A, 0.0, 1.0);
            jmdlDPoint4d_rotateXY (&h2B, &h0B, 0.0, 1.0);

            bsiDEllipse4d_initFrom4dVectors (&ellipseA, &h3A, &h0A, &h2A, 0.0, pSurface->sweep);
            bsiHConic_initLine4d (pRight, &h1A, &h1B);

            if (orientForSweep)
                {
                bsiDEllipse4d_initFrom4dVectors (&ellipseB, &h3B, &h0B, &h2B, 0.0, pSurface->sweep);
                bsiHConic_initLine4d (pLeft,  &h0A, &h0B);
                }
            else
                {
                bsiDEllipse4d_initFrom4dVectors (&ellipseB, &h3B, &h0B, &h2B, pSurface->sweep, -pSurface->sweep);
                bsiHConic_initLine4d (pLeft,  &h0B, &h0A);
                }

            bsiHConic_initHEllipse (pBottom, &ellipseA);
            bsiHConic_initHEllipse (pTop, &ellipseB);
            status = SUCCESS;
            }
            break;

        case RC_Cylinder:
        case RC_Cone:
            theta0 = pSurface->parameterRange.low.x;
            theta1 = pSurface->parameterRange.high.x;
            c0 = cos (theta0);
            s0 = sin (theta0);
            c1 = cos (theta1);
            s1 = sin (theta1);
            zA = pSurface->parameterRange.low.y;
            zB = pSurface->parameterRange.high.y;

            if (pSurface->type == RC_Cone)
                {
                rA = zA;
                rB = zB;
                }
            else
                {
                rA = rB = 1.0;
                }

            point0A.Init ( rA * c0, rA * s0, zA);
            point0B.Init ( rB * c0, rB * s0, zB);
            point1A.Init ( rA * c1, rA * s1, zA);
            point1B.Init ( rB * c1, rB * s1, zB);


            if (orientForSweep)
                {
                bsiHConic_initZEllipse (pBottom, theta0, theta1, rA, zA);
                bsiHConic_initLine (pRight, &point1A, &point1B);
                bsiHConic_initZEllipse (pTop, theta0, theta1, rB, zB);
                bsiHConic_initLine (pLeft,  &point0A, &point0B);
                }
            else
                {
                bsiHConic_initZEllipse (pBottom, theta0, theta1, rA, zA);
                bsiHConic_initLine (pRight, &point1A, &point1B);
                bsiHConic_initZEllipse (pTop, theta1, theta0, rB, zB);
                bsiHConic_initLine (pLeft,  &point0B, &point0A);
                }
            status = SUCCESS;
            break;

        case RC_Sphere:
            {
            theta0 = pSurface->parameterRange.low.x;
            theta1 = pSurface->parameterRange.high.x;
            phi0   = pSurface->parameterRange.low.y;
            phi1   = pSurface->parameterRange.high.y;

            cPhi0   = cos (phi0);
            sPhi0   = sin (phi0);
            cPhi1   = cos (phi1);
            sPhi1   = sin (phi1);

            /* Yowzers.  fabs (cPhi0) is usually not the social thing to do. However, it is supposed
                to be a RADIUS, hence positive. */

            if (orientForSweep)
                {
                bsiHConic_initZEllipse (pBottom, theta0, theta1, fabs (cPhi0), sPhi0);
                bsiHConic_initMeridian (pRight, phi0, phi1, theta1, 1.0, 0.0);
                bsiHConic_initZEllipse (pTop, theta0, theta1, fabs (cPhi1), sPhi1);
                bsiHConic_initMeridian (pLeft,  phi0, phi1, theta0, 1.0, 0.0);
                }
            else
                {
                bsiHConic_initZEllipse (pBottom, theta0, theta1, fabs (cPhi0), sPhi0);
                bsiHConic_initMeridian (pRight, phi0, phi1, theta1, 1.0, 0.0);
                bsiHConic_initZEllipse (pTop, theta1, theta0, fabs (cPhi1), sPhi1);
                bsiHConic_initMeridian (pLeft,  phi1, phi0, theta0, 1.0, 0.0);
                }

            status = SUCCESS;
            }
            break;

        case RC_Disk:
            theta0 = pSurface->parameterRange. low.x;
            theta1 = pSurface->parameterRange.high.x;

            rA     = pSurface->parameterRange. low.y;
            rB     = pSurface->parameterRange.high.y;

            c0 = cos (theta0);
            s0 = sin (theta0);
            c1 = cos (theta1);
            s1 = sin (theta1);
            zA = 0.0;
            zB = 0.0;

            point0A.Init ( rA * c0, rA * s0, zA);
            point0B.Init ( rB * c0, rB * s0, zB);

            point1A.Init ( rA * c1, rA * s1, zA);
            point1B.Init ( rB * c1, rB * s1, zB);

            bsiHConic_initNull (pBottom);
            bsiHConic_initNull (pTop);
            if (orientForSweep)
                {
                bsiHConic_initZEllipse (pBottom, theta0, theta1, rA, zA);
                bsiHConic_initLine (pRight, &point1A, &point1B);
                bsiHConic_initZEllipse (pTop, theta0, theta1, rB, zB);
                bsiHConic_initLine (pLeft,  &point0A, &point0B);
                }
            else
                {
                bsiHConic_initZEllipse (pBottom, theta0, theta1, rA, zA);
                bsiHConic_initLine (pRight, &point1A, &point1B);
                bsiHConic_initZEllipse (pTop, theta1, theta0, rB, zB);
                bsiHConic_initLine (pLeft,  &point0B, &point0A);
                }
            status = SUCCESS;
            break;

        case RC_Torus:
            {
            double a0, a1;              /* radial coordinates at phi0, phi1 */
            double z0, z1;              /* z coordinates at phi0, phi1 */
            double hoopRadius = pSurface->hoopRadius;
            theta0 = pSurface->parameterRange.low.x;
            theta1 = pSurface->parameterRange.high.x;
            phi0   = pSurface->parameterRange.low.y;
            phi1   = pSurface->parameterRange.high.y;

            cPhi0   = cos (phi0);
            sPhi0   = sin (phi0);
            cPhi1   = cos (phi1);
            sPhi1   = sin (phi1);

            a0 = 1.0 + hoopRadius * cPhi0;
            a1 = 1.0 + hoopRadius * cPhi1;
            z0 = hoopRadius * sPhi0;
            z1 = hoopRadius * sPhi1;

            if (orientForSweep)
                {
                bsiHConic_initZEllipse (pBottom, theta0, theta1, a0, z0);
                bsiHConic_initMeridian (pRight, phi0, phi1, theta1, hoopRadius, 1.0);
                bsiHConic_initZEllipse (pTop, theta0, theta1, a1, z1);
                bsiHConic_initMeridian (pLeft,  phi0, phi1, theta0, hoopRadius, 1.0);
                }
            else
                {
                bsiHConic_initZEllipse (pBottom, theta0, theta1, a0, z0);
                bsiHConic_initMeridian (pRight, phi0, phi1, theta1, hoopRadius, 1.0);
                bsiHConic_initZEllipse (pTop, theta1, theta0, a1, z1);
                bsiHConic_initMeridian (pLeft,  phi1, phi0, theta0, hoopRadius, 1.0);
                }
            status = SUCCESS;
            }
            break;
        default:
            bsiHConic_initNull (pBottom);
            bsiHConic_initNull (pTop);
            bsiHConic_initNull (pRight);
            bsiHConic_initNull (pLeft);
            break;
        }

    return status;
    }
/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            07/97
*
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt       bsiRotatedConic_boundaryComponents

(
HConic      *pBottom,               /* <= bottom conic.  NULL if not required */
HConic      *pTop,                  /* <= top conic      NULL if not required */
HConic      *pLeft,                 /* <= left conic     NULL if not required */
HConic      *pRight,                /* <= right conic    NULL if not required */
const RotatedConic  *pSurface       /* => the rotated conic surface */
)
    {
    StatusInt status = bsiRotatedConic_localBoundaryComponents (pBottom, pTop, pLeft, pRight, pSurface, false);
    if (SUCCESS == status)
        {
        bsiHConic_multiplyHMatrix (pBottom,     pBottom,    &pSurface->rotationMap.M0);
        bsiHConic_multiplyHMatrix (pTop,        pTop,       &pSurface->rotationMap.M0);
        bsiHConic_multiplyHMatrix (pLeft,       pLeft,      &pSurface->rotationMap.M0);
        bsiHConic_multiplyHMatrix (pRight,      pRight,     &pSurface->rotationMap.M0);
        }
    return status;
    }

/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            07/97
*
*  Evaluate 4D tangent (partial derivative) vectors.
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt   bsiRotatedConic_tangents

(
DPoint4dP pTangent0,    /* <= partial wrt x of (4d) parametric coordinates */
      DPoint4d      *pTangent1, /* <= partial wrt y of (4d) parametric coordinates */
      DPoint4d      *pTangent2, /* <= partial wrt z of (4d) parametric coordinates */
      DPoint4d      *pTangent3, /* <= partial wrt w of (4d) parametric coordinates */
const RotatedConic  *pSurface,  /* => surface to evaluate */
const DPoint4d      *pParam,    /* => parameter space coordinates.  (E.g. as returned by
                                        bsiRotatedConic_transformPointArray) */
      int           coordSys    /* => coordinates in which tangents are computed. May be
                                        RC_COORDSYS_world
                                        RC_COORDSYS_local
                                */
)
    {
    StatusInt status = ERROR;

    if (pSurface && coordSys == RC_COORDSYS_world || coordSys == RC_COORDSYS_local)
        {
        switch (pSurface->type)
            {
            case RC_Disk:
                bsiQuadric_diskPartials(pTangent0, pTangent1, pTangent2, pTangent3, pParam);
                status = SUCCESS;
                break;
            case RC_Torus:
                bsiQuadric_torusPartials (pTangent0, pTangent1, pTangent2, pTangent3,
                                pParam, pSurface->hoopRadius);
                status = SUCCESS;
                break;
            case RC_Cylinder:
                bsiQuadric_cylinderPartials (pTangent0, pTangent1, pTangent2, pTangent3,
                                pParam);
                status = SUCCESS;
                break;
            case RC_Cone:
                bsiQuadric_conePartials (pTangent0, pTangent1, pTangent2, pTangent3,
                                pParam);
                status = SUCCESS;
                break;
            case RC_Sphere:
                bsiQuadric_spherePartials (pTangent0, pTangent1, pTangent2, pTangent3,
                                pParam);
                status = SUCCESS;
                break;
            }

        if (SUCCESS == status && coordSys == RC_COORDSYS_world)
            {
            if (pTangent0)
                bsiDMatrix4d_multiply4dPoints (&pSurface->rotationMap.M0, pTangent0, pTangent0, 1);
            if (pTangent1)
                bsiDMatrix4d_multiply4dPoints (&pSurface->rotationMap.M0, pTangent1, pTangent1, 1);
            if (pTangent2)
                bsiDMatrix4d_multiply4dPoints (&pSurface->rotationMap.M0, pTangent2, pTangent2, 1);
            if (pTangent3)
                bsiDMatrix4d_multiply4dPoints (&pSurface->rotationMap.M0, pTangent3, pTangent3, 1);
            }

        }
    return status;
    }

/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            07/97
*
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt   bsiRotatedConic_transformDPoint4dArray

(
DPoint4dP pOut,      /* <= returned points */
DPoint4dCP pIn,       /* => input points */
int         numPoint,   /* => number of points */
const RotatedConic  *pSurface,  /* => surface */
int         inSys,      /* => input coordinate system */
int         outSys      /* => output coordinate system */
)
    {
    StatusInt status = ERROR;
    int i;
    double theta, alpha;
    if (outSys == inSys || !pSurface)
        {
        memcpy (pOut, pIn, numPoint * sizeof (DPoint4d));
        status = SUCCESS;
        }
    else if (inSys == RC_COORDSYS_world)
        {
        if (outSys == RC_COORDSYS_local)
            {
            bsiDMatrix4d_multiply4dPoints (&pSurface->rotationMap.M1, pOut, pIn, numPoint);
            status = SUCCESS;
            }
        else if (outSys == RC_COORDSYS_parameter || outSys == RC_COORDSYS_parameter01)
            {
            bsiDMatrix4d_multiply4dPoints (&pSurface->rotationMap.M1, pOut, pIn, numPoint);
            status = bsiRotatedConic_transformDPoint4dArray (pOut, pOut, numPoint, pSurface, RC_COORDSYS_local, outSys);
            }
        }
    else if (inSys == RC_COORDSYS_local)
        {
        if (outSys == RC_COORDSYS_world)
            {
            bsiDMatrix4d_multiply4dPoints (&pSurface->rotationMap.M0, pOut, pIn, numPoint);
            status = SUCCESS;
            }
        else if (outSys == RC_COORDSYS_parameter || outSys == RC_COORDSYS_parameter01)
            {
            status = SUCCESS;
            /* BRANCH_ON_SURFACE_TYPE */
            switch (pSurface->type)
                {
                case RC_Cone:
                    for (i = 0; i < numPoint; i++)
                        bsiQuadric_cartesianToCone (pOut + i, pIn + i);
                    break;
                case RC_Cylinder:
                    for (i = 0; i < numPoint; i++)
                        bsiQuadric_cartesianToCylinder (pOut + i, pIn + i);
                    break;
                case RC_Sphere:
                    for (i = 0; i < numPoint; i++)
                        bsiQuadric_cartesianToSphere (pOut + i, pIn + i);
                    break;
                case RC_Disk:
                    for (i = 0; i < numPoint; i++)
                        bsiQuadric_cartesianToDisk (pOut + i, pIn + i);
                    break;
                case RC_Torus:
                    for (i = 0; i < numPoint; i++)
                        bsiQuadric_cartesianToTorus (pOut + i, pIn + i, pSurface->hoopRadius);
                    break;
                case RC_Plane:
                    /* parameter space is local space */
                    memcpy (pOut, pIn, numPoint * sizeof(DPoint4d));
                    break;
                }

            if (outSys == RC_COORDSYS_parameter01 && SUCCESS == status)
                {
                status = bsiRotatedConic_transformDPoint4dArray (pOut, pOut, numPoint, pSurface, RC_COORDSYS_parameter, outSys);
                }
            }
        }
    else if (inSys == RC_COORDSYS_parameter )
        {
        if (outSys == RC_COORDSYS_local)
            {

            status = SUCCESS;
            /* BRANCH_ON_SURFACE_TYPE */
            switch (pSurface->type)
                {
                case RC_Cone:
                    for (i = 0; i < numPoint; i++)
                        bsiQuadric_coneToCartesian (pOut + i, pIn + i);
                    break;
                case RC_Cylinder:
                    for (i = 0; i < numPoint; i++)
                        bsiQuadric_cylinderToCartesian (pOut + i, pIn + i);
                    break;
                case RC_Sphere:
                    for (i = 0; i < numPoint; i++)
                        bsiQuadric_sphereToCartesian (pOut + i, pIn + i);
                    break;
                case RC_Disk:
                    for (i = 0; i < numPoint; i++)
                        bsiQuadric_diskToCartesian (pOut + i, pIn + i);
                    break;
                case RC_Plane:
                    /* parameter space is local space */
                    memcpy (pOut, pIn, numPoint * sizeof(DPoint4d));
                    break;
                case RC_Torus:
                    for (i = 0; i < numPoint; i++)
                        bsiQuadric_torusToCartesian (pOut + i, pIn + i, pSurface->hoopRadius);
                    break;
                }
            }
        else if (outSys == RC_COORDSYS_world)
            {
            if (SUCCESS == bsiRotatedConic_transformDPoint4dArray (pOut, pIn, numPoint, pSurface, inSys, RC_COORDSYS_local))
                {
                bsiDMatrix4d_multiply4dPoints (&pSurface->rotationMap.M0, pOut, pOut, numPoint);
                status = SUCCESS;
                }
            }
        else if (outSys == RC_COORDSYS_parameter01)
            {
            double theta0 = pSurface->parameterRange.low.x;
            double dtheta = pSurface->parameterRange.high.x - theta0;
            double alpha0 = pSurface->parameterRange.low.y;
            double dalpha = pSurface->parameterRange.high.y - alpha0;

            if (dtheta != 0.0 && dalpha != 0.0)
                {
                double dalpha_inv = 1.0 / dalpha;
                double dtheta_inv = 1.0 / dtheta;
                /* BRANCH_ON_SURFACE_TYPE */
                switch (pSurface->type)
                    {
                    case RC_Plane:
                        {
                        for (i = 0; i < numPoint; i++)
                            {
                            theta = pIn[i].x;
                            alpha = pIn[i].y;
                            pOut[i].x = (theta - theta0) * dtheta_inv;
                            pOut[i].y = (alpha - alpha0) * dalpha_inv;
                            pOut[i].z = pIn[i].z;
                            pOut[i].w = pIn[i].w;
                            }
                        status = SUCCESS;
                        }
                        break;
                    case RC_Cone:
                    case RC_Cylinder:
                    case RC_Disk:
                        {
                        for (i = 0; i < numPoint; i++)
                            {
                            theta = pIn[i].x;
                            alpha = pIn[i].y;
                            pOut[i].x = bsiTrig_normalizeAngleToSweep (theta, theta0, dtheta);
                            pOut[i].y = (alpha - alpha0) * dalpha_inv;
                            pOut[i].z = pIn[i].z;
                            pOut[i].w = pIn[i].w;
                            }
                        status = SUCCESS;
                        break;
                        }
                    case RC_Torus:
                    case RC_Sphere:
                        {
                        for (i = 0; i < numPoint; i++)
                            {
                            theta = pIn[i].x;
                            alpha = pIn[i].y;
                            pOut[i].x = bsiTrig_normalizeAngleToSweep (theta, theta0, dtheta);
                            pOut[i].y = bsiTrig_normalizeAngleToSweep (alpha, alpha0, dalpha);
                            pOut[i].z = pIn[i].z;
                            pOut[i].w = pIn[i].w;
                            }
                        status = SUCCESS;
                        break;
                        }
                    }
                }
            }
        }
    else if (inSys == RC_COORDSYS_parameter01)
        {
        /* Convert to parameter, recurse to go as needed to get from there to somewhere else */
        double theta0 = pSurface->parameterRange.low.x;
        double dtheta = pSurface->parameterRange.high.x - theta0;
        double alpha0 = pSurface->parameterRange.low.y;
        double dalpha = pSurface->parameterRange.high.y - alpha0;
        for (i = 0; i < numPoint; i++)
            {
            pOut[i].x = theta0 + pIn[i].x * dtheta;
            pOut[i].y = alpha0 + pIn[i].y * dalpha;
            pOut[i].z = pIn[i].z;
            pOut[i].w = pIn[i].w;
            }
        if (outSys != RC_COORDSYS_parameter)
            {
            status = bsiRotatedConic_transformDPoint4dArray (pOut, pOut, numPoint, pSurface, RC_COORDSYS_parameter, outSys);
            }
        else
            {
            status = SUCCESS;
            }
        }

    return status;
    }

/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            06/00
*
*  Pull a (world) point back to the surface.  For natural quadrics,
*  this is finding the closest point.  For others, it finds the
*  intersection of a line back to the center.
*  The retraction is NOT restricted to the bounded surface.
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt   bsiRotatedConic_retractDPoint3dToSurface

(
const RotatedConic  *pSurface,  /* => surface */
DPoint3dP pOut,
DPoint3dCP pIn,
int         inSys,
int         outSys,
bool        retractParameterXY,
bool        retractParameterZ
)
    {
    DPoint4d worldPoint, facePoint;
    int status;
    worldPoint.InitFrom (*pIn, 1.0);
    status = bsiRotatedConic_retractDPoint4dToSurface
                (pSurface, &facePoint, &worldPoint, inSys, outSys, retractParameterXY, retractParameterZ);
    if (SUCCESS != status
        || !facePoint.GetProjectedXYZ (*pOut))
        {
        status = ERROR;
        }
    return status;
    }

/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            06/00
*
*  Pull a (world) point back to the surface.  For natural quadrics,
*  this is finding the closest point.  For others, it finds the
*  intersection of a line back to the center.
*  The retraction is NOT restricted to the bounded surface.
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt   bsiRotatedConic_retractDPoint4dToSurface

(
const RotatedConic  *pSurface,  /* => surface */
DPoint4dP pOut,
DPoint4dCP pIn,
int         inSys,
int         outSys,
bool          retractParameterXY,
bool          retractParameterZ
)
    {
    DPoint4d parameterPoint;
    StatusInt status = bsiRotatedConic_transformDPoint4dArray
            (&parameterPoint, pIn, 1, pSurface, inSys, RC_COORDSYS_parameter01);
    if (SUCCESS == status)
        {
        if (retractParameterZ)
            {
            switch (pSurface->type)
                {
                case RC_Cone:
                case RC_Cylinder:
                case RC_Torus:
                case RC_Sphere:
                    parameterPoint.z = 1.0;
                    break;
                case RC_Disk:
                case RC_Plane:
                    parameterPoint.z = 0.0;
                    break;
                }
            }

        if (retractParameterXY)
            {
            if (parameterPoint.x < 0.0)
                parameterPoint.x = 0.0;
            if (parameterPoint.x > 1.0)
                parameterPoint.x = 1.0;
            if (parameterPoint.y < 0.0)
                parameterPoint.y = 0.0;
            if (parameterPoint.y > 1.0)
                parameterPoint.y = 1.0;
            }

        status = bsiRotatedConic_transformDPoint4dArray (pOut, &parameterPoint, 1, pSurface, RC_COORDSYS_parameter01, outSys);
        }
    return status;
    }

/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            07/97
*
*  Compute general silhouette curves.
*  Returns SUCCESS (but no output!!!) if exact conics can be computed
*  from bsiRotatedConic_interiorConicSilhouette
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt    bsiRotatedConic_generalSilhouette

(
const RotatedConic  *pSurface,      /* => the rotated conic surface */
DPoint4dCP pEyePoint,     /* => eyepoint (nonzero weight) or view vector (zero weight) */
SilhouetteArrayHandler handlerFunc, /* => callback for points */
double              tolerance,      /* => distance from surface */
void                *pUserData      /* => arbitrary pointer */
)
    {
    StatusInt status = ERROR;

    /* BRANCH_ON_SURFACE_TYPE */
    switch (pSurface ? pSurface->type : RC_NullSurface)
        {
        case RC_Cone:
        case RC_Sphere:
        case RC_Plane:
        case RC_Disk:
        case RC_Cylinder:
            {
            status = SUCCESS;
            }
            break;
        case RC_Torus:
            {
            return bsiRotatedConic_torusGeneralSilhouette
                        (
                        pSurface,
                        pEyePoint,
                        handlerFunc,
                        tolerance,
                        pUserData
                        );
            }
        }
    return status;
    }



/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            07/97
*
*  Compute (if possible!!!) silhouette curves as exact conics.
*  Note special status return if silhouette is non-conic (e.g. for
*  torus).
*  Return values are:
*  SUCCESS - pNumSilhouette indicates number of conics returned.
*           (pNumSilhouette==0 is a valid return with SUCCESS.)
*  ERROR   - no support for silhouettes.
*  STATUS_ROTATEDCONIC_NONCONIC_SILHOUETTE - Silhouettes must be
*           computed as non-conic curves.
*
*  Returned curves are strictly 'interior' -- no overlap with
*  boundaries.
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt    bsiRotatedConic_interiorConicSilhouette

(
HConic      *pSilhouette,   /* <= 0, 1, or 2 sihouette conics.  Note for HConic_Ellipse, the ellipse
may have multiple arc sectors. */
int         *pNumSilhouette,
const RotatedConic  *pSurface,      /* => the rotated conic surface */
DPoint4dCP pEyePoint      /* => eyepoint (nonzero weight) or view vector (zero weight) */
)
    {
    DPoint4d localEye;
    StatusInt status = ERROR;
    int numSolution;
    int numSilhouette;
    DPoint2d trigPoint[2];
    int i;
    double c, s, theta;
    double theta0, theta1, sweep, alpha0, alpha1;
    double angleTol = bsiTrig_smallAngle();
    int    fullCircle;
    DPoint4d linePoint[2];
    int     surfaceType = RC_NullSurface;
    *pNumSilhouette = 0;

    /* BRANCH_ON_SURFACE_TYPE */
    switch (pSurface ? (surfaceType = pSurface->type) : RC_NullSurface)
        {
        case RC_Cone:
        case RC_Cylinder:
            {
            double exy;
            double ez;
            bsiRotatedConic_transformDPoint4dArray (&localEye, pEyePoint, 1, pSurface, RC_COORDSYS_world, RC_COORDSYS_local);
            /* z >> xy in eye vector means eye is on axis,
                hence no silhouette lines for either cone or cylinder,
                and regardless of weight.
            */
            ez = fabs (localEye.z);
            exy = fabs (localEye.x * localEye.x + localEye.y * localEye.y);
            if (exy <= angleTol * ez)
                {
                *pNumSilhouette = 0;
                status = SUCCESS;
                }
            else
                {
                numSolution = bsiMath_solveApproximateUnitQuadratic (
                    &trigPoint[0].x, &trigPoint[0].y,
                    &trigPoint[1].x, &trigPoint[1].y,
                    surfaceType == RC_Cone ? -localEye.z : -localEye.w,
                    localEye.x, localEye.y,
                    s_lineUnitCircleIntersectionTolerance
                    );

                theta0 = pSurface->parameterRange.low.x;
                theta1 = pSurface->parameterRange.high.x;
                sweep  = theta1 - theta0;
                fullCircle = bsiTrig_isAngleFullCircle (sweep);

                alpha0 = pSurface->parameterRange.low.y;
                alpha1 = pSurface->parameterRange.high.y;

                for (numSilhouette = 0, i = 0; i < numSolution; i++)
                    {
                    c = trigPoint[i].x;
                    s = trigPoint[i].y;
                    theta = bsiTrig_atan2 (s, c);
                    if (fullCircle ||
                        (   fabs (theta - theta0) > angleTol
                         && fabs (theta - theta1) > angleTol
                         && bsiTrig_angleInSweep (theta, theta0, sweep))
                        )
                        {
                        if (surfaceType == RC_Cone)
                            {
                            bsiDPoint4d_setComponents (linePoint    , alpha0 * c, alpha0 * s, alpha0, 1.0);
                            bsiDPoint4d_setComponents (linePoint + 1, alpha1 * c, alpha1 * s, alpha1, 1.0);
                            }
                        else /* RC_Cylinder */
                            {
                            bsiDPoint4d_setComponents (linePoint    , c, s, alpha0, 1.0);
                            bsiDPoint4d_setComponents (linePoint + 1, c, s, alpha1, 1.0);
                            }

                        bsiRotatedConic_transformDPoint4dArray (
                                            linePoint,
                                            linePoint, 2,
                                            pSurface,
                                            RC_COORDSYS_local,
                                            RC_COORDSYS_world
                                            );
                        bsiHConic_initLine4d (pSilhouette + numSilhouette, linePoint, linePoint + 1);
                        numSilhouette++;
                        }
                    }
                *pNumSilhouette = numSilhouette;
                status = SUCCESS;
                }
            }
            break;
        case RC_Sphere:
            {
            DEllipse4d silhouette, clippedSilhouette;
            bsiRotatedConic_transformDPoint4dArray (&localEye, pEyePoint, 1, pSurface, RC_COORDSYS_world, RC_COORDSYS_local);
            if (   bsiGeom_ellipsoidSilhouette (&silhouette, NULL, NULL, &localEye)
                && SUCCESS == jmdlDEllipse4d_sphericalClip (&clippedSilhouette, &silhouette, &pSurface->parameterRange)
                && clippedSilhouette.sectors.n > 0)
                {
                /* Debugging assists:
                    set noClip to force the full silhouette ellipse out all the time
                    set showMeridians to force out the equator and the 0+90 degree meridians (unclipped)
                */
                static bool    noClip = 0;
                static bool    showMeridians = 0;
                if (noClip)
                    clippedSilhouette = silhouette;
                bsiDEllipse4d_multiplyByHMatrix (&clippedSilhouette, &clippedSilhouette, &pSurface->rotationMap.M0);
                bsiHConic_initHEllipse (pSilhouette, &clippedSilhouette);
                numSilhouette = 1;
                *pNumSilhouette = numSilhouette;

                if (showMeridians)
                    {
                    /* Useful for debugging */
                    bsiHConic_initZEllipse (pSilhouette + 1, 0.0, msGeomConst_2pi, 1.0, 0.0);
                    bsiHConic_initMeridian (pSilhouette + 2, 0.0, msGeomConst_2pi,                 0.0, 1.0, 0.0);
                    bsiHConic_initMeridian (pSilhouette + 3, 0.0, msGeomConst_2pi, msGeomConst_piOver2, 1.0, 0.0);
                    bsiHConic_multiplyHMatrix (pSilhouette + 1, pSilhouette + 1, &pSurface->rotationMap.M0);
                    bsiHConic_multiplyHMatrix (pSilhouette + 2, pSilhouette + 2, &pSurface->rotationMap.M0);
                    bsiHConic_multiplyHMatrix (pSilhouette + 3, pSilhouette + 3, &pSurface->rotationMap.M0);
                    *pNumSilhouette = 4;

                    }

                status = SUCCESS;
                }
            }
            break;
        case RC_Plane:
        case RC_Disk:
            status = SUCCESS;
            break;

        case RC_Torus:
            {
            double axy, azw;
            static double eyeOnZTol = 1.0e-4;
            double a = 1.0;
            double b = pSurface->hoopRadius;

            double thetaA = pSurface->parameterRange.low.x;
            double thetaB = pSurface->parameterRange.high.x;

            double phiA   = pSurface->parameterRange.low.y;
            double phiB   = pSurface->parameterRange.high.y;

            bsiRotatedConic_transformDPoint4dArray (&localEye, pEyePoint, 1, pSurface, RC_COORDSYS_world, RC_COORDSYS_local);
            axy = sqrt (localEye.x * localEye.x + localEye.y * localEye.y);
            azw = sqrt (localEye.z * localEye.z + a * a *localEye.w * localEye.w);

            if (axy < eyeOnZTol * azw)
                {
                /* Looking down from a point on th ez axis */
                double alpha = localEye.w * b;
                double beta  = localEye.w * a;
                double gamma = -localEye.z;
                double cosPhi0, sinPhi0, cosPhi1, sinPhi1;
                int    numPhi = bsiMath_solveApproximateUnitQuadratic
                                            (
                                            &cosPhi0, &sinPhi0,
                                            &cosPhi1, &sinPhi1,
                                            alpha, beta, gamma,
                                            s_lineUnitCircleIntersectionTolerance
                                            );
                if (numPhi == 2)
                    {
                    double r0 = a + b * cosPhi0;
                    double z0 = b * sinPhi0;
                    double r1 = a + b * cosPhi1;
                    double z1 = b * sinPhi1;
                    double phi0 = bsiTrig_atan2 (sinPhi0, cosPhi0);
                    double phi1 = bsiTrig_atan2 (sinPhi1, cosPhi1);
                    int num = 0;

                    if (bsiTrig_angleInSweep (phi0, phiA, phiB - phiA))
                        {
                        bsiHConic_initZEllipse (pSilhouette + num, thetaA, thetaB, r0, z0);
                        bsiHConic_multiplyHMatrix (&pSilhouette[num], pSilhouette + num, &pSurface->rotationMap.M0);
                        num++;
                        }

                    if (bsiTrig_angleInSweep (phi1, phiA, phiB - phiA))
                        {
                        bsiHConic_initZEllipse (pSilhouette + num, thetaA, thetaB, r1, z1);
                        bsiHConic_multiplyHMatrix (&pSilhouette[num], pSilhouette + num, &pSurface->rotationMap.M0);
                        num++;
                        }
                    *pNumSilhouette = num;
                    status = SUCCESS;
                    }
                }
            else if (azw < eyeOnZTol * axy)
                {
                /* looking in from infinity on the xy plane */
                double thetaEye = bsiTrig_atan2 (localEye.y, localEye.x);
                double thetaStar[2], phiStar[2];
                int i;

                thetaStar[0] = thetaEye + msGeomConst_piOver2;
                thetaStar[1] = thetaEye - msGeomConst_piOver2;
                numSilhouette = 0;
                for (i = 0; i < 2; i++)
                    {
                    if (bsiTrig_angleInSweep (thetaStar[i], thetaA, thetaB - thetaA))
                        {
                        bsiHConic_initMeridian (&pSilhouette[numSilhouette++], phiA, phiB, thetaStar[i], b, a);
                        }
                    }

                phiStar[0] =  msGeomConst_piOver2;
                phiStar[1] = -msGeomConst_piOver2;

                for (i = 0; i < 2; i++)
                    {
                    if (bsiTrig_angleInSweep (phiStar[i], phiA, phiB - phiA))
                        {
                        bsiHConic_initZEllipse (
                                                &pSilhouette[numSilhouette++],
                                                thetaA,
                                                thetaB,
                                                a,
                                                phiStar[i] > 0.0 ? b : -b
                                                );
                        }
                    }

                for (i = 0; i < numSilhouette; i++)
                    {
                    bsiHConic_multiplyHMatrix (pSilhouette + i, pSilhouette + i, &pSurface->rotationMap.M0);
                    }
                *pNumSilhouette = numSilhouette;
                status = SUCCESS;

                }

            if (status != SUCCESS)
                {
                /* Do general silhouette calculation */
                }
            }
            break;
        }
    return status;
    }

/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            07/97
*
*  Test if point is within the surface box.  Only 2D surface parts
*  of point are examined.    (i.e. radial and weight parts are ignored)
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool        bsiRotatedConic_isPointOnSurfacePatch

(
DPoint4dCP pPoint,          /* <= coordinates of point to test */
int         inSys,          /* => coordinate system in which point is given.
RC_COORDSYS_world, RC_COORDSYS_local, RC_COORDSYS_parameter, RC_COORDSYS_parameter01
*/
const RotatedConic  *pSurface       /* => the rotated conic surface */
)
    {
    DPoint4d localPoint;
    double tol = bsiTrig_smallAngle ();
    double a0 = -tol;
    double a1 = 1.0 + tol;
    if (pSurface->type == RC_Sphere)
        {
        if (inSys == RC_COORDSYS_parameter01)
            {
            localPoint = *pPoint;
            return a0 <= localPoint.x && localPoint.x <= a1 && a0 <= localPoint.y && localPoint.y <= a1;
            }
        else
            {
            bsiRotatedConic_transformDPoint4dArray (&localPoint, pPoint, 1, pSurface, inSys, RC_COORDSYS_parameter);
            return jmdlRotatedConic_pointInSphericalRange (localPoint.x, localPoint.y, &pSurface->parameterRange);
            }
        }
    else
        {
        bsiRotatedConic_transformDPoint4dArray (&localPoint, pPoint, 1, pSurface, inSys, RC_COORDSYS_parameter01);
        return a0 <= localPoint.x && localPoint.x <= a1 && a0 <= localPoint.y && localPoint.y <= a1;
        }
    }

/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            07/97
*
+----------------------------------------------------------------------*/
static void expandLineIntersection

(
DPoint3dP pWorldPoint,      /* <= 0 to 4 cartesian world coordinates */
DPoint4dP pLocalPoint,           /* <= 0 to 4 homogeneous local coordinates of surface */
DPoint3dP pLineParameters,       /* <= 0 to 4 line parameters (x,y components weight point 0, point 1 */
int         *pNumIntersection,      /* <= number of intersections */
const RotatedConic *pSurface,       /* => surface */
DPoint4dCP pLocal0,         /* => local 0 point */
DPoint4dCP pLocal1,         /* => local 1 point */
double      lambda,                 /* => parameter */
bool        bTestZ,                 /* => enables test for valid z before incrementing */
double      z0,                     /* => desired z value. */
double      zTol                    /* => tolerance for z test */
)
    {
    int i = *pNumIntersection;
    DPoint4d localPoint, worldPoint, paramPoint;
    double lambda0 = 1.0 - lambda;
    int increment = 1;

    if (pLineParameters)
        {
        pLineParameters[i].x = 1.0 - lambda;
        pLineParameters[i].y = lambda;
        pLineParameters[i].z = 1.0;
        }

    bsiDPoint4d_add2ScaledDPoint4d (&localPoint, NULL, pLocal0, lambda0, pLocal1, lambda);

    if (pLocalPoint)
        pLocalPoint[i] = localPoint;

    if (pWorldPoint)
        {
        bsiDMatrix4d_multiply4dPoints (&pSurface->rotationMap.M0, &worldPoint, &localPoint, 1); worldPoint.GetProjectedXYZ (pWorldPoint[i]) ; /* THISWAS a bool thrown away as a statement */
        }

    if (bTestZ)
        {
        bsiRotatedConic_transformDPoint4dArray
                            (
                            &paramPoint, &localPoint, 1,
                            pSurface,
                            RC_COORDSYS_local,
                            RC_COORDSYS_parameter
                            );

        /* ummm. . does this have to be unweighted? */
        if (fabs (paramPoint.z - z0) > zTol)
            increment = 0;
        }
    *pNumIntersection += increment;
    }

/*
Clip an extended line segment to an-origin centered bounding box.
@param pOut0 OUT clipped start
@param pOut1 OUT clipped end
@param pGamma0 OUT parameter at clip start
@param pGamma1 OUT parameter at clip end
@param pIn0 IN original start
@param pIn1 IN original end
@param ax IN distance from origin to x clip
@param ay IN distance from origin to y clip
@param az IN distance from origin to z clip
@param alpha0 start parameter of (extended) line
@param alpha1 end parameter of (extended) line.  Assumed greater than alpha0!!!!
*/
static bool    clipLineToCenteredBox

(
DPoint4dP pOut0,
DPoint4dP pOut1,
double  *pGamma0,
double  *pGamma1,
DPoint4dP pIn0,
DPoint4dP pIn1,
double  ax,
double  ay,
double  az,
double alpha0,
double alpha1
)
    {
    DPoint4d planeCoffs[6];
    int numPlane = 0;
    /* Active range of clippee */
    double gamma0 = alpha0;
    double gamma1 = alpha1;
    double gamma;
    double h0, h1;
    int i;

    if (ax > 0.0)
        {
        bsiDPoint4d_setComponents (&planeCoffs[numPlane++],  1.0, 0.0, 0.0, -ax);
        bsiDPoint4d_setComponents (&planeCoffs[numPlane++], -1.0, 0.0, 0.0, -ax);
        }

    if (ay > 0.0)
        {
        bsiDPoint4d_setComponents (&planeCoffs[numPlane++], 0.0,  1.0, 0.0, -ay);
        bsiDPoint4d_setComponents (&planeCoffs[numPlane++], 0.0, -1.0, 0.0, -ay);
        }

    if (az > 0.0)
        {
        bsiDPoint4d_setComponents (&planeCoffs[numPlane++], 0.0, 0.0,  1.0, -az);
        bsiDPoint4d_setComponents (&planeCoffs[numPlane++], 0.0, 0.0, -1.0, -az);
        }


    for (i = 0; i < numPlane; i++)
        {

        h0 = bsiDPoint4d_dotProduct (&planeCoffs[i], pIn0);
        h1 = bsiDPoint4d_dotProduct (&planeCoffs[i], pIn1);

        if (bsiTrig_safeDivide (&gamma, -h0, h1 - h0, 0.0))
            {
            if (h1 > h0)
                {
                /* Line is heading outward -- plane can move the upper limit downwards. */
                if (gamma < gamma1)
                    gamma1 = gamma;
                }
            else
                {
                /* Line is heading outward -- plane can move the lower limit upwards. */
                if (gamma > gamma0)
                    gamma0 = gamma;
                }
            if (gamma0 >= gamma1)
                return false;
            }
        }

    *pGamma0 = gamma0;
    *pGamma1 = gamma1;
    bsiDPoint4d_interpolate (pOut0, pIn0, gamma0, pIn1);
    bsiDPoint4d_interpolate (pOut1, pIn0, gamma1, pIn1);
    return true;
    }

/*----------------------------------------------------------------*//**
@description Initialize the (exactly 3) coefficients of a quadratic bezier
    which is the square of linear interpolation between two values.
The linear interpolation is
    a0 * (1-u) + a1 * u
Squaring,
    a0^2 (1-u)^2 + 2 a0 a1 u (1-u) + a1^2 u^2
And this is the bezier polynomial with coefficients a0^2, a0*a1, a1^2
    (the factor of two on the middle coefficient goes into the basis function.)
@param pA OUT array of 3 coefficients.
@param a0 IN value at 0
@param a1 IN value at 1
+----------------------------------------------------------------------*/
static void initSquaredLinearBezier

(
double *pA,
double a0,
double a1
)
    {
    pA[0] = a0 * a0;
    pA[1] = a0 * a1;
    pA[2] = a1 * a1;
    }

/*----------------------------------------------------------------*//**
@description Initializes a (order 5, degree 4) polynomial for points along a line;
    this traces the implicit equation of a torus.
@param pA OUT 5 bezier coefficients
@param pOrder OUT the order (always 5)
@param pPoint0 IN start of line
@param pPoint1 IN end of line
@param hoopRadius IN the torus has major circle in xy plane with radius 1; hoopRadius is the other radius.
+----------------------------------------------------------------------*/
static void initLineUnitTorusBezier

(
double *pA,
int    *pOrder,
DPoint4dP pPoint0,
DPoint4dP pPoint1,
double   hoopRadius
)
    {
    double xx[3];
    double yy[3];
    double zz[3];
    double ww[3];
    double f2[3];
    double f4[5];
    double g2[3];
    double g4[5];

    double a;
    int i;

    initSquaredLinearBezier (xx, pPoint0->x, pPoint1->x);
    initSquaredLinearBezier (yy, pPoint0->y, pPoint1->y);
    initSquaredLinearBezier (zz, pPoint0->z, pPoint1->z);
    initSquaredLinearBezier (ww, pPoint0->w, pPoint1->w);

    a = 1.0 - hoopRadius * hoopRadius;
    /* Torus equation is
        x^2 + y^2 + z^2 - w^2 (1-hoopRadius^2) = 4 w^2 (x^2 + y^2)
       where x,y,z,w are all linear functions of the bezier parameter.
       Form up squares of the x,y,z,w interpolants.
       Add, square, and sum for complete polynomial.
    */
    for (i = 0; i < 3; i++)
        f2[i] = xx[i] + yy[i] + zz[i] + a * ww[i];

    for (i = 0; i < 3; i++)
        g2[i] = xx[i] + yy[i];

    bsiBezier_univariateProduct (f4, 0, 1, f2, 3, 0, 1, f2, 3, 0, 1);
    bsiBezier_univariateProduct (g4, 0, 1, g2, 3, 0, 1, ww, 3, 0, 1);

    for (i = 0; i < 5; i++)
        pA[i] = f4[i] - 4.0 * g4[i];

    *pOrder = 5;
    }

/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            07/97
*
*  Find the intersection of line
*   X=L0*alpha0 + L1*alpha1
*  with the analytic surface.
*
*  All intersections are returned (i.e. both line and surface are
*  unbounded)
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt       bsiRotatedConic_intersectLine

(
DPoint3dP pWorldPoint,      /* <= 0 to 4 cartesian world coordinates */
DPoint4dP pLocalPoint,           /* <= 0 to 4 homogeneous local coordinates of surface */
DPoint3dP pLineParameters,       /* <= 0 to 4 line parameters (x,y components weight point 0, point 1 */
int         *pNumIntersection,      /* <= number of intersections */
const RotatedConic    *pSurface,            /* => the rotated conic surface */
DPoint3dCP pLinePoints      /* => two points on the line */
)
    {
    StatusInt status = ERROR;
    int i;
    DPoint4d A0, A1;
    DPoint4d C0, C1;
    double f00, f01, f11;
    double alpha0[2], alpha1[2];

    bsiDPoint4d_copyAndWeight (&A0, pLinePoints    , 1.0);
    bsiDPoint4d_copyAndWeight (&A1, pLinePoints + 1, 1.0);
    *pNumIntersection = 0;

    /* BRANCH_ON_SURFACE_TYPE */
    switch (pSurface ? pSurface->type : RC_NullSurface)
        {
        case RC_Cone:
        case RC_Sphere:
        case RC_Cylinder:
            if (SUCCESS == bsiRotatedConic_quadraticProducts (&f00, &f01, &f11, &C0, &C1, pSurface, &A0, &A1))
                {
                int numRoot = bsiMath_solveConvexQuadratic (alpha0, alpha1, f00, 2.0 * f01, f11);

                for (i = 0; i < numRoot; i++)
                    {
                    if (pLineParameters)
                        {
                        pLineParameters[i].x = alpha0[i];
                        pLineParameters[i].y = alpha1[i];
                        pLineParameters[i].z = 1.0;
                        }
                    if (pLocalPoint)
                        bsiDPoint4d_add2ScaledDPoint4d (pLocalPoint + i , NULL, &C0, alpha0[i], &C1, alpha1[i]);
                    if (pWorldPoint)
                        bsiDPoint3d_add2ScaledDPoint3d (pWorldPoint + i , NULL, pLinePoints, alpha0[i], pLinePoints + 1, alpha1[i]);
                    }
                *pNumIntersection = numRoot;
                status = SUCCESS;
                }
            break;

        case RC_Plane:
        case RC_Disk:
            {
            DPoint4d B0, B1;
            double dz, limit;
            static double s_maxParam = 1.0e12;

            bsiDMatrix4d_multiply4dPoints (&pSurface->rotationMap.M1, &B0, &A0, 1);
            bsiDMatrix4d_multiply4dPoints (&pSurface->rotationMap.M1, &B1, &A1, 1);
            dz = B1.z - B0.z;
            limit = s_maxParam * fabs (dz);

            if (fabs (B0.z) < limit && fabs (B1.z) < limit)
                {
                double lambda = -B0.z / ( B1.z - B0.z);

                expandLineIntersection (
                                pWorldPoint, pLocalPoint, pLineParameters, pNumIntersection,
                                pSurface, &B0, &B1, lambda,
                                false, 0.0, 0.0);
                }
            status = SUCCESS;
            }
            break;

        case RC_Torus:
            {
            DPoint4d B0, B1;
            DPoint4d C0, C1;
            double fB, fC;
            double bExact = pSurface->hoopRadius;
            static double s_expansionFactor = 1.5;
            double bBox   = s_expansionFactor * bExact;
            static double s_offSurfaceTol = 1.0e-2;
            static double s_maxParam = 1.0e12;
            double gamma0, gamma1;
            bsiDMatrix4d_multiply4dPoints (&pSurface->rotationMap.M1, &B0, &A0, 1);
            bsiDMatrix4d_multiply4dPoints (&pSurface->rotationMap.M1, &B1, &A1, 1);
            /* Find the portion of the (effectively infinite) line which falls within a range
                box around the full torus
            */
            if (clipLineToCenteredBox (&C0, &C1, &gamma0, &gamma1, &B0, &B1,
                        1.0 + bBox, 1.0 + bBox, bBox, -s_maxParam, s_maxParam))
                {
                double localCoffs[10];
                double localRoots[10];
                int localOrder;
                int numLocalRoot;

                initLineUnitTorusBezier (localCoffs, &localOrder, &C0, &C1, bExact);
                bsiBezier_univariateRoots (localRoots, &numLocalRoot, localCoffs, localOrder);

                for (i = 0; i < numLocalRoot; i++)
                    {
                    /* Each root is a fractional paramter along C0..C1 */
                    fC = localRoots[i];
                    /* Convert fractional parameters from segment C0..C1 to segment B0..B1 */
                    fB = (1.0 - fC) * gamma0 + fC * gamma1;
                    expandLineIntersection (
                                        pWorldPoint, pLocalPoint, pLineParameters, pNumIntersection,
                                        pSurface, &B0, &B1, fB,
                                        true, 1.0, s_offSurfaceTol
                                        );
                    }
            }


            status = SUCCESS;
            }
            break;
        }

    return status;
    }

/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            04/99
*
+----------------------------------------------------------------------*/
static StatusInt   expandRationalCurveIntersection

(
DPoint4dP pWorldPoint,      /* <=> world coordinates of computed points. */
DPoint4dP pLocalPoint,           /* <=> local coordinates of computed points. */
double      *pParam,                /* <=> polynomial parameters */
int         *pNumIntersection,      /* <=> number of intersections.  May not be NULL */
int         maxIntersection,        /* => buffer limits */
const       PolyCoffs *pXGlobal,
const       PolyCoffs *pYGlobal,
const       PolyCoffs *pZGlobal,
const       PolyCoffs *pWGlobal,
const       PolyCoffs *pXLocal,
const       PolyCoffs *pYLocal,
const       PolyCoffs *pZLocal,
const       PolyCoffs *pWLocal,
double      param                   /* => parameter to store */
)
    {
    int i = *pNumIntersection;

    if (i >= maxIntersection)
        return ERROR;

    *pNumIntersection += 1;

    if (pParam)
        {
        pParam[i] = param;
        }

    if (pWorldPoint)
        {
        bsiDPoint4d_setComponents
            (
            pWorldPoint + i,
            bsiPolycoffs_evaluate (pXGlobal, param),
            bsiPolycoffs_evaluate (pYGlobal, param),
            bsiPolycoffs_evaluate (pZGlobal, param),
            bsiPolycoffs_evaluate (pWGlobal, param)
            );
        }

    if (pLocalPoint)
        {
        bsiDPoint4d_setComponents
            (
            pLocalPoint + i,
            bsiPolycoffs_evaluate (pXLocal, param),
            bsiPolycoffs_evaluate (pYLocal, param),
            bsiPolycoffs_evaluate (pZLocal, param),
            bsiPolycoffs_evaluate (pWLocal, param)
            );
        }

    return SUCCESS;
    }

/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            04/99
*
+----------------------------------------------------------------------*/
static     void jmdlRotatedConic_updateMaxDegree

(
int         *pDegree,
const PolyCoffs     *pCoffs
)
    {
    int degree;
    if (pCoffs)
        {
        degree = bsiPolycoffs_getDegree (pCoffs);
        if (degree > *pDegree)
            *pDegree = degree;
        }
    }

/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            04/99
*
*  Find the intersections of a rational polynomial curve with the
*  surface.
*
*  All intersections are returned (i.e. both line and surface are
*  unbounded)
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt       bsiRotatedConic_intersectRationalCurveUnbounded

(
DPoint4dP pWorldPoint,    /* <= 0 to 4n cartesian world coordinates, where n is the
polynomial degree. */
DPoint4dP pLocalPoint,   /* <= 0 to 4n homogeneous local coordinates of surface */
double      *pRootParam,    /* <= 0 to 4n polynomial parameters of the intersections */
int           *pNumIntersection,   /* <= number of intersections */
int         maxIntersection,    /* => max intersections permitted */
const RotatedConic  *pSurface,      /* => the rotated conic surface */
const PolyCoffs     *pXPoly,        /* => x polynomial */
const PolyCoffs     *pYPoly,        /* => y polynomial */
const PolyCoffs     *pZPoly,        /* => z polynomial */
const PolyCoffs     *pWPoly         /* => w polynomial */
)
    {
    StatusInt status = ERROR;
    bool      equationValid;
    PolyCoffs xGlobal, yGlobal, zGlobal, wGlobal;
    PolyCoffs xLocal, yLocal, zLocal, wLocal;
    PolyCoffs xx, yy, zz, ww, ff, roots;
    int i;
    int numIntersection = 0;
    DPoint4d globalCoffPoint, localCoffPoint;
    int maxDegree;

    if (!pSurface)
        return status;

    /* Find the maximum degree of the polynomials */
    maxDegree = 0;
    jmdlRotatedConic_updateMaxDegree (&maxDegree, pXPoly);
    jmdlRotatedConic_updateMaxDegree (&maxDegree, pYPoly);
    jmdlRotatedConic_updateMaxDegree (&maxDegree, pZPoly);
    jmdlRotatedConic_updateMaxDegree (&maxDegree, pWPoly);

    bsiPolycoffs_copyOrInit (&xGlobal, pXPoly, 0.0, maxDegree);
    bsiPolycoffs_copyOrInit (&yGlobal, pYPoly, 0.0, maxDegree);
    bsiPolycoffs_copyOrInit (&zGlobal, pZPoly, 0.0, maxDegree);
    bsiPolycoffs_copyOrInit (&wGlobal, pWPoly, 1.0, maxDegree);

    bsiPolycoffs_copyOrInit (&xLocal, NULL, 0.0, maxDegree);
    bsiPolycoffs_copyOrInit (&yLocal, NULL, 0.0, maxDegree);
    bsiPolycoffs_copyOrInit (&zLocal, NULL, 0.0, maxDegree);
    bsiPolycoffs_copyOrInit (&wLocal, NULL, 0.0, maxDegree);

    for (i = 0; i <= maxDegree; i++)
        {
        bsiPolycoffs_getCoff (&globalCoffPoint.x, &xGlobal,  i);
        bsiPolycoffs_getCoff (&globalCoffPoint.y, &yGlobal,  i);
        bsiPolycoffs_getCoff (&globalCoffPoint.z, &zGlobal,  i);
        bsiPolycoffs_getCoff (&globalCoffPoint.w, &wGlobal,  i);

        bsiDMatrix4d_multiply4dPoints (&pSurface->rotationMap.M1,
                        &localCoffPoint, &globalCoffPoint, 1);

        bsiPolycoffs_setCoff (&xLocal, localCoffPoint.x, i);
        bsiPolycoffs_setCoff (&yLocal, localCoffPoint.y, i);
        bsiPolycoffs_setCoff (&zLocal, localCoffPoint.z, i);
        bsiPolycoffs_setCoff (&wLocal, localCoffPoint.w, i);
        }


    /* BRANCH_ON_SURFACE_TYPE */
    equationValid = false;
    switch (pSurface ? pSurface->type : RC_NullSurface)
        {
        case RC_Cone:
            if (   SUCCESS == bsiPolycoffs_multiply (&xx, &xLocal, &xLocal)
                && SUCCESS == bsiPolycoffs_multiply (&yy, &yLocal, &yLocal)
                && SUCCESS == bsiPolycoffs_multiply (&ww, &zLocal, &zLocal)
                )
                {
                bsiPolycoffs_add (&ff, &xx, &yy);
                bsiPolycoffs_addScaled (&ff, &ff, &zz, -1.0);
                equationValid = true;
                }
            break;

        case RC_Sphere:
            if (   SUCCESS == bsiPolycoffs_multiply (&xx, &xLocal, &xLocal)
                && SUCCESS == bsiPolycoffs_multiply (&yy, &yLocal, &yLocal)
                && SUCCESS == bsiPolycoffs_multiply (&zz, &zLocal, &zLocal)
                && SUCCESS == bsiPolycoffs_multiply (&ww, &wLocal, &wLocal)
                )
                {
                bsiPolycoffs_add (&ff, &xx, &yy);
                bsiPolycoffs_add (&ff, &ff, &zz);
                bsiPolycoffs_addScaled (&ff, &ff, &ww, -1.0);
                equationValid = true;
                }
            break;

        case RC_Cylinder:
            if (   SUCCESS == bsiPolycoffs_multiply (&xx, &xLocal, &xLocal)
                && SUCCESS == bsiPolycoffs_multiply (&yy, &yLocal, &yLocal)
                && SUCCESS == bsiPolycoffs_multiply (&ww, &wLocal, &wLocal)
                )
                {
                bsiPolycoffs_add (&ff, &xx, &yy);
                bsiPolycoffs_addScaled (&ff, &ff, &ww, -1.0);
                equationValid = true;
                }
            break;

        case RC_Plane:
            ff = zLocal;
            equationValid = true;
            break;

        case RC_Disk:
            ff = zLocal;
            equationValid = true;
            break;

        case RC_Torus:
            {
            PolyCoffs rho2, r2, gg, rho2w2;
            double a = pSurface->hoopRadius;
            if (   SUCCESS == bsiPolycoffs_multiply (&xx, &xLocal, &xLocal)
                && SUCCESS == bsiPolycoffs_multiply (&yy, &yLocal, &yLocal)
                && SUCCESS == bsiPolycoffs_multiply (&zz, &zLocal, &zLocal)
                && SUCCESS == bsiPolycoffs_multiply (&ww, &wLocal, &wLocal)
                )
                bsiPolycoffs_add (&rho2, &xx, &yy);
                bsiPolycoffs_add (&r2, &rho2, &zz);
                bsiPolycoffs_addScaled (&gg, &r2, &ww, 1 - a* a);
                if (   SUCCESS == bsiPolycoffs_multiply (&ff, &gg, &gg)
                    && SUCCESS == bsiPolycoffs_multiply (&rho2w2, &rho2, &ww))
                    {
                    bsiPolycoffs_addScaled (&ff, &ff, &rho2w2, -4.0);
                    }
                equationValid = true;
            }
            break;
        }

    if (equationValid)
        {
        double root;
        bsiPolycoffs_realRoots (&roots, &ff);

        for (i = 0; SUCCESS == bsiPolycoffs_getCoff (&root, &roots, i); i++)
            {
            expandRationalCurveIntersection (
                                pWorldPoint, pLocalPoint, pRootParam,
                                &numIntersection, maxIntersection,
                                &xGlobal, &yGlobal, &zGlobal, &wGlobal,
                                &xLocal, &yLocal, &zLocal, &wLocal,
                                root
                                );
            }
        status = SUCCESS;
        }

    if (pNumIntersection)
        *pNumIntersection = numIntersection;
    return status;
    }

/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            04/99
*
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void     bsiRotatedConic_assembleConicCoefficients

(
PolyCoffs     *pX,
PolyCoffs     *pY,
PolyCoffs     *pZ,
PolyCoffs     *pW,
DConic4dCP pConic,        /* => the conic */
double      phaseAngle      /* => rotate axes by this angle to set origin of
polynomial */
)
    {
    DPoint4d AA[3], BB[3];
    double c = cos (phaseAngle);
    double s = sin (phaseAngle);
    int j;
#ifdef USE_INITIALIZERS
    RotMatrix M = {{
                {-1.0, 0.0, 1.0},   /* That's column 0!!! */
                { 0.0, 2.0, 0.0},
                { 1.0, 0.0, 1.0}
                }};
#else
    RotMatrix M;
    M.InitFromRowValues (
            -1, 0, 1,
             0, 2, 0,
             1, 0, 1);
#endif
    bsiDPoint4d_add2ScaledDPoint4d (&AA[0], NULL, &pConic->vector0, c, &pConic->vector90, -s);
    bsiDPoint4d_add2ScaledDPoint4d (&AA[1], NULL, &pConic->vector0, s, &pConic->vector90,  c);
    AA[2] = pConic->center;
    for (j = 0; j < 3; j++)
        {
        bsiDPoint4d_add3ScaledDPoint4d (&BB[j], NULL,
                        &AA[0], M.form3d[0][j],
                        &AA[1], M.form3d[1][j],
                        &AA[2], M.form3d[2][j]);
        }
    bsiPolycoffs_initQuadratic (pX, BB[2].x, BB[1].x, BB[0].x);
    bsiPolycoffs_initQuadratic (pY, BB[2].y, BB[1].y, BB[0].y);
    bsiPolycoffs_initQuadratic (pZ, BB[2].z, BB[1].z, BB[0].z);
    bsiPolycoffs_initQuadratic (pW, BB[2].w, BB[1].w, BB[0].w);
    }
/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            04/99
*
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt    bsiRotatedConic_intersectDConic4d

(
const RotatedConic  *pSurface,              /* => the rotated conic surface */
DPoint3dP pPointArray,      /* <= array of intersection points */
DPoint4dP pSurfaceParamArray,    /* array of intersection points in surface parameter
space */
double      *pThetaArray,           /* <= array of intersection angles in the conic parameterization */
int         *pNumIntersection,      /* <= number of intersections */
int         maxIntersection,        /* => size of output arrays */
DConic4dCP pConic,                  /* => the conic */
bool        boundEllipse,           /* => true to check bounds on ellipse */
bool        boundSurface            /* => true to check bounds on surface */
)
    {
    int i, j;
    /*  Can really only have 8 global points.  However, each points
        on edge of a partial range might show up twice, so allow more
        in total.
    */

#define SEGMENT_LIMIT 8
#define TOTAL_LIMIT   16

    DPoint4d quadrantGlobal[SEGMENT_LIMIT];
    DPoint4d quadrantLocal[SEGMENT_LIMIT];
    DPoint4d quadrantSurfaceParameter[SEGMENT_LIMIT];
    double   quadrantParameter[SEGMENT_LIMIT];
    double   solutionAngle[TOTAL_LIMIT];
    int      numInQuadrant;
    int      numTotal = 0;
    int      numOut = 0;
    double  phaseAngle, t0, t1;

    PolyCoffs xPoly, yPoly, zPoly, wPoly;

    for (i = 0; i < 2; i++)
        {
        phaseAngle = i * msGeomConst_pi;
        t0 = -1.0;
        t1 = 1.0;
        bsiRotatedConic_assembleConicCoefficients (&xPoly, &yPoly, &zPoly, &wPoly, pConic, phaseAngle);
        if (SUCCESS == bsiRotatedConic_intersectRationalCurveUnbounded
                                (
                                quadrantGlobal,
                                quadrantLocal,
                                quadrantParameter,
                                &numInQuadrant,
                                SEGMENT_LIMIT,
                                pSurface,
                                &xPoly, &yPoly, &zPoly, &wPoly)
                                )
            {
            bsiRotatedConic_transformDPoint4dArray (
                            quadrantSurfaceParameter,
                            quadrantLocal,
                            numInQuadrant,
                            pSurface,
                            RC_COORDSYS_local,
                            RC_COORDSYS_parameter
                            );

            for (j = 0; j < numInQuadrant; j++)
                {
                double t = quadrantParameter[j];
                double theta = phaseAngle + bsiTrig_atan2 (2.0 * t, 1.0 - t * t);
                if (   t >= t0
                    && t <= t1
                    && (!boundEllipse || bsiDConic4d_angleInSweep (pConic, theta))
                    && -1 == bsiDoubleArray_angleIndexInArray (solutionAngle, numTotal, theta)
                    && (!boundSurface || bsiRotatedConic_isPointOnSurfacePatch (&quadrantLocal[j],
                                    RC_COORDSYS_local, pSurface)))
                    {
                    /* Copy to user arrays */
                    if (numOut < maxIntersection)
                        {
                        if (pThetaArray)
                            pThetaArray[numOut] = theta;
                        if (pPointArray)
                            quadrantGlobal[j].GetProjectedXYZ (pPointArray[ numOut]);
                        if (pSurfaceParamArray)
                            pSurfaceParamArray[numOut] = quadrantSurfaceParameter[j];
                        numOut++;
                        }

                    /* Copy angle to local array for duplicate tests */
                    if (numTotal < TOTAL_LIMIT)
                        {
                        solutionAngle[numTotal] = theta;
                        numTotal++;
                        }
                    }
                }
            }
        }

    if (pNumIntersection)
        *pNumIntersection = numOut;
    return SUCCESS;
    }

/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            04/99
*
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt    bsiRotatedConic_intersectDEllipse3d

(
const RotatedConic  *pSurface,          /* => the rotated conic surface */
DPoint3dP pPointArray,       /* <= array of intersection points */
DPoint4dP pSurfaceParamArray, /* array of intersection points in surface parameter
space */
double      *pThetaArray,       /* <= array of intersection angles in the conic parameterization */
int         *pNumIntersection,  /* <= number of intersections */
int         maxIntersection,    /* => size of output arrays */
DEllipse3dCP pEllipse,           /* => the ellipse */
bool        boundEllipse,       /* => true to check bounds on ellipse */
bool        boundSurface        /* => true to check bounds on surface */
)
    {
    DConic4d conic;
    bsiDConic4d_initFromDEllipse3d (&conic, pEllipse);

    return bsiRotatedConic_intersectDConic4d (pSurface,
                        pPointArray, pSurfaceParamArray, pThetaArray, pNumIntersection,
                        maxIntersection, &conic, boundEllipse, boundSurface);
    }

/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            07/97
*
*  Compute worse-than-conic parts of intersection with plane.
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt    bsiRotatedConic_generalIntersectPlane

(
const RotatedConic  *pSurface,      /* => the rotated conic surface */
DPoint4dCP pPlane,        /* => eyepoint (nonzero weight) or view vector (zero weight) */
SilhouetteArrayHandler handlerFunc, /* => callback for points */
double              tolerance,      /* => distance from surface */
void                *pUserData      /* => arbitrary pointer */
)
    {
    StatusInt status = ERROR;

    switch (pSurface ? pSurface->type : RC_NullSurface)
        {
        case RC_Torus:
            {
            return bsiRotatedConic_torusGeneralIntersectPlane
                        (
                        pSurface,
                        pPlane,
                        handlerFunc,
                        tolerance,
                        pUserData
                        );
            }
        }
    return status;
    }
/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            07/97
*
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt   bsiHConic_evaluateDPoint4d /* ERROR if parameterization of this quadric type is not supported */

(
DPoint4dP pPoint,           /* <= point on the conic */
const HConic        *pConic,        /* => unclipped (but possibly multi-sector) conic */
      double        param           /* => parameter for evaluation */
)
    {
    StatusInt status = ERROR;
    switch (pConic->type)
        {
        case HConic_Line:
            if (pPoint)
                bsiDPoint4d_add2ScaledDPoint4d (pPoint, NULL,
                                        &pConic->coordinates.vector0, 1.0 - param,
                                        &pConic->coordinates.vector90, param
                                        );
            status = SUCCESS;
            break;
        case HConic_Ellipse:
            if (pPoint)
                bsiDEllipse4d_evaluateDPoint4d (&pConic->coordinates, pPoint, param);
            status = SUCCESS;
            break;
        }
#ifdef SHOW_HITS
    if (SUCCESS == status)
        {
        DPoint3d cPoint; pPoint->GetProjectedXYZ (cPoint) ; /* THISWAS a bool thrown away as a statement */
        debug_displayPoint (&cPoint, 0);
        }
#endif
    return status;
    }


/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            07/97
*
*  Find intersections of a conic with a half plane extending away
*  from the Z axis.
+----------------------------------------------------------------------*/
static      StatusInt       jmdlHConic_addZHalfPlaneBreaks /* ERROR if intersection with conic is not supported */

(
double      *pParam,        /* <= array of parameter values */
int         *pNumParam,     /* <= number of parameters */
int         maxParam,       /* => max params allowed */
const HConic        *pConic,        /* => unclipped (but possibly multi-sector) conic */
      double        theta,          /* => angle from xz plane to plane */
      double        r0,             /* => radial limit */
      double        r1              /* => radial limit */
)
    {
    StatusInt status = ERROR;
    DPoint4d planeCoffs;
    DPoint4d piercePoint;
    double c0, c1, rPierce, f0, f1, deltac;
    double mu0, mu1;
    double cosine = cos (theta);
    double sine   = sin (theta);
    double w;

    double relTol = bsiTrig_smallAngle ();

    bsiDPoint4d_setComponents (&planeCoffs, -sine, cosine, 0.0, 0.0);
    switch (pConic->type)
        {
        case HConic_Line:

            c0 = bsiDPoint4d_dotProduct (&pConic->coordinates.vector0,  &planeCoffs);
            c1 = bsiDPoint4d_dotProduct (&pConic->coordinates.vector90, &planeCoffs);
            /* No-fuss unnormalized parameter values */
            mu0 = -c1;
            mu1 =  c0;

            bsiDPoint4d_add2ScaledDPoint4d (&piercePoint, NULL,
                                        &pConic->coordinates.vector0,  mu0,
                                        &pConic->coordinates.vector90, mu1);
            rPierce = piercePoint.x * cosine + piercePoint.y * sine;

            w = piercePoint.w;

            f0 = rPierce - w * r0;
            f1 = rPierce - w * r1;

            deltac = c1 - c0;
            if (f0 * f1 <= 0.0 && *pNumParam < maxParam && fabs (deltac) > relTol * fabs (c0) )
                {
                pParam[*pNumParam] = -c0 / ( c1 - c0);
                *pNumParam += 1;
                }

            status = SUCCESS;
            break;

        case HConic_Ellipse:
            {
            DPoint3d param[2];
            int numInt;
            int i;
            numInt = bsiDEllipse4d_intersectPlane (param,
                        &pConic->coordinates.center,
                        &pConic->coordinates.vector0,
                        &pConic->coordinates.vector90,
                        &planeCoffs
                        );

            for (i = 0; i < numInt; i++)
                {
                if (*pNumParam < maxParam)
                    {
                    pParam[*pNumParam] = param[i].z;
                    *pNumParam += 1;
                    }
                }
            }
        }
    return SUCCESS;
    }

/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            07/97
*
*  Find intersections of a conic with a cylinder around the  +Z axis.
+----------------------------------------------------------------------*/
static StatusInt   jmdlHConic_addZCylinderBreaks /* ERROR if intersection with conic is not supported */

(
double      *pParam,        /* <= array of parameter values */
int         *pNumParam,     /* <= number of parameters */
int         maxParam,       /* => max params allowed */
const HConic        *pConic,        /* => unclipped (but possibly multi-sector) conic */
      double        radius          /* => cylinder radius */
)
    {
    StatusInt status = ERROR;
    double c00, c01, c11;
    double lambda0[2], lambda1[2];
    int numSolution;
    int i;
    switch (pConic->type)
        {
        case HConic_Line:
            c00 =       bsiQuadric_circleProduct (&pConic->coordinates.vector0,  &pConic->coordinates.vector0,  radius);
            c01 = 2.0 * bsiQuadric_circleProduct (&pConic->coordinates.vector0,  &pConic->coordinates.vector90, radius);
            c11 =       bsiQuadric_circleProduct (&pConic->coordinates.vector90, &pConic->coordinates.vector90, radius);
            numSolution = bsiMath_solveConvexQuadratic (lambda0, lambda1, c00, c01, c11);
            for (i = 0; i < numSolution; i++)
                {
                if (*pNumParam < maxParam)
                    {
                    pParam[*pNumParam] = lambda1[i];
                    *pNumParam += 1;
                    }
                }
            status = SUCCESS;
            break;

        case HConic_Ellipse:
            {
            DPoint4d A, U, V;
            double cosInt[4];
            double sinInt[4];
            double thetaInt[4];
            int numInt;
            A = pConic->coordinates.center;
            U = pConic->coordinates.vector0;
            V = pConic->coordinates.vector90;
            bsiMath_conicIntersectUnitCircle (cosInt, sinInt, thetaInt, &numInt,
                                    A.x, U.x, V.x,
                                    A.y, U.y, V.y,
                                    radius * A.w, radius * U.w, radius * V.w);
            for (i = 0; i < numInt; i++)
                {
                if (*pNumParam < maxParam)
                    {
                    pParam[*pNumParam] = thetaInt[i];
                    *pNumParam += 1;
                    }
                }
            }
            break;
        }
    return status;
    }
/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            07/97
*
*  Find parameters where an HConic intersects the surface's
*  transverse half spaces.   If the HConic is in the surface, these are
*  candidate points for clipping the HConic to the surface patch.
*
*  If the HConic is significantly out of the surface, result is
*  unpredictable.
*  Transverse spaces are constructed so that slightly out-of-surface
*  clips make sense.
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt       bsiRotatedConic_intersectUnboundedHConicWithTransverseSpaces /* ERROR if intersection with conic is not supported */

(
double      *pParam,        /* <= array of parameter values */
int         *pNumParam,     /* <= number of parameters */
int         maxParam,       /* => max params allowed */
const HConic        *pConic,        /*  => unclipped (but possibly multi-sector) conic */
const RotatedConic  *pSurface       /* => the rotated conic surface */
)
    {
    StatusInt status = ERROR;
    HConic localConic = *pConic;
    double x0 = pSurface->parameterRange.low.x;
    double x1 = pSurface->parameterRange.high.x;
    double y0 = pSurface->parameterRange.low.y;
    double y1 = pSurface->parameterRange.high.y;
    double tolerance = 1.0e-12;  /* Maybe should be local tol from surface.  However, global tol was
                                        probably already used to force r=0 as needed */

    *pNumParam = 0;
    /* BRANCH_ON_SURFACE_TYPE */
    switch (pSurface->type)
        {
        case RC_Disk:
            bsiHConic_multiplyHMatrix (
                            &localConic, pConic, &pSurface->rotationMap.M1);

            if (y0 > tolerance)
                jmdlHConic_addZCylinderBreaks (pParam, pNumParam, maxParam, &localConic, y0);
            if (y1 > tolerance)
                jmdlHConic_addZCylinderBreaks (pParam, pNumParam, maxParam, &localConic, y1);
            if (!bsiTrig_isAngleFullCircle (x1 - x0))
                {
                jmdlHConic_addZHalfPlaneBreaks (pParam, pNumParam, maxParam, &localConic, x0, y0, y1);
                jmdlHConic_addZHalfPlaneBreaks (pParam, pNumParam, maxParam, &localConic, x1, y0, y1);
                }
            status = SUCCESS;
            break;
        }
    return status;
    }
/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            07/97
*
*  Clip an HConic to a set of half spaces which define the boundary of
*  this surface.   If the HConic is in the surface, this clips the
*  the HConic to the surface patch.   If the HConic is significantly
*  out of the surface, result is unpredictable.
*  Transverse spaces are constructed so that slightly out-of-surface
*  clips make sense.
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt       bsiRotatedConic_clipHConicToTransverseSpaces /* ERROR if intersection with conic is not supported */

(
HConic      *pClipped,      /* <= clipped conic. */
const HConic        *pUnclipped,    /*  => unclipped (but possibly multi-sector) conic */
const RotatedConic  *pSurface       /* => the rotated conic surface */
)
    {
    double a0, a1, aMid, step;
    StatusInt status = ERROR;
    int i;
#define MAX_BREAK 40
    double breakParam[MAX_BREAK];
    int numBreak;
    DPoint4d worldPoint;
    double minParamStep = 1.0e-10;
    HConic unclipped;

    unclipped = *pClipped = *pUnclipped;

    bsiDEllipse4d_clearSectors (&pClipped->coordinates);


    if (SUCCESS == bsiRotatedConic_intersectUnboundedHConicWithTransverseSpaces (
                            breakParam, &numBreak, MAX_BREAK, &unclipped, pSurface))
        {
        for (i = 0; SUCCESS == bsiDEllipse4d_getSegment (&a0, &a1, &pUnclipped->coordinates, i); i++)
            {
            if (numBreak < MAX_BREAK)
                breakParam[numBreak++] = a0;
            if (numBreak < MAX_BREAK)
                breakParam[numBreak++] = a1;
            }

        if (pClipped->type == HConic_Ellipse)
            {
            /* Normalize all breaks and insert dummies at the period break */
            for (i = 0; i < numBreak; i++)
                breakParam[i] = bsiTrig_getNormalizedAngle (breakParam[i]);
            if (numBreak < MAX_BREAK)
                breakParam[numBreak++] = -msGeomConst_pi;
            if (numBreak < MAX_BREAK)
                breakParam[numBreak++] = msGeomConst_pi;
            }


        bsiDoubleArray_sort (breakParam, numBreak, true);
        /* Does this logic break because of period shifts */
        for (i = 1; i < numBreak; i++)
            {
            a0 = breakParam[i-1];
            a1 = breakParam[i];
            step     = a1 - a0;
            aMid = 0.5 * (a0 + a1);
            if (step >= minParamStep)
                {
                if (bsiRange1d_pointIsIn (&unclipped.coordinates.sectors, aMid))
                    {
                    if (   SUCCESS == bsiHConic_evaluateDPoint4d (&worldPoint, &unclipped, aMid)
                        && bsiRotatedConic_isPointOnSurfacePatch (&worldPoint, RC_COORDSYS_world, pSurface))
                        {
                        bsiRange1d_addUnordered (&pClipped->coordinates.sectors, a0, a1);
                        }
                    }
                }
            }
        status = SUCCESS;
        }
    return status;
    }

/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            07/97
*
*  Find the intersection of a plane with the surface.
*
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt       bsiRotatedConic_intersectPlane /* ERROR if intersection is not conic */

(
HConic      *pHConic,       /* <= array of 0, 1, or 2 conics */
int         *pNumConic,     /* <= number of conics */
const RotatedConic  *pSurface,      /* => the rotated conic surface */
DPoint4dCP pPlane,          /* => homogeneous vector of plane */
bool          bounded       /* => true to bound by parameter range */
)
    {
    StatusInt status = ERROR;
    int surfaceType, i;
    DPoint4d localPlane;
    *pNumConic = 0;

    /* BRANCH_ON_SURFACE_TYPE */
    switch (pSurface ? (surfaceType = pSurface->type) : RC_NullSurface)
        {
        case RC_Plane:
            bsiDMatrix4d_multiplyTransposePoints (&pSurface->rotationMap.M0, &localPlane, pPlane, 1);
            bsiHConic_initUnitSquarePlaneIntersection (pHConic, pNumConic, &localPlane,
                                        bounded ? &pSurface->parameterRange : NULL);
            status = SUCCESS;
            break;

        case RC_Cone:
            {
            bsiDMatrix4d_multiplyTransposePoints (&pSurface->rotationMap.M0, &localPlane, pPlane, 1);
            omdHConic_initUnitConePlaneIntersection (pHConic, pNumConic, &localPlane,
                                        bounded ? &pSurface->parameterRange : NULL);
            status = SUCCESS;
            }
            break;

        case RC_Cylinder:
            {
            bsiDMatrix4d_multiplyTransposePoints (&pSurface->rotationMap.M0, &localPlane, pPlane, 1);
            omdHConic_initUnitCylinderPlaneIntersection (pHConic, pNumConic, &localPlane,
                                        bounded ? &pSurface->parameterRange : NULL);
            status = SUCCESS;
            break;
            }

        case RC_Disk:
            {
            bsiDMatrix4d_multiplyTransposePoints (&pSurface->rotationMap.M0, &localPlane, pPlane, 1);
            omdHConic_initUnitDiskPlaneIntersection (pHConic, pNumConic, &localPlane,
                                        bounded ? &pSurface->parameterRange : NULL);
            status = SUCCESS;
            break;
            }

        case RC_Sphere:
            {
            bsiDMatrix4d_multiplyTransposePoints (&pSurface->rotationMap.M0, &localPlane, pPlane, 1);
            omdHConic_initUnitSpherePlaneIntersection (pHConic, pNumConic, &localPlane,
                                        bounded ? &pSurface->parameterRange : NULL);
            status = SUCCESS;
            break;
            }
        case RC_Torus:
            {
            bsiDMatrix4d_multiplyTransposePoints (&pSurface->rotationMap.M0, &localPlane, pPlane, 1);
            status = omdHConic_initTorusPlaneIntersection (pHConic, pNumConic, pSurface->hoopRadius, &localPlane,
                                        bounded ? &pSurface->parameterRange : NULL);
            break;
            }
        }

    if (SUCCESS == status)
        {
        for (i = 0; i < *pNumConic; i++)
            {
            bsiHConic_multiplyHMatrix (pHConic + i,     pHConic + i,    &pSurface->rotationMap.M0);
            }
        }

    return status;
    }

/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            07/97
*
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void       bsiRotatedConic_getHMap

(
DMap4dP pHMap,      /* <= the surface's local coordinates */
const RotatedConic  *pSurface       /* => the rotated conic surface */
)
    {
    *pHMap = pSurface->rotationMap;
    }

/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            07/97
*
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void       bsiRotatedConic_setHMap

(
RotatedConic  *pSurface,            /* <= the rotated conic surface */
DMap4dCP pHMap      /* => map to use for the surface's local coordinates */
)
    {
    pSurface->rotationMap = *pHMap;
    }

/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            07/97
*
*  A rotated conic is defined as some reference surface transformed by
*  an DMap4d B.   Replace the DMap4d by PreMap * B * PostMap.
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void       bsiRotatedConic_transform

(
RotatedConic  *pSurface,            /* <= the rotated conic surface */
DMap4dCP pPreMap,           /* => map to premultiply local frame */
DMap4dCP pPostMap       /* <= map to postmultiply local frame */
)
    {

    if (pPreMap)
        bsiDMap4d_multiply (
                        &pSurface->rotationMap,
                        (const HMapP)pPreMap,
                        &pSurface->rotationMap
                        );

    if (pPostMap)
        bsiDMap4d_multiply (
                        &pSurface->rotationMap,
                        &pSurface->rotationMap,
                        (const HMapP)pPostMap
                        );
    }



#ifdef goryIntersector
/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            07/97
*
*  Find the intersection of line
*   X=L0*alpha0 + L1*alpha1
*  with the rotated conic.
+----------------------------------------------------------------------*/
xPublic      StatusInt       bsiRotatedConic_intersectLine4d

(
DPoint4dP pIntersectionPoint,
DPoint3dP pThetaPoint,      /* <= array of 0,1,or 2 theta-format points */
DPoint3dP pPhiPoint,                /* <= array of 0,1,or 2 phi-format points */
DPoint3dP pLineParameters,       /* <= array of 0,1,or 2 line parameters */
int         numIntersection,        /* <= number of intersections */
const RotatedConic    *pSurface,            /* <= the rotated conic surface */
DPoint4dCP pLinePoints      /* <= two points on the line */
)
    {
    StatusInt status = ERROR;
    DPoint4d localLinePoints[2];

    bsiDMatrix4d_multiply4dPoints (&pSurface->rotationMap.M1, localLinePoints, pLinePoints, 2);

    if (pSurface->type == RotatedLine)
        {
        DPoint3d trigPoint[2];
        RotMatrix matLxy, matLzw, invLzw;
        RotMatrix matBxy, matBzw, invBzw;
        RotMatrix matM;
        int numSolution;
        double condLzw, condBzw;

        jmdlRotMatrix_initXYFromDPoint4d (&matLxy, &matLzw, localLinePoints, localLinePoints + 1);
        jmdlRotMatrix_initXYFromDPoint4d (&matBxy, &matBzw, &pSurface->conic.vector0, &pSurface->conic.vector90);

        jmdlSVD_invertXY (&invLzw, &condLzw, &matLzw);
        jmdlSVD_invertXY (&invBzw, &condBzw, &matBzw);

        if (condLzw <= 0.0 && condBzw <= 0.0)
            {
            }
        else if (condLzw >= condBzw)
            {
            bsiRotMatrix_multiplyRotMatrixRotMatrixXY (&matM, &matLxy, &invLzw);
            bsiRotMatrix_multiplyRotMatrixRotMatrixXY (&matM, &matM, &matBzw);
            numSolution = jmdlRotatedConic_computeSingularRotations (trigPoint, &matBxy, &matM);
            }
        else
            {
            bsiRotMatrix_multiplyRotMatrixRotMatrixXY (&matM, &matBxy, &invBzw);
            bsiRotMatrix_multiplyRotMatrixRotMatrixXY (&matM, &matM, &matLzw);
            numSolution = jmdlRotatedConic_computeSingularRotations (trigPoint, &matM, &matLxy);
            }
        }
    return status;
    }
#endif

/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            07/97
*
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt bsiRotatedConic_initAxis

(
RotatedConic        *pSurface,
DPoint3dCP pCenter,
DVec3dCP pAxis,
DPoint3dCP pTargetPoints,
int                 numTargetPoint
)
    {
    DPoint3d projectedPoint, normalVector;
    DPoint3d newCenter;
    double distanceSquared;
    DVec3d xAxis, yAxis, zAxis;
    double maxDistanceSquared;
    Transform transform;
    int    iMax= -1;
    double param;

    int i;
    maxDistanceSquared = -1.0;
    if (pAxis->DotProduct (*pAxis) <= 0.0)
        return ERROR;

    /* Find the point that is farthest from the axis. */
    for (i = 0; i < numTargetPoint; i++)
        {
        bsiGeom_projectPointToRay (&projectedPoint, &param, &pTargetPoints[i], pCenter, pAxis);
        normalVector.DifferenceOf (pTargetPoints[i], projectedPoint);
        distanceSquared = normalVector.DotProduct (normalVector);
        if (distanceSquared > maxDistanceSquared)
            {
            iMax = i;
            maxDistanceSquared = distanceSquared;
            newCenter = projectedPoint;
            }
        }

    if ( iMax < 0)
        return ERROR;

    /* Point the x axis towards the farthest point. */
    zAxis = *pAxis;
    zAxis.Normalize ();
    xAxis.DifferenceOf (pTargetPoints[iMax], newCenter);
    xAxis.Normalize ();
    yAxis.CrossProduct (zAxis, xAxis);
    transform.SetTranslation  (newCenter);
    transform.SetMatrixColumn (xAxis, 0);
    transform.SetMatrixColumn (yAxis, 1);
    transform.SetMatrixColumn (zAxis, 2);
    bsiDMap4d_initFromTransform(&pSurface->rotationMap, &transform, false) ; /* THISWAS a bool thrown away as a statement */
    return SUCCESS;
    }

/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            07/97
*
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt bsiRotatedConic_initRotatedLine

(
RotatedConic        *pSurface,
DPoint3dCP pStart,
DPoint3dCP pEnd,
DPoint3dCP pCenter,
DVec3dCP pAxis,
double              sweep
)
    {
    DPoint3d point[2];
    DPoint4d center, vector0, vector90;
    StatusInt status = ERROR;

    point[0] = *pStart;
    point[1] = *pEnd;
    bsiRotatedConic_clear (pSurface);
    pSurface->type = RC_RotatedLine;

    if (SUCCESS == bsiRotatedConic_initAxis (pSurface, pCenter, pAxis, point, 2))
        {
        bsiDPoint4d_setComponents (&center, 0.0, 0.0, 0.0, 0.0);
        bsiDPoint4d_copyAndWeight (&vector0, pStart, 1.0);
        bsiDPoint4d_copyAndWeight (&vector90, pEnd,   1.0);
        bsiDEllipse4d_initFromDPoint4d (&pSurface->conic, &center, &vector0, &vector90, 0.0, 1.0);
        bsiDEllipse4d_multiplyByHMatrix (&pSurface->conic, &pSurface->conic, &pSurface->rotationMap.M1);
        bsiRotatedConic_setSweep (pSurface, sweep);
        status = SUCCESS;
        }

    return status;
    }

/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            07/97
*
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt bsiRotatedConic_initRotatedEllipse

(
RotatedConic        *pSurface,
DEllipse4dP pHEllipse,
DPoint3dCP pCenter,
DVec3dCP pAxis,
double              sweep
)
    {
    DPoint3d point[5];
    double phi0, phi1, dPhi;
    int numTarget = 0;
    StatusInt status = ERROR;

    bsiRotatedConic_clear (pSurface);
    pSurface->type = RC_RotatedEllipse;
    bsiDEllipse4d_getSegment (&phi0, &phi1, pHEllipse, 0);
    dPhi = phi1 - phi0;
    bsiDEllipse4d_evaluateDPoint3d(pHEllipse, &point[numTarget++], phi0) ;
    bsiDEllipse4d_evaluateDPoint3d(pHEllipse, &point[numTarget++], phi0 + 0.25 * dPhi);
    bsiDEllipse4d_evaluateDPoint3d(pHEllipse, &point[numTarget++], phi0 + 0.50 * dPhi);
    bsiDEllipse4d_evaluateDPoint3d(pHEllipse, &point[numTarget++], phi0 + 0.75 * dPhi);
    bsiDEllipse4d_evaluateDPoint3d(pHEllipse, &point[numTarget++], phi1);

    if (SUCCESS == bsiRotatedConic_initAxis (pSurface, pCenter, pAxis, point, numTarget))
        {
        pSurface->conic = *pHEllipse;
        bsiDEllipse4d_multiplyByHMatrix (&pSurface->conic, &pSurface->conic, &pSurface->rotationMap.M1);
        bsiRotatedConic_setSweep (pSurface, sweep);
        status = SUCCESS;
        }

    return status;
    }

/*----------------------------------------------------------------*//**
@description Initialize a rotated DEllispe3d.
@param pSurface OUT initialized struture.
@param pEllipse IN base ellipse.
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt bsiRotatedConic_initRotatedDEllipse3d
(
RotatedConic        *pSurface,
DEllipse3dCP        pEllipse,
DPoint3dCP          pCenter,
DVec3dCP            pAxis,
double              sweep
)
    {
    DEllipse4d hEllipse;
    bsiDEllipse4d_initFromDEllipse3d (&hEllipse, pEllipse);
    return bsiRotatedConic_initRotatedEllipse (pSurface, &hEllipse, pCenter, pAxis, sweep);
    }

/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            07/97
*
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt bsiRotatedConic_initUnitCylinder

(
RotatedConic        *pSurface,
DMap4dCP pFrame,
double      theta0,
double      sweep
)
    {

    bsiRotatedConic_clear (pSurface);
    pSurface->type = RC_Cylinder;
    if (pFrame)
        {
        pSurface->rotationMap = *pFrame;
        }
    else
        {
        bsiDMap4d_initIdentity (&pSurface->rotationMap);
        }

    bsiRotatedConic_setParameterRange (pSurface, theta0, sweep, 0.0, 1.0);

    return SUCCESS;
    }

/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            07/97
*
*  Initialize with specified surface type and parameter range.
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt bsiRotatedConic_initFrameAndSweep

(
RotatedConic        *pSurface,      /* <= intitialized surface */
RotatedConicType    type,           /* => Surface type. */
DMap4dCP pFrame,    /* => Placement frame.  If NULL, identity frame is used */
double      theta0,
double      dTheta,
double      alpha0,
double      dAlpha
)
    {

    bsiRotatedConic_clear (pSurface);
    pSurface->type = type;;
    if (pFrame)
        {
        pSurface->rotationMap = *pFrame;
        }
    else
        {
        bsiDMap4d_initIdentity (&pSurface->rotationMap);
        }

    bsiRotatedConic_setParameterRange (pSurface, theta0, dTheta, alpha0, dAlpha);

    return SUCCESS;
    }

/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            07/97
*
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt bsiRotatedConic_initUnitSquare

(
RotatedConic        *pSurface,
DMap4dCP pFrame
)
    {

    bsiRotatedConic_clear (pSurface);
    pSurface->type = RC_Plane;
    if (pFrame)
        {
        pSurface->rotationMap = *pFrame;
        }
    else
        {
        bsiDMap4d_initIdentity (&pSurface->rotationMap);
        }

    bsiRotatedConic_setParameterRange (pSurface, 0.0, 1.0, 0.0, 1.0);

    return SUCCESS;
    }

/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            07/97
*
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt bsiRotatedConic_initParallelogram

(
RotatedConic        *pSurface,
DPoint3dCP pOrigin,
DPoint3dCP pXPoint,
DPoint3dCP pYPoint
)
    {
    DVec3d xVector, yVector, zVector;
    Transform transform;
    DMap4d map;
    double magX, magY, magZ;
    StatusInt status = ERROR;

    bsiRotatedConic_clear (pSurface);

    xVector.DifferenceOf (*pXPoint, *pOrigin);
    yVector.DifferenceOf (*pYPoint, *pOrigin);
    zVector.CrossProduct (xVector, yVector);

    magX = xVector.Magnitude ();
    magY = yVector.Magnitude ();
    magZ = zVector.Magnitude ();

    if (magZ > bsiTrig_smallAngle () * magX * magY)
        {
        zVector.Scale (zVector, sqrt (magX * magY) / magZ);

        transform.SetTranslation (*pOrigin);
        transform.SetMatrixColumn (xVector, 0);
        transform.SetMatrixColumn (yVector, 1);
        transform.SetMatrixColumn (zVector, 2);
        bsiDMap4d_initFromTransform(&map, &transform, false) ; /* THISWAS a bool thrown away as a statement */
        bsiRotatedConic_initUnitSquare (pSurface, &map);
        status = SUCCESS;
        }

    return status;
    }

/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            07/97
*
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int bsiRotatedConic_surfaceDegree    /* 1 for planar, 2 quadric, 4 toroid */

(
const RotatedConic    *pSurface
)
    {
    int degree = 4;
    switch (pSurface->type)
        {
        case RC_Sphere:
        case RC_Cylinder:
        case RC_Cone:
            degree = 2;
            break;
        case RC_Disk:
            degree = 1;
            break;
        case RC_Plane:
            degree = 1;
            break;
        default:
            degree = 4;
        }
    return degree;
    }

/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            07/97
*
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int bsiRotatedConic_functionMask

(
const RotatedConic    *pSurface
)
    {
    int mask = 0;
    /* BRANCH_ON_SURFACE_TYPE */

    switch (pSurface->type)
        {
        case RC_Cylinder:
        case RC_Cone:
        case RC_Disk:
        case RC_Plane:
        case RC_Sphere:
            mask |= RC_EvaluateBoundaryMask
                 |  RC_IntersectLineMask
                 |  RC_IntersectPlaneMask
                 |  RC_SilhouetteMask;
            break;
        case RC_Torus:
            mask |= RC_EvaluateBoundaryMask
                 |  RC_IntersectLineMask
                 |  RC_IntersectPlaneMask
                 |  RC_SilhouetteMask;
            break;
        }
    return mask;
    }

/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            07/97
*
|
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt  bsiRotatedConic_getLocalRuleLimits

(
HConic          *pBaseConic,
HConic          *pEndConic,
const   RotatedConic    *pSurface
)
    {
    StatusInt status = ERROR;

    switch (pSurface->type)
        {
        case RC_Cylinder:
        case RC_Cone:
        case RC_RotatedLine:
            {
            status = bsiRotatedConic_localBoundaryComponents (pBaseConic, pEndConic, NULL, NULL, pSurface, true);
            }
        }

    if (SUCCESS != status)
        {
        bsiHConic_initNull(pBaseConic);
        bsiHConic_initNull(pEndConic);
        }

    return status;
    }
/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            07/97
*
*  Return the conic which is swept about the z axis in the natural
*  coordinates.
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt  bsiRotatedConic_getLocalRotationGenerator

(
HConic          *pHConic,
double          *pSweep,
const   RotatedConic    *pSurface
)
    {
    StatusInt status = ERROR;
    /* BRANCH_ON_SURFACE_TYPE */
    switch (pSurface->type)
        {
        case RC_Torus:
        case RC_Sphere:
        case RC_Cylinder:
        case RC_Cone:
        case RC_Disk:
            status = bsiRotatedConic_localBoundaryComponents (NULL, NULL, pHConic, NULL, pSurface, false);
            if (SUCCESS == status)
                {
                *pSweep = pSurface->parameterRange.high.x - pSurface->parameterRange.low.x;
                }
            break;

        case RC_RotatedLine:
        case RC_RotatedEllipse:
            status = bsiRotatedConic_localBoundaryComponents (NULL, NULL, pHConic, NULL, pSurface, false);
            if (SUCCESS == status)
                {
                *pSweep = pSurface->sweep;
                }
            break;

        default:
            break;
        }

    if (SUCCESS != status)
        {
        *pSweep = 0.0;
        bsiHConic_initNull(pHConic);
        }

    return status;
    }

/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            07/97
*
*  Return true if the surface patch can possibly hide points on the
*  surface's own silhouette.  For instance, a torus silhouette can
*  disappear when it goes into the donut hole, but the silhouette ruling
*  of a cone cannot disappear behind any part of the cone.
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool       bsiRotatedConic_canHideSilhouettePoints

(
const RotatedConic    *pSurface
)
    {
    bool    canHide = true;
    switch (pSurface->type)
        {
        case RC_Cylinder:
        case RC_Cone:
        case RC_Plane:
        case RC_Sphere:
            canHide = false;
            break;
        }
    return canHide;
    }
/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            07/97
*
*  Revise basis vectors so vector0 is in the xy plane and vector 90
*  is parallel to the z axis.
| ERROR can occur due to (1) out-of-plane conditions or (2) skewing     |
+----------------------------------------------------------------------*/
static   StatusInt    jmdlRotatedConic_squareLocalBasis

(
DPoint3dP pVector0,
DPoint3dP pVector90,
double      *pThetaStart,
double      *pThetaEnd,
double      tolerance
)
    {
    DPoint3d normal;
    DPoint3d zVector, xyVector;
    double r0, r90, rMean, thetaRef, c0, s0, rho0, rhoZ;
    double dz0dTheta;
    StatusInt status = ERROR;

    r0 = pVector0->Magnitude ();
    r90 = pVector90->Magnitude ();
    rMean = sqrt (r0 * r90);
    bsiDPoint3d_sizedCrossProduct (&normal, pVector0, pVector90, rMean);
    rho0 = sqrt (pVector0->z * pVector0->z + pVector90->z * pVector90->z);

    if (fabs (normal.z) < tolerance && rho0 > tolerance)
        {
        /* We are in the right plane */
        /* There are two places where the ellipse crosses the z plane. Choose the one where it
            crosses moving upward as the 0 angle. */
        c0 = -pVector90->z / rho0;
        s0 = pVector0->z / rho0;
        dz0dTheta = -s0 * pVector0->z + c0 * pVector90->z;

        if (dz0dTheta < 0.0)
            {
            c0 = -c0;
            s0 = -s0;
            }
        thetaRef = bsiTrig_atan2 (s0, c0);

        /* Now the ellipse crosses upwards at theta00 */
        bsiDPoint3d_add2ScaledDPoint3d (&xyVector, NULL, pVector0, c0, pVector90, s0);
        bsiDPoint3d_add2ScaledDPoint3d (&zVector, NULL, pVector0, -s0, pVector90, c0);

        rhoZ = sqrt (zVector.x * zVector.x + zVector.y * zVector.y);

        if (rhoZ <= tolerance)
            {
            *pVector0 = xyVector;
            *pVector90 = zVector;
            *pThetaStart -= thetaRef;
            *pThetaEnd   -= thetaRef;
            status = SUCCESS;
            }
        }
    return status;
    }
/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            07/97
*
*  Lock an angle to a nearby multiple of pi/2.
+----------------------------------------------------------------------*/
static   double    jmdlRotatedConic_lockAngle

(
double phi,
double tol
)
    {
    double phiNew = phi;
    if (phi < 0)
        {
        phiNew = -jmdlRotatedConic_lockAngle (-phi, tol);
        }
    else if (fabs (phi - msGeomConst_piOver2) <= tol)
        {
        phiNew = msGeomConst_piOver2;
        }
    else if (fabs (phi - msGeomConst_pi) <= tol)
        {
        phiNew = msGeomConst_pi;
        }
    else if (phi > msGeomConst_pi)
        {
        double f = phi / msGeomConst_piOver2;
        int    fi = (int)f;
        double residualAngle = (f - fi) * msGeomConst_piOver2;;
        if (residualAngle < tol)
            {
            phiNew = fi * msGeomConst_piOver2;
            }
        else if (msGeomConst_piOver2 - residualAngle < tol)
            {
            phiNew = (fi + 1) * msGeomConst_piOver2;
            }
        }
    return phiNew;
    }
/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            07/97
*
*  Adjust basis vectors and angles so curve falls in positive angle
*  space.   Basis vectors are assumed to be squared.
+----------------------------------------------------------------------*/
static   StatusInt    jmdlRotatedConic_classifyRotation

(
DMap4dP pHMap,
DRange3dP pRange,
int                 *pType,
double              *pHoopRadius,
DPoint3dCP pCenter,     /* => base circle center */
DVec3dCP pVector0,    /* => base ellipse 0 vector. Assumed perpendicular to z */
DVec3dCP pVector90,   /* => base ellipse 90 vector. Assumed parallel to z */
double sweep,               /* => sweep angle for rotation (around z) */
double phi0,                /* => start angle on original ellipse */
double phi1,                /* => end angle on original ellipse */
double      tolerance
)
    {
    DVec3d unitX, unitY, unitZ, circleNormal;
    DPoint3d inverseCenter;
    double r0, r90, rX;
    StatusInt status = ERROR;
    DVec3d vector0, vector90;
    DPoint3d center;
    double smallAngle = bsiTrig_smallAngle ();

    Transform transform;
    vector0 = *pVector0;
    vector90 = *pVector90;
    center = *pCenter;

    unitX = vector0;
    unitZ = vector90;
    r0 = unitX.Normalize ();
    r90 = unitZ.Normalize ();
    unitY.NormalizedCrossProduct (unitZ, unitX);
    inverseCenter.Init (
                            unitX.DotProduct (center),
                            unitY.DotProduct (center),
                            unitZ.DotProduct (center));

    rX = fabs (inverseCenter.x);

    if (fabs (inverseCenter.y) > tolerance)
        {
        /* Rotation axis is not in plane of base circle */
        }
    else if (rX <= tolerance)
        {
        /* Sphere */
        double phiMid, halfPhiSweep, toleranceAngle;
        double smallSweep       = 100.0 * smallAngle;

        toleranceAngle = tolerance / r0;
        phi0 = jmdlRotatedConic_lockAngle (phi0, toleranceAngle);
        phi1 = jmdlRotatedConic_lockAngle (phi1, toleranceAngle);

        phiMid       = 0.5 * (phi0 + phi1);
        halfPhiSweep    = 0.5 * fabs (phi1 - phi0);



        if (   bsiTrig_angleInSweep (phiMid, -msGeomConst_piOver2, msGeomConst_pi)
            && bsiTrig_angleInSweep (phi0, -msGeomConst_piOver2, msGeomConst_pi)
            && bsiTrig_angleInSweep (phi1, -msGeomConst_piOver2, msGeomConst_pi)
            )
            {
            *pType = RC_Sphere;
            phiMid = bsiTrig_adjustAngleToSweep (phiMid, -msGeomConst_piOver2, msGeomConst_piOver2);

            bsiDPoint3d_sizedCrossProduct (&circleNormal, &vector90, &vector0, r0);
            transform.SetTranslation (DPoint3d::From (0.0, 0.0, center.z));
            transform.SetMatrixColumn (vector0, 0);
            transform.SetMatrixColumn (circleNormal, 1);
            transform.SetMatrixColumn (vector90, 2);
            memset (pRange, 0, sizeof (DRange3d));
            pRange->low.x   = 0.0;
            pRange->high.x  = sweep;
            pRange->low.y   = phiMid - halfPhiSweep;
            pRange->high.y  = phiMid + halfPhiSweep;
            status = CHANGE_TO_BOOL(bsiDMap4d_initFromTransform (pHMap, &transform, false) ? SUCCESS : ERROR);
            }
        else if (   bsiTrig_angleInSweep (phiMid, msGeomConst_piOver2, msGeomConst_pi)
            && bsiTrig_angleInSweep (phi0, msGeomConst_piOver2, msGeomConst_pi)
            && bsiTrig_angleInSweep (phi1, msGeomConst_piOver2, msGeomConst_pi)
            )
            {
            *pType = RC_Sphere;
            phiMid = bsiTrig_adjustAngleToSweep (msGeomConst_pi - phiMid,
                                        -msGeomConst_piOver2, msGeomConst_piOver2);
            vector0.Scale (vector0, -1.0);
            bsiDPoint3d_sizedCrossProduct (&circleNormal, &vector90, &vector0, r0);
            transform.SetTranslation (DPoint3d::From (0.0, 0.0, center.z));
            transform.SetMatrixColumn (vector0, 0);
            transform.SetMatrixColumn (circleNormal, 1);
            transform.SetMatrixColumn (vector90, 2);
            memset (pRange, 0, sizeof (DRange3d));
            pRange->low.x   = 0.0;
            pRange->high.x  = sweep;
            pRange->low.y   = phiMid - halfPhiSweep;
            pRange->high.y  = phiMid + halfPhiSweep;
            status = CHANGE_TO_BOOL(bsiDMap4d_initFromTransform (pHMap, &transform, false) ? SUCCESS : ERROR);
            }
        else if (fabs(sweep) + smallAngle >= msGeomConst_pi
                && halfPhiSweep > smallSweep)
            {
            /* Look for cases where the generator hangs symmetrically from the
                North or South pole and swings enough to make a full theta circle.
                Juggle the angles and recurse to handle it. */
            phiMid = bsiTrig_adjustAngleToSweep (phiMid, -msGeomConst_piOver2, msGeomConst_piOver2);
            if (fabs (phiMid - msGeomConst_piOver2) < smallAngle)
                {
                status = jmdlRotatedConic_classifyRotation (
                            pHMap, pRange, pType, pHoopRadius,
                            &center, &vector0, &vector90,
                            msGeomConst_2pi,
                            msGeomConst_piOver2 - halfPhiSweep,
                            msGeomConst_piOver2,
                            tolerance);
                }
            else if (fabs (phiMid + msGeomConst_piOver2) < smallAngle)
                {
                status = jmdlRotatedConic_classifyRotation (
                            pHMap, pRange, pType, pHoopRadius,
                            &center, &vector0, &vector90,
                            msGeomConst_2pi,
                            -msGeomConst_piOver2,
                            -msGeomConst_piOver2 + halfPhiSweep,
                            tolerance);
                }

            }
        }
    else if (r0 < rX)
        {
        /* Torus with no chance to cross the axis */
        double lambda;
        double rCenter;
        if (inverseCenter.x < 0.0)
            {
            vector0.Scale (vector0, -1.0);
            phi0 = msGeomConst_pi - phi0;
            phi1 = msGeomConst_pi - phi1;
            inverseCenter.x = - inverseCenter.x;
            }
        rCenter = rX;
        lambda = r90 / r0;
        transform.SetTranslation (DPoint3d::From (0.0, 0.0, center.z));
        transform.SetMatrixColumn (DVec3d::From (center.x, center.y, 0.0), 0);
        transform.SetMatrixColumn (DVec3d::From (-center.y, center.x, 0.0), 1);
        transform.SetMatrixColumn (DVec3d::From (0.0, 0.0, lambda * rCenter), 2);

        memset (pRange, 0, sizeof (DRange3d));
        pRange->low.x   = 0.0;
        pRange->high.x  = sweep;
        pRange->low.y   = phi0;
        pRange->high.y  = phi1;
        *pHoopRadius = r0 / rCenter;
        *pType = RC_Torus;

        status = CHANGE_TO_BOOL(bsiDMap4d_initFromTransform (pHMap, &transform, false) ? SUCCESS : ERROR);
        }
    return status;
    }

/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            07/97
*
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt  bsiRotatedConic_normalizeConic
        /* SUCCESS if the surface's conic was converted to a purely cartesian form */

(
RotatedConic *pDest,   /* <=> surface */
const RotatedConic *pSource     /* => original surface */
)
    {
    RotatedConic source = *(RotatedConic *)pSource; /* HP needs typecast */
    StatusInt status = ERROR;
    switch (source.type)
        {
        case RC_RotatedLine:
            if (source.conic.vector0.w == 1.0 && source.conic.vector90.w == 1.0)
                {
                double r0sq =  source.conic.vector0.x * source.conic.vector0.x
                            +  source.conic.vector0.y * source.conic.vector0.y;
                double r1sq = source.conic.vector90.x * source.conic.vector90.x
                            + source.conic.vector90.y * source.conic.vector90.y;
                if (r0sq >= r1sq)
                    {
                    status = SUCCESS;
                    }
                else
                    {
                    DPoint4d tempPoint;
                    tempPoint = source.conic.vector0;
                    source.conic.vector0 = source.conic.vector90;
                    source.conic.vector90 = tempPoint;
                    status = SUCCESS;
                    }

                }
            break;

        case RC_RotatedEllipse:
            if (source.conic.vector0.w == 0.0 && source.conic.vector90.w == 0.0
                && source.conic.center.w == 1.0)
                {
                status = SUCCESS;
                }
            break;
        }

    if (SUCCESS == status)
        *pDest = source;
    else
        *pDest = *pSource;

    return status;
    }
static void thisIsAGoodPlaceToBreak

(
)
    {

    }
/*----------------------------------------------------------------*//**
*
* @bsihdr EarlinLutz                            07/97
*
*  Make a copy of pSurface, attempting to reclassifying if to the
*  simplest form possible.
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool    bsiRotatedConic_copyClassified    /* true if the surface was reclassified in the copy */

(
RotatedConic    *pDest,   /* <= copied surface */
const RotatedConic *pSource     /* => original surface */
)
    {
    RotatedConic source;
    DPoint3d origin, xPoint, yPoint, zPoint;
    Transform localFrame;
    DMap4d localMap;
    //static double minTorusRatio = 1.001;  /* Don't allow torus become lemon or apple */
    memset (&source, 0, sizeof (source));
    DEllipse4d     conic;
    double       smallAngle = bsiTrig_smallAngle ();
    bool        changed = false;
    switch (pSource->type)
        {
        case RC_RotatedLine:
            {
            if (SUCCESS == bsiRotatedConic_normalizeConic (&source, pSource))
                {
                /*  The normalized line is strictly real within the local system, with r0 >= r1 */
                double r0, r1, theta0, theta1;
                conic = source.conic;

                r0 = sqrt ( conic.vector0.x *  conic.vector0.x +  conic.vector0.y *  conic.vector0.y);
                r1 = sqrt (conic.vector90.x * conic.vector90.x + conic.vector90.y * conic.vector90.y);

                if (r0 <= source.tolerance && r1 <= source.tolerance)
                    {
                    /* straight line on z axis */
                    thisIsAGoodPlaceToBreak ();
                    }
                else if (fabs (conic.vector0.z - conic.vector90.z) <= source.tolerance)
                    {
                    if (r0 <= source.tolerance && r1 > source.tolerance)
                        {
                        origin.Init (         0.0,        0.0, conic.vector90.z);
                        xPoint.Init (  conic.vector90.x, conic.vector90.y,        0.0);
                        yPoint.Init ( -conic.vector90.y, conic.vector90.x,        0.0);
                        zPoint.Init (         0.0,        0.0,         r1);
                        bsiTransform_initFrom4Points (&localFrame, &origin, &xPoint, &yPoint, &zPoint); bsiDMap4d_initFromTransform (&localMap, &localFrame, false) ; /* THISWAS a bool thrown away as a statement */
                        bsiDMap4d_multiply (&source.rotationMap, &source.rotationMap, &localMap);
                        bsiRotatedConic_setParameterRange (&source, 0.0, source.sweep, 0.0, 1.0);
                        source.type = RC_Disk;
                        changed = true;
                        }
                    else if (r1 <= source.tolerance && r0 > source.tolerance)
                        {
                        origin.Init (         0.0,       0.0,  conic.vector0.z);
                        xPoint.Init (   conic.vector0.x, conic.vector0.y,        0.0);
                        yPoint.Init (  -conic.vector0.y, conic.vector0.x,        0.0);
                        zPoint.Init (         0.0,       0.0,         r0);
                        bsiTransform_initFrom4Points (&localFrame,
                                        &origin, &xPoint, &yPoint, &zPoint); bsiDMap4d_initFromTransform (&localMap, &localFrame, false) ; /* THISWAS a bool thrown away as a statement */
                        bsiDMap4d_multiply (&source.rotationMap, &source.rotationMap, &localMap);
                        bsiRotatedConic_setParameterRange (&source, 0.0, source.sweep, 0.0, 1.0);
                        source.type = RC_Disk;
                        changed = true;
                        }
                    else
                        {
                        double delta;
                        theta0 = bsiTrig_atan2(  conic.vector0.y,  conic.vector0.x);
                        theta1 = bsiTrig_atan2( conic.vector90.y, conic.vector90.x);
                        delta = fabs (theta0 - theta1) * r1;
                        if (delta > pSource->tolerance)
                            {
                            /* it's a disk with a hole and a skewed sweeper */
                            thisIsAGoodPlaceToBreak ();
                            }
                        else
                            {
                            /* simple disk with hole */
                            double rMin, rMax;
                            double rFract;
                            if (r0 > r1)
                                {
                                xPoint.Init (    conic.vector0.x,  conic.vector0.y, 0.0);
                                rMax = r0;
                                rMin = r1;
                                }
                            else
                                {
                                xPoint.Init (   conic.vector90.x, conic.vector90.y,  0.0);
                                rMax = r1;
                                rMin = r0;
                                }

                            origin.Init (         0.0,       0.0,  conic.vector0.z);
                            yPoint.Init (  -xPoint.y,   xPoint.x,        0.0);
                            zPoint.Init (         0.0,       0.0,         r0);
                            bsiTransform_initFrom4Points (&localFrame,
                                            &origin, &xPoint, &yPoint, &zPoint); bsiDMap4d_initFromTransform(&localMap, &localFrame, false) ; /* THISWAS a bool thrown away as a statement */
                            bsiDMap4d_multiply (&source.rotationMap, &source.rotationMap, &localMap);
                            rFract = rMin / rMax;
                            bsiRotatedConic_setParameterRange (&source, 0.0, source.sweep, rFract, 1.0 - rFract);
                            source.type = RC_Disk;
                            changed = true;
                            }
                        }
                    }
                else if (r1 < smallAngle * r0)
                    {
                    /* Cone with apex */
                    theta0 = bsiTrig_atan2( conic.vector0.y, conic.vector0.x);
                    origin.Init (  0.0,             0.0,             conic.vector90.z);
                    xPoint.Init (  conic.vector0.x, conic.vector0.y, conic.vector90.z);
                    yPoint.Init ( -conic.vector0.y, conic.vector0.x, conic.vector90.z);
                    zPoint.Init (  0.0,             0.0,             conic.vector0.z);
                    bsiTransform_initFrom4Points (&localFrame, &origin, &xPoint, &yPoint, &zPoint); bsiDMap4d_initFromTransform (&localMap, &localFrame, false) ; /* THISWAS a bool thrown away as a statement */
                    bsiDMap4d_multiply (&source.rotationMap, &source.rotationMap, &localMap);
                    bsiRotatedConic_setParameterRange (&source, 0.0, source.sweep, 0.0, 1.0);
                    source.type = RC_Cone;
                    changed = true;
                    }
                else
                    {
                    /* Both points are off-axis, and at different altitudes.  Are they coplanar with the
                        axis ? */
                    theta0 = bsiTrig_atan2(  conic.vector0.y,  conic.vector0.x);
                    theta1 = bsiTrig_atan2( conic.vector90.y, conic.vector90.x);
                    if (!bsiTrig_equalAngles (theta0, theta1))
                        {
                        /* it's a hyperboloid or cone with internal apex */
                        }
                    else
                        {
                        double lambda = r1 / r0;
                        DMap4d localToCylinderMap;
                        double dZ = conic.vector90.z - conic.vector0.z;
                        double scale = dZ >= 0.0 ? 1.0 : -1.0;

                        /* Truncated cone */
                        origin.Init (  0.0,           0.0,             conic.vector0.z);
                        xPoint.Init (  conic.vector0.x, conic.vector0.y, conic.vector0.z);
                        yPoint.Init ( -scale * conic.vector0.y, scale * conic.vector0.x, conic.vector0.z);
                        zPoint.Init (  0.0,           0.0,             conic.vector90.z); bsiDMap4d_init3PointCameraAndFrustum(NULL, NULL, &localToCylinderMap, &origin, &xPoint, &yPoint, &zPoint, lambda) ; /* THISWAS a bool thrown away as a statement */;
                        bsiDMap4d_invert (&localMap, &localToCylinderMap);
                        bsiDMap4d_multiply (&source.rotationMap, &source.rotationMap, &localMap);
                        source.type = RC_Cylinder;
                        bsiRotatedConic_setParameterRange (&source, 0.0, scale * source.sweep, 0.0, 1.0);
                        changed = true;
                        }
                    }
                break;
                }
            }
        case RC_RotatedEllipse:
            {
            if (SUCCESS == bsiRotatedConic_normalizeConic (&source, pSource))
                {
                DEllipse4d normalizedEllipse;
                double ellipseAngle0, ellipseAngle1;
                DVec3d vector0, vector90;
        DPoint3d center;
                DMap4d placementMap;
                DRange3d newRange;
                double hoopRadius;
                int newType;

                bsiDEllipse4d_normalizeAxes (&normalizedEllipse, &source.conic);
                bsiDEllipse4d_getSegment (&ellipseAngle0, &ellipseAngle1, &normalizedEllipse, 0);

                bsiDPoint4d_cartesianFromHomogeneous (&normalizedEllipse.vector0 , &vector0);
                bsiDPoint4d_cartesianFromHomogeneous (&normalizedEllipse.vector90, &vector90);
                bsiDPoint4d_cartesianFromHomogeneous (&normalizedEllipse.center  , &center  );

                if (   SUCCESS == jmdlRotatedConic_squareLocalBasis (&vector0, &vector90, &ellipseAngle0, &ellipseAngle1,
                                                                    source.tolerance)
                    && SUCCESS == jmdlRotatedConic_classifyRotation (&placementMap, &newRange, &newType, &hoopRadius,
                                                                    &center, &vector0, &vector90,
                                                                    source.sweep, ellipseAngle0, ellipseAngle1,
                                                                    source.tolerance)
                    )
                    {
                    source.type = (RotatedConicType)newType;
                    source.hoopRadius = hoopRadius;
                    source.parameterRange = newRange;
                    bsiDMap4d_multiply (&source.rotationMap, &source.rotationMap, &placementMap);
                    changed = true;
                    }
                }
            break;
            }
        }

    if (changed)
        {
        *pDest = source;
        }
    else
        {
        *pDest = *pSource;
        }
    return changed;
    }

/*---------------------------------------------------------------------------------**//**
* Convert to rotated conic form.
* @indexVerb rotatedConic
* @bsihdr                                                       EarlinLutz      04/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool               bsiDEllipsoid3d_getRotatedConic

(
DEllipsoid3dCP pInstance,
RotatedConic      *pConic
)
    {
    DMap4d hFrame;

    double theta0, dTheta, phi0, dPhi;
    bool    boolStat;

    /* Build a rotated conic. */
    boolStat = bsiDMap4d_initFromTransform (&hFrame, &pInstance->frame, false);

    if (boolStat)
        {
        bsiDEllipsoid3d_getScalarNaturalParameterSweep (pInstance, &theta0, &dTheta, &phi0, &dPhi);
        bsiRotatedConic_initFrameAndSweep
                    (
                    pConic, RC_Sphere, &hFrame,
                    theta0, dTheta,
                    phi0, dPhi
                    );
        }
    return boolStat;
    }

/*---------------------------------------------------------------------------------**//**
* Convert to rotated conic form.
* @indexVerb rotatedConic
* @bsihdr                                                       EarlinLutz      04/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool           bsiDCone3d_getRotatedConic

(
DCone3dCP pInstance,
RotatedConic  *pConic
)
    {
    DMap4d hFrame;

    double theta0, dTheta, z0, dz, z1;
    bool    boolStat;
    //static double s_coneTol = 1.0e-12;
    RotatedConic workConic;
    DPoint3d line0, line1, center;
    DVec3d axis;
    double r0, r1;
    double cosTheta, sinTheta;

    /* Build a rotated conic.  RotatedConic's differentiate between cone and cylinder
            a little arbitrarily, so we generate it as a rotated line segment and
            let RotConic make the decision. */
    boolStat = bsiDMap4d_initFromTransform (&hFrame, &pInstance->frame, false);

    if (boolStat)
        {
        bsiDCone3d_getScalarNaturalParameterSweep (pInstance, &theta0, &dTheta, &z0, &dz);
        z1 = z0 + dz;
        cosTheta = cos (theta0);
        sinTheta = sin (theta0);
        r0 = bsiDCone3d_heightToRadius (pInstance, z0);
        r1 = bsiDCone3d_heightToRadius (pInstance, z1);

        line0.Init ( r0 * cosTheta, r0 * sinTheta, z0);
        line1.Init ( r1 * cosTheta, r1 * sinTheta, z1);

        center.Init ( 0.0, 0.0, 0.0);
        axis.Init (   0.0, 0.0, 1.0);

        bsiRotatedConic_initRotatedLine
                    (
                    &workConic,
                    &line0,
                    &line1,
                    &center,
                    &axis,
                    dTheta
                    );
        boolStat = bsiRotatedConic_copyClassified (pConic, &workConic);
        if (boolStat)
            bsiRotatedConic_transform (pConic, &hFrame, NULL);
        }

    return boolStat;
    }
/*---------------------------------------------------------------------------------**//**
* Convert to rotated conic form.
* @indexVerb rotatedConic
* @bsihdr                                                       EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool               bsiDToroid3d_getRotatedConic

(
DToroid3dCP pInstance,
RotatedConic      *pConic
)
    {
    DMap4d hFrame;

    double theta0, dTheta, phi0, dPhi;
    bool    boolStat;

    /* Build a rotated conic. */
    boolStat = bsiDMap4d_initFromTransform (&hFrame, &pInstance->frame, false);

    if (boolStat)
        {
        bsiDToroid3d_getScalarNaturalParameterSweep (pInstance, &theta0, &dTheta, &phi0, &dPhi);
        bsiRotatedConic_initFrameAndSweep
                        (
                        pConic, RC_Torus, &hFrame,
                        theta0, dTheta,
                        phi0, dPhi
                        );
        bsiRotatedConic_setHoopRadius (pConic, pInstance->minorAxisRatio);
        }
    return boolStat;
    }

/*---------------------------------------------------------------------------------**//**
* Convert to rotated conic form.
* @indexVerb rotatedConic
* @bsihdr                                                       EarlinLutz      04/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool           bsiDDisk3d_getRotatedConic

(
DDisk3dCP pInstance,
RotatedConic  *pConic
)
    {
    DMap4d hFrame;

    double theta0, theta1, dTheta;
    bool    boolStat;
    //static double s_coneTol = 1.0e-12;
    RotatedConic workConic;
    DPoint3d line0, line1, center;
    DVec3d axis;
    double r0, r1;
    double cosTheta, sinTheta;

    /* Build a rotated conic.  RotatedConic's differentiate between disk and cylinder
            a little arbitrarily, so we generate it as a rotated line segment and
            let RotConic make the decision. */
    boolStat = bsiDMap4d_initFromTransform (&hFrame, &pInstance->frame, false);

    if (boolStat)
        {
        bsiDDisk3d_getScalarNaturalParameterSweep (pInstance, &r0, &r1, &theta0, &theta1);
        dTheta = theta1 - theta0;

        cosTheta = cos (theta0);
        sinTheta = sin (theta0);

        line0.Init ( r0 * cosTheta, r0 * sinTheta, 0.0);
        line1.Init ( r1 * cosTheta, r1 * sinTheta, 0.0);

        center.Init ( 0.0, 0.0, 0.0);
        axis.Init (   0.0, 0.0, 1.0);

        bsiRotatedConic_initRotatedLine
                    (
                    &workConic,
                    &line0,
                    &line1,
                    &center,
                    &axis,
                    dTheta
                    );
        boolStat = bsiRotatedConic_copyClassified (pConic, &workConic);
        if (boolStat)
            bsiRotatedConic_transform (pConic, &hFrame, NULL);
        }

    return boolStat;
    }

END_BENTLEY_GEOMETRY_NAMESPACE
