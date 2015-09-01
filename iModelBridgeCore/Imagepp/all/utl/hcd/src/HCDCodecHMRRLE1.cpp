//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hcd/src/HCDCodecHMRRLE1.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HCDCodecHMRRLE1
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HCDCodecHMRRLE1.h>
#include <Imagepp/all/h/HCDPacketRLE.h>

static const Byte s_BitMask[8]    = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };


//-----------------------------------------------------------------------------
// public
// Default constructor
//-----------------------------------------------------------------------------
HCDCodecHMRRLE1::HCDCodecHMRRLE1()
    : HCDCodecRLE1()
    {
    m_LineHeader = false;
    m_OneLineMode = false;
    m_LineIndexesTable = false;
    m_pLineIndexesTable = 0;
    m_LineIndexesTableSize = 0;
    }

//-----------------------------------------------------------------------------
// public
// Default constructor
//-----------------------------------------------------------------------------
HCDCodecHMRRLE1::HCDCodecHMRRLE1(size_t pi_Width,
                                 size_t pi_Height)
    : HCDCodecRLE1(pi_Width, pi_Height)
    {
    m_LineHeader = false;
    m_OneLineMode = false;
    m_LineIndexesTable = false;
    m_pLineIndexesTable = 0;
    m_LineIndexesTableSize = 0;
    }

//-----------------------------------------------------------------------------
// public
// Copy constructor
//-----------------------------------------------------------------------------
HCDCodecHMRRLE1::HCDCodecHMRRLE1(const HCDCodecHMRRLE1& pi_rObj)
    : HCDCodecRLE1(pi_rObj)
    {
    m_LineHeader = pi_rObj.m_LineHeader;
    m_OneLineMode = pi_rObj.m_OneLineMode;

    m_LineIndexesTable = pi_rObj.m_LineIndexesTable;

    if(!m_LineIndexesTable)
        {
        m_pLineIndexesTable = 0;
        m_LineIndexesTableSize = 0;
        }
    else
        {
        m_pLineIndexesTable = new uint32_t[GetHeight()];
        memcpy(m_pLineIndexesTable, pi_rObj.m_pLineIndexesTable, GetHeight() * sizeof(uint32_t));
        m_LineIndexesTableSize = GetHeight();
        m_LastCompressionIndex = pi_rObj.m_LastCompressionIndex;
        }
    }

//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HCDCodecHMRRLE1::~HCDCodecHMRRLE1()
    {
    if(m_pLineIndexesTable != 0)
        delete[] m_pLineIndexesTable;
    }

//-----------------------------------------------------------------------------
// public
// Clone
//-----------------------------------------------------------------------------
HCDCodec* HCDCodecHMRRLE1::Clone() const
    {
    return new HCDCodecHMRRLE1(*this);
    }

#define CHANGE_STATE(p) \
    while(p > 32767) \
    { \
        (*pRun) = 32767; \
        pRun++; \
        (*pRun) = 0; \
        pRun++; \
        p -= 32767; \
    } \
    (*pRun) = (unsigned short)p; \
    ++pRun; \
    OnState = !OnState;

