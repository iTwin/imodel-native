#include <msgeomstructs.h>
#include <math.h>
#include <float.h>
#include <limits.h>

#define PANIC_SIZE (1.0e100)

#define FIX_MIN(value, min)          if (value < min) min = value
#define FIX_MAX(value, max)          if (value > max) max = value
#define FIX_MINMAX(value, min, max)  FIX_MIN(value, min); FIX_MAX(value, max);

//BEGIN FUNCTION bsiGraphicsPoint_initFromDPoint4d
/*-----------------------------------------------------------------*//**
* @struct DPoint2d
* A GraphicsPoint is an 4d (homogeneous) point with an additional double
*   and 2 integers for labeling use during graphics operations.
* @fields
* @field DPoint4d point = homogeneous point
* @field double a = double label field.
* @field        mask = integer mask used by GraphicsPointArray.
* @field    userData =  integer label field.
* @endfields
* @bsistruct                                        EarlinLutz      09/00
+----------------------------------------------------------------------*//* METHOD(GraphicsPoint,none,init) */
/*---------------------------------------------------------------------------------**//**
* Initialize a graphics point from complete data
*
* @instance pInstance <= graphics point to initialize
* @param pPoint => point to add
* @param a      => floating point label value
* @param mask   => mask value
* @param userData => user data labal
* @indexVerb init
* @bsihdr                                                      EarlinLutz      12/97
+---------------+---------------+---------------+---------------+---------------+------*/
Public void bsiGraphicsPoint_initFromDPoint4d
(
        GraphicsPoint  *pInstance,
const   DPoint4d        *pPoint,
        double          a,
        int             mask,
        int             userData
)
    {
    pInstance->point = *pPoint;
    pInstance->mask = mask;
    pInstance->a = a;
    pInstance->userData = userData;
    }

//END
//BEGIN FUNCTION bsiGraphicsPoint_initFromDPoint3d
/* METHOD(GraphicsPoint,none,init) */
/*---------------------------------------------------------------------------------**//**
* Initialize a graphics point from complete data
*
* @instance pInstance <= graphics point to initialize
* @param pPoint => point to add
* @param a      => floating point label value
* @param mask   => mask value
* @param userData => user data labal
* @indexVerb init
* @bsihdr                                                      EarlinLutz      12/97
+---------------+---------------+---------------+---------------+---------------+------*/
Public void bsiGraphicsPoint_initFromDPoint3d
(
        GraphicsPoint  *pInstance,
const   DPoint3d        *pPoint,
        double          weight,
        double          a,
        int             mask,
        int             userData
)
    {
    pInstance->point.x = pPoint->x;
    pInstance->point.y = pPoint->y;
    pInstance->point.z = pPoint->z;
    pInstance->point.w = weight;

    pInstance->a = a;
    pInstance->mask = mask;
    pInstance->userData = userData;
    }

//END
//BEGIN FUNCTION bsiGraphicsPoint_init
/* METHOD(GraphicsPoint,none,init) */
/*---------------------------------------------------------------------------------**//**
* Initialize a graphics point from complete data
*
* @instance pInstance <= graphics point to initialize
* @param pPoint => point to add
* @param a      => floating point label value
* @param mask   => mask value
* @param userData => user data labal
* @indexVerb init
* @bsihdr                                                      EarlinLutz      12/97
+---------------+---------------+---------------+---------------+---------------+------*/
Public void bsiGraphicsPoint_init
(
        GraphicsPoint  *pInstance,
        double          x,
        double          y,
        double          z,
        double          w,
        double          a,
        int             mask,
        int             userData
)
    {
    pInstance->point.x = x;
    pInstance->point.y = y;
    pInstance->point.z = z;
    pInstance->point.w = w;

    pInstance->mask = mask;
    pInstance->a = a;
    pInstance->userData = userData;
    }

