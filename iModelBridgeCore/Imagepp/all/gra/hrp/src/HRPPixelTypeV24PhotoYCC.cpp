//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrp/src/HRPPixelTypeV24PhotoYCC.cpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HRPPixelTypeV24PhotoYCC
//-----------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>

#include <Imagepp/all/h/HRPPixelTypeV24PhotoYCC.h>

#include <Imagepp/all/h/HRPPaletteOctreeR8G8B8.h>
#include <Imagepp/all/h/HRPChannelOrgPhotoYCC.h>

#include <Imagepp/all/h/HRPPixelTypeI8R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeV24B8G8R8.h>
#include <Imagepp/all/h/HRPPixelTypeV24R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeV32PR8PG8PB8A8.h>
#include <Imagepp/all/h/HRPPixelTypeV32R8G8B8A8.h>

HPM_REGISTER_CLASS(HRPPixelTypeV24PhotoYCC, HRPPixelType)

// STL typeDef
typedef map<HCLASS_ID, HRPPixelConverter*, less<HCLASS_ID>, allocator<HRPPixelConverter*> >
MapHRPPixelTypeToConverter;

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
#if 0
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

// L = 1.3584 * L8bits
static const int32_t s_LTable[256] =
    {
    0,1,2,4,5,6,8,9,10,12,13,14,16,17,19,20,
    21,23,24,25,27,28,29,31,32,33,35,36,38,39,40,42,
    43,44,46,47,48,50,51,52,54,55,57,58,59,61,62,63,
    65,66,67,69,70,71,73,74,76,77,78,80,81,82,84,85,
    86,88,89,91,92,93,95,96,97,99,100,101,103,104,105,107,
    108,110,111,112,114,115,116,118,119,120,122,123,124,126,127,129,
    130,131,133,134,135,137,138,139,141,142,143,145,146,148,149,150,
    152,153,154,156,157,158,160,161,163,164,165,167,168,169,171,172,
    173,175,176,177,179,180,182,183,184,186,187,188,190,191,192,194,
    195,196,198,199,201,202,203,205,206,207,209,210,211,213,214,215,
    217,218,220,221,222,224,225,226,228,229,230,232,233,235,236,237,
    239,240,241,243,244,245,247,248,249,251,252,254,255,256,258,259,
    260,262,263,264,266,267,268,270,271,273,274,275,277,278,279,281,
    282,283,285,286,287,289,290,292,293,294,296,297,298,300,301,302,
    304,305,306,308,309,311,312,313,315,316,317,319,320,321,323,324,
    326,327,328,330,331,332,334,335,336,338,339,340,342,343,345,346
    };

// C1 = 2.2179 * (C1 - 156.0)
static const int32_t s_C1Table[256] =
    {
    -345,-343,-341,-339,-337,-334,-332,-330,-328,-326,-323,-321,-319,-317,-314,-312,
    -310,-308,-306,-303,-301,-299,-297,-294,-292,-290,-288,-286,-283,-281,-279,-277,
    -275,-272,-270,-268,-266,-263,-261,-259,-257,-255,-252,-250,-248,-246,-243,-241,
    -239,-237,-235,-232,-230,-228,-226,-224,-221,-219,-217,-215,-212,-210,-208,-206,
    -204,-201,-199,-197,-195,-192,-190,-188,-186,-184,-181,-179,-177,-175,-172,-170,
    -168,-166,-164,-161,-159,-157,-155,-153,-150,-148,-146,-144,-141,-139,-137,-135,
    -133,-130,-128,-126,-124,-121,-119,-117,-115,-113,-110,-108,-106,-104,-102,-99,
    -97,-95,-93,-90,-88,-86,-84,-82,-79,-77,-75,-73,-70,-68,-66,-64,
    -62,-59,-57,-55,-53,-51,-48,-46,-44,-42,-39,-37,-35,-33,-31,-28,
    -26,-24,-22,-19,-17,-15,-13,-11,-8,-6,-4,-2,0,2,4,6,
    8,11,13,15,17,19,22,24,26,28,31,33,35,37,39,42,
    44,46,48,51,53,55,57,59,62,64,66,68,70,73,75,77,
    79,82,84,86,88,90,93,95,97,99,102,104,106,108,110,113,
    115,117,119,121,124,126,128,130,133,135,137,139,141,144,146,148,
    150,153,155,157,159,161,164,166,168,170,172,175,177,179,181,184,
    186,188,190,192,195,197,199,201,204,206,208,210,212,215,217,219
    };

