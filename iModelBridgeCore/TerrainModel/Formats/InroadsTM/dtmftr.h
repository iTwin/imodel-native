//---------------------------------------------------------------------------+
// $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+
/*----------------------------------------------------------------------------*/
/* dtmftr.h                                            aec    08-Feb-1994     */
/*----------------------------------------------------------------------------*/
/* Data structures and constants for pad, longitudinal, and transverse        */
/* feature functions.                                                         */
/*----------------------------------------------------------------------------*/

#pragma once

/*----------------------------------------------------------------------------*/
/* Dependent include files                                                    */
/*----------------------------------------------------------------------------*/

#include <disfnc.h>
#include <dtmstr.h>

/*----------------------------------------------------------------------------*/
/* Longitudinal feature                                                       */
/*----------------------------------------------------------------------------*/

#define LNGFTR_INTERVAL            0x001
#define LNGFTR_STROKE              0x002
#define LNGFTR_LOOP                0x004
#define LNGFTR_ATTACH_TAG          0x008
#define LNGFTR_THIN				   0x010
#define LNGFTR_DELETE_PRIMARY      0x10

#define LNGFTR_OFFSET_OFFSET       0x0001
#define LNGFTR_OFFSET_SLOPE        0x0002
#define LNGFTR_SLOPE_OFFSET        0x0004
#define LNGFTR_ELEV_ELEV           0x0008
#define LNGFTR_ELEV_SLOPE          0x0010
#define LNGFTR_SLOPE_ELEV          0x0020
#define LNGFTR_DRAPE               0x0040
#define LNGFTR_LATERAL_SLOPE       0x0080
#define LNGFTR_PROJECT_SLOPE       0x0100
#define LNGFTR_PROJECT_TWO_SLOPES  0x0200
#define LNGFTR_PROJECT_SLOPE_SRF   0x0400
#define LNGFTR_ALL_VERT_METHODS    0x07FF

#define LNGFTR_OFFSET_FROM_PRIMARY  0x0800
#define LNGFTR_ALONG_SELECTED       0x1000
#define LNGFTR_PROJECT_TWO_SLOPESHZ 0x2000
#define LNGFTR_PROJECT_SLOPE_SRFHZ  0x4000
#define LNGFTR_ALL_HORZ_METHODS     0x7800

#define LNGFTR_FEATURE                1
#define LNGFTR_BEGIN                  2
#define LNGFTR_END                    3
#define LNGFTR_DONE                   4

/*----------------------------------------------------------------------------*/
/* Transverse feature                                                         */
/*----------------------------------------------------------------------------*/

#define TRNFTR_INTERVAL       0x01     /* values for opt                      */
#define TRNFTR_STROKE         0x02
#define TRNFTR_DENSIFY        0x04
#define TRNFTR_SLOPE          0x08
#define TRNFTR_ATTACH_TAG     0x10

#define TRNFTR_CELL           0x01
#define TRNFTR_SYMBOL         0x02
#define TRNFTR_TICK           0x04
                                       /* values for step                     */
#define TRNFTR_PRIMARY           1
#define TRNFTR_SECONDARY         2
#define TRNFTR_REFERENCE         3
#define TRNFTR_BEGIN             4
#define TRNFTR_END               5
#define TRNFTR_DONE              6
#define TRNFTR_MAXSTEP           5

/*----------------------------------------------------------------------------*/
/* Slope feature                                                              */
/*----------------------------------------------------------------------------*/

#define SLPFTR_BRMINT  0x0001
#define SLPFTR_DENTRN  0x0002
#define SLPFTR_INDSLP  0x0004
#define SLPFTR_MAXDST  0x0008
#define SLPFTR_ADDSRF  0x0010
#define SLPFTR_ATTTAG  0x0020
#define SLPFTR_ELVMET  0x0040
