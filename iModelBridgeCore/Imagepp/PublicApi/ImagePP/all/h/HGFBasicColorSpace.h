//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGFBasicColorSpace.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Basic XYZ ColorSpace converter base class declaration
//-----------------------------------------------------------------------------

#pragma once

class HGFBasicColorSpace
    {
public:

    HGFBasicColorSpace(unsigned short pi_BitsPerPixel = 8);
    HGFBasicColorSpace(double pi_GammaCorrection, unsigned short pi_BitsPerPixel = 8);

    virtual ~HGFBasicColorSpace();

    // Gamma...
    void    UseGammaCorrection(double pi_GammaCorrection, bool pi_UseGammaFactor = true);
    bool   IsGammaCorrected();
    double GetGammaCorrectionFactor();

    // Conversion both side between RGB and CIE XYZ
    void ConvertToXYZ(unsigned short Red,  unsigned short Green, unsigned short Blue,
                      double* po_pX, double* po_Y,  double* po_pZ);

    void ConvertFromXYZ(double  pi_X,   double   pi_Y,     double   pi_Z,
                        Byte*  po_pRed,Byte*   po_pGreen, Byte*  po_pBlue);

    void ConvertFromXYZ(double  pi_X,   double   pi_Y,     double   pi_Z,
                        unsigned short* po_pRed,unsigned short*  po_pGreen, unsigned short* po_pBlue);

    // Color properties.
    void GetPureColor(double  pi_X, double  pi_Y, double pi_Z,
                      double* po_x, double* po_y);

    double Yellowness(double pi_Y, double pi_Z);
    double Whitness  (double pi_X, double pi_Y, double pi_Z);

    // Color difference evaluation
    _HDLLg double EuclideanColorDifference(double  pi_X1, double  pi_Y1, double  pi_Z1,
                                            double  pi_X2, double  pi_Y2, double  pi_Z2,
                                            double* po_dX, double* po_dY, double* po_dZ);

    _HDLLg double EuclideanColorDifference(double  pi_X1, double  pi_Y1, double  pi_Z1,
                                            double  pi_X2, double  pi_Y2, double  pi_Z2);

protected:
    // Initialize lookup table to speed up the convertion process
    void BuildLookupTableFromXYZ();
    void BuildLookupTableToXYZ();

    // ----------------------------------
    // RGB to XYZ conversion lookup table.
    // ----------------------------------
    // Red component lookup table
    double* m_pRGBToXRed;
    double* m_pRGBToYRed;
    double* m_pRGBToZRed;

    // Green component lookup table
    double* m_pRGBToXGreen;
    double* m_pRGBToYGreen;
    double* m_pRGBToZGreen;

    // Blue component lookup table
    double* m_pRGBToXBlue;
    double* m_pRGBToYBlue;
    double* m_pRGBToZBlue;

    // White point reference.
    double m_ReferenceWhiteXYZ[3];

    // ----------------------------------
    // XYZ to RGB conversion table.
    // ----------------------------------
    double m_RGBToXYZMatrix [3][3];

    double m_GammaCorrectionFactor;
    bool   m_UseGammaCorrection;

    unsigned short m_BitsPerPixel;
    unsigned short m_ChannelSize;

private:

    // Disable unused method to avoid unappropriate compiler initiative
    HGFBasicColorSpace(const HGFBasicColorSpace& pi_rSource);
    HGFBasicColorSpace& operator=(const HGFBasicColorSpace& pi_rSource);
    };

#include "HGFBasicColorSpace.hpp"
