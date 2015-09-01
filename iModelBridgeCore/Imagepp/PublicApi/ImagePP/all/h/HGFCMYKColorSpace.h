//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGFCMYKColorSpace.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// CMYK  ColorSpace converter class declaration.
//-----------------------------------------------------------------------------

#include "HGFBasicColorSpace.h"

BEGIN_IMAGEPP_NAMESPACE

class HGFCMYKColorSpace : public HGFBasicColorSpace
    {
public:
    HGFCMYKColorSpace();
    HGFCMYKColorSpace(bool pi_InvertedMode);
    virtual ~HGFCMYKColorSpace();

    // Conversion both side between RGB and CMYK Pixel by Pixel
    void ConvertFromRGB (Byte pi_Red,   Byte pi_Green,    Byte pi_Blue,
                         Byte* pi_pCyan, Byte* pi_pMagenta, Byte* pi_pYellow, Byte* pi_pBlack);

    void ConvertToRGB (Byte pi_Cyan, Byte pi_Magenta, Byte pi_Yellow, Byte pi_Black,
                       Byte* po_pRed, Byte* po_pGreen,  Byte* po_pBlue);

    // Conversion both side between an array of RGB and and array of CMYK
    void ConvertArrayFromRGB (Byte const* pi_pRed, Byte const* pi_pGreen, Byte const* pi_pBlue,
                              Byte* pi_pCyan, Byte* pi_pMagenta, Byte* pi_pYellow, Byte* pi_pBlack,
                              unsigned long  pi_ArraySize);

    void ConvertArrayToRGB (Byte const* pi_pCyan, Byte const* pi_pMagenta, Byte const* pi_pYellow, Byte const* pi_pBlack,
                            Byte* po_pRed,  Byte* po_pGreen,   Byte* po_pBlue,
                            unsigned long pi_ArraySize);

    void ConvertArrayFromRGB (Byte const* pi_pRBGData, Byte* po_pCMYKData, unsigned long  pi_ArraySizeInPixel);
    void ConvertArrayToRGB   (Byte const* pi_pCMYKData, Byte* po_pRBGData, unsigned long  pi_ArraySizeInPixel);

protected:

private:
    // Disable unused method to avoid unappropriate compiler initiative
    HGFCMYKColorSpace(const HGFCMYKColorSpace& pi_rSource);
    HGFCMYKColorSpace& operator=(const HGFCMYKColorSpace& pi_rSource);

    bool m_InvertedMode;
    };

END_IMAGEPP_NAMESPACE