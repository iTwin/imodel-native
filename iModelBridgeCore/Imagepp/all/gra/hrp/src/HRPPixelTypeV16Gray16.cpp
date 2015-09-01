//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrp/src/HRPPixelTypeV16Gray16.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for class HRPPixelTypeV16Gray16
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HRPPixelTypeV16Gray16.h>
#include <Imagepp/all/h/HRPChannelOrgGray.h>

#include <Imagepp/all/h/HRPPixelTypeV32R8G8B8A8.h>
#include <Imagepp/all/h/HRPPixelTypeV24R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeV48R16G16B16.h>
#include <Imagepp/all/h/HRPPixelTypeV64R16G16B16A16.h>
#include <Imagepp/all/h/HFCMath.h>

HPM_REGISTER_CLASS(HRPPixelTypeV16Gray16, HRPPixelTypeGray)

// STL typeDef
typedef map<HCLASS_ID, HRPPixelConverter*, less<HCLASS_ID>, allocator<HRPPixelConverter*> >
MapHRPPixelTypeToConverter;

//-----------------------------------------------------------------------------
//  s_V16Gray16_V16Gray16 - Converter between HRPPixelTypeV16Gray16
//-----------------------------------------------------------------------------
struct ConverterV16Gray16_V16Gray16 : public HRPPixelConverter
    {
    DEFINE_T_SUPER(HRPPixelConverter)

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        memcpy(pio_pDestRawData, pi_pSourceRawData, pi_PixelsCount*2);
        };

    virtual HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterV16Gray16_V16Gray16(*this));
        }
    };
struct ConverterV16Gray16_V16Gray16 s_V16Gray16_V16Gray16;

//-----------------------------------------------------------------------------
//  s_V24R8G8B8_V16Gray16 - Converter
//-----------------------------------------------------------------------------
struct ConverterV24R8G8B8_V16Gray16 : public HRPPixelConverter
    {
    DEFINE_T_SUPER(HRPPixelConverter)

    void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        Byte* pSrc  = (Byte*)pi_pSourceRawData;
        unsigned short* pDest = (unsigned short*)pio_pDestRawData;

        for(size_t i(0); i < pi_PixelsCount; ++i)
            {
            pDest[i] = CONVERT_8BIT_TO_16BIT((Byte)(pSrc[0] * REDFACTOR + pSrc[1] * GREENFACTOR + pSrc[2] * BLUEFACTOR));

            // next RGB
            pSrc+=3;
            }
        };

    HRPPixelConverter* AllocateCopy() const override {
        return(new ConverterV24R8G8B8_V16Gray16(*this));
        }
    };
struct ConverterV24R8G8B8_V16Gray16 s_V24R8G8B8_V16Gray16;

//-----------------------------------------------------------------------------
//  s_V32R8G8B8A8_V16Gray16 - Converter
//-----------------------------------------------------------------------------
struct ConverterV32R8G8B8A8_V16Gray16 : public HRPPixelConverter
    {
public:
    DEFINE_T_SUPER(HRPPixelConverter)

    void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        Byte*  pSrc  = (Byte*)pi_pSourceRawData;
        unsigned short* pDest = (unsigned short*)pio_pDestRawData;

        for(size_t i(0); i < pi_PixelsCount; ++i)
            {
            pDest[i] = CONVERT_8BIT_TO_16BIT((Byte)(pSrc[0] * REDFACTOR + pSrc[1] * GREENFACTOR + pSrc[2] * BLUEFACTOR));

            // next RGBA
            pSrc+=4;
            }
        };

    virtual void Compose(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        Byte*  pSrc = (Byte*)pi_pSourceRawData;
        unsigned short* pDest = (unsigned short*)pio_pDestRawData;

        for(size_t i(0); i < pi_PixelsCount; ++i)
            {
            unsigned short grayValue(CONVERT_8BIT_TO_16BIT((Byte)(pSrc[0] * REDFACTOR + pSrc[1] * GREENFACTOR + pSrc[2] * BLUEFACTOR)));

            // ((Src * SrcAlpha) + (Dst * (MaxAlpha - SrcAlpha))) / MaxAlpha
            pDest[i] = ((grayValue * (int64_t)CONVERT_8BIT_TO_16BIT(pSrc[3])) + (pDest[i] * (int64_t)(0xFFFF - CONVERT_8BIT_TO_16BIT(pSrc[3])))) / 0xFFFF;

            // next RGBA
            pSrc+=4;
            }
        };

    virtual const short* GetLostChannels() const override
        {
        return m_LostChannels;
        };

    HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterV32R8G8B8A8_V16Gray16(*this));
        }

