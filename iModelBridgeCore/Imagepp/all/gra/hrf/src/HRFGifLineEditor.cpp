//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFGifLineEditor.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class HRFGifLineEditor
//---------------------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HRFGifFile.h>
#include <Imagepp/all/h/HCDCodecHMRGif.h>
#include <Imagepp/all/h/HRFGifLineEditor.h>
#include <Imagepp/all/h/HCDPacket.h>
#include <Imagepp/all/h/HRPPixelTypeI8R8G8B8A8.h>

#define DEFAULT_SIZE 1000

//-----------------------------------------------------------------------------
// public
// Construction
//-----------------------------------------------------------------------------
HRFGifLineEditor::HRFGifLineEditor(HFCPtr<HRFRasterFile> pi_rpRasterFile,
                                   uint32_t              pi_Page,
                                   unsigned short       pi_Resolution,
                                   HFCAccessMode         pi_AccessMode)
    : HRFResolutionEditor(pi_rpRasterFile,
                          pi_Page,
                          pi_Resolution,
                          pi_AccessMode)
    {
    m_pRasterFile        = static_cast<HRFGifFile*>(GetRasterFile().GetPtr());
    m_PosInFile          = m_pRasterFile->m_ListPageDataOffset[m_Page];

    uint32_t bitPerPixel;

    // Local color table is present.
    if (m_pRasterFile->m_ListGifGraphicBlock[pi_Page].ImageDescriptor.PackedField & 0x80)
        bitPerPixel = (m_pRasterFile->m_ListGifGraphicBlock[pi_Page].ImageDescriptor.PackedField & 0x07) + 1;
    else if (m_pRasterFile->m_GifHeader.PackedField & 0x80)
        bitPerPixel = (m_pRasterFile->m_GifHeader.PackedField & 0x07) + 1;
    else
        bitPerPixel = 8;

    m_pCodec = new HCDCodecHMRGIF((uint32_t)m_pResolutionDescriptor->GetWidth(),
                                  (uint32_t)m_pResolutionDescriptor->GetHeight(),
                                  bitPerPixel);

    m_pCodec->SetDecompressMinCodeSize(m_pRasterFile->m_ListPageDecompressMinCodeSize[pi_Page]);

    m_pCompressBuffer = 0;
    m_BufferSize      = (uint32_t)m_pResolutionDescriptor->GetWidth() + DEFAULT_SIZE; // Default size
    }

//-----------------------------------------------------------------------------
// public
// Destruction
//-----------------------------------------------------------------------------
HRFGifLineEditor::~HRFGifLineEditor()
    {
    if (m_pCompressBuffer)
        free(m_pCompressBuffer);
    }

//-----------------------------------------------------------------------------
// private
// ReadFromFile
//-----------------------------------------------------------------------------
bool HRFGifLineEditor::ReadFromFile(uint32_t&         pio_rLastValidIndex,
                                     uint32_t&         pio_rMaxBufferSize,
                                     HFCBinStream*  pi_pFile)
    {
    HPRECONDITION(GetRasterFile()->SharingControlIsLocked());

    Byte DataSize   = 0;
    uint32_t MaxSize    = pio_rMaxBufferSize;

    if (!m_EndDataReach)
        {
        if(pi_pFile->GetCurrentPos() != m_PosInFile)
            pi_pFile->SeekToPos(m_PosInFile);
        pi_pFile->Read(&DataSize ,1);
        m_PosInFile = (uint32_t)pi_pFile->GetCurrentPos();

        if (DataSize == 0)
            m_EndDataReach = true;
        }

    while (!m_EndDataReach && pio_rLastValidIndex <= MaxSize)
        {
        if (pio_rLastValidIndex+DataSize+1 >= pio_rMaxBufferSize)
            {
            m_pCompressBuffer = (Byte*) realloc(m_pCompressBuffer, pio_rMaxBufferSize + DataSize+1);
            pio_rMaxBufferSize += DataSize+1;
            }
        m_pCompressBuffer[pio_rLastValidIndex++] = DataSize;

        if(pi_pFile->GetCurrentPos() != m_PosInFile)
            pi_pFile->SeekToPos(m_PosInFile);
        pi_pFile->Read((m_pCompressBuffer + pio_rLastValidIndex), sizeof(Byte) * DataSize);
        m_PosInFile = (uint32_t)pi_pFile->GetCurrentPos();

        pio_rLastValidIndex += DataSize;

        if (!m_EndDataReach && pio_rLastValidIndex <= MaxSize)
            {
            if(pi_pFile->GetCurrentPos() != m_PosInFile)
                pi_pFile->SeekToPos(m_PosInFile);
            pi_pFile->Read(&DataSize, 1);
            m_PosInFile = (uint32_t)pi_pFile->GetCurrentPos();

            if (DataSize == 0)
                m_EndDataReach = true;
            }
        }
    return true;
    }


