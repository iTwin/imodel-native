//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGFLUVCube.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//---------------------------------------------------------------------------
// Class : HGFLUVSet
//---------------------------------------------------------------------------

#pragma once

#include "HGFColorSet.h"

BEGIN_IMAGEPP_NAMESPACE

class HGFLUVCube : public HGFColorSet
    {
    HDECLARE_CLASS_ID(HGFLUVCubeId, HGFColorSet)

public:
    static const double L_MIN;
    static const double L_MAX;

    static const double U_MIN;
    static const double U_MAX;

    static const double V_MIN;
    static const double V_MAX;


    HGFLUVCube();
    HGFLUVCube(double pi_Lmin, double pi_Lmax,
               double pi_Umin, double pi_Umax,
               double pi_Vmin, double pi_Vmax);
    HGFLUVCube(const HGFLUVCube& pi_rSrc);
    virtual         ~HGFLUVCube();

    HGFLUVCube& operator=(const HGFLUVCube& pi_rSrc);

    virtual bool   IsIn(Byte pi_R, Byte pi_G, Byte pi_B) const;

protected:

private:

    double         LimitedFastCubicRoot(double pi_Number) const;
    void            BuildLookupTable();
    void            ConvertRBGToXYZ(Byte pi_Red, Byte pi_Green, Byte pi_Blue,
                                    double*        pi_pX,double*       pi_pY,    double*       pi_pZ);

    double m_Lmin;
    double m_Lmax;
    double m_Umin;
    double m_Umax;
    double m_Vmin;
    double m_Vmax;

    // The static zone!
    static bool s_Initialized;

    // Coefficients for fast cubic root function
    static double s_C[24];

    // Red component lookup table
    static double s_RGBToXRed  [256];
    static double s_RGBToYRed  [256];
    static double s_RGBToZRed  [256];

    // Green component lookup table
    static double s_RGBToXGreen[256];
    static double s_RGBToYGreen[256];
    static double s_RGBToZGreen[256];

    // Blue component lookup table
    static double s_RGBToXBlue [256];
    static double s_RGBToYBlue [256];
    static double s_RGBToZBlue [256];

    // X Table Value multiplicated by 4
    static double s_XRed_x4  [256];
    static double s_XGreen_x4[256];
    static double s_XBlue_x4 [256];

    // Y Table Value multiplicated by 9
    static double s_YRed_x9  [256];
    static double s_YGreen_x9[256];
    static double s_YBlue_x9 [256];

    // X Table value added to
    // Y Table Value multiplicated by 15 added to
    // Z Table Value multiplicated by 3
    static double s_Red_XAnd15YAnd3Z   [256];
    static double s_Green__XAnd15YAnd3Z[256];
    static double s_Blue__XAnd15YAnd3Z [256];

    static double s_ReferenceWhiteXYZ[3];

    // U0 and V0 constant.
    static double s_UPrime;
    static double s_VPrime;
    };

END_IMAGEPP_NAMESPACE
#include "HGFLUVCube.hpp"

