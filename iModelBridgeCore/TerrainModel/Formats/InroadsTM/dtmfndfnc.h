//---------------------------------------------------------------------------+
// $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+
/*----------------------------------------------------------------------------*/
/* dtmfndfnc.h                                         aec    07-Feb-1994     */
/*----------------------------------------------------------------------------*/
/* Function prototypes for functions which find DTM related things.           */
/*----------------------------------------------------------------------------*/

#pragma once

/*----------------------------------------------------------------------------*/
/* Dependent include files                                                    */
/*----------------------------------------------------------------------------*/

#include <dtmstr.h>
#include <dtmfnd.h>

/*----------------------------------------------------------------------------*/
/* Function prototypes                                                        */
/*----------------------------------------------------------------------------*/

int aecDTM_getElevationAtXY /* <=  TRUE if error                   */
(
  DPoint3d *pntP,                      /* <=> point's loc and elev.           */
  struct CIVdtmsrf *srfP,              /*  => surface to use                  */
  struct CIVdtmtin **seedTriPP,        /*  => seed tri. (or NULL)             */
  BOOL bIgnoreRangeTins = FALSE,       /* <=                                  */
  BOOL bIgnoreDeletedTins = FALSE      /* <=                                  */
);

void aecDTM_getElevationAtXYGivenTriangle  /* <=  TRUE if error    */
(
  DPoint3d *pP,                        /* <=> location and elev.              */
  DPoint3d *triP                       /*  => triangle to use                 */
);

int aecDTM_findSurfaceByName /* <= TRUE if error                   */
(
  struct CIVdtmsrf **srfPP,            /* <= found surface                    */
  struct CIVdtmprj *prjP,              /* => DTM project (or NULL)            */
  wchar_t *srfNameP                    /* => surface name pointer             */
);

int aecDTM_findSurfaceByGUID /* <= NULL if error                   */
(
  struct CIVdtmsrf **srfPP,             /* <= found surface                   */
  struct CIVdtmprj *prjP,               /* => DTM project (or NULL)           */
  BeGuid *srfGUIDp                        /* => surface BeGuid                    */
);

int aecDTM_findPointFile /* <= TRUE if error                       */
(
  struct CIVdtmfil **filPP,            /* <= found file                       */
  struct CIVdtmsrf *srfP,              /* => surface to look in               */
  struct CIVdtmpnt *pntP               /* => point within file                */
);

int aecDTM_findPointType    /* <= TRUE if error               */
(
  long *pointTypeP,                    /* <= point's type                     */
  long *pointTypeMaskP,                /* <= point's type as masked value     */
  struct CIVdtmblk **inpblkP,          /* <= block where point is             */
  struct CIVdtmsrf *srfP,              /* => surface to use                   */
  struct CIVdtmpnt *pP                 /* => point to use                     */
);

int aecDTM_findTriangle     /* <=  TRUE if error                   */
(
  struct CIVdtmtin **tinPP,            /* <=> starting/found triangle         */
  int *rngP,                           /* <=  TRUE: range triangle            */
  int *rptP,                           /* <=  TRUE: input pnt is tin pnt      */
  int *sidP,                           /* <=  TRUE: input pnt on tin side     */
  struct CIVdtmsrf *srfP,              /*  => surface to use                  */
  DPoint3d *corP                       /*  => coordinates to use              */
);

int aecDTM_isPointInsideTriangle /* <= TRUE if point inside        */
(
  struct CIVdtmtin **triPP,            /* <=> starting/found triangle         */
  int *fndP,                           /* <=  TRUE: range triangle            */
  int *trptP,                          /* <=  TRUE: input pnt is tin pnt      */
  int *tsidP,                          /* <=  TRUE: input pnt on tin side     */
  DPoint3d *pntP,                      /* =>  point to use                    */
  DPoint3d *tP                         /* =>  triangle coordinates            */
);

int aecDTM_findFirstTriangle /* <= TRUE if error                   */
(
  struct CIVdtmtin **tinPP,            /* <= found triangle                   */
  struct CIVdtmsrf *srfP               /* => surface to use                   */
);

