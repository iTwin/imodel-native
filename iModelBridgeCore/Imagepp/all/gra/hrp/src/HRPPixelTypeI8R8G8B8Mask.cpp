//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrp/src/HRPPixelTypeI8R8G8B8Mask.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for class HRPPixelTypeI8R8G8B8
//-----------------------------------------------------------------------------
#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HRPPixelTypeI8R8G8B8Mask.h>
#include <Imagepp/all/h/HRPPixelTypeV24R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeV32R8G8B8A8.h>

HPM_REGISTER_CLASS(HRPPixelTypeI8R8G8B8Mask, HRPPixelTypeI8R8G8B8)

// STL typeDef
typedef map<HCLASS_ID, HRPPixelConverter*, less<HCLASS_ID>, allocator<HRPPixelConverter*> >
MapHRPPixelTypeToConverter;

/*----------------------------------------------------------------------------+
|    Converter Definition
+----------------------------------------------------------------------------*/
//-----------------------------------------------------------------------------
//  s_I8R8G8B8_I8R8G8B8Mask - Converter From I8R8G8B8 to I8R8G8B8Mask
//-----------------------------------------------------------------------------
class ConverterI8R8G8B8_I8R8G8B8Mask : public HRPPixelConverter
    {
public:
    DEFINE_T_SUPER(HRPPixelConverter)

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        HASSERT(false); // should not happen

        unsigned char* pSource = (unsigned char*)pi_pSourceRawData;
        unsigned char* pDestination = (unsigned char*)pio_pDestRawData;

        for(size_t i(0); i < pi_PixelsCount; ++i)
            {
            pDestination[i] = pSource[i] ? 1 : 0;
            }
        };

    virtual HRPPixelConverter* AllocateCopy() const
        {
        return(new ConverterI8R8G8B8_I8R8G8B8Mask(*this));
        };
    };

static ConverterI8R8G8B8_I8R8G8B8Mask s_I8R8G8B8_I8R8G8B8Mask;

//-----------------------------------------------------------------------------
//  s_V24R8G8B8_I8R8G8B8Mask - Converter From V24R8G8B8 to I8R8G8B8Mask
//-----------------------------------------------------------------------------
class ConverterV24R8G8B8_I8R8G8B8Mask : public HRPPixelConverter
    {
public:
    DEFINE_T_SUPER(HRPPixelConverter)

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        unsigned char* pSource = (unsigned char*)pi_pSourceRawData;
        unsigned char* pDest   = (unsigned char*)pio_pDestRawData;

        for(size_t i(0); i < pi_PixelsCount; ++i)
            {
            // this special pixel type know only two RGB value.
            HASSERT((pSource[i*3] == 0 && pSource[i*3+1] == 0 && pSource[i*3+2] == 0) || pSource[i*3+0] == 255 && pSource[i*3+1] == 255 && pSource[i*3+2] == 255);

            pDest[i] = pSource[i*3] ? 1 : 0;
            }
        };

    virtual HRPPixelConverter* AllocateCopy() const override
        {
        return(new ConverterV24R8G8B8_I8R8G8B8Mask(*this));
        };
    };

static ConverterV24R8G8B8_I8R8G8B8Mask s_V24R8G8B8_I8R8G8B8Mask;

