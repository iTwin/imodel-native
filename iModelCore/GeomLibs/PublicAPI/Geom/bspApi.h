/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <Geom/GeomApi.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

typedef union bezierInfo BezierInfo;

typedef struct intLink IntLink;
typedef struct ssiTolerance
    {
    double    xyzSame;
    double    xyzTol;
    double    uvSame;
    double    uvTol;
    double    maxStep;
    } SsiTolerance;

typedef struct ssiPt
    {
    DPoint3d    xyz;
    DPoint3d    norm0;
    DPoint3d    norm1;
    DPoint2d    uv0;
    DPoint2d    uv1;
    } SsiPt;
    
struct intLink
    {
    IntLink             *next;
    IntLink             *last;
    int                 closed;
    int                 number;
    DPoint2d            *uv0;
    DPoint2d            *uv1;
    DPoint3d            *xyz;
    DPoint3d            *norm0;
    DPoint3d            *norm1;
    MSBsplineSurface    *surf0;
    MSBsplineSurface    *surf1;
    };


typedef struct boundIntersect
    {
    bool            surface0;
    double          distance;
    DPoint3d        point;
    DPoint3d        normal0;
    DPoint3d        normal1;
    } BoundIntersect;


typedef struct evaluator
    {
    MSBsplineSurface    *surf;
    int                 offset;
    double              distance;
    } Evaluator;

/*  See also function headers in the file for more documentation
    on the following routines */
typedef int (*PFBezCurve_Stop)(BezierInfo*,MSBsplineCurve*);
typedef int (*PFBezCurve_Sort)(BezierInfo*,BezierInfo*,MSBsplineCurve*,MSBsplineCurve*);
typedef int (*PFBezCurve_Go) (BezierInfo*,MSBsplineCurve*, int);
typedef int (*PFBezCurve_Select)(BezierInfo*,BezierInfo*,BezierInfo*,MSBsplineCurve*,MSBsplineCurve*);

typedef int (*PFBezCurveBezCurve_Stop)(BezierInfo*,MSBsplineCurve*,MSBsplineCurve*);
typedef int (*PFBezCurveBezCurve_Sort)(int *,BezierInfo**,MSBsplineCurve**);
typedef int (*PFBezCurveBezCurve_Go) (BezierInfo*,MSBsplineCurve*, MSBsplineCurve*,int,int);
typedef int (*PFBezCurveBezCurve_Select)(BezierInfo*,BezierInfo**,MSBsplineCurve**);

typedef int (*PFBezPatch_Stop)(BezierInfo*,MSBsplineSurface*);
typedef int (*PFBezPatch_Sort)(int*,BezierInfo**,MSBsplineSurface**);
typedef int (*PFBezPatch_Go)(BezierInfo*,MSBsplineSurface*,int);
typedef int (*PFBezPatch_Select)(BezierInfo*,BezierInfo**,MSBsplineSurface**);

typedef int (*PFBezPatchBezPatch_Stop)(BezierInfo*,MSBsplineSurface*,MSBsplineSurface*);
typedef int (*PFBezPatchBezPatch_Sort)(int*,BezierInfo**,MSBsplineSurface**,MSBsplineSurface**);
typedef int (*PFBezPatchBezPatch_Go)(BezierInfo*,MSBsplineSurface*,MSBsplineSurface*,int,int);
typedef int (*PFBezPatchBezPatch_Select)(BezierInfo*,BezierInfo**,MSBsplineSurface**,MSBsplineSurface**);

typedef int (*PFBSurf_Sort)   (int*,BezierInfo*,int*uStart,int*vStart,int uNum,int vNum, MSBsplineSurface*);
typedef int (*PFBSurf_Go)     (BezierInfo*,MSBsplineSurface*,int uSeg,int vSeg, int uStart, int vStart, int uNum, int vNum);
typedef int (*PFBSurf_Process)(BezierInfo*, MSBsplineSurface*);
typedef int (*PFBSurf_Select) (BezierInfo*, BezierInfo*, MSBsplineSurface*,int uSeg, int vSeg, int uStart, int vStart, int uNum, int vNum);
typedef void (*PFBSurf_Clean)  (BezierInfo*, int uNum, int vNum);

