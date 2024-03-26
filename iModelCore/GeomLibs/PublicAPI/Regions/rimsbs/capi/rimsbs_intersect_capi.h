/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
/* DO NOT EDIT!  THIS FILE IS GENERATED. */

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*----------------------------------------------------------------------+
|                                                                       |
| name          debug_displayCurve                                      |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void debug_displayCurve
(
MSBsplineCurve      *curveP,
int                 displayMode
);

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRIMSBS_segmentEllipseIntersection                       |
|                                                                       |
|                                                                       |
| Intersect xy parts of an ellipse and a line segment.   Announce       |
| the intersection (or proximity) to an intersection list.              |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool    jmdlRIMSBS_segmentEllipseIntersection
(
RIMSBS_Context      *pContext,              /* IN      application context */
RG_Header           *pRG,                   /* IN      region context */
RG_IntersectionList *pRGIL,                 /* IN OUT  list of intersection parameters */
RG_EdgeData         *pEdgeData0,            /* IN      first edge data.  Assumed linear */
RG_EdgeData         *pEdgeData1,            /* IN      second edge data. */
DEllipse3d          *pEllipse               /* IN      ellipse geometry from second curve. */
);

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRIMSBS_ellipseEllipseIntersection                       |
|                                                                       |
|                                                                       |
| Intersect xy parts of an ellipse and a line segment.   Announce       |
| the intersection (or proximity) to an intersection list.              |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool    jmdlRIMSBS_ellipseEllipseIntersection
(
RIMSBS_Context      *pContext,              /* IN      application context */
RG_Header           *pRG,                   /* IN      region context */
RG_IntersectionList *pRGIL,                 /* IN OUT  list of intersection parameters */
RG_EdgeData         *pEdgeData0,            /* IN      first edge data.  Assumed linear */
DEllipse3d          *pEllipse0,             /* IN      ellipse geometry from first curve. */
RG_EdgeData         *pEdgeData1,            /* IN      second edge data. */
DEllipse3d          *pEllipse1              /* IN      ellipse geometry from second curve. */
);

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRIMSBS_segmentMSBSplineCurveIntersection                |
|                                                                       |
|                                                                       |
| Intersect xy parts of ancurve and a line segment.   Announce          |
| the intersection (or proximity) to an intersection list.              |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool    jmdlRIMSBS_segmentMSBSplineCurveIntersection
(
RIMSBS_Context      *pContext,              /* IN      application context */
RG_Header           *pRG,                   /* IN      region context */
RG_IntersectionList *pRGIL,                 /* IN OUT  list of intersection parameters */
RG_EdgeData         *pEdgeData0,            /* IN      first edge data.  Assumed linear */
RG_EdgeData         *pEdgeData1,            /* IN      second edge data. */
MSBsplineCurve      *pCurve                 /* IN      curve data from second edge */
);

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRIMSBS_MSBsplineCurveMSBsplineCurveIntersection         |
|                                                                       |
|                                                                       |
| Intersect xy parts of two curves.   Announce                          |
| the intersection (or proximity) to an intersection list.              |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool    jmdlRIMSBS_MSBsplineCurveMSBsplineCurveIntersection
(
RIMSBS_Context      *pContext,              /* IN      application context */
RG_Header           *pRG,                   /* IN      region context */
RG_IntersectionList *pRGIL,                 /* IN OUT  list of intersection parameters */
RG_EdgeData         *pEdgeData0,            /* IN      first edge data.  Assumed linear */
MSBsplineCurve      *pCurve0,               /* IN      curve data from first edge */
RG_EdgeData         *pEdgeData1,            /* IN      second edge data. */
MSBsplineCurve      *pCurve1                /* IN      curve data from second edge */
);

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRIMSBS_curveCurveIntersection                           |
|                                                                       |
|                                                                       |
| Intersect xy parts of two curves.   Announce                          |
| the intersection (or proximity) to an intersection list.              |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool    jmdlRIMSBS_MSBsplineCurveDEllipse3dIntersection
(
RIMSBS_Context      *pContext,              /* IN      application context */
RG_Header           *pRG,                   /* IN      region context */
RG_IntersectionList *pRGIL,                 /* IN OUT  list of intersection parameters */
RG_EdgeData         *pEdgeData0,            /* IN      first edge data.  Assumed linear */
MSBsplineCurve      *pCurve0,               /* IN      curve data from first edge */
RG_EdgeData         *pEdgeData1,            /* IN      second edge data. */
DEllipse3d          *pEllipse1,             /* IN      ellipse */
MSBsplineCurve      *pCurve1                /* IN      optional curve rep of ellipse. */

);

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRIMSBS_checkClosedEdge                                  |
|                                                                       |
|                                                                       |
| Compare endpoints to each other.  No inspection of edge body.         |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool    jmdlRIMSBS_checkClosedEdge
(
RIMSBS_Context      *pContext,              /* IN      application context */
RG_Header           *pRG,                   /* IN      region context */
RG_IntersectionList *pRGIL,                 /* IN OUT  list of intersection parameters */
RG_EdgeData         *pEdgeData0             /* IN      Edge data. */
);

/*----------------------------------------------------------------------+
|                                                                       |
| Name:     jmdlRIMSBS_MSBsplineCurveMSBsplineCurveIntersection         |
|                                                                       |
|                                                                       |
| Intersect xy parts of two curves.   Announce                          |
| the intersection (or proximity) to an intersection list.              |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool    jmdlRIMSBS_MSBsplineCurveSelfIntersection
(
RIMSBS_Context      *pContext,              /* IN      application context */
RG_Header           *pRG,                   /* IN      region context */
RG_IntersectionList *pRGIL,                 /* IN OUT  list of intersection parameters */
RG_EdgeData         *pEdgeData0,            /* IN      first edge data.  Assumed spline */
MSBsplineCurve      *pCurve0                /* IN      curve data from first edge */
);

END_BENTLEY_GEOMETRY_NAMESPACE

