//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFSLOStripEditor.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class HRFSLOStripEditor
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HRFSLOStripEditor.h>
#include <Imagepp/all/h/HRFSLOStripAdapter.h>
#include <Imagepp/all/h/HCDPacket.h>
#include <Imagepp/all/h/HCDCodecHMRRLE1.h>
#include <Imagepp/all/h/HCDCodecIdentity.h>


// Initialisation
Byte HRFSLOStripEditor::m_BitMask[] = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};

//-----------------------------------------------------------------------------
// Private
// SetBitOn
//-----------------------------------------------------------------------------
inline void HRFSLOStripEditor::SetBitOn (Byte* pByte, int32_t bit)
    {
    *pByte = *pByte | m_BitMask[bit];
    }

//-----------------------------------------------------------------------------
// Private
// SetBitOff
//-----------------------------------------------------------------------------
inline void HRFSLOStripEditor::SetBitOff (Byte* pByte, int32_t bit)
    {
    *pByte = *pByte & ~m_BitMask[bit];
    }

//-----------------------------------------------------------------------------
// Private
// GetBit
//-----------------------------------------------------------------------------
inline Byte HRFSLOStripEditor::GetBit (Byte* pByte, int32_t bit)
    {
    return (*pByte & m_BitMask[bit]);
    }

//-----------------------------------------------------------------------------
// Public
// Construction
//-----------------------------------------------------------------------------
HRFSLOStripEditor::HRFSLOStripEditor(HFCPtr<HRFRasterFile> pi_rpRasterFile,
                                     uint32_t              pi_Page,
                                     unsigned short       pi_Resolution,
                                     HFCAccessMode         pi_AccessMode,
                                     HRFResolutionEditor*  pi_pResolutionEditor)
    : HRFResolutionEditor( pi_rpRasterFile, pi_Page, pi_Resolution, pi_AccessMode)
    {
    HPRECONDITION (pi_pResolutionEditor != 0);

    m_pSrcResolutionEditor  = pi_pResolutionEditor;
    m_AllImageRead          = false;
    m_BitsPerPixel          = (Byte)GetResolutionDescriptor()->GetPixelType()->CountPixelRawDataBits();
    m_ScanLineOrientation   = m_pSrcResolutionEditor->GetResolutionDescriptor()->GetScanlineOrientation();

    HASSERT(m_pSrcResolutionEditor->GetResolutionDescriptor()->GetWidth() <= ULONG_MAX);
    HASSERT(m_pSrcResolutionEditor->GetResolutionDescriptor()->GetHeight() <= ULONG_MAX);
    HASSERT(m_pSrcResolutionEditor->GetResolutionDescriptor()->GetBytesPerWidth() <= ULONG_MAX);

    m_SrcImageWidth         = (uint32_t)m_pSrcResolutionEditor->GetResolutionDescriptor()->GetWidth();
    m_SrcImageHeight        = (uint32_t)m_pSrcResolutionEditor->GetResolutionDescriptor()->GetHeight();
    m_SrcImageBlockHeight   = m_pSrcResolutionEditor->GetResolutionDescriptor()->GetBlockHeight();
    m_SrcImageBytesPerWidth = (uint32_t)m_pSrcResolutionEditor->GetResolutionDescriptor()->GetBytesPerWidth();

    HASSERT(GetResolutionDescriptor()->GetWidth() <= ULONG_MAX);
    HASSERT(GetResolutionDescriptor()->GetHeight() <= ULONG_MAX);

    m_AdaptorWidth          = (uint32_t)GetResolutionDescriptor()->GetWidth();
    m_AdaptorHeight         = (uint32_t)GetResolutionDescriptor()->GetHeight();
    m_AdaptorBlockWidth     = GetResolutionDescriptor()->GetBlockWidth();
    m_AdaptorBlockHeight    = GetResolutionDescriptor()->GetBlockHeight();
    m_AdaptorBlockPerHeight = (uint32_t) ceil(m_AdaptorHeight / (double)m_AdaptorBlockHeight);
    m_AdaptorBlockPerWidth  = (uint32_t) ceil(m_AdaptorWidth / (double)m_AdaptorBlockWidth);

    m_SrcStripsHeight       = m_AdaptorBlockHeight;
    m_SrcStripsWidth        = m_SrcImageWidth;
    m_SrcStripsPerHeight    = (uint32_t)ceil (m_SrcImageHeight / (double)m_SrcStripsHeight);
    m_SrcStripsPerWidth     = 1;
    m_SrcStripsBytesPerWidth= (uint32_t)ceil (m_SrcStripsWidth / 8.0);
    m_SrcStripsPadding      = CalcPadding (m_SrcImageHeight, m_SrcStripsHeight);

    m_ppBlocks     = 0;
    m_ppTempBlocks = 0;

    // Does not supported another pixel type at this time...
    HPOSTCONDITION(m_BitsPerPixel == 1);

    // Must have line acces
    HPOSTCONDITION(m_SrcImageBlockHeight == 1);
    }


//-----------------------------------------------------------------------------
// public
// Destruction
//-----------------------------------------------------------------------------
HRFSLOStripEditor::~HRFSLOStripEditor()
    {

    }

//-----------------------------------------------------------------------------
// Public
// ReadBlock
// Read an Uncompressed Block
//-----------------------------------------------------------------------------
HSTATUS HRFSLOStripEditor::ReadBlock(uint64_t pi_PosBlockX,
                                     uint64_t pi_PosBlockY,
                                     Byte*   po_pData,
                                     HFCLockMonitor const* pi_pSisterFileLock)
    {
    HPRECONDITION (m_AccessMode.m_HasReadAccess);
    HPRECONDITION (po_pData != 0);

    HFCPtr<HCDCodecHMRRLE1> pCodec;
    uint32_t DataWidthInByte;
    size_t  DataHeight;
    size_t  DataSize;
    HSTATUS Status = H_SUCCESS;

    HASSERT (pi_PosBlockX < m_AdaptorWidth);
    HASSERT (pi_PosBlockY < m_AdaptorHeight);

    // Allocate a packet
    HFCPtr<HCDPacket> pPacket = new HCDPacket();
    pPacket->SetBufferOwnership(true);

    // Fill packet using the ReadBlock method with Packet
    Status = ReadBlock(pi_PosBlockX, pi_PosBlockY, pPacket);

    if (Status == H_SUCCESS)
        {
        pCodec          = ((HFCPtr<HCDCodecHMRRLE1>&)pPacket->GetCodec());
        DataWidthInByte = (uint32_t) (ceil(pCodec->GetWidth() / 8.0));
        DataHeight      = pCodec->GetHeight();
        DataSize        = DataWidthInByte * DataHeight;

        // Create a packet to uncompress data
        HFCPtr<HCDPacket> UncompressedPacket = new HCDPacket(po_pData, DataSize, DataSize);
        UncompressedPacket->SetBufferOwnership(false);
        pPacket->Decompress(UncompressedPacket);
        }

    return Status;
    }

