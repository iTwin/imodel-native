//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPLigthnessContrastStretch.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRPFunctionFilters
//-----------------------------------------------------------------------------
// Some common function filters.
//-----------------------------------------------------------------------------

#pragma once

#include "HRPFunctionFilter.h"

// Forward declaration.
class HGFLuvColorSpace;

//-----------------------------------------------------------------------------
// HRPColortwistFilter
//-----------------------------------------------------------------------------

class HRPLigthnessContrastStretch : public HRPFunctionFilter
    {
    HDECLARE_CLASS_ID(1351, HRPFunctionFilter)
    

public:

    // Primary methods
    HRPLigthnessContrastStretch();
    HRPLigthnessContrastStretch(const HCLASS_ID pi_PixelTypeClassID);
    HRPLigthnessContrastStretch(const HFCPtr<HRPPixelType>& pi_pFilterPixelType);
    HRPLigthnessContrastStretch(const HRPLigthnessContrastStretch& pi_rSrcFilter);

    virtual        ~HRPLigthnessContrastStretch();

    // Get/Set methods
    virtual void    SetInterval(        int pi_ChannelIndex,
                                        int pi_MinValue,
                                        int pi_MaxValue);

    virtual void    GetInterval(        int  pi_ChannelIndex,
                                        int* po_pMinValue,
                                        int* po_pMaxValue) const;


    virtual void    SetContrastInterval(int pi_ChannelIndex,
                                        int pi_MinContrastValue,
                                        int pi_MaxContrastValue);

    virtual void    GetContrastInterval(int  pi_ChannelIndex,
                                        int* po_MinContrastValue,
                                        int* po_MaxContrastValue) const;

    virtual void    GetGammaFactor(     int     pi_ChannelIndex,
                                        double* po_pGammaFactor) const;

    virtual void    SetGammaFactor(     int    pi_ChannelIndex,
                                        double pi_GammaFactor);
    // Cloning
    virtual HRPFilter* Clone() const override;

protected:

    // methods
    virtual void    Function  (const void*  pi_pSrcRawData,
                               void*  po_pDestRawData,
                               uint32_t pi_PixelsCount) const;

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
    unsigned short m_Channels;
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

