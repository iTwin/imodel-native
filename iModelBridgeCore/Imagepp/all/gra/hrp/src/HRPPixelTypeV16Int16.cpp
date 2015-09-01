//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrp/src/HRPPixelTypeV16Int16.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for class HRPPixelTypeV16Int16
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HFCMath.h>
#include <Imagepp/all/h/HRPChannelOrgInt.h>
#include <Imagepp/all/h/HRPPixelTypeV16Int16.h>
#include <Imagepp/all/h/HRPPixelTypeV24R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeV32R8G8B8A8.h>
#include <Imagepp/all/h/HRPPixelTypeV48R16G16B16.h>
#include <Imagepp/all/h/HRPPixelTypeV64R16G16B16A16.h>

HPM_REGISTER_CLASS(HRPPixelTypeV16Int16, HRPPixelType)

// STL typeDef
typedef map<HCLASS_ID, HRPPixelConverter*, less<HCLASS_ID>, allocator<HRPPixelConverter*> >
MapHRPPixelTypeToConverter;

//-----------------------------------------------------------------------------
//  s_V16Int16_V16Int16 - Converter between HRPPixelTypeV16Int16
//-----------------------------------------------------------------------------
struct ConverterV16Int16_V16Int16 : public HRPPixelConverter
    {
    DEFINE_T_SUPER(HRPPixelConverter)

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        memcpy(pio_pDestRawData, pi_pSourceRawData, pi_PixelsCount * 2);
        };

    virtual HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterV16Int16_V16Int16(*this));
        }
    };
struct ConverterV16Int16_V16Int16 s_V16Int16_V16Int16;

//-----------------------------------------------------------------------------
//  s_V24R8G8B8_V16Int16 - Converter
//-----------------------------------------------------------------------------
struct ConverterV24R8G8B8_V16Int16 : public HRPPixelConverter
    {
    DEFINE_T_SUPER(HRPPixelConverter)

    void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        Byte* pSrc  = (Byte*)pi_pSourceRawData;
        short* pDest = (short*)pio_pDestRawData;

        for (size_t i(0); i < pi_PixelsCount; ++i)
            {
            pDest[i] = CONVERT_8BIT_TO_16BIT((Byte)(pSrc[0] * REDFACTOR +
                                                      pSrc[1] * GREENFACTOR +
                                                      pSrc[2] * BLUEFACTOR)) +
                       SHRT_MIN;

            // next RGB
            pSrc += 3;
            }
        };

    HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterV24R8G8B8_V16Int16(*this));
        }
    };
struct ConverterV24R8G8B8_V16Int16 s_V24R8G8B8_V16Int16;

//-----------------------------------------------------------------------------
//  s_V32R8G8B8A8_V16Int16 - Converter
//-----------------------------------------------------------------------------
struct ConverterV32R8G8B8A8_V16Int16 : public HRPPixelConverter
    {
public:
    DEFINE_T_SUPER(HRPPixelConverter)

    void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        Byte* pSrc  = (Byte*)pi_pSourceRawData;
        short* pDest = (short*)pio_pDestRawData;

        for(size_t i(0); i < pi_PixelsCount; ++i)
            {
            pDest[i] = CONVERT_8BIT_TO_16BIT((Byte)(pSrc[0] * REDFACTOR +
                                                      pSrc[1] * GREENFACTOR +
                                                      pSrc[2] * BLUEFACTOR)) + SHRT_MIN;

            // next RGBA
            pSrc+=4;
            }
        };


    virtual void Compose(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        Byte* pSrc  = (Byte*)pi_pSourceRawData;
        short* pDest = (short*)pio_pDestRawData;

        for(size_t i(0); i < pi_PixelsCount; ++i)
            {
            short IntValue(CONVERT_8BIT_TO_16BIT((Byte)(pSrc[0] * REDFACTOR + pSrc[1] * GREENFACTOR + pSrc[2] * BLUEFACTOR)) + SHRT_MIN);

            // ((Src * SrcAlpha) + (Dst * (MaxAlpha - SrcAlpha))) / MaxAlpha
            pDest[i] = ((IntValue * pSrc[3]) + (pDest[i] * (0xFF - pSrc[3]))) / 0xFF;

            // next RGBA
            pSrc+=4;
            }
        };

    virtual const short* GetLostChannels() const override
        {
        return m_LostChannels;
        };

    HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterV32R8G8B8A8_V16Int16(*this));
        }

