//---------------------------------------------------------------------------+
// $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+
/*----------------------------------------------------------------------*/
/* srfflg.c					   tmi	  21-May-1990	*/
/*----------------------------------------------------------------------*/
/* Utilities to clear the different options in the DTM data structures. */
/*----------------------------------------------------------------------*/


/*----------------------------------------------------------------------*/
/* Include files                                                        */
/*----------------------------------------------------------------------*/
#include "stdafx.h"
#include "TimeStampUtils.h"


/*----------------------------------------------------------------------*/
/* Private static function prototypes                                   */
/*----------------------------------------------------------------------*/
static int aecDTM_clearAllTrianglesProcessedFlagProcess(void *,long,DPoint3d *,struct CIVdtmtin *,unsigned long);
static int aecDTM_clearAllPointsIgnoreSegmentFlagProcess(void *,int,long,DPoint3d *,struct CIVdtmpnt *);


/*%-----------------------------------------------------------------------
 FUNC: aecDTM_isPointProcessedFlagSet
 DESC: Returns TRUE if point's processing flag is set.
 HIST: Original - tmi 18-Jan-1994
 MISC:
 KEYW: DTM POINT FLAG PROCESSED
-----------------------------------------------------------------------%*/

int aecDTM_isPointProcessedFlagSet /* <= TRUE if set         */
(
  struct CIVdtmpnt *pntP               /* => point to check             */
)
{
  int sts = FALSE;
  if ( pntP != (struct CIVdtmpnt *)0  &&  pntP->flg & DTM_C_PNTPRC ) sts = TRUE;
  return ( sts );
}



/*%-----------------------------------------------------------------------
 FUNC: aecDTM_setPointDeletedFlag
 DESC: It sets the point Deleted flag for a single point.
 HIST: Original - tmi 18-Jan-1994
 MISC:
 KEYW: DTM POINT FLAG DELETED SET
-----------------------------------------------------------------------%*/

void aecDTM_setPointDeletedFlag
(
  struct CIVdtmpnt *pntP                /* => point to use              */
)
{
  if ( pntP != (struct CIVdtmpnt *)0 ) pntP->flg |= DTM_C_PNTDEL;
}



/*%-----------------------------------------------------------------------
 FUNC: aecDTM_isPointDeletedFlagSet
 DESC: Returns TRUE if point's processing flag is set.
 HIST: Original - tmi 18-Jan-1994
 MISC:
 KEYW: DTM POINT FLAG DELETED
-----------------------------------------------------------------------%*/

int aecDTM_isPointDeletedFlagSet /* <= TRUE if set         */
(
  struct CIVdtmpnt *pntP               /* => point to check             */
)
{
  int sts = FALSE;
  if ( pntP != (struct CIVdtmpnt *)0  &&  pntP->flg & DTM_C_PNTDEL ) sts = TRUE;
  return ( sts );
}



/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_setPointConstructionFlag
 DESC: It sets the point Construction flag for a single point.
 HIST: Original - tmi 22-Jun-1994
 MISC:
 KEYW: DTM POINT FLAG CONSTRUCTION SET
-----------------------------------------------------------------------------%*/

void aecDTM_setPointConstructionFlag
(
  struct CIVdtmpnt *pntP               /* => point to use                     */
)
{
  if ( pntP != (struct CIVdtmpnt *)0 ) pntP->flg |= DTM_C_PNTCON;
}




/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_isPointConstructionFlagSet
 DESC: Returns TRUE if point's construction flag is set.
 HIST: Original - tmi 22-Jun-1994
 MISC:
 KEYW: DTM POINT FLAG CONSTRUCTION
-----------------------------------------------------------------------------%*/

int aecDTM_isPointConstructionFlagSet /* <= TRUE if set             */
(
  struct CIVdtmpnt *pntP               /* => point to check                    */
)
{
  int sts = FALSE;
  if ( pntP != (struct CIVdtmpnt *)0  &&  pntP->flg & DTM_C_PNTCON ) sts = TRUE;
  return ( sts );
}





/*%-----------------------------------------------------------------------
 FUNC: aecDTM_clearAllTrianglesProcessedFlag
 DESC: It clears the bits in all the triangle records for the input surface.
 HIST: Original - tmi 21-May-1990
 MISC:
 KEYW: DTM TRIANGLE FLAG PROCESSED CLEAR
-----------------------------------------------------------------------%*/

