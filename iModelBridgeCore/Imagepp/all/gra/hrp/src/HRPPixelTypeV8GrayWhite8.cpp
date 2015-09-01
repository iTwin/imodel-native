//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrp/src/HRPPixelTypeV8GrayWhite8.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HRPPixelTypeV8GrayWhite8
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HRPPixelTypeV8GrayWhite8.h>
#include <Imagepp/all/h/HRPPixelConverter.h>
#include <Imagepp/all/h/HRPChannelOrgGrayWhite.h>
#include <Imagepp/all/h/HRPPixelTypeV32R8G8B8A8.h>

#include <Imagepp/all/h/HFCMath.h>

HPM_REGISTER_CLASS(HRPPixelTypeV8GrayWhite8, HRPPixelType)

// STL typeDef
typedef map<HCLASS_ID, HRPPixelConverter*, less<HCLASS_ID>, allocator<HRPPixelConverter*> >
MapHRPPixelTypeToConverter;

//-----------------------------------------------------------------------------
//  s_V8GrayWhite8_V8GrayWhite8 - Converter between HRPPixelTypeV8GrayWhite8
//-----------------------------------------------------------------------------
struct ConverterV8GrayWhite8_V8GrayWhite8 : public HRPPixelConverter
    {
    DEFINE_T_SUPER(HRPPixelConverter)

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        // Save a function call if only one pixel to copy
        if(pi_PixelsCount == 1)
            *((Byte*)pio_pDestRawData) = *((Byte*)pi_pSourceRawData);
        else
            memcpy(pio_pDestRawData, pi_pSourceRawData, pi_PixelsCount);
        };

    virtual HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterV8GrayWhite8_V8GrayWhite8(*this));
        }
    };
struct ConverterV8GrayWhite8_V8GrayWhite8 s_V8GrayWhite8_V8GrayWhite8;

//-----------------------------------------------------------------------------
//  s_V32R8G8B8A8_V8GrayWhite8 - Converter
//-----------------------------------------------------------------------------
class ConverterV32R8G8B8A8_V8GrayWhite8 : public HRPPixelConverter
    {
public:
    DEFINE_T_SUPER(HRPPixelConverter)

    void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        Byte* pSrc  = (Byte*)pi_pSourceRawData;
        Byte* pDest = (Byte*)pio_pDestRawData;

        while(pi_PixelsCount)
            {
            pDest[0] = (Byte)(255 - uint32_t(pSrc[0] * REDFACTOR + pSrc[1] * GREENFACTOR + pSrc[2] * BLUEFACTOR));
            pi_PixelsCount--;

            // Increment Destination
            pDest++;

            // Increment Source by 3 bytes !
            pSrc+=4;
            }
        };

    virtual void Compose(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        Byte* pSrc = (Byte*)pi_pSourceRawData;
        Byte* pDestComposite = (Byte*)pio_pDestRawData;
        Byte Gray;

        HFCMath (*pQuotients) (HFCMath::GetInstance());

        // Copy entire bytes
        while(pi_PixelsCount)
            {
            // calculate an average gray for the RGB value
            Gray = Byte(pSrc[0] * REDFACTOR + pSrc[1] * GREENFACTOR + pSrc[2] * BLUEFACTOR);

            // (S * alpha) + (D * (1 - alpha))
            *pDestComposite = 255 - pQuotients->DivideBy255ToByte((Gray * pSrc[3]) + ((255-*pDestComposite) * (255 - pSrc[3])));

            --pi_PixelsCount;
            ++pDestComposite;
            pSrc += 4;
            }
        };

    virtual const short* GetLostChannels() const override
        {
        return m_LostChannels;
        }

    HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterV32R8G8B8A8_V8GrayWhite8(*this));
        }

private:

    static short m_LostChannels[];
    };
