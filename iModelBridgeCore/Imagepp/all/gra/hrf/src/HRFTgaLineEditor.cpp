//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFTgaLineEditor.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class HRFTgaLineEditor
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HFCMath.h>
#include <Imagepp/all/h/HRFTgaLineEditor.h>
#include <Imagepp/all/h/HRFTgaFile.h>

#ifndef DIVROUNDUP
#define DIVROUNDUP(x,y) ((x % y) != 0 ? (x / y) + 1 : x / y)
#endif

#define TGA_RASTERFILE  static_cast<HRFTgaFile*>(GetRasterFile().GetPtr())

/**-----------------------------------------------------------------------------
 Constructor of the class HRFTgaLineEditor. This method initializes the intern
 attibutes for later use.

 @param pi_rpRasterFile A pointer to the associate raster file.
 @param pi_Page The number of the associate page descriptor.
 @param pi_Resolution The number of the associate resolution descriptor.
 @param pi_AccessMode The access and sharing modes for the file.
------------------------------------------------------------------------------*/
HRFTgaLineEditor::HRFTgaLineEditor(HFCPtr<HRFRasterFile> pi_rpRasterFile,
                                   uint32_t              pi_Page,
                                   unsigned short       pi_Resolution,
                                   HFCAccessMode         pi_AccessMode)
    : HRFResolutionEditor(pi_rpRasterFile,
                          pi_Page,
                          pi_Resolution,
                          pi_AccessMode)
    {
    HFCPtr<HRFTgaFile> pTgaFile = TGA_RASTERFILE;

    m_BytesPerLine = pTgaFile->m_pTgaFileHeader->m_ImageWidth * DIVROUNDUP(pTgaFile->m_pTgaFileHeader->m_PixelDepth, 8);
    m_RasterDataOffset = pTgaFile->GetRasterDataOffset();
    m_AlphaChannelBits = pTgaFile->m_pTgaFileHeader->m_ImageDescriptor & 0x0E;
    if (m_AlphaChannelBits != 0 && pTgaFile->m_pTgaExtentionArea && pTgaFile->m_pTgaExtentionArea->m_AttributesType != 3)
        m_AlphaChannelBits = 0;

    HPOSTCONDITION(m_AlphaChannelBits == 0 || m_AlphaChannelBits == 8);
    }

/**-----------------------------------------------------------------------------
 Public destructor for the class.
------------------------------------------------------------------------------*/
HRFTgaLineEditor::~HRFTgaLineEditor()
    {
    // This does nothing!
    }

