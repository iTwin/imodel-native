//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for class HIMFilteredImage
//-----------------------------------------------------------------------------

#include <ImageppInternal.h>


#include <ImagePP/all/h/HIMFilteredImage.h>
#include <ImagePP/all/h/HRARepPalParms.h>
#include <ImagePP/all/h/HGF2DGrid.h>
#include <ImagePP/all/h/HRAHistogramOptions.h>
#include <ImagePP/all/h/HGF2DIdentity.h>
#include <ImagePP/all/h/HIMFilteredImage.h>
#include <ImagePP/all/h/HRPPixelTypeV32R8G8B8A8.h>
#include <ImagePP/all/h/HRPPixelTypeV24R8G8B8.h>
#include <ImagePP/all/h/HFCException.h>
#include <ImagePP/all/h/HRPTypeAdaptFilter.h>
#include <ImagePP/all/h/HFCGrid.h>
#include <ImagePP/all/h/HGF2DTranslation.h>
#include <ImagePP/all/h/HRPComplexFilter.h>
#include <ImagePP/all/h/HRPFilter.h>
#include <ImagePP/all/h/HRAStoredRaster.h>
#include <ImagePP/all/h/HRPPixelType.h>
#include <ImagePP/all/h/HRAImageOp.h>
#include <ImagePP/all/h/HRABitmap.h>
#include <ImagePP/all/h/HRATransaction.h>
#include <ImagePPInternal/gra/HRACopyToOptions.h>

//&&MM temp: to review when removing s_AddHRPFilterToPipeline
#include <ImagePP/all/h/HPMPool.h>
#include <ImagePP/all/h/HRADEMRaster.h>
#include <ImagePP/all/h/HIMFilteredImage.h>
#include <ImagePP/all/h/HIMFilteredImage.h>
#include <ImagePP/all/h/HGFScanlines.h>
#include <ImagePP/all/h/HGF2DTranslation.h>
#include <ImagePP/all/h/HRPMapFilters8.h>
#include <ImagePP/all/h/HRPMapFilters16.h>
#include <ImagePP/all/h/HRAImageOp.h>
#include <ImagePP/all/h/HRAImageOpConvFilter.h>
#include <ImagePPInternal/gra/HRAImageSampler.h>
#include <ImagePP/all/h/HRAImageOpMapFilters.h>
#include <ImagePP/all/h/HRPPixelTypeV24R8G8B8.h>
#include <ImagePP/all/h/HRPPixelTypeV32R8G8B8A8.h>
#include <ImagePP/all/h/HCDPacket.h>
#include <ImagePP/all/h/HRPConvFilterV24R8G8B8.h>
#include <ImagePP/all/h/HGF2DIdentity.h>
#include <ImagePP/all/h/HRPFunctionFilters.h>
#include <ImagePP/all/h/HRAImageOpFunctionFilters.h>
#include <ImagePP/all/h/HRAImageOpDensitySlicingFilter.h>
#include <ImagePP/all/h/HRPDensitySlicingFilter.h>
#include <ImagePP/all/h/HRPDensitySlicingFilter8.h>
#include <ImagePP/all/h/HRPDensitySlicingFilter16.h>
#include <ImagePP/all/h/HRPLigthnessDensitySlicingFilter.h>
#include <ImagePP/all/h/HRPLigthnessDensitySlicingFilter8.h>
#include <ImagePP/all/h/HRPLigthnessDensitySlicingFilter16.h>
#include <ImagePP/all/h/HRAImageOpContrastStretchFilter.h>
#include <ImagePP/all/h/HRPContrastStretchFilter8.h>
#include <ImagePP/all/h/HRPContrastStretchFilter16.h>
#include <ImagePP/all/h/HRPLigthnessContrastStretch.h>
#include <ImagePP/all/h/HRPLigthnessContrastStretch8.h>
#include <ImagePP/all/h/HRPLigthnessContrastStretch16.h>
#include <ImagePP/all/h/HRAStoredRaster.h>
#include <ImagePP/all/h/HRPComplexFilter.h>
#include <ImagePP/all/h/HRPDEMFilter.h>
#include <ImagePPInternal/gra/HRAImageNode.h>