//END
//BEGIN FUNCTION bsiGraphicsPoint_setInOutArray
/* METHOD(GraphicsPoint,setInOutArray,none) */
/*-----------------------------------------------------------------*//**
* Sets the in and out bits of the mask.
* @param bIn  => in bit setting.  Any nonzero sets it.
* @param bOut => out bit setting. Any nonzero sets it.
* @param returns the exact bits set in the point.  Calling with NULL point
*               is a way to convert generic bool    classifitions to
*               exact bits for the mask.
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public int bsiGraphicsPoint_setInOutArray
(
GraphicsPoint   *pGP,
int             numGP,
bool            bIn,
bool            bOut
)
    {
    int mask = 0;
    int mask0 = ~(HPOINT_MASK_INOUT_BIT_IN | HPOINT_MASK_INOUT_BIT_OUT);
    int i;
    if (bIn)
        mask |= HPOINT_MASK_INOUT_BIT_IN;
    if (bOut)
        mask |= HPOINT_MASK_INOUT_BIT_OUT;
    if (pGP)
        {
        for (i = 0; i < numGP; i++)
            {
            pGP[i].mask &= mask0;
            pGP[i].mask |= mask;
            }
        }
    return mask;
    }

//END
//BEGIN FUNCTION bsiDPoint2d_crossProduct
/* VBSUB(Point2dCrossProduct) */
/* METHOD(DPoint2d,none,crossProduct) */
/*-----------------------------------------------------------------*//**
* @description Returns the (scalar) cross product of two vectors.
* @instance pCrossProduct <= cross product of vector1 and vector2
* @param pVector1 => first vector
* @param pVector2 => second vector
* @indexVerb crossProduct
* @bsihdr                                                              EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public double bsiDPoint2d_crossProduct
(
const DPoint2d  *pVector0,
const DPoint2d  *pVector1
)

    {
    return pVector0->x * pVector1->y - pVector0->y * pVector1->x;
    }


//END
//BEGIN FUNCTION bsiDPoint2d_dotProduct
/* VBSUB(Point2dDotProduct) */
/* METHOD(DPoint2d,none,dotProduct) */
/*-----------------------------------------------------------------*//**
* @description Returns the (scalar) dot product of two vectors.
* @instance pVector1 => first vector.
* @param pVector2 => second vector.
* @return dot product of the two vectors
* @indexVerb dotProduct
* @bsihdr                                                              EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public double bsiDPoint2d_dotProduct
(
const DPoint2d  *pVector1,
const DPoint2d  *pVector2
)
    {
    return pVector1->x * pVector2->x + pVector1->y * pVector2->y;
    }


//END
//BEGIN FUNCTION bsiDPoint2d_distanceSquared
/* VBSUB(Point2dDistanceSquared) */
/* METHOD(DPoint2d,none,distanceSquared) */
/*-----------------------------------------------------------------*//**
* @description Return the squared distance between two points or vectors.
* @instance pPoint1 => start point
* @param pPoint2 => end point
* @return squared distance between points
* @indexVerb distance squared
* @bsihdr                                                              EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public double bsiDPoint2d_distanceSquared
(
const DPoint2d  *pPoint1,
const DPoint2d  *pPoint2
)

    {
    double dx = pPoint2->x - pPoint1->x;
    double dy = pPoint2->y - pPoint1->y;
    return dx * dx + dy * dy;
    }

//END
//BEGIN FUNCTION bsiDPoint3d_zero
/* VBSUB(Point3dZero) */
/* METHOD(DPoint3d,none,zero) */
/*-----------------------------------------------------------------*//**
 @description Sets all components of a point or vector to zero.
 @instance pPoint <= The zeroed vector.
@group "DPoint3d Initialization"
 @bsimethod                                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public void bsiDPoint3d_zero
(
DPoint3d        *pPoint
)
    {
    pPoint->x = 0.0;
    pPoint->y = 0.0;
    pPoint->z = 0.0;
    }

//END
//BEGIN FUNCTION bsiDPoint3d_setXYZ
/* VBSUB(Point3dFromXYZ) */
/* CSVFUNC(fromXYZ) */
/* METHOD(DPoint3d,none,init) */
/*-----------------------------------------------------------------*//**
 @description Sets the x,y, and z components of a point

 @instance pPoint <= Point or vector whose componts are set.
 @param ax => The x component.
 @param ay => The y component.
 @param az => The z component.
@group "DPoint3d Initialization"
 @bsimethod                                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public void bsiDPoint3d_setXYZ
(
DPoint3d    *pPoint,
double       ax,
double       ay,
double       az
)
    {
    pPoint->x = ax;
    pPoint->y = ay;
    pPoint->z = az;
    }

//END
//BEGIN FUNCTION bsiDPoint3d_interpolate
/* VBSUB(Point3dInterpolate) */
/* CSVFUNC(interpolate) */
/* METHOD(DPoint3d,none,interpolate) */
/*-----------------------------------------------------------------*//**
 @description Computes a point whose position is given by a fractional
 argument and two endpoints.

 @instance pSum <= The interpolated point.
 @param pPoint0 => The point corresponding to fractionParameter of 0.
 @param fractionParameter => The fractional parametric coordinate.
                0.0 is the start of the segment, 1.0 is the end, 0.5 is midpoint.
 @param pPoint1 => The point corresponding to fractionParameter of 1.
@group "DPoint3d Addition"
 @bsimethod                                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public void bsiDPoint3d_interpolate
(
        DPoint3d        *pSum,
const   DPoint3d        *pPoint0,
        double           fractionParameter,
const   DPoint3d        *pPoint1
)
    {
    pSum->x = pPoint0->x + fractionParameter * (pPoint1->x - pPoint0->x);
    pSum->y = pPoint0->y + fractionParameter * (pPoint1->y - pPoint0->y);
    pSum->z = pPoint0->z + fractionParameter * (pPoint1->z - pPoint0->z);
    }

//END
//BEGIN FUNCTION bsiDPoint3d_distanceSquared
/* VBSUB(Point3dDistanceSquared) */
/* CSVFUNC(distanceSquared) */
/* METHOD(DPoint3d,none,distanceSquared) */
/*-----------------------------------------------------------------*//**
 @description Computes the squared distance between two points.

 @instance pPoint1 => The first point.
 @param pPoint2 => The second point.
 @return The squared distance between the points.
@group "DPoint3d Distance"
 @bsimethod                                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public double bsiDPoint3d_distanceSquared
(
const   DPoint3d        *pPoint1,
const   DPoint3d        *pPoint2
)
    {
    double      xdist, ydist, zdist;

    xdist = (pPoint2->x - pPoint1->x);
    ydist = (pPoint2->y - pPoint1->y);
    zdist = (pPoint2->z - pPoint1->z);

    return (xdist*xdist + ydist*ydist + zdist*zdist);
    }

//END
//BEGIN FUNCTION bsiDPoint3d_distanceSquaredXY
/* VBSUB(Point3dDistanceSquaredXY) */
/* CSVFUNC(distanceSquaredXY) */
/* METHOD(DPoint3d,none,distanceSquaredXY) */
/*-----------------------------------------------------------------*//**
 @description Computes the squared distance between two points, using only the
       xy parts.

 @instance pPoint1 => The first point
 @param pPoint2 => The second point
 @return The squared distance between the XY projections of the two points.
                (i.e. any z difference is ignored)
@group "DPoint3d Distance"
 @bsimethod                                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public double bsiDPoint3d_distanceSquaredXY
(
const   DPoint3d        *pPoint1,
const   DPoint3d        *pPoint2
)
    {
    double      xdist, ydist;

    xdist = pPoint2->x - pPoint1->x;
    ydist = pPoint2->y - pPoint1->y;

    return (xdist*xdist + ydist*ydist);
    }

//END
//BEGIN FUNCTION bsiDPoint3d_distanceXY
/* VBSUB(Point3dDistanceXY) */
/* CSVFUNC(distanceXY) */
/* METHOD(DPoint3d,none,distanceXY) */
/*-----------------------------------------------------------------*//**
 @description Computes the distance between two points, using
   only x and y components.

 @instance pPoint1 => The first point
 @param pPoint2 => The second point
 @return The distance between the XY projections of the two points.
                (i.e. any z difference is ignored)
@group "DPoint3d Distance"
 @bsimethod                                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public double bsiDPoint3d_distanceXY
(
const   DPoint3d        *pPoint1,
const   DPoint3d        *pPoint2
)
    {
    double      xdist, ydist;

    xdist = pPoint2->x - pPoint1->x;
    ydist = pPoint2->y - pPoint1->y;

    return sqrt (xdist*xdist + ydist*ydist);
    }

//END
//BEGIN FUNCTION bsiDPoint3d_isDisconnect
/* METHOD(DPoint3d,none,isDisconnect) */
/*-----------------------------------------------------------------*//**
 @instance pPoint => point to test.
 @return true if the point has coordinates which indicate it is
   a disconnect (separator) ponit.
@group "DPoint3d Queries"
 @bsimethod                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public bool    bsiDPoint3d_isDisconnect
(
const DPoint3d *pPoint
)
    {
    return pPoint->x == DISCONNECT
        || pPoint->y == DISCONNECT
        || pPoint->z == DISCONNECT;
    }

//END
//BEGIN FUNCTION bsiDPoint4d_copyAndWeight
/* METHOD(DPoint4d,none,initFrom) */
/*-----------------------------------------------------------------*//**
* Initializ a homogeneous point from a 3D point and separate weight.
* NOTE The xyz components copied unchanged, i.e. not multiplied by the
* weight.
*
* @instance pInstance <= homogeneous point
* @param pPoint => cartesian point
* @param w => weight component
* @indexVerb init
* @indexVerb copy
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public void bsiDPoint4d_copyAndWeight
(
      DPoint4d  *pInstance,
const DPoint3d  *pPoint,
      double    w
)
    {
    pInstance->x = pPoint->x;
    pInstance->y = pPoint->y;
    pInstance->z = pPoint->z;
    pInstance->w = w;
    }


//END
//BEGIN FUNCTION bsiDPoint4d_planeFromOriginAndNormal
/* METHOD(DPoint4d,none,planeFromOriginAndNormal) */
/*-----------------------------------------------------------------*//**
* Computes the homogeneous vector for a plane defined by 3D origin
* and normal.
* NOTE If the normal vector is null, a 0000 vector is returned.
*
* @instance pInstance <= homogeneous vector for plane
* @param pOrigin <= origin point
* @param pNormal <= normal vector
* @return true unless normal is null
* @indexVerb init
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public bool    bsiDPoint4d_planeFromOriginAndNormal
(
      DPoint4d  *pInstance,
const DPoint3d  *pOrigin,
const DPoint3d  *pNormal
)

    {
    DPoint3d unitNormal;
    bool    result;
    double d;

    unitNormal = *pNormal;
    if (bsiDPoint3d_normalizeInPlace(&unitNormal) > 0.0)
        {
        d = bsiDPoint3d_dotProduct (&unitNormal, pOrigin);
        bsiDPoint4d_copyAndWeight( pInstance, &unitNormal, -d );
        result = true;
        }
    else
        {
        pInstance->x = pInstance->y = pInstance->z = pInstance->w = 0.0;
        result = false;
        }
    return result;
    }


//END
//BEGIN FUNCTION bsiDRange3d_init
/*-----------------------------------------------------------------*//**
 @description DRange3d
 A DRange3d structure contains 2 points for use as min (low) and max (high)
 corners of a bounding box.

 For a non-null bounding box, each component (x,y,z) of the min point is
 strictly less than the corresponding component of the max point.


 The base init function DRange3d#init() initializes the range to large
 values with max less than min.  This null state can be identified as a
 null box later on.

 Methods named DRange3d#initFrom(...) initialize the box around
 various combinations of points.  (E.g. one point, two points, 3 points,
 array of points.)

 Methods named DRange3d#extend(..) expand an existing box to include additional
 range. (E.g. one point, array of points, components of point)

 @author   EarlinLutz
 @group DRange3d
 @bsimethod                                               EarlinLutz      12/97
+===============+===============+===============+===============+======*/
/* VBSUB(Range3dInit) */
/* METHOD(DRange3d,none,init) */
/*-----------------------------------------------------------------*//**
 @description Initializes a range cube with (inverted) large positive and negative
 values.
 @instance pRange OUT the initialized range.
 @group "DRange3d Initialization"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public void bsiDRange3d_init
(
DRange3d        *pRange
)

    {
    pRange->low.x = pRange->low.y = pRange->low.z =  DBL_MAX;
    pRange->high.x = pRange->high.y = pRange->high.z = -DBL_MAX;
    }

//END
//BEGIN FUNCTION bsiDRange3d_isNull
/* VBSUB(Range3dIsNull) */
/* METHOD(DRange3d,none,isNull) */
/*-----------------------------------------------------------------*//**
 @description Check if the range is exactly the same as the null ranges of a just-initialized
 range.
 @instance pRange IN range to test.
 @return true if the range is null.
 @group "DRange3d Queries"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public bool    bsiDRange3d_isNull
(
const   DRange3d        *pRange
)
    {
    if (
           pRange->low.x == DBL_MAX
        && pRange->low.y == DBL_MAX
        && pRange->low.z ==  DBL_MAX
        && pRange->high.x == -DBL_MAX
        && pRange->high.y == -DBL_MAX
        && pRange->high.z == -DBL_MAX
        )
        {
        return true;
        }
    else if (
           fabs(pRange->low.x) < PANIC_SIZE
        && fabs(pRange->low.y) < PANIC_SIZE
        && fabs(pRange->low.z) < PANIC_SIZE
        && fabs(pRange->high.x) < PANIC_SIZE
        && fabs(pRange->high.y) < PANIC_SIZE
        && fabs(pRange->high.z) < PANIC_SIZE
        )
        {
        return false;
        }
    else
        {
        /* It doesn't match the definition of a null, but it is definitely not normal.
         This is a good place for a breakpoint!!*/
