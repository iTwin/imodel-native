/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/structs/dpoint4d.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------*//**
* @struct DPoint4d
* A DPoint4d structure holds components of a 4D (homogeneous) point.
* @fields
* @field double x x component of point or vector
* @field double y y component of point or vector
* @field double z z component of point or vector
* @field double w w (weight) component
* @endfields
* @bsistruct                                        EarlinLutz      09/00
+----------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE


/*-----------------------------------------------------------------*//**
* @instance pInstance <= point to zero out.
* @indexVerb zero
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint4d_setZero

(
DPoint4dP pInstance
)

    {
    pInstance->x = pInstance->y = pInstance->z = pInstance->w = 0.0;
    }


/*-----------------------------------------------------------------*//**
* @instance pInstance <= point to fill
* @param xComponent => x component
* @param yComponent => y component
* @param zComponent => z component
* @param wComponent => w component
* @indexVerb set
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint4d_setComponents

(
DPoint4dP pInstance,
double          xComponent,
double          yComponent,
double          zComponent,
double          wComponent
)

    {
    pInstance->x = xComponent;
    pInstance->y = yComponent;
    pInstance->z = zComponent;
    pInstance->w = wComponent;
    }


/*-----------------------------------------------------------------*//**
* Fill a DPoint4d, using given xyz components and weight.
* All components are copied in directly --
*  the xyz components are not multiplied by the weight.
*
* @instance pInstance <= point to be filled
* @param pSource => xyz components
* @param w       => w component
* @indexVerb init
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint4d_initFromDPoint3dAndWeight

(
DPoint4dP pInstance,
DPoint3dCP pSource,
double      w
)
    {
    pInstance->x = pSource->x;
    pInstance->y = pSource->y;
    pInstance->z = pSource->z;
    pInstance->w = w;
    }



/*-----------------------------------------------------------------*//**
* Copy 4 components (xyzw) from a double array into this instance
*
* @instance pInstance <= point to be filled
* @param pArray => array of doubles
* @indexVerb init
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint4d_initFromArray

(
DPoint4dP pInstance,
double      *pArray
)
    {
    pInstance->x = pArray[0];
    pInstance->y = pArray[1];
    pInstance->z = pArray[2];
    pInstance->w = pArray[3];
    }



/*-----------------------------------------------------------------*//**
* Copies component data out of this instance into
* doubles pXCoord, pYCoord, pZCoord and pWCoord.
*
* @instance pInstance => point whose components are set
* @param pXCoord <= x component
* @param pYCoord <= y component
* @param pZCoord <= z component
* @param pWCoord <= w component
* @indexVerb get
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint4d_getComponents

(
DPoint4dCP pInstance,
double      *pXCoord,
double      *pYCoord,
double      *pZCoord,
double      *pWCoord
)
    {
    *pXCoord = pInstance->x;
    *pYCoord = pInstance->y;
    *pZCoord = pInstance->z;
    *pWCoord = pInstance->w;
    }



/*-----------------------------------------------------------------*//**
* Set x,y,z or w component of a point.
*
* @instance pInstance <= point whose components are set
* @param a => component value
* @param index => 0=x, 1=y, 2=z, 3=w, others cyclic
* @indexVerb set
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint4d_setComponent

(
DPoint4dP pInstance,
double       a,
int         index
)
    {
    index = index & 0x11;
    if (index == 0)
        {
        pInstance->x = a;
        }
    else if (index == 1)
        {
        pInstance->y = a;
        }
    else if (index == 2)
        {
        pInstance->z = a;
        }
    else /* index == 3 */
        {
        pInstance->w = a;
        }
    }


/*-----------------------------------------------------------------*//**
* @instance pInstance => point whose components are accessed
* @param index => 0=x, 1=y, 2=z, 3=w, others cyclic
* @return specified component of the point or vector
* @indexVerb get
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDPoint4d_getComponent

(
DPoint4dCP pInstance,
int         index
)
    {
    index = index & 0x11;
    if (index == 0)
        {
        return pInstance->x;
        }
    else if (index == 1)
        {
        return pInstance->y;
        }
    else if (index == 2)
        {
        return pInstance->z;
        }
    else /* (index == 3) */
        {
        return pInstance->w;
        }
    }

/*-----------------------------------------------------------------*//**
* @param pInstance IN      point whose components are accessed
* magnitude as pure 4d point -- sqrt sum of squares.
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDPoint4d_magnitudeXYZW
(
DPoint4dCP pInstance
)
    {
    return sqrt (bsiDPoint4d_dotProduct (pInstance, pInstance));
    }


/*-----------------------------------------------------------------*//**
*
* Return the full 4d (xyzw) dot product of two homogeneous points.
* @instance pInstance => first point
* @param pVec2 => second second
* @return dot product of two homogeneous points.
* @indexVerb dotProduct
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDPoint4d_dotProduct

(
DPoint4dCP pInstance,
DPoint4dCP pPoint
)

    {
    return
                  pInstance->x * pPoint->x
                + pInstance->y * pPoint->y
                + pInstance->z * pPoint->z
                + pInstance->w * pPoint->w
                ;
    }


/*-----------------------------------------------------------------*//**
* Return the dot product of only the xy parts of two homogeneous points.  Ignore z, ignore w.
* @instance pInstance => first point
* @param    pPoint => second point
* @return dot product of two homogeneous points.
* @indexVerb dotProduct
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDPoint4d_dotProductXY

(
DPoint4dCP pInstance,
DPoint4dCP pPoint
)

    {
    return
                  pInstance->x * pPoint->x
                + pInstance->y * pPoint->y
                ;
    }


/*-----------------------------------------------------------------*//**
*
* Return the xyz dot product of two homogeneous points, i.e. ignore w.
* @instance pInstance => first point
* @param pVec2 => second second
* @return dot product of two homogeneous points.
* @indexVerb dotProduct
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDPoint4d_dotProductXYZ

(
DPoint4dCP pInstance,
DPoint4dCP pPoint2
)

    {
    return
                  pInstance->x * pPoint2->x
                + pInstance->y * pPoint2->y
                + pInstance->z * pPoint2->z
                ;
    }


/*-----------------------------------------------------------------*//**
*
* Return the xyz dot product of two homogeneous points, i.e. ignore z.
* @instance pInstance => first point
* @param pPoint => second second
* @return dot product of two homogeneous points.
* @indexVerb dotProduct
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDPoint4d_dotProductXYW

(
DPoint4dCP pInstance,
DPoint4dCP pPoint
)

    {
    return
                  pInstance->x * pPoint->x
                + pInstance->y * pPoint->y
                + pInstance->w * pPoint->w
                ;
    }


/*-----------------------------------------------------------------*//**
*
* @instance pInstance   => first point
* @param    x       => x component of second point
* @param    y       => y component of second point
* @param    z       => z component of second point
* @param    w         => w component of second point
* @return dot product of two homogeneous points.
* @indexVerb dotProduct
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDPoint4d_dotComponents

(
DPoint4dCP pInstance,
double    x,
double    y,
double    z,
double    w
)

    {
    return
                  pInstance->x * x
                + pInstance->y * y
                + pInstance->z * z
                + pInstance->w * w
                ;
    }


/*-----------------------------------------------------------------*//**
*
* @instance pInstance   => first point
* @param pPoint2   => second point
* @param    w         => w component of second point
* @return dot product of two homogeneous points.
* @indexVerb dotProduct
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDPoint4d_dotWeightedPoint

(
DPoint4dCP pInstance,
DPoint3dCP pPoint2,
double    w
)

    {
    return
                  pInstance->x * pPoint2->x
                + pInstance->y * pPoint2->y
                + pInstance->z * pPoint2->z
                + pInstance->w * w
                ;
    }



/*-----------------------------------------------------------------*//**
* Return the dot product of a plane normal and a vector 'to the
* eyepoint'.   The plane is given as cartesian origin and normal; the
* eye is given as homogeneous point, i.e. weight zero for flat view,
* nonzero for perspective.
* Eyepoints constucted 'by hand' usually look like this:
* Flat view "from infinity" looking in direction (xyz):
*       eyepoint = (x,y,z,0)
* i.e. a top view has eyepoint (0,0,1,0)
* Perspective from eyepoint at (x,y,z): eyepoint (x,y,z,1)
* When viewing is constructed by a sequence of homogeneous
* transformations, with the final (device) projection to the xy plane,
* the (pretransform) eyepoint is 'by definition'
*       Tinverse * (0,0,1,0)'
* i.e column 2 (zero based) of the composite viewing transform.
* (Note that the weight part can be nonzero.)
*
* @instance pInstance => eye point 0 weight for flat view eye at infinity
* @param pOrigin => any cartesian point on plane
* @param pNormal => cartesian plane normal
* @return dot product of plane normal with vector towards eye.
* @indexVerb eyepoint
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDPoint4d_eyePlaneTest

(
DPoint4dCP pInstance,
DPoint3dCP pOrigin,
DPoint3dCP pNormal
)

    {
    double w = pInstance->w;
    if (w == 0.0)
        {
        /* pInstance is just xyz vector components.  Simple dot with normal.*/
        /* Plane orign doesn't matter.*/
        return    pInstance->x * pNormal->x
                + pInstance->y * pNormal->y
                + pInstance->z * pNormal->z;
        }
    else
        {
        /* pInstance is weighted.*/
        double dot1 =
                  pInstance->x * pNormal->x
                + pInstance->y * pNormal->y
                + pInstance->z * pNormal->z;

        double dot2 =
                  pOrigin->x * pNormal->x
                + pOrigin->y * pNormal->y
                + pOrigin->z * pNormal->z;

        return dot1 - w * dot2;
        }
    }




