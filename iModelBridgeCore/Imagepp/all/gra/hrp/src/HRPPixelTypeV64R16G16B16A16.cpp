//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrp/src/HRPPixelTypeV64R16G16B16A16.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for class HRPPixelTypeV64R16G16B16A16
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HRPPixelTypeV64R16G16B16A16.h>
#include <Imagepp/all/h/HRPPixelTypeV24R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeV32R8G8B8A8.h>
#include <Imagepp/all/h/HRPPixelTypeV48R16G16B16.h>
#include <Imagepp/all/h/HFCMath.h>

HPM_REGISTER_CLASS(HRPPixelTypeV64R16G16B16A16, HRPPixelTypeRGB)

// STL typeDef
typedef map<HCLASS_ID, HRPPixelConverter*, less<HCLASS_ID>, allocator<HRPPixelConverter*> >
MapHRPPixelTypeToConverter;

//-----------------------------------------------------------------------------
//  s_V64R16G16B16A16_V64R16G16B16A16 - Converter
//-----------------------------------------------------------------------------
struct ConverterV64R16G16B16A16_V64R16G16B16A16 : public HRPPixelConverter
    {
    DEFINE_T_SUPER(HRPPixelConverter)

    void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        HPRECONDITION(UINT_MAX / 8 > pi_PixelsCount);

        // 4 channels and 2 bytes per channel
        memcpy(pio_pDestRawData, pi_pSourceRawData, pi_PixelsCount * 8);
        };

    virtual void Compose(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        unsigned short* pSrc  = (unsigned short*)pi_pSourceRawData;
        unsigned short* pDest = (unsigned short*)pio_pDestRawData;

        for(size_t i(0); i < pi_PixelsCount*4; i+=4)
            {
            // If source pixel is fully transparent, destination is unaltered
            if(pSrc[i+3] != 0)
                {
                // Destination pixel is fully transparent or source pixel is fully opaque.
                if(pDest[i+3] == 0 || pSrc[i+3] == 0xFFFF)
                    {
                    // Copy source pixel
                    memcpy(&pDest[i], &pSrc[i], 8);
                    }
                else
                    {
                    // premul = ((Adst * Cdst)/MaxAlpha)
                    // Cdst'  = ((Asrc * (Csrc - premul)) / MaxAlpha) + premul
                    int64_t PremultDestColor = (pDest[i+3] * pDest[i]) / 0xFFFF;
                    pDest[i] = (unsigned short)(((pSrc[i + 3] * (pSrc[i] - PremultDestColor)) / 0xFFFF) + PremultDestColor);

                    PremultDestColor = (pDest[i+3] * pDest[i+1]) / 0xFFFF;
                    pDest[i + 1] = (unsigned short)(((pSrc[i + 3] * (pSrc[i + 1] - PremultDestColor)) / 0xFFFF) + PremultDestColor);

                    PremultDestColor = (pDest[i+3] * pDest[i+2]) / 0xFFFF;
                    pDest[i + 2] = (unsigned short)(((pSrc[i + 3] * (pSrc[i + 2] - PremultDestColor)) / 0xFFFF) + PremultDestColor);

                    // Adst' = MaxAlpha - (( (MaxAlpha - Asrc) * (MaxAlpha - Adst) ) / MaxAlpha
                    // --> Transparency percentages are multiplied
                    pDest[i+3] = 0xFFFF - (((0xFFFF - (int64_t)pSrc[i+3]) * (0xFFFF - pDest[i+3])) / 0xFFFF);
                    }
                }
            }

        };

    HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterV64R16G16B16A16_V64R16G16B16A16(*this));
        }
    };
static ConverterV64R16G16B16A16_V64R16G16B16A16 s_V64R16G16B16A16_V64R16G16B16A16;

//-----------------------------------------------------------------------------
//  s_V24R8G8B8_V64R16G16B16A16 - Converter
//-----------------------------------------------------------------------------
struct ConverterV24R8G8B8_V64R16G16B16A16 : public HRPPixelConverter
    {
    DEFINE_T_SUPER(HRPPixelConverter)

    void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        Byte*  pSrc  = (Byte*)pi_pSourceRawData;
        unsigned short* pDest = (unsigned short*)pio_pDestRawData;

        for(size_t i(0); i < pi_PixelsCount*3; i+=3)
            {
            pDest[0] = CONVERT_8BIT_TO_16BIT(pSrc[i]);
            pDest[1] = CONVERT_8BIT_TO_16BIT(pSrc[i+1]);
            pDest[2] = CONVERT_8BIT_TO_16BIT(pSrc[i+2]);
            pDest[3] = 0xFFFF;

            // Next RGBA
            pDest+=4;
            }
        };

    HRPPixelConverter* AllocateCopy() const override {
        return(new ConverterV24R8G8B8_V64R16G16B16A16(*this));
        }
    };
