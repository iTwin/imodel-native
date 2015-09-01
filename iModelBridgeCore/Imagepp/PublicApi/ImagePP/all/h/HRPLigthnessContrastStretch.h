//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPLigthnessContrastStretch.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRPFunctionFilters
//-----------------------------------------------------------------------------
// Some common function filters.
//-----------------------------------------------------------------------------

#pragma once

#include "HRPFunctionFilter.h"

BEGIN_IMAGEPP_NAMESPACE
// Forward declaration.
class HGFLuvColorSpace;

//-----------------------------------------------------------------------------
// HRPColortwistFilter
//-----------------------------------------------------------------------------

class HRPLigthnessContrastStretch : public HRPFunctionFilter
    {
    HDECLARE_CLASS_ID(HRPFilterId_LigthnessContrastStretch, HRPFunctionFilter)
    

public:

    // Primary methods
    HRPLigthnessContrastStretch();
    HRPLigthnessContrastStretch(const HCLASS_ID pi_PixelTypeClassID);
    HRPLigthnessContrastStretch(const HFCPtr<HRPPixelType>& pi_pFilterPixelType);
    HRPLigthnessContrastStretch(const HRPLigthnessContrastStretch& pi_rSrcFilter);

    virtual        ~HRPLigthnessContrastStretch();

    // Get/Set methods
    IMAGEPP_EXPORT void    SetInterval(uint32_t pi_ChannelIndex,
                        int pi_MinValue,
                        int pi_MaxValue);

    IMAGEPP_EXPORT void    GetInterval(uint32_t pi_ChannelIndex,
                        int* po_pMinValue,
                        int* po_pMaxValue) const;


    IMAGEPP_EXPORT void    SetContrastInterval(uint32_t pi_ChannelIndex,
                                int pi_MinContrastValue,
                                int pi_MaxContrastValue);

    IMAGEPP_EXPORT void    GetContrastInterval(uint32_t pi_ChannelIndex,
                                int* po_MinContrastValue,
                                int* po_MaxContrastValue) const;

    IMAGEPP_EXPORT void    GetGammaFactor(uint32_t pi_ChannelIndex, double* po_pGammaFactor) const;

    IMAGEPP_EXPORT void    SetGammaFactor(uint32_t pi_ChannelIndex, double pi_GammaFactor);

    virtual HRPFilter* Clone() const override;

protected:

    // methods
    virtual void    Function  (const void*  pi_pSrcRawData,
                               void*  po_pDestRawData,
                               uint32_t pi_PixelsCount) const override;

private:
    // Disabled method.
    HRPLigthnessContrastStretch& operator = (const HRPLigthnessContrastStretch& pi_rFilter);

    void                DeepCopy(const HRPLigthnessContrastStretch& pi_rSrc);

    void                FunctionN8 ( const void*  pi_pSrcRawData,
                                     void*  po_pDestRawData,
                                     uint32_t pi_PixelsCount) const;

    void                FunctionN16( const void*  pi_pSrcRawData,
                                     void*  po_pDestRawData,
                                     uint32_t pi_PixelsCount) const;
    // members
    uint32_t    m_Channels;
    unsigned short m_ChannelWidth;
    double      m_MaxSampleValue;

    int*         m_pMinValue;
    int*         m_pMaxValue;
    int*         m_pMinContrastValue;
    int*         m_pMaxContrastValue;

    double*      m_pGammaFactor;

    HGFLuvColorSpace*
    m_pColorSpaceConverter;
    };
END_IMAGEPP_NAMESPACE

