//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrp/src/HRPPixelTypeV32A8R8G8B8.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HRPPixelTypeV32A8R8G8B8
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HRPPixelTypeV32A8R8G8B8.h>
#include <Imagepp/all/h/HRPPixelConverter.h>
#include <Imagepp/all/h/HRPChannelOrgARGB.h>

#include <Imagepp/all/h/HRPPaletteOctreeR8G8B8.h>

#include <Imagepp/all/h/HRPPixelTypeI8R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeV24R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeV32R8G8B8A8.h>
#include <Imagepp/all/h/HFCMath.h>

HPM_REGISTER_CLASS(HRPPixelTypeV32A8R8G8B8, HRPPixelType)

// STL typeDef
typedef map<HCLASS_ID, HRPPixelConverter*, less<HCLASS_ID>, allocator<HRPPixelConverter*> >
MapHRPPixelTypeToConverter;

//-----------------------------------------------------------------------------
//  s_V32A8R8G8B8_V32A8R8G8B8 - Converter
//-----------------------------------------------------------------------------
class ConverterV32A8R8G8B8_V32A8R8G8B8 : public HRPPixelConverter
    {
public:
    DEFINE_T_SUPER(HRPPixelConverter)

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        memcpy((Byte*)pio_pDestRawData, (Byte*)pi_pSourceRawData, pi_PixelsCount*4);
        };

    virtual void ConvertLostChannel(   const void* pi_pSourceRawData,
                            void* pio_pDestRawData,
                            size_t pi_PixelsCount,
                            const bool* pi_pChannelsMask) const override
        {
        // &&Mart
        // Not OK. Should work with ints...

        Byte* pSourceComposite =  (Byte*)pi_pSourceRawData;
        Byte* pDestComposite = (Byte*)pio_pDestRawData;

        // Copy entire bytes
        while(pi_PixelsCount)
            {
            if(pi_pChannelsMask[0])
                {
                pDestComposite[0] = pSourceComposite[0];
                }

            pi_PixelsCount -= 1;
            pDestComposite +=4;
            pSourceComposite+=4;
            }
        };

    virtual void Compose(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {

        Byte* pSourceComposite =  (Byte*)pi_pSourceRawData;
        Byte* pDestComposite = (Byte*)pio_pDestRawData;
        Byte PremultDestColor;

        HFCMath (*pQuotients) (HFCMath::GetInstance());

        // Copy entire bytes
        while(pi_PixelsCount)
            {
            // If source pixel is fully transparent, destination is unaltered
            if (pSourceComposite[0] != 0)
                {
                if (pDestComposite[0] == 0 || pSourceComposite[0] == 255)
                    {
                    // Destination pixel is fully transparent, or source pixel
                    // is fully opaque. Copy source pixel,
                    *((uint32_t*)pDestComposite) = *((uint32_t*)pSourceComposite);
                    }
                else
                    {
                    // Cdst' = Asrc * (Csrc - (Adst * Cdst)) + (Adst * Cdst)
                    // Alphas are in [0, 255]. Tweak -> shift to divide, slight error induced.
                    PremultDestColor = pQuotients->DivideBy255ToByte(pDestComposite[0] * pDestComposite[1]);
                    pDestComposite[1] = pQuotients->DivideBy255ToByte(pSourceComposite[0] * (pSourceComposite[1] - PremultDestColor)) + PremultDestColor;

                    PremultDestColor = pQuotients->DivideBy255ToByte(pDestComposite[0] * pDestComposite[2]);
                    pDestComposite[2] = pQuotients->DivideBy255ToByte(pSourceComposite[0] * (pSourceComposite[2] - PremultDestColor)) + PremultDestColor;

                    PremultDestColor = pQuotients->DivideBy255ToByte(pDestComposite[0] * pDestComposite[3]);
                    pDestComposite[3] = pQuotients->DivideBy255ToByte(pSourceComposite[0] * (pSourceComposite[3] - PremultDestColor)) + PremultDestColor;

                    // Adst' = 1 - ( (1 - Asrc) * (1 - Adst) )
                    // --> Transparency percentages are multiplied
                    pDestComposite[0] = 255 - (pQuotients->DivideBy255ToByte((255 - pSourceComposite[0]) * (255 - pDestComposite[0])));
                    }
                }

            --pi_PixelsCount;
            pDestComposite += 4;
            pSourceComposite += 4;
            }
        };

    HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterV32A8R8G8B8_V32A8R8G8B8(*this));
        }
    };
static ConverterV32A8R8G8B8_V32A8R8G8B8        s_V32A8R8G8B8_V32A8R8G8B8;