//-----------------------------------------------------------------------------
// public
// CompressSubset
//-----------------------------------------------------------------------------
size_t HCDCodecHMRRLE1::CompressSubset(const void* pi_pInData,
                                       size_t pi_InDataSize,
                                       void* po_pOutBuffer,
                                       size_t po_OutBufferSize)
    {
    uint32_t State;
    size_t LinesCount;
    size_t LongsCount;
    size_t BytesCount;
    size_t BitsCount;
    unsigned short* pCurLine = NULL;
    size_t Pixels;
    size_t LongsPerLine;
    size_t ExtraBytesPerLine;
    size_t ExtraBitsPerLine;
    size_t Width;
    size_t OnState;
    size_t LineNumber;
    uint32_t Value;
    size_t PaddingBytes;
    Byte ByteValue;

    Byte* pSrcRawData = (Byte*)pi_pInData;
    unsigned short* pRun = (unsigned short*)po_pOutBuffer;

    // is it the first packet?
    if(GetSubsetPosY() == 0)
        {
        SetCurrentState(STATE_COMPRESS);
        }

    if(!m_OneLineMode)
        {
        LinesCount = GetSubsetHeight();
        Width = GetSubsetWidth();
        PaddingBytes = GetLinePaddingBits() >> 3;
        }
    else
        {
        LinesCount = 1;
        Width = (GetSubsetWidth() + GetLinePaddingBits()) * GetSubsetHeight();
        PaddingBytes = 0;
        }

    LongsPerLine = Width >> 5;
    ExtraBytesPerLine = (Width % 32) >> 3;
    ExtraBitsPerLine = Width % 8;

    LineNumber = GetSubsetPosY();

    if(LineNumber == 0)
        m_LastCompressionIndex = 0;

    while(LinesCount != 0)
        {
        if(m_LineIndexesTable)
            {   
            HASSERT_X64 ((((Byte*)pRun - (Byte*)po_pOutBuffer) >> 1)+m_LastCompressionIndex < UINT32_MAX);

            m_pLineIndexesTable[LineNumber] = (uint32_t)((((Byte*)pRun - (Byte*)po_pOutBuffer) >> 1)
                                                       + m_LastCompressionIndex);
            }

        // should we add a header at the beginning of each line?
        // if yes, keep a pointer to the beginning of the current line
        if(m_LineHeader)
            {
            pCurLine = pRun;

            // skeep the header
            pRun += 4;
            }

        // each line must begin with an off state
        Pixels = 0;
        OnState = false;

        LongsCount = LongsPerLine;

        while(LongsCount != 0)
            {
            Value = *((uint32_t*)pSrcRawData);

            if(Value == 0xFFFFFFFF)
                {
                if(OnState)
                    {
                    Pixels += 32;
                    }
                else
                    {
                    CHANGE_STATE(Pixels)
                    Pixels = 32;
                    }
                }
            else if(Value == 0x00000000)
                {
                if(!OnState)
                    {
                    Pixels += 32;
                    }
                else
                    {
                    CHANGE_STATE(Pixels);
                    Pixels = 32;
                    }
                }
            else
                {
                Value = (pSrcRawData[0] << 24) |
                        (pSrcRawData[1] << 16) |
                        (pSrcRawData[2] << 8) |
                        pSrcRawData[3];

                // test the 4 bytes
                BitsCount = 32;

                while(BitsCount != 0)
                    {
                    State = Value >> 31;

                    if(State != OnState)
                        {
                        CHANGE_STATE(Pixels)
                        Pixels = 1;
                        }
                    else
                        {
                        Pixels++;
                        }


                    Value <<= 1;

                    --BitsCount;
                    }
                }

            pSrcRawData += 4;
            --LongsCount;
            }


        BytesCount = ExtraBytesPerLine;

        while(BytesCount != 0)
            {
            ByteValue = *pSrcRawData;

            BitsCount = 8;

            while(BitsCount != 0)
                {
                State = ByteValue >> 7;

                if(State != OnState)
                    {
                    CHANGE_STATE(Pixels)
                    Pixels = 1;
                    }
                else
                    {
                    Pixels++;
                    }


                ByteValue <<= 1;

                --BitsCount;
                }

            --BytesCount;
            ++pSrcRawData;
            }

        if(ExtraBitsPerLine != 0)
            {
            ByteValue = *pSrcRawData;

            BitsCount = ExtraBitsPerLine;

            while(BitsCount != 0)
                {
                State = ByteValue >> 7;

                if(State != OnState)
                    {
                    CHANGE_STATE(Pixels)
                    Pixels = 1;
                    }
                else
                    {
                    Pixels++;
                    }

                ByteValue <<= 1;

                --BitsCount;
                }

            ++pSrcRawData;
            }

        CHANGE_STATE(Pixels)

        // each line must end with an OFF state
        if(!OnState)
            {
            (*pRun) = 0;
            pRun++;
            }

        pSrcRawData += PaddingBytes;

        // store the scanline header if necessary
        if(m_LineHeader)
            {
            // scanline header
            *pCurLine = 0x5900;
            pCurLine++;
            // words to follow
            *pCurLine = (unsigned short)(pRun - (pCurLine + 1));
            pCurLine++;
            // line number
            *pCurLine = (unsigned short)(LineNumber + 1);
            pCurLine++;
            // pixel offset
            *pCurLine = 0;
            }

        LineNumber++;
        LinesCount--;
        }

    SetSubsetPosY(GetSubsetPosY() + GetSubsetHeight());

    if(GetSubsetPosY() == GetHeight())
        Reset();

    size_t CompressedSize = ((Byte*)pRun) - ((Byte*)po_pOutBuffer);

    m_LastCompressionIndex += (CompressedSize >> 1);

    return(CompressedSize);
    }

