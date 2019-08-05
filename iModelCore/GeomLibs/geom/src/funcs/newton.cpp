/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
//////////////////////////////////////////////////////////////////////////////////
//
// NewtonIterationsRRToRR -- Newton Raphson iterations for 2 functions of 2 varaibles
//
// Author: Earlin Lutz   April 2007
//
//
///////////////////////////////////////////////////////////////////////////////////
#include <bsibasegeomPCH.h>
#include    <math.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

#ifdef printDebug
static int sDebug = 0;
#endif
/*-----------------------------------------------------------------*//**
// Detailed setup with same tolerances for both vars ...
 @bsimethod                                  EarlinLutz      04/07
+---------------+---------------+---------------+---------------+------*/
NewtonIterationsRRToRR::NewtonIterationsRRToRR
(
double abstol,
double reltol,
int    maxIterations,
int    successiveConvergenceTarget,
double minStep
)
    {
    if (abstol < 0.0)
        abstol = 0.0;
    if (reltol < 0.0)
        reltol = 0.0;
    if (reltol < 1.0e-12)
        reltol = 1.0e-12;
    mAbstolU = mAbstolV = mAbstolW = abstol;
    mReltolU = mReltolV = mReltolW = reltol;
    mMaxIterations = maxIterations;
    mSuccessiveConvergenceTarget = successiveConvergenceTarget;
    mSoftAbstolU = mSoftAbstolV = -1.0;
    mMinStepU = mMinStepV = minStep;
    }
void NewtonIterationsRRToRR::SetSoftTolerance (double abstolU, double abstolV)
    {
    mSoftAbstolU = abstolU;
    mSoftAbstolV = abstolV;
    }
/*-----------------------------------------------------------------*//**
 @bsimethod                                  EarlinLutz      04/07
+---------------+---------------+---------------+---------------+------*/
bool NewtonIterationsRRToRR::CheckIterationStart
(
double &u,
double &v,
int numIterations
)
    {
#ifdef printDebug
    if (sDebug)
        printf ("Start iteration %d at %lg,%lg\n",
                    numIterations, u, v);
#endif
    return numIterations < mMaxIterations;
    }

/*-----------------------------------------------------------------*//**
 @bsimethod                                  EarlinLutz      04/07
+---------------+---------------+---------------+---------------+------*/
bool NewtonIterationsRRToRR::CheckIterationStart
(
DVec3dCR uvw,
int numIterations
)
    {
    return numIterations < mMaxIterations;
    }



/*-----------------------------------------------------------------*//**
// Convergence tester.
// Returns true if FULLY CONVERGED.
// Returns false if any convergence condition is not satisfied.
// Note that the numConverged parameter may be either set to zero or incremented by
//   this test.
// @param u IN first  variable.
// @param v IN second variable.
// @param f IN first function value.
// @param g IN second function value.
// @param du IN proposed update to u
// @param dv IN proposed update to v
// @param numConverged INOUT evolving counter of successive steps that have converged.
 @bsimethod                                  EarlinLutz      04/07
+---------------+---------------+---------------+---------------+------*/
bool NewtonIterationsRRToRR::CheckConvergence
(
double u,
double v,
double f,
double g,
double &du,
double &dv,
int &numConverged
)
    {
#ifdef printDebug
    if (sDebug)
        printf ("   du,dv=%lg,%lg    f,g=%lg,%lg\n", du, dv, f, g);
#endif
    if (    fabs (du) < mAbstolU + fabs (mReltolU * u)
        &&  fabs (dv) < mAbstolV + fabs (mReltolV * v))
        {
        numConverged++;
        return numConverged >= mSuccessiveConvergenceTarget;
        }
    numConverged = 0;
    return false;
    }

