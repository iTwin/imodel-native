//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrp/src/HRPPixelTypeV32PRPhotoYCCA8.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HRPPixelTypeV32PRPhotoYCCA8.cpp
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HRPPixelTypeV32PRPhotoYCCA8.h>

#include <Imagepp/all/h/HRPPaletteOctreeR8G8B8.h>
#include <Imagepp/all/h/HRPChannelOrgPRPhotoYCCA8.h>

#include <Imagepp/all/h/HRPPixelTypeI8R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeV24B8G8R8.h>
#include <Imagepp/all/h/HRPPixelTypeV24R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeV32R8G8B8A8.h>
#include <Imagepp/all/h/HRPPixelTypeV24PhotoYCC.h>
#include "v24rgb.h" // for YCC conversion factor

#include <Imagepp/all/h/HFCMath.h>

HPM_REGISTER_CLASS(HRPPixelTypeV32PRPhotoYCCA8, HRPPixelType)

// STL typeDef
typedef map<HCLASS_ID, HRPPixelConverter*, less<HCLASS_ID>, allocator<HRPPixelConverter*> >
MapHRPPixelTypeToConverter;

/* Important info: newYCC
 *  Refer to comments in HRPPixelTypeV24PhotoYcc.cpp regarding
 *      coefficients used and alpha composition formula.
 */

#if oldYCC
// INFORMATION:
//
// Matrix to convert PrYCCA -> RGBA (From FPXView sample app.)
//
// Y        C1       C2       A / 255
// 1.3586   0     1.82187   -249.571
// 1.3586 -0.43   -0.927    194.44
// 1.3586 2.218      0      -345.94
//
// Then pass through the s_LookUpTable for [0,255] range RGB.

#define CLAMPTABLE(A)    ((A)<=(0) ? (0) : (A)<(343) ? (A) : (342))
#define CLAMPINVTABLE(A) (Byte)((A)<=(0) ? (0) : (A)<(256) ? (A) : (255))
// From Flashpix standards document, to convert RGB in [0, 361] to [0, 255]
static const Byte s_LookUpTable[] =
    {
    0,1,1,2,2,3,4,5,6,7,
    8,9,10,11,12,13,14,15,16,17,
    18,19,20,22,23,24,25,26,28,29,
    30,31,33,34,35,36,38,39,40,41,
    43,44,45,47,48,49,51,52,53,55,
    56,57,59,60,61,63,64,65,67,68,
    70,71,72,74,75,76,78,79,81,82,
    83,85,86,88,89,91,92,93,95,96,
    98,99,101,102,103,105,106,108,109,111,
    112,113,115,116,118,119,121,122,123,125,
    126,128,129,130,132,133,134,136,137,138,
    140,141,142,144,145,146,148,149,150,152,
    153,154,155,157,158,159,160,162,163,164,
    165,166,168,169,170,171,172,174,175,176,
    177,178,179,180,182,183,184,185,186,187,
    188,189,190,191,192,194,195,196,197,198,
    199,200,201,202,203,204,204,205,206,207,
    208,209,210,211,212,213,213,214,215,216,
    217,217,218,219,220,221,221,222,223,223,
    224,225,225,226,227,227,228,229,229,230,
    230,231,231,232,233,233,234,234,235,235,
    236,236,236,237,237,238,238,238,239,239,
    240,240,240,241,241,241,242,242,242,242,
    243,243,243,244,244,244,244,245,245,245,
    245,245,246,246,246,246,246,247,247,247,
    247,247,247,248,248,248,248,248,248,249,
    249,249,249,249,249,249,249,249,250,250,
    250,250,250,250,250,250,250,250,251,251,
    251,251,251,251,251,251,251,251,251,251,
    251,251,252,252,252,252,252,252,252,252,
    252,252,252,252,252,252,252,252,252,253,
    253,253,253,253,253,253,253,253,253,253,
    253,253,253,253,253,253,253,254,254,254,
    254,254,254,254,254,254,254,254,254,254,
    254,254,255           // All others up to 361 are = 255
    };


/* Gamma to apply to RGB values as first step when converting
   to YCC.
   From "Colour space conversions" by Adrian Ford & Alan Roberts (August 11, 1998)

   For R,G,B in [0, 1]:
   if (Val < 0.018)
        NewVal = Val * 4.5
   else
        NewVal = 1.099 * pow(Val, 0.45) - 0.099

   Values have been rounded.
*/
static const unsigned short s_RGBGammaTable[] =
    {
    0,5,9,14,18,23,27,30,34,37,
    40,43,46,48,51,53,55,58,60,62,
    64,66,68,70,72,73,75,77,78,80,
    82,83,85,86,88,89,91,92,94,95,
    97,98,99,101,102,103,104,106,107,108,
    109,111,112,113,114,115,116,118,119,120,
    121,122,123,124,125,126,127,128,129,130,
    131,132,133,134,135,136,137,138,139,140,
    141,142,143,144,145,146,147,147,148,149,
    150,151,152,153,154,154,155,156,157,158,
    159,159,160,161,162,163,164,164,165,166,
    167,168,168,169,170,171,171,172,173,174,
    174,175,176,177,177,178,179,180,180,181,
    182,182,183,184,185,185,186,187,187,188,
    189,189,190,191,191,192,193,193,194,195,
    195,196,197,197,198,199,199,200,201,201,
    202,203,203,204,205,205,206,206,207,208,
    208,209,209,210,211,211,212,213,213,214,
    214,215,216,216,217,217,218,218,219,220,
    220,221,221,222,223,223,224,224,225,225,
    226,227,227,228,228,229,229,230,230,231,
    232,232,233,233,234,234,235,235,236,236,
    237,238,238,239,239,240,240,241,241,242,
    242,243,243,244,244,245,245,246,246,247,
    247,248,248,249,249,250,251,251,252,252,
    253,253,254,254,255,255
    };
   
#endif


