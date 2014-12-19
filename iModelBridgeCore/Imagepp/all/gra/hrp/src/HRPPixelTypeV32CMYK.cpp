//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrp/src/HRPPixelTypeV32CMYK.cpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HRPPixelTypeV32CMYK
//-----------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>

#include <Imagepp/all/h/HRPPixelTypeV32CMYK.h>
#include <Imagepp/all/h/HRPChannelOrgCMYK.h>

#include <Imagepp/all/h/HRPPixelTypeV24R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeV32R8G8B8A8.h>
#include <Imagepp/all/h/HFCMath.h>

HPM_REGISTER_CLASS(HRPPixelTypeV32CMYK, HRPPixelType)

// STL typeDef
typedef map<HCLASS_ID, HRPPixelConverter*, less<HCLASS_ID>, allocator<HRPPixelConverter*> >
MapHRPPixelTypeToConverter;


// This is really not accurate, to get correct color conversion, we must use a
// non linear mathematical function.  Maybe pass trough the the XYZ color space..
static const double s_AdjustmentMatrixFromCMYK[3][3] = {0.85, 0.10, 0.05,
                                                        0.20, 0.70, 0.10,
                                                        0.05, 0.15, 0.80
                                                       };

/* Taken from CMYKColorSpace???
double AdjustmentMatrix[3][3] = { 1.206100, -0.16045, -0.055327,
-0.343030,  1.51350, -0.167750,
-0.011065, -0.27376,  1.284900 };*/

static const double s_AdjustmentMatrixToCMYK[3][3] = {1.0, 0.0, 0.0,
                                                      0.0, 1.0, 0.0,
                                                      0.0, 0.0, 1.0
                                                     };

// Functional but the not appropriate for imaging application
// should be use only for "pie chart" color conversion
inline void s_V32CMYK_to_V24RGB(Byte const* pSrc, Byte* pDest)
    {
    Byte Black = (255 - pSrc[3]);
    Byte Red   = (Black * (255 - pSrc[0])) / 255;
    Byte Green = (Black * (255 - pSrc[1])) / 255;
    Byte Blue  = (Black * (255 - pSrc[2])) / 255;

    pDest[0] = (Byte)((s_AdjustmentMatrixFromCMYK[0][0] * Red) + (s_AdjustmentMatrixFromCMYK[0][1] * Green) + (s_AdjustmentMatrixFromCMYK[0][2] * Blue));
    pDest[1] = (Byte)((s_AdjustmentMatrixFromCMYK[1][0] * Red) + (s_AdjustmentMatrixFromCMYK[1][1] * Green) + (s_AdjustmentMatrixFromCMYK[1][2] * Blue));
    pDest[2] = (Byte)((s_AdjustmentMatrixFromCMYK[2][0] * Red) + (s_AdjustmentMatrixFromCMYK[2][1] * Green) + (s_AdjustmentMatrixFromCMYK[2][2] * Blue));
    }

inline void s_V24RGB_to_V32CMYK(Byte const* pSrc, Byte* pDest)
    {
    Byte Red   = 255 - ((Byte)((s_AdjustmentMatrixToCMYK[0][0] * pSrc[0]) + (s_AdjustmentMatrixToCMYK[0][1] * pSrc[1]) + (s_AdjustmentMatrixToCMYK[0][2] * pSrc[2])));
    Byte Green = 255 - ((Byte)((s_AdjustmentMatrixToCMYK[1][0] * pSrc[0]) + (s_AdjustmentMatrixToCMYK[1][1] * pSrc[1]) + (s_AdjustmentMatrixToCMYK[1][2] * pSrc[2])));
    Byte Blue  = 255 - ((Byte)((s_AdjustmentMatrixToCMYK[2][0] * pSrc[0]) + (s_AdjustmentMatrixToCMYK[2][1] * pSrc[1]) + (s_AdjustmentMatrixToCMYK[2][2] * pSrc[2])));

    // Convert CMY to CMYK
    // Extract the black value
    pDest[3] = min(min(Red, Green), Blue);
    pDest[0] = Red - pDest[3];
    pDest[1] = Green - pDest[3]; // (Magenta  * Divider) * 255;
    pDest[2] = Blue - pDest[3]; // (Yellow   * Divider) * 255;
    }

