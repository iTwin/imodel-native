//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hcd/src/HCDCodecTgaRLE.cpp $
//:>
//:>  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for class HCDCodecTGARLE
//-----------------------------------------------------------------------------

#include <ImageppInternal.h>

#include <ImagePP/all/h/HCDCodecTgaRLE.h>

#define HCD_CODEC_NAME "TGA RLE ENCODED"

//-----------------------------------------------------------------------------
// public
// Default constructor
//-----------------------------------------------------------------------------
HCDCodecTGARLE::HCDCodecTGARLE()
    : HCDCodecImage(HCD_CODEC_NAME)
    {
    m_AlphaChannelBits = 0;
    }

//-----------------------------------------------------------------------------
// public
// Default constructor
//-----------------------------------------------------------------------------
HCDCodecTGARLE::HCDCodecTGARLE(size_t pi_Width,
                               size_t pi_Height,
                               size_t pi_BitsPerPixel,
                               Byte pi_AlphaChannelBits)
    : HCDCodecImage(HCD_CODEC_NAME,
                    pi_Width,
                    pi_Height,
                    pi_BitsPerPixel),
    m_AlphaChannelBits(pi_AlphaChannelBits)
    {
    HPRECONDITION((pi_BitsPerPixel == 32 && (pi_AlphaChannelBits == 0 || pi_AlphaChannelBits == 8)) ||
                  (pi_BitsPerPixel != 32 && pi_AlphaChannelBits == 0));

    m_NumberOfBitsPerPixelInOutput = 0;
    }

//-----------------------------------------------------------------------------
// public
// Copy constructor
//-----------------------------------------------------------------------------
HCDCodecTGARLE::HCDCodecTGARLE(const HCDCodecTGARLE& pi_rObj)
    : HCDCodecImage(pi_rObj)
    {
    m_AlphaChannelBits = pi_rObj.m_AlphaChannelBits;
    }

//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HCDCodecTGARLE::~HCDCodecTGARLE()
    {
    }

//-----------------------------------------------------------------------------
// public
// Clone
//-----------------------------------------------------------------------------
HCDCodec* HCDCodecTGARLE::Clone() const
    {
    return new HCDCodecTGARLE(*this);
    }

//-----------------------------------------------------------------------------
// public
// CompressSubset
//-----------------------------------------------------------------------------
size_t HCDCodecTGARLE::CompressSubset(const void* pi_pInData,
                                      size_t pi_InDataSize,
                                      void* po_pOutBuffer,
                                      size_t po_OutBufferSize)
    {
    HPRECONDITION (m_NumberOfBitsPerPixelInOutput != 0);

    uint32_t          Len;
    Byte           BytesPerPixel   = (m_NumberOfBitsPerPixelInOutput + 1) / 8;
    size_t           NbPixel         = pi_InDataSize * 8 / GetBitsPerPixel();
    Byte*          pCode;
    Byte*          pDest           = (Byte*) po_pOutBuffer;
    std::unique_ptr<Byte[]> pTempBuffer; 

    Byte const* pPixel = (Byte const*) pi_pInData;
    
    po_OutBufferSize = 0;

    if (GetSubsetPosY() == 0)
        SetCurrentState (STATE_DECOMPRESS);

    // If BytesPerPixel is equal to 2, we must convert the data from 24 bits to 16 bits.
    // else, we use the raster data in pi_pInData without conversion.
    if (BytesPerPixel == 2)
        {
        pTempBuffer.reset(new Byte[pi_InDataSize]);
        Byte* pBuffer = pTempBuffer.get();
        uint16_t pOutput;
        for (uint32_t i = 0, j = 0; j < pi_InDataSize; j+=3)
            {
            pOutput = ((Byte*)pi_pInData)[j] >> 3;
            pOutput <<= 5;
            pOutput |= ((Byte*)pi_pInData)[j+1] >> 3;
            pOutput <<= 5;
            pOutput |= ((Byte*)pi_pInData)[j+2] >> 3;

            *(pBuffer+i++) = *(Byte*)&pOutput;
            *(pBuffer+i++) = *(((Byte*)&pOutput)+1);
            }

        pPixel = pTempBuffer.get();
        }

    while (NbPixel > 1)
        {
        if (0 == memcmp (pPixel, pPixel+BytesPerPixel, BytesPerPixel))
            {
            // The to consecutive pixels are equal. This is a packet of equal pixels
            Len = 0;
            while ((NbPixel - Len > 1) && (Len < 0x7f) && (0 == memcmp(pPixel, pPixel+(BytesPerPixel), BytesPerPixel)))
                {
                Len++;          // Len is always one smaller than the real number of pixel
                pPixel += BytesPerPixel;
                }

            // Write the code and the pixel of the packet in the buffer
            *pDest = (Byte)(0x80 | Len);

            memcpy(pDest + 1, pPixel, BytesPerPixel);

            // Conversion for the pixel
            if (BytesPerPixel >= 3)
                {
                std::swap(pDest[1], pDest[1 + 2]);  // swap red and blue
                
                if ((BytesPerPixel == 4) && (m_AlphaChannelBits == 0))
                    pDest[1 + 3] = 255;            // Fake the maximum opacity
                }

            pDest += BytesPerPixel + 1;
            pPixel += BytesPerPixel;
            po_OutBufferSize += BytesPerPixel + 1;
            NbPixel -= Len + 1;
            }
        else
            {
            // The two consecutive pixels are not equal. We have a raw packet.
            Len = 0;
            pCode = pDest++;            // Keep the position of the code for update at the end.

            while ((NbPixel - Len > 0) && (Len < 0x80) && (0 != memcmp (pPixel, pPixel+BytesPerPixel, BytesPerPixel)))
                {
                memcpy(pDest, pPixel, BytesPerPixel);

                // Conversion for the pixel
                if (BytesPerPixel >= 3)
                    {
                    std::swap(pDest[0], pDest[2]);  // swap red and blue

                    if ((BytesPerPixel == 4) && (m_AlphaChannelBits == 0))
                        pDest[3] = 255;            // Fake the maximum opacity
                    }

                pDest += BytesPerPixel;
                pPixel += BytesPerPixel;
                Len++;
                }

            *pCode = (Byte)(Len-1);
            po_OutBufferSize += (BytesPerPixel * Len) + 1;
            NbPixel -= Len;
            }
        }

    if (1 == NbPixel)   // last pixel is a raw.
        {
//      Len = 1;
        *pDest = 0; // (Len-1)
        ++pDest;

        memcpy(pDest, pPixel, BytesPerPixel);

        // Conversion for the pixel
        if (BytesPerPixel >= 3)
            {
            std::swap(pDest[0], pDest[2]);  // swap red and blue

            if ((BytesPerPixel == 4) && (m_AlphaChannelBits == 0))
                pDest[3] = 255;            // Fake the maximum opacity
            }

        po_OutBufferSize += (BytesPerPixel * 1/*len*/) + 1;

        --NbPixel;
        }

    HASSERT (NbPixel == 0);

    return po_OutBufferSize;
    }