/*-----------------------------------------------------------------*//**
// Convergence tester.
// Returns true if FULLY CONVERGED.
// Returns false if any convergence condition is not satisfied.
// Note that the numConverged parameter may be either set to zero or incremented by
//   this test.
// @param u IN first  variable.
// @param v IN second variable.
// @param f IN first function value.
// @param g IN second function value.
// @param du IN proposed update to u
// @param dv IN proposed update to v
// @param numConverged INOUT evolving counter of successive steps that have converged.
 @bsimethod                                  EarlinLutz      04/07
+---------------+---------------+---------------+---------------+------*/
bool NewtonIterationsRRToRR::CheckConvergence
(
DVec3dCR U,
DVec3dCR F,
DVec3dCR dU,
int &numConverged
)
    {
#ifdef printDebug
    if (sDebug)
        printf ("   du,dv,dw=%lg,%lg,%lg    f,g,h=%lg,%lg,%lg\n", dU.x, dU.y, dU.z, F.x, F.y, F.z);
#endif
    if (    fabs (dU.x) < mAbstolU + fabs (mReltolU * U.x)
        &&  fabs (dU.y) < mAbstolV + fabs (mReltolV * U.y)
        &&  fabs (dU.z) < mAbstolW + fabs (mReltolW * U.z))
        {
        numConverged++;
        return numConverged >= mSuccessiveConvergenceTarget;
        }
    numConverged = 0;
    return false;
    }

    
/*-----------------------------------------------------------------*//**
 @bsimethod                                  EarlinLutz      10/07
Compute a single Newton step given function and derivatives.
return false if unable to invert the matrix.
+---------------+---------------+---------------+---------------+------*/
static bool ComputeNewtonStep
(
double f,
double g,
double dfdu,
double dfdv,
double dgdu,
double dgdv,
double &du,
double &dv
)
    {
    double ad = dfdu * dgdv;
    double bc = dfdv * dgdu;
    double det = ad - bc;
    double perm = fabs (ad) + fabs (bc);
    double au  = f * dgdv - g * dfdv;
    double av  = dfdu * g - f * dgdu;
    double hf   = DoubleOps::Hypotenuse (dfdu, dfdv);
    double hg   = DoubleOps::Hypotenuse (dgdu, dgdv);
    static double s_rowFractionTolerance = 1.0e-10;
    // fail if one row is vastly larger than the other ...
    if (hf <= s_rowFractionTolerance * hg
        || hg <= s_rowFractionTolerance * hf)
        return false;
    double determinantTest;
    if (!DoubleOps::SafeDivide (determinantTest, perm, det, 0.0))
        return false;
    if (!DoubleOps::SafeDivide (du, au, det, 0.0))
        return false;
    if (!DoubleOps::SafeDivide (dv, av, det, 0.0))
        return false;
    return true;
    }

/*-----------------------------------------------------------------*//**
 @bsimethod                                  EarlinLutz      10/07
Compute a single Newton step given function and derivatives.
return false if unable to invert the matrix.
+---------------+---------------+---------------+---------------+------*/
static bool ComputeNewtonStep
(
DVec3dCR F,
RotMatrixCR J,
DVec3dR dU
)
    {
    return J.Solve (dU, F);
    }


static bool SetReturnValues (double &uu, double &vv, double u, double v, double f0, double g0, double f, double g)
    {
    double df = fabs (f) - fabs (f0);
    double dg = fabs (g) - fabs (g0);
    if (df <= 0.0 && dg <= 0.0)
        {
        uu = u;
        vv = v;
        return true;
        }
    return true;
    }

static bool SetReturnValues (double &uu, double u, double f0, double f)
    {
    double df = fabs (f) - fabs (f0);
    if (df <= 0.0)
        {
        uu = u;
        return true;
        }
    return true;
    }


// when initial function values are really small, a "good" iteration might affect f0 only in roundoff that goes the wrong direction.
//  The FuzzyFunctionValue is f0 with absolute shift by small du and dv applied to the respective partial derivatives.
static double FuzzyFunctionValue (double f0, double dfdu, double du, double dfdv, double dv)
    {
    return fabs (f0) + fabs (dfdu * du) + fabs (dfdv * dv);
    }
static double FuzzyFunctionValue (double f0, double dfdu, double du)
    {
    return fabs (f0) + fabs (dfdu * du);
    }

