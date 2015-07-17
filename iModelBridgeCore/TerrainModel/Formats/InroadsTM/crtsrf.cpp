//---------------------------------------------------------------------------+
// $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+
/*----------------------------------------------------------------------------*/
/* crtsrf.c                                            tmi    24-Jan-1994     */
/*----------------------------------------------------------------------------*/
/* It creates a brand new surface.                                            */
/*----------------------------------------------------------------------------*/



/*----------------------------------------------------------------------------*/
/* Include files                                                              */
/*----------------------------------------------------------------------------*/
#include "stdafx.h"

/*----------------------------------------------------------------------------*/
/* Extrenal functions                                                              */
/*----------------------------------------------------------------------------*/


/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_createSurface
 DESC: Create a surface without a data structure using this one.
 HIST: Original - tmi 15-Jan-1993
 MISC:
 KEYW: DTM SURFACE CREATE PROJECT ALLOCATE
-----------------------------------------------------------------------------%*/

int aecDTM_createSurface /* <= TRUE if error                       */
(
  struct CIVdtmsrf **srfPP,         /* <= surface created (or NULL)           */
  struct CIVdtmprj *prjP,           /* => project to use (or NULL)            */
  wchar_t *nameP,                   /* => name                                */
  wchar_t *descP,                   /* => description (or NULL)               */
  wchar_t *fileP,                   /* => file name (or NULL)                 */
  wchar_t *matP,                    /* => material name (or NULL)             */
  double maxTriLength,              /* => max tri length (normally 0.)        */
  double scale,                     /* => scale (normally 0.)                 */
  long type                         /* => surface type                        */
)
{
  return aecDTM_createSurfaceEx ( srfPP, prjP, nameP, descP,
                                  fileP, matP, maxTriLength, scale, TRUE, type );
}


/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_createSurfaceEx
 DESC: Create a surface without a data structure using this one.  This gives
       the option to not updated the explorer.
 HIST: Original - tmi 15-Jan-1993
 MISC:
 KEYW: DTM SURFACE CREATE PROJECT ALLOCATE
-----------------------------------------------------------------------------%*/

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
  long type                         /* => type                                */
)
{
  struct CIVdtmsrf *srfP = (struct CIVdtmsrf *)0;
  int sts = SUCCESS;
  unsigned long version = DTM_C_DTMVER;

  if ( prjP == (struct CIVdtmprj *)0 )
    aecDTM_getActiveProject ( &prjP, (wchar_t *)0, (wchar_t *)0 );

  if ( prjP == (struct CIVdtmprj *)0 )
    sts = DTM_M_NOPRJS;
  else if ( ( sts = aecDTM_findSurfaceByName ( (struct CIVdtmsrf **)0, prjP, nameP ) ) == SUCCESS )
    sts = DTM_M_DUPSNM;
  else
    sts = aecDTM_allocateSurface ( &srfP, prjP, fileP, nameP, descP, &scale, &maxTriLength, &version, matP, type );

  if ( sts == SUCCESS && srfP )
  {
    aecGuid_generate ( &srfP->guid );
    aecDTM_setSurfaceModifiedFlag ( srfP );
    // TODO:Account for those rare cases where prjP is actually nonNULL.
	//		Keep a running list here of cases/places where it's observed:
	//		(1) when loading DTM files
  }

  if ( srfPP != (struct CIVdtmsrf **)0 )
  {
    *srfPP = srfP;
  }

  return ( sts );
}

