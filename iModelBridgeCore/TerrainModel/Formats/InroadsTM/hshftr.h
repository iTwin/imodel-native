//---------------------------------------------------------------------------+
// $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+
/*----------------------------------------------------------------------------*/
/* hshftr.h                                          aec    11-Dec-1998       */
/*----------------------------------------------------------------------------*/
/* Function prototypes for various DTM utilities functions.                   */
/*----------------------------------------------------------------------------*/

#pragma once

/*----------------------------------------------------------------------------*/
/* Dependent include files                                                    */
/*----------------------------------------------------------------------------*/

#include <dtmstr.h>

/*----------------------------------------------------------------------------*/
/* Function prototypes                                                        */
/*----------------------------------------------------------------------------*/

int aecDTM_hashFeatureCreate
(
  struct CIVdtmsrf *srfP
);

int aecDTM_hashAllFeatures
(
  struct CIVdtmsrf *srfP
);

int aecDTM_hashInsertFeature
(
  struct CIVdtmsrf *srfP,
  struct CIVdtmftr *ftrP
);

int aecDTM_hashDeleteFeature
(
  struct CIVdtmsrf *srfP,
  struct CIVdtmftr *ftrP
);

int aecDTM_hashFindFeatureByGUID
(
  CIVdtmftr **ftrPP,
  struct CIVdtmsrf *srfP,
  BeGuid *guidP
);

int aecDTM_hashFindFeatureByName
(
  CIVdtmftr **ftrP,
  struct CIVdtmsrf *srfP,
  wchar_t *name
);

void aecDTM_hashFeatureDestroy
(
  struct CIVdtmsrf *srfP
);
