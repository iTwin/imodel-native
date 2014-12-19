//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrp/src/HRPPixelTypeV1GrayWhite1.cpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HRPPixelTypeV1GrayWhite1
//-----------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>

#include <Imagepp/all/h/HRPPixelTypeV1GrayWhite1.h>
#include <Imagepp/all/h/HRPChannelOrgGrayWhite.h>
#include <Imagepp/all/h/HRPPixelConverter.h>
#include <Imagepp/all/h/HRPPixelTypeV32R8G8B8A8.h>
#include <Imagepp/all/h/HRPPixelTypeV8Gray8.h>
#include <Imagepp/all/h/HRPPixelTypeV8GrayWhite8.h>

#include <Imagepp/all/h/HFCMath.h>

// According non linear human vision perception, a source having
// only 18% of a reference luminance appears about half bright.
// We usually dont have any reference, so, take the max range as
// reference wich give: 255 * 0.18 = 45.9
#define TWOCOLORTHRESHOLD   (46)

HPM_REGISTER_CLASS(HRPPixelTypeV1GrayWhite1, HRPPixelType)

// STL typeDef
typedef map<HCLASS_ID, HRPPixelConverter*, less<HCLASS_ID>, allocator<HRPPixelConverter*> >
MapHRPPixelTypeToConverter;


static Byte s_BitMask[8]    = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };
static Byte s_NotBitMask[8] = {0x7F, 0xBF, 0xDF, 0xEF, 0xF7, 0xFB, 0xFD, 0xFE };
static Byte s_SrcMask[9]    = {0x00, 0x80, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC, 0xFE, 0xFF };
static Byte s_DestMask[9]   = {0xFF, 0x7F, 0x3F, 0x1F, 0x0F, 0x07, 0x03, 0x01, 0x00 };

//-----------------------------------------------------------------------------
//  s_V1GrayWhite1_V1GrayWhite1 - Converter between HRPPixelTypeV1GrayWhite1
//-----------------------------------------------------------------------------
struct ConverterV1GrayWhite1_V1GrayWhite1 : public HRPPixelConverter
    {
    DEFINE_T_SUPER(HRPPixelConverter)

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData) const
        {
        *((Byte*)pio_pDestRawData) = (*((Byte*)pi_pSourceRawData) & 0x80) | (*((Byte*)pio_pDestRawData) & 0x7f);
        }

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const
        {
        // Minimize overhead if only one pixel to copy.  In this case, we will do just one "if"
        if(pi_PixelsCount <= 8)
            // Copy n pixels (0 <= n <= 8)
            *((Byte*)pio_pDestRawData) = (*((Byte*)pio_pDestRawData) & s_DestMask[pi_PixelsCount]) |
                                           (*((Byte*)pi_pSourceRawData)& s_SrcMask[pi_PixelsCount]);
        else
            {
            Byte* pSrc =  (Byte*)pi_pSourceRawData;
            Byte* pDest = (Byte*)pio_pDestRawData;

            // Copy entire bytes
            while(pi_PixelsCount >= 8)
                {
                *pDest = *pSrc;
                pi_PixelsCount -= 8;
                pDest++;
                pSrc++;
                }

            // Copy remaining n pixels (0 <= n <= 7)
            *pDest = (*pDest & s_DestMask[pi_PixelsCount]) | (*pSrc & s_SrcMask[pi_PixelsCount]);
            }
        };
    virtual void    Convert(const void* pi_pSourceRawData,
                            void* pio_pDestRawData,
                            size_t pi_PixelsCount,
                            const bool* pi_pChannelsMask) const
        {
        return T_Super::Convert(pi_pSourceRawData,pio_pDestRawData,pi_PixelsCount,pi_pChannelsMask);
        }

    virtual HRPPixelConverter* AllocateCopy() const {
        return(new ConverterV1GrayWhite1_V1GrayWhite1(*this));
        }
    };
static struct ConverterV1GrayWhite1_V1GrayWhite1 s_V1GrayWhite1_V1GrayWhite1;


