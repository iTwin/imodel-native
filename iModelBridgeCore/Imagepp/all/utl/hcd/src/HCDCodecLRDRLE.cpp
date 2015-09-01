//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hcd/src/HCDCodecLRDRLE.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for class HCDCodecLRDRLE
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HCDCodecLRDRLE.h>

#define HCD_CODEC_NAME L"LRDRLE1bits"

static const unsigned char s_StartBitMask [8]        = {0xFF, 0x7F, 0x3F, 0x1F, 0x0F, 0x07, 0x03, 0x01};
static const unsigned char s_EndingBitMask[8]        = {0x80, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC, 0xFE, 0xFF};
static const unsigned char s_PixelPositionBitMask[8] = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};

//-----------------------------------------------------------------------------
// public
// Default constructor
//-----------------------------------------------------------------------------
HCDCodecLRDRLE::HCDCodecLRDRLE()
    : HCDCodecImage(HCD_CODEC_NAME)
    {
    m_OneLineMode = false;
    }

//-----------------------------------------------------------------------------
// public
// Default constructor
//-----------------------------------------------------------------------------
HCDCodecLRDRLE::HCDCodecLRDRLE(size_t pi_Width,
                               size_t pi_Height)
    : HCDCodecImage(HCD_CODEC_NAME,
                    pi_Width,
                    pi_Height,
                    1)
    {
    m_OneLineMode = false;
    }

//-----------------------------------------------------------------------------
// public
// Copy constructor
//-----------------------------------------------------------------------------
HCDCodecLRDRLE::HCDCodecLRDRLE(const HCDCodecLRDRLE& pi_rObj)
    : HCDCodecImage(pi_rObj)
    {
    m_OneLineMode = pi_rObj.m_OneLineMode;
    }

//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HCDCodecLRDRLE::~HCDCodecLRDRLE()
    {
    }

//-----------------------------------------------------------------------------
// public
// Clone
//-----------------------------------------------------------------------------
HCDCodec* HCDCodecLRDRLE::Clone() const
    {
    return new HCDCodecLRDRLE(*this);
    }

