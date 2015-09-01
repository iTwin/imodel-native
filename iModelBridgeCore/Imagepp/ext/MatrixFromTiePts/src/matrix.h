/*--------------------------------------------------------------------------------------+
|
|     $Source: ext/MatrixFromTiePts/src/matrix.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
/*====================================================================
**
**  Operations matricielles.
**
**      Ce fichier contient les definitions relatives aux
**      operations matricielles. (matrix.c)
**
**  HMR FUNCTIONS
**         mat_solve
**         mat_create
**         mat_destroy
**         mat_dmxmul
**         mat_dpofa
**         mat_dposl
**         mat_dpodi
**         mat_ddot
**         mat_daxpy
**         mat_dscal
**         mat_print
**         mat_ltfill
**         mat_v
**         mat_ptr
**         MAT_NULL
**  END HMR FUNCTIONS
**
**  FICHIER
**      matrix.h
**
**  AUTEUR
**      Surveying Engineering (University of Calgary)   Sept. 1987
**      Alain Lapierre (Mise a jour standard ANSI)      Jan. 93
**====================================================================*/

#ifndef __MATRIX_H__
#define __MATRIX_H__

#include "oldhtypes.h"

#ifndef __HMR_MDL
#   if !defined(_INC_STDLIB)
#      include <malloc.h>
#   endif
#   include <math.h>
#endif

/******** Structures utilisees par matrix.c */

typedef struct tagMATRIX {
        int msize, rows, cols;
        }MATRIX;

typedef struct  tagMAT_ {
        int msize, rows, cols;
        double *mat_value;
        }MAT_;

typedef MAT_ * pMAT_;

/*====================================================================
** Prototypes des fonctions de matrix.c
**====================================================================*/
#if defined (__cplusplus)
extern "C" {
#endif


pMAT_  mat_solve   (pMAT_   a,
                    pMAT_   b);

pMAT_  mat_create  (int     r,
                    int     c);

pMAT_ mat_createFromRawData (int32_t   pi_dimX,
                             int32_t   pi_dimY,
                             double * pi_pRawData );

void   mat_destroy (pMAT_   a);

pMAT_  mat_dmxmul  (pMAT_   a,
                    pMAT_   b,
                    int     job);

int    mat_dpofa   (pMAT_   a);

int    mat_dposl   (pMAT_   a,
                               pMAT_   b);

double mat_dpodi   (pMAT_   a,
                    int     job);

double mat_ddot    (int     count,
                    double *ptr_x,
                    int     incx,
                    double *ptr_y,
                    int     incy );

int    mat_daxpy   (int     count,
                    double  a,
                    double *ptr_x,
                    int     incx,
                    double *ptr_y,
                    int     incy );

int    mat_dscal   (int     count,
                    double  a,
                    double *ptr_x,
                    int     incx );

void   mat_print   (pMAT_   a);

pMAT_  mat_ltfill  (pMAT_   a);

HSTATUS mat_setIdentity (pMAT_ pio_pMatrix);

int8_t mat_isNull      (pMAT_ pio_pMatrix);

pMAT_   mat_copy        (pMAT_ pi_pMatToCopy);

HSTATUS mat_copyData    (pMAT_ po_pOutputMatrix,
                         pMAT_ pi_pInputMatrix);

HSTATUS mat_qrSolve     (pMAT_ a, pMAT_ bx, pMAT_ by);




/******** Macro-substitutions requises par matrix.c */

#define mat_v(a,r,c)    ((pMAT_ )a)->mat_value[ (r) * (a->cols) + (c) ]
#define mat_ptr(a,r,c)  &( ((pMAT_ )a)->mat_value[ (r) * (a->cols) + (c) ] )
#define MAT_NULL        ((pMAT_ )0)



/* Definitions pour type de multiplications de matrices (mat_dmxmul) */

#define MULT_AXB    1
#define MULT_ATXB   2
#define MULT_AXBT   3
#define MULT_ATXBT  4

/* Definition pour type de calcule de la matrice dans la fonction (mat_dpodi)*/
#define MAT_DET_INV 1
#define MAT_INV     2
#define MAT_DET     3


#if defined (__cplusplus)
}
#endif

#endif /* __MATRIX_H__ */