//-----------------------------------------------------------------------------
//  s_V1Gray1_V32R8G8B8A8 - Converter
//-----------------------------------------------------------------------------
struct ConverterV1GrayWhite1_V32R8G8B8A8 : public HRPPixelConverter
    {
    DEFINE_T_SUPER(HRPPixelConverter)

    void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData) const
        {
        ((Byte*)pio_pDestRawData)[0] = ((Byte*)pio_pDestRawData)[1] = ((Byte*)pio_pDestRawData)[2] = (*((Byte*)pi_pSourceRawData) & 0x80 ? 0 : 255);
        ((Byte*)pio_pDestRawData)[3] = 255;
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
                uint32_t rgbaValue = 0xFF000000;
                for(size_t i=0; i < 32; ++i)
                    ((uint32_t*)pDest)[i] = rgbaValue;

                pSrc += 4;
                pDest += 128;

                pi_PixelsCount -= 32;
                }
            else if((pi_PixelsCount >= 32) && (*((uint32_t*)pSrc) == 0x00000000))
                {
                memset(pDest, 0xff, 128);

                pSrc += 4;
                pDest += 128;

                pi_PixelsCount -= 32;
                }
            else
                // if we have 0xff, we can assume height "1"s pixels
                if((*pSrc == 0xff) && (pi_PixelsCount >= 8))
                    {
                    uint32_t rgbaValue = 0xFF000000;
                    for(size_t i=0; i < 8; ++i)
                        ((uint32_t*)pDest)[i] = rgbaValue;

                    pSrc++;
                    pDest += 32;

                    pi_PixelsCount -= 8;
                    }
                else
                    // if we have 0x00, we can assume height "0"'s pixels
                    if((*pSrc == 0x00) && (pi_PixelsCount >= 8))
                        {
                        memset(pDest, 0xff, 32);

                        pSrc++;
                        pDest += 32;

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
                            pDest[3] = 255;

                            // Increment Destination by 4 bytes !
                            pDest+=4;

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

    virtual HRPPixelConverter* AllocateCopy() const {
        return(new ConverterV1GrayWhite1_V32R8G8B8A8(*this));
        }
    };
static struct ConverterV1GrayWhite1_V32R8G8B8A8 s_V1GrayWhite1_V32R8G8B8A8;

