//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: ext/MatrixFromTiePts/src/MatrixFromTiePts.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Function GetTransfoMatrixFromScaleAndTiePts
//          GetProjectiveTransfoMatrixFromScaleAndTiePts
//          GetAffineTransfoMatrixFromScaleAndTiePts
//-----------------------------------------------------------------------------


#include "..\h\MatrixFromTiePts.h"
#include "oldhtypes.h"
#include "hcnvrtif.h"
#include "matrix.h"
#include "trfmodel.h"
#include "hbasemod.h"

/*fh========================================================================
** GetTransfoMatrixFromScaleAndTiePts
**
** DESCRIPTION: This function fill a 4x4 matrix with the transformation model from
**              the given vector of points and vector of scale.
**
**
** PARAMETERS:  po_pMatrix              -> A 4x4 matrix
**              pi_NbVal_GeoTiePoint    -> Number of tie point in pi_pVal_GeoTiePoint vector
**              pi_pVal_GeoTiePoint     -> Pointer to a vector of tie point
**              pi_NbVal_GeoPixelScale  -> Number of tie point in pi_pVal_GeoPixelscale vector
**              pi_pVal_GeoPixelScale   -> Pointer to a vector of pixel scale
**
**
** RETURNS:     This function returns H_SUCCESS on success. It returns H_ERROR
**              otherwise
**
**  Cedric Boulanger 00/07/26 - Original Version
**===========================================================================*/

long GetTransfoMatrixFromScaleAndTiePts (double          po_pMatrix[4][4],
                                            unsigned short  pi_NbVal_GeoTiePoint,
                                            double*         pi_pVal_GeoTiePoint,
                                            unsigned short  pi_NbVal_GeoPixelScale,
                                            double*         pi_pVal_GeoPixelScale)

{
    pMAT_ pMatrix;
    pMatrix = mat_create(4,4);

    trf_init();

    HSTATUS res = sHConvertTIF_MakeModelerFromScaleAndTiePts (pMatrix,
                                                              pi_NbVal_GeoTiePoint,
                                                              pi_pVal_GeoTiePoint,
                                                              pi_NbVal_GeoPixelScale,
                                                              pi_pVal_GeoPixelScale);

    memcpy (po_pMatrix, pMatrix->mat_value, 16*sizeof(double));

    trf_end();
    mat_destroy(pMatrix);

    return res;
}


/*fh========================================================================
** GetProjectiveTransfoMatrixFromScaleAndTiePts
**
** DESCRIPTION: This function fill a 4x4 matrix with the transformation model from
**              the given vector of points and vector of scale.
**
**
**
** PARAMETERS:  po_pMatrix              -> A 4x4 matrix
**              pi_NbVal_GeoTiePoint    -> Number of tie point in pi_pVal_GeoTiePoint vector
**              pi_pVal_GeoTiePoint     -> Pointer to a vector of tie point
**              pi_NbVal_GeoPixelScale  -> Number of tie point in pi_pVal_GeoPixelscale vector
**              pi_pVal_GeoPixelScale   -> Pointer to a vector of pixel scale
**
**
** RETURNS:     This function returns H_SUCCESS on success. It returns H_ERROR
**              otherwise
**
**===========================================================================*/
long GetProjectiveTransfoMatrixFromScaleAndTiePts(double         po_pMatrix[4][4],
                                                     unsigned short pi_NbVal_GeoTiePoint,
                                                     double*        pi_pVal_GeoTiePoint)

