//---------------------------------------------------------------------------+
// $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+
/*----------------------------------------------------------------------------*/
/* dtmtrifnc.h                                           aec    08-Feb-1994   */
/*----------------------------------------------------------------------------*/
/* Function prototypes for functions used during triangularization process.   */
/*----------------------------------------------------------------------------*/

#pragma once

/*----------------------------------------------------------------------------*/
/* Dependent include files                                                    */
/*----------------------------------------------------------------------------*/

#include <dtmstr.h>
#include <dtmtri.h>

/*----------------------------------------------------------------------------*/
/* Function prototypes                                                        */
/*----------------------------------------------------------------------------*/

void aecDTM_triangulateSetErrorPoint
(
    struct CIVdtmpnt *dtmPntP
);

int aecDTM_validateTinPtr   /* <= TRUE if error                    */
(
    struct CIVdtmblk **inpblkP,          /* <= block where triangle is        */
    struct CIVdtmsrf *srfP,              /* => surface to use                 */
    struct CIVdtmtin *tinP               /* => triangle to use                */
);
