//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hra/src/HRAPixelTypeReplacer.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for class HRAPixelTypeReplacer
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HRAPixelTypeReplacer.h>

#include <Imagepp/all/h/HRPPixelTypeV24R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeV32R8G8B8A8.h>
#include <Imagepp/all/h/HRAHistogramOptions.h>
#include <Imagepp/all/h/HRARepPalParms.h>
#include <Imagepp/all/h/HRADrawOptions.h>
#include <Imagepp/all/h/HRAMessages.h>
#include <Imagepp/all/h/HRPMessages.h>
#include <ImagePPInternal/gra/HRACopyToOptions.h>



HPM_REGISTER_CLASS(HRAPixelTypeReplacer, HRAImageView)


HMG_BEGIN_DUPLEX_MESSAGE_MAP(HRAPixelTypeReplacer, HRAImageView, HMG_NO_NEED_COHERENCE_SECURITY)
HMG_REGISTER_MESSAGE(HRAPixelTypeReplacer, HRAContentChangedMsg, NotifyContentChanged)
HMG_REGISTER_MESSAGE(HRAPixelTypeReplacer, HRPPaletteChangedMsg, NotifyPaletteChanged)
HMG_END_MESSAGE_MAP()

//-----------------------------------------------------------------------------
// public
// default constructor
//-----------------------------------------------------------------------------
HRAPixelTypeReplacer::HRAPixelTypeReplacer()
    :   HRAImageView()
    {
    }

//-----------------------------------------------------------------------------
// public
// constructor
//-----------------------------------------------------------------------------
HRAPixelTypeReplacer::HRAPixelTypeReplacer( const HFCPtr<HRARaster>& pi_pSource,
                                            const HFCPtr<HRPPixelType>& pi_pPixelType)
    :   HRAImageView((const HFCPtr<HRARaster>&)pi_pSource)
    {
    HPRECONDITION(pi_pPixelType != 0);

    // the source and the replacing pixeltype must have the same number of bits
    // to represent raw data
    HPRECONDITION(pi_pPixelType->CountPixelRawDataBits() ==
                  pi_pSource->GetPixelType()->CountPixelRawDataBits());

    m_pPixelType = pi_pPixelType;

    // link on the pixel type
    LinkTo(m_pPixelType);
    }

//-----------------------------------------------------------------------------
// public
// copy constructor
//-----------------------------------------------------------------------------
HRAPixelTypeReplacer::HRAPixelTypeReplacer(const HRAPixelTypeReplacer& pi_rObject)
    :   HRAImageView(pi_rObject)
    {
    m_pPixelType = pi_rObject.m_pPixelType;

    // link on the pixel type
    LinkTo(m_pPixelType);
    }

//-----------------------------------------------------------------------------
// public
// destructor
//-----------------------------------------------------------------------------
HRAPixelTypeReplacer::~HRAPixelTypeReplacer()
    {
    // unlink from the pixel type

    if(m_pPixelType != 0)
        UnlinkFrom(m_pPixelType);
    }

//-----------------------------------------------------------------------------
// public
// Clone
//-----------------------------------------------------------------------------
HFCPtr<HRARaster> HRAPixelTypeReplacer::Clone (HPMObjectStore* pi_pStore, HPMPool* pi_pLog) const
    {
    return new HRAPixelTypeReplacer(*this);
    }

//-----------------------------------------------------------------------------
// public
// Clone
//-----------------------------------------------------------------------------
HPMPersistentObject* HRAPixelTypeReplacer::Clone () const
    {
    return new HRAPixelTypeReplacer(*this);
    }

//-----------------------------------------------------------------------------
// public
// GetPixelType
//-----------------------------------------------------------------------------
HFCPtr<HRPPixelType> HRAPixelTypeReplacer::GetPixelType() const
    {
    return m_pPixelType;
    }