//-----------------------------------------------------------------------------
// public
// ReadBlock
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFGifLineEditor::ReadBlock(uint64_t  pi_PosBlockX,
                                    uint64_t  pi_PosBlockY,
                                    Byte*     po_pData,
                                    HFCLockMonitor const* pi_pSisterFileLock)
    {
    HPRECONDITION(m_AccessMode.m_HasReadAccess);
    HPRECONDITION(po_pData != 0);

    HSTATUS Status = H_SUCCESS;
    HCDPacket Uncompressed;

    size_t ImageCompressedIndex   = 0;
    int32_t BufferCompressedIndex  = 0;

    if (!GetRasterFile()->GetAccessMode().m_HasCreateAccess)
        {
        // Lock the sister file if needed
        HFCLockMonitor SisterFileLock;
        if(pi_pSisterFileLock == 0)
            {
            // Get lock and synch.
            AssignRasterFileLock(GetRasterFile(), SisterFileLock, true);
            pi_pSisterFileLock = &SisterFileLock;
            }

        // Restart the file at the begining when ask for a before block
        if ((m_pRasterFile->m_ListGifGraphicBlock[m_Page].ImageDescriptor.LineIndex > pi_PosBlockY))
            {
            m_pRasterFile->SaveGifFile(true);
            m_pRasterFile->Open();

            // Do this for all pages
            for (uint32_t Page=0; Page < m_pRasterFile->CountPages(); Page++)
                m_pRasterFile->m_ListGifGraphicBlock[Page].ImageDescriptor.LineIndex = 0;

            m_PosInFile = m_pRasterFile->m_ListPageDataOffset[m_Page];
            }

        if ((pi_PosBlockY == 0) || (m_pRasterFile->m_ListGifGraphicBlock[m_Page].ImageDescriptor.LineIndex == 0))
            {
            m_EndDataReach       = false;

            m_pCodec->SetSubset((uint32_t)m_pResolutionDescriptor->GetWidth(), 1);

            m_BufferSize        = (uint32_t)m_pResolutionDescriptor->GetWidth() + DEFAULT_SIZE;
            m_pCompressBuffer   = (Byte*) malloc(m_BufferSize);

            // Move  the file pointer to the begining of the actual data.
            m_pRasterFile->m_pGifFile->SeekToPos(m_pRasterFile->m_ListPageDataOffset[m_Page]);

            // Fill the buffer for first time.
            m_MaxOffsetInBuffer = 0;

            ReadFromFile(m_MaxOffsetInBuffer, m_BufferSize, m_pRasterFile->m_pGifFile);
            }


        // Move to the specified line
        while (m_pRasterFile->m_ListGifGraphicBlock[m_Page].ImageDescriptor.LineIndex <= pi_PosBlockY)
            {
            ImageCompressedIndex = m_pCodec->GetCompressedImageIndex();

            // we decompress the data in the output buffer
            HFCPtr<HCDPacket> pCompressed = new HCDPacket();

            Byte* pReturnBuffer = new Byte[m_BufferSize];
            memcpy(pReturnBuffer, m_pCompressBuffer, m_MaxOffsetInBuffer);

            // Set the current codec to the Packet.
            pCompressed->SetCodec((HFCPtr<HCDCodec> &) m_pCodec);
            pCompressed->SetBuffer(pReturnBuffer, m_BufferSize);

            // Set the compressed buffer.
            pCompressed->SetDataSize(m_MaxOffsetInBuffer);
            pCompressed->SetBufferOwnership(false);

            Uncompressed.SetBuffer(po_pData, m_pResolutionDescriptor->GetBlockWidth());

            pCompressed->Decompress(&Uncompressed);

            if (pi_PosBlockY+1 != m_pResolutionDescriptor->GetHeight())
                {
                HASSERT_X64(m_pCodec->GetCompressedImageIndex() - ImageCompressedIndex < ULONG_MAX);
                BufferCompressedIndex = (int32_t)(m_pCodec->GetCompressedImageIndex() - ImageCompressedIndex);

                HASSERT(BufferCompressedIndex >= 0);

                m_MaxOffsetInBuffer = m_MaxOffsetInBuffer - BufferCompressedIndex;

                memcpy(m_pCompressBuffer,
                       m_pCompressBuffer + BufferCompressedIndex,
                       m_MaxOffsetInBuffer);
                }

            if (!m_EndDataReach)
                {
                ReadFromFile(m_MaxOffsetInBuffer,
                             m_BufferSize,
                             m_pRasterFile->m_pGifFile);
                }
            m_pRasterFile->m_ListGifGraphicBlock[m_Page].ImageDescriptor.LineIndex++;

            if (m_pRasterFile->m_ListGifGraphicBlock[m_Page].ImageDescriptor.LineIndex == m_pResolutionDescriptor->GetHeight())
                {
                free(m_pCompressBuffer);
                m_pCompressBuffer = 0;
                }

            delete[] pReturnBuffer;
            }

        // Unlock the sister file
        SisterFileLock.ReleaseKey();

        }
    else
        Status = H_NOT_FOUND;

    return Status;
    }

