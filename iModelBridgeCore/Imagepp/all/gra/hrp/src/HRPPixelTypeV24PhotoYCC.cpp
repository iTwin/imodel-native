//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrp/src/HRPPixelTypeV24PhotoYCC.cpp $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HRPPixelTypeV24PhotoYCC
//-----------------------------------------------------------------------------

#include <ImageppInternal.h>


#include <Imagepp/all/h/HRPPixelTypeV24PhotoYCC.h>

#include <Imagepp/all/h/HRPPaletteOctreeR8G8B8.h>
#include <Imagepp/all/h/HRPChannelOrgPhotoYCC.h>

#include <Imagepp/all/h/HRPPixelTypeI8R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeV24B8G8R8.h>
#include <Imagepp/all/h/HRPPixelTypeV24R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeV32PR8PG8PB8A8.h>
#include <Imagepp/all/h/HRPPixelTypeV32R8G8B8A8.h>
#include "v24rgb.h" // for YCC conversion factor

HPM_REGISTER_CLASS(HRPPixelTypeV24PhotoYCC, HRPPixelType)

// STL typeDef
typedef map<HCLASS_ID, HRPPixelConverter*, less<HCLASS_ID>, allocator<HRPPixelConverter*> >
MapHRPPixelTypeToConverter;

/* Important info: newYCC
 *
 * Coefficients are from MSDN (http://msdn.microsoft.com/en-us/library/ff635643.aspx)
 *   => If link is broken, topic name is "Color Conversion (RGB to YCbCr)"
 *
 * Coefficients from Wikipedia (http://en.wikipedia.org/wiki/YCbCr) have also been considered,
 *   and images ended up with small, yet noticeable differences compared to MSDN's and source image.
 *
 * Blending equations: http://en.wikipedia.org/wiki/Alpha_compositing
 *
 * Use inlined functions to convert (YCC/RGB) and to do the alpha compositing
 * 
 */
 

#if oldYCC
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

static const uint16_t s_RGBGammaTable[] =
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
    
#else

// 1.4 * (Y - 128)
extern const float pixel_YCC_to_RGB_Convert_redTable[256] = 
    {
    -179.2f,-177.8f,-176.4f,-175.0f,-173.6f,-172.2f,-170.8f,-169.4f,-168.0f,-166.6f,-165.2f,
    -163.8f,-162.4f,-161.0f,-159.6f,-158.2f,-156.8f,-155.4f,-154.0f,-152.6f,-151.2f,
    -149.8f,-148.4f,-147.0f,-145.6f,-144.2f,-142.8f,-141.4f,-140.0f,-138.6f,-137.2f,
    -135.8f,-134.4f,-133.0f,-131.6f,-130.2f,-128.8f,-127.4f,-126.0f,-124.6f,-123.2f,
    -121.8f,-120.4f,-119.0f,-117.6f,-116.2f,-114.8f,-113.4f,-112.0f,-110.6f,-109.2f,
    -107.8f,-106.4f,-105.0f,-103.6f,-102.2f,-100.8f,-99.4f,-98.0f,-96.6f,-95.2f,
    -93.8f,-92.4f,-91.0f,-89.6f,-88.2f,-86.8f,-85.4f,-84.0f,-82.6f,-81.2f,
    -79.8f,-78.4f,-77.0f,-75.6f,-74.2f,-72.8f,-71.4f,-70.0f,-68.6f,-67.2f,
    -65.8f,-64.4f,-63.0f,-61.6f,-60.2f,-58.8f,-57.4f,-56.0f,-54.6f,-53.2f,
    -51.8f,-50.4f,-49.0f,-47.6f,-46.2f,-44.8f,-43.4f,-42.0f,-40.6f,-39.2f,
    -37.8f,-36.4f,-35.0f,-33.6f,-32.2f,-30.8f,-29.4f,-28.0f,-26.6f,-25.2f,
    -23.8f,-22.4f,-21.0f,-19.6f,-18.2f,-16.8f,-15.4f,-14.0f,-12.6f,-11.2f,
    -9.8f,-8.4f,-7.0f,-5.6f,-4.2f,-2.8f,-1.4f,0.0f,1.4f,2.8f,
    4.2f,5.6f,7.0f,8.4f,9.8f,11.2f,12.6f,14.0f,15.4f,16.8f,
    18.2f,19.6f,21.0f,22.4f,23.8f,25.2f,26.6f,28.0f,29.4f,30.8f,
    32.2f,33.6f,35.0f,36.4f,37.8f,39.2f,40.6f,42.0f,43.4f,44.8f,
    46.2f,47.6f,49.0f,50.4f,51.8f,53.2f,54.6f,56.0f,57.4f,58.8f,
    60.2f,61.6f,63.0f,64.4f,65.8f,67.2f,68.6f,70.0f,71.4f,72.8f,
    74.2f,75.6f,77.0f,78.4f,79.8f,81.2f,82.6f,84.0f,85.4f,86.8f,
    88.2f,89.6f,91.0f,92.4f,93.8f,95.2f,96.6f,98.0f,99.4f,100.8f,
    102.2f,103.6f,105.0f,106.4f,107.8f,109.2f,110.6f,112.0f,113.4f,114.8f,
    116.2f,117.6f,119.0f,120.4f,121.8f,123.2f,124.6f,126.0f,127.4f,128.8f,
    130.2f,131.6f,133.0f,134.4f,135.8f,137.2f,138.6f,140.0f,141.4f,142.8f,
    144.2f,145.6f,147.0f,148.4f,149.8f,151.2f,152.6f,154.0f,155.4f,156.8f,
    158.2f,159.6f,161.0f,162.4f,163.8f,165.2f,166.6f,168.0f,169.4f,170.8f,
    172.2f,173.6f,175.0f,176.4f
    };

