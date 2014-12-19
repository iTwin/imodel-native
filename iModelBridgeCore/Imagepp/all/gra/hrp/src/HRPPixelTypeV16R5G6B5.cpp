//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrp/src/HRPPixelTypeV16R5G6B5.cpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HRPPixelTypeV16R5G6B5
//-----------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>
#include <Imagepp/all/h/HRPPixelTypeV16R5G6B5.h>
#include <Imagepp/all/h/HRPPixelConverter.h>
#include <Imagepp/all/h/HRPChannelOrgR5G6B5.h>


#include <Imagepp/all/h/HRPPixelTypeI8R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeV24R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeV32R8G8B8A8.h>
#include <Imagepp/all/h/HFCMath.h>

HPM_REGISTER_CLASS(HRPPixelTypeV16R5G6B5, HRPPixelType)

// STL typeDef
typedef map<HCLASS_ID, HRPPixelConverter*, less<HCLASS_ID>, allocator<HRPPixelConverter*> >
MapHRPPixelTypeToConverter;


//-----------------------------------------------------------------------------
//  s_V16R5G6B5_V16R5G6B5 - Converter
//-----------------------------------------------------------------------------
struct ConverterV16R5G6B5_V16R5G6B5 : public HRPPixelConverter
    {
public:
    DEFINE_T_SUPER(HRPPixelConverter)

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData) const
        {
        memcpy(pio_pDestRawData, pi_pSourceRawData, 2); // 2 * sizeof(Byte)
        };

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const
        {

        unsigned short* pSourceComposite =  (unsigned short*)pi_pSourceRawData;
        unsigned short*  pDestComposite = (unsigned short*)pio_pDestRawData;

        // Copy entire bytes
        while(pi_PixelsCount)
            {
            memcpy(pDestComposite, pSourceComposite, sizeof(unsigned short)); // 2 * sizeof(Byte)

            pi_PixelsCount--;
            pDestComposite++;
            pSourceComposite++;
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
        return(new ConverterV16R5G6B5_V16R5G6B5(*this));
        }
    };
static ConverterV16R5G6B5_V16R5G6B5        s_V16R5G6B5_V16R5G6B5;

//-----------------------------------------------------------------------------
//  s_V16R5G6B5_V24R8G8B8 - Converter
//-----------------------------------------------------------------------------
struct ConverterV16R5G6B5_V24R8G8B8 : public HRPPixelConverter
    {
public:
    DEFINE_T_SUPER(HRPPixelConverter)

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData) const
        {
        ((Byte*)pio_pDestRawData)[0] = (Byte)((*((unsigned short*)pi_pSourceRawData) & 0xF800) >> 8);
        ((Byte*)pio_pDestRawData)[1] = (Byte)((*((unsigned short*)pi_pSourceRawData) & 0x07E0) >> 3);
        ((Byte*)pio_pDestRawData)[2] = (Byte)((*((unsigned short*)pi_pSourceRawData) & 0x001F) << 3);
        };

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const
        {

        unsigned short* pSourceComposite =  (unsigned short*)pi_pSourceRawData;
        Byte*  pDestComposite = (Byte*)pio_pDestRawData;

        // Copy entire bytes
        while(pi_PixelsCount)
            {
            pDestComposite[0] = (Byte)((*pSourceComposite & 0xF800) >> 8);
            pDestComposite[1] = (Byte)((*pSourceComposite & 0x07E0) >> 3);
            pDestComposite[2] = (Byte)((*pSourceComposite & 0x001F) << 3);

            pi_PixelsCount -= 1;
            pDestComposite += 3;
            pSourceComposite += 1;
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
        return(new ConverterV16R5G6B5_V24R8G8B8(*this));
        }
    };
static ConverterV16R5G6B5_V24R8G8B8        s_V16R5G6B5_V24R8G8B8;

//-----------------------------------------------------------------------------
//  s_V24R8G8B8_V16R5G6B5 - Converter
//-----------------------------------------------------------------------------
struct ConverterV24R8G8B8_V16R5G6B5 : public HRPPixelConverter
    {
public:
    DEFINE_T_SUPER(HRPPixelConverter)

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData) const
        {
        unsigned short* pDestComposite = (unsigned short*) pio_pDestRawData;
        Byte*  pSourceComposite = (Byte*)pi_pSourceRawData;

        *pDestComposite = pSourceComposite[0] >> 3;
        *pDestComposite <<= 6;
        *pDestComposite |= pSourceComposite[1] >> 2;
        *pDestComposite <<= 5;
        *pDestComposite |= pSourceComposite[2] >> 3;
        };

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const
        {

        Byte*  pSourceComposite =  (Byte*)pi_pSourceRawData;
        unsigned short* pDestComposite = (unsigned short*)pio_pDestRawData;

        // Copy entire bytes
        while(pi_PixelsCount)
            {
            *pDestComposite = pSourceComposite[0] >> 3;
            *pDestComposite <<= 6;
            *pDestComposite |= pSourceComposite[1] >> 2;
            *pDestComposite <<= 5;
            *pDestComposite |= pSourceComposite[2] >> 3;

            pi_PixelsCount -= 1;
            pDestComposite += 1;
            pSourceComposite += 3;
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
        return(new ConverterV24R8G8B8_V16R5G6B5(*this));
        }
    };
