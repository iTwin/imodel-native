/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include <Geom/MstnOnly/GeomPrivateApi.h>
#include "DeprecatedFunctions.h"
#include "../memory/ArrayWrapper.h"
typedef VArrayWrapper<int>      IntArrayWrapper;
typedef VArrayWrapper<DPoint3d>   DPoint3dArrayWrapper;
#define FIX_MIN(value, min)          if (value < min) min = value
#define FIX_MAX(value, max)          if (value > max) max = value
#define FIX_MINMAX(value, min, max)  FIX_MIN(value, min); FIX_MAX(value, max);
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*-----------------------------------------------------------------*//**
Return a crude 5-case summary of the z-direction components of a matrix.
@param pMatrix => matrix to analyze.
@return
<ul>
<li>  1 if complete effect of z entries is identity.</li>
<li>  -1 if complete effect of z entries is to negate z</li>
<li>  2 if complete effect of z entries is non-unit positive scale on z (zeros off diagonal)</li>
<li>  -2 if complete efect of z entries is non-unit negative scale on z (zeros off diagonal)</li>
<li>  0 if there are any off-diagonal nonzeros, so the matrix has z effects other than simple scaling.</li>
</ul>
@group "RotMatrix Queries"
 @bsimethod                                     EarlinLutz      02/02
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int bsiRotMatrix_summaryZEffects


(
RotMatrixCP pMatrix
)
    {
    double tol = bsiTrig_smallAngle ();
    double zz = pMatrix->form3d[2][2];
    if (    fabs (pMatrix->form3d[2][0]) > tol
       ||   fabs (pMatrix->form3d[2][1]) > tol
       ||   fabs (pMatrix->form3d[0][2]) > tol
       ||   fabs (pMatrix->form3d[1][2]) > tol
       ||   fabs (zz) < tol
       )
        {
        return 0;
        }
    if (fabs (zz - 1.0) < tol)
        return 1;
    if (fabs (zz + 1.0) < tol)
        return -1;
    if (zz > 0.0)
        return 2;
    if (zz < 0.0)
        return -2;
    return 0;   /* Can't get here. */
    }

/*-----------------------------------------------------------------*//**
* @description Store origin and unnormalized vector.
* @param pPlane <= initialized plane.
* @param pOrigin => origin point
* @param pNormal => normal vector
* @group "DPlane3d Initialization"
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public  void bsiDPlane3d_initFromOriginAndNormal

(
DPlane3dP pPlane,
DPoint3dCP pOrigin,
DVec3dCP pNormal
)
    {
    pPlane->origin = *pOrigin;
    pPlane->normal = *pNormal;
    }

/*-----------------------------------------------------------------*//**
* @description Return the plane as a DPoint4d.
* @param pPlane => plane structure with origin, normal
* @param pHPlane <= 4D plane coefficients
* @see bsiDPlane3d_initFromDPoint4d
* @group "DPlane3d Queries"
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public  void      bsiDPlane3d_getDPoint4d

(
DPlane3dCP pPlane,
DPoint4dP pHPlane
)
    {
    bsiDPlane3d_getImplicitPlaneCoefficients
                            (
                            pPlane,
                            &pHPlane->x,
                            &pHPlane->y,
                            &pHPlane->z,
                            &pHPlane->w
                            );
    pHPlane->w = -pHPlane->w;
    }


/*-----------------------------------------------------------------*//**
* @description Convert the plane to implicit coeffcients ax+by+cz=d.
* @remarks WARNING: Check your usage.  It is about equally common to write the plane equation with
*       negated d, i.e. ax+by+cz+d=0.  If so, pass in (a,b,c,-d).
* @param pPlane => plane structure with origin, normal
* @param pA <= 4D plane x-coefficient
* @param pB <= 4D plane y-coefficient
* @param pC <= 4D plane z-coefficient
* @param pD <= 4D plane constant coefficient
* @see bsiDPlane3d_initFromImplicitPlaneCoefficients
* @group "DPlane3d Initialization"
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public  void bsiDPlane3d_getImplicitPlaneCoefficients

(
DPlane3dCP pPlane,
double      *pA,
double      *pB,
double      *pC,
double      *pD
)
    {
    *pA = pPlane->normal.x;
    *pB = pPlane->normal.y;
    *pC = pPlane->normal.z;
    *pD = pPlane->origin.DotProduct (pPlane->normal);
    }

/*---------------------------------------------------------------------------------**//**
* Get the parameter range as start/sweep pairs.
* @indexVerb parameterRange
* @bsihdr                                                       EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public  void    bsiDDisk3d_getScalarNaturalParameterSweep

(
DDisk3dCP pInstance,
double          *pRadius0,
double          *pRadiusDelta,
double          *pAngle0,
double          *pAngleSweep
)
    {
    *pRadius0       = pInstance->parameterRange.low.x;
    *pRadiusDelta   = pInstance->parameterRange.high.x - *pRadius0;

    *pAngle0        = pInstance->parameterRange.low.y;
    *pAngleSweep    = pInstance->parameterRange.high.y - *pAngle0;
    }

/*---------------------------------------------------------------------------------**//**
* @description Set all fields of the cone from arguments.
* @param pCone <= cone to initialize
* @param pFrame => Transformation whose translation, x, y, and z columns are the
*                   center, 0-degree vector, 90-degree vector, and axis direction.
*                   if <code>null</code>, an identity is used.
* @param radiusFraction => top circle radius divided by bottom circle radius.
* @param pRange     => parameter range in <code>(theta, z)</code> coordinates.
* @group "DCone3d Initialization"
* @bsihdr                                                       EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public  void    bsiDCone3d_setFrameAndFraction
(
DCone3dP pCone,
TransformCP pFrame,
double       radiusFraction,
DRange2dCP pRange
)
    {
    Transform dFrame;
    if (pFrame)
        dFrame = *pFrame;
    else
        dFrame.InitIdentity ();
    bsiDCone3d_set (pCone, &dFrame, radiusFraction, pRange);
    }

/*---------------------------------------------------------------------------------**//**
* @description Compute angles for silhouettes of the cone with respect to a (possibly perspective) view transformation.
* @param pCone  => cone to evaluate
* @param pTrigPoint <= array where x,y are cosine, sine of
*                      silhouette angles. z is zero -- maybe a convenient
*                      place for the angles if you need to stash them
* @param pMap       => view transformation
* @param pEyePoint => eyepoint, in same coordinates.
*                     For perspective, from xyz, set w=1
*                     For flat view in direction xyz, set w=0
* @return number of silhouette angles.
* @group "DCone3d Silhouette"
* @bsihdr                                                       EarlinLutz      03/99
+---------------+---------------+---------------+---------------+------*/
Public  int      bsiDCone3d_silhouetteAngles

(
DCone3dCP pCone,
DPoint3dP pTrigPoint,
DMap4dCP pMap,
DPoint4dCP pEyePoint
)
    {
    Transform Tinverse;
    int numOut = 0;
    DPoint4d cylinderEye, coneEye;
    double lambda = pCone->radiusFraction;
    double mu = 1.0 - lambda;
    static double s_coneSilhouetteTangencyTolerance = 1.0e-14;
    int i;


    if (Tinverse.InverseOf (pCone->frame))

        {
        cylinderEye = *pEyePoint;
        /* invert the eye as needed: */
        if (pMap)
            bsiDMatrix4d_multiply4dPoints (&pMap->M1, &cylinderEye, &cylinderEye, 1);
        Tinverse.Multiply (&cylinderEye, &cylinderEye, 1);

        /* Homogeneous transform to put cone apex at infinity */
        coneEye.x = cylinderEye.x;
        coneEye.y = cylinderEye.y;
        coneEye.z = cylinderEye.z;
        coneEye.w = cylinderEye.w - mu * cylinderEye.z;

        pTrigPoint[0].Zero ();
        pTrigPoint[1].Zero ();

        numOut = bsiMath_solveApproximateUnitQuadratic
                        (
                        &pTrigPoint[0].x, &pTrigPoint[0].y,
                        &pTrigPoint[1].x, &pTrigPoint[1].y,
                        -coneEye.w,
                        coneEye.x,
                        coneEye.y,
                        s_coneSilhouetteTangencyTolerance
                        );
        for (i = 0; i < numOut; i++)
            {
            pTrigPoint[i].z = atan2 (pTrigPoint[i].y, pTrigPoint[i].x);
            }

        }

    return numOut;
    }
/*-----------------------------------------------------------------*//**
* Multiply an array of points by a matrix, using all components of both the matrix
* and the points.
*
* @instance pA => matrix term of product.
* @param pOutPoint <= Array of products A*pPoint[i] renormalized
* @param pInPoint => Array of points points
* @param n => number of points
* @see
* @indexVerb
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDMatrix4d_multiplyAndRenormalizeDPoint3dArray

(
DMatrix4dCP pA,
DPoint3dP pOutPoint,
DPoint3dCP pInPoint,
int n
)
    {
    int i;
    const DPoint3d *sourceP;
    DPoint3d *pDest;
    double x,y,z;
    double rX, rY, rZ, rW;

    for ( i = 0 , sourceP = pInPoint , pDest = pOutPoint ;
          i < n ;
          i++, sourceP++, pDest++ )
        {
        x = sourceP->x;
        y = sourceP->y;
        z = sourceP->z;
        rX =
            pA->coff[0][0] * x +
            pA->coff[0][1] * y +
            pA->coff[0][2] * z +
            pA->coff[0][3];
        rY =
            pA->coff[1][0] * x +
            pA->coff[1][1] * y +
            pA->coff[1][2] * z +
            pA->coff[1][3];
        rZ =
            pA->coff[2][0] * x +
            pA->coff[2][1] * y +
            pA->coff[2][2] * z +
            pA->coff[2][3];
        rW =
            pA->coff[3][0] * x +
            pA->coff[3][1] * y +
            pA->coff[3][2] * z +
            pA->coff[3][3];

        if (rW == 1.0 || rW == 0.0)
            {
            pDest->x = rX;
            pDest->y = rY;
            pDest->z = rZ;
            }
        else
            {
            double a = 1.0 / rW;
            pDest->x = rX * a;
            pDest->y = rY * a;
            pDest->z = rZ * a;
            }
        }
    }


/*-----------------------------------------------------------------*//**
*
* Matrix multiplication, using all components of both the matrix
* and the points.
*
* @instance pA => Matrix term of multiplication.
* @param pOutPoint <= Array of homogeneous products A*pInPoint[i]
* @param pInPoint => Array of homogeneous points
* @param n => number of points
* @see bsiDMatrix4d_multiplyAndRenormalize
* @indexVerb
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDMatrix4d_multiply4dPoints

(
DMatrix4dCP pA,
DPoint4dP pOutPoint,
DPoint4dCP pInPoint,
int n
)
    {
    int i;
    const DPoint4d *pCurrPoint;
    DPoint4d *pDest;
    double x,y,z,w;

    for ( i = 0 , pCurrPoint = pInPoint , pDest = pOutPoint ;
          i < n ;
          i++, pCurrPoint++, pDest++ )
        {
        x = pCurrPoint->x;
        y = pCurrPoint->y;
        z = pCurrPoint->z;
        w = pCurrPoint->w;
        pDest->x =
            pA->coff[0][0] * x +
            pA->coff[0][1] * y +
            pA->coff[0][2] * z +
            pA->coff[0][3] * w ;
        pDest->y =
            pA->coff[1][0] * x +
            pA->coff[1][1] * y +
            pA->coff[1][2] * z +
            pA->coff[1][3] * w ;
        pDest->z =
            pA->coff[2][0] * x +
            pA->coff[2][1] * y +
            pA->coff[2][2] * z +
            pA->coff[2][3] * w ;
        pDest->w =
            pA->coff[3][0] * x +
            pA->coff[3][1] * y +
            pA->coff[3][2] * z +
            pA->coff[3][3] * w ;
        }

    }

/*-----------------------------------------------------------------*//**
* Matrix*point multiplication, with input points represented by
* separate DPoint3d and weight arrays.

* @instance pA => matrix
* @param pHPoint <= Array of homogeneous products A*pPoint[i]
* @param pPoint => Array of xyz coordinates
* @param pWeight => weight array. If NULL, unit weight is used
* @param n => number of points
* @see
* @indexVerb
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDMatrix4d_multiplyWeightedDPoint3dArray

(
DMatrix4dCP pA,
DPoint4dP pHPoint,
DPoint3dCP pPoint,
const double *pWeight,
int n
)
    {
    int i;
    const DPoint3d *pCurrPoint;
    double w;
    if ( !pWeight )
        {
        for ( i = 0 , pCurrPoint = pPoint ; i < n; i++, pCurrPoint++ )
            {
            pHPoint[i].x =
                pA->coff[0][0] * pCurrPoint->x +
                pA->coff[0][1] * pCurrPoint->y +
                pA->coff[0][2] * pCurrPoint->z +
                pA->coff[0][3];
            pHPoint[i].y =
                pA->coff[1][0] * pCurrPoint->x +
                pA->coff[1][1] * pCurrPoint->y +
                pA->coff[1][2] * pCurrPoint->z +
                pA->coff[1][3];
            pHPoint[i].z =
                pA->coff[2][0] * pCurrPoint->x +
                pA->coff[2][1] * pCurrPoint->y +
                pA->coff[2][2] * pCurrPoint->z +
                pA->coff[2][3];
            pHPoint[i].w =
                pA->coff[3][0] * pCurrPoint->x +
                pA->coff[3][1] * pCurrPoint->y +
                pA->coff[3][2] * pCurrPoint->z +
                pA->coff[3][3];
            }
        }
    else
        {
        for ( i = 0 , pCurrPoint = pPoint ; i < n; i++, pCurrPoint++ )
            {
            w = pWeight[i];
            pHPoint[i].x =
                pA->coff[0][0] * pCurrPoint->x +
                pA->coff[0][1] * pCurrPoint->y +
                pA->coff[0][2] * pCurrPoint->z +
                pA->coff[0][3] * w;
            pHPoint[i].y =
                pA->coff[1][0] * pCurrPoint->x +
                pA->coff[1][1] * pCurrPoint->y +
                pA->coff[1][2] * pCurrPoint->z +
                pA->coff[1][3] * w;
            pHPoint[i].z =
                pA->coff[2][0] * pCurrPoint->x +
                pA->coff[2][1] * pCurrPoint->y +
                pA->coff[2][2] * pCurrPoint->z +
                pA->coff[2][3] * w;
            pHPoint[i].w =
                pA->coff[3][0] * pCurrPoint->x +
                pA->coff[3][1] * pCurrPoint->y +
                pA->coff[3][2] * pCurrPoint->z +
                pA->coff[3][3] * w;
            }
        }
    }


/*---------------------------------------------------------------------------------**//**
* @description Return a rule line at specified longitude (angle around cone).
* to the z range
* @param pCone  => cone to evaluate
* @param pSegment  <= ruling segment.
* @param theta => longitude angle (radians)
* @return true if theta is within the parameter range for the cone.
* @group "DCone3d Rule Lines"
* @bsihdr                                                       EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public  bool     bsiDCone3d_getRuleLine

(
DCone3dCP pCone,
DSegment3dP pSegment,
double          theta
)
    {
    double              cosTheta = cos (theta);
    double              sinTheta = sin (theta);
    double              r1 = pCone->radiusFraction;
    double              z0, z1, theta0, theta1;
    double              sweep;

    bsiDCone3d_getScalarNaturalParameterRange (pCone, &theta0, &theta1, &z0, &z1);
    sweep = theta1 - theta0;

    pCone->frame.Multiply (*(&pSegment->point[0]), cosTheta, sinTheta, z0);
    pCone->frame.Multiply (*(&pSegment->point[1]), r1 * cosTheta, r1 * sinTheta, z1);

    return Angle::InSweepAllowPeriodShift (theta, theta0, sweep);
    }

/*---------------------------------------------------------------------------------**//**
* @description Return the cone's local radius at the specified local height.
* @param pCone  => cone to evaluate
* @param z => height in local coordinates
* @return the cone's radius in local coordinates
* @group "DCone3d Local Coordinates"
* @bsihdr                                                       EarlinLutz      03/99
+---------------+---------------+---------------+---------------+------*/
Public  double   bsiDCone3d_heightToRadius

