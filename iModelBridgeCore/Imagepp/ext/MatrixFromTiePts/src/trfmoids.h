/*--------------------------------------------------------------------------------------+
|
|     $Source: ext/MatrixFromTiePts/src/trfmoids.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
/*====================================================================
**  Octimage/MicroStation - Transformation Models
**
**      This module contains the define for the
**      transformation models functions.
**
**  DATA TYPES
**
**
**         ...
**
**  FILE
**      trfmoids.h
**
**====================================================================*/

#ifndef __TRFMOIDS_H__
#define __TRFMOIDS_H__

#include "..\h\MatrixFromTiePts.h"

#define N_PTS_NONE        0



/*
** Pour chaque type de transformation, on definit les dimensions des
** matrices necessaires et le nombre minimum de pts requis pour une
** solution unique.
**
**      ex: N_COEF_SIMI_2D      nb de coefficients a trouver. (ex: Tx,Ty,E,R)
**                              (i.e. longueur du vecteur X pour la resolution
**                                    par variation de parametres.)
**
**          N_COEF_SIMI_2D_X    longueur du vecteur contenant les coefficients
**                              pour X. A noter qu'un espace est conserve pour
**                              le centre de gravite. (ex: CG,a,b,c)
**
**          N_COEF_SIMI_2D_Y    longueur du vecteur contenant les coefficients
**                              pour X. A noter qu'un espace est conserve pour
**                              le centre de gravite. (ex: CG,-b,a,d)
**
**          N_PTS_SIMI_2D       nbre de points requis pour solution unique
**
**          N_ITER_HELM_2D      si le modele est resolu par iterations parce
**                              que non lineaire, on donne le nbre d'iterations
**                              souhaite.
*/

#define N_COEF_TRAN_2D    2
#define N_COEF_TRAN_2D_X  1
#define N_COEF_TRAN_2D_Y  1
#define N_PTS_TRAN_2D     1

#define N_COEF_HELM_2D    3
#define N_COEF_HELM_2D_X  4
#define N_COEF_HELM_2D_Y  4
#define N_PTS_HELM_2D     2
#define N_ITER_HELM_2D    30

#define N_COEF_SIMI_2D    4
#define N_COEF_SIMI_2D_X  4
#define N_COEF_SIMI_2D_Y  4
#define N_PTS_SIMI_2D     2

#define N_COEF_AFFI_2D    6
#define N_COEF_AFFI_2D_X  4
#define N_COEF_AFFI_2D_Y  4
#define N_PTS_AFFI_2D     3

#define N_COEF_PROJ_2D    8
#define N_COEF_PROJ_2D_X  6
#define N_COEF_PROJ_2D_Y  6
#define N_PTS_PROJ_2D     4

#define N_COEF_MAT_2D     6
#define N_COEF_MAT_2D_X   3
#define N_COEF_MAT_2D_Y   3
#define N_PTS_MAT_2D      0

#define N_COEF_TPS_2D     4
#define N_COEF_TPS_2D_X   4
#define N_COEF_TPS_2D_Y   4
#define N_PTS_TPS_2D      4

#define N_PTS_POLY_2D_1   3
#define N_PTS_POLY_2D_2   6
#define N_PTS_POLY_2D_3   10
#define N_PTS_POLY_2D_4   15
#define N_PTS_POLY_2D_5   21

#define N_YPROD_MAX       10

#define L_CONTROL_POINTS    32
#define L_COEFFICIENTS      32
#define L_MODEL_DESC        128

#define TRANSFO_DIRECT  0       /* From Uncorrected to Base */
#define TRANSFO_INVERSE 1       /* From Base to Uncorrected */
#define TRANSFO_BOTH    2

#define XIND 0
#define YIND 1
#define ZIND 2

#define PTS_BAD 0
#define PTS_OK  1

#define COPY_X      0x01
#define COPY_Y      0x02
#define COPY_Z      0x04
#define COPY_XY     COPY_X | COPY_Y
#define COPY_XYZ    COPY_X | COPY_Y | COPY_Z

/*
** Values for operation (trf_ctrlPtsModif())
*/

#define CTRL_PTS_MODIF_INSERT   1
#define CTRL_PTS_MODIF_APPEND   2
#define CTRL_PTS_MODIF_DELETE   3
#define CTRL_PTS_MODIF_SET      4
#define CTRL_PTS_MODIF_GET      5


typedef int TRFHANDLE;

/*
** Macros to specify if a specific model represents the default for a
** certain number of points when many models have the same number of points.
*/
#define TRF_DEFAULT     0
#define TRF_NON_DEFAULT 1





#endif /* __TRFMOIDS_H__ */