void aecDTM_clearAllTrianglesProcessedFlag
(
  struct CIVdtmsrf *srfP                 /* => surface to use           */
)
{
  if ( srfP != (struct CIVdtmsrf *)0 )
    aecDTM_sendAllTriangles ( (void *)0, srfP, DTM_C_NOBREK|DTM_C_DELETE, aecDTM_clearAllTrianglesProcessedFlagProcess, (void *)0 );
}




/*%-----------------------------------------------------------------------
 FUNC: aecDTM_clearAllTrianglesProcessedFlag
 DESC: It clears the bits in each triangle.
 HIST: Original - tmi 21-May-1990
 MISC: static
 KEYW: DTM TRIANGLE FLAG PROCESSED
-----------------------------------------------------------------------%*/

static int aecDTM_clearAllTrianglesProcessedFlagProcess
(
  void *,
  long,
  DPoint3d *,
  struct CIVdtmtin *tin,
  unsigned long
)
{
  aecDTM_clearTriangleProcessedFlag ( tin );
  return ( SUCCESS );
}




/*%-----------------------------------------------------------------------
 FUNC: aecDTM_clearTriangleProcessedFlag
 DESC: It clears the processed flag for a single triangle.
 HIST: Original - tmi 18-Jan-1994
 MISC:
 KEYW: DTM TRIANGLE FLAG PROCESSED CLEAR
-----------------------------------------------------------------------%*/

void aecDTM_clearTriangleProcessedFlag
(
  struct CIVdtmtin *tinP                /* => triangle to use           */
)
{
  if ( tinP != (struct CIVdtmtin *)0 ) tinP->flg &= ~DTM_C_TINPRC;
}




/*%-----------------------------------------------------------------------
 FUNC: aecDTM_setTriangleProcessedFlag
 DESC: It sets the processed flag for a single triangle.
 HIST: Original - tmi 18-Jan-1994
 MISC:
 KEYW: DTM TRIANGLE FLAG PROCESSED SET
-----------------------------------------------------------------------%*/

void aecDTM_setTriangleProcessedFlag
(
  struct CIVdtmtin *tinP                /* => triangle to use           */
)
{
  if ( tinP != (struct CIVdtmtin *)0 ) tinP->flg |= DTM_C_TINPRC;
}




/*%-----------------------------------------------------------------------
 FUNC: aecDTM_isTriangleProcessedFlagSet
 DESC: Returns TRUE if input triangle's processing bit is set.
 HIST: Original - tmi 18-Jan-1994
 MISC:
 KEYW: DTM TRIANGLE FLAG PROCESSED
-----------------------------------------------------------------------%*/

int aecDTM_isTriangleProcessedFlagSet /* <= TRUE if set      */
(
  struct CIVdtmtin  *tinP              /* => triangle to check          */
)
{
  int sts = FALSE;
  if ( tinP != (struct CIVdtmtin *)0  &&  tinP->flg & DTM_C_TINPRC ) sts = TRUE;
  return ( sts );
}


/*%-----------------------------------------------------------------------
 FUNC: aecDTM_clearTriangleDeletedFlag
 DESC: It clears the Deleted flag for a single triangle.
 HIST: Original - tmi 18-Jan-1994
 MISC:
 KEYW: DTM TRIANGLE FLAG DELETED CLEAR
-----------------------------------------------------------------------%*/

void aecDTM_clearTriangleDeletedFlag
(
  struct CIVdtmtin *tinP                /* => triangle to use           */
)
{
  if ( tinP != (struct CIVdtmtin *)0 ) tinP->flg &= ~DTM_C_TINDEL;
}




/*%-----------------------------------------------------------------------
 FUNC: aecDTM_setTriangleDeletedFlag
 DESC: It sets the Deleted flag for a single triangle.
 HIST: Original - tmi 18-Jan-1994
 MISC:
 KEYW: DTM TRIANGLE FLAG DELETED SET
-----------------------------------------------------------------------%*/

void aecDTM_setTriangleDeletedFlag
(
  struct CIVdtmtin *tinP                /* => triangle to use           */
)
{
  if ( tinP != (struct CIVdtmtin *)0 ) tinP->flg |= DTM_C_TINDEL;
}




