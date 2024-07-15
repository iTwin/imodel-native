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
Public GEOMDLLIMPEXP int      bspconv_convertLstringToCurve
(
int             *type,                  /* OUT     curve type */
int             *rational,              /* OUT     rational (weights included) */
BsplineDisplay  *display,               /* OUT     display parameters */
BsplineParam    *params,                /* OUT     number of poles etc. */
DPoint3d        **poles,                /* OUT     pole coordinates */
double          **knots,                /* OUT     knot vector */
double          **weights,              /* OUT     weights (if (Rational) */
DPoint3d        *pointP,                /* IN      points */
int             numPoints               /* IN      number of points */
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspconv_computeCurveFromArc
(
DPoint3d        *poles,                /* OUT     poles of arc */
double          *knots,                /* OUT     interior knots only */
double          *weights,
BsplineParam    *params,
double          start,
double          sweep,
double          axis1,
double          axis2
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int bspconv_convertArcToCurve
(
int             *type,                  /* OUT     curve type */
int             *rational,              /* OUT     rational (weights included) */
BsplineDisplay  *display,               /* OUT     display parameters */
BsplineParam    *params,                /* OUT     number of poles etc. */
DPoint3d        **poles,                /* OUT     pole coordinates */
double          **knots,                /* OUT     knot vector */
double          **weights,              /* OUT     weights (if (Rational) */
double          start,
double          sweep,
double          axis1,
double          axis2,
RotMatrix       *rotMatrixP,
DPoint3d        *centerP
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP StatusInt   bspconv_convertDEllipse3dToCurve
(
MSBsplineCurve  *pCurve,                /* OUT     curve structure to be initialized with newly allocated poles, knots, and weights. */
const DEllipse3d    *pEllipse           /* IN      ellipse */
);

/*----------------------------------------------------------------------+
@return curve primitive with one or more bspline curves of the offset of an ellipse.
@param pEllipse IN base ellipse
@param offsetDistance IN signed offset distance. Positive is outwards.
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP ICurvePrimitivePtr bspconv_convertDEllipse3dOffsetToCurveChain
(
DEllipse3dCP pEllipse,          /* => ellipse */
double       offsetDistance     /* => SIGNED offset. Positive is outward. */
);
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int     bspconv_arcToCurveStruct
(
MSBsplineCurve  *curveP,
double          start,
double          sweep,
double          axis1,
double          axis2,
RotMatrix       *rotMatrixP,
DPoint3d        *centerP
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int     bspconv_lstringToCurveStruct
(
MSBsplineCurve  *curveP,
DPoint3d        *pointP,
int             nPoints
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int bspconv_coneToSurface
(
MSBsplineSurface    *surf,
double              topRad,
double              baseRad,
RotMatrix           *rotMatrixP,
DPoint3d            *topCenterP,
DPoint3d            *baseCenterP
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspconv_dropToCurves
(
MSBsplineCurve      ***curves,         /* IN OUT  ptr to array of ptr's to */
int                 *numCurves,        /*  MSBsplineCurve structures */
MSBsplineSurface    *surface,
int                 direction
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int bspconv_extractCapAsSurface
(
MSBsplineSurface    *capP,              /* OUT     cap surface (planar) */
MSBsplineSurface    *solidP,            /* IN      surface */
double              tolerance,          /* IN      choord height tolerance */
bool                lastRow             /* IN      false gives surface start */
);

/*---------------------------------------------------------------------*//**
Convert an arc of center + vector0*cos(theta)+vector90*sin(theta)
Convert a portion of a DConic4d to a (single) bspline curve.
@param curveP OUT initialized curve structure
@param ellipseP OUT ellipse -- undefined if hyperbola, parabola, or line.
@param isEllipseP OUT true if the curve is (simply) an ellipse.
@param hConicP IN homogeneous conic.
@param s0 IN start parameter in conic angle space.
@param s1 IN end parameter in conic angle space
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt bspcurv_convertHomogeneousConicToCurve
(
MSBsplineCurve* curveP,         /* OUT     Curve structure to initialize and fill */
DEllipse3dP     ellipseP,
bool*           isEllipseP,     /* OUT     true if the spline has standard mapping to ellipse
                                            (e.g. false for any hyperbola or parabola) */
DPoint4dCP      centerP,
DPoint4dCP      vector0P,
DPoint4dCP      vector90P,
double          s0,             /* IN      start parameter */
double          s1              /* IN      end parameter */
);

/*---------------------------------------------------------------------*//**
Convert a portion of a DConic4d to a (single) bspline curve.
@param curveP OUT initialized curve structure
@param ellipseP OUT ellipse form -- undefined if hyperbola or parabola.
@param isEllipseP OUT true if the ellipse is defined
@param segmentP OUT segment if curve is (simply) a segment.
@param isSegmentP OUT true if segment is defined
@param hConicP IN homogeneous conic.
@param s0 IN start parameter in conic angle space.
@param s1 IN end parameter in conic angle space
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt bspcurv_simplifyHConic
(
MSBsplineCurve  *curveP,        /* OUT     Curve structure to initialize and fill */
DEllipse3dP      ellipseP,      /* OUT     ellipse (undefined if curve parabola or hyperbola) */
      bool      *isEllipseP, /* OUT     true if result is a standard ellipse */
DSegment3dP     segmentP,
      bool      *isSegmentP,
const HConic    *hConicP,       /* IN      conic to evaluate */
      double    s0,             /* IN      start parameter */
      double    s1              /* IN      end parameter */
);

END_BENTLEY_GEOMETRY_NAMESPACE