(
DCone3dCP pCone,
double          z
)
    {
    return 1.0 + (pCone->radiusFraction - 1.0) * z;
    }

/*---------------------------------------------------------------------------------**//**
* @description Get the parameter range as start/sweep pairs.
* @param pCone      => cone to evaluate
* @param pTheta0        <= start angle
* @param pThetaSweep    <= angle sweep
* @param pZ0            <= start altitude
* @param pZSweep        <= altitude sweep
* @group "DCone3d Parameter Range"
* @bsihdr                                                       EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public  void    bsiDCone3d_getScalarNaturalParameterSweep

(
DCone3dCP pCone,
double          *pTheta0,
double          *pThetaSweep,
double          *pZ0,
double          *pZSweep
)
    {
    *pTheta0 = pCone->parameterRange.low.x;
    *pThetaSweep = pCone->parameterRange.high.x - *pTheta0;

    *pZ0 = pCone->parameterRange.low.y;
    *pZSweep = pCone->parameterRange.high.y - *pZ0;
    }

/*---------------------------------------------------------------------------------**//**
* @description Return the range of the natural parameter for the active surface patch.
* @param pCone  => cone to evaluate
* @param pParam1Start => start value of natural parameter.
* @param pParam1End   => end value of natural parameter.
* @param pParam2Start => start value of natural parameter.
* @param pParam2End   => end value of natural parameter.
* @group "DCone3d Parameter Range"
* @bsihdr                                                       EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public  void     bsiDCone3d_getScalarNaturalParameterRange

(
DCone3dCP pCone,
double    *pParam1Start,
double    *pParam1End,
double    *pParam2Start,
double    *pParam2End
)
    {
    *pParam1Start = pCone->parameterRange.low.x;
    *pParam1End   = pCone->parameterRange.high.x;
    *pParam2Start = pCone->parameterRange.low.y;
    *pParam2End   = pCone->parameterRange.high.y;
    }
/*---------------------------------------------------------------------------------**//**
* Get the parameter range as start/sweep pairs.
* @indexVerb parameterRange
* @bsihdr                                                       EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public  void    bsiDEllipsoid3d_getScalarNaturalParameterSweep

(
DEllipsoid3dCP pInstance,
double          *pTheta0,
double          *pThetaSweep,
double          *pPhi0,
double          *pPhiSweep
)
    {
    *pTheta0 = pInstance->parameterRange.low.x;
    *pThetaSweep = pInstance->parameterRange.high.x - *pTheta0;

    *pPhi0 = pInstance->parameterRange.low.y;
    *pPhiSweep = pInstance->parameterRange.high.y - *pPhi0;
    }

static const DRange2d s_defaultParameterRange =
        {
            { -msGeomConst_pi, 0.0},
            {  msGeomConst_pi, 1.0},
        };



/*---------------------------------------------------------------------------------**//**
* @description Set all fields of the cone from arguments.
* @param pCone <= cone to initialize
* @param pFrame => Transformation whose translation, x, y, and z columns are the
*                   center, 0-degree vector, 90-degree vector, and axis direction.
*                   if <code>null</code>, an identity is used.
* @param radiusFraction => top circle radius divided by bottom circle radius.
* @param pRange     => parameter range in <code>(theta, phi)</code> coordinates.
* @group "DCone3d Initialization"
* @bsihdr                                                       EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public  void    bsiDCone3d_set

(
DCone3dP pCone,
TransformCP pFrame,
double       radiusFraction,
DRange2dCP pRange
)
    {
    if (pFrame)
        pCone->frame = *pFrame;
    else
        pCone->frame.InitIdentity ();

    if (pRange)
        pCone->parameterRange = *pRange;
    else
        {
        pCone->parameterRange = s_defaultParameterRange;
        }

    pCone->radiusFraction = radiusFraction;
    }


/*-----------------------------------------------------------------*//**
* Search an array for the closest point, using only
* x and y components.  Useful for screenproximity
* tests between points at different depths.
*
* @param pPoint => fixed point for tests
* @param pArray => array of test points
* @param nPoint => number of points
* @see
* @return index of closest point
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public  int bsiGeom_closestXYDPoint4d


(
DPoint3dCP pPoint,
DPoint4dCP pArray,
int             nPoint
)

    {
    int     iCurr, iMin;
    double  dCurr, dMin;

    iMin = -1;
    dMin = DBL_MAX;
    for (iCurr = 0; iCurr < nPoint; iCurr++)
        {
        if (pArray[iCurr].RealDistanceSquaredXY (&dCurr, *pPoint)
            && (iMin == -1 || dCurr < dMin))
            {
            dMin = dCurr;
            iMin = iCurr;
            }
        }

    return iMin;
    }

/*----------------------------------------------------------------------------+
| BARYCENTRIC COORDINATE FUNCTIONS:
|
| For a given triangle T with vertices v0, v1, v2, every point q in the plane
| of T is uniquely represented by its barycentric coordinates (b0, b1, b2)
| relative to T:
|
| q = b0 * v0 + b1 * v1 + b2 * v2,
| 1 = b0 + b1 + b2.
|
+----------------------------------------------------------------------------*/


Public bool    bsiDPoint3d_barycentricFromDPoint2dTriangleVectors

(
DPoint3dP pInstance,
double    *pArea,
DPoint2dCP pPoint,
DPoint2dCP pOrigin,
DPoint2dCP pVector1,
DPoint2dCP pVector2
)
    {
    bool        status = true;
    double      denom, numer1, numer2;

    /*
    Let vectors v1 and v2 emanating from the plane's origin define triangle T,
    and let q be a point in the plane.  Then the system for finding the
    barycentric coordinates of q relative to T reduces from 3x3 to 2x2:
      -             - -    -   -     -      -            - -    -   -     -
      | 0 v1.x v2.x | | b0 |   | q.x |      | v1.x  v2.x | | b1 |   | q.x |
      | 0 v1.y v2.y | | b1 | = | q.y |  =>  | v1.y  v2.y | | b2 | = | q.y |
      | 1    1    1 | | b2 |   | 1   |      -            - -    -   -     -
      -             - -    -   -     -
    We use Cramer's Rule to solve this system for b1 and b2; b0 can be found
    by subtracting the sum of the other two coords from 1.0.
    */

    /* Calculate numerators of Cramer's Rule formulae */
    if (pOrigin)
        {
        /*
        Since barycoords are invariant under affine transformation, we can
        translate the triangle and point so that pOrigin is the origin.  This
        gives us the dimension reduction detailed above.
        */
        numer1 = (pPoint->x - pOrigin->x) * pVector2->y -
                 (pPoint->y - pOrigin->y) * pVector2->x;
        numer2 = (pPoint->y - pOrigin->y) * pVector1->x -
                 (pPoint->x - pOrigin->x) * pVector1->y;
        }
    else
        {
        numer1 = pPoint->x * pVector2->y - pPoint->y * pVector2->x;
        numer2 = pPoint->y * pVector1->x - pPoint->x * pVector1->y;
        }

    /*
    Calculate denominator of Cramer's Rule formulae.  On a good day, denom is
    twice the signed area of T.  On a bad day (i.e. when T is long and skinny)
    we get subtractive cancellation, but there's no way around it!
    */
    denom  = pVector1->x * pVector2->y - pVector2->x * pVector1->y;

    /* Return false and barycoords (1,0,0) if denom relatively close to zero */
    if (! DoubleOps::SafeDivide (pInstance->y, numer1, denom, 0.0))
        status = false;

    if (! DoubleOps::SafeDivide (pInstance->z, numer2, denom, 0.0))
        status = false;

    pInstance->x = 1.0 - pInstance->y - pInstance->z;

    if (pArea)
        *pArea = 2.0 * denom;
    return status;
    }

/*-----------------------------------------------------------------*//**
* @description Sets this instance to the barycentric coordinates of pPoint
* relative to the triangle (pVertex0, pVertex1, pVertex2) in the xy-plane.
*
* @instance pInstance   <= barycentric coordinates of pPoint relative to T
* @param pPoint         => point in plane
* @param pVertex0       => vertex 0 of triangle T
* @param pVertex1       => vertex 1 of triangle T
* @param pVertex2       => vertex 2 of triangle T
* @see bsiDPoint3d_barycentricFromDPoint2dTriangleVectors
* @see bsiDPoint2d_fromBarycentricAndDPoint2dTriangle
* @group "DPoint3d Barycentric"
* @return true if and only if the area of T is sufficiently large.
* @bsihdr                                                                       DavidAssaf      10/98
+---------------+---------------+---------------+---------------+------*/
Public   bool    bsiDPoint3d_barycentricFromDPoint2dTriangle

(
DPoint3dP pInstance,
DPoint2dCP pPoint,
DPoint2dCP pVertex0,
DPoint2dCP pVertex1,
DPoint2dCP pVertex2
)
    {
    DPoint2d        q, v1, v2;

    q.x  = pPoint->x   - pVertex0->x;
    q.y  = pPoint->y   - pVertex0->y;
    v1.x = pVertex1->x - pVertex0->x;
    v1.y = pVertex1->y - pVertex0->y;
    v2.x = pVertex2->x - pVertex0->x;
    v2.y = pVertex2->y - pVertex0->y;

    return bsiDPoint3d_barycentricFromDPoint2dTriangleVectors
            (pInstance, NULL, &q, NULL, &v1, &v2);
    }

/*-----------------------------------------------------------------*//**
* @description Sets this instance to the barycentric coordinates of pPoint
* relative to the triangle (pVertex0, pVertex1, pVertex2) in the xy-plane.
*
* @instance pInstance   <= barycentric coordinates of pPoint relative to T
* @param pPoint         => point in plane
* @param pVertex0       => vertex 0 of triangle T
* @param pVertex1       => vertex 1 of triangle T
* @param pVertex2       => vertex 2 of triangle T
* @see bsiDPoint3d_barycentricFromDPoint2dTriangleVectors
* @see bsiDPoint2d_fromBarycentricAndDPoint2dTriangle
* @group "DPoint3d Barycentric"
* @return true if and only if the area of T is sufficiently large.
* @bsihdr                                                                       DavidAssaf      10/98
+---------------+---------------+---------------+---------------+------*/
Public  GEOMDLLIMPEXP bool    bsiDPoint3d_barycentricFromDPoint2dTriangle
(
DPoint3dR uvw,
DPoint3dR dUVWdX,
DPoint3dR dUVWdY,
double   &area,
DPoint2dCR point,
DPoint2dCR vertex0,
DPoint2dCR vertex1,
DPoint2dCR vertex2
)
    {
    DPoint2d        q, v1, v2;

    q.x  = point.x   - vertex0.x;
    q.y  = point.y   - vertex0.y;
    v1.x = vertex1.x - vertex0.x;
    v1.y = vertex1.y - vertex0.y;
    v2.x = vertex2.x - vertex0.x;
    v2.y = vertex2.y - vertex0.y;
    double divArea;
    if (bsiDPoint3d_barycentricFromDPoint2dTriangleVectors
            (&uvw, &area, &q, NULL, &v1, &v2)
        && DoubleOps::SafeDivide (divArea, 2.0, area, 0.0))
        {
        
        DVec2d edgeVector[3];
        edgeVector[0].DifferenceOf (vertex2, vertex1);  // vector opposite point0
        edgeVector[1].DifferenceOf (vertex0, vertex2);  // vector opposite point1
        edgeVector[2].DifferenceOf (vertex1, vertex0);  // vector opposite point2
        dUVWdX.Init (-edgeVector[0].y * divArea, -edgeVector[1].y * divArea, -edgeVector[2].y * divArea);
        dUVWdY.Init ( edgeVector[0].x * divArea,  edgeVector[1].x * divArea, edgeVector[2].x * divArea);
        return true;
        }
    uvw.Zero ();
    dUVWdX.Zero ();
    dUVWdY.Zero ();
    area = 0.0;
    return false;
    }

/*-----------------------------------------------------------------*//**
* Given a space point spacePontP, finds the closest point on the plane
* containing the 3 points in pPlanePoint.  Stores the closest point
* coordinates in pClosePoint, and the s and t coordinates (as defined
* in bsiGeom_evaluateSkewedPlane) in sP and tP.
*
* @param pClosePoint <= point on plane.  May be null pointer
* @param sP <= parametric coordinate on s axis
* @param tP <= parametric coordinate on t axis
* @param pPlanePoint => origin, s=1, and t=1 points
* @param pSpacePoint => point to be projected
* @return true unless the plane points are collinear
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public  bool     bsiGeom_closestPointOnSkewedPlane

(
DPoint3dP pClosePoint,
double          *sP,
double          *tP,
DPoint3dCP pPlanePoint,
DPoint3dCP pSpacePoint
)
    {
    double          s,t;
    double          dotUU, dotUV, dotVV, dotUQ, dotVQ;
    bool            result = true;
    DPoint3d        vectorU, vectorV, vectorQ;

    vectorU.DifferenceOf (pPlanePoint[1], pPlanePoint[0]);
    vectorV.DifferenceOf (pPlanePoint[2], pPlanePoint[0]);
    vectorQ.DifferenceOf (*pSpacePoint, pPlanePoint[0]);

    dotUU = vectorU.DotProduct (vectorU);
    dotVV = vectorV.DotProduct (vectorV);
    dotUV = vectorU.DotProduct (vectorV);
    dotUQ = vectorU.DotProduct (vectorQ);
    dotVQ = vectorV.DotProduct (vectorQ);

    if (!bsiSVD_solve2x2 (&s, &t, dotUU, dotUV, dotUV, dotVV, dotUQ, dotVQ))
        {
        result = false;
        }

    else

        {
        if (sP)
            *sP = s;
        if (tP)
            *tP = t;

        if (pClosePoint)
            pClosePoint->SumOf(pPlanePoint[0], vectorU, s, vectorV, t);
        }

    return  result;
    }

/*-----------------------------------------------------------------*//**
* @description Compute the minimum distance from a point to a triangle.
* @instance pSpacePoint   <= point in space
* @param pVertex0       => vertex of T
* @param pVertex1       => vertex of T
* @param pVertex2       => vertex of T
* @param pClosePoint    <= projection of space point onto plane of triangle
* @param pBCoords       <= barycentric coordinates of closest point
* @return minimum distance
* @group "DPoint3d Barycentric"
* @bsihdr                                       EarlinLutz      10/04
+---------------+---------------+---------------+---------------+------*/
Public  double bsiDPoint3d_minDistToTriangle

