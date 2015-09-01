//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hra/src/HRACachedHistogramRaster.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>
    //:> must be first for PreCompiledHeader Option

#include <Imagepp/all/h/HRACachedHistogramRaster.h>
#include <Imagepp/all/h/HRAHistogramOptions.h>
#include <Imagepp/all/h/HRAMessages.h>
#include <Imagepp/all/h/HRPMessages.h>



HPM_REGISTER_CLASS(HRACachedHistogramRaster, HRAImageView)


HMG_BEGIN_DUPLEX_MESSAGE_MAP(HRACachedHistogramRaster, HRARaster, HMG_NO_NEED_COHERENCE_SECURITY)
HMG_REGISTER_MESSAGE(HRACachedHistogramRaster, HGFGeometryChangedMsg, NotifySomethingChanged)
HMG_REGISTER_MESSAGE(HRACachedHistogramRaster, HRAContentChangedMsg, NotifySomethingChanged)
HMG_REGISTER_MESSAGE(HRACachedHistogramRaster, HRPPaletteChangedMsg, NotifySomethingChanged)
HMG_END_MESSAGE_MAP()


/** -----------------------------------------------------------------------------
    Default constructor. For persistence purposes only, don't use directly.
    -----------------------------------------------------------------------------
*/
HRACachedHistogramRaster::HRACachedHistogramRaster()
    :   HRAImageView(HFCPtr<HRARaster>())
    {
    }


/** -----------------------------------------------------------------------------
    Constructor
    -----------------------------------------------------------------------------
*/
HRACachedHistogramRaster::HRACachedHistogramRaster(const HFCPtr<HRARaster>& pi_pSource)
    :   HRAImageView(pi_pSource)
    {
    }


/** -----------------------------------------------------------------------------
    Copy constructor
    -----------------------------------------------------------------------------
*/
HRACachedHistogramRaster::HRACachedHistogramRaster(const HRACachedHistogramRaster& pi_rObj)
    :   HRAImageView(pi_rObj)
    {
    }


/** -----------------------------------------------------------------------------
    Assignment operator
    -----------------------------------------------------------------------------
*/
HRACachedHistogramRaster& HRACachedHistogramRaster::operator=(const HRACachedHistogramRaster& pi_rObj)
    {
    if (this != &pi_rObj)
        {
        HRAImageView::operator=(pi_rObj);

        // Don't copy the histograms
        ClearHistogramCache();
        }

    return *this;
    }


/** -----------------------------------------------------------------------------
    Destructor
    -----------------------------------------------------------------------------
*/
HRACachedHistogramRaster::~HRACachedHistogramRaster()
    {
    ClearHistogramCache();
    }


/** -----------------------------------------------------------------------------
    public
    Clone
    -----------------------------------------------------------------------------
*/
HRARaster* HRACachedHistogramRaster::Clone (HPMObjectStore* pi_pStore,
                                            HPMPool*        pi_pLog) const
    {
    return new HRACachedHistogramRaster(*this);
    }
/** -----------------------------------------------------------------------------
    public
    Clone
    -----------------------------------------------------------------------------
*/
HPMPersistentObject* HRACachedHistogramRaster::Clone () const
    {
    return new HRACachedHistogramRaster(*this);
    }

//-----------------------------------------------------------------------------
// public
// Draw
//-----------------------------------------------------------------------------
void HRACachedHistogramRaster::_Draw(HGFMappedSurface& pio_destSurface, HRADrawOptions const& pi_Options) const
    {
    GetSource()->Draw(pio_destSurface, pi_Options);
    }

/** -----------------------------------------------------------------------------
    Receive a change notification. We handle geometry and content changes
    the same way: we simply flush the histogram cache and propagate the
    message.
    -----------------------------------------------------------------------------
*/
bool HRACachedHistogramRaster::NotifySomethingChanged(const HMGMessage& pi_rMessage)
    {
    // Flush the histograms
    ClearHistogramCache();

    return true;
    }


/** -----------------------------------------------------------------------------
    Compute a histogram. Reuse what is in the cache if possible. Otherwise,
    compute a new one and keep it in the cache.
    -----------------------------------------------------------------------------
*/
void HRACachedHistogramRaster::ComputeHistogram(HRAHistogramOptions* pio_pOptions,
                                                bool                pi_ForceRecompute)
    {
    HPRECONDITION(pio_pOptions != 0);

    if (pio_pOptions->GetSamplingOptions().GetRegionToScan() != 0)
        {
        // Check if we have it cached
        HistogramList::const_iterator Itr(m_Histograms.begin());
        while (Itr != m_Histograms.end())
            {
            if ((*Itr)->CanBeUsedInPlaceOf(*pio_pOptions))
                break;

            ++Itr;
            }

        if (!pi_ForceRecompute && (Itr != m_Histograms.end()))
            {
            // Reuse a cached histogram

            HPRECONDITION((*Itr)->GetHistogram()->GetEntryFrequenciesSize() <=
                          pio_pOptions->GetHistogram()->GetEntryFrequenciesSize());

            HFCPtr<HRPHistogram> pHisto((*Itr)->GetHistogram());
            HFCPtr<HRPHistogram> pOutHisto(pio_pOptions->GetHistogram());

            HASSERT(pOutHisto->GetChannelCount() == pHisto->GetChannelCount());
            for (uint32_t channelIndex = 0; channelIndex < pHisto->GetChannelCount(); channelIndex++)
                {
                for (uint32_t i = 0; i < pHisto->GetEntryFrequenciesSize(); i++)
                    pOutHisto->IncrementEntryCount(i, pHisto->GetEntryCount(i, channelIndex), channelIndex);

                }
            }
        else
            {
            // Have the ancestor compute the histogram
            GetSource()->ComputeHistogram(pio_pOptions, pi_ForceRecompute);

            // Cache it
            m_Histograms.push_back(new HRAHistogramOptions(*pio_pOptions));
            }

        }
    else
        {
        // Normal behavior

        GetSource()->ComputeHistogram(pio_pOptions, pi_ForceRecompute);
        }
    }


/** -----------------------------------------------------------------------------
    Flush the contents of the histograms cache
    -----------------------------------------------------------------------------
*/
void HRACachedHistogramRaster::ClearHistogramCache()
    {
    // Delete the stored pointers
    HistogramList::const_iterator Itr(m_Histograms.begin());
    while (Itr != m_Histograms.end())
        {
        delete *Itr;

        ++Itr;
        }

    m_Histograms.clear();
    }

