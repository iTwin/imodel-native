/*--------------------------------------------------------------------------------------+
|
|     $Source: ext/MatrixFromTiePts/src/matrix.c $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
/*mh================================================================ HMR
**
**  Operations matricielles.
**
**      Ce fichier contient les fonctions relatives aux operations matricielles
**      et a la resolution de systemes d'equation (methode Choleski)
**
**===========================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "matrix.h"
#include "libgeom.h"

#define SIGN(a, b) ((b) >= 0.0 ? fabs(a) : -fabs(a))
#define SQR(a)     ((a) * (a))

HSTATUS mat_qrsolv ( pMAT_ a, pMAT_ c, pMAT_ d, pMAT_ b);
HSTATUS mat_qrdcmp (pMAT_ a, pMAT_ c, pMAT_ d, int32_t *sing);
HSTATUS mat_rsolv  (pMAT_ a, pMAT_ d, pMAT_ b);

/*fh=========================================================================
**
**  mat_solve
**
**  Alain Lapierre  Version originale                         27/1/93
**
**===========================================================================*/

pMAT_ mat_solve(

pMAT_ a,
pMAT_ b)
  {
  pMAT_ ata, atb;

  /* Transposee de a * a */
  if ((ata = mat_dmxmul(a,a,MULT_ATXB)) == MAT_NULL)
     return(MAT_NULL);

  /* Transposee de a * b */
  if ((atb = mat_dmxmul(a,b,MULT_ATXB)) == MAT_NULL)
     {
     mat_destroy(ata);
     return(MAT_NULL);
     }

  if (mat_dpofa(ata) != 0)
     {
     mat_destroy(ata);
     mat_destroy(atb);
     return(MAT_NULL);
     }


  if(mat_dposl(ata, atb) != 0)
     {
     mat_destroy(ata);
     mat_destroy(atb);
     return(MAT_NULL);
     }

  mat_destroy(ata);
  return(atb);
  }

/*fh=========================================================================
**
**  mat_dmxmul
**
**  Ashley Tam (UC) C version of the Ensulib by Mike Mepham   Sept. 87
**  Alain Lapierre  Mise a jour pour standard ANSI C          27/1/93\
**
**===========================================================================*/

pMAT_  mat_dmxmul(

pMAT_ a,
pMAT_ b,
int   job)

{
   pMAT_ c;
   double    s;
   int       i, j, k;

   switch ( job )
   {
   case MULT_AXB:     /*    A  *  B    */
      if ( a->cols != b->rows )
         return( MAT_NULL );
      else
      {
         if ((c = mat_create(a->rows, b->cols )) == MAT_NULL)
           return(MAT_NULL);
         for ( i = 0; i < a->rows; ++i )
         {
            for ( j = 0; j < b->cols; ++j )
            {
               s  = 0.0e0;
               for ( k = 0; k < a->cols; ++k )
               {
                  s += mat_v( a, i, k ) * mat_v( b, k, j );
               }
               mat_v( c, i, j ) = s;
            }
         }
         return( c );
      }
   case MULT_ATXB:     /*    At  *  B    */
      if ( a->rows != b->rows )
         return( MAT_NULL );
      else
      {
         if ((c= mat_create(a->cols, b->cols )) == MAT_NULL)
           return(MAT_NULL);
         for ( i = 0; i < a->cols; ++i )
         {
            for ( j = 0; j < b->cols; ++j )
            {
               s  = 0.0e0;
               for ( k = 0; k < a->rows; ++k )
               {
                  s += mat_v( a, k, i ) * mat_v( b, k, j );
               }
               mat_v( c, i, j ) = s;
            }
         }
         return( c );
      }
   case MULT_AXBT:     /*    A  *  Bt    */
      if ( a->cols != b->cols )
         return( MAT_NULL );
      else
      {
         if ((c = mat_create(a->rows, b->rows )) == MAT_NULL)
           return(MAT_NULL);
         for ( i = 0; i < a->rows; ++i )
         {
            for ( j = 0; j < b->rows; ++j )
            {
               s  = 0.0e0;
               for ( k = 0; k < a->cols; ++k )
               {
                  s += mat_v( a, i, k ) * mat_v( b, j, k );
               }
               mat_v( c, i, j ) = s;
            }
         }
         return( c );
      }
   case MULT_ATXBT:     /*    At  *  Bt    */
      if ( a->rows != b->cols )
         return( MAT_NULL );
      else
      if ((c = mat_create( a->cols, b->rows )) == MAT_NULL)
           return(MAT_NULL);
      {
         for ( i = 0; i < a->cols; ++i )
         {
            for ( j = 0; j < b->rows; ++j )
            {
               s  = 0.0e0;
               for ( k = 0; k < a->rows; ++k )
               {
                  s += mat_v( a, k, i ) * mat_v( b, j, k );
               }
               mat_v( c, i, j ) = s;
            }
         }
         return( c );
      }
   default:
      return( MAT_NULL );
   }
}