//-----------------------------------------------------------------------------
//  s_V24R8G8B8_V32A8R8G8B8 - Converter
//-----------------------------------------------------------------------------
class ConverterV24R8G8B8_V32A8R8G8B8 : public HRPPixelConverter
    {
public:
    DEFINE_T_SUPER(HRPPixelConverter)

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {

        Byte* pSourceComposite =  (Byte*)pi_pSourceRawData;
        uint32_t* pDestComposite = (uint32_t*)pio_pDestRawData;

        // Copy entire bytes
        while(pi_PixelsCount)
            {
            *pDestComposite = (pSourceComposite[2] << 24) |
                              (pSourceComposite[1] << 16) |
                              (pSourceComposite[0] << 8) |
                              255;

            pi_PixelsCount -= 1;
            pDestComposite++;
            pSourceComposite+=3;
            }
        };

    HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterV24R8G8B8_V32A8R8G8B8(*this));
        }

    };
static ConverterV24R8G8B8_V32A8R8G8B8        s_V24R8G8B8_V32A8R8G8B8;


//-----------------------------------------------------------------------------
//  s_I8R8G8B8_V32A8R8G8B8 - Converter
//-----------------------------------------------------------------------------
struct ConverterI8R8G8B8_V32A8R8G8B8 : public HRPPixelConverter
    {
public:
    DEFINE_T_SUPER(HRPPixelConverter)

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {

        Byte* pSrc =  (Byte*)pi_pSourceRawData;
        uint32_t* pDest = (uint32_t*)pio_pDestRawData;
        const HRPPixelPalette& rPalette = GetSourcePixelType()->GetPalette();

        // Copy entire bytes
        while(pi_PixelsCount)
            {
            Byte* pSourceComposite = (Byte*)rPalette.GetCompositeValue(*pSrc);

            *pDest = (pSourceComposite[2] << 24) |
                     (pSourceComposite[1] << 16) |
                     (pSourceComposite[0] << 8) |
                     255;

            pi_PixelsCount--;
            pDest++;
            pSrc++;
            }
        };

    HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterI8R8G8B8_V32A8R8G8B8(*this));
        }

    };
static ConverterI8R8G8B8_V32A8R8G8B8        s_I8R8G8B8_V32A8R8G8B8;


//-----------------------------------------------------------------------------
//  s_V32R8G8B8A8_V32A8R8G8B8 - Converter
//-----------------------------------------------------------------------------
struct ConverterV32R8G8B8A8_V32A8R8G8B8 : public HRPPixelConverter
    {
public:
    DEFINE_T_SUPER(HRPPixelConverter)

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {

        Byte* pSourceComposite =  (Byte*)pi_pSourceRawData;
        uint32_t* pDestComposite = (uint32_t*)pio_pDestRawData;

        // Copy entire bytes
        while(pi_PixelsCount)
            {
            // Destination is ARGB in an integer
            *pDestComposite = (pSourceComposite[2] << 24) |
                              (pSourceComposite[1] << 16) |
                              (pSourceComposite[0] << 8) |
                              pSourceComposite[3];

            ++pDestComposite;
            pi_PixelsCount -= 1;
            pSourceComposite+=4;
            }
        };

    virtual void Compose(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        Byte* pSourceComposite =  (Byte*)pi_pSourceRawData;
        Byte* pDestComposite = (Byte*)pio_pDestRawData;

        HFCMath (*pQuotients) (HFCMath::GetInstance());

        // Copy entire bytes
        while(pi_PixelsCount)
            {
            // If source pixel is fully transparent, destination is unaltered
            if (pSourceComposite[3] != 0)
                {
                if (pDestComposite[0] == 0 || pSourceComposite[3] == 255)
                    {
                    // Destination pixel is fully transparent, or source pixel is fully opaque. Copy source pixel.
                    pDestComposite[0] = pSourceComposite[3];
                    pDestComposite[1] = pSourceComposite[0];
                    pDestComposite[2] = pSourceComposite[1];
                    pDestComposite[3] = pSourceComposite[2];
                    }
                else
                    {
                    // Cdst' = Asrc * (Csrc - (Adst * Cdst)) + (Adst * Cdst)
                    // Alphas are in [0, 255]. Tweak -> shift to divide, slight error induced.
                    Byte PremultDestColor = pQuotients->DivideBy255ToByte(pDestComposite[0] * pDestComposite[1]);
                    pDestComposite[1] = pQuotients->DivideBy255ToByte(pSourceComposite[3] * (pSourceComposite[0] - PremultDestColor)) + PremultDestColor;

                    PremultDestColor = pQuotients->DivideBy255ToByte(pDestComposite[0] * pDestComposite[2]);
                    pDestComposite[2] = pQuotients->DivideBy255ToByte(pSourceComposite[3] * (pSourceComposite[1] - PremultDestColor)) + PremultDestColor;

                    PremultDestColor = pQuotients->DivideBy255ToByte(pDestComposite[0] * pDestComposite[3]);
                    pDestComposite[3] = pQuotients->DivideBy255ToByte(pSourceComposite[3] * (pSourceComposite[2] - PremultDestColor)) + PremultDestColor;

                    // Adst' = 1 - ( (1 - Asrc) * (1 - Adst) )
                    // --> Transparency percentages are multiplied
                    pDestComposite[0] = 255 - (pQuotients->DivideBy255ToByte((255 - pSourceComposite[3]) * (255 - pDestComposite[0])));
                    }
                }

            --pi_PixelsCount;
            pDestComposite += 4;
            pSourceComposite += 4;
            }
        };

    HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterV32R8G8B8A8_V32A8R8G8B8(*this));
        }

    };