//-----------------------------------------------------------------------------
// public
// CompressSubset
//-----------------------------------------------------------------------------
size_t HCDCodecLRDRLE::CompressSubset(const void* pi_pInData,
                                      size_t      pi_InDataSize,
                                      void*       po_pOutBuffer,
                                      size_t      po_OutBufferSize)
    {
    HPRECONDITION(GetSubsetHeight() == 1);

    // is it the first packet?
    if(GetSubsetPosY() == 0)
        {
        SetCurrentState(STATE_COMPRESS);
        }

    Byte*  pSrc  = (Byte* )pi_pInData;
    unsigned short* pDest = (unsigned short*)po_pOutBuffer;

    unsigned short BytePosition     = 0;
    unsigned short StartRunPosition = 0;
    unsigned short StopRunPosition  = 0;

    uint32_t LineBytesCount = (uint32_t)((GetSubsetWidth() + 7.0) / 8.0);

    // loop until we compress all the bytes
    while(BytePosition < LineBytesCount)
        {
        HASSERT(StartRunPosition <= GetSubsetWidth());
        HASSERT(BytePosition == (uint32_t)(StartRunPosition / 8.0) );

        // First, find the run start position.
        if (!(StartRunPosition % 8))
            {
            // To be fast, skip all byte full of background...
            while(BytePosition < LineBytesCount && !pSrc[BytePosition])
                {
                BytePosition++;
                StartRunPosition += 8;
                }
            StartRunPosition = (unsigned short)(MIN(GetSubsetWidth(), StartRunPosition));
            }
        HASSERT(StartRunPosition <= GetSubsetWidth());

        // If the line is not ended...
        if (BytePosition < LineBytesCount)
            {
            // Find the first pixel position from the StartRunPosition.
            int BitPosition = StartRunPosition %8;
            int PixelState = 0;

            while (!PixelState && (BytePosition < LineBytesCount) )
                {
                PixelState = pSrc[BytePosition] & s_PixelPositionBitMask[BitPosition++];

                StartRunPosition++;

                // At the end of the byte, try the next one.
                if (BitPosition == 8)
                    {
                    BitPosition = 0;
                    BytePosition++;
                    }
                }

            HASSERT(BytePosition == (uint32_t)(StartRunPosition / 8.0) );

            if (BytePosition < LineBytesCount)
                {
                StopRunPosition   = StartRunPosition;

                // Second, find the end position of the run lentth.
                BitPosition = StopRunPosition % 8;
                HASSERT(PixelState);
                while (PixelState && (BytePosition < LineBytesCount))
                    {
                    PixelState = pSrc[BytePosition] & s_PixelPositionBitMask[BitPosition++];
                    if (PixelState)
                        StopRunPosition++;

                    // could be optimized if pSrc[BytePosition] == 0xff, then skip all pSrc[BytePosition] == 0xff ..

                    // At the end of the byte, try the next one.
                    if (BitPosition == 8)
                        {
                        BitPosition = 0;
                        BytePosition++;
                        }
                    }

                StopRunPosition = (unsigned short)(MIN(GetSubsetWidth(), StopRunPosition));

                HASSERT(StopRunPosition >= StartRunPosition);
                HASSERT(StartRunPosition <= GetSubsetWidth());
                HASSERT(StopRunPosition  <= GetSubsetWidth());

                // Third step : create the run...
                *pDest = StartRunPosition;
                pDest++;

                // Look where ut stop.
                *pDest = StopRunPosition;
                pDest++;

                // To properly finish the process get prepared for the next run.
                StartRunPosition = StopRunPosition + 1;

                if (StartRunPosition >  GetSubsetWidth())
                    {
                    // Break the loop!
                    BytePosition = (unsigned short)LineBytesCount;
                    }
                }
            }
        else if (BytePosition >= LineBytesCount && !StopRunPosition)
            {
            // Just an empty line.
            StartRunPosition = 0;
            }
        }

    *pDest = 0xFFF0;                         // Mark the end of the line.
    pDest++;

    if (GetSubsetPosY() < GetHeight())
        *pDest = 0xFFF0;                     // end of the line.
    else
        *pDest = 0xFFF2;                     // end of the raster.
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
size_t HCDCodecLRDRLE::DecompressSubset(const void* pi_pInData,
                                        size_t      pi_InDataSize,
                                        void*       po_pOutBuffer,
                                        size_t      pi_OutBufferSize)
    {
    HPRECONDITION(GetSubsetHeight() == 1);
//    HDEBUGCODE( Int32 SubsetWidthBytesCount = (UInt32)(((double)GetSubsetWidth() + 7.0) / 8.0); );

    // Is it the first packet?
    if(GetSubsetPosY() == 0)
        {
        SetCurrentState(STATE_DECOMPRESS);
        }

    unsigned short* pSrc  = (unsigned short*)pi_pInData;
    Byte*  pDest = (Byte*)po_pOutBuffer;

    unsigned short RunStartPosition;
    unsigned short RunStopPosition;

    uint32_t   FullByteCount;
    uint32_t   StartingBit = 0;
    uint32_t   EndingBit   = 8;
    uint32_t   RunPositionIntoDest;

    size_t   OutDataSize = ((GetSubsetWidth() + GetLinePaddingBits()) / 8) * GetSubsetHeight();

    // Clean the buffer.
    memset(pDest, 0, OutDataSize);

    do
        {
        // Start position in bits of the run into the line buffer.
        RunStartPosition = *((unsigned short*)pSrc);
        pSrc++;

        // Stop position in bits of the run into the line buffer.
        RunStopPosition = *((unsigned short*)pSrc);
        pSrc++;

        // Fill all bits from RunStartPosition to RunStartPosition.
        if (RunStartPosition && (RunStartPosition < 0xFFF0))
            {
            if (RunStopPosition < RunStartPosition)
                {
                OutDataSize = 0;
                goto WRAPUP;
                }

            // The run are 1 based index (first column is 1 not zero),
            // buffer access is zero based, so..
            RunStopPosition--;
            RunStartPosition--;

            StartingBit = RunStartPosition % 8;
            EndingBit   = RunStopPosition  % 8;

            RunPositionIntoDest = uint32_t( (double)RunStartPosition / 8.0);
            FullByteCount       = uint32_t(((double)(RunStopPosition - (RunStartPosition - StartingBit))) / 8.0);

            // Be sure to stay inbound.
            if ((FullByteCount + RunPositionIntoDest) > OutDataSize)
                {
                OutDataSize = 0;
                goto WRAPUP;
                }

            // Verrify if we have less than a byte to fill.
            if ((RunStopPosition - RunStartPosition) < 8)
                {
                //  Within the same byte.
                if (RunPositionIntoDest == uint32_t((double)RunStopPosition / 8.0))
                    {
                    HASSERT(StartingBit <= EndingBit);
                    pDest[RunPositionIntoDest] |= (s_EndingBitMask[EndingBit] & s_StartBitMask[StartingBit]);
                    }
                else
                    {
                    HASSERT((RunPositionIntoDest + 1) == uint32_t((double)RunStopPosition / 8.0));
                    pDest[RunPositionIntoDest++ ] |= s_StartBitMask[StartingBit];

                    // Be sure to stay inbound.
                    HASSERT((FullByteCount + RunPositionIntoDest) <= OutDataSize);
                    pDest[RunPositionIntoDest]    |= s_EndingBitMask[EndingBit];
                    }
                }
            else
                {
                HASSERT(FullByteCount > 0);

                // Fill the first byte.
                pDest[RunPositionIntoDest] |= s_StartBitMask[StartingBit];
                RunPositionIntoDest++;

                if (FullByteCount > 1)
                    {
                    HASSERT((RunPositionIntoDest + (FullByteCount - 1)) < OutDataSize);

                    // Set all contigus full byte forground pixels minus the last one.
                    memset(&pDest[RunPositionIntoDest] , 0xFF, FullByteCount - 1);
                    RunPositionIntoDest += (FullByteCount - 1);
                    }
                HASSERT(RunPositionIntoDest < OutDataSize);

                // Set the forground pixel into the very last byte of the run.
                pDest[RunPositionIntoDest] |= s_EndingBitMask[EndingBit];
                }
            }
        }
    while(RunStartPosition < 0xFFF0);

    SetCompressedImageIndex(GetCompressedImageIndex() + (((Byte*)pSrc) - ((Byte*)pi_pInData)));
    SetSubsetPosY(GetSubsetPosY() + GetSubsetHeight());

    if(GetSubsetPosY() == GetHeight())
        Reset();

WRAPUP:
    return(OutDataSize);
    }

//-----------------------------------------------------------------------------
// public
// HasLineAccess
//-----------------------------------------------------------------------------
bool HCDCodecLRDRLE::HasLineAccess() const
    {
    return true;
    }

//-----------------------------------------------------------------------------
// public
// SetOneLineMode
//-----------------------------------------------------------------------------
void HCDCodecLRDRLE::SetOneLineMode(bool pi_Enable)
    {
    m_OneLineMode = pi_Enable;
    }

//-----------------------------------------------------------------------------
// public
// IsBitsPerPixelSupported
//-----------------------------------------------------------------------------
bool HCDCodecLRDRLE::IsBitsPerPixelSupported(size_t pi_Bits) const
    {
    if(pi_Bits == 1)
        return true;
    else
        return false;
    }


//-----------------------------------------------------------------------------
// public
// GetSubsetMaxCompressedSize
//-----------------------------------------------------------------------------
size_t HCDCodecLRDRLE::GetSubsetMaxCompressedSize() const
    {
    return ((GetSubsetWidth() + 4) * 2 * GetSubsetHeight());
    }

/*
// If there is no starting run into the current byte..
BytePosition++;

// Rapidly skip all byte full of foreground...
while(pSrc[BytePosition] == 0xFF && BytePosition < LineBytesCount)
{
    StopRunPosition += 8;
    BytePosition++;
}

if (StopRunPosition < GetSubsetWidth())
{
    // Find the next background bits to see where the run end whitin the current byte
    if ((pSrc[BytePosition] & s_StartBitMask[1]) == s_StartBitMask [1])
        StopRunPosition += 7;
    else if ((pSrc[BytePosition] & s_StartBitMask[2]) == s_StartBitMask [2])
        StopRunPosition += 6;
    else if ((pSrc[BytePosition] & s_StartBitMask[3]) == s_StartBitMask [3])
        StopRunPosition += 5;
    else if ((pSrc[BytePosition] & s_StartBitMask[4]) == s_StartBitMask [4])
        StopRunPosition += 4;
    else if ((pSrc[BytePosition] & s_StartBitMask[5]) == s_StartBitMask [5])
        StopRunPosition += 3;
    else if ((pSrc[BytePosition] & s_StartBitMask[6]) == s_StartBitMask [6])
        StopRunPosition += 2;
    else if ((pSrc[BytePosition] & s_StartBitMask[7]) == s_StartBitMask [7])
        StopRunPosition += 1;
}*/