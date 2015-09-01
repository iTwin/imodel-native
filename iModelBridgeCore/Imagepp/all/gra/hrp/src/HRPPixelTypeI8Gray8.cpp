//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrp/src/HRPPixelTypeI8Gray8.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HRPPixelTypeI8Gray8
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HRPPixelTypeI8Gray8.h>
#include <Imagepp/all/h/HRPPixelTypeV8Gray8.h>
#include <Imagepp/all/h/HRPPixelTypeV1Gray1.h>
#include <Imagepp/all/h/HRPPixelTypeV24R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeV32R8G8B8A8.h>
#include <Imagepp/all/h/HFCMath.h>


HPM_REGISTER_CLASS(HRPPixelTypeI8Gray8, HRPPixelTypeI8R8G8B8)

typedef map<HCLASS_ID, HRPPixelConverter*, less<HCLASS_ID>, allocator<HRPPixelConverter*> >
MapHRPPixelTypeToConverter;

static Byte s_BitMask[8]    = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };

//-----------------------------------------------------------------------------
//  s_I8Gray8_I8Gray8 - Converter between HRPPixelTypeI8Gray8
//-----------------------------------------------------------------------------
class ConverterI8Gray8_I8Gray8 : public HRPPixelConverter
    {
public:
    DEFINE_T_SUPER(HRPPixelConverter)

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        Byte* pSrc = (Byte*)pi_pSourceRawData;
        Byte* pDest = (Byte*)pio_pDestRawData;

        memcpy(pDest, pSrc, pi_PixelsCount);
        };

    virtual HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterI8Gray8_I8Gray8(*this));
        };
    };

static ConverterI8Gray8_I8Gray8 s_I8Gray8_I8Gray8;

//-----------------------------------------------------------------------------
//  s_I8Gray8_V24R8G8B8 - Converter
//-----------------------------------------------------------------------------
class ConverterI8Gray8_V24R8G8B8 : public HRPPixelConverter
    {
public:
    DEFINE_T_SUPER(HRPPixelConverter)

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        Byte* pSrc = (Byte*)pi_pSourceRawData;
        Byte* pDest = (Byte*)pio_pDestRawData;

        while(pi_PixelsCount)
            {
            // Set this output pixel
            *pDest++ = *pSrc;
            *pDest++ = *pSrc;
            *pDest++ = *pSrc++;
            pi_PixelsCount--;
            }
        };

    virtual HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterI8Gray8_V24R8G8B8(*this));
        };
    };

static ConverterI8Gray8_V24R8G8B8 s_I8Gray8_V24R8G8B8;


//-----------------------------------------------------------------------------
//  s_V24R8G8B8_I8Gray8 - Converter
//-----------------------------------------------------------------------------
class ConverterV24R8G8B8_I8Gray8 : public HRPPixelConverter
    {
public:
    DEFINE_T_SUPER(HRPPixelConverter)

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        Byte* pSrc = (Byte*)pi_pSourceRawData;
        Byte* pDest = (Byte*)pio_pDestRawData;

        while(pi_PixelsCount)
            {
            *pDest = Byte(pSrc[0] * REDFACTOR +
                            pSrc[1] * GREENFACTOR +
                            pSrc[2] * BLUEFACTOR);
            pSrc += 3;
            pDest++;
            pi_PixelsCount--;
            }
        };

    virtual HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterV24R8G8B8_I8Gray8 (*this));
        };
    };

static ConverterV24R8G8B8_I8Gray8 s_V24R8G8B8_I8Gray8;


//-----------------------------------------------------------------------------
//  s_I8Gray8_V32R8G8B8A8 - Converter
//-----------------------------------------------------------------------------
class ConverterI8Gray8_V32R8G8B8A8 : public HRPPixelConverter
    {
public:
    DEFINE_T_SUPER(HRPPixelConverter)

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        Byte* pSrc = (Byte*)pi_pSourceRawData;
        Byte* pDest = (Byte*)pio_pDestRawData;

        while(pi_PixelsCount)
            {
            // Set this output pixel
            *pDest++ = *pSrc;
            *pDest++ = *pSrc;
            *pDest++ = *pSrc++;
            *pDest++ = 0xff;    // opaque
            pi_PixelsCount--;
            }
        };

    virtual HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterI8Gray8_V32R8G8B8A8(*this));
        };
    };

