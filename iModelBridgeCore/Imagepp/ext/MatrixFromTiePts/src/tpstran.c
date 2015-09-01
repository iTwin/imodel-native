/*--------------------------------------------------------------------------------------+
|
|     $Source: ext/MatrixFromTiePts/src/tpstran.c $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "trfmodel.h"

/*____________________________________________________________________________
|  Static Variable daclaration
|____________________________________________________________________________*/

static int8_t   s_deltaYisAllocated[2]  = {FALSE, FALSE};
static int8_t   s_deltaYoptimisation[2] = {FALSE, FALSE};
static int32_t   s_nbDeltaY[2] = {0, 0};
static double * s_deltaY[2] = {NULL, NULL};


/*____________________________________________________________________________
|  trf_transfoCoordTps2D
|
|  Stephane Poulin   JULY 1998
|____________________________________________________________________________*/
void trf_transfoCoordTps2D (int32_t            mapping,
                             COEFFICIENTS_3D  * coef3D,
                             DCOORD_3D        * ptIn,
                             DCOORD_3D        * ptOut )
{
    int32_t i, j;
    double sum_x, sum_y;
    double r2;


    /*
    ** Compute the summation
    */
    sum_x = 0.0;
    sum_y = 0.0;
    for (i=3, j=0; i<coef3D->nbTpsCoef[XIND]; i++, j++)
    {

        r2 = SQR(mat_v(coef3D->tpsPoints[mapping],j,0) - ptIn->x) +
             SQR(mat_v(coef3D->tpsPoints[mapping],j,1) - ptIn->y);

        if (0.0 != r2)
        {
            sum_x += coef3D->tpsCoef[mapping][XIND][i] * r2 * log(r2);
            sum_y += coef3D->tpsCoef[mapping][YIND][i] * r2 * log(r2);
        }
    }

    ptOut->x = ptIn->x + ( coef3D->tpsCoef[mapping][XIND][0] + coef3D->tpsCoef[mapping][XIND][1] * ptIn->x + coef3D->tpsCoef[mapping][XIND][2] * ptIn->y + sum_x );
    ptOut->y = ptIn->y + ( coef3D->tpsCoef[mapping][YIND][0] + coef3D->tpsCoef[mapping][YIND][1] * ptIn->x + coef3D->tpsCoef[mapping][YIND][2] * ptIn->y + sum_y );
    ptOut->z = 0.0;

}

/*____________________________________________________________________________
|  trf_transfoCoordPrepTps2D
|
|  Stephane Poulin   JULY 1998
|____________________________________________________________________________*/
void trf_transfoCoordPrepTps2D (int32_t            mapping,
                                 pCOEFFICIENTS_3D   coef3D,
                                 pDCOORD_3D         ptIn,
                                 double           *yprod)
{
    /*
    ** yprod[0] = Y value of the input point
    */
    yprod[0] = ptIn->y;

    /*
    ** yprod[1] = A0 + A2y for the X solution
    */
    yprod[1] = coef3D->tpsCoef[mapping][XIND][0] +
               coef3D->tpsCoef[mapping][XIND][2] * ptIn->y;
    /*
    ** yprod[1] = A0 + A2y for the Y solution
    */
    yprod[2] = coef3D->tpsCoef[mapping][YIND][0] +
               coef3D->tpsCoef[mapping][YIND][2] * ptIn->y;
}

