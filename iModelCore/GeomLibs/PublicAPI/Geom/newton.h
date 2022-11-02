/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

#include <Geom/msgeomstructs_typedefs.h>

#ifdef __cplusplus

// Interface for an object that can evaluate a function from R to R
class GEOMDLLIMPEXP  FunctionRToR
{
public:
// Virtual function
GEOMAPI_VIRTUAL ValidatedDouble EvaluateRToR (double u) = 0;
};


// Interface for an object that can evaluate 2 functions of 2 variables with derivatives.
class GEOMDLLIMPEXP  FunctionRRToRRD
{
public:
// Virtual function
// @param [in] u  first variable
// @param [in] v  second variable
// @param [out]f  first function value
// @param [out]g  second function value
// @param [out]dfdu  derivative of f wrt u
// @param [out]dfdv  derivative of f wrt v
// @param [out]dgdu  derivative of g wrt u
// @param [out]dgdv  derivative of g wrt v
// @return true if function was evaluated.
GEOMAPI_VIRTUAL bool EvaluateRRToRRD (double u, double v, double &f, double &g, double &dfdu, double &dfdv, double &dgdu, double &dgdv) = 0;

#define DECLARE_VIRTUALS_FunctionRRToRRD(PREFIX,SUFFIX) \
PREFIX bool EvalauteRRToRRD(double,double, double&, double&,\
         double&, double&,  double&, double&) SUFFIX;
};


// Interface for an object that can evaluate a 1d to 1d function and derivative.
class GEOMDLLIMPEXP  FunctionRToRD
{
public:
// Virtual function
// @param [in] u  variable
// @param [out]f  function value
// @param [out]dfdu  derivative of f wrt u
// @return true if function was evaluated.
GEOMAPI_VIRTUAL bool EvaluateRToRD
(
double u,
double &f,
double &dfdu
) = 0;

};



// Interface for an object that can evaluate 2 functions of 2 variables (no derivatives)
class GEOMDLLIMPEXP  FunctionRRToRR
{
public:
// Virtual function
// @param [in] u  first variable
// @param [in] v  second variable
// @param [out]f  first function value
// @param [out]g  second function value
GEOMAPI_VIRTUAL bool EvaluateRRToRR
(
double u,
double v,
double &f,
double &g
) = 0;

#define DECLARE_VIRTUALS_FunctionRRToRR_NoD(PREFIX,SUFFIX) \
PREFIX bool EvalauteRRToRR(double,double, double&, double&) SUFFIX;
};

class GEOMDLLIMPEXP  FunctionRRRToRRR
{
public:
// Virtual function
// @param [in] uvw current values of 3 independent variables.
// @param [out]f  3 computed function values
GEOMAPI_VIRTUAL bool EvaluateRRRToRRR
(
DVec3dCR uvw,
DVec3dR fgh
) = 0;

};

// Interface for an object that can evaluate 2 functions of 1 variable with derivatives.
class GEOMDLLIMPEXP  FunctionRToRRD
{
public:
// Virtual function
// @param [in] u  first variable
// @param [out]f  first function value
// @param [out]dfdu  derivative of f wrt u
// @param [out]dfdv  derivative of f wrt v
// @return true if function was evaluated.
GEOMAPI_VIRTUAL bool EvaluateRToRRD
(
double u,
double &f0,
double &f1,
double &dfdu,
double &dfdv
) = 0;

// Virtual function
// @param [in] u  first variable
// @param [out]f  first function value
// @return true if function was evaluated.
GEOMAPI_VIRTUAL bool EvaluateRToRR
(
double u,
double &f0,
double &f1
) = 0;


#define DECLARE_VIRTUALS_FunctionRToRR(PREFIX,SUFFIX) \
PREFIX bool EvaluateRToRRD(double,double&, double&, double&, double&) SUFFIX;\
PREFIX bool EvaluateRToRR(double,double&, double&) SUFFIX;
};