//&&MM need to think about active image that does things like:
// HFCPtr<HRPFilter> pNewFilter = ((HFCPtr<HIMFilteredImage>&)pRaster)->GetFilter()->ComposeWith(pi_rpFilter);
// in case of 2 map filters they will merge and modify the internal map but the result will still be a specific class ex. contrast. 

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
void s_AddHRPFilterToPipeline(ImageOpList& imageOpList, HRPFilter& filter)
    {
    if(filter.IsCompatibleWith(HRPComplexFilter::CLASS_ID))
        {
        HRPComplexFilter* pComplexFilter = static_cast<HRPComplexFilter*>(&filter);

        for(HRPComplexFilter::ListFilters::const_iterator itr(pComplexFilter->GetList().begin()); itr != pComplexFilter->GetList().end(); ++itr)
            s_AddHRPFilterToPipeline(imageOpList, **itr);
        }
    else if(filter.IsCompatibleWith(HRPConvFilterV24R8G8B8::CLASS_ID))
        {
        HRPConvFilterV24R8G8B8* pConvFilter = static_cast<HRPConvFilterV24R8G8B8*>(&filter);

        HRAImageOpConvolutionFilter::Kernel kernel(pConvFilter->GetNeighbourhood().GetHeight());
        for(uint32_t y=0; y < pConvFilter->GetNeighbourhood().GetHeight(); ++y)
            for(uint32_t x=0; x < pConvFilter->GetNeighbourhood().GetWidth(); ++x)
                {
                kernel[y].push_back(pConvFilter->GetWeightMatrix()[pConvFilter->GetNeighbourhood().GetWidth()*y +x]);
                }

        HRAImageOpPtr pNewOp = HRAImageOpConvolutionFilter::CreateCustomConvolutionFilter(kernel, pConvFilter->GetNeighbourhood());
        imageOpList.push_back(pNewOp);
        }
    else if(filter.IsCompatibleWith(HRPAlphaReplacer::CLASS_ID))
        {
        // Alpha replacer filter
        HRPAlphaReplacer* alphaReplacerFilter = static_cast<HRPAlphaReplacer*>(&filter);
        Byte defaultAlpha = alphaReplacerFilter->GetDefaultAlpha();

        VectorHRPAlphaRange ranges;
        ListHRPAlphaRange HRPranges = alphaReplacerFilter->GetAlphaRanges();
        for(uint32_t i=0; i<HRPranges.size(); i++)
            ranges.push_back(HRPranges[i]);

        HRAImageOpPtr pNewOp = HRAImageOpAlphaReplacerFilter::CreateAlphaReplacerFilter (defaultAlpha, ranges);
        imageOpList.push_back(pNewOp);
        }
    else if(filter.IsCompatibleWith(HRPAlphaComposer::CLASS_ID))
        {
        // Alpha composer filter
        HRPAlphaComposer* alphaComposerFilter = static_cast<HRPAlphaComposer*>(&filter);
        Byte defaultAlpha = alphaComposerFilter->GetDefaultAlpha();

        VectorHRPAlphaRange ranges;
        ListHRPAlphaRange HRPranges = alphaComposerFilter->GetAlphaRanges();
        for(uint32_t i=0; i<HRPranges.size(); i++)
            ranges.push_back(HRPranges[i]);

        HRAImageOpPtr pNewOp = HRAImageOpAlphaComposerFilter::CreateAlphaComposerFilter (defaultAlpha, ranges);
        imageOpList.push_back(pNewOp);
        }
    else if(filter.IsCompatibleWith(HRPHistogramScalingFilter::CLASS_ID))
        {
        HRPHistogramScalingFilter* pHRPHistoFilter = static_cast<HRPHistogramScalingFilter*>(&filter);

        // An histogram scaling is a subset of a contrast stretch.
        HRAImageOpContrastStretchFilterPtr pNewOp = HRAImageOpContrastStretchFilter::CreateContrastStretchFilter(*pHRPHistoFilter->GetFilterPixelType());

        uint32_t channelCount = pHRPHistoFilter->GetFilterPixelType()->GetChannelOrg().CountChannels();

        for(uint32_t i=0; i < channelCount; ++i)
            {
            uint16_t minValue, maxValue;
            pHRPHistoFilter->GetInterval((uint16_t)i, &minValue, &maxValue);

            if(pHRPHistoFilter->GetScalingMode() == HRPHistogramScalingFilter::INPUT_RANGE_CLIPPING)
                pNewOp->SetInterval((uint16_t)i, minValue, maxValue);
            else
                pNewOp->SetContrastInterval((uint16_t)i, minValue, maxValue);
            }

        imageOpList.push_back(pNewOp);
        }
    else if(((filter.IsCompatibleWith(HRPContrastStretchFilter8::CLASS_ID) || (filter.IsCompatibleWith(HRPContrastStretchFilter16::CLASS_ID)))))
        {
        int32_t MinValue[4];
        int32_t MaxValue[4];
        int32_t MinContrastValue[4];
        int32_t MaxContrastValue[4];
        double GammaFactor[4];

        uint32_t channelCount = 0;
        HFCPtr<HRPPixelType> pPixelType;

        if(filter.IsCompatibleWith(HRPContrastStretchFilter8::CLASS_ID))
            {
            HRPContrastStretchFilter8* pHRPContrastStretchFilter = static_cast<HRPContrastStretchFilter8*>(&filter);

            pPixelType = pHRPContrastStretchFilter->GetFilterPixelType();
            channelCount = pHRPContrastStretchFilter->GetFilterPixelType()->GetChannelOrg().CountChannels();

            for(uint32_t i=0; i < channelCount; ++i)
                {
                pHRPContrastStretchFilter->GetContrastInterval(i, &MinContrastValue[i], &MaxContrastValue[i]);
                pHRPContrastStretchFilter->GetInterval(i, &MinValue[i], &MaxValue[i]);    
                pHRPContrastStretchFilter->GetGammaFactor(i, &GammaFactor[i]);
                }
            }
        else if(filter.IsCompatibleWith(HRPContrastStretchFilter16::CLASS_ID))
            {
            HRPContrastStretchFilter16* pHRPContrastStretchFilter = static_cast<HRPContrastStretchFilter16*>(&filter);

            pPixelType = pHRPContrastStretchFilter->GetFilterPixelType();
            channelCount = pHRPContrastStretchFilter->GetFilterPixelType()->GetChannelOrg().CountChannels();

            for(uint32_t i=0; i < channelCount; ++i)
                {
                pHRPContrastStretchFilter->GetContrastInterval(i, &MinContrastValue[i], &MaxContrastValue[i]);
                pHRPContrastStretchFilter->GetInterval(i, &MinValue[i], &MaxValue[i]);    
                pHRPContrastStretchFilter->GetGammaFactor(i, &GammaFactor[i]);
                }
            }

        HRAImageOpContrastStretchFilterPtr pNewOp = HRAImageOpContrastStretchFilter::CreateContrastStretchFilter(*pPixelType);

        for(uint16_t i=0; i < channelCount; ++i)
            {
            pNewOp->SetInterval(i, (uint16_t)MinValue[i], (uint16_t)MaxValue[i]);
            pNewOp->SetContrastInterval(i, (uint16_t)MinContrastValue[i], (uint16_t)MaxContrastValue[i]);
            pNewOp->SetGammaFactor(i, GammaFactor[i]);
            }

        imageOpList.push_back(pNewOp);
        }
    else if(((filter.IsCompatibleWith(HRPLigthnessContrastStretch8::CLASS_ID) || (filter.IsCompatibleWith(HRPLigthnessContrastStretch16::CLASS_ID)))))
        {
        HRAImageOpPtr pNewOp = HRAImageOpLightnessContrastStretchFilter::CreateLightnessContrastStretchFilter();
        HRAImageOpLightnessContrastStretchFilter* pLightnessContrastStretchFilter = static_cast<HRAImageOpLightnessContrastStretchFilter*>(pNewOp.get());

        int32_t MinValue;
        int32_t MaxValue;
        int32_t MinContrastValue;
        int32_t MaxContrastValue;
        double GammaFactor=0.0;

        float normalizedMinValue=0.0;
        float normalizedMaxValue=0.0;
        float normalizedMinContrastValue=0.0;
        float normalizedMaxContrastValue=0.0;
        if(filter.IsCompatibleWith(HRPLigthnessContrastStretch8::CLASS_ID))
            {
            HRPLigthnessContrastStretch8* pHRPLightnessContrastStretchFilter = static_cast<HRPLigthnessContrastStretch8*>(&filter);
            pHRPLightnessContrastStretchFilter->GetContrastInterval(0, &MinContrastValue, &MaxContrastValue);
            pHRPLightnessContrastStretchFilter->GetInterval(0, &MinValue, &MaxValue);
            pHRPLightnessContrastStretchFilter->GetGammaFactor(0, &GammaFactor);

            // Normalize values between 0 and 100 because HRAImageOpLightnessContrastStretchFilter expects that (while HRPLigthnessContrastStretch8 expects
            // values between 0 and 255).
            normalizedMinValue = (float) MinValue / 255 * 100;
            normalizedMaxValue = (float) MaxValue / 255 * 100;
            normalizedMinContrastValue = (float) MinContrastValue / 255 * 100;
            normalizedMaxContrastValue = (float) MaxContrastValue / 255 * 100;
            }
        else if(filter.IsCompatibleWith(HRPLigthnessContrastStretch16::CLASS_ID))
            {
            HRPLigthnessContrastStretch16* pHRPLightnessContrastStretchFilter = static_cast<HRPLigthnessContrastStretch16*>(&filter);
            pHRPLightnessContrastStretchFilter->GetContrastInterval(0, &MinContrastValue, &MaxContrastValue);
            pHRPLightnessContrastStretchFilter->GetInterval(0, &MinValue, &MaxValue);
            pHRPLightnessContrastStretchFilter->GetGammaFactor(0, &GammaFactor);          

            // Normalize values between 0 and 100 because HRAImageOpLightnessContrastStretchFilter expects that (while HRPLigthnessContrastStretch8 expects
            // values between 0 and 65535).
            normalizedMinValue = (float) MinValue / 65535 * 100;
            normalizedMaxValue = (float) MaxValue / 65535 * 100;
            normalizedMinContrastValue =(float) MinContrastValue / 65535 * 100;
            normalizedMaxContrastValue = (float) MaxContrastValue / 65535 * 100;
            }
        pLightnessContrastStretchFilter->SetInterval(normalizedMinValue, normalizedMaxValue);
        pLightnessContrastStretchFilter->SetContrastInterval(normalizedMinContrastValue, normalizedMaxContrastValue);
        pLightnessContrastStretchFilter->SetGammaFactor(GammaFactor);

        imageOpList.push_back(pNewOp);
        }

    else if(((filter.IsCompatibleWith(HRPDensitySlicingFilter8::CLASS_ID)) || (filter.IsCompatibleWith(HRPDensitySlicingFilter16::CLASS_ID))))
        {
        HRAImageOpDensitySlicingFilterPtr pNewOp;

        int32_t StartIndex;
        int32_t EndIndex;
        int32_t StartColor;
        int32_t EndColor;
        int32_t Opacity;

        if(filter.IsCompatibleWith(HRPDensitySlicingFilter8::CLASS_ID))
            {
            pNewOp = HRAImageOpDensitySlicingFilter::CreateDensitySlicingFilter(HRAImageOpBaseDensitySlicingFilter::PIXELDEPTH_8bits);

            HRPDensitySlicingFilter8* pHRPDensitySlicingFilter = static_cast<HRPDensitySlicingFilter8*>(&filter);
            for(int32_t i(0); i<pHRPDensitySlicingFilter->GetSliceCount(); i++)
                {
                pHRPDensitySlicingFilter->GetSliceInfo(i, &StartIndex, &EndIndex, &StartColor, &EndColor, &Opacity);
                pNewOp->AddSlice(StartIndex, EndIndex, StartColor, EndColor, Opacity/100.0);
                pNewOp->SetDesaturationFactor(pHRPDensitySlicingFilter->GetDesaturationFactor()/100.0);
                }
            }
        else if(filter.IsCompatibleWith(HRPDensitySlicingFilter16::CLASS_ID))
            {
            pNewOp = HRAImageOpDensitySlicingFilter::CreateDensitySlicingFilter(HRAImageOpBaseDensitySlicingFilter::PIXELDEPTH_16bits);

            HRPDensitySlicingFilter16* pHRPDensitySlicingFilter = static_cast<HRPDensitySlicingFilter16*>(&filter);
            for(int32_t i(0); i<pHRPDensitySlicingFilter->GetSliceCount(); i++)
                {
                pHRPDensitySlicingFilter->GetSliceInfo(i, &StartIndex, &EndIndex, &StartColor, &EndColor, &Opacity);
                pNewOp->AddSlice(StartIndex, EndIndex, StartColor, EndColor, Opacity/100.0);
                pNewOp->SetDesaturationFactor(pHRPDensitySlicingFilter->GetDesaturationFactor()/100.0);
                }
            }          
        imageOpList.push_back(pNewOp);
        }
    else if(((filter.IsCompatibleWith(HRPLigthnessDensitySlicingFilter8::CLASS_ID)) || (filter.IsCompatibleWith(HRPLigthnessDensitySlicingFilter16::CLASS_ID))))
        {    
        HRAImageOpLightnessDensitySlicingFilterPtr pNewOp;

        int32_t StartIndex;
        int32_t EndIndex;
        int32_t StartColor;
        int32_t EndColor;
        int32_t Opacity;

        if(filter.IsCompatibleWith(HRPLigthnessDensitySlicingFilter8::CLASS_ID))
            {
            pNewOp = HRAImageOpLightnessDensitySlicingFilter::CreateLightnessDensitySlicingFilter(HRAImageOpBaseDensitySlicingFilter::PIXELDEPTH_8bits);

            HRPLigthnessDensitySlicingFilter8* pHRPLigthnessDensitySlicingFilter = static_cast<HRPLigthnessDensitySlicingFilter8*>(&filter);
            for(int32_t i(0); i<pHRPLigthnessDensitySlicingFilter->GetSliceCount(); i++)
                {
                pHRPLigthnessDensitySlicingFilter->GetSliceInfo(i, &StartIndex, &EndIndex, &StartColor, &EndColor, &Opacity);
                pNewOp->AddSlice(StartIndex/2.55F, EndIndex/2.55F, StartColor, EndColor, Opacity/100.0);
                pNewOp->SetDesaturationFactor(pHRPLigthnessDensitySlicingFilter->GetDesaturationFactor()/100.0);
                }
            }
        else if(filter.IsCompatibleWith(HRPLigthnessDensitySlicingFilter16::CLASS_ID))
            {
            pNewOp = HRAImageOpLightnessDensitySlicingFilter::CreateLightnessDensitySlicingFilter(HRAImageOpBaseDensitySlicingFilter::PIXELDEPTH_16bits);

            HRPLigthnessDensitySlicingFilter16* pHRPLigthnessDensitySlicingFilter = static_cast<HRPLigthnessDensitySlicingFilter16*>(&filter);
            for(int32_t i(0); i<pHRPLigthnessDensitySlicingFilter->GetSliceCount(); i++)
                {
                pHRPLigthnessDensitySlicingFilter->GetSliceInfo(i, &StartIndex, &EndIndex, &StartColor, &EndColor, &Opacity);
                pNewOp->AddSlice(StartIndex/655.35F, EndIndex/655.35F, StartColor, EndColor, Opacity/100.0);
                pNewOp->SetDesaturationFactor(pHRPLigthnessDensitySlicingFilter->GetDesaturationFactor()/100.0);
                }
            }
        imageOpList.push_back(pNewOp);
        }
    else if(filter.IsCompatibleWith(HRPColorReplacerFilter::CLASS_ID))
        {
        HRPColorReplacerFilter* colorReplacerFilter = static_cast<HRPColorReplacerFilter*>(&filter);

        HRAImageOpColorReplacerFilter::RGBSetList selectedRGBSet;
        HRPColorReplacerFilter::RGBSetList const& HRPRGBSetList = colorReplacerFilter->GetSelectedRGBSet();
        for(HRPColorReplacerFilter::RGBSetList::const_iterator iterator = HRPRGBSetList.begin(); iterator != HRPRGBSetList.end(); ++iterator)
            selectedRGBSet.push_back(*iterator);

        HRAImageOpColorReplacerFilter::RGBSetList removeRGBSet;
        HRPColorReplacerFilter::RGBSetList const& HRPRGBRemoveList = colorReplacerFilter->GetSelectedRemoveRGBSet();
        for(HRPColorReplacerFilter::RGBSetList::const_iterator iterator = HRPRGBRemoveList.begin(); iterator != HRPRGBRemoveList.end(); ++iterator)
            removeRGBSet.push_back(*iterator);

        HRAImageOpPtr pNewOp = HRAImageOpColorReplacerFilter::CreateColorReplacerFilter(*((const Byte(*)[])(colorReplacerFilter->GetNewColor())), selectedRGBSet, removeRGBSet);

        imageOpList.push_back(pNewOp);
        }
    else if(filter.IsCompatibleWith(HRPColortwistFilter::CLASS_ID))
        {
        const double (*pMatrix)[][4] = (const double(*)[][4])(static_cast<HRPColortwistFilter*>(&filter)->GetMatrix());
        HRAImageOpPtr pNewOp = HRAImageOpColortwistFilter::CreateColortwistFilter(*pMatrix);
        imageOpList.push_back(pNewOp);
        }
    else if(filter.IsCompatibleWith(HRPColorBalanceFilter::CLASS_ID))
        {
        HRAImageOpPtr pNewOp = HRAImageOpBrightnessFilter::CreateBrightnessFilter(static_cast<HRPColorBalanceFilter*>(&filter)->GetRedVariation() / 255.0);
        imageOpList.push_back(pNewOp);
        }
    else if(filter.IsCompatibleWith(HRPColorBalanceFilter16::CLASS_ID))
        {
        HRAImageOpPtr pNewOp = HRAImageOpBrightnessFilter::CreateBrightnessFilter(static_cast<HRPColorBalanceFilter16*>(&filter)->GetRedVariation() / 65535.0);
        imageOpList.push_back(pNewOp);
        }
    else if(filter.IsCompatibleWith(HRPGammaFilter::CLASS_ID))
        {
        HRAImageOpPtr pNewOp = HRAImageOpGammaFilter::CreateGammaFilter(static_cast<HRPGammaFilter*>(&filter)->GetGamma());
        imageOpList.push_back(pNewOp);
        }
        else if(filter.IsCompatibleWith(HRPGammaFilter16::CLASS_ID))
        {
        HRAImageOpPtr pNewOp = HRAImageOpGammaFilter::CreateGammaFilter(static_cast<HRPGammaFilter16*>(&filter)->GetGamma());
        imageOpList.push_back(pNewOp);
        }
    else if(filter.IsCompatibleWith(HRPInvertFilter16::CLASS_ID) ||
                filter.IsCompatibleWith(HRPInvertFilter::CLASS_ID))
        {
        HRAImageOpPtr pNewOp = HRAImageOpInvertFilter::CreateInvertFilter();
        imageOpList.push_back(pNewOp);
        }
        else if(filter.IsCompatibleWith(HRPTintFilter::CLASS_ID) || 
                filter.IsCompatibleWith(HRPTintFilter16::CLASS_ID))
        {
        Byte rgb8[3];

        // Assume always RGB for tint. Tint is a factor so index 1 will give us the tint factor.
        if(filter.IsCompatibleWith(HRPTintFilter::CLASS_ID))
            {                
            rgb8[0] = static_cast<HRPTintFilter*>(&filter)->GetMap(0)[255];     
            rgb8[1] = static_cast<HRPTintFilter*>(&filter)->GetMap(1)[255];
            rgb8[2] = static_cast<HRPTintFilter*>(&filter)->GetMap(2)[255];
            }
        else
            {
            rgb8[0] = static_cast<HRPTintFilter16*>(&filter)->GetMap(0)[65535] >> 8;     
            rgb8[1] = static_cast<HRPTintFilter16*>(&filter)->GetMap(1)[65535] >> 8;
            rgb8[2] = static_cast<HRPTintFilter16*>(&filter)->GetMap(2)[65535] >> 8;
            }

        HRAImageOpPtr pNewOp = HRAImageOpTintFilter::CreateTintFilter(rgb8);
        imageOpList.push_back(pNewOp);
        }
    else if(filter.IsCompatibleWith(HRPContrastFilter::CLASS_ID))
        {
        int8_t contrast = static_cast<HRPContrastFilter*>(&filter)->GetIntensity();
        double normalizedContrast = contrast < 0 ? contrast/128.0 : contrast/127.0;  
        HRAImageOpPtr pNewOp = HRAImageOpContrastFilter::CreateContrastFilter(normalizedContrast);
        imageOpList.push_back(pNewOp);
        }
    else if(filter.IsCompatibleWith(HRPContrastFilter16::CLASS_ID))
        {
        int16_t contrast = static_cast<HRPContrastFilter16*>(&filter)->GetIntensity();
        double normalizedContrast = contrast < 0 ? contrast/32768.0 : contrast/32512.0;  
        HRAImageOpPtr pNewOp = HRAImageOpContrastFilter::CreateContrastFilter(normalizedContrast);
        imageOpList.push_back(pNewOp);
        }
    }

