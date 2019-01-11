//---------------------------------------------------------------------------+
// $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+
/*----------------------------------------------------------------------------*/
/* tinuti.c                                        tmi    10-Apr-1990         */
/*----------------------------------------------------------------------------*/
/* Contains utilities for triangles.                                          */
/*----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*/
/* Include files                                                              */
/*----------------------------------------------------------------------------*/
#include "stdafx.h"





/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_swapNeighboringTriangles
 DESC: Swaps two neighboring triangles. Input should be two neighboring
       triangles. All appropriate information is updated.
 HIST: Original - tmi 10-Apr-1990
 MISC:
 KEYW: DTM TRIANGLES NEIGHBORS SWAP
-----------------------------------------------------------------------------%*/

int aecDTM_swapNeighboringTriangles /* <= TRUE if error            */
(
  struct CIVdtmsrf *srfP,            /* => surface to use                     */
  struct CIVdtmtin *aP,              /* => first triangle                     */
  struct CIVdtmtin *bP               /* => first triangle's neighbor          */
)
{
  struct CIVdtmpnt *p1P, *p2P, *p3P, *p4P;
  struct CIVdtmtin *n1P, *n2P, *n3P, *n4P;
  int sts = SUCCESS, b1, b2, b3, b4;
  long sida, sidb, i1, i2, i3, i4;

  if ( ( sts = aecDTM_getTriangleSideIndex ( &sida, &sidb, aP, bP, TRUE ) ) == SUCCESS )
  {
    p1P = sida == 3 ? aP->p1  : * ( &aP->p1  + sida     );
    p2P = sida == 1 ? aP->p3  : * ( &aP->p1  + sida - 2 );
    p3P = sidb == 3 ? bP->p1  : * ( &bP->p1  + sidb     );
    p4P = sidb == 1 ? bP->p3  : * ( &bP->p1  + sidb - 2 );

    n1P = sida == 3 ? aP->n12 : * ( &aP->n12 + sida     );
    n2P = sida == 1 ? aP->n31 : * ( &aP->n12 + sida - 2 );
    n3P = sidb == 3 ? bP->n12 : * ( &bP->n12 + sidb     );
    n4P = sidb == 1 ? bP->n31 : * ( &bP->n12 + sidb - 2 );

    b1 = sida == 1 ? aP->flg & DTM_C_TINB23 : ( sida == 2 ? aP->flg & DTM_C_TINB31 : aP->flg & DTM_C_TINB12 );
    b2 = sida == 1 ? aP->flg & DTM_C_TINB31 : ( sida == 2 ? aP->flg & DTM_C_TINB12 : aP->flg & DTM_C_TINB23 );
    b3 = sidb == 1 ? bP->flg & DTM_C_TINB23 : ( sidb == 2 ? bP->flg & DTM_C_TINB31 : bP->flg & DTM_C_TINB12 );
    b4 = sidb == 1 ? bP->flg & DTM_C_TINB31 : ( sidb == 2 ? bP->flg & DTM_C_TINB12 : bP->flg & DTM_C_TINB23 );
    aP->flg &= ~(DTM_C_TINB12 | DTM_C_TINB23 | DTM_C_TINB31);
    bP->flg &= ~(DTM_C_TINB12 | DTM_C_TINB23 | DTM_C_TINB31);

    if ( ( sts = aecDTM_getTriangleSideIndex ( &sida, &i1, aP, n1P, TRUE ) ) == SUCCESS )
      if ( ( sts = aecDTM_getTriangleSideIndex ( &sida, &i2, aP, n2P, TRUE ) ) == SUCCESS )
	if ( ( sts = aecDTM_getTriangleSideIndex ( &sidb, &i3, bP, n3P, TRUE ) ) == SUCCESS )
	  if ( ( sts = aecDTM_getTriangleSideIndex ( &sidb, &i4, bP, n4P, TRUE ) ) == SUCCESS )
	  {
            if ( srfP->dis.tinfnc ) (*srfP->dis.tinfnc)(srfP,aP,0,srfP->dis.tinsym), (*srfP->dis.tinfnc)(srfP,bP,0,srfP->dis.tinsym);
            if ( srfP->dis.confnc ) (*srfP->dis.confnc)(srfP,aP,0,srfP->dis.consym), (*srfP->dis.confnc)(srfP,bP,0,srfP->dis.consym);
	    aP->p1 = p1P;
	    aP->p2 = bP->p3 = p2P;
	    aP->p3 = bP->p2 = p4P;
	    bP->p1 = p3P;

	    aP->n12 = n1P;
	    aP->n23 = bP;
	    aP->n31 = n4P;

	    bP->n12 = n3P;
	    bP->n23 = aP;
	    bP->n31 = n2P;

	    if ( i1 ) * ( &n1P->n12 + i1 - 1 ) = aP;
	    if ( i2 ) * ( &n2P->n12 + i2 - 1 ) = bP;
	    if ( i3 ) * ( &n3P->n12 + i3 - 1 ) = bP;
	    if ( i4 ) * ( &n4P->n12 + i4 - 1 ) = aP;

            if ( b1 ) aP->flg |= DTM_C_TINB12;
            if ( b2 ) bP->flg |= DTM_C_TINB31;
            if ( b3 ) bP->flg |= DTM_C_TINB12;
            if ( b4 ) aP->flg |= DTM_C_TINB31;

            if ( srfP->dis.tinfnc ) (*srfP->dis.tinfnc)(srfP,aP,1,srfP->dis.tinsym), (*srfP->dis.tinfnc)(srfP,bP,1,srfP->dis.tinsym);
            if ( srfP->dis.confnc ) (*srfP->dis.confnc)(srfP,aP,1,srfP->dis.consym), (*srfP->dis.confnc)(srfP,bP,1,srfP->dis.consym);
	  }
  }

  return ( sts );
}




