//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrp/src/HRPPixelTypeV1Gray1.cpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HRPPixelTypeV1Gray1
//-----------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>
#include <Imagepp/all/h/HRPPixelTypeV1Gray1.h>
#include <Imagepp/all/h/HRPPixelConverter.h>
#include <Imagepp/all/h/HRPChannelOrgGray.h>


#include <Imagepp/all/h/HRPPixelTypeV32R8G8B8A8.h>
#include <Imagepp/all/h/HRPPixelTypeV8Gray8.h>
#include <Imagepp/all/h/HFCMath.h>

// According non linear human vision perception, a source having
// only 18% of a reference luminance appears about half bright.
// We usually dont have any reference, so, take the max range as
// reference wich give: 255 * 0.18 = 45.9
#define TWOCOLORTHRESHOLD   (46)

HPM_REGISTER_CLASS(HRPPixelTypeV1Gray1, HRPPixelTypeGray)

// STL typeDef
typedef map<HCLASS_ID, HRPPixelConverter*, less<HCLASS_ID>, allocator<HRPPixelConverter*> >
MapHRPPixelTypeToConverter;


static Byte s_BitMask[8]    = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };
static Byte s_NotBitMask[8] = {0x7F, 0xBF, 0xDF, 0xEF, 0xF7, 0xFB, 0xFD, 0xFE };
static Byte s_SrcMask[9]    = {0x00, 0x80, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC, 0xFE, 0xFF };
static Byte s_DestMask[9]   = {0xFF, 0x7F, 0x3F, 0x1F, 0x0F, 0x07, 0x03, 0x01, 0x00 };

//-----------------------------------------------------------------------------
//  s_V1Gray1_V1Gray1 - Converter between HRPPixelTypeV1Gray1
//-----------------------------------------------------------------------------
struct ConverterV1Gray1_V1Gray1 : public HRPPixelConverter
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
        return(new ConverterV1Gray1_V1Gray1(*this));
        }
    };
static struct ConverterV1Gray1_V1Gray1 s_V1Gray1_V1Gray1;

//-----------------------------------------------------------------------------
//  s_V1Gray1_V8Gray8 - Converter
//-----------------------------------------------------------------------------
struct ConverterV1Gray1_V8Gray8 : public HRPPixelConverter
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
        return(new ConverterV1Gray1_V8Gray8(*this));
        }
    };
static struct ConverterV1Gray1_V8Gray8 s_V1Gray1_V8Gray8;

//-----------------------------------------------------------------------------
//  s_V8Gray8_V1Gray1 - Converter
//-----------------------------------------------------------------------------
struct ConverterV8Gray8_V1Gray1 : public HRPPixelConverter
    {
    DEFINE_T_SUPER(HRPPixelConverter)

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData) const
        {
        if((*((Byte*)pi_pSourceRawData)) >= TWOCOLORTHRESHOLD)
            (*((Byte*)pio_pDestRawData)) = (*((Byte*)pio_pDestRawData)) | 0x80; // set bit
        else
            (*((Byte*)pio_pDestRawData)) = (*((Byte*)pio_pDestRawData)) & 0x7F; // clear bit
        }

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const
        {
        Byte* pSrc =  (Byte*)pi_pSourceRawData;
        Byte* pDest = (Byte*)pio_pDestRawData;
        Byte Index = 0;

        while(pi_PixelsCount)
            {
            if(*pSrc >= TWOCOLORTHRESHOLD)
                *pDest = *pDest | s_BitMask[Index];
            else
                *pDest = *pDest & s_NotBitMask[Index];

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
        return(new ConverterV8Gray8_V1Gray1(*this));
        }
    };
static struct ConverterV8Gray8_V1Gray1 s_V8Gray8_V1Gray1;

//-----------------------------------------------------------------------------
//  s_V32R8G8B8A8_V1Gray1 - Converter
//-----------------------------------------------------------------------------
struct ConverterV32R8G8B8A8_V1Gray1 : public HRPPixelConverter
    {
public:
    DEFINE_T_SUPER(HRPPixelConverter)

    void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData) const
        {
        Byte Gray = Byte(((Byte*)pi_pSourceRawData)[0] * REDFACTOR +
                             ((Byte*)pi_pSourceRawData)[1] * GREENFACTOR +
                             ((Byte*)pi_pSourceRawData)[2] * BLUEFACTOR);

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
            Gray = Byte(pSrc[0] * REDFACTOR +
                          pSrc[1] * GREENFACTOR +
                          pSrc[2] * BLUEFACTOR);

            if(Gray >= TWOCOLORTHRESHOLD)
                *pDest = *pDest | s_BitMask[Index];
            else
                *pDest = *pDest & s_NotBitMask[Index];

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
                    *pDest |= 0x80; // set bit
                else
                    *pDest &= 0x7F; // clear bit
                }
            else
                {
                Byte gray = (Byte)(pSrc[0] * REDFACTOR + pSrc[1] * GREENFACTOR +  pSrc[2] * BLUEFACTOR);

                // (S * alpha) + (D * (1 - alpha))
                if(HFCMath::GetInstance()->DivideBy255((gray * pSrc[3]) + (((*pDest >> 7) & 0x01) * (255 - pSrc[3]))) >= TWOCOLORTHRESHOLD)
                    *pDest |= 0x80; // set bit
                else
                    *pDest &= 0x7F; // clear bit
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
                        *pDest |= s_BitMask[Index]; // set bit
                    else
                        *pDest &= s_NotBitMask[Index]; // clear bit
                    }
                else
                    {
                    Byte gray = (Byte)(pSrc[0] * REDFACTOR + pSrc[1] * GREENFACTOR +  pSrc[2] * BLUEFACTOR);

                    // (S * alpha) + (D * (1 - alpha))
                    if(pQuotients->DivideBy255((gray * pSrc[3]) + (((*pDest >> (7-Index)) & 0x01) * (255 - pSrc[3]))) >= TWOCOLORTHRESHOLD)
                        *pDest |= s_BitMask[Index]; // set bit
                    else
                        *pDest &= s_NotBitMask[Index]; // clear bit
                    }
                }

            --pi_PixelsCount;

            // Next RGBA pixel
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
        return(new ConverterV32R8G8B8A8_V1Gray1(*this));
        }
    };