//-----------------------------------------------------------------------------
//  s_V32CMYK_V32CMYK - Converter
//-----------------------------------------------------------------------------
struct ConverterV32CMYK_V32CMYK : public HRPPixelConverter
    {
public:
    DEFINE_T_SUPER(HRPPixelConverter)

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData) const
        {
        memcpy(pio_pDestRawData, pi_pSourceRawData, 4);
        }

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

    HRPPixelConverter* AllocateCopy() const {
        return(new ConverterV32CMYK_V32CMYK(*this));
        }
    };
static struct ConverterV32CMYK_V32CMYK        s_V32CMYK_V32CMYK;

//-----------------------------------------------------------------------------
//  s_V32CMYK_V24B8G8R8 - Converter
//-----------------------------------------------------------------------------
struct ConverterV32CMYK_V24R8G8B8 : public HRPPixelConverter
    {
public:
    DEFINE_T_SUPER(HRPPixelConverter)

    // Functional but the not appropriate for imaging application
    // should be use only for "pie chart" color conversion

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData) const
        {
        s_V32CMYK_to_V24RGB((const Byte*)pi_pSourceRawData, (Byte*)pio_pDestRawData);
        };

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const
        {
        Byte const* pSourceComposite =  (Byte const*)pi_pSourceRawData;
        Byte* pDestComposite = (Byte*)pio_pDestRawData;

        // Copy entire bytes
        while(pi_PixelsCount)
            {
            s_V32CMYK_to_V24RGB(pSourceComposite, pDestComposite);

            --pi_PixelsCount;
            pDestComposite += 3;
            pSourceComposite += 4;
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
        return(new ConverterV32CMYK_V24R8G8B8(*this));
        }
    };
static struct ConverterV32CMYK_V24R8G8B8        s_V32CMYK_V24R8G8B8;

//-----------------------------------------------------------------------------
//  s_V24R8G8B8_V32CMYK - Converter
//-----------------------------------------------------------------------------
struct ConverterV24R8G8B8_V32CMYK : public HRPPixelConverter
    {
public:
    DEFINE_T_SUPER(HRPPixelConverter)

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData) const
        {
        s_V24RGB_to_V32CMYK((const Byte*)pi_pSourceRawData, (Byte*)pio_pDestRawData);
        };

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const
        {
        Byte const* pSourceComposite = (Byte const*)pi_pSourceRawData;
        Byte*       pDestComposite   = (Byte*)pio_pDestRawData;

        // Copy entire bytes
        while(pi_PixelsCount)
            {
            s_V24RGB_to_V32CMYK(pSourceComposite, pDestComposite);

            --pi_PixelsCount;
            pDestComposite += 4;
            pSourceComposite += 3;
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
        return(new ConverterV24R8G8B8_V32CMYK(*this));
        }
    };
static struct ConverterV24R8G8B8_V32CMYK        s_V24R8G8B8_V32CMYK;


//-----------------------------------------------------------------------------
//  s_V32CMYK_V32R8G8B8A8 - Converter
//-----------------------------------------------------------------------------
class ConverterV32CMYK_V32R8G8B8A8 : public HRPPixelConverter
    {
public:
    DEFINE_T_SUPER(HRPPixelConverter)

    // Functionnal but the not appropriate for imaging application
    // should be use only for "pie chart" color conversion

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData) const
        {
        s_V32CMYK_to_V24RGB((const Byte*)pi_pSourceRawData, (Byte*)pio_pDestRawData);
        ((Byte*)pio_pDestRawData)[3] = 0xff;  // opaque
        };

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const
        {
        Byte const* pSourceComposite =  (Byte const*)pi_pSourceRawData;
        Byte* pDestComposite = (Byte*)pio_pDestRawData;

        // Copy entire bytes
        while(pi_PixelsCount)
            {
            s_V32CMYK_to_V24RGB(pSourceComposite, pDestComposite);
            pDestComposite[3] = 0xff; // opaque

            --pi_PixelsCount;
            pDestComposite += 4;
            pSourceComposite += 4;
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
        return(new ConverterV32CMYK_V32R8G8B8A8(*this));
        }
    };