// -0.343 * (Cb - 128)
extern const float pixel_YCC_to_RGB_Convert_greenTable1[256] = 
    {
    43.904f,43.561f,43.218f,42.875f,42.532f,42.189f,41.846f,41.503f,41.16f,40.817f,40.474f,
    40.131f,39.788f,39.445f,39.102f,38.759f,38.416f,38.073f,37.73f,37.387f,37.044f,
    36.701f,36.358f,36.015f,35.672f,35.329f,34.986f,34.643f,34.3f,33.957f,33.614f,
    33.271f,32.928f,32.585f,32.242f,31.899f,31.556f,31.213f,30.87f,30.527f,30.184f,
    29.841f,29.498f,29.155f,28.812f,28.469f,28.126f,27.783f,27.44f,27.097f,26.754f,
    26.411f,26.068f,25.725f,25.382f,25.039f,24.696f,24.353f,24.01f,23.667f,23.324f,
    22.981f,22.638f,22.295f,21.952f,21.609f,21.266f,20.923f,20.58f,20.237f,19.894f,
    19.551f,19.208f,18.865f,18.522f,18.179f,17.836f,17.493f,17.15f,16.807f,16.464f,
    16.121f,15.778f,15.435f,15.092f,14.749f,14.406f,14.063f,13.72f,13.377f,13.034f,
    12.691f,12.348f,12.005f,11.662f,11.319f,10.976f,10.633f,10.29f,9.947f,9.604f,
    9.261f,8.918f,8.575f,8.232f,7.889f,7.546f,7.203f,6.86f,6.517f,6.174f,
    5.831f,5.488f,5.145f,4.802f,4.459f,4.116f,3.773f,3.43f,3.087f,2.744f,
    2.401f,2.058f,1.715f,1.372f,1.029f,0.686f,0.343f,-0.0f,-0.343f,-0.686f,
    -1.029f,-1.372f,-1.715f,-2.058f,-2.401f,-2.744f,-3.087f,-3.43f,-3.773f,-4.116f,
    -4.459f,-4.802f,-5.145f,-5.488f,-5.831f,-6.174f,-6.517f,-6.86f,-7.203f,-7.546f,
    -7.889f,-8.232f,-8.575f,-8.918f,-9.261f,-9.604f,-9.947f,-10.29f,-10.633f,-10.976f,
    -11.319f,-11.662f,-12.005f,-12.348f,-12.691f,-13.034f,-13.377f,-13.72f,-14.063f,-14.406f,
    -14.749f,-15.092f,-15.435f,-15.778f,-16.121f,-16.464f,-16.807f,-17.15f,-17.493f,-17.836f,
    -18.179f,-18.522f,-18.865f,-19.208f,-19.551f,-19.894f,-20.237f,-20.58f,-20.923f,-21.266f,
    -21.609f,-21.952f,-22.295f,-22.638f,-22.981f,-23.324f,-23.667f,-24.01f,-24.353f,-24.696f,
    -25.039f,-25.382f,-25.725f,-26.068f,-26.411f,-26.754f,-27.097f,-27.44f,-27.783f,-28.126f,
    -28.469f,-28.812f,-29.155f,-29.498f,-29.841f,-30.184f,-30.527f,-30.87f,-31.213f,-31.556f,
    -31.899f,-32.242f,-32.585f,-32.928f,-33.271f,-33.614f,-33.957f,-34.3f,-34.643f,-34.986f,
    -35.329f,-35.672f,-36.015f,-36.358f,-36.701f,-37.044f,-37.387f,-37.73f,-38.073f,-38.416f,
    -38.759f,-39.102f,-39.445f,-39.788f,-40.131f,-40.474f,-40.817f,-41.16f,-41.503f,-41.846f,
    -42.189f,-42.532f,-42.875f,-43.218f
    };
