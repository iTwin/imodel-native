/*--------------------------------------------------------------------------------------+
|
|     $Source: ext/MatrixFromTiePts/src/tpscoef.c $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <math.h>

#include "oldhtypes.h"
#include "trfmodel.h"
#include "matrix.h"


HSTATUS trf_getObservationMatrix    ( pMAT_ po_obs,
                                      pMAT_ ctrlPts,
                                      pMAT_ badPts );

HSTATUS trf_getBMatrix              ( pMAT_ B,
                                      pMAT_ ctrlPts,
                                      pMAT_ badPts,
                                      int8_t isX );

HSTATUS trf_getMatrixOfActivePoints ( pMAT_ po_pGoodPts,
                                      pMAT_ po_pBadPts,
                                      pTRANSFO_MODEL_DATA_3D    pi_pModel );

HSTATUS trf_transformTpsSourcePoints (pMAT_ pi_pPoints,
                                      pMAT_ po_pPoints,
                                      double   *po_c,
                                      double   *po_minX,
                                      double   *po_minY );

HSTATUS trf_untransformTpsCoefs     (pMAT_      pi_transformedPoints,
                                     pMAT_      pi_transformedCoefs,
                                     double    pi_c,
                                     double    pi_minX,
                                     double    pi_minY,
                                     pMAT_      po_coefs );

HSTATUS trf_setTpsCoefficients      (COEFFICIENTS_3D *po_pCoefs,
                                     pMAT_            pi_solx,
                                     pMAT_            pi_soly,
                                     int32_t          pi_direction );


/*____________________________________________________________________________
|  trf_calcCoefTps2D
|
|  DESCRIPTION
|    Compute the coefficients for the Thin Plate Spline transformation model.
|
|  Stephane Poulin   JULY 1998
|____________________________________________________________________________*/

