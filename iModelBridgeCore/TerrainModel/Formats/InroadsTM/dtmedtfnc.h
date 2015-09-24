//---------------------------------------------------------------------------+
// $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+
/*----------------------------------------------------------------------------*/
/* dtmedtfnc.h                                           aec    08-Feb-1994   */
/*----------------------------------------------------------------------------*/
/* Function prototypes for DTM point and area editing functions.              */
/*----------------------------------------------------------------------------*/

#pragma once

/*----------------------------------------------------------------------------*/
/* Dependent include files                                                    */
/*----------------------------------------------------------------------------*/

#include <dtmstr.h>
#include <dtmedt.h>

/*----------------------------------------------------------------------------*/
/* Function prototypes                                                        */
/*----------------------------------------------------------------------------*/
int aecDTM_addPointsExt     /* <= TRUE if error                    */
(
  struct CIVdtmpnt **pntPP,            /* <= pntr to 1st pnt (or NULL)        */
  struct CIVdtmsrf *srfP,              /* => surface to add pnts to           */
  long pointType,                      /* => type of point to add             */
  long ncor,                           /* => # of points being added          */
  DPoint3d *corP                       /* => list of point coordinates        */
);

int aecDTM_addPointsToFile  /* <= TRUE if error                    */
(
  struct CIVdtmpnt **pntPP,            /* <= ptr to 1st pnt (or NULL)         */
  struct CIVdtmsrf *srfP,              /* => surface to add points to         */
  struct CIVdtmfil *filP,              /* => file ptr to add pnts to          */
  long pointType,                      /* => type of pnt being added          */
  int closeString,                     /* => TRUE: ensure closed ply.         */
  long ncor,                           /* => # of points being added          */
  DPoint3d *corP                       /* => list of point coords.            */
);

int aecDTM_addPointsToFileEx  /* <= TRUE if error                  */
(
  struct CIVdtmpnt **pntPP,              /* <= ptr to 1st pnt (or NULL)       */
  struct CIVdtmsrf *srfP,                /* => surface to add points to       */
  struct CIVdtmfil *filP,                /* => file ptr to add pnts to        */
  long typ,                              /* => type of pnt being added        */
  int closeString,                       /* => TRUE: ensure closed ply.       */
  long ncor,                             /* => # of points being added        */
  DPoint3d *corP,                        /* => list of point coords.          */
  long nPntsAlc                          /* => # pnts to allocate             */
);

int aecDTM_addPointsCheck   /*  <= TRUE if error                   */
(
  struct CIVdtmsrf *srfP,              /*  => srf pnts are added to           */
  long typ,                            /*  => type of points we're add.       */
  long *ncorP,                         /* <=> # coords being added            */
  DPoint3d *corP,                      /*  => list of coordinates             */
  int *closeStringP,                   /* <=  TRUE: close polygons            */
  int deallocExteriors                 /*  => TRUE or FALSE                   */
);

int aecDTM_addFeature /* <= TRUE if error                  */
(
  BeGuid *guidP,                          /*<=  guid of created feature         */
  struct CIVdtmsrf *srfP,               /* => surface to add feature to       */
  long opt,                             /* => DTM_C_APPEND, etc.              */
  wchar_t *name,                        /* => name of feature                 */
  wchar_t *description,                 /* => feature description (or NULL)   */
  wchar_t *parentName,                  /* => name of parent feature (or NULL)*/
  long featureType,                     /* => type of feature to add          */
  long numPnts,                         /* => # of points being added         */
  CIVdtmpnt *ftrPntsP,                  /* => list of feature points          */
  DPoint3d *pntsP,                      /* => list of point coordinates       */
  unsigned char *pntFlgsP,                    /* => list of point properties        */
  double density,                       /* => density interval                */
  CIVdtmstynam *stylesP,                /* => feature's styles (or NULL)      */
  long numStyles,                       /* => # of styles                     */
  CIVdtmpaynam *payItemsP,              /* => pay items (or NULL)             */
  long numPayItems,                     /* => # of pay items                  */
  unsigned char flag,                          /* => feature's flags                 */
  BOOL bRemoveDuplicates = TRUE,        /* => should almost always be TRUE    */
  BOOL bCloseString = TRUE              /* => should almost always be TRUE    */
);

int aecDTM_addFeatureCheck  /*  <= TRUE if error                   */
(
  struct CIVdtmsrf *srfP,              /*  => srf pnts are added to           */
  long ftrTyp,                         /*  => type of feature we're adding.   */
  long *numPntsP,                      /* <=> # coords being added            */
  CIVdtmpnt *pntsP,                    /*  => list of coordinates             */
  int *closeStringP,                   /* <=  TRUE: close polygons            */
  int deallocExteriors,                /*  => TRUE or FALSE                   */
  BOOL bRemoveDuplicates = TRUE        /*  => should almost always be TRUE    */
);

int aecDTM_addFeaturePointsToFile /* <= TRUE if error              */
(
  struct CIVdtmpnt **pntPP,             /* <= ptr to 1st pnt (or NULL)        */
  struct CIVdtmsrf *srfP,               /* => surface to add points to        */
  struct CIVdtmfil *filP,               /* => file ptr to add pnts to         */
  long typ,                             /* => type of pnt being added         */
  int closeString,                      /* => TRUE: ensure closed ply.        */
  long ncor,                            /* => # of points being added         */
  CIVdtmpnt *corP,                      /* => list of point coords.           */
  long nPntsAlc                         /* => # pnts to allocate              */
);

