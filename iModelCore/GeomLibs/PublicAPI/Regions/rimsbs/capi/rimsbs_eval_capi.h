/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
/* DO NOT EDIT!  THIS FILE IS GENERATED. */

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRIMSBS_initPartialEllipse                               |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void     jmdlRIMSBS_initPartialEllipse
(
      DEllipse3d    *pInstance,
const DEllipse3d    *pParentEllipse,
      double        s0,
      double        s1
);

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRIMSBS_getMappedCurveRange
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool    jmdlRIMSBS_getMappedCurveRange
(
RIMSBS_Context  *pContext,
DRange3d        *pRange,
RIMSBS_CurveId  curveId,
double          s0,
double          s1
);

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRIMSBS_getCurveRange                                    |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool    jmdlRIMSBS_getCurveRange
(
RIMSBS_Context  *pContext,
DRange3d        *pRange,
RIMSBS_CurveId  curveId
);

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRIMSBS_applyScalePowers                                 |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void    jmdlRIMSBS_applyScalePowers
(
DPoint3d        *pXYZ,
int             numDerivatives,
double          scale
);

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRIMSBS_scaleDPoint3dArrayInPlace                        |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void    jmdlRIMSBS_scaleDPoint3dArrayInPlace
(
DPoint3d        *pXYZ,
int             n,
double          scale
);

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRIMSBS_evaluateMappedDerivatives                        |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool        jmdlRIMSBS_evaluateMappedDerivatives
(
RIMSBS_Context  *pContext,
DPoint3d        *pXYZ,
int             numDerivatives,
double          param,
RG_CurveId      curveId,
double          s0,
double          s1
);

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRIMSBS_evaluateDerivatives                              |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool        jmdlRIMSBS_evaluateDerivatives
(
RIMSBS_Context  *pContext,
DPoint3d        *pXYZ,
int             numDerivatives,
double          param,
RG_CurveId      curveId
);

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRIMSBS_evaluateMapped                                   |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool        jmdlRIMSBS_evaluateMapped
(
RIMSBS_Context  *pContext,
DPoint3d        *pXYZ,
DPoint3d        *pTangent,
double          *pParam,
int             nParam,
RG_CurveId      curveId,
double          s0,
double          s1
);

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRIMSBS_evaluate                                         |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool        jmdlRIMSBS_evaluate
(
RIMSBS_Context  *pContext,
DPoint3d        *pXYZ,
DPoint3d        *pTangent,
double          *pParam,
int             nParam,
RG_CurveId      curveId
);

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRIMSBS_getCurveInterval                                 |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool        jmdlRIMSBS_getCurveInterval
(
RIMSBS_Context  *pContext,
RG_CurveId      *pParentCurveId,
double          *pStartFraction,
double          *pEndFraction,
RG_CurveId      curveId,
bool            reversed
);

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRIMSBS_getDEllipse3d                                    |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool        jmdlRIMSBS_getDEllipse3d
(
RIMSBS_Context  *pContext,
DEllipse3d      *pEllipse,
RG_CurveId      curveId,
bool            reversed
);

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRIMSBS_getMappedMSBsplineCurve                          |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool        jmdlRIMSBS_getMappedMSBsplineCurve
(
RIMSBS_Context  *pContext,
MSBsplineCurve  *pCurve,
RG_CurveId      curveId,
double          s0,
double          s1
);

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRIMSBS_getMSBsplineCurve                                |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool        jmdlRIMSBS_getMSBsplineCurve
(
RIMSBS_Context  *pContext,
MSBsplineCurve  *pCurve,
RG_CurveId      curveId,
bool            reversed
);

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRIMSBS_sweptPopertiesMapped                             |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool        jmdlRIMSBS_sweptPopertiesMapped
(
RIMSBS_Context  *pContext,
double          *pArea,
double          *pAngle,
const DPoint3d  *pPoint,
int             curveId,
double          s0,
double          s1
);

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRIMSBS_sweptProperties                                  |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool        jmdlRIMSBS_sweptProperties
(
RIMSBS_Context  *pContext,
RG_EdgeData     *pEdgeData,
double          *pArea,
double          *pAngle,
const DPoint3d  *pPoint
);

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRIMSBS_curveCurveIntersection                           |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool    jmdlRIMSBS_curveCurveIntersection
(
RIMSBS_Context          *pContext,          /* IN      general context */
RG_Header               *pRG,               /* IN      receives declarations of intersections */
RG_IntersectionList     *pIntersections,    /* IN OUT  list of intersection parameters */
RG_EdgeData             *pEdgeData0,        /* IN      segment edge data */
RG_EdgeData             *pEdgeData1         /* IN      curve edge data */
);

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRIMSBS_segmentCurveIntersectionMapped                   |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool    jmdlRIMSBS_segmentCurveIntersectionMapped
(
RIMSBS_Context          *pContext,          /* IN      general context */
RG_Header               *pRG,               /* IN      receives declarations of intersections */
RG_IntersectionList     *pIntersections,    /* IN OUT  list of intersection parameters */
RG_EdgeData             *pEdgeData0,        /* IN      segment edge data */
RG_EdgeData             *pEdgeData1,        /* IN      curve edge data, known to be mapped */
int                     parentCurveId,
double                  s0,
double                  s1
);

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRIMSBS_segmentCurveIntersection                         |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool    jmdlRIMSBS_segmentCurveIntersection
(
RIMSBS_Context          *pContext,          /* IN      general context */
RG_Header               *pRG,               /* IN      receives declarations of intersections */
RG_IntersectionList     *pIntersections,    /* IN OUT  list of intersection parameters */
RG_EdgeData     *pEdgeData0,                /* IN      segment edge data */
RG_EdgeData     *pEdgeData1                 /* IN      curve edge data */
);

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRIMSBS_getClosestXYPointOnMappedCurve                   |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool        jmdlRIMSBS_getClosestXYPointOnMappedCurve
(
RIMSBS_Context  *pContext,
double   *pMinParam,        /* IN      parameter at closest approach point */
double   *pMinDistSquared,  /* IN      squard distance to closest approach point */
DPoint3d *pMinPoint,        /* IN      closest approach point */
DPoint3d *pMinTangent,      /* IN      tangent vector at closest approach point */
DPoint3d *pPoint,           /* IN      space point */
RG_CurveId  curveId,        /* IN      curve identifier */
double    s0,               /* IN      start of active interval */
double    s1                /* IN      end param for active interval */
);

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRIMSBS_getClosestXYPointOnCurve                         |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool        jmdlRIMSBS_getClosestXYPointOnCurve
(
RIMSBS_Context  *pContext,
double   *pMinParam,        /* IN      parameter at closest approach point */
double   *pMinDistSquared,  /* IN      squard distance to closest approach point */
DPoint3d *pMinPoint,        /* IN      closest approach point */
DPoint3d *pMinTangent,      /* IN      tangent vector at closest approach point */
DPoint3d *pPoint,           /* IN      space point */
RG_CurveId  curveId         /* IN      curve identifier */
);

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRIMSBS_appendAllCurveSamplePoints                       |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void    jmdlRIMSBS_appendAllCurveSamplePoints
(
RIMSBS_Context  *pContext,
EmbeddedDPoint3dArray *pXYZArray
);

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRIMSBS_transformCurve                                   |
|                                                                       |
|                                                                       |
| Transform a single curve.  No action for subcurves -- only parents    |
| can be transformed!!!                                                 |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool            jmdlRIMSBS_transformCurve
(
RIMSBS_Context          *pContext,
int                     curveIndex,
const Transform         *pTransform
);

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRIMSBS_transformCurve                                   |
|                                                                       |
|                                                                       |
| Transform all curves in place.                                        |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void            jmdlRIMSBS_transformAllCurves
(
RIMSBS_Context          *pContext,
const Transform         *pTransform
);

END_BENTLEY_GEOMETRY_NAMESPACE