private:

    static short m_LostChannels[];
    };
short ConverterV32R8G8B8A8_V16Int16::m_LostChannels[] = {3, -1};
static ConverterV32R8G8B8A8_V16Int16 s_V32R8G8B8A8_V16Int16;

//-----------------------------------------------------------------------------
//  ConverterV48R16G16B16_V16Int16 - Converter
//-----------------------------------------------------------------------------
struct ConverterV48R16G16B16_V16Int16 : public HRPPixelConverter
    {
public:
    DEFINE_T_SUPER(HRPPixelConverter)

    void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        unsigned short* pSrc  = (unsigned short*)pi_pSourceRawData;
        short*  pDest = (short*)pio_pDestRawData;

        for(size_t i(0); i < pi_PixelsCount; ++i)
            {
            pDest[i] = (short)((pSrc[0] * REDFACTOR   +
                                 pSrc[1] * GREENFACTOR +
                                 pSrc[2] * BLUEFACTOR) + SHRT_MIN);

            // next RGB
            pSrc+=3;
            }
        };

    HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterV48R16G16B16_V16Int16(*this));
        }

    };
struct ConverterV48R16G16B16_V16Int16 s_V48R16G16B16_V16Int16;

//-----------------------------------------------------------------------------
//  ConverterV64R16G16B16A16_V16Int16 - Converter
//-----------------------------------------------------------------------------
struct ConverterV64R16G16B16A16_V16Int16 : public HRPPixelConverter
    {
public:
    DEFINE_T_SUPER(HRPPixelConverter)

    void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        unsigned short* pSrc  = (unsigned short*)pi_pSourceRawData;
        short*  pDest = (short*)pio_pDestRawData;

        for(size_t i(0); i < pi_PixelsCount; ++i)
            {
            pDest[i] = (short)((pSrc[0] * REDFACTOR   +
                                 pSrc[1] * GREENFACTOR +
                                 pSrc[2] * BLUEFACTOR) + SHRT_MIN);

            // next RGBA
            pSrc+=4;
            }
        };

    virtual void Compose(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        unsigned short* pSrc  = (unsigned short*)pi_pSourceRawData;
        short*  pDest = (short*)pio_pDestRawData;

        for(size_t i(0); i < pi_PixelsCount; ++i)
            {
            short IntValue((short)((pSrc[0] * REDFACTOR   +
                                      pSrc[1] * GREENFACTOR +
                                      pSrc[2] * BLUEFACTOR) + SHRT_MIN));

            // ((Src * SrcAlpha) + (Dst * (MaxAlpha - SrcAlpha))) / MaxAlpha
            pDest[i] = ((IntValue * pSrc[3]) + (pDest[i] * (0xFFFF - pSrc[3]))) / 0xFFFF;

            // next RGBA
            pSrc+=4;
            }
        };

    virtual const short* GetLostChannels() const override
        {
        return m_LostChannels;
        };

    HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterV64R16G16B16A16_V16Int16(*this));
        }

private:

    static short m_LostChannels[];
    };
short ConverterV64R16G16B16A16_V16Int16::m_LostChannels[] = {3, -1};
static ConverterV64R16G16B16A16_V16Int16 s_V64R16G16B16A16_V16Int16;



//-----------------------------------------------------------------------------
//  s_V16Int16_V24R8G8B8 - Converter
//-----------------------------------------------------------------------------
struct ConverterV16Int16_V24R8G8B8 : public HRPPixelConverter
    {
    DEFINE_T_SUPER(HRPPixelConverter)

    void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        short* pSrc = (short*)pi_pSourceRawData;
        Byte* pDst = (Byte*)pio_pDestRawData;

        for(size_t i(0); i < pi_PixelsCount; ++i)
            {
            pDst[0] = pDst[1] = pDst[2] = CONVERT_16BIT_TO_8BIT((pSrc[i] - SHRT_MIN));

            // Next RGB
            pDst+=3;
            }
        };

    HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterV16Int16_V24R8G8B8(*this));
        }
    };
