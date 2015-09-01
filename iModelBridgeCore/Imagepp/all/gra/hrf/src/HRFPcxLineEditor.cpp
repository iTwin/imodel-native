//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFPcxLineEditor.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class HRFPcxLineEditor
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>
                //:> must be first for PreCompiledHeader Option
#include <Imagepp/all/h/HRFPcxLineEditor.h>
#include <Imagepp/all/h/HRFPcxFile.h>

//:Ignore
#define BUFFER_SIZE 16
#define END_OF_HEADER_OFFSET 128
//:End Ignore

/**-----------------------------------------------------------------------------
 Constructor of the class HRFPcxLineEditor. This method initializes the intern
 attibutes for later use.

 @param pi_rpRasterFile A pointer to the associate raster file.
 @param pi_Page The number of the associate page descriptor.
 @param pi_Resolution The number of the associate resolution descriptor.
 @param pi_AccessMode The access and sharing modes for the file.
------------------------------------------------------------------------------*/
HRFPcxLineEditor::HRFPcxLineEditor(HFCPtr<HRFRasterFile> pi_rpRasterFile,
                                   uint32_t              pi_Page,
                                   unsigned short       pi_Resolution,
                                   HFCAccessMode         pi_AccessMode)
    : HRFResolutionEditor(pi_rpRasterFile,
                          pi_Page,
                          pi_Resolution,
                          pi_AccessMode)
    {
    m_FileBufferSize  = GetResolutionDescriptor()->GetBytesPerBlockWidth();
    m_pFileBuffer     = new Byte[m_FileBufferSize];
    m_FileBufferIndex = 0;
    m_FilePos         = END_OF_HEADER_OFFSET;
    m_CurrentLineNumber = 0;
    m_ConvertSize     = ((HFCPtr<HRFPcxFile>&)pi_rpRasterFile)->m_pPcxHdr->BytesPerLine * ((HFCPtr<HRFPcxFile>&)pi_rpRasterFile)->m_pPcxHdr->NumBitPlanes;

    m_pConvert = new Byte[m_ConvertSize];

    // Required only in write mode.
    if (pi_AccessMode.m_HasWriteAccess || pi_AccessMode.m_HasCreateAccess)
        {
        m_pEncoded = new Byte[m_ConvertSize * 3];
        }
    }

/**-----------------------------------------------------------------------------
 Public destructor for the class.
------------------------------------------------------------------------------*/
HRFPcxLineEditor::~HRFPcxLineEditor()
    {
    // This does nothing!
    }

