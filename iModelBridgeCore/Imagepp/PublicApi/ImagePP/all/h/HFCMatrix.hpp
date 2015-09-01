//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCMatrix.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

//*****************************************************************************
// Friend functions

BEGIN_IMAGEPP_NAMESPACE
//-----------------------------------------------------------------------------
// operator* template function
// Multiplies a number by a row.
//-----------------------------------------------------------------------------
template<size_t Columns, class NumericType> HFCMatrixRow<Columns, NumericType>
operator*(NumericType pi_RawNumber,
          const HFCMatrixRow<Columns, NumericType>& pi_rMatrix)
    {
    // Create new row
    HFCMatrixRow<Columns, NumericType>   NewRow;

    // Multiply
    for(size_t i = 0; i < Columns ; ++i)
        NewRow.m_Values[i] = pi_rMatrix.m_Values[i] * pi_RawNumber;

    // Return recipient
    return(NewRow);
    }

//-----------------------------------------------------------------------------
// operator* template function
// Multiplies a number by a matrix.
//-----------------------------------------------------------------------------
template<size_t Rows, size_t Columns, class NumericType> HFCMatrix<Rows, Columns, NumericType>
operator*(NumericType pi_RawNumber, const HFCMatrix<Rows, Columns, NumericType>& pi_rMatrix)
    {
    // Create recipient matrix
    HFCMatrix<Rows, Columns, NumericType>   NewMatrix;

    // Set rows to rows of self scaled by factor
    for(size_t i = 0 ; i < Rows ; ++i)
        NewMatrix[i] = pi_rMatrix[i] * pi_RawNumber;

    // return recipient
    return (NewMatrix);
    }



//*****************************************************************************
//*****************************************************************************
// HFCMatrixRow method definition
//*****************************************************************************


//-----------------------------------------------------------------------------
// Constructor for an empty row (values set to 0.0).
//-----------------------------------------------------------------------------
template<size_t Columns, class NumericType>
inline HFCMatrixRow<Columns, NumericType>::HFCMatrixRow()
    {
    // Set all values to 0.0
    for(register size_t i = 0; i < Columns ; ++i)
        m_Values[i] = 0;
    }




//-----------------------------------------------------------------------------
// Constructor from raw values
//-----------------------------------------------------------------------------
template<size_t Columns, class NumericType>
inline HFCMatrixRow<Columns, NumericType>::HFCMatrixRow(const NumericType* pi_pRawValues)
    {
    // Set all values to 0.0
    for(register size_t i = 0; i < Columns ; ++i)
        m_Values[i] = pi_pRawValues[i];
    }


//-----------------------------------------------------------------------------
// Copy constructor.
//-----------------------------------------------------------------------------
template<size_t Columns, class NumericType>
inline HFCMatrixRow<Columns, NumericType>::HFCMatrixRow(const HFCMatrixRow<Columns, NumericType>& pi_rObj)
    {
    // Duplicate all positions
    for(register size_t i = 0; i < Columns ; ++i)
        m_Values[i] = pi_rObj.m_Values[i];
    }


//-----------------------------------------------------------------------------
// The destructor.
//-----------------------------------------------------------------------------
template<size_t Columns, class NumericType>
inline HFCMatrixRow<Columns, NumericType>::~HFCMatrixRow()
    {
    }


//-----------------------------------------------------------------------------
// operator=
// Assignment operator.  It duplicates another row object.
//-----------------------------------------------------------------------------
template<size_t Columns, class NumericType>
inline HFCMatrixRow<Columns, NumericType>&
HFCMatrixRow<Columns, NumericType>::operator=(const HFCMatrixRow<Columns, NumericType>& pi_rObj)
    {
    // Check that object to copy is not self
    if (this != &pi_rObj)
        {
        // Duplicate all values of given row
        for(register size_t i = 0; i < Columns ; ++i)
            m_Values[i] = pi_rObj.m_Values[i];
        }

    // Return reference to self
    return (*this);
    }


//-----------------------------------------------------------------------------
// operator=
// Assignment operator.  It copies the given array
//-----------------------------------------------------------------------------
template<size_t Columns, class NumericType>
inline HFCMatrixRow<Columns, NumericType>&
HFCMatrixRow<Columns, NumericType>::operator=(const NumericType* pi_pRawValues)
    {
    // Duplicate all values of given row
    for(register size_t i = 0; i < Columns ; ++i)
        m_Values[i] = pi_pRawValues[i];

    // Return reference to self
    return (*this);
    }




