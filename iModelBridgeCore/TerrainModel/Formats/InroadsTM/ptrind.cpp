//---------------------------------------------------------------------------+
// $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+
/*----------------------------------------------------------------------------*/
/* ptrind.c                                            tmi    10-Apr-1990     */
/*----------------------------------------------------------------------------*/
/* Contains functions to change pointers into indeces and vice versa.         */
/* These transformations are required when saving and loading surface         */
/* and alignment information to and from the dtm project file.                */
/*----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*/
/* Include files                                                              */
/*----------------------------------------------------------------------------*/
#include "stdafx.h"



/*----------------------------------------------------------------------------*/
/* Private static function prototypes                                         */
/*----------------------------------------------------------------------------*/
static void aecDTM_convertIndexToPtrProcess(size_t *,struct CIVptrind *,struct CIVptrind *,size_t *);






/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_buildPtrIndexTable
 DESC: Builds the pointer / index table.
 HIST: Original - tmi 10-Apr-1990
 MISC:
 KEYW: DTM POINTER INDEX TABLE BUILD
-----------------------------------------------------------------------------%*/

void aecDTM_buildPtrIndexTable
(
  struct CIVdtmsrf *srfP,                /* => surface to use                 */
  struct CIVptrind *ptrP,                /* => point/index table              */
  size_t *pTinOffset              /* => # pnts before 1st tin (or NULL)*/
)
{
  struct CIVdtmblk *blkP;
  int i, j;
  size_t tmp;

  if ( pTinOffset ) *pTinOffset = 0L;

  for ( tmp = 0, i = j = 0; i < DTM_C_NMPNTF; i++ )
    for ( blkP = srfP->pntf[i]->blk; blkP; blkP = blkP->nxt, j++ )
    {
      ptrP[j].ind = tmp;
      ptrP[j].p = (unsigned char *) blkP->rec.pnt;
      tmp += blkP->use;
    }

  for ( blkP = srfP->rngf->blk; blkP; blkP = blkP->nxt, j++ )
  {
    ptrP[j].ind = tmp;
    ptrP[j].p = (unsigned char *) blkP->rec.pnt;
    tmp += blkP->use;
  }

  if ( srfP->version > 5 )
  {
    for ( i = 0; i < DTM_C_NMFTRF; i++ )
      for ( blkP = srfP->ftrf[i]->blk; blkP; blkP = blkP->nxt, j++ )
      {
        ptrP[j].ind = tmp;
        ptrP[j].p = (unsigned char *) blkP->rec.ftr;
        tmp += blkP->use;
      }

    for ( blkP = srfP->styf->blk; blkP; blkP = blkP->nxt, j++ )
    {
      ptrP[j].ind = tmp;
      ptrP[j].p = (unsigned char *) blkP->rec.sty;
      tmp += blkP->use;
    }

    if ( srfP->version > 7 )
    {
      for ( blkP = srfP->payf->blk; blkP; blkP = blkP->nxt, j++ )
      {
        ptrP[j].ind = tmp;
        ptrP[j].p = (unsigned char *) blkP->rec.pay;
        tmp += blkP->use;
      }
    }
  }

  if ( srfP->version > 8 )
  {
    for ( blkP = srfP->corf->blk; blkP; blkP = blkP->nxt, j++ )
    {
      ptrP[j].ind = tmp;
      ptrP[j].p = (unsigned char *) blkP->rec.cor;
      tmp += blkP->use;
    }

    for ( blkP = srfP->cmpf->blk; blkP; blkP = blkP->nxt, j++ )
    {
      ptrP[j].ind = tmp;
      ptrP[j].p = (unsigned char *) blkP->rec.cmp;
      tmp += blkP->use;
    }

    for ( blkP = srfP->cmpMemf->blk; blkP; blkP = blkP->nxt, j++ )
    {
      ptrP[j].ind = tmp;
      ptrP[j].p = (unsigned char *) blkP->rec.cmpMem;
      tmp += blkP->use;
    }
  }

  if ( pTinOffset ) *pTinOffset = tmp;

  for ( blkP = srfP->tinf->blk; blkP; blkP = blkP->nxt, j++ )
  {
    ptrP[j].ind = tmp;
    ptrP[j].p = (unsigned char *) blkP->rec.tin;
    tmp += blkP->use;
  }
}





