/*--------------------------------------------------------------------------------------+
|
|     $Source: ext/MatrixFromTiePts/src/mattran.c $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
/*mh================================================================ HMR
**  Octimage/MicroStation - Transformation Models
**
**      This file contains the functions relative to the
**      transformation of coordinates using the affine transform
**
**===========================================================================*/
#include <string.h>

#include "trfmodel.h"

/*
**  Private definitions
*/
#define TRANSFOMAT_HORIZONTAL               0x01
#define TRANSFOMAT_VERTICAL                 0x02
#define TRANSFOMAT_NO_MAPPING               0x04
#define TRANSFOMAT_NO_MIN_MAX               0x08
#define TRANSFOMAT_INPUT_COMPLETE           0x10

/*
**  Static functions prototypes
*/
static HSTATUS  s_IdentifySignificative (double    pi_FirstPixelX,
                                         double    pi_FirstPixelY,
                                         double    pi_LastPixelX,
                                         double    pi_LastPixelY,
                                         double    pi_ImageDimensionX,
                                         double    pi_ImageDimensionY,
                                         double   *po_pFirstSignificativeX,
                                         double   *po_pFirstSignificativeY,
                                         double   *po_pLastSignificativeX,
                                         double   *po_pLastSignificativeY,
                                         int8_t   *po_pSignifactiveExist);

/*fh=========================================================================
**
**  trf_transfoCoordMat2D
**
**  Jean Papillon          Version originale        97/11/07
**===========================================================================*/

void trf_transfoCoordMat2D (int32_t                mapping,
                             pCOEFFICIENTS_3D   coef3D,
                             pDCOORD_3D         ptIn,
                             pDCOORD_3D         ptOut)
{
    double yprod[2];
    trf_transfoCoordPrepMat2D(mapping, coef3D, ptIn, yprod);
    trf_transfoCoordDoMat2D(mapping, coef3D, ptIn, ptOut, yprod);
}


/*fh=========================================================================
**
**  trf_transfoCoordPrepMat2D
**
**  Jean Papillon          Version originale        97/11/07
**===========================================================================*/

void trf_transfoCoordPrepMat2D (int32_t            mapping,
                                 pCOEFFICIENTS_3D   coef3D,
                                 pDCOORD_3D         ptIn,
                                 double           *yprod)
{
    yprod[0] = coef3D->coef[mapping][XIND][2] * ptIn->y;
    yprod[1] = coef3D->coef[mapping][YIND][2] * ptIn->y;
}



/*fh=========================================================================
**
**  trf_transfoCoordDoMat2D
**
**  Jean Papillon          Version originale        97/11/07
**===========================================================================*/



void trf_transfoCoordDoMat2D  (int32_t             mapping,
                                pCOEFFICIENTS_3D    coef3D,
                                pDCOORD_3D          ptIn,
                                pDCOORD_3D          ptOut,
                                double            *yprod)
    {
    ptOut->x =        coef3D->coef[mapping][XIND][1] * ptIn->x +
                      yprod[0] +
                                          coef3D->coef[mapping][XIND][0];

    ptOut->y =        coef3D->coef[mapping][YIND][1] * ptIn->x +
                      yprod[1] +
                                          coef3D->coef[mapping][YIND][0];
    ptOut->z = 0.0;
    }

/*fh=========================================================================
**
**  trf_transfoCoordDoMatScanLine2D
**
**  Jean Papillon          Version originale        97/11/07
**===========================================================================*/
void trf_transfoCoordDoMatScanLine2D(int32_t           mapping,
                                                                                         pCOEFFICIENTS_3D       coef3D,
                                                                                         double                scanLineOriginX,
                                                                                         double                scanLinePixelSizeX,
                                                                                         int32_t                       scanLineLength,
                                                                                         pDCOORD_3D             ptOut,
                                                                                         double               *yprod)
{
    double tempX, tempY;
    int32_t pt;

        tempX = yprod[0] +
                        coef3D->coef[mapping][XIND][0];
    tempY = yprod[1] +
                        coef3D->coef[mapping][YIND][0];

        for(pt=0; pt < scanLineLength; pt++, scanLineOriginX += scanLinePixelSizeX )
        {
        ptOut[pt].x=  coef3D->coef[mapping][XIND][1] * scanLineOriginX + tempX;
        ptOut[pt].y=  coef3D->coef[mapping][YIND][1] * scanLineOriginX + tempY;
        }
}