/*fh=========================================================================
**
**  mat_dpofa
**
**  Cleve Moler     Univ. Nouveau-Mexique (Coyright LINPACK)  08/14/78
**  Ashley Tam (UC) C version                                 Sept. 87
**  Alain Lapierre  Mise a jour pour standard ANSI C          27/1/93
**
**===========================================================================*/


int mat_dpofa (

pMAT_ a)

{
    int     row, col;
    double  s, t;
    int     info, inc;

    inc = a->cols;

    mat_v( a, 0, 0 ) = sqrt( mat_v( a, 0, 0 ) );

    for ( col = 1; col < a->cols; ++col )
    {
       info = col;
       s    = 0.0e0;

       for ( row = 0; row < col ; ++row )
       {
          t = mat_ddot( row, mat_ptr( a, 0, row), inc, mat_ptr( a, 0, col ), inc );
          t = mat_v( a, row, col ) - t;

          if(mat_v( a, row, row ) == 0.0)
              return(1);

          t = t / mat_v( a, row, row );
          mat_v( a, row, col ) = t;
          s = s + t * t;
       }

       s = mat_v( a, col, col ) - s;

       if ( s > 0.0e0 )
          mat_v( a, col, col ) = sqrt( s );
       else
          return( info );
    }
    info = 0;
    return ( info );
}

/*fh=========================================================================
**
**  mat_dposl
**
**  Cleve Moler     Univ. Nouveau-Mexique (Coyright LINPACK)  08/14/78
**  Ashley Tam (UC) C version                                 Sept. 87
**  Alain Lapierre  Mise a jour pour standard ANSI C          27/1/93
**
**===========================================================================*/

int mat_dposl (

pMAT_ a,
pMAT_ b)

{
    int    i, inc;
    double t;
    double          s;

    inc     = a->cols;

    for ( i = 0; i < a->rows; ++i )
    {
       t = mat_ddot( i, mat_ptr(a,0,i), inc, mat_ptr(b,0,0), 1 );


       if(mat_v( a, i, i ) == 0.0)
           return(1);

       mat_v( b, i, 0 ) = ( mat_v( b, i, 0 ) - t ) / mat_v( a, i, i );
    }

    for ( i = (a->rows -1); i >= 0; --i )
    {
       if(mat_v( a, i, i ) == 0.0)
           return(1);

       mat_v( b, i, 0 ) = mat_v( b, i, 0 ) / mat_v( a, i, i );
       s  = mat_v( b, i, 0 );
       mat_daxpy( i, -( mat_v(b,i,0) ), mat_ptr(a,0,i), inc, mat_ptr(b,0,0), 1 );
    }

    return( 0 );
}

