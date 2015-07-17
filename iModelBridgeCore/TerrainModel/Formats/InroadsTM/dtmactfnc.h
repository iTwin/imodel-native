//---------------------------------------------------------------------------+
// $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+
/*----------------------------------------------------------------------------*/
/* dtmactfnc.h                                       aec    07-Feb-1994       */
/*----------------------------------------------------------------------------*/
/* Function prototypes for functions which set and get active DTM entities.   */
/*----------------------------------------------------------------------------*/

#pragma once

/*----------------------------------------------------------------------------*/
/* Dependent include files                                                    */
/*----------------------------------------------------------------------------*/

#include <dtmstr.h>

/*----------------------------------------------------------------------------*/
/* Function prototypes                                                        */
/*----------------------------------------------------------------------------*/
int aecDTM_getActiveSurface /* <= TRUE if error            */
(
  struct CIVdtmsrf **srfPP,            /* <= pointer to active surface        */
  wchar_t *srfNameP,                   /* <= name of active surface           */
  wchar_t *srfDescP                    /* <= description of active surf.      */
);

int aecDTM_getActiveProject /* <= TRUE if error                    */
(
  struct CIVdtmprj **prjPP,            /* <= pointer to active project        */
  wchar_t *prjNameP,                   /* <= active surface's name            */
  wchar_t *prjDescP                    /* <= active surface's desc.           */
);

int aecDTM_getActiveSurface /* <= TRUE if error                   */
(
  struct CIVdtmsrf **srfPP,          /* <= pointer to active surface          */
  wchar_t *srfNameP,                 /* <= name of active surface             */
  wchar_t *srfDescP                  /* <= description of active surf.        */
);
