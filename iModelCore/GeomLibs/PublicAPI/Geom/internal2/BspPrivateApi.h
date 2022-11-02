/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <Geom/bspApi.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE
/*----------------------------------------------------------------------+
|                                                                       |
|   Surface offset data                                                 |
|                                                                       |
+----------------------------------------------------------------------*/
typedef struct liftedgedata
    {
    int                 edge;
    double              value;
    double              distance;
    MSBsplineSurface    *surface;
    } LiftEdgeData;


/*----------------------------------------------------------------------+
|                                                                       |
|   Bezier Information Structure                                        |
| Each union variant in this structure is used in one classic bspline   |
| recursive subdivision algorithm -- ssi, offset, tube, silhouette etc. |
|                                                                       |
| All should be rewritten someday .....                                 |
+----------------------------------------------------------------------*/
typedef union bezierInfo
    {
#define COMPILE_OLD_DVector_tube
#ifdef COMPILE_OLD_DVector_tube
     struct                            /* 20 words */
        {
        MSBsplineSurface    *surface;
        MSBsplineSurface    *patch;
        MSBsplineCurve      *section;
        RotMatrix           *matrix;
        bool                rigidSweep;
        bool                closedTrace;
        } tube;
#endif
#define COMPILE_OLD_DVector_ssi
#ifdef COMPILE_OLD_DVector_ssi
    struct                             /* 356 words */
        {
        BoundBox        b0;
        BoundBox        b1;
        DRange2d       range0;
        DRange2d       range1;
        double          *uKts0;
        double          *vKts0;
        double          *uKts1;
        double          *vKts1;
        int             showMarch;
        SsiTolerance    *tol;
        IntLink         **chainPP;
        Evaluator       *eval0;
        Evaluator       *eval1;
        } ssi;
#endif
#define COMPILE_OLD_DVector_off
#ifdef COMPILE_OLD_DVector_off
    struct                             /* 100 words */
        {
        MSBsplineCurve  curve;
        double          error;
        double          *distance;
        double          tolerance;
        int             continuity;
        int             cuspTreatment;
        double          *knots;
        RotMatrix       *matrix;
        } off;
#endif        
#define COMPILE_OLD_DVector_silhouette
#ifdef COMPILE_OLD_DVector_silhouette
    struct
        {
        double          tolerance;
        DRange2d        range;
        BoundBox        box;
        double          *uKts;
        double          *vKts;
        Transform       viewTransform;
        } silhouette;
#endif
    void           *userDataP;
    } BezierInfo;

END_BENTLEY_GEOMETRY_NAMESPACE



