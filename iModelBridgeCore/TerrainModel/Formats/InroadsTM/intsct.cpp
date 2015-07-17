//---------------------------------------------------------------------------+
// $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+
/*----------------------------------------------------------------------------*/
/* intsct.c                                            dgc    01-Feb-1994     */
/*----------------------------------------------------------------------------*/
/* General geometric intersection utility.                                    */
/* From "Algorithms in C++", Sedgewick, Robert                                */
/*----------------------------------------------------------------------------*/

#include "stdafx.h"


/*----------------------------------------------------------------------------*/
/* Include files                                                              */
/*----------------------------------------------------------------------------*/
#include <intfnc.h>
#include <intsct.h>
#include <tckfnc.h>



/*----------------------------------------------------------------------------*/
/* Private data structures                                                    */
/*----------------------------------------------------------------------------*/
typedef struct
{
  double y;
  ULong  end:   1;
  ULong  iSeg: 31;
} YStrip;

typedef struct
{
  double xlo;
  double xhi;
  void *eleP;
  struct Node *nodeP;
} IElement;

typedef struct Node
{
  IElement *eleP;
  struct Node *nextP;
  struct Node *prevP;
} Node;

typedef struct
{
  int (*intFuncP)( void *, void *, void * );
  void *mdlDescP;
  void *userDataP;

  YStrip *yP;
  int nY;

  IElement *eleP;
  int nEle;

  int preAlcSize;
} Intersect;



/*----------------------------------------------------------------------------*/
/* Private static function prototypes                                         */
/*----------------------------------------------------------------------------*/
static int  scanCompare( const void *, const void * );
static int  intersectCheck( Intersect *, IElement *, IElement * );
static int  eleNodeInsert( Node **, IElement * );
static void eleNodeDelete( Node **, Node * );



/*----------------------------------------------------------------------------+
 FUNC: aecIntersect_begin
 DESC:
 HIST: original						dgc	10/1994
 MISC:
 KEYW: INTERSECTION ELEMENT BEGIN
+-----------------------------------------------------------------------------*/

int aecIntersect_begin      /* <= TRUE if error                    */
(
  void **thisPP,                       /* <= Intersect object                 */
  int (*intFuncP)(void*,void*,void*),  /* => Intersect function               */
  void *mdlDescP,                      /* => MDL Descriptor                   */
  void *userDataP,                     /* => user data pointer                */
  int preAlcSize                       /* => pre-alloc size ( or 0 )          */
)
{
  Intersect *isP = NULL;
  int sts = SUCCESS;

  *thisPP = NULL;

  isP = ( Intersect * )calloc( 1, sizeof( Intersect ) );
  if( isP == NULL )
    sts = AEC_E_MEMALF;

  if( sts == SUCCESS )
  {
    isP->intFuncP = intFuncP;
    isP->mdlDescP  = mdlDescP;
    isP->userDataP = userDataP;
    isP->preAlcSize = preAlcSize;
  }

  if( sts == SUCCESS && preAlcSize > 0 )
  {
    if( sts == SUCCESS )
    {
      isP->yP = ( YStrip * )calloc( 2 * preAlcSize + 256, sizeof( YStrip ) );
      if( isP->yP == NULL )
        sts = AEC_E_MEMALF;
    }

    if( sts == SUCCESS )
    {
      isP->eleP = ( IElement * )calloc( preAlcSize + 256, sizeof( IElement ) );
      if( isP->eleP == NULL )
        sts = AEC_E_MEMALF;
    }
  }

  if( sts == SUCCESS )
    *thisPP = isP;

  return( sts );
}


/*----------------------------------------------------------------------------+
 FUNC: aecIntersect_insert
 DESC:
 HIST: original						dgc	10/1994
 MISC:
 KEYW: INTERSECTION ELEMENT INSERT
+-----------------------------------------------------------------------------*/

int aecIntersect_insert     /* <= TRUE if error                    */
(
  void *thisP,                         /* => Intersect object                 */
  void *eleP,                          /* => Intersect element                */
  double xlo,                          /* => xlo of eleP                      */
  double ylo,                          /* => ylo of eleP                      */
  double xhi,                          /* => xhi of eleP                      */
  double yhi                           /* => yhi of eleP                      */
)
{
  Intersect *isP = ( Intersect * )thisP;
  int sts = SUCCESS;

  aecTicker_show();

  if( sts == SUCCESS )
  {
    YStrip yVal;

    yVal.y = ylo;
    yVal.iSeg = isP->nEle;
    yVal.end = 0;

    if( ( isP->nY + 1 ) > 2 * isP->preAlcSize )
      sts = aecTable_insert( (void**)&isP->yP, &isP->nY, &yVal, sizeof( yVal ) );
    else
      isP->yP[ isP->nY++ ] = yVal;
  }

  if( sts == SUCCESS )
  {
    YStrip yVal;

    yVal.y = yhi;
    yVal.end = 1;
    yVal.iSeg = isP->nEle;

    if( ( isP->nY + 1 ) > 2 * isP->preAlcSize )
      sts = aecTable_insert( (void**)&isP->yP, &isP->nY, &yVal, sizeof( yVal ) );
    else
      isP->yP[ isP->nY++ ] = yVal;
  }

  if( sts == SUCCESS )
  {
    IElement ele;

    memset( &ele, 0, sizeof( IElement ) );
    ele.eleP = eleP;
    ele.xlo = xlo;
    ele.xhi = xhi;

    if( ( isP->nEle + 1 ) > isP->preAlcSize )
      sts = aecTable_insert( (void**)&isP->eleP, &isP->nEle, &ele, sizeof( ele ) );
    else
      isP->eleP[ isP->nEle++ ] = ele;
  }

  return( sts );
}



