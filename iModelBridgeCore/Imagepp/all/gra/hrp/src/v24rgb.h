//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrp/src/v24rgb.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#include <Imagepp/all/h/HRPPaletteOctreeR8G8B8.h>

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

// *** YCC conversion factor ***
#define RGB_to_YCC_Y0  0.299F
#define RGB_to_YCC_Y1  0.587F
#define RGB_to_YCC_Y2  0.114F

#define RGB_to_YCC_Cb0 -0.168935F
#define RGB_to_YCC_Cb1 -0.331665F
#define RGB_to_YCC_Cb2  0.50059F

#define RGB_to_YCC_Cr0  0.499813F
#define RGB_to_YCC_Cr1 -0.418531F
#define RGB_to_YCC_Cr2 -0.081282F

//From HRPPixelTypeV24PhotoYcc.cpp
extern const float pixel_YCC_to_RGB_Convert_redTable[256];
extern const float pixel_YCC_to_RGB_Convert_greenTable1[256];
extern const float pixel_YCC_to_RGB_Convert_greenTable2[256];
extern const float pixel_YCC_to_RGB_Convert_blueTable[256];

#define CLAMP_8BIT_RANGE(A) ((A)<=(0) ? (0) : (A)<(256) ? (uint32_t)(A): (255))

//RGB_to_YCC doesn't need to be clamped to 0..255
inline uint32_t Convert_RGB_to_YCC_Y (uint32_t R, uint32_t G, uint32_t B)  { return (uint32_t)(RGB_to_YCC_Y0  * R + RGB_to_YCC_Y1  * G + RGB_to_YCC_Y2  * B);}
inline uint32_t Convert_RGB_to_YCC_Cb(uint32_t R, uint32_t G, uint32_t B)  { return (uint32_t)(RGB_to_YCC_Cb0 * R + RGB_to_YCC_Cb1 * G + RGB_to_YCC_Cb2 * B + 128);}
inline uint32_t Convert_RGB_to_YCC_Cr(uint32_t R, uint32_t G, uint32_t B)  { return (uint32_t)(RGB_to_YCC_Cr0 * R + RGB_to_YCC_Cr1 * G + RGB_to_YCC_Cr2 * B + 128);}

//YCC_to_RGB function has to be clamped to 0..255
inline uint32_t Convert_YCC_to_RGB_Red   (uint32_t Y, uint32_t Cr)           { return CLAMP_8BIT_RANGE(Y + pixel_YCC_to_RGB_Convert_redTable[Cr]);}
inline uint32_t Convert_YCC_to_RGB_Green (uint32_t Y, uint32_t Cb, uint32_t Cr){ return CLAMP_8BIT_RANGE(Y + pixel_YCC_to_RGB_Convert_greenTable1[Cb] + pixel_YCC_to_RGB_Convert_greenTable2[Cr]);}
inline uint32_t Convert_YCC_to_RGB_Blue  (uint32_t Y, uint32_t Cb)           { return CLAMP_8BIT_RANGE(Y + pixel_YCC_to_RGB_Convert_blueTable[Cb]);}

// *** Blend routines ***
// Adst' = 1 - ( (1 - Asrc) * (1 - Adst) ) for values in [0..1]
inline Byte Blend_Alpha_Channel(uint32_t Asrc, uint32_t Adst) { return (Byte)(255 - ( (255 - Asrc) * (255 - Adst) ) / 255);}

// PRdst' = PRsrc - PRdst*Asrc + PRdst
inline Byte Blend_PRsrc_PRdst    (uint32_t PRsrc, uint32_t PRdst, uint32_t Asrc) { return (Byte)CLAMP_8BIT_RANGE(PRsrc - ((PRdst * Asrc) / 255) + PRdst);}

// PRdst' = PRsrc - dst*Asrc + dst
inline Byte Blend_PRsrc_OPAQUEdst(uint32_t PRsrc, uint32_t dst, uint32_t Asrc)   { return (Byte)CLAMP_8BIT_RANGE(PRsrc - ((dst * Asrc) / 255) + dst);}

// dst' = Asrc*(src-dst) + dst
inline Byte Blend_src_OPAQUEdst(uint32_t src, uint32_t dst, uint32_t Asrc) { return (Byte)CLAMP_8BIT_RANGE( (Asrc * (src - dst)) / 255 + dst);}

// dst' = (Asrc * (src - (Adst * dst)) + (Adst * dst)) / Aout (simplified)
inline Byte Blend_src_dst(uint32_t src, uint32_t dst, uint32_t Asrc, uint32_t Adst, uint32_t Aout) { return (Byte)CLAMP_8BIT_RANGE( (Asrc * (src - ((dst*Adst) / 255)) + (dst*Adst) ) / Aout);}

