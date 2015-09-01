//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrp/src/HRPPixelTypeV64R16G16B16X16.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for class HRPPixelTypeV64R16G16B16X16
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HRPPixelTypeV64R16G16B16X16.h>
#include <Imagepp/all/h/HRPPixelTypeV24R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeV32R8G8B8A8.h>
#include <Imagepp/all/h/HRPPixelTypeV48R16G16B16.h>
#include <Imagepp/all/h/HRPPixelTypeV64R16G16B16A16.h>
#include <Imagepp/all/h/HFCMath.h>

HPM_REGISTER_CLASS(HRPPixelTypeV64R16G16B16X16, HRPPixelTypeRGB)

// STL typeDef
typedef map<HCLASS_ID, HRPPixelConverter*, less<HCLASS_ID>, allocator<HRPPixelConverter*> >
MapHRPPixelTypeToConverter;

//-----------------------------------------------------------------------------
//  ConverterV64R16G16B16X16_V64R16G16B16X16 - Converter
//-----------------------------------------------------------------------------
struct ConverterV64R16G16B16X16_V64R16G16B16X16 : public HRPPixelConverter
    {
    DEFINE_T_SUPER(HRPPixelConverter)

    void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        HPRECONDITION(UINT_MAX / 8 > pi_PixelsCount);

        // 4 channels and 2 bytes per channel
        memcpy(pio_pDestRawData, pi_pSourceRawData, pi_PixelsCount * 4 * 2);
        };

    HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterV64R16G16B16X16_V64R16G16B16X16(*this));
        }
    };
static ConverterV64R16G16B16X16_V64R16G16B16X16 s_V64R16G16B16X16_V64R16G16B16X16;

//-----------------------------------------------------------------------------
//  s_V24R8G8B8_V64R16G16B16X16 - Converter
//-----------------------------------------------------------------------------
struct ConverterV24R8G8B8_V64R16G16B16X16 : public HRPPixelConverter
    {
    DEFINE_T_SUPER(HRPPixelConverter)

    void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        Byte*  pSrc = (Byte*)pi_pSourceRawData;
        unsigned short* pDst = (unsigned short*)pio_pDestRawData;

        for(size_t i(0); i < pi_PixelsCount*3; i+=3)
            {
            pDst[0] = CONVERT_8BIT_TO_16BIT(pSrc[i]);
            pDst[1] = CONVERT_8BIT_TO_16BIT(pSrc[i+1]);
            pDst[2] = CONVERT_8BIT_TO_16BIT(pSrc[i+2]);
            pDst[3] = 0xFFFF;

            // NextRGBA
            pDst+=4;
            }
        };

    HRPPixelConverter* AllocateCopy() const override {
        return(new ConverterV24R8G8B8_V64R16G16B16X16(*this));
        }

    };
static ConverterV24R8G8B8_V64R16G16B16X16 s_V24R8G8B8_V64R16G16B16X16;

//-----------------------------------------------------------------------------
//  s_V32R8G8B8A8_V64R16G16B16X16 - Converter
//-----------------------------------------------------------------------------
struct ConverterV32R8G8B8A8_V64R16G16B16X16 : public HRPPixelConverter
    {
    DEFINE_T_SUPER(HRPPixelConverter)

    void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        Byte*  pSrc = (Byte*)pi_pSourceRawData;
        unsigned short* pDst = (unsigned short*)pio_pDestRawData;

        for(size_t i(0); i < pi_PixelsCount*4; i+=4)
            {
            pDst[i]   = CONVERT_8BIT_TO_16BIT(pSrc[0]);
            pDst[i+1] = CONVERT_8BIT_TO_16BIT(pSrc[i+1]);
            pDst[i+2] = CONVERT_8BIT_TO_16BIT(pSrc[i+2]);
            pDst[i+3] = 0xFFFF;
            }
        };

    virtual void Compose(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        Byte*  pSrc = (Byte*)pi_pSourceRawData;
        unsigned short* pDst = (unsigned short*)pio_pDestRawData;

        // 4 channel per pixel
        for(size_t i(0); i < pi_PixelsCount*4; i+=4)
            {
            // ((SrcAlpha * (Src - Dst)) / MaxAlpha) + Dst

            pDst[i]   = ((CONVERT_8BIT_TO_16BIT(pSrc[i+3]) * (int64_t)(CONVERT_8BIT_TO_16BIT(pSrc[i])   - pDst[i]))   / 0xFFFF) + pDst[i];
            pDst[i+1] = ((CONVERT_8BIT_TO_16BIT(pSrc[i+3]) * (int64_t)(CONVERT_8BIT_TO_16BIT(pSrc[i+1]) - pDst[i+1])) / 0xFFFF) + pDst[i+1];
            pDst[i+2] = ((CONVERT_8BIT_TO_16BIT(pSrc[i+3]) * (int64_t)(CONVERT_8BIT_TO_16BIT(pSrc[i+2]) - pDst[i+2])) / 0xFFFF) + pDst[i+2];
            pDst[i+3] = 0xFFFF;
            }
        };

    virtual const short* GetLostChannels() const override
        {
        return m_LostChannels;
        }

    HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterV32R8G8B8A8_V64R16G16B16X16(*this));
        }

