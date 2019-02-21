/*--------------------------------------------------------------------------------------+
|
|     $Source: ext/MatrixFromTiePts/src/matcoef.c $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
/*mh================================================================ HMR
**  Octimage/MicroStation - Transformation Models
**
**      This file contains the functions relative to the computation
**      of the coefficients for the Matrix transform
**
**===========================================================================*/

#include "trfmodel.h"


/*fh=========================================================================
**
**  trf_SetCoefAffi2D
**
**  Jean Papillon  November 1997
**===========================================================================*/
int32_t trf_SetCoefficientsMat2D   (pCOEFFICIENTS_3D        pi_pCoefs,
                                  pTRANSFO_MODEL_DATA_3D  po_pModel)
{
    int32_t Retour = HSUCCESS;
    int32_t iDimension;
    int32_t iCoef;
    double Divisor;

    /*
    **  Validate given data.
    **  - Number of coeficients to transform planar coordinates must be
    **    N_COEF_MAT_2D_X and N_COEF_MAT_2D_Y
    */
    if (pi_pCoefs->nbCoef [XIND] != N_COEF_MAT_2D_X || pi_pCoefs->nbCoef [YIND] != N_COEF_MAT_2D_Y)
    {
        Retour = HERROR;
        goto WRAPUP;
    }

    /*
    ** Copy number of coeficients
    */
    po_pModel->coef3D.nbCoef[0] = pi_pCoefs->nbCoef[0];
    po_pModel->coef3D.nbCoef[1] = pi_pCoefs->nbCoef[1];
    po_pModel->coef3D.nbCoef[2] = pi_pCoefs->nbCoef[2];

    /*
    **  Copy given direct model coeficients into model data
    */
    for (iDimension = 0; iDimension < 3; iDimension++)
        for (iCoef = 0; iCoef < pi_pCoefs->nbCoef [iDimension]; iCoef++)
            po_pModel->coef3D.coef [TRANSFO_DIRECT] [iDimension] [iCoef] =
                    pi_pCoefs->coef [TRANSFO_DIRECT] [iDimension] [iCoef];


    /*
    **  Compute inverse model coeficients into model data
    **  Digital Image Warping (p. 50)
    */
    Divisor = (pi_pCoefs->coef [TRANSFO_DIRECT] [XIND] [1] * pi_pCoefs->coef [TRANSFO_DIRECT] [YIND] [2]) -
              (pi_pCoefs->coef [TRANSFO_DIRECT] [XIND] [2] * pi_pCoefs->coef [TRANSFO_DIRECT] [YIND] [1]);

    po_pModel->coef3D.coef [TRANSFO_INVERSE] [XIND] [1] =
        pi_pCoefs->coef [TRANSFO_DIRECT] [YIND] [2] / Divisor;

    po_pModel->coef3D.coef [TRANSFO_INVERSE] [XIND] [2] =
        pi_pCoefs->coef [TRANSFO_DIRECT] [XIND] [2] / Divisor * -1.0;

    po_pModel->coef3D.coef [TRANSFO_INVERSE] [XIND] [0] =
        ((pi_pCoefs->coef [TRANSFO_DIRECT] [XIND] [2] * pi_pCoefs->coef [TRANSFO_DIRECT] [YIND] [0]) -
         (pi_pCoefs->coef [TRANSFO_DIRECT] [XIND] [0] * pi_pCoefs->coef [TRANSFO_DIRECT] [YIND] [2])) /
         Divisor;

    po_pModel->coef3D.coef [TRANSFO_INVERSE] [YIND] [1] =
        pi_pCoefs->coef [TRANSFO_DIRECT] [YIND] [1] / Divisor * -1.0;

    po_pModel->coef3D.coef [TRANSFO_INVERSE] [YIND] [2] =
        pi_pCoefs->coef [TRANSFO_DIRECT] [XIND] [1] / Divisor;

    po_pModel->coef3D.coef [TRANSFO_INVERSE] [YIND] [0] =
        ((pi_pCoefs->coef [TRANSFO_DIRECT] [XIND] [0] * pi_pCoefs->coef [TRANSFO_DIRECT] [YIND] [1]) -
         (pi_pCoefs->coef [TRANSFO_DIRECT] [XIND] [1] * pi_pCoefs->coef [TRANSFO_DIRECT] [YIND] [0])) /
         Divisor;

       po_pModel->coef3D.coef[TRANSFO_INVERSE][ZIND][0] = 0.0;
       po_pModel->coef3D.coef[TRANSFO_INVERSE][ZIND][1] = 0.0;
       po_pModel->coef3D.coef[TRANSFO_INVERSE][ZIND][2] = 0.0;

WRAPUP:
    return (Retour);
}
