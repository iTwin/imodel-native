//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hcd/src/HCDLZWEncoder.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HCDLZWEncoder
//
// Adaptation of libtiff-lzw-compression-kit(tif_lzw.c,v 1.3 2003/03/12)
// See : http://www.libtiff.org/
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HCDLZWEncoder.h>
#include <Imagepp/all/h/HFCMath.h>

HCDLZWEncoder::HCDLZWEncoder()
    {
    }

HCDLZWEncoder::~HCDLZWEncoder()
    {
    }

void HCDLZWEncoder::ClearHashTable()
    {
    memset(m_hashTable, -1, sizeof(m_hashTable));
    }

// HChk Uncomment to enable compression ratio checking.
//#define  LZW_CHECK_COMPRESSION_RATIO 1

#ifdef LZW_CHECK_COMPRESSION_RATIO
#define    PutNextCode(output, code)                   \
    {                                                       \
        nextdata = (nextdata << nbits) | code;              \
        nextbits += nbits;                                  \
        *output++ = CONVERT_TO_BYTE(nextdata >> (nextbits-8));      \
        nextbits -= 8;                                      \
        if (nextbits >= 8)                                  \
        {                                                   \
            *output++ = CONVERT_TO_BYTE(nextdata >> (nextbits-8));  \
            nextbits -= 8;                                  \
        }                                                   \
        outputcount+=nbits;                                 \
    }
#else
#define    PutNextCode(output, code)                   \
    {                                                       \
        nextdata = (nextdata << nbits) | code;              \
        nextbits += nbits;                                  \
        *output++ = CONVERT_TO_BYTE(nextdata >> (nextbits-8));      \
        nextbits -= 8;                                      \
        if (nextbits >= 8)                                  \
        {                                                   \
            *output++ = CONVERT_TO_BYTE(nextdata >> (nextbits-8));  \
            nextbits -= 8;                                  \
        }                                                   \
    }
#endif



