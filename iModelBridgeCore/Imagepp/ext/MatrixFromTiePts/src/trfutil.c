/*--------------------------------------------------------------------------------------+
|
|     $Source: ext/MatrixFromTiePts/src/trfutil.c $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
/*mh================================================================ HMR
**  Octimage/MicroStation - Transformation Models
**
**      This file contains the utilities functions for the different
**      transformation models.
**
**===========================================================================*/

#include "trfmodel.h"


/*fh=========================================================================
**
**  trf_findNoActivePoints
**
**  Alain Lapierre  3/2/93
**===========================================================================*/

int32_t trf_findNoActivePoints(pLIST lPoints)
  {
  int32_t nbPointsMax, nbPointsActive, n;
  pCONTROL_POINTS_3D ctrl;

  ctrl=trf_getCtrlPtsAddr(lPoints);
  nbPointsMax = (int32_t)list_count(lPoints);

  for(nbPointsActive=0, n=0; n < nbPointsMax; n++)
     if (ctrl[n].pointActive)
        nbPointsActive++;
  return(nbPointsActive);
  }


/*fh=========================================================================
**
**  trf_copyActivePoints
**
**  Alain Lapierre  3/2/93
**===========================================================================*/

void trf_copyActivePoints(pMAT_    pts,
                           pLIST    lPoints,
                           uint32_t  badOrOk,
                           uint32_t  xOrY,
                           double  *minx,
                           double  *miny)
  {
  int src,dest;
  long nPoints;
  pCONTROL_POINTS_3D ctrl;
  long nActivePoints;


  /*
  ** Point to the beginning of the list of points
  */

  ctrl=trf_getCtrlPtsAddr(lPoints);

  /*
  ** Find the mean value of the list
  ** which will serve as offset for the translation.
  ** This is done to make too big numbers smaller enough
  ** for future matrix operations (pow and square operations)
  ** (We divide at each iteration because summing them before
  **  dividing could cause an overflow)
  */

  nPoints = list_count(lPoints);
  nActivePoints = trf_findNoActivePoints(lPoints);

  *minx = *miny = 0.0;

  for(src=0; src < nPoints; src++)
     {
     if (ctrl[src].pointActive)
        {
        if (badOrOk == PTS_BAD)
           {
           *minx += ctrl[src].pointBad.x / nActivePoints;
           *miny += ctrl[src].pointBad.y / nActivePoints;
           }
        else
           {
           *minx += ctrl[src].pointOk.x / nActivePoints;
           *miny += ctrl[src].pointOk.y / nActivePoints;
           }
        }
     }

  /*
  ** Copy the active points in the MATRIX
  ** and substract the offset (minimum value) from the copied coordinate
  */
  for(src=0, dest=0; src < nPoints; src++)
     {
     if (ctrl[src].pointActive)
        {
        if (xOrY & COPY_X)
           {
           if (badOrOk == PTS_BAD)
             mat_v(pts,dest++,0) = ctrl[src].pointBad.x - *minx;
           else
             mat_v(pts,dest++,0) = ctrl[src].pointOk.x  - *minx;
           }
        if (xOrY & COPY_Y)
           {
           if (badOrOk == PTS_BAD)
             mat_v(pts,dest++,0) = ctrl[src].pointBad.y - *miny;
           else
             mat_v(pts,dest++,0) = ctrl[src].pointOk.y  - *miny;
           }
              }
     }

  }


/*fh=========================================================================
**
**  trf_transfoModelInit
**
**  Alain Lapierre  3/2/93
**===========================================================================*/

int32_t trf_transfoModelInit(pTRANSFO_MODEL_DATA_3D modele)
  {
  modele->modelType = TRANSFO_NONE;
  modele->modelDesc[0] = '\0';
  if (list_create_darray(&modele->ctrlPts3D, (int32_t)L_CONTROL_POINTS,
                         (int32_t) L_CONTROL_POINTS,0L,
                         sizeof(CONTROL_POINTS_3D)) != TRUE)
     return(TRF_ERROR_NO_MEMORY);
  modele->coef3D.nbCoef[XIND]=0;
  modele->coef3D.nbCoef[YIND]=0;
  modele->coef3D.nbCoef[ZIND]=0;
  modele->toleranceRadian = 0.000048; /* 1/10 min. d'arc */
  modele->toleranceMeter  = 0.001;    /* 1 millimeter */

  return(HSUCCESS);
  }

/*fh=========================================================================
**
**  trf_transfoModelDestroy
**
**  Alain Lapierre  3/2/93
**===========================================================================*/

void trf_transfoModelDestroy(
pTRANSFO_MODEL_DATA_3D modele)
  {
  /*
  ** Free the thin plate spline coefficients
  */
  if (modele->coef3D.tpsCoefIsAllocated == TRUE)
  {
      trf_freeTpsCoefficients (&modele->coef3D);
  }

  list_destroy (&modele->ctrlPts3D);
  }

/*fh=========================================================================
**
**  trf_getCtrlPtsAddr
**
**  Alain Lapierre  3/2/93
**===========================================================================*/

pCONTROL_POINTS_3D trf_getCtrlPtsAddr(
pLIST lCtrl)
  {
  pCONTROL_POINTS_3D ctrlAddr;
  int32_t current;

  current=list_position(lCtrl);
  list_first(lCtrl);
  list_get_addr(lCtrl,(void **)&ctrlAddr);
  list_goto(lCtrl,current);
  return(ctrlAddr);
  }

