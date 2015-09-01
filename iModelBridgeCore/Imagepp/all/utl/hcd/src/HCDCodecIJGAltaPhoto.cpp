//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hcd/src/HCDCodecIJGAltaPhoto.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HCDCodecIJGAltaPhoto
//--------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HCDCodecIJGAltaPhoto.h>

HCDCodecIJGAltaPhoto::HCDCodecIJGAltaPhoto()
    {
    SetAltaPhotoCodecParameters();
    }

HCDCodecIJGAltaPhoto::HCDCodecIJGAltaPhoto(uint32_t pi_Width,
                                           uint32_t pi_Height,
                                           uint32_t pi_BitsPerPixel)
    : HCDCodecIJG(pi_Width, pi_Height, pi_BitsPerPixel)
    {
    SetAltaPhotoCodecParameters();
    }

HCDCodecIJGAltaPhoto::HCDCodecIJGAltaPhoto(const HCDCodecIJGAltaPhoto& pi_rObj)
    : HCDCodecIJG(pi_rObj)
    {
    }

HCDCodecIJGAltaPhoto::~HCDCodecIJGAltaPhoto()
    {
    }

void HCDCodecIJGAltaPhoto::SetAltaPhotoCodecParameters()
    {
    // 1:6 zigzagged table
    static unsigned int KodakQuantTable6x[8 * 8] =
        {
        10, 10, 10, 10, 10, 10, 10, 10,
        10, 10, 11, 10, 10, 10, 10, 11,
        10, 10, 10, 11, 13, 18, 14, 12,
        12, 11, 12, 15, 20, 16, 14, 14,
        15, 16, 20, 28, 29, 22, 20, 19,
        18, 18, 21, 25, 23, 25, 26, 28,
        33, 41, 37, 35, 32, 32, 44, 47,
        50, 54, 74, 68, 63, 94, 102,143
        };

    /* Old 1:6 non-zigzagged table
        static unsigned int KodakQuantTable6x[8 * 8] =
        {
            10,    10,    10,    10,    10,    11,    15,    20,
            10,    10,    10,    10,    10,    12,    16,    21,
            10,    10,    10,    10,    11,    14,    18,    25,
            10,    10,    10,    12,    14,    18,    23,    32,
            11,    11,    12,    15,    19,    25,    32,    44,
            13,    14,    16,    20,    26,    35,    47,    63,
            18,    20,    22,    28,    37,    50,    68,    94,
            28,    29,    33,    41,    54,    74,    102,143
        };
    */
    /* Old 1:8 non-zigzagged table
        static unsigned int KodakQuantTable8x[8 * 8] =
        {
            8,    8,    8,    12,    19,    31,    50,    83,
            8,    8,    11,    15,    23,    35,    57,    93,
            9,    11,    18,    25,    35,    50,    77,    124,
            13,    16,    26,    41,    59,    84,    126,196,
            21,    25,    38,    62,    99,    151,230,255,
            36,    41,    58,    94,    161,255,255,255,
            63,    71,    95,    150,255,255,255,255,
            115,127,166,255,255,255,255,255
        };
    */
    SetQuality(50);

    // luminance table
    SetQuantizationTable(0, KodakQuantTable6x);

    // chrominance table
    SetQuantizationTable(1, KodakQuantTable6x);

    // set the sampling mode
    SetSubsamplingMode(HCDCodecIJG::S411);
    }