{
    pMAT_ pMatrix;
    int8_t TrfInitialized = FALSE;

    pMatrix = mat_create(4,4);

    trf_init();
    TrfInitialized = TRUE;


// =======================================

    HDEF    (Status, H_SUCCESS);
    int32_t             i;
    TRFHANDLE           TrfModel;
    int8_t             TrfCreated=FALSE;
    CONTROL_POINTS_3D   ctrlPts;
    int32_t            NbPts=0;
    int32_t            PtsPosition=0;
    int32_t             TransfoModel=TRANSFO_AFFI_2D;
    COEFFICIENTS_3D     coefs;
    int32_t             modelType;

    /*
    ** Need at least one tie point
    */
    if (pi_NbVal_GeoTiePoint<6)
    {
        HSET(Status,H_ERROR);
        goto WRAPUP;
    }

    /*
    ** Create a trf_model from the list of point
    */
    if ( ( TrfModel = trf_modelNew ( ) ) < 0 )
    {
        HSET(Status,H_ERROR);
        goto WRAPUP;
    }
    else
    {
        TrfCreated=TRUE;
    }

    /*
    ** Add point to the trf_model
    */
    NbPts= pi_NbVal_GeoTiePoint/6;
    for (i=0;i<NbPts; i++ )
    {
        /*
        ** Add each pair of 3D points
        */
        PtsPosition=i*6;
        ctrlPts.pointActive=TRUE;
        ctrlPts.pointBad.x=pi_pVal_GeoTiePoint[PtsPosition];
        ctrlPts.pointBad.y=pi_pVal_GeoTiePoint[PtsPosition+1];
        ctrlPts.pointBad.z=pi_pVal_GeoTiePoint[PtsPosition+2];
        ctrlPts.pointOk.x=pi_pVal_GeoTiePoint[PtsPosition+3];
        ctrlPts.pointOk.y=pi_pVal_GeoTiePoint[PtsPosition+4];
        ctrlPts.pointOk.z=pi_pVal_GeoTiePoint[PtsPosition+5];

        if ( trf_ctrlPtsModif ( TrfModel, & ctrlPts, CTRL_PTS_MODIF_APPEND ) != H_SUCCESS )
        {
            HSET(Status,H_ERROR);
            goto WRAPUP;
        }
    }

    /*
    ** Set the model to use
    */
    if (NbPts == 1)
    {
        TransfoModel=TRANSFO_TRAN_2D;
    }
    else if (NbPts == 2)
    {
        TransfoModel=TRANSFO_SIMI_2D;
    }
    else if (NbPts == 3)
    {
        /*
        ** 3 pts
        */
        TransfoModel=TRANSFO_AFFI_2D;
    }
    else
    {
        /*
        ** 4 pts and more
        */
        TransfoModel=TRANSFO_PROJ_2D;
    }

    /*
    ** Compute the model coefficients
    */
    if ( trf_modelSetType ( TrfModel, TransfoModel ) != HSUCCESS )
    {
        HSET(Status,H_ERROR);
        goto WRAPUP;
    }

    if ( trf_modelCalcCoef ( TrfModel, TRANSFO_BOTH ) != HSUCCESS )
    {
        HSET(Status,H_ERROR);
        goto WRAPUP;
    }

    if (HISERROR(HSET(Status, trf_modelGetCoefficients (TrfModel, &coefs))))
    {
        goto WRAPUP;
    }



    if (HISERROR(HSET(Status, trf_modelGetType(TrfModel, &modelType))))
    {
        goto WRAPUP;
    }

    mat_v(pMatrix, 0, 0) = 1.0;
    mat_v(pMatrix, 0, 1) = 0.0;
    mat_v(pMatrix, 0, 2) = 0.0;
    mat_v(pMatrix, 0, 3) = 0.0;

    mat_v(pMatrix, 1, 0) = 0.0;
    mat_v(pMatrix, 1, 1) = 1.0;
    mat_v(pMatrix, 1, 2) = 0.0;
    mat_v(pMatrix, 1, 3) = 0.0;

    mat_v(pMatrix, 2, 0) = 0.0;
    mat_v(pMatrix, 2, 1) = 0.0;
    mat_v(pMatrix, 2, 2) = 1.0;
    mat_v(pMatrix, 2, 3) = 0.0;

    mat_v(pMatrix, 3, 0) = 0.0;
    mat_v(pMatrix, 3, 1) = 0.0;
    mat_v(pMatrix, 3, 2) = 0.0;
    mat_v(pMatrix, 3, 3) = 1.0;



    switch (modelType)
    {
        case TRANSFO_TRAN_2D:
            mat_v(pMatrix, 0, 3) = (double) coefs.coef[0][0][0];
            mat_v(pMatrix, 1, 3) = (double) coefs.coef[0][1][0];
            break;

        case TRANSFO_HELM_2D:
        case TRANSFO_SIMI_2D:
        case TRANSFO_AFFI_2D:
            mat_v(pMatrix, 0, 0) = (double)  coefs.coef[0][0][1];
            mat_v(pMatrix, 0, 1) = (double)  coefs.coef[0][0][2];
            mat_v(pMatrix, 0, 3) = (double) (  coefs.coef[1][0][0]
                                             + coefs.coef[0][0][3]
                                             - (coefs.coef[0][0][0]*coefs.coef[0][0][1])
                                             - (coefs.coef[0][1][0]*coefs.coef[0][0][2]));
            mat_v(pMatrix, 1, 0) = (double) coefs.coef[0][1][1];
            mat_v(pMatrix, 1, 1) = (double) coefs.coef[0][1][2];
            mat_v(pMatrix, 1, 3) = (double) (  coefs.coef[1][1][0]
                                             + coefs.coef[0][1][3]
                                             - (coefs.coef[0][0][0]*coefs.coef[0][1][1])
                                             - (coefs.coef[0][1][0]*coefs.coef[0][1][2]));
            break;

        case TRANSFO_MAT_2D:
            mat_v(pMatrix, 0, 0) = (double) coefs.coef[0][0][1];
            mat_v(pMatrix, 0, 1) = (double) coefs.coef[0][0][2];
            mat_v(pMatrix, 0, 3) = (double) coefs.coef[0][0][0];
            mat_v(pMatrix, 1, 0) = (double) coefs.coef[0][1][1];
            mat_v(pMatrix, 1, 1) = (double) coefs.coef[0][1][2];
            mat_v(pMatrix, 1, 3) = (double) coefs.coef[0][1][0];
            break;

        case TRANSFO_PROJ_2D:
            {
                        double k1 = (double) (coefs.coef[0][0][3] - (coefs.coef[0][0][1] * coefs.coef[0][0][0])
                                               - (coefs.coef[0][0][2] * coefs.coef[0][1][0]));
                        double k2 = (double) (1.0 - (coefs.coef[0][0][4] * coefs.coef[0][0][0])
                                               - (coefs.coef[0][0][5] * coefs.coef[0][1][0]));
                        double k3 = (double) (coefs.coef[0][1][3] - (coefs.coef[0][1][1] * coefs.coef[0][0][0])
                                               - (coefs.coef[0][1][2] * coefs.coef[0][1][0]));

                mat_v(pMatrix, 0, 0) = (double) ((coefs.coef[0][0][1]
                                                                     + (coefs.coef[0][0][4]
                                                                                                         * coefs.coef[1][0][0])) / k2);
                mat_v(pMatrix, 0, 1) = (double) ((coefs.coef[0][0][2]
                                                                     + (coefs.coef[0][0][5]
                                                                                                         * coefs.coef[1][0][0])) / k2);
                mat_v(pMatrix, 0, 3) = (double) ((k1 + ( k2 * coefs.coef[1][0][0])) / k2);

                mat_v(pMatrix, 1, 0) = (double) ((coefs.coef[0][1][1]
                                                                     + (coefs.coef[0][0][4]
                                                                                                         * coefs.coef[1][1][0])) / k2);
                mat_v(pMatrix, 1, 1) = (double) ((coefs.coef[0][1][2]
                                                                     + (coefs.coef[0][0][5]
                                                                                                         * coefs.coef[1][1][0])) / k2);
                mat_v(pMatrix, 1, 3) = (double) ((k3 + ( k2 * coefs.coef[1][1][0])) / k2);

                mat_v(pMatrix, 3, 0) = (double) (coefs.coef[0][0][4] / k2);
                mat_v(pMatrix, 3, 1) = (double) (coefs.coef[0][0][5] / k2);
                mat_v(pMatrix, 3, 3) = 1.0;
            }
            break;

        default:
            HSET(Status, HERROR);
            goto WRAPUP;
    }



// ============================

    memcpy (po_pMatrix, pMatrix->mat_value, 16*sizeof(double));

    trf_end();



WRAPUP:

    mat_destroy(pMatrix);

    if (TrfCreated)
        trf_modelDestroy (TrfModel);

    if (TrfInitialized)
        trf_end();

    HRET (Status);
}


