/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BENTLEY_GEOMETRY_NAMESPACE
//#include <Geom/msgeomstructs_typedefs.h>

#ifdef __cplusplus

//!
//! @description Abstract interface for multi-variate function of a single variable.
//!
class GEOMDLLIMPEXP BSIVectorIntegrand
{
public:
GEOMAPI_VIRTUAL ~BSIVectorIntegrand(){}
GEOMAPI_VIRTUAL void EvaluateVectorIntegrand (double t, double *pF) = 0;
GEOMAPI_VIRTUAL int  GetVectorIntegrandCount () = 0;
};

//!
//! @description Abstract interface for multi-variate function of a single variable.
//!
class GEOMDLLIMPEXP BSIIncrementalVectorIntegrand : public BSIVectorIntegrand
{
public:
GEOMAPI_VIRTUAL ~BSIIncrementalVectorIntegrand(){}
// Return true to continue integration.
GEOMAPI_VIRTUAL bool AnnounceIntermediateIntegral (double t, double *pIntegrals) = 0;
};


class BSIVectorIntegrandXY
{
public:
GEOMAPI_VIRTUAL void EvaluateVectorIntegrand (double x, double y, double *pF) = 0;
GEOMAPI_VIRTUAL int GetVectorIntegrandCount () = 0;
};


//!
//! @description Support class for numerical quadature
//! Constructor BSIQuadraturePoints () initializes for Simpson's rule.
//! Select other rules via InitNewtonCotes(numPoints) or InitGaussian(numPoints);
//!
struct GEOMDLLIMPEXP BSIQuadraturePoints
{
private:
    static int const MaxPoints = 20;
    double mWi[MaxPoints];
    double mUi[MaxPoints];
    int    mNumEval;

    double mU0, mU1;    // Limits of idealized interval.
    double mDivDU;      // Precomputed 1/ (mU1 - mU0);
    double mConvergencePower;    // Expected degree of convergence.

    bool AddEval (double u, double w);
    void Init(double u0, double u1);
public:

//!
//! @description Setup for a "Newton-Cotes" regularly spaced quadrature.
//! @remark Gaussian quadrature points are usually more effective.
//!   numEval == 1 is "rectangle rule"
//!   numEval == 2 is "trapezoid rule"
//!   numEval == 3 is "Simpson's rule"
//!
//! @param [in] numEval  requested number of points.
//! @returns Number of points actually to be used.  max is 4
//!
int InitUniform (int numEval);

//!
//! @description Setup for a "Gaussian" quadrature.
//! @param [in] numEval  requested number of points.
//! @returns Number of points actually to be used.  max is 5
//!
int InitGauss (int numEval);

//! @description Setup for "Gauss-Lobatto" quadrature. This uses endpoints plus gauss-like interior points.
//! @param [in] numEval requested number of points.
//! @remarks numEval may be 3, 4, and 5.
//! @remarks numEval == 3 is the same as "Simpson"
int InitGaussLobatto (int numEval);

//! @description Setup for a "Kronrod" part of Gauss-Kronrad quadrature.
//!   The simple Gauss x values appear (in order) at the odd positions of the Kronrod rule.
//! @param numEval IN requested number of points. ONLY 7 IS SUPPORTED
//! @param gaussPartner IN corresponding simple Gauss rule.
//! @returns Number of points actually to be used.
int InitGaussKronrod (int numEval, BSIQuadraturePoints &gaussPartner);

//!
//! @description
//! Get the coordinate a weight of the i'th quadrature point, mapped to
//!    interval a0..a1
//! @param [in] i  index of evaluation point.
//! @param [in] a0  start of mapped interval.
//! @param [in] a1  end of mapped interval.
//! @param [out]a  evaluation coordinate.
//! @param [out]w  weight.  Both the local quadrature weight and the
//!    interval length are incorporated in the returned weight.
//! @returns false if index is out of range.
//!
bool GetEval (int i, double a0, double a1, double &a, double &w) const;

//! Get the coordinates and weight of quarature point (i,j) in a 2D rectangular domain
//! with specified quadrature for each direction.
//! @param [in] xRule x direction quadrature rule
//! @param [in] ix point index in x rule.
//! @param [in] x0 started of x mapped interval
//! @param [in] x1 end of x mapped interval
//! @param [in] yRule y direction quadrature rule.
//! @param [in] iy point index in y rule.
//! @param [in] y0 start of y mapped interval
//! @param [in] y1 end of y mapped interval
//! @param [out] x x coordinate to evaluate.
//! @param [out] y y coordinate to evaluate.
static bool GetXYEval (
    BSIQuadraturePoints const &xRule, int ix, double x0, double x1,
    BSIQuadraturePoints const &yRule, int iy, double y0, double y1,
    double &x, double &y, double &w
    );
    


//!
//! @description Return the number of points in the quadrature rule.
//!
int GetNumEval () const;

//!
//! @description Return the exponent for the convergence rate of the
//!    rule.
//!
double GetConvergencePower () const;

//!
//! @description Evaluate and accumulate function values over an interval.
//! @param [in] function  function object that can be called as often as needed.
//! @param [in] t0  start of interval.
//! @param [in] t1  end of interval.
//! @param [in,out] pSums  accumulating sums.
//! @params numInterval IN number of intervals to use within t0..t1.
//!
void AccumulateWeightedSums
(
BSIVectorIntegrand &function,
double t0,
double t1,
double *pSums,
int     numInterval
);

/*----------------------------------------------------------------*//**
@description Integrate over an interval.
<ul>
<li> In each interval, form gauss sums with one and then two subintervals.
<li> Apply 1 step Romberg (Richardson) extrapolation.
<li> Accept the extrapolation as that interval's contribution.
<li> Accumulate the max extrapolation as error bound.
<li> The ongoing integration at the end of each interval,
<li> exit if the announcement function returns false
<li> The caller is can capture sums as announced
</ul>
@param [in] function function object that can be called as often as needed.
@param [in t0 start of interval.
@param [in] t1 end of interval.
@params [in] numInterval number of intervals to use within t0..t1.
@params [out] totalErrorBound 
@return false if function.AnnounceIntermediateIntergral () returned false.
@bsimethod
+---------------+---------------+---------------+---------------+------*/
bool IntegrateWithRombergExtrapolation
(
BSIIncrementalVectorIntegrand &function,
double t0,
double t1,
uint32_t numInterval,
double &totalErrorBound
);
//!
//! constructor -- initialize to Simpson's rule.
//!
BSIQuadraturePoints ();
};



