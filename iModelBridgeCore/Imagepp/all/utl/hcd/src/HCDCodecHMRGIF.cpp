//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hcd/src/HCDCodecHMRGIF.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for class HCDCodecHMRGIF
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HCDCodecHMRGIF.h>
#include <Imagepp/all/h/HFCMath.h>


// NOTE: THERE ARE A LOT OF CONSTANT NUMBERS; I DO NOT MAKE "DEFINE" WITH THEM BECAUSE I DON'T
// KNOW WHAT THEY MEAN. THE ALGO COME FROM THE WEB.

//-----------------------------------------------------------------------------
// public
// Default constructor
//-----------------------------------------------------------------------------
HCDCodecHMRGIF::HCDCodecHMRGIF()
    : HCDCodecGIF()
    {
    m_BitsPerPixel = 0;

    InitObject();
    }

//-----------------------------------------------------------------------------
// public
// Default constructor
//-----------------------------------------------------------------------------
HCDCodecHMRGIF::HCDCodecHMRGIF(uint32_t pi_Width,
                               uint32_t pi_Height,
                               uint32_t pi_BitsPerPixel)
    : HCDCodecGIF(pi_Width, pi_Height, pi_BitsPerPixel)
    {
    m_BitsPerPixel = (Byte)pi_BitsPerPixel;

    InitObject();
    }

//-----------------------------------------------------------------------------
// public
// Copy constructor
//-----------------------------------------------------------------------------
HCDCodecHMRGIF::HCDCodecHMRGIF(const HCDCodecHMRGIF& pi_rObj)
    : HCDCodecGIF(pi_rObj)
    {
    m_BitsPerPixel  = pi_rObj.m_BitsPerPixel;

    m_DecompressMinCodeSize = pi_rObj.m_DecompressMinCodeSize;

    m_BytesUnread  = pi_rObj.m_BitsPerPixel;
    m_MaxCode      = pi_rObj.m_MaxCode;
    m_CodeSize     = pi_rObj.m_CodeSize;
    m_ClearCode    = pi_rObj.m_ClearCode;
    m_EofCode      = pi_rObj.m_EofCode;
    m_FirstFree    = pi_rObj.m_FirstFree;
    m_BitOffset    = pi_rObj.m_BitOffset;
    m_FreeCode     = pi_rObj.m_FreeCode;

    m_CodeBufferSize = pi_rObj.m_CodeBufferSize;
    if (m_CodeBufferSize != 0)
        {
        m_pCodeBuffer = new Byte[m_CodeBufferSize];
        memcpy(m_pCodeBuffer, pi_rObj.m_pCodeBuffer, m_CodeBufferSize * sizeof(Byte));
        }

    m_CodeTableSize = pi_rObj.m_CodeTableSize;
    if (m_CodeTableSize != 0)
        {
        m_pCodeTable = new CodeEntry[m_CodeTableSize];
        memcpy(m_pCodeTable, pi_rObj.m_pCodeTable, m_CodeTableSize * sizeof(CodeEntry));
        }

    m_CodeBufferSizeCompress = pi_rObj.m_CodeBufferSizeCompress;
    if (m_CodeBufferSizeCompress != 0)
        {
        m_pCodeBufferCompress = new Byte[m_CodeBufferSizeCompress];
        memcpy(m_pCodeBufferCompress, pi_rObj.m_pCodeBufferCompress, m_CodeBufferSizeCompress * sizeof(Byte));
        }

    m_CodeTableSizeCompress = pi_rObj.m_CodeTableSizeCompress;
    if (m_CodeTableSizeCompress != 0)
        {
        m_pCodeTableCompress = new CodeCompressEntry[m_CodeTableSizeCompress];
        memcpy(m_pCodeTableCompress, pi_rObj.m_pCodeTableCompress, m_CodeTableSizeCompress * sizeof(CodeCompressEntry));
        }

    m_BufferDataForNextBlockSize = pi_rObj.m_BufferDataForNextBlockSize;
    if (m_BufferDataForNextBlockSize != 0)
        {
        m_pBufferDataForNextBlock = (Byte*)malloc(m_BufferDataForNextBlockSize*sizeof(Byte));
        memcpy(m_pBufferDataForNextBlock, pi_rObj.m_pBufferDataForNextBlock, m_BufferDataForNextBlockSize * sizeof(Byte));
        }

    m_BufferDataForNextBlockPos  = pi_rObj.m_BufferDataForNextBlockPos;
    }

