//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFPictLineEditor.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class HRFPictLineEditor
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HRFPictFile.h>
#include <Imagepp/all/h/HRFPictLineEditor.h>

#include <Imagepp/all/h/HCDPacket.h>
#include <Imagepp/all/h/HCDCodecHMRPackBits.h>
#include <Imagepp/all/h/HCDCodecIdentity.h>

#include <Imagepp/all/h/HRPPixelTypeV24R8G8B8.h>

//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// public
// Construction
//-----------------------------------------------------------------------------
HRFPictLineEditor::HRFPictLineEditor(HFCPtr<HRFRasterFile> pi_rpRasterFile,
                                     uint32_t              pi_Page,
                                     unsigned short       pi_Resolution,
                                     HFCAccessMode         pi_AccessMode)
    :   HRFResolutionEditor(    pi_rpRasterFile,
                                pi_Page,
                                pi_Resolution,
                                pi_AccessMode),
    m_ReadPosInFile(0),
    m_WritePosInFile(0),
    m_CurrentWriteLine(0),
    m_CurrentReadLine(0),

    m_BytesPerBlockWidth((uint32_t)GetResolutionDescriptor()->GetBytesPerBlockWidth()),
    m_ImageWidth((uint32_t)GetResolutionDescriptor()->GetWidth()),
    m_ImageHeigth((uint32_t)GetResolutionDescriptor()->GetHeight()),

    m_ReorganizeLineColorRepresentation(GetResolutionDescriptor()->GetPixelType()->IsCompatibleWith(HRPPixelTypeV24R8G8B8::CLASS_ID))
    {
    m_pRasterFile = static_cast<HRFPictFile*>(GetRasterFile().GetPtr());

    m_RowBytes          = m_pRasterFile->m_DataHeader.m_PackBitHeader.m_RowBytes & 0x7FFF;

    // NOTE: This buffer could probably be shorter. We got to find a fail-safe rule that can encompasses all the cases.
    m_LineBufferSize    = (uint32_t)(m_BytesPerBlockWidth * 1.5L);

    m_pReadLineBuffer   = new Byte[m_LineBufferSize];

    // Initialize the write buffers only if the file has Create or Write access
    if (m_pRasterFile->m_pPictFile->GetAccessMode().m_HasCreateAccess || m_pRasterFile->m_pPictFile->GetAccessMode().m_HasWriteAccess)
        {
        m_pWriteLineBuffer  = new Byte[m_LineBufferSize];
        m_pWriteLineBuffer2 = new Byte[m_LineBufferSize];
        }


    HFCPtr<HRPPixelType> pPixelType(GetResolutionDescriptor()->GetPixelType());

    // Setup the packbits codec
    m_pCodec = new HCDCodecHMRPackBits();

    // RGB support
    if (pPixelType->IsCompatibleWith(HRPPixelTypeV24R8G8B8::CLASS_ID))
        {
        // Bytes per block is divisible by 3 without remain
        HASSERT((m_BytesPerBlockWidth % 3) == 0);

        // We will pack the bytes of the line per color. Divide the current line in 3 subsets, one for each color.
        m_SubsetWidth   = m_BytesPerBlockWidth / 3;
        m_SubsetHeight  = GetResolutionDescriptor()->GetBlockHeight() * 3;
        }
    // Palette support
    else
        {
        //This file has a palette, we need only one subset
        m_SubsetWidth   = m_BytesPerBlockWidth;
        m_SubsetHeight  = GetResolutionDescriptor()->GetBlockHeight();
        }

    m_pCodec->SetSubset(m_SubsetWidth, m_SubsetHeight, 0, 0);

    // The codec has no need to know our pixel size. Let it think it is manipulating bytes instead.
    m_pCodec->SetBitsPerPixel(8);
    m_pCodec->SetLinePaddingBits(0);
    }

//-----------------------------------------------------------------------------
// public
// Destruction
//-----------------------------------------------------------------------------
HRFPictLineEditor::~HRFPictLineEditor()
    {
    }