(
DPoint3dCP pSpacePoint,
DPoint3dCP pVertex0,
DPoint3dCP pVertex1,
DPoint3dCP pVertex2,
DPoint3dP pClosePoint,
DPoint3dP pBoundedUVW,
DPoint3dP pUnboundedUVW
)
    {
    DPoint3d planePoint;
    double s, t;
    DPoint3d xyz[3];
    static DPoint3d uvwCorner[3] =
        {
            {1,0,0},
            {0,1,0},
            {0,0,1}
        };

    /* Ugh.  Compute for each edge independently.  */
    DSegment3d testSeg[3];
    DPoint3d   testPoint[3];
    double     testParam[3];
    double     testDistanceSquared[3];

    int i, iMin, jMin;
    xyz[0] = *pVertex0;
    xyz[1] = *pVertex1;
    xyz[2] = *pVertex2;
    if (pUnboundedUVW)
        pUnboundedUVW->Zero ();
    
    if (    bsiGeom_closestPointOnSkewedPlane (&planePoint, &s, &t, xyz, pSpacePoint))
        {
        if (pUnboundedUVW)
            pUnboundedUVW->Init ( 1.0 - s - t, s, t);
        if (s >= 0.0
        &&  t >= 0.0
        &&  s + t <= 1.0)
            {
            if (pBoundedUVW)
                pBoundedUVW->Init ( 1.0 - s - t, s, t);
            if (pClosePoint)
                *pClosePoint = planePoint;
            return planePoint.Distance (*pSpacePoint);
            }
        }

    testSeg[0].Init (*pVertex0, *pVertex1);
    testSeg[1].Init (*pVertex1, *pVertex2);
    testSeg[2].Init (*pVertex2, *pVertex0);

    for (i = 0; i < 3; i++)
        {
        testSeg[i].ProjectPointBounded (testPoint[i], testParam[i], *pSpacePoint);
        testDistanceSquared[i] = testPoint[i].DistanceSquared (*pSpacePoint);
        }
    iMin = 0;
    if (testDistanceSquared[1] < testDistanceSquared[0])
        iMin = 1;
    if (testDistanceSquared[2] < testDistanceSquared[iMin])
        iMin = 2;

    jMin = iMin + 1;
    if (jMin == 3)
        jMin = 0;

    if (pClosePoint)
        *pClosePoint = testPoint[iMin];
    if (pBoundedUVW)
        pBoundedUVW->Interpolate (uvwCorner[iMin], testParam[iMin], uvwCorner[jMin]);
    return sqrt (testDistanceSquared[iMin]);
    }

/*-----------------------------------------------------------------*//**
* @description Applies transformation to simplify the problem of finding the barycentric
* coordinates of a 3d point relative to a triangle.  Returned are the
* components of the new 2d problem.
*
* @instance pInstance   => point to find barycentric coords of
* @param pNewPoint      <= point in plane of new triangle with same barycoords
* @param pNewVector1    <= side of new triangle
* @param pNewVector2    <= side of new triangle
* @param pVector1       => side of old triangle
* @param pVector2       => side of old triangle
* @group "DPoint3d Barycentric"
* @bsihdr                                                                       DavidAssaf      10/98
+---------------+---------------+---------------+---------------+------*/
Public void transformBarycentric3dSystemTo2d

(
DPoint3dCP pInstance,
DPoint2dP pNewPoint,
DPoint2dP pNewVector1,
DPoint2dP pNewVector2,
DPoint3dCP pVector1,
DPoint3dCP pVector2
)
    {
    /*
    Projecting the 3D point q onto the plane spanned by 3D vectors v1, v2
    (which form triangle T) is a least squares problem:

      [v1 v2] [b1 b2]^ = q

    The resulting normal equations (below) determine the barycoords b1, b2
    (corresponding to v1, v2) of the projection relative to T:
      -              - -    -   -      -
      | v1.v1  v1.v2 | | b1 | = | q.v1 |
      | v1.v2  v2.v2 | | b2 |   | q.v2 |
      -              - -    -   -      -

    or equivalently, [newVector1 newVector2] [b1 b2]^ = newPoint.

    This latter form shows that the 3D problem reduces to a 2D problem:
    finding the barycentric coordinates of newPoint relative to the
    triangle in the xy-plane spanned by vectors newVector1, newVector2.
    */
    pNewVector1->x =    pVector1->x * pVector1->x +
                        pVector1->y * pVector1->y +
                        pVector1->z * pVector1->z;
    pNewVector1->y =
    pNewVector2->x =    pVector1->x * pVector2->x +
                        pVector1->y * pVector2->y +
                        pVector1->z * pVector2->z;

    pNewVector2->y =    pVector2->x * pVector2->x +
                        pVector2->y * pVector2->y +
                        pVector2->z * pVector2->z;

    pNewPoint->x =     pInstance->x * pVector1->x +
                       pInstance->y * pVector1->y +
                       pInstance->z * pVector1->z;

    pNewPoint->y =     pInstance->x * pVector2->x +
                       pInstance->y * pVector2->y +
                       pInstance->z * pVector2->z;
    }



/*-----------------------------------------------------------------*//**
* @description Sets this instance to the barycentric coordinates of pPoint
* relative to the triangle (pVertex0, pVertex1, pVertex2) in space.
* Points p and r in space have the same barycentric coordinates relative to
* T if and only if they project to the same point q in the plane of T;
* then their barycentric coordinates relative to T are those of q.
*
* @instance pInstance   <= barycentric coordinates of pPoint relative to T
* @param pPoint         => point in space
* @param pVertex0       => vertex of triangle T
* @param pVertex1       => vertex of triangle T
* @param pVertex2       => vertex of triangle T
* @see bsiDPoint3d_barycentricFromDPoint2dTriangle
* @see bsiDPoint3d_fromBarycentricAndDPoint3dTriangle
* @return true if and only if the area of T is sufficiently large.
* @group "DPoint3d Barycentric"
* @bsihdr                                                                       DavidAssaf      10/98
+---------------+---------------+---------------+---------------+------*/
Public  bool    bsiDPoint3d_barycentricFromDPoint3dTriangle

(
DPoint3dP pInstance,
DPoint3dCP pPoint,
DPoint3dCP pVertex0,
DPoint3dCP pVertex1,
DPoint3dCP pVertex2
)
    {
    DPoint3d        q, v1, v2;
    DPoint2d        newPoint, newV1, newV2;

    /*
    Translating by pVertex0 compresses the triangle definition from 3 points
    to 2 vectors while preserving barycentric coords.
    */
    q.x  = pPoint->x   - pVertex0->x;
    q.y  = pPoint->y   - pVertex0->y;
    q.z  = pPoint->z   - pVertex0->z;
    v1.x = pVertex1->x - pVertex0->x;
    v1.y = pVertex1->y - pVertex0->y;
    v1.z = pVertex1->z - pVertex0->z;
    v2.x = pVertex2->x - pVertex0->x;
    v2.y = pVertex2->y - pVertex0->y;
    v2.z = pVertex2->z - pVertex0->z;

    /* decrement dimension of problem */
    transformBarycentric3dSystemTo2d (&q, &newPoint, &newV1, &newV2, &v1, &v2);

    return bsiDPoint3d_barycentricFromDPoint2dTriangleVectors
            (pInstance, &newPoint, NULL, &newV1, &newV2);
    }
/*-----------------------------------------------------------------*//**
* @description Sets this instance to the barycentric coordinates of pPoint
* relative to the triangle (pOrigin, pVector1-pOrigin, pVector2-pOrigin)
* in the xy-plane.
*
* @instance pInstance   <= barycentric coordinates of pPoint relative to T
* @param pArea          <= area of triangle.
* @param pPoint         => point in plane
* @param pOrigin        => vertex of triangle T (may be null for origin)
* @param pVector1       => side vector of T (emanating from pOrigin)
* @param pVector2       => side vector of T (emanating from pOrigin)
* @see bsiDPoint3d_barycentricFromDPoint2dTriangle
* @see bsiDPoint2d_fromBarycentricAndDPoint2dTriangleVectors
* @group "DPoint3d Barycentric"
* @return true if and only if the area of T is sufficiently large.
* @bsihdr                                      DavidAssaf      10/98
+---------------+---------------+---------------+---------------+------*/
Public  bool    bsiDPoint3d_barycentricFromDPoint2dTriangleVectors

(
DPoint3dP pInstance,
DPoint2dCP pPoint,
DPoint2dCP pOrigin,
DPoint2dCP pVector1,
DPoint2dCP pVector2
)
    {
    return bsiDPoint3d_barycentricFromDPoint2dTriangleVectors (pInstance, NULL, pPoint, pOrigin,
                    pVector1, pVector2);
    }
/*-----------------------------------------------------------------*//**
* @description Sets this instance to the point in the plane with the given barycentric
* coordinates relative to triangle T (pVertex0, pVertex1, pVertex2).
*
* @instance pInstance   <= point with given barycoords relative to T
* @param pBaryCoords    => barycentric coordinates relative to T
* @param pVertex0       => vertex 0 of triangle T
* @param pVertex1       => vertex 1 of triangle T
* @param pVertex2       => vertex 2 of triangle T
* @see bsiDPoint3d_barycentricFromDPoint2dTriangle
* @group "DPoint2d Barycentric"
* @bsihdr                                                               DavidAssaf      10/98
+---------------+---------------+---------------+---------------+------*/
Public  void bsiDPoint2d_fromBarycentricAndDPoint2dTriangle

(
DPoint2dP pInstance,
DPoint3dCP pBaryCoords,
DPoint2dCP pVertex0,
DPoint2dCP pVertex1,
DPoint2dCP pVertex2
)
    {
    pInstance->x =  pBaryCoords->x * pVertex0->x +
                    pBaryCoords->y * pVertex1->x +
                    pBaryCoords->z * pVertex2->x;
    pInstance->y =  pBaryCoords->x * pVertex0->y +
                    pBaryCoords->y * pVertex1->y +
                    pBaryCoords->z * pVertex2->y;
    }

/*-----------------------------------------------------------------*//**
* @description Dot the plane normal with the vector from the plane origin to the point.
* @remarks If the plane normal is a unit vector, this is the true distance from the
*       plane to the point.  If not, it is a scaled distance.
* @param pPlane => plane to evaluate
* @param pPoint => point for evaluation
* @return dot product
* @group "DPlane3d Projection"
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public  double  bsiDPlane3d_evaluate

(
DPlane3dCP pPlane,
DPoint3dCP pPoint
)
    {
    return pPoint->DotDifference(pPlane->origin, pPlane->normal);
    }
/*---------------------------------------------------------------------------------**//**
*
* Add a point to the array.
*
* @param    pInstance <=> header to receive new point
* @param pPoint => point being added.
* @return true unless rubber array could not be extended.
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public  bool     jmdlGraphicsPointArray_addDPoint3d
(
GraphicsPointArrayP pInstance,
const DPoint3d              *pPoint
)
    {
    GraphicsPoint  element;
    bsiGraphicsPoint_init (&element,
                    pPoint->x, pPoint->y, pPoint->z, 1.0,
                    0.0, HPOINT_NORMAL, 0);
    pInstance->vbArray_hdr.push_back (element);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* Mark the break between disconnected line segments.
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public  void jmdlGraphicsPointArray_markBreak
(
GraphicsPointArrayP         pInstance
)
    {
    if (pInstance && pInstance->vbArray_hdr.size () > 0)
        pInstance->vbArray_hdr.back ().mask |= HPOINT_MASK_BREAK;
    }

/*-----------------------------------------------------------------*//**
*
* Clear the range set to an empty set. (No intervals)
*
* @see
* @indexVerb
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiRange1d_clear
(
SmallSetRange1dP rangeSetP
)

    {
    rangeSetP->n = 0;
    }
/*-----------------------------------------------------------------*//**
*
* Set an arc sweep as the only entry in a small range set.
* All prior entries in the set are ignored.  The sweep is inserted
* as is, with no attempt to normalize direction or range.
* Caveat emptor.
*
* @param theta0 => Start of interval.
* @param dtheta => Angular sweep.  May be negative.
* @see #addArcSweep
* @see #setArcSweep
* @indexVerb
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiRange1d_setUncheckedArcSweep

(
SmallSetRange1dP arcSetP,
double          theta0,
double          dtheta
)


    {
    double theta1 = theta0 + dtheta;

    arcSetP->interval[0].minValue = theta0;
    arcSetP->interval[0].maxValue = theta1;
    arcSetP->n = 1;
    }

/*-----------------------------------------------------------------*//**
*
* Set an arc sweep as the only entry in a small range set.
* All prior entries in the set are ignored.  The arc sweep is normalized
* and split as needed.
*
* @param theta0 => start of interval.
* @param dtheta => angular sweep.  May be negative.
* @see #addArcSweep
* @see #setUncheckedArcSweep
* @indexVerb
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiRange1d_setArcSweep

(
SmallSetRange1dP arcSetP,
double          theta0,
double          dtheta
)


    {
    double theta1;

    if (dtheta < 0.0)
        {
        theta0 = theta0 + dtheta;
        dtheta = -dtheta;
        }

    if (dtheta >= msGeomConst_2pi)
        {
        arcSetP->interval[0].minValue = -msGeomConst_pi;
        arcSetP->interval[0].maxValue =  msGeomConst_pi;
        arcSetP->n = 1;
        }
    else
        {
        theta1 = theta0 + dtheta;

        if (theta1 <= msGeomConst_pi)
            {
            arcSetP->interval[0].minValue = theta0;
            arcSetP->interval[0].maxValue = theta1;
            arcSetP->n = 1;
            }
        else
            {
            theta1 -= msGeomConst_2pi;
            arcSetP->interval[0].minValue = -msGeomConst_pi;
            arcSetP->interval[0].maxValue =  theta1;
            arcSetP->interval[1].minValue = theta0;
            arcSetP->interval[1].maxValue = msGeomConst_pi;
            arcSetP->n = 2;
            }
        }
    }


/*-----------------------------------------------------------------*//**
* Get the i'th sector angular range
* @instance pEllipse => ellipse whose angular range is queried.
* @param pStartAngle <= start angle
* @param pEndAngle <= end angle
* @param i => sector to read
* @see
* @return true if sector index is valid.
* @indexVerb
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public  bool    bsiDEllipse4d_getSector

(
DEllipse4dCP pEllipse,
double    *pStartAngle,
double    *pEndAngle,
int             i
)
    {
    bool    boolStat = false;
    if (pEllipse && 0 <= i && i < pEllipse->sectors.n)
        {
        *pStartAngle = pEllipse->sectors.interval[i].minValue;
        *pEndAngle = pEllipse->sectors.interval[i].maxValue;
        boolStat = true;
        }
    else
        {
        *pStartAngle = *pEndAngle = 0.0;
        }
    return boolStat;
    }

/*-----------------------------------------------------------------*//**
* Compute the transfer matrix to normalize a weighted, uncentered
* ellipse into a centered cartesian ellipse.

* @param pMatrix <= transfer matrix
* @param pInverse <= its inverse.   Pass NULL if not needed.
* @param w0 => cosine weight
* @param w90 => sine weight
* @param wCenter => center weight
* @see
* @return true if weights define an angle change.
* @indexVerb
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public  bool    bsiEllipse_angularTransferMatrix

(
RotMatrixP pMatrix,
RotMatrixP pInverse,
double          w0,
double          w90,
double          wCenter
)
    {
    double mu;
    double wc2;
    double ww2;
    static double relTol = 1.0e-14;
    bool    boolStat = true;

    wc2 = wCenter * wCenter;
    ww2 = w0 * w0 + w90 * w90;
    mu = wc2 - ww2;
    if ( mu <= relTol * wc2)
        {
        /* It's a hyperbola or parabola*/
        pMatrix->InitIdentity ();
        if (pInverse)
            pInverse->InitIdentity ();
        return false;
        }
    else if (mu == wc2)     /* Yes, exact equality test -- if wc2 is small the squaring will */
                            /* wipe force its bits so far to the right they have no effect on the subtraction*/
        {
        /* It's already practically a circle.*/
        pMatrix->InitIdentity ();
        if (pInverse)
            pInverse->InitIdentity ();
        }
    else
        {
        double divW = 1.0 / sqrt (ww2);
        double bx = w0  * divW;
        double by = w90 * divW;
        double gamma = wc2 / mu;
        double rootGamma = sqrt (gamma);
        double centerScale = gamma / wCenter;

        pMatrix->SetColumn (0, gamma * bx, gamma * by, 0.0);
        pMatrix->SetColumn (1, rootGamma * -by, rootGamma * bx, 0.0);
        pMatrix->SetColumn (2, - centerScale * w0, - centerScale * w90, 1.0);
        if (pInverse)
            {
            pInverse->InverseOf (*pMatrix);
            }
        }
    return boolStat;
    }