//-----------------------------------------------------------------------------
// public
// GetRepresentativePalette
//-----------------------------------------------------------------------------
unsigned short HRAPixelTypeReplacer::GetRepresentativePalette(HRARepPalParms* pio_pRepPalParms)
    {
    HPRECONDITION(pio_pRepPalParms != 0);

    unsigned short CountUsed;

    if(pio_pRepPalParms->UseCache())
        // call the parent method
        CountUsed = HRARaster::GetRepresentativePaletteCache(pio_pRepPalParms);
    else
        CountUsed = 0;

    // if no operation has been done at the parent level or if the cache is not
    // updated
    if(CountUsed == 0)
        {
        // copy the parameter structure because we will change some of
        // the attributes in it and we do not want to overwrite the
        // original structure
        HRARepPalParms RepPalParms(*pio_pRepPalParms);

        // set a replacing pixel type
        HRASamplingOptions TmpSamplingOptions(RepPalParms.GetSamplingOptions());

        // Only take the outermost replacer into account.
        if (TmpSamplingOptions.GetSrcPixelTypeReplacer() == 0)
            TmpSamplingOptions.SetSrcPixelTypeReplacer(m_pPixelType);

        RepPalParms.SetSamplingOptions(TmpSamplingOptions);

        // disable the use of the cache, we want the replaced data,
        // not the original data in the various caches
        RepPalParms.SetCacheUse(false);

        // call the parent method
        CountUsed = HRAImageView::GetRepresentativePalette(&RepPalParms);

        if(pio_pRepPalParms->UseCache())
            {
            // update the palette cache
            UpdateRepPalCache(CountUsed, RepPalParms.GetPixelType()->GetPalette());
            }

        // since the pixel type in the new and old HRPRepPalParms objects
        // are the same, we do not need to copy the palette
        }

    return CountUsed;
    }


//-----------------------------------------------------------------------------
// public
// ComputeHistogram
//-----------------------------------------------------------------------------
void HRAPixelTypeReplacer::ComputeHistogram(HRAHistogramOptions* pio_pOptions,
                                            bool                pi_ForceRecompute)
    {
    HPRECONDITION(pio_pOptions != 0);

    if (pi_ForceRecompute || (GetHistogram() == 0) || (GetHistogram() != 0 && !GetHistogram()->CanBeUsedInPlaceOf(*pio_pOptions)))
        {
        // copy the parameter structure because we will change some of
        // the attributes in it and we do not want to overwrite the
        // original structure
        HRAHistogramOptions TmpOptions(*pio_pOptions);

        // set a replacing pixel type
        HRASamplingOptions TmpSamplingOptions(TmpOptions.GetSamplingOptions());
        TmpSamplingOptions.SetSrcPixelTypeReplacer(m_pPixelType);
        TmpOptions.SetSamplingOptions(TmpSamplingOptions);

        // call the parent method
        HRAImageView::ComputeHistogram(&TmpOptions, pi_ForceRecompute);

        // copy histogram into the original
        HFCPtr<HRPHistogram> pTmpHistogram(TmpOptions.GetHistogram());
        HFCPtr<HRPHistogram> pHistogram   (pio_pOptions->GetHistogram());

        HASSERT(pTmpHistogram->GetChannelCount() == pHistogram->GetChannelCount());
        for (uint32_t ChannelIndex = 0; ChannelIndex < pTmpHistogram->GetChannelCount(); ChannelIndex++)
            {
            for (uint32_t i = 0; i < pTmpHistogram->GetEntryFrequenciesSize(ChannelIndex); i++)
                pHistogram->SetEntryCount(i, pTmpHistogram->GetEntryCount(i, ChannelIndex), ChannelIndex);
            }

        // set histogram into HRARaster
        SetHistogram(pio_pOptions);

        // since the pixel type in the new and old HRPRepPalParms objects
        // are the same, we do not need to copy the palette
        }
    else
        {
        HPRECONDITION(GetHistogram()->GetHistogram()->GetEntryFrequenciesSize() <=
                      pio_pOptions->GetHistogram()->GetEntryFrequenciesSize());

        HFCPtr<HRPHistogram> pHisto(new HRPHistogram(*GetHistogram()->GetHistogram()));
        HFCPtr<HRPHistogram> pOutHisto(pio_pOptions->GetHistogram());

        HASSERT(pHisto->GetChannelCount() == pOutHisto->GetChannelCount());
        for (uint32_t ChannelIndex = 0; ChannelIndex < pOutHisto->GetChannelCount(); ChannelIndex++)
            {
            for (uint32_t i = 0; i < pHisto->GetEntryFrequenciesSize(ChannelIndex); i++)
                pOutHisto->IncrementEntryCount(i, pHisto->GetEntryCount(i, ChannelIndex), ChannelIndex);
            }
        }
    }

