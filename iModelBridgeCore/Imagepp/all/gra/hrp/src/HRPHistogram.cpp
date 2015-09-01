//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrp/src/HRPHistogram.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HRPHistogram
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HRPHistogram.h>

HPM_REGISTER_CLASS(HRPHistogram, HPMPersistentObject)

//-----------------------------------------------------------------------------
// Constructor.  (used ONLY for persistence)
//-----------------------------------------------------------------------------

HRPHistogram::HRPHistogram()
    {
    m_ColorSpace = NATIVE;
    ConstructUsingRGBColorSpace(0, 256, 1);
    }

//-----------------------------------------------------------------------------
// Normal constructor
//-----------------------------------------------------------------------------

HRPHistogram::HRPHistogram(const HRPPixelPalette& pi_rPalette, HRPHistogram::COLOR_SPACE pi_ColorSpace)
    {

    // Assign the new color space.
    m_ColorSpace = pi_ColorSpace;

    if (m_ColorSpace == LIGHTNESS)
        ConstructUsingLightnessColorSpace(256);
    else
        {
        if (m_ColorSpace != NATIVE)
            m_ColorSpace = GRAYSCALE;

        uint32_t MaxChannelWidth = 0;
        for(uint32_t ChanelIndex= 0; ChanelIndex < pi_rPalette.GetChannelOrg().CountChannels(); ChanelIndex++)
            MaxChannelWidth = MAX(MaxChannelWidth, pi_rPalette.GetChannelOrg().GetChannelPtr(ChanelIndex)->GetSize ());


        ConstructUsingRGBColorSpace(0, 1 << MaxChannelWidth, 1);
        }
    }

//-----------------------------------------------------------------------------
// Normal constructor
//-----------------------------------------------------------------------------

HRPHistogram::HRPHistogram(uint32_t**  pi_pEntryFrequencies,
                           uint32_t  pi_EntryFrequenciesSize,
                           uint32_t  pi_ChannelCount,
                           HRPHistogram::COLOR_SPACE pi_ColorSpace)
    {
    // Assign the new color space.
    m_ColorSpace = pi_ColorSpace;

    // Rebuild the Histogram according the new given ColorSpace.
    if (m_ColorSpace == LIGHTNESS)
        ConstructUsingLightnessColorSpace(pi_EntryFrequenciesSize);
    else
        {
        if (m_ColorSpace != NATIVE)
            m_ColorSpace = AutoSelectColorSpace(pi_ChannelCount);

        ConstructUsingRGBColorSpace(pi_pEntryFrequencies,
                                    pi_EntryFrequenciesSize,
                                    pi_ChannelCount);
        }
    }

//-----------------------------------------------------------------------------
// Normal constructor
//-----------------------------------------------------------------------------

HRPHistogram::HRPHistogram(uint32_t*  pi_pEntryFrequencies,
                           uint32_t pi_EntryFrequenciesSize,
                           HRPHistogram::COLOR_SPACE pi_ColorSpace)
    {
    // Assign the new color space.
    m_ColorSpace = pi_ColorSpace;

    // Rebuild the Histogram according the new given ColorSpace.
    if (m_ColorSpace == LIGHTNESS)
        ConstructUsingLightnessColorSpace(pi_EntryFrequenciesSize);
    else
        {
        if (m_ColorSpace != NATIVE)
            m_ColorSpace = GRAYSCALE;

        ConstructUsingRGBColorSpace(&pi_pEntryFrequencies, pi_EntryFrequenciesSize, 1);
        }
    }

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------

HRPHistogram::HRPHistogram(uint32_t pi_EntryFrequenciesSize,
                           uint32_t pi_ChannelCount,
                           HRPHistogram::COLOR_SPACE pi_ColorSpace)
    {
    // Assign the new color space.
    m_ColorSpace = pi_ColorSpace;

    // Rebuild the Histogram according the new given ColorSpace.
    if (m_ColorSpace == LIGHTNESS)
        ConstructUsingLightnessColorSpace(pi_EntryFrequenciesSize);
    else
        {
        if (m_ColorSpace != NATIVE)
            m_ColorSpace = AutoSelectColorSpace(pi_ChannelCount);

        ConstructUsingRGBColorSpace(0, pi_EntryFrequenciesSize, pi_ChannelCount);
        }
    }

//-----------------------------------------------------------------------------
// Copy constructor.
//-----------------------------------------------------------------------------

