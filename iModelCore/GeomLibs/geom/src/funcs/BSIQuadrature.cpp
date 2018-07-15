/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/funcs/BSIQuadrature.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include <math.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE


/*-----------------------------------------------------------------*//**
@description Add a single point to the current rule.
@bsimethod                                  EarlinLutz      10/07
+---------------+---------------+---------------+---------------+------*/
bool BSIQuadraturePoints::AddEval (double u, double w)
    {
    if (mNumEval < MaxPoints)
        {
        mUi[mNumEval] = u;
        mWi[mNumEval] = w;
        mNumEval++;
        }
    return false;
    }

/*-----------------------------------------------------------------*//**
@description First step of setting up a quadrature rule.
Retain interval start and end, precompute inverse interval length, and set
point count to zero.
@bsimethod                                  EarlinLutz      10/07
+---------------+---------------+---------------+---------------+------*/
void BSIQuadraturePoints::Init(double u0, double u1)
    {
    mU0 = u0;
    mU1 =  u1;
    mDivDU = 1.0 / (mU1 - mU0);
    mNumEval = 0;
    }



/*-----------------------------------------------------------------*//**
@description Setup for a "Newton-Cotes" regularly spaced quadrature.
@remark Gaussian quadrature points are usually more effective.
   numEval == 1 is "rectangle rule"
   numEval == 2 is "trapezoid rule"
   numEval == 3 is "Simpson's rule"

@param numEval IN requested number of points.
@returns Number of points actually to be used.  max is 4
 @bsimethod                                  EarlinLutz      10/07
+---------------+---------------+---------------+---------------+------*/
int BSIQuadraturePoints::InitUniform (int numEval)
    {
    Init (0.0, 1.0);
    if (numEval < 1)
        {
        AddEval (0.5, 1.0);
        mConvergencePower = 2.0;
        }
    else if (numEval == 2)
        {
        AddEval (0.0, 0.5);
        AddEval (1.0, 0.5);
        mConvergencePower = 2.0;    // No better the rectangle.
        }
    else if (numEval == 3)
        {
        double a = 1.0 / 6.0;
        AddEval (0.0, a);
        AddEval (0.5, 4.0 / 6.0);
        AddEval (1.0, 1);
        mConvergencePower = 4.0;
        }
    else //if (numEval == 4)
        {
        double w3 = 3.0 / 8.0;
        double w1 = 1.0 / 8.0;
        AddEval (0.0, w1);
        AddEval (1.0 / 3.0, w3);
        AddEval (2.0 / 3.0, w3);
        AddEval (1.0, w1);
        mConvergencePower = 4.0;    // No better than Simpson.
        }
    return mNumEval;
    }

/*-----------------------------------------------------------------*//**
@description Setup for a "Gaussian" quadrature.
@param numEval IN requested number of points.
<returns>Number of points actually to be used.  max is 4</returns>
 @bsimethod                                  EarlinLutz      10/07
+---------------+---------------+---------------+---------------+------*/
int BSIQuadraturePoints::InitGauss (int numEval)
    {
    Init (-1.0, 1.0);

    if (numEval <= 1)
        {
        AddEval (0.0, 2.0);
        mConvergencePower = 2.0;
        }
    else if (numEval == 2)
        {
        double a = 1.0 / sqrt (3.0);
        AddEval (-a, 1.0);
        AddEval ( a, 1.0);
        mConvergencePower = 4.0;
        }
    else if (numEval == 3)
        {
        double a = sqrt (0.6);
        double b = 5.0 / 9.0;
        AddEval (-a, b);
        AddEval (0.0, 8.0 / 9.0);
        AddEval ( a, b);
        mConvergencePower = 6.0;
        }
    else if (numEval == 5)
        {
        double q = 2.0 * sqrt (10.0 / 7.0);
        double b = 13.0 * sqrt (70.0);
        double a1 = sqrt (5.0 - q) / 3.0;
        double a2 = sqrt (5.0 + q) / 3.0;
        double w1 = (322.0 + b) / 900.0;
        double w2 = (322.0 - b) / 900;
        AddEval (-a2, w2);
        AddEval (-a1, w1);
        AddEval (0.0, 128.0 / 225.0);
        AddEval ( a1, w1);
        AddEval ( a2, w2);
        mConvergencePower = 10.0;
        }
    else if (numEval == 7)
        {
        AddEval (-0.949107912342758524526189684048,   0.129484966168869693270611432679);
        AddEval (-0.741531185599394439863864773281,   0.279705391489276667901467771424);
        AddEval ( -0.405845151377397166906606412077,  0.381830050505118944950369775489);
        AddEval (  0.0,                               0.417959183673469387755102040816);
        AddEval (  0.405845151377397166906606412077,  0.381830050505118944950369775489);
        AddEval (  0.741531185599394439863864773281,  0.279705391489276667901467771424);
        AddEval (  0.949107912342758524526189684048,  0.129484966168869693270611432679);
        mConvergencePower = 14;
        }
    else // if (numEval == 4)
        {
        double q = 2.0 * sqrt (6.0 / 5.0);
        double r = sqrt (30.0);
        double a = sqrt ((3 - q) / 7.0);
        double b = (18.0 + r) / 36.0;
        AddEval (-a, b);
        AddEval ( a, b);
        double aa = sqrt ((3 + q) / 7.0);
        double bb = (18.0 - r) / 36.0;
        AddEval (-aa, bb);
        AddEval ( aa, bb);
        mConvergencePower = 8.0;
        }
    return mNumEval;
    }