// C2 = 1.8215 * (C2 - 137.0)
static const int32_t s_C2Table[256] =
    {
    -249,-247,-245,-244,-242,-240,-238,-236,-234,-233,-231,-229,-227,-225,-224,-222,
    -220,-218,-216,-214,-213,-211,-209,-207,-205,-204,-202,-200,-198,-196,-194,-193,
    -191,-189,-187,-185,-183,-182,-180,-178,-176,-174,-173,-171,-169,-167,-165,-163,
    -162,-160,-158,-156,-154,-153,-151,-149,-147,-145,-143,-142,-140,-138,-136,-134,
    -132,-131,-129,-127,-125,-123,-122,-120,-118,-116,-114,-112,-111,-109,-107,-105,
    -103,-102,-100,-98,-96,-94,-92,-91,-89,-87,-85,-83,-81,-80,-78,-76,
    -74,-72,-71,-69,-67,-65,-63,-61,-60,-58,-56,-54,-52,-51,-49,-47,
    -45,-43,-41,-40,-38,-36,-34,-32,-30,-29,-27,-25,-23,-21,-20,-18,
    -16,-14,-12,-10,-9,-7,-5,-3,-1,0,1,3,5,7,9,10,
    12,14,16,18,20,21,23,25,27,29,30,32,34,36,38,40,
    41,43,45,47,49,51,52,54,56,58,60,61,63,65,67,69,
    71,72,74,76,78,80,81,83,85,87,89,91,92,94,96,98,
    100,102,103,105,107,109,111,112,114,116,118,120,122,123,125,127,
    129,131,132,134,136,138,140,142,143,145,147,149,151,153,154,156,
    158,160,162,163,165,167,169,171,173,174,176,178,180,182,183,185,
    187,189,191,193,194,196,198,200,202,204,205,207,209,211,213,214
    };

// 0.194 * C1
static const int32_t s_C1ProductTable[256] =
    {
    -67,-66,-66,-65,-65,-64,-64,-64,-63,-63,-62,-62,-61,-61,-61,-60,
    -60,-59,-59,-58,-58,-58,-57,-57,-56,-56,-55,-55,-55,-54,-54,-53,
    -53,-52,-52,-52,-51,-51,-50,-50,-49,-49,-49,-48,-48,-47,-47,-46,
    -46,-46,-45,-45,-44,-44,-43,-43,-43,-42,-42,-41,-41,-40,-40,-40,
    -39,-39,-38,-38,-37,-37,-37,-36,-36,-35,-35,-34,-34,-33,-33,-33,
    -32,-32,-31,-31,-30,-30,-30,-29,-29,-28,-28,-27,-27,-27,-26,-26,
    -25,-25,-24,-24,-24,-23,-23,-22,-22,-21,-21,-21,-20,-20,-19,-19,
    -18,-18,-18,-17,-17,-16,-16,-15,-15,-15,-14,-14,-13,-13,-12,-12,
    -12,-11,-11,-10,-10,-9,-9,-9,-8,-8,-7,-7,-6,-6,-6,-5,
    -5,-4,-4,-3,-3,-3,-2,-2,-1,-1,0,0,0,0,0,1,
    1,2,2,3,3,3,4,4,5,5,6,6,6,7,7,8,
    8,9,9,9,10,10,11,11,12,12,12,13,13,14,14,15,
    15,15,16,16,17,17,18,18,18,19,19,20,20,21,21,21,
    22,22,23,23,24,24,24,25,25,26,26,27,27,27,28,28,
    29,29,30,30,30,31,31,32,32,33,33,33,34,34,35,35,
    36,36,37,37,37,38,38,39,39,40,40,40,41,41,42,42
    };

// 0.509 * C2
static const int32_t s_C2ProductTable[256] =
    {
    -127,-126,-125,-124,-123,-122,-121,-120,-119,-118,-117,-116,-115,-114,-114,-113,
    -112,-111,-110,-109,-108,-107,-106,-105,-104,-103,-102,-101,-101,-100,-99,-98,
    -97,-96,-95,-94,-93,-92,-91,-90,-89,-89,-88,-87,-86,-85,-84,-83,
    -82,-81,-80,-79,-78,-77,-76,-76,-75,-74,-73,-72,-71,-70,-69,-68,
    -67,-66,-65,-64,-63,-63,-62,-61,-60,-59,-58,-57,-56,-55,-54,-53,
    -52,-51,-50,-50,-49,-48,-47,-46,-45,-44,-43,-42,-41,-40,-39,-38,
    -38,-37,-36,-35,-34,-33,-32,-31,-30,-29,-28,-27,-26,-25,-25,-24,
    -23,-22,-21,-20,-19,-18,-17,-16,-15,-14,-13,-12,-12,-11,-10,-9,
    -8,-7,-6,-5,-4,-3,-2,-1,0,0,0,1,2,3,4,5,
    6,7,8,9,10,11,12,12,13,14,15,16,17,18,19,20,
    21,22,23,24,25,25,26,27,28,29,30,31,32,33,34,35,
    36,37,38,38,39,40,41,42,43,44,45,46,47,48,49,50,
    50,51,52,53,54,55,56,57,58,59,60,61,62,63,63,64,
    65,66,67,68,69,70,71,72,73,74,75,76,76,77,78,79,
    80,81,82,83,84,85,86,87,88,89,89,90,91,92,93,94,
    95,96,97,98,99,100,101,101,102,103,104,105,106,107,108,109
    };

