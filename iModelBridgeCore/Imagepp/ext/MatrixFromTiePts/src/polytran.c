/*--------------------------------------------------------------------------------------+
|
|     $Source: ext/MatrixFromTiePts/src/polytran.c $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
/*mh================================================================ HMR
**  Octimage/MicroStation - Transformation Models
**
**      This file contains the functions relative to the
**      transformation of coordinate using a  polynomial (deg 1 a 5)
**
**===========================================================================*/
#include "trfmodel.h"


/*fh=========================================================================
**
**  trf_transfoCoordPoly2D
**
**  MARTIN A. BUSSIERES     VERSION ORIGINALE         10:21:53  1/10/1989
**  DONALD MORISSETTE       Changement de l'algo      05/06/90
**  ALAIN LAPIERRE          Adaptation pour Octimage 4 21/12/92
**===========================================================================*/

void trf_transfoCoordPoly2D(int32_t            mapping,
                             pCOEFFICIENTS_3D   coef3D,
                             pDCOORD_3D         ptIn,
                             pDCOORD_3D         ptOut)
    {
    double yprod[10];
    trf_transfoCoordPrepPoly2D(mapping, coef3D, ptIn, yprod);
    trf_transfoCoordDoPoly2D(mapping, coef3D, ptIn, ptOut, yprod);
    }


/*fh=========================================================================
**
**  trf_transfoCoordDoPoly2D
**
**  MARTIN A. BUSSIERES     VERSION ORIGINALE         10:21:53  1/10/1989
**  DONALD MORISSETTE       Changement de l'algo      05/06/90
**  ALAIN LAPIERRE          Adaptation pour Octimage 4 21/12/92
**===========================================================================*/

void trf_transfoCoordDoPoly2D(int32_t          mapping,
                               pCOEFFICIENTS_3D coef3D,
                               pDCOORD_3D       ptIn,
                               pDCOORD_3D       ptOut,
                               double          *yprod)

   {
    int32_t i;
    int32_t nbTermes;     /* nombre maximum de termes en X ou Y */
    double *coef, *prod;
    double x, *pout;
    int32_t maxt;

    /*
    ** Trouver le nombre maximum de termes en X ou en Y
    ** apres avoir le premier coefficient servant a la translation
    */

    nbTermes = max(coef3D->nbCoef[XIND]-1,coef3D->nbCoef[YIND]-1);

    /* calculer les puissances de X */

    x = ptIn->x - coef3D->coef[mapping][0][0];    /* soustraire l'offset           */
                                              /* -->diminue l'erreur de calcul */


    /*
    ** Ajustements fait par Alain Lapierre pour adaptation a la structure
    ** des modeles mathematiques de Octimage 4 21/12/92
    */
    pout = (double *) ptOut;

    /*
    ** calculer X et Y resultants
    */
    for(i=0;i<2;i++)
       {
        prod = &(yprod[i*5]);
        coef = coef3D->coef[mapping][i];
        maxt = coef3D->nbCoef[i]-1;

        switch( maxt )
           {
            case  2 :
                pout[i] = coef[1] + (x * coef[2]);
                break;

            case  3 :
                pout[i] = coef[1] + (x * coef[2]) + prod[0];
                break;

            case  4 :
                pout[i] = coef[1] + (x * (coef[2]+prod[0])) + prod[1];
                break;

            case  5 :
            case  6 :
                pout[i] = coef[1] + (x * (coef[2]+prod[0]+(x * coef[5]))) +
                        prod[1];
                break;

            case  7 :
            case  8 :
                pout[i] = coef[1] + (x * (coef[2]+prod[0]+(x *
                        (coef[5] + prod[1])))) +
                        prod[2];
                break;

            case  9 :
            case 10 :
            case 11 :
                pout[i] = coef[1] + (x * (coef[2]+prod[0]+(x *
                        (coef[5] + prod[1] +(x * coef[9]))))) +
                        prod[2];
                break;

            case 12 :
            case 13 :
                pout[i] = coef[1] + (x * (coef[2]+prod[0]+(x *
                        (coef[5] + prod[1] +(x * coef[9] + prod[2]))))) +
                        prod[3];
                break;

            case 14 :
            case 15 :
            case 16 :
            case 17 :
                pout[i] = coef[1] + (x * (coef[2]+prod[0]+(x *
                        (coef[5] + prod[1] +(x * coef[9] +
                        prod[2] + (x * coef[14])))))) +
                        prod[3];
                break;

            case 18 :
            case 19 :
                pout[i] = coef[1] + (x * (coef[2]+prod[0]+(x *
                        (coef[5] + prod[1] +(x * coef[9] +
                        prod[2] + (x * (coef[14] + prod[3]))))))) +
                        prod[4];
                break;

            case 20 :
            case 21 :
                pout[i] = coef[1] + (x * (coef[2]+prod[0]+(x *
                        (coef[5] + prod[1] +(x * coef[9] +
                        prod[2] + (x * (coef[14] + prod[3]+(x*coef[20])))))))) +
                        prod[4];
                break;
           }
       }/* for */


   mapping = (mapping == TRANSFO_DIRECT) ? TRANSFO_INVERSE : TRANSFO_DIRECT;
   ptOut->x= ptOut->x + coef3D->coef[mapping][XIND][0];
   ptOut->y= ptOut->y + coef3D->coef[mapping][YIND][0];
   ptOut->z = 0.0;
   }