/*-----------------------------------------------------------------*//**
* @instance pInstance => first point
* @param pVec2 => second point
* @return distance between projections of two homnogeneous points.
* @indexVerb distance
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDPoint4d_realDistance

(
DPoint4dCP pInstance,
DPoint4dCP pVec2
)

    {
    double w1 = pInstance->w;
    double w2 = pVec2->w;
    double denom = w1 * w2;
    double distance = 0.0;
    if (w1 == 1.0 && w2 == 1.0)     /* Nearly always, so save the weighting ... */
        {
        double dx = pInstance->x - pVec2->x;
        double dy = pInstance->y - pVec2->y;
        double dz = pInstance->z - pVec2->z;
        distance = sqrt ( dx * dx + dy * dy + dz * dz);
        }
    else if (denom != 0.0)
        {
        /* Brute force: normalize each x,y,z coordiante, i.e. 6 divides.
           Or: ( X1/w1 - X2/w2 ) = (w2*X1 - w1*X2)/(w1 * w2), i.e. 6 multiplies and one divide.
           If division is modestly slower, this is a win.
        */
        double dx = w2 * pInstance->x - w1 * pVec2->x;
        double dy = w2 * pInstance->y - w1 * pVec2->y;
        double dz = w2 * pInstance->z - w1 * pVec2->z;
        distance = sqrt ( (dx * dx + dy * dy + dz * dz) / (denom * denom) );
        }
    return distance;
    }



/*-----------------------------------------------------------------*//**
* @instance pVec1 => first point
* @param pDistSquared <= squared distance
* @param pVec2 => second point
* @return true iff the homogeneous point was properly normalized.
* @indexVerb distance
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDPoint4d_realDistanceSquaredXY

(
DPoint4dCP pVec1,
double    *pDistanceSquared,
DPoint3dCP pVec2
)

    {
    double w1 = pVec1->w;
    if (w1 == 1.0)     /* Nearly always, so save the weighting ... */
        {
        double dx = pVec1->x - pVec2->x;
        double dy = pVec1->y - pVec2->y;
        *pDistanceSquared = dx * dx + dy * dy;
        return true;
        }
    else if (w1 != 0.0)
        {
        /* We believe that 3 multiplies and 1 division are faster than 2 divisions */
        double dx = pVec1->x - w1 * pVec2->x;
        double dy = pVec1->y - w1 * pVec2->y;
        *pDistanceSquared =  (dx * dx + dy * dy) / ( w1 * w1);
        return true;
        }
    return false;
    }


/*-----------------------------------------------------------------*//**
* @instance pVec1 => first point
* @param pDistSquared <= squared distance
* @param pVec2 => second point
* @return true iff the homogeneous point was properly normalized.
* @indexVerb distance
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDPoint4d_realDistanceSquaredDPoint3d

(
DPoint4dCP pVec1,
double    *pDistanceSquared,
DPoint3dCP pVec2
)
    {
    double w1 = pVec1->w;
    if (w1 == 1.0)     /* Nearly always, so save the weighting ... */
        {
        double dx = pVec1->x - pVec2->x;
        double dy = pVec1->y - pVec2->y;
        double dz = pVec1->z - pVec2->z;
        *pDistanceSquared = dx * dx + dy * dy + dz * dz;
        return true;
        }
    else if (w1 != 0.0)
        {
        /* We believe that 3 multiplies and 1 division are faster than 2 divisions */
        double dx = pVec1->x - w1 * pVec2->x;
        double dy = pVec1->y - w1 * pVec2->y;
        double dz = pVec1->z - w1 * pVec2->z;
        *pDistanceSquared =  (dx * dx + dy * dy + dz * dz) / ( w1 * w1);
        return true;
        }
    return false;
    }


/*-----------------------------------------------------------------*//**
* @instance pVec1 => first point
* @param pDistSquared <= squared distance
* @param pVec2 => second point
* @return true iff the homogeneous points were properly normalized.
* @indexVerb distance
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDPoint4d_realDistanceSquared

(
DPoint4dCP pVec1,
double    *pDistanceSquared,
DPoint4dCP pVec2
)

    {
    double w1 = pVec1->w;
    double w2 = pVec2->w;
    if (w1 == 1.0 && w2 == 1.0)     /* Nearly always, so save the weighting ... */
        {
        double dx = pVec1->x - pVec2->x;
        double dy = pVec1->y - pVec2->y;
        double dz = pVec1->z - pVec2->z;
        *pDistanceSquared = dx * dx + dy * dy + dz * dz;
        return true;
        }
    else if (w1 != 0.0)
        {
        double dx = w2 * pVec1->x - w1 * pVec2->x;
        double dy = w2 * pVec1->y - w1 * pVec2->y;
        double dz = w2 * pVec1->z - w1 * pVec2->z;
        *pDistanceSquared =  (dx * dx + dy * dy + dz * dz) / ( w1 * w1 * w2 * w2);
        return true;
        }
    return false;
    }