static ConverterV32CMYK_V32R8G8B8A8        s_V32CMYK_V32R8G8B8A8;


//-----------------------------------------------------------------------------
//  s_V32R8G8B8A8_V32CMYK - Converter
//-----------------------------------------------------------------------------
class ConverterV32R8G8B8A8_V32CMYK : public HRPPixelConverter
    {
public:
    DEFINE_T_SUPER(HRPPixelConverter)

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData) const
        {
        // Don't care about alpha
        s_V24RGB_to_V32CMYK((const Byte*)pi_pSourceRawData, (Byte*)pio_pDestRawData);
        };

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const
        {
        Byte const* pSourceComposite = (Byte const*)pi_pSourceRawData;
        Byte* pDestComposite   = (Byte*)pio_pDestRawData;

        while(pi_PixelsCount)
            {
            // Don't care about alpha
            s_V24RGB_to_V32CMYK(pSourceComposite, pDestComposite);

            --pi_PixelsCount;
            pDestComposite += 4;
            pSourceComposite += 4;
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
        Byte* pSourceComposite = (Byte*)pi_pSourceRawData;
        Byte* pDestComposite = (Byte*)pio_pDestRawData;

        // If source pixel is fully transparent, destination is unaltered
        if (pSourceComposite[3] != 0)
            {
            if (pSourceComposite[3] == 255)
                {
                // Destination pixel is fully transparent, or source pixel is fully opaque. Copy source pixel.
                s_V24RGB_to_V32CMYK(pSourceComposite, pDestComposite);
                }
            else
                {
                HFCMath (*pQuotients) (HFCMath::GetInstance());

                // Convert to RGB for compose.
                Byte RGB[3];
                s_V32CMYK_to_V24RGB(pDestComposite, RGB);

                // alpha * (S - D) + D
                RGB[0] = pQuotients->DivideBy255ToByte(pSourceComposite[3] * (pSourceComposite[0] - RGB[0])) + RGB[0];
                RGB[1] = pQuotients->DivideBy255ToByte(pSourceComposite[3] * (pSourceComposite[1] - RGB[1])) + RGB[1];
                RGB[2] = pQuotients->DivideBy255ToByte(pSourceComposite[3] * (pSourceComposite[2] - RGB[2])) + RGB[2];

                // Convert back to CMYK
                s_V24RGB_to_V32CMYK(RGB, pDestComposite);
                }
            }
        };

    virtual void Compose(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const
        {
        Byte* pSourceComposite = (Byte*)pi_pSourceRawData;
        Byte* pDestComposite   = (Byte*)pio_pDestRawData;

        HFCMath (*pQuotients) (HFCMath::GetInstance());

        // Copy entire bytes
        while(pi_PixelsCount)
            {
            // If source pixel is fully transparent, destination is unaltered
            if (pSourceComposite[3] != 0)
                {
                if (pSourceComposite[3] == 255)
                    {
                    // Destination pixel is fully transparent, or source pixel is fully opaque. Copy source pixel.
                    s_V24RGB_to_V32CMYK(pSourceComposite, pDestComposite);
                    }
                else
                    {
                    // Convert to RGB for compose.
                    Byte RGB[3];
                    s_V32CMYK_to_V24RGB(pDestComposite, RGB);

                    // alpha * (S - D) + D
                    RGB[0] = pQuotients->DivideBy255ToByte(pSourceComposite[3] * (pSourceComposite[0] - RGB[0])) + RGB[0];
                    RGB[1] = pQuotients->DivideBy255ToByte(pSourceComposite[3] * (pSourceComposite[1] - RGB[1])) + RGB[1];
                    RGB[2] = pQuotients->DivideBy255ToByte(pSourceComposite[3] * (pSourceComposite[2] - RGB[2])) + RGB[2];

                    // Convert back to CMYK
                    s_V24RGB_to_V32CMYK(RGB, pDestComposite);
                    }
                }

            --pi_PixelsCount;
            pDestComposite += 4;
            pSourceComposite += 4;
            }
        };

    virtual const short* GetLostChannels() const
        {
        return m_LostChannels;
        }

    HRPPixelConverter* AllocateCopy() const {
        return(new ConverterV32R8G8B8A8_V32CMYK(*this));
        }

private:

    static short m_LostChannels[];
    };
short ConverterV32R8G8B8A8_V32CMYK::m_LostChannels[] = {3, -1};
static ConverterV32R8G8B8A8_V32CMYK        s_V32R8G8B8A8_V32CMYK;


//-----------------------------------------------------------------------------
//  Dictionnary of converters from other pixel types
//-----------------------------------------------------------------------------
struct V32CMYKConvertersFrom : public MapHRPPixelTypeToConverter
    {
    V32CMYKConvertersFrom() : MapHRPPixelTypeToConverter ()
        {
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV32CMYK::CLASS_ID, &s_V32CMYK_V32CMYK));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV24R8G8B8::CLASS_ID, &s_V24R8G8B8_V32CMYK));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV32R8G8B8A8::CLASS_ID, &s_V32R8G8B8A8_V32CMYK));
        };
    };

