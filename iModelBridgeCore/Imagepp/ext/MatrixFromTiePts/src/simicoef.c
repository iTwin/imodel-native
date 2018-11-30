/*--------------------------------------------------------------------------------------+
|
|     $Source: ext/MatrixFromTiePts/src/simicoef.c $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
/*mh================================================================ HMR
**  Octimage/MicroStation - Transformation Models
**
**      This file contains the functions relative to the computation
**      of the coefficients for the similitude transform
**
**===========================================================================*/

#include "trfmodel.h"


static void trf_prepSimiMat2D(pMAT_ mat,pMAT_ pts);

/*fh=========================================================================
**
**  trf_calcCoefSimi2D
**
**  Alain Lapierre  22/12/92
**===========================================================================*/



int32_t trf_calcCoefSimi2D(int32_t                code,
                         pTRANSFO_MODEL_DATA_3D model)
    {
    pMAT_ mat, ptsBad, ptsOk, sol;
    int32_t     nbPoints;
    double minBadX, minBadY, minOkX, minOkY;

    /*
    ** Validate the number of ctrl points required,
    ** find the size of the matrice and allocate it
    */

    nbPoints = trf_findNoActivePoints(&model->ctrlPts3D);
    if (N_COEF_SIMI_2D > nbPoints * 2)
        return(TRF_ERROR_BAD_NUMBER_POINTS);

    if ((mat = mat_create(nbPoints * 2, N_COEF_SIMI_2D)) == MAT_NULL)
        return(TRF_ERROR_NO_MEMORY);
    if ((ptsBad = mat_create(nbPoints * 2, 1)) == MAT_NULL)
        {
        mat_destroy(mat);
        return(TRF_ERROR_NO_MEMORY);
        }
    if ((ptsOk = mat_create(nbPoints * 2, 1)) == MAT_NULL)
        {
        mat_destroy(mat);
        mat_destroy(ptsBad);
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


    /*
    ** --------------------
    **   TRANSFO DIRECT
    ** --------------------
    */
    if ((code == TRANSFO_DIRECT) || (code == TRANSFO_BOTH))
       {
       trf_prepSimiMat2D(mat, ptsBad);

       /*
       ** Matrix resolution
       */
       if ((sol=mat_solve (mat,ptsOk)) == MAT_NULL)
               {
           mat_destroy(mat);
           mat_destroy(ptsBad);
           mat_destroy(ptsOk);
               return (TRF_ERROR_MATRIX);
               }

       /*
       ** Put back the coefficients in the model structure in good order
       */
       model->coef3D.coef[TRANSFO_DIRECT][XIND][1] = sol->mat_value[0];
       model->coef3D.coef[TRANSFO_DIRECT][XIND][2] = sol->mat_value[1];
       model->coef3D.coef[TRANSFO_DIRECT][XIND][3] = sol->mat_value[2];

       model->coef3D.coef[TRANSFO_DIRECT][YIND][1] = -sol->mat_value[1];
       model->coef3D.coef[TRANSFO_DIRECT][YIND][2] = sol->mat_value[0];
       model->coef3D.coef[TRANSFO_DIRECT][YIND][3] = sol->mat_value[3];

       model->coef3D.coef[TRANSFO_DIRECT][ZIND][0] = 0.0;
       model->coef3D.coef[TRANSFO_DIRECT][ZIND][1] = 0.0;
       model->coef3D.coef[TRANSFO_DIRECT][ZIND][2] = 0.0;
       model->coef3D.coef[TRANSFO_DIRECT][ZIND][3] = 0.0;

       model->coef3D.nbCoef[XIND] = N_COEF_SIMI_2D_X;
       model->coef3D.nbCoef[YIND] = N_COEF_SIMI_2D_Y;
       model->coef3D.nbCoef[ZIND] = 0;
       mat_destroy(sol);
       }

    /*
    ** --------------------
    **   TRANSFO INVERSE
    ** --------------------
    */
    if ((code == TRANSFO_INVERSE) || (code == TRANSFO_BOTH))
       {
       trf_prepSimiMat2D(mat,ptsOk);

       /*
       ** Matrix resolution
       */
       if ((sol=mat_solve (mat,ptsBad)) == MAT_NULL)
               {
           mat_destroy(mat);
           mat_destroy(ptsBad);
           mat_destroy(ptsOk);
               return (TRF_ERROR_MATRIX);
               }


       /*
       ** Put back the coefficients in the model structure in good order
       */
       model->coef3D.coef[TRANSFO_INVERSE][XIND][1] = sol->mat_value[0];
       model->coef3D.coef[TRANSFO_INVERSE][XIND][2] = sol->mat_value[1];
       model->coef3D.coef[TRANSFO_INVERSE][XIND][3] = sol->mat_value[2];

       model->coef3D.coef[TRANSFO_INVERSE][YIND][1] = -sol->mat_value[1];
       model->coef3D.coef[TRANSFO_INVERSE][YIND][2] = sol->mat_value[0];
       model->coef3D.coef[TRANSFO_INVERSE][YIND][3] = sol->mat_value[3];

       model->coef3D.coef[TRANSFO_INVERSE][ZIND][0] = 0.0;
       model->coef3D.coef[TRANSFO_INVERSE][ZIND][1] = 0.0;
       model->coef3D.coef[TRANSFO_INVERSE][ZIND][2] = 0.0;
       model->coef3D.coef[TRANSFO_INVERSE][ZIND][3] = 0.0;

       model->coef3D.nbCoef[XIND] = N_COEF_SIMI_2D_X;
       model->coef3D.nbCoef[YIND] = N_COEF_SIMI_2D_Y;
       model->coef3D.nbCoef[ZIND] = 0;
       mat_destroy(sol);
       }

    mat_destroy(mat);
    mat_destroy(ptsBad);
    mat_destroy(ptsOk);
    return (HSUCCESS);
   }


/*sh=========================================================================
**
**  trf_prepSimiMat2D
**
**  DESCRIPTION
**
**      This function prepares the matrix for resolution using Choleski.
**      The matrix has the form:
**             -         -
**            | x  y 1 0  |
**            | y -x 0 1  |
**            | .....     |
**            | .....     |
**             -         -
**
**
**  RETURN VALUE
**      None
**
**  SEE ALSO
**      trf_calcCoefSimi2D
**
**
**  --- Programmeur ---     --- Travail fait ---      ------- Date -------
**  Alain Lapierre          VERSION ORIGINALE         23/12/92
**===========================================================================*/

static void trf_prepSimiMat2D(
pMAT_ mat,
pMAT_ pts)
   {
   int32_t src,dest;

   for(src=0,dest=0; dest < mat->rows; src+=2)
     {
     mat_v(mat,dest,0) = mat_v(pts,src,0);  /* X */
     mat_v(mat,dest,1) = mat_v(pts,src+1,0);/* Y */
     mat_v(mat,dest,2) = 1.0;
     mat_v(mat,dest,3) = 0.0;
     dest++;
     mat_v(mat,dest,0) = mat_v(pts,src+1,0);/* Y */
     mat_v(mat,dest,1) = -mat_v(pts,src,0); /* -X*/
     mat_v(mat,dest,2) = 0.0;
     mat_v(mat,dest,3) = 1.0;
     dest++;
     }
   }
