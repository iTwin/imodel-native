//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hcd/src/HCDCodecLZW.cpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

// Methods for class HCDCodecLZW
//-----------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>
#include <Imagepp/all/h/HCDCodecLZW.h>
#include <Imagepp/all/h/HCDLZWDecoder.h>
#include <Imagepp/all/h/HCDLZWEncoder.h>

#define XREPEAT4(n, op)        \
    switch (n) {        \
    default: { HSINTX i; for (i = n-4; i > 0; i--) { op; } } \
    case 2:  op;        \
    case 1:  op;        \
    case 0:  ;            \
    }

#define HCD_CODEC_NAME     L"LZW"


//-----------------------------------------------------------------------------
// public
// Default constructor
//-----------------------------------------------------------------------------
HCDCodecLZW::HCDCodecLZW()
    : HCDCodecImage(HCD_CODEC_NAME)
    {
    }

//-----------------------------------------------------------------------------
// public
// Default constructor
//-----------------------------------------------------------------------------
HCDCodecLZW::HCDCodecLZW(size_t     pi_Width,
                         size_t     pi_Height,
                         size_t     pi_BitsPerPixel,
                         unsigned short pi_Predictor)
    : HCDCodecImage(HCD_CODEC_NAME,
                    pi_Width,
                    pi_Height,
                    pi_BitsPerPixel)
    {
    m_Predictor = pi_Predictor;
    }

//-----------------------------------------------------------------------------
// public
// Copy constructor
//-----------------------------------------------------------------------------
HCDCodecLZW::HCDCodecLZW(const HCDCodecLZW& pi_rObj)
    : HCDCodecImage(pi_rObj)
    {
    m_Predictor = pi_rObj.m_Predictor;
    }

//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HCDCodecLZW::~HCDCodecLZW()
    {
    }

//-----------------------------------------------------------------------------
// public
// Clone
//-----------------------------------------------------------------------------
HCDCodec* HCDCodecLZW::Clone() const
    {
    return new HCDCodecLZW(*this);
    }

//-----------------------------------------------------------------------------
// public
// CompressSubset
//-----------------------------------------------------------------------------
size_t HCDCodecLZW::CompressSubset(const void* pi_pInData,
                                   size_t pi_InDataSize,
                                   void* po_pOutBuffer,
                                   size_t po_OutBufferSize)
    {
    size_t OutDataSize(0);

    HCDLZWEncoder   LZWEncoder;
    OutDataSize = LZWEncoder.Encode((Byte*)pi_pInData, pi_InDataSize, (Byte*)po_pOutBuffer, po_OutBufferSize);

    return OutDataSize;
    }

//-----------------------------------------------------------------------------
// public
// DecompressSubset
//-----------------------------------------------------------------------------
size_t HCDCodecLZW::DecompressSubset(const void*  pi_pInData,
                                     size_t pi_InDataSize,
                                     void*  po_pOutBuffer,
                                     size_t pi_OutBufferSize)
    {
    HDEBUGCODE(uint64_t OUTBOUND_OFFSET = (uint64_t)(po_pOutBuffer) + pi_OutBufferSize;);

    size_t OutDataSize = 0;

    // is it the first packet?
    if(GetSubsetPosY() == 0)
        {
        SetCurrentState(STATE_DECOMPRESS);
        }

    //TR 232508
    HCDLZWDecoder lzwDecoder;
    HSTATUS Error = lzwDecoder.Decode((Byte*)pi_pInData,
                                      pi_InDataSize,
                                      (Byte*)po_pOutBuffer,
                                      pi_OutBufferSize);

    if (Error == H_SUCCESS)
        {
        // libTiff 5.0 LZW bug.
        HASSERT(!( ((Byte*)pi_pInData)[0] == 0 && (((Byte*)pi_pInData)[1] & 0x1)));

#ifdef LZW_A_SLOW_DECODER
        // lzwDecoder.Decode2((Byte*)pi_pInData, pi_InDataSize, (Byte*)po_pOutBuffer, pi_OutBufferSize);
#endif

        if( m_Predictor == 2 )
            {
            HUINTX stride = GetBitsPerPixel() / 8;

            HUINTX rowSize = GetWidth() * stride;
            HUINTX cc      = rowSize;
            size_t row     = pi_OutBufferSize;

            char* op = (char*) po_pOutBuffer;

            while( row >= rowSize )
                {
                char* cp = (char*) op;

                HASSERT((uint64_t)cp < OUTBOUND_OFFSET);

                cc = rowSize;
                stride = GetBitsPerPixel() / 8;
                if (cc > stride)
                    {
                    cc -= stride;
                    // Pipeline the most common cases.
                    if (stride == 3)
                        {
                        HASSERT(((uint64_t)cp + 3) <= OUTBOUND_OFFSET);

                        uint32_t cr = cp[0];
                        uint32_t cg = cp[1];
                        uint32_t cb = cp[2];
                        do
                            {
                            cc -= 3, cp += 3;

                            HASSERT(((uint64_t)cp + 3) <= OUTBOUND_OFFSET);

                            cp[0] = (char)(cr += cp[0]);
                            cp[1] = (char)(cg += cp[1]);
                            cp[2] = (char)(cb += cp[2]);
                            }
                        while ( cc > 0);
                        }
                    else if (stride == 4)
                        {
                        HASSERT(((uint64_t)cp + 4) <= OUTBOUND_OFFSET);

                        uint32_t cr = cp[0];
                        uint32_t cg = cp[1];
                        uint32_t cb = cp[2];
                        uint32_t ca = cp[3];
                        do
                            {
                            cc -= 4, cp += 4;

                            HASSERT(((uint64_t)cp + 4) <= OUTBOUND_OFFSET);

                            cp[0] = (char)(cr += cp[0]);
                            cp[1] = (char)(cg += cp[1]);
                            cp[2] = (char)(cb += cp[2]);
                            cp[3] = (char)(ca += cp[3]);
                            }
                        while ( cc > 0);
                        }
                    else
                        {
                        do
                            {
                            HASSERT(((uint64_t)cp + stride) <= OUTBOUND_OFFSET);

                            XREPEAT4(stride, cp[stride] += *cp; cp++)
                            cc -= stride;
                            }
                        while ( cc > 0);
                        }
                    }
                row -= rowSize;

                op += rowSize;
                HASSERT(((uint64_t)op) <= OUTBOUND_OFFSET);
                }
            }

        SetCompressedImageIndex(GetCompressedImageIndex() + pi_InDataSize);
        SetSubsetPosY(GetSubsetPosY() + GetSubsetHeight());

        OutDataSize = ((GetSubsetWidth() + GetLinePaddingBits()) / 8) * GetSubsetHeight();

        if(GetSubsetPosY() == GetHeight())
            Reset();
        }

    return OutDataSize;
    }


//-----------------------------------------------------------------------------
// public
// SetDimensions
//-----------------------------------------------------------------------------
void HCDCodecLZW::SetDimensions(size_t pi_Width, size_t pi_Height)
    {
    HCDCodecImage::SetDimensions(pi_Width, pi_Height);

    /*

    CODE EN CONSEQUENCE

    */
    }

//-----------------------------------------------------------------------------
// public
// IsBitsPerPixelSupported
//-----------------------------------------------------------------------------
bool HCDCodecLZW::IsBitsPerPixelSupported(size_t pi_Bits) const
    {
    if((pi_Bits % 8) == 0)
        return true;
    else
        return false;
    }