//-----------------------------------------------------------------------------
// public
// DecompressSubset
//-----------------------------------------------------------------------------
size_t HCDCodecHMRRLE1::DecompressSubset(const void* pi_pInData,
                                         size_t pi_InDataSize,
                                         void* po_pOutBuffer,
                                         size_t pi_OutBufferSize)
    {
    // is it the first packet?
    if(GetSubsetPosY() == 0)
        {
        SetCurrentState(STATE_DECOMPRESS);
        }

    unsigned short* pRun = (unsigned short*)pi_pInData;

    // test if we simply want to skip the lines
    if(pi_OutBufferSize == 0)
        {
        // if yes, parse lines

        for(uint32_t Line = 0; Line < GetSubsetHeight(); Line++)
            {
            // skip the code
            pRun += 1;

            // add the words count
            pRun += *pRun;
            }
        }
    else
        {
        // get the width and height
        size_t Width;
        size_t Height;

        size_t Count;
        size_t PixelsCount;
        size_t PixelsInByte;
        size_t NumberOfInitialBits;
        size_t NumberOfBytes;
        size_t NumberOfEndBits;
        bool OnState;

        size_t ExtraBytes;

        Byte* pDestRawData = (Byte*)po_pOutBuffer;

        size_t LinesCount;

        if(!m_OneLineMode)
            {
            Width  = GetSubsetWidth();
            Height = GetSubsetHeight();
            LinesCount =  Height;
            ExtraBytes = GetLinePaddingBits() / 8;
            }
        else
            {
            LinesCount = 1;
            Width = GetSubsetWidth() * GetSubsetHeight();
            Height = 1;
            ExtraBytes = 0;
            }

        size_t WordsToFollow = 0;
        unsigned short* pStartLine = NULL;

        // parse each line
        while(LinesCount != 0)
            {
            // if there is a bytes count at the beginning of each line,
            // we skeep it
            if(m_LineHeader)
                {
                WordsToFollow = pRun[1];
                WordsToFollow -= 2;

                pRun+=4;
                pStartLine = pRun;
                }

            OnState = false;

            PixelsCount = Width;

            PixelsInByte = 0;

            // parse each pixel runs in row
            while(PixelsCount != 0)
                {
                // get the length of the run
                Count = *pRun;

                // !!! PATCH
                // !!! SOME RLE FILES HAVE LINES ENCODED WITH MORE PIXELS THAN THE PHYSICAL IMAGE
                // !!! WIDTH; WE MUST TEST THAT CASE AND IGNORE THE EXTRA PIXELS
                if(((int32_t)PixelsCount - (int32_t)Count) >= 0)
                    {
                    // NORMAL MODE

                    // total number of pixels of the line less the length of the run
                    PixelsCount -= Count;
                    }
                else
                    {
                    // BUGGED ENCODED MODE

                    Count = PixelsCount;
                    PixelsCount = 0;
                    }

                // is there a byte already partially set
                if(PixelsInByte > 0)
                    {
                    // is there other pixels after this byte
                    if(Count >= (8 - PixelsInByte))
                        {
                        //yes, fill the byte
                        NumberOfInitialBits = (8 - PixelsInByte);
                        PixelsInByte = 0;
                        Count -= NumberOfInitialBits;
                        }
                    else
                        {
                        // no, fill partially the byte
                        NumberOfInitialBits = Count;
                        PixelsInByte += NumberOfInitialBits;
                        Count = 0;
                        }


                    // fill the bits in the byte
                    while(NumberOfInitialBits)
                        {
                        *pDestRawData <<= 1;

                        if(OnState)
                            *pDestRawData |= 0x01;
                        else
                            *pDestRawData &= 0xfe;

                        NumberOfInitialBits--;
                        }

                    // should we increment the byte
                    if(PixelsInByte == 0)
                        {
                        pDestRawData++;
                        }
                    else
                        {
                        // is it the last pixel?
                        // if yes, shift the byte
                        if(PixelsCount == 0)
                            *pDestRawData <<= (8 - PixelsInByte);
                        }
                    }

                // determine how much bytes and extra bits to fill
                NumberOfBytes = Count / 8;
                NumberOfEndBits = Count % 8;

                // fill the bytes
                if(NumberOfBytes > 0)
                    {
                    if(OnState)
                        memset(pDestRawData, 0xff, NumberOfBytes);
                    else
                        memset(pDestRawData, 0x00, NumberOfBytes);

                    pDestRawData += NumberOfBytes;
                    }

                // fill the bits
                if(NumberOfEndBits > 0)
                    {
                    if(OnState)
                        *pDestRawData = 0xff;
                    else
                        *pDestRawData = 0x00;

                    PixelsInByte = NumberOfEndBits;
                    }

                // alternate the state for the next run
                OnState = !OnState;

                // go to next run
                pRun++;
                }

            // at the end of the line, test if a byte is partially set
            // and if yes, advance to the next byte
            if(PixelsInByte != 0)
                pDestRawData++;

            // add the padding
            pDestRawData += ExtraBytes;

            // if last state was on state, add the 0 off state
            if(!OnState)
                pRun++;

            // test the line header extra bytes
            if(m_LineHeader)
                {
                pRun += ((int32_t)WordsToFollow - (int32_t)(pRun - pStartLine));
                }

            // decrement the number of lines to parse
            LinesCount--;
            }

        }

    SetCompressedImageIndex(GetCompressedImageIndex() +
                            (((Byte*)pRun) - ((Byte*)pi_pInData)));

    SetSubsetPosY(GetSubsetPosY() + GetSubsetHeight());

    size_t OutDataSize = ((GetSubsetWidth() + GetLinePaddingBits()) / 8) * GetSubsetHeight();

    if(GetSubsetPosY() == GetHeight())
        Reset();

    return(OutDataSize);
    }


