//---------------------------------------------------------------------------+
// $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+
/*----------------------------------------------------------------------------*/
/* pntadd.c                                                tmi    17-Oct-1990 */
/*----------------------------------------------------------------------------*/
/* Adds a single regular point to the specified surface using the coordinates */
/* identified by the user.                                                    */
/*----------------------------------------------------------------------------*/

#include "stdafx.h"

// USTN-XM TODO: Implement transient display

/*----------------------------------------------------------------------------*/
/* Private static function prototypes                                         */
/*----------------------------------------------------------------------------*/
static int  aecDTM_pointAddAffected(struct CIVpntedt *);
static int  aecDTM_pointAddCreate(struct CIVpntedt *);
static int  aecDTM_pointAddRestore(struct CIVpntedt *);
static int  aecDTM_pointAddFree(struct CIVpntedt *);






/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_pointAdd
 DESC: Adds a single regular point to the specified surface using the
       coordinates identified by the user.
 HIST: Original - tmi 17-Oct-1990
 MISC: The check for the maximum triangle side length is not used.
       Range triangles are displayed during the editting.
 KEYW: DTM POINT ADD DYNAMICS
-----------------------------------------------------------------------------%*/

int aecDTM_pointAdd            /* <= TRUE if error                 */
(
  struct CIVpntedt *pntaddP               /* => point edit data structure     */
)
{
  static struct CIVdtmpnt pnt;
  static unsigned char add = 0;
  static int writeState = 0;
  static int cleaned = TRUE;
  int sts = SUCCESS;

  switch ( pntaddP->state )
  {
    case ( PNTEDT_INIT ) :                                  /* initialize     */
      {
        memset ( &pnt, 0, sizeof(struct CIVdtmpnt) );
        if( cleaned )
        {
          cleaned = FALSE;
        }
        pntaddP->pnt = &pnt;
        aecDTM_countSurfaceData ( &pntaddP->npnt, &pntaddP->ntin, pntaddP->srf );
        if ( !(pntaddP->opt & PNTEDT_TICK) ) aecTicker_stop();
      }
      break;

    case ( PNTEDT_STRD ) :                                  /* start dynamics */
      pntaddP->tin = (struct CIVdtmtin *)0;
      sts = aecDTM_findTriangle ( &pntaddP->tin, 0, &pntaddP->rpt, 0, pntaddP->srf, &pntaddP->loc );
      if ( pntaddP->rpt == -1 )
      {
        sts = aecDTM_pointAddAffected ( pntaddP );
        if ( add )
          sts = aecDTM_addPointsExt ( &pntaddP->pnt, pntaddP->srf, DTM_C_DTMREG, 1L, &pntaddP->loc );
        else
          DTMDPOINTTOPOINT ( pntaddP->srf, pntaddP->loc, pntaddP->pnt );
        sts = aecDTM_pointAddCreate ( pntaddP );
      }
      break;

    case ( PNTEDT_PRCD ) :                                  /* process dyns.  */
      if ( pntaddP->rpt == -1 )
      {
        sts = aecDTM_pointAddRestore ( pntaddP );
      }
      pntaddP->state = PNTEDT_STRD, aecDTM_pointAdd ( pntaddP );
      break;

    case ( PNTEDT_STPD ) :                                  /* end dynamics   */
      if ( pntaddP->rpt == -1 )
      {
        sts = aecDTM_pointAddRestore ( pntaddP );
        sts = aecDTM_pointAddFree ( pntaddP );
      }
      break;

    case ( PNTEDT_ADD ) :                                   /* add point      */
      pntaddP->state = PNTEDT_STPD, aecDTM_pointAdd ( pntaddP );
      add = 1;
      pntaddP->state = PNTEDT_STRD, aecDTM_pointAdd ( pntaddP );
      add = 0;
      sts = aecDTM_pointAddFree ( pntaddP );
      pntaddP->state = PNTEDT_INIT, aecDTM_pointAdd ( pntaddP );
      pntaddP->rpt = 1;
      break;

    case ( PNTEDT_CLEAN ) :                                 /* cleanup        */
      sts = aecDTM_pointAddFree ( pntaddP );
      cleaned = TRUE;
      break;
  }

  return ( sts );
}



/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_pointAddAffected
 DESC: Checks to see of the triangles which we are about to generate
       affect the same set of triangles that were just put back.  If so,
       then there is no need to draw and then erase these triangles.
 HIST: Original - tmi 05-Nov-1990
 MISC: static
 KEYW: DTM POINT ADD AFFECTED
-----------------------------------------------------------------------------%*/