//-----------------------------------------------------------------------------
// Public
// ReadBlock
// Read compressed Block
//-----------------------------------------------------------------------------
HSTATUS HRFSLOStripEditor::ReadBlock(uint64_t            pi_PosBlockX,
                                     uint64_t            pi_PosBlockY,
                                     HFCPtr<HCDPacket>&  po_rpPacket,
                                     HFCLockMonitor const* pi_pSisterFileLock)
    {
    HPRECONDITION (m_AccessMode.m_HasReadAccess);
    HPRECONDITION (pi_PosBlockX <= ULONG_MAX && pi_PosBlockY <= ULONG_MAX);

    HSTATUS Status      = H_SUCCESS;
    uint32_t ColumnIndex;
    uint32_t LineIndex;

    HASSERT (pi_PosBlockX < m_AdaptorWidth);
    HASSERT (pi_PosBlockY < m_AdaptorHeight);

    // Get the index of the strip required
    GetAdaptorBlockIndex((uint32_t)pi_PosBlockX, (uint32_t)pi_PosBlockY, &ColumnIndex, &LineIndex);

    // Check if the image is in memory
    if (!m_AllImageRead)
        {
        // Read the whole image and keep it in RLE1
        Status = BuildRLE1Image(m_ppBlocks);
        }
    else if (m_ppBlocks[LineIndex][ColumnIndex] == 0)
        {
        BlockTable ppBlocksTable;
        Status = BuildRLE1Image(ppBlocksTable);
        if (Status == H_SUCCESS)
            m_ppBlocks = ppBlocksTable; // we have all blocks, replace the entire table

        ppBlocksTable = 0;  // delete the temporary table
        }

    if (Status == H_SUCCESS)
        {
        // Packet must not be null
        HASSERT(m_ppBlocks[LineIndex][ColumnIndex] != NULL);

        if (po_rpPacket->GetBufferSize() == 0)
            {
            // The packet doesn't have a buffer
            // Set the buffer containned in m_ppBlocks
            po_rpPacket->SetBuffer(m_ppBlocks[LineIndex][ColumnIndex]->GetBufferAddress(),
                                   m_ppBlocks[LineIndex][ColumnIndex]->GetBufferSize());
            po_rpPacket->SetDataSize (m_ppBlocks[LineIndex][ColumnIndex]->GetDataSize());

            // Release ownership on the source packet
            m_ppBlocks[LineIndex][ColumnIndex]->SetBufferOwnership(false);

            // Set ownership on the buffer of the destination packet
            po_rpPacket->SetBufferOwnership(true);

            // Set the source Codec in the destination
            po_rpPacket->SetCodec(m_ppBlocks[LineIndex][ColumnIndex]->GetCodec());
            }
        else
            {
            // Packet already own of the buffer

            // Check if we have enough space to copy the current block
            if (po_rpPacket->GetBufferSize() < m_ppBlocks[LineIndex][ColumnIndex]->GetBufferSize())
                {
                // Allocate a new buffer
                po_rpPacket->SetBuffer(new Byte[m_ppBlocks[LineIndex][ColumnIndex]->GetDataSize()],
                                       m_ppBlocks[LineIndex][ColumnIndex]->GetDataSize());
                po_rpPacket->SetBufferOwnership(true);
                }

            // Copy block
            memcpy(po_rpPacket->GetBufferAddress(),
                   m_ppBlocks[LineIndex][ColumnIndex]->GetBufferAddress(),
                   m_ppBlocks[LineIndex][ColumnIndex]->GetDataSize());

            // Set data size
            po_rpPacket->SetDataSize(m_ppBlocks[LineIndex][ColumnIndex]->GetDataSize());

            // Set codec
            po_rpPacket->SetCodec(m_ppBlocks[LineIndex][ColumnIndex]->GetCodec());

            }

        // No longer required, delete the Packet
        m_ppBlocks[LineIndex][ColumnIndex] = 0;
        }

    return Status;
    }

//-----------------------------------------------------------------------------
// Public
// WriteBlock
// Write an uncompressed block
//-----------------------------------------------------------------------------
HSTATUS HRFSLOStripEditor::WriteBlock(uint64_t      pi_PosBlockX,
                                      uint64_t      pi_PosBlockY,
                                      const Byte*   pi_pData,
                                      HFCLockMonitor const* pi_pSisterFileLock)
    {
    HPRECONDITION (m_AccessMode.m_HasWriteAccess || m_AccessMode.m_HasCreateAccess);
    HPRECONDITION (pi_PosBlockX <= ULONG_MAX && pi_PosBlockY <= ULONG_MAX);

    HSTATUS Status = H_SUCCESS;

    // Get current block size
    uint32_t BlockHeight = m_AdaptorBlockHeight;
    uint32_t BlockWidth  = m_AdaptorBlockWidth;

    // Adjust Block Height if it is the last block
    if ((uint32_t)pi_PosBlockY == (m_AdaptorBlockPerHeight-1) * m_AdaptorBlockHeight)
        {
        BlockHeight = m_AdaptorHeight - (uint32_t)pi_PosBlockY;
        }

    // Create a codec to compress the source data in RLE1
    HFCPtr<HCDCodec> pRLE1Codec = new HCDCodecHMRRLE1(BlockWidth, BlockHeight);
    ((HFCPtr<HCDCodecHMRRLE1>&)pRLE1Codec)->EnableLineIndexesTable(true);
    ((HFCPtr<HCDCodecHMRRLE1>&)pRLE1Codec)->SetLinePaddingBits(CalcPadding (BlockWidth, 8));

    // Create a Packet to hold RLE1 compressed data
    HFCPtr<HCDPacket> pRLE1Packet = new HCDPacket (pRLE1Codec, 0, 0);
    pRLE1Packet->SetBufferOwnership(true);

    // Compress Source Data
    Compress1BitData (pi_pData, pRLE1Packet);

    // Use WriteBlock with Packet
    Status = WriteBlock(pi_PosBlockX, pi_PosBlockY, pRLE1Packet);

    return Status;
    }

//-----------------------------------------------------------------------------
// Public
// WriteBlock
// Write a compressed block
//-----------------------------------------------------------------------------
HSTATUS HRFSLOStripEditor::WriteBlock(uint64_t            pi_PosBlockX,
                                      uint64_t            pi_PosBlockY,
                                      const HFCPtr<HCDPacket>&  pi_rpPacket,
                                      HFCLockMonitor const*     pi_pSisterFileLock)
    {
    HPRECONDITION (m_AccessMode.m_HasWriteAccess || m_AccessMode.m_HasCreateAccess );
    HPRECONDITION (pi_PosBlockX <= ULONG_MAX && pi_PosBlockY <= ULONG_MAX);

    HSTATUS Status = H_SUCCESS;

    // Create a copy of the source packet as the destination packet
    HFCPtr<HCDPacket> pTransformedPacket = new HCDPacket(*(pi_rpPacket.GetPtr()));

    // Set Ownership
    pTransformedPacket->SetBufferOwnership(true);

    // Transform Block back to it's original SLO
    TransformRLE1PacketFromSLO4 (pTransformedPacket);

    // Save block
    Status = SaveBlock ((uint32_t)pi_PosBlockX, (uint32_t)pi_PosBlockY, pTransformedPacket);

    return Status;
    }

//-----------------------------------------------------------------------------
// Private
// GetAdaptorBlockIndex
// Retur the index of a memory block.
//-----------------------------------------------------------------------------
void HRFSLOStripEditor::GetAdaptorBlockIndex (uint32_t pi_PosBlockX,
                                              uint32_t pi_PosBlockY,
                                              uint32_t* po_pColumnIndex,
                                              uint32_t* po_pLineIndex)
    {
    HPRECONDITION(m_AdaptorBlockWidth > 0);
    HPRECONDITION(m_AdaptorBlockHeight > 0);
    HPRECONDITION(pi_PosBlockX < m_AdaptorWidth);
    HPRECONDITION(pi_PosBlockY < m_AdaptorHeight);

    *po_pColumnIndex = pi_PosBlockX / m_AdaptorBlockWidth;
    *po_pLineIndex   = pi_PosBlockY / m_AdaptorBlockHeight;
    }

//-----------------------------------------------------------------------------
// Private
// CalcPadding
// Compute the number of padding bits
//-----------------------------------------------------------------------------
uint32_t HRFSLOStripEditor::CalcPadding (uint32_t pi_TotalWidth,
                                       uint32_t pi_Width )
    {
    HPRECONDITION (pi_Width != 0);
    uint32_t Padding;

    Padding = pi_Width - (pi_TotalWidth % pi_Width);

    if(Padding == pi_Width)
        {
        Padding = 0;
        }

    return Padding;
    }