HPM_REGISTER_CLASS(HIMFilteredImage, HRAImageView)

//-----------------------------------------------------------------------------
// public
// constructor
//-----------------------------------------------------------------------------
HIMFilteredImage::HIMFilteredImage()
    :HRAImageView(HFCPtr<HRARaster>())
    {
    //LinkTo(GetSource());
    }

//-----------------------------------------------------------------------------
// public
// constructor
//-----------------------------------------------------------------------------
HIMFilteredImage::HIMFilteredImage(const HFCPtr<HRARaster>& pi_pSource,
                                   const HFCPtr<HRPFilter>& pi_rpFilter,
                                   const HFCPtr<HRPPixelType> pi_pPixelType)
    :HRAImageView(pi_pSource)
    {
    HPRECONDITION(pi_rpFilter != 0);

    m_pFilter = pi_rpFilter;
    m_pPixelType = pi_pPixelType;

    s_AddHRPFilterToPipeline(m_imageOps, *m_pFilter);

    InitObject();
    //LinkTo(GetSource());
    }


//-----------------------------------------------------------------------------
// public
// Copy constructor
//-----------------------------------------------------------------------------
HIMFilteredImage::HIMFilteredImage(const HIMFilteredImage& pi_rFilteredImage)
    :HRAImageView(pi_rFilteredImage)
    {
    m_pFilter = pi_rFilteredImage.m_pFilter->Clone();
    HPOSTCONDITION(m_pFilter != 0);

    s_AddHRPFilterToPipeline(m_imageOps, *m_pFilter);

    if (pi_rFilteredImage.m_pPixelType != 0)
        {
        m_pPixelType = (HRPPixelType*)pi_rFilteredImage.m_pPixelType->Clone();
        HPOSTCONDITION(m_pPixelType != 0);
        }
    }