// -0.711 * (Cr - 128)
extern const float pixel_YCC_to_RGB_Convert_greenTable2[256] = 
    {
    91.008f,90.297f,89.586f,88.875f,88.164f,87.453f,86.742f,86.031f,85.32f,84.609f,83.898f,
    83.187f,82.476f,81.765f,81.054f,80.343f,79.632f,78.921f,78.21f,77.499f,76.788f,
    76.077f,75.366f,74.655f,73.944f,73.233f,72.522f,71.811f,71.1f,70.389f,69.678f,
    68.967f,68.256f,67.545f,66.834f,66.123f,65.412f,64.701f,63.99f,63.279f,62.568f,
    61.857f,61.146f,60.435f,59.724f,59.013f,58.302f,57.591f,56.88f,56.169f,55.458f,
    54.747f,54.036f,53.325f,52.614f,51.903f,51.192f,50.481f,49.77f,49.059f,48.348f,
    47.637f,46.926f,46.215f,45.504f,44.793f,44.082f,43.371f,42.66f,41.949f,41.238f,
    40.527f,39.816f,39.105f,38.394f,37.683f,36.972f,36.261f,35.55f,34.839f,34.128f,
    33.417f,32.706f,31.995f,31.284f,30.573f,29.862f,29.151f,28.44f,27.729f,27.018f,
    26.307f,25.596f,24.885f,24.174f,23.463f,22.752f,22.041f,21.33f,20.619f,19.908f,
    19.197f,18.486f,17.775f,17.064f,16.353f,15.642f,14.931f,14.22f,13.509f,12.798f,
    12.087f,11.376f,10.665f,9.954f,9.243f,8.532f,7.821f,7.11f,6.399f,5.688f,
    4.977f,4.266f,3.555f,2.844f,2.133f,1.422f,0.711f,-0.0f,-0.711f,-1.422f,
    -2.133f,-2.844f,-3.555f,-4.266f,-4.977f,-5.688f,-6.399f,-7.11f,-7.821f,-8.532f,
    -9.243f,-9.954f,-10.665f,-11.376f,-12.087f,-12.798f,-13.509f,-14.22f,-14.931f,-15.642f,
    -16.353f,-17.064f,-17.775f,-18.486f,-19.197f,-19.908f,-20.619f,-21.33f,-22.041f,-22.752f,
    -23.463f,-24.174f,-24.885f,-25.596f,-26.307f,-27.018f,-27.729f,-28.44f,-29.151f,-29.862f,
    -30.573f,-31.284f,-31.995f,-32.706f,-33.417f,-34.128f,-34.839f,-35.55f,-36.261f,-36.972f,
    -37.683f,-38.394f,-39.105f,-39.816f,-40.527f,-41.238f,-41.949f,-42.66f,-43.371f,-44.082f,
    -44.793f,-45.504f,-46.215f,-46.926f,-47.637f,-48.348f,-49.059f,-49.77f,-50.481f,-51.192f,
    -51.903f,-52.614f,-53.325f,-54.036f,-54.747f,-55.458f,-56.169f,-56.88f,-57.591f,-58.302f,
    -59.013f,-59.724f,-60.435f,-61.146f,-61.857f,-62.568f,-63.279f,-63.99f,-64.701f,-65.412f,
    -66.123f,-66.834f,-67.545f,-68.256f,-68.967f,-69.678f,-70.389f,-71.1f,-71.811f,-72.522f,
    -73.233f,-73.944f,-74.655f,-75.366f,-76.077f,-76.788f,-77.499f,-78.21f,-78.921f,-79.632f,
    -80.343f,-81.054f,-81.765f,-82.476f,-83.187f,-83.898f,-84.609f,-85.32f,-86.031f,-86.742f,
    -87.453f,-88.164f,-88.875f,-89.586f
    };
