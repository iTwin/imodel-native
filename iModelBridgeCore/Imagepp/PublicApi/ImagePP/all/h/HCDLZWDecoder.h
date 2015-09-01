//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HCDLZWDecoder.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HCDLZWDecoder
//-----------------------------------------------------------------------------
// LZW codec lass.
//-----------------------------------------------------------------------------
#pragma once

#include "HCDLZWEncoder.h"

//#define LZW_A_SLOW_DECODER 1      //  HChck  debugging purpose

//#define LZW_TRACE 1   // Debugging purpose

#ifdef LZW_TRACE
#include "HFCBinStream.h"
#define LZW_TRACE_FILE "file://E:\\dev_I++\\lzwDecoderTrace.dat"
#define LZW_DEBUG_TRACE(x) HDEBUGCODE(x)
#else
#define LZW_DEBUG_TRACE(x)
#endif

BEGIN_IMAGEPP_NAMESPACE

class HCDLZWDecoder
    {
public:

    HCDLZWDecoder();
    ~HCDLZWDecoder();

    size_t Decode(Byte const* pi_pInputBuffer, size_t pi_inputBytesCount, Byte* po_pOutputBuffer, size_t pi_outputBytesCount);

#ifdef LZW_A_SLOW_DECODER
    size_t  Decode2(Byte* pi_pInputBuffer, size_t pi_inputBytesCount, Byte* po_pOutputBuffer, size_t po_outputBytesCount);
#endif

protected:

private:

    int32_t GetCode();
    Byte*  DecodeStack(Byte* pi_pStackBottom, int32_t pi_Code);

#ifdef LZW_A_SLOW_DECODER
    void    InitDecodeTable();
#endif

    int32_t* m_pPrefixCode;
    Byte* m_pSecondByte;
    Byte* m_pDecodeStack;
    Byte  m_LZWCodeSize;
    int32_t m_NumBits;
    int32_t m_MaxCode;

    uint32_t m_BitBuffer;
    int32_t m_BitCount;
    Byte  m_PixelValue;
    int32_t m_NextCode;
    Byte* m_pStackTop;

    Byte const* m_pInputBuffer;      // compressed data
    size_t m_pInputBytesCount;
    uint32_t m_CurrentPos;
    unsigned short m_Predictor;

#ifdef LZW_A_SLOW_DECODER
    string  m_decodeCodeTable[LZW_TABLE_SIZE];
#endif

    LZW_DEBUG_TRACE(HFCBinStream* m_pfile;)
    };

END_IMAGEPP_NAMESPACE