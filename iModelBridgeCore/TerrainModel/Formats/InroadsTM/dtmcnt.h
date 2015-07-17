//---------------------------------------------------------------------------+
// $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+
/*----------------------------------------------------------------------------*/
/* dtmcnt.h                                          aec    08-Feb-1994       */
/*----------------------------------------------------------------------------*/
/* Function prototypes for functions which count DTM entities.                */
/*----------------------------------------------------------------------------*/

#pragma once

/*----------------------------------------------------------------------------*/
/* Dependent include files                                                    */
/*----------------------------------------------------------------------------*/

#include <dtmstr.h>

/*----------------------------------------------------------------------------*/
/* Function prototypes                                                        */
/*----------------------------------------------------------------------------*/

void aecDTM_countSurfaceBlocks
(
  int *nblkP,                          /* <= # of blocks                      */
  struct CIVdtmsrf *srfP               /* => surface to use                   */
);

void aecDTM_countSurfacePoints
(
  long *npntP,                         /* <= # points in surface              */
  struct CIVdtmsrf *srfP               /* => surface to use                   */
);

void aecDTM_countSurfacePointsInFile
(
  struct CIVdtmfil *filP               /* => file to count                    */
);

void aecDTM_countSurfaceTriangle
(
  long *ntinP,                         /* <= # triangles in surface           */
  struct CIVdtmsrf *srfP               /* => surface to use                   */
);

void aecDTM_countSurfaceData
(
  long *npntP,                         /* <= # points in surface              */
  long *ntinP,                         /* <= # triangles in surface           */
  struct CIVdtmsrf *srfP               /* => surface to use                   */
);

void aecDTM_countSurfaceDataInFile
(
  long *nrecP,                         /* <= # records in file                */
  struct CIVdtmsrf *srfP,              /* => surface to use                   */
  int typ                              /* => point type                       */
);

void aecDTM_countPointsInLinearFeature
(
  long *npntP,                         /* <= # points in feature              */
  struct CIVdtmsrf *srfP,              /* => surface containing feature       */
  struct CIVdtmblk *inpblkP,           /* => block cont. ftr. (or NULL)       */
  struct CIVdtmpnt *pP                 /* => first point in linear ftr.       */
);

void aecDTM_countBoundaries
(
  long *numIntP,                       /* <= # of interior boundaries         */
  long *numExtP,                       /* <= # of exterior boundaries         */
  struct CIVdtmsrf *srfP               /* => surface to use                   */
);

void aecDTM_countSurfaceFeatures
(
  long *nftrP,                         /* <= # features in surface             */
  struct CIVdtmsrf *srfP               /* => surface to use                    */
);