//-----------------------------------------------------------------------------
// public
// ReadBlock
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFPictLineEditor::ReadBlock(uint64_t                pi_PosBlockX,
                                     uint64_t                pi_PosBlockY,
                                     Byte*                   po_pData,
                                     HFCLockMonitor const*   pi_pSisterFileLock)
    {

    HPRECONDITION (m_AccessMode.m_HasReadAccess);
    HPRECONDITION (po_pData != 0);
    HPRECONDITION (pi_PosBlockX == 0);
    HPRECONDITION (pi_PosBlockY >= 0);
    HPRECONDITION (pi_PosBlockY < GetResolutionDescriptor()->GetHeight());
    HPRECONDITION (m_pResolutionDescriptor->GetBlockType() == HRFBlockType::LINE);

    HSTATUS         Status = H_ERROR;
    HFCLockMonitor  SisterFileLock;


    // Lock the sister file for the ReadBlock operation
    if (pi_pSisterFileLock == 0)
        {
        // Lock the file.
        AssignRasterFileLock(GetRasterFile(), SisterFileLock, true);
        pi_pSisterFileLock = &SisterFileLock;
        }


    // The desired line is before our current or the current line has been reinitialized
    if (m_CurrentReadLine > pi_PosBlockY || 0 == m_CurrentReadLine)
        {
        // Restart from 0 cause our algorithm can only navigate forward
        m_CurrentReadLine = 0;
        m_ReadPosInFile   = m_pRasterFile->GetOffBytesToData();
        }

    // Reset the file pointer to this editor's current position if not already the case
    if (m_pRasterFile->m_pPictFile->GetCurrentPos() != m_ReadPosInFile)
        m_pRasterFile->m_pPictFile->SeekToPos(m_ReadPosInFile);

    // The desired line is after our current line
    if (m_CurrentReadLine < pi_PosBlockY)
        {
        // Skip lines until we reach the desired one
        while (m_CurrentReadLine < pi_PosBlockY)
            {
            if (!SkipLine(&m_CurrentReadLine))
                {
                Status = H_READ_ERROR;
                goto WRAPUP;
                }
            }
        }

    if (!ReadAndUnpackLine(po_pData, m_BytesPerBlockWidth))
        {
        Status = H_READ_ERROR;
        goto WRAPUP;
        }

    // Unlock the sister file
    SisterFileLock.ReleaseKey();

    if (m_ReorganizeLineColorRepresentation)
        ReorganizeColorsToRGB(po_pData, m_BytesPerBlockWidth);

    Status = H_SUCCESS;

WRAPUP:

    if (Status != H_SUCCESS)
        {
        // Reset our position in the file
        m_CurrentReadLine   = 0;
        m_ReadPosInFile     = m_pRasterFile->GetOffBytesToData();
        }
    else
        {
        // Save our position in the file
        m_ReadPosInFile     = m_pRasterFile->m_pPictFile->GetCurrentPos();
        }

    return Status;
    }


//-----------------------------------------------------------------------------
// public
// WriteBlock
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFPictLineEditor::WriteBlock(uint64_t               pi_PosBlockX,
                                      uint64_t               pi_PosBlockY,
                                      const Byte*            pi_pData,
                                      HFCLockMonitor const*  pi_pSisterFileLock)
    {
    HPRECONDITION (m_AccessMode.m_HasWriteAccess || m_AccessMode.m_HasCreateAccess);
    HPRECONDITION (pi_pData != 0);
    HPRECONDITION (pi_PosBlockX == 0);
    HPRECONDITION (pi_PosBlockY >= 0);
    HPRECONDITION (pi_PosBlockY < GetResolutionDescriptor()->GetHeight());
    HPRECONDITION (m_pResolutionDescriptor->GetBlockType() == HRFBlockType::LINE);

    HSTATUS         Status = H_ERROR;
    HFCLockMonitor  SisterFileLock;

    if (0 == m_CurrentWriteLine)
        {
        // Restart from 0 cause our algorithm can only navigate forward
        m_CurrentWriteLine = 0;
        m_WritePosInFile   = m_pRasterFile->GetOffBytesToData();
        }

    // Reset the file pointer to this editor's current position if not already the case
    if (m_pRasterFile->m_pPictFile->GetCurrentPos() != m_WritePosInFile)
        m_pRasterFile->m_pPictFile->SeekToPos(m_WritePosInFile);

    if (m_CurrentWriteLine > pi_PosBlockY)
        {
        HASSERT(0); // Not Supported
        Status = H_WRITE_ERROR;
        goto WRAPUP;
        }

    // The desired line is after our current line
    if (m_CurrentWriteLine < pi_PosBlockY)
        {
        // Skip lines until we reach the desired one
        while (m_CurrentWriteLine < pi_PosBlockY)
            {
            if (!SkipLine(&m_CurrentWriteLine))
                {
                Status = H_WRITE_ERROR;
                goto WRAPUP;
                }
            }
        }

    if (m_ReorganizeLineColorRepresentation)
        ReorganizeColorsToPict(pi_pData, m_BytesPerBlockWidth, m_pWriteLineBuffer2, m_BytesPerBlockWidth);
    else
        memcpy(m_pWriteLineBuffer2, pi_pData, m_BytesPerBlockWidth);


    // Lock the sister file for the WriteBlock operation
    if (pi_pSisterFileLock == 0)
        {
        // Lock the file.
        AssignRasterFileLock(GetRasterFile(), SisterFileLock, true);
        pi_pSisterFileLock = &SisterFileLock;
        }

    if (!PackAndWriteLine(m_pWriteLineBuffer2, m_BytesPerBlockWidth))
        {
        Status = H_WRITE_ERROR;
        goto WRAPUP;
        }

    // Current line not supposed to be greater than image height
    HASSERT(m_CurrentWriteLine <= m_ImageHeigth);

    // We just wrote our last line successfully. Write padding so we end on 32bits boudary.
    if (m_CurrentWriteLine == m_ImageHeigth)
        {
        uint32_t PosInFile            = (uint32_t)m_pRasterFile->m_pPictFile->GetCurrentPos();

        uint32_t PaddingStartIndex    = PosInFile % 4;
        uint32_t PaddingBytesQty      = 4 - PaddingStartIndex;
        if (m_pRasterFile->m_pPictFile->Write(&s_FilePadding[PaddingStartIndex], PaddingBytesQty) != PaddingBytesQty)
            {
            Status = H_WRITE_ERROR;
            goto WRAPUP;
            }
        }

    // Unlock the sister file
    SisterFileLock.ReleaseKey();

    Status = H_SUCCESS;

WRAPUP:

    if (Status != H_SUCCESS)
        {
        // Reset our position in the file
        m_CurrentWriteLine  = 0;
        m_WritePosInFile    = m_pRasterFile->GetOffBytesToData();
        }
    else
        {
        // Save our position in the file
        m_WritePosInFile    = m_pRasterFile->m_pPictFile->GetCurrentPos();
        }

    return Status;
    }


