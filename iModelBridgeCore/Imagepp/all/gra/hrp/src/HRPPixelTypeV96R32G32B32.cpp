//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrp/src/HRPPixelTypeV96R32G32B32.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for class HRPPixelTypeV96R32G32B32
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HRPPixelTypeV96R32G32B32.h>
#include <Imagepp/all/h/HRPPixelTypeV24R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeV32R8G8B8A8.h>
#include <Imagepp/all/h/HRPPixelTypeV48R16G16B16.h>
#include <Imagepp/all/h/HRPPixelTypeV64R16G16B16A16.h>
#include <Imagepp/all/h/HFCMath.h>

HPM_REGISTER_CLASS(HRPPixelTypeV96R32G32B32, HRPPixelTypeRGB)

// STL typeDef
typedef map<HCLASS_ID, HRPPixelConverter*, less<HCLASS_ID>, allocator<HRPPixelConverter*> >
MapHRPPixelTypeToConverter;

//-----------------------------------------------------------------------------
//  s_V96R32G32B32_V96R32G32B32 - Converter
//-----------------------------------------------------------------------------
struct ConverterV96R32G32B32_V96R32G32B32 : public HRPPixelConverter
    {
    DEFINE_T_SUPER(HRPPixelConverter)

    void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        // 3 channels * 4 bytes per channel = 12
        HPRECONDITION(UINT_MAX / 12 > pi_PixelsCount);

        // 3 channels * 4 bytes per channel
        memcpy(pio_pDestRawData, pi_pSourceRawData, pi_PixelsCount * 3 * 4);
        };

    HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterV96R32G32B32_V96R32G32B32(*this));
        }
    };
static ConverterV96R32G32B32_V96R32G32B32 s_V96R32G32B32_V96R32G32B32;


//-----------------------------------------------------------------------------
//  s_V24R8G8B8_V96R32G32B332 - Converter
//-----------------------------------------------------------------------------
struct ConverterV24R8G8B8_V96R32G32B32 : public HRPPixelConverter
    {
    DEFINE_T_SUPER(HRPPixelConverter)

    void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        Byte*  pSrc = (Byte*)pi_pSourceRawData;
        uint32_t* pDst = (uint32_t*)pio_pDestRawData;

        // 3 Channels per pixel
        for(uint32_t i(0); i < pi_PixelsCount*3; i+=3)
            {
            pDst[i]   = CONVERT_8BIT_TO_32BIT(pSrc[i]);
            pDst[i+1] = CONVERT_8BIT_TO_32BIT(pSrc[i+1]);
            pDst[i+2] = CONVERT_8BIT_TO_32BIT(pSrc[i+2]);
            }
        };

    HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterV24R8G8B8_V96R32G32B32(*this));
        }

    };
static ConverterV24R8G8B8_V96R32G32B32 s_V24R8G8B8_V96R32G32B32;

//-----------------------------------------------------------------------------
//  s_V48R16G16B16_V96R32G32B332 - Converter
//-----------------------------------------------------------------------------
struct ConverterV48R16G16B16_V96R32G32B32 : public HRPPixelConverter
    {
    DEFINE_T_SUPER(HRPPixelConverter)

    void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        unsigned short* pSrc = (unsigned short*)pi_pSourceRawData;
        uint32_t* pDst = (uint32_t*)pio_pDestRawData;

        // 3 Channels per pixel
        for(unsigned short i(0); i < pi_PixelsCount*3; i+=3)
            {
            pDst[i]   = CONVERT_16BIT_TO_32BIT(pSrc[i]);
            pDst[i+1] = CONVERT_16BIT_TO_32BIT(pSrc[i+1]);
            pDst[i+2] = CONVERT_16BIT_TO_32BIT(pSrc[i+2]);
            }
        };

    HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterV48R16G16B16_V96R32G32B32(*this));
        }

    };
static ConverterV48R16G16B16_V96R32G32B32 s_V48R16G16B16_V96R32G32B32;

