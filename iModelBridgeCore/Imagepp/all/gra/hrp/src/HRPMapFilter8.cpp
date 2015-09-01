//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrp/src/HRPMapFilter8.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for class HRPMapFilter8
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HRPMapFilter8.h>
#include <Imagepp/all/h/HRPPixelTypeV24R8G8B8.h>

//-----------------------------------------------------------------------------
// public
// Constructor.
//-----------------------------------------------------------------------------
HRPMapFilter8::HRPMapFilter8()
    :HRPFunctionFilter(new HRPPixelTypeV24R8G8B8())
    {
    m_Channels = 3;

    Initialize();
    }

//-----------------------------------------------------------------------------
// public
// Constructor.
//-----------------------------------------------------------------------------
HRPMapFilter8::HRPMapFilter8(Byte pi_channels)
    :HRPFunctionFilter(new HRPPixelTypeV24R8G8B8())
    {
    m_Channels = pi_channels;

    Initialize();
    }

//-----------------------------------------------------------------------------
// public
// Constructor.
//-----------------------------------------------------------------------------
HRPMapFilter8::HRPMapFilter8(const HFCPtr<HRPPixelType>& pi_pFilterPixelType)
    :HRPFunctionFilter(pi_pFilterPixelType)
    {
    HPRECONDITION(pi_pFilterPixelType->CountIndexBits() == 0);
    HPRECONDITION((pi_pFilterPixelType->CountPixelRawDataBits() % 8) == 0);

    m_Channels = (Byte)(pi_pFilterPixelType->CountPixelRawDataBits() / 8);

    Initialize();
    }

//-----------------------------------------------------------------------------
// public
// Copy constructor
//-----------------------------------------------------------------------------
HRPMapFilter8::HRPMapFilter8(const HRPMapFilter8& pi_rFilter)
    : HRPFunctionFilter(pi_rFilter)
    {
    m_Channels = pi_rFilter.m_Channels;

    Initialize();

    HASSERT(m_pMap     != 0);
    HASSERT(m_Channels >  0);

    // copy the map
    memcpy(m_pMap, pi_rFilter.m_pMap, 256 * m_Channels);
    }

//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HRPMapFilter8::~HRPMapFilter8()
    {
    DeepDelete();
    }

//-----------------------------------------------------------------------------
// public
// ComposeWith
//-----------------------------------------------------------------------------
HRPFilter* HRPMapFilter8::ComposeWith(const HRPFilter* pi_pFilter)
    {
    HPRECONDITION(pi_pFilter != 0);

    HRPFilter* pFilter = 0;

    // verify if the parameter is a convolution filter
    if(!pi_pFilter->IsCompatibleWith(HRPMapFilter8::CLASS_ID) ||
       !((HRPTypedFilter*)pi_pFilter)->GetFilterPixelType()->HasSamePixelInterpretation(*GetFilterPixelType()))
        {
        // if not, call the parent method
        pFilter = HRPFunctionFilter::ComposeWith(pi_pFilter);
        }
    else
        {
        HRPMapFilter8* pComposedFilter = (HRPMapFilter8*)this->Clone();

        HRPMapFilter8* pMapFilter = (HRPMapFilter8*)pi_pFilter;

        // merge the tables
        for(uint32_t Channel = 0; Channel < m_Channels; Channel++)
            for(unsigned short Index = 0; Index < 256; Index++)
                {
                pComposedFilter->m_pMap[(Channel << 8) + Index] =
                    pMapFilter->m_pMap[(Channel << 8) +
                                       m_pMap[(Channel << 8) + Index]];
                }

        pFilter = pComposedFilter;
        }

    return pFilter;
    }

//-----------------------------------------------------------------------------
// private
// Function
//-----------------------------------------------------------------------------
void HRPMapFilter8::Function( const void* pi_pSrcRawData,
                              void* po_pDestRawData,
                              uint32_t PixelsCount) const
    {
    HPRECONDITION (po_pDestRawData != 0);
    HPRECONDITION (pi_pSrcRawData  != 0);
    HPRECONDITION (PixelsCount      > 0);

    Byte* pSrcComposite = (Byte*)pi_pSrcRawData;
    Byte* pDstComposite = (Byte*)po_pDestRawData;

    if (m_Channels == 3)
        {
        while(PixelsCount)
            {
            *pDstComposite++ = m_pMap[*pSrcComposite++];        // Channel 0
            *pDstComposite++ = m_pMap[256 + *pSrcComposite++];    // Channel 1
            *pDstComposite++ = m_pMap[512 + *pSrcComposite++];    // Channel 2

            --PixelsCount;
            }
        }
    else if (m_Channels == 1)
        {
        while(PixelsCount)
            {
            *pDstComposite++ = m_pMap[*pSrcComposite++];

            --PixelsCount;
            }
        }
    else
        {
        uint32_t ChannelIndex;

        while(PixelsCount)
            {
            for(ChannelIndex = 0; ChannelIndex < m_Channels; ChannelIndex++)
                *pDstComposite++ = m_pMap[(ChannelIndex << 8) + *pSrcComposite++];

            --PixelsCount;
            }
        }
    }

//-----------------------------------------------------------------------------
// public
// GetMap
//-----------------------------------------------------------------------------
Byte* HRPMapFilter8::GetMap(uint32_t pi_ChannelIndex)
    {
    HPRECONDITION(pi_ChannelIndex < m_Channels);
    HPRECONDITION(m_pMap != 0);

    return (m_pMap + 256 * pi_ChannelIndex);
    }

//-----------------------------------------------------------------------------
// public
// SetMap
//-----------------------------------------------------------------------------
void HRPMapFilter8::SetMap(size_t pi_ChannelIndex, const Byte* pi_pMap)
    {
    HPRECONDITION(pi_ChannelIndex < m_Channels);
    HPRECONDITION(m_pMap != 0);

    // copy the map
    memcpy(m_pMap + 256 * pi_ChannelIndex, pi_pMap, 256);
    }

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// private
// DeepDelete
//-----------------------------------------------------------------------------
void HRPMapFilter8::DeepDelete()
    {
    delete m_pMap;
    m_pMap = 0;
    }

//-----------------------------------------------------------------------------
// private
// Initialize
//-----------------------------------------------------------------------------
void HRPMapFilter8::Initialize()
    {
    HPRECONDITION(m_Channels > 0);

    // create a map
    m_MapSize = m_Channels * 256;
    m_pMap = new Byte[m_MapSize];
    }