HRPHistogram::HRPHistogram(const HRPHistogram& pi_rObj)
    {
    DeepCopy(pi_rObj);
    }

//-----------------------------------------------------------------------------
// The destructor.
//-----------------------------------------------------------------------------

HRPHistogram::~HRPHistogram()
    {
    DeepDelete();
    }

//-----------------------------------------------------------------------------
// Copies everything owned by the object
//-----------------------------------------------------------------------------

void HRPHistogram::DeepCopy(const HRPHistogram& pi_rObj)
    {
    m_ChannelCount          = pi_rObj.GetChannelCount();
    m_EntryFrequenciesSize  = pi_rObj.m_EntryFrequenciesSize;
    m_pEntryFrequencies     = new HArrayAutoPtr<uint32_t>[m_ChannelCount];
    m_ColorSpace            = pi_rObj.m_ColorSpace;

    for (uint32_t ChannelIndex = 0; ChannelIndex < m_ChannelCount; ChannelIndex++)
        {
        m_pEntryFrequencies[ChannelIndex] = new uint32_t[pi_rObj.m_EntryFrequenciesSize];
        memcpy(m_pEntryFrequencies[ChannelIndex], pi_rObj.m_pEntryFrequencies[ChannelIndex], m_EntryFrequenciesSize * sizeof(uint32_t));
        }
    }

//-----------------------------------------------------------------------------
// Deletes everything owned by the object
//-----------------------------------------------------------------------------

void HRPHistogram::DeepDelete()
    {
    // Free allocated memory if any
    if (m_pEntryFrequencies != 0)
        {
        for (uint32_t ChannelIndex = 0; ChannelIndex < m_ChannelCount; ChannelIndex++)
            {
            m_pEntryFrequencies[ChannelIndex] = 0;
            }
        m_pEntryFrequencies = 0;
        }

    // Reset members.
    m_ChannelCount         = 0;
    m_EntryFrequenciesSize = 0;
    }

//-----------------------------------------------------------------------------
// Assignment operator.  It duplicates another pixel type, doing a deep copy.
//-----------------------------------------------------------------------------

HRPHistogram& HRPHistogram::operator=(const HRPHistogram& pi_rObj)
    {
    if (this != &pi_rObj)
        {
        DeepDelete();
        DeepCopy(pi_rObj);
        }
    return *this;
    }

//-----------------------------------------------------------------------------
// public
// FindEntryWithMinimumCount.
//-----------------------------------------------------------------------------