private:

    static short m_LostChannels[];
    };
short ConverterV32R8G8B8A8_V64R16G16B16X16::m_LostChannels[] = {3, -1};
static ConverterV32R8G8B8A8_V64R16G16B16X16 s_V32R8G8B8A8_V64R16G16B16X16;

//-----------------------------------------------------------------------------
//  s_V48R16G16B16_V64R16G16B16X16 - Converter
//-----------------------------------------------------------------------------
struct ConverterV48R16G16B16_V64R16G16B16X16 : public HRPPixelConverter
    {
    DEFINE_T_SUPER(HRPPixelConverter)

    void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        unsigned short* pSrc = (unsigned short*)pi_pSourceRawData;
        unsigned short* pDst = (unsigned short*)pio_pDestRawData;

        for(size_t i(0); i < pi_PixelsCount*3; i+=3)
            {
            pDst[0] = pSrc[i];
            pDst[1] = pSrc[i+1];
            pDst[2] = pSrc[i+2];
            pDst[3] = 0xFFFF;

            // NextRGBA
            pDst+=4;
            }
        };

    HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterV48R16G16B16_V64R16G16B16X16(*this));
        }

    };
static ConverterV48R16G16B16_V64R16G16B16X16 s_V48R16G16B16_V64R16G16B16X16;


//-----------------------------------------------------------------------------
//  ConverterV64R16G16B16A16_V64R16G16B16X16 - Converter
//-----------------------------------------------------------------------------
struct ConverterV64R16G16B16A16_V64R16G16B16X16 : public HRPPixelConverter
    {
    DEFINE_T_SUPER(HRPPixelConverter)

    void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        unsigned short* pSrc = (unsigned short*)pi_pSourceRawData;
        unsigned short* pDst = (unsigned short*)pio_pDestRawData;

        for(size_t i(0); i < pi_PixelsCount*4; i+=4)
            {
            pDst[i]   = pSrc[i];
            pDst[i+1] = pSrc[i+1];
            pDst[i+2] = pSrc[i+2];
            pDst[i+3] = 0xFFFF;
            }
        };

    virtual void Compose(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        unsigned short* pSrc = (unsigned short*)pi_pSourceRawData;
        unsigned short* pDst = (unsigned short*)pio_pDestRawData;

        // 4 channel per pixel
        for(size_t i(0); i < pi_PixelsCount*4; i+=4)
            {
            // ((SrcAlpha * (Src - Dst)) / MaxAlpha) + Dst
            pDst[i]   = ((pSrc[i+3] * (int64_t)(pSrc[i]   - pDst[i]))   / 0xFFFF) + pDst[i];
            pDst[i+1] = ((pSrc[i+3] * (int64_t)(pSrc[i+1] - pDst[i+1])) / 0xFFFF) + pDst[i+1];
            pDst[i+2] = ((pSrc[i+3] * (int64_t)(pSrc[i+2] - pDst[i+2])) / 0xFFFF) + pDst[i+2];
            pDst[i+3] = 0xFFFF;
            }
        };

    virtual const short* GetLostChannels() const override
        {
        return m_LostChannels;
        }

    HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterV64R16G16B16A16_V64R16G16B16X16(*this));
        }

private:

    static short m_LostChannels[];
    };
short ConverterV64R16G16B16A16_V64R16G16B16X16::m_LostChannels[] = {3, -1};
static ConverterV64R16G16B16A16_V64R16G16B16X16 s_V64R16G16B16A16_V64R16G16B16X16;

