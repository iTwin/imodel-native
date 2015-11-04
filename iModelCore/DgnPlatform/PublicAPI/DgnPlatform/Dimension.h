/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/Dimension.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
/** @cond BENTLEY_SDK_Internal */

#if defined (NEEDS_WORK_DGNITEM)
#include <DgnPlatform/ElementGeom.h>
#include <DgnPlatform/DgnPlatform.h>
#include <Bentley/ScopedArray.h>
#include "DimensionStyle.h"
#include "DimensionHandler.h"
#include "DimensionStyleProps.r.h"
#include "DimensionElem.h"
//__PUBLISH_SECTION_END__

BEGIN_BENTLEY_DGN_NAMESPACE
struct DimArcInfo
    {
    DPoint3d       start;
    DPoint3d       end;
    DPoint3d       center;
    double         radius;
    double         startAng;
    double         sweepAng;
    bool           notCircular;
    };

struct AdimSegmentTextBoxes;
END_BENTLEY_DGN_NAMESPACE

DGNPLATFORM_TYPEDEFS(AdimSegmentTextBoxes)
DGNPLATFORM_TYPEDEFS(DimMLText)
DGNPLATFORM_TYPEDEFS(DimFormattedText)
DGNPLATFORM_TYPEDEFS(DimArcInfo)
DGNPLATFORM_TYPEDEFS(DimDerivedData)
DGNPLATFORM_TYPEDEFS(DimOptionBlockHeader)

#define DIM_BLOCK_ALIGNED(typeName,pointerName) typeName localVar; if (NULL != pointerName) {Bentley::UnalignedMemcpy (&localVar, pointerName, sizeof(typeName)); pointerName = &localVar; }

/*======================================================================+
|                                                                       |
|   Function Definitions                                                |
|                                                                       |
+======================================================================*/
BEGIN_BENTLEY_NAMESPACE

DGNPLATFORM_EXPORT int mdlDim_extractStringsWide
(
WChar       wStrings[][MAX_DIMSTRING], /* <= Dimension strings          */
int*          stringConfig,              /* <= Text configuration         */
ElementHandleCR  dimElement,                       /* => Dimension element buffer   */
int           pointNo                    /* => Point number of text       */
);

DGNPLATFORM_EXPORT int mdlDim_deleteTextCluster
(
EditElementHandleR dimElement,
int           pointNo
);

DGNPLATFORM_EXPORT int mdlDim_insertTextCluster
(
EditElementHandleR dimElement,             /* <=> Dimension element buffer  */
int          pointNo,                   /*  => Point number of text      */
WChar      wStrings[][MAX_DIMSTRING]  /*  => Dimension strings         */
);

DGNPLATFORM_EXPORT int adim_getDimArcInfo (DimArcInfoP, ElementHandleCR dimElement);
DGNPLATFORM_EXPORT int adim_getArcInfo (DimArcInfoP, RotMatrixP , DisplayPathCP, AssocPointP, ElementHandleCR dimElement);

DGNPLATFORM_EXPORT int adim_getSweepTrans
(
double    *sweepAngle,       /* <= Sweep angle                         */
RotMatrixP outMatrix,        /* <= Rotation matrix (if not NULL)       */
DPoint3dP  origin,           /* => Origin of vectors                   */
DPoint3dP  xAxis,            /* => End of first line                   */
DPoint3dP  yAxis,            /* => End of second line                  */
RotMatrixP cTrans,
bool       i3d
);

DGNPLATFORM_EXPORT int   adim_transformInternalParameters
(
EditElementHandleR     elmP,                  /* <=> element to transform */
DgnModelP        srcDgnModel,
DgnModelP        dstDgnModel,
TransformCP pTrans,
bool             modifyAllowed           /*  => allowed to change element size */
);

DGNPLATFORM_EXPORT double    adim_getUnitScale
(
DgnModelP        srcDgnModel,
DgnModelP        dstDgnModel,
TransformCP pTrans          /* Used when src or dest is a cell model ref */
);

DGNPLATFORM_EXPORT int mdlDim_getHeightDirect
(
double*       pHeight,
ElementHandleCR pElement,
int           relative
);

DGNPLATFORM_EXPORT int mdlDim_setHeightDirect
(                               /* <=  SUCCESS if dimension has height; else ERROR */
EditElementHandleR dim,             /*  => */
double        height,           /*  => new value of height to use */
int           relative          /*  => */
);

DGNPLATFORM_EXPORT int mdlDim_getTextOffsetDirect
(
DPoint2d           *pPoint,     /* <= */
int                *pJustify,   /* <= */
ElementHandleCR dim,        /* => */
int                 segNo       /* => */
);

DGNPLATFORM_EXPORT  bool             mdlDim_getEffectiveAnnotationScale
(
double *                pValueOut,
ElementHandleCR pElm
);

DGNPLATFORM_EXPORT void         adim_transformDimPoints
(
EditElementHandleR     dimElement,
RotMatrixCP     directionMatrix,
TransformCP     trans,
bool                transformPoints,
bool                transformDistances
);

DGNPLATFORM_EXPORT void             mdlDim_dumpOverrideLinkages
(
ElementHandleCR dimElm
);

DGNPLATFORM_EXPORT int   adim_deleteOptionBlock
(
EditElementHandleR dim,               /* => Dimension element block is in  */
int           reqType             /* => Requested block type           */
);

DGNPLATFORM_EXPORT void  adim_getDimensionUnits (DimUnitBlockP, WCharP, WCharP, ElementHandleCR, bool);

DGNPLATFORM_EXPORT int   mdlDim_insertOptionBlock
(
EditElementHandleR      dim,                /* => Dimension element to work on   */
DimOptionBlockHeaderP   newBlock,        /* => Option block to insert         */
uint64_t* cellId
);

DGNPLATFORM_EXPORT void  adim_insertViewRotBlock
(
EditElementHandleR dimElement,
RotMatrixCR     rMatrix
);

DGNPLATFORM_EXPORT int   adim_getCellId
(
uint64_t*      uniqueIdP,             /* <= uniqueId */
ElementHandleCR elmP,                  /*  => element to test */
unsigned short  type                   /*  => app value to look for **/
);

DGNPLATFORM_EXPORT Dgn::AngleFormatVals    adim_getAngleFormat
(
ElementHandleCR pDim
);

DGNPLATFORM_EXPORT bool     mdlDim_hasV7incompatibleOverrides
(
int         *pNumOverrideLinkages,
ElementHandleCR pElementIn
);

DGNPLATFORM_EXPORT void     adim_checkUseOfAngularSettings
(
bool&               primary,
bool&               secondary,
ElementHandleCR     dimElement
);

DGNPLATFORM_EXPORT void     adim_setWitnessLinesFromTemplate
(
EditElementHandleR  dimElement,
bool                noWitness   /* => true: no witness, false: use template */
);

/*======================================================================+
|                                                                       |
|   Function Definitions                                                |
|                                                                       |
+======================================================================*/
DGNPLATFORM_EXPORT int mdlDim_extractPointsD
(
DPoint3dP  outPoints,
ElementHandleCR    dimElement,
int             pointNo,
int             nPoints
);

DGNPLATFORM_EXPORT void const* mdlDim_getOptionBlock
(
ElementHandleCR    dimElement,          /* => Dimension element block is in  */
int             reqType,             /* => Requested block type           */
uint64_t*      elementID
);

DGNPLATFORM_EXPORT DimOptionBlockHeaderP mdlDim_getOrCreateOptionBlock
(
EditElementHandleR pDim,      /* <=> */
int             type,       /* => one of ADBLK_ */
int             size,        /* => */
uint64_t*      elementID
);

DGNPLATFORM_EXPORT  int   mdlDim_isPointAssociative
(
bool              *pIsAssociative,    /* <= */
ElementHandleCR pDimElementIn,     /* => */
int               pointNo               /* => */
);

