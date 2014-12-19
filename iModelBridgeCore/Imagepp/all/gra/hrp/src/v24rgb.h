//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrp/src/v24rgb.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#include <Imagepp/all/h/HRPPaletteOctreeR8G8B8.h>

extern struct ConverterV8Gray8_V24R8G8B8 g_V8Gray8_V24R8G8B8; // exported for raptc24i.cpp
extern struct ConverterV1Gray1_V24R8G8B8 g_V1Gray1_V24R8G8B8; // exported for raptc24i.cpp

// According non linear human vision perception, a source having
// only 18% of a reference luminance appears about half bright.
// We usually dont have any reference, so, take the max range as
// reference wich give: 255 * 0.18 = 45.9
#define TWOCOLORTHRESHOLD   (46)


static const Byte s_BitMask[8]    = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };
static const Byte s_NotBitMask[8] = {0x7F, 0xBF, 0xDF, 0xEF, 0xF7, 0xFB, 0xFD, 0xFE };
static const Byte s_SrcMask[9]    = {0x00, 0x80, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC, 0xFE, 0xFF };
static const Byte s_DestMask[9]   = {0xFF, 0x7F, 0x3F, 0x1F, 0x0F, 0x07, 0x03, 0x01, 0x00 };

// These macros are useful for template classes below
// RGB_RED, RGB_BLUE, BGR_RED, BGR_BLUE are useful for instantiation of
// these template classes
#define RGB_RED   (0)
#define RGB_BLUE  (2)
#define BGR_RED   (2)
#define BGR_BLUE  (0)
#define GFrom     (1)
#define GTo       (1)

//-----------------------------------------------------------------------------
//  s_V24R8G8B8_V24R8G8B8 - Converter between g_PixelTypeFingerprintV24R8G8B8
//-----------------------------------------------------------------------------
template<int RTo, int BTo, int RFrom, int BFrom>
struct ConverterV24R8G8B8_V24R8G8B8 : public HRPPixelConverter
    {
    DEFINE_T_SUPER(HRPPixelConverter)

    void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData) const
        {
        ((Byte*)pio_pDestRawData)[RTo] = ((Byte*)pi_pSourceRawData)[RFrom];
        ((Byte*)pio_pDestRawData)[GTo] = ((Byte*)pi_pSourceRawData)[GFrom];
        ((Byte*)pio_pDestRawData)[BTo] = ((Byte*)pi_pSourceRawData)[BFrom];
        };
    void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const
        {
        if ((RTo == RFrom) && (BTo == BFrom))
            {
            memcpy (pio_pDestRawData, pi_pSourceRawData, pi_PixelsCount*3);
            }
        else
            {
            Byte* pSrc  = (Byte*)pi_pSourceRawData;
            Byte* pDest = (Byte*)pio_pDestRawData;

            while(pi_PixelsCount)
                {
                pDest[RTo] = pSrc[RFrom];
                pDest[GTo] = pSrc[GFrom];
                pDest[BTo] = pSrc[BFrom];

                // One less pixel to go
                --pi_PixelsCount;

                // Increment Source and Destination by 3 bytes each !
                pDest+=3;
                pSrc+=3;
                }
            }
        };
    virtual void    Convert(const void* pi_pSourceRawData,
                            void* pio_pDestRawData,
                            size_t pi_PixelsCount,
                            const bool* pi_pChannelsMask) const
        {
        return T_Super::Convert(pi_pSourceRawData,pio_pDestRawData,pi_PixelsCount,pi_pChannelsMask);
        }

    HRPPixelConverter* AllocateCopy() const {
        return(new ConverterV24R8G8B8_V24R8G8B8<RTo,BTo,RFrom,BFrom>(*this));
        }
    };

//-----------------------------------------------------------------------------
//  s_V8Gray8_V24R8G8B8 - Converter
//-----------------------------------------------------------------------------
struct ConverterV8Gray8_V24R8G8B8 : public HRPPixelConverter
    {
    DEFINE_T_SUPER(HRPPixelConverter)

    void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData) const
        {
        ((Byte*)pio_pDestRawData)[0] =
            ((Byte*)pio_pDestRawData)[1] =
                ((Byte*)pio_pDestRawData)[2] = *((Byte*)pi_pSourceRawData);
        }
    void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const
        {
        Byte* pSrc  = (Byte*)pi_pSourceRawData;
        Byte* pDest = (Byte*)pio_pDestRawData;

        while(pi_PixelsCount)
            {
            pDest[0] = pDest[1] = pDest[2] = pSrc[0];

            pi_PixelsCount--;

            // Increment Destination by 3 bytes !
            pDest+=3;

            // Increment Source
            pSrc++;
            }
        };
    virtual void    Convert(const void* pi_pSourceRawData,
                            void* pio_pDestRawData,
                            size_t pi_PixelsCount,
                            const bool* pi_pChannelsMask) const
        {
        return T_Super::Convert(pi_pSourceRawData,pio_pDestRawData,pi_PixelsCount,pi_pChannelsMask);
        }

    HRPPixelConverter* AllocateCopy() const {
        return(new ConverterV8Gray8_V24R8G8B8(*this));
        }
    };