/*-----------------------------------------------------------------*//**
@description Setup for a "Gaussian" quadrature.
@param numEval IN requested number of points.
<returns>Number of points actually to be used.  max is 4</returns>
 @bsimethod                                  EarlinLutz      10/07
+---------------+---------------+---------------+---------------+------*/
int BSIQuadraturePoints::InitGaussLobatto (int numEval)
    {
    Init (-1.0, 1.0);

    if (numEval <= 2)       // rectangle
        {
        AddEval (0.0, 1.0);
        AddEval (1.0, 1.0);
        mConvergencePower = 2.0;
        }
    else if (numEval == 3)  // simpson
        {
        double a = 1.0 / 3.0;
        AddEval (-1.0, a);
        AddEval (0.0, 4.0 * a);
        AddEval (1.0, a);
        mConvergencePower = 4.0;
        }
    else if (numEval == 4)
        {
        double a = sqrt (0.2);
        double b = 1.0 / 6.0;
        AddEval (-1.0, b);
        AddEval (-a, 5.0 * b);
        AddEval (a, 5.0 * b);
        AddEval ( 1.0, b);
        mConvergencePower = 6.0;
        }
    else
        {
        double q1 = 0.10;
        double q0 = 32.0 / 45.0;
        double qa = 49.0 / 90.0;
        double a = sqrt (3.0 / 7.0);
        AddEval (-1.0, q1);
        AddEval (-a, qa);
        AddEval (0.0, q0);
        AddEval (a, qa);
        AddEval ( 1.0, q1);
        mConvergencePower = 8.0;
        }
    return mNumEval;
    }



/*-----------------------------------------------------------------*//**
@description
 Get the coordinate a weight of the i'th quadrature point, mapped to
    interval a0..a1
@param i IN index of evaluation point.
@param a0 IN start of mapped interval.
@param a1 IN end of mapped interval.
@param a  OUT evaluation coordinate.
@param w  OUT weight.  Both the local quadrature weight and the
    interval length are incorporated in the returned weight.
@returns false if index is out of range.
 @bsimethod                                  EarlinLutz      10/07
+---------------+---------------+---------------+---------------+------*/
bool BSIQuadraturePoints::GetEval (int i, double a0, double a1, double &a, double &w) const
    {
    if (0 <= i && i < mNumEval)
        {
        double f = mDivDU * (a1 - a0);
        a = a0 + (mUi[i] - mU0) * f;
        w = mWi[i]* f;;
        return true;
        }
    else
        {
        a = 0.5 * (a0 + a1);
        w = 0.0;
        return false;
        }
    }

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
bool BSIQuadraturePoints::GetXYEval
(
BSIQuadraturePoints const &xRule, int ix, double x0, double x1,
BSIQuadraturePoints const &yRule, int iy, double y0, double y1,
double &x, double &y, double &w
)
    {
    double wx, wy;
    if (xRule.GetEval (ix, x0, x1, x, wx)
        && yRule.GetEval (iy, y0, y1, y, wy))
        {
        w = wx * wy;
        return true;
        }
    return false;
    }


