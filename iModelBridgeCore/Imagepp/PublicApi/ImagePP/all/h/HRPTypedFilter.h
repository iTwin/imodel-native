//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPTypedFilter.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRPTypedFilter
//-----------------------------------------------------------------------------
// This class describes the interface for a filter with a pixel type associated
// Abstract class.
//-----------------------------------------------------------------------------
#pragma once

#include "HRPFilter.h"
#include "HRPPixelConverter.h"

BEGIN_IMAGEPP_NAMESPACE
class HRPTypedFilter : public HRPFilter
    {
    HDECLARE_CLASS_ID(HRPFilterId_Typed, HRPFilter)

public:
    // Primary methods
    IMAGEPP_EXPORT virtual                 ~HRPTypedFilter();

    // Cloning
    virtual HRPFilter* Clone() const override = 0;

    // Pixel types
    const HFCPtr<HRPPixelType>&
    GetFilterPixelType() const;
    IMAGEPP_EXPORT void                    SetInputPixelType(
        const HFCPtr<HRPPixelType>& pi_pInputPixelType);
    IMAGEPP_EXPORT void                    SetOutputPixelType(
        const HFCPtr<HRPPixelType>& pi_pOutputPixelType);

protected:

    // Primary methods
    IMAGEPP_EXPORT                         HRPTypedFilter( const HFCPtr<HRPPixelType>& pi_pFilterPixelType);
    IMAGEPP_EXPORT                         HRPTypedFilter( const HFCPtr<HRPPixelType>& pi_pFilterPixelType,
                                                   const HRPPixelNeighbourhood& pi_rNeighbourhood);

    IMAGEPP_EXPORT                         HRPTypedFilter( const HRPTypedFilter& pi_rFilter);

    // Converters
    bool                   AreThereLostChannels() const;
    const HFCPtr<HRPPixelConverter>&
    GetInputConverter() const;
    const HFCPtr<HRPPixelConverter>&
    GetOutputConverter() const;
    const HFCPtr<HRPPixelConverter>&
    GetInOutConverter() const;
    const bool*            GetLostChannelsMask() const;


private:

    // Atributes
    HFCPtr<HRPPixelType>                m_pFilterPixelType;
    HFCPtr<HRPPixelConverter>           m_pInputConverter;
    HFCPtr<HRPPixelConverter>           m_pOutputConverter;
    HFCPtr<HRPPixelConverter>           m_pInOutConverter;
    bool                               m_LostChannels;

    // 128 is an arbitrary maximum number of channels
    bool                               m_CopyLostChannelsMask[HRPCHANNELORG_MAX_CHANNELS_COUNT];

    // Methods
    void                                SetInputConverter();
    void                                SetOutputConverter();
    void                                UpdateLostChannelsProcessing();
    };
END_IMAGEPP_NAMESPACE

#include "HRPTypedFilter.hpp"

