//---------------------------------------------------------------------------+
// $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+
/*----------------------------------------------------------------------------*/
/* addtin.c                                        tmi    17-Jan-1994         */
/*----------------------------------------------------------------------------*/
/* Various utilities to add triangles to a surface.                           */
/*----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*/
/* Include files                                                              */
/*----------------------------------------------------------------------------*/
#include "stdafx.h"

extern boolean tinAltMoveMethod;


/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_addTriangle
 DESC: Adds a single new triangle to the internal triangle memory blocks.
 HIST: Original - tmi 10-Apr-1990
 MISC:
 KEYW: DTM TRIANGLE ADD
-----------------------------------------------------------------------------%*/

int aecDTM_addTriangle    /* <= TRUE if error                      */
(
  struct CIVdtmtin **tinPP,          /* <= ptr pointing to new triangle       */
  struct CIVdtmsrf *srfP,            /* => surface where tin is added         */
  struct CIVdtmpnt *p1P,             /* => ptr to first vertice point.        */
  struct CIVdtmpnt *p2P,             /* => ptr to second vertice point.       */
  struct CIVdtmpnt *p3P,             /* => ptr to third vertice point.        */
  struct CIVdtmtin *n12P,            /* => ptr to tri neigh. side 1-2         */
  struct CIVdtmtin *n23P,            /* => ptr to tri neigh. side 2-3         */
  struct CIVdtmtin *n31P             /* => ptr to tri neigh. side 3-1         */
)
{
  DPoint3d p1, p2, p3;
  DPoint3d *moveOriginP = NULL;
  struct CIVdtmpnt *altTinPntsP = NULL;
  struct CIVdtmpnt *tPnt1 = NULL;
  struct CIVdtmpnt *tPnt2 = NULL;
  struct CIVdtmpnt *tPnt3 = NULL;
  struct CIVdtmblk *blkP = NULL;
  int sts = SUCCESS;

  if ( tinAltMoveMethod )
  {
    altTinPntsP = (struct CIVdtmpnt *)alloca ( sizeof ( struct CIVdtmpnt ) * 3 );
    moveOriginP = (DPoint3d *)alloca ( sizeof ( DPoint3d ) );

    memcpy ( &altTinPntsP[0], p1P, sizeof ( struct CIVdtmpnt ) );
    memcpy ( &altTinPntsP[1], p2P, sizeof ( struct CIVdtmpnt ) );
    memcpy ( &altTinPntsP[2], p3P, sizeof ( struct CIVdtmpnt ) );
    memcpy ( moveOriginP, &p1P->cor, sizeof ( DPoint3d ) );
            
    VSUB ( altTinPntsP[0].cor, *moveOriginP, altTinPntsP[0].cor );
    VSUB ( altTinPntsP[1].cor, *moveOriginP, altTinPntsP[1].cor );
    VSUB ( altTinPntsP[2].cor, *moveOriginP, altTinPntsP[2].cor );

    tPnt1 = &altTinPntsP[0];
    tPnt2 = &altTinPntsP[1];
    tPnt3 = &altTinPntsP[2];
  }
  else
  {
    tPnt1 = p1P;
    tPnt2 = p2P;
    tPnt3 = p3P;
  }

  *tinPP = NULL;

  VSUBXY( tPnt1->cor, tPnt2->cor, p1 );
  VSUBXY( tPnt3->cor, tPnt2->cor, p2 );
  VCROSSXY( p1, p2, p3 );

  if( p3.z > 0. )
  {
    if ( srfP->ntinstk > 0 )
      sts = aecDTM_triangleStackGet ( tinPP, srfP );
    else if ( ( sts = aecDTM_allocateBlock ( &blkP, srfP->tinf, (long)DTM_C_BLKSIZTIN, 0 ) ) == SUCCESS )
    {
      srfP->tinf->nrec++;
      blkP->use++;
      *tinPP = blkP->rec.tin + blkP->use - 1;
    }

    if( sts == SUCCESS )
    {
      aecDTM_setSurfaceModifiedFlag ( srfP );

      (*tinPP)->flg = 0;
      (*tinPP)->p1  = p1P;
      (*tinPP)->p2  = p2P;
      (*tinPP)->p3  = p3P;
      (*tinPP)->n12 = n12P;
      (*tinPP)->n23 = n23P;
      (*tinPP)->n31 = n31P;
    }
  }
  else
  {
    aecDTM_triangulateSetErrorPoint ( p3P );
    sts = DTM_M_TOLPRB;
  }

  return ( sts );
}
