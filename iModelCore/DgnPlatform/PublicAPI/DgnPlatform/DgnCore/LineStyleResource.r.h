/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/LineStyleResource.r.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
/** @cond BENTLEY_SDK_Internal */

//--------------------------------------------------------------------
// This file is included by both .cpp/h and .r files
//--------------------------------------------------------------------

#include    "../DgnPlatform.r.h"

/*----------------------------------------------------------------------+
|                                                                       |
|   Macros                                                              |
|                                                                       |
+----------------------------------------------------------------------*/
#define LC_RSCSIZE(n) (sizeof(LineCodeRsc)  + ((n) * sizeof(LineCodeStroke))  - sizeof(LineCodeStroke))
#define LP_RSCSIZE(n) (sizeof(LinePointRsc) + ((n) * sizeof(PointSymInfo))    - sizeof(PointSymInfo))
#define LS_RSCSIZE(n) (sizeof(LineStyleRsc) + ((n) * sizeof(ComponentInfo))   - sizeof(ComponentInfo))

#define LS_MAX_NAME                    64          /* Maximum name length         */
#define LS_MAX_DESCR                   128         /* Resource description length */
#define LC_MAX_STROKES                 32          /* Max strokes in line code    */

#define LSATTR_UNITMASK                     0x00000003
#define LSATTR_UNITMASTER                   0x00000000   /* Units = master units         */
#define LSATTR_UNITUOR                      0x00000001   /* Unit def in world coords     */
#define LSATTR_UNITDEV                      0x00000002   /* Unit def in device coords    */
#define LSATTR_NOSNAP                       0x00000004   /* Only snap to the center line */
#define LSATTR_CONTINUOUS                   0x00000008   /* Continuous lines             */
#define LSATTR_NORANGE                      0x00000010   /* Dont use lstyle for range    */
#define LSATTR_SHAREDCELL_SCALE_INDEPENDENT 0x00000020   /* Make the line scale independent of scale of shared cells */
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
BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE
struct LineStyleParamsResource
    {
    uint32_t    modifiers;      /* see STYLEMOD_... above              */
    uint32_t    reserved;
    double      scale;          /* Applied to all length values        */
    double      dashScale;      /* Applied to adjustable dash strokes  */
    double      gapScale;       /* Applied to adjustable gap strokes   */
    double      startWidth;     /* Taper start width                   */
    double      endWidth;       /* Taper end width                     */
    double      distPhase;      /* Phase shift by distance             */
    double      fractPhase;     /* Phase shift by fraction             */
    uint32_t    lineMask;       /* Multiline line mask                 */
    uint32_t    mlineFlags;     /* Multiline flags                     */
    DPoint3d    normal;
    RotMatrix   rMatrix;
    };

/*----------------------------------------------------------------------+
|                                                                       |
|   Line style name map resource structures.                            |
|                                                                       |
|   The line style name map resources use the same structure as         |
|   StringLists but they are always stored in packed little-endian      |
|   format.                                                             |
|                                                                       |
+----------------------------------------------------------------------*/
#ifdef resource

typedef StringList LineStyleNames;

resourceclass LineStyleNames        RTYPE_LineStyleNames;

#endif

/*------------------------
|                                                                       |
|   Line Style Resource                                                 |
|   RTYPE_LineStyle                                                     |
|                                                                       |
|   This resource definition is used for the "Compound" line style      |
|   component. It can be defined in a resource text file using the      |
|   type RTYPE_LineStyle.                                               |
|                                                                       |
|   For the first release, only the following fields are used.          |
|                                                                       |
|       descr                                                           |
|       nComp                                                           |
|       component[n].type                                               |
|       component[n].id                                                 |
|       component[n].offset                                             |
|                                                                       |
|   Make sure all other fields are set to zero to insure compatibility  |
|   with future versions                                                |
|                                                                       |
+----------------------------------------------------------------------*/
typedef struct
    {
    uint32_t    type;
    uint32_t    id;
    uint32_t    __lReserved1, __lReserved2, __lReserved3, __lReserved4;
    double      offset;
    double      __dReserved1;
    double      __dReserved2;
    double      __dReserved3;
    } ComponentInfo;