/*-----------------------------------------------------------------*//**
@description Return the number of points in the quadrature rule.
 @bsimethod                                  EarlinLutz      10/07
+---------------+---------------+---------------+---------------+------*/
int BSIQuadraturePoints::GetNumEval () const
    {
    return mNumEval;
    }

/*-----------------------------------------------------------------*//**
@description Return the exponent for the convergence rate of the
    rule.
 @bsimethod                                  EarlinLutz      10/07
+---------------+---------------+---------------+---------------+------*/
double BSIQuadraturePoints::GetConvergencePower () const
    {
    return mConvergencePower;
    }

/*-----------------------------------------------------------------*//**
@description Evaluate and accumulate function values over an interval.
@param function IN function object that can be called as often as needed.
@param t0 IN start of interval.
@param t1 IN end of interval.
@param pSums INOUT accumulating sums.
@params numInterval IN number of intervals to use within t0..t1.
 @bsimethod                                  EarlinLutz      10/07
+---------------+---------------+---------------+---------------+------*/
void BSIQuadraturePoints::AccumulateWeightedSums
(
BSIVectorIntegrand &function,
double t0,
double t1,
double *pSums,
int     numInterval
)
    {
    int nFunc = function.GetVectorIntegrandCount ();
    if (nFunc < 1)
        return;
    double *pF = (double*)_alloca (nFunc * sizeof (double));
    if (numInterval < 1)
        numInterval = 1;
    int numEval = GetNumEval ();
    double dt = (t1 - t0) / numInterval;
    for (int interval = 0; interval < numInterval; interval++)
        {
        double tA = t0 + interval * dt;
        double tB = t0 + (interval + 1) * dt;
        for (int i = 0; i < numEval; i++)
            {
            double t, w;
            GetEval (i, tA, tB, t, w);
            function.EvaluateVectorIntegrand (t, pF);
            for (int k = 0; k < nFunc; k++)
                pSums[k] +=  w * pF[k];
            }
        }
    }

struct BSIIntegrationStack
{
static const uint32_t s_maxStackDepth = 10;
static const uint32_t s_maxIntegrand = 20;
double m_stack[s_maxStackDepth][s_maxIntegrand];
uint32_t m_numIntegrand;
uint32_t m_stackDepth;
BSIIntegrationStack (uint32_t numIntegrand) : m_numIntegrand (numIntegrand), m_stackDepth (0){}

double *TopOfStackAddress ()
    {
    if (m_stackDepth == 0)
        return nullptr;
    return m_stack[m_stackDepth - 1];
    }

void PushZeroFrame ()
    {
    if (m_stackDepth >= s_maxStackDepth)
        return;
    for (uint32_t i = 0; i < m_numIntegrand; i++)
        m_stack[m_stackDepth][i] = 0.0;
    m_stackDepth++;
    }

void AccumulateToNewFrame
(
BSIQuadraturePoints &gauss,
BSIVectorIntegrand &function,
double t0,
double t1,
int     numInterval
)
    {
    PushZeroFrame ();
    uint32_t stackIndex = m_stackDepth - 1;
    gauss.AccumulateWeightedSums (function, t0, t1, m_stack[stackIndex], numInterval);
    }

// Add weight*data to top of stack.
void AccumulateToTopOfStack (double *data, double weight)
    {
    if (m_stackDepth == 0)
        return;
    uint32_t stackIndex = m_stackDepth - 1;
    for (uint32_t k = 0; k < m_numIntegrand; k++)
        m_stack[stackIndex][k] += weight * data[k];
    }
// Return the max difference between the two frames at the top of the stack.
double MaxDiff ()
    {
    if (m_stackDepth < 2)
        return DBL_MAX;
    uint32_t stackIndex0 = m_stackDepth - 1;
    uint32_t stackIndex1 = m_stackDepth - 2;
    double dMax = 0.0;
    for (uint32_t k = 0; k < m_numIntegrand; k++)
        {
        double d = fabs (m_stack[stackIndex0][k] - m_stack[stackIndex1][k]);
        if (d > dMax)
            dMax = d;
        }
    return dMax;
    }
// Add top frame to next.
// Pop top
void AddAndPop ()
    {
    if (m_stackDepth < 2)
        return;
    uint32_t stackIndex0 = m_stackDepth - 1;
    uint32_t stackIndex1 = m_stackDepth - 2;
    for (uint32_t k = 0; k < m_numIntegrand; k++)
        m_stack[stackIndex1][k] += m_stack[stackIndex0][k];
    m_stackDepth--;
    }

// replace b by a - factor*(b-a), where a is top of stack and b is one below.
// i.e. a = top of stack is "better" estimate
//      b = next entry is old estimate.
//      factor is the richardson factor for combining
void SubtractFramesRombergDifference (double factor)
    {
    if (m_stackDepth < 2)
        return;
    uint32_t stackIndex0 = m_stackDepth - 1;
    uint32_t stackIndex1 = m_stackDepth - 2;
    for (uint32_t k = 0; k < m_numIntegrand; k++)
        {
        double a = m_stack[stackIndex0][k];
        double b = m_stack[stackIndex1][k];
        m_stack[stackIndex1][k] = a - factor * (b-a);
        }
    m_stackDepth--;
    }

void GetTopOfStack (double *data)
    {
    if (m_stackDepth == 0)
        return;
    uint32_t stackIndex = m_stackDepth - 1;
    for (uint32_t k = 0; k < m_numIntegrand; k++)
        data[k] = m_stack[stackIndex][k];
    }
// pop the top stack frame.
void PopFrame ()
    {
    if (m_stackDepth > 0)
        m_stackDepth--;
    }

};

