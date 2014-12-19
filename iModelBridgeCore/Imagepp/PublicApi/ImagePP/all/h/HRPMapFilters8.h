//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPMapFilters8.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
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
class HRPCustomMap8Filter : public HRPMapFilter8
    {
    HDECLARE_CLASS_ID(1546, HRPMapFilter8)
    

public:

    // Primary methods

    _HDLLg                 HRPCustomMap8Filter();
    _HDLLg                 HRPCustomMap8Filter(Byte pi_channels);
    _HDLLg                 HRPCustomMap8Filter(const HFCPtr<HRPPixelType>& pi_pFilterPixelType);
    _HDLLg virtual         ~HRPCustomMap8Filter();

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
    HDECLARE_CLASS_ID(1206, HRPMapFilter8)
    

public:

    // Primary methods

    _HDLLg                 HRPColorBalanceFilter();
    _HDLLg                 HRPColorBalanceFilter(  int32_t pi_RedVar,
                                                   int32_t pi_GreenVar,
                                                   int32_t pi_BlueVar);
    _HDLLg                 HRPColorBalanceFilter(  int32_t pi_GlobalVar);
    _HDLLg                 HRPColorBalanceFilter(int32_t pi_GlobalVar, const HFCPtr<HRPPixelType>& pi_pFilterPixelType);

    _HDLLg virtual         ~HRPColorBalanceFilter();

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
    HDECLARE_CLASS_ID(1205, HRPMapFilter8)
    

public:

    _HDLLg                 HRPContrastFilter();
    _HDLLg                 HRPContrastFilter(int8_t pi_Var);
    _HDLLg                 HRPContrastFilter(int8_t pi_Var,const HFCPtr<HRPPixelType>& pi_pFilterPixelType);
    _HDLLg virtual         ~HRPContrastFilter();

    virtual HRPFilter* Clone() const override;

    _HDLLg int8_t        GetIntensity() const;

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
    HDECLARE_CLASS_ID(1256, HRPMapFilter8)
    

public:
    enum HistogramScalingMode
        {
        INPUT_RANGE_CLIPPING,
        OUTPUT_RANGE_COMPRESSION
        };

    // Primary methods
    _HDLLg                 HRPHistogramScalingFilter();
    _HDLLg                 HRPHistogramScalingFilter(const HFCPtr<HRPPixelType>&   pi_pFilterPixelType,
                                                     HistogramScalingMode          pi_ScalingMode);
    _HDLLg virtual         ~HRPHistogramScalingFilter();

    virtual HRPFilter* Clone() const override;

    // Get/Set methods
    _HDLLg void            SetInterval(unsigned short pi_ChannelIndex,
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
    HDECLARE_CLASS_ID(1264, HRPMapFilter8)
    

public:

    _HDLLg HRPGammaFilter();
    _HDLLg HRPGammaFilter(double pi_Gamma);
    _HDLLg virtual ~HRPGammaFilter();

    virtual HRPFilter* Clone() const override;

protected:
    HRPGammaFilter(const HRPGammaFilter& pi_rObj);

private:
    // Disabled methods
    HRPGammaFilter& operator=(const HRPGammaFilter& pi_rObj);
    };

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

class HRPInvertFilter : public HRPMapFilter8
    {
    HDECLARE_CLASS_ID(1265, HRPMapFilter8)
    

public:

    _HDLLg HRPInvertFilter();
    _HDLLg virtual ~HRPInvertFilter();

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
    HDECLARE_CLASS_ID(1266, HRPMapFilter8)
    

public:

    _HDLLg HRPTintFilter();
    _HDLLg HRPTintFilter(Byte pi_TintColor[3]);
    _HDLLg virtual ~HRPTintFilter();

    virtual HRPFilter* Clone() const override;

protected:
    HRPTintFilter(const HRPTintFilter& pi_rObj);

private:
    // Disabled methods
    HRPTintFilter& operator =(const HRPTintFilter& pi_rObj);
    };