static ConverterV24R8G8B8_V16R5G6B5        s_V24R8G8B8_V16R5G6B5;



//-----------------------------------------------------------------------------
//  s_V16R5G6B5_V32R8G8B8A8 - Converter
//-----------------------------------------------------------------------------
struct ConverterV16R5G6B5_V32R8G8B8A8 : public HRPPixelConverter
    {
public:
    DEFINE_T_SUPER(HRPPixelConverter)

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData) const
        {
        ((Byte*)pio_pDestRawData)[0] = (Byte)((*((unsigned short*)pi_pSourceRawData) & 0xF800) >> 8);
        ((Byte*)pio_pDestRawData)[1] = (Byte)((*((unsigned short*)pi_pSourceRawData) & 0x07E0) >> 3);
        ((Byte*)pio_pDestRawData)[2] = (Byte)((*((unsigned short*)pi_pSourceRawData) & 0x001F) << 3);
        ((Byte*)pio_pDestRawData)[3] = 0xff; // opaque
        };

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const
        {

        unsigned short* pSourceComposite =  (unsigned short*)pi_pSourceRawData;
        Byte*  pDestComposite = (Byte*)pio_pDestRawData;

        // Copy entire bytes
        while(pi_PixelsCount)
            {
            pDestComposite[0] = (Byte)((*pSourceComposite & 0xF800) >> 8);
            pDestComposite[1] = (Byte)((*pSourceComposite & 0x07E0) >> 3);
            pDestComposite[2] = (Byte)((*pSourceComposite & 0x001F) << 3);
            pDestComposite[3] = 0xff; // opaque

            pi_PixelsCount -= 1;
            pDestComposite += 4;
            pSourceComposite += 1;
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
        return(new ConverterV16R5G6B5_V32R8G8B8A8(*this));
        }
    };
static ConverterV16R5G6B5_V32R8G8B8A8        s_V16R5G6B5_V32R8G8B8A8;

