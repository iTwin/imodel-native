//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrp/src/HRPPixelTypeV32B8G8R8X8.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HRPPixelTypeV32B8G8R8X8
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HRPPixelTypeV32B8G8R8X8.h>
#include <Imagepp/all/h/HRPPixelTypeV24B8G8R8.h>
#include <Imagepp/all/h/HRPPixelTypeV24R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeV32R8G8B8A8.h>
#include <Imagepp/all/h/HRPPixelTypeV32R8G8B8X8.h>
#include <Imagepp/all/h/HFCMath.h>

HPM_REGISTER_CLASS(HRPPixelTypeV32B8G8R8X8, HRPPixelTypeBGR)

// STL typeDef
typedef map<HCLASS_ID, HRPPixelConverter*, less<HCLASS_ID>, allocator<HRPPixelConverter*> >
MapHRPPixelTypeToConverter;

//-----------------------------------------------------------------------------
//  s_V32B8G8R8X8_V32B8G8R8X8 - Converter
//-----------------------------------------------------------------------------
class ConverterV32B8G8R8X8_V32B8G8R8X8 : public HRPPixelConverter
    {
public:
    DEFINE_T_SUPER(HRPPixelConverter)

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        Byte* pSourceComposite =  (Byte*)pi_pSourceRawData;
        Byte* pDestComposite = (Byte*)pio_pDestRawData;

        // Copy entire bytes
        memcpy(pDestComposite, pSourceComposite, 4 * pi_PixelsCount);
        };

    HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterV32B8G8R8X8_V32B8G8R8X8(*this));
        }
    };
static ConverterV32B8G8R8X8_V32B8G8R8X8        s_V32B8G8R8X8_V32B8G8R8X8;

//-----------------------------------------------------------------------------
//  s_V32B8G8R8X8_V32R8G8B8X8 - Converter
//-----------------------------------------------------------------------------
class ConverterV32B8G8R8X8_V32R8G8B8X8 : public HRPPixelConverter
    {
public:
    DEFINE_T_SUPER(HRPPixelConverter)

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {

        Byte* pSourceComposite =  (Byte*)pi_pSourceRawData;
        Byte* pDestComposite = (Byte*)pio_pDestRawData;

        for(size_t i(0); i < pi_PixelsCount; ++i)
            {
            pDestComposite[0] = pSourceComposite[2];
            pDestComposite[1] = pSourceComposite[1];
            pDestComposite[2] = pSourceComposite[0];
            pDestComposite[3] = pSourceComposite[3];

            pSourceComposite += 4;
            pDestComposite   += 4;
            }
        };

    HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterV32B8G8R8X8_V32R8G8B8X8(*this));
        }
    };
static ConverterV32B8G8R8X8_V32R8G8B8X8        s_V32B8G8R8X8_V32R8G8B8X8;


//-----------------------------------------------------------------------------
//  s_V32B8G8R8X8_V32R8G8B8A8 - Converter
//-----------------------------------------------------------------------------
class ConverterV32B8G8R8X8_V32R8G8B8A8 : public HRPPixelConverter
    {
public:
    DEFINE_T_SUPER(HRPPixelConverter)

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {

        Byte* pSourceComposite =  (Byte*)pi_pSourceRawData;
        Byte* pDestComposite = (Byte*)pio_pDestRawData;

        for(size_t i(0); i < pi_PixelsCount; ++i)
            {
            pDestComposite[0] = pSourceComposite[2];
            pDestComposite[1] = pSourceComposite[1];
            pDestComposite[2] = pSourceComposite[0];
            pDestComposite[3] = 0xFF;

            pSourceComposite += 4;
            pDestComposite   += 4;
            }
        };

    HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterV32B8G8R8X8_V32R8G8B8A8(*this));
        }
    };
static ConverterV32B8G8R8X8_V32R8G8B8A8        s_V32B8G8R8X8_V32R8G8B8A8;

//-----------------------------------------------------------------------------
//  s_V32B8G8R8X8_V24B8G8R8 - Converter
//-----------------------------------------------------------------------------
class ConverterV32B8G8R8X8_V24B8G8R8 : public HRPPixelConverter
    {
public:
    DEFINE_T_SUPER(HRPPixelConverter)

    void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        Byte* pSrc  = (Byte*)pi_pSourceRawData;
        Byte* pDest = (Byte*)pio_pDestRawData;

        while(pi_PixelsCount)
            {
            pDest[0] = pSrc[0];
            pDest[1] = pSrc[1];
            pDest[2] = pSrc[2];

            // One less pixel to go
            pi_PixelsCount--;

            // Increment Source and Destination by 3 bytes each !
            pDest+=3;
            pSrc+=4;
            }
        };

    virtual const short* GetLostChannels() const override
        {
        return m_LostChannels;
        }

    HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterV32B8G8R8X8_V24B8G8R8(*this));
        }

private:
    static short m_LostChannels[];
    };