/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_convertIndexToPtr
 DESC: Converts indeces to pointers.
 HIST: Original - tmi 10-Apr-1990
 MISC:
 KEYW: DTM POINTER INDEX CONVERT
-----------------------------------------------------------------------------%*/

void aecDTM_convertIndexToPtr
(
  struct CIVdtmsrf *srfP,                    /* => surface to use             */
  struct CIVptrind *ptrP,                    /* => first entry of p/i table   */
  struct CIVptrind *eptrP                    /* => last entry of p/i table    */
)
{
  struct CIVdtmblk *blkP;
  struct CIVdtmtin *tinP;
  struct CIVdtmftr *ftrP;
  size_t sizp, sizt, sizs, sizpay;
  int i;

  sizp = sizeof(struct CIVdtmpnt);
  sizt = sizeof(struct CIVdtmtin);
  sizs = sizeof(struct CIVdtmsty);
  sizpay = sizeof(struct CIVdtmpay);

  for ( blkP = srfP->tinf->blk; blkP; blkP = blkP->nxt )
    for ( tinP = blkP->rec.tin; tinP < blkP->rec.tin + blkP->use; tinP++ )
    {
      aecDTM_convertIndexToPtrProcess ( (size_t *)&tinP->p1,  ptrP, eptrP, &sizp );
      aecDTM_convertIndexToPtrProcess ( (size_t *)&tinP->p2,  ptrP, eptrP, &sizp );
      aecDTM_convertIndexToPtrProcess ( (size_t *)&tinP->p3,  ptrP, eptrP, &sizp );
      aecDTM_convertIndexToPtrProcess ( (size_t *)&tinP->n12, ptrP, eptrP, &sizt );
      aecDTM_convertIndexToPtrProcess ( (size_t *)&tinP->n23, ptrP, eptrP, &sizt );
      aecDTM_convertIndexToPtrProcess ( (size_t *)&tinP->n31, ptrP, eptrP, &sizt );
    }

  if ( srfP->version > 5 )
  {
    for ( i = 0; i < DTM_C_NMFTRF; i++ )
      for ( blkP = srfP->ftrf[i]->blk; blkP; blkP = blkP->nxt )
        for ( ftrP = blkP->rec.ftr; ftrP < blkP->rec.ftr + blkP->use; ftrP++ )
        {
          aecDTM_convertIndexToPtrProcess ( (size_t *)&ftrP->p1,  ptrP, eptrP, &sizp );
          aecDTM_convertIndexToPtrProcess ( (size_t *)&ftrP->s1,  ptrP, eptrP, &sizs );

          if ( srfP->version > 7 )
            aecDTM_convertIndexToPtrProcess ( (size_t *)&ftrP->pay, ptrP, eptrP, &sizpay );
        }
  }
}




/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_convertIndexToPtrProcess
 DESC: Converts a set of indeces to pointers.
 HIST: Original - tmi 10-Apr-1990
 MISC: static
 KEYW: DTM POINTER INDEX CONVERT PROCESS
-----------------------------------------------------------------------------%*/

static void aecDTM_convertIndexToPtrProcess
(
  size_t *ftrP,
  struct CIVptrind *ptrP,
  struct CIVptrind *eptrP,
  size_t *sizP
)
{
  //assert ( (*ftrP >= 0 && *ftrP <= LONG_MAX) || *ftrP == -1 );

  if ( *ftrP == -1 )
    *ftrP = 0L;
  else
  {
    struct CIVptrind *p1P, *p2P, *pP;

    p1P = ptrP;
    p2P = eptrP;
    pP  = (struct CIVptrind *)0;

    while ( p2P > p1P + 1 )
    {
      pP = p1P + ( (p2P-p1P) >> 1 );
      if ( pP->ind >= *ftrP )
        p2P = pP;
      else
        p1P = pP;
    }

    pP = p2P->ind > *ftrP ? p1P : p2P;

    *ftrP = *ftrP - pP->ind;
    *ftrP = (size_t) pP->p + *ftrP * *sizP;
  }
}




