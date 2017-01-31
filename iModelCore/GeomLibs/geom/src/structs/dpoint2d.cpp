/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/structs/dpoint2d.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_NAMESPACE


/*-----------------------------------------------------------------*//**
* @struct DPoint2d
* A DPoint2d structure holds cartesian components of a 2D vector or point.
* @fields
* @field double x x component of point or vector
* @field double y y component of point or vector
* @endfields
* @bsistruct                                        EarlinLutz      09/00
+----------------------------------------------------------------------*/


/*-----------------------------------------------------------------*//**
* @indexVerb init
* @bsihdr                                                               EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint2d_initFromFPoint2d

(
DPoint2dP pDPoint,
FPoint2dCP pFPoint
)
    {
    pDPoint->x = pFPoint->x;
    pDPoint->y = pFPoint->y;
    }

/*-----------------------------------------------------------------*//**
* @indexVerb init
* @bsihdr                                                               EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint2d_initFromFPoint3d

(
DPoint2dP pDPoint,
FPoint3dCP pFPoint
)
    {
    pDPoint->x = pFPoint->x;
    pDPoint->y = pFPoint->y;
    }



/*-----------------------------------------------------------------*//**
* Compute 2d cross product of two vectors given as components
* @param x0 => first vector x component.
* @param y0 => first vector y component.
* @param x1 => second vector x component
* @param y1 => second vector y component.
* @return (scalar) cross product of the vectors.
* @indexVerb crossProduct
* @bsihdr                                                               EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDPoint2d_crossXYXY

(
double  x0,
double  y0,
double  x1,
double  y1
)
    {
    return x0 * y1 - y0 * x1;
    }


/*-----------------------------------------------------------------*//**
* Compute 2d dot product of two vectors given as components
* @param x0 => first vector x component.
* @param y0 => first vector y component.
* @param x1 => second vector x component
* @param y1 => second vector y component.
* @return dot product of the two vectors
* @indexVerb dotProduct
* @bsihdr                                                               EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDPoint2d_dotXYXY

(
double  x0,
double  y0,
double  x1,
double  y1
)
    {
    return x0 * y0 + x1 * y1;
    }

/* VBSUB(Point2dMagnitudeSquared) */

/*-----------------------------------------------------------------*//**
* @description Compute the square of the magnitude of the vector.
* @param pVector => vector whose length is to be computed
* @return squared magnitude of the vector
* @group "DPoint2d Distance"
* @bsihdr                                                              EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDPoint2d_magnitudeSquared

(
DPoint2dCP pVector
)
    {
    return pVector->x * pVector->x + pVector->y * pVector->y;
    }

/* VBSUB(Point2dCrossProduct) */

/*-----------------------------------------------------------------*//**
* @description Returns the (scalar) cross product of two vectors.
* @param pVector1 => first vector
* @param pVector2 => second vector
* @return cross product
* @group "DPoint2d Dot and Cross"
* @bsihdr                                                              EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDPoint2d_crossProduct

(
DPoint2dCP pVector1,
DPoint2dCP pVector2
)
    {
    return pVector1->x * pVector2->y - pVector1->y * pVector2->x;
    }

/* VBSUB(Point2dCrossProduct3Points) */

/*-----------------------------------------------------------------*//**
* @description Returns the (scalar) cross product of two vectors.
* @remarks The vectors are computed from Origin to Target1 and from Origin to Target2.
* @param pOrigin => base point
* @param pTarget1 => target of first vector
* @param pTarget2 => target of second vector
* @return cross product
* @group "DPoint2d Dot and Cross"
* @bsihdr                                                              EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDPoint2d_crossProduct3DPoint2d

(
DPoint2dCP pOrigin,
DPoint2dCP pTarget1,
DPoint2dCP pTarget2
)
    {
    DPoint2d tmpVector1, tmpVector2;

    bsiDPoint2d_subtractDPoint2dDPoint2d (&tmpVector1, pTarget1, pOrigin);
    bsiDPoint2d_subtractDPoint2dDPoint2d (&tmpVector2, pTarget2, pOrigin);
    return  tmpVector1.x * tmpVector2.y - tmpVector1.y * tmpVector2.x;
    }

/* VBSUB(Point2dDotProduct) */

/*-----------------------------------------------------------------*//**
* @description Returns the (scalar) dot product of two vectors.
* @param pVector1 => first vector.
* @param pVector2 => second vector.
* @return dot product
* @group "DPoint2d Dot and Cross"
* @bsihdr                                                              EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDPoint2d_dotProduct

(
DPoint2dCP pVector1,
DPoint2dCP pVector2
)
    {
    return pVector1->x * pVector2->x + pVector1->y * pVector2->y;
    }

/* VBSUB(Point2dDotProduct3Points) */

/*-----------------------------------------------------------------*//**
* @description Returns the (scalar) dot product of two vectors.
* @remarks The vectors are computed from Origin to Target1 and from Origin to Target2.
* @param pOrigin => base point
* @param pTarget1 => target of first vector
* @param pTarget2 => target of second vector
* @return dot product
* @group "DPoint2d Dot and Cross"
* @bsihdr                                                              EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDPoint2d_dotProduct3DPoint2d

(
DPoint2dCP pOrigin,
DPoint2dCP pTarget1,
DPoint2dCP pTarget2
)
    {
    DPoint2d tmpVector1, tmpVector2;

    bsiDPoint2d_subtractDPoint2dDPoint2d (&tmpVector1, pTarget1, pOrigin);
    bsiDPoint2d_subtractDPoint2dDPoint2d (&tmpVector2, pTarget2, pOrigin);
    return  tmpVector1.x * tmpVector2.x + tmpVector1.y * tmpVector2.y;
    }

/* VBSUB(Point2dDistanceSquared) */

