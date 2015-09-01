//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hcd/src/HCDPacketRLE.cpp $
//:>    $RCSfile: HCDPacketRLE.cpp,v $
//:>   $Revision: 1.12 $
//:>       $Date: 2008/11/24 19:52:55 $
//:>     $Author: Ghislain.Tardif $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for class HCDPacketRLE
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HCDPacketRLE.h>
#include <Imagepp/all/h/HCDCodecHMRRLE1.h>

// We assume blacks are on 0 and whites on 1
#define ON_BLACK_STATE(bufferIndex) (!(bufferIndex & 0x00000001))       // block run is ON even number. 0,2,4,6...
#define RLE_RUN_LIMIT 32767

#ifdef __HMR_DEBUG
//#define DEBUG_RLE_PACKET
#endif


//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
HCDPacketRLE::HCDPacketRLE()
    : m_BufferOwner(false),
      m_TotalBufferSize(0),
      m_TotalDataSize(0)
    {
    }

//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
HCDPacketRLE::HCDPacketRLE(uint32_t pi_WidthPixels,
                           uint32_t pi_HeightPixels)
    : m_pCodec(new HCDCodecHMRRLE1(pi_WidthPixels,
                                   pi_HeightPixels)),
    m_BufferOwner(false),
    m_TotalBufferSize(0),
    m_TotalDataSize(0)
    {
    m_pCodec->SetSubset(pi_WidthPixels, 1);    // Subset by line.

    RunBuffer runBuffer = {0,0,0};

    m_RunBuffers.resize(m_pCodec->GetHeight(), runBuffer);
    }

//-----------------------------------------------------------------------------
// public
// destructor
//-----------------------------------------------------------------------------
HCDPacketRLE::~HCDPacketRLE()
    {
    if(HasBufferOwnership())
        FreeBuffers();
    }

//-----------------------------------------------------------------------------
// public
// Clone
//-----------------------------------------------------------------------------
HCDPacketRLE* HCDPacketRLE::Clone() const
    {
    // copy construct the current codec
    return new HCDPacketRLE(*this);
    }

//-----------------------------------------------------------------------------
// public
// SetBufferOwnership
//-----------------------------------------------------------------------------
void HCDPacketRLE::SetBufferOwnership(bool pi_Owner)
    {
    m_BufferOwner = pi_Owner;
    }

//-----------------------------------------------------------------------------
// public
// SetLineDataSize
//-----------------------------------------------------------------------------
void HCDPacketRLE::SetLineDataSize(HUINTX pi_PosY, size_t pi_DataSize)
    {
    HPRECONDITION(pi_PosY < m_RunBuffers.size());
    HPRECONDITION(pi_DataSize <= m_RunBuffers[pi_PosY].m_BufferSize);

    // Validate number of pixels in input buffer
#ifdef DEBUG_RLE_PACKET
    if(pi_DataSize)
        {
        unsigned short* pRLERuns = (unsigned short*)m_RunBuffers[pi_PosY].m_pBuffer;
        uint32_t PixelCountInLine = 0;
        for(size_t i=0; i < (pi_DataSize >> 1); ++i)
            PixelCountInLine += pRLERuns[i];

        HPRECONDITION(PixelCountInLine == m_pCodec->GetWidth());
        }
#endif

    m_TotalDataSize = (m_TotalDataSize - m_RunBuffers[pi_PosY].m_DataSize) + pi_DataSize;

    m_RunBuffers[pi_PosY].m_DataSize = pi_DataSize;

    HPOSTCONDITION(ValidateLineIntegrity(pi_PosY));
    }

//-----------------------------------------------------------------------------
// public
// SetLineBuffer
//
// Note : This method replace the line buffer by pi_pBuffer
//-----------------------------------------------------------------------------
void HCDPacketRLE::SetLineBuffer(HUINTX     pi_PosY,
                                 Byte*    pi_pBuffer,
                                 size_t     pi_BufferSize,
                                 size_t     pi_DataSize)
    {
    HPRECONDITION(pi_DataSize <= pi_BufferSize);
    HPRECONDITION(pi_pBuffer ? true : pi_BufferSize == 0);  // A NULL buffer should have a size of 0.
    HPRECONDITION(pi_PosY < m_RunBuffers.size());

    // Validate number of pixels in input buffer
#ifdef DEBUG_RLE_PACKET
    if(pi_pBuffer && pi_DataSize)
        {
        unsigned short* pRLERuns = (unsigned short*)pi_pBuffer;
        uint32_t PixelCountInLine = 0;
        for(size_t i=0; i < (pi_DataSize >> 1); ++i)
            PixelCountInLine += pRLERuns[i];

        HPRECONDITION(PixelCountInLine == m_pCodec->GetWidth());
        }
#endif

    RunBuffer& runBuffer = m_RunBuffers[pi_PosY];

    // delete previous buffer if the packet is the owner of it
    if (HasBufferOwnership())
        delete[] runBuffer.m_pBuffer;

    m_TotalBufferSize = (m_TotalBufferSize - runBuffer.m_BufferSize) + pi_BufferSize;
    m_TotalDataSize   = (m_TotalDataSize - runBuffer.m_DataSize) + pi_DataSize;

    // set the buffer
    runBuffer.m_pBuffer    = pi_pBuffer;
    runBuffer.m_DataSize   = pi_DataSize;
    runBuffer.m_BufferSize = pi_BufferSize;

    HPOSTCONDITION(ValidateLineIntegrity(pi_PosY));
    }