int32_t trf_calcCoefTps2D ( int32_t                pi_direction,
                          pTRANSFO_MODEL_DATA_3D pio_pModel )
{
        pMAT_   a;
    pMAT_   bx;
    pMAT_   by;
    pMAT_   solx;
    pMAT_   soly;
    pMAT_   dstPts;
    pMAT_   srcPts;
    pMAT_   transformed_srcPts;
    int32_t nbPoints;
    double C;
    double minx;
    double miny;
        int32_t i;
        int32_t j;

    int8_t srcIdentical       = FALSE;
        int8_t dstIdentical       = FALSE;
    int8_t a_isAllocated      = FALSE;
    int8_t srcPts_isAllocated = FALSE;
    int8_t dstPts_isAllocated = FALSE;
    int8_t bx_isAllocated     = FALSE;
    int8_t by_isAllocated     = FALSE;
    int8_t solx_isAllocated   = FALSE;
    int8_t soly_isAllocated   = FALSE;
    int8_t transformed_srcPts_isAllocated = FALSE;
    int8_t useTransformedPoints = FALSE;


    HDEF(Status, HSUCCESS);

    /*
    ** Validate the number of ctrl points required,
    ** find the size of the matrice and allocate it
    */
    nbPoints = trf_findNoActivePoints(&pio_pModel->ctrlPts3D);
    if (N_PTS_TPS_2D > nbPoints)
    {
         HSET(Status, TRF_ERROR_BAD_NUMBER_POINTS);
         goto WRAPUP;
    }

    /*
    ** If we have more than 3 control points, we transform all of them to
    ** avoid numerical instabilities
    */
    if (nbPoints > 3)
        useTransformedPoints = TRUE;
    /*
    ** Allocate the Observation Matrix
    */
    if ((a = mat_create(nbPoints + 3, nbPoints + 3)) == MAT_NULL)
    {
        HSET(Status, TRF_ERROR_NO_MEMORY);
        goto WRAPUP;
    }
    a_isAllocated = TRUE;

    /*
    ** Allocate the matrix to store the source points (Bad Pts)
    */
    if ((srcPts = mat_create(nbPoints, 2)) == MAT_NULL)
    {
        HSET(Status, TRF_ERROR_NO_MEMORY);
        goto WRAPUP;
    }
    srcPts_isAllocated = TRUE;

    /*
    ** Allocate the matrix to store the destination points (Good Pts)
    */
    if ((dstPts = mat_create(nbPoints, 2)) == MAT_NULL)
    {
        HSET(Status, TRF_ERROR_NO_MEMORY);
        goto WRAPUP;
    }
    dstPts_isAllocated = TRUE;

    /*
    ** Allocate matrix to store transformed source points
    */
    if ((transformed_srcPts = mat_create(nbPoints, 2)) == MAT_NULL)
    {
        HSET(Status, TRF_ERROR_NO_MEMORY);
        goto WRAPUP;
    }
    transformed_srcPts_isAllocated = TRUE;

    /*
    ** Allocate the b matrix (Ax = b) for the transformed x solution
    */
    if ((bx = mat_create(nbPoints + 3, 1)) == MAT_NULL)
    {
        HSET(Status, TRF_ERROR_NO_MEMORY);
        goto WRAPUP;
    }
    bx_isAllocated = TRUE;

    /*
    ** Allocate the b matrix (Ax = b) for the transformed y solution
    */
    if ((by = mat_create(nbPoints + 3, 1)) == MAT_NULL)
    {
        HSET(Status, TRF_ERROR_NO_MEMORY);
        goto WRAPUP;
    }
    by_isAllocated = TRUE;

    /*
    ** Allocate a matrix to store the x and y solution
    */
    if ((solx = mat_create(nbPoints + 3, 1)) == MAT_NULL)
    {
        HSET(Status, TRF_ERROR_NO_MEMORY);
        goto WRAPUP;
    }
    solx_isAllocated = TRUE;

    if ((soly = mat_create(nbPoints + 3, 1)) == MAT_NULL)
    {
        HSET(Status, TRF_ERROR_NO_MEMORY);
        goto WRAPUP;
    }
    soly_isAllocated = TRUE;

        /*
    ** Allocate memory to store the coefficients in the model
    */
    if (HISERROR (trf_allocTpsCoefficients (&pio_pModel->coef3D, nbPoints + 3)))
    {
        HSET(Status, TRF_ERROR_NO_MEMORY);
        goto WRAPUP;
    }

    /*
    ** Get a matrix of points from model
    */
    if (HISERROR (trf_getMatrixOfActivePoints ( dstPts, srcPts, pio_pModel )))
    {
        HSET(Status, HERROR);
        goto WRAPUP;
    }

        /*
        ** Check for identical source points
        ** 2 identical source points -> error
        */
    for (i=0; i<nbPoints; i++)
        {
            for (j=i+1; j<nbPoints; j++)
                {
                        if ((mat_v(srcPts,i,0) == mat_v(srcPts,j,0))
                                && (mat_v(srcPts,i,1) == mat_v(srcPts,j,1)))
                        {
                                srcIdentical = TRUE;
                                break;
                        }
                }
        }
    if (srcIdentical)
    {
        HSET(Status, HERROR);
        goto WRAPUP;
    }

        /*
        ** Check for identical destination points
        ** 2 identical dest.points   -> error
        */
    for (i=0; i<nbPoints; i++)
        {
            for (j=i+1; j<nbPoints; j++)
                {
                        if ((mat_v(dstPts,i,0) == mat_v(dstPts,j,0))
                                && (mat_v(dstPts,i,1) == mat_v(dstPts,j,1)))
                        {
                                dstIdentical = TRUE;
                                break;
                        }
                }
        }
    if (dstIdentical)
    {
        HSET(Status, HERROR);
        goto WRAPUP;
    }

    /*
    ** Allocate memory to store the coefficients in the model
    */
    if (HISERROR (trf_allocTpsCoefficients (&pio_pModel->coef3D, nbPoints + 3)))
    {
        HSET(Status, TRF_ERROR_NO_MEMORY);
        goto WRAPUP;
    }

    /*
    ** Copy the points
    */
    if (HISERROR(mat_copyData(pio_pModel->coef3D.tpsPoints[TRANSFO_DIRECT], srcPts)))
    {
        HSET(Status, HERROR);
        goto WRAPUP;
    }

    if (HISERROR(mat_copyData(pio_pModel->coef3D.tpsPoints[TRANSFO_INVERSE], dstPts)))
    {
        HSET(Status, HERROR);
        goto WRAPUP;
    }
    /*
    ** Compute Coefficients
    */
    if (TRANSFO_DIRECT == pi_direction || TRANSFO_BOTH == pi_direction)
    {

        /*
        ** Apply a transformation on source points to obtain a better
        ** numerical stability
        */
        if (TRUE == useTransformedPoints)
        {
            if (HISERROR (trf_transformTpsSourcePoints (srcPts, transformed_srcPts, &C, &minx, &miny)))
            {
                HSET(Status, HERROR);
                goto WRAPUP;
            }
        }
        else
        {
            /*
            ** Simply copy the control points
            */
            mat_copyData(transformed_srcPts, srcPts);
        }
        /*
        ** Get the matrix A (observation matrix) in Ax = b
        */
        if (HISERROR (trf_getObservationMatrix (a, dstPts, transformed_srcPts)))
        {
            HSET(Status, HERROR);
            goto WRAPUP;
        }

        /*
        ** Get b matrix (Ax = b) for the x and y solution
        */
        if (HISERROR (trf_getBMatrix (bx, dstPts, srcPts, TRUE)))
        {
            HSET(Status, HERROR);
            goto WRAPUP;
        }
        if (HISERROR (trf_getBMatrix (by, dstPts, srcPts, FALSE)))
        {
            HSET(Status, HERROR);
            goto WRAPUP;
        }

        /*
        ** Solve the system of linear equation
        */
        if (HISERROR( mat_qrSolve (a, bx, by)))
        {
            HSET(Status, HERROR);
            goto WRAPUP;
        }

        if (TRUE == useTransformedPoints)
        {
            /*
            ** Restore coefficients
            */
            if (HISERROR (trf_untransformTpsCoefs(transformed_srcPts, bx, C, minx, miny, solx)))
            {
                HSET(Status, HERROR);
                goto WRAPUP;
            }
            if (HISERROR (trf_untransformTpsCoefs(transformed_srcPts, by, C, minx, miny, soly)))
            {
                HSET(Status, HERROR);
                goto WRAPUP;
            }
        }
        else
        {
            /*
            ** Simply copy the coefficients
            */
            mat_copyData(solx, bx);
            mat_copyData(soly, by);
        }

        /*
        ** Set Coefficients
        */
        if (HISERROR (trf_setTpsCoefficients (&pio_pModel->coef3D, solx, soly, TRANSFO_DIRECT)))
        {
            HSET(Status, HERROR);
            goto WRAPUP;
        }
    }
    if (TRANSFO_INVERSE == pi_direction || TRANSFO_BOTH == pi_direction)
    {
        pMAT_   tmpMat;

        /*
        ** The inverse is computed by inverting the source and destination points
        */
        tmpMat = srcPts;
        srcPts = dstPts;
        dstPts = tmpMat;

        if (TRUE == useTransformedPoints)
        {
            /*
            ** Apply a transformation on source points to obtain a better
            ** numerical stability
            */
            if (HISERROR (trf_transformTpsSourcePoints (srcPts, transformed_srcPts, &C, &minx, &miny)))
            {
                HSET(Status, HERROR);
                goto WRAPUP;
            }
        }
        else
        {
            /*
            ** Simply copy the control points
            */
            mat_copyData(transformed_srcPts, srcPts);
        }

        /*
        ** Get the matrix A (observation matrix) in Ax = b
        */
        if (HISERROR (trf_getObservationMatrix (a, dstPts, transformed_srcPts)))
        {
            HSET(Status, HERROR);
            goto WRAPUP;
        }

        /*
        ** Get b matrix (Ax = b) for the x and y solution
        */
        if (HISERROR (trf_getBMatrix (bx, dstPts, srcPts, TRUE)))
        {
            HSET(Status, HERROR);
            goto WRAPUP;
        }
        if (HISERROR (trf_getBMatrix (by, dstPts, srcPts, FALSE)))
        {
            HSET(Status, HERROR);
            goto WRAPUP;
        }

        /*
        ** Solve the system of linear equation
        */
        if (HISERROR (mat_qrSolve (a, bx, by)))
        {
            HSET(Status, HERROR);
            goto WRAPUP;
        }

        if (TRUE == useTransformedPoints)
        {
            /*
            ** Restore coefficients
            */
            if (HISERROR (trf_untransformTpsCoefs(transformed_srcPts, bx, C, minx, miny, solx)))
            {
                HSET(Status, HERROR);
                goto WRAPUP;
            }
            if (HISERROR (trf_untransformTpsCoefs(transformed_srcPts, by, C, minx, miny, soly)))
            {
                HSET(Status, HERROR);
                goto WRAPUP;
            }
        }
        else
        {
            /*
            ** Simply copy the coefficients
            */
            mat_copyData(solx, bx);
            mat_copyData(soly, by);
        }

        /*
        ** Set Coefficients
        */
        if (HISERROR (trf_setTpsCoefficients (&pio_pModel->coef3D, solx, soly, TRANSFO_INVERSE)))
        {
            HSET(Status, HERROR);
            goto WRAPUP;
        }
    }

WRAPUP:
    if (a_isAllocated)
        mat_destroy (a);
    if (srcPts_isAllocated)
        mat_destroy (srcPts);
    if (dstPts_isAllocated)
        mat_destroy (dstPts);
    if (bx_isAllocated)
        mat_destroy (bx);
    if (by_isAllocated)
        mat_destroy (by);
    if (solx_isAllocated)
        mat_destroy (solx);
    if (soly_isAllocated)
        mat_destroy (soly);
    if (transformed_srcPts_isAllocated)
        mat_destroy (transformed_srcPts);

    HRET(Status);
}

