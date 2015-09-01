//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPContrastStretchFilter8.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

// Class : HRPContrastStretchFilter8
//-----------------------------------------------------------------------------
// A contrast stretch filter for 8bits multiple channels pixel types
//-----------------------------------------------------------------------------
#pragma once

#include "HRPMapFilter8.h"

BEGIN_IMAGEPP_NAMESPACE
class HRPContrastStretchFilter8 : public HRPMapFilter8
    {
    HDECLARE_CLASS_ID(HRPFilterId_ContrastStretch8, HRPMapFilter8)
    

public:             // Primary methods
    IMAGEPP_EXPORT          HRPContrastStretchFilter8();
    IMAGEPP_EXPORT          HRPContrastStretchFilter8(const HFCPtr<HRPPixelType>&      pi_pFilterPixelType);
    HRPContrastStretchFilter8(const HRPContrastStretchFilter8& pi_rFilter);

    IMAGEPP_EXPORT virtual  ~HRPContrastStretchFilter8();

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

    virtual void            GetGammaFactor( int32_t   pi_ChannelIndex,
                                            double* po_pGammaFactor) const;

    virtual void            SetGammaFactor( int32_t  pi_ChannelIndex,
                                            double pi_GammaFactor,
                                            bool   pi_MapUpdate = true);
protected:

private:
    HRPContrastStretchFilter8&
    operator=(const HRPContrastStretchFilter8& pi_rFilter);

#ifdef __HMR_DEBUG_MEMBER
    void            MapDump(int32_t pi_ChannelIndex);
#endif

    void            DeepCopy(const HRPContrastStretchFilter8& pi_rSrc);
    void            BuildMap(int32_t pi_ChannelIndex);

    // members
    int32_t*           m_pMinValue;
    int32_t*           m_pMaxValue;

    int32_t*           m_pMinContrastValue;
    int32_t*           m_pMaxContrastValue;

    double*        m_pGammaFactor;
    };
END_IMAGEPP_NAMESPACE


