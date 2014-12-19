//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrp/src/HRPPixelTypeV32Float32.cpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for class HRPPixelTypeV32Float32
//-----------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>

#include <Imagepp/all/h/HRPPixelTypeV32Float32.h>
#include <Imagepp/all/h/HRPChannelOrgFloat.h>

#include <Imagepp/all/h/HRPPixelTypeV32R8G8B8A8.h>
#include <Imagepp/all/h/HRPPixelTypeV24R8G8B8.h>
#include <Imagepp/all/h/HFCMath.h>

HPM_REGISTER_CLASS(HRPPixelTypeV32Float32, HRPPixelType)

// STL typeDef
typedef map<HCLASS_ID, HRPPixelConverter*, less<HCLASS_ID>, allocator<HRPPixelConverter*> >
MapHRPPixelTypeToConverter;

//-----------------------------------------------------------------------------
//  s_V32Float32_V32Float32 - Converter between HRPPixelTypeV32Float32
//-----------------------------------------------------------------------------
struct ConverterV32Float32_V32Float32 : public HRPPixelConverter
    {
    DEFINE_T_SUPER(HRPPixelConverter)

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData) const
        {
        *((float*)pio_pDestRawData) = *((float*)pi_pSourceRawData);
        };

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const
        {
        memcpy(pio_pDestRawData, pi_pSourceRawData, pi_PixelsCount * 4);
        };
    virtual void    Convert(const void* pi_pSourceRawData,
                            void* pio_pDestRawData,
                            size_t pi_PixelsCount,
                            const bool* pi_pChannelsMask) const
        {
        return T_Super::Convert(pi_pSourceRawData,pio_pDestRawData,pi_PixelsCount,pi_pChannelsMask);
        }

    virtual HRPPixelConverter* AllocateCopy() const {
        return(new ConverterV32Float32_V32Float32(*this));
        }
    };
struct ConverterV32Float32_V32Float32 s_V32Float32_V32Float32;

//-----------------------------------------------------------------------------
//  s_V24R8G8B8_V32Float32 - Converter
//-----------------------------------------------------------------------------
struct ConverterV24R8G8B8_V32Float32 : public HRPPixelConverter
    {
    DEFINE_T_SUPER(HRPPixelConverter)

    void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData) const
        {
        //It is the application's task to convert color data to float data.
        *(float*)pio_pDestRawData = 0;
        };

    void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const
        {
        Byte* pSrc  = (Byte*)pi_pSourceRawData;

        for (size_t i(0); i < pi_PixelsCount; ++i)
            {
            //It is the application's task to convert color data to float data.
            *(float*)pio_pDestRawData = 0;

            // next RGB
            pSrc += 3;
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
        return(new ConverterV24R8G8B8_V32Float32(*this));
        }
    };
struct ConverterV24R8G8B8_V32Float32 s_V24R8G8B8_V32Float32;