//-----------------------------------------------------------------------------
// public
// SetLineData
//
// Note : This method change data into the line buffer if the buffer size
//        is large enough to contain the new.
//        This method is use by MergeRun().
//-----------------------------------------------------------------------------
void HCDPacketRLE::SetLineData(HUINTX           pi_PosY,
                               const Byte*    pi_pData,
                               size_t           pi_DataSize)
    {
    HPRECONDITION(pi_DataSize > 0);
    HPRECONDITION(pi_PosY < m_RunBuffers.size());

    // Validate number of pixels in input buffer
#ifdef DEBUG_RLE_PACKET
    if (pi_pData && pi_DataSize)
        {
        unsigned short* pRLERuns = (unsigned short*)pi_pData;
        uint32_t PixelCountInLine = 0;
        for (size_t i=0; i < (pi_DataSize >> 1); ++i)
            PixelCountInLine += pRLERuns[i];

        HPRECONDITION(PixelCountInLine == m_pCodec->GetWidth());
        }
#endif

    RunBuffer& runBuffer = m_RunBuffers[pi_PosY];
    size_t BufferSize = runBuffer.m_BufferSize;

    // check if the buffer is large enough to contains the new data
    if (pi_DataSize > runBuffer.m_BufferSize)
        {
        if (HasBufferOwnership())
            delete[] runBuffer.m_pBuffer;

        runBuffer.m_pBuffer = new Byte[pi_DataSize];
        BufferSize = pi_DataSize;
        }


    m_TotalBufferSize = (m_TotalBufferSize - runBuffer.m_BufferSize) + BufferSize;
    m_TotalDataSize   = (m_TotalDataSize - runBuffer.m_DataSize) + pi_DataSize;

    // set data
    memcpy(runBuffer.m_pBuffer, pi_pData, pi_DataSize);
    runBuffer.m_DataSize   = pi_DataSize;
    runBuffer.m_BufferSize = BufferSize;

    HPOSTCONDITION(ValidateLineIntegrity(pi_PosY));
    }

//-----------------------------------------------------------------------------
// public
// ClearAll
//-----------------------------------------------------------------------------
void HCDPacketRLE::ClearAll()
    {
    for(size_t i=0; i < m_RunBuffers.size(); ++i)
        {
        SetLineBuffer((uint32_t)i,0,0,0);
        }
    }

//-----------------------------------------------------------------------------
// public
// FreeBuffers
//-----------------------------------------------------------------------------
void HCDPacketRLE::FreeBuffers()
    {
    HPRECONDITION(HasBufferOwnership());

    for(size_t i=0; i < m_RunBuffers.size(); ++i)
        {
        delete[] m_RunBuffers[i].m_pBuffer;
        m_RunBuffers[i].m_pBuffer = 0;
        m_RunBuffers[i].m_BufferSize = 0;
        m_RunBuffers[i].m_DataSize = 0;
        }

    m_TotalBufferSize = 0;
    m_TotalDataSize   = 0;
    }

//-----------------------------------------------------------------------------
// public
// GetCodec
//-----------------------------------------------------------------------------
HFCPtr<HCDCodecHMRRLE1> const& HCDPacketRLE::GetCodec() const
    {
    HPRECONDITION(m_pCodec != NULL);
    HPRECONDITION(m_pCodec->IsCompatibleWith(HCDCodecHMRRLE1::CLASS_ID));

    return m_pCodec;
    }


//-----------------------------------------------------------------------------
// private section
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// private
// Copy Constructor
//-----------------------------------------------------------------------------
HCDPacketRLE::HCDPacketRLE(const HCDPacketRLE& pi_rPacket)
    {
    if(pi_rPacket.m_pCodec != 0)
        m_pCodec = (HCDCodecHMRRLE1*)(pi_rPacket.m_pCodec)->Clone();
    else
        m_pCodec = 0;

    // create a new buffers, the packet IS the owner of these buffers
    m_BufferOwner = true;

    m_RunBuffers.reserve(pi_rPacket.m_RunBuffers.size());

    for(size_t i=0; i < pi_rPacket.m_RunBuffers.size(); ++i)
        {
        RunBuffer const& toCopyRunBuffer = pi_rPacket.m_RunBuffers[i];
        RunBuffer  newEntry = {0,0,0};
        if (toCopyRunBuffer.m_DataSize > 0)
            {
            newEntry.m_BufferSize  = toCopyRunBuffer.m_DataSize;
            newEntry.m_DataSize    = toCopyRunBuffer.m_DataSize;
            newEntry.m_pBuffer     = new Byte[toCopyRunBuffer.m_DataSize];
            memcpy (newEntry.m_pBuffer, toCopyRunBuffer.m_pBuffer, toCopyRunBuffer.m_DataSize);
            }
        else
            {
            newEntry.m_pBuffer = 0;
            newEntry.m_BufferSize  = toCopyRunBuffer.m_DataSize;
            newEntry.m_DataSize    = toCopyRunBuffer.m_DataSize;
            }
        m_RunBuffers.push_back(newEntry);
        }

    m_TotalBufferSize = pi_rPacket.m_TotalBufferSize;
    m_TotalDataSize   = pi_rPacket.m_TotalDataSize;
    }