uint32_t HRPHistogram::FindEntryWithMinimumCount(uint32_t pi_Channel) const
    {
    HPRECONDITION(pi_Channel < m_ChannelCount);

    uint32_t MinimumCount = ULONG_MAX;
    uint32_t Entry = 0;

    for(uint32_t EntryIndex = 0; EntryIndex < m_EntryFrequenciesSize; EntryIndex++)
        {
        if(m_pEntryFrequencies[pi_Channel][EntryIndex] <= MinimumCount)
            {
            MinimumCount = m_pEntryFrequencies[pi_Channel][EntryIndex];
            Entry = EntryIndex;
            }
        }
    return Entry;
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

void HRPHistogram::ConstructUsingRGBColorSpace(uint32_t**  pi_pEntryFrequencies,
                                               uint32_t  pi_EntryFrequenciesSize,
                                               uint32_t  pi_ChannelCount)
    {
    HPRECONDITION(m_ColorSpace != LIGHTNESS);

    m_ChannelCount         = pi_ChannelCount;
    m_pEntryFrequencies    = new HArrayAutoPtr<uint32_t>[m_ChannelCount];
    m_EntryFrequenciesSize = pi_EntryFrequenciesSize;

    for (uint32_t ChannelIndex = 0; ChannelIndex < m_ChannelCount; ChannelIndex++)
        {
        m_pEntryFrequencies[ChannelIndex] = new uint32_t[m_EntryFrequenciesSize];
        if (!pi_pEntryFrequencies)
            {
            // If we don't have any given freq, initialize them..
            for(uint32_t EntryIndex = 0; EntryIndex < m_EntryFrequenciesSize; EntryIndex++)
                m_pEntryFrequencies[ChannelIndex][EntryIndex] = 0;
            }
        else
            {
            memcpy(m_pEntryFrequencies[ChannelIndex], pi_pEntryFrequencies[ChannelIndex], m_EntryFrequenciesSize * sizeof(uint32_t));
            }
        }
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

void HRPHistogram::ConstructUsingLightnessColorSpace(unsigned int pi_EntryFrequenciesSize)
    {
    m_ColorSpace           = LIGHTNESS;
    m_ChannelCount         = 1;
    m_pEntryFrequencies    = new HArrayAutoPtr<uint32_t>[m_ChannelCount];

    m_EntryFrequenciesSize = pi_EntryFrequenciesSize;

    for (uint32_t ChannelIndex = 0; ChannelIndex < m_ChannelCount; ChannelIndex++)
        {
        m_pEntryFrequencies[ChannelIndex] = new uint32_t[m_EntryFrequenciesSize];

        // If we don't have any given freq, initialize them..
        for(uint32_t EntryIndex = 0; EntryIndex < m_EntryFrequenciesSize; EntryIndex++)
            m_pEntryFrequencies[ChannelIndex][EntryIndex] = 0;
        }
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

void HRPHistogram::SetSamplingColorSpace(HRPHistogram::COLOR_SPACE pi_ColorSpace)
    {
    HASSERT (pi_ColorSpace != NATIVE);

    // Don't waste any time if there is real change..
    if (m_ColorSpace != pi_ColorSpace && pi_ColorSpace != NATIVE)
        {
        unsigned int EntryFrequenciesSize = m_EntryFrequenciesSize;

        // When changing the current color space, invalidate and
        // destroy the current Histogram.
        if (m_EntryFrequenciesSize || m_pEntryFrequencies != 0)
            {
            DeepDelete();
            }

        // Assign the new color space.
        m_ColorSpace = pi_ColorSpace;

        // Rebuild the Histogram according the new given ColorSpace.
        switch(m_ColorSpace)
            {
            case GRAYSCALE :
                ConstructUsingRGBColorSpace(0, 256, 1);
                break;

            case RGB       :
                ConstructUsingRGBColorSpace(0, 256, 3);
                break;

            case RGBA      :
                ConstructUsingRGBColorSpace(0, 256, 4);
                break;

            case LIGHTNESS :
                ConstructUsingLightnessColorSpace(EntryFrequenciesSize ? EntryFrequenciesSize : 256);
                break;

            default        : // Invalid or not handled ColorSpace.
                ConstructUsingRGBColorSpace(0, 256, 3);

                // Inform something wrong just happen when debugging.
                HASSERT(false);
            }
        }
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPHistogram::COLOR_SPACE HRPHistogram::AutoSelectColorSpace(uint32_t pi_ChannelCount)
    {
    HRPHistogram::COLOR_SPACE AutoSelection;

    switch (pi_ChannelCount)
        {
        case 1 :
            AutoSelection = GRAYSCALE;
            break;
        case 3 :
            AutoSelection = RGB;
            break;
        case 4 :
            AutoSelection = RGBA;
            break;

        default :
            AutoSelection = RGB;
            HASSERT(false); // Inform something wrong just happen when debugging.
        }
    return AutoSelection;
    }

//-----------------------------------------------------------------------------
// public
// FindEntryWithMaximumCount.
//-----------------------------------------------------------------------------

uint32_t HRPHistogram::FindEntryWithMaximumCount(uint32_t pi_Channel) const
    {
    HPRECONDITION(pi_Channel    < m_ChannelCount);

    uint32_t MaximumCount = 0;
    uint32_t Entry = 0;

    for(uint32_t EntryIndex = 0; EntryIndex < m_EntryFrequenciesSize; EntryIndex++)
        {
        if(m_pEntryFrequencies[pi_Channel][EntryIndex] >= MaximumCount)
            {
            MaximumCount = m_pEntryFrequencies[pi_Channel][EntryIndex];
            Entry = EntryIndex;
            }
        }
    return Entry;
    }

//-----------------------------------------------------------------------------
// Remove all frequencies.
//-----------------------------------------------------------------------------

void HRPHistogram::Clear()
    {
    for (uint32_t ChannelIndex = 0; ChannelIndex < m_ChannelCount; ChannelIndex++)
        {
        // If we don't have any given freq, initialize them..
        for(uint32_t EntryIndex = 0; EntryIndex < m_EntryFrequenciesSize; EntryIndex++)
            m_pEntryFrequencies[ChannelIndex][EntryIndex] = 0;
        }
    }