/*-----------------------------------------------------------------*//**
* @description Return the squared distance between two points or vectors.
* @param pPoint1 => start point
* @param pPoint2 => end point
* @return squared distance between points
* @group "DPoint2d Distance"
* @bsihdr                                                              EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDPoint2d_distanceSquared

(
DPoint2dCP pPoint1,
DPoint2dCP pPoint2
)
    {
    double dx = pPoint2->x - pPoint1->x;
    double dy = pPoint2->y - pPoint1->y;
    return dx * dx + dy * dy;
    }

/* VBSUB(Point2dAddScaled) */

/*-----------------------------------------------------------------*//**
* @description Adds an origin and a scaled vector.
* @param pSum <= pOrigin + pVector * scale
* @param pOrigin => origin.  May be null.
* @param pVector => direction vector
* @param scale => scale factor
* @group "DPoint2d Addition"
* @bsihdr                                                              EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint2d_addScaledDPoint2d

(
DPoint2dP pResult,
DPoint2dCP pOrigin,
DPoint2dCP pVector,
double    scale
)
    {
    pResult->x = pOrigin->x + pVector->x * scale;
    pResult->y = pOrigin->y + pVector->y * scale;
    }

/* VBSUB(Point2dInterpolate) */

/*-----------------------------------------------------------------*//**
* @description Compute the point at an interpolated (fractional) position between a start and end point.
* @param pResult <= interpolated point
* @param pPoint0 => start point (at parameter s=0)
* @param s => interpolation parameter
* @param pPoint1 => end point (at parameter s=1)
* @group "DPoint2d Addition"
* @bsihdr                                                              EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint2d_interpolate

(
DPoint2dP pResult,
DPoint2dCP pPoint0,
double    s,
DPoint2dCP pPoint1
)
    {
    double s0 = 1.0 - s;
    pResult->x = s0 * pPoint0->x + s * pPoint1->x;
    pResult->y = s0 * pPoint0->y + s * pPoint1->y;
    }

/* VBSUB(Point2dAdd) */

/*-----------------------------------------------------------------*//**
* @description Compute the sum of two points or vectors.
* @param pSum <= computed point or vector
* @param pPoint1 => First point or vector
* @param pPoint2 => Second point or vector
* @group "DPoint2d Addition"
* @bsihdr                                                              EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint2d_addDPoint2dDPoint2d

(
DPoint2dP pSum,
DPoint2dCP pPoint1,
DPoint2dCP pPoint2
)
    {
    pSum->x = pPoint1->x + pPoint2->x;
    pSum->y = pPoint1->y + pPoint2->y;
    }


/*-----------------------------------------------------------------*//**
* Add a vector to the instance.
*
* @instance <= pResult + pVector
* @param pVector => vector to add
* @see
* @indexVerb add
* @bsihdr                                                               EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint2d_add

(
DPoint2dP pResult,
DPoint2dCP pVector
)
    {
    pResult->x += pVector->x;
    pResult->y += pVector->y;
    }


/*-----------------------------------------------------------------*//**
* @description Subtract one vector from another in place.
*
* @instance <= pResult - pVector
* @param pPoint => vector to subtract
* @see
* @indexVerb subtract
* @bsihdr                                                               EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint2d_subtract

(
DPoint2dP pResult,
DPoint2dCP pVector
)
    {
    pResult->x -= pVector->x;
    pResult->y -= pVector->y;
    }

/* VBSUB(Point2dSubtract) */

/*-----------------------------------------------------------------*//**
* @description Return the difference of two points or vectors.
* @param pResult <= pPoint1 - pPoint2
* @param pPoint1 => First point or vector.
* @param pPoint2 => Second (subtracted) point or vector.
* @group "DPoint2d Subtraction"
* @bsihdr                                                              EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint2d_subtractDPoint2dDPoint2d

(
DPoint2dP pResult,
DPoint2dCP pPoint1,
DPoint2dCP pPoint2
)
    {
    pResult->x = pPoint1->x - pPoint2->x;
    pResult->y = pPoint1->y - pPoint2->y;
    }


/*-----------------------------------------------------------------*//**
* @description Scale a vector.
* @param pResult <= scaled vector
* @param pPoint => vector to scale
* @param s    => scale factor
* @group "DPoint2d Scale"
* @bsihdr                                                              EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint2d_scale

(
DPoint2dP pResult,
DPoint2dCP pPoint,
double    s
)
    {
    pResult->x = pPoint->x * s;
    pResult->y = pPoint->y * s;
    }


/*-----------------------------------------------------------------*//**
* @description Normalizes a given vector in place.
* @remarks If the input vector length is 0, the output vector is the zero vector.
* @param pVec <=> vector to normalize
* @return original length
* @group "DPoint2d Normalize"
* @bsihdr                                                              EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDPoint2d_normalize

(
DPoint2dP pVec
)
    {
    double magnitude = sqrt (pVec->x*pVec->x + pVec->y*pVec->y);

    if (magnitude > 0.0)
        {
        double f = 1.0 / magnitude;
        pVec->x *= f;
        pVec->y *= f;
        }

    return  magnitude;
    }


/*-----------------------------------------------------------------*//**
* @description Computes a unit vector in the direction of a given vector.
* @remarks If the input vector length is 0, the output vector is the zero vector.
* @param pUnitVector <= normalized vector
* @param pVector => vector to normalize
* @return original length
* @group "DPoint2d Normalize"
* @bsihdr                                                              EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDPoint2d_normalizeVector

(
DPoint2dP pUnitVector,
DPoint2dCP pVector
)
    {
    double magnitude = sqrt (pVector->x*pVector->x + pVector->y*pVector->y);

    *pUnitVector = *pVector;
    if (magnitude > 0.0)
        {
        double f = 1.0 / magnitude;
        pUnitVector->x *= f;
        pUnitVector->y *= f;
        }

    return magnitude;
    }

/* VBSUB(Point2dNormalize) */
/*-----------------------------------------------------------------*//**
* @description Computes a unit vector in the direction of a given vector.
* @remarks If the input vector length is 0, the output vector is the zero vector.
* @param pUnitVector <= normalized vector
* @param pVector => vector to normalize
* @group "DPoint2d Normalize"
* @bsihdr                                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiDPoint2d_unitVector

(
DPoint2dP pUnitVector,
DPoint2dCP pVector
)
    {
    bsiDPoint2d_normalizeVector (pUnitVector, pVector);
    }


/*-----------------------------------------------------------------*//**
* @description Rotate a vector by 90 degrees in the counterclockwise direction.
* @param pRot <= rotated vector (may be same as pVec)
* @param pVec => original vector
* @group "DPoint2d Angles"
* @bsihdr                                                              EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint2d_rotate90

(
DPoint2dP pRot,
DPoint2dCP pVec
)
    {
    double x = -pVec->y;        /* Local copies to allow in-place udpate */
    double y = pVec->x;
    pRot->x = x;
    pRot->y = y;
    }


