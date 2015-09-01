//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRAGenericBicubicSampler.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#pragma once

#include "HRAGenericSampler.h"

#include "HCDPacket.h"
#include "HRPPixelConverter.h"

BEGIN_IMAGEPP_NAMESPACE

template <class T>
class HRAGenericBicubicSampler : public HRAGenericSampler
    {
    HDECLARE_CLASS_ID(HRAGenericId_BicubicSampler, HRAGenericSampler)

public:

    // Primary methods
    HRAGenericBicubicSampler(HGSMemorySurfaceDescriptor const&   pi_rMemorySurface,
                             const HGF2DRectangle&               pi_rSampleDimension,
                             double                              pi_DeltaX,
                             double                              pi_DeltaY);
    virtual         ~HRAGenericBicubicSampler();

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

    /*
        The parameter "m_a" affects the trade-off between smoothing and
        sharpening. Its value can be between -1.0 and 0.0.
        More negative values do more sharpening and less smoothing.
        Less negative values do more smoothing and less sharpening.

        The paper
            Parker, J.A., R.V. Kenyon, and Troxel, D.E., "Comparison of
                 Interpolating Methods for Image Resampling", _IEEE Transations on
                 Medical Systems_, Vol. MI-2, No. 1, March, 1983, pp. 31-39.
        recommends using a=-1 if the resulting image is to be viewed directly
        by humans without any further processing.  However that paper
        recommends using a=-0.5 if the resulting image is to be processed
        further.  If I remember correctly (it's been a while since I read that
        paper), the rationale is that a=-0.5 is likely to give the most
        numerical accurate pixel values, which may be important if further
        numerical processing is to be done.  However, sharpened images
        generally look better to human viewers than do images whose pixels are
        numerically correct.
    */
    static const double    s_a;

    // Optimization (Precomputing with m_a)
    static const double    s_2a;
    static const double    s_2aP3;
    static const double    s_aP2;
    static const double    s_aP3;

    // Store information about the 4x4 required for
    // bi-cubic-interpolation. Index [1,1] represents
    // the pixel from which x and y delta are computed
    // (always relative to the center of the pixel in
    // both directions).
    class Sample
        {
    public:

        Sample(double pi_PositionX, double pi_PositionY);
        Sample();
        ~Sample();

        void    SetPosition(double pi_PositionX, double pi_PositionY);
        void    TranslateX(double pi_DeltaX);

        uint32_t GetLine0() const;
        uint32_t GetLine1() const;
        uint32_t GetLine2() const;
        uint32_t GetLine3() const;
        uint32_t GetColumn0() const;
        uint32_t GetColumn1() const;
        uint32_t GetColumn2() const;
        uint32_t GetColumn3() const;

        double GetXDelta() const;
        double GetYDelta() const;

    private:

        // Position relative to center of pixel 1 (line 1 & column 1).
        double m_PositionX;
        double m_PositionY;

        // disabled methods
        Sample(const Sample& pi_rObj);
        Sample& operator=(const Sample& pi_rObj);
        };

    // disabled methods
    HRAGenericBicubicSampler();
    HRAGenericBicubicSampler(const HRAGenericBicubicSampler& pi_rObj);
    HRAGenericBicubicSampler& operator=(const HRAGenericBicubicSampler& pi_rObj);
    };

END_IMAGEPP_NAMESPACE
#include "HRAGenericBicubicSampler.hpp"


