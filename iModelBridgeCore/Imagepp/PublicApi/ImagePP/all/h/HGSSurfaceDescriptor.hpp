//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGSSurfaceDescriptor.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HGSSurfaceDescriptor
//-----------------------------------------------------------------------------
// General class for surfaces.
//-----------------------------------------------------------------------------
BEGIN_IMAGEPP_NAMESPACE
//-----------------------------------------------------------------------------
// public
// SetDimensions
//-----------------------------------------------------------------------------
inline void HGSSurfaceDescriptor::SetDimensions(uint32_t pi_Width, uint32_t pi_Height)
    {
    m_Width  =  pi_Width;
    m_Height = pi_Height;
    }

//-----------------------------------------------------------------------------
// public
// GetWidth
//-----------------------------------------------------------------------------
inline uint32_t HGSSurfaceDescriptor::GetWidth() const
    {
    return m_Width;
    }

//-----------------------------------------------------------------------------
// public
// GetHeight
//-----------------------------------------------------------------------------
inline uint32_t HGSSurfaceDescriptor::GetHeight() const
    {
    return m_Height;
    }


//-----------------------------------------------------------------------------
// public
// SetDataDimensions
//-----------------------------------------------------------------------------
inline void HGSSurfaceDescriptor::SetDataDimensions(uint32_t pi_Width, uint32_t pi_Height)
    {
    m_DataWidth  =  pi_Width;
    m_DataHeight = pi_Height;
    }

//-----------------------------------------------------------------------------
// public
// GetDataWidth
//-----------------------------------------------------------------------------
inline uint32_t HGSSurfaceDescriptor::GetDataWidth() const
    {
    return m_DataWidth;
    }

//-----------------------------------------------------------------------------
// public
// GetDataHeight
//-----------------------------------------------------------------------------
inline uint32_t HGSSurfaceDescriptor::GetDataHeight() const
    {
    return m_DataHeight;
    }


//-----------------------------------------------------------------------------
// public
// SetOffsets
//-----------------------------------------------------------------------------
inline void HGSSurfaceDescriptor::SetOffsets(HUINTX pi_OffsetX,
                                             HUINTX pi_OffsetY)
    {
    m_OffsetX = pi_OffsetX;
    m_OffsetY = pi_OffsetY;
    }

//-----------------------------------------------------------------------------
// public
// GetOffsets
//-----------------------------------------------------------------------------
inline void HGSSurfaceDescriptor::GetOffsets(HUINTX* po_pOffsetX,
                                             HUINTX* po_pOffsetY) const
    {
    *po_pOffsetX = m_OffsetX;
    *po_pOffsetY = m_OffsetY;
    }

END_IMAGEPP_NAMESPACE