//-----------------------------------------------------------------------------
// Private
// BuildRLE1Image
// Build an RLE1 stripped image in memory
//-----------------------------------------------------------------------------
HSTATUS HRFSLOStripEditor::BuildRLE1Image(BlockTable&   po_rppBlocks)
    {
    HPRECONDITION(po_rppBlocks == 0);
    HSTATUS Status = H_SUCCESS;

    // Allocate a Table to hold all blocks of the image
    AllocBlockTable (po_rppBlocks, m_AdaptorBlockPerHeight, m_AdaptorBlockPerWidth);

    if (m_ScanLineOrientation.IsScanlineHorizontal())
        {
        // Horizontal SLO
        Status = BuildRLE1HorizontalStrippedImage(po_rppBlocks);
        }
    else
        {
        // Vertical SLO
        BlockTable pppSourceStrips;
        AllocBlockTable (pppSourceStrips, m_SrcStripsPerHeight, m_SrcStripsPerWidth);

        Status = BuildRLE1VerticalStrippedImage (pppSourceStrips);

        if (Status == H_SUCCESS)
            BuildHorizontalStrips (po_rppBlocks, pppSourceStrips, m_AdaptorHeight, m_AdaptorWidth, m_AdaptorBlockHeight);

        // This table is no more required
        pppSourceStrips = 0;
        }

    // Flag that the image has been entirely read
    if (Status == H_SUCCESS)
        m_AllImageRead = true;
    else
        po_rppBlocks = 0;

    return Status;
    }

//-----------------------------------------------------------------------------
// Private
// BuildRLE1HorizontalStrippedImage
// Build an RLE1 stripped image in memory
//-----------------------------------------------------------------------------
HSTATUS HRFSLOStripEditor::BuildRLE1HorizontalStrippedImage(BlockTable&  pio_ppPackets)
    {
    // Read whole image and keep it compressed (RLE1) in memory
    HSTATUS Status      = H_SUCCESS;
    uint32_t y            = 0;
    uint32_t StripIndex   = 0;

    HFCPtr<HCDPacket>       pStripPacket;
    HFCPtr<HCDCodec>        pCodec;
    HArrayAutoPtr <Byte>  pBuffer;

    pBuffer = new Byte[m_SrcImageBytesPerWidth * m_SrcStripsHeight];

    do
        {
        uint32_t DestinationIndex;
        uint32_t CurrentBlockHeight = GetSrcStripHeight(0, y);

        pCodec = new HCDCodecHMRRLE1(m_SrcImageWidth, CurrentBlockHeight);

        ((HFCPtr<HCDCodecHMRRLE1>&)pCodec)->EnableLineIndexesTable(true);
        ((HFCPtr<HCDCodecHMRRLE1>&)pCodec)->SetLinePaddingBits(CalcPadding (m_SrcImageWidth, 8));

        // Create the packet
        pStripPacket = new HCDPacket(pCodec, 0, 0, 0);
        pStripPacket->SetBufferOwnership(true);

        // read an uncompressed 1 bit block
        Status = Read1BitBlock(0, y, pBuffer);

        if (Status == H_SUCCESS)
            {
            // Compress data
            Compress1BitData (pBuffer, pStripPacket);

            // Transform packet according to the current SLO
            TransformRLE1PacketToSlo4 (pStripPacket);

            // compute index where to store the new strip in memory
            if ((m_ScanLineOrientation.IsScanlineHorizontal() && m_ScanLineOrientation.IsLower()) ||
                (m_ScanLineOrientation.IsScanlineVertical()   && m_ScanLineOrientation.IsRight())  )
                {
                // Invert strip index
                DestinationIndex = m_SrcStripsPerHeight - StripIndex - 1;
                }
            else
                {
                DestinationIndex = StripIndex;
                }

            // Add the block to the block table
            AddBlock (pio_ppPackets, pStripPacket, DestinationIndex, 0);

            y += CurrentBlockHeight;
            StripIndex++;
            }
        }
    while(y < m_SrcImageHeight && Status == H_SUCCESS);

    return Status;
    }

//-----------------------------------------------------------------------------
// Private
// BuildRLE1VerticalStrippedImage
// Build an RLE1 stripped image in memory
//-----------------------------------------------------------------------------
HSTATUS HRFSLOStripEditor::BuildRLE1VerticalStrippedImage(BlockTable&  pio_ppPackets)
    {
    // Read whole image and keep it compressed (RLE1) in memory
    HSTATUS Status      = H_SUCCESS;
    uint32_t y            = 0;
    uint32_t StripIndex   = 0;

    HFCPtr<HCDPacket>       pStripPacket;
    HFCPtr<HCDCodec>        pCodec;
    HArrayAutoPtr <Byte>  pBuffer;
    HArrayAutoPtr <Byte>  pTransposedBuffer;

    pBuffer           = new Byte[m_SrcImageBytesPerWidth * m_SrcStripsHeight];
    pTransposedBuffer = new Byte[(uint32_t)ceil (m_SrcStripsHeight/8.0) * m_SrcImageWidth];

    do
        {
        uint32_t DestinationIndex;
        uint32_t CurrentBlockHeight = GetSrcStripHeight(0, y);

        // Create a codec with the dimension of the transposed data
        pCodec = new HCDCodecHMRRLE1(CurrentBlockHeight, m_SrcImageWidth);

        ((HFCPtr<HCDCodecHMRRLE1>&)pCodec)->EnableLineIndexesTable(true);
        ((HFCPtr<HCDCodecHMRRLE1>&)pCodec)->SetLinePaddingBits(CalcPadding (CurrentBlockHeight, 8));

        // Create the packet to compress the transposed data
        pStripPacket = new HCDPacket(pCodec, 0, 0, 0);
        pStripPacket->SetBufferOwnership(true);

        // Read an uncompressed block from the adapted file
        Status = Read1BitBlock(0, y, pBuffer);

        if (Status == H_SUCCESS)
            {
            // Transpose 1 bit buffer
            Transform1BitBuffer(pTransposedBuffer, pBuffer, m_SrcImageWidth, CurrentBlockHeight);

            // Compress to RLE1
            Compress1BitData (pTransposedBuffer, pStripPacket);

            // Compute index where to store the new strip
            if (m_ScanLineOrientation.IsScanlineVertical() && m_ScanLineOrientation.IsRight())
                {
                // Invert strip index
                DestinationIndex = m_SrcStripsPerHeight - StripIndex - 1;
                }
            else
                {
                DestinationIndex = StripIndex;
                }

            // Store the block in the block table
            AddBlock (pio_ppPackets, pStripPacket, DestinationIndex, 0);

            y += CurrentBlockHeight;
            StripIndex++;
            }
        }
    while(y < m_SrcImageHeight && Status == H_SUCCESS);

    return Status;
    }

//-----------------------------------------------------------------------------
// Private
// GetSrcStripHeight
// Compute the height of a strip in the adapted file
//-----------------------------------------------------------------------------
uint32_t HRFSLOStripEditor::GetSrcStripHeight(uint32_t x, uint32_t y)
    {
    uint32_t CurrentBlockHeight;

    if (m_ScanLineOrientation.IsScanlineHorizontal())
        {
        // Horizontal SLO
        if (m_ScanLineOrientation.IsUpper())
            {
            CurrentBlockHeight = MAX (0, MIN (m_SrcImageHeight - y, m_SrcStripsHeight));
            }
        else
            {
            if (m_SrcStripsPadding > 0)
                {
                if (y < (m_SrcImageHeight % m_SrcStripsHeight))
                    {
                    CurrentBlockHeight = m_SrcImageHeight % m_SrcStripsHeight;
                    }
                else
                    {
                    CurrentBlockHeight = m_SrcStripsHeight;
                    }
                }
            else
                {
                CurrentBlockHeight = m_SrcStripsHeight;
                }
            }
        }
    else
        {
        // Vertical SLO
        if (m_ScanLineOrientation.IsLeft())
            {
            CurrentBlockHeight = MAX (0, MIN (m_SrcImageHeight - y, m_SrcStripsHeight));
            }
        else
            {
            if (m_SrcStripsPadding > 0)
                {
                if (y < (m_SrcImageHeight % m_SrcStripsHeight))
                    {
                    CurrentBlockHeight = m_SrcImageHeight % m_SrcStripsHeight;
                    }
                else
                    {
                    CurrentBlockHeight = m_SrcStripsHeight;
                    }
                }
            else
                {
                CurrentBlockHeight = m_SrcStripsHeight;
                }
            }
        }

    return (CurrentBlockHeight);
    }