//-----------------------------------------------------------------------------
// operator==
// Checks if the matrix rows are identical
//-----------------------------------------------------------------------------
template<size_t Columns, class NumericType>
inline bool
HFCMatrixRow<Columns, NumericType>::operator==(const HFCMatrixRow<Columns, NumericType>& pi_rObj) const
    {
    bool   ReturnValue = true;

    // Check if all values are equal
    for(register size_t i = 0; i < Columns && ReturnValue ; ++i)
        ReturnValue = (m_Values[i] == pi_rObj.m_Values[i]);

    // Return answer
    return(ReturnValue);
    }


//-----------------------------------------------------------------------------
// IsEqualTo
// Checks if the matrix rows are identical within the default global tolerance
//-----------------------------------------------------------------------------
template<size_t Columns, class NumericType>
inline bool
HFCMatrixRow<Columns, NumericType>::IsEqualTo(const HFCMatrixRow<Columns, NumericType>& pi_rObj) const
    {
    bool   ReturnValue = true;

    // Check if all values are equal
    for(register size_t i = 0; i < Columns && ReturnValue ; ++i)
        ReturnValue = HDOUBLE_EQUAL_EPSILON(m_Values[i], pi_rObj.m_Values[i]);

    // Return answer
    return(ReturnValue);
    }


//-----------------------------------------------------------------------------
// IsEqualTo
// Checks if the matrix rows are identical within the default global tolerance
//-----------------------------------------------------------------------------
template<size_t Columns, class NumericType>
inline bool
HFCMatrixRow<Columns, NumericType>::IsEqualTo(const HFCMatrixRow<Columns, NumericType>& pi_rObj,
                                              double                      pi_Tolerance) const
    {
    // The tolerance may not be negative
    HPRECONDITION(pi_Tolerance >= 0);

    // The given tolerance must be smaller or equal than maximum epsilon
    HPRECONDITION(pi_Tolerance <= HMAX_EPSILON);

    bool   ReturnValue = true;

    // Check if all values are equal
    for(register size_t i = 0; i < Columns && ReturnValue ; ++i)
        ReturnValue = HDOUBLE_EQUAL(m_Values[i], pi_rObj.m_Values[i], pi_Tolerance);

    // Return answer
    return(ReturnValue);
    }


//-----------------------------------------------------------------------------
// operator!=
// Checks if the matrix rows are different
//-----------------------------------------------------------------------------
template<size_t Columns, class NumericType>
inline bool
HFCMatrixRow<Columns, NumericType>::operator!=(const HFCMatrixRow<Columns, NumericType>& pi_rObj) const
    {
    return(!operator==(pi_rObj));
    }


//-----------------------------------------------------------------------------
// operator[]
// Returns a constant reference to value of the row at specified column
//-----------------------------------------------------------------------------
template<size_t Columns, class NumericType>
inline const NumericType&
HFCMatrixRow<Columns, NumericType>::operator[](size_t pi_ColumnNumber) const
    {
    // The column index must be valid
    HPRECONDITION(pi_ColumnNumber < Columns && pi_ColumnNumber >= 0);

    // Return constant reference to value
    return (m_Values[pi_ColumnNumber]);
    }


//-----------------------------------------------------------------------------
// operator[]
// Returns a reference to the number in row for specified column
//-----------------------------------------------------------------------------
template<size_t Columns, class NumericType>
inline NumericType&
HFCMatrixRow<Columns, NumericType>::operator[](size_t pi_ColumnNumber)
    {
    // The columns index must be valid
    HPRECONDITION(pi_ColumnNumber < Columns);
    HPRECONDITION(pi_ColumnNumber >= 0);

    // Return reference to value
    return(m_Values[pi_ColumnNumber]);
    }


//-----------------------------------------------------------------------------
// operator+
// Adds two rows together.
//-----------------------------------------------------------------------------
template<size_t Columns, class NumericType>
inline HFCMatrixRow<Columns, NumericType>
HFCMatrixRow<Columns, NumericType>::operator+(const HFCMatrixRow<Columns, NumericType>& pi_rObj) const
    {
    // Create a duplicate of self
    HFCMatrixRow<Columns, NumericType>   NewRow(*this);

    // Increment all positions of copy by values of given
    for(register size_t i = 0; i < Columns ; ++i)
        NewRow.m_Values[i] += pi_rObj.m_Values[i];

    // return recipient
    return(NewRow);
    }

