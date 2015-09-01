//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HCDPacketRLE.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Inline methods for class HCDPacketRLE
//-----------------------------------------------------------------------------

#include "HCDCodecHMRRLE1.h"

BEGIN_IMAGEPP_NAMESPACE
//-----------------------------------------------------------------------------
// public method
//-----------------------------------------------------------------------------
inline bool HCDPacketRLE::HasBufferOwnership() const
    {
    return m_BufferOwner;
    }

//-----------------------------------------------------------------------------
// public method
//-----------------------------------------------------------------------------
inline size_t HCDPacketRLE::GetLineDataSize(HUINTX pi_PosY) const
    {
    HPRECONDITION(pi_PosY < m_RunBuffers.size());
    return m_RunBuffers[pi_PosY].m_DataSize;
    }

//-----------------------------------------------------------------------------
// public method
//-----------------------------------------------------------------------------
inline size_t HCDPacketRLE::GetLineBufferSize(HUINTX pi_PosY) const
    {
    HPRECONDITION(pi_PosY < m_RunBuffers.size());
    return m_RunBuffers[pi_PosY].m_BufferSize;
    }

//-----------------------------------------------------------------------------
// public method
//-----------------------------------------------------------------------------
inline Byte* HCDPacketRLE::GetLineBuffer(HUINTX pi_PosY) const
    {
    HPRECONDITION(pi_PosY < m_RunBuffers.size());
    return m_RunBuffers[pi_PosY].m_pBuffer;
    }

//-----------------------------------------------------------------------------
// public
// GetBufferSize
//-----------------------------------------------------------------------------
inline size_t HCDPacketRLE::GetBufferSize() const
    {
    return m_TotalBufferSize;
    }

//-----------------------------------------------------------------------------
// public
// GetDataSize
//-----------------------------------------------------------------------------
inline size_t HCDPacketRLE::GetDataSize() const
    {
    return m_TotalDataSize;
    }

inline HCDPacketRLE::RLEScanlineGenerator* HCDPacketRLE::GetRLEScanlineGenerator (HUINTX    pi_PosY,
        bool     pi_State) const
    {
    HPRECONDITION(pi_PosY < m_pCodec->GetHeight());

    return new RLEScanlineGenerator((const unsigned short*)GetLineBuffer(pi_PosY),
                                    m_pCodec->GetWidth(),
                                    pi_State);
    }


//-----------------------------------------------------------------------------
// Inline methods for class HCDPacketRLE::RLEScanlineGenerator
//-----------------------------------------------------------------------------

inline HCDPacketRLE::RLEScanlineGenerator::RLEScanlineGenerator(const unsigned short*  pi_pRun,
                                                                HUINTX          pi_Width,
                                                                bool           pi_State)
    : m_pRun(pi_pRun),
      m_Width(pi_Width),
      m_State(pi_State),
      m_PixelCount(0)
    {
    }


inline HCDPacketRLE::RLEScanlineGenerator::~RLEScanlineGenerator()
    {

    }

inline HUINTX HCDPacketRLE::RLEScanlineGenerator::GetFirstScanline(HUINTX* po_pPosX)
    {
    HPRECONDITION(po_pPosX != 0);

    if (!m_State)
        {
        m_PixelCount = 0;
        m_Index = 0;
        }
    else
        {
        m_PixelCount = *m_pRun;
        m_Index = 1;
        }

    while (m_PixelCount < m_Width)
        {
        while (m_pRun[m_Index] == 0)
            ++m_Index;

        if ((m_Index & 1) == m_State)
            {
            *po_pPosX = m_PixelCount;
            return m_pRun[m_Index];
            }

        m_PixelCount += m_pRun[m_Index];
        ++m_Index;
        }

    return 0;
    }

inline HUINTX HCDPacketRLE::RLEScanlineGenerator::GetNextScanline(HUINTX* po_pPosX)
    {
    HPRECONDITION(po_pPosX != 0);

    if (m_PixelCount < m_Width)
        {
        m_PixelCount += m_pRun[m_Index];
        ++m_Index;

        while (m_PixelCount < m_Width)
            {
            while (m_pRun[m_Index] == 0)
                ++m_Index;

            if ((m_Index & 1) == m_State)
                {
                *po_pPosX = m_PixelCount;
                return m_pRun[m_Index];
                }

            m_PixelCount += m_pRun[m_Index];
            ++m_Index;
            }
        }

    return 0;
    }

inline HUINTX HCDPacketRLE::RLEScanlineGenerator::GetCurrentScanline(HUINTX* po_pPosX)
    {
    HPRECONDITION(po_pPosX != 0);

    if (m_PixelCount < m_Width)
        {
        *po_pPosX = m_PixelCount;
        return m_pRun[m_Index];
        }

    return 0;
    }

END_IMAGEPP_NAMESPACE