/*-----------------------------------------------------------------*//**
* @description Rotate a vector by an angle.
* @param pRot <= vector rotated by the given angle (may be same as pVec)
* @param pVec => original vector
* @param radians => radian rotation angle (positive: counterclockwise; negative: clockwise)
* @group "DPoint2d Angles"
* @bsihdr                                                              EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint2d_rotate

(
DPoint2dP pRot,
DPoint2dCP pVec,
double  radians
)
    {
    double c = cos (radians);
    double s = sin (radians);

    double x = pVec->x;        /* Local copies to allow in-place udpate */
    double y = pVec->y;
    pRot->x =  c * x + s * y;
    pRot->y = -s * x + c * y;
    }

/* VBSUB(Point2dDotDifference) */

/*-----------------------------------------------------------------*//**
* @description Returns the (scalar) dot product of two vectors.
* @remarks One vector is computed internally as the difference TargetPoint - Origin.
*   The other is given directly as a single parameter.
* @param pTargetPoint => end of computed vector
* @param pOrigin => start point of computed vector
* @param pVector => final vector
* @return dot product
* @group "DPoint2d Dot and Cross"
* @bsihdr                                                              EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDPoint2d_dotDifference

(
DPoint2dCP pTargetPoint,
DPoint2dCP pOrigin,
DPoint2dCP pVector
)
    {
    return
            (pTargetPoint->x - pOrigin->x) * pVector->x
        +   (pTargetPoint->y - pOrigin->y) * pVector->y;
    }

/* VBSUB(Point2dZero) */

/*-----------------------------------------------------------------*//**
* @description Sets all components of a point or vector to zero.
* @instance pVec <= vector to zero
* @group "DPoint2d Initialization"
* @bsihdr                                                              EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint2d_zero

(
DPoint2dP pVec
)
    {
    pVec->x = 0.0;
    pVec->y = 0.0;
    }

/* VBSUB(Point2dOne) */

/*-----------------------------------------------------------------*//**
* @description Returns a point or vector with all components 1.0.
* @instance pPoint <= The initialized point or vector.
* @bsihdr                                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint2d_one

(
DPoint2dP pPoint
)
    {
    pPoint->x = 1.0;
    pPoint->y = 1.0;
    }



/*-----------------------------------------------------------------*//**
* copy 2 components (xy) from a double array to the DPoint2d
* @instance pPoint <= point whose components are set
* @param pXy => x, y components
* @see
* @indexVerb init
* @bsihdr                                                              EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint2d_initFromArray

(
DPoint2dP pPoint,
const   double      *pXy
)

    {
    pPoint->x = pXy[0];
    pPoint->y = pXy[1];
    }


/*-----------------------------------------------------------------*//**
* @param pSource => x, y components
* @see
* @indexVerb init
* @bsihdr                                                              EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint2d_initFromDPoint3d

(
DPoint2dP pPoint,
DPoint3dCP pSource
)

    {
    pPoint->x = pSource->x;
    pPoint->y = pSource->y;
    }


/* VBSUB(Point2dFromXY) */

/*-----------------------------------------------------------------*//**
* @description Returns a point with specified x and y components.
* @param pPoint <= point whose components are set
* @param x => x component
* @param y => y component
* @group "DPoint2d Initialization"
* @bsihdr                                                              EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint2d_setComponents

(
DPoint2dP pPoint,
double       x,
double       y
)
    {
    pPoint->x = x;
    pPoint->y = y;
    }


/*-----------------------------------------------------------------*//**
* @description Sets a single component of a point.
* @remarks If the index is neither 0 nor 1, it is interpreted cyclically.
* @param pPoint <= point or vector whose component is altered.
* @param a => value of component
* @param index => index of component (0=x, 1=y, others cyclic)
* @group "DPoint2d Modification"
* @bsihdr                                                               EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint2d_setComponent

(
DPoint2dP pPoint,
double       a,
int         index
)
    {
    index = index & 0x01;
    if (index == 0)
        {
        pPoint->x = a;
        }
    else /* if (index == 1) */
        {
        pPoint->y = a;
        }
    }

/* VBSUB(Point2dGetComponent) */

/*-----------------------------------------------------------------*//**
* @description Gets a single component of a point.  If the index is out of
* range 0,1, it is interpreted cyclically.
*
* @instance pPoint => point or vector whose components are accessed.
* @param index => 0=x, 1=y, others cyclic
* @return specified component of the point or vector.
* @indexVerb get
* @bsihdr                                                               EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDPoint2d_getComponent

(
DPoint2dCP pPoint,
int         index
)
    {
    index = index & 0x01;
    if (index == 0)
        {
        return pPoint->x;
        }
    else /* (index == 1) */
        {
        return pPoint->y;
        }
    }


