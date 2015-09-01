//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrp/src/HRPPixelTypeV48R16G16B16.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for class HRPPixelTypeV48R16G16B16
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Note
//
// This pixel type work with channel set to 11 bits. We must re-design pixel type
// to support most pixel channel
//-----------------------------------------------------------------------------
#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HRPPixelTypeV48R16G16B16.h>
#include <Imagepp/all/h/HRPPixelTypeV24R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeV32R8G8B8A8.h>
#include <Imagepp/all/h/HRPPixelTypeV64R16G16B16A16.h>
#include <Imagepp/all/h/HFCMath.h>

HPM_REGISTER_CLASS(HRPPixelTypeV48R16G16B16, HRPPixelTypeRGB)

// STL typeDef
typedef map<HCLASS_ID, HRPPixelConverter*, less<HCLASS_ID>, allocator<HRPPixelConverter*> >
MapHRPPixelTypeToConverter;

//-----------------------------------------------------------------------------
//  s_V48R16G16B16_V48R16G16B16 - Converter
//-----------------------------------------------------------------------------
struct ConverterV48R16G16B16_V48R16G16B16 : public HRPPixelConverter
    {
    DEFINE_T_SUPER(HRPPixelConverter)

    void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        HPRECONDITION(UINT_MAX / 6 > pi_PixelsCount);

        // 3 channels * 2 bytes per channel
        memcpy(pio_pDestRawData, pi_pSourceRawData, pi_PixelsCount * 3 * 2);
        };

    HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterV48R16G16B16_V48R16G16B16(*this));
        }
    };
static ConverterV48R16G16B16_V48R16G16B16 s_V48R16G16B16_V48R16G16B16;


//-----------------------------------------------------------------------------
//  s_V24R8G8B8_V48R16G16B16 - Converter
//-----------------------------------------------------------------------------
struct ConverterV24R8G8B8_V48R16G16B16 : public HRPPixelConverter
    {
    DEFINE_T_SUPER(HRPPixelConverter)

    void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        Byte*  pSrc = (Byte*)pi_pSourceRawData;
        unsigned short* pDst = (unsigned short*)pio_pDestRawData;

        // 3 Channels per pixel
        for(size_t i(0); i < pi_PixelsCount*3; i+=3)
            {
            pDst[i]   = CONVERT_8BIT_TO_16BIT(pSrc[i]);
            pDst[i+1] = CONVERT_8BIT_TO_16BIT(pSrc[i+1]);
            pDst[i+2] = CONVERT_8BIT_TO_16BIT(pSrc[i+2]);
            }
        };

    HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterV24R8G8B8_V48R16G16B16(*this));
        }

    };
static ConverterV24R8G8B8_V48R16G16B16 s_V24R8G8B8_V48R16G16B16;