//      throwException ("BadRange");
        return true;
        }
    }

//END
//BEGIN FUNCTION bsiDRange3d_initFrom2Points
/* VBSUB(Range3dFromPoint3dPoint3d) */
/* METHOD(DRange3d,none,initFrom) */
/*-----------------------------------------------------------------*//**
 @description Initializes the range to contain the two given points.
 @param pRange OUT initialized range.
 @param pPoint0 IN first point
 @param pPoint1 IN second point
 @group "DRange3d Initialization"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public void bsiDRange3d_initFrom2Points
(
        DRange3d        *pRange,
const   DPoint3d        *pPoint0,
const   DPoint3d        *pPoint1
)


    {
    pRange->low = pRange->high = *pPoint0;

    FIX_MINMAX (pPoint1->x, pRange->low.x, pRange->high.x);
    FIX_MINMAX (pPoint1->y, pRange->low.y, pRange->high.y);
    FIX_MINMAX (pPoint1->z, pRange->low.z, pRange->high.z);
    }
//END
//BEGIN FUNCTION bsiDRange3d_initFromArray
/* METHOD(DRange3d,none,initFrom) */
/*-----------------------------------------------------------------*//**
 @description Initializes the range to contain the range of the given array of points.
 If there are no points in the array, an empty initialized range is returned.

 @param pRange OUT initialized range
 @param pPoint => array of points to search
 @param n => number of points in array
 @group "DRange3d Initialization"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public void bsiDRange3d_initFromArray
(
        DRange3d        *pRange,
const   DPoint3d        *pPoint,
        int             n
)

    {
    int i;
    DPoint3d *  minP = &pRange->low;
    DPoint3d *  maxP = &pRange->high;
    if (n < 1)
        {
        bsiDRange3d_init (pRange);
        }
    else
        {
        *minP = *maxP = pPoint[0];
        for (i=1; i<n; i++)
            {
            if (!bsiDPoint3d_isDisconnect (&pPoint[i]))
                {
                FIX_MINMAX ( pPoint[i].x, minP->x, maxP->x );
                FIX_MINMAX ( pPoint[i].y, minP->y, maxP->y );
                FIX_MINMAX ( pPoint[i].z, minP->z, maxP->z );
                }
            }
        }
    }


//END
//BEGIN FUNCTION bsiDRange3d_extendByDistance
/* METHOD(DRange3d,none,extend) */
/*-----------------------------------------------------------------*//**
 @description Extend each axis by the given distance on both ends of the range.
 @instance pRange IN OUT updated range
 @param extend IN distance to extend
 @group "DRange3d Extend"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public void bsiDRange3d_extendByDistance
(
DRange3d        *pRange,
double           extend
)

    {
    if (!bsiDRange3d_isNull (pRange))
        {
        pRange->low.x -= extend;
        pRange->low.y -= extend;
        pRange->low.z -= extend;

        pRange->high.x += extend;
        pRange->high.y += extend;
        pRange->high.z += extend;
        }
    }

//END
//BEGIN FUNCTION bsiDRange3d_getLargestCoordinate
/* METHOD(DRange3d,none,largestCoordinate) */
/*-----------------------------------------------------------------*//**
 @instance pRange IN range to query
 @return the largest individual coordinate value among (a) range min point,
 (b) range max point, and (c) range diagonal vector.
 @group "DRange3d Queries"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public double bsiDRange3d_getLargestCoordinate
(
const DRange3d  *pRange
)

    {
     double     max;
     DPoint3d diagonal;
     if (bsiDRange3d_isNull (pRange))
            return 0.0;

     max = fabs(pRange->low.x);
     FIX_MAX(fabs(pRange->high.x), max);
     FIX_MAX(fabs(pRange->low.y), max);
     FIX_MAX(fabs(pRange->high.y), max);
     FIX_MAX(fabs(pRange->low.z), max);
     FIX_MAX(fabs(pRange->high.z), max);

     bsiDPoint3d_subtractDPoint3dDPoint3d(&diagonal, &pRange->high, &pRange->low);

     FIX_MAX(fabs(diagonal.x), max);
     FIX_MAX(fabs(diagonal.y), max);
     FIX_MAX(fabs(diagonal.z), max);

     return max;

    }


//END
//BEGIN FUNCTION bsiDTransform3d_multiplyComponents
/* VBSUB(Point3dFromTransform3dTimesXYZ) */
/* CSVFUNC(multiply) */
/* METHOD(DTransform3d,none,multiply) */
/*-----------------------------------------------------------------*//**
 @description Returns the product of a matrix times a point, with the point
       specified as explict x, y, and z values.
 @instance pTransform => The transformation to apply
 @param pPoint <= result of transformation * point operation
 @param x => The x component of the point
 @param y => The y component of the point
 @param z => The z component of the point
 @group "DTransform3d Multiplication"
 @bsimethod                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public void bsiDTransform3d_multiplyComponents
(
const DTransform3d  *pTransform,
      DPoint3d      *pPoint,
      double         x,
      double         y,
      double         z
)
    {
    pPoint->x =   pTransform->translation.x
                + pTransform->matrix.column[0].x * x
                + pTransform->matrix.column[1].x * y
                + pTransform->matrix.column[2].x * z;

    pPoint->y =   pTransform->translation.y
                + pTransform->matrix.column[0].y * x
                + pTransform->matrix.column[1].y * y
                + pTransform->matrix.column[2].y * z;

    pPoint->z =   pTransform->translation.z
                + pTransform->matrix.column[0].z * x
                + pTransform->matrix.column[1].z * y
                + pTransform->matrix.column[2].z * z;
    }



//END
//BEGIN FUNCTION bsiTrig_safeDivide
/* METHOD(Geom,safeDivide,none) */
/*-----------------------------------------------------------------*//**
* Try to divide numerator by denominator.  If denominator is near zero,
*   return false and set the result to a default value.
* @param pRatio <= numerator / denominator if safe, defaultRatio otherwise
* @param numerator => numerator of ratio
* @param denominator => denominator of ratio
* @param defaultRatio => default ratio if b is small
* @return true if division is numerically safe.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public bool    bsiTrig_safeDivide
(
double  *pRatio,
double  numerator,
double  denominator,
double  defaultRatio
)
    {
    static double s_relTol = 1.0e-12;
    if (fabs (denominator) > s_relTol * fabs (numerator))
        {
        *pRatio = numerator / denominator;
        return true;
        }
    else
        {
        *pRatio = defaultRatio;
        return false;
        }
    }

//END
//BEGIN FUNCTION bsiTrig_atan2
/* METHOD(Geom,atan2,none) */
/*-----------------------------------------------------------------*//**
* Wrap the system atan2 with checks for (0,0) arguments.
*
* @param y => numerator
* @param x => denominator
* @return arctan of y/x.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public double bsiTrig_atan2
(
double  y,
double  x
)

    {
    if (x == 0.0 && y == 0.0)
        return 0.0;
    else
        return atan2(y,x);
    }


//END
//BEGIN FUNCTION bsiGeom_cyclic3dAxis
/* METHOD(Geom,cyclic3dAxis,none) */
/*-----------------------------------------------------------------*//**
*
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public int bsiGeom_cyclic3dAxis
(
int i
)
    {
    if  (0 <= i)
        {
        if  (i < 3)
            {
            return  i;
            }
        else
            return  i % 3;
        }
    else
        {
        if  (i > -3)
            {
            return  3 + i;
            }
        else
            {
            return  2 - ((-1 - i) % 3);
            }
        }
    }

//END
//BEGIN FUNCTION bsiTrig_normalizeToPeriod
/*---------------------------------------------------------------------------------**//**
* Normalize an value to a period with specified base value.
* (Typically used with angles, but period is given as an arg so it might be other than 2pi)
*
* @bsimethod                                                    EarlinLutz      08/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public double bsiTrig_normalizeToPeriod
(
double a,
double a0,
double da
)
    {
    double a1 = a0 + da;
    double b;
    int ib;
    /* Short circuit the common case where a is already in period.
        (If da is negative, this should always fail.) */
    if (a >= a0 && a < a1)
        {
        /* Already in period.  Leave it alone. */
        return a;
        }
    else if (da < 0.0)
        {
        /* Recurse on negated coordinates */
        return - bsiTrig_normalizeToPeriod (-a, -a0, -da);
        }
    else if (da == 0.0)
        {
        /* Invalid period. Leave a alone. */
        return a;
        }
    else if (a < a0)
        {
        b = a + da;
        /* Quick out for adjacent period ... */
        if (b > a0)
            return b;
        /* Shift and recurse for far regions.  We expect the shift to be right, but
            let the recursion correct it if there is bit-level error. */
        b = (a0 - a) / da;
        ib = 1 + (int)b;
        return bsiTrig_normalizeToPeriod (a + ib * da, a0, da);
        }
    else    /* a >= a1 */
        {
        b = a - da;
        /* Quick out for adjacent period ... */
        if (b < a1)
            return b;
        /* Shift and recurse for far regions.  We expect the shift to be right, but
            let the recursion correct it if there is bit-level error. */
        b = (a - a0) / da;
        ib =(int)b;
        return bsiTrig_normalizeToPeriod (a - ib * da, a0, da);
        }
    }

