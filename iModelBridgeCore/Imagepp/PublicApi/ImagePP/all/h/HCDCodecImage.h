//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HCDCodecImage.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HCDCodecImage
//-----------------------------------------------------------------------------
// General class for CodecImages.
//-----------------------------------------------------------------------------

#pragma once

#include "HCDCodec.h"

BEGIN_IMAGEPP_NAMESPACE

class HCDPacketRLE;

class HNOVTABLEINIT HCDCodecImage : public HCDCodec
    {
    HDECLARE_CLASS_ID(HCDCodecId_Image, HCDCodec)

public:

    // destructor

    virtual         ~HCDCodecImage();


    // compression

    virtual void            Reset();

    // settings

    size_t                  GetBitsPerPixel() const;

    virtual size_t          GetLinePaddingBits() const;

    virtual size_t          GetWidth() const;

    virtual size_t          GetHeight() const;

    virtual bool            IsBitsPerPixelSupported(size_t pi_Bits) const = 0;

    virtual void            SetBitsPerPixel(size_t pi_BitsPerPixel);

    virtual void            SetDimensions(size_t pi_Width, size_t pi_Height);

    virtual void            SetLinePaddingBits(size_t pi_Bits);


    virtual size_t          GetDataSize() const;

    virtual HCDCodec*       Clone() const override = 0;

    // subset

    virtual size_t          GetSubsetMaxCompressedSize() const;

    virtual size_t          GetMinimumSubsetSize() const;
    virtual void            SetSubsetSize(size_t pi_Size);

    virtual size_t          GetSubsetWidth() const;

    virtual size_t          GetSubsetHeight() const;

    virtual size_t          GetSubsetPosY() const;

    virtual bool            HasLineAccess() const;

    virtual void            SetSubset(size_t pi_Width,
                                      size_t pi_Height,
                                      size_t pi_PosX = 0,
                                      size_t pi_PosY = 0);

    virtual void            SetSubsetPosY(size_t pi_PosY);


    // others

    virtual size_t          GetCompressedImageIndex() const;

    bool                    IsCodecImage() const;

    IMAGEPP_EXPORT static bool      SetCodecForImage(const HFCPtr<HCDCodec>& pi_pCodec,
                                             size_t pi_Width,
                                             size_t pi_Height,
                                             size_t pi_BitsPerPixel,
                                             size_t pi_LinePaddingBits = 0);

protected:

    // constructors

    HCDCodecImage(const WString& pi_rCodecName);

    HCDCodecImage(const WString& pi_rCodecName,
                  size_t pi_Width,
                  size_t pi_Height,
                  size_t pi_BitsPerPixel);

    HCDCodecImage(const HCDCodecImage& pi_rObj);


    // others

    virtual void   SetCompressedImageIndex(size_t pi_Index);


private:

    size_t          m_Width;

    size_t          m_Height;

    size_t          m_BitsPerPixel;

    size_t          m_SubsetWidth;

    size_t          m_SubsetHeight;

    size_t          m_SubsetPosX;

    size_t          m_SubsetPosY;


    size_t          m_LinePaddingBits;


    size_t          m_CompressedImageIndex;
    };

END_IMAGEPP_NAMESPACE

#include "HCDCodecImage.hpp"

