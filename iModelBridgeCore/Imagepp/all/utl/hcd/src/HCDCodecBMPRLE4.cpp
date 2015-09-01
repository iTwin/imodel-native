//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hcd/src/HCDCodecBMPRLE4.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HCDCodecBMPRLE4
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HCDCodecBMPRLE4.h>
#include <Imagepp/all/h/HFCMath.h>

#define HCD_CODEC_NAME L"BMP RLE 4 bits"

//-----------------------------------------------------------------------------
// public
// Default constructor
//-----------------------------------------------------------------------------
HCDCodecBMPRLE4::HCDCodecBMPRLE4()
    : HCDCodecImage(HCD_CODEC_NAME)
    {
    }

//-----------------------------------------------------------------------------
// public
// Default constructor
//-----------------------------------------------------------------------------
HCDCodecBMPRLE4::HCDCodecBMPRLE4(size_t pi_Width,
                                 size_t pi_Height)
    : HCDCodecImage(HCD_CODEC_NAME,
                    pi_Width,
                    pi_Height,
                    4)
    {
    }

//-----------------------------------------------------------------------------
// public
// Copy constructor
//-----------------------------------------------------------------------------
HCDCodecBMPRLE4::HCDCodecBMPRLE4 (const HCDCodecBMPRLE4& pi_rObj)
    : HCDCodecImage(pi_rObj)
    {
    }

//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HCDCodecBMPRLE4::~HCDCodecBMPRLE4()
    {
    }

//-----------------------------------------------------------------------------
// public
// Clone
//-----------------------------------------------------------------------------
HCDCodec* HCDCodecBMPRLE4::Clone() const
    {
    return new HCDCodecBMPRLE4(*this);
    }

//-----------------------------------------------------------------------------
// public
// CompressSubset
//-----------------------------------------------------------------------------
size_t HCDCodecBMPRLE4::CompressSubset(const void* pi_pInData,
                                       size_t      pi_InDataSize,
                                       void*       po_pOutBuffer,
                                       size_t      po_OutBufferSize)
    {
    // This codec is supported in read only so we do not need to write
    // the compress methode.

    HASSERT (false);
    return 0;
    }