//-----------------------------------------------------------------------------
// public
// DecompressSubset
//-----------------------------------------------------------------------------
size_t HCDCodecTGARLE::DecompressSubset(const void* pi_pInData,
                                        size_t      pi_InDataSize,
                                        void*       po_pOutBuffer,
                                        size_t      pi_OutBufferSize)
    {
    uint32_t i;
    uint32_t j;
    uint16_t Input;
    Byte  Count;
    Byte pPixel[4];  // worst case
    size_t  NbBytes = 0;

    if (GetSubsetPosY() == 0)
        SetCurrentState (STATE_DECOMPRESS);

    switch (m_NumberOfBitsPerPixelInOutput)
        {
        case 8 :
            for (i = 0; i < pi_InDataSize;)
                {
                Count = (((Byte*)pi_pInData)[i] & 0x7f) + 1;
                if (((Byte*)pi_pInData)[i++] & 0x80)
                    {
                    // We are in the case of a RunLength packet
                    // The next pixel represent the pixel to be reproduce "Count" time
                    *pPixel = ((Byte*)pi_pInData)[i++];
                    for (j = 0; j < Count; j++)
                        ((Byte*)po_pOutBuffer)[NbBytes++] = *pPixel;
                    }
                else
                    {
                    // We are in the case of a Raw data packet
                    // The following "Count" pixels are not encoded.
                    for (j = 0; j < Count; j++)
                        ((Byte*)po_pOutBuffer)[NbBytes++] = ((Byte*)pi_pInData)[i++];
                    }
                }
            break;
        case 15 :
        case 16 :
            for (i = 0; i < pi_InDataSize;)
                {
                Count = (((Byte*)pi_pInData)[i] & 0x7f) + 1;
                if (((Byte*)pi_pInData)[i++] & 0x80)
                    {
                    // RunLength packet
                    Input = *(uint16_t*)((Byte*)pi_pInData + i);
                    i += 2;
                    pPixel[0] = ((Input & 0x7c00) >> 10) * 0xff / 0x1f;
                    pPixel[1] = (Byte)(((Input & 0x3e0) >> 5)   * 0xff / 0x1f);
                    pPixel[2] =  (Input & 0x1f)          * 0xff / 0x1f;
                    for (j = 0; j < Count; j++)
                        {
                        memcpy ((Byte*)po_pOutBuffer + NbBytes, pPixel, 3);
                        NbBytes += 3;
                        }
                    }
                else
                    {
                    // RawData packet
                    for (j = 0; j < Count; j++)
                        {
                        Input = *(uint16_t*)((Byte*)pi_pInData + i);
                        i += 2;
                        ((Byte*)po_pOutBuffer)[NbBytes++] = ((Input & 0x7c00) >> 10) * 0xff / 0x1f;
                        ((Byte*)po_pOutBuffer)[NbBytes++] = (Byte)(((Input & 0x3e0) >> 5)   * 0xff / 0x1f);
                        ((Byte*)po_pOutBuffer)[NbBytes++] =  (Input & 0x1f)          * 0xff / 0x1f;
                        }
                    }
                }
            break;
        case 24 :
        case 32 :
            uint32_t PixelSizeInByte = static_cast<uint32_t>(GetBitsPerPixel() / 8);
            Byte Swap;
            for (i = 0; i < pi_InDataSize;)
                {
                Count = (((Byte*)pi_pInData)[i] & 0x7f) + 1;
                if (((Byte*)pi_pInData)[i++] & 0x80)
                    {
                    // RunLength packet
                    memcpy (pPixel, (Byte*)pi_pInData+i, PixelSizeInByte);
                    i += PixelSizeInByte;
                    Swap = pPixel[0];
                    pPixel[0] = pPixel[2];
                    pPixel[2] = Swap;

                    // if the alpha channel was not used
                    if (GetBitsPerPixel() == 32 && m_AlphaChannelBits == 0)
                        pPixel[3] = 0xff;

                    for (j = 0; j < Count; j++)
                        {
                        memcpy ((Byte*)po_pOutBuffer+NbBytes, pPixel, PixelSizeInByte);
                        NbBytes += PixelSizeInByte;
                        }
                    }
                else
                    {
                    // RawData packet
                    for (j = 0; j < Count; j++)
                        {
                        memcpy ((Byte*)po_pOutBuffer+NbBytes, (Byte*)pi_pInData+i, PixelSizeInByte);
                        Swap = ((Byte*)po_pOutBuffer)[NbBytes];
                        ((Byte*)po_pOutBuffer)[NbBytes] = ((Byte*)po_pOutBuffer)[NbBytes+2];
                        ((Byte*)po_pOutBuffer)[NbBytes+2] = Swap;

                        // if the alpha channel was not used
                        if (GetBitsPerPixel() == 32 && m_AlphaChannelBits == 0)
                            ((Byte*)po_pOutBuffer)[NbBytes+3] = 0xff;

                        NbBytes += PixelSizeInByte;
                        i += PixelSizeInByte;
                        }
                    }

                }
            break;
        }

    if (NbBytes > pi_OutBufferSize)
        return 0;

    return NbBytes;
    }

