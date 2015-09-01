/*--------------------------------------------------------------------------------------+
|
|     $Source: ext/MatrixFromTiePts/src/server.c $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
/*mh================================================================ HMR
**  Octimage/MicroStation - Transformation Models
**
**  This file contains the functions and table relative to the
**  management of mathematical models services. All the mathematical
**  models used in geocoding are accessible through this file.
**
**===========================================================================*/
#include <string.h>

#include "trfmodel.h"


/*
*****************************************************************************
**  Local structures and variables declarations
*****************************************************************************
*/

typedef struct tag_TRANSFO_MODEL_SERVER{
  int8_t                initialized;
  LIST                   modelList;             /* List containing all loaded models  */
  TRFHANDLE              modelHdl;              /* Handle of the current model */
  pTRANSFO_MODEL_DATA_3D pModel;                /* Copy of the current model   */
                                                /* All function pointers are for
                                                   the current model, NULL if none or invalid */

  int32_t (*calcCoef)(int32_t,pTRANSFO_MODEL_DATA_3D);
  void   (*transfoCoord)(int32_t,pCOEFFICIENTS_3D,pDCOORD_3D,pDCOORD_3D);
  void   (*transfoCoordPrep)(int32_t,pCOEFFICIENTS_3D,pDCOORD_3D,double *);
  void   (*transfoCoordDo)(int32_t,pCOEFFICIENTS_3D,pDCOORD_3D,pDCOORD_3D,double *);
  void   (*transfoCoordDoScanLine)(int32_t,pCOEFFICIENTS_3D,double,double,int32_t,pDCOORD_3D,double *);
  void   (*transfoDoScanLine)(int32_t,pCOEFFICIENTS_3D,double,double,int32_t,double *,TRANSFO_RSP_BUFF_INFO *,TRANSFO_RSP_COLOR_INFO *);

}TRANSFO_MODEL_SERVER;

typedef TRANSFO_MODEL_SERVER * pTRANSFO_MODEL_SERVER;

static TRANSFO_MODEL_SERVER s_trf = {FALSE};