//-----------------------------------------------------------------------------
//  s_V32R8G8B8A8_V32Float32 - Converter
//-----------------------------------------------------------------------------
struct ConverterV32R8G8B8A8_V32Float32 : public HRPPixelConverter
    {
public:
    DEFINE_T_SUPER(HRPPixelConverter)

    void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData) const
        {
        //It is the application's task to convert color data to float data.
        *(float*)pio_pDestRawData = 0;
        };

    void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const
        {
        Byte* pSrc  = (Byte*)pi_pSourceRawData;
        float* pDest = (float*)pio_pDestRawData;

        for(size_t i(0); i < pi_PixelsCount; ++i)
            {
            //It is the application's task to convert color data to float data.
            pDest[i] = 0;

            // next RGBA
            pSrc+=4;
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
        Byte* pSrc  = (Byte*)pi_pSourceRawData;
        float* pDest = (float*)pio_pDestRawData;

        //It is the application's task to convert color data to float data.
        float FloatValue = 0;

        // ((Src * SrcAlpha) + (Dst * (MaxAlpha - SrcAlpha))) / MaxAlpha
        *pDest = ((FloatValue * pSrc[3]) + (*pDest * (0xFF - pSrc[3]))) / 0xFF;
        };

    virtual void Compose(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const
        {
        Byte* pSrc  = (Byte*)pi_pSourceRawData;
        float* pDest = (float*)pio_pDestRawData;

        for(size_t i(0); i < pi_PixelsCount; ++i)
            {
            //It is the application's task to convert color data to float data.
            float FloatValue = 0;

            // ((Src * SrcAlpha) + (Dst * (MaxAlpha - SrcAlpha))) / MaxAlpha
            pDest[i] = ((FloatValue * pSrc[3]) + (pDest[i] * (0xFF - pSrc[3]))) / 0xFF;

            // next RGBA
            pSrc+=4;
            }
        };

    virtual const short* GetLostChannels() const
        {
        return m_LostChannels;
        };

    HRPPixelConverter* AllocateCopy() const {
        return(new ConverterV32R8G8B8A8_V32Float32(*this));
        }

private:

    static short m_LostChannels[];
    };
short ConverterV32R8G8B8A8_V32Float32::m_LostChannels[] = {3, -1};
static ConverterV32R8G8B8A8_V32Float32 s_V32R8G8B8A8_V32Float32;

//-----------------------------------------------------------------------------
//  s_V32Float32_V24R8G8B8 - Converter
//-----------------------------------------------------------------------------
struct ConverterV32Float32_V24R8G8B8 : public HRPPixelConverter
    {
    DEFINE_T_SUPER(HRPPixelConverter)

    void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData) const
        {
        Byte* pDst = (Byte*)pio_pDestRawData;

        //It is the application's task to convert float data to color data.
        pDst[0] = pDst[1] = pDst[2] = 0;
        };

    void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const
        {
        Byte* pDst = (Byte*)pio_pDestRawData;

        for(size_t i(0); i < pi_PixelsCount; ++i)
            {
            //It is the application's task to convert float data to color data.
            pDst[0] = pDst[1] = pDst[2] = 0;

            // Next RGB
            pDst+=3;
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
        return(new ConverterV32Float32_V24R8G8B8(*this));
        }
    };
struct ConverterV32Float32_V24R8G8B8 s_V32Float32_V24R8G8B8;

//-----------------------------------------------------------------------------
//  s_V32Float32_V32R8G8B8A8 - Converter
//-----------------------------------------------------------------------------
struct ConverterV32Float32_V32R8G8B8A8 : public HRPPixelConverter
    {
    DEFINE_T_SUPER(HRPPixelConverter)

    void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData) const
        {
        Byte* pDst = (Byte*)pio_pDestRawData;

        //It is the application's task to convert float data to color data.
        pDst[0] = pDst[1] = pDst[2] = 0;
        pDst[3] = 0xFF;
        };

    void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const
        {
        Byte* pDst = (Byte*)pio_pDestRawData;

        for(size_t i(0); i < pi_PixelsCount; ++i)
            {
            //It is the application's task to convert float data to color data.
            pDst[0] = pDst[1] = pDst[2] = 0;
            pDst[3] = 0xFF;

            // Next RGBA
            pDst+=4;
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
        return(new ConverterV32Float32_V32R8G8B8A8(*this));
        }
    };
static ConverterV32Float32_V32R8G8B8A8 s_V32Float32_V32R8G8B8A8;

//-----------------------------------------------------------------------------
//  Dictionnary of converters from other pixel types
//-----------------------------------------------------------------------------
struct V32Float32ConvertersFrom : public MapHRPPixelTypeToConverter
    {
    V32Float32ConvertersFrom() : MapHRPPixelTypeToConverter()
        {
        insert(MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV32Float32::CLASS_ID,  &s_V32Float32_V32Float32));
        insert(MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV24R8G8B8::CLASS_ID,   &s_V24R8G8B8_V32Float32));
        insert(MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV32R8G8B8A8::CLASS_ID, &s_V32R8G8B8A8_V32Float32));
        };
    };