/*fh=========================================================================
**
**  mat_dpodi
**
**  Cleve Moler      Univ. Nouveau-Mexique (Coyright LINPACK)  08/14/78
**  Ashley Tam (UC)  C version                                 Sept. 87
**  Marc Bedard      Mise a jour pour standard ANSI C          01/11/93
**  Martin Bussieres 0.0L --> 0.0                              17 Nov 93
**
**===========================================================================*/
double mat_dpodi(pMAT_ a, int job)
{
    int i, j, k;
    double t, s;
    double det_entier;
    double det_exposant;

    double det=0.0;

    int inc;
    double *ptr_1, *ptr_2;

    t=0.0;
    s=0.0;

    inc=a->cols;

    /*
    ** Calcule du determinant
    */
    if ((job == MAT_DET) || (job == MAT_DET_INV))
    {
     det_entier = 1.0;
     det_exposant = 0.0;

     s = 10.0;
     det= 0.0;

     for(i=1; i<=a->rows; i++)
     {
       det_entier = ((mat_v(a,i-1,i-1))*(mat_v(a,i-1,i-1)))*det_entier;
       if (det_entier == 0)
         return det; /*Pas de chiffre = 0 sur la diagonale*/
       if (det_entier < 1)
         while (det_entier < 1)
           {
            det_entier = s*det_entier;
            det_exposant = det_exposant - 1;
           }
       if(det_entier >= s)
         while (det_entier >= s)
           {
             det_entier = det_entier/s;
             det_exposant = det_exposant + 1;
           }
     }
       det = det_entier * pow(10.0, det_exposant);
    }

     /*
     ** Calcule de l'inverse
     */
     if (job == MAT_DET)
      return det;

     for (k=0; k<a->rows; k++)
     {
        mat_v(a,k,k) = 1/mat_v(a,k,k);
        t = -mat_v(a,k,k);
        mat_dscal(k, t, mat_ptr(a,0,k), inc);

        for(j=k+1; j<a->cols; j++)
        {
          t = mat_v(a,k,j);
          mat_v(a,k,j) = 0.0;

          mat_daxpy(k+1, t, mat_ptr(a,0,k), inc, mat_ptr(a,0,j), inc);
        }
     }

    /*
    ** Formation de inverse * trans(inverse)
    */
    for(j=0; j<a->rows; j++)
    {
      for(k=j; k<a->cols; k++)
        {
          ptr_1 = mat_ptr(a,j,k);
          ptr_2 = mat_ptr(a,k,k);
          t = 0;
          for (i=a->rows-k; i>0; --i,++ptr_1, ++ptr_2)
            t = t + ((*ptr_1) * (*ptr_2));
          mat_v(a,j,k) = t;
        }
    }
   return(det);
}

/*fh=========================================================================
**
**  mat_ddot
**
**  Jack Dogarra    Copyright LINPACK                         03/11/78
**  Ashley Tam (UC) C version                                 Sept. 87
**  Alain Lapierre  Mise a jour pour standard ANSI C          27/1/93
**
**===========================================================================*/

double mat_ddot(

int    count,
double *ptr_x,
int    incx,
double *ptr_y,
int    incy )

{  double sum;
   int    i;

   sum  = 0.0e0;
   for ( i = 1; i <= count; ++i, ptr_x += incx, ptr_y += incy )
       sum = sum + ( *ptr_x * *ptr_y );

   return( sum );
}

/*fh=========================================================================
**
**  mat_daxpy
**
**  Jack Dogarra    Copyright LINPACK                         03/11/78
**  Ashley Tam (UC) C version                                 Sept. 87
**  Alain Lapierre  Mise a jour pour standard ANSI C          27/1/93
**
**===========================================================================*/


int mat_daxpy(

int    count,
double a,
double *ptr_x,
int    incx,
double *ptr_y,
int    incy )

{
   int i;

   for ( i = 1; i <= count; ++i, ptr_x += incx, ptr_y += incy )
      *ptr_y = ( a * ( *ptr_x ) ) + *ptr_y ;

   return( count );
}

