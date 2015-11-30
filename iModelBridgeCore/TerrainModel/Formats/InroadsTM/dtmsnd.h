//---------------------------------------------------------------------------+
// $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+
/*----------------------------------------------------------------------------*/
/* dtmsnd.h                                          aec    07-Feb-1994       */
/*----------------------------------------------------------------------------*/
/* Function prototypes for functions used to send data to user's function.    */
/*----------------------------------------------------------------------------*/

#pragma once

/*----------------------------------------------------------------------------*/
/* Dependent include files                                                    */
/*----------------------------------------------------------------------------*/

#include <dtmstr.h>

/*----------------------------------------------------------------------------*/
/* Function prototypes                                                        */
/*----------------------------------------------------------------------------*/

int aecDTM_sendAllPoints    /* <= TRUE if error                    */
(
  void *mdlDescP,                      /* => mdl app desc. (or NULL)          */
  struct CIVdtmsrf *srfP,              /* => surface to use                   */
  int opt,                             /* => options                          */
  int typmsk,                          /* => point type (zero for all)        */
  int (*usrfncP)(void *,int,long,      /* => your function                    */
       DPoint3d *,struct CIVdtmpnt *),
  void *datP                           /* => your data                        */
);

int aecDTM_sendAllPointsHonoringFence /* <= TRUE if error          */
(
  void *mdlDescP,                      /* => mdl app. desc (or NULL)          */
  struct CIVdtmsrf *srfP,              /* => surface to use                   */
  int opt,                             /* => options                          */
  int typmsk,                          /* => point type (zero for all)        */
  int (*usrfncP)(void *,int,long,      /* => your function                    */
     DPoint3d *,struct CIVdtmpnt *),
  void *usrdatP                        /* => your data                        */
);

int aecDTM_sendAllTriangles /* <= TRUE if error                    */
(
  void *mdlDescP,                      /* => mdl app desc. (or NULL)          */
  struct CIVdtmsrf *srfP,              /* => surface with triangles           */
  int opt,                             /* => options                          */
  int (*usrfncP)(void *,long,          /* => your function                    */
      DPoint3d *,struct CIVdtmtin *,unsigned long),
  void *datP                           /* => your data                        */
);

int aecDTM_sendAllFeatures  /* <= TRUE if error                    */
(
  void *mdlDescP,                      /* => mdl app desc. (or NULL)          */
  struct CIVdtmsrf *srfP,              /* => surface with triangles           */
  int opt,                             /* => options                          */
  int typmsk,                          /* => point type (zero for all)        */
  int (*usrfncP)(void *,               /* => your function                    */
       struct CIVdtmsrf *,int,
       struct CIVdtmftr *),
  void *datP                           /* => your data                        */
);
