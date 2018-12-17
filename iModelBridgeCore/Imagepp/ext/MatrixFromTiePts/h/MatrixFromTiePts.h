/*--------------------------------------------------------------------------------------+
|
|     $Source: ext/MatrixFromTiePts/h/MatrixFromTiePts.h $
|    $RCSfile: readImage.cpp,v $
|   $Revision: 1.1.2.16 $
|       $Date: 2010/02/19 14:13:19 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once


#define TRANSFO_NONE     -1 /* No transformation model */
#define TRANSFO_TRAN_2D   0 /* Translation en x et y */
#define TRANSFO_SIMI_2D   1 /* Translation x,y + Rotation + Echelle */
#define TRANSFO_POLY_2D_1 2 /* Polynomiale deg 1 */
#define TRANSFO_AFFI_2D   3 /* Tran. x,y + Rotation + Echelle + Axes */
#define TRANSFO_PROJ_2D   4 /* Projection d'un plan sur un autre */
#define TRANSFO_POLY_2D_2 5 /* Polynomiale deg 2 */
#define TRANSFO_POLY_2D_3 6 /* Polynomiale deg 3 */
#define TRANSFO_POLY_2D_4 7 /* Polynomiale deg 4 */ /* A tester ??? */
#define TRANSFO_POLY_2D_5 8 /* Polynomiale deg 5 */ /* A tester ??? */
#define TRANSFO_MAT_2D    9 /* Matrice de transformation */
#define TRANSFO_HELM_2D   10
#define TRANSFO_TPS_2D    11
#define TRANSFO_MAX       12

#define TRF_ERROR_MAX_ITERATIONS    -1
#define TRF_ERROR_NO_MEMORY         -2
#define TRF_ERROR_BAD_NUMBER_POINTS -3
#define TRF_ERROR_MATRIX            -4


long GetTransfoMatrixFromScaleAndTiePts (double          po_pMatrix[4][4],
                                         unsigned short  pi_NbVal_GeoTiePoint,
                                         double*         pi_pVal_GeoTiePoint,
                                         unsigned short  pi_NbVal_GeoPixelScale,
                                         double*         pi_pVal_GeoPixelScale);

long GetProjectiveTransfoMatrixFromScaleAndTiePts(double         po_pMatrix[4][4],
                                                  unsigned short pi_NbVal_GeoTiePoint,
                                                  double*        pi_pVal_GeoTiePoint);

long GetAffineTransfoMatrixFromScaleAndTiePts(double         po_pMatrix[4][4],
                                              unsigned short pi_NbVal_GeoTiePoint,
                                              double*        pi_pVal_GeoTiePoint);

long GetTransfoMatrixFromTiePts(double           po_pMatrix[4][4],
                                unsigned short   pi_NbVal_GeoTiePoint,
                                const double*    pi_pVal_GeoTiePoint,
                                long             pi_ModelType);

