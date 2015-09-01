//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hgf/src/HGFLUVCube.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//---------------------------------------------------------------------------
// Inline methods for class HGFLUVSet
//---------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HGFLUVCube.h>

const double HGFLUVCube::L_MIN = 0.0;
const double HGFLUVCube::L_MAX = 100.0;

const double HGFLUVCube::U_MIN = -134.0;
const double HGFLUVCube::U_MAX = 220.0;

const double HGFLUVCube::V_MIN = -140.0;
const double HGFLUVCube::V_MAX = 122.0;

double HGFLUVCube::s_C[24] = { 0.0015319394088521, -0.018843445653409,
                                0.10170534986,      -0.31702448761286,
                                0.63520892642253,   -0.88106985991189,
                                1.051750376454,      0.4267412323558,
                                1.507908365919e-5,  -3.7095709111375e-4,
                                0.0040043972242353, -0.024964114079723,
                                0.10003913718511,   -0.27751961573273,
                                0.66256121926465,    0.53766026150315,
                                1.4842542902609e-7, -7.3027601203435e-6,
                                1.5766326109233e-4, -0.0019658008013138,
                                0.015755176844105,  -0.0874132014051,
                                0.41738741349777,    0.6774094811598
                              };

bool HGFLUVCube::s_Initialized = false;

// Red component lookup table
double HGFLUVCube::s_RGBToXRed  [256];
double HGFLUVCube::s_RGBToYRed  [256];
double HGFLUVCube::s_RGBToZRed  [256];

// Green component lookup table
double HGFLUVCube::s_RGBToXGreen[256];
double HGFLUVCube::s_RGBToYGreen[256];
double HGFLUVCube::s_RGBToZGreen[256];

// Blue component lookup table
double HGFLUVCube::s_RGBToXBlue [256];
double HGFLUVCube::s_RGBToYBlue [256];
double HGFLUVCube::s_RGBToZBlue [256];

// X Table Value multiplicated by 4
double HGFLUVCube::s_XRed_x4  [256];
double HGFLUVCube::s_XGreen_x4[256];
double HGFLUVCube::s_XBlue_x4 [256];

// Y Table Value multiplicated by 9
double HGFLUVCube::s_YRed_x9  [256];
double HGFLUVCube::s_YGreen_x9[256];
double HGFLUVCube::s_YBlue_x9 [256];

// X Table value added to
// Y Table Value multiplicated by 15 added to
// Z Table Value multiplicated by 3
double HGFLUVCube::s_Red_XAnd15YAnd3Z   [256];
double HGFLUVCube::s_Green__XAnd15YAnd3Z[256];
double HGFLUVCube::s_Blue__XAnd15YAnd3Z [256];

double HGFLUVCube::s_ReferenceWhiteXYZ[3];