/*fh========================================================================
** GetAffineTransfoMatrixFromScaleAndTiePts
**
** DESCRIPTION: This function fill a 4x4 matrix with the transformation model from
**              the given vector of points and vector of scale.
**
**
**
** PARAMETERS:  po_pMatrix              -> A 4x4 matrix
**              pi_NbVal_GeoTiePoint    -> Number of tie point in pi_pVal_GeoTiePoint vector
**              pi_pVal_GeoTiePoint     -> Pointer to a vector of tie point
**              pi_NbVal_GeoPixelScale  -> Number of tie point in pi_pVal_GeoPixelscale vector
**              pi_pVal_GeoPixelScale   -> Pointer to a vector of pixel scale
**
**
** RETURNS:     This function returns H_SUCCESS on success. It returns H_ERROR
**              otherwise
**
**===========================================================================*/
long GetAffineTransfoMatrixFromScaleAndTiePts(double         po_pMatrix[4][4],
                                                 unsigned short pi_NbVal_GeoTiePoint,
                                                 double*        pi_pVal_GeoTiePoint)

{
    pMAT_ pMatrix;
    int8_t TrfInitialized = FALSE;

    pMatrix = mat_create(4,4);

    trf_init();
    TrfInitialized = TRUE;


// =======================================

    HDEF    (Status, H_SUCCESS);
    int32_t             i;
    TRFHANDLE           TrfModel;
    int8_t             TrfCreated=FALSE;
    CONTROL_POINTS_3D   ctrlPts;
    int32_t            NbPts=0;
    int32_t            PtsPosition=0;
    int32_t             TransfoModel=TRANSFO_AFFI_2D;
    COEFFICIENTS_3D     coefs;
    int32_t             modelType;

    /*
    ** Need at least one tie point
    */
    if (pi_NbVal_GeoTiePoint<6)
    {
        HSET(Status,H_ERROR);
        goto WRAPUP;
    }

    /*
    ** Create a trf_model from the list of point
    */
    if ( ( TrfModel = trf_modelNew ( ) ) < 0 )
    {
        HSET(Status,H_ERROR);
        goto WRAPUP;
    }
    else
    {
        TrfCreated=TRUE;
    }

    /*
    ** Add point to the trf_model
    */
    NbPts= pi_NbVal_GeoTiePoint/6;
    for (i=0;i<NbPts; i++ )
    {
        /*
        ** Add each pair of 3D points
        */
        PtsPosition=i*6;
        ctrlPts.pointActive=TRUE;
        ctrlPts.pointBad.x=pi_pVal_GeoTiePoint[PtsPosition];
        ctrlPts.pointBad.y=pi_pVal_GeoTiePoint[PtsPosition+1];
        ctrlPts.pointBad.z=pi_pVal_GeoTiePoint[PtsPosition+2];
        ctrlPts.pointOk.x=pi_pVal_GeoTiePoint[PtsPosition+3];
        ctrlPts.pointOk.y=pi_pVal_GeoTiePoint[PtsPosition+4];
        ctrlPts.pointOk.z=pi_pVal_GeoTiePoint[PtsPosition+5];

        if ( trf_ctrlPtsModif ( TrfModel, & ctrlPts, CTRL_PTS_MODIF_APPEND ) != H_SUCCESS )
        {
            HSET(Status,H_ERROR);
            goto WRAPUP;
        }
    }

    /*
    ** Set the model to use
    */
    if (NbPts == 1)
    {
        TransfoModel=TRANSFO_TRAN_2D;
    }
    else if (NbPts == 2)
    {
        TransfoModel=TRANSFO_SIMI_2D;
    }
    else
    {
        /*
        ** 3 pts and more
        */
        TransfoModel=TRANSFO_AFFI_2D;
    }

    /*
    ** Compute the model coefficients
    */
    if ( trf_modelSetType ( TrfModel, TransfoModel ) != HSUCCESS )
    {
        HSET(Status,H_ERROR);
        goto WRAPUP;
    }

    if ( trf_modelCalcCoef ( TrfModel, TRANSFO_BOTH ) != HSUCCESS )
    {
        HSET(Status,H_ERROR);
        goto WRAPUP;
    }

    if (HISERROR(HSET(Status, trf_modelGetCoefficients (TrfModel, &coefs))))
    {
        goto WRAPUP;
    }



    if (HISERROR(HSET(Status, trf_modelGetType(TrfModel, &modelType))))
    {
        goto WRAPUP;
    }

    mat_v(pMatrix, 0, 0) = 1.0;
    mat_v(pMatrix, 0, 1) = 0.0;
    mat_v(pMatrix, 0, 2) = 0.0;
    mat_v(pMatrix, 0, 3) = 0.0;

    mat_v(pMatrix, 1, 0) = 0.0;
    mat_v(pMatrix, 1, 1) = 1.0;
    mat_v(pMatrix, 1, 2) = 0.0;
    mat_v(pMatrix, 1, 3) = 0.0;

    mat_v(pMatrix, 2, 0) = 0.0;
    mat_v(pMatrix, 2, 1) = 0.0;
    mat_v(pMatrix, 2, 2) = 1.0;
    mat_v(pMatrix, 2, 3) = 0.0;

    mat_v(pMatrix, 3, 0) = 0.0;
    mat_v(pMatrix, 3, 1) = 0.0;
    mat_v(pMatrix, 3, 2) = 0.0;
    mat_v(pMatrix, 3, 3) = 1.0;



    switch (modelType)
    {
        case TRANSFO_TRAN_2D:
            mat_v(pMatrix, 0, 3) = (double) coefs.coef[0][0][0];
            mat_v(pMatrix, 1, 3) = (double) coefs.coef[0][1][0];
            break;

        case TRANSFO_HELM_2D:
        case TRANSFO_SIMI_2D:
        case TRANSFO_AFFI_2D:
            mat_v(pMatrix, 0, 0) = (double)  coefs.coef[0][0][1];
            mat_v(pMatrix, 0, 1) = (double)  coefs.coef[0][0][2];
            mat_v(pMatrix, 0, 3) = (double) (  coefs.coef[1][0][0]
                                             + coefs.coef[0][0][3]
                                             - (coefs.coef[0][0][0]*coefs.coef[0][0][1])
                                             - (coefs.coef[0][1][0]*coefs.coef[0][0][2]));
            mat_v(pMatrix, 1, 0) = (double) coefs.coef[0][1][1];
            mat_v(pMatrix, 1, 1) = (double) coefs.coef[0][1][2];
            mat_v(pMatrix, 1, 3) = (double) (  coefs.coef[1][1][0]
                                             + coefs.coef[0][1][3]
                                             - (coefs.coef[0][0][0]*coefs.coef[0][1][1])
                                             - (coefs.coef[0][1][0]*coefs.coef[0][1][2]));
            break;

        case TRANSFO_MAT_2D:
            mat_v(pMatrix, 0, 0) = (double) coefs.coef[0][0][1];
            mat_v(pMatrix, 0, 1) = (double) coefs.coef[0][0][2];
            mat_v(pMatrix, 0, 3) = (double) coefs.coef[0][0][0];
            mat_v(pMatrix, 1, 0) = (double) coefs.coef[0][1][1];
            mat_v(pMatrix, 1, 1) = (double) coefs.coef[0][1][2];
            mat_v(pMatrix, 1, 3) = (double) coefs.coef[0][1][0];
            break;

        case TRANSFO_PROJ_2D:
            {
                        double k1 = (double) (coefs.coef[0][0][3] - (coefs.coef[0][0][1] * coefs.coef[0][0][0])
                                               - (coefs.coef[0][0][2] * coefs.coef[0][1][0]));
                        double k2 = (double) (1.0 - (coefs.coef[0][0][4] * coefs.coef[0][0][0])
                                               - (coefs.coef[0][0][5] * coefs.coef[0][1][0]));
                        double k3 = (double) (coefs.coef[0][1][3] - (coefs.coef[0][1][1] * coefs.coef[0][0][0])
                                               - (coefs.coef[0][1][2] * coefs.coef[0][1][0]));

                mat_v(pMatrix, 0, 0) = (double) ((coefs.coef[0][0][1]
                                                                     + (coefs.coef[0][0][4]
                                                                                                         * coefs.coef[1][0][0])) / k2);
                mat_v(pMatrix, 0, 1) = (double) ((coefs.coef[0][0][2]
                                                                     + (coefs.coef[0][0][5]
                                                                                                         * coefs.coef[1][0][0])) / k2);
                mat_v(pMatrix, 0, 3) = (double) ((k1 + ( k2 * coefs.coef[1][0][0])) / k2);

                mat_v(pMatrix, 1, 0) = (double) ((coefs.coef[0][1][1]
                                                                     + (coefs.coef[0][0][4]
                                                                                                         * coefs.coef[1][1][0])) / k2);
                mat_v(pMatrix, 1, 1) = (double) ((coefs.coef[0][1][2]
                                                                     + (coefs.coef[0][0][5]
                                                                                                         * coefs.coef[1][1][0])) / k2);
                mat_v(pMatrix, 1, 3) = (double) ((k3 + ( k2 * coefs.coef[1][1][0])) / k2);

                mat_v(pMatrix, 3, 0) = (double) (coefs.coef[0][0][4] / k2);
                mat_v(pMatrix, 3, 1) = (double) (coefs.coef[0][0][5] / k2);
                mat_v(pMatrix, 3, 3) = 1.0;
            }
            break;

        default:
            HSET(Status, HERROR);
            goto WRAPUP;
    }



// ============================

    memcpy (po_pMatrix, pMatrix->mat_value, 16*sizeof(double));

    trf_end();



WRAPUP:

    mat_destroy(pMatrix);

    if (TrfCreated)
        trf_modelDestroy (TrfModel);

    if (TrfInitialized)
        trf_end();

    HRET (Status);
}