/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_checkTrianglesForSwap
 DESC: Checks to see if two neighboring triangles can be swapped.  That is,
       it returns SUCCESS if the side between the two triangles is
       not a breakline, and DTM_M_NOSWAP if it is.
 HIST: Original - tmi 22-Oct-1990
 MISC:
 KEYW: DTM TRIANGLES SWAP CHECK
-----------------------------------------------------------------------------%*/

int aecDTM_checkTrianglesForSwap    /* <= TRUE if NOT swappable    */
(
  struct CIVdtmtin *aP,                /* => first triangle                   */
  struct CIVdtmtin *bP                 /* => first triangle's neighbor        */
)
{
  int sts = DTM_M_NOSWAP;

  if ( aP != (struct CIVdtmtin *)0  &&  bP != (struct CIVdtmtin *)0 )
  {
    long sida, sidb;

    if ( ( sts = aecDTM_getTriangleSideIndex ( &sida, &sidb, aP, bP, TRUE ) ) == SUCCESS )
      if ( aP )
      {
        if ( sida == 1  &&  aP->flg & DTM_C_TINB12  ||
             sida == 2  &&  aP->flg & DTM_C_TINB23  ||
             sida == 3  &&  aP->flg & DTM_C_TINB31 ) sts = DTM_M_NOSWAP;
      }
      else if ( bP )
        if ( sidb == 1  &&  bP->flg & DTM_C_TINB12  ||
             sidb == 2  &&  bP->flg & DTM_C_TINB23  ||
             sidb == 3  &&  bP->flg & DTM_C_TINB31 ) sts = DTM_M_NOSWAP;
  }

  return ( sts );
}




/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_getTriangleSideIndex
 DESC: Returns the triangle side of input triangle 'a' for which triangle
       'b' is the neighbor. If 'b' is not a neighbor of 'a' then the
       function returns DTM_M_NOSIDE.
 HIST: Original - tmi 10-Apr-1990
 MISC:
 KEYW: DTM TRIANGLES SIDE INDEX GET
-----------------------------------------------------------------------------%*/

