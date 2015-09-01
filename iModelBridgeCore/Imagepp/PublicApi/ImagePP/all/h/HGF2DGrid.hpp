//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGF2DGrid.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE
// this class must be replace by HFC2DGrid

#define HGS_GLOBAL_EPSILON 0.00001

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------

inline HGF2DGrid::HGF2DGrid (const HGF2DExtent& pi_rExtent)
    : m_Extent(pi_rExtent)
    {
    HPRECONDITION(pi_rExtent.IsDefined());
    }

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
inline HGF2DGrid::HGF2DGrid (const HGF2DExtent& pi_rExtent,
                             const HFCPtr<HGF2DCoordSys>&  pi_rpCoordSys)
    : m_Extent(pi_rExtent)
    {
    HPRECONDITION(pi_rExtent.IsDefined());

    m_Extent.ChangeCoordSys(pi_rpCoordSys);
    }

//-----------------------------------------------------------------------------
// Copy Constructor
//-----------------------------------------------------------------------------
inline HGF2DGrid::HGF2DGrid(const HGF2DGrid& pi_rObj)
    : m_Extent(pi_rObj.m_Extent)
    {
    }

//-----------------------------------------------------------------------------
// The destructor.
//-----------------------------------------------------------------------------
inline HGF2DGrid::~HGF2DGrid()
    {
    }


//-----------------------------------------------------------------------------
// operator=
// Assignment operator.  It duplicates another line object.
//-----------------------------------------------------------------------------
inline HGF2DGrid& HGF2DGrid::operator=(const HGF2DGrid& pi_rObj)
    {
    // Check that object to copy is not self
    if (this != &pi_rObj)
        {
        m_Extent = pi_rObj.m_Extent;
        }

    // Return reference to self
    return (*this);
    }



//-----------------------------------------------------------------------------
// GetXMin
// Gets the minimum X value of grid
//-----------------------------------------------------------------------------
inline int32_t HGF2DGrid::GetXMin() const
    {
    return (int32_t)floor(m_Extent.GetXMin() + HGS_GLOBAL_EPSILON);
    }

//-----------------------------------------------------------------------------
// GetYMin
// Gets the minimum Y value of grid
//-----------------------------------------------------------------------------
inline int32_t HGF2DGrid::GetYMin() const
    {
    return (int32_t)floor(m_Extent.GetYMin() + HGS_GLOBAL_EPSILON);
    }

//-----------------------------------------------------------------------------
// GetXMax
// Gets the maximum X value of grid
//-----------------------------------------------------------------------------
inline int32_t HGF2DGrid::GetXMax() const
    {
    return (int32_t)floor(m_Extent.GetXMax() + HGS_GLOBAL_EPSILON) - 1;
    }

//-----------------------------------------------------------------------------
// GetYMax
// Gets the maximum Y value of grid
//-----------------------------------------------------------------------------
inline int32_t HGF2DGrid::GetYMax() const
    {
    return (int32_t)floor(m_Extent.GetYMax() + HGS_GLOBAL_EPSILON) - 1;
    }


//-----------------------------------------------------------------------------
// GetWidth
// Gets the width of the grid
//-----------------------------------------------------------------------------
inline int32_t HGF2DGrid::GetWidth() const
    {
    return GetXMax() - GetXMin() + 1;
    }

//-----------------------------------------------------------------------------
// GetHeight
// Gets the height of the grid
//-----------------------------------------------------------------------------
inline int32_t HGF2DGrid::GetHeight() const
    {
    return GetYMax() - GetYMin() + 1;
    }


//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------

inline HGF2DInclusiveGrid::HGF2DInclusiveGrid (const HGF2DExtent& pi_rExtent)
    : m_Extent(pi_rExtent)
    {
    HPRECONDITION(pi_rExtent.IsDefined());
    }

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
inline HGF2DInclusiveGrid::HGF2DInclusiveGrid (const HGF2DExtent& pi_rExtent,
                                               const HFCPtr<HGF2DCoordSys>&  pi_rpCoordSys)
    : m_Extent(pi_rExtent)
    {
    HPRECONDITION(pi_rExtent.IsDefined());

    m_Extent.ChangeCoordSys(pi_rpCoordSys);
    }

//-----------------------------------------------------------------------------
// Copy Constructor
//-----------------------------------------------------------------------------
inline HGF2DInclusiveGrid::HGF2DInclusiveGrid(const HGF2DInclusiveGrid& pi_rObj)
    : m_Extent(pi_rObj.m_Extent)
    {
    }

//-----------------------------------------------------------------------------
// The destructor.
//-----------------------------------------------------------------------------
inline HGF2DInclusiveGrid::~HGF2DInclusiveGrid()
    {
    }


//-----------------------------------------------------------------------------
// operator=
// Assignment operator.  It duplicates another line object.
//-----------------------------------------------------------------------------
inline HGF2DInclusiveGrid& HGF2DInclusiveGrid::operator=(const HGF2DInclusiveGrid& pi_rObj)
    {
    // Check that object to copy is not self
    if (this != &pi_rObj)
        {
        m_Extent = pi_rObj.m_Extent;
        }

    // Return reference to self
    return (*this);
    }



//-----------------------------------------------------------------------------
// GetXMin
// Gets the minimum X value of grid
//-----------------------------------------------------------------------------
inline int32_t HGF2DInclusiveGrid::GetXMin() const
    {
    return (int32_t)floor(m_Extent.GetXMin() + HGS_GLOBAL_EPSILON);
    }

//-----------------------------------------------------------------------------
// GetYMin
// Gets the minimum Y value of grid
//-----------------------------------------------------------------------------
inline int32_t HGF2DInclusiveGrid::GetYMin() const
    {
    return (int32_t)floor(m_Extent.GetYMin() + HGS_GLOBAL_EPSILON);
    }

//-----------------------------------------------------------------------------
// GetXMax
// Gets the maximum X value of grid
//-----------------------------------------------------------------------------
inline int32_t HGF2DInclusiveGrid::GetXMax() const
    {
    return (int32_t)floor(m_Extent.GetXMax() - HGS_GLOBAL_EPSILON);
    }

//-----------------------------------------------------------------------------
// GetYMax
// Gets the maximum Y value of grid
//-----------------------------------------------------------------------------
inline int32_t HGF2DInclusiveGrid::GetYMax() const
    {
    return (int32_t)floor(m_Extent.GetYMax() - HGS_GLOBAL_EPSILON);
    }


//-----------------------------------------------------------------------------
// GetWidth
// Gets the width of the grid
//-----------------------------------------------------------------------------
inline int32_t HGF2DInclusiveGrid::GetWidth() const
    {
    return GetXMax() - GetXMin() + 1;
    }

//-----------------------------------------------------------------------------
// GetHeight
// Gets the height of the grid
//-----------------------------------------------------------------------------
inline int32_t HGF2DInclusiveGrid::GetHeight() const
    {
    return GetYMax() - GetYMin() + 1;
    }


END_IMAGEPP_NAMESPACE