class GEOMDLLIMPEXP NewtonIterationsRRToRR
{
protected:
    double mAbstolU;
    double mAbstolV;
    double mAbstolW;
    double mReltolU;
    double mReltolV;
    double mReltolW;
    double mSoftAbstolU;
    double mSoftAbstolV;
    double mSoftAbstolW;
    int    mMaxIterations;
    int    mSuccessiveConvergenceTarget;
    double mMinStepU;
    double mMinStepV;
public:

// Default constructor sets up arbitrary tolerances.
NewtonIterationsRRToRR ();

// Detailed setup with same tolerances for both vars ...
// @param [in] abstol  absolute tolerance (required, to distinguish this from default constructor)
// @param [in] reltol  relative tolerance.
// @param [in] maxIterations  maximum number of iterations.
// @param [in] successiveConvergenceTarget  number of successive iterations which must be converged to
//              accept.  A common strategy is to set tolerances to some "medium" level -- e.g. about
//              12 digits -- but require 2 successive iterations to converge.  This will "usually"
//              lead to full 16 digit accuracy but allow ornery functions to quit at the medium convergence
NewtonIterationsRRToRR
(
double abstol,
double reltol = 1.0e-12,
int    maxIterations = 20,
int    successiveConvergenceTarget = 2,
double minStep = 1.0e-11
);

// Set a tolerance for convergence in cases that get close and have a failed step.
void SetSoftTolerance (double abstolU, double absTolV);


// Convergence tester.
// Returns true if FULLY CONVERGED.
// Returns false if any convergence condition is not satisfied.
// Note that the numConverged parameter may be either set to zero or incremented by
//   this test.
// @param [in] u  first  variable.
// @param [in] v  second variable.
// @param [in] f  first function value.
// @param [in] g  second function value.
// @param [in] du  proposed update to u. Usually left unchanged, but may be adjusted by the implementation.
// @param [in] dv  proposed update to v. Usually left unchanged, but may be adjusted by the implementation.
// @param [in,out] numConverged  evolving counter of successive steps that have converged.
GEOMAPI_VIRTUAL bool CheckConvergence
(
double u,
double v,
double f,
double g,
double &du,
double &dv,
int &numConverged
);

// Convergence tester.
// Returns true if FULLY CONVERGED.
// Returns false if any convergence condition is not satisfied.
// Note that the numConverged parameter may be either set to zero or incremented by
//   this test.
// @param [in] U  independent var
// @param [in] F  function values
// @param [in,out] proposed update to U.   May be modified.
// @param [in,out] numConverged  evolving counter of successive steps that have converged.
GEOMAPI_VIRTUAL bool CheckConvergence
(
DVec3dCR U,
DVec3dCR F,
DVec3dCR dU,
int &numConverged
);

// Run Newton stesp for a function represented by a FunctionRRToRR, ignoring off-diagonal parts.
// This is invoked as an emergency backup internally when regular Newton fails.
// @param [in,out] u  first  variable.
// @param [in,out] v  second variable.
// @param [in] evaluator  evaluator object.
// @param [in] factor to apply to steps
bool RunDiagonalNewton (
        double &u, double &v,
        FunctionRRToRRD &evaluator
        );

// Test if an iteration can be started.
// @param [in,out] u  first  variable.  Usually left unchanged, but may be adjusted (e.g. forced to limits) by the implementation.
// @param [in,out] v  second variable.  Usually left unchanged, but may be adjusted (e.g. forced to limits) by the implementation.
// @param [in] numIterations  number of iterations completed.
GEOMAPI_VIRTUAL bool CheckIterationStart
(
double &u,
double &v,
int numIterations
);

// Test if an iteration can be started.
// @param [in,out] uvw independent vars.   Usually left unchanged, but may be adjusted (e.g. forced to limits) by the implementation.
// @param [in] numIterations  number of iterations completed.
GEOMAPI_VIRTUAL bool CheckIterationStart
(
DVec3dCR uvw,
int numIterations
);

// Run Newton stesp for a function represented by a FunctionRRToRRD
// @param [in,out] u  first  variable.
// @param [in,out] v  second variable.
// @param [in] evaluator  evaluator object.
// @param [in] maxdu  max step for u
// @param [in] maxdv  max step for v
bool RunApproximateNewton (double &u, double &v, FunctionRRToRR &evaluator, double maxdu, double maxdv);

// Run Newton stesp for a function represented by a FunctionRRToRRD
// @param [in,out] uvw independent variable
// @param [in] evaluator  evaluator object.
// @param [in] maxDuvw max allowed steps for u,v,w
bool RunApproximateNewton (DVec3dR uvw, FunctionRRRToRRR &evaluator, DVec3dCR maxDuvw);


// Run Newton stesp for a function represented by a FunctionRRToRR,
//    evaluating derivatives numerically.
// @param [in,out] u  first  variable.
// @param [in,out] v  second variable.
// @param [in] evaluator  evaluator object.
bool RunNewton (
        double &u, double &v,
        FunctionRRToRRD &evaluator
        );


// Run Newton stesp for a function represented by two FunctionRToRRD
// @param [in,out] u  first  variable.
// @param [in,out] v  second variable.
// @param [in] evaluatorA  first evaluator object
// @param [in] evaluatorB  second evaluator object
bool RunNewtonDifference (double &u, double &v,
            FunctionRToRRD &evaluatorA, FunctionRToRRD &evaluatorB);

//! Run Newton stesp for a function represented by a FunctionRToRRD
bool RunNewton (
double &u,                  //!< [in,out] u  variable.
FunctionRToRD &evaluator    //!< [in] evaluator  evaluator object.
);

};