//-----------------------------------------------------------------------------
//  s_V24PhotoYCC_V24PhotoYCC - Converter
//-----------------------------------------------------------------------------
struct ConverterV24PhotoYCC_V24PhotoYCC : public HRPPixelConverter
    {
public:
    DEFINE_T_SUPER(HRPPixelConverter)

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData) const
        {
        memcpy(pio_pDestRawData, pi_pSourceRawData, 3);
        }

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const
        {
        memcpy(pio_pDestRawData, pi_pSourceRawData, pi_PixelsCount * 3);

        };
    virtual void    Convert(const void* pi_pSourceRawData,
                            void* pio_pDestRawData,
                            size_t pi_PixelsCount,
                            const bool* pi_pChannelsMask) const
        {
        return T_Super::Convert(pi_pSourceRawData,pio_pDestRawData,pi_PixelsCount,pi_pChannelsMask);
        }

    HRPPixelConverter* AllocateCopy() const {
        return(new ConverterV24PhotoYCC_V24PhotoYCC(*this));
        }
    };
static struct ConverterV24PhotoYCC_V24PhotoYCC        s_V24PhotoYCC_V24PhotoYCC;

//-----------------------------------------------------------------------------
//  s_V24PhotoYCC_V24R8G8B8 - Converter
//-----------------------------------------------------------------------------
struct ConverterV24PhotoYCC_V24R8G8B8 : public HRPPixelConverter
    {
public:
    DEFINE_T_SUPER(HRPPixelConverter)

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData) const
        {
        int32_t L  = s_LTable[((Byte*)pi_pSourceRawData)[0]];
        int32_t C1 = s_C1Table[((Byte*)pi_pSourceRawData)[1]];
        int32_t C2 = s_C2Table[((Byte*)pi_pSourceRawData)[2]];

        int32_t Rdisplay = L + C2;
        int32_t Gdisplay = L - s_C1ProductTable[((Byte*)pi_pSourceRawData)[1]] -
                          s_C2ProductTable[((Byte*)pi_pSourceRawData)[2]];
        int32_t Bdisplay = L + C1;

        ((Byte*)pio_pDestRawData)[0] = s_LookUpTable[CLAMPTABLE(Rdisplay)];
        ((Byte*)pio_pDestRawData)[1] = s_LookUpTable[CLAMPTABLE(Gdisplay)];
        ((Byte*)pio_pDestRawData)[2] = s_LookUpTable[CLAMPTABLE(Bdisplay)];
        }

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const
        {
        int32_t Rdisplay;
        int32_t Gdisplay;
        int32_t Bdisplay;

        int32_t L;
        int32_t C1;
        int32_t C2;

        Byte* pSourceComposite =  (Byte*)pi_pSourceRawData;
        Byte* pDestComposite = (Byte*)pio_pDestRawData;

        // Copy entire bytes
        while(pi_PixelsCount)
            {
            L = s_LTable[pSourceComposite[0]];
            C1 = s_C1Table[pSourceComposite[1]];
            C2 = s_C2Table[pSourceComposite[2]];

            Rdisplay = L + C2;
            Gdisplay = L - s_C1ProductTable[pSourceComposite[1]] - s_C2ProductTable[pSourceComposite[2]];
            Bdisplay = L + C1;

            pDestComposite[0] = s_LookUpTable[CLAMPTABLE(Rdisplay)];
            pDestComposite[1] = s_LookUpTable[CLAMPTABLE(Gdisplay)];
            pDestComposite[2] = s_LookUpTable[CLAMPTABLE(Bdisplay)];

            pi_PixelsCount -= 1;
            pDestComposite += 3;
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
        return(new ConverterV24PhotoYCC_V24R8G8B8(*this));
        }
    };
static struct ConverterV24PhotoYCC_V24R8G8B8        s_V24PhotoYCC_V24R8G8B8;

