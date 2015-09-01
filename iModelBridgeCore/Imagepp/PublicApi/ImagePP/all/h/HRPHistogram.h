//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPHistogram.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRPHistogram
//-----------------------------------------------------------------------------
#pragma once

#include "HPMPersistentObject.h"
#include "HFCPtr.h"
#include "HRPPixelPalette.h"

BEGIN_IMAGEPP_NAMESPACE
class HRPHistogram : public HPMPersistentObject, public HPMShareableObject<HRPHistogram>
    {
    HPM_DECLARE_CLASS_DLL(IMAGEPP_EXPORT,  HRPHistogramId)

public:
    enum COLOR_SPACE
        {
        NATIVE = 0,
        GRAYSCALE,
        RGB,
        RGBA,
        LIGHTNESS,
        // LUV,
        // LAB,
        // XYZ
        // HSV
        // YCC
        // CMY
        };

    // Primary methods
    IMAGEPP_EXPORT                 HRPHistogram(uint32_t**  pi_pEntryFrequencies,
                                        uint32_t  pi_EntryFrequenciesSize,
                                        uint32_t  pi_ChannelCount,
                                        HRPHistogram::COLOR_SPACE pi_ColorSpace = HRPHistogram::NATIVE);

    IMAGEPP_EXPORT                 HRPHistogram(uint32_t*   pi_pEntryFrequencies,
                                        uint32_t  pi_EntryFrequenciesSize,
                                        HRPHistogram::COLOR_SPACE pi_ColorSpace = HRPHistogram::NATIVE);

    IMAGEPP_EXPORT                 HRPHistogram(uint32_t  pi_EntryFrequenciesSize,
                                        uint32_t  pi_ChannelCount = 1,
                                        HRPHistogram::COLOR_SPACE pi_ColorSpace = HRPHistogram::NATIVE);

    IMAGEPP_EXPORT                 HRPHistogram();
    IMAGEPP_EXPORT                 HRPHistogram(const HRPPixelPalette& pi_rPalette, HRPHistogram::COLOR_SPACE pi_ColorSpace = HRPHistogram::NATIVE);
    IMAGEPP_EXPORT                 HRPHistogram(const HRPHistogram& pi_rObj);
    IMAGEPP_EXPORT virtual         ~HRPHistogram();

    HRPHistogram&   operator=(const HRPHistogram& pi_rObj);

    // Entries
    uint32_t        GetEntryCount(uint32_t pi_EntryIndex, uint32_t pi_Channel = 0) const;
    void            SetEntryCount(uint32_t pi_EntryIndex, uint32_t Count, uint32_t pi_Channel = 0);
    void            IncrementEntryCount(uint32_t pi_EntryIndex, uint32_t Count = 1, uint32_t pi_Channel = 0);

    IMAGEPP_EXPORT uint32_t        FindEntryWithMinimumCount(uint32_t pi_Channel = 0) const;
    IMAGEPP_EXPORT uint32_t        FindEntryWithMaximumCount(uint32_t pi_Channel = 0) const;
    uint32_t        GetEntryFrequenciesSize(uint32_t pi_Channel = 0) const;
    void            GetEntryFrequencies(uint32_t* pio_pEntryFrequencies, uint32_t pi_Channel = 0) const;
    uint32_t        GetChannelCount() const;

    IMAGEPP_EXPORT void            SetSamplingColorSpace(HRPHistogram::COLOR_SPACE pi_ColorSpace);
    const HRPHistogram::COLOR_SPACE
    GetSamplingColorSpace() const;

    void            Clear();

protected:

private:
    void ConstructUsingRGBColorSpace(uint32_t**  pi_pEntryFrequencies,
                                     uint32_t  pi_EntryFrequenciesSize,
                                     uint32_t  pi_ChannelCount);

    void ConstructUsingLightnessColorSpace(unsigned int pi_EntryFrequenciesSize);

    HRPHistogram::COLOR_SPACE AutoSelectColorSpace(uint32_t pi_ChannelCount);

    HArrayAutoPtr<HArrayAutoPtr<uint32_t> >
    m_pEntryFrequencies;
    uint32_t        m_EntryFrequenciesSize;
    uint32_t        m_ChannelCount;

    COLOR_SPACE     m_ColorSpace;

    // Methods
    void            DeepDelete();
    void            DeepCopy(const HRPHistogram& pi_rObj);
    };
END_IMAGEPP_NAMESPACE

#include "HRPHistogram.hpp"