short ConverterV32R8G8B8A8_V1Gray1::m_LostChannels[] = {3, -1};
static struct ConverterV32R8G8B8A8_V1Gray1 s_V32R8G8B8A8_V1Gray1;

//-----------------------------------------------------------------------------
//  Dictionnary of converters from other pixel types
//-----------------------------------------------------------------------------
struct V1Gray1ConvertersFrom : public MapHRPPixelTypeToConverter
    {
    V1Gray1ConvertersFrom() : MapHRPPixelTypeToConverter ()
        {
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV1Gray1::CLASS_ID, &s_V1Gray1_V1Gray1));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV8Gray8::CLASS_ID, &s_V8Gray8_V1Gray1));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV32R8G8B8A8::CLASS_ID, &s_V32R8G8B8A8_V1Gray1));
        };
    };

//-----------------------------------------------------------------------------
//  Dictionnary of converters from other pixel types
//-----------------------------------------------------------------------------
struct V1Gray1ConvertersTo : public MapHRPPixelTypeToConverter
    {
    V1Gray1ConvertersTo() : MapHRPPixelTypeToConverter ()
        {
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV1Gray1::CLASS_ID, &s_V1Gray1_V1Gray1));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV8Gray8::CLASS_ID, &s_V1Gray1_V8Gray8));
        };
    };

//-----------------------------------------------------------------------------
// Constructor for 1 bits grayscale
//-----------------------------------------------------------------------------
HRPPixelTypeV1Gray1::HRPPixelTypeV1Gray1()
    : HRPPixelTypeGray(1,0),
      HRPPixelType1BitInterface()
    {
    }


//-----------------------------------------------------------------------------
// Copy constructor
//-----------------------------------------------------------------------------
HRPPixelTypeV1Gray1::HRPPixelTypeV1Gray1(const HRPPixelTypeV1Gray1& pi_rObj)
    : HRPPixelTypeGray(pi_rObj),
      HRPPixelType1BitInterface(pi_rObj)
    {
    }

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
HRPPixelTypeV1Gray1::~HRPPixelTypeV1Gray1()
    {
    }

//-----------------------------------------------------------------------------
// Clone method
//-----------------------------------------------------------------------------
HPMPersistentObject* HRPPixelTypeV1Gray1::Clone() const
    {
    return new HRPPixelTypeV1Gray1(*this);
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
unsigned short HRPPixelTypeV1Gray1::CountValueBits() const
    {
    return 1;
    }

//-----------------------------------------------------------------------------
// Get1BitInterface
//-----------------------------------------------------------------------------
HRPPixelType1BitInterface* HRPPixelTypeV1Gray1::Get1BitInterface()
    {
    return this;
    }

//-----------------------------------------------------------------------------
// HasConverterFrom
//-----------------------------------------------------------------------------
const HRPPixelConverter* HRPPixelTypeV1Gray1::HasConverterFrom(const HRPPixelType* pi_pPixelTypeFrom) const
    {
    HFCMonitor Monitor(HRPPixelType::s_ConverterAccess);

    // This declaration needs to remain local in order to prevent initialization problems
    static struct V1Gray1ConvertersFrom s_ConvertersFrom;

    MapHRPPixelTypeToConverter::const_iterator Itr;
    Itr = s_ConvertersFrom.find (pi_pPixelTypeFrom->GetClassID());

    return ((Itr == s_ConvertersFrom.end()) ? 0 : (*Itr).second);
    }

//-----------------------------------------------------------------------------
// HasConverterTo
//-----------------------------------------------------------------------------
const HRPPixelConverter* HRPPixelTypeV1Gray1::HasConverterTo(const HRPPixelType* pi_pPixelTypeTo) const
    {
    HFCMonitor Monitor(HRPPixelType::s_ConverterAccess);

    // This declaration needs to remain local in order to prevent initialization problems
    static struct V1Gray1ConvertersTo s_ConvertersTo;

    MapHRPPixelTypeToConverter::const_iterator Itr;
    Itr = s_ConvertersTo.find (pi_pPixelTypeTo->GetClassID());

    return ((Itr == s_ConvertersTo.end()) ? 0 : (*Itr).second);
    }

