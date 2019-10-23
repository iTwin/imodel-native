//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HCDLZWEncoder
//-----------------------------------------------------------------------------
// LZW codec lass.
//-----------------------------------------------------------------------------
#pragma once

/*
 * The TIFF spec specifies that encoded bit
 * strings range from 9 to 12 bits.
 */
constexpr int32_t    LZW_BITS_MIN = 9;        // start with 9 bits
constexpr int32_t    LZW_BITS_MAX = 12;       // max of 12 bit strings

constexpr int32_t LZW_MAXCODE(int32_t n) { return ((1L << (n)) - 1); }

// predefined codes
constexpr int32_t    LZW_CODE_CLEAR = 256;        // code to clear string table
constexpr int32_t    LZW_CODE_EOI = 257;        // end-of-information code
constexpr int32_t    LZW_CODE_FIRST = 258;        // first free code entry
constexpr int32_t    LZW_CODE_MAX = LZW_MAXCODE(LZW_BITS_MAX);
constexpr int32_t    LZW_HASH_SHIFT = 13 - 8;

constexpr int32_t    LZW_TABLE_SIZE = LZW_MAXCODE(LZW_BITS_MAX) + 1L;

constexpr int32_t    LZW_CHECK_GAP = 10000;        // enc_ratio check interval
constexpr int32_t    LZW_HASH_SIZE = 9001L;        // 91% occupancy

BEGIN_IMAGEPP_NAMESPACE

typedef uint16_t hcode_t;            // codes fit in 16 bits

class HCDLZWEncoder
    {
public:

    HCDLZWEncoder();
    ~HCDLZWEncoder();

    size_t Encode(Byte* pi_pInputBuffer, size_t pi_inputBytesCount, Byte* po_pOutputBuffer, size_t po_outputBytesCount);

protected:

private:

    void  ClearHashTable();

    typedef struct
        {
        int32_t  hash;
        hcode_t    code;
        } hash_t;

    hash_t m_hashTable[LZW_HASH_SIZE];
    };

END_IMAGEPP_NAMESPACE