//-----------------------------------------------------------------------------
//  s_V32R8G8B8A8_V16R5G6B5 - Converter
//-----------------------------------------------------------------------------
struct ConverterV32R8G8B8A8_V16R5G6B5 : public HRPPixelConverter
    {
public:
    DEFINE_T_SUPER(HRPPixelConverter)

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData) const
        {
        unsigned short* pDestComposite = (unsigned short*) pio_pDestRawData;
        Byte*  pSourceComposite = (Byte*)pi_pSourceRawData;

        *pDestComposite = pSourceComposite[0] >> 3;
        *pDestComposite <<= 6;
        *pDestComposite |= pSourceComposite[1] >> 2;
        *pDestComposite <<= 5;
        *pDestComposite |= pSourceComposite[2] >> 3;
        };

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const
        {

        Byte*  pSourceComposite =  (Byte*)pi_pSourceRawData;
        unsigned short* pDestComposite = (unsigned short*)pio_pDestRawData;

        // Copy entire bytes
        while(pi_PixelsCount)
            {
            *pDestComposite = pSourceComposite[0] >> 3;
            *pDestComposite <<= 6;
            *pDestComposite |= pSourceComposite[1] >> 2;
            *pDestComposite <<= 5;
            *pDestComposite |= pSourceComposite[2] >> 3;

            pi_PixelsCount -= 1;
            pDestComposite += 1;
            pSourceComposite += 4;
            }
        };
    virtual void    Convert(const void* pi_pSourceRawData,
                            void* pio_pDestRawData,
                            size_t pi_PixelsCount,
                            const bool* pi_pChannelsMask) const
        {
        return T_Super::Convert(pi_pSourceRawData,pio_pDestRawData,pi_PixelsCount,pi_pChannelsMask);
        }

    virtual void Compose(const void* pi_pSourceRawData, void* pio_pDestRawData) const
        {
        Byte const* pSourceComposite = (Byte const*)pi_pSourceRawData;
        unsigned short*      pDestComposite = (unsigned short*)pio_pDestRawData;

        // If source pixel is fully transparent, destination is unaltered
        if (pSourceComposite[3] != 0)
            {
            if (pSourceComposite[3] == 255)
                {
                // Source pixel is fully opaque. Copy source pixel.
                *pDestComposite = pSourceComposite[0] >> 3;
                *pDestComposite <<= 6;
                *pDestComposite |= pSourceComposite[1] >> 2;
                *pDestComposite <<= 5;
                *pDestComposite |= pSourceComposite[2] >> 3;
                }
            else
                {
                HFCMath (*pQuotients) (HFCMath::GetInstance());

                Byte red   = (Byte)((*pDestComposite & 0xF800) >> 8);
                Byte green = (Byte)((*pDestComposite & 0x07E0) >> 3);
                Byte blue  = (Byte)((*pDestComposite & 0x001F) << 3);

                *pDestComposite = 0xFFFF & ((pQuotients->DivideBy255(pSourceComposite[3] * (pSourceComposite[0] - red)) + red) >> 3);
                *pDestComposite <<= 6;
                *pDestComposite |= (pQuotients->DivideBy255(pSourceComposite[3] * (pSourceComposite[1] - green)) + green) >> 2;
                *pDestComposite <<= 5;
                *pDestComposite |= (pQuotients->DivideBy255(pSourceComposite[3] * (pSourceComposite[2] - blue)) + blue) >> 3;
                }
            }
        };

    virtual void Compose(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const
        {
        HFCMath (*pQuotients) (HFCMath::GetInstance());

        Byte const*  pSourceComposite =  (Byte const*)pi_pSourceRawData;
        unsigned short*       pDestComposite = (unsigned short*)pio_pDestRawData;

        // Copy entire bytes
        while(pi_PixelsCount)
            {
            // If source pixel is fully transparent, destination is unaltered
            if (pSourceComposite[3] != 0)
                {
                if (pSourceComposite[3] == 255)
                    {
                    // Source pixel is fully opaque. Copy source pixel.
                    *pDestComposite = pSourceComposite[0] >> 3;
                    *pDestComposite <<= 6;
                    *pDestComposite |= pSourceComposite[1] >> 2;
                    *pDestComposite <<= 5;
                    *pDestComposite |= pSourceComposite[2] >> 3;
                    }
                else
                    {
                    Byte red   = (Byte)((*pDestComposite & 0xF800) >> 8);
                    Byte green = (Byte)((*pDestComposite & 0x07E0) >> 3);
                    Byte blue  = (Byte)((*pDestComposite & 0x001F) << 3);

                    *pDestComposite = 0xFFFF & ((pQuotients->DivideBy255(pSourceComposite[3] * (pSourceComposite[0] - red)) + red) >> 3);
                    *pDestComposite <<= 6;
                    *pDestComposite |= (pQuotients->DivideBy255(pSourceComposite[3] * (pSourceComposite[1] - green)) + green) >> 2;
                    *pDestComposite <<= 5;
                    *pDestComposite |= (pQuotients->DivideBy255(pSourceComposite[3] * (pSourceComposite[2] - blue)) + blue) >> 3;
                    }
                }

            --pi_PixelsCount;
            ++pDestComposite;
            pSourceComposite += 4;
            }
        };

    HRPPixelConverter* AllocateCopy() const {
        return(new ConverterV32R8G8B8A8_V16R5G6B5(*this));
        }
    };

static ConverterV32R8G8B8A8_V16R5G6B5        s_V32R8G8B8A8_V16R5G6B5;


