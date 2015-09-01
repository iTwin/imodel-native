//:>---s-----------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hra/src/HRAImageSampler.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <ImagePPInternal/gra/HRAImageSampler.h>
#include <Imagepp/all/h/HRPPixelType.h>
#include <ImagePP/all/h/HGF2DTransfoModel.h>
#include <ImagePP/all/h/HGF2DDisplacement.h>
#include <ImagePPInternal/gra/ImageCommon.h>
#include <ImagePPInternal/gra/HRAImageSurface.h>



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HRAImageSampler::HRAImageSampler(HRPPixelNeighbourhood const& neighbourhood, HFCPtr<HGF2DTransfoModel>& destToSrcTransfo)
    :m_pDestToSrcTransfo(destToSrcTransfo),
     m_neighbourhood(neighbourhood),
     m_enableMultiThreading(ImageppLib::GetHost().GetImageppLibAdmin()._IsSamplerMultiThreadingEnabled())
    {
    BeAssert(destToSrcTransfo != NULL);

    if(destToSrcTransfo->IsStretchable(HGLOBAL_EPSILON))
        {
        HGF2DDisplacement Displacement;
    	destToSrcTransfo->GetStretchParams(&m_scaleX, &m_scaleY, &Displacement);
        m_offsetX = Displacement.GetDeltaX();
        m_offsetY = Displacement.GetDeltaY();
        m_isStretchable = true;
        // We expect that sources will produce the closest 1:N and that we are closing the gap to the
        // remainder of the stretch op. So at most a factor of 2.
        // If not it will be problematic for sampler with neighbourhood. Since they should use a bigger window
        // to compute the "averaged" pixel.
//         BeAssertOnce(m_scaleX < 2.00001 && m_scaleY <= 2.00001); Can happen with extreme zoom out!

        m_hasTranslationOnly = HDOUBLE_EQUAL_EPSILON(m_scaleX, 1.0) && HDOUBLE_EQUAL_EPSILON(m_scaleY, 1.0);
        }
    else
        {
        m_offsetX = m_offsetY = DBL_MAX;
        m_scaleX = m_scaleY = DBL_MAX;
        m_isStretchable = m_hasTranslationOnly = false;
        }
    m_preservesLinearity = destToSrcTransfo->PreservesLinearity();
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
HRAImageSampler::~HRAImageSampler(){};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool HRAImageSampler::IsStretchable() const {return m_isStretchable;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Alexandre.Gariepy 06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool HRAImageSampler::PreservesLinearity() const {return m_preservesLinearity;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
HGF2DTransfoModel const& HRAImageSampler::GetTransfoModel() const
    {
    return *m_pDestToSrcTransfo;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
HRPPixelNeighbourhood const& HRAImageSampler::GetNeighbourdhood() const
    {
    return m_neighbourhood;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HRAImageSamplePtr HRAImageSampler::ComputeSample(ImagePPStatus& status, uint32_t width, uint32_t height, PixelOffset const& outOffset, 
                                                 HRAImageSurfaceR inData, PixelOffset const& inOffset,
                                                 IImageAllocatorR allocator)
    {
    status = IMAGEPP_STATUS_UnknownError;

    //  We need to generate more than one pixel to consider a non 1:1 stretch.
    // If:
    //  width >= 0
    //  width*deltaX <= width + 1   ---> 1 is the pixel tolerance.       
    //  width*deltaX >= width - 1   ---> 1 is the pixel tolerance.      
    //  --------
    // a) (w-1)/w <= dx <= (w+1)/w
    // b) 0 <= dx - (w-1)/w <= (w+1)/w - (w-1)/w
    // c) 0 <= dx -1 + 1/w <= 2/w
    // d) 0 <= dx*w - w + 1 <= 2
    // Then we have a factor of 1:1 if |width*(deltaX-1)+1| <= 2  
    // With a tolerance of 0.5 we have : |w*(dx-1)-0.5| < 1
    //bool isNoStrechX = fabs(outWidth*(deltaX-1) + 1) <= 2;
    // *** UPDATE ***
    // - Doesn't work with W = 2, delta = 0.5. It gives 1:1 when it should not.
    // - We also need take the m_offset, inOffset and outOffset into account. That makes a difference has to when we skip pixel.
    // - Another tentative was to calculate the first and last pixel position and if it matches the amount to copy
    //   that would mean 1:1. Unfortunately the scale value has some precision error. 
    //   A scale of 1.0 might has a slight error that make us skip a pixel when the src and dest have a translation close to 0.5.
    // Finally, we decided to test for 1.0 with an epsilon. This is what the old code was doing.
    if (m_hasTranslationOnly)
        {
        PixelOffset offset((m_offsetX + (outOffset.x + 0.5)) - inOffset.x, (m_offsetY + (outOffset.y + 0.5)) - inOffset.y);
        return Translate(status, width, height, inData, offset, allocator);
        }
    
    HRAImageSamplePtr pOutSample = HRAImageSample::CreateSample(status, width, height, inData.GetPixelTypePtr(), allocator);
    if (IMAGEPP_STATUS_Success != status)
        return NULL;

    if (m_isStretchable)
        status = _Stretch(*pOutSample, outOffset, inData, inOffset);
    else
        status = _Warp(*pOutSample, outOffset, inData, inOffset);

    BeAssert(IMAGEPP_STATUS_Success == status ? pOutSample->ValidateIntegrity() : true);

    return pOutSample;
    } 

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
HRAImageSamplePtr HRAImageSampler::Translate(ImagePPStatus& status, uint32_t width, uint32_t height, HRAImageSurfaceR inData, PixelOffset const& offset, IImageAllocatorR allocator)
    {
    HFCInclusiveGrid srcGrid;
    srcGrid.InitFromLenght(offset.x, offset.y, width, height);

    if (srcGrid.GetXMin() >= 0 && srcGrid.GetYMin() >= 0)
        {
        PixelOffset64 offset64(srcGrid.GetXMin(), srcGrid.GetYMin());
        HRAImageSamplePtr pOutSample = inData.CreateBufferReference(status, width, height, offset64);
        if(IMAGEPP_STATUS_Success == status)
            return pOutSample;
        }

    // For some reasons, we cannot reference input data. It might be because:
    //  - Is it not supported (N1, RLE)
    //  - or we need to generate data that we do not have.
    //  - or ???
    // Copy requested data to a new sample.

    // Use floor in case we have negative offset.  We what to take every pixels we touch.
    PixelOffset64 offset64((int64_t)floor(offset.x), (int64_t)floor(offset.y));
    return inData.CopyToSample(status, width, height, offset64, allocator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  09/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ImagePPStatus HRAImageSampler::test_Stretch(HRAImageSampleR outData, PixelOffset const& outOffset, HRAImageSurfaceR inData, PixelOffset const& inOffset)
    {
    if (!m_isStretchable)
        return IMAGEPP_STATUS_UnknownError;

    return _Stretch(outData, outOffset, inData, inOffset);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  09/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ImagePPStatus HRAImageSampler::test_Warp(HRAImageSampleR outData, PixelOffset const& outOffset, HRAImageSurfaceR inData, PixelOffset const& inOffset)
    {
    return _Warp(outData, outOffset, inData, inOffset);
    }