bool NewtonIterationsRRToRR::RunDiagonalNewton
(
double &uu,
double &vv,
FunctionRRToRRD &evaluator
)
    {
    int numConverged = 0;
    double u = uu;
    double v = vv;
    double f=0.0, g=0.0, dfdu, dfdv, dgdu, dgdv, du, dv;
    double f0 = -1.0, g0 = -1.0;

    for (int numIterations = 0; CheckIterationStart (u, v, numIterations); numIterations++)
        {
        if (!evaluator.EvaluateRRToRRD (u, v, f, g, dfdu, dfdv, dgdu, dgdv))
            return false;
        if (numIterations == 0)
            {
            f0 = f;
            g0 = g;
            }
        bool uOK = DoubleOps::SafeDivide (du, f, dfdu, 0.0);
        bool vOK = DoubleOps::SafeDivide (dv, g, dgdv, 0.0);
        if (!uOK && !vOK)
            {
            return SetReturnValues (uu, vv, u, v, f0, g0, f, g);
            }
        // Heuristically, a big diagonal indicates a safe step.
        // So let each diagonal indicate a reduction factor on the respective steps:
        double uFactor, vFactor;
        double a = DoubleOps::Hypotenuse (dfdu, dgdv);
        DoubleOps::SafeDivide (uFactor, fabs (dfdu), a, 0.0);
        DoubleOps::SafeDivide (vFactor, fabs (dgdv), a, 0.0);

        u -= uFactor * du;
        v -= vFactor * dv;
        if (CheckConvergence (u, v, f, g, du, dv, numConverged))
            {
            return SetReturnValues (uu, vv, u, v,
                  FuzzyFunctionValue (f0, dfdu, mAbstolU, dfdv, mAbstolV),
                  FuzzyFunctionValue (g0, dgdu, mAbstolU, dgdv, mAbstolV),
                  f, g);
            }
        }
    return SetReturnValues (uu, vv, u, v, f0, g0, f, g);
    }



/*-----------------------------------------------------------------*//**
 @bsimethod                                  EarlinLutz      04/07
// Run Newton stesp for a function represented by a Newton2BivariateEvaluator
// @param u INOUT first  variable.
// @param v INOUT second variable.
// @param evaluator IN evaluator object.
+---------------+---------------+---------------+---------------+------*/
bool NewtonIterationsRRToRR::RunNewton
(
double &uu,
double &vv,
FunctionRRToRRD &evaluator
)
    {
    int numConverged = 0;
    double u = uu;
    double v = vv;
    double f=0.0, g=0.0, dfdu, dfdv, dgdu, dgdv, du, dv;
    double du0 = DBL_MAX;
    double dv0 = DBL_MAX;
    double f0=0.0, g0=0.0;
    static double s_smallDeltaForFunctionValueTest = 1.0e-14;
    for (int numIterations = 0; CheckIterationStart (u, v, numIterations); numIterations++)
        {
        if (!evaluator.EvaluateRRToRRD (u, v, f, g, dfdu, dfdv, dgdu, dgdv))
            return false;
        if (numIterations == 0)
            {
            f0 = f;
            g0 = g;
            }
        if (!ComputeNewtonStep(f, g, dfdu, dfdv, dgdu, dgdv, du, dv))
            {
            return RunDiagonalNewton (uu, vv, evaluator);
            }
        u -= du;
        v -= dv;
        du0 = du;
        dv0 = dv;
        if (CheckConvergence (u, v, f, g, du, dv, numConverged))
            {
            return SetReturnValues (uu, vv, u, v,
                  FuzzyFunctionValue (f0, dfdu, s_smallDeltaForFunctionValueTest, dfdv, s_smallDeltaForFunctionValueTest),
                  FuzzyFunctionValue (g0, dgdu, s_smallDeltaForFunctionValueTest, dgdv, s_smallDeltaForFunctionValueTest),
                  f, g);
            }
        }
    return SetReturnValues (uu, vv, u, v, f0, g0, f, g);
    }

