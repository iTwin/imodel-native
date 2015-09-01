//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrp/src/HRPPixelTypeV24PhotoYCC.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HRPPixelTypeV24PhotoYCC
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


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
    -179.2,-177.8,-176.4,-175.0,-173.6,-172.2,-170.8,-169.4,-168.0,-166.6,-165.2,
    -163.8,-162.4,-161.0,-159.6,-158.2,-156.8,-155.4,-154.0,-152.6,-151.2,
    -149.8,-148.4,-147.0,-145.6,-144.2,-142.8,-141.4,-140.0,-138.6,-137.2,
    -135.8,-134.4,-133.0,-131.6,-130.2,-128.8,-127.4,-126.0,-124.6,-123.2,
    -121.8,-120.4,-119.0,-117.6,-116.2,-114.8,-113.4,-112.0,-110.6,-109.2,
    -107.8,-106.4,-105.0,-103.6,-102.2,-100.8,-99.4,-98.0,-96.6,-95.2,
    -93.8,-92.4,-91.0,-89.6,-88.2,-86.8,-85.4,-84.0,-82.6,-81.2,
    -79.8,-78.4,-77.0,-75.6,-74.2,-72.8,-71.4,-70.0,-68.6,-67.2,
    -65.8,-64.4,-63.0,-61.6,-60.2,-58.8,-57.4,-56.0,-54.6,-53.2,
    -51.8,-50.4,-49.0,-47.6,-46.2,-44.8,-43.4,-42.0,-40.6,-39.2,
    -37.8,-36.4,-35.0,-33.6,-32.2,-30.8,-29.4,-28.0,-26.6,-25.2,
    -23.8,-22.4,-21.0,-19.6,-18.2,-16.8,-15.4,-14.0,-12.6,-11.2,
    -9.8,-8.4,-7.0,-5.6,-4.2,-2.8,-1.4,0.0,1.4,2.8,
    4.2,5.6,7.0,8.4,9.8,11.2,12.6,14.0,15.4,16.8,
    18.2,19.6,21.0,22.4,23.8,25.2,26.6,28.0,29.4,30.8,
    32.2,33.6,35.0,36.4,37.8,39.2,40.6,42.0,43.4,44.8,
    46.2,47.6,49.0,50.4,51.8,53.2,54.6,56.0,57.4,58.8,
    60.2,61.6,63.0,64.4,65.8,67.2,68.6,70.0,71.4,72.8,
    74.2,75.6,77.0,78.4,79.8,81.2,82.6,84.0,85.4,86.8,
    88.2,89.6,91.0,92.4,93.8,95.2,96.6,98.0,99.4,100.8,
    102.2,103.6,105.0,106.4,107.8,109.2,110.6,112.0,113.4,114.8,
    116.2,117.6,119.0,120.4,121.8,123.2,124.6,126.0,127.4,128.8,
    130.2,131.6,133.0,134.4,135.8,137.2,138.6,140.0,141.4,142.8,
    144.2,145.6,147.0,148.4,149.8,151.2,152.6,154.0,155.4,156.8,
    158.2,159.6,161.0,162.4,163.8,165.2,166.6,168.0,169.4,170.8,
    172.2,173.6,175.0,176.4
    };