DGNPLATFORM_EXPORT  BentleyStatus  mdlDim_suppressWitnessline
(
EditElementHandleR pElementIn,
int         pointNo,
bool        bSuppressFlag
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  06/01
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT bool     mdlDim_getPrimaryToleranceAccuracy
(
uint16_t    *pValueOut,
ElementHandleCR pElm
);

DGNPLATFORM_EXPORT StatusInt    mdlDim_setPrimaryToleranceAccuracy
(
EditElementHandleR pElm,
uint16_t    *pValueIn
);

DGNPLATFORM_EXPORT bool     mdlDim_getSecondaryToleranceAccuracy
(
uint16_t    *pValueOut,
ElementHandleCR pElm
);

DGNPLATFORM_EXPORT StatusInt    mdlDim_setSecondaryToleranceAccuracy
(
EditElementHandleR pElm,
uint16_t    *pValueIn
);

DGNPLATFORM_EXPORT bool          mdlDim_getMultiJustVertical
(
uint16_t    *pValueOut,
ElementHandleCR pElm
);

DGNPLATFORM_EXPORT StatusInt     mdlDim_setMultiJustVertical
(
EditElementHandleR pElm,
uint16_t    *pValueIn
);

DGNPLATFORM_EXPORT bool          mdlDim_getNoReduceFraction
(
uint16_t    *pValueOut,
ElementHandleCR pElm
);

DGNPLATFORM_EXPORT StatusInt    mdlDim_setNoReduceFraction
(
EditElementHandleR pElm,
uint16_t    *pValueIn
);

DGNPLATFORM_EXPORT bool          mdlDim_getNoReduceAltFraction
(
uint16_t    *pValueOut,
ElementHandleCR pElm
);

DGNPLATFORM_EXPORT StatusInt    mdlDim_setNoReduceAltFraction
(
EditElementHandleR pElm,
uint16_t    *pValueIn
);

DGNPLATFORM_EXPORT bool         mdlDim_getNoReduceTolFraction
(
uint16_t    *pValueOut,
ElementHandleCR pElm
);

DGNPLATFORM_EXPORT StatusInt     mdlDim_setNoReduceTolFraction
(
EditElementHandleR pElm,
uint16_t    *pValueIn
);

DGNPLATFORM_EXPORT bool          mdlDim_getNoReduceSecFraction
(
uint16_t    *pValueOut,
ElementHandleCR pElm
);

DGNPLATFORM_EXPORT StatusInt    mdlDim_setNoReduceSecFraction
(
EditElementHandleR pElm,
uint16_t    *pValueIn
);

DGNPLATFORM_EXPORT bool          mdlDim_getNoReduceAltSecFraction
(
uint16_t    *pValueOut,
ElementHandleCR pElm
);

DGNPLATFORM_EXPORT StatusInt     mdlDim_setNoReduceAltSecFraction
(
EditElementHandleR pElm,
uint16_t    *pValueIn
);

DGNPLATFORM_EXPORT bool          mdlDim_getNoReduceTolSecFraction
(
uint16_t    *pValueOut,
ElementHandleCR pElm
);

DGNPLATFORM_EXPORT StatusInt    mdlDim_setNoReduceTolSecFraction
(
EditElementHandleR pElm,
uint16_t    *pValueIn
);

DGNPLATFORM_EXPORT bool          mdlDim_getNoteLeaderType
(
uint16_t    *pValueOut,
ElementHandleCR pElm
);

DGNPLATFORM_EXPORT StatusInt     mdlDim_setNoteLeaderType
(
EditElementHandleR pElm,
uint16_t    *pValueIn
);

DGNPLATFORM_EXPORT bool          mdlDim_getNoteScaleFrame
(
bool        *pValueOut,
ElementHandleCR pElm
);

DGNPLATFORM_EXPORT StatusInt     mdlDim_setNoteScaleFrame
(
EditElementHandleR pElm,
bool         *pValueIn
);

DGNPLATFORM_EXPORT bool          mdlDim_getNoteTerminator
(
uint16_t    *pValueOut,
ElementHandleCR pElm
);

DGNPLATFORM_EXPORT StatusInt     mdlDim_setNoteTerminator
(
EditElementHandleR pElm,
uint16_t    *pValueIn
);

DGNPLATFORM_EXPORT bool          mdlDim_getNoteTerminatorType
(
uint16_t    *pValueOut,
ElementHandleCR pElm
);

DGNPLATFORM_EXPORT StatusInt     mdlDim_setNoteTerminatorType
(
EditElementHandleR pElm,
uint16_t    *pValueIn
);

DGNPLATFORM_EXPORT bool          mdlDim_getNoteTextRotation
(
uint16_t    *pValueOut,
ElementHandleCR pElm
);

DGNPLATFORM_EXPORT StatusInt     mdlDim_setNoteTextRotation
(
EditElementHandleR pElm,
uint16_t    *pValueIn
);

DGNPLATFORM_EXPORT bool          mdlDim_getNoteHorAttachment
(
uint16_t    *pValueOut,
ElementHandleCR pElm
);

DGNPLATFORM_EXPORT StatusInt     mdlDim_setNoteHorAttachment
(
EditElementHandleR pElm,
uint16_t    *pValueIn
);

DGNPLATFORM_EXPORT bool          mdlDim_getNoteVerLeftAttachment
(
uint16_t    *pValueOut,
ElementHandleCR pElm
);

DGNPLATFORM_EXPORT StatusInt     mdlDim_setNoteVerLeftAttachment
(
EditElementHandleR pElm,
uint16_t    *pValueIn
);

DGNPLATFORM_EXPORT bool          mdlDim_getNoteVerRightAttachment
(
uint16_t    *pValueOut,
ElementHandleCR pElm
);

DGNPLATFORM_EXPORT StatusInt     mdlDim_setNoteVerRightAttachment
(
EditElementHandleR pElm,
uint16_t    *pValueIn
);

DGNPLATFORM_EXPORT  StatusInt     mdlDim_getNoteHorizontalJustification
(
Dgn::DimStyleProp_MLNote_Justification   *eJust,
ElementHandleCR dimElement
);

DGNPLATFORM_EXPORT  StatusInt   mdlDim_setNoteHorizontalJustification
(
EditElementHandleR dimElement,
Dgn::DimStyleProp_MLNote_Justification   eJust
);

DGNPLATFORM_EXPORT  StatusInt     mdlDim_getNoteFrameType
(
Dgn::DimStyleProp_MLNote_FrameType   *eFrameType,
ElementHandleCR dimElement
);

DGNPLATFORM_EXPORT  StatusInt   mdlDim_setNoteFrameType
(
EditElementHandleR dimElement,
Dgn::DimStyleProp_MLNote_FrameType   eFrameType
);

DGNPLATFORM_EXPORT  StatusInt   mdlDim_getNoteLeaderDisplay
(
bool                *bDisplay,
ElementHandleCR dimElement
);

DGNPLATFORM_EXPORT  StatusInt  mdlDim_setNoteLeaderDisplay
(
EditElementHandleR dimElement,
bool                bDisplay
);

DGNPLATFORM_EXPORT  bool        mdlDim_getNoteAllowAutoMode
(
ElementHandleCR dimElement
);

DGNPLATFORM_EXPORT  StatusInt   mdlDim_setNoteAllowAutoMode
(
EditElementHandleR dimElement,
bool                bAllow
);

enum    DimProxyOption
    {
    DIMPROXY_OPTION_Invalid         = -1,
    DIMPROXY_OPTION_NoProxy         =  0,
    DIMPROXY_OPTION_JustProxy       =  1,
    DIMPROXY_OPTION_Both            =  2,
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  01/02
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT void  mdlDim_setProxyOption
(
DimProxyOption   iOption
);

#if defined (DEBUG_FUNCTIONS_ONLY)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  07/01
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT StatusInt     mdlDimText_getTextNodeProperties
(
DimMLTextP pText,
TextParamWide   *pWide,
double          *pWidth,
double          *pHeight
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  07/01
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT StatusInt     mdlDimText_getPreValueProperties
(
DimMLTextP pText,
WChar         *pStringOut,
int             stringOutBufSize,
TextParamWide   *pWide,
double          *pWidth,
double          *pHeight,
DPoint2d        *pOrigin,
DPoint2d        *pGlyphSize
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  07/01
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT StatusInt     mdlDimText_getValueProperties
(
DimMLTextP pText,
WChar         *pStringOut,
int             stringOutBufSize,
TextParamWide   *pWide,
double          *pWidth,
double          *pHeight,
DPoint2d        *pOrigin,
DPoint2d        *pGlyphSize
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  07/01
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT StatusInt     mdlDimText_getPostValueProperties
(
DimMLTextP pText,
WChar         *pStringOut,
int             stringOutBufSize,
TextParamWide   *pWide,
double          *pWidth,
double          *pHeight,
DPoint2d        *pOrigin,
DPoint2d        *pGlyphSize
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  07/01
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT StatusInt     mdlDimText_getPrefixProperties
(
DimMLTextP pText,
WChar         *pStringOut,
int             stringOutBufSize,
TextParamWide   *pWide,
double          *pWidth,
double          *pHeight,
DPoint2d        *pOrigin,
DPoint2d        *pGlyphSize
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  07/01
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT StatusInt     mdlDimText_getSuffixProperties
(
DimMLTextP pText,
WChar         *pStringOut,
int             stringOutBufSize,
TextParamWide   *pWide,
double          *pWidth,
double          *pHeight,
DPoint2d        *pOrigin,
DPoint2d        *pGlyphSize
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  07/01
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT StatusInt     mdlDimText_getAllStringsForDebug
(
DimMLTextP       pText,
TextParamWide*  pTextParamWide,
WChar*        pPreValue,
WChar*        pValue,
WChar*        pPostValue
);
#endif  // defined (DEBUG_FUNCTIONS_ONLY)

DGNPLATFORM_EXPORT StatusInt     mdlDimText_getTextDescr
(
DimMLTextP pText,
EditElementHandleR  textNodeElement,
ElementHandleCR    elementIn,
WCharCP       pwPlaceholderStr
);

/*---------------------------------------------------------------------------------------

    Dimension multiline text functions

---------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  06/01
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT Dgn::DimFormattedText      *mdlDimText_getFormatter
(
DimMLTextCP pText,
const int         nth
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  06/01
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT StatusInt        mdlDimText_addFormatter
(
DimMLTextP pText,
Dgn::DimFormattedText    *pFmt
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  10/01
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT Dgn::DimFormattedText         *mdlDimText_getNodeFormatter
(
DimMLTextCP pText
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  06/01
+---------------+---------------+---------------+---------------+---------------+------*/
typedef StatusInt (*PFDimTextFmtTraverseFunction)
    (
    Dgn::DimFormattedText    **ppFmt,
    void                *pData,
    DgnModelP        modelRef,
    DimMLTextP pText,
    int                 currentRow
    );


// Currently used by convertv7tcb in ustation, when that moves these don't need to be exported
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BentleySystems  03/01
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT Dgn::UnitDefinition    adim_getUnitDefFromDimUnit
(
Dgn::DimUnits const&   dimUnit
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BentleySystems  03/01
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT Dgn::DimUnits      adim_getDimUnitFromUnitDef
(
UnitDefinitionCR unitDef
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BentleySystems  03/01
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT StatusInt     mdlDim_removeStyle
(
EditElementHandleR pElementIn
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BentleySystems  03/01
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT StatusInt     mdlDim_setStyleUniqueId
(
EditElementHandleR pElementIn,
uint64_t styleUniqueIdIn
);

DGNPLATFORM_EXPORT int mdlDim_scale
(
EditElementHandleR dimP,           /* <=> dimension element to scale */
double          scale,          /*  => scale factor */
bool         modifyAllowed   /*  => allowed to change element size */
);


DGNPLATFORM_EXPORT bool mdlDim_isTextOutside (ElementHandleCR element);

DGNPLATFORM_EXPORT int             mdlDim_fixRadialPoints (EditElementHandleR element, DisplayPathCP    pathP);

DGNPLATFORM_EXPORT int     mdlDim_getDimData
(
void        *dataP,
int         *dataSizeP,
ElementHandleCR elmP,
int         dataType,
uint32_t    dimArg
);

DGNPLATFORM_EXPORT int     mdlDim_setDimData
(
void        *dataP,
int         *dataSizeP,
EditElementHandleR elmP,
int         dataType,
uint32_t    dimArg
);

DGNPLATFORM_EXPORT bool      mdlDim_partTypeIsAnyDimLine
(
uint32_t partType
);

DGNPLATFORM_EXPORT bool      mdlDim_partTypeIsAnyExtension
(
uint32_t partType
);

DGNPLATFORM_EXPORT bool      mdlDim_partTypeIsAnyText
(
uint32_t partType
);

DGNPLATFORM_EXPORT bool      mdlDim_partTypeIsAnyTerminator
(
uint32_t partType
);

DGNPLATFORM_EXPORT double  mdlDim_getStackHeight
(
ElementHandleCR    dimElement,
int             segNo
);

DGNPLATFORM_EXPORT bool        mdlDim_hasLeaderedText
(
BitMaskP        pBitMask,
ElementHandleCR    dimElement
);

DGNPLATFORM_EXPORT int     mdlDim_getPointNumber
(
ElementHandleCR pElm,
int         segment,
int         partType,
int         partSub,
int         closeVertex
);

DGNPLATFORM_EXPORT StatusInt   mdlDim_getPartSymbology
(
Dgn::Symbology  *pSymbology,
ElementHandleCR elementIn,
uint32_t     partName
);

DGNPLATFORM_EXPORT bool    mdlDim_synchProxyCheckSum
(
EditElementHandleR  pElmdscr
);

DGNPLATFORM_EXPORT StatusInt    mdlDim_setWitnessSymbologyFlag
(
EditElementHandleR pDimElm,
bool                useWitnessSymb,
int                 fromNo,
int                 toNo
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    09/01
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT StatusInt       mdlDim_getDimRotMatrix
(
RotMatrixP rMatrix,
ElementHandleCR pDim
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/04
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT StatusInt       mdlDim_getDimRawMatrix
(
RotMatrixP pRMatrix,
ElementHandleCR pDim
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    09/01
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT StatusInt       mdlDim_setDimRotMatrix
(
EditElementHandleR dimElement,
RotMatrixCP  pRMatrix
);

DGNPLATFORM_EXPORT void             mdlDim_createRotationMatrix
(
RotMatrixR          matrixOut,
DPoint3dCR          point1,
DPoint3dCR          point2,
DPoint3dCR          point3,
int                 alignment,
RotMatrixCR         viewMatrix,
bool                dimIs3d
);

/*------------------------------------------------------------------------*//**
  The mdlDim_setUnitLabel function is used to set the string denoting the specified
dimension's units.
* @param        pElementIn IN OUT is the dimension element to set the units string on.
* @param        pLabelIn IN is the label string to set denoting the dimension's units.
* @param        labelFlag IN is one of the DIMLABEL_ constant values, listed below. This value
determines which of the unit label strings is being set.
DIMLABEL_MASTUNIT
DIMLABEL_SUBUNIT
DIMLABEL_SECONDARY_MASTUNIT
DIMLABEL_SECONDARY_SUBUNIT
* @return       SUCCESS unless the dimension element could not be updated, then ERROR.
* @ALinkJoin    usmthmdlDim_getUnitLabelC
* @Group        "Dimension Functions"
* @bsimethod
+--------------+--------------+---------------+----------------+-------------*/
DGNPLATFORM_EXPORT int   mdlDim_setUnitLabel
(
EditElementHandleR pElementIn,     /* <=> */
const WChar  *pLabelIn,       /* => NULL not setting */
int             labelFlag            /* => DIMLABEL_xxx */
);

/*------------------------------------------------------------------------*//**
  The mdlDim_getUnitLabel function is used to get the string denoting the specified
dimension's units.
* @param        pLabelOut OUT is the label string to set denoting the dimension's units.
* @param        pElementIn IN OUT is the dimension element to get the units string from.
* @param        labelFlags IN is one of the DIMLABEL_ constant values, listed below. This value
determines which of the unit label strings is returned.
DIMLABEL_MASTUNIT
DIMLABEL_SUBUNIT
DIMLABEL_SECONDARY_MASTUNIT
DIMLABEL_SECONDARY_SUBUNIT
* @return       SUCCESS unless the dimension element was invalid, then ERROR.
* @ALinkJoin    usmthmdlDim_setUnitLabelC
* @Group        "Dimension Functions"
* @bsimethod
+--------------+--------------+---------------+----------------+-------------*/
DGNPLATFORM_EXPORT int   mdlDim_getUnitLabel
(
WCharP        pLabelOut,         /* <= pass in NULL if not interested */
ElementHandleCR pElementIn,        /* <=> */
int             labelFlags          /* => DIMLABEL_xxx */
);

/*------------------------------------------------------------------------*//**
  The mdlDim_removeUnitLabel function is used to remove the string denoting the
specified dimension's units.
* @param        pElementIn IN is the dimension element to remove the units string from.
* @param        labelFlags IN is one of the DIMLABEL_ constant values, listed below. This value
determines which of the unit label strings is removed.
DIMLABEL_MASTUNIT
DIMLABEL_SUBUNIT
DIMLABEL_SECONDARY_MASTUNIT
DIMLABEL_SECONDARY_SUBUNIT
* @return       SUCCESS unless the dimension element was invalid, then ERROR.
* @Group        "Dimension Functions"
* @bsimethod
+--------------+--------------+---------------+----------------+-------------*/
DGNPLATFORM_EXPORT int   mdlDim_removeUnitLabel
(
EditElementHandleR pElementIn,
int         labelFlags      /* => DIMLABEL_xxx */
);

/*---------------------------------------------------------------------------------**//**
  Get an alternate unit separator character from dimension.
* @param        pSeparatorChar     OUT
* @param        pElementIn IN is the dimension element to get the separator from.
* @param        bFirstChar IN set to true for preceding and false for following separator
* @Group        "Dimension Functions"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT StatusInt     mdlDim_getAlternateSeparator
(
WCharP        pSeparatorChar,
ElementHandleCR    dimElement,
bool            bFirstChar
);

/*---------------------------------------------------------------------------------**//**
  The mdlDim_setCustomAlternateSeparator is used to set custom alternate unit separator.
* @param        pElementIn IN is the dimension element to set the separator on.
* @param        pSeparatorChar IN
* @param        bFirstChar IN set to true for preceding and false for following separator
* @Group        "Dimension Functions"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT StatusInt     mdlDim_setCustomAlternateSeparator
(
EditElementHandleR pElementIn,
WChar     pSeparatorChar,
bool        bFirstChar
);

/*---------------------------------------------------------------------------------**//**
  The mdlDim_setAlternateSeparator is used to set preferred alternate unit representation style.
* @param        pElementIn IN is the dimension element to set the separator on.
* @param        iType IN is one of the constant values listed below. This value determines
which of the presentation styles is used.
0 dual values are presented on separate lines
1 dual values are presented on in line with the value with default parentheses separator
2 dual values are presented on in line with the value with default square separator
3 dual values are presented on in line with the value with custom separator
* @ALinkJoin    usmtmdlDim_setCustomAlternateSeparatorC
* @Group        "Dimension Functions"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT StatusInt     mdlDim_setAlternateSeparator
(
EditElementHandleR pElementIn,
int         iType
);

/*------------------------------------------------------------------------*//**
  The mdlDim_overridesClearAll function is used to reset all of the override flags
on the given dimension element.
* @param        pElementIn IN OUT is the dimension element on which the override flags are
cleared.
* @return       This function is of type void and does not return a value.
* @Group        "Dimension Functions"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT void  mdlDim_overridesClearAll
(
EditElementHandleR pElementIn
);

/*------------------------------------------------------------------------*//**
  The mdlDim_deletePointOverride function is used to delete overrides for the
given point.
* @param        pElementIn IN OUT is the dimension element containing the point to remove
overrides from.
* @param        pointNo is the point within the dimention element to remove overrides
from.
* @return       SUCCESS if element is valid and point is in range of points.
* @ALinkJoin    usmthmdlDim_setPointOverrideC
* @Group        "Dimension Functions"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT StatusInt     mdlDim_deletePointOverride
(
EditElementHandleR pElementIn,
int                 pointNo
);

/*---------------------------------------------------------------------------------**//**
  Adds overrides to dimension element.
* @param        pElementIn IN OUT dimension to receive overrides.
* @param        pOverrides IN overrides to apply
* @param        pointNo IN set for point
* @return       SUCCESS if overrides are added.
* @Group        "Dimension Functions"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT StatusInt     mdlDim_setPointOverride
(
EditElementHandleR pElementIn,
Dgn::DimPointOverrides   *pOverrides,
int                 pointNo
);

/*---------------------------------------------------------------------------------**//**
  Retrieves overrides for point.
* @param        pOverridesOut OUT  point overrides
* @param        pElementIn IN  element to extract overrides from
* @param        pointNo IN  extract overrides for this point
* @return       SUCCESS if extraction is successful.
* @Group        "Dimension Functions"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT StatusInt     mdlDim_getPointOverride
(
Dgn::DimPointOverrides   *pOverridesOut,
ElementHandleCR pElementIn,
int                  pointNo
);

/*---------------------------------------------------------------------------------**//**
  Deletes overrides for given point
* @param        pElementIn IN OUT dimension to remove overrides.
* @param        segmentNo IN  remove overrides for this segment
* @return       SUCCESS if element is valid and segment is in range of points.
* @Group        "Dimension Functions"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT StatusInt     mdlDim_deleteSegmentOverride
(
EditElementHandleR elementIn,
int             segmentNo
);

/*---------------------------------------------------------------------------------**//**
  Adds overrides to dimension element.
* @param        pElementIn IN OUT dimension to receive overrides.
* @param        pOverrides IN overrides to apply
* @param        segmentNo IN segment for overrides
* @return       SUCCESS if overrides are added.
* @Group        "Dimension Functions"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT StatusInt     mdlDim_setSegmentOverride
(
EditElementHandleR     elementIn,
Dgn::DimSegmentOverrides *pOverrides,
int                 segmentNo
);

/*------------------------------------------------------------------------*//**
  The mdlDim_getSegmentOverride function is used to retrieve the overrides for the
specified segment in the given dimension element.
* @param        pOverridesOut OUT is the overrides retrieved from the specified segment.
* @param        pElementIn IN is the dimension element containing the segment to get the
overrides from.
* @param        pointNo IN indicates the segment of the dimension element to retrieve the
overrides from.
* @return       SUCCESS if the overrides are extracted successfully, otherwise ERROR.
* @ALinkJoin    usmthmdlDim_deleteSegmentOverrideC usmthmdlDim_setSegmentOverrideC
* @Group        "Dimension Functions"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT StatusInt     mdlDim_getSegmentOverride
(
Dgn::DimSegmentOverrides *pOverridesOut,
ElementHandleCR        dimElement,
int                 pointNo
);

/*---------------------------------------------------------------------------------**//**
  Deletes overrides for given point
* @param        pElementIn IN OUT dimension to remove overrides.
* @param        segmentNo IN  remove overrides for this segment
* @return       SUCCESS if element is valid and segment is in range of points.
* @Group        "Dimension Functions"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT StatusInt     mdlDim_deleteSegmentFlagOverride
(
EditElementHandleR pElementIn,
int             segmentNo
);

/*---------------------------------------------------------------------------------**//**
  Adds overrides to dimension element.
* @param        pElementIn IN OUT dimension to receive overrides.
* @param        pOverrides IN overrides to apply
* @param        segmentNo IN segment for overrides
* @return       SUCCESS if overrides are added.
* @Group        "Dimension Functions"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT StatusInt     mdlDim_setSegmentFlagOverride
(
EditElementHandleR pElementIn,
Dgn::DimSegmentFlagOverrides *pOverrides,
int                     segmentNo
);

/*------------------------------------------------------------------------*//**
  The mdlDim_getSegmentFlagOverride function is used to retrieve flag overrides
from the specified segment of the given dimension element.
* @param        pOverridesOut OUT is the segment flag overrides retrieved from the dimension
segment.
* @param        pElementIn IN is the dimension element containing the segment to retrieve
the overrides from.
* @param        segmentNo IN indicates the segment of the dimension to retrieve the
information from.
* @return       SUCCESS if the segment flag overrides are retrieved successfully.
* @ALinkJoin    usmthmdlDim_deleteSegmentFlagOverrideC usmthmdlDim_setSegmentFlagOverrideC
* @Group        "Dimension Functions"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT StatusInt     mdlDim_getSegmentFlagOverride
(
Dgn::DimSegmentFlagOverrides *pOverridesOut,
ElementHandleCR pElementIn,
int                     segmentNo
);

/*---------------------------------------------------------------------------------**//**
  Deletes overrides for given dimension
* @param        pElementIn IN OUT dimension to remove overrides.
* @return       SUCCESS if element is valid
* @Group        "Dimension Functions"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT StatusInt     mdlDim_deleteOverallOverride
(
EditElementHandleR pElementIn
);

/*---------------------------------------------------------------------------------**//**
  Adds overrides to dimension element.
* @param        pElementIn IN OUT dimension to receive overrides.
* @param        pOverrides IN overrides to apply
* @return       SUCCESS if overrides are added.
* @Group        "Dimension Functions"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT StatusInt     mdlDim_setOverallOverride
(
EditElementHandleR pElementIn,
Dgn::DimOverallOverrides *pOverrides
);

/*------------------------------------------------------------------------*//**
  The mdlDim_getOverallOverride function is used to retrieve overall overrides from
the specified dimension element.
* @param        pOverridesOut OUT is the overall overrides retrieved from the dimension
element.
* @param        pElementIn IN is the dimension element to retrieve the overrides from.
* @return       SUCCESS if the overall override information is retrieved successfully.
* @ALinkJoin    usmthmdlDim_deleteOverallOverrideC usmthmdlDim_setOverallOverrideC
* @Group        "Dimension Functions"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT StatusInt     mdlDim_getOverallOverride
(
Dgn::DimOverallOverrides     *pOverridesOut,
ElementHandleCR pElementIn
);

/*---------------------------------------------------------------------------------**//**
  Deletes extensions for given dimension
* @param        pElementIn IN OUT dimension to remove overrides.
* @return       SUCCESS if element is valid
* @Group        "Dimension Functions"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT StatusInt     mdlDim_deleteStyleExtension
(
EditElementHandleR pElementIn
);

/*---------------------------------------------------------------------------------**//**
  Adds extensions to dimension element.
* @param        pElementIn IN OUT dimension to receive overrides.
* @param        pExtensions IN extensions to apply
* @return       SUCCESS if extensions are added.
* @Group        "Dimension Functions"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT StatusInt     mdlDim_setStyleExtension
(
EditElementHandleR             pElementIn,
Dgn::DimStyleExtensions const*   pExtensions
);


/*---------------------------------------------------------------------------------**//**
  Deletes overrides for given dimension
* @param        pElementIn IN OUT dimension to remove overrides.
* @return       SUCCESS if element is valid
* @Group        "Dimension Functions"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT StatusInt     mdlDim_deleteOrdinateOverride
(
EditElementHandleR pElementIn
);

/*---------------------------------------------------------------------------------**//**
  Adds overrides to dimension element.
* @param        pElementIn IN OUT dimension to receive overrides.
* @param        pOverrides IN overrides to apply
* @return       SUCCESS if overrides are added.
* @Group        "Dimension Functions"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT StatusInt     mdlDim_setOrdinateOverride
(
EditElementHandleR pElementIn,
Dgn::DimOrdinateOverrides *pOverrides
);

/*---------------------------------------------------------------------------------**//**
  Retrieves overrides for point.
* @param        pOverridesOut OUT ordinate overrides
* @param        pElementIn IN
* @return       override extraction status.
* @Group        "Dimension Functions"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT StatusInt     mdlDim_getOrdinateOverride
(
Dgn::DimOrdinateOverrides *pOverridesOut,
ElementHandleCR pElementIn
);

/*---------------------------------------------------------------------------------**//**
  Get effective witnessline property.
* @param        pValue OUT value or NULL if not required
* @param        pElm IN
* @param        pointNo IN
* @return       true if override exists
* @Group        "Dimension Functions"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT bool          mdlDim_pointGetWitnessOffset
(
double      *pValue,
ElementHandleCR pElm,
int         pointNo
);

/*------------------------------------------------------------------------*//**
  The mdlDim_pointGetWitnessExtend function is used to get the extension value of
the witness line for the specified point in the given dimension element.
* @param        pValue OUT is the extension value retrieved from the dimension element.
* @param        pElm IN is the dimension element containing the point to get the information
from.
* @param        pointNo IN indicates the point in the dimension element to get the
information from.
* @return       true if the override for this setting is set.
* @ALinkJoin    usmthmdlDim_pointSetWitnessExtendC
* @Group        "Dimension Functions"
* @bsimethod
+--------------+--------------+---------------+----------------+-------------*/
DGNPLATFORM_EXPORT bool          mdlDim_pointGetWitnessExtend
(
double      *pValue,
ElementHandleCR pElm,
int         pointNo
);

/*------------------------------------------------------------------------*//**
  The mdlDim_pointGetWitnessColor function is used to get the color index of the
witness line for the specified point in the given dimension element.
* @param        pValue OUT is the color index retrieved from the dimension element.
* @param        pElm IN is the dimension element containing the point to get the information
from.
* @param        pointNo IN indicates the point in the dimension element to get the
information from.
* @return       true if the override for this setting is set.
* @ALinkJoin    usmthmdlDim_pointSetWitnessColorC
* @Group        "Dimension Functions"
* @bsimethod
+--------------+--------------+---------------+----------------+-------------*/
DGNPLATFORM_EXPORT bool          mdlDim_pointGetWitnessColor
(
uint32_t    *pValue,
ElementHandleCR pElm,
int          pointNo
);

/*------------------------------------------------------------------------*//**
  The mdlDim_pointGetWitnessWeight function is used to get the line weight value
applied to the witness line at the specified point of the given dimension element.
* @param        pValue OUT is the line weight retrieved from the dimension element.
* @param        pElm IN is the dimension element containing the point to get the information
from.
* @param        pointNo IN indicates the point in the dimension element to get the
information from.
* @return       true if the override for this setting is set.
* @ALinkJoin    usmthmdlDim_pointSetWitnessWeightC
* @Group        "Dimension Functions"
* @bsimethod
+--------------+--------------+---------------+----------------+-------------*/
DGNPLATFORM_EXPORT bool          mdlDim_pointGetWitnessWeight
(
uint32_t    *pValue,
ElementHandleCR pElm,
int         pointNo
);

/*------------------------------------------------------------------------*//**
  The mdlDim_pointGetWitnessStyle function is used to get the index of the line
style applied to the witness line at the specified point of the given dimension
element.
* @param        pValue OUT is the line style index retrieved from the dimension element.
* @param        pElm IN is the dimension element containing the point to get the information
from.
* @param        pointNo IN indicates the point in the dimension element to get the
information from.
* @return       true if the override for this setting is set.
* @ALinkJoin    usmthmdlDim_pointSetWitnessStyleC
* @Group        "Dimension Functions"
* @bsimethod
+--------------+--------------+---------------+----------------+-------------*/
DGNPLATFORM_EXPORT bool          mdlDim_pointGetWitnessStyle
(
int32_t    *pValue,
ElementHandleCR pElm,
int         pointNo
);

/*------------------------------------------------------------------------*//**
  The mdlDim_pointSetWitnessOffset function is used to set the effective
witnessline offset property at the specified point of the given dimension element.
* @param        pElm IN is the dimension element containing the point on which to set the
value
* @param        pointNo IN is the point at which to set the value.
* @param        pValueIn IN is the value to set or NULL to restore default
* @return       SUCCESS if the value was set successfully, otherwise ERROR.
* @ALinkJoin    usmthmdlDim_pointGetWitnessOffsetC
* @Group        "Dimension Functions"
* @bsimethod
+--------------+--------------+---------------+----------------+-------------*/
DGNPLATFORM_EXPORT StatusInt     mdlDim_pointSetWitnessOffset
(
EditElementHandleR pElm,
int         pointNo,
double      *pValueIn
);

/*------------------------------------------------------------------------*//**
  The mdlDim_pointSetWitnessExtend function is used to set the effective
witnessline extension valu at the specified point of the given dimension element.
* @param        pElm IN is the dimension element containing the point on which to set the
value
* @param        pointNo IN is the point at which to set the value.
* @param        pValueIn IN is the value to set.
* @return       SUCCESS if the value was set successfully, otherwise ERROR.
* @ALinkJoin    usmthmdlDim_pointGetWitnessExtendC
* @Group        "Dimension Functions"
* @bsimethod
+--------------+--------------+---------------+----------------+-------------*/
DGNPLATFORM_EXPORT StatusInt     mdlDim_pointSetWitnessExtend
(
EditElementHandleR pElm,
int         pointNo,
double      *pValueIn
);

/*------------------------------------------------------------------------*//**
  The mdlDim_pointSetWitnessColor function is used to set the effective witnessline
color index at the specified point of the given dimension element.
* @param        pElm IN is the dimension element containing the point on which to set the
value
* @param        pointNo IN is the point at which to set the value.
* @param        pValueIn IN is the value to set.
* @return       SUCCESS if the value was set successfully, otherwise ERROR.
* @ALinkJoin    usmthmdlDim_pointGetWitnessColorC
* @Group        "Dimension Functions"
* @bsimethod
+--------------+--------------+---------------+----------------+-------------*/
DGNPLATFORM_EXPORT StatusInt     mdlDim_pointSetWitnessColor
(
EditElementHandleR pElm,
int         pointNo,
uint32_t   *pValueIn
);

/*------------------------------------------------------------------------*//**
  The mdlDim_pointSetWitnessWeight function is used to set the effective
witnessline line weight value at the specified point of the given dimension element.
* @param        pElm IN is the dimension element containing the point on which to set the
value
* @param        pointNo IN is the point at which to set the value.
* @param        pValueIn IN is the value to set.
* @return       SUCCESS if the value was set successfully, otherwise ERROR.
* @ALinkJoin    usmthmdlDim_pointGetWitnessWeightC
* @Group        "Dimension Functions"
* @bsimethod
+--------------+--------------+---------------+----------------+-------------*/
DGNPLATFORM_EXPORT StatusInt     mdlDim_pointSetWitnessWeight
(
EditElementHandleR pElm,
int         pointNo,
uint32_t   *pValueIn
);

/*------------------------------------------------------------------------*//**
  The mdlDim_pointSetWitnessStyle function is used to set the effective line style
index at the specified point of the given dimension element.
* @param        pElm IN is the dimension element containing the point on which to set the
value
* @param        pointNo IN is the point at which to set the value.
* @param        pValueIn IN is the value to set.
* @return       SUCCESS if the value was set successfully, otherwise ERROR.
* @ALinkJoin    usmthmdlDim_pointGetWitnessStyleC
* @Group        "Dimension Functions"
* @bsimethod
+--------------+--------------+---------------+----------------+-------------*/
DGNPLATFORM_EXPORT StatusInt     mdlDim_pointSetWitnessStyle
(
EditElementHandleR pElm,
int         pointNo,
int32_t    *pValueIn
);

/*------------------------------------------------------------------------*//**
  The mdlDim_segmentGetTextRotation function is used to get the effective text
rotation value from the specified segment of the given dimension element.
* @param        pValue OUT is the retrieved value.
* @param        pElm IN is the dimension element containing the segment to get the value
from.
* @param        segmentNo IN is the segment from which to get the value.
* @return       true if the override for this setting is set.
* @ALinkJoin    usmthmdlDim_segmentSetTextRotationC
* @Group        "Dimension Functions"
* @bsimethod
+--------------+--------------+---------------+----------------+-------------*/
DGNPLATFORM_EXPORT bool       mdlDim_segmentGetTextRotation
(
double         *pValue,
ElementHandleCR pElm,
int             segmentNo
);

/*------------------------------------------------------------------------*//**
  The mdlDim_segmentGetCurveStartTangent function is used to retrieve the
effective start tangency bvector for notes with curved leaders.
* @param        pStartTangentOut OUT is the start tangency bvector of the curved leader,
pointing into the curve
* @param        pElm IN is the dimension element to get the tangency from.
* @param        segmentNo IN is the zero-based segment number of the dimension. For dimensions
with only one segment, segNo is zero. Start and End tangencies for curved leader notes are
stored on the first segment.
* @return       true if override exists
* @ALinkJoin    usmthmdlDim_segmentSetCurveStartTangentC
* @Group        "Dimension Functions"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT bool         mdlDim_segmentGetCurveStartTangent
(
DPoint3dP       pStartTangentOut,
ElementHandleCR pElm,
int             segmentNo
);

/*------------------------------------------------------------------------*//**
  The mdlDim_segmentGetCurveEndTangent function is used to retrieve the
effective end tangency bvector for notes with curved leaders.
* @param        pEndTangentOut OUT is the end tangency bvector of the curved leader,
pointing into the curve
* @param        pElm IN is the dimension element to get the tangency from.
* @param        segmentNo IN is the zero-based segment number of the dimension. For dimensions
with only one segment, segNo is zero. Start and End tangencies for curved leader notes are
stored on the first segment.
* @return       true if override exists
* @ALinkJoin    usmthmdlDim_segmentSetCurveEndTangentC
* @Group        "Dimension Functions"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT bool         mdlDim_segmentGetCurveEndTangent
(
DPoint3dP       pEndTangentOut,
ElementHandleCR pElm,
int             segmentNo
);

/*------------------------------------------------------------------------*//**
  The mdlDim_segmentGetTextJustification function is used to get the effective
text justification value from the specified segment of the given dimension element.
* @param        pValue OUT is the retrieved value.
* @param        pElm IN is the dimension element containing the segment to get the value
from.
* @param        segmentNo IN is the segment from which to get the value.
* @return       true if the override for this setting is set.
* @ALinkJoin    usmthmdlDim_segmentSetTextJustificationC
* @Group        "Dimension Functions"
* @bsimethod
+--------------+--------------+---------------+----------------+-------------*/
DGNPLATFORM_EXPORT bool          mdlDim_segmentGetTextJustification
(
uint16_t    *pValue,
ElementHandleCR pElm,
int         segmentNo
);

/*---------------------------------------------------------------------------------**//**
  The mdlDim_segmentSetTextRotation function is used to set the effective text
rotation for the specified segment of the given dimension element.
* @param        pElm IN is the dimension element containing the segment to set the value on.
* @param        segmentNo IN is the segment on which to set the value.
* @param        pValueIn IN is the value to set, or NULL to restore the default.
* @return       SUCCESS if the value was set, otherwise ERROR.
* @ALinkJoin    usmthmdlDim_segmentGetTextRotationC
* @Group        "Dimension Functions"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT StatusInt     mdlDim_segmentSetTextRotation
(
EditElementHandleR dimElement,
int         segmentNo,
double      *pValueIn
);

/*------------------------------------------------------------------------*//**
  The mdlDim_segmentSetCurveStartTangent function is used to set the
effective start tangency bvector for notes with curved leaders.
* @param        pElm IN is the dimension element to get the tangency on.
* @param        segmentNo IN is the zero-based segment number of the dimension. For dimensions
with only one segment, segNo is zero. Start and End tangencies for curved leader notes are
stored on the first segment.
* @param        pStartTangentIn IN is the start tangency bvector of the curved leader,
pointing into the curve
* @return       SUCCESS if the value was set, otherwise ERROR.
* @ALinkJoin    usmthmdlDim_segmentGetCurveStartTangentC
* @Group        "Dimension Functions"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT StatusInt     mdlDim_segmentSetCurveStartTangent
(
EditElementHandleR pElm,
int             segmentNo,
DPoint3dP  pStartTangentIn
);

/*------------------------------------------------------------------------*//**
  The mdlDim_segmentSetCurveEndTangent function is used to set the
effective end tangency bvector for notes with curved leaders.
* @param        pElm IN is the dimension element to set the tangency on.
* @param        segmentNo IN is the zero-based segment number of the dimension. For dimensions
with only one segment, segNo is zero. Start and End tangencies for curved leader notes are
stored on the first segment.
* @param        pEndTangentIn IN is the end tangency bvector of the curved leader,
pointing into the curve
* @return       SUCCESS if the value was set, otherwise ERROR.
* @ALinkJoin    usmthmdlDim_segmentGetCurveEndTangentC
* @Group        "Dimension Functions"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT StatusInt     mdlDim_segmentSetCurveEndTangent
(
EditElementHandleR pElm,
int             segmentNo,
DPoint3dP  pEndTangentIn
);

/*------------------------------------------------------------------------*//**
  The mdlDim_segmentSetTextJustification function is used to set the effective
text justification for the specified segment of the given dimension on.
* @param        pElm IN is the dimension element containing the segment to set the value on.
* @param        segmentNo IN is the segment on which to set the value.
* @param        pValueIn IN is the value to set, or NULL to restore the default.
* @return       SUCCESS if the value was set, otherwise ERROR.
* @ALinkJoin    usmthmdlDim_segmentGetTextJustificationC
* @Group        "Dimension Functions"
* @bsimethod
+--------------+--------------+---------------+----------------+-------------*/
DGNPLATFORM_EXPORT StatusInt     mdlDim_segmentSetTextJustification
(
EditElementHandleR pElm,
int         segmentNo,
uint16_t    *pValueIn
);

/*------------------------------------------------------------------------*//**
  The mdlDim_segmentFlagGetUnderlineText function is used to determine whether
the underline text override flag is set on the specified segment of the given
dimension.
* @param        pValue OUT is the value of the flag, either true or false, or it could be set to
NULL if not required
* @param        pElm IN is the dimension element containing the segment to get the flag
value from.
* @param        segmentNo IN is the segment from which to get the flag value.
* @return       true if the override is set.
* @ALinkJoin    usmthmdlDim_segmentFlagSetUnderlineTextC
* @Group        "Dimension Functions"
* @bsimethod
+--------------+--------------+---------------+----------------+-------------*/
DGNPLATFORM_EXPORT bool          mdlDim_segmentFlagGetUnderlineText
(
bool        *pValue,
ElementHandleCR pElm,
int         segmentNo
);

/*------------------------------------------------------------------------*//**
  The mdlDim_segmentFlagGetSuppressLeftDimLine function is used to determine
whether the SuppressLeftDimensionLine flag has been set on the specified segment
of the given dimension element.
* @param        pValue OUT is the value of the flag, either true or false, or it could be set to
NULL if not required
* @param        pElm IN is the dimension element containing the segment to get the flag
value from.
* @param        segmentNo IN is the segment from which to get the flag value.
* @return       true if the override is set.
* @ALinkJoin    usmthmdlDim_segmentFlagSetSuppressLeftDimLineC
* @Group        "Dimension Functions"
* @bsimethod
+--------------+--------------+---------------+----------------+-------------*/
DGNPLATFORM_EXPORT bool          mdlDim_segmentFlagGetSuppressLeftDimLine
(
bool        *pValue,
ElementHandleCR pElm,
int         segmentNo
);

/*------------------------------------------------------------------------*//**
  The mdlDim_segmentFlagGetSuppressRightDimLine function is used to determine
whether the SuppressRightDimensionLine flag has been set on specified segment of
the given dimension element.
* @param        pValue OUT is the value of the flag, either true or false, or it could be set to
NULL if not required
* @param        pElm IN is the dimension element containing the segment to get the flag
value from.
* @param        segmentNo IN is the segment from which to get the flag value.
* @return       true if the override is set.
* @ALinkJoin    usmthmdlDim_segmentFlagSetSuppressRightDimLineC
* @Group        "Dimension Functions"
* @bsimethod
+--------------+--------------+---------------+----------------+-------------*/
DGNPLATFORM_EXPORT bool          mdlDim_segmentFlagGetSuppressRightDimLine
(
bool        *pValue,
ElementHandleCR pElm,
int         segmentNo
);

/*------------------------------------------------------------------------*//**
  The mdlDim_segmentFlagGetPrimaryIsReference function is used to determine
whether the PrimaryIsReference flag has been set on the specified segment of the
given dimension element.
* @param        pValue OUT is the value of the flag, either true or false, or it could be set to
NULL if not required
* @param        pElm IN is the dimension element containing the segment to get the flag
value from.
* @param        segmentNo IN is the segment from which to get the flag value.
* @return       true if the override is set.
* @ALinkJoin    usmthmdlDim_segmentFlagSetPrimaryIsReferenceC
* @Group        "Dimension Functions"
* @bsimethod
+--------------+--------------+---------------+----------------+-------------*/
DGNPLATFORM_EXPORT bool          mdlDim_segmentFlagGetPrimaryIsReference
(
bool        *pValue,
ElementHandleCR pElm,
int         segmentNo
);

/*------------------------------------------------------------------------*//**
  The mdlDim_segmentFlagGetSecondaryIsReference function is used to determine
whether the SecondaryIsReference flag has been set on the specified segment of
the given dimension element.
* @param        pValue OUT is the value of the flag, either true or false, or it could be set to
NULL if not required
* @param        pElm IN is the dimension element containing the segment to get the flag
value from.
* @param        segmentNo IN is the segment from which to get the flag value.
* @return       true if the override is set.
* @ALinkJoin    usmthmdlDim_segmentFlagSetSecondaryIsReferenceC
* @Group        "Dimension Functions"
* @bsimethod
+--------------+--------------+---------------+----------------+-------------*/
DGNPLATFORM_EXPORT bool          mdlDim_segmentFlagGetSecondaryIsReference
(
bool        *pValue,
ElementHandleCR pElm,
int         segmentNo
);

/*------------------------------------------------------------------------*//**
  The mdlDim_segmentFlagSetUnderlineText function is used to set the underline
text flag on the specified segment of the given dimension element.
* @param        pElm IN is the dimension element containing the segment to set the flag value
on.
* @param        segmentNo IN is the segment on which to set the flag value.
* @param        pValueIn IN is the value of the flag, either true or false, or it could be set to
NULL to restore the default flag setting.
* @return       SUCCESS if the flag value is set successfully, otherwise ERROR.
* @ALinkJoin    usmthmdlDim_segmentFlagGetUnderlineTextC
* @Group        "Dimension Functions"
* @bsimethod
+--------------+--------------+---------------+----------------+-------------*/
DGNPLATFORM_EXPORT StatusInt     mdlDim_segmentFlagSetUnderlineText
(
EditElementHandleR pElm,
int         segmentNo,
bool        *pValueIn
);

/*------------------------------------------------------------------------*//**
  The mdlDim_segmentFlagSetSuppressLeftDimLine function is used to set the
SuppressLeftDimensionLine flag on the specified segment of the given dimension
element.
* @param        pElm IN is the dimension element containing the segment to set the flag value
on.
* @param        segmentNo IN is the segment on which to set the flag value.
* @param        pValueIn IN is the value of the flag, either true or false, or it could be set to
NULL to restore the default flag setting.
* @return       SUCCESS if the flag value is set successfully, otherwise ERROR.
* @ALinkJoin    usmthmdlDim_segmentFlagGetSuppressLeftDimLineC
* @Group        "Dimension Functions"
* @bsimethod
+--------------+--------------+---------------+----------------+-------------*/
DGNPLATFORM_EXPORT StatusInt     mdlDim_segmentFlagSetSuppressLeftDimLine
(
EditElementHandleR pElm,
int         segmentNo,
bool        *pValueIn
);

/*------------------------------------------------------------------------*//**
  The mdlDim_segmentFlagSetSuppressRightDimLine function is used to set the
SuppressRightDimensionLine flag on the specified segment of the given dimension
element.
* @param        pElm IN is the dimension element containing the segment to set the flag value
on.
* @param        segmentNo IN is the segment on which to set the flag value.
* @param        pValueIn IN is the value of the flag, either true or false, or it could be set to
NULL to restore the default flag setting.
* @return       SUCCESS if the flag value is set successfully, otherwise ERROR.
* @ALinkJoin    usmthmdlDim_segmentFlagGetSuppressRightDimLineC
* @Group        "Dimension Functions"
* @bsimethod
+--------------+--------------+---------------+----------------+-------------*/
DGNPLATFORM_EXPORT StatusInt     mdlDim_segmentFlagSetSuppressRightDimLine
(
EditElementHandleR pElm,
int         segmentNo,
bool        *pValueIn
);

/*------------------------------------------------------------------------*//**
  The mdlDim_segmentFlagSetPrimaryIsReference function is used to set the
PrimaryIsReference flag on the specified segment of the given dimension element.
* @param        pElm IN is the dimension element containing the segment to set the flag value
on.
* @param        segmentNo IN is the segment on which to set the flag value.
* @param        pValueIn IN is the value of the flag, either true or false, or it could be set to
NULL to restore the default flag setting.
* @return       SUCCESS if the flag value is set successfully, otherwise ERROR.
* @ALinkJoin    usmthmdlDim_segmentFlagGetPrimaryIsReferenceC
* @Group        "Dimension Functions"
* @bsimethod
+--------------+--------------+---------------+----------------+-------------*/
DGNPLATFORM_EXPORT StatusInt     mdlDim_segmentFlagSetPrimaryIsReference
(
EditElementHandleR pElm,
int         segmentNo,
bool        *pValueIn
);

/*------------------------------------------------------------------------*//**
  The mdlDim_segmentFlagSetSecondaryIsReference function is used to set the
SecondaryIsReference flag on the specified segment of the given dimension
element.
* @param        pElm IN is the dimension element containing the segment to set the flag value
on.
* @param        segmentNo IN is the segment on which to set the flag value.
* @param        pValueIn IN is the value of the flag, either true or false, or it could be set to
NULL to restore the default flag setting.
* @return       SUCCESS if the flag value is set successfully, otherwise ERROR.
* @ALinkJoin    usmthmdlDim_segmentFlagGetSecondaryIsReferenceC
* @Group        "Dimension Functions"
* @bsimethod
+--------------+--------------+---------------+----------------+-------------*/
DGNPLATFORM_EXPORT StatusInt     mdlDim_segmentFlagSetSecondaryIsReference
(
EditElementHandleR pElm,
int         segmentNo,
bool        *pValueIn
);

/*---------------------------------------------------------------------------------**//**
  The mdlDim_overallGetRefScale function is used to get the RefScale
*               property on the given dimension element
*
* @param        pValueOut   OUT points to the effective property.
* @param        pElementIn  IN the dimension element to get the property value from.
* @return       true if override exists
* @ALinkJoin    usmthmdlDim_overallSetRefScaleC
* @Group        "Dimension Functions"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT bool          mdlDim_overallGetRefScale
(
double      *pValueOut,
ElementHandleCR pElementIn
);

/*---------------------------------------------------------------------------------**//**
  The mdlDim_overallGetAngleQuadrant function is used to get the angle
quadrant property on the given dimension element.
* @param        pValueOut   OUT the quadrant property value of the dimension element.
* @param        pElm        IN the dimension element to get the quadrant property from.
* @return       true if exists
* @Group        "Dimension Functions"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT bool          mdlDim_overallGetAngleQuadrant
(
uint16_t    *pValueOut,
ElementHandleCR pElm
);

/*---------------------------------------------------------------------------------**//**
  The mdlDim_overallSetAngleQuadrant function is used to set the angle
quadrant property on the given dimension element.
* @param        pElm        IN the dimension element to set the value on.
* @param        pValueIn    IN value or NULL to restore default.
* @return       SUCCESS if can be set.
* @Group        "Dimension Functions"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT StatusInt     mdlDim_overallSetAngleQuadrant
(
EditElementHandleR pElm,
uint16_t    *pValueIn
);

/*---------------------------------------------------------------------------------**//**
  The mdlDim_overallGetSlantAngle function is used to get the slant angle
property on the given dimension element. This property is only used by 3d dimensions
with arbitrary alignment.
* @param        pValueOut   OUT the slant angle property value of the dimension element.
* @param        pElm        IN the dimension element to get the slant angle property from.
* @return       true if exists
* @Group        "Dimension Functions"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT bool          mdlDim_overallGetSlantAngle
(
double      *pValueOut,
ElementHandleCR pElm
);

/*---------------------------------------------------------------------------------**//**
  The mdlDim_overallSetSlantAngle function is used to set the slant angle
property on the given dimension element.  This property is only used by 3d dimensions
with arbitrary alignment.
* @param        pElm        IN the dimension element to set the value on.
* @param        pValueIn    IN value or NULL to restore default.
* @return       SUCCESS if can be set.
* @Group        "Dimension Functions"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT StatusInt     mdlDim_overallSetSlantAngle
(
EditElementHandleR pElm,
double      *pValueIn
);

/*---------------------------------------------------------------------------------**//**
  The mdlDim_overallGetModelAnnotationScale function is used to get the
*               Model Annotation Scale property on the given dimension element
*
* @param        pValueOut   OUT points to the effective property.
* @param        pElementIn  IN the dimension element to get the property value from.
* @return       true if override exists
* @ALinkJoin    usmthmdlDim_overallSetModelAnnotationScaleC
* @Group        "Dimension Functions"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT bool          mdlDim_overallGetModelAnnotationScale
(
double                  *pValueOut,
ElementHandleCR pElementIn
);

/*---------------------------------------------------------------------------------**//**
  The mdlDim_overallSetModelAnnotationScale function is used to set the
*               Model Annotation Scale property on the given dimension element.
*
* @param        pElm IN is the dimension element to set the property for.
* @param        pValueIn IN is the value of the property, it could be set to
NULL to restore the default setting.
* @return       SUCCESS if the property value is set successfully, otherwise ERROR.
* @ALinkJoin    usmthmdlDim_overallGetModelAnnotationScaleC
* @Group        "Dimension Functions"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT StatusInt    mdlDim_overallSetModelAnnotationScale
(
EditElementHandleR pElm,
double      *pValueIn
);

/*---------------------------------------------------------------------------------**//**
  The mdlDim_getPrimaryRoundOff function is used to get the primary unit
*               round off value property on the given dimension element.
* @param        pValueOut   OUT the primary round off value
* @param        pElm        IN the dimension element to get the value from
* @return       true if property is set exists
* @Group        "Dimension Functions"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT bool          mdlDim_getPrimaryRoundOff
(
double      *pValueOut,
ElementHandleCR pElm
);

/*---------------------------------------------------------------------------------**//**
  The mdlDim_getSecondaryRoundOff function is used to get the secondary unit
*               round off value property on the given dimension element.
* @param        pValueOut   OUT the secondary roundoff value
* @param        pElm        IN the dimension element to get the value from
* @return       true if property is set exists
* @Group        "Dimension Functions"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT bool          mdlDim_getSecondaryRoundOff
(
double      *pValueOut,
ElementHandleCR pElm
);

/*---------------------------------------------------------------------------------**//**
  The mdlDim_setPrimaryRoundOff function is used to set the primary unit
*               round off value property on the given dimension elmeent
* @param        pElm        IN the dimension element to set the value on.
* @param        pValueIn    IN value or NULL to restore default.
* @return       SUCCESS if property can be set.
* @Group        "Dimension Functions"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT StatusInt     mdlDim_setPrimaryRoundOff
(
EditElementHandleR pElm,
double      *pValueIn
);

/*---------------------------------------------------------------------------------**//**
  The mdlDim_setSecondaryRoundOff function is used to set the primary unit
*               round off value property on the given dimension elmeent
* @param        pElm        IN the dimension element to set the value on.
* @param        pValueIn    IN value or NULL to restore default.
* @return       SUCCESS if property can be set.
* @Group        "Dimension Functions"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT StatusInt     mdlDim_setSecondaryRoundOff
(
EditElementHandleR pElm,
double      *pValueIn
);

/*---------------------------------------------------------------------------------**//**
  The mdlDim_getElbowLength function is used to get the elbow length of
*               property on the given multiline note dimension element.
* @param        pValueOut   OUT the elbow length value
* @param        pElm        IN the dimension element to get the value from
* @return       true if property is set exists
* @Group        "Dimension Functions"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT bool          mdlDim_getNoteElbowLength
(
double      *pValueOut,
ElementHandleCR pElm
);

/*---------------------------------------------------------------------------------**//**
  The mdlDim_setElbowLength function is used to set the elbow length
*               value property on the given dimension element
* @param        pElm        IN the dimension element to set the value on.
* @param        pValueIn    IN value or NULL to restore default.
* @return       SUCCESS if property can be set.
* @Group        "Dimension Functions"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT StatusInt     mdlDim_setNoteElbowLength
(
EditElementHandleR pElm,
double      *pValueIn
);

/*---------------------------------------------------------------------------------**//**
  The mdlDim_getNoteLeftMargin function is used to get the left margin of
*               property on the given multiline note dimension element.
* @param        pValueOut   OUT the left margin value
* @param        pElm        IN the dimension element to get the value from
* @return       true if property is set exists
* @Group        "Dimension Functions"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT bool          mdlDim_getNoteLeftMargin
(
double      *pValueOut,
ElementHandleCR pElm
);

/*---------------------------------------------------------------------------------**//**
  The mdlDim_setNoteLeftMargin function is used to set the left margin
*               value property (as a factor of text height) on the given note
                dimension element
* @param        pElm        IN the dimension element to set the value on.
* @param        pValueIn    IN value or NULL to restore default.
* @return       SUCCESS if property can be set.
* @Group        "Dimension Functions"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT StatusInt     mdlDim_setNoteLeftMargin
(
EditElementHandleR pElm,
double      *pValueIn
);

/*---------------------------------------------------------------------------------**//**
  The mdlDim_getNoteLowerMargin function is used to get the lower margin of
*               property on the given multiline note dimension element.
* @param        pValueOut   OUT the lower margin value
* @param        pElm        IN the dimension element to get the value from
* @return       true if property is set exists
* @Group        "Dimension Functions"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT bool          mdlDim_getNoteLowerMargin
(
double      *pValueOut,
ElementHandleCR pElm
);

/*---------------------------------------------------------------------------------**//**
  The mdlDim_setNoteLowerMargin function is used to set the lower margin
*               value property (as a factor of text height) on the given note
                dimension element
* @param        pElm        IN the dimension element to set the value on.
* @param        pValueIn    IN value or NULL to restore default.
* @return       SUCCESS if property can be set.
* @Group        "Dimension Functions"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT StatusInt     mdlDim_setNoteLowerMargin
(
EditElementHandleR pElm,
double      *pValueIn
);

/*---------------------------------------------------------------------------------**//**
  The mdlDim_getNoteFrameScale function is used to get the style note
*               frame scale value property on the given dimension element
* @param        pValueOut   OUT value or NULL to restore default.
* @param        pElm        IN the dimension element to set the value on.
* @return       true if property exists.
* @Group        "Dimension Functions"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT bool          mdlDim_getNoteFrameScale
(
double                  *pValueOut,
ElementHandleCR pElm
);

/*---------------------------------------------------------------------------------**//**
  The mdlDim_setNoteFrameScale function is used to set the style note
*               frame scale value property on the given dimension element
* @param        pElm        IN the dimension element to set the value on.
* @param        pValueIn    IN value or NULL to restore default.
* @return       true if property exists.
* @Group        "Dimension Functions"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT StatusInt     mdlDim_setNoteFrameScale
(
EditElementHandleR pElm,
double      *pValueIn
);

/*---------------------------------------------------------------------------------**//**
  The mdlDim_getAnnotationScale function is used to get the style annotation
*               scale value property on the given dimension element
* @param        pValueOut   OUT value or NULL to restore default.
* @param        pElm        IN the dimension element to set the value on.
* @return       SUCCESS if property can be set.
* @Group        "Dimension Functions"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT bool          mdlDim_getAnnotationScale
(
double                  *pValueOut,
ElementHandleCR pElm
);

/*---------------------------------------------------------------------------------**//**
  The mdlDim_setAnnotationScale function is used to set the style annotation
*               scale value property on the given dimension element
* @param        pElm        IN the dimension element to set the value on.
* @param        pValueIn    IN value or NULL to restore default.
* @return       SUCCESS if property can be set.
* @Group        "Dimension Functions"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT StatusInt    mdlDim_setAnnotationScale
(
EditElementHandleR pElm,
double      *pValueIn
);

/*---------------------------------------------------------------------------------**//**
  The mdlDim_getNotUseModelAnnotationScaleFlag function is used to get
*               the use model annotation scale flag property on the given dimension element
* @param        pValueOut   OUT value or NULL to restore default.
* @param        pElm        IN the dimension element to set the value on.
* @return       true if property exists.
* @Group        "Dimension Functions"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT bool          mdlDim_getNotUseModelAnnotationScaleFlag
(
bool                    *pValueOut,
ElementHandleCR pElm
);

/*---------------------------------------------------------------------------------**//**
  The mdlDim_getTextLocation function is used to get the text location
*               property on the given dimension element
* @param        pValueOut   OUT value or NULL to restore default.
* @param        pElm        IN the dimension element to set the value on.
* @return       SUCCESS if property can be set.
* @Group        "Dimension Functions"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT StatusInt         mdlDim_getTextLocation
(
Dgn::DimStyleProp_Text_Location  *pValueOut,
ElementHandleCR pElm
);

/*---------------------------------------------------------------------------------**//**
  The mdlDim_setTextLocation function is used to set the text location
*               property on the given dimension element
                dimension element
* @param        pElm        IN the dimension element to set the value on.
* @param        pValueIn    IN value or NULL to restore default.
* @return       SUCCESS if property can be set.
* @Group        "Dimension Functions"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT StatusInt         mdlDim_setTextLocation
(
EditElementHandleR pElm,
Dgn::DimStyleProp_Text_Location  *pValueIn
);

/*---------------------------------------------------------------------------------**//**
  Get effective ordinate X value.
* @param        pValueOut   OUT points to the effective X ordinate value.
* @param        pElementIn  IN the dimension element to get the X ordinate value from.
* @return       true if override exists
* @Group        "Dimension Functions"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT bool          mdlDim_ordinateGetStartValueX
(
double      *pValueOut,
ElementHandleCR pElementIn
);

/*---------------------------------------------------------------------------------**//**
  Get effective ordinate Y value.
* @param        pValueOut   OUT points to the effective Y ordinate value.
* @param        pElementIn  IN the dimension element to get the Y ordinate value from.
* @return       true if override exists
* @Group        "Dimension Functions"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT bool          mdlDim_ordinateGetStartValueY
(
double      *pValueOut,
ElementHandleCR pElementIn
);

/*---------------------------------------------------------------------------------**//**
  Set effective ordinate X value.
* @param        pElm  IN the dimension element to set the value on.
* @param        pValueIn IN value or NULL to restore default
* @return       SUCCESS if can be set
* @Group        "Dimension Functions"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT StatusInt     mdlDim_ordinateSetStartValueX
(
EditElementHandleR pElm,
double      *pValueIn
);

/*---------------------------------------------------------------------------------**//**
  Set effective ordinate Y value.
* @param        pElm  IN the dimension element to set the value on.
* @param        pValueIn IN value or NULL to restore default
* @return       SUCCESS if can be set
* @Group        "Dimension Functions"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT StatusInt     mdlDim_ordinateSetStartValueY
(
EditElementHandleR pElm,
double      *pValueIn
);

/*---------------------------------------------------------------------------------**//**
  Set extended text to dimension.
*
* @param        pElementIn  IN is the dimension element containing the segment to set the text on.
* @param        pText       IN is the text context containing text to be added
* @param        segmentNo   IN is the segment on which to set the text.
* @return       SUCCESS if the text is set successfully, otherwise ERROR
* @ALinkJoin    usmthmdlDim_getTextC
* @Group        "Dimension Functions"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT StatusInt     mdlDim_setText
(
EditElementHandleR pElementIn,
DimMLTextP pText,
int         segmentNo
);

/*---------------------------------------------------------------------------------**//**
  Get text from given segment.
*
* @param        pText       OUT is the text context from segment
* @param        pElementIn  IN is the dimension element containing the segment to get the text.
* @param        segmentNo   IN is the segment on which to get the text.
* @return       SUCCESS if the text is get successfully, otherwise ERROR
* @ALinkJoin    usmthmdlDim_setTextC
* @Group        "Dimension Functions"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT StatusInt     mdlDim_getText
(
DimMLTextP pText,
ElementHandleCR pElementIn,
int          segmentNo
);

/*---------------------------------------------------------------------------------**//**
  Deletes multiline text from given segment
*
* @param        pElementIn  IN is the dimension element containing the segment to delete the text.
* @param        segmentNo   IN is the segment on which to delete the text.
* @return       SUCCESS if the text is deleted successfully, otherwise ERROR
* @ALinkJoin    usmthmdlDim_setTextC
* @Group        "Dimension Functions"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT StatusInt     mdlDim_deleteText
(
EditElementHandleR pElementIn,
int       segmentNo
);

/*---------------------------------------------------------------------------------**//**
  Creates multiline text holder for dimension.
* @param        ppText      IN pointer to be allocated
* @return       SUCCESS if the text holder can be allocated, otherwise ERROR
* @ALinkJoin    usmthmdlDimText_free
* @Group        "Dimension Functions"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT StatusInt     mdlDimText_create
(
DimMLTextP *ppText
);

/*---------------------------------------------------------------------------------**//**
  Releases multiline text holder for dimension.
* @param        ppText      IN pointer to be freed
* @return       SUCCESS if the text holder is valid and can be freed, otherwise ERROR
* @ALinkJoin    usmthmdlDimText_create
* @Group        "Dimension Functions"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT StatusInt     mdlDimText_free
(
DimMLTextP *ppText
);

/*---------------------------------------------------------------------------------**//**
  Scale multiline text holder components by uniform scale.
*
* @param        pText    In MLText context to scale
* @param        scaleIn  IN scale factor
* @return       SUCCESS if the text holder is valid and can be scaled, otherwise ERROR
* @Group        "Dimension Functions"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT StatusInt     mdlDimText_scale
(
DimMLTextP pText,
double      scaleIn
);

/*---------------------------------------------------------------------------------**//**
  Set text for dimension using text node descriptor.
* @param        pText           IN text holder
* @param        pNodeDescrIn    IN text node descriptor with text and formatting
* @param        bContainsValuePlaceHolder    IN text string includes value placeholder
* @return       SUCCESS if the text holder is valid and strings can be set.
* @Group        "Dimension Functions"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public          StatusInt     mdlDimText_setStringsFromTextNode
(
DimMLTextP      pText,
ElementHandleCR    textNodeElement,
bool            bContainsValuePlaceHolder
);

/*-----------------------------------------------------------------------**//**
* Sets the dimension text prefix strings from the contents of
* the specified text node element.
* @param pText IN the multi-line dimension text element to set the prefix
* strings on.
* @param pNodeDescrIn IN the text node containing the prefix strings to set
* @remarks This function assumes the text in the text node is positioned in
* the manner that it is to be placed into the multi-line text element.
* @return SUCCESS unless an error occurs.
* @Group "Dimension Functions"
* @ALinkJoin usmthmdlDimText_setSuffixStringsFromTextNodeC
* @bsimethod
+---------------+---------------+---------------+---------------+------------*/
DGNPLATFORM_EXPORT StatusInt     mdlDimText_setPrefixStringsFromTextNode
(
DimMLTextP pText,
ElementHandleCR    textNodeElement
);

/*-----------------------------------------------------------------------**//**
* Sets the dimension text suffix strings from the contents of
* the specified text node element.
* @param pText IN the multi-line dimension text element to set the suffix
* strings on.
* @param pNodeDescrIn IN the text node containing the suffix strings to set
* @remarks This function assumes the text in the text node is positioned in
* the manner that it is to be placed into the multi-line text element.
* @return SUCCESS unless an error occurs.
* @Group "Dimension Functions"
* @ALinkJoin usmthmdlDimText_setPrefixStringsFromTextNodeC
* @bsimethod
+---------------+---------------+---------------+---------------+------------*/
DGNPLATFORM_EXPORT StatusInt     mdlDimText_setSuffixStringsFromTextNode
(
DimMLTextP pText,
ElementHandleCR    textNodeElement
);

/*------------------------------------------------------------------------*//**
* @bsimethod
+--------------+--------------+---------------+----------------+-------------*/
DGNPLATFORM_EXPORT void     mdlDim_createAndInsertOptionBlock
(
EditElementHandleR dimElmP,
RotMatrixP rMatrixP
);

/*------------------------------------------------------------------------*//**
* @bsimethod
+--------------+--------------+---------------+----------------+-------------*/
DGNPLATFORM_EXPORT bool     mdlDim_isLinearDimension
(
ElementHandleCR dimElement
);

/*------------------------------------------------------------------------*//**
* @bsimethod
+--------------+--------------+---------------+----------------+-------------*/
DGNPLATFORM_EXPORT bool     mdlDim_isAngularDimension
(
ElementHandleCR dimElement
);

/*------------------------------------------------------------------------*//**
* @bsimethod
+--------------+--------------+---------------+----------------+-------------*/
DGNPLATFORM_EXPORT bool     mdlDim_isRadialDimension
(
ElementHandleCR dimElement
);

/*------------------------------------------------------------------------*//**
* @bsimethod
+--------------+--------------+---------------+----------------+-------------*/
DGNPLATFORM_EXPORT bool     mdlDim_isNoteDimension
(
ElementHandleCR dimElement
);

/*------------------------------------------------------------------------*//**
* @bsimethod
+--------------+--------------+---------------+----------------+-------------*/
DGNPLATFORM_EXPORT bool     mdlDim_isOrdinateDimension
(
ElementHandleCR dimElement
);

/*------------------------------------------------------------------------*//**
* @bsimethod
+--------------+--------------+---------------+----------------+-------------*/
DGNPLATFORM_EXPORT bool     mdlDim_isLabelLineDimension
(
ElementHandleCR dimElement
);

/*------------------------------------------------------------------------*//**
* @bsimethod
+--------------+--------------+---------------+----------------+-------------*/
DGNPLATFORM_EXPORT int      mdlDim_getTextPointNo
(
int                 *pointNo,
ElementHandleCR dim,
int                  segNo
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/03
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT StatusInt    mdlDim_setOverridesDirect
(
EditElementHandleR     dimElement,         /* <=> */
DimStylePropMaskP   shieldFlags,        /* <=> */
bool                applyStyle          /*  => */
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/04
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT StatusInt    mdlDim_isWitnessOnInStyle
(
DgnDimStyleCP       dimStyle,       /* => */
ElementHandleCR pDimElm,        /* => */
int                 iPoint          /* => */
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/03
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT DimDerivedDataP  mdlDimDerivedData_create
(
unsigned short          flags           //  =>
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/03
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT void     mdlDimDerivedData_free
(
DimDerivedDataP *pDerivedData           //  =>
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/03
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT StatusInt        mdlDim_getDerivedData
(
DimDerivedDataP pDerivedData,      // <=
ElementHandleCR    element            // =>
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/03
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT StatusInt        mdlDimDerivedData_getTermDir
(
DPoint3dP  pDir,                  // <=
DimDerivedDataP derivedData,            //  =>
bool            left,                   //  => true = left, false = right
int             iSegment                //  =>
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/03
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT StatusInt        mdlDimDerivedData_getIsTextDocked
(
bool            *pIsTextDocked,         // <=
DimDerivedDataP derivedData,            //  =>
int             iSegment                //  =>
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/04
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT StatusInt    mdlDimDerivedData_getArcDefPoint
(
DimDerivedDataP     pDerivedData,           // =>
DPoint3dP  pArcDefPoint            // <=
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    04/04
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT StatusInt    mdlDimDerivedData_getTextBox
(
DimDerivedDataP     pDerivedData,           //  =>
DPoint3dP           pOrigin,                //  <=
DPoint3dP           pXVec,                  //  <=
DPoint3dP           pZVec,                  //  <=
DPoint2dP           pSize,                  //  <=
bool                primary,                //  =>
int                 iSegment                //  =>
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          03/03
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT void         mdlDim_initDimTextFromStyle
(
DimTextP            dimTextP,
DimensionStyleCP    dgnDimStyleP
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar 09/03
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT Dgn::DimensionType    adim_getTemplateNumber
(
ElementHandleCR dimElement
);

DGNPLATFORM_EXPORT StatusInt        mdlDimLinkage_convertFromPointLinkage
(
Dgn::DimPointOverrides *   pOverridesOut,
LinkageHeaderP          pLinkageIn
);

DGNPLATFORM_EXPORT StatusInt        mdlDimLinkage_convertFromSegmentLinkage
(
Dgn::DimSegmentOverrides *   pOverridesOut,
LinkageHeaderP          pLinkageIn
);

DGNPLATFORM_EXPORT StatusInt        mdlDimLinkage_convertFromOverallLinkage
(
Dgn::DimOverallOverrides *   pOverridesOut,
LinkageHeaderP          pLinkageIn
);

DGNPLATFORM_EXPORT StatusInt        mdlDimLinkage_convertFromStyleExtensionLinkage
(
Dgn::DimStyleExtensions *    pOverridesOut,
LinkageHeaderP          pLinkageIn
);

DGNPLATFORM_EXPORT StatusInt        mdlDimLinkage_convertFromOrdinateLinkage
(
Dgn::DimOrdinateOverrides *  pOverridesOut,
LinkageHeaderP          pLinkageIn
);

/*------------------------------------------------------------------------*//**
   The mdlDim_getStyleExtension function is used to retrieve style extensions from
the specified dimension element.
* @Param        pExtensionsOut OUT the style extensions retrieved from the dimension
element.
* @Param        pElementIn IN is the dimension element to retrieve the extensions from.
* @Return       SUCCESS if the extensions information is retrieved successfully.
* @ALinkJoin    usmthmdlDim_deleteStyleExtensionC usmthmdlDim_setStyleExtensionC
* @Group        "Dimension Functions"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT StatusInt    mdlDim_getStyleExtension
(
Dgn::DimStyleExtensions*    pExtensionsOut,
ElementHandleCR                     dimElement
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BentleySystems  03/01
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT uint64_t mdlDim_getDimStyleID
(
ElementHandleCR pElementIn
);

DGNPLATFORM_EXPORT  void adim_initDimText (ElementHandleCR dimElement, int, Dgn::DimText*, DimensionStyleCR);

END_BENTLEY_NAMESPACE

//__PUBLISH_SECTION_START__

#endif
/** @endcond */