//-----------------------------------------------------------------------------
// public
// destructor
//-----------------------------------------------------------------------------
HIMFilteredImage::~HIMFilteredImage()
    {
    DeepDelete();
    //UnlinkFrom(GetSource());
    }

//-----------------------------------------------------------------------------
// public
// Clone
//-----------------------------------------------------------------------------
HFCPtr<HRARaster> HIMFilteredImage::Clone (HPMObjectStore* pi_pStore, HPMPool* pi_pLog) const
    {
    return new HIMFilteredImage(*this);
    }
//-----------------------------------------------------------------------------
// public
// Clone
//-----------------------------------------------------------------------------
HPMPersistentObject* HIMFilteredImage::Clone () const
    {
    return new HIMFilteredImage(*this);
    }

//-----------------------------------------------------------------------------
// public
// ContainsPixelsWithChannel
//-----------------------------------------------------------------------------
bool HIMFilteredImage::ContainsPixelsWithChannel(
    HRPChannelType::ChannelRole pi_Role,
    Byte                      pi_Id) const
    {
    return (GetPixelType()->GetChannelOrg().GetChannelIndex(pi_Role, pi_Id) != HRPChannelType::FREE);
    }


//-----------------------------------------------------------------------------
// public
// GetPixelSizeRange
//-----------------------------------------------------------------------------
void HIMFilteredImage::GetPixelSizeRange(HGF2DExtent& po_rMinimum, HGF2DExtent& po_rMaximum) const
    {
    // Get the average pixel size and return it for both
    po_rMinimum = GetAveragePixelSize();
    po_rMaximum = po_rMinimum;
    }

