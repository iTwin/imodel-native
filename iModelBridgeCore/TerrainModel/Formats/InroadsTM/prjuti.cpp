//---------------------------------------------------------------------------+
// $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+
/*----------------------------------------------------------------------------*/
/* prjuti.c                                            tmi    25-Jan-1991     */
/*----------------------------------------------------------------------------*/
/* Contains various utilities for surface projects.                           */
/*----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*/
/* Include files                                                              */
/*----------------------------------------------------------------------------*/
#include "stdafx.h"





/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_addSurfaceToProject
 DESC: Adds a surface pointer to a surface project.
 HIST: Original - tmi 25-Jan-1991
 MISC:
 KEYW: DTM PROJECT SURFACE ADD
-----------------------------------------------------------------------------%*/

int aecDTM_addSurfaceToProject /* <= TRUE if error                 */
(
  struct CIVdtmprj *prjP,                 /* => dtm project to use (or NULL)  */
  struct CIVdtmsrf *srfP                  /* => surface to use                */
)
{
  int sts = SUCCESS;

  if ( srfP != (struct CIVdtmsrf *)0 )
  {
    if ( prjP == (struct CIVdtmprj *)0 )
      aecDTM_getActiveProject ( &prjP, (wchar_t *)0, (wchar_t *)0 );

    if ( prjP == (struct CIVdtmprj *)0 )
      sts = DTM_M_NOPRJS;
    else
    {
      srfP->prj = prjP;
      prjP->asrf = prjP->nsrf++;

      if ( prjP->nsrf == 1 )
          prjP->srfs = (CIVdtmsrf* *)calloc ((unsigned int)prjP->nsrf, sizeof(CIVdtmsrf*));
      else
          prjP->srfs = (CIVdtmsrf* *)realloc ((void *)prjP->srfs, (unsigned int)prjP->nsrf * sizeof(CIVdtmsrf*));

      if ( ! prjP->srfs )
        sts = DTM_M_MEMALF;
      else
        prjP->srfs[prjP->asrf] = srfP;
    }
  }

  return ( sts );
}




/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_removeSurfaceFromProject
 DESC: It deletes a surface from a surface project.
 HIST: Original - tmi 25-Jan-1991
       10/94 - wbw - added two more active surfaces
 MISC:
 KEYW: DTM PROJECT SURFACE DELETE
-----------------------------------------------------------------------------%*/

int aecDTM_removeSurfaceFromProject /* <= TRUE if error            */
(
  struct CIVdtmprj *prjP,                /* => DTM project to use (or NULL)   */
  struct CIVdtmsrf *srfP                 /* => surface to remove              */
)
{
  int sts = SUCCESS;

  if ( srfP != (struct CIVdtmsrf *)0 )
  {
    if ( prjP == (struct CIVdtmprj *)0 )
      aecDTM_getActiveProject ( &prjP, (wchar_t *)0, (wchar_t *)0 );

    if ( prjP == (struct CIVdtmprj *)0 )
      sts = DTM_M_NOPRJS;
    else
    {
      int i;

      for ( i = 0; i < prjP->nsrf  &&  prjP->srfs[i] != srfP; i++ );

      if ( i != prjP->nsrf )
      {
      memcpy ((void *)&prjP->srfs[i], (void *)&prjP->srfs[i + 1], (int)(prjP->nsrf - i - 1)*sizeof(CIVdtmsrf*));

        prjP->nsrf--;
        if ( prjP->nsrf == 0 )
          prjP->asrf = prjP->asrf1 = prjP->asrf2 = -1;
        else
        {
          if ( prjP->asrf != 0 )
            if ( i <= prjP->asrf )
              prjP->asrf--;

          if ( prjP->asrf1 != 0 )
            if ( i <= prjP->asrf1 )
              prjP->asrf1 = (short)(i - 1);

          if ( prjP->asrf2 != 0 )
            if ( i <= prjP->asrf2 )
              prjP->asrf2 = (short)(i - 1);
        }

        if ( prjP->nsrf == 0 )
        {
          free ( (void *)prjP->srfs );
          prjP->srfs = 0;
        }
        else if ((prjP->srfs = (CIVdtmsrf**)realloc ((void *)prjP->srfs, (unsigned int)prjP->nsrf * sizeof(CIVdtmsrf*))) == 0L)
          sts = DTM_M_MEMALF;
      }
    }
  }

  return ( sts );
}