//-----------------------------------------------------------------------------
// private
// ReorganizeColorsToRGB
// Reorganize a RGBRGBRGBRGB.. line to a RRRR..GGGG..BBBB.. line.
//-----------------------------------------------------------------------------
void HRFPictLineEditor::ReorganizeColorsToRGB  (Byte*         pio_Buffer,
                                                size_t          pi_BufferSize)
    {
    HPRECONDITION(m_LineBufferSize >= pi_BufferSize);
    HPRECONDITION(m_BytesPerBlockWidth <= pi_BufferSize);
    HPRECONDITION(m_BytesPerBlockWidth % 3 == 0);

    memcpy(m_pReadLineBuffer, pio_Buffer, pi_BufferSize);


    // Set a ptr to the start of each third of the input line
    const Byte*   pInRed      = m_pReadLineBuffer;
    const Byte*   pInGreen    = m_pReadLineBuffer + m_BytesPerBlockWidth / 3;
    const Byte*   pInBlue     = m_pReadLineBuffer + 2 * m_BytesPerBlockWidth / 3;

    // Reorganize a RRRGGGBBB line to an RGBRGBRGB line
    for (uint32_t i = 0; i < m_BytesPerBlockWidth; i += 3)
        {
        // Create an RGB section using each third of the input line
        *pio_Buffer++   = *pInRed++;
        *pio_Buffer++   = *pInGreen++;
        *pio_Buffer++   = *pInBlue++;
        }
    }


//-----------------------------------------------------------------------------
// private
// ReorganizeColorsToPict
// Reorganize a RRRR..GGGG..BBBB.. line to a RGBRGBRGBRGB.. line.
//-----------------------------------------------------------------------------
void HRFPictLineEditor::ReorganizeColorsToPict (const Byte*   pi_InBuffer,
                                                size_t          pi_InSize,
                                                Byte*         po_OutBuffer,
                                                size_t          pi_OutSize)
    {
    HPRECONDITION(pi_InSize <= pi_OutSize);
    HPRECONDITION(m_BytesPerBlockWidth <= pi_OutSize);
    HPRECONDITION(m_BytesPerBlockWidth % 3 == 0);

    // Set a ptr to the start of each third of the output line
    Byte*         pOutRed     = po_OutBuffer;
    Byte*         pOutGreen   = po_OutBuffer + m_BytesPerBlockWidth / 3;
    Byte*         pOutBlue    = po_OutBuffer + 2 * m_BytesPerBlockWidth / 3;

    // Reorganize a RGBRGBRGB line to an RRRGGGBBB line
    for (uint32_t i = 0; i < m_BytesPerBlockWidth; i += 3)
        {
        // From an RGB section add to each third of the output line
        *pOutRed++      = *pi_InBuffer++;
        *pOutGreen++    = *pi_InBuffer++;
        *pOutBlue++     = *pi_InBuffer++;
        }
    }