/*----------------------------------------------------------------*//**
@description Evaluate and accumulate function values over an interval.
@param [in] function function object that can be called as often as needed.
@param [in t0 start of interval.
@param [in] t1 end of interval.
@params [in] numInterval number of intervals to use within t0..t1.
@params [out] totalErrorBound 
@return false if function.AnnounceIntermediateIntergral () returned false.
 @bsimethod                                  EarlinLutz      10/07
+---------------+---------------+---------------+---------------+------*/
bool BSIQuadraturePoints::IntegrateWithRombergExtrapolation
(
BSIIncrementalVectorIntegrand &function,
double t0,
double t1,
uint32_t numInterval,
double &totalErrorBound
)
    {
    double u0 = t0, u1;
    if (numInterval < 1)
        numInterval = 1;
    double du = (t1 - t0) / numInterval;
    BSIIntegrationStack stack ((uint32_t)function.GetVectorIntegrandCount ());
    stack.PushZeroFrame ();
    if (!function.AnnounceIntermediateIntegral (t0, stack.TopOfStackAddress ()))
        return false;
    totalErrorBound = 0.0;
    double rombergFactor = 1.0 / (pow (2.0, GetConvergencePower ()) - 1.0);
    for (uint32_t interval = 0; interval < numInterval; interval++, u0 = u1)
        {
        if (interval + 1 == numInterval)
            u1 = t1;
        else
            u1 = t0 + (interval + 1) * du;
        stack.AccumulateToNewFrame (*this, function, u0, u1, 1);
        stack.AccumulateToNewFrame (*this, function, u0, u1, 2);
        double dMax = stack.MaxDiff ();
        totalErrorBound += dMax * rombergFactor;
        stack.SubtractFramesRombergDifference (rombergFactor);
        stack.AddAndPop ();
        if (!function.AnnounceIntermediateIntegral (u1, stack.TopOfStackAddress ()))
            return false;
        }
    return true;
    }

/*-----------------------------------------------------------------*//**
constructor -- initialize to Simpson's rule.
 @bsimethod                                  EarlinLutz      10/07
+---------------+---------------+---------------+---------------+------*/
BSIQuadraturePoints::BSIQuadraturePoints ()
    {
    InitUniform (3);
    }
// Gauss Kronrod weights from http://people.sc.fsu.edu/~jburkardt/m_src/kronrod/kronrod_test01.m
// See also http://people.sc.fsu.edu/~jburkardt/m_src/kronrod/kronrod.html
// Lots of datasets  http://people.sc.fsu.edu/~jburkardt/datasets/datasets.html

static double wk7[7] = {
    0.104656226026467265194, 0.268488089868333440729,
    0.401397414775962222905, 0.450916538658474142345,
    0.401397414775962222905, 0.268488089868333440729,
    0.104656226026467265194
    };

static double xk7[7] = {
    -0.96049126870802028342, -0.77459666924148337704, -0.43424374934680255800,
    0.0,
    0.43424374934680255800, 0.77459666924148337704, 0.96049126870802028342
    };