/*fh=========================================================================
**
**  mat_dscal
**
**  Jack Dogarra    Copyright LINPACK                         03/11/78
**  Ashley Tam (UC) C version                                 Sept. 87
**  Alain Lapierre  Mise a jour pour standard ANSI C          27/1/93
**
**===========================================================================*/

int mat_dscal(

int    count,
double a,
double *ptr_x,
int    incx )

{
   int i;

   for ( i = 1; i <= count; ++i, ptr_x += incx )
      *ptr_x =  a * ( *ptr_x ) ;

   return( count );
}

/*fh=========================================================================
**
**  mat_create
**
**  Ashley Tam (UC) Original version                          Sept. 87
**  Alain Lapierre  Mise a jour pour standard ANSI C          27/1/93
**  Alain Robert  Rendre Compiler and platform independant 1995/03/24
**===========================================================================*/

pMAT_ mat_create(

int r,
int c)
{
    pMAT_ m;

    /*
    ** Allocate matrix structure
    */
    if ((m = (pMAT_ )malloc(sizeof(MAT_))) == NULL)
        return(MAT_NULL);

    /*
    ** Allocate space for data
    */
    if ((m->mat_value =(double*) malloc(sizeof(double) * r * c)) == NULL)
    {
        free(m);
        return(MAT_NULL);
    }

    /*
    ** Save Matrix definition
    */
    m->rows = r;
    m->cols = c;
    m->msize = r * c;

    return(m);

}


/*fh=========================================================================
**
**  mat_destroy
**
**  Ashley Tam (UC) Original Library                          Sept. 87
**  Alain Robert  New function                                                    1995/03/24
**
**===========================================================================*/

void mat_destroy(pMAT_ a)
{
    /*
    ** Free space previously allocated for data
    */
    if ((a != NULL) && (a->mat_value != NULL))
    {
        free(a->mat_value);
        free(a);

    }
}


/*fh=========================================================================
**
**  mat_print
**
**  Alain Lapierre  Version originale                         27/1/93
**
**===========================================================================*/

void mat_print(
pMAT_ matrix)
    {
    int rows, cols;
    printf("\n");
    for(rows=0; rows < matrix->rows; rows++, printf("\n"))
     for(cols=0; cols < matrix->cols; cols++)
            printf("%15lf",mat_v(matrix,rows,cols));
    }

/*fh=========================================================================
**
**  mat_ltfill
**
**  Ashley Tam (UC) C version of the Ensulib by Mike Mepham   Sept. 87
**  Marc Bedard  Mise a jour pour standard ANSI C             2/10/93\
**
**===========================================================================*/
pMAT_ mat_ltfill (
pMAT_ a)
{

    int     row, col;

    if (a->rows != a->cols)
      return(MAT_NULL);
    for (row = 1; row < a->rows; ++row)
      for ( col = 0; col < row; ++col )
        mat_v(a, row, col) = mat_v(a, col , row);
    return(a);
}

/*fh=========================================================================
**
**  mat_setIdentity
**
**  DESCRIPTION
**      Set an identity matrix
**
**
**  Stephane Poulin            3/11/97
**===========================================================================*/
HSTATUS mat_setIdentity(pMAT_ pio_pMatrix)
{
    int row, col;

    if (pio_pMatrix == MAT_NULL)
        return(HERROR);

    /*
    ** Must be a square matrix, else return
    */
    if (pio_pMatrix->rows != pio_pMatrix->cols)
        return(HERROR);

    for (row = 0; row < pio_pMatrix->rows; row++)
        for (col = 0; col < pio_pMatrix->cols; col++)
        {
            mat_v(pio_pMatrix, row, col) = (double) (row == col);
        }

    return(HSUCCESS);

}
/*fh=========================================================================
**  mat_isNull
**
**  DESCRIPTION
**      Test whether the matrix is null
**
**
**  Stephane Poulin     FEB 1999
**===========================================================================*/
int8_t mat_isNull(pMAT_ pio_pMatrix)
{
    int row, col;
    int8_t isNull = TRUE;

    assert(pio_pMatrix != MAT_NULL);

    for (row = 0; row < pio_pMatrix->rows; row++)
    {
        for (col = 0; col < pio_pMatrix->cols; col++)
        {
            if (mat_v(pio_pMatrix, row, col) != 0.0)
            {
                isNull = FALSE;

                /*
                ** Set row and col to their maximum values to exit the loops
                */
                row = pio_pMatrix->rows;
                col = pio_pMatrix->cols;
            }
        }
    }

    return(isNull);

}

