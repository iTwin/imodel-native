/*--------------------------------------------------------------------------------------+
|
|     $Source: ext/MatrixFromTiePts/src/polycoef.c $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
/*mh================================================================ HMR
**  Octimage/MicroStation - Transformation Models
**
**      This file contains the functions relative to the computation
**      of the coefficients for the polynomial transform (deg 1 to 5)
**
**===========================================================================*/

#include "trfmodel.h"


static  int32_t trf_trianglePascal (int32_t );
static  int32_t trf_calcCoefPoly   (int32_t, pLIST, pCOEFFICIENTS_3D ,int32_t);


/*fh=========================================================================
**
**  trf_calcCoefPoly2D1
**
**  Alain Lapierre  18/12/92
**===========================================================================*/



int32_t trf_calcCoefPoly2D1(int32_t                   code,
                          pTRANSFO_MODEL_DATA_3D    model)
    {
    int32_t ret;

    ret= trf_calcCoefPoly (code,&model->ctrlPts3D,&model->coef3D,1);

    return(ret);
    }


/*fh=========================================================================
**
**  trf_calcCoefPoly2D2
**
**  Alain Lapierre  18/12/92    Original version
**===========================================================================*/



int32_t trf_calcCoefPoly2D2(int32_t                   code,
                          pTRANSFO_MODEL_DATA_3D    model)
    {
    int32_t ret;

    ret= trf_calcCoefPoly (code,&model->ctrlPts3D,&model->coef3D,2);

    return(ret);
    }



/*fh=========================================================================
**
**  trf_calcCoefPoly2D3
**
**  Alain Lapierre  18/12/92    Original version
**===========================================================================*/


int32_t trf_calcCoefPoly2D3(int32_t                   code,
                          pTRANSFO_MODEL_DATA_3D    model)
    {
    int32_t ret;

    ret= trf_calcCoefPoly (code,&model->ctrlPts3D,&model->coef3D,3);

    return(ret);
    }



/*fh=========================================================================
**
**  trf_calcCoefPoly2D4
**
**  Alain Lapierre  18/12/92    Original version
**===========================================================================*/


int32_t trf_calcCoefPoly2D4(int32_t                code,
                          pTRANSFO_MODEL_DATA_3D model)
    {
    int32_t ret;

    ret= trf_calcCoefPoly (code,&model->ctrlPts3D,&model->coef3D,4);

    return(ret);
    }



/*fh=========================================================================
**
**  trf_calcCoefPoly2D5
**
**  Alain Lapierre  18/12/92    Original version
**===========================================================================*/



int32_t trf_calcCoefPoly2D5(int32_t                code,
                          pTRANSFO_MODEL_DATA_3D model)
    {
    int32_t ret;

    ret= trf_calcCoefPoly (code,&model->ctrlPts3D,&model->coef3D,5);

    return(ret);
    }


/*sh=========================================================================
**
**  trf_calcCoefPoly
**
**  INCLUDE
**
**      "trfmodel.h"
**
**  DESCRIPTION
**
**      Cette fonction calcule les coefficients des polynomes a "X" termes
**      etant donnee les paires de points de controle 'points'.  Les
**      coefficients des polynomes sont places dans 'coe' (Forward et
**      Backward mapping,  pour X et Y).
**
**      La forme des polynomes est:
**
**                                             2      2
**      XX  = a  +  a X  + a Y  +  a XY  +  a X  + a Y
**             0     1      2       3        4      5
**
**                                             2      2
**      YY  = b  +  b X  + b Y  +  b XY  +  b X  + b Y
**             0     1      2       3        4      5
**
**
**
**      Les mappings calcules dependent de 'code':
**
**      0: TRANSFO_DIRECT
**      1: TRANSFO_INVERSE
**      2: TRANSFO_BOTH
**
**
**  PARAMETERS
**
**      code
**      points  - liste des listes de points
**      coe     - coefficients du polynome
**      degpol  - degre du polynome recherche
**
**
**  RETURN VALUE
**
**       HSUCCESS: Ok
**       TRF_ERROR_MAX_ITERATIONS :Nombre maximal d'iterations atteind
**                                 pour au moins un polynome
**       TRF_ERROR_NO_MEMORY:      Pas assez de memoire
**       TRF_ERROR_BAD_NUMBER_POINTS: Pas assez de points de controle
**       TRF_ERROR_MATRIX           : Resolution matricielle non possible
**
**  SEE ALSO
**       trf_transfoCoordPoly2D
**
**
**  Author                      Work done                       Date
**  MARTIN A. BUSSIERES     VERSION ORIGINALE         15:23:37  6/1/1989
**  DONALD MORISSETTE       MODIFIER ORDRE DES COEF   28-05-90
**  ALAIN LAPIERRE          modifie les types de var
**                          pour etre standard stdtypes.h
**                          modifie les struct de donnees
**                                      pour etre standard Octimage 4   17 dec. 1992
**                                      change code de retour en erreur VOIR AL 18/12/92
**                         modifie resolution de la matrice
**                         plutot que resoud on appelle mat_solve
**                         qui fait une Choleski                    3/2/93
**===========================================================================*/


