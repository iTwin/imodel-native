//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPMapFilters16.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRPMapFilters16
//-----------------------------------------------------------------------------
// Map filters
//-----------------------------------------------------------------------------

#pragma once

#include "HRPMapFilter16.h"

//-----------------------------------------------------------------------------
//  Custom Map8 Filter
//-----------------------------------------------------------------------------
BEGIN_IMAGEPP_NAMESPACE
class HRPCustomMap16Filter : public HRPMapFilter16
    {
    HDECLARE_CLASS_ID(HRPFilterId_CustomMap16, HRPMapFilter16)
    

public:

    // Primary methods

    HRPCustomMap16Filter();
    HRPCustomMap16Filter(Byte pi_channels);
    HRPCustomMap16Filter(const HFCPtr<HRPPixelType>& pi_pFilterPixelType);
    virtual         ~HRPCustomMap16Filter();

    virtual HRPFilter* Clone() const override;

protected:
    HRPCustomMap16Filter(const HRPCustomMap16Filter& pi_rObj);
private:
    // Disabled
    HRPCustomMap16Filter& operator=(const HRPCustomMap16Filter& pi_rObj);
    };


//-----------------------------------------------------------------------------
//  Color Balance Filter
//-----------------------------------------------------------------------------
class HRPColorBalanceFilter16 : public HRPMapFilter16
    {
    HDECLARE_CLASS_ID(HRPFilterId_ColorBalance16, HRPMapFilter16)
    

public:
    // Primary methods
    IMAGEPP_EXPORT                HRPColorBalanceFilter16();
    IMAGEPP_EXPORT                HRPColorBalanceFilter16( int32_t pi_RedVar,
                                                   int32_t pi_GreenVar,
                                                   int32_t pi_BlueVar);
    IMAGEPP_EXPORT                HRPColorBalanceFilter16( int32_t pi_GlobalVar);
    IMAGEPP_EXPORT                HRPColorBalanceFilter16(int32_t pi_GlobalVar, const HFCPtr<HRPPixelType>& pi_pFilterPixelType);

    IMAGEPP_EXPORT virtual         ~HRPColorBalanceFilter16();

    virtual HRPFilter* Clone() const override;


    int32_t        GetRedVariation() const;
    int32_t        GetGreenVariation() const;
    int32_t        GetBlueVariation() const;

protected:
    HRPColorBalanceFilter16(const HRPColorBalanceFilter16& pi_rObj);
private:
    // Disabled
    HRPColorBalanceFilter16&
    operator=(const HRPColorBalanceFilter16& pi_rObj);

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
class HRPContrastFilter16 : public HRPMapFilter16
    {
    HDECLARE_CLASS_ID(HRPFilterId_Contrast16, HRPMapFilter16)
    

public:
    IMAGEPP_EXPORT                HRPContrastFilter16();
    IMAGEPP_EXPORT                HRPContrastFilter16(short pi_Var);
    IMAGEPP_EXPORT                HRPContrastFilter16(short pi_Var,const HFCPtr<HRPPixelType>& pi_pFilterPixelType);
    IMAGEPP_EXPORT virtual         ~HRPContrastFilter16();

    virtual HRPFilter* Clone() const override;

    short GetIntensity() const;

protected:
    void            Init(short pi_Var);

    HRPContrastFilter16(const HRPContrastFilter16& pi_rObj);

private:

    // Disabled
    HRPContrastFilter16&
    operator=(const HRPContrastFilter16& pi_rObj);

    int32_t        m_Intensity;
    };

//-----------------------------------------------------------------------------
//  HRPHistogramScalingFilter
//-----------------------------------------------------------------------------
class HRPHistogramScalingFilter16 : public HRPMapFilter16
    {
    HDECLARE_CLASS_ID(HRPFilterId_HistogramScaling16, HRPMapFilter16)
    

public:
    enum HistogramScalingMode
        {
        INPUT_RANGE_CLIPPING,
        OUTPUT_RANGE_COMPRESSION
        };

    // Primary methods
    HRPHistogramScalingFilter16();
    HRPHistogramScalingFilter16(const HFCPtr<HRPPixelType>&   pi_pFilterPixelType,
                                HistogramScalingMode          pi_ScalingMode);
    virtual         ~HRPHistogramScalingFilter16();

    virtual HRPFilter* Clone() const override;

    // Get/Set methods
    void            SetInterval(Byte      pi_ChannelIndex,
                                unsigned short pi_MinValue,
                                unsigned short pi_MaxValue);

    void            GetInterval(Byte      pi_ChannelIndex,
                                unsigned short*    po_pMinValue,
                                unsigned short*    po_pMaxValue);

    void                    SetScalingMode(HistogramScalingMode pi_ScalingMode);
    HistogramScalingMode    GetScalingMode() const;

protected:
    HRPHistogramScalingFilter16(const HRPHistogramScalingFilter16& pi_rObj);

private:

    // members
    HistogramScalingMode    m_ScalingMode;

    // disable methods
    HRPHistogramScalingFilter16&
    operator=(const HRPHistogramScalingFilter16& pi_rObj);
    };

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

class HRPGammaFilter16 : public HRPMapFilter16
    {
    HDECLARE_CLASS_ID(HRPFilterId_Gamma16, HRPMapFilter16)
    

public:

    HRPGammaFilter16();
    HRPGammaFilter16(double pi_Gamma);
    virtual ~HRPGammaFilter16();

    virtual HRPFilter* Clone() const override;

    double  GetGamma() const;

protected:
    HRPGammaFilter16(const HRPGammaFilter16& pi_rObj);

private:
    double m_gamma;
    // Disabled methods
    HRPGammaFilter16& operator=(const HRPGammaFilter16& pi_rObj);
    };

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

class HRPInvertFilter16 : public HRPMapFilter16
    {
    HDECLARE_CLASS_ID(HRPFilterId_Invert16, HRPMapFilter16)
    

public:

    IMAGEPP_EXPORT          HRPInvertFilter16();
    IMAGEPP_EXPORT virtual ~HRPInvertFilter16();

    virtual HRPFilter* Clone() const override;

protected:
    HRPInvertFilter16(const HRPInvertFilter16& pi_rObj);

private:
    // Disabled methods
    HRPInvertFilter16& operator =(const HRPInvertFilter16& pi_rObj);
    };

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

class HRPTintFilter16 : public HRPMapFilter16
    {
    HDECLARE_CLASS_ID(HRPFilterId_Tint16, HRPMapFilter16)
    

public:

    IMAGEPP_EXPORT          HRPTintFilter16();
    IMAGEPP_EXPORT          HRPTintFilter16(unsigned short pi_TintColor[3]);
    IMAGEPP_EXPORT virtual ~HRPTintFilter16();

    virtual HRPFilter* Clone() const override;

protected:
    HRPTintFilter16(const HRPTintFilter16& pi_rObj);

private:
    // Disabled methods
    HRPTintFilter16& operator =(const HRPTintFilter16& pi_rObj);
    };
END_IMAGEPP_NAMESPACE


