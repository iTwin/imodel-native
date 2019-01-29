//---------------------------------------------------------------------------+
// $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+
/*----------------------------------------------------------------------------*/
/* allpnt.c                                        tmi    23-Apr-1990         */
/*----------------------------------------------------------------------------*/
/* Utilities to send all (or some) points to a user-specified function.       */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* Include files                                                              */
/*----------------------------------------------------------------------------*/
#include "stdafx.h"




/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_sendAllPoints
 DESC: When it is necessary to loop through all points in a network,
       function allows you to do that.  You have the option to do all
       all points (deleted and nondeleted), or just the nondeleted
       points.  Additionally, you can pass information directly through
       to your routine by passing a pointer to a structure containing your
       information as the 'dat' argument. The format of the function that
       is called is:
              int usrfnc
              (
                void *datP,
                int typ,
                long np,
                DPoint3d *corP,
                struct CIVdtmpnt *pntP
              )
       You can also control which point types are sent to your function.
       Do this using the 'typmsk' argument.  OR the values defined in
       civdtm.h for the types you want.  If 'typmsk' is zero, all types
       will be processed.
 HIST: Original - tmi 17-Jan-1994
 MISC:
 KEYW: DTM POINT SEND ALL
-----------------------------------------------------------------------------%*/

int aecDTM_sendAllPoints    /* <= TRUE if error                    */
(
  void *mdlDescP,                      /* => mdl app desc. (or NULL)          */
  struct CIVdtmsrf *srfP,              /* => surface to use                   */
  int opt,                             /* => options                          */
  int typmsk,                          /* => point type (zero for all)        */
  int (*usrfncP)(void *,int,long,      /* => your function                    */
       DPoint3d *,struct CIVdtmpnt *),
  void *datP                           /* => your data                        */
)
{
  struct CIVdtmblk *blkP;
  struct CIVdtmpnt *pP, *begpP;
  DPoint3d *pntP;
  int sts = SUCCESS, i, alcsiz = 100, elvMask = opt & (DTM_C_PNTWEL | DTM_C_PNTWOE);
  long j, cnt = 1L, siz;

  pntP = (DPoint3d *) malloc ( (unsigned int)alcsiz * sizeof(DPoint3d) );
  aecInterrupt_initialize();

  for ( i = 0; i < DTM_C_NMPNTF  &&  sts == SUCCESS; i++ )
    if ( !typmsk  ||  ( 1 << i & typmsk ) )
      for ( blkP = srfP->pntf[i]->blk; blkP  &&  sts == SUCCESS; blkP = blkP->nxt )
	for ( pP = blkP->rec.pnt; pP < blkP->rec.pnt + blkP->use  &&  sts == SUCCESS; pP++ )
	  if ( !(opt & DTM_C_NOBREK)  &&  aecInterrupt_check() )
            sts = DTM_M_PRCINT;
          else if ( opt & DTM_C_DELETE  ||  !aecDTM_isPointDeletedFlagSet(pP) )
	  {
            begpP = pP;
            if ( !(opt & DTM_C_NODFYS) || !(pP->flg & DTM_C_PNTDFY) )
	            DTMPOINTTODPOINT ( srfP, pP, pntP[0] );

            if ( i != DTM_C_DTMREG  &&  !(opt & DTM_C_SNDONE) )
            {
              if ( !elvMask  ||  (elvMask == DTM_C_PNTWEL  &&  !(pP->flg & DTM_C_PNTELV) )  ||  (elvMask == DTM_C_PNTWOE  &&  pP->flg & DTM_C_PNTELV) )
              {
                aecDTM_countPointsInLinearFeature ( &siz, srfP, blkP, pP );
                if ( siz > alcsiz )
                  alcsiz = (int)siz, pntP = (DPoint3d *) realloc ( (void *)pntP, (unsigned int)alcsiz * sizeof(DPoint3d) );

                for ( cnt = 1L, pP++, j = 1; j < siz; j++, pP++ )
                  if ( opt & DTM_C_DELETE || !aecDTM_isPointDeletedFlagSet(pP) )
                    if ( !(opt & DTM_C_NOCONS) || (opt & DTM_C_NOCONS && !aecDTM_isPointConstructionFlagSet(pP) ) )
                      if ( !(opt & DTM_C_NODFYS) || !(pP->flg & DTM_C_PNTDFY) )
                        if ( !(opt & DTM_C_NOTINS) || !(pP->flg & DTM_C_PNTTIN) )
                        {
                          DTMPOINTTODPOINT ( srfP, pP, pntP[cnt] );
                          cnt++;
                        }
                pP--;

                if ( cnt == 1 && (opt & DTM_C_NOTINS) && (begpP->flg & DTM_C_PNTTIN ) )
                    continue;
              }
              else
                continue;
            }
            else if ( i == DTM_C_DTMREG && 
                      ( (opt & DTM_C_NODFYS) && (pP->flg & DTM_C_PNTDFY) ) ||
                      ( (opt & DTM_C_NOTINS) && (pP->flg & DTM_C_PNTTIN) ) )
            {
              continue;
            }

            sts = (*usrfncP)( datP, i, cnt, pntP, begpP );
	  }

  aecInterrupt_stop();
  free ( (void *)pntP );

  return ( sts );
}

