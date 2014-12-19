//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGSMemoryBaseSurfaceDescriptor.hpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------
// Class HGSMemoryBaseSurfaceDescriptor
//---------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// public
// GetSLO
//-----------------------------------------------------------------------------
inline HGFSLO HGSMemoryBaseSurfaceDescriptor::GetSLO() const
    {
    return m_SLO;
    }


//-----------------------------------------------------------------------------
// public
// GetBytesPerRow
//-----------------------------------------------------------------------------
inline uint32_t HGSMemoryBaseSurfaceDescriptor::GetBytesPerRow() const
    {
    return m_BytesPerRow;
    }