//-----------------------------------------------------------------------------
//  s_V32PRPhotoYCCA8_V32PRPhotoYCCA8 - Converter
//-----------------------------------------------------------------------------
struct ConverterV32PRPhotoYCCA8_V32PRPhotoYCCA8 : public HRPPixelConverter
    {
public:
    DEFINE_T_SUPER(HRPPixelConverter)

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        memcpy(pio_pDestRawData, pi_pSourceRawData, 4 * pi_PixelsCount);
        }

    virtual void ConvertLostChannel(   const void* pi_pSourceRawData,
                            void* pio_pDestRawData,
                            size_t pi_PixelsCount,
                            const bool* pi_pChannelsMask) const override
        {
        Byte* const pSourceComposite =  (Byte*)pi_pSourceRawData;
        Byte* pDestComposite = (Byte*)pio_pDestRawData;

        for(uint32_t pix = 0; pix < pi_PixelsCount; ++pix)
            {
            if(pi_pChannelsMask[3])
                {
                pDestComposite[4 * pix    ] = pDestComposite[4 * pix    ] * pSourceComposite[4 * pix + 3] / 255;
                pDestComposite[4 * pix + 1] = pDestComposite[4 * pix + 1] * pSourceComposite[4 * pix + 3] / 255;
                pDestComposite[4 * pix + 2] = pDestComposite[4 * pix + 2] * pSourceComposite[4 * pix + 3] / 255;
                pDestComposite[4 * pix + 3] = pSourceComposite[4 * pix + 3];
                }
            }
        };

    virtual void Compose(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        Byte* const pSourceComposite =  (Byte*)pi_pSourceRawData;
        Byte* pDestComposite = (Byte*)pio_pDestRawData;

        for(uint32_t pix = 0; pix < pi_PixelsCount ; ++ pix)
            {
            // If transparent, do nothing
            if (0 != pSourceComposite[4 * pix + 3])
                {
                if (0 == pDestComposite[4 * pix + 3] || 255 == pSourceComposite[4 * pix + 3])
                    {
                    // Destination pixel is fully transparent, or source pixel is fully opaque. Copy source pixel.
                    *((uint32_t*)pDestComposite) = *((uint32_t*)pSourceComposite);
                    }
                else
                    {        
                    pDestComposite[4 * pix    ] = Blend_PRsrc_PRdst(pSourceComposite[4 * pix    ], pDestComposite[4 * pix    ], pSourceComposite[4 * pix + 3]);
                    pDestComposite[4 * pix + 1] = Blend_PRsrc_PRdst(pSourceComposite[4 * pix + 1], pDestComposite[4 * pix + 1], pSourceComposite[4 * pix + 3]);
                    pDestComposite[4 * pix + 2] = Blend_PRsrc_PRdst(pSourceComposite[4 * pix + 2], pDestComposite[4 * pix + 2], pSourceComposite[4 * pix + 3]);
                    
                    pDestComposite[4 * pix + 3] = Blend_Alpha_Channel(pSourceComposite[4 * pix + 3], pDestComposite[4 * pix + 3]);
                    }
                }
            }
        }

    HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterV32PRPhotoYCCA8_V32PRPhotoYCCA8(*this));
        }

    };
static struct ConverterV32PRPhotoYCCA8_V32PRPhotoYCCA8        s_V32PRPhotoYCCA8_V32PRPhotoYCCA8;

//-----------------------------------------------------------------------------
//  s_V32PRPhotoYCCA8_V24PhotoYCC - Converter
//-----------------------------------------------------------------------------
struct ConverterV32PRPhotoYCCA8_V24PhotoYCC : public HRPPixelConverter
    {
public:
    DEFINE_T_SUPER(HRPPixelConverter)

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        Byte* const pSourceComposite =  (Byte*)pi_pSourceRawData;
        Byte* pDestComposite = (Byte*)pio_pDestRawData;

#if oldYCC
        while(pi_PixelsCount)
            {
            if(pSourceComposite[3] != 0)
                {
                pDestComposite[0] = CLAMPINVTABLE(pSourceComposite[0] * 255 / pSourceComposite[3]);
                pDestComposite[1] = CLAMPINVTABLE(pSourceComposite[1] * 255 / pSourceComposite[3]);
                pDestComposite[2] = CLAMPINVTABLE(pSourceComposite[2] * 255 / pSourceComposite[3]);
                }
            else
                {
                pDestComposite[0] = 0;
                pDestComposite[1] = 0;
                pDestComposite[2] = 0;
                }

            pi_PixelsCount -= 1;
            pDestComposite+=3;
            pSourceComposite+=4;
            }
#else
        for(uint32_t pix = 0; pix < pi_PixelsCount; ++pix)
            {
            if(0 != pSourceComposite[4 * pix + 3])
                {
                // Demultiply by alpha
                pDestComposite[3 * pix    ] = pSourceComposite[4 * pix    ] * 255 / pSourceComposite[4 * pix + 3];
                pDestComposite[3 * pix + 1] = pSourceComposite[4 * pix + 1] * 255 / pSourceComposite[4 * pix + 3];
                pDestComposite[3 * pix + 2] = pSourceComposite[4 * pix + 2] * 255 / pSourceComposite[4 * pix + 3];
                }
            else
                {
                // Copy pixel as-is
                pDestComposite[3  * pix    ] = pSourceComposite[4 * pix    ];
                pDestComposite[3  * pix + 1] = pSourceComposite[4 * pix + 1];
                pDestComposite[3  * pix + 2] = pSourceComposite[4 * pix + 2];
                }
            }
#endif
        };

    virtual void Compose(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        Byte* const pSourceComposite =  (Byte*)pi_pSourceRawData;
        Byte* pDestComposite = (Byte*)pio_pDestRawData;

        for(uint32_t pix = 0; pix < pi_PixelsCount; ++pix)
            {
            // If transparent, do nothing
            if (0 != pSourceComposite[4 * pix + 3])
                {
                if (255 == pSourceComposite[4 * pix + 3])
                    {
                    // Source pixel is fully opaque. Copy source pixel,
                    memcpy(&pDestComposite[3 * pix], &pSourceComposite[4 * pix], 3);
                    }
                else
                    {
                    pDestComposite[3 * pix    ] = Blend_PRsrc_OPAQUEdst(pSourceComposite[4 * pix    ], pDestComposite[3 * pix    ], pSourceComposite[ 4 * pix + 3]);
                    pDestComposite[3 * pix + 1] = Blend_PRsrc_OPAQUEdst(pSourceComposite[4 * pix + 1], pDestComposite[3 * pix + 1], pSourceComposite[ 4 * pix + 3]);
                    pDestComposite[3 * pix + 2] = Blend_PRsrc_OPAQUEdst(pSourceComposite[4 * pix + 2], pDestComposite[3 * pix + 2], pSourceComposite[ 4 * pix + 3]);
                    }
                }
            }
        };

    virtual const short* GetLostChannels() const override
        {
        return m_LostChannels;
        }

    HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterV32PRPhotoYCCA8_V24PhotoYCC(*this));
        }


private:

    static short m_LostChannels[];

    };
short ConverterV32PRPhotoYCCA8_V24PhotoYCC::m_LostChannels[] = {3, -1};
static struct ConverterV32PRPhotoYCCA8_V24PhotoYCC        s_V32PRPhotoYCCA8_V24PhotoYCC;

