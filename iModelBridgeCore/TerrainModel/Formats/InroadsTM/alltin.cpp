//---------------------------------------------------------------------------+
// $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+
/*----------------------------------------------------------------------------*/
/* alltin.c                                    tmi        23-Apr-1990         */
/*----------------------------------------------------------------------------*/
/* Utilities to send all (or some) triangles to a user-specified              */
/* function.                                                                  */
/*----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*/
/* Include files                                                              */
/*----------------------------------------------------------------------------*/
#include "stdafx.h"


/*----------------------------------------------------------------------------*/
/* Local constants                                                            */
/*----------------------------------------------------------------------------*/
#define AEC_MAXTHREADS         4       /* assume no more that 4 processors    */


/*----------------------------------------------------------------------------*/
/* Local data structures                                                      */
/*----------------------------------------------------------------------------*/
typedef struct tagDTMsendAllTinData
{
  void *mdlDescP;
  struct CIVdtmsrf *srfP;
  int opt;
  int (*usrfncP)(void *,long, DPoint3d *,struct CIVdtmtin *,unsigned long);
  void *datP;

  long  numThreads;
  long threadID;
} DTMsendAllTinData;



/*----------------------------------------------------------------------------*/
/* Local static functions                                                     */
/*----------------------------------------------------------------------------*/
static int _aecDTM_sendAllTrianglesSingleThread(void *,struct CIVdtmsrf *,int,int (*)(void *,long,DPoint3d *,struct CIVdtmtin *,unsigned long),void *,long,long);
static int _aecDTM_sendAllTrianglesMultipleThreads(void *,struct CIVdtmsrf *,int,int (*)(void *,long,DPoint3d *,struct CIVdtmtin *,unsigned long),void *);
//DHTODOstatic unsigned long __stdcall _aecDTM_sendAllTrianglesMultipleThreadsPrc(void *);
static void _aecDTM_sendAllTrianglesLimits(struct CIVdtmtin **,struct CIVdtmtin **,struct CIVdtmblk *,long,long);





/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_sendAllTriagles
 DESC: When it is necessary to loop through all triangles in a network,
       function allows you to do that.  You have the option to do all
       triangles -- deleted and nondeleted, or just the nondeleted
       triangles.  Additionally, you can pass information directly through
       to your routine by passing a pointer to a structure containing your
       information as the 'dat' argument. The format of the function that
       is called is:

          int usrfnc
          (
            void *datP,
            long np,
            DPoint3d *pP,
            struct CIVdtmtin *tP,
            unsigned long threadID
          )
 HIST: Original - tmi 17-Jan-1994
 MISC:
 KEYW: DTM TRIANGLE
-----------------------------------------------------------------------------%*/

int aecDTM_sendAllTriangles /* <= TRUE if error                    */
(
  void *mdlDescP,                      /* => mdl app desc. (or NULL)          */
  struct CIVdtmsrf *srfP,              /* => surface with triangles           */
  int opt,                             /* => options                          */
  int (*usrfncP)(void *,long,          /* => your function                    */
      DPoint3d *,struct CIVdtmtin *,unsigned long),
  void *datP                           /* => your data                        */
)
{
  int sts = SUCCESS;

  aecInterrupt_initialize();
  aecTicker_initialize();

  sts = _aecDTM_sendAllTrianglesSingleThread ( mdlDescP, srfP, opt, usrfncP, datP, 0, 0 );

  aecInterrupt_stop();
  aecTicker_stop();

  return ( sts );
}



/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_sendAllTriaglesSingleThread
 DESC: Called when only single thread is available on machine.
 HIST: Original - tmi 17-Jan-1994
 MISC: static
 KEYW: DTM TRIANGLE THREAD SINGLE
-----------------------------------------------------------------------------%*/

static int _aecDTM_sendAllTrianglesSingleThread
(
  void *mdlDescP,
  struct CIVdtmsrf *srfP,
  int opt,
  int (*usrfncP)(void *,long, DPoint3d *,struct CIVdtmtin *,unsigned long),
  void *datP,
  long numThreads,
  long threadID
)
{
  struct CIVdtmblk *blkP;
  struct CIVdtmtin *tP, *startTinP, *stopTinP;
  DPoint3d pP[4];
  int sts = SUCCESS;

  for ( blkP = srfP->tinf->blk; blkP  &&  sts == SUCCESS; blkP = blkP->nxt )
  {
    _aecDTM_sendAllTrianglesLimits ( &startTinP, &stopTinP, blkP, numThreads, threadID );

    for ( tP = startTinP; tP < stopTinP  &&  sts == SUCCESS; tP++ )
      if ( !(opt & DTM_C_NOBREK)  &&  aecInterrupt_check() )
        sts = DTM_M_PRCINT;
      else if ( opt & DTM_C_DELETE  ||  !aecDTM_isTriangleDeletedFlagSet ( tP ) )
      {
        DTMTINTODPOINT ( srfP, tP, pP );  pP[3] = pP[0];
		aecTicker_show();
        sts = (*usrfncP)( datP, 4L, pP, tP, threadID );
      }
  }

  return ( sts );
}

  //DHTODO
//static unsigned long __stdcall _aecDTM_sendAllTrianglesMultipleThreadsPrc
//(
//  void *inputDatP
//)
//{
//  DTMsendAllTinData *datP = (DTMsendAllTinData *)inputDatP;
//
//  return ( _aecDTM_sendAllTrianglesSingleThread ( datP->mdlDescP, datP->srfP, datP->opt, datP->usrfncP, datP->datP, datP->numThreads, datP->threadID ) );
//}




/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_sendAllTriaglesLimits
 DESC: Computes starting and stopping triangle limits for processing.
 HIST: Original - tmi 21-Aug-1996
 MISC: static
 KEYW: DTM TRIANGLE LIMITS
-----------------------------------------------------------------------------%*/

static void _aecDTM_sendAllTrianglesLimits
(
  struct CIVdtmtin **startTinPP,
  struct CIVdtmtin **stopTinPP,
  struct CIVdtmblk *blkP,
  long numThreads,
  long threadID
)
{
  long threadSize, remainder;

  if ( numThreads < 2 ) numThreads = 1;

  if ( threadID < 0  ||  numThreads == 1 ) threadID = 0;

  threadSize  = blkP->use / numThreads;
  remainder   = blkP->use % numThreads;
  *startTinPP = blkP->rec.tin + threadID * threadSize;
  *stopTinPP  = *startTinPP + threadSize;

  if ( numThreads > 1  &&  threadID == numThreads-1 )
    *stopTinPP += remainder;
}