// 1.765 * (Cb - 128)
extern const float pixel_YCC_to_RGB_Convert_blueTable[256] = 
    {
    -225.92f,-224.155f,-222.39f,-220.625f,-218.86f,-217.095f,-215.33f,-213.565f,-211.8f,-210.035f,-208.27f,
    -206.505f,-204.74f,-202.975f,-201.21f,-199.445f,-197.68f,-195.915f,-194.15f,-192.385f,-190.62f,
    -188.855f,-187.09f,-185.325f,-183.56f,-181.795f,-180.03f,-178.265f,-176.5f,-174.735f,-172.97f,
    -171.205f,-169.44f,-167.675f,-165.91f,-164.145f,-162.38f,-160.615f,-158.85f,-157.085f,-155.32f,
    -153.555f,-151.79f,-150.025f,-148.26f,-146.495f,-144.73f,-142.965f,-141.2f,-139.435f,-137.67f,
    -135.905f,-134.14f,-132.375f,-130.61f,-128.845f,-127.08f,-125.315f,-123.55f,-121.785f,-120.02f,
    -118.255f,-116.49f,-114.725f,-112.96f,-111.195f,-109.43f,-107.665f,-105.9f,-104.135f,-102.37f,
    -100.605f,-98.84f,-97.075f,-95.31f,-93.545f,-91.78f,-90.015f,-88.25f,-86.485f,-84.72f,
    -82.955f,-81.19f,-79.425f,-77.66f,-75.895f,-74.13f,-72.365f,-70.6f,-68.835f,-67.07f,
    -65.305f,-63.54f,-61.775f,-60.01f,-58.245f,-56.48f,-54.715f,-52.95f,-51.185f,-49.42f,
    -47.655f,-45.89f,-44.125f,-42.36f,-40.595f,-38.83f,-37.065f,-35.3f,-33.535f,-31.77f,
    -30.005f,-28.24f,-26.475f,-24.71f,-22.945f,-21.18f,-19.415f,-17.65f,-15.885f,-14.12f,
    -12.355f,-10.59f,-8.825f,-7.06f,-5.295f,-3.53f,-1.765f,0.0f,1.765f,3.53f,
    5.295f,7.06f,8.825f,10.59f,12.355f,14.12f,15.885f,17.65f,19.415f,21.18f,
    22.945f,24.71f,26.475f,28.24f,30.005f,31.77f,33.535f,35.3f,37.065f,38.83f,
    40.595f,42.36f,44.125f,45.89f,47.655f,49.42f,51.185f,52.95f,54.715f,56.48f,
    58.245f,60.01f,61.775f,63.54f,65.305f,67.07f,68.835f,70.6f,72.365f,74.13f,
    75.895f,77.66f,79.425f,81.19f,82.955f,84.72f,86.485f,88.25f,90.015f,91.78f,
    93.545f,95.31f,97.075f,98.84f,100.605f,102.37f,104.135f,105.9f,107.665f,109.43f,
    111.195f,112.96f,114.725f,116.49f,118.255f,120.02f,121.785f,123.55f,125.315f,127.08f,
    128.845f,130.61f,132.375f,134.14f,135.905f,137.67f,139.435f,141.2f,142.965f,144.73f,
    146.495f,148.26f,150.025f,151.79f,153.555f,155.32f,157.085f,158.85f,160.615f,162.38f,
    164.145f,165.91f,167.675f,169.44f,171.205f,172.97f,174.735f,176.5f,178.265f,180.03f,
    181.795f,183.56f,185.325f,187.09f,188.855f,190.62f,192.385f,194.15f,195.915f,197.68f,
    199.445f,201.21f,202.975f,204.74f,206.505f,208.27f,210.035f,211.8f,213.565f,215.33f,
    217.095f,218.86f,220.625f,222.39f
    };
    
#endif

//-----------------------------------------------------------------------------
//  s_V24PhotoYCC_V24PhotoYCC - Converter
//-----------------------------------------------------------------------------
struct ConverterV24PhotoYCC_V24PhotoYCC : public HRPPixelConverter
    {
public:
    DEFINE_T_SUPER(HRPPixelConverter)

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        memcpy(pio_pDestRawData, pi_pSourceRawData, pi_PixelsCount * 3);
        };

    HRPPixelConverter* AllocateCopy() const override{
        return(new ConverterV24PhotoYCC_V24PhotoYCC(*this));
        }
    };
static struct ConverterV24PhotoYCC_V24PhotoYCC        s_V24PhotoYCC_V24PhotoYCC;

