//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HCDCodecHMRGIF.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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
    short prior_code;
    short code_id;
    unsigned char added_char;
    } CodeCompressEntry;


typedef struct DecompressionStruct
    {
    short OldCode;
    short InputCode;
    short Code;
    short SuffixChar;
    short FinalChar;
    short StackPtr;
    short MinCodeSize;
    } DecompressionStruct;


typedef struct CompressionStruct
    {
    short PrefixCode;
    short SuffixChar;
    short hx;
    short d;
    short MinCodeSize;
    } CompressionStruct;


typedef struct CodeEntry
    {
    short prefix;            /* prefix code */
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
                                   size_t pi_OutBufferSize);

    size_t          DecompressSubset(const void* pi_pInData,
                                     size_t pi_InDataSize,
                                     void*  po_pOutBuffer,
                                     size_t pi_OutBufferSize);

    IMAGEPP_EXPORT void            SetDecompressMinCodeSize(short pi_MinCodeSize);

    virtual void    SetDimensions(size_t pi_Width, size_t pi_Height);

    virtual bool   HasLineAccess() const;

    virtual HCDCodec*  Clone() const override;

    size_t          GetSubsetMaxCompressedSize() const;

    void            SetLineHeader(bool  pi_Enable);
    void            SetOneLineMode(bool pi_Enable);

    bool           IsOneLineMode() const;
    bool           HasLineHeader() const;

    bool           HasLineIndexesTable() const;

protected:

private:
    void        InitTable(short pi_MinCodeSize);

    short ReadCode (Byte* po_pDataBuffer,
                          size_t& pi_rPosBuffer,
                          size_t& pi_rBufferSize);

    short GetByte  (Byte* po_pDataBuffer,
                          size_t& pi_rPosBuffer,
                          size_t& pi_rBufferSize);

    bool       SetByte  (uint32_t pi_Code,
                          Byte* po_pDataBuffer,
                          size_t& pi_rPosBuffer,
                          size_t& pi_rBufferSize);

    bool       WriteBlock(short pi_NbByte,
                           Byte* po_pDataBuffer,
                           size_t& pi_rPosBuffer,
                           size_t& pi_rBufferSize);

    bool       WriteCode(short pi_Code,
                          Byte* po_pDataBuffer,
                          size_t& pi_rPosBuffer,
                          size_t& pi_rBufferSize);

    void        InitObject();

    Byte m_BitsPerPixel;

    short m_BytesUnread;
    short m_MaxCode;
    short m_CodeSize;
    short m_ClearCode;
    short m_EofCode;
    short m_FirstFree;
    short m_BitOffset;
    short m_FreeCode;
    short m_DecompressMinCodeSize;

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