//-----------------------------------------------------------------------------
//  s_V24PhotoYCC_V24B8G8R8 - Converter
//-----------------------------------------------------------------------------
struct ConverterV24PhotoYCC_V24B8G8R8 : public HRPPixelConverter
    {
public:
    DEFINE_T_SUPER(HRPPixelConverter)

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData) const
        {
        int32_t L  = s_LTable[((Byte*)pi_pSourceRawData)[0]];
        int32_t C1 = s_C1Table[((Byte*)pi_pSourceRawData)[1]];
        int32_t C2 = s_C2Table[((Byte*)pi_pSourceRawData)[2]];

        int32_t Rdisplay = L + C2;
        int32_t Gdisplay = L - s_C1ProductTable[((Byte*)pi_pSourceRawData)[1]] -
                          s_C2ProductTable[((Byte*)pi_pSourceRawData)[2]];
        int32_t Bdisplay = L + C1;

        ((Byte*)pio_pDestRawData)[2] = s_LookUpTable[CLAMPTABLE(Rdisplay)];
        ((Byte*)pio_pDestRawData)[1] = s_LookUpTable[CLAMPTABLE(Gdisplay)];
        ((Byte*)pio_pDestRawData)[0] = s_LookUpTable[CLAMPTABLE(Bdisplay)];
        };

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const
        {
        int32_t Rdisplay;
        int32_t Gdisplay;
        int32_t Bdisplay;

        int32_t L;
        int32_t C1;
        int32_t C2;

        Byte* pSourceComposite =  (Byte*)pi_pSourceRawData;
        Byte* pDestComposite = (Byte*)pio_pDestRawData;

        // Copy entire bytes
        while(pi_PixelsCount)
            {
            L = s_LTable[pSourceComposite[0]];
            C1 = s_C1Table[pSourceComposite[1]];
            C2 = s_C2Table[pSourceComposite[2]];

            Rdisplay = L + C2;
            Gdisplay = L - s_C1ProductTable[pSourceComposite[1]] - s_C2ProductTable[pSourceComposite[2]];
            Bdisplay = L + C1;

            pDestComposite[2] = s_LookUpTable[CLAMPTABLE(Rdisplay)];
            pDestComposite[1] = s_LookUpTable[CLAMPTABLE(Gdisplay)];
            pDestComposite[0] = s_LookUpTable[CLAMPTABLE(Bdisplay)];

            pi_PixelsCount -= 1;
            pDestComposite += 3;
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

    // convert to BGR
    virtual void ConvertToValue(const void* pi_pSourceRawData, void* pio_pDestRawData) const
        {
        Convert(pi_pSourceRawData, pio_pDestRawData);
        };

    HRPPixelConverter* AllocateCopy() const {
        return(new ConverterV24PhotoYCC_V24B8G8R8(*this));
        }
    };
static struct ConverterV24PhotoYCC_V24B8G8R8        s_V24PhotoYCC_V24B8G8R8;

//-----------------------------------------------------------------------------
//  s_V24PhotoYCC_I8R8G8B8 - Converter
//-----------------------------------------------------------------------------
struct ConverterV24PhotoYCC_I8R8G8B8 : public HRPPixelConverter
    {
    HRPPaletteOctreeR8G8B8     m_QuantizedPalette;

public:
    DEFINE_T_SUPER(HRPPixelConverter)

    ConverterV24PhotoYCC_I8R8G8B8()
        {
        };

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData) const
        {
        int32_t L  = s_LTable[((Byte*)pi_pSourceRawData)[0]];
        int32_t C1 = s_C1Table[((Byte*)pi_pSourceRawData)[1]];
        int32_t C2 = s_C2Table[((Byte*)pi_pSourceRawData)[2]];

        int32_t Rdisplay = L + C2;
        int32_t Gdisplay = L - s_C1ProductTable[((Byte*)pi_pSourceRawData)[1]] -
                          s_C2ProductTable[((Byte*)pi_pSourceRawData)[2]];
        int32_t Bdisplay = L + C1;

        // get a good index for the R,G,B blend values
        *((Byte*)pio_pDestRawData) = m_QuantizedPalette.GetIndex(
                                           s_LookUpTable[CLAMPTABLE(Rdisplay)],
                                           s_LookUpTable[CLAMPTABLE(Gdisplay)],
                                           s_LookUpTable[CLAMPTABLE(Bdisplay)]);
        };

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const
        {
        int32_t Rdisplay;
        int32_t Gdisplay;
        int32_t Bdisplay;

        int32_t L;
        int32_t C1;
        int32_t C2;

        Byte* pSourceComposite =  (Byte*)pi_pSourceRawData;
        Byte* pDestRawData = (Byte*)pio_pDestRawData;

        // Copy entire bytes
        while(pi_PixelsCount)
            {
            L = s_LTable[pSourceComposite[0]];
            C1 = s_C1Table[pSourceComposite[1]];
            C2 = s_C2Table[pSourceComposite[2]];

            Rdisplay = L + C2;
            Gdisplay = L - s_C1ProductTable[pSourceComposite[1]] - s_C2ProductTable[pSourceComposite[2]];
            Bdisplay = L + C1;

            // get a good index for the R,G,B blend values
            *pDestRawData = m_QuantizedPalette.GetIndex(
                                s_LookUpTable[CLAMPTABLE(Rdisplay)],
                                s_LookUpTable[CLAMPTABLE(Gdisplay)],
                                s_LookUpTable[CLAMPTABLE(Bdisplay)]);

            pi_PixelsCount -= 1;
            pDestRawData += 1;
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

    // convert to RGB
    virtual void ConvertToValue(const void* pi_pSourceRawData, void* pio_pDestRawData) const
        {
        int32_t L  = s_LTable[((Byte*)pi_pSourceRawData)[0]];
        int32_t C1 = s_C1Table[((Byte*)pi_pSourceRawData)[1]];
        int32_t C2 = s_C2Table[((Byte*)pi_pSourceRawData)[2]];

        int32_t Rdisplay = L + C2;
        int32_t Gdisplay = L - s_C1ProductTable[((Byte*)pi_pSourceRawData)[1]] -
                          s_C2ProductTable[((Byte*)pi_pSourceRawData)[2]];
        int32_t Bdisplay = L + C1;

        ((Byte*)pio_pDestRawData)[0] = s_LookUpTable[CLAMPTABLE(Rdisplay)];
        ((Byte*)pio_pDestRawData)[1] = s_LookUpTable[CLAMPTABLE(Gdisplay)];
        ((Byte*)pio_pDestRawData)[2] = s_LookUpTable[CLAMPTABLE(Bdisplay)];
        };

    HRPPixelConverter* AllocateCopy() const {
        return(new ConverterV24PhotoYCC_I8R8G8B8(*this));
        }

protected:

    virtual void Update()
        {
        const HRPPixelPalette& rPalette = GetDestinationPixelType()->GetPalette();

        // fill the octree with the destination palette entries
        int32_t NbIndex(rPalette.CountUsedEntries());
        for(int Index = 0; Index < NbIndex; Index++)
            m_QuantizedPalette.AddCompositeValue(rPalette.GetCompositeValue(Index),
                                                 (Byte)Index);
        }
    };
static struct ConverterV24PhotoYCC_I8R8G8B8        s_V24PhotoYCC_I8R8G8B8;

//-----------------------------------------------------------------------------
//  s_V24R8G8B8_V24PhotoYCC - Converter
//-----------------------------------------------------------------------------
struct ConverterV24R8G8B8_V24PhotoYCC : public HRPPixelConverter
    {
public:
    DEFINE_T_SUPER(HRPPixelConverter)

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData) const
        {
        // Apply gamma correction
//            Byte RCorrected = s_RGBGammaTable[((Byte *)pi_pSourceRawData)[0]];
//            Byte GCorrected = s_RGBGammaTable[((Byte *)pi_pSourceRawData)[1]];
//            Byte BCorrected = s_RGBGammaTable[((Byte *)pi_pSourceRawData)[2]];
        Byte RCorrected = ((Byte*)pi_pSourceRawData)[0];
        Byte GCorrected = ((Byte*)pi_pSourceRawData)[1];
        Byte BCorrected = ((Byte*)pi_pSourceRawData)[2];

        // Premultiply some factors (opt)
        double RMult = 0.299 * RCorrected;
        double GMult = 0.587 * GCorrected;
        double BMult = 0.114 * BCorrected;

        // Compute luma and chroma values
        double L = RMult + GMult + BMult;
        double C1 = -RMult - GMult + 0.886 * BCorrected;
        double C2 = 0.701 * RCorrected - GMult - BMult;

        // Scale in [0, 255]
        short LS  = (short)(L / 1.402);
        short C1S = (short)(C1 * 0.436862745 + 156);   // 0.436862745 = 111.4 / 255
        short C2S = (short)(C2 * 0.531921569 + 137);   // 0.531921569 = 135.64 / 255

        ((Byte*)pio_pDestRawData)[0] = CLAMPINVTABLE(LS);
        ((Byte*)pio_pDestRawData)[1] = CLAMPINVTABLE(C1S);
        ((Byte*)pio_pDestRawData)[2] = CLAMPINVTABLE(C2S);
        };

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const
        {
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

        Byte* pSourceComposite =  (Byte*)pi_pSourceRawData;
        Byte* pDestComposite = (Byte*)pio_pDestRawData;

        // Copy entire bytes
        while(pi_PixelsCount)
            {
            // Apply gamma correction
//                RCorrected = s_RGBGammaTable[((Byte *)pSourceComposite)[0]];
//                GCorrected = s_RGBGammaTable[((Byte *)pSourceComposite)[1]];
//                BCorrected = s_RGBGammaTable[((Byte *)pSourceComposite)[2]];
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

            pi_PixelsCount -= 1;
            pDestComposite += 3;
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
        return(new ConverterV24R8G8B8_V24PhotoYCC(*this));
        }
    };
static struct ConverterV24R8G8B8_V24PhotoYCC        s_V24R8G8B8_V24PhotoYCC;

//-----------------------------------------------------------------------------
//  s_V32R8G8B8A8_V24PhotoYCC - Converter
//-----------------------------------------------------------------------------
class ConverterV32R8G8B8A8_V24PhotoYCC : public HRPPixelConverter
    {
public:
    DEFINE_T_SUPER(HRPPixelConverter)

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData) const
        {
        // Apply gamma correction
//            Byte RCorrected = s_RGBGammaTable[((Byte *)pi_pSourceRawData)[0]];
//            Byte GCorrected = s_RGBGammaTable[((Byte *)pi_pSourceRawData)[1]];
//            Byte BCorrected = s_RGBGammaTable[((Byte *)pi_pSourceRawData)[2]];
        Byte RCorrected = ((Byte*)pi_pSourceRawData)[0];
        Byte GCorrected = ((Byte*)pi_pSourceRawData)[1];
        Byte BCorrected = ((Byte*)pi_pSourceRawData)[2];

        // Premultiply some factors (opt)
        double RMult = 0.299 * RCorrected;
        double GMult = 0.587 * GCorrected;
        double BMult = 0.114 * BCorrected;

        // Compute luma and chroma values
        double L = RMult + GMult + BMult;
        double C1 = -RMult - GMult + 0.886 * BCorrected;
        double C2 = 0.701 * RCorrected - GMult - BMult;

        // Scale in [0, 255]
        short LS = (short)(L / 1.402);
        short C1S = (short)(C1 * 0.436862745 + 156);   // 0.436862745 = 111.4 / 255
        short C2S = (short)(C2 * 0.531921569 + 137);   // 0.531921569 = 135.64 / 255

        ((Byte*)pio_pDestRawData)[0] = CLAMPINVTABLE(LS);
        ((Byte*)pio_pDestRawData)[1] = CLAMPINVTABLE(C1S);
        ((Byte*)pio_pDestRawData)[2] = CLAMPINVTABLE(C2S);
        };

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const
        {
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

        Byte* pSourceComposite =  (Byte*)pi_pSourceRawData;
        Byte* pDestComposite = (Byte*)pio_pDestRawData;

        // Copy entire bytes
        while(pi_PixelsCount)
            {
            // Apply gamma correction
//                RCorrected = s_RGBGammaTable[((Byte *)pSourceComposite)[0]];
//                GCorrected = s_RGBGammaTable[((Byte *)pSourceComposite)[1]];
//                BCorrected = s_RGBGammaTable[((Byte *)pSourceComposite)[2]];
            RCorrected = pSourceComposite[0];
            GCorrected = pSourceComposite[1];
            BCorrected = pSourceComposite[2];

#if 0   // New possible formula, simpler  DM September 2012
            LS = 0.299 * RCorrected +  0.587 * GCorrected + 0.114 * BCorrected;
            C1S = (0.564*(BCorrected - LS)) + 128.0;
            C2S = (0.713*(RCorrected - LS)) + 128.0;
#endif

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

            pi_PixelsCount -= 1;
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

    //-----------------------------------------------------------------------------
    // Compose / Call Convert for the moment.
    //-----------------------------------------------------------------------------
    virtual void Compose(const void* pi_pSourceRawData,
                         void*       pio_pDestRawData) const
        {
        Convert(pi_pSourceRawData, pio_pDestRawData);
        }

    virtual void Compose(const void* pi_pSourceRawData,
                         void*       pio_pDestRawData,
                         size_t      pi_PixelsCount) const
        {
        Convert(pi_pSourceRawData, pio_pDestRawData, pi_PixelsCount);
        }


    virtual const short* GetLostChannels() const
        {
        return m_LostChannels;
        }

    HRPPixelConverter* AllocateCopy() const {
        return(new ConverterV32R8G8B8A8_V24PhotoYCC(*this));
        }

private:

    static short m_LostChannels[];
    };
short ConverterV32R8G8B8A8_V24PhotoYCC::m_LostChannels[] = {3, -1};
static ConverterV32R8G8B8A8_V24PhotoYCC        s_V32R8G8B8A8_V24PhotoYCC;

//-----------------------------------------------------------------------------
//  s_V32PR8PG8PB8A8_V24PhotoYCC - Converter
//-----------------------------------------------------------------------------
struct ConverterV32PR8PG8PB8A8_V24PhotoYCC : public HRPPixelConverter
    {
public:
    DEFINE_T_SUPER(HRPPixelConverter)

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData) const
        {
        Byte R;
        Byte G;
        Byte B;

        if(((Byte*)pi_pSourceRawData)[3] != 0)
            {
            R = ((Byte*)pi_pSourceRawData)[0] * 255 / ((Byte*)pi_pSourceRawData)[3];
            G = ((Byte*)pi_pSourceRawData)[1] * 255 / ((Byte*)pi_pSourceRawData)[3];
            B = ((Byte*)pi_pSourceRawData)[2] * 255 / ((Byte*)pi_pSourceRawData)[3];
            }
        else
            {
            R = G = B = 0;
            }

        // Apply gamma correction
//            Byte RCorrected = s_RGBGammaTable[((Byte *)pi_pSourceRawData)[0]];
//            Byte GCorrected = s_RGBGammaTable[((Byte *)pi_pSourceRawData)[1]];
//            Byte BCorrected = s_RGBGammaTable[((Byte *)pi_pSourceRawData)[2]];
        Byte RCorrected = ((Byte*)pi_pSourceRawData)[0];
        Byte GCorrected = ((Byte*)pi_pSourceRawData)[1];
        Byte BCorrected = ((Byte*)pi_pSourceRawData)[2];

        // Premultiply some factors (opt)
        double RMult = 0.299 * RCorrected;
        double GMult = 0.587 * GCorrected;
        double BMult = 0.114 * BCorrected;

        // Compute luma and chroma values
        double L = RMult + GMult + BMult;
        double C1 = -RMult - GMult + 0.886 * BCorrected;
        double C2 = 0.701 * RCorrected - GMult - BMult;

        // Scale in [0, 255]
        short LS = (short)(L / 1.402);
        short C1S = (short)(C1 * 0.436862745 + 156);   // 0.436862745 = 111.4 / 255
        short C2S = (short)(C2 * 0.531921569 + 137);   // 0.531921569 = 135.64 / 255

        ((Byte*)pio_pDestRawData)[0] = CLAMPINVTABLE(LS);
        ((Byte*)pio_pDestRawData)[1] = CLAMPINVTABLE(C1S);
        ((Byte*)pio_pDestRawData)[2] = CLAMPINVTABLE(C2S);
        };

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const
        {
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

        Byte* pSourceComposite =  (Byte*)pi_pSourceRawData;
        Byte* pDestComposite = (Byte*)pio_pDestRawData;

        // Copy entire bytes
        while(pi_PixelsCount)
            {
            // Apply gamma correction
            if(pSourceComposite[3] != 0)
                {
//                    RCorrected = s_RGBGammaTable[pSourceComposite[0] * 255 / pSourceComposite[3]];
//                    GCorrected = s_RGBGammaTable[pSourceComposite[1] * 255 / pSourceComposite[3]];
//                    BCorrected = s_RGBGammaTable[pSourceComposite[2] * 255 / pSourceComposite[3]];
                RCorrected = pSourceComposite[0] * 255 / pSourceComposite[3];
                GCorrected = pSourceComposite[1] * 255 / pSourceComposite[3];
                BCorrected = pSourceComposite[2] * 255 / pSourceComposite[3];
                }
            else
                {
//                    RCorrected = GCorrected = BCorrected = s_RGBGammaTable[0];
                RCorrected = GCorrected = BCorrected = 0;
                }

            // Premultiply some factors (opt)
            RMult = 0.299 * RCorrected;
            GMult = 0.587 * GCorrected;
            BMult = 0.114 * BCorrected;

            // Compute luma and chroma values
            L = RMult + GMult + BMult;
            C1 = -RMult - GMult + 0.886 * BCorrected;
            C2 = 0.701 * RCorrected - GMult - BMult;

            // Scale in [0, 255]
            LS =  (short)(L / 1.402);
            C1S = (short)(C1 * 0.436862745 + 156);   // 0.436862745 = 111.4 / 255
            C2S = (short)(C2 * 0.531921569 + 137);   // 0.531921569 = 135.64 / 255

            pDestComposite[0] = CLAMPINVTABLE(LS);
            pDestComposite[1] = CLAMPINVTABLE(C1S);
            pDestComposite[2] = CLAMPINVTABLE(C2S);

            pi_PixelsCount -= 1;
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

    //-----------------------------------------------------------------------------
    // Compose / Call Convert for the moment.
    //-----------------------------------------------------------------------------
    virtual void Compose(const void* pi_pSourceRawData,
        void*       pio_pDestRawData) const
    {
        Convert(pi_pSourceRawData, pio_pDestRawData);
    }

    virtual void Compose(const void* pi_pSourceRawData,
        void*       pio_pDestRawData,
        size_t      pi_PixelsCount) const
    {
        Convert(pi_pSourceRawData, pio_pDestRawData, pi_PixelsCount);
    }


    virtual const short* GetLostChannels() const
        {
        return m_LostChannels;
        }

    HRPPixelConverter* AllocateCopy() const {
        return(new ConverterV32PR8PG8PB8A8_V24PhotoYCC(*this));
        }

private:

    static short m_LostChannels[];
    };
short ConverterV32PR8PG8PB8A8_V24PhotoYCC::m_LostChannels[] = {3, -1};
static struct ConverterV32PR8PG8PB8A8_V24PhotoYCC        s_V32PR8PG8PB8A8_V24PhotoYCC;


//-----------------------------------------------------------------------------
//  s_V24PhotoYCC_V32PR8PG8PB8A8 - Converter
//-----------------------------------------------------------------------------
struct ConverterV24PhotoYCC_V32PR8PG8PB8A8 : public HRPPixelConverter
    {
public:
    DEFINE_T_SUPER(HRPPixelConverter)

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData) const
        {
        int32_t L  = s_LTable[((Byte*)pi_pSourceRawData)[0]];
        int32_t C1 = s_C1Table[((Byte*)pi_pSourceRawData)[1]];
        int32_t C2 = s_C2Table[((Byte*)pi_pSourceRawData)[2]];

        int32_t Rdisplay = L + C2;
        int32_t Gdisplay = L - s_C1ProductTable[((Byte*)pi_pSourceRawData)[1]] -
                          s_C2ProductTable[((Byte*)pi_pSourceRawData)[2]];
        int32_t Bdisplay = L + C1;

        ((Byte*)pio_pDestRawData)[0] = s_LookUpTable[CLAMPTABLE(Rdisplay)];
        ((Byte*)pio_pDestRawData)[1] = s_LookUpTable[CLAMPTABLE(Gdisplay)];
        ((Byte*)pio_pDestRawData)[2] = s_LookUpTable[CLAMPTABLE(Bdisplay)];
        ((Byte*)pio_pDestRawData)[3] = 255;
        }

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const
        {
        int32_t Rdisplay;
        int32_t Gdisplay;
        int32_t Bdisplay;

        int32_t L;
        int32_t C1;
        int32_t C2;

        Byte* pSourceComposite =  (Byte*)pi_pSourceRawData;
        Byte* pDestComposite = (Byte*)pio_pDestRawData;

        // Copy entire bytes
        while(pi_PixelsCount)
            {
            L = s_LTable[pSourceComposite[0]];
            C1 = s_C1Table[pSourceComposite[1]];
            C2 = s_C2Table[pSourceComposite[2]];

            Rdisplay = L + C2;
            Gdisplay = L - s_C1ProductTable[pSourceComposite[1]] - s_C2ProductTable[pSourceComposite[2]];
            Bdisplay = L + C1;

            pDestComposite[0] = s_LookUpTable[CLAMPTABLE(Rdisplay)];
            pDestComposite[1] = s_LookUpTable[CLAMPTABLE(Gdisplay)];
            pDestComposite[2] = s_LookUpTable[CLAMPTABLE(Bdisplay)];
            pDestComposite[3] = 255;

            pi_PixelsCount -= 1;
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
        return(new ConverterV24PhotoYCC_V32PR8PG8PB8A8(*this));
        }
    };
static struct ConverterV24PhotoYCC_V32PR8PG8PB8A8    s_V24PhotoYCC_V32PR8PG8PB8A8;

//-----------------------------------------------------------------------------
//  Dictionnary of converters from other pixel types
//-----------------------------------------------------------------------------
struct V24PhotoYCCConvertersFrom : public MapHRPPixelTypeToConverter
    {
    V24PhotoYCCConvertersFrom() : MapHRPPixelTypeToConverter ()
        {
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV24PhotoYCC::CLASS_ID, &s_V24PhotoYCC_V24PhotoYCC));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV24R8G8B8::CLASS_ID, &s_V24R8G8B8_V24PhotoYCC));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV32PR8PG8PB8A8::CLASS_ID, &s_V32PR8PG8PB8A8_V24PhotoYCC));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV32R8G8B8A8::CLASS_ID, &s_V32R8G8B8A8_V24PhotoYCC));
        };
    };