static TRANSFO_MODEL_CODE_2D trf_transfoModel2D[] = {
{TRANSFO_MAT_2D   , N_PTS_MAT_2D   ,TRF_DEFAULT    , 0  , trf_transfoCoordMat2D , trf_transfoCoordPrepMat2D , trf_transfoCoordDoMat2D , trf_transfoCoordDoMatScanLine2D , trf_transfoDoMatScanLine2D },
{TRANSFO_TRAN_2D  , N_PTS_TRAN_2D  ,TRF_DEFAULT    , trf_calcCoefTran2D , trf_transfoCoordTran2D, trf_transfoCoordPrepTran2D, trf_transfoCoordDoTran2D, trf_transfoCoordDoTranScanLine2D, trf_transfoDoTranScanLine2D},
{TRANSFO_HELM_2D  , N_PTS_HELM_2D  ,TRF_NON_DEFAULT, trf_calcCoefHelm2D , trf_transfoCoordHelm2D, trf_transfoCoordPrepHelm2D, trf_transfoCoordDoHelm2D, trf_transfoCoordDoHelmScanLine2D, trf_transfoDoHelmScanLine2D},
{TRANSFO_SIMI_2D  , N_PTS_SIMI_2D  ,TRF_DEFAULT    , trf_calcCoefSimi2D , trf_transfoCoordSimi2D, trf_transfoCoordPrepSimi2D, trf_transfoCoordDoSimi2D, trf_transfoCoordDoSimiScanLine2D, trf_transfoDoSimiScanLine2D},
{TRANSFO_POLY_2D_1, N_PTS_POLY_2D_1,TRF_NON_DEFAULT, trf_calcCoefPoly2D1, trf_transfoCoordPoly2D, trf_transfoCoordPrepPoly2D, trf_transfoCoordDoPoly2D, trf_transfoCoordDoPolyScanLine2D, trf_transfoDoPolyScanLine2D},
{TRANSFO_AFFI_2D  , N_PTS_AFFI_2D  ,TRF_DEFAULT    , trf_calcCoefAffi2D , trf_transfoCoordAffi2D, trf_transfoCoordPrepAffi2D, trf_transfoCoordDoAffi2D, trf_transfoCoordDoAffiScanLine2D, trf_transfoDoAffiScanLine2D},
{TRANSFO_TPS_2D   , N_PTS_TPS_2D   ,TRF_NON_DEFAULT, trf_calcCoefTps2D  , trf_transfoCoordTps2D , trf_transfoCoordPrepTps2D , trf_transfoCoordDoTps2D , trf_transfoCoordDoTpsScanLine2D , trf_transfoDoTpsScanLine2D },
{TRANSFO_PROJ_2D  , N_PTS_PROJ_2D  ,TRF_DEFAULT    , trf_calcCoefProj2D , trf_transfoCoordProj2D, trf_transfoCoordPrepProj2D, trf_transfoCoordDoProj2D, trf_transfoCoordDoProjScanLine2D, trf_transfoDoProjScanLine2D},
{TRANSFO_POLY_2D_2, N_PTS_POLY_2D_2,TRF_DEFAULT    , trf_calcCoefPoly2D2, trf_transfoCoordPoly2D, trf_transfoCoordPrepPoly2D, trf_transfoCoordDoPoly2D, trf_transfoCoordDoPolyScanLine2D, trf_transfoDoPolyScanLine2D},
{TRANSFO_POLY_2D_3, N_PTS_POLY_2D_3,TRF_DEFAULT    , trf_calcCoefPoly2D3, trf_transfoCoordPoly2D, trf_transfoCoordPrepPoly2D, trf_transfoCoordDoPoly2D, trf_transfoCoordDoPolyScanLine2D, trf_transfoDoPolyScanLine2D},
{TRANSFO_POLY_2D_4, N_PTS_POLY_2D_4,TRF_DEFAULT    , trf_calcCoefPoly2D4, trf_transfoCoordPoly2D, trf_transfoCoordPrepPoly2D, trf_transfoCoordDoPoly2D, trf_transfoCoordDoPolyScanLine2D, trf_transfoDoPolyScanLine2D},
{TRANSFO_POLY_2D_5, N_PTS_POLY_2D_5,TRF_DEFAULT    , trf_calcCoefPoly2D5, trf_transfoCoordPoly2D, trf_transfoCoordPrepPoly2D, trf_transfoCoordDoPoly2D, trf_transfoCoordDoPolyScanLine2D, trf_transfoDoPolyScanLine2D},
{TRANSFO_NONE     , N_PTS_NONE     ,TRF_DEFAULT    , 0                  , 0                     , 0                         ,  0                      , 0                               , 0},
};

static void trf_setDefaultFunctions();
static int8_t trf_initialized();
static int32_t trf_getModelPosition(int32_t modelNo);




/*fh========================================================================
**
**  trf_init
**
**  Alain Lapierre  Feb 93 - Original version
**===========================================================================*/


int32_t trf_init()
  {

  /*
  ** Check if already initilized
  */
  if (trf_initialized())
    return(HSUCCESS);
  /*
  ** Initialize the list and set default starting values
  */

  if (list_create_darray(&s_trf.modelList,(int32_t)20,(int32_t) 20,0L,
                         sizeof(pTRANSFO_MODEL_DATA_3D)) != TRUE)
     return(TRF_ERROR_NO_MEMORY);

  s_trf.initialized      = TRUE;
  s_trf.modelHdl         = -1;
  s_trf.pModel           = NULL;
  s_trf.calcCoef         = NULL;
  s_trf.transfoCoord     = NULL;
  s_trf.transfoCoordPrep = NULL;
  s_trf.transfoCoordDo   = NULL;
/*&& AL Added transfoCoord for a Scan line */
  s_trf.transfoCoordDoScanLine   = NULL;
/*&&end Added transfoCoord for a Scan line */
  s_trf.transfoDoScanLine   = NULL;


  return(HSUCCESS);
  }


/*fh========================================================================
**
**  trf_end
**
**  Alain Lapierre  Feb 93 - Original version
**===========================================================================*/