/*fh=========================================================================
**
**  trf_transfoDoMatScanLine2D
**
**  Jean Papillon          original version          Dec 97
**===========================================================================*/
void trf_transfoDoMatScanLine2D (int32_t                pi_Mapping,
                                  pCOEFFICIENTS_3D       pi_pCoef3D,
                                  double                pi_ScanLineOriginX,
                                  double                pi_ScanLinePixelSizeX,
                                  int32_t               pi_ScanLineLength,
                                  double               *pi_pYprod,
                                  TRANSFO_RSP_BUFF_INFO  *pio_pBufferInfo,
                                  TRANSFO_RSP_COLOR_INFO *pi_pColorInfo)
{
    double InputPixelCoordUserX;
    double InputPixelCoordUserY;
    double FirstPixelX;
    double FirstPixelY;
    double LastPixelX;
    double LastPixelY;
    double FirstSignificativePixelX;
    double FirstSignificativePixelY;
    double LastSignificativePixelX;
    double LastSignificativePixelY;
    int8_t SignificativeExist;
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
    int32_t BaseIndex;
    double IncrementX;
    double IncrementY;
    int32_t OptimizationStatus;

    /*
    **  To reduce the number of address resolutions,
    **  initialize some simple variables with more
    **  complex ones
    */
    ImageDimensionX = pio_pBufferInfo->m_ImageDimension.x;
    ImageDimensionY = pio_pBufferInfo->m_ImageDimension.y;
    InputBitmapSizeX = pio_pBufferInfo->m_InputBufferSize.x;
    InputBitmapSizeY = pio_pBufferInfo->m_InputBufferSize.y;
    StartInputBitmapPixelX = pio_pBufferInfo->m_InputBufferOrigin.x;
    StartInputBitmapPixelY = pio_pBufferInfo->m_InputBufferOrigin.y;
    EndInputBitmapPixelX = StartInputBitmapPixelX + InputBitmapSizeX - 1;
    EndInputBitmapPixelY = StartInputBitmapPixelY + InputBitmapSizeY - 1;
    pInputBuffer = pio_pBufferInfo->m_pInputBuffer;

    BaseIndex = (InputBitmapSizeY - 1 + StartInputBitmapPixelY) * InputBitmapSizeX - StartInputBitmapPixelX;
    IncrementX = pi_pCoef3D->coef[pi_Mapping][XIND][1] * pi_ScanLinePixelSizeX / pio_pBufferInfo->m_InputImagePixelSize;
    IncrementY = pi_pCoef3D->coef[pi_Mapping][YIND][1] * pi_ScanLinePixelSizeX / pio_pBufferInfo->m_InputImagePixelSize;

    /*
    **  Compute first input pixel coordinates in user units
    */
    FirstPixelX = (pi_pCoef3D->coef[pi_Mapping][XIND][1] * pi_ScanLineOriginX +
        pi_pYprod[0] + pi_pCoef3D->coef[pi_Mapping][XIND][0] -
        pio_pBufferInfo->m_InputImageOrigin.x) /
        pio_pBufferInfo->m_InputImagePixelSize;
    FirstPixelY = (pi_pCoef3D->coef[pi_Mapping][YIND][1] * pi_ScanLineOriginX +
        pi_pYprod[1] + pi_pCoef3D->coef[pi_Mapping][YIND][0] -
        pio_pBufferInfo->m_InputImageOrigin.y) /
        pio_pBufferInfo->m_InputImagePixelSize;

    LastPixelX = FirstPixelX + (pi_ScanLineLength - 1) * IncrementX;
    LastPixelY = FirstPixelY + (pi_ScanLineLength - 1) * IncrementY;

    /*
    **  Do some analysis in order to use the best optimized function
    */
    OptimizationStatus = 0;

    /*
    **  All needed input pixels are in the provided input bitmap
    **  By the way, check if there is at least one input pixel needed.
    **  Initilize the output buffer to background color if no input
    **  oixel is needed
    */

    s_IdentifySignificative (FirstPixelX, FirstPixelY, LastPixelX, LastPixelY,
                             ImageDimensionX, ImageDimensionY,
                             &FirstSignificativePixelX, &FirstSignificativePixelY,
                             &LastSignificativePixelX, &LastSignificativePixelY,
                             &SignificativeExist);

    if (SignificativeExist == FALSE)
    {
        memset (pio_pBufferInfo->m_pOutputBuffer, pi_pColorInfo->m_DefaultPixelValue, pi_ScanLineLength);
        goto WRAPUP;
    }

    if (pi_pColorInfo->m_ProcessPixel == TRUE && pInputBuffer != NULL &&
        FirstSignificativePixelX >= StartInputBitmapPixelX && FirstSignificativePixelX <= EndInputBitmapPixelX &&
        FirstSignificativePixelY >= StartInputBitmapPixelY && FirstSignificativePixelY <= EndInputBitmapPixelY &&
        LastSignificativePixelX >= StartInputBitmapPixelX && LastSignificativePixelX <= EndInputBitmapPixelX &&
        LastSignificativePixelY >= StartInputBitmapPixelY && LastSignificativePixelY <= EndInputBitmapPixelY)
    {
        OptimizationStatus |= TRANSFOMAT_INPUT_COMPLETE;
    }

    /*
    **  All input pixels on a horizontal line
    */
    if (fabs(LastPixelY - FirstPixelY) <= 1.0)
    {
        OptimizationStatus |= TRANSFOMAT_HORIZONTAL;
    }

    /*
    **  All input pixels on a vertical line
    */
    if (fabs(LastPixelX - FirstPixelX) <= 1.0)
    {
        OptimizationStatus |= TRANSFOMAT_VERTICAL;
    }

    /*
    **  Color mapping needed
    */
    if (pi_pColorInfo->m_MapInputColors != TRUE)
    {
        OptimizationStatus |= TRANSFOMAT_NO_MAPPING;
    }

    /*
    **  Color range to support
    */
    if (pi_pColorInfo->m_MinColor == 0 && pi_pColorInfo->m_MaxColor > 254)
    {
        OptimizationStatus |= TRANSFOMAT_NO_MIN_MAX;
    }

    /*
    **  Process pixels
    */
    InputPixelCoordUserX = FirstPixelX;
    InputPixelCoordUserY = FirstPixelY;
    pOutputBuffer = pio_pBufferInfo->m_pOutputBuffer;

    switch (OptimizationStatus)
    {
    case TRANSFOMAT_INPUT_COMPLETE | TRANSFOMAT_HORIZONTAL | TRANSFOMAT_NO_MAPPING | TRANSFOMAT_NO_MIN_MAX:

        InputPixelCoordY = (int32_t) InputPixelCoordUserY;
        BaseIndex -= (InputPixelCoordY * InputBitmapSizeX);

            for(iPixel=0; iPixel < pi_ScanLineLength; iPixel++)
        {
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

                    if ((InputPixelCoordX >= ImageDimensionX) ||
                        (InputPixelCoordY >= ImageDimensionY) )
                {
                    *pOutputBuffer = pi_pColorInfo->m_DefaultPixelValue;
                }
                else
                {
                        Color = pInputBuffer[BaseIndex + InputPixelCoordX];
                    *pOutputBuffer = Color;
                }
            }

            /*
            **  Compute next input pixel coordinates in user units
            */
            InputPixelCoordUserX +=  IncrementX;

            pOutputBuffer++;
        }
        break;


    case TRANSFOMAT_HORIZONTAL | TRANSFOMAT_NO_MAPPING | TRANSFOMAT_NO_MIN_MAX:

        InputPixelCoordY = (int32_t) InputPixelCoordUserY;
        BaseIndex -= (InputPixelCoordY * InputBitmapSizeX);

            for(iPixel=0; iPixel < pi_ScanLineLength; iPixel++)
        {
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

                    if ((InputPixelCoordX >= ImageDimensionX) ||
                        (InputPixelCoordY >= ImageDimensionY) )
                {
                    *pOutputBuffer = pi_pColorInfo->m_DefaultPixelValue;
                }
                else
                {
                    /*
                    **  Identify the output pixel value
                    **  Assign it directly if we are allowed to and if
                    **  the reference pixel is into the received input buffer
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
                            Color = pInputBuffer[BaseIndex + InputPixelCoordX];
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

                *pOutputBuffer = Color;
                }
            }

            /*
            **  Compute next input pixel coordinates in user units
            */
            InputPixelCoordUserX +=  IncrementX;

            pOutputBuffer++;
        }
        break;


    case TRANSFOMAT_INPUT_COMPLETE | TRANSFOMAT_VERTICAL | TRANSFOMAT_NO_MAPPING | TRANSFOMAT_NO_MIN_MAX:

        InputPixelCoordX = (int32_t) InputPixelCoordUserX;
        BaseIndex += InputPixelCoordX;

            for(iPixel=0; iPixel < pi_ScanLineLength; iPixel++)
        {
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
                InputPixelCoordY = (int32_t) InputPixelCoordUserY;

                    if ((InputPixelCoordX >= ImageDimensionX) ||
                        (InputPixelCoordY >= ImageDimensionY) )
                {
                    *pOutputBuffer = pi_pColorInfo->m_DefaultPixelValue;
                }
                else
                {
                        Color = pInputBuffer[BaseIndex - (InputPixelCoordY * InputBitmapSizeX)];
                    *pOutputBuffer = Color;
                }
            }

            /*
            **  Compute next input pixel coordinates in user units
            */
            InputPixelCoordUserY +=  IncrementY;

            pOutputBuffer++;
        }
        break;


    case TRANSFOMAT_VERTICAL | TRANSFOMAT_NO_MAPPING | TRANSFOMAT_NO_MIN_MAX:

        InputPixelCoordX = (int32_t) InputPixelCoordUserX;
        BaseIndex += InputPixelCoordX;

            for(iPixel=0; iPixel < pi_ScanLineLength; iPixel++)
        {
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
                    **  Assign it directly if we are allowed to and if
                    **  the reference pixel is into the received input buffer
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
                            Color = pInputBuffer[BaseIndex - (InputPixelCoordY * InputBitmapSizeX)];
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

                *pOutputBuffer = Color;
                }
            }

            /*
            **  Compute next input pixel coordinates in user units
            */
            InputPixelCoordUserY +=  IncrementY;

            pOutputBuffer++;
        }
        break;


    case TRANSFOMAT_INPUT_COMPLETE | TRANSFOMAT_NO_MAPPING | TRANSFOMAT_NO_MIN_MAX:
            for(iPixel=0; iPixel < pi_ScanLineLength; iPixel++)
        {
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
                         Color = pInputBuffer[BaseIndex + InputPixelCoordX - (InputPixelCoordY * InputBitmapSizeX)];
                    *pOutputBuffer = Color;
                }
            }

            /*
            **  Compute next input pixel coordinates in user units
            */
            InputPixelCoordUserX +=  IncrementX;
            InputPixelCoordUserY +=  IncrementY;

            pOutputBuffer++;
        }
        break;


    case TRANSFOMAT_NO_MAPPING | TRANSFOMAT_NO_MIN_MAX:
            for(iPixel=0; iPixel < pi_ScanLineLength; iPixel++)
        {
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
                    **  Assign it directly if we are allowed to and if
                    **  the reference pixel is into the received input buffer
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
                            Color = pInputBuffer[BaseIndex + InputPixelCoordX - (InputPixelCoordY * InputBitmapSizeX)];
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

                *pOutputBuffer = Color;
                }
            }

            /*
            **  Compute next input pixel coordinates in user units
            */
            InputPixelCoordUserX +=  IncrementX;
            InputPixelCoordUserY +=  IncrementY;

            pOutputBuffer++;
        }
        break;

    default:
            for(iPixel=0; iPixel < pi_ScanLineLength; iPixel++)
        {
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
                    **  Assign it directly if we are allowed to and if
                    **  the reference pixel is into the received input buffer
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
                            Color = pInputBuffer[BaseIndex + InputPixelCoordX - (InputPixelCoordY * InputBitmapSizeX)];
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
                        {
                                Color = pi_pColorInfo->m_DefaultPixelValue;
                        }
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

            /*
            **  Compute next input pixel coordinates in user units
            */
            InputPixelCoordUserX +=  IncrementX;
            InputPixelCoordUserY +=  IncrementY;

            pOutputBuffer++;
        }
    }