//-----------------------------------------------------------------------------
// operator-
// Substracts two rows together.
//-----------------------------------------------------------------------------
template<size_t Columns, class NumericType>
inline HFCMatrixRow<Columns, NumericType>
HFCMatrixRow<Columns, NumericType>::operator-(const HFCMatrixRow<Columns, NumericType>& pi_rObj) const
    {
    // Create a duplicate of self
    HFCMatrixRow<Columns, NumericType>   NewRow(*this);

    // Decrement all positions of copy by values of given
    for(register size_t i = 0; i < Columns ; ++i)
        NewRow.m_Values[i] -= pi_rObj.m_Values[i];

    // return recipient
    return(NewRow);
    }


//-----------------------------------------------------------------------------
// operator- (unary)
// Negates the row
//-----------------------------------------------------------------------------
template<size_t Columns, class NumericType>
inline HFCMatrixRow<Columns, NumericType>
HFCMatrixRow<Columns, NumericType>::operator-() const
    {
    // Create recipient
    HFCMatrixRow<Columns, NumericType>   NewRow;

    // Set values negated
    for(register size_t i = 0; i < Columns ; ++i)
        NewRow.m_Values[i] = - m_Values[i];

    // return recipient
    return(NewRow);
    }


//-----------------------------------------------------------------------------
// operator/
// Divides a row by a number
//-----------------------------------------------------------------------------
template<size_t Columns, class NumericType>
inline HFCMatrixRow<Columns, NumericType>
HFCMatrixRow<Columns, NumericType>::operator/(NumericType pi_Divisor) const
    {
    // The divisor must be different from 0
    HPRECONDITION(pi_Divisor != 0);

    // Create new row
    HFCMatrixRow<Columns, NumericType>   NewRow;

    // Set proper values
    for(register size_t i = 0; i < Columns ; ++i)
        NewRow.m_Values[i] = m_Values[i] / pi_Divisor;

    // return recipient
    return(NewRow);
    }


//-----------------------------------------------------------------------------
// operator*
// Multiplies a row by a number.
//-----------------------------------------------------------------------------
template<size_t Columns, class NumericType>
inline HFCMatrixRow<Columns, NumericType>
HFCMatrixRow<Columns, NumericType>::operator*(NumericType pi_RawNumber) const
    {
    // Create new row
    HFCMatrixRow<Columns, NumericType>   NewRow;

    // Set proper values
    for(register size_t i = 0; i < Columns ; ++i)
        NewRow.m_Values[i] = m_Values[i] * pi_RawNumber;

    // Return recipient
    return(NewRow);
    }


//-----------------------------------------------------------------------------
// operator+=
// Increments self by another row of same dimension
//-----------------------------------------------------------------------------
template<size_t Columns, class NumericType>
inline HFCMatrixRow<Columns, NumericType>&
HFCMatrixRow<Columns, NumericType>::operator+=(const HFCMatrixRow<Columns,  NumericType>& pi_rObj)
    {
    // Increment all values of self row by values at same position
    // in given row
    for(register size_t i = 0; i < Columns ; ++i)
        m_Values[i] += pi_rObj.m_Values[i];

    // Return reference to self
    return(*this);
    }

//-----------------------------------------------------------------------------
// operator-=
// Decrements self by another row of same dimension
//-----------------------------------------------------------------------------
template<size_t Columns, class NumericType>
inline HFCMatrixRow<Columns, NumericType>&
HFCMatrixRow<Columns, NumericType>::operator-=(const HFCMatrixRow<Columns, NumericType>& pi_rObj)
    {
    // Decrement all values of self row by values at same position
    // in given row
    for(register size_t i = 0; i < Columns ; ++i)
        m_Values[i] -= pi_rObj.m_Values[i];

    // Return reference to self
    return(*this);
    }


//-----------------------------------------------------------------------------
// operator*=
// Scales row by a value.
//-----------------------------------------------------------------------------
template<size_t Columns, class NumericType>
inline HFCMatrixRow<Columns, NumericType>&
HFCMatrixRow<Columns, NumericType>::operator*=(NumericType pi_RawValue)
    {
    // Scale all values of self row by given factor
    for(register size_t i = 0; i < Columns ; ++i)
        m_Values[i] *= pi_RawValue;

    // Return reference to self
    return(*this);
    }