//-----------------------------------------------------------------------------
// public
// GetRLEInterface
//-----------------------------------------------------------------------------
HCDCodecRLEInterface* HCDCodecHMRRLE1::GetRLEInterface()
    {
    if(!IsOneLineMode())
        return this;

    return 0;
    }

//-----------------------------------------------------------------------------
// public
// CompressSubsetFromRLE
//-----------------------------------------------------------------------------
size_t HCDCodecHMRRLE1::CompressSubsetFromRLE(HFCPtr<HCDPacketRLE> const& pi_rpPacketRLE, void* po_pOutBuffer, size_t po_OutBufferSize)
    {
    HPRECONDITION(!IsOneLineMode());    // For now we do not support this option.
    HPRECONDITION(pi_rpPacketRLE->GetCodec()->GetHeight() >= GetSubsetHeight());
    HPRECONDITION(pi_rpPacketRLE->GetCodec()->GetWidth() == GetSubsetWidth());
    HPRECONDITION(GetWidth() == GetSubsetWidth());

    // is it the first packet?
    if(GetSubsetPosY() == 0)
        {
        SetCurrentState(STATE_COMPRESS);
        m_LastCompressionIndex = 0;
        }

    unsigned short* pOutRuns = (unsigned short*)po_pOutBuffer;

    // Compress subset
    for(uint32_t Line=0; Line < GetSubsetHeight(); ++Line)
        {
        if(m_LineIndexesTable)
            {  
            HASSERT_X64 ((((Byte*)pOutRuns - (Byte*)po_pOutBuffer) >> 1) + m_LastCompressionIndex < UINT32_MAX);

            m_pLineIndexesTable[Line] = (uint32_t)((((Byte*)pOutRuns - (Byte*)po_pOutBuffer) >> 1) + m_LastCompressionIndex);
            }

        unsigned short* pLineHead = pOutRuns;    // Keep track of the beginning of the line in case we need to add an header and to determine if we ended with blacks.

        // Skip header if we have one.
        if(m_LineHeader)
            {
            // Header is as follow: [Tag, wordsToFollow, lineNumber, pixelOffset]
            pOutRuns+=4;                        // Skip header: 4 WORD.
            }

        unsigned short const* pCurrentLine = (unsigned short const*)pi_rpPacketRLE->GetLineBuffer(Line);
        size_t         CurrRunIndex = 0;
        size_t         MaxCurrRunIndex = pi_rpPacketRLE->GetLineDataSize(Line) >> 1;

        while(CurrRunIndex < MaxCurrRunIndex)
            {
            uint32_t PixelsFromCurrRun = pCurrentLine[CurrRunIndex];
            ++CurrRunIndex;

            // Append all pixels of the same state. That take care of the 32767,0,123...
            while(CurrRunIndex+1 < MaxCurrRunIndex && pCurrentLine[CurrRunIndex] == 0)
                {
                PixelsFromCurrRun += pCurrentLine[CurrRunIndex+1];
                CurrRunIndex+=2;
                }

            // Encode current run into destination.
            while(PixelsFromCurrRun > 32767)
                {
                pOutRuns[0] = 32767;
                pOutRuns[1] = 0;
                PixelsFromCurrRun -= 32767;
                pOutRuns+=2;
                }
            pOutRuns[0] = (unsigned short)PixelsFromCurrRun;
            ++pOutRuns;
            }

        // Must end with blacks.
        if(!((pOutRuns - pLineHead) & 0x00000001)) // black runs are ON even number. 0,2,4,6...
            {
            pOutRuns[0] = 0;
            ++pOutRuns;
            }

        // Store the scanline header
        if(m_LineHeader)
            {
            // scanline header
            pLineHead[0] = 0x5900;
            // words to follow
            pLineHead[1] = (unsigned short)(pOutRuns - (pLineHead + 2));
            // line number
            pLineHead[2] = (unsigned short)(Line + 1);
            // pixel offset
            pLineHead[3] = 0;
            }
        }

    SetSubsetPosY(GetSubsetPosY() + GetSubsetHeight());

    if(GetSubsetPosY() == GetHeight())
        Reset();

    size_t CompressedSize = ((Byte*)pOutRuns) - ((Byte*)po_pOutBuffer);

    m_LastCompressionIndex += (CompressedSize >> 1);

    return(CompressedSize);
    }