/*-----------------------------------------------------------------*//**
* @description Copy the array of points.
* @param pDest <= destination array
* @param pSource => source array
* @param n => number of points in arrays
* @group "DPoint2d Copy"
* @bsihdr                                                              EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint2d_copyArray

(
DPoint2dP pDest,
DPoint2dCP pSource,
int          n
)
    {
    memcpy (pDest, pSource, n*sizeof(DPoint2d) );
    }


/*-----------------------------------------------------------------*//**
* @description Copy indexed points.
* @param pDest <= destination array
* @param pSource => source array
* @param pIndex => array of 0-based indices into source array
* @param nIndex => number of array entries
* @group "DPoint2d Copy"
* @bsihdr                                                              EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint2d_copyIndexedArray

(
DPoint2dP pDest,
DPoint2dCP pSource,
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
* @description Extract the coordinates of the point.
* @param pPoint => point whose components are retrieved
* @param pXCoord <= x component
* @param pYCoord <= y component
* @group "DPoint2d Queries"
* @bsihdr                                                              EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint2d_getComponents

(
DPoint2dCP pPoint,
double      *pXCoord,
double      *pYCoord
)
    {
    *pXCoord = pPoint->x;
    *pYCoord = pPoint->y;
    }


/*-----------------------------------------------------------------*//**
* @description Add two arrays of points.
* @param pArray <=> Array whose points are to be incremented
* @param pDelta => increment to add to each point
* @param numPoints => number of points
* @group "DPoint2d Addition"
* @bsihdr                                                              EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint2d_addDPoint2dArray

(
DPoint2dP pArray,
DPoint2dCP pDelta,
int              numPoints
)
    {
    int         i;
    DPoint2d   *pPoint = pArray;
    double      x = pDelta->x;
    double      y = pDelta->y;

    for (i=0; i < numPoints; i++)
        {
        pPoint->x += x;
        pPoint->y += y;
        pPoint++;
        }
    }


/*-----------------------------------------------------------------*//**
* @description Subtract two arrays of points.
* @param pArray <=> Array whose points are to be decremented
* @param pDelta => decrement to subtract from each point
* @param numVerts => number of points
* @group "DPoint2d Subtraction"
* @bsihdr                                                              EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint2d_subtractDPoint2dArray

(
DPoint2dP pArray,
DPoint2dCP pDelta,
int              numVerts
)
    {
    int         i;
    DPoint2d   *pPoint = pArray;
    double      x = pDelta->x;
    double      y = pDelta->y;

    for (i=0; i<numVerts; i++)
        {
        pPoint->x -= x;
        pPoint->y -= y;
        pPoint++;
        }
    }

/* VBSUB(Point2dAdd2Scaled) */

/*-----------------------------------------------------------------*//**
* @description Adds an origin and two scaled vectors.
* @param pSum <= pOrigin + pVector1 * scale1 + pVector2 * scale2
* @param pOrigin => origin.  May be null.
* @param pVector1 => direction vector
* @param scale1 => scale factor
* @param pVector2 => direction vector
* @param scale2 => scale factor
* @group "DPoint2d Addition"
* @bsihdr                                                              EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint2d_add2ScaledDPoint2d

(
DPoint2dP pSum,
DPoint2dCP pOrigin,
DPoint2dCP pVector1,
double           scale1,
DPoint2dCP pVector2,
double           scale2
)
    {
    if (pOrigin)
        {
        pSum->x = pOrigin->x + pVector1->x * scale1 + pVector2->x * scale2;
        pSum->y = pOrigin->y + pVector1->y * scale1 + pVector2->y * scale2;
        }
    else
        {
        pSum->x = pVector1->x * scale1 + pVector2->x * scale2;
        pSum->y = pVector1->y * scale1 + pVector2->y * scale2;
        }
    }

/* VBSUB(Point2dAdd3Scaled) */

/*-----------------------------------------------------------------*//**
* @description Adds an origin and three scaled vectors.
* @param pSum <= pOrigin + pVector1 * scale1 + pVector2 * scale2 + pVector3 * scale3
* @param pOrigin => origin.  May be null.
* @param pVector1 => direction vector
* @param scale1 => scale factor
* @param pVector2 => direction vector
* @param scale2 => scale factor
* @param pVector3 => direction vector
* @param scale3 => scale factor
* @group "DPoint2d Addition"
* @bsihdr                                                              EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint2d_add3ScaledDPoint2d

(
DPoint2dP pSum,
DPoint2dCP pOrigin,
DPoint2dCP pVector1,
double          scale1,
DPoint2dCP pVector2,
double          scale2,
DPoint2dCP pVector3,
double          scale3
)
    {
    DPoint2d sum;
    sum.x = pVector1->x * scale1 + pVector2->x * scale2 + pVector3->x * scale3;
    sum.y = pVector1->y * scale1 + pVector2->y * scale2 + pVector3->y * scale3;
    if (pOrigin)
        {
        sum.x += pOrigin->x;
        sum.y += pOrigin->y;
        }
    *pSum = sum;
    }


/*-----------------------------------------------------------------*//**
* @description Sets pNormal to the unit vector in the direction of pPoint1 - pPoint2.
* @param pNormal <= normalized vector from pPoint2 to pPoint1
* @param pPoint1 => point 1
* @param pPoint2 => point 2
* @return distance between input points
* @group "DPoint2d Subtraction"
* @bsihdr                                                              EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDPoint2d_computeNormal

(
DPoint2dP pNormal,
DPoint2dCP pPoint1,
DPoint2dCP pPoint2
)
    {
    bsiDPoint2d_subtractDPoint2dDPoint2d (pNormal, pPoint1, pPoint2);
    return bsiDPoint2d_normalize (pNormal);
    }

/* VBSUB(Point2dSignedAngleBetweenVectors) */