int32_t trf_end()
  {
  pTRANSFO_MODEL_DATA_3D pModel;

  if(! trf_initialized())
     return(HERROR);

  /*
  ** Free the memory used by the transformation models and the list header.
  */

    if (list_first(&s_trf.modelList) >= 0L)
    {
        do
        {
            list_get(&s_trf.modelList,&pModel);
            if(pModel != NULL)
            {
                trf_transfoModelDestroy(pModel);
                free(pModel);
            }
        }
        while(list_next(&s_trf.modelList) >= 0);
    }

  list_destroy(&s_trf.modelList);

  s_trf.initialized      = FALSE;

  return(HSUCCESS);
  }


/*fh========================================================================
**
**  trf_modelNew
**
**  Alain Lapierre  Feb 93 - Original version
**===========================================================================*/


TRFHANDLE  trf_modelNew()
  {
  pTRANSFO_MODEL_DATA_3D pModel;
  TRFHANDLE              hdl;
  int32_t                x, y;

  if(! trf_initialized())
     return(-1);

  /*
  ** Allocate the memory for a transformation model
  */

  pModel = (pTRANSFO_MODEL_DATA_3D) malloc(sizeof(TRANSFO_MODEL_DATA_3D));
  if (pModel == NULL)
     return(-1);


    /*
    ** Thin Plate Spline
    */
    for (x=0; x<2; x++)
        for (y=0; y<3; y++)
            pModel->coef3D.tpsCoef[x][y] = NULL;

    pModel->coef3D.tpsPoints[TRANSFO_DIRECT]  = MAT_NULL;
    pModel->coef3D.tpsPoints[TRANSFO_INVERSE] = MAT_NULL;

    pModel->coef3D.nbTpsCoef[XIND]=0;
    pModel->coef3D.nbTpsCoef[YIND]=0;
    pModel->coef3D.nbTpsCoef[ZIND]=0;

    pModel->coef3D.tpsCoefIsAllocated = FALSE;

  /*
  ** Initialize it with default values
  */
  if(trf_transfoModelInit(pModel) != HSUCCESS)
     return(-1);

  /*
  ** Add it at the end of the list
  */

  list_append(&s_trf.modelList, &pModel);
  hdl = (TRFHANDLE)list_last(&s_trf.modelList);
  trf_modelSetCurrent(hdl);

  return(hdl);
  }


/*fh========================================================================
**
**  trf_modelDestroy
**
**  Alain Lapierre  Feb 93 - Original version
**===========================================================================*/


int32_t trf_modelDestroy(
TRFHANDLE  trfHdl)
  {
  if(! trf_initialized())
     return(HERROR);

  /*
  ** Set this model current
  */
  if(trf_modelSetCurrent(trfHdl) != HSUCCESS)
     return(HERROR);

  /*
  ** Free the memory used by this model and set the pointer to NULL
  */
  if(s_trf.pModel != NULL)
      {
      trf_transfoModelDestroy(s_trf.pModel);
      free(s_trf.pModel);
      }
  s_trf.pModel = NULL;
  list_set(&s_trf.modelList, &s_trf.pModel);

  return(HSUCCESS);
  }


/*fh========================================================================
**
**  trf_modelSetCurrent
**
**  Alain Lapierre  Feb 93 - Original version
**===========================================================================*/


int32_t trf_modelSetCurrent(
TRFHANDLE  trfHdl)
  {
  int32_t total;

  if(! trf_initialized())
     return(HERROR);

  /*
  ** Chech if the model is already current
  */
  if(trfHdl == s_trf.modelHdl)
     return(HSUCCESS);

  /*
  ** Find this model in the list
  */

  total = list_count(&s_trf.modelList);
  if(trfHdl >= total || trfHdl < 0)
     return(HERROR);

  list_goto(&s_trf.modelList, (int32_t) trfHdl);


  /*
  ** Make it current and set the function pointers according to this model type
  */

  list_get(&s_trf.modelList, &s_trf.pModel);
  s_trf.modelHdl = trfHdl;

  trf_setDefaultFunctions();


  return(HSUCCESS);
  }


/*fh========================================================================
**
**  trf_modelSetType
**
**  Alain Lapierre  Feb 93 - Original version
**===========================================================================*/


