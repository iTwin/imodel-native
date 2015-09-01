//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRABilinearSamplerN8.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//:>-----------------------------------------------------------------------------
//:> Class : HRABilinearSamplerN8
//:>-----------------------------------------------------------------------------

#pragma once

#include "HRAGenericSampler.h"
#include "HRPPixelType.h"

BEGIN_IMAGEPP_NAMESPACE
class HCDPacket;
class HRPPixelConverter;
class HGSMemorySurfaceDescriptor;

class HRABilinearSamplerN8 : public HRAGenericSampler
    {
    HDECLARE_CLASS_ID(HRABilinearSamplerN8Id, HRAGenericSampler)

public:

    // Primary methods
    HRABilinearSamplerN8(HGSMemorySurfaceDescriptor const&     pi_rMemorySurface,
                         const HGF2DRectangle&                 pi_rSampleDimension,
                         double                                pi_DeltaX,
                         double                                pi_DeltaY);
    virtual         ~HRABilinearSamplerN8();

    virtual void const* GetPixel(double pi_PosX, double pi_PosY) const override;

    virtual void    GetPixels(const double*     pi_pPositionsX,
                              const double*     pi_pPositionsY,
                              size_t            pi_PixelCount,
                              void*             po_pBuffer) const override;

    virtual void    GetPixels(double            pi_PositionX,
                              double            pi_PositionY,
                              size_t            pi_PixelCount,
                              void*             po_pBuffer) const override;

    virtual HFCPtr<HRPPixelType>
    GetOutputPixelType() const;

    virtual bool   TryToUse(const HFCPtr<HRPPixelType>& pi_rpOutputPixelType);

private:

    // Hold one pixel, used in GetPixel()
    mutable Byte   m_TempData[HRPPixelType::MAX_PIXEL_BYTES];

    //:> Information about the source data
    uint32_t        m_SourceBytesPerLine;
    uint32_t        m_SourceBytesPerPixel;
    bool           m_SLO4;
    HFCPtr<HCDPacket>
    m_pPacket;

    uint32_t        m_BytesPerPixel;

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

    Byte*         ComputeAddress(HUINTX  pi_PosX,
                                   HUINTX  pi_PosY,
                                   size_t  pi_NeededPixels = ULONG_MAX) const;


    class Sample
        {
    public:

        Sample(double pi_PositionX, double pi_PositionY);
        ~Sample();

        void    SetPosition(double pi_PositionX, double pi_PositionY);
        void    TranslateX(double pi_DeltaX);

        uint32_t GetFirstLine() const;
        uint32_t GetSecondLine() const;
        uint32_t GetFirstColumn() const;
        uint32_t GetSecondColumn() const;

        double GetXDeltaOfFirstPixel() const;
        double GetYDeltaOfFirstPixel() const;

    private:

        double m_PositionX;
        double m_PositionY;

        bool   m_XIsPastCenter;
        bool   m_YIsPastCenter;

        // disabled methods
        Sample();
        Sample(const Sample& pi_rObj);
        Sample& operator=(const Sample& pi_rObj);
        };

    // disabled methods
    HRABilinearSamplerN8();
    HRABilinearSamplerN8(const HRABilinearSamplerN8& pi_rObj);
    HRABilinearSamplerN8& operator=(const HRABilinearSamplerN8& pi_rObj);
    };

END_IMAGEPP_NAMESPACE

#include "HRABilinearSamplerN8.hpp"

