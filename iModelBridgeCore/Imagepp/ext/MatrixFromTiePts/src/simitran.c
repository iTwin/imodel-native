/*--------------------------------------------------------------------------------------+
|
|     $Source: ext/MatrixFromTiePts/src/simitran.c $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
/*mh================================================================ HMR
**  Octimage/MicroStation - Transformation Models
**
**      This file contains the functions relative to the
**      transformation of coordinates using the similitude transform
**
**===========================================================================*/

#include "trfmodel.h"


/*fh=========================================================================
**
**  trf_transfoCoordSimi2D
**
**   ALAIN LAPIERRE         Version originale             21/12/92
**===========================================================================*/



void trf_transfoCoordSimi2D(int32_t            mapping,
                             pCOEFFICIENTS_3D   coef3D,
                             pDCOORD_3D         ptIn,
                             pDCOORD_3D         ptOut)
    {
    double yprod[2];
    trf_transfoCoordPrepSimi2D(mapping, coef3D, ptIn, yprod);
    trf_transfoCoordDoSimi2D(mapping, coef3D, ptIn, ptOut, yprod);
    }


/*fh=========================================================================
**
**  trf_transfoCoordPrepSimi2D
**
**   ALAIN LAPIERRE          Version originale            21/12/92
**===========================================================================*/

void trf_transfoCoordPrepSimi2D(int32_t            mapping,
                                 pCOEFFICIENTS_3D   coef3D,
                                 pDCOORD_3D         ptIn,
                                 double            *yprod)
    {
    double normY;

    normY = ptIn->y - coef3D->coef[mapping][YIND][0];

    yprod[0] = coef3D->coef[mapping][XIND][2] * normY;
    yprod[1] = coef3D->coef[mapping][YIND][2] * normY;
    }


/*fh=========================================================================
**
**  trf_transfoCoordDoSimi2D
**
**   ALAIN LAPIERRE         Version originale             21/12/92
**===========================================================================*/


void trf_transfoCoordDoSimi2D(int32_t          mapping,
                               pCOEFFICIENTS_3D coef3D,
                               pDCOORD_3D       ptIn,
                               pDCOORD_3D       ptOut,
                               double          *yprod)
    {
    double normX;
        int32_t mappingTran;

        mappingTran = (mapping == TRANSFO_DIRECT) ? TRANSFO_INVERSE : TRANSFO_DIRECT;
    normX = ptIn->x - coef3D->coef[mapping][XIND][0];

    ptOut->x=  coef3D->coef[mapping][XIND][1] * normX +
               yprod[0] +
               coef3D->coef[mapping][XIND][3] +
                           coef3D->coef[mappingTran][XIND][0];
    ptOut->y=  coef3D->coef[mapping][YIND][1] * normX +
               yprod[1] +
               coef3D->coef[mapping][YIND][3] +
                           coef3D->coef[mappingTran][YIND][0];
    ptOut->z = 0.0;
    }

/*fh=========================================================================
**
**  trf_transfoCoordDoSimiScanLine2D
**
**  ALAIN LAPIERRE          original version          Feb 97
**===========================================================================*/
void trf_transfoCoordDoSimiScanLine2D(int32_t           mapping,
                                                                                         pCOEFFICIENTS_3D       coef3D,
                                                                                         double                scanLineOriginX,
                                                                                         double                scanLinePixelSizeX,
                                                                                         int32_t                       scanLineLength,
                                                                                         pDCOORD_3D             ptOut,
                                                                                         double               *yprod)
{
    double normX, tempX, tempY;
    int32_t pt;
        int32_t mappingTran;

        mappingTran = (mapping == TRANSFO_DIRECT) ? TRANSFO_INVERSE : TRANSFO_DIRECT;
        tempX =    yprod[0] +
               coef3D->coef[mapping][XIND][3] +
                           coef3D->coef[mappingTran][XIND][0];
    tempY =    yprod[1] +
               coef3D->coef[mapping][YIND][3] +
                           coef3D->coef[mappingTran][YIND][0];

        for(pt=0; pt < scanLineLength; pt++, scanLineOriginX += scanLinePixelSizeX )
        {

            normX = scanLineOriginX - coef3D->coef[mapping][XIND][0];

        ptOut[pt].x =  coef3D->coef[mapping][XIND][1] * normX + tempX;
        ptOut[pt].y =  coef3D->coef[mapping][YIND][1] * normX + tempY;
    }
}

/*fh=========================================================================
**
**  trf_transfoDoSimiScanLine2D
**
**  Jean Papillon          original version          Dec 97
**===========================================================================*/
void trf_transfoDoSimiScanLine2D (int32_t                pi_Mapping,
                                   pCOEFFICIENTS_3D       pi_pCoef3D,
                                   double                pi_ScanLineOriginX,
                                   double                pi_ScanLinePixelSizeX,
                                   int32_t               pi_ScanLineLength,
                                   double               *pi_pYprod,
                                   TRANSFO_RSP_BUFF_INFO *pio_pBufferInfo,
                                   TRANSFO_RSP_COLOR_INFO *pi_pColorInfo)
{
    double TempX, TempY;
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
        int32_t MappingTran;
    unsigned char  Color;

    /*
    **  To reduce the number of address resolutions,
    **  initialize some simple variables with more
    **  complex ones
    */
    CoefX0 = pi_pCoef3D->coef[pi_Mapping][XIND][0];
    CoefX1 = pi_pCoef3D->coef[pi_Mapping][XIND][1] / pio_pBufferInfo->m_InputImagePixelSize;
    CoefY1 = pi_pCoef3D->coef[pi_Mapping][YIND][1] / pio_pBufferInfo->m_InputImagePixelSize;
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
    **  Compute some constant values for the line
    */
        MappingTran = (pi_Mapping == TRANSFO_DIRECT) ? TRANSFO_INVERSE : TRANSFO_DIRECT;
        TempX = (pi_pYprod[0] +
            pi_pCoef3D->coef[pi_Mapping][XIND][3] +
                        pi_pCoef3D->coef[MappingTran][XIND][0] -
            pio_pBufferInfo->m_InputImageOrigin.x) /
            pio_pBufferInfo->m_InputImagePixelSize;
    TempY = (pi_pYprod[1] +
            pi_pCoef3D->coef[pi_Mapping][YIND][3] +
                        pi_pCoef3D->coef[MappingTran][YIND][0] -
            pio_pBufferInfo->m_InputImageOrigin.y) /
            pio_pBufferInfo->m_InputImagePixelSize;

    /*
    **  Process pixels
    */
    OutputPixelCoordUserX = pi_ScanLineOriginX - CoefX0;
    pOutputBuffer = pio_pBufferInfo->m_pOutputBuffer;

        for(iPixel=0; iPixel < pi_ScanLineLength; iPixel++)
    {
        /*
        **  Compute input pixel coordinates in user units
        */
        InputPixelCoordUserX =  CoefX1 * OutputPixelCoordUserX + TempX;
        InputPixelCoordUserY =  CoefY1 * OutputPixelCoordUserX + TempY;

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
