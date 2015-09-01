//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hcd/src/HCDCodecCRL8.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HCDCodecCRL8
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HCDCodecCRL8.h>


#define HCD_CODEC_NAME L"CRL8bits"

//-----------------------------------------------------------------------------
// public
// Default constructor
//-----------------------------------------------------------------------------
HCDCodecCRL8::HCDCodecCRL8()
    : HCDCodecImage(HCD_CODEC_NAME)
    {
    m_LineHeader  = false;
    m_OneLineMode = false;
    }

//-----------------------------------------------------------------------------
// public
// Default constructor
//-----------------------------------------------------------------------------
HCDCodecCRL8::HCDCodecCRL8(size_t pi_Width,
                           size_t pi_Height)
    : HCDCodecImage(HCD_CODEC_NAME,
                    pi_Width,
                    pi_Height,
                    8)
    {
    m_LineHeader  = false;
    m_OneLineMode = false;
    }

//-----------------------------------------------------------------------------
// public
// Copy constructor
//-----------------------------------------------------------------------------
HCDCodecCRL8::HCDCodecCRL8(const HCDCodecCRL8& pi_rObj)
    : HCDCodecImage(pi_rObj)
    {
    m_LineHeader  = pi_rObj.m_LineHeader;
    m_OneLineMode = pi_rObj.m_OneLineMode;
    }

//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HCDCodecCRL8::~HCDCodecCRL8()
    {
    }

//-----------------------------------------------------------------------------
// public
// Clone
//-----------------------------------------------------------------------------
HCDCodec* HCDCodecCRL8::Clone() const
    {
    return new HCDCodecCRL8(*this);
    }

//-----------------------------------------------------------------------------
// public
// CompressSubset
//-----------------------------------------------------------------------------
size_t HCDCodecCRL8::CompressSubset(const void* pi_pInData,
                                    size_t      pi_InDataSize,
                                    void*       po_pOutBuffer,
                                    size_t      po_OutBufferSize)
    {
    // is it the first packet?
    if(GetSubsetPosY() == 0)
        {
        SetCurrentState(STATE_COMPRESS);
        }

    Byte*  pSrc  = (Byte* )pi_pInData;
    unsigned short* pDest = (unsigned short*)po_pOutBuffer;

    unsigned short* pCurLine = NULL;

    size_t   LinesCount = GetSubsetHeight();

    size_t   LineBytesCount;
    Byte   ColorIndex;
    unsigned short Count;

    while(LinesCount != 0)
        {
        LineBytesCount = GetSubsetWidth();

        if(m_LineHeader)
            {
            // Remember the scan line header position.
            pCurLine = pDest;

            // skeep the header
            pDest += 4;
            }

        // loop until we compress all the bytes
        while(LineBytesCount != 0)
            {
            // get the value of the byte
            ColorIndex = *pSrc;
            Count = 1;

            pSrc++;
            LineBytesCount--;

            // compare the value with the following bytes and test if there is a repetitive pattern
            while(Count < USHRT_MAX && LineBytesCount != 0 && ColorIndex == *pSrc)
                {
                pSrc++;
                LineBytesCount--;
                Count++;
                }

            *pDest = (unsigned short)ColorIndex;
            pDest++;

            // if the len is longer than 1, store the repetitiv pattern
            *pDest = Count;
            pDest++;
            }

        // store the scanline header if necessary
        if(m_LineHeader)
            {
            // scanline header
            *pCurLine = 0x5901;
            pCurLine++;

            // words to follow
            *pCurLine = (unsigned short)(pDest - (pCurLine + 1));
            pCurLine++;

            // line number
            *pCurLine = (unsigned short)(GetSubsetPosY() + 1);
            pCurLine++;

            // pixel offset
            *pCurLine = 0;
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
size_t HCDCodecCRL8::DecompressSubset(const void* pi_pInData,
                                      size_t      pi_InDataSize,
                                      void*       po_pOutBuffer,
                                      size_t      pi_OutBufferSize)
    {
    // Is it the first packet?
    if(GetSubsetPosY() == 0)
        {
        SetCurrentState(STATE_DECOMPRESS);
        }

    size_t   BytesCount;

    unsigned short* pSrc  = (unsigned short*)pi_pInData;
    Byte*  pDest = (Byte*)po_pOutBuffer;

    unsigned short Count;
    Byte   ColorIndex;

    size_t LinesCount = GetSubsetHeight();

    // Problem found : a scannable flag seem to be there (in the raster data )
    // and it's not handle, so.. 4 word are used to describe it.

    while(LinesCount != 0)
        {
        BytesCount = GetSubsetWidth();

        if (m_LineHeader)
            {
            pSrc += 4;

            // Inspect the line scannable flag.
//             HDEBUGCODE( UShort SCN_1 = pSrc[0]; );  // scanline header
//             HDEBUGCODE( UShort SCN_2 = pSrc[1]; );  // words to follow
//             HDEBUGCODE( UShort SCN_3 = pSrc[2]; );  // line number
//             HDEBUGCODE( UShort SCN_4 = pSrc[3]; );  // pixel offset
            }

        while(BytesCount)
            {
            ColorIndex = (Byte)(*pSrc);
            pSrc++;

            Count = *((unsigned short*)pSrc);
            pSrc++;

            HASSERT(Count > 0);
            memset(pDest, ColorIndex, Count);

            HASSERT(Count <= (int32_t)BytesCount);
            BytesCount -= Count;
            pDest      += Count;

            HASSERT(BytesCount >= 0);
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
bool HCDCodecCRL8::HasLineAccess() const
    {
    return true;
    }

//-----------------------------------------------------------------------------
// public
// SetLineHeader
//-----------------------------------------------------------------------------
void HCDCodecCRL8::SetLineHeader(bool pi_Enable)
    {
    m_LineHeader = pi_Enable;
    }


//-----------------------------------------------------------------------------
// public
// SetOneLineMode
//-----------------------------------------------------------------------------
void HCDCodecCRL8::SetOneLineMode(bool pi_Enable)
    {
    m_OneLineMode = pi_Enable;
    }

//-----------------------------------------------------------------------------
// public
// IsBitsPerPixelSupported
//-----------------------------------------------------------------------------
bool HCDCodecCRL8::IsBitsPerPixelSupported(size_t pi_Bits) const
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
size_t HCDCodecCRL8::GetSubsetMaxCompressedSize() const
    {
    return (GetSubsetWidth() * 5 * GetSubsetHeight());
    }