static struct ConverterV32R8G8B8A8_V32A8R8G8B8        s_V32R8G8B8A8_V32A8R8G8B8;

//-----------------------------------------------------------------------------
//  s_V32A8R8G8B8_V32R8G8B8A8 - Converter
//-----------------------------------------------------------------------------
struct ConverterV32A8R8G8B8_V32R8G8B8A8 : public HRPPixelConverter
    {
public:
    DEFINE_T_SUPER(HRPPixelConverter)

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        Byte* pSourceComposite =  (Byte*)pi_pSourceRawData;
        Byte* pDestComposite = (Byte*)pio_pDestRawData;

        // Copy entire bytes
        while(pi_PixelsCount)
            {
            pDestComposite[0] = pSourceComposite[1];
            pDestComposite[1] = pSourceComposite[2];
            pDestComposite[2] = pSourceComposite[3];
            pDestComposite[3] = pSourceComposite[0];

            pi_PixelsCount -= 1;
            pDestComposite+=4;
            pSourceComposite+=4;
            }
        };

    virtual void Compose(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        Byte* pSourceComposite =  (Byte*)pi_pSourceRawData;
        Byte* pDestComposite = (Byte*)pio_pDestRawData;

        HFCMath (*pQuotients) (HFCMath::GetInstance());

        // Copy entire bytes
        while(pi_PixelsCount)
            {
            // If source pixel is fully transparent, destination is unaltered
            if (pSourceComposite[0] != 0)
                {
                if (pDestComposite[3] == 0 || pSourceComposite[0] == 255)
                    {
                    // Destination pixel is fully transparent, or source pixel is fully opaque. Copy source pixel.
                    pDestComposite[0] = pSourceComposite[1];
                    pDestComposite[1] = pSourceComposite[2];
                    pDestComposite[2] = pSourceComposite[3];
                    pDestComposite[3] = pSourceComposite[0];
                    }
                else
                    {
                    // Cdst' = Asrc * (Csrc - (Adst * Cdst)) + (Adst * Cdst)
                    // Alphas are in [0, 255]. Tweak -> shift to divide, slight error induced.
                    Byte PremultDestColor = pQuotients->DivideBy255ToByte(pDestComposite[3] * pDestComposite[0]);
                    pDestComposite[0] = pQuotients->DivideBy255ToByte(pSourceComposite[0] * (pSourceComposite[1] - PremultDestColor)) + PremultDestColor;

                    PremultDestColor = pQuotients->DivideBy255ToByte(pDestComposite[3] * pDestComposite[1]);
                    pDestComposite[1] = pQuotients->DivideBy255ToByte(pSourceComposite[0] * (pSourceComposite[2] - PremultDestColor)) + PremultDestColor;

                    PremultDestColor = pQuotients->DivideBy255ToByte(pDestComposite[3] * pDestComposite[2]);
                    pDestComposite[2] = pQuotients->DivideBy255ToByte(pSourceComposite[0] * (pSourceComposite[3] - PremultDestColor)) + PremultDestColor;

                    // Adst' = 1 - ( (1 - Asrc) * (1 - Adst) )
                    // --> Transparency percentages are multiplied
                    pDestComposite[3] = 255 - (pQuotients->DivideBy255ToByte((255 - pSourceComposite[0]) * (255 - pDestComposite[3])));
                    }
                }

            --pi_PixelsCount;
            pDestComposite += 4;
            pSourceComposite += 4;
            }
        };

    HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterV32A8R8G8B8_V32R8G8B8A8(*this));
        }
    };
static struct ConverterV32A8R8G8B8_V32R8G8B8A8        s_V32A8R8G8B8_V32R8G8B8A8;