static int aecDTM_pointAddAffected
(
  struct CIVpntedt *pntadd
)
{
  struct CIVpntedt tmp;
  int sts = SUCCESS, sam = ( pntadd->ntinlst == 0 ) ? 0 : 1;
  long i, j;

  sts = aecDTM_findAffectedTriangles ( &pntadd->tin, &tmp.ntinlst, &tmp.tinlst, pntadd->srf, &pntadd->loc );
  for ( i = 0; i < pntadd->ntinlst  &&  sam; i++ )
  {
    for ( j = 0; j < tmp.ntinlst; j++ )
      if ( tmp.tinlst[j] == pntadd->tinlst[i] )
        break;
    if ( j == tmp.ntinlst ) sam = 0;
  }
  for ( i = 0; i < tmp.ntinlst  &&  sam; i++ )
  {
    for ( j = 0; j < pntadd->ntinlst; j++ )
      if ( pntadd->tinlst[j] == tmp.tinlst[i] )
        break;
    if ( j == pntadd->ntinlst ) sam = 0;
  }

  if ( ! sam )
  {
  }

  sts = aecDTM_pointAddFree ( pntadd );
  sts = aecDTM_pointAddFree ( &tmp );

  return ( sts );
}




/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_pointAddCreate
 DESC: Called when adding a point and triangulating in a local mode.
 HIST: Original - tmi 02-Nov-1990
 MISC: static
 KEYW: DTM POINT ADD CREATE
-----------------------------------------------------------------------------%*/

static int aecDTM_pointAddCreate
(
  struct CIVpntedt *pntadd
)
{
  int sts = SUCCESS, rpt;

  if ( pntadd->srf->tinf->nrec - pntadd->srf->tinf->ndel < 1L )
  {
    int statusState = aecParams_getStatusState( NULL );
    int writeState = 0;
    int tmp = 0;

    aecParams_setStatusState ( &tmp );
    aecDTM_triangulate ( NULL, NULL, NULL, pntadd->srf, 0, NULL, NULL, NULL, NULL );
    aecParams_setStatusState ( &statusState );

  }


  if ( pntadd->ntin > 0L )
    if ( aecDTM_findTriangle ( &pntadd->tin, 0, &rpt, 0, pntadd->srf, &pntadd->loc ) == SUCCESS  &&  rpt != -1 )
      sts = aecDTM_findAffectedTriangles ( &pntadd->tin, &pntadd->ntinlst, &pntadd->tinlst, pntadd->srf, &pntadd->loc );
    else
    {
      aecDTM_updateSurfaceRange ( pntadd->srf, pntadd->pnt );
      if ( ( sts = aecDTM_triangulatePoint ( pntadd->srf, pntadd->srf->regf, pntadd->pnt, &pntadd->tin, &pntadd->ntinlst, &pntadd->tinlst ) ) == SUCCESS )
        aecDTM_pointDeleteCleanup ( pntadd->srf, pntadd->pnt, pntadd->ntinlst, pntadd->tinlst );
    }

  return ( sts );
}



/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_pointAddRestore
 DESC: Restores the model to it's previous state.
 HIST: Original - tmi 02-Nov-1990
 MISC: static
 KEYW: DTM POINT ADD RESTORE
-----------------------------------------------------------------------------%*/

static int aecDTM_pointAddRestore
(
  struct CIVpntedt *pntadd
)
{
  int sts = SUCCESS;
  long i, npntlst = 0, *pntlst = NULL, nnbrlst, *nbrlst = NULL;

  for ( i = 0; i < pntadd->ntinlst  &&  sts == SUCCESS; i++ )
    sts = aecDTM_triangleStackPut ( pntadd->srf, (struct CIVdtmtin *)pntadd->tinlst[i], -1 );

  if ( sts == SUCCESS )
    if ( ( sts = aecDTM_pointAddFree ( pntadd ) ) == SUCCESS )
      if ( ( sts = aecDTM_getPointNeighbors ( &npntlst, &pntlst, (long *)0, (long **)0, &nnbrlst, &nbrlst, pntadd->srf, pntadd->pnt, (struct CIVdtmtin *)0, 0 ) ) == SUCCESS )
        if ( ( sts = aecDTM_patch ( pntadd->srf, npntlst, pntlst, nbrlst, &pntadd->ntinlst, &pntadd->tinlst ) ) == SUCCESS )
          aecDTM_pointDeleteCleanup ( pntadd->srf, pntadd->pnt, pntadd->ntinlst, pntadd->tinlst );

  if ( npntlst > 0L )
      free ( pntlst );
  if ( npntlst > 1L )
      free ( nbrlst );

  return ( sts );
}




/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_pointAddFree
 DESC: Frees up the triangle list memory.
 HIST: Original - tmi 02-Nov-1990
 MISC: static
 KEYW: DTM POINT ADD FREE MEMORY
-----------------------------------------------------------------------------%*/

static int aecDTM_pointAddFree
(
  struct CIVpntedt *pntadd
)
{
  int sts = SUCCESS;

  if ( pntadd->ntinlst > 0L )
      free ( pntadd->tinlst );
  pntadd->ntinlst = 0L;
  pntadd->tinlst = (long *)0;
  pntadd->tin = (struct CIVdtmtin *)0;

  return ( sts );
}