//-----------------------------------------------------------------------------
// public
// DecompressSubsetToRLE
//-----------------------------------------------------------------------------
void HCDCodecHMRRLE1::DecompressSubsetToRLE(const void* pi_pInData, size_t pi_InDataSize, HFCPtr<HCDPacketRLE>& pio_rpRLEPacket)
    {
    HPRECONDITION(!IsOneLineMode());    // For now we do not support this option.
    HPRECONDITION(pio_rpRLEPacket->GetCodec()->GetHeight() >= GetSubsetHeight());
    HPRECONDITION(pio_rpRLEPacket->GetCodec()->GetWidth() == GetSubsetWidth());
    HPRECONDITION(GetWidth() == GetSubsetWidth());
    HPRECONDITION(pio_rpRLEPacket->HasBufferOwnership());    // Must be owner of buffers.

    // is it the first packet?
    if(GetSubsetPosY() == 0)
        {
        SetCurrentState(STATE_DECOMPRESS);
        }

    unsigned short const* pRunLine = (unsigned short const*)pi_pInData;

    // Decompress subset
    for(uint32_t Line(0); Line < GetSubsetHeight(); ++Line)
        {
        // Skip header if we have one.
        uint32_t WordsToNextLine = 0;   // Valid only in m_LineHeader mode
        if(m_LineHeader)
            {
            // Header is as follow: [Tag, wordsToFollow, lineNumber, pixelOffset]
            WordsToNextLine = pRunLine[1] - 2;  // WordsToFollow - lineNumber and pixelOffset entries;
            pRunLine+=4;                        // Skip header: 4 WORD.
            }

        // N.B. pRunLine is always aligned to the the first RLE run of the current line.
        uint32_t RunLineIndex = 0;
        uint32_t PixelsFromCurrentRun = 0;

        // Count the number of runs required to encode our width.
        while (PixelsFromCurrentRun < GetSubsetWidth())
            {
            PixelsFromCurrentRun += pRunLine[RunLineIndex];
            ++RunLineIndex;
            }

        // Have we read beyond the size stored in the line header?
        HASSERT(WordsToNextLine ? RunLineIndex <= WordsToNextLine : true);

        // We start and end with blacks so we always have an odd number of entries. Make sure it is true in case we need a trailing empty black.
        size_t LineDataSize = (RunLineIndex | 0x1) * sizeof(unsigned short);

        // Alloc buffer if it is not large enough.
        if(pio_rpRLEPacket->GetLineBufferSize(Line) < LineDataSize)
            {
            HASSERT(pio_rpRLEPacket->HasBufferOwnership());    // Must be owner of buffer.
            pio_rpRLEPacket->SetLineBuffer(Line, new Byte[LineDataSize], LineDataSize, 0/*pi_DataSize*/);
            }

        // Copy RLE runs from source.
        unsigned short* pOutLineBuffer = (unsigned short*)pio_rpRLEPacket->GetLineBuffer(Line);
        memcpy(pOutLineBuffer, pRunLine, RunLineIndex * sizeof (unsigned short));

        // !!!! PATCH buggy encoded lines
        if(PixelsFromCurrentRun > GetSubsetWidth())
            pOutLineBuffer[RunLineIndex-1] -= (unsigned short)(PixelsFromCurrentRun - GetSubsetWidth());

        // We must end with a black run. So if current entry would be black that means the last entry we wrote is white and thus we must add an empty black to close the RLE run.
        if(!(RunLineIndex & 0x1))     // black runs are ON even number: 0,2,4,6...
            {
            pOutLineBuffer[RunLineIndex] = 0;
            ++RunLineIndex;
            }

        // Packet is filled with data.
        HASSERT(LineDataSize == RunLineIndex * sizeof (unsigned short));
        pio_rpRLEPacket->SetLineDataSize(Line, LineDataSize);

        // adjust buffer to the next line
        if(WordsToNextLine)   // Non zero only in m_LineHeader mode.
            pRunLine+=WordsToNextLine;
        else
            pRunLine+=RunLineIndex;
        }

    SetSubsetPosY(GetSubsetPosY() + GetSubsetHeight());

    SetCompressedImageIndex(GetCompressedImageIndex() + (((Byte*)pRunLine) - ((Byte*)pi_pInData)));

    if(GetSubsetPosY() == GetHeight())
        Reset();
    }

