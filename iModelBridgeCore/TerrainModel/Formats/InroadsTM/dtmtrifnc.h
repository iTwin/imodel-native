//---------------------------------------------------------------------------+
// $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+
/*----------------------------------------------------------------------------*/
/* dtmtrifnc.h                                           aec    08-Feb-1994   */
/*----------------------------------------------------------------------------*/
/* Function prototypes for functions used during triangularization process.   */
/*----------------------------------------------------------------------------*/

#pragma once

/*----------------------------------------------------------------------------*/
/* Dependent include files                                                    */
/*----------------------------------------------------------------------------*/

#include <dtmstr.h>
#include <dtmtri.h>

/*----------------------------------------------------------------------------*/
/* Function prototypes                                                        */
/*----------------------------------------------------------------------------*/

int aecDTM_triangulate /* <= TRUE if error                 */
(
  long *numPointsP,                    /* <= # points triangulated (or NULL)  */
  long *numTrianglesP,                 /* <= # triangles generated (or NULL)  */
  long *timeP,                         /* <= triang. time (secs) (or NULL)    */
  struct CIVdtmsrf *srfP,              /* => surface to triangulate           */
  int options,                         /* => options                          */
  double *maxTriangleLengthP,          /* => max. tri length (or NULL)        */
  byte *extDataChecksP,                /* => TRUE, FALSE (or NULL)            */
  double *ftrFilterToleranceP,         /* => feature filter tol. (or NULL)    */
  void *distriP,           /* => triangle display pars (or NULL)  */
  boolean useAltMoveMethod = FALSE     /* => user alternate move to 0,0,0 method */
);

int aecDTM_patch            /* <= TRUE if error                    */
(
  struct CIVdtmsrf *srfP,              /* => surface to use                   */
  long npntlst,                        /* => # of points in hole perimeter    */
  long *pntlstP,                       /* => list of points                   */
  long *nbrlstP,                       /* => list of neighboring triangles    */
  long *nnewlstP,                      /* => # of new triangles in hole       */
  long **newlstPP                      /* => list of new triangle in hole     */
);

int aecDTM_patchHole        /* <= TRUE if error                    */
(
  struct CIVdtmsrf *srfP,              /* => surface to use                   */
  long npnt,                           /* => # of points in hole perimeter    */
  long *pntlstP,                       /* => list of points                   */
  long *neilstP,                       /* => list of neighboring triangles    */
  long *nnewlstP,                      /* => # of new triangles in hole       */
  long **newlstPP                      /* => list of new triangle in hole     */
);

int aecDTM_patchDelaunay    /* <= TRUE if error                    */
(
  struct CIVdtmsrf *srfP,              /* => surface to use                   */
  long npntlst,                        /* => # points in perimter polygon     */
  long *pntlstP,                       /* => list of ptrs to perimeter pnts   */
  long nnewlst,                        /* => # triangles in new list          */
  long *newlstP                        /* => list of new triangles ptrs       */
);

void aecDTM_patchCheck
(
  long *npntlstP,                      /* => # points in list                 */
  long *pntlstP,                       /* => point list                       */
  long *nbrlstP                        /* => neighbor triangle list           */
);

void aecDTM_delaunayTriangleInitRecursionDepthCheck();

int aecDTM_delaunayTriangle /* <= TRUE if error                    */
(
  struct CIVdtmsrf *srfP,              /* => surface to use                   */
  struct CIVdtmpnt *pP,                /* => point being added                */
  struct CIVdtmtin *aP,                /* => current triangle                 */
  long *ntinstkP,                      /* => # tins on tin stack              */
  long **tinstkPP                      /* => tin stack triangles              */
);

int aecDTM_delaunayTriangleCheck /* <= TRUE if error               */
(
  struct CIVdtmsrf *srfP,              /* => surface to use                   */
  struct CIVdtmpnt *pntP,              /* => point being added                */
  struct CIVdtmtin *tinP,              /* => starting triangle                */
  struct CIVdtmpnt *paP,               /* => triangle edge point to use       */
  struct CIVdtmpnt *pbP,               /* => 2nd triangle edge point to use   */
  long *ntinstkP,                      /* => # tins on tin stack              */
  long **tinstkPP                      /* => triangle stack                   */
);

int aecDTM_triangleStack    /* <=  TRUE if error                   */
(
  long *ntinstkP,                      /* <=> # triangles on stack            */
  long **tinstkPP,                     /* <=> triangle stack                  */
  struct CIVdtmtin *tinP               /*  => triangle to add to stack        */
);

int aecDTM_triangleStackPut /* <= TRIE if error                    */
(
  struct CIVdtmsrf *srfP,              /* => surface to use                   */
  struct CIVdtmtin *tinP,              /* => triangle to put on stack         */
  int dis                              /* => TRUE: display input triangle     */
);

int aecDTM_triangleStackGet /* <= TRUE if error                    */
(
  struct CIVdtmtin **tinPP,            /* <= retrieved triangle               */
  struct CIVdtmsrf *srfP               /* => surface to use                   */
);

int aecDTM_triangulatePoint /* <= TRUE if error                    */
(
  struct CIVdtmsrf *srfP,              /* => surface to use                   */
  struct CIVdtmfil *filP,              /* => dtm file containing point        */
  struct CIVdtmpnt *pntP,              /* => point to triangulate             */
  struct CIVdtmtin **tinPP,            /* => triangle to start process in     */
  long *ntinstkP,                      /* => # triangles on triangle stack    */
  long **tinstkPP                      /* => pointer to triangles on stack    */
);

int aecDTM_triangulateLinear /* <= TRUE if error                   */
(
  void *tmp,                           /* => triangulate data structure       */
  int typ,                             /* => type of input points             */
  long np,                             /* => # points in string               */
  DPoint3d *p,                         /* => array of point coordinates       */
  struct CIVdtmpnt *pnt                /* => pointer to first linear point    */
);

int aecDTM_triangulateLinearFree /* <= TRUE if error               */
(
  struct CIVtinLineData *dat           /* => internal data structure          */
);

void aecDTM_triangulateClearErrorPoint
(
);

void aecDTM_triangulateSetErrorPoint
(
    struct CIVdtmpnt *dtmPntP
);

int aecDTM_validateTinPtr   /* <= TRUE if error                    */
(
    struct CIVdtmblk **inpblkP,          /* <= block where triangle is        */
    struct CIVdtmsrf *srfP,              /* => surface to use                 */
    struct CIVdtmtin *tinP               /* => triangle to use                */
);
