//---------------------------------------------------------------------------+
// $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+

#pragma once

/*----------------------------------------------------------------------------*/
/* Data structures                                                            */
/*----------------------------------------------------------------------------*/

#define DTM_C_FTRGAP             0x1   /* Discontinuity in feature            */

typedef struct                         /* dtm DPoint3d + flag structure       */
{
    DPoint3d cor;                      /* point coordinates                   */
    unsigned char flg;                       /* bit field flags                     */
} DTMDPoint;
