//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hcd/src/HCDCodecErMapperSupported.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HCDCodecIJG
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HCDCodecErMapperSupported.h>

#define HCD_CODEC_NAME     L"ErMapperSupportedCodec"

//-----------------------------------------------------------------------------
// public
// Default constructor
//-----------------------------------------------------------------------------
HCDCodecErMapperSupported::HCDCodecErMapperSupported()
    : HCDCodecImage(HCD_CODEC_NAME)
    {
    m_CompressionRatio = 10;
    }

//-----------------------------------------------------------------------------
// public
// Copy constructor
//-----------------------------------------------------------------------------
HCDCodecErMapperSupported::HCDCodecErMapperSupported(const HCDCodecErMapperSupported& pi_rObj)
    : HCDCodecImage(pi_rObj)
    {
    if (&pi_rObj != this)
        {
        m_CompressionRatio = pi_rObj.m_CompressionRatio;
        }
    }

//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HCDCodecErMapperSupported::~HCDCodecErMapperSupported()
    {
    }

//-----------------------------------------------------------------------------
// public
// Clone
//-----------------------------------------------------------------------------
HCDCodec* HCDCodecErMapperSupported::Clone() const
    {
    return new HCDCodecErMapperSupported(*this);
    }

//-----------------------------------------------------------------------------
// public
// CompressSubset
//-----------------------------------------------------------------------------
size_t HCDCodecErMapperSupported::CompressSubset(const void* pi_pInData,
                                                 size_t      pi_InDataSize,
                                                 void*       po_pOutBuffer,
                                                 size_t      po_OutBufferSize)
    {
    return 0;
    }

//-----------------------------------------------------------------------------
// public
// DecompressSubset
//-----------------------------------------------------------------------------
size_t HCDCodecErMapperSupported::DecompressSubset(const void*  pi_pInData,
                                                   size_t pi_InDataSize,
                                                   void*  po_pOutBuffer,
                                                   size_t pi_OutBufferSize)
    {
    return 0;
    }

//-----------------------------------------------------------------------------
// public
// HasLineAccess
//-----------------------------------------------------------------------------
bool HCDCodecErMapperSupported::HasLineAccess() const
    {
    return false;
    }

//-----------------------------------------------------------------------------
// public
// IsBitsPerPixelSupported
//-----------------------------------------------------------------------------
bool HCDCodecErMapperSupported::IsBitsPerPixelSupported(size_t pi_Bits) const
    {
    return false;
    }

//-----------------------------------------------------------------------------
// public
// SetCompressionRatio
//-----------------------------------------------------------------------------
void HCDCodecErMapperSupported::SetCompressionRatio(unsigned short pi_Ratio)
    {
    if (pi_Ratio == 0)
        m_CompressionRatio = 10;    // 0 is not a good value.  Use default suggested by the library
    else
        m_CompressionRatio = pi_Ratio;
    }

//-----------------------------------------------------------------------------
// public
// SetCompressionRatio
//-----------------------------------------------------------------------------
unsigned short HCDCodecErMapperSupported::GetCompressionRatio() const
    {
    return m_CompressionRatio;
    }

//-----------------------------------------------------------------------------
// protected
// HCDCodecErMapperSupported
//-----------------------------------------------------------------------------
HCDCodecErMapperSupported::HCDCodecErMapperSupported(const WString& pi_rCodecName)
    : HCDCodecImage(pi_rCodecName)
    {
    m_CompressionRatio = 10;
    }