//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hra/src/HRAHistogramOptions.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HRAHistogramOptions.h>

/** -----------------------------------------------------------------------------
    Copy constructor
    -----------------------------------------------------------------------------
*/
HRAHistogramOptions::HRAHistogramOptions(const HRAHistogramOptions& pi_rOptions)
    {
    DeepCopy(pi_rOptions);
    }


/** -----------------------------------------------------------------------------
    Constructor
    -----------------------------------------------------------------------------
*/
HRAHistogramOptions::HRAHistogramOptions(const HRPPixelType* pi_pPixelType, HRPHistogram::COLOR_SPACE pi_ColorSpace)
    {
    HPRECONDITION(pi_pPixelType != 0);

    m_ColorSpace = pi_ColorSpace;
    m_pPixelType = (HRPPixelType*)pi_pPixelType->Clone();

    if (pi_pPixelType->CountIndexBits())
        {
        const HRPPixelPalette* pPalette = &(pi_pPixelType->GetPalette());

        HASSERT(pPalette != 0);
        HASSERT(pPalette->GetMaxEntries());

        m_pHistogram = new HRPHistogram   (*pPalette);
        }
    else
        {
        const HRPChannelOrg& ChannelOrg = pi_pPixelType->GetChannelOrg();

        uint32_t ChannelsCount     = ChannelOrg.CountChannels();
        HASSERT(ChannelsCount > 0);

        uint32_t ChannelsValueBits = ChannelOrg.CountPixelCompositeValueBits();
        uint32_t ChannelWidth = 0;

        if (ChannelsCount == 1 || m_ColorSpace == HRPHistogram::LIGHTNESS)
            {
            ChannelWidth = ChannelsValueBits;
            }
        else
            {
            if (ChannelsValueBits == 24 ||ChannelsValueBits == 32)
                ChannelWidth = 8;
            else if (ChannelsValueBits == 16 ||
                     ChannelsValueBits == 48 ||
                     ChannelsValueBits == 64)
                ChannelWidth = 16;
            //An 32 bits histogram is just impractical for now.
            //So we use a 16 bits instead.
            else if (ChannelsValueBits == 96)
                ChannelWidth = 16;
            }

        HASSERT(ChannelWidth > 0);
        m_pHistogram = new HRPHistogram((1 << ChannelWidth),  ChannelsCount);
        }
    }

/** -----------------------------------------------------------------------------
    Constructor
    -----------------------------------------------------------------------------
*/
HRAHistogramOptions::HRAHistogramOptions(const HFCPtr<HRPHistogram>&    pio_rpHistogram,
                                         const HRPPixelType*            pi_pPixelType)
    {
    HPRECONDITION(pio_rpHistogram != 0);

    m_pHistogram = pio_rpHistogram;
    m_ColorSpace = pio_rpHistogram->GetSamplingColorSpace();
    m_pPixelType = pi_pPixelType ? (HRPPixelType*)pi_pPixelType->Clone() : 0;
    }


/** -----------------------------------------------------------------------------
    Constructor
    -----------------------------------------------------------------------------
*/
HRAHistogramOptions::HRAHistogramOptions(const HFCPtr<HRPHistogram>&    pio_rpHistogram,
                                         const HRPPixelType*            pi_pPixelType,
                                         const HRASamplingOptions&      pi_rOptions)
    :  m_SamplingOptions(pi_rOptions)
    {
    HPRECONDITION(pio_rpHistogram != 0);

    m_pHistogram = pio_rpHistogram;
    m_ColorSpace = pio_rpHistogram->GetSamplingColorSpace();
    m_pPixelType = pi_pPixelType ? (HRPPixelType*)pi_pPixelType->Clone() : 0;
    }


/** -----------------------------------------------------------------------------
    Destructor
    -----------------------------------------------------------------------------
*/
HRAHistogramOptions::~HRAHistogramOptions()
    {
    }

/** ---------------------------------------------------------------------------
    Tell if the current histogram can be used to serve the request specified
    in the parameter. There are a few conditions for this to be true:
        1 - They must both represent the same region (RegionToScan).
        2 - We must sample at least the same amount of data.
        3 - The histogram must be in the same PixelType.
        4 - No option must use a filter. This is because of the restriction
            on HRASamplingOptions that says that we can't keep an options
            object for a long time. We don't know the age of the object,
            so we don't take any chances :-)
    ---------------------------------------------------------------------------
*/
bool HRAHistogramOptions::CanBeUsedInPlaceOf(const HRAHistogramOptions& pi_rHistogramOptions) const
    {
    bool CanBeUsed;

    // Multiple returns used for simplification.

    if (m_SamplingOptions.GetRegionToScan() != 0 && pi_rHistogramOptions.m_SamplingOptions.GetRegionToScan() != 0)
        CanBeUsed = m_SamplingOptions.GetRegionToScan()->Matches(*pi_rHistogramOptions.m_SamplingOptions.GetRegionToScan());
    else
        CanBeUsed = m_SamplingOptions.GetRegionToScan() == 0 && pi_rHistogramOptions.m_SamplingOptions.GetRegionToScan() == 0;

    if (!CanBeUsed)
        return false;

    if (m_SamplingOptions.GetSrcPixelTypeReplacer() != 0 && pi_rHistogramOptions.m_SamplingOptions.GetSrcPixelTypeReplacer() != 0)
        CanBeUsed = m_SamplingOptions.GetSrcPixelTypeReplacer()->HasSamePixelInterpretation(*pi_rHistogramOptions.m_SamplingOptions.GetSrcPixelTypeReplacer());
    else
        CanBeUsed = m_SamplingOptions.GetSrcPixelTypeReplacer() == 0 && pi_rHistogramOptions.m_SamplingOptions.GetSrcPixelTypeReplacer() == 0;

    if (!CanBeUsed)
        return false;

    if (m_pPixelType != 0 && pi_rHistogramOptions.m_pPixelType != 0)
        CanBeUsed = m_pPixelType->HasSamePixelInterpretation(*pi_rHistogramOptions.m_pPixelType);
    else
        CanBeUsed = m_pPixelType == 0 && pi_rHistogramOptions.m_pPixelType == 0;

    if (!CanBeUsed)
        return false;

    if (m_SamplingOptions.GetPixelsToScan() < pi_rHistogramOptions.m_SamplingOptions.GetPixelsToScan())
        return false;

    if (m_SamplingOptions.GetTilesToScan() < pi_rHistogramOptions.m_SamplingOptions.GetTilesToScan())
        return false;

    if (m_SamplingOptions.GetPyramidImageSize() < pi_rHistogramOptions.m_SamplingOptions.GetPyramidImageSize())
        return false;

    return true;
    }

/** -----------------------------------------------------------------------------
    DeepCopy
    -----------------------------------------------------------------------------
*/
void HRAHistogramOptions::DeepCopy(const HRAHistogramOptions& pi_rObj)
    {
    m_pPixelType      = pi_rObj.m_pPixelType ? (HRPPixelType*)pi_rObj.m_pPixelType->Clone() : 0;

    m_SamplingOptions = pi_rObj.m_SamplingOptions;

    if (pi_rObj.m_pHistogram != 0)
        m_pHistogram = new HRPHistogram(*pi_rObj.m_pHistogram);
    else
        m_pHistogram = 0;

    m_ColorSpace          = pi_rObj.m_ColorSpace;
    }