//-----------------------------------------------------------------------------
//  s_V64R16G16B16X16_V24R8G8B8 - Converter
//-----------------------------------------------------------------------------
struct ConverterV64R16G16B16X16_V24R8G8B8 : public HRPPixelConverter
    {
    DEFINE_T_SUPER(HRPPixelConverter)

    void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        unsigned short* pSrc = (unsigned short*)pi_pSourceRawData;
        Byte*  pDst = (Byte*)pio_pDestRawData;

        for(size_t i(0); i < pi_PixelsCount*3; i+=3)
            {
            pDst[i]   = CONVERT_16BIT_TO_8BIT(pSrc[0]);
            pDst[i+1] = CONVERT_16BIT_TO_8BIT(pSrc[1]);
            pDst[i+2] = CONVERT_16BIT_TO_8BIT(pSrc[2]);

            // Next RGBX
            pSrc+=4;
            }
        };

    HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterV64R16G16B16X16_V24R8G8B8(*this));
        }
    };
static ConverterV64R16G16B16X16_V24R8G8B8 s_V64R16G16B16X16_V24R8G8B8;

//-----------------------------------------------------------------------------
//  s_V64R16G16B16X16_V32R8G8B8A8 - Converter
//-----------------------------------------------------------------------------
struct ConverterV64R16G16B16X16_V32R8G8B8A8 : public HRPPixelConverter
    {
    DEFINE_T_SUPER(HRPPixelConverter)

    void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        unsigned short* pSrc = (unsigned short*)pi_pSourceRawData;
        Byte*  pDst = (Byte*)pio_pDestRawData;

        for(size_t i(0); i < pi_PixelsCount*4; i+=4)
            {
            pDst[i]   = CONVERT_16BIT_TO_8BIT(pSrc[i]);
            pDst[i+1] = CONVERT_16BIT_TO_8BIT(pSrc[i+1]);
            pDst[i+2] = CONVERT_16BIT_TO_8BIT(pSrc[i+2]);
            pDst[i+3] = 0xFF;
            }
        };

    HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterV64R16G16B16X16_V32R8G8B8A8(*this));
        }
    };
static ConverterV64R16G16B16X16_V32R8G8B8A8 s_V64R16G16B16X16_V32R8G8B8A8;

//-----------------------------------------------------------------------------
//  ConverterV64R16G16B16X16_V48R16G16B16 - Converter
//-----------------------------------------------------------------------------
struct ConverterV64R16G16B16X16_V48R16G16B16 : public HRPPixelConverter
    {
    DEFINE_T_SUPER(HRPPixelConverter)

    void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        unsigned short* pSrc = (unsigned short*)pi_pSourceRawData;
        unsigned short* pDst = (unsigned short*)pio_pDestRawData;

        for(size_t i(0); i < pi_PixelsCount*3; i+=3)
            {
            pDst[i]   = pSrc[0];
            pDst[i+1] = pSrc[1];
            pDst[i+2] = pSrc[2];

            // Next RGBX
            pSrc+=4;
            }
        };

    HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterV64R16G16B16X16_V48R16G16B16(*this));
        }
    };
static ConverterV64R16G16B16X16_V48R16G16B16 s_V64R16G16B16X16_V48R16G16B16;

//-----------------------------------------------------------------------------
//  ConverterV64R16G16B16X16_V64R16G16B16A16 - Converter
//-----------------------------------------------------------------------------
struct ConverterV64R16G16B16X16_V64R16G16B16A16 : public HRPPixelConverter
    {
    DEFINE_T_SUPER(HRPPixelConverter)

    void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        unsigned short* pSrc = (unsigned short*)pi_pSourceRawData;
        unsigned short* pDst = (unsigned short*)pio_pDestRawData;

        for(size_t i(0); i < pi_PixelsCount*4; i+=4)
            {
            pDst[i]   = pSrc[i];
            pDst[i+1] = pSrc[i+1];
            pDst[i+2] = pSrc[i+2];
            pDst[i+3] = 0xFFFF;
            }
        };

    HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterV64R16G16B16X16_V64R16G16B16A16(*this));
        }
    };
static ConverterV64R16G16B16X16_V64R16G16B16A16 s_V64R16G16B16X16_V64R16G16B16A16;