short ConverterV32B8G8R8X8_V24B8G8R8::m_LostChannels[] = {3, -1};
static ConverterV32B8G8R8X8_V24B8G8R8        s_V32B8G8R8X8_V24B8G8R8;

//-----------------------------------------------------------------------------
//  s_V32R8G8B8X8_V32B8G8R8X8 - Converter
//-----------------------------------------------------------------------------
class ConverterV32R8G8B8X8_V32B8G8R8X8 : public HRPPixelConverter
    {
public:
    DEFINE_T_SUPER(HRPPixelConverter)

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {

        Byte* pSourceComposite =  (Byte*)pi_pSourceRawData;
        Byte* pDestComposite = (Byte*)pio_pDestRawData;

        for(size_t i(0); i < pi_PixelsCount; ++i)
            {
            pDestComposite[0] = pSourceComposite[2];
            pDestComposite[1] = pSourceComposite[1];
            pDestComposite[2] = pSourceComposite[0];
            pDestComposite[3] = pSourceComposite[3];

            pSourceComposite += 4;
            pDestComposite   += 4;
            }
        };

    HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterV32R8G8B8X8_V32B8G8R8X8(*this));
        }
    };
static ConverterV32R8G8B8X8_V32B8G8R8X8        s_V32R8G8B8X8_V32B8G8R8X8;


//-----------------------------------------------------------------------------
//  s_V32R8G8B8A8_V32B8G8R8X8 - Converter
//-----------------------------------------------------------------------------
class ConverterV32R8G8B8A8_V32B8G8R8X8 : public HRPPixelConverter
    {
public:
    DEFINE_T_SUPER(HRPPixelConverter)

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {

        Byte* pSourceComposite =  (Byte*)pi_pSourceRawData;
        Byte* pDestComposite = (Byte*)pio_pDestRawData;

        for(size_t i(0); i < pi_PixelsCount; ++i)
            {
            pDestComposite[0] = pSourceComposite[2];
            pDestComposite[1] = pSourceComposite[1];
            pDestComposite[2] = pSourceComposite[0];
            pDestComposite[3] = 0xFF;

            pSourceComposite += 4;
            pDestComposite   += 4;
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
            // alpha * (S - D) + D
            pDestComposite[2] = pQuotients->DivideBy255ToByte(pSourceComposite[3] * (pSourceComposite[0] - pDestComposite[2])) + pDestComposite[2];
            pDestComposite[1] = pQuotients->DivideBy255ToByte(pSourceComposite[3] * (pSourceComposite[1] - pDestComposite[1])) + pDestComposite[1];
            pDestComposite[0] = pQuotients->DivideBy255ToByte(pSourceComposite[3] * (pSourceComposite[2] - pDestComposite[0])) + pDestComposite[0];
            pDestComposite[3] = 0xFF;

            --pi_PixelsCount;
            pDestComposite+=4;
            pSourceComposite+=4;
            }
        };

    HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterV32R8G8B8A8_V32B8G8R8X8(*this));
        }
    };
static ConverterV32R8G8B8A8_V32B8G8R8X8        s_V32R8G8B8A8_V32B8G8R8X8;

//-----------------------------------------------------------------------------
//  s_V24R8G8B8_V32B8G8R8X8 - Converter
//-----------------------------------------------------------------------------
struct ConverterV24R8G8B8_V32B8G8R8X8 : public HRPPixelConverter
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
            pDestComposite[2] = pSourceComposite[0];
            pDestComposite[1] = pSourceComposite[1];
            pDestComposite[0] = pSourceComposite[2];

            pDestComposite[3] = 0xff; // opaque

            pi_PixelsCount -= 1;
            pDestComposite+=4;
            pSourceComposite+=3;
            }
        };

    HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterV24R8G8B8_V32B8G8R8X8(*this));
        }

    };
static struct ConverterV24R8G8B8_V32B8G8R8X8        s_V24R8G8B8_V32B8G8R8X8;

//-----------------------------------------------------------------------------
//  s_V32B8G8R8X8_V24R8G8B8 - Converter
//-----------------------------------------------------------------------------
struct ConverterV32B8G8R8X8_V24R8G8B8 : public HRPPixelConverter
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
            pDestComposite[0] = pSourceComposite[2];
            pDestComposite[1] = pSourceComposite[1];
            pDestComposite[2] = pSourceComposite[0];

            pi_PixelsCount -= 1;
            pDestComposite+=3;
            pSourceComposite+=4;
            }
        };

    virtual const short* GetLostChannels() const override
        {
        return m_LostChannels;
        }

    HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterV32B8G8R8X8_V24R8G8B8(*this));
        }


private:

    static short m_LostChannels[];

    };
short ConverterV32B8G8R8X8_V24R8G8B8::m_LostChannels[] = {3, -1};
static struct ConverterV32B8G8R8X8_V24R8G8B8        s_V32B8G8R8X8_V24R8G8B8;

