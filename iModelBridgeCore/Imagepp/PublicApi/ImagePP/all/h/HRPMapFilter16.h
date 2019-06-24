//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRPMapFilter16
//-----------------------------------------------------------------------------
// A map filter for 8bits multiple channels pixel types
//-----------------------------------------------------------------------------

#pragma once

#include "HRPFunctionFilter.h"

BEGIN_IMAGEPP_NAMESPACE
class HRPMapFilter16 : public HRPFunctionFilter
    {
    HDECLARE_CLASS_ID(HRPFilterId_Map16, HRPFunctionFilter)

public:

    // Primary methods
    HRPMapFilter16();
    HRPMapFilter16(Byte pi_channels);
    HRPMapFilter16(const HFCPtr<HRPPixelType>& pi_pFilterPixelType);

    virtual            ~HRPMapFilter16();

    // Cloning
    HRPFilter* Clone() const override = 0;

    // Composing
    HRPFilter*
    ComposeWith(const HRPFilter* pi_pFilter) override;

    // Map methods
    void            SetMap(Byte pi_ChannelIndex, const uint16_t* pi_pMap);
    uint16_t*        GetMap(Byte pi_ChannelIndex);

protected:
    HRPMapFilter16(const HRPMapFilter16& pi_rFilter);

    void    Function(const void* pi_pSrcRawData, void* po_pDestRawData, uint32_t PixelsCount) const override;
    int32_t             GetMapSize() const;

    Byte          m_Channels;

private:
    // Disabled method.
    HRPMapFilter16& operator=(const HRPMapFilter16& i_Src);

    // members
    uint16_t*       m_pMap;
    uint32_t        m_MapSize;

    // methods
    void Initialize();
    void DeepDelete();
    };
END_IMAGEPP_NAMESPACE