/*-----------------------------------------------------------------*//**
* Let F = [cos(theta), sin(theta), 1+alpha*cos(theta)+beta*sin(theta)]
*     G = [cos(phi), sin(phi), 1]
* and G = M*F   (possibly scaled?)
* Return the phi corresponding to theta.

* @param theta => known angle prior to transform
* @param pMatrix => transfer matrix.
* @param  => matrix M
* @param alpha => cosine coefficient
* @param beta => sine coefficient
* @see
* @return modified angle
* @indexVerb
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public  double bsiDEllipse4d_transferAngle

(
double          theta,
RotMatrixP pMatrix,

double          alpha,
double          beta
)
    {
    DPoint3d G;
    double cosTheta = cos(theta);
    double sinTheta = sin(theta);
    double wF       = 1.0 + alpha * cosTheta + beta * sinTheta;
    double phi;
    pMatrix->MultiplyComponents(G, cosTheta, sinTheta, wF);
    phi = atan2 (G.y, G.x);
    return phi;
    }

/*-----------------------------------------------------------------*//**
* Let F = [cos(theta), sin(theta), 1+alpha*cos(theta)+beta*sin(theta)]
*     G = [cos(phi), sin(phi), 1]
* and G = M*F   (possibly scaled?)
* Replace all angles (theta) in an ellispe's stroke intervals by
* corresponding phi angles.

* @param pDest <=> Ellipse whose angles are corrected.
* @param pSource => source of angle data
* @param pMatrix => matrix M
* @param alpha => cosine coefficient
* @param beta => sine coefficient
* @see
* @indexVerb
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public  bool    bsiDEllipse4d_transferAngles

(
DEllipse4dP pDest,
DEllipse4dCP pSource,
RotMatrixP pMatrix,
double          alpha,
double          beta
)
    {
    int i;
    double phi0, phi1, theta0, theta1, thetaMid, phiMid, dPhi;
    SmallSetRange1d sectors;

    sectors = pSource->sectors;
    bsiRange1d_clear (&pDest->sectors);
    for (i = 0; i < sectors.n; i++)
        {
        theta0 = sectors.interval[i].minValue;
        theta1 = sectors.interval[i].maxValue;
        if (Angle::IsFullCircle (theta1 - theta0))
            {
            bsiRange1d_setArcSweep (&pDest->sectors, 0.0, msGeomConst_2pi);
            return true;
            }
        else
            {
            thetaMid = 0.5 * (theta1 + theta0);
            phi0 = bsiDEllipse4d_transferAngle (theta0, pMatrix, alpha, beta);
            phi1 = bsiDEllipse4d_transferAngle (theta1, pMatrix, alpha, beta);
            dPhi = phi1 - phi0;
            phiMid = bsiDEllipse4d_transferAngle (thetaMid, pMatrix, alpha, beta);

            if ((phiMid - phi0) * (phi1 - phiMid) < 0.0)
                {
                if (dPhi > 0.0)
                    {
                    phi1 -= msGeomConst_2pi;
                    }
                else
                    {
                    phi1 +=  msGeomConst_2pi;
                    }
                }
            bsiRange1d_addUnordered (&pDest->sectors, phi0, phi1);
                }
        }
    return true;
    }

/*-----------------------------------------------------------------*//**
*
* Add an interval with no test for min/max relationship
*
* @param minValue => new interval min.
* @param maxValue => new interval max
* @see #addArcSweep
* @see #setArcSweep
* @see #setUncheckedArcSweep
* @indexVerb
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void      bsiRange1d_addUnordered

(
SmallSetRange1dP setP,
double          minValue,
double          maxValue
)


    {
    int i = setP->n;
    if (i >= MSGEOM_SMALL_SET_SIZE)
        {
        /* ignore the overflow */
        }
    else
        {
        setP->interval[i].minValue = minValue;
        setP->interval[i].maxValue = maxValue;
        setP->n++;
        }
    }



/*-----------------------------------------------------------------*//**
*
* Find new basis vectors with 0 weights on the U and V vectors, and unit
* on the C vector.  This computation is not possible if the curve is
* a hyperbola or parabola when projected to 3D.
*
* @instance pNormalized <= normalized form
* @param pWeighted => unnormalized form
* @see
* @return true if the curve really is an ellipse (i.e. not hyperbola or parabola)
* @indexVerb
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public  bool    bsiDEllipse4d_normalizeWeights

(
DEllipse4dP pNormalized,
DEllipse4dCP pWeighted
)

    {
    double w0 = pWeighted->vector0.w;
    double w90 = pWeighted->vector90.w;
    double wCenter = pWeighted->center.w;
    RotMatrix   transferMatrix;
    RotMatrix   inverseTransferMatrix;
    bool    boolStat = bsiEllipse_angularTransferMatrix(
                                        &transferMatrix,
                                        &inverseTransferMatrix,
                                        w0,
                                        w90,
                                        wCenter
                                        );
    DPoint4d    vector0, vector90, center;

    if (boolStat)
        {
        double recip = 1.0 / wCenter;
        double alpha = w0 * recip;
        double beta  = w90 * recip;

        vector0.SumOf(pWeighted->vector0, recip, pWeighted->center, - alpha * recip);
        vector90.SumOf(pWeighted->vector90, recip, pWeighted->center, - beta * recip);
        center.Scale (pWeighted->center, recip);

        /* The transfer matrix is of the form*/
        /*          [ rxx  rxy  cx]*/
        /*          [ ryx  ryy  cy]*/
        /*          [  0    0    1]*/
        pNormalized->vector0.SumOf(vector0, transferMatrix.form3d[0][0], vector90, transferMatrix.form3d[1][0]);
        pNormalized->vector90.SumOf(vector0, transferMatrix.form3d[0][1], vector90, transferMatrix.form3d[1][1]);
        pNormalized->center.SumOf(center, vector0, transferMatrix.form3d[0][2], vector90, transferMatrix.form3d[1][2]);
        bsiDEllipse4d_transferAngles( pNormalized, pWeighted, &inverseTransferMatrix, w0 / wCenter, w90 / wCenter);
        }
    else
        {
        *pNormalized = *pWeighted;
        }
    return boolStat;
    }
/*-----------------------------------------------------------------*//**
* Computes the silhouette ellipse of an ellipsoid under arbitrary
* DMap4d and viewpoint.
*
* @param pHEllipse <= silhouette ellipse/parabola/hyperbola
* @param pEllipsoidPoint => 4 defining points of the ellipsoid
* @param pHMap => further mapping
* @param pEyePoint => eyepoint
* @return false iff the eyeponit is inside the ellipsoid.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public  bool    bsiGeom_ellipsoidSilhouette

(
DEllipse4dP pHEllipse,
DPoint3dCP pEllipsoidPoint,
DMap4dCP pHMap,
DPoint4dCP pEyePoint
)
    {
    Transform axisTransform; /* basis matrix defined by 4 ellipsoid points*/
    DMap4d axisMap;       /* Homogeneous form of basis matrix*/
    DMap4d BMap;                /* pHMap * axisMap -- the 'full' B matrix for the ellipsoid.*/
    bool    result = false;

    if (pEllipsoidPoint)
        {
        /* In its local coordinate system, the ellipsoid is a unit sphere centered at
            the origin.  Construct this mapping and concatentate it with the given DMap4d
            to give the full transformation of the 'ellipsoid' (which may go to infinity
            if the weight vanishes)
        */
        axisTransform.InitFrom4Points(pEllipsoidPoint[0], pEllipsoidPoint[1], pEllipsoidPoint[2], pEllipsoidPoint[3]);
        result = bsiDMap4d_initFromTransform (&axisMap, &axisTransform, false);

        if (pHMap)
            bsiDMap4d_multiply (&BMap, pHMap, &axisMap);
        else
            BMap = axisMap;
        }
    else
        {
        if (pHMap)
            BMap = *pHMap;
        else
            bsiDMap4d_initIdentity (&BMap);
        result = true;
        }


    if (result)
        {
        DPoint4d localEye;
        DMatrix4d Q, QT, BQT;
        double mag;

        static int indexVector[3] = {2, 0, 1};

        result = false;


        bsiDMatrix4d_multiply4dPoints (&BMap.M1, &localEye, pEyePoint, 1);
        /* Characteristic matrix for sphere negates the w component to turn the eye 'point'*/
        /* into an eye 'plane'.*/
        localEye.w = - localEye.w;

        /* Build orthgonal transformation which rotates z towards the eye.  After this*/
        /* rotation, the silhouette is a constant-z curve.*/
        if (bsiDMatrix3d_initSelectiveRotation (
                        (double*)&Q, &mag, 4, (double *)&localEye, indexVector, 3)
            && fabs (localEye.w) < mag
           )
            {
            DPoint4d vectorW;
            double sineThetaHat   = - localEye.w / mag;
            double cosineThetaHat = sqrt (1.0 - sineThetaHat * sineThetaHat);

            QT.TransposeOf (Q);
            BQT.InitProduct (BMap.M0, QT);

            BQT.GetColumn (pHEllipse->vector0, 0);
            BQT.GetColumn (pHEllipse->vector90, 1);
            BQT.GetColumn (vectorW, 2);
            BQT.GetColumn (pHEllipse->center, 3);
            pHEllipse->vector0.Scale (pHEllipse->vector0, cosineThetaHat);
            pHEllipse->vector90.Scale (pHEllipse->vector90, cosineThetaHat);
            pHEllipse->center.SumOf(pHEllipse->center, vectorW, sineThetaHat);
            bsiRange1d_setArcSweep( &pHEllipse->sectors, 0.0, msGeomConst_2pi );

            result = true;
            }
        }

    return result;
    }


/*-----------------------------------------------------------------*//**
@nodoc DEllipse4d
@description Convert a homogeneous ellipse to cartesian.  Callers should beware of the following
 significant points:
<UL>
<LI>A homogeneous "ellipse" may appear as a hyperbola or parabola in xyz space.
   Hence the conversion can fail.
<LI>When the conversion succeeds, it is still a Very Bad Thing To Do numerically
   because a homogeneous ellipse with "nice" numbers can have very large center and axis
   coordinates.   It is always preferable to do calculations directly on the homogeneous
   ellipse if possible.
<LI>When the conversion succeeds, the axis may be non-perpendicular.  A subsequent call
   may be made to initWithPerpendicularAxes to correct this.
</UL>
 @param pEllipse <= initialized ellipse
 @param pSource => homogeneous ellipse
 @param sector  => angular sector index.  If out of bounds, a full ellipse is created.
 @return true if homogeneous parts allow reduction to simple ellipse. (false if the homogeneous
    parts are a parabola or hyperbola.)
 @group "DEllipse3d Initialization"
 @bsimethod                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public  bool     bsiDEllipse3d_initFromDEllipse4d

(
DEllipse3dP pEllipse,
DEllipse4dCP pSource,
int           sector
)
    {
    DEllipse4d  normalizedSource;
    double      theta0, theta1;
    bool        funcStat = false;
    /* Try to eliminate the weights on the vectors of the source ellipse */
    if (bsiDEllipse4d_normalizeWeights (&normalizedSource, pSource))
        {
        funcStat = true;
        pEllipse->center.XyzOf (normalizedSource.center);
        pEllipse->vector0.XyzOf (normalizedSource.vector0);
        pEllipse->vector90.XyzOf (normalizedSource.vector90);

        if (bsiDEllipse4d_getSector (&normalizedSource, &theta0, &theta1, sector))
            {
            pEllipse->start = theta0;
            pEllipse->sweep = theta1 - theta0;
            }
        else
            {
            pEllipse->start = 0.0;
            pEllipse->sweep = msGeomConst_2pi;
            }
        }
    return funcStat;
    }

/*-----------------------------------------------------------------*//**
* This routine will find the intersection between a general conic
* and a unit circle. The conic is in the form of:
* x = centerx + ux * cos(theta) + vx*sin(theta)
* y = centery + uy * cos(theta) + vy*sin(theta)
* w = centerw + uw * cos(theta) + vw*sin(theta)
*   where centerx, centery, centerw, ux, uy, uw, vx, vy, vw are constants
*   and    PI < = theta < = PI
* A unit circle is x^2 + Y^2 = 1
* Return values: number of solutions found.
*               0: no intersection
*               -1: input error or polynomial solver failed.
*
* @param pCosValue <= 0 to 4 cosine values
* @param pSinValue <= 0 to 4 sine values
* @param pThetaValue <= 0 to 4 angle values
* @param pNumInt <= number of intersections
* @param centerx
* @param ux
* @param vx
* @param centery
* @param uy
* @param vy
* @param cenerw
* @param uw
* @param vw
* @return -1 if the conic is (exactly) a unit circle,
*               else number of intersections.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public  int bsiMath_conicIntersectUnitCircle

(
double          *pCosValue,
double          *pSinValue,
double          *pThetaValue,
int             *pNumInt,
double          centerx,
double          ux,
double          vx,
double          centery,
double          uy,
double          vy,
double          centerw,
double          uw,
double          vw
)
    {
    RotMatrix B;
    B.form3d[0][0] = ux;
    B.form3d[1][0] = uy;
    B.form3d[2][0] = uw;

    B.form3d[0][1] = vx;
    B.form3d[1][1] = vy;
    B.form3d[2][1] = vw;

    B.form3d[0][2] = centerx;
    B.form3d[1][2] = centery;
    B.form3d[2][2] = centerw;
    return bsiBezier_conicIntersectUnitCircle (pCosValue, pSinValue, pThetaValue, pNumInt,
                                    NULL, NULL, &B);
    }


/*-----------------------------------------------------------------*//**
* Compute B so X'BX = X'AX and B is symmetric.

* @param pA <= symmetric coefficients
* @param pB => nonsymmetric coefficients
* @see
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public  void bsiQCoff_symmetrize

(
RotMatrixP pA,
RotMatrixCP pB
)
    {
    double bxy = 0.5 * (pB->form3d[0][1] + pB->form3d[1][0]);
    double bxz = 0.5 * (pB->form3d[0][2] + pB->form3d[2][0]);
    double byz = 0.5 * (pB->form3d[2][1] + pB->form3d[1][2]);

    pA->form3d[0][0] = pB->form3d[0][0];
    pA->form3d[1][1] = pB->form3d[1][1];
    pA->form3d[2][2] = pB->form3d[2][2];

    pA->form3d[0][1] = pA->form3d[1][0] = bxy;
    pA->form3d[0][2] = pA->form3d[2][0] = bxz;
    pA->form3d[1][2] = pA->form3d[2][1] = byz;
    }




/*-----------------------------------------------------------------*//**
* Compute a matrix A such that
*   A*(c s 1)' = H * B where
*  H is the matrix
* [ 0 -1 -s][bx]        [c]
* [ 1  0  c][by] == A * [s]
* [ s -c  0][bz]        [1]

* @param pA <= coefficient matrix
* @param pVecB => vector
* @see
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public  void bsiQCoff_HOperator

(
RotMatrixP   pA,
DPoint3dCP pVecB
)
    {
    pA->InitFromColumnVectors (
            DVec3d::From (0.0,      pVecB->z,   -pVecB->y),
            DVec3d::From (-pVecB->z,     0.0,    pVecB->x),
            DVec3d::From (-pVecB->y,   pVecB->x,     0.0)
            );
    }




/*-----------------------------------------------------------------*//**
* Compute the matrix of a quadric section whose intersections with
* the unit circle are the cosine and sine of the angles where pPoint
* projects to the quadric.
* That is,
*   A = sym(D*B'* (I - QW') * B)
* Where sym is the symmetrizing operator and  B, D, Q, and W are things
* that need some explanation.

* @param pA <= matrix of new quadric section
* @param pB => matrix of existing quadric section
* @param pPoint => point being projected to matrix B.
* @param pPoint
* @see
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public  void bsiQCoff_projectToEllipse

(
RotMatrixP pA,
RotMatrixCP pB,
DPoint3dCP pPoint
)
    {
    RotMatrix D, DT;
    RotMatrix E;
    DVec3d BtW;
    RotMatrix BT;
    RotMatrix product;
    BT.TransposeOf (*pB);

    BtW = DVec3d::FromColumn (BT, 2);

    bsiQCoff_HOperator (&D, &BtW);
    DT.TransposeOf (D);
    E.InitIdentity ();

    DVec3d col2 = DVec3d::FromColumn (E, 2);
    col2.DifferenceOf (col2, *pPoint);
    E.SetColumn (col2, 2);

    product.InitProduct (DT, BT);
    product.InitProduct (product, E);
    product.InitProduct (product, *pB);
    bsiQCoff_symmetrize (pA, &product);
    }

/*-----------------------------------------------------------------*//**
* This routine finds the points of intersection between an implicit
* conic (specified by matrix A) X^AX = 0  and the unit circle
* x^2 + Y^2 = 1
* Returns  : number of intersections found.
*            -1: conic = circle or input error or polynomial solver failed.
*
* @param pCosValue <= x coordinates of intersections
* @param pSinValue <= y coordinates of intersections
* @param pThetaValue <= angular positions of intersections
* @param pNumInt <= number of intersections
* @param pCoefficientMatrix => matrix defining implicit conic
* @return 0 if success, nonzero if error
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public  StatusInt bsiMath_implicitConicIntersectUnitCircle

(
double          *pCosValue,
double    *pSinValue,
double    *pThetaValue,
int       *pNumInt,
RotMatrixCP pCoefficientMatrix
)
    {
    return 0 == bsiBezier_implicitConicIntersectUnitCircle
                (pCosValue, pSinValue, pThetaValue, pNumInt, NULL, NULL, pCoefficientMatrix)
        ? ERROR : SUCCESS;
    }

/*-----------------------------------------------------------------*//**
*
* Initializes a range cube with (inverted) large positive and negative
* values.
*
* @param
* @see
* @indexVerb init
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public  void     bsiDRange2d_init

(
DRange2dP pRange        /* <= range to be initialized */
)

    {
    pRange->low.x = pRange->low.y = DBL_MAX;
    pRange->high.x = pRange->high.y = -DBL_MAX;
    }