private:

    static short m_LostChannels[];
    };
short ConverterV32R8G8B8A8_V16Gray16::m_LostChannels[] = {3, -1};
static ConverterV32R8G8B8A8_V16Gray16 s_V32R8G8B8A8_V16Gray16;

//-----------------------------------------------------------------------------
//  ConverterV48R16G16B16_V16Gray16 - Converter
//-----------------------------------------------------------------------------
struct ConverterV48R16G16B16_V16Gray16 : public HRPPixelConverter
    {
public:
    DEFINE_T_SUPER(HRPPixelConverter)

    void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        unsigned short* pSrc  = (unsigned short*)pi_pSourceRawData;
        unsigned short* pDest = (unsigned short*)pio_pDestRawData;

        for(size_t i(0); i < pi_PixelsCount; ++i)
            {
            pDest[i] = (unsigned short)(pSrc[0] * REDFACTOR + pSrc[1] * GREENFACTOR + pSrc[2] * BLUEFACTOR);

            // next RGB
            pSrc+=3;
            }
        };

    HRPPixelConverter* AllocateCopy() const override{
        return(new ConverterV48R16G16B16_V16Gray16(*this));
        }

    };
struct ConverterV48R16G16B16_V16Gray16 s_V48R16G16B16_V16Gray16;

//-----------------------------------------------------------------------------
//  ConverterV64R16G16B16A16_V16Gray16 - Converter
//-----------------------------------------------------------------------------
struct ConverterV64R16G16B16A16_V16Gray16 : public HRPPixelConverter
    {
public:
    DEFINE_T_SUPER(HRPPixelConverter)

    void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        unsigned short* pSrc  = (unsigned short*)pi_pSourceRawData;
        unsigned short* pDest = (unsigned short*)pio_pDestRawData;

        for(size_t i(0); i < pi_PixelsCount; ++i)
            {
            pDest[i] = (unsigned short)(pSrc[0] * REDFACTOR + pSrc[1] * GREENFACTOR + pSrc[2] * BLUEFACTOR);

            // next RGBA
            pSrc+=4;
            }
        };

    virtual void Compose(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        unsigned short* pSrc  = (unsigned short*)pi_pSourceRawData;
        unsigned short* pDest = (unsigned short*)pio_pDestRawData;

        for(size_t i(0); i < pi_PixelsCount; ++i)
            {
            unsigned short grayValue((unsigned short)(pSrc[0] * REDFACTOR + pSrc[1] * GREENFACTOR + pSrc[2] * BLUEFACTOR));

            // ((Src * SrcAlpha) + (Dst * (MaxAlpha - SrcAlpha))) / MaxAlpha
            pDest[i] = (((int64_t)grayValue * pSrc[3]) + ((int64_t)pDest[i] * (0xFFFF - pSrc[3]))) / 0xFFFF;

            // next RGBA
            pSrc+=4;
            }
        };

    virtual const short* GetLostChannels() const override
        {
        return m_LostChannels;
        };

    HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterV64R16G16B16A16_V16Gray16(*this));
        }

private:

    static short m_LostChannels[];
    };
short ConverterV64R16G16B16A16_V16Gray16::m_LostChannels[] = {3, -1};
static ConverterV64R16G16B16A16_V16Gray16 s_V64R16G16B16A16_V16Gray16;

