//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrp/src/HRPLigthnessContrastStretch.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRPFunctionFilters
//-----------------------------------------------------------------------------
// Some common function filters.
//-----------------------------------------------------------------------------
#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HRPLigthnessContrastStretch.h>
#include <Imagepp/all/h/HRPPixelTypeV24R8G8B8.h>
#include <Imagepp/all/h/HGFLuvColorSpace.h>
#include <Imagepp/all/h/HRPPixelTypeFactory.h>

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPLigthnessContrastStretch::HRPLigthnessContrastStretch()
    :HRPFunctionFilter(new HRPPixelTypeV24R8G8B8())
    {
    m_Channels       = 3;
    m_ChannelWidth   = 8;
    m_MaxSampleValue = 255;

    m_pMinValue         = new int   [m_Channels];
    m_pMaxValue         = new int   [m_Channels];
    m_pMinContrastValue = new int   [m_Channels];
    m_pMaxContrastValue = new int   [m_Channels];
    m_pGammaFactor      = new double[m_Channels];

    for (uint32_t ChannelIndex = 0; ChannelIndex < m_Channels; ChannelIndex++)
        {
        m_pMinValue        [ChannelIndex] = 0;
        m_pMaxValue        [ChannelIndex] = 100;

        m_pMinContrastValue[ChannelIndex] = 0;
        m_pMaxContrastValue[ChannelIndex] = 100;

        m_pGammaFactor     [ChannelIndex] = 1.0;
        }
    m_pColorSpaceConverter = new HGFLuvColorSpace(DEFAULT_GAMMA_FACTOR, m_ChannelWidth);
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPLigthnessContrastStretch::HRPLigthnessContrastStretch(const HFCPtr<HRPPixelType>& pi_pFilterPixelType)
    :HRPFunctionFilter(pi_pFilterPixelType)
    {
    if (pi_pFilterPixelType->CountIndexBits()) //  GetPalette())
        {
        m_ChannelWidth   = pi_pFilterPixelType->CountIndexBits();
        m_Channels       = pi_pFilterPixelType->GetChannelOrg().CountChannels();
        }
    else
        {
        m_ChannelWidth   = pi_pFilterPixelType->GetChannelOrg().GetChannelPtr(0)->GetSize();
        m_Channels       = pi_pFilterPixelType->CountPixelRawDataBits() / m_ChannelWidth;
        }

    m_MaxSampleValue = (1 << m_ChannelWidth) - 1;

    m_pMinValue         = new int   [m_Channels];
    m_pMaxValue         = new int   [m_Channels];
    m_pMinContrastValue = new int   [m_Channels];
    m_pMaxContrastValue = new int   [m_Channels];
    m_pGammaFactor      = new double[m_Channels];

    for (uint32_t ChannelIndex = 0; ChannelIndex < m_Channels; ChannelIndex++)
        {
        m_pMinValue        [ChannelIndex] = 0;
        m_pMaxValue        [ChannelIndex] = 100;

        m_pMinContrastValue[ChannelIndex] = 0;
        m_pMaxContrastValue[ChannelIndex] = 100;

        m_pGammaFactor     [ChannelIndex] = 1.0;
        }
    m_pColorSpaceConverter = new HGFLuvColorSpace(DEFAULT_GAMMA_FACTOR, m_ChannelWidth);
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPLigthnessContrastStretch::HRPLigthnessContrastStretch(const HRPLigthnessContrastStretch& pi_rFilter)
    :HRPFunctionFilter(pi_rFilter)
    {
    DeepCopy(pi_rFilter);
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPLigthnessContrastStretch::HRPLigthnessContrastStretch(const HCLASS_ID pi_PixelTypeClassID)
    :HRPFunctionFilter(HRPPixelTypeFactory::GetInstance()->Create(pi_PixelTypeClassID))
    {
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPLigthnessContrastStretch::~HRPLigthnessContrastStretch()
    {
    delete []m_pMinValue;
    delete []m_pMaxValue;
    delete []m_pMinContrastValue;
    delete []m_pMaxContrastValue;
    delete []m_pGammaFactor;

    delete m_pColorSpaceConverter;
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPFilter* HRPLigthnessContrastStretch::Clone() const
    {
    return new HRPLigthnessContrastStretch(*this);
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

void HRPLigthnessContrastStretch::Function( const void*  pi_pSrcRawData,
                                            void*  po_pDestRawData,
                                            uint32_t pi_PixelsCount) const
    {
    HPRECONDITION(m_pColorSpaceConverter != 0);
    HPRECONDITION(pi_pSrcRawData  != 0);
    HPRECONDITION(po_pDestRawData != 0);
    HPRECONDITION(pi_PixelsCount  > 0);
    HPRECONDITION(m_Channels > 0 && m_Channels <= 4);

    if (m_ChannelWidth == 8)
        {
        FunctionN8( pi_pSrcRawData, po_pDestRawData, pi_PixelsCount);
        }
    else if (m_ChannelWidth == 16)
        {
        FunctionN16( pi_pSrcRawData, po_pDestRawData, pi_PixelsCount);
        }
    else
        {
        HASSERT(false);
        }
    }




//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

void HRPLigthnessContrastStretch::FunctionN8( const void*  pi_pSrcRawData,
                                              void*  po_pDestRawData,
                                              uint32_t pi_PixelsCount) const
    {
    HPRECONDITION(m_pColorSpaceConverter != 0);
    HPRECONDITION(pi_pSrcRawData  != 0);
    HPRECONDITION(po_pDestRawData != 0);
    HPRECONDITION(pi_PixelsCount > 0);
    HPRECONDITION(m_Channels > 0 && m_Channels <= 4);

    Byte* pSrcRawData  = (Byte*)pi_pSrcRawData;
    Byte* pDestRawData = (Byte*)po_pDestRawData;

    double L;
    double U;
    double V;

    // The code as been duplicated to avoid "if" statement on each loop
    // to improve performance.

    if(m_Channels == 1)
        {
        while(pi_PixelsCount)
            {
            //------------------------------------------
            // Convert to L*u*v
            m_pColorSpaceConverter->ConvertFromRGB (*pSrcRawData, *pSrcRawData, *pSrcRawData, &L, &U, &V);
            pSrcRawData++;

            //------------------------------------------
            // Process the LigthnessContrastStretch

            // Input clipping.
            if (L < m_pMinValue[0])
                L = 0.0;
            else if (L > m_pMaxValue[0])
                L = 100.0;
            else
                L = ((L - m_pMinValue[0]) / (double)(m_pMaxValue[0] - m_pMinValue[0])) * 100.0;

            //----------------------------------------
            // Process gamme adjustement if required.
            if (!HDOUBLE_EQUAL_EPSILON(m_pGammaFactor[0], 1.0))
                {
                L = (unsigned char)(100.0 * pow( L / 100.0, 1 / m_pGammaFactor[0]));
                }

            //----------------------------------------
            // Compute output clipping if any.
            if ( m_pMaxContrastValue[0] < 100 || m_pMinContrastValue[0] > 0)
                {
                // Take care of inverted handle.
                if ( m_pMaxContrastValue[0] > m_pMinContrastValue[0] )
                    {
                    L = ((L /100.0) * ((m_pMaxContrastValue[0] - m_pMinContrastValue[0]))) + m_pMinContrastValue[0];
                    }
                else
                    {
                    L = (((100.0 - L) /100.0) * ((m_pMinContrastValue[0] - m_pMaxContrastValue[0]))) + m_pMaxContrastValue[0];
                    }
                }

            //------------------------------------------
            // Convert back to original pixeltype (Grayscale)
            m_pColorSpaceConverter->ConvertToRGB (L, U, V, pDestRawData, pDestRawData, pDestRawData);
            pDestRawData++;

            pi_PixelsCount--;
            }
        }
    else if(m_Channels == 3)
        {
        while(pi_PixelsCount)
            {
            //------------------------------------------
            // Convert to L*u*v
            m_pColorSpaceConverter->ConvertFromRGB (*pSrcRawData, *(pSrcRawData + 1), *(pSrcRawData + 2), &L, &U, &V);

            //------------------------------------------
            // Process the LigthnessContrastStretch
            // Input clipping.
            if (L <= m_pMinValue[0])
                L = 0.0;
            else if (L >= m_pMaxValue[0])
                L = 100.0;
            else
                L = ((L - m_pMinValue[0]) / (double)(m_pMaxValue[0] - m_pMinValue[0])) * 100.0;

            HASSERT(L >= 0.0 && L <= 100.0);

            //----------------------------------------
            // Process gamme adjustement if required.
            if (!HDOUBLE_EQUAL_EPSILON(m_pGammaFactor[0], 1.0))
                {
                L = (unsigned char)(100.0 * pow( L / 100.0, 1 / m_pGammaFactor[0]));
                HASSERT(L >= 0.0 && L <= 100.0);
                }

            //----------------------------------------
            // Compute output clipping if any.
            if ( m_pMaxContrastValue[0] < 100 || m_pMinContrastValue[0] > 0)
                {
                // Take care of inverted handle.
                if ( m_pMaxContrastValue[0] > m_pMinContrastValue[0] )
                    {
                    L = ((L /100.0) * (m_pMaxContrastValue[0] - m_pMinContrastValue[0])) + m_pMinContrastValue[0];
                    }
                else
                    {
                    L = (((100.0 - L) /100.0) * (m_pMinContrastValue[0] - m_pMaxContrastValue[0])) + m_pMaxContrastValue[0];
                    }
                HASSERT(L >= 0.0 && L <= 100.0);
                }

            //------------------------------------------
            // Convert back to original pixeltype (RGB or RGBA)
            m_pColorSpaceConverter->ConvertToRGB (L, U, V, pDestRawData, pDestRawData + 1, pDestRawData + 2);

            pDestRawData += m_Channels;
            pSrcRawData += m_Channels;

            pi_PixelsCount--;
            }
        }
    else if(m_Channels == 4)
        {
        while(pi_PixelsCount)
            {
            //------------------------------------------
            // Convert to L*u*v
            m_pColorSpaceConverter->ConvertFromRGB (*pSrcRawData, *(pSrcRawData + 1), *(pSrcRawData + 2), &L, &U, &V);

            //------------------------------------------
            // Process the LigthnessContrastStretch
            // Input clipping.
            if (L <= m_pMinValue[0])
                L = 0.0;
            else if (L >= m_pMaxValue[0])
                L = 100.0;
            else
                L = ((L - m_pMinValue[0]) / (double)(m_pMaxValue[0] - m_pMinValue[0])) * 100.0;

            HASSERT(L >= 0.0 && L <= 100.0);

            //----------------------------------------
            // Process gamme adjustement if required.
            if (!HDOUBLE_EQUAL_EPSILON(m_pGammaFactor[0], 1.0))
                {
                L = (unsigned char)(100.0 * pow( L / 100.0, 1 / m_pGammaFactor[0]));
                HASSERT(L >= 0.0 && L <= 100.0);
                }

            //----------------------------------------
            // Compute output clipping if any.
            if ( m_pMaxContrastValue[0] < 100 || m_pMinContrastValue[0] > 0)
                {
                // Take care of inverted handle.
                if ( m_pMaxContrastValue[0] > m_pMinContrastValue[0] )
                    {
                    L = ((L /100.0) * (m_pMaxContrastValue[0] - m_pMinContrastValue[0])) + m_pMinContrastValue[0];
                    }
                else
                    {
                    L = (((100.0 - L) /100.0) * (m_pMinContrastValue[0] - m_pMaxContrastValue[0])) + m_pMaxContrastValue[0];
                    }
                HASSERT(L >= 0.0 && L <= 100.0);
                }

            //------------------------------------------
            // Convert back to original pixeltype (RGB or RGBA)
            m_pColorSpaceConverter->ConvertToRGB (L, U, V, pDestRawData, pDestRawData + 1, pDestRawData + 2);
            *(pDestRawData + 3) = pSrcRawData[3];

            pDestRawData += m_Channels;
            pSrcRawData += m_Channels;

            pi_PixelsCount--;
            }
        }
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

void HRPLigthnessContrastStretch::FunctionN16( const void*  pi_pSrcRawData,
                                               void*  po_pDestRawData,
                                               uint32_t pi_PixelsCount) const
    {
    HPRECONDITION(m_pColorSpaceConverter != 0);
    HPRECONDITION(pi_pSrcRawData  != 0);
    HPRECONDITION(po_pDestRawData != 0);
    HPRECONDITION(pi_PixelsCount > 0);
    HPRECONDITION(m_Channels > 0 && m_Channels <= 4);

    unsigned short* pSrcRawData  = (unsigned short*)pi_pSrcRawData;
    unsigned short* pDestRawData = (unsigned short*)po_pDestRawData;

    double L;
    double U;
    double V;

    // The code as been duplicated to avoid "if" statement on each loop
    // to improve performance.

    if(m_Channels == 1)
        {
        while(pi_PixelsCount)
            {
            //------------------------------------------
            // Convert to L*u*v
            m_pColorSpaceConverter->ConvertFromRGB (*pSrcRawData, *pSrcRawData, *pSrcRawData, &L, &U, &V);
            pSrcRawData++;

            //------------------------------------------
            // Process the LigthnessContrastStretch
            //------------------------------------------
            // Process the LigthnessContrastStretch

            // Input clipping.
            if (L < m_pMinValue[0])
                L = 0.0;
            else if (L > m_pMaxValue[0])
                L = 100.0;
            else
                L = ((L - m_pMinValue[0]) / (double)(m_pMaxValue[0] - m_pMinValue[0])) * 100.0;

            //----------------------------------------
            // Process gamme adjustement if required.
            if (!HDOUBLE_EQUAL_EPSILON(m_pGammaFactor[0], 1.0))
                {
                L = (unsigned int)(100.0 * pow( L / 100.0, 1 / m_pGammaFactor[0]));
                }

            //----------------------------------------
            // Compute output clipping if any.
            if ( m_pMaxContrastValue[0] < 100 || m_pMinContrastValue[0] > 0)
                {
                // Take care of inverted handle.
                if ( m_pMaxContrastValue[0] > m_pMinContrastValue[0] )
                    {
                    L = ((L /100.0) * (m_pMaxContrastValue[0] - m_pMinContrastValue[0])) + m_pMinContrastValue[0];
                    }
                else
                    {
                    L = (((100.0 - L) /100.0) * (m_pMinContrastValue[0] - m_pMaxContrastValue[0])) + m_pMaxContrastValue[0];
                    }
                }

            //------------------------------------------
            // Convert back to original pixeltype (Grayscale)
            m_pColorSpaceConverter->ConvertToRGB (L, U, V, pDestRawData, pDestRawData, pDestRawData);
            pDestRawData++;

            pi_PixelsCount--;
            }
        }
    else if(m_Channels == 3)
        {
        while(pi_PixelsCount)
            {
            //------------------------------------------
            // Convert to L*u*v
            m_pColorSpaceConverter->ConvertFromRGB (*pSrcRawData, *(pSrcRawData + 1), *(pSrcRawData + 2), &L, &U, &V);

            //------------------------------------------
            // Process the LigthnessContrastStretch
            // Input clipping.
            if (L <= m_pMinValue[0])
                L = 0.0;
            else if (L >= m_pMaxValue[0])
                L = 100.0;
            else
                L = ((L - m_pMinValue[0]) / (double)(m_pMaxValue[0] - m_pMinValue[0])) * 100.0;

            HASSERT(L >= 0.0 && L <= 100.0);

            //----------------------------------------
            // Process gamme adjustement if required.
            if (!HDOUBLE_EQUAL_EPSILON(m_pGammaFactor[0], 1.0))
                {
                L = (unsigned char)(100.0 * pow( L / 100.0, 1 / m_pGammaFactor[0]));
                HASSERT(L >= 0.0 && L <= 100.0);
                }

            //----------------------------------------
            // Compute output clipping if any.
            if ( m_pMaxContrastValue[0] < 100 || m_pMinContrastValue[0] > 0)
                {
                // Take care of inverted handle.
                if ( m_pMaxContrastValue[0] > m_pMinContrastValue[0] )
                    {
                    L = ((L /100.0) * (m_pMaxContrastValue[0] - m_pMinContrastValue[0])) + m_pMinContrastValue[0];
                    }
                else
                    {
                    L = (((100.0 - L) /100.0) * (m_pMinContrastValue[0] - m_pMaxContrastValue[0])) + m_pMaxContrastValue[0];
                    }
                HASSERT(L >= 0.0 && L <= 100.0);
                }

            //------------------------------------------
            // Convert back to original pixeltype (RGB or RGBA)
            m_pColorSpaceConverter->ConvertToRGB (L, U, V, pDestRawData, pDestRawData + 1, pDestRawData + 2);

            pDestRawData += m_Channels;
            pSrcRawData += m_Channels;

            pi_PixelsCount--;
            }
        }
    else if(m_Channels == 4)
        {
        while(pi_PixelsCount)
            {
            //------------------------------------------
            // Convert to L*u*v
            m_pColorSpaceConverter->ConvertFromRGB (*pSrcRawData, *(pSrcRawData + 1), *(pSrcRawData + 2), &L, &U, &V);

            //------------------------------------------
            // Process the LigthnessContrastStretch
            // Input clipping.
            if (L <= m_pMinValue[0])
                L = 0.0;
            else if (L >= m_pMaxValue[0])
                L = 100.0;
            else
                L = ((L - m_pMinValue[0]) / (double)(m_pMaxValue[0] - m_pMinValue[0])) * 100.0;

            HASSERT(L >= 0.0 && L <= 100.0);

            //----------------------------------------
            // Process gamme adjustement if required.
            if (!HDOUBLE_EQUAL_EPSILON(m_pGammaFactor[0], 1.0))
                {
                L = (unsigned char)(100.0 * pow( L / 100.0, 1 / m_pGammaFactor[0]));
                HASSERT(L >= 0.0 && L <= 100.0);
                }

            //----------------------------------------
            // Compute output clipping if any.
            if ( m_pMaxContrastValue[0] < 100 || m_pMinContrastValue[0] > 0)
                {
                // Take care of inverted handle.
                if ( m_pMaxContrastValue[0] > m_pMinContrastValue[0] )
                    {
                    L = ((L /100.0) * (m_pMaxContrastValue[0] - m_pMinContrastValue[0])) + m_pMinContrastValue[0];
                    }
                else
                    {
                    L = (((100.0 - L) /100.0) * (m_pMinContrastValue[0] - m_pMaxContrastValue[0])) + m_pMaxContrastValue[0];
                    }
                HASSERT(L >= 0.0 && L <= 100.0);
                }

            //------------------------------------------
            // Convert back to original pixeltype (RGB or RGBA)
            m_pColorSpaceConverter->ConvertToRGB (L, U, V, pDestRawData, pDestRawData + 1, pDestRawData + 2);
            *(pDestRawData + 3) = pSrcRawData[3];

            pDestRawData += m_Channels;
            pSrcRawData += m_Channels;

            pi_PixelsCount--;
            }
        }
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

void HRPLigthnessContrastStretch::DeepCopy(const HRPLigthnessContrastStretch& pi_rSrc)
    {
    m_Channels       = pi_rSrc.m_Channels;
    m_ChannelWidth   = pi_rSrc.m_ChannelWidth;
    m_MaxSampleValue = pi_rSrc.m_MaxSampleValue;

    m_pMinValue         = new int   [m_Channels];
    m_pMaxValue         = new int   [m_Channels];
    m_pMinContrastValue = new int   [m_Channels];
    m_pMaxContrastValue = new int   [m_Channels];
    m_pGammaFactor      = new double[m_Channels];

    m_pColorSpaceConverter = new HGFLuvColorSpace(DEFAULT_GAMMA_FACTOR, m_ChannelWidth);

    for (uint32_t ChannelIndex = 0; ChannelIndex < m_Channels; ChannelIndex++)
        {
        m_pMinValue        [ChannelIndex] = pi_rSrc.m_pMinValue[ChannelIndex];
        m_pMaxValue        [ChannelIndex] = pi_rSrc.m_pMaxValue[ChannelIndex];

        m_pMinContrastValue[ChannelIndex] = pi_rSrc.m_pMinContrastValue[ChannelIndex];
        m_pMaxContrastValue[ChannelIndex] = pi_rSrc.m_pMaxContrastValue[ChannelIndex];

        m_pGammaFactor     [ChannelIndex] = pi_rSrc.m_pGammaFactor[ChannelIndex];
        }
    }

//-----------------------------------------------------------------------------
// public
// GetInterval
//-----------------------------------------------------------------------------

void HRPLigthnessContrastStretch::GetInterval(uint32_t pi_ChannelIndex,
                                              int* po_pMinValue,
                                              int* po_pMaxValue) const
    {
    HPRECONDITION(pi_ChannelIndex < m_Channels);
    HPRECONDITION(po_pMinValue != 0);
    HPRECONDITION(po_pMaxValue != 0);

    *po_pMinValue = (int)(ceil((double)m_pMinValue[pi_ChannelIndex] / 100.0 * m_MaxSampleValue));
    *po_pMaxValue = (int)(ceil((double)m_pMaxValue[pi_ChannelIndex] / 100.0 * m_MaxSampleValue));
    }

//-----------------------------------------------------------------------------
// public
// SetMap
//-----------------------------------------------------------------------------

void HRPLigthnessContrastStretch::SetInterval(uint32_t pi_ChannelIndex,
                                              int pi_MinValue,
                                              int pi_MaxValue)
    {
    HPRECONDITION(pi_ChannelIndex < m_Channels);
    HPRECONDITION(pi_MinValue <= pi_MaxValue);
    HPRECONDITION(pi_MinValue <= m_MaxSampleValue && pi_MinValue >= 0);
    HPRECONDITION(pi_MaxValue <= m_MaxSampleValue && pi_MaxValue >= 0);

    m_pMinValue[pi_ChannelIndex] = (int)(((double)pi_MinValue / m_MaxSampleValue) * 100.0);
    m_pMaxValue[pi_ChannelIndex] = (int)(((double)pi_MaxValue / m_MaxSampleValue) * 100.0);
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

void HRPLigthnessContrastStretch::SetContrastInterval(uint32_t pi_ChannelIndex,
                                                      int pi_MinContrastValue,
                                                      int pi_MaxContrastValue)
    {
    HPRECONDITION(pi_ChannelIndex < m_Channels);
    HPRECONDITION(pi_MinContrastValue <= m_MaxSampleValue && pi_MinContrastValue >= 0);
    HPRECONDITION(pi_MaxContrastValue <= m_MaxSampleValue && pi_MaxContrastValue >= 0);

    m_pMinContrastValue[pi_ChannelIndex] = (int)(((double)pi_MinContrastValue / m_MaxSampleValue) * 100.0);
    m_pMaxContrastValue[pi_ChannelIndex] = (int)(((double)pi_MaxContrastValue / m_MaxSampleValue) * 100.0);
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

void HRPLigthnessContrastStretch::GetContrastInterval(uint32_t pi_ChannelIndex,
                                                      int* po_MinContrastValue,
                                                      int* po_MaxContrastValue) const
    {
    HPRECONDITION(pi_ChannelIndex < m_Channels);
    HPRECONDITION(po_MinContrastValue != 0);
    HPRECONDITION(po_MaxContrastValue != 0);

    *po_MinContrastValue = (int)(ceil((double)m_pMinContrastValue[pi_ChannelIndex] / 100.0 * m_MaxSampleValue));
    *po_MaxContrastValue = (int)(ceil((double)m_pMaxContrastValue[pi_ChannelIndex] / 100.0 * m_MaxSampleValue));
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

void HRPLigthnessContrastStretch::SetGammaFactor(uint32_t  pi_ChannelIndex,
                                                 double pi_GammaFactor)
    {
    HPRECONDITION(pi_ChannelIndex < m_Channels);
    HPRECONDITION(pi_GammaFactor <= 10.0 && pi_GammaFactor > 0.0);

    m_pGammaFactor[pi_ChannelIndex] = pi_GammaFactor;
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

void HRPLigthnessContrastStretch::GetGammaFactor(uint32_t   pi_ChannelIndex,
                                                 double* po_pGammaFactor) const
    {
    HPRECONDITION(pi_ChannelIndex < m_Channels);
    HPRECONDITION(po_pGammaFactor != 0);

    *po_pGammaFactor = m_pGammaFactor[pi_ChannelIndex];
    }
