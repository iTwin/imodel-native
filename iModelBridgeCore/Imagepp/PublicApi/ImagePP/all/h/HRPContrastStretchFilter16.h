//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPContrastStretchFilter16.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRPContrastStretchFilter16
//-----------------------------------------------------------------------------
// A contrast stretch filter for 16bits multiple channels pixel types
//-----------------------------------------------------------------------------
#pragma once

#include "HRPMapFilter16.h"

BEGIN_IMAGEPP_NAMESPACE
class HRPContrastStretchFilter16 : public HRPMapFilter16
    {
    HDECLARE_CLASS_ID(HRPFilterId_ContrastStretch16, HRPMapFilter16)

public:             // Primary methods
    IMAGEPP_EXPORT          HRPContrastStretchFilter16();
    IMAGEPP_EXPORT          HRPContrastStretchFilter16(const HFCPtr<HRPPixelType>&       pi_pFilterPixelType);
    HRPContrastStretchFilter16(const HRPContrastStretchFilter16& pi_rFilter);

    virtual            ~HRPContrastStretchFilter16();

    // Cloning
    HRPFilter* Clone() const override;

    // Get/Set methods
    virtual void            SetInterval(int32_t pi_ChannelIndex,
                                        int32_t pi_MinValue,
                                        int32_t pi_MaxValue,
                                        bool pi_MapUpdate = true);

    virtual void            GetInterval(int32_t pi_ChannelIndex,
                                        int32_t* po_pMinValue,
                                        int32_t* po_pMaxValue) const;


    virtual void            SetContrastInterval(int32_t pi_ChannelIndex,
                                                int32_t pi_MinContrastValue,
                                                int32_t pi_MaxContrastValue,
                                                bool pi_MapUpdate = true);

    virtual void            GetContrastInterval(int32_t pi_ChannelIndex,
                                                int32_t* po_MinContrastValue,
                                                int32_t* po_MaxContrastValue) const;

    virtual void            GetGammaFactor(int32_t   pi_ChannelIndex,
                                           double* po_pGammaFactor) const;

    virtual void            SetGammaFactor(int32_t  pi_ChannelIndex,
                                           double pi_GammaFactor,
                                           bool   pi_MapUpdate = true);
protected:

private:
    HRPContrastStretchFilter16&
    operator=(const HRPContrastStretchFilter16& pi_rFilter);

#ifdef __HMR_DEBUG_MEMBER
    void            MapDump(int32_t pi_ChannelIndex);
#endif

    void            DeepCopy(const HRPContrastStretchFilter16& pi_rSrc);
    void            BuildMap(int32_t pi_ChannelIndex);

    // members
    int32_t*        m_pMinValue;
    int32_t*        m_pMaxValue;

    int32_t*        m_pMinContrastValue;
    int32_t*        m_pMaxContrastValue;

    double*     m_pGammaFactor;
    };
END_IMAGEPP_NAMESPACE


