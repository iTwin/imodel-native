//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hcd/src/HCDCodecHMRPackBits.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HCDCodecHMRPackBits
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// The algorithms for compression/decompression come from the book
// Programming graphic files in C and C++, by John Levine, Wiley.
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HCDException.h>
#include <Imagepp/all/h/HCDCodecHMRPackBits.h>


//-----------------------------------------------------------------------------
// public
// Default constructor
//-----------------------------------------------------------------------------
HCDCodecHMRPackBits::HCDCodecHMRPackBits()
    : HCDCodecPackBits()
    {
    }

//-----------------------------------------------------------------------------
// public
// Default constructor
//-----------------------------------------------------------------------------
HCDCodecHMRPackBits::HCDCodecHMRPackBits(uint32_t pi_Width,
                                         uint32_t pi_Height,
                                         uint32_t pi_BitsPerPixel)
    : HCDCodecPackBits(pi_Width, pi_Height, pi_BitsPerPixel)
    {
    }

//-----------------------------------------------------------------------------
// public
// Copy constructor
//-----------------------------------------------------------------------------
HCDCodecHMRPackBits::HCDCodecHMRPackBits(const HCDCodecHMRPackBits& pi_rObj)
    : HCDCodecPackBits(pi_rObj)
    {
    }

//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HCDCodecHMRPackBits::~HCDCodecHMRPackBits()
    {
    }

//-----------------------------------------------------------------------------
// public
// GetSubsetMaxCompressedSize
//-----------------------------------------------------------------------------
size_t HCDCodecHMRPackBits::GetSubsetMaxCompressedSize() const
    {
    size_t NbBytes;

    NbBytes = (((GetSubsetWidth() * GetBitsPerPixel()) + 7) / 8) * GetSubsetHeight();

    // if we have this string "AAB" to compress,
    // the algo must be write "2A1B", 4 bytes ??? I think it's the worst case
    return ((size_t)(NbBytes / 3) + 1) * 4;
    }

//-----------------------------------------------------------------------------
// public
// Clone
//-----------------------------------------------------------------------------
HCDCodec* HCDCodecHMRPackBits::Clone() const
    {
    return new HCDCodecHMRPackBits(*this);
    }

//-----------------------------------------------------------------------------
// public
// CompressSubset
//-----------------------------------------------------------------------------
size_t HCDCodecHMRPackBits::CompressSubset(const void* pi_pInData,
                                           size_t pi_InDataSize,
                                           void* po_pOutBuffer,
                                           size_t po_OutBufferSize)
    {
    // is is the first subset
    if(GetSubsetPosY() == 0)
        SetCurrentState(STATE_COMPRESS);

    Byte* pIn = (Byte*)pi_pInData;
    Byte* pOut = (Byte*)po_pOutBuffer;

    size_t BytesPerLine =   (GetSubsetWidth() * GetBitsPerPixel()
                             + GetLinePaddingBits()) / 8;

    for(size_t Line = 0; Line < GetSubsetHeight(); Line++)
        {
        pOut = CompressRun(pIn, BytesPerLine, pOut);
        pIn += BytesPerLine;
        }

    SetSubsetPosY(GetSubsetPosY() + GetSubsetHeight());

    // is it the last subset?
    if(GetSubsetPosY() == GetHeight())
        Reset();

    HPOSTCONDITION(pOut <= (Byte*)po_pOutBuffer + po_OutBufferSize);
    return(pOut - (Byte*) po_pOutBuffer);
    }

//-----------------------------------------------------------------------------
// public
// DecompressSubset
//-----------------------------------------------------------------------------
size_t HCDCodecHMRPackBits::DecompressSubset(const void* pi_pInData,
                                             size_t pi_InDataSize,
                                             void* po_pOutBuffer,
                                             size_t po_OutBufferSize)
    {
    // is is the first subset
    if(GetSubsetPosY() == 0)
        SetCurrentState(STATE_DECOMPRESS);

    Byte* pIn = (Byte*)pi_pInData;
    Byte* pOut = (Byte*)po_pOutBuffer;

    size_t BytesPerLine = (GetSubsetWidth() * GetBitsPerPixel()
                           + GetLinePaddingBits()) / 8;

    size_t OutSize = BytesPerLine * GetSubsetHeight();
    // Decompress the full subset in one call.
    pIn = DecompressRun(pIn, pOut, OutSize);

    SetSubsetPosY(GetSubsetPosY() + GetSubsetHeight());

    size_t InDataSize = pIn - (Byte*)pi_pInData;

    // is it the last subset?
    if(GetSubsetPosY() == GetHeight())
        Reset();
    else
        SetCompressedImageIndex(GetCompressedImageIndex() + InDataSize);

    return(OutSize);
    }


