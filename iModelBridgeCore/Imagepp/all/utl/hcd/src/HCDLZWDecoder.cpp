//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hcd/src/HCDLZWDecoder.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HCDLZWDecoder
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HCDLZWDecoder.h>

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HCDLZWDecoder::HCDLZWDecoder()
    {
    m_pPrefixCode  = new int32_t[LZW_TABLE_SIZE * sizeof(int32_t)];
    m_pSecondByte  = new Byte[LZW_TABLE_SIZE];
    m_pDecodeStack = new Byte[LZW_TABLE_SIZE];

    m_BitCount    = 0;
    m_BitBuffer   = 0;
    m_CurrentPos  = 0;
    m_MaxCode     = LZW_MAXCODE(LZW_BITS_MIN);
    m_NumBits     = LZW_BITS_MIN;
    m_NextCode    = LZW_CODE_FIRST;
    m_PixelValue  = 0;
    m_pInputBuffer = 0;
    m_pInputBytesCount = 0;

    LZW_DEBUG_TRACE
    (
        HFCPtr<HFCURL> pUrl(HFCURL::Instanciate(LZW_TRACE_FILE));
        m_pfile = HFCBinStream::Instanciate(pUrl, HFC_WRITE_AND_CREATE);
    )
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HCDLZWDecoder::~HCDLZWDecoder()
    {
    delete []m_pPrefixCode;
    delete []m_pSecondByte;
    delete []m_pDecodeStack;

    LZW_DEBUG_TRACE(delete m_pfile;)
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

int32_t HCDLZWDecoder::GetCode()
    {
    if(m_CurrentPos < (uint32_t)m_pInputBytesCount)
        {
        int32_t Code;

        m_BitBuffer = (m_BitBuffer << 8) | m_pInputBuffer[m_CurrentPos++];

        m_BitCount += 8;

        if( m_BitCount < m_NumBits )
            {
            if(m_CurrentPos >= (uint32_t)m_pInputBytesCount)
                // No more data : Compressed data does not contain an End of Information code.
                return LZW_CODE_EOI;

            m_BitBuffer = (m_BitBuffer << 8) | m_pInputBuffer[m_CurrentPos++];
            m_BitCount += 8;
            }
        Code = ((m_BitBuffer >> (m_BitCount - m_NumBits)) & m_MaxCode);

        m_BitCount -= m_NumBits;

        return Code;
        }

    // No more data : Compressed data does not contain an End of Information code.
    return LZW_CODE_EOI;
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

Byte* HCDLZWDecoder::DecodeStack(Byte* pi_pStackBottom, int32_t pi_Code)
    {
    while( pi_Code > (LZW_CODE_CLEAR - 1) )
        {
        HASSERT(pi_Code < LZW_TABLE_SIZE);
        HASSERT(pi_pStackBottom <= &m_pDecodeStack[LZW_TABLE_SIZE-1]);
        *pi_pStackBottom++ = m_pSecondByte[pi_Code];

        HASSERT(m_pPrefixCode[pi_Code] != pi_Code);     // infinite loop
        pi_Code = m_pPrefixCode[pi_Code];
        }

    *pi_pStackBottom = (Byte)pi_Code;

    return pi_pStackBottom;
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

size_t HCDLZWDecoder::Decode(Byte const* pi_pInputBuffer, size_t pi_inputBytesCount, Byte* po_pOutputBuffer, size_t pi_outputBytesCount)
    {
    m_pInputBuffer = pi_pInputBuffer;
    m_pInputBytesCount = pi_inputBytesCount;

    size_t Pos(0);
    int32_t newCode(GetCode());
    int32_t oldCode(0);

    // First code is always a clear code
    if (newCode != LZW_CODE_CLEAR)
        return 0;

    LZW_DEBUG_TRACE(char msg[512];)

    while((Pos < pi_outputBytesCount) && newCode != LZW_CODE_EOI)
        {
        LZW_DEBUG_TRACE
        (
            sprintf(msg, "Code = %d\n", newCode);
            m_pfile->Write(msg, strlen(msg));
        )

        if( newCode == LZW_CODE_CLEAR) // reset all decompression parameters
            {
            LZW_DEBUG_TRACE
            (
                sprintf(msg, "***** Clear Code Case : \n      Pos = %d pi_outputBytesCount = %d m_pInputBuffer[m_CurrentPos] = %d m_CurrentPos = %d\n      m_BitBuffer = %d m_BitCount = %d m_NextCode = %d m_NumBits = %d m_MaxCode = %d\n", Pos, pi_outputBytesCount, (int)m_pInputBuffer[m_CurrentPos], m_CurrentPos, m_BitBuffer, m_BitCount, m_NextCode, m_NumBits, m_MaxCode);
                m_pfile->Write(msg, strlen(msg));
            )

            m_NextCode = LZW_CODE_FIRST;
            m_MaxCode  = LZW_MAXCODE(LZW_BITS_MIN);
            m_NumBits  = LZW_BITS_MIN;
            newCode    = GetCode();


            LZW_DEBUG_TRACE
            (
                sprintf(msg, "Code = %d\n", newCode);
                m_pfile->Write(msg, strlen(msg));
            )

            if(newCode == LZW_CODE_EOI)
                {
                break;
                }

            HASSERT((Pos + 1) <= pi_outputBytesCount);

            po_pOutputBuffer[Pos++] = (Byte)newCode;
            oldCode      = newCode;
            m_PixelValue = (Byte)newCode;
            }
        else
            {
            LZW_DEBUG_TRACE
            (
                sprintf(msg, "@@@@@ NOT Clear Code Case : \n     Pos = %d pi_outputBytesCount = %d m_pInputBuffer[m_CurrentPos] = %d m_CurrentPos = %d\n      m_BitBuffer = %d m_BitCount = %d m_NextCode = %d m_NumBits = %d m_MaxCode = %d\n", Pos, pi_outputBytesCount, (int)m_pInputBuffer[m_CurrentPos], m_CurrentPos, m_BitBuffer, m_BitCount, m_NextCode, m_NumBits, m_MaxCode);
                m_pfile->Write(msg, strlen(msg));
            )

            if( newCode >= m_NextCode )
                {
                *m_pDecodeStack = m_PixelValue;
                m_pStackTop     = DecodeStack(m_pDecodeStack + 1, oldCode);
                }
            else
                {
                m_pStackTop = DecodeStack(m_pDecodeStack, newCode);
                }

            m_PixelValue = *m_pStackTop;              // output decoded stack_top in reverse

            while( m_pStackTop >= m_pDecodeStack )
                {
                // Do not allow to set pixel out of bound
                if (Pos <  pi_outputBytesCount)
                    {
                    po_pOutputBuffer[Pos++] = *m_pStackTop--;
                    }
                else
                    {
                    Pos++;
                    *m_pStackTop--;
                    }
                }

            if( m_NextCode < m_MaxCode )  // Add to stack_top table if not full
                {
                LZW_DEBUG_TRACE
                (
                    sprintf(msg, "prefix[%d] = %d\n", m_NextCode, oldCode);
                    m_pfile->Write(msg, strlen(msg));
                )

                m_pPrefixCode[m_NextCode] = oldCode;
                m_pSecondByte[m_NextCode] = m_PixelValue;

                // Will cause an infinite loop
                // Not always true. HASSERT(m_pPrefixCode[m_NextCode] != m_NextCode && m_pPrefixCode[oldCode] != oldCode);

                m_NextCode++;

                if( m_NextCode >= m_MaxCode )
                    {
                    m_NumBits++;

                    if( m_NumBits > LZW_BITS_MAX )
                        {
                        HASSERT(false);             // should not happen
                        m_NumBits = LZW_BITS_MAX;
                        }

                    m_MaxCode = LZW_MAXCODE(m_NumBits);
                    }
                }
            oldCode = newCode;
            }
        newCode = GetCode();
        }

    return Pos; // Number of bytes decoded.
    }


//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

#ifdef LZW_A_SLOW_DECODER

Byte* pOutputBufferItr;
Byte* pInputBufferItr;
int32_t nbits;
int32_t nextbits;
int32_t nextdata;
int32_t nbitsmask;

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

hcode_t    GetNextCode()
    {
    hcode_t code;

    nextdata = (nextdata<<8) | *(pInputBufferItr)++;
    nextbits += 8;
    if (nextbits < nbits)
        {
        nextdata = (nextdata<<8) | *(pInputBufferItr)++;
        nextbits += 8;
        }
    code = (hcode_t)((nextdata >> (nextbits-nbits)) & nbitsmask);
    nextbits -= nbits;
    return code;
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

void writeData(Byte* buffer, size_t count)
    {
    HASSERT(count > 0);

    memcpy(pOutputBufferItr, buffer, count);

    pOutputBufferItr+=count;
    }


//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

void HCDLZWDecoder::InitDecodeTable()
    {
    for(int i(0); i < 256; ++i)
        {
        m_decodeCodeTable[i] = i;
        }
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

size_t HCDLZWDecoder::Decode2(Byte* pi_pInputBuffer, size_t pi_inputBytesCount, Byte* po_pOutputBuffer, size_t po_outputBytesCount)
    {
    hcode_t code;
    hcode_t oldCode;
    pOutputBufferItr = po_pOutputBuffer;
    pInputBufferItr = pi_pInputBuffer;
    nbits =LZW_BITS_MIN;
    nextbits =0;
    nextdata =0;
    nbitsmask = LZW_MAXCODE(LZW_BITS_MIN);

    int32_t  nextFreeEntry = LZW_CODE_FIRST;
    Byte* outLimit = po_pOutputBuffer + po_outputBytesCount-1;

    while((code = GetNextCode()) != LZW_CODE_EOI)
        {
        HASSERT((pInputBufferItr - pi_pInputBuffer) < pi_inputBytesCount);

        if(pOutputBufferItr >= outLimit)
            {
            // HASSERT(false);     // A LZW_CODE_EOI is probably missing
            break;
            }

        if(code == LZW_CODE_CLEAR)
            {
            InitDecodeTable();
            nextFreeEntry = LZW_CODE_FIRST;
            nbits = LZW_BITS_MIN;
            nbitsmask = LZW_MAXCODE(LZW_BITS_MIN);
            code = GetNextCode();
            if(code == LZW_CODE_EOI)
                {
                break;
                }

            HASSERT((pOutputBufferItr - po_pOutputBuffer) < po_outputBytesCount);
            writeData((Byte*)&m_decodeCodeTable[code][0], m_decodeCodeTable[code].size());

            oldCode = code;
            }
        else
            {
            HASSERT(nextFreeEntry < LZW_TABLE_SIZE);

            // Code in table ?
            if(code < nextFreeEntry)
                {
                HASSERT((pOutputBufferItr - po_pOutputBuffer) < po_outputBytesCount);
                writeData((Byte*)&m_decodeCodeTable[code][0], m_decodeCodeTable[code].size());

                HASSERT(nextFreeEntry < nbitsmask);
                m_decodeCodeTable[nextFreeEntry++] = m_decodeCodeTable[oldCode] + (char)m_decodeCodeTable[code][0];

                oldCode = code;
                }
            else
                {
                // outString = StringFromCode(oldCode) + FirstChar(StringFromCode(oldCode));
                string outString(m_decodeCodeTable[oldCode] + (char)m_decodeCodeTable[oldCode][0]);

                HASSERT((pOutputBufferItr - po_pOutputBuffer) < po_outputBytesCount);
                writeData((Byte*)&outString[0], outString.size());

                HASSERT(nextFreeEntry < nbitsmask);
                m_decodeCodeTable[nextFreeEntry++] = outString;

                oldCode = code;
                }

            if (nextFreeEntry >= nbitsmask)
                {
                if (++nbits > LZW_BITS_MAX)        // should not happen
                    {
                    HASSERT(false);
                    nbits = LZW_BITS_MAX;
                    }
                nbitsmask = LZW_MAXCODE(nbits);
                }
            }
        }

    return 0;
    }
#endif