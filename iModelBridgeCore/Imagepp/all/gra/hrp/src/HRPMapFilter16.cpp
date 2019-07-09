//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for class HRPMapFilter16
//-----------------------------------------------------------------------------

#include <ImageppInternal.h>


#include <ImagePP/all/h/HRPMapFilter16.h>
#include <ImagePP/all/h/HRPPixelTypeV48R16G16B16.h>

//-----------------------------------------------------------------------------
// public
// Constructor.
//-----------------------------------------------------------------------------

HRPMapFilter16::HRPMapFilter16()
    :HRPFunctionFilter(new HRPPixelTypeV48R16G16B16())
    {
    m_Channels = 3;
    Initialize();
    }

//-----------------------------------------------------------------------------
// public
// Constructor.
//-----------------------------------------------------------------------------

HRPMapFilter16::HRPMapFilter16(Byte pi_channels)
    :HRPFunctionFilter(new HRPPixelTypeV48R16G16B16())
    {
    m_Channels = pi_channels;
    Initialize();
    }

//-----------------------------------------------------------------------------
// public
// Constructor.
//-----------------------------------------------------------------------------

HRPMapFilter16::HRPMapFilter16(const HFCPtr<HRPPixelType>& pi_pFilterPixelType)
    :HRPFunctionFilter(pi_pFilterPixelType)
    {
    HPRECONDITION(pi_pFilterPixelType->CountIndexBits() == 0);
    HPRECONDITION((pi_pFilterPixelType->CountPixelRawDataBits() % 16) == 0);

    m_Channels = (Byte)pi_pFilterPixelType->CountPixelRawDataBits() / 16;
    Initialize();
    }

//-----------------------------------------------------------------------------
// Private
// Copy constructor
//-----------------------------------------------------------------------------

HRPMapFilter16::HRPMapFilter16(const HRPMapFilter16& pi_rFilter)
    :HRPFunctionFilter(pi_rFilter)
    {
    m_Channels = pi_rFilter.m_Channels;
    Initialize();

    HASSERT(m_pMap   != 0);
    HASSERT(m_MapSize > 0);

    // copy the map
    memcpy(m_pMap, pi_rFilter.m_pMap, m_MapSize * sizeof(uint16_t));
    }

//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------

HRPMapFilter16::~HRPMapFilter16()
    {
    DeepDelete();
    }

//-----------------------------------------------------------------------------
// public
// ComposeWith
//-----------------------------------------------------------------------------
HRPFilter* HRPMapFilter16::ComposeWith(const HRPFilter* pi_pFilter)
    {
    HPRECONDITION(pi_pFilter != 0);

    HRPFilter* pFilter = 0;

    // verify if the parameter is a convolution filter
    if(!pi_pFilter->IsCompatibleWith(HRPMapFilter16::CLASS_ID) ||
       !((HRPTypedFilter*)pi_pFilter)->GetFilterPixelType()->HasSamePixelInterpretation(*GetFilterPixelType()))
        {
        // if not, call the parent method
        pFilter = HRPFunctionFilter::ComposeWith(pi_pFilter);
        }
    else
        {
        HRPMapFilter16* pComposedFilter = (HRPMapFilter16*)this->Clone();
        HRPMapFilter16* pMapFilter      = (HRPMapFilter16*)pi_pFilter;

        // merge the tables
        for(size_t Channel = 0; Channel < m_Channels; Channel++)
            {
            for(size_t Index = 0; Index < 65536; Index++)
                {
                HASSERT( ((Channel << 16) + Index) < m_MapSize );
                pComposedFilter->m_pMap[(Channel << 16) + Index] = pMapFilter->m_pMap[(Channel << 16) + m_pMap[(Channel << 16) + Index]];
                }
            }
        pFilter = pComposedFilter;
        }
    return pFilter;
    }