static ConverterV24R8G8B8_V64R16G16B16A16 s_V24R8G8B8_V64R16G16B16A16;

//-----------------------------------------------------------------------------
//  s_V32R8G8B8A8_V64R16G16B16A16 - Converter
//-----------------------------------------------------------------------------
struct ConverterV32R8G8B8A8_V64R16G16B16A16 : public HRPPixelConverter
    {
    DEFINE_T_SUPER(HRPPixelConverter)

    void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        Byte*  pSrc  = (Byte*)pi_pSourceRawData;
        unsigned short* pDest = (unsigned short*)pio_pDestRawData;

        for(uint32_t i(0); i < pi_PixelsCount*4; i+=4)
            {
            pDest[i]   = CONVERT_8BIT_TO_16BIT(pSrc[i]);
            pDest[i+1] = CONVERT_8BIT_TO_16BIT(pSrc[i+1]);
            pDest[i+2] = CONVERT_8BIT_TO_16BIT(pSrc[i+2]);
            pDest[i+3] = CONVERT_8BIT_TO_16BIT(pSrc[i+3]);
            }
        };

    virtual void Compose(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        Byte*  pV32Src  = (Byte*)pi_pSourceRawData;
        unsigned short* pV64Dest = (unsigned short*)pio_pDestRawData;

        while(pi_PixelsCount)
            {
            // If source pixel is fully transparent, destination is unaltered
            if(pV32Src[3] != 0)
                {
                unsigned short V64Src[4];

                V64Src[0] = CONVERT_8BIT_TO_16BIT(pV32Src[0]);
                V64Src[1] = CONVERT_8BIT_TO_16BIT(pV32Src[1]);
                V64Src[2] = CONVERT_8BIT_TO_16BIT(pV32Src[2]);
                V64Src[3] = CONVERT_8BIT_TO_16BIT(pV32Src[3]);

                // Destination pixel is fully transparent or source pixel is fully opaque.
                if(pV64Dest[3] == 0 || pV32Src[3] == 0xFF)
                    {
                    // Copy source pixel
                    memcpy(pV64Dest, V64Src, 2*4);
                    }
                else
                    {
                    // premul = ((Adst * Cdst)/MaxAlpha)
                    // Cdst'  = ((Asrc * (Csrc - premul)) / MaxAlpha) + premul
                    int64_t PremultDestColor = (pV64Dest[3] * pV64Dest[0]) / 0xFFFF;
                    pV64Dest[0] = (unsigned short)(((V64Src[3] * (V64Src[0] - PremultDestColor)) / 0xFFFF) + PremultDestColor);

                    PremultDestColor = (pV64Dest[3] * pV64Dest[1]) / 0xFFFF;
                    pV64Dest[1] = (unsigned short)(((V64Src[3] * (V64Src[1] - PremultDestColor)) / 0xFFFF) + PremultDestColor);

                    PremultDestColor = (pV64Dest[3] * pV64Dest[2]) / 0xFFFF;
                    pV64Dest[2] = (unsigned short)(((V64Src[3] * (V64Src[2] - PremultDestColor)) / 0xFFFF) + PremultDestColor);

                    // Adst' = MaxAlpha - (( (MaxAlpha - Asrc) * (MaxAlpha - Adst) ) / MaxAlpha
                    // --> Transparency percentages are multiplied
                    pV64Dest[3] = 0xFFFF - (((0xFFFF - V64Src[3]) * (0xFFFF - (int64_t)pV64Dest[3])) / 0xFFFF);
                    }
                }

            --pi_PixelsCount;
            pV32Src+=4;
            pV64Dest+=4;
            }
        };

    HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterV32R8G8B8A8_V64R16G16B16A16(*this));
        }
    };

static ConverterV32R8G8B8A8_V64R16G16B16A16 s_V32R8G8B8A8_V64R16G16B16A16;

//-----------------------------------------------------------------------------
//  ConverterV48R16G16B16_V64R16G16B16A16 - Converter
//-----------------------------------------------------------------------------
struct ConverterV48R16G16B16_V64R16G16B16A16 : public HRPPixelConverter
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

            // Next RGBA
            pDst+=4;
            }
        };

    HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterV48R16G16B16_V64R16G16B16A16(*this));
        }
    };
