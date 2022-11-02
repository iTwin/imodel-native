/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
/* DO NOT EDIT!  THIS FILE IS GENERATED. */

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* rational deCasteljau algorithm
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bspproc_refineLinearApprox
(
DPoint3d        *outPoles,             /* 0.0 to 0.5 approx */
double          *outWts,
DPoint3d        *inPoles,              /* IN OUT  weighted poles of approx */
double          *inWts,                /*  on return is 0.5 to 1.0 approx */
int             order,                 /* IN      number of points OUT     MAX_ORDER */
int             rational,
int             inIncr,                /* IN      increment btw in poles/wts */
int             outIncr                /* IN      increment btw out poles/wts */
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double   bspproc_setParameter
(
double          in,                    /* IN      parameter in segment 0.0 to 1.0 */
int             start,                 /* IN      knot offset of segment */
double          *knots                 /* IN      full knot vector */
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double   bspproc_setParameterWithOrder
(
double          in,                    /* IN      parameter in segment 0.0 to 1.0 */
int             start,                 /* IN      knot offset of segment */
double          *knots,                /* IN      full knot vector */
int             order
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bspproc_setPatchParameter
(
DPoint2d        *out,
int             code,
DPoint2d        *in
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bspproc_freePreparedCurve
(
MSBsplineCurve  *bezier,               /*  IN      Bezier curve */
int             **starts               /* IN OUT  offsets of segments */
);

/*---------------------------------------------------------------------------------**//**
* Free the surface and arrays previously allocated by bspproc_prepareSurface
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bspproc_freePreparedSurface
(
MSBsplineSurface    *bezierP,          /* IN OUT  bezier surface */
int                 **uStartsPP,       /* IN OUT  offsets of segments U */
int                 **vStartsPP        /* IN OUT  offsets of segments V */
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int     bspproc_prepareSurface
(
MSBsplineSurface    *bezier,           /* OUT     bezier surface */
int                 *uNumSegs,         /* OUT     number of segs U */
int                 *vNumSegs,         /* OUT     number of segs V */
int                 **uStarts,         /* OUT     offsets of segments U */
int                 **vStarts,         /* OUT     offsets of segments V */
MSBsplineSurface    *bspline           /* IN      Bspline surface */
);

/*---------------------------------------------------------------------------------**//**
* stopFunc : Return true if converged after setting output information else return false goFunc : Return true if this Bezier is still worth
* considering else return false sortFunc : Return 1 or 0 depending on which bezier should be processed first selectFunc : Set output
* information based on information from the 2 halves
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspproc_processBezier
(
BezierInfo      *infoP,                /* IN OUT  information desired of curve */
MSBsplineCurve  *bezier,               /* IN      Bezier curve to process */
PFBezCurve_Stop stopFunc,     /* IN      ends recursion & loads output */
PFBezCurve_Sort sortFunc,     /* IN      sorting function, or NULL */
PFBezCurve_Go   goFunc,       /* IN      optional, to check half */
PFBezCurve_Select selectFunc  /* IN      loads ouput based on halves */
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspproc_doubleProcessBezier
(
BezierInfo      *infoP,                /* IN OUT  information desired of curve */
MSBsplineCurve  *bezier0,              /* IN      Bezier curve to process */
MSBsplineCurve  *bezier1,              /* IN      Bezier curve to process */
PFBezCurveBezCurve_Stop   stopFunc,      /* IN      ends recursion & loads output */
PFBezCurveBezCurve_Sort   sortFunc,      /* IN      sorting function, or NULL */
PFBezCurveBezCurve_Go     goFunc,        /* IN      optional, to check half */
PFBezCurveBezCurve_Select selectFunc     /* IN      loads ouput based on halves */
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspproc_processBezierPatchFromStack
(
StatusInt           *processStatusP,    /* Result of processing.  Set to ERROR if
                                                this function is unable to start processing */
BezierInfo          *infoP,            /* IN OUT  information desired of patch */
MSBsplineSurface    *patch,            /* IN      Bezier patch to process */
PFBezPatch_Stop  stopFunc,  /* IN      ends recursion & loads output */
PFBezPatch_Sort  sortFunc,  /* IN      sorting function, or NULL */
PFBezPatch_Go    goFunc,    /* IN      to check half, or NULL */
PFBezPatch_Select selectFunc /* IN      loads ouput, or NULL */
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspproc_processBezierPatch
(
BezierInfo          *infoP,            /* IN OUT  information desired of patch */
MSBsplineSurface    *patch,            /* IN      Bezier patch to process */
PFBezPatch_Stop  stopFunc,  /* IN      ends recursion & loads output */
PFBezPatch_Sort  sortFunc,  /* IN      sorting function, or NULL */
PFBezPatch_Go    goFunc,    /* IN      to check half, or NULL */
PFBezPatch_Select selectFunc /* IN      loads ouput, or NULL */
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspproc_doubleProcessPreallocatedBezierPatch
(
BezierInfo          *infoP,            /* IN OUT  information desired of patch */
MSBsplineSurface    *patch0,           /* IN      Bezier patch to process */
DPoint3d            *poles0,           /* IN      pole buffer for surface 0 */
double              *weights0,         /* IN      weight buffer for surface 0 */
int                 count0,             /* IN      buffer size for surface 0 */
MSBsplineSurface    *patch1,           /* IN      Bezier patch to process */
DPoint3d            *poles1,           /* IN      pole buffer for surface 1 */
double              *weights1,         /* IN      weight buffer for surface 1 */
int                 count1,             /* IN      buffer size for surface 1 */
BezierInfo          *infoX,             /* IN      buffer of bezier info structs */
PFBezPatchBezPatch_Stop  stopFunc,  /* IN      ends recursion & loads output */
PFBezPatchBezPatch_Sort  sortFunc,  /* IN      sorting function, or NULL */
PFBezPatchBezPatch_Go    goFunc,    /* IN      to check half, or NULL */
PFBezPatchBezPatch_Select selectFunc /* IN      loads ouput, or NULL */
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspproc_doubleProcessBezierPatchFromHeap
(
BezierInfo          *infoP,            /* IN OUT  information desired of patch */
MSBsplineSurface    *patch0,           /* IN      Bezier patch to process */
MSBsplineSurface    *patch1,           /* IN      Bezier patch to process */
PFBezPatchBezPatch_Stop  stopFunc,  /* IN      ends recursion & loads output */
PFBezPatchBezPatch_Sort  sortFunc,  /* IN      sorting function, or NULL */
PFBezPatchBezPatch_Go    goFunc,    /* IN      to check half, or NULL */
PFBezPatchBezPatch_Select selectFunc /* IN      loads ouput, or NULL */
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP StatusInt    bspproc_doubleProcessBezierPatchFromStack   /* ERROR if stack not big enough */
(
StatusInt           *procStatusP,      /* OUT     status of the real processing. */
BezierInfo          *infoP,            /* IN OUT  information desired of patch */
MSBsplineSurface    *patch0,           /* IN      Bezier patch to process */
MSBsplineSurface    *patch1,           /* IN      Bezier patch to process */
PFBezPatchBezPatch_Stop  stopFunc,  /* IN      ends recursion & loads output */
PFBezPatchBezPatch_Sort  sortFunc,  /* IN      sorting function, or NULL */
PFBezPatchBezPatch_Go    goFunc,    /* IN      to check half, or NULL */
PFBezPatchBezPatch_Select selectFunc /* IN      loads ouput, or NULL */
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspproc_doubleProcessBezierPatch
(
BezierInfo          *infoP,            /* IN OUT  information desired of patch */
MSBsplineSurface    *patch0,           /* IN      Bezier patch to process */
MSBsplineSurface    *patch1,           /* IN      Bezier patch to process */
PFBezPatchBezPatch_Stop  stopFunc,  /* IN      ends recursion & loads output */
PFBezPatchBezPatch_Sort  sortFunc,  /* IN      sorting function, or NULL */
PFBezPatchBezPatch_Go    goFunc,    /* IN      to check half, or NULL */
PFBezPatchBezPatch_Select selectFunc /* IN      loads ouput, or NULL */
);

/*---------------------------------------------------------------------------------**//**
* Process a B-spline curve by dividing it into Bezier segments and recursively processing the segments. The following functions control the
* recursion. sortFunc : Order the Bezier segments for further processing goFunc : Return true if this Bezier is still worth considering else
* return false selectFunc : Assigns the output for _processBspline by selecting from the results on the segments cleanFunc : Called before
* _processBspline returns
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspproc_processBspline
(
BezierInfo      *infoP,        /* IN OUT  information desired of curve */
MSBsplineCurve  *bspline,      /* IN      Bspline curve to process */
FPBsplineSort    sortFunc,     /* IN      sorting function, or NULL */
FPBsplineGo      goFunc,       /* IN      go function, or NULL */
FPBsplineProcess processFunc,  /* IN      acts on Bezier segments */
FPBsplineSelect  selectFunc,   /* IN      loads ouput based on segments */
FPBsplineClean   cleanFunc     /* IN      called before returning */
);


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspproc_initializeBezierPatch
(
MSBsplineSurface        *patchP,            /* OUT     bezier patch */
MSBsplineSurface        *surfP              /* IN      surface (bezier prepped) */
);

/*---------------------------------------------------------------------------------**//**
* Initialize a bezier curve struct to hold a bezier of same order as a given curve.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspproc_initializeBezierSegment
(
MSBsplineCurve  *segP,           /* OUT     bezier segment of curve */
MSBsplineCurve  *curveP          /* IN      curve (bezier prepped) */
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void      bspproc_freeBezierSegment
(
MSBsplineCurve  *segP                    /* OUT     bezier segment of curve */
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void      bspproc_freeBezierPatch
(
MSBsplineSurface        *surfaceP                /* OUT     bezier patch of surface */
);

/*---------------------------------------------------------------------------------**//**
* Analogy to mdlBspline_netPoles. Copy poles and weights from a 'prepped curve' index to working arrays
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bspproc_segmentPoles
(
DPoint3d            *poles,
double              *weights,
int                 uIndex,
MSBsplineCurve   *curveP
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspproc_processBsplineSurface
(
BezierInfo          *infoP,           /* IN OUT  information desired of surf */
MSBsplineSurface    *bspline,         /* IN      Bspline surface to process */
PFBSurf_Sort       sortFunc,          /* IN      sorting function, or NULL */
PFBSurf_Go         goFunc,            /* IN      go function, or NULL */
PFBSurf_Process    processFunc,       /* IN      acts on Bezier patches */
PFBSurf_Select     selectFunc,        /* IN      loads ouput based on patches */
PFBSurf_Clean      cleanFunc          /* IN      called before returning */
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspproc_doubleProcessBsplineSurface
(
BezierInfo          *infoP,           /* IN OUT  information desired of surf */
MSBsplineSurface    *bspline0,        /* IN      Bspline surface to process */
MSBsplineSurface    *bspline1,        /* IN      Bspline surface to process */
PFBSurfBSurf_Sort       sortFunc,          /* IN      sorting function, or NULL */
PFBSurfBSurf_Go         goFunc,            /* IN      go function, or NULL */
PFBSurfBSurf_Process    processFunc,       /* IN      acts on Bezier patches */
PFBSurfBSurf_Select     selectFunc,        /* IN      loads ouput based on patches */
PFBSurfBSurf_Clean      cleanFunc          /* IN      called before returning */
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspproc_doubleProcessSurfaceByCurves
(
MSBsplineSurface    *outSurface,   /* OUT     != bspline0 && != bspline1 */
MSBsplineSurface    *bspline0,
MSBsplineSurface    *bspline1,
PFBCurveBCurveBCurveVoidInt processFunc,   /* IN      acts on row/column curves of surface */
PFBSurfBSurfBSurfVoidInt    boundaryFunc,  /* IN      acts on boundaries of input surfaces */
void                *args,
int                 direction
);

/*---------------------------------------------------------------------------------**//**
* Process a B-spline surface by operating on polygonal mesh. processFunc : receives 4 corners of polygons and normals.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspproc_processSurfaceMesh
(
MSBsplineSurface    *surfP,            /* IN      surface to process */
double              tolerance,         /* IN      stroke tolerance */
PFDPoint3dPDPoint3dPDPoint2dPVoidP processFunc,
int                 (*stopFunc)(),     /* IN      stop function */
void                *args              /* IN      arguments */
);

/*---------------------------------------------------------------------------------**//**
* Process a B-spline curve by operating on line segments
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspproc_processCurveAsLines
(
MSBsplineCurve  *curveP,                /* IN      curve to process */
double          tolerance,              /* IN      stroke tolerance */
RotMatrix       *matrix,                /* IN      of view to stroke in, or NULL */
int             (*processFunc)(),       /* IN      process function */
int             (*stopFunc)(),          /* IN      stop function */
void            *args                   /* IN      arguments */
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspproc_processCurvePolygon
(
MSBsplineCurve  *curveP,                /* IN      curve to process */
PFDPoint3dPIntVoidP processFunc,       /* IN      process function */
void            *args                   /* IN      arguments */
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspproc_processInterpolationCurvePolygon
(
MSInterpolationCurve    *curveP,                /* IN      curve to process */
PFDPoint3dPIntVoidP processFunc,       /* IN      process function */
void            *args                   /* IN      arguments */
);

/*---------------------------------------------------------------------------------**//**
* 1 +--+--+--+--+ |\ |\ |\ |\ | This routine checks the triangles of | \| \| \| \| the polygon control net. If the lower +--+--+--+--+ left
* triangle is not hit (i.e. the v |\ |\ |\ |\ | intersection's barycentric coordinates | \| \| \| \| add up to more than 1.0 for the triangle)
* +--+--+--+--+ then the upper right triangles is |\ |\ |\ |\ | checked. | \| \| \| \| 0 +--+--+--+--+ 0 u 1
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspproc_processControlNet
(
DPoint3d        *points,               /* IN      points defining net */
int             uNum,                  /* IN      number of points in u (varies fastest) */
int             vNum,                  /* IN      number of points in v */
PFDPoint3dPDPoint3dPDPoint3dPVoidPBool_IntIntIntInt goFunc,      /* IN      decides whether or not to process triangle */
PFDPoint3dPDPoint3dPDPoint3dPVoidPBool_IntIntIntInt processFunc, /* IN      preocesses triangle */
PFDPoint3dPDPoint3dPDPoint3dPVoidPBool_IntIntIntInt selectFunc,  /* IN      accumulates results */
void            *argsP                 /* IN      passed through to processFunc */
);

/*---------------------------------------------------------------------------------**//**
* Four ways to use this function: 1). numPts != 0 && data != NULL -> evaluate at given parametrs #). numPts != 0 && data == NULL -> evaluate
* at numPts evenly 2). rulesByLength == true ... spaced along arc length 3). rulesByLength == false ... spaced from 0.0 to 1.0 4). numPts == 0
* && -> stroke curve according to data[0] = chord tol chord tolerance, set numPts to number of points returned
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspcurv_evaluateCurve
(
DPoint3d        **pts,         /* OUT     evaluated points */
double          *data,         /* IN      params to evaluate at */
int             *numPts,       /* IN OUT  number evaluated points */
MSBsplineCurve  *curve         /* IN      curve structure */
);
END_BENTLEY_GEOMETRY_NAMESPACE

