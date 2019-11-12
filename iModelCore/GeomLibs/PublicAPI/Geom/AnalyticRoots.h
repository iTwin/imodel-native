/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

/*__BENTLEY_INTERNAL_ONLY__*/
/*__PUBLISH_SECTION_START__*/
/*__PUBLISH_SECTION_END__*/
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

#include <Geom/msgeomstructs_typedefs.h>

#ifdef __cplusplus

class GEOMDLLIMPEXP AnalyticRoots
{
public:
static int SolveLinear  (double c[2], double s[1]);
static int SolveQuadric (double c[3], double s[2]);
static int SolveCubic   (double c[4], double s[3]);
// farSolution is the root farthest from the inflection.
// This will have the steepest slope, hence the least numerical sensitivity.
static int SolveCubic   (double c[4], double s[3], double &farSolution);
static int SolveQuartic (double c[5], double s[4]);

static bool SafeDivide
(
double &result,
double numerator,
double denominator,
double defaultResult
);


static int SolveBezierQuadric
        (
        double y0,
        double y1,
        double y2,
        double e,
        double uvals[2],
        bool bRestrictSolutionsTo01
        );

// Find solutions (u values) of the bezier-form cubic
// y0 (1-u)^3 + 3 y1 u(1-u)^2 + 3 y2 u^2 (1-u) + y3 u^3= e
// i.e. y0, y1, y2, y3 are coefficients of bezier-basis polynomial, e is y level whose crossings
// are needed.
//
static int SolveBezierCubic
        (
        double y0,
        double y1,
        double y2,
        double y3,
        double e,
        double uvals[2],
        bool bRestrictSolutionsTo01
        );

// Find solutions (u values) of the bezier-form quartic
// y0 (1-u)u^4 + etc = e
//
static int SolveBezierQuartic
        (
        double y0,
        double y1,
        double y2,
        double y3,
        double y4,
        double e,
        double roots[4],
        bool bRestrictSolutionsTo01
        );

static int IntersectImplicitLineUnitCircle
(
double &c1,
double &s1,
double &c2,
double &s2,
int    &solutionType,
double alpha,
double beta,
double gamma,
double reltol
);

//! Compute the angle shfit {thetaMax} so that the trig form
//!            {beta * cos(theta) + gamma * sin(theta)}
//! can be written as
//!            {amplitiude * cos (theta - thetaMax)}
//! @param [in] beta cosine coefficient.
//! @param [in] gamma sine coefficient.
//! @param [out] thetaMin angle of minimum value.
//! @param [out] thetaMax angle of maximum value.
//! @param [out] amplitiude (positive) amplitude.
static bool NormalizeCosine (double beta, double gamma, double &thetaMin, double &thetaMax, double &amplitude);


//! Return 0 or 2 angle roots of  trig form {alpha + beta * cos + gamma * sin} for a bounded interval.
//! There is no test for tangency --- a (numerically) exact tangency goes in twice (unnoticed).
//! @param [in] alpha constant coff
//! @param [in] beta  cosine coff
//! @param [in] gamma sine coff
//! @param [in] theta0 start angle of interval
//! @param [in] sweep interval sweep
//! @param [out] angles 
//! @param [out] count angle count.  0 if no amplitude.
//! @param [out] thetaMax angle where maximum value alpha + amplitude occurs.
//! @param [out] thetaMin angle where minimum value alpha - almplitude occurs.
//! @param [out] amplitude (absolute) amplitude of wave.
static bool LinearTrigFormRoots (double alpha, double beta, double gamma, double *angle, size_t &count, double &thetaMax, double &thetaMin, double &amplitude);
};
#endif
END_BENTLEY_GEOMETRY_NAMESPACE