struct ConverterV16Int16_V24R8G8B8 s_V16Int16_V24R8G8B8;

//-----------------------------------------------------------------------------
//  s_V16Int16_V32R8G8B8A8 - Converter
//-----------------------------------------------------------------------------
struct ConverterV16Int16_V32R8G8B8A8 : public HRPPixelConverter
    {
    DEFINE_T_SUPER(HRPPixelConverter)

    void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        short* pSrc = (short*)pi_pSourceRawData;
        Byte* pDst = (Byte*)pio_pDestRawData;

        for(size_t i(0); i < pi_PixelsCount; ++i)
            {
            pDst[0] = pDst[1] = pDst[2] = CONVERT_16BIT_TO_8BIT((pSrc[i] - SHRT_MIN));
            pDst[3] = 0xFF;

            // Next RGBA
            pDst+=4;
            }
        };

    HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterV16Int16_V32R8G8B8A8(*this));
        }
    };
static ConverterV16Int16_V32R8G8B8A8 s_V16Int16_V32R8G8B8A8;

//-----------------------------------------------------------------------------
//  s_V16Int16_V48R16G16B16 - Converter
//-----------------------------------------------------------------------------
struct ConverterV16Int16_V48R16G16B16 : public HRPPixelConverter
    {
    DEFINE_T_SUPER(HRPPixelConverter)

    void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        short*  pSrc  = (short*)pi_pSourceRawData;
        unsigned short* pDest = (unsigned short*)pio_pDestRawData;

        for(size_t i(0); i < pi_PixelsCount; ++i)
            {
            pDest[0] = (unsigned short)(pSrc[i] - SHRT_MIN);
            pDest[1] = pDest[0];
            pDest[2] = pDest[0];

            // Next RGB
            pDest+=3;
            }
        };

    HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterV16Int16_V48R16G16B16(*this));
        }
    };
struct ConverterV16Int16_V48R16G16B16 s_V16Int16_V48R16G16B16;

//-----------------------------------------------------------------------------
//  s_V16Int16_V64R16G16B16A16 - Converter
//-----------------------------------------------------------------------------
struct ConverterV16Int16_V64R16G16B16A16 : public HRPPixelConverter
    {
    DEFINE_T_SUPER(HRPPixelConverter)

    void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        short*  pSrc   = (short*) pi_pSourceRawData;
        unsigned short*  pDest = (unsigned short*) pio_pDestRawData;

        for(size_t i(0); i < pi_PixelsCount; ++i)
            {
            pDest[0] = (unsigned short)(pSrc[i] - SHRT_MIN);
            pDest[1] = pDest[0];
            pDest[2] = pDest[0];
            pDest[3] = 0xFFFF;

            // Next RGBA
            pDest+=4;
            }
        };

    HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterV16Int16_V64R16G16B16A16(*this));
        }
    };
struct ConverterV16Int16_V64R16G16B16A16 s_V16Int16_V64R16G16B16A16;

//-----------------------------------------------------------------------------
//  Dictionnary of converters from other pixel types
//-----------------------------------------------------------------------------
struct V16Int16ConvertersFrom : public MapHRPPixelTypeToConverter
    {
    V16Int16ConvertersFrom() : MapHRPPixelTypeToConverter()
        {
        insert(MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV16Int16::CLASS_ID,        &s_V16Int16_V16Int16));
        insert(MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV24R8G8B8::CLASS_ID,       &s_V24R8G8B8_V16Int16));
        insert(MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV32R8G8B8A8::CLASS_ID,     &s_V32R8G8B8A8_V16Int16));
        insert(MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV48R16G16B16::CLASS_ID,    &s_V48R16G16B16_V16Int16));
        insert(MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV64R16G16B16A16::CLASS_ID, &s_V64R16G16B16A16_V16Int16));
        };
    };