//-----------------------------------------------------------------------------
//  s_V32R8G8B8A8_V48R16B16G16 - Converter
//-----------------------------------------------------------------------------
struct ConverterV32R8G8B8A8_V48R16G16B16 : public HRPPixelConverter
    {
    DEFINE_T_SUPER(HRPPixelConverter)

    void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        Byte*  pSrc = (Byte*)pi_pSourceRawData;
        unsigned short* pDst = (unsigned short*)pio_pDestRawData;

        // 3 Channels per pixel
        for(size_t i(0); i < pi_PixelsCount*3; i+=3)
            {
            pDst[i]   = CONVERT_8BIT_TO_16BIT(pSrc[0]);
            pDst[i+1] = CONVERT_8BIT_TO_16BIT(pSrc[1]);
            pDst[i+2] = CONVERT_8BIT_TO_16BIT(pSrc[2]);

            // Next RGBA
            pSrc+=4;
            }
        };

    virtual void Compose(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        Byte*  pSrc = (Byte*)pi_pSourceRawData;
        unsigned short* pDst = (unsigned short*)pio_pDestRawData;

        // Dst has 3 channel per pixel
        for(size_t i(0); i < pi_PixelsCount*3; i+=3)
            {
            // ((SrcAlpha * (Src - Dst)) / MaxAlpha) + Dst
            pDst[i]   = ((CONVERT_8BIT_TO_16BIT(pSrc[3]) * (int64_t)(CONVERT_8BIT_TO_16BIT(pSrc[0]) - pDst[i]))   / 0xFFFF) + pDst[i];
            pDst[i+1] = ((CONVERT_8BIT_TO_16BIT(pSrc[3]) * (int64_t)(CONVERT_8BIT_TO_16BIT(pSrc[1]) - pDst[i+1])) / 0xFFFF) + pDst[i+1];
            pDst[i+2] = ((CONVERT_8BIT_TO_16BIT(pSrc[3]) * (int64_t)(CONVERT_8BIT_TO_16BIT(pSrc[2]) - pDst[i+2])) / 0xFFFF) + pDst[i+2];

            // Next RGBA
            pSrc+=4;
            }
        };

    virtual const short* GetLostChannels() const override
        {
        return m_LostChannels;
        }

    HRPPixelConverter* AllocateCopy() const override {
        return(new ConverterV32R8G8B8A8_V48R16G16B16(*this));
        }

private:

    static short m_LostChannels[];
    };
short ConverterV32R8G8B8A8_V48R16G16B16::m_LostChannels[] = {3, -1};
static ConverterV32R8G8B8A8_V48R16G16B16 s_V32R8G8B8A8_V48R16G16B16;

//-----------------------------------------------------------------------------
//  s_V48R16G16B16_V24R8G8B8 - Converter
//-----------------------------------------------------------------------------
struct ConverterV48R16G16B16_V24R8G8B8 : public HRPPixelConverter
    {
    DEFINE_T_SUPER(HRPPixelConverter)

    void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        unsigned short* pSrc = (unsigned short*)pi_pSourceRawData;
        Byte*  pDst = (Byte*)pio_pDestRawData;

        // 3 Channels per pixel.
        for(size_t i(0); i < pi_PixelsCount*3; i+=3)
            {
            pDst[i]   = CONVERT_16BIT_TO_8BIT(pSrc[i]);
            pDst[i+1] = CONVERT_16BIT_TO_8BIT(pSrc[i+1]);
            pDst[i+2] = CONVERT_16BIT_TO_8BIT(pSrc[i+2]);
            }
        };

    HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterV48R16G16B16_V24R8G8B8(*this));
        }
    };
static ConverterV48R16G16B16_V24R8G8B8 s_V48R16G16B16_V24R8G8B8;

//-----------------------------------------------------------------------------
//  s_V48R16G16B16_V32R8G8B8A8 - Converter
//-----------------------------------------------------------------------------
struct ConverterV48R16G16B16_V32R8G8B8A8 : public HRPPixelConverter
    {
    DEFINE_T_SUPER(HRPPixelConverter)

    void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        unsigned short* pSrc = (unsigned short*)pi_pSourceRawData;
        Byte*  pDst = (Byte*)pio_pDestRawData;

        // 4 Channels per pixel.
        for(size_t i(0); i < pi_PixelsCount*3; i+=3)
            {
            pDst[0] = CONVERT_16BIT_TO_8BIT(pSrc[i]);
            pDst[1] = CONVERT_16BIT_TO_8BIT(pSrc[i+1]);
            pDst[2] = CONVERT_16BIT_TO_8BIT(pSrc[i+2]);
            pDst[3] = 0xFF;

            // Next RGBA pixel
            pDst+=4;
            }
        };

    HRPPixelConverter* AllocateCopy() const override {
        return(new ConverterV48R16G16B16_V32R8G8B8A8(*this));
        }
    };
static ConverterV48R16G16B16_V32R8G8B8A8 s_V48R16G16B16_V32R8G8B8A8;