/**-----------------------------------------------------------------------------
 Read an uncompressed block of pixels on this resolution.
 The block position must be specified and can be compruted by the resolution
 descriptor.
 The @i{po_pData} must be allocated by the user of this function.
 The size must be > ${#GetBlockSizeInByte()}.

 @param pi_PosBlockX The X position of the block in the file.
 @param pi_PosBlockY The Y position of the block in the file.
 @param po_pData The buffer to be returned with the raster data.

 @return HSTATUS H_SUCCESS if the readint operation went right.
------------------------------------------------------------------------------*/
HSTATUS HRFPcxLineEditor::ReadBlock(uint64_t pi_PosBlockX,
                                    uint64_t pi_PosBlockY,
                                    Byte*  po_pData,
                                    HFCLockMonitor const* pi_pSisterFileLock)
    {
    /**
        Here, we did not implement a codec for this file format. But the raster data are
        always compressed with a simple RLE algorithm. The decompression has been implement
        directly into the ReadBlock. Since the data in the file format are paded on 2 bytes
        and since we do not specify it into the ResolutionDescriptor, we use two different
        buffer size. ConvertSize is the size in bytes of one paded line in the file. ExportSize
        is the size in bytes of the none paded line returned in po_pData.
    */

    HPRECONDITION (po_pData != 0);
    HPRECONDITION (m_AccessMode.m_HasReadAccess);
    HPRECONDITION (pi_PosBlockX == 0);
    HPRECONDITION (pi_PosBlockY >= 0);
    HPRECONDITION (pi_PosBlockY < GetResolutionDescriptor()->GetHeight());

    HSTATUS             Status       = H_SUCCESS;
    HFCPtr<HRFPcxFile>  pPcxFile     = static_cast<HRFPcxFile*>(GetRasterFile().GetPtr());
    HFCLockMonitor SisterFileLock;

    if (pPcxFile->GetAccessMode().m_HasCreateAccess)
        {
        Status = H_NOT_FOUND;
        goto WRAPUP;
        }

    // Lock the sister file for the ReadBlock operation
    if(pi_pSisterFileLock == 0)
        {
        // Lock the file.
        AssignRasterFileLock(GetRasterFile(), SisterFileLock, true);
        pi_pSisterFileLock = &SisterFileLock;
        }

    //:> Reset the file pointer to the begining of the file if an edition occured since
    //:> the last read or if we already red the line asked.
    if ((m_CurrentLineNumber > pi_PosBlockY) || (0 == m_CurrentLineNumber))
        {
        m_CurrentLineNumber = 0;
        m_FileBufferIndex   = 0;
        m_FilePos           = END_OF_HEADER_OFFSET;
        pPcxFile->m_pPcxFile->SeekToPos(END_OF_HEADER_OFFSET);

        uint32_t ReadSize = (uint32_t)MIN(pPcxFile->m_pPcxFile->GetSize() - pPcxFile->m_pPcxFile->GetCurrentPos(), (uint64_t)m_FileBufferSize);
        if (pPcxFile->m_pPcxFile->Read(m_pFileBuffer, ReadSize) != ReadSize)
            {
            Status = H_ERROR;
            goto WRAPUP;    // H_ERROR;
            }
        }
    //:> If the line asked is further than the current one, we must skip those lines
    while (m_CurrentLineNumber < pi_PosBlockY)
        {
        Status = ReadAndDecompressPcxLine (0, pi_pSisterFileLock);
        ++m_CurrentLineNumber;
        }

    // Read and decompress the current line
    //        memset (pConvert, 0x00, m_ConvertSize);
    Status = ReadAndDecompressPcxLine (m_pConvert, pi_pSisterFileLock);
    ++m_CurrentLineNumber;


    //:> Unlock the sister file
    SisterFileLock.ReleaseKey();

    if (!ConvertDataToPcxFormat(m_pConvert, po_pData))
        {
        Status = H_ERROR;
        goto WRAPUP;    // H_ERROR;
        }


WRAPUP:
    return Status;
    }

/**-----------------------------------------------------------------------------
 Read an uncompressed block of pixels on this resolution.
 This method passes the RLE decompression algorithm on the raster data comming
 from the file and return the decompressed data in @i{po_pConvert}

 @param po_pConvert The buffer to return the decompressed data.
------------------------------------------------------------------------------*/
HSTATUS HRFPcxLineEditor::ReadAndDecompressPcxLine(Byte* po_pConvert, HFCLockMonitor const* pi_pSisterFileLock)
    {
    HFCPtr<HRFPcxFile>  pPcxFile   = static_cast<HRFPcxFile*>(GetRasterFile().GetPtr());
    uint32_t             DataIndex  = 0;
    Byte               Val;
    Byte               RunValue;
    Byte               RunCount;
    HSTATUS             Status = H_ERROR;


    //:> decoding the data... We did not do a codec for this file format
    do
        {
        while ((m_FileBufferIndex < m_FileBufferSize-1) && (DataIndex < m_ConvertSize))
            {
            Val = m_pFileBuffer[m_FileBufferIndex++];
            if ((Val & 0xC0) == 0xC0)
                {
                RunCount = Val & 0x3F;
                RunValue = m_pFileBuffer[m_FileBufferIndex++];
                }
            else
                {
                RunCount = 1;
                RunValue = Val;
                }

            //:> Write the pixel run to the buffer
            if (po_pConvert != 0)
                memset (po_pConvert+DataIndex, RunValue, (RunCount < (m_ConvertSize-DataIndex) ? RunCount : m_ConvertSize-DataIndex));

            DataIndex+=RunCount;
            }

        //:> Reload the reading buffer when it is empty.
        if (DataIndex < m_ConvertSize)
            {
            HASSERT ((m_FileBufferIndex == m_FileBufferSize-1) || (m_FileBufferIndex == m_FileBufferSize));

            // Lock the sister file
            HFCLockMonitor SisterFileLock;
            if(pi_pSisterFileLock == 0)
                {
                // Lock the file.
                AssignRasterFileLock(GetRasterFile(), SisterFileLock, false);
                pi_pSisterFileLock = &SisterFileLock;
                }

            pPcxFile->m_pPcxFile->SeekToPos(m_FilePos+m_FileBufferIndex);
            uint32_t ReadSize = (uint32_t)MIN(pPcxFile->m_pPcxFile->GetSize() - pPcxFile->m_pPcxFile->GetCurrentPos(), (uint64_t)m_FileBufferSize);
            if (pPcxFile->m_pPcxFile->Read(m_pFileBuffer, ReadSize) != ReadSize)
                goto WRAPUP;    // H_ERROR;

            //:> Unlock the sister file.
            SisterFileLock.ReleaseKey();

            m_FilePos += m_FileBufferIndex;
            m_FileBufferIndex = 0;
            }

        }
    while (DataIndex < m_ConvertSize);

    //:> Assert that we have decoded at least the exact size of a line.
    HPOSTCONDITION (DataIndex >= m_ConvertSize);

    Status = H_SUCCESS;

WRAPUP:
    return Status;
    }