//-----------------------------------------------------------------------------
// public
// GetAveragePixelSize
//-----------------------------------------------------------------------------
HGF2DExtent HIMFilteredImage::GetAveragePixelSize() const
    {
    HGF2DExtent ResultExtent(GetCoordSys());

    // Extract from source the minimum pixel size (since the filtered uses this highest resolution)
    HGF2DExtent DumExtent(GetCoordSys());
    GetSource()->GetPixelSizeRange(ResultExtent, DumExtent);

    return(ResultExtent);
    }


//-----------------------------------------------------------------------------
// public
// GetFilter
//-----------------------------------------------------------------------------
HFCPtr<HRPFilter> HIMFilteredImage::GetFilter() const
    {
    return m_pFilter;
    }

//-----------------------------------------------------------------------------
// Notification for content changed
//-----------------------------------------------------------------------------
/*bool HIMFilteredImage::NotifyContentChanged(HMGMessage& pi_rMessage)
{
    // create a new shape
    HVEShape Shape((((HRAContentChangedMsg&)pi_rMessage).GetShape()).GetExtent());

    // create a new msg with that shape
    HRAContentChangedMsg Msg(Shape);

    // call the parent method with the new msg
    HRAImageView::NotifyContentChanged(Msg);

    // notify the raster that the content has changed
    Propagate(HRAContentChangedMsg(Msg));

    // do not propagate anymore the msg with the old shape
    return false;

    return true;
}
*/