int aecDTM_getTriangleSideIndex /* <= TRUE if error                */
(
  long *sidaP,                    /* <= triangle A side index                 */
  long *sidbP,                    /* <= triangle B side index                 */
  struct CIVdtmtin *aP,           /* => first triangle                        */
  struct CIVdtmtin *bP,           /* => first triangle's neighbor             */
  BOOL bSetErrorPoint             /* => set error point if no side is found   */
)
{
  int sts = SUCCESS;

  if ( sidaP != (long *)0 )
      if (!aP || aP == (CIVdtmtin *)-1) *sidaP = 0;
    else if ( aP->n12 == bP ) *sidaP = 1;
    else if ( aP->n23 == bP ) *sidaP = 2;
    else if ( aP->n31 == bP ) *sidaP = 3;
    else                      *sidaP = -1,  sts = DTM_M_NOSIDE;

  if ( sidbP != (long *)0 )
      if (!bP || bP == (CIVdtmtin *)-1) *sidbP = 0;
    else if ( bP->n12 == aP ) *sidbP = 1;
    else if ( bP->n23 == aP ) *sidbP = 2;
    else if ( bP->n31 == aP ) *sidbP = 3;
    else                      *sidbP = -1,  sts = DTM_M_NOSIDE;

  if ( sts == DTM_M_NOSIDE && bSetErrorPoint == TRUE )
  {
    if ( aP )
        aecDTM_triangulateSetErrorPoint ( aP->p1 );
    else if ( bP )
        aecDTM_triangulateSetErrorPoint ( bP->p1 );
  }

  return ( sts );
}





/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_updateTriangleNeighbor
 DESC: Given pointers to a triangle and its neigboring triangle, this
       function finds the side of the neighboring triangle that has
       triangle as its neighbor, and then assigns the new triangle
       pointer as that neighbor. Note that this function does not check
       to make sure that the input triangle and neighboring triangle
       in fact neighbors - so they better be.
 HIST: Original - tmi 10-Apr-1990
 MISC:
 KEYW: DTM TRIANGLE NEIGHBOR UPDATE
-----------------------------------------------------------------------------%*/

void aecDTM_updateTriangleNeighbor
(
  struct CIVdtmtin *tinP,                /* => first triangle                 */
  struct CIVdtmtin *neiP,                /* => neighboring triangle           */
  struct CIVdtmtin *newtinP              /* => triangle to replace first tin  */
)
{
  if ( neiP )
    if ( neiP->n12 == tinP )
      neiP->n12 = newtinP;
    else if ( neiP->n23 == tinP )
      neiP->n23 = newtinP;
    else
      neiP->n31 = newtinP;
}



/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_isTriangleRangeTriangle
 DESC: Given a surface and a triangle this function returns true if one of
       the triangle's vertices is a range point and false if it isn't.
 HIST: Original - tmi 10-Apr-1990
 MISC:
 KEYW: DTM TRIANGLE RANGE
-----------------------------------------------------------------------------%*/

int aecDTM_isTriangleRangeTriangle /* <= TRUE if range triangle    */
(
  struct CIVdtmsrf *srfP,                /* => surface to use                 */
  struct CIVdtmtin *tinP                 /* => triangle to use                */
)
{
  int sts = FALSE;

  if ( srfP != (struct CIVdtmsrf *)0  &&  tinP != (struct CIVdtmtin *)0 )
  {
    struct CIVdtmpnt *firstRangeP, *lastRangeP;

    firstRangeP = srfP->rngf->blk->rec.pnt;
    lastRangeP  = srfP->rngf->blk->rec.pnt + srfP->rngf->blk->use - 1;

    sts = ( ( tinP->p1 >= firstRangeP  &&  tinP->p1 <= lastRangeP ) ||
            ( tinP->p2 >= firstRangeP  &&  tinP->p2 <= lastRangeP ) ||
	    ( tinP->p3 >= firstRangeP  &&  tinP->p3 <= lastRangeP ) );
  }

  return ( sts );
}


