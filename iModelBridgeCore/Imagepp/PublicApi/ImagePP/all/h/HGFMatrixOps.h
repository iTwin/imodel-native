/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ImagePP/all/h/HGFMatrixOps.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

#pragma once

#include "HFCMatrix.h"

BEGIN_IMAGEPP_NAMESPACE

// For some reason, the compiler does not compute
#if (0)
template<size_t Rows, class NumericType>
HFCMatrix<Rows, Rows, NumericType> InverseMatrix(const HFCMatrix<Rows, Rows, NumericType>& pi_rMatrix);
template<size_t Rows, class NumericType>
double CalculateDeterminant(const HFCMatrix<Rows, Rows, NumericType>& pi_rMatrix);
template<class NumericType>
double CalculateDeterminant(const HFCMatrix<2, 2, NumericType>& pi_rMatrix);

template<int Rows, class NumericType>
HFCMatrix<Rows - 1, Rows - 1, NumericType> ExtractSubMatrix(const HFCMatrix<Rows, Rows, NumericType>& pi_rMatrix,
                                                            size_t pi_Row,
                                                            size_t pi_Column);

template<class NumericType>
HFCMatrix<1, 1, NumericType> ExtractSubMatrix(const HFCMatrix<2, 2, NumericType>& pi_rMatrix,
                                              size_t pi_Row,
                                              size_t pi_Column);


#else

HFCMatrix<4, 4> InverseMatrix(const HFCMatrix<4, 4>& pi_rMatrix);
double CalculateDeterminant(const HFCMatrix<4, 4, double>& pi_rMatrix);
double CalculateDeterminant(const HFCMatrix<3, 3, double>& pi_rMatrix);
double CalculateDeterminant(const HFCMatrix<2, 2, double>& pi_rMatrix);
HFCMatrix<3, 3> ExtractSubMatrix(const HFCMatrix<4, 4>& pi_rMatrix,
                                 size_t pi_Row,
                                 size_t pi_Column);

HFCMatrix<2, 2> ExtractSubMatrix(const HFCMatrix<3, 3>& pi_rMatrix,
                                 size_t pi_Row,
                                 size_t pi_Column);

HFCMatrix<1, 1> ExtractSubMatrix(const HFCMatrix<2, 2>& pi_rMatrix,
                                 size_t pi_Row,
                                 size_t pi_Column);


#endif


END_IMAGEPP_NAMESPACE

#include "HGFMatrixOps.hpp"

