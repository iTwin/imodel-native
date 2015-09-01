//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFBmpCompressLineEditor.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------
// Class HRFBmpCompressLineEditor
//---------------------------------------------------------------------------------------

//***********************************
//* This editor is not used for now *
//***********************************

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HRFBmpFile.h>
#include <Imagepp/all/h/HCDPacket.h>
#include <Imagepp/all/h/HCDCodecBMPRLE8.h>
#include <Imagepp/all/h/HRFBmpCompressLineEditor.h>

//-----------------------------------------------------------------------------
// public
// Construction
//-----------------------------------------------------------------------------
HRFBmpCompressLineEditor::HRFBmpCompressLineEditor(HFCPtr<HRFRasterFile> pi_rpRasterFile,
                                                   uint32_t              pi_Page,
                                                   unsigned short       pi_Resolution,
                                                   HFCAccessMode         pi_AccessMode)
    : HRFResolutionEditor(pi_rpRasterFile,
                          pi_Page,
                          pi_Resolution,
                          pi_AccessMode)
    {
    m_pRasterFile        = (HFCPtr<HRFBmpFile>&)GetRasterFile();

    // Position should be the first valid data in file.
    m_PosInFile          = m_pRasterFile->m_BmpFileHeader.m_OffBitsToData;
    m_NumberOfLineRead   = 0;

    m_pCodec = new HCDCodecBMPRLE8();

    m_pCodec->SetDimensions(GetResolutionDescriptor()->GetWidth(),
                            GetResolutionDescriptor()->GetHeight());

    m_pCodec->SetLinePaddingBits(m_pRasterFile->m_PaddingBitsPerRow);
    m_pCodec->SetSubset(GetResolutionDescriptor()->GetWidth(), 1);

    m_BufferSize        = 32768;
    m_MininumSizeToAdd  = 1024;
    m_pCompressBuffer   = (Byte*)malloc(m_BufferSize*sizeof(Byte));

    // At first position we put offset to data in file.
    m_pLinesOffsetBuffer = m_pRasterFile->m_pLinesOffsetBuffer;
    m_pLinesOffsetBuffer[0] = 0;

    // Fill the buffer for first time.
    if(m_pRasterFile->m_pBmpFile->GetCurrentPos() != m_PosInFile)
        m_pRasterFile->m_pBmpFile->SeekToPos(m_PosInFile);

    m_pRasterFile->m_pBmpFile->Read(m_pCompressBuffer, sizeof(Byte) * m_BufferSize);
    m_PosInFile = m_pRasterFile->m_pBmpFile->GetCurrentPos();

    m_MaxOffsetInBuffer = m_BufferSize-1;
    }

//-----------------------------------------------------------------------------
// public
// Destruction
//-----------------------------------------------------------------------------
HRFBmpCompressLineEditor::~HRFBmpCompressLineEditor()
    {
    free(m_pCompressBuffer);
    }