//-----------------------------------------------------------------------------
// private
// operator=
//-----------------------------------------------------------------------------
HCDPacketRLE& HCDPacketRLE::operator=(const HCDPacketRLE& pi_rPacket)
    {
    if (this != &pi_rPacket)
        {
        // delete the buffer if the packet is owner
        if(HasBufferOwnership())
            FreeBuffers();

        if(pi_rPacket.m_pCodec != 0)
            m_pCodec = (HCDCodecHMRRLE1*)(pi_rPacket.m_pCodec)->Clone();
        else
            m_pCodec = 0;

        // create a new buffers, the packet IS the owner of these buffers
        m_BufferOwner = true;

        m_RunBuffers.reserve(pi_rPacket.m_RunBuffers.size());

        for(size_t i=0; i < pi_rPacket.m_RunBuffers.size(); ++i)
            {
            RunBuffer const& toCopyRunBuffer = pi_rPacket.m_RunBuffers[i];

            RunBuffer& newEntry = m_RunBuffers[i];
            if (toCopyRunBuffer.m_BufferSize != 0)
                {
                newEntry.m_BufferSize  = toCopyRunBuffer.m_BufferSize;
                newEntry.m_DataSize    = toCopyRunBuffer.m_DataSize;
                newEntry.m_pBuffer     = new Byte[toCopyRunBuffer.m_DataSize];

                if(toCopyRunBuffer.m_DataSize > 0)
                    memcpy (newEntry.m_pBuffer, toCopyRunBuffer.m_pBuffer, toCopyRunBuffer.m_DataSize);
                }
            else
                {
                newEntry.m_pBuffer     = 0;
                newEntry.m_BufferSize  = 0;
                newEntry.m_DataSize    = 0;
                }
            }

        m_TotalBufferSize = pi_rPacket.m_TotalBufferSize;
        m_TotalDataSize   = pi_rPacket.m_TotalDataSize;
        }

    return(*this);
    }

//-----------------------------------------------------------------------------
// private
// ValidateLineIntegrity
//-----------------------------------------------------------------------------
bool HCDPacketRLE::ValidateLineIntegrity(HUINTX pi_NoLine) const
    {
    HPRECONDITION(pi_NoLine < m_pCodec->GetHeight());

    RunBuffer const& runBuffer = m_RunBuffers[pi_NoLine];

    unsigned short const* pRleBuffer = (unsigned short const*)runBuffer.m_pBuffer;

    if(pRleBuffer == 0 || runBuffer.m_DataSize == 0)
        return true; // Consider an empty buffer or runs as valid.

    size_t RunCount = runBuffer.m_DataSize >> 1;

    // Must have an even number of entries so it end on black.
    if(!ON_BLACK_STATE(RunCount-1))        // index 0, 2, 4 are black.
        return false;

    uint32_t Width = 0;

    bool previsousWasRleLimit = false;
    for(size_t i=0; i < RunCount; ++i)
        {
#ifdef __CHECK_BADLY_ENCODED_BUT_VALID_RLE_RUNS
        // skip first and last, others entries that are 0 must have a previous of RLE_RUN_LIMIT
        if(i > 0 && i < RunCount-1 && (pRleBuffer[i] == 0 && !previsousWasRleLimit))
            return false;   // valid but badly encoded.
#endif

        previsousWasRleLimit = (RLE_RUN_LIMIT == pRleBuffer[i]);

        Width += pRleBuffer[i];
        }

    return Width == m_pCodec->GetWidth();
    }

//-----------------------------------------------------------------------------
// private
// ValidateDataIntegrity
//-----------------------------------------------------------------------------
bool HCDPacketRLE::ValidateDataIntegrity() const
    {
    if(m_RunBuffers.size() != m_pCodec->GetHeight())
        return false;

    size_t BufferSize = 0;
    size_t DataSize = 0;

    for(uint32_t Line=0; Line < m_RunBuffers.size(); ++Line)
        {
        BufferSize += m_RunBuffers[Line].m_BufferSize;
        DataSize += m_RunBuffers[Line].m_DataSize;

        if(!ValidateLineIntegrity(Line))
            return false;
        }

    if(m_TotalBufferSize != BufferSize || m_TotalDataSize != DataSize)
        return false;

    return true;
    }