//-----------------------------------------------------------------------------
//  s_V24R8G8B8_V8Gray8 - Converter
//-----------------------------------------------------------------------------
template<int RFrom, int BFrom>
struct ConverterV24R8G8B8_V8Gray8 : public HRPPixelConverter
    {
    DEFINE_T_SUPER(HRPPixelConverter)

    void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData) const
        {
        *((Byte*)pio_pDestRawData) = (Byte)(((Byte*)pi_pSourceRawData)[RFrom] * REDFACTOR +
                                                ((Byte*)pi_pSourceRawData)[GFrom] * GREENFACTOR +
                                                ((Byte*)pi_pSourceRawData)[BFrom] * BLUEFACTOR);
        }
    void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const
        {
        Byte* pSrc  = (Byte*)pi_pSourceRawData;
        Byte* pDest = (Byte*)pio_pDestRawData;

        while(pi_PixelsCount)
            {
            pDest[0] = (Byte)(pSrc[RFrom] * REDFACTOR +
                                pSrc[GFrom] * GREENFACTOR +
                                pSrc[BFrom] * BLUEFACTOR);
            pi_PixelsCount--;

            // Increment Destination
            pDest++;

            // Increment Source by 3 bytes !
            pSrc+=3;
            }
        };
    virtual void    Convert(const void* pi_pSourceRawData,
                            void* pio_pDestRawData,
                            size_t pi_PixelsCount,
                            const bool* pi_pChannelsMask) const
        {
        return T_Super::Convert(pi_pSourceRawData,pio_pDestRawData,pi_PixelsCount,pi_pChannelsMask);
        }

    HRPPixelConverter* AllocateCopy() const {
        return(new ConverterV24R8G8B8_V8Gray8<RFrom, BFrom>(*this));
        }
    };

//-----------------------------------------------------------------------------
//  s_V8GrayWhite8_V24R8G8B8 - Converter
//-----------------------------------------------------------------------------
struct ConverterV8GrayWhite8_V24R8G8B8 : public HRPPixelConverter
    {
    DEFINE_T_SUPER(HRPPixelConverter)

    void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData) const
        {
        ((Byte*)pio_pDestRawData)[0] =
            ((Byte*)pio_pDestRawData)[1] =
                ((Byte*)pio_pDestRawData)[2] = (255 - *((Byte*)pi_pSourceRawData));
        }
    void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const
        {
        Byte* pSrc  = (Byte*)pi_pSourceRawData;
        Byte* pDest = (Byte*)pio_pDestRawData;

        while(pi_PixelsCount)
            {
            pDest[0] = pDest[1] = pDest[2] = (255 - pSrc[0]);

            pi_PixelsCount--;

            // Increment Destination by 3 bytes !
            pDest+=3;

            // Increment Source
            pSrc++;
            }
        };
    virtual void    Convert(const void* pi_pSourceRawData,
                            void* pio_pDestRawData,
                            size_t pi_PixelsCount,
                            const bool* pi_pChannelsMask) const
        {
        return T_Super::Convert(pi_pSourceRawData,pio_pDestRawData,pi_PixelsCount,pi_pChannelsMask);
        }

    HRPPixelConverter* AllocateCopy() const {
        return(new ConverterV8GrayWhite8_V24R8G8B8(*this));
        }
    };

