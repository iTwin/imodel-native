//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hcd/src/HCDCodecIdentity.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HCDCodecIdentity
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HCDCodecIdentity.h>

#define HCD_CODEC_NAME L"Uncompressed"
//-----------------------------------------------------------------------------
// public
// Default constructor
//-----------------------------------------------------------------------------
HCDCodecIdentity::HCDCodecIdentity()
    : HCDCodecVector(HCD_CODEC_NAME)
    {
    }


//-----------------------------------------------------------------------------
// public
// constructor
//-----------------------------------------------------------------------------
HCDCodecIdentity::HCDCodecIdentity(size_t pi_DataSize)
    : HCDCodecVector(HCD_CODEC_NAME, pi_DataSize)
    {
    }

//-----------------------------------------------------------------------------
// public
// Copy constructor
//-----------------------------------------------------------------------------
HCDCodecIdentity::HCDCodecIdentity(const HCDCodecIdentity& pi_rObj)
    : HCDCodecVector(pi_rObj)
    {
    }

//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HCDCodecIdentity::~HCDCodecIdentity()
    {
    }


//-----------------------------------------------------------------------------
// public
// Clone
//-----------------------------------------------------------------------------
HCDCodec* HCDCodecIdentity::Clone() const
    {
    return new HCDCodecIdentity(*this);
    }

//-----------------------------------------------------------------------------
// public
// CompressSubset
//-----------------------------------------------------------------------------
size_t HCDCodecIdentity::CompressSubset(    const void* pi_pInData,
                                            size_t pi_InDataSize,
                                            void* po_pOutBuffer,
                                            size_t pi_OutBufferSize)
    {
    HPRECONDITION(GetSubsetSize() <= pi_OutBufferSize);

    // is it the first packet?
    if(GetSubsetPos() == 0)
        {
        SetCurrentState(STATE_COMPRESS);
        }

    memcpy(po_pOutBuffer, pi_pInData, GetSubsetSize());

    SetSubsetPos(GetSubsetPos() + GetSubsetSize());

    // is it the last subset
    if(GetSubsetPos() == GetDataSize())
        {
        Reset();
        }

    return GetSubsetSize();
    }

//-----------------------------------------------------------------------------
// public
// DecompressSubset
//-----------------------------------------------------------------------------
size_t HCDCodecIdentity::DecompressSubset(const void* pi_pInData,
                                          size_t pi_InDataSize,
                                          void* po_pOutBuffer,
                                          size_t pi_OutBufferSize)
    {
    HPRECONDITION(GetSubsetSize() <= pi_OutBufferSize);

    // is it the first packet?
    if(GetSubsetPos() == 0)
        {
        SetCurrentState(STATE_DECOMPRESS);
        }

    memcpy(po_pOutBuffer, pi_pInData, GetSubsetSize());

    SetSubsetPos(GetSubsetPos() + GetSubsetSize());

    // is it the last subset
    if(GetSubsetPos() == GetDataSize())
        {
        Reset();
        }

    return GetSubsetSize();
    }

//-----------------------------------------------------------------------------
// public
// GetSubsetMaxCompressedSize
//-----------------------------------------------------------------------------
size_t HCDCodecIdentity::GetSubsetMaxCompressedSize() const
    {
    return (GetSubsetSize());
    }


//-----------------------------------------------------------------------------
// public
// SetName
//-----------------------------------------------------------------------------
void HCDCodecIdentity::SetName(const WString& pi_rName)
    {
    m_CodecName = pi_rName;
    }