/*%-----------------------------------------------------------------------
 FUNC: aecDTM_isTriangleDeletedFlagSet
 DESC: Returns TRUE if input triangle's processing bit is set.
 HIST: Original - tmi 18-Jan-1994
 MISC:
 KEYW: DTM TRIANGLE FLAG DELETED
-----------------------------------------------------------------------%*/

int aecDTM_isTriangleDeletedFlagSet /* <= TRUE if set      */
(
  struct CIVdtmtin  *tinP              /* => triangle to check          */
)
{
  int sts = FALSE;
  if ( tinP != (struct CIVdtmtin *)0  &&  tinP->flg & DTM_C_TINDEL ) sts = TRUE;
  return ( sts );
}


/*%-----------------------------------------------------------------------
 FUNC: aecDTM_clearTriangleRemovedFlag
 DESC: It clears the Removed flag for a single triangle.
 HIST: Original - tmi 18-Jan-1994
 MISC:
 KEYW: DTM TRIANGLE FLAG REMOVED CLEAR
-----------------------------------------------------------------------%*/

void aecDTM_clearTriangleRemovedFlag
(
  struct CIVdtmtin *tinP                /* => triangle to use           */
)
{
  if ( tinP != (struct CIVdtmtin *)0 ) tinP->flg &= ~DTM_C_TINREM;
}




/*%-----------------------------------------------------------------------
 FUNC: aecDTM_setTriangleRemovedFlag
 DESC: It sets the Removed flag for a single triangle.
 HIST: Original - tmi 18-Jan-1994
 MISC:
 KEYW: DTM TRIANGLE FLAG REMOVED SET
-----------------------------------------------------------------------%*/

void aecDTM_setTriangleRemovedFlag
(
  struct CIVdtmtin *tinP                /* => triangle to use           */
)
{
  if ( tinP != (struct CIVdtmtin *)0 ) tinP->flg |= DTM_C_TINREM;
}




/*%-----------------------------------------------------------------------
 FUNC: aecDTM_isTriangleRemovedFlagSet
 DESC: Returns TRUE if input triangle's processing bit is set.
 HIST: Original - tmi 18-Jan-1994
 MISC:
 KEYW: DTM TRIANGLE FLAG REMOVED
-----------------------------------------------------------------------%*/

int aecDTM_isTriangleRemovedFlagSet /* <= TRUE if set      */
(
  struct CIVdtmtin  *tinP              /* => triangle to check          */
)
{
  int sts = FALSE;
  if ( tinP != (struct CIVdtmtin *)0  &&  tinP->flg & DTM_C_TINREM ) sts = TRUE;
  return ( sts );
}





/*%-----------------------------------------------------------------------
 FUNC: aecDTM_clearSurfaceModifiedFlag
 DESC: It clears the modified bit in the surface header.
 HIST: Original - tmi 16-Aug-1993
 MISC:
 KEYW: DTM SURFACE FLAG MODIFIED CLEAR
-----------------------------------------------------------------------%*/

void aecDTM_clearSurfaceModifiedFlag
(
  struct CIVdtmsrf *srfP                  /* => surface to clear bit in */
)
{
  if ( srfP != (struct CIVdtmsrf *)0 ) srfP->flg &= ~DTM_C_SRFMOD;
}




/*%-----------------------------------------------------------------------
 FUNC: aecDTM_setSurfaceModifiedFlag
 DESC: It sets the modified bit in the surface header.
 HIST: Original - tmi 18-Jan-1994
 MISC:
 KEYW: DTM SURFACE FLAG MODIFIED SET
-----------------------------------------------------------------------%*/

void aecDTM_setSurfaceModifiedFlag
(
  struct CIVdtmsrf *srfP                  /* => surface to clear bit in */
)
{
  if ( srfP != (struct CIVdtmsrf *)0 )
  {
    srfP->flg |= DTM_C_SRFMOD;
    aecTimeStamp_getUserName ( srfP->revby );
    aecTimeStamp_getStamp ( &srfP->revdate );
    if ( srfP->ptrIndexTableP != (void *)0 ) ((struct CIVptrindTwo *)srfP->ptrIndexTableP)->buildTable = TRUE;
  }
}




/*%-----------------------------------------------------------------------
 FUNC: aecDTM_isSurfaceModifiedFlagSet
 DESC: Returns TRUE if input surface's modified flag is set
 HIST: Original - tmi 18-Jan-1994
 MISC:
 KEYW: DTM SURFACE FLAG MODIFIED
-----------------------------------------------------------------------%*/