/*____________________________________________________________________________
|  trf_setTpsCoefficients
|
|  DESCRIPTION
|    Set the coefficients for the Thin Plate Spline transformation model.
|
|  Stephane Poulin   JULY 1998
|____________________________________________________________________________*/

HSTATUS trf_setTpsCoefficients (COEFFICIENTS_3D *po_pCoefs,
                                pMAT_            pi_solx,
                                pMAT_            pi_soly,
                                int32_t          pi_direction )
{
    int32_t x;
    int32_t direction;
    HDEF(Status, HSUCCESS);

    /*
    ** Validate matrix size
    */
    if (pi_solx->rows != pi_soly->rows || pi_solx->cols != pi_soly->cols || pi_solx->cols != 1)
    {
        HSET(Status, HERROR);
        goto WRAPUP;
    }

    if (TRANSFO_DIRECT == pi_direction)
        direction = TRANSFO_DIRECT;
    else if (TRANSFO_INVERSE == pi_direction)
        direction = TRANSFO_INVERSE;
    else
    {
        HSET(Status, HERROR);
        goto WRAPUP;
    }

    /*
    ** Copy the coefficients
    */
    for (x=0; x<pi_solx->rows; x++)
    {
        po_pCoefs->tpsCoef[direction][XIND][x] = mat_v(pi_solx,x,0);
        po_pCoefs->tpsCoef[direction][YIND][x] = mat_v(pi_soly,x,0);
        po_pCoefs->tpsCoef[direction][ZIND][x] = 0.0;
    }

    /*
    ** Set the number of coefficients
    */
    po_pCoefs->nbTpsCoef[XIND] = pi_solx->rows;
    po_pCoefs->nbTpsCoef[YIND] = pi_soly->rows;
    po_pCoefs->nbTpsCoef[ZIND] = pi_solx->rows;

WRAPUP:
    HRET(Status);
}

