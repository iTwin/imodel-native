//---------------------------------------------------------------------------+
// $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+
/*----------------------------------------------------------------------------*/
/* counts.c                                            tmi	10-Apr-1990   */
/*----------------------------------------------------------------------------*/
/* Contains all functions which count things in the product.		      */
/*----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*/
/* Include files                                                              */
/*----------------------------------------------------------------------------*/
#include "stdafx.h"


/*----------------------------------------------------------------------------*/
/* Private static function prototypes                                         */
/*----------------------------------------------------------------------------*/
static int aecDTM_countBoundariesProcess(void *,int,long,DPoint3d *,struct CIVdtmpnt *);




/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_countSurfaceBlocks
 DESC: Count the number of allocated blocks of memory in a surface.
 HIST: Original - tmi 10-Apr-1990
 MISC:
 KEYW: DTM COUNT BLOCKS
-----------------------------------------------------------------------------%*/

void aecDTM_countSurfaceBlocks
(
  int *nblkP,                          /* <= # of blocks                        */
  struct CIVdtmsrf *srfP               /* => surface to use                     */
)
{
  if ( nblkP != (int *)0 )
  {
    *nblkP = 0;

    if ( srfP != (struct CIVdtmsrf *)0 )
    {
      struct CIVdtmblk *blkP;
      int i;

      *nblkP = 0;

      for ( i = 0; i < DTM_C_NMPNTF; i++ )
      {
        blkP = srfP->pntf[i]->blk;
        while ( blkP )
        {
          (*nblkP)++;
          blkP = blkP->nxt;
        }
      }

      blkP = srfP->rngf->blk;
      while ( blkP )
      {
        (*nblkP)++;
        blkP = blkP->nxt;
      }

      blkP = srfP->tinf->blk;
      while ( blkP )
      {
        (*nblkP)++;
        blkP = blkP->nxt;
      }
      
      if ( srfP->version > 5 )
      {
        for ( i = 0; i < DTM_C_NMFTRF; i++ )
        {
          blkP = srfP->ftrf[i]->blk;
          while ( blkP )
          {
            (*nblkP)++;
            blkP = blkP->nxt;
          }
        }

        blkP = srfP->styf->blk;
        while ( blkP )
        {
          (*nblkP)++;
          blkP = blkP->nxt;
        }

        if ( srfP->version > 7 )
        {
          blkP = srfP->payf->blk;
          while ( blkP )
          {
            (*nblkP)++;
            blkP = blkP->nxt;
          }
        }
      }

      if ( srfP->version > 8 )
      {
        blkP = srfP->corf->blk;
        while ( blkP )
        {
          (*nblkP)++;
          blkP = blkP->nxt;
        }

        blkP = srfP->cmpf->blk;
        while ( blkP )
        {
          (*nblkP)++;
          blkP = blkP->nxt;
        }

        blkP = srfP->cmpMemf->blk;
        while ( blkP )
        {
          (*nblkP)++;
          blkP = blkP->nxt;
        }

      }
    }
  }
}




/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_countSurfacePoints
 DESC: Counts the total number of points in a surface.
 HIST: Original - tmi 10-Apr-1990
 MISC:
 KEYW: DTM COUNT POINTS
-----------------------------------------------------------------------------%*/

void aecDTM_countSurfacePoints
(
  long *npntP,                         /* <= # points in surface               */
  struct CIVdtmsrf *srfP               /* => surface to use                    */
)
{
  if ( npntP != (long *)0 )
  {
    *npntP = 0L;

    if ( srfP != (struct CIVdtmsrf *)0 )
    {
      long i;

      for ( *npntP = 0, i = 0; i < DTM_C_NMPNTF; *npntP += srfP->pntf[i]->nrec, i++ )
        aecDTM_countSurfacePointsInFile ( srfP->pntf[i] );
    }
  }
}



/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_countSurfacePointsInFile
 DESC: Counts the number of points within a certain file.
 HIST: Original - tmi 24-Apr-1990
 MISC:
 KEYW: DTM COUNT POINTS FILE
-----------------------------------------------------------------------------%*/