/**-----------------------------------------------------------------------------
 Read an uncompressed block of pixels on this resolution.
 The block position must be specified and can be computed by the resolution
 descriptor.
 The @i{po_pData} must be allocated by the user of this function.
 The size must be > ${#GetBlockSizeInByte()}.

 @param pi_PosBlockX The X position of the block in the file.
 @param pi_PosBlockY The Y position of the block in the file.
 @param po_pData The buffer to be returned with the raster data.

 @return HSTATUS H_SUCCESS if the readint operation went right.
------------------------------------------------------------------------------*/
HSTATUS HRFTgaLineEditor::ReadBlock(uint64_t pi_PosBlockX,
                                    uint64_t pi_PosBlockY,
                                    Byte*  po_pData,
                                    HFCLockMonitor const* pi_pSisterFileLock)
    {
    HPRECONDITION (po_pData != 0);
    HPRECONDITION (pi_PosBlockX == 0);
    HPRECONDITION (pi_PosBlockY < GetResolutionDescriptor()->GetHeight());
    HPRECONDITION (m_AccessMode.m_HasReadAccess);

    HFCPtr<HRFTgaFile>  pTgaFile    = TGA_RASTERFILE;
    HSTATUS             Status = H_ERROR;
    Byte                BytePerPixel;
    unsigned short      Input;
    uint64_t            Offset;
    uint64_t            Line;
    uint32_t           i;
    uint32_t           j;
    HFCLockMonitor      SisterFileLock;

    if (pTgaFile->GetAccessMode().m_HasCreateAccess)
        {
        Status = H_NOT_FOUND;
        goto WRAPUP;
        }

    // If the lines are write from bottom to up, we convert the index of
    // the line to read from the end of the file.
    if ((pTgaFile->m_pTgaFileHeader->m_ImageDescriptor & 0x20) == 0)
        Line = (uint64_t)GetResolutionDescriptor()->GetHeight() - pi_PosBlockY - 1;
    else
        Line = pi_PosBlockY;

    // Caculate the offset of the first pixel of the line.
    Offset  = m_RasterDataOffset + (Line * m_BytesPerLine);

    // Lock the sister file if needed
    if(pi_pSisterFileLock == 0)
        {
        // Get lock and synch.
        AssignRasterFileLock(GetRasterFile(), SisterFileLock, true);
        pi_pSisterFileLock = &SisterFileLock;
        }

    if (pTgaFile->m_pTgaFile->GetCurrentPos() != Offset)
        pTgaFile->m_pTgaFile->SeekToPos(Offset);

    switch (pTgaFile->GetBitsPerPixel())
        {
        case 8 :
            if(pTgaFile->m_pTgaFile->Read(po_pData, m_BytesPerLine) != m_BytesPerLine)
                goto WRAPUP;    // H_ERROR
            break;
        case 15 :
        case 16 :
            // Convert V16B5G5R5 to V24R8G8B8
            m_pLineBuffer = new Byte[m_BytesPerLine];

            if(pTgaFile->m_pTgaFile->Read(m_pLineBuffer, m_BytesPerLine) != m_BytesPerLine)
                goto WRAPUP;

            for (i = 0, j = 0; j < GetResolutionDescriptor()->GetBlockSizeInBytes(); i++, j+=3)
                {
                Input = ((unsigned short*)m_pLineBuffer.get())[i];

                *po_pData++ = CONVERT_TO_BYTE(((Input & 0x7C00) >> 10) * 0xFF / 0x1F);
                *po_pData++ = CONVERT_TO_BYTE(((Input & 0x3E0) >> 5)   * 0xFF / 0x1F);
                *po_pData++ = CONVERT_TO_BYTE((Input & 0x1F)          * 0xFF / 0x1F);
                }
            break;
        case 24 :
        case 32 :
            // Convert V24B8G8R8 to V24R8G8B8 or convert V32B8G8R8A8 to V32R8G8B8A8
            m_pLineBuffer = new Byte[m_BytesPerLine];

            if(pTgaFile->m_pTgaFile->Read(m_pLineBuffer, m_BytesPerLine) != m_BytesPerLine)
                goto WRAPUP;

            BytePerPixel = (Byte)pTgaFile->GetBitsPerPixel() / 8;
            Byte* pOutPtr = po_pData;
            Byte* pBufferPtr = m_pLineBuffer;
            for (i = 0; i < m_BytesPerLine; pBufferPtr += BytePerPixel, i += BytePerPixel)
                {
                *pOutPtr++ = pBufferPtr[2];
                *pOutPtr++ = pBufferPtr[1];
                *pOutPtr++ = *pBufferPtr;

                if (BytePerPixel == 4)
                    if (m_AlphaChannelBits != 0) // special case for 32 bits and alpha
                        *pOutPtr++ = pBufferPtr[3];
                    else
                        *pOutPtr++ = 255;       // Fake the maximum opacity.
                }
            break;
        }

    // Unlock the sister file.
    SisterFileLock.ReleaseKey();

    Status = H_SUCCESS;

WRAPUP:
    return Status;
    }

