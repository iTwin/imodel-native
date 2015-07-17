//---------------------------------------------------------------------------+
// $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+
/*----------------------------------------------------------------------------*/
/* dtmmemfnc.h                                           aec    07-Feb-1994   */
/*----------------------------------------------------------------------------*/
/* Functions prototypes for functions which allocated and deallocate memory   */
/* for DTM data structures.                                                   */
/*----------------------------------------------------------------------------*/

#pragma once

/*----------------------------------------------------------------------------*/
/* Dependent include files                                                    */
/*----------------------------------------------------------------------------*/

#include <dtmstr.h>
#include <dtmmem.h>

/*----------------------------------------------------------------------------*/
/* Function prototypes                                                        */
/*----------------------------------------------------------------------------*/

int aecDTM_createSurface /* <= TRUE if error               */
(
  struct CIVdtmsrf **srfPP,            /* <= surface created (or NULL)        */
  struct CIVdtmprj *prjP,              /* => project to use (or NULL)         */
  wchar_t *nameP,                      /* => name                             */
  wchar_t *descP,                      /* => description (or NULL)            */
  wchar_t *fileP,                      /* => file name (or NULL)              */
  wchar_t *matP,                       /* => material name (or NULL)          */
  double maxTriLength,                 /* => max tri length (normally 0.)     */
  double scale,                        /* => scale (normally 0.)              */
  long type = 0                        /* => surface type                     */
);

int aecDTM_createSurfaceEx /* <= TRUE if error                     */
(
  struct CIVdtmsrf **srfPP,         /* <= surface created (or NULL)           */
  struct CIVdtmprj *prjP,           /* => project to use (or NULL)            */
  wchar_t *nameP,                   /* => name                                */
  wchar_t *descP,                   /* => description (or NULL)               */
  wchar_t *fileP,                   /* => file name (or NULL)                 */
  wchar_t *matP,                    /* => material name (or NULL)             */
  double maxTriLength,              /* => max tri length (normally 0.)        */
  double scale,                     /* => scale (normally 0.)                 */
  int updateExplorer,               /* => update explorer window with surface */
  long type = 0                     /* => surface type                        */
);

int aecDTM_deleteProject
(
  void
);

int aecDTM_deleteSurface    /* <= TRUE if error                    */
(
  struct CIVdtmprj *prjP,              /* => DTM prj w/ srf (or NULL)         */
  struct CIVdtmsrf *srfP,              /* => surface to delete                */
  int emptyOnly                        /* => TRUE: just empty surface         */
);

int aecDTM_deleteSurfaceEx  /* <= TRUE if error                    */
(
  struct CIVdtmprj *prjP,              /* => DTM prj w/ srf (or NULL)         */
  struct CIVdtmsrf *srfP,              /* => surface to delete                */
  int emptyOnly,                       /* => TRUE: just empty surface         */
  int updateExplorer                   /* => remove surface from explorer win */
);

int aecDTM_allocateSurface  /* <= TRUE if error                    */
(
  struct CIVdtmsrf **srfPP,            /* <= ptr to new surface (or NULL)     */
  struct CIVdtmprj *prjP,              /* => DTM prj to use (or NULL)         */
  wchar_t *filP,                       /* => surface file name (or NULL)      */
  wchar_t *namP,                       /* => surface name (or NULL)           */
  wchar_t *desP,                       /* => surface desc (or NULL)           */
  double *sclP,                        /* => surface scale (or NULL)          */
  double *mtlP,                        /* => max. tri. length (or NULL)       */
  unsigned long *versionP,             /* => version # (or NULL)              */
  wchar_t *matP,                       /* => surface material (or NULL)       */
  long type = 0                        /* => surface type                     */
);

int aecDTM_allocateFile     /* <= TRUE if error                    */
(
  struct CIVdtmfil **filPP,            /* <= ptr to allocated file            */
  long pointType                       /* => type of point                    */
);

int aecDTM_allocateBlock    /* <= TRUE if error                    */
(
  struct CIVdtmblk **blkPP,            /* <= ptr to allocated block           */
  struct CIVdtmfil *filP,              /* => file to allocate block in        */
  long nrec,                           /* => # records to allocate            */
  int frc                              /* => TRUE: force fract. alloc.        */
);

int aecDTM_deallocateFile   /* <= TRUE if error                    */
(
  struct CIVdtmfil *filP               /* => DTM file to deallocate           */
);

int aecDTM_deallocateBlock  /* <= TRUE if error                    */
(
  struct CIVdtmblk *blkP               /* => block of memory to dealloc.      */
);

int aecDTM_deallocateTriangleStack /* <= TRUE of error             */
(
  struct CIVdtmsrf *srfP               /* => srf with stack to free           */
);

int aecDTM_addSurfaceToProject /* <= TRUE if error                 */
(
  struct CIVdtmprj *prjP,              /* => dtm project to use (or NULL)     */
  struct CIVdtmsrf *srfP               /* => surface to use                   */
);

int aecDTM_removeSurfaceFromProject /* <= TRUE if error            */
(
  struct CIVdtmprj *prjP,              /* => DTM project to use (or NULL)     */
  struct CIVdtmsrf *srfP               /* => surface to remove                */
);