/**-----------------------------------------------------------------------------
 Makes a conversion on the decompressed raster data to return them in a
 recognize format in our libraries.
 If the rasters are 24-bit, it is in @b{plane} so we must put together the
 information into a @b{pixel} format.
 If the rasters are 4-bit indexed, it is also in @b{plane}. We must take
 the first bit of each plane to reconstruct the index of the first color.

 @param pi_pConvert The decompressed data in the Pcx format.
 @param po_pData The buffer to be returned with the formated raster data.

 @return HSTATUS H_SUCCESS if the readint operation went right.
------------------------------------------------------------------------------*/
bool HRFPcxLineEditor::ConvertDataToPcxFormat(Byte* pi_pConvert, Byte* po_pData)
    {
    HPRECONDITION(pi_pConvert != 0);
    HPRECONDITION(po_pData != 0);

    bool   Result = true;
    uint32_t ExportSize  = (uint32_t)GetResolutionDescriptor()->GetBytesPerWidth();
    uint32_t Index1;
    uint32_t Index2;
    uint32_t Index3;
    uint32_t i           = 0;
    uint32_t j           = 0;

    //:> Parsing the decoded line for multi-planes data.
    switch (GetResolutionDescriptor()->GetBitsPerPixel())
        {
        case 24 :
            //:> Converting the decoded data from 24 bits PLANE to 24 bits PIXEL
            Index1 = m_ConvertSize/3;
            Index2 = Index1 + Index1;
            for (; j < ExportSize; i++)
                {
                po_pData[j++] = pi_pConvert[i];
                po_pData[j++] = pi_pConvert[Index1+i];
                po_pData[j++] = pi_pConvert[Index2+i];
                }
            break;
        case 4 :
            //:> Converting the decoded data from 4 bits PLANE to 4 bits PIXEL
            Index1 = m_ConvertSize/4;
            Index2 = Index1 + Index1;
            Index3 = Index2 + Index1;
            for (; j < ExportSize-3; i++, j+=4)
                {
                //:> The index is compute by taking 1 bits from each plane starting with the
                //:> less significant bit of the index
                po_pData[j] =   ((pi_pConvert[i]       &0x80) >> 3) | ((pi_pConvert[Index1+i]&0x80) >> 2) |
                                ((pi_pConvert[Index2+i]&0x80) >> 1) | ((pi_pConvert[Index3+i]&0x80))      |
                                ((pi_pConvert[i]       &0x40) >> 6) | ((pi_pConvert[Index1+i]&0x40) >> 5) |
                                ((pi_pConvert[Index2+i]&0x40) >> 4) | ((pi_pConvert[Index3+i]&0x40) >> 3);

                po_pData[j+1] = ((pi_pConvert[i]&0x20)        >> 1) | ((pi_pConvert[Index1+i]&0x20))      |
                                ((pi_pConvert[Index2+i]&0x20) << 1) | ((pi_pConvert[Index3+i]&0x20) << 2) |
                                ((pi_pConvert[i]&0x10)        >> 4) | ((pi_pConvert[Index1+i]&0x10) >> 3) |
                                ((pi_pConvert[Index2+i]&0x10) >> 2) | ((pi_pConvert[Index3+i]&0x10) >> 1);

                po_pData[j+2] = ((pi_pConvert[i]&0x08)        << 1) | ((pi_pConvert[Index1+i]&0x08) << 2) |
                                ((pi_pConvert[Index2+i]&0x08) << 3) | ((pi_pConvert[Index3+i]&0x08) << 4) |
                                ((pi_pConvert[i]&0x04)        >> 2) | ((pi_pConvert[Index1+i]&0x04) >> 1) |
                                ((pi_pConvert[Index2+i]&0x04))      | ((pi_pConvert[Index3+i]&0x04) << 1);

                po_pData[j+3] = ((pi_pConvert[i]&0x02)        << 3) | ((pi_pConvert[Index1+i]&0x02) << 4) |
                                ((pi_pConvert[Index2+i]&0x02) << 5) | ((pi_pConvert[Index3+i]&0x02) << 6) |
                                ((pi_pConvert[i]&0x01))             | ((pi_pConvert[Index1+i]&0x01) << 1) |
                                ((pi_pConvert[Index2+i]&0x01) << 2) | ((pi_pConvert[Index3+i]&0x01) << 3);
                }

            //:> This part complete the line without busting the allocated buffer
            if (1 <= ExportSize - j)
                po_pData[j] =   ((pi_pConvert[i]       &0x80) >> 3) | ((pi_pConvert[Index1+i]&0x80) >> 2) |
                                ((pi_pConvert[Index2+i]&0x80) >> 1) | ((pi_pConvert[Index3+i]&0x80))      |
                                ((pi_pConvert[i]       &0x40) >> 6) | ((pi_pConvert[Index1+i]&0x40) >> 5) |
                                ((pi_pConvert[Index2+i]&0x40) >> 4) | ((pi_pConvert[Index3+i]&0x40) >> 3);
            if (2 <= ExportSize - j)
                po_pData[j+1] = ((pi_pConvert[i]&0x20)        >> 1) | ((pi_pConvert[Index1+i]&0x20))      |
                                ((pi_pConvert[Index2+i]&0x20) << 1) | ((pi_pConvert[Index3+i]&0x20) << 2) |
                                ((pi_pConvert[i]&0x10)        >> 4) | ((pi_pConvert[Index1+i]&0x10) >> 3) |
                                ((pi_pConvert[Index2+i]&0x10) >> 2) | ((pi_pConvert[Index3+i]&0x10) >> 1);
            if (3 <= ExportSize - j)
                po_pData[j+2] = ((pi_pConvert[i]&0x08)        << 1) | ((pi_pConvert[Index1+i]&0x08) << 2) |
                                ((pi_pConvert[Index2+i]&0x08) << 3) | ((pi_pConvert[Index3+i]&0x08) << 4) |
                                ((pi_pConvert[i]&0x04)        >> 2) | ((pi_pConvert[Index1+i]&0x04) >> 1) |
                                ((pi_pConvert[Index2+i]&0x04))      | ((pi_pConvert[Index3+i]&0x04) << 1);
            HDEBUGCODE( if (4 <= ExportSize - j) )
                HDEBUGCODE(     HASSERT (0);         )
                break;
        case 1:
        case 8:
            memcpy (po_pData, pi_pConvert, ExportSize);
            break;
        default :
            Result = false;
        }

    return Result;
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
HSTATUS HRFPcxLineEditor::WriteBlock(uint64_t     pi_PosBlockX,
                                     uint64_t     pi_PosBlockY,
                                     const Byte*  pi_pData,
                                     HFCLockMonitor const* pi_pSisterFileLock)
    {
    HPRECONDITION (m_AccessMode.m_HasWriteAccess || m_AccessMode.m_HasCreateAccess);
    HPRECONDITION (pi_pData != 0);
    HPRECONDITION (pi_PosBlockX == 0);
    HPRECONDITION (pi_PosBlockY >= 0);
    HPRECONDITION (pi_PosBlockY < GetResolutionDescriptor()->GetHeight());
    HPRECONDITION (m_pEncoded != 0);

    HSTATUS             Status       = H_ERROR;
    HFCPtr<HRFPcxFile>  pPcxFile     = static_cast<HRFPcxFile*>(GetRasterFile().GetPtr());
    Byte               Val;
    Byte               RunCount;
    uint32_t             i;
    uint32_t             j;
    uint32_t             Index1;
    uint32_t             Index2;
    uint32_t             Index3;
    uint32_t             ImportSize   = (uint32_t)GetResolutionDescriptor()->GetBytesPerWidth();
    uint32_t             PlaneSize    = pPcxFile->m_pPcxHdr->BytesPerLine;
    uint32_t             ConvertIndex = 0;
    uint32_t             EncodedIndex = 0;
    HFCLockMonitor      SisterFileLock;

    //:> Parsing the decoded line
    switch (GetResolutionDescriptor()->GetBitsPerPixel())
        {
        case 24 :
            //:> Converting the data from 24 bits PIXEL to 24 bits PIXEL
            Index1 = m_ConvertSize/3;
            Index2 = Index1 + Index1;

            //:> Initialise the padding bytes of pConvert with 0x00
            m_pConvert[Index1-1] = 0x00;
            m_pConvert[Index2-1] = 0x00;
            m_pConvert[m_ConvertSize-1] = 0x00;

            //:> Fill pConvert with the data received in parameters
            for (i=0, j=0; j < ImportSize; i++)
                {
                m_pConvert[i]        = pi_pData[j++];
                m_pConvert[Index1+i] = pi_pData[j++];
                m_pConvert[Index2+i] = pi_pData[j++];
                }
            break;
        case 4 :
            //:> Converting the decoded data from 4 bits PLANE to 4 bits PIXEL
            Index1 = m_ConvertSize/4;
            Index2 = Index1 + Index1;
            Index3 = Index2 + Index1;

            //:> Initialise the padding bytes of pConvert with 0x00
            m_pConvert[Index1-1] = 0x00;
            m_pConvert[Index2-1] = 0x00;
            m_pConvert[Index3-1] = 0x00;
            m_pConvert[m_ConvertSize-1] = 0x00;

            //:> Fill pConvert with the data received in parameters
            for (i=0, j=0; j < ImportSize; j += 4, i++)
                {
                //:> Each planes of m_pConvert are made from a succession of bit of same weight of each index.
                m_pConvert[i] = ((pi_pData[j] & 0x10) << 3) | ((pi_pData[j] & 0x01) << 6) |
                                ((pi_pData[j+1]&0x10) << 1) | ((pi_pData[j+1]&0x01) << 4) |
                                ((pi_pData[j+2]&0x10) >> 1) | ((pi_pData[j+2]&0x01) << 2) |
                                ((pi_pData[j+3]&0x10) >> 3) | ((pi_pData[j+3]&0x01));

                m_pConvert[i+Index1] = ((pi_pData[j] & 0x20) << 2) | ((pi_pData[j] & 0x02) << 5) |
                                       ((pi_pData[j+1]&0x20))      | ((pi_pData[j+1]&0x02) << 3) |
                                       ((pi_pData[j+2]&0x20) >> 2) | ((pi_pData[j+2]&0x02) << 1) |
                                       ((pi_pData[j+3]&0x20) >> 4) | ((pi_pData[j+3]&0x02) >> 1);

                m_pConvert[i+Index2] = ((pi_pData[j] & 0x40) << 1) | ((pi_pData[j] & 0x04) << 4) |
                                       ((pi_pData[j+1]&0x40) >> 1) | ((pi_pData[j+1]&0x04) << 2) |
                                       ((pi_pData[j+2]&0x40) >> 3) | ((pi_pData[j+2]&0x04))      |
                                       ((pi_pData[j+3]&0x40) >> 5) | ((pi_pData[j+3]&0x04) >> 2);

                m_pConvert[i+Index3] = ((pi_pData[j] & 0x80))      | ((pi_pData[j] & 0x08) << 3) |
                                       ((pi_pData[j+1]&0x80) >> 2) | ((pi_pData[j+1]&0x08) << 1) |
                                       ((pi_pData[j+2]&0x80) >> 4) | ((pi_pData[j+2]&0x08) >> 1) |
                                       ((pi_pData[j+3]&0x80) >> 6) | ((pi_pData[j+3]&0x08) >> 3);
                }
            break;
        case 1 :
        case 8 :
            //:> Initialise the padding byte of m_pConvert with 0x00
            m_pConvert[m_ConvertSize-1] = 0x00;
            memcpy (m_pConvert, pi_pData, ImportSize);
            break;
        default :
            goto WRAPUP;
        }

    //:> Encode the scan line
    do
        {
        Val = m_pConvert[ConvertIndex++];
        RunCount = 0;
        while ((ConvertIndex < m_ConvertSize) && (m_pConvert[ConvertIndex] == Val) && (RunCount < 62) && (ConvertIndex%PlaneSize != 0))
            {
            ++RunCount;
            ++ConvertIndex;
            }

        if ((RunCount > 0) || ((Val & 0xC0) == 0xC0))
            m_pEncoded[EncodedIndex++] = (RunCount+1) | 0xC0;

        m_pEncoded[EncodedIndex++] = Val;
        }
    while (ConvertIndex < m_ConvertSize);

    // Lock the sister file
    if(pi_pSisterFileLock == 0)
        {
        // Lock the file.
        AssignRasterFileLock(GetRasterFile(), SisterFileLock, false);
        pi_pSisterFileLock = &SisterFileLock;
        }

    //:> Set the file pointer for the first line
    if (0 == pi_PosBlockY)
        {
        pPcxFile->m_pPcxFile->SeekToPos(END_OF_HEADER_OFFSET);
        m_CurrentLineNumber = 0;
        }

    //:> Make sure that the file pointer is at the write place into the file.
    if (m_CurrentLineNumber != pi_PosBlockY)
        {
        HASSERT (!GetRasterFile()->GetAccessMode().m_HasCreateAccess);

        pPcxFile->m_pPcxFile->SeekToPos(END_OF_HEADER_OFFSET);
        uint32_t ReadSize = (uint32_t)MIN(pPcxFile->m_pPcxFile->GetSize() - pPcxFile->m_pPcxFile->GetCurrentPos(), (uint64_t)m_FileBufferSize);
        if (pPcxFile->m_pPcxFile->Read(m_pFileBuffer, ReadSize) != ReadSize)
            goto WRAPUP;    // H_ERROR

        m_CurrentLineNumber = 0;
        m_FileBufferIndex   = 0;
        m_FilePos = END_OF_HEADER_OFFSET;

        while (m_CurrentLineNumber != pi_PosBlockY)
            {
            ReadAndDecompressPcxLine(0, pi_pSisterFileLock);
            ++m_CurrentLineNumber;
            }
        }

    //:> Write the encoded scan line to the file.
    if(pPcxFile->m_pPcxFile->Write(m_pEncoded, EncodedIndex) != EncodedIndex)
        goto WRAPUP;    // H_ERROR

    //:> Increment the modifications counter;
    GetRasterFile()->SharingControlIncrementCount();

    ++m_CurrentLineNumber;

    //:> After the last output to the file, keep the value of the file pointer for appending of the VGA Palette
    if (pi_PosBlockY == GetResolutionDescriptor()->GetHeight() - 1)
        {
        pPcxFile->m_VGAPaletteOffset = (uint32_t)(pPcxFile->m_pPcxFile->GetCurrentPos());

        // It some circumstances the new offset may be the same as the old one.
        // If this is the case reset m_OldVGAPaletteOffset so we ensure that the VGA palette is appended to the file.
        if(pPcxFile->m_VGAPaletteOffset == pPcxFile->m_OldVGAPaletteOffset)
            pPcxFile->m_OldVGAPaletteOffset=0;

        //:> Set the end of file caracter at the current position.
        if (pPcxFile->m_pPcxFile->IsCompatibleWith(HFCLocalBinStream::CLASS_ID))
            ((HFCPtr<HFCLocalBinStream>&)pPcxFile->m_pPcxFile)->SetEOF();

        }

    //:> Unlock the sister file.
    SisterFileLock.ReleaseKey();

    Status = H_SUCCESS;

WRAPUP:
    return Status;
    }

//-----------------------------------------------------------------------------
// public
// OnSynchronizedSharingControl
//-----------------------------------------------------------------------------
void HRFPcxLineEditor::OnSynchronizedSharingControl()
    {
    m_CurrentLineNumber = 0;
    m_FileBufferIndex   = 0;
    m_FilePos           = END_OF_HEADER_OFFSET;
    static_cast<HRFPcxFile*>(GetRasterFile().GetPtr())->m_pPcxFile->SeekToPos(END_OF_HEADER_OFFSET);
    static_cast<HRFPcxFile*>(GetRasterFile().GetPtr())->m_pPcxFile->Read(m_pFileBuffer, m_FileBufferSize);
    }