int aecDTM_addTriangle      /* <= TRUE if error                    */
(
  struct CIVdtmtin **tinPP,            /* <= ptr pointing to new triangle     */
  struct CIVdtmsrf *srfP,              /* => surface where tin is added       */
  struct CIVdtmpnt *p1P,               /* => ptr to first vertice point.      */
  struct CIVdtmpnt *p2P,               /* => ptr to second vertice point.     */
  struct CIVdtmpnt *p3P,               /* => ptr to third vertice point.      */
  struct CIVdtmtin *n12P,              /* => ptr to tri neigh. side 1-2       */
  struct CIVdtmtin *n23P,              /* => ptr to tri neigh. side 2-3       */
  struct CIVdtmtin *n31P               /* => ptr to tri neigh. side 3-1       */
);

int aecDTM_pointAdd         /* <= TRUE if error                    */
(
  struct CIVpntedt *pntaddP            /* => point add data structure         */
);

int aecDTM_deletePoint      /* <= TRUE if error                    */
(
  struct CIVdtmsrf *srfP,              /* => surface with point (or NULL)     */
  struct CIVdtmfil *inpfilP,           /* => file with point (or NULL)        */
  struct CIVdtmpnt *pntP               /* => point to delete                  */
);

int aecDTM_pointDelete      /* <= TRUE if error                    */
(
  struct CIVpntedt *pntdelP            /* => point edit data structure        */
);

void aecDTM_pointDeleteCleanup
(
  struct CIVdtmsrf *srfP,              /* => surface your using               */
  struct CIVdtmpnt *pntP,              /* => point we deleted                 */
  long nnewlst,                        /* => # tins in affected list          */
  long *newlstP                        /* => array of ptrs to triangle list   */
);

void aecDTM_deleteTriangle
(
  struct CIVdtmsrf *srfP,              /* => surface containing tin           */
  struct CIVdtmtin *tinP,              /* => triangle to delete               */
  int display                          /* => -1: don't dis, else do           */
);

void aecDTM_undeleteTriangle
(
  struct CIVdtmsrf *srfP,              /* => surface containing tin           */
  struct CIVdtmtin *tinP,              /* => triangle to delete               */
  int display                          /* => -1: don't dis, else do           */
);

int aecDTM_rotateAroundPoint /* <= TRUE if error                   */
(
  struct CIVdtmpnt **npntPP,           /* <= neigh. triangle (or NULL)        */
  struct CIVdtmtin **ntinPP,           /* <= neigh. point (or NULL)           */
  int *pindP,                          /* <= neigh. point index (or NULL)     */
  int *tindP,                          /* <= neigh. triangle index (or NULL)  */
  struct CIVdtmpnt *pntP,              /* => point to rotate about            */
  struct CIVdtmtin *tinP,              /* => current triangle                 */
  int dir                              /* => 0: rotate clock,  1: ctr-clock.  */
);

int aecDTM_getPointNeighbors /* <= TRUE if error                   */
(
  long *npntlstP,                      /* <= # point neigh. (or NULL)         */
  long **pntlstPP,                     /* <= list of neigh. points (or NULL)  */
  long *ntinlstP,                      /* <= # triangle neigh. (or NULL)      */
  long **tinlstPP,                     /* <= list of neigh. tri. (or NULL)    */
  long *nnbrlstP,                      /* <= # tri. neigh. tri. (or NULL)     */
  long **nbrlstPP,                     /* <= list of tri. neigh. tri. (or NULL) */
  struct CIVdtmsrf *srfP,              /* => surface to use                   */
  struct CIVdtmpnt *pntP,              /* => point to rotate about            */
  struct CIVdtmtin *inptinP,           /* => current triangle                 */
  int dir                              /* => 0: rotate clock, 1: ctr-clock.   */
);

int aecDTM_deleteFeatureByGUID /* <= TRUE if error                 */
(
  struct CIVdtmsrf *srfP,              /*  => surface with feature (or NULL)  */
  BeGuid *guidP                          /*  => guid of feature to delete       */
);

int aecDTM_deleteFeature /* <= TRUE if error                         */
(
  struct CIVdtmsrf *srfP,         /* => surface with feature (or NULL)          */
  struct CIVdtmfil *inftrfilP,    /* => file with feature (or NULL)             */
  struct CIVdtmfil *inpntfilP,    /* => file with feature's points ( or NULL )  */
  struct CIVdtmftr *ftrP          /* => feature to delete                       */
);

int aecDTM_deleteStyle      /* <= TRUE if error                    */
(
  struct CIVdtmsrf *srfP,              /* => surface with point (or NULL)     */
  struct CIVdtmsty *styP               /* => style to delete                  */
);

int aecDTM_deleteAllFeatures
(
  struct CIVdtmsrf *srfP,              /* => surface with triangles           */
  int typmsk                           /* => feature type (zero for all)      */
);

