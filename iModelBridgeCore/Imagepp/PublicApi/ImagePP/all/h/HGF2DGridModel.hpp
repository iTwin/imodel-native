//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGF2DGridModel.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE
//-----------------------------------------------------------------------------
// GetDirectGridWidth
//-----------------------------------------------------------------------------
inline double HGF2DGridModel::GetDirectGridWidth() const
    {
    return m_directExtent.GetWidth() / m_nColumns;
    }

//-----------------------------------------------------------------------------
// GetDirectGridHeight
//-----------------------------------------------------------------------------
inline double HGF2DGridModel::GetDirectGridHeight() const
    {
    return m_directExtent.GetHeight() / m_nRows;
    }

//-----------------------------------------------------------------------------
// GetNumberOfColumns
//-----------------------------------------------------------------------------
inline uint32_t HGF2DGridModel::GetNumberOfColumns() const
    {
    return m_nColumns;
    }

//-----------------------------------------------------------------------------
// GetNumberOfRows
//-----------------------------------------------------------------------------
inline uint32_t HGF2DGridModel::GetNumberOfRows() const
    {
    return m_nRows;
    }
//-----------------------------------------------------------------------------
// ComputeRowColumnFromDirectCoordinate
//-----------------------------------------------------------------------------
inline void HGF2DGridModel::ComputeRowColumnFromDirectCoordinate (uint32_t* po_row, uint32_t* po_column, double pi_x, double pi_y) const
    {
    HASSERT(NULL != po_row);
    HASSERT(NULL != po_column);
    HASSERT(m_directExtent.IsPointOutterIn(HGF2DPosition(pi_x, pi_y)));

    // Calculate the row and column specific to coordinate
    *po_row    = (int32_t)((pi_y - m_directExtent.GetYMin()) / GetDirectGridHeight());
    *po_column = (int32_t)((pi_x - m_directExtent.GetXMin()) / GetDirectGridWidth());

    // If the input point is on the extent, take the last model of the grid
    if (*po_row >= m_nRows)
        (*po_row)--;
    if (*po_column >= m_nColumns)
        (*po_column)--;

    HASSERT(*po_row >= 0 && *po_row < m_nRows);
    HASSERT(*po_column >= 0 && *po_column < m_nColumns);
    }

//-----------------------------------------------------------------------------
// ComputeModelIndexFromRowColumn
//-----------------------------------------------------------------------------
inline uint32_t HGF2DGridModel::ComputeModelIndexFromRowColumn (uint32_t pi_row, uint32_t pi_column) const
    {
    HPRECONDITION(pi_row < m_nRows && pi_column < m_nColumns);
    HPRECONDITION(pi_row >= 0 && pi_column >= 0);

    return pi_row * m_nColumns + pi_column;
    }
END_IMAGEPP_NAMESPACE