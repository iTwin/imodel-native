//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hcd/src/HCDCodecZlib.cpp $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HCDCodecZlib
//-----------------------------------------------------------------------------

#include <ImageppInternal.h>


#include <Imagepp/all/h/HCDCodecZlib.h>
#include <zlib/zlib.h>

//-----------------------------------------------------------------------------
// public
// Default constructor
//-----------------------------------------------------------------------------
HCDCodecZlib::HCDCodecZlib()
    : HCDCodecDeflate()
    {
    }

//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
HCDCodecZlib::HCDCodecZlib(size_t pi_DataSize)
    : HCDCodecDeflate(pi_DataSize)
    {
    // These members are only used if Predictor == 2.
    m_Predictor       = 1;  // None
    m_BitsPerPixel    = 8;
    m_SamplesPerPixel = 1;
    m_Width           = 0;
    }

HCDCodecZlib::HCDCodecZlib(size_t pi_DataSize, uint32_t pi_Width, uint32_t pi_BitsPerPixel, uint16_t pi_Predictor, uint32_t pi_SamplesPerPixel)
    : HCDCodecDeflate(pi_DataSize)
{
    // These members are only used if Predictor == 2.
    m_Predictor         = pi_Predictor;
    m_BitsPerPixel      = pi_BitsPerPixel;
    m_SamplesPerPixel   = pi_SamplesPerPixel;
    m_Width             = pi_Width;
}

//-----------------------------------------------------------------------------
// public
// Copy constructor
//-----------------------------------------------------------------------------
HCDCodecZlib::HCDCodecZlib(const HCDCodecZlib& pi_rObj)
    : HCDCodecDeflate(pi_rObj)
    {
    m_Predictor       = pi_rObj.m_Predictor;
    m_BitsPerPixel    = pi_rObj.m_BitsPerPixel;
    m_SamplesPerPixel = pi_rObj.m_SamplesPerPixel;
    m_Width           = pi_rObj.m_Width;
    }

//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HCDCodecZlib::~HCDCodecZlib()
    {
    }

//-----------------------------------------------------------------------------
// public
// Clone
//-----------------------------------------------------------------------------
HCDCodec* HCDCodecZlib::Clone() const
    {
    return new HCDCodecZlib(*this);
    }

//-----------------------------------------------------------------------------
// public
// CompressSubset
//-----------------------------------------------------------------------------
size_t HCDCodecZlib::CompressSubset(const void* pi_pInData,
                                    size_t pi_InDataSize,
                                    void* po_pOutBuffer,
                                    size_t pi_OutBufferSize)
    {
    HASSERT_X64(pi_InDataSize < ULONG_MAX);
    HASSERT_X64(pi_OutBufferSize < ULONG_MAX);

    int err;

    uLongf OutLen = (uLongf)pi_OutBufferSize;

    // Use level 5 compression. Varies from 1 to 9, 6 being the default.
    // There seems to be a big difference between 5 and 6 on execution time for
    // big images, with only a small size penalty.
    err = compress2((Byte*)po_pOutBuffer, &OutLen, (Byte*)pi_pInData, (uint32_t)pi_InDataSize, 5);

    if(err != Z_OK)
        OutLen = 0;

    return OutLen;
    }


// Predictor == 2 and SamplesPerPixel == 1
//
//  Data_T   --> uint16_t or uint32_t
//
//  pi_WidthSize --> Width * (BitsPerPixel / 8)
//  
template<class Data_T>
void HorizontalDifferencing_16_32(Byte* pio_Buffer, size_t pi_BufferSize, uint32_t pi_WidthSize)
    {
    HDEBUGCODE(uint64_t OUTBOUND_OFFSET = (uint64_t)(pio_Buffer)+pi_BufferSize;);

    uint32_t cc = pi_WidthSize;
    size_t  row = pi_BufferSize;

    Byte* op = (Byte*)pio_Buffer;

    while (row >= pi_WidthSize)
        {
        char* cp = (char*)op;

        HASSERT((uint64_t)cp < OUTBOUND_OFFSET);

        cc = pi_WidthSize;

        Data_T* wp = (Data_T*)cp;
        size_t wc = cc / sizeof(Data_T);

        HASSERT((cc % 2) == 0);

        if (wc > 1)
            {
            wc -= 1;
            do
                {
                wp[1] += wp[0];
                wp++;
                wc -= 1;
                } while (wc > 0);
            }

        row -= pi_WidthSize;
        op += pi_WidthSize;
        HASSERT(((uint64_t)op) <= OUTBOUND_OFFSET);
        }
    }

// Predictor == 2 
//
//  Data_T   --> uint8_t or Byte
//  pi_SamplesPerPixel --> 3 or 4 normally for V24R8G8B8 / V32R8G8B8
//  pi_WidthSize --> Width * (BitsPerPixel / 8)
// 
#define REPEAT4(n, op)		\
    switch (n) {		\
    default: { size_t i; for (i = n-4; i > 0; i--) { op; } } \
    case 4:  op;		\
    case 3:  op;		\
    case 2:  op;		\
    case 1:  op;		\
    case 0:  ;			\
    }