//-----------------------------------------------------------------------------
//  s_V32R8G8B8A8_I8R8G8B8Mask - Converter From V32R8G8B8A8 to I8R8G8B8Mask
//-----------------------------------------------------------------------------
class ConverterV32R8G8B8A8_I8R8G8B8Mask : public HRPPixelConverter
    {
public:
    DEFINE_T_SUPER(HRPPixelConverter)

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        unsigned char* pSource = (unsigned char*)pi_pSourceRawData;
        unsigned char* pDest   = (unsigned char*)pio_pDestRawData;

        for(size_t i(0); i < pi_PixelsCount; ++i)
            {
            // this special pixel type know only 3 RGBA value.
            HASSERT((pSource[i*4] == 0 && pSource[i*4+1] == 0 && pSource[i*4+2] == 0 && pSource[i*4+3] == 0) ||
                   (pSource[i*4] == 0 && pSource[i*4+1] == 0 && pSource[i*4+2] == 0 && pSource[i*4+3] == 255) ||
                   (pSource[i*4] == 255 && pSource[i*4+1] == 255 && pSource[i*4+2] == 255 && pSource[i*4+3] == 255));

            pDest[i] = pSource[i*4] ? 1 : 0;
            }
        };

    virtual void Compose(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        unsigned char* pSource = (unsigned char*)pi_pSourceRawData;
        unsigned char* pDest   = (unsigned char*)pio_pDestRawData;

        for(size_t i(0); i < pi_PixelsCount; ++i)
            {
            // this special pixel type know only 3 RGBA value.
            HASSERT((pSource[i*4] == 0 && pSource[i*4+1] == 0 && pSource[i*4+2] == 0 && pSource[i*4+3] == 0) ||
                   (pSource[i*4] == 0 && pSource[i*4+1] == 0 && pSource[i*4+2] == 0 && pSource[i*4+3] == 255) ||
                   (pSource[i*4] == 255 && pSource[i*4+1] == 255 && pSource[i*4+2] == 255 && pSource[i*4+3] == 255));

            if(pSource[i*4+3] == 255)  // If source is opaque alter destination pixel
                pDest[i] = pSource[i*4] ? 1 : 0;
            }
        };

    virtual HRPPixelConverter* AllocateCopy() const override
        {
        return(new ConverterV32R8G8B8A8_I8R8G8B8Mask(*this));
        };
    };

static ConverterV32R8G8B8A8_I8R8G8B8Mask s_V32R8G8B8A8_I8R8G8B8Mask;

//-----------------------------------------------------------------------------
//  s_I8R8G8B8Mask_V32R8G8B8A8 - Converter From I8R8G8B8Mask to V32R8G8B8A8
//-----------------------------------------------------------------------------
class ConverterI8R8G8B8Mask_V32R8G8B8A8 : public HRPPixelConverter
    {
public:
    DEFINE_T_SUPER(HRPPixelConverter)

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        unsigned char* pSource = (unsigned char*)pi_pSourceRawData;
        long*    pDest   = (long*)pio_pDestRawData;

        for(size_t i(0); i < pi_PixelsCount; ++i)
            {
            if(pSource[i])
                {
                pDest[i] = 0xFFFFFFFF;  // {255,255,255,255}
                }
            else
                {
                pDest[i] = 0x00000000;  // {0,0,0,0}
                }
            }
        };

    virtual HRPPixelConverter* AllocateCopy() const override
        {
        return(new ConverterI8R8G8B8Mask_V32R8G8B8A8(*this));
        };
    };

static ConverterI8R8G8B8Mask_V32R8G8B8A8 s_I8R8G8B8Mask_V32R8G8B8A8;

//-----------------------------------------------------------------------------
//  s_I8R8G8B8Mask_V24R8G8B8 - Converter From I8R8G8B8Mask to V24R8G8B8
//-----------------------------------------------------------------------------
class ConverterI8R8G8B8Mask_V24R8G8B8 : public HRPPixelConverter
    {
public:
    DEFINE_T_SUPER(HRPPixelConverter)

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        unsigned char* pSource = (unsigned char*)pi_pSourceRawData;
        unsigned char*   pDest   = (unsigned char*)pio_pDestRawData;

        for(size_t i(0); i < pi_PixelsCount; ++i)
            {
            if(pSource[i])
                {
                pDest[i] = 255;
                pDest[i+1] = 255;
                pDest[i+2] = 255;
                }
            else
                {
                pDest[i] = 0;
                pDest[i+1] = 0;
                pDest[i+2] = 0;
                }
            }
        };

    virtual HRPPixelConverter* AllocateCopy() const override
        {
        return(new ConverterI8R8G8B8Mask_V24R8G8B8(*this));
        };
    };

static ConverterI8R8G8B8Mask_V24R8G8B8 s_I8R8G8B8Mask_V24R8G8B8;

//-----------------------------------------------------------------------------
//  Dictionnary of converters from other pixel types
//-----------------------------------------------------------------------------
struct I8R8G8B8MaskConvertersFrom : public MapHRPPixelTypeToConverter
    {
    I8R8G8B8MaskConvertersFrom() : MapHRPPixelTypeToConverter ()
        {
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV24R8G8B8::CLASS_ID, &s_V24R8G8B8_I8R8G8B8Mask));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV32R8G8B8A8::CLASS_ID, &s_V32R8G8B8A8_I8R8G8B8Mask));
        };
    };

