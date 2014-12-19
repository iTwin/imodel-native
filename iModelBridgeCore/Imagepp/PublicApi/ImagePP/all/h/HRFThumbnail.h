//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFThumbnail.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRFThumbnail
//-----------------------------------------------------------------------------
// This class describes a thumbnail of an image.
//-----------------------------------------------------------------------------

#pragma once

#include "HFCPtr.h"
#include "HFCAccessMode.h"

class HRPPixelType;
class HRPPixelPalette;

class HRFThumbnail : public HFCShareableObject<HRFThumbnail>
    {
    HDECLARE_SEALEDCLASS_ID(1461)

public:
    // Thumbnail creation et destruction
    _HDLLg HRFThumbnail();
    _HDLLg HRFThumbnail(uint32_t                     pi_Width,
                 uint32_t                     pi_Height,
                 const HFCPtr<HRPPixelType>&  pi_rpPixelType,
                 const Byte*                 pi_pData,
                 HFCAccessMode                pi_AccessMode,
                 bool                        pi_IsComposed = false,
                 unsigned short              pi_BitsAlignment = 8);

    HRFThumbnail(const HRFThumbnail& pi_rObj);

    _HDLLg ~HRFThumbnail();

    // Thumbnail data size
    _HDLLg uint32_t                GetWidth         () const;
    _HDLLg uint32_t                GetHeight        () const;
    unsigned short                  GetBitsAlignment () const;
    size_t                          GetBytesPerWidth () const;
    _HDLLg size_t                          GetSizeInBytes   () const;

    // Color Space
    _HDLLg const HFCPtr<HRPPixelType>&     GetPixelType  () const;
    const HRPPixelPalette&          GetPalette    () const;
    void                            SetPalette    (const HRPPixelPalette& pi_rPalette);

    // Editor Read-Write
    HFCAccessMode                   GetAccessMode () const;
    _HDLLg bool                     Read          (Byte*                 po_pData) const;
    _HDLLg bool                     Write         (const Byte*           pi_pData);

    // data State
    bool                           IsComposed() const;

    bool                           IsDataChanged () const;
    bool                           IsPaletteChanged() const;

protected:
private:
    // The thumbnail size
    uint32_t                m_Width;
    uint32_t                m_Height;
    unsigned short         m_BitsAlignment;
    size_t                  m_BytesPerWidth;
    size_t                  m_SizeInBytes;

    // Storage information
    HFCPtr<HRPPixelType>    m_pPixelType;
    HArrayAutoPtr<Byte>   m_pData;
    HFCAccessMode           m_AccessMode;
    bool                   m_IsComposed;
    bool                   m_IsDataChanged;
    bool                   m_IsPaletteChanged;

    // Not implemented
    HRFThumbnail& operator=(const HRFThumbnail& pi_rObj);
    };


