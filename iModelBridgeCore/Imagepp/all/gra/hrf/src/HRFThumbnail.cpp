//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFThumbnail.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRFThumbnail
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>
 // Must be the first include.
#include <Imagepp/all/h/HRFThumbnail.h>
#include <Imagepp/all/h/HRPPixelType.h>


//-----------------------------------------------------------------------------
// Public
// Default constructor.
//-----------------------------------------------------------------------------
HRFThumbnail::HRFThumbnail()
    {
    }

//-----------------------------------------------------------------------------
// public
// Constructor
// Thumbnail Creation
//-----------------------------------------------------------------------------
HRFThumbnail::HRFThumbnail(uint32_t                     pi_Width,
                                  uint32_t                     pi_Height,
                                  const HFCPtr<HRPPixelType>&  pi_rpPixelType,
                                  const Byte*                 pi_pData,
                                  HFCAccessMode                pi_AccessMode,
                                  bool                        pi_IsComposed,
                                  unsigned short              pi_BitsAlignment)
    {
    // Storage information
    m_pPixelType    = pi_rpPixelType;
    m_IsComposed    = pi_IsComposed;
    m_AccessMode    = pi_AccessMode;

    // The Thumbnail size
    m_Width         = pi_Width;
    m_Height        = pi_Height;
    m_BitsAlignment = pi_BitsAlignment;
    m_BytesPerWidth = ((m_pPixelType->CountPixelRawDataBits() * m_Width) + (m_BitsAlignment-1)) / m_BitsAlignment * m_BitsAlignment / 8;
    m_SizeInBytes   = m_BytesPerWidth * m_Height;

    m_pData = new Byte[m_SizeInBytes];
    memcpy(m_pData, pi_pData, m_SizeInBytes);
    m_IsDataChanged    = false;
    m_IsPaletteChanged = false;
    }

//-----------------------------------------------------------------------------
// public
// Copy Constructor
// Thumbnail Creation
//-----------------------------------------------------------------------------
HRFThumbnail::HRFThumbnail(const HRFThumbnail& pi_rObj)
    {
    m_IsComposed =  pi_rObj.m_IsComposed;
    // Storage information
    m_pPixelType = pi_rObj.m_pPixelType;

    // The Thumbnail size
    m_Width             = pi_rObj.m_Width;
    m_Height            = pi_rObj.m_Height;
    m_SizeInBytes       = pi_rObj.m_SizeInBytes;
    m_BitsAlignment     = pi_rObj.m_BitsAlignment;
    m_BytesPerWidth     = pi_rObj.m_BytesPerWidth;
    m_AccessMode        = pi_rObj.m_AccessMode;
    m_IsDataChanged     = pi_rObj.m_IsDataChanged;
    m_IsPaletteChanged  = pi_rObj.m_IsPaletteChanged;

    m_pData = new Byte[m_SizeInBytes];
    memcpy(m_pData, pi_rObj.m_pData, m_SizeInBytes);
    }

//-----------------------------------------------------------------------------
// public
// Destructor
// Thumbnail Destruction
//-----------------------------------------------------------------------------
HRFThumbnail::~HRFThumbnail()
    {
    }

//-----------------------------------------------------------------------------
// public
// GetWidth
// Thumbnail size
//-----------------------------------------------------------------------------
uint32_t HRFThumbnail::GetWidth() const
    {
    return m_Width;
    }

//-----------------------------------------------------------------------------
// public
// GetHeight
// Thumbnail size
//-----------------------------------------------------------------------------
uint32_t HRFThumbnail::GetHeight() const
    {
    return m_Height;
    }

//-----------------------------------------------------------------------------
// public
// Return the size of the data.
//-----------------------------------------------------------------------------
unsigned short HRFThumbnail::GetBitsAlignment() const
    {
    return m_BitsAlignment;
    }

//-----------------------------------------------------------------------------
// public
// Return the size of the data.
//-----------------------------------------------------------------------------
size_t HRFThumbnail::GetBytesPerWidth() const
    {
    return m_BytesPerWidth;
    }

//-----------------------------------------------------------------------------
// public
// Return the size of the data.
//-----------------------------------------------------------------------------
size_t HRFThumbnail::GetSizeInBytes() const
    {
    return m_SizeInBytes;
    }

//-----------------------------------------------------------------------------
// public
// Storage information
// GetPixelType
//-----------------------------------------------------------------------------
const HFCPtr<HRPPixelType>& HRFThumbnail::GetPixelType() const
    {
    return m_pPixelType;
    }

//-----------------------------------------------------------------------------
// public
// Storage information
// GetPalette
//-----------------------------------------------------------------------------
const HRPPixelPalette& HRFThumbnail::GetPalette() const
    {
    return m_pPixelType->GetPalette();
    }

//-----------------------------------------------------------------------------
// public
// Storage information
// SetPalette
//-----------------------------------------------------------------------------
void HRFThumbnail::SetPalette(const HRPPixelPalette& pi_rPalette)
    {
    // Lock the palette and unlock it after the replace value
    HRPPixelPalette& rPalette = m_pPixelType->LockPalette();
    rPalette = pi_rPalette;
    m_pPixelType->UnlockPalette();
    m_IsPaletteChanged = true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
Byte const* HRFThumbnail::GetDataP() const
    {
    HPRECONDITION (m_AccessMode.m_HasReadAccess);

    return m_pData;
    }

//-----------------------------------------------------------------------------
// public
// Read
// Thumbnail data
//-----------------------------------------------------------------------------
bool HRFThumbnail::Read(Byte* po_pData) const
    {
    HPRECONDITION(po_pData != 0);
    HPRECONDITION (m_AccessMode.m_HasReadAccess);
    memcpy(po_pData, m_pData, m_SizeInBytes);

    return true;
    }

//-----------------------------------------------------------------------------
// public
// Write
// Thumbnail data
//-----------------------------------------------------------------------------
bool HRFThumbnail::Write(const Byte* pi_pData)
    {
    HPRECONDITION (m_AccessMode.m_HasWriteAccess);
    HPRECONDITION(pi_pData != 0);

    memcpy(m_pData, pi_pData, m_SizeInBytes);
    m_IsDataChanged = true;

    return true;
    }

//-----------------------------------------------------------------------------
// public
// IsDataChanged
// Thumbnail data State
//-----------------------------------------------------------------------------
bool HRFThumbnail::IsComposed () const
    {
    return m_IsComposed;
    }

//-----------------------------------------------------------------------------
// public
// IsDataChanged
// Thumbnail data State
//-----------------------------------------------------------------------------
bool HRFThumbnail::IsDataChanged () const
    {
    return m_IsDataChanged;
    }

//-----------------------------------------------------------------------------
// public
// IsPaletteChanged
// Thumbnail data State
//-----------------------------------------------------------------------------
bool HRFThumbnail::IsPaletteChanged() const
    {
    return m_IsPaletteChanged;
    }