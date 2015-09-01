//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRANearestSamplerN1.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#pragma once

#include "HRANearestSampler.h"

BEGIN_IMAGEPP_NAMESPACE
class HCDPacket;
class HGSMemorySurfaceDescriptor;

class HRANearestSamplerN1 : public HRANearestSampler
    {
    HDECLARE_CLASS_ID(HRANearestSamplerId_N1, HRANearestSampler)

public:

    // Primary methods
    HRANearestSamplerN1(HGSMemorySurfaceDescriptor const&  pi_rMemorySurface,
                        const HGF2DRectangle&              pi_rSampleDimension,
                        double                             pi_DeltaX,
                        double                             pi_DeltaY);
    virtual         ~HRANearestSamplerN1();

    virtual void const* GetPixel(double pi_PosX, double pi_PosY) const override;

    virtual void    GetPixels(const double*    pi_pPositionsX,
                              const double*    pi_pPositionsY,
                              size_t            pi_PixelCount,
                              void*             po_pBuffer) const override;

    virtual void    GetPixels(double           pi_PositionX,
                              double           pi_PositionY,
                              size_t            pi_PixelCount,
                              void*             po_pBuffer) const override;


protected:

private:

    uint32_t        m_DataWidth;
    uint32_t        m_DataHeight;

    HFCPtr<HCDPacket>
    m_pPacket;

    uint32_t        m_BitsPerPixel;
    uint32_t        m_PixelsPerByte;
    size_t          m_BytesPerLine;
    bool           m_SLO4;
    Byte          m_Mask;
    mutable Byte  m_TmpValue;

    // optimization
    bool           m_StretchByLine;
    bool           m_ReverseLine;

    void            ComputeAddress(HUINTX     pi_PosX,
                                   HUINTX     pi_PosY,
                                   Byte**   po_ppRawData,
                                   Byte*    po_pBitIndex) const;

    // disabled methods
    HRANearestSamplerN1();
    HRANearestSamplerN1(const HRANearestSamplerN1& pi_rObj);
    HRANearestSamplerN1&      operator=(const HRANearestSamplerN1& pi_rObj);
    };
END_IMAGEPP_NAMESPACE