typedef int (*FPBsplineGo)(BezierInfo*,MSBsplineCurve*,int,int,int);
typedef int (*FPBsplineProcess)(BezierInfo*,MSBsplineCurve*);
typedef int (*FPBsplineSelect)
(
BezierInfo          *infoP,
BezierInfo          *infoSeg,
MSBsplineCurve      *bezierP,
int                 index,                     /* => index of segment */
int                 start,                     /* => knot offset of segment */
int                 numberOfSegments
);

typedef int (*FPBsplineSort)
(
int            *tags,
BezierInfo     *infoP,
int            *start,
int             numSegments,
MSBsplineCurve *curve
);

typedef int (*FPBsplineGo)(BezierInfo*,MSBsplineCurve*,int,int,int);
typedef int (*FPBsplineProcess)(BezierInfo*,MSBsplineCurve*);
typedef int (*FPBsplineSelect)
(
BezierInfo          *infoP,
BezierInfo          *infoSeg,
MSBsplineCurve      *bezierP,
int                 index,                     /* => index of segment */
int                 start,                     /* => knot offset of segment */
int                 numberOfSegments
);
typedef void (*FPBsplineClean)(BezierInfo*,int numSegments);

typedef void (*FPBsplineClean)(BezierInfo*,int numSegments);

typedef int (*PFBCurveBCurve_Sort)    (int*,BezierInfo*, int*, int*, int, int, MSBsplineCurve*,MSBsplineCurve*);
typedef int (*PFBCurveBCurve_Go)      (BezierInfo*, MSBsplineCurve*,MSBsplineCurve*,int,int,int,int,int,int);
typedef int (*PFBCurveBCurve_Process) (BezierInfo*,MSBsplineCurve*,MSBsplineCurve*);
typedef int (*PFBCurveBCurve_Select)  (BezierInfo*,BezierInfo*,MSBsplineCurve*,MSBsplineCurve*,int,int,int,int,int,int);
typedef void (*PFBCurveBCurve_Clean)   (BezierInfo*,int,int);



typedef int (*PFBSurfBSurf_Sort)   (int*,BezierInfo*,
                                    int*uStart0,int*vStart0,int *uStart1,int *vStart1,
                                    int uNum0,int vNum0, int uNum1, int vNum1,
                                    MSBsplineSurface*,MSBsplineSurface*);
typedef int (*PFBSurfBSurf_Go)     (BezierInfo*,MSBsplineSurface*,MSBsplineSurface*,
                                    int uSeg0, int vSeg0, int uSeg1, int vSeg1,
                                    int uStart0,int vStart0,int uStart1,int vStart1,
                                    int uNum0,int vNum0, int uNum1, int vNum1);
typedef int (*PFBSurfBSurf_Process)(BezierInfo*, MSBsplineSurface*, MSBsplineSurface*);
typedef int (*PFBSurfBSurf_Select) (BezierInfo*, BezierInfo*, MSBsplineSurface*,MSBsplineSurface*,
                                    int uSeg0, int vSeg0, int uSeg1, int vSeg1,
                                    int uStart0,int vStart0,int uStart1,int vStart1,
                                    int uNum0,int vNum0, int uNum1, int vNum1);
typedef void (*PFBSurfBSurf_Clean)  (BezierInfo*, int uNum0, int vNum0, int uNum1, int vNum1);

typedef int (*PFBCurveBCurveBCurveVoidInt)(MSBsplineCurve*,MSBsplineCurve*,MSBsplineCurve*,void*,int);
typedef int (*PFBSurfBSurfBSurfVoidInt)(MSBsplineSurface*,MSBsplineSurface*,MSBsplineSurface*,void*,int);