//-----------------------------------------------------------------------------
//  s_V16Gray16_V24R8G8B8 - Converter
//-----------------------------------------------------------------------------
struct ConverterV16Gray16_V24R8G8B8 : public HRPPixelConverter
    {
    DEFINE_T_SUPER(HRPPixelConverter)

    void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        unsigned short* pSrc = (unsigned short*)pi_pSourceRawData;
        Byte*  pDst = (Byte*)pio_pDestRawData;

        for(size_t i(0); i < pi_PixelsCount; ++i)
            {
            pDst[0] = pDst[1] = pDst[2] = CONVERT_16BIT_TO_8BIT(pSrc[i]);

            // Next RGB
            pDst+=3;
            }
        };

    HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterV16Gray16_V24R8G8B8(*this));
        }
    };
struct ConverterV16Gray16_V24R8G8B8 s_V16Gray16_V24R8G8B8;


//-----------------------------------------------------------------------------
//  s_V16Gray16_V32R8G8B8A8 - Converter
//-----------------------------------------------------------------------------
struct ConverterV16Gray16_V32R8G8B8A8 : public HRPPixelConverter
    {
    DEFINE_T_SUPER(HRPPixelConverter)

    void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        unsigned short* pSrc = (unsigned short*)pi_pSourceRawData;
        Byte*  pDst = (Byte*)pio_pDestRawData;

        for(size_t i(0); i < pi_PixelsCount; ++i)
            {
            pDst[0] = pDst[1] = pDst[2] = CONVERT_16BIT_TO_8BIT(pSrc[i]);
            pDst[3] = 0xFF;

            // Next RGBA
            pDst+=4;
            }
        };

    HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterV16Gray16_V32R8G8B8A8(*this));
        }
    };
static ConverterV16Gray16_V32R8G8B8A8 s_V16Gray16_V32R8G8B8A8;

//-----------------------------------------------------------------------------
//  s_V16Gray16_V48R16G16B16 - Converter
//-----------------------------------------------------------------------------
struct ConverterV16Gray16_V48R16G16B16 : public HRPPixelConverter
    {
    DEFINE_T_SUPER(HRPPixelConverter)

    void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        unsigned short*  pSrc  = (unsigned short*) pi_pSourceRawData;
        unsigned short*  pDest = (unsigned short*) pio_pDestRawData;

        for(size_t i(0); i < pi_PixelsCount; ++i)
            {
            pDest[0] = pDest[1] = pDest[2] = pSrc[i];

            // Next RGB
            pDest+=3;
            }
        };

    HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterV16Gray16_V48R16G16B16(*this));
        }
    };
struct ConverterV16Gray16_V48R16G16B16 s_V16Gray16_V48R16G16B16;

//-----------------------------------------------------------------------------
//  s_V16Gray16_V64R16G16B16A16 - Converter
//-----------------------------------------------------------------------------
struct ConverterV16Gray16_V64R16G16B16A16 : public HRPPixelConverter
    {
    DEFINE_T_SUPER(HRPPixelConverter)

    void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        unsigned short*  pSrc  = (unsigned short*) pi_pSourceRawData;
        unsigned short*  pDest = (unsigned short*) pio_pDestRawData;

        for(size_t i(0); i < pi_PixelsCount; ++i)
            {
            pDest[0] = pDest[1] = pDest[2] = pSrc[i];
            pDest[3] = 0xFFFF;

            // Next RGBA
            pDest+=4;
            }
        };

    HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterV16Gray16_V64R16G16B16A16(*this));
        }
    };
struct ConverterV16Gray16_V64R16G16B16A16 s_V16Gray16_V64R16G16B16A16;