struct ConverterV48R16G16B16_V64R16G16B16A16 s_V48R16G16B16_V64R16G16B16A16;

//-----------------------------------------------------------------------------
//  s_V64R16G16B16A16_V24R8G8B8 - Converter
//-----------------------------------------------------------------------------
struct ConverterV64R16G16B16A16_V24R8G8B8 : public HRPPixelConverter
    {
    DEFINE_T_SUPER(HRPPixelConverter)

    void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        unsigned short* pSrc  = (unsigned short*)pi_pSourceRawData;
        Byte*  pDest = (Byte*)pio_pDestRawData;

        for(size_t i(0); i < pi_PixelsCount*3; i+=3)
            {
            pDest[i]   = CONVERT_16BIT_TO_8BIT(pSrc[0]);
            pDest[i+1] = CONVERT_16BIT_TO_8BIT(pSrc[1]);
            pDest[i+2] = CONVERT_16BIT_TO_8BIT(pSrc[2]);

            // Next RGBA
            pSrc+=4;
            }
        };

    virtual void Compose(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        unsigned short const* pSrc = (unsigned short const*)pi_pSourceRawData;
        Byte*        pDst = (Byte*)pio_pDestRawData;

        HFCMath const* pQuotients = HFCMath::GetInstance();

        // Dst has 3 channel per pixel
        for(size_t i(0); i < pi_PixelsCount*3; i+=3)
            {
            // ((SrcAlpha * (Src - Dst)) / MaxAlpha) + Dst

#if 1       // Compute in 8bits to avoid computation in 64 bits.
            pDst[i] = pQuotients->DivideBy255ToByte(CONVERT_16BIT_TO_8BIT(pSrc[3]) * (CONVERT_16BIT_TO_8BIT(pSrc[0]) - pDst[i])) + pDst[i];
            pDst[i + 1] = pQuotients->DivideBy255ToByte(CONVERT_16BIT_TO_8BIT(pSrc[3]) * (CONVERT_16BIT_TO_8BIT(pSrc[1]) - pDst[i + 1])) + pDst[i + 1];
            pDst[i + 2] = pQuotients->DivideBy255ToByte(CONVERT_16BIT_TO_8BIT(pSrc[3]) * (CONVERT_16BIT_TO_8BIT(pSrc[2]) - pDst[i + 2])) + pDst[i + 2];
#else 
            pDst[i]   = (int64_t)CONVERT_16BIT_TO_8BIT((pSrc[3] * (pSrc[0] - (int64_t)CONVERT_8BIT_TO_16BIT(pDst[i])))   / 0xFFFF) + pDst[i];
            pDst[i+1] = (int64_t)CONVERT_16BIT_TO_8BIT((pSrc[3] * (pSrc[1] - (int64_t)CONVERT_8BIT_TO_16BIT(pDst[i+1]))) / 0xFFFF) + pDst[i+1];
            pDst[i+2] = (int64_t)CONVERT_16BIT_TO_8BIT((pSrc[3] * (pSrc[2] - (int64_t)CONVERT_8BIT_TO_16BIT(pDst[i+2]))) / 0xFFFF) + pDst[i+2];
#endif

            // Next RGBA
            pSrc+=4;
            }
        };

    HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterV64R16G16B16A16_V24R8G8B8(*this));
        }
    };
static ConverterV64R16G16B16A16_V24R8G8B8 s_V64R16G16B16A16_V24R8G8B8;