//-----------------------------------------------------------------------------
// public
// ReadBlock
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFBmpCompressLineEditor::ReadBlock(uint64_t           pi_PosBlockX,
                                            uint64_t           pi_PosBlockY,
                                            HFCPtr<HCDPacket>& po_rpPacket)
    {
    HPRECONDITION(m_AccessMode.m_HasReadAccess);
    HPRECONDITION(po_rpPacket != 0);

    HSTATUS Status = H_SUCCESS;

    uint32_t NumberOfPixelRead = 0;
    uint32_t IndexInBuffer     = 0;

    if (!GetRasterFile()->GetAccessMode().m_HasCreateAccess)
        {
        if (pi_PosBlockY < m_NumberOfLineRead)
            {
            // Set the current codec to the Packet.
            po_rpPacket->SetCodec((HFCPtr<HCDCodec> &) m_pCodec);

            // Test if there is a buffer already defined
            if(po_rpPacket->GetBufferSize() == 0)
                {
                // Copy compress line data to the Packet
                Byte* pReturnBytes = new Byte[m_pLinesOffsetBuffer[pi_PosBlockY+1] -
                                                  m_pLinesOffsetBuffer[pi_PosBlockY]];

                int32_t oldFilePtr = m_pRasterFile->m_pBmpFile->GetCurrentPos();

                m_pRasterFile->m_pBmpFile->SeekToPos(m_pRasterFile->m_BmpFileHeader.m_OffBitsToData +
                                                     m_pLinesOffsetBuffer[pi_PosBlockY]);
                m_pRasterFile->m_pBmpFile->Read(pReturnBytes, m_pLinesOffsetBuffer[pi_PosBlockY+1] -
                                                m_pLinesOffsetBuffer[pi_PosBlockY]);

                // Reset the file pointer to where it were.
                m_pRasterFile->m_pBmpFile->SeekToPos(oldFilePtr);

                po_rpPacket->SetBuffer(pReturnBytes,  m_pLinesOffsetBuffer[pi_PosBlockY+1]);
                po_rpPacket->SetBufferOwnership(true);
                }
            else
                {
                HASSERT(IndexInBuffer+1 <= po_rpPacket->GetBufferSize());

                m_pRasterFile->m_pBmpFile->SeekToPos(m_pRasterFile->m_BmpFileHeader.m_OffBitsToData + m_pLinesOffsetBuffer[pi_PosBlockY]);
                m_pRasterFile->m_pBmpFile->Read(po_rpPacket->GetBufferAddress(),
                                                m_pLinesOffsetBuffer[pi_PosBlockY+1] - m_pLinesOffsetBuffer[pi_PosBlockY]);
                // Reset the file pointer to where it were.
                m_pRasterFile->m_pBmpFile->SeekToPos(m_pRasterFile->m_BmpFileHeader.m_OffBitsToData + m_pLinesOffsetBuffer[m_NumberOfLineRead]);
                }
            po_rpPacket->SetDataSize(m_pLinesOffsetBuffer[pi_PosBlockY+1]);

            }
        else
            {
            while (m_NumberOfLineRead <= pi_PosBlockY)
                {
                // Move the rest of the line to the begining of the buffer
                m_MaxOffsetInBuffer -= IndexInBuffer;

                memcpy(m_pCompressBuffer,
                       m_pCompressBuffer + IndexInBuffer,
                       m_MaxOffsetInBuffer+1);

                NumberOfPixelRead = 0;
                IndexInBuffer     = 0;

                while (NumberOfPixelRead < GetResolutionDescriptor()->GetWidth())
                    {
                    // Resize the buffer when to small to check the data.
                    // The IndexInBuffer + the next index must be include in the buffer.
                    if ((IndexInBuffer+1) > (m_BufferSize-1))
                        {
                        // Convert Buffer size in index.
                        uint32_t SizeToAdd = m_MininumSizeToAdd + ((IndexInBuffer+1) - (m_BufferSize-1));
                        m_pCompressBuffer = (Byte*) realloc(m_pCompressBuffer, (m_BufferSize + SizeToAdd)*sizeof(Byte));
                        m_BufferSize = m_BufferSize + SizeToAdd;
                        }
                    // Full the buffer when to small to check the data.
                    // The IndexInBuffer + the next index must be include in the buffer.
                    if (IndexInBuffer+1 > m_MaxOffsetInBuffer)
                        {
                        // Full the buffer after the MaxOffsetInBuffer.
                        if(m_pRasterFile->m_pBmpFile->GetCurrentPos() != m_PosInFile)
                            m_pRasterFile->m_pBmpFile->SeekToPos(m_PosInFile);

                        m_pRasterFile->m_pBmpFile->Read((m_pCompressBuffer + m_MaxOffsetInBuffer + 1), sizeof(Byte) *
                                                        (m_BufferSize   - (m_MaxOffsetInBuffer+1)));
                        m_PosInFile = m_pRasterFile->m_pBmpFile->GetCurrentPos();

                        m_MaxOffsetInBuffer = m_BufferSize-1;
                        }

                    if (m_pCompressBuffer[IndexInBuffer] != 0x00)
                        {
                        // We are in encode mode and Buffer[IndexBuffer] contains
                        // the number of pixels to be drawn using the color indexe
                        // in the next byte
                        NumberOfPixelRead = NumberOfPixelRead + m_pCompressBuffer[IndexInBuffer];

                        // The next byte that contains usedfull information is :
                        IndexInBuffer     = IndexInBuffer + 2;
                        }

                    // We know that we are in absulute mode or that have an escape code
                    // of the encode mode
                    else if (m_pCompressBuffer[IndexInBuffer+1] >= 0x03)
                        {
                        // We know that we are in absolute mode and Buffer[IndexBuffer+1]
                        // contains the number of not compress byte that follow.
                        // Each run must be aligned on a word boundary.

                        NumberOfPixelRead  = NumberOfPixelRead + m_pCompressBuffer[IndexInBuffer + 1];
                        IndexInBuffer      = IndexInBuffer + 2 +
                                             (((m_pCompressBuffer[IndexInBuffer + 1] +1) / 2) * 2);
                        }
                    else if (m_pCompressBuffer[IndexInBuffer+1] == 0x02)
                        {
                        // We know that we have an delta escape code of the encode mode.
                        // And we by passe those code
                        IndexInBuffer = IndexInBuffer + 4;
                        }
                    else
                        {
                        // We know that we have an escape code of the encode mode.
                        // And we by passe those code
                        IndexInBuffer = IndexInBuffer + 2;
                        }
                    }
                HASSERT(NumberOfPixelRead == GetResolutionDescriptor()->GetWidth());

                m_NumberOfLineRead++;
                m_pLinesOffsetBuffer[m_NumberOfLineRead] = IndexInBuffer + m_pLinesOffsetBuffer[m_NumberOfLineRead - 1];

                // Resize the buffer when to small for the codec.
                // The IndexInBuffer must be include in the buffer.
                if (IndexInBuffer > (m_BufferSize-1))
                    {
                    // Convert Buffer size in index.
                    uint32_t SizeToAdd = m_MininumSizeToAdd + (IndexInBuffer - (m_BufferSize-1));
                    m_pCompressBuffer = (Byte*) realloc(m_pCompressBuffer, (m_BufferSize + SizeToAdd)*sizeof(Byte));
                    m_BufferSize = m_BufferSize + SizeToAdd;
                    }

                // Full the buffer when to small to check the data.
                if (IndexInBuffer > m_MaxOffsetInBuffer)
                    {
                    // Full the buffer after the MaxOffsetInBuffer
                    if(m_pRasterFile->m_pBmpFile->GetCurrentPos() != m_PosInFile)
                        m_pRasterFile->m_pBmpFile->SeekToPos(m_PosInFile);

                    m_pRasterFile->m_pBmpFile->Read((m_pCompressBuffer + m_MaxOffsetInBuffer + 1), sizeof(Byte) *
                                                    (m_BufferSize   - (m_MaxOffsetInBuffer+1)));

                    m_PosInFile         = m_pRasterFile->m_pBmpFile->GetCurrentPos();
                    m_MaxOffsetInBuffer = m_BufferSize-1;

                    }
                }
            // We readed a entire line so we retrun it and move the rest of the buffer
            // to the begining.

            // Set the current codec to the Packet.
            po_rpPacket->SetCodec((HFCPtr<HCDCodec> &) m_pCodec);

            // Test if there is a buffer already defined
            if(po_rpPacket->GetBufferSize() == 0)
                {
                // Copy compress line data to the Packet
                Byte* pReturnBytes = new Byte[IndexInBuffer];
                memcpy(pReturnBytes, m_pCompressBuffer, IndexInBuffer);

                po_rpPacket->SetBuffer(pReturnBytes, IndexInBuffer);
                po_rpPacket->SetBufferOwnership(true);
                po_rpPacket->SetDataSize(IndexInBuffer);
                }
            else
                {
                HASSERT(IndexInBuffer+1 <= po_rpPacket->GetBufferSize());
                po_rpPacket->SetDataSize(IndexInBuffer+1);
                memcpy(po_rpPacket->GetBufferAddress(), m_pCompressBuffer, IndexInBuffer+1);
                }

            // Move the rest of the line to the begining of the buffer
            m_MaxOffsetInBuffer -= IndexInBuffer;

            memcpy(m_pCompressBuffer,
                   m_pCompressBuffer + IndexInBuffer,
                   m_MaxOffsetInBuffer+1);
            }
        }
    else
        Status = H_ERROR;

    return Status;
    }