/*fh========================================================================
** GetProjectiveTransfoMatrixFromScaleAndTiePts
**
** DESCRIPTION: This function fill a 4x4 matrix with the transformation model from
**              the given vector of points and vector of scale.
**
**
**
** PARAMETERS:  po_pMatrix              -> A 4x4 matrix
**              pi_NbVal_GeoTiePoint    -> Number of tie point in pi_pVal_GeoTiePoint vector
**              pi_pVal_GeoTiePoint     -> Pointer to a vector of tie point
**              pi_NbVal_GeoPixelScale  -> Number of tie point in pi_pVal_GeoPixelscale vector
**              pi_pVal_GeoPixelScale   -> Pointer to a vector of pixel scale
**
**
** RETURNS:     This function returns H_SUCCESS on success. It returns H_ERROR
**              otherwise
**
**===========================================================================*/
long GetTransfoMatrixFromTiePts(double           po_pMatrix[4][4],
                                   unsigned short   pi_NbVal_GeoTiePoint,
                                   const double*    pi_pVal_GeoTiePoint,
                                   long             pi_ModelType)

{
    pMAT_ pMatrix;
    int8_t TrfInitialized = FALSE;

    pMatrix = mat_create(4,4);

    trf_init();
    TrfInitialized = TRUE;


// =======================================

    HDEF    (Status, H_SUCCESS);
    int32_t             i;
    TRFHANDLE           TrfModel;
    int8_t             TrfCreated=FALSE;
    CONTROL_POINTS_3D   ctrlPts;
    int32_t            NbPts=0;
    int32_t            PtsPosition=0;
    int32_t             TransfoModel=TRANSFO_AFFI_2D;
    COEFFICIENTS_3D     coefs;
    int32_t             modelType;

    /*
    ** Need at least one tie point
    */
    if (pi_NbVal_GeoTiePoint<6)
    {
        HSET(Status,H_ERROR);
        goto WRAPUP;
    }

    /*
    ** Create a trf_model from the list of point
    */
    if ( ( TrfModel = trf_modelNew ( ) ) < 0 )
    {
        HSET(Status,H_ERROR);
        goto WRAPUP;
    }
    else
    {
        TrfCreated=TRUE;
    }

    /*
    ** Add point to the trf_model
    */
    NbPts= pi_NbVal_GeoTiePoint/6;
    for (i=0;i<NbPts; i++ )
    {
        /*
        ** Add each pair of 3D points
        */
        PtsPosition=i*6;
        ctrlPts.pointActive=TRUE;
        ctrlPts.pointBad.x=pi_pVal_GeoTiePoint[PtsPosition];
        ctrlPts.pointBad.y=pi_pVal_GeoTiePoint[PtsPosition+1];
        ctrlPts.pointBad.z=pi_pVal_GeoTiePoint[PtsPosition+2];
        ctrlPts.pointOk.x=pi_pVal_GeoTiePoint[PtsPosition+3];
        ctrlPts.pointOk.y=pi_pVal_GeoTiePoint[PtsPosition+4];
        ctrlPts.pointOk.z=pi_pVal_GeoTiePoint[PtsPosition+5];

        if ( trf_ctrlPtsModif ( TrfModel, & ctrlPts, CTRL_PTS_MODIF_APPEND ) != H_SUCCESS )
        {
            HSET(Status,H_ERROR);
            goto WRAPUP;
        }
    }

    /*
    ** Set the model to use
    */
    TransfoModel = pi_ModelType;
    /*
    if (NbPts == 1)
    {
        TransfoModel=TRANSFO_TRAN_2D;
    }
    else if (NbPts == 2)
    {
        TransfoModel=TRANSFO_SIMI_2D;
    }
    else if (NbPts == 3)
    {
        //
        // 3 pts
        //
        TransfoModel=TRANSFO_AFFI_2D;
    }
    else
    {
        //
        // 4 pts and more
        //
        TransfoModel=TRANSFO_PROJ_2D;
    }
    */

    /*
    ** Compute the model coefficients
    */
    if ( trf_modelSetType ( TrfModel, TransfoModel ) != HSUCCESS )
    {
        HSET(Status,H_ERROR);
        goto WRAPUP;
    }

    if ( trf_modelCalcCoef ( TrfModel, TRANSFO_BOTH ) != HSUCCESS )
    {
        HSET(Status,H_ERROR);
        goto WRAPUP;
    }

    if (HISERROR(HSET(Status, trf_modelGetCoefficients (TrfModel, &coefs))))
    {
        goto WRAPUP;
    }



    if (HISERROR(HSET(Status, trf_modelGetType(TrfModel, &modelType))))
    {
        goto WRAPUP;
    }

    mat_v(pMatrix, 0, 0) = 1.0;
    mat_v(pMatrix, 0, 1) = 0.0;
    mat_v(pMatrix, 0, 2) = 0.0;
    mat_v(pMatrix, 0, 3) = 0.0;

    mat_v(pMatrix, 1, 0) = 0.0;
    mat_v(pMatrix, 1, 1) = 1.0;
    mat_v(pMatrix, 1, 2) = 0.0;
    mat_v(pMatrix, 1, 3) = 0.0;

    mat_v(pMatrix, 2, 0) = 0.0;
    mat_v(pMatrix, 2, 1) = 0.0;
    mat_v(pMatrix, 2, 2) = 1.0;
    mat_v(pMatrix, 2, 3) = 0.0;

    mat_v(pMatrix, 3, 0) = 0.0;
    mat_v(pMatrix, 3, 1) = 0.0;
    mat_v(pMatrix, 3, 2) = 0.0;
    mat_v(pMatrix, 3, 3) = 1.0;



    switch (modelType)
    {
        case TRANSFO_TRAN_2D:
            mat_v(pMatrix, 0, 3) = (double) coefs.coef[0][0][0];
            mat_v(pMatrix, 1, 3) = (double) coefs.coef[0][1][0];
            break;

        case TRANSFO_HELM_2D:
        case TRANSFO_SIMI_2D:
        case TRANSFO_AFFI_2D:
            mat_v(pMatrix, 0, 0) = (double)  coefs.coef[0][0][1];
            mat_v(pMatrix, 0, 1) = (double)  coefs.coef[0][0][2];
            mat_v(pMatrix, 0, 3) = (double) (  coefs.coef[1][0][0]
                                             + coefs.coef[0][0][3]
                                             - (coefs.coef[0][0][0]*coefs.coef[0][0][1])
                                             - (coefs.coef[0][1][0]*coefs.coef[0][0][2]));
            mat_v(pMatrix, 1, 0) = (double) coefs.coef[0][1][1];
            mat_v(pMatrix, 1, 1) = (double) coefs.coef[0][1][2];
            mat_v(pMatrix, 1, 3) = (double) (  coefs.coef[1][1][0]
                                             + coefs.coef[0][1][3]
                                             - (coefs.coef[0][0][0]*coefs.coef[0][1][1])
                                             - (coefs.coef[0][1][0]*coefs.coef[0][1][2]));
            break;

        case TRANSFO_MAT_2D:
            mat_v(pMatrix, 0, 0) = (double) coefs.coef[0][0][1];
            mat_v(pMatrix, 0, 1) = (double) coefs.coef[0][0][2];
            mat_v(pMatrix, 0, 3) = (double) coefs.coef[0][0][0];
            mat_v(pMatrix, 1, 0) = (double) coefs.coef[0][1][1];
            mat_v(pMatrix, 1, 1) = (double) coefs.coef[0][1][2];
            mat_v(pMatrix, 1, 3) = (double) coefs.coef[0][1][0];
            break;

        case TRANSFO_PROJ_2D:
            {
                        double k1 = (double) (coefs.coef[0][0][3] - (coefs.coef[0][0][1] * coefs.coef[0][0][0])
                                               - (coefs.coef[0][0][2] * coefs.coef[0][1][0]));
                        double k2 = (double) (1.0 - (coefs.coef[0][0][4] * coefs.coef[0][0][0])
                                               - (coefs.coef[0][0][5] * coefs.coef[0][1][0]));
                        double k3 = (double) (coefs.coef[0][1][3] - (coefs.coef[0][1][1] * coefs.coef[0][0][0])
                                               - (coefs.coef[0][1][2] * coefs.coef[0][1][0]));

                mat_v(pMatrix, 0, 0) = (double) ((coefs.coef[0][0][1]
                                                                     + (coefs.coef[0][0][4]
                                                                                                         * coefs.coef[1][0][0])) / k2);
                mat_v(pMatrix, 0, 1) = (double) ((coefs.coef[0][0][2]
                                                                     + (coefs.coef[0][0][5]
                                                                                                         * coefs.coef[1][0][0])) / k2);
                mat_v(pMatrix, 0, 3) = (double) ((k1 + ( k2 * coefs.coef[1][0][0])) / k2);

                mat_v(pMatrix, 1, 0) = (double) ((coefs.coef[0][1][1]
                                                                     + (coefs.coef[0][0][4]
                                                                                                         * coefs.coef[1][1][0])) / k2);
                mat_v(pMatrix, 1, 1) = (double) ((coefs.coef[0][1][2]
                                                                     + (coefs.coef[0][0][5]
                                                                                                         * coefs.coef[1][1][0])) / k2);
                mat_v(pMatrix, 1, 3) = (double) ((k3 + ( k2 * coefs.coef[1][1][0])) / k2);

                mat_v(pMatrix, 3, 0) = (double) (coefs.coef[0][0][4] / k2);
                mat_v(pMatrix, 3, 1) = (double) (coefs.coef[0][0][5] / k2);
                mat_v(pMatrix, 3, 3) = 1.0;
            }
            break;

        default:
            HSET(Status, HERROR);
            goto WRAPUP;
    }



// ============================

    memcpy (po_pMatrix, pMatrix->mat_value, 16*sizeof(double));

    trf_end();



WRAPUP:

    mat_destroy(pMatrix);

    if (TrfCreated)
        trf_modelDestroy (TrfModel);

    if (TrfInitialized)
        trf_end();

    HRET (Status);
}