//-----------------------------------------------------------------------------
//  s_V24PhotoYCC_V24R8G8B8 - Converter
// Templated version that works for both V24R8G8B8 and V24B8G8R8 as output
// R, G, B values represent their relative index within a pixel
//-----------------------------------------------------------------------------
template<uint32_t R, uint32_t G, uint32_t B>
struct ConverterV24PhotoYCC_V24R8G8B8 : public HRPPixelConverter
    {
public:
    DEFINE_T_SUPER(HRPPixelConverter)

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        Byte* const pSourceComposite =  (Byte*)pi_pSourceRawData;
        Byte* pDestComposite = (Byte*)pio_pDestRawData;

#if oldYCC
        int32_t Rdisplay;
        int32_t Gdisplay;
        int32_t Bdisplay;

        int32_t L;
        int32_t C1;
        int32_t C2;

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

            --pi_PixelsCount;
            pDestComposite += 3;
            pSourceComposite += 3;
            }
#else

        for(uint32_t pix = 0; pix < pi_PixelsCount; ++pix)
            {
            pDestComposite[3 * pix + R] = static_cast<Byte>(Convert_YCC_to_RGB_Red  (pSourceComposite[3 * pix], pSourceComposite[3 * pix + 2]));
            pDestComposite[3 * pix + G] = static_cast<Byte>(Convert_YCC_to_RGB_Green(pSourceComposite[3 * pix], pSourceComposite[3 * pix + 1], pSourceComposite[3 * pix + 2]) );
            pDestComposite[3 * pix + B] = static_cast<Byte>(Convert_YCC_to_RGB_Blue (pSourceComposite[3 * pix], pSourceComposite[3 * pix + 1]));
            }
#endif
        };

    HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterV24PhotoYCC_V24R8G8B8<R,G,B>(*this));
        }
    };
static struct ConverterV24PhotoYCC_V24R8G8B8<0,1,2>        s_V24PhotoYCC_V24R8G8B8; // RGB

//-----------------------------------------------------------------------------
//  s_V24PhotoYCC_V24B8G8R8 - Converter
//-----------------------------------------------------------------------------
static struct ConverterV24PhotoYCC_V24R8G8B8<2,1,0>        s_V24PhotoYCC_V24B8G8R8; // BGR

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

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        Byte* const pSourceComposite =  (Byte*)pi_pSourceRawData;
        Byte* pDestRawData = (Byte*)pio_pDestRawData;

#if oldYCC
        int32_t Rdisplay;
        int32_t Gdisplay;
        int32_t Bdisplay;

        int32_t L;
        int32_t C1;
        int32_t C2;
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
#else

        for(uint32_t pix = 0; pix < pi_PixelsCount; ++pix)
            {  
            pDestRawData[pix] = m_QuantizedPalette.GetIndex(
                    static_cast<Byte>( Convert_YCC_to_RGB_Red  (pSourceComposite[3 * pix], pSourceComposite[3 * pix + 2]) ),
                    static_cast<Byte>( Convert_YCC_to_RGB_Green(pSourceComposite[3 * pix], pSourceComposite[3 * pix + 1], pSourceComposite[3 * pix + 2]) ),
                    static_cast<Byte>( Convert_YCC_to_RGB_Blue (pSourceComposite[3 * pix], pSourceComposite[3 * pix + 1]) ));
            }
#endif
        };

    HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterV24PhotoYCC_I8R8G8B8(*this));
        }

