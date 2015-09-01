/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ImagePP/all/h/HGFMatrixOps.hpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE


// For some reason, the compiler does not compute
#if (0)
/** -----------------------------------------------------------------------------
    Inverses the matrix

    @param pi_rMatrix  The 4 by 4 matrix to obtain inverse of.

    @return A new 4 by 4 matrix containing the inverted matrix.
    -----------------------------------------------------------------------------
*/
template<size_t Rows, class NumericType> HFCMatrix<Rows, Rows, NumericType> InverseMatrix(const HFCMatrix<Rows, Rows, NumericType>& pi_rMatrix)
    {

    // Compute determinant
    double Det = CalculateDeterminant(pi_rMatrix);

    HASSERT(Det != 0.0);


    // Calculate transpose
    HFCMatrix<Rows, Rows, NumericType> ResMatrix(pi_rMatrix.CalculateTranspose());

    // Obtain inverse
    ResMatrix /= Det;

    return(ResMatrix);
    }


/** -----------------------------------------------------------------------------
    Inverses the matrix

    @param pi_rMatrix  The 4 by 4 matrix to obtain inverse of.

    @return A new 4 by 4 matrix containing the inverted matrix.
    -----------------------------------------------------------------------------
*/
template<class NumericType> HFCMatrix<4, 4, NumericType> InverseMatrix(const HFCMatrix<4, 4, NumericType>& pi_rMatrix)
    {

    // Compute determinant
    double Det = CalculateDeterminant(pi_rMatrix);

    HASSERT(Det != 0.0);

    // Calculate transpose
    HFCMatrix<4, 4, NumericType> ResMatrix(pi_rMatrix.CalculateTranspose());

    // Obtain inverse
    ResMatrix /= Det;

    return(ResMatrix);
    }

#endif




#if (0)
/** -----------------------------------------------------------------------------
    CalculatesDeterminant

    @param pi_rMatrix  The 4 by 4 matrix to obtain determinant of

    @return the determinant of the matrix
    -----------------------------------------------------------------------------
*/
template<size_t Rows, class NumericType> CalculateDeterminant(const HFCMatrix<Rows, Rows, NumericType>& pi_rMatrix)
    {
    double ResDet = 0;
    size_t Column;

    // For every row
    for (Column = 0 ; Column < Rows ; ++Column)
        {
        // Extract sub-matrix
        HFCMatrix<Rows - 1, Rows - 1> TempSubMatrix = ExtractSubMatrix(pi_rMatrix, 0, Column);

        // Compute sign (either added or subtracted)
        int Sign = (((Column % 2) == 0) ? 1 : -1);

        // Add to previous result
        ResDet += Sign * pi_rMatrix[0][Column] * CalculateDeterminant(TempSubMatrix);
        }

    }

#endif




#if (0)
/** -----------------------------------------------------------------------------
    Extracts the sub matrix by ignoring indicated row and column

    @param pi_rMatrix  The 4 by 4 matrix to obtain sub matrix of.

    @param pi_Column The column to disregard in result

    @param pi_Row The row to disregard in result

    @return A new 3 by 3 matrix containing the sub matrix
    -----------------------------------------------------------------------------
*/
template<size_t Rows, class NumericType> HFCMatrix<Rows - 1, Rows - 1, NumericType> ExtractSubMatrix(const HFCMatrix<Rows, Rows, NumericType>& pi_rMatrix, size_t pi_Row, size_t pi_Column)
    {
    // The column and row to ignore must exist.
    HPRECONDITION(pi_Column >= 0);
    HPRECONDITION(pi_Row >= 0);
    HPRECONDITION(pi_Column < Rows);
    HPRECONDITION(pi_Row < Rows);


    HFCMatrix<Rows - 1, Rows - 1> ResMatrix;
    size_t RowSrc;
    size_t ColumnSrc;
    size_t RowDst = 0;
    size_t ColumnDst = 0;


    // for every row
    for (RowSrc = 0 ; RowSrc < Rows ; ++RowSrc)
        {
        // Check if this row must be taken
        if (RowSrc != pi_Row)
            {
            // For every row of source
            for (ColumnSrc = 0 ; ColumnSrc < Rows ; ++ColumnSrc)
                {
                if (ColumnSrc != pi_Column)
                    {
                    ResMatrix[RowDst][ColumnDst] = pi_rMatrix[RowSrc][ColumnSrc];

                    // This row must be taken
                    ++ColumnDst;

                    }
                }
            // This row must be taken
            ++RowDst;

            }
        }

    return(ResMatrix);

    }

#endif




END_IMAGEPP_NAMESPACE