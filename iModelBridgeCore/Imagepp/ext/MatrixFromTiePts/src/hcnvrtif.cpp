/*--------------------------------------------------------------------------------------+
|
|     $Source: ext/MatrixFromTiePts/src/hcnvrtif.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Function sHConvertTIF_MakeModelerFromScaleAndTiePts
//-----------------------------------------------------------------------------

#include "oldhtypes.h"
#include "hcnvrtif.h"
#include "matrix.h"
#include "trfmoids.h"
#include "trfmodel.h"
#include "hbasemod.h"

/*fh========================================================================
** sHConvertTIF_MakeModelerFromScaleAndTiePts
**
** DESCRIPTION: This function constructs a HBasicModeler object from a
**              list of one tie point and a scale factor or from several
**              tie point and no scale.
**
**
** PARAMETERS:  po_pMatrix              -> A pointer to a contructed pMat_ object
**              pi_NbVal_GeoTiePoint    -> Number of tie point in pi_pVal_GeoTiePoint vector
**              pi_pVal_GeoTiePoint     -> Pointer to a vector of tie point
**              pi_NbVal_GeoPixelScale  -> Number of tie point in pi_pVal_GeoPixelscale vector
**              pi_pVal_GeoPixelScale   -> Pointer to a vector of pixel scale
**
**
** RETURNS:     This function returns H_SUCCESS on success. It returns H_ERROR
**              otherwise
**
**  Marc Bedard 98/08/31 - Original Version
**  Cedric Boulanger 00/07/25 - Derived from original version
**===========================================================================*/
HSTATUS sHConvertTIF_MakeModelerFromScaleAndTiePts (pMAT_           po_pMatrix,
                                                    uint16_t       pi_NbVal_GeoTiePoint,
                                                    double        *pi_pVal_GeoTiePoint,
                                                    uint16_t       pi_NbVal_GeoPixelScale,
                                                    double        *pi_pVal_GeoPixelScale)
{
    HDEF    (Status, H_SUCCESS);
    int32_t             i;
    TRFHANDLE           TrfModel;
    int8_t             TrfCreated=FALSE;
    CONTROL_POINTS_3D   ctrlPts;
    int32_t            NbPts=0;
    int32_t            PtsPosition=0;
    int32_t             TransfoModel=TRANSFO_AFFI_2D;
    double             ScaleX=1.0;
    double             ScaleY=1.0;
    double             ScaleZ=0.0;
    HBasicModeler       Modeler;
    int8_t             ModelerCreated=FALSE;

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
    if (NbPts==1)
    {
        TransfoModel=TRANSFO_TRAN_2D;
    }
    else if (NbPts==2)
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

    /*
    ** Create a Basic Modeler from the trf_model
    */
    if (HISERROR (HBasicModeler_ConstructorFromTrfModel (&Modeler, TrfModel)))
    {
        HSET (Status, H_ERROR);
        goto WRAPUP;
    }
    ModelerCreated = TRUE;

    /*
    ** Add scaling to Modeler if present
    */
    if (pi_NbVal_GeoPixelScale==3)
    {
        DCOORD  Offset;

        /*
        ** Set scale factor into basic modeler
        */
        ScaleX=pi_pVal_GeoPixelScale[0];
        ScaleY=pi_pVal_GeoPixelScale[1];
        ScaleZ=pi_pVal_GeoPixelScale[2];

        /*
        ** we must scale from translation point
        */
        if (HISERROR (HBasicModeler_GetTranslation (&Modeler,&Offset.x,&Offset.y)))
        {
            HSET (Status, H_ERROR);
            goto WRAPUP;
        }

        if (HISERROR (HBasicModeler_AddScalingXY (&Modeler,&Offset,ScaleX,ScaleY)))
        {
            HSET (Status, H_ERROR);
            goto WRAPUP;
        }
    }

     if ((HBasicModeler_GetMatrix(&Modeler, po_pMatrix)) != HSUCCESS)
    {
        HSET(Status,H_ERROR);
        goto WRAPUP;
    }


WRAPUP:

    if (TrfCreated)
        trf_modelDestroy (TrfModel);

    if (ModelerCreated)
        HBasicModeler_Destructor(&Modeler);

    HRET (Status);
}

