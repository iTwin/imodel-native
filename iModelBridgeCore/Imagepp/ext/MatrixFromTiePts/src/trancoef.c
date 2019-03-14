/*--------------------------------------------------------------------------------------+
|
|     $Source: ext/MatrixFromTiePts/src/trancoef.c $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
/*mh================================================================ HMR
**  Octimage/MicroStation - Transformation Models
**
**      This file contains the functions relative to the computation
**      of the coefficients for the translation transform
**
**===========================================================================*/

#include "trfmodel.h"


/*fh=========================================================================
**
**  trf_calcCoefTran2D
**
**  Alain Lapierre  22/12/92     Original version
**===========================================================================*/



int32_t trf_calcCoefTran2D(int32_t                code,
                         pTRANSFO_MODEL_DATA_3D model)
    {
    int32_t nbPoints;
    pCONTROL_POINTS_3D ctrl;

    /* Validate the number of ctrl points required,
       find the size of the matrice and allocate it */

    nbPoints = trf_findNoActivePoints(&model->ctrlPts3D);
    if (nbPoints < N_COEF_TRAN_2D_X)
        return(TRF_ERROR_BAD_NUMBER_POINTS);

    ctrl=trf_getCtrlPtsAddr(&model->ctrlPts3D);

    /*
    ** --------------------
    **   TRANSFO DIRECT
    ** --------------------
    */
    if ((code == TRANSFO_DIRECT) || (code == TRANSFO_BOTH))
       {
       model->coef3D.coef[TRANSFO_DIRECT][XIND][0] =
                ctrl[0].pointOk.x - ctrl[0].pointBad.x;

       model->coef3D.coef[TRANSFO_DIRECT][YIND][0] =
                ctrl[0].pointOk.y - ctrl[0].pointBad.y;

       model->coef3D.coef[TRANSFO_DIRECT][ZIND][0] = 0.0;

       model->coef3D.nbCoef[XIND] = N_COEF_TRAN_2D_X;
       model->coef3D.nbCoef[YIND] = N_COEF_TRAN_2D_Y;
       model->coef3D.nbCoef[ZIND] = 0;
       }

    /*
    ** --------------------
    **   TRANSFO INVERSE
    ** --------------------
    */
    if ((code == TRANSFO_INVERSE) || (code == TRANSFO_BOTH))
       {
       model->coef3D.coef[TRANSFO_INVERSE][XIND][0] =
                ctrl[0].pointBad.x - ctrl[0].pointOk.x;

       model->coef3D.coef[TRANSFO_INVERSE][YIND][0] =
                ctrl[0].pointBad.y - ctrl[0].pointOk.y;

       model->coef3D.coef[TRANSFO_INVERSE][ZIND][0] = 0.0;

       model->coef3D.nbCoef[XIND] = N_COEF_TRAN_2D_X;
       model->coef3D.nbCoef[YIND] = N_COEF_TRAN_2D_Y;
       model->coef3D.nbCoef[ZIND] = 0;
       }

    return (HSUCCESS);
   }


