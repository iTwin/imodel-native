//---------------------------------------------------------------------------+
// $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+
/*----------------------------------------------------------------------------*/
/* rotpnt.c                                            tmi    19-Oct-1990     */
/*----------------------------------------------------------------------------*/
/* Several utilities to collect information about one or more                 */
/* triangles which neigbor a point.                                           */
/*----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*/
/* Include files                                                              */
/*----------------------------------------------------------------------------*/
#include "stdafx.h"


/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_rotateAroundPoint
 DESC: Given a point and a triangle which has as one of its vertices the
       input point, this function returns the neighboring triangle. The
       function moves clockwise around the point if dir is input as 0, else
       the function moves counterclockwise around the point.  Clockwise
       around a point means that when you are
       standing on the point looking towards the input triangle, the
       triangle to the right of the input triangle is returned.  It also
       returns a point to the point on the far side of the returned
       triangle.  Finally, it returns the indeces of the returned triangle
       and point (0:n12, 1:n23, 2:n31;  0:p1, 1:p2, 2:p3).  If the input
       point is not one of the input triangles vertices, then a status of
       DTM_M_NOPNTF is returned. If you don't want a particular piece of
       information returned, then pass in a null pointer for that variable.
 HIST: Original - tmi 21-May-1990
 MISC:
 KEYW: DTM POINT ROTATE AROUND
-----------------------------------------------------------------------------%*/

int aecDTM_rotateAroundPoint /* <= TRUE if error                   */
(
  struct CIVdtmpnt **npntPP,    /* <= neighboring triangle (or NULL)          */
  struct CIVdtmtin **ntinPP,    /* <= neighboring point (or NULL)             */
  int *pindP,                   /* <= neighboring point index (or NULL)       */
  int *tindP,                   /* <= neighboring triangle index (or NULL)    */
  struct CIVdtmpnt *pntP,       /* => point to rotate about                   */
  struct CIVdtmtin *tinP,       /* => current triangle                        */
  int dir                       /* => 0: rotate clockwise,  1: counter-clock. */
)
{
  struct CIVdtmpnt *tnpntP = (struct CIVdtmpnt *)0;
  struct CIVdtmtin *tntinP = (struct CIVdtmtin *)0;
  int sts = SUCCESS, tpind = 0, ttind = 0;

  if ( pindP  ) *pindP  = 0;
  if ( tindP  ) *tindP  = 0;
  if ( ntinPP ) *ntinPP = (struct CIVdtmtin *)0;
  if ( npntPP ) *npntPP = (struct CIVdtmpnt *)0;

  if ( pntP == tinP->p1 )
  {
    ttind  = ( dir == 1 ) ? 0 : 2;
    tntinP = ( dir == 1 ) ? tinP->n12 : tinP->n31;
  }
  else if ( pntP == tinP->p2 )
  {
    ttind  = ( dir == 1 ) ? 1 : 0;
    tntinP = ( dir == 1 ) ? tinP->n23 : tinP->n12;
  }
  else if ( pntP == tinP->p3 )
  {
    ttind  = ( dir == 1 ) ? 2 : 1;
    tntinP = ( dir == 1 ) ? tinP->n31 : tinP->n23;
  }
  else
    return ( DTM_M_NOPNTF );

  if ( tntinP )
    if ( pntP == tntinP->p1 )
    {
      tpind  = ( dir == 1 ) ? 1 : 2;
      tnpntP = ( dir == 1 ) ? tntinP->p2 : tntinP->p3;
    }
    else if ( pntP == tntinP->p2 )
    {
      tpind  = ( dir == 1 ) ? 2 : 0;
      tnpntP = ( dir == 1 ) ? tntinP->p3 : tntinP->p1;
    }
    else
    {
      tpind  = ( dir == 1 ) ? 0 : 1;
      tnpntP = ( dir == 1 ) ? tntinP->p1 : tntinP->p2;
    }

  if ( pindP  ) *pindP  = tpind;
  if ( tindP  ) *tindP  = ttind;
  if ( ntinPP ) *ntinPP = tntinP;
  if ( npntPP ) *npntPP = tnpntP;

  return ( sts );
}