//-----------------------------------------------------------------------------
//  s_V24PhotoYCC_V32PRPhotoYCCA8 - Converter
//-----------------------------------------------------------------------------
struct ConverterV24PhotoYCC_V32PRPhotoYCCA8 : public HRPPixelConverter
    {
public:
    DEFINE_T_SUPER(HRPPixelConverter)

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {

        Byte* const pSourceComposite =  (Byte*)pi_pSourceRawData;
        Byte* pDestComposite = (Byte*)pio_pDestRawData;

#if oldYCC
        while(pi_PixelsCount)
            {
            memcpy(pDestComposite, pSourceComposite, 3);

            pDestComposite[3] = 0xff; // opaque

            pi_PixelsCount -= 1;
            pDestComposite+=4;
            pSourceComposite+=3;
            }
#else
        for(uint32_t pix = 0; pix < pi_PixelsCount; ++pix)
            {
            memcpy(&pDestComposite[4 * pix], &pSourceComposite[3 * pix], 3);
            pDestComposite[4 * pix + 3] = 255; // opaque
            }
#endif
        };

    HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterV24PhotoYCC_V32PRPhotoYCCA8(*this));
        }

    };
static struct ConverterV24PhotoYCC_V32PRPhotoYCCA8        s_V24PhotoYCC_V32PRPhotoYCCA8;

//-----------------------------------------------------------------------------
//  s_V32PRPhotoYCCA8_V24R8G8B8 - Converter
// Templated version that works for both V24R8G8B8 and V24B8G8R8 as output
// R, G, B values represent their relative index within a pixel
//-----------------------------------------------------------------------------
template<uint32_t R, uint32_t G, uint32_t B>
struct ConverterV32PRPhotoYCCA8_V24R8G8B8 : public HRPPixelConverter
    {
public:
    DEFINE_T_SUPER(HRPPixelConverter)

    virtual void Compose(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        
        Byte* const pSourceComposite =  (Byte*)pi_pSourceRawData;
        Byte* pDestComposite = (Byte*)pio_pDestRawData;

#if oldYCC
        short R;
        short G;
        short B;

        float PreMultY;
        float AlphaPercent;

        while(pi_PixelsCount)
            {
            PreMultY = (float)(1.3586 * pSourceComposite[0]);
            AlphaPercent = ((float) pSourceComposite[3]) / (float)255.0;

            R = (short)(PreMultY + (1.82187 * pSourceComposite[2]) + (-249.571 * AlphaPercent));
            G = (short)(PreMultY + (-0.43 * pSourceComposite[1]) + (-0.927 * pSourceComposite[2]) + (194.44 * AlphaPercent));
            B = (short)(PreMultY + (2.218 * pSourceComposite[1]) + (-345.94 * AlphaPercent));

            // (S * alpha) + (D * (1 - alpha))
            pDestComposite[0] = ((s_LookUpTable[CLAMPTABLE(R)] * pSourceComposite[3]) +
                                 (pDestComposite[0] * (255 - pSourceComposite[3]))) / 255;
            pDestComposite[1] = ((s_LookUpTable[CLAMPTABLE(G)] * pSourceComposite[3]) +
                                 (pDestComposite[1] * (255 - pSourceComposite[3]))) / 255;
            pDestComposite[2] = ((s_LookUpTable[CLAMPTABLE(B)] * pSourceComposite[3]) +
                                 (pDestComposite[2] * (255 - pSourceComposite[3]))) / 255;

            pi_PixelsCount -= 1;
            pDestComposite += 3;
            pSourceComposite += 4;
            }
#else
        for(uint32_t pix = 0; pix < pi_PixelsCount; ++pix)
            {  
            // If transparent, do nothing
            if(0 != pSourceComposite[4 * pix + 3])
                {
                if(255 == pSourceComposite[4 * pix + 3])
                    {
                    // Only conversion is required. Overwrite dest.
                    pDestComposite[3 * pix + R] = static_cast<Byte>(Convert_YCC_to_RGB_Red  (pSourceComposite[4 * pix], pSourceComposite[4 * pix + 2]));
                    pDestComposite[3 * pix + G] = static_cast<Byte>(Convert_YCC_to_RGB_Green(pSourceComposite[4 * pix], pSourceComposite[4 * pix + 1], pSourceComposite[4 * pix + 2]));
                    pDestComposite[3 * pix + B] = static_cast<Byte>(Convert_YCC_to_RGB_Blue (pSourceComposite[4 * pix], pSourceComposite[4 * pix + 1]));
                    }
                else
                    {
                    // Demultiply
                    const uint32_t Y  = (pSourceComposite[4 * pix    ] * 255) / pSourceComposite[4 * pix + 3];
                    const uint32_t Cb = (pSourceComposite[4 * pix + 1] * 255) / pSourceComposite[4 * pix + 3];
                    const uint32_t Cr = (pSourceComposite[4 * pix + 2] * 255) / pSourceComposite[4 * pix + 3];
                    
                    // Convert YCbCr to V24RGB, premultiply in RGB color space and compose
                    const uint32_t Rsrc = Convert_YCC_to_RGB_Red  (Y, Cr);
                    const uint32_t Gsrc = Convert_YCC_to_RGB_Green(Y, Cb, Cr);
                    const uint32_t Bsrc = Convert_YCC_to_RGB_Blue (Y, Cb);
                    
                    pDestComposite[3 * pix + R] = Blend_src_OPAQUEdst(Rsrc, pDestComposite[3 * pix + R], pSourceComposite[4 * pix + 3]);
                    pDestComposite[3 * pix + G] = Blend_src_OPAQUEdst(Gsrc, pDestComposite[3 * pix + G], pSourceComposite[4 * pix + 3]);
                    pDestComposite[3 * pix + B] = Blend_src_OPAQUEdst(Bsrc, pDestComposite[3 * pix + B], pSourceComposite[4 * pix + 3]);
                    }
                }
            }
#endif
        };

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        
        Byte* const pSourceComposite =  (Byte*)pi_pSourceRawData;
        Byte* pDestComposite = (Byte*)pio_pDestRawData;

#if oldYCC
        short R;
        short G;
        short B;

        float PreMultY;
        float AlphaPercent;
        
        // Copy entire bytes
        while(pi_PixelsCount)
            {
            PreMultY = (float)(1.3586 * pSourceComposite[0]);
            AlphaPercent = ((float) pSourceComposite[3]) / (float)255.0;

            R = (short)(PreMultY + (1.82187 * pSourceComposite[2]) + (-249.571 * AlphaPercent));
            G = (short)(PreMultY + (-0.43 * pSourceComposite[1]) + (-0.927 * pSourceComposite[2]) + (194.44 * AlphaPercent));
            B = (short)(PreMultY + (2.218 * pSourceComposite[1]) + (-345.94 * AlphaPercent));

            pDestComposite[0] = s_LookUpTable[CLAMPTABLE(R)];
            pDestComposite[1] = s_LookUpTable[CLAMPTABLE(G)];
            pDestComposite[2] = s_LookUpTable[CLAMPTABLE(B)];

            pi_PixelsCount -= 1;
            pDestComposite += 3;
            pSourceComposite += 4;
            }
#else
        for(uint32_t pix = 0; pix < pi_PixelsCount; ++pix)
            {
            uint32_t Y;
            uint32_t Cb;
            uint32_t Cr;
            
            if(0 != pSourceComposite[4 * pix + 3])
                {
                // Demultiply by alpha
                Y  = (pSourceComposite[4 * pix    ] * 255) / pSourceComposite[4 * pix + 3];
                Cb = (pSourceComposite[4 * pix + 1] * 255) / pSourceComposite[4 * pix + 3];
                Cr = (pSourceComposite[4 * pix + 2] * 255) / pSourceComposite[4 * pix + 3];
                }
            else
                {
                // No alpha value. Treat as V24PhotoYCC.
                Y  = pSourceComposite[4 * pix    ];
                Cb = pSourceComposite[4 * pix + 1];
                Cr = pSourceComposite[4 * pix + 2];
                }
                
            pDestComposite[3 * pix + R] = static_cast<Byte>(Convert_YCC_to_RGB_Red  (Y, Cr));
            pDestComposite[3 * pix + G] = static_cast<Byte>(Convert_YCC_to_RGB_Green(Y, Cb, Cr));
            pDestComposite[3 * pix + B] = static_cast<Byte>(Convert_YCC_to_RGB_Blue (Y, Cb));
            }
#endif
        };

    virtual const short* GetLostChannels() const override
        {
        return m_LostChannels;
        }
    
    HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterV32PRPhotoYCCA8_V24R8G8B8<R,G,B>(*this));
        }