/*-----------------------------------------------------------------*//**
* Interpolates between two homogeneous vectors.                         |
*
* @instance pInstance <= interpolated point
* @param pPoint0 => s=0 point
* @param s => interpolation parameter
* @param pPoint1 => s=1 point
* @indexVerb interpolate
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint4d_interpolate

(
DPoint4dP pInstance,
DPoint4dCP pPoint0,
double     s,
DPoint4dCP pPoint1
)

    {
    pInstance->x = pPoint0->x  + s * (pPoint1->x - pPoint0->x);
    pInstance->y = pPoint0->y  + s * (pPoint1->y - pPoint0->y);
    pInstance->z = pPoint0->z  + s * (pPoint1->z - pPoint0->z);
    pInstance->w = pPoint0->w  + s * (pPoint1->w - pPoint0->w);
    }

/*---------------------------------------------------------------------------------**//**
* @description Implementation of slerp: spherical linear interpolation of quaternions.
* @remarks Slerp is constant velocity interpolation between rotations, as opposed to linearly
*       interpolating quaternions and normalizing the result (nlerp), which traverses the
*       great arc between the quaternions in non-constant time (faster near the input quaternions
*       than in between them).  The classic paper is Ken Shoemake, "Animating Rotation with Quaternion Curves",
*       SIGGRAPH Proceedings, 245–254, 1985.
* @remarks The quaternions are assumed to be normalized, i.e. of the form (x,y,z,w), where x*x + y*y + z*z + w*w = 1.
* @remarks If the quaternions' dot product is negative, it and pQuaternion1 are negated internally so that
*       the interpolation occurs along the shorter of the two great arcs determined by the quaternions.
* @param pSum               OUT     The interpolated quaternion, normalized
* @param pQuaternion0       IN      The normalized quaternion corresponding to fractionParameter of 0.0
* @param fractionParameter  IN      The interpolation parameter in [0,1]
* @param pQuaternion1       IN      The normalized quaternion corresponding to fractionParameter of 1.0
* @return true if normalization of the resulting quaternion succeeded.
* @group "DPoint4d Quaternion"
* @bsimethod                                                    DavidAssaf      07/06
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     bsiDPoint4d_interpolateQuaternions

(
DPoint4dP    pSum,
DPoint4dCP    pQuaternion0,
double      fractionParameter,
DPoint4dCP    pQuaternion1
)
    {
    DPoint4d        q0, q1, q2;
    double          angle, angleOfInterpolant, dot;
    static double   s_maxSafeCosine = 0.9995;

    // return exact quats for special values
    if (0.0 == fractionParameter)
        {
        *pSum = *pQuaternion0;
        return true;
        }
    if (1.0 == fractionParameter)
        {
        *pSum = *pQuaternion1;
        return true;
        }
    if (0.5 == fractionParameter)
        {
        bsiDPoint4d_addDPoint4dDPoint4d (pSum, pQuaternion0, pQuaternion1);
        return 0.0 < bsiDPoint4d_normalizeQuaternion (pSum);
        }

    q0  = *pQuaternion0;
    q1  = *pQuaternion1;
    dot = bsiDPoint4d_dotProduct (&q0, &q1);

    // prevent interpolation through the longer great arc
    if (dot < 0.0)
        {
        bsiDPoint4d_negateInPlace (&q1);
        dot = -dot;
        }

    // if nearly parallel, use nlerp
    if (dot > s_maxSafeCosine)
        {
        bsiDPoint4d_interpolate (pSum, &q0, fractionParameter, &q1);
        return 0.0 < bsiDPoint4d_normalizeQuaternion (pSum);
        }

    // safety check
    if (dot < -1.0)
        dot = -1.0;
    else if (dot > 1.0)
        dot = 1.0;

    // create orthonormal basis {q0, q2}
    bsiDPoint4d_addScaledDPoint4d (&q2, &q1, &q0, -dot);
    bsiDPoint4d_normalizeQuaternion (&q2);

    angle = acos (dot);
    angleOfInterpolant = angle * fractionParameter;
    bsiDPoint4d_add2ScaledDPoint4d (pSum, NULL, &q0, cos (angleOfInterpolant), &q2, sin (angleOfInterpolant));
    return true;
    }


/*-----------------------------------------------------------------*//**
* Initialize a homogeneous point from a 3D point and separate weight.
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
Public GEOMDLLIMPEXP void bsiDPoint4d_copyAndWeight

(
DPoint4dP pInstance,
DPoint3dCP pPoint,
double    w
)
    {
    pInstance->x = pPoint->x;
    pInstance->y = pPoint->y;
    pInstance->z = pPoint->z;
    pInstance->w = w;
    }



/*-----------------------------------------------------------------*//**
* Copy the xyz components out of a homogeneous point.  The weight is
* not referenced, i.e. the xyz components are NOT normalized.
*
* @instance pInstance => homogeneous point
* @param pPoint <= cartesian point
* @indexVerb get
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint4d_cartesianFromHomogeneous

(
DPoint4dCP pInstance,
DPoint3dP pPoint
)
    {
    pPoint->x = pInstance->x;
    pPoint->y = pInstance->y;
    pPoint->z = pInstance->z;
    }


/*-----------------------------------------------------------------*//**
* Copy the xyw components out of a homogeneous point.  The z component
* not referenced. This is a copy, not a normalization.
*
* @instance pInstance => homogeneous point
* @param pPoint <= xyw parts copied to xyz
* @indexVerb get
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint4d_getXYW

(
DPoint4dCP pInstance,
DPoint3dP pPoint
)
    {
    pPoint->x = pInstance->x;
    pPoint->y = pInstance->y;
    pPoint->z = pInstance->w;
    }


/*-----------------------------------------------------------------*//**
* Set components of a 3d point from 3 indices into a homogeneous point.
* Indices are interpreted cyclically.
*
* @instance pInstance   => homogeneous point with coordinates to select
* @param    pPoint  <= output point
* @param    xIndex  => index for x component of output
* @param    yIndex  => index for y component of output
* @param    zIndex  => index for z component of output
* @indexVerb get
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint4d_selectDPoint3d

(
DPoint4dCP pInstance,
DPoint3dP pPoint,
int       xIndex,
int       yIndex,
int       zIndex
)

    {
    const double *pHArray = (const double *)pInstance;

    /* Force cyclic ordering.  Low order bits even work for negative numbers!! */
    xIndex &= 0x3;
    yIndex &= 0x3;
    zIndex &= 0x3;

    pPoint->x = pHArray[xIndex];
    pPoint->y = pHArray[yIndex];
    pPoint->z = pHArray[zIndex];
    }



