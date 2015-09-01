//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRAGenericBilinearSampler.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//:>-----------------------------------------------------------------------------
//:> Class : HRAGenericBilinearSampler
//:>-----------------------------------------------------------------------------

#pragma once

#include "HRAGenericSampler.h"

#include "HCDPacket.h"
#include "HRPPixelConverter.h"

BEGIN_IMAGEPP_NAMESPACE

template<class T>
class HRAGenericBilinearSampler : public HRAGenericSampler
    {
    HDECLARE_CLASS_ID(HRAGenericId_BilinearSampler, HRAGenericSampler)

public:

    // Primary methods
    HRAGenericBilinearSampler(HGSMemorySurfaceDescriptor const&      pi_rMemorySurface,
                              const HGF2DRectangle&                  pi_rSampleDimension,
                              double                                 pi_DeltaX,
                              double                                 pi_DeltaY);
    virtual         ~HRAGenericBilinearSampler();

    virtual void*   GetPixel(double            pi_PosX,
                             double            pi_PosY) const;

    virtual void    GetPixels(const double*    pi_pPositionsX,
                              const double*    pi_pPositionsY,
                              size_t            pi_PixelCount,
                              void*             po_pBuffer) const;

    virtual void    GetPixels(double           pi_PositionX,
                              double           pi_PositionY,
                              size_t            pi_PixelCount,
                              void*             po_pBuffer) const;

    virtual HFCPtr<HRPPixelType>
    GetOutputPixelType() const;

    virtual bool   TryToUse(const HFCPtr<HRPPixelType>& pi_rpOutputPixelType);

private:

    T m_ChannelMinValue;
    T m_ChannelMaxValue;

    // Hold one pixel, used in GetPixel()
    mutable Byte m_pTempData[HRPPixelType::MAX_PIXEL_BYTES];

    HFCPtr<HCDPacket>        m_pPacket;

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
    HRAGenericBilinearSampler();
    HRAGenericBilinearSampler(const HRAGenericBilinearSampler<T>& pi_rObj);
    HRAGenericBilinearSampler<T>& operator=(const HRAGenericBilinearSampler<T>& pi_rObj);
    };

END_IMAGEPP_NAMESPACE
#include "HRAGenericBilinearSampler.hpp"