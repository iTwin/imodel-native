//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hcd/src/HCDPacket.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for class HCDPacket
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HCDPacket.h>
#include <Imagepp/all/h/HFCMonitor.h>
#include <ImagePP/all/h/HCDCodec.h>


//-----------------------------------------------------------------------------
// public
// Default constructor
//-----------------------------------------------------------------------------
HCDPacket::HCDPacket()
    {
    // default value (no buffer)

    m_pBuffer = 0;
    m_BufferSize = 0;
    m_DataSize = 0;
    m_BufferOwner = false;
    }

//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
HCDPacket::HCDPacket(Byte* pi_pBuffer,
                     size_t pi_BufferSize,
                     size_t pi_DataSize)
    {
    // set the attributes
    m_pBuffer = pi_pBuffer;
    m_BufferSize = pi_BufferSize;
    m_DataSize = pi_DataSize;

    // by default, the packet is not the owner of the buffer
    m_BufferOwner = false;
    }

//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
HCDPacket::HCDPacket(const HFCPtr<HCDCodec>& pi_pCodec,
                     Byte* pi_pBuffer,
                     size_t pi_BufferSize,
                     size_t pi_DataSize)
    {
    // set attributes
    m_pCodec     = pi_pCodec;
    m_pBuffer    = pi_pBuffer;
    m_BufferSize = pi_BufferSize;
    m_DataSize   = pi_DataSize;

    // by default, the packet is not the owner of the buffer
    m_BufferOwner = false;
    }

//-----------------------------------------------------------------------------
// public
// Copy constructor
//-----------------------------------------------------------------------------
HCDPacket::HCDPacket(const HCDPacket& pi_rObj)
    {
    if(pi_rObj.m_pCodec != 0)
        m_pCodec = (HCDCodec*)(pi_rObj.m_pCodec)->Clone();
    else
        m_pCodec = 0;

    // create a new buffer
    // the packet IS the owner of this buffer
    if (pi_rObj.m_DataSize > 0)
        {
        m_pBuffer = new Byte[pi_rObj.m_DataSize];
        memcpy (m_pBuffer, pi_rObj.m_pBuffer, pi_rObj.m_DataSize);
        }
    else
        {
        m_pBuffer = 0;
        }

    m_BufferSize  = pi_rObj.m_DataSize;
    m_DataSize    = pi_rObj.m_DataSize;
    m_BufferOwner = true;
    }

//-----------------------------------------------------------------------------
// protected
// HRABitmap::operator= - Assignment operator
//-----------------------------------------------------------------------------
HCDPacket& HCDPacket::operator=(const HCDPacket& pi_rPacket)
    {
    if (this != &pi_rPacket)
        {
        // delete the buffer if the packet is owner
        if((m_BufferOwner == true) && (m_pBuffer != 0))
            delete[] m_pBuffer;

        // Perform initialization of the object
        if(pi_rPacket.m_pCodec != 0)
            m_pCodec = (HCDCodec*)(pi_rPacket.m_pCodec)->Clone();
        else
            m_pCodec = 0;

        // create a new buffer
        // the packet IS the owner of this buffer
        if (pi_rPacket.m_DataSize > 0)
            {
            m_pBuffer = new Byte[pi_rPacket.m_DataSize];
            memcpy (m_pBuffer, pi_rPacket.m_pBuffer, pi_rPacket.m_DataSize);
            }
        else
            m_pBuffer = 0;
        m_BufferSize = pi_rPacket.m_DataSize;
        m_DataSize = pi_rPacket.m_DataSize;
        m_BufferOwner = true;
        }

    return(*this);
    }

//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HCDPacket::~HCDPacket()
    {
    // delete the buffer if the packet is owner
    if((m_BufferOwner == true) && (m_pBuffer != 0))
        delete[] m_pBuffer;
    }

//-----------------------------------------------------------------------------
// public
// Clone
//-----------------------------------------------------------------------------
HCDPacket* HCDPacket::Clone() const
    {
    // copy construct the current codec
    return new HCDPacket(*this);
    }