/*____________________________________________________________________________
|  trf_allocTpsCoefficients
|
|  DESCRIPTION
|    Alloc memory for the coefficients for the Thin Plate Spline
|    transformation model.
|
|  Stephane Poulin   JULY 1998
|____________________________________________________________________________*/

HSTATUS trf_allocTpsCoefficients (COEFFICIENTS_3D *po_pCoefs,
                                  int32_t          pi_nCoefs )
{
    int32_t x, y;
    int32_t nbPoints;

    HDEF(Status, HSUCCESS);

    if (pi_nCoefs <= 0)
        goto WRAPUP;

    /*
    ** Free older coefficients
    */
    if (TRUE == po_pCoefs->tpsCoefIsAllocated)
    {
        trf_freeTpsCoefficients(po_pCoefs);
    }

    for (x=0; x<2; x++)
    {
        for (y=0; y<3; y++)
        {
            po_pCoefs->tpsCoef[x][y] = (double*)malloc(pi_nCoefs * sizeof(double));
            if (NULL == po_pCoefs->tpsCoef[x][y])
            {
                HSET(Status, TRF_ERROR_NO_MEMORY);
                goto WRAPUP;
            }
        }
    }

    nbPoints = max(0, pi_nCoefs-3);
    /*
    ** Allocate the source and destination matrix of points
    */
    if (MAT_NULL == (po_pCoefs->tpsPoints[TRANSFO_DIRECT] = mat_create(nbPoints, 2)))
    {
        HSET(Status, TRF_ERROR_NO_MEMORY);
        goto WRAPUP;
    }
    if (MAT_NULL == (po_pCoefs->tpsPoints[TRANSFO_INVERSE] = mat_create(nbPoints, 2)))
    {
        HSET(Status, TRF_ERROR_NO_MEMORY);
        goto WRAPUP;
    }

    po_pCoefs->nbTpsCoef[0] = pi_nCoefs;
    po_pCoefs->nbTpsCoef[1] = pi_nCoefs;
    po_pCoefs->nbTpsCoef[2] = pi_nCoefs;

    po_pCoefs->tpsCoefIsAllocated = TRUE;


WRAPUP:
    if (HISERROR(Status))
    {
        trf_freeTpsCoefficients(po_pCoefs);
    }

    HRET(Status);
}