/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_convertPtrToIndex
 DESC: Converts a set of pointers to indeces.
 HIST: Original - tmi 10-Apr-1990
 MISC:
 KEYW: DTM POINTER INDEX CONVERT PROCESS
-----------------------------------------------------------------------------%*/

void aecDTM_convertPtrToIndex
(
  size_t *ftrP,                      /* => address of thing to convert        */
  struct CIVptrind *ptrP,            /* => first entry in p/i table           */
  struct CIVptrind *eptrP,           /* => last entry in p/i table            */
  size_t *sizP                       /* => size of input entry                */
)
{
  //assert ( *ftrP >= 0 && *ftrP <= LONG_MAX );

  if ( *ftrP == 0L )
    *ftrP = (size_t)-1;
  else
  {
    struct CIVptrind *p1P, *p2P, *pP;

    p1P = ptrP;
    p2P = eptrP;
    pP = (struct CIVptrind *)0;

    while ( p2P > p1P + 1 )
    {
      pP = p1P + ( (p2P-p1P) >> 1 );
      if ( (size_t)pP->p >= (size_t) *ftrP )
        p2P = pP;
      else
        p1P = pP;
    }

    pP = (size_t) p2P->p > (size_t) *ftrP ? p1P : p2P;

    *ftrP = (size_t) *ftrP - (size_t) pP->p;
    *ftrP = pP->ind + *ftrP / *sizP;
  }
}





static int aecDTM_comparePoints
(
  const void *a,		/* (i) */
  const void *b			/* (i) */
)
{
  struct CIVptrind *p1P = (struct CIVptrind *)a;
  struct CIVptrind *p2P = (struct CIVptrind *)b;

  if( p1P->p < p2P->p )
    return( -1 );
  else if( p1P->p > p2P->p )
    return( 1 );
  else
    return( 0 );
}


/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_sortPtrIndexTableByPtr
 DESC: Sort the pointer - index table using the pointer as the key.
 HIST: Original - tmi 10-Apr-1990
 MISC:
 KEYW: DTM POINTER INDEX SORT
-----------------------------------------------------------------------------%*/

void aecDTM_sortPtrIndexTableByPtr
(
  struct CIVptrind *ptrP,                    /* => first entry in p/i table   */
  struct CIVptrind *eptrP                    /* => last entry in p/i table    */
)
{
  size_t num = 1 + (eptrP - ptrP);

  qsort ( (void *)ptrP, num, sizeof(struct CIVptrind), aecDTM_comparePoints );
}




/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_sortPtrIndexTableByIndex
 DESC: Sort the pointer - index table using the index as the key.
 HIST: Original - tmi 10-Apr-1990
 MISC:
 KEYW: DTM POINTER INDEX SORT
-----------------------------------------------------------------------------%*/

void aecDTM_sortPtrIndexTableByIndex
(
  struct CIVptrind *ptrP,                    /* => first entry in p/i table   */
  struct CIVptrind *eptrP                    /* => last entry in p/i table    */
)
{
  struct CIVptrind *p1P, *p2P, tmp;

  for ( p1P = ptrP; p1P < eptrP; p1P++ )
    for ( p2P = p1P+1; p2P <= eptrP; p2P++ )
      if ( p2P->ind < p1P->ind )
      {
	tmp = *p1P;
	*p1P = *p2P;
	*p2P = tmp;
      }
}