/*-----------------------------------------------------------------*//**
* @description Returns (signed, counterclockwise) angle between two vectors.
* @remarks The angle is in radians. The angle range is from -pi to +pi; positive
*   angles are counterclockwise, negative angles are clockwise.
* @instance pVector1 => first vector
* @param pVector2 => second vector
* @return angle in radians
* @group "DPoint2d Angles"
* @bsihdr                                                              EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDPoint2d_angleBetweenVectors

(
DPoint2dCP pVector1,
DPoint2dCP pVector2
)
    {
    double cross, dot;
    cross = bsiDPoint2d_crossProduct (pVector1, pVector2);
    dot     = bsiDPoint2d_dotProduct (pVector1, pVector2);
    return  bsiTrig_atan2 (cross, dot);
    }

/* VBSUB(Point2dDistance) */

/*-----------------------------------------------------------------*//**
* @description Returns the distance between 2 points
* @param pPoint0 => first point
* @param pPoint1 => second point
* @return distance from point 0 to point 1
* @group "DPoint2d Distance"
* @bsihdr                                                              EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDPoint2d_distance

(
DPoint2dCP pPoint0,
DPoint2dCP pPoint1
)
    {
    double      xdist, ydist;
    xdist = (pPoint1->x - pPoint0->x);
    ydist = (pPoint1->y - pPoint0->y);

    return (sqrt (xdist*xdist + ydist*ydist));
    }

/* VBSUB(Point2dMagnitude) */

/*-----------------------------------------------------------------*//**
* @description Returns the magnitude (length) of a vector.
* @param pVector => vector
* @return Length of the vector.
* @group "DPoint2d Distance"
* @bsihdr                                                              EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDPoint2d_magnitude

(
DPoint2dCP pVector
)
    {
    return (sqrt (pVector->x * pVector->x + pVector->y * pVector->y));
    }

/* VBSUB(Point2dNegate) */

/*-----------------------------------------------------------------*//**
* @description Returns the negative of a vector.
* @param pResult <= negated vector
* @param pVector => input
* @group "DPoint2d Scale"
* @bsihdr                                                              EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint2d_negate

(
DPoint2dP pResult,
DPoint2dCP pVector
)
    {
    pResult->x = - pVector->x;
    pResult->y = - pVector->y;
    }

/* VBSUB(Point2dAreVectorsParallel) */

/*-----------------------------------------------------------------*//**
* @description Test if two vectors are parallel.
* @param pVector1 => First vector
* @param pVector2 => Second vector
* @return true if vectors are (nearly) parallel.
* @group "DPoint2d Angles"
* @bsihdr                                                              EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDPoint2d_areParallel

(
DPoint2dCP pVector1,
DPoint2dCP pVector2
)

    {
    double      a2 = bsiDPoint2d_dotProduct (pVector1, pVector1);
    double      b2 = bsiDPoint2d_dotProduct (pVector2, pVector2);
    double      cross;
    double      eps = bsiTrig_smallAngle(); /*  a small angle tolerance
                                            (in radians) */
    cross = bsiDPoint2d_crossProduct (pVector1, pVector2);

    return  cross * cross <= eps * eps * a2 * b2;
    }

/* VBSUB(Point2dAreVectorsPerpendicular) */

/*-----------------------------------------------------------------*//**
* @description Test if two vectors are perpendicular.
* @instance pVector1 => First vector
* @param pVector2 => Second vector
* @return true if vectors are (nearly) parallel.
* @indexVerb angle
* @bsihdr                                                              EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDPoint2d_arePerpendicular

(
DPoint2dCP pVector1,
DPoint2dCP pVector2
)

    {
    double      a2 = bsiDPoint2d_dotProduct (pVector1, pVector1);
    double      b2 = bsiDPoint2d_dotProduct (pVector2, pVector2);
    double      cross;
    double      eps = bsiTrig_smallAngle(); /*  a small angle tolerance
                                            (in radians) */
    cross = bsiDPoint2d_dotProduct (pVector1, pVector2);

    return  cross * cross <= eps * eps * a2 * b2;
    }


/*-----------------------------------------------------------------*//**
* @description Sets pOutVec[i] to pOrigin + scale*pInVec[i], for 0 &le; i &lt; numPoint.
* @param pOutVec <= output array
* @param pOrigin => origin for points
* @param pInVec => input array
* @param numPoint => number of points in arrays
* @param scale => scale
* @group "DPoint2d Addition"
* @bsihdr                                                              EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint2d_addScaledDPoint2dArray

(
DPoint2dP pOutVec,
DPoint2dCP pOrigin,
DPoint2dCP pInVec,
int          numPoint,
double       scale
)
    {
        int         i;
        DPoint2d   *pDest;
const   DPoint2d   *pSource;
        double      x0, y0;

    x0 = pOrigin->x;
    y0 = pOrigin->y;

    for (i = 0, pDest = pOutVec, pSource = pInVec ;
         i < numPoint; i++, pSource++, pDest++)
        {
        pDest->x = x0 + pSource->x * scale;
        pDest->y = y0 + pSource->y * scale;
        }
    }

/* VBSUB(Point2dEqual) */

/*-----------------------------------------------------------------*//**
* @description Test if two points or vectors are exactly equal.
* @param pVector1 => First point or vector
* @param pVector2 => Second point or vector
* @return true if the points are identical.
* @group "DPoint2d Equal"
* @bsihdr                                                               EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDPoint2d_pointEqual

(
DPoint2dCP pVector1,
DPoint2dCP pVector2
)
    {
    bool    result;
    static DPoint2d     zeroVec = { 0.0, 0.0};

    if (pVector1 && pVector2)
        {
        result =   pVector1->x == pVector2->x
                && pVector1->y == pVector2->y   ;
        }
    else if (pVector1)
        {
        result = bsiDPoint2d_pointEqual (pVector1, &zeroVec);
        }
    else if (pVector2)
        {
        result = bsiDPoint2d_pointEqual (pVector2, &zeroVec);
        }
    else /* Both are null. Call them equal. */
        result = true;

    return  result;
    }