//!
//! @description Support class for numerical quadature OVER TRIANGLE
//! Constructor BSIQuadraturePoints () initializes for cornerpoint only.
//! Select other rules via InitStrang
//!
struct GEOMDLLIMPEXP BSITriangleQuadraturePoints
{
private:
    static int const MaxPoints = 20;
    double mWi[MaxPoints];
    double mUi[MaxPoints];
    double mVi[MaxPoints];
    int    mNumEval;

    double mConvergencePower;    // Expected degree of convergence.

    bool AddEval (double u, double v, double w);
    void Init();
public:


//!
//! @description Setup for a "Strang" rule as defined in http://people.sc.fsu.edu/~jburkardt/datasets/quadrature_rules_tri/quadrature_rules_tri.html
//! @param [in] selector rule selector.
//! @remarks; Selectors are:
//!         (1 --- centroid;  exact degree 1)
//!         (2 --- 3 midsides;  exact degree 2)
//!         (3 --- centroid and one towards each vertex;  exact degree 3)
//!         (4 --- 6 points, vertices and midsides of an inset triangle;  exact degree 3)
//!         (5 --- 6 points, 2 inset from each edge;  exact degree 4)
//!         (6 --- 7 points, centroid and 2 insets from each edge -- insets equally weighted;  exact degree 4)
//!         (7 --- 7 points, centroid, one towards each vertex, one towards each edge;  exact degree 5)
//!         (8 --- 9 points;  exact degree 6)
//!         (9 --- 12 points;  exact degree 6)
//!         (10 --- 13 points -- negative weight at centroid;  exact degree 7)
int InitStrang (int selector);

//!
//! @description
//! Get the coordinates and weight of the i'th quadrature point
//! @param [in] i  index of evaluation point.
//! @param [out] u  evaluation coordinate.
//! @param [out] v  evaluation coordinate.
//! @param [out] w  weight.  
//! @returns false if index is out of range.
//!
bool GetEval (int i, double &u, double &v, double &w) const;    

//!
//! @description Return the number of points in the quadrature rule.
//!
int GetNumEval () const;

//!
//! @description Return the exponent for the convergence rate of the
//!    rule.
//!
double GetConvergencePower () const;

//!
//! constructor -- initialize to corner rule. (all weights 1/3, uv coordinates (00) (10) (01)
//! Note all rules have weights that add to ONE -- 
//!
BSITriangleQuadraturePoints ();

void AccumulateWeightedSums(BSIVectorIntegrandXY &function, double *pSums);

void AccumulateWeightedSumsMapped
    (
    BSIVectorIntegrandXY &function, double *pSums, 
    double ax, double ay,
    double bx, double by,
    double cx, double cy
    );

};



#endif
END_BENTLEY_GEOMETRY_NAMESPACE