protected:

    virtual void Update() override
        {
        const HRPPixelPalette& rPalette = GetDestinationPixelType()->GetPalette();

        // fill the octree with the destination palette entries
        int32_t NbIndex(rPalette.CountUsedEntries());
        for(int32_t Index = 0; Index < NbIndex; Index++)
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

        int16_t LS;
        int16_t C1S;
        int16_t C2S;

        // Copy entire bytes
        while(pi_PixelsCount)
            {
            // Apply gamma correction
//            RCorrected = s_RGBGammaTable[((Byte *)pSourceComposite)[0]];
//            GCorrected = s_RGBGammaTable[((Byte *)pSourceComposite)[1]];
//            BCorrected = s_RGBGammaTable[((Byte *)pSourceComposite)[2]];
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
            LS  = (int16_t)(L / 1.402);
            C1S = (int16_t)(C1 * 0.436862745 + 156);   // 0.436862745 = 111.4 / 255
            C2S = (int16_t)(C2 * 0.531921569 + 137);   // 0.531921569 = 135.64 / 255

            pDestComposite[0] = CLAMPINVTABLE(LS);
            pDestComposite[1] = CLAMPINVTABLE(C1S);
            pDestComposite[2] = CLAMPINVTABLE(C2S);

            --pi_PixelsCount;
            pDestComposite += 3;
            pSourceComposite += 3;
            }
#else

        for(uint32_t pix = 0; pix < pi_PixelsCount; ++pix)
            {
            pDestComposite[3 * pix]     = static_cast<Byte>( Convert_RGB_to_YCC_Y (pSourceComposite[3 * pix], pSourceComposite[3 * pix + 1], pSourceComposite[3 * pix + 2]) );
            pDestComposite[3 * pix + 1] = static_cast<Byte>( Convert_RGB_to_YCC_Cb(pSourceComposite[3 * pix], pSourceComposite[3 * pix + 1], pSourceComposite[3 * pix + 2]) );
            pDestComposite[3 * pix + 2] = static_cast<Byte>( Convert_RGB_to_YCC_Cr(pSourceComposite[3 * pix], pSourceComposite[3 * pix + 1], pSourceComposite[3 * pix + 2]) );
            }

#endif
        };

    HRPPixelConverter* AllocateCopy() const  override{
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

        int16_t LS;
        int16_t C1S;
        int16_t C2S;

        // Copy entire bytes
        while(pi_PixelsCount)
            {
            // Apply gamma correction
//            RCorrected = s_RGBGammaTable[((Byte *)pSourceComposite)[0]];
//            GCorrected = s_RGBGammaTable[((Byte *)pSourceComposite)[1]];
//            BCorrected = s_RGBGammaTable[((Byte *)pSourceComposite)[2]];
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
            LS  = (int16_t)(L / 1.402);
            C1S = (int16_t)(C1 * 0.436862745 + 156);   // 0.436862745 = 111.4 / 255
            C2S = (int16_t)(C2 * 0.531921569 + 137);   // 0.531921569 = 135.64 / 255

            pDestComposite[0] = CLAMPINVTABLE(LS);
            pDestComposite[1] = CLAMPINVTABLE(C1S);
            pDestComposite[2] = CLAMPINVTABLE(C2S);

            --pi_PixelsCount;
            pDestComposite += 3;
            pSourceComposite += 4;
            }
#else

        for(uint32_t pix = 0; pix < pi_PixelsCount; ++pix)
            {
            pDestComposite[3 * pix]     = static_cast<Byte>( Convert_RGB_to_YCC_Y (pSourceComposite[4 * pix], pSourceComposite[4 * pix + 1], pSourceComposite[4 * pix + 2]) );
            pDestComposite[3 * pix + 1] = static_cast<Byte>( Convert_RGB_to_YCC_Cb(pSourceComposite[4 * pix], pSourceComposite[4 * pix + 1], pSourceComposite[4 * pix + 2]) );
            pDestComposite[3 * pix + 2] = static_cast<Byte>( Convert_RGB_to_YCC_Cr(pSourceComposite[4 * pix], pSourceComposite[4 * pix + 1], pSourceComposite[4 * pix + 2]) );
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
                // Source temporarily in V32-PhotoYCC color space
                uint32_t Y  = Convert_RGB_to_YCC_Y (pSourceComposite[4 * pix], pSourceComposite[4 * pix + 1], pSourceComposite[4 * pix + 2]);
                uint32_t Cb = Convert_RGB_to_YCC_Cb(pSourceComposite[4 * pix], pSourceComposite[4 * pix + 1], pSourceComposite[4 * pix + 2]);
                uint32_t Cr = Convert_RGB_to_YCC_Cr(pSourceComposite[4 * pix], pSourceComposite[4 * pix + 1], pSourceComposite[4 * pix + 2]);
                
                if(255 == pSourceComposite[4 * pix + 3])
                    {
                    // Only conversion is required. Overwrite dest.
                    pDestComposite[3 * pix    ] = static_cast<Byte>(Y );
                    pDestComposite[3 * pix + 1] = static_cast<Byte>(Cb);
                    pDestComposite[3 * pix + 2] = static_cast<Byte>(Cr);
                    }
                else
                    {
                    pDestComposite[3 * pix    ] = Blend_src_OPAQUEdst(Y , pDestComposite[3 * pix    ], pSourceComposite[4 * pix + 3]);
                    pDestComposite[3 * pix + 1] = Blend_src_OPAQUEdst(Cb, pDestComposite[3 * pix + 1], pSourceComposite[4 * pix + 3]);
                    pDestComposite[3 * pix + 2] = Blend_src_OPAQUEdst(Cr, pDestComposite[3 * pix + 2], pSourceComposite[4 * pix + 3]);
                    }
                }
            }
        }


    virtual const int16_t* GetLostChannels() const override
        {
        return m_LostChannels;
        }

    HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterV32R8G8B8A8_V24PhotoYCC(*this));
        }