typedef struct LineStyleRsc
    {
    char            descr[LS_MAX_DESCR];               // WIP_CHAR_OK - in resource
    uint32_t        __lHdr1;
    uint32_t        __lHdr2;
    uint32_t        auxType;
    uint32_t        __auxID;
    uint32_t        __lRes1;
    uint32_t        __lRes2;
    double          __dReserved1;
    double          __dReserved2;
    uint32_t        __lRes3;
#ifdef resource
    ComponentInfo   component[];
#else
    uint32_t        nComp;
    ComponentInfo   component[1];
#endif
    } LineStyleRsc;

#ifdef resource
resourceclass LineStyleRsc      RTYPE_LineStyle;
#endif

/*----------------------------------------------------------------------+
|                                                                       |
|   Line Code Resource                                                  |
|   RTYPE_LineCode                                                      |
|                                                                       |
|   This resource defines the line code component of the line style.    |
|   In the user documentation, this is referred to as the "stroke       |
|   pattern" component.                                                 |
|                                                                       |
|   For the first release, only the following fields are used.          |
|                                                                       |
|       descr                                                           |
|       phase                                                           |
|       options                                                         |
|       maxIterate                                                      |
|       nStrokes                                                        |
|       stroke[n].length                                                |
|       stroke[n].width                                                 |
|       stroke[n].endWidth                                              |
|       stroke[n].strokeMode                                            |
|       stroke[n].widthMode                                             |
|                                                                       |
|   Make sure all other fields are set to zero to insure compatibility  |
|   with future versions                                                |
|                                                                       |
+----------------------------------------------------------------------*/
typedef struct
    {
    double      length;         /* Stroke length                       */
    double      width;          /* Stroke width (or start width)       */
    double      endWidth;       /* End width of tapered stroke         */
    double      dReserved;
    long        lReserved1;
    long        lReserved2;
    long        lReserved3;

    uint8_t     strokeMode;     /* bit 0: dash          | gap    dash  */
                                /* bit 1: trace mode    | linear ray   */
                                /* bit 2: scale mode    | off    on    */
                                /* bit 3: start invert                 */
                                /* bit 4: end invert                   */

    uint8_t     widthMode;      /* bit 0: left half     | off    on    */
                                /* bit 1: right half    | off    on    */
                                /* bit 2: in taper      | no     yes   */
                                /* bit 3: end taper     | no     yes   */
    uint8_t     capMode;
    uint8_t     bReserved;
    } StrokeData, LineCodeStroke;

typedef struct
    {
    char          descr[LS_MAX_DESCR];                      // WIP_CHAR_OK - in resource
    uint32_t      _lHdr1, _lHdr2, _auxType, _auxID;
    double        phase;
    double        orgAngle_unused;
    double        endAngle_unused;
    double        _dReserved1, _dReserved2, _dReserved3;
    uint32_t      options;
    uint32_t      maxIterate;
    uint32_t      _lReserved;
#ifdef resource
    StrokeData    stroke[];
#else
    uint32_t      nStrokes;
    StrokeData    stroke[1];
#endif
    } LineCodeRsc;

#ifdef resource
resourceclass LineCodeRsc       RTYPE_LineCode;
#endif