private:

    static short m_LostChannels[];
    };
template<> short ConverterV32PRPhotoYCCA8_V24R8G8B8<0,1,2>::m_LostChannels[] = {3, -1};
static struct ConverterV32PRPhotoYCCA8_V24R8G8B8<0,1,2>        s_V32PRPhotoYCCA8_V24R8G8B8; // RGB

//-----------------------------------------------------------------------------
//  s_V32PRPhotoYCCA8_V24B8G8R8 - Converter
//-----------------------------------------------------------------------------
template<> short ConverterV32PRPhotoYCCA8_V24R8G8B8<2,1,0>::m_LostChannels[] = {3, -1};
static struct ConverterV32PRPhotoYCCA8_V24R8G8B8<2,1,0>        s_V32PRPhotoYCCA8_V24B8G8R8; // BGR

//-----------------------------------------------------------------------------
//  s_V32PRPhotoYCCA8_I8R8G8B8 - Converter
//-----------------------------------------------------------------------------
struct ConverterV32PRPhotoYCCA8_I8R8G8B8 : public HRPPixelConverter
    {
    HRPPaletteOctreeR8G8B8 m_QuantizedPalette;

public:
    DEFINE_T_SUPER(HRPPixelConverter)

    ConverterV32PRPhotoYCCA8_I8R8G8B8()
        {
        };

    virtual void Compose(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {

        Byte* const pSourceComposite =  (Byte*)pi_pSourceRawData;
        Byte* pDestRawData = (Byte*)pio_pDestRawData;
        Byte* pDestComposite; //Used to retrieve values from our palette

        const HRPPixelPalette& rPalette = GetDestinationPixelType()->GetPalette();

#if oldYCC
        short R;
        short G;
        short B;

        float PreMultY;
        float AlphaPercent;
        
        Byte Blend[3];

        // Copy entire bytes
        while(pi_PixelsCount)
            {
            pDestComposite = (Byte*)rPalette.GetCompositeValue(*pDest);

            PreMultY = (float)(1.3586 * pSourceComposite[0]);
            AlphaPercent = ((float) pSourceComposite[3]) / (float)255.0;

            R = (short)(PreMultY + (1.82187 * pSourceComposite[2]) + (-249.571 * AlphaPercent));
            G = (short)(PreMultY + (-0.43 * pSourceComposite[1]) + (-0.927 * pSourceComposite[2]) + (194.44 * AlphaPercent));
            B = (short)(PreMultY + (2.218 * pSourceComposite[1]) + (-345.94 * AlphaPercent));

            // (S * alpha) + (D * (1 - alpha))
            Blend[0] = ((s_LookUpTable[CLAMPTABLE(R)] * pSourceComposite[3]) +
                        (pDestComposite[0] * (255 - pSourceComposite[3]))) / 255;
            Blend[1] = ((s_LookUpTable[CLAMPTABLE(G)] * pSourceComposite[3]) +
                        (pDestComposite[1] * (255 - pSourceComposite[3]))) / 255;
            Blend[2] = ((s_LookUpTable[CLAMPTABLE(B)] * pSourceComposite[3]) +
                        (pDestComposite[2] * (255 - pSourceComposite[3]))) / 255;

            // get a good index for the R,G,B blend values
            *pDest = m_QuantizedPalette.GetIndex(
                         Blend[0],
                         Blend[1],
                         Blend[2]);

            pi_PixelsCount -= 1;
            pDest += 1;
            pSourceComposite += 4;
            }
#else


        for(uint32_t pix = 0; pix < pi_PixelsCount; ++pix)
            {  
            pDestComposite = (Byte*)rPalette.GetCompositeValue(pDestRawData[pix]);
            
            // If transparent, do nothing
            if(0 != pSourceComposite[4 * pix + 3])
                {
                Byte Rdest;
                Byte Gdest;
                Byte Bdest;
                
                if(255 == pSourceComposite[4 * pix + 3])
                    {
                    // Only conversion is required. Overwrite dest.
                    Rdest = static_cast<Byte>(Convert_YCC_to_RGB_Red  (pSourceComposite[4 * pix], pSourceComposite[4 * pix + 2]));
                    Gdest = static_cast<Byte>(Convert_YCC_to_RGB_Green(pSourceComposite[4 * pix], pSourceComposite[4 * pix + 1], pSourceComposite[4 * pix + 2]));
                    Bdest = static_cast<Byte>(Convert_YCC_to_RGB_Blue (pSourceComposite[4 * pix], pSourceComposite[4 * pix + 1]));
                    }
                else
                    {
                    // Demultiply
                    const uint32_t Y =  (pSourceComposite[4 * pix    ] * 255) / pSourceComposite[4 * pix + 3];
                    const uint32_t Cb = (pSourceComposite[4 * pix + 1] * 255) / pSourceComposite[4 * pix + 3];
                    const uint32_t Cr = (pSourceComposite[4 * pix + 2] * 255) / pSourceComposite[4 * pix + 3];
                    
                    // Convert YCbCr to V24RGB and compose
                    Rdest = Blend_src_OPAQUEdst(Convert_YCC_to_RGB_Red  (Y, Cr),     pDestComposite[0], pSourceComposite[4 * pix + 3]);
                    Gdest = Blend_src_OPAQUEdst(Convert_YCC_to_RGB_Green(Y, Cb, Cr), pDestComposite[1], pSourceComposite[4 * pix + 3]);
                    Bdest = Blend_src_OPAQUEdst(Convert_YCC_to_RGB_Blue (Y, Cb),     pDestComposite[2], pSourceComposite[4 * pix + 3]);
                    }
                    
                // get a good index for the R,G,B blend values
                pDestRawData[pix] = m_QuantizedPalette.GetIndex(Rdest, Gdest, Bdest);
                }
            }
#endif
        };

   virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {

        Byte* const pSourceComposite = (Byte*)pi_pSourceRawData;
        Byte* pDestRawData = (Byte*)pio_pDestRawData;

#if oldYCC
        short R;
        short G;
        short B;

        float PreMultY;
        float AlphaPercent;
        
        // Copy entire bytes
        while(pi_PixelsCount)
            {
            PreMultY = (float)(1.3586 * pSourceComposite[0]);
            AlphaPercent = ((float) pSourceComposite[3]) / (float)255.0;

            R = (short)(PreMultY + (1.82187 * pSourceComposite[2]) + (-249.571 * AlphaPercent));
            G = (short)(PreMultY + (-0.43 * pSourceComposite[1]) + (-0.927 * pSourceComposite[2]) + (194.44 * AlphaPercent));
            B = (short)(PreMultY + (2.218 * pSourceComposite[1]) + (-345.94 * AlphaPercent));

            // get a good index for the R,G,B blend values
            *pDest = m_QuantizedPalette.GetIndex(
                         s_LookUpTable[CLAMPTABLE(R)],
                         s_LookUpTable[CLAMPTABLE(G)],
                         s_LookUpTable[CLAMPTABLE(B)]);

            pi_PixelsCount -= 1;
            pDest += 1;
            pSourceComposite += 4;
            } 
#else
        for(uint32_t pix = 0; pix < pi_PixelsCount; ++pix)
            {
            uint32_t Y;
            uint32_t Cb;
            uint32_t Cr;
            
            if(0 != pSourceComposite[4 * pix + 3])
                {
                // Demultiply by alpha
                Y  = (pSourceComposite[4 * pix    ] * 255) / pSourceComposite[4 * pix + 3];
                Cb = (pSourceComposite[4 * pix + 1] * 255) / pSourceComposite[4 * pix + 3];
                Cr = (pSourceComposite[4 * pix + 2] * 255) / pSourceComposite[4 * pix + 3];
                }
            else
                {
                // No alpha value. Treat as V24PhotoYCC.
                Y  = pSourceComposite[4 * pix    ];
                Cb = pSourceComposite[4 * pix + 1];
                Cr = pSourceComposite[4 * pix + 2];
                }

            // get a good index for the R,G,B blend values
            pDestRawData[pix] = m_QuantizedPalette.GetIndex(static_cast<Byte>(Convert_YCC_to_RGB_Red  (Y, Cr)),
                                                            static_cast<Byte>(Convert_YCC_to_RGB_Green(Y, Cb, Cr)),
                                                            static_cast<Byte>(Convert_YCC_to_RGB_Blue (Y, Cb)));
            }
#endif
        };

   virtual const short* GetLostChannels() const override
        {
        return m_LostChannels;
        }

    HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterV32PRPhotoYCCA8_I8R8G8B8(*this));
        }