//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HCDCodecHMRGIF::~HCDCodecHMRGIF()
    {
    }

//-----------------------------------------------------------------------------
// public
// Clone
//-----------------------------------------------------------------------------
HCDCodec* HCDCodecHMRGIF::Clone() const
    {
    return new HCDCodecHMRGIF(*this);
    }

//-----------------------------------------------------------------------------
// public
// CompressSubset
//-----------------------------------------------------------------------------
size_t HCDCodecHMRGIF::CompressSubset(const  void* pi_pInData,
                                      size_t pi_InDataSize,
                                      void*  po_pOutBuffer,
                                      size_t po_OutBufferSize)
    {
    bool  Status = true;

    size_t InDataBufferPos  = 0;
    size_t OutDataBufferPos = 0;

    // Is it the first packet?
    if(GetSubsetPosY() == 0)
        {
        SetCurrentState(STATE_COMPRESS);

        m_CompresionStruct.MinCodeSize = (short)GetBitsPerPixel();
        if (m_CompresionStruct.MinCodeSize < 2 || m_CompresionStruct.MinCodeSize > 9)
            {
            if (m_CompresionStruct.MinCodeSize == 1)
                m_CompresionStruct.MinCodeSize = 2;
            else
                Status = false;
            }

        m_pCodeBufferCompress  = new Byte[256+3];
        m_CodeBufferSizeCompress  = 256+3;

        m_pCodeTableCompress = new struct CodeCompressEntry[TABLE_SIZE];

        if (m_pCodeTableCompress == NULL)
            Status = false;

        // Record the minimum code size
        if (Status)
            Status = SetByte(m_CompresionStruct.MinCodeSize, (Byte*)po_pOutBuffer, OutDataBufferPos, po_OutBufferSize);

        m_BitOffset = 0;

        if (Status)
            {
            InitTable(m_CompresionStruct.MinCodeSize);
            Status = WriteCode(m_ClearCode, (Byte*)po_pOutBuffer, OutDataBufferPos, po_OutBufferSize);
            m_CompresionStruct.SuffixChar = GetByte((Byte*)pi_pInData, InDataBufferPos, pi_InDataSize);
            }
        if (m_CompresionStruct.SuffixChar >= 0 && Status)
            {
            m_CompresionStruct.PrefixCode = m_CompresionStruct.SuffixChar;
            }
        else if (m_CompresionStruct.SuffixChar != -1)
            Status = false;

        }
    // Some byte have already been read in the last call, so we need to put them in the good buffer.
    if (m_BufferDataForNextBlockPos != 0)
        {
//        HASSERT(m_BufferDataForNextBlockPos < GetSubsetWidth());
        // We need to copy that is in the m_pBufferDataForNextBlock in po_pOutBuffer.

        // TO DOOOOOO !!!!!

        memcpy(po_pOutBuffer, m_pBufferDataForNextBlock, m_BufferDataForNextBlockPos);

        OutDataBufferPos = m_BufferDataForNextBlockPos;

        m_BufferDataForNextBlockPos  = 0;
        m_BufferDataForNextBlockSize = 0;
        free(m_pBufferDataForNextBlock);
        m_pBufferDataForNextBlock=0;
        }

    while ((InDataBufferPos < GetSubsetWidth()) && Status)
        {
        m_CompresionStruct.SuffixChar = GetByte((Byte*)pi_pInData, InDataBufferPos, pi_InDataSize);

        if (m_CompresionStruct.SuffixChar != m_EofCode)
            {
            m_CompresionStruct.hx = (m_CompresionStruct.PrefixCode ^ m_CompresionStruct.SuffixChar << 5) % TABLE_SIZE;
            m_CompresionStruct.d = 1;

            for (;;)
                {
                if (m_pCodeTableCompress[m_CompresionStruct.hx].code_id == 0)
                    {
                    Status = WriteCode(m_CompresionStruct.PrefixCode, (Byte*)po_pOutBuffer, OutDataBufferPos, po_OutBufferSize);

                    m_CompresionStruct.d = m_FreeCode;

                    if (m_FreeCode <= LARGEST_CODE)
                        {
                        m_pCodeTableCompress[m_CompresionStruct.hx].prior_code = m_CompresionStruct.PrefixCode;
                        m_pCodeTableCompress[m_CompresionStruct.hx].added_char = (unsigned char)m_CompresionStruct.SuffixChar;
                        m_pCodeTableCompress[m_CompresionStruct.hx].code_id = m_FreeCode;
                        m_FreeCode++;
                        }

                    if (m_CompresionStruct.d == m_MaxCode)
                        {
                        if (m_CodeSize < 12)
                            {
                            m_CodeSize++;
                            m_MaxCode <<= 1;
                            }
                        else
                            {
                            Status = WriteCode(m_ClearCode, (Byte*)po_pOutBuffer, OutDataBufferPos, po_OutBufferSize);
                            InitTable(m_CompresionStruct.MinCodeSize);
                            }
                        }
                    m_CompresionStruct.PrefixCode = m_CompresionStruct.SuffixChar;
                    break;
                    }

                if (m_pCodeTableCompress[m_CompresionStruct.hx].prior_code == m_CompresionStruct.PrefixCode &&
                    m_pCodeTableCompress[m_CompresionStruct.hx].added_char == m_CompresionStruct.SuffixChar)
                    {
                    m_CompresionStruct.PrefixCode = m_pCodeTableCompress[m_CompresionStruct.hx].code_id;
                    break;
                    }

                m_CompresionStruct.hx += m_CompresionStruct.d;
                m_CompresionStruct.d += 2;
                if (m_CompresionStruct.hx >= TABLE_SIZE)
                    m_CompresionStruct.hx -= TABLE_SIZE;
                }
            }
        else
            Status = false;
        }

    SetSubsetPosY(GetSubsetPosY() + GetSubsetHeight());

    // We compress the last block.
    if(GetSubsetPosY() == GetHeight())
        {
        Status = WriteCode(m_CompresionStruct.PrefixCode, (Byte*)po_pOutBuffer, OutDataBufferPos, po_OutBufferSize);

        if (Status)
            Status = WriteCode(m_EofCode, (Byte*)po_pOutBuffer, OutDataBufferPos, po_OutBufferSize);

        // Make sure the code buffer is flushed
        if (m_BitOffset > 0 && Status)
            {
            unsigned short LastBlockSize = (m_BitOffset + 7)/8;
            if (LastBlockSize <= MAXIMUM_BLOCK_SIZE)
                {
                Status = WriteBlock(LastBlockSize, (Byte*)po_pOutBuffer, OutDataBufferPos, po_OutBufferSize);
                }
            else //TR 225371 - If the last block is 255 bytes and some bits long, it should be written as
                //            two separated blocks.
                {
                Status = WriteBlock(MAXIMUM_BLOCK_SIZE, (Byte*)po_pOutBuffer, OutDataBufferPos, po_OutBufferSize);

                LastBlockSize = LastBlockSize - MAXIMUM_BLOCK_SIZE;

                HASSERT(LastBlockSize == 1);

                //Write the remaning bits in one block of 1 byte.
                m_pCodeBufferCompress[0] = m_pCodeBufferCompress[MAXIMUM_BLOCK_SIZE];

                Status = WriteBlock(LastBlockSize, (Byte*)po_pOutBuffer, OutDataBufferPos, po_OutBufferSize);
                }
            }

        // End-of-data code.
        if (Status)
            Status = WriteBlock(0, (Byte*)po_pOutBuffer, OutDataBufferPos, po_OutBufferSize);


        delete[] m_pCodeTableCompress;
        delete[] m_pCodeBufferCompress;

        Reset();
        }

    if (!Status)
        OutDataBufferPos = 0;

    return OutDataBufferPos;
    }