/*____________________________________________________________________________
|  trf_freeTpsCoefficients
|
|  DESCRIPTION
|    Free memory for the coefficients of the Thin Plate Spline
|    transformation model.
|
|  Stephane Poulin   JULY 1998
|____________________________________________________________________________*/
HSTATUS trf_freeTpsCoefficients ( COEFFICIENTS_3D *po_pCoefs )
{
    int32_t x, y;
    HDEF(Status, HSUCCESS);


    if (FALSE == po_pCoefs->tpsCoefIsAllocated)
        goto WRAPUP;

    /*
    ** For direct and inverse coefficients
    */
    for (x=0; x<2; x++)
    {
        /*
        ** For x, y, z, coefficients
        */
        for (y=0; y<3; y++)
        {
            /*
            ** Free memory for the coefficients
            */
            if (NULL != po_pCoefs->tpsCoef[x][y])
                free(po_pCoefs->tpsCoef[x][y]);
        }
    }

    /*
    ** Free the matrix of points
    */
    if (MAT_NULL != po_pCoefs->tpsPoints[TRANSFO_DIRECT])
        mat_destroy(po_pCoefs->tpsPoints[TRANSFO_DIRECT]);

    po_pCoefs->tpsPoints[TRANSFO_DIRECT] = MAT_NULL;

    if (MAT_NULL != po_pCoefs->tpsPoints[TRANSFO_INVERSE])
        mat_destroy(po_pCoefs->tpsPoints[TRANSFO_INVERSE]);

    po_pCoefs->tpsPoints[TRANSFO_INVERSE] = MAT_NULL;

    /*
    ** Set the number of coefficients to 0
    */
    po_pCoefs->nbTpsCoef[XIND] = 0;
    po_pCoefs->nbTpsCoef[YIND] = 0;
    po_pCoefs->nbTpsCoef[ZIND] = 0;

    po_pCoefs->tpsCoefIsAllocated = FALSE;

WRAPUP:
    HRET(Status);
}

/*____________________________________________________________________________
|  trf_getMatrixOfActivePoints
|
|  DESCRIPTION
|    Get the active control points for a model.
|
|  Stephane Poulin   JULY 1998
|____________________________________________________________________________*/

