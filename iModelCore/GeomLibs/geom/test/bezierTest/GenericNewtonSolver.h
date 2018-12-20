/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/test/bezierTest/GenericNewtonSolver.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once


#ifndef _MXSTUFF_
#include "MXStuff.h"
#endif
class TGenericNewtonSolver;

#ifdef MaxSurfStyle
#define FIRST_INDEX 1
#define TEST_INDEX(_i,_n) _i <= _n
#define INDEX_TYPE long
#else
#define FIRST_INDEX 0
#define TEST_INDEX(_i,_n) _i < _n
#define INDEX_TYPE size_t
#endif





// range, tolerance, and step size controls for one variable in the multivariate 
//  Newton iterator.
class TDofControls
{
friend TGenericNewtonSolver;
protected:
// convergence tolerance;
double mTolerance;
// absolute minimum value allowed 
double mMinValue;
// absolute maximum value allowed
double mMaxValue;
// largest change allowed in a single step
double mMaxStep;
// step to impose for approximate derivatives
double mApproximateDerivativeStep;
public:
// Default values -- no step size or range limits
TDofControls ();
TDofControls
      (
      double tolerance,
      double minValue = -DBL_MAX,
      double maxValue = DBL_MAX,
      double maxStep = 1.0,
      double approximateDerivativeStep = 1.0e-3
      );

// return 1.0 if maxStep is zero or negative.
// return 1.0 if step is less than maxStep
// otherwise return step/maxStep
static double ComputeStepsizeRestraintFactor (double step, double maxStep);

// return smaller of restraint factors for minX, maxX
static double ComptueIntervalRestraintFactor (double x, double step, double minX, double maxX);

// Apply all restraints to determine a scale factor applicable to a step from x.
double ComputeRestraintFactor (double x, double step, bool applyStepLimits = true, bool applyMinMaxLimits = true) const;

// Compare to tolerance.  Zero or negative tolerance means always accept.
bool IsConverged (double step) const;
};


// Interface class for user code to interact with the Newton solver steps.
class INewtonSolverEvents
{
// Ask permission to start an iteration.
// The callee may adjust variable values arbitrarily.
// Default implementation returns true.
public: virtual bool StartIteration (TGenericNewtonSolver &solver);
// Request newton evaluation with derivatives.
// The default implementaton returns false.
public: virtual bool EvaluateNewtonFunctionWithDerivatives (TGenericNewtonSolver &solver);
// Request newton evaluation.
// The default implementation returns false.
public: virtual bool EvaluateNewtonFunction (TGenericNewtonSolver &solver);
// Annouce an accepted solution.  Default returns true.
public: virtual bool IsAcceptedSolution (TGenericNewtonSolver &solver);

};

// Multidimensional Newton solver.  This class manages
// 1) individual tolerance and range limits per variable
// 2) Iteration logic with iteration count limits
// 3) Optional approximate derivatives from repeated function calls.
class TGenericNewtonSolver
{
private:
INDEX_TYPE mNumDofs;

bvector <TDofControls> mTDofControls;
bvector <double> mX;  // Indepdendent variables
bvector <double> mDX;
bvector <double> mF;
// saved function and variables ...
bvector <double> mF0;
bvector <double> mX0;

RowMajorMatrix mJ;  // jacobian mJ[i,j] = derivative of mF[i] wrt variable j.
bvector<double> mSolveDX;
bvector<double> mSolveF;

INDEX_TYPE mMaxIterations;
INDEX_TYPE mSuccessiveConvegencesRequired;

INDEX_TYPE mNumIterations;
INDEX_TYPE mNumSuccessiveConvergences;

public:
// Constructor for Newton iterator.
// Caller may make additional calls to provide TDofControls per variable.
TGenericNewtonSolver
  (
  INDEX_TYPE numDofs,
  INDEX_TYPE maxIterations = 20,
  INDEX_TYPE defaultSuccessiveConvergencesRequired = 2
  );

void SetX (INDEX_TYPE i, double value) const;
double GetX (INDEX_TYPE i) const;
void SetF (INDEX_TYPE i, double value);
void SetDerivative (INDEX_TYPE i, INDEX_TYPE j, double value);


// 
void SetMaxIterations (INDEX_TYPE maxIterations) {mMaxIterations = maxIterations;}
void SetSuccessiveConvergencesRequried (INDEX_TYPE successiveConvergencesRequired) {mSuccessiveConvegencesRequired = successiveConvergencesRequired;}

// Queries and actions asked during iterations ...
// Reset all counters to zero ..
void ClearIterationCounters ();
// Test if another iteration is permitted (i.e. check iteration counters)
bool CanStartIteration ();

// Ask if ready to accept after a step ...
bool CanAccept (bool thisStepConvrged);

// Save current x and function values.
void SaveXF ();
// Restore saved x and function values.
void RestoreXF ();

// Compute the dofIndex column of the jacobian as the mF-mF0 divided by delta ...
void ComputeApproximateDerivative (INDEX_TYPE dofIndex, double delta);

bool ValidIndex (INDEX_TYPE i) const;

// Set all X values to a constant ..
void SetAllX (double value);
// Set all TDofControls from a single source
void SetAllTDofControls (TDofControls const &data);

// Set a single DOF value
void SetX (INDEX_TYPE i, double value);

// Set a single TDofControls
void SetTDofControls (INDEX_TYPE i, TDofControls const &controls);

bool GetTDofControls (INDEX_TYPE i, TDofControls &controls) const;

double GetLastChange (INDEX_TYPE i, double value) const;

double GetFunction (INDEX_TYPE i, double value) const;

// to be called when jacobian is fully assembled.
// Returns true if the update step was computed (i.e. jacobian is invertible)
bool ComputeUnConstrainedStep ();

// Compute the factor to be applied to limit steps size, based on range and step size limits
// in each variable.
double ComputeConstraintFactor (bool applyStepLimits = true, bool applyMinMaxLimits = true);

// Return the largest change in any DOF
double MaxStep ();

// Subtract (scaled) DOF changes from DOF
void ApplyNegatedStep (double factor = 1.0);

// Return true if all steps are within their individual tolerance.
bool IsStepConverged ();

// Run Newton iterations with an evaluator that returns derivatives
bool RunIterations (INewtonSolverEvents &evaluator);

// Make repeated function calls to compute approximate derivatives.
bool EvaluateAllApproximateDerivatives (INewtonSolverEvents &evaluator);

// Run Newton iterations with an evaluator that returns only function values.
//  The evaluator will be called repeatedly to obtain approximate derivatives
bool RunIterationsWithApproximateDerivatives (INewtonSolverEvents &evaluator);

};