//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HCDCodec.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HCDCodec
//-----------------------------------------------------------------------------
// General class for codecs.
//-----------------------------------------------------------------------------

#pragma once

#include <Imagepp/all/h/HFCExclusiveKey.h>

BEGIN_IMAGEPP_NAMESPACE

class HCDPacketRLE;

// Binary optimization: Speed up read/write when codec support direct decompression/compression to/from RLE format.
class HNOVTABLEINIT HCDCodecRLEInterface
    {
public:
    virtual void     DecompressSubsetToRLE(const void* pi_pInData, size_t pi_InDataSize, HFCPtr<HCDPacketRLE>& pio_rpRLEPacket) = 0;
    virtual size_t   CompressSubsetFromRLE(HFCPtr<HCDPacketRLE> const& pi_rpPacketRLE, void* po_pOutBuffer, size_t po_OutBufferSize) = 0;
    };

class HNOVTABLEINIT HCDCodec : public HFCShareableObject<HCDCodec>,
    public HFCExclusiveKey     // to have access to claim/release instead of including m_Key and adding the methods
    {
    HDECLARE_BASECLASS_ID(HCDCodecId_Base)

public:

    // codec states enumeration
    enum State
        {
        STATE_NONE,
        STATE_COMPRESS,
        STATE_DECOMPRESS
        };

    // destructor
    virtual         ~HCDCodec();

    // compression
    virtual size_t          CompressSubset(const void* pi_pInData,
                                           size_t pi_InDataSize,
                                           void* po_pOutBuffer,
                                           size_t pi_OutBufferSize) = 0;

    virtual size_t          DecompressSubset(const void* pi_pInData,
                                             size_t pi_InDataSize,
                                             void* po_pOutBuffer,
                                             size_t pi_OutBufferSize) = 0;

    State                   GetCurrentState() const;
    virtual void            Reset();

    virtual HCDCodec*       Clone() const = 0;

    virtual size_t          GetDataSize() const = 0;

    // subset
    virtual size_t          GetMinimumSubsetSize() const = 0;
    virtual size_t          GetSubsetMaxCompressedSize() const = 0;
    virtual void            SetSubsetSize(size_t pi_Size) = 0;

    // others
    virtual double          GetEstimatedCompressionRatio() const;
    virtual bool            HasRandomAccess() const;
    virtual bool            IsCodecImage() const;
    virtual bool            IsCodecVector() const;
    virtual const WString&  GetName() const;

    virtual HCDCodecRLEInterface* 
                            GetRLEInterface();

protected:

    // constructors
    HCDCodec(const WString& pi_rCodecName);
    HCDCodec(const HCDCodec& pi_rObj);


    // compression
    void                    SetCurrentState(State pi_State);

    WString         m_CodecName;

private:

    State          m_CurrentState;
    };

END_IMAGEPP_NAMESPACE