//-----------------------------------------------------------------------------
// Private
// Read1BitBlock
// Read an uncompressed data block from the adapted file
//-----------------------------------------------------------------------------
HSTATUS HRFSLOStripEditor::Read1BitBlock(uint32_t  x,
                                         uint32_t  y,
                                         Byte*   pio_pData)
    {
    HPRECONDITION(pio_pData != 0);
    HSTATUS Status            = H_SUCCESS;
    uint32_t LinesRead          = 0;
    uint32_t SourceBlockHeight  = m_SrcImageBlockHeight;
    uint32_t BytesPerLine       = m_SrcImageBytesPerWidth;
    uint32_t CurrentBlockHeight = GetSrcStripHeight(0, y);

    Byte* pData;

    // Read Line by line
    if ((m_ScanLineOrientation.IsScanlineHorizontal() && m_ScanLineOrientation.IsUpper()) ||
        (m_ScanLineOrientation.IsScanlineVertical()   && m_ScanLineOrientation.IsLeft())  )
        {
        // read lines sequentially
        pData = pio_pData;

        while (LinesRead < CurrentBlockHeight && Status == H_SUCCESS)
            {
            Status = m_pSrcResolutionEditor->ReadBlock(x, y, pData);

            y += SourceBlockHeight;
            pData += SourceBlockHeight * BytesPerLine;
            LinesRead += SourceBlockHeight;
            }
        }
    else
        {
        // Read lines Upside down
        // Take the adress of the last line in the uncompressed buffer
        pData = pio_pData + (CurrentBlockHeight-1) * BytesPerLine;

        while (LinesRead < CurrentBlockHeight && Status == H_SUCCESS)
            {
            Status = m_pSrcResolutionEditor->ReadBlock(x, y, pData);

            y += SourceBlockHeight;
            pData -= SourceBlockHeight * BytesPerLine;
            LinesRead += SourceBlockHeight;
            }
        }

    return Status;
    }

//-----------------------------------------------------------------------------
// Private
// TransformRLE1PacketToSlo4
// Apply a transformation depending on the Scanline Orientation
//-----------------------------------------------------------------------------
void HRFSLOStripEditor::TransformRLE1PacketToSlo4(HFCPtr<HCDPacket>& pio_pPacket)
    {
    switch (m_ScanLineOrientation.m_ScanlineOrientation)
        {
        case HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL:
            {
            // Nothing to do
            }
        break;
        case HRFScanlineOrientation::UPPER_RIGHT_HORIZONTAL:
            {
            // Perform Horizontal Swap on the packet
            HorizontalSwapRLE1Packet (pio_pPacket);
            }
        break;
        case HRFScanlineOrientation::LOWER_LEFT_HORIZONTAL:
            {
            // Nothing to do
            }
        break;
        case HRFScanlineOrientation::LOWER_RIGHT_HORIZONTAL:
            {
            // Perform Horizontal Swap on the packet
            HorizontalSwapRLE1Packet (pio_pPacket);
            }
        break;
        case HRFScanlineOrientation::UPPER_LEFT_VERTICAL:
            {
            // Transpose Packet
            TransposeRLE1Packet (pio_pPacket, false, false);
            }
        break;
        case HRFScanlineOrientation::UPPER_RIGHT_VERTICAL:
            {
            // Transpose Packet
            TransposeRLE1Packet (pio_pPacket, false, false);
            }
        break;
        case HRFScanlineOrientation::LOWER_LEFT_VERTICAL:
            {
            // Transpose Packet
            TransposeRLE1Packet (pio_pPacket, true, false);
            }
        break;
        case HRFScanlineOrientation::LOWER_RIGHT_VERTICAL:
            {
            // Transpose Packet
            TransposeRLE1Packet (pio_pPacket, true, false);
            }
        break;
        default:
            {
            HASSERT(false);
            }
        }
    }

//-----------------------------------------------------------------------------
// Private
// Transform1BitBuffer
// Apply a transformation on an uncompressed block,
// depending on the Scanline Orientation
//-----------------------------------------------------------------------------
void HRFSLOStripEditor::Transform1BitBuffer(Byte* po_pTransformedData,
                                            Byte* pi_pSourceData,
                                            uint32_t pi_dimX,
                                            uint32_t pi_dimY)
    {
    switch (m_ScanLineOrientation.m_ScanlineOrientation)
        {
        case HRFScanlineOrientation::UPPER_LEFT_VERTICAL:
            {
            // Transpose Buffer
            Transpose1BitBuffer (po_pTransformedData, pi_pSourceData, pi_dimX, pi_dimY);
            }
        break;
        case HRFScanlineOrientation::UPPER_RIGHT_VERTICAL:
            {
            // Transpose Buffer
            Transpose1BitBuffer (po_pTransformedData, pi_pSourceData, pi_dimX, pi_dimY);
            }
        break;
        case HRFScanlineOrientation::LOWER_LEFT_VERTICAL:
            {
            // Transpose and Flip Buffer
            TransposeAndFlip1BitBuffer (po_pTransformedData, pi_pSourceData, pi_dimX, pi_dimY);
            }
        break;
        case HRFScanlineOrientation::LOWER_RIGHT_VERTICAL:
            {
            // Transpose and Flip Buffer
            TransposeAndFlip1BitBuffer (po_pTransformedData, pi_pSourceData, pi_dimX, pi_dimY);
            }
        break;
        default:
            {
            HASSERT(false);
            }
        }
    }