//-----------------------------------------------------------------------------
// public
// HasLineAccess
//-----------------------------------------------------------------------------
bool HCDCodecTGARLE::HasLineAccess() const
    {
    return true;
    }

//-----------------------------------------------------------------------------
// public
// IsBitsPerPixelSupported
//-----------------------------------------------------------------------------
bool HCDCodecTGARLE::IsBitsPerPixelSupported(size_t pi_Bits) const
    {
    bool Result = false;

    switch (pi_Bits)
        {
        case 8 :
        case 16 :
        case 24 :
        case 32 :
            Result = true;
            break;
        }

    return Result;
    }

//-----------------------------------------------------------------------------
// public
// GetSubsetMaxCompressedSize
//-----------------------------------------------------------------------------
size_t HCDCodecTGARLE::GetSubsetMaxCompressedSize() const
    {
    return (GetSubsetWidth() * GetSubsetHeight() * GetBitsPerPixel() * 3 / 0x10);
    }


/**----------------------------------------------------------------------------
 Set the alpha channel bits for the codec.

 @param pi_AlphaChannelBits The number of bits for the alpha channel. If the
                            BitsPerPixel is 32, the Pi_AlphaChannelBits can be
                            set to 0 or 8, otherwise 0

 @see GetAlphaChannelBits
-----------------------------------------------------------------------------*/
void HCDCodecTGARLE::SetAlphaChannelBits(Byte pi_AlphaChannelBits)
    {
    HPRECONDITION((GetBitsPerPixel() == 32 && (pi_AlphaChannelBits == 0 || pi_AlphaChannelBits == 8)) ||
                  (GetBitsPerPixel() != 32 && pi_AlphaChannelBits == 0));

    m_AlphaChannelBits = pi_AlphaChannelBits;
    }

/**----------------------------------------------------------------------------
 Get the alpha channel bits for the codec.

 @return Byte The alpha channel bits

 @see SetAlphaChannelBits
-----------------------------------------------------------------------------*/
Byte HCDCodecTGARLE::GetAlphaChannelBits() const
    {
    return m_AlphaChannelBits;
    }

//-----------------------------------------------------------------------------
// public
// SetNumberOfBitsPerPixelInOutput
//-----------------------------------------------------------------------------
void HCDCodecTGARLE::SetNumberOfBitsPerPixelInOutput(Byte pi_BitsPerPixel)
    {
    m_NumberOfBitsPerPixelInOutput = pi_BitsPerPixel;
    }
