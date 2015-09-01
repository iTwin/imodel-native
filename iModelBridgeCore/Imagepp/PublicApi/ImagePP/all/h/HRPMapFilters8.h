//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPMapFilters8.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRPMapFilters8
//-----------------------------------------------------------------------------
// Map filters
//-----------------------------------------------------------------------------
#pragma once

#include "HRPMapFilter8.h"

//-----------------------------------------------------------------------------
//  Custom Map8 Filter
//-----------------------------------------------------------------------------
BEGIN_IMAGEPP_NAMESPACE
class HRPCustomMap8Filter : public HRPMapFilter8
    {
    HDECLARE_CLASS_ID(HRPFilterId_CustomMap8, HRPMapFilter8)
    

public:

    // Primary methods

    IMAGEPP_EXPORT                 HRPCustomMap8Filter();
    IMAGEPP_EXPORT                 HRPCustomMap8Filter(Byte pi_channels);
    IMAGEPP_EXPORT                 HRPCustomMap8Filter(const HFCPtr<HRPPixelType>& pi_pFilterPixelType);
    IMAGEPP_EXPORT virtual         ~HRPCustomMap8Filter();

    virtual HRPFilter* Clone() const override;

protected:
    HRPCustomMap8Filter(const HRPCustomMap8Filter& pi_rObj);
private:
    // Disabled
    HRPCustomMap8Filter& operator=(const HRPCustomMap8Filter& pi_rObj);
    };

//-----------------------------------------------------------------------------
//  Color Balance Filter
//-----------------------------------------------------------------------------
class HRPColorBalanceFilter : public HRPMapFilter8
    {
    HDECLARE_CLASS_ID(HRPFilterId_ColorBalance, HRPMapFilter8)
    

public:

    // Primary methods

    IMAGEPP_EXPORT                 HRPColorBalanceFilter();
    IMAGEPP_EXPORT                 HRPColorBalanceFilter(  int32_t pi_RedVar,
                                                   int32_t pi_GreenVar,
                                                   int32_t pi_BlueVar);
    IMAGEPP_EXPORT                 HRPColorBalanceFilter(  int32_t pi_GlobalVar);
    IMAGEPP_EXPORT                 HRPColorBalanceFilter(int32_t pi_GlobalVar, const HFCPtr<HRPPixelType>& pi_pFilterPixelType);

    IMAGEPP_EXPORT virtual         ~HRPColorBalanceFilter();

    virtual HRPFilter* Clone() const override;


    int32_t        GetRedVariation() const;
    int32_t        GetGreenVariation() const;
    int32_t        GetBlueVariation() const;

protected:
    HRPColorBalanceFilter(const HRPColorBalanceFilter& pi_rObj);
private:
    // Disabled
    HRPColorBalanceFilter&
    operator=(const HRPColorBalanceFilter& pi_rObj);

    void             InitializeMap(         int32_t pi_RedVar,
                                            int32_t pi_GreenVar,
                                            int32_t pi_BlueVar);


    int32_t        m_RedVar;
    int32_t        m_GreenVar;
    int32_t        m_BlueVar;
    };

//-----------------------------------------------------------------------------
//  Contrast Filter
//-----------------------------------------------------------------------------
class HRPContrastFilter : public HRPMapFilter8
    {
    HDECLARE_CLASS_ID(HRPFilterId_Contrast, HRPMapFilter8)
    

public:

    IMAGEPP_EXPORT                 HRPContrastFilter();
    IMAGEPP_EXPORT                 HRPContrastFilter(int8_t pi_Var);
    IMAGEPP_EXPORT                 HRPContrastFilter(int8_t pi_Var,const HFCPtr<HRPPixelType>& pi_pFilterPixelType);
    IMAGEPP_EXPORT virtual         ~HRPContrastFilter();

    virtual HRPFilter* Clone() const override;

    IMAGEPP_EXPORT int8_t        GetIntensity() const;

protected:
    void            Init(int8_t pi_Var);

    HRPContrastFilter(const HRPContrastFilter& pi_rObj);

private:

    // Disabled
    HRPContrastFilter&
    operator=(const HRPContrastFilter& pi_rObj);

    int32_t        m_Intensity;
    };


