//---------------------------------------------------------------------------+
// $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+
/*----------------------------------------------------------------------------*/
/* dtmio.h                                            aec    07-Feb-1994      */
/*----------------------------------------------------------------------------*/
/* Data structures and constants used by DTM load/save & import/export        */
/* functions and commands.                                                    */
/*----------------------------------------------------------------------------*/

#pragma once

/*----------------------------------------------------------------------------*/
/* Dependent include files                                                    */
/*----------------------------------------------------------------------------*/

#include <dtmstr.h>

/*----------------------------------------------------------------------------*/
/* Load DTM file things                                                       */
/*----------------------------------------------------------------------------*/

#define LODDTM_LOD    0x1              /* force load                          */
#define LODDTM_TTN    0x10             /* use TTN filter                      */

/*----------------------------------------------------------------------------*/
/* Load ASCII data things                                                     */
/*----------------------------------------------------------------------------*/

#define LODDAT_SOE      0x1            /* load as soe                         */
#define LODDAT_NEE      0x2            /* load as nee                         */
#define LODDAT_ENE      0x4            /* load as ene                         */
#define LODDAT_IDSOE    0x8            /* load as i.d. soe                    */
#define LODDAT_IDNEE   0x10            /* load as i.d. nee                    */
#define LODDAT_IDENE   0x20            /* load as i.d. ene                    */
#define LODDAT_FMTMSK  0x3F            /* format mask                         */
#define LODDAT_INGR    0x40            /* load ingr data                      */

/*----------------------------------------------------------------------------*/
/* Load DEM data things                                                       */
/*----------------------------------------------------------------------------*/

#define LODDEM_CNV    0x1              /* convert data units                  */

/*----------------------------------------------------------------------------*/
/* Load .TTN file things                                                      */
/*----------------------------------------------------------------------------*/

#ifndef     TTN_OLD_VERSION
#    define TTN_OLD_VERSION 1
#endif

#ifndef     XYZ_VERSION
#    define XYZ_VERSION 4
#endif

/*----------------------------------------------------------------------------*/
/* Import graphic element things                                              */
/*----------------------------------------------------------------------------*/

#define LODELY_INS                0x1  /* elements inside fence               */
#define LODELY_OUT                0x2  /* elements outside fence              */
#define LODELY_SNG                0x4  /* single element mode                 */
#define LODELY_LVL                0x8  /* load elements by level              */
#define LODELY_FEN               0x10  /* load elements within fence          */
#define LODELY_MODMSK            0x1C  /* mode mask                           */
#define LODELY_CVTTXT            0x20  /* interpret text as elevation         */
#define LODELY_BNDELV            0x40  /* don't use input elevations          */
#define LODELY_MAX               0x80  /* use maximum segment length          */
#define LODELY_CRVVRT           0x100  /* only use curvestring verts.         */
#define LODELY_DRAPE            0x200  /* drape element on surface            */
#define LODELY_DRAPELIN         0x400  /* drape element on surface - linear point type */
#define LODELY_THN             0x2000  /* thin linear elements                */
#define LODELY_EXCLUSIVE       0x4000  /* exclusive point types               */
#define LODELY_PNTDENSITY      0x8000  /* use feature point density interval  */
#define LODELY_DNT            0x10000  /* exclude features from triangulation */
#define LODELY_FTRAPPEND      0x20000  /* append new features                 */
#define LODELY_FTRREPLACE     0x40000  /* rename new features                 */
#define LODELY_FTRUSETAGS     0x80000  /* use tag for name, style, etc.       */
#define LODELY_DRAPEVRTSONLY 0x100000  /* use tag for name, style, etc.       */

/*----------------------------------------------------------------------------*/
/* Save DTM file things                                                       */
/*----------------------------------------------------------------------------*/

#define SAVDTM_TTN    0x1              /* 1: save as ttn                      */

#define SAVDAT_INS       0x1           /* inside fence                        */
#define SAVDAT_OUT       0x2           /* outside fence                       */
#define SAVDAT_NEE       0x1           /* save as nee                         */
#define SAVDAT_ENE       0x2           /* save as ene                         */
#define SAVDAT_IDNEE     0x4           /* save as i.d. nee                    */
#define SAVDAT_IDENE     0x8           /* save as i.d. ene                    */
#define SAVDAT_FMTMSK    0xF           /* format mask                         */
#define SAVDAT_INGR     0x40           /* ingr compatible                     */