//-----------------------------------------------------------------------------
// public
// WriteBlock
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS  HRFBmpCompressLineEditor::WriteBlock(uint64_t     pi_PosBlockX,
                                              uint64_t     pi_PosBlockY,
                                              const Byte*  pi_pData)
    {
    HPRECONDITION (m_AccessMode.m_HasWriteAccess || m_AccessMode.m_HasCreateAccess);
    HPRECONDITION (m_pResolutionDescriptor->GetCodec() != 0);

    HSTATUS Status = H_SUCCESS;

    // Restart the file at the begining when the ask for a before block
    if (((!GetRasterFile()->GetAccessMode().m_HasCreateAccess) && (pi_PosBlockY == 0)) ||
        (m_NumberOfLineRead > pi_PosBlockY))
        {
        m_pRasterFile->Close();
        m_pRasterFile->Open();
        m_NumberOfLineRead = 0;
        }

    // we take for granted that there is compression
    // we compress the data
    HCDPacket Uncompressed((Byte*)pi_pData, m_pResolutionDescriptor->GetBlockSize(), m_pResolutionDescriptor->GetBlockSize());

    // Assign a default Codec
    HFCPtr<HCDCodec> pCodec = (HCDCodec*)m_pResolutionDescriptor->GetCodec()->Clone();

    HCDCodecImage::SetCodecForImage(pCodec,
                                    m_pResolutionDescriptor->GetBlockWidth(),
                                    m_pResolutionDescriptor->GetBlockHeight(),
                                    m_pResolutionDescriptor->GetBitsPerPixel(),
                                    32);  // Padding per width for BMP

    size_t MaxSubsetCompressed = pCodec->GetSubsetMaxCompressedSize();

    HFCPtr<HCDPacket> pCompressed(new HCDPacket(pCodec, new Byte[MaxSubsetCompressed], MaxSubsetCompressed));

    pCompressed->SetBufferOwnership(true);

    Uncompressed.Compress(pCompressed);

    WriteBlock(pi_PosBlockX, pi_PosBlockY, pCompressed);

    return Status;
    }

//-----------------------------------------------------------------------------
// public
// WriteBlock
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFBmpCompressLineEditor::WriteBlock(uint64_t                 pi_PosBlockX,
                                             uint64_t                 pi_PosBlockY,
                                             const HFCPtr<HCDPacket>& pi_rpPacket)
    {
    HPRECONDITION(m_AccessMode.m_HasWriteAccess || m_AccessMode.m_HasCreateAccess);
    HPRECONDITION(pi_rpPacket != 0);

    HSTATUS Status = H_SUCCESS;

    // Restart the file at the begining when the ask for a before block
    if (((!GetRasterFile()->GetAccessMode().m_HasCreateAccess) && (pi_PosBlockY == 0)) ||
        (m_NumberOfLineRead > pi_PosBlockY))
        {
        m_pRasterFile->Close();
        m_pRasterFile->Open();
        m_NumberOfLineRead = 0;
        }

    if (pi_PosBlockY == 0)
        m_pRasterFile->m_pBmpFile->SeekToPos(m_pRasterFile->m_BmpFileHeader.m_OffBitsToData);

    m_pRasterFile->m_pBmpFile->Write(pi_rpPacket->GetBufferAddress(), sizeof(Byte) * pi_rpPacket->GetDataSize());

    return Status;
    }