/*-----------------------------------------------------------------*//**
 @bsimethod                                  EarlinLutz      04/07
// Run Newton steps for a univariate function.
// @param u INOUT variable.
// @param evaluator IN evaluator object.
+---------------+---------------+---------------+---------------+------*/
bool NewtonIterationsRRToRR::RunNewton
(
double &uu,
FunctionRToRD &evaluator
)
    {
    int numConverged = 0;
    double u = uu;
    double f=0.0, dfdu;
    double du0 = DBL_MAX;
    double f0=0.0;
    static double s_smallDeltaForFunctionValueTest = 1.0e-14;
    for (int numIterations = 0; CheckIterationStart (u, u, numIterations); numIterations++)
        {
        if (!evaluator.EvaluateRToRD (u, f, dfdu))
            return false;
        if (numIterations == 0)
            {
            f0 = f;
            }
        auto du = DoubleOps::ValidatedDivide (f, dfdu);
        if (!du.IsValid ())
            return false;
        u -= du;
        du0 = du;
        if (CheckConvergence (u, u, f, f, du.Value (), du.Value (), numConverged))
            {
            return SetReturnValues (uu, u,
                  FuzzyFunctionValue (f0, dfdu, s_smallDeltaForFunctionValueTest),
                  f);
            }
        }
    return SetReturnValues (uu, u, f0, f);
    }

/*-----------------------------------------------------------------*//**
Compute a step size for approximate derivatives, using
min and max relative size and min absolute size.
 @bsimethod                                  EarlinLutz      04/07
+---------------+---------------+---------------+---------------+------*/
static double ComputeDerivativeStep
(
double u,
double relStep,
double minStep = 0
)
    {
    static double sMaxRelStep = 1.0e-6;
    static double sMinRelStep = 1.0e-14;
    static double sMinStep    = 1.0e-12;
    static double sDefaultRelStep = 1.0e-8;
    if (relStep == 0.0)
        relStep = sDefaultRelStep;
    if (relStep < sMinRelStep)
        relStep = sMinRelStep;
    if (relStep > sMaxRelStep)
        relStep = sMaxRelStep;
    double du = relStep * u;
    if (fabs (du) < sMinStep)
        du = sMinStep;
    if (fabs (du) < minStep)
        du = minStep;
    return du;
    }

/*-----------------------------------------------------------------*//**
Evaluate an RRToRR function and approximate its derivatives.
 @bsimethod                                  EarlinLutz      04/07
+---------------+---------------+---------------+---------------+------*/
static bool ApproximateDerivatives
(
FunctionRRToRR &evaluator,
double u,
double v,
double &f,
double &g,
double &dfdu,
double &dfdv,
double &dgdu,
double &dgdv,
double minStepU,
double minStepV
)
    {
    double du = ComputeDerivativeStep (u, 0.0, minStepU);
    double dv = ComputeDerivativeStep (v, 0.0, minStepV);
    double fu, gu, fv, gv;
    double u1 = u + du;
    double v1 = v + dv;
    if (   !evaluator.EvaluateRRToRR (u, v, f, g)
        || !evaluator.EvaluateRRToRR (u1, v, fu, gu)
        || !evaluator.EvaluateRRToRR (u, v1, fv, gv))
        return false;

    if (   !DoubleOps::SafeDivide (dfdu, fu - f, du, 0.0)
        || !DoubleOps::SafeDivide (dgdu, gu - g, du, 0.0)
        || !DoubleOps::SafeDivide (dfdv, fv - f, dv, 0.0)
        || !DoubleOps::SafeDivide (dgdv, gv - g, dv, 0.0))
        return false;

    return true;
    }

/*-----------------------------------------------------------------*//**
Evaluate an RRToRR function and approximate its derivatives.
 @bsimethod                                  EarlinLutz      04/07
+---------------+---------------+---------------+---------------+------*/
static bool ApproximateDerivatives
(
FunctionRRRToRRR &evaluator,
DVec3dCR U,
DVec3dR F,
RotMatrixR J
)
    {
    double du = ComputeDerivativeStep (U.x, 0.0);
    double dv = ComputeDerivativeStep (U.y, 0.0);
    double dw = ComputeDerivativeStep (U.z, 0.0);
    DVec3d Fu, Fv, Fw;
    DVec3d Uu, Uv, Uw;
    Uu = U; Uu.x += du;
    Uv = U; Uv.y += dv;
    Uw = U; Uw.z += dw;
    if (   !evaluator.EvaluateRRRToRRR (U, F)
        || !evaluator.EvaluateRRRToRRR (Uu, Fu)
        || !evaluator.EvaluateRRRToRRR (Uv, Fv)
        || !evaluator.EvaluateRRRToRRR (Uw, Fw))
        return false;
    DVec3d Ju, Jv, Jw;

    if (  Ju.SafeDivide (Fu-F, du)
       && Jv.SafeDivide (Fv-F, dv)
       && Jw.SafeDivide (Fw-F, dw)
       )
        {
        J = RotMatrix::FromColumnVectors(Ju, Jv, Jw);
        return true;
        }
    return false;
    }