protected:

    virtual void Update() override
        {
        const HRPPixelPalette& rPalette = GetDestinationPixelType()->GetPalette();

        // fill the octree with the destination palette entries
        int32_t NbIndex(rPalette.CountUsedEntries());
        for(int Index = 0; Index < NbIndex; Index++)
            m_QuantizedPalette.AddCompositeValue(rPalette.GetCompositeValue(Index), (Byte)Index);
        }

private:
    static short m_LostChannels[];
    };
short ConverterV32PRPhotoYCCA8_I8R8G8B8::m_LostChannels[] = {3, -1};
static struct ConverterV32PRPhotoYCCA8_I8R8G8B8        s_V32PRPhotoYCCA8_I8R8G8B8;

//-----------------------------------------------------------------------------
//  s_V24R8G8B8_V32PRPhotoYCCA8 - Converter
//-----------------------------------------------------------------------------
struct ConverterV24R8G8B8_V32PRPhotoYCCA8 : public HRPPixelConverter
    {
public:
    DEFINE_T_SUPER(HRPPixelConverter)

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        Byte* const pSourceComposite =  (Byte*)pi_pSourceRawData;
        Byte* pDestComposite = (Byte*)pio_pDestRawData;

#if oldYCC
        Byte RCorrected;
        Byte GCorrected;
        Byte BCorrected;

        double RMult;
        double GMult;
        double BMult;

        double L;
        double C1;
        double C2;

        short LS;
        short C1S;
        short C2S;

        // Copy entire bytes
        while(pi_PixelsCount)
            {
//              RCorrected = s_RGBGammaTable[pSourceComposite[0]];
//              GCorrected = s_RGBGammaTable[pSourceComposite[1]];
//              BCorrected = s_RGBGammaTable[pSourceComposite[2]];
            RCorrected = pSourceComposite[0];
            GCorrected = pSourceComposite[1];
            BCorrected = pSourceComposite[2];

            // Premultiply some factors (opt)
            RMult = 0.299 * RCorrected;
            GMult = 0.587 * GCorrected;
            BMult = 0.114 * BCorrected;

            // Compute luma and chroma values
            L = RMult + GMult + BMult;
            C1 = -RMult - GMult + 0.886 * BCorrected;
            C2 = 0.701 * RCorrected - GMult - BMult;

            // Scale in [0, 255]
            LS  = (short)(L / 1.402);
            C1S = (short)(C1 * 0.436862745 + 156);   // 0.436862745 = 111.4 / 255
            C2S = (short)(C2 * 0.531921569 + 137);   // 0.531921569 = 135.64 / 255

            pDestComposite[0] = CLAMPINVTABLE(LS);
            pDestComposite[1] = CLAMPINVTABLE(C1S);
            pDestComposite[2] = CLAMPINVTABLE(C2S);
            pDestComposite[3] = 0xff;

            pi_PixelsCount -= 1;
            pDestComposite += 4;
            pSourceComposite += 3;
            }
#else
        for(uint32_t pix = 0; pix < pi_PixelsCount; ++pix)
            {
            pDestComposite[4 * pix    ] = static_cast<Byte>(Convert_RGB_to_YCC_Y (pSourceComposite[3 * pix], pSourceComposite[3 * pix + 1], pSourceComposite[3 * pix + 2]));
            pDestComposite[4 * pix + 1] = static_cast<Byte>(Convert_RGB_to_YCC_Cb(pSourceComposite[3 * pix], pSourceComposite[3 * pix + 1], pSourceComposite[3 * pix + 2]));
            pDestComposite[4 * pix + 2] = static_cast<Byte>(Convert_RGB_to_YCC_Cr(pSourceComposite[3 * pix], pSourceComposite[3 * pix + 1], pSourceComposite[3 * pix + 2]));
            pDestComposite[4 * pix + 3] = 255; // Opaque
            }
#endif
        };

    HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterV24R8G8B8_V32PRPhotoYCCA8(*this));
        }
    };
