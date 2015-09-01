//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGFHVVHSpatialAttribute.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Inline methods for class HGFHVVHSpatialAttribute
//-----------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE
//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
inline HGFHVVHSpatialAttribute::HGFHVVHSpatialAttribute(void* pi_pNode, const HGF2DExtent& pi_rExtent)
    : m_Extent(pi_rExtent)
    {
    // m_pNode can be 0...
    m_pNode = pi_pNode;
    }


//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
inline HGFHVVHSpatialAttribute::~HGFHVVHSpatialAttribute()
    {
    }


//-----------------------------------------------------------------------------
// Retrieve the node pointer.
//-----------------------------------------------------------------------------
inline void* HGFHVVHSpatialAttribute::GetNode() const
    {
    // m_pNode can be 0...
    return m_pNode;
    }


//-----------------------------------------------------------------------------
// Set the node pointer.
//-----------------------------------------------------------------------------
inline void HGFHVVHSpatialAttribute::SetNode(void* pi_pNode)
    {
    // m_pNode can be 0...
    m_pNode = pi_pNode;
    }


//-----------------------------------------------------------------------------
// Set the extent
//-----------------------------------------------------------------------------
inline void HGFHVVHSpatialAttribute::SetExtent(const HGF2DExtent& pi_rExtent)
    {
    m_Extent = pi_rExtent;
    }


//-----------------------------------------------------------------------------
// Retrieve the extent
//-----------------------------------------------------------------------------
inline HGF2DExtent& HGFHVVHSpatialAttribute::GetExtent() const
    {
    return m_Extent;
    }

END_IMAGEPP_NAMESPACE