/*fh=========================================================================
**
**  trf_transfoCoordDoPolyScanLine2D
**
**  ALAIN LAPIERRE          original version          Feb 97
**===========================================================================*/
void trf_transfoCoordDoPolyScanLine2D(int32_t           mapping,
                                                                                         pCOEFFICIENTS_3D       coef3D,
                                                                                         double                scanLineOriginX,
                                                                                         double                scanLinePixelSizeX,
                                                                                         int32_t                       scanLineLength,
                                                                                         pDCOORD_3D             ptOut,
                                                                                         double               *yprod)
{
    int32_t pt;
        int32_t mappingTran;
    int32_t i;
    int32_t nbTermes;     /* nombre maximum de termes en X ou Y */
    double *coef, *prod;
    double x, *pout;
    int32_t maxt;

        mappingTran = (mapping == TRANSFO_DIRECT) ? TRANSFO_INVERSE : TRANSFO_DIRECT;

        for(pt=0; pt < scanLineLength; pt++, scanLineOriginX += scanLinePixelSizeX )
        {

        /*
        ** Trouver le nombre maximum de termes en X ou en Y
        ** apres avoir le premier coefficient servant a la translation
        */

        nbTermes = max(coef3D->nbCoef[XIND]-1,coef3D->nbCoef[YIND]-1);

        /* calculer les puissances de X */

        x = scanLineOriginX - coef3D->coef[mapping][0][0];    /* soustraire l'offset           */
                                                  /* -->diminue l'erreur de calcul */


        /*
        ** Ajustements fait par Alain Lapierre pour adaptation a la structure
        ** des modeles mathematiques de Octimage 4 21/12/92
        */
        pout = (double *) &ptOut[pt];

        /*
        ** calculer X et Y resultants
        */
        for(i=0;i<2;i++)
           {
            prod = &(yprod[i*5]);
            coef = coef3D->coef[mapping][i];
            maxt = coef3D->nbCoef[i]-1;

            switch( maxt )
               {
                case  2 :
                    pout[i] = coef[1] + (x * coef[2]);
                    break;

                case  3 :
                    pout[i] = coef[1] + (x * coef[2]) + prod[0];
                    break;

                case  4 :
                    pout[i] = coef[1] + (x * (coef[2]+prod[0])) + prod[1];
                    break;

                case  5 :
                case  6 :
                    pout[i] = coef[1] + (x * (coef[2]+prod[0]+(x * coef[5]))) +
                            prod[1];
                    break;

                case  7 :
                case  8 :
                    pout[i] = coef[1] + (x * (coef[2]+prod[0]+(x *
                            (coef[5] + prod[1])))) +
                            prod[2];
                    break;

                case  9 :
                case 10 :
                case 11 :
                    pout[i] = coef[1] + (x * (coef[2]+prod[0]+(x *
                            (coef[5] + prod[1] +(x * coef[9]))))) +
                            prod[2];
                    break;

                case 12 :
                case 13 :
                    pout[i] = coef[1] + (x * (coef[2]+prod[0]+(x *
                            (coef[5] + prod[1] +(x * coef[9] + prod[2]))))) +
                            prod[3];
                    break;

                case 14 :
                case 15 :
                case 16 :
                case 17 :
                    pout[i] = coef[1] + (x * (coef[2]+prod[0]+(x *
                            (coef[5] + prod[1] +(x * coef[9] +
                            prod[2] + (x * coef[14])))))) +
                            prod[3];
                    break;

                case 18 :
                case 19 :
                    pout[i] = coef[1] + (x * (coef[2]+prod[0]+(x *
                            (coef[5] + prod[1] +(x * coef[9] +
                            prod[2] + (x * (coef[14] + prod[3]))))))) +
                            prod[4];
                    break;

                case 20 :
                case 21 :
                    pout[i] = coef[1] + (x * (coef[2]+prod[0]+(x *
                            (coef[5] + prod[1] +(x * coef[9] +
                            prod[2] + (x * (coef[14] + prod[3]+(x*coef[20])))))))) +
                            prod[4];
                    break;
               }
           }/* for */


       ptOut[pt].x= ptOut[pt].x + coef3D->coef[mappingTran][XIND][0];
       ptOut[pt].y= ptOut[pt].y + coef3D->coef[mappingTran][YIND][0];
   } /* for pts */
}