// -0.343 * (Cb - 128)
extern const float pixel_YCC_to_RGB_Convert_greenTable1[256] = 
    {
    43.904,43.561,43.218,42.875,42.532,42.189,41.846,41.503,41.16,40.817,40.474,
    40.131,39.788,39.445,39.102,38.759,38.416,38.073,37.73,37.387,37.044,
    36.701,36.358,36.015,35.672,35.329,34.986,34.643,34.3,33.957,33.614,
    33.271,32.928,32.585,32.242,31.899,31.556,31.213,30.87,30.527,30.184,
    29.841,29.498,29.155,28.812,28.469,28.126,27.783,27.44,27.097,26.754,
    26.411,26.068,25.725,25.382,25.039,24.696,24.353,24.01,23.667,23.324,
    22.981,22.638,22.295,21.952,21.609,21.266,20.923,20.58,20.237,19.894,
    19.551,19.208,18.865,18.522,18.179,17.836,17.493,17.15,16.807,16.464,
    16.121,15.778,15.435,15.092,14.749,14.406,14.063,13.72,13.377,13.034,
    12.691,12.348,12.005,11.662,11.319,10.976,10.633,10.29,9.947,9.604,
    9.261,8.918,8.575,8.232,7.889,7.546,7.203,6.86,6.517,6.174,
    5.831,5.488,5.145,4.802,4.459,4.116,3.773,3.43,3.087,2.744,
    2.401,2.058,1.715,1.372,1.029,0.686,0.343,-0.0,-0.343,-0.686,
    -1.029,-1.372,-1.715,-2.058,-2.401,-2.744,-3.087,-3.43,-3.773,-4.116,
    -4.459,-4.802,-5.145,-5.488,-5.831,-6.174,-6.517,-6.86,-7.203,-7.546,
    -7.889,-8.232,-8.575,-8.918,-9.261,-9.604,-9.947,-10.29,-10.633,-10.976,
    -11.319,-11.662,-12.005,-12.348,-12.691,-13.034,-13.377,-13.72,-14.063,-14.406,
    -14.749,-15.092,-15.435,-15.778,-16.121,-16.464,-16.807,-17.15,-17.493,-17.836,
    -18.179,-18.522,-18.865,-19.208,-19.551,-19.894,-20.237,-20.58,-20.923,-21.266,
    -21.609,-21.952,-22.295,-22.638,-22.981,-23.324,-23.667,-24.01,-24.353,-24.696,
    -25.039,-25.382,-25.725,-26.068,-26.411,-26.754,-27.097,-27.44,-27.783,-28.126,
    -28.469,-28.812,-29.155,-29.498,-29.841,-30.184,-30.527,-30.87,-31.213,-31.556,
    -31.899,-32.242,-32.585,-32.928,-33.271,-33.614,-33.957,-34.3,-34.643,-34.986,
    -35.329,-35.672,-36.015,-36.358,-36.701,-37.044,-37.387,-37.73,-38.073,-38.416,
    -38.759,-39.102,-39.445,-39.788,-40.131,-40.474,-40.817,-41.16,-41.503,-41.846,
    -42.189,-42.532,-42.875,-43.218
    };
// -0.711 * (Cr - 128)
extern const float pixel_YCC_to_RGB_Convert_greenTable2[256] = 
    {
    91.008,90.297,89.586,88.875,88.164,87.453,86.742,86.031,85.32,84.609,83.898,
    83.187,82.476,81.765,81.054,80.343,79.632,78.921,78.21,77.499,76.788,
    76.077,75.366,74.655,73.944,73.233,72.522,71.811,71.1,70.389,69.678,
    68.967,68.256,67.545,66.834,66.123,65.412,64.701,63.99,63.279,62.568,
    61.857,61.146,60.435,59.724,59.013,58.302,57.591,56.88,56.169,55.458,
    54.747,54.036,53.325,52.614,51.903,51.192,50.481,49.77,49.059,48.348,
    47.637,46.926,46.215,45.504,44.793,44.082,43.371,42.66,41.949,41.238,
    40.527,39.816,39.105,38.394,37.683,36.972,36.261,35.55,34.839,34.128,
    33.417,32.706,31.995,31.284,30.573,29.862,29.151,28.44,27.729,27.018,
    26.307,25.596,24.885,24.174,23.463,22.752,22.041,21.33,20.619,19.908,
    19.197,18.486,17.775,17.064,16.353,15.642,14.931,14.22,13.509,12.798,
    12.087,11.376,10.665,9.954,9.243,8.532,7.821,7.11,6.399,5.688,
    4.977,4.266,3.555,2.844,2.133,1.422,0.711,-0.0,-0.711,-1.422,
    -2.133,-2.844,-3.555,-4.266,-4.977,-5.688,-6.399,-7.11,-7.821,-8.532,
    -9.243,-9.954,-10.665,-11.376,-12.087,-12.798,-13.509,-14.22,-14.931,-15.642,
    -16.353,-17.064,-17.775,-18.486,-19.197,-19.908,-20.619,-21.33,-22.041,-22.752,
    -23.463,-24.174,-24.885,-25.596,-26.307,-27.018,-27.729,-28.44,-29.151,-29.862,
    -30.573,-31.284,-31.995,-32.706,-33.417,-34.128,-34.839,-35.55,-36.261,-36.972,
    -37.683,-38.394,-39.105,-39.816,-40.527,-41.238,-41.949,-42.66,-43.371,-44.082,
    -44.793,-45.504,-46.215,-46.926,-47.637,-48.348,-49.059,-49.77,-50.481,-51.192,
    -51.903,-52.614,-53.325,-54.036,-54.747,-55.458,-56.169,-56.88,-57.591,-58.302,
    -59.013,-59.724,-60.435,-61.146,-61.857,-62.568,-63.279,-63.99,-64.701,-65.412,
    -66.123,-66.834,-67.545,-68.256,-68.967,-69.678,-70.389,-71.1,-71.811,-72.522,
    -73.233,-73.944,-74.655,-75.366,-76.077,-76.788,-77.499,-78.21,-78.921,-79.632,
    -80.343,-81.054,-81.765,-82.476,-83.187,-83.898,-84.609,-85.32,-86.031,-86.742,
    -87.453,-88.164,-88.875,-89.586
    };