//-----------------------------------------------------------------------------
// public
// DecompressSubset
//-----------------------------------------------------------------------------
size_t HCDCodecBMPRLE4::DecompressSubset(const void* pi_pInData,
                                         size_t      pi_InDataSize,
                                         void*       po_pOutBuffer,
                                         size_t      pi_OutBufferSize)
    {
    // Is it the first packet?
    if(GetSubsetPosY() == 0)
        {
        SetCurrentState(STATE_DECOMPRESS);
        }

    int32_t Count;
    size_t BytesCount;
    Byte XDisplacement    = 0;
    Byte YDisplacement    = 0;
    bool  EndOfLineReached = false;
    Byte* pSrc  = (Byte*)pi_pInData;
    Byte* pDest = (Byte*)po_pOutBuffer;

    bool  NeedToBePack = false;
    Byte ByteToBePack = 0x00;

    size_t ExactBytePerLine = ((GetSubsetWidth() * GetBitsPerPixel() + 4) / 8);

    BytesCount = pi_OutBufferSize;

    // We begin to write at the begining of the last line !!!
    pDest += (pi_OutBufferSize - ExactBytePerLine);

    while(BytesCount)
        {
        if(*pSrc != 0)
            {
            // We are in encode mode and (*pSrc) contains
            // the number of pixels to be drawn using the color indexe
            // in the next byte
            Count = (*pSrc);
            pSrc++;

            if(NeedToBePack)
                {
                memset(pDest, (ByteToBePack | ((*(pSrc)) >> 4)), 1);
                pDest++;
                Count--;
                BytesCount--;

                if ((Count/2) > 0)
                    memset(pDest, (((*(pSrc)) >> 4) | ((*(pSrc)) << 4)), (Count/2));

                // We have 4 bits to be pack with the next 4 bits.
                if ((Count % 2) != 0)
                    {
                    ByteToBePack = CONVERT_TO_BYTE((*(pSrc)) << 4);
                    NeedToBePack = true;
                    }
                else
                    NeedToBePack = false;
                }
            else
                {
                memset(pDest, *(pSrc), (Count/2));

                // We have 4 bits to be pack with the next 4 bits.
                if ((Count % 2) != 0)
                    {
                    ByteToBePack = (((*(pSrc)) >> 4) << 4);
                    NeedToBePack = true;
                    }
                else
                    NeedToBePack = false;
                }

            pSrc++;
            pDest += (Count/2);
            BytesCount -= (Count/2);
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

                unsigned short PadingByte = (unsigned short)((2 - ((Count/2) % 2)) % 2);

                if(NeedToBePack)
                    {
                    memset(pDest, (ByteToBePack | ((*(pSrc)) >> 4)), 1);
                    pDest++;
                    Count--;
                    BytesCount--;

                    while(Count > 1)
                        {
                        memset(pDest, (((*(pSrc)) << 4) | ((*(pSrc+1)) >> 4)), 1);
                        pSrc++;
                        pDest++;
                        BytesCount--;

                        Count -=2 ;
                        }
                    pSrc++;

                    // We have 4 bits to be pack with the next 4 bits.
                    if (Count != 0)
                        {
                        ByteToBePack = (((*(pSrc)) >> 4) << 4);
                        NeedToBePack = true;
                        }
                    else
                        NeedToBePack = false;

                    pSrc += PadingByte;
                    }
                else
                    {
                    memcpy(pDest, pSrc, (Count/2));

                    pSrc += (Count/2);

                    // We have 4 bits to be pack with the next 4 bits.
                    if ((Count % 2) != 0)
                        {
                        ByteToBePack = (((*(pSrc)) >> 4) << 4);
                        NeedToBePack = true;
                        }
                    else
                        NeedToBePack = false;

                    pSrc += PadingByte;
                    pDest += (Count/2);
                    BytesCount -= (Count/2);
                    EndOfLineReached = false;
                    }
                }
            else if(*pSrc == 0x02)
                {
                // We know that we have an delta escape code of the encode mode.
                // memset(pDest, 0x00, BytesCount);

                size_t OffsetToEndOfLine;
                size_t TotalByteOfTheDisplacement;

                XDisplacement = *(++pSrc);
                YDisplacement = *(++pSrc);

                TotalByteOfTheDisplacement = (YDisplacement * ExactBytePerLine) + (XDisplacement/2);

                if(NeedToBePack)
                    {
                    memset(pDest, ByteToBePack, 1);

                    pDest++;
                    BytesCount--;
                    TotalByteOfTheDisplacement--;
                    }

                OffsetToEndOfLine = ((BytesCount-1) % ExactBytePerLine + 1);

                BytesCount -= TotalByteOfTheDisplacement;

                if (TotalByteOfTheDisplacement <= OffsetToEndOfLine)
                    {
                    memset(pDest, 0x00, TotalByteOfTheDisplacement);
                    pDest += TotalByteOfTheDisplacement;
                    }
                else
                    {
                    // Fill the current line first.
                    memset(pDest, 0x00, OffsetToEndOfLine);
                    TotalByteOfTheDisplacement -= OffsetToEndOfLine;
                    pDest += OffsetToEndOfLine;

                    // Fill all complete line.
                    while(TotalByteOfTheDisplacement >= ExactBytePerLine)
                        {
                        pDest -= (2*ExactBytePerLine);
                        memset(pDest, 0x00, ExactBytePerLine);
                        TotalByteOfTheDisplacement -= ExactBytePerLine;
                        pDest += ExactBytePerLine;

                        }

                    // Fill the last line.
                    if(TotalByteOfTheDisplacement != 0)
                        {
                        pDest -= (2*ExactBytePerLine);
                        memset(pDest, 0x00, TotalByteOfTheDisplacement);
                        pDest += TotalByteOfTheDisplacement;
                        }
                    }
                // We have 4 bits to be pack with the next 4 bits.
                if ((XDisplacement % 2) != 0)
                    {
                    ByteToBePack = 0x00;
                    NeedToBePack = true;
                    }
                else
                    NeedToBePack = false;

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
                    size_t NumberOfBytesToEndOfLine = BytesCount % ExactBytePerLine;

                    if(NumberOfBytesToEndOfLine != 0)
                        {
                        // Memset the destination buffer to zero because some
                        // application (Photoshop 5.5) do not encode the byte
                        // set to zero at the end of the line.
                        memset(pDest, 0x00, NumberOfBytesToEndOfLine);

                        pDest += NumberOfBytesToEndOfLine;
                        BytesCount -= NumberOfBytesToEndOfLine;
                        }

                    pDest -= (2*ExactBytePerLine);
                    EndOfLineReached = true;
                    }
                else if(*pSrc == 0x01)
                    {
                    // We are at the end of the bitmap.
                    if (BytesCount != 0)
                        {
                        // Fill the rest of the data to zero.
                        pDest = (Byte*)po_pOutBuffer;

                        memset(pDest, 0x00, BytesCount);

                        BytesCount = 0;
                        EndOfLineReached = true;
                        }
                    }

                pSrc++;
                }
            }

        if (!EndOfLineReached && (BytesCount % ExactBytePerLine) == 0)
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
                pDest -= (2*ExactBytePerLine);

                EndOfLineReached = true;
                }
            }
        }

    SetSubsetPosY(GetSubsetPosY() + GetSubsetHeight());

    size_t OutDataSize = (ExactBytePerLine * GetSubsetHeight());

    if(GetSubsetPosY() == GetHeight())
        Reset();

    return(OutDataSize);
    }

//-----------------------------------------------------------------------------
// public
// HasLineAccess
//-----------------------------------------------------------------------------
bool HCDCodecBMPRLE4::HasLineAccess() const
    {
    return false;
    }

//-----------------------------------------------------------------------------
// public
// IsBitsPerPixelSupported
//-----------------------------------------------------------------------------
bool HCDCodecBMPRLE4::IsBitsPerPixelSupported(size_t pi_Bits) const
    {
    if(pi_Bits == 4)
        return true;
    else
        return false;
    }


//-----------------------------------------------------------------------------
// public
// GetSubsetMaxCompressedSize
//-----------------------------------------------------------------------------
size_t HCDCodecBMPRLE4::GetSubsetMaxCompressedSize() const
    {
    return (GetSubsetWidth() * 3 * GetSubsetHeight());
    }