//-----------------------------------------------------------------------------
//  s_V32R8G8B8A8_V96R32G32B332 - Converter
//-----------------------------------------------------------------------------
struct ConverterV32R8G8B8A8_V96R32G32B332 : public HRPPixelConverter
    {
    DEFINE_T_SUPER(HRPPixelConverter)

    void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        Byte*  pSrc = (Byte*)pi_pSourceRawData;
        uint32_t* pDst = (uint32_t*)pio_pDestRawData;

        // 3 Channels per pixel
        for(uint32_t i(0); i < pi_PixelsCount*3; i+=3)
            {
            pDst[i]   = CONVERT_8BIT_TO_32BIT(pSrc[0]);
            pDst[i+1] = CONVERT_8BIT_TO_32BIT(pSrc[1]);
            pDst[i+2] = CONVERT_8BIT_TO_32BIT(pSrc[2]);

            // Next RGBA
            pSrc+=4;
            }
        };

    virtual void Compose(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        Byte*  pSrc = (Byte*)pi_pSourceRawData;
        uint32_t* pDst = (uint32_t*)pio_pDestRawData;

        // Dst has 3 channel per pixel
        for(uint32_t i(0); i < pi_PixelsCount*3; i+=3)
            {
            // ((SrcAlpha * (Src - Dst)) / MaxAlpha) + Dst
            pDst[i]   = (uint32_t)(((uint64_t)CONVERT_8BIT_TO_32BIT(pSrc[3]) * (CONVERT_8BIT_TO_32BIT(pSrc[0]) - pDst[i]))   / 0xFFFFFFFF) + pDst[i];
            pDst[i+1] = (uint32_t)(((uint64_t)CONVERT_8BIT_TO_32BIT(pSrc[3]) * (CONVERT_8BIT_TO_32BIT(pSrc[1]) - pDst[i+1])) / 0xFFFFFFFF) + pDst[i+1];
            pDst[i+2] = (uint32_t)(((uint64_t)CONVERT_8BIT_TO_32BIT(pSrc[3]) * (CONVERT_8BIT_TO_32BIT(pSrc[2]) - pDst[i+2])) / 0xFFFFFFFF) + pDst[i+2];

            // Next RGBA
            pSrc+=4;
            }
        };

    virtual const short* GetLostChannels() const override
        {
        return m_LostChannels;
        }

    HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterV32R8G8B8A8_V96R32G32B332(*this));
        }

private:

    static short m_LostChannels[];
    };
short ConverterV32R8G8B8A8_V96R32G32B332::m_LostChannels[] = {3, -1};
static ConverterV32R8G8B8A8_V96R32G32B332 s_V32R8G8B8A8_V96R32G32B332;

//-----------------------------------------------------------------------------
//  s_V96R32G32B32_V24R8G8B8 - Converter
//-----------------------------------------------------------------------------
struct ConverterV96R32G32B32_V24R8G8B8 : public HRPPixelConverter
    {
    DEFINE_T_SUPER(HRPPixelConverter)

    void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        uint32_t* pSrc = (uint32_t*)pi_pSourceRawData;
        Byte*  pDst = (Byte*)pio_pDestRawData;

        // 3 Channels per pixel.
        for(uint32_t i(0); i < pi_PixelsCount*3; i+=3)
            {
            pDst[i]   = CONVERT_32BIT_TO_8BIT(pSrc[i]);
            pDst[i+1] = CONVERT_32BIT_TO_8BIT(pSrc[i+1]);
            pDst[i+2] = CONVERT_32BIT_TO_8BIT(pSrc[i+2]);
            }
        };

    HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterV96R32G32B32_V24R8G8B8(*this));
        }
    };
static ConverterV96R32G32B32_V24R8G8B8 s_V96R32G32B32_V24R8G8B8;

//-----------------------------------------------------------------------------
//  s_V96R32G32B32_V32R8G8B8A8 - Converter
//-----------------------------------------------------------------------------
struct ConverterV96R32G32B32_V32R8G8B8A8 : public HRPPixelConverter
    {
    DEFINE_T_SUPER(HRPPixelConverter)

    void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        uint32_t* pSrc = (uint32_t*)pi_pSourceRawData;
        Byte*  pDst = (Byte*)pio_pDestRawData;

        // 4 Channels per pixel.
        for(uint32_t i(0); i < pi_PixelsCount*3; i+=3)
            {
            pDst[0] = CONVERT_32BIT_TO_8BIT(pSrc[i]);
            pDst[1] = CONVERT_32BIT_TO_8BIT(pSrc[i+1]);
            pDst[2] = CONVERT_32BIT_TO_8BIT(pSrc[i+2]);
            pDst[3] = 0xFF;

            // Next RGBA pixel
            pDst+=4;
            }
        };

    HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterV96R32G32B32_V32R8G8B8A8(*this));
        }
    };
static ConverterV96R32G32B32_V32R8G8B8A8 s_V96R32G32B32_V32R8G8B8A8;


