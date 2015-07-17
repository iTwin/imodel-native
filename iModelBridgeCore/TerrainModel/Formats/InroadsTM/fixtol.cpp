//---------------------------------------------------------------------------+
// $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+
/*----------------------------------------------------------------------------*/
/* Include files                                                              */
/*----------------------------------------------------------------------------*/
#include "stdafx.h"

/*----------------------------------------------------------------------------*/
/* Private static function prototypes                                         */
/*----------------------------------------------------------------------------*/
static int aecDTM_fixToleranceProcess(void *,int,long,DPoint3d *,struct CIVdtmpnt *);
static void aecDTM_doLongWay(double *);


int aecDTM_fixTolerance  /* TRUE if error                         */
(
    CIVdtmsrf *pSrf                   /* => surface to fix                     */
)
{
    int nStatus = SUCCESS;

    aecTicker_initialize();

    nStatus = aecDTM_sendAllPoints( NULL, pSrf, 0, 0, aecDTM_fixToleranceProcess, NULL );

    aecTicker_stop();

    return nStatus;
}


static int aecDTM_fixToleranceProcess
(
    void *,
    int,
    long np,
    DPoint3d *,
    struct CIVdtmpnt *pnt
)
{
    int i;

    aecTicker_show ();

    for ( i = 0; i < np; i++ )
    {
      aecDTM_doLongWay ( &pnt[i].cor.x );
      aecDTM_doLongWay ( &pnt[i].cor.y );
      aecDTM_doLongWay ( &pnt[i].cor.z );
    }

    return ( SUCCESS );
}


static void aecDTM_doLongWay
(
    double *pnt      /* <=> */
)
{
    double dTemp;
    
    dTemp = *pnt * 1000.0;

    if ( dTemp > 0 )
      dTemp = floor ( dTemp + 0.5 );
    else
      dTemp = ceil ( dTemp - 0.5 );

    *pnt = dTemp / 1000.0;
}     