//-----------------------------------------------------------------------------
// Notification for content changed
//-----------------------------------------------------------------------------
bool HRAPixelTypeReplacer::NotifyContentChanged(const HMGMessage& pi_rMessage)
    {
    // invalidate the representative palette cache
    InvalidateRepPalCache();


    // propagate the shape
    return true;
    }


//-----------------------------------------------------------------------------
// public
// Notify palette changed
//-----------------------------------------------------------------------------
bool HRAPixelTypeReplacer::NotifyPaletteChanged(const HMGMessage& pi_rMessage)
    {
    // invalidate the representative palette cache
    InvalidateRepPalCache();

    // propagate the message
    return true;
    }


//-----------------------------------------------------------------------------
// public
// ContainsPixelsWithChannel
//-----------------------------------------------------------------------------
bool HRAPixelTypeReplacer::ContainsPixelsWithChannel(
    HRPChannelType::ChannelRole pi_Role,
    Byte                      pi_Id) const
    {
    return (m_pPixelType->GetChannelOrg().GetChannelIndex(pi_Role, pi_Id) != HRPChannelType::FREE);
    }

//-----------------------------------------------------------------------------
// public
// Draw
//-----------------------------------------------------------------------------
void HRAPixelTypeReplacer::_Draw(HGFMappedSurface& pio_destSurface, HRADrawOptions const& pi_Options) const
    {
    HRADrawOptions Options(pi_Options);

    // set the replacing pixel type in the options if there is no already defined
    if(Options.GetReplacingPixelType() == 0)
        Options.SetReplacingPixelType(m_pPixelType);

    GetSource()->Draw(pio_destSurface, Options);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ImagePPStatus HRAPixelTypeReplacer::_CopyFrom(HRARaster& srcRaster, HRACopyFromOptions const& pi_rOptions)
    {
    HRACopyFromOptions Options(pi_rOptions);

    // set the replacing pixel type in the options if there is no already defined
    if(pi_rOptions.GetDestReplacingPixelType() == 0)
        Options.SetDestReplacingPixelType(m_pPixelType);

    return T_Super::_CopyFrom(srcRaster, Options);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 07/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ImagePPStatus HRAPixelTypeReplacer::_BuildCopyToContext(ImageTransformNodeR imageNode, HRACopyToOptionsCR options)
    {
    HRACopyToOptions newOptions(options);
    newOptions.SetReplacingPixelType(GetPixelType());

    return GetSource()->BuildCopyToContext(imageNode, newOptions);
    }

//-----------------------------------------------------------------------------
// public
// CopyFromLegacy
//-----------------------------------------------------------------------------
void HRAPixelTypeReplacer::CopyFromLegacy(const HFCPtr<HRARaster>& pi_pSrcRaster, const HRACopyFromLegacyOptions& pi_rOptions)
    {
    HRACopyFromLegacyOptions Options(pi_rOptions);

    // set the replacing pixel type in the options if there is no already defined
    if(pi_rOptions.GetDestReplacingPixelType() == 0)
        Options.SetDestReplacingPixelType(m_pPixelType);

    T_Super::CopyFromLegacy(pi_pSrcRaster, Options);
    }

//-----------------------------------------------------------------------------
// public
// CopyFromLegacy
//-----------------------------------------------------------------------------
void HRAPixelTypeReplacer::CopyFromLegacy(const HFCPtr<HRARaster>& pi_pSrcRaster)
    {
    HRACopyFromLegacyOptions Options;
    Options.SetDestReplacingPixelType(m_pPixelType);

    T_Super::CopyFromLegacy(pi_pSrcRaster, Options);
    }