//-----------------------------------------------------------------------------
// Private
// TransposeRLE1Packet
// Transpose a compressed block
//-----------------------------------------------------------------------------
void HRFSLOStripEditor::TransposeRLE1Packet(HFCPtr<HCDPacket>& pio_pPacket,
                                            bool              pi_Flip,
                                            bool              pi_Swap)
    {
    HPRECONDITION(pio_pPacket != 0);
    HPRECONDITION(pio_pPacket->GetCodec() != 0);
    HPRECONDITION(pio_pPacket->GetCodec()->GetClassID() == HCDCodecHMRRLE1::CLASS_ID);
    HPRECONDITION(((HFCPtr<HCDCodecHMRRLE1>&)pio_pPacket->GetCodec())->HasLineIndexesTable());

    HArrayAutoPtr <Byte> pTransposedData;
    HFCPtr<HCDPacket> pTransposedPacket;
    HFCPtr<HCDCodec>  pTransposedCodec;

    //  Original          Simple           Transposition    Transposition  Transposition
    //  Raw Data          Transposition    + Flip           + Swap         + Flip + Swap
    //  [1, 2, 3, 4]      [ 1, 5,  9 ]     [ 4, 8, 12 ]     [ 9, 5, 1]     [12, 8, 4]
    //  [5, 6, 7, 8]  =>  [ 2, 6, 10 ]     [ 3, 7, 11 ]     [10, 6, 2]     [11, 7, 3]
    //  [9,10,11,12]      [ 3, 7, 11 ]     [ 2, 6, 10 ]     [11, 7, 3]     [10, 6, 2]
    //                    [ 4, 8, 12 ]     [ 1, 5,  9 ]     [12, 8, 4]     [ 9, 5, 1]


    uint32_t PacketHeight = static_cast<uint32_t>(((HFCPtr<HCDCodecHMRRLE1>&) pio_pPacket->GetCodec())->GetHeight());
    uint32_t PacketWidth  = static_cast<uint32_t>(((HFCPtr<HCDCodecHMRRLE1>&) pio_pPacket->GetCodec())->GetWidth());

    uint32_t NewPacketHeight      = PacketWidth;
    uint32_t NewPacketWidth       = PacketHeight;
    uint32_t NewPaketWidthInBytes = (NewPacketWidth % 8) ? (NewPacketWidth / 8) + 1 : (NewPacketWidth / 8);
    uint32_t NewPaketSize         = NewPaketWidthInBytes * NewPacketHeight;

    // Create a buffer to store the transposed data
    pTransposedData = new Byte [NewPaketSize];
    memset(pTransposedData, 0, NewPaketSize * sizeof (Byte));

    uint32_t*         LineIndexTable = ((HFCPtr<HCDCodecHMRRLE1>&)pio_pPacket->GetCodec())->GetLineIndexesTable();
    unsigned short* pSourceBuffer  = reinterpret_cast<unsigned short*>(pio_pPacket->GetBufferAddress());
    unsigned short* pSourceLine;

    // Transpose the source packet.
    if (!pi_Flip && !pi_Swap)
        {
        // Proceed to simple transposition
        for (uint32_t x=0; x<PacketHeight; ++x)
            {
            pSourceLine = pSourceBuffer + LineIndexTable[x];
            TransposeRLE1Line (pTransposedData, pSourceLine, x, PacketWidth, NewPaketWidthInBytes);
            }
        }
    else if(pi_Flip && !pi_Swap)
        {
        // Transpose and flip (Vertical)
        for (uint32_t x=0; x<PacketHeight; ++x)
            {
            pSourceLine = pSourceBuffer + LineIndexTable[x];
            TransposeAndFlipRLE1Line (pTransposedData, pSourceLine, x, PacketWidth, NewPaketWidthInBytes);
            }
        }
    else if (!pi_Flip && pi_Swap)
        {
        // Transpose and Swap
        for (uint32_t x=0, srcLine=PacketHeight-1; x<=PacketHeight-1; ++x, --srcLine)
            {
            pSourceLine = pSourceBuffer + LineIndexTable[srcLine];
            TransposeRLE1Line (pTransposedData, pSourceLine, x, PacketWidth, NewPaketWidthInBytes);
            }
        }
    else
        {
        // Transpose Flip + Swap
        for (uint32_t x=0, srcLine=PacketHeight-1; x<=PacketHeight-1; ++x, --srcLine)
            {
            pSourceLine = pSourceBuffer + LineIndexTable[srcLine];
            TransposeAndFlipRLE1Line (pTransposedData, pSourceLine, x, PacketWidth, NewPaketWidthInBytes);
            }
        }

    // Create the transposed packet and codec
    pTransposedCodec  = new HCDCodecHMRRLE1(NewPacketHeight, NewPacketWidth);
    ((HFCPtr<HCDCodecHMRRLE1>&)pTransposedCodec)->SetLinePaddingBits(CalcPadding (NewPacketHeight, 8));
    pTransposedPacket = new HCDPacket(pTransposedCodec, pTransposedData, NewPaketSize, NewPaketSize);

    pTransposedPacket->SetBufferOwnership(true);
    pTransposedData.release();

    //Create the output packet
    pio_pPacket = new HCDPacket(new HCDCodecHMRRLE1(NewPacketWidth, NewPacketHeight), NULL, 0, 0);

    // Enable LineIndexTable
    ((HFCPtr<HCDCodecHMRRLE1>&)(pio_pPacket->GetCodec()))->EnableLineIndexesTable(true);
    ((HFCPtr<HCDCodecHMRRLE1>&)(pio_pPacket->GetCodec()))->SetLinePaddingBits(CalcPadding(NewPacketWidth, 8));

    // Allow the packet to dispose it's buffer
    pio_pPacket->SetBufferOwnership(true);

    // Compress RLE1
    pTransposedPacket->Compress(pio_pPacket);

    // Shrink the buffer
    pio_pPacket->ShrinkBufferToDataSize();
    }

//-----------------------------------------------------------------------------
// Private
// TransposeRLE1Line
// Transpose a compressed line.
// The transposed is returned uncompressed
//-----------------------------------------------------------------------------
void HRFSLOStripEditor::TransposeRLE1Line(Byte*   po_pTransposedData,
                                          unsigned short*  pi_pSourceRLE1Line,
                                          uint32_t  pi_LineNumber,
                                          uint32_t  pi_NumberOfPixels,
                                          uint32_t  pi_TransposedDataWidthInBytes)
    {
    unsigned short* pRun           = pi_pSourceRLE1Line;
    uint32_t NumberOfPixels = pi_NumberOfPixels;
    Byte*  pByte          = po_pTransposedData + pi_LineNumber / 8;
    bool    State          = false;
    Byte   Mask           = 0x80 >> (pi_LineNumber % 8);

    //  Original data           Will become...
    //  [1, 2, 3, 4]     =>     [1]
    //                          [2]
    //                          [3]
    //                          [4]

    while (NumberOfPixels > 0)
        {
        // Process only foreground pixels
        if (State)
            {
            for (uint32_t x=0; x<*pRun; ++x)
                {
                *pByte = *pByte | Mask;
                pByte += pi_TransposedDataWidthInBytes;
                }
            }
        else
            {
            pByte += *pRun * pi_TransposedDataWidthInBytes;
            }

        NumberOfPixels -= *pRun;
        ++pRun;
        State = !State;
        }
    }

//-----------------------------------------------------------------------------
// Private
// TransposeAndFlipRLE1Line
//-----------------------------------------------------------------------------
void HRFSLOStripEditor::TransposeAndFlipRLE1Line    (Byte*   po_pTransposedData,
                                                     unsigned short*  pi_pSourceRLE1Line,
                                                     uint32_t  pi_LineNumber,
                                                     uint32_t  pi_NumberOfPixels,
                                                     uint32_t  pi_TransposedDataWidthInBytes)
    {
    unsigned short* pRun           = pi_pSourceRLE1Line;
    uint32_t NumberOfPixels = pi_NumberOfPixels;
    Byte*  pByte          =   (po_pTransposedData)
                                + (pi_NumberOfPixels - 1) * (pi_TransposedDataWidthInBytes)
                                + (pi_LineNumber / 8);
    bool    State          = false;
    Byte   Mask           = m_BitMask[pi_LineNumber % 8];

    //  Original data           Will become...
    //  [1, 2, 3, 4]     =>     [4]
    //                          [3]
    //                          [2]
    //                          [1]
    //

    while (NumberOfPixels > 0)
        {
        if (State)
            {
            for (uint32_t x=0; x<*pRun; ++x)
                {
                *pByte = *pByte | Mask;
                pByte -= pi_TransposedDataWidthInBytes;
                }
            }
        else
            {
            pByte -= *pRun * pi_TransposedDataWidthInBytes;
            }

        NumberOfPixels -= *pRun;
        ++pRun;
        State = !State;
        }
    }

//-----------------------------------------------------------------------------
// Private
// HorizontalSwapRLE1Packet
//-----------------------------------------------------------------------------
void HRFSLOStripEditor::HorizontalSwapRLE1Packet(HFCPtr<HCDPacket>& pio_pPacket)
    {
    HPRECONDITION(pio_pPacket != 0);
    HPRECONDITION(pio_pPacket->GetCodec() != 0);
    HPRECONDITION(pio_pPacket->GetCodec()->GetClassID() == HCDCodecHMRRLE1::CLASS_ID);
    HPRECONDITION(((HFCPtr<HCDCodecHMRRLE1>&)pio_pPacket->GetCodec())->HasLineIndexesTable());

    uint32_t PacketHeight = static_cast<uint32_t>(((HFCPtr<HCDCodecHMRRLE1>&) pio_pPacket->GetCodec())->GetHeight());

    // Use pre-define "unsigned short" to be sure to have an unsigned 16bits data type.
    unsigned short* BufferAdress = reinterpret_cast<unsigned short*>(pio_pPacket->GetBufferAddress());
    uint32_t* LineIndexTable = ((HFCPtr<HCDCodecHMRRLE1>&)pio_pPacket->GetCodec())->GetLineIndexesTable();

    // Invert data horizontaly into a block

    //  Original data            Will become...
    //  [ 1, 2, 3, 4]            [ 4, 3, 2, 1]
    //  [ 5, 6, 7, 8]      =>    [ 8, 7, 6, 5]
    //  [ 9,10,11,12]            [12,11,10, 9]
    //
    for (uint32_t LineIndex = 0; LineIndex < PacketHeight; ++LineIndex)
        {
        unsigned short* pStart;
        unsigned short* pEnd;

        // Take the address of the beginning of the line
        pStart = BufferAdress + LineIndexTable[LineIndex];

        // Take the address of the end of the line
        if (LineIndex == PacketHeight - 1)
            pEnd = pStart + ((pio_pPacket->GetDataSize() >> 1) - LineIndexTable[LineIndex] - 1);
        else
            pEnd = pStart + (LineIndexTable[LineIndex + 1] - LineIndexTable[LineIndex] - 1);

        // Swap values
        while (pEnd > pStart)
            {
            unsigned short Run;

            Run    = *pStart;
            *pStart = *pEnd;
            *pEnd   =  Run;

            ++pStart;
            --pEnd;
            }
        }
    }

