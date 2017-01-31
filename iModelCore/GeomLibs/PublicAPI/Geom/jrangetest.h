/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Geom/jrangetest.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__BENTLEY_INTERNAL_ONLY__*/
BEGIN_BENTLEY_GEOMETRY_NAMESPACE
#define MAX_FRUST_PLANES            6
#define MAX_RANGE_POINTS            8

#define DRANGE_STATE_null           0
#define DRANGE_STATE_unknown        1
#define DRANGE_STATE_primary        2
#define DRANGE_STATE_exact          3
#define DRANGE_STATE_approx         4

#define DRANGE_PRIMARY_range        0
#define DRANGE_PRIMARY_boxCorner    1
#define DRANGE_PRIMARY_plane        2
#define DRANGE_PRIMARY_hMap         3

#define RANGE_TEST_Error            0
#define RANGE_TEST_In               1
#define RANGE_TEST_Out              2
#define RANGE_TEST_Partial          3

#define RANGEPLANE_None             0x00
#define RANGEPLANE_XMin             0x01
#define RANGEPLANE_XMax             0x02
#define RANGEPLANE_YMin             0x04
#define RANGEPLANE_YMax             0x08
#define RANGEPLANE_ZMin             0x10
#define RANGEPLANE_ZMax             0x20
#define RANGEPLANE_DPoint4d         0x40

#if defined (mdl)
typedef int RangeState;
typedef int RangePrimary;
typedef int RangeTestResult;
typedef int RangePlaneMask;

#define DRange_null         DRANGE_STATE_null,
#define DRange_unknown      DRANGE_STATE_unknown
#define DRange_primary      DRANGE_STATE_primary
#define DRange_exact        DRANGE_STATE_exact
#define DRange_approx       DRANGE_STATE_approx

#define DRange_range        DRANGE_PRIMARY_range
#define DRange_boxCorner    DRANGE_PRIMARY_boxCorner
#define DRange_plane        DRANGE_PRIMARY_plane
#define DRange_hMap         DRANGE_PRIMARY_hMap

#define Range_Error         RANGE_TEST_Error
#define Range_In            RANGE_TEST_In
#define Range_Out           RANGE_TEST_Out
#define Range_Partial       RANGE_TEST_Partial

#define RangePlane_None     RANGEPLANE_None
#define RangePlane_XMin     RANGEPLANE_XMin
#define RangePlane_XMax     RANGEPLANE_XMax
#define RangePlane_YMin     RANGEPLANE_YMin
#define RangePlane_YMax     RANGEPLANE_YMax
#define RangePlane_ZMin     RANGEPLANE_ZMin
#define RangePlane_ZMax     RANGEPLANE_ZMax
#define RangePlane_DPoint4d RANGEPLANE_DPoint4d

#elif defined (__jmdl)

#else

typedef enum RangeState
    {
    DRange_null         = DRANGE_STATE_null,
    DRange_unknown      = DRANGE_STATE_unknown,
    DRange_primary      = DRANGE_STATE_primary,
    DRange_exact        = DRANGE_STATE_exact,
    DRange_approx       = DRANGE_STATE_approx
    } RangeState;

typedef enum RangePrimary
    {
    DRange_range        = DRANGE_PRIMARY_range,
    DRange_boxCorner    = DRANGE_PRIMARY_boxCorner,
    DRange_plane        = DRANGE_PRIMARY_plane,
    DRange_hMap         = DRANGE_PRIMARY_hMap
    }RangePrimary;

typedef enum RangeTestResult
    {
    Range_Error         = RANGE_TEST_Error,
    Range_In            = RANGE_TEST_In,
    Range_Out           = RANGE_TEST_Out,
    Range_Partial       = RANGE_TEST_Partial
    } RangeTestResult;

typedef enum RangePlaneMask
    {
    RangePlane_None     = RANGEPLANE_None,
    RangePlane_XMin     = RANGEPLANE_XMin,
    RangePlane_XMax     = RANGEPLANE_XMax,
    RangePlane_YMin     = RANGEPLANE_YMin,
    RangePlane_YMax     = RANGEPLANE_YMax,
    RangePlane_ZMin     = RANGEPLANE_ZMin,
    RangePlane_ZMax     = RANGEPLANE_ZMax,
    RangePlane_DPoint4d = RANGEPLANE_DPoint4d     /* Plane defined by DPoint4d. */
    } RangePlaneMask;

#endif


#define RANGENODETEST_FIELDS(__OType__)         \
    int             groupNumber[3];             \
    DRange3d        range;                      \
    __OType__       oObj;                       \
    /* -------------------------------------- */


#define RANGETEST_FIELDS                        \
    int         emptyRange;                     \
    int         rangeState;                     \
    DRange3d    range;                          \
    int         boxCornerState;                 \
    DPoint3d    cornerPoints[MAX_RANGE_POINTS]; \
    int         planeState;                     \
    DPoint4d    plane[MAX_FRUST_PLANES];        \
    int         hMapState;                      \
    DMap4d      map;                            \
    /* -------------------------------------- */


#if !defined (__jmdl)

typedef struct
    {
    RANGENODETEST_FIELDS(void*)
    } RangeNodeTest;

typedef struct
    {
    RANGETEST_FIELDS
    } RangeTest;

#endif


END_BENTLEY_GEOMETRY_NAMESPACE
