/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__BENTLEY_INTERNAL_ONLY__*/

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/* We allow CURVES to be order 26, but must be prepared for (much!!) higher degree
   1D polynomials as intermediate computational forms.
*/
#define MAX_BEZIER_CURVE_ORDER 26
#define MAX_BEZIER_ORDER MAX_BEZIER_CURVE_ORDER*3

typedef StatusInt (*DPoint4dArrayHandler) (void *, const DPoint4d *, int);

typedef bool    (*DPoint4dSubdivisionHandler) (void *, const DPoint4d *, int, double, double);

#ifndef EXTRACT_CONTEXT_DEFINED
#define EXTRACT_CONTEXT_DEFINED
/*
B-spline curve info and local storage for extracting the Bez spline pole
corresp to jth Bez knot.  Shared among successive calls to
bsiBezierDPoint4d_extractNextBezierFromBspline().
*/
typedef struct _ExtractContext
    {
    /* B-spline curve info */
    const DPoint4d  *pPoles;
    int             numPoles;
    const double    *pKnots;
    int             numKnots;
    double          knotTolerance;
    int             order;
    bool            bClosed;

    /* local storage */
    double      pLocalBezKnots[2 * (MAX_BEZIER_CURVE_ORDER - 1)];
    double      pLocalBspKnots[2 * MAX_BEZIER_CURVE_ORDER - 1];
    DPoint4d    pLocalBspPoles[MAX_BEZIER_CURVE_ORDER];
    DPoint4d    sharedPole;     // between successive Bezier segments
    int         mu;             // highest master idx s.t. BspKnot[mu] <= next Bezier start knot
    int         finalMu;        // index of rightmost knot in the last interior B-spline knot cluster
    int         status;         // 0 signals first call/last call
    int         mult;           // multiplicity of the knot between successive Bezier segments
    int         finalMult;      // multiplicity of the last knot
    } ExtractContext;

#endif
/* Minimum relative distance separating unique knots in B-splines */
#define RELATIVE_BSPLINE_KNOT_TOLERANCE 1E-12

// Minimum relative distance for computing the multiplicity of start/end knots in B-splines.
// Needs to be coarser to catch IGDS-era knot discrepancies in fake-periodic curves
// and DXF slop (TR #143684).
#define RELATIVE_BSPLINE_EXT_KNOT_TOLERANCE 1E-7


END_BENTLEY_GEOMETRY_NAMESPACE


#include "bezeval.fdf"
#include "bezroot.fdf"
#include "bezierDPoint4d.fdf"
#include "implicitbezier.fdf"
#include "ruledBezier.fdf"