//-----------------------------------------------------------------------------
// Private
// BuildHorizontalStrips
// Construct horizontal strips from vertical strips
//-----------------------------------------------------------------------------
HSTATUS HRFSLOStripEditor::BuildHorizontalStrips (BlockTable& po_rpppBlocks,
                                                  BlockTable& pi_rpppBlocks,
                                                  uint32_t    pi_height,
                                                  uint32_t    pi_width,
                                                  uint32_t    pi_StripHeight)
    {
    HSTATUS Status = H_SUCCESS;

    uint32_t HorizStripsPerHeight = (uint32_t)ceil (pi_height / (double) pi_StripHeight);
    uint32_t VertStripsPerWidth   = (uint32_t)ceil (pi_width  / (double) pi_StripHeight);

    // For all new horizontal strips
    // Process them from bottom to top and trunk the source buffers after being copied to save memory
    for (int32_t HorzStripIndex = HorizStripsPerHeight-1; HorzStripIndex >= 0; HorzStripIndex--)
        {
        HArrayAutoPtr <Byte>  pBuffer;
        HArrayAutoPtr <uint32_t>  pLit; //Lit: LineIndexTable
        uint32_t NewStripHeight  = MIN (pi_StripHeight, pi_height - HorzStripIndex * pi_StripHeight);
        uint32_t SourceLineIndex = HorzStripIndex * pi_StripHeight;
        size_t CurrentDataSize = 0;

        // Create a codec for the new strip
        HFCPtr<HCDCodecHMRRLE1> pNewCodec = new HCDCodecHMRRLE1 (pi_width, NewStripHeight);
        pNewCodec->EnableLineIndexesTable(true);
        pNewCodec->SetLinePaddingBits(CalcPadding (pi_width, 8));

        // Evaluate the size of the buffer required for the new strip
        size_t NewBufferSize = (uint32_t)(ceil (pi_width/8.0) * (double)pi_StripHeight * pNewCodec->GetEstimatedCompressionRatio());

        // Allocate a buffer for the current packet
        pBuffer = new Byte[NewBufferSize];

        // Allocate a Line Index Table
        pLit = new uint32_t[NewStripHeight];

        // for all lines in the new horizontal strip
        for (uint32_t Line = 0; Line < NewStripHeight; Line++)
            {
            size_t TotalLineSize = 0;

            // for all existing vertical strips, copy the line
            for (uint32_t VertStripIndex = 0; VertStripIndex < VertStripsPerWidth; VertStripIndex++)
                {
                uint32_t                IndexOfLineToCopy = SourceLineIndex + Line;
                HFCPtr<HCDPacket>       pVertStrip      = pi_rpppBlocks[VertStripIndex][0];
                HFCPtr<HCDCodecHMRRLE1> pVertCodec      = ((HFCPtr<HCDCodecHMRRLE1>&)pVertStrip->GetCodec());
                uint32_t*                 VertStripLIT    = pVertCodec->GetLineIndexesTable();
                Byte*                 PosToCopyFrom   = pVertStrip->GetBufferAddress() + VertStripLIT[IndexOfLineToCopy] * sizeof(unsigned short);
                uint32_t                VertStripHeight = static_cast<uint32_t>(pVertCodec->GetHeight());
                Byte*                 PosToCopy;
                size_t                  SizeOfLineToCopy;

                // Compute the size of the current line to copy
                if (IndexOfLineToCopy != VertStripHeight-1)
                    {
                    SizeOfLineToCopy = (VertStripLIT[IndexOfLineToCopy + 1] - VertStripLIT[IndexOfLineToCopy]) * sizeof (unsigned short);
                    }
                else
                    {
                    SizeOfLineToCopy = pVertStrip->GetDataSize() - VertStripLIT[IndexOfLineToCopy] * sizeof (unsigned short);
                    }

                // Check if there is enough space to copy the current line
                if (NewBufferSize < CurrentDataSize + SizeOfLineToCopy + 3 * sizeof (unsigned short))
                    {
                    // realloc the buffer
                    Byte* pTmpBuffer;
                    size_t TmpBufferSize =  (size_t)(NewBufferSize
                                                     + SizeOfLineToCopy
                                                     + ((uint32_t)ceil (pi_width/8.0) * pi_StripHeight * HorzStripIndex)
                                                     * pNewCodec->GetEstimatedCompressionRatio());

                    // Try to realloc memory
                    pTmpBuffer = (Byte*) renew(pBuffer, NewBufferSize, TmpBufferSize);

                    pBuffer.release();
                    pBuffer       = pTmpBuffer;
                    NewBufferSize = TmpBufferSize;
                    }

                PosToCopy = pBuffer + CurrentDataSize;

                // Append two RLE1 lines
                if (VertStripIndex)
                    {
                    unsigned short* pRun = (unsigned short*)PosToCopyFrom;
                    unsigned short* pPreviousRun = (unsigned short*)PosToCopy - 1;
                    uint32_t length;

                    // check if last line section end with a zero length run and
                    // the current line section begin with a zero length run
                    if (*pRun == 0 && *pPreviousRun == 0)
                        {
                        // remove zero run from current and previous line section
                        pRun++;
                        pPreviousRun--;
                        CurrentDataSize  -= sizeof(unsigned short);
                        PosToCopyFrom    += sizeof(unsigned short);
                        SizeOfLineToCopy -= sizeof(unsigned short);
                        TotalLineSize    -= sizeof(unsigned short);
                        }

                    // add the length of the last line section and the
                    // current line section
                    length = *pPreviousRun + *pRun;
                    if (length > 32767)
                        {
                        (*pPreviousRun) = 32767;
                        pPreviousRun++;
                        (*pPreviousRun) = 0;
                        pPreviousRun++;
                        length -= 32767;
                        CurrentDataSize += 2 * sizeof(unsigned short);
                        TotalLineSize   += 2 * sizeof(unsigned short);
                        }

                    // adjust the last value of the previous line section
                    (*pPreviousRun) = (unsigned short)length;

                    PosToCopyFrom += sizeof(unsigned short);
                    SizeOfLineToCopy -= sizeof(unsigned short);
                    }

                PosToCopy = pBuffer + CurrentDataSize;

                // Copy source data to the output buffer
                memcpy (PosToCopy, PosToCopyFrom, SizeOfLineToCopy);
                CurrentDataSize += SizeOfLineToCopy;

                TotalLineSize += SizeOfLineToCopy;
                }

            // Adjust LineIndexTable of the new horizontal strip
            HASSERT_X64(((CurrentDataSize - TotalLineSize) >> 1) < ULONG_MAX);
            pLit[Line] = (uint32_t)((CurrentDataSize - TotalLineSize) >> 1);
            }
        pNewCodec->SetLineIndexesTable(pLit);

        // create a packet for the new strip
        HFCPtr<HCDPacket> pPacket = new HCDPacket((HFCPtr<HCDCodec>)pNewCodec, pBuffer, NewBufferSize, CurrentDataSize);
        pPacket->SetBufferOwnership(true);

        // release buffer
        pBuffer.release();

        pPacket->ShrinkBufferToDataSize();

        // save new strip
        AddBlock (po_rpppBlocks, pPacket, HorzStripIndex, 0);

        // Shrink source buffers to save memory
        for (uint32_t Index = 0; Index < VertStripsPerWidth; Index++)
            {
            uint32_t* LineIndexTable;
            LineIndexTable = ((HFCPtr<HCDCodecHMRRLE1>&) pi_rpppBlocks[Index][0]->GetCodec())->GetLineIndexesTable();
            pi_rpppBlocks[Index][0]->SetDataSize(LineIndexTable[SourceLineIndex] * sizeof(unsigned short));
            pi_rpppBlocks[Index][0]->ShrinkBufferToDataSize();
            }
        }

    return Status;
    }