static struct ConverterV24R8G8B8_V32PRPhotoYCCA8        s_V24R8G8B8_V32PRPhotoYCCA8;

//-----------------------------------------------------------------------------
//  s_V32PRPhotoYCCA8_V32R8G8B8A8 - Converter
//-----------------------------------------------------------------------------
struct ConverterV32PRPhotoYCCA8_V32R8G8B8A8 : public HRPPixelConverter
    {
public:
    DEFINE_T_SUPER(HRPPixelConverter)

  virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {

        Byte* const pSourceComposite =  (Byte*)pi_pSourceRawData;
        Byte* pDestComposite = (Byte*)pio_pDestRawData;

#if oldYCC
        short R;
        short G;
        short B;

        float PreMultY;
        float AlphaPercent;
        
        // Copy entire bytes
        while(pi_PixelsCount)
            {
            PreMultY = (float)(1.3586 * pSourceComposite[0]);
            AlphaPercent = ((float) pSourceComposite[3]) / (float)255.0;

            R = (short)(PreMultY + (1.82187 * pSourceComposite[2]) + (-249.571 * AlphaPercent));
            G = (short)(PreMultY + (-0.43 * pSourceComposite[1]) + (-0.927 * pSourceComposite[2]) + (194.44 * AlphaPercent));
            B = (short)(PreMultY + (2.218 * pSourceComposite[1]) + (-345.94 * AlphaPercent));

            pDestComposite[0] = s_LookUpTable[CLAMPTABLE(R)];
            pDestComposite[1] = s_LookUpTable[CLAMPTABLE(G)];
            pDestComposite[2] = s_LookUpTable[CLAMPTABLE(B)];
            pDestComposite[3] = pSourceComposite[3];

            pi_PixelsCount -= 1;
            pDestComposite += 4;
            pSourceComposite += 4;
            }
#else
        for(uint32_t pix = 0; pix < pi_PixelsCount; ++pix)
            {
            uint32_t Y;
            uint32_t Cb;
            uint32_t Cr;
            
            if(0 != pSourceComposite[4 * pix + 3])
                {
                // Demultiply by alpha then convert
                Y  = (pSourceComposite[4 * pix    ] * 255) / pSourceComposite[4 * pix + 3];
                Cb = (pSourceComposite[4 * pix + 1] * 255) / pSourceComposite[4 * pix + 3];
                Cr = (pSourceComposite[4 * pix + 2] * 255) / pSourceComposite[4 * pix + 3];
                }
            else
                {
                // No alpha value. Treat as V24PhotoYCC.
                Y  = pSourceComposite[4 * pix    ];
                Cb = pSourceComposite[4 * pix + 1];
                Cr = pSourceComposite[4 * pix + 2];
                }
                
            pDestComposite[4 * pix    ] = static_cast<Byte>(Convert_YCC_to_RGB_Red  (Y, Cr));
            pDestComposite[4 * pix + 1] = static_cast<Byte>(Convert_YCC_to_RGB_Green(Y, Cb, Cr));
            pDestComposite[4 * pix + 2] = static_cast<Byte>(Convert_YCC_to_RGB_Blue (Y, Cb));
            pDestComposite[4 * pix + 3] = pSourceComposite[4 * pix + 3];
            }
#endif
        };

    virtual void Compose(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        Byte* const pSourceComposite =  (Byte*)pi_pSourceRawData;
        Byte* pDestComposite = (Byte*)pio_pDestRawData;

#if oldYCC
        short R;
        short G;
        short B;

        float PreMultY;
        float AlphaPercent;

        HFCMath (*pQuotients) (HFCMath::GetInstance());

        // Copy entire bytes
        while(pi_PixelsCount)
            {
            // If source pixel is fully transparent, destination is unaltered
            if (pSourceComposite[3] != 0)
                {
                PreMultY = (float)(1.3586 * pSourceComposite[0]);
                AlphaPercent = ((float) pSourceComposite[3]) / (float)255.0;

                R = (short)(PreMultY + (1.82187 * pSourceComposite[2]) + (-249.571 * AlphaPercent));
                G = (short)(PreMultY + (-0.43 * pSourceComposite[1]) + (-0.927 * pSourceComposite[2]) + (194.44 * AlphaPercent));
                B = (short)(PreMultY + (2.218 * pSourceComposite[1]) + (-345.94 * AlphaPercent));

                if (pDestComposite[3] == 0 || pSourceComposite[3] == 255)
                    {
                    // Destination pixel is fully transparent, or source pixel is fully opaque. Copy source pixel,
                    pDestComposite[0] = s_LookUpTable[CLAMPTABLE(R)];
                    pDestComposite[1] = s_LookUpTable[CLAMPTABLE(G)];
                    pDestComposite[2] = s_LookUpTable[CLAMPTABLE(B)];
                    pDestComposite[3] = pSourceComposite[3];
                    }
                else
                    {
                    // Cdst' = Asrc * (Csrc - (Adst * Cdst)) + (Adst * Cdst)
                    // Alphas are in [0, 255]. Tweak -> shift to divide, slight error induced.
                    Byte PremultDestColor = pQuotients->DivideBy255ToByte(pDestComposite[3] * pDestComposite[0]);
                    pDestComposite[0] = pQuotients->DivideBy255ToByte(pSourceComposite[3] * (s_LookUpTable[CLAMPTABLE(R)] - PremultDestColor)) + PremultDestColor;

                    PremultDestColor = pQuotients->DivideBy255ToByte(pDestComposite[3] * pDestComposite[1]);
                    pDestComposite[1] = pQuotients->DivideBy255ToByte(pSourceComposite[3] * (s_LookUpTable[CLAMPTABLE(G)] - PremultDestColor)) + PremultDestColor;

                    PremultDestColor = pQuotients->DivideBy255ToByte(pDestComposite[3] * pDestComposite[2]);
                    pDestComposite[2] = pQuotients->DivideBy255ToByte(pSourceComposite[3] * (s_LookUpTable[CLAMPTABLE(B)] - PremultDestColor)) + PremultDestColor;

                    // Adst' = 1 - ( (1 - Asrc) * (1 - Adst) )
                    // --> Transparency percentages are multiplied
                    pDestComposite[3] = 255 - (pQuotients->DivideBy255ToByte((255 - pSourceComposite[3]) * (255 - pDestComposite[3])));
                    }
                }

            --pi_PixelsCount;
            pDestComposite += 4;
            pSourceComposite += 4;
            }
#else
        for(uint32_t pix = 0; pix < pi_PixelsCount; ++pix)
            {
            // If transparent, do nothing
            if(0 != pSourceComposite[4 * pix + 3])
                {
                // Demultiply by alpha then convert
                const uint32_t Y  = pSourceComposite[4 * pix    ] * 255 / pSourceComposite[4 * pix + 3];
                const uint32_t Cb = pSourceComposite[4 * pix + 1] * 255 / pSourceComposite[4 * pix + 3];
                const uint32_t Cr = pSourceComposite[4 * pix + 2] * 255 / pSourceComposite[4 * pix + 3];
                
                if (0 == pDestComposite[4 * pix + 3] || 255 == pSourceComposite[4 * pix + 3])
                    {
                    // Destination pixel is fully transparent, or source pixel is fully opaque. Copy source pixel.
                    pDestComposite[4 * pix    ] = static_cast<Byte>(Convert_YCC_to_RGB_Red  (Y, Cr));
                    pDestComposite[4 * pix + 1] = static_cast<Byte>(Convert_YCC_to_RGB_Green(Y, Cb, Cr));
                    pDestComposite[4 * pix + 2] = static_cast<Byte>(Convert_YCC_to_RGB_Blue (Y, Cb));
                    pDestComposite[4 * pix + 3] = pSourceComposite[4 * pix + 3];
                    }
                else
                    {
                    const uint32_t Aout = Blend_Alpha_Channel(pSourceComposite[4 * pix + 3], pDestComposite[4 * pix + 3]);                    
                    pDestComposite[4 * pix    ] = Blend_src_dst( Convert_YCC_to_RGB_Red  (Y,Cr),    pDestComposite[4 * pix    ], pSourceComposite[4 * pix + 3], pDestComposite[4 * pix + 3], Aout);
                    pDestComposite[4 * pix + 1] = Blend_src_dst( Convert_YCC_to_RGB_Green(Y,Cb,Cr), pDestComposite[4 * pix + 1], pSourceComposite[4 * pix + 3], pDestComposite[4 * pix + 3], Aout);
                    pDestComposite[4 * pix + 2] = Blend_src_dst( Convert_YCC_to_RGB_Blue (Y,Cb),    pDestComposite[4 * pix + 2], pSourceComposite[4 * pix + 3], pDestComposite[4 * pix + 3], Aout);
                    pDestComposite[4 * pix + 3] = static_cast<Byte>(Aout);
                    }
                }
            }
#endif            
        };

    HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterV32PRPhotoYCCA8_V32R8G8B8A8(*this));
        }

