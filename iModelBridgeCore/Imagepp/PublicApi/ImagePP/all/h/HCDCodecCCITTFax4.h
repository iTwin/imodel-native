//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HCDCodecCCITTFax4
//-----------------------------------------------------------------------------
#pragma once

#include "HCDCodecCCITT.h"

BEGIN_IMAGEPP_NAMESPACE

class HCDCodecHMRRLE1;
class HCDPacketRLE;

class HCDCodecCCITTFax4 : public HCDCodecCCITT, HCDCodecRLEInterface
    {
    HDECLARE_CLASS_ID(HCDCodecId_CCITTFax4, HCDCodecCCITT)

public:
    IMAGEPP_EXPORT         HCDCodecCCITTFax4();

    IMAGEPP_EXPORT         HCDCodecCCITTFax4(uint32_t pi_Width, uint32_t pi_Height);


    HCDCodecCCITTFax4(const HCDCodecCCITTFax4& pi_rObj);

    virtual         ~HCDCodecCCITTFax4();

    size_t          CompressSubset(const void* pi_pInData,size_t pi_InDataSize,
                                   void* po_pOutBuffer,   size_t pi_OutBufferSize) override;

    size_t          DecompressSubset(const void* pi_pInData, size_t pi_InDataSize,
                                     void* po_pOutBuffer,    size_t pi_OutBufferSize) override;

    // Binary optimization: Speed up read when codec support direct decompression to RLE format.
    HCDCodecRLEInterface*   GetRLEInterface() override;
    IMAGEPP_EXPORT virtual void             DecompressSubsetToRLE(const void* pi_pInData, size_t pi_InDataSize, HFCPtr<HCDPacketRLE>& pio_rpRLEPacket) override;
    IMAGEPP_EXPORT virtual size_t           CompressSubsetFromRLE(HFCPtr<HCDPacketRLE> const& pi_rpPacketRLE, void* po_pOutBuffer, size_t po_OutBufferSize) override;

    bool           HasLineAccess() const override;

    void            Reset() override;

    virtual HCDCodec* Clone() const override;
    
protected:

private:

    struct CCITT4State
        {
        CCITT4State();
        CCITT4State(const CCITT4State& pi_rObj);
        ~CCITT4State();

        void Pre(int32_t pi_Width, int32_t pi_Height, bool pi_Decode, bool pi_UseBitRevTable, bool pi_InvertResult);
        void Post();
        void Reset();
        void GrowMaxLRS();

        void EncodeLineFromBit();
        void EncodeLineFromRLE();

        void DecodeLine();
        void DecodeLineRLE();

        // Buffer use for RLE decode/encode routines.
        void SetupForRLEMode(Byte* pio_pG4Buffer, size_t pi_G4BufferSize, uint16_t* pio_pRLEBuffer, size_t pi_RLEBufferSize);

        // Buffer use for RLE decode/encode routines. Must have setup for RLEMode before calling this method.
        void SetRLEBuffer(uint16_t* pio_pRLEBuffer, size_t pi_RLEBufferSize);

        // Buffer use for uncompress decode/encode routines.
        void SetupForUncompressMode(Byte* pio_pG4Buffer, size_t pi_G4BufferSize, Byte* pio_pUncompressBuffer, size_t pi_UncompressBufferSize, size_t pi_UncompressLineSize);

        // Byte(s) read(decode) or write(encode) in m_g4Buffer.
        size_t GetG4BufferDataSize() const;

        // Byte(s) write(decode) or read(encode) in m_RLEBuffer.
        size_t GetRLEBufferDataSize() const;

        // Byte(s) write(decode) or read(encode) in m_UncompressBuffer.
        size_t GetUncompressBufferDataSize() const;

        int32_t   GetG4Error() const {
            return m_g4error;
            };

    private:
        void GetCode();

        void GetWhite();
        void GetBlack();

        void GetWhiteCodes();

        void GetBlackCodes();

        void CodeWhite(int32_t value);
        void CodeBlack(int32_t value);
        void OutputBits(int32_t word, int32_t nbits);

        void CreateLRSForEncode();

        void EncodeLine();

        void FillSpan(Byte* cp, int32_t x, int32_t count);
        int32_t  FindSpan(Byte** bpp, int32_t bs, int32_t be, Byte const* tab) const;
        int32_t  FindDiff(Byte* cp, int32_t bitStart, int32_t bitEnd, int32_t color) const;

        uint32_t            m_max_lrs;                    /*  max lrs allowed per scan line */
        Byte            m_g4mask;                     /*  mask word used to get next bit in g4word */

        Byte            m_g4word;                     /*  current Byte that is being disassembled */

        int32_t            m_g4error;                    /*  error flag for getting red_code */

        int32_t            m_code;                       /*  value of code returned by red_getcode () */

        uint32_t           m_lrsCount;

        int32_t* m_buf1;       /* buffer of transitions */
        char* m_buf1c;      /* color of transitions */
        int32_t* m_buf2;       /* buffer of transitions */
        char* m_buf2c;      /* color of transitions */

        int32_t* m_a;          /* pointer to buffer of transitions */
        char* m_acolor;     /* pointer to colors of transitions */
        int32_t* m_b;          /* pointer to buffer of transitions */
        char* m_bcolor;     /* pointer to colors of transitions */

        int32_t m_bstop;
        int32_t m_bstopp1;
        int32_t m_width;
        int32_t m_height;
        bool m_bufcode;

        // input/ouput buffers. Set via SetupForRLEMode() or SetupForUncompressMode().
        Byte* m_g4Buffer;
        size_t m_g4BufferSize;
        size_t m_g4BufferIndex;

        uint16_t* m_RLEBuffer;
        size_t   m_RLEBufferSize;
        size_t   m_RLEBufferIndex;

        Byte*  m_UncompressBuffer;
        size_t   m_UncompressBufferSize;
        size_t   m_UncompressBufferIndex;
        size_t   m_UncompressBufferLineSize;

        bool  m_bufferAllocated;
        bool  m_decodeMode;

        bool  m_bitrevtable;               //  lsb packing option
        bool  m_invertResult;              //  Invert black&white
        };

    CCITT4State     m_CCITT4State;
    };

END_IMAGEPP_NAMESPACE