int32_t trf_modelSetType(TRFHANDLE  trfHdl,
                        int32_t    modelType)
  {
  if(! trf_initialized())
     return(HERROR);

  /*
  ** Make this model current
  */
  if(trf_modelSetCurrent(trfHdl) != HSUCCESS || s_trf.pModel == NULL)
     return(HERROR);

  /*
  ** Change the model type and pointers and update the function pointers
  */
  if(trf_getModelPosition(modelType) == -1)
     s_trf.pModel->modelType = TRANSFO_NONE;
  else
     s_trf.pModel->modelType = modelType;

  trf_setDefaultFunctions();

  return(HSUCCESS);
  }


/*fh========================================================================
**
**  trf_modelGetType
**
**  Alain Lapierre  Feb 93 - Original version
**===========================================================================*/


int32_t trf_modelGetType(TRFHANDLE  trfHdl,
                        int32_t    *modelType)
  {
  if(! trf_initialized())
     return(HERROR);

  /*
  ** Make this model current
  */
  if(trf_modelSetCurrent(trfHdl) != HSUCCESS || s_trf.pModel == NULL)
     return(HERROR);


  /*
  ** Return the model type
  */

  *modelType = s_trf.pModel->modelType;

  return(HSUCCESS);
  }


/*fh========================================================================
**
**  trf_modelSetCoordinateSystem
**
**  Alain Lapierre  Feb 93 - Original version
**===========================================================================*/


int32_t trf_modelSetCoordinateSystem(TRFHANDLE  trfHdl,
                                    char      *coordinateSystem)
  {
  if(! trf_initialized())
     return(HERROR);

  /*
  ** Make this model current
  */
  if(trf_modelSetCurrent(trfHdl) != HSUCCESS  || s_trf.pModel == NULL)
     return(HERROR);

  /*
  ** Change the coordinate system
  */
  strncpy(s_trf.pModel->modelDesc,coordinateSystem,L_MODEL_DESC);

  return(HSUCCESS);
  }


/*fh========================================================================
**
**  trf_modelGetCoordinateSystem
**
**  Alain Lapierre  Feb 93 - Original version
**===========================================================================*/


int32_t trf_modelGetCoordinateSystem(TRFHANDLE  trfHdl,
                                    char      *coordinateSystem)
  {
  if(! trf_initialized())
     return(HERROR);

  /*
  ** Make this model current
  */
  if(trf_modelSetCurrent(trfHdl) != HSUCCESS  || s_trf.pModel == NULL)
     return(HERROR);


  /*
  ** Copy the coordinate system
  */
  strncpy(coordinateSystem,s_trf.pModel->modelDesc,L_MODEL_DESC);
  return(HSUCCESS);
  }


/*fh========================================================================
**
**  trf_modelCalcCoef
**
**  Alain Lapierre  Feb 93 - Original version
**===========================================================================*/


int32_t trf_modelCalcCoef(TRFHANDLE  trfHdl,
                         int32_t    mapping)
  {
  if(! trf_initialized())
     return(HERROR);

  /*
  ** Make this model current
  */
  if(trf_modelSetCurrent(trfHdl) != HSUCCESS || s_trf.pModel == NULL)
     return(HERROR);

  /*
  ** If the function pointer is OK , calculates the coefficients.
  */
  if(s_trf.calcCoef == NULL)
     return(HERROR);

  return((*s_trf.calcCoef)(mapping,s_trf.pModel));

  }


/*fh========================================================================
**
**  trf_modelTransfoCoord
**
**  Alain Lapierre  Feb 93 - Original version
**===========================================================================*/


int32_t trf_modelTransfoCoord(TRFHANDLE  trfHdl,
                             pDCOORD_3D PtsIn,
                             pDCOORD_3D PtsOut,
                             int32_t    mapping)
  {
  if(! trf_initialized())
     return(HERROR);

  /*
  ** Make this model current
  */
  if(trf_modelSetCurrent(trfHdl) != HSUCCESS || s_trf.pModel == NULL)
     return(HERROR);

  /*
  ** If the function pointer is OK , transform the coordinates
  */

  if(s_trf.transfoCoord == NULL)
     return(HERROR);

  (*s_trf.transfoCoord)(mapping,&s_trf.pModel->coef3D, PtsIn, PtsOut);

  return(HSUCCESS);
  }


/*fh========================================================================
**
**  trf_modelTransfoCoordPrep
**
**  Alain Lapierre  Feb 93 - Original version
**===========================================================================*/