//-----------------------------------------------------------------------------
// operator/=
// Scales row by a value. The given value may not be 0
//-----------------------------------------------------------------------------
template<size_t Columns, class NumericType>
inline HFCMatrixRow<Columns, NumericType>&
HFCMatrixRow<Columns, NumericType>::operator/=(NumericType pi_RawValue)
    {
    // The divisor may not be 0
    HPRECONDITION (pi_RawValue != 0);

    // Scale all values of self row by given factor
    for(register size_t i = 0; i < Columns ; ++i)
        m_Values[i] /= pi_RawValue;

    // Return reference to self
    return(*this);
    }




//*****************************************************************************
//*****************************************************************************
// HFCMatrix method definition
//*****************************************************************************


/**----------------------------------------------------------------------------
 The default constructor for this class.  It creates a matrix of numeric values
 of type @r{NumericType}, having @{R} rows and @{C} columns, without doing
 any initialisation of the values.
-----------------------------------------------------------------------------*/

template<size_t Rows, size_t Columns, class NumericType>
inline HFCMatrix<Rows, Columns, NumericType>::HFCMatrix()
    {
    }

/**----------------------------------------------------------------------------
 Copy constructor for a matrix of numeric values of type
 @r{NumericType}, having @{R} rows and @{C} columns, using another
 matrix as a source of values to use for initialisation.

 @param pi_rObj Reference to another matrix to duplicate.
-----------------------------------------------------------------------------*/
template<size_t Rows, size_t Columns, class NumericType>
inline HFCMatrix<Rows, Columns, NumericType>::HFCMatrix(const HFCMatrix<Rows, Columns, NumericType>& pi_rObj)
    {
    // Copy all rows
    for(register size_t i = 0 ; i < Rows ; ++i)
        m_Rows[i] = pi_rObj.m_Rows[i];
    }

/**----------------------------------------------------------------------------
 Constructor for a matrix of numeric values of type @r{NumericType}, having @{R}
 rows and @{C} columns, using a native raw matrix as a source of values to
 use for initialisation.

 @param pi_pRawValues A pointer to a properly sized array of initialisation
                      values to copy from.
-----------------------------------------------------------------------------*/
template<size_t Rows, size_t Columns, class NumericType>
inline HFCMatrix<Rows, Columns, NumericType>::HFCMatrix(const NumericType* pi_pRawValues)
    {
    for (register size_t i = 0 ; i < Rows ; ++i)
        m_Rows = pi_pRawValues[i];
    }

/**----------------------------------------------------------------------------
 Constructor for a matrix of numeric values of type @r{NumericType}, having @{R}
 rows and @{C} columns, using an array of matrix rows of type HFCMatrixRow
 as a source of values to use for initialisation.

 @param pi_pRows A pointer to a properly sized array of matrix rows containing
                 initialisation values to copy from.
-----------------------------------------------------------------------------*/
template<size_t Rows, size_t Columns, class NumericType>
inline HFCMatrix<Rows, Columns, NumericType>::HFCMatrix(const HFCMatrixRow<Columns, NumericType>* pi_pRows)
    {
    for (register size_t i = 0 ; i < Rows ; ++i)
        m_Rows = pi_pRows[i];
    }

/**----------------------------------------------------------------------------
 Constructor for a matrix of numeric values of type @r{NumericType}, having @{R}
 rows and @{C} columns, using a native raw matrix as a source of values to
 use for initialisation.

 @note This version is not supported under Microsoft Visual C+++ v5.0.

 @param pi_aaRawValues A reference to a properly sized matrix of initialisation
                       values to copy from.
-----------------------------------------------------------------------------*/
template<size_t Rows, size_t Columns, class NumericType>
inline HFCMatrix<Rows, Columns, NumericType>::HFCMatrix(const NumericType pi_aaRawValues[Rows][Columns])
    {
    for (size_t i = 0 ; i < Rows ; ++i)
        m_Rows = pi_aaRawValues[i];
    }



/**----------------------------------------------------------------------------
 The destructor for this class.
-----------------------------------------------------------------------------*/
template<size_t Rows, size_t Columns, class NumericType>
inline HFCMatrix<Rows, Columns, NumericType>::~HFCMatrix()
    {
    }


/**----------------------------------------------------------------------------
 Assignment operator that initializes this matrix with values found in another
 specified matrix.

 @param pi_rObj Reference to the matrix to duplicate.

 @return Reference to self that can be used as an l-value.
-----------------------------------------------------------------------------*/
template<size_t Rows, size_t Columns, class NumericType>
inline HFCMatrix<Rows, Columns, NumericType>&
HFCMatrix<Rows, Columns, NumericType>::operator=(const HFCMatrix<Rows, Columns, NumericType>& pi_rObj)
    {
    // Check that object to copy is not self
    if (this != &pi_rObj)
        {
        // Copy all rows
        for(register size_t i = 0 ; i < Rows ; ++i)
            m_Rows[i] = pi_rObj.m_Rows[i];
        }

    // Return reference to self
    return(*this);
    }