//-----------------------------------------------------------------------------
//  s_I8R8G8B8_V16B5G5R5 - Converter
//-----------------------------------------------------------------------------
struct ConverterI8R8G8B8_V16R5G6B5 : public HRPPixelConverter
    {
public:
    DEFINE_T_SUPER(HRPPixelConverter)

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData) const
        {
        Byte* pSourceComposite = (Byte*)GetSourcePixelType()->GetPalette().GetCompositeValue(*((Byte*)pi_pSourceRawData));
        unsigned short* pDestComposite = (unsigned short*) pio_pDestRawData;

        *pDestComposite = pSourceComposite[0] >> 3;
        *pDestComposite <<= 6;
        *pDestComposite |= pSourceComposite[1] >> 2;
        *pDestComposite <<= 5;
        *pDestComposite |= pSourceComposite[2] >> 3;
        };

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const
        {
        Byte*  pSrc =  (Byte*)pi_pSourceRawData;
        unsigned short* pDestComposite = (unsigned short*)pio_pDestRawData;
        Byte* pSourceComposite;

        const HRPPixelPalette& rPalette = GetSourcePixelType()->GetPalette();

        // Copy entire bytes
        while(pi_PixelsCount)
            {
            pSourceComposite = (Byte*)rPalette.GetCompositeValue(*pSrc);

            *pDestComposite = pSourceComposite[0] >> 3;
            *pDestComposite <<= 6;
            *pDestComposite |= pSourceComposite[1] >> 2;
            *pDestComposite <<= 5;
            *pDestComposite |= pSourceComposite[2] >> 3;

            pSrc++;
            pDestComposite++;

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

    HRPPixelConverter* AllocateCopy() const {
        return(new ConverterI8R8G8B8_V16R5G6B5(*this));
        }
    };
static ConverterI8R8G8B8_V16R5G6B5        s_I8R8G8B8_V16R5G6B5;

//-----------------------------------------------------------------------------
//  Dictionnary of converters from other pixel types
//-----------------------------------------------------------------------------
struct V16B5G6R5ConvertersFrom : public MapHRPPixelTypeToConverter
    {
    V16B5G6R5ConvertersFrom() : MapHRPPixelTypeToConverter ()
        {
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeI8R8G8B8::CLASS_ID, &s_I8R8G8B8_V16R5G6B5));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV24R8G8B8::CLASS_ID, &s_V24R8G8B8_V16R5G6B5));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV16R5G6B5::CLASS_ID, &s_V16R5G6B5_V16R5G6B5));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV32R8G8B8A8::CLASS_ID, &s_V32R8G8B8A8_V16R5G6B5));
        };
    };



//-----------------------------------------------------------------------------
//  Dictionnary of converters from other pixel types
//-----------------------------------------------------------------------------
struct V16B5G6R5ConvertersTo : public MapHRPPixelTypeToConverter
    {
    V16B5G6R5ConvertersTo() : MapHRPPixelTypeToConverter ()
        {
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV24R8G8B8::CLASS_ID, &s_V16R5G6B5_V24R8G8B8));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV16R5G6B5::CLASS_ID, &s_V16R5G6B5_V16R5G6B5));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV32R8G8B8A8::CLASS_ID, &s_V16R5G6B5_V32R8G8B8A8));
        };
    };

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
HRPPixelTypeV16R5G6B5::HRPPixelTypeV16R5G6B5()
    : HRPPixelType(HRPChannelOrgR5G6B5(), 0)
    {
    }

//-----------------------------------------------------------------------------
// Copy constructor
//-----------------------------------------------------------------------------
HRPPixelTypeV16R5G6B5::HRPPixelTypeV16R5G6B5(const HRPPixelTypeV16R5G6B5& pi_rObj)
    : HRPPixelType(pi_rObj)
    {
    }

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
HRPPixelTypeV16R5G6B5::~HRPPixelTypeV16R5G6B5()
    {
    }

//-----------------------------------------------------------------------------
// Clone method
//-----------------------------------------------------------------------------
HPMPersistentObject* HRPPixelTypeV16R5G6B5::Clone() const
    {
    return new HRPPixelTypeV16R5G6B5(*this);
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
unsigned short HRPPixelTypeV16R5G6B5::CountValueBits() const
    {
    return 16;
    }

//-----------------------------------------------------------------------------
// HasConverterFrom
//-----------------------------------------------------------------------------
const HRPPixelConverter* HRPPixelTypeV16R5G6B5::HasConverterFrom(const HRPPixelType* pi_pPixelTypeFrom) const
    {
    HFCMonitor Monitor(HRPPixelType::s_ConverterAccess);

    // This declaration needs to remain local in order to prevent initialization problems
    static struct V16B5G6R5ConvertersFrom s_ConvertersFrom;

    MapHRPPixelTypeToConverter::const_iterator Itr;
    Itr = s_ConvertersFrom.find (pi_pPixelTypeFrom->GetClassID());

    return ((Itr == s_ConvertersFrom.end()) ? 0 : (*Itr).second);
    }

//-----------------------------------------------------------------------------
// HasConverterTo
//-----------------------------------------------------------------------------
const HRPPixelConverter* HRPPixelTypeV16R5G6B5::HasConverterTo(const HRPPixelType* pi_pPixelTypeTo) const
    {
    HFCMonitor Monitor(HRPPixelType::s_ConverterAccess);

    // This declaration needs to remain local in order to prevent initialization problems
    static struct V16B5G6R5ConvertersTo s_ConvertersTo;

    MapHRPPixelTypeToConverter::const_iterator Itr;
    Itr = s_ConvertersTo.find (pi_pPixelTypeTo->GetClassID());

    return ((Itr == s_ConvertersTo.end()) ? 0 : (*Itr).second);
    }