void AccumulateGaussKronrod
(
double *kronrodX,       // Kronrod points.
double *kronrodWeights,  // Kronrod weights
double *gaussWeights,    // Gauss weights.  These correspond to the ODD indices in the Kronrod points.
int numKronrod,
BSIVectorIntegrand &function,
double t0,
double t1,
double *pGaussSums,
double *pKronrodSums
)
    {
    double tm = 0.5 * (t0 + t1);
    double dt = 0.5 * (t1 - t0);
    int nFunc = function.GetVectorIntegrandCount ();
    if (nFunc < 1)
        return;
    double *pF = (double*)_alloca (nFunc * sizeof (double));
    for (int i = 0, m = 0; i < 7; i++)
        {
        double xi = tm + dt * kronrodX[i];
        function.EvaluateVectorIntegrand (xi, pF);
        // Kronrod sum receives all points ...
        double kronrodWeight = dt * kronrodWeights[i];
        for (int j = 0; j < nFunc; j++)
            {
            pKronrodSums[j] += kronrodWeight * pF[j];
            }
        // Gauss sum only gets odd points
        if (i & 0x01)
            {
            double gaussWeight = dt * gaussWeights[m];
            for (int j = 0; j < nFunc; j++)
                {
                pGaussSums[j] += gaussWeight * pF[j];
                }
            m++;
            }
        }
    }

/*-----------------------------------------------------------------*//**
@description Setup for a "Kronrod" part of Gauss-Kronrad quadrature.
@param numEval IN requested number of points. ONLY 7 IS SUPPORTED
<returns>Number of points actually to be used. </returns>
 @bsimethod                                  EarlinLutz      10/07
+---------------+---------------+---------------+---------------+------*/
int BSIQuadraturePoints::InitGaussKronrod (int numEval, BSIQuadraturePoints &gaussPartner)
    {
    Init (-1.0, 1.0);
    numEval = 7;
    mConvergencePower = 12.0;
    for (int i = 0; i < numEval; i++)
        {
        AddEval (xk7[i], wk7[i]);
        }
    gaussPartner.InitGauss (3);
    return numEval;
    }


BSITriangleQuadraturePoints::BSITriangleQuadraturePoints ()
    {
    Init ();
    double b = 1.0 / 3.0;
    AddEval (b,b,1.0);
    }

void BSITriangleQuadraturePoints::Init ()
    {
    mNumEval = 0;
    }
    
int BSITriangleQuadraturePoints::GetNumEval () const
    {
    return mNumEval;
    }

bool BSITriangleQuadraturePoints::AddEval (double u, double v, double w)
    {
    if (mNumEval < MaxPoints)
        {
        mUi[mNumEval] = u;
        mVi[mNumEval] = v;
        mWi[mNumEval] = w;
        mNumEval++;
        }
    return false;
    }

static double s_strang4W[] = {
    0.16666666666666666667,
    0.16666666666666666667,
    0.16666666666666666667,
    0.16666666666666666667,
    0.16666666666666666667,
    0.16666666666666666667
    };
static double s_strang4XY[][2] = {
    {0.659027622374092,  0.231933368553031},
    {0.659027622374092,  0.109039009072877},
    {0.231933368553031,  0.659027622374092},
    {0.231933368553031,  0.109039009072877},
    {0.109039009072877,  0.659027622374092},
    {0.109039009072877,  0.231933368553031}
    };


static double s_strang5W[] = {
    0.109951743655322,
    0.109951743655322,
    0.109951743655322,
    0.223381589678011,
    0.223381589678011,
    0.223381589678011
    };
static double s_strang5XY[][2] = {
    {0.816847572980459,  0.091576213509771},
    {0.091576213509771,  0.816847572980459},
    {0.091576213509771,  0.091576213509771},
    {0.108103018168070,  0.445948490915965},
    {0.445948490915965,  0.108103018168070},
    {0.445948490915965,  0.445948490915965}
    };

static double s_strang6W[] = {
    0.375000000000000,
    0.104166666666667,
    0.104166666666667,
    0.104166666666667,
    0.104166666666667,
    0.104166666666667,
    0.104166666666667
    };
static double s_strang6XY[][2] = {
    {0.333333333333333,  0.333333333333333},
    {0.736712498968435,  0.237932366472434},
    {0.736712498968435,  0.025355134551932},
    {0.237932366472434,  0.736712498968435},
    {0.237932366472434,  0.025355134551932},
    {0.025355134551932,  0.736712498968435},
    {0.025355134551932,  0.237932366472434}
    };