/*fh=========================================================================
**
**  mat_createFromRawData
**
**  DESCRIPTION
**      This create a new matrix and initialize it's content with the values
**      contained in the rawData buffer
**
**
**  Stephane Poulin            FEB 1999
**===========================================================================*/
pMAT_ mat_createFromRawData (int32_t   pi_dimX,
                             int32_t   pi_dimY,
                             double * pi_pRawData )
{
    int32_t   row;
    int32_t   col;
    pMAT_     tmpMat;
    double * pData = pi_pRawData;

    HDEF(Status, HSUCCESS);

    tmpMat = mat_create(pi_dimX, pi_dimY);
    if (MAT_NULL == tmpMat)
    {
        HSET(Status, HERROR);
        goto WRAPUP;
    }

    /*
    ** Initialize matrix data
    */
    for (row = 0; row < tmpMat->rows; row++)
    {
        for (col = 0; col < tmpMat->cols; col++)
        {
            mat_v(tmpMat, row, col) = *pData;
            pData++;
        }
    }

WRAPUP:
    if (HISERROR(Status))
    {
        tmpMat = MAT_NULL;
    }

    HRET(tmpMat);
}

/*fh=========================================================================
**
**  mat_copy
**
**  DESCRIPTION
**      This function allocate memory and take a copy of an input matrix.
**
**
**  Stephane Poulin            4/11/97
**===========================================================================*/

pMAT_ mat_copy (pMAT_ pi_pMatToCopy)
{

    pMAT_   matrix;

    if (pi_pMatToCopy == MAT_NULL)
        goto WRAPUP;

    /*
    ** Allocate a new matrix
    */
    matrix = mat_create (pi_pMatToCopy->rows, pi_pMatToCopy->cols);
    if (matrix == MAT_NULL)
        goto WRAPUP;


    /*
    ** Copy matrix data
    */

    if (HSUCCESS != mat_copyData (matrix, pi_pMatToCopy))
    {
        goto WRAPUP;
    }

    return (matrix);


WRAPUP:
    return (MAT_NULL);
}

/*fh=========================================================================
**
**  mat_copyData
**
**  DESCRIPTION
**      This function take a copy of an input matrix.
**
**
**  Stephane Poulin            4/11/97
**===========================================================================*/
HSTATUS mat_copyData (pMAT_ po_pOutputMatrix, pMAT_ pi_pInputMatrix)
{

    int     row, col;

    if (po_pOutputMatrix == MAT_NULL || pi_pInputMatrix == MAT_NULL)
        goto WRAPUP;

    /*
    ** Matrix must be the same size
    */
    if (po_pOutputMatrix->rows != pi_pInputMatrix->rows ||
        po_pOutputMatrix->cols != pi_pInputMatrix->cols)
    {
        goto WRAPUP;
    }

    /*
    ** Copy matrix data
    */
    for (row = 0; row < pi_pInputMatrix->rows; row++)
        for (col = 0; col < pi_pInputMatrix->cols; col++)
            mat_v(po_pOutputMatrix, row, col) = mat_v(pi_pInputMatrix, row, col);

    return (HSUCCESS);

WRAPUP:
    return (HERROR);
}

