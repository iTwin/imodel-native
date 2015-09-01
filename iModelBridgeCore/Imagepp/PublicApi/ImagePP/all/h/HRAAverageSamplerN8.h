//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRAAverageSamplerN8.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//:>-----------------------------------------------------------------------------
//:> Class : HRAAverageSamplerN8
//:>-----------------------------------------------------------------------------

#pragma once

#include "HRAGenericSampler.h"
#include "HRPPixelType.h"

BEGIN_IMAGEPP_NAMESPACE
class HCDPacket;
class HRPPixelConverter;
class HGSMemorySurfaceDescriptor;


class HRAAverageSamplerN8 : public HRAGenericSampler
    {
    HDECLARE_CLASS_ID(HRAAverageSamplerN8Id, HRAGenericSampler)

public:

    // Primary methods
    HRAAverageSamplerN8(HGSMemorySurfaceDescriptor const&  pi_rMemorySurface,
                        const HGF2DRectangle&              pi_rSampleDimension,
                        double                             pi_DeltaX,
                        double                             pi_DeltaY);
    virtual         ~HRAAverageSamplerN8();

    virtual void const* GetPixel(double pi_PosX, double pi_PosY) const override;

    virtual void    GetPixels(const double*  pi_pPositionsX,
                              const double*  pi_pPositionsY,
                              size_t          pi_PixelCount,
                              void*           po_pBuffer) const override;

    virtual void    GetPixels(double         pi_PositionX,
                              double         pi_PositionY,
                              size_t          pi_PixelCount,
                              void*           po_pBuffer) const override;

    virtual HFCPtr<HRPPixelType> GetOutputPixelType() const override;

    virtual bool   TryToUse(const HFCPtr<HRPPixelType>& pi_rpOutputPixelType) override;

protected:

    uint32_t        m_DataWidth;
    uint32_t        m_DataHeight;

private:

    // Hold one pixel, used in GetPixel()
    mutable Byte m_TempData[HRPPixelType::MAX_PIXEL_BYTES];

    //:> Information about the source data
    size_t          m_SourceBytesPerLine;
    size_t          m_SourceBytesPerPixel;
    bool           m_SLO4;
    HFCPtr<HCDPacket>
    m_pPacket;

    size_t          m_BytesPerPixel;

    // Pixel type used to average and return results
    // (if different from source pixel type)
    HFCPtr<HRPPixelType>
    m_pWorkingPixelType;

    // Converter to sampling pixeltype if necessary
    HFCPtr<HRPPixelConverter>
    m_pConverter;

    // Keep full converted lines for reuse
    HAutoPtr<Byte*>
    m_ppConvertedLines;

    // optimization
    bool           m_StretchByLine;
    bool           m_StretchLineByTwo;

    Byte*         ComputeAddress(HUINTX  pi_PosX,
                                   HUINTX  pi_PosY,
                                   uint32_t pi_NeededPixels = ULONG_MAX) const;

    void            StretchByTwo(uint32_t pi_PositionX,
                                 uint32_t pi_PositionY,
                                 size_t  pi_PixelCount,
                                 Byte* po_pBuffer) const;

    // disabled methods
    HRAAverageSamplerN8();
    HRAAverageSamplerN8(const HRAAverageSamplerN8& pi_rObj);
    HRAAverageSamplerN8&
    operator=(const HRAAverageSamplerN8& pi_rObj);
    };

END_IMAGEPP_NAMESPACE