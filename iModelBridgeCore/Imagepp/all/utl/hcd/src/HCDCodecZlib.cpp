//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hcd/src/HCDCodecZlib.cpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HCDCodecZlib
//-----------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>

#include <Imagepp/all/h/HCDCodecZlib.h>
#include <zlib/zlib.h>

//-----------------------------------------------------------------------------
// public
// Default constructor
//-----------------------------------------------------------------------------
HCDCodecZlib::HCDCodecZlib()
    : HCDCodecDeflate()
    {
    }

//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
HCDCodecZlib::HCDCodecZlib(size_t pi_DataSize)
    : HCDCodecDeflate(pi_DataSize)
    {
    }

//-----------------------------------------------------------------------------
// public
// Copy constructor
//-----------------------------------------------------------------------------
HCDCodecZlib::HCDCodecZlib(const HCDCodecZlib& pi_rObj)
    : HCDCodecDeflate(pi_rObj)
    {
    }

//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HCDCodecZlib::~HCDCodecZlib()
    {
    }

//-----------------------------------------------------------------------------
// public
// Clone
//-----------------------------------------------------------------------------
HCDCodec* HCDCodecZlib::Clone() const
    {
    return new HCDCodecZlib(*this);
    }

//-----------------------------------------------------------------------------
// public
// CompressSubset
//-----------------------------------------------------------------------------
size_t HCDCodecZlib::CompressSubset(const void* pi_pInData,
                                    size_t pi_InDataSize,
                                    void* po_pOutBuffer,
                                    size_t pi_OutBufferSize)
    {
    HASSERT_X64(pi_InDataSize < ULONG_MAX);
    HASSERT_X64(pi_OutBufferSize < ULONG_MAX);

    int err;

    uint32_t OutLen = (uint32_t)pi_OutBufferSize;

    // Use level 5 compression. Varies from 1 to 9, 6 being the default.
    // There seems to be a big difference between 5 and 6 on execution time for
    // big images, with only a small size penalty.
    err = compress2((Byte*)po_pOutBuffer, (DWORD*)&OutLen, (Byte*)pi_pInData, (uint32_t)pi_InDataSize, 5);

    if(err != Z_OK)
        OutLen = 0;

    return OutLen;
    }

//-----------------------------------------------------------------------------
// public
// DecompressSubset
//-----------------------------------------------------------------------------
size_t HCDCodecZlib::DecompressSubset(const void* pi_pInData,
                                      size_t pi_InDataSize,
                                      void* po_pOutBuffer,
                                      size_t pi_OutBufferSize)
    {
    HASSERT_X64(pi_InDataSize < ULONG_MAX);
    HASSERT_X64(pi_OutBufferSize < ULONG_MAX);

    int err;

    uint32_t OutLen = (uint32_t)pi_OutBufferSize;

    err = uncompress((Byte*)po_pOutBuffer, (DWORD*)&OutLen, (Byte*)pi_pInData, (uint32_t)pi_InDataSize);

    if(err != Z_OK)
        OutLen = 0;

    return OutLen;
    }

//-----------------------------------------------------------------------------
// public
// GetSubsetMaxCompressedSize
//-----------------------------------------------------------------------------
size_t HCDCodecZlib::GetSubsetMaxCompressedSize() const
    {
    return ((size_t)(GetSubsetSize() * 1.1 + 12 +
                     //security extra
                     5));
    }