static ConverterI8Gray8_V32R8G8B8A8 s_I8Gray8_V32R8G8B8A8;

//-----------------------------------------------------------------------------
//  s_V32R8G8B8A8_I8Gray8 - Converter
//-----------------------------------------------------------------------------
class ConverterV32R8G8B8A8_I8Gray8 : public HRPPixelConverter
    {
public:
    DEFINE_T_SUPER(HRPPixelConverter)

    void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        Byte* pSrc  = (Byte*)pi_pSourceRawData;
        Byte* pDest = (Byte*)pio_pDestRawData;

        while(pi_PixelsCount)
            {
            pDest[0] = Byte(pSrc[0] * REDFACTOR +
                              pSrc[1] * GREENFACTOR +
                              pSrc[2] * BLUEFACTOR);
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
            Gray = Byte(pSrc[0] * REDFACTOR +
                          pSrc[1] * GREENFACTOR +
                          pSrc[2] * BLUEFACTOR);

            // (S * alpha) + (D * (1 - alpha))
            *pDestComposite = pQuotients->DivideBy255ToByte((Gray * pSrc[3]) + (*pDestComposite * (255 - pSrc[3])));

            pi_PixelsCount -= 1;
            pDestComposite++;
            pSrc += 4;
            }
        };

    virtual const short* GetLostChannels() const override
        {
        return m_LostChannels;
        }

    HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterV32R8G8B8A8_I8Gray8(*this));
        }

private:

    static short m_LostChannels[];
    };
short ConverterV32R8G8B8A8_I8Gray8::m_LostChannels[] = {3, -1};
static ConverterV32R8G8B8A8_I8Gray8 s_V32R8G8B8A8_I8Gray8;

//-----------------------------------------------------------------------------
//  s_I8Gray8_V8Gray8 - Converter
//-----------------------------------------------------------------------------
struct ConverterI8Gray8_V8Gray8 : public HRPPixelConverter
    {
    DEFINE_T_SUPER(HRPPixelConverter)


    void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        memcpy(pio_pDestRawData, pi_pSourceRawData, pi_PixelsCount);
        };

    virtual HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterI8Gray8_V8Gray8(*this));
        }
    };
static struct ConverterI8Gray8_V8Gray8 s_I8Gray8_V8Gray8;

//-----------------------------------------------------------------------------
//  s_V8Gray8_I8Gray8 - Converter
//-----------------------------------------------------------------------------
struct ConverterV8Gray8_I8Gray8 : public HRPPixelConverter
    {
public:
    DEFINE_T_SUPER(HRPPixelConverter)

    ConverterV8Gray8_I8Gray8()
        {
        };

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        memcpy(pio_pDestRawData, pi_pSourceRawData, pi_PixelsCount);
        };

    virtual HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterV8Gray8_I8Gray8(*this));
        }

    };
static struct ConverterV8Gray8_I8Gray8 s_V8Gray8_I8Gray8;

//-----------------------------------------------------------------------------
//  s_V1Gray1_I8Gray8 - Converter
//-----------------------------------------------------------------------------
class ConverterV1Gray1_I8Gray8 : public HRPPixelConverter
    {
public:
    DEFINE_T_SUPER(HRPPixelConverter)


    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        Byte* pSrc =  (Byte*)pi_pSourceRawData;
        Byte* pDest = (Byte*)pio_pDestRawData;
        Byte Index = 0;

        while(pi_PixelsCount)
            {
            // Set this output pixel
            *pDest = (*pSrc & s_BitMask[Index] ? 255 : 0);

            // One less pixel to go
            pi_PixelsCount--;

            // Increment Destination
            pDest++;

            // Increment Source
            Index++;
            if(Index == 8)
                {
                // Start reading from a new source byte
                Index = 0;
                pSrc++;
                }
            }
        };

    virtual HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterV1Gray1_I8Gray8(*this));
        }

    };
static ConverterV1Gray1_I8Gray8 s_V1Gray1_I8Gray8;