//-----------------------------------------------------------------------------
//  s_V96R32G32B32_V48R16G16B16 - Converter
//-----------------------------------------------------------------------------
struct ConverterV96R32G32B32_V48R16G16B16 : public HRPPixelConverter
    {
    DEFINE_T_SUPER(HRPPixelConverter)

    void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        uint32_t* pSrc = (uint32_t*)pi_pSourceRawData;
        unsigned short*  pDst = (unsigned short*)pio_pDestRawData;

        // 3 Channels per pixel.
        for(uint32_t i(0); i < pi_PixelsCount*3; i+=3)
            {
            pDst[i]   = CONVERT_32BIT_TO_16BIT(pSrc[i]);
            pDst[i+1] = CONVERT_32BIT_TO_16BIT(pSrc[i+1]);
            pDst[i+2] = CONVERT_32BIT_TO_16BIT(pSrc[i+2]);
            }
        };

    HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterV96R32G32B32_V48R16G16B16(*this));
        }
    };
static ConverterV96R32G32B32_V48R16G16B16 s_V96R32G32B32_V48R16G16B16;

//-----------------------------------------------------------------------------
//  Dictionnary of converters from other pixel types
//-----------------------------------------------------------------------------
struct V96R32G32B32ConvertersFrom : public MapHRPPixelTypeToConverter
    {
    V96R32G32B32ConvertersFrom() : MapHRPPixelTypeToConverter ()
        {
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV96R32G32B32::CLASS_ID, &s_V96R32G32B32_V96R32G32B32));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV24R8G8B8::CLASS_ID, &s_V24R8G8B8_V96R32G32B32));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV32R8G8B8A8::CLASS_ID, &s_V32R8G8B8A8_V96R32G32B332));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV48R16G16B16::CLASS_ID, &s_V48R16G16B16_V96R32G32B32));
        };
    };

//-----------------------------------------------------------------------------
//  Dictionnary of converters from other pixel types
//-----------------------------------------------------------------------------
struct V96R32G32B32ConvertersTo : public MapHRPPixelTypeToConverter
    {
    V96R32G32B32ConvertersTo() : MapHRPPixelTypeToConverter ()
        {
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV96R32G32B32::CLASS_ID, &s_V96R32G32B32_V96R32G32B32));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV24R8G8B8::CLASS_ID, &s_V96R32G32B32_V24R8G8B8));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV32R8G8B8A8::CLASS_ID, &s_V96R32G32B32_V32R8G8B8A8));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV48R16G16B16::CLASS_ID, &s_V96R32G32B32_V48R16G16B16));
        };
    };

//-----------------------------------------------------------------------------
// Default Constructor
//-----------------------------------------------------------------------------
HRPPixelTypeV96R32G32B32::HRPPixelTypeV96R32G32B32()
    : HRPPixelTypeRGB(32,32,32,0,0)
    {
    }

//-----------------------------------------------------------------------------
// Copy constructor
//-----------------------------------------------------------------------------
HRPPixelTypeV96R32G32B32::HRPPixelTypeV96R32G32B32(const HRPPixelTypeV96R32G32B32& pi_rObj)
    : HRPPixelTypeRGB(pi_rObj)
    {
    }

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
HRPPixelTypeV96R32G32B32::~HRPPixelTypeV96R32G32B32()
    {
    }

//-----------------------------------------------------------------------------
// Clone method
//-----------------------------------------------------------------------------
HPMPersistentObject* HRPPixelTypeV96R32G32B32::Clone() const
    {
    return new HRPPixelTypeV96R32G32B32(*this);
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
unsigned short HRPPixelTypeV96R32G32B32::CountValueBits() const
    {
    return 96;
    }

//-----------------------------------------------------------------------------
// HasConverterFrom
//-----------------------------------------------------------------------------
const HRPPixelConverter* HRPPixelTypeV96R32G32B32::HasConverterFrom(const HRPPixelType* pi_pPixelTypeFrom) const
    {
    HFCMonitor Monitor(HRPPixelType::s_ConverterAccess);

    // This declaration needs to remain local in order to prevent initialization problems
    static struct V96R32G32B32ConvertersFrom s_ConvertersFrom;

    MapHRPPixelTypeToConverter::const_iterator Itr;
    Itr = s_ConvertersFrom.find (pi_pPixelTypeFrom->GetClassID());

    return ((Itr == s_ConvertersFrom.end()) ? 0 : (*Itr).second);
    }

//-----------------------------------------------------------------------------
// HasConverterTo
//-----------------------------------------------------------------------------
const HRPPixelConverter* HRPPixelTypeV96R32G32B32::HasConverterTo(const HRPPixelType* pi_pPixelTypeTo) const
    {
    HFCMonitor Monitor(HRPPixelType::s_ConverterAccess);

    // This declaration needs to remain local in order to prevent initialization problems
    static struct V96R32G32B32ConvertersTo s_ConvertersTo;

    MapHRPPixelTypeToConverter::const_iterator Itr;
    Itr = s_ConvertersTo.find (pi_pPixelTypeTo->GetClassID());

    return ((Itr == s_ConvertersTo.end()) ? 0 : (*Itr).second);
    }


