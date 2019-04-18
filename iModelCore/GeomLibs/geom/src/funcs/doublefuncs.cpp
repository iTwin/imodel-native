/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include        <stdlib.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*---------------------------------------------------------------------------------**//**
*
* @param pDest      <=> array of doubles
* @param pNumOut    <=> number of doubles in destination
* @param maxOut     => maximum allowed in destination
* @param pSource    => source array
* @param numSource  => number of doubles in source
* @return false if less than requested copies were made.
* @bsimethod                                                    EarlinLutz      01/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool        bsiDoubleArray_appendArray

(
double *pDest,
int     *pNumOut,
int     maxOut,
const   double  *pSource,
int     numSource
)
    {
    int i, k;
    for (i = 0, k = *pNumOut; i < numSource && k < maxOut; i++, k++)
        {
        pDest[k] = pSource[i];
        }
    *pNumOut = k;
    return i == numSource;
    }

/*---------------------------------------------------------------------------------**//**
* Reverse an array in place.
* @param pArray     <=> array of doubles
* @param n          => number of doubles
* @bsimethod                                                    EarlinLutz      01/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDoubleArray_reverseInPlace
(
double *pArray,
int     n
)
    {
    int i, k;
    for (i = 0, k = n - 1; i < k; i++, k--)
        {
        double a = pArray[i];
        pArray[i] = pArray[k];
        pArray[k] = a;
        }
    }




/*---------------------------------------------------------------------------------**//**
*
* @param pDest      <=> array of doubles
* @param pNumOut    <=> number of doubles in destination
* @param maxOut     => maximum allowed in destination
* @param pSource    => source array
* @param numSource  => number of doubles in source
* @param cutoff     => threshold for test.
* @return false if less than requested copies were made.
* @bsimethod                                                    EarlinLutz      01/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool        bsiDoubleArray_appendArrayGE

(
double *pDest,
int     *pNumOut,
int     maxOut,
const   double  *pSource,
int     numSource,
double  cutoff
)
    {
    int i, k;
    for (i = 0, k = *pNumOut; i < numSource && k < maxOut; i++, k++)
        {
        if (pSource[i] >= cutoff)
            pDest[k] = pSource[i];
        }
    *pNumOut = k;
    return i == numSource;
    }

/*---------------------------------------------------------------------------------**//**
*
* @param pDest      <=> array of doubles
* @param pNumOut    <=> number of doubles in destination
* @param maxOut     => maximum allowed in destination
* @param pSource    => source array
* @param numSource  => number of doubles in source
* @param cutoff     => threshold for test.
* @return false if less than requested copies were made.
* @bsimethod                                                    EarlinLutz      01/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool        bsiDoubleArray_appendArrayLE

(
double *pDest,
int     *pNumOut,
int     maxOut,
const   double  *pSource,
int     numSource,
double  cutoff
)
    {
    int i, k;
    for (i = 0, k = *pNumOut; i < numSource && k < maxOut; i++, k++)
        {
        if (pSource[i] <= cutoff)
            pDest[k] = pSource[i];
        }
    *pNumOut = k;
    return i == numSource;
    }

/*---------------------------------------------------------------------------------**//**
*
* @param pDest      <=> array of doubles
* @param pNumOut    <=> number of doubles in destination
* @param maxOut     => maximum allowed in destination
* @param a          => value to append
* @return false if array overflow.
* @bsimethod                                                    EarlinLutz      01/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool        bsiDoubleArray_append

(
double *pDest,
int     *pNumOut,
int     maxOut,
double  a
)
    {
    int k = *pNumOut;
    if (k < maxOut)
        {
        pDest[k++] = a;
        *pNumOut = k;
        return true;
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
*
* Append midpoints of intervals in the source array to the dest array.
* @param pDest      <=> array of doubles
* @param pNumOut    <=> number of doubles in destination
* @param maxOut     => maximum allowed in destination
* @param pSource    => source array
* @param numSource  => number of doubles in source
* @param cutoff     => threshold for test.
* @return false if less than requested copies were made.
* @bsimethod                                                    EarlinLutz      01/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool        bsiDoubleArray_appendMidPoints

(
double *pDest,
int     *pNumOut,
int     maxOut,
const   double  *pSource,
int     numSource
)
    {
    int i, k;
    for (i = 1, k = *pNumOut; i < numSource && k < maxOut; i++, k++)
        {
        pDest[k] = 0.5 * (pSource[i-1] + pSource[i]);
        }
    *pNumOut = k;
    return i == numSource;
    }

/*---------------------------------------------------------------------------------**//**
* @param pArray      => array of angles
* @param num         => number of angles
* @param angle      => angle to test
* @return index of first matching angle (according to sameAngle test).  -1 if no match.
* @bsimethod                                                    EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int     bsiDoubleArray_angleIndexInArray

(
const   double *pArray,
int     num,
double  theta
)
    {
    int k;
    for (k = 0; k < num; k++)
        if (bsiTrig_equalAngles (theta, pArray[k]))
            return k;
    return -1;
    }

/*---------------------------------------------------------------------------------**//**
* @param pArray      => array of doubles
* @param num         => number of doubles
* @return max absolute value in array.
* @bsimethod                                                    EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double     bsiDoubleArray_maxAbs

(
const   double *pArray,
int     num
)
    {
    int k;
    double aMax = 0.0;
    double a;
    for (k = 0; k < num; k++)
        {
        a = fabs (pArray[k]);
        if (a > aMax)
            aMax = a;
        }
    return aMax;
    }

/*---------------------------------------------------------------------------------**//**
* @param pMin       <= min value in array
* @param pMax       <= max value in array
* @param pArray      => array of doubles
* @param num         => number of doubles
* @return false if the array is empty.
* @bsimethod                                                    EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool        bsiDoubleArray_minMax

(
double  *pMin,
double  *pMax,
const   double *pArray,
int     num
)
    {
    if (num <= 0)
        {
        *pMin = *pMax = 0.0;
        return false;
        }
    else
        {
        int k;
        double aMin, aMax;
        double a;
        aMax = aMin = pArray[0];
        for (k = 1; k < num; k++)
            {
            a = pArray[k];
            if (a > aMax)
                aMax = a;
            if (a < aMin)
                aMin = a;
            }
        *pMin = aMin;
        *pMax = aMax;
        return true;
        }
    }


/*---------------------------------------------------------------------------------**//**
* @param pArray <= array of doubles.
* @param n      => number of points.
* @param a0     => first value.
* @param a1     => last value.
* @bsimethod                                                    EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void    bsiDoubleArray_uniformGrid

(
double *pArray,
int     n,
double a0,
double a1
)
    {
    if (n == 1)
        {
        pArray[0] = a0;
        }
    else if (n > 1)
        {
        double da = (a1 - a0) / (n - 1);
        int nm1 = n - 1;
        int k;
        pArray[0] = a0;
        for (k = 1; k < nm1; k++)
            {
            pArray[k] = a0 + k * da;
            }
        pArray[n - 1] = a1;
        }
    }
END_BENTLEY_GEOMETRY_NAMESPACE