//-----------------------------------------------------------------------------
//  Dictionnary of converters from other pixel types
//-----------------------------------------------------------------------------
struct I8R8G8B8MaskConvertersTo : public MapHRPPixelTypeToConverter
    {
    I8R8G8B8MaskConvertersTo() : MapHRPPixelTypeToConverter ()
        {
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV32R8G8B8A8::CLASS_ID, &s_I8R8G8B8Mask_V32R8G8B8A8));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV24R8G8B8::CLASS_ID, &s_I8R8G8B8Mask_V24R8G8B8));
        };
    };


//-----------------------------------------------------------------------------
// Default Constructor
//-----------------------------------------------------------------------------
HRPPixelTypeI8R8G8B8Mask::HRPPixelTypeI8R8G8B8Mask()
    :HRPPixelTypeI8R8G8B8()
    {
    }

//-----------------------------------------------------------------------------
// Constructor for Index 8 bits with palette true color 24 bits RGB
// with a palette
//-----------------------------------------------------------------------------
HRPPixelTypeI8R8G8B8Mask::HRPPixelTypeI8R8G8B8Mask(const HRPPixelPalette& pi_Palette)
    :HRPPixelTypeI8R8G8B8(pi_Palette)
    {
    }

//-----------------------------------------------------------------------------
// Copy constructor
//-----------------------------------------------------------------------------
HRPPixelTypeI8R8G8B8Mask::HRPPixelTypeI8R8G8B8Mask(const HRPPixelTypeI8R8G8B8Mask& pi_rObj)
    :HRPPixelTypeI8R8G8B8(pi_rObj)
    {
    }

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
HRPPixelTypeI8R8G8B8Mask::~HRPPixelTypeI8R8G8B8Mask()
    {
    }

//-----------------------------------------------------------------------------
// Clone method
//-----------------------------------------------------------------------------
HPMPersistentObject* HRPPixelTypeI8R8G8B8Mask::Clone() const
    {
    return new HRPPixelTypeI8R8G8B8Mask(*this);
    }

//-----------------------------------------------------------------------------
// HasConverterFrom
//-----------------------------------------------------------------------------
const HRPPixelConverter* HRPPixelTypeI8R8G8B8Mask::HasConverterFrom(const HRPPixelType* pi_pPixelTypeFrom) const
    {
    HFCMonitor Monitor(HRPPixelType::s_ConverterAccess);

    // This declaration needs to remain local in order to prevent initialization problems
    static struct I8R8G8B8MaskConvertersFrom s_ConvertersFrom;

    MapHRPPixelTypeToConverter::const_iterator Itr;
    Itr = s_ConvertersFrom.find (pi_pPixelTypeFrom->GetClassID());

    return ((Itr == s_ConvertersFrom.end()) ? 0 : (*Itr).second);
    }

//-----------------------------------------------------------------------------
// HasConverterTo
//-----------------------------------------------------------------------------
const HRPPixelConverter* HRPPixelTypeI8R8G8B8Mask::HasConverterTo(const HRPPixelType* pi_pPixelTypeTo) const
    {
    HFCMonitor Monitor(HRPPixelType::s_ConverterAccess);

    // no ConverterTo should be involve with this pixel because it it is used only as a destination surface.
    // A convert to 32 bits was implemented to avoid infinite loop while looking to a complex converter.
    //HASSERT(false);
    // When doing a copy from of a mosaic into a bitmap of type mask a conversion from mask to v32 occurs.
    // The destination bitmap is converted to v32 to be compatible with the mosaic, the mosaic is drawn and the
    // bitmap is then reconverted to mask.  This will cause no harm in this case becuase the destination bitmap
    // is always fill with 0.

    // This declaration needs to remain local in order to prevent initialization problems
    static struct I8R8G8B8MaskConvertersTo s_ConvertersTo;

    MapHRPPixelTypeToConverter::const_iterator Itr;
    Itr = s_ConvertersTo.find (pi_pPixelTypeTo->GetClassID());

    return ((Itr == s_ConvertersTo.end()) ? 0 : (*Itr).second);
    }