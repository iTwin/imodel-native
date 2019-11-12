//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Inline methods for class HGFSpatialCriteria
//-----------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
inline HGFSpatialCriteria::HGFSpatialCriteria(const HGF2DExtent& pi_rRegion)
    : m_Region(pi_rRegion)
    {
    }


//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
inline HGFSpatialCriteria::~HGFSpatialCriteria()
    {
    }


/** -----------------------------------------------------------------------------
    Gain access to the extent object stored inside the criteria.

    @return A reference to the internal HGF2Dextent object.

    Example:
    @code
    @end

    -----------------------------------------------------------------------------
*/
inline HGF2DExtent& HGFSpatialCriteria::GetRegion()
    {
    return m_Region;
    }

END_IMAGEPP_NAMESPACE