/*____________________________________________________________________________
|  trf_transfoCoordDoTps2D
|
|  Stephane Poulin   JULY 1998
|____________________________________________________________________________*/
void trf_transfoCoordDoTps2D  (int32_t             mapping,
                                pCOEFFICIENTS_3D    coef3D,
                                pDCOORD_3D          ptIn,
                                pDCOORD_3D          ptOut,
                                double            *yprod)
{
    int32_t i, j;
    double sum_x, sum_y;
    double r2;


    /*
    ** Compute the summation
    */
    sum_x = 0.0;
    sum_y = 0.0;
    for (i=3, j=0; i<coef3D->nbTpsCoef[XIND]; i++, j++)
    {

        r2 = SQR(mat_v(coef3D->tpsPoints[mapping],j,0) - ptIn->x) +
             SQR(mat_v(coef3D->tpsPoints[mapping],j,1) - ptIn->y);

        if (0.0 != r2)
        {
            sum_x += coef3D->tpsCoef[mapping][XIND][i] * r2 * log(r2);
            sum_y += coef3D->tpsCoef[mapping][YIND][i] * r2 * log(r2);
        }
    }

    ptOut->x = ptIn->x + ( yprod[1] + coef3D->tpsCoef[mapping][XIND][1] * ptIn->x + sum_x );
    ptOut->y = ptIn->y + ( yprod[2] + coef3D->tpsCoef[mapping][YIND][1] * ptIn->x + sum_y );
    ptOut->z = 0.0;
}


/*____________________________________________________________________________
|  trf_transfoCoordDoTpsScanLine2D
|
|  Stephane Poulin   JULY 1998
|____________________________________________________________________________*/
void trf_transfoCoordDoTpsScanLine2D(int32_t            mapping,
                                                                                             pCOEFFICIENTS_3D   coef3D,
                                                                                             double            scanLineOriginX,
                                                                                             double            scanLinePixelSizeX,
                                                                                             int32_t           scanLineLength,
                                                                                             pDCOORD_3D         ptOut,
                                                                                             double           *yprod)
{
    int32_t pt;

    int32_t i, j;
    double sum_x, sum_y;
    double r2;

    /*
    ** Optimisation, Compute the y part of r2 for the scanline
    */
    if (s_deltaYisAllocated[mapping])
    {
        if (s_nbDeltaY[mapping] != coef3D->tpsPoints[mapping]->rows)
        {
            if (s_deltaY[mapping] != 0)
                free(s_deltaY[mapping]);
            s_deltaY[mapping] = (double*)malloc ( coef3D->tpsPoints[mapping]->rows * sizeof(double));
            if (NULL == s_deltaY[mapping])
            {
                s_deltaYoptimisation[mapping] = FALSE;
                s_deltaYisAllocated[mapping]  = FALSE;
            }
            else
            {
                s_nbDeltaY[mapping] =  coef3D->tpsPoints[mapping]->rows;
                s_deltaYisAllocated[mapping] = TRUE;
                s_deltaYoptimisation[mapping] = TRUE;
            }
        }
    }
    else
    {
        s_deltaY[mapping] = (double*)malloc ( coef3D->tpsPoints[mapping]->rows * sizeof(double));
        if (NULL == s_deltaY[mapping])
        {
            s_deltaYoptimisation[mapping] = FALSE;
            s_deltaYisAllocated[mapping]  = FALSE;
        }
        else
        {
            s_nbDeltaY[mapping] =  coef3D->tpsPoints[mapping]->rows;
            s_deltaYisAllocated[mapping] = TRUE;
            s_deltaYoptimisation[mapping] = TRUE;
        }
    }

    if (TRUE == s_deltaYoptimisation[mapping])
    {
        for (j=0; j<s_nbDeltaY[mapping]; j++)
        {
            s_deltaY[mapping][j] = SQR(mat_v(coef3D->tpsPoints[mapping],j,1) - yprod[0]);
        }
    }


    if( s_deltaYoptimisation[mapping] )
        {
        for(pt=0; pt < scanLineLength; pt++, scanLineOriginX += scanLinePixelSizeX )
        {
            sum_x = 0.0;
            sum_y = 0.0;

            /*
            ** Compute the summation
            */
            for (i=3, j=0; i<coef3D->nbTpsCoef[XIND]; i++, j++)
            {
                r2 = SQR(mat_v(coef3D->tpsPoints[mapping],j,0) - scanLineOriginX) + s_deltaY[mapping][j];

                if (0.0 != r2)
                {
                    sum_x += coef3D->tpsCoef[mapping][XIND][i] * r2 * log(r2);
                    sum_y += coef3D->tpsCoef[mapping][YIND][i] * r2 * log(r2);
                }
            }

            ptOut[pt].x = scanLineOriginX + ( yprod[1] + coef3D->tpsCoef[mapping][XIND][1] * scanLineOriginX + sum_x );
            ptOut[pt].y = yprod[0]        + ( yprod[2] + coef3D->tpsCoef[mapping][YIND][1] * scanLineOriginX + sum_y );
            ptOut[pt].z = 0.0;
        }
    }
    else
    {
        for(pt=0; pt < scanLineLength; pt++, scanLineOriginX += scanLinePixelSizeX )
            {
            sum_x = 0.0;
            sum_y = 0.0;

            /*
            ** Compute the summation
            */
            for (i=3, j=0; i<coef3D->nbTpsCoef[XIND]; i++, j++)
            {
                r2 = SQR(mat_v(coef3D->tpsPoints[mapping],j,0) - scanLineOriginX) +
                     SQR(mat_v(coef3D->tpsPoints[mapping],j,1) - yprod[0]);

                if (0.0 != r2)
                {
                    sum_x += coef3D->tpsCoef[mapping][XIND][i] * r2 * log(r2);
                    sum_y += coef3D->tpsCoef[mapping][YIND][i] * r2 * log(r2);
                }
            }

            ptOut[pt].x = scanLineOriginX + ( yprod[1] + coef3D->tpsCoef[mapping][XIND][1] * scanLineOriginX + sum_x );
            ptOut[pt].y = yprod[0]        + ( yprod[2] + coef3D->tpsCoef[mapping][YIND][1] * scanLineOriginX + sum_y );
            ptOut[pt].z = 0.0;
        }
    }
}