//-----------------------------------------------------------------------------
// public
// Compress
//-----------------------------------------------------------------------------
bool HCDPacket::Compress(HCDPacket* pio_pOutPacket) const
    {
    HASSERT(pio_pOutPacket->GetCodec() != 0);

    // test if the packet is the owner of the buffer
    if(pio_pOutPacket->m_BufferOwner)
        {
        // if yes, compress at the minimum subset level and realloc the output
        // buffer if required

        HFCPtr<HCDCodec> pCodec(pio_pOutPacket->m_pCodec);
        HFCMonitor Monitor(pCodec);

        // get and set the minimum subet size
        size_t MinimumSubsetSize = pCodec->GetMinimumSubsetSize();
        pCodec->SetSubsetSize(MinimumSubsetSize);

        // calculate the number of subsets in the input buffer
        size_t NumberOfSubsets = (m_DataSize + MinimumSubsetSize - 1) / MinimumSubsetSize;
        for(size_t SubsetIndex = 0; SubsetIndex < NumberOfSubsets; SubsetIndex++)
            {
            // test if there is enough memory for the following subset compression in
            // the output buffer
            if((pio_pOutPacket->m_BufferSize - pio_pOutPacket->m_DataSize) < pCodec->GetSubsetMaxCompressedSize())
                {
                // if no, reallocate the output buffer

                // calculate the number of extra bytes to allocate after the actual data size
                size_t ExtraBytes = (size_t)((NumberOfSubsets - (SubsetIndex + 1)) * MinimumSubsetSize *
                                             pCodec->GetEstimatedCompressionRatio() +
                                             pCodec->GetSubsetMaxCompressedSize());
                size_t NewBufferSize = pio_pOutPacket->m_DataSize + ExtraBytes;

                pio_pOutPacket->m_pBuffer = (Byte*)renew(pio_pOutPacket->m_pBuffer, pio_pOutPacket->m_BufferSize, NewBufferSize);
                pio_pOutPacket->m_BufferSize = NewBufferSize;
                }

            size_t DataSize = pCodec->CompressSubset( m_pBuffer + (SubsetIndex * MinimumSubsetSize),
                                                      MinimumSubsetSize,
                                                      pio_pOutPacket->m_pBuffer + pio_pOutPacket->m_DataSize,
                                                      pio_pOutPacket->m_BufferSize - pio_pOutPacket->m_DataSize);

            pio_pOutPacket->m_DataSize += DataSize;
            }
        }
    else
        {
        HFCMonitor Monitor(pio_pOutPacket->GetCodec());

        // compress the subset defined in the codec of the output packet
        size_t DataSize = (pio_pOutPacket->GetCodec())->CompressSubset(
                              GetBufferAddress(),
                              GetDataSize(),
                              pio_pOutPacket->GetBufferAddress(),
                              pio_pOutPacket->GetBufferSize());

        // set the data size of the output packet
        pio_pOutPacket->SetDataSize(DataSize);
        }

    // return an error if the data size is 0
    return (pio_pOutPacket->m_DataSize != 0);
    }

//-----------------------------------------------------------------------------
// public
// Decompress
//-----------------------------------------------------------------------------
bool HCDPacket::Decompress(HCDPacket* pio_pOutPacket)
    {
    HASSERT(m_pCodec != 0);
    HFCMonitor Monitor(m_pCodec);

    // decompress the subset defined in the codec of the current packet
    size_t DataSize = m_pCodec->DecompressSubset( GetBufferAddress(),
                                                  GetDataSize(),
                                                  pio_pOutPacket->GetBufferAddress(),
                                                  pio_pOutPacket->GetBufferSize());


    // set the data size of the output packet
    pio_pOutPacket->SetDataSize(DataSize);

    // set no codec in the output packet (data uncompressed)
    pio_pOutPacket->SetCodec(HFCPtr<HCDCodec>());

    // return an error if the data size is 0
    return (DataSize != 0);
    }

//-----------------------------------------------------------------------------
// public
// SetCodec
//-----------------------------------------------------------------------------
void HCDPacket::SetCodec(const HFCPtr<HCDCodec>& pi_pCodec)
    {
    // set the codec
    m_pCodec = pi_pCodec;
    }

//-----------------------------------------------------------------------------
// public
// SetDataSize
//-----------------------------------------------------------------------------
void HCDPacket::SetDataSize(size_t pi_DataSize)
    {
    // set the data size
    m_DataSize = pi_DataSize;
    }

//-----------------------------------------------------------------------------
// public
// SetBuffer
//-----------------------------------------------------------------------------
void HCDPacket::SetBuffer(void* pi_pBuffer, size_t pi_BufferSize)
    {
    HPRECONDITION(pi_pBuffer ? true : pi_BufferSize == 0);  // A NULL buffer should have a size of 0.

    // delete previous buffer if the packet is the owner of it
    if(m_BufferOwner)
        delete[] m_pBuffer;

    // set the buffer
    m_pBuffer = (Byte*)pi_pBuffer;
    m_BufferSize = pi_BufferSize;
    }

//-----------------------------------------------------------------------------
// public
// SetBufferOwnership
//-----------------------------------------------------------------------------
void HCDPacket::SetBufferOwnership(bool pi_Owner)
    {
    // indicate to the packet if it is the owner of the buffer
    m_BufferOwner = pi_Owner;
    }

//-----------------------------------------------------------------------------
// public
// ShrinkBufferToDataSize
//-----------------------------------------------------------------------------
void HCDPacket::ShrinkBufferToDataSize()
    {
    // shrink the buffer to the data size
    if(m_DataSize != m_BufferSize)
        {
        m_pBuffer = (Byte*)renew (m_pBuffer, m_BufferSize, m_DataSize);

        // set the buffer size to the data size
        m_BufferSize = m_DataSize;
        }
    }


//-----------------------------------------------------------------------------
// public
// IncreaseBuffer
//-----------------------------------------------------------------------------
void HCDPacket::ChangeBufferSize(size_t pi_NewBufferSize)
    {
    HPRECONDITION(pi_NewBufferSize > m_DataSize);

    // increase the buffer
    m_pBuffer = (Byte*)renew(m_pBuffer, m_BufferSize, pi_NewBufferSize);

    HASSERT(pi_NewBufferSize ? m_pBuffer != 0 : true);       // The renew failed bad things will happen.

    // set the new buffer size
    m_BufferSize = pi_NewBufferSize;
    }


//-----------------------------------------------------------------------------
// public inline method
// GetCodec
//-----------------------------------------------------------------------------
HFCPtr<HCDCodec>& HCDPacket::GetCodec() const
    {
    // return the codec
    return (HFCPtr<HCDCodec>&)m_pCodec;
    }