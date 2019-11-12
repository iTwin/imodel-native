//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------
// Class HGSMemoryBaseSurfaceDescriptor
//---------------------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE
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

END_IMAGEPP_NAMESPACE