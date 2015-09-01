//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrp/src/HRPTypedFilter.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HRPTypedFilter
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HRPTypedFilter.h>

//-----------------------------------------------------------------------------
// protected
// Constructor.
//-----------------------------------------------------------------------------
HRPTypedFilter::HRPTypedFilter(const HFCPtr<HRPPixelType>& pi_pFilterPixelType)
    :  HRPFilter()
    {
    HPRECONDITION(pi_pFilterPixelType != 0);

    m_pFilterPixelType = pi_pFilterPixelType;
    m_LostChannels = false;
    }

//-----------------------------------------------------------------------------
// protected
// Constructor.
//-----------------------------------------------------------------------------
HRPTypedFilter::HRPTypedFilter(const HFCPtr<HRPPixelType>& pi_pFilterPixelType,
                               const HRPPixelNeighbourhood& pi_rNeighbourhood) :
    HRPFilter(pi_rNeighbourhood)
    {
    HPRECONDITION(pi_pFilterPixelType != 0);

    m_pFilterPixelType = pi_pFilterPixelType;
    m_LostChannels = false;
    }

//-----------------------------------------------------------------------------
// protected
// Copy constructor.
//-----------------------------------------------------------------------------
HRPTypedFilter::HRPTypedFilter(const HRPTypedFilter& pi_rFilter)
    : HRPFilter(pi_rFilter)
    {
    m_pFilterPixelType = pi_rFilter.m_pFilterPixelType;
    m_pInputConverter = pi_rFilter.m_pInputConverter;
    m_pOutputConverter = pi_rFilter.m_pOutputConverter;
    m_pInOutConverter = pi_rFilter.m_pInOutConverter;
    m_LostChannels = pi_rFilter.m_LostChannels;

    // Copy the lost channels mask
    for(uint32_t Index = 0; Index < HRPCHANNELORG_MAX_CHANNELS_COUNT; Index++)
        m_CopyLostChannelsMask[Index] = pi_rFilter.m_CopyLostChannelsMask[Index];
    }

//-----------------------------------------------------------------------------
// public
// Destructor.
//-----------------------------------------------------------------------------
HRPTypedFilter::~HRPTypedFilter()
    {
    }

//-----------------------------------------------------------------------------
// public
// SetInputPixelType.
//-----------------------------------------------------------------------------
void HRPTypedFilter::SetInputPixelType(const HFCPtr<HRPPixelType>& pi_pInputPixelType)
    {
    HPRECONDITION(pi_pInputPixelType != 0);

    HRPFilter::SetInputPixelType(pi_pInputPixelType);

    SetInputConverter();

    // Update lost channels processing when filtering
    if(m_pOutputConverter != 0)
        UpdateLostChannelsProcessing();
    }

//-----------------------------------------------------------------------------
// public
// SetOutputPixelType.
//-----------------------------------------------------------------------------
void HRPTypedFilter::SetOutputPixelType(const HFCPtr<HRPPixelType>& pi_pOutputPixelType)
    {
    HPRECONDITION(pi_pOutputPixelType != 0);

    HRPFilter::SetOutputPixelType(pi_pOutputPixelType);

    SetOutputConverter();

    // Update lost channels processing when filtering
    if(m_pInputConverter != 0)
        UpdateLostChannelsProcessing();
    }


//-----------------------------------------------------------------------------
// private
// SetInputConverter
//-----------------------------------------------------------------------------
void HRPTypedFilter::SetInputConverter()
    {
    const HFCPtr<HRPPixelType>& pInputPixelType = GetInputPixelType();

    HASSERT(pInputPixelType != 0);

    // Compare the input and the filter pixel types to check if they are different
    // (we do not use == because we do not want to compare the default
    //              raw data value in each pixel type)
    if(!((pInputPixelType->GetClassID() == m_pFilterPixelType->GetClassID()) &&
         (pInputPixelType->GetPalette() == m_pFilterPixelType->GetPalette())))
        // Create an input converter
        {
        m_pInputConverter =
            m_pFilterPixelType->GetConverterFrom(pInputPixelType);

        HASSERT(m_pInputConverter != 0);
        }
    else
        {
        m_pInputConverter = 0;
        }
    }