//-----------------------------------------------------------------------------
//  Dictionnary of converters from other pixel types
//-----------------------------------------------------------------------------
struct V16Int16ConvertersTo : public MapHRPPixelTypeToConverter
    {
    V16Int16ConvertersTo()
        {
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV16Int16::CLASS_ID,        &s_V16Int16_V16Int16));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV24R8G8B8::CLASS_ID,       &s_V16Int16_V24R8G8B8));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV32R8G8B8A8::CLASS_ID,     &s_V16Int16_V32R8G8B8A8));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV48R16G16B16::CLASS_ID,    &s_V16Int16_V48R16G16B16));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV64R16G16B16A16::CLASS_ID, &s_V16Int16_V64R16G16B16A16));
        };
    };

//-----------------------------------------------------------------------------
// Routine that cast an Int16 NoDataValue to a double NoDataValue
//-----------------------------------------------------------------------------
static HAutoPtr<double> CastNoDataValuePtr(const int16_t* pi_pNoDataValue)
    {
    HAutoPtr<double> pNoDataValue;

    if (0 != pi_pNoDataValue)
        pNoDataValue = new double( *pi_pNoDataValue );

    return pNoDataValue;
    }

//-----------------------------------------------------------------------------
// Constructor for 16 bits integer
//-----------------------------------------------------------------------------
HRPPixelTypeV16Int16::HRPPixelTypeV16Int16(HRPChannelType::ChannelRole pi_Role,
                                           const int16_t*              pi_pNoDataValue)
    : HRPPixelType(HRPChannelOrgInt(16, pi_Role, CastNoDataValuePtr(pi_pNoDataValue).get()), 0)
    {
    }

//-----------------------------------------------------------------------------
// Copy constructor
//-----------------------------------------------------------------------------
HRPPixelTypeV16Int16::HRPPixelTypeV16Int16(const HRPPixelTypeV16Int16& pi_rObj)
    : HRPPixelType(pi_rObj)
    {
    }

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
HRPPixelTypeV16Int16::~HRPPixelTypeV16Int16()
    {
    }

//-----------------------------------------------------------------------------
// Clone method
//-----------------------------------------------------------------------------
HPMPersistentObject* HRPPixelTypeV16Int16::Clone() const
    {
    return new HRPPixelTypeV16Int16(*this);
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
unsigned short HRPPixelTypeV16Int16::CountValueBits() const
    {
    return 16;
    }

//-----------------------------------------------------------------------------
// HasConverterFrom
//-----------------------------------------------------------------------------
const HRPPixelConverter* HRPPixelTypeV16Int16::HasConverterFrom(const HRPPixelType* pi_pPixelTypeFrom) const
    {
    HFCMonitor Monitor(HRPPixelType::s_ConverterAccess);

    // This declaration needs to remain local in order to prevent initialization problems
    static struct V16Int16ConvertersFrom s_ConvertersFrom;

    MapHRPPixelTypeToConverter::const_iterator Itr;
    Itr = s_ConvertersFrom.find (pi_pPixelTypeFrom->GetClassID());

    return ((Itr == s_ConvertersFrom.end()) ? 0 : (*Itr).second);
    }

//-----------------------------------------------------------------------------
// HasConverterTo
//-----------------------------------------------------------------------------
const HRPPixelConverter* HRPPixelTypeV16Int16::HasConverterTo(const HRPPixelType* pi_pPixelTypeTo) const
    {
    HFCMonitor Monitor(HRPPixelType::s_ConverterAccess);

    // This declaration needs to remain local in order to prevent initialization problems
    static struct V16Int16ConvertersTo s_ConvertersTo;

    MapHRPPixelTypeToConverter::const_iterator Itr;
    Itr = s_ConvertersTo.find (pi_pPixelTypeTo->GetClassID());

    return ((Itr == s_ConvertersTo.end()) ? 0 : (*Itr).second);
    }