//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFThumbnail.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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

BEGIN_IMAGEPP_NAMESPACE
class HRPPixelType;
class HRPPixelPalette;

class HRFThumbnail : public HFCShareableObject<HRFThumbnail>
    {
    HDECLARE_SEALEDCLASS_ID(HRFThumbnailId_Base)

public:
    // Thumbnail creation et destruction
    IMAGEPP_EXPORT HRFThumbnail();
    IMAGEPP_EXPORT HRFThumbnail(uint32_t                     pi_Width,
                 uint32_t                     pi_Height,
                 const HFCPtr<HRPPixelType>&  pi_rpPixelType,
                 const Byte*                 pi_pData,
                 HFCAccessMode                pi_AccessMode,
                 bool                        pi_IsComposed = false,
                 unsigned short              pi_BitsAlignment = 8);

    HRFThumbnail(const HRFThumbnail& pi_rObj);

    IMAGEPP_EXPORT ~HRFThumbnail();

    // Thumbnail data size
    IMAGEPP_EXPORT uint32_t                GetWidth         () const;
    IMAGEPP_EXPORT uint32_t                GetHeight        () const;
    unsigned short                  GetBitsAlignment () const;
    size_t                          GetBytesPerWidth () const;
    IMAGEPP_EXPORT size_t                          GetSizeInBytes   () const;

    // Color Space
    IMAGEPP_EXPORT const HFCPtr<HRPPixelType>&     GetPixelType  () const;
    const HRPPixelPalette&          GetPalette    () const;
    void                            SetPalette    (const HRPPixelPalette& pi_rPalette);

    // Editor Read-Write
    HFCAccessMode                   GetAccessMode () const;
    IMAGEPP_EXPORT bool                     Read          (Byte*                 po_pData) const;
    IMAGEPP_EXPORT bool                     Write         (const Byte*           pi_pData);

    IMAGEPP_EXPORT Byte const* GetDataP() const;

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
END_IMAGEPP_NAMESPACE


