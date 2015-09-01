//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hcd/src/HCDCodecImage.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HCDCodecImage
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HCDCodecImage.h>
#include <Imagepp/all/h/HCDCodecVector.h>

//-----------------------------------------------------------------------------
// protected
// Default constructor
//-----------------------------------------------------------------------------
HCDCodecImage::HCDCodecImage(const WString& pi_rCodecName)
    : HCDCodec(pi_rCodecName)
    {
    // default values fo the attributes

    m_Width = 0;
    m_Height = 0;

    m_SubsetWidth = 0;
    m_SubsetHeight = 0;
    m_SubsetPosX = 0;
    m_SubsetPosY = 0;

    m_LinePaddingBits = 0;

    m_CompressedImageIndex = 0;

    m_BitsPerPixel = 0;
    }

//-----------------------------------------------------------------------------
// protected
// Constructor
//-----------------------------------------------------------------------------
HCDCodecImage::HCDCodecImage(const WString& pi_rCodecName,
                             size_t         pi_Width,
                             size_t         pi_Height,
                             size_t         pi_BitsPerPixel)
    : HCDCodec(pi_rCodecName)
    {
    // set the image attributes
    m_Width = pi_Width;
    m_Height = pi_Height;
    m_BitsPerPixel = pi_BitsPerPixel;

    // set the subset attributes
    // by default, the subset dimensions are the same as those of the image
    m_SubsetWidth = pi_Width;
    m_SubsetHeight = pi_Height;
    m_SubsetPosX = 0;
    m_SubsetPosY = 0;

    // by default, the line padding bits is 0
    m_LinePaddingBits = 0;

    // the compressed image index is 0 (no subset decompressed yet)
    m_CompressedImageIndex = 0;
    }


//-----------------------------------------------------------------------------
// protected
// Copy constructor
//-----------------------------------------------------------------------------
HCDCodecImage::HCDCodecImage(const HCDCodecImage& pi_rObj)
    : HCDCodec(pi_rObj)
    {
    // copy the attributes

    m_Width = pi_rObj.m_Width;
    m_Height = pi_rObj.m_Height;
    m_BitsPerPixel = pi_rObj.m_BitsPerPixel;

    m_SubsetWidth = pi_rObj.m_Width;
    m_SubsetHeight = pi_rObj.m_Height;
    m_SubsetPosX = pi_rObj.m_SubsetPosX;
    m_SubsetPosY = pi_rObj.m_SubsetPosY;

    m_LinePaddingBits = pi_rObj.m_LinePaddingBits;

    m_CompressedImageIndex = pi_rObj.m_CompressedImageIndex;
    }

//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HCDCodecImage::~HCDCodecImage()
    {
    }

//-----------------------------------------------------------------------------
// public
// GetMinimumSubsetSize
//-----------------------------------------------------------------------------
size_t HCDCodecImage::GetMinimumSubsetSize() const
    {
    size_t MinimumSubsetSize;

    size_t ImageWidthInBytes = (m_Width * m_BitsPerPixel + m_LinePaddingBits + 7) / 8;

    if(HasLineAccess())
        MinimumSubsetSize = ImageWidthInBytes;
    else
        MinimumSubsetSize = ImageWidthInBytes * m_Height;

    return MinimumSubsetSize;
    }

//-----------------------------------------------------------------------------
// public
// SetSubsetSize
//-----------------------------------------------------------------------------
void HCDCodecImage::SetSubsetSize(size_t pi_Size)
    {
    size_t ImageWidthInBytes = (m_Width * m_BitsPerPixel + m_LinePaddingBits) / 8;

    size_t pi_NumberOfLines = pi_Size / ImageWidthInBytes;

    HASSERT(pi_NumberOfLines != 0);
    HASSERT_X64(pi_NumberOfLines <  ULONG_MAX);

    SetSubset(m_Width, pi_NumberOfLines, m_SubsetPosX, m_SubsetPosY);
    }

//-----------------------------------------------------------------------------
// public
// GetSubsetMaxCompressedSize
//-----------------------------------------------------------------------------
size_t HCDCodecImage::GetSubsetMaxCompressedSize() const
    {
    // HLXXX by default, return the subset size * CONSTANT
    return (static_cast<size_t>(((m_SubsetWidth * m_BitsPerPixel + 7) / 8)
                                * m_SubsetHeight
                                // extra
                                * 1.5));
    }

//-----------------------------------------------------------------------------
// public
// HasLineAccess
//-----------------------------------------------------------------------------
bool HCDCodecImage::HasLineAccess() const
    {
    // default
    return false;
    }