//-----------------------------------------------------------------------------
// public
// DecompressSubset
//-----------------------------------------------------------------------------
size_t HCDCodecHMRGIF::DecompressSubset(const  void* pi_pInData,
                                        size_t pi_InDataSize,
                                        void*  po_pOutBuffer,
                                        size_t pi_OutBufferSize)
    {
    bool Status = true;

    size_t OutDataBufferPos = 0;
    size_t InDataBufferPos  = 0;

    // is it the first packet?
    if(GetSubsetPosY() == 0)
        {
        SetCurrentState(STATE_DECOMPRESS);

        InDataBufferPos  = 0;

        // Should be done only 1 time.
        m_pCodeBuffer     = new Byte[64];
        m_CodeBufferSize  = 64;

        m_pCodeTable = new struct CodeEntry[LARGEST_CODE + 1];

        if (m_pCodeTable == NULL)
            Status = false;

        m_DecompresionStruct.MinCodeSize = m_DecompressMinCodeSize;

        if (m_DecompresionStruct.MinCodeSize < 2 || m_DecompresionStruct.MinCodeSize > 9)
            {
            if (m_DecompresionStruct.MinCodeSize == 1)
                m_DecompresionStruct.MinCodeSize = 2;
            else
                Status = false;
            }

        if (Status)
            InitTable(m_DecompresionStruct.MinCodeSize);

        m_DecompresionStruct.StackPtr      = 0;

        // Force "read_code" to start a new.
        m_BitOffset   = 64*8;
        m_BytesUnread = 0;
        }

    // Some byte have already been read in the last call, so we need to put them in the good buffer.
    if (m_BufferDataForNextBlockPos != 0)
        {
        // We need to copy that is in the m_pBufferDataForNextBlock in po_pOutBuffer.
        if(m_BufferDataForNextBlockPos >= GetSubsetWidth())
            {
            // The buffer contain more then one line.
            memcpy(po_pOutBuffer, m_pBufferDataForNextBlock, GetSubsetWidth());

            OutDataBufferPos = GetSubsetWidth();

            memcpy(m_pBufferDataForNextBlock,
                   m_pBufferDataForNextBlock + GetSubsetWidth(),
                   m_BufferDataForNextBlockPos - GetSubsetWidth());

            m_BufferDataForNextBlockPos =  m_BufferDataForNextBlockPos - GetSubsetWidth();
            }
        else
            {
            // The buffer contain last then one line.
            memcpy(po_pOutBuffer, m_pBufferDataForNextBlock, m_BufferDataForNextBlockPos);

            OutDataBufferPos = m_BufferDataForNextBlockPos;

            m_BufferDataForNextBlockPos  = 0;
            m_BufferDataForNextBlockSize = 0;
            free(m_pBufferDataForNextBlock);
            m_pBufferDataForNextBlock=0;
            }
        }

    while(Status && OutDataBufferPos < GetSubsetWidth())
        {
        if ((m_DecompresionStruct.Code = ReadCode((Byte*)pi_pInData, InDataBufferPos, pi_InDataSize)) != m_EofCode)
            {
            if (m_DecompresionStruct.Code == m_ClearCode)
                {
                InitTable(m_DecompresionStruct.MinCodeSize);

                m_DecompresionStruct.Code       = ReadCode((Byte*)pi_pInData, InDataBufferPos, pi_InDataSize);
                m_DecompresionStruct.OldCode    = m_DecompresionStruct.Code;
                m_DecompresionStruct.SuffixChar = m_DecompresionStruct.Code;
                m_DecompresionStruct.FinalChar  = m_DecompresionStruct.Code;

                if (!SetByte(m_DecompresionStruct.SuffixChar, (Byte*)po_pOutBuffer, OutDataBufferPos, pi_OutBufferSize))
                    Status = false;
                }
            else
                {
                m_DecompresionStruct.InputCode = m_DecompresionStruct.Code;

                if (m_DecompresionStruct.Code >= m_FreeCode)
                    {
                    m_DecompresionStruct.Code = m_DecompresionStruct.OldCode;
                    m_pCodeTable[m_DecompresionStruct.StackPtr++].stack = (Byte)m_DecompresionStruct.FinalChar;
                    }

                while (m_DecompresionStruct.Code >= m_FirstFree)
                    {
                    if (m_DecompresionStruct.StackPtr >= m_FreeCode)
                        {
                        //TR 280786 - This should not happen. Maybe DEFAULT_SIZE in
                        //HRFGifLineEditor.cpp needs to be increased.
                        HASSERT(0);
                        break;
                        }

                    m_pCodeTable[m_DecompresionStruct.StackPtr++].stack = m_pCodeTable[m_DecompresionStruct.Code].suffix;
                    m_DecompresionStruct.Code                           = m_pCodeTable[m_DecompresionStruct.Code].prefix;
                    }

                m_DecompresionStruct.FinalChar                      = m_DecompresionStruct.Code;
                m_DecompresionStruct.SuffixChar                     = m_DecompresionStruct.Code;
                m_pCodeTable[m_DecompresionStruct.StackPtr++].stack = (Byte)m_DecompresionStruct.FinalChar;

                while (m_DecompresionStruct.StackPtr > 0)
                    {
                    if (!SetByte(m_pCodeTable[--m_DecompresionStruct.StackPtr].stack, (Byte*)po_pOutBuffer, OutDataBufferPos, pi_OutBufferSize))
                        Status = false;
                    }

                m_pCodeTable[m_FreeCode].suffix = (Byte)m_DecompresionStruct.SuffixChar;
                m_pCodeTable[m_FreeCode].prefix = m_DecompresionStruct.OldCode;

                m_FreeCode++;
                m_DecompresionStruct.OldCode = m_DecompresionStruct.InputCode;

                if (m_FreeCode >= m_MaxCode)
                    {
                    if (m_CodeSize < 12)
                        {
                        m_CodeSize++;
                        m_MaxCode <<= 1;
                        }
                    }
                }
            }
        else
            Status = false;
        }

    SetCompressedImageIndex(GetCompressedImageIndex() + InDataBufferPos);

    SetSubsetPosY(GetSubsetPosY() + GetSubsetHeight());

    if(GetSubsetPosY() == GetHeight())
        {
        free((char*) m_pCodeTable);
        delete[] m_pCodeBuffer;

        Reset();
        }
    // A error occur during decompression
    if (!Status)
        OutDataBufferPos = 0;

    return OutDataBufferPos;
    }


