//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hcd/src/HCDCodecRLE8.cpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HCDCodecRLE8
//-----------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>
#include <Imagepp/all/h/HCDCodecRLE8.h>

static const Byte s_BitMask[8]    = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };

#define HCD_CODEC_NAME L"RLE8bits"

//-----------------------------------------------------------------------------
// public
// Default constructor
//-----------------------------------------------------------------------------
HCDCodecRLE8::HCDCodecRLE8()
    : HCDCodecImage(HCD_CODEC_NAME)
    {
    m_LineHeader = false;
    m_OneLineMode = false;
    }

//-----------------------------------------------------------------------------
// public
// Default constructor
//-----------------------------------------------------------------------------
HCDCodecRLE8::HCDCodecRLE8(size_t pi_Width,
                           size_t pi_Height)
    : HCDCodecImage(HCD_CODEC_NAME,
                    pi_Width,
                    pi_Height,
                    8)
    {
    m_LineHeader = false;
    m_OneLineMode = false;
    }

//-----------------------------------------------------------------------------
// public
// Copy constructor
//-----------------------------------------------------------------------------
HCDCodecRLE8::HCDCodecRLE8(const HCDCodecRLE8& pi_rObj)
    : HCDCodecImage(pi_rObj)
    {
    m_LineHeader = pi_rObj.m_LineHeader;
    m_OneLineMode = pi_rObj.m_OneLineMode;
    }

//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HCDCodecRLE8::~HCDCodecRLE8()
    {
    }

//-----------------------------------------------------------------------------
// public
// Clone
//-----------------------------------------------------------------------------
HCDCodec* HCDCodecRLE8::Clone() const
    {
    return new HCDCodecRLE8(*this);
    }

//-----------------------------------------------------------------------------
// public
// CompressSubset
//-----------------------------------------------------------------------------
size_t HCDCodecRLE8::CompressSubset(const void* pi_pInData,
                                    size_t pi_InDataSize,
                                    void* po_pOutBuffer,
                                    size_t po_OutBufferSize)
    {
    // HLXXX Note: The compression algorithm is not optimal. The compression has been
    // done very simply. However, there are some combinations of run lenght and
    // non-run lenghts that could be smaller in length.. The algo is more complex however....

    // is it the first packet?
    if(GetSubsetPosY() == 0)
        {
        SetCurrentState(STATE_COMPRESS);
        }

    Byte* pSrc = (Byte*)pi_pInData;
    int8_t* pDest = (int8_t*)po_pOutBuffer;

    int8_t* pLen;

    size_t LinesCount = GetSubsetHeight();

    size_t LineBytesCount;
    Byte Value;
    size_t Len;

    while(LinesCount != 0)
        {
        LineBytesCount = GetSubsetWidth();

        // loop until we compress all the bytes
        while(LineBytesCount != 0)
            {
            // get the value of the byte
            Value = *pSrc;
            Len = 1;

            pSrc++;
            LineBytesCount--;

            // compare the value with the following bytes and test if there is a repetitive pattern
            while(Len < 128 && LineBytesCount != 0 && Value == *pSrc)
                {
                pSrc++;
                LineBytesCount--;
                Len++;
                }

            // if the len is longer than 1, store the repetitiv pattern
            if(Len > 1)
                {
                *pDest = (int8_t)(-1 * Len);
                pDest++;
                *((Byte*)pDest) = Value;
                pDest++;
                }
            else
                {
                // otherwise, test for the longer non-repetitive pattern
                pLen = pDest;

                pDest++;
                *pDest = Value;
                pDest++;

                // test for the longer non repetitive string
                while(Len < 127 && ((LineBytesCount == 1) || (LineBytesCount > 1 && *pSrc != *(pSrc + 1))))
                    {
                    *pDest = *pSrc;
                    pDest++;
                    Len++;
                    pSrc++;
                    LineBytesCount--;
                    }

                *pLen = (int8_t)Len;
                }
            }

        pSrc += (GetLinePaddingBits() >> 3);

        LinesCount--;
        }

    SetSubsetPosY(GetSubsetPosY() + GetSubsetHeight());

    if(GetSubsetPosY() == GetHeight())
        Reset();

    return(((Byte*)pDest) - ((Byte*)po_pOutBuffer));
    }

//-----------------------------------------------------------------------------
// public
// DecompressSubset
//-----------------------------------------------------------------------------
size_t HCDCodecRLE8::DecompressSubset(const void* pi_pInData,
                                      size_t pi_InDataSize,
                                      void* po_pOutBuffer,
                                      size_t pi_OutBufferSize)
    {
    // is it the first packet?
    if(GetSubsetPosY() == 0)
        {
        SetCurrentState(STATE_DECOMPRESS);
        }

    size_t BytesCount;
    int8_t* pSrc = (int8_t*)pi_pInData;
    Byte* pDest = (Byte*)po_pOutBuffer;

    int32_t Count;

    size_t LinesCount = GetSubsetHeight();

    while(LinesCount != 0)
        {
        BytesCount = GetSubsetWidth();

        while(BytesCount)
            {
            if(*pSrc < 0)
                {
                Count = -1 * (*pSrc);
                pSrc++;
                memset(pDest, *((Byte*)pSrc), Count);

                pSrc++;
                pDest += Count;
                HPRECONDITION(Count <= (int32_t)BytesCount);
                BytesCount -= Count;
                }
            else if(*pSrc > 0)
                {
                Count = *pSrc;
                pSrc++;
                memcpy(pDest, pSrc, Count);

                pSrc += Count;
                pDest += Count;
                HPRECONDITION(Count <= (int32_t)BytesCount);
                BytesCount -= Count;
                }
            else
                {
                pSrc++;
                }
            }

        pDest += (GetLinePaddingBits() >> 3);
        LinesCount--;
        }

    SetCompressedImageIndex(GetCompressedImageIndex() +
                            (((Byte*)pSrc) - ((Byte*)pi_pInData)));

    SetSubsetPosY(GetSubsetPosY() + GetSubsetHeight());

    size_t OutDataSize = ((GetSubsetWidth() + GetLinePaddingBits()) / 8) * GetSubsetHeight();

    if(GetSubsetPosY() == GetHeight())
        Reset();

    return(OutDataSize);
    }

//-----------------------------------------------------------------------------
// public
// HasLineAccess
//-----------------------------------------------------------------------------
bool HCDCodecRLE8::HasLineAccess() const
    {
    return true;
    }

//-----------------------------------------------------------------------------
// public
// SetLineHeader
//-----------------------------------------------------------------------------
void HCDCodecRLE8::SetLineHeader(bool pi_Enable)
    {
    m_LineHeader = pi_Enable;
    }


//-----------------------------------------------------------------------------
// public
// SetOneLineMode
//-----------------------------------------------------------------------------
void HCDCodecRLE8::SetOneLineMode(bool pi_Enable)
    {
    m_OneLineMode = pi_Enable;
    }

//-----------------------------------------------------------------------------
// public
// IsBitsPerPixelSupported
//-----------------------------------------------------------------------------
bool HCDCodecRLE8::IsBitsPerPixelSupported(size_t pi_Bits) const
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
size_t HCDCodecRLE8::GetSubsetMaxCompressedSize() const
    {
    return (GetSubsetWidth() * 5 * GetSubsetHeight());
    }

