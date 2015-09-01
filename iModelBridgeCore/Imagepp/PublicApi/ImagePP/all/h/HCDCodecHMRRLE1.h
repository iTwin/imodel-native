//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HCDCodecHMRRLE1.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HCDCodecHMRRLE1
//-----------------------------------------------------------------------------
// HMRRLE1 codec lass.
//-----------------------------------------------------------------------------
#pragma once

#include "HCDCodecRLE1.h"

BEGIN_IMAGEPP_NAMESPACE

class HCDCodecHMRRLE1 : public HCDCodecRLE1, HCDCodecRLEInterface
    {
    HDECLARE_CLASS_ID(HCDCodecId_HMRRLE1, HCDCodecRLE1)

public:

    IMAGEPP_EXPORT                 HCDCodecHMRRLE1();

    IMAGEPP_EXPORT                 HCDCodecHMRRLE1(size_t pi_Width,
                                           size_t pi_Height);

    IMAGEPP_EXPORT                 HCDCodecHMRRLE1(const HCDCodecHMRRLE1& pi_rObj);

    IMAGEPP_EXPORT                 ~HCDCodecHMRRLE1();

    IMAGEPP_EXPORT virtual size_t   CompressSubset(const void*      pi_pInData,
                                           size_t           pi_InDataSize,
                                           void*            po_pOutBuffer,
                                           size_t           pi_OutBufferSize);

    IMAGEPP_EXPORT virtual size_t   DecompressSubset(const void*    pi_pInData,
                                             size_t                 pi_InDataSize,
                                             void*                  po_pOutBuffer,
                                             size_t                 pi_OutBufferSize);

    // Binary optimization: Speed up read when codec support direct decompression to RLE format.
    virtual HCDCodecRLEInterface*   GetRLEInterface();
    IMAGEPP_EXPORT virtual void             DecompressSubsetToRLE(const void* pi_pInData, size_t pi_InDataSize, HFCPtr<HCDPacketRLE>& pio_rpRLEPacket) override;
    IMAGEPP_EXPORT virtual size_t           CompressSubsetFromRLE(HFCPtr<HCDPacketRLE> const& pi_rpPacketRLE, void* po_pOutBuffer, size_t po_OutBufferSize) override;

    virtual void    SetDimensions(size_t pi_Width, size_t pi_Height);

    virtual bool           HasLineAccess() const;

    virtual HCDCodec* Clone() const override;

    IMAGEPP_EXPORT void             SetLineHeader(bool pi_Enable);

    IMAGEPP_EXPORT void             SetOneLineMode(bool pi_Enable);

    bool                    IsOneLineMode() const;

    bool                    HasLineHeader() const;

    IMAGEPP_EXPORT uint32_t*         GetLineIndexesTable() const;
    IMAGEPP_EXPORT void             EnableLineIndexesTable(bool pi_Enable);
    IMAGEPP_EXPORT void             SetLineIndexesTable(const uint32_t* pi_pTable);
    IMAGEPP_EXPORT bool             HasLineIndexesTable() const;


    IMAGEPP_EXPORT static size_t    GetSizeOf       (const void*    pi_pCompressedData,
                                             uint32_t       pi_Width,
                                             uint32_t       pi_Height);

protected:

private:

    bool           m_LineHeader;
    bool           m_OneLineMode;
    bool           m_LineIndexesTable;
    uint32_t*       m_pLineIndexesTable;
    size_t         m_LineIndexesTableSize;

    size_t        m_LastCompressionIndex;
    };

END_IMAGEPP_NAMESPACE
