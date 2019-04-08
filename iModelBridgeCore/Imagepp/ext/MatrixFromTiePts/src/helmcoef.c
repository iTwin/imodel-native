/*--------------------------------------------------------------------------------------+
|
|     $Source: ext/MatrixFromTiePts/src/helmcoef.c $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
/*mh================================================================ HMR
**  Octimage/MicroStation - Transformation Models
**
**      This file contains the functions relative to the computation
**      of the coefficients for the helmlitude transform
**
**===========================================================================*/

#include "trfmodel.h"


static void trf_prepHelmMatA2D(pMAT_   mat,
                                pMAT_   pts,
                                double sinTeta,
                                double cosTeta);

static void trf_prepHelmMatW2D(pMAT_   matW,
                                pMAT_   ptsSrc,
                                pMAT_   ptsDest,
                                double sinTeta,
                                double cosTeta,
                                double Tx,
                                double Ty);

/*fh=========================================================================
**
**  trf_calcCoefHelm2D
**
**  Alain Lapierre  May 94
**===========================================================================*/



int32_t trf_calcCoefHelm2D(int32_t                code,
                         pTRANSFO_MODEL_DATA_3D model)
    {
    pMAT_ matA, matW, ptsBad, ptsOk, sol=MAT_NULL;
    int32_t     nbPoints, iteration;
    double minBadX, minBadY, minOkX, minOkY;
    double Teta, Tx, Ty, sinTeta, cosTeta;
    DCOORD gravityCenter, firstPointBad,firstPointOk;

    /*
    ** Validate the number of ctrl points required,
    ** find the size of the matrice and allocate it
    */

    nbPoints = trf_findNoActivePoints(&model->ctrlPts3D);
    if (N_COEF_HELM_2D > nbPoints * 2)
        return(TRF_ERROR_BAD_NUMBER_POINTS);

    if ((matA = mat_create(nbPoints * 2, N_COEF_HELM_2D)) == MAT_NULL)
        return(TRF_ERROR_NO_MEMORY);
    if ((ptsBad = mat_create(nbPoints * 2, 1)) == MAT_NULL)
        {
        mat_destroy(matA);
        return(TRF_ERROR_NO_MEMORY);
        }
    if ((ptsOk = mat_create(nbPoints * 2, 1)) == MAT_NULL)
        {
        mat_destroy(matA);
        mat_destroy(ptsBad);
        return(TRF_ERROR_NO_MEMORY);
        }

    if ((matW = mat_create(nbPoints * 2, 1)) == MAT_NULL)
        {
        mat_destroy(matA);
        mat_destroy(ptsBad);
        mat_destroy(ptsOk);
        return(TRF_ERROR_NO_MEMORY);
        }

    /*
    ** Copy the points in matrix type structured
    ** and substract from those values the minimum values for
    ** each coordinates. The minimum value is placed in the first
    ** position of the coefficients vector and will be used when
    ** transforming coordinates to translate the result in the
    ** original system.
    */

    trf_copyActivePoints(ptsBad, &model->ctrlPts3D, PTS_BAD, COPY_XY, &minBadX, &minBadY);
    model->coef3D.coef[TRANSFO_DIRECT][XIND][0] = minBadX;
    model->coef3D.coef[TRANSFO_DIRECT][YIND][0] = minBadY;

    trf_copyActivePoints(ptsOk,  &model->ctrlPts3D, PTS_OK,  COPY_XY, &minOkX, &minOkY);
    model->coef3D.coef[TRANSFO_INVERSE][XIND][0] = minOkX;
    model->coef3D.coef[TRANSFO_INVERSE][YIND][0] = minOkY;

    gravityCenter.x = 0.0;
    gravityCenter.y = 0.0;
    gravityCenter.z = 0.0;
    firstPointBad.x = mat_v(ptsBad,0,0);
    firstPointBad.y = mat_v(ptsBad,1,0);
    firstPointBad.z = 0.0;
    firstPointOk.x  = mat_v(ptsOk,0,0);
    firstPointOk.y  = mat_v(ptsOk,1,0);
    firstPointOk.z  = 0.0;


    /*
    ** --------------------
    **   TRANSFO DIRECT
    ** --------------------
    */

    /*
    ** Find starting values for the 3 parameters: Teta, Tx, Ty
    ** Those estimations are necessary to help the model converge
    ** towards the solution.
    **
    ** Tx: 0.0 because the pts have already been recentered to the gravity center
    ** Ty: 0.0 because the pts have already been recentered to the gravity center
    ** Teta: angle between the 2 lines given by:
    **                 line 1 (gravity center (0,0) and the first ptBad) and
    **                 line 2 (gravity center (0,0) and the first ptOk)
    */

    Tx = 0.0;
    Ty = 0.0;
    Teta = angle(&gravityCenter,&firstPointBad) -
           angle(&gravityCenter,&firstPointOk);


    /*
    ** Do the iteration process to find Teta, Tx, Ty
    */

    if ((code == TRANSFO_DIRECT) || (code == TRANSFO_BOTH))
       {
       for(iteration=0; iteration < N_ITER_HELM_2D ; iteration++)
          {
          /*
          ** Preparation of matrix A and W before resolution
          */

          if(sol != MAT_NULL)
             mat_destroy(sol);

          sol=MAT_NULL;

          sinTeta = sin(Teta);
          cosTeta = cos(Teta);
          trf_prepHelmMatA2D(matA, ptsBad, sinTeta, cosTeta);
          trf_prepHelmMatW2D(matW, ptsBad, ptsOk, sinTeta, cosTeta, Tx, Ty);

          /*
          ** Matrix resolution
          */
          if ((sol=mat_solve (matA,matW)) == MAT_NULL)
               {
           mat_destroy(matA);
           mat_destroy(ptsBad);
           mat_destroy(ptsOk);
           mat_destroy(matW);
               return (TRF_ERROR_MATRIX);
               }

          /*
          ** Update the 3 parameters to make them converge toward the
          ** the solution. Those same 3 parameters will be reused in
          ** the next iteration.
          */

          Teta -= sol->mat_value[0];
          Tx   -= sol->mat_value[1];
          Ty   -= sol->mat_value[2];

          /*
          ** If the delta to apply to Teta if lesser than the tolerance,
          ** stop iterations.
          */

          if (fabs(sol->mat_value[0]) < model->toleranceRadian &&
              fabs(sol->mat_value[1]) < model->toleranceMeter  &&
              fabs(sol->mat_value[2]) < model->toleranceMeter)
              iteration= N_ITER_HELM_2D;

          }

       /*
       ** Put back the coefficients in the model structure in good order
       */

       sinTeta = sin(Teta);
       cosTeta = cos(Teta);

       model->coef3D.coef[TRANSFO_DIRECT][XIND][1] = cosTeta;
       model->coef3D.coef[TRANSFO_DIRECT][XIND][2] = sinTeta;
       model->coef3D.coef[TRANSFO_DIRECT][XIND][3] = Tx;

       model->coef3D.coef[TRANSFO_DIRECT][YIND][1] = -sinTeta;
       model->coef3D.coef[TRANSFO_DIRECT][YIND][2] = cosTeta;
       model->coef3D.coef[TRANSFO_DIRECT][YIND][3] = Ty;

       model->coef3D.coef[TRANSFO_DIRECT][ZIND][0] = 0.0;
       model->coef3D.coef[TRANSFO_DIRECT][ZIND][1] = 0.0;
       model->coef3D.coef[TRANSFO_DIRECT][ZIND][2] = 0.0;
       model->coef3D.coef[TRANSFO_DIRECT][ZIND][3] = 0.0;

       model->coef3D.nbCoef[XIND] = N_COEF_HELM_2D_X;
       model->coef3D.nbCoef[YIND] = N_COEF_HELM_2D_Y;
       model->coef3D.nbCoef[ZIND] = 0;

       if(sol != MAT_NULL)
          mat_destroy(sol);
       sol=MAT_NULL;
       }

    /*
    ** --------------------
    **   TRANSFO INVERSE
    ** --------------------
    */
    /*
    ** Find starting values for the 3 parameters: Teta, Tx, Ty
    ** Those estimations are necessary to help the model converge
    ** towards the solution.
    **
    ** Tx: 0.0 because the pts have already been recentered to the gravity center
    ** Ty: 0.0 because the pts have already been recentered to the gravity center
    ** Teta: angle between the 2 lines given by:
    **                 line 1 (gravity center (0,0) and the first ptBad) and
    **                 line 2 (gravity center (0,0) and the first ptOk)
    */

    Tx = 0.0;
    Ty = 0.0;
    Teta = angle(&gravityCenter,&firstPointOk) -
           angle(&gravityCenter,&firstPointBad);


    /*
    ** Do the iteration process to find Teta, Tx, Ty
    */

    if ((code == TRANSFO_DIRECT) || (code == TRANSFO_BOTH))
       {
       for(iteration=0; iteration < N_ITER_HELM_2D; iteration++)
          {
          /*
          ** Preparation of matrix A and W before resolution
          */

          if(sol != MAT_NULL)
             mat_destroy(sol);
          sol=MAT_NULL;

          sinTeta = sin(Teta);
          cosTeta = cos(Teta);
          trf_prepHelmMatA2D(matA, ptsOk, sinTeta, cosTeta);
          trf_prepHelmMatW2D(matW, ptsOk, ptsBad, sinTeta, cosTeta, Tx, Ty);

          /*
          ** Matrix resolution
          */
          if ((sol=mat_solve (matA,matW)) == MAT_NULL)
               {
           mat_destroy(matA);
           mat_destroy(ptsBad);
           mat_destroy(ptsOk);
           mat_destroy(matW);
               return (TRF_ERROR_MATRIX);
               }

          /*
          ** Update the 3 parameters to make them converge toward the
          ** the solution. Those same 3 parameters will be reused in
          ** the next iteration.
          */

          Teta -= sol->mat_value[0];
          Tx   -= sol->mat_value[1];
          Ty   -= sol->mat_value[2];

          /*
          ** If the delta to apply to Teta if lesser than the tolerance,
          ** stop iterations.
          */

          if (fabs(sol->mat_value[0]) < model->toleranceRadian &&
              fabs(sol->mat_value[1]) < model->toleranceMeter  &&
              fabs(sol->mat_value[2]) < model->toleranceMeter)
              iteration= N_ITER_HELM_2D;

          }

       /*
       ** Put back the coefficients in the model structure in good order
       */

       sinTeta = sin(Teta);
       cosTeta = cos(Teta);

       model->coef3D.coef[TRANSFO_INVERSE][XIND][1] = cosTeta;
       model->coef3D.coef[TRANSFO_INVERSE][XIND][2] = sinTeta;
       model->coef3D.coef[TRANSFO_INVERSE][XIND][3] = Tx;

       model->coef3D.coef[TRANSFO_INVERSE][YIND][1] = -sinTeta;
       model->coef3D.coef[TRANSFO_INVERSE][YIND][2] = cosTeta;
       model->coef3D.coef[TRANSFO_INVERSE][YIND][3] = Ty;

       model->coef3D.coef[TRANSFO_INVERSE][ZIND][0] = 0.0;
       model->coef3D.coef[TRANSFO_INVERSE][ZIND][1] = 0.0;
       model->coef3D.coef[TRANSFO_INVERSE][ZIND][2] = 0.0;
       model->coef3D.coef[TRANSFO_INVERSE][ZIND][3] = 0.0;

       model->coef3D.nbCoef[XIND] = N_COEF_HELM_2D_X;
       model->coef3D.nbCoef[YIND] = N_COEF_HELM_2D_Y;
       model->coef3D.nbCoef[ZIND] = 0;

       if(sol != MAT_NULL)
          mat_destroy(sol);
       sol=MAT_NULL;
       }


    mat_destroy(matA);
    mat_destroy(ptsBad);
    mat_destroy(ptsOk);
    mat_destroy(matW);
    return (HSUCCESS);
   }