//-----------------------------------------------------------------------------
// private
// SetOutputConverter
//-----------------------------------------------------------------------------
void HRPTypedFilter::SetOutputConverter()
    {
    const HFCPtr<HRPPixelType>& pOutputPixelType = GetOutputPixelType();

    HASSERT(pOutputPixelType != 0);

    // Compare the filter and output pixel types to check if they are different
    // (we do not use == because we do not want to compare the default
    //              raw data value in each pixel type)
    if(!((pOutputPixelType->GetClassID() == m_pFilterPixelType->GetClassID()) &&
         (pOutputPixelType->GetPalette() == m_pFilterPixelType->GetPalette())))
        // Create an output converter
        {
        m_pOutputConverter =
            m_pFilterPixelType->GetConverterTo(pOutputPixelType);

        HASSERT(m_pOutputConverter != 0);
        }
    else
        {
        m_pOutputConverter = 0;
        }
    }

//-----------------------------------------------------------------------------
// private
// UpdateLostChannelsProcessing
//-----------------------------------------------------------------------------
void HRPTypedFilter::UpdateLostChannelsProcessing()
    {
    // by default, there is no lost channels
    m_LostChannels = false;

    // not go further if there is one or no converter defined
    if(!(m_pInputConverter == 0 || m_pOutputConverter == 0))
        {
        // first, we must have an input converter to have the possibility of having
        // lost channels
        const short* pLostChannels;
        if((pLostChannels = m_pInputConverter->GetLostChannels()) != NULL)
            {
            const HRPChannelOrg& rInputChannelOrg = GetInputPixelType()->GetChannelOrg();

            size_t ChannelIndex;

            // clear the copy mask
            for(ChannelIndex = 0; ChannelIndex < rInputChannelOrg.CountChannels(); ChannelIndex++)
                m_CopyLostChannelsMask[ChannelIndex] = false;

            // get the lost channel in the input conversion
            const short* pLostChannels = m_pInputConverter->GetLostChannels();

            // for each channel lost, we verify if this channel is in the output pixel type
            const HRPChannelOrg& rOutputChannelOrg = GetOutputPixelType()->GetChannelOrg();
            HRPChannelType::ChannelRole ChannelRole;
            unsigned short ChannelId=0;
            ChannelIndex = 0;
            while(pLostChannels[ChannelIndex] != -1)
                {
                // get the channel role and id of the lost channel
                ChannelRole = rInputChannelOrg[pLostChannels[ChannelIndex]]->GetRole();
                if(ChannelRole == HRPChannelType::USER)
                    ChannelId = rInputChannelOrg[pLostChannels[ChannelIndex]]->GetId();

                // verify if this channel role (and id) is in the output channel org
                for(uint32_t OutChannelIndex = 0; OutChannelIndex < rOutputChannelOrg.CountChannels(); OutChannelIndex++)
                    {
                    if(rOutputChannelOrg[OutChannelIndex]->GetRole() == ChannelRole)
                        {
                        if(ChannelRole == HRPChannelType::USER)
                            {
                            if(rOutputChannelOrg[OutChannelIndex]->GetId() == ChannelId)
                                {
                                // update the lost channels mask
                                m_CopyLostChannelsMask[pLostChannels[ChannelIndex]] = true;

                                // set the flag telling that there are lost channels
                                m_LostChannels = true;

                                break;
                                }
                            }
                        else
                            {
                            // update the lost channels mask
                            m_CopyLostChannelsMask[pLostChannels[ChannelIndex]] = true;

                            // set the flag telling that there are lost channels
                            m_LostChannels = true;

                            break;
                            }
                        }
                    }

                // next lost channel
                ChannelIndex++;
                }

            // if there are lost channels, we create an in/out converter
            if(m_LostChannels)
                {
                m_pInOutConverter = GetInputPixelType()->GetConverterTo(GetOutputPixelType());
                }
            }
        }
    }