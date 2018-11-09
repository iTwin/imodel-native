//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCMatrix.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HFCMatrix
//-----------------------------------------------------------------------------

#pragma once

BEGIN_IMAGEPP_NAMESPACE
/**

    This template class is not usually used directly, but through the
    interface of HFCMatrix objects that creates appropriate instances of
    this class.  This class is used to give a representation of a single row
    of a matrix that is compatible with the nature of the HFCMatrix class,
    providing the interface for managing rows of a matrix.

    This is a template class.  It requires two template parameters,
    according to the following syntax:

    @code
    template<size_t C, class T = double> class HFCMatrix;
    @end

    where @r{C} is the number of columns in the matrix row, and @r{T} is a
    numerical type that supports arithmetic and comparison operators.

    @see HFCMatrix

*/

template <size_t Columns, class HNumericType = double> class HNOVTABLEINIT HFCMatrixRow
    {
public:
    //:> Primary methods
    HFCMatrixRow();
    HFCMatrixRow(const HFCMatrixRow<Columns, HNumericType>& pi_rObj);
    HFCMatrixRow(const HNumericType* pi_pRawValues);
    ~HFCMatrixRow();

    HFCMatrixRow<Columns, HNumericType>&
    operator=(const HFCMatrixRow<Columns, HNumericType>& pi_rObj);
    HFCMatrixRow<Columns, HNumericType>&
    operator=(const HNumericType* pi_pRawValues);



    //:> Compare operator
    bool           operator==(const HFCMatrixRow<Columns, HNumericType>& pi_rObj) const;
    bool           operator!=(const HFCMatrixRow<Columns, HNumericType>& pi_rObj) const;
    bool           IsEqualTo(const HFCMatrixRow<Columns, HNumericType>& pi_rObj) const;
    bool           IsEqualTo(const HFCMatrixRow<Columns, HNumericType>& pi_rObj,
                              double                      pi_Tolerance) const;

    //:> Value related methods
    HNumericType&    operator[](size_t pi_ColumnNumber);
    const HNumericType&
    operator[](size_t pi_ColumnNumber) const;

    //:> Arithmetic operations
    HFCMatrixRow<Columns, HNumericType>
    operator+(const HFCMatrixRow<Columns, HNumericType>& pi_rObj) const;
    HFCMatrixRow<Columns, HNumericType>
    operator-(const HFCMatrixRow<Columns, HNumericType>& pi_rObj) const;
    HFCMatrixRow<Columns, HNumericType>
    operator-() const;
    HFCMatrixRow<Columns, HNumericType>
    operator/(HNumericType pi_RawValue) const;
    HFCMatrixRow<Columns, HNumericType>
    operator*(HNumericType pi_RawValue) const;
    HFCMatrixRow<Columns, HNumericType>&
    operator+=(const HFCMatrixRow<Columns, HNumericType>& pi_rObj);
    HFCMatrixRow<Columns, HNumericType>&
    operator-=(const HFCMatrixRow<Columns, HNumericType>& pi_rObj);
    HFCMatrixRow<Columns, HNumericType>&
    operator*=(HNumericType            pi_Multiplicand);
    HFCMatrixRow<Columns, HNumericType>&
    operator/=(HNumericType            pi_Divisor);

protected:

private:

public:
    HNumericType     m_Values[Columns];
    };



/**

    This template class can be used to create and instantiate small
    bi-dimensional matrix. The interface provided enables to perform some of
    the most basic operations, like simple arithmetic and comparison
    operators. The package however does not provide any of the interfaces
    applicable to square matrix, such as determinant calculation and matrix
    inversion.

    This is a template class.  It requires three template parameters,
    according to the following syntax:

    @code
    template<size_t R, size_t C, class T = double> class HFCMatrix;
    @end

    where @r{R} is the number of rows, @{C} is the number of columns, and @{T} is a
    numerical type that supports arithmetic and comparison operators.

    This class also uses another template class in its interface,
    @r{HFCMatrixRow}, that is used to provide the same kind of representation
    for single rows into the matrix.

    The present package is in no way optimized for special cases that can
    occur for speeding matrix operation. A triangular matrix will not be
    recognized, nor a diagonal matrix. This fact implies that operations can
    be quite long to perform if the matrix is of great size. It is
    unadvisable to instantiate a matrix with more than 400 positions (20 by
    20). Even then, all methods returning an array will usually perform a
    full matrix copy, which can be lengthy.

    @see HFCMatrixRow
*/