/* VBSUB(Point2dEqualTolerance) */

/*-----------------------------------------------------------------*//**
* @description Test if the x and y components of two points or vectors are equal within tolerance.
* @remarks Tests are done against the absolute value of <EM>each</EM> component difference
*   (i.e., not against the sum of these absolute differences or the square root of the sum of the squares of these differences).
* @param pVector1 => first point or vector
* @param pVector2 => second point or vector
* @param tolerance => tolerance
* @return true if all components are within given tolerance of each other.
* @group "DPoint2d Equal"
* @bsihdr                                                               EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDPoint2d_pointEqualTolerance

(
DPoint2dCP pVector1,
DPoint2dCP pVector2,
double                  tolerance
)
    {
    bool                result;
    static DPoint2d     zeroVec = { 0.0, 0.0 };

    if (pVector1 && pVector2)
        {
        result = fabs(pVector1->x - pVector2->x) <= tolerance &&
                 fabs(pVector1->y - pVector2->y) <= tolerance;
        }
    else if (pVector1)
        {
        result = bsiDPoint2d_pointEqualTolerance (pVector1, &zeroVec, tolerance);
        }
    else if (pVector2)
        {
        result = bsiDPoint2d_pointEqualTolerance (pVector2, &zeroVec, tolerance);
        }
    else /* Both are null. Call them equal. */
        result = 1;

    return  result;
    }

/* VBSUB(Point2dMaxAbs) */

/*-----------------------------------------------------------------*//**
* @description Finds the largest absolute value among the components of a point or vector.
* @param pVector => point or vector
* @return largest absolute value among point coordinates.
* @group "DPoint2d Queries"
* @bsihdr                                                               EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDPoint2d_maxAbs

(
DPoint2dCP pVector
)
    {
    double maxVal = fabs (pVector->x);
    if (fabs (pVector->y) > maxVal)
        maxVal = fabs (pVector->y);
    return maxVal;
    }

typedef enum
    {
    CLOSURE_POINT           = 0,
    BOTTOM_LEFT_CHAMFER     = 1,
    BOTTOM_OFFSET           = 2,
    BOTTOM_RIGHT_CHAMFER    = 3,
    RIGHT_BEVEL             = 4,
    TOP_RIGHT_CHAMFER       = 5,
    TOP_OFFSET              = 6,
    TOP_LEFT_CHAMFER        = 7,
    LEFT_BEVEL              = 8
    } _EdgeId;

/*-----------------------------------------------------------------*//**
* Clip a convex, closed linestring to a plane defined by a reference point,
* unit normal, and offset.
* @param ux => outward normal component
* @param uy => outward normal component
* @return false if point buffer limit exceeded.
* @indexVerb
* @bsihdr                                                               EarlinLutz      10/98
+---------------+---------------+---------------+---------------+------*/
static bool    clipConvexToOffsetPlane

(
DPoint2dP xy,
int       *pLabel,
DPoint2dP xyWork,
int       *pLabelWork,
int       *pNumPoint,
int       maxOut,
double    x0,
double    y0,
double    ux,
double    uy,
double    a,
int       chamferLabel
)
    {
    int k;
    int numOut = 0;
    int numIn = *pNumPoint;
    double h0 = 0.0, h1, s;
    int numBelow = 0;

    /* Copy live parts to work array */
    for (k = 0; k < numIn && numOut < maxOut; k++, h0 = h1)
        {
        h1 = ux * (xy[k].x - x0) + uy * (xy[k].y - y0) - a;

        if (k > 0 && h1 * h0 < 0.0)
            {
            s = -h0 / (h1 - h0);
            if (numOut >= maxOut)
                return false;
            xyWork[numOut].x = (1.0 - s) * xy[k-1].x + s * xy[k].x;
            xyWork[numOut].y = (1.0 - s) * xy[k-1].y + s * xy[k].y;
            if (h0 < 0.0)
                pLabelWork[numOut] = chamferLabel;
            else
                pLabelWork[numOut] = pLabel[k-1];
            numOut++;
            }

        if (h1 <= 0.0)
            {
            if (numOut >= maxOut)
                return false;
            pLabelWork[numOut] = pLabel[k];
            xyWork[numOut] = xy[k];
            numOut++;
            numBelow++;
            }
        }

    if (  xyWork[0].x != xyWork[numOut-1].x
       && xyWork[0].y != xyWork[numOut-1].y)
        {
        if (numOut >= maxOut)
            return false;
        pLabelWork[numOut] = CLOSURE_POINT;
        xyWork[numOut] = xyWork[0];
        numOut++;
        }

    if (numBelow != numIn || numIn != numOut)
        {
        /* Data was changed by the clip.  Copy back. */
        for (k = 0; k < numOut; k++)
            {
            xy[k] = xyWork[k];
            pLabel[k] = pLabelWork[k];
            }
        }

    *pNumPoint = numOut;
    return true;
    }