//-----------------------------------------------------------------------------
//  Dictionnary of converters from other pixel types
//-----------------------------------------------------------------------------
struct V24PhotoYCCConvertersTo : public MapHRPPixelTypeToConverter
    {
    V24PhotoYCCConvertersTo() : MapHRPPixelTypeToConverter ()
        {
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV24PhotoYCC::CLASS_ID, &s_V24PhotoYCC_V24PhotoYCC));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV32PR8PG8PB8A8::CLASS_ID, &s_V24PhotoYCC_V32PR8PG8PB8A8));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV24R8G8B8::CLASS_ID, &s_V24PhotoYCC_V24R8G8B8));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeI8R8G8B8::CLASS_ID, &s_V24PhotoYCC_I8R8G8B8));
        insert (MapHRPPixelTypeToConverter::value_type(HRPPixelTypeV24B8G8R8::CLASS_ID, &s_V24PhotoYCC_V24B8G8R8));
        };
    };

//-----------------------------------------------------------------------------
// Constructor for true color 24 bits RGB
//-----------------------------------------------------------------------------
HRPPixelTypeV24PhotoYCC::HRPPixelTypeV24PhotoYCC()
    : HRPPixelType(HRPChannelOrgPhotoYCC(), 0)
    {
    }


//-----------------------------------------------------------------------------
// Copy constructor
//-----------------------------------------------------------------------------
HRPPixelTypeV24PhotoYCC::HRPPixelTypeV24PhotoYCC(const HRPPixelTypeV24PhotoYCC& pi_rObj)
    : HRPPixelType(pi_rObj)
    {
    }

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
HRPPixelTypeV24PhotoYCC::~HRPPixelTypeV24PhotoYCC()
    {
    }