void HorizontalDifferencing_8(Byte* pio_Buffer, size_t pi_BufferSize, uint32_t pi_WidthSize, uint32_t pi_SamplesPerPixel)
    {
    HDEBUGCODE(uint64_t OUTBOUND_OFFSET = (uint64_t)(pio_Buffer)+pi_BufferSize;);

    uint32_t cc = pi_WidthSize;
    size_t  row = pi_BufferSize;

    Byte* op = (Byte*)pio_Buffer;

    while (row >= pi_WidthSize)
    {
        char* cp = (char*)op;

        HASSERT((uint64_t)cp < OUTBOUND_OFFSET);

        cc = pi_WidthSize;
        if (cc > pi_SamplesPerPixel)
        {
            cc -= pi_SamplesPerPixel;
            // Pipeline the most common cases.
            if (pi_SamplesPerPixel == 3)
            {
                HASSERT(((uint64_t)cp + 3) <= OUTBOUND_OFFSET);

                uint32_t cr = cp[0];
                uint32_t cg = cp[1];
                uint32_t cb = cp[2];
                do
                {
                    cc -= 3, cp += 3;

                    HASSERT(((uint64_t)cp + 3) <= OUTBOUND_OFFSET);

                    cp[0] = ((cr += cp[0]) & 0xFF);
                    cp[1] = ((cg += cp[1]) & 0xFF);
                    cp[2] = ((cb += cp[2]) & 0xFF);
                } while (cc > 0);
            }
            else if (pi_SamplesPerPixel == 4)
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

                    cp[0] = ((cr += cp[0]) & 0xFF);
                    cp[1] = ((cg += cp[1]) & 0xFF);
                    cp[2] = ((cb += cp[2]) & 0xFF);
                    cp[3] = ((ca += cp[3]) & 0xFF);
                } while (cc > 0);
            }
            else
            {
                do
                {
                    HASSERT(((uint64_t)cp + pi_SamplesPerPixel) <= OUTBOUND_OFFSET);

                    REPEAT4(pi_SamplesPerPixel, cp[pi_SamplesPerPixel] += *cp; cp++)
                        cc -= pi_SamplesPerPixel;
                } while (cc > 0);
            }
        }

        row -= pi_WidthSize;
        op += pi_WidthSize;
        HASSERT(((uint64_t)op) <= OUTBOUND_OFFSET);
    }
}



//-----------------------------------------------------------------------------
// public
// DecompressSubset
//-----------------------------------------------------------------------------
size_t HCDCodecZlib::DecompressSubset(const void* pi_pInData,
                                      size_t pi_InDataSize,
                                      void* po_pOutBuffer,
                                      size_t pi_OutBufferSize)
    {
    HASSERT_X64(pi_InDataSize < ULONG_MAX);
    HASSERT_X64(pi_OutBufferSize < ULONG_MAX);

    int err;

    uLongf OutLen = (uLongf)pi_OutBufferSize;

    err = uncompress((Byte*)po_pOutBuffer, &OutLen, (Byte*)pi_pInData, (uint32_t)pi_InDataSize);

    if(err != Z_OK)
        OutLen = 0;

    if (m_Predictor == 2)
        {
        if (1 == m_SamplesPerPixel && m_BitsPerPixel == 16)         // V16Gray
            HorizontalDifferencing_16_32<uint16_t>((Byte*)po_pOutBuffer, pi_OutBufferSize, m_Width * 2);
        else if (1 == m_SamplesPerPixel && m_BitsPerPixel == 32)    // V32Gray
            HorizontalDifferencing_16_32<uint32_t>((Byte*)po_pOutBuffer, pi_OutBufferSize, m_Width * 4);
        else if (4 == m_SamplesPerPixel && m_BitsPerPixel == 32)    // V32RGBA
            HorizontalDifferencing_8((Byte*)po_pOutBuffer, pi_OutBufferSize, m_Width * m_SamplesPerPixel, m_SamplesPerPixel);
        else if (3 == m_SamplesPerPixel && m_BitsPerPixel == 24)    // V24RGB
            HorizontalDifferencing_8((Byte*)po_pOutBuffer, pi_OutBufferSize, m_Width * m_SamplesPerPixel, m_SamplesPerPixel);
        else
            { HASSERT(false);}
        }

    return OutLen;
    }

//-----------------------------------------------------------------------------
// public
// GetSubsetMaxCompressedSize
//-----------------------------------------------------------------------------
size_t HCDCodecZlib::GetSubsetMaxCompressedSize() const
    {
    return ((size_t)(GetSubsetSize() * 1.1 + 12 +
                     //security extra
                     5));
    }
