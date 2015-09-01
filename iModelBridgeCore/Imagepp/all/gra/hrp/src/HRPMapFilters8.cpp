//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrp/src/HRPMapFilters8.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRPMapFilters8
//-----------------------------------------------------------------------------
// Map filters
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HRPMapFilters8.h>

//-----------------------------------------------------------------------------
//  Custom Map8  Filter
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPCustomMap8Filter::HRPCustomMap8Filter()
    :HRPMapFilter8()
    {
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPCustomMap8Filter::HRPCustomMap8Filter(Byte pi_channels)
    :HRPMapFilter8(pi_channels)
    {
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPCustomMap8Filter::HRPCustomMap8Filter(const HFCPtr<HRPPixelType>& pi_pFilterPixelType)
    :HRPMapFilter8(pi_pFilterPixelType)
    {
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPCustomMap8Filter::HRPCustomMap8Filter(const HRPCustomMap8Filter& pi_rObj)
    :HRPMapFilter8(pi_rObj)
    {
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
HRPFilter* HRPCustomMap8Filter::Clone() const
    {
    return new HRPCustomMap8Filter(*this);
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPCustomMap8Filter::~HRPCustomMap8Filter()
    {
    }

//-----------------------------------------------------------------------------
//  Color Balance Filter
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPColorBalanceFilter::HRPColorBalanceFilter()
    :HRPMapFilter8()
    {
    m_RedVar = 0;
    m_GreenVar = 0;
    m_BlueVar = 0;
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPColorBalanceFilter::HRPColorBalanceFilter(   int32_t pi_RedVar,
                                                int32_t pi_GreenVar,
                                                int32_t pi_BlueVar)
    :HRPMapFilter8()
    {
    m_RedVar = pi_RedVar;
    m_GreenVar = pi_GreenVar;
    m_BlueVar = pi_BlueVar;

    InitializeMap(pi_RedVar, pi_GreenVar, pi_BlueVar);
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPColorBalanceFilter::HRPColorBalanceFilter(int32_t pi_GlobalVar)
    :HRPMapFilter8()
    {
    m_RedVar = pi_GlobalVar;
    m_GreenVar = pi_GlobalVar;
    m_BlueVar = pi_GlobalVar;

    InitializeMap(pi_GlobalVar, pi_GlobalVar, pi_GlobalVar);
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPColorBalanceFilter::HRPColorBalanceFilter(int32_t pi_GlobalVar, const HFCPtr<HRPPixelType>& pi_pFilterPixelType)
    :HRPMapFilter8(pi_pFilterPixelType)
    {
    m_RedVar = pi_GlobalVar;
    m_GreenVar = pi_GlobalVar;
    m_BlueVar = pi_GlobalVar;

    InitializeMap(pi_GlobalVar, pi_GlobalVar, pi_GlobalVar);

    size_t AlphaIndex;
    if ((AlphaIndex = pi_pFilterPixelType->GetChannelOrg().GetChannelIndex(HRPChannelType::ALPHA, 0)) != HRPChannelType::FREE)
        {
        Byte map[256];

        //Don't apply color balance on alpha channel if present
        for(unsigned index = 0; index < 256; ++ index)
            {
            map[index] = (Byte)index;
            }

        SetMap(AlphaIndex, map);
        }

    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPColorBalanceFilter::HRPColorBalanceFilter(const HRPColorBalanceFilter& pi_rObj)
    :HRPMapFilter8(pi_rObj)
    {
    m_RedVar = pi_rObj.m_RedVar;
    m_GreenVar = pi_rObj.m_GreenVar;
    m_BlueVar = pi_rObj.m_BlueVar;
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPFilter* HRPColorBalanceFilter::Clone() const
    {
    return new HRPColorBalanceFilter(*this);
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPColorBalanceFilter::~HRPColorBalanceFilter()
    {
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

void HRPColorBalanceFilter::InitializeMap( int32_t pi_RedVar,
                                           int32_t pi_GreenVar,
                                           int32_t pi_BlueVar)
    {
    Byte Map[256];
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

        for(unsigned short ByteIndex = 0; ByteIndex < 256; ByteIndex++)
            {
            if((ByteIndex + Var) < 0)
                Map[ByteIndex] = 0;
            else if((ByteIndex + Var) > 255)
                Map[ByteIndex] = 255;
            else
                Map[ByteIndex] = (Byte)(ByteIndex + Var);
            }

        SetMap(ChannelIndex, Map);
        }
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

int32_t HRPColorBalanceFilter::GetRedVariation() const
    {
    return m_RedVar;
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

int32_t HRPColorBalanceFilter::GetGreenVariation() const
    {
    return m_GreenVar;
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

int32_t HRPColorBalanceFilter::GetBlueVariation() const
    {
    return m_BlueVar;
    }

//-----------------------------------------------------------------------------
//  Contrast Filter
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPContrastFilter::HRPContrastFilter()
    :HRPMapFilter8()
    {
    m_Intensity = 0;
    }


//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPContrastFilter::HRPContrastFilter(int8_t pi_Var)
    :HRPMapFilter8()
    {
    Init(pi_Var);
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPContrastFilter::HRPContrastFilter(int8_t pi_Var,const HFCPtr<HRPPixelType>& pi_pFilterPixelType)
    :HRPMapFilter8(pi_pFilterPixelType)
    {
    Init(pi_Var);

    size_t AlphaIndex;
    if ((AlphaIndex = pi_pFilterPixelType->GetChannelOrg().GetChannelIndex(HRPChannelType::ALPHA, 0)) != HRPChannelType::FREE)
        {
        Byte map[256];

        //Don't apply color balance on alpha channel if present
        for(unsigned index = 0; index < 256; ++ index)
            {
            map[index] = (Byte)index;
            }

        SetMap(AlphaIndex, map);
        }
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPContrastFilter::HRPContrastFilter(const HRPContrastFilter& pi_rObj)
    :HRPMapFilter8(pi_rObj)
    {
    m_Intensity = pi_rObj.m_Intensity;
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPFilter* HRPContrastFilter::Clone() const
    {
    return new HRPContrastFilter(*this);
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

void HRPContrastFilter::Init(int8_t pi_Var)
    {
    Byte Map[256];
    int32_t X;

    if(pi_Var > 0)
        {
        for(X = 0; X < pi_Var; X++)
            Map[X] = 0;

        for(X = pi_Var; X <= 255 - pi_Var; X++)
            Map[X] = (Byte)(255 * (X - pi_Var) / (255 - 2 * pi_Var));

        for(X = 256 - pi_Var; X <= 255; X++)
            Map[X] = 255;
        }
    else
        {
        for(X = 0; X <= 255; X++)
            Map[X] = (Byte)((255 - (-2 * pi_Var)) * X / 255 - pi_Var);
        }

    for(Byte ChannelIndex = 0; ChannelIndex < 3; ChannelIndex++)
        SetMap(ChannelIndex, Map);

    m_Intensity = pi_Var;
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPContrastFilter::~HRPContrastFilter()
    {
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

int8_t HRPContrastFilter::GetIntensity() const
    {
    return (int8_t)m_Intensity;
    }

//-----------------------------------------------------------------------------
//  HistogramScaling Filter
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// public
// Constructor.
//-----------------------------------------------------------------------------

HRPHistogramScalingFilter::HRPHistogramScalingFilter()
    : HRPMapFilter8()
    {
    m_ScalingMode = HRPHistogramScalingFilter::INPUT_RANGE_CLIPPING;
    }

//-----------------------------------------------------------------------------
// public
// Constructor.
//-----------------------------------------------------------------------------

HRPHistogramScalingFilter::HRPHistogramScalingFilter(const HFCPtr<HRPPixelType>& pi_pFilterPixelType,
                                                     HistogramScalingMode        pi_ScalingMode)
    :HRPMapFilter8(pi_pFilterPixelType)
    {
    m_ScalingMode = pi_ScalingMode;
    }

//-----------------------------------------------------------------------------
// public
// Copy Constructor.
//-----------------------------------------------------------------------------

HRPHistogramScalingFilter::HRPHistogramScalingFilter(const HRPHistogramScalingFilter& pi_rObj)
    :HRPMapFilter8(pi_rObj)
    {
    m_ScalingMode = pi_rObj.m_ScalingMode;
    }

//-----------------------------------------------------------------------------
// public
// Clone.
//-----------------------------------------------------------------------------

HRPFilter* HRPHistogramScalingFilter::Clone() const
    {
    return new HRPHistogramScalingFilter(*this);
    }

//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------

HRPHistogramScalingFilter::~HRPHistogramScalingFilter()
    {
    }

//-----------------------------------------------------------------------------
// public
// GetInterval
//-----------------------------------------------------------------------------

void HRPHistogramScalingFilter::GetInterval(unsigned short pi_ChannelIndex,
                                            unsigned short*    po_pMinValue,
                                            unsigned short*    po_pMaxValue)
    {
    HPRECONDITION(po_pMinValue != 0);
    HPRECONDITION(po_pMaxValue != 0);

    Byte* pMap = GetMap(pi_ChannelIndex);

    if(m_ScalingMode == HRPHistogramScalingFilter::INPUT_RANGE_CLIPPING)
        {
        unsigned short minvalue = 0;
        while(pMap[minvalue] == 0 && minvalue != 255)
            ++minvalue;

        unsigned short maxvalue = 255;
        while(pMap[maxvalue] == 255 && maxvalue != 0)
            --maxvalue;

        *po_pMinValue = MAX(0, minvalue-1);
        *po_pMaxValue = MIN(255, maxvalue+1);
        }
    else
        {        
        *po_pMinValue = pMap[0];
        *po_pMaxValue = pMap[255];
        }
    }

//-----------------------------------------------------------------------------
// public
// SetMap
//-----------------------------------------------------------------------------

void HRPHistogramScalingFilter::SetInterval(unsigned short pi_ChannelIndex,
                                            unsigned short pi_MinValue,
                                            unsigned short pi_MaxValue)
    {
    HPRECONDITION(pi_MinValue <= pi_MaxValue);
    HPRECONDITION(pi_MinValue < 256);
    HPRECONDITION(pi_MaxValue < 256);

    // build map
    Byte Map[256];
    if (m_ScalingMode == HRPHistogramScalingFilter::INPUT_RANGE_CLIPPING)
        {
        unsigned short i;
        for (i = 0; i < pi_MinValue; i++)
            Map[i] = 0;

        for (i = pi_MinValue; i < pi_MaxValue; i++)
            Map[i] = (Byte)(((double)(i - pi_MinValue) / (double)(pi_MaxValue - pi_MinValue)) * 255.0);

        for (i = pi_MaxValue; i < 256; i++)
            Map[i] = 255;
        }
    else
        {
        double Step = (double)(pi_MaxValue - pi_MinValue) / 255.0;
        for (unsigned short i = 0; i < 256; i++)
            {
            Map[i] = (Byte)((double)pi_MinValue + (i*Step));
            }
        }
    SetMap(pi_ChannelIndex, Map);
    }

//-----------------------------------------------------------------------------
// public
// SetScalingMode
//-----------------------------------------------------------------------------

void HRPHistogramScalingFilter::SetScalingMode(HistogramScalingMode pi_ScalingMode)
    {
    m_ScalingMode = pi_ScalingMode;
    }

//-----------------------------------------------------------------------------
// public
// GetScalingMode
//-----------------------------------------------------------------------------

HRPHistogramScalingFilter::HistogramScalingMode HRPHistogramScalingFilter::GetScalingMode() const
    {
    return m_ScalingMode;
    }

//-----------------------------------------------------------------------------
//  Gamma Filter
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPGammaFilter::HRPGammaFilter()
    {
    // Do nothing. As if gamma == 1.
    m_gamma = 1.0;
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPGammaFilter::HRPGammaFilter(double pi_Gamma)
    :m_gamma(pi_Gamma)
    {
    HASSERT(pi_Gamma != 0);

    if(pi_Gamma != 1.0)
        {
        Byte Map[256];
        double Power = 1.0 / pi_Gamma;
        for (int Index = 0; Index < 256; ++Index)
            {
            double Intensity = static_cast<double>(Index) / 255.0;
            Map[Index] = (Byte)((255.0 * pow(Intensity, Power) + .499999));
            }

        for(int Channel = 0; Channel < 3; ++Channel)
            SetMap(Channel, Map);
        }
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPGammaFilter::HRPGammaFilter(const HRPGammaFilter& pi_rObj)
    :HRPMapFilter8(pi_rObj),
    m_gamma(pi_rObj.m_gamma)
    {
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPFilter* HRPGammaFilter::Clone() const
    {
    return new HRPGammaFilter(*this);
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPGammaFilter::~HRPGammaFilter()
    {
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
double HRPGammaFilter::GetGamma() const {return m_gamma;}

//-----------------------------------------------------------------------------
//  Invert Filter
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPInvertFilter::HRPInvertFilter()
    {
    Byte Map[256];

    for(unsigned Channel = 0; Channel < 3; ++Channel)
        {
        for(unsigned short Index = 0; Index < 256; ++Index)
            {
            Map[Index] = (Byte)(255 - Index);
            }
        SetMap(Channel, Map);
        }
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPInvertFilter::HRPInvertFilter(const HRPInvertFilter& pi_rObj)
    :HRPMapFilter8(pi_rObj)
    {
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPFilter* HRPInvertFilter::Clone() const
    {
    return new HRPInvertFilter(*this);
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPInvertFilter::~HRPInvertFilter()
    {
    }

//-----------------------------------------------------------------------------
//  Tint Filter
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPTintFilter::HRPTintFilter()
    {
    // Do nothing (equivalent to tintColor of 255, 255,255).
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPTintFilter::HRPTintFilter(Byte pi_TintColor[3])
    {
    for(unsigned Channel = 0; Channel < 3; ++Channel)
        {
        Byte Map[256];

        for(unsigned short Index = 0; Index < 256; ++Index)
            Map[Index] = (Byte)((Index * pi_TintColor[Channel]) / 255U);

        SetMap((Byte)Channel, Map);
        }
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPTintFilter::HRPTintFilter(const HRPTintFilter& pi_rObj)
    :HRPMapFilter8(pi_rObj)
    {
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPFilter* HRPTintFilter::Clone() const
    {
    return new HRPTintFilter(*this);
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPTintFilter::~HRPTintFilter()
    {
    }
