//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrp/src/HRPChannelOrg.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HRPChannelOrg
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HRPChannelOrg.h>

//-----------------------------------------------------------------------------
// Constructor.  It takes no argument, object properties must be set with
// channel and palette setup methods.
//-----------------------------------------------------------------------------
HRPChannelOrg::HRPChannelOrg()
    : m_ChannelsByPosition()
    {
    size_t i;

    // Pixel values have initially a 0 size since there is no channel yet
    m_PixelCompositeValueBits = 0;

    // Initialize the index of channels by role
    for (i=0; i< HRPCHANNELTYPE_NB_CHANNEL_ROLES; ++i)
        m_ChannelsByRole[i] = HRPChannelType::FREE;

    // Initialize the index of channels by id
    for (i=0; i<HRPCHANNELORG_MAX_CHANNELS_COUNT; ++i)
        m_ChannelsById[i] = HRPChannelType::FREE;
    }

//-----------------------------------------------------------------------------
// Copy constructor.
//-----------------------------------------------------------------------------
HRPChannelOrg::HRPChannelOrg(const HRPChannelOrg& pi_rObj)
    : m_ChannelsByPosition()
    {
    unsigned short i;

    // Initialize the index of channels by role
    for (i=0; i< HRPCHANNELTYPE_NB_CHANNEL_ROLES; ++i)
        m_ChannelsByRole[i] = HRPChannelType::FREE;

    // Initialize the index of channels by id
    for (i=0; i<HRPCHANNELORG_MAX_CHANNELS_COUNT; ++i)
        m_ChannelsById[i] = HRPChannelType::FREE;

    DeepCopy(pi_rObj);
    }

//-----------------------------------------------------------------------------
// The destructor.
//-----------------------------------------------------------------------------
HRPChannelOrg::~HRPChannelOrg()
    {
    DeepDelete();
    }

//-----------------------------------------------------------------------------
// Adds a new channel definition into this pixel type.  Uses a constant
// reference to a channel object to copy from.
//-----------------------------------------------------------------------------
uint32_t HRPChannelOrg::AddChannel(const HRPChannelType& pi_rChannelType)
    {
    Byte Index = (Byte)m_ChannelsByPosition.size();

    // Dont allow duplicate channels of the same types
    HRPChannelType::ChannelRole Role = pi_rChannelType.GetRole();
    HASSERT(m_ChannelsByRole[Role] == HRPChannelType::FREE);
    m_ChannelsByRole[Role] = Index;

    unsigned short Id = pi_rChannelType.GetId();
    if(Id != 0)
        {
        HASSERT(m_ChannelsById[Id] != HRPChannelType::FREE);
        m_ChannelsById[Id] = Index;
        }

    HRPChannelType* pChannel = new HRPChannelType(pi_rChannelType);

    m_ChannelsByPosition.push_back (pChannel);

    m_PixelCompositeValueBits += pi_rChannelType.GetSize();

    return Index;
    }

//-----------------------------------------------------------------------------
// Copies everything owned by the object
//-----------------------------------------------------------------------------
void HRPChannelOrg::DeepCopy(const HRPChannelOrg& pi_rObj)
    {
    // Copy the content of the channels list
    // Doing this instead of operator= on the lists ensures that
    // each channel type object is copied (not only the pointers
    // to the channel types)
    m_PixelCompositeValueBits = 0;

    for(uint32_t i=0; i < pi_rObj.CountChannels(); i++)
        AddChannel(*(pi_rObj.GetChannelPtr(i)));
    }

//-----------------------------------------------------------------------------
// Deletes everything owned by the object
//-----------------------------------------------------------------------------
void HRPChannelOrg::DeepDelete()
    {
    unsigned short i;

    m_ChannelsByPosition.clear();

    // Reset the index of channels by role
    for (i=0; i< HRPCHANNELTYPE_NB_CHANNEL_ROLES; i++)
        m_ChannelsByRole[i] = HRPChannelType::FREE;

    // Reset the index of channels by id
    for (i=0; i<HRPCHANNELORG_MAX_CHANNELS_COUNT; i++)
        m_ChannelsById[i] = HRPChannelType::FREE;
    }

//-----------------------------------------------------------------------------
// Deletes a channel definition from this pixel type.
//-----------------------------------------------------------------------------
void HRPChannelOrg::DeleteChannel(HRPChannelType* pi_pChannel)
    {
    HPRECONDITION(pi_pChannel != 0);

    ListHRPChannelType::iterator Itr;
    if ((Itr = find (m_ChannelsByPosition.begin(), m_ChannelsByPosition.end(), (const HRPChannelType*)pi_pChannel)) ==
        m_ChannelsByPosition.end())
        {
        HASSERT(0);
        }

    m_ChannelsById[pi_pChannel->GetId()] = HRPChannelType::FREE;
    m_ChannelsByRole[pi_pChannel->GetRole()] = HRPChannelType::FREE;

    m_PixelCompositeValueBits -= pi_pChannel->GetSize();

    // This must be last because channels are kept using HFCPtrs.
    // Removing the channel will delete it (we're normally the only
    // one pointing to it.)
    m_ChannelsByPosition.erase (Itr);
    }

//-----------------------------------------------------------------------------
// Returns true only if both types have the same structure (palette not compared)
//-----------------------------------------------------------------------------
bool HRPChannelOrg::HasSameChannelsAs(const HRPChannelOrg& pi_rChannelOrg) const
    {
    bool State = true;

    // WE USE MULTIPLE RETURNS BECAUSE THIS IS FASTER

    // Return false if not same number of channels
    if(CountChannels() != pi_rChannelOrg.CountChannels())
        {
        State = false;
        }
    else
        {
        // Return false if one of the channels is different
        for(uint32_t i=0; i < CountChannels() && State; i++)
            {
            if((*(GetChannelPtr(i))) != (*(pi_rChannelOrg.GetChannelPtr(i))))
                State = false;
            }
        }

    return State;
    }

//-----------------------------------------------------------------------------
// Assignment operator.  It duplicates another pixel type, doing a deep copy.
//-----------------------------------------------------------------------------
HRPChannelOrg& HRPChannelOrg::operator=(const HRPChannelOrg& pi_rObj)
    {
    if (this != &pi_rObj)
        {
        DeepDelete();
        DeepCopy(pi_rObj);
        }
    return *this;
    }

//-----------------------------------------------------------------------------
// Equal To operator
//-----------------------------------------------------------------------------
bool HRPChannelOrg::operator==(const HRPChannelOrg& pi_rObj) const
    {
    return(m_PixelCompositeValueBits == pi_rObj.m_PixelCompositeValueBits &&
           HasSameChannelsAs(pi_rObj));
    }

//-----------------------------------------------------------------------------
// Not Equal To operator
//-----------------------------------------------------------------------------
bool HRPChannelOrg::operator!=(const HRPChannelOrg& pi_rObj) const
    {
    return(!operator==(pi_rObj));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool HRPChannelOrg::HaveSameSize() const
    {
    for(uint32_t i=1; i < CountChannels(); ++i)
        {
        if(GetChannelPtr(i-1)->GetSize() != GetChannelPtr(i)->GetSize())
            return false;
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool HRPChannelOrg::HaveSameDataType() const
    {
    for(uint32_t i=1; i < CountChannels(); ++i)
        {
        if(GetChannelPtr(i-1)->GetDataType() != GetChannelPtr(i)->GetDataType())
            return false;
        }

    return true;

    }