//END
//BEGIN FUNCTION bsiTrig_normalizeToPeriodAroundZero
/*---------------------------------------------------------------------------------**//**
* @description normalize to a period centered around zero.  Shift by a multiple
*   of fullPeriod to bring to between negative and positive half period.
* @param a IN periodic value
* @param fullPeriod IN the full period.
* @bsimethod                                                    EarlinLutz      08/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public double bsiTrig_normalizeToPeriodAroundZero
(
double a,
double fullPeriod
)
    {
    double halfPeriod;
    if (fullPeriod == 0.0)
        return a;
    halfPeriod = 0.5 * fullPeriod;
    if (fabs (a) < halfPeriod)
        return a;
    return bsiTrig_normalizeToPeriod (a, -halfPeriod, fullPeriod);
    }
//END
//BEGIN FUNCTION bsiDPoint3d_getLargestXYCoordinate
/* METHOD(DPoint3d,largestXYCoordinate,none) */
/*-----------------------------------------------------------------*//**
* Returns an upper bound for both the largest absolute value x or y coordinate
* and the greatest distance between any two x or y coordinates in an array
* of points.
*
* @param pPointArray => array of points to test
* @param numPoint => number of points > 0
* @see
* @return double
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public double bsiDPoint3d_getLargestXYCoordinate
(
const   DPoint3d    *pPointArray,
        int         numPoint
)
    {
    DRange3d tmpRange;
    bsiDRange3d_initFromArray(&tmpRange, pPointArray, numPoint);
    tmpRange.low.z = tmpRange.high.z = 0.0;
    return (bsiDRange3d_getLargestCoordinate(&tmpRange));
    }
//END
//BEGIN FUNCTION bsiSVD_solve2x2_go
/*-----------------------------------------------------------------*//**
* Solve a 2x2 linear system -- unconditional svd.
* Solution process uses the SVD (Singular Value Decomposition)
* to compute the matrix determinant.   This is pricey (2 square roots)
* compared to a simple Cramer's rule, but provides a clearer test
* for nearzero determinant.
* Explicit SVD formulas from Blinn, Jim "Consider the lowly 2x2 Matrix",
*   IEEE Computer Graphics and Applications, March 1996, 8288.
* (This is a beautiful paper  READ IT!!!!)
*
* @param pX0 <= first solution parameter
* @param pX1 <= second solution parameter
* @param a00 => 00 coefficient of matrix
* @param a01 => 01 coefficient of matrix
* @param a10 => 10 coefficient of matrix
* @param a11 => 11 coefficient of matrix
* @param b0 => 0 coefficient on RHS
* @param b1 => 1 coefficient on RHS
* @see
* @return true iff the system is invertible
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
static bool    bsiSVD_solve2x2_go
(
double      *pX0,
double      *pX1,
double      a00,
double      a01,
double      a10,
double      a11,
double      b0,
double      b1
)
    {
    double Ebar = a00 + a11;
    double Fbar = a00 - a11;
    double Gbar = a01 + a10;
    double Hbar = a01 - a10;
    double rootEH = 0.5 * sqrt (Ebar * Ebar + Hbar * Hbar);
    double rootFG = 0.5 * sqrt (Fbar * Fbar + Gbar * Gbar);

    double wLarge = rootEH + rootFG;    /* SUM of two sqrts, cannot be negative */
    double wSmall = rootEH - rootFG;    /* DIFFERENCE of same, absolute value
                                            cannot exceed prior sum. */

    static double relTol = 1.0e-12;

    bool    result;

    if (fabs(wSmall) <= relTol * wLarge)
        {
        *pX0 = *pX1 = 0;
        result = false;
        }
    else
        {
        double determinant = wLarge * wSmall;   /* We know that neither of these
                                                    is zero */
        double det0 = b0 * a11 - b1 * a01;
        double det1 = a00 * b1 - a10 * b0;
        *pX0 = det0 / determinant;
        *pX1 = det1 / determinant;
        result = true;
        }
    return result;
    }