//-----------------------------------------------------------------------------
// public
// HasLineAccess
//-----------------------------------------------------------------------------
bool HCDCodecHMRGIF::HasLineAccess() const
    {
    return false;
    }


//-----------------------------------------------------------------------------
// public
// HasLineHeader
//-----------------------------------------------------------------------------
bool HCDCodecHMRGIF::HasLineHeader() const
    {
    return false;
    }

//-----------------------------------------------------------------------------
// public
// SetDimensions
//-----------------------------------------------------------------------------
void HCDCodecHMRGIF::SetDimensions(size_t pi_Width, size_t pi_Height)
    {
    HCDCodecGIF::SetDimensions(pi_Width, pi_Height);
    }

//-----------------------------------------------------------------------------
// public
// HasLineIndexesTable
//-----------------------------------------------------------------------------
bool HCDCodecHMRGIF::HasLineIndexesTable() const
    {
    return false;
    }


//-----------------------------------------------------------------------------
// Protected
// InitTable
// Initialize the table.
// (Used in compression and decompression mode)
//-----------------------------------------------------------------------------
void HCDCodecHMRGIF::InitTable(short pi_MinCodeSize)
    {
    m_CodeSize  = pi_MinCodeSize + 1;
    m_ClearCode = 1 << pi_MinCodeSize;
    m_EofCode   = m_ClearCode + 1;
    m_FirstFree = m_ClearCode + 2;
    m_FreeCode  = m_FirstFree;
    m_MaxCode   = 1 << m_CodeSize;

    if (GetCurrentState() == STATE_COMPRESS)
        {
        for (unsigned short i = 0; i < TABLE_SIZE; i++)
            m_pCodeTableCompress[i].code_id = 0;
        }
    }