//-----------------------------------------------------------------------------
// public
// InitObject
//-----------------------------------------------------------------------------
void HIMFilteredImage::InitObject()
    {
    }

//-----------------------------------------------------------------------------
// public
// DeepDelete
//-----------------------------------------------------------------------------
void HIMFilteredImage::DeepDelete()
    {
    m_pFilter = 0;
    }

//-----------------------------------------------------------------------------
// public
// GetPixelType
//-----------------------------------------------------------------------------

HFCPtr<HRPPixelType> HIMFilteredImage::GetPixelType() const
    {
    HFCPtr<HRPPixelType> pPixelType = m_pPixelType;

    if(pPixelType == 0)
        {
        HFCPtr<HRPPixelType> pFilterPixelType;

        // No pixeltype specified. Try to obtain filter pixeltype
        if (m_pFilter->IsCompatibleWith(HRPTypedFilter::CLASS_ID))
            {
            pFilterPixelType = ((HFCPtr<HRPTypedFilter>&)m_pFilter)->GetFilterPixelType();
            }
        else if (m_pFilter->IsCompatibleWith(HRPTypeAdaptFilter::CLASS_ID))
            {
            pFilterPixelType = ((HFCPtr<HRPTypeAdaptFilter>&)m_pFilter)->
                               GetPreferredFilterFor(GetSource()->GetPixelType())->GetFilterPixelType();
            }
        else if (m_pFilter->IsCompatibleWith(HRPComplexFilter::CLASS_ID))
            {
            const HRPComplexFilter::ListFilters FilterList = ((HFCPtr<HRPComplexFilter>&)m_pFilter)->GetList();
            HASSERT(!FilterList.empty());
            HASSERT(FilterList.back()->IsCompatibleWith(HRPTypedFilter::CLASS_ID));

            if (!FilterList.empty() && FilterList.back()->IsCompatibleWith(HRPTypedFilter::CLASS_ID))
                {
                pFilterPixelType =  ((HRPTypedFilter*)FilterList.back())->GetFilterPixelType();
                }
            }
        else
            {
            HASSERT (false);
            }

        // Use source pixeltype by default
        pPixelType = GetSource()->GetPixelType();

        if (pFilterPixelType != 0)
            {
            // the returned pixeltype will be the highest pixeltype between the source and filter
            if (pFilterPixelType->CountPixelRawDataBits() > pPixelType->CountPixelRawDataBits() ||
                pFilterPixelType->CountValueBits() > pPixelType->CountValueBits())
                {
                pPixelType = pFilterPixelType;
                }

            // Should be the highest + not lost the alpha channel if possible.
            // Special case to solve this problem temporary if the source is 48 or 64 we select 32Alpha,
            // the default one presently in the filter.
            //
            // ** The solution will be create a new HRPAlphaReplacer and Compose for 16bits +
            // ** HIMTranslucentImageCreator::CreateTranslucentRaster should instantiate the good one base
            // ** on the pixeltype 24 or 48 or ...
            else if (pFilterPixelType->GetChannelOrg().GetChannelIndex(HRPChannelType::ALPHA, 0) != HRPChannelType::FREE)
                {
                pPixelType = pFilterPixelType;
                }
            }
        }

    return pPixelType;
    }


//-----------------------------------------------------------------------------
// public
// SetFilter
//-----------------------------------------------------------------------------
void HIMFilteredImage::SetFilter(const HFCPtr<HRPFilter>& pi_rpFilter)
    {
    HPRECONDITION(pi_rpFilter != 0);

    m_pFilter = pi_rpFilter;

    m_imageOps.clear();
    s_AddHRPFilterToPipeline(m_imageOps, *m_pFilter);
    }


//-----------------------------------------------------------------------------
// public
// SetPixelType
//-----------------------------------------------------------------------------
void HIMFilteredImage::SetPixelType(const HFCPtr<HRPPixelType> pi_pPixelType)
    {
    m_pPixelType = pi_pPixelType;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ImagePPStatus HIMFilteredImage::_BuildCopyToContext(ImageTransformNodeR imageNode, HRACopyToOptionsCR options)
    {
    HRACopyToOptions newOptions(options);

    for (std::list<HRAImageOpPtr>::const_reverse_iterator opItr(m_imageOps.rbegin()); opItr != m_imageOps.rend(); ++opItr)
        imageNode.AddImageOp(*opItr, true/*atFront*/);
        
    return GetSource()->BuildCopyToContext(imageNode, newOptions);
    }


