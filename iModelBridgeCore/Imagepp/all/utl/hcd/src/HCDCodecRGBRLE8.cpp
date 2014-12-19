//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hcd/src/HCDCodecRGBRLE8.cpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for class HCDCodecRGBRLE8
//-----------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>
#include <Imagepp/all/h/HCDCodecRGBRLE8.h>

#define HCD_CODEC_NAME L"RGB RLE 8 bits"

//-----------------------------------------------------------------------------
// public
// Default constructor
//-----------------------------------------------------------------------------
HCDCodecRGBRLE8::HCDCodecRGBRLE8()
    : HCDCodecImage(HCD_CODEC_NAME)
    {
    m_LineHeader = false;
    m_OneLineMode = false;
    }

//-----------------------------------------------------------------------------
// public
// Default constructor
//-----------------------------------------------------------------------------
HCDCodecRGBRLE8::HCDCodecRGBRLE8(size_t pi_Width,
                                 size_t pi_Height)
    : HCDCodecImage(HCD_CODEC_NAME,
                    pi_Width,
                    pi_Height,
                    24)
    {
    m_LineHeader = false;
    m_OneLineMode = false;
    }

//-----------------------------------------------------------------------------
// public
// Copy constructor
//-----------------------------------------------------------------------------
HCDCodecRGBRLE8::HCDCodecRGBRLE8(const HCDCodecRGBRLE8& pi_rObj)
    : HCDCodecImage(pi_rObj)
    {
    m_LineHeader = pi_rObj.m_LineHeader;
    m_OneLineMode = pi_rObj.m_OneLineMode;
    }

//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HCDCodecRGBRLE8::~HCDCodecRGBRLE8()
    {
    }

//-----------------------------------------------------------------------------
// public
// Clone
//-----------------------------------------------------------------------------
HCDCodec* HCDCodecRGBRLE8::Clone() const
    {
    return new HCDCodecRGBRLE8(*this);
    }

//-----------------------------------------------------------------------------
// public
// CompressSubset
//-----------------------------------------------------------------------------
size_t HCDCodecRGBRLE8::CompressSubset(const void* pi_pInData,
                                       size_t      pi_InDataSize,
                                       void*       po_pOutBuffer,
                                       size_t      po_OutBufferSize)
    {
    // HLXXX Note: The compression algorithm is not optimal. The compression has been
    // done very simply. However, there are some combinations of run lenght and
    // non-run lenghts that could be smaller in length.. The algo is more complex however....

    // is it the first packet?
    if(GetSubsetPosY() == 0)
        {
        SetCurrentState(STATE_COMPRESS);
        }

    Byte* pSrc  = (Byte*)pi_pInData;
    int8_t* pDest = (int8_t*)po_pOutBuffer;

    Byte* pColorPlan      = new Byte[GetSubsetWidth()];
    Byte* pColorPlanBegin = pColorPlan;

    unsigned short ColorPlanCount = 0;

    int8_t* pLen;

    size_t LinesCount = GetSubsetHeight();

    size_t LineBytesCount;
    Byte Value;
    int32_t Len;

    while(LinesCount != 0)
        {
        LineBytesCount = GetSubsetWidth();

        // Decompress every color plan (R,G and B)
        while(ColorPlanCount != 3)
            {
            // Construct the Color Plan buffer
            for(uint32_t i = 0; i < GetSubsetWidth(); ++i)
                pColorPlanBegin[i] = pSrc[(i*3)+ColorPlanCount];

            // Move the Color Plan pointer to the beginning of the buffer
            // for the next color plan
            pColorPlan = pColorPlanBegin;

            // loop until we compress all the bytes
            while(LineBytesCount != 0)
                {
                // get the value of the byte
                Value = *pColorPlan;
                Len = 1;

                pColorPlan++;
                LineBytesCount--;

                // compare the value with the following bytes and test if there is a repetitive pattern
                while(LineBytesCount != 0 && Value == *pColorPlan && Len < 128)
                    {
                    pColorPlan++;
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
                    while(LineBytesCount != 0 &&
                          ((LineBytesCount < 2 || *pColorPlan != *(pColorPlan + 1))) &&   // We need at least two entries.
                          Len < 127)
                        {
                        *pDest = *pColorPlan;
                        pDest++;
                        Len++;
                        pColorPlan++;
                        LineBytesCount--;
                        }

                    *pLen = (int8_t)Len;
                    }
                }
            ColorPlanCount++;
            LineBytesCount = GetSubsetWidth();
            }
        ColorPlanCount = 0;
        pSrc += ((GetSubsetWidth() * 24) + GetLinePaddingBits()) / 8;

        LinesCount--;
        }
    delete[] pColorPlanBegin;

    SetSubsetPosY(GetSubsetPosY() + GetSubsetHeight());

    if(GetSubsetPosY() == GetHeight())
        Reset();

    return(((Byte*)pDest) - ((Byte*)po_pOutBuffer));
    }

