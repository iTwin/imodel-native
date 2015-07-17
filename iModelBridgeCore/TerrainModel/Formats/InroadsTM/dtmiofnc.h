//---------------------------------------------------------------------------+
// $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+
/*----------------------------------------------------------------------------*/
/* dtmiofnc.h                                         aec    07-Feb-1994      */
/*----------------------------------------------------------------------------*/
/* Function prototypes for DTM load/save & import/export functions.           */
/*----------------------------------------------------------------------------*/

#pragma once

/*----------------------------------------------------------------------------*/
/* Dependent include files                                                    */
/*----------------------------------------------------------------------------*/

#include <dtmstr.h>
#include <dtmio.h>
#include <dtmttn.h>

/*----------------------------------------------------------------------------*/
/* Function prototypes                                                        */
/*----------------------------------------------------------------------------*/

int aecDTM_load             /* <= TRUE if error                    */
(
  struct CIVdtmsrf **srfPP,            /* <= new, loaded surface              */
  int *fileWasTTN,                     /* <= TRUE if TTN load (or NULL)       */
  struct CIVdtmprj *prjP,              /* => DTM project (or NULL)            */
  wchar_t *fileNameP                   /* => file to load                     */
);

int aecDTM_loadDTM          /* <= TRUE if error                    */
(
  struct CIVdtmsrf **srfPP,            /* => pointer to new loaded srf        */
  struct CIVdtmprj *prjP,              /* => DTM project (or NULL)            */
  wchar_t *fileNameP                   /* => file to load                     */
);

int aecDTM_save             /* <= TRUE if error                    */
(
  struct CIVdtmsrf *srfP,              /* => surface to save                  */
  wchar_t *fileNameP,                  /* => file name to save to             */
  unsigned long version                /* => version to save (0 = latest)     */
);

void aecDTM_buildPtrIndexTable
(
  struct CIVdtmsrf *srfP,              /* => surface to use                   */
  struct CIVptrind *ptrP,              /* => point/index table                */
  size_t *pTinOffset            /* # points before first tin (or NULL) */
);

void aecDTM_convertIndexToPtr
(
  struct CIVdtmsrf *srfP,              /* => surface to use                   */
  struct CIVptrind *ptrP,              /* => first entry of p/i table         */
  struct CIVptrind *eptrP              /* => last entry of p/i table          */
);

void aecDTM_convertPtrToIndex
(
  size_t *ftrP,                        /* => address of thing to convert      */
  struct CIVptrind *ptrP,              /* => first entry in p/i table         */
  struct CIVptrind *eptrP,             /* => last entry in p/i table          */
  size_t *sizP                         /* => size of input entry              */
);

void aecDTM_sortPtrIndexTableByPtr
(
  struct CIVptrind *ptrP,              /* => first entry in p/i table         */
  struct CIVptrind *eptrP              /* => last entry in p/i table          */
);

void aecDTM_sortPtrIndexTableByIndex
(
  struct CIVptrind *ptrP,              /* => first entry in p/i table         */
  struct CIVptrind *eptrP              /* => last entry in p/i table          */
);

int aecDTM_indexTableBuild  /* <= TRUE if error                    */
(
  struct CIVdtmsrf *srfP               /* => surface to use                   */
);

int aecDTM_indexTableFree   /* <= TRUE if error                    */
(
  struct CIVdtmsrf *srfP               /* => surface to use                   */
);

int CIVadfsrf               /* <= TRUE if error                    */
(
  struct CIVdtmsrf *srfP,              /* =>                                  */
  wchar_t *fileNameP                   /* =>                                  */
);

