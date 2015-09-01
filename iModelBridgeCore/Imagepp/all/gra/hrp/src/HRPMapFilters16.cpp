//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrp/src/HRPMapFilters16.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRPMapFilters16
//-----------------------------------------------------------------------------
// Map filters
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HRPMapFilters16.h>

//-----------------------------------------------------------------------------
//  Custom Map8  Filter
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPCustomMap16Filter::HRPCustomMap16Filter()
    :HRPMapFilter16()
    {
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPCustomMap16Filter::HRPCustomMap16Filter(Byte pi_channels)
    :HRPMapFilter16(pi_channels)
    {
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPCustomMap16Filter::HRPCustomMap16Filter(const HFCPtr<HRPPixelType>& pi_pFilterPixelType)
    :HRPMapFilter16(pi_pFilterPixelType)
    {
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPCustomMap16Filter::HRPCustomMap16Filter(const HRPCustomMap16Filter& pi_rObj)
    :HRPMapFilter16(pi_rObj)
    {
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPFilter* HRPCustomMap16Filter::Clone() const
    {
    return new HRPCustomMap16Filter(*this);
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPCustomMap16Filter::~HRPCustomMap16Filter()
    {
    }

//-----------------------------------------------------------------------------
//  Color Balance Filter
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPColorBalanceFilter16::HRPColorBalanceFilter16()
    :HRPMapFilter16()
    {
    m_RedVar   = 0;
    m_GreenVar = 0;
    m_BlueVar  = 0;
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPColorBalanceFilter16::HRPColorBalanceFilter16( int32_t pi_RedVar,
                                                  int32_t pi_GreenVar,
                                                  int32_t pi_BlueVar)
    :HRPMapFilter16()
    {
    m_RedVar   = pi_RedVar;
    m_GreenVar = pi_GreenVar;
    m_BlueVar  = pi_BlueVar;

    InitializeMap(pi_RedVar, pi_GreenVar, pi_BlueVar);
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPColorBalanceFilter16::HRPColorBalanceFilter16(int32_t pi_GlobalVar)
    :HRPMapFilter16()
    {
    m_RedVar   = pi_GlobalVar;
    m_GreenVar = pi_GlobalVar;
    m_BlueVar  = pi_GlobalVar;

    InitializeMap(pi_GlobalVar, pi_GlobalVar, pi_GlobalVar);
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPColorBalanceFilter16::HRPColorBalanceFilter16(int32_t pi_GlobalVar, const HFCPtr<HRPPixelType>& pi_pFilterPixelType)
    :HRPMapFilter16(pi_pFilterPixelType)
    {
    m_RedVar   = pi_GlobalVar;
    m_GreenVar = pi_GlobalVar;
    m_BlueVar  = pi_GlobalVar;

    InitializeMap(pi_GlobalVar, pi_GlobalVar, pi_GlobalVar);

    int AlphaIndex;
    if ((AlphaIndex = pi_pFilterPixelType->GetChannelOrg().GetChannelIndex(HRPChannelType::ALPHA, 0)) != HRPChannelType::FREE)
        {
        unsigned short map[65536];

        //Don't apply color balance on alpha channel if present
        for(int index = 0; index < 65536; index++)
            {
            map[index] = (unsigned short)index;
            }
        SetMap((Byte)AlphaIndex, map);
        }
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPColorBalanceFilter16::HRPColorBalanceFilter16(const HRPColorBalanceFilter16& pi_rObj)
    :HRPMapFilter16(pi_rObj)
    {
    m_RedVar   = pi_rObj.m_RedVar;
    m_GreenVar = pi_rObj.m_GreenVar;
    m_BlueVar  = pi_rObj.m_BlueVar;
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPFilter* HRPColorBalanceFilter16::Clone() const
    {
    return new HRPColorBalanceFilter16(*this);
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPColorBalanceFilter16::~HRPColorBalanceFilter16()
    {
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

void HRPColorBalanceFilter16::InitializeMap(int32_t pi_RedVar,
                                            int32_t pi_GreenVar,
                                            int32_t pi_BlueVar)
    {
    unsigned short Map[65536];
    int32_t Var=0;

    for(Byte ChannelIndex = 0; ChannelIndex < 3; ChannelIndex++)
        {
        switch(ChannelIndex)
            {
            case 0:
                Var = pi_RedVar;
                break;
            case 1:
                Var = pi_GreenVar;
                break;
            case 2:
                Var = pi_BlueVar;
                break;
            }

        for(int SampleIndex = 0; SampleIndex < 65536; SampleIndex++)
            {
            if((SampleIndex + Var) < 0)
                Map[SampleIndex] = 0;
            else if((SampleIndex + Var) > 65535)
                Map[SampleIndex] = 65535;
            else
                Map[SampleIndex] = (unsigned short)(SampleIndex + Var);
            }
        SetMap(ChannelIndex, Map);
        }
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

int32_t HRPColorBalanceFilter16::GetRedVariation() const
    {
    return m_RedVar;
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

int32_t HRPColorBalanceFilter16::GetGreenVariation() const
    {
    return m_GreenVar;
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

int32_t HRPColorBalanceFilter16::GetBlueVariation() const
    {
    return m_BlueVar;
    }

//-----------------------------------------------------------------------------
//  Contrast Filter
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPContrastFilter16::HRPContrastFilter16()
    :HRPMapFilter16()
    {
    m_Intensity = 0;
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPContrastFilter16::HRPContrastFilter16(short pi_Var)
    :HRPMapFilter16()
    {
    Init(pi_Var);
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPContrastFilter16::HRPContrastFilter16(short pi_Var,const HFCPtr<HRPPixelType>& pi_pFilterPixelType)
    :HRPMapFilter16(pi_pFilterPixelType)
    {
    Init(pi_Var);

    uint32_t AlphaIndex;
    if ((AlphaIndex = pi_pFilterPixelType->GetChannelOrg().GetChannelIndex(HRPChannelType::ALPHA, 0)) != HRPChannelType::FREE)
        {
        unsigned short map[65536];

        //Don't apply color balance on alpha channel if present
        for(int index = 0; index < 65536; index++)
            {
            map[index] = (unsigned short)index;
            }
        SetMap((Byte)AlphaIndex, map);
        }
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPContrastFilter16::HRPContrastFilter16(const HRPContrastFilter16& pi_rObj)
    :HRPMapFilter16(pi_rObj)
    {
    m_Intensity = pi_rObj.m_Intensity;
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPFilter* HRPContrastFilter16::Clone() const
    {
    return new HRPContrastFilter16(*this);
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

void HRPContrastFilter16::Init(short pi_Var)
    {
    unsigned short Map[65536];
    int32_t X;

    if(pi_Var > 0)
        {
        for(X = 0; X < pi_Var; X++)
            Map[X] = 0;

        for(X = pi_Var; X <= 65535 - pi_Var; X++)
            Map[X] = (unsigned short)(65535.0 * ((X - pi_Var) / (65535 - 2.0 * pi_Var)));

        for(X = 65536 - pi_Var; X <= 65535; X++)
            Map[X] = 65535;
        }
    else
        {
        for(X = 0; X <= 65535; X++)
            Map[X] = (unsigned short)((65535 - (-2 * pi_Var)) * X / 65535 - pi_Var);
        }

    for(Byte ChannelIndex = 0; ChannelIndex < 3; ChannelIndex++)
        SetMap(ChannelIndex, Map);

    m_Intensity = pi_Var;
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPContrastFilter16::~HRPContrastFilter16()
    {
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

short HRPContrastFilter16::GetIntensity() const
    {
    return (short)m_Intensity;
    }

//-----------------------------------------------------------------------------
//  HistogramScaling Filter
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// public
// Constructor.
//-----------------------------------------------------------------------------

HRPHistogramScalingFilter16::HRPHistogramScalingFilter16()
    :HRPMapFilter16()
    {
    m_ScalingMode = HRPHistogramScalingFilter16::INPUT_RANGE_CLIPPING;
    }

//-----------------------------------------------------------------------------
// public
// Constructor.
//-----------------------------------------------------------------------------

HRPHistogramScalingFilter16::HRPHistogramScalingFilter16(const HFCPtr<HRPPixelType>& pi_pFilterPixelType,
                                                         HistogramScalingMode        pi_ScalingMode)
    :HRPMapFilter16(pi_pFilterPixelType)
    {
    m_ScalingMode = pi_ScalingMode;
    }

//-----------------------------------------------------------------------------
// public
// Copy Constructor.
//-----------------------------------------------------------------------------

HRPHistogramScalingFilter16::HRPHistogramScalingFilter16(const HRPHistogramScalingFilter16& pi_rObj)
    :HRPMapFilter16(pi_rObj)
    {
    m_ScalingMode = pi_rObj.m_ScalingMode;
    }

//-----------------------------------------------------------------------------
// public
// Clone.
//-----------------------------------------------------------------------------

HRPFilter* HRPHistogramScalingFilter16::Clone() const
    {
    return new HRPHistogramScalingFilter16(*this);
    }

//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------

HRPHistogramScalingFilter16::~HRPHistogramScalingFilter16()
    {
    }

//-----------------------------------------------------------------------------
// public
// GetInterval
//-----------------------------------------------------------------------------

void HRPHistogramScalingFilter16::GetInterval(Byte   pi_ChannelIndex,
                                              unsigned short* po_pMinValue,
                                              unsigned short* po_pMaxValue)
    {
    HPRECONDITION(po_pMinValue != 0);
    HPRECONDITION(po_pMaxValue != 0);

    unsigned short* pMap = GetMap(pi_ChannelIndex);
    *po_pMinValue = pMap[0];
    *po_pMaxValue = pMap[255];
    }

//-----------------------------------------------------------------------------
// public
// SetMap
//-----------------------------------------------------------------------------

void HRPHistogramScalingFilter16::SetInterval(Byte  pi_ChannelIndex,
                                              unsigned short pi_MinValue,
                                              unsigned short pi_MaxValue)
    {
    HPRECONDITION(pi_MinValue <= pi_MaxValue);
    HPRECONDITION(pi_MinValue < 65536);
    HPRECONDITION(pi_MaxValue < 65536);

    // Build the map
    unsigned short Map[65536];
    if (m_ScalingMode == HRPHistogramScalingFilter16::INPUT_RANGE_CLIPPING)
        {
        int i;
        for (i = 0; i < pi_MinValue; i++)
            Map[i] = 0;

        for (i = pi_MinValue; i < pi_MaxValue; i++)
            Map[i] = (unsigned short)(((double)(i - pi_MinValue) / (double)(pi_MaxValue - pi_MinValue)) * 65535.0);

        for (i = pi_MaxValue; i < 65536; i++)
            Map[i] = 65535;
        }
    else
        {
        double Step = (double)(pi_MaxValue - pi_MinValue) / 65536.0;
        double Factor = 0;
        for (int i = 0; i < 65536; i++)
            {
            Map[i] = (unsigned short)((double)pi_MinValue + Factor);
            Factor += Step;
            }
        }

    SetMap(pi_ChannelIndex, Map);
    }

//-----------------------------------------------------------------------------
// public
// SetScalingMode
//-----------------------------------------------------------------------------

void HRPHistogramScalingFilter16::SetScalingMode(HistogramScalingMode pi_ScalingMode)
    {
    m_ScalingMode = pi_ScalingMode;
    }

//-----------------------------------------------------------------------------
// public
// GetScalingMode
//-----------------------------------------------------------------------------

HRPHistogramScalingFilter16::HistogramScalingMode HRPHistogramScalingFilter16::GetScalingMode() const
    {
    return m_ScalingMode;
    }


//-----------------------------------------------------------------------------
//  Gamma Filter
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------

HRPGammaFilter16::HRPGammaFilter16()
    {
    // Do nothing. As if gamma == 1.
    m_gamma = 1.0;
    }

//-----------------------------------------------------------------------------
HRPGammaFilter16::HRPGammaFilter16(double pi_Gamma)
    {
    HPRECONDITION( HDOUBLE_GREATER_EPSILON(pi_Gamma, 0.0) );

    // If gamma == 1, there is nothing to do.
    if(pi_Gamma != 1.0)
        {
        unsigned short Map[65536];
        double Power = 1.0 / pi_Gamma;
        for (int Index = 0; Index < 65536; ++Index)
            {
            HASSERT(Index < GetMapSize());

            double Intensity = static_cast<double>(Index) / 65535.0;
            Map[Index] = (unsigned short)((65535.0 * pow(Intensity, Power) + .499999));
            }

        for(Byte Channel = 0; Channel < 3; ++Channel)
            SetMap(Channel, Map);
        }

    m_gamma = pi_Gamma;
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPGammaFilter16::HRPGammaFilter16(const HRPGammaFilter16& pi_rObj)
    :HRPMapFilter16(pi_rObj)
    {
    m_gamma = pi_rObj.m_gamma;
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
double HRPGammaFilter16::GetGamma() const {return m_gamma;}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPFilter* HRPGammaFilter16::Clone() const
    {
    return new HRPGammaFilter16(*this);
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPGammaFilter16::~HRPGammaFilter16()
    {
    }

//-----------------------------------------------------------------------------
//  Invert Filter
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPInvertFilter16::HRPInvertFilter16()
    {
    unsigned short Map[65536];

    for(Byte Channel = 0; Channel < 3; ++Channel)
        {
        for(int Index = 0; Index < 65536; ++Index)
            {
            Map[Index] = (unsigned short)(65535 - Index);
            }
        SetMap(Channel, Map);
        }
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPInvertFilter16::HRPInvertFilter16(const HRPInvertFilter16& pi_rObj)
    :HRPMapFilter16(pi_rObj)
    {
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPFilter* HRPInvertFilter16::Clone() const
    {
    return new HRPInvertFilter16(*this);
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPInvertFilter16::~HRPInvertFilter16()
    {
    }

//-----------------------------------------------------------------------------
//  Tint Filter
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPTintFilter16::HRPTintFilter16()
    {
    // Do nothing (equivalent to tintColor of (65535, 65535, 65535).
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPTintFilter16::HRPTintFilter16(unsigned short pi_TintColor[3])
    {
    for(Byte Channel = 0; Channel < 3; ++Channel)
        {
        unsigned short Map[65556];

        for(int Index = 0; Index < 65556; ++Index)
            Map[Index] = (unsigned short)((Index * pi_TintColor[Channel]) / 65555U);

        SetMap(Channel, Map);
        }
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPTintFilter16::HRPTintFilter16(const HRPTintFilter16& pi_rObj)
    :HRPMapFilter16(pi_rObj)
    {
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPFilter* HRPTintFilter16::Clone() const
    {
    return new HRPTintFilter16(*this);
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPTintFilter16::~HRPTintFilter16()
    {
    }