//-----------------------------------------------------------------------------
//  Dictionnary of converters from other pixel types
//-----------------------------------------------------------------------------
struct I8Gray8ConvertersFrom : public MapHRPPixelTypeToConverter
    {
    I8Gray8ConvertersFrom() : MapHRPPixelTypeToConverter ()
        {
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeI8Gray8::CLASS_ID, &s_I8Gray8_I8Gray8));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV8Gray8::CLASS_ID, &s_V8Gray8_I8Gray8));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV1Gray1::CLASS_ID, &s_V1Gray1_I8Gray8));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV24R8G8B8::CLASS_ID, &s_V24R8G8B8_I8Gray8));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV32R8G8B8A8::CLASS_ID, &s_V32R8G8B8A8_I8Gray8));
        };
    };

//-----------------------------------------------------------------------------
//  Dictionnary of converters from other pixel types
//-----------------------------------------------------------------------------
struct I8Gray8ConvertersTo : public MapHRPPixelTypeToConverter
    {
    I8Gray8ConvertersTo() : MapHRPPixelTypeToConverter ()
        {
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeI8Gray8::CLASS_ID, &s_I8Gray8_I8Gray8));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV8Gray8::CLASS_ID, &s_I8Gray8_V8Gray8));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV24R8G8B8::CLASS_ID, &s_I8Gray8_V24R8G8B8));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV32R8G8B8A8::CLASS_ID, &s_I8Gray8_V32R8G8B8A8));
        };
    };


//-----------------------------------------------------------------------------
// Constructor for Index 8 bits with palette true color 24 bits RGB
//-----------------------------------------------------------------------------
HRPPixelTypeI8Gray8::HRPPixelTypeI8Gray8()
    : HRPPixelTypeI8R8G8B8()
    {
    // by default, the palette of HRPPixelTypeI8R8G8B8 is gray scale
    (const_cast<HRPPixelPalette&>(GetPalette())).SetReadOnly(true);
    }



//-----------------------------------------------------------------------------
// Copy constructor
//-----------------------------------------------------------------------------
HRPPixelTypeI8Gray8::HRPPixelTypeI8Gray8(const HRPPixelTypeI8Gray8& pi_rObj)
    : HRPPixelTypeI8R8G8B8(pi_rObj)
    {
    }

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
HRPPixelTypeI8Gray8::~HRPPixelTypeI8Gray8()
    {
    // for debugging
    // validate if the palette is changed

    HDEBUGCODE( HRPPixelTypeI8R8G8B8 PixelType; )

    HDEBUGCODE( if (PixelType.GetPalette() != GetPalette()) )
        HDEBUGCODE(     HASSERT(0); )
        }


//-----------------------------------------------------------------------------
// Clone method
//-----------------------------------------------------------------------------
HPMPersistentObject* HRPPixelTypeI8Gray8::Clone() const
    {
    return new HRPPixelTypeI8Gray8(*this);
    }

//-----------------------------------------------------------------------------
// HasConverterFrom
//-----------------------------------------------------------------------------
const HRPPixelConverter* HRPPixelTypeI8Gray8::HasConverterFrom(const HRPPixelType* pi_pPixelTypeFrom) const
    {
    HFCMonitor Monitor(HRPPixelType::s_ConverterAccess);

    // This declaration needs to remain local in order to prevent initialization problems
    static struct I8Gray8ConvertersFrom s_ConvertersFrom;

    MapHRPPixelTypeToConverter::const_iterator Itr;
    Itr = s_ConvertersFrom.find (pi_pPixelTypeFrom->GetClassID());

    return ((Itr == s_ConvertersFrom.end()) ? 0 : (*Itr).second);
    }

//-----------------------------------------------------------------------------
// HasConverterTo
//-----------------------------------------------------------------------------
const HRPPixelConverter* HRPPixelTypeI8Gray8::HasConverterTo(const HRPPixelType* pi_pPixelTypeTo) const
    {
    HFCMonitor Monitor(HRPPixelType::s_ConverterAccess);

    // This declaration needs to remain local in order to prevent initialization problems
    static struct I8Gray8ConvertersTo s_ConvertersTo;

    MapHRPPixelTypeToConverter::const_iterator Itr;
    Itr = s_ConvertersTo.find (pi_pPixelTypeTo->GetClassID());

    return ((Itr == s_ConvertersTo.end()) ? 0 : (*Itr).second);
    }

