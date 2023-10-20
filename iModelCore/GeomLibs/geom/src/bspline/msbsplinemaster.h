/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
// WE CANNOT EXPORT bsp AND mdlBspline names from here -- wrong memory coupling.
#define GEOMDLL_BSPLINEIMPEXP
#pragma once

BEGIN_BENTLEY_GEOMETRY_NAMESPACE
#define dlmSystem_mdlFree(_p) BSIBaseGeom::Free (_p)
#define dlmSystem_mdlMalloc(_p) BSIBaseGeom::Malloc (_p)
#define dlmSystem_mdlCalloc(_p,_q) BSIBaseGeom::Calloc (_p,_q)
#define dlmSystem_mdlRealloc(_p,_q) BSIBaseGeom::Realloc (_p,_q)

#define dlmSystem_mdlFreeWithDescr(_p,_d) BSIBaseGeom::Free (_p)
#define dlmSystem_mdlMallocWithDescr(_p,_d) BSIBaseGeom::Malloc (_p)
#define dlmSystem_mdlCallocWithDescr(_p,_q,_d) BSIBaseGeom::Calloc (_p,_q)
#define dlmSystem_mdlReallocWithDescr(_p,_q,_d) BSIBaseGeom::Realloc (_p,_q)

#define msbspline_free(_p) BSIBaseGeom::Free (_p)
#define msbspline_malloc(_p,_d) BSIBaseGeom::Malloc (_p)
#define msbspline_calloc(_p,_q,_d) BSIBaseGeom::Calloc (_p,_q)
#define msbspline_realloc(_p,_q) BSIBaseGeom::Realloc (_p,_q)
#define mdlRMatrix_fromColumnVectors bsiRotMatrix_initFromColumnVectors
#define mdlRMatrix_fromTMatrix bsiRotMatrix_initFromTransform

#define mdlRMatrix_multiplyPoint(_point,_transform) bsiRotMatrix_multiplyDPoint3d (_transform, _point);


typedef int (*PFBsplineStroke)
(
void    *arg1P,
void    *arg2P,
void    *arg3P,
void    *arg4P,
void    *arg5P,
void    *arg6P,
void    *arg7P,
void    *arg8P
);

typedef int (*PBBCurveVoidPInt)(MSBsplineCurve*, void *, int);

int mdlCnv_roundDoubleToLong (double);


#define polyutil_isPolygonConvex(points,count,dimension,closed) bsiGeom_testXYPolygonConvex((DPoint3dCP)points,count)

#define RMAXI4                  2147483647.0
#define RMINI4                  (-2147483648.0)

#define fc_rmaxi4               RMAXI4
#define fc_rmini4               RMINI4

#define fc_tinyVal                      1.0e-14
#define fc_zero 0.0
#define fc_1em15 1.0e-15
#define fc_epsilon 0.00001
#define fc_p001 0.001
#define fc_p01     0.01
#define fc_3       3.0
#define fc_4       4.0
#define fc_5       5.0
#define fc_p0001   0.0001
#define fc_hugeVal 1e37
#define fc_nearZero 1.0e-14
#define fc_100 100
#define NULLFUNC 0
#define MAX_VERTICES (5000)
// begin bspline.h
#define MAX_BSORDER         MAX_ORDER
#define MAX_BSPOLES         MAX_VERTICES
#define MAX_BSKNOTS         MAX_VERTICES
#define MAX_BSBOUNDS        MAX_VERTICES
#define MAX_BSPANS          30
#define MAX_INSERT          40
#define MAX_ARCPOLES        7
#define MAX_ARCKNOTS        6
#define COSINE_TOLERANCE    .995

#define MAX_CLIPBATCH 200

#define           MDLERR_INSFINFO                     (-103)
#define           MDLERR_BADELEMENT                   (-105)
#define           MDLERR_BADARG                       (-126)

#define INTERSECT_NONE                      0x00
#define INTERSECT_ORIGIN                    0x01
#define INTERSECT_SPAN                      0x01 << 1
#define INTERSECT_END                       0x01 << 2

#define BSPL_CONVERGE_TOL    1.0
#define INTERPOLATION_TANGENT_SCALE 0.5
#define SMALLEST_ALLOWED_REFERENCE_SIZE 1.0
#define RELATIVE_RESOLUTION 1.0e-8

/* for Newton-Raphson iteration */
#define REV_LIMIT               10
#define STATUS_CONVERGED         0
#define STATUS_NONCONVERGED      1
#define STATUS_DIVERGING         2
#define STATUS_BADNORMAL         3

/* for append routines */
#define CONTINUITY_NONE         0
#define CONTINUITY_LEFT         1
#define CONTINUITY_MID          2
#define CONTINUITY_RIGHT        3

/* for SSI calculations */
#define CODE_UV0                            1
#define CODE_UV1                            2
#define CODE_XYZ                            3

/* quadratic equation solver */
#define SOLUTION_NONE       0
#define SOLUTION_SAME       1
#define SOLUTION_TWO        2
#define SOLUTION_ALL        3

#define MAX_CLIPBATCH       200 // MAX_VERTICES    /* 121 */
#define        MAX_BSPBATCH        MAX_CLIPBATCH

/* Minimum relative distance separating unique exterior knots in B-splines (see jmdl_bezier.h) */
#define RELATIVE_BSPLINE_EXT_KNOT_TOLERANCE 1E-7



#include "bspLocal.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void curveMinWeightAndMaxProjectedMagnitude
(
double              &wMin,
double              &pMax,
MSBsplineCurveCP  pCurve
);
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void surfaceMinWeightAndMaxMagnitude
(
double              &wMin,
double              &pMax,
MSBsplineSurfaceCP  pSurf
);
END_BENTLEY_GEOMETRY_NAMESPACE

#include "sortutil.h"