static int32_t trf_calcCoefPoly (
/**************/

int32_t            code,
pLIST              points,   /* liste des listes de points */
pCOEFFICIENTS_3D   coe,      /* coefficients du polynome */
int32_t            degpol)   /* degre du polynome recherche */
/*$END$*/

   {
    int32_t  i, j, m, l, ii;
    pMAT_ mat, pts, sol, ptsActive;
    double  xx = 0.0;
    double  yy = 0.0;
    double  px = 0.0;
    double  py = 0.0;
    int32_t     nbterme, vecsize;
    double minBadX, minBadY, minOkX, minOkY;

    /*
    ** -Trouve
    **     -le nombre de termes du polynome (nombre de coefficients)
    **     -le nombre d'elements du vecteur
    **     -le nombre d'elements de la matrice
    */
    nbterme = trf_trianglePascal (degpol);
    vecsize = trf_findNoActivePoints(points);

    if (nbterme > vecsize)
          return(TRF_ERROR_BAD_NUMBER_POINTS);
    /*
    **
    */

    if ((mat = mat_create(vecsize, nbterme)) == MAT_NULL)
        return(TRF_ERROR_NO_MEMORY);
    if ((pts = mat_create(vecsize, 1)) == MAT_NULL)
        {
        mat_destroy(mat);
        return(TRF_ERROR_NO_MEMORY);
        }
    if ((ptsActive = mat_create(vecsize * 2, 1)) == MAT_NULL)
        {
        mat_destroy(mat);
        mat_destroy(pts);

        return(TRF_ERROR_NO_MEMORY);
        }

    /*
    **  set le nbr de terme du polynome pour X et Y (+1 pour la translation)
    */
    coe->nbCoef[XIND] = nbterme+1;
    coe->nbCoef[YIND] = nbterme+1;
    coe->nbCoef[ZIND] = 0;

    /*
    ** --------------------
    **   TRANSFO DIRECT
    ** --------------------
    */

    trf_copyActivePoints(ptsActive, points, PTS_BAD, COPY_XY, &minBadX, &minBadY);
    coe->coef [TRANSFO_DIRECT][XIND][0]=minBadX;
    coe->coef [TRANSFO_DIRECT][YIND][0]=minBadY;

    if ((code == TRANSFO_DIRECT) || (code == TRANSFO_BOTH))
       {
        /*
        **  En X
        */
        for (i = 0, j = 0;  i < vecsize;  i++)
           {
            xx = ptsActive->mat_value[i*2];
            yy = ptsActive->mat_value[i*2+1];

            for (l = 0;  l <= degpol;  l++)
               {
                for (m = 0;  m <= l;  m++, j++)
                   {
                    if ((xx == 0.0) && ((l-m) == 0))
                        px = 1.0;
                    else
                        px = pow (xx, (double)(l-m));

                    if ((yy == 0.0) && ((m) == 0))
                        py = 1.0;
                    else
                        py = pow (yy, (double)(m));

                    mat->mat_value[j] = px * py;
                   }
                /*
                ** Replace les coef.
                ** in mat : a ax ay ax2 axy ay2 ax3 ax2y axy2 ay3 ....
                **                  ---         ---
                ** out mat: a ax ay axy ax2 ay2 ax2y axy2 ax3 ay3 ....
                **                      ---               ---
                */
                if (l >= 2)
                   {
                    m  = trf_trianglePascal (l) - l - 1;
                    m  += nbterme * i;
                    px = mat->mat_value[m];
                    for (ii=m; ii<m+l-1; ii++)
                        mat->mat_value[ii] = mat->mat_value[ii+1];
                    mat->mat_value[ii] = px;
                   };
               }

           }

        trf_copyActivePoints(pts, points, PTS_OK, COPY_X, &minOkX, &minOkY);
        coe->coef [TRANSFO_INVERSE][XIND][0]=minOkX;

        /*
        ** Matrix resolution
        */
        if ((sol=mat_solve (mat,pts)) == MAT_NULL)
                 {
             mat_destroy(mat);
             mat_destroy(pts);
             mat_destroy(ptsActive);
                 return (TRF_ERROR_MATRIX);
                 }

        for (i = 0;  i < nbterme;  i++)
           coe->coef [TRANSFO_DIRECT][XIND][i+1] = sol->mat_value[i];
        mat_destroy(sol);

        /*
        **  En Y
        */

        trf_copyActivePoints(pts, points, PTS_OK, COPY_Y, &minOkX, &minOkY);
        coe->coef [TRANSFO_INVERSE][YIND][0]=minOkY;

        /*
        ** Matrix resolution
        */
        if ((sol=mat_solve (mat,pts)) == MAT_NULL)
                 {
             mat_destroy(mat);
             mat_destroy(pts);
             mat_destroy(ptsActive);
                 return (TRF_ERROR_MATRIX);
                 }

        for (i = 0;  i < nbterme;  i++)
            coe->coef [TRANSFO_DIRECT][YIND][i+1] = sol->mat_value[i];


        /*
        **  En Z, mise a zero parce que la structure est prevue pour un Z
        */

        for (i = 0;  i < nbterme+1;  i++)
            coe->coef [TRANSFO_DIRECT][ZIND][i] = 0.0;

        mat_destroy(sol);
       }



    /*
    ** --------------------
    **   TRANSFO INVERSE
    ** --------------------
    */
    trf_copyActivePoints(ptsActive, points, PTS_OK, COPY_XY, &minOkX, &minOkY);
    coe->coef [TRANSFO_INVERSE][XIND][0]=minOkX;
    coe->coef [TRANSFO_INVERSE][YIND][0]=minOkY;

    if ((code == TRANSFO_INVERSE) || (code == TRANSFO_BOTH))
       {
        /*
        **  En X
        */
        for (i = 0, j = 0;  i < vecsize;  i++)
           {
            xx = ptsActive->mat_value[i*2];
            yy = ptsActive->mat_value[i*2+1];

            for (l = 0;  l <= degpol;  l++)
               {
                for (m = 0;  m <= l;  m++, j++)
                   {
                    if ((xx == 0.0) && ((l-m) == 0))
                        px = 1.0;
                    else
                        px = pow (xx, (double)(l-m));

                    if ((yy == 0.0) && ((m) == 0))
                        py = 1.0;
                    else
                        py = pow (yy, (double)(m));

                    mat->mat_value[j] = px * py;
                   }
                /*
                ** Replace les coef.
                ** in mat : a ax ay ax2 axy ay2 ax3 ax2y axy2 ay3 ....
                **                  ---         ---
                ** out mat: a ax ay axy ax2 ay2 ax2y axy2 ax3 ay3 ....
                **                      ---               ---
                */
                if (l >= 2)
                   {
                           m  = trf_trianglePascal (l) - l - 1;
                    m  += nbterme * i;
                    px = mat->mat_value[m];
                    for (ii=m; ii<m+l-1; ii++)
                        mat->mat_value[ii] = mat->mat_value[ii+1];
                    mat->mat_value[ii] = px;
                   };

               }

           }

        trf_copyActivePoints(pts, points, PTS_BAD, COPY_X, &minBadX, &minBadY);
        coe->coef [TRANSFO_DIRECT][XIND][0]=minBadX;

        /*
        ** Matrix resolution
        */
        if ((sol=mat_solve (mat,pts)) == MAT_NULL)
                 {
             mat_destroy(mat);
             mat_destroy(pts);
             mat_destroy(ptsActive);
                 return (TRF_ERROR_MATRIX);
                 }

        for (i = 0;  i < nbterme;  i++)
            coe->coef [TRANSFO_INVERSE][XIND][i+1] = sol->mat_value[i];
        mat_destroy(sol);

        /*
        **  En Y
        */
        trf_copyActivePoints(pts, points, PTS_BAD, COPY_Y, &minBadX, &minBadY);
        coe->coef [TRANSFO_DIRECT][YIND][0]=minBadY;

        /*
        ** Matrix resolution
        */
        if ((sol=mat_solve (mat,pts)) == MAT_NULL)
                 {
             mat_destroy(mat);
             mat_destroy(pts);
             mat_destroy(ptsActive);
                 return (TRF_ERROR_MATRIX);
                 }

        for (i = 0;  i < nbterme;  i++)
            coe->coef [TRANSFO_INVERSE][YIND][i+1] = sol->mat_value[i];

        /*
        **  En Z, mise a zero parce que la structure est prevue pour un Z
        */

        for (i = 0;  i < nbterme+1;  i++)
            coe->coef [TRANSFO_INVERSE][ZIND][i] = 0.0;

        mat_destroy(sol);
       }

    mat_destroy(mat);
    mat_destroy(pts);
    mat_destroy(ptsActive);
    return (HSUCCESS); /* Ok ! */ /* Change de 1 a 0, AL 18/12/92 */
   }


/*sh=========================================================================
**
**  trf_trianglePascal (n)
**
**  DESCRIPTION
**
**
**  VALEUR RETOURNEE
**
**
**  --- Programmeur ---     --- Travail fait ---      ------- Date -------
**  MARTIN A. BUSSIERES     VERSION ORIGINALE         15:23:37  6/1/1989
**  ALAIN LAPIERRE          modifie les types de var
**                          pour etre standard stdtypes.h
**                          modifie les struct de donnees
**                                      pour etre standard Octimage 4   17 dec. 1992
*************************************************************************/

static
int32_t trf_trianglePascal (
/*********************/

int32_t n)

   {
    int32_t i, ret;

    ret = 0;
    for (i = 0;  i <= n;  i++)
        ret += i + 1;

    return (ret);
   }