private:

    static int16_t m_LostChannels[];
    };
int16_t ConverterV32R8G8B8A8_V24PhotoYCC::m_LostChannels[] = {3, -1};
static ConverterV32R8G8B8A8_V24PhotoYCC        s_V32R8G8B8A8_V24PhotoYCC;

//-----------------------------------------------------------------------------
//  s_V32PR8PG8PB8A8_V24PhotoYCC - Converter
//-----------------------------------------------------------------------------
struct ConverterV32PR8PG8PB8A8_V24PhotoYCC : public HRPPixelConverter
    {
public:
    DEFINE_T_SUPER(HRPPixelConverter)

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        Byte* const pSourceComposite =  (Byte*)pi_pSourceRawData;
        Byte* pDestComposite = (Byte*)pio_pDestRawData;

        uint32_t RCorrected;
        uint32_t GCorrected;
        uint32_t BCorrected;

#if oldYCC
        double RMult;
        double GMult;
        double BMult;

        double L;
        double C1;
        double C2;

        int16_t LS;
        int16_t C1S;
        int16_t C2S;

        // Copy entire bytes
        while(pi_PixelsCount)
            {
            // Apply gamma correction
            if(pSourceComposite[3] != 0)
                {
//                RCorrected = s_RGBGammaTable[pSourceComposite[0] * 255 / pSourceComposite[3]];
//                GCorrected = s_RGBGammaTable[pSourceComposite[1] * 255 / pSourceComposite[3]];
//                BCorrected = s_RGBGammaTable[pSourceComposite[2] * 255 / pSourceComposite[3]];
                RCorrected = pSourceComposite[0] * 255 / pSourceComposite[3];
                GCorrected = pSourceComposite[1] * 255 / pSourceComposite[3];
                BCorrected = pSourceComposite[2] * 255 / pSourceComposite[3];
                }
            else
                {
//                RCorrected = GCorrected = BCorrected = s_RGBGammaTable[0];
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
            LS =  (int16_t)(L / 1.402);
            C1S = (int16_t)(C1 * 0.436862745 + 156);   // 0.436862745 = 111.4 / 255
            C2S = (int16_t)(C2 * 0.531921569 + 137);   // 0.531921569 = 135.64 / 255

            pDestComposite[0] = CLAMPINVTABLE(LS);
            pDestComposite[1] = CLAMPINVTABLE(C1S);
            pDestComposite[2] = CLAMPINVTABLE(C2S);

            --pi_PixelsCount;
            pDestComposite += 3;
            pSourceComposite += 4;
            }
#else

        for(uint32_t pix = 0; pix < pi_PixelsCount; ++pix)
            {
            if (0 != pSourceComposite[4 * pix + 3])
                {
                RCorrected = pSourceComposite[4 * pix    ] * 255 / pSourceComposite[4 * pix + 3]; 
                GCorrected = pSourceComposite[4 * pix + 1] * 255 / pSourceComposite[4 * pix + 3];
                BCorrected = pSourceComposite[4 * pix + 2] * 255 / pSourceComposite[4 * pix + 3];
                }
            else
                {
                RCorrected = pSourceComposite[4 * pix    ];
                GCorrected = pSourceComposite[4 * pix + 1];
                BCorrected = pSourceComposite[4 * pix + 2];
                }

            pDestComposite[3 * pix]     = static_cast<Byte>( Convert_RGB_to_YCC_Y (RCorrected, GCorrected, BCorrected) ); 
            pDestComposite[3 * pix + 1] = static_cast<Byte>( Convert_RGB_to_YCC_Cb(RCorrected, GCorrected, BCorrected) );            
            pDestComposite[3 * pix + 2] = static_cast<Byte>( Convert_RGB_to_YCC_Cr(RCorrected, GCorrected, BCorrected) );            
            }
            
        }
    
    
#endif

    virtual void Compose(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {
        
        Byte* const pSourceComposite =  (Byte*)pi_pSourceRawData;
        Byte* pDestComposite = (Byte*)pio_pDestRawData;
        
        for(uint32_t pix = 0; pix < pi_PixelsCount; ++pix)
            {
            // If transparent, do nothing
            if (0 != pSourceComposite[4 * pix + 3])
                {
                uint32_t RCorrected = (pSourceComposite[4 * pix    ] * 255) / pSourceComposite[4 * pix + 3];
                uint32_t GCorrected = (pSourceComposite[4 * pix + 1] * 255) / pSourceComposite[4 * pix + 3];
                uint32_t BCorrected = (pSourceComposite[4 * pix + 2] * 255) / pSourceComposite[4 * pix + 3];
                
                uint32_t Y  = Convert_RGB_to_YCC_Y (RCorrected, GCorrected, BCorrected);
                uint32_t Cb = Convert_RGB_to_YCC_Cb(RCorrected, GCorrected, BCorrected);
                uint32_t Cr = Convert_RGB_to_YCC_Cr(RCorrected, GCorrected, BCorrected);
                
                if(255 == pSourceComposite[4 * pix + 3])
                    {
                    // Only conversion is required. Overwrite dest.
                    pDestComposite[3 * pix    ] = static_cast<Byte>(Y );
                    pDestComposite[3 * pix + 1] = static_cast<Byte>(Cb);
                    pDestComposite[3 * pix + 2] = static_cast<Byte>(Cr);
                    }
                else
                    {          
                    pDestComposite[3 * pix    ] = Blend_src_OPAQUEdst(Y , pDestComposite[3 * pix    ], pSourceComposite[4 * pix + 3]);
                    pDestComposite[3 * pix + 1] = Blend_src_OPAQUEdst(Cb, pDestComposite[3 * pix + 1], pSourceComposite[4 * pix + 3]);
                    pDestComposite[3 * pix + 2] = Blend_src_OPAQUEdst(Cr, pDestComposite[3 * pix + 2], pSourceComposite[4 * pix + 3]);
                    }
                }
            }
            
        };


    virtual const int16_t* GetLostChannels() const override
        {
        return m_LostChannels;
        }

    HRPPixelConverter* AllocateCopy() const  override{
        return(new ConverterV32PR8PG8PB8A8_V24PhotoYCC(*this));
        }

private:

    static int16_t m_LostChannels[];
    };
int16_t ConverterV32PR8PG8PB8A8_V24PhotoYCC::m_LostChannels[] = {3, -1};
static struct ConverterV32PR8PG8PB8A8_V24PhotoYCC        s_V32PR8PG8PB8A8_V24PhotoYCC;


//-----------------------------------------------------------------------------
//  s_V24PhotoYCC_V32PR8PG8PB8A8 - Converter
//-----------------------------------------------------------------------------
struct ConverterV24PhotoYCC_V32PR8PG8PB8A8 : public HRPPixelConverter
    {
public:
    DEFINE_T_SUPER(HRPPixelConverter)

    virtual void Convert(const void* pi_pSourceRawData, void* pio_pDestRawData, size_t pi_PixelsCount) const override
        {

        Byte* const pSourceComposite =  (Byte*)pi_pSourceRawData;
        Byte* pDestComposite = (Byte*)pio_pDestRawData;

#if oldYCC
        int32_t Rdisplay;
        int32_t Gdisplay;
        int32_t Bdisplay;

        int32_t L;
        int32_t C1;
        int32_t C2;

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

            --pi_PixelsCount;
            pDestComposite += 4;
            pSourceComposite += 3;
            }
#else

        for(uint32_t pix = 0; pix < pi_PixelsCount; ++pix)
            {
            pDestComposite[4 * pix    ] = static_cast<Byte>(Convert_YCC_to_RGB_Red  (pSourceComposite[3 * pix], pSourceComposite[3 * pix + 2]) );
            pDestComposite[4 * pix + 1] = static_cast<Byte>(Convert_YCC_to_RGB_Green(pSourceComposite[3 * pix], pSourceComposite[3 * pix + 1], pSourceComposite[3 * pix + 2]) );
            pDestComposite[4 * pix + 2] = static_cast<Byte>(Convert_YCC_to_RGB_Blue (pSourceComposite[3 * pix], pSourceComposite[3 * pix + 1]) );
            pDestComposite[4 * pix + 3] = 255;
            }
#endif
        };

    HRPPixelConverter* AllocateCopy() const  override{
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
uint16_t HRPPixelTypeV24PhotoYCC::CountValueBits() const
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

