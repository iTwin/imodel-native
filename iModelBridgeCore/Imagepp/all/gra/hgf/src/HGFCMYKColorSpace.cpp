//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hgf/src/HGFCMYKColorSpace.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HGFCMYKColorSpace
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HGFCMYKColorSpace.h>

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------

HGFCMYKColorSpace::HGFCMYKColorSpace()
    {
    // Nothing to do here at this time.
    m_InvertedMode = false;
    }

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------

HGFCMYKColorSpace::HGFCMYKColorSpace(bool pi_InvertedMode)
    {
    // m_InvertedMode :
    //           It appears that Adobe Photoshop writes inverted
    //           data in CMYK JPEG files: 0 represents 100% ink coverage
    //           rather than 0% ink as you'd expect.
    //
    //           This is arguably a bug in Photoshop, but if you need to work
    //           with Photoshop CMYK files, you will have to deal with it in
    //           your application.  We cannot "fix" this in the library by
    //           inverting because that would break other applications.

    m_InvertedMode = pi_InvertedMode;
    }

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------

HGFCMYKColorSpace::~HGFCMYKColorSpace()
    {
    // Nothing to do here at this time.
    }

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------

inline void HGFCMYKColorSpace::ConvertToRGB (Byte pi_Cyan, Byte pi_Magenta, Byte pi_Yellow, Byte pi_Black,
                                             Byte* po_pRed, Byte* po_pGreen,  Byte* po_pBlue)
    {
    // Functionnal but the not appropriate for imaging application
    // should be use only for "pie chart" color conversion
    unsigned short Black;

    Byte Red;
    Byte Green;
    Byte Blue;

    if (m_InvertedMode)
        {
        Black = pi_Black;

        Red   = (Byte)((Black * pi_Cyan   ) / 255);
        Green = (Byte)((Black * pi_Magenta) / 255);
        Blue  = (Byte)((Black * pi_Yellow ) / 255);
        }
    else
        {
        Black = (255 - pi_Black);

        Red   = (Byte)((Black * (255 - pi_Cyan)   ) / 255);
        Green = (Byte)((Black * (255 - pi_Magenta)) / 255);
        Blue  = (Byte)((Black * (255 - pi_Yellow )) / 255);
        }

    // Temporay code to test adjustemnent matrix wich will reduce conversion error
    // This is really not accurate, to get correct color conversion, we must use a
    // non linear mathematical function.  Maybe pass trough the the XYZ color space..

    double AdjustmentMatrix[3][3] = { 0.85 , 0.10 , 0.05,
                                      0.20 , 0.70 , 0.10,
                                      0.05,  0.15,  0.80
                                    };
    /*
    // Keep for debugging purpose, will be remove soon...
    double AdjustmentMatrix[3][3] = { 1.0, 0.0, 0.0,
                                      0.0, 1.0, 0.0,
                                      0.0, 0.0, 1.0};*/

    *po_pRed   = (Byte)((AdjustmentMatrix[0][0] * Red) + (AdjustmentMatrix[0][1] * Green) + (AdjustmentMatrix[0][2] * Blue));
    *po_pGreen = (Byte)((AdjustmentMatrix[1][0] * Red) + (AdjustmentMatrix[1][1] * Green) + (AdjustmentMatrix[1][2] * Blue));
    *po_pBlue  = (Byte)((AdjustmentMatrix[2][0] * Red) + (AdjustmentMatrix[2][1] * Green) + (AdjustmentMatrix[2][2] * Blue));
    }

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------

void HGFCMYKColorSpace::ConvertArrayToRGB (Byte const* pi_pCyan, Byte const* pi_pMagenta, Byte const* pi_pYellow, Byte const* pi_pBlack,
                                           Byte* po_pRed,  Byte* po_pGreen,   Byte* po_pBlue,
                                           unsigned long pi_ArraySize)
    {
    // Not yet implement.
    HPRECONDITION(false);

    for (unsigned long ArrayIndex = 0; ArrayIndex < pi_ArraySize; ArrayIndex++)
        {
        ++po_pRed;
        ++po_pGreen;
        ++po_pBlue;
        }
    }

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------