/*____________________________________________________________________________
|  mat_qrSolve
|
|  DESCRIPTION
|    Solve a system of linear equations of the form Ax = b by QR decomposition.
|
|  PARAMETERS
|    a  -   The observation matrix
|    bx -   The matrix b witch will be overwritten by the solution of the
|           equation.
|    by -   Idem as bx but for the y solution.
|
|  Stephane Poulin   JULY 1998
|____________________________________________________________________________*/

HSTATUS mat_qrSolve (pMAT_ a, pMAT_ bx, pMAT_ by)
{
    int32_t ret;
    int32_t n;
    pMAT_ c = MAT_NULL;
    pMAT_ d = MAT_NULL;

        HDEF (Status, HSUCCESS);

    /*
    ** Check if the input matrix is a square matrix
    */
    if (a->rows == a->cols && a->rows != 0)
    {
        n = a->rows;
    }
    else
    {
        HSET(Status, HERROR);
        goto WRAPUP;
    }

    c = mat_create(n, 1);
    if (c == MAT_NULL)
    {
        HSET(Status, HERROR);
        goto WRAPUP;
    }

    d = mat_create(n, 1);
    if (d == MAT_NULL)
    {
        HSET(Status, HERROR);
        goto WRAPUP;
    }

    /*
    ** Perform the QR decomposition
    */
    if ( HISERROR (mat_qrdcmp (a, c, d, &ret)))
    {
        HSET(Status, HERROR);
        goto WRAPUP;
    }

    /*
    ** Check for singularity
    */
    if (ret == 1)
    {
        HSET(Status, HERROR);
        goto WRAPUP;
    }

    /*
    ** Solve the linear system
    */
    if ( HISERROR (mat_qrsolv (a, c, d, bx)))
    {
        HSET(Status, HERROR);
        goto WRAPUP;
    }
    if ( HISERROR (mat_qrsolv (a, c, d, by)))
    {
        HSET(Status, HERROR);
        goto WRAPUP;
    }


WRAPUP:
    if (MAT_NULL != c)
        mat_destroy (c);
    if (MAT_NULL != d)
        mat_destroy (d);

    HRET(Status);
}


/*____________________________________________________________________________
|  mat_qrdcmp
|
|  DESCRIPTION
|    Construct the QR decomposition of the matrix A. The upper triangular
|    matrix R is returned in the upper triangle of A, except for the diagonal
|    elements of R witch are returned in D.
|
|    sing return true (1) if singularity is encountered but the decomposition
|    is still completed; otherwise it returns FALSE (0).
|
|  SEE ALSO
|    mat_qrSolve
|
|  REFERENCE
|    Numerical Recepies in C, Second Edition. pages 98-100.
|
|  Stephane Poulin   JULY 1998
|____________________________________________________________________________*/

