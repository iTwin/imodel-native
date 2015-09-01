//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGFSpatialCriteria.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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