/*----------------------------------------------------------------------+
|                                                                       |
|   Line Point Resource                                                 |
|   RTYPE_LinePoint                                                     |
|                                                                       |
|   For the first release, only the following fields are used.          |
|                                                                       |
|       descr                                                           |
|       lcType                                                          |
|       lcID                                                            |
|       maxIterate                                                      |
|       nStrokes                                                        |
|       symbol[n].symType                                               |
|       symbol[n].symID                                                 |
|       symbol[n].strokeNo                                              |
|       symbol[n].mod1                                                  |
|                                                                       |
|   Make sure all other fields are set to zero to insure compatibility  |
|   with future versions                                                |
|                                                                       |
|   mod1 field is defined as follows                                    |
|                                                                       |
|   Bits 0-1                                                            |
|       0 - Ignore strokeNo - no point at stroke                        |
|       1 - Point at origin of strokeNo                                 |
|       2 - Point at end of strokeNo                                    |
|       3 - Point at center of strokeNo                                 |
|                                                                       |
|   Bit 2                                                               |
|       Point at line origin if set                                     |
|                                                                       |
|   Bit 3                                                               |
|       Point at line end if set                                        |
|                                                                       |
|   Bit 4                                                               |
|       Point at each vertex if set                                     |
|                                                                       |
+----------------------------------------------------------------------*/
typedef struct
    {
    uint32_t      symType;              //  must specify a symbol component in the same file
    uint32_t      symID;                //                  ""
    uint16_t      strokeNo;             //  If (!(mod1 & LCPOINT_ANYVERTEX)) && (mod1 & LCPOINT_ONSTROKE)) selects stroke number from stroke pattern
    uint16_t      mod1;
    uint32_t      mod2;                 //  unused

    double        xOffset;
    double        yOffset;
    double        zOffset;              //  unused
    double        xAngle;               //  unused
    double        yAngle;               //  unused
    double        zAngle;               //  angle in degrees
    double        xScale;               //  unused
    double        yScale;               //  unused
    double        zScale;               //  unused
    } PointSymInfo, PointSymRscInfo;

typedef struct
    {
    char          descr[LS_MAX_DESCR];                           // WIP_CHAR_OK - in resource
    uint32_t      __lHdr1, __lHdr2, __auxType, __auxID;
    double        __dReserved1;
    uint32_t      lcType;
    uint32_t      lcID;
    uint32_t      __lReserved1;
#ifdef resource
    PointSymInfo  symbol[];     /* Point symbol info                   */
#else
    uint32_t      nSym;         /* Number of point symbols             */
    PointSymInfo  symbol[1];    /* Point symbol components             */
#endif
    } LinePointRsc;

#ifdef resource
resourceclass LinePointRsc      RTYPE_LinePoint;
#endif

/*----------------------------------------------------------------------+
|                                                                       |
|   Point Symbol Resource                                               |
|   RTYPE_PointSym                                                      |
|                                                                       |
|   Only the following fields are used.                                 |
|                                                                       |
|       descr                                                           |
|       auxType - set to RTYPE_PointSym for V8, 0 = RTYPE_PointSymV7    |
|       range                                                           |
|       symFlags                                                        |
|       nBytes                                                          |
|                                                                       |
|   Make sure all other fields are set to zero to ensure compatibility  |
|   with future versions                                                |
|                                                                       |
+----------------------------------------------------------------------*/
                                /* Values for "symFlags"               */
#define LSSYM_3D                0x01    /* 3d symbol                   */
#define LSSYM_NOSCALE           0x02    /* Acad compat - don't scale symbol at all. */

#if defined (BEIJING_DGNGRAPHICS_WIP)
    *** Move SymbolRange and PointSymRsc into Bentley namespace.
#endif // defined (BEIJING_DGNGRAPHICS_WIP)
#if !defined (mdl_type_resource_generator) && !defined (mdl_resource_compiler)
    typedef DPoint3d T_DPoint3d;
#else
    typedef DPoint3d T_DPoint3d;
#endif

typedef struct
    {
    T_DPoint3d    low;
    T_DPoint3d    high;
    } SymbolRange;

typedef struct
    {
    enum SymbolType
        {
        ST_Elements = 0,
        ST_XGraphics= 100,

        };

    struct
        {
        char        descr[LS_MAX_DESCR];                           // WIP_CHAR_OK - in resource
        uint32_t    symbolType, __lHdr2, __auxType, __auxID;
        SymbolRange range;
        T_DPoint3d  __offset_unused;
        double      scale;
        double      __dReserve[12];
        } header;

    uint32_t    symFlags;
#ifndef resource
    uint32_t    nBytes;
    uint8_t     symBuf[8];
#else
    uint8_t     symBuf[];
#endif

    SymbolType GetSymbolType () { return (SymbolType)header.symbolType; }
    void SetSymbolType (SymbolType sType) { header.symbolType = sType; }
    } PointSymRsc;

#ifdef resource
resourceclass PointSymRsc       RTYPE_PointSym;
#endif

END_BENTLEY_DGNPLATFORM_NAMESPACE

/** @endcond */