//-----------------------------------------------------------------------------
//  Dictionnary of converters from other pixel types
//-----------------------------------------------------------------------------
struct V32B8G8R8X8ConvertersFrom : public MapHRPPixelTypeToConverter
    {
    V32B8G8R8X8ConvertersFrom() : MapHRPPixelTypeToConverter ()
        {
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV32B8G8R8X8::CLASS_ID, &s_V32B8G8R8X8_V32B8G8R8X8));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV32R8G8B8X8::CLASS_ID, &s_V32R8G8B8X8_V32B8G8R8X8));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV32R8G8B8A8::CLASS_ID, &s_V32R8G8B8A8_V32B8G8R8X8));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV24R8G8B8::CLASS_ID, &s_V24R8G8B8_V32B8G8R8X8));
        };
    };

//-----------------------------------------------------------------------------
//  Dictionnary of converters from other pixel types
//-----------------------------------------------------------------------------
struct V32B8G8R8X8ConvertersTo : public MapHRPPixelTypeToConverter
    {
    V32B8G8R8X8ConvertersTo() : MapHRPPixelTypeToConverter ()
        {
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV32B8G8R8X8::CLASS_ID, &s_V32B8G8R8X8_V32B8G8R8X8));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV32R8G8B8X8::CLASS_ID, &s_V32B8G8R8X8_V32R8G8B8X8));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV32R8G8B8A8::CLASS_ID, &s_V32B8G8R8X8_V32R8G8B8A8));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV24B8G8R8::CLASS_ID, &s_V32B8G8R8X8_V24B8G8R8));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV24R8G8B8::CLASS_ID, &s_V32B8G8R8X8_V24R8G8B8));
        };
    };

//-----------------------------------------------------------------------------
// Constructor for true color 24 bits RGB
//-----------------------------------------------------------------------------
HRPPixelTypeV32B8G8R8X8::HRPPixelTypeV32B8G8R8X8()
    : HRPPixelTypeBGR(8,8,8,8,0,false)
    {
    }


//-----------------------------------------------------------------------------
// Copy constructor
//-----------------------------------------------------------------------------
HRPPixelTypeV32B8G8R8X8::HRPPixelTypeV32B8G8R8X8(const HRPPixelTypeV32B8G8R8X8& pi_rObj)
    : HRPPixelTypeBGR(pi_rObj)
    {
    }

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
HRPPixelTypeV32B8G8R8X8::~HRPPixelTypeV32B8G8R8X8()
    {
    }

//-----------------------------------------------------------------------------
// Clone method
//-----------------------------------------------------------------------------
HPMPersistentObject* HRPPixelTypeV32B8G8R8X8::Clone() const
    {
    return new HRPPixelTypeV32B8G8R8X8(*this);
    }

/** -----------------------------------------------------------------------------
    This function is used to know the number of "value" bits contain in a pixel
    of this pixel type.

    @b{Example:} @list{HRPPixelTypeV32B8G8R8X8 should return 32.}
                 @list{HRPPixelTypeI8R8G8B8A8 should return 0.}
                 @list{HRPPixelTypeI8VA8R8G8B8 should return 8.}

    @return The number of "value" bits contain in a pixel of this pixel type.
    @end

    @see HRPPixelType::CountIndexBits()
    @see HRPPixelType::CountPixelRawData()
    @end
    -----------------------------------------------------------------------------
 */
unsigned short HRPPixelTypeV32B8G8R8X8::CountValueBits() const
    {
    return 32;
    }

//-----------------------------------------------------------------------------
// HasConverterFrom
//-----------------------------------------------------------------------------
const HRPPixelConverter* HRPPixelTypeV32B8G8R8X8::HasConverterFrom(const HRPPixelType* pi_pPixelTypeFrom) const
    {
    HFCMonitor Monitor(HRPPixelType::s_ConverterAccess);

    // This declaration needs to remain local in order to prevent initialization problems
    static struct V32B8G8R8X8ConvertersFrom s_ConvertersFrom;

    MapHRPPixelTypeToConverter::const_iterator Itr;
    Itr = s_ConvertersFrom.find (pi_pPixelTypeFrom->GetClassID());

    return ((Itr == s_ConvertersFrom.end()) ? 0 : (*Itr).second);
    }

//-----------------------------------------------------------------------------
// HasConverterTo
//-----------------------------------------------------------------------------
const HRPPixelConverter* HRPPixelTypeV32B8G8R8X8::HasConverterTo(const HRPPixelType* pi_pPixelTypeTo) const
    {
    HFCMonitor Monitor(HRPPixelType::s_ConverterAccess);

    // This declaration needs to remain local in order to prevent initialization problems
    static struct V32B8G8R8X8ConvertersTo s_ConvertersTo;

    MapHRPPixelTypeToConverter::const_iterator Itr;
    Itr = s_ConvertersTo.find (pi_pPixelTypeTo->GetClassID());

    return ((Itr == s_ConvertersTo.end()) ? 0 : (*Itr).second);
    }