void aecDTM_countSurfacePointsInFile
(
  struct CIVdtmfil *filP               /* => file to count                      */
)
{
  if ( filP != (struct CIVdtmfil *)0 )
  {
    struct CIVdtmblk *blkP;
    struct CIVdtmpnt *pntP;

    for ( filP->nrec = filP->ndel = 0, blkP = filP->blk; blkP; blkP = blkP->nxt )
      for ( filP->nrec += blkP->use, pntP = blkP->rec.pnt; pntP < blkP->rec.pnt + blkP->use; pntP++ )
        if ( aecDTM_isPointDeletedFlagSet ( pntP ) )
          filP->ndel++;
  }
}




/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_countSurfaceTriangles
 DESC: Counts the total number of triangles in a surface.
 HIST: Original - tmi 10-Apl-1990
 MISC:
 KEYW: DTM COUNT TRIANGLES
-----------------------------------------------------------------------------%*/

void aecDTM_countSurfaceTriangle
(
  long *ntinP,                         /* <= # triangles in surface               */
  struct CIVdtmsrf *srfP               /* => surface to use                       */
)
{
  if ( ntinP != (long *)0 )
  {
    *ntinP = 0L;

    if ( srfP != (struct CIVdtmsrf *)0 )
    {
      struct CIVdtmfil *filP;
      struct CIVdtmblk *blkP;
      struct CIVdtmtin *tinP;

      for ( filP = srfP->tinf, filP->nrec = filP->ndel = 0, blkP = filP->blk; blkP; blkP = blkP->nxt )
        for ( filP->nrec += blkP->use, tinP = blkP->rec.tin; tinP < blkP->rec.tin + blkP->use; tinP++ )
          if ( aecDTM_isTriangleDeletedFlagSet ( tinP ) )
            filP->ndel++;

      *ntinP = filP->nrec;
    }
  }
}




/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_countSurfaceData
 DESC: Counts points and triangles using the information in the file headers.
       The deleted points and triangles are NOT included in totals.
 HIST: Original - tmi 17-Oct-1990
 MISC:
 KEYW: DTM COUNT POINT TRIANGLE QUICKLY
-----------------------------------------------------------------------------%*/

void aecDTM_countSurfaceData
(
  long *npntP,                         /* <= # points in surface                  */
  long *ntinP,                         /* <= # triangles in surface               */
  struct CIVdtmsrf *srfP               /* => surface to use                       */
)
{
  if ( npntP != (long *)0 ) *npntP = 0L;
  if ( ntinP != (long *)0 ) *ntinP = 0L;

  if ( srfP != (struct CIVdtmsrf *)0 )
  {
    if ( npntP != (long *)0 )
    {
      int i;
      long tmp;

      for ( i = 0; i < DTM_C_NMPNTF; *npntP += tmp, i++ )
        aecDTM_countSurfaceDataInFile ( &tmp, srfP, i );
    }

    if ( ntinP != (long *)0 )
      *ntinP = srfP->tinf->nrec - srfP->tinf->ndel;
  }
}



/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_countSurfaceDataInFile
 DESC: It quickly computes the number of non-deleted points in the
       specified file.
 HIST: Original - tmi 20-Apr-1991
 MISC:
 KEYW: DTM COUNT FILE QUICKLY
-----------------------------------------------------------------------------%*/

void aecDTM_countSurfaceDataInFile
(
  long *nrecP,                         /* <= # records in file                 */
  struct CIVdtmsrf *srfP,              /* => surface to use                    */
  int typ                              /* => point type                        */
)
{
  if ( nrecP != (long *)0 )
  {
    *nrecP = 0L;

    if ( srfP != (struct CIVdtmsrf *)0 )
      *nrecP = srfP->pntf[typ]->nrec - srfP->pntf[typ]->ndel;
  }
}




/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_countPointsInLinearFeature
 DESC: Its counts the number of points that make up a breakline, interior
       boundary, or exterior boundary.
 HIST: Original - tmi 26-Apr-1990
 MISC:
 KEYW: DTM COUNT POINTS BREAKLINE INTERIOR EXTERIOR
-----------------------------------------------------------------------------%*/

