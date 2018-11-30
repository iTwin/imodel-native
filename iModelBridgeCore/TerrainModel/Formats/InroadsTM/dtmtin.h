//---------------------------------------------------------------------------+
// $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+
/*----------------------------------------------------------------------------*/
/* dtmtin.h                                              aec    08-Feb-1994   */
/*----------------------------------------------------------------------------*/
/* Function prototypes for various utility DTM triangle related functions.    */
/*----------------------------------------------------------------------------*/

#pragma once

/*----------------------------------------------------------------------------*/
/* Dependent include files                                                    */
/*----------------------------------------------------------------------------*/

#include <dtmstr.h>

/*----------------------------------------------------------------------------*/
/* Function prototypes                                                        */
/*----------------------------------------------------------------------------*/

int aecDTM_swapNeighboringTriangles /* <= TRUE if error            */
(
  struct CIVdtmsrf *srfP,              /* => surface to use                   */
  struct CIVdtmtin *aP,                /* => first triangle                   */
  struct CIVdtmtin *bP                 /* => first triangle's neighbor        */
);

int aecDTM_checkTrianglesForSwap /* <= TRUE if NOT swappable       */
(
  struct CIVdtmtin *aP,                /* => first triangle                   */
  struct CIVdtmtin *bP                 /* => first triangle's neighbor        */
);

int aecDTM_getTriangleSideIndex /* <= TRUE if error                */
(
  long *sidaP,                         /* <= triangle A side index            */
  long *sidbP,                         /* <= triangle B side index            */
  struct CIVdtmtin *aP,                /* => first triangle                   */
  struct CIVdtmtin *bP,                /* => first triangle's neighbor        */
  BOOL bSetErrorPoint                  /* => set error point if no side is found*/
);

void aecDTM_updateTriangleNeighbor
(
  struct CIVdtmtin *tinP,              /* => first triangle                   */
  struct CIVdtmtin *neiP,              /* => neighboring triangle             */
  struct CIVdtmtin *newtinP            /* => triangle to replace first tin    */
);

int aecDTM_isTriangleRangeTriangle /* <= TRUE if range triangle    */
(
  struct CIVdtmsrf *srfP,              /* => surface to use                   */
  struct CIVdtmtin *tinP               /* => triangle to use                  */
);