//-----------------------------------------------------------------------------
//  Dictionnary of converters from other pixel types
//-----------------------------------------------------------------------------
struct V64R16G16B16X16ConvertersFrom : public MapHRPPixelTypeToConverter
    {
    V64R16G16B16X16ConvertersFrom() : MapHRPPixelTypeToConverter ()
        {
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV64R16G16B16X16::CLASS_ID, &s_V64R16G16B16X16_V64R16G16B16X16));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV24R8G8B8::CLASS_ID, &s_V24R8G8B8_V64R16G16B16X16));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV32R8G8B8A8::CLASS_ID, &s_V32R8G8B8A8_V64R16G16B16X16));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV48R16G16B16::CLASS_ID, &s_V48R16G16B16_V64R16G16B16X16));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV64R16G16B16A16::CLASS_ID, &s_V64R16G16B16A16_V64R16G16B16X16));
        };
    };

//-----------------------------------------------------------------------------
//  Dictionnary of converters from other pixel types
//-----------------------------------------------------------------------------
struct V64R16G16B16X16ConvertersTo : public MapHRPPixelTypeToConverter
    {
    V64R16G16B16X16ConvertersTo() : MapHRPPixelTypeToConverter ()
        {
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV64R16G16B16X16::CLASS_ID, &s_V64R16G16B16X16_V64R16G16B16X16));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV24R8G8B8::CLASS_ID, &s_V64R16G16B16X16_V24R8G8B8));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV32R8G8B8A8::CLASS_ID, &s_V64R16G16B16X16_V32R8G8B8A8));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV48R16G16B16::CLASS_ID, &s_V64R16G16B16X16_V48R16G16B16));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV64R16G16B16A16::CLASS_ID, &s_V64R16G16B16X16_V64R16G16B16A16));
        };
    };

//-----------------------------------------------------------------------------
// Default Constructor
//-----------------------------------------------------------------------------
HRPPixelTypeV64R16G16B16X16::HRPPixelTypeV64R16G16B16X16()
    : HRPPixelTypeRGB(16,16,16,16,0,false)
    {
    }

//-----------------------------------------------------------------------------
// Copy constructor
//-----------------------------------------------------------------------------
HRPPixelTypeV64R16G16B16X16::HRPPixelTypeV64R16G16B16X16(const HRPPixelTypeV64R16G16B16X16& pi_rObj)
    : HRPPixelTypeRGB(pi_rObj)
    {
    }

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
HRPPixelTypeV64R16G16B16X16::~HRPPixelTypeV64R16G16B16X16()
    {
    }

//-----------------------------------------------------------------------------
// Clone method
//-----------------------------------------------------------------------------
HPMPersistentObject* HRPPixelTypeV64R16G16B16X16::Clone() const
    {
    return new HRPPixelTypeV64R16G16B16X16(*this);
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
unsigned short HRPPixelTypeV64R16G16B16X16::CountValueBits() const
    {
    return 64;
    }

//-----------------------------------------------------------------------------
// HasConverterFrom
//-----------------------------------------------------------------------------
const HRPPixelConverter* HRPPixelTypeV64R16G16B16X16::HasConverterFrom(const HRPPixelType* pi_pPixelTypeFrom) const
    {
    HFCMonitor Monitor(HRPPixelType::s_ConverterAccess);

    // This declaration needs to remain local in order to prevent initialization problems
    static struct V64R16G16B16X16ConvertersFrom s_ConvertersFrom;

    MapHRPPixelTypeToConverter::const_iterator Itr;
    Itr = s_ConvertersFrom.find (pi_pPixelTypeFrom->GetClassID());

    return ((Itr == s_ConvertersFrom.end()) ? 0 : (*Itr).second);
    }

//-----------------------------------------------------------------------------
// HasConverterTo
//-----------------------------------------------------------------------------
const HRPPixelConverter* HRPPixelTypeV64R16G16B16X16::HasConverterTo(const HRPPixelType* pi_pPixelTypeTo) const
    {
    HFCMonitor Monitor(HRPPixelType::s_ConverterAccess);

    // This declaration needs to remain local in order to prevent initialization problems
    static struct V64R16G16B16X16ConvertersTo s_ConvertersTo;

    MapHRPPixelTypeToConverter::const_iterator Itr;
    Itr = s_ConvertersTo.find (pi_pPixelTypeTo->GetClassID());

    return ((Itr == s_ConvertersTo.end()) ? 0 : (*Itr).second);
    }