HSTATUS trf_getMatrixOfActivePoints ( pMAT_                   po_pGoodPts,
                                      pMAT_                   po_pBadPts,
                                      pTRANSFO_MODEL_DATA_3D  pi_pModel )
{
    int32_t             src;
    int32_t            nPoints;
    int32_t            nActivePoints;
    pLIST               lPoints;
    pCONTROL_POINTS_3D  ctrl;
        int32_t             offset;


    /*
    ** Get a ptr to the list of pts
    */
    lPoints = &pi_pModel->ctrlPts3D;

    /*
    ** Point to the beginning of the list of points
    */
    ctrl=trf_getCtrlPtsAddr(lPoints);


    nPoints = list_count(lPoints);
    nActivePoints = trf_findNoActivePoints(lPoints);

    /*
    ** Copy the active points in the MATRIX
    */
    offset = 0;
        for(src=0; src < nPoints; src++)
    {
        if (ctrl[src].pointActive)
        {
            mat_v(po_pBadPts,src-offset,0)  = ctrl[src].pointBad.x;
            mat_v(po_pBadPts,src-offset,1)  = ctrl[src].pointBad.y;
            mat_v(po_pGoodPts,src-offset,0) = ctrl[src].pointOk.x;
            mat_v(po_pGoodPts,src-offset,1) = ctrl[src].pointOk.y;
        }
                else
                {
                        offset++;
                }
    }

    HRET(HSUCCESS);
}

/*____________________________________________________________________________
|  trf_getObservationMatrix
|
|  DESCRIPTION
|    Compute the matrix A in Ax = B for the resolution of thin plate spline
|    coefficients.
|
|  Stephane Poulin   JULY 1998
|____________________________________________________________________________*/

HSTATUS trf_getObservationMatrix ( pMAT_ po_obs, pMAT_ ctrlPts, pMAT_ badPts)
{
    int32_t x, y;


    HDEF(Status, HSUCCESS);

    if (po_obs->rows != po_obs->cols)
    {
        HSET(Status, HERROR);
        goto WRAPUP;
    }
    /*
    ** Fill first line
    */
    for (x=0; x<po_obs->cols; x++)
    {
        mat_v(po_obs,0,x) = (x<3) ? 0.0: 1.0;
    }
    /*
    ** Fill Second Line
    */
    for (x=0; x<po_obs->cols; x++)
    {
        mat_v(po_obs,1,x) = (x<3) ? 0.0: mat_v(badPts,x-3,0);
    }
    /*
    ** Fill third Line
    */
    for (x=0; x<po_obs->cols; x++)
    {
        mat_v(po_obs,2,x) = (x<3) ? 0.0: mat_v(badPts,x-3,1);
    }

    /*
    ** Copy the three first lines into the first three columnes
    */
    for (x=0; x<3; x++)
    {
        for (y=0; y<po_obs->cols; y++)
        {
            mat_v(po_obs,y,x) = mat_v(po_obs,x,y);
        }
    }

    /*
    ** Fill the rest of the matrix
    */
    for (x=3; x<po_obs->rows; x++)
    {
        for (y=3; y<po_obs->cols; y++)
        {
            if ( x == y)
                mat_v(po_obs,x,y) = 0.0;
            else
            {
                double r2;

                r2 = SQR(mat_v(badPts,x-3,0) - mat_v(badPts,y-3,0)) +
                     SQR(mat_v(badPts,x-3,1) - mat_v(badPts,y-3,1));

                mat_v(po_obs,x,y) = r2 * log(r2);
            }
        }
    }

WRAPUP:
    HRET(Status);
}

/*____________________________________________________________________________
|  trf_getBMatrix
|
|  DESCRIPTION
|    Compute the matrix B in Ax = B for the resolution of thin plate spline
|    coefficients.
|
|  Stephane Poulin   JULY 1998
|____________________________________________________________________________*/

HSTATUS trf_getBMatrix (pMAT_   B,
                        pMAT_   ctrlPts,
                        pMAT_   badPts,
                        int8_t isX)
{
    int32_t x;
    int32_t idx;
    HDEF (Status, HSUCCESS);


    if (B->rows != ctrlPts->rows + 3)
    {
        HSET(Status, HERROR);
        goto WRAPUP;
    }

    idx = (isX) ? 0: 1;

    mat_v(B,0,0) = 0.0;
    mat_v(B,1,0) = 0.0;
    mat_v(B,2,0) = 0.0;

    for (x=3; x<B->rows; x++)
    {
        mat_v(B,x,0) = mat_v(ctrlPts,x-3,idx) - mat_v(badPts,x-3,idx);
    }

WRAPUP:
    HRET(Status);
}


/*____________________________________________________________________________
|  trf_transformTpsSourcePoints
|
|  DESCRIPTION
|    Modify the points to acheive a better numerical stability.
|
|  Stephane Poulin   JULY 1998
|____________________________________________________________________________*/