//END
//BEGIN FUNCTION bsiSVD_solve2x2
/* METHOD(Geom,solve2x2,none) */
/*-----------------------------------------------------------------*//**
* Solve a 2x2 linear system..
* Solution process uses the SVD (Singular Value Decomposition)
* to compute the matrix determinant.   This is pricey (2 square roots)
* compared to a simple Cramer's rule, but provides a clearer test
* for nearzero determinant.
* Explicit SVD formulas from Blinn, Jim "Consider the lowly 2x2 Matrix",
*   IEEE Computer Graphics and Applications, March 1996, 8288.
* (This is a beautiful paper  READ IT!!!!)
*
* @param pX0 <= first solution parameter
* @param pX1 <= second solution parameter
* @param a00 => 00 coefficient of matrix
* @param a01 => 01 coefficient of matrix
* @param a10 => 10 coefficient of matrix
* @param a11 => 11 coefficient of matrix
* @param b0 => 0 coefficient on RHS
* @param b1 => 1 coefficient on RHS
* @see
* @return true iff the system is invertible
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public bool    bsiSVD_solve2x2
(
double      *pX0,
double      *pX1,
double      a00,
double      a01,
double      a10,
double      a11,
double      b0,
double      b1
)
    {
    /* We all believe (or do we) that SVD is the most stable way to solve a system.
       Maybe this is true for badly conditioned systems.
       However, for a diagonal system with large scale difference between entries, we observe BAD THINGS.
           (Namely, the results differ from the obvious one-step division.  Just common sense
           says that the adding and subtracting of diagonals can't be a good thing.)
       We also observe that this is corrected by scaling each column of the matrix to its largest entry,
            which makes the diagonal system an identity.
       So here we go.
    */

    double scale0 = fabs (a00);
    double scale1 = fabs (a11);