//-----------------------------------------------------------------------------
//  s_V24R8G8B8_V8GrayWhite8 - Converter
//-----------------------------------------------------------------------------
template<int RFrom, int BFrom>
struct ConverterV24R8G8B8_V8GrayWhite8 : public HRPPixelConverter
    {
    DEFINE_T_SUPER(HRPPixelConverter)

    void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData) const
        {
        *((Byte*)pio_pDestRawData) = 255 - (Byte)(((Byte*)pi_pSourceRawData)[RFrom] * REDFACTOR +
                                                      ((Byte*)pi_pSourceRawData)[GFrom] * GREENFACTOR +
                                                      ((Byte*)pi_pSourceRawData)[BFrom] * BLUEFACTOR);
        }
    void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const
        {
        Byte* pSrc  = (Byte*)pi_pSourceRawData;
        Byte* pDest = (Byte*)pio_pDestRawData;

        while(pi_PixelsCount)
            {
            pDest[0] = 255 - (Byte)(pSrc[RFrom] * REDFACTOR +
                                      pSrc[GFrom] * GREENFACTOR +
                                      pSrc[BFrom] * BLUEFACTOR);
            pi_PixelsCount--;

            // Increment Destination
            pDest++;

            // Increment Source by 3 bytes !
            pSrc+=3;
            }
        };
    virtual void    Convert(const void* pi_pSourceRawData,
                            void* pio_pDestRawData,
                            size_t pi_PixelsCount,
                            const bool* pi_pChannelsMask) const
        {
        return T_Super::Convert(pi_pSourceRawData,pio_pDestRawData,pi_PixelsCount,pi_pChannelsMask);
        }

    HRPPixelConverter* AllocateCopy() const {
        return(new ConverterV24R8G8B8_V8GrayWhite8<RFrom, BFrom>(*this));
        }
    };

//-----------------------------------------------------------------------------
//  s_V1Gray1_V24R8G8B8 - Converter
//-----------------------------------------------------------------------------
struct ConverterV1Gray1_V24R8G8B8 : public HRPPixelConverter
    {
    DEFINE_T_SUPER(HRPPixelConverter)

    void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData) const
        {
        ((Byte*)pio_pDestRawData)[0] =
            ((Byte*)pio_pDestRawData)[1] =
                ((Byte*)pio_pDestRawData)[2] =
                    (*((Byte*)pi_pSourceRawData) & 0x80 ? 255 : 0);
        }
    void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const
        {
        Byte* pSrc =  (Byte*)pi_pSourceRawData;
        Byte* pDest = (Byte*)pio_pDestRawData;
        Byte Index;
        Byte IndexMax;

        while(pi_PixelsCount)
            {
            if((pi_PixelsCount >= 32) && (*((uint32_t*)pSrc) == 0xffffffff))
                {
                memset(pDest, 0xff, 96);

                pSrc += 4;
                pDest += 96;

                pi_PixelsCount -= 32;
                }
            else if((pi_PixelsCount >= 32) && (*((uint32_t*)pSrc) == 0x00000000))
                {
                memset(pDest, 0x00, 96);

                pSrc += 4;
                pDest += 96;

                pi_PixelsCount -= 32;
                }
            else
                // if we have 0xff, we can assume height "1"s pixels
                if((*pSrc == 0xff) && (pi_PixelsCount >= 8))
                    {
                    memset(pDest, 0xff, 24);

                    pSrc++;
                    pDest += 24;

                    pi_PixelsCount -= 8;
                    }
                else
                    // if we have 0x00, we can assume height "0"'s pixels
                    if((*pSrc == 0x00) && (pi_PixelsCount >= 8))
                        {
                        memset(pDest, 0x00, 24);

                        pSrc++;
                        pDest += 24;

                        pi_PixelsCount -= 8;
                        }
                    else
                        {
                        // otherwise, we must parse each bit (pixel) in the byte

                        if(pi_PixelsCount > 8)
                            {
                            IndexMax = 8;

                            pi_PixelsCount -= 8;
                            }
                        else
                            {
                            IndexMax = (Byte)pi_PixelsCount;

                            pi_PixelsCount = 0;
                            }

                        Index = 0;

                        while(Index != IndexMax)
                            {
                            // Set this output pixel
                            pDest[0] = pDest[1] = pDest[2] = (*pSrc & s_BitMask[Index] ? 255 : 0);

                            // Increment Destination by 3 bytes !
                            pDest+=3;

                            // Increment Source index
                            Index++;
                            }

                        // increment source pointer
                        pSrc++;
                        }
            }
        };
    virtual void    Convert(const void* pi_pSourceRawData,
                            void* pio_pDestRawData,
                            size_t pi_PixelsCount,
                            const bool* pi_pChannelsMask) const
        {
        return T_Super::Convert(pi_pSourceRawData,pio_pDestRawData,pi_PixelsCount,pi_pChannelsMask);
        }

    HRPPixelConverter* AllocateCopy() const {
        return(new ConverterV1Gray1_V24R8G8B8(*this));
        }
    };