/*-----------------------------------------------------------------*//**
 @bsimethod                                  EarlinLutz      04/07
// Run Newton steps
// @param u INOUT first  variable.
// @param v INOUT second variable.
// @param evaluator IN evaluator object.
+---------------+---------------+---------------+---------------+------*/
bool NewtonIterationsRRToRR::RunApproximateNewton
(
double &uu,
double &vv,
FunctionRRToRR &evaluator,
double maxdu,
double maxdv
)
    {
    int numConverged = 0;
    double u = uu;
    double v = vv;
    double f, g, dfdu, dfdv, dgdu, dgdv, du, dv;
    for (int numIterations = 0; CheckIterationStart (u, v, numIterations); numIterations++)
        {
        if (!ApproximateDerivatives (evaluator, u, v, f, g, dfdu, dfdv, dgdu, dgdv, mMinStepU, mMinStepV))
            return false;
        if (!ComputeNewtonStep(f, g, dfdu, dfdv, dgdu, dgdv, du, dv))
            return false;
        if (fabs (du) > maxdu)
            du = du > 0.0 ? maxdu : -maxdu;
        if (fabs (dv) > maxdv)
            dv = dv > 0.0 ? maxdv : -maxdv;
        u -= du;
        v -= dv;
        if (CheckConvergence (u, v, f, g, du, dv, numConverged))
            {
            uu = u;
            vv = v;
            return true;
            }
        }
    return false;
    }


/*-----------------------------------------------------------------*//**
 @bsimethod                                  EarlinLutz      04/07
// Run Newton steps
// @param u INOUT first  variable.
// @param v INOUT second variable.
// @param evaluator IN evaluator object.
+---------------+---------------+---------------+---------------+------*/
bool NewtonIterationsRRToRR::RunApproximateNewton
(
DVec3dR uvw,
FunctionRRRToRRR &evaluator,
DVec3dCR maxDuvw
)
    {
    int numConverged = 0;
    DVec3d U = uvw;
    DVec3d F;
    RotMatrix J;
    DVec3d dU;
#define MAX_DU_SAVE 10
    DVec3d duSave[MAX_DU_SAVE]; // save them up to observe in debugger
    for (int numIterations = 0; CheckIterationStart (U, numIterations); numIterations++)
        {
        if (!ApproximateDerivatives (evaluator, U, F, J))
            return false;
        if (!ComputeNewtonStep(F, J, dU))
            return false;
        dU.x = DoubleOps::Clamp (dU.x, -maxDuvw.x, maxDuvw.x);
        dU.y = DoubleOps::Clamp (dU.y, -maxDuvw.y, maxDuvw.y);
        dU.z = DoubleOps::Clamp (dU.z, -maxDuvw.z, maxDuvw.z);
        duSave[numIterations % MAX_DU_SAVE] = dU;
        U = U - dU;
        if (CheckConvergence (U, F, dU, numConverged))
            {
            uvw = U;
            return true;
            }
        }
    return false;
    }