//-----------------------------------------------------------------------------
//  Dictionnary of converters from other pixel types
//-----------------------------------------------------------------------------
struct V32CMYKConvertersTo : public MapHRPPixelTypeToConverter
    {
    V32CMYKConvertersTo() : MapHRPPixelTypeToConverter ()
        {
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV32CMYK::CLASS_ID, &s_V32CMYK_V32CMYK));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV24R8G8B8::CLASS_ID, &s_V32CMYK_V24R8G8B8));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV24R8G8B8::CLASS_ID, &s_V32CMYK_V32R8G8B8A8));
        };
    };

//-----------------------------------------------------------------------------
// Constructor for true color 24 bits RGB
//-----------------------------------------------------------------------------
HRPPixelTypeV32CMYK::HRPPixelTypeV32CMYK()
    : HRPPixelType(HRPChannelOrgCMYK(), 0)
    {
    }


//-----------------------------------------------------------------------------
// Copy constructor
//-----------------------------------------------------------------------------
HRPPixelTypeV32CMYK::HRPPixelTypeV32CMYK(const HRPPixelTypeV32CMYK& pi_rObj)
    : HRPPixelType(pi_rObj)
    {
    }

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
HRPPixelTypeV32CMYK::~HRPPixelTypeV32CMYK()
    {
    }

//-----------------------------------------------------------------------------
// Clone method
//-----------------------------------------------------------------------------
HPMPersistentObject* HRPPixelTypeV32CMYK::Clone() const
    {
    return new HRPPixelTypeV32CMYK(*this);
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
unsigned short HRPPixelTypeV32CMYK::CountValueBits() const
    {
    return 32;
    }


//-----------------------------------------------------------------------------
// HasConverterFrom
//-----------------------------------------------------------------------------
const HRPPixelConverter* HRPPixelTypeV32CMYK::HasConverterFrom(const HRPPixelType* pi_pPixelTypeFrom) const
    {
    HFCMonitor Monitor(HRPPixelType::s_ConverterAccess);

    // This declaration needs to remain local in order to prevent initialization problems
    static struct V32CMYKConvertersFrom s_ConvertersFrom;

    MapHRPPixelTypeToConverter::const_iterator Itr;
    Itr = s_ConvertersFrom.find (pi_pPixelTypeFrom->GetClassID());

    return ((Itr == s_ConvertersFrom.end()) ? 0 : (*Itr).second);
    }

//-----------------------------------------------------------------------------
// HasConverterTo
//-----------------------------------------------------------------------------
const HRPPixelConverter* HRPPixelTypeV32CMYK::HasConverterTo(const HRPPixelType* pi_pPixelTypeTo) const
    {
    HFCMonitor Monitor(HRPPixelType::s_ConverterAccess);

    // This declaration needs to remain local in order to prevent initialization problems
    static struct V32CMYKConvertersTo s_ConvertersTo;

    MapHRPPixelTypeToConverter::const_iterator Itr;
    Itr = s_ConvertersTo.find (pi_pPixelTypeTo->GetClassID());

    return ((Itr == s_ConvertersTo.end()) ? 0 : (*Itr).second);
    }