//-----------------------------------------------------------------------------
// private
// ReadPackedByteQty
// Read the packed byte quantity field from the file.
//-----------------------------------------------------------------------------
bool HRFPictLineEditor::ReadPackedByteQty     (uint32_t*         po_pPackedByteQty)
    {
    Byte ReadByte;

    *po_pPackedByteQty = 0;

    if (m_pRasterFile->m_pPictFile->Read(&ReadByte, sizeof ReadByte) != sizeof ReadByte)
        return false; // Error

    *po_pPackedByteQty = ReadByte;

    // There is two bytes reserved for the qty of packed bytes
    if (m_RowBytes > 250)
        {
        if (m_pRasterFile->m_pPictFile->Read(&ReadByte, sizeof ReadByte) != sizeof ReadByte)
            return false; // Error

        *po_pPackedByteQty = ((*po_pPackedByteQty << 8) & 0xFF00) | ReadByte;
        }

    return true; //No error

    }


//-----------------------------------------------------------------------------
// private
// WritePackedByteQty
// Write the packed bytes quantity field to the file.
//-----------------------------------------------------------------------------
bool HRFPictLineEditor::WritePackedByteQty    (uint32_t        pi_PackedByteQty)
    {
    Byte WrittenByte;

    // There is two bytes reserved for the qty of packed bytes
    if (m_RowBytes > 250)
        {
        WrittenByte = static_cast<Byte>((pi_PackedByteQty >> 8) & 0x00FF);
        if (m_pRasterFile->m_pPictFile->Write(&WrittenByte, sizeof WrittenByte) != sizeof WrittenByte)
            return false; // Error
        }

    WrittenByte = static_cast<Byte>(pi_PackedByteQty & 0x00FF);
    if (m_pRasterFile->m_pPictFile->Write(&WrittenByte, sizeof WrittenByte) != sizeof WrittenByte)
        return false; // Error

    return true; //No Error
    }


//-----------------------------------------------------------------------------
// private
// SkipLine
// Skip a line in the file using the packed bytes qty field as navigation
// guidance. Used only in read mode for the moment.
//-----------------------------------------------------------------------------
bool HRFPictLineEditor::SkipLine              (uint32_t*         pio_pLineIterator)
    {
    uint32_t PackedByteQty;
    if (!ReadPackedByteQty(&PackedByteQty))
        return false; // Error

    m_pRasterFile->m_pPictFile->Seek(PackedByteQty);

    ++(*pio_pLineIterator);

    return true; //No error
    }


//-----------------------------------------------------------------------------
// private
// ReadAndUnpackLine
// Read and unpack the next packed line of the file to the buffer passed as
// an argument.
//-----------------------------------------------------------------------------
bool HRFPictLineEditor::ReadAndUnpackLine     (Byte*         po_OutBuffer,
                                                size_t          pi_OutSize)
    {
    uint32_t PackedByteQty;
    if (!ReadPackedByteQty(&PackedByteQty))
        return false; // Error

    HASSERT(PackedByteQty <= m_LineBufferSize);
    if (m_pRasterFile->m_pPictFile->Read(m_pReadLineBuffer, PackedByteQty) != PackedByteQty)
        return false; // Error

    uint32_t UnpackedBytesQty;
    UnpackedBytesQty = (uint32_t)m_pCodec->DecompressSubset(m_pReadLineBuffer, PackedByteQty, po_OutBuffer, pi_OutSize);

    HPOSTCONDITION(UnpackedBytesQty == pi_OutSize);

    ++m_CurrentReadLine;

    return true; //No error
    }


//-----------------------------------------------------------------------------
// private
// PackAndWriteLine
// Pack and write to the file the line that is passed as an argument.
//-----------------------------------------------------------------------------
bool HRFPictLineEditor::PackAndWriteLine      (const Byte*   pi_InBuffer,
                                                size_t          pi_InSize)
    {
    HPRECONDITION(pi_InSize < m_LineBufferSize);

    uint32_t PackedBytesQty;

    // Pack the bytes of the three subsets of the line
    PackedBytesQty = (uint32_t)m_pCodec->CompressSubset(pi_InBuffer, pi_InSize, m_pWriteLineBuffer, m_LineBufferSize);

    HASSERT(m_LineBufferSize >= PackedBytesQty);

    // Write the packed line to the file
    if (!WritePackedByteQty(PackedBytesQty))
        return false; // Error
    if (m_pRasterFile->m_pPictFile->Write(m_pWriteLineBuffer, PackedBytesQty) != PackedBytesQty)
        return false; // Error

    ++m_CurrentWriteLine;

    return true; // No Error
    }