//-----------------------------------------------------------------------------
// public
// WriteBlock
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS  HRFGifLineEditor::WriteBlock(uint64_t        pi_PosBlockX,
                                      uint64_t        pi_PosBlockY,
                                      const Byte*     pi_pData,
                                      HFCLockMonitor const* pi_pSisterFileLock)
    {
    HPRECONDITION (m_AccessMode.m_HasWriteAccess || m_AccessMode.m_HasCreateAccess);
    HPRECONDITION (m_pResolutionDescriptor->GetCodec() != 0);

    HSTATUS Status = H_SUCCESS;
    Byte* pReturnBuffer;

    // Compute Histogram if CreationMode and Transparency
    if (GetRasterFile()->GetAccessMode().m_HasCreateAccess &&
        m_pResolutionDescriptor->GetPixelType()->GetClassID() == HRPPixelTypeI8R8G8B8A8::CLASS_ID &&
        m_pRasterFile->m_pHistoCreationMode != 0)
        {
        for(uint32_t i=0; i<m_pResolutionDescriptor->GetBlockSizeInBytes(); ++i)
            ++(m_pRasterFile->m_pHistoCreationMode[pi_pData[i]]);
        }

    // we take for granted that there is compression
    // we compress the data
    HCDPacket Uncompressed((Byte*)pi_pData, m_pResolutionDescriptor->GetBlockSizeInBytes(), m_pResolutionDescriptor->GetBlockSizeInBytes());

    if (pi_PosBlockY == 0)
        {
        // Move  the file pointer to the begining of the actual data.
//        m_pRasterFile->SeekToPos(temp);
        // Set the codec subset.
        m_pCodec->SetSubset((uint32_t)m_pResolutionDescriptor->GetWidth(), 1);
        }

    // Lock the sister file if needed
    HFCLockMonitor SisterFileLock;
    if(pi_pSisterFileLock == 0)
        {
        // Get lock and synch.
        AssignRasterFileLock(GetRasterFile(), SisterFileLock, false);
        pi_pSisterFileLock = &SisterFileLock;
        }

    // Restart the file at the begining when the ask for a before block
    if (((!GetRasterFile()->GetAccessMode().m_HasCreateAccess) && (pi_PosBlockY == 0)) ||
        (m_pRasterFile->m_ListGifGraphicBlock[m_Page].ImageDescriptor.LineIndex > pi_PosBlockY))
        {
        m_pRasterFile->SaveGifFile(true);
        m_pRasterFile->Create();
        uint32_t Page;
        // Do this for all pages
        for (Page=0; Page < m_pRasterFile->CountPages(); Page++)
            m_pRasterFile->AssignStructTo(m_pRasterFile->GetPageDescriptor(Page));
        // Do this for all pages
        for (Page=0; Page < m_pRasterFile->CountPages(); Page++)
            m_pRasterFile->m_ListGifGraphicBlock[Page].ImageDescriptor.LineIndex = 0;

        }
    size_t MaxSubsetCompressed = m_pCodec->GetSubsetMaxCompressedSize();

    pReturnBuffer = (Byte*) malloc(MaxSubsetCompressed);

    HFCPtr<HCDPacket> pCompressed(new HCDPacket((HFCPtr<HCDCodec> &) m_pCodec, pReturnBuffer, MaxSubsetCompressed));

    pCompressed->SetBufferOwnership(false);

    Uncompressed.Compress(pCompressed);

    Status = WriteBlock(pi_PosBlockX, pi_PosBlockY, pCompressed);

    // Increment the sharing control modification counter.
    GetRasterFile()->SharingControlIncrementCount();

    // Unlock the sister file.
    SisterFileLock.ReleaseKey();

    free(pReturnBuffer);
    m_pRasterFile->m_ListGifGraphicBlock[m_Page].ImageDescriptor.LineIndex++;
    m_pRasterFile->SetDirty(true);

    return Status;
    }