//-----------------------------------------------------------------------------
//  Dictionnary of converters from other pixel types
//-----------------------------------------------------------------------------
struct V16Gray16ConvertersFrom : public MapHRPPixelTypeToConverter
    {
    V16Gray16ConvertersFrom() : MapHRPPixelTypeToConverter()
        {
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV16Gray16::CLASS_ID, &s_V16Gray16_V16Gray16));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV24R8G8B8::CLASS_ID, &s_V24R8G8B8_V16Gray16));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV32R8G8B8A8::CLASS_ID, &s_V32R8G8B8A8_V16Gray16));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV48R16G16B16::CLASS_ID, &s_V48R16G16B16_V16Gray16));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV64R16G16B16A16::CLASS_ID, &s_V64R16G16B16A16_V16Gray16));
        };
    };

//-----------------------------------------------------------------------------
//  Dictionnary of converters from other pixel types
//-----------------------------------------------------------------------------
struct V16Gray16ConvertersTo : public MapHRPPixelTypeToConverter
    {
    V16Gray16ConvertersTo()
        {
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV16Gray16::CLASS_ID, &s_V16Gray16_V16Gray16));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV32R8G8B8A8::CLASS_ID, &s_V16Gray16_V32R8G8B8A8));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV24R8G8B8::CLASS_ID, &s_V16Gray16_V24R8G8B8));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV48R16G16B16::CLASS_ID, &s_V16Gray16_V48R16G16B16));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV64R16G16B16A16::CLASS_ID, &s_V16Gray16_V64R16G16B16A16));
        };
    };

//-----------------------------------------------------------------------------
// Constructor for 16 bits grayscale
//-----------------------------------------------------------------------------
HRPPixelTypeV16Gray16::HRPPixelTypeV16Gray16()
    : HRPPixelTypeGray(16,0)
    {
    }

//-----------------------------------------------------------------------------
// Copy constructor
//-----------------------------------------------------------------------------
HRPPixelTypeV16Gray16::HRPPixelTypeV16Gray16(const HRPPixelTypeV16Gray16& pi_rObj)
    : HRPPixelTypeGray(pi_rObj)
    {
    }

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
HRPPixelTypeV16Gray16::~HRPPixelTypeV16Gray16()
    {
    }

//-----------------------------------------------------------------------------
// Clone method
//-----------------------------------------------------------------------------
HPMPersistentObject* HRPPixelTypeV16Gray16::Clone() const
    {
    return new HRPPixelTypeV16Gray16(*this);
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
unsigned short HRPPixelTypeV16Gray16::CountValueBits() const
    {
    return 16;
    }

//-----------------------------------------------------------------------------
// HasConverterFrom
//-----------------------------------------------------------------------------
const HRPPixelConverter* HRPPixelTypeV16Gray16::HasConverterFrom(const HRPPixelType* pi_pPixelTypeFrom) const
    {
    HFCMonitor Monitor(HRPPixelType::s_ConverterAccess);

    // This declaration needs to remain local in order to prevent initialization problems
    static struct V16Gray16ConvertersFrom s_ConvertersFrom;

    MapHRPPixelTypeToConverter::const_iterator Itr;
    Itr = s_ConvertersFrom.find (pi_pPixelTypeFrom->GetClassID());

    return ((Itr == s_ConvertersFrom.end()) ? 0 : (*Itr).second);
    }

//-----------------------------------------------------------------------------
// HasConverterTo
//-----------------------------------------------------------------------------
const HRPPixelConverter* HRPPixelTypeV16Gray16::HasConverterTo(const HRPPixelType* pi_pPixelTypeTo) const
    {
    HFCMonitor Monitor(HRPPixelType::s_ConverterAccess);

    // This declaration needs to remain local in order to prevent initialization problems
    static struct V16Gray16ConvertersTo s_ConvertersTo;

    MapHRPPixelTypeToConverter::const_iterator Itr;
    Itr = s_ConvertersTo.find (pi_pPixelTypeTo->GetClassID());

    return ((Itr == s_ConvertersTo.end()) ? 0 : (*Itr).second);
    }