static double s_strang7W[] = {
    0.22500000000000000,
    0.12593918054482717,
    0.12593918054482717,
    0.12593918054482717,
    0.13239415278850616,
    0.13239415278850616,
    0.13239415278850616
    };
static double s_strang7XY[][2] = {
    {0.33333333333333333,  0.33333333333333333},
    {0.79742698535308720,  0.10128650732345633},
    {0.10128650732345633,  0.79742698535308720},
    {0.10128650732345633,  0.10128650732345633},
    {0.05971587178976981,  0.47014206410511505},
    {0.47014206410511505,  0.05971587178976981},
    {0.47014206410511505,  0.47014206410511505}
    };
    
static double s_strang8W[] = {
    0.205950504760887,
    0.205950504760887,
    0.205950504760887,
    0.063691414286223,
    0.063691414286223,
    0.063691414286223,
    0.063691414286223,
    0.063691414286223,
    0.063691414286223
    };
static double s_strang8XY[][2] = {
    {0.124949503233232,  0.437525248383384},
    {0.437525248383384,  0.124949503233232},
    {0.437525248383384,  0.437525248383384},
    {0.797112651860071,  0.165409927389841},
    {0.797112651860071,  0.037477420750088},
    {0.165409927389841,  0.797112651860071},
    {0.165409927389841,  0.037477420750088},
    {0.037477420750088,  0.797112651860071},
    {0.037477420750088,  0.165409927389841}

    };
    
static double s_strang9W[] = {
    0.050844906370207,
    0.050844906370207,
    0.050844906370207,
    0.116786275726379,
    0.116786275726379,
    0.116786275726379,
    0.082851075618374,
    0.082851075618374,
    0.082851075618374,
    0.082851075618374,
    0.082851075618374,
    0.082851075618374
    };
static double s_strang9XY[][2] = {
    {0.873821971016996,  0.063089014491502},
    {0.063089014491502,  0.873821971016996},
    {0.063089014491502,  0.063089014491502},
    {0.501426509658179,  0.249286745170910},
    {0.249286745170910,  0.501426509658179},
    {0.249286745170910,  0.249286745170910},
    {0.636502499121399,  0.310352451033785},
    {0.636502499121399,  0.053145049844816},
    {0.310352451033785,  0.636502499121399},
    {0.310352451033785,  0.053145049844816},
    {0.053145049844816,  0.636502499121399},
    {0.053145049844816,  0.310352451033785}
    };
    
static double s_strang10W[] = {
    -0.149570044467670,
    0.175615257433204,
    0.175615257433204,
    0.175615257433204,
    0.053347235608839,
    0.053347235608839,
    0.053347235608839,
    0.077113760890257,
    0.077113760890257,
    0.077113760890257,
    0.077113760890257,
    0.077113760890257,
    0.077113760890257
    };
static double s_strang10XY[][2] = {
    {0.333333333333333,  0.333333333333333},
    {0.479308067841923,  0.260345966079038},
    {0.260345966079038,  0.479308067841923},
    {0.260345966079038,  0.260345966079038},
    {0.869739794195568,  0.065130102902216},
    {0.065130102902216,  0.869739794195568},
    {0.065130102902216,  0.065130102902216},
    {0.638444188569809,  0.312865496004875},
    {0.638444188569809,  0.048690315425316},
    {0.312865496004875,  0.638444188569809},
    {0.312865496004875,  0.048690315425316},
    {0.048690315425316,  0.638444188569809},
    {0.048690315425316,  0.312865496004875}
    };