//!
//! Wrapper class to for DEllipse3d to participate in generic xy iterative searches.
//!
class GEOMDLLIMPEXP Function_DEllipse3d_AngleToXY : public FunctionRToRRD
{
private:
    DEllipse3d mEllipse;
public:
Function_DEllipse3d_AngleToXY (DEllipse3dCR ellipse);
DECLARE_VIRTUALS_FunctionRToRR (, override)
};

//!
//! Wrapper class to for DEllipse3d with offset to participate in generic xy iterative searches.
//! Ellipse parameterization is
//!    X = center + U cos q + V sin q
//!    tangent = - U sin(theta) + V cos(theta)
//!    normalizedPlaneNormal = normalizedCrossProduct (U, V)
//!    curvePerp = tangent cross normalizedPlaneNormal
//!
class GEOMDLLIMPEXP Function_DEllipse3dOffset_AngleToXY : public FunctionRToRRD
{
private:
    DEllipse3d mEllipse;
    // Precomputed plane normal (vector0 CROSS vector90, normalized)
    DVec3d mPlaneNormal;
    // Precomputed vector0 CROSS planeNormal ...
    DVec3d mUcrossN;
    // Precomputed vector90 CROSS planeNormal ...
    DVec3d mVcrossN;
    // Offset ...
    double mOffset;
public:
Function_DEllipse3dOffset_AngleToXY (DEllipse3dCR ellipse, double offset);
DECLARE_VIRTUALS_FunctionRToRR (, override)
};


//!
//! Wrapper class to for Bezier curve to participate in generic xy iterative searches.
//!
class GEOMDLLIMPEXP Function_Bezier_FractionToXY : public FunctionRToRRD
{
private:
    static const int sMaxCurveOrder = 26;
    DPoint4d mPoles[sMaxCurveOrder];
    int mOrder;
    bool mbIsUnitWeight;
public:
Function_Bezier_FractionToXY (DPoint4dCP pPoles, int order);
Function_Bezier_FractionToXY (DPoint3dCP pPoles, int order);
DECLARE_VIRTUALS_FunctionRToRR (, override)
};