HSTATUS mat_qrdcmp (pMAT_ a, pMAT_ c, pMAT_ d, int32_t *sing)
{
        HDEF (Status, HSUCCESS);

        int32_t i, j, k;
        double scale;
        double sigma;
        double sum;
        double tau;
    int32_t n;


    /*
    ** Check if the input matrix is a square matrix
    */
    if (a->rows == a->cols && a->rows != 0)
    {
        n = a->rows;
    }
    else
    {
        HSET(Status, HERROR);
        goto WRAPUP;
    }

        *sing = 0;

        for (k=0; k<n-1; k++)
        {
                scale = 0.0;

                for (i=k; i<n; i++)
                {
                        scale = max (scale, fabs(mat_v(a,i,k)));
                }

//              if (HDOUBLE_EQUAL_EPSILON(scale, 0.0))
        if (0.0 == scale)
                {
                        /*
                        ** Singular Case
                        */
                        *sing = 1;
                        mat_v(c,k,0) = mat_v(d,k,0) = 0.0;
                }
                else
                {
                        /*
                        ** Form Qk and Qk * A
                        */
                        for (i=k; i<n; i++)
                        {
                                mat_v(a,i,k) /= scale;
                        }

                        for (sum=0.0, i=k; i<n; i++)
                        {
                                sum += ( mat_v(a,i,k) * mat_v(a,i,k) );
                        }

                        sigma = SIGN(sqrt(sum), mat_v(a,k,k));
                        mat_v(a,k,k) += sigma;
                        mat_v(c,k,0) = sigma * mat_v(a,k,k);
                        mat_v(d,k,0) = -scale * sigma;

                        for (j=k+1; j<n; j++)
                        {
                                for (sum=0.0, i=k; i<n; i++)
                                {
                                        sum += mat_v(a,i,k) * mat_v(a,i,j);
                                }
                                tau = sum/mat_v(c,k,0);
                                for (i=k; i<n; i++)
                                {
                                        mat_v(a,i,j) -= tau * mat_v(a,i,k);
                                }
                        }
                }
        }

        mat_v(d,n-1,0) = mat_v(a,n-1,n-1);

        if (mat_v(d,n-1,0) == 0.0)
        {
                *sing = 1;
        }

WRAPUP:
    HRET (Status);
}
/*____________________________________________________________________________
|  mat_qrsolv
|
|  DESCRIPTION
|    Solve the set of n linear equation Ax = B. A, C and D are input as the
|    output of the routine mat_qrdcmp and are not modified. B is input as the
|    right-hand side vector, and is overwritten with the solution vector on
|    output
|
|  SEE ALSO
|    mat_qrSolve
|
|  REFERNCE
|    Numerical Recepies in C, Second Edition. pages 98-100.
|
|  Stephane Poulin   JULY 1998
|____________________________________________________________________________*/

HSTATUS mat_qrsolv ( pMAT_ a, pMAT_ c, pMAT_ d, pMAT_ b)
{
        HDEF(Status, HSUCCESS);

        int32_t i, j;
        double sum;
        double tau;
    int32_t n;


    /*
    ** Check if the input matrix is a square matrix
    */
    if (a->rows == a->cols && a->rows != 0)
    {
        n = a->rows;
    }
    else
    {
        HSET(Status, HERROR);
        goto WRAPUP;
    }

    for (j=0; j<n-1; j++)
        {
                for (sum=0.0, i=j; i<n; i++)
                {
                        sum += mat_v(a,i,j) * mat_v(b,i,0);
                }

                tau = sum/mat_v(c,j,0);

                for (i=j; i<n; i++)
                {
                        mat_v(b,i,0) -= tau * mat_v(a,i,j);
                }
        }

        mat_rsolv (a, d, b);

WRAPUP:
    HRET(Status);
}
/*____________________________________________________________________________
|  mat_rsolv
|
|  DESCRIPTION
|    Solve the set of n linear equation Rx = B. where R is an upper triangular
|    matrix stored in A and D. A and D are input as the output of the routine
|    mat_qrdcmp and are not modified. B is input as th eright-hand side vector,
|    and is overwritten with the solution vector on output.
|
|  SEE ALSO
|    mat_qrSolve
|
|  REFERNCE
|    Numerical Recepies in C, Second Edition. pages 98-100.
|
|  Stephane Poulin   JULY 1998
|____________________________________________________________________________*/

HSTATUS mat_rsolv (pMAT_ a, pMAT_ d, pMAT_ b)
{
        HDEF(Status, HSUCCESS);

        int32_t i, j;
        double sum;
    int32_t n;

    /*
    ** Check if the input matrix is a square matrix
    */
    if (a->rows == a->cols && a->rows != 0)
    {
        n = a->rows;
    }
    else
    {
        HSET(Status, HERROR);
        goto WRAPUP;
    }

    mat_v(b,n-1,0) /= mat_v(d,n-1,0);

        for (i=n-2; i>=0; i--)
        {
                for (sum=0.0, j=i+1; j<=n-1; j++)
                {
                        sum += mat_v(a,i,j) * mat_v(b,j,0);
                }
                mat_v(b,i,0) = (mat_v(b,i,0)-sum)/mat_v(d,i,0);
        }

WRAPUP:
        HRET(Status);
}