//-----------------------------------------------------------------------------
// public
// WriteBlock
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFGifLineEditor::WriteBlock(uint64_t                 pi_PosBlockX,
                                     uint64_t                 pi_PosBlockY,
                                     const HFCPtr<HCDPacket>& pi_rpPacket,
                                     HFCLockMonitor const*    pi_pSisterFileLock)
    {
    HPRECONDITION(m_AccessMode.m_HasWriteAccess || m_AccessMode.m_HasCreateAccess);
    HPRECONDITION(pi_rpPacket != 0);

    HSTATUS Status = H_ERROR;

    // Lock the sister file if needed
    HFCLockMonitor SisterFileLock;
    if(pi_pSisterFileLock == 0)
        {
        // Get lock and synch.
        AssignRasterFileLock(GetRasterFile(), SisterFileLock, false);
        pi_pSisterFileLock = &SisterFileLock;
        }

    // Restart the file at the begining when the ask for a before block
    if ((!GetRasterFile()->GetAccessMode().m_HasCreateAccess) && (pi_PosBlockY == 0))
        {
        m_pRasterFile->SaveGifFile(true);
        m_pRasterFile->Create();
        uint32_t Page;
        // Do this for all pages
        for (Page=0; Page < m_pRasterFile->CountPages(); Page++)
            m_pRasterFile->AssignStructTo(m_pRasterFile->GetPageDescriptor(Page));
        // Do this for all pages
        for (Page=0; Page < m_pRasterFile->CountPages(); Page++)
            m_pRasterFile->m_ListGifGraphicBlock[Page].ImageDescriptor.LineIndex = 0;
        }

    if (pi_rpPacket->GetDataSize() != 0)
        if (m_pRasterFile->m_pGifFile->Write((pi_rpPacket->GetBufferAddress()), pi_rpPacket->GetDataSize()) != pi_rpPacket->GetDataSize())
            goto WRAPUP;

    // Increment the sharing control modification counter.
    GetRasterFile()->SharingControlIncrementCount();

    Status = H_SUCCESS;
    m_pRasterFile->SetDirty(true);

WRAPUP:
    // Unlock the sister file.
    SisterFileLock.ReleaseKey();

    return Status;
    }

//-----------------------------------------------------------------------------
// public
// OnSynchronizedSharingControl
//-----------------------------------------------------------------------------
void HRFGifLineEditor::OnSynchronizedSharingControl()
    {
    m_pRasterFile->SaveGifFile(true);
    m_pRasterFile->Open();

    // Do this for all pages
    for (uint32_t Page=0; Page < m_pRasterFile->CountPages(); Page++)
        m_pRasterFile->m_ListGifGraphicBlock[Page].ImageDescriptor.LineIndex = 0;

    m_PosInFile = m_pRasterFile->m_ListPageDataOffset[m_Page];
    }