//-----------------------------------------------------------------------------
// public
// IsCodecImage
//-----------------------------------------------------------------------------
bool HCDCodecImage::IsCodecImage() const
    {
    return true;
    }

//-----------------------------------------------------------------------------
// public
// Reset
//-----------------------------------------------------------------------------
void HCDCodecImage::Reset()
    {
    // reset the subset to the dimensions of the image
    // set also its position to (0,0)
    m_SubsetWidth = m_Width;
    m_SubsetHeight = m_Height;
    m_SubsetPosX = 0;
    m_SubsetPosY = 0;

    // reset the compressed image index
    m_CompressedImageIndex = 0;

    // reset the codec
    HCDCodec::Reset();
    }

//-----------------------------------------------------------------------------
// public
// SetCodecForImage
//-----------------------------------------------------------------------------
bool HCDCodecImage::SetCodecForImage(const HFCPtr<HCDCodec>& pi_pCodec,
                                      size_t pi_Width,
                                      size_t pi_Height,
                                      size_t pi_BitsPerPixel,
                                      size_t pi_LinePaddingBits)
    {
    // by default, the result is false
    bool Result = false;

    // test the type of codec
    if(pi_pCodec->IsCodecImage())
        {
        // image codec, set the attributes correctly

        ((HFCPtr<HCDCodecImage>&)pi_pCodec)->SetDimensions(pi_Width, pi_Height);
        ((HFCPtr<HCDCodecImage>&)pi_pCodec)->SetBitsPerPixel(pi_BitsPerPixel);
        ((HFCPtr<HCDCodecImage>&)pi_pCodec)->SetLinePaddingBits(pi_LinePaddingBits);

        Result = true;
        }
    else if(pi_pCodec->IsCodecVector())
        {
        // vector codec, set the attributes correctly

        ((HFCPtr<HCDCodecVector>&)pi_pCodec)->SetDataSize(
            (pi_Width * pi_BitsPerPixel + pi_LinePaddingBits) / 8 *
            pi_Height);

        Result = true;
        }

    return Result;
    }


//-----------------------------------------------------------------------------
// public
// SetSubset
//-----------------------------------------------------------------------------
void HCDCodecImage::SetCompressedImageIndex(size_t pi_Index)
    {
    // set the compressed image index
    m_CompressedImageIndex = pi_Index;
    }

//-----------------------------------------------------------------------------
// public
// SetBitsPerPixel
//-----------------------------------------------------------------------------
void HCDCodecImage::SetBitsPerPixel(size_t pi_BitsPerPixel)
    {
    // set the number of bits per pixel
    m_BitsPerPixel = pi_BitsPerPixel;
    }

//-----------------------------------------------------------------------------
// public
// SetDimensions
//-----------------------------------------------------------------------------
void HCDCodecImage::SetDimensions(size_t pi_Width, size_t pi_Height)
    {
    // set the dimensions of the image
    m_Width = pi_Width;
    m_Height = pi_Height;

    // reset the subset dimensions to those of the image
    m_SubsetWidth = pi_Width;
    m_SubsetHeight = pi_Height;
    m_SubsetPosX = 0;
    m_SubsetPosY = 0;
    }

//-----------------------------------------------------------------------------
// public inline method
// SetLinePaddingBits
//-----------------------------------------------------------------------------
void HCDCodecImage::SetLinePaddingBits(size_t pi_Bits)
    {
    // set the line padding bits
    m_LinePaddingBits = pi_Bits;
    }

//-----------------------------------------------------------------------------
// public
// SetSubset
//-----------------------------------------------------------------------------
void HCDCodecImage::SetSubset(  size_t pi_Width,
                                size_t pi_Height,
                                size_t pi_PosX,
                                size_t pi_PosY)
    {
    // set the subset dimensions
    m_SubsetWidth = pi_Width;
    m_SubsetHeight = pi_Height;
    m_SubsetPosX = pi_PosX;
    m_SubsetPosY = pi_PosY;
    }


//-----------------------------------------------------------------------------
// public
// GetDataSize
//-----------------------------------------------------------------------------
size_t HCDCodecImage::GetDataSize() const
    {
    size_t DataSize = (m_Width * m_BitsPerPixel + m_LinePaddingBits) / 8 * m_Height;

    return DataSize;
    }

//-----------------------------------------------------------------------------
// public
// SetSubsetPosY
//-----------------------------------------------------------------------------
void HCDCodecImage::SetSubsetPosY(size_t pi_PosY)
    {
    // set the starting line subset position
    m_SubsetPosY = pi_PosY;
    }