/*-----------------------------------------------------------------*//**
*
* Initizlizes the range to contain the range of the given array of points.
* If there are no points in the array, the range is initialized by
* DRange2d.init()
*
* @param pPoint => array of points to search
* @param n => number of points in array
* @see
* @indexVerb init
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public  void     bsiDRange2d_initFromArray

(
DRange2dP pRange,
DPoint2dCP pPoint,
int             n
)

    {
    int i;
    DPoint2d *  minP = &pRange->low;
    DPoint2d *  maxP = &pRange->high;
    if (n < 1)
        {
        bsiDRange2d_init (pRange);
        }
    else
        {
        *minP = *maxP = pPoint[0];
        for (i=1; i<n; i++)
            {
            FIX_MINMAX ( pPoint[i].x, minP->x, maxP->x );
            FIX_MINMAX ( pPoint[i].y, minP->y, maxP->y );
            }
        }
    }

/*-----------------------------------------------------------------*//**
*
* @return the largest individual coordinate value among (a) range min point,
* (b) range max point, and (c) range diagonal vector.
* @see
* @indexVerb extrema
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public  double bsiDRange2d_getLargestCoordinate

(
DRange2dCP pRange
)

    {
     double     max;
     DPoint2d diagonal;

     max = fabs(pRange->low.x);
     FIX_MAX(fabs(pRange->high.x), max);
     FIX_MAX(fabs(pRange->low.y), max);
     FIX_MAX(fabs(pRange->high.y), max);

     diagonal.DifferenceOf (pRange->high, pRange->low);

     FIX_MAX(fabs(diagonal.x), max);
     FIX_MAX(fabs(diagonal.y), max);

     return max;

    }
/*-----------------------------------------------------------------*//**
*
* Compute the intersection of a range cube and a ray.
*
* If there is not a finite intersection, both params are set to 0 and
* and both points to pPoint0.
*
* @param pParam0 <= ray parameter where cube is entered
* @param pParam1 <= ray parameter where cube is left
* @param pPoint0 <= entry point
* @param pPoint1 <= exit point
* @param pStart => start point of ray
* @param pDirection => direction of ray
* @return true if non-empty intersection.
* @see
* @indexVerb intersect
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public  bool        bsiDRange2d_intersectRay

(
DRange2dCP pRange,
double      *pParam0,
double      *pParam1,
DPoint2dP pPoint0,
DPoint2dP pPoint1,
DPoint2dCP pStart,
DPoint2dCP pDirection
)
    {
    double s0, s1;      /* parameters of 'in' segment */
    int     contents = -1;
    bool    boolStat;
    /* save points in case of duplicate pointers by caller */
    DPoint2d start;
    DPoint2d direction;

    start = *pStart;
    direction = *pDirection;

    bsiRange1d_intersectLine
                (
                &s0, &s1, &contents,
                pStart->x, pDirection->x,
                pRange->low.x, pRange->high.x
                );
    bsiRange1d_intersectLine
                (
                &s0, &s1, &contents,
                pStart->y, pDirection->y,
                pRange->low.y, pRange->high.y
                );

    if (contents > 0)
        {
        boolStat = true;
        }
    else
        {
        s0 = 0.0;
        s1 = 0.0;
        boolStat = false;
        }

    /* Copy to outputs (all optional) */
    if (pParam0)
        *pParam0 = s0;

    if (pParam1)
        *pParam1 = s1;

    if (pPoint0)
        pPoint0->SumOf (start, direction, s0);

    if (pPoint1)
        pPoint1->SumOf (start, direction, s1);

    return  boolStat;
    }


/*-----------------------------------------------------------------*//**
* Project a point to a plane defined by origin and (not necessarily unit)
* normal vector.
*
* @param pOutPoint <= projected point (or NULL)
* @param pInPoint  => point to project to plane
* @param pNormal   => plane normal
* @param pOrigin   => plane origin
* @return signed distance from point to plane.  If the plane normal has zero length,
*           distance to plane origin.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public  double bsiDPoint3d_distancePointToPlane

(
DPoint3dP pOutPoint,
DPoint3dCP pInPoint,
DPoint3dCP pNormal,
DPoint3dCP pOrigin
)
    {
    double      dist, mag;
    DPoint3d    diff;
    DPoint3d    unitNorm;

    mag = unitNorm.Normalize (*pNormal);
    if (mag == 0.0)
        {
        if (pOutPoint)
            *pOutPoint = *pOrigin;
        return pInPoint->Distance (*pOrigin);
        }

    diff.DifferenceOf (*pOrigin, *pInPoint);
    dist = diff.DotProduct (unitNorm);

    if (pOutPoint)
        pOutPoint->SumOf (*pInPoint, unitNorm, dist);

    return  dist;
    }
/*-----------------------------------------------------------------*//**
* @description Compute the intersection point of a line and a plane.
*
* @param pParam <= intersection parameter within line
* @param pPoint <= intersection point
* @param pLineStart => point on line at parameter 0.0
* @param pLineEnd => point on line at parameter 1.0
* @param pOrigin => any point on plane
* @param pNormal => normal vector for plane
* @return true unless the line is parallel to the plane.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public  bool    bsiGeom_linePlaneIntersection

(
double      *pParam,
DPoint3dP pPoint,
DPoint3dCP pLineStart,
DPoint3dCP pLineEnd,
DPoint3dCP pOrigin,
DPoint3dCP pNormal
)
    {
    bool    result;
    double dot1, dot2, param;
    static double maxFactor = 1.0e14;

    dot2 = pLineEnd->DotDifference(*pLineStart, *((DVec3d const*) pNormal));
    dot1 = pOrigin->DotDifference(*pLineStart, *((DVec3d const*) pNormal));

    if (fabs(dot1) < maxFactor * fabs(dot2))
        {
        param = dot1 / dot2;
        result = true;
        }
    else

        {
        param = 0.0;
        result = false;
        }

    if (pParam)
        *pParam = param;
    if (pPoint)
        pPoint->Interpolate (*pLineStart, param, *pLineEnd);
    return  result;
    }

/*---------------------------------------------------------------------------------
* Functions to implement "Union-Find" in a EmbeddedIntArray.
*
* Start with an empty array.
*
* Call jmdlVArrayInt_newClusterIndex () as needed to get cluster id's.
*
* Call jmdlVArrayInt_mergeClusters (cluster0, cluster1) to do "union" operation
* on the clusters. Returns the id of the merged cluster.  Thereafter cluster0
* and cluster1 are still valid ids -- you can get the merged cluster from either
* by calling  ...
*
* mergedCluseter = jmdlVArrayInt_getMergedClusterIndex (cluster)
*
* Implementation:
* A cluster indices are indices into the array.
* A new cluster is a new entry at the end of the array, referencing itself as its parent.
+---------------+---------------+---------------+---------------+---------------+------*/

/*---------------------------------------------------------------------------------**//**
* Create a new cluster index for a union-find algorithm.
*
* @param    pInstance => int array being used for union find.
* @return
* @bsimethod                                                    EarlinLutz      06/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      jmdlVArrayInt_newClusterIndex
(
EmbeddedIntArray  *pInstance
)
    {
    int index = IntArrayWrapper::getCount (pInstance);

    IntArrayWrapper::set (pInstance, &index, index);
    return  index;
    }


/*---------------------------------------------------------------------------------**//**
* Search upwards in the union-find structure for a parent cluster.
* Fixup indices along the way!!!   The parent index always is a root (i.e. is its own parent)
* @param    pInstance <=> int array being used for union find.
* @param cluster0 => first cluster id
* @return the merged cluster index.
* @bsimethod                                                    EarlinLutz      06/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int  jmdlVArrayInt_getMergedClusterIndexExt
(
EmbeddedIntArray  *pInstance,
int         cluster,
int         depth
)
    {
    int parent;
    static int errors = 0;
    if (SUCCESS == IntArrayWrapper::get (pInstance, &parent, cluster))
        {
        if (parent != cluster)
            {
            if (depth > 10)
                {
                errors++;
                IntArrayWrapper::set (pInstance, &cluster, cluster);
                }
            parent = jmdlVArrayInt_getMergedClusterIndexExt (pInstance, parent, depth + 1);
            IntArrayWrapper::set (pInstance, &parent, cluster);
            }
        }
    return  parent;
    }

/**
* @param pDestArray array where shuffled data is placed
* @param pSourceArray original array
* @param pIndexArray index information
* @see
* @return SUCCESS if
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP StatusInt jmdlVArrayInt_shuffleArray
(
        EmbeddedDPoint3dArray     *pDestArray,    // array where shuffled data is placed
const   EmbeddedDPoint3dArray     *pSourceArray,  // original array
const   bvector<int>   *pIndexArray    // index information
)
    {
    uint32_t i;
    int j;
    int status = ERROR;
    size_t n = (int)pIndexArray->size ();
    for (i = 0; i < n; i++)
        {
        j = pIndexArray->at (i);
        if (j >= 0)
            DPoint3dArrayWrapper::moveItem (pDestArray, j, pSourceArray, i);
        }
    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* Search upwards in the union-find structure for a parent cluster.
* Fixup indices along the way!!!   The parent index always is a root (i.e. is its own parent)
* @param    pInstance <=> int array being used for union find.
* @param cluster0 => first cluster id
* @return the merged cluster index.
* @bsimethod                                                    EarlinLutz      06/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int  jmdlVArrayInt_getMergedClusterIndex
(
EmbeddedIntArray  *pInstance,
int         cluster
)
    {
    return jmdlVArrayInt_getMergedClusterIndexExt (pInstance, cluster, 0);
    }

/*---------------------------------------------------------------------------------**//**
* @param    pInstance <=> int array being used for union find.
* @param cluster0 => first cluster id
* @param cluster1 => second cluster id
* @return the merged cluster index (may be different from both!!)
* @bsimethod                                                    EarlinLutz      06/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      jmdlVArrayInt_mergeClusters
(
EmbeddedIntArray  *pInstance,
int         cluster0,
int         cluster1
)
    {
    int parent0 = jmdlVArrayInt_getMergedClusterIndex (pInstance, cluster0);
    int parent1 = jmdlVArrayInt_getMergedClusterIndex (pInstance, cluster1);
    if (parent1 != parent0)
        {
        IntArrayWrapper::set (pInstance,  &parent1, parent0);
        jmdlVArrayInt_getMergedClusterIndex (pInstance, cluster0);
        }
    return  parent1;
    }

/*----------------------------------------------------------------------+
|FUNC           compareDPoint2dX                                        |
|AUTHOR         EarlinLutz                              06/00           |
+----------------------------------------------------------------------*/
static int compareDPoint2dX
(
const void    *pElem1,
const void    *pElem2
)
    {
    DPoint2dCP pA = (DPoint2dCP) pElem1;
    DPoint2dCP pB= (DPoint2dCP) pElem2;
    if (pA->x < pB->x)
        return -1;
    if (pA->x > pB->x)
        return 1;
    return 0;
    }

/*----------------------------------------------------------------------+
|FUNC           arePointsClose_absXYZ                                   |
|AUTHOR         EarlinLutz                              07/01           |
+----------------------------------------------------------------------*/
static bool    arePointsClose_absXYZ
(
const DPoint3d *pPoint0,
const DPoint3d *pPoint1,
double epsilon
)
    {
    return  fabs (pPoint0->x - pPoint1->x) <= epsilon
        &&  fabs (pPoint0->y - pPoint1->y) <= epsilon
        &&  fabs (pPoint0->z - pPoint1->z) <= epsilon
        ;
    }

/*----------------------------------------------------------------------+
|FUNC           arePointsClose_absXY                                    |
|AUTHOR         EarlinLutz                              07/01           |
+----------------------------------------------------------------------*/
static bool    arePointsClose_absXY
(
const DPoint3d *pPoint0,
const DPoint3d *pPoint1,
double epsilon
)
    {
    return  fabs (pPoint0->x - pPoint1->x) <= epsilon
        &&  fabs (pPoint0->y - pPoint1->y) <= epsilon
        ;
    }

typedef bool    (*PointComparisonFunction)
    (
    const DPoint3d *pPoint0,
    const DPoint3d *pPoint1,
    double epsilon
    );