private:

    };
static struct ConverterV32PRPhotoYCCA8_V32R8G8B8A8        s_V32PRPhotoYCCA8_V32R8G8B8A8;

//-----------------------------------------------------------------------------
//  s_V32R8G8B8A8_V32PRPhotoYCCA8 - Converter
//-----------------------------------------------------------------------------
struct ConverterV32R8G8B8A8_V32PRPhotoYCCA8 : public HRPPixelConverter
    {
public:
    DEFINE_T_SUPER(HRPPixelConverter)

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        Byte* const pSourceComposite =  (Byte*)pi_pSourceRawData;
        Byte* pDestComposite = (Byte*)pio_pDestRawData;

#if oldYCC
        Byte RCorrected;
        Byte GCorrected;
        Byte BCorrected;

        double RMult;
        double GMult;
        double BMult;

        double L;
        double C1;
        double C2;

        short LS;
        short C1S;
        short C2S;

        // Copy entire bytes
        while(pi_PixelsCount)
            {
//                RCorrected = s_RGBGammaTable[pSourceComposite[0]];
//                GCorrected = s_RGBGammaTable[pSourceComposite[1]];
//                BCorrected = s_RGBGammaTable[pSourceComposite[2]];
            RCorrected = pSourceComposite[0];
            GCorrected = pSourceComposite[1];
            BCorrected = pSourceComposite[2];

            // Premultiply some factors (opt)
            RMult = 0.299 * RCorrected;
            GMult = 0.587 * GCorrected;
            BMult = 0.114 * BCorrected;

            // Compute luma and chroma values
            L = RMult + GMult + BMult;
            C1 = -RMult - GMult + 0.886 * BCorrected;
            C2 = 0.701 * RCorrected - GMult - BMult;

            // Scale in [0, 255]
            LS  = (short)(L / 1.402);
            C1S = (short)(C1 * 0.436862745 + 156);   // 0.436862745 = 111.4 / 255
            C2S = (short)(C2 * 0.531921569 + 137);   // 0.531921569 = 135.64 / 255

            pDestComposite[0] = CLAMPINVTABLE(LS) * pSourceComposite[3] / 255;
            pDestComposite[1] = CLAMPINVTABLE(C1S) * pSourceComposite[3] / 255;
            pDestComposite[2] = CLAMPINVTABLE(C2S) * pSourceComposite[3] / 255;
            pDestComposite[3] = pSourceComposite[3];

            pi_PixelsCount -= 1;
            pDestComposite += 4;
            pSourceComposite += 4;
            }
#else
        for(uint32_t pix = 0; pix < pi_PixelsCount; ++pix)
            {
            pDestComposite[4 * pix]     = static_cast<Byte>( (Convert_RGB_to_YCC_Y (pSourceComposite[4 * pix], pSourceComposite[4 * pix + 1], pSourceComposite[4 * pix + 2]) ) * pSourceComposite[4 * pix + 3] / 255 );
            pDestComposite[4 * pix + 1] = static_cast<Byte>( (Convert_RGB_to_YCC_Cb(pSourceComposite[4 * pix], pSourceComposite[4 * pix + 1], pSourceComposite[4 * pix + 2]) ) * pSourceComposite[4 * pix + 3] / 255 ); 
            pDestComposite[4 * pix + 2] = static_cast<Byte>( (Convert_RGB_to_YCC_Cr(pSourceComposite[4 * pix], pSourceComposite[4 * pix + 1], pSourceComposite[4 * pix + 2]) ) * pSourceComposite[4 * pix + 3] / 255 ); 
            pDestComposite[4 * pix + 3] = pSourceComposite[4 * pix + 3];
            }
#endif
        };

    virtual void Compose(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        Byte* const pSourceComposite =  (Byte*)pi_pSourceRawData;
        Byte* pDestComposite = (Byte*)pio_pDestRawData;
        
        for(uint32_t pix = 0; pix < pi_PixelsCount; ++pix)
            {
            // If transparent, do nothing
            if(0 != pSourceComposite[4 * pix + 3])
                {
                // Convert and multiply by alpha
                const uint32_t Y  = (Convert_RGB_to_YCC_Y (pSourceComposite[4 * pix], pSourceComposite[4 * pix + 1], pSourceComposite[4 * pix + 2]) ) * pSourceComposite[4 * pix + 3] / 255;
                const uint32_t Cb = (Convert_RGB_to_YCC_Cb(pSourceComposite[4 * pix], pSourceComposite[4 * pix + 1], pSourceComposite[4 * pix + 2]) ) * pSourceComposite[4 * pix + 3] / 255;
                const uint32_t Cr = (Convert_RGB_to_YCC_Cr(pSourceComposite[4 * pix], pSourceComposite[4 * pix + 1], pSourceComposite[4 * pix + 2]) ) * pSourceComposite[4 * pix + 3] / 255;
                                    
                if (0 == pDestComposite[4 * pix + 3] || 255 == pSourceComposite[4 * pix + 3])
                    {
                    // Destination pixel is fully transparent, or source pixel is fully opaque. Copy source pixel. 
                    pDestComposite[4 * pix    ] = static_cast<Byte>(Y );
                    pDestComposite[4 * pix + 1] = static_cast<Byte>(Cb);
                    pDestComposite[4 * pix + 2] = static_cast<Byte>(Cr);
                    pDestComposite[4 * pix + 3] = pSourceComposite[4 * pix + 3];
                    }
                else
                    {
                    // Compose. at this point both source and destination are in V32PRPhotoYccA8 format.
                    pDestComposite[4 * pix    ] = Blend_PRsrc_PRdst(Y,  pDestComposite[4 * pix    ], pSourceComposite[4 * pix + 3]);
                    pDestComposite[4 * pix + 1] = Blend_PRsrc_PRdst(Cb, pDestComposite[4 * pix + 1], pSourceComposite[4 * pix + 3]);
                    pDestComposite[4 * pix + 2] = Blend_PRsrc_PRdst(Cr, pDestComposite[4 * pix + 2], pSourceComposite[4 * pix + 3]);
                    
                    pDestComposite[4 * pix + 3] = Blend_Alpha_Channel(pSourceComposite[4 * pix + 3], pDestComposite[4 * pix + 3]);  
                    }
                }
            }
        };

    HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterV32R8G8B8A8_V32PRPhotoYCCA8(*this));
        }
    };