class Newton2DifferenceEvaluator : public FunctionRRToRRD
{
public:
    FunctionRToRRD &mEvaluatorA;
    FunctionRToRRD &mEvaluatorB;

/*-----------------------------------------------------------------*//**
 @bsimethod                                  EarlinLutz      04/07
+---------------+---------------+---------------+---------------+------*/
Newton2DifferenceEvaluator
(
FunctionRToRRD &evaluatorA,
FunctionRToRRD &evaluatorB
) : mEvaluatorA (evaluatorA),
    mEvaluatorB(evaluatorB)
    {
    }

/*-----------------------------------------------------------------*//**
// Virtual function implementation
// @param u IN first variable
// @param v IN second variable
// @param f OUT first function value
// @param g OUT second function value
// @param dfdu OUT derivative of f wrt u
// @param dfdv OUT derivative of f wrt v
// @param dgdu OUT derivative of g wrt u
// @param dgdv OUT derivative of g wrt v
// @return true if function was evaluated.
 @bsimethod                                  EarlinLutz      04/07
+---------------+---------------+---------------+---------------+------*/
bool EvaluateRRToRRD
(
double u,
double v,
double &f,
double &g,
double &dfdu,
double &dfdv,
double &dgdu,
double &dgdv
) override 
    {
    double f0, g0, f1, g1;
    double df0du, dg0du, df1dv, dg1dv;
    if (   mEvaluatorA.EvaluateRToRRD (u, f0, g0, df0du, dg0du)
        && mEvaluatorB.EvaluateRToRRD (v, f1, g1, df1dv, dg1dv))
        {
        f = f0 - f1;
        g = g0 - g1;
        dfdu = df0du;
        dfdv = - df1dv;
        dgdu = dg0du;
        dgdv = - dg1dv;
        return true;
        }
    return false;
    }
};

/*-----------------------------------------------------------------*//**
// Run Newton stesp for a function represented by two Newton2UnivariateEvaluator's
// @param u INOUT first  variable.
// @param v INOUT second variable.
// @param evaluatorA IN first evaluator object
// @param evaluatorB IN second evaluator object
 @bsimethod                                  EarlinLutz      04/07
+---------------+---------------+---------------+---------------+------*/
bool NewtonIterationsRRToRR::RunNewtonDifference
(
double &u,
double &v,
FunctionRToRRD &evaluatorA,
FunctionRToRRD &evaluatorB
)
    {
    Newton2DifferenceEvaluator f(evaluatorA, evaluatorB);
    return RunNewton (u, v, f);
    }



/*-----------------------------------------------------------------*//**
CONSTRUCTOR
 @bsimethod                                  EarlinLutz      04/07
+---------------+---------------+---------------+---------------+------*/
Function_DEllipse3d_AngleToXY::Function_DEllipse3d_AngleToXY (DEllipse3dCR ellipse)
    {
    mEllipse = ellipse;
    }


/*-----------------------------------------------------------------*//**
Virtual function implementation.
@param u IN first variable
@param f OUT first function value
@param dfdu OUT derivative of f wrt u
@param dfdv OUT derivative of f wrt v
@return true if function was evaluated.
@bsimethod                                EarlinLutz      04/07
+---------------+---------------+---------------+---------------+------*/
bool Function_DEllipse3d_AngleToXY::EvaluateRToRRD
(
double theta,
double &f0,
double &f1,
double &df0,
double &df1
)
    {
    DPoint3d X;
    DVec3d dX;
    double c = cos (theta);
    double s = sin (theta);
    X.SumOf (mEllipse.center, mEllipse.vector0, c, mEllipse.vector90, s);
    dX.SumOf (mEllipse.vector0, -s, mEllipse.vector90, c);
//    mEllipse.Evaluate (X, dX, ddX, theta);
    f0 = X.x;
    f1 = X.y;
    df0 = dX.x;
    df1 = dX.y;
    return true;
    }

/*-----------------------------------------------------------------*//**
Virtual function implementation.
@param u IN first variable
@param f OUT first function value
@return true if function was evaluated.
@bsimethod                                EarlinLutz      04/07
+---------------+---------------+---------------+---------------+------*/
bool Function_DEllipse3d_AngleToXY::EvaluateRToRR
(
double theta,
double &f0,
double &f1
)
    {
    DPoint3d X;
    double c = cos (theta);
    double s = sin (theta);
    X.SumOf (mEllipse.center, mEllipse.vector0, c, mEllipse.vector90, s);

    //DPoint3d X, dX, ddX;
    //mEllipse.Evaluate (X, dX, ddX, theta);
    f0 = X.x;
    f1 = X.y;
    return true;
    }