/*--------------------------------------------------------------------*//*
* @param pXYZArray => array of n points, containing possibly matched points.
* @param pCycleArray => array of n indices, arranged as cyclic linked lists
*               joining points with identical points.  May be null pointer.
* @param pBlockedIndexArray => array containing packed blocks of point indices,
*               each terminated by index -1.  This will contain at least n+1
*               and at most 2n indices.  May be null pointer.
* @param absTol = absolute tolerance for common points.
* @param relTol = relative tolerance for common points.
* @return number of distinct points, hence number of cycles and blocks
*               in the index arrays.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
static int jmdlVArrayDPoint3d_identifyMatchedVertices_generic
(
const EmbeddedDPoint3dArray *pXYZArray,
EmbeddedIntArray      *pCycleArray,
EmbeddedIntArray      *pBlockedIndexArray,
double          absTol,
double          relTol,
DPoint3d        *pSortVector,
PointComparisonFunction cb_pointsClose
)
    {
    const DPoint3d *pXYZBuffer = pXYZArray->data ();
    int numXYZ = (int)pXYZArray->size ();

    bvector<DPoint2d> sortArray;
    DPoint3d normalizedSortVector;
    DPoint2d *pSortBuffer;
    int *pCycleBuffer;
    DPoint3d point0, point1;
    double lowerBlockCoordinate, upperBlockCoordinate;
    int i0, i1;
    int k0, k1, k2;
    int numBlock = 0;

    double LargestCoordinate = bsiDPoint3d_getLargestCoordinateDifference (pXYZBuffer, numXYZ);
    double epsilon;

    if (numXYZ == 0)
        {
        if (pBlockedIndexArray)
            IntArrayWrapper::empty (pBlockedIndexArray);
        if (pCycleArray)
            IntArrayWrapper::empty (pCycleArray);
        return 0;
        }

    if (absTol <= 0.0)
        absTol = 0.0;
    if (relTol <= 0.0)
        relTol = 0.0;

    epsilon = absTol + relTol * LargestCoordinate;
    normalizedSortVector.Normalize (*pSortVector);

    sortArray.reserve (numXYZ);
    /* Force data into the sort area.. */
    {
    DPoint2d zero;
    zero.Zero ();
    for (int i = 0; i < numXYZ; i++)
        sortArray.push_back (zero);
    }
    pSortBuffer = sortArray.data ();

    if (pBlockedIndexArray)
        {
        IntArrayWrapper::empty (pBlockedIndexArray);
        }

    /* Search for non-disconnect reference point */
    for (k0 = 0; k0 < numXYZ; k0++)
        {
        if (!pXYZBuffer[ k0].IsDisconnect ())
            break;
        }

    point0 = pXYZBuffer[k0];
    /* Initialize sort indices as identity permutation with dot product along
        skewed dimension as sort quantity. */
    for (;k0 < numXYZ; k0++)
        {
        if (!pXYZBuffer[ k0].IsDisconnect ())
            {
            pSortBuffer[k0].x = pXYZBuffer[ k0].DotDifference(point0, *((DVec3d *) &normalizedSortVector));
            pSortBuffer[k0].y = (double)k0;
            }
        }

    if (pCycleArray)
        {
        IntArrayWrapper::empty (pCycleArray);
        for (k0 = 0; k0 < numXYZ; k0++)
            {
            IntArrayWrapper::add (pCycleArray, k0);
            }
        pCycleBuffer = IntArrayWrapper::getPtr (pCycleArray, 0);
        }
    else
        {
        pCycleBuffer = NULL;
        }

    qsort ((void*) pSortBuffer, numXYZ, sizeof (DPoint2d), compareDPoint2dX);

    for (i0 = 0; i0 < numXYZ; i0++)
        {
        k0 = (int)pSortBuffer[i0].y;
        if (k0 >= 0)
            {
            lowerBlockCoordinate = pSortBuffer[i0].x;
            upperBlockCoordinate = lowerBlockCoordinate + epsilon;
            /* This coordinate starts a new block.
               Record it and all succeeding near points into a block of the index array.
               The points that are near in ALL directions are clustered in a block
               with almost the same sort coordinate;  walk through the block
               of points with similar sort coordinate, begin aware that there may
               be (a) points previously picked out from prior blocks, (index -1)
                    (b) points far away but not yet recorded.
               In the cycle index array, each index is initially a singleton cycle.
               When a point is identified as part of a block, it is still a singleton
               cycle.  The singleton cycle is spliced together with the growing block
               cycle by swapping successor indices athe the new point and the base
               point of the block.
            */
            if (pBlockedIndexArray)
                IntArrayWrapper::add (pBlockedIndexArray, k0);
            point0 = pXYZBuffer[k0];
            numBlock++;
            for (i1 = i0 + 1;
                 i1 < numXYZ && pSortBuffer[i1].x < upperBlockCoordinate;
                 i1++)
                {
                k1 = (int)pSortBuffer[i1].y;
                if (k1 >= 0)
                    {
                    point1 = pXYZBuffer[k1];
                    if (cb_pointsClose (&point0, &point1, epsilon))
                        {
                        pSortBuffer[i1].y = -1;
                        if (pBlockedIndexArray)
                            IntArrayWrapper::add (pBlockedIndexArray, k1);
                        if (pCycleBuffer)
                            {
                            k2 = pCycleBuffer[k0];
                            pCycleBuffer[k0] = k1;
                            pCycleBuffer[k1] = k2;
                            }
                        }
                    }
                }
            if (pBlockedIndexArray)
                IntArrayWrapper::add (pBlockedIndexArray, -1);
            }
        }
    return numBlock;
    }


/*--------------------------------------------------------------------*//*
* @param pXYZArray => array of n points, containing possibly matched points.
* @param pCycleArray => array of n indices, arranged as cyclic linked lists
*               joining points with identical points.  May be null pointer.
* @param pBlockedIndexArray => array containing packed blocks of point indices,
*               each terminated by index -1.  This will contain at least n+1
*               and at most 2n indices.  May be null pointer.
* @param absTol = absolute tolerance for common points.
* @param relTol = relative tolerance for common points.
* @return number of distinct points, hence number of cycles and blocks
*               in the index arrays.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int jmdlVArrayDPoint3d_identifyMatchedVertices
(
const EmbeddedDPoint3dArray *pXYZArray,
EmbeddedIntArray      *pCycleArray,
EmbeddedIntArray      *pBlockedIndexArray,
double          absTol,
double          relTol
)
    {
    DPoint3d sortVector;
    sortVector.x = 0.5677470545;
    sortVector.y = 1.8340234005;
    sortVector.z = 1.3472498290;
    return jmdlVArrayDPoint3d_identifyMatchedVertices_generic (pXYZArray, pCycleArray, pBlockedIndexArray,
                    absTol, relTol, &sortVector, arePointsClose_absXYZ);
    }


/*--------------------------------------------------------------------*//*
* @param pXYZArray => array of n points, containing possibly matched points.
* @param pCycleArray => array of n indices, arranged as cyclic linked lists
*               joining points with identical points.  May be null pointer.
* @param pBlockedIndexArray => array containing packed blocks of point indices,
*               each terminated by index -1.  This will contain at least n+1
*               and at most 2n indices.  May be null pointer.
* @param absTol = absolute tolerance for common points.
* @param relTol = relative tolerance for common points.
* @return number of distinct points, hence number of cycles and blocks
*               in the index arrays.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int jmdlVArrayDPoint3d_identifyMatchedVerticesXY
(
const EmbeddedDPoint3dArray *pXYZArray,
EmbeddedIntArray      *pCycleArray,
EmbeddedIntArray      *pBlockedIndexArray,
double          absTol,
double          relTol
)
    {
    DPoint3d sortVector;
    sortVector.x = 0.5677470545;
    sortVector.y = 1.8340234005;
    sortVector.z = 0.0;
    return jmdlVArrayDPoint3d_identifyMatchedVertices_generic (pXYZArray, pCycleArray, pBlockedIndexArray,
                    absTol, relTol, &sortVector, arePointsClose_absXY);
    }

/*-----------------------------------------------------------------*//**
* @description Returns an upper bound for both the largest absolute value x, y or z
* coordinate and the greatest distance between any two x,y or z coordinates
* in an array of points.
*
* @param pPointArray => array of points to test
* @param numPoint => number of points
* @return upper bound as described above. or zero if no points
* @see bsiDPoint3d_getLargestCoordinateDifference, bsiDPoint3d_getLargestWeightedCoordinateDifference, bsiDPoint3d_getLargestXYCoordinate
* @group "DPoint3d Queries"
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDPoint3d_getLargestCoordinate

(
DPoint3dCP pPointArray,
int         numPoint
)
    {
    if (pPointArray && numPoint > 0)
        {
        DRange3d tmpRange;
        tmpRange.InitFrom(pPointArray, numPoint);
        return tmpRange.LargestCoordinate ();
        }
    else
        {
        return 0.0;
        }
    }


/*-----------------------------------------------------------------*//**
* @description Returns an upper bound for the greatest distance between any two x, y or z
* coordinates in an array of points.
*
* @param pPointArray => array of points to test
* @param numPoint => number of points
* @return upper bound as described above, or zero if no points
* @see bsiDPoint3d_getLargestCoordinate, bsiDPoint3d_getLargestWeightedCoordinateDifference, bsiDPoint3d_getLargestXYCoordinate
* @group "DPoint3d Queries"
* @bsihdr                                       DavidAssaf      04/02
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDPoint3d_getLargestCoordinateDifference

(
DPoint3dCP pPointArray,
int         numPoint
)
    {
    if (pPointArray && numPoint > 0)
        {
        DRange3d tmpRange;
        DPoint3d diagonal;
        tmpRange.InitFrom(pPointArray, numPoint);
        diagonal.DifferenceOf (tmpRange.high, tmpRange.low);
        return diagonal.MaxAbs ();
        }
    else
        {
        return 0.0;
        }
    }


/*-----------------------------------------------------------------*//**
* @description Returns an upper bound for the greatest distance between any two x, y or z
* coordinates in an array of weighted points.
* @remarks Points with zero weight are ignored.
*
* @param pPointArray => array of weighted points to test
* @param pWeightArray => array of weights
* @param numPoint => number of points and weights
* @return upper bound as described above, or zero if no points
* @group "DPoint3d Queries"
* @see bsiDPoint3d_getLargestCoordinateDifference, bsiDPoint3d_getLargestCoordinate, bsiDPoint3d_getLargestXYCoordinate
* @bsihdr                                       DavidAssaf      04/02
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDPoint3d_getLargestWeightedCoordinateDifference

(
DPoint3dCP    pPointArray,
const   double*     pWeightArray,
int         numPoint
)
    {
    if (!pWeightArray)
        return bsiDPoint3d_getLargestCoordinateDifference (pPointArray, numPoint);

    if (pPointArray && numPoint > 0)
        {
        DRange3d    tmpRange;
        DPoint3d    diagonal;
        double      wRecip;
        int         i;

        tmpRange.Init ();
        for (i = 0; i < numPoint; i++)
            if (DoubleOps::SafeDivide (wRecip, 1.0, pWeightArray[i], 0.0))
                tmpRange.Extend (pPointArray[i].x * wRecip, pPointArray[i].y * wRecip, pPointArray[i].z * wRecip);

        diagonal.DifferenceOf (tmpRange.high, tmpRange.low);
        return diagonal.MaxAbs ();
        }
    else
        {
        return 0.0;
        }
    }

/*-----------------------------------------------------------------*//**
* @description Add a given point to each of the points of an array.
*
* @param pArray <=> array whose points are to be incremented
* @param pDelta => point to add to each point of the array
* @param numPoints => number of points
* @group "DPoint3d Addition"
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint3d_addDPoint3dArray

(
DPoint3dP pArray,
DPoint3dCP pDelta,
int              numPoints
)
    {
    int         i;
    DPoint3d   *pPoint = pArray;
    double      x = pDelta->x;
    double      y = pDelta->y;
    double      z = pDelta->z;

    for (i=0; i < numPoints; i++)
        {
        pPoint->x += x;
        pPoint->y += y;
        pPoint->z += z;
        pPoint++;
        }
    }

/*-----------------------------------------------------------------*//**
* @description Subtract a given point from each of the points of an array.
*
* @param pArray <=> Array whose points are to be decremented
* @param pDelta => point to subtract from each point of the array
* @param numVerts => number of points
* @group "DPoint3d Addition"
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint3d_subtractDPoint3dArray

(
DPoint3dP pArray,
DPoint3dCP pDelta,
int              numVerts
)
    {
    int         i;
    DPoint3d   *pPoint = pArray;
    double      x = pDelta->x;
    double      y = pDelta->y;
    double      z = pDelta->z;

    for (i=0; i<numVerts; i++)
        {
        pPoint->x -= x;
        pPoint->y -= y;
        pPoint->z -= z;
        pPoint++;
        }
    }


/*-----------------------------------------------------------------*//**
* @description Copy the given number of DPoint3d structures from the pSource array to the pDest array.
*
* @param pDest <= destination array
* @param pSource => source array
* @param n => number of points
* @group "DPoint3d Copy"
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint3d_copyArray

(
DPoint3dP pDest,
DPoint3dCP pSource,
int          n
)
    {
#if defined (__jmdl)
    int i;
    for (i = 0; i < n ;i++)
        {
        pDest[i]= pSource[i];
        }
#else
    memcpy (pDest, pSource, n*sizeof(DPoint3d) );
#endif
    }


/*-----------------------------------------------------------------*//**
* @description Reverse the order of points in the array.
* @param pXYZ => source array
* @param n => number of points
* @group "DPoint3d Copy"
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint3d_reverseArrayInPlace
(
DPoint3dP pXYZ,
int          n
)
    {
    int i, j;
    for (i= 0, j = n - 1; i < j; i++, j--)
        {
        DPoint3d xyz = pXYZ[i];
        pXYZ[i] = pXYZ[j];
        pXYZ[j] = xyz;
        }
    }
/*-----------------------------------------------------------------*//**
* @description Approximate a plane through a set of points.
* @remarks The method used is:
    <ul>
    <li>Find the bounding box.</li>
    <li>Choose the axis with greatest range.</li>
    <li>Take two points that are on the min and max of this axis.</li>
    <li>Also take as a third point the point that is most distant from the line connecting the two extremal points.</li>
    <li>Form plane through these 3 points.</li>
    </ul>
* @param pNormal <= plane normal
* @param pOrigin <= origin for plane
* @param pPoint => point array
* @param numPoint => number of points
* @param tolerance => max allowable deviation from colinearity (or nonpositive to compute minimal tolerance)
* @return true if the points define a clear plane; false if every point lies on the line (within tolerance) joining the two extremal points.
* @group "DPoint3d Queries"
* @bsihdr                                       DavidAssaf      06/01
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     bsiGeom_planeThroughPointsTol

(
DPoint3dP pNormal,
DPoint3dP pOrigin,
DPoint3dCP pPoint,
int             numPoint,
double          tolerance
)
    {
    int i;
    int i0, i1;
    bool    result = false;

    DPoint3d point0;
    DPoint3d point1;
    DPoint3d vector;

    DPoint3d currVector;
    DPoint3d currNormal;
    DPoint3d maxNormal;

    double delta, deltaMax;
    static double relTol = 1.0e-12;
    double myTol = bsiDPoint3d_getLargestCoordinateDifference (pPoint, numPoint) * relTol;
    double myTol2;

    if (tolerance > myTol)
        myTol = tolerance;

    myTol2 = myTol * myTol;

    if (numPoint > 2)
        {
        bsiGeom_findWidelySeparatedPoints
            (&point0, &i0, &point1, &i1, pPoint, numPoint);

        vector.NormalizedDifference (point1, point0);

        deltaMax = 0.0;
        for (i = 0; i < numPoint; i++)
            {
            if ( i != i0 && i != i1)
                {
                currVector.DifferenceOf (pPoint[ i], point0);
                currNormal.SumOf (currVector, vector, -currVector.DotProduct (vector));
                delta = currNormal.DotProduct (currNormal);
                if (delta > deltaMax)
                    {
                    maxNormal = currNormal;
                    deltaMax = delta;
                    }
                }
            }


        if (deltaMax > myTol2)
            {
            *pOrigin = point1;
            pNormal->CrossProduct (vector, maxNormal);
            result = true;
            }
        }
    return result;
    }


/*-----------------------------------------------------------------*//**
* @description Approximate a plane through a set of points.
* @remarks This function calls ~mbsiGeom_planeThroughPointsTol with tolerance = 0.0 to force usage of smallest colinearity tolerance.
* @param pNormal <= plane normal
* @param pOrigin <= origin for plane
* @param pPoint => point array
* @param numPoint => number of points
* @return true if the points define a clear plane; false if every point lies on the line joining the two extremal points.
* @group "DPoint3d Queries"
* @bsihdr                                       DavidAssaf      12/98
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     bsiGeom_planeThroughPoints

(
DPoint3dP pNormal,
DPoint3dP pOrigin,
DPoint3dCP pPoint,
int             numPoint
)
    {
    return bsiGeom_planeThroughPointsTol (pNormal, pOrigin, pPoint, numPoint, 0.0);
    }
/*-----------------------------------------------------------------*//**
* @description Find two points (and their indices) in the given array of points that are relatively far from each other.
* @remarks The returned points are not guaranteed to be the points with farthest separation.
*
* @param pMinPoint  <= first of the two widely separated points (or null)
* @param pMinIndex  <= index of first point (or null)
* @param pMaxPoint  <= second of the two widely separated points (or null)
* @param pMaxIndex  <= index of second point (or null)
* @param pPoints    => array of points
* @param numPts     => number of points
* @return false if numPts < 2
* @group "DPoint3d Queries"
* @bsihdr                                       EarlinLutz      12/97
* @bsihdr                                       DavidAssaf      12/98
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool        bsiGeom_findWidelySeparatedPoints

(
DPoint3dP pMinPoint,
int                 *pMinIndex,
DPoint3dP pMaxPoint,
int                 *pMaxIndex,
const DPoint3d      *pPoints,
int                 numPts
)
    {
    double const*pArray;        /* TREAT DPOINT3D AS ARRAY OF 3 DOUBLES*/
    double      aMin[3];
    double      aMax[3];
    double      a;
    double      delta, deltaMax;
    int         iMin[3];
    int         iMax[3];
    int         kMax;
    int         minIndex, maxIndex;
    int         i, k;

    if (numPts < 2)
        return false;

    /* Find extrema on each axis, keeping track of their indices in the array.*/
    pArray = (double const*)pPoints;
    for (k = 0; k < 3; k++)         /* init min/max vals/indices w/ 1st pt */
        {
        aMin[k] = aMax[k] = pArray[k];
        iMin[k] = iMax[k] = 0;
        }
    for (i = 1; i < numPts; i++)    /* compare min/max vals/indices w/ other pts */
        {
        pArray = (double const*)(pPoints + i);
        for (k = 0; k < 3; k++)
            {
            a = pArray[k];
            if (a < aMin[k])
                {
                aMin[k] = a;
                iMin[k] = i;
                }
            else if (a > aMax[k])
                {
                aMax[k] = a;
                iMax[k] = i;
                }
            }
        }

    /* Find the axis (kMax) with largest range.*/
    kMax = 0;
    deltaMax = fabs (aMax[0] - aMin[0]);

    for (k = 1; k < 3; k++)
        {
        delta = fabs (aMax[k] - aMin[k]);
        if (delta > deltaMax)
            {
            deltaMax = delta;
            kMax = k;
            }
        }

    minIndex = iMin[kMax];
    maxIndex = iMax[kMax];

    if (pMinIndex)
        *pMinIndex = minIndex;
    if (pMaxIndex)
        *pMaxIndex = maxIndex;
    if (pMinPoint)
        *pMinPoint = pPoints[minIndex];
    if (pMaxPoint)
        *pMaxPoint = pPoints[maxIndex];

    return true;
    }