//-----------------------------------------------------------------------------
//  s_V24R8G8B8_V1Gray1 - Converter
//-----------------------------------------------------------------------------
template<int RFrom, int BFrom>
struct ConverterV24R8G8B8_V1Gray1 : public HRPPixelConverter
    {
    DEFINE_T_SUPER(HRPPixelConverter)

    void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData) const
        {
        Byte Gray = (Byte)(((Byte*)pi_pSourceRawData)[RFrom] * REDFACTOR +
                               ((Byte*)pi_pSourceRawData)[GFrom] * GREENFACTOR +
                               ((Byte*)pi_pSourceRawData)[BFrom] * BLUEFACTOR);

        if(Gray >= TWOCOLORTHRESHOLD)
            (*((Byte*)pio_pDestRawData)) = (*((Byte*)pio_pDestRawData)) | 0x80; // set bit
        else
            (*((Byte*)pio_pDestRawData)) = (*((Byte*)pio_pDestRawData)) & 0x7F; // clear bit

        }
    void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const
        {
        Byte Gray;

        Byte* pSrc =  (Byte*)pi_pSourceRawData;
        Byte* pDest = (Byte*)pio_pDestRawData;
        Byte Index = 0;

        while(pi_PixelsCount)
            {
            Gray =(Byte)(pSrc[RFrom] * REDFACTOR +
                           pSrc[GFrom] * GREENFACTOR +
                           pSrc[BFrom] * BLUEFACTOR);

            if(Gray >= TWOCOLORTHRESHOLD)
                *pDest = *pDest | s_BitMask[Index];
            else
                *pDest = *pDest & s_NotBitMask[Index];

            pi_PixelsCount--;

            // Increment Source by 3 bytes !
            pSrc+=3;

            // Increment Source
            Index++;
            if(Index == 8)
                {
                // Start writing to a new destination byte
                Index = 0;
                pDest++;
                }
            }
        };
    virtual void    Convert(const void* pi_pSourceRawData,
                            void* pio_pDestRawData,
                            size_t pi_PixelsCount,
                            const bool* pi_pChannelsMask) const
        {
        return T_Super::Convert(pi_pSourceRawData,pio_pDestRawData,pi_PixelsCount,pi_pChannelsMask);
        }

    HRPPixelConverter* AllocateCopy() const {
        return(new ConverterV24R8G8B8_V1Gray1<RFrom,BFrom>(*this));
        }
    };

//-----------------------------------------------------------------------------
//  s_V1Gray1_V24R8G8B8 - Converter
//-----------------------------------------------------------------------------
struct ConverterV1GrayWhite1_V24R8G8B8 : public HRPPixelConverter
    {
    DEFINE_T_SUPER(HRPPixelConverter)

    void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData) const
        {
        ((Byte*)pio_pDestRawData)[0] =
            ((Byte*)pio_pDestRawData)[1] =
                ((Byte*)pio_pDestRawData)[2] =
                    (*((Byte*)pi_pSourceRawData) & 0x80 ? 0 : 255);
        }
    void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const
        {
        Byte* pSrc =  (Byte*)pi_pSourceRawData;
        Byte* pDest = (Byte*)pio_pDestRawData;
        Byte Index;
        Byte IndexMax;

        while(pi_PixelsCount)
            {
            if((pi_PixelsCount >= 32) && (*((uint32_t*)pSrc) == 0xffffffff))
                {
                memset(pDest, 0x00, 96);

                pSrc += 4;
                pDest += 96;

                pi_PixelsCount -= 32;
                }
            else if((pi_PixelsCount >= 32) && (*((uint32_t*)pSrc) == 0x00000000))
                {
                memset(pDest, 0xff, 96);

                pSrc += 4;
                pDest += 96;

                pi_PixelsCount -= 32;
                }
            else
                // if we have 0xff, we can assume height "1"s pixels
                if((*pSrc == 0xff) && (pi_PixelsCount >= 8))
                    {
                    memset(pDest, 0x00, 24);

                    pSrc++;
                    pDest += 24;

                    pi_PixelsCount -= 8;
                    }
                else
                    // if we have 0x00, we can assume height "0"'s pixels
                    if((*pSrc == 0x00) && (pi_PixelsCount >= 8))
                        {
                        memset(pDest, 0xff, 24);

                        pSrc++;
                        pDest += 24;

                        pi_PixelsCount -= 8;
                        }
                    else
                        {
                        // otherwise, we must parse each bit (pixel) in the byte

                        if(pi_PixelsCount > 8)
                            {
                            IndexMax = 8;

                            pi_PixelsCount -= 8;
                            }
                        else
                            {
                            IndexMax = (Byte)pi_PixelsCount;

                            pi_PixelsCount = 0;
                            }

                        Index = 0;

                        while(Index != IndexMax)
                            {
                            // Set this output pixel
                            pDest[0] = pDest[1] = pDest[2] = (*pSrc & s_BitMask[Index] ? 0 : 255);

                            // Increment Destination by 3 bytes !
                            pDest+=3;

                            // Increment Source index
                            Index++;
                            }

                        // increment source pointer
                        pSrc++;
                        }
            }
        };
    virtual void    Convert(const void* pi_pSourceRawData,
                            void* pio_pDestRawData,
                            size_t pi_PixelsCount,
                            const bool* pi_pChannelsMask) const
        {
        return T_Super::Convert(pi_pSourceRawData,pio_pDestRawData,pi_PixelsCount,pi_pChannelsMask);
        }

    HRPPixelConverter* AllocateCopy() const {
        return(new ConverterV1GrayWhite1_V24R8G8B8(*this));
        }
    };

