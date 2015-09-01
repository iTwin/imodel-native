//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hcd/src/HCDCodecBMPRLE8.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for class HCDCodecBMPRLE8
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HCDCodecBMPRLE8.h>

static const Byte s_BitMask[8]    = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };

#define HCD_CODEC_NAME L"BMP RLE8bits"

//-----------------------------------------------------------------------------
// public
// Default constructor
//-----------------------------------------------------------------------------
HCDCodecBMPRLE8::HCDCodecBMPRLE8()
    : HCDCodecImage(HCD_CODEC_NAME)
    {
    }

//-----------------------------------------------------------------------------
// public
// Default constructor
//-----------------------------------------------------------------------------
HCDCodecBMPRLE8::HCDCodecBMPRLE8(size_t pi_Width,
                                 size_t pi_Height)
    : HCDCodecImage(HCD_CODEC_NAME,
                    pi_Width,
                    pi_Height,
                    8)
    {
    }

//-----------------------------------------------------------------------------
// public
// Copy constructor
//-----------------------------------------------------------------------------
HCDCodecBMPRLE8::HCDCodecBMPRLE8(const HCDCodecBMPRLE8& pi_rObj)
    : HCDCodecImage(pi_rObj)
    {
    }

//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HCDCodecBMPRLE8::~HCDCodecBMPRLE8()
    {
    }

//-----------------------------------------------------------------------------
// public
// Clone
//-----------------------------------------------------------------------------
HCDCodec* HCDCodecBMPRLE8::Clone() const
    {
    return new HCDCodecBMPRLE8(*this);
    }