/*-----------------------------------------------------------------*//**
* Copies an array of homogeneous points
*
* @param pOutPoint <= destination array
* @param m => maximum number of points to copy
* @param pInPoint => source array
* @param n => number of points
* @return number of copied
* @indexVerb copy
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int bsiDPoint4d_copyArray


(
DPoint4dP pOutPoint,
int           m,
DPoint4dCP pInPoint,
int           n
)
    {
    int i;
    if (n < 0)
        n = 0;
    if (n > m)
        n = m;

    for ( i = 0; i < n ; i++)
        pOutPoint[i] = pInPoint[i];
    return n;
    }


/*-----------------------------------------------------------------*//**
* In-place reversal.
*
* @param pPointArray <=> array to modify
* @param n => number of points
* @return number of copied
* @indexVerb reverse
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiDPoint4d_reverseArrayInPlace


(
DPoint4dP pPointArray,
int           n
)
    {
    int i, j;
    DPoint4d point;
    for ( i = 0, j = n - 1; i < j ; i++, j--)
        {
        point = pPointArray[i];
        pPointArray[i] = pPointArray[j];
        pPointArray[j] = point;
        }
    }


/*-----------------------------------------------------------------*//**
* copies n DPoint4d structures from the pSource array to the pDest
* using an index array to rearrange (not necessarily 1to1) the order.
* The indexing assigns pDest[i] = pSource[indexP[i]].

*
* @param pDest <= destination array
* @param pSource => source array
* @param pIndex => array of indices into source array
* @param nIndex => number of points
* @indexVerb copy
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint4d_copyIndexedArray


(
DPoint4dP pDest,
DPoint4dCP pSource,
int         *pIndex,
int          nIndex
)
    {
    int     i;
    int    *indP;

    for (i = 0, indP = pIndex; i < nIndex; i++, indP++)
        {
        pDest[i] = pSource[*indP];
        }
    }




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
Public GEOMDLLIMPEXP bool    bsiDPoint4d_planeFromOriginAndNormal

(
DPoint4dP pInstance,
DPoint3dCP pOrigin,
DPoint3dCP pNormal
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



/*-----------------------------------------------------------------*//**
* Computes the homogeneous vectors for planes defined by 3D origin
* and normal.
* Any given planes with 0 normal are ignored and omitted from the
* output array.  (Hence caller should revise their count with the

*
* @param pPlane <= homogeneous vector for plane
* @param pOrigin <= origin point
* @param pNormal <= normal vector
* @param int       numIn                 <= number of origins, normals
* @return number of planes with nonzero normal.
* @indexVerb init
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int bsiDPoint4d_planesFromOriginAndNormalArrays

(
DPoint4dP pPlane,
DPoint3dCP pOrigin,
DPoint3dCP pNormal,
int       numIn
)
    {
    int numOut = 0;
    int i;
    DPoint4d currPlane;
    for (i = 0; i < numIn; i++)
        {
        if (bsiDPoint4d_planeFromOriginAndNormal (&currPlane, pOrigin + i, pNormal + i ))
            {
            pPlane[numOut++] = currPlane;
            }
        }
    return numOut;
    }



/*-----------------------------------------------------------------*//**
* Computes the homogeneous coordinate vector for a plane defined by
* 3 3D points.
*
* @instance pInstance => homogeneous vector for plane
* @param pOrigin <= origin point
* @param pPoint1 <= another point on plane
* @param pPoint2 <= another point on plane
* @return true if normal is well defined.
* @indexVerb init
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDPoint4d_planeFrom3DPoint3d

(
DPoint4dP pInstance,
DPoint3dCP pOrigin,
DPoint3dCP pPoint1,
DPoint3dCP pPoint2
)

    {
    DPoint3d normal, vector1, vector2;
    bsiDPoint3d_subtractDPoint3dDPoint3d ( &vector1, pPoint1, pOrigin );
    bsiDPoint3d_subtractDPoint3dDPoint3d ( &vector2, pPoint2, pOrigin );
    bsiDPoint3d_crossProduct ( &normal, &vector1, &vector2 );
    return bsiDPoint4d_planeFromOriginAndNormal ( pInstance, pOrigin, &normal );
    }


/*-----------------------------------------------------------------*//**
* Computes the homogeneous coordinate vector for a plane defined by
* a DPoint4d origin and a pair of 3D vectors.
*
* @instance pInstance => homogeneous vector for plane
* @param pOrigin <= a point on the plane.
* @param pVector1 <= a vector in the plane.
* @param pVector2 <= another vector in the plane.
* @return false if origin, vectors are not independent.
* @indexVerb init
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDPoint4d_planeFromDPoint4dAndDPoint3dVectors

(
DPoint4dP pInstance,
DPoint4dCP pOrigin,
DPoint3dCP pVector0,
DPoint3dCP pVector1
)

    {
    if (pOrigin->w == 0.0)
        {
        /* Really given three vectors. */
        bsiDPoint4d_setComponents (pInstance, 0.0, 0.0, 0.0, 1.0);
        }
    else
        {
        DPoint3d W;
        double a, b;
        a = pOrigin->w;
        bsiDPoint3d_crossProduct (&W, pVector0, pVector1);
        b = bsiDPoint4d_dotWeightedPoint (pOrigin, &W, 0.0);
        bsiDPoint4d_setComponents (pInstance, a * W.x, a * W.y, a * W.z, -b);
        }
    /* UGH -- need to test zero.  No good tolerance around. */
    return pInstance->x != 0.0 || pInstance->y != 0.0 || pInstance->z != 0.0;
    }