//-----------------------------------------------------------------------------
//  s_V24R8G8B8_V1GrayWhite1 - Converter
//-----------------------------------------------------------------------------
template<int RFrom, int BFrom>
struct ConverterV24R8G8B8_V1GrayWhite1 : public HRPPixelConverter
    {
    DEFINE_T_SUPER(HRPPixelConverter)

    void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData) const
        {
        Byte Gray =  (Byte)(((Byte*)pi_pSourceRawData)[RFrom] * REDFACTOR +
                                ((Byte*)pi_pSourceRawData)[GFrom] * GREENFACTOR +
                                ((Byte*)pi_pSourceRawData)[BFrom] * BLUEFACTOR);

        if(Gray >= TWOCOLORTHRESHOLD)
            (*((Byte*)pio_pDestRawData)) = (*((Byte*)pio_pDestRawData)) & 0x7f; // set bit
        else
            (*((Byte*)pio_pDestRawData)) = (*((Byte*)pio_pDestRawData)) | 0x80; // clear bit

        }
    void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const
        {
        Byte Gray;

        Byte* pSrc =  (Byte*)pi_pSourceRawData;
        Byte* pDest = (Byte*)pio_pDestRawData;
        Byte Index = 0;

        while(pi_PixelsCount)
            {
            Gray = (Byte)(pSrc[RFrom] * REDFACTOR +
                            pSrc[GFrom] * GREENFACTOR +
                            pSrc[BFrom] * BLUEFACTOR);

            if(Gray >= TWOCOLORTHRESHOLD)
                *pDest = *pDest & s_NotBitMask[Index];
            else
                *pDest = *pDest | s_BitMask[Index];

            pi_PixelsCount--;

            // Increment Source by 3 bytes !
            pSrc+=3;

            // Increment Source
            Index++;
            if(Index == 8)
                {
                // Start writing to a new destination byte
                Index = 0;
                pDest++;
                }
            }
        };
    virtual void    Convert(const void* pi_pSourceRawData,
                            void* pio_pDestRawData,
                            size_t pi_PixelsCount,
                            const bool* pi_pChannelsMask) const
        {
        return T_Super::Convert(pi_pSourceRawData,pio_pDestRawData,pi_PixelsCount,pi_pChannelsMask);
        }

    HRPPixelConverter* AllocateCopy() const {
        return(new ConverterV24R8G8B8_V1GrayWhite1<RFrom,BFrom>(*this));
        }
    };

