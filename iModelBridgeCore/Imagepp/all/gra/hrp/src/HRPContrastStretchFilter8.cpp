//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrp/src/HRPContrastStretchFilter8.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for class HRPContrastStretchFilter8
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HRPContrastStretchFilter8.h>
#include <Imagepp/all/h/HRPPixelTypeV24R8G8B8.h>

//-----------------------------------------------------------------------------
//  Custom Map8  Filter
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// public
// Constructor.
//-----------------------------------------------------------------------------

HRPContrastStretchFilter8::HRPContrastStretchFilter8()
    :HRPMapFilter8(new HRPPixelTypeV24R8G8B8())
    {
    m_Channels = 3;

    m_pMinValue         = new int32_t [m_Channels];
    m_pMaxValue         = new int32_t [m_Channels];
    m_pMinContrastValue = new int32_t [m_Channels];
    m_pMaxContrastValue = new int32_t [m_Channels];
    m_pGammaFactor      = new double[m_Channels];

    for (int32_t ChannelIndex = 0; ChannelIndex < m_Channels; ChannelIndex++)
        {
        m_pMinValue        [ChannelIndex] = 0;
        m_pMaxValue        [ChannelIndex] = 255;

        m_pMinContrastValue[ChannelIndex] = 0;
        m_pMaxContrastValue[ChannelIndex] = 255;

        m_pGammaFactor     [ChannelIndex] = 1.0;
        }
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPContrastStretchFilter8::HRPContrastStretchFilter8(const HRPContrastStretchFilter8& pi_rFilter)
    :HRPMapFilter8(pi_rFilter)
    {
    DeepCopy(pi_rFilter);
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPContrastStretchFilter8::HRPContrastStretchFilter8(const HFCPtr<HRPPixelType>& pi_pFilterPixelType)
    : HRPMapFilter8(pi_pFilterPixelType)
    {
    HPRECONDITION(pi_pFilterPixelType->CountIndexBits() == 0);
    HPRECONDITION((pi_pFilterPixelType->CountPixelRawDataBits() % 8) == 0);

    m_Channels = (Byte)(pi_pFilterPixelType->CountPixelRawDataBits() / 8);

    m_pMinValue         = new int32_t [m_Channels];
    m_pMaxValue         = new int32_t [m_Channels];
    m_pMinContrastValue = new int32_t [m_Channels];
    m_pMaxContrastValue = new int32_t [m_Channels];
    m_pGammaFactor      = new double[m_Channels];

    for (int32_t ChannelIndex = 0; ChannelIndex < m_Channels; ChannelIndex++)
        {
        m_pMinValue        [ChannelIndex] = 0;
        m_pMaxValue        [ChannelIndex] = 255;

        m_pMinContrastValue[ChannelIndex] = 0;
        m_pMaxContrastValue[ChannelIndex] = 255;

        m_pGammaFactor     [ChannelIndex] = 1.0;
        }
    }

//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------

HRPContrastStretchFilter8::~HRPContrastStretchFilter8()
    {
    delete []m_pMinValue;
    delete []m_pMaxValue;
    delete []m_pMinContrastValue;
    delete []m_pMaxContrastValue;
    delete []m_pGammaFactor;
    }

//-----------------------------------------------------------------------------
// public
// GetInterval
//-----------------------------------------------------------------------------

void HRPContrastStretchFilter8::GetInterval(int32_t pi_ChannelIndex,
                                            int32_t* po_pMinValue,
                                            int32_t* po_pMaxValue) const
    {
    HPRECONDITION(pi_ChannelIndex < m_Channels);
    HPRECONDITION(po_pMinValue != 0);
    HPRECONDITION(po_pMaxValue != 0);

    *po_pMinValue = m_pMinValue[pi_ChannelIndex];
    *po_pMaxValue = m_pMaxValue[pi_ChannelIndex];
    }

//-----------------------------------------------------------------------------
// public
// SetMap
//-----------------------------------------------------------------------------

void HRPContrastStretchFilter8::SetInterval(int32_t pi_ChannelIndex,
                                            int32_t pi_MinValue,
                                            int32_t pi_MaxValue,
                                            bool pi_MapUpdate)
    {
    HPRECONDITION(pi_ChannelIndex < m_Channels);
    HPRECONDITION(pi_MinValue <= pi_MaxValue);
    HPRECONDITION(pi_MinValue < 256 && pi_MinValue >= 0);
    HPRECONDITION(pi_MaxValue < 256 && pi_MaxValue >= 0);

    m_pMinValue[pi_ChannelIndex] = pi_MinValue;
    m_pMaxValue[pi_ChannelIndex] = pi_MaxValue;

    if (pi_MapUpdate)
        BuildMap(pi_ChannelIndex);
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

void HRPContrastStretchFilter8::SetContrastInterval(int32_t pi_ChannelIndex,
                                                    int32_t pi_MinContrastValue,
                                                    int32_t pi_MaxContrastValue,
                                                    bool pi_MapUpdate)
    {
    HPRECONDITION(pi_ChannelIndex < m_Channels);
    HPRECONDITION(pi_MinContrastValue < 256 && pi_MinContrastValue >= 0);
    HPRECONDITION(pi_MaxContrastValue < 256 && pi_MinContrastValue >= 0);

    m_pMinContrastValue[pi_ChannelIndex] = pi_MinContrastValue;
    m_pMaxContrastValue[pi_ChannelIndex] = pi_MaxContrastValue;

    if (pi_MapUpdate)
        BuildMap(pi_ChannelIndex);
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

void HRPContrastStretchFilter8::GetContrastInterval(int32_t pi_ChannelIndex,
                                                    int32_t* po_MinContrastValue,
                                                    int32_t* po_MaxContrastValue) const
    {
    HPRECONDITION(pi_ChannelIndex < m_Channels);
    HPRECONDITION(po_MinContrastValue != 0);
    HPRECONDITION(po_MaxContrastValue != 0);

    *po_MinContrastValue = m_pMinContrastValue[pi_ChannelIndex];
    *po_MaxContrastValue = m_pMaxContrastValue[pi_ChannelIndex];
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

void HRPContrastStretchFilter8::SetGammaFactor(int32_t  pi_ChannelIndex,
                                               double pi_GammaFactor,
                                               bool   pi_MapUpdate)
    {
    HPRECONDITION(pi_ChannelIndex < m_Channels);
    HPRECONDITION(pi_GammaFactor <= 100.0 && pi_GammaFactor >= 0.0);

    m_pGammaFactor[pi_ChannelIndex] = pi_GammaFactor;

    if (pi_MapUpdate)
        BuildMap(pi_ChannelIndex);
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

void HRPContrastStretchFilter8::GetGammaFactor(int32_t   pi_ChannelIndex,
                                               double* po_pGammaFactor) const
    {
    HPRECONDITION(pi_ChannelIndex < m_Channels);
    HPRECONDITION(po_pGammaFactor != 0);

    *po_pGammaFactor = m_pGammaFactor[pi_ChannelIndex];
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

void HRPContrastStretchFilter8::BuildMap(int32_t pi_ChannelIndex)
    {
    int32_t i;
    Byte* pMap = GetMap(pi_ChannelIndex);

    //----------------------------------------
    // Input clipping.
    for (i = 0; i < m_pMinValue[pi_ChannelIndex]; i++)
        pMap[i] = 0;

    for (i = m_pMinValue[pi_ChannelIndex]; i < m_pMaxValue[pi_ChannelIndex]; i++)
        pMap[i] = (Byte)(((double)(i - m_pMinValue[pi_ChannelIndex]) / (double)(m_pMaxValue[pi_ChannelIndex] - m_pMinValue[pi_ChannelIndex])) * 255.0);

    for (i = m_pMaxValue[pi_ChannelIndex]; i < 256; i++)
        pMap[i] = 255;

    //----------------------------------------
    // Process gamme adjustement if required.
    if (!HDOUBLE_EQUAL_EPSILON(m_pGammaFactor[pi_ChannelIndex], 1.0))
        {
        for (i = 0; i <= 255; i++)
            {
            pMap[i] = (unsigned char)(255.0 * pow( pMap[i] / 255.0, 1 / m_pGammaFactor[pi_ChannelIndex]));
            }
        }

    //----------------------------------------
    // Compute output clipping if any.
    if ( m_pMaxContrastValue[pi_ChannelIndex] < 255 || m_pMinContrastValue[pi_ChannelIndex] > 0)
        {
        // Take care of inverted handle.
        if ( m_pMaxContrastValue[pi_ChannelIndex] > m_pMinContrastValue[pi_ChannelIndex] )
            {
            for (i = 0; i <= 255; i++)
                pMap[i] = (Byte)(((double)(pMap[i] /255.0) * (m_pMaxContrastValue[pi_ChannelIndex] - m_pMinContrastValue[pi_ChannelIndex])) + m_pMinContrastValue[pi_ChannelIndex]);
            }
        else
            {
            for (i = 0; i <= 255; i++)
                pMap[i] = (Byte)((( (255.0 - (double)(pMap[i])) /255.0) * (m_pMinContrastValue[pi_ChannelIndex] - m_pMaxContrastValue[pi_ChannelIndex])) + m_pMaxContrastValue[pi_ChannelIndex]);
            }
        }
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

void HRPContrastStretchFilter8::DeepCopy(const HRPContrastStretchFilter8& pi_rSrc)
    {
    m_Channels = pi_rSrc.m_Channels;

    m_pMinValue         = new int32_t [m_Channels];
    m_pMaxValue         = new int32_t [m_Channels];
    m_pMinContrastValue = new int32_t [m_Channels];
    m_pMaxContrastValue = new int32_t [m_Channels];
    m_pGammaFactor      = new double[m_Channels];

    for (int32_t ChannelIndex = 0; ChannelIndex < m_Channels; ChannelIndex++)
        {
        m_pMinValue        [ChannelIndex] = pi_rSrc.m_pMinValue[ChannelIndex];
        m_pMaxValue        [ChannelIndex] = pi_rSrc.m_pMaxValue[ChannelIndex];

        m_pMinContrastValue[ChannelIndex] = pi_rSrc.m_pMinContrastValue[ChannelIndex];
        m_pMaxContrastValue[ChannelIndex] = pi_rSrc.m_pMaxContrastValue[ChannelIndex];

        m_pGammaFactor     [ChannelIndex] = pi_rSrc.m_pGammaFactor[ChannelIndex];
        }
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPFilter* HRPContrastStretchFilter8::Clone() const
    {
    return new HRPContrastStretchFilter8(*this);
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

#ifdef __HMR_DEBUG_MEMBER

void HRPContrastStretchFilter8::MapDump(int32_t pi_ChannelIndex)
    {
    Byte* pMap = GetMap(pi_ChannelIndex);

    for (int32_t i = 0; i <= 255; i++)
        {
        WChar Text[10] = {0};

        BeStringUtilities::Snwprintf(Text, L"%d\n", pMap[i]);
        HDEBUGTEXT(Text)
        }
    }

#endif