//-----------------------------------------------------------------------------
// public
// GetSubsetMaxCompressedSize
//-----------------------------------------------------------------------------
size_t HCDCodecHMRGIF::GetSubsetMaxCompressedSize() const
    {
    return (GetSubsetWidth() * GetSubsetHeight() * 2 + 256);
    }


//-----------------------------------------------------------------------------
// Protected
// ReadCode
// Read a code.
// (Used in decompression mode)
//-----------------------------------------------------------------------------
short HCDCodecHMRGIF::ReadCode(Byte* po_pDataBuffer,
                                size_t& pi_rPosBuffer,
                                size_t& pi_rBufferSize)
    {
    HPRECONDITION(po_pDataBuffer != 0);

    short m_ByteOffset;
    short m_BitsLeft;

    short BytesToMove, i;
    short NewByte;
    int32_t temp;

    static short mask[12] =
        {   0x001, 0x003, 0x007, 0x00F,
        0x01F, 0x03F, 0x07F, 0x0FF,
        0x1FF, 0x3FF, 0x7FF, 0xFFF
        };

    m_ByteOffset = m_BitOffset >> 3;
    m_BitsLeft   = m_BitOffset & 7;

    if (m_ByteOffset >= 61)
        {
        BytesToMove = 64 - m_ByteOffset;

        for (i = 0; i < BytesToMove; i++)
            m_pCodeBuffer[i] = m_pCodeBuffer[m_ByteOffset + i];

        while (i < 64 )
            {
            if (m_BytesUnread == 0)
                {
                // Get the length of the next record. A zero-length record denotes "end of data".
                m_BytesUnread = GetByte((Byte*)po_pDataBuffer, pi_rPosBuffer, pi_rBufferSize);

                if (m_BytesUnread < 1)
                    if (m_BytesUnread == 0)    /* end of data */
                        break;
                    else
                        return m_BytesUnread;
                }

            NewByte = GetByte((Byte*)po_pDataBuffer, pi_rPosBuffer, pi_rBufferSize);

            if (NewByte < 0)
                return NewByte;

            m_pCodeBuffer[i++] = CONVERT_TO_BYTE(NewByte);
            m_BytesUnread--;
            }

        m_BitOffset = m_BitsLeft;
        m_ByteOffset = 0;
        }

    m_BitOffset += m_CodeSize;
    temp = (long) m_pCodeBuffer[m_ByteOffset] |
           (long) m_pCodeBuffer[m_ByteOffset + 1] << 8  |
           (long) m_pCodeBuffer[m_ByteOffset + 2] << 16;

    if (m_BitsLeft > 0)
        temp >>= (long) m_BitsLeft;

    return (int) (temp) & mask[m_CodeSize - 1];
    }