/**----------------------------------------------------------------------------
 Assignment operator that initializes this matrix with values found in the
 specified native matrix.

 @param pi_rObj Pointer to the arrays of values to use to initialize this
                matrix.

 @return Reference to self that can be used as an l-value.
-----------------------------------------------------------------------------*/
template<size_t Rows, size_t Columns, class NumericType>
inline HFCMatrix<Rows, Columns, NumericType>&
HFCMatrix<Rows, Columns, NumericType>::operator=(const NumericType* pi_pRawValues)
    {
    // Copy all rows
    for(register size_t i = 0 ; i < Rows ; ++i)
        m_Rows[i] = pi_pRawValues[i];

    // Return reference to self
    return(*this);
    }


/**----------------------------------------------------------------------------
 Assignment operator that initializes this matrix with values found in the
 specified native matrix.

 @param pi_rObj Pointer to the arrays of values to use to initialize this
                matrix.

 @return Reference to self that can be used as an l-value.
-----------------------------------------------------------------------------*/
template<size_t Rows, size_t Columns, class NumericType>
inline HFCMatrix<Rows, Columns, NumericType>&
HFCMatrix<Rows, Columns, NumericType>::operator=(const HFCMatrixRow<Columns, NumericType>* pi_pRows)
    {
    // Copy all rows
    for(register size_t i = 0 ; i < Rows ; ++i)
        m_Rows[i] = pi_pRows[i];

    // Return reference to self
    return(*this);
    }


/**----------------------------------------------------------------------------
 Assignment operator that initializes this matrix with values found in another
 specified matrix.

 @note This version is not supported under Microsoft Visual C+++ v5.0.

 @param pi_rObj Reference to a properly sized matrix of values to use to
                initialize this matrix.

 @return Reference to self that can be used as an l-value.
-----------------------------------------------------------------------------*/
template<size_t Rows, size_t Columns, class NumericType> HFCMatrix<Rows, Columns, NumericType>&
HFCMatrix<Rows, Columns, NumericType>::operator=(const NumericType pi_aaRawValues[Rows][Columns])
    {
    // Copy all rows
    for(size_t i = 0 ; i < Rows ; ++i)
        m_Rows[i] = pi_aaRawValues[i];

    // Return reference to self
    return(*this);
    }



/**----------------------------------------------------------------------------
 Equality comparison operator.  Checks if two matrix having the same
 geometry have all values positions equal or different in a one to one
 compare operation. Two matrix to be equal must have all values
 identical. If a single one is different, then the matrix is considered
 different.

 @param pi_rObj  Reference to the matrix to compare with.

 @return true if both matrix are the same, false otherwise.

 @see IsEqualTo
-----------------------------------------------------------------------------*/
template<size_t Rows, size_t Columns, class NumericType>
inline bool HFCMatrix<Rows, Columns, NumericType>::operator==(const HFCMatrix<Rows, Columns, NumericType>& pi_rObj) const
    {
    bool   ReturnValue = true;

    // Check if all rows are equal
    for(register size_t i = 0; i < Columns && ReturnValue ; ++i)
        ReturnValue = (m_Rows[i] == pi_rObj.m_Rows[i]);

    // Return answer
    return(ReturnValue);
    }

/**----------------------------------------------------------------------------
 Non-equality comparison operator.  Checks if two matrix having the same
 geometry have all values positions equal or different in a one to one
 compare operation. Two matrix to be equal must have all values
 identical. If a single one is different, then the matrix is considered
 different.

 @param pi_rObj  Reference to the matrix to compare with.

 @return false if both matrix are the same, true otherwise.

 @see IsEqualTo
-----------------------------------------------------------------------------*/
template<size_t Rows, size_t Columns, class NumericType> inline bool
HFCMatrix<Rows, Columns, NumericType>::operator!=(const HFCMatrix<Rows, Columns, NumericType>& pi_rObj) const
    {
    return(!operator==(pi_rObj));
    }