//-----------------------------------------------------------------------------
// public
// DecompressSubset
//-----------------------------------------------------------------------------
size_t HCDCodecRGBRLE8::DecompressSubset(const void* pi_pInData,
                                         size_t      pi_InDataSize,
                                         void*       po_pOutBuffer,
                                         size_t      pi_OutBufferSize)
    {
    // is it the first packet?
    if(GetSubsetPosY() == 0)
        {
        SetCurrentState(STATE_DECOMPRESS);
        }

    size_t BytesCount;

    int8_t* pSrc = (int8_t*)pi_pInData;
    Byte* pDest = (Byte*)po_pOutBuffer;

    Byte* pColorPlan = new Byte[GetSubsetWidth()];
    Byte* pColorPlanBegin = pColorPlan;

    int32_t Count;
    unsigned short ColorPlanCount = 0;

    size_t LinesCount = GetSubsetHeight();

    while(LinesCount != 0)
        {
        // Decompress every color plan (R,G and B)
        while(ColorPlanCount != 3)
            {
            // Get the number of pixel by Width
            BytesCount = GetSubsetWidth();

            while(BytesCount)
                {
                if(*pSrc < 0)
                    {
                    Count = -1 * (*pSrc);
                    pSrc++;
                    memset(pColorPlan, *((Byte*)pSrc), Count);

                    pSrc++;
                    pColorPlan += Count;
                    BytesCount -= Count;
                    }
                else if(*pSrc > 0)
                    {
                    Count = *pSrc;
                    pSrc++;
                    memcpy(pColorPlan, pSrc, Count);

                    pSrc += Count;
                    pColorPlan += Count;
                    BytesCount -= Count;
                    }
                else
                    {
                    pSrc++;
                    }
                }
            // Replace the pixel in RGB order in the out buffer
            for(uint32_t i = 0; i < GetSubsetWidth(); ++i)
                pDest[(i*3)+ColorPlanCount] = pColorPlanBegin[i];

            // Move the Color Plan pointer to the beginning of the buffer
            // for the next color plan
            pColorPlan = pColorPlanBegin;

            ColorPlanCount++;
            }
        pDest += ((GetSubsetWidth() * 24) + GetLinePaddingBits()) / 8;
        ColorPlanCount = 0;
        LinesCount--;
        }

    // Delete the temporary buffer
    delete[] pColorPlanBegin;

    SetCompressedImageIndex(GetCompressedImageIndex() +
                            (((Byte*)pSrc) - ((Byte*)pi_pInData)));

    SetSubsetPosY(GetSubsetPosY() + GetSubsetHeight());

    size_t OutDataSize = (((GetSubsetWidth() * 24) + GetLinePaddingBits()) / 8) * GetSubsetHeight();

    if(GetSubsetPosY() == GetHeight())
        Reset();

    return(OutDataSize);
    }

//-----------------------------------------------------------------------------
// public
// HasLineAccess
//-----------------------------------------------------------------------------
bool HCDCodecRGBRLE8::HasLineAccess() const
    {
    return true;
    }

//-----------------------------------------------------------------------------
// public
// SetLineHeader
//-----------------------------------------------------------------------------
void HCDCodecRGBRLE8::SetLineHeader(bool pi_Enable)
    {
    m_LineHeader = pi_Enable;
    }


//-----------------------------------------------------------------------------
// public
// SetOneLineMode
//-----------------------------------------------------------------------------
void HCDCodecRGBRLE8::SetOneLineMode(bool pi_Enable)
    {
    m_OneLineMode = pi_Enable;
    }

//-----------------------------------------------------------------------------
// public
// IsBitsPerPixelSupported
//-----------------------------------------------------------------------------
bool HCDCodecRGBRLE8::IsBitsPerPixelSupported(size_t pi_Bits) const
    {
    if(pi_Bits == 24)
        return true;
    else
        return false;
    }


//-----------------------------------------------------------------------------
// public
// GetSubsetMaxCompressedSize
//-----------------------------------------------------------------------------
size_t HCDCodecRGBRLE8::GetSubsetMaxCompressedSize() const
    {
    return (GetSubsetWidth() * 5 * GetSubsetHeight());
    }