#define USE_PRECOMPUTED_DIVISION
#ifdef USE_PRECOMPUTED_DIVISION
    double factor0, factor1;
#endif
    double a;
    bool    boolstat;


    a = fabs (a10);
    if (a > scale0)
        scale0 = a;

    a = fabs (a01);
    if (a > scale1)
        scale1 = a;


    if (scale0 == 0.0 || scale1 == 0.0)
        {
        *pX0 = 0.0;
        *pX1 = 0.0;
        return false;
        }


#ifdef USE_PRECOMPUTED_DIVISION
    factor0 = 1.0 / scale0;
    factor1 = 1.0 / scale1;
    a00 *= factor0;
    a10 *= factor0;
    a01 *= factor1;
    a11 *= factor1;
#else
    a00 /= scale0;
    a10 /= scale0;
    a01 /= scale1;
    a11 /= scale1;
#endif
    boolstat = bsiSVD_solve2x2_go (pX0, pX1, a00, a01, a10, a11, b0, b1);
    /* If fail, these are zeros */
#ifdef USE_PRECOMPUTED_DIVISION
    *pX0 *= factor0;
    *pX1 *= factor1;
#else
    *pX0 /= scale0;
    *pX1 /= scale1;
#endif
    return boolstat;
    }

//END
//BEGIN FUNCTION bsiDPoint3d_crossProduct3DPoint3d
/* VBSUB(Point3dCrossProduct3Points) */
/* CSVFUNC(crossProductToPoints) */
/* METHOD(DPoint3d,none,crossProductToPoints) */
/*-----------------------------------------------------------------*//**
* @description Returns the (vector) cross product of two vectors.
*   The vectors are computed from the Origin to Target1 and Target2.
* @instance pCrossProduct <= product
* @param pOrigin => The base point for computing vectors.
* @param pTarget1 => The target point for the first vector.
* @param pTarget2 => The target point for the second vector.
* @bsihdr                                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public void bsiDPoint3d_crossProduct3DPoint3d
(
        DPoint3d      *pCrossProduct,
const   DPoint3d    *pOrigin,
const   DPoint3d    *pTarget1,
const   DPoint3d    *pTarget2
)
    {
    double x1 = pTarget1->x - pOrigin->x;
    double y1 = pTarget1->y - pOrigin->y;
    double z1 = pTarget1->z - pOrigin->z;

    double x2 = pTarget2->x - pOrigin->x;
    double y2 = pTarget2->y - pOrigin->y;
    double z2 = pTarget2->z - pOrigin->z;

    pCrossProduct->x = y1 * z2 - z1 * y2;
    pCrossProduct->y = z1 * x2 - x1 * z2;
    pCrossProduct->z = x1 * y2 - y1 * x2;
    }



//END
//BEGIN FUNCTION bsiDPoint3d_dotProduct
/* VBSUB(Point3dDotProduct) */
/* CSVFUNC(dotProduct) */
/* METHOD(DPoint3d,none,dotProduct) */
/*-----------------------------------------------------------------*//**
* @description Returns the (scalar) dot product of two vectors.
* @instance pVector1 => The first vector
* @param pVector2 => The second vector
* @return The dot product of the two vectors
* @bsihdr                                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public double bsiDPoint3d_dotProduct
(
const   DPoint3d *pVector1,
const   DPoint3d *pVector2
)
    {
    return (pVector1->x*pVector2->x + pVector1->y*pVector2->y + pVector1->z*pVector2->z);
    }


//END
//BEGIN FUNCTION bsiDPoint3d_dotProductXY
/* VBSUB(Point3dDotProductXY) */
/* CSVFUNC(dotProductXY) */
/* METHOD(DPoint3d,none,dotProductXY) */
/*-----------------------------------------------------------------*//**
* @description Returns the (scalar) dot product of xy parts of two vectors.
* @instance pVector1 => The first vector
* @param pVector2 => The second vector
* @return The dot product of the xy parts of the two vectors
* @bsihdr                                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public double bsiDPoint3d_dotProductXY
(
const   DPoint3d   *pVector1,
const   DPoint3d   *pVector2
)
    {
    return (pVector1->x*pVector2->x + pVector1->y*pVector2->y);
    }

//END
//BEGIN FUNCTION bsiDPoint3d_normalizeInPlace
/* METHOD(DPoint3d,none,normalize) */
/*-----------------------------------------------------------------*//**
* @description Replaces a vector by a unit vector in the same direction, and returns
* the original length.
* If the input vector length is 0, the output vector is a zero vector
* and the returned length is 0.
*
* @instance pVector <=> vector to be normalized
* @return The length prior to normalization
* @bsihdr                                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public double bsiDPoint3d_normalizeInPlace
(
DPoint3d      *pVector
)
    {
    double  magnitude =
            sqrt ( pVector->x * pVector->x
                 + pVector->y * pVector->y
                 + pVector->z * pVector->z);

    if (magnitude > 0.0)
        {
        double f = 1.0 / magnitude;
        pVector->x *= f;
        pVector->y *= f;
        pVector->z *= f;
        }

    return  magnitude;
    }


//END
//BEGIN FUNCTION bsiDPoint3d_subtractDPoint3dDPoint3d
/* VBSUB(Point3dSubtract) */
/* CSVFUNC(Subtract) */
/* METHOD(DPoint3d,none,differenceOf) */
/*-----------------------------------------------------------------*//**
* @description Subtract coordinates of two vectors or points. (Compute Point1 - Point2)
*
* @instance pVector <= The difference vector
* @param pPoint1 => The first point
* @param pPoint2 => The second (subtracted) point.
* @bsihdr                                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public void bsiDPoint3d_subtractDPoint3dDPoint3d
(
        DPoint3d          *pVector,
const   DPoint3d        *pPoint1,
const   DPoint3d        *pPoint2
)
    {
    pVector->x = pPoint1->x - pPoint2->x;
    pVector->y = pPoint1->y - pPoint2->y;
    pVector->z = pPoint1->z - pPoint2->z;
    }



//END
//BEGIN FUNCTION bsiDPoint3d_addScaledDPoint3d
/* VBSUB(Point3dAddScaled) */
/* CSVFUNC(Add) */
/* METHOD(DPoint3d,none,sumOf) */
/*-----------------------------------------------------------------*//**
* @description Adds an origin and a scaled vector.
*
* @instance pSum <= The sum
* @param pOrigin => Origin for the sum.
* @param pVector => The vector to be added.
* @param scale => The scale factor.
* @bsihdr                                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public void bsiDPoint3d_addScaledDPoint3d
(
        DPoint3d        *pSum,
const   DPoint3d        *pOrigin,
const   DPoint3d          *pVector,
        double           scale
)
    {
    if (pOrigin)
        {
        pSum->x = pOrigin->x + pVector->x * scale;
        pSum->y = pOrigin->y + pVector->y * scale;
        pSum->z = pOrigin->z + pVector->z * scale;
        }
    else
        {
        pSum->x = pVector->x * scale;
        pSum->y = pVector->y * scale;
        pSum->z = pVector->z * scale;
        }
    }
//END
