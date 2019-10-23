//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HCDCodecHMRGIF
//-----------------------------------------------------------------------------
// HMRGIF codec lass.
//-----------------------------------------------------------------------------
#pragma once

#include "HCDCodecGIF.h"

BEGIN_IMAGEPP_NAMESPACE

#define LARGEST_CODE       4095
#define TABLE_SIZE         5003
#define MAXIMUM_BLOCK_SIZE  255 //(In bytes) The maximum block size is limited
//by the fact that only 1 byte must be used to
//specify the size of a block.

typedef struct CodeCompressEntry
    {
    int16_t prior_code;
    int16_t code_id;
    unsigned char added_char;
    } CodeCompressEntry;


typedef struct DecompressionStruct
    {
    int16_t OldCode;
    int16_t InputCode;
    int16_t Code;
    int16_t SuffixChar;
    int16_t FinalChar;
    int16_t StackPtr;
    int16_t MinCodeSize;
    } DecompressionStruct;


typedef struct CompressionStruct
    {
    int16_t PrefixCode;
    int16_t SuffixChar;
    int16_t hx;
    int16_t d;
    int16_t MinCodeSize;
    } CompressionStruct;


typedef struct CodeEntry
    {
    int16_t prefix;            /* prefix code */
    Byte  suffix;            /* suffix character */
    Byte  stack;

    } CodeEntry;


class HCDCodecHMRGIF : public HCDCodecGIF
    {
    HDECLARE_CLASS_ID(HCDCodecId_GIFHMR, HCDCodecGIF)

public:
    IMAGEPP_EXPORT                HCDCodecHMRGIF();

    IMAGEPP_EXPORT                 HCDCodecHMRGIF(uint32_t pi_Width,
                                          uint32_t pi_Height,
                                          uint32_t pi_BitsPerPixel);

    IMAGEPP_EXPORT                 HCDCodecHMRGIF(const HCDCodecHMRGIF& pi_rObj);

    IMAGEPP_EXPORT                 ~HCDCodecHMRGIF();

    size_t          CompressSubset(const  void* pi_pInData,
                                   size_t pi_InDataSize,
                                   void*  po_pOutBuffer,
                                   size_t pi_OutBufferSize) override;

    size_t          DecompressSubset(const void* pi_pInData,
                                     size_t pi_InDataSize,
                                     void*  po_pOutBuffer,
                                     size_t pi_OutBufferSize) override;

    IMAGEPP_EXPORT void            SetDecompressMinCodeSize(int16_t pi_MinCodeSize);

    void    SetDimensions(size_t pi_Width, size_t pi_Height) override;

    bool   HasLineAccess() const override;

    virtual HCDCodec*  Clone() const override;

    size_t          GetSubsetMaxCompressedSize() const override;

    void            SetLineHeader(bool  pi_Enable);
    void            SetOneLineMode(bool pi_Enable);

    bool           IsOneLineMode() const;
    bool           HasLineHeader() const;

    bool           HasLineIndexesTable() const;

protected:

private:
    void        InitTable(int16_t pi_MinCodeSize);

    int16_t ReadCode (Byte* po_pDataBuffer,
                          size_t& pi_rPosBuffer,
                          size_t& pi_rBufferSize);

    int16_t GetByte  (Byte* po_pDataBuffer,
                          size_t& pi_rPosBuffer,
                          size_t& pi_rBufferSize);

    bool       SetByte  (uint32_t pi_Code,
                          Byte* po_pDataBuffer,
                          size_t& pi_rPosBuffer,
                          size_t& pi_rBufferSize);

    bool       WriteBlock(int16_t pi_NbByte,
                           Byte* po_pDataBuffer,
                           size_t& pi_rPosBuffer,
                           size_t& pi_rBufferSize);

    bool       WriteCode(int16_t pi_Code,
                          Byte* po_pDataBuffer,
                          size_t& pi_rPosBuffer,
                          size_t& pi_rBufferSize);

    void        InitObject();

    Byte m_BitsPerPixel;

    bool  m_tableIsValid;
    int16_t m_BytesUnread;
    int16_t m_MaxCode;
    int16_t m_CodeSize;
    int16_t m_ClearCode;
    int16_t m_EofCode;
    int16_t m_FirstFree;
    int16_t m_BitOffset;
    int16_t m_FreeCode;
    int16_t m_DecompressMinCodeSize;

    // Decompression
    Byte* m_pCodeBuffer;
    size_t  m_CodeBufferSize;

    CodeEntry*  m_pCodeTable;
    size_t      m_CodeTableSize;

    // Compression
    Byte* m_pCodeBufferCompress;
    size_t  m_CodeBufferSizeCompress;

    CodeCompressEntry* m_pCodeTableCompress;
    size_t             m_CodeTableSizeCompress;

    DecompressionStruct  m_DecompresionStruct;
    CompressionStruct    m_CompresionStruct;

    Byte* m_pBufferDataForNextBlock;
    size_t  m_BufferDataForNextBlockSize;
    size_t  m_BufferDataForNextBlockPos;

    HCDCodecHMRGIF&      operator=(const HCDCodecHMRGIF& pi_rObj);
    };

END_IMAGEPP_NAMESPACE