//-----------------------------------------------------------------------------
// public
// CompressSubset
//-----------------------------------------------------------------------------
size_t HCDCodecBMPRLE8::CompressSubset(const void* pi_pInData,
                                       size_t pi_InDataSize,
                                       void* po_pOutBuffer,
                                       size_t po_OutBufferSize)
    {
    // HLXXX Note: The compression algorithm is not optimal. The compression has been
    // done very simply. However, there are some combinations of run lenght and
    // non-run lenghts that could be quicker... The algo is more complex however....

    // is it the first packet?
    if(GetSubsetPosY() == 0)
        {
        SetCurrentState(STATE_COMPRESS);
        }

    Byte* pSrc = (Byte*)pi_pInData;
    Byte* pDest = (Byte*)po_pOutBuffer;

    Byte* pLen;

    size_t LinesCount = GetSubsetHeight();

    size_t LineBytesCount;
    Byte Value;
    int32_t Len;

    Byte* pLineBuffer = new Byte[GetSubsetWidth()*2];
    Byte* pLineBufferBegin = pLineBuffer;
    uint32_t LineBufferCount = 0;

    // We begin to compress the last line first !!!
    pSrc += ((GetSubsetHeight()*GetSubsetWidth()) - GetSubsetWidth());

    while(LinesCount != 0)
        {
        LineBytesCount = GetSubsetWidth();

        // Loop until we compress all the bytes
        while(LineBytesCount != 0)
            {
            // Get the value of the first byte
            Value = *pSrc;
            Len = 1;

            pSrc++;
            LineBytesCount--;

            // Temporary variable used to find repetitive pattern.
            Byte* pTmpSrc = pSrc;
            size_t  TmpLineBytesCount = LineBytesCount;
            int32_t TmpLen = Len;

            // Compare the value with the following bytes and test if there
            // is a repetitive pattern.
            while(TmpLineBytesCount != 0 && Value == *pTmpSrc && TmpLen < 255)
                {
                pTmpSrc++;
                TmpLineBytesCount--;
                TmpLen++;
                }

            // If the len is longer or equal to 3 bytes or if we reach the last 3 bytes
            // of the line, store the repetitiv pattern.
            if(TmpLen >= 3 || TmpLineBytesCount<3)
                {
                *pLineBuffer = (Byte)TmpLen;
                pLineBuffer++;
                *pLineBuffer = Value;
                pLineBuffer++;

                LineBufferCount += 2;
                LineBytesCount = TmpLineBytesCount;
                pSrc = pTmpSrc;
                }
            // Otherwise, test for the longer non-repetitive pattern
            else
                {
                // If LineBytesCount<3 we are at the end of the line and we do not
                // want to write absolute code with a len smaler then 3. So we will force
                // compresion at the next loop.
                if (!(LineBytesCount<3))
                    {
                    // We need to write at least 3 bytes.
                    *pLineBuffer = 0;
                    pLineBuffer++;

                    // We will write the len later.
                    pLen    = pLineBuffer;
                    pLineBuffer++;

                    // First byte
                    *pLineBuffer  = Value;
                    pLineBuffer++;

                    // Second byte
                    *pLineBuffer  = *pSrc;
                    pLineBuffer++;
                    pSrc++;
                    Len++;
                    LineBytesCount--;

                    // Third byte
                    *pLineBuffer  = *pSrc;
                    pLineBuffer++;
                    pSrc++;
                    Len++;
                    LineBytesCount--;

                    // Test for the longer non repetitive string greater than 3
                    while(LineBytesCount != 0  &&
                          ((LineBytesCount < 3) || (*pSrc != *(pSrc+1) || *pSrc != *(pSrc+2) || *pSrc != *(pSrc+3))) &&
                          Len < 255)
                        {
                        *pLineBuffer = *pSrc;
                        pLineBuffer++;
                        Len++;
                        pSrc++;
                        LineBytesCount--;
                        }
                    *pLen = (Byte)Len;

                    // Find the number of pading byte to use.
                    // Each run must be aligned on a word boundary (16bits).
                    uint32_t PadBytesCount = *pLen % 2;
                    while(PadBytesCount != 0)
                        {
                        *pLineBuffer = 0;
                        pLineBuffer++;
                        Len++;
                        PadBytesCount--;
                        }
                    LineBufferCount += (2 + Len);
                    }
                }
            }

        memcpy(pDest, pLineBufferBegin, LineBufferCount);
        pDest += LineBufferCount;

        // End of line code (0x0000)
        *pDest = 0x00;
        pDest++;
        *pDest = 0x00;
        pDest++;

        pSrc -= (GetSubsetWidth()*2);

        LinesCount--;
        pLineBuffer     = pLineBufferBegin;
        LineBufferCount = 0;
        }

    delete pLineBufferBegin;

    // End of bits maps (0x0001)
    *pDest = 0x00;
    pDest++;
    *pDest = 0x01;
    pDest++;

    SetSubsetPosY(GetSubsetPosY() + GetSubsetHeight());

    if(GetSubsetPosY() == GetHeight())
        Reset();

    return(((Byte*)pDest) - ((Byte*)po_pOutBuffer));
    }