/**----------------------------------------------------------------------------
 Equality comparison method.  Checks if two matrix having the same
 geometry have all values positions equal or different in a one to one
 compare operation. Two matrix to be equal must have all values
 identical. If a single one is different, then the matrix is considered
 different.

 @param pi_rObj  Reference to the matrix to compare with.

 @return true if both matrix are the same, false otherwise.

 @see operator==
-----------------------------------------------------------------------------*/
template<size_t Rows, size_t Columns, class NumericType>
inline bool HFCMatrix<Rows, Columns, NumericType>::IsEqualTo(const HFCMatrix<Rows, Columns, NumericType>& pi_rObj) const
    {
    bool   ReturnValue = true;

    // Check if all rows are equal
    for(register size_t i = 0; i < Columns && ReturnValue ; ++i)
        ReturnValue = m_Rows[i].IsEqualTo(pi_rObj.m_Rows[i]);

    // Return answer
    return(ReturnValue);
    }

/**----------------------------------------------------------------------------
 Similarity comparison method.  Checks if two matrix having the same
 geometry have all values positions being similar or different in a one to one
 compare operation.  Two values are considered similar if their difference is
 lower than the specified tolerance value. Two matrix to be similar must have all
 values identical. If a single one is different, then the matrix is considered
 different.

 This method is to be used with floating point values to compare equality
 within a given error tolerance caused by limits of floating point arithmetic
 processing.

 @param pi_rObj       Reference to the matrix to compare with.
 @param pi_Tolerance  Maximal difference to tolerate between two value to make
                      them to be considered similar.

 @return true if both matrix are the same, false otherwise.

 @see operator==
-----------------------------------------------------------------------------*/
template<size_t Rows, size_t Columns, class NumericType>
inline bool
HFCMatrix<Rows, Columns, NumericType>::IsEqualTo(const HFCMatrix<Rows, Columns, NumericType>& pi_rObj,
                                                 double                            pi_Tolerance) const
    {
    // The tolerance may not be negative
    HPRECONDITION(pi_Tolerance >= 0);

    // The given tolerance must be smaller or equal than maximum epsilon
    HPRECONDITION(pi_Tolerance <= HMAX_EPSILON);

    bool   ReturnValue = true;

    // Check if all rows are equal
    for(size_t i = 0; i < Columns && ReturnValue ; ++i)
        ReturnValue = (m_Rows[i].IsEqualTo(pi_rObj.m_Rows[i], pi_Tolerance));

    // Return answer
    return(ReturnValue);
    }

/**----------------------------------------------------------------------------
 One of the dereference operators. These operators return either a reference to
 the specified row or a const reference to the speficied row, depending on
 the constness of the self matrix. There is no operator provided
 permitting to obtain a reference to columns, since the HFCMatrixRow class has
 exactly the same interface to reach a given value in the row.

 @param pi_RowNumber  Numeric index of the row to access, the first one being
                      indexed as zero.

 @return A reference to the specified row in the matrix, under the form of an
         object of class HFCMatrixRow.

 @see HFCMatrixRow
-----------------------------------------------------------------------------*/
template<size_t Rows, size_t Columns, class NumericType>
inline const HFCMatrixRow<Columns, NumericType>&
HFCMatrix<Rows, Columns, NumericType>::operator[](size_t pi_RowNumber) const
    {
    // The row index must be valid
    HPRECONDITION(pi_RowNumber < Rows && pi_RowNumber >= 0);

    // Return constant reference to self
    return(m_Rows[pi_RowNumber]);
    }

/**----------------------------------------------------------------------------
 One of the dereference operators. These operators return either a reference to
 the specified row or a const reference to the speficied row, depending on
 the constness of the self matrix. There is no operator provided
 permitting to obtain a reference to columns, since the HFCMatrixRow class has
 exactly the same interface to reach a given value in the row.

 @param pi_RowNumber  Numeric index of the row to access, the first one being
                      indexed as zero.

 @return A reference to the specified row in the matrix, under the form of an
         object of class HFCMatrixRow.

 @see HFCMatrixRow
-----------------------------------------------------------------------------*/
template<size_t Rows, size_t Columns, class NumericType>
inline HFCMatrixRow<Columns, NumericType>&
HFCMatrix<Rows, Columns, NumericType>::operator[](size_t pi_RowNumber)
    {
    // The row index must be valid
    HPRECONDITION(pi_RowNumber < Rows && pi_RowNumber >= 0);

    return(m_Rows[pi_RowNumber]);
    }