static struct ConverterV32R8G8B8A8_V32PRPhotoYCCA8        s_V32R8G8B8A8_V32PRPhotoYCCA8;

//-----------------------------------------------------------------------------
//  Dictionary of converters from other pixel types
//-----------------------------------------------------------------------------
struct V32PRPhotoYCCA8ConvertersFrom : public MapHRPPixelTypeToConverter
    {
    V32PRPhotoYCCA8ConvertersFrom() : MapHRPPixelTypeToConverter ()
        {
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV32PRPhotoYCCA8::CLASS_ID, &s_V32PRPhotoYCCA8_V32PRPhotoYCCA8));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV24R8G8B8::CLASS_ID, &s_V24R8G8B8_V32PRPhotoYCCA8));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV24PhotoYCC::CLASS_ID, &s_V24PhotoYCC_V32PRPhotoYCCA8));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV32R8G8B8A8::CLASS_ID, &s_V32R8G8B8A8_V32PRPhotoYCCA8));
        };
    };

//-----------------------------------------------------------------------------
//  Dictionary of converters from other pixel types
//-----------------------------------------------------------------------------
struct V32PRPhotoYCCA8ConvertersTo : public MapHRPPixelTypeToConverter
    {
    V32PRPhotoYCCA8ConvertersTo() : MapHRPPixelTypeToConverter ()
        {
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV32PRPhotoYCCA8::CLASS_ID, &s_V32PRPhotoYCCA8_V32PRPhotoYCCA8));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV24R8G8B8::CLASS_ID, &s_V32PRPhotoYCCA8_V24R8G8B8));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeI8R8G8B8::CLASS_ID, &s_V32PRPhotoYCCA8_I8R8G8B8));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV24B8G8R8::CLASS_ID, &s_V32PRPhotoYCCA8_V24B8G8R8));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV24PhotoYCC::CLASS_ID, &s_V32PRPhotoYCCA8_V24PhotoYCC));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV32R8G8B8A8::CLASS_ID, &s_V32PRPhotoYCCA8_V32R8G8B8A8));
        };
    };

//-----------------------------------------------------------------------------
// Constructor for true color 24 bits RGB
//-----------------------------------------------------------------------------
HRPPixelTypeV32PRPhotoYCCA8::HRPPixelTypeV32PRPhotoYCCA8()
    : HRPPixelType(HRPChannelOrgPRPhotoYCCA8(), 0)
    {
    }


//-----------------------------------------------------------------------------
// Copy constructor
//-----------------------------------------------------------------------------
HRPPixelTypeV32PRPhotoYCCA8::HRPPixelTypeV32PRPhotoYCCA8(const HRPPixelTypeV32PRPhotoYCCA8& pi_rObj)
    : HRPPixelType(pi_rObj)
    {
    }

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
HRPPixelTypeV32PRPhotoYCCA8::~HRPPixelTypeV32PRPhotoYCCA8()
    {
    }

//-----------------------------------------------------------------------------
// Clone method
//-----------------------------------------------------------------------------
HPMPersistentObject* HRPPixelTypeV32PRPhotoYCCA8::Clone() const
    {
    return new HRPPixelTypeV32PRPhotoYCCA8(*this);
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
unsigned short HRPPixelTypeV32PRPhotoYCCA8::CountValueBits() const
    {
    return 32;
    }

//-----------------------------------------------------------------------------
// HasConverterFrom
//-----------------------------------------------------------------------------
const HRPPixelConverter* HRPPixelTypeV32PRPhotoYCCA8::HasConverterFrom(const HRPPixelType* pi_pPixelTypeFrom) const
    {
    HFCMonitor Monitor(HRPPixelType::s_ConverterAccess);

    // This declaration needs to remain local in order to prevent initialization problems
    static struct V32PRPhotoYCCA8ConvertersFrom s_ConvertersFrom;

    MapHRPPixelTypeToConverter::const_iterator Itr;
    Itr = s_ConvertersFrom.find (pi_pPixelTypeFrom->GetClassID());

    return ((Itr == s_ConvertersFrom.end()) ? 0 : (*Itr).second);
    }

//-----------------------------------------------------------------------------
// HasConverterTo
//-----------------------------------------------------------------------------
const HRPPixelConverter* HRPPixelTypeV32PRPhotoYCCA8::HasConverterTo(const HRPPixelType* pi_pPixelTypeTo) const
    {
    HFCMonitor Monitor(HRPPixelType::s_ConverterAccess);

    // This declaration needs to remain local in order to prevent initialization problems
    static struct V32PRPhotoYCCA8ConvertersTo s_ConvertersTo;

    MapHRPPixelTypeToConverter::const_iterator Itr;
    Itr = s_ConvertersTo.find (pi_pPixelTypeTo->GetClassID());

    return ((Itr == s_ConvertersTo.end()) ? 0 : (*Itr).second);
    }