/*----------------------------------------------------------------------------+
 FUNC: aecIntersect_end
 DESC:
 HIST: original						dgc	10/1994
 MISC:
 KEYW: INTERSECTION ELEMENT END
+-----------------------------------------------------------------------------*/

int aecIntersect_end        /* <= TRUE if error                    */
(
  void *thisP                          /* => Intersect object                 */
)
{
  Intersect *isP = ( Intersect * )thisP;
  Node *rootP = NULL;
  int i;
  int sts = SUCCESS;

  aecInterrupt_initialize();

  if( isP->nY > 1 )
    qsort( isP->yP, isP->nY, sizeof( YStrip ), scanCompare );

  sts = aecInterrupt_check();

  for( i = 0; sts == SUCCESS && i < isP->nY; i++ )
  {
    IElement *eleP = &isP->eleP[ isP->yP[i].iSeg ];

    aecTicker_show();

    if( !isP->yP[i].end )
      sts = eleNodeInsert( &rootP, eleP );
    else
    {
      Node *nodeP;

      eleNodeDelete( &rootP, eleP->nodeP );

      for( nodeP = rootP; sts == SUCCESS && nodeP; nodeP = nodeP->nextP )
        sts = intersectCheck( isP, eleP, nodeP->eleP );
    }

    if( sts == SUCCESS )
      sts = aecInterrupt_check();
  }

  if ( sts != SUCCESS )
  {
      Node *nextP = NULL;
      Node *nodeP;

      for( nodeP = rootP; nodeP; nodeP = nextP )
      {
        nextP = nodeP->nextP;

        if ( nodeP )
          free ( nodeP );
      }
  }

  if( isP->eleP )
    free( isP->eleP );

  if( isP->yP )
    free( isP->yP );

  free( isP );

  aecInterrupt_stop();

  return( sts );
}



/*----------------------------------------------------------------------------+
 FUNC: scanCompare
 DESC:
 HIST: original						dgc	10/1994
 MISC: static
 KEYW: INTERSECTION ELEMENT SCAN COMPARE
+-----------------------------------------------------------------------------*/

static int scanCompare
(
  const void *a,
  const void *b
)
{
  YStrip *s0 = ( YStrip * )a;
  YStrip *s1 = ( YStrip * )b;

  aecTicker_show();

  if( s0->y < s1->y )
    return( -1 );

  if( s0->y > s1->y )
    return( 1 );

  if( s0->end < s1->end )
    return( -1 );

  if( s0->end > s1->end )
    return( 1 );

  if( s0->iSeg < s1->iSeg )
    return( -1 );

  if( s0->iSeg > s1->iSeg )
    return( 1 );

  return( 0 );
}



/*----------------------------------------------------------------------------+
 FUNC: intersectCheck
 DESC:
 HIST: original						dgc	10/1994
 MISC: static
 KEYW: INTERSECTION ELEMENT CHECK
+-----------------------------------------------------------------------------*/

static int intersectCheck
(
  Intersect *isP,
  IElement *s0,
  IElement *s1
)
{
  int sts = SUCCESS;

  if( s0->xlo > s1->xhi )
    return( sts );

  if( s0->xhi < s1->xlo )
    return( sts );

  sts = (*isP->intFuncP)( s0->eleP, s1->eleP, isP->userDataP );

  return( sts );
}



/*%-----------------------------------------------------------------------------
 FUNC: eleNodeInsert
 DESC: 
 HIST: original						dgc	10/1994
 MISC: static
 KEYW: INTERSECTION ELEMENT NODE INSERT
 ----------------------------------------------------------------------------%*/

static int eleNodeInsert
(
  Node **rootPP,
  IElement *eleP
)
{
  Node *nodeP = NULL;
  int sts = SUCCESS;

  nodeP = ( Node * ) calloc( 1, sizeof( Node ) );

  if( nodeP )
  {
    eleP->nodeP = nodeP;

    nodeP->eleP = eleP;

    nodeP->nextP = *rootPP;

    if( nodeP->nextP )
      nodeP->nextP->prevP = nodeP;

    *rootPP = nodeP;
  }
  else
    sts = AEC_E_MEMALF;

  return( sts );
}



/*%-----------------------------------------------------------------------------
 FUNC: eleNodeDelete
 DESC: 
 HIST: original						dgc	10/1994
 MISC: static
 KEYW: INTERSECTION ELEMENT NODE DELETE
 ----------------------------------------------------------------------------%*/

static void eleNodeDelete
(
  Node **rootPP,
  Node *nodeP
)
{
  if( rootPP && *rootPP && nodeP )
  {
    if( nodeP->nextP )
      nodeP->nextP->prevP = nodeP->prevP;

    if( nodeP->prevP )
      nodeP->prevP->nextP = nodeP->nextP;
    else
      *rootPP = nodeP->nextP;

    free( nodeP );
  }
}