size_t HCDLZWEncoder::Encode(Byte* pi_pInputBuffer, size_t pi_inputBytesCount, Byte* po_pOutputBuffer, size_t po_outputBytesCount)
    {
#ifdef LZW_CHECK_COMPRESSION_RATIO
    int32_t     inputcount(0);
    int32_t     outputcount(0);
    int32_t     checkpoint(LZW_CHECK_GAP);
    int32_t     enc_ratio(0);
#endif
    int32_t     nextbits(0);
    int32_t     nextdata(0);
    int32_t      freeEntry(LZW_CODE_FIRST);
    int32_t      maxcode(LZW_MAXCODE(LZW_BITS_MIN));
    int32_t      nbits(LZW_BITS_MIN);
    Byte*      pOutputBufferItr = po_pOutputBuffer;
    Byte*      pInputBufferItr = pi_pInputBuffer;
    // The 4 here insures there is space for 2 max-sized codes.
    HDEBUGCODE(Byte*      pOutputLimit = po_pOutputBuffer + po_outputBytesCount-1 - 4);
    Byte*      pInputLimit = pi_pInputBuffer + pi_inputBytesCount;
    int32_t     disp;
    hcode_t     entry;
    register int32_t code;
    register int32_t fcode;
    register int32_t hash;
    register hash_t* pHashEntry;


    if(pi_inputBytesCount > 0)
        {
        ClearHashTable();

        PutNextCode(pOutputBufferItr, LZW_CODE_CLEAR);

        entry = *pInputBufferItr++;
        --pi_inputBytesCount;
#ifdef LZW_CHECK_COMPRESSION_RATIO
        ++inputcount;
#endif

        while(pInputBufferItr < pInputLimit)
            {
            code = *pInputBufferItr++;
#ifdef LZW_CHECK_COMPRESSION_RATIO
            ++inputcount;
#endif

            fcode = ((int32_t)code << LZW_BITS_MAX) + entry;
            hash = (code << LZW_HASH_SHIFT) ^ entry;               // xor hashing

            if(hash >= LZW_HASH_SIZE)
                {
                hash -= LZW_HASH_SIZE;
                }

            pHashEntry = &m_hashTable[hash];
            if(pHashEntry->hash == fcode)
                {
                entry = pHashEntry->code;
                continue;
                }

            if(pHashEntry->hash >= 0)
                {
                // Primary hash failed, check secondary hash
                disp = LZW_HASH_SIZE - hash;
                if(hash == 0)
                    {
                    disp = 1;
                    }

                do
                    {
                    // Avoid pointer arithmetric cuz of wraparound problems with segments
                    if((hash -= disp) < 0)
                        {
                        hash += LZW_HASH_SIZE;
                        }

                    pHashEntry = &m_hashTable[hash];
                    if(pHashEntry->hash == fcode)
                        {
                        entry = pHashEntry->code;
                        goto HIT;
                        }
                    }
                while(pHashEntry->hash >= 0);
                }

            // Verify there is space in the buffer for the code and
            // any potential Clear code that might be emitted below.
            // The value of limit is setup so that there are at least
            // 4 bytes free--room for 2 codes
            HDEBUGCODE(HASSERT(pOutputBufferItr <= pOutputLimit));

            // New entry, emit code and add to table
            PutNextCode(pOutputBufferItr, entry);

            entry = (hcode_t)code;
            pHashEntry->code = (hcode_t)(freeEntry); 
            ++freeEntry;
            pHashEntry->hash = fcode;

            if(freeEntry == LZW_CODE_MAX-1)
                {
                // Table is full, emit clear code and reset
                ClearHashTable();
#ifdef LZW_CHECK_COMPRESSION_RATIO
                enc_ratio = 0;
                inputcount = 0;
                outputcount = 0;
#endif
                freeEntry = LZW_CODE_FIRST;
                PutNextCode(pOutputBufferItr, LZW_CODE_CLEAR);
                nbits = LZW_BITS_MIN;
                maxcode = LZW_MAXCODE(LZW_BITS_MIN);
                }
            else
                {
                // if the next entry is going to be too big for the code size,
                // then increase it, if possible.
                if(freeEntry > maxcode)
                    {
                    ++nbits;
                    HASSERT(nbits <= LZW_BITS_MAX);
                    maxcode = (int32_t) LZW_MAXCODE(nbits);
                    }
#ifdef LZW_CHECK_COMPRESSION_RATIO
                else if(inputcount >= checkpoint)
                    {
                    int32_t ratio;

                    // check compression ratio and, if things seem to be slipping,
                    // clear the hash table and reset state. The compression ratio
                    // is a 24+8-bit fractional number
                    checkpoint = inputcount + LZW_CHECK_GAP;

                    if (inputcount > 0x007fffff)
                        {
                        // NB: shift will overflow
                        ratio = outputcount >> 8;
                        ratio = (ratio == 0 ? 0x7fffffff : inputcount/ratio);
                        }
                    else
                        {
                        ratio = (inputcount<<8) / outputcount;
                        }

                    if(ratio <= enc_ratio)
                        {
                        ClearHashTable();
                        enc_ratio = 0;
                        inputcount = 0;
                        outputcount = 0;
                        freeEntry = LZW_CODE_FIRST;
                        PutNextCode(pOutputBufferItr, LZW_CODE_CLEAR);
                        nbits = LZW_BITS_MIN;
                        maxcode = LZW_MAXCODE(LZW_BITS_MIN);
                        }
                    else
                        {
                        enc_ratio = ratio;
                        }
                    }
#endif
                }
HIT:
            ;
            }

        // Finish off an encoded strip by flusing the last string and tacking
        // on an End Of Information code.
        if(entry != (hcode_t)-1)
            {
            PutNextCode(pOutputBufferItr, entry);
            }

        PutNextCode(pOutputBufferItr, LZW_CODE_EOI);
        if(nextbits > 0)
            {
            *pOutputBufferItr++ = CONVERT_TO_BYTE(nextdata << (8-nextbits));
            }
        }

    return pOutputBufferItr - po_pOutputBuffer;
    }