/**----------------------------------------------------------------------------
 Produces a matrix as the addition of this one with another. The result
 and the provided matrix must have the same geometry as the self matrix. The
 addition of the two matrixes is performed by adding values of matrixes on a
 one to one basis:

 @c{Result[0][0] = Self[0][0] + Given[0][0]} and so on.

 @param pi_rObj Reference to another matrix to add to this one.

 @return A matrix containing the result of the addition.
-----------------------------------------------------------------------------*/
template<size_t Rows, size_t Columns, class NumericType>
inline HFCMatrix<Rows, Columns, NumericType>
HFCMatrix<Rows, Columns, NumericType>::operator+(const HFCMatrix<Rows, Columns, NumericType>& pi_rObj) const
    {
    // Create duplicate of self
    HFCMatrix<Rows, Columns, NumericType>   NewMatrix(*this);

    // Increment all rows by rows of given matrix
    for(register size_t i = 0 ; i < Rows ; ++i)
        NewMatrix.m_Rows[i] += pi_rObj.m_Rows[i];

    // Return recipient
    return(NewMatrix);
    }


/**----------------------------------------------------------------------------
 Produces a matrix as the substraction of this one with another. The result
 and the provided matrix must have the same geometry as the self matrix. The
 substraction of the two matrixes is performed by substracting values of matrixes
 on a one to one basis:

 @c{Result[0][0] = Self[0][0] - Given[0][0]} and so on.

 @param pi_rObj Reference to another matrix to substract to this one.

 @return A matrix containing the result of the substraction.
-----------------------------------------------------------------------------*/
template<size_t Rows, size_t Columns, class NumericType>
inline HFCMatrix<Rows, Columns, NumericType>
HFCMatrix<Rows, Columns, NumericType>::operator-(const HFCMatrix<Rows, Columns, NumericType>& pi_rObj) const
    {
    // Create duplicate of self
    HFCMatrix<Rows, Columns, NumericType>   NewMatrix(*this);

    // Decrement all rows by rows of given matrix
    for(register size_t i = 0 ; i < Rows ; ++i)
        NewMatrix.m_Rows[i] -= pi_rObj.m_Rows[i];

    // Return recipient
    return(NewMatrix);
    }


/**----------------------------------------------------------------------------
 Produces a matrix as the negation of self. The negation of a matrix is
 performed by negating values of the matrix on a one to one basis:

 @c{Result[0][0] = -Self[0][0]} and so on.

 @return A matrix containing the result of the negation.
-----------------------------------------------------------------------------*/
template<size_t Rows, size_t Columns, class NumericType>
inline HFCMatrix<Rows, Columns, NumericType>
HFCMatrix<Rows, Columns, NumericType>::operator-() const
    {
    // Create recipient
    HFCMatrix<Rows, Columns, NumericType>   NewMatrix;

    // Set all values negated
    for(register size_t i = 0 ; i < Rows ; ++i)
        NewMatrix.m_Rows[i] = -m_Rows[i];

    // Return recipient
    return(NewMatrix);
    }


/**----------------------------------------------------------------------------
 Produces a matrix of the same geometry as self being the scaled version
 of self where all values are multiplied by the provided factor.

 @param pi_Divisor   The scale factor to multiply all values with.

 @return A matrix containing the divided values.
-----------------------------------------------------------------------------*/
template<size_t Rows, size_t Columns, class NumericType>
inline HFCMatrix<Rows, Columns, NumericType>
HFCMatrix<Rows, Columns, NumericType>::operator*(NumericType pi_Factor) const
    {
    // The divisor must be different from 0
    HPRECONDITION(pi_Factor != 0);

    // Create recipine matrix
    HFCMatrix<Rows, Columns, NumericType>   NewMatrix;

    // Set rows of new matrix to scaled rows of self
    for(register size_t i = 0 ; i < Rows ; ++i)
        NewMatrix.m_Rows[i] = m_Rows[i] * pi_Factor;

    // Return recipient
    return(NewMatrix);
    }


/**----------------------------------------------------------------------------
 Produces a matrix of the same geometry as self being the scaled down version
 of self where all values are divided by the provided factor.

 @param pi_Divisor   Divisor to apply on all values in this matrix.

 @return A matrix containing the divided values.
-----------------------------------------------------------------------------*/
template<size_t Rows, size_t Columns, class NumericType>
inline HFCMatrix<Rows, Columns, NumericType>
HFCMatrix<Rows, Columns, NumericType>::operator/(NumericType pi_Divisor) const
    {
    // The divisor must be different from 0
    HPRECONDITION(pi_Divisor != 0);

    // Create recipine matrix
    HFCMatrix<Rows, Columns, NumericType>   NewMatrix;

    // Set rows of new matrix to scaled rows of self
    for(register size_t i = 0 ; i < Rows ; ++i)
        NewMatrix.m_Rows[i] = m_Rows[i] / pi_Divisor;

    // Return recipient
    return(NewMatrix);
    }