int32_t trf_modelTransfoCoordPrep(TRFHANDLE  trfHdl,
                                 pDCOORD_3D PtsIn,
                                 int32_t    mapping,
                                 double    *yProd)
  {
/*&&AL Improved performance */
#if 0
  if(! trf_initialized())
     return(HERROR);
#endif
/*&&end Improved performance */

  /*
  ** Make this model current
  */
  if(trf_modelSetCurrent(trfHdl) != HSUCCESS || s_trf.pModel == NULL)
     return(HERROR);



  /*
  ** If the function pointer is OK , compute the y products
  */
  if(s_trf.transfoCoordPrep == NULL)
     return(HERROR);

  (*s_trf.transfoCoordPrep)(mapping,&s_trf.pModel->coef3D, PtsIn, yProd);

  return(HSUCCESS);


  }


/*fh========================================================================
**
**  trf_modelTransfoCoordFinal
**
**  Alain Lapierre  Feb 93 - Original version
**===========================================================================*/


int32_t trf_modelTransfoCoordFinal(TRFHANDLE  trfHdl,
                                  pDCOORD_3D PtsIn,
                                  pDCOORD_3D PtsOut,
                                  int32_t    mapping,
                                  double    *yProd)
  {
/*&&AL Improved performance */
#if 0
  if(! trf_initialized())
     return(HERROR);
#endif
/*&&end Improved performance */

  /*
  ** Make this model current
  */
  if(trf_modelSetCurrent(trfHdl) != HSUCCESS || s_trf.pModel == NULL)
     return(HERROR);

  /*
  ** If the function pointer is OK , compute the final coordinates
  */
  if(s_trf.transfoCoordDo == NULL)
     return(HERROR);

  (*s_trf.transfoCoordDo)(mapping,&s_trf.pModel->coef3D, PtsIn, PtsOut, yProd);

  return(HSUCCESS);



  }


/*fh========================================================================
**
**  trf_ctrlPtsModif
**
**  Alain Lapierre  Feb 93 - Original version
**===========================================================================*/


int32_t trf_ctrlPtsModif(TRFHANDLE          trfHdl,
                         pCONTROL_POINTS_3D ctrlPts,
                         int32_t            operation)
  {
  int32_t ModelType;

  if(! trf_initialized())
     return(HERROR);

  /*
  ** Make this model current
  */
  if(trf_modelSetCurrent(trfHdl) != HSUCCESS || s_trf.pModel == NULL)
     return(HERROR);

  /*
  ** There is no control point for Matrix model
  */
  if (trf_modelGetType (trfHdl, &ModelType) != HSUCCESS && ModelType == TRANSFO_MAT_2D)
     return(HERROR);

  /*
  ** Execute the requested operation
  */
  switch(operation)
     {
     case CTRL_PTS_MODIF_INSERT:
        if(list_insert(&s_trf.pModel->ctrlPts3D, ctrlPts)== FALSE)
           return(HERROR);

        break;

     case CTRL_PTS_MODIF_APPEND:
        if(list_append(&s_trf.pModel->ctrlPts3D, ctrlPts)== FALSE)
           return(HERROR);

        break;

     case CTRL_PTS_MODIF_DELETE:
        if(list_delete(&s_trf.pModel->ctrlPts3D)== FALSE)
           return(HERROR);

        break;

     case CTRL_PTS_MODIF_SET:
        if(list_set(&s_trf.pModel->ctrlPts3D, ctrlPts)== FALSE)
           return(HERROR);

        break;

     case CTRL_PTS_MODIF_GET:
        if(list_get(&s_trf.pModel->ctrlPts3D, ctrlPts)== FALSE)
           return(HERROR);

        break;

     default:
        return(HERROR);

     }

  return(HSUCCESS);
  }



/*fh========================================================================
**
**  trf_modelGetCoefficients
**
**  Alain Lapierre  June 94 - Original version
**===========================================================================*/


int32_t trf_modelGetCoefficients(TRFHANDLE           trfHdl,
                                pCOEFFICIENTS_3D    coefs)
  {
  if(! trf_initialized())
     return(HERROR);

  /*
  ** Make this model current
  */
  if(trf_modelSetCurrent(trfHdl) != HSUCCESS || s_trf.pModel == NULL)
     return(HERROR);


  /*
  ** Return the model type
  */

  *coefs = s_trf.pModel->coef3D;

  return(HSUCCESS);
  }


