//---------------------------------------------------------------------------+
// $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+
/*----------------------------------------------------------------------------*/
/* deltin.c                                         tmi    13-Mar-1991        */
/*----------------------------------------------------------------------------*/
/* It sets the delete bit for the input triangle.                             */
/*----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*/
/* Include files                                                              */
/*----------------------------------------------------------------------------*/
#include "stdafx.h"






/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_deleteTriangle
 DESC: It deletes the input triangle
 HIST: Original - tmi 13-Mar-1991
 MISC:
 KEYW: DTM TRIANGLE DELETE
-----------------------------------------------------------------------------%*/

void aecDTM_deleteTriangle
(
  struct CIVdtmsrf *srfP,               /* => surface containing tin          */
  struct CIVdtmtin *tinP,               /* => triangle to delete              */
  int display                           /* => -1: don't dis, else do          */
)
{
  if ( tinP != (struct CIVdtmtin *)0 )
    if ( !aecDTM_isTriangleDeletedFlagSet ( tinP ) )
    {
      aecDTM_setSurfaceModifiedFlag ( srfP );
      aecDTM_setTriangleDeletedFlag ( tinP );
      srfP->tinf->ndel++;

      if ( display != -1 )
      {
        if ( srfP->dis.tinfnc ) (*srfP->dis.tinfnc) ( srfP, tinP, display, srfP->dis.tinsym );
        if ( srfP->dis.confnc ) (*srfP->dis.confnc) ( srfP, tinP, display, srfP->dis.consym );
      }
    }
}




/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_undeleteTriangle
 DESC: It undeletes a previously deleted triangle
 HIST: Original - tmi 13-Mar-1991
 MISC:
 KEYW: DTM TRIANGLE UNDELETE
-----------------------------------------------------------------------------%*/

void aecDTM_undeleteTriangle
(
  struct CIVdtmsrf *srfP,               /* => surface containing tin          */
  struct CIVdtmtin *tinP,               /* => triangle to delete              */
  int display                           /* => -1: don't dis, else do          */
)
{
  if ( tinP != (struct CIVdtmtin *)0 )
    if ( aecDTM_isTriangleDeletedFlagSet ( tinP ) )
    {
      aecDTM_setSurfaceModifiedFlag ( srfP );
      aecDTM_clearTriangleRemovedFlag ( tinP );
      aecDTM_clearTriangleDeletedFlag ( tinP );
      srfP->tinf->ndel--;

      if ( display != -1 )
      {
        if ( srfP->dis.tinfnc ) (*srfP->dis.tinfnc) ( srfP, tinP, display, srfP->dis.tinsym );
        if ( srfP->dis.confnc ) (*srfP->dis.confnc) ( srfP, tinP, display, srfP->dis.consym );
      }
    }
}