inline void HGFCMYKColorSpace::ConvertFromRGB (Byte pi_Red,   Byte pi_Green,    Byte pi_Blue,
                                               Byte* pi_pCyan, Byte* pi_pMagenta, Byte* pi_pYellow, Byte* pi_pBlack)
    {
    /*
    double AdjustmentMatrix[3][3] = { 1.206100, -0.16045, -0.055327,
                                     -0.343030,  1.51350, -0.167750,
                                     -0.011065, -0.27376,  1.284900 };*/

    double AdjustmentMatrix[3][3] = { 1.0, 0.0, 0.0,
                                      0.0, 1.0, 0.0,
                                      0.0, 0.0, 1.0
                                    };

    Byte Red   = (Byte)((AdjustmentMatrix[0][0] * pi_Red) + (AdjustmentMatrix[0][1] * pi_Green) + (AdjustmentMatrix[0][2] * pi_Blue));
    Byte Green = (Byte)((AdjustmentMatrix[1][0] * pi_Red) + (AdjustmentMatrix[1][1] * pi_Green) + (AdjustmentMatrix[1][2] * pi_Blue));
    Byte Blue  = (Byte)((AdjustmentMatrix[2][0] * pi_Red) + (AdjustmentMatrix[2][1] * pi_Green) + (AdjustmentMatrix[2][2] * pi_Blue));

    Byte Cyan    = 255 - Red;
    Byte Magenta = 255 - Green;
    Byte Yellow  = 255 - Blue;

    if (m_InvertedMode)
        {
        //This code, used for writing CMYK JPEG doesn't work
        HASSERT(0);
        Cyan    = Red;
        Magenta = Green;
        Yellow  = Blue;
        }

    // Convert CMY to CMYK
    // Extract the black value
    *pi_pBlack   = MIN(MIN(Cyan, Magenta), Yellow);

    *pi_pCyan    = (Cyan  - *pi_pBlack);
    *pi_pMagenta = (Magenta - *pi_pBlack); // (Magenta  * Divider) * 255;
    *pi_pYellow  = (Yellow  - *pi_pBlack); // (Yellow   * Divider) * 255;
    }

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------

void HGFCMYKColorSpace::ConvertArrayFromRGB (Byte const* pi_pRed, Byte const* pi_pGreen, Byte const* pi_pBlue,
                                             Byte* pi_pCyan, Byte* pi_pMagenta, Byte* pi_pYellow, Byte* pi_pBlack,
                                             unsigned long  pi_ArraySize)
    {
    // Not yet implement.
    HPRECONDITION(false);
    }

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------

void HGFCMYKColorSpace::ConvertArrayFromRGB (Byte const* pi_pRBGData,
                                             Byte* po_pCMYKData,
                                             unsigned long  pi_ArraySizeInPixel)
    {
    // For optimization, the user may use the same buffer for conversion,
    // To do allow this, we have just convert the buffer from the end to the begining.
    // Important Note : it should be large enough to hold CMYK data.

    // The input buffer format must be : R1, G1, B1, R2, G2, B2, .... Rn, Gn, Bn
    // And
    // The output buffer format must be : C1, M1, Y1, K1, C2, M2, Y2, K2, ...., Cn, Mn, Yn, Kn
    // When each color compoment as the size of a Byte.

    // Because at this time of developpment we are not sure of the conversion algo,
    // We do single color funtion call.  But for efficiency, we MUST recode
    // the conversion here to improve performance.

    unsigned int RGBBufferOffset  = 0;
    unsigned int CMYKBufferOffset = 0;

    while(pi_ArraySizeInPixel)
        {
        --pi_ArraySizeInPixel;

        CMYKBufferOffset = pi_ArraySizeInPixel * 4;
        RGBBufferOffset  = pi_ArraySizeInPixel * 3;

        ConvertFromRGB (*(pi_pRBGData  + RGBBufferOffset     ),  // Red in
                        *(pi_pRBGData  + RGBBufferOffset  + 1),  // Green in
                        *(pi_pRBGData  + RGBBufferOffset  + 2),  // Blue in
                        po_pCMYKData + CMYKBufferOffset    ,   // Cyan out
                        po_pCMYKData + CMYKBufferOffset + 1,   // Magenta out
                        po_pCMYKData + CMYKBufferOffset + 2,   // Yellow out
                        po_pCMYKData + CMYKBufferOffset + 3);  // Black out
        }
    }

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------

void HGFCMYKColorSpace::ConvertArrayToRGB (Byte const* pi_pCMYKData,
                                           Byte* po_pRBGData,
                                           unsigned long  pi_ArraySizeInPixel)
    {
    // The input buffer format must be : C1, M1, Y1, K1, C2, M2, Y2, K2, ...., Cn, Mn, Yn, Kn
    // And
    // The output buffer format must be : R1, G1, B1, R2, G2, B2, .... Rn, Gn, Bn
    // When each color compoment as the size of a Byte.

    // Because at this time of developpment we are not sure of the conversion algo,
    // We do single color funtion call.  But for efficiency, we MUST recode
    // the conversion here to improve performance.

    unsigned int RGBBufferOffset  = 0;
    unsigned int CMYKBufferOffset = 0;

    unsigned long Iterator = 0;

    for (Iterator = 0; Iterator < pi_ArraySizeInPixel; Iterator++)
        {
        // 4 is the size in byte of a single element into the CMYK color space.
        CMYKBufferOffset = Iterator * 4;

        // 3 is the size in byte of a single element into the RGB color space.
        RGBBufferOffset  = Iterator * 3;

        ConvertToRGB (*(pi_pCMYKData + CMYKBufferOffset  + 0),  // Cyan in
                      *(pi_pCMYKData + CMYKBufferOffset  + 1),  // Magenta in
                      *(pi_pCMYKData + CMYKBufferOffset  + 2),  // Yellow in
                      *(pi_pCMYKData + CMYKBufferOffset  + 3),  // Black in
                      po_pRBGData + RGBBufferOffset + 0,         // Red out
                      po_pRBGData + RGBBufferOffset + 1,        // Green out
                      po_pRBGData + RGBBufferOffset + 2);       // Blue out
        }
    }