/*sh=========================================================================
**
**  trf_prepHelmMatA2D
**
**  DESCRIPTION
**
**      This function prepares the matrix A for resolution using Choleski.
**      The matrix has the form:
**                    -              -
**                   | dFx    dFx dFx |
**           dF      | ---    --- --- |
**        A =-- =    | dTeta  dTx dTy |
**           dX      |                |
**                   | dFy    dFy dFy |
**                   | ---    --- --- |
**                   | dTeta  dTx dTy |
**                    -              -
**
**
**        A(0,0) = -x sin(Teta) + y cos(Teta)
**        A(0,1) = 1
**        A(0,2) = 0
**        A(1,0) = -x cos(Teta) - y sin(Teta)
**        A(1,1) = 0
**        A(1,2) = 1
**
**
**
**
**
**  RETURN VALUE
**      None
**
**  SEE ALSO
**      trf_calcCoefHelm2D
**
**
**  --- Programmeur ---     --- Travail fait ---      ------- Date -------
**  Alain Lapierre          VERSION ORIGINALE         May 94
**===========================================================================*/

static
void trf_prepHelmMatA2D(pMAT_   mat,
                         pMAT_   pts,
                         double sinTeta,
                         double cosTeta)
   {
   int32_t src,dest;

   for(src=0,dest=0; dest < mat->rows; src+=2)
     {
     mat_v(mat,dest,0) = - mat_v(pts,src,0)   * sinTeta +
                           mat_v(pts,src+1,0) * cosTeta;
     mat_v(mat,dest,1) = 1.0;
     mat_v(mat,dest,2) = 0.0;
     dest++;
     mat_v(mat,dest,0) = - mat_v(pts,src,0)   * cosTeta -
                           mat_v(pts,src+1,0) * sinTeta;
     mat_v(mat,dest,1) = 0.0;
     mat_v(mat,dest,2) = 1.0;
     dest++;
     }
   }


