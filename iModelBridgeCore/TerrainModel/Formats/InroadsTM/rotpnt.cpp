//---------------------------------------------------------------------------+
// $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
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



/*----------------------------------------------------------------------------*/
/* Constants and macros                                                       */
/*----------------------------------------------------------------------------*/
#define     ALCSIZ    10


/*----------------------------------------------------------------------------*/
/* Private static function prototypes                                         */
/*----------------------------------------------------------------------------*/
static int aecDTM_getPointNeighborsAllocate(long *,long **,long **,long **);





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




/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_getPointNeighbors
 DESC: It builds a list of all the points, triangles, and neigboring
       triangles to the input point.  If you don't want a particular
       piece of information, pass in a null pointer for that variable.
 HIST: Original - tmi 19-Oct-1990
 MISC:
 KEYW: DTM POINT NEIGHBORS GET
-----------------------------------------------------------------------------%*/

int aecDTM_getPointNeighbors /* <= TRUE if error                   */
(
  long *npntlstP,                /* <= # point neighbors (or NULL)            */
  long **pntlstPP,               /* <= list of neighboring points (or NULL)   */
  long *ntinlstP,                /* <= # triangle neighbors (or NULL)         */
  long **tinlstPP,               /* <= list of neighboring triangles (or NULL)*/
  long *nnbrlstP,                /* <= # triangle neighbor triangles (or NULL)*/
  long **nbrlstPP,               /* <= list of triangle neighbor triangles (or NULL) */
  struct CIVdtmsrf *srfP,        /* => surface to use                         */
  struct CIVdtmpnt *pntP,        /* => point to rotate about                  */
  struct CIVdtmtin *inptinP,     /* => current triangle                       */
  int dir                        /* => 0: rotate clockwise, 1: counter-clock. */
)
{
  struct CIVdtmtin *tinP, *ntinP = inptinP;
  struct CIVdtmpnt *npntP;
  DPoint3d cor;
  int sts = SUCCESS, pind;
  long num = 0L, nalloc = 0L;

  if ( npntlstP ) *npntlstP = 1L, *pntlstPP = (long *)0;
  if ( ntinlstP ) *ntinlstP = 0L, *tinlstPP = (long *)0;
  if ( nnbrlstP ) *nnbrlstP = 0L, *nbrlstPP = (long *)0;
  DTMPOINTTODPOINT ( srfP, pntP, cor );

  if ( ( sts = aecDTM_findTriangle ( &ntinP, 0, 0, 0, srfP, &cor ) ) == SUCCESS )
    if ( ( sts = aecDTM_rotateAroundPoint ( &npntP, &ntinP, &pind, (int *)0, pntP, ntinP, dir ) ) == SUCCESS )
      if ( ntinP != (struct CIVdtmtin *)0 )
      {
        BOOL bFinished = FALSE;
        int loopCnt = 0;
        tinP = ntinP;

        do
        {
          if ( num >= nalloc*ALCSIZ )
            if ( ( sts = aecDTM_getPointNeighborsAllocate ( &nalloc, pntlstPP, tinlstPP, nbrlstPP ) ) != SUCCESS )
              return ( sts );

          num++;
          if ( npntlstP ) (*pntlstPP)[(*npntlstP)++] = (long) npntP;
          if ( ntinlstP ) (*tinlstPP)[(*ntinlstP)++] = (long) ntinP;
          if ( nnbrlstP ) (*nbrlstPP)[(*nnbrlstP)++] = (long) ((pind == 0) ? ntinP->n31 : *(&ntinP->n12 + pind - 1));

          sts = aecDTM_rotateAroundPoint ( &npntP, &ntinP, &pind, (int *)0, pntP, ntinP, dir );

          if ( loopCnt > 1000 && sts == SUCCESS && ntinP )
          {
            for ( int i = 0; i < *ntinlstP && !bFinished; i++ )
            {
                if ( (*tinlstPP)[i] == (long)ntinP )
                    bFinished = TRUE;
            }
          }

          loopCnt++;

        } while ( ntinP != tinP  &&  ntinP != (struct CIVdtmtin *)0  && !bFinished && sts == SUCCESS );
      }

  if ( pntlstPP  &&  *pntlstPP != (long *)0 )
    (*pntlstPP)[0] = (*pntlstPP)[(*npntlstP)-1];
  else if ( npntlstP != (long *)0 )
    *npntlstP = 0L;

  return ( sts );
}





/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_getPointNeighborsAllocate
 DESC: It allocates memory for the data structures returned by this
       list generation function.
 HIST: Original - tmi 19-Oct-1990
 MISC: static
 KEYW: DTM POINT NEIGHBORS GET ALLOCATE MEMORY
-----------------------------------------------------------------------------%*/

static int aecDTM_getPointNeighborsAllocate
(
  long *nalloc,
  long **pntlst,
  long **tinlst,
  long **nbrlst
)
{
  int sts = SUCCESS;

  (*nalloc)++;

  if ( pntlst )
  {
    if ( *nalloc == 1 )
      *pntlst = (long *) calloc ( (unsigned int)(*nalloc*ALCSIZ+1), sizeof(long) );
    else
      *pntlst = (long *) realloc ( *pntlst, (unsigned int)((*nalloc*ALCSIZ+1)*sizeof(long)) );
    if ( *pntlst == (long *)0 )
      sts = DTM_M_MEMALF;
  }

  if ( sts == SUCCESS  &&  tinlst )
  {
    if ( *nalloc == 1 )
      *tinlst = (long *) calloc ( (unsigned int)(*nalloc*ALCSIZ), sizeof(long) );
    else
      *tinlst = (long *) realloc ( *tinlst, (unsigned int)((*nalloc*ALCSIZ)*sizeof(long)) );
    if ( *tinlst == (long *)0 )
      sts = DTM_M_MEMALF;
  }

  if ( sts == SUCCESS  &&  nbrlst )
  {
    if ( *nalloc == 1 )
      *nbrlst = (long *) calloc ( (unsigned int)(*nalloc*ALCSIZ), sizeof(long) );
    else
      *nbrlst = (long *) realloc ( *nbrlst, (unsigned int)((*nalloc*ALCSIZ)*sizeof(long)) );
    if ( *nbrlst == (long *)0 )
      sts = DTM_M_MEMALF;
  }

  return ( sts );
}