short ConverterV32R8G8B8A8_V8GrayWhite8::m_LostChannels[] = {3, -1};
static ConverterV32R8G8B8A8_V8GrayWhite8 s_V32R8G8B8A8_V8GrayWhite8;

//-----------------------------------------------------------------------------
//  Dictionnary of converters from other pixel types
//-----------------------------------------------------------------------------
struct V8GrayWhite8ConvertersFrom : public MapHRPPixelTypeToConverter
    {
    V8GrayWhite8ConvertersFrom() : MapHRPPixelTypeToConverter()
        {
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV8GrayWhite8::CLASS_ID, &s_V8GrayWhite8_V8GrayWhite8));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV32R8G8B8A8::CLASS_ID, &s_V32R8G8B8A8_V8GrayWhite8));
        };
    };

//-----------------------------------------------------------------------------
//  Dictionnary of converters from other pixel types
//-----------------------------------------------------------------------------
struct V8GrayWhite8ConvertersTo : public MapHRPPixelTypeToConverter
    {
    V8GrayWhite8ConvertersTo()
        {
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV8GrayWhite8::CLASS_ID, &s_V8GrayWhite8_V8GrayWhite8));
        };
    };

//-----------------------------------------------------------------------------
// Constructor for 8 bits grayscale
//-----------------------------------------------------------------------------
HRPPixelTypeV8GrayWhite8::HRPPixelTypeV8GrayWhite8()
    : HRPPixelType(HRPChannelOrgGrayWhite(8,
                                          HRPChannelType::UNUSED,
                                          HRPChannelType::VOID_CH,
                                          0),
                   0)
    {
    }

//-----------------------------------------------------------------------------
// Copy constructor
//-----------------------------------------------------------------------------
HRPPixelTypeV8GrayWhite8::HRPPixelTypeV8GrayWhite8(const HRPPixelTypeV8GrayWhite8& pi_rObj)
    : HRPPixelType(pi_rObj)
    {
    }

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
HRPPixelTypeV8GrayWhite8::~HRPPixelTypeV8GrayWhite8()
    {
    }

//-----------------------------------------------------------------------------
// Clone method
//-----------------------------------------------------------------------------
HPMPersistentObject* HRPPixelTypeV8GrayWhite8::Clone() const
    {
    return new HRPPixelTypeV8GrayWhite8(*this);
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
unsigned short HRPPixelTypeV8GrayWhite8::CountValueBits() const
    {
    return 8;
    }

//-----------------------------------------------------------------------------
// HasConverterFrom
//-----------------------------------------------------------------------------
const HRPPixelConverter* HRPPixelTypeV8GrayWhite8::HasConverterFrom(const HRPPixelType* pi_pPixelTypeFrom) const
    {
    HFCMonitor Monitor(HRPPixelType::s_ConverterAccess);

    // This declaration needs to remain local in order to prevent initialization problems
    static struct V8GrayWhite8ConvertersFrom s_ConvertersFrom;

    MapHRPPixelTypeToConverter::const_iterator Itr;
    Itr = s_ConvertersFrom.find (pi_pPixelTypeFrom->GetClassID());

    return ((Itr == s_ConvertersFrom.end()) ? 0 : (*Itr).second);
    }

//-----------------------------------------------------------------------------
// HasConverterTo
//-----------------------------------------------------------------------------
const HRPPixelConverter* HRPPixelTypeV8GrayWhite8::HasConverterTo(const HRPPixelType* pi_pPixelTypeTo) const
    {
    HFCMonitor Monitor(HRPPixelType::s_ConverterAccess);

    // This declaration needs to remain local in order to prevent initialization problems
    static struct V8GrayWhite8ConvertersTo s_ConvertersTo;

    MapHRPPixelTypeToConverter::const_iterator Itr;
    Itr = s_ConvertersTo.find (pi_pPixelTypeTo->GetClassID());

    return ((Itr == s_ConvertersTo.end()) ? 0 : (*Itr).second);
    }