// U0 and V0 constant.
double HGFLUVCube::s_UPrime;
double HGFLUVCube::s_VPrime;


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
inline void HGFLUVCube::ConvertRBGToXYZ(Byte pi_Red, Byte pi_Green, Byte pi_Blue,
                                        double*       pi_pX, double*       pi_pY,    double*       pi_pZ)
    {
    *pi_pX = s_RGBToXRed[pi_Red] + s_RGBToXGreen[pi_Green] + s_RGBToXBlue[pi_Blue];
    *pi_pY = s_RGBToYRed[pi_Red] + s_RGBToYGreen[pi_Green] + s_RGBToYBlue[pi_Blue];
    *pi_pZ = s_RGBToZRed[pi_Red] + s_RGBToZGreen[pi_Green] + s_RGBToZBlue[pi_Blue];
    }

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
void HGFLUVCube::BuildLookupTable()
    {
    int    TableIndex;

    double NormalizedValue;

    // Given by Poynton's : RGB Standard to Rec 709 (D65 white point)
    double XYZToRBGMatrix [3][3] = { 0.412453, 0.357580, 0.180423,
                                      0.212671, 0.715160, 0.072169,
                                      0.019334, 0.119193, 0.950227
                                    };

    /* // Given by Poynton's : International Standard (D65) adopted for HDTV
    double XYZToRBGMatrix [3][3] = { 0.640, 0.300, 0.150,
                                     0.330, 0.600, 0.060,
                                     0.030, 0.100, 0.790};*/

    /* // Given by Alan Watt 3D Computer Graphics
    double XYZToRBGMatrix [3][3] = { 0.584, 0.188, 0.179,
                                     0.311, 0.614, 0.075,
                                     0.047, 0.103, 0.939};*/

    /*// Old NTSC Standard for 1953 phosphore into television.
    double XYZToRBGMatrix [3][3] = { 0.6070, 0.1740, 0.2000,
                                     0.2990, 0.5870, 0.1440,
                                     0.0000, 0.0660, 1.1120};*/

    for (TableIndex = 0; TableIndex < 256; TableIndex++)
        {
        // Divide TableIndex by 255.0 because valid RBG value sould be
        // normalize to fit whitn 0 to 1 range, instead of 0 to 255.
        NormalizedValue = (double)TableIndex / 255.0;

        s_RGBToXRed   [TableIndex] = NormalizedValue * XYZToRBGMatrix[0][0];
        s_RGBToXGreen [TableIndex] = NormalizedValue * XYZToRBGMatrix[0][1];
        s_RGBToXBlue  [TableIndex] = NormalizedValue * XYZToRBGMatrix[0][2];

        s_RGBToYRed   [TableIndex] = NormalizedValue * XYZToRBGMatrix[1][0];
        s_RGBToYGreen [TableIndex] = NormalizedValue * XYZToRBGMatrix[1][1];
        s_RGBToYBlue  [TableIndex] = NormalizedValue * XYZToRBGMatrix[1][2];

        s_RGBToZRed   [TableIndex] = NormalizedValue * XYZToRBGMatrix[2][0];
        s_RGBToZGreen [TableIndex] = NormalizedValue * XYZToRBGMatrix[2][1];
        s_RGBToZBlue  [TableIndex] = NormalizedValue * XYZToRBGMatrix[2][2];

        // To improve LUV conversion processing, build some more lookup table
        // built with pre-calculated constant value.

        // X Table Value multiplicated by 4
        s_XRed_x4  [TableIndex] = s_RGBToXRed  [TableIndex] * 4;
        s_XGreen_x4[TableIndex] = s_RGBToXGreen[TableIndex] * 4;
        s_XBlue_x4 [TableIndex] = s_RGBToXBlue [TableIndex] * 4;

        // Y Table Value multiplicated by 9
        s_YRed_x9  [TableIndex] = s_RGBToYRed  [TableIndex] * 9;
        s_YGreen_x9[TableIndex] = s_RGBToYGreen[TableIndex] * 9;
        s_YBlue_x9 [TableIndex] = s_RGBToYBlue [TableIndex] * 9;

        // X Table value added to
        // Y Table Value multiplicated by 15 added to
        // Z Table Value multiplicated by 3
        s_Red_XAnd15YAnd3Z   [TableIndex] = s_RGBToXRed  [TableIndex] + (15.0 * s_RGBToYRed  [TableIndex]) + (3.0 * s_RGBToZRed[TableIndex]);
        s_Green__XAnd15YAnd3Z[TableIndex] = s_RGBToXGreen[TableIndex] + (15.0 * s_RGBToYGreen[TableIndex]) + (3.0 * s_RGBToZGreen[TableIndex]);
        s_Blue__XAnd15YAnd3Z [TableIndex] = s_RGBToXBlue [TableIndex] + (15.0 * s_RGBToYBlue [TableIndex]) + (3.0 * s_RGBToZBlue[TableIndex]);
        }

    // Compute reference white from RGB(255, 255, 255) to XYZ
    ConvertRBGToXYZ(255, 255, 255, &s_ReferenceWhiteXYZ[0], &s_ReferenceWhiteXYZ[1], &s_ReferenceWhiteXYZ[2]);

    s_UPrime = (4.0 * s_ReferenceWhiteXYZ [0]) / (s_ReferenceWhiteXYZ [0] + ( 15.0 * s_ReferenceWhiteXYZ [1] ) + (3.0 * s_ReferenceWhiteXYZ [2]));
    s_VPrime = (9.0 * s_ReferenceWhiteXYZ [1])/  (s_ReferenceWhiteXYZ [0] + ( 15.0 * s_ReferenceWhiteXYZ [1] ) + (3.0 * s_ReferenceWhiteXYZ [2]));
    }

