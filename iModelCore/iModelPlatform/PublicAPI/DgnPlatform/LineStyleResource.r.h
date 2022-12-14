/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
/** @cond BENTLEY_SDK_Internal */

//--------------------------------------------------------------------
// This file is included by both .cpp/h and .r files
//--------------------------------------------------------------------

#include    "DgnPlatform.r.h"

/*----------------------------------------------------------------------+
|                                                                       |
|   Macros                                                              |
|                                                                       |
+----------------------------------------------------------------------*/
#define LS_MAX_DESCR                   128         /* Resource description length */
#define LC_MAX_STROKES                 32          /* Max strokes in line code    */

#define LSATTR_UNITMASK                     0x00000003
#define LSATTR_UNITDEV                      0x00000002   /* Unit def in device coords    */
#define LSATTR_UNITMETERS                   0x00000003   // Line style units are meters, introduced in DgnDb
#define LSATTR_NOSNAP                       0x00000004   /* Only snap to the center line */
#define LSATTR_CONTINUOUS                   0x00000008   /* Continuous lines             */
#define LSATTR_NORANGE                      0x00000010   /* Dont use lstyle for range    */
#define LSATTR_NOWIDTH                      0x00000040   /* Lines with no width - speeds locate.  This is set on read.*/
#define LSATTR_PHYSICAL                     0x00000080   /* Line styles are physical and should be scaled as such.  This only applies to styles in dgnlibs, not resources. */

/*----------------------------------------------------------------------+
|                                                                       |
|   Line Style params structure.                                        |
|                                                                       |
|   Modifier options are applied to the "modifiers" structure member    |
|   and can be combined using the bitwise OR ("|") operator.            |
|                                                                       |
+----------------------------------------------------------------------*/
#define STYLEMOD_SCALE          0x01        /* scale present           */
#define STYLEMOD_DSCALE         0x02        /* dash scale present      */
#define STYLEMOD_GSCALE         0x04        /* gap scale present       */

#define STYLEMOD_SWIDTH         0x08        /* start width present     */
#define STYLEMOD_EWIDTH         0x10        /* end width present       */

#define STYLEMOD_DISTPHASE      0x20        /* distance phase present  */
#define STYLEMOD_FRACTPHASE     0x40        /* fraction phase present  */
#define STYLEMOD_CENTERPHASE    0x80        /* centered end stretch    */

/*-----------------------------------------------------------------------
The modifiers in the second byte apply only in 3d
-----------------------------------------------------------------------*/
#define STYLEMOD_NORMAL         0x0100      /* surface normal present  */
#define STYLEMOD_RMATRIX        0x0200      /* rotation matrix present */

/*-----------------------------------------------------------------------
Multiline modifiers
-----------------------------------------------------------------------*/
/*0x0400 was temporarily  the line style number present for multilines only.  It was never released.*/
#define STYLEMOD_LINEMASK       0x0800      /* A multiline line mask present */
#define STYLEMOD_MLINEFLAGS     0x1000      /* A multiline flag present (LINE, ORGCAP, etc.)*/

/*-----------------------------------------------------------------------
 If the TRUE_WIDTH modifier is present then the start and end width
 are interpreted as UOR widths and are not scaled.
-----------------------------------------------------------------------*/
#define STYLEMOD_TRUE_WIDTH     0x02000
/* The SEGMODE bit was added after NOSEGMODE.  If both bits are ON, NOSEGMODE
   is used.  This behavior is desirable because curves will have NOSEGMODE set,
   and it must not be overridden. */
#define STYLEMOD_SEGMODE        0x40000000  /* force single segment  */
#define STYLEMOD_NOSEGMODE      0x80000000  /* disable single segment  */

/*======================================================================+
|                                                                       |
|   Resource structures                                                 |
|                                                                       |
+======================================================================*/
BEGIN_BENTLEY_DGN_NAMESPACE

END_BENTLEY_DGN_NAMESPACE

/** @endcond */