//-----------------------------------------------------------------------------
// public
// IsOneLineMode
//-----------------------------------------------------------------------------
bool HCDCodecHMRRLE1::IsOneLineMode() const
    {
    return m_OneLineMode;
    }

//-----------------------------------------------------------------------------
// public
// SetLineHeader
//-----------------------------------------------------------------------------
void HCDCodecHMRRLE1::SetLineHeader(bool pi_Enable)
    {
    m_LineHeader = pi_Enable;
    }


//-----------------------------------------------------------------------------
// public
// SetOneLineMode
//-----------------------------------------------------------------------------
void HCDCodecHMRRLE1::SetOneLineMode(bool pi_Enable)
    {
    m_OneLineMode = pi_Enable;
    }

//-----------------------------------------------------------------------------
// public
// EnableLineIndexesTable
//-----------------------------------------------------------------------------
void HCDCodecHMRRLE1::EnableLineIndexesTable(bool pi_Enable)
    {
    if(m_pLineIndexesTable != 0)
        {
        delete m_pLineIndexesTable;
        m_pLineIndexesTable = 0;
        m_LineIndexesTableSize = 0;
        }

    if(pi_Enable)
        {
        m_LineIndexesTable = true;
        m_pLineIndexesTable = new uint32_t[GetHeight()];
        m_LineIndexesTableSize = GetHeight();
        }
    else
        {
        m_LineIndexesTable = false;
        }
    }

