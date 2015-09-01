//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hgf/src/HGFMatrixOps.cpp $
//:>
//:>  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------


#include "hstdcpp.h"
#include "HGFMatrixOps.h"



/** -----------------------------------------------------------------------------
    Inverses the matrix

    @param pi_rMatrix  The 4 by 4 matrix to obtain inverse of.

    @return A new 4 by 4 matrix containing the inverted matrix.
    -----------------------------------------------------------------------------
*/
HFCMatrix<4, 4> InverseMatrix(const HFCMatrix<4, 4>& pi_rMatrix)
    {

    // Compute determinant
    double Det = CalculateDeterminant(pi_rMatrix);

    HASSERT(Det != 0.0);


    // Calculate transpose
    HFCMatrix<4, 4> ResMatrix(pi_rMatrix.CalculateTranspose());

    // Obtain inverse
    ResMatrix /= Det;

    return(ResMatrix);
    }


/** -----------------------------------------------------------------------------
    CalculatesDeterminant

    @param pi_rMatrix  The 4 by 4 matrix to obtain determinant of

    @return the determinant of the matrix
    -----------------------------------------------------------------------------
*/
double CalculateDeterminant(const HFCMatrix<4, 4, double>& pi_rMatrix)
    {
    double ResDet = 0;
    size_t Column;

    // for every column
    for (Column = 0 ; Column < 4 ; ++Column)
        {
        // Extract sub-matrix
        HFCMatrix<3, 3> TempSubMatrix = ExtractSubMatrix(pi_rMatrix, 0, Column);

        // Compute sign (either added or subtracted)
        int Sign = (((Column % 2) == 0) ? 1 : -1);

        // Add to previous result
        ResDet += Sign * pi_rMatrix[0][Column] * CalculateDeterminant(TempSubMatrix);
        }

    return(ResDet);
    }


/** -----------------------------------------------------------------------------
    CalculatesDeterminant

    @param pi_rMatrix  The 3 by 3 matrix to obtain determinant of

    @return the determinant of the matrix
    -----------------------------------------------------------------------------
*/
double CalculateDeterminant(const HFCMatrix<3, 3, double>& pi_rMatrix)
    {
    double ResDet = 0;
    size_t Column;

    // For every column
    for (Column = 0 ; Column < 3 ; ++Column)
        {
        // Extract sub-matrix
        HFCMatrix<2, 2> TempSubMatrix = ExtractSubMatrix(pi_rMatrix, 0, Column);

        // Compute sign (either added or subtracted)
        int Sign = (((Column % 2) == 0) ? 1 : -1);

        // Add to previous result
        ResDet += Sign * pi_rMatrix[0][Column] * CalculateDeterminant(TempSubMatrix);
        }

    return(ResDet);
    }




/** -----------------------------------------------------------------------------
    CalculatesDeterminant

    @param pi_rMatrix  The 2 by 2 matrix to obtain determinant of

    @return the determinant of the matrix
    -----------------------------------------------------------------------------
*/
double CalculateDeterminant(const HFCMatrix<2, 2>& pi_rMatrix)
    {
    return(pi_rMatrix[0][0] * pi_rMatrix[1][1] - pi_rMatrix[0][1] * pi_rMatrix[1][0]);
    }




/** -----------------------------------------------------------------------------
    Extracts the sub matrix by ignoring indicated row and column

    @param pi_rMatrix  The 4 by 4 matrix to obtain sub matrix of.

    @param pi_Column The column to disregard in result

    @param pi_Row The row to disregard in result

    @return A new 3 by 3 matrix containing the sub matrix
    -----------------------------------------------------------------------------
*/
HFCMatrix<3, 3> ExtractSubMatrix(const HFCMatrix<4, 4>& pi_rMatrix, size_t pi_Row, size_t pi_Column)
    {
    // The column and row to ignore must exist.
    HPRECONDITION(pi_Column >= 0);
    HPRECONDITION(pi_Row >= 0);
    HPRECONDITION(pi_Column < 4);
    HPRECONDITION(pi_Row < 4);


    HFCMatrix<3, 3> ResMatrix;
    size_t RowSrc;
    size_t ColumnSrc;
    size_t RowDst = 0;
    size_t ColumnDst = 0;


    // for every row
    for (RowSrc = 0 ; RowSrc < 4 ; ++RowSrc)
        {
        // Check if this row must be taken
        if (RowSrc != pi_Row)
            {
            ColumnDst = 0;

            // For every row of source
            for (ColumnSrc = 0 ; ColumnSrc < 4 ; ++ColumnSrc)
                {
                if (ColumnSrc != pi_Column)
                    {
                    ResMatrix[RowDst][ColumnDst] = pi_rMatrix[RowSrc][ColumnSrc];

                    // Increment next column number
                    ++ColumnDst;

                    }
                }

            // Increment next row number
            ++RowDst;

            }
        }

    return(ResMatrix);

    }


/** -----------------------------------------------------------------------------
    Extracts the sub matrix by ignoring indicated row and column

    @param pi_rMatrix  The 3 by 3 matrix to obtain sub matrix of.

    @param pi_Column The column to disregard in result

    @param pi_Row The row to disregard in result

    @return A new 2 by 2 matrix containing the sub matrix
    -----------------------------------------------------------------------------
*/
HFCMatrix<2, 2> ExtractSubMatrix(const HFCMatrix<3, 3>& pi_rMatrix, size_t pi_Row, size_t pi_Column)
    {
    // The column and row to ignore must exist.
    HPRECONDITION(pi_Column >= 0);
    HPRECONDITION(pi_Row >= 0);
    HPRECONDITION(pi_Column < 3);
    HPRECONDITION(pi_Row < 3);


    HFCMatrix<2, 2> ResMatrix;
    size_t RowSrc;
    size_t ColumnSrc;
    size_t RowDst = 0;
    size_t ColumnDst = 0;


    // for every row
    for (RowSrc = 0 ; RowSrc < 3 ; ++RowSrc)
        {
        // Check if this row must be taken
        if (RowSrc != pi_Row)
            {
            ColumnDst = 0;

            // For every row of source
            for (ColumnSrc = 0 ; ColumnSrc < 3 ; ++ColumnSrc)
                {
                if (ColumnSrc != pi_Column)
                    {
                    ResMatrix[RowDst][ColumnDst] = pi_rMatrix[RowSrc][ColumnSrc];

                    // Increment next column number
                    ++ColumnDst;
                    }
                }

            // Increment next row number
            ++RowDst;

            }
        }

    return(ResMatrix);

    }




/** -----------------------------------------------------------------------------
    Extracts the sub matrix by ignoring indicated row and column

    @param pi_rMatrix  The 4 by 4 matrix to obtain sub matrix of.

    @param pi_Column The column to disregard in result

    @param pi_Row The row to disregard in result

    @return A new 3 by 3 matrix containing the sub matrix
    -----------------------------------------------------------------------------
*/
HFCMatrix<1, 1> ExtractSubMatrix(const HFCMatrix<2, 2>& pi_rMatrix, size_t pi_Row, size_t pi_Column)
    {
    // The column and row to ignore must exist.
    HPRECONDITION(pi_Column >= 0);
    HPRECONDITION(pi_Row >= 0);
    HPRECONDITION(pi_Column < 2);
    HPRECONDITION(pi_Row < 2);


    HFCMatrix<1, 1> ResMatrix;

    ResMatrix[0][0] = pi_rMatrix[(pi_Row == 0 ? 1 : 0)][(pi_Column == 0 ? 1 : 0)];

    return(ResMatrix);

    }





