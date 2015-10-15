//---------------------------------------------------------------------------+
// $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+
/*----------------------------------------------------------------------------*/
/* fndsrf.c					   tmi	    10-Apr-1990       */
/*----------------------------------------------------------------------------*/
/* Various utilities to find surfaces.                                        */
/*----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*/
/* Include files                                                              */
/*----------------------------------------------------------------------------*/
#include "stdafx.h"



/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_findSurfaceByName
 DESC: Given a ASCII surface name, this function returns a pointer to the
       corresponding surface.
 HIST: Original - tmi 10-Apr-1990
 MISC:
 KEYW: DTM FIND SURFACE NAME
-----------------------------------------------------------------------------%*/

int aecDTM_findSurfaceByName /* <= TRUE if error                   */
(
  struct CIVdtmsrf **srfPP,             /* <= found surface                   */
  struct CIVdtmprj *prjP,               /* => DTM project (or NULL)           */
  wchar_t *srfNameP                     /* => surface name pointer            */
)
{
  int sts = DTM_M_NOSRFF;

  if ( prjP == (struct CIVdtmprj *)0 )
    aecDTM_getActiveProject ( &prjP, (wchar_t *)0, (wchar_t *)0 );

  if ( prjP == (struct CIVdtmprj *)0 )
    sts = DTM_M_NOPRJS;
  else
  {
    struct CIVdtmsrf *tmpP;
    int i;

    for ( i = 0; i < prjP->nsrf  &&  sts != SUCCESS; i++ )
    {
      tmpP = (struct CIVdtmsrf *) prjP->srfs[i];
      if ( ! wcscmp ( srfNameP, tmpP->nam ) )
      {
        if ( srfPP != (struct CIVdtmsrf **)0 ) *srfPP = (struct CIVdtmsrf *) tmpP;
        sts = SUCCESS;
      }
    }
  }

  return ( sts );
}



/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_findSurfaceByGUID
 DESC: Given a surface BeSQLite::BeGuid, this function returns a pointer to the
       corresponding surface.
 HIST: Original - twl 29-Oct-1998
 MISC:
 KEYW: DTM FIND SURFACE BY BeSQLite::BeGuid
-----------------------------------------------------------------------------%*/

int aecDTM_findSurfaceByGUID /* <= TRUE if error                   */
(
  struct CIVdtmsrf **srfPP,             /* <= found surface                   */
  struct CIVdtmprj *prjP,               /* => DTM project (or NULL)           */
  BeSQLite::BeGuid *srfGUIDp                        /* => surface BeSQLite::BeGuid                    */
)
{
  int sts = DTM_M_NOSRFF;

  if ( prjP == (struct CIVdtmprj *)0 )
    aecDTM_getActiveProject ( &prjP, (wchar_t *)0, (wchar_t *)0 );

  if ( prjP == (struct CIVdtmprj *)0 )
    sts = DTM_M_NOPRJS;
  else
  {
    struct CIVdtmsrf *tmpP;
    int i;

    for ( i = 0; i < prjP->nsrf  &&  sts != SUCCESS; i++ )
    {
      tmpP = (struct CIVdtmsrf *) prjP->srfs[i];
      if ( !aecGuid_compare ( srfGUIDp, &tmpP->guid ) )
      {
        if ( srfPP != (struct CIVdtmsrf **)0 ) *srfPP = (struct CIVdtmsrf *) tmpP;
        sts = SUCCESS;
      }
    }
  }

  return ( sts );
}



