//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCMaths.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Utility maths functions declaration.
//-----------------------------------------------------------------------------

#pragma once

BEGIN_IMAGEPP_NAMESPACE
//-----------------------------------------------------------------------------
// Function prototypes.
//-----------------------------------------------------------------------------

// Work for any double number [-MAX_DOUBLE, MAX_DOUBLE],
// much faster than conventionnal pow(double x, 1.0/3.0);
double FastCubicRoot(double x);

// Domain limited [0, 1], faster than the previous FastCubicRoot.
double LimitedFastCubicRoot(double pi_Number);

// Descriptive statistics - Measure of central tendency
double GeometricMean(double* pi_pArray, size_t pi_ArraySize);
double GeometricMean(double pi_FirstValue, double pi_SecondValue);

double ArithmeticMean(double* pi_pArray, size_t pi_ArraySize);
double ArithmeticMean(double pi_FirstValue, double pi_SecondValue);

double HarmonicMean(double* pi_pArray, size_t pi_ArraySize);
double HarmonicMean(double pi_FirstValue, double pi_SecondValue);

END_IMAGEPP_NAMESPACE

#include "HFCMaths.hpp"