//-----------------------------------------------------------------------------
//  s_V1GrayWhite1_V8Gray8 - Converter
//-----------------------------------------------------------------------------
struct ConverterV1GrayWhite1_V8Gray8 : public HRPPixelConverter
    {
    DEFINE_T_SUPER(HRPPixelConverter)

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData) const
        {
        *((Byte*)pio_pDestRawData) = (*((Byte*)pi_pSourceRawData) & 0x80 ? 0 : 255);
        }

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const
        {
        Byte* pSrc =  (Byte*)pi_pSourceRawData;
        Byte* pDest = (Byte*)pio_pDestRawData;
        Byte Index = 0;

        while(pi_PixelsCount)
            {
            // Set this output pixel
            *pDest = (*pSrc & s_BitMask[Index] ? 0 : 255);

            // One less pixel to go
            pi_PixelsCount--;

            // Increment Destination
            pDest++;

            // Increment Source
            Index++;
            if(Index == 8)
                {
                // Start reading from a new source byte
                Index = 0;
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

    virtual HRPPixelConverter* AllocateCopy() const {
        return(new ConverterV1GrayWhite1_V8Gray8(*this));
        }
    };
static struct ConverterV1GrayWhite1_V8Gray8 s_V1GrayWhite1_V8Gray8;

//-----------------------------------------------------------------------------
//  s_V1GrayWhite1_V8GrayWhite8 - Converter
//-----------------------------------------------------------------------------
struct ConverterV1GrayWhite1_V8GrayWhite8 : public HRPPixelConverter
    {
    DEFINE_T_SUPER(HRPPixelConverter)

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData) const
        {
        *((Byte*)pio_pDestRawData) = (*((Byte*)pi_pSourceRawData) & 0x80 ? 255 : 0);
        }

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const
        {
        Byte* pSrc =  (Byte*)pi_pSourceRawData;
        Byte* pDest = (Byte*)pio_pDestRawData;
        Byte Index = 0;

        while(pi_PixelsCount)
            {
            // Set this output pixel
            *pDest = (*pSrc & s_BitMask[Index] ? 255 : 0);

            // One less pixel to go
            pi_PixelsCount--;

            // Increment Destination
            pDest++;

            // Increment Source
            Index++;
            if(Index == 8)
                {
                // Start reading from a new source byte
                Index = 0;
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

    virtual HRPPixelConverter* AllocateCopy() const {
        return(new ConverterV1GrayWhite1_V8GrayWhite8(*this));
        }
    };
static struct ConverterV1GrayWhite1_V8GrayWhite8 s_V1GrayWhite1_V8GrayWhite8;



//-----------------------------------------------------------------------------
//  s_V8Gray8_V1GrayWhite1 - Converter
//-----------------------------------------------------------------------------
struct ConverterV8Gray8_V1GrayWhite1 : public HRPPixelConverter
    {
    DEFINE_T_SUPER(HRPPixelConverter)

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData) const
        {
        if((*((Byte*)pi_pSourceRawData)) >= TWOCOLORTHRESHOLD)
            (*((Byte*)pio_pDestRawData)) = (*((Byte*)pio_pDestRawData)) & 0x7F; // set bit
        else
            (*((Byte*)pio_pDestRawData)) = (*((Byte*)pio_pDestRawData)) | 0x80; // clear bit
        }

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const
        {
        Byte* pSrc =  (Byte*)pi_pSourceRawData;
        Byte* pDest = (Byte*)pio_pDestRawData;
        Byte Index = 0;

        while(pi_PixelsCount)
            {
            if(*pSrc >= TWOCOLORTHRESHOLD)
                *pDest = *pDest & s_BitMask[Index];
            else
                *pDest = *pDest | s_NotBitMask[Index];

            pi_PixelsCount--;

            // Increment Source
            pSrc++;

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

    virtual HRPPixelConverter* AllocateCopy() const {
        return(new ConverterV8Gray8_V1GrayWhite1(*this));
        }
    };
static struct ConverterV8Gray8_V1GrayWhite1 s_V8Gray8_V1GrayWhite1;

//-----------------------------------------------------------------------------
//  s_V32R8G8B8A8_V1GrayWhite1 - Converter
//-----------------------------------------------------------------------------
struct ConverterV32R8G8B8A8_V1GrayWhite1 : public HRPPixelConverter
    {
public:
    DEFINE_T_SUPER(HRPPixelConverter)

    void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData) const
        {
        Byte Gray = Byte(((Byte*)pi_pSourceRawData)[0] * REDFACTOR +
                             ((Byte*)pi_pSourceRawData)[1] * GREENFACTOR +
                             ((Byte*)pi_pSourceRawData)[2] * BLUEFACTOR);

        if(Gray >= TWOCOLORTHRESHOLD)
            (*((Byte*)pio_pDestRawData)) = (*((Byte*)pio_pDestRawData)) & 0x7f; // clear bit
        else
            (*((Byte*)pio_pDestRawData)) = (*((Byte*)pio_pDestRawData)) | 0x80; // set bit

        }
    void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const
        {
        Byte Gray;

        Byte* pSrc =  (Byte*)pi_pSourceRawData;
        Byte* pDest = (Byte*)pio_pDestRawData;
        Byte Index = 0;

        while(pi_PixelsCount)
            {
            Gray = Byte(pSrc[0] * REDFACTOR +
                          pSrc[1] * GREENFACTOR +
                          pSrc[2] * BLUEFACTOR);

            if(Gray >= TWOCOLORTHRESHOLD)
                *pDest = *pDest & s_NotBitMask[Index];  // clear bit
            else
                *pDest = *pDest | s_BitMask[Index];     // set bit

            pi_PixelsCount--;

            // Increment Source by 3 bytes !
            pSrc+=4;

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

    void Compose(const void* pi_pSourceRawData, void* pio_pDestRawData) const
        {
        Byte const* pSrc = (Byte const*)pi_pSourceRawData;

        // If source pixel is fully transparent, destination is unaltered
        if (pSrc[3] != 0)
            {
            Byte* pDest = (Byte*)pio_pDestRawData;

            if (pSrc[3] == 255)
                {
                // Source pixel is fully opaque. Copy source pixel.
                if((pSrc[0] * REDFACTOR + pSrc[1] * GREENFACTOR +  pSrc[2] * BLUEFACTOR) >= TWOCOLORTHRESHOLD)
                    *pDest &= 0x7F; // clear bit
                else
                    *pDest |= 0x80; // set bit

                }
            else
                {
                Byte gray = (Byte)(pSrc[0] * REDFACTOR + pSrc[1] * GREENFACTOR +  pSrc[2] * BLUEFACTOR);

                // (S * alpha) + (D * (1 - alpha))
                if(HFCMath::GetInstance()->DivideBy255((gray * pSrc[3]) + (((*pDest >> 7) & 0x01) * (255 - pSrc[3]))) >= TWOCOLORTHRESHOLD)
                    *pDest &= 0x7F; // clear bit

                else
                    *pDest |= 0x80; // set bit
                }
            }
        }

    void Compose(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const
        {
        Byte const* pSrc  = (Byte const*)pi_pSourceRawData;
        Byte*       pDest = (Byte*)pio_pDestRawData;
        Byte Index = 0;

        HFCMath (*pQuotients) (HFCMath::GetInstance());

        while(pi_PixelsCount)
            {
            // If source pixel is fully transparent, destination is unaltered
            if (pSrc[3] != 0)
                {
                if (pSrc[3] == 255)
                    {
                    // Source pixel is fully opaque. Copy source pixel.
                    if((pSrc[0] * REDFACTOR + pSrc[1] * GREENFACTOR +  pSrc[2] * BLUEFACTOR) >= TWOCOLORTHRESHOLD)
                        *pDest &= s_NotBitMask[Index]; // clear bit
                    else
                        *pDest |= s_BitMask[Index]; // set bit

                    }
                else
                    {
                    Byte gray = (Byte)(pSrc[0] * REDFACTOR + pSrc[1] * GREENFACTOR +  pSrc[2] * BLUEFACTOR);

                    // (S * alpha) + (D * (1 - alpha))
                    if(pQuotients->DivideBy255((gray * pSrc[3]) + (((*pDest >> (7-Index)) & 0x01) * (255 - pSrc[3]))) >= TWOCOLORTHRESHOLD)
                        *pDest &= s_NotBitMask[Index]; // clear bit
                    else
                        *pDest |= s_BitMask[Index]; // set bit

                    }
                }

            --pi_PixelsCount;

            // Increment Source by 3 bytes !
            pSrc+=4;

            // Increment Source
            ++Index;
            if(Index == 8)
                {
                // Start writing to a new destination byte
                Index = 0;
                ++pDest;
                }
            }
        };

    virtual const short* GetLostChannels() const
        {
        return m_LostChannels;
        }

private:

    static short m_LostChannels[];

    HRPPixelConverter* AllocateCopy() const {
        return(new ConverterV32R8G8B8A8_V1GrayWhite1(*this));
        }
    };
short ConverterV32R8G8B8A8_V1GrayWhite1::m_LostChannels[] = {3, -1};
static struct ConverterV32R8G8B8A8_V1GrayWhite1 s_V32R8G8B8A8_V1GrayWhite1;

//-----------------------------------------------------------------------------
//  Dictionnary of converters from other pixel types
//-----------------------------------------------------------------------------
struct V1GrayWhite1ConvertersFrom : public MapHRPPixelTypeToConverter
    {
    V1GrayWhite1ConvertersFrom() : MapHRPPixelTypeToConverter ()
        {
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV1GrayWhite1::CLASS_ID, &s_V1GrayWhite1_V1GrayWhite1));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV8Gray8::CLASS_ID, &s_V8Gray8_V1GrayWhite1));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV32R8G8B8A8::CLASS_ID, &s_V32R8G8B8A8_V1GrayWhite1));
        };
    };

//-----------------------------------------------------------------------------
//  Dictionnary of converters from other pixel types
//-----------------------------------------------------------------------------
struct V1GrayWhite1ConvertersTo : public MapHRPPixelTypeToConverter
    {
    V1GrayWhite1ConvertersTo() : MapHRPPixelTypeToConverter ()
        {
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV1GrayWhite1::CLASS_ID, &s_V1GrayWhite1_V1GrayWhite1));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV8Gray8::CLASS_ID, &s_V1GrayWhite1_V8Gray8));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV8GrayWhite8::CLASS_ID, &s_V1GrayWhite1_V8GrayWhite8));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV32R8G8B8A8::CLASS_ID, &s_V1GrayWhite1_V32R8G8B8A8));
        };
    };

