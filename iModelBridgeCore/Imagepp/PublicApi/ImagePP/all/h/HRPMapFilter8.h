//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPMapFilter8.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRPMapFilter8
//-----------------------------------------------------------------------------
// A map filter for 8bits multiple channels pixel types
//-----------------------------------------------------------------------------

#pragma once

#include "HRPFunctionFilter.h"

BEGIN_IMAGEPP_NAMESPACE
class HRPMapFilter8 : public HRPFunctionFilter
    {
    HDECLARE_CLASS_ID(HRPFilterId_Map8, HRPFunctionFilter)

public:

    // Primary methods
    HRPMapFilter8();
    HRPMapFilter8(Byte pi_channels);
    HRPMapFilter8(const HFCPtr<HRPPixelType>& pi_pFilterPixelType);


    virtual            ~HRPMapFilter8();

    // Cloning
    HRPFilter* Clone() const override = 0;

    // Composing

    virtual HRPFilter* ComposeWith(const HRPFilter* pi_pFilter);


    // Map methods

    IMAGEPP_EXPORT virtual void  SetMap(size_t pi_ChannelIndex, const Byte* pi_pMap);
    IMAGEPP_EXPORT virtual Byte* GetMap(uint32_t pi_ChannelIndex);

protected:
    HRPMapFilter8(const HRPMapFilter8& pi_rFilter);

    virtual void    Function(const void* pi_pSrcRawData, void* po_pDestRawData, uint32_t PixelsCount) const;

    Byte          m_Channels;
    size_t          m_MapSize;

private:
    // Disabled method.
    HRPMapFilter8& operator=(const HRPMapFilter8& i_rSrc);

    // members
    Byte*         m_pMap;

    // methods
    void Initialize();
    void DeepDelete();
    };
END_IMAGEPP_NAMESPACE

