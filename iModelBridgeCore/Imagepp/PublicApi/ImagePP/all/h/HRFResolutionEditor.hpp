//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFResolutionEditor.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Inline methods for class HRFResolutionEditor
//-----------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE
//-----------------------------------------------------------------------------
// public
// GetFileRaster
// Allow to obtain the associated raster file
//-----------------------------------------------------------------------------
inline HFCPtr<HRFRasterFile> HRFResolutionEditor::GetRasterFile()
    {
    return (m_pRasterFile);
    }

//-----------------------------------------------------------------------------
// Public
// GetResolutionDescriptor
// Resolution information
//-----------------------------------------------------------------------------
inline const HFCPtr<HRFResolutionDescriptor>&  HRFResolutionEditor::GetResolutionDescriptor() const
    {
    return m_pResolutionDescriptor;
    }

//-----------------------------------------------------------------------------
// Public
// GetResolutionCapabilities
// Resolution information
//-----------------------------------------------------------------------------
inline const HFCPtr<HRFRasterFileCapabilities>&  HRFResolutionEditor::GetResolutionCapabilities() const
    {
    return m_pResolutionCapabilities;
    }

//-----------------------------------------------------------------------------
// public
// GetFileRaster
// Palette information
//-----------------------------------------------------------------------------
inline void HRFResolutionEditor::SetPalette(const HRPPixelPalette& pi_rPalette)
    {
    HRPPixelPalette& rPalette = m_pResolutionDescriptor->GetPixelType()->LockPalette();

    HASSERT(pi_rPalette.GetChannelOrg() == rPalette.GetChannelOrg());

    // Lock the palette and unlock it after the replace value
    rPalette = pi_rPalette;
    m_pResolutionDescriptor->GetPixelType()->UnlockPalette();
    m_pResolutionDescriptor->PaletteChanged();
    }

//-----------------------------------------------------------------------------
// public
// GetFileRaster
// Palette information
//-----------------------------------------------------------------------------
inline const HRPPixelPalette& HRFResolutionEditor::GetPalette() const
    {
    return m_pResolutionDescriptor->GetPixelType()->GetPalette();
    }

//-----------------------------------------------------------------------------
// public
// NoMoreRead
//
// This method is used by the cache. It's called when the resolution is cached.
// Note that a ReadBlock() can be called after a call to NoMoreRead.() This
// method was implemented to deallocate working buffer in HRFAdaptor
//-----------------------------------------------------------------------------
inline void HRFResolutionEditor::NoMoreRead()
    {
    }

//-----------------------------------------------------------------------------
// public
// GetResolutionIndex
//
// This method return the resolution index. This method is use to know which
// resolution index is use on HGFTileID when HRFInternetImagingFile is an
// unlimited resolution.
//-----------------------------------------------------------------------------
inline unsigned short HRFResolutionEditor::GetResolutionIndex() const
    {
    return m_Resolution;
    }

//-----------------------------------------------------------------------------
// public
// GetPage
//
// This method return the page index.
//-----------------------------------------------------------------------------
inline uint32_t HRFResolutionEditor::GetPage() const
    {
    return m_Page;
    }
END_IMAGEPP_NAMESPACE