//-----------------------------------------------------------------------------
// Constructor for 1 bits grayscale
//-----------------------------------------------------------------------------
HRPPixelTypeV1GrayWhite1::HRPPixelTypeV1GrayWhite1()
    : HRPPixelType(HRPChannelOrgGrayWhite(  1,
                                            HRPChannelType::UNUSED,
                                            HRPChannelType::VOID_CH,
                                            0),
                   0),
    HRPPixelType1BitInterface()
    {
    }


//-----------------------------------------------------------------------------
// Copy constructor
//-----------------------------------------------------------------------------
HRPPixelTypeV1GrayWhite1::HRPPixelTypeV1GrayWhite1(const HRPPixelTypeV1GrayWhite1& pi_rObj)
    : HRPPixelType(pi_rObj),
      HRPPixelType1BitInterface(pi_rObj)
    {
    }

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
HRPPixelTypeV1GrayWhite1::~HRPPixelTypeV1GrayWhite1()
    {
    }

//-----------------------------------------------------------------------------
// Clone method
//-----------------------------------------------------------------------------
HPMPersistentObject* HRPPixelTypeV1GrayWhite1::Clone() const
    {
    return new HRPPixelTypeV1GrayWhite1(*this);
    }

/** -----------------------------------------------------------------------------
    This function is used to know the number of "value" bits contain in a pixel
    of this pixel type.

    @b{Example:} @list{HRPPixelTypeV32R8G8B8A8 should return 32.}
                 @list{HRPPixelTypeI8R8G8B8A8 should return 0.}
                 @list{HRPPixelTypeI8VA8R8G8B8 should return 8.}

    @return The number of "value" bits contain in a pixel of this pixel type.
    @end

    @see HRPPixelType::CountIndexBits()
    @see HRPPixelType::CountPixelRawData()
    @end
    -----------------------------------------------------------------------------
 */