/*fh========================================================================
**
**  trf_modelSetCoefficients
**
**  Jean Papillon  November 97 - Original version
**===========================================================================*/


int32_t trf_modelSetCoefficients(TRFHANDLE           pi_TrfHandle,
                                pCOEFFICIENTS_3D    pi_pCoefs)
  {
  int32_t ModelType;
  int32_t Retour;

  if(! trf_initialized())
     return(HERROR);

  /*
  ** Make this model current
  */
  if(trf_modelSetCurrent(pi_TrfHandle) != HSUCCESS || s_trf.pModel == NULL)
     return(HERROR);


  /*
  ** This service is available for TRANSFO_MAT_2D only
  */
  if (trf_modelGetType (pi_TrfHandle, &ModelType) != HSUCCESS && ModelType != TRANSFO_MAT_2D)
     return(HERROR);


  /*
  ** Dispatch processing to specific model (currently, TRANSFO_MAT_2D is the only model
  ** (currently, TRANSFO_MAT_2D is the only model supporting this service)
  */
  Retour = trf_SetCoefficientsMat2D (pi_pCoefs, s_trf.pModel);


  return (Retour);
  }

/*sh========================================================================
**  trf_setDefaultFunctions
**
**  DESCRIPTION
**
**      The trf_setDefaultFunctions() should be called any time the model type
**      is changed in the current model structure (s_trf).
**      The purpose of this is to optimize the access to the default
**      function when we call it repeatidly (ex: for resampling)
**
**
**  PARAMETERS
**
**
**  RETURN VALUE
**
**      none
**
**  Alain Lapierre  Feb 93 - Original version
**===========================================================================*/
static
void trf_setDefaultFunctions()
  {
  int   position;

  if ((s_trf.pModel == NULL) || (position = trf_getModelPosition(s_trf.pModel->modelType)) == -1)
     {
     s_trf.calcCoef         = NULL;
     s_trf.transfoCoord     = NULL;
     s_trf.transfoCoordPrep = NULL;
     s_trf.transfoCoordDo   = NULL;
     s_trf.transfoCoordDoScanLine   = NULL;
     s_trf.transfoDoScanLine   = NULL;
     }
  else
     {
     s_trf.calcCoef         = trf_transfoModel2D[position].calcCoef;
     s_trf.transfoCoord     = trf_transfoModel2D[position].transfoCoord;
     s_trf.transfoCoordPrep = trf_transfoModel2D[position].transfoCoordPrep;
     s_trf.transfoCoordDo   = trf_transfoModel2D[position].transfoCoordDo;
     s_trf.transfoCoordDoScanLine   = trf_transfoModel2D[position].transfoCoordDoScanLine;
     s_trf.transfoDoScanLine   = trf_transfoModel2D[position].transfoDoScanLine;
     }
  }

/*sh========================================================================
**  trf_initialized
**
**  DESCRIPTION
**
**
**  PARAMETERS
**
**
**  RETURN VALUE
**
**      none
**
**  Alain Lapierre  Feb 93 - Original version
**===========================================================================*/
static
int8_t trf_initialized()
  {
  return(s_trf.initialized);
  }


/*sh========================================================================
**  trf_getModelPosition
**
**  DESCRIPTION
**
**      The trf_getModelPosition finds in the trf_transfoModel2D the
**      position of this model.
**
**  PARAMETERS
**      modelNo  model number (use the macros in TRFMODEL.H ex: TRANSFO_SIMI_2D)
**
**  RETURN VALUE
**      position of this model in the table trf_transfoModel2D (starting at 0)
**      TRANSFO_NONE if this model was not found in the table
**
**
**  Alain Lapierre  May 94 - Original version
**===========================================================================*/
static
int32_t trf_getModelPosition(
int32_t modelType)
  {
  int i;

  if(modelType <= TRANSFO_NONE ||
     modelType > TRANSFO_MAX)
     {
     return(TRANSFO_NONE);
     }

  for(i=0; i<TRANSFO_MAX; i++)
     {
     if(modelType == trf_transfoModel2D[i].modelType)
        return(i);
     }

  return(TRANSFO_NONE);
  }


