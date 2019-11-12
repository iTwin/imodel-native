/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

/*__BENTLEY_INTERNAL_ONLY__*/

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/// <summary>Matrix (of doubles) stored packed in row major order.</summary>
struct RowMajorMatrix : bvector<double>
{
private:
size_t m_numRows;
size_t m_numColumns;
public:
GEOMDLLIMPEXP RowMajorMatrix (size_t numRows, size_t numColumns, double *data = NULL, bool transposeData = false);
GEOMDLLIMPEXP RowMajorMatrix ();
GEOMDLLIMPEXP void SetSizes (size_t numRows, size_t numColumns, double *data = NULL, bool transposeData = false);
size_t NumRows () const { return m_numRows;}
size_t NumColumns () const { return m_numColumns;}
size_t Index (size_t i, size_t j) const { return i * m_numColumns + j;}
double const &At (size_t i, size_t j) const { return at(i * m_numColumns + j);}
double const &AtCyclic (size_t i, size_t j) const { return at((i % m_numRows) * m_numColumns + (j % m_numColumns));}

double &At (size_t i, size_t j) { return at(i * m_numColumns + j);}
double &AtCyclic (size_t i, size_t j) { return at((i % m_numRows) * m_numColumns + (j % m_numColumns));}
bool IsSquare () const { return m_numRows == m_numColumns;}
GEOMDLLIMPEXP void SwapColumns (size_t i, size_t j);
GEOMDLLIMPEXP void SwapRows (size_t i, size_t j);
/// <summary>Swap columns of this matrix and of a partner column index vector</summary>
GEOMDLLIMPEXP void SwapColumns (size_t i, size_t j, bvector<size_t> &index);
/// <summary>return the max absolute entry in the column from a pivot downwards.</summary>
GEOMDLLIMPEXP double MaxAbsBelowPivot (size_t pivot, size_t &i) const;
/// <summary>return the max absolute entry in the trailing block from a diagonal entry forward.</summary>
GEOMDLLIMPEXP double MaxAbsBeyondPivot (size_t pivot, size_t &i, size_t &j) const;
/// <summary>set all entries to zero</summary>
GEOMDLLIMPEXP void SetZero ();
/// <summary>diagonals to 1, all others zero</summary>
GEOMDLLIMPEXP void SetIdentity ();
/// <summary>Apply elimination step from given pivot.   Elimination is applied in the trailing block and the rhs.</summary>
/// <returns>false if unable to divide by pivot. This is an untoleranced test</returns>
/// <remarks> In the pivot row, entries in columns beyond the pivot are divided by the pivot.</remarks>
/// <remarks> The pivot itself is unaltered.</remarks>
/// <remarks> The pivot itself is unaltered.</remarks>
GEOMDLLIMPEXP bool BlockElimination (size_t pivot, RowMajorMatrix &rhs);

/// <summary>Apply back substitution. This assumes the matrix and rhs were both updated by the BlockElimination method.</summary>
/// <remarks>The BlockElimination method must be applied up to and including the final pivot.</summary>
/// <remarks>The BlockElimination step leaves implied 1's on the diagonal.  Hence back substitution is just multiply and add -- no failure possibility</remarks>
GEOMDLLIMPEXP void BackSubstitute (RowMajorMatrix &rhs);
/// <summary>Multiply the matrix time vector x, return in y</summary>
GEOMDLLIMPEXP void Multiply (bvector<double> const &x, bvector<double> &y) const;

/// <summary>Add s*x[i]*y[j] to each entry i,j </summary>
GEOMDLLIMPEXP void AddScaledOuterProductInPlace (double s, bvector<double> const &x, bvector<double> const&y);

/// <summary>Store into a column</summary>
GEOMDLLIMPEXP void SetColumn (size_t j, bvector<double> const &x);
/// <summary>Store into a row</summary>
GEOMDLLIMPEXP void SetRow (size_t i, bvector<double> const &x);

/// <summary>Store into a checked position.  Any out-of-bounds index is skipped.</summary>
/// <returns>Number of in-bounds indices, i.e. 0 or 1</returns>
GEOMDLLIMPEXP size_t Set (ptrdiff_t i, ptrdiff_t j, double value);
/// <summary>Access from a checked position.  If index is out of bounds a zero is returned..</summary>
/// <returns>Number of in-bounds indices, i.e. 0 or 1</returns>
GEOMDLLIMPEXP size_t Get (ptrdiff_t i, ptrdiff_t j, double &value);

/// <summary>Store into consecutive places in a row.  Any out-of-bounds index is skipped.</summary>
/// <returns>Number of in-bounds indices</returns>
GEOMDLLIMPEXP size_t SetInRow (ptrdiff_t i, ptrdiff_t j, double value0, double value1);
/// <summary>Store into consecutive places in a row.  Any out-of-bounds index is skipped.</summary>
/// <returns>Number of in-bounds indices</returns>
GEOMDLLIMPEXP size_t SetInRow (ptrdiff_t i, ptrdiff_t j, double value0, double value1, double value2);

/// <summary>Store x,y,z into consecutive places in a row.  Any out-of-bounds index is skipped.</summary>
/// <returns>Number of in-bounds indices</returns>
GEOMDLLIMPEXP size_t SetInRow (ptrdiff_t i, ptrdiff_t j, DPoint3dCR data);

/// <summary>Store doubles into consecutive places in a row.  Any out-of-bounds index is skipped.</summary>
/// <returns>Number of in-bounds indices</returns>
GEOMDLLIMPEXP size_t SetInRow (ptrdiff_t i, ptrdiff_t j, double const *data, size_t num);
/// <summary>Execute Get at consecutive places in a row.  Outputs at out-of-bounds indices are set to zero.</summary>
/// <returns>Number of entries actually copied.  All others others are set to zero -- every entry of data[] is touched even if it maps out of bounds.</returns>
GEOMDLLIMPEXP size_t GetFromRow (ptrdiff_t i, ptrdiff_t j, double *data, size_t maxReturn);

/// <summary>Execute Get at consecutive places in a row.  Outputs at out-of-bounds indices are set to zero.</summary>
/// <returns>Number of coordinates (0,1,2,3) actually copied.  Others are set to zero.</returns>
GEOMDLLIMPEXP size_t GetFromRow (ptrdiff_t i, ptrdiff_t j, DPoint3dR xyz);

/// <summary>Read from a column</summary>
GEOMDLLIMPEXP void GetColumn (size_t i, bvector<double> &x) const;

/// <summary>Copy the entire matrix into the caller's buffer.</summary>
/// <returns>number of items copied.</returns>
GEOMDLLIMPEXP size_t GetAll (double * data, size_t maxCount, bool transpose = false) const;


static GEOMDLLIMPEXP void Print (RowMajorMatrix const &matrix);
};

struct LinearAlgebra
{
private:
LinearAlgebra ();
public:
/// <summary>solve Ax=b with x replacing b and A destroyed.</summary>
static GEOMDLLIMPEXP bool SolveInplaceGaussPartialPivot (RowMajorMatrix &matrix, RowMajorMatrix &rhs);
/// <summary>Invert a matrix.</summary>
static GEOMDLLIMPEXP bool InvertFullSwap (RowMajorMatrix const &matrix, RowMajorMatrix &inverse);

/// <summary>solve Ax=b with x replacing b and A destroyed.</summary>
static GEOMDLLIMPEXP bool SolveInplaceGaussFullPivot (RowMajorMatrix &matrix, RowMajorMatrix &rhs, double &condition);

};



END_BENTLEY_GEOMETRY_NAMESPACE

