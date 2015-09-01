//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrp/src/HRPOrFilter.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for class HRPOrFilter
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HRPOrFilter.h>

Byte HRPOrFilter::m_sMask[]       = { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };
bool HRPOrFilter::m_sTableUpdated  = false;
Byte HRPOrFilter::m_sTable[2][256];

//-----------------------------------------------------------------------------
// public
// Default constructor
//-----------------------------------------------------------------------------
HRPOrFilter::HRPOrFilter()
    : HRPFilter()
    {
    m_OnBit = true;

    InitObject();
    }

//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
HRPOrFilter::HRPOrFilter(bool pi_OnBit)
    : HRPFilter()
    {
    m_OnBit = pi_OnBit;

    InitObject();
    }

//-----------------------------------------------------------------------------
// public
// Copy constructor
//-----------------------------------------------------------------------------
HRPOrFilter::HRPOrFilter(const HRPOrFilter& pi_rFilter)
    : HRPFilter(pi_rFilter)
    {
    m_OnBit = pi_rFilter.m_OnBit;

    InitObject();
    }

//-----------------------------------------------------------------------------
// public
// destructor
//-----------------------------------------------------------------------------
HRPOrFilter::~HRPOrFilter()
    {
    }

//-----------------------------------------------------------------------------
// public
// Clone
//-----------------------------------------------------------------------------
HRPFilter* HRPOrFilter::Clone() const
    {
    return new HRPOrFilter(*this);
    }

//-----------------------------------------------------------------------------
// public
// Convert
//-----------------------------------------------------------------------------
void HRPOrFilter::Convert(  HRPPixelBuffer* pi_pInputBuffer,
                            HRPPixelBuffer* pio_pOutputBuffer)
    {
    HPRECONDITION(pi_pInputBuffer != 0);
    HPRECONDITION(pio_pOutputBuffer != 0);

    uint32_t LinesCount = pi_pInputBuffer->GetHeight();

    Byte* pSrc = (Byte*)pi_pInputBuffer->GetBufferPtr();
    Byte  SrcLinePaddingBytes = (Byte)pi_pInputBuffer->GetPaddingBytes();

    Byte* pDst = (Byte*)pio_pOutputBuffer->GetBufferPtr();
    Byte  DstLinePaddingBytes = (Byte)pio_pOutputBuffer->GetPaddingBytes();

    uint32_t BytesPerLine = (pi_pInputBuffer->GetWidth() + 7) / 8;
    uint32_t BytesCount;

    bool LastBitOn;
    bool NewLastBitOn;

    while(LinesCount != 0)
        {
        LastBitOn = false;

        BytesCount = BytesPerLine;

        while(BytesCount != 0)
            {
            *pDst = m_sTable[m_OnBit][*pSrc];

            NewLastBitOn = *pSrc & 0x01;

            if(LastBitOn)
                {
                if(m_OnBit)
                    *pDst |= 0x80;
                else
                    *pDst &= 0x7f;
                }

            if(NewLastBitOn == m_OnBit)
                LastBitOn = true;
            else
                LastBitOn = false;

            pSrc++;
            pDst++;

            BytesCount--;
            }

        pSrc += SrcLinePaddingBytes;
        pDst += DstLinePaddingBytes;

        LinesCount--;
        }
    }

//-----------------------------------------------------------------------------
// public
// UpdateTable
//-----------------------------------------------------------------------------
void HRPOrFilter::UpdateTable()
    {
    bool OnBitState;
    bool LastBit;
    bool NewBit;

    for(uint32_t OnBit = 0; OnBit < 2; OnBit++)
        {
        OnBitState = (OnBit == 1);

        for(uint32_t ByteIndex = 0; ByteIndex < 256; ByteIndex++)
            {
            LastBit = !OnBitState;

            for(uint32_t BitIndex = 0; BitIndex < 8; BitIndex++)
                {
                if((ByteIndex & m_sMask[BitIndex]) != 0)
                    NewBit = true;
                else
                    NewBit = false;

                if((LastBit == OnBitState) || (NewBit == OnBitState))
                    {
                    if(OnBitState)
                        m_sTable[OnBit][ByteIndex] |= m_sMask[BitIndex];
                    else
                        m_sTable[OnBit][ByteIndex] &= ~(m_sMask[BitIndex]);
                    }
                else
                    {

                    if(OnBitState)
                        m_sTable[OnBit][ByteIndex] &= ~(m_sMask[BitIndex]);
                    else
                        m_sTable[OnBit][ByteIndex] |= m_sMask[BitIndex];
                    }

                LastBit = NewBit;
                }
            }
        }

    m_sTableUpdated      = true;
    }

//-----------------------------------------------------------------------------
// private
// InitObject
//-----------------------------------------------------------------------------
void HRPOrFilter::InitObject()
    {
    if(!m_sTableUpdated)
        UpdateTable();
    }