/*-----------------------------------------------------------------*//**
CONSTRUCTOR
 @bsimethod                                  EarlinLutz      04/07
+---------------+---------------+---------------+---------------+------*/
Function_DEllipse3dOffset_AngleToXY::Function_DEllipse3dOffset_AngleToXY
(
DEllipse3dCR ellipse,
double offset
)
    {
    mEllipse = ellipse;
    mOffset = offset;
    mPlaneNormal.NormalizedCrossProduct (mEllipse.vector0, mEllipse.vector90);
    mUcrossN.CrossProduct (mEllipse.vector0,  mPlaneNormal);
    mVcrossN.CrossProduct (mEllipse.vector90, mPlaneNormal);
    }

/*-----------------------------------------------------------------*//**
Virtual function implementation.
@param u IN first variable
@param f OUT first function value
@param dfdu OUT derivative of f wrt u
@param dfdv OUT derivative of f wrt v
@return true if function was evaluated.
@bsimethod                                EarlinLutz      04/07
+---------------+---------------+---------------+---------------+------*/
bool Function_DEllipse3dOffset_AngleToXY::EvaluateRToRRD
(
double theta,
double &f0,
double &f1,
double &df0,
double &df1
)
    {
    double cc = cos (theta);
    double ss = sin (theta);
    DPoint3d ellipsePoint;
    DPoint3d offsetPoint;
    DVec3d unitPerp;
    DVec3d W;
    DVec3d dW;
    DVec3d dR;
    ellipsePoint.SumOf (mEllipse.center, mEllipse.vector0, cc, mEllipse.vector90, ss);
    // W is the unnormalized pependicular vector ....
    W.SumOf (mUcrossN, -ss, mVcrossN, cc);
    double WdotW = W.DotProduct (W);
    double magW = sqrt (WdotW);
    // derivative ...
    dW.SumOf (mUcrossN, -cc, mVcrossN, -ss);
    dR.SumOf (mEllipse.vector0, -ss, mEllipse.vector90, cc);
    double divMagW;

    bool    divideOK = DoubleOps::SafeDivide (divMagW, 1.0, magW, 0.0);
    double aa = W.DotProduct (dW) * divMagW * divMagW * divMagW;
    // unit perpendicular ...
    unitPerp.Scale (W, divMagW);

    offsetPoint.SumOf (ellipsePoint, unitPerp, mOffset);
    f0 = offsetPoint.x;
    f1 = offsetPoint.y;
    df0 = dR.x + mOffset * (dW.x * divMagW - W.x * aa);
    df1 = dR.y + mOffset * (dW.y * divMagW - W.y * aa);
    return divideOK ? true : false;
    }

/*-----------------------------------------------------------------*//**
Virtual function implementation.
@param u IN first variable
@param f OUT first function value
@param dfdu OUT derivative of f wrt u
@param dfdv OUT derivative of f wrt v
@return true if function was evaluated.
@bsimethod                                EarlinLutz      04/07
+---------------+---------------+---------------+---------------+------*/
bool Function_DEllipse3dOffset_AngleToXY::EvaluateRToRR
(
double theta,
double &f0,
double &f1
)
    {
    double cc = cos (theta);
    double ss = sin (theta);
    DPoint3d ellipsePoint;
    DPoint3d offsetPoint;
    DVec3d unitPerp;
    DVec3d W;
    ellipsePoint.SumOf (mEllipse.center, mEllipse.vector0, cc, mEllipse.vector90, ss);
    W.SumOf (mUcrossN, -ss, mVcrossN, cc);

    double WdotW = W.DotProduct (W);
    double magW = sqrt (WdotW);
    double divMagW;

    bool    divideOK = DoubleOps::SafeDivide (divMagW, 1.0, magW, 0.0);
    unitPerp.Scale (W, divMagW);
    offsetPoint.SumOf (ellipsePoint, unitPerp, mOffset);
    f0 = offsetPoint.x;
    f1 = offsetPoint.y;
    return divideOK ? true : false;
    }


