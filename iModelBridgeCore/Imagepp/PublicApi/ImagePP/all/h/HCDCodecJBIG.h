//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HCDCodecJBIG
//-----------------------------------------------------------------------------
#pragma once

#include "HCDCodecImage.h"

BEGIN_IMAGEPP_NAMESPACE

class HCDCodecJBIG : public HCDCodecImage
    {
    HDECLARE_CLASS_ID(HCDCodecId_JBIG, HCDCodecImage)

public:

    typedef vector<uint32_t, allocator<uint32_t> >
    HUINTVector;

    typedef vector< HUINTVector, allocator< HUINTVector > >
    HUINTVectorVector;

    // primary methods
    IMAGEPP_EXPORT                  HCDCodecJBIG();
    IMAGEPP_EXPORT                  HCDCodecJBIG(size_t pi_Width,
                                            size_t pi_Height);
    IMAGEPP_EXPORT                  HCDCodecJBIG(const HCDCodecJBIG& pi_rObj);
    IMAGEPP_EXPORT virtual          ~HCDCodecJBIG();

    // overriden methods
    virtual HCDCodec*       Clone() const override;

    size_t          CompressSubset(const void* pi_pInData,
                                           size_t pi_InDataSize,
                                           void* po_pOutBuffer,
                                           size_t pi_OutBufferSize) override;
    size_t          DecompressSubset(const void* pi_pInData,
                                             size_t pi_InDataSize,
                                             void* po_pOutBuffer,
                                             size_t pi_OutBufferSize) override;

    bool            IsBitsPerPixelSupported(size_t pi_Bits) const override;

    static void             output_bie(Byte* start, size_t len, void* file);

protected:

    void InitObject();

    Byte*                  m_pOutBuffer;
    };

END_IMAGEPP_NAMESPACE
