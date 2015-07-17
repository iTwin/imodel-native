//---------------------------------------------------------------------------+
// $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+
/*----------------------------------------------------------------------------*/
/* pntdel.c                                              tmi    17-Oct-1990   */
/*----------------------------------------------------------------------------*/
/* Controls interactive deletion a DTM point.                                 */
/*----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*/
/* Include files                                                              */
/*----------------------------------------------------------------------------*/
#include "stdafx.h"




/*----------------------------------------------------------------------------*/
/* Private static function prototypes                                         */
/*----------------------------------------------------------------------------*/
static int aecDTM_pointDeleteProcess(struct CIVpntedt *);






/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_pointDelete
 DESC: Controls interactive deletion a DTM point.
 HIST: Original - tmi 17-Oct-1990
 MISC:
 KEYW: DTM POINT DELETE DYNAMICS
-----------------------------------------------------------------------------%*/

int aecDTM_pointDelete            /* <= TRUE if error              */
(
  struct CIVpntedt *pntdelP                  /* => point edit data structure  */
)
{
  int sts = SUCCESS;

  switch ( pntdelP->state )
  {
    case ( PNTEDT_INIT ) :                                /* initialize */
      aecDTM_countSurfaceData ( (long *)0, &pntdelP->ntin, pntdelP->srf );
      if ( pntdelP->opt & PNTEDT_CON )
      {
        aecTicker_stop();
      }
      break;

    case ( PNTEDT_PRCD ) :                                /* process */
      sts = aecDTM_pointDeleteProcess ( pntdelP );
      break;
  }

  return ( sts );
}




/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_pointDeleteProcess
 DESC: Deletes the point and patches the hole that was left.
 HIST: Original - tmi 21-Feb-1991
 MISC: static
 KEYW: DTM POINT DELETE DYNAMICS PROCESS
-----------------------------------------------------------------------------%*/

static int aecDTM_pointDeleteProcess
(
  struct CIVpntedt *pntdel
)
{
  struct CIVdtmfil *fil = (struct CIVdtmfil *)0;
  int sts = SUCCESS;
  long i, npntlst, *pntlst, ntinlst, *tinlst, nnbrlst, *nbrlst;

  if ( !(pntdel->opt & PNTEDT_NOTRI) )
    if ( pntdel->ntin > 0L )
      if ( ( sts = aecDTM_getPointNeighbors ( &npntlst, &pntlst, &ntinlst, &tinlst, &nnbrlst, &nbrlst, pntdel->srf, pntdel->pnt, pntdel->tin, 0 ) ) == SUCCESS )
      {
        if ( !(pntdel->opt & PNTEDT_NOPATCH) )
        {
          for ( i = 0; i < ntinlst  &&  sts == SUCCESS; i++ )
            sts = aecDTM_triangleStackPut ( pntdel->srf, (struct CIVdtmtin *) tinlst[i], -1 );

          if ( ntinlst > 0L )
              free ( (void*)tinlst );

          if ( sts == SUCCESS )
            if ( ( sts = aecDTM_patch ( pntdel->srf, npntlst, pntlst, nbrlst, &ntinlst, &tinlst ) ) == SUCCESS )
            {
              aecDTM_pointDeleteCleanup ( pntdel->srf, pntdel->pnt, ntinlst, tinlst );
              sts = aecDTM_findPointFile ( &fil, pntdel->srf, pntdel->pnt );
            }
        }
        else
        {
            for ( i = 0; i < ntinlst; i++ )
                aecDTM_deleteTriangle ( pntdel->srf, (struct CIVdtmtin *)tinlst[i], -1 );
        }

        if ( npntlst > 0L )
            free ( (void *)pntlst );
        if ( nnbrlst > 0L )
            free ( (void *)nbrlst );
        if ( ntinlst > 0L )
            free ( (void *)tinlst );
      }

  aecDTM_deletePoint ( pntdel->srf, fil, pntdel->pnt );

  return ( sts );
}




/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_pointDeleteCleanup
 DESC: It checks to see if the input point is within a boundary.  If so,
       all the triangles connected to it are deleted.  Also, a check is
       made to see if any of the triangles are range triangles.  If so,
       they are deleted.  As of 13-Dec-1993, it also applies the maximum
       triangle length to the list of triangles.
 HIST: Original - tmi 01-Nov-1990
 MISC:
 KEYW: DTM POINT DELETE CLEANUP
-----------------------------------------------------------------------------%*/

void aecDTM_pointDeleteCleanup
(
  struct CIVdtmsrf *srfP,            /* => surface your using                 */
  struct CIVdtmpnt *pntP,            /* => point we deleted                   */
  long nnewlst,                      /* => # tins in affected list            */
  long *newlstP                      /* => array of ptrs to triangle list     */
)
{
  struct CIVdtmtin *tinP;
  long i;
  double siz = srfP->par.maxsid * srfP->par.maxsid;

  if ( !aecDTM_isPointProcessedFlagSet(pntP)  &&  aecDTM_isPointInsideBoundary ( srfP, pntP ) )
    for ( i = 0; i < nnewlst; i++ )
    {
      tinP = (struct CIVdtmtin *) newlstP[i];
      if ( !aecDTM_isTriangleDeletedFlagSet(tinP) ) aecDTM_setTriangleDeletedFlag(tinP), srfP->tinf->ndel++;
      if ( srfP->dis.tinfnc )(*srfP->dis.tinfnc) ( srfP, tinP, 2, srfP->dis.tinsym );
      if ( srfP->dis.confnc )(*srfP->dis.confnc) ( srfP, tinP, 2, srfP->dis.consym );
    }

  for ( i = 0; i < nnewlst; i++ )
  {
    tinP = (struct CIVdtmtin *) newlstP[i];

    if ( aecDTM_isTriangleRangeTriangle ( srfP, tinP ) )
    {
      if ( !aecDTM_isTriangleDeletedFlagSet(tinP) ) aecDTM_setTriangleDeletedFlag(tinP), srfP->tinf->ndel++;
      if ( srfP->dis.tinfnc )(*srfP->dis.tinfnc) ( srfP, tinP, 2, srfP->dis.tinsym );
      if ( srfP->dis.confnc )(*srfP->dis.confnc) ( srfP, tinP, 2, srfP->dis.consym );
    }
    else
      aecDTM_applyMaximumTriangleLength ( srfP, siz, tinP );
  }
}



