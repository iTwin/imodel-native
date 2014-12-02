/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnHandlers/Dimension/DimStyleInternal.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

BEGIN_EXTERN_C

/*----------------------------------------------------------------------+
|                                                                       |
|       Overall override flags                                          |
|                                                                       |
+----------------------------------------------------------------------*/
#define     OVERALL_Override_RefScale                   (0x1<<0)  // dimOverallOverrides.refScale present
#define     OVERALL_Override_AngleQuadrant              (0x1<<2)  // dimOverallOverrides.angleQuadrant present
#define     OVERALL_Override_SlantAngle                 (0x1<<3)  // dimOverallOverrides.slantAngle present
#define     OVERALL_Override_ModelAnnotationScale       (0x1<<4)  // dimOverallOverrides.dModelAnnotationScale present

/*----------------------------------------------------------------------+
|                                                                       |
|       Point override flags                                            |
|                                                                       |
+----------------------------------------------------------------------*/
#define     POINT_Override_WitnessColor             (0x1<<0)      // dimPointOverrides.color  present
#define     POINT_Override_WitnessWeight            (0x1<<1)      // dimPointOverrides.weight present
#define     POINT_Override_WitnessStyle             (0x1<<2)      // dimPointOverrides.style  present
#define     POINT_Override_WitnessOffset            (0x1<<3)      // dimPointOverrides.witnessOffset present
#define     POINT_Override_WitnessExtend            (0x1<<4)      // dimPointOverrides.witnessExtend present

/*----------------------------------------------------------------------+
|                                                                       |
|       Segment overrides                                               |
|                                                                       |
+----------------------------------------------------------------------*/
#define     SEGMENT_Override_TextRotation           (0x1<<0)    // dimSegmentOverrides.textRotation present
#define     SEGMENT_Override_TextJustification      (0x1<<1)    // dimSegmentOverrides.textJustification present
#define     SEGMENT_Override_CurveStartTangent      (0x1<<2)    // dimSegmentOverrides.curveStartTangent present
#define     SEGMENT_Override_CurveEndTangent        (0x1<<3)    // dimSegmentOverrides.curveEndTangent present

/*----------------------------------------------------------------------+
|                                                                       |
|       Segment override flags                                          |
|                                                                       |
+----------------------------------------------------------------------*/
#define     SEGMENTFLAG_Override_UnderlineText          (0x1<<0)
#define     SEGMENTFLAG_Override_SuppressLeftDimLine    (0x1<<1)
#define     SEGMENTFLAG_Override_SuppressRightDimLine   (0x1<<2)
#define     SEGMENTFLAG_Override_PrimaryIsReference     (0x1<<3)
#define     SEGMENTFLAG_Override_SecondaryIsReference   (0x1<<4)

/*----------------------------------------------------------------------+
|                                                                       |
|       Tool Overrides                                                  |
|                                                                       |
+----------------------------------------------------------------------*/
#define     ORDINATE_Override_StartValueX               (0x1<<0)    // dimOrdinateOverrides.startValueX present
#define     ORDINATE_Override_StartValueY               (0x1<<1)    // dimOrdinateOverrides.startValueY present

/*---------------------------------------------------------------------------------------

    Unpublished dimension style functions. Functions that are intended to be
    public should be declared in msdimstyle.fdf in pubinc.

---------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BentleySystems  11/00
+---------------+---------------+---------------+---------------+---------------+------*/
int   dimStyleEntry_isStyleElement
(
DgnElementCP pCandidate
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    DonFu           06/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt     dimStyleTable_createDirect
(
MSElementDescrH ppTableEd,
ElementId               tableElementIdIn,
DgnModelP            pDgnModelIn
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    10/01
+---------------+---------------+---------------+---------------+---------------+------*/
Public void     mdlDimStyle_importFromDgnModel
(
int            *pCopied,
DgnModelP    srcDgnModel,
DgnModelP    destDgnModel
);

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    BentleySystems  11/00
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt     dimStyleEntry_setId
(
MSElementDescrP pEntryDescrIn,
UInt32          entryIdIn
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BentleySystems  11/00
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt     dimStyleEntry_setDescription
(
MSElementDescrH ppStyleEd,
const WChar    *pDescriptionIn
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BentleySystems  11/00
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt     dimStyleEntry_create
(
MSElementDescrH ppStyleEd,
MSElementDescrP pTemplateEd,
WChar         *pStyleName,
WChar         *pStyleDescription,
MSElementDescrP pTableEd,
bool             getActive
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BentleySystems  11/00
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt     dimStyleEntry_delete
(
MSElementDescrP pEntryDescrIn
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BentleySystems  12/00
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt     mdlDimStyle_makeDefaultStyleActive
(
void
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BentleySystems  04/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt dimStyleTable_getDgnModel
(
DgnModelP    *pDgnModelOut,          /* <= output modelRef */
MSElementDescrP pStyleTableDescrIn     /* => input style table element descriptor */
);


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/06
+---------------+---------------+---------------+---------------+---------------+------*/
Public StatusInt    dimStyleEntry_findByUniqueIdInFile
(
MSElementDescrH ppStyleEdP,         /* <= */
ElementId           elementID,          /* => */
DgnProjectP         fileObj             /* => */
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    08/03
+---------------+---------------+---------------+---------------+---------------+------*/
Public MSElementDescrP   dgnDimStyle_createElement
(
DgnDimStyleCP           dgnDimStyleP,
const DgnModelP      destDgnModel
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  10/01
+---------------+---------------+---------------+---------------+---------------+------*/
Public void     dgnDimStyle_init
(
DgnDimStyleP    dgnDimStyleP
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    11/01
+---------------+---------------+---------------+---------------+---------------+------*/
Public StatusInt        dgnDimStyle_applyUorScale
(
DimStyleSettings   *dsP,
double              uorScale
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    11/01
+---------------+---------------+---------------+---------------+---------------+------*/
Public StatusInt        dgnDimStyle_scaleBetweenModels
(
DgnDimStyleP    dgnDimStyleP,
DgnModelP       srcCache,
DgnModelP       dstCache
);


/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    BentleySystems  11/00
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt     dimStyleEntry_findById
(
MSElementDescrH ppStyleEd,
UInt32          entryId,
MSElementDescrP pTableEd
);

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    BentleySystems  11/00
+---------------+---------------+---------------+---------------+---------------+------*/
 StatusInt     dimStyleEntry_getId
(
UInt32                 *pEntryIdOut,
MSElementDescrCP pEd
);

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    BentleySystems  11/00
+---------------+---------------+---------------+---------------+---------------+------*/
 StatusInt     dimStyleEntry_findByUniqueId
(
MSElementDescrH ppStyleEd,
ElementId           entryId,
MSElementDescrP pTableEd
);

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    BentleySystems  11/00
+---------------+---------------+---------------+---------------+---------------+------*/
 StatusInt     dimStyleTable_create
(
MSElementDescrH ppTableEd,
DgnModelP        modelRefIn,
ElementId           tableElementIdIn
);


END_EXTERN_C