//-----------------------------------------------------------------------------
// Clone method
//-----------------------------------------------------------------------------
HPMPersistentObject* HRPPixelTypeV24PhotoYCC::Clone() const
    {
    return new HRPPixelTypeV24PhotoYCC(*this);
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
unsigned short HRPPixelTypeV24PhotoYCC::CountValueBits() const
    {
    return 24;
    }


//-----------------------------------------------------------------------------
// HasConverterFrom
//-----------------------------------------------------------------------------
const HRPPixelConverter* HRPPixelTypeV24PhotoYCC::HasConverterFrom(const HRPPixelType* pi_pPixelTypeFrom) const
    {
    HFCMonitor Monitor(HRPPixelType::s_ConverterAccess);

    // This declaration needs to remain local in order to prevent initialization problems
    static struct V24PhotoYCCConvertersFrom s_ConvertersFrom;

    MapHRPPixelTypeToConverter::const_iterator Itr;
    Itr = s_ConvertersFrom.find (pi_pPixelTypeFrom->GetClassID());

    return ((Itr == s_ConvertersFrom.end()) ? 0 : (*Itr).second);
    }

//-----------------------------------------------------------------------------
// HasConverterTo
//-----------------------------------------------------------------------------
const HRPPixelConverter* HRPPixelTypeV24PhotoYCC::HasConverterTo(const HRPPixelType* pi_pPixelTypeTo) const
    {
    HFCMonitor Monitor(HRPPixelType::s_ConverterAccess);

    // This declaration needs to remain local in order to prevent initialization problems
    static struct V24PhotoYCCConvertersTo s_ConvertersTo;

    MapHRPPixelTypeToConverter::const_iterator Itr;
    Itr = s_ConvertersTo.find (pi_pPixelTypeTo->GetClassID());

    return ((Itr == s_ConvertersTo.end()) ? 0 : (*Itr).second);
    }

