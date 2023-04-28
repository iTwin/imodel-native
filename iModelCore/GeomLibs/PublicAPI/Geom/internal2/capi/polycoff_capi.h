/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

/*----------------------------------------------------------------------+
|                                                                       |
| name          bsiPolycoffs_init                                               |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt bsiPolycoffs_init

(
PolyCoffs *pA,      /* OUT     initialized polynomial */
int       degree    /* IN      polynomial degree. (degree + 1 coffs are zeroed */
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          bsiPolycoffs_raiseDegree                                        |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt bsiPolycoffs_raiseDegree

(
PolyCoffs *pA,      /* IN OUT  polynomial */
int       degree    /* IN      degree of polynomial */
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          bsiPolycoffs_setCoff                                    |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt bsiPolycoffs_setCoff

(
PolyCoffs *pA,      /* IN OUT  polynomial */
double    coff,     /* IN      coefficient to set */
int       degree    /* IN      degree of term */
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          bsiPolycoffs_addToCoff                                  |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt bsiPolycoffs_addToCoff

(
PolyCoffs *pA,      /* IN OUT  polynomial */
double    coff,     /* IN      value to add */
int       degree    /* IN      degree of term */
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          bsiPolycoffs_multiply                                           |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt bsiPolycoffs_multiply        /* degree of product = sum of degrees  */

(
PolyCoffs *pC,  /* OUT     product coefficients. */
const PolyCoffs *pA,  /* IN      factor coefficients */
const PolyCoffs *pB   /* =. factor coefficients */
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          bsiPolycoffs_initLinear                                 |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void bsiPolycoffs_initLinear

(
PolyCoffs *pA,      /* OUT     product coefficients.  Must be sized (d0 + d1 + 1) */
double  c0,         /* OUT     constant coefficient */
double  c1          /* OUT     linear coefficient */
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          bsiPolycoffs_initQuadratic                              |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void bsiPolycoffs_initQuadratic

(
PolyCoffs *pA,      /* OUT     product coefficients.  Must be sized (d0 + d1 + 1) */
double  c0,         /* OUT     constant coefficient */
double  c1,         /* OUT     linear coefficient */
double  c2          /* OUT     quadratic coefficient */
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          bsiPolycoffs_initSquaredLinear                          |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void bsiPolycoffs_initSquaredLinear

(
PolyCoffs *pA,      /* OUT     product coefficients.  Must be sized (d0 + d1 + 1) */
double  c0,         /* OUT     constant coefficient */
double  c1          /* OUT     linear coefficient */
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          bsiPolycoffs_add                                                |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void bsiPolycoffs_add

(
PolyCoffs *pC,  /* OUT     summed coefficients. */
const PolyCoffs *pA,  /* IN      term coefficients */
const PolyCoffs *pB   /* =. term coefficients */
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          bsiPolycoffs_addScaled                                  |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void bsiPolycoffs_addScaled

(
PolyCoffs *pC,  /* OUT     summed coefficients. */
const PolyCoffs *pA,  /* IN      term coefficients */
const PolyCoffs *pB,   /* =. term coefficients */
double scale    /* scale factor for pB */
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          bsiPolycoffs_initLineUnitTorus                          |
|                                                                       |
|                                                                       |
| Construct the (degree 4) polynomial for the intersection of a line    |
| and a unit torus.  (Torus major circle is unit circle in xy plane,    |
| minor circle radius given)                                            |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void bsiPolycoffs_initLineUnitTorus

(
PolyCoffs       *pA,      /* OUT     polynomial */
DPoint4dCP pPoint0, /* IN      line start point */
DPoint4dCP pPoint1, /* IN      line end point */
double          r         /* IN      minor radius */
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          bsiPolycoffs_realRoots                                  |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void           bsiPolycoffs_realRoots

(
PolyCoffs     *pRoot,     /* OUT     coefficients are roots */
const PolyCoffs     *pA         /* IN      polynomial to solve */
);

/*----------------------------------------------------------------------+
|                                                                       |
| name          bsiPolycoffs_getCoff                                    |
|                                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt    bsiPolycoffs_getCoff

(
double    *pCoff,               /* OUT     i'th coefficient */
const PolyCoffs *pA,            /* IN      coefficient array */
int       i
);

/*---------------------------------------------------------------------------------**//**
* @return the degree of the polynomial.
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bsiPolycoffs_getDegree

(
const PolyCoffs *pA             /* IN      polynomial whose degree is queried */
);

/*---------------------------------------------------------------------------------**//**
* Copy a polynomial and raise the output polynomial degree.  If
* input polynomial is a null pointer, set the output to a default constant
* value.
* @return SUCCESS if the copy and degree raising were completed.
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP StatusInt    bsiPolycoffs_copyOrInit

(
PolyCoffs *pOut,                /* OUT     destination polynomial */
const   PolyCoffs *pIn,                 /* IN      source polynomial */
double    defaultConstValue,    /* IN      constant term (constant polynomial) to use when pIn is null */
        int       defaultdegree         /* IN      minimum degree of output. */
);

/*---------------------------------------------------------------------------------**//**
* Evaluate the polynomial at the specified point.
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double    bsiPolycoffs_evaluate

(
const   PolyCoffs *pA,      /* IN      polynomial to evaluate. */
double     x        /* IN      point for evaluation */
);