int aecDTM_findAffectedTriangles /* <= TRUE if error               */
(
  struct CIVdtmtin **triPP,            /* <=> starting/first triangle         */
  long *ntinlstP,                      /* <=  # tins in list                  */
  long **tinlstPP,                     /* <=  array of tin addresses          */
  struct CIVdtmsrf *srfP,              /*  => surface to use                  */
  DPoint3d *locP                       /*  => loc. of pnt to be added         */
);

int aecDTM_findOutsideTriangle /* <= TRUE if error                 */
(
  struct CIVdtmtin **tinPP,            /* <= found triangle                   */
  struct CIVdtmtin **prvtinPP,         /* <= previous triangle                */
  struct CIVdtmsrf *srfP               /* => surface to use                   */
);

void aecDTM_findNonDeletedNeighborTriangle
(
  struct CIVdtmtin **inputTinPP,       /* <=> current/neighbor trian.         */
  int rpt,                             /*  => TRUE: repeat point              */
  int side                             /*  => TRUE: point falls onside        */
);

int aecDTM_getSurfacePerimeter /* <= TRUE if error                 */
(
  long *nvrtP,                         /* <= # of perimeter vertices          */
  DPoint3d **vrtPP,                    /* <= per. verts. (free)        */
  struct CIVdtmsrf *srfP               /* => surface to use                   */
);

void aecDTM_getSurfaceFileIndex
(
  long *indexP,                        /* <= file index                       */
  long pointType                       /* => point type                       */
);

void aecDTM_getSurfaceFeatureFileIndex
(
  long *indexP,                       /* <= file index                        */
  long featureType                    /* => feature type                      */
);

int aecDTM_findFeatureByGUID /* <= TRUE if error                   */
(
  struct CIVdtmftr **ftrPP,             /* <= found feature                   */
  struct CIVdtmsrf *srfP,               /* => DTM surface (or NULL)           */
  BeGuid *guidP                           /* => surface guid pointer            */
);

int aecDTM_findFeatureByName /* <= TRUE if error                   */
(
  struct CIVdtmftr **ftrPP,             /* <= found feature                   */
  struct CIVdtmsrf *srfP,               /* => DTM surface (or NULL)           */
  wchar_t *ftrNameP                        /* => surface name pointer            */
);

int aecDTM_findExteriorFeature /* <= TRUE if error                 */
(
    struct CIVdtmftr **ftrPP,             /* <= found feature                 */
    struct CIVdtmsrf *srfP                /* => DTM surface (or NULL)         */
);

int aecDTM_findFeatureByPoint  /* <= TRUE if error                   */
(
    struct CIVdtmftr **ftrPP,             /* <= found feature                   */
    struct CIVdtmsrf *srfP,               /* => DTM surface (or NULL)           */
    struct CIVdtmpnt *pntP                /* => feature's point                 */
);

int aecDTM_findFeatureType   /* <= TRUE if error                   */
(
  long *typP,                          /* <= feature's type                   */
  struct CIVdtmblk **inpblkP,          /* <= block where feature is           */
  struct CIVdtmsrf *srfP,              /* => surface to use                   */
  struct CIVdtmftr *fP                 /* => feature to use                   */
);

int aecDTM_findFeatureFile  /* <= TRUE if error                    */
(
  struct CIVdtmfil **filPP,            /* <= found file                       */
  struct CIVdtmsrf *srfP,              /* => surface to look in               */
  struct CIVdtmftr *ftrP               /* => point within file                */
);

int aecDTM_findStyleBlock   /* <= TRUE if error                    */
(
  struct CIVdtmblk **inpblkP,          /* <= block where feature is           */
  struct CIVdtmsrf *srfP,              /* => surface to use                   */
  struct CIVdtmsty *sP                 /* => style to use                     */
);

int aecDTM_removeChildlessFtrs
(
  struct CIVdtmsrf *srfP
);