//-----------------------------------------------------------------------------
//  Dictionnary of converters from other pixel types
//-----------------------------------------------------------------------------
struct V32Float32ConvertersTo : public MapHRPPixelTypeToConverter
    {
    V32Float32ConvertersTo()
        {
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV32Float32::CLASS_ID,  &s_V32Float32_V32Float32));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV32R8G8B8A8::CLASS_ID, &s_V32Float32_V32R8G8B8A8));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV24R8G8B8::CLASS_ID,   &s_V32Float32_V24R8G8B8));
        };
    };

//-----------------------------------------------------------------------------
// Routine that cast a float NoDataValue to a double NoDataValue
//-----------------------------------------------------------------------------
static HAutoPtr<double> CastNoDataValuePtr(const float* pi_pNoDataValue)
    {
    HAutoPtr<double> pNoDataValue;

    if (0 != pi_pNoDataValue)
        pNoDataValue = new double( *pi_pNoDataValue );

    return pNoDataValue;
    }


//-----------------------------------------------------------------------------
// Constructor for 32 bits float
//-----------------------------------------------------------------------------
HRPPixelTypeV32Float32::HRPPixelTypeV32Float32(HRPChannelType::ChannelRole pi_Role,
                                               const float*               pi_pNoDataValue)
    : HRPPixelType(HRPChannelOrgFloat(32, pi_Role, CastNoDataValuePtr(pi_pNoDataValue).get()), 0)
    {
    }

//-----------------------------------------------------------------------------
// Copy constructor
//-----------------------------------------------------------------------------
HRPPixelTypeV32Float32::HRPPixelTypeV32Float32(const HRPPixelTypeV32Float32& pi_rObj)
    : HRPPixelType(pi_rObj)
    {
    }

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
HRPPixelTypeV32Float32::~HRPPixelTypeV32Float32()
    {
    }

//-----------------------------------------------------------------------------
// Clone method
//-----------------------------------------------------------------------------
HPMPersistentObject* HRPPixelTypeV32Float32::Clone() const
    {
    return new HRPPixelTypeV32Float32(*this);
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
unsigned short HRPPixelTypeV32Float32::CountValueBits() const
    {
    return 32;
    }

//-----------------------------------------------------------------------------
// HasConverterFrom
//-----------------------------------------------------------------------------
const HRPPixelConverter* HRPPixelTypeV32Float32::HasConverterFrom(const HRPPixelType* pi_pPixelTypeFrom) const
    {
    HFCMonitor Monitor(HRPPixelType::s_ConverterAccess);

    // This declaration needs to remain local in order to prevent initialization problems
    static struct V32Float32ConvertersFrom s_ConvertersFrom;

    MapHRPPixelTypeToConverter::const_iterator Itr;
    Itr = s_ConvertersFrom.find (pi_pPixelTypeFrom->GetClassID());

    return ((Itr == s_ConvertersFrom.end()) ? 0 : (*Itr).second);
    }

//-----------------------------------------------------------------------------
// HasConverterTo
//-----------------------------------------------------------------------------
const HRPPixelConverter* HRPPixelTypeV32Float32::HasConverterTo(const HRPPixelType* pi_pPixelTypeTo) const
    {
    HFCMonitor Monitor(HRPPixelType::s_ConverterAccess);

    // This declaration needs to remain local in order to prevent initialization problems
    static struct V32Float32ConvertersTo s_ConvertersTo;

    MapHRPPixelTypeToConverter::const_iterator Itr;
    Itr = s_ConvertersTo.find (pi_pPixelTypeTo->GetClassID());

    return ((Itr == s_ConvertersTo.end()) ? 0 : (*Itr).second);
    }