//-----------------------------------------------------------------------------
// Private
// AllocBlockTable
// Allocate memory to store image blocks
//-----------------------------------------------------------------------------
void HRFSLOStripEditor::AllocBlockTable(BlockTable& pio_ppBlock,
                                        uint32_t    pi_Line,
                                        uint32_t    pi_Column)
    {
    pio_ppBlock = new HArrayAutoPtr<HFCPtr<HCDPacket> > [pi_Line];

    // Allocate blocks
    for (uint32_t NoTile=0; NoTile < pi_Line; NoTile++)
        pio_ppBlock[NoTile] = new HFCPtr<HCDPacket>[pi_Column];

    }

//-----------------------------------------------------------------------------
// Private
// DeleteBlockTable
// Release memory
//-----------------------------------------------------------------------------
void HRFSLOStripEditor::DeleteBlockTable(BlockTable& pio_ppBlock)
    {
    pio_ppBlock = 0;
    }

//-----------------------------------------------------------------------------
// Private
// AddBlock
// Add a block to memory
//-----------------------------------------------------------------------------
void HRFSLOStripEditor::AddBlock(BlockTable&        pio_ppBlock,
                                 HFCPtr<HCDPacket>& pi_pPacket,
                                 uint32_t           pi_pAdaptorLineIndex,
                                 uint32_t           pi_pAdaptorColumnIndex)
    {
    pio_ppBlock[pi_pAdaptorLineIndex][pi_pAdaptorColumnIndex] = pi_pPacket;
    }

//-----------------------------------------------------------------------------
// Private
// Compress1BitData
// Compress a block to RLE1
//-----------------------------------------------------------------------------
void HRFSLOStripEditor::Compress1BitData    (const Byte*      pi_pData,
                                             HFCPtr<HCDPacket>& pio_pPacket)
    {
    HPRECONDITION(pio_pPacket != 0);
    HPRECONDITION(pio_pPacket->GetCodec() != 0);
    HPRECONDITION(pio_pPacket->GetCodec()->GetClassID() == HCDCodecHMRRLE1::CLASS_ID);

    size_t UncompressedSize = (size_t)ceil (((HFCPtr<HCDCodecHMRRLE1>&) pio_pPacket->GetCodec())->GetWidth() / 8.0) *
                              ((HFCPtr<HCDCodecHMRRLE1>&) pio_pPacket->GetCodec())->GetHeight();


    HCDPacket UncompressedPacket((Byte*)pi_pData,  UncompressedSize, UncompressedSize);

    UncompressedPacket.SetBufferOwnership(false);

    // Reset codec to it's original value!
    pio_pPacket->GetCodec()->Reset();

    // Reset packet datasize
    pio_pPacket->SetDataSize(0);

    // Recompress to RLE1
    UncompressedPacket.Compress(pio_pPacket);

    // Shrink the buffer
    pio_pPacket->ShrinkBufferToDataSize();
    }

//-----------------------------------------------------------------------------
// Private
// Transpose1BitBuffer
// Transpose an uncompressed block
//-----------------------------------------------------------------------------
void    HRFSLOStripEditor::Transpose1BitBuffer  (Byte*   po_pDest,
                                                 Byte*   pi_pSource,
                                                 uint32_t  pi_dimX,
                                                 uint32_t  pi_dimY)
    {
    uint32_t rotatedDimX = pi_dimY;
    uint32_t rotatedDimY = pi_dimX;

    uint32_t BytesPerDstLine = (uint32_t)ceil (rotatedDimX/8.0);
    uint32_t DstBufferSize   = (uint32_t)ceil (rotatedDimX/8.0) * rotatedDimY;

    memset(po_pDest, 0, DstBufferSize);

    Byte* pSrc =  pi_pSource;
    Byte* pDst;
    uint32_t Nbits;

    //  Original          Simple
    //  Data              Transposition
    //  [1, 2, 3, 4]      [ 1, 5,  9 ]
    //  [5, 6, 7, 8]  =>  [ 2, 6, 10 ]
    //  [9,10,11,12]      [ 3, 7, 11 ]
    //                    [ 4, 8, 12 ]

    //process each source pixels
    for (uint32_t line=0; line<pi_dimY; ++line)
        {
        int32_t BitToSet = line%8;

        for (uint32_t col=0; col<pi_dimX; col+=8, ++pSrc)
            {
            if (*pSrc != 0)
                {
                pDst = po_pDest + col * BytesPerDstLine + (line/8);

                Nbits = MIN(8, pi_dimX-col);

                for (uint32_t bit=0; bit<Nbits; ++bit)
                    {
                    if (GetBit (pSrc, bit))
                        {
                        SetBitOn (pDst, BitToSet);
                        }

                    pDst += BytesPerDstLine;
                    }
                }
            }
        }
    }

//-----------------------------------------------------------------------------
// Private
// TransposeAndFlip1BitBuffer
// Transpose and flip an uncompressed block
//-----------------------------------------------------------------------------
void HRFSLOStripEditor::TransposeAndFlip1BitBuffer  (Byte*   po_pDest,
                                                     Byte*   pi_pSource,
                                                     uint32_t  pi_dimX,
                                                     uint32_t  pi_dimY)
    {
    uint32_t rotatedDimX = pi_dimY;
    uint32_t rotatedDimY = pi_dimX;

    //UInt32 SrcBufferSize   = (UInt32)ceil (pi_dimX/8.0) * pi_dimY;
    uint32_t BytesPerDstLine = (uint32_t)ceil (rotatedDimX/8.0);
    uint32_t DstBufferSize   = (uint32_t)ceil (rotatedDimX/8.0) * rotatedDimY;

    memset(po_pDest, 0, DstBufferSize);

    Byte* pSrc =  pi_pSource;
    Byte* pDst;
    uint32_t Nbits;

    //  Original          Transposition
    //  Raw Data          + Flip
    //  [1, 2, 3, 4]      [ 4, 8, 12 ]
    //  [5, 6, 7, 8]  =>  [ 3, 7, 11 ]
    //  [9,10,11,12]      [ 2, 6, 10 ]
    //                    [ 1, 5,  9 ]

    //process each source pixels
    for (uint32_t line=0; line<pi_dimY; ++line)
        {
        int32_t BitToSet = line%8;

        for (uint32_t col=0; col<pi_dimX; col+=8, ++pSrc)
            {
            if (*pSrc != 0)
                {
                pDst = po_pDest + (pi_dimX-col-1) * BytesPerDstLine + (line/8);

                Nbits = MIN(8, pi_dimX-col);

                for (uint32_t bit=0; bit<Nbits; ++bit)
                    {
                    if (GetBit (pSrc, bit))
                        {
                        SetBitOn (pDst, BitToSet);
                        }

                    pDst -= BytesPerDstLine;
                    }
                }
            }
        }
    }

