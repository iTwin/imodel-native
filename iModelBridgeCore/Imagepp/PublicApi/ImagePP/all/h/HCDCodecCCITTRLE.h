//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HCDCodecCCITTRLE
//-----------------------------------------------------------------------------
#pragma once

#include "HCDCodecCCITT.h"

BEGIN_IMAGEPP_NAMESPACE

class HCDCodecHMRRLE1;
class HCDPacketRLE;

class HCDCodecCCITTRLE : public HCDCodecCCITT, HCDCodecRLEInterface
    {
    HDECLARE_CLASS_ID(HCDCodecId_CCITTRLE, HCDCodecCCITT)

public:
    IMAGEPP_EXPORT         HCDCodecCCITTRLE();

    IMAGEPP_EXPORT         HCDCodecCCITTRLE(uint32_t pi_Width, uint32_t pi_Height);


    HCDCodecCCITTRLE(const HCDCodecCCITTRLE& pi_rObj);

    virtual         ~HCDCodecCCITTRLE();

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

    struct CCITTRLEState
        {
        CCITTRLEState();
        CCITTRLEState(const CCITTRLEState& pi_rObj);
        ~CCITTRLEState();

        void Pre(int32_t pi_Width, int32_t pi_Height, bool pi_Decode, bool pi_UseBitRevTable, bool pi_InvertResult);
        void Post();
        void Reset();
        void GrowMaxLRS();

        void EncodeLineFromBit();
        void EncodeLineFromRLE();

        void DecodeLine();
        void DecodeLineRLE();

        // Buffer use for RLE decode/encode routines.
        void SetupForRLEMode(Byte* pio_pCCITTRLEBuffer, size_t pi_CCITTRLEBufferSize, uint16_t* pio_pRLEBuffer, size_t pi_RLEBufferSize);

        // Buffer use for RLE decode/encode routines. Must have setup for RLEMode before calling this method.
        void SetRLEBuffer(uint16_t* pio_pRLEBuffer, size_t pi_RLEBufferSize);

        // Buffer use for uncompress decode/encode routines.
        void SetupForUncompressMode(Byte* pio_pCCITTRLEBuffer, size_t pi_CCITTRLEBufferSize, Byte* pio_pUncompressBuffer, size_t pi_UncompressBufferSize, size_t pi_UncompressLineSize);

        // Byte(s) read(decode) or write(encode) in m_ccittrleBuffer.
        size_t GetCCITTRLEBufferDataSize() const;

        // Byte(s) write(decode) or read(encode) in m_RLEBuffer.
        size_t GetRLEBufferDataSize() const;

        // Byte(s) write(decode) or read(encode) in m_UncompressBuffer.
        size_t GetUncompressBufferDataSize() const;

        int32_t   GetCCITTRLEError() const {
            return m_ccittrleError;
            };

    private:
        void GetCode(int32_t pi_color);

        void GetWhite();
        void GetBlack();

        void CodeWhite(int32_t value);
        void CodeBlack(int32_t value);
        void OutputBits(int32_t word, int32_t nbits);

        void CreateLRSForEncode();

        void EncodeLine();

        void FillSpan(Byte* cp, int32_t x, int32_t count);
        int32_t  FindSpan(Byte** bpp, int32_t bs, int32_t be, Byte const* tab) const;
        int32_t  FindDiff(Byte* cp, int32_t bitStart, int32_t bitEnd, int32_t color) const;

        uint32_t           m_max_lrs;                    /*  max lrs allowed per scan line */
        Byte m_ccittrleMask;                     /*  mask word used to get next bit in g4word */

        Byte m_ccittrleWord;                     /*  current Byte that is being disassembled */

        int32_t            m_ccittrleError;                    /*  error flag for getting red_code */

        int32_t            m_code;                       /*  value of code returned by red_getcode () */

        uint32_t           m_lrsCount;

        bool  m_eol;
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
        Byte* m_ccittrleBuffer;
        size_t m_ccittrleBufferSize;
        size_t m_ccittrleBufferIndex;

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

    CCITTRLEState     m_CCITTRLEState;
    };

END_IMAGEPP_NAMESPACE