//-----------------------------------------------------------------------------
// private
// Function
//-----------------------------------------------------------------------------
void HRPMapFilter16::Function( const void*  pi_pSrcRawData,
                               void*  po_pDestRawData,
                               uint32_t PixelsCount) const
    {
    HPRECONDITION(pi_pSrcRawData  != 0);
    HPRECONDITION(po_pDestRawData != 0);
    HPRECONDITION(PixelsCount > 0);

    uint16_t* pSrcComposite = (uint16_t*)pi_pSrcRawData;
    uint16_t* pDstComposite = (uint16_t*)po_pDestRawData;

    if (m_Channels == 4)
        {
        while(PixelsCount)
            {
            HASSERT(*pSrcComposite < m_MapSize);
            *pDstComposite = m_pMap[*pSrcComposite];               // Channel 0
            pDstComposite++;
            pSrcComposite++;

            HASSERT((65536U  + *pSrcComposite) < m_MapSize);
            *pDstComposite = m_pMap[65536U  + *pSrcComposite];    // Channel 1
            pDstComposite++;
            pSrcComposite++;

            HASSERT((131072U + *pSrcComposite) < m_MapSize);
            *pDstComposite = m_pMap[131072U + *pSrcComposite];        // Channel 2
            pDstComposite++;
            pSrcComposite++;

            // Skip the Alpha Channel.
            *pDstComposite = *pSrcComposite;
            pDstComposite++;
            pSrcComposite++;

            --PixelsCount;
            }
        }
    else if (m_Channels == 3)
        {
        while(PixelsCount)
            {
            HASSERT(*pSrcComposite < m_MapSize);
            *pDstComposite = m_pMap[*pSrcComposite];            // Channel 0
            pDstComposite++;
            pSrcComposite++;

            HASSERT((65536U  + *pSrcComposite) < m_MapSize);
            *pDstComposite = m_pMap[65536U  + *pSrcComposite];    // Channel 1
            pDstComposite++;
            pSrcComposite++;

            HASSERT((131072U + *pSrcComposite) < m_MapSize);
            *pDstComposite = m_pMap[131072U + *pSrcComposite];    // Channel 2
            pDstComposite++;
            pSrcComposite++;

            --PixelsCount;
            }
        }
    else if (m_Channels == 1)
        {
        while(PixelsCount)
            {
            HASSERT(*pSrcComposite < m_MapSize);

            *pDstComposite++ = m_pMap[*pSrcComposite++];
            --PixelsCount;
            }
        }
    HDEBUGCODE(else HASSERT(false););
    }

//-----------------------------------------------------------------------------
// public
// GetMap
//-----------------------------------------------------------------------------
uint16_t* HRPMapFilter16::GetMap(Byte pi_ChannelIndex)
    {
    HPRECONDITION(pi_ChannelIndex < m_Channels);

    return (m_pMap + ((uint32_t)pi_ChannelIndex << 16));
    }

//-----------------------------------------------------------------------------
// public
// SetMap
//-----------------------------------------------------------------------------
void HRPMapFilter16::SetMap(Byte pi_ChannelIndex, const uint16_t* pi_pMap)
    {
    HPRECONDITION(pi_pMap != 0);
    HPRECONDITION(pi_ChannelIndex < m_Channels);

    HASSERT((((uint32_t)pi_ChannelIndex << 16) + 65535U) < m_MapSize);

    // copy the map
    memcpy(m_pMap + ((uint32_t)pi_ChannelIndex << 16), pi_pMap, 65536U * sizeof(uint16_t));
    }

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// private
// DeepDelete
//-----------------------------------------------------------------------------
void HRPMapFilter16::DeepDelete()
    {
    delete m_pMap;
    m_pMap = 0;
    }

//-----------------------------------------------------------------------------
// private
// Initialize
//-----------------------------------------------------------------------------
void HRPMapFilter16::Initialize()
    {
    HPRECONDITION(m_Channels > 0);

    // create a map
    m_MapSize = (uint32_t)m_Channels << 16;
    m_pMap    = new uint16_t[m_MapSize];
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

int32_t HRPMapFilter16::GetMapSize() const
    {
    return m_MapSize;
    }
