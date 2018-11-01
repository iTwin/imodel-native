//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRAImageOpConvFilter.h $
//:>
//:>  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#pragma once

#include <ImagePP/all/h/HRAImageOp.h>

BEGIN_IMAGEPP_NAMESPACE 

/*----------------------------------------------------------------------------+
|struct HRAImageOpConvolutionFilter
+----------------------------------------------------------------------------*/
struct HRAImageOpConvolutionFilter : public HRAImageOp
{
public:

    typedef bvector<bvector<int32_t> > Kernel;

    struct ConvFilter
        {
        ConvFilter();
        virtual ~ConvFilter();

        virtual ImagePPStatus _Init(HRAImageOpConvolutionFilter::Kernel const& kernel, HRPPixelNeighbourhood const& pixelNeighbourdhood) = 0;

        virtual ImagePPStatus _Apply(HRAImageSampleR out, HRAImageSampleCR inputData) = 0;
        };

    //! Create a generic convolution filter.
    IMAGEPPTEST_EXPORT static HRAImageOpPtr CreateCustomConvolutionFilter(Kernel const& kernel, HRPPixelNeighbourhood const& pixelNeighbourhood);

    //! Create a average filter. 
    IMAGEPPTEST_EXPORT static HRAImageOpPtr CreateAverageFilter();

    //! Create a blur filter. 
    IMAGEPPTEST_EXPORT static HRAImageOpPtr CreateBlurFilter(uint8_t intensity);

    //! Create a detail filter. 
    IMAGEPPTEST_EXPORT static HRAImageOpPtr CreateDetailFilter();

    //! Create a edge enhance filter. 
    IMAGEPPTEST_EXPORT static HRAImageOpPtr CreateEdgeEnhanceFilter();

    //! Create a find edge filter. 
    IMAGEPPTEST_EXPORT static HRAImageOpPtr CreateFindEdgeFilter();

    //! Create a sharpen filter. 
    IMAGEPPTEST_EXPORT static HRAImageOpPtr CreateSharpenFilter(uint8_t intensity);

    //! Create a smooth filter. 
    IMAGEPPTEST_EXPORT static HRAImageOpPtr CreateSmoothFilter();

    IMAGEPPTEST_EXPORT void SetKernel(Kernel const& kernel, HRPPixelNeighbourhood const& neighborhood);

protected:
    virtual ImagePPStatus _GetAvailableInputPixelType(HFCPtr<HRPPixelType>& pixelType, uint32_t index, const HFCPtr<HRPPixelType> pixelTypeToMatch) override;

    virtual ImagePPStatus _GetAvailableOutputPixelType(HFCPtr<HRPPixelType>& pixelType, uint32_t index, const HFCPtr<HRPPixelType> pixelTypeToMatch) override;

    virtual ImagePPStatus _SetInputPixelType(HFCPtr<HRPPixelType> pixelType) override;

    virtual ImagePPStatus _SetOutputPixelType(HFCPtr<HRPPixelType> pixelType) override;

    virtual ImagePPStatus _Process(HRAImageSampleR out, HRAImageSampleCR inputData, ImagepOpParams& params) override;

private:

    ImagePPStatus UpdateFilter();

    ConvFilter* CreateFilter(HRPPixelType const& pixelType) const;

    bool IsSupportedPixeltype(HRPPixelType const& pixelType) const;

    HRAImageOpConvolutionFilter();
    virtual ~HRAImageOpConvolutionFilter();
    
    Kernel m_kernel;

    std::unique_ptr<ConvFilter> m_pFilter;
};

END_IMAGEPP_NAMESPACE