//-----------------------------------------------------------------------------
//  s_V64R16G16B16A16_V32R8G8B8A8 - Converter
//-----------------------------------------------------------------------------
struct ConverterV64R16G16B16A16_V32R8G8B8A8 : public HRPPixelConverter
    {
    DEFINE_T_SUPER(HRPPixelConverter)

    void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        unsigned short* pSrc  = (unsigned short*)pi_pSourceRawData;
        Byte*  pDest = (Byte*)pio_pDestRawData;

        for(size_t i(0); i < pi_PixelsCount*4; i+=4)
            {
            pDest[i]   = CONVERT_16BIT_TO_8BIT(pSrc[i]);
            pDest[i+1] = CONVERT_16BIT_TO_8BIT(pSrc[i+1]);
            pDest[i+2] = CONVERT_16BIT_TO_8BIT(pSrc[i+2]);
            pDest[i+3] = CONVERT_16BIT_TO_8BIT(pSrc[i+3]);
            }
        };

    virtual void Compose(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        unsigned short* pV64Src  = (unsigned short*)pi_pSourceRawData;
        Byte*  pV32Dest = (Byte*)pio_pDestRawData;
        HFCMath (*pQuotients) (HFCMath::GetInstance());

        while(pi_PixelsCount)
            {
            Byte V32Src[4];
            V32Src[3] = CONVERT_16BIT_TO_8BIT(pV64Src[3]);

            // If source pixel is fully transparent, destination is unaltered
            if(V32Src[3] != 0)
                {
                // Convert the remaining channels
                V32Src[0] = CONVERT_16BIT_TO_8BIT(pV64Src[0]);
                V32Src[1] = CONVERT_16BIT_TO_8BIT(pV64Src[1]);
                V32Src[2] = CONVERT_16BIT_TO_8BIT(pV64Src[2]);

                // Destination pixel is fully transparent or source pixel is fully opaque
                if (pV32Dest[3] == 0 || V32Src[3] == 0xFF)
                    {
                    // Copy source pixel
                    *((uint32_t*)pV32Dest) = *((uint32_t*)V32Src);
                    }
                else
                    {
                    // premul = ((Adst * Cdst)/MaxAlpha)
                    // Cdst'  = ((Asrc * (Csrc - premul)) / MaxAlpha) + premul
                    Byte PremultDestColor = pQuotients->DivideBy255ToByte(pV32Dest[3] * pV32Dest[0]);
                    pV32Dest[0] = pQuotients->DivideBy255ToByte(V32Src[3] * (V32Src[0] - PremultDestColor)) + PremultDestColor;

                    PremultDestColor = pQuotients->DivideBy255ToByte(pV32Dest[3] * pV32Dest[1]);
                    pV32Dest[1] = pQuotients->DivideBy255ToByte(V32Src[3] * (V32Src[1] - PremultDestColor)) + PremultDestColor;

                    PremultDestColor = pQuotients->DivideBy255ToByte(pV32Dest[3] * pV32Dest[2]);
                    pV32Dest[2] = pQuotients->DivideBy255ToByte(V32Src[3] * (V32Src[2] - PremultDestColor)) + PremultDestColor;

                    // Adst' = MaxAlpha - (( (MaxAlpha - Asrc) * (MaxAlpha - Adst) ) / MaxAlpha
                    // --> Transparency percentages are multiplied
                    pV32Dest[3] = 0xFF - (pQuotients->DivideBy255ToByte((0xFF - V32Src[3]) * (0xFF - pV32Dest[3])));
                    }

                }

            --pi_PixelsCount;
            pV64Src+=4;
            pV32Dest+=4;
            }
        };
    HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterV64R16G16B16A16_V32R8G8B8A8(*this));
        }
    };
static ConverterV64R16G16B16A16_V32R8G8B8A8 s_V64R16G16B16A16_V32R8G8B8A8;

//-----------------------------------------------------------------------------
//  s_V64R16G16B16A16_V48R16G16B16 - Converter
//-----------------------------------------------------------------------------
struct ConverterV64R16G16B16A16_V48R16G16B16 : public HRPPixelConverter
    {
    DEFINE_T_SUPER(HRPPixelConverter)

        void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        unsigned short* pSrc = (unsigned short*)pi_pSourceRawData;
        unsigned short* pDst = (unsigned short*)pio_pDestRawData;

        for(size_t i(0); i < pi_PixelsCount*3; i+=3)
            {
            // copy 3 channels and 2 bytes per channel
            memcpy(&pDst[i], pSrc, 3*2);

            // Next RGBA
            pSrc+=4;
            }
        };

    virtual void Compose(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        unsigned short* pSrc = (unsigned short*)pi_pSourceRawData;
        unsigned short* pDst = (unsigned short*)pio_pDestRawData;

        // Dst has 3 channel per pixel
        for(size_t i(0); i < pi_PixelsCount*3; i+=3)
            {
            // ((SrcAlpha * (Src - Dst)) / MaxAlpha) + Dst
            pDst[i]   = ((pSrc[3] * (int64_t)(pSrc[0] - pDst[i]))   / 0xFFFF) + pDst[i];
            pDst[i+1] = ((pSrc[3] * (int64_t)(pSrc[1] - pDst[i+1])) / 0xFFFF) + pDst[i+1];
            pDst[i+2] = ((pSrc[3] * (int64_t)(pSrc[2] - pDst[i+2])) / 0xFFFF) + pDst[i+2];

            // Next RGBA
            pSrc+=4;
            }
        };

    virtual const short* GetLostChannels() const override
        {
        return m_LostChannels;
        };

    HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterV64R16G16B16A16_V48R16G16B16(*this));
        }