//-----------------------------------------------------------------------------
// Protected
// GetByte
// Get a byte for the input buffer.
// (Used in compression and decompression mode)
//-----------------------------------------------------------------------------
short HCDCodecHMRGIF::GetByte(Byte* po_pDataBuffer,
                               size_t& pi_rPosBuffer,
                               size_t& pi_rBufferSize)
    {
    short Value = 0;

    if (pi_rPosBuffer < pi_rBufferSize)
        {
        Value = po_pDataBuffer[pi_rPosBuffer];
        pi_rPosBuffer++;
        }
    else
        Value = m_EofCode;

    return Value;
    }


//-----------------------------------------------------------------------------
// Protected
// SetByte
// Set a byte in the output buffer.
// (Used in compression and decompression mode)
//-----------------------------------------------------------------------------
bool HCDCodecHMRGIF::SetByte(uint32_t pi_Code,
                              Byte* po_pDataBuffer,
                              size_t& pi_rPosBuffer,
                              size_t& pi_rBufferSize)
    {
    bool Status = true;

    // We are try to write data that goes with the block.
    if (pi_rPosBuffer >= GetSubsetWidth() && GetCurrentState() == STATE_DECOMPRESS)
        {
        if (m_BufferDataForNextBlockPos >= m_BufferDataForNextBlockSize)
            {
            // The buffer is not intitialize, so we initialise it.
            if (m_BufferDataForNextBlockPos == 0)
                {
                m_pBufferDataForNextBlock     = (Byte*)malloc(128*sizeof(Byte));
                m_BufferDataForNextBlockSize  = 128;
                m_BufferDataForNextBlockPos   = 0;
                }
            // The buffer is full, so we realloc it.
            else
                {
                m_pBufferDataForNextBlock = (Byte*) realloc(m_pBufferDataForNextBlock, (m_BufferDataForNextBlockSize + 128)*sizeof(Byte));
                m_BufferDataForNextBlockSize += 128;
                }
            }
        m_pBufferDataForNextBlock[m_BufferDataForNextBlockPos] = (Byte)pi_Code;
        m_BufferDataForNextBlockPos++;
        }
    else
        {
        if (pi_rPosBuffer >= pi_rBufferSize)
            {
            // The buffer is not intitialize, so we initialise it.
            if (pi_rBufferSize == 0)
                po_pDataBuffer = new Byte[1024];
            // The buffer is full, so we realloc it.
            else
                po_pDataBuffer = (Byte*)renew(po_pDataBuffer, pi_rBufferSize, pi_rBufferSize + 256);

            pi_rBufferSize += 256;

            // A error occur in malloc or realloc.
            if (po_pDataBuffer == 0)
                Status = false;
            }

        if (Status)
            {
            // Write the code in the output buffer
            po_pDataBuffer[pi_rPosBuffer] = (Byte)pi_Code;
            pi_rPosBuffer++;
            }
        }

    return Status;
    }

