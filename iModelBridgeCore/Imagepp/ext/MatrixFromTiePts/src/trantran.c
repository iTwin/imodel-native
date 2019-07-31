/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
/*mh================================================================ HMR
**  Octimage/MicroStation - Transformation Models
**
**      This file contains the functions relative to the
**      transformation of coordinates using the translation transform
**
**===========================================================================*/
#include "trfmodel.h"


/*fh=========================================================================
**
**  trf_transfoCoordTran2D
**
**    ALAIN LAPIERRE          Version originale            21/12/92
**===========================================================================*/

void trf_transfoCoordTran2D(int32_t            mapping,
                             pCOEFFICIENTS_3D   coef3D,
                             pDCOORD_3D         ptIn,
                             pDCOORD_3D         ptOut)
    {
    double yprod[1];
    trf_transfoCoordPrepTran2D(mapping, coef3D, ptIn, yprod);
    trf_transfoCoordDoTran2D(mapping, coef3D, ptIn, ptOut, yprod);
    }


/*fh=========================================================================
**
**  trf_transfoCoordPrepTran2D
**
**    ALAIN LAPIERRE         Version originale              21/12/92
**===========================================================================*/



void trf_transfoCoordPrepTran2D(int32_t            mapping,
                                 pCOEFFICIENTS_3D   coef3D,
                                 pDCOORD_3D         ptIn,
                                 double            *yprod)
     {
            yprod[0] = coef3D->coef[mapping][YIND][0] + ptIn->y;
     }


/*fh=========================================================================
**
**  trf_transfoCoordDoTran2D
**
**    ALAIN LAPIERRE         Version originale              21/12/92
**===========================================================================*/



void trf_transfoCoordDoTran2D(int32_t          mapping,
                               pCOEFFICIENTS_3D coef3D,
                               pDCOORD_3D       ptIn,
                               pDCOORD_3D       ptOut,
                               double          *yprod)
    {
    ptOut->x=  coef3D->coef[mapping][XIND][0] + ptIn->x;
    ptOut->y=  yprod[0];
    ptOut->z = 0.0;
    }

/*&& AL Added transfoCoord for a Scan line */

/*fh=========================================================================
**
**  trf_transfoCoordDoTranScanLine2D
**
**  ALAIN LAPIERRE          original version          Feb 97
**===========================================================================*/
void trf_transfoCoordDoTranScanLine2D(int32_t           mapping,
                                                                                         pCOEFFICIENTS_3D       coef3D,
                                                                                         double                scanLineOriginX,
                                                                                         double                scanLinePixelSizeX,
                                                                                         int32_t                       scanLineLength,
                                                                                         pDCOORD_3D             ptOut,
                                                                                         double               *yprod)
{
    int32_t pt;

        for(pt=0; pt < scanLineLength; pt++, scanLineOriginX += scanLinePixelSizeX )
        {
                ptOut[pt].x=  coef3D->coef[mapping][XIND][0] + scanLineOriginX;
                ptOut[pt].y=  yprod[0];
        }
}

/*&&end Added transfoCoord for a Scan line*/