/*sh=========================================================================
**
**  trf_prepHelmMatW2D
**
**  DESCRIPTION
**
**      This function prepares the matrix W for resolution using Choleski.
**      The matrix has the form:
**                    -                            -
**                   | (x1 =( ax1 + by1 + c)) - x1  |
**                   |                              |
**       W = Fx - X  | (y1 =(-bx1 + ay1 + d)) - y1  |
**                   |                              |
**                   | (x2 =( ax2 + by2 + c)) - x2  |
**                   |                              |
**                   | (y2 =(-bx2 + ay2 + d)) - y2  |
**                   |                              |
**                   | .....                        |
**                   |                              |
**                   | .....                        |
**                    -                            -
**
**  RETURN VALUE
**      None
**
**  SEE ALSO
**      trf_calcCoefHelm2D
**
**
**  --- Programmeur ---     --- Travail fait ---      ------- Date -------
**  Alain Lapierre          VERSION ORIGINALE         May 94
**===========================================================================*/

static
void trf_prepHelmMatW2D(pMAT_      matW,
                         pMAT_      ptsSrc,
                         pMAT_      ptsDest,
                         double    sinTeta,
                         double    cosTeta,
                         double    Tx,
                         double    Ty)
  {
   int32_t i;
   double xx,yy;

   for(i=0; i < matW->rows; i+=2)
     {
     xx = mat_v(ptsSrc,i,0)   * cosTeta +
          mat_v(ptsSrc,i+1,0) * sinTeta + Tx;

     yy =-mat_v(ptsSrc,i,0)   * sinTeta +
          mat_v(ptsSrc,i+1,0) * cosTeta + Ty;


     mat_v(matW,i,0)   = xx - mat_v(ptsDest,i,0);
     mat_v(matW,i+1,0) = yy - mat_v(ptsDest,i+1,0);
     }
  }

