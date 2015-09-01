//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRAImageOpDensitySlicingFilter.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

#include <Imagepp/all/h/HRAImageOp.h>


IMAGEPP_REF_COUNTED_PTR(HRAImageOpDensitySlicingFilter)
IMAGEPP_REF_COUNTED_PTR(HRAImageOpLightnessDensitySlicingFilter)

BEGIN_IMAGEPP_NAMESPACE

/*----------------------------------------------------------------------------+
|struct HRAImageOpBaseDensitySlicingFilter
+----------------------------------------------------------------------------*/
struct HRAImageOpBaseDensitySlicingFilter : public HRAImageOp
{
public:

    enum PixelDepth
        {
        PIXELDEPTH_8bits = 8,       // range [0, 255]
        PIXELDEPTH_16bits = 16,     // range [0, 65535]
        };

    template<class ValueType_T>
    struct SliceInfos_T
        {
        ValueType_T m_StartValue;
        ValueType_T m_EndValue;
        uint32_t    m_StartColor;    // RGB8
        uint32_t    m_EndColor;      // RGB8
        double      m_Opacity;       // [0, 1]
        };

    struct DensitySlicingFilter
        {
        DensitySlicingFilter(){};
        virtual ~DensitySlicingFilter(){};
        virtual ImagePPStatus _Apply(HRAImageSampleR out, HRAImageSampleCR inputData) = 0;
        };

    //! a desaturation factor. Range is [0,1]
    void SetDesaturationFactor(double desaturationFactor);
    double GetDesaturationFactor() const;

protected:
    virtual ImagePPStatus _GetAvailableInputPixelType(HFCPtr<HRPPixelType>& pixelType, uint32_t index, const HFCPtr<HRPPixelType> pixelTypeToMatch) override;
    virtual ImagePPStatus _GetAvailableOutputPixelType(HFCPtr<HRPPixelType>& pixelType, uint32_t index, const HFCPtr<HRPPixelType> pixelTypeToMatch) override;
    virtual ImagePPStatus _SetInputPixelType(HFCPtr<HRPPixelType> pixelType) override;
    virtual ImagePPStatus _SetOutputPixelType(HFCPtr<HRPPixelType> pixelType) override;
    virtual ImagePPStatus _Process(HRAImageSampleR out, HRAImageSampleCR inputData, ImagepOpParams& params) override;

    bool IsSupportedPixeltype(HRPPixelType const& pixeltype) const;
   
    HRAImageOpBaseDensitySlicingFilter(PixelDepth depth);
    virtual ~HRAImageOpBaseDensitySlicingFilter();

    PixelDepth m_pixelDepth;
    double m_DesaturationFactor;
    std::unique_ptr<DensitySlicingFilter> m_pFilter;
    
    virtual ImagePPStatus _UpdateFilter() = 0;
};

/*----------------------------------------------------------------------------+
|struct HRAImageOpDensitySlicingFilter
+----------------------------------------------------------------------------*/
struct HRAImageOpDensitySlicingFilter : public HRAImageOpBaseDensitySlicingFilter
{
public:
    typedef SliceInfos_T<uint32_t> SliceInfos;        
    typedef bvector<SliceInfos> SliceList;

    //! Create a density slicing filter. Pixel depth is used to specify the domain of slice values and the pixel depth of produced output.
    //! PIXELDEPTH_8bits  will have a range of [0,255]. 
    //! PIXELDEPTH_16bits will have a range of [0, 65535].
    static HRAImageOpDensitySlicingFilterPtr CreateDensitySlicingFilter(PixelDepth depth);    

    //! Add a slice. 
    //! StartValue and EndValue range is PixelDepth dependent.
    //! StartColor and EndColor are RGB8. 
    //! Opacity range is [0,1].
    uint32_t AddSlice (uint32_t StartValue, uint32_t EndValue, uint32_t StartColor, uint32_t EndColor, double Opacity);

    //! Remove all slices.
    void ClearSlices();
    
    //! Return current slices list.
    SliceList const& GetSlices() const;
 
private:
    HRAImageOpDensitySlicingFilter(PixelDepth depth);
    virtual ~HRAImageOpDensitySlicingFilter();

    virtual ImagePPStatus _UpdateFilter() override;

    SliceList m_SliceList;
};

/*----------------------------------------------------------------------------+
|struct HRAImageOpLightnessDensitySlicingFilter
+----------------------------------------------------------------------------*/
struct HRAImageOpLightnessDensitySlicingFilter : public HRAImageOpBaseDensitySlicingFilter
{
public:
    typedef SliceInfos_T<float> SliceInfos;
    typedef bvector<SliceInfos> SliceList;

    //! Create a LightnessDensitySlicingFilter. Pixels depth is used to specify the depth of the produced output.
    //! Lightness slice values have a range of [0, 100].
    static HRAImageOpLightnessDensitySlicingFilterPtr CreateLightnessDensitySlicingFilter(PixelDepth depth);

    //! Add a slice. 
    //! StartValue and EndValue range is [0, 100].
    //! StartColor and EndColor are RGB8. 
    //! Opacity range is [0,1].
    uint32_t AddSlice (float StartValue, float EndValue, uint32_t StartColor, uint32_t EndColor, double Opacity);

    //! Remove all slices.
    void ClearSlices();

    //! Return current slices list.
    SliceList const& GetSlices() const;

private:
    HRAImageOpLightnessDensitySlicingFilter(PixelDepth depth);
    virtual ~HRAImageOpLightnessDensitySlicingFilter();

    virtual ImagePPStatus _UpdateFilter() override;

    SliceList m_SliceList;
};

END_IMAGEPP_NAMESPACE
