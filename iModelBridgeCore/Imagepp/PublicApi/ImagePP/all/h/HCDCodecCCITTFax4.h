//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HCDCodecCCITTFax4.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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
                                   void* po_pOutBuffer,   size_t pi_OutBufferSize);

    size_t          DecompressSubset(const void* pi_pInData, size_t pi_InDataSize,
                                     void* po_pOutBuffer,    size_t pi_OutBufferSize);

    // Binary optimization: Speed up read when codec support direct decompression to RLE format.
    virtual HCDCodecRLEInterface*   GetRLEInterface();
    IMAGEPP_EXPORT virtual void             DecompressSubsetToRLE(const void* pi_pInData, size_t pi_InDataSize, HFCPtr<HCDPacketRLE>& pio_rpRLEPacket) override;
    IMAGEPP_EXPORT virtual size_t           CompressSubsetFromRLE(HFCPtr<HCDPacketRLE> const& pi_rpPacketRLE, void* po_pOutBuffer, size_t po_OutBufferSize) override;

    bool           HasLineAccess() const;

    void            Reset();

    virtual HCDCodec* Clone() const override;
    
protected:

private:

    struct CCITT4State
        {
        CCITT4State();
        CCITT4State(const CCITT4State& pi_rObj);
        ~CCITT4State();

        void Pre(long pi_Width, long pi_Height, bool pi_Decode, bool pi_UseBitRevTable, bool pi_InvertResult);
        void Post();
        void Reset();
        void GrowMaxLRS();

        void EncodeLineFromBit();
        void EncodeLineFromRLE();

        void DecodeLine();
        void DecodeLineRLE();

        // Buffer use for RLE decode/encode routines.
        void SetupForRLEMode(Byte* pio_pG4Buffer, size_t pi_G4BufferSize, unsigned short* pio_pRLEBuffer, size_t pi_RLEBufferSize);

        // Buffer use for RLE decode/encode routines. Must have setup for RLEMode before calling this method.
        void SetRLEBuffer(unsigned short* pio_pRLEBuffer, size_t pi_RLEBufferSize);

        // Buffer use for uncompress decode/encode routines.
        void SetupForUncompressMode(Byte* pio_pG4Buffer, size_t pi_G4BufferSize, Byte* pio_pUncompressBuffer, size_t pi_UncompressBufferSize, size_t pi_UncompressLineSize);

        // Byte(s) read(decode) or write(encode) in m_g4Buffer.
        size_t GetG4BufferDataSize() const;

        // Byte(s) write(decode) or read(encode) in m_RLEBuffer.
        size_t GetRLEBufferDataSize() const;

        // Byte(s) write(decode) or read(encode) in m_UncompressBuffer.
        size_t GetUncompressBufferDataSize() const;

        long   GetG4Error() const {
            return m_g4error;
            };

    private:
        void GetCode();

        void GetWhite();
        void GetBlack();

        void GetWhiteCodes();

        void GetBlackCodes();

        void CodeWhite(long value);
        void CodeBlack(long value);
        void OutputBits(long word, long nbits);

        void CreateLRSForEncode();

        void EncodeLine();

        void FillSpan(Byte* cp, int x, int count);
        int  FindSpan(Byte** bpp, int bs, int be, register Byte const* tab) const;
        int  FindDiff(Byte* cp, int bitStart, int bitEnd, int color) const;

        long            m_max_lrs;                    /*  max lrs allowed per scan line */
        Byte            m_g4mask;                     /*  mask word used to get next bit in g4word */

        Byte            m_g4word;                     /*  current Byte that is being disassembled */

        long            m_g4error;                    /*  error flag for getting red_code */

        long            m_code;                       /*  value of code returned by red_getcode () */

        long            m_lrsCount;

        long* m_buf1;       /* buffer of transitions */
        char* m_buf1c;      /* color of transitions */
        long* m_buf2;       /* buffer of transitions */
        char* m_buf2c;      /* color of transitions */

        long* m_a;          /* pointer to buffer of transitions */
        char* m_acolor;     /* pointer to colors of transitions */
        long* m_b;          /* pointer to buffer of transitions */
        char* m_bcolor;     /* pointer to colors of transitions */

        long m_bstop;
        long m_bstopp1;
        long m_width;
        long m_height;
        bool m_bufcode;

        // input/ouput buffers. Set via SetupForRLEMode() or SetupForUncompressMode().
        Byte* m_g4Buffer;
        size_t m_g4BufferSize;
        size_t m_g4BufferIndex;

        unsigned short* m_RLEBuffer;
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