/*-----------------------------------------------------------------*//**
* qsort comparator.  Compare w parts of pA, pB; if equal comapre (squared)
*       distance from origin.
* @bsihdr                                       EarlinLutz 08/02
+---------------+---------------+---------------+---------------+------*/
static int compareW

(
DPoint4dCP pA,
DPoint4dCP pB
)
    {
    double rrA, rrB;
    if (pA->w < pB->w)
        return -1;
    if (pA->w > pB->w)
        return 1;
    rrA = pA->x * pA->x + pA->y * pA->y;
    rrB = pB->x * pB->x + pB->y * pB->y;

    if (rrA < rrB)
        return -1;
    if (rrA > rrB)
        return 1;
    return 0;
    }

/*-----------------------------------------------------------------*//**
* @description Trim points from the tail of an evolving hull if they
*   are covered by segment from new point back to earlier point.
* @param pXYZA IN evolving hull array.
* @param num IN number of points in initial hull.
* @param pXYZNew = new point being added to hull.
* @return number of points in hull array after removals.
* @bsihdr                                       EarlinLutz 08/02
+---------------+---------------+---------------+---------------+------*/
static int trimHull

(
DPoint4dP pXYZA,
int     num,
DPoint4dCP pXYZNew
)
    {
    int i0, i1;
    double dx0, dy0, dx1, dy1;
    double cross;
    while (num >= 2)
        {
        i0 = num - 1;
        i1 = i0 - 1;
        dx0 = pXYZA[i0].x - pXYZNew->x;
        dy0 = pXYZA[i0].y - pXYZNew->y;
        dx1 = pXYZA[i1].x - pXYZNew->x;
        dy1 = pXYZA[i1].y - pXYZNew->y;
        cross = dx0 * dy1 - dy0 * dx1;
        if (cross < 0.0)
            break;
        num--;
        }
    return num;
    }

/*-----------------------------------------------------------------*//**
* @description Compute a convex hull of given points.  Each output point
*       is one of the inputs, including its z part.
* @param pOutBuffer OUT Convex hull points.  First/last point NOT duplicated.
*       This must be allocated to the caller, large enough to contain numIn points.
* @param pNumOut OUT number of points on hull
* @param pInBuffer IN input points.
* @param numIn IN number of input points.
* @param iMax IN index of point at maximal radius, i.e. guaranteed to be on hull.
* @bsihdr                                       EarlinLutz 08/02
+---------------+---------------+---------------+---------------+------*/
static bool    bsiDPoint3dArray_convexHullXY_go

(
DPoint3dP pOutBuffer,
int         *pNumOut,
DPoint4dP pXYZA,
int         numIn,
int         iMax
)
    {
    int i;
    int numOut;
    double shift = 8.0 * atan (1.0); /* 2pi */
    double theta0 = pXYZA[iMax].w;
    for (i = 0; i < numIn; i++)
        {
        if (pXYZA[i].w < theta0)
            pXYZA[i].w += shift;
        }
    qsort (pXYZA, numIn, sizeof (DPoint4d),
            (int (*)(const void *, const void *))compareW);
    numOut = 1;
    for (i = 1; i < numIn; i++)
        {
        numOut = trimHull (pXYZA, numOut, &pXYZA[i]);
        pXYZA[numOut++] = pXYZA[i];
        }
    if (numOut > 2)
        numOut = trimHull (pXYZA, numOut, &pXYZA[0]);
    *pNumOut = numOut;
    return true;
    }

/*-----------------------------------------------------------------*//**
* @description Compute a convex hull of a point array, ignoring z-coordinates.
* @remarks Each output point is one of the inputs, including its z-coordinate.
* @param pOutBuffer OUT convex hull points, first/last point <em>not</em> duplicated.
*                       This must be allocated by the caller, large enough to contain numIn points.
* @param pNumOut    OUT number of points on hull
* @param pInBuffer  IN  input points
* @param numIn      IN  number of input points
* @return false if numin is nonpositive or memory allocation failure
* @group "DPoint3d Queries"
* @bsihdr                                       EarlinLutz      08/02
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDPoint3dArray_convexHullXY

(
DPoint3dP pOutBuffer,
int         *pNumOut,
DPoint3dP pInBuffer,
int         numIn
)
    {
    int i;
    double xsum, ysum;
    double dx, dy, rr, rrMax;
    int iMax, k;
    double x0, y0;
    int numOut = 0;
    bool    boolstat = false;
    int numSort;

    numOut = 0;
    if (pNumOut)
        *pNumOut = numOut;

    if (numIn <= 0)
        return false;

    bvector<DPoint4d>sortXYZA ((size_t)(numIn + 1));

    if (numIn == 1)
        {
        numOut = 1;
        pOutBuffer[0] = pInBuffer[0];
        boolstat = true;
        }
    else
        {
        /* Compute centroid of all points, relative to first point. */
        xsum = ysum = 0.0;
        x0 = pInBuffer[0].x;
        y0 = pInBuffer[0].y;
        for (i = 1; i < numIn; i++)
            {
            xsum += pInBuffer[i].x - x0;
            ysum += pInBuffer[i].y - y0;
            }
        x0 += xsum / numIn;
        y0 += ysum / numIn;
        /* Set up work array with x,y,i,angle in local coordinates around centroid. */
        iMax = -1;
        rrMax = 0.0;
        numSort = 0;
        for (i = 0; i < numIn; i++)
            {
            dx = sortXYZA[numSort].x = pInBuffer[i].x - x0;
            dy = sortXYZA[numSort].y = pInBuffer[i].y - y0;
            sortXYZA[numSort].z = (double)i;
            sortXYZA[numSort].w = Angle::Atan2 (dy, dx);
            rr = dx * dx + dy * dy;
            if (rr > 0.0)
                {
                if (rr > rrMax)
                    {
                    iMax = numSort;
                    rrMax = rr;
                    }
                numSort++;
                }
            }

        if (numSort == 0)
            {
            /* All points are at the centroid. Copy the first one out. */
            numOut = 1;
            if (pOutBuffer)
                pOutBuffer[0] = pInBuffer[0];
            boolstat = true;
            }
        else
            {
            boolstat = bsiDPoint3dArray_convexHullXY_go (pOutBuffer, &numOut, &sortXYZA[0], numSort, iMax);
            if (boolstat)
                {
                for (i = 0; i < numOut; i++)
                    {
                    k = (int)sortXYZA[i].z;
                    pOutBuffer[i] = pInBuffer[k];
                    }
                }
            else
                numOut = 0;
            }
        }
    if (pNumOut)
        *pNumOut = numOut;
    return boolstat;
    }

/*-----------------------------------------------------------------*//**
* qsort comparator.  Compare x parts of pA, pB
* @bsihdr                                       EarlinLutz 08/02
+---------------+---------------+---------------+---------------+------*/
static int compareX
(
const DPoint3d *pA,
const DPoint3d *pB
)
    {
    if (pA->x < pB->x)
        return -1;
    if (pA->x > pB->x)
        return 1;
    return 0;
    }

/*-----------------------------------------------------------------*//**
* qsort comparator.  Compare y parts of pA, pB
* @bsihdr                                       EarlinLutz 08/02
+---------------+---------------+---------------+---------------+------*/
static int compareY
(
const DPoint3d *pA,
const DPoint3d *pB
)
    {
    if (pA->y < pB->y)
        return -1;
    if (pA->y > pB->y)
        return 1;
    return 0;
    }

/*-----------------------------------------------------------------*//**
* qsort comparator.  Compare x parts of pA, pB
* @bsihdr                                       EarlinLutz 08/02
+---------------+---------------+---------------+---------------+------*/
static int compareZ
(
const DPoint3d *pA,
const DPoint3d *pB
)
    {
    if (pA->z < pB->z)
        return -1;
    if (pA->z > pB->z)
        return 1;
    return 0;
    }

/*---------------------------------------------------------------------------------**//**
@description Sort points along any direction with clear variation.
@param pXYZ IN OUT points to sort.
@param numXYZ IN number of points.
@group "DPoint3d Sorting"
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint3dArray_sortAlongAnyDirection
(
DPoint3d *pXYZ,
int      numXYZ
)
    {
    if (numXYZ < 2)
        return;
    DRange3d range;
    range.Init ();
    range.Extend (pXYZ, numXYZ);
    DVec3d diagonal;
    diagonal.DifferenceOf (range.high, range.low);

    if (diagonal.x >= diagonal.y && diagonal.x >= diagonal.z)
        qsort (pXYZ, numXYZ, sizeof (DPoint3d),
            (int (*)(const void *, const void *))compareX);
    else if (diagonal.y >= diagonal.z)
        qsort (pXYZ, numXYZ, sizeof (DPoint3d),
            (int (*)(const void *, const void *))compareY);
    else
        qsort (pXYZ, numXYZ, sizeof (DPoint3d),
            (int (*)(const void *, const void *))compareZ);
    }

/*-----------------------------------------------------------------*//**
* @description Compute a transformation which, if the points are coplanar in 3D, transforms all to the z=0 plane.
* @remarks Optionally returns post-transform range data so the caller can assess planarity.   If non-coplanar points are given,
    the plane will be chosen to pass through 3 widely separated points.   If the points are "close" to coplanar, the choice of
    "widely separated" will give an intuitively reasonable plane, but is not a formal "best" plane by any particular condition.
* @param pTransformedPoints OUT the points after transformation.  May be NULL.
* @param pWorldToPlane OUT transformation from world to plane.  May be NULL.
* @param pPlaneToWorld OUT transformation from plane to world.  May be NULL.
* @param pRange OUT range of the points in the transformed system.  May be NULL.
* @param pPoints IN pretransformed points
* @param numPoint IN number of points
* @return true if a plane was computed.  This does not guarantee that the points are coplanar.
    The false condition is for highly degenerate (colinear or single point) data, not
    an assessment of deviation from the proposed plane.
* @group "DPoint3d Modification"
* @bsihdr                                       EarlinLutz      08/02
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDPoint3dArray_transformToPlane

(
DPoint3dP pTransformedPoints,
TransformP pWorldToPlane,
TransformP pPlaneToWorld,
DRange3dP pRange,
DPoint3dCP pPoints,
int numPoint
)
    {
    Transform worldToPlane, planeToWorld;
    DVec3d normal;
    DPoint3d origin;
    DVec3d xVec, yVec, zVec;
    bool    boolstat = false;
    if (   !bsiGeom_planeThroughPoints (&normal, &origin, pPoints, numPoint)
        || !normal.GetNormalizedTriad(xVec, yVec, zVec)
       )
        {
        if (pWorldToPlane)
            pWorldToPlane->InitIdentity ();
        if (pPlaneToWorld)
            pPlaneToWorld->InitIdentity ();
        if (pRange)
            pRange->Init ();
        if (pTransformedPoints)
            bsiDPoint3d_copyArray (pTransformedPoints, pPoints, numPoint);
        }
    else
        {
        planeToWorld.InitFromOriginAndVectors(origin, xVec, yVec, zVec);
        worldToPlane.InvertRigidBodyTransformation (planeToWorld);

        if (pWorldToPlane)
            *pWorldToPlane = worldToPlane;
        if (pPlaneToWorld)
            *pPlaneToWorld = planeToWorld;

        if (pTransformedPoints)
            {
            worldToPlane.Multiply (pTransformedPoints, pPoints, numPoint);
            if (pRange)
                pRange->InitFrom(pTransformedPoints, numPoint);
            }
        else
            {
            // If range requested but no buffer given, have to do it one by one ourselves.
            if (pRange)
                {
                DPoint3d xyz;
                int i;

                pRange->Init ();
                for (i = 0; i < numPoint; i++)
                    {
                    worldToPlane.Multiply (xyz, pPoints[i]);
                    pRange->Extend (xyz);
                    }
                }
            }
        boolstat = true;
        }
    return boolstat;
    }

/*-----------------------------------------------------------------*//**
* @description Find the closest point in an array to the given point.
* @param pDist2 <= squared distance of closest point to test point (or NULL)
* @param pPointArray => point array
* @param n => number of points
* @param pTestPoint => point to test
* @return index of nearest point, or negative if n is nonpositive
* @group "DPoint3d Queries"
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int bsiGeom_nearestPointinDPoint3dArray

(
double          *pDist2,
DPoint3dCP pPointArray,
int              n,
DPoint3dCP pTestPoint
)
    {
    double minDist2, dist2;
    int    minIndex, i;
    if (n <= 0)
        {
        minDist2= 0.0;
        minIndex = -1;
        }
    else

        {
        minDist2 = pTestPoint->DistanceSquared (*pPointArray);
        minIndex = 0;
        for (i = 1; i < n; i++)
            {
            dist2 = pTestPoint->DistanceSquared (pPointArray[ i]);
            if (dist2 < minDist2)
                {
                minDist2 = dist2;
                minIndex = i;
                }
            }
        }
    if (pDist2)
        *pDist2 = minDist2;
    return minIndex;
    }

/*---------------------------------------------------------------------------------**//**
* @description Copy the given number of DPoint2d structures, setting all z-coordinates to zero.
* @param pDest <= destination array
* @param pSource => source array
* @param n => number of points
* @group "DPoint3d Copy"
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint3d_copyDPoint2dArray

(
DPoint3dP pDest,
DPoint2dCP pSource,
int          n
)
    {
    int i;
    for (i = 0; i < n ;i++)
        {
        pDest[i].x = pSource[i].x;
        pDest[i].y = pSource[i].y;
        pDest[i].z = 0.0;
        }
    }

/*-----------------------------------------------------------------*//**
* @description Test if an array of points is effectively a straight line from the first to the last.
* @param pOnLine <= true if all points are all within tolerance of the (bounded) line segment from the first point to the last point.
* @param pPointArray => array of points
* @param numPoint => number of points
* @param tolerance => absolute tolerance for distance test (or nonpositive to compute minimal tolerance)
* @return same as pOnLine
* @group "DPoint3d Queries"
* @bsihdr                                       DavidAssaf      12/98
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool        bsiGeom_pointArrayColinearTest

(
bool        *pOnLine,
const   DPoint3d    *pPointArray,
int         numPoint,
double      tolerance
)
    {
    DPoint3d    vector;
    double d01, s;
    int i;
    DPoint3d p0, p1, proj;
    static double relTol = 1.0e-12;
    double myTol = bsiDPoint3d_getLargestCoordinate (pPointArray, numPoint) * relTol;
    double myTol2;

    if (tolerance > myTol)
        myTol = tolerance;

    myTol2 = myTol * myTol;

    *pOnLine = false;

    if (numPoint < 2)
        return *pOnLine;

    p0 = pPointArray[0];
    p1 = pPointArray[numPoint - 1];
    vector.DifferenceOf (p1, p0);
    d01 = vector.Normalize ();

    if (d01 <= myTol)
        return *pOnLine;

    for (i = 1; i < numPoint - 1; i++)
        {
        s = pPointArray[i].DotDifference(p0, *((DVec3d*) &vector));
        proj.SumOf (p0, vector, s);
        if (proj.DistanceSquared (pPointArray[i]) > myTol2)
            return *pOnLine;
        if (s < 0.0)
            {
            /* projected before line segment's start */
            if (-s * d01 > myTol)
                return *pOnLine;
            }
        else if (s > d01)
            {
            /* projected after line segment's end */
            if ((s - d01) * d01 > myTol)
                return *pOnLine;
            }
        }
    *pOnLine = true;
    return *pOnLine;
    }