/*-----------------------------------------------------------------*//**
* @instance pInstance => homogeneous plane coeffciients
* @param pOrigin <= cartesian orign
* @param pNormal <= cartesian normal
* @return true if
* @indexVerb get
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDPoint4d_originAndNormalFromPlane

(
DPoint4dCP pInstance,
DPoint3dP pOrigin,
DPoint3dP pNormal
)
    {
    bsiDPoint4d_cartesianFromHomogeneous (pInstance, pNormal);
    bsiDPoint3d_scale (pOrigin, pNormal, pInstance->w);

    return  bsiDPoint3d_normalizeInPlace (pNormal) > 0.0;
    }





/*-----------------------------------------------------------------*//**
* Adds two homogeneous points.
*
* @instance pInstance <= pt1 + pt2
* @param pt1 => point 1
* @param pt2 => point 2
* @indexVerb add
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint4d_addDPoint4dDPoint4d

(
DPoint4dP pInstance,
DPoint4dCP pt1,
DPoint4dCP pt2
)
    {
    pInstance->x = pt1->x + pt2->x;
    pInstance->y = pt1->y + pt2->y;
    pInstance->z = pt1->z + pt2->z;
    pInstance->w = pt1->w + pt2->w;
    }


/*-----------------------------------------------------------------*//**
* Scale each point by the other's weight and return the difference
*
* @instance sum <= pA * pB->w - pB * pA->w
* @indexVerb subtract
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint4d_weightedDifference

(
DPoint4dP pInstance,
DPoint4dCP pA,
DPoint4dCP pB
)
    {
    double wA = pA->w;
    double wB = pB->w;
    pInstance->x = pA->x * wB - pB->x * wA;
    pInstance->y = pA->y * wB - pB->y * wA;
    pInstance->z = pA->z * wB - pB->z * wA;
    pInstance->w = 0.0;
    }


/*-----------------------------------------------------------------*//**
* Scale each point by the other's weight and return the difference.
* (Note that the w component of the result is always zero)
*
* @instance sum <= pA * pB->w - pB * pA->w
* @indexVerb subtract
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint4d_weightedDifferenceDPoint4dDPoint3d

(
DPoint4dP pInstance,
DPoint4dCP pA,
DPoint3dCP pB,
double       wB
)
    {
    double wA = pA->w;
    pInstance->x = pA->x * wB - pB->x * wA;
    pInstance->y = pA->y * wB - pB->y * wA;
    pInstance->z = pA->z * wB - pB->z * wA;
    pInstance->w = 0.0;
    }


/*-----------------------------------------------------------------*//**
* Scale each point by the other's weight and return the difference.
* (Note that the w component of the result is always zero)
*
* @instance sum <= pA * pB->w - pB * pA->w
* @indexVerb subtract
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint4d_weightedDifferenceDPoint3dDPoint4d

(
DPoint4dP pInstance,
DPoint3dCP pA,
double         wA,
DPoint4dCP pB
)
    {
    double wB = pB->w;
    pInstance->x = pA->x * wB - pB->x * wA;
    pInstance->y = pA->y * wB - pB->y * wA;
    pInstance->z = pA->z * wB - pB->z * wA;
    pInstance->w = 0.0;
    }


/*-----------------------------------------------------------------*//**
* Add a vector to the instance.
*
* @instance pInstance <= pInstance + pVector
* @param pVector => vector to add
* @indexVerb add
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint4d_add

(
DPoint4dP pInstance,
DPoint4dCP pVector
)
    {
    pInstance->x += pVector->x;
    pInstance->y += pVector->y;
    pInstance->z += pVector->z;
    pInstance->w += pVector->w;
    }


/*-----------------------------------------------------------------*//**
* Subtract a vector from the instance.
*
* @instance pInstance <= pInstance - pVector
* @param pPoint => vector to subtract
* @indexVerb subtract
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint4d_subtract

(
DPoint4dP pInstance,
DPoint4dCP pVector
)
    {
    pInstance->x -= pVector->x;
    pInstance->y -= pVector->y;
    pInstance->z -= pVector->z;
    pInstance->w -= pVector->w;
    }


/*-----------------------------------------------------------------*//**
* Subtract second point from first.
*
* @instance <= pPoint1 - pPoint2
* @param pPoint1 => first point
* @param pPoint2 => second point
* @indexVerb subtract
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint4d_subtractDPoint4dDPoint4d

(
DPoint4dP pInstance,
DPoint4dCP pPoint1,
DPoint4dCP pPoint2
)
    {
    pInstance->x = pPoint1->x - pPoint2->x;
    pInstance->y = pPoint1->y - pPoint2->y;
    pInstance->z = pPoint1->z - pPoint2->z;
    pInstance->w = pPoint1->w - pPoint2->w;
    }


/*-----------------------------------------------------------------*//**
* Adds two homogeneous points to a base point.
*
* @instance pInstance <= P0 + P1 * s1 + P2 * s2
* @param pPoint0 => base point
* @param pPoint1 => point 1
* @param scale1 => scale factor for point 1
* @param pPoint2 => point 2
* @param scale2 => scale factor for point 2
* @indexVerb add
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint4d_add2ScaledDPoint4d

(
DPoint4dP pInstance,
DPoint4dCP pPoint0,
DPoint4dCP pPoint1,
double      scale1,
DPoint4dCP pPoint2,
double      scale2
)
    {
    if (pPoint0)
        {
        pInstance->x = pPoint0->x + pPoint1->x * scale1 + pPoint2->x * scale2;
        pInstance->y = pPoint0->y + pPoint1->y * scale1 + pPoint2->y * scale2;
        pInstance->z = pPoint0->z + pPoint1->z * scale1 + pPoint2->z * scale2;
        pInstance->w = pPoint0->w + pPoint1->w * scale1 + pPoint2->w * scale2;
        }
    else
        {
        pInstance->x = pPoint1->x * scale1 + pPoint2->x * scale2;
        pInstance->y = pPoint1->y * scale1 + pPoint2->y * scale2;
        pInstance->z = pPoint1->z * scale1 + pPoint2->z * scale2;
        pInstance->w = pPoint1->w * scale1 + pPoint2->w * scale2;
        }
    }



/*-----------------------------------------------------------------*//**
* Adds three homogeneous points to a base point.
*
* @instance pInstance <= P0 + P1 * s1 + P2 * s2
* @param pPoint0 => base point
* @param pPoint1 => point 1
* @param scale1 => scale factor for point 1
* @param pPoint2 => point 2
* @param scale2 => scale factor for point 2
* @param pPoint3 => point 3
* @param scale3 => scale factor for point 3
* @indexVerb add
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint4d_add3ScaledDPoint4d

(
DPoint4dP pInstance,
DPoint4dCP pPoint0,
DPoint4dCP pPoint1,
double      scale1,
DPoint4dCP pPoint2,
double      scale2,
DPoint4dCP pPoint3,
double      scale3
)
    {
    if (pPoint0)
        {
        pInstance->x = pPoint0->x + pPoint1->x * scale1 + pPoint2->x * scale2 + pPoint3->x * scale3;
        pInstance->y = pPoint0->y + pPoint1->y * scale1 + pPoint2->y * scale2 + pPoint3->y * scale3;
        pInstance->z = pPoint0->z + pPoint1->z * scale1 + pPoint2->z * scale2 + pPoint3->z * scale3;
        pInstance->w = pPoint0->w + pPoint1->w * scale1 + pPoint2->w * scale2 + pPoint3->w * scale3;
        }
    else
        {
        pInstance->x = pPoint1->x * scale1 + pPoint2->x * scale2 + pPoint3->x * scale3;
        pInstance->y = pPoint1->y * scale1 + pPoint2->y * scale2 + pPoint3->y * scale3;
        pInstance->z = pPoint1->z * scale1 + pPoint2->z * scale2 + pPoint3->z * scale3;
        pInstance->w = pPoint1->w * scale1 + pPoint2->w * scale2 + pPoint3->w * scale3;
        }
    }



/*-----------------------------------------------------------------*//**
* Adds two homogeneous points to a base point.
*
* @instance pInstance <= P0 + P1 * s1 + P2 * s2
* @param pPoint0 => base point
* @param pPoint1 => point 1
* @param scale1 => scale factor for point 1
* @indexVerb add
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint4d_addScaledDPoint4d

(
DPoint4dP pInstance,
DPoint4dCP pPoint0,
DPoint4dCP pPoint1,
double      scale1
)
    {
    if (pPoint0)
        {
        pInstance->x = pPoint0->x + pPoint1->x * scale1;
        pInstance->y = pPoint0->y + pPoint1->y * scale1;
        pInstance->z = pPoint0->z + pPoint1->z * scale1;
        pInstance->w = pPoint0->w + pPoint1->w * scale1;
        }
    else
        {
        pInstance->x = pPoint1->x * scale1;
        pInstance->y = pPoint1->y * scale1;
        pInstance->z = pPoint1->z * scale1;
        pInstance->w = pPoint1->w * scale1;
        }
    }


/*---------------------------------------------------------------------------------**//**
* Apply a Givens to the two points, as if they are columns of a matrix to be postmultiplied
* by the Givens matrx.
* The Givens matrix is
*       R(0,0)=R(1,1)=c
*       R(0,1)=-s
*       R(1,0)=s
*
* @param    pOut0   <= rotation of pIn0
* @param    pOut1   <= rotation of pIn1
* @param    pIn0    => first vector
* @param    pIn1    => second vector
* @param    c   => cosine of givens rotation.
* @param    s   => sine of givens rotation.
* @bsihdr                                                       EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint4d_applyGivensRotation

(
DPoint4dP pOut0,
DPoint4dP pOut1,
DPoint4dCP pIn0,
DPoint4dCP pIn1,
double          c,
double          s
)
    {
    double a, b;
    int i;
    double *p0 = (double*)pIn0;
    double *p1 = (double*)pIn1;

    for (i = 0; i < 4; i++)
        {
        a = p0[i];
        b = p1[i];
        p0[i] = a * c + b * s;
        p1[i] = b * c - a * s;
        }
    }


/*---------------------------------------------------------------------------------**//**
* Apply a hyperbolic rotation to the two points, as if they are columns of a matrix to be postmultiplied
* by the hyperbolic matrix
* The hyperbolic matrix is
*       R(0,0)=R(1,1)=secant
*       R(0,1)=R(1,0)= tangent
*
* @param    pOut0   <= rotation of pIn0
* @param    pOut1   <= rotation of pIn1
* @param    pIn0    => first vector
* @param    pIn1    => second vector
* @param    secant  => secant (diagonal) part of the hyperbolic transformation
* @param    tangent => tangent (offdiagonal) part of the hyperbolic transformation
* @bsihdr                                                       EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint4d_applyHyperbolicReflection

(
DPoint4dP pOut0,
DPoint4dP pOut1,
DPoint4dCP pIn0,
DPoint4dCP pIn1,
double          secant,
double          tangent
)
    {
    double a, b;
    int i;
    double *p0 = (double*)pIn0;
    double *p1 = (double*)pIn1;

    for (i = 0; i < 4; i++)
        {
        a = p0[i];
        b = p1[i];
        p0[i] = a * secant + b * tangent;
        p1[i] = b * secant + a * tangent;
        }
    }


/*-----------------------------------------------------------------*//**
* Normalize a 4d point and tangent so as to produce a vector parallel
* but probably a different length from the 3d tangent.
*
* @param pTangent <= normalized point
* @param pX => homogeneous point
* @param pdX => homogeneous
* @indexVerb tangent
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint4d_pseudoTangent

(
DPoint3dP pTangent,
DPoint4dCP pX,
DPoint4dCP pdX
)
    {
    /* Formula: x = X / w               where w = X.w
                dx = (w * dX - X * dw) / w^2
                pseudoTangent = w * dX - X * dw
    */
    double w = pX->w;
    double dw = pdX->w;
    pTangent->x = w * pdX->x - pX->x * dw;
    pTangent->y = w * pdX->y - pX->y * dw;
    pTangent->z = w * pdX->z - pX->z * dw;
    }

