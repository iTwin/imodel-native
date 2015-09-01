//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRAImageOpContrastStretchFilter.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

#include <ImagePP/all/h/HRAImageOp.h>

IMAGEPP_REF_COUNTED_PTR(HRAImageOpLightnessContrastStretchFilter)

BEGIN_IMAGEPP_NAMESPACE

/*----------------------------------------------------------------------------+
|struct HRAImageOpLightnessContrastStretchFilter
+----------------------------------------------------------------------------*/
struct HRAImageOpLightnessContrastStretchFilter : public HRAImageOp
{
public:
    struct LightnessContrastStretchFilter
        {
        LightnessContrastStretchFilter(){};
        virtual ~LightnessContrastStretchFilter(){};
        virtual ImagePPStatus _Apply(HRAImageSampleR out, HRAImageSampleCR inputData) = 0;
        };

    //! Create a generic ContrastStretchFilter
    static HRAImageOpLightnessContrastStretchFilterPtr CreateLightnessContrastStretchFilter();

    // Get/Set methods
    // MinValue and MaxValue are in the interval [0.0, 100.0]
    void GetInterval(float& MinValue, float& MaxValue) const;
    void SetInterval(float  MinValue, float  MaxValue);

    // MinContrastValue and MaxContrastValue are in the interval [0.0, 100.0]
    void GetContrastInterval(float& MinContrastValue, float& MaxContrastValue) const;
    void SetContrastInterval(float  MinContrastValue, float  MaxContrastValue);

    // GammaFactor is in the interval ]0.0, 10.0] (notice the exclusion of 0.0)
    void GetGammaFactor(double& GammaFactor) const;
    void SetGammaFactor(double GammaFactor);

protected:
    virtual ImagePPStatus _GetAvailableInputPixelType(HFCPtr<HRPPixelType>& pixelType, uint32_t index, const HFCPtr<HRPPixelType> pixelTypeToMatch) override;
    virtual ImagePPStatus _GetAvailableOutputPixelType(HFCPtr<HRPPixelType>& pixelType, uint32_t index, const HFCPtr<HRPPixelType> pixelTypeToMatch) override;
    virtual ImagePPStatus _SetInputPixelType(HFCPtr<HRPPixelType> pixelType) override;
    virtual ImagePPStatus _SetOutputPixelType(HFCPtr<HRPPixelType> pixelType) override;
    virtual ImagePPStatus _Process(HRAImageSampleR out, HRAImageSampleCR inputData, ImagepOpParams& params) override;

    bool IsSupportedPixeltype(HRPPixelType const& pixeltype) const;

private:
    HRAImageOpLightnessContrastStretchFilter();
    virtual ~HRAImageOpLightnessContrastStretchFilter();

    ImagePPStatus UpdateFilter();

    float m_MinValue;
    float m_MaxValue;
    float m_MinContrastValue;
    float m_MaxContrastValue;

    double m_GammaFactor;
    std::unique_ptr<LightnessContrastStretchFilter> m_pFilter;

};
END_IMAGEPP_NAMESPACE