/*-----------------------------------------------------------------*//**
CONSTRUCTOR
 @bsimethod                                  EarlinLutz      04/07
+---------------+---------------+---------------+---------------+------*/
Function_Bezier_FractionToXY::Function_Bezier_FractionToXY
(
DPoint4dCP pPoles,
int        order
)
    {
    if (order <= 0)
        {
        mOrder = 1;
        mPoles[0].Zero ();
        }
    else
        {
        if (order > sMaxCurveOrder)
            order = sMaxCurveOrder;
        mOrder = order;
        memcpy (mPoles, pPoles, order * sizeof (DPoint4d));
        }
    double weightTol = 1.0e-15;
    mbIsUnitWeight = true;
    for (int i = 0; i < order; i++)
        {
        if (fabs (mPoles[i].w - 1.0) > weightTol)
            {
            mbIsUnitWeight = false;
            }
        }
    }

/*-----------------------------------------------------------------*//**
CONSTRUCTOR
 @bsimethod                                  EarlinLutz      04/07
+---------------+---------------+---------------+---------------+------*/
Function_Bezier_FractionToXY::Function_Bezier_FractionToXY
(
DPoint3dCP pPoles,
int        order
)
    {
    if (order <= 0)
        {
        mOrder = 1;
        mPoles[0].Zero ();
        }
    else
        {
        if (order > sMaxCurveOrder)
            order = sMaxCurveOrder;
        mOrder = order;
        for (int i = 0; i < mOrder; i++)
            {
            mPoles[i].SetComponents (pPoles[i].x, pPoles[i].y, pPoles[i].z, 1.0);
            }
        }
    mbIsUnitWeight = true;
    }

/*-----------------------------------------------------------------*//**
Virtual function implementation.
@param u IN fraction
@param f0 OUT first function value
@param f1 OUT second function value.
@param df0 OUT derivative of f0 wrt u.
@param df1 OUT derivative of fy wrt u.
@return true if function was evaluated.
@bsimethod                                EarlinLutz      04/07
+---------------+---------------+---------------+---------------+------*/
bool Function_Bezier_FractionToXY::EvaluateRToRRD
(
double u,
double &f0,
double &f1,
double &df0,
double &df1
)
    {
    //double degree = mOrder - 1;
    DPoint4d Q, dQ;
    bsiBezier_functionAndDerivativeExt
                (
                (double *)&Q,
                (double *)&dQ,
                 NULL,
                (double *)mPoles,
                mOrder,
                4,
                u);

    f0 = Q.x;
    f1 = Q.y;
    df0 = dQ.x;
    df1 = dQ.y;
    if (mbIsUnitWeight)
        return true;

    double dw;
    double dw2;
    if (!DoubleOps::SafeDivide (dw, 1.0, Q.w, 0.0))
        return false;
    f0 *= dw;
    f1 *= dw;
    dw2 = dw * dw;
    df0 = (dQ.x * Q.w - Q.x * dQ.w) * dw2;
    df1 = (dQ.y * Q.w - Q.y * dQ.w) * dw2;
    return true;
    }

/*-----------------------------------------------------------------*//**
Virtual function implementation.
@param u IN first variable
@param f OUT first function value
@param dfdu OUT derivative of f wrt u
@param dfdv OUT derivative of f wrt v
@return true if function was evaluated.
@bsimethod                                EarlinLutz      04/07
+---------------+---------------+---------------+---------------+------*/
bool Function_Bezier_FractionToXY::EvaluateRToRR
(
double u,
double &f0,
double &f1
)
    {
    //double degree = mOrder - 1;
    DPoint4d Q, dQ;
    bsiBezier_functionAndDerivativeExt
                (
                (double *)&Q,
                (double *)&dQ,
                 NULL,
                (double *)mPoles,
                mOrder,
                4,
                u);

    f0 = Q.x;
    f1 = Q.y;
    if (mbIsUnitWeight)
        return true;

    double dw;
    if (!DoubleOps::SafeDivide (dw, 1.0, Q.w, 0.0))
        return false;
    f0 *= dw;
    f1 *= dw;
    return true;
    }


END_BENTLEY_GEOMETRY_NAMESPACE