/*---------------------------------------------------------------------------------**//**
* @description Tests if two homogeneous derivatives are parallel in homogeneous space.
* @remarks Use bsiTrig_smallAngle() for tightest tolerance.
*
* @param pDot           OUT     dot product of pseudotangets, e.g., for determining if antiparallel (or NULL)
* @param pOrigin1       IN      the first homogeneous point
* @param pDerivative1   IN      the homogeneous derivative at pOrigin1
* @param pOrigin2       IN      the second homogeneous point
* @param pDerivative2   IN      the homogeneous derivative at pOrigin2
* @param tolerance      IN      radian tolerance for angle between vectors
* @return true if the derivatives' pseudotangents are parallel within tolerance
* @see bsiDPoint4d_pseudoTangent
* @bsimethod                                                    DavidAssaf      05/06
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     bsiDPoint4d_areParallelDerivatives

(
double*         pDot,
DPoint4dCP  pOrigin1,
DPoint4dCP  pDerivative1,
DPoint4dCP  pOrigin2,
DPoint4dCP  pDerivative2,
double          tolerance
)
    {
    DPoint3d    tangent1, tangent2;
    bool        parallel;
    bsiDPoint4d_pseudoTangent (&tangent1, pOrigin1, pDerivative1);
    bsiDPoint4d_pseudoTangent (&tangent2, pOrigin2, pDerivative2);
    parallel = bsiDPoint3d_areParallelTolerance (&tangent1, &tangent2, tolerance);

    if (pDot)
        *pDot = bsiDPoint3d_dotProduct (&tangent1, &tangent2);

    return parallel;
    }


/*-----------------------------------------------------------------*//**
* @description Normalizes a homogeneous point (by dividing by w part).
*
* @param pIn => homogeneous point
* @param pOut <= normalized point
* @return true if normalization succeeded
* @group "DPoint4d Normalize"
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDPoint4d_normalize
(
DPoint4dCP  pIn,
DPoint3dP  pOut
)
    {
    bool    result = true;
    if (pIn->w == 1.0)
        {
        pOut->x = pIn->x;
        pOut->y = pIn->y;
        pOut->z = pIn->z;
        }
    else if (pIn->w == 0.0)
        {
        pOut->x = pOut->y = pOut->z = 0.0;
        result = false;
        }
    else
        {
        double a = 1.0 / pIn->w;
        pOut->x = pIn->x * a;
        pOut->y = pIn->y * a;
        pOut->z = pIn->z * a;
        }
    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @description Normalizes a quaternion in place (i.e., so that it satisfies x*x + y*y + z*z + w*w = 1).
* @remarks The process is exactly analogous to normalization of a Cartesian vector.
*
* @param pQuaternion    IN OUT  quaternion to be normalized
* @return magnitude of quaternion
* @group "DPoint4d Quaternion"
* @bsimethod                                                    DavidAssaf      07/06
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double   bsiDPoint4d_normalizeQuaternion

(
DPoint4dP    pQuaternion
)
    {
    double  magnitude = sqrt (pQuaternion->x * pQuaternion->x + pQuaternion->y * pQuaternion->y + pQuaternion->z * pQuaternion->z + pQuaternion->w * pQuaternion->w);

    if (magnitude > 0.0)
        {
        double f = 1.0 / magnitude;
        pQuaternion->x *= f;
        pQuaternion->y *= f;
        pQuaternion->z *= f;
        pQuaternion->w *= f;
        }

    return magnitude;
    }


/*-----------------------------------------------------------------*//**
* Normalizes a homogeneous point, first, and second derivatives.
*
* @instance pInstance => homogeneous point
* @param pRPoint <= normalized point
* @return true if normalization succeeded
* @indexVerb normalize
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDPoint4d_normalizePointAndDerivatives

(
DPoint3dP pX,
DPoint3dP pdX,
DPoint3dP pddX,
DPoint4dCP pZ,
DPoint4dCP pdZ,
DPoint4dCP pddZ
)
    {
    bool    result = true;
    if (pZ->w == 0.0)
        {
        if (pX)
            memset (pX, 0, sizeof(DPoint3d));
        if (pdX)
            memset (pdX, 0, sizeof(DPoint3d));
        if (pddX)
            memset (pddX, 0, sizeof(DPoint3d));
        result = false;
        }
    else
        {
        double a = 1.0 / pZ->w;
        if (pX)
            {
            pX->x = pZ->x * a;
            pX->y = pZ->y * a;
            pX->z = pZ->z * a;
            }

        if (pdX || pddX)
            {
            double aa = a * a;
            double b = 2.0 * pdZ->w * ( aa * a);
            DPoint3d Q, R;

            Q.x = pdZ->x * pZ->w - pZ->x * pdZ->w;
            Q.y = pdZ->y * pZ->w - pZ->y * pdZ->w;
            Q.z = pdZ->z * pZ->w - pZ->z * pdZ->w;

            if (pdX)
                {
                pdX->x = Q.x * aa;
                pdX->y = Q.y * aa;
                pdX->z = Q.z * aa;
                }

            if (pddX)
                {
                R.x = pddZ->x * pZ->w - pZ->x * pddZ->w;
                R.y = pddZ->y * pZ->w - pZ->y * pddZ->w;
                R.z = pddZ->z * pZ->w - pZ->z * pddZ->w;
                pddX->x = R.x * aa - Q.x * b;
                pddX->y = R.y * aa - Q.y * b;
                pddX->z = R.z * aa - Q.z * b;
                }
            }
        }
    return result;
    }



/*-----------------------------------------------------------------*//**
* Initializes the instance by normalizing the weight of the source.
*
* @instance pInstance => point to normalize
* @return true if normalization succeeded
* @indexVerb init
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool      bsiDPoint4d_initWithNormalizedWeight

(
DPoint4dP pInstance,
DPoint4dCP pSource
)
    {
    bool    result = true;

    if (pSource->w == 0.0)
        {
        *pInstance = *pSource;
        result = false;
        }
    else
        {
        double a = 1.0 / pSource->w;
        pInstance->x = pSource->x * a;
        pInstance->y = pSource->y * a;
        pInstance->z = pSource->z * a;
        pInstance->w = 1.0;
        }

    return result;
    }


/*-----------------------------------------------------------------*//**
* Divide through by weight component.
*
* @instance pInstance => point to normalize
* @return true if normalization succeeded
* @indexVerb normalize
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool      bsiDPoint4d_normalizeWeightInPlace

(
DPoint4dP pInstance
)
    {
    bool    result = true;

    if (pInstance->w == 0.0)
        {
        result = false;
        }
    else
        {
        double a = 1.0 / pInstance->w;
        pInstance->x *= a;
        pInstance->y *= a;
        pInstance->z *= a;
        pInstance->w  = 1.0;
        }

    return result;
    }


/*-----------------------------------------------------------------*//**
* Normalizes a homogeneous plane (by dividing through by the vector
* magnitude).
*
* @instance pInstance <= normalized plane
* @param pPlane0 => homogeneous plane
* @return true unless normal is zero vector.
* @indexVerb normalize
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDPoint4d_normalizePlane

(
DPoint4dP pInstance,
DPoint4dCP pPlane0
)

    {
    bool    result = true;
    double mag = sqrt  (pPlane0->x * pPlane0->x
                      + pPlane0->y * pPlane0->y
                      + pPlane0->z * pPlane0->z);

    if (mag == 0.0)
        {
        *pInstance = *pPlane0;
        result = false;
        }
    else
        {
        pInstance->x = pPlane0->x / mag;
        pInstance->y = pPlane0->y / mag;
        pInstance->z = pPlane0->z / mag;
        pInstance->w = pPlane0->w / mag;
        }
    return result;
    }



/*-----------------------------------------------------------------*//**
* Normalizes an array of  homogeneous points (by dividing by w part.)
* If w==0 is encountered, treat that point as w==1, set return value false, and
* continue with remaining points.
*
* @param pRPoint <= normalized points
* @param pHPoint => homogeneous points
* @param numPoint => number of points in both arrays
* @return false if any zero weights were detected.
* @indexVerb normalize
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     bsiDPoint4d_normalizeArray


(
DPoint3dP pRPoint,
DPoint4dCP pHPoint,
int       numPoint
)
    {
    bool    result = true;
    int i;
    double a;

    for (i = 0; i < numPoint; i++, pRPoint++, pHPoint++)
        {
        if (pHPoint->w == 0.0)
            {
            a = 1.0;
            result = false;
            }
        else
            {
            a = 1.0 / pHPoint->w;
            }

        pRPoint->x = pHPoint->x * a;
        pRPoint->y = pHPoint->y * a;
        pRPoint->z = pHPoint->z * a;
        }
    return result;
    }


/*-----------------------------------------------------------------*//**
* Normalizes an array of  homogeneous points (by dividing by w part.)
* If w==0 is encountered, treat that point as w==1, set return value false, and
* continue with remaining points.
*
* @param pPointArray <=> homogeneous points
* @param numPoint => number of points in array
* @return false if any zero weights were detected.
* @indexVerb normalize
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     bsiDPoint4d_normalizeArrayInPlace


(
DPoint4dP pPointArray,
int       numPoint
)
    {
    bool    result = true;
    DPoint4d *pPoint;
    int i;
    double a;

    for (i = 0, pPoint = pPointArray; i < numPoint; i++, pPoint++)
        {
        if (pPoint->w == 0.0)
            {
            result = false;
            }
        else
            {
            a = 1.0 / pPoint->w;
            pPoint->x *= a;
            pPoint->y *= a;
            pPoint->z *= a;
            pPoint->w = 1.0;
            }
        }
    return result;
    }


/*-----------------------------------------------------------------*//**
* sets pOutVec to pInVec*scale.
*
* @instance pInstance <= output vector
* @param pInVec => input vector
* @param scale => scale
* @indexVerb scale
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint4d_scale

(
DPoint4dP pInstance,
DPoint4dCP pPoint,
double       scale
)

    {
    pInstance->x = pPoint->x*scale;
    pInstance->y = pPoint->y*scale;
    pInstance->z = pPoint->z*scale;
    pInstance->w = pPoint->w*scale;
    }


/*-----------------------------------------------------------------*//**
* Scale a point in place.
*
* @instance pInstance <=> point to scale
* @param scale => scale factor
* @indexVerb scale
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint4d_scaleInPlace

(
DPoint4dP pInstance,
double       scale
)

    {
    pInstance->x = pInstance->x*scale;
    pInstance->y = pInstance->y*scale;
    pInstance->z = pInstance->z*scale;
    pInstance->w = pInstance->w*scale;
    }




/*-----------------------------------------------------------------*//**
* Negate a point.
*
* @instance pInstance <= output point
* @param pPoint => input point
* @indexVerb scale
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint4d_negate

(
DPoint4dP pInstance,
DPoint4dCP pPoint
)

    {
    pInstance->x = -pPoint->x;
    pInstance->y = -pPoint->y;
    pInstance->z = -pPoint->z;
    pInstance->w = -pPoint->w;
    }


/*-----------------------------------------------------------------*//**
* Negate all components of a point in place.
* @instance pInstance <=> point to negate.
* @indexVerb scale
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint4d_negateInPlace

(
DPoint4dP pInstance
)

    {
    pInstance->x = -pInstance->x;
    pInstance->y = -pInstance->y;
    pInstance->z = -pInstance->z;
    pInstance->w = -pInstance->w;
    }

    
/*-----------------------------------------------------------------*//**
* Exact equality test between points.  (Also see method with same name
* but added tolerance argument.)
*
* @instance pInstance => vector
* @param pVec2 => vector
* @return true if the points are identical.
* @indexVerb equal
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDPoint4d_pointEqual

(
DPoint4dCP pInstance,
DPoint4dCP pVec2
)
    {
    bool                    result;
    static DPoint4d     zeroVec = { 0.0, 0.0, 0.0, 0.0 };

    if (pInstance && pVec2)
        {
        result =
                   pInstance->x == pVec2->x
                && pInstance->y == pVec2->y
                && pInstance->z == pVec2->z
                && pInstance->w == pVec2->w     ;
        }
    else if (pInstance)
        {
        result = bsiDPoint4d_pointEqual (pInstance, &zeroVec);
        }
    else if (pVec2)
        {
        result = bsiDPoint4d_pointEqual (pVec2, &zeroVec);
        }
    else /* Both are null. Call them equal. */
        result = 1;

    return  result;
    }


