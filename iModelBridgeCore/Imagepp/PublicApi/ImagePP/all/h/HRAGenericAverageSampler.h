//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRAGenericAverageSampler.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//:>-----------------------------------------------------------------------------
//:> Class : HRAGenericAverageSampler
//:>-----------------------------------------------------------------------------

#pragma once

#include "HRAGenericSampler.h"
#include "HGF2DRectangle.h"
#include "HRPPixelType.h"

BEGIN_IMAGEPP_NAMESPACE
class HCDPacket;
class HRPPixelConverter;
class HGSMemorySurfaceDescriptor;

template <class T, class TS>
class HRAGenericAverageSampler : public HRAGenericSampler
    {
    HDECLARE_CLASS_ID(HRAGenericId_AverageSampler, HRAGenericSampler)

public:

    // Primary methods
    HRAGenericAverageSampler(HGSMemorySurfaceDescriptor const&  pi_rMemorySurface,
                             const HGF2DRectangle&              pi_rSampleDimension,
                             double                             pi_DeltaX,
                             double                             pi_DeltaY);
    virtual         ~HRAGenericAverageSampler();

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
     
    virtual void    StretchByTwo(uint32_t pi_PositionX,
                                 uint32_t pi_PositionY,
                                 size_t  pi_PixelCount,
                                 Byte* po_pBuffer) const;

protected:

    bool                m_StretchLineByTwo;
    HFCPtr<HCDPacket>   m_pPacket;

    // Hold one pixel, used in GetPixel()
    mutable Byte m_pTempData[HRPPixelType::MAX_PIXEL_BYTES];

    //:> Information about the source data
    uint32_t   m_SourceBytesPerLine;
    uint32_t   m_SourceBytesPerPixel;
    bool        m_SLO4;

    uint32_t   m_BytesPerPixel;
    uint32_t   m_NbChannels;
    unsigned short m_NbBytesPerChannel;

    // Pixel type used to average and return results
    // (if different from source pixel type)
    HFCPtr<HRPPixelType>
    m_pWorkingPixelType;

    // Converter to sampling pixeltype if necessary
    HFCPtr<HRPPixelConverter> m_pConverter;

    // Keep full converted lines for reuse
    HAutoPtr<Byte*> m_ppConvertedLines;

    Byte* ComputeAddress(const HFCPtr<HCDPacket>& pi_rpPacket,
                          HUINTX                   pi_PosX,
                          HUINTX                   pi_PosY,
                          size_t                   pi_NeededPixels = ULONG_MAX) const;

private:
    // disabled methods
    HRAGenericAverageSampler();
    HRAGenericAverageSampler(const HRAGenericAverageSampler& pi_rObj);
    HRAGenericAverageSampler&
    operator=(const HRAGenericAverageSampler& pi_rObj);
    };

//-------------------------------------------------------------------------------------
// HRAGenericAverageSparseDataSampler : This class must be used instead of
//                                      HRAGenericAverageSampler when the raster data
//                                      contains hole (i.e. no data).
//-------------------------------------------------------------------------------------
template <class T, class TS>
class HRAGenericAverageSparseDataSampler : public HRAGenericAverageSampler<T, TS>
    {
    HDECLARE_CLASS_ID(HRAGenericId_AverageSparseDataSampler, HRAGenericSampler)         //DM-Android should be HRAGenericAverageSampler

public:

    // Primary methods
    HRAGenericAverageSparseDataSampler(HGSMemorySurfaceDescriptor const&  pi_rMemorySurface,
                                       const HGF2DRectangle&              pi_rSampleDimension,
                                       double                             pi_DeltaX,
                                       double                             pi_DeltaY);
    virtual         ~HRAGenericAverageSparseDataSampler();

    virtual void const* GetPixel(double pi_PosX, double pi_PosY) const override;

    virtual void    GetPixels(const double*  pi_pPositionsX,
                              const double*  pi_pPositionsY,
                              size_t          pi_PixelCount,
                              void*           po_pBuffer) const override;

    virtual void    GetPixels(double         pi_PositionX,
                              double         pi_PositionY,
                              size_t          pi_PixelCount,
                              void*           po_pBuffer) const override;

private:

    // disabled methods
    HRAGenericAverageSparseDataSampler();
    HRAGenericAverageSparseDataSampler(const HRAGenericAverageSparseDataSampler& pi_rObj);
    HRAGenericAverageSparseDataSampler&
    operator=(const HRAGenericAverageSparseDataSampler& pi_rObj);

    T m_NoDataValue;
    };

//-------------------------------------------------------------------------------------
// HRAGenericAverageSparseDataSampler : This class must be used instead of
//                                      HRAGenericAverageSampler when the raster data
//                                      are integer and don't contains hole
//                                      (i.e. no data). The benefit of using this class
//                                      is that strech by a factor of two is optimized.
//-------------------------------------------------------------------------------------
template <class T, class TS>
class HRAGenericAverageSamplerInteger : public HRAGenericAverageSampler<T, TS>
    {
    HDECLARE_CLASS_ID(HRAGenericId_AverageSamplerInteger, HRAGenericSampler)             //DM-Android should be HRAGenericAverageSampler

public:

    HRAGenericAverageSamplerInteger(HGSMemorySurfaceDescriptor const&  pi_rMemorySurface,
                                    const HGF2DRectangle&              pi_rSampleDimension,
                                    double                             pi_DeltaX,
                                    double                             pi_DeltaY);
    virtual         ~HRAGenericAverageSamplerInteger();

protected:

    virtual void    StretchByTwo(uint32_t pi_PositionX,
                                 uint32_t pi_PositionY,
                                 size_t  pi_PixelCount,
                                 Byte* po_pBuffer) const override;

private:

    uint32_t m_DataWidth;
    uint32_t m_DataHeight;
    };
END_IMAGEPP_NAMESPACE