/*-----------------------------------------------------------------*//**
* @description Test if an array of points is effectively a straight line from the first to the last.
* @param pPointArray => array of points
* @param numPoint => number of points
* @param tolerance => absolute tolerance for distance test (or nonpositive to compute minimal tolerance)
* @return true if all points are within tolerance of the (bounded) line segment from the first point to the last point.
* @group "DPoint3d Queries"
* @bsihdr                                       DavidAssaf      12/98
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool        bsiGeom_isDPoint3dArrayColinear

(
const   DPoint3d    *pPointArray,
int         numPoint,
double      tolerance
)
    {
    bool    onLine;
    return bsiGeom_pointArrayColinearTest (&onLine, pPointArray, numPoint, tolerance);
    }

/*-----------------------------------------------------------------*//**
* Fill the affine part using xyz vectors for each row of the basis
* part and an xyz vector for the translation
*
* @instance pA <= matrix initialized as an identity
* @param pRow0 => data for row 0 of leading 3x3 submatrix
* @param pRow1 => data for row 1 of leading 3x3 submatrix
* @param pRow2 => data for row 2 of leading 3x3 submatrix
* @param pTranslation => data for translation part of matrix
* @see
* @indexVerb
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDMatrix4d_initAffineRows

(
DMatrix4dP pA,
DPoint3dCP pRow0,
DPoint3dCP pRow1,
DPoint3dCP pRow2,
DPoint3dCP pTranslation
)
    {
    pA->SetRow ( 0, pRow0->x, pRow0->y, pRow0->z, pTranslation->x );
    pA->SetRow ( 1, pRow1->x, pRow1->y, pRow1->z, pTranslation->y );
    pA->SetRow ( 2, pRow2->x, pRow2->y, pRow2->z, pTranslation->z );
    pA->SetRow ( 3, 0.0, 0.0, 0.0, 1.0 );
    }

/*-----------------------------------------------------------------*//**
* Fill the affine part using xyz vectors for each column of the basis
* part and an xyz vector for the translation

* @instance pA <= matrix initialized as an identity
* @param pCol0 => data for column 0 of leading 3x3 submatrix
* @param pCol1 => data for column 1 of leading 3x3 submatrix
* @param pCol2 => data for column 2 of leading 3x3 submatrix
* @param pTranslation => data for translation part of matrix
* @see
* @indexVerb
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDMatrix4d_initAffineColumns

(
DMatrix4dP pA,
DPoint3dCP pCol0,
DPoint3dCP pCol1,
DPoint3dCP pCol2,
DPoint3dCP pTranslation
)

    {
    pA->SetRow ( 0, pCol0->x, pCol1->x, pCol2->x, pTranslation->x );
    pA->SetRow ( 1, pCol0->y, pCol1->y, pCol2->y, pTranslation->y );
    pA->SetRow ( 2, pCol0->z, pCol1->z, pCol2->z, pTranslation->z );
    pA->SetRow ( 3, 0.0, 0.0, 0.0, 1.0 );
    }
/*-----------------------------------------------------------------*//**
* @param pEigenvectors <= matrix of eigenvectors.
* @param pEigenvalues  => eigenvalues corresponding to columns of the eigenvector matrix.
* @param pInstance      => matrix whose eigenvectors and eigenvalues are computed.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiRotMatrix_symmetricEigensystem
(
RotMatrixP pEigenvectors,
DPoint3dP pEigenvalues,
RotMatrixCP pInstance
)
    {
    RotMatrix workMatrix = *pInstance;
    int numIteration;
    bsiGeom_jacobi3X3 (
              (double*)pEigenvalues,
              (double (*)[3])pEigenvectors,
              &numIteration,
              (double (*)[3])&workMatrix
              );
        }
/*-----------------------------------------------------------------*//**
Test if a matrix is "just" a rotation around z (i.e. in the xy plane)
@param pMatrix => matrix to analyze.
@param pRadians <= angle in radians.  This angle is the direction of column 0
of the matrix.
@return false if there are any non-rotational effects, or rotation is around any other axis.
@group "RotMatrix Rotations"
 @bsimethod                                     EarlinLutz      02/02
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiRotMatrix_isXYRotation


(
RotMatrixCP pMatrix,
double  *pRadians
)
    {
    double radians = 0.0;
    bool    result = false;
    if (   pMatrix->IsOrthogonal ()
       &&  1 == bsiRotMatrix_summaryZEffects (pMatrix)
       &&   pMatrix->Determinant () > 0.0
       )
        {
        radians = atan2 (pMatrix->form3d[1][0], pMatrix->form3d[0][0]);
        result = true;
        }
    if (pRadians)
        *pRadians = radians;
    return result;
    }
/*-----------------------------------------------------------------*//**
 Approximate a coordinate frame through a set of points.
 The xy plane is determined by planeThroughPoints.
 The xy axes are arbitrary within that plane, and z is perpendicular.
 @param pTransform <= transformation
 @param pPoint => The point array
 @param numPoint => The number of points

 @return true if the points define a clear plane.
 @group "Transform Initialization"
 @bsimethod                                                       DavidAssaf      12/98
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     bsiTransform_initFromPlaneOfDPoint3dArray

(
TransformP pTransform,
bvector <DPoint3d> const &points)
    {
    DVec3d normal;
    DPoint3d origin;
    DVec3d vector0, vector1, vector2;
    bool    boolstat = false;
    if (points.size () > 0 && bsiGeom_planeThroughPoints (&normal, &origin, points.data (), (int)points.size ()))
        {
        boolstat = normal.GetNormalizedTriad (vector0, vector1, vector2);
        pTransform->InitFromOriginAndVectors(origin, vector0, vector1, vector2);
        }
    return boolstat;
    }
/*---------------------------------------------------------------------------------**//**
* @description Tests if two vectors are parallel.
* @remarks Use bsiTrig_smallAngle() for tolerance corresponding to bsiDPoint3d_areParallel.
*
* @param pVector1   IN      the first vector
* @param pVector2   IN      the second vector
* @param tolerance  IN      radian tolerance for angle between vectors
* @return true if the vectors are parallel within tolerance
* @bsimethod                                                    DavidAssaf      05/06
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     bsiDPoint3d_areParallelTolerance
(
DPoint3dCP  pVector1,
DPoint3dCP  pVector2,
double      tolerance
)
    {
    DPoint3d    cross;
    double      a2 = pVector1->DotProduct (*pVector1);
    double      b2 = pVector2->DotProduct (*pVector2);
    double      c2;

    cross.CrossProduct (*pVector1, *pVector2);

    c2 = cross.DotProduct (cross);

    /* a2,b2,c2 are squared lengths of respective vectors */
    /* c2 = sin^2(theta) * a2 * b2 */
    /* For small theta, sin^2(theta)~~theta^2 */
    /* Zero-length vector case falls through as equality */

    return c2 <= tolerance * tolerance * a2 * b2;
    }

/*---------------------------------------------------------------------------------**//**
* @description Append a DPoint3d to the end of the array.  The array count is increased
*       by one.
* @param pHeader    IN OUT  array to modify.
* @param pPoint     IN      DPoint3d to append to the array.
* @return true if operation is successful
* @group        "DPoint3d Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool      jmdlEmbeddedDPoint3dArray_addDPoint3d
(
        EmbeddedDPoint3dArray   *pHeader,
const   DPoint3d                *pPoint
)
    {
    if (pHeader)
        {
        pHeader->push_back (*pPoint);
        return true;
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @description Get a DPoint3d from a specified index in the array.
*
* @param pHeader    IN      header of array to access.
* @param pPoint     OUT     DPoint3d accessed from the array.
* @param index      IN      index of DPoint3d to access. Any negative index indicates
*                           highest numbered element in the array.
* @return false if the index is too large, i.e., no DPoint3d was accessed.
* @group        "DPoint3d Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool      jmdlEmbeddedDPoint3dArray_getDPoint3d
(
bvector<DPoint3d> const   *pHeader,
        DPoint3d                *pPoint,
        int                     index
)
    {
    size_t i = (size_t)index;
    if ( pHeader != nullptr
        && i < pHeader->size ())
        {
        *pPoint = pHeader->at(i);
        return true;
        }
    return false;
    }
/*---------------------------------------------------------------------------------**//**
* @description Return the number of DPoint3ds in the array.
*
* @param pHeader    IN      array to query.
* @return array count
* @group        "DPoint3d Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int   jmdlEmbeddedDPoint3dArray_getCount
(
const   EmbeddedDPoint3dArray   *pHeader
)
    {
    if (!pHeader)
        return 0;
    return (int) pHeader->size ();
    }
/*---------------------------------------------------------------------------------**//**
* @description Reduce the count (number of DPoint3ds) in the array to zero.
*       Existing memory is retained so the array can be refilled to its prior
*       size without requiring reallocation.
* @param pHeader    IN OUT  array to modify
* @group        "DPoint3d Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void  jmdlEmbeddedDPoint3dArray_empty
(
EmbeddedDPoint3dArray *pHeader
)
    {
    if (pHeader)
        pHeader->clear ();
    }

/*---------------------------------------------------------------------------------**//**
* @description Get a pointer to the contiguous buffer at specified index.  This pointer
*       may become invalid if array contents are altered.
* @param pHeader    IN      array to access.
* @param index      IN      index of array entry.  Any negative index indicates the final
*                           DPoint3d in the array.
* @return pointer to contiguous buffer (simple C array).
* @group        "DPoint3d Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP DPoint3d* jmdlEmbeddedDPoint3dArray_getPtr
(
EmbeddedDPoint3dArray   *pHeader,
int                     index
)
    {
    return DPoint3dArrayWrapper::getPtr (pHeader, index);
    }

/*---------------------------------------------------------------------------------**//**
* @description Grab (borrow) an array from the cache.  Caller is responsible
*       for using ~mEmbeddedDPoint3dArray_drop to return the array to the cache when
*       finished.   Controlled "grab and drop" of cache arrays is faster than using
*       either local variables (~mEmbeddedDPoint3dArray_init and
*       ~mEmbeddedDPoint3dArray_releaseMem) or heap allocation
*       (~mEmbeddedDPoint3dArray_new and ~mEmbeddedDPoint3dArray_free)
*       because the preallocated variable size parts of cached arrays are immediately
*       available without revisiting the system cache.
* @return An array header obtained from the cache.
* @group        "DPoint3d Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP EmbeddedDPoint3dArray *jmdlEmbeddedDPoint3dArray_grab
(
void
)
    {
    return new EmbeddedDPoint3dArray ();
    }


/*---------------------------------------------------------------------------------**//**
* @description Drop (return) an array to the cache.  Use this to dispose of arrays
*       borrowed with ~mEmbeddedDPoint3dArray_grab.
* @param pHeader    IN      pointer to array to return to cache.
* @return always returns NULL.
* @group        "DPoint3d Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP EmbeddedDPoint3dArray *jmdlEmbeddedDPoint3dArray_drop
(
EmbeddedDPoint3dArray     *pHeader
)
    {
    if (pHeader)
        delete pHeader;
    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @description Append an array of DPoint3d to the end of the array.
*
* @param pHeader    IN OUT  header of array receiving values
* @param pPoint     IN      array of data to add
* @param n          IN      number to add.
* @return true if operation is successful
* @group        "DPoint3d Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool      jmdlEmbeddedDPoint3dArray_addDPoint3dArray
(
        EmbeddedDPoint3dArray   *pHeader,
const   DPoint3d                *pPoint,
        int                     n
)
    {
    return pHeader != NULL
            && SUCCESS == DPoint3dArrayWrapper::insert (pHeader, pPoint, -1, n);
    }
/*---------------------------------------------------------------------------------**//**
* @description Insert at a specified position, shifting others to higher
*       positions as needed.
* @param pHeader    IN OUT  array to modify.
* @param pPoint     IN      data to insert.
* @param index      IN      index at which the value is to appear in the array.
*                           The special index -1 (negative one) indicates to
*                           insert at the end of the array.
* @return true if operation is successful
* @group        "DPoint3d Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool      jmdlEmbeddedDPoint3dArray_insertDPoint3d
(
        EmbeddedDPoint3dArray   *pHeader,
const   DPoint3d                *pPoint,
        int                     index
)
    {
    return SUCCESS == DPoint3dArrayWrapper::insert (pHeader, pPoint, index);
    }

/*---------------------------------------------------------------------------------**//**
* @description Store a DPoint3d in the array at the specified index.
*
* @param pHeader    IN OUT  array to modify.
* @param pPoint     IN      DPoint3d to store.
* @param index      IN      position where the DPoint3d is stored.  A negative
*                           indicates replacement of the current final DPoint3d.  If the
*                           index is beyond the final current DPoint3d, zeros are
*                           inserted to fill to the new index.
* @return false if the index required array expansion and the reallocation failed.
* @group        "DPoint3d Array"
* @bsimethod                                    EarlinLutz      01/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool      jmdlEmbeddedDPoint3dArray_setDPoint3d
(
EmbeddedDPoint3dArray   *pHeader,
DPoint3dCP              pPoint,
int                     index
)
    {
    return pHeader != NULL
        && SUCCESS == DPoint3dArrayWrapper::set (pHeader, pPoint, index);
    }
END_BENTLEY_GEOMETRY_NAMESPACE