//-----------------------------------------------------------------------------
// Private
// TransformRLE1PacketFromSLO4
// Transform data from SLO 4 to the current SLO.
//-----------------------------------------------------------------------------
void HRFSLOStripEditor::TransformRLE1PacketFromSLO4 (HFCPtr<HCDPacket>& pio_pPacket)
    {

    switch (m_ScanLineOrientation.m_ScanlineOrientation)
        {
        case HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL:
            {
            // Nothing to do
            }
        break;
        case HRFScanlineOrientation::UPPER_RIGHT_HORIZONTAL:
            {
            // From SLO 4 to SLO 5
            // Perform Horizontal Swap on the packet
            HorizontalSwapRLE1Packet (pio_pPacket);
            }
        break;
        case HRFScanlineOrientation::LOWER_LEFT_HORIZONTAL:
            {
            // Nothing to do
            }
        break;
        case HRFScanlineOrientation::LOWER_RIGHT_HORIZONTAL:
            {
            // From SLO 4 to SLO 7
            // Perform Horizontal Swap on the packet
            HorizontalSwapRLE1Packet (pio_pPacket);
            }
        break;
        case HRFScanlineOrientation::UPPER_LEFT_VERTICAL:
            {
            // From SLO 4 to SLO 0
            // Transpose Packet
            TransposeRLE1Packet (pio_pPacket, false, false);
            }
        break;
        case HRFScanlineOrientation::UPPER_RIGHT_VERTICAL:
            {
            // From SLO 4 to SLO 1
            // Transpose (Flip) Packet
            TransposeRLE1Packet (pio_pPacket, true, false);
            }
        break;
        case HRFScanlineOrientation::LOWER_LEFT_VERTICAL:
            {
            // From SLO 4 to SLO 2
            // Transpose (Swap) Packet
            TransposeRLE1Packet (pio_pPacket, false, true);
            }
        break;
        case HRFScanlineOrientation::LOWER_RIGHT_VERTICAL:
            {
            // From SLO 4 to SLO 3
            // Transpose (Flip and Swap) Packet
            TransposeRLE1Packet (pio_pPacket, true, true);
            }
        break;
        default:
            {
            HASSERT(false);
            }
        }
    }

//-----------------------------------------------------------------------------
// Private
// SaveBlock
// Write a data block to the adapted file
//-----------------------------------------------------------------------------
HSTATUS HRFSLOStripEditor::SaveBlock   (uint32_t           pi_PosBlockX,
                                        uint32_t           pi_PosBlockY,
                                        HFCPtr<HCDPacket>& pi_rpTransformedPacket)
    {
    HSTATUS Status = H_SUCCESS;

    // SLO 5 can be written immediately
    if (m_ScanLineOrientation.m_ScanlineOrientation == HRFScanlineOrientation::UPPER_RIGHT_HORIZONTAL)
        {
        Status = sWriteBlock(pi_PosBlockX, pi_PosBlockY, pi_rpTransformedPacket);
        }
    else
        {
        // All SLO but SLO 5
        // Compute the index where to store the packet
        uint32_t BlockIndex;

        if (m_ScanLineOrientation.IsUpper())
            {
            BlockIndex = pi_PosBlockY / m_AdaptorBlockHeight;
            }
        else
            {
            BlockIndex = m_AdaptorBlockPerHeight - pi_PosBlockY / m_AdaptorBlockHeight - 1;
            }

        // Allocate a temp block table
        if (m_ppTempBlocks == 0)
            AllocBlockTable (m_ppTempBlocks, m_AdaptorBlockPerHeight, m_AdaptorBlockPerWidth);  // AllocBlockTable (m_ppTempBlocks, m_SrcStripsPerHeight, m_SrcStripsPerWidth);


        AddBlock (m_ppTempBlocks, pi_rpTransformedPacket, BlockIndex, 0);

        // if we just stored the last block, write them to the adapted file
        if ( (m_ScanLineOrientation.IsUpper() && BlockIndex == m_AdaptorBlockPerHeight - 1) ||
             (m_ScanLineOrientation.IsLower() && BlockIndex == 0))
            {
            if (m_ScanLineOrientation.IsScanlineVertical())
                {
                AllocBlockTable(m_ppBlocks, m_SrcStripsPerHeight, m_SrcStripsPerWidth);
                BuildHorizontalStrips  (m_ppBlocks, m_ppTempBlocks, m_SrcImageHeight, m_SrcImageWidth, m_AdaptorBlockHeight);

                }
            else
                {
                m_ppBlocks = m_ppTempBlocks;
                }

            // Delete Temp Block Table
            m_ppTempBlocks = 0;

            // Write all blocks
            uint32_t posX = 0;
            uint32_t posY = 0;

            // First block may be smaller than other one...
            uint32_t BlockWrittenHeight = static_cast<uint32_t>(MAX (0, MIN(m_AdaptorBlockHeight, ((HFCPtr<HCDCodecImage>&)(pi_rpTransformedPacket->GetCodec()))->GetHeight())));

            for (uint32_t x=0; x<m_SrcStripsPerHeight && Status == H_SUCCESS; ++x)
                {
                Status = sWriteBlock(posX, posY, m_ppBlocks[x][0]);

                posY += BlockWrittenHeight; // m_AdaptorBlockHeight;
                BlockWrittenHeight = m_AdaptorBlockHeight;

                // Delete the  packet
                m_ppBlocks[x][0] = 0;
                }
            }
        }

    return Status;
    }

//-----------------------------------------------------------------------------
// Private
// sWriteBlock
// Write a data block to the adapted file
//-----------------------------------------------------------------------------
HSTATUS HRFSLOStripEditor::sWriteBlock (uint32_t           pi_PosBlockX,
                                        uint32_t           pi_PosBlockY,
                                        HFCPtr<HCDPacket>& rpPacket)
    {
    HSTATUS Status = H_SUCCESS;
    HArrayAutoPtr<Byte> pData;
    uint32_t x = pi_PosBlockX;
    uint32_t y = pi_PosBlockY;

    // If the packet is compressed, uncompress it
    if ((rpPacket->GetCodec() != 0) && (rpPacket->GetCodec()->GetClassID() != HCDCodecIdentity::CLASS_ID))
        {
        // The packet is compressed
        size_t Width  = ((HFCPtr<HCDCodecImage>&) (rpPacket->GetCodec()))->GetWidth();
        size_t Height = ((HFCPtr<HCDCodecImage>&) (rpPacket->GetCodec()))->GetHeight();
        size_t BufferSize = (uint32_t)ceil(Width/8.0) * Height;

        pData = new Byte[BufferSize];

        // Define a new packet to uncompress the data
        HCDPacket UncompressedPacket(pData, BufferSize, BufferSize);
        UncompressedPacket.SetBufferOwnership(false);
        rpPacket->Decompress(&UncompressedPacket);
        }
    else
        {
        pData = rpPacket->GetBufferAddress();
        pData.release();
        }


    uint32_t CurrentBlockHeight;
    CurrentBlockHeight = static_cast<uint32_t>(MAX (0, MIN(m_AdaptorBlockHeight, ((HFCPtr<HCDCodecImage>&)(rpPacket->GetCodec()))->GetHeight())));

    uint32_t LinesWritten = 0;
    Byte* pBlock = pData;

    if (m_ScanLineOrientation.IsUpper() || m_ScanLineOrientation.IsScanlineVertical())
        {
        // write lines sequentially
        while (LinesWritten < CurrentBlockHeight && Status == H_SUCCESS)
            {
            Status = m_pSrcResolutionEditor->WriteBlock(x, y, pBlock);

            y            += m_SrcImageBlockHeight;
            pBlock       += m_SrcImageBlockHeight * m_SrcImageBytesPerWidth;
            LinesWritten += m_SrcImageBlockHeight;
            }
        }
    else
        {
        // Write lines Upside down
        // Take the adress of the last line in the uncompressed buffer
        pBlock = pData + (CurrentBlockHeight-1) * m_SrcImageBytesPerWidth;

        while (LinesWritten < CurrentBlockHeight && Status == H_SUCCESS)
            {
            Status = m_pSrcResolutionEditor->WriteBlock(x, y, pBlock);

            y            += m_SrcImageBlockHeight;
            pBlock       -= m_SrcImageBlockHeight * m_SrcImageBytesPerWidth;
            LinesWritten += m_SrcImageBlockHeight;
            }
        }

    return Status;
    }