// PRdst' = Asrc * src - PRdst*Asrc + PRdst
inline Byte Blend_src_PRdst(uint32_t src, uint32_t PRdst, uint32_t Asrc) { return (Byte)CLAMP_8BIT_RANGE(Asrc * ((src - PRdst) / 255) + PRdst);}

//-----------------------------------------------------------------------------
//  s_V24R8G8B8_V24R8G8B8 - Converter between g_PixelTypeFingerprintV24R8G8B8
//-----------------------------------------------------------------------------
template<int RTo, int BTo, int RFrom, int BFrom>
struct ConverterV24R8G8B8_V24R8G8B8 : public HRPPixelConverter
    {
    DEFINE_T_SUPER(HRPPixelConverter)

    void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
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

    HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterV24R8G8B8_V24R8G8B8<RTo,BTo,RFrom,BFrom>(*this));
        }
    };

//-----------------------------------------------------------------------------
//  s_V8Gray8_V24R8G8B8 - Converter
//-----------------------------------------------------------------------------
struct ConverterV8Gray8_V24R8G8B8 : public HRPPixelConverter
    {
    DEFINE_T_SUPER(HRPPixelConverter)

    void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
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

    HRPPixelConverter* AllocateCopy() const  override{
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

    void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
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

    HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterV24R8G8B8_V8Gray8<RFrom, BFrom>(*this));
        }
    };

//-----------------------------------------------------------------------------
//  s_V8GrayWhite8_V24R8G8B8 - Converter
//-----------------------------------------------------------------------------
struct ConverterV8GrayWhite8_V24R8G8B8 : public HRPPixelConverter
    {
    DEFINE_T_SUPER(HRPPixelConverter)

    void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
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

    HRPPixelConverter* AllocateCopy() const  override{
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

    void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
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

    HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterV24R8G8B8_V8GrayWhite8<RFrom, BFrom>(*this));
        }
    };

//-----------------------------------------------------------------------------
//  s_V1Gray1_V24R8G8B8 - Converter
//-----------------------------------------------------------------------------
struct ConverterV1Gray1_V24R8G8B8 : public HRPPixelConverter
    {
    DEFINE_T_SUPER(HRPPixelConverter)

    void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
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

    HRPPixelConverter* AllocateCopy() const  override{
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

    void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        Byte* pSrc =  (Byte*)pi_pSourceRawData;
        Byte* pDest = (Byte*)pio_pDestRawData;
        uint32_t Index = 0;

        while(pi_PixelsCount)
            {
            if((pSrc[RFrom] * REDFACTOR + pSrc[GFrom] * GREENFACTOR + pSrc[BFrom] * BLUEFACTOR) >= TWOCOLORTHRESHOLD)
                *pDest = *pDest | s_BitMask[Index];
            else
                *pDest = *pDest & s_NotBitMask[Index];

            --pi_PixelsCount;

            // Increment Source by 3 bytes !
            pSrc+=3;

            // Increment Source
            ++Index;
            if(Index == 8)
                {
                // Start writing to a new destination byte
                Index = 0;
                ++pDest;
                }
            }

        // Clear padding bits if we are not on a byte boundary.
        if(Index != 0)
            *pDest &= ~(0xFF >> Index);
        };

    HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterV24R8G8B8_V1Gray1<RFrom,BFrom>(*this));
        }
    };

//-----------------------------------------------------------------------------
//  s_V1Gray1_V24R8G8B8 - Converter
//-----------------------------------------------------------------------------
struct ConverterV1GrayWhite1_V24R8G8B8 : public HRPPixelConverter
    {
    DEFINE_T_SUPER(HRPPixelConverter)

    void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
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

    HRPPixelConverter* AllocateCopy() const  override{
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

    void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        Byte* pSrc =  (Byte*)pi_pSourceRawData;
        Byte* pDest = (Byte*)pio_pDestRawData;
        uint32_t Index = 0;

        while(pi_PixelsCount)
            {
            if((pSrc[RFrom] * REDFACTOR + pSrc[GFrom] * GREENFACTOR + pSrc[BFrom] * BLUEFACTOR) >= TWOCOLORTHRESHOLD)
                *pDest = *pDest & s_NotBitMask[Index];
            else
                *pDest = *pDest | s_BitMask[Index];

            --pi_PixelsCount;

            // Increment Source by 3 bytes !
            pSrc+=3;

            // Increment Source
            ++Index;
            if(Index == 8)
                {
                // Start writing to a new destination byte
                Index = 0;
                ++pDest;
                }
            }

        // Clear padding bits if we are not on a byte boundary.
        if(Index != 0)
            *pDest &= ~(0xFF >> Index);
        };

    HRPPixelConverter* AllocateCopy() const  override{
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

    void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
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

    HRPPixelConverter* AllocateCopy() const  override{
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

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
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

    HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterV24R8G8B8_I8R8G8B8<RFrom,BFrom>(*this));
        }


protected:

    virtual void Update() override
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