HSTATUS trf_transformTpsSourcePoints (pMAT_    pi_pPoints,
                                      pMAT_    po_pPoints,
                                      double *po_c,
                                      double *po_minX,
                                      double *po_minY )
{

    int32_t x;
    double minX, minY;
    double maxX, maxY;
    double c;

    HDEF(Status, HSUCCESS);

    /*
    ** Validations
    */
    if (pi_pPoints->rows <= 0)
    {
        HSET(Status, HERROR);
        goto WRAPUP;
    }

    /*
    ** Input and output matrix must be the same dimension and have at least
    ** two columns to store (x,y) values.
    */
    if ((pi_pPoints->rows != po_pPoints->rows) ||
        (pi_pPoints->cols != po_pPoints->cols) ||
        (pi_pPoints->cols != 2))
    {
        HSET(Status, HERROR);
        goto WRAPUP;
    }

    /*
    ** Find min max
    */
    minX = mat_v(pi_pPoints, 0,0);
    minY = mat_v(pi_pPoints, 0,1);
    maxX = mat_v(pi_pPoints, 0,0);
    maxY = mat_v(pi_pPoints, 0,1);

    for (x=1; x<pi_pPoints->rows; x++)
    {
        minX = min(minX, mat_v(pi_pPoints, x,0));
        minY = min(minY, mat_v(pi_pPoints, x,1));
        maxX = max(maxX, mat_v(pi_pPoints, x,0));
        maxY = max(maxY, mat_v(pi_pPoints, x,1));
    }

    c = max ((maxX - minX), (maxY - minY));

    /*
    ** C must not equal to zero
    */
    if (HDOUBLE_EQUAL_EPSILON(c, 0.0))
    {
        HSET(Status, HERROR);
        goto WRAPUP;
    }

    /*
    ** Transform all points
    */
    for (x=0; x<pi_pPoints->rows; x++)
    {
        mat_v(po_pPoints,x,0) = (mat_v(pi_pPoints,x,0) - minX) / c;
        mat_v(po_pPoints,x,1) = (mat_v(pi_pPoints,x,1) - minY) / c;
    }

    /*
    ** Set output values
    */
    *po_c = c;
    *po_minX = minX;
    *po_minY = minY;

WRAPUP:
    HRET(Status);
}

/*____________________________________________________________________________
|  trf_untransformTpsCoefs
|
|  DESCRIPTION
|    Modify the coefficients computed with transformed control points.
|
|  Stephane Poulin   JULY 1998
|____________________________________________________________________________*/
HSTATUS trf_untransformTpsCoefs (pMAT_     pi_transformedPoints,
                                 pMAT_     pi_transformedCoefs,
                                 double   pi_c,
                                 double   pi_minX,
                                 double   pi_minY,
                                 pMAT_     po_coefs )
{

    double sum;
    int32_t x;
    HDEF(Status, HSUCCESS);


    /*
    ** Compute first coef (A0)
    */
    sum = 0.0;
    for (x=3; x<pi_transformedCoefs->rows; x++)
    {
        sum += 2.0 * mat_v(pi_transformedCoefs, x, 0) * (SQR(mat_v(pi_transformedPoints,x-3,0)) +
                                                         SQR(mat_v(pi_transformedPoints,x-3,1)));
    }

    mat_v(po_coefs,0,0) =  mat_v(pi_transformedCoefs,0,0) -
                          (mat_v(pi_transformedCoefs,1,0) * pi_minX / pi_c) -
                          (mat_v(pi_transformedCoefs,2,0) * pi_minY / pi_c) -
                          (log (pi_c) * sum);

    /*
    ** Compute second and third coef
    */
    mat_v(po_coefs,1,0) = mat_v(pi_transformedCoefs,1,0) / pi_c;
    mat_v(po_coefs,2,0) = mat_v(pi_transformedCoefs,2,0) / pi_c;

    /*
    ** Compute remaining coefs
    */
    for (x=3; x<pi_transformedCoefs->rows; x++)
    {
        mat_v(po_coefs,x,0) = mat_v(pi_transformedCoefs,x,0) / SQR(pi_c);
    }


    HRET(Status);
}