// 1.765 * (Cb - 128)
extern const float pixel_YCC_to_RGB_Convert_blueTable[256] = 
    {
    -225.92,-224.155,-222.39,-220.625,-218.86,-217.095,-215.33,-213.565,-211.8,-210.035,-208.27,
    -206.505,-204.74,-202.975,-201.21,-199.445,-197.68,-195.915,-194.15,-192.385,-190.62,
    -188.855,-187.09,-185.325,-183.56,-181.795,-180.03,-178.265,-176.5,-174.735,-172.97,
    -171.205,-169.44,-167.675,-165.91,-164.145,-162.38,-160.615,-158.85,-157.085,-155.32,
    -153.555,-151.79,-150.025,-148.26,-146.495,-144.73,-142.965,-141.2,-139.435,-137.67,
    -135.905,-134.14,-132.375,-130.61,-128.845,-127.08,-125.315,-123.55,-121.785,-120.02,
    -118.255,-116.49,-114.725,-112.96,-111.195,-109.43,-107.665,-105.9,-104.135,-102.37,
    -100.605,-98.84,-97.075,-95.31,-93.545,-91.78,-90.015,-88.25,-86.485,-84.72,
    -82.955,-81.19,-79.425,-77.66,-75.895,-74.13,-72.365,-70.6,-68.835,-67.07,
    -65.305,-63.54,-61.775,-60.01,-58.245,-56.48,-54.715,-52.95,-51.185,-49.42,
    -47.655,-45.89,-44.125,-42.36,-40.595,-38.83,-37.065,-35.3,-33.535,-31.77,
    -30.005,-28.24,-26.475,-24.71,-22.945,-21.18,-19.415,-17.65,-15.885,-14.12,
    -12.355,-10.59,-8.825,-7.06,-5.295,-3.53,-1.765,0.0,1.765,3.53,
    5.295,7.06,8.825,10.59,12.355,14.12,15.885,17.65,19.415,21.18,
    22.945,24.71,26.475,28.24,30.005,31.77,33.535,35.3,37.065,38.83,
    40.595,42.36,44.125,45.89,47.655,49.42,51.185,52.95,54.715,56.48,
    58.245,60.01,61.775,63.54,65.305,67.07,68.835,70.6,72.365,74.13,
    75.895,77.66,79.425,81.19,82.955,84.72,86.485,88.25,90.015,91.78,
    93.545,95.31,97.075,98.84,100.605,102.37,104.135,105.9,107.665,109.43,
    111.195,112.96,114.725,116.49,118.255,120.02,121.785,123.55,125.315,127.08,
    128.845,130.61,132.375,134.14,135.905,137.67,139.435,141.2,142.965,144.73,
    146.495,148.26,150.025,151.79,153.555,155.32,157.085,158.85,160.615,162.38,
    164.145,165.91,167.675,169.44,171.205,172.97,174.735,176.5,178.265,180.03,
    181.795,183.56,185.325,187.09,188.855,190.62,192.385,194.15,195.915,197.68,
    199.445,201.21,202.975,204.74,206.505,208.27,210.035,211.8,213.565,215.33,
    217.095,218.86,220.625,222.39
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
            LS  = (short)(L / 1.402);
            C1S = (short)(C1 * 0.436862745 + 156);   // 0.436862745 = 111.4 / 255
            C2S = (short)(C2 * 0.531921569 + 137);   // 0.531921569 = 135.64 / 255

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

        short LS;
        short C1S;
        short C2S;

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
            LS  = (short)(L / 1.402);
            C1S = (short)(C1 * 0.436862745 + 156);   // 0.436862745 = 111.4 / 255
            C2S = (short)(C2 * 0.531921569 + 137);   // 0.531921569 = 135.64 / 255

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


    virtual const short* GetLostChannels() const override
        {
        return m_LostChannels;
        }

    HRPPixelConverter* AllocateCopy() const  override{
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

        short LS;
        short C1S;
        short C2S;

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
            LS =  (short)(L / 1.402);
            C1S = (short)(C1 * 0.436862745 + 156);   // 0.436862745 = 111.4 / 255
            C2S = (short)(C2 * 0.531921569 + 137);   // 0.531921569 = 135.64 / 255

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


    virtual const short* GetLostChannels() const override
        {
        return m_LostChannels;
        }

    HRPPixelConverter* AllocateCopy() const  override{
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