/*-----------------------------------------------------------------*//**
* Computes a "beveled" polygon due to offseting a single edge, using
* 0 or 1 adjacent edges before and after to determine bevel conditions.
* @param pPointArray <= array of (typically 3, 5, 6, or 7) points of polygon, with first/last
*               point duplicated.
* @param pLabelArray <= (optional) parallel array of labels describing the origin of each edge.
*       If the main edge is envisioned as going from left to right, these labels distinguish
*       parts of the "lower" and "upper" offset paths and the ends as follows, with
*       the sequence showing counterclockwise order of the path parts:
*           0 -- closing point of loop (duplicates start point)
*           1 -- start point of a chamfer edge at the left the lower offset.
*           2 -- start point of the main edge of the lower offset.
*           3 -- start point of a chamfer edge at the right of the lower offset.
*           4 -- start point of a bevel cut at pPoint1.
*           5 -- start point of a chamfer at right of upper offset.
*           6 -- start point of a the main upper offset edge
*           7 -- start point of a chamfer edge at left of upper offset.
*           8 -- start point of a bevel cut at pPoint0.
*       (Note that codes 1,2,3 are lower offset, and 5,6,7 are upper. The return from a
*       single call to this function can have only 6 of the 8 codes.)
* @param pNumPoint <= number of points.
* @param maxPoint => allocated size of pPointArray
* @param pPoint0 => optional pre-edge context point.  If null, a flush (90 degree)
*                       bevel condition is used.
* @param pPoint1 => start of edge
* @param pPoint2 => end of edge.
* @param pPoint3 => optional post-edge context point.  If null, a flush (90 degree)
*                       bevel condition is used.
* @param offset  => offset distance.
* @param maxBevelAngle => maximum turn angle, in radians, for which a simple bevel is computed.
*                   (Try 1.6 to start)
* @return false if zero-length vectors prevented calculation.
* @indexVerb offset
* @bsihdr                                                               EarlinLutz      10/98
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDPoint2d_labeledBevelOffset

(
DPoint2dP pPointArray,
int       *pLabelArray,
int       *pNumPoint,
int       maxPoint,
DPoint2dCP pPoint0,
DPoint2dCP pPoint1,
DPoint2dCP pPoint2,
DPoint2dCP pPoint3,
double    offset,
double    maxBevelAngle
)
    {
    static double s_maxBevelAngle = 3.0;
    static double s_panicAngle = 3.141; /* A little less than pi */
    DPoint2d U0, U1, U2, V1;
#define MAX_POLYPOINT 10
    DPoint2d uv[MAX_POLYPOINT], uvWork[MAX_POLYPOINT];
    int label[MAX_POLYPOINT], labelWork[MAX_POLYPOINT];
    bool    intersectionOK;
    double d0, d1, d2;
    double theta1, theta2;
    double xx, yy;
    double alpha1, alpha2;
    double c1, s1, c2, s2;
    double u, v;
    int i;
    int k;
    double t1, t2;

    *pNumPoint = 0;

    if (maxPoint < 7)
        return false;

    if (maxBevelAngle <= 0.0 || maxBevelAngle > s_maxBevelAngle)
        maxBevelAngle = s_maxBevelAngle;

    d1 = bsiDPoint2d_computeNormal (&U1, pPoint2, pPoint1);

    if (d1 <= 0.0)
        return false;

    if (pPoint0)
        {
        d0 = bsiDPoint2d_computeNormal (&U0, pPoint1, pPoint0);
        if (d0 <= 0.0)
            {
            theta1 = 0.0;
            }
        else
            theta1 = bsiDPoint2d_angleBetweenVectors (&U0, &U1);
        }
    else
        {
        U0 = U1;
        theta1 = 0.0;
        }

    if (pPoint3)
        {
        d2 = bsiDPoint2d_computeNormal (&U2, pPoint3, pPoint2);
        if (d2 <= 0.0)
            {
            theta2 = 0.0;
            }
        else
            theta2 = bsiDPoint2d_angleBetweenVectors (&U1, &U2);
        }
    else
        {
        U2 = U1;
        theta2 = 0.0;
        }

    if (theta1 >  s_panicAngle)
        theta1 =  s_panicAngle;

    if (theta1 < -s_panicAngle)
        theta1 = -s_panicAngle;

    if (theta2 >  s_panicAngle)
        theta2 =  s_panicAngle;

    if (theta2 < -s_panicAngle)
        theta2 = -s_panicAngle;

    alpha1 = msGeomConst_piOver2 - 0.5 * theta1;
    alpha2 = msGeomConst_piOver2 + 0.5 * theta2;

    /* Vector pointing upwards (at the bevel angle) from start */
    c1 = cos (alpha1);
    s1 = sin (alpha1);

    /* Vector pointing upwards (at the bevel angle) from end */
    c2 = cos (alpha2);
    s2 = sin (alpha2);
    bsiTrig_safeDivide (&t1, c1, s1, 100.0 * c1);
    bsiTrig_safeDivide (&t2, c2, s2, 100.0 * c2);


    /* Define two lines in a nice coordinate system (x along edge 12, y perpendicular)
        1) Through origin, at angle alpha1 from x axis.  y = x s1/c1
        2) Through (d1,0) at angle alpha2 from x axis.   y = (x - d1) s2/c2
      If these intersect
    */

    k = 0;
    intersectionOK = bsiSVD_solve2x2 (&xx, &yy, s1, -c1, s2, -c2, 0.0, s2*d1);
    if (!intersectionOK || (intersectionOK && fabs (yy) > offset))
        {
        bsiDPoint2d_setComponents (uv + k, offset * t1,  offset);
        label[k] = LEFT_BEVEL;
        k++;
        bsiDPoint2d_setComponents (uv + k,-offset * t1, -offset);
        label[k] = BOTTOM_OFFSET;
        k++;
        bsiDPoint2d_setComponents (uv + k,d1 - offset * t2, -offset);
        label[k] = RIGHT_BEVEL;
        k++;
        bsiDPoint2d_setComponents (uv + k,d1 + offset * t2, offset);
        label[k] = TOP_OFFSET;
        k++;
        }
    else if (0.0 < yy)
        {
        bsiDPoint2d_setComponents (uv + k, xx, yy);
        label[k] = LEFT_BEVEL;
        k++;
        bsiDPoint2d_setComponents (uv + k,-offset * t1, -offset);
        label[k] = BOTTOM_OFFSET;
        k++;
        bsiDPoint2d_setComponents (uv + k,d1 - offset * t2, -offset);
        label[k] = RIGHT_BEVEL;
        k++;
        }
    else /* (-offset < yy < 0.0) */
        {
        bsiDPoint2d_setComponents (uv + k, offset * t1,  offset);
        label[k] = LEFT_BEVEL;
        k++;
        bsiDPoint2d_setComponents (uv + k, xx, yy);
        label[k] = BOTTOM_OFFSET;
        k++;
        bsiDPoint2d_setComponents (uv + k,d1 + offset * t2, offset);
        label[k] = RIGHT_BEVEL;
        k++;
        }
    uv[k] = uv[0];
    label[k] = CLOSURE_POINT;
    k++;

    if (fabs (theta1) > maxBevelAngle)
        {
        /* Clip the polygon to a plane offset from the start */
        double sign = c1 > 0.0 ? -1.0 : 1.0;
        clipConvexToOffsetPlane (uv, label, uvWork, labelWork,
                    &k, MAX_POLYPOINT, 0.0, 0.0, sign * c1, sign * s1,
                    offset,
                    sign * s1 > 0 ? TOP_LEFT_CHAMFER : BOTTOM_LEFT_CHAMFER
                    );
        }

    if (fabs (theta2) > maxBevelAngle)
        {
        /* Clip the polygon to a plane offset from the end */
        double sign = c2 > 0.0 ? 1.0 : -1.0;
        clipConvexToOffsetPlane (uv, label, uvWork, labelWork,
                    &k, MAX_POLYPOINT,  d1, 0.0, sign * c2, sign * s2,
                    offset,
                    sign * s2 > 0 ? TOP_RIGHT_CHAMFER : BOTTOM_RIGHT_CHAMFER
                    );
        }

    bsiDPoint2d_rotate90 (&V1, &U1);
    *pNumPoint = k;
    for (i = 0; i < k; i++)
        {
        u = uv[i].x;
        v = uv[i].y;
        pPointArray[i].x = pPoint1->x + U1.x * u + V1.x * v;
        pPointArray[i].y = pPoint1->y + U1.y * u + V1.y * v;
        if (pLabelArray)
            pLabelArray[i] = label[i];
        }
    return true;
    }