//-----------------------------------------------------------------------------
// public
// DecompressSubset
//-----------------------------------------------------------------------------
size_t HCDCodecBMPRLE8::DecompressSubset(const void* pi_pInData,
                                         size_t      pi_InDataSize,
                                         void*       po_pOutBuffer,
                                         size_t      pi_OutBufferSize)
    {
    // Is it the first packet?
    if(GetSubsetPosY() == 0)
        {
        SetCurrentState(STATE_DECOMPRESS);
        }

    size_t Count;
    size_t BytesCount;
    size_t LineBytesCount;
    Byte XDisplacement    = 0;
    Byte YDisplacement    = 0;
    bool  EndOfLineReached = false;

    Byte* pSrc  = (Byte*)pi_pInData;
    Byte* pDest = (Byte*)po_pOutBuffer;

    BytesCount = pi_OutBufferSize;

    // We begin to write at the begining of the last line !!!
    pDest += (pi_OutBufferSize - GetSubsetWidth());

    while(BytesCount)
        {
        LineBytesCount = GetSubsetWidth();

        while(LineBytesCount > 0)
            {
            if(*pSrc != 0)
                {
                // We are in encode mode and (*pSrc) contains
                // the number of pixels to be drawn using the color indexe
                // in the next byte
                Count = (*pSrc);
                pSrc++;
                if(Count >= LineBytesCount)
                    {
                    memset(pDest, *(pSrc), LineBytesCount);
                    pDest += LineBytesCount;
                    BytesCount -= LineBytesCount;
                    LineBytesCount -= LineBytesCount;
                    }
                else
                    {
                    memset(pDest, *(pSrc), Count);
                    pDest += Count;
                    BytesCount -= Count;
                    LineBytesCount -= Count;
                    }

                pSrc++;
                EndOfLineReached = false;
                }
            else
                {
                // We know that we are in absulute mode or that we have an escape code
                // of the encode mode
                pSrc++;

                if(*pSrc >= 0x03)
                    {
                    // We know that we are in absolute mode and (*pSrc)
                    // contains the number of not compress byte that follow.
                    // Each run must be aligned on a word boundary.

                    Count = (*pSrc);
                    pSrc++;
                    if(Count >= LineBytesCount)
                        {
                        memcpy(pDest, pSrc, LineBytesCount);
                        pDest += LineBytesCount;
                        BytesCount -= LineBytesCount;
                        LineBytesCount -= LineBytesCount;
                        }
                    else
                        {
                        memcpy(pDest, pSrc, Count);
                        pDest += Count;
                        BytesCount -= Count;
                        LineBytesCount -= Count;
                        }

                    pSrc += (((int32_t)((Count + 1) / 2)) * 2);
                    EndOfLineReached = false;
                    }
                else if(*pSrc == 0x02)
                    {
                    // We know that we have an delta escape code of the encode mode.
                    // memset(pDest, 0x00, BytesCount);

                    size_t OffsetToEndOfLine;
                    size_t TotalByteOfTheDisplacement;

                    XDisplacement = *(++pSrc);
                    YDisplacement = *(++pSrc);

                    TotalByteOfTheDisplacement = (YDisplacement * GetSubsetWidth()) + XDisplacement;
                    OffsetToEndOfLine          = ((BytesCount-1) % GetSubsetWidth() + 1);

                    BytesCount -= TotalByteOfTheDisplacement;

                    if (TotalByteOfTheDisplacement <= OffsetToEndOfLine)
                        {
                        memset(pDest, 0x00, TotalByteOfTheDisplacement);
                        pDest += TotalByteOfTheDisplacement;
                        LineBytesCount -= TotalByteOfTheDisplacement;
                        }
                    else
                        {
                        // Fill the current line first.
                        memset(pDest, 0x00, OffsetToEndOfLine);
                        TotalByteOfTheDisplacement -= OffsetToEndOfLine;
                        pDest += OffsetToEndOfLine;

                        // Fill all complete line.
                        while(TotalByteOfTheDisplacement >= GetSubsetWidth())
                            {
                            pDest -= (2*GetSubsetWidth());
                            memset(pDest, 0x00, GetSubsetWidth());
                            TotalByteOfTheDisplacement -= GetSubsetWidth();
                            pDest += GetSubsetWidth();

                            }

                        // Fill the last line.
                        if(TotalByteOfTheDisplacement != 0)
                            {
                            pDest -= (2*GetSubsetWidth());
                            memset(pDest, 0x00, TotalByteOfTheDisplacement);
                            pDest += TotalByteOfTheDisplacement;
                            LineBytesCount = GetSubsetWidth() - TotalByteOfTheDisplacement;
                            }
                        }
                    EndOfLineReached = false;
                    pSrc++;
                    }
                else
                    {
                    // We know that we have an escape code of the encode mode.
                    // End of line or end of bitmap.

                    if((!EndOfLineReached) && *pSrc == 0x00 && BytesCount != 0)
                        {
                        // We are at the end of the line.
                        // So we want to move at the begining of the next line but in reverse.
                        size_t NumberOfBytesToEndOfLine = BytesCount % GetSubsetWidth();

                        if(NumberOfBytesToEndOfLine == 0)
                            NumberOfBytesToEndOfLine = GetSubsetWidth(); // Empty line

                        // Memset the destination buffer to zero because some
                        // application (Photoshop 5.5) do not encode the byte
                        // set to zero at the end of the line.
                        memset(pDest, 0x00, NumberOfBytesToEndOfLine);

                        pDest += NumberOfBytesToEndOfLine;
                        BytesCount -= NumberOfBytesToEndOfLine;
                        LineBytesCount -= NumberOfBytesToEndOfLine;

                        pDest -= (2*GetSubsetWidth());
                        EndOfLineReached = true;
                        }
                    else if(*pSrc == 0x01)
                        {
                        // We are at the end of the bitmap.
                        if (BytesCount != 0)
                            {
                            size_t OffsetToEndOfLine;

                            OffsetToEndOfLine = ((BytesCount-1) % GetSubsetWidth() + 1);

                            // Fill the current line first.
                            memset(pDest, 0x00, OffsetToEndOfLine);
                            BytesCount -= OffsetToEndOfLine;
                            pDest += OffsetToEndOfLine;

                            // Fill the rest of the data to zero.
                            pDest = (Byte*)po_pOutBuffer;

                            memset(pDest, 0x00, BytesCount);

                            BytesCount = 0;
                            LineBytesCount = 0;
                            EndOfLineReached = true;
                            }
                        }

                    pSrc++;
                    }
                }

            if (!EndOfLineReached && (BytesCount % GetSubsetWidth()) == 0)
                {
                while((pSrc[0] != 0x00) && (pSrc[1] == 0x00))
                    {
                    if(*pSrc != 0)
                        {
                        // We are in encode mode and (*pSrc) contains
                        // the number of pixels to be drawn using the color indexe
                        // in the next byte
                        pSrc+=2;
                        }
                    else
                        {
                        // We know that we are in absulute mode or that we have an escape code
                        // of the encode mode
                        pSrc++;

                        if(*pSrc >= 0x03)
                            {
                            // We know that we are in absolute mode and (*pSrc)
                            // contains the number of not compress byte that follow.
                            // Each run must be aligned on a word boundary.
                            Count = (*pSrc);
                            pSrc++;
                            pSrc += (((int32_t)((Count + 1) / 2)) * 2);

                            }
                        else
                            pSrc++;
                        }

                    }

                if (BytesCount != 0)
                    {
                    // We are at the end of the line.
                    // So we want to move at the begining of the next line but in reverse.
                    pDest -= (2*GetSubsetWidth());

                    EndOfLineReached = true;
                    LineBytesCount = 0;
                    }
                }
            }
        }

    SetSubsetPosY(GetSubsetPosY() + GetSubsetHeight());

    size_t OutDataSize = (GetSubsetWidth() * GetSubsetHeight());

    if(GetSubsetPosY() == GetHeight())
        Reset();

    return(OutDataSize);

    }

//-----------------------------------------------------------------------------
// public
// HasLineAccess
//-----------------------------------------------------------------------------
bool HCDCodecBMPRLE8::HasLineAccess() const
    {
    return false;
    }

//-----------------------------------------------------------------------------
// public
// IsBitsPerPixelSupported
//-----------------------------------------------------------------------------
bool HCDCodecBMPRLE8::IsBitsPerPixelSupported(size_t pi_Bits) const
    {
    if(pi_Bits == 8)
        return true;
    else
        return false;
    }


//-----------------------------------------------------------------------------
// public
// GetSubsetMaxCompressedSize
//-----------------------------------------------------------------------------
size_t HCDCodecBMPRLE8::GetSubsetMaxCompressedSize() const
    {
    return (GetSubsetWidth() * 5 * GetSubsetHeight());
    }