int aecDTM_isSurfaceModifiedFlagSet /* <= TRUE if set       */
(
  struct CIVdtmsrf *srfP                 /* => surface to check         */
)
{
  int sts = FALSE;
  if ( srfP != (struct CIVdtmsrf *)0  &&  srfP->flg & DTM_C_SRFMOD ) sts = TRUE;
  return ( sts );
}




/*%-----------------------------------------------------------------------
 FUNC: aecDTM_clearSurfaceTrianglesNotUpToDateFlag
 DESC: It clears the triangles not up to date bit in the surface header.
 HIST: Original - twl 12-Mar-2002
 MISC:
 KEYW: DTM SURFACE FLAG TRIANGLES NOT UP TO DATE CLEAR
-----------------------------------------------------------------------%*/

void aecDTM_clearSurfaceTinOutOfDateFlag
(
  struct CIVdtmsrf *srfP                  /* => surface to clear bit in */
)
{
  if ( srfP != (struct CIVdtmsrf *)0 ) srfP->par.tinOutOfDate = 0;
}




/*%-----------------------------------------------------------------------
 FUNC: aecDTM_setSurfaceTrianglesNotUpToDateFlag
 DESC: It sets the triangles not up to date bit in the surface header.
 HIST: Original - twl 12-Mar-2002
 MISC:
 KEYW: DTM SURFACE FLAG TRIANGLES NOT UP TO DATE SET
-----------------------------------------------------------------------%*/

void aecDTM_setSurfaceTinOutOfDateFlag
(
  struct CIVdtmsrf *srfP                  /* => surface to set bit in */
)
{
  if ( srfP != (struct CIVdtmsrf *)0 )
  {
    srfP->par.tinOutOfDate = 1;
  }
}




/*%-----------------------------------------------------------------------
 FUNC: aecDTM_setFeatureDeletedFlag
 DESC: It sets the feature Deleted flag for a single feature.
 HIST: Original - twl 12-Nov-1998
 MISC:
 KEYW: DTM FEATURE FLAG DELETED SET
-----------------------------------------------------------------------%*/

void aecDTM_setFeatureDeletedFlag
(
  struct CIVdtmftr *ftrP                /* => feature to use              */
)
{
  if ( ftrP != (struct CIVdtmftr *)0 ) ftrP->flg |= DTM_C_FTRDEL;
}



/*%-----------------------------------------------------------------------
 FUNC: aecDTM_isFeatureDeletedFlagSet
 DESC: Returns TRUE if input feature's deleted bit is set.
 HIST: Original - twl 28-Oct-1998
 MISC:
 KEYW: DTM FEATURE FLAG DELETED
-----------------------------------------------------------------------%*/

int aecDTM_isFeatureDeletedFlagSet /* <= TRUE if set      */
(
  struct CIVdtmftr  *ftrP              /* => feature to check          */
)
{
  int sts = FALSE;
  if ( ftrP != (struct CIVdtmftr *)0  &&  ftrP->flg & DTM_C_FTRDEL ) sts = TRUE;
  return ( sts );
}



/*%-----------------------------------------------------------------------
 FUNC: aecDTM_setStyleDeletedFlag
 DESC: It sets the style Deleted flag for a single style.
 HIST: Original - twl 12-Nov-1998
 MISC:
 KEYW: DTM STYLE FLAG DELETED SET
-----------------------------------------------------------------------%*/

void aecDTM_setStyleDeletedFlag
(
  struct CIVdtmsty *styP                /* => feature to use              */
)
{
  if ( styP != (struct CIVdtmsty *)0 ) styP->flg |= DTM_C_STYDEL;
}


/*%-----------------------------------------------------------------------
 FUNC: aecDTM_isStyleDeletedFlagSet
 DESC: Returns TRUE if input style's deleted bit is set.
 HIST: Original - twl 12-Nov-1998
 MISC:
 KEYW: DTM STYLE FLAG DELETED
-----------------------------------------------------------------------%*/

int aecDTM_isStyleDeletedFlagSet /* <= TRUE if set      */
(
  struct CIVdtmsty  *styP              /* => feature to check          */
)
{
  int sts = FALSE;
  if ( styP != (struct CIVdtmsty *)0  &&  styP->flg & DTM_C_STYDEL ) sts = TRUE;
  return ( sts );
}