//-----------------------------------------------------------------------------
//  Dictionnary of converters from other pixel types
//-----------------------------------------------------------------------------
struct V48R16G16B16ConvertersFrom : public MapHRPPixelTypeToConverter
    {
    V48R16G16B16ConvertersFrom() : MapHRPPixelTypeToConverter ()
        {
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV48R16G16B16::CLASS_ID, &s_V48R16G16B16_V48R16G16B16));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV24R8G8B8::CLASS_ID, &s_V24R8G8B8_V48R16G16B16));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV32R8G8B8A8::CLASS_ID, &s_V32R8G8B8A8_V48R16G16B16));
        };
    };

//-----------------------------------------------------------------------------
//  Dictionnary of converters from other pixel types
//-----------------------------------------------------------------------------
struct V48R16G16B16ConvertersTo : public MapHRPPixelTypeToConverter
    {
    V48R16G16B16ConvertersTo() : MapHRPPixelTypeToConverter ()
        {
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV48R16G16B16::CLASS_ID, &s_V48R16G16B16_V48R16G16B16));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV24R8G8B8::CLASS_ID, &s_V48R16G16B16_V24R8G8B8));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV32R8G8B8A8::CLASS_ID, &s_V48R16G16B16_V32R8G8B8A8));
        };
    };

//-----------------------------------------------------------------------------
// Default Constructor
//-----------------------------------------------------------------------------
HRPPixelTypeV48R16G16B16::HRPPixelTypeV48R16G16B16()
    : HRPPixelTypeRGB(16,16,16,0,0)
    {
    }

//-----------------------------------------------------------------------------
// Copy constructor
//-----------------------------------------------------------------------------
HRPPixelTypeV48R16G16B16::HRPPixelTypeV48R16G16B16(const HRPPixelTypeV48R16G16B16& pi_rObj)
    : HRPPixelTypeRGB(pi_rObj)
    {
    }

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
HRPPixelTypeV48R16G16B16::~HRPPixelTypeV48R16G16B16()
    {
    }

//-----------------------------------------------------------------------------
// Clone method
//-----------------------------------------------------------------------------
HPMPersistentObject* HRPPixelTypeV48R16G16B16::Clone() const
    {
    return new HRPPixelTypeV48R16G16B16(*this);
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
unsigned short HRPPixelTypeV48R16G16B16::CountValueBits() const
    {
    return 48;
    }

//-----------------------------------------------------------------------------
// HasConverterFrom
//-----------------------------------------------------------------------------
const HRPPixelConverter* HRPPixelTypeV48R16G16B16::HasConverterFrom(const HRPPixelType* pi_pPixelTypeFrom) const
    {
    HFCMonitor Monitor(HRPPixelType::s_ConverterAccess);

    // This declaration needs to remain local in order to prevent initialization problems
    static struct V48R16G16B16ConvertersFrom s_ConvertersFrom;

    MapHRPPixelTypeToConverter::const_iterator Itr;
    Itr = s_ConvertersFrom.find (pi_pPixelTypeFrom->GetClassID());

    return ((Itr == s_ConvertersFrom.end()) ? 0 : (*Itr).second);
    }

//-----------------------------------------------------------------------------
// HasConverterTo
//-----------------------------------------------------------------------------
const HRPPixelConverter* HRPPixelTypeV48R16G16B16::HasConverterTo(const HRPPixelType* pi_pPixelTypeTo) const
    {
    HFCMonitor Monitor(HRPPixelType::s_ConverterAccess);

    // This declaration needs to remain local in order to prevent initialization problems
    static struct V48R16G16B16ConvertersTo s_ConvertersTo;

    MapHRPPixelTypeToConverter::const_iterator Itr;
    Itr = s_ConvertersTo.find (pi_pPixelTypeTo->GetClassID());

    return ((Itr == s_ConvertersTo.end()) ? 0 : (*Itr).second);
    }