//-----------------------------------------------------------------------------
// public
// GetLineIndexesTable
//-----------------------------------------------------------------------------
uint32_t* HCDCodecHMRRLE1::GetLineIndexesTable() const
    {
    return(m_pLineIndexesTable);
    }

//-----------------------------------------------------------------------------
// public
// HasLineAccess
//-----------------------------------------------------------------------------
bool HCDCodecHMRRLE1::HasLineAccess() const
    {
    return !m_OneLineMode;
    }


//-----------------------------------------------------------------------------
// public
// HasLineHeader
//-----------------------------------------------------------------------------
bool HCDCodecHMRRLE1::HasLineHeader() const
    {
    return m_LineHeader;
    }

//-----------------------------------------------------------------------------
// public
// SetDimensions
//-----------------------------------------------------------------------------
void HCDCodecHMRRLE1::SetDimensions(size_t pi_Width, size_t pi_Height)
    {
    HCDCodecRLE1::SetDimensions(pi_Width, pi_Height);

    if(m_LineIndexesTable)
        {
        if(m_pLineIndexesTable != 0)
            {
            delete[] m_pLineIndexesTable;
            m_pLineIndexesTable = 0;
            m_LineIndexesTableSize = 0;
            }

        m_pLineIndexesTable = new uint32_t[GetHeight()];
        HASSERT(m_pLineIndexesTable != 0);
        m_LineIndexesTableSize = GetHeight();
        }
    }

//-----------------------------------------------------------------------------
// public
// HasLineIndexesTable
//-----------------------------------------------------------------------------
bool HCDCodecHMRRLE1::HasLineIndexesTable() const
    {
    return(m_LineIndexesTable);
    }


//-----------------------------------------------------------------------------
// public
// SetLineIndexesTable
//-----------------------------------------------------------------------------
void HCDCodecHMRRLE1::SetLineIndexesTable(const uint32_t* pi_pTable)
    {
    HPRECONDITION(m_pLineIndexesTable != 0);

    // copy the table
    memcpy(m_pLineIndexesTable, pi_pTable, m_LineIndexesTableSize * sizeof(uint32_t));
    }



//-----------------------------------------------------------------------------
// static
// GetSizeOf
//
// Return the number of byte of the compressed subset data
//-----------------------------------------------------------------------------
size_t HCDCodecHMRRLE1::GetSizeOf(const void*   pi_pCompressedData,
                                  uint32_t     pi_Width,
                                  uint32_t     pi_Height)
    {
    HPRECONDITION(pi_pCompressedData != 0);

    unsigned short* pRun((unsigned short*)pi_pCompressedData);
    size_t BufferSize = 0;
    uint32_t PixelCount = 0;
    while (pi_Height != 0)
        {
        while (PixelCount < pi_Width)
            {
            PixelCount += *pRun++;
            ++BufferSize;
            }
        HPOSTCONDITION(PixelCount == pi_Width);

        if (((pRun - (unsigned short*)pi_pCompressedData) & 0x01) == 0)
            {
            HPRECONDITION(*pRun == 0);
            ++BufferSize;
            ++pRun;
            }

        PixelCount = 0;
        --pi_Height;
        }

    return BufferSize * sizeof(unsigned short);
    }
