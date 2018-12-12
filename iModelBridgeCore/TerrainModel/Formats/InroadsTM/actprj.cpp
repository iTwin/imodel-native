//---------------------------------------------------------------------------+
// $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+
/*----------------------------------------------------------------------------*/
/* actprj.c                                    tmi    07-Jun-1990             */
/*----------------------------------------------------------------------------*/
/* It is used to define the active project.				      */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* Include files                                                              */
/*----------------------------------------------------------------------------*/
#include "stdafx.h"

int  aecDTM_connectToActiveProject ( struct CIVdtmprj **p, wchar_t *defaultName, wchar_t *defaultDesc );


/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_projectSurfaceInitialize
 DESC: Creates the base project and one default surface within it.
 HIST: Original - tmi 03-Sep-1990
 MISC: static
 KEYW: DTM PROJECT SURFACE INITIALIZE
-----------------------------------------------------------------------------%*/

int aecDTM_projectSurfaceInitialize
(
  void
)
{
  CIVdtmsrf *srfP = NULL;
  int sts = SUCCESS;

  if( aecDTM_getActiveProject( NULL, NULL, NULL ) != SUCCESS )
  {
    struct CIVdtmprj *prjP = NULL;
    wchar_t name[DTM_C_NAMSIZ], desc[DTM_C_NAMSIZ], material[CIV_C_NAMSIZ];

    wcscpy ( name, DEFAULT_OBJECT_NAME );
    desc[0]     = '\0';
    material[0] = '\0';

    if ( ( sts = aecDTM_connectToActiveProject ( &prjP, name, desc ) ) == SUCCESS )
      if ( ( sts = aecDTM_createSurface ( &srfP, prjP, name, desc, NULL, material, 0., 0. ) ) == SUCCESS )
	  {
        aecDTM_clearSurfaceModifiedFlag ( srfP );
	  }

  }

  return sts;
}