//-----------------------------------------------------------------------------
//  s_I8R8G8B8_V24R8G8B8 - Converter
//-----------------------------------------------------------------------------
template<int RTo, int BTo>
struct ConverterI8R8G8B8_V24R8G8B8 : public HRPPixelConverter
    {
    DEFINE_T_SUPER(HRPPixelConverter)

    void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData) const
        {
        Byte* pSourceComposite = (Byte*)GetSourcePixelType()->GetPalette().GetCompositeValue(*((Byte*)pi_pSourceRawData));

        ((Byte*)pio_pDestRawData)[RTo] = ((Byte*)pSourceComposite)[0];
        ((Byte*)pio_pDestRawData)[GTo] = ((Byte*)pSourceComposite)[1];
        ((Byte*)pio_pDestRawData)[BTo] = ((Byte*)pSourceComposite)[2];
        };
    void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const
        {
        Byte* pSrc  = (Byte*)pi_pSourceRawData;
        Byte* pDest = (Byte*)pio_pDestRawData;

        Byte* pSourceComposite;
        const HRPPixelPalette& rPalette = GetSourcePixelType()->GetPalette();

        while(pi_PixelsCount)
            {
            pSourceComposite = (Byte*)(rPalette.GetCompositeValue(*pSrc));

            pDest[RTo] = pSourceComposite[0];
            pDest[GTo] = pSourceComposite[1];
            pDest[BTo] = pSourceComposite[2];

            // One less pixel to go
            pi_PixelsCount--;

            // Increment Destination by 3 bytes !
            pDest+=3;

            // Increment Destination by 1 byte (index) !
            pSrc++;
            }
        };
    virtual void    Convert(const void* pi_pSourceRawData,
                            void* pio_pDestRawData,
                            size_t pi_PixelsCount,
                            const bool* pi_pChannelsMask) const
        {
        return T_Super::Convert(pi_pSourceRawData,pio_pDestRawData,pi_PixelsCount,pi_pChannelsMask);
        }

    HRPPixelConverter* AllocateCopy() const {
        return(new ConverterI8R8G8B8_V24R8G8B8<RTo,BTo>(*this));
        }
    };

//-----------------------------------------------------------------------------
//  s_V24R8G8B8_I8R8G8B8 - Converter
//-----------------------------------------------------------------------------
template<int RFrom, int BFrom>
class ConverterV24R8G8B8_I8R8G8B8 : public HRPPixelConverter
    {
    HRPPaletteOctreeR8G8B8 m_QuantizedPalette;

public:
    DEFINE_T_SUPER(HRPPixelConverter)

    ConverterV24R8G8B8_I8R8G8B8()
        {
        };

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData) const
        {
        // get a good index for the R,G,B source values
        *((Byte*)pio_pDestRawData) = m_QuantizedPalette.GetIndex(
                                           ((Byte*)pi_pSourceRawData)[RFrom],
                                           ((Byte*)pi_pSourceRawData)[GFrom],
                                           ((Byte*)pi_pSourceRawData)[BFrom]);

        };

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const
        {
        Byte* pSrc = (Byte*)pi_pSourceRawData;
        Byte* pDest = (Byte*)pio_pDestRawData;

        // convert all pixels
        while(pi_PixelsCount)
            {
            // get a good index for the R,G,B source values
            *pDest        = m_QuantizedPalette.GetIndex(
                                pSrc[RFrom],
                                pSrc[GFrom],
                                pSrc[BFrom]);

            // increment the pointer to the next 24-bit (3 bytes) composite value
            pSrc+=3;

            // increment the destination
            pDest++;

            pi_PixelsCount--;
            }
        };
    virtual void    Convert(const void* pi_pSourceRawData,
                            void* pio_pDestRawData,
                            size_t pi_PixelsCount,
                            const bool* pi_pChannelsMask) const
        {
        return T_Super::Convert(pi_pSourceRawData,pio_pDestRawData,pi_PixelsCount,pi_pChannelsMask);
        }

    virtual void ConvertToValue(const void* pi_pSourceRawData, void* pio_pDestRawData) const
        {
        ((Byte*)pio_pDestRawData)[0] = ((Byte*)pi_pSourceRawData)[RFrom];
        ((Byte*)pio_pDestRawData)[1] = ((Byte*)pi_pSourceRawData)[GFrom];
        ((Byte*)pio_pDestRawData)[2] = ((Byte*)pi_pSourceRawData)[BFrom];
        };

    HRPPixelConverter* AllocateCopy() const {
        return(new ConverterV24R8G8B8_I8R8G8B8<RFrom,BFrom>(*this));
        }


protected:

    virtual void Update()
        {
        const HRPPixelPalette& rPalette = GetDestinationPixelType()->GetPalette();

        // fill the octree with the destination palette entries
        long nbIndex = rPalette.CountUsedEntries();
        for(int Index = 0; Index < nbIndex; Index++)
            {
            m_QuantizedPalette.AddCompositeValue(rPalette.GetCompositeValue(Index),
                                                 (Byte)Index);
            }
        }

    };