// Utility/Callback struture for the function  f(x) = x + gamma * x^5.
// which appears in myriad settings when dealing with clothoids.
// <ul>
// <li> This is used "in both directions" for clothoid approximations with
// <li> s = distance along clothoid
// <li> x = distance along x axis (starting at the inflection)
// <li> gamma = a constant, typically something like 1/(2 R L)^2.
// <li> The true series is 
// <li> x = s - (1/5)(1/2) A s^5 + (1/9) (1/24) A^2 s^9 +  . . .
// <li> As a (very good) approximation,     x = s - (1/5)(1/2) A s^5
// <li> If x is known, this class is the iterative function with to solve for s with gamma = -1/(40 R^2 L^2)
// <li> (NOTE THE negative sign there)
// <li> A (good) approximation of the inverse is
// <li>  s = x + (1/5)(1/2) A s^5
// <li> If s is known, this class is the iterative function to solve for x with gamma = +1/(40 R^2 L^2)
// <li> (NOTE THE positive sign there)
// <ul
struct GEOMDLLIMPEXP ClothoidCosineApproximation : FunctionRToRD
{
double m_targetValue;
double m_gamma;
//! Publicly available counter of iterative calls.  This is cleared by methods that trigger newton calls.
static size_t s_evaluationCount;
//! 
//! <ul>
//! <li>Instantiate the clothoid cosine approximation function with caller-supplied coefficient of u^5 term.
//! <li>This is a low level constructor -- the caller is responsible for incorporating typical R, L and sign into gamma.
//! </ul>
ClothoidCosineApproximation (double targetValue, double gamma);

//! Evaluate {f(u) = u + gamma * u^5 - targetValue}
bool EvaluateRToRD (double u, double &f, double &dfdu) override;

//! @returns Given target value f, return u so {u(1 + alpha * u^4/ (40 R*R*L*L) = f}
//! <ul>
//! <li>In use case with {alpha = positive one}, f is distance along spiral and the return value is (approximate) distance along axis.
//! <li>In use case with (alpha = negative one}, f is distance along axis and the return value is (approximate) distance along spiral.
//! <li>If curvature (rather than R) is known, call with (alpha * curvature * curvature, 1.0, L, f)
//! <li>Returns the computed {coff= alpha/(/ (40 R*R*L*L)}
//! </ul>
//!
static ValidatedDouble Invert40R2L2Map (double alpha, double R, double L, double f, double &coff);
//! @returns {f(u) = u * (1 + alpha * u^4/ (40 R R L L)}
//! <ul>
//! <li>In use case with {alpha = positive one}, return value is (approximate) distance along spiral and u is distance along axis.
//! <li>In use case with (alpha = negative one}, u is distance along spiral and the return value is (approximate) distance along axis.
//! </ul>
static double Evaluate40R2L2Map (double alpha, double R, double L, double u);

//! @returns {f(u) = u * (1 + alpha * u^4/ (40 R R L L)}, along with 3 derivatives as return parameters.
//! <ul>
//! <li>In use case with {alpha = positive one}, return value is (approximate) distance along spiral and u is distance along axis.
//! <li>In use case with (alpha = negative one}, u is distance along spiral and the return value is (approximate) distance along axis.
//! </ul>
static double Evaluate40R2L2Map (double alpha, double R, double L, double u, double &dfdu, double &d2fdu2, double &d3fdu3);

//! @returns {f(u) = u * (1 + alpha * u^4/ (10 (4 R R - L L)  L L)}
//! <ul>
//! <li>In use case with {alpha = positive one}, return value is (approximate) distance along spiral and u is distance along axis.
//! <li>In use case with (alpha = negative one}, u is distance along spiral and the return value is (approximate) distance along axis.
//! </ul>
static double EvaluateItalianCzechR2L2Map (double alpha, double R, double L, double u);

//! @returns Given target value f, return u so {u(1 + alpha * u^4/ (10 (4 R R - L L)  L L) = f = u (1 + coff * u^4}
//! <ul>
//! <li>In use case with {alpha = positive one}, f is distance along spiral and the return value is (approximate) distance along axis.
//! <li>In use case with (alpha = negative one}, f is distance along axis and the return value is (approximate) distance along spiral.
//! <li>For L/R < 0.1, the demoniator differs from {40 R R L L} by (only!) 0.25%.
//! </ul>
static ValidatedDouble InvertItalianCzechR2L2Map (double alpha, double R, double L, double f, double &coff);

//! return the czech/italian gamma factor {2R/ sqrt(4 R R - L L)}
static double CzechGamma (double R, double L);

//! @returns {f(u) = u * (1 + gamma * u^4}, along with 3 derivatives as return parameters.
static double EvaluateU4Map(double gamma, double u, double &dfdu, double &d2fdu2, double &d3fdu3);
};

// Utility/Callback struture for the Polish cubic and its degree-17 polynomial for distance as a function of x.

struct GEOMDLLIMPEXP PolishDistanceApproximation: FunctionRToRD
    {
    double m_targetDistance;
    double m_radius1;
    double m_length1;
    double m_x4TermCoefficient;
    double m_cubicYCoefficient;
    
    void SetTargetDistance (double distance);
    //! Publicly available counter of iterative calls.  This is cleared by methods that trigger newton calls.
    static size_t s_evaluationCount;
    //! 
    //! <ul>
    //! <li>Instantiate the clothoid cosine approximation function with caller-supplied length1 and radius1
    //! <li>Precompute useful coefficients
    //! <li>start with target distance 0.
    //! </ul>
    PolishDistanceApproximation(double radius1, double length1);

    //! Evaluate the term x^4 / (4 length1^2 radius1^2) ..
    double SeriesX4Term(double x);

    //! Evaluate {f(u) = x * (1 + a0 * x^4 + a1 * x^8 + a2 * x^12 + a3 * x^16)}
    bool EvaluateRToRD(double u, double &f, double &dfdu) override;

    //! Evaluate {y = m * x^3} on the cubic
    double XToY (double u);
    // Evaluate distance-at-x with direct use of radius1 and length1 (not related to instance)
    static double XToApproximateDistance(double radius1, double length1, double x);
    // Evaluate distance-at-x with direct use of radius1 and length1 (not related to instance)
    static double XToApproximateDistanceD(double radius1, double length1, double x,
        double *dSdx = nullptr,
        double *d2Sdx2 = nullptr,
        double *d3Sdx3 = nullptr);
    // Return the coefficient of x^4 parts of the bigger series
    static double SeriesX4Coefficient(double radius1, double length1);
    // Return the coefficient of y = m * x^3
    static double XToYCubicCoefficient(double radius1, double length1);


    //! @returns Given target distance, return x so XToApproximateDistance (x) matches targetDistance
    ValidatedDouble InvertXToApproximateDistance (double targetDistance);

    };


#endif
END_BENTLEY_GEOMETRY_NAMESPACE
