//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrp/src/HRPPixelTypeV16B5G5R5.cpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HRPPixelTypeV16B5G5R5
//-----------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>
#include <Imagepp/all/h/HRPPixelTypeV16B5G5R5.h>
#include <Imagepp/all/h/HRPPixelConverter.h>
#include <Imagepp/all/h/HRPChannelOrgB5G5R5.h>


#include <Imagepp/all/h/HRPPixelTypeI8R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeV24R8G8B8.h>

HPM_REGISTER_CLASS(HRPPixelTypeV16B5G5R5, HRPPixelType)

// STL typeDef
typedef map<HCLASS_ID, HRPPixelConverter*, less<HCLASS_ID>, allocator<HRPPixelConverter*> >
MapHRPPixelTypeToConverter;


//-----------------------------------------------------------------------------
//  s_V16B5G5R5_V16B5G5R5 - Converter
//-----------------------------------------------------------------------------
struct ConverterV16B5G5R5_V16B5G5R5 : public HRPPixelConverter
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
        return(new ConverterV16B5G5R5_V16B5G5R5(*this));
        }
    };
static ConverterV16B5G5R5_V16B5G5R5        s_V16B5G5R5_V16B5G5R5;

//-----------------------------------------------------------------------------
//  s_V16B5G5R5_V24R8G8B8 - Converter
//-----------------------------------------------------------------------------
struct ConverterV16B5G5R5_V24R8G8B8 : public HRPPixelConverter
    {
public:
    DEFINE_T_SUPER(HRPPixelConverter)

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData) const
        {
        ((Byte*)pio_pDestRawData)[0] = (Byte)((*((unsigned short*)pi_pSourceRawData) & 0x001F) << 3);
        ((Byte*)pio_pDestRawData)[1] = (Byte)((*((unsigned short*)pi_pSourceRawData) & 0x03E0) >> 2);
        ((Byte*)pio_pDestRawData)[2] = (Byte)((*((unsigned short*)pi_pSourceRawData) & 0x7C00) >> 7);
        };

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const
        {

        unsigned short* pSourceComposite =  (unsigned short*)pi_pSourceRawData;
        Byte*  pDestComposite = (Byte*)pio_pDestRawData;

        // Copy entire bytes
        while(pi_PixelsCount)
            {
            pDestComposite[0] = (Byte)((*pSourceComposite & 0x001F) << 3);
            pDestComposite[1] = (Byte)((*pSourceComposite & 0x03E0) >> 2);
            pDestComposite[2] = (Byte)((*pSourceComposite & 0x7C00) >> 7);

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
        return(new ConverterV16B5G5R5_V24R8G8B8(*this));
        }
    };
static ConverterV16B5G5R5_V24R8G8B8        s_V16B5G5R5_V24R8G8B8;

//-----------------------------------------------------------------------------
//  s_V24R8G8B8_V16B5G5R5 - Converter
//-----------------------------------------------------------------------------
struct ConverterV24R8G8B8_V16B5G5R5 : public HRPPixelConverter
    {
public:
    DEFINE_T_SUPER(HRPPixelConverter)

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData) const
        {
        unsigned short* pDestComposite = (unsigned short*) pio_pDestRawData;
        Byte*  pSourceComposite = (Byte*)pi_pSourceRawData;

        *pDestComposite = pSourceComposite[2] >> 3;
        *pDestComposite <<= 5;
        *pDestComposite |= pSourceComposite[1] >> 3;
        *pDestComposite <<= 5;
        *pDestComposite |= pSourceComposite[0] >> 3;
        };

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const
        {

        Byte*  pSourceComposite =  (Byte*)pi_pSourceRawData;
        unsigned short* pDestComposite = (unsigned short*)pio_pDestRawData;

        // Copy entire bytes
        while(pi_PixelsCount)
            {
            *pDestComposite = pSourceComposite[2] >> 3;
            *pDestComposite <<= 5;
            *pDestComposite |= pSourceComposite[1] >> 3;
            *pDestComposite <<= 5;
            *pDestComposite |= pSourceComposite[0] >> 3;

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
        return(new ConverterV24R8G8B8_V16B5G5R5(*this));
        }
    };
static ConverterV24R8G8B8_V16B5G5R5        s_V24R8G8B8_V16B5G5R5;