/*-----------------------------------------------------------------*//**
* Computes a "beveled" polygon due to offseting a single edge, using
* 0 or 1 adjacent edges before and after to determine bevel conditions.
* @param pPointArray <= array of (typically 3, 5, 6, or 7) points of polygon, with
* @param pNumPoint <= number of points.
* @param maxPoint => allocated size of pPointArray
* @param pPoint0 => optional pre-edge context point.  If null, a flush (90 degree)
*                       bevel condition is used.
* @param pPoint1 => start of edge
* @param pPoint2 => end of edge.
* @param pPoint3 => optional post-edge context point.  If null, a flush (90 degree)
*                       bevel condition is used.
* @param offset  => offset distance.
* @param maxBevelAngle => maximum turn angle, in radians, for which a simple bevel is computed.
*                   (Try 1.6 to start)
* @return false if zero-length vectors prevented calculation.
* @indexVerb offset
* @bsihdr                                                               EarlinLutz      10/98
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDPoint2d_bevelOffset

(
DPoint2dP pPointArray,
int       *pNumPoint,
int       maxPoint,
DPoint2dCP pPoint0,
DPoint2dCP pPoint1,
DPoint2dCP pPoint2,
DPoint2dCP pPoint3,
double    offset,
double    maxBevelAngle
)
    {
    return bsiDPoint2d_labeledBevelOffset (pPointArray, NULL, pNumPoint, maxPoint,
                    pPoint0, pPoint1, pPoint2, pPoint3, offset, maxBevelAngle);
    }

/*-----------------------------------------------------------------*//**
* @return true if the point has coordinates which indicate it is
*   a disconnect (separator) ponit.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDPoint2d_isDisconnect

(
DPoint2dCP pPoint
)
    {
    return pPoint->x == DISCONNECT
        || pPoint->y == DISCONNECT;
    }


/*-----------------------------------------------------------------*//**
* Initialize a point with all coordinates as the disconnect value.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint2d_initDisconnect

(
DPoint2dP pPoint
)
    {
    pPoint->x = pPoint->y = DISCONNECT;
    }

/*---------------------------------------------------------------------------------**//**
* @description Compute a unit vector perpendicular to the given vector.
*
* @remarks Input may equal output.
*
* @param pRotatedVector <=      The rotated and scaled vector.
* @param pVector        =>      The source vector
* @return true if the input vector has nonzero length
* @bsimethod                                                    DavidAssaf      04/05
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     bsiDPoint2d_unitPerpendicular

(
DPoint2dP    pRotatedVector,
DPoint2dCP    pVector
)
    {
    double  a, d2, x = pVector->x, y = pVector->y;
    pRotatedVector->x = -y;
    pRotatedVector->y =  x;
    d2 = x * x + y * y;
    if (d2 == 0.0)
        return false;
    a = 1.0 / sqrt (d2);
    pRotatedVector->x *= a;
    pRotatedVector->y *= a;
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @description Compute the unit vector perpendicular to the given vector with the given handedness.
*
* @remarks Input may equal output.
*
* @param pRotatedVector <=      The rotated and scaled vector.
* @param pVector        =>      The source vector
* @param bRightHanded   =>      Whether the returned vector points to the right of the given vector.
* @return true if the input vector has nonzero length
* @bsimethod                                                    DavidAssaf      04/05
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     bsiDPoint2d_unitPerpendicularWithHandedness

(
DPoint2dP    pRotatedVector,
DPoint2dCP    pVector,
bool        bRightHanded
)
    {
    double  a, d2, x = pVector->x, y = pVector->y;
    pRotatedVector->x = -y;
    pRotatedVector->y =  x;
    d2 = x * x + y * y;
    if (d2 == 0.0)
        return false;
    a = 1.0 / sqrt (d2);
    if (bRightHanded)
        a = -a;
    pRotatedVector->x *= a;
    pRotatedVector->y *= a;
    return true;
    }
END_BENTLEY_NAMESPACE