//-----------------------------------------------------------------------------
// Public
// SetDecompressMinCodeSize
// Set LZW min code size that should be used for decompression
//-----------------------------------------------------------------------------
void HCDCodecHMRGIF::SetDecompressMinCodeSize(short pi_MinCodeSize)
    {
    m_DecompressMinCodeSize = pi_MinCodeSize;
    }

//-----------------------------------------------------------------------------
// Protected
// WriteBlock
// Write a block of maximum 255 bytes in the output buffer.
// (Used in compression mode)
//-----------------------------------------------------------------------------
bool HCDCodecHMRGIF::WriteBlock(short pi_NbByte,
                                 Byte* po_pDataBuffer,
                                 size_t& pi_rPosBuffer,
                                 size_t& pi_rBufferSize)
    {
    bool Status = true;

    HPRECONDITION(pi_NbByte < 256);

    Status = SetByte(pi_NbByte, po_pDataBuffer, pi_rPosBuffer, pi_rBufferSize);
    if (Status)
        {
        for (short i = 0; i < pi_NbByte; i++)
            Status = SetByte(m_pCodeBufferCompress[i], po_pDataBuffer, pi_rPosBuffer, pi_rBufferSize);
        }
    return Status;
    }


//-----------------------------------------------------------------------------
// Protected
// WriteCode
// Write a code generate by the Compress method.
// (Used in compression mode)
//-----------------------------------------------------------------------------
bool HCDCodecHMRGIF::WriteCode(short pi_Code,
                                Byte* po_pDataBuffer,
                                size_t& pi_rPosBuffer,
                                size_t& pi_rBufferSize)
    {
    bool Status = true;

    int32_t temp;
    short ByteOffset;
    short BitsLeft;

    ByteOffset = m_BitOffset >> 3;
    BitsLeft   = m_BitOffset & 7;

    // Code Buffer Compress is full write block and emty the buffer.
    if (ByteOffset >= 254)
        {
        Status = WriteBlock(ByteOffset, po_pDataBuffer, pi_rPosBuffer, pi_rBufferSize);
        m_pCodeBufferCompress[0] = m_pCodeBufferCompress[ByteOffset];
        m_BitOffset = BitsLeft;
        ByteOffset = 0;
        }

    if (BitsLeft > 0)
        {
        temp = ((long)pi_Code << BitsLeft) | m_pCodeBufferCompress[ByteOffset];
        m_pCodeBufferCompress[ByteOffset] = CONVERT_TO_BYTE(temp);
        m_pCodeBufferCompress[ByteOffset + 1] = CONVERT_TO_BYTE(temp >> 8);
        m_pCodeBufferCompress[ByteOffset + 2] = CONVERT_TO_BYTE(temp >> 16);
        }
    else
        {
        m_pCodeBufferCompress[ByteOffset] = CONVERT_TO_BYTE(pi_Code);
        m_pCodeBufferCompress[ByteOffset + 1] = pi_Code >> 8;
        }

    m_BitOffset += m_CodeSize;

    return Status;
    }

//-----------------------------------------------------------------------------
// private
// InitObject
//-----------------------------------------------------------------------------
void HCDCodecHMRGIF::InitObject()
    {
    m_DecompressMinCodeSize = m_BitsPerPixel;
    m_CodeBufferSize = 0;
    m_CodeTableSize = 0;
    m_CodeBufferSizeCompress = 0;
    m_CodeTableSizeCompress = 0;

    m_BufferDataForNextBlockPos = 0;
    m_BufferDataForNextBlockSize = 0;
    m_pBufferDataForNextBlock = 0;
    }
