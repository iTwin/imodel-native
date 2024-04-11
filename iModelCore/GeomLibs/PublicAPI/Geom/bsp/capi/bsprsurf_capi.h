/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
/* DO NOT EDIT!  THIS FILE IS GENERATED. */

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bsprsurf_combineSurfaces
(
MSBsplineSurface    *outSurface,
MSBsplineSurface    *surf1,
MSBsplineSurface    *surf2,
int                 edge,
bool                forceContinuity,
bool                reparamSurface
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bsprsurf_appendSurfaces
(
MSBsplineSurface    *outSurface,
MSBsplineSurface    *inSurface1,
MSBsplineSurface    *inSurface2,
int                 edge,
bool                forceContinuity,
bool                reparamSurface
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bsprsurf_blendSurface
(
MSBsplineSurface    *outSurface,
MSBsplineSurface    *inSurface1,
MSBsplineSurface    *inSurface2,
double              param1,
double              param2,
int                 degree,
double              mag1,
double              mag2,
int                 direction
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsprsurf_netPoles
(
DPoint3d            *poles,
double              *weights,
int                 uIndex,
int                 vIndex,
MSBsplineSurface    *surface
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bsprsurf_minDistToSurface
(
double*             distance,           /* <= distance to closest point on curve */
DPoint3dP           minPt,              /* <= closest point on curve */
DPoint2dP           uv,                 /* <= parameter of closest point */
DPoint3dCP          testPt,             /* => point to calculate dist from */
MSBsplineSurface*   surface,            /* => input surface */
double*             tolerance           /* => tolerance to use in calculation */
);



/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bsprsurf_boresiteToSurface
(
DPoint3dP           borePt,
DPoint2dP           param,
DPoint3dCP          testPt,
DPoint3dCP          direction,
MSBsplineSurface*   surface,
double*             tolerance
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bsprsurf_allBoresiteToSurface
(
DPoint3d            **minPt,
DPoint2d            **param,
int                 *numPts,
DPoint3dCP          testPt,
DVec3dCP            direction,
MSBsplineSurfaceCP  surface,
double              *tolerance
);


/*---------------------------------------------------------------------------------**//**
* This routine finds the intersections between ray and surface analyticaly. See James Kajiya's Ray Tracing Parametric Patches for ref on
* Computer Graphics, July 1982, Vol. 16. Number 3
* IMPORTANT: This routine only takes u, v orders OUT     4 surface!!!
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bsprsurf_lineXSurface
(
DPoint3d            **intPts,       /* OUT     all intersection points on surface */
DPoint2d            **param,        /* OUT     u, v parameters of intersections */
int                 *numPts,        /* OUT     number of intersections */
DSegment3d           *segmentP,          /* IN      segmentP->point[1] is the true end point */
MSBsplineSurface    *surfaceP       /* IN      surface */
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bsprsurf_blendRails
(
MSBsplineSurface    *outSurf,
MSBsplineSurface    *inSurf1,
MSBsplineSurface    *inSurf2,
MSBsplineCurve      *railCv1,
MSBsplineCurve      *railCv2,
double              *tol,
bool                continuity
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int   bsprsurf_crossSectionSurface
(
MSBsplineSurface        *surface,
MSBsplineCurve          *curves,
int                     numCurves,
int                     processCurves,
int                     roundEnds,
double                  processTol
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bsprsurf_sweepAlongTwoTraces
(
MSBsplineSurface        *surfaceP,
MSBsplineCurve          *trace0P,
MSBsplineCurve          *trace1P,
MSBsplineCurve          *sectionP,
DPoint3d                *crossTangents,
double                  gapTolerance,
double                  fitTolerance,
int                     numSweepings,
bool                    rigidSweep,
bool                    useArcLength,
bool                    checkGaps
);

END_BENTLEY_GEOMETRY_NAMESPACE