void aecDTM_countPointsInLinearFeature
(
  long *npntP,                         /* <= # points in feature               */
  struct CIVdtmsrf *srfP,              /* => surface containing feature        */
  struct CIVdtmblk *inpblkP,           /* => block cont. ftr. (or NULL)        */
  struct CIVdtmpnt *pP                 /* => first point in linear ftr.        */
)
{
  if ( npntP != (long *)0 )
  {
    struct CIVdtmblk *blkP = (struct CIVdtmblk *)0;

    *npntP = 0L;

    if ( inpblkP != (struct CIVdtmblk *)0 )
      blkP = inpblkP;
    else
      aecDTM_findPointType ( (long *)0, (long *)0, &blkP, srfP, pP );

    if ( blkP != (struct CIVdtmblk *)0 )
      if ( pP >= blkP->rec.pnt  &&  pP < blkP->rec.pnt + blkP->use )
        for ( *npntP = 1L, pP++; pP < blkP->rec.pnt + blkP->use  &&  pP->flg & DTM_C_PNTPUD; pP++, (*npntP)++ );
  }
}


/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_countBoundaries
 DESC: It returns number of interior and/or exterior boundaries.
 HIST: Original - tmi 26-May-1994
 MISC:
 KEYW: DTM COUNT BOUNDARIES
-----------------------------------------------------------------------------%*/

void aecDTM_countBoundaries
(
  long *numIntP,                       /* <= # of interior boundaries         */
  long *numExtP,                       /* <= # of exterior boundaries         */
  struct CIVdtmsrf *srfP               /* => surface to use                   */
)
{
  if ( srfP != (struct CIVdtmsrf *)0 )
  {
    if ( numIntP != (long *)0 )
    {
      *numIntP = 0L;
      aecDTM_sendAllPoints ( (void *)0, srfP, DTM_C_NOBREK, DTM_C_INTMSK, aecDTM_countBoundariesProcess, (void *)numIntP );
    }

    if ( numExtP != (long *)0 )
    {
      aecDTM_countSurfaceDataInFile ( numExtP, srfP, DTM_C_DTMEXT );
      if ( *numExtP > 0L )
        *numExtP = 1L;
    }
  }
}



/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_countBoundariesProcess
 DESC: It receives each interior boundary and increments counter.
 HIST: Original - tmi 26-May-1994
 MISC: static
 KEYW: DTM COUNT BOUNDARIES PROCESS
-----------------------------------------------------------------------------%*/

static int aecDTM_countBoundariesProcess
(
  void *numIntP,
  int,
  long,
  DPoint3d *,
  struct CIVdtmpnt *
)
{
  (*(long *)numIntP)++;

  return ( SUCCESS );
}




/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_countSurfaceFeatures
 DESC: Counts the total number of features in a surface.
 HIST: Original - twl 28-Oct-1998
 MISC:
 KEYW: DTM COUNT FEATURES
-----------------------------------------------------------------------------%*/

void aecDTM_countSurfaceFeatures
(
  long *nftrP,                         /* <= # features in surface             */
  struct CIVdtmsrf *srfP               /* => surface to use                    */
)
{
  int i;

  if ( nftrP != (long *)0 )
  {
    *nftrP = 0L;

    if ( srfP->version > 5 )
    {
      if ( srfP != (struct CIVdtmsrf *)0 )
      {
        for ( *nftrP = 0, i = 0; i < DTM_C_NMFTRF; i++ )
        {
          if ( srfP->ftrf[i] != (struct CIVdtmfil *)0 )
          {
            struct CIVdtmblk *blkP;
            struct CIVdtmftr *ftrP;

            for ( srfP->ftrf[i]->nrec = srfP->ftrf[i]->ndel = 0, blkP = srfP->ftrf[i]->blk; blkP; blkP = blkP->nxt )
              for ( srfP->ftrf[i]->nrec += blkP->use, ftrP = blkP->rec.ftr; ftrP < blkP->rec.ftr + blkP->use; ftrP++ )
                if ( aecDTM_isFeatureDeletedFlagSet ( ftrP ) )
                  srfP->ftrf[i]->ndel++;

            *nftrP += srfP->ftrf[i]->nrec;
          }
        }
      }
    }
  }
}



