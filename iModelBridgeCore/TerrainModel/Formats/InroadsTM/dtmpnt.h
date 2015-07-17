//---------------------------------------------------------------------------+
// $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+
/*----------------------------------------------------------------------------*/
/* dtmpnt.h                                          aec    08-Feb-1994       */
/*----------------------------------------------------------------------------*/
/* Function prototypes for various utility DTM point functions.               */
/*----------------------------------------------------------------------------*/

#pragma once

/*----------------------------------------------------------------------------*/
/* Dependent include files                                                    */
/*----------------------------------------------------------------------------*/

#include <dtmstr.h>

/*----------------------------------------------------------------------------*/
/* Function prototypes                                                        */
/*----------------------------------------------------------------------------*/

void aecDTM_removeConstructionPoints
(
  struct CIVdtmsrf *srfP               /* => surface to process               */
);

int aecDTM_isPointRangePoint /* <= TRUE if point is range point    */
(
  struct CIVdtmsrf *srfP,              /* => surface to use                   */
  struct CIVdtmpnt *pntP               /* => point to check                   */
);

int aecDTM_getIndexOfTrianglePointByCoordinates /* <= TRUE if error */
(
  int *pntIndexP,                      /* <= point's index (0, 1, 2, 3)       */
  DPoint3d *pntP,                      /* => point coordinates to use         */
  DPoint3d *tP                         /* => array of all triangle coords.    */
);

int aecDTM_getIndexOfTrianglePointByPointer
(
  int *pntIndexP,                      /* <= point's index (0, 1, 2, 3)       */
  struct CIVdtmpnt *pP,                /* => point to use                     */
  struct CIVdtmtin *tP                 /* => triangle to use                  */
);

int aecDTM_isPointInsideBoundary /* < TRUE if point is ins.        */
(
  struct CIVdtmsrf *srfP,              /* => surface to use                   */
  struct CIVdtmpnt *pntP               /* => point to use                     */
);

int aecDTM_isPointFirstLinearPoint /* <= TRUE if first linear pnt. */
(
  struct CIVdtmpnt *pntP               /* => point to check                   */
);

void aecDTM_removeDuplicateDTMPoints
(
  long *nvrtP,                         /* <=> # of vertices in line.          */
  CIVdtmpnt *vrtP,                     /* <=> linestring vertices             */
  double tolerance                     /*  => tolerance to use                */
);