/*-----------------------------------------------------------------*//**
* @instance pInstance => vector
* @param pVec2 => vector
* @param tolerance => tolerance
* @return true if all components are within given tolerance of each other.
* @indexVerb equal
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDPoint4d_pointEqualTolerance

(
DPoint4dCP pInstance,
DPoint4dCP pVec2,
double                  tolerance
)
    {
    bool                result;
    static DPoint4d     zeroVec = { 0.0, 0.0, 0.0, 0.0 };

    if (pInstance && pVec2)
        {
        result = fabs(pInstance->x - pVec2->x) <= tolerance &&
                 fabs(pInstance->y - pVec2->y) <= tolerance &&
                 fabs(pInstance->z - pVec2->z) <= tolerance &&
                 fabs(pInstance->w - pVec2->w) <= tolerance;
        }
    else if (pInstance)
        {
        result = bsiDPoint4d_pointEqualTolerance (pInstance, &zeroVec, tolerance);
        }
    else if (pVec2)
        {
        result = bsiDPoint4d_pointEqualTolerance (pVec2, &zeroVec, tolerance);
        }
    else /* Both are null. Call them equal. */
        result = 1;

    return  result;
    }



/*-----------------------------------------------------------------*//**
* @instance pInstance => vector
* @param pVec2 => vector
* @param tolerance => tolerance
* @param xyzTol => tolerance for absolute difference between x,y,z components.
* @param wTol => tolerance for absolute difference between w components.
* @return true if all components are within given tolerance of each other,
*       using different tolerances for xyz and w data.
* @indexVerb equal
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDPoint4d_pointEqualMixedTolerance

(
DPoint4dCP pInstance,
DPoint4dCP pVec2,
double                  xyzTol,
double                  wTol
)
    {
    bool                result;
    static DPoint4d     zeroVec = { 0.0, 0.0, 0.0, 0.0 };

    if (pInstance && pVec2)
        {
        result = fabs(pInstance->x - pVec2->x) <= xyzTol &&
                 fabs(pInstance->y - pVec2->y) <= xyzTol &&
                 fabs(pInstance->z - pVec2->z) <= xyzTol &&
                 fabs(pInstance->w - pVec2->w) <= wTol;
        }
    else if (pInstance)
        {
        result = bsiDPoint4d_pointEqualMixedTolerance (pInstance, &zeroVec, xyzTol, wTol);
        }
    else if (pVec2)
        {
        result = bsiDPoint4d_pointEqualMixedTolerance (pVec2, &zeroVec, xyzTol, wTol);
        }
    else /* Both are null. Call them equal. */
        result = true;

    return  result;
    }



