//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrp/src/HRPLigthnessDensitySlicingFilter.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for class HRPLigthnessDensitySlicingFilter
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HRPLigthnessDensitySlicingFilter.h>
#include <Imagepp/all/h/HRPPixelTypeV24R8G8B8.h>
#include <Imagepp/all/h/HGFLightnessColorSpace.h>

//-----------------------------------------------------------------------------
//  Custom Map8  Filter
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// public
// Constructor.
//-----------------------------------------------------------------------------

HRPLigthnessDensitySlicingFilter::HRPLigthnessDensitySlicingFilter()
    :HRPFunctionFilter(new HRPPixelTypeV24R8G8B8())
    {
    m_ChannelWidth       = 8;
    m_MaxSampleValue     = 255;
    m_Channels           = 3;
    m_DesaturationFactor = 0.0;

    m_pColorSpaceConverter = new HGFLightnessColorSpace(DEFAULT_GAMMA_FACTOR, m_ChannelWidth);
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPLigthnessDensitySlicingFilter::HRPLigthnessDensitySlicingFilter(const HRPLigthnessDensitySlicingFilter& pi_rFilter)
    :HRPFunctionFilter(pi_rFilter)
    {
    DeepCopy(pi_rFilter);
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPLigthnessDensitySlicingFilter::HRPLigthnessDensitySlicingFilter(const HFCPtr<HRPPixelType>& pi_pFilterPixelType)
    : HRPFunctionFilter(pi_pFilterPixelType)
    {
    if (pi_pFilterPixelType->CountIndexBits()) //  GetPalette())
        {
        m_ChannelWidth   = pi_pFilterPixelType->CountIndexBits();
        m_Channels       = (unsigned short)pi_pFilterPixelType->GetChannelOrg().CountChannels();
        }
    else
        {
        m_ChannelWidth       = pi_pFilterPixelType->GetChannelOrg().GetChannelPtr(0)->GetSize();
        m_Channels           = (unsigned short)pi_pFilterPixelType->CountPixelRawDataBits() / m_ChannelWidth;

        HASSERT(m_ChannelWidth == 8 || m_ChannelWidth == 16);
        }

    m_MaxSampleValue     = (1 << m_ChannelWidth) - 1;
    m_DesaturationFactor = 0.0;

    m_pColorSpaceConverter = new HGFLightnessColorSpace(DEFAULT_GAMMA_FACTOR, m_ChannelWidth);
    }

//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------

HRPLigthnessDensitySlicingFilter::~HRPLigthnessDensitySlicingFilter()
    {
    m_SliceList.clear();
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

int32_t HRPLigthnessDensitySlicingFilter::AddSlice( int32_t pi_StartIndex,
                                                 int32_t pi_EndIndex,
                                                 uint32_t pi_StartColor,
                                                 uint32_t pi_EndColor,
                                                 int32_t pi_Opacity)
    {
    HPRECONDITION(pi_StartIndex >= 0 && pi_StartIndex <= m_MaxSampleValue);
    HPRECONDITION(pi_EndIndex   >= 0 && pi_EndIndex   <= m_MaxSampleValue);
    HPRECONDITION(pi_StartIndex <= pi_EndIndex);
    HPRECONDITION(pi_Opacity    >= 0 && pi_Opacity <= 100);

    SliceInfo NewSlice;

    NewSlice.m_StartIndex = (int32_t)(MAX(MIN(pi_StartIndex, m_MaxSampleValue), 0.0));
    NewSlice.m_EndIndex   = (int32_t)(MAX(MIN(pi_EndIndex,   m_MaxSampleValue), 0.0));

    // Do not allow the pi_EndColor tobe before the pi_StartColor
    if (NewSlice.m_EndIndex < NewSlice.m_StartIndex)
        NewSlice.m_EndIndex = NewSlice.m_StartIndex;

    NewSlice.m_StartColor = pi_StartColor;
    NewSlice.m_EndColor   = pi_EndColor;

    NewSlice.m_Opacity    = (int32_t)(MAX(MIN(pi_Opacity, 100.0), 0.0));

    m_SliceList.push_back(NewSlice);

    return (int32_t)m_SliceList.size();
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

bool HRPLigthnessDensitySlicingFilter::ModifySlice (int32_t pi_StartIndex,
                                                     int32_t pi_EndIndex,
                                                     uint32_t pi_StartColor,
                                                     uint32_t pi_EndColor,
                                                     int32_t pi_Opacity)
    {
    HPRECONDITION(pi_StartIndex >= 0);
    HPRECONDITION(pi_EndIndex   >= 0);
    HPRECONDITION(pi_StartIndex <= pi_EndIndex);

    int32_t SelectedSlice = GetSliceIndex (pi_StartIndex, pi_EndIndex);
    HASSERT(SelectedSlice >= 0);

    bool SliceModified = false;

    // If the slice exist..
    if (SelectedSlice >= 0)
        {
        m_SliceList[SelectedSlice].m_StartIndex = (int32_t)(MAX(MIN(pi_StartIndex, m_MaxSampleValue), 0.0));
        m_SliceList[SelectedSlice].m_EndIndex   = (int32_t)(MAX(MIN(pi_EndIndex,   m_MaxSampleValue), 0.0));

        // Do not allow the pi_EndColor tobe before the pi_StartColor
        if (m_SliceList[SelectedSlice].m_EndIndex < m_SliceList[SelectedSlice].m_StartIndex)
            m_SliceList[SelectedSlice].m_EndIndex = m_SliceList[SelectedSlice].m_StartIndex;

        m_SliceList[SelectedSlice].m_StartColor = pi_StartColor;
        m_SliceList[SelectedSlice].m_EndColor   = pi_EndColor;

        m_SliceList[SelectedSlice].m_Opacity    = (int32_t)(MAX(MIN(pi_Opacity, 100.0), 0.0));

        SliceModified = true;
        }

    return SliceModified;
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

int32_t HRPLigthnessDensitySlicingFilter::GetSliceIndex (int32_t pi_StartIndex,
                                                      int32_t pi_EndIndex) const
    {
    HPRECONDITION(pi_StartIndex >= 0);
    HPRECONDITION(pi_EndIndex   >= 0);
    HPRECONDITION(pi_StartIndex <= pi_EndIndex);

    bool  SliceFound = false;
    int   SliceIndex = 0;

    while (!SliceFound && SliceIndex < (int)(m_SliceList.size()) )
        {
        if ( (m_SliceList[SliceIndex].m_StartIndex == pi_StartIndex) && (m_SliceList[SliceIndex].m_EndIndex == pi_EndIndex) )
            SliceFound = true;
        else
            SliceIndex++;
        }

    if (!SliceFound)
        SliceIndex = -1;

    return SliceIndex;
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

bool HRPLigthnessDensitySlicingFilter::GetSliceInfo(int32_t pi_SliceIndex,
                                                     int32_t* po_pStartIndex,
                                                     int32_t* po_pEndIndex,
                                                     int32_t* po_pStartColor,
                                                     int32_t* po_pEndColor,
                                                     int32_t* po_pOpacity) const
    {
    HPRECONDITION(pi_SliceIndex >= 0);
    HPRECONDITION(pi_SliceIndex < (int)(m_SliceList.size()));

    HPRECONDITION(po_pStartIndex != 0);
    HPRECONDITION(po_pEndIndex   != 0);
    HPRECONDITION(po_pStartColor != 0);
    HPRECONDITION(po_pEndColor   != 0);
    HPRECONDITION(po_pOpacity    != 0);

    // Play safe, be sure the given Index stay inbound..
    if (pi_SliceIndex >= 0 && pi_SliceIndex < (int)(m_SliceList.size()))
        {
        *po_pStartIndex = m_SliceList[pi_SliceIndex].m_StartIndex;
        *po_pEndIndex   = m_SliceList[pi_SliceIndex].m_EndIndex;
        *po_pStartColor = m_SliceList[pi_SliceIndex].m_StartColor;
        *po_pEndColor   = m_SliceList[pi_SliceIndex].m_EndColor;
        *po_pOpacity    = m_SliceList[pi_SliceIndex].m_Opacity;
        }

    // Return true if the given Index is valid.
    return (pi_SliceIndex >= 0 && pi_SliceIndex < (int)(m_SliceList.size()));
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

void HRPLigthnessDensitySlicingFilter::RemoveSlice (int32_t pi_StartIndex,
                                                    int32_t pi_EndIndex)
    {
    HPRECONDITION(pi_StartIndex >= 0);
    HPRECONDITION(pi_EndIndex   >= 0);
    HPRECONDITION(pi_StartIndex <= pi_EndIndex);

    RemoveSlice (GetSliceIndex(pi_StartIndex, pi_EndIndex));
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

void HRPLigthnessDensitySlicingFilter::RemoveSlice(int32_t pi_SliceIndex)
    {
    HPRECONDITION(pi_SliceIndex >= 0);
    HPRECONDITION(pi_SliceIndex < (int)(m_SliceList.size()));

    // Dont try to remove something we don't own..
    if (pi_SliceIndex >= 0 && pi_SliceIndex < (int)(m_SliceList.size()))
        {
        vector<SliceInfo >::iterator SliceItr = m_SliceList.begin();
        int32_t CurrentIndex = 0;

        while(CurrentIndex < pi_SliceIndex)
            {
            HASSERT(SliceItr != m_SliceList.end());

            CurrentIndex++;
            SliceItr++;
            }
        HASSERT(SliceItr != m_SliceList.end());
        m_SliceList.erase(SliceItr);
        }
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

void HRPLigthnessDensitySlicingFilter::RemoveAll()
    {
    m_SliceList.clear();
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

int32_t HRPLigthnessDensitySlicingFilter::GetSliceCount() const
    {
    return (int32_t)m_SliceList.size();
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

void HRPLigthnessDensitySlicingFilter::DeepCopy(const HRPLigthnessDensitySlicingFilter& pi_rSrc)
    {
    m_SliceList          = pi_rSrc.m_SliceList;
    m_ChannelWidth       = pi_rSrc.m_ChannelWidth;
    m_MaxSampleValue     = pi_rSrc.m_MaxSampleValue;
    m_Channels           = pi_rSrc.m_Channels;
    m_DesaturationFactor = pi_rSrc.m_DesaturationFactor;

    m_pColorSpaceConverter = new HGFLightnessColorSpace(DEFAULT_GAMMA_FACTOR, m_ChannelWidth);
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
HRPFilter* HRPLigthnessDensitySlicingFilter::Clone() const
    {
    return new HRPLigthnessDensitySlicingFilter(*this);
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

void HRPLigthnessDensitySlicingFilter::Function ( const void*  pi_pSrcRawData,
                                                  void*  po_pDestRawData,
                                                  uint32_t pi_PixelsCount) const
    {
    HPRECONDITION(pi_pSrcRawData  != 0);
    HPRECONDITION(po_pDestRawData != 0);
    HPRECONDITION(pi_PixelsCount   > 0);
    HPRECONDITION(m_DesaturationFactor >= 0.0 && m_DesaturationFactor <= 1.0);

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
        HASSERT(!m_ChannelWidth);
        FunctionPalette( pi_pSrcRawData, po_pDestRawData, pi_PixelsCount);
        }
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

void HRPLigthnessDensitySlicingFilter::FunctionN8( const void*  pi_pSrcRawData,
                                                   void*  po_pDestRawData,
                                                   uint32_t pi_PixelsCount) const
    {
    Byte* pSrcRawData  = (Byte*)pi_pSrcRawData;
    Byte* pDestRawData = (Byte*)po_pDestRawData;

    double Scale = 255.0 / 100.0 + HGLOBAL_EPSILON;

    // The code as been duplicated to avoid "if" statement on each loop
    // to improve performance.


    if(m_Channels == 1)
        {
        while(pi_PixelsCount)
            {
            *pDestRawData = *pSrcRawData;

            // Next one!
            pSrcRawData++;
            pDestRawData++;
            pi_PixelsCount--;
            }
        }
    else if (m_Channels == 3)
        {
        while(pi_PixelsCount)
            {
            Byte GrayValue  =  (Byte)(m_pColorSpaceConverter->ConvertFromRGB (pSrcRawData[0], pSrcRawData[1], pSrcRawData[2]) * Scale);

            bool  IsSliced = false;

            int     RedValue=0;
            int     GreenValue=0;
            int     BlueValue=0;
            double Opacity=0.0;

            vector<SliceInfo >::const_iterator SliceItr = m_SliceList.begin();
            while (!IsSliced && SliceItr != m_SliceList.end())
                {
                if (GrayValue >= (*SliceItr).m_StartIndex && GrayValue <= (*SliceItr).m_EndIndex)
                    {
                    double GradientFactor = (GrayValue - (*SliceItr).m_StartIndex) / (double)(MAX((*SliceItr).m_EndIndex - (*SliceItr).m_StartIndex, 1.0));

                    RedValue   = (int)(((((*SliceItr).m_StartColor & 0x00FF0000) >> 16) * (1 - GradientFactor)) +
                                       ((((*SliceItr).m_EndColor   & 0x00FF0000) >> 16) * GradientFactor));

                    GreenValue = (int)(((((*SliceItr).m_StartColor & 0x0000FF00) >> 8) * (1 - GradientFactor)) +
                                       ((((*SliceItr).m_EndColor   & 0x0000FF00) >> 8) * GradientFactor));

                    BlueValue  = (int)((((*SliceItr).m_StartColor & 0x000000FF)        * (1 - GradientFactor)) +
                                       (((*SliceItr).m_EndColor   & 0x000000FF)        * GradientFactor));

                    Opacity   = (*SliceItr).m_Opacity / 100.0;

                    // Indicate the current pixel is in this slide, and break the loop.
                    IsSliced   = true;
                    }
                else
                    SliceItr++;
                }

            if (IsSliced)
                {
                HASSERT(Opacity >= 0.0 && Opacity <= 1.0);

                // Red channel
                *(pDestRawData    ) = (Byte)((pSrcRawData[0] * (1 - Opacity)) + (RedValue * Opacity));

                // Green channel
                *(pDestRawData + 1) = (Byte)((pSrcRawData[1] * (1 - Opacity)) + (GreenValue * Opacity));

                // Blue channel
                *(pDestRawData + 2) = (Byte)((pSrcRawData[2] * (1 - Opacity)) + (BlueValue * Opacity));
                }
            else
                {
                *(pDestRawData    ) = (Byte)((pSrcRawData[0] * (1.0 - m_DesaturationFactor)) + (GrayValue * m_DesaturationFactor));
                *(pDestRawData + 1) = (Byte)((pSrcRawData[1] * (1.0 - m_DesaturationFactor)) + (GrayValue * m_DesaturationFactor));
                *(pDestRawData + 2) = (Byte)((pSrcRawData[2] * (1.0 - m_DesaturationFactor)) + (GrayValue * m_DesaturationFactor));
                }

            // Get ready to process the next pixel
            pSrcRawData  += m_Channels;
            pDestRawData += m_Channels;
            pi_PixelsCount--;
            }
        }
    else
        {
        HASSERT(m_Channels  == 4);

        while(pi_PixelsCount)
            {
            Byte GrayValue  =  (Byte)(m_pColorSpaceConverter->ConvertFromRGB (pSrcRawData[0], pSrcRawData[1], pSrcRawData[2]) * Scale);

            bool  IsSliced = false;

            int     RedValue=0;
            int     GreenValue=0;
            int     BlueValue=0;
            double Opacity=0.0;

            vector<SliceInfo >::const_iterator SliceItr = m_SliceList.begin();
            while (!IsSliced && SliceItr != m_SliceList.end())
                {
                if (GrayValue >= (*SliceItr).m_StartIndex && GrayValue <= (*SliceItr).m_EndIndex)
                    {
                    double GradientFactor = (GrayValue - (*SliceItr).m_StartIndex) / (double)(MAX((*SliceItr).m_EndIndex - (*SliceItr).m_StartIndex, 1.0));

                    RedValue   = (int)(((((*SliceItr).m_StartColor & 0x00FF0000) >> 16) * (1 - GradientFactor)) +
                                       ((((*SliceItr).m_EndColor   & 0x00FF0000) >> 16) * GradientFactor));

                    GreenValue = (int)(((((*SliceItr).m_StartColor & 0x0000FF00) >> 8) * (1 - GradientFactor)) +
                                       ((((*SliceItr).m_EndColor   & 0x0000FF00) >> 8) * GradientFactor));

                    BlueValue  = (int)((((*SliceItr).m_StartColor & 0x000000FF)        * (1 - GradientFactor)) +
                                       (((*SliceItr).m_EndColor   & 0x000000FF)        * GradientFactor));

                    Opacity   = (*SliceItr).m_Opacity / 100.0;

                    // Indicate the current pixel is in this slide, and break the loop.
                    IsSliced   = true;
                    }
                else
                    SliceItr++;
                }

            if (IsSliced)
                {
                HASSERT(Opacity >= 0.0 && Opacity <= 1.0);

                // Red channel
                *(pDestRawData    ) = (Byte)((pSrcRawData[0] * (1 - Opacity)) + (RedValue * Opacity));

                // Green channel
                *(pDestRawData + 1) = (Byte)((pSrcRawData[1] * (1 - Opacity)) + (GreenValue * Opacity));

                // Blue channel
                *(pDestRawData + 2) = (Byte)((pSrcRawData[2] * (1 - Opacity)) + (BlueValue * Opacity));

                // Keep the same alpha channel.
                *(pDestRawData + 3) = pSrcRawData[3];
                }
            else
                {
                *(pDestRawData    ) = (Byte)((pSrcRawData[0] * (1.0 - m_DesaturationFactor)) + (GrayValue * m_DesaturationFactor));
                *(pDestRawData + 1) = (Byte)((pSrcRawData[1] * (1.0 - m_DesaturationFactor)) + (GrayValue * m_DesaturationFactor));
                *(pDestRawData + 2) = (Byte)((pSrcRawData[2] * (1.0 - m_DesaturationFactor)) + (GrayValue * m_DesaturationFactor));
                *(pDestRawData + 3) = pSrcRawData[3];
                }

            // Get ready to process the next pixel
            pSrcRawData  += m_Channels;
            pDestRawData += m_Channels;
            pi_PixelsCount--;
            }
        }
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

void HRPLigthnessDensitySlicingFilter::FunctionN16( const void*  pi_pSrcRawData,
                                                    void*  po_pDestRawData,
                                                    uint32_t pi_PixelsCount) const
    {
    HPRECONDITION(pi_pSrcRawData  != 0);
    HPRECONDITION(po_pDestRawData != 0);
    HPRECONDITION(pi_PixelsCount   > 0);
    HPRECONDITION(m_DesaturationFactor >= 0.0 && m_DesaturationFactor <= 1.0);

    unsigned short* pSrcRawData  = (unsigned short*)pi_pSrcRawData;
    unsigned short* pDestRawData = (unsigned short*)po_pDestRawData;

    double Scale = 65535.0 / 100.0 + HGLOBAL_EPSILON;


    // The code as been duplicated to avoid "if" statement on each loop
    // to improve performance.

    if(m_Channels == 1)
        {
        while(pi_PixelsCount)
            {
            *pDestRawData = *pSrcRawData;

            // Next one!
            pSrcRawData++;
            pDestRawData++;
            pi_PixelsCount--;
            }
        }
    else if (m_Channels == 3)
        {
        while(pi_PixelsCount)
            {
            unsigned short GrayValue  =  (unsigned short)(m_pColorSpaceConverter->ConvertFromRGB (pSrcRawData[0], pSrcRawData[1], pSrcRawData[2]) * Scale);

            bool   IsSliced = false;

            int     RedValue=0;
            int     GreenValue=0;
            int     BlueValue=0;
            double Opacity=0.0;

            vector<SliceInfo >::const_iterator SliceItr = m_SliceList.begin();
            while (!IsSliced && SliceItr != m_SliceList.end())
                {
                if (GrayValue >= (*SliceItr).m_StartIndex && GrayValue <= (*SliceItr).m_EndIndex)
                    {
                    double GradientFactor = (GrayValue - (*SliceItr).m_StartIndex) / (double)(MAX((*SliceItr).m_EndIndex - (*SliceItr).m_StartIndex, 1.0));

                    RedValue   = (int)(((((*SliceItr).m_StartColor & 0x00FF0000) >> 16) * (1 - GradientFactor)) +
                                       ((((*SliceItr).m_EndColor   & 0x00FF0000) >> 16) * GradientFactor));

                    // 8 bit value to 16 bit value
                    RedValue <<= 8;

                    GreenValue = (int)(((((*SliceItr).m_StartColor & 0x0000FF00) >> 8) * (1 - GradientFactor)) +
                                       ((((*SliceItr).m_EndColor   & 0x0000FF00) >> 8) * GradientFactor));

                    // 8 bit value to 16 bit value
                    GreenValue <<= 8;

                    BlueValue  = (int)((((*SliceItr).m_StartColor & 0x000000FF)        * (1 - GradientFactor)) +
                                       (((*SliceItr).m_EndColor   & 0x000000FF)        * GradientFactor));

                    // 8 bit value to 16 bit value
                    BlueValue <<= 8;

                    Opacity   = (*SliceItr).m_Opacity / 100.0;

                    // Indicate the current pixel is in this slide, and break the loop.
                    IsSliced   = true;
                    }
                else
                    SliceItr++;
                }

            if (IsSliced)
                {
                HASSERT(Opacity >= 0.0 && Opacity <= 1.0);

                // Red channel
                *(pDestRawData    ) = (unsigned short)((pSrcRawData[0] * (1 - Opacity)) + (RedValue * Opacity));

                // Green channel
                *(pDestRawData + 1) = (unsigned short)((pSrcRawData[1] * (1 - Opacity)) + (GreenValue * Opacity));

                // Blue channel
                *(pDestRawData + 2) = (unsigned short)((pSrcRawData[2] * (1 - Opacity)) + (BlueValue * Opacity));
                }
            else
                {
                *(pDestRawData    ) = (unsigned short)((pSrcRawData[0] * (1.0 - m_DesaturationFactor)) + (GrayValue * m_DesaturationFactor));
                *(pDestRawData + 1) = (unsigned short)((pSrcRawData[1] * (1.0 - m_DesaturationFactor)) + (GrayValue * m_DesaturationFactor));
                *(pDestRawData + 2) = (unsigned short)((pSrcRawData[2] * (1.0 - m_DesaturationFactor)) + (GrayValue * m_DesaturationFactor));
                }

            // Get ready to process the next pixel
            pSrcRawData  += m_Channels;
            pDestRawData += m_Channels;
            pi_PixelsCount--;
            }
        }
    else
        {
        HASSERT(m_Channels  == 4);

        while(pi_PixelsCount)
            {
            unsigned short GrayValue  =  (unsigned short)(m_pColorSpaceConverter->ConvertFromRGB (pSrcRawData[0], pSrcRawData[1], pSrcRawData[2]) * Scale);

            bool   IsSliced = false;

            int     RedValue=0;
            int     GreenValue=0;
            int     BlueValue=0;
            double Opacity=0.0;

            vector<SliceInfo >::const_iterator SliceItr = m_SliceList.begin();
            while (!IsSliced && SliceItr != m_SliceList.end())
                {
                if (GrayValue >= (*SliceItr).m_StartIndex && GrayValue <= (*SliceItr).m_EndIndex)
                    {
                    double GradientFactor = (GrayValue - (*SliceItr).m_StartIndex) / (double)(MAX((*SliceItr).m_EndIndex - (*SliceItr).m_StartIndex, 1.0));

                    RedValue   = (int)(((((*SliceItr).m_StartColor & 0x00FF0000) >> 16) * (1 - GradientFactor)) +
                                       ((((*SliceItr).m_EndColor   & 0x00FF0000) >> 16) * GradientFactor));

                    // 8 bit value to 16 bit value
                    RedValue <<= 8;

                    GreenValue = (int)(((((*SliceItr).m_StartColor & 0x0000FF00) >> 8) * (1 - GradientFactor)) +
                                       ((((*SliceItr).m_EndColor   & 0x0000FF00) >> 8) * GradientFactor));

                    // 8 bit value to 16 bit value
                    GreenValue <<= 8;

                    BlueValue  = (int)((((*SliceItr).m_StartColor & 0x000000FF)        * (1 - GradientFactor)) +
                                       (((*SliceItr).m_EndColor   & 0x000000FF)        * GradientFactor));

                    // 8 bit value to 16 bit value
                    BlueValue <<= 8;

                    Opacity   = (*SliceItr).m_Opacity / 100.0;

                    // Indicate the current pixel is in this slide, and break the loop.
                    IsSliced   = true;
                    }
                else
                    SliceItr++;
                }

            if (IsSliced)
                {
                HASSERT(Opacity >= 0.0 && Opacity <= 1.0);

                // Red channel
                *(pDestRawData    ) = (unsigned short)((pSrcRawData[0] * (1 - Opacity)) + (RedValue * Opacity));

                // Green channel
                *(pDestRawData + 1) = (unsigned short)((pSrcRawData[1] * (1 - Opacity)) + (GreenValue * Opacity));

                // Blue channel
                *(pDestRawData + 2) = (unsigned short)((pSrcRawData[2] * (1 - Opacity)) + (BlueValue * Opacity));

                // Keep the same alpha channel.
                *(pDestRawData + 3) = pSrcRawData[3];
                }
            else
                {
                *(pDestRawData    ) = (unsigned short)((pSrcRawData[0] * (1.0 - m_DesaturationFactor)) + (GrayValue * m_DesaturationFactor));
                *(pDestRawData + 1) = (unsigned short)((pSrcRawData[1] * (1.0 - m_DesaturationFactor)) + (GrayValue * m_DesaturationFactor));
                *(pDestRawData + 2) = (unsigned short)((pSrcRawData[2] * (1.0 - m_DesaturationFactor)) + (GrayValue * m_DesaturationFactor));
                *(pDestRawData + 3) = pSrcRawData[3];
                }

            // Get ready to process the next pixel
            pSrcRawData  += m_Channels;
            pDestRawData += m_Channels;
            pi_PixelsCount--;
            }
        }
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

void HRPLigthnessDensitySlicingFilter::FunctionPalette( const void*  pi_pSrcRawData,
                                                        void*  po_pDestRawData,
                                                        uint32_t pi_PixelsCount) const
    {
    // Not yet supported!
    HASSERT(false);
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

void HRPLigthnessDensitySlicingFilter::SetDesaturationFactor(double pi_DesaturationFactor)
    {
    HPRECONDITION(pi_DesaturationFactor >= 0.0 && pi_DesaturationFactor <= 100.0);

    m_DesaturationFactor = MIN( MAX(pi_DesaturationFactor, 0.0), 100.0);

    // Internally we normalize the factor to optimize some computation.
    m_DesaturationFactor /= 100.0;
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

double HRPLigthnessDensitySlicingFilter::GetDesaturationFactor() const
    {
    // Return the de-normalized value.
    return m_DesaturationFactor * 100.0;
    }