unsigned short HRPPixelTypeV1GrayWhite1::CountValueBits() const
    {
    return 1;
    }

//-----------------------------------------------------------------------------
// Get1BitInterface
//-----------------------------------------------------------------------------
HRPPixelType1BitInterface* HRPPixelTypeV1GrayWhite1::Get1BitInterface()
    {
    return this;
    }

//-----------------------------------------------------------------------------
// HasConverterFrom
//-----------------------------------------------------------------------------
const HRPPixelConverter* HRPPixelTypeV1GrayWhite1::HasConverterFrom(const HRPPixelType* pi_pPixelTypeFrom) const
    {
    HFCMonitor Monitor(HRPPixelType::s_ConverterAccess);

    // This declaration needs to remain local in order to prevent initialization problems
    static struct V1GrayWhite1ConvertersFrom s_ConvertersFrom;

    MapHRPPixelTypeToConverter::const_iterator Itr;
    Itr = s_ConvertersFrom.find (pi_pPixelTypeFrom->GetClassID());

    return ((Itr == s_ConvertersFrom.end()) ? 0 : (*Itr).second);
    }

//-----------------------------------------------------------------------------
// HasConverterTo
//-----------------------------------------------------------------------------
const HRPPixelConverter* HRPPixelTypeV1GrayWhite1::HasConverterTo(const HRPPixelType* pi_pPixelTypeTo) const
    {
    HFCMonitor Monitor(HRPPixelType::s_ConverterAccess);

    // This declaration needs to remain local in order to prevent initialization problems
    static struct V1GrayWhite1ConvertersTo s_ConvertersTo;

    MapHRPPixelTypeToConverter::const_iterator Itr;
    Itr = s_ConvertersTo.find (pi_pPixelTypeTo->GetClassID());

    return ((Itr == s_ConvertersTo.end()) ? 0 : (*Itr).second);
    }