/**----------------------------------------------------------------------------
 Increments self matrix by another. The given matrix must have the
 same geometry as the self matrix. The incrementation of a matrix is
 performed by adding values from self matrix on a one to one basis:

 @c{Self[0][0] += Given[0][0]} and so on.

 @param pi_rObj  A reference to the matrix to be added.

 @return A reference to self that can be used as an l-value.
-----------------------------------------------------------------------------*/
template<size_t Rows, size_t Columns, class NumericType>
inline HFCMatrix<Rows, Columns, NumericType>&
HFCMatrix<Rows, Columns, NumericType>::operator+=(const HFCMatrix<Rows, Columns, NumericType>& pi_rObj)
    {
    // Each row is incremented by rows of same position of given matrix
    for(register size_t i = 0 ; i < Rows ; ++i)
        m_Rows[i] += pi_rObj.m_Rows[i];

    // Return reference to self
    return(*this);
    }

/**----------------------------------------------------------------------------
 Decrements self matrix by another. The given matrix must have the
 same geometry as the self matrix. The decrementation of a matrix is
 performed by substracting values from self matrix on a one to one basis:

 @c{Self[0][0] -= Given[0][0]} and so on.

 @param pi_rObj  A reference to the matrix to be substracted.

 @return A reference to self that can be used as an l-value.
-----------------------------------------------------------------------------*/
template<size_t Rows, size_t Columns, class NumericType>
inline HFCMatrix<Rows, Columns, NumericType>&
HFCMatrix<Rows, Columns, NumericType>::operator-=(const HFCMatrix<Rows, Columns, NumericType>& pi_rObj)
    {
    // Each row is decremented by rows of same position of given matrix
    for(register size_t i = 0 ; i < Rows ; ++i)
        m_Rows[i] -= pi_rObj.m_Rows[i];

    // Return reference to self
    return(*this);
    }


/**----------------------------------------------------------------------------
 Scales the matrix by a specified factor.

 @param pi_RawValue The scale factor to multiply all values with.

 @return A reference to self that can be used as an l-value.
-----------------------------------------------------------------------------*/
template<size_t Rows, size_t Columns, class NumericType>
inline HFCMatrix<Rows, Columns, NumericType>&
HFCMatrix<Rows, Columns, NumericType>::operator*=(NumericType pi_RawValue)
    {
    // Scale each row
    for(register size_t i = 0 ; i < Rows ; ++i)
        m_Rows[i] *= pi_RawValue;

    // Return reference to self
    return(*this);
    }

/**----------------------------------------------------------------------------
 Scales down the matrix by a specified divisor.

 @param pi_RawValue The divisor with which will be divided all values with.
                    This value must be different from 0.

 @return A reference to self that can be used as an l-value.
-----------------------------------------------------------------------------*/
template<size_t Rows, size_t Columns, class NumericType>
inline HFCMatrix<Rows, Columns, NumericType>&
HFCMatrix<Rows, Columns, NumericType>::operator/=(NumericType pi_RawValue)
    {
    // The divisor must be different from 0.0
    HPRECONDITION (pi_RawValue != 0);

    // Scale each row
    for(register size_t i = 0 ; i < Rows ; ++i)
        m_Rows[i] /= pi_RawValue;

    // Return reference to self
    return(*this);
    }



/**----------------------------------------------------------------------------
 Returns the tranpose matrix of self. The tranpose of a matrix is
 obtained by changing rows into columns, and columns into rows. The
 returned matrix does not have the same geometry as the self matrix. The
 number of rows becomes the number of columns, and vice-versa.

 @return The transposed version of this matrix.
-----------------------------------------------------------------------------*/
template<size_t Rows, size_t Columns, class NumericType>
HFCMatrix<Columns, Rows, NumericType>
HFCMatrix<Rows, Columns, NumericType>::CalculateTranspose() const
    {
    // Declare recipient matrix
    HFCMatrix<Columns, Rows, NumericType>   NewMatrix;

    // Declare required index counters
    register size_t i;
    register size_t j;

    // For each row of result
    for (i = 0 ; i < Columns ; ++i)
        {
        // For each column of this row
        for (j = 0 ; j < Rows ; ++j)
            {
            // Obtain and set value
            NewMatrix.m_Rows[i][j] = m_Rows[j][i];
            }
        }

    // Return result
    return(NewMatrix);
    }
END_IMAGEPP_NAMESPACE