template<size_t Rows, size_t Columns, class HNumericType = double> class HNOVTABLEINIT HFCMatrix
    {
public:

    //:> Primary methods

    HFCMatrix();
    HFCMatrix(const HFCMatrix<Rows, Columns, HNumericType>& pi_rObj);
    HFCMatrix(const HFCMatrixRow<Columns, HNumericType>* pi_pRows);
    HFCMatrix(const HNumericType* pi_pRawValues);
    HFCMatrix(const HNumericType pi_aaRawValues[Rows][Columns]);
    HFCMatrix<Rows, Columns, HNumericType>&
    operator=(const HNumericType pi_aaRawValues[Rows][Columns]);
    HFCMatrix<Rows, Columns, HNumericType>&
    operator=(const HNumericType* pi_pRawValues);
    HFCMatrix<Rows, Columns, HNumericType>&
    operator=(const HFCMatrixRow<Columns, HNumericType>* pi_pnRows);


    HFCMatrix<Rows, Columns, HNumericType>&
    operator=(const HFCMatrix<Rows, Columns, HNumericType>& pi_rObj);
    ~HFCMatrix();


    //:> Compare operator
    bool           operator==(const HFCMatrix<Rows, Columns, HNumericType>& pi_rObj) const;
    bool           operator!=(const HFCMatrix<Rows, Columns, HNumericType>& pi_rObj) const;
    bool           IsEqualTo(const HFCMatrix<Rows, Columns, HNumericType>& pi_rObj) const;
    bool           IsEqualTo(const HFCMatrix<Rows, Columns, HNumericType>& pi_rObj,
                              double                         pi_Tolerance) const;

    //:> Value related methods
    HFCMatrixRow<Columns, HNumericType>&
    operator[](size_t pi_RowNumber);
    const HFCMatrixRow<Columns, HNumericType>&
    operator[](size_t pi_RowNumber) const;


    //:> Array multiplication
    /**----------------------------------------------------------------------------
     Produces a matrix containing the result of the matrix multiplication of
     self with the specifed matrix. Matrix multiplication is performed by
     obtaining the cross product of rows from self with columns from given. For this
     reason, the given matrix must have as many rows as self has columns. The
     result matrix has as much row as the first operand (self) but as many
     columns as the second operand (given).

     @note This operation is a template method declared and defined in a template
           class. Such an implementation is not supported on some compilers, like
           Microsoft VisualC++ v5.0. In the latter case, only multiplication of
           two square matrix is supported (R = C = C2).

     @param pi_rObj Reference to another matrix to multiply with.  Must have as many
                    rows as the left operand has columns.

     @return A matrix containing the result of the matrix multiplication.
    -----------------------------------------------------------------------------*/	
    template<size_t OtherColumns> HFCMatrix<Rows, OtherColumns, HNumericType>
    operator*(const HFCMatrix<Columns, OtherColumns, HNumericType>& pi_rObj) const
    {
    // Create recipient matrix
    HFCMatrix<Rows, OtherColumns, HNumericType>   NewMatrix;

    // Declare required index counters
    size_t i;
    size_t j;
    size_t k;

    // For all rows of new matrix
    for (i = 0 ; i < Rows ; ++i)
        {
        // For all columns of new matrix of this row
        for (j = 0 ; j < OtherColumns ; ++j)
            {
            // Init position to 0
            NewMatrix.m_Rows[i][j] = 0;

            // Start summation of products
            for (k = 0 ; k < Columns ; ++k)
                {
                NewMatrix.m_Rows[i][j] += m_Rows[i][k] * pi_rObj.m_Rows[k][j];
                }
            }
        }

    // Return result
    return(NewMatrix);

    }




    //:> Basic Arithmetic operations
    HFCMatrix<Rows, Columns, HNumericType>
    operator+(const HFCMatrix<Rows, Columns, HNumericType>& pi_rObj) const;
    HFCMatrix<Rows, Columns, HNumericType>
    operator-(const HFCMatrix<Rows, Columns, HNumericType>& pi_rObj) const;
    HFCMatrix<Rows, Columns, HNumericType>
    operator-() const;
    HFCMatrix<Rows, Columns, HNumericType>
    operator/(HNumericType pi_RawValue) const;
    HFCMatrix<Rows, Columns, HNumericType>
    operator*(HNumericType pi_RawValue) const;
    HFCMatrix<Rows, Columns, HNumericType>&
    operator+=(const HFCMatrix<Rows, Columns, HNumericType>& pi_rObj);
    HFCMatrix<Rows, Columns, HNumericType>&
    operator-=(const HFCMatrix<Rows, Columns, HNumericType>& pi_rObj);
    HFCMatrix<Rows, Columns, HNumericType>&
    operator*=(HNumericType        pi_Multiplicand);
    HFCMatrix<Rows, Columns, HNumericType>&
    operator/=(HNumericType        pi_Divisor);


    //:> Matrix specific operations
    HFCMatrix<Columns, Rows, HNumericType>
    CalculateTranspose() const;


protected:

private:

public:  //:> public because we use HPM_DECLARE_TYPE instead of HPM_DECLARE_CLASS

    HFCMatrixRow<Columns, HNumericType>     m_Rows[Rows];
    };


// Associated Template method
template<size_t Columns, class HNumericType> inline HFCMatrixRow<Columns, HNumericType>
operator*(HNumericType pi_RawDouble,
          const HFCMatrixRow<Columns, HNumericType>& pi_rMatrix);
template<size_t Rows, size_t Columns, class HNumericType> inline HFCMatrix<Rows, Columns, HNumericType>
operator*(HNumericType pi_RawDouble, const HFCMatrix<Rows, Columns, HNumericType>& pi_rMatrix);

END_IMAGEPP_NAMESPACE

#include "HFCMatrix.hpp"