typedef int (*PFDPoint3dPIntVoidP)(DPoint3dP,int,void*);
typedef int (*PFDPoint3dPDPoint3dPDPoint3dPVoidPBool_IntIntIntInt)(
                        DPoint3d*,DPoint3d*,DPoint3d*,void*,bool,int,int,int,int);

typedef int (*PFDPoint3dPDPoint3dPDPoint2dPVoidP)(DPoint3dP xyz,DPoint3dP normal,DPoint2dP uv, void*);

typedef int (*PFBCurveVoidPInt)(MSBsplineCurve*,void*,int);



typedef int ( *ProcessFuncSurfaceWireFrame)
(
void            *pArg,
MSBsplineCurve  *segCurve,
double          u0,
double          u1,
double          v0,
double          v1
);

typedef int (*ProcessFuncSurfaceControlPolygon)
(
void                        *pArg,
DPoint3d                    *pPoints,
int                         nPoints,
const MSBsplineSurface*     pSurface,
double                      u0,
double                      u1,
double                      v0,
double                      v1
);


typedef void MSBsplineCurve_AnnounceParameter
(
MSBsplineCurve *pCurve,
double          startParam,
double          endParam,
double          parameter,
void            *pUserData1,
void            *pUserData2
);

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

typedef bool    (*PlankingSurfaceEvaluator)
    (
    void *pUserData,
    DPoint2d *pUV,
    DPoint3d *pXYZ,
    DPoint3d *pdXYZdU,
    DPoint3d *pdXYZdV,
    DPoint3d *pd2XYZdUdU,
    DPoint3d *pd2XYZdVdV,
    DPoint3d *pd2XYZdUdV
    );

#ifdef COMPILE_MSBSPLINE_BOOLEANDATA
#ifndef MSBSPLINE_BOOLEANDATA
    #define MSBSPLINE_BOOLEANDATA
        typedef struct booleandata             /* specific to each surface in SurfaceChain */
            {
            int32_t         numBounds;         /* number of new trim boundaries for this surface */
            BsurfBoundary*  bounds;            /* trim boundaries for this surface created by SSI */
            BoundBox        box;               /* bounding box of this surface */
            void const*     edP;               /* generating element descriptor for this surface */
            void*           trimEdP;           /* trimmed element descriptor for this surface */
            } BooleanData;
#endif
#endif
enum
{
    STROKETOL_ChoordHeight    = 0x0001,
    STROKETOL_StrokeSize      = 0x0002,
    STROKETOL_PatchSegments   = 0x0003,
    STROKETOL_XYProjection    = 0x0100,
    STROKETOL_Perspective     = 0x0200,
};

END_BENTLEY_GEOMETRY_NAMESPACE

#include <Geom/capi/bspmisc_capi.h>
#include <Geom/bsp/capi/bspsurf_capi.h>
#include <Geom/bsp/capi/bspproc_capi.h>
#include <Geom/bsp/capi/bsp_curvature_capi.h>
#include <Geom/bsp/capi/bspsurfacetrim_capi.h>
#include <Geom/capi/bsputil_capi.h>
#include <Geom/bsp/capi/bsprsurf_capi.h>
#include <Geom/bsp/capi/bspfit_capi.h>
#include <Geom/bsp/capi/bspdcurv_capi.h>
#include <Geom/bsp/capi/bspconv_capi.h>
#include <Geom/bsp/capi/bspprof_capi.h>
#include <Geom/bsp/capi/bspcci_capi.h>
#include <Geom/bsp/capi/bspdsurf_capi.h>
#include <Geom/bsp/capi/bspssi_capi.h>
#include <Geom/bsp/capi/bspicurv_capi.h>
#include <Geom/bsp/capi/bspmesh_capi.h>
#include <Geom/bsp/capi/bspcurvcurv_capi.h>
#include <Geom/capi/bspcurv_capi.h>
#include <Geom/bsp/capi/bspplank_capi.h>
#include <Geom/bsp/capi/plank_capi.h>
#include <Geom/bsp/capi/tristrip_capi.h>
#include <Geom/bsp/capi/bsp_mxspline_capi.h>