/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_indexTableBuild
 DESC: Builds a hidden pointer/index table that Civil SDK user's can use to
       obtain index id's of point's and triangles.
 HIST: Original - tmi 23-Jun-1994
 MISC:
 KEYW: POINTER INDEX TABLE BUILD
-----------------------------------------------------------------------------%*/

int aecDTM_indexTableBuild  /* <= TRUE if error                    */
(
  struct CIVdtmsrf *srfP               /* => surface to use                   */
)
{
  int sts = SUCCESS;

  if ( srfP != (struct CIVdtmsrf *)0 )
  {
    int nblk;

    aecDTM_indexTableFree ( srfP );

    aecDTM_countSurfaceBlocks ( &nblk, srfP );
    if ( nblk > 0 )
    {
      struct CIVptrindTwo *ptrIndexP, *indexPtrP;

      if      ( ( ptrIndexP = (struct CIVptrindTwo *)calloc ( 1, sizeof(struct CIVptrindTwo) ) ) == (void *)0 )
        sts = DTM_M_MEMALF;
      else if ( ( indexPtrP = (struct CIVptrindTwo *)calloc ( 1, sizeof(struct CIVptrindTwo) ) ) == (void *)0 )
        sts = DTM_M_MEMALF;
      else if ( ( ptrIndexP->piP = (struct CIVptrind *) calloc ( (unsigned int)nblk, sizeof(struct CIVptrind) ) ) == 0L )
        sts = DTM_M_MEMALF;
      else if ( ( indexPtrP->piP = (struct CIVptrind *) calloc ( (unsigned int)nblk, sizeof(struct CIVptrind) ) ) == 0L )
        sts = DTM_M_MEMALF;
      else
      {
        ptrIndexP->epiP = ptrIndexP->piP + nblk - 1;
        aecDTM_buildPtrIndexTable ( srfP, ptrIndexP->piP, &ptrIndexP->tinOffset );
        aecDTM_sortPtrIndexTableByPtr ( ptrIndexP->piP, ptrIndexP->epiP );
        ptrIndexP->buildTable = FALSE;
        srfP->ptrIndexTableP = ptrIndexP;

        indexPtrP->epiP = indexPtrP->piP + nblk - 1;
        aecDTM_buildPtrIndexTable ( srfP, indexPtrP->piP, &indexPtrP->tinOffset );
        aecDTM_sortPtrIndexTableByIndex ( indexPtrP->piP, indexPtrP->epiP );
        indexPtrP->buildTable = FALSE;
        srfP->indexPtrTableP = indexPtrP;
      }
    }
  }

  return ( sts );
}




/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_indexTableFree
 DESC: Frees the hidden pointer/index table.
 HIST: Original - tmi 23-Jun-1994
 MISC:
 KEYW: POINTER INDEX TABLE FREE
-----------------------------------------------------------------------------%*/

int aecDTM_indexTableFree   /* <= TRUE if error                    */
(
  struct CIVdtmsrf *srfP               /* => surface with table to free       */
)
{
  int sts = SUCCESS;

  if ( srfP != (struct CIVdtmsrf *)0 )
  {
    if ( srfP->ptrIndexTableP != (void *)0 )
    {
      struct CIVptrindTwo *ptrIndexP = (struct CIVptrindTwo *)srfP->ptrIndexTableP;

      if ( ptrIndexP->piP != (struct CIVptrind *)0 )
          free ( ptrIndexP->piP );
      else
          free ( srfP->ptrIndexTableP );
    }
    srfP->ptrIndexTableP = (void *)0;


    if ( srfP->indexPtrTableP != (void *)0 )
    {
      struct CIVptrindTwo *indexPtrP = (struct CIVptrindTwo *)srfP->indexPtrTableP;

      if ( indexPtrP->piP != (struct CIVptrind *)0 )
          free ( indexPtrP->piP );
      else
          free ( srfP->indexPtrTableP );
    }
    srfP->indexPtrTableP = (void *)0;
  }

  return ( sts );
}