//-----------------------------------------------------------------------------
//  HRPHistogramScalingFilter
//-----------------------------------------------------------------------------
class HRPHistogramScalingFilter : public HRPMapFilter8
    {
    HDECLARE_CLASS_ID(HRPFilterId_HistogramScaling, HRPMapFilter8)
    

public:
    enum HistogramScalingMode
        {
        INPUT_RANGE_CLIPPING,
        OUTPUT_RANGE_COMPRESSION
        };

    // Primary methods
    IMAGEPP_EXPORT                 HRPHistogramScalingFilter();
    IMAGEPP_EXPORT                 HRPHistogramScalingFilter(const HFCPtr<HRPPixelType>&   pi_pFilterPixelType,
                                                     HistogramScalingMode          pi_ScalingMode);
    IMAGEPP_EXPORT virtual         ~HRPHistogramScalingFilter();

    virtual HRPFilter* Clone() const override;

    // Get/Set methods
    IMAGEPP_EXPORT void            SetInterval(unsigned short pi_ChannelIndex,
                                       unsigned short pi_MinValue,
                                       unsigned short pi_MaxValue);

    void            GetInterval(unsigned short pi_ChannelIndex,
                                unsigned short*    po_pMinValue,
                                unsigned short*    po_pMaxValue);

    void                    SetScalingMode(HistogramScalingMode pi_ScalingMode);
    HistogramScalingMode    GetScalingMode() const;

protected:
    HRPHistogramScalingFilter(const HRPHistogramScalingFilter& pi_rObj);

private:

    // members
    HistogramScalingMode    m_ScalingMode;

    // disable methods
    HRPHistogramScalingFilter&
    operator=(const HRPHistogramScalingFilter& pi_rObj);
    };

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

class HRPGammaFilter : public HRPMapFilter8
    {
    HDECLARE_CLASS_ID(HRPFilterId_Gamma, HRPMapFilter8)
    

public:

    IMAGEPP_EXPORT HRPGammaFilter();
    IMAGEPP_EXPORT HRPGammaFilter(double pi_Gamma);
    IMAGEPP_EXPORT virtual ~HRPGammaFilter();

    virtual HRPFilter* Clone() const override;

    double  GetGamma() const;
protected:
    HRPGammaFilter(const HRPGammaFilter& pi_rObj);

private:
    double m_gamma;
    // Disabled methods
    HRPGammaFilter& operator=(const HRPGammaFilter& pi_rObj);
    };

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

class HRPInvertFilter : public HRPMapFilter8
    {
    HDECLARE_CLASS_ID(HRPFilterId_Invert, HRPMapFilter8)
    

public:

    IMAGEPP_EXPORT HRPInvertFilter();
    IMAGEPP_EXPORT virtual ~HRPInvertFilter();

    virtual HRPFilter* Clone() const override;

protected:
    HRPInvertFilter(const HRPInvertFilter& pi_rObj);

private:
    // Disabled methods
    HRPInvertFilter& operator =(const HRPInvertFilter& pi_rObj);
    };

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

class HRPTintFilter : public HRPMapFilter8
    {
    HDECLARE_CLASS_ID(HRPFilterId_Tint, HRPMapFilter8)
    

public:

    IMAGEPP_EXPORT HRPTintFilter();
    IMAGEPP_EXPORT HRPTintFilter(Byte pi_TintColor[3]);
    IMAGEPP_EXPORT virtual ~HRPTintFilter();

    virtual HRPFilter* Clone() const override;

protected:
    HRPTintFilter(const HRPTintFilter& pi_rObj);

private:
    // Disabled methods
    HRPTintFilter& operator =(const HRPTintFilter& pi_rObj);
    };
END_IMAGEPP_NAMESPACE

