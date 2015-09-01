//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPChannelOrg.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRPChannelOrg
//-----------------------------------------------------------------------------
// Channel Organisation: describes order and nature of channels used in rasters
// and pixel palettes.
//-----------------------------------------------------------------------------
#pragma once

#include "HRPChannelType.h"
#include "HFCPtr.h"

BEGIN_IMAGEPP_NAMESPACE
#define HRPCHANNELORG_MAX_CHANNELS_COUNT (254)


class HRPChannelOrg : public HFCShareableObject<HRPChannelOrg>
    {
    HDECLARE_BASECLASS_ID(HRPChannelOrgId_Base)

public:

    // Primary methods

    IMAGEPP_EXPORT                 HRPChannelOrg();
    IMAGEPP_EXPORT                 HRPChannelOrg(const HRPChannelOrg& pi_rObj);
    IMAGEPP_EXPORT virtual         ~HRPChannelOrg();

    HRPChannelOrg&  operator=(const HRPChannelOrg& pi_rObj);

    // Channel Setup
    IMAGEPP_EXPORT uint32_t AddChannel(const HRPChannelType& pi_rChannelType);
    void            DeleteChannel(HRPChannelType* pi_pChannel);

    // Channel access methods
    const HRPChannelType*
    GetChannelPtr(uint32_t pi_Index) const;

    uint32_t        CountChannels() const;

    const HRPChannelType*
    operator[](uint32_t pi_Index) const;

    IMAGEPP_EXPORT /*IppImaging_Needs*/ bool    operator==(const HRPChannelOrg& pi_rChannelOrg) const;
    bool           operator!=(const HRPChannelOrg& pi_rChannelOrg) const;
    uint32_t        GetChannelIndex(HRPChannelType::ChannelRole pi_Role,
                                    unsigned short pi_Id) const;
    // Other methods
    uint32_t        CountPixelCompositeValueBits() const;

    //! All channels have the same size.    
    bool HaveSameSize() const;

    //! All channels have the same data type.
    bool HaveSameDataType() const;
    
protected:

private:

    // Attributes

    // The following list stores pointers to the channels sorted
    // by insertion position.  It gives quick access to channels
    // when it position is known.
    typedef vector<HFCPtr<HRPChannelType>, allocator<HFCPtr<HRPChannelType> > >
    ListHRPChannelType;
    ListHRPChannelType          m_ChannelsByPosition;

    // The following list stores pointers to the channels sorted
    // by Id.  Access is slower than on the list by position because
    // a binary search is done upon access.
    Byte m_ChannelsById[HRPCHANNELORG_MAX_CHANNELS_COUNT];

    // This array stores lists of channels for each role of channel
    // This gives quick access to channels when their role is known
    // This is not used to index channels of role UNUSED and USER
    Byte m_ChannelsByRole[HRPCHANNELTYPE_NB_CHANNEL_ROLES];

    // Number of bits for the pixel value (sum of the number of
    // bits of all channels).
    uint32_t                        m_PixelCompositeValueBits;

    // Methods
    void           DeepDelete();
    void           DeepCopy(const HRPChannelOrg& pi_rObj);
    bool            HasSameChannelsAs(const HRPChannelOrg& pi_rChannelOrg) const;
    };
END_IMAGEPP_NAMESPACE

#include "HRPChannelOrg.hpp"