/*fh=========================================================================
**
**  trf_transfoDoPolyScanLine2D
**
**  Jean Papillon          original version          Dec 97
**===========================================================================*/
void trf_transfoDoPolyScanLine2D (int32_t                pi_Mapping,
                                   pCOEFFICIENTS_3D       pi_pCoef3D,
                                   double                pi_ScanLineOriginX,
                                   double                pi_ScanLinePixelSizeX,
                                   int32_t               pi_ScanLineLength,
                                   double               *pi_pYprod,
                                   TRANSFO_RSP_BUFF_INFO *pio_pBufferInfo,
                                   TRANSFO_RSP_COLOR_INFO *pi_pColorInfo)
{
    int32_t     NbTermes;        /* nombre maximum de termes en X ou Y */
    double    *pCoef;
    double    *pProd;
    double     X;
    double    *pInputPixelCoordUser;
    int32_t     Maxt;
    int32_t     i;
    double     OutputPixelCoordUserX;
    DCOORD_3D   InputPixelCoordUser;
    int32_t    InputPixelCoordX;
    int32_t    InputPixelCoordY;
    int32_t    InputBitmapSizeX;
    int32_t    InputBitmapSizeY;
    int32_t    StartInputBitmapPixelX;
    int32_t    StartInputBitmapPixelY;
    int32_t    EndInputBitmapPixelX;
    int32_t    EndInputBitmapPixelY;
    int32_t    ImageDimensionX;
    int32_t    ImageDimensionY;
    unsigned char     *pOutputBuffer;
    unsigned char     *pInputBuffer;
    int32_t    iPixel;
        int32_t     MappingTran;
    unsigned char      Color;

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

    /*
    **  Compute some constant values for the line
    */
        MappingTran = (pi_Mapping == TRANSFO_DIRECT) ? TRANSFO_INVERSE : TRANSFO_DIRECT;

    /*
    **  Process pixels
    */
    OutputPixelCoordUserX = pi_ScanLineOriginX;
    pOutputBuffer = pio_pBufferInfo->m_pOutputBuffer;

        for(iPixel=0; iPixel < pi_ScanLineLength; iPixel++)
    {
        /*
        ** Trouver le nombre maximum de termes en X ou en Y
        ** apres avoir le premier coefficient servant a la translation
        */
        NbTermes = max(pi_pCoef3D->nbCoef[XIND] - 1, pi_pCoef3D->nbCoef[YIND] - 1);

        /* calculer les puissances de X */
        X = OutputPixelCoordUserX - pi_pCoef3D->coef[pi_Mapping][0][0];    /* soustraire l'offset*/
                                                  /* -->diminue l'erreur de calcul */

        /*
        ** Ajustements fait par Alain Lapierre pour adaptation a la structure
        ** des modeles mathematiques de Octimage 4 21/12/92
        */
        pInputPixelCoordUser = (double *) &InputPixelCoordUser;

        /*
        ** calculer X et Y resultants
        */
        for(i=0;i<2;i++)
           {
            pProd = &(pi_pYprod [i*5]);
            pCoef = pi_pCoef3D->coef [pi_Mapping][i];
            Maxt = pi_pCoef3D->nbCoef [i] - 1;

            switch( Maxt )
               {
                case  2 :
                    pInputPixelCoordUser[i] = pCoef[1] + (X * pCoef[2]);
                    break;

                case  3 :
                    pInputPixelCoordUser[i] = pCoef[1] + (X * pCoef[2]) + pProd[0];
                    break;

                case  4 :
                    pInputPixelCoordUser[i] = pCoef[1] + (X * (pCoef[2]+pProd[0])) + pProd[1];
                    break;

                case  5 :
                case  6 :
                    pInputPixelCoordUser[i] = pCoef[1] + (X * (pCoef[2]+pProd[0]+(X*pCoef[5]))) +
                            pProd[1];
                    break;

                case  7 :
                case  8 :
                    pInputPixelCoordUser[i] = pCoef[1] + (X * (pCoef[2]+pProd[0]+(X *
                            (pCoef[5] + pProd[1])))) +
                            pProd[2];
                    break;

                case  9 :
                case 10 :
                case 11 :
                    pInputPixelCoordUser[i] = pCoef[1] + (X * (pCoef[2]+pProd[0]+(X *
                            (pCoef[5] + pProd[1] +(X * pCoef[9]))))) +
                            pProd[2];
                    break;

                case 12 :
                case 13 :
                    pInputPixelCoordUser[i] = pCoef[1] + (X * (pCoef[2]+pProd[0]+(X *
                            (pCoef[5] + pProd[1] +(X * pCoef[9] + pProd[2]))))) +
                            pProd[3];
                    break;

                case 14 :
                case 15 :
                case 16 :
                case 17 :
                    pInputPixelCoordUser[i] = pCoef[1] + (X * (pCoef[2]+pProd[0]+(X *
                            (pCoef[5] + pProd[1] +(X * pCoef[9] +
                            pProd[2] + (X * pCoef[14])))))) +
                            pProd[3];
                    break;

                case 18 :
                case 19 :
                    pInputPixelCoordUser[i] = pCoef[1] + (X * (pCoef[2]+pProd[0]+(X *
                            (pCoef[5] + pProd[1] +(X * pCoef[9] +
                            pProd[2] + (X * (pCoef[14] + pProd[3]))))))) +
                            pProd[4];
                    break;

                case 20 :
                case 21 :
                    pInputPixelCoordUser[i] = pCoef[1] + (X * (pCoef[2]+pProd[0]+(X *
                            (pCoef[5] + pProd[1] +(X * pCoef[9] +
                            pProd[2] + (X * (pCoef[14] + pProd[3]+(X*pCoef[20])))))))) +
                            pProd[4];
                    break;
               }
           }/* for */

        InputPixelCoordUser.x= (InputPixelCoordUser.x +
                                pi_pCoef3D->coef[MappingTran][XIND][0] -
                                pio_pBufferInfo->m_InputImageOrigin.x) /
                                pio_pBufferInfo->m_InputImagePixelSize;
        InputPixelCoordUser.y= (InputPixelCoordUser.y +
                                pi_pCoef3D->coef[MappingTran][YIND][0] -
                                pio_pBufferInfo->m_InputImageOrigin.y) /
                                pio_pBufferInfo->m_InputImagePixelSize;
        /*
        **  Assign default value if pixel is outside the input image
        */
        if (InputPixelCoordUser.x < 0.0 ||
            InputPixelCoordUser.y < 0.0)
        {
            *pOutputBuffer = pi_pColorInfo->m_DefaultPixelValue;
                }
        else
        {
                InputPixelCoordX = (int32_t) InputPixelCoordUser.x;
            InputPixelCoordY = (int32_t) InputPixelCoordUser.y;

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
                    LCOORD_2D InputPixelCoord;

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

/*fh=========================================================================
**
**  trf_transfoCoordPrepPoly2D
**
**  MARTIN A. BUSSIERES     VERSION ORIGINALE         10:21:53  1/10/1989
**  DONALD MORISSETTE       Changenemt de l'algo      05/06/1990
**  ALAIN LAPIERRE          Adaptation pour Octimage 4 21/12/92
**===========================================================================*/

void trf_transfoCoordPrepPoly2D(int32_t            mapping,
                                 pCOEFFICIENTS_3D   coef3D,
                                 pDCOORD_3D         ptIn,
                                 double            *yprod)
   {
    int32_t i;
    int32_t nbTermes;     /* nombre maximum de termes en X ou Y       */
    double y;
    double *prod, *coef;

    /*
    ** Trouver le nombre maximum de termes en X ou en Y
    ** apres avoir enleve la translation
    */
    nbTermes = max(coef3D->nbCoef[XIND]-1,coef3D->nbCoef[YIND]-1);

    y = ptIn->y - coef3D->coef[mapping][YIND][0];  /* soustraire l'offset           */
                                                   /* -->diminue l'erreur de calcul */

    /*
    ** Ajustements fait par Alain Lapierre pour adaptation a la structure
    ** des modeles mathematiques de Octimage 4 21/12/92
    */


    for(i=0;i<2;i++)
       {
        prod = &(yprod[i*5]);
        coef = coef3D->coef[mapping][i];

        /* evaluer les 'nbTermes' premiers produits impliquant X et Y */
        switch( nbTermes )
           {
            case  2 :
                break;

            case  3 :
                prod[0] = y * coef[3];
                break;

            case  4 :
            case  5 :
                prod[0] = y * coef[4];
                prod[1] = y * coef[3];
                break;

            case  6 :
                prod[0] = y * coef[4];
                prod[1] = y * (coef[3] + (y * coef[6]));
                break;

            case  7 :
                prod[0] = y * coef[4];
                prod[1] = y * coef[7];
                prod[2] = y * (coef[3] + (y * coef[6]));
                break;

            case  8 :
            case  9 :
                prod[0] = y * (coef[4] + (y * coef[8]));
                prod[1] = y * coef[7];
                prod[2] = y * (coef[3] + (y * coef[6]));
                break;

            case 10 :
                prod[0] = y * (coef[4] + (y * coef[8]));
                prod[1] = y * coef[7];
                prod[2] = y * (coef[3] + (y * (coef[6] + (y * coef[10]))));
                break;

            case 11 :
                prod[0] = y * (coef[4] + (y * coef[8]));
                prod[1] = y * (coef[7] + (y * coef[11]));
                prod[2] = y * (coef[3] + (y * (coef[6] + (y * coef[10]))));
                break;

            case 12 :
                prod[0] = y * (coef[4] + (y * coef[8]));
                prod[1] = y * (coef[7] + (y * coef[11]));
                prod[2] = y * coef[12];
                prod[3] = y * (coef[3] + (y * (coef[6] + (y * coef[10]))));
                break;

            case 13 :
            case 14 :
                prod[0] = y * (coef[4] + (y * (coef[8] + (y * coef[13]))));
                prod[1] = y * (coef[7] + (y * coef[11]));
                prod[2] = y * coef[12];
                prod[3] = y * (coef[3] + (y * (coef[6] + (y * coef[10]))));
                break;

            case 15 :
                prod[0] = y * (coef[4] + (y * (coef[8] + (y * coef[13]))));
                prod[1] = y * (coef[7] + (y * coef[11]));
                prod[2] = y * coef[12];
                prod[3] = y * (coef[3] + (y * (coef[6] + (y *
                            (coef[10] + (y * coef[15]))))));
                break;

            case 16 :
                prod[0] = y * (coef[4] + (y * (coef[8] + (y * coef[13]))));
                prod[1] = y * (coef[7] + (y * coef[11]));
                prod[2] = y * (coef[12] + (y * coef[16]));
                prod[3] = y * (coef[3] + (y * (coef[6] + (y *
                            (coef[10] + (y * coef[15]))))));
                break;

            case 17 :
                prod[0] = y * (coef[4] + (y * (coef[8] + (y * coef[13]))));
                prod[1] = y * (coef[7] + (y * coef[11]));
                prod[2] = y * (coef[12] + (y * coef[16]));
                prod[3] = y * (coef[3] + (y * (coef[6] + (y *
                            (coef[10] + (y * coef[15]))))));
                break;

            case 18 :
                prod[0] = y * (coef[4] + (y * (coef[8] + (y * coef[13]))));
                prod[1] = y * (coef[7] + (y * coef[11]));
                prod[2] = y * (coef[12] + (y * coef[16]));
                prod[3] = y * coef[18];
                prod[4] = y * (coef[3] + (y * (coef[6] + (y *
                            (coef[10] + (y * coef[15]))))));
                break;

            case 19 :
            case 20 :
                prod[0] = y * (coef[4] + (y * (coef[8] + (y *
                        (coef[13] + (y * coef[19]))))));
                prod[1] = y * (coef[7] + (y * coef[11]));
                prod[2] = y * (coef[12] + (y * coef[16]));
                prod[3] = y * coef[18];
                prod[4] = y * (coef[3] + (y * (coef[6] + (y *
                            (coef[10] + (y * coef[15]))))));
                break;

            case 21 :
                prod[0] = y * (coef[4] + (y * (coef[8] + (y *
                        (coef[13] + (y * coef[19]))))));
                prod[1] = y * (coef[7] + (y * coef[11]));
                prod[2] = y * (coef[12] + (y * coef[16]));
                prod[3] = y * coef[18];
                prod[4] = y * (coef[3] + (y * (coef[6] + (y *
                            (coef[10] + (y * (coef[15] + (y * coef[21]))))))));
                break;
           }
       }
   }


#if 0   /* non utilise dans cette version */
/*fh=========================================================================
**
**  ADD_COEF
**
**  MARTIN A. BUSSIERES     VERSION ORIGINALE         10:21:53  1/10/1989
**===========================================================================*/

void add_coef(
/************/

int32_t   mapping,            /* 0: in -> out   1: out -> in    */
int32_t   xy,                 /* 0: XIND   1: YIND                    */
int32_t   no_coef,            /* numero du coefficient (1 - 21) */
double   val,               /* valeur du coefficient          */
pPOLYNOME pol)              /* polynome                       */
/*$END$*/

   {
    /* ajuster le nombre de coefficients */
    if(pol->ncoef[xy] < no_coef)
        pol->ncoef[xy] = no_coef;

    /* inscrire la valeur du coefficient */
    pol->coef[mapping][xy][no_coef - 1] = val;
   }



/*fh=========================================================================
**
**  GET_COEF
**
**  MARTIN A. BUSSIERES     VERSION ORIGINALE         10:21:53  1/10/1989
**===========================================================================*/

int32_t get_coef(
/*************/

int32_t     mapping,          /* 0: in -> out   1: out -> in    */
int32_t     xy,               /* 0: X   1: Y                    */
int32_t     no_coef,          /* numero du coefficient (1 - 21) */
double FAR *val,              /* valeur du coefficient          */
pPOLYNOME   pol)              /* polynome                       */
/*$END$*/

   {
    /* ajuster le nombre de coefficients */
    if(pol->ncoef[xy] < no_coef)
        return(0);

    /* lire la valeur du coefficient */
    *val = pol->coef[mapping][xy][no_coef - 1];

    return(1);
   }



/*fh=========================================================================
**
**  INIT_POLYNOME
**
**  MARTIN A. BUSSIERES     VERSION ORIGINALE         10:21:53  1/10/1989
**  DONALD MORISSETTE       ajouter une zone pour offset  05/06/90
**===========================================================================*/

void init_polynome(
/*****************/

pPOLYNOME pol)
/*$END$*/

   {
    int32_t i,j,k;

    for(i=0;i<2;i++)
       {
        pol->ncoef[i] = 0;

        for(j=0;j<2;j++)
        for(k=0;k<MAX_TERMES+1;k++)             /* +1 --> les offsets */
            pol->coef[i][j][k] = 0.0;
       }
   }


#endif
