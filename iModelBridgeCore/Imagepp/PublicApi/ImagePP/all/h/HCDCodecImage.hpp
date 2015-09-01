//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HCDCodecImage.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Inline methods for class HCDCodecImage
//-----------------------------------------------------------------------------
BEGIN_IMAGEPP_NAMESPACE
//-----------------------------------------------------------------------------
// public inline method
// GetCompressedImageIndex
//-----------------------------------------------------------------------------
inline size_t HCDCodecImage::GetCompressedImageIndex() const
    {
    // return the compressed image index
    return m_CompressedImageIndex;
    }

//-----------------------------------------------------------------------------
// public inline method
// GetLinePaddingBits
//-----------------------------------------------------------------------------
inline size_t HCDCodecImage::GetLinePaddingBits() const
    {
    // return the line padding bits
    return m_LinePaddingBits;
    }

//-----------------------------------------------------------------------------
// public inline method
// GetBitsPerPixel
//-----------------------------------------------------------------------------
inline size_t HCDCodecImage::GetBitsPerPixel() const
    {
    // return the number of bits per pixel
    return m_BitsPerPixel;
    }

//-----------------------------------------------------------------------------
// public inline method
// GetSubsetPosY
//-----------------------------------------------------------------------------
inline size_t HCDCodecImage::GetSubsetPosY() const
    {
    // return the subset starting line position
    return m_SubsetPosY;
    }

//-----------------------------------------------------------------------------
// public inline method
// GetSubsetHeight
//-----------------------------------------------------------------------------
inline size_t HCDCodecImage::GetSubsetHeight() const
    {
    // return the subset height
    return m_SubsetHeight;
    }

//-----------------------------------------------------------------------------
// public inline method
// GetSubsetWidth
//-----------------------------------------------------------------------------
inline size_t HCDCodecImage::GetSubsetWidth() const
    {
    // return the subset width
    return m_SubsetWidth;
    }

//-----------------------------------------------------------------------------
// public inline method
// GetHeight
//-----------------------------------------------------------------------------
inline size_t HCDCodecImage::GetHeight() const
    {
    // return the height of the image
    return m_Height;
    }

//-----------------------------------------------------------------------------
// public inline method
// GetWidth
//-----------------------------------------------------------------------------
inline size_t HCDCodecImage::GetWidth() const
    {
    // return the width of the image
    return m_Width;
    }
END_IMAGEPP_NAMESPACE