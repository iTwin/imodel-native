//---------------------------------------------------------------------------+
// $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+
/*----------------------------------------------------------------------------*/
/* dtmflgfnc.h                                       aec    08-Feb-1994       */
/*----------------------------------------------------------------------------*/
/* Function prototypes for functions which modify flags within DTM entities.  */
/*----------------------------------------------------------------------------*/

#pragma once

/*----------------------------------------------------------------------------*/
/* Dependent include files                                                    */
/*----------------------------------------------------------------------------*/

#include <dtmflg.h>

/*----------------------------------------------------------------------------*/
/* Function prototypes                                                        */
/*----------------------------------------------------------------------------*/

void aecDTM_markRangeTriangles
(
  struct CIVdtmsrf *srfP               /* => surface to use                   */
);

void aecDTM_unmarkRangeTriangles
(
  struct CIVdtmsrf *srfP               /* => surface to use                   */
);

int aecDTM_markBoundaryTriangles /* <= TRUE if error               */
(
  struct CIVdtmsrf *srfP               /* => surface to use                   */
);

void aecDTM_setTriangleSideBreaklineFlag
(
  struct CIVdtmpnt *p1P,               /* => first breakline point            */
  struct CIVdtmpnt *p2P,               /* => second breakline point           */
  struct CIVdtmtin *tinP               /* => triangle                         */
);

int aecDTM_isPointProcessedFlagSet /* <= TRUE if set               */
(
  struct CIVdtmpnt *pntP               /* => point to check                   */
);

void aecDTM_setPointDeletedFlag
(
  struct CIVdtmpnt *pntP               /* => point to use                     */
);

int aecDTM_isPointDeletedFlagSet /* <= TRUE if set                 */
(
  struct CIVdtmpnt *pntP               /* => point to check                   */
);

void aecDTM_setPointConstructionFlag
(
  struct CIVdtmpnt *pntP               /* => point to use                     */
);

int aecDTM_isPointConstructionFlagSet /* <= TRUE if set            */
(
  struct CIVdtmpnt *pntP               /* => point to check                   */
);

void aecDTM_clearAllTrianglesProcessedFlag
(
  struct CIVdtmsrf *srfP               /* => surface to use                   */
);

void aecDTM_clearTriangleProcessedFlag
(
  struct CIVdtmtin *tinP               /* => triangle to use                  */
);

void aecDTM_setTriangleProcessedFlag
(
  struct CIVdtmtin *tinP               /* => triangle to use                  */
);

int aecDTM_isTriangleProcessedFlagSet /* <= TRUE if set            */
(
  struct CIVdtmtin  *tinP              /* => triangle to check                */
);

void aecDTM_clearTriangleDeletedFlag
(
  struct CIVdtmtin *tinP               /* => triangle to use                  */
);

void aecDTM_setTriangleDeletedFlag
(
  struct CIVdtmtin *tinP               /* => triangle to use                  */
);

int aecDTM_isTriangleDeletedFlagSet /* <= TRUE if set              */
(
  struct CIVdtmtin  *tinP              /* => triangle to check                */
);

void aecDTM_clearTriangleRemovedFlag
(
  struct CIVdtmtin *tinP               /* => triangle to use                  */
);

void aecDTM_setTriangleRemovedFlag
(
  struct CIVdtmtin *tinP               /* => triangle to use                  */
);

int aecDTM_isTriangleRemovedFlagSet /* <= TRUE if set              */
(
  struct CIVdtmtin  *tinP              /* => triangle to check                */
);

void aecDTM_clearSurfaceModifiedFlag
(
  struct CIVdtmsrf *srfP               /* => surface to clear bit in          */
);

void aecDTM_setSurfaceModifiedFlag
(
  struct CIVdtmsrf *srfP               /* => surface to clear bit in          */
);

int aecDTM_isSurfaceModifiedFlagSet /* <= TRUE if set              */
(
  struct CIVdtmsrf *srfP               /* => surface to check                 */
);

void aecDTM_clearSurfaceTinOutOfDateFlag
(
  struct CIVdtmsrf *srfP                  /* => surface to clear bit in */
);

void aecDTM_setSurfaceTinOutOfDateFlag
(
  struct CIVdtmsrf *srfP                  /* => surface to set bit in */
);

void aecDTM_setFeatureDeletedFlag
(
  struct CIVdtmftr *ftrP               /* => feature to use                   */
);

int aecDTM_isFeatureDeletedFlagSet /* <= TRUE if set               */
(
  struct CIVdtmftr  *ftrP              /* => feature to check                 */
);

void aecDTM_setStyleDeletedFlag
(
  struct CIVdtmsty *styP               /* => feature to use                   */
);

int aecDTM_isStyleDeletedFlagSet /* <= TRUE if set                 */
(
  struct CIVdtmsty  *styP              /* => feature to check                 */
);

int aecDTM_isPayItemDeletedFlagSet /* <= TRUE if set        */
(
  struct CIVdtmpay  *payP              /* => pay item to check         */
);

void aecDTM_clearAllPointsIgnoreSegmentFlag
(
  struct CIVdtmsrf *srfP                 /* => surface to use           */
);

void aecDTM_clearPointIgnoreSegmentFlag
(
  struct CIVdtmpnt *pntP               /* => point to use                     */
);

int aecDTM_isLockTrianglesFlagSet /* <= TRUE if set       */
(
  struct CIVdtmsrf *srfP               /* => surface to check         */
);

int aecDTM_isSurfaceTinOutOfDateFlagSet /* <= TRUE if set       */
(
  struct CIVdtmsrf *srfP                 /* => surface to check         */
);