/*-----------------------------------------------------------------*//**
* @instance pInstance => vector
* @return largest absoluted value among point coordinates.
* @indexVerb magnitude
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDPoint4d_maxAbs

(
DPoint4dCP pInstance
)
    {
    double maxVal = fabs (pInstance->x);
    if (fabs (pInstance->y) > maxVal)
        maxVal = fabs (pInstance->y);
    if (fabs (pInstance->z) > maxVal)
        maxVal = fabs (pInstance->z);
    if (fabs (pInstance->w) > maxVal)
        maxVal = fabs (pInstance->w);
    return maxVal;
    }



/*-----------------------------------------------------------------*//**
* @description Returns the angle of rotation represented by the quaternion and
*       sets pAxis to be the normalized vector about which the quaternion rotates.
* @remarks The quaternion is assumed to be normalized, i.e. of the form (x,y,z,w), where x*x + y*y + z*z + w*w = 1.
*
* @param pQuaternion    IN      input quaternion
* @param pAxis          OUT     normalized axis of rotation
* @return rotation angle (in radians) between 0 and Pi, inclusive
* @group "DPoint4d Quaternion"
* @bsihdr                                       DavidAssaf      6/98
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDPoint4d_getRotationAngleAndVectorFromQuaternion

(
DPoint4dCP    pQuaternion,
DPoint3dP    pAxis
)
    {
    /* Source : Hoschek & Lasser, Fundamentals of CAGD, 1993, p 10. */
    DPoint3d        axis;
    double          sinHalfAngle, angle;

    axis.x = pQuaternion->x;
    axis.y = pQuaternion->y;
    axis.z = pQuaternion->z;

    sinHalfAngle = bsiDPoint3d_normalizeInPlace (&axis);

    /* pQuat->w = cosHalfAngle */
    angle = 2.0 * bsiTrig_atan2 (sinHalfAngle, pQuaternion->w); /* range [0,2PI] */

    /* shift angle range to [0,Pi] by flipping axis and using smaller angle */
    if (angle > msGeomConst_pi)
        {
        angle = msGeomConst_2pi - angle;
        axis.x = - axis.x;
        axis.y = - axis.y;
        axis.z = - axis.z;
        }

    // no rotation => any vector will do: use 001
    if (angle == 0.0)
        {
        axis.x = axis.y = 0.0;
        axis.z = 1.0;
        }

    *pAxis = axis;
    return angle;
    }


/*-----------------------------------------------------------------*//**
* @return true if the point has coordinates which indicate it is
*   a disconnect (separator) ponit.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDPoint4d_isDisconnect

(
DPoint4dCP pPoint
)
    {
    return pPoint->x == DISCONNECT
        || pPoint->y == DISCONNECT
        || pPoint->z == DISCONNECT
        || pPoint->w == DISCONNECT;
    }


/*-----------------------------------------------------------------*//**
* Initialize a point with all coordinates as the disconnect value.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint4d_initDisconnect

(
DPoint4dP pPoint
)
    {
    pPoint->x = pPoint->y = pPoint->z = pPoint->w = DISCONNECT;
    }
END_BENTLEY_GEOMETRY_NAMESPACE