private:

    static short m_LostChannels[];
    };
short ConverterV64R16G16B16A16_V48R16G16B16::m_LostChannels[] = {3, -1};
static ConverterV64R16G16B16A16_V48R16G16B16 s_V64R16G16B16A16_V48R16G16B16;

//-----------------------------------------------------------------------------
//  Dictionnary of converters from other pixel types
//-----------------------------------------------------------------------------
struct V64R16G16B16A16ConvertersFrom : public MapHRPPixelTypeToConverter
    {
    V64R16G16B16A16ConvertersFrom() : MapHRPPixelTypeToConverter ()
        {
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV64R16G16B16A16::CLASS_ID, &s_V64R16G16B16A16_V64R16G16B16A16));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV24R8G8B8::CLASS_ID, &s_V24R8G8B8_V64R16G16B16A16));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV32R8G8B8A8::CLASS_ID, &s_V32R8G8B8A8_V64R16G16B16A16));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV48R16G16B16::CLASS_ID, &s_V48R16G16B16_V64R16G16B16A16));
        };
    };

//-----------------------------------------------------------------------------
//  Dictionnary of converters from other pixel types
//-----------------------------------------------------------------------------
struct V64R16G16B16A16ConvertersTo : public MapHRPPixelTypeToConverter
    {
    V64R16G16B16A16ConvertersTo() : MapHRPPixelTypeToConverter ()
        {
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV64R16G16B16A16::CLASS_ID, &s_V64R16G16B16A16_V64R16G16B16A16));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV24R8G8B8::CLASS_ID, &s_V64R16G16B16A16_V24R8G8B8));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV32R8G8B8A8::CLASS_ID, &s_V64R16G16B16A16_V32R8G8B8A8));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV48R16G16B16::CLASS_ID, &s_V64R16G16B16A16_V48R16G16B16));
        };
    };

//-----------------------------------------------------------------------------
// Default Constructor
//-----------------------------------------------------------------------------
HRPPixelTypeV64R16G16B16A16::HRPPixelTypeV64R16G16B16A16()
    : HRPPixelTypeRGB(16,16,16,16,0)
    {
    }

//-----------------------------------------------------------------------------
// Copy constructor
//-----------------------------------------------------------------------------
HRPPixelTypeV64R16G16B16A16::HRPPixelTypeV64R16G16B16A16(const HRPPixelTypeV64R16G16B16A16& pi_rObj)
    : HRPPixelTypeRGB(pi_rObj)
    {
    }

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
HRPPixelTypeV64R16G16B16A16::~HRPPixelTypeV64R16G16B16A16()
    {
    }

//-----------------------------------------------------------------------------
// Clone method
//-----------------------------------------------------------------------------
HPMPersistentObject* HRPPixelTypeV64R16G16B16A16::Clone() const
    {
    return new HRPPixelTypeV64R16G16B16A16(*this);
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
unsigned short HRPPixelTypeV64R16G16B16A16::CountValueBits() const
    {
    return 64;
    }

//-----------------------------------------------------------------------------
// HasConverterFrom
//-----------------------------------------------------------------------------
const HRPPixelConverter* HRPPixelTypeV64R16G16B16A16::HasConverterFrom(const HRPPixelType* pi_pPixelTypeFrom) const
    {
    HFCMonitor Monitor(HRPPixelType::s_ConverterAccess);

    // This declaration needs to remain local in order to prevent initialization problems
    static struct V64R16G16B16A16ConvertersFrom s_ConvertersFrom;

    MapHRPPixelTypeToConverter::const_iterator Itr;
    Itr = s_ConvertersFrom.find (pi_pPixelTypeFrom->GetClassID());

    return ((Itr == s_ConvertersFrom.end()) ? 0 : (*Itr).second);
    }

//-----------------------------------------------------------------------------
// HasConverterTo
//-----------------------------------------------------------------------------
const HRPPixelConverter* HRPPixelTypeV64R16G16B16A16::HasConverterTo(const HRPPixelType* pi_pPixelTypeTo) const
    {
    HFCMonitor Monitor(HRPPixelType::s_ConverterAccess);

    // This declaration needs to remain local in order to prevent initialization problems
    static struct V64R16G16B16A16ConvertersTo s_ConvertersTo;

    MapHRPPixelTypeToConverter::const_iterator Itr;
    Itr = s_ConvertersTo.find (pi_pPixelTypeTo->GetClassID());

    return ((Itr == s_ConvertersTo.end()) ? 0 : (*Itr).second);
    }