/*fh=========================================================================
**
**  trf_transfoDoTranScanLine2D
**
**  Jean Papillon          original version          Dec 97
**===========================================================================*/
void trf_transfoDoTranScanLine2D (int32_t                pi_Mapping,
                                   pCOEFFICIENTS_3D       pi_pCoef3D,
                                   double                pi_ScanLineOriginX,
                                   double                pi_ScanLinePixelSizeX,
                                   int32_t               pi_ScanLineLength,
                                   double               *pi_pYprod,
                                   TRANSFO_RSP_BUFF_INFO *pio_pBufferInfo,
                                   TRANSFO_RSP_COLOR_INFO *pi_pColorInfo)
{
    double CoefX0;
    double CoefX1;
    double CoefY1;
    double OutputPixelCoordUserX;
    double InputPixelCoordUserX;
    double InputPixelCoordUserY;
    int32_t InputPixelCoordX;
    int32_t InputPixelCoordY;
    int32_t InputBitmapSizeX;
    int32_t InputBitmapSizeY;
    int32_t StartInputBitmapPixelX;
    int32_t StartInputBitmapPixelY;
    int32_t EndInputBitmapPixelX;
    int32_t EndInputBitmapPixelY;
    int32_t ImageDimensionX;
    int32_t ImageDimensionY;
    unsigned char  *pOutputBuffer;
    unsigned char  *pInputBuffer;
    int32_t iPixel;
    unsigned char  Color;

    /*
    **  To reduce the number of address resolutions,
    **  initialize some simple variables with more
    **  complex ones
    */
    CoefX0 = pi_pCoef3D->coef[pi_Mapping][XIND][0];
/*
    CoefX0 = pi_pCoef3D->coef[pi_Mapping][XIND][0] - pio_pBufferInfo->m_InputImageOrigin.x;
    InputPixelCoordUserY = (pi_pYprod [0] - pio_pBufferInfo->m_InputImageOrigin.y) /
                            pio_pBufferInfo->m_InputImagePixelSize;
*/
    CoefX1 = pi_pCoef3D->coef[pi_Mapping][XIND][1];
    CoefY1 = pi_pCoef3D->coef[pi_Mapping][YIND][1];
    ImageDimensionX = pio_pBufferInfo->m_ImageDimension.x;
    ImageDimensionY = pio_pBufferInfo->m_ImageDimension.y;
    InputBitmapSizeX = pio_pBufferInfo->m_InputBufferSize.x;
    InputBitmapSizeY = pio_pBufferInfo->m_InputBufferSize.y;
    StartInputBitmapPixelX = pio_pBufferInfo->m_InputBufferOrigin.x;
    StartInputBitmapPixelY = pio_pBufferInfo->m_InputBufferOrigin.y;
    EndInputBitmapPixelX = StartInputBitmapPixelX + InputBitmapSizeX;
    EndInputBitmapPixelY = StartInputBitmapPixelY + InputBitmapSizeY;
    pInputBuffer = pio_pBufferInfo->m_pInputBuffer;

    /*
    **  Process pixels
    */
    OutputPixelCoordUserX = pi_ScanLineOriginX;
    pOutputBuffer = pio_pBufferInfo->m_pOutputBuffer;

        for(iPixel=0; iPixel < pi_ScanLineLength; iPixel++)
    {
        /*
        **  Compute input pixel coordinates in user units
        */
        InputPixelCoordUserX =  CoefX0 + OutputPixelCoordUserX;
/*
*/
        InputPixelCoordUserY =  pi_pYprod [0];
/*
        InputPixelCoordUserX =  (CoefX0 + OutputPixelCoordUserX) /
                            pio_pBufferInfo->m_InputImagePixelSize;
*/
        /*
        **  Assign default value if pixel is outside the input image
        */
        if (InputPixelCoordUserX < 0.0 ||
            InputPixelCoordUserY < 0.0)
        {
            *pOutputBuffer = pi_pColorInfo->m_DefaultPixelValue;
                }
        else
        {
                InputPixelCoordX = (int32_t) InputPixelCoordUserX;
            InputPixelCoordY = (int32_t) InputPixelCoordUserY;

                if ((InputPixelCoordX >= ImageDimensionX) ||
                    (InputPixelCoordY >= ImageDimensionY) )
            {
                *pOutputBuffer = pi_pColorInfo->m_DefaultPixelValue;
            }
            else
            {
                /*
                **  Identify the output pixel value
                **  Assign it directly is we are allowed to and if
                **  we the reference pixel is into the received input buffer
                **
                **  Use provided callback function otherwise
                */
                if (pi_pColorInfo->m_ProcessPixel == TRUE &&
                                   pInputBuffer != NULL &&
                       InputPixelCoordX >= StartInputBitmapPixelX &&
                       InputPixelCoordY >= StartInputBitmapPixelY &&
                       InputPixelCoordX <= EndInputBitmapPixelX &&
                       InputPixelCoordY <= EndInputBitmapPixelY)
                {
                        Color = pInputBuffer[(InputBitmapSizeY - (InputPixelCoordY - StartInputBitmapPixelY)) * InputBitmapSizeX +
                                             (InputPixelCoordX - StartInputBitmapPixelX)];
                }
                else
                {
                    DCOORD_3D InputPixelCoordUser;
                    LCOORD_2D InputPixelCoord;

                    InputPixelCoordUser.x = InputPixelCoordUserX;
                    InputPixelCoordUser.y = InputPixelCoordUserY;
                    InputPixelCoord.x = InputPixelCoordX;
                    InputPixelCoord.y = InputPixelCoordY;

                    Color = pi_pColorInfo->m_pPixelRspFunction (&InputPixelCoordUser,
                                           &InputPixelCoord,
                                           pi_pColorInfo->m_pPixelRspFunctionParam);
                }

                /*
                **  Check resulting color and change it if needed
                */
                   /*
                   ** Restrict the pixel value to the min-max range
                   ** when a resampling occured. (Not for the background pixel
                   ** which should be out of this range to avoid conflicts
                   ** beetween the background and the image)
                   */

                    if(!(pi_pColorInfo->m_UntouchedColorStatus &&
                         pi_pColorInfo->m_UntouchedColor == Color))
                    {
                   /*
                   ** We must check if the color is transparent.
                   ** Transparent color must be set to background color
                   ** and not min-max color else the transparency would be lost
                   */
                       if(pi_pColorInfo->m_TransparentInputColors)
                       {
                         if (pi_pColorInfo->m_pIsTransparentColor[Color])
                                 Color = pi_pColorInfo->m_DefaultPixelValue;
                       }

                       if((int32_t) Color > pi_pColorInfo->m_MaxColor)
                           Color = (unsigned char)pi_pColorInfo->m_MaxColor;
                       else if((int32_t) Color < pi_pColorInfo->m_MinColor)
                           Color = (unsigned char)pi_pColorInfo->m_MinColor;

                    }

                   /*
                   **  Remap the color of the input pixel
                   */
                   if(pi_pColorInfo->m_MapInputColors)
                       Color = pi_pColorInfo->m_pColorMapping [Color];

                *pOutputBuffer = Color;
            }
        }

        OutputPixelCoordUserX += pi_ScanLinePixelSizeX;
        pOutputBuffer++;
    }
}