//-----------------------------------------------------------------------------
//  Dictionnary of converters from other pixel types
//-----------------------------------------------------------------------------
struct V32A8R8G8B8ConvertersFrom : public MapHRPPixelTypeToConverter
    {
    V32A8R8G8B8ConvertersFrom() : MapHRPPixelTypeToConverter ()
        {
        // XXX to implement V32A8R8G8B8, I8R8G8B8A8, V24B8G8R8, V1Gray1, V8Gray8
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV32A8R8G8B8::CLASS_ID, &s_V32A8R8G8B8_V32A8R8G8B8));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV24R8G8B8::CLASS_ID, &s_V24R8G8B8_V32A8R8G8B8));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeI8R8G8B8::CLASS_ID, &s_I8R8G8B8_V32A8R8G8B8));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV32R8G8B8A8::CLASS_ID, &s_V32R8G8B8A8_V32A8R8G8B8));
        };
    };

//-----------------------------------------------------------------------------
//  Dictionnary of converters from other pixel types
//-----------------------------------------------------------------------------
struct V32A8R8G8B8ConvertersTo : public MapHRPPixelTypeToConverter
    {
    V32A8R8G8B8ConvertersTo() : MapHRPPixelTypeToConverter ()
        {
        // XXX to implement V32A8R8G8B8, I8R8G8B8A8, V24R8G8B8, V1Gray1, V8Gray8
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV32A8R8G8B8::CLASS_ID, &s_V32A8R8G8B8_V32A8R8G8B8));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV32R8G8B8A8::CLASS_ID, &s_V32A8R8G8B8_V32R8G8B8A8));
        //        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV24B8G8R8::CLASS_ID, &s_V32A8R8G8B8_V24B8G8R8));
//        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV24R8G8B8::CLASS_ID, &s_V32A8R8G8B8_V24R8G8B8));
//        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeI8R8G8B8::CLASS_ID, &s_V32A8R8G8B8_I8R8G8B8));
        };
    };

//-----------------------------------------------------------------------------
// Constructor for true color 24 bits RGB
//-----------------------------------------------------------------------------
HRPPixelTypeV32A8R8G8B8::HRPPixelTypeV32A8R8G8B8()
    : HRPPixelType(HRPChannelOrgARGB(), 0)
    {
    }

//-----------------------------------------------------------------------------
// Copy constructor
//-----------------------------------------------------------------------------
HRPPixelTypeV32A8R8G8B8::HRPPixelTypeV32A8R8G8B8(const HRPPixelTypeV32A8R8G8B8& pi_rObj)
    : HRPPixelType(pi_rObj)
    {
    }

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
HRPPixelTypeV32A8R8G8B8::~HRPPixelTypeV32A8R8G8B8()
    {
    }

//-----------------------------------------------------------------------------
// Clone method
//-----------------------------------------------------------------------------
HPMPersistentObject* HRPPixelTypeV32A8R8G8B8::Clone() const
    {
    return new HRPPixelTypeV32A8R8G8B8(*this);
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
unsigned short HRPPixelTypeV32A8R8G8B8::CountValueBits() const
    {
    return 32;
    }

//-----------------------------------------------------------------------------
// HasConverterFrom
//-----------------------------------------------------------------------------
const HRPPixelConverter* HRPPixelTypeV32A8R8G8B8::HasConverterFrom(const HRPPixelType* pi_pPixelTypeFrom) const
    {
    HFCMonitor Monitor(HRPPixelType::s_ConverterAccess);

    // This declaration needs to remain local in order to prevent initialization problems
    static struct V32A8R8G8B8ConvertersFrom s_ConvertersFrom;

    MapHRPPixelTypeToConverter::const_iterator Itr;
    Itr = s_ConvertersFrom.find (pi_pPixelTypeFrom->GetClassID());

    return ((Itr == s_ConvertersFrom.end()) ? 0 : (*Itr).second);
    }

//-----------------------------------------------------------------------------
// HasConverterTo
//-----------------------------------------------------------------------------
const HRPPixelConverter* HRPPixelTypeV32A8R8G8B8::HasConverterTo(const HRPPixelType* pi_pPixelTypeTo) const
    {
    HFCMonitor Monitor(HRPPixelType::s_ConverterAccess);

    // This declaration needs to remain local in order to prevent initialization problems
    static struct V32A8R8G8B8ConvertersTo s_ConvertersTo;

    MapHRPPixelTypeToConverter::const_iterator Itr;
    Itr = s_ConvertersTo.find (pi_pPixelTypeTo->GetClassID());

    return ((Itr == s_ConvertersTo.end()) ? 0 : (*Itr).second);
    }