WRAPUP:
    return;
}


static HSTATUS  s_IdentifySignificative (double    pi_FirstPixelX,
                                         double    pi_FirstPixelY,
                                         double    pi_LastPixelX,
                                         double    pi_LastPixelY,
                                         double    pi_ImageDimensionX,
                                         double    pi_ImageDimensionY,
                                         double   *po_pFirstSignificativeX,
                                         double   *po_pFirstSignificativeY,
                                         double   *po_pLastSignificativeX,
                                         double   *po_pLastSignificativeY,
                                         int8_t   *po_pSignificativeExist)
{
    int8_t FirstPixelProcessed;
    int8_t LastPixelProcessed;
    double TempCoordinate;
    double IntersectionX[4];
    double IntersectionY[4];
    int32_t NumberOfIntersection;
    double aCoef;
    double bCoef;
    HDEF    (Status, H_SUCCESS);

    /*
    **  Initialize some variables to default values
    */
    *po_pSignificativeExist = TRUE;
    FirstPixelProcessed = FALSE;
    LastPixelProcessed = FALSE;

    /*
    **  If both coordinates received are inside the image, nothing else
    **  has to be done
    */
    if (pi_FirstPixelX >= 0.0 && pi_FirstPixelX < pi_ImageDimensionX &&
        pi_FirstPixelY >= 0.0 && pi_FirstPixelY < pi_ImageDimensionY)
    {
        *po_pFirstSignificativeX = pi_FirstPixelX;
        *po_pFirstSignificativeY = pi_FirstPixelY;
        FirstPixelProcessed = TRUE;
    }

    if (pi_LastPixelX >= 0.0 && pi_LastPixelX < pi_ImageDimensionX &&
        pi_LastPixelY >= 0.0 && pi_LastPixelY < pi_ImageDimensionY)
    {
        *po_pLastSignificativeX = pi_LastPixelX;
        *po_pLastSignificativeY = pi_LastPixelY;
        LastPixelProcessed = TRUE;
    }

    if (FirstPixelProcessed == TRUE && LastPixelProcessed == TRUE)
        goto WRAPUP;

    /*
    **  If both coordinates received are out of the same image side,
    **  there is no significative pixel to process
    */
    if ((pi_FirstPixelX < 0.0 && pi_LastPixelX < 0.0) ||
        (pi_FirstPixelY < 0.0 && pi_LastPixelY < 0.0) ||
        (pi_FirstPixelX >= pi_ImageDimensionX && pi_LastPixelX >= pi_ImageDimensionX) ||
        (pi_FirstPixelY >= pi_ImageDimensionY && pi_LastPixelY >= pi_ImageDimensionY))
    {
        *po_pSignificativeExist = FALSE;
        goto WRAPUP;
    }

    /*
    **  Check if the line is horizontal
    */
    if (fabs (pi_LastPixelY - pi_FirstPixelY) < 1.0)
    {
        *po_pFirstSignificativeY = pi_LastPixelY;
        *po_pLastSignificativeY = pi_LastPixelY;

        /*
        **  Identify First significative pixel
        */
        if (pi_FirstPixelX < pi_LastPixelX)                       /*Left -> Right*/
        {
            if (pi_FirstPixelX < 0.0)
            {
                *po_pFirstSignificativeX = 0.0;
            }
            else if (pi_FirstPixelX >= pi_ImageDimensionX)
            {
                *po_pSignificativeExist = FALSE;
            }
            else
                *po_pFirstSignificativeX = pi_FirstPixelX;
        }
        else                                                                    /*Right - > Left*/
        {
            if (pi_FirstPixelX >= pi_ImageDimensionX)
            {
                *po_pFirstSignificativeX = pi_ImageDimensionX - 1;
            }
            else if (pi_FirstPixelX < 0.0)
            {
                *po_pSignificativeExist = FALSE;
            }
            else
                *po_pFirstSignificativeX = pi_FirstPixelX;
        }

        /*
        **  Identify Last significative pixel
        */
        if (pi_FirstPixelX < pi_LastPixelX)                       /*Left -> Right*/
        {
            if (pi_LastPixelX >= pi_ImageDimensionX)
            {
                *po_pLastSignificativeX = pi_ImageDimensionX - 1;
            }
            else if (pi_LastPixelX < 0.0)
            {
                *po_pSignificativeExist = FALSE;
            }
            else
                *po_pLastSignificativeX = pi_LastPixelX;
        }
        else                                                                    /*Right - > Left*/
        {
            if (pi_LastPixelX < 0.0)
            {
                *po_pLastSignificativeX = 0.0;
            }
            else if (pi_LastPixelX >= pi_ImageDimensionX)
            {
                *po_pSignificativeExist = FALSE;
            }
            else
                *po_pLastSignificativeX = pi_LastPixelX;
        }

        goto WRAPUP;
    }

    /*
    **  Check if the line is vertical
    */
    if (fabs (pi_LastPixelX - pi_FirstPixelX) < 1.0)
    {
        *po_pFirstSignificativeX = pi_LastPixelX;
        *po_pLastSignificativeX = pi_LastPixelX;

        /*
        **  Identify First significative pixel
        */
        if (pi_FirstPixelY < pi_LastPixelY)                                     /*Bottom -> Top*/
        {
            if (pi_FirstPixelY < 0.0)
            {
                *po_pFirstSignificativeY = 0.0;
            }
            else if (pi_FirstPixelY >= pi_ImageDimensionY)
            {
                *po_pSignificativeExist = FALSE;
            }
            else
                *po_pFirstSignificativeY = pi_FirstPixelY - 1;
        }
        else                                                                    /*Top - > Bottom*/
        {
            if (pi_FirstPixelY >= pi_ImageDimensionY - 1)
            {
                *po_pFirstSignificativeY = pi_ImageDimensionY - 1;
            }
            else if (pi_FirstPixelY < 0.0)
            {
                *po_pSignificativeExist = FALSE;
            }
            else
                *po_pFirstSignificativeY = pi_FirstPixelY;
        }

        /*
        **  Identify Last significative pixel
        */
        if (pi_FirstPixelY < pi_LastPixelY)                       /*Bottom -> Top*/
        {
            if (pi_LastPixelY >= pi_ImageDimensionY)
            {
                *po_pLastSignificativeY = pi_ImageDimensionY - 1;
            }
            else if (pi_LastPixelY < 0.0)
            {
                *po_pSignificativeExist = FALSE;
            }
            else
                *po_pLastSignificativeY = pi_LastPixelY;
        }
        else                                                                    /*Top - > Bottom*/
        {
            if (pi_LastPixelY < 0.0)
            {
                *po_pLastSignificativeY = 0.0;
            }
            else if (pi_LastPixelY >= pi_ImageDimensionY)
            {
                *po_pSignificativeExist = FALSE;
            }
            else
                *po_pLastSignificativeY = pi_LastPixelY;
        }

        goto WRAPUP;
    }

    /*
    **  The line is oblique
    */
    /*
    **  Compute the line equation coeficients and use them
    **  to identify the intersections between the line and
    **  the image edges
    */
    aCoef = (pi_LastPixelY - pi_FirstPixelY) / (pi_LastPixelX - pi_FirstPixelX);
    bCoef = pi_LastPixelY - (aCoef * pi_LastPixelX);

    NumberOfIntersection = 0;

    /* Bottom edge */
    TempCoordinate = bCoef * -1.0 / aCoef;
    if (TempCoordinate >= 0.0 && TempCoordinate < pi_ImageDimensionX)
    {
        IntersectionX[NumberOfIntersection] = TempCoordinate;
        IntersectionY[NumberOfIntersection] = 0.0;
        NumberOfIntersection++;
    }

    /* Top edge */
    TempCoordinate = (pi_ImageDimensionY - 1 - bCoef) / aCoef;
    if (TempCoordinate >= 0.0 && TempCoordinate < pi_ImageDimensionX)
    {
        IntersectionX[NumberOfIntersection] = TempCoordinate;
        IntersectionY[NumberOfIntersection] = pi_ImageDimensionY - 1;
        NumberOfIntersection++;
    }

    /* Left edge */
    if (bCoef >= 0.0 && bCoef < pi_ImageDimensionY)
    {
        IntersectionX[NumberOfIntersection] = 0.0;
        IntersectionY[NumberOfIntersection] = bCoef;
        NumberOfIntersection++;
    }

    /* Right edge */
    TempCoordinate = aCoef * (pi_ImageDimensionX - 1) + bCoef;
    if (TempCoordinate >= 0.0 && TempCoordinate < pi_ImageDimensionY)
    {
        IntersectionX[NumberOfIntersection] = pi_ImageDimensionX - 1;
        IntersectionY[NumberOfIntersection] = TempCoordinate;
        NumberOfIntersection++;
    }

    /*
    **  No significative pixel exists if there is zero or one intersection
    **  with the image
    */
    if (NumberOfIntersection < 2)
    {
        *po_pSignificativeExist = FALSE;
        goto WRAPUP;
    }

    /*
    **  Eliminate duplicate intersections if such ones exist
    */
    if (NumberOfIntersection > 2)
    {
        int32_t i, j, k;

        for (i = 0; i < NumberOfIntersection; i++)
        {
            for (j = i + 1; j < NumberOfIntersection; j++)
            {
                if (IntersectionX[i] == IntersectionX[j] && IntersectionY[i] == IntersectionY[j])
                {
                    for (k = j + 1; k < NumberOfIntersection; k++)
                    {
                        IntersectionX[j] = IntersectionX[k];
                        IntersectionY[j] = IntersectionY[k];
                        NumberOfIntersection--;
                    }
                }
            }
        }
    }

    /*
    **  Two intersections remain
    **  Sort them the same way the reference pixels are
    */
    if (pi_FirstPixelX < pi_LastPixelX)
    {
        if (IntersectionX[1] < IntersectionX[0])
        {
            IntersectionX[2] = IntersectionX[0];
            IntersectionY[2] = IntersectionY[0];

            IntersectionX[0] = IntersectionX[1];
            IntersectionY[0] = IntersectionY[1];

            IntersectionX[1] = IntersectionX[2];
            IntersectionY[1] = IntersectionY[2];
        }
    }
    else
        if (IntersectionX[1] > IntersectionX[0])
        {
            IntersectionX[2] = IntersectionX[0];
            IntersectionY[2] = IntersectionY[0];

            IntersectionX[0] = IntersectionX[1];
            IntersectionY[0] = IntersectionY[1];

            IntersectionX[1] = IntersectionX[2];
            IntersectionY[1] = IntersectionY[2];
        }


    /*
    **  Initialize output values that have not been already initialized
    */
    if (FirstPixelProcessed != TRUE)
    {
        *po_pFirstSignificativeX = IntersectionX[0];
        *po_pFirstSignificativeY = IntersectionY[0];
    }

    if (LastPixelProcessed != TRUE)
    {
        *po_pLastSignificativeX = IntersectionX[1];
        *po_pLastSignificativeY = IntersectionY[1];
    }

WRAPUP:

    HRET (Status);
}