int BSITriangleQuadraturePoints::InitStrang (int selector)
    {
    Init ();
    if (selector == 1)
        {
        double a = 2.0 / 3.0;
        double b = 1.0 / 6.0;
        double w = 1.0 / 3.0;
        AddEval (a, b, 0.5 * w);
        AddEval (b, a, 0.5 * w);
        AddEval (b, b, 0.5 * w);
        mConvergencePower = 2.0;
        }
    else if (selector == 2)
        {
        double a = 0.5;
        double w = 1.0 / 3.0;
        AddEval (a, 0.0, 0.5 * w);
        AddEval (a, a, 0.5 * w);
        AddEval (0.0, a, 0.5 * w);
        mConvergencePower = 2.0;   
        }
    else if (selector == 3)
        {
        double a = 0.6;
        double b = 1.0 / 3.0;
        double c = 0.2;
        double w0 = - 9.0 / 16.0;
        double w1 = 25.0 / 48.0;
        AddEval (b, b, 0.5 * w0);
        AddEval (a, c, 0.5 * w1);
        AddEval (c, a, 0.5 * w1);
        AddEval (c, c, 0.5 * w1);
        mConvergencePower = 2.0;
        }
    else if (selector == 4)
        {
        for (int i = 0; i < 6; i++)
            AddEval (s_strang4XY[i][0], s_strang4XY[i][1], 0.5 * s_strang4W[i]);
        mConvergencePower = 2.0;
        }
   else if (selector == 5)
        {
        for (int i = 0; i < 6; i++)
            AddEval (s_strang5XY[i][0], s_strang5XY[i][1], 0.5 * s_strang5W[i]);
        mConvergencePower = 2.0;
        }
    else if (selector == 6)
        {
        for (int i = 0; i < 7; i++)
            AddEval (s_strang6XY[i][0], s_strang6XY[i][1], 0.5 * s_strang6W[i]);
        mConvergencePower = 2.0;
        }
     else if (selector == 7)
        {
        for (int i = 0; i < 7; i++)
            AddEval (s_strang7XY[i][0], s_strang7XY[i][1], 0.5 * s_strang7W[i]);
        mConvergencePower = 2.0;
        }
     else if (selector == 8)
        {
        for (int i = 0; i < 9; i++)
            AddEval (s_strang8XY[i][0], s_strang8XY[i][1], 0.5 * s_strang8W[i]);
        mConvergencePower = 2.0;
        }
     else if (selector == 9)
        {
        for (int i = 0; i < 12; i++)
            AddEval (s_strang9XY[i][0], s_strang9XY[i][1], 0.5 * s_strang9W[i]);
        mConvergencePower = 2.0;
        }    
     else if (selector == 10)
        {
        for (int i = 0; i < 13; i++)
            AddEval (s_strang10XY[i][0], s_strang10XY[i][1], 0.5 * s_strang10W[i]);
        mConvergencePower = 2.0;
        }
    return mNumEval;
    }

void BSITriangleQuadraturePoints::AccumulateWeightedSums(BSIVectorIntegrandXY &function, double *pSums)
    {
    int nFunc = function.GetVectorIntegrandCount ();
    if (nFunc < 1)
        return;
    double *pF = (double*)_alloca (nFunc * sizeof (double));
    int numEval = GetNumEval ();

    for (int i = 0; i < numEval; i++)
        {
        function.EvaluateVectorIntegrand (mUi[i], mVi[i], pF);
        for (int k = 0; k < nFunc; k++)
            pSums[k] +=  mWi[i] * pF[k];
        }
    }
    
void BSITriangleQuadraturePoints::AccumulateWeightedSumsMapped
    (
    BSIVectorIntegrandXY &function, double *pSums, 
    double ax, double ay,
    double bx, double by,
    double cx, double cy
    )
    {
    int nFunc = function.GetVectorIntegrandCount ();
    if (nFunc < 1)
        return;
    double *pF = (double*)_alloca (nFunc * sizeof (double));
    int numEval = GetNumEval ();
    double Ux = bx - ax;
    double Uy = by - ay;
    double Vx = cx - ax;
    double Vy = cy - ay;
    double cons = (Ux * Vy - Uy * Vx);

    for (int i = 0; i < numEval; i++)
        {
        function.EvaluateVectorIntegrand (mUi[i]*Ux + mVi[i]*Vx + ax, mUi[i]*Uy + mVi[i]*Vy + ay, pF);
        for (int k = 0; k < nFunc; k++)
            pSums[k] +=  cons * mWi[i] * pF[k];
        }
    }


bool BSITriangleQuadraturePoints::GetEval (int i, double &u, double &v, double &w) const
    {
    if (0 <= i && i < mNumEval)
        {
        u = mUi[i];
        v = mVi[i];
        w = mWi[i];
        return true;
        }
    else
        {
        u = v = w = 0.0;
        return false;
        }
    }

END_BENTLEY_GEOMETRY_NAMESPACE