//-----------------------------------------------------------------------------
// public
// HasLineAccess
//-----------------------------------------------------------------------------
bool HCDCodecHMRPackBits::HasLineAccess() const
    {
    return true;
    }

//-----------------------------------------------------------------------------
// private
// CompressRun
//-----------------------------------------------------------------------------
Byte* HCDCodecHMRPackBits::CompressRun(Byte*  pi_pIn,
                                        size_t  pi_InSize,
                                        Byte*  pOut)
    {

    enum state { BASE, LITERAL, RUN, LITERAL_RUN };
    state State = BASE;

    int32_t IdenticalBytes;
    Byte ByteValue;

    size_t BytesCount = pi_InSize;

    Byte* pLastLiteral = 0;

    while(BytesCount > 0)
        {
        ByteValue = *pi_pIn++;
        BytesCount--;
        IdenticalBytes = 1;

        for(; BytesCount > 0 && *pi_pIn == ByteValue; BytesCount--, pi_pIn++)
            IdenticalBytes++;

again:
        switch(State)
            {
            case BASE:

                if(IdenticalBytes > 1)
                    {
                    State = RUN;

                    if(IdenticalBytes > 128)
                        {
                        *pOut++ = -127;
                        *pOut++ = ByteValue;
                        IdenticalBytes -=128;
                        goto again;
                        }

                    *pOut++ = (Byte)(-(IdenticalBytes - 1));
                    *pOut++ = ByteValue;
                    }
                else
                    {
                    pLastLiteral = pOut;

                    *pOut++ = 0;
                    *pOut++ = ByteValue;
                    State = LITERAL;
                    }
                break;

            case LITERAL:

                if(IdenticalBytes > 1)
                    {
                    State = LITERAL_RUN;

                    if(IdenticalBytes > 128)
                        {
                        *pOut++ = -127;
                        *pOut++ = ByteValue;
                        IdenticalBytes -= 128;
                        goto again;
                        }

                    *pOut++ = (Byte)(- (IdenticalBytes - 1));
                    *pOut++ = ByteValue;
                    }
                else
                    {
                    if(++(*pLastLiteral) == 127)
                        State = BASE;

                    *pOut++ = ByteValue;
                    }

                break;

            case RUN:

                if(IdenticalBytes > 1)
                    {
                    if(IdenticalBytes > 128)
                        {
                        *pOut++ = -127;
                        *pOut++ = ByteValue;
                        IdenticalBytes -= 128;
                        goto again;
                        }

                    *pOut++ = (Byte)(- (IdenticalBytes - 1));
                    *pOut++ = ByteValue;
                    }
                else
                    {
                    pLastLiteral = pOut;
                    *pOut++ = 0;
                    *pOut++ = ByteValue;
                    State = LITERAL;
                    }
                break;

            case LITERAL_RUN:

                if( IdenticalBytes == 1 &&
                    pOut[-2] == (char)-1 &&
                    *pLastLiteral < 126)
                    {
                    State = (((*pLastLiteral) += 2) == 127 ? BASE: LITERAL);

                    pOut[-2] = pOut[-1];
                    }
                else
                    {
                    State = RUN;
                    }

                goto again;
            }
        }


    return pOut;
    }

//-----------------------------------------------------------------------------
// private
// DecompressRun
//-----------------------------------------------------------------------------
Byte* HCDCodecHMRPackBits::DecompressRun(Byte*    pi_pIn,
                                          Byte*    pi_pOut,
                                          size_t    pi_OutSize)
    {
    size_t BytesCount = pi_OutSize;

    int32_t IdenticalBytes;
    Byte  ByteValue;

    while(BytesCount > 0)
        {
        IdenticalBytes = (int32_t)*pi_pIn++;

        if(IdenticalBytes >= 128)
            IdenticalBytes -= 256;

        if(IdenticalBytes < 0)
            {

            if(IdenticalBytes == -128)
                continue;

            IdenticalBytes = -IdenticalBytes + 1;

            if(IdenticalBytes > (int32_t)BytesCount)
                throw HCDCorruptedPackbitsDataException();

            BytesCount -= IdenticalBytes;

            for(ByteValue = *pi_pIn++; IdenticalBytes-- > 0;)
                *pi_pOut++ = ByteValue;
            }
        else
            {
            ++IdenticalBytes;

            if(IdenticalBytes > (int32_t)BytesCount)
                throw HCDCorruptedPackbitsDataException();

            memcpy(pi_pOut, pi_pIn, IdenticalBytes);

            pi_pIn += IdenticalBytes;

            pi_pOut += IdenticalBytes;
            BytesCount -= IdenticalBytes;
            }
        }

    return(pi_pIn);
    }
