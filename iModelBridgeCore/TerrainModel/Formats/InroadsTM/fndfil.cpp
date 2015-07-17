//---------------------------------------------------------------------------+
// $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+
/*----------------------------------------------------------------------------*/
/* fndfil.c                                        tmi    31-Jan-1991         */
/*----------------------------------------------------------------------------*/
/* Various utilities to find files.                                           */
/*----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*/
/* Include files                                                              */
/*----------------------------------------------------------------------------*/
#include "stdafx.h"





/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_findPointFile
 DESC: Given a surface and a point within that surface, this function finds
       the file that contains the point.
 HIST: Original - tmi 31-Jan-1991
 MISC:
 KEYW: DTM FIND FILE POINT
-----------------------------------------------------------------------------%*/

int aecDTM_findPointFile /* <= TRUE if error                       */
(
  struct CIVdtmfil **filPP,         /* <= found file                          */
  struct CIVdtmsrf *srfP,           /* => surface to look in                  */
  struct CIVdtmpnt *pntP            /* => point within file                   */
)
{
  int sts = SUCCESS;

  if ( filPP != (struct CIVdtmfil **)0 )
  {
    long typ;

    *filPP = (struct CIVdtmfil *)0;

    if ( ( sts = aecDTM_findPointType ( &typ, (long *)0, (struct CIVdtmblk **)0, srfP, pntP ) ) == SUCCESS )
      *filPP = srfP->pntf[typ];
  }

  return ( sts );
}