/**-----------------------------------------------------------------------------
 Writes an uncompressed block of pixels on this resolution.
 The block position must be specified and can be computed by the resolution
 descripor.
 The value of @i{pi_pData} must be set by the user of this function.
 The size of the @i{pi_pData} buffer must be > ${#GetBlockSizeInByte()}.

 @param pi_PosBlockX The X position of the block in the file.
 @param pi_PosBlockY The Y position of the block in the file.
 @param pi_pData The buffer to be returned with the raster data.

 @return HSTATUS H_SUCCESS if the writing operation went right.
------------------------------------------------------------------------------*/
HSTATUS HRFTgaLineEditor::WriteBlock(uint64_t     pi_PosBlockX,
                                     uint64_t     pi_PosBlockY,
                                     const Byte*  pi_pData,
                                     HFCLockMonitor const* pi_pSisterFileLock)
    {
    HPRECONDITION (m_AccessMode.m_HasWriteAccess || m_AccessMode.m_HasCreateAccess);
    HPRECONDITION (pi_pData != 0);

    uint32_t                         i;
    Byte                            Swap;
    HSTATUS                         Status = H_ERROR;
    HArrayAutoPtr<unsigned short>           pLineBuffer;
    HFCPtr<HRFTgaFile>              pTgaFile   = TGA_RASTERFILE;
    HFCPtr<HRFResolutionDescriptor> pResolutionDescriptor = GetResolutionDescriptor();
    uint64_t                        Offset      = pTgaFile->GetRasterDataOffset();
    uint64_t                        Line;

    if (pTgaFile->GetAccessMode().m_HasWriteAccess  || pTgaFile->GetAccessMode().m_HasCreateAccess)
        {
        if ((pTgaFile->m_pTgaFileHeader->m_ImageDescriptor & 0x20) == 0)
            Line = (uint32_t)pResolutionDescriptor->GetHeight() - pi_PosBlockY - 1;
        else
            Line = pi_PosBlockY;

        // Caculate the offset of the first pixel of the line.
        Offset  = m_RasterDataOffset + (Line * m_BytesPerLine);

        // Lock the sister file if needed
        HFCLockMonitor SisterFileLock;
        if(pi_pSisterFileLock == 0)
            {
            // Get lock and synch.
            AssignRasterFileLock(GetRasterFile(), SisterFileLock, false);
            pi_pSisterFileLock = &SisterFileLock;
            }

        if (pTgaFile->m_pTgaFile->GetCurrentPos() != Offset)
            pTgaFile->m_pTgaFile->SeekToPos(Offset);

        // Convert and write the raster data to file
        switch (pTgaFile->GetBitsPerPixel())
            {
            case 8 :
                {
                uint32_t DataSize = pResolutionDescriptor->GetBytesPerBlockWidth();
                if(pTgaFile->m_pTgaFile->Write(pi_pData, DataSize) != DataSize)
                    goto WRAPUP;    // H_ERROR
                break;
                }

            case 16 :
                {
                // In edition mode only
                HASSERT (!GetAccessMode().m_HasCreateAccess);

                uint32_t LineWidthInByte = (uint32_t)pResolutionDescriptor->GetWidth() << 1;
                unsigned short pOutput;
                m_pLineBuffer = new Byte[LineWidthInByte];
                Byte* pBufferPtr = m_pLineBuffer;
                for (i = 0; i < pResolutionDescriptor->GetBytesPerBlockWidth(); i+=3)
                    {
                    pOutput = pi_pData[i] >> 3;
                    pOutput <<= 5;
                    pOutput |= pi_pData[i+1] >> 3;
                    pOutput <<= 5;
                    pOutput |= pi_pData[i+2] >> 3;

                    *(pBufferPtr++) = *(Byte*)&pOutput;
                    *(pBufferPtr++) = *(((Byte*)&pOutput)+1);
                    }

                if(pTgaFile->m_pTgaFile->Write(m_pLineBuffer, LineWidthInByte) != LineWidthInByte)
                    goto WRAPUP;    // H_ERROR
                break;
                }
            case 24 :
            case 32 :
                // Swap Red and Blue
                m_pLineBuffer = new Byte[pResolutionDescriptor->GetBytesPerBlockWidth()];
                memcpy (m_pLineBuffer, pi_pData, pResolutionDescriptor->GetBytesPerBlockWidth());
                for (i = 0; i < pResolutionDescriptor->GetBytesPerBlockWidth(); i += (pResolutionDescriptor->GetBitsPerPixel()/8))
                    {
                    Swap = m_pLineBuffer[i];
                    m_pLineBuffer[i] = m_pLineBuffer[i+2];
                    m_pLineBuffer[i+2] = Swap;
                    }

                uint32_t DataSize = pResolutionDescriptor->GetBytesPerBlockWidth();
                if(pTgaFile->m_pTgaFile->Write(m_pLineBuffer, DataSize) != DataSize)
                    goto WRAPUP;    // H_ERROR
                break;
            }

        // Increment the counters
        GetRasterFile()->SharingControlIncrementCount();

        // Unlock the sister file.
        SisterFileLock.ReleaseKey();
        }

    Status = H_SUCCESS;

WRAPUP:
    return Status;
    }