/*____________________________________________________________________________
|  trf_transfoDoTpsScanLine2D
|
|  Stephane Poulin   JULY 1998
|____________________________________________________________________________*/

void trf_transfoDoTpsScanLine2D (int32_t                 pi_Mapping,
                                   pCOEFFICIENTS_3D       pi_pCoef3D,
                                   double                pi_ScanLineOriginX,
                                   double                pi_ScanLinePixelSizeX,
                                   int32_t               pi_ScanLineLength,
                                   double               *pi_pYprod,
                                   TRANSFO_RSP_BUFF_INFO *pio_pBufferInfo,
                                   TRANSFO_RSP_COLOR_INFO *pi_pColorInfo)
{
    double OutputPixelCoordUserX;
    double InputPixelCoordUserX;
    double InputPixelCoordUserY;
    int32_t InputPixelCoordX;
    int32_t InputPixelCoordY;
    int32_t InputBitmapSizeX;
    int32_t InputBitmapSizeY;
    int32_t InputBitmapLastIndexY;
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
    int32_t i, j;

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
    EndInputBitmapPixelX = StartInputBitmapPixelX + InputBitmapSizeX;
    EndInputBitmapPixelY = StartInputBitmapPixelY + InputBitmapSizeY;
    pInputBuffer = pio_pBufferInfo->m_pInputBuffer;

    InputBitmapLastIndexY = InputBitmapSizeY - 1;

    /*
    ** Optimisation, Compute the y part of r2 for the scanline
    */
    if (s_deltaYisAllocated[pi_Mapping])
    {
        if (s_nbDeltaY[pi_Mapping] != pi_pCoef3D->tpsPoints[pi_Mapping]->rows)
        {
            if (s_deltaY[pi_Mapping] != 0)
                free(s_deltaY[pi_Mapping]);
            s_deltaY[pi_Mapping] = (double*)malloc ( pi_pCoef3D->tpsPoints[pi_Mapping]->rows * sizeof(double));
            if (NULL == s_deltaY[pi_Mapping])
            {
                s_deltaYoptimisation[pi_Mapping] = FALSE;
                s_deltaYisAllocated[pi_Mapping]  = FALSE;
            }
            else
            {
                s_nbDeltaY[pi_Mapping] =  pi_pCoef3D->tpsPoints[pi_Mapping]->rows;
                s_deltaYisAllocated[pi_Mapping] = TRUE;
                s_deltaYoptimisation[pi_Mapping] = TRUE;
            }
        }
    }
    else
    {
        s_deltaY[pi_Mapping] = (double*)malloc ( pi_pCoef3D->tpsPoints[pi_Mapping]->rows * sizeof(double));
        if (NULL == s_deltaY[pi_Mapping])
        {
            s_deltaYoptimisation[pi_Mapping] = FALSE;
            s_deltaYisAllocated[pi_Mapping]  = FALSE;
        }
        else
        {
            s_nbDeltaY[pi_Mapping] =  pi_pCoef3D->tpsPoints[pi_Mapping]->rows;
            s_deltaYisAllocated[pi_Mapping] = TRUE;
            s_deltaYoptimisation[pi_Mapping] = TRUE;
        }
    }

    if (TRUE == s_deltaYoptimisation[pi_Mapping])
    {
        for (j=0; j<s_nbDeltaY[pi_Mapping]; j++)
        {
            s_deltaY[pi_Mapping][j] = SQR(mat_v(pi_pCoef3D->tpsPoints[pi_Mapping],j,1) - pi_pYprod[0]);
        }
    }

    /*
    **  Process pixels
    */
    OutputPixelCoordUserX = pi_ScanLineOriginX;
    pOutputBuffer = pio_pBufferInfo->m_pOutputBuffer;

        for(iPixel=0; iPixel < pi_ScanLineLength; iPixel++)
    {
        double sum_x, sum_y;
        double r2;

        /*
        **  Compute input pixel coordinates in user units
        */
        sum_x = 0.0;
        sum_y = 0.0;

        if( s_deltaYoptimisation[pi_Mapping] )
        {
            for (i=3, j=0; i<pi_pCoef3D->nbTpsCoef[XIND]; i++, j++)
            {
                r2 = SQR(mat_v(pi_pCoef3D->tpsPoints[pi_Mapping],j,0) - OutputPixelCoordUserX) + s_deltaY[pi_Mapping][j];

                if (0.0 != r2)
                {
                    sum_x += pi_pCoef3D->tpsCoef[pi_Mapping][XIND][i] * r2 * log(r2);
                    sum_y += pi_pCoef3D->tpsCoef[pi_Mapping][YIND][i] * r2 * log(r2);
                }
            }
        }
        else
        {
            for (i=3, j=0; i<pi_pCoef3D->nbTpsCoef[XIND]; i++, j++)
            {
                r2 = SQR(mat_v(pi_pCoef3D->tpsPoints[pi_Mapping],j,0) - OutputPixelCoordUserX) +
                     SQR(mat_v(pi_pCoef3D->tpsPoints[pi_Mapping],j,1) -  pi_pYprod[0]);

                if (0.0 != r2)
                {
                    sum_x += pi_pCoef3D->tpsCoef[pi_Mapping][XIND][i] * r2 * log(r2);
                    sum_y += pi_pCoef3D->tpsCoef[pi_Mapping][YIND][i] * r2 * log(r2);
                }
            }
        }

        InputPixelCoordUserX = (OutputPixelCoordUserX + ( pi_pYprod[1] + pi_pCoef3D->tpsCoef[pi_Mapping][XIND][1] * OutputPixelCoordUserX + sum_x ) - pio_pBufferInfo->m_InputImageOrigin.x) / pio_pBufferInfo->m_InputImagePixelSize;
        InputPixelCoordUserY = (pi_pYprod[0]          + ( pi_pYprod[2] + pi_pCoef3D->tpsCoef[pi_Mapping][YIND][1] * OutputPixelCoordUserX + sum_y ) - pio_pBufferInfo->m_InputImageOrigin.y) / pio_pBufferInfo->m_InputImagePixelSize;

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
                       InputPixelCoordX < EndInputBitmapPixelX &&
                       InputPixelCoordY < EndInputBitmapPixelY)
                {
                        Color = pInputBuffer[(InputBitmapLastIndexY - (InputPixelCoordY - StartInputBitmapPixelY)) * InputBitmapSizeX +
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