//-----------------------------------------------------------------------------
//  s_I8R8G8B8_V16B5G5R5 - Converter
//-----------------------------------------------------------------------------
struct ConverterI8R8G8B8_V16B5G5R5 : public HRPPixelConverter
    {
public:
    DEFINE_T_SUPER(HRPPixelConverter)

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData) const
        {
        Byte* pSourceComposite = (Byte*)GetSourcePixelType()->GetPalette().GetCompositeValue(*((Byte*)pi_pSourceRawData));

        *((unsigned short*)pio_pDestRawData) =  (pSourceComposite[0] >> 3) << 10 |
                                         (pSourceComposite[1] >> 3) << 5  |
                                         (pSourceComposite[2] >> 3);

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

            *pDestComposite = (pSourceComposite[0] >> 3) << 10 |
                              (pSourceComposite[1] >> 3) << 5  |
                              (pSourceComposite[2] >> 3);

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
        return(new ConverterI8R8G8B8_V16B5G5R5(*this));
        }
    };
static ConverterI8R8G8B8_V16B5G5R5        s_I8R8G8B8_V16B5G5R5;

//-----------------------------------------------------------------------------
//  Dictionnary of converters from other pixel types
//-----------------------------------------------------------------------------
struct V16B5G5R5ConvertersFrom : public MapHRPPixelTypeToConverter
    {
    V16B5G5R5ConvertersFrom() : MapHRPPixelTypeToConverter ()
        {
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeI8R8G8B8::CLASS_ID, &s_I8R8G8B8_V16B5G5R5));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV24R8G8B8::CLASS_ID, &s_V24R8G8B8_V16B5G5R5));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV16B5G5R5::CLASS_ID, &s_V16B5G5R5_V16B5G5R5));
        };
    };



//-----------------------------------------------------------------------------
//  Dictionnary of converters from other pixel types
//-----------------------------------------------------------------------------
struct V16B5G5R5ConvertersTo : public MapHRPPixelTypeToConverter
    {
    V16B5G5R5ConvertersTo() : MapHRPPixelTypeToConverter ()
        {
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV24R8G8B8::CLASS_ID, &s_V16B5G5R5_V24R8G8B8));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV16B5G5R5::CLASS_ID, &s_V16B5G5R5_V16B5G5R5));
        };
    };

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
HRPPixelTypeV16B5G5R5::HRPPixelTypeV16B5G5R5()
    : HRPPixelType(HRPChannelOrgB5G5R5(), 0)
    {
    }

//-----------------------------------------------------------------------------
// Copy constructor
//-----------------------------------------------------------------------------
HRPPixelTypeV16B5G5R5::HRPPixelTypeV16B5G5R5(const HRPPixelTypeV16B5G5R5& pi_rObj)
    : HRPPixelType(pi_rObj)
    {
    }

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
HRPPixelTypeV16B5G5R5::~HRPPixelTypeV16B5G5R5()
    {
    }

//-----------------------------------------------------------------------------
// Clone method
//-----------------------------------------------------------------------------
HPMPersistentObject* HRPPixelTypeV16B5G5R5::Clone() const
    {
    return new HRPPixelTypeV16B5G5R5(*this);
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
unsigned short HRPPixelTypeV16B5G5R5::CountValueBits() const
    {
    return 16;
    }

//-----------------------------------------------------------------------------
// HasConverterFrom
//-----------------------------------------------------------------------------
const HRPPixelConverter* HRPPixelTypeV16B5G5R5::HasConverterFrom(const HRPPixelType* pi_pPixelTypeFrom) const
    {
    HFCMonitor Monitor(HRPPixelType::s_ConverterAccess);

    // This declaration needs to remain local in order to prevent initialization problems
    static struct V16B5G5R5ConvertersFrom s_ConvertersFrom;

    MapHRPPixelTypeToConverter::const_iterator Itr;
    Itr = s_ConvertersFrom.find (pi_pPixelTypeFrom->GetClassID());

    return ((Itr == s_ConvertersFrom.end()) ? 0 : (*Itr).second);
    }

//-----------------------------------------------------------------------------
// HasConverterTo
//-----------------------------------------------------------------------------
const HRPPixelConverter* HRPPixelTypeV16B5G5R5::HasConverterTo(const HRPPixelType* pi_pPixelTypeTo) const
    {
    HFCMonitor Monitor(HRPPixelType::s_ConverterAccess);

    // This declaration needs to remain local in order to prevent initialization problems
    static struct V16B5G5R5ConvertersTo s_ConvertersTo;

    MapHRPPixelTypeToConverter::const_iterator Itr;
    Itr = s_ConvertersTo.find (pi_pPixelTypeTo->GetClassID());

    return ((Itr == s_ConvertersTo.end()) ? 0 : (*Itr).second);
    }

