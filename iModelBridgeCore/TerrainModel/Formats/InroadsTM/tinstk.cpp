//---------------------------------------------------------------------------+
// $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+
/*----------------------------------------------------------------------------*/
/* tinstk.c                                        tmi    01-Nov-1990         */
/*----------------------------------------------------------------------------*/
/* Various tin stack utilities.                                               */
/*----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*/
/* Include files                                                              */
/*----------------------------------------------------------------------------*/
#include "stdafx.h"


/*----------------------------------------------------------------------------*/
/* Constants and macros                                                       */
/*----------------------------------------------------------------------------*/
#define     TINSTK_SIZ    100




/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_triangleStack
 DESC: Puts the address of the input triangle onto the input stack.
 HIST: Original - tmi 29-Jan-1994
 MISC:
 KEYW: DTM STACK TRIANGLE
-----------------------------------------------------------------------------%*/

int aecDTM_triangleStack    /* <=  TRUE if error                   */
(
  long *ntinstkP,                      /* <=> # triangles on stack            */
  long **tinstkPP,                     /* <=> triangle stack                  */
  struct CIVdtmtin *tinP               /*  => triangle to add to stack        */
)
{
  int sts = SUCCESS;

  if ( *ntinstkP % TINSTK_SIZ == 0 )
  {
    unsigned int nalc = (unsigned int) ((*ntinstkP / TINSTK_SIZ + 1) * TINSTK_SIZ);
    if ( *ntinstkP == 0 )
      *tinstkPP = (long *) malloc ( nalc * sizeof(long) );
    else
      *tinstkPP = (long *) realloc ( *tinstkPP, nalc * sizeof(long) );
    if ( *tinstkPP == (long *)0 )
      return ( DTM_M_MEMALF );
  }

  (*tinstkPP)[(*ntinstkP)++] = (long) tinP;

  return ( sts );
}




/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_triangleStackPut
 DESC: Allows you to put triangles that have been removed onto a stack
       so that later on, when you are adding triangles, you don't have to
       allocate more memory.  It is mainly used for the dynamic point
       editting commands.  Use aecDTM_triangleStackGet to retrieve a triangle
       from the stack.
 HIST: Original - tmi 02-Nov-1990
 MISC:
 KEYW: DTM STACK TRIANGLE
-----------------------------------------------------------------------------%*/

int aecDTM_triangleStackPut /* <= TRIE if error                    */
(
  struct CIVdtmsrf *srfP,             /* => surface to use                    */
  struct CIVdtmtin *tinP,             /* => triangle to put on stack          */
  int dis                             /* => TRUE: display input triangle      */
)
{
  int sts = SUCCESS;

  if ( srfP != (struct CIVdtmsrf *)0  &&  tinP != (struct CIVdtmtin *)0 )
  {
    long i;

    for ( i = 0; i < srfP->ntinstk; i++ )  /* don't add to stack if already there */
      if ( srfP->tinstk[i] == (long)tinP )
        return ( sts );

    if ( ( sts = aecDTM_triangleStack ( &srfP->ntinstk, &srfP->tinstk, tinP ) ) == SUCCESS )
    {
      aecDTM_setTriangleRemovedFlag ( tinP );
      aecDTM_deleteTriangle ( srfP, tinP, dis );
    }
  }

  return ( sts );
}




/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_triangleStackGet
 DESC: Use this function to retrieve a single triangle from the triangle
       stack of the input surface.
 HIST: Original - tmi 02-Nov-1990
 MISC:
 KEYW: DTM TRIANGLE STACK GET
-----------------------------------------------------------------------------%*/

int aecDTM_triangleStackGet    /* <= TRUE if error                 */
(
  struct CIVdtmtin **tinPP,               /* <= retrieved triangle            */
  struct CIVdtmsrf *srfP                  /* => surface to use                */
)
{
  int sts = SUCCESS;

  if ( tinPP != (struct CIVdtmtin **)0 )
  {
    *tinPP = (struct CIVdtmtin *)0;

    if ( srfP != (struct CIVdtmsrf *)0 )
      if ( srfP->ntinstk == 0 )
        *tinPP = (struct CIVdtmtin *)0;
      else
      {
        srfP->ntinstk--;
        *tinPP = (struct CIVdtmtin *) srfP->tinstk[srfP->ntinstk];

        aecDTM_undeleteTriangle ( srfP, *tinPP, -1 );
        memset ( *tinPP, 0, sizeof(struct CIVdtmtin) );

        if ( srfP->ntinstk == 0 )
        {
          free ( srfP->tinstk );
          srfP->tinstk = (long *)0;
        }
      }
  }

  return ( sts );
}
