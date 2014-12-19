//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGFLuvColorSpace.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// L*u*v* ColorSpace converter class declaration.
//-----------------------------------------------------------------------------

#pragma once

#include "HGFBasicColorSpace.h"

class HGFLuvColorSpace : public HGFBasicColorSpace
    {
public:
    _HDLLg HGFLuvColorSpace(unsigned short pi_BitsPerPixel = 8);
    _HDLLg HGFLuvColorSpace(double pi_GammaCorrection, unsigned short pi_BitsPerPixel = 8);
    _HDLLg virtual ~HGFLuvColorSpace();

    // Conversion both side between RGB and CIE LUV Pixel by Pixel
    _HDLLg void ConvertFromRGB (Byte   pi_Red, Byte   pi_Green, Byte   pi_Blue,
                                double* pi_pL,  double* pi_pU,    double* pi_pV) const;

    _HDLLg void ConvertFromRGB (unsigned short pi_Red, unsigned short pi_Green, unsigned short pi_Blue,
                                double* pi_pL,  double* pi_pU,    double* pi_pV) const;

    _HDLLg void ConvertToRGB (double  pi_L,    double  pi_U,      double  pi_V,
                              Byte*  po_pRed, Byte*  po_pGreen, Byte*  po_pBlue) const;

    _HDLLg void ConvertToRGB (double  pi_L,    double  pi_U,       double  pi_V,
                              unsigned short*  po_pRed,unsigned short*  po_pGreen, unsigned short*  po_pBlue) const;


    // These method are slower BUT take care of the RGB values when out of gammut.
    _HDLLg bool SafeConvertToRGB (double  pi_L,      double  pi_U,        double  pi_V,
                                  Byte*  po_pRed,   Byte*  po_pGreen,   Byte*  po_pBlue) const;

    _HDLLg bool SafeConvertToRGB (double   pi_L,    double   pi_U,      double   pi_V,
                                  unsigned short*  po_pRed, unsigned short*  po_pGreen, unsigned short*  po_pBlue) const;

    // Conversion both side between an array of RGB and and array of CIE LUV
    void ConvertArrayFromRGB (Byte*  pi_pRed, Byte*  pi_pGreen, Byte*  pi_pBlue,
                              double* po_pL,   double* po_pU,     double* po_pV,
                              uint32_t pi_ArraySize) const;

    void ConvertArrayToRGB (double* pi_L,    double* pi_U,      double* pi_V,
                            Byte*  po_pRed, Byte*  po_pGreen, Byte*  po_pBlue,
                            uint32_t pi_ArraySize) const;

    void GetChromaHueSaturation( double  pi_L,      double  pi_U,   double  pi_V,
                                 double* po_Chroma, double* po_Hue, double* po_Saturation) const;

#if 0
    // To be killed.
    // Used for test experimentation and debugging.
    void RetainMinMax(double  pi_L,  double pi_U,  double pi_V,
                      double* MinL, double* MaxL, double* MinU,
                      double* MaxU, double* MinV, double* MaxV);
#endif

protected:
    // Nothing here at this time.

private:
    // Precompted lookup table
    // X Table Value multiplicated by 4
    double* m_pXRed_x4;
    double* m_pXGreen_x4;
    double* m_pXBlue_x4;

    // Y Table Value multiplicated by 9
    double* m_pYRed_x9;
    double* m_pYGreen_x9;
    double* m_pYBlue_x9;

    // X Table value added to
    // Y Table Value multiplicated by 15 added to
    // Z Table Value multiplicated by 3
    double* m_pRed_XAnd15YAnd3Z;
    double* m_pGreen__XAnd15YAnd3Z;
    double* m_pBlue__XAnd15YAnd3Z;

    // U0 and V0 constant.
    double m_UPrime;
    double m_VPrime;

    // Initialize lookup table to speed up the convertion process
    void BuildLookupTableLUVToRGB();

    // Disable unused method to avoid unappropriate compiler initiative
    HGFLuvColorSpace(const HGFLuvColorSpace& pi_rSource);
    HGFLuvColorSpace& operator=(const HGFLuvColorSpace& pi_rSource);
    };