/*%-----------------------------------------------------------------------
 FUNC: aecDTM_isPayItemDeletedFlagSet
 DESC: Returns TRUE if input pay items's deleted bit is set.
 HIST: Original - twl 10-Oct-2003
 MISC:
 KEYW: DTM PAY ITEM FLAG DELETED
-----------------------------------------------------------------------%*/

int aecDTM_isPayItemDeletedFlagSet /* <= TRUE if set        */
(
  struct CIVdtmpay  *payP              /* => pay item to check         */
)
{
  int sts = FALSE;
  if ( payP != (struct CIVdtmpay *)0  &&  payP->flg & DTM_C_PAYDEL ) sts = TRUE;
  return ( sts );
}


/*%-----------------------------------------------------------------------
 FUNC: aecDTM_clearAllPointsIgnoreSegmentFlag
 DESC: It clears the bits in all the point records for the input surface.
 HIST: Original - twl 27-Oct-2003
 MISC:
 KEYW: DTM POINT FLAG IGNORE SEGMENT CLEAR
-----------------------------------------------------------------------%*/

void aecDTM_clearAllPointsIgnoreSegmentFlag
(
  struct CIVdtmsrf *srfP                 /* => surface to use           */
)
{
  if ( srfP != (struct CIVdtmsrf *)0 )
    aecDTM_sendAllPoints ( (void *)0, srfP, DTM_C_NOBREK|DTM_C_DELETE, 0, aecDTM_clearAllPointsIgnoreSegmentFlagProcess, (void *)0 );
}



/*%-----------------------------------------------------------------------
 FUNC: aecDTM_clearAllPointsIgnoreSegmentFlagProcess
 DESC: It clears the bits in each point.
 HIST: Original - twl 27-Oct-2003
 MISC: static
 KEYW: DTM POINT FLAG IGNORE SEGMENT
-----------------------------------------------------------------------%*/

static int aecDTM_clearAllPointsIgnoreSegmentFlagProcess
(
  void *jnk,
  int typ,
  long np,
  DPoint3d *p,
  struct CIVdtmpnt *pnt
)
{
  int i;

  for (i=0;i<np;i++)
      aecDTM_clearPointIgnoreSegmentFlag ( &pnt[i] );

  return ( SUCCESS );
}



/*%-----------------------------------------------------------------------
 FUNC: aecDTM_clearPointIgnoreSegmentFlag
 DESC: It clears the point ignore segment flag for a single point.
 HIST: Original - twl 27-Oct-2003
 MISC:
 KEYW: DTM POINT FLAG IGNORE SEGMENT CLEAR
-----------------------------------------------------------------------%*/

void aecDTM_clearPointIgnoreSegmentFlag
(
  struct CIVdtmpnt *pntP                /* => point to use              */
)
{
  if ( pntP != (struct CIVdtmpnt *)0 ) pntP->flg &= ~DTM_C_PNTIGN;
}


/*%-----------------------------------------------------------------------
 FUNC: aecDTM_isLockTrianglesFlagSet
 DESC: 
 HIST: 
 MISC:
 KEYW: 
-----------------------------------------------------------------------%*/

int aecDTM_isLockTrianglesFlagSet /* <= TRUE if set       */
(
  struct CIVdtmsrf *srfP                 /* => surface to check         */
)
{
  int sts = FALSE;
  if ( srfP != (struct CIVdtmsrf *)0  &&  srfP->par.lockTin ) sts = TRUE;
  return ( sts );
}


/*%-----------------------------------------------------------------------
 FUNC: aecDTM_isTinOutOfDateFlagSet
 DESC: Returns TRUE if input surface's the triangles not up to date bit flag is set
 HIST: Original - tmi 18-Jan-1994
 MISC:
 KEYW: DTM SURFACE FLAG TRIANGLES NOT UP TO DATE
-----------------------------------------------------------------------%*/

int aecDTM_isSurfaceTinOutOfDateFlagSet /* <= TRUE if set       */
(
  struct CIVdtmsrf *srfP                 /* => surface to check         */
)
{
  int sts = FALSE;
  if ( srfP != (struct CIVdtmsrf *)0  &&  srfP->par.tinOutOfDate ) sts = TRUE;
  return ( sts );
}
