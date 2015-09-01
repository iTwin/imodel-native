//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGFMappedSurface.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HGFMappedSurface
//-----------------------------------------------------------------------------
// Inline methods
//-----------------------------------------------------------------------------
BEGIN_IMAGEPP_NAMESPACE
//-----------------------------------------------------------------------------
// GetCoordSys
//-----------------------------------------------------------------------------
inline HFCPtr<HGF2DCoordSys> HGFMappedSurface::GetCoordSys() const
    {
    return m_pCoordSysContainer->GetCoordSys();
    }

//-----------------------------------------------------------------------------
// GetDeviceCoordSys
//-----------------------------------------------------------------------------
inline HFCPtr<HGF2DCoordSys> HGFMappedSurface::GetSurfaceCoordSys() const
    {
    return m_pSurfaceCoordSys;
    }

//-----------------------------------------------------------------------------
// GetCoordSysContainer
//-----------------------------------------------------------------------------
inline HFCPtr<HGFCoordSysContainer> HGFMappedSurface::GetCoordSysContainer() const
    {
    return m_pCoordSysContainer;
    }

//-----------------------------------------------------------------------------
// GetCoordSysForSLO
//-----------------------------------------------------------------------------
inline HFCPtr<HGF2DCoordSys> HGFMappedSurface::GetCoordSysForSLO() const
    {
    return m_pCoordSysForSLO;
    }

//-----------------------------------------------------------------------------
// GetTransfoModelToRef
//-----------------------------------------------------------------------------
inline HFCPtr<HGF2DTransfoModel> HGFMappedSurface::GetTransfoModelToRef() const
    {
    return GetCoordSys()->GetTransfoModelTo(GetCoordSys()->GetReference());
    }

//-----------------------------------------------------------------------------
// GetXOffset
//-----------------------------------------------------------------------------
inline uint32_t HGFMappedSurface::GetXOffset() const
    {
    return m_OffsetX;
    }

//-----------------------------------------------------------------------------
// GetYOffset
//-----------------------------------------------------------------------------
inline uint32_t HGFMappedSurface::GetYOffset() const
    {
    return m_OffsetY;
    }

//-----------------------------------------------------------------------------
// GetWidth
//-----------------------------------------------------------------------------
inline uint32_t HGFMappedSurface::GetWidth() const
    {
    return m_Width;
    }

//-----------------------------------------------------------------------------
// GetHeight
//-----------------------------------------------------------------------------
inline uint32_t HGFMappedSurface::GetHeight() const
    {
    return m_Height;
    }

END_IMAGEPP_NAMESPACE