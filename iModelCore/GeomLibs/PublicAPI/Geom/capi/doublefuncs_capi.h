/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
/* DO NOT EDIT!  THIS FILE IS GENERATED. */

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

//!
//!
//! @param pDest      IN OUT  array of doubles
//! @param pNumOut    IN OUT  number of doubles in destination
//! @param maxOut     IN      maximum allowed in destination
//! @param pSource    IN      source array
//! @param numSource  IN      number of doubles in source
//! @return false if less than requested copies were made.
//!
Public GEOMDLLIMPEXP bool        bsiDoubleArray_appendArray
(
double *pDest,
int     *pNumOut,
int     maxOut,
const   double  *pSource,
int     numSource
);

//!
//! Reverse an array in place.
//! @param pArray     IN OUT  array of doubles
//! @param n          IN      number of doubles
//!
Public GEOMDLLIMPEXP void bsiDoubleArray_reverseInPlace
(
double *pArray,
int     n
);

//!
//!
//! @param pDest      IN OUT  array of doubles
//! @param num    IN      number of doubles
//!
Public GEOMDLLIMPEXP void    bsiDoubleArray_sort
(
double *doubles,        /* IN OUT  array of Doubles to be sorted (in place) */
int    numDoubles,      /* IN      number of Doubles to sort */
int    ascend           /* IN      true for ascending order */
);

//!
//!
//! @param pDest      IN OUT  array of doubles
//! @param pNumOut    IN OUT  number of doubles in destination
//! @param maxOut     IN      maximum allowed in destination
//! @param pSource    IN      source array
//! @param numSource  IN      number of doubles in source
//! @param cutoff     IN      threshold for test.
//! @return false if less than requested copies were made.
//!
Public GEOMDLLIMPEXP bool        bsiDoubleArray_appendArrayGE
(
double *pDest,
int     *pNumOut,
int     maxOut,
const   double  *pSource,
int     numSource,
double  cutoff
);

//!
//!
//! @param pDest      IN OUT  array of doubles
//! @param pNumOut    IN OUT  number of doubles in destination
//! @param maxOut     IN      maximum allowed in destination
//! @param pSource    IN      source array
//! @param numSource  IN      number of doubles in source
//! @param cutoff     IN      threshold for test.
//! @return false if less than requested copies were made.
//!
Public GEOMDLLIMPEXP bool        bsiDoubleArray_appendArrayLE
(
double *pDest,
int     *pNumOut,
int     maxOut,
const   double  *pSource,
int     numSource,
double  cutoff
);

//!
//!
//! @param pDest      IN OUT  array of doubles
//! @param pNumOut    IN OUT  number of doubles in destination
//! @param maxOut     IN      maximum allowed in destination
//! @param a          IN      value to append
//! @return false if array overflow.
//!
Public GEOMDLLIMPEXP bool        bsiDoubleArray_append
(
double *pDest,
int     *pNumOut,
int     maxOut,
double  a
);

//!
//!
//! Append midpoints of intervals in the source array to the dest array.
//! @param pDest      IN OUT  array of doubles
//! @param pNumOut    IN OUT  number of doubles in destination
//! @param maxOut     IN      maximum allowed in destination
//! @param pSource    IN      source array
//! @param numSource  IN      number of doubles in source
//! @param cutoff     IN      threshold for test.
//! @return false if less than requested copies were made.
//!
Public GEOMDLLIMPEXP bool        bsiDoubleArray_appendMidPoints
(
double *pDest,
int     *pNumOut,
int     maxOut,
const   double  *pSource,
int     numSource
);

//!
//! @param pArray      IN      array of angles
//! @param num         IN      number of angles
//! @param angle      IN      angle to test
//! @return index of first matching angle (according to sameAngle test).  -1 if no match.
//!
Public GEOMDLLIMPEXP int     bsiDoubleArray_angleIndexInArray
(
const   double *pArray,
int     num,
double  theta
);

//!
//! @param pArray      IN      array of doubles
//! @param num         IN      number of doubles
//! @return max absolute value in array.
//!
Public GEOMDLLIMPEXP double     bsiDoubleArray_maxAbs
(
const   double *pArray,
int     num
);

//!
//! @param pMin       OUT     min value in array
//! @param pMax       OUT     max value in array
//! @param pArray      IN      array of doubles
//! @param num         IN      number of doubles
//! @return false if the array is empty.
//!
Public GEOMDLLIMPEXP bool        bsiDoubleArray_minMax
(
double  *pMin,
double  *pMax,
const   double *pArray,
int     num
);

//!
//! @param pArray OUT     array of doubles.
//! @param n      IN      number of points.
//! @param a0     IN      first value.
//! @param a1     IN      last value.
//!
Public GEOMDLLIMPEXP void    bsiDoubleArray_uniformGrid
(
double *pArray,
int     n,
double a0,
double a1
);

END_BENTLEY_GEOMETRY_NAMESPACE

