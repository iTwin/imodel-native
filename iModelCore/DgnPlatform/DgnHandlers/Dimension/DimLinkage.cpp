/*----------------------------------------------------------------------+
|
|   $Source: DgnHandlers/Dimension/DimLinkage.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+----------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>
#include    "DimStyleInternal.h"


USING_NAMESPACE_BENTLEY_DGNPLATFORM

/*----------------------------------------------------------------------+
|                                                                       |
|   Local Definitions                                                   |
|                                                                       |
+----------------------------------------------------------------------*/
#define MIN(a,b)    ((a)<(b)?(a):(b))

    /**
     *  It is very unlikely that dimension text will ever reach MAX_TEXT_LENGTH limit.
     *  Since MAX_TEXT_LENGTH is such big junk of data reduce the number to about 2k.
     */
#define MAX_SINGLE_FORMATTED_TEXT_LENGTH    2048

/*---------------------------------------------------------------------------------------
    Linkage subtypes
---------------------------------------------------------------------------------------*/
#define LINKAGESUBTYPE_OverallOverride      0
#define LINKAGESUBTYPE_SegmentOverride      1
#define LINKAGESUBTYPE_PointOverride        2
#define LINKAGESUBTYPE_SegmentFlagOverride  3
#define LINKAGESUBTYPE_OrdinateOverride     4
#define LINKAGESUBTYPE_FormattedText        5
#define LINKAGESUBTYPE_StyleExtension       6

#define MAX_LinkageSubTypes                 7

#define DIMTEXTNODEOPTIONS_HasPlaceHolder   (0x0001 << 0)
#define DIMTEXTNODEOPTIONS_NoTolStrings     (0x0001 << 1)

/*---------------------------------------------------------------------------------------
    Override linkage structures
---------------------------------------------------------------------------------------*/
typedef struct secondaryhdr
    {
    UInt16      type;
    UInt16      size;
    } SecondaryHdr;

typedef struct dimextlinkagehdr
    {
    LinkageHeader   hdr;
    SecondaryHdr    shdr;
    } DimExtLinkageHdr;

typedef struct dimpointlinkage
    {
    LinkageHeader   hdr;
    SecondaryHdr    shdr;
    UInt16          point;
    UInt32          modifiers;                  // This will tell what is saved in linkage
    byte            metaData[sizeof(DimPointOverrides)];
    } DimPointLinkage;

typedef struct dimsegmentlinkage
    {
    LinkageHeader   hdr;
    SecondaryHdr    shdr;
    UInt16          segment;
    UInt32          modifiers;                  // This will tell what is saved in linkage
    byte            metaData[sizeof(DimSegmentOverrides)];
    } DimSegmentLinkage;

typedef struct dimsegmentflaglinkage
    {
    LinkageHeader   hdr;
    SecondaryHdr    shdr;
    UInt16          segment;
    UInt32          modifiers;                  // This will tell what is saved in linkage
    } DimSegmentFlagLinkage;

typedef struct dimoveralllinkage
    {
    LinkageHeader   hdr;
    SecondaryHdr    shdr;
    UInt16          unused;
    UInt32          modifiers;                  // This will tell what is saved in linkage
    byte            metaData[sizeof(DimOverallOverrides)];
    } DimOverallLinkage;

typedef struct dimstyleextensionlinkage
    {
    LinkageHeader   hdr;
    SecondaryHdr    shdr;
    UInt16          unused;
    UInt32          modifiers;                  // This will tell what is saved in linkage
    byte            metaData[sizeof(DimStyleExtensions)];
    } DimStyleExtensionLinkage;

typedef struct dimordinatelinkage
    {
    LinkageHeader   hdr;
    SecondaryHdr    shdr;
    UInt16          unused;
    UInt32          modifiers;                  // This will tell what is saved in linkage
    byte            metaOrdinate[sizeof(DimOrdinateOverrides)];
    } DimOrdinateLinkage;

typedef struct dimformattedtextlinkage
    {
    LinkageHeader   hdr;
    SecondaryHdr    shdr;
    UInt16          segment;
    UInt32          modifiers;                  // This will tell what is saved in linkage
    byte            meta[4];
    } DimFormattedTextLinkage;

/*----------------------------------------------------------------------+
|                                                                       |
|       Function prototypes                                             |
|                                                                       |
+----------------------------------------------------------------------*/
static StatusInt   dimlinkage_extractPointOverridesByPointNo
(
UInt16              **ppLinkageOut,
DimPointOverrides   *pOverridesOut,
int                 *pIndexOut,
ElementHandleCR        dimElement,
int                 pointNo
);

static StatusInt   dimlinkage_extractSegmentOverridesBySegmentNo
(
UInt16              **ppLinkageOut,
DimSegmentOverrides *pOverridesOut,
int                 *pIndexOut,
ElementHandleCR        dimElement,
int                 segmentNo
);

static StatusInt   dimlinkage_extractSegmentFlagOverridesBySegmentNo
(
UInt16                  **ppLinkageOut,
DimSegmentFlagOverrides *pOverridesOut,
int                     *pIndexOut,
ElementHandleCR            dimElement,
int                     segmentNo
);

static StatusInt   dimlinkage_extractOverallOverrides
(
UInt16              **ppLinkageOut,
DimOverallOverrides *pOverridesOut,
ElementHandleCR        dimElement,
int                 *pIndexOut
);

static StatusInt       dimlinkage_extractStyleExtensions
(
UInt16              **ppLinkageOut,
DimStyleExtensions  *pExtensionsOut,
ElementHandleCR        dimElement,
int                 *pIndexOut
);

static StatusInt       dimlinkage_extractOrdinateOverrides
(
UInt16              **ppLinkageOut,
DimOrdinateOverrides *pOverridesOut,
ElementHandleCR        dimElement,
int                 *pIndexOut
);

static StatusInt       dimlinkage_extractNthTextFormatBySegmentNo
(
UInt16            **ppLinkageOut,
DimFormattedText  **ppTextFormatOut,
int                *pIndexOut,
ElementHandleCR        dimElement,
int                 segmentNo,
int                 nth
);

/*----------------------------------------------------------------------+
|                                                                       |
|   External Functions                                                  |
|                                                                       |
+----------------------------------------------------------------------*/
/*----------------------------------------------------------------------+
|                                                                       |
|   External Data                                                       |
|                                                                       |
+----------------------------------------------------------------------*/

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  02/01
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    isValidDimension
(
ElementHandleCR     dimElement
)
    {
    return  (dimElement.IsValid() && DIMENSION_ELM == dimElement.GetLegacyType());
    }

/*---------------------------------------------------------------------------------**//**
* Test whether the input element and point index are valid.
*
* @param        dimElement      <=>
* @param        pointNo         =>
* @return       true if valid
* @bsimethod                                                    petri.niiranen  02/01
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    isValidDimensionAndPoint
(
ElementHandleCR dimElement,
int         pointNo
)
    {
    return  (isValidDimension (dimElement) &&
             pointNo < dimElement.GetElementCP()->ToDimensionElm().nPoints &&
             pointNo >  -1);
    }

/*---------------------------------------------------------------------------------**//**
* Test whether the input element and segment index are valid.
*
* @param        dimElement      <=>
* @param        segmentNo       =>
* @return       true if valid
* @bsimethod                                                    petri.niiranen  02/01
+---------------+---------------+---------------+---------------+---------------+------*/
static bool     isValidDimensionAndSegment
(
ElementHandleCR elementIn,
int             segmentNo
)
    {
    if (!isValidDimension (elementIn))
        return false;

    int numSegments = DimensionHandler::GetInstance().GetNumSegments (elementIn);

    return (0 <= segmentNo) && (numSegments > segmentNo);
    }

/*=================================================================================**//**
*
* Data conversion functions
*
+===============+===============+===============+===============+===============+======*/
/*---------------------------------------------------------------------------------**//**
* Converts override data to file format.
*
* @param        pBuf        <= raw format data
* @param        pOverrides  => override data.
* @return       raw data pointer.
* @bsimethod                                                    petri.niiranen  02/01
+---------------+---------------+---------------+---------------+---------------+------*/
static byte    *dimlinkage_appendPointData
(
byte                *pBuf,
DimPointOverrides   *pOverrides
)
    {
    byte    *pMod = NULL;

    /*-----------------------------------------------------------------------------------
        Modifiers will be written last.
    -----------------------------------------------------------------------------------*/
    pMod = pBuf;
    pBuf += sizeof (long); // ***NEEDS WORK sizeof(long) varies. 

    if (pOverrides->modifiers & POINT_Override_WitnessColor)
        ByteStreamHelper::AppendUInt32 (pBuf, pOverrides->color);

    if (pOverrides->modifiers & POINT_Override_WitnessWeight)
        ByteStreamHelper::AppendUInt32 (pBuf, pOverrides->weight);

    if (pOverrides->modifiers & POINT_Override_WitnessStyle)
        ByteStreamHelper::AppendInt32 (pBuf, pOverrides->style);

    if (pOverrides->modifiers & POINT_Override_WitnessOffset)
        ByteStreamHelper::AppendDouble (pBuf, pOverrides->witnessOffset);

    if (pOverrides->modifiers & POINT_Override_WitnessExtend)
        ByteStreamHelper::AppendDouble (pBuf, pOverrides->witnessExtend);

    /* TBD ... */

    /*-----------------------------------------------------------------------------------
        Save modifiers.
    -----------------------------------------------------------------------------------*/
    ByteStreamHelper::AppendUInt32 (pMod, pOverrides->modifiers);

    return  pBuf;
    }

/*---------------------------------------------------------------------------------**//**
* Converts override data from file format.
*
* @param        pOverrides      <= override data
* @param        pBuf            => raw data
* @bsimethod                                                    petri.niiranen  02/01
+---------------+---------------+---------------+---------------+---------------+------*/
static void    dimlinkage_extractPointData
(
DimPointOverrides   *pOverrides,
byte const *&       pBuf
)
    {
    ByteStreamHelper::ExtractUInt32 (pOverrides->modifiers, pBuf);

    if (pOverrides->modifiers & POINT_Override_WitnessColor)
        ByteStreamHelper::ExtractUInt32 (pOverrides->color, pBuf);

    if (pOverrides->modifiers & POINT_Override_WitnessWeight)
        ByteStreamHelper::ExtractUInt32 (pOverrides->weight, pBuf);

    if (pOverrides->modifiers & POINT_Override_WitnessStyle)
        ByteStreamHelper::ExtractInt32 (pOverrides->style, pBuf);

    if (pOverrides->modifiers & POINT_Override_WitnessOffset)
        ByteStreamHelper::ExtractDouble (pOverrides->witnessOffset, pBuf);

    if (pOverrides->modifiers & POINT_Override_WitnessExtend)
        ByteStreamHelper::ExtractDouble (pOverrides->witnessExtend, pBuf);

    /* TBD ... */
    }

/*---------------------------------------------------------------------------------**//**
* Converts override data to file format.
*
* @param        pBuf        <= raw format data
* @param        pOverrides  => override data.
* @return       raw data pointer.
* @bsimethod                                                    petri.niiranen  02/01
+---------------+---------------+---------------+---------------+---------------+------*/
static byte    *dimlinkage_appendSegmentData
(
byte                *pBuf,
DimSegmentOverrides *pOverrides
)
    {
    byte    *pMod = NULL;

    /*-----------------------------------------------------------------------------------
        Modifiers will be written last.
    -----------------------------------------------------------------------------------*/
    pMod = pBuf;
    pBuf += sizeof (long);

    if (pOverrides->modifiers & SEGMENT_Override_TextRotation)
        ByteStreamHelper::AppendDouble (pBuf, pOverrides->textRotation);

    if (pOverrides->modifiers & SEGMENT_Override_TextJustification)
        ByteStreamHelper::AppendULong (pBuf, pOverrides->textJustification);

    if (pOverrides->modifiers & SEGMENT_Override_CurveStartTangent)
        ByteStreamHelper::AppendDPoint3d (pBuf, pOverrides->curveStartTangent);

    if (pOverrides->modifiers & SEGMENT_Override_CurveEndTangent)
        ByteStreamHelper::AppendDPoint3d (pBuf, pOverrides->curveEndTangent);

/* TBD ... */

    /*-----------------------------------------------------------------------------------
        Save modifiers.
    -----------------------------------------------------------------------------------*/
    ByteStreamHelper::AppendUInt32 (pMod, pOverrides->modifiers);

    return  pBuf;
    }

/*---------------------------------------------------------------------------------**//**
* Converts override data from file format.
*
* @param        pOverrides      <= override data
* @param        pBuf            => raw data
* @bsimethod                                                    petri.niiranen  02/01
+---------------+---------------+---------------+---------------+---------------+------*/
static void    dimlinkage_extractSegmentData
(
DimSegmentOverrides *pOverrides,
byte const *&       pBuf
)
    {
    ByteStreamHelper::ExtractUInt32 (pOverrides->modifiers, pBuf);

    if (pOverrides->modifiers & SEGMENT_Override_TextRotation)
        ByteStreamHelper::ExtractDouble (pOverrides->textRotation, pBuf);

    if (pOverrides->modifiers & SEGMENT_Override_TextJustification)
        {
        long    lngJust;

        ByteStreamHelper::ExtractLong (lngJust, pBuf);
        pOverrides->textJustification = (short) lngJust;
        }

    if (pOverrides->modifiers & SEGMENT_Override_CurveStartTangent)
        ByteStreamHelper::ExtractDPoint3d (pOverrides->curveStartTangent, pBuf);

    if (pOverrides->modifiers & SEGMENT_Override_CurveEndTangent)
        ByteStreamHelper::ExtractDPoint3d (pOverrides->curveEndTangent, pBuf);

/* TBD ... */
    }

/*---------------------------------------------------------------------------------**//**
* Converts override data to file format.
*
* @param        pBuf        <= raw format data
* @param        pOverrides  => override data.
* @return       raw data pointer.
* @bsimethod                                                    petri.niiranen  02/01
+---------------+---------------+---------------+---------------+---------------+------*/
static byte    *dimlinkage_appendSegmentFlagData
(
byte                    *pBuf,
DimSegmentFlagOverrides *pOverrides
)
    {
    byte    *pMod = NULL;

    /*-----------------------------------------------------------------------------------
        Modifiers will be written last.
    -----------------------------------------------------------------------------------*/
    pMod = pBuf;
    pBuf += sizeof (long);

    /*-----------------------------------------------------------------------------------
        Save modifiers. Flags have only modifiers field.
    -----------------------------------------------------------------------------------*/
    ByteStreamHelper::AppendUInt32 (pMod, pOverrides->modifiers);

    return  pBuf;
    }

/*---------------------------------------------------------------------------------**//**
* Converts override data from file format.
*
* @param        pOverrides      <= override data
* @param        pBuf            => raw data
* @bsimethod                                                    petri.niiranen  02/01
+---------------+---------------+---------------+---------------+---------------+------*/
static void    dimlinkage_extractSegmentFlagData
(
DimSegmentFlagOverrides *pOverrides,
byte const *&           pBuf
)
    {
    ByteStreamHelper::ExtractUInt32 (pOverrides->modifiers, pBuf);
    }

/*---------------------------------------------------------------------------------**//**
* Converts override data to file format.
*
* @param        pBuf        <= raw format data
* @param        pOverrides  => override data.
* @return       raw data pointer.
* @bsimethod                                                    petri.niiranen  02/01
+---------------+---------------+---------------+---------------+---------------+------*/
static byte    *dimlinkage_appendOverallData
(
byte                *pBuf,
DimOverallOverrides *pOverrides
)
    {
    byte    *pMod = NULL;

    /*-----------------------------------------------------------------------------------
        Modifiers will be written last.
    -----------------------------------------------------------------------------------*/
    pMod = pBuf;
    pBuf += sizeof (long);

    if (pOverrides->modifiers & OVERALL_Override_RefScale)
        ByteStreamHelper::AppendDouble (pBuf, pOverrides->refScale);

    if (pOverrides->modifiers & OVERALL_Override_AngleQuadrant)
        ByteStreamHelper::AppendUInt16 (pBuf, (short) pOverrides->angleQuadrant);

    if (pOverrides->modifiers & OVERALL_Override_SlantAngle)
        ByteStreamHelper::AppendDouble (pBuf, pOverrides->slantAngle);

    if (pOverrides->modifiers & OVERALL_Override_ModelAnnotationScale)
        ByteStreamHelper::AppendDouble (pBuf, pOverrides->dModelAnnotationScale);

    /*-----------------------------------------------------------------------------------
        Save modifiers.
    -----------------------------------------------------------------------------------*/
    ByteStreamHelper::AppendUInt32 (pMod, pOverrides->modifiers);

    return  pBuf;
    }

/*---------------------------------------------------------------------------------**//**
* Converts override data to file format.
*
* @param        pBuf        <= raw format data
* @param        pOverrides  => override data.
* @return       raw data pointer.
* @bsimethod                                                    petri.niiranen  02/01
+---------------+---------------+---------------+---------------+---------------+------*/
static byte    *dimlinkage_appendStyleExtensionData
(
byte*                       pBuf,
DimStyleExtensions const*   pExtensions
)
    {
    byte    *pMod = NULL;

    /*-----------------------------------------------------------------------------------
        Modifiers will be written last.
    -----------------------------------------------------------------------------------*/
    pMod = pBuf;
    pBuf += sizeof (long);

    if (pExtensions->modifiers & STYLE_Extension_PrimaryToleranceAccuracy)
        ByteStreamHelper::AppendUInt16 (pBuf, pExtensions->primaryTolAcc);

    if (pExtensions->modifiers & STYLE_Extension_SecondaryToleranceAccuracy)
        ByteStreamHelper::AppendUInt16 (pBuf, pExtensions->secondaryTolAcc);

    if (pExtensions->modifiers & STYLE_Extension_RoundOff)
        ByteStreamHelper::AppendDouble (pBuf, pExtensions->dRoundOff);

    if (pExtensions->modifiers & STYLE_Extension_SecondaryRoundOff)
        ByteStreamHelper::AppendDouble (pBuf, pExtensions->dSecondaryRoundOff);

    if (pExtensions->modifiers & STYLE_Extension_OrdinateDatumValue)
        ByteStreamHelper::AppendDouble (pBuf, pExtensions->dOrdinateDatumValue);

    if (pExtensions->modifiers & STYLE_Extension_Flags)
        ByteStreamHelper::AppendUShort (pBuf, *(UShort*)&pExtensions->flags);

    if (pExtensions->modifiers & STYLE_Extension_StackedFractionScale)
        ByteStreamHelper::AppendDouble (pBuf, pExtensions->stackedFractionScale);

    if (pExtensions->modifiers & STYLE_Extension_NoteElbowLength)
        ByteStreamHelper::AppendDouble (pBuf, pExtensions->dNoteElbowLength);

    if (pExtensions->modifiers & STYLE_Extension_NoteLeftMargin)
        ByteStreamHelper::AppendDouble (pBuf, pExtensions->dNoteLeftMargin);

    if (pExtensions->modifiers & STYLE_Extension_NoteLowerMargin)
        ByteStreamHelper::AppendDouble (pBuf, pExtensions->dNoteLowerMargin);

    if (pExtensions->modifiers & STYLE_Extension_Flags2)
        ByteStreamHelper::AppendULong (pBuf, *(ULong*)&pExtensions->flags2);

    if (pExtensions->modifiers & STYLE_Extension_NoteTerminatorFont)
        ByteStreamHelper::AppendUInt32 (pBuf, pExtensions->iNoteTerminatorFont);

    if (pExtensions->modifiers & STYLE_Extension_NoteTerminatorChar)
        ByteStreamHelper::AppendUInt16 (pBuf, pExtensions->iNoteTerminatorChar);

    if (pExtensions->modifiers & STYLE_Extension_AnnotationScale)
        ByteStreamHelper::AppendDouble (pBuf, pExtensions->dAnnotationScale);

    if (pExtensions->modifiers & STYLE_Extension_InlineTextLift)
        ByteStreamHelper::AppendDouble (pBuf, pExtensions->dInlineTextLift);

    if (pExtensions->modifiers & STYLE_Extension_Dwg_Flags)
        ByteStreamHelper::AppendUShort (pBuf, *(UShort*)&pExtensions->dwgSpecifics.flags);

    if (pExtensions->modifiers & STYLE_Extension_BncElbowLength)
        ByteStreamHelper::AppendDouble (pBuf, pExtensions->dBncElbowLength);

    /* outside min leader is now unused, but reserve it for possible future re-use */
    if (pExtensions->modifiers & STYLE_Extension_OutsideMinLeader_Reserved)
        ByteStreamHelper::AppendDouble (pBuf, pExtensions->dOutsideMinLeader_Reserved);

    if (pExtensions->modifiers & STYLE_Extension_NoteFrameScale)
        ByteStreamHelper::AppendDouble (pBuf, pExtensions->dNoteFrameScale);

    if (pExtensions->modifiers & STYLE_Extension_Flags3)
        ByteStreamHelper::AppendUInt32 (pBuf, *(UInt32*)&pExtensions->flags3);

    if (pExtensions->modifiers & STYLE_Extension_DirectionMode)
        ByteStreamHelper::AppendUInt16 (pBuf, pExtensions->directionMode);

    if (pExtensions->modifiers & STYLE_Extension_DirectionBaseDir)
        ByteStreamHelper::AppendDouble (pBuf, pExtensions->directionBaseDir);

    /*-----------------------------------------------------------------------------------
        Save modifiers.
    -----------------------------------------------------------------------------------*/
    ByteStreamHelper::AppendUInt32 (pMod, pExtensions->modifiers);

    return  pBuf;
    }

/*---------------------------------------------------------------------------------**//**
* Converts override data from file format.
*
* @param        pOverrides      <=
* @param        pBuf            =>
* @bsimethod                                                    petri.niiranen  02/01
+---------------+---------------+---------------+---------------+---------------+------*/
static void    dimlinkage_extractOverallData
(
DimOverallOverrides *pOverrides,
byte const *&       pBuf
)
    {
    ByteStreamHelper::ExtractUInt32 (pOverrides->modifiers, pBuf);

    if (pOverrides->modifiers & OVERALL_Override_RefScale)
        ByteStreamHelper::ExtractDouble (pOverrides->refScale, pBuf);

    if (pOverrides->modifiers & OVERALL_Override_AngleQuadrant)
        ByteStreamHelper::ExtractUInt16 (pOverrides->angleQuadrant, pBuf);

    if (pOverrides->modifiers & OVERALL_Override_SlantAngle)
        ByteStreamHelper::ExtractDouble (pOverrides->slantAngle, pBuf);

    if (pOverrides->modifiers & OVERALL_Override_ModelAnnotationScale)
        ByteStreamHelper::ExtractDouble (pOverrides->dModelAnnotationScale, pBuf);
    }

/*---------------------------------------------------------------------------------**//**
* Converts style extension data from file format.
*
* @param        pOverrides      <=
* @param        pBuf            =>
* @bsimethod                                                    petri.niiranen  02/01
+---------------+---------------+---------------+---------------+---------------+------*/
static UInt16      dimlinkage_extractStyleExtensionData
(
DimStyleExtensions  *pExtensions,
byte const *&       pBuf
)
    {
    byte const * pBufHead = NULL;

    ByteStreamHelper::ExtractUInt32 (pExtensions->modifiers, pBuf);

    /* only count meta data size */
    pBufHead = pBuf;

    if (pExtensions->modifiers & STYLE_Extension_PrimaryToleranceAccuracy)
        ByteStreamHelper::ExtractUInt16 (pExtensions->primaryTolAcc, pBuf);

    if (pExtensions->modifiers & STYLE_Extension_SecondaryToleranceAccuracy)
        ByteStreamHelper::ExtractUInt16 (pExtensions->secondaryTolAcc, pBuf);

    if (pExtensions->modifiers & STYLE_Extension_RoundOff)
        ByteStreamHelper::ExtractDouble (pExtensions->dRoundOff, pBuf);

    if (pExtensions->modifiers & STYLE_Extension_SecondaryRoundOff)
        ByteStreamHelper::ExtractDouble (pExtensions->dSecondaryRoundOff, pBuf);

    if (pExtensions->modifiers & STYLE_Extension_OrdinateDatumValue)
        ByteStreamHelper::ExtractDouble (pExtensions->dOrdinateDatumValue, pBuf);

    if (pExtensions->modifiers & STYLE_Extension_Flags)
        ByteStreamHelper::ExtractUShort ((UShort&)pExtensions->flags, pBuf);

    if (pExtensions->modifiers & STYLE_Extension_StackedFractionScale)
        ByteStreamHelper::ExtractDouble (pExtensions->stackedFractionScale, pBuf);

    if (pExtensions->modifiers & STYLE_Extension_NoteElbowLength)
        ByteStreamHelper::ExtractDouble (pExtensions->dNoteElbowLength, pBuf);

    if (pExtensions->modifiers & STYLE_Extension_NoteLeftMargin)
        ByteStreamHelper::ExtractDouble (pExtensions->dNoteLeftMargin, pBuf);

    if (pExtensions->modifiers & STYLE_Extension_NoteLowerMargin)
        ByteStreamHelper::ExtractDouble (pExtensions->dNoteLowerMargin, pBuf);

    if (pExtensions->modifiers & STYLE_Extension_Flags2)
        ByteStreamHelper::ExtractULong ((ULong&)pExtensions->flags2, pBuf);

    if (pExtensions->modifiers & STYLE_Extension_NoteTerminatorFont)
        ByteStreamHelper::ExtractUInt32 (pExtensions->iNoteTerminatorFont, pBuf);

    if (pExtensions->modifiers & STYLE_Extension_NoteTerminatorChar)
        ByteStreamHelper::ExtractUInt16 (pExtensions->iNoteTerminatorChar, pBuf);

    if (pExtensions->modifiers & STYLE_Extension_AnnotationScale)
        ByteStreamHelper::ExtractDouble (pExtensions->dAnnotationScale, pBuf);

    if (pExtensions->modifiers & STYLE_Extension_InlineTextLift)
        ByteStreamHelper::ExtractDouble (pExtensions->dInlineTextLift, pBuf);

    if (pExtensions->modifiers & STYLE_Extension_Dwg_Flags)
        ByteStreamHelper::ExtractUShort ((UShort&)pExtensions->dwgSpecifics.flags, pBuf);

    if (pExtensions->modifiers & STYLE_Extension_BncElbowLength)
        ByteStreamHelper::ExtractDouble (pExtensions->dBncElbowLength, pBuf);

    /* outside min leader is now unused, but reserve it for possible future re-use */
    if (pExtensions->modifiers & STYLE_Extension_OutsideMinLeader_Reserved)
        ByteStreamHelper::ExtractDouble (pExtensions->dOutsideMinLeader_Reserved, pBuf);

    if (pExtensions->modifiers & STYLE_Extension_NoteFrameScale)
        ByteStreamHelper::ExtractDouble (pExtensions->dNoteFrameScale, pBuf);

    if (pExtensions->modifiers & STYLE_Extension_Flags3)
        ByteStreamHelper::ExtractULong ((ULong&)pExtensions->flags3, pBuf);

    if (pExtensions->modifiers & STYLE_Extension_DirectionMode)
        ByteStreamHelper::ExtractUInt16 (pExtensions->directionMode, pBuf);

    if (pExtensions->modifiers & STYLE_Extension_DirectionBaseDir)
        ByteStreamHelper::ExtractDouble (pExtensions->directionBaseDir, pBuf);

    return  static_cast<UInt16>(pBuf - pBufHead);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          07/05
+---------------+---------------+---------------+---------------+---------------+------*/
static UInt16              getStyleExtensionsActualDataSize
(
DimStyleExtensionLinkage    *pLinkageIn
)
    {
    DimStyleExtensions      extensions;
    byte const *            buffer      = (byte const *)&pLinkageIn->modifiers;

    /* count all stored data bytes */
    return  dimlinkage_extractStyleExtensionData(&extensions, buffer);
    }

/*---------------------------------------------------------------------------------**//**
* Converts override data to file format.
*
* @param        pBuf        <= raw format data
* @param        pOverrides  => override data.
* @return       raw data pointer.
* @bsimethod                                                    petri.niiranen  02/01
+---------------+---------------+---------------+---------------+---------------+------*/
static byte    *dimlinkage_appendOrdinateData
(
byte                 *pBuf,
DimOrdinateOverrides *pOverrides
)
    {
    byte    *pMod = NULL;

    /*-----------------------------------------------------------------------------------
        Modifiers will be written last.
    -----------------------------------------------------------------------------------*/
    pMod = pBuf;
    pBuf += sizeof (long);

    if (pOverrides->modifiers & ORDINATE_Override_StartValueX)
        ByteStreamHelper::AppendDouble (pBuf, pOverrides->startValueX);

    if (pOverrides->modifiers & ORDINATE_Override_StartValueY)
        ByteStreamHelper::AppendDouble (pBuf, pOverrides->startValueY);

/* TBD ... */

    /*-----------------------------------------------------------------------------------
        Save modifiers.
    -----------------------------------------------------------------------------------*/
    ByteStreamHelper::AppendUInt32 (pMod, pOverrides->modifiers);

    return  pBuf;
    }

/*---------------------------------------------------------------------------------**//**
* Converts override data from file format.
*
* @param        pOverrides      <=
* @param        pBuf            =>
* @bsimethod                                                    petri.niiranen  02/01
+---------------+---------------+---------------+---------------+---------------+------*/
static void    dimlinkage_extractOrdinateData
(
DimOrdinateOverrides *pOverrides,
byte const *&        pBuf
)
    {
    ByteStreamHelper::ExtractUInt32 (pOverrides->modifiers, pBuf);

    if (pOverrides->modifiers & ORDINATE_Override_StartValueX)
        ByteStreamHelper::ExtractDouble (pOverrides->startValueX, pBuf);

    if (pOverrides->modifiers & ORDINATE_Override_StartValueY)
        ByteStreamHelper::ExtractDouble (pOverrides->startValueY, pBuf);

/* TBD ... */
    }

/*---------------------------------------------------------------------------------**//**
* Converts text format data to file format.
*
* @param        pBuf        <= raw format data
* @param        pFmt        => format data.
* @return       raw data pointer.
* @bsimethod                                                    petri.niiranen  02/01
+---------------+---------------+---------------+---------------+---------------+------*/
struct DimFormattedTextFlags
    {
    UInt16  component:3;
    /*-----------------------------------------------------------
    Needs work: this numBytes limits max text string length upto
    DIMTEXT_MAX_STRINGLENGTH which is 255.  We have to rework
    this thing out to be able to hold text element max size 64K
    in the future.
    -----------------------------------------------------------*/
    UInt16  numStringBytes:8;
    UInt16  reserved:5;

    DimFormattedTextFlags (UInt16 comp, UInt16 nBytes)
        :
        component (comp),
        numStringBytes (nBytes),
        reserved (0)
        {
        }

    UInt16  AsUInt16 () { return *((UInt16*) this); }
    };

/*---------------------------------------------------------------------------------**//**
* Converts text format data to file format.
*
* @param        pBuf        <= raw format data
* @param        pFmt        => format data.
* @return       raw data pointer.
* @bsimethod                                                    petri.niiranen  02/01
+---------------+---------------+---------------+---------------+---------------+------*/
static byte    *dimlinkage_appendTextFormatData
(
byte                *pBuf,
DimFormattedText    *pFmt
)
    {
    byte*           pMod       = NULL;
    UInt32          modifiers  = 0;
    TextParamWide   textParams = pFmt->GetTextParamWide();

    /*-----------------------------------------------------------------------------------
        Modifiers will be written last.
    -----------------------------------------------------------------------------------*/
    pMod = pBuf;
    pBuf += sizeof (long);

    DimFormattedTextFlags flags (pFmt->GetComponentID (), (UInt16)pFmt->GetVariStringNumBytes());

    UInt16  iFlags = flags.AsUInt16();

    if (0 != iFlags)
        {
        modifiers |= (0x1 << 0);
        ByteStreamHelper::AppendUInt16 (pBuf, iFlags);
        }

    UInt32  font = textParams.font;

    if (0 != font)
        {
        modifiers |= (0x1 << 1);
        ByteStreamHelper::AppendUInt32 (pBuf, font);
        }

    DPoint2d origin = pFmt->GetOrigin();

    if (0.0 != origin.x || 0.0 != origin.y)
        {
        modifiers |= (0x1 << 2);

        ByteStreamHelper::AppendDouble (pBuf, origin.x);
        ByteStreamHelper::AppendDouble (pBuf, origin.y);
        }

    DPoint2d scale = pFmt->GetScale();

    if (1.0 != scale.x || 1.0 != scale.y)
        {
        modifiers |= (0x1 << 3);

        ByteStreamHelper::AppendDouble (pBuf, scale.x);
        ByteStreamHelper::AppendDouble (pBuf, scale.y);
        }

    double width = pFmt->GetWidth();

    if (1.0 != width)
        {
        modifiers |= (0x1 << 4);
        ByteStreamHelper::AppendDouble (pBuf, width);
        }

    double height = pFmt->GetHeight();

    if (1.0 != height)
        {
        modifiers |= (0x1 << 5);
        ByteStreamHelper::AppendDouble (pBuf, height);
        }

    CharCP    pVariStr = pFmt->GetVariString ();

    if (NULL != pVariStr && 0 != flags.numStringBytes)
        {
        modifiers |= (0x1 << 6);
        memcpy (pBuf, pVariStr, flags.numStringBytes);
        pBuf += flags.numStringBytes;
        }

    TextFormattingLinkage    textLink = pFmt->GetTextParamAsLinkageData ();
    UInt16      dataSize = 2 * LinkageUtil::GetWords (&textLink.linkHeader);

    if (0 != dataSize)
        {
        modifiers |= (0x1 << 7);

        ByteStreamHelper::AppendShort (pBuf, dataSize);

        // textLink is already in compressed format just memcpy to buffer
        memcpy (pBuf, &textLink, dataSize);
        pBuf += dataSize;
        }

    UInt16  just = (UInt16)textParams.just;

    if (0 != just)
        {
        modifiers |= (0x1 << 8);
        ByteStreamHelper::AppendUInt16 (pBuf, just);
        }

    UInt32  nodeNumber  = 0;

    if (pFmt->IsNodeComponent () && 0 != nodeNumber)
        {
        modifiers |= (0x1 << 9);
        ByteStreamHelper::AppendUInt32 (pBuf, nodeNumber);
        }

    double rotation = pFmt->GetRotation();

    if (0.0 != rotation)
        {
        modifiers |= (0x1 << 10);
        ByteStreamHelper::AppendDouble (pBuf, rotation);
        }

    double  dLineSpacing = textParams.lineSpacing;

    if (pFmt->IsNodeComponent() && 0.0 != dLineSpacing)
        {
        modifiers |= (0x1 << 11);
        ByteStreamHelper::AppendDouble (pBuf, dLineSpacing);
        }

    UInt16  lineAlignment = textParams.renderingFlags.lineAlignment;

    if (0 != lineAlignment)
        {
        modifiers |= (0x1 << 12);
        ByteStreamHelper::AppendUInt16 (pBuf, lineAlignment);
        }

    /* TBD ... */

    /*-----------------------------------------------------------------------------------
        Save modifiers.
    -----------------------------------------------------------------------------------*/
    ByteStreamHelper::AppendUInt32 (pMod, modifiers);

    return  pBuf;
    }

/*---------------------------------------------------------------------------------**//**
* Converts text format data from file format.
*
* @param        nameOfParam     <=>
* @return       what the return value means
* @bsimethod                                                    petri.niiranen  06/01
+---------------+---------------+---------------+---------------+---------------+------*/
static void    dimlinkage_extractTextFormatData
(
DimFormattedText    **ppFmt,
byte const *&       pBuf
)
    {
    UInt32      modifiers = 0;

    if (NULL == ppFmt || NULL == *ppFmt)
        return  ;

    ByteStreamHelper::ExtractUInt32 (modifiers, pBuf);

    DimFormattedTextFlags   flags(0, 0);

    if (modifiers & (0x1 << 0))
        {
        ByteStreamHelper::ExtractUInt16 ((UInt16&) flags, pBuf);
        (*ppFmt)->SetComponentID (flags.component);
        }

    UInt32 font = 0; // will be pushed to ppFmt below

    if (modifiers & (0x1 << 1))
        ByteStreamHelper::ExtractUInt32 (font, pBuf);

    if (modifiers & (0x1 << 2))
        {
        DPoint2d    origin;

        ByteStreamHelper::ExtractDouble (origin.x, pBuf);
        ByteStreamHelper::ExtractDouble (origin.y, pBuf);
        (*ppFmt)->SetOrigin (origin);
        }

    if (modifiers & (0x1 << 3))
        {
        DPoint2d    scale;

        ByteStreamHelper::ExtractDouble (scale.x, pBuf);
        ByteStreamHelper::ExtractDouble (scale.y, pBuf);
        (*ppFmt)->SetScale (scale);
        }

    if (modifiers & (0x1 << 4))
        {
        double  width;

        ByteStreamHelper::ExtractDouble (width, pBuf);
        (*ppFmt)->SetWidth (width);
        }
    else
        {
        (*ppFmt)->SetWidth (1.0);
        }

    if (modifiers & (0x1 << 5))
        {
        double  height;

        ByteStreamHelper::ExtractDouble (height, pBuf);
        (*ppFmt)->SetHeight (height);
        }
    else
        {
        (*ppFmt)->SetHeight (1.0);
        }

    if (modifiers & (0x1 << 6))
        {
        UInt16  stringSize = flags.numStringBytes;

        (*ppFmt)->SetVariString ((char const*) pBuf, stringSize);
        pBuf += stringSize;
        }

    if (modifiers & (0x1 << 7))
        {
        UInt16      paramSize;
        TextFormattingLinkage    textLink;

        memset (&textLink, 0, sizeof (textLink));

        ByteStreamHelper::ExtractUInt16 (paramSize, pBuf);
        memcpy (&textLink, pBuf, paramSize);
        (*ppFmt)->SetTextParamFromLinkageData (textLink);
        pBuf += paramSize;
        }

    UInt16  just = 0;  // will be pushed to ppFmt below

    if (modifiers & (0x1 << 8))
        ByteStreamHelper::ExtractUInt16 (just, pBuf);

    if (modifiers & (0x1 << 9))
        {
        UInt32  nodeNumber = 0;

        ByteStreamHelper::ExtractUInt32 (nodeNumber, pBuf);
        (*ppFmt)->SetNodeNumber (nodeNumber);
        }
    else
        {
        (*ppFmt)->SetNodeNumber (0);
        }

    if (modifiers & (0x1 << 10))
        {
        double    rotation;

        ByteStreamHelper::ExtractDouble (rotation, pBuf);
        (*ppFmt)->SetRotation (rotation);
        }
    else
        {
        (*ppFmt)->SetRotation (0.0);
        }

    double    dLineSpacing = 0.0; // will be pushed to ppFmt below

    if (modifiers & (0x1 << 11))
        ByteStreamHelper::ExtractDouble (dLineSpacing, pBuf);

    UInt16  lineAlignment = 0;   // will be pushed to ppFmt below

    if (modifiers & (0x1 << 12))
        ByteStreamHelper::ExtractUInt16 (lineAlignment, pBuf);

    /*--------------------------------------------------------------------------
       The following parts of the TextParamWide are not stored in the TextFormattingLinkage,
       we've collected them above from the DimFormattedText linkage, now we
       push them into the structure.
    --------------------------------------------------------------------------*/
    TextParamWide   textParams = (*ppFmt)->GetTextParamWide();

    textParams.font = font;
    textParams.just = static_cast <int>(just);
    textParams.renderingFlags.lineAlignment = lineAlignment;
    textParams.lineSpacing = dLineSpacing;

    (*ppFmt)->SetTextParamWide (textParams);
    }

/*=================================================================================**//**
*
* Convert from raw data and vice versa
*
+===============+===============+===============+===============+===============+======*/
/*---------------------------------------------------------------------------------**//**
* Initializes linkage header and populates common information.
*
* @param        pHdr        <=> address of data to initialize
* @param        secondaryID =>  sub type
* @param        dataSize    =>  expected data size.
* @bsimethod                                                    petri.niiranen  02/01
+---------------+---------------+---------------+---------------+---------------+------*/
static void    dimlinkage_initializeLinkage
(
DimExtLinkageHdr   *pHdr,
UInt16              secondaryID,
int                 dataSize
)
    {
    memset (pHdr, 0, dataSize);
    pHdr->hdr.user      = true;
    pHdr->hdr.primaryID = LINKAGEID_DimExtensionLinkage;

    pHdr->shdr.type     = secondaryID;
    }

/*---------------------------------------------------------------------------------**//**
* Creates linkage with raw data.
*
* @param        pLinkageOut     <= new linkage data
* @param        pointNoIn       => data represents this point
* @param        pOverridesIn    => override data
* @return       data size in bytes
* @bsimethod                                                    petri.niiranen  02/01
+---------------+---------------+---------------+---------------+---------------+------*/
static int     dimlinkage_convertToPointLinkage
(
DimPointLinkage     *pLinkageOut,
int                 pointNoIn,
DimPointOverrides   *pOverridesIn
)
    {
    int     linkBytes;
    byte    *pEnd = NULL;

    dimlinkage_initializeLinkage ((DimExtLinkageHdr *) pLinkageOut, LINKAGESUBTYPE_PointOverride, sizeof (*pLinkageOut));

    pLinkageOut->point         = (UInt16) pointNoIn;

    /* Convert data to file format */
    pEnd = dimlinkage_appendPointData ((byte *) &pLinkageOut->modifiers, pOverridesIn);

    linkBytes = ((pEnd - ((byte *) pLinkageOut) + 7) & ~7);
    LinkageUtil::SetWords (&pLinkageOut->hdr, linkBytes / 2);

    return  linkBytes;
    }

/*---------------------------------------------------------------------------------**//**
* Convert linkage data to override information.
*
* @param        pOverridesOut   <=
* @param        pLinkageIn      =>
* @return       always SUCCESS
* @bsimethod                                                    petri.niiranen  02/01
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt       dimlinkage_convertFromPointLinkage
(
DimPointOverrides   *pOverridesOut,
DimPointLinkage     *pLinkageIn
)
    {
    byte const * buffer = (byte const *)&pLinkageIn->modifiers;
    
    memset (pOverridesOut, 0, sizeof (*pOverridesOut));
    dimlinkage_extractPointData (pOverridesOut, buffer);

    return  SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar 06/05
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt        BentleyApi::mdlDimLinkage_convertFromPointLinkage
(
DimPointOverrides *     pOverridesOut,
LinkageHeader *         pLinkHdrIn
)
    {
    DimPointLinkage * pPointLinkage = (DimPointLinkage *) pLinkHdrIn;
    return dimlinkage_convertFromPointLinkage (pOverridesOut, pPointLinkage);
    }

/*---------------------------------------------------------------------------------**//**
* Create linkage with raw data.
*
* @param        pLinkageOut     <= new linkage data
* @param        segmentNoIn     => data represents this segment
* @param        pOverridesIn    => override data
* @return       data size in bytes
* @bsimethod                                                    petri.niiranen  02/01
+---------------+---------------+---------------+---------------+---------------+------*/
static int     dimlinkage_convertToSegmentLinkage
(
DimSegmentLinkage     *pLinkageOut,
int                   segmentNoIn,
DimSegmentOverrides   *pOverridesIn
)
    {
    int     linkBytes;
    byte    *pEnd = NULL;

    dimlinkage_initializeLinkage ((DimExtLinkageHdr *) pLinkageOut, LINKAGESUBTYPE_SegmentOverride, sizeof (*pLinkageOut));

    pLinkageOut->segment       = (UInt16) segmentNoIn;

    /* Convert data to file format */
    pEnd = dimlinkage_appendSegmentData ((byte *) &pLinkageOut->modifiers, pOverridesIn);

    linkBytes = ((pEnd - ((byte *) pLinkageOut) + 7) & ~7);
    LinkageUtil::SetWords (&pLinkageOut->hdr, linkBytes / 2);

    return  linkBytes;
    }

/*---------------------------------------------------------------------------------**//**
* Convert linkage data to override information.
*
* @param        pOverridesOut   <=
* @param        pLinkageIn      =>
* @return       always SUCCESS
* @bsimethod                                                    petri.niiranen  02/01
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt      dimlinkage_convertFromSegmentLinkage
(
DimSegmentOverrides   *pOverridesOut,
DimSegmentLinkage     *pLinkageIn
)
    {
    byte const * buffer = (byte const *)&pLinkageIn->modifiers;
    
    memset (pOverridesOut, 0, sizeof (*pOverridesOut));
    dimlinkage_extractSegmentData (pOverridesOut, buffer);

    return  SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar 06/05
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt        BentleyApi::mdlDimLinkage_convertFromSegmentLinkage
(
DimSegmentOverrides *   pOverridesOut,
LinkageHeader *         pLinkHdrIn
)
    {
    DimSegmentLinkage * pSegmentLinkage = (DimSegmentLinkage *) pLinkHdrIn;
    return dimlinkage_convertFromSegmentLinkage (pOverridesOut, pSegmentLinkage);
    }

/*---------------------------------------------------------------------------------**//**
* Create linkage with raw data.
*
* @param        pLinkageOut     <= new linkage data
* @param        segmentNoIn     => data represents this segment
* @param        pOverridesIn    => override data
* @return       data size in bytes
* @bsimethod                                                    petri.niiranen  03/01
+---------------+---------------+---------------+---------------+---------------+------*/
static int     dimlinkage_convertToSegmentFlagLinkage
(
DimSegmentFlagLinkage   *pLinkageOut,
int                     segmentNoIn,
DimSegmentFlagOverrides *pOverridesIn
)
    {
    int     linkBytes;
    byte    *pEnd = NULL;

    dimlinkage_initializeLinkage ((DimExtLinkageHdr *) pLinkageOut, LINKAGESUBTYPE_SegmentFlagOverride, sizeof (*pLinkageOut));

    pLinkageOut->segment       = (UInt16) segmentNoIn;

    /* Convert data to file format */
    pEnd = dimlinkage_appendSegmentFlagData ((byte *) &pLinkageOut->modifiers, pOverridesIn);

    linkBytes = ((pEnd - ((byte *) pLinkageOut) + 7) & ~7);
    LinkageUtil::SetWords (&pLinkageOut->hdr, linkBytes / 2);

    return  linkBytes;
    }

/*---------------------------------------------------------------------------------**//**
* Convert linkage data to override information.
*
* @param        pOverridesOut   <=
* @param        pLinkageIn      =>
* @return       always SUCCESS
* @bsimethod                                                    petri.niiranen  02/01
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt       dimlinkage_convertFromSegmentFlagLinkage
(
DimSegmentFlagOverrides   *pOverridesOut,
DimSegmentFlagLinkage     *pLinkageIn
)
    {
    byte const * buffer = (byte const *)&pLinkageIn->modifiers;
    
    memset (pOverridesOut, 0, sizeof (*pOverridesOut));
    dimlinkage_extractSegmentFlagData (pOverridesOut, buffer);

    return  SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar 06/05
+---------------+---------------+---------------+---------------+---------------+------*/
Public StatusInt        mdlDimLinkage_convertFromSegmentFlagLinkage
(
DimSegmentFlagOverrides *   pOverridesOut,
LinkageHeader *         pLinkHdrIn
)
    {
    DimSegmentFlagLinkage * pSegmentFlagLinkage = (DimSegmentFlagLinkage *) pLinkHdrIn;
    return dimlinkage_convertFromSegmentFlagLinkage (pOverridesOut, pSegmentFlagLinkage);
    }

/*---------------------------------------------------------------------------------**//**
* Create linkage with raw data.
*
* @param        pLinkageOut     <= new linkage data
* @param        pOverridesIn    => override data
* @return       data size in bytes
* @bsimethod                                                    petri.niiranen  02/01
+---------------+---------------+---------------+---------------+---------------+------*/
static int     dimlinkage_convertToOverallLinkage
(
DimOverallLinkage       *pLinkageOut,
DimOverallOverrides     *pOverridesIn
)
    {
    int     linkBytes;
    byte    *pEnd = NULL;

    dimlinkage_initializeLinkage ((DimExtLinkageHdr *) pLinkageOut, LINKAGESUBTYPE_OverallOverride, sizeof (*pLinkageOut));

    /* Convert data to file format */
    pEnd = dimlinkage_appendOverallData ((byte *) &pLinkageOut->modifiers, pOverridesIn);

    linkBytes = ((pEnd - ((byte *) pLinkageOut) + 7) & ~7);
    LinkageUtil::SetWords (&pLinkageOut->hdr, linkBytes / 2);

    return  linkBytes;
    }

/*---------------------------------------------------------------------------------**//**
* Create linkage with raw data.
*
* @param        pLinkageOut     <= new linkage data
* @param        pOverridesIn    => override data
* @return       data size in bytes
* @bsimethod                                                    JoshSchifter    09/01
+---------------+---------------+---------------+---------------+---------------+------*/
static int     dimlinkage_convertToStyleExtensionlLinkage
(
DimStyleExtensionLinkage*   pLinkageOut,
DimStyleExtensions const*   pExtensionsIn
)
    {
    int     linkBytes;
    byte    *pEnd = NULL;

    dimlinkage_initializeLinkage ((DimExtLinkageHdr *) pLinkageOut, LINKAGESUBTYPE_StyleExtension, sizeof (*pLinkageOut));

    /* Convert data to file format */
    pEnd = dimlinkage_appendStyleExtensionData ((byte *) &pLinkageOut->modifiers, pExtensionsIn);

    linkBytes = ((pEnd - ((byte *) pLinkageOut) + 7) & ~7);
    LinkageUtil::SetWords (&pLinkageOut->hdr, linkBytes / 2);

    return  linkBytes;
    }

/*---------------------------------------------------------------------------------**//**
* Convert linkage data to override information.
*
* @param        pOverridesOut   <=
* @param        pLinkageIn      =>
* @return       always SUCCESS
* @bsimethod                                                    petri.niiranen  02/01
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt       dimlinkage_convertFromOverallLinkage
(
DimOverallOverrides     *pOverridesOut,
DimOverallLinkage       *pLinkageIn
)
    {
    byte const * buffer = (byte const *)&pLinkageIn->modifiers;
    
    memset (pOverridesOut, 0, sizeof (*pOverridesOut));
    dimlinkage_extractOverallData (pOverridesOut, buffer);

    return  SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar 06/05
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt        BentleyApi::mdlDimLinkage_convertFromOverallLinkage
(
DimOverallOverrides *   pOverridesOut,
LinkageHeader *         pLinkHdrIn
)
    {
    DimOverallLinkage * pOverallLinkage = (DimOverallLinkage *) pLinkHdrIn;
    return dimlinkage_convertFromOverallLinkage (pOverridesOut, pOverallLinkage);
    }

/*---------------------------------------------------------------------------------**//**
* Convert linkage data to style extension information.
*
* @param        pExtensionsOut  <=
* @param        pLinkageIn      =>
* @return       always SUCCESS
* @bsimethod                                                    JoshSchifter    09/01
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt       dimlinkage_convertFromStyleExtensionLinkage
(
DimStyleExtensions          *pExtensionsOut,
DimStyleExtensionLinkage    *pLinkageIn
)
    {
    byte const * buffer = (byte const *)&pLinkageIn->modifiers;
    
    memset (pExtensionsOut, 0, sizeof (*pExtensionsOut));
    dimlinkage_extractStyleExtensionData (pExtensionsOut, buffer);

    return  SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar 06/05
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt        BentleyApi::mdlDimLinkage_convertFromStyleExtensionLinkage
(
DimStyleExtensions *   pOverridesOut,
LinkageHeader *         pLinkHdrIn
)
    {
    DimStyleExtensionLinkage * pStyleExtensionLinkage = (DimStyleExtensionLinkage *) pLinkHdrIn;
    return dimlinkage_convertFromStyleExtensionLinkage (pOverridesOut, pStyleExtensionLinkage);
    }

/*---------------------------------------------------------------------------------**//**
* Create linkage with raw data.
*
* @param        pLinkageOut     <= new linkage data
* @param        pOverridesIn    => override data
* @return       data size in bytes
* @bsimethod                                                    petri.niiranen  02/01
+---------------+---------------+---------------+---------------+---------------+------*/
static int     dimlinkage_convertToOrdinateLinkage
(
DimOrdinateLinkage       *pLinkageOut,
DimOrdinateOverrides     *pOverridesIn
)
    {
    int     linkBytes;
    byte    *pEnd = NULL;

    dimlinkage_initializeLinkage ((DimExtLinkageHdr *) pLinkageOut, LINKAGESUBTYPE_OrdinateOverride, sizeof (*pLinkageOut));

    /* Convert data to file format */
    pEnd = dimlinkage_appendOrdinateData ((byte *) &pLinkageOut->modifiers, pOverridesIn);

    linkBytes = ((pEnd - ((byte *) pLinkageOut) + 7) & ~7);
    LinkageUtil::SetWords (&pLinkageOut->hdr, linkBytes / 2);

    return  linkBytes;
    }

/*---------------------------------------------------------------------------------**//**
* Convert linkage data to override information.
*
* @param        pOverridesOut   <=
* @param        pLinkageIn      =>
* @return       always SUCCESS
* @bsimethod                                                    petri.niiranen  02/01
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt       dimlinkage_convertFromOrdinateLinkage
(
DimOrdinateOverrides     *pOverridesOut,
DimOrdinateLinkage       *pLinkageIn
)
    {
    byte const * buffer = (byte const *)&pLinkageIn->modifiers;
    
    memset (pOverridesOut, 0, sizeof (*pOverridesOut));
    dimlinkage_extractOrdinateData (pOverridesOut, buffer);

    return  SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar 06/05
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt        BentleyApi::mdlDimLinkage_convertFromOrdinateLinkage
(
DimOrdinateOverrides *   pOverridesOut,
LinkageHeader *         pLinkHdrIn
)
    {
    DimOrdinateLinkage * pOrdinateLinkage = (DimOrdinateLinkage *) pLinkHdrIn;
    return dimlinkage_convertFromOrdinateLinkage (pOverridesOut, pOrdinateLinkage);
    }

/*---------------------------------------------------------------------------------**//**
* Convert data to text formatting linkage information.
*
* @param        nameOfParam     <=>
* @return       what the return value means
* @bsimethod                                                    petri.niiranen  06/01
+---------------+---------------+---------------+---------------+---------------+------*/
static int     dimlinkage_convertToTextFormatLinkage
(
DimFormattedTextLinkage     *pFmtLinkageOut,
int                         segmentNo,
DimFormattedText            *pFormatIn
)
    {
    int     linkBytes;
    byte    *pEnd = NULL;

    dimlinkage_initializeLinkage ((DimExtLinkageHdr *) pFmtLinkageOut, LINKAGESUBTYPE_FormattedText, sizeof (*pFmtLinkageOut));
    pFmtLinkageOut->segment = (UInt16) segmentNo;

    /* Convert data to file format */
    pEnd = dimlinkage_appendTextFormatData ((byte *) &pFmtLinkageOut->modifiers, pFormatIn);

    linkBytes = ((pEnd - ((byte *) pFmtLinkageOut) + 7) & ~7);
    LinkageUtil::SetWords (&pFmtLinkageOut->hdr, linkBytes / 2);

    return  linkBytes;
    }

/*---------------------------------------------------------------------------------**//**
* Convert linkage data to text formatting information.
*
* @param        nameOfParam     <=>
* @return       what the return value means
* @bsimethod                                                    petri.niiranen  06/01
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt       dimlinkage_convertFromTextFormatLinkage
(
DimFormattedText            **ppFormatOut,
DimFormattedTextLinkage     *pFmtLinkageIn
)
    {
    byte const * buffer = (byte const *) &pFmtLinkageIn->modifiers;
    
    dimlinkage_extractTextFormatData (ppFormatOut, buffer);

    return  SUCCESS;
    }

/*=================================================================================**//**
*
* Element manipulation methods
*
+===============+===============+===============+===============+===============+======*/
/*---------------------------------------------------------------------------------**//**
* Appends overrides to element.
*
* @param        dimElement      <=>
* @param        pOverridesIn    => data to add
* @param        pointNo         => point to apply data
* @return       addition status
* @bsimethod                                                    petri.niiranen  02/01
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt    dimlinkage_addPointOverride
(
EditElementHandleR  dimElement,
DimPointOverrides*  pOverridesIn,
int                 pointNo
)
    {
    DimPointLinkage     linkage;

    if (0 == pOverridesIn->modifiers)
        return SUCCESS;

    dimlinkage_convertToPointLinkage (&linkage, pointNo, pOverridesIn);

    return dimElement.AppendElementLinkage (NULL, linkage.hdr, (byte*)&linkage + sizeof(linkage.hdr));
    }

/*---------------------------------------------------------------------------------**//**
* Extract nth subtype linkage by id.
*
* @param        ppRawDataOut    <= raw data out
* @param        pLinkageOut     <= linkage at index
* @param        maxSizeIn       => output buffer size
* @param        pElement        => element to extract information
* @param        expectedSubType => extract this sub type linkage
* @param        index           => get data at this index
* @return       SUCCESS if data is found.
* @bsimethod                                                    petri.niiranen  02/01
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt   dimlinkage_extractLinkageByIndex
(
UInt16             **ppRawDataOut,
DimExtLinkageHdr   *pLinkageOut,
size_t             maxSizeIn,
ElementHandleCR       dimElement,
int                expectedSubType,
int                index
)
    {
    UInt16                      *pRaw = NULL;
    int                         linkageCounter = 0, subTypeCounter = 0;
    StatusInt                   status = ERROR;

    while (NULL != (pRaw = (UInt16*) linkage_extractLinkageByIndex (NULL, dimElement.GetElementCP(), LINKAGEID_DimExtensionLinkage, NULL, linkageCounter)))
        {
        LinkageHeader      *pHeader = NULL;
        size_t             fullsize;
        DimExtLinkageHdr   *pDimExtHdr = NULL;

        pDimExtHdr = (DimExtLinkageHdr *) pRaw;

        if (expectedSubType == pDimExtHdr->shdr.type)
            {
            if (index == subTypeCounter)
                {
                // return in linkage
                if (NULL != pLinkageOut)
                    {
                    // manually convert data since we do not know data size
                    pHeader  = (LinkageHeader *) pRaw;
                    fullsize = LinkageUtil::GetWords (pHeader) * 2;

                    memcpy (pLinkageOut, pHeader, MIN (fullsize, maxSizeIn));
                    }

                // return raw data
                if (NULL != ppRawDataOut)
                    *ppRawDataOut = pRaw;

                // and exit loop
                status = SUCCESS;
                break;
                }

            subTypeCounter++;
            }

        linkageCounter++;
        }

    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* Deletes nth subtype linkage from element.
*
* @param        dimElement      <=>
* @param        expectedSubType =>  type of linkage to delete
* @param        index           =>  index to given subtype
* @return       Deletion status
* @bsimethod                                                    petri.niiranen  02/01
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt       dimlinkage_deleteLinkageByIndex
(
EditElementHandleR dimElement,
int         expectedSubType,
int         index
)
    {
    UInt16              *pRaw          = NULL;
    int                 linkageCounter = 0;
    int                 subTypeCounter = 0;
    int                 nDeleted       = 0;

    while (NULL != (pRaw = (UInt16*) linkage_extractLinkageByIndex (NULL, dimElement.GetElementCP(), LINKAGEID_DimExtensionLinkage, NULL, linkageCounter)))
        {
        DimExtLinkageHdr   *pDimExtHdr = NULL;

        pDimExtHdr = (DimExtLinkageHdr *) pRaw;

        if (expectedSubType == pDimExtHdr->shdr.type)
            {
            if (index == subTypeCounter)
                {
                nDeleted = linkage_deleteLinkageByIndex (dimElement.GetElementP(), LINKAGEID_DimExtensionLinkage, linkageCounter);
                break;
                }

            subTypeCounter++;
            }

        linkageCounter++;
        }

    return  nDeleted ? SUCCESS : ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* Extract point overrides by point number.
*
* @param        ppLinkageOut    <= raw data out
* @param        pOverridesOut   <= point overrides
* @param        pIndexOut       <= data found at this index
* @param        pElement        => element to extract information
* @param        pointNo         => get data from this point
* @return       SUCCESS if data is found.
* @bsimethod                                                    petri.niiranen  02/01
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt       dimlinkage_extractPointOverridesByPointNo
(
UInt16              **ppLinkageOut,
DimPointOverrides   *pOverridesOut,
int                 *pIndexOut,
ElementHandleCR        dimElement,
int                 pointNo
)
    {
    StatusInt           status = ERROR;
    int                 index = 0;
    DimPointLinkage     linkageHdr;

    if (! isValidDimensionAndPoint (dimElement, pointNo))
        return  ERROR;

    while (SUCCESS == (status = dimlinkage_extractLinkageByIndex (ppLinkageOut, (DimExtLinkageHdr *) &linkageHdr,
                                                                  sizeof (linkageHdr), dimElement,
                                                                  LINKAGESUBTYPE_PointOverride, index)))
        {
        if (pointNo == (int) linkageHdr.point)
            {
            if (pOverridesOut)
                dimlinkage_convertFromPointLinkage (pOverridesOut, &linkageHdr);

            if (pIndexOut)
                *pIndexOut = index;

            break;
            }

        index++;
        }

    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* Appends overrides to element
*
* @param        dimElement      <=>
* @param        pOverridesIn    => data to add
* @param        pointNo         => point to apply data
* @return       addition status
* @bsimethod                                                    petri.niiranen  02/01
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt        dimlinkage_addSegmentOverride
(
EditElementHandleR      dimElement,
DimSegmentOverrides*    pOverridesIn,
int                     segmentNo
)
    {
    DimSegmentLinkage     linkage;

    if (0 == pOverridesIn->modifiers)
        return SUCCESS;

    dimlinkage_convertToSegmentLinkage (&linkage, segmentNo, pOverridesIn);

    return dimElement.AppendElementLinkage (NULL, linkage.hdr, (byte*)&linkage + sizeof(linkage.hdr));
    }

/*---------------------------------------------------------------------------------**//**
* Extract segment overrides by segment number.
*
* @param        ppLinkageOut    <= raw data out
* @param        pOverridesOut   <= overrides
* @param        pIndexOut       <= data found at this index
* @param        pElement        => element to extract information
* @param        pointNo         => get data from this segment
* @return       SUCCESS if data is found.
* @bsimethod                                                    petri.niiranen  02/01
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt       dimlinkage_extractSegmentOverridesBySegmentNo
(
UInt16              **ppLinkageOut,
DimSegmentOverrides *pOverridesOut,
int                 *pIndexOut,
ElementHandleCR        element,
int                 segmentNo
)
    {
    StatusInt            status = ERROR;
    int                  index = 0;
    DimSegmentLinkage    linkageHdr;

    if (! isValidDimensionAndSegment (element, segmentNo))
        return  ERROR;

    while (SUCCESS == (status = dimlinkage_extractLinkageByIndex (ppLinkageOut, (DimExtLinkageHdr *) &linkageHdr,
                                                                  sizeof (linkageHdr), element,
                                                                  LINKAGESUBTYPE_SegmentOverride, index)))
        {
        if (segmentNo == (int) linkageHdr.segment)
            {
            if (NULL != pOverridesOut)
                dimlinkage_convertFromSegmentLinkage (pOverridesOut, &linkageHdr);

            if (pIndexOut)
                *pIndexOut = index;

            break;
            }

        index++;
        }

    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* Appends segment flag overrides to element
*
* @param        dimElement      <=>
* @param        pOverridesIn    => data to add
* @param        pointNo         => point to apply data
* @return       addition status
* @bsimethod                                                    petri.niiranen  02/01
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt            dimlinkage_addSegmentFlagOverride
(
EditElementHandleR          dimElement,
DimSegmentFlagOverrides*    pOverridesIn,
int                         segmentNo
)
    {
    DimSegmentFlagLinkage     linkage;

    if (0 == pOverridesIn->modifiers)
        return SUCCESS;

    dimlinkage_convertToSegmentFlagLinkage (&linkage, segmentNo, pOverridesIn);

    return dimElement.AppendElementLinkage (NULL, linkage.hdr, (byte*)&linkage + sizeof(linkage.hdr));
    }

/*---------------------------------------------------------------------------------**//**
* Extract segment flag overrides by segment number.
*
* @param        ppLinkageOut    <= raw data out
* @param        pOverridesOut   <= overrides
* @param        pIndexOut       <= data found at this index
* @param        pElement        => element to extract information
* @param        pointNo         => get data from this segment
* @return       SUCCESS if data is found.
* @bsimethod                                                    petri.niiranen  02/01
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt       dimlinkage_extractSegmentFlagOverridesBySegmentNo
(
UInt16                  **ppLinkageOut,
DimSegmentFlagOverrides *pOverridesOut,
int                     *pIndexOut,
ElementHandleCR            element,
int                     segmentNo
)
    {
    StatusInt               status = ERROR;
    int                     index = 0;
    DimSegmentFlagLinkage   linkageHdr;

    if (! isValidDimensionAndSegment (element, segmentNo))
        return  ERROR;

    while (SUCCESS == (status = dimlinkage_extractLinkageByIndex (ppLinkageOut, (DimExtLinkageHdr *) &linkageHdr,
                                                                  sizeof (linkageHdr), element, LINKAGESUBTYPE_SegmentFlagOverride, index)))
        {
        if (segmentNo == (int) linkageHdr.segment)
            {
            if (NULL != pOverridesOut)
                dimlinkage_convertFromSegmentFlagLinkage (pOverridesOut, &linkageHdr);

            if (pIndexOut)
                *pIndexOut = index;

            break;
            }

        index++;
        }

    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* Extract overall overrides.
*
* @param        ppLinkageOut    <=> raw data out
* @param        pOverridesOut   <=  linkage
* @param        pElement        =>  get data from this dimension
* @return       SUCCESS if extraction is ok.
* @bsimethod                                                    petri.niiranen  02/01
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt       dimlinkage_extractOverallOverrides
(
UInt16                  **ppLinkageOut,
DimOverallOverrides     *pOverridesOut,
ElementHandleCR            dimElement,
int                     *pIndexOut
)
    {
    int                 status = ERROR;
    int                 index = 0;
    DimOverallLinkage   linkageHdr;

    if (! isValidDimension (dimElement))
        return  ERROR;

    if (SUCCESS == (status = dimlinkage_extractLinkageByIndex (ppLinkageOut, (DimExtLinkageHdr *) &linkageHdr,
                                                               sizeof (linkageHdr), dimElement,
                                                               LINKAGESUBTYPE_OverallOverride, index)))
        {
        if (NULL != pOverridesOut)
            dimlinkage_convertFromOverallLinkage (pOverridesOut, &linkageHdr);

        if (NULL != pIndexOut)
            *pIndexOut = index;
        }

    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* Tests whether given element is a valid dimension style or dimension element.
*
* @param        pCandidate      => element to tedt
* @return       true if valid style or dimension element.
* @bsimethod                                                    petri.niiranen  11/00
+---------------+---------------+---------------+---------------+---------------+------*/
int      BentleyApi::dimStyleEntry_isStyleOrDimension (ElementHandleCR element)
    {
    return  (element.IsValid() &&
             element.GetLegacyType() == DIMENSION_ELM ||
             dimStyleEntry_isStyleElement (element.GetElementCP()));
    }
/*---------------------------------------------------------------------------------**//**
* Extract style extensions.
*
* @param        ppLinkageOut    <=> raw data out
* @param        pExtensionsOut  <=  linkage
* @param        pElement        =>  get data from this dimension
* @return       SUCCESS if extraction is ok.
* @bsimethod                                                    JoshSchifter    09/01
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt       dimlinkage_extractStyleExtensions
(
UInt16              **ppLinkageOut,
DimStyleExtensions  *pExtensionsOut,
ElementHandleCR        dimElement,
int                 *pIndexOut
)
    {
    int                         status = ERROR;
    int                         index = 0;
    DimStyleExtensionLinkage    linkageHdr;

    if (! dimStyleEntry_isStyleOrDimension (dimElement))
        return  ERROR;

    if (SUCCESS == (status = dimlinkage_extractLinkageByIndex (ppLinkageOut, (DimExtLinkageHdr *) &linkageHdr,
                                                               sizeof (linkageHdr), dimElement,
                                                               LINKAGESUBTYPE_StyleExtension, index)))
        {
        if (NULL != pExtensionsOut)
            dimlinkage_convertFromStyleExtensionLinkage (pExtensionsOut, &linkageHdr);

        if (NULL != pIndexOut)
            *pIndexOut = index;
        }

    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* Appends overrides to element
*
* @param        dimElement      <=>
* @param        pOverridesIn    => data to add
* @return       addition status
* @bsimethod                                                    petri.niiranen  02/01
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt        dimlinkage_addOverallOverride
(
EditElementHandleR      dimElement,
DimOverallOverrides*    pOverridesIn
)
    {
    if (0 == pOverridesIn->modifiers)
        return SUCCESS;

    DimOverallLinkage     linkage;
    dimlinkage_convertToOverallLinkage (&linkage, pOverridesIn);

    if (SUCCESS != dimElement.AppendElementLinkage (NULL, linkage.hdr, (byte*)&linkage + sizeof(linkage.hdr)))
        return ERROR;

#if defined (SET_DYNAMIC_RANGE_ON_ALL_ANNOTATIONS)
    if (pOverridesIn->modifiers & OVERALL_Override_ModelAnnotationScale)
        dimElement->hdr.dhdr.props.b.dynamicRange = true;
#endif

    return  SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
    Find and preserve possible future additions to the style exention linkage meta data:

    If meta data size stored in linkage (shdr.size) is greater than currently known size,
    the extra unknown data must have come from a future addition which needs to be
    preserved.  If shdr.size is zero or is smaller than currently known size, all stored
    data is known and there is no extra data to preserve.

    This above logic is based on following 2 assumptions:
    1) A new style extension is always appended at the end of linkage data.
    2) Its data size has correctly been saved as shdr.size by a future version that adds
       the new extension.

    Possible scenarios:
    - A style extensions linkage that is created in XM or any version thereafter:
      Such a new linkage will have been saved by this code, and more importantly shdr.size
      will also have been assigned a correct data size (either at very begining and at the
      end of this function).  That is, we should never see shdr.size=0 in this type of
      linkages.
    - An old style extensions linkage that was created prior to XM:
      In this case, we get shdr.size=0 at the very first time when the style is modified,
      and nothing but shdr.size should be changed in this code.  From there on, we always
      get shdr.size > 0, but its value stays the same every time it hits here again, thus
      there is still no extra size of data to be added.

* @bsimethod                                                    Don.Fu          07/05
+---------------+---------------+---------------+---------------+---------------+------*/
static bool                preserveUnknownStyleExtensionsLinkage
(
DimStyleExtensionLinkage    **ppLinkageOut,
DimStyleExtensionLinkage    *pLinkageIn,
ElementHandleCR                dimElement
)
    {
    DimStyleExtensionLinkage    existingLinkage;
    byte                        *pExistingData = NULL;

    /*-----------------------------------------------------------------------------------
    Update the input new linkage by storing actual meta data size.  This this a vital
    step for below algrithm to succeed.  Any new or modified style must be unconditionally
    updated with its currently known meta data size prior to bing saved.
    -----------------------------------------------------------------------------------*/
    pLinkageIn->shdr.size = getStyleExtensionsActualDataSize (pLinkageIn);

    if (SUCCESS == dimlinkage_extractLinkageByIndex ((UInt16**)&pExistingData, (DimExtLinkageHdr *)&existingLinkage,
                                                     sizeof(existingLinkage), dimElement, LINKAGESUBTYPE_StyleExtension, 0))
        {
        /* Only need to preserve new linkages in XM or laster versions(see title comment): */
        if (existingLinkage.shdr.size > 0)
            {
            /* Find the extra unknown meta data at the end of the existing linkage: */
            UInt32  knownSize = getStyleExtensionsActualDataSize (&existingLinkage);
            UInt32  baseSize = offsetof(DimStyleExtensionLinkage, metaData) + knownSize;
            int     extraSize = existingLinkage.shdr.size - knownSize;
            byte    *pExtraData = pExistingData + baseSize;

            /*---------------------------------------------------------------------------
            Add the extra unknown data into linkage only if the existing linkage is bigger.
            Ignore the case when existing linkage is smaller as there is no data loss in
            such a case.  The modified mask should ensure all known data to be caught.
            ---------------------------------------------------------------------------*/
            if (extraSize > 0)
                {
                int     newBaseSize = offsetof(DimStyleExtensionLinkage, metaData) + pLinkageIn->shdr.size;
                int     newTotalSize = newBaseSize + extraSize;
                byte    *pEndLinkage = NULL;

                /* pad new linkage size */
                newTotalSize = (newTotalSize + 7) & ~7;

                *ppLinkageOut = (DimStyleExtensionLinkage*) calloc (1, newTotalSize);

                /* copy base linkage */
                memcpy (*ppLinkageOut, pLinkageIn, newBaseSize);

                pEndLinkage = (byte*) &(*ppLinkageOut)->hdr + newBaseSize;

                /* copy extra linkage blob */
                memcpy (pEndLinkage, pExtraData, extraSize);

                /* reset total linkage size */
                LinkageUtil::SetWords (&((*ppLinkageOut)->hdr), newTotalSize / 2);

                /* save off existing meta data size */
                (*ppLinkageOut)->shdr.size += (UInt16)extraSize;

                return  true;
                }
            }
        }

    return  false;
    }

/*---------------------------------------------------------------------------------**//**
* Appends style extensions to element
*
* @param        dimElement      <=>
* @param        pExtensionsIn    => data to add
* @return       addition status
* @bsimethod                                                    petri.niiranen  02/01
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt       dimlinkage_addStyleExtension
(
EditElementHandleR          dimElement,            /*  => */
DimStyleExtensions const*   pExtensionsIn          /* <=> */
)
    {
    DimStyleExtensions          extensions = *pExtensionsIn;
    DimStyleExtensionLinkage    linkage;

    /* The flags modifier bit is not to be trusted!  Its value must be derived
       from the state of the flags themselves.  If any flags are on, switch on the
       modifier bit. */
    if (*(UShort *)&extensions.flags)
        extensions.modifiers |= STYLE_Extension_Flags;
    else
        extensions.modifiers &= ~STYLE_Extension_Flags;

    if (*(UInt32 *)&extensions.flags2)
        extensions.modifiers |= STYLE_Extension_Flags2;
    else
        extensions.modifiers &= ~STYLE_Extension_Flags2;

    if (*(UInt32 *)&extensions.flags3)
        extensions.modifiers |= STYLE_Extension_Flags3;
    else
        extensions.modifiers &= ~STYLE_Extension_Flags3;

    if (extensions.modifiers)
        {
        StatusInt                   status;
        DimStyleExtensionLinkage    *pNewLinkage = NULL, *pLinkage = &linkage;

        dimlinkage_convertToStyleExtensionlLinkage (&linkage, &extensions);

        /* check if there is unknown meta data in linkage that needs to be preserved */
        if (preserveUnknownStyleExtensionsLinkage(&pNewLinkage, &linkage, dimElement))
            pLinkage = pNewLinkage;

        /* now we can delete the old linkage before attaching the new linkage */
        mdlDim_deleteStyleExtension (dimElement);
        
        status = dimElement.AppendElementLinkage (NULL, pLinkage->hdr, (byte*)pLinkage + sizeof(pLinkage->hdr));

        if (NULL != pNewLinkage)
            free (pNewLinkage);

        return  status;
        }

    /* Delete existing linkage */
    mdlDim_deleteStyleExtension (dimElement);

    return  SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* Extract ordinate overrides.
*
* @param        ppLinkageOut    <=> raw data out
* @param        pOverridesOut   <=  linkage
* @param        pElement        =>  get data from this dimension
* @return       SUCCESS if extraction is ok.
* @bsimethod                                                    petri.niiranen  02/01
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt       dimlinkage_extractOrdinateOverrides
(
UInt16              **ppLinkageOut,
DimOrdinateOverrides *pOverridesOut,
ElementHandleCR        dimElement,
int                 *pIndexOut
)
    {
    int                 status = ERROR;
    int                 index = 0;
    DimOrdinateLinkage  linkageHdr;

    if (! isValidDimension (dimElement))
        return  ERROR;

    if (SUCCESS == (status = dimlinkage_extractLinkageByIndex (ppLinkageOut, (DimExtLinkageHdr *) &linkageHdr,
                                                               sizeof (linkageHdr), dimElement,
                                                               LINKAGESUBTYPE_OrdinateOverride, index)))
        {
        if (NULL != pOverridesOut)
            dimlinkage_convertFromOrdinateLinkage (pOverridesOut, &linkageHdr);

        if (NULL != pIndexOut)
            *pIndexOut = index;
        }

    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* Appends overrides to element
*
* @param        dimElement      <=>
* @param        pOverridesIn    => data to add
* @return       addition status
* @bsimethod                                                    petri.niiranen  02/01
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt        dimlinkage_addOrdinateOverride
(
EditElementHandleR      dimElement,
DimOrdinateOverrides*   pOverridesIn
)
    {
    DimOrdinateLinkage     linkage;

    if (0 == pOverridesIn->modifiers)
        return SUCCESS;

    dimlinkage_convertToOrdinateLinkage (&linkage, pOverridesIn);

    return dimElement.AppendElementLinkage (NULL, linkage.hdr, (byte*)&linkage + sizeof(linkage.hdr));
    }

/*---------------------------------------------------------------------------------**//**
* Extract nth text format for given segment.
*
* @param        ppLinkageOut    <= raw data out
* @param        pTextFormatOut  <= string & formats
* @param        pIndexOut       <= data found at this index
* @param        pElement        => element to extract information
* @param        segmentNo       => get from this segment
* @param        nth             => get data from this point
* @return       SUCCESS if data is found.
* @bsimethod                                                    petri.niiranen  02/01
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt       dimlinkage_extractNthTextFormatBySegmentNo
(
UInt16            **ppLinkageOut,
DimFormattedText  **ppTextFormatOut,
int                *pIndexOut,
ElementHandleCR        element,
int                 segmentNo,
int                 nth
)
    {
    StatusInt                   status       = ERROR;
    int                         linkageIndex = 0;
    int                         nthIndex     = 0;
    int                         dataSize     = 0;
    DimFormattedTextLinkage     *pFmtLinkage = NULL;

    if (! isValidDimensionAndSegment (element, segmentNo))
        return  ERROR;

    // At this point we have no idea of the size of data we will receive therefore
    // allocate some space.
    dataSize    = sizeof (*pFmtLinkage) + MAX_SINGLE_FORMATTED_TEXT_LENGTH * sizeof (char);
    dataSize   *= 2;
    pFmtLinkage = (DimFormattedTextLinkage*) _alloca (dataSize);

    while (SUCCESS == (status = dimlinkage_extractLinkageByIndex (ppLinkageOut, (DimExtLinkageHdr *) pFmtLinkage,
                                                                  dataSize, element, LINKAGESUBTYPE_FormattedText, linkageIndex)))
        {
        if (segmentNo == (int) pFmtLinkage->segment)
            {
            if (nth == nthIndex)
                {
                if (ppTextFormatOut)
                    dimlinkage_convertFromTextFormatLinkage (ppTextFormatOut, pFmtLinkage);

                if (pIndexOut)
                    *pIndexOut = linkageIndex;

                break;
                }

            nthIndex++;
            }

        linkageIndex++;
        }

    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* Add text format for given segment.
*
* @param        dimElement      <=>
* @param        pFmt             => format data
* @param        segmentNo        =>
* @return       StatusInt
* @bsimethod                                                    petri.niiranen  06/01
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt    dimlinkage_addTextFormatterBySegmentNo
(
EditElementHandleR  dimElement,
DimFormattedText*   pFmt,
int                 segmentNo
)
    {
    DimFormattedTextLinkage     *pLinkage = NULL;
    if (NULL == pFmt)
        return ERROR;

    size_t  linkageSize = pFmt->GetLinkageMaxNumBytes();
    pLinkage = (DimFormattedTextLinkage*) _alloca (linkageSize);
    dimlinkage_convertToTextFormatLinkage (pLinkage, segmentNo, pFmt);

    return dimElement.AppendElementLinkage (NULL, pLinkage->hdr, (byte*)pLinkage + sizeof(pLinkage->hdr));
    }

/*---------------------------------------------------------------------------------**//**
* Returns the number of given sub type linkages found.
*
* @param        dimElement      <=>
* @param        expectedSubType =>  sub type to count
* @bsimethod                                                    petri.niiranen  02/01
+---------------+---------------+---------------+---------------+---------------+------*/
static int      dimlinkage_getLinkageCount
(
ElementHandleCR dimElement,
int             expectedSubType
)
    {
    int     count = 0;

    while (SUCCESS == dimlinkage_extractLinkageByIndex (NULL, NULL, 0, dimElement, expectedSubType, count))
        count++;

    return  count;
    }

/*=================================================================================**//**
*
* Public functions
*
+===============+===============+===============+===============+===============+======*/
/*---------------------------------------------------------------------------------**//**
* Tests element for V7 incompatible overrides.
* @bsimethod                                                    petri.niiranen  08/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool          BentleyApi::mdlDim_hasV7incompatibleOverrides
(
int         *pNumOverrideLinkages,
ElementHandleCR dimElement
)
    {
    int     subType, nOverrides = 0;

    for (subType = 0; subType < MAX_LinkageSubTypes; subType++)
        {
        nOverrides += dimlinkage_getLinkageCount (dimElement, subType);
        }

    if (pNumOverrideLinkages)
        *pNumOverrideLinkages = nOverrides;

    return (nOverrides != 0);
    }

/*---------------------------------------------------------------------------------**//**
* Deletes overrides for given point
*
* @param        pDimensionIn    <=> dimension to remove overrides.
* @param        pointNo         =>  remove overrides for this point
* @return       SUCCESS if element is valid and point is in range of points.
* @bsimethod                                                    petri.niiranen  02/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt     BentleyApi::mdlDim_deletePointOverride
(
EditElementHandleR     dimElement,
int                 pointNo
)
    {
    int     index;

    if (! isValidDimensionAndPoint (dimElement, pointNo))
        return  ERROR;

    if (SUCCESS == dimlinkage_extractPointOverridesByPointNo (NULL, NULL, &index, dimElement, pointNo))
        dimlinkage_deleteLinkageByIndex (dimElement, LINKAGESUBTYPE_PointOverride, index);

    return  SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* Adds overrides to dimension element.
*
* @param        pDimensionIn    <=> dimension to receive overrides.
* @param        pOverrides      => overrides to apply
* @param        pointNo         => set for point
* @return       SUCCESS if overrides are added.
* @bsimethod                                                    petri.niiranen  02/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt     BentleyApi::mdlDim_setPointOverride
(
EditElementHandleR     dimElement,
DimPointOverrides   *pOverrides,
int                 pointNo
)
    {
    int             status;

    if (! isValidDimensionAndPoint (dimElement, pointNo))
        return  ERROR;

    // delete any existing
    mdlDim_deletePointOverride (dimElement, pointNo);

    status = dimlinkage_addPointOverride (dimElement, pOverrides, pointNo);

#if defined (DEBUG_DIM_OVERRIDES)
    dumpPointOverrides (dimElement, "After addition.");
#endif

    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* Retrieves overrides for point.
*
* @param        pOverridesOut   <=  point overrides
* @param        dimElement      =>  element to extract overrides from
* @param        pointNo         =>  extract overrides for this point
* @return       SUCCESS if extraction is successful.
* @bsimethod                                                    petri.niiranen  02/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt     BentleyApi::mdlDim_getPointOverride
(
DimPointOverrides   *pOverridesOut,
ElementHandleCR dimElement,
int                  pointNo
)
    {
    return  dimlinkage_extractPointOverridesByPointNo (NULL, pOverridesOut, NULL, dimElement, pointNo);
    }

/*---------------------------------------------------------------------------------**//**
* Deletes overrides for given point
*
* @param        pDimensionIn    <=> dimension to remove overrides.
* @param        segmentNo       =>  remove overrides for this segment
* @return       SUCCESS if element is valid and segment is in range of points.
* @bsimethod                                                    petri.niiranen  02/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt     BentleyApi::mdlDim_deleteSegmentOverride
(
EditElementHandleR elementIn,
int             segmentNo
)
    {
    int     index;

    if (! isValidDimensionAndSegment (elementIn, segmentNo))
        return  ERROR;

    if (SUCCESS == dimlinkage_extractSegmentOverridesBySegmentNo (NULL, NULL, &index, elementIn, segmentNo))
        dimlinkage_deleteLinkageByIndex (elementIn, LINKAGESUBTYPE_SegmentOverride, index);

    return  SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* Adds overrides to dimension element.
*
* @param        pDimensionIn    <=> dimension to receive overrides.
* @param        pOverrides      => overrides to apply
* @param        segmentNo       => segment for overrides
* @return       SUCCESS if overrides are added.
* @bsimethod                                                    petri.niiranen  02/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt     BentleyApi::mdlDim_setSegmentOverride
(
EditElementHandleR     elementIn,
DimSegmentOverrides *pOverrides,
int                 segmentNo
)
    {
    StatusInt       status;

    if (! isValidDimensionAndSegment (elementIn, segmentNo))
        return  ERROR;

    // delete any existing
    mdlDim_deleteSegmentOverride (elementIn, segmentNo);

    status = dimlinkage_addSegmentOverride (elementIn, pOverrides, segmentNo);

#if defined (DEBUG_DIM_OVERRIDES)
        {
        char    msg[128];

        sprintf (msg, "Segment (%d): After addition.", segmentNo);
        dumpSegmentOverrides (dimElement, msg);
        }
#endif

    /* mdlDim_dumpDimension (dimElement); */

    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* Retrieves overrides for point.
*
* @param        pOverridesOut   <= segment overrides
* @param        dimElement      =>
* @param        segmentNo       =>
* @return       override extraction status.
* @bsimethod                                                    petri.niiranen  02/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt     BentleyApi::mdlDim_getSegmentOverride
(
DimSegmentOverrides *pOverridesOut,
ElementHandleCR dimElement,
int                 segmentNo
)
    {
    return  dimlinkage_extractSegmentOverridesBySegmentNo (NULL, pOverridesOut, NULL, dimElement, segmentNo);
    }

/*---------------------------------------------------------------------------------**//**
* Deletes overrides for given point
*
* @param        pDimensionIn    <=> dimension to remove overrides.
* @param        segmentNo       =>  remove overrides for this segment
* @return       SUCCESS if element is valid and segment is in range of points.
* @bsimethod                                                    petri.niiranen  02/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt     BentleyApi::mdlDim_deleteSegmentFlagOverride
(
EditElementHandleR dimElement,
int             segmentNo
)
    {
    int     index;

    if (! isValidDimensionAndSegment (dimElement, segmentNo))
        return  ERROR;

    if (SUCCESS == dimlinkage_extractSegmentFlagOverridesBySegmentNo (NULL, NULL, &index, dimElement, segmentNo))
        dimlinkage_deleteLinkageByIndex (dimElement, LINKAGESUBTYPE_SegmentFlagOverride, index);

    return  SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* Adds overrides to dimension element.
*
* @param        pDimensionIn    <=> dimension to receive overrides.
* @param        pOverrides      => overrides to apply
* @param        segmentNo       => segment for overrides
* @return       SUCCESS if overrides are added.
* @bsimethod                                                    petri.niiranen  02/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt     BentleyApi::mdlDim_setSegmentFlagOverride
(
EditElementHandleR         dimElement,
DimSegmentFlagOverrides *pOverrides,
int                     segmentNo
)
    {
    StatusInt       status;

    if (! isValidDimensionAndSegment (dimElement, segmentNo))
        return  ERROR;

    // delete any existing
    mdlDim_deleteSegmentFlagOverride (dimElement, segmentNo);

    status = dimlinkage_addSegmentFlagOverride (dimElement, pOverrides, segmentNo);

#if defined (DEBUG_DIM_OVERRIDES)
    dumpSegmentFlagOverrides (dimElement, "Segment: After addition.");
#endif

    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* Retrieves overrides for point.
*
* @param        pOverridesOut   <= segment overrides
* @param        dimElement      =>
* @param        segmentNo       =>
* @return       override extraction status.
* @bsimethod                                                    petri.niiranen  02/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt     BentleyApi::mdlDim_getSegmentFlagOverride
(
DimSegmentFlagOverrides *pOverridesOut,
ElementHandleCR dimElement,
int                     segmentNo
)
    {
    return  dimlinkage_extractSegmentFlagOverridesBySegmentNo (NULL, pOverridesOut, NULL, dimElement, segmentNo);
    }

/*---------------------------------------------------------------------------------**//**
* Deletes overrides for given dimension
*
* @param        pDimensionIn    <=> dimension to remove overrides.
* @return       SUCCESS if element is valid
* @bsimethod                                                    petri.niiranen  02/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt     BentleyApi::mdlDim_deleteOverallOverride
(
EditElementHandleR dimElement
)
    {
    int     index;

    if (! isValidDimension (dimElement))
        return  ERROR;

    if (SUCCESS == dimlinkage_extractOverallOverrides (NULL, NULL, dimElement, &index))
        {
        dimlinkage_deleteLinkageByIndex (dimElement, LINKAGESUBTYPE_OverallOverride, index);

        dimElement.GetElementP()->SetDynamicRange(false);
        }

    return  SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* Adds overrides to dimension element.
*
* @param        pDimensionIn    <=> dimension to receive overrides.
* @param        pOverrides      => overrides to apply
* @return       SUCCESS if overrides are added.
* @bsimethod                                                    petri.niiranen  02/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt     BentleyApi::mdlDim_setOverallOverride
(
EditElementHandleR dimElement,
DimOverallOverrides *pOverrides
)
    {
    int             status;

    if (! isValidDimension (dimElement))
        return  ERROR;

    // delete any existing
    mdlDim_deleteOverallOverride (dimElement);

    status = dimlinkage_addOverallOverride (dimElement, pOverrides);

#if defined (DEBUG_DIM_OVERRIDES)
    dumpOverallOverrides (dimElement, "Overall: After addition.");
#endif

    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* Deletes style extensions for given dimension
*
* @param        pDimensionIn    <=> dimension to remove extension.
* @return       SUCCESS if element is valid
* @bsimethod                                                    JoshSchifter    09/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt     BentleyApi::mdlDim_deleteStyleExtension
(
EditElementHandleR dimElement
)
    {
    int     index;

    if (! dimStyleEntry_isStyleOrDimension (dimElement))
        return  ERROR;

    if (SUCCESS == dimlinkage_extractStyleExtensions (NULL, NULL, dimElement, &index))
        dimlinkage_deleteLinkageByIndex (dimElement, LINKAGESUBTYPE_StyleExtension, index);

    return  SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* Adds style extensions to dimension element.
*
* @param        pDimensionIn    <=> dimension to receive overrides.
* @param        pOverrides      => overrides to apply
* @return       SUCCESS if overrides are added.
* @bsimethod                                                    JoshSchifter    09/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt     BentleyApi::mdlDim_setStyleExtension
(
EditElementHandleR             dimElement,
DimStyleExtensions const*   pExtensions
)
    {
    int             status;

    if (! dimStyleEntry_isStyleOrDimension (dimElement))
        return  ERROR;

    // Do NOT delete existing linkage - must preserve unknown data in below function:
    status = dimlinkage_addStyleExtension (dimElement, pExtensions);

#if defined (DEBUG_DIM_OVERRIDES)
    dumpStyleExtensions (dimElement, "Style Extensions: After addition.");
#endif

    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* Retrieves overall overrides.
*
* @param        pOverridesOut   <= overall overrides
* @param        dimElement      =>
* @return       override extraction status.
* @bsimethod                                                    petri.niiranen  02/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt     BentleyApi::mdlDim_getOverallOverride
(
DimOverallOverrides     *pOverridesOut,
ElementHandleCR        dimElement
)
    {
    return  dimlinkage_extractOverallOverrides (NULL, pOverridesOut, dimElement, NULL);
    }

/*---------------------------------------------------------------------------------**//**
* Retrieves style extensions.
*
* @param        pExtensionsOut  <= style extensions
* @param        dimElement      =>
* @return       override extraction status.
* @bsimethod                                                    JoshSchifter    09/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt     BentleyApi::mdlDim_getStyleExtension
(
DimStyleExtensions  *pExtensionsOut,
ElementHandleCR        dimElement
)
    {
    return  dimlinkage_extractStyleExtensions (NULL, pExtensionsOut, dimElement, NULL);
    }

/*---------------------------------------------------------------------------------**//**
* Deletes overrides for given dimension
*
* @param        pDimensionIn    <=> dimension to remove overrides.
* @return       SUCCESS if element is valid
* @bsimethod                                                    petri.niiranen  02/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt     BentleyApi::mdlDim_deleteOrdinateOverride
(
EditElementHandleR dimElement
)
    {
    int     index;

    if (! isValidDimension (dimElement))
        return  ERROR;

    if (SUCCESS == dimlinkage_extractOrdinateOverrides (NULL, NULL, dimElement, &index))
        dimlinkage_deleteLinkageByIndex (dimElement, LINKAGESUBTYPE_OrdinateOverride, index);

    return  SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* Adds overrides to dimension element.
*
* @param        pDimensionIn    <=> dimension to receive overrides.
* @param        pOverrides      => overrides to apply
* @return       SUCCESS if overrides are added.
* @bsimethod                                                    petri.niiranen  02/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt     BentleyApi::mdlDim_setOrdinateOverride
(
EditElementHandleR dimElement,
DimOrdinateOverrides *pOverrides
)
    {
    int             status;

    if (! isValidDimension (dimElement))
        return  ERROR;

    // delete any existing
    mdlDim_deleteOrdinateOverride (dimElement);

    status = dimlinkage_addOrdinateOverride (dimElement, pOverrides);

#if defined (DEBUG_DIM_OVERRIDES)
    dumpOrdinateOverrides (dimElement, "Ordinate: After addition.");
#endif

    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* Retrieves overrides for point.
*
* @param        pOverridesOut   <= ordinate overrides
* @param        dimElement      =>
* @return       override extraction status.
* @bsimethod                                                    petri.niiranen  02/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt     BentleyApi::mdlDim_getOrdinateOverride
(
DimOrdinateOverrides    *pOverridesOut,
ElementHandleCR            dimElement
)
    {
    return  dimlinkage_extractOrdinateOverrides (NULL, pOverridesOut, dimElement, NULL);
    }

/*---------------------------------------------------------------------------------**//**
* Set extended text to dimension.
*
* @param        dimElement      <=> dimension
* @param        pText            => text blob
* @param        segmentNo        => text for segment
* @return       StatusInt
* @bsimethod                                                    petri.niiranen  06/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt     BentleyApi::mdlDim_setText
(
EditElementHandleR dimElement,
DimMLText   *pText,
int         segmentNo
)
    {
    int                 i;
    DimFormattedText    *pFmt  = NULL;
    StatusInt           status = SUCCESS;

    if (! isValidDimensionAndSegment (dimElement, segmentNo) || NULL == pText)
        return  ERROR;

    // delete any existing
    mdlDim_deleteText (dimElement, segmentNo);

    i = 0;
    // for reading efficiency use traverse
    while (NULL    != (pFmt = mdlDimText_getFormatter (pText, i)) &&
           SUCCESS == dimlinkage_addTextFormatterBySegmentNo (dimElement, pFmt, segmentNo))
        {
        // initialize for next loop
        pFmt = NULL;
        i++;
        }

    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  09/01
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt       setSimpleText
(
DimStrings*     pwDimStrings,
WCharCP         pwSimpleString,
int             iPartType,
int             iSubType
)
    {
    WString *pwString = adim_getSimpleStringPtrByType (pwDimStrings, iPartType, iSubType);

    if (NULL != pwString && wcslen (pwSimpleString))
        {
        pwString->assign(pwSimpleString);
        return  SUCCESS;
        }

    return  ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    11/01
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt    mdlDim_setTextParamsFromElement
(
EditElementHandleR  dimElement,
ElementHandleCR     textElem,
int                 iSegmentNo
)
    {
    if (DIMENSION_ELM != dimElement.GetLegacyType())
        return ERROR;

    if (TEXT_ELM != textElem.GetLegacyType())
        return ERROR;

    /*--------------------------------------------------------------------------
        Extract the TextParams from the text element
    --------------------------------------------------------------------------*/
    BeAssert(textElem.IsValid());
    
    TextElemHandler* textHandler = dynamic_cast<TextElemHandler*> (&textElem.GetHandler());
    if (NULL == textHandler)
        return ERROR;
    
    DPoint2d        textSize;
    TextParamWide   textParam;
    BentleyStatus status;
    if (SUCCESS != (status = textHandler->GetTextParams(textElem, textParam)))
        return status;
    
    if (SUCCESS != (status = textHandler->GetFontSize(textElem, textSize)))
        return status;
    
    DgnTextStylePtr textStyleObj = DgnTextStyle::Create(*dimElement.GetDgnProject());
    DgnTextStylePersistence::Legacy::FromTextParamWide(*textStyleObj, textParam, textSize, 0);
    LegacyTextStyle textStyle = DgnTextStylePersistence::Legacy::ToLegacyStyle(*textStyleObj);

    textStyle.just = static_cast<UInt16>(TextElementJustification::LeftMiddle);
    textStyle.flags.fractions = !dimElement.GetElementCP()->ToDimensionElm().flag.stackFract;  // textParam doesn't have fraction flag
    return DimensionHandler::GetInstance().SetTextStyle (dimElement, &textStyle, true, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  03/02
+---------------+---------------+---------------+---------------+---------------+------*/
static ElementHandle getValidSimpleTextElement (ElementHandleCR textElem)
    {
    ElementHandle textElement;
    
    if (TEXT_NODE_ELM == textElem.GetLegacyType())
        {
        if (0 == textElem.GetElementCP()->ToText_node_2d().componentCount)
            return ElementHandle();

        if (1 == textElem.GetElementCP()->ToText_node_2d().componentCount)
            textElement = ChildElemIter (textElem, ExposeChildrenReason::Count);
        }
    else if (TEXT_ELM == textElem.GetLegacyType())
        textElement = textElem;
    else
        return ElementHandle();
    
    // Refactored isValidForTextCluster
    // Ultimately what matters is the size of the varichar that will get written to the dimension element; the byte count of that string cannot exceed MAX_DIMSTR.
    //  Instead of allocating big buffers and doing string conversions, why not just get it off the text element?
    //  numchars is the byte count of the varichar on the text element, not strictly the number of logical characters.
    //  Additionally, it is the same for 2D and 3D elements.
    
    if (!textElement.IsValid() || TEXT_ELM != textElement.GetLegacyType())
        {return ElementHandle (); }
    
    return ((textElement.GetElementCP ()->ToText_2d().numchars <= MAX_DIMSTR) ? textElement : ElementHandle ());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  09/01
+---------------+---------------+---------------+---------------+---------------+------*/
static void        setSimpleTextElement
(
EditElementHandleR  dimElement,
ElementHandleCR     textElem,
int                 truePointNo,
int                 iSegmentNo,
int                 iPartType,
int                 iSubType,
bool                bBlankToleranceStrings
)
    {
    // make sure no mltexts are left behind
    if ((ADTYPE_TEXT_UPPER == iPartType || ADTYPE_TEXT_SINGLE == iPartType) &&
        ADSUB_NONE == iSubType)
        {
        mdlDim_deleteText (dimElement, iSegmentNo);
        }

    BeAssert(textElem.IsValid());
    
    TextParamWide   textParam;
    BentleyStatus   status;
    if (SUCCESS != (status = TextElemHandler::GetTextParams(textElem, textParam)))
        return;
    
    WString newString;
    if (SUCCESS != TextElemHandler::GetString (textElem, newString))
        return;
    
    // Push the props onto the dimension
    mdlDim_setTextParamsFromElement (dimElement, textElem, iSegmentNo);
    
    // get the existing strings from dimension
    // aka mdlDim_getStrings
    DimStringConfig     config;
    DimStrings          dimStrings;
    DimensionHandler* hdlr = dynamic_cast<DimensionHandler*> (&dimElement.GetHandler());
    hdlr->GetStrings(dimElement, dimStrings, truePointNo, &config);
    // determine which string we got and replace with the new one
    setSimpleText (&dimStrings, newString.GetWCharCP(), iPartType, iSubType);

    // add space chars for tolerance strings to disable display of tolerances:
    if (bBlankToleranceStrings && ADTYPE_TEXT_SINGLE == iPartType && ADSUB_NONE == iSubType)
        {
        static WChar  blankString[] = L" ";
        setSimpleText (&dimStrings, blankString, ADTYPE_TEXT_SINGLE, ADSUB_TOL_UPPER);
        setSimpleText (&dimStrings, blankString, ADTYPE_TEXT_SINGLE, ADSUB_TOL_LOWER);
        }

    // aka mdlDim_setStrings
    mdlDim_deleteTextCluster (dimElement, truePointNo);
    hdlr->InsertStrings (dimElement, dimStrings, truePointNo);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  09/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       DimensionHandler::SetTextFromDescr
(
EditElementHandleR      dimElement,
ElementHandleCR         textElementIn,
bool                    bHasValuePlaceHolder,
int                     iSegmentNo,
int                     iPartType,
int                     iSubType
)
    {
    StatusInt       status = ERROR;
    int             truePointNo;
    DimStrings      dimStrings;
    ElementHandle   textElement;

    if (!textElementIn.IsValid())
        return  ERROR;

    if (SUCCESS != BentleyApi::mdlDim_getTextPointNo (&truePointNo, dimElement, iSegmentNo))
        return  ERROR;

    if ((textElement = getValidSimpleTextElement (textElementIn)).IsValid())
        {
        setSimpleTextElement (dimElement, textElement, truePointNo, iSegmentNo, iPartType, iSubType, false);
        status = SUCCESS;
        }
    else
        {
        DimMLText   *pText = NULL;
        bool        bRejectInput = (ADTYPE_TEXT_LOWER == iPartType ||
                                    ADSUB_TOL_UPPER == iSubType ||
                                    ADSUB_TOL_LOWER == iSubType ||
                                    ADSUB_LIM_UPPER == iSubType ||
                                    ADSUB_LIM_LOWER == iSubType);

        /*----------------------------------------------------------------------
          TR#96375:
          For now, reject non-simple text for secondary (lower) dimension text.
          Eventually, we need to support this, but for now rejecting it is
          better than overridding the primary text.
        ----------------------------------------------------------------------*/

        // reject input if we're trying to use complex stuff for stacked sections.
        if (bRejectInput)
            return  ERROR;

        // we have main text replacement. Clear only main string
        DimensionHandler* hdlr = dynamic_cast<DimensionHandler*> (&dimElement.GetHandler());
        hdlr->GetStrings (dimElement, dimStrings, truePointNo, NULL);
        dimStrings.GetPrimaryStrings()[0].clear();
        mdlDim_deleteTextCluster (dimElement, truePointNo);
        hdlr->InsertStrings (dimElement, dimStrings, truePointNo);

        if (SUCCESS == (status = mdlDimText_create (&pText)) &&
            SUCCESS == (status = mdlDimText_setStringsFromTextNode (pText, textElementIn, bHasValuePlaceHolder)) &&
            SUCCESS == (status = mdlDim_setText (dimElement, pText, iSegmentNo)))
            {};

        if (NULL != pText)
            mdlDimText_free (&pText);
        }

    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* Get text from given segment.
*
* @param        pText       <=
* @param        dimElement  => dimension
* @param        segmentNo   => segment
* @return       StatusInt
* @bsimethod                                                    petri.niiranen  06/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt     BentleyApi::mdlDim_getText
(
DimMLText   *pText,
ElementHandleCR dimElement,
int          segmentNo
)
    {
    DimFormattedText    *pFmt  = NULL;
    int                 nth    = 0;
    StatusInt           status;

    if (! isValidDimensionAndSegment (dimElement, segmentNo) || NULL == pText)
        return  ERROR;

    do
        {
        pFmt = new DimFormattedText ();

        if (SUCCESS == (status = dimlinkage_extractNthTextFormatBySegmentNo (NULL, &pFmt, NULL,
                                                                             dimElement, segmentNo,
                                                                             nth)))
            {
            mdlDimText_addFormatter (pText, pFmt);
            pFmt = NULL;
            nth++;
            }
        else
            {
            if (NULL != pFmt)
                delete (pFmt);
            }
        }
    while (SUCCESS == status);

    return  nth ? SUCCESS : ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* Deletes multiline text from given segment
*
* @param        pDimensionIn    <=> dimension to remove text.
* @param        segmentNo       =>  remove text from this segment
* @return       SUCCESS if element is valid and segment is in range of points.
* @bsimethod                                                    petri.niiranen  02/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt     BentleyApi::mdlDim_deleteText
(
EditElementHandleR dimElement,
int       segmentNo
)
    {
    int     nth;
    int     deleteAtIndex = 0;

    if (! isValidDimensionAndSegment (dimElement, segmentNo))
        return  ERROR;

    nth = 0;
    while (SUCCESS == dimlinkage_extractNthTextFormatBySegmentNo (NULL, NULL, &deleteAtIndex,
                                                                  dimElement, segmentNo,
                                                                  0))
        {
        dimlinkage_deleteLinkageByIndex (dimElement, LINKAGESUBTYPE_FormattedText, deleteAtIndex);
        }

    return  SUCCESS;
    }

/*=================================================================================**//**
*
* DimOverride collection functions
*
+===============+===============+===============+===============+===============+======*/
/*---------------------------------------------------------------------------------**//**
* Clear all overrides from element.
*
* @param        dimElement      <=>
* @bsimethod                                                    petri.niiranen  04/01
+---------------+---------------+---------------+---------------+---------------+------*/
void  BentleyApi::mdlDim_overridesClearAll
(
EditElementHandleR dimElement
)
    {
    int     deleteAll = -1;

    // Remove all linkages to start with
    linkage_deleteLinkageByIndex (dimElement.GetElementP(), LINKAGEID_DimExtensionLinkage, deleteAll);
    }

/*---------------------------------------------------------------------------------**//**
* Extract all override information from element.
*
* @param        ppOverridesOut  <=  collection of overrides
* @param        dimElement      <=>
* @return       SUCCESS if valid element and pointer.
* @see          mdlDim_overridesSet,
*               mdlDim_overridesPointDeleted,
*               mdlDim_overridesPointInserted
*               mdlDim_overridesFreeAll
* @bsimethod                                                    petri.niiranen  02/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt    BentleyApi::mdlDim_overridesGet
(
DimOverrides**  ppOverridesOut,
ElementHandleCR dimElement
)
    {
    StatusInt     status = ERROR;
    if (isValidDimension (dimElement) && NULL != ppOverridesOut)
        {
        DimensionElm const *pDim = &dimElement.GetElementCP()->ToDimensionElm();
        int                     nDimPoints = pDim->nPoints;
        int                     index, slot, dataSize;
        int                     nPoints, nSegments, nSgmtFlgs, nOverall, nOrdinate, nMLTexts, nFmts, nStyleExt;
        DimPointOverrides       point;
        DimSegmentOverrides     segment;
        DimOverallOverrides     overall;
        DimStyleExtensions      styleExt;
        DimSegmentFlagOverrides sgmtflg;
        DimOrdinateOverrides    ordinate;
        DimMLText               *pText     = NULL;
        int                     numSegments = DimensionHandler::GetInstance().GetNumSegments (dimElement);

        // Query data size
        nPoints   = dimlinkage_getLinkageCount (dimElement, LINKAGESUBTYPE_PointOverride);
        nSegments = dimlinkage_getLinkageCount (dimElement, LINKAGESUBTYPE_SegmentOverride);
        nSgmtFlgs = dimlinkage_getLinkageCount (dimElement, LINKAGESUBTYPE_SegmentFlagOverride);
        nOverall  = dimlinkage_getLinkageCount (dimElement, LINKAGESUBTYPE_OverallOverride);
        nStyleExt = dimlinkage_getLinkageCount (dimElement, LINKAGESUBTYPE_StyleExtension);
        nOrdinate = dimlinkage_getLinkageCount (dimElement, LINKAGESUBTYPE_OrdinateOverride);
        nFmts     = dimlinkage_getLinkageCount (dimElement, LINKAGESUBTYPE_FormattedText);
        dataSize  = sizeof (**ppOverridesOut);

        // With texts we don't know the data size in advances therefore allocate
        // array of DimMLTexts to hold data. Default to number of segments.
        nMLTexts  = (0 == nFmts) ? 0 : numSegments;

        // Allocate and initialize
        if (NULL != (*ppOverridesOut = (DimOverrides*) malloc (dataSize)))
            {
            memset (*ppOverridesOut, 0, dataSize);

            (*ppOverridesOut)->nPoints   = nPoints;
            (*ppOverridesOut)->nSegments = nSegments;
            (*ppOverridesOut)->nSgmtFlgs = nSgmtFlgs;
            (*ppOverridesOut)->nOverall  = nOverall;
            (*ppOverridesOut)->nStyleExt = nStyleExt;
            (*ppOverridesOut)->nOrdinate = nOrdinate;
            (*ppOverridesOut)->nMLTexts  = nMLTexts;

            // Set pointer offsets
            (*ppOverridesOut)->pStyleExt = (StyleExtension *)      calloc (nStyleExt, sizeof (*(*ppOverridesOut)->pStyleExt));
            (*ppOverridesOut)->pOverall  = (OverallOverride *)     calloc (nOverall,  sizeof (*(*ppOverridesOut)->pOverall));
            (*ppOverridesOut)->pSegment  = (SegmentOverride *)     calloc (nSegments, sizeof (*(*ppOverridesOut)->pSegment));
            (*ppOverridesOut)->pSgmtFlag = (SegmentFlagOverride *) calloc (nSgmtFlgs, sizeof (*(*ppOverridesOut)->pSgmtFlag));
            (*ppOverridesOut)->pPoint    = (PointOverride *)       calloc (nPoints,   sizeof (*(*ppOverridesOut)->pPoint));
            (*ppOverridesOut)->pOrdinate = (OrdinateOverride *)    calloc (nOrdinate, sizeof (*(*ppOverridesOut)->pOrdinate));

            if (nMLTexts)
                (*ppOverridesOut)->pMLText   = (DimMLTextOverride *)   calloc (nMLTexts,  sizeof (*(*ppOverridesOut)->pMLText));

            // --- Get style extensions ---
            slot = 0;
            for (index = 0; index < nStyleExt; index++)
                {
                if (SUCCESS == mdlDim_getStyleExtension (&styleExt, dimElement))
                    {
                    StyleExtension      *pStyleExt = (*ppOverridesOut)->pStyleExt + slot;

                    pStyleExt->styleExt = styleExt;
                    pStyleExt->unused  = 0;
                    slot++;
                    }
                }

            // --- Get overall overrides ---
            slot = 0;
            for (index = 0; index < nOverall; index++)
                {
                if (SUCCESS == mdlDim_getOverallOverride (&overall, dimElement))
                    {
                    OverallOverride     *pOverall = (*ppOverridesOut)->pOverall + slot;

                    pOverall->overall = overall;
                    pOverall->unused  = 0;
                    slot++;
                    }
                }

            // --- Get segment flag overrides ---
            slot = 0;
            for (index = 0; index < numSegments; index++)
                {
                if (SUCCESS == mdlDim_getSegmentFlagOverride (&sgmtflg, dimElement, index) && slot <= nSgmtFlgs)
                    {
                    SegmentFlagOverride *pSgmtFlag = (*ppOverridesOut)->pSgmtFlag + slot;

                    pSgmtFlag->sgmtflg   = sgmtflg;
                    pSgmtFlag->segmentNo = static_cast<UInt16>(index);
                    slot++;
                    }
                }

            // --- Get segment overrides ---
            slot = 0;
            for (index = 0; index < numSegments; index++)
                {
                if (SUCCESS == mdlDim_getSegmentOverride (&segment, dimElement, index) && slot <= nSegments)
                    {
                    SegmentOverride     *pSegment = (*ppOverridesOut)->pSegment + slot;

                    pSegment->segment   = segment;
                    pSegment->segmentNo =  static_cast<UInt16>(index);
                    slot++;
                    }
                }

            // --- Get point overrides ---
            slot = 0;
            for (index = 0; index < nDimPoints; index++)
                {
                if (SUCCESS == mdlDim_getPointOverride (&point, dimElement, index) && slot <= nPoints)
                    {
                    PointOverride   *pPoint = (*ppOverridesOut)->pPoint + slot;

                    pPoint->point   = point;
                    pPoint->pointNo =  static_cast<UInt16>(index);
                    slot++;
                    }
                }

            // --- TOOL SPECIFIC ---
            // --- Get ordinate overrides ---
            slot = 0;
            for (index = 0; index < nOrdinate; index++)
                {
                if (SUCCESS == mdlDim_getOrdinateOverride (&ordinate, dimElement))
                    {
                    OrdinateOverride    *pOrdinate = (*ppOverridesOut)->pOrdinate + slot;

                    pOrdinate->ordinate = ordinate;
                    pOrdinate->unused  = 0;
                    slot++;
                    }
                }

            // --- Get texts ---
            slot = 0;
            for (index = 0; index < nMLTexts; index++)
                {
                mdlDimText_create (&pText);

                if (SUCCESS == mdlDim_getText (pText, dimElement, index))
                    {
                    DimMLTextOverride  *pTxt = (*ppOverridesOut)->pMLText + slot;

                    pTxt->pText     = pText;
                    pTxt->segmentNo =  static_cast<UInt16>(index);
                    slot++;
                    }
                else
                    {
                    mdlDimText_free (&pText);
                    }

                pText = NULL;
                }

            status = SUCCESS;
            }
        }

#if defined (DEBUG_DIM_OVERRIDES)
    overridesDump (*ppOverridesOut, " after mdlDim_overridesGet");
#endif

    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* Increment segment and point linkages above new point.
*
* @param        pOverridesIn    <=> override collection
* @see          mdlDim_overridesGet,
*               mdlDim_overridesSet,
*               mdlDim_overridesPointDeleted
*               mdlDim_overridesFreeAll
* @bsimethod                                                    petri.niiranen  02/01
+---------------+---------------+---------------+---------------+---------------+------*/
void     BentleyApi::mdlDim_overridesPointInserted
(
DimOverrides    *pOverridesIn,
int             pointNo
)
    {
    int         index;

    if (pOverridesIn->nPoints)
        {
        for (index = 0; index < pOverridesIn->nPoints; index++)
            {
            PointOverride   *pPoint = pOverridesIn->pPoint + index;

            if (pPoint->pointNo >= pointNo)
                pPoint->pointNo++;
            }
        }

    if (pOverridesIn->nSegments)
        {
        for (index = 0; index < pOverridesIn->nSegments; index++)
            {
            SegmentOverride     *pSegment = pOverridesIn->pSegment + index;

            if (pSegment->segmentNo >= (pointNo - 1))
                pSegment->segmentNo++;
            }
        }

    if (pOverridesIn->nSgmtFlgs)
        {
        for (index = 0; index < pOverridesIn->nSgmtFlgs; index++)
            {
            SegmentFlagOverride     *pSgmtFlag = pOverridesIn->pSgmtFlag + index;

            if (pSgmtFlag->segmentNo >= (pointNo - 1))
                pSgmtFlag->segmentNo++;
            }
        }

    if (pOverridesIn->nMLTexts)
        {
        for (index = 0; index < pOverridesIn->nMLTexts; index++)
            {
            DimMLTextOverride  *pTxt = pOverridesIn->pMLText + index;

            if (pTxt->segmentNo >= (pointNo - 1))
                pTxt->segmentNo++;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* Sets override information to element. This function should only be used in context
* with <code>mdlDim_overridesGet</code>. This implementation is very robust and
* intended only for restoring extracted data. It clears all existing override linkages
* before adding a new one.
*
* @param        dimElement      <=> element to set overrides to
* @param        pOverridesIn    => override collection to apply
* @return       SUCCESS if element is valid.
* @see          mdlDim_overridesGet,
*               mdlDim_overridesPointDeleted,
*               mdlDim_overridesPointInserted
*               mdlDim_overridesFreeAll
* @bsimethod                                                    petri.niiranen  02/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt    BentleyApi::mdlDim_overridesSet
(
EditElementHandleR elementIn,
DimOverrides    *pOverridesIn
)
    {
    int     status    = ERROR;
    int     index;

#if defined (DEBUG_DIM_OVERRIDES)
    overridesDump (pOverridesIn, " into mdlDim_overridesSet");
#endif
    
    if (isValidDimension (elementIn) && NULL != pOverridesIn)
        {
        mdlDim_overridesClearAll (elementIn);

        if (pOverridesIn->nPoints)
            {
            for (index = 0; index < pOverridesIn->nPoints; index++)
                {
                PointOverride   *pPoint = pOverridesIn->pPoint + index;

                if (! pPoint->hdr.deleted)
                    mdlDim_setPointOverride (elementIn, &pPoint->point, pPoint->pointNo);
                }
            }

        if (pOverridesIn->nSegments)
            {
            for (index = 0; index < pOverridesIn->nSegments; index++)
                {
                SegmentOverride *pSegment = pOverridesIn->pSegment + index;

                if (! pSegment->hdr.deleted)
                    mdlDim_setSegmentOverride (elementIn, &pSegment->segment, pSegment->segmentNo);
                }
            }

        if (pOverridesIn->nSgmtFlgs)
            {
            for (index = 0; index < pOverridesIn->nSgmtFlgs; index++)
                {
                SegmentFlagOverride *pSgmtFlag = pOverridesIn->pSgmtFlag + index;

                if (! pSgmtFlag->hdr.deleted)
                    mdlDim_setSegmentFlagOverride (elementIn, &pSgmtFlag->sgmtflg, pSgmtFlag->segmentNo);
                }
            }

        if (pOverridesIn->nStyleExt)
            {
            for (index = 0; index < pOverridesIn->nStyleExt; index++)
                {
                StyleExtension  *pStyleExt = pOverridesIn->pStyleExt + index;

                if (! pStyleExt->hdr.deleted)
                    mdlDim_setStyleExtension (elementIn, &pStyleExt->styleExt);
                }
            }

        if (pOverridesIn->nOverall)
            {
            for (index = 0; index < pOverridesIn->nOverall; index++)
                {
                OverallOverride     *pOverall = pOverridesIn->pOverall + index;

                if (! pOverall->hdr.deleted)
                    mdlDim_setOverallOverride (elementIn, &pOverall->overall);
                }
            }

        if (pOverridesIn->nOrdinate)
            {
            for (index = 0; index < pOverridesIn->nOrdinate; index++)
                {
                OrdinateOverride     *pOrdinate = pOverridesIn->pOrdinate + index;

                if (! pOrdinate->hdr.deleted)
                    mdlDim_setOrdinateOverride (elementIn, &pOrdinate->ordinate);
                }
            }

        if (pOverridesIn->nMLTexts)
            {
            for (index = 0; index < pOverridesIn->nMLTexts; index++)
                {
                DimMLTextOverride  *pTxt = pOverridesIn->pMLText + index;

                if (!pTxt->hdr.deleted)
                    mdlDim_setText (elementIn, pTxt->pText, pTxt->segmentNo);
                }
            }

        status = SUCCESS;
        }

    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* Frees any memory allocated for override collection.
*
* @param        ppOverridesIn   <=> override collection to free
* @see          mdlDim_overridesGet
* @bsimethod                                                    petri.niiranen  02/01
+---------------+---------------+---------------+---------------+---------------+------*/
void  BentleyApi::mdlDim_overridesFreeAll
(
DimOverrides    **ppOverridesIn
)
    {
    if (NULL == ppOverridesIn ||
        NULL == *ppOverridesIn)
        return  ;

    if (NULL != (*ppOverridesIn)->pOverall)
        {
        free ((*ppOverridesIn)->pOverall);
        (*ppOverridesIn)->pOverall = NULL;
        }

    if (NULL != (*ppOverridesIn)->pSegment)
        {
        free ((*ppOverridesIn)->pSegment);
        (*ppOverridesIn)->pSegment = NULL;
        }

    if (NULL != (*ppOverridesIn)->pSgmtFlag)
        {
        free ((*ppOverridesIn)->pSgmtFlag);
        (*ppOverridesIn)->pSgmtFlag = NULL;
        }

    if (NULL != (*ppOverridesIn)->pStyleExt)
        {
        free ((*ppOverridesIn)->pStyleExt);
        (*ppOverridesIn)->pStyleExt = NULL;
        }

    if (NULL != (*ppOverridesIn)->pPoint)
        {
        free ((*ppOverridesIn)->pPoint);
        (*ppOverridesIn)->pPoint = NULL;
        }

    if (NULL != (*ppOverridesIn)->pOrdinate)
        {
        free ((*ppOverridesIn)->pOrdinate);
        (*ppOverridesIn)->pOrdinate = NULL;
        }

    if (NULL != (*ppOverridesIn)->pMLText)
        {
        DimMLTextOverride  *pTxt = NULL;
        int                 slot  = 0;

        for (slot = 0; slot < (*ppOverridesIn)->nMLTexts; slot++)
            {
            pTxt = (*ppOverridesIn)->pMLText + slot;
            mdlDimText_free (&pTxt->pText);
            }

        free ((*ppOverridesIn)->pMLText);
        (*ppOverridesIn)->pMLText = NULL;
        }

    /*-----------------------------------------------------------------------------------
        This should be last - free entire override collection
    -----------------------------------------------------------------------------------*/
    if (NULL != *ppOverridesIn)
        {
        free (*ppOverridesIn);
        (*ppOverridesIn) = NULL;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  02/01
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    isOverride
(
UInt32  modifiers,
int     field
)
    {
    return  (modifiers & field) ? true : false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  02/02
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt   clearPropertyBit
(
      UInt32*   pModifiers,
const UInt32    propertyField
)
    {
    StatusInt   status = ERROR;

    if (NULL != pModifiers)
        {
        *pModifiers &= ~propertyField;
        status = SUCCESS;
        }

    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* Return pointer to point override.
*
* @param        pOverridesIn    <=>
* @param        pointNo         =>
* @bsimethod                                                    petri.niiranen  02/01
+---------------+---------------+---------------+---------------+---------------+------*/
static DimPointOverrides       *getDimPointOverridesByPointNo
(
DimOverrides    *pOverridesIn,
int             pointNo
)
    {
    if (NULL != pOverridesIn && pOverridesIn->nPoints)
        {
        int     nth, truePntNo;

        truePntNo = pointNo < 0 ? 0 : pointNo;

        for (nth = 0; nth < pOverridesIn->nPoints; nth++)
            {
            PointOverride   *pPoint = pOverridesIn->pPoint + nth;

            if (!pPoint->hdr.deleted && pPoint->pointNo == truePntNo)
                return  &pPoint->point;
            }
        }

    return  NULL;
    }

/*---------------------------------------------------------------------------------**//**
* Return pointer to segment override.
*
* @param        pOverridesIn    <=>
* @param        segmentNo       =>
* @bsimethod                                                    petri.niiranen  02/01
+---------------+---------------+---------------+---------------+---------------+------*/
static DimSegmentOverrides     *getDimSegmentOverridesBySegmentNo
(
DimOverrides    *pOverridesIn,
int             segmentNo
)
    {
    if (NULL != pOverridesIn && pOverridesIn->nSegments)
        {
        int     nth, trueSegNo;

        trueSegNo = segmentNo < 0 ? 0 : segmentNo;

        for (nth = 0; nth < pOverridesIn->nSegments; nth++)
            {
            SegmentOverride   *pSegment = pOverridesIn->pSegment + nth;

            if (!pSegment->hdr.deleted && pSegment->segmentNo == trueSegNo)
                return  &pSegment->segment;
            }
        }

    return  NULL;
    }

/*---------------------------------------------------------------------------------**//**
* Return pointer to segment flag override.
*
* @param        pOverridesIn    <=>
* @param        segmentNo       =>
* @bsimethod                                                    petri.niiranen  02/01
+---------------+---------------+---------------+---------------+---------------+------*/
static DimSegmentFlagOverrides         *getDimSegmentFlagOverridesBySegmentNo
(
DimOverrides    *pOverridesIn,
int             segmentNo
)
    {
    if (NULL != pOverridesIn && pOverridesIn->nSgmtFlgs)
        {
        int     nth, trueSegNo;

        trueSegNo = segmentNo < 0 ? 0 : segmentNo;

        for (nth = 0; nth < pOverridesIn->nSgmtFlgs; nth++)
            {
            SegmentFlagOverride   *pSgmtFlag = pOverridesIn->pSgmtFlag + nth;

            if (!pSgmtFlag->hdr.deleted && pSgmtFlag->segmentNo == trueSegNo)
                return  &pSgmtFlag->sgmtflg;
            }
        }

    return  NULL;
    }

/*---------------------------------------------------------------------------------**//**
* Return pointer to segment text.
*
* @param        pOverridesIn    <=>
* @param        segmentNo       =>
* @bsimethod                                                    petri.niiranen  06/01
+---------------+---------------+---------------+---------------+---------------+------*/
static DimMLText       *getDimMLTextBySegmentNo
(
DimOverrides    *pOverridesIn,
int             segmentNo
)
    {
    if (NULL != pOverridesIn && pOverridesIn->nMLTexts)
        {
        int     nth, trueSegNo;

        trueSegNo = segmentNo < 0 ? 0 : segmentNo;

        for (nth = 0; nth < pOverridesIn->nMLTexts; nth++)
            {
            DimMLTextOverride  *pTxt = pOverridesIn->pMLText + nth;

            if (!pTxt->hdr.deleted && pTxt->segmentNo == trueSegNo)
                return  pTxt->pText;
            }
        }

    return  NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  06/01
+---------------+---------------+---------------+---------------+---------------+------*/
static bool        point_getWitnessExtend
(
DimPointOverrides   *pPoint,
double              *pProperty
)
    {
    bool        bIsOverride = false;

    if (NULL != pPoint)
        if (bIsOverride = isOverride (pPoint->modifiers, POINT_Override_WitnessExtend))
            if (pProperty)
                *pProperty = pPoint->witnessExtend;

    return  bIsOverride;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  06/01
+---------------+---------------+---------------+---------------+---------------+------*/
static bool        point_getWitnessOffset
(
DimPointOverrides   *pPoint,
double              *pProperty
)
    {
    bool        bIsOverride = false;

    if (NULL != pPoint)
        if (bIsOverride = isOverride (pPoint->modifiers, POINT_Override_WitnessOffset))
            if (pProperty)
                *pProperty = pPoint->witnessOffset;

    return  bIsOverride;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  06/01
+---------------+---------------+---------------+---------------+---------------+------*/
static bool        point_getWitnessColor
(
DimPointOverrides   *pPoint,
UInt32              *pProperty
)
    {
    bool        bIsOverride = false;

    if (NULL != pPoint)
        if (bIsOverride = isOverride (pPoint->modifiers, POINT_Override_WitnessColor))
            if (pProperty)
                *pProperty = pPoint->color;

    return  bIsOverride;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  06/01
+---------------+---------------+---------------+---------------+---------------+------*/
static bool        point_getWitnessWeight
(
DimPointOverrides   *pPoint,
UInt32              *pProperty
)
    {
    bool        bIsOverride = false;

    if (NULL != pPoint)
        if (bIsOverride = isOverride (pPoint->modifiers, POINT_Override_WitnessWeight))
            if (pProperty)
                *pProperty = pPoint->weight;

    return  bIsOverride;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  06/01
+---------------+---------------+---------------+---------------+---------------+------*/
static bool        point_getWitnessStyle
(
DimPointOverrides   *pPoint,
UInt32              *pProperty
)
    {
    bool        bIsOverride = false;

    if (NULL != pPoint)
        if (bIsOverride = isOverride (pPoint->modifiers, POINT_Override_WitnessStyle))
            if (pProperty)
                *pProperty = pPoint->style;

    return  bIsOverride;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  06/01
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    segment_getTextRotation
(
DimSegmentOverrides *pSegment,
double              *pProperty
)
    {
    bool        bIsOverride = false;

    if (NULL != pSegment)
        if (bIsOverride = isOverride (pSegment->modifiers, SEGMENT_Override_TextRotation))
            if (pProperty)
                *pProperty = pSegment->textRotation;

    return  bIsOverride;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar 11/03
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    segment_getCurveStartTangent
(
DimSegmentOverrides *pSegment,
DPoint3d            *pProperty
)
    {
    bool        bIsOverride = false;

    if (NULL != pSegment)
        if (bIsOverride = isOverride (pSegment->modifiers, SEGMENT_Override_CurveStartTangent))
            if (pProperty)
                *pProperty = pSegment->curveStartTangent;

    return  bIsOverride;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar 11/03
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    segment_getCurveEndTangent
(
DimSegmentOverrides *pSegment,
DPoint3d            *pProperty
)
    {
    bool        bIsOverride = false;

    if (NULL != pSegment)
        if (bIsOverride = isOverride (pSegment->modifiers, SEGMENT_Override_CurveEndTangent))
            if (pProperty)
                *pProperty = pSegment->curveEndTangent;

    return  bIsOverride;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  06/01
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    segment_getTextJustification
(
DimSegmentOverrides *pSegment,
UInt16              *pProperty
)
    {
    bool        bIsOverride = false;

    if (NULL != pSegment)
        if (bIsOverride = isOverride (pSegment->modifiers, SEGMENT_Override_TextJustification))
            if (pProperty)
                *pProperty = pSegment->textJustification;

    return  bIsOverride;
    }

/*---------------------------------------------------------------------------------**//**
* Segment flags struct has no other members than modifiers. All members are of
* bool    type so they only hold true of false. If the modifier flag
* is on, then the property is set.
* @bsimethod                                                    petri.niiranen  06/01
+---------------+---------------+---------------+---------------+---------------+------*/
static bool     segmentflag_getBoolProperty
(
DimSegmentFlagOverrides *pSgmtFlag,
bool                    *pProperty,
int                     field
)
    {
    bool        bIsOverride = false;

    if (NULL != pSgmtFlag)
        if (bIsOverride = isOverride (pSgmtFlag->modifiers, field))
            if (pProperty)
                *pProperty = bIsOverride;

    return  bIsOverride;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  06/01
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    segmentflag_getUnderlineText
(
DimSegmentFlagOverrides *pSgmtFlag,
bool                    *pProperty
)
    {
    return  segmentflag_getBoolProperty (pSgmtFlag, pProperty, SEGMENTFLAG_Override_UnderlineText);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  06/01
+---------------+---------------+---------------+---------------+---------------+------*/
static bool        segmentflag_getSuppressLeftDimLine
(
DimSegmentFlagOverrides *pSgmtFlag,
bool                    *pProperty
)
    {
    return  segmentflag_getBoolProperty (pSgmtFlag, pProperty, SEGMENTFLAG_Override_SuppressLeftDimLine);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  06/01
+---------------+---------------+---------------+---------------+---------------+------*/
static bool        segmentflag_getSuppressRightDimLine
(
DimSegmentFlagOverrides *pSgmtFlag,
bool                    *pProperty
)
    {
    return  segmentflag_getBoolProperty (pSgmtFlag, pProperty, SEGMENTFLAG_Override_SuppressRightDimLine);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  06/01
+---------------+---------------+---------------+---------------+---------------+------*/
static bool     segmentflag_isPrimaryReference
(
DimSegmentFlagOverrides *pSgmtFlag,
bool                    *pProperty
)
    {
    return  segmentflag_getBoolProperty (pSgmtFlag, pProperty, SEGMENTFLAG_Override_PrimaryIsReference);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    petri.niiranen  06/01
+---------------+---------------+---------------+---------------+---------------+------*/
static bool     segmentflag_isSecondaryReference
(
DimSegmentFlagOverrides *pSgmtFlag,
bool                    *pProperty
)
    {
    return  segmentflag_getBoolProperty (pSgmtFlag, pProperty, SEGMENTFLAG_Override_SecondaryIsReference);
    }

/*=================================================================================**//**
*
* Semi-Public API
*
+===============+===============+===============+===============+===============+======*/
/*---------------------------------------------------------------------------------**//**
* Clear segment flag property.
*
* @param        pOverridesIn    <=>
* @param        segmentNo       =>
* @param        propertyField   =>
* @return   SUCCESS, if override exist and got cleared.
* @bsimethod                                                    petri.niiranen  02/02
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt     BentleyApi::mdlDim_overridesClearSegmentPropertyBit
(
DimOverrides*   pOverridesIn,
const int       segmentNo,
const UInt32    propertyField
)
    {
    StatusInt                status = ERROR;
    DimSegmentOverrides*     pSgmt  = NULL;

    if (NULL != (pSgmt = getDimSegmentOverridesBySegmentNo (pOverridesIn, segmentNo)))
        {
        status = clearPropertyBit (&pSgmt->modifiers, propertyField);
        }

    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* Get witnessline property.
*
* @param        pProperty       <=
* @param        pOverridesIn    =>
* @param        pointNo         =>
* @param        defaultValueIn  =>
* @return       true if override exists
* @bsimethod                                                    petri.niiranen  02/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool     BentleyApi::mdlDim_overridesGetPointWitnessExtend
(
double          *pProperty,
DimOverrides    *pOverridesIn,
int             pointNo,
double          defaultValueIn
)
    {
    if (pProperty)
        *pProperty = defaultValueIn;

    return  point_getWitnessExtend (getDimPointOverridesByPointNo (pOverridesIn, pointNo), pProperty);
    }

/*---------------------------------------------------------------------------------**//**
* Get witnessline property.
*
* @param        pProperty       <=
* @param        pOverridesIn    =>
* @param        pointNo         =>
* @param        defaultValueIn  =>
* @return       true if override exists
* @bsimethod                                                    petri.niiranen  02/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool     BentleyApi::mdlDim_overridesGetPointWitnessOffset
(
double          *pProperty,
DimOverrides    *pOverridesIn,
int             pointNo,
double          defaultValueIn
)
    {
    if (pProperty)
        *pProperty = defaultValueIn;

    return  point_getWitnessOffset (getDimPointOverridesByPointNo (pOverridesIn, pointNo), pProperty);
    }

/*---------------------------------------------------------------------------------**//**
* Get witnessline property.
*
* @param        pColor          <=
* @param        pOverridesIn    =>
* @param        pointNo         =>
* @param        defaultValueIn  =>
* @return       true if override exists
* @bsimethod                                                    petri.niiranen  02/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool     BentleyApi::mdlDim_overridesGetPointWitnessColor
(
UInt32          *pProperty,
DimOverrides    *pOverridesIn,
int             pointNo,
UInt32          defaultValueIn
)
    {
    if (pProperty)
        *pProperty = defaultValueIn;

    return  point_getWitnessColor (getDimPointOverridesByPointNo (pOverridesIn, pointNo), pProperty);
    }

/*---------------------------------------------------------------------------------**//**
* Get witnessline property.
*
* @param        pProperty       <=
* @param        pOverridesIn    =>
* @param        pointNo         =>
* @param        defaultValueIn  =>
* @return       true if override exists
* @bsimethod                                                    petri.niiranen  02/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool     BentleyApi::mdlDim_overridesGetPointWitnessWeight
(
UInt32          *pProperty,
DimOverrides    *pOverridesIn,
int             pointNo,
UInt32          defaultValueIn
)
    {
    if (pProperty)
        *pProperty = defaultValueIn;

    return  point_getWitnessWeight (getDimPointOverridesByPointNo (pOverridesIn, pointNo), pProperty);
    }

/*---------------------------------------------------------------------------------**//**
* Get witnessline property.
*
* @param        pProperty       <=
* @param        pOverridesIn    =>
* @param        pointNo         =>
* @param        defaultValueIn  =>
* @return       true if override exists
* @bsimethod                                                    petri.niiranen  02/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool     BentleyApi::mdlDim_overridesGetPointWitnessStyle
(
long            *pProperty,
DimOverrides    *pOverridesIn,
int             pointNo,
long            defaultValueIn
)
    {
    if (pProperty)
        *pProperty = defaultValueIn;

    return  point_getWitnessStyle (getDimPointOverridesByPointNo (pOverridesIn, pointNo), (UInt32*) pProperty);
    }

/*---------------------------------------------------------------------------------**//**
* Get segment property.
*
* @param        pProperty       <=
* @param        pOverridesIn    =>
* @param        segmentNo       =>
* @param        defaultValueIn  =>
* @return       true if override exists
* @bsimethod                                                    petri.niiranen  02/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool     BentleyApi::mdlDim_overridesGetSegmentTextRotation
(
double          *pProperty,
DimOverrides    *pOverridesIn,
int             segmentNo,
double          defaultValueIn
)
    {
    if (pProperty)
        *pProperty = defaultValueIn;

    return  segment_getTextRotation (getDimSegmentOverridesBySegmentNo (pOverridesIn, segmentNo), pProperty);
    }

/*---------------------------------------------------------------------------------**//**
* Get segment property.
*
* @param        pProperty       <=
* @param        pOverridesIn    =>
* @param        segmentNo       =>
* @param        defaultValueIn  =>
* @return       true if override exists
* @bsimethod                                                    petri.niiranen  02/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool     BentleyApi::mdlDim_overridesGetSegmentTextJustification
(
UInt16          *pProperty,
DimOverrides    *pOverridesIn,
int             segmentNo,
UInt16          defaultValueIn
)
    {
    if (pProperty)
        *pProperty = defaultValueIn;

    return  segment_getTextJustification (getDimSegmentOverridesBySegmentNo (pOverridesIn, segmentNo), pProperty);
    }

/*---------------------------------------------------------------------------------**//**
* Get segment property.
*
* @param        pProperty       <=
* @param        pOverridesIn    =>
* @param        segmentNo       =>
* @param        defaultValueIn  =>
* @return       true if override exists
* @bsimethod                                                    SunandSandurkar 12/03
+---------------+---------------+---------------+---------------+---------------+------*/
bool     BentleyApi::mdlDim_overridesGetSegmentCurveStartTangent
(
DPoint3d        *pProperty,
DimOverrides    *pOverridesIn,
int             segmentNo,
DPoint3d        *defaultValueIn
)
    {
    if (pProperty)
        *pProperty = *defaultValueIn;

    return  segment_getCurveStartTangent (getDimSegmentOverridesBySegmentNo (pOverridesIn, segmentNo), pProperty);
    }

/*---------------------------------------------------------------------------------**//**
* Get segment property.
*
* @param        pProperty       <=
* @param        pOverridesIn    =>
* @param        segmentNo       =>
* @param        defaultValueIn  =>
* @return       true if override exists
* @bsimethod                                                    SunandSandurkar 12/03
+---------------+---------------+---------------+---------------+---------------+------*/
bool     BentleyApi::mdlDim_overridesGetSegmentCurveEndTangent
(
DPoint3d        *pProperty,
DimOverrides    *pOverridesIn,
int             segmentNo,
DPoint3d        *defaultValueIn
)
    {
    if (pProperty)
        *pProperty = *defaultValueIn;

    return  segment_getCurveEndTangent (getDimSegmentOverridesBySegmentNo (pOverridesIn, segmentNo), pProperty);
    }

/*---------------------------------------------------------------------------------**//**
* Get segment flag property.
*
* @param        pProperty       <=
* @param        pOverridesIn    =>
* @param        segmentNo       =>
* @param        defaultValueIn  =>
* @return       true if override exists
* @bsimethod                                                    petri.niiranen  02/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool     BentleyApi::mdlDim_overridesGetSegmentFlagUnderlineText
(
bool            *pProperty,
DimOverrides    *pOverridesIn,
int             segmentNo,
bool            defaultValueIn
)
    {
    if (pProperty)
        *pProperty = defaultValueIn;

    return  segmentflag_getUnderlineText ( \
                    getDimSegmentFlagOverridesBySegmentNo (pOverridesIn, segmentNo), \
                    pProperty);
    }

/*---------------------------------------------------------------------------------**//**
* Get segment flag property.
* @bsimethod                                                    petri.niiranen  02/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool     BentleyApi::mdlDim_overridesGetSegmentFlagSuppressLeftDimLine
(
bool            *pProperty,
DimOverrides    *pOverridesIn,
int             segmentNo,
bool            defaultValueIn
)
    {
    if (pProperty)
        *pProperty = defaultValueIn;

    return  segmentflag_getSuppressLeftDimLine ( \
                    getDimSegmentFlagOverridesBySegmentNo (pOverridesIn, segmentNo), \
                    pProperty);
    }

/*---------------------------------------------------------------------------------**//**
* Get segment flag property.
* @bsimethod                                                    petri.niiranen  02/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool     BentleyApi::mdlDim_overridesGetSegmentFlagSuppressRightDimLine
(
bool            *pProperty,
DimOverrides    *pOverridesIn,
int             segmentNo,
bool            defaultValueIn
)
    {
    if (pProperty)
        *pProperty = defaultValueIn;

    return  segmentflag_getSuppressRightDimLine ( \
                    getDimSegmentFlagOverridesBySegmentNo (pOverridesIn, segmentNo), \
                    pProperty);
    }

/*---------------------------------------------------------------------------------**//**
* Get segment flag property.
* @bsimethod                                                    petri.niiranen  02/01
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool     mdlDim_overridesGetSegmentFlagPrimaryIsReference
(
bool            *pProperty,
DimOverrides    *pOverridesIn,
int             segmentNo,
bool            defaultValueIn
)
    {
    if (pProperty)
        *pProperty = defaultValueIn;

    return  segmentflag_isPrimaryReference ( \
                    getDimSegmentFlagOverridesBySegmentNo (pOverridesIn, segmentNo), \
                    pProperty);
    }

/*---------------------------------------------------------------------------------**//**
* Get segment flag property.
* @bsimethod                                                    petri.niiranen  02/01
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool     mdlDim_overridesGetSegmentFlagSecondaryIsReference
(
bool            *pProperty,
DimOverrides    *pOverridesIn,
int             segmentNo,
bool            defaultValueIn
)
    {
    if (pProperty)
        *pProperty = defaultValueIn;

    return  segmentflag_isSecondaryReference ( \
                    getDimSegmentFlagOverridesBySegmentNo (pOverridesIn, segmentNo), \
                    pProperty);
    }

/*---------------------------------------------------------------------------------**//**
* Get overall property
* @bsimethod                                                    JoshSchifter    08/01
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool     mdlDim_overridesGetOverallRefScale
(
double          *pProperty,
DimOverrides    *pOverridesIn,
double          defaultValueIn
)
    {
    DimOverallOverrides    *pOverall  = NULL;
    bool                    bIsOverride = false;

    if (pOverridesIn->nOverall)
        {
        pOverall   = &pOverridesIn->pOverall->overall;
        bIsOverride = isOverride (pOverall->modifiers, OVERALL_Override_RefScale);

        if (pProperty)
            *pProperty = pOverall->refScale;
        }
    else
        {
        if (pProperty)
            *pProperty = defaultValueIn;
        }

    return  bIsOverride;
    }

/*---------------------------------------------------------------------------------**//**
* Get overall property
* @bsimethod                                                    JoshSchifter    08/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool     BentleyApi::mdlDim_overridesGetOverallAngleQuadrant
(
UInt16          *pProperty,
DimOverrides    *pOverridesIn,
UInt16          defaultValueIn
)
    {
    DimOverallOverrides    *pOverall  = NULL;
    bool                    bIsOverride = false;

    if (pOverridesIn->nOverall)
        {
        pOverall   = &pOverridesIn->pOverall->overall;
        bIsOverride = isOverride (pOverall->modifiers, OVERALL_Override_AngleQuadrant);

        if (pProperty)
            *pProperty = pOverall->angleQuadrant;
        }
    else
        {
        if (pProperty)
            *pProperty = defaultValueIn;
        }

    return  bIsOverride;
    }

/*---------------------------------------------------------------------------------**//**
* Get overall property
* @bsimethod                                                    JoshSchifter    10/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool     BentleyApi::mdlDim_overridesGetOverallSlantAngle
(
double          *pProperty,
DimOverrides    *pOverridesIn,
double          defaultValueIn
)
    {
    DimOverallOverrides    *pOverall  = NULL;
    bool                    bIsOverride = false;

    if (pOverridesIn->nOverall)
        {
        pOverall   = &pOverridesIn->pOverall->overall;
        bIsOverride = isOverride (pOverall->modifiers, OVERALL_Override_SlantAngle);

        if (pProperty)
            *pProperty = pOverall->slantAngle;
        }
    else
        {
        if (pProperty)
            *pProperty = defaultValueIn;
        }

    return  bIsOverride;
    }

/*---------------------------------------------------------------------------------**//**
* Get style extension property
* @bsimethod                                                    JoshSchifter    09/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool     BentleyApi::mdlDim_extensionsGetPrimaryTolAccuracy
(
UInt16          *pProperty,
DimOverrides    *pOverridesIn,
UInt16          defaultValueIn
)
    {
    DimStyleExtensions      *pStyleExt  = NULL;
    bool                    bHasExtension = false;

    if (pOverridesIn->nStyleExt)
        {
        pStyleExt   = &pOverridesIn->pStyleExt->styleExt;
        bHasExtension = isOverride (pStyleExt->modifiers, STYLE_Extension_PrimaryToleranceAccuracy);

        if (pProperty)
            *pProperty = pStyleExt->primaryTolAcc;
        }
    else
        {
        if (pProperty)
            *pProperty = defaultValueIn;
        }

    return  bHasExtension;
    }

/*---------------------------------------------------------------------------------**//**
* Get overall property
* @bsimethod                                                    JoshSchifter    09/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool     BentleyApi::mdlDim_extensionsGetSecondaryTolAccuracy
(
UInt16          *pProperty,
DimOverrides    *pOverridesIn,
UInt16          defaultValueIn
)
    {
    DimStyleExtensions      *pStyleExt  = NULL;
    bool                    bHasExtension = false;

    if (pOverridesIn->nStyleExt)
        {
        pStyleExt       = &pOverridesIn->pStyleExt->styleExt;
        bHasExtension   = isOverride (pStyleExt->modifiers, STYLE_Extension_SecondaryToleranceAccuracy);

        if (pProperty)
            *pProperty = pStyleExt->secondaryTolAcc;
        }
    else
        {
        if (pProperty)
            *pProperty = defaultValueIn;
        }

    return  bHasExtension;
    }

/*---------------------------------------------------------------------------------**//**
* Get overall property
* @bsimethod                                                    SunandSandurkar     09/02
+---------------+---------------+---------------+---------------+---------------+------*/
void     BentleyApi::mdlDim_extensionsGetOrdinateUseDatumValueFlag
(
bool            *pProperty,
DimOverrides    *pOverridesIn,
bool            defaultValueIn
)
    {
    DimStyleExtensions      *pStyleExt  = NULL;

    if (pOverridesIn->nStyleExt)
        {
        pStyleExt       = &pOverridesIn->pStyleExt->styleExt;

        if (pProperty)
            *pProperty = pStyleExt->flags.uOrdUseDatumValue;
        }
    else
        {
        if (pProperty)
            *pProperty = defaultValueIn;
        }
    }

/*---------------------------------------------------------------------------------**//**
* Get overall property
* @bsimethod                                                    SunandSandurkar     09/02
+---------------+---------------+---------------+---------------+---------------+------*/
void     BentleyApi::mdlDim_extensionsGetOrdinateReverseDecrementFlag
(
bool            *pProperty,
DimOverrides    *pOverridesIn,
bool            defaultValueIn
)
    {
    DimStyleExtensions      *pStyleExt  = NULL;

    if (pOverridesIn->nStyleExt)
        {
        pStyleExt       = &pOverridesIn->pStyleExt->styleExt;

        if (pProperty)
            *pProperty = pStyleExt->flags.uOrdDecrementReverse;
        }
    else
        {
        if (pProperty)
            *pProperty = defaultValueIn;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar     08/03
+---------------+---------------+---------------+---------------+---------------+------*/
void     BentleyApi::mdlDim_extensionsGetOrdinateFreeLocationFlag
(
bool            *pProperty,
DimOverrides    *pOverridesIn,
bool            defaultValueIn
)
    {
    DimStyleExtensions      *pStyleExt  = NULL;

    if (pOverridesIn->nStyleExt)
        {
        pStyleExt       = &pOverridesIn->pStyleExt->styleExt;

        if (pProperty)
            *pProperty = pStyleExt->flags2.uOrdFreeLocation;
        }
    else
        {
        if (pProperty)
            *pProperty = defaultValueIn;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          02/04
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt    BentleyApi::mdlDim_setOrdinateFreeLocationFlag
(
EditElementHandleR     dimElement,
bool                *pFlagValue
)
    {
    DimStyleExtensions  extensions;

    if (DIMENSION_ELM != dimElement.GetLegacyType())
        return  DGNHANDLERS_STATUS_BadElement;

    if  (NULL == pFlagValue)
        return  SUCCESS;

    memset (&extensions, 0, sizeof (extensions));

    mdlDim_getStyleExtension (&extensions, dimElement);

    extensions.modifiers |= STYLE_Extension_Flags2;

    extensions.flags2.uOrdFreeLocation = *pFlagValue;

    return  mdlDim_setStyleExtension (dimElement, &extensions);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar     08/03
+---------------+---------------+---------------+---------------+---------------+------*/
Public void     mdlDim_extensionsGetNotUseModelAnnotationScaleFlag
(
bool            *pProperty,
DimOverrides    *pOverridesIn,
bool            defaultValueIn
)
    {
    DimStyleExtensions      *pStyleExt  = NULL;

    if (pOverridesIn->nStyleExt)
        {
        pStyleExt       = &pOverridesIn->pStyleExt->styleExt;

        if (pProperty)
            *pProperty = pStyleExt->flags2.uNotUseModelAnnotationScale;
        }
    else
        {
        if (pProperty)
            *pProperty = defaultValueIn;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar     08/03
+---------------+---------------+---------------+---------------+---------------+------*/
bool          BentleyApi::mdlDim_getNotUseModelAnnotationScaleFlag
(
bool            *pValueOut,
ElementHandleCR    dimElement
)
    {
    bool                    bHasExtensions    = false;
    DimStyleExtensions      extensions;

    if (SUCCESS == mdlDim_getStyleExtension (&extensions, dimElement))
        {
        bHasExtensions = isOverride (extensions.modifiers, STYLE_Extension_Flags2);
        if (pValueOut)
            *pValueOut = bHasExtensions ? extensions.flags2.uNotUseModelAnnotationScale : false;
        }

    return  bHasExtensions;
    }

/*---------------------------------------------------------------------------------**//**
* Get text location override.
*
* @param        pValueOut   <=
* @param        dimElement        =>
* @bsimethod                                                    SunandSandurkar 07/03
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt         BentleyApi::mdlDim_getTextLocation
(
DimStyleProp_Text_Location  *pValueOut,
ElementHandleCR                dimElement
)
    {
    DimStyleExtensions      extensions;

    if (NULL == pValueOut)
        return SUCCESS;

    // Note : This property is a combination of uTextLocation and embed.
    // Refer midimstyle.h for details.
    DimensionElm const* pDim = &dimElement.GetElementCP()->ToDimensionElm();
    if (SUCCESS != mdlDim_getStyleExtension (&extensions, dimElement) ||
        !isOverride (extensions.modifiers, STYLE_Extension_Flags2))
        {
        *pValueOut = (DimStyleProp_Text_Location) (!pDim->flag.embed);
        }
    else
        {
        switch (extensions.flags2.uTextLocation)
            {
            case 2:
                *pValueOut = DIMSTYLE_VALUE_Text_Location_TopLeft;
                break;
            case 1:
                *pValueOut = DIMSTYLE_VALUE_Text_Location_Outside;
                break;
            case 0:
            default:
                *pValueOut = (DimStyleProp_Text_Location) (!pDim->flag.embed);
                break;
            }
        }

    return  SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
@Description    Set the dimension text location override.
*
* @param        dimElement        IN the dimension element to set the value on.
* @param        pValueIn    IN value or NULL to restore default.
* @return       SUCCESS if can be set.
* @Group        "Dimension Functions"
* @bsimethod                                                   SunandSandurkar  10/03
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt         BentleyApi::mdlDim_setTextLocation
(
EditElementHandleR dimElement,
DimStyleProp_Text_Location  *pValueIn
)
    {
    DimStyleExtensions      extensions;

    memset (&extensions, 0, sizeof (extensions));
    mdlDim_getStyleExtension (&extensions, dimElement);
    DimensionElm * dim = &dimElement.GetElementP()->ToDimensionElmR();
    if (NULL != pValueIn)
        {
        extensions.modifiers    |= STYLE_Extension_Flags2;

        // the embed flag needs to be kept upto date if the text location is inline or above
        switch (*pValueIn)
            {
            case DIMSTYLE_VALUE_Text_Location_Inline:
                extensions.flags2.uTextLocation = 0;
                dim->flag.embed = 1;
                break;

            case DIMSTYLE_VALUE_Text_Location_Above:
                extensions.flags2.uTextLocation = 0;
                dim->flag.embed = 0;
                break;

            case DIMSTYLE_VALUE_Text_Location_Outside:
                extensions.flags2.uTextLocation = 1;
                break;

            case DIMSTYLE_VALUE_Text_Location_TopLeft:
                extensions.flags2.uTextLocation = 2;
                break;
            }
        }

    return  mdlDim_setStyleExtension (dimElement, &extensions);
    }

/*---------------------------------------------------------------------------------**//**
* Get text location property
* @bsimethod                                                    SunandSandurkar 12/03
+---------------+---------------+---------------+---------------+---------------+------*/
bool                 BentleyApi::mdlDim_overridesGetTextLocation
(
DimStyleProp_Text_Location  *pValueOut,
DimOverrides                *pOverridesIn,
bool                        embed
)
    {
    DimStyleProp_Text_Location  textLoc     = DIMSTYLE_VALUE_Text_Location_Inline;
    bool                        bIsOverride = false;
    if (!pOverridesIn->nStyleExt)
        {
        textLoc = (DimStyleProp_Text_Location) (!embed);
        }
    else
        {
        DimStyleExtensions *pStyleExt = &pOverridesIn->pStyleExt->styleExt;
        bIsOverride = isOverride (pStyleExt->modifiers, STYLE_Extension_Flags2);

        if (!bIsOverride)
            {
            textLoc = (DimStyleProp_Text_Location) (!embed);
            }
        else
            {
            switch (pStyleExt->flags2.uTextLocation)
                {
                case 2:
                    textLoc = DIMSTYLE_VALUE_Text_Location_TopLeft;
                    break;
                case 1:
                    textLoc = DIMSTYLE_VALUE_Text_Location_Outside;
                    break;
                case 0:
                default:
                    textLoc = (DimStyleProp_Text_Location) (!embed);
                    break;
                }
            }
        }

    if (pValueOut)
        *pValueOut = textLoc;

    return  bIsOverride;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    09/02
+---------------+---------------+---------------+---------------+---------------+------*/
void     BentleyApi::mdlDim_extensionsGetLabelLineSuppressAngleFlag
(
bool            *pProperty,
DimOverrides    *pOverridesIn,
bool            defaultValueIn
)
    {
    DimStyleExtensions      *pStyleExt  = NULL;

    if (pOverridesIn->nStyleExt)
        {
        pStyleExt       = &pOverridesIn->pStyleExt->styleExt;

        if (pProperty)
            *pProperty = pStyleExt->flags.labelLineSupressAngle;
        }
    else
        {
        if (pProperty)
            *pProperty = defaultValueIn;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    09/02
+---------------+---------------+---------------+---------------+---------------+------*/
void     BentleyApi::mdlDim_extensionsGetLabelLineSuppressLengthFlag
(
bool            *pProperty,
DimOverrides    *pOverridesIn,
bool            defaultValueIn
)
    {
    DimStyleExtensions      *pStyleExt  = NULL;

    if (pOverridesIn->nStyleExt)
        {
        pStyleExt       = &pOverridesIn->pStyleExt->styleExt;

        if (pProperty)
            *pProperty = pStyleExt->flags.labelLineSupressLength;
        }
    else
        {
        if (pProperty)
            *pProperty = defaultValueIn;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    09/02
+---------------+---------------+---------------+---------------+---------------+------*/
void     BentleyApi::mdlDim_extensionsGetLabelLineInvertLabelsFlag
(
bool            *pProperty,
DimOverrides    *pOverridesIn,
bool            defaultValueIn
)
    {
    DimStyleExtensions      *pStyleExt  = NULL;

    if (pOverridesIn->nStyleExt)
        {
        pStyleExt       = &pOverridesIn->pStyleExt->styleExt;

        if (pProperty)
            *pProperty = pStyleExt->flags.labelLineInvertLabels;
        }
    else
        {
        if (pProperty)
            *pProperty = defaultValueIn;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar   07/03
+---------------+---------------+---------------+---------------+---------------+------*/
void     BentleyApi::mdlDim_extensionsGetLabelLineAdjacentLabelsFlag
(
bool            *pProperty,
DimOverrides    *pOverridesIn,
bool            defaultValueIn
)
    {
    DimStyleExtensions      *pStyleExt  = NULL;

    if (pOverridesIn->nStyleExt)
        {
        pStyleExt       = &pOverridesIn->pStyleExt->styleExt;

        if (pProperty)
            *pProperty = pStyleExt->flags.uLabelLineAdjacentLabels;
        }
    else
        {
        if (pProperty)
            *pProperty = defaultValueIn;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar    04/03
+---------------+---------------+---------------+---------------+---------------+------*/
Public void     mdlDim_extensionsGetMultiJustVerticalFlag
(
int             *pProperty,
DimOverrides    *pOverridesIn,
int             defaultValueIn
)
    {
    DimStyleExtensions      *pStyleExt  = NULL;

    if (pOverridesIn->nStyleExt)
        {
        pStyleExt       = &pOverridesIn->pStyleExt->styleExt;

        if (pProperty)
            *pProperty = pStyleExt->flags.uMultiJustVertical;
        }
    else
        {
        if (pProperty)
            *pProperty = defaultValueIn;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar    04/03
+---------------+---------------+---------------+---------------+---------------+------*/
void     BentleyApi::mdlDim_extensionsGetNoReduceFractionFlag
(
bool            *pProperty,
DimOverrides    *pOverridesIn,
bool            defaultValueIn
)
    {
    DimStyleExtensions      *pStyleExt  = NULL;

    if (pOverridesIn->nStyleExt)
        {
        pStyleExt       = &pOverridesIn->pStyleExt->styleExt;

        if (pProperty)
            *pProperty = pStyleExt->flags.uNoReduceFraction;
        }
    else
        {
        if (pProperty)
            *pProperty = defaultValueIn;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar    04/03
+---------------+---------------+---------------+---------------+---------------+------*/
void     BentleyApi::mdlDim_extensionsGetNoReduceAltFractionFlag
(
bool            *pProperty,
DimOverrides    *pOverridesIn,
bool            defaultValueIn
)
    {
    DimStyleExtensions      *pStyleExt  = NULL;

    if (pOverridesIn->nStyleExt)
        {
        pStyleExt       = &pOverridesIn->pStyleExt->styleExt;

        if (pProperty)
            *pProperty = pStyleExt->flags.uNoReduceAltFraction;
        }
    else
        {
        if (pProperty)
            *pProperty = defaultValueIn;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar    04/03
+---------------+---------------+---------------+---------------+---------------+------*/
void     BentleyApi::mdlDim_extensionsGetNoReduceTolFractionFlag
(
bool            *pProperty,
DimOverrides    *pOverridesIn,
bool            defaultValueIn
)
    {
    DimStyleExtensions      *pStyleExt  = NULL;

    if (pOverridesIn->nStyleExt)
        {
        pStyleExt       = &pOverridesIn->pStyleExt->styleExt;

        if (pProperty)
            *pProperty = pStyleExt->flags.uNoReduceTolFraction;
        }
    else
        {
        if (pProperty)
            *pProperty = defaultValueIn;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar    04/03
+---------------+---------------+---------------+---------------+---------------+------*/
void     BentleyApi::mdlDim_extensionsGetNoReduceSecFractionFlag
(
bool            *pProperty,
DimOverrides    *pOverridesIn,
bool            defaultValueIn
)
    {
    DimStyleExtensions      *pStyleExt  = NULL;

    if (pOverridesIn->nStyleExt)
        {
        pStyleExt       = &pOverridesIn->pStyleExt->styleExt;

        if (pProperty)
            *pProperty = pStyleExt->flags.uNoReduceSecFraction;
        }
    else
        {
        if (pProperty)
            *pProperty = defaultValueIn;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar    04/03
+---------------+---------------+---------------+---------------+---------------+------*/
void     BentleyApi::mdlDim_extensionsGetNoReduceAltSecFractionFlag
(
bool            *pProperty,
DimOverrides    *pOverridesIn,
bool            defaultValueIn
)
    {
    DimStyleExtensions      *pStyleExt  = NULL;

    if (pOverridesIn->nStyleExt)
        {
        pStyleExt       = &pOverridesIn->pStyleExt->styleExt;

        if (pProperty)
            *pProperty = pStyleExt->flags.uNoReduceAltSecFraction;
        }
    else
        {
        if (pProperty)
            *pProperty = defaultValueIn;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar    04/03
+---------------+---------------+---------------+---------------+---------------+------*/
void     BentleyApi::mdlDim_extensionsGetNoReduceTolSecFractionFlag
(
bool            *pProperty,
DimOverrides    *pOverridesIn,
bool            defaultValueIn
)
    {
    DimStyleExtensions      *pStyleExt  = NULL;

    if (pOverridesIn->nStyleExt)
        {
        pStyleExt       = &pOverridesIn->pStyleExt->styleExt;

        if (pProperty)
            *pProperty = pStyleExt->flags.uNoReduceTolSecFraction;
        }
    else
        {
        if (pProperty)
            *pProperty = defaultValueIn;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar    07/03
+---------------+---------------+---------------+---------------+---------------+------*/
void     BentleyApi::mdlDim_extensionsGetNoteLeaderType
(
int             *pProperty,
DimOverrides    *pOverridesIn,
bool            defaultValueIn
)
    {
    DimStyleExtensions      *pStyleExt  = NULL;

    if (pOverridesIn->nStyleExt)
        {
        pStyleExt       = &pOverridesIn->pStyleExt->styleExt;

        if (pProperty)
            *pProperty = pStyleExt->flags2.uNoteLeaderType;
        }
    else
        {
        if (pProperty)
            *pProperty = defaultValueIn;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/09
+---------------+---------------+---------------+---------------+---------------+------*/
int      BentleyApi::adim_extensionsGetNoteTerminator (DimStyleExtensions const& extensions)
    {
    int value = extensions.flags2.uNoteTerminator;

    if (0 == value)
        value = 1;
    else if (1 == value)
        value = 0;

    return value;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/09
+---------------+---------------+---------------+---------------+---------------+------*/
void      BentleyApi::adim_extensionsSetNoteTerminator (DimStyleExtensions& extensions, int value)
    {
    if (0 == value)
        value = 1;
    else if (1 == value)
        value = 0;

    extensions.flags2.uNoteTerminator = value;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar    07/03
+---------------+---------------+---------------+---------------+---------------+------*/
void     BentleyApi::mdlDim_extensionsGetNoteTerminator
(
int             *pProperty,
DimOverrides    *pOverridesIn,
int             defaultValueIn
)
    {
    if (pOverridesIn->nStyleExt)
        {
        if (pProperty)
            *pProperty = adim_extensionsGetNoteTerminator (pOverridesIn->pStyleExt->styleExt);
        }
    else
        {
        if (pProperty)
            *pProperty = defaultValueIn;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar    07/03
+---------------+---------------+---------------+---------------+---------------+------*/
void     BentleyApi::mdlDim_extensionsGetNoteTerminatorType
(
int             *pProperty,
DimOverrides    *pOverridesIn,
int             defaultValueIn
)
    {
    DimStyleExtensions      *pStyleExt  = NULL;

    if (pOverridesIn->nStyleExt)
        {
        pStyleExt       = &pOverridesIn->pStyleExt->styleExt;

        if (pProperty)
            *pProperty = pStyleExt->flags2.uNoteTerminatorType;
        }
    else
        {
        if (pProperty)
            *pProperty = defaultValueIn;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar    07/03
+---------------+---------------+---------------+---------------+---------------+------*/
void     BentleyApi::mdlDim_extensionsGetNoteTextRotation
(
int             *pProperty,
DimOverrides    *pOverridesIn,
int             defaultValueIn
)
    {
    DimStyleExtensions      *pStyleExt  = NULL;

    if (pOverridesIn->nStyleExt)
        {
        pStyleExt       = &pOverridesIn->pStyleExt->styleExt;

        if (pProperty)
            *pProperty = pStyleExt->flags2.uNoteTextRotation;
        }
    else
        {
        if (pProperty)
            *pProperty = defaultValueIn;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar    07/03
+---------------+---------------+---------------+---------------+---------------+------*/
Public void     mdlDim_extensionsGetNoteHorAttachment
(
int             *pProperty,
DimOverrides    *pOverridesIn,
int             defaultValueIn
)
    {
    DimStyleExtensions      *pStyleExt  = NULL;

    if (pOverridesIn->nStyleExt)
        {
        pStyleExt       = &pOverridesIn->pStyleExt->styleExt;

        if (pProperty)
            *pProperty = pStyleExt->flags2.uNoteHorAttachment;
        }
    else
        {
        if (pProperty)
            *pProperty = defaultValueIn;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar    07/03
+---------------+---------------+---------------+---------------+---------------+------*/
Public void     mdlDim_extensionsGetNoteVerLeftAttachment
(
int             *pProperty,
DimOverrides    *pOverridesIn,
int             defaultValueIn
)
    {
    DimStyleExtensions      *pStyleExt  = NULL;

    if (pOverridesIn->nStyleExt)
        {
        pStyleExt       = &pOverridesIn->pStyleExt->styleExt;

        if (pProperty)
            adim_getMLNoteVerLeftAttachment (pStyleExt, (UInt16 *) pProperty);
        }
    else
        {
        if (pProperty)
            *pProperty = defaultValueIn;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar    07/03
+---------------+---------------+---------------+---------------+---------------+------*/
Public void     mdlDim_extensionsGetNoteVerRightAttachment
(
int             *pProperty,
DimOverrides    *pOverridesIn,
int             defaultValueIn
)
    {
    DimStyleExtensions      *pStyleExt  = NULL;

    if (pOverridesIn->nStyleExt)
        {
        pStyleExt       = &pOverridesIn->pStyleExt->styleExt;

        if (pProperty)
            adim_getMLNoteVerRightAttachment (pStyleExt, (UInt16 *) pProperty);
        }
    else
        {
        if (pProperty)
            *pProperty = defaultValueIn;
        }
    }

/*---------------------------------------------------------------------------------**//**
* Get overall property
* @bsimethod                                                    petri.niiranen  09/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool     BentleyApi::mdlDim_extensionsGetRoundOff
(
double          *pProperty,
DimOverrides    *pOverridesIn,
double          defaultValueIn
)
    {
    DimStyleExtensions      *pStyleExt  = NULL;
    bool                    bHasExtension = false;

    if (pOverridesIn->nStyleExt)
        {
        pStyleExt       = &pOverridesIn->pStyleExt->styleExt;
        bHasExtension   = isOverride (pStyleExt->modifiers, STYLE_Extension_RoundOff);

        if (pProperty)
            *pProperty = pStyleExt->dRoundOff;
        }
    else
        {
        if (pProperty)
            *pProperty = defaultValueIn;
        }

    return  bHasExtension;
    }

/*---------------------------------------------------------------------------------**//**
* Get overall property
* @bsimethod                                                    petri.niiranen  09/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool     BentleyApi::mdlDim_extensionsGetSecondaryRoundOff
(
double          *pProperty,
DimOverrides    *pOverridesIn,
double          defaultValueIn
)
    {
    DimStyleExtensions      *pStyleExt  = NULL;
    bool                    bHasExtension = false;

    if (pOverridesIn->nStyleExt)
        {
        pStyleExt       = &pOverridesIn->pStyleExt->styleExt;
        bHasExtension   = isOverride (pStyleExt->modifiers, STYLE_Extension_SecondaryRoundOff);

        if (pProperty)
            *pProperty = pStyleExt->dSecondaryRoundOff;
        }
    else
        {
        if (pProperty)
            *pProperty = defaultValueIn;
        }

    return  bHasExtension;
    }

/*---------------------------------------------------------------------------------**//**
* Get overall property
* @bsimethod                                                    SunandSandurkar     09/02
+---------------+---------------+---------------+---------------+---------------+------*/
bool     BentleyApi::mdlDim_extensionsGetOrdinateDatumValue
(
double          *pProperty,
DimOverrides    *pOverridesIn,
double          defaultValueIn
)
    {
    DimStyleExtensions      *pStyleExt  = NULL;
    bool                    bHasExtension = false;

    if (pOverridesIn->nStyleExt)
        {
        pStyleExt       = &pOverridesIn->pStyleExt->styleExt;
        bHasExtension   = isOverride (pStyleExt->modifiers, STYLE_Extension_OrdinateDatumValue);

        if (pProperty)
            *pProperty = pStyleExt->dOrdinateDatumValue;
        }
    else
        {
        if (pProperty)
            *pProperty = defaultValueIn;
        }

    return  bHasExtension;
    }

/*---------------------------------------------------------------------------------**//**
* Get overall property
* @bsimethod                                                    SunandSandurkar     10/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool     mdlDim_extensionsGetNoteElbowLength
(
double          *pProperty,
DimOverrides    *pOverridesIn,
double          defaultValueIn
)
    {
    DimStyleExtensions      *pStyleExt  = NULL;
    bool                    bHasExtension = false;

    if (pOverridesIn->nStyleExt)
        {
        pStyleExt       = &pOverridesIn->pStyleExt->styleExt;
        bHasExtension   = isOverride (pStyleExt->modifiers, STYLE_Extension_NoteElbowLength);

        if (pProperty)
            *pProperty = pStyleExt->dNoteElbowLength;
        }
    else
        {
        if (pProperty)
            *pProperty = defaultValueIn;
        }

    return  bHasExtension;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    12/02
+---------------+---------------+---------------+---------------+---------------+------*/
bool     BentleyApi::mdlDim_extensionsGetStackedFractionScale
(
double          *pProperty,
DimOverrides    *pOverridesIn,
double          defaultValueIn
)
    {
    DimStyleExtensions      *pStyleExt  = NULL;
    bool                    bHasExtension = false;

    if (pOverridesIn->nStyleExt)
        {
        pStyleExt       = &pOverridesIn->pStyleExt->styleExt;
        bHasExtension   = isOverride (pStyleExt->modifiers, STYLE_Extension_StackedFractionScale);

        if (pProperty)
            *pProperty = (0.0 < pStyleExt->stackedFractionScale) ? pStyleExt->stackedFractionScale : 1.0;
        }
    else
        {
        if (pProperty)
            *pProperty = defaultValueIn;
        }

    return  bHasExtension;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/04
+---------------+---------------+---------------+---------------+---------------+------*/
bool     BentleyApi::mdlDim_extensionsGetInlineTextLift
(
double          *pProperty,
DimOverrides    *pOverridesIn,
double          defaultValueIn
)
    {
    DimStyleExtensions      *pStyleExt  = NULL;
    bool                    bHasExtension = false;

    if (pOverridesIn->nStyleExt)
        {
        pStyleExt       = &pOverridesIn->pStyleExt->styleExt;
        bHasExtension   = isOverride (pStyleExt->modifiers, STYLE_Extension_InlineTextLift);

        if (pProperty)
            *pProperty = pStyleExt->dInlineTextLift;
        }
    else
        {
        if (pProperty)
            *pProperty = defaultValueIn;
        }

    return  bHasExtension;
    }

/*---------------------------------------------------------------------------------**//**
* Get overall property
* @bsimethod                                                    SunandSandurkar     07/03
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool     mdlDim_extensionsGetNoteLeftMargin
(
double          *pProperty,
DimOverrides    *pOverridesIn,
double          defaultValueIn
)
    {
    DimStyleExtensions      *pStyleExt  = NULL;
    bool                    bHasExtension = false;

    if (pOverridesIn->nStyleExt)
        {
        pStyleExt       = &pOverridesIn->pStyleExt->styleExt;
        bHasExtension   = isOverride (pStyleExt->modifiers, STYLE_Extension_NoteLeftMargin);

        if (pProperty)
            *pProperty = pStyleExt->dNoteLeftMargin;
        }
    else
        {
        if (pProperty)
            *pProperty = defaultValueIn;
        }

    return  bHasExtension;
    }

/*---------------------------------------------------------------------------------**//**
* Get overall property
* @bsimethod                                                    SunandSandurkar     07/03
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool     mdlDim_extensionsGetNoteLowerMargin
(
double          *pProperty,
DimOverrides    *pOverridesIn,
double          defaultValueIn
)
    {
    DimStyleExtensions      *pStyleExt  = NULL;
    bool                    bHasExtension = false;

    if (pOverridesIn->nStyleExt)
        {
        pStyleExt       = &pOverridesIn->pStyleExt->styleExt;
        bHasExtension   = isOverride (pStyleExt->modifiers, STYLE_Extension_NoteLowerMargin);

        if (pProperty)
            *pProperty = pStyleExt->dNoteLowerMargin;
        }
    else
        {
        if (pProperty)
            *pProperty = defaultValueIn;
        }

    return  bHasExtension;
    }

/*---------------------------------------------------------------------------------**//**
* Get overall property
* @bsimethod                                                    SunandSandurkar     07/03
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool     mdlDim_extensionsGetAnnotationScale
(
double          *pProperty,
DimOverrides    *pOverridesIn,
double          defaultValueIn
)
    {
    DimStyleExtensions      *pStyleExt  = NULL;
    bool                    bHasExtension = false;

    if (pOverridesIn->nStyleExt)
        {
        pStyleExt       = &pOverridesIn->pStyleExt->styleExt;
        bHasExtension   = isOverride (pStyleExt->modifiers, STYLE_Extension_AnnotationScale);

        if (pProperty)
            *pProperty = pStyleExt->dAnnotationScale;
        }
    else
        {
        if (pProperty)
            *pProperty = defaultValueIn;
        }

    return  bHasExtension;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar     10/02
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool     mdlDim_extensionsGetNoteFrameScale
(
double          *pProperty,
DimOverrides    *pOverridesIn,
double          defaultValueIn
)
    {
    DimStyleExtensions      *pStyleExt  = NULL;
    bool                    bHasExtension = false;

    if (pOverridesIn->nStyleExt)
        {
        pStyleExt       = &pOverridesIn->pStyleExt->styleExt;
        bHasExtension   = isOverride (pStyleExt->modifiers, STYLE_Extension_NoteFrameScale);

        if (pProperty)
            *pProperty = pStyleExt->dNoteFrameScale;
        }
    else
        {
        if (pProperty)
            *pProperty = defaultValueIn;
        }

    return  bHasExtension;
    }

/*---------------------------------------------------------------------------------**//**
* Get overall property
* @bsimethod                                                    SunandSandurkar     07/03
+---------------+---------------+---------------+---------------+---------------+------*/
bool     BentleyApi::mdlDim_extensionsGetNoteTermChar
(
UInt16          *pProperty,
DimOverrides    *pOverridesIn,
UInt16          defaultValueIn
)
    {
    DimStyleExtensions      *pStyleExt  = NULL;
    bool                    bHasExtension = false;

    if (pOverridesIn->nStyleExt)
        {
        pStyleExt       = &pOverridesIn->pStyleExt->styleExt;
        bHasExtension   = isOverride (pStyleExt->modifiers, STYLE_Extension_NoteTerminatorChar);

        if (pProperty)
            *pProperty = pStyleExt->iNoteTerminatorChar;
        }
    else
        {
        if (pProperty)
            *pProperty = defaultValueIn;
        }

    return  bHasExtension;
    }

/*---------------------------------------------------------------------------------**//**
* Get overall property
* @bsimethod                                                    SunandSandurkar     07/03
+---------------+---------------+---------------+---------------+---------------+------*/
bool     BentleyApi::mdlDim_extensionsGetNoteTermFont
(
UInt32          *pProperty,
DimOverrides    *pOverridesIn,
UInt32          defaultValueIn
)
    {
    DimStyleExtensions      *pStyleExt  = NULL;
    bool                    bHasExtension = false;

    if (pOverridesIn->nStyleExt)
        {
        pStyleExt       = &pOverridesIn->pStyleExt->styleExt;
        bHasExtension   = isOverride (pStyleExt->modifiers, STYLE_Extension_NoteTerminatorFont);

        if (pProperty)
            *pProperty = pStyleExt->iNoteTerminatorFont;
        }
    else
        {
        if (pProperty)
            *pProperty = defaultValueIn;
        }

    return  bHasExtension;
    }

/*---------------------------------------------------------------------------------**//**
* Get ordinate property
* @bsimethod                                                    petri.niiranen  04/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool     BentleyApi::mdlDim_overridesGetOrdinateStartValueX
(
double          *pProperty,
DimOverrides    *pOverridesIn,
double          defaultValueIn
)
    {
    DimOrdinateOverrides    *pOrdinate  = NULL;
    bool                    bIsOverride = false;

    if (pOverridesIn->nOrdinate)
        {
        pOrdinate   = &pOverridesIn->pOrdinate->ordinate;
        bIsOverride = isOverride (pOrdinate->modifiers, ORDINATE_Override_StartValueX);

        if (pProperty)
            *pProperty = pOrdinate->startValueX;
        }
    else
        {
        if (pProperty)
            *pProperty = defaultValueIn;
        }

    return  bIsOverride;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          11/05
+---------------+---------------+---------------+---------------+---------------+------*/
bool     BentleyApi::mdlDim_isMinLeaderIgnored
(
DimOverrides    *pOverridesIn,
bool            defaultValueIn
)
    {
    bool        ignoreMinLeader = defaultValueIn;

    if (pOverridesIn->nStyleExt)
        {
        DimStyleExtensions  *pStyleExt  = &pOverridesIn->pStyleExt->styleExt;

        if (isOverride(pStyleExt->modifiers, STYLE_Extension_Flags3))
            ignoreMinLeader = pStyleExt->flags3.uIgnoreMinLeader;
        }

    return  ignoreMinLeader;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          02/05
+---------------+---------------+---------------+---------------+---------------+------*/
bool     BentleyApi::mdlDim_extensionsGetBncElbowLength
(
double          *pProperty,
DimOverrides    *pOverridesIn,
double          defaultValueIn
)
    {
    DimStyleExtensions      *pStyleExt  = NULL;
    bool                    bHasExtension = false;

    if (pProperty)
        *pProperty = defaultValueIn;

    if (pOverridesIn->nStyleExt)
        {
        pStyleExt       = &pOverridesIn->pStyleExt->styleExt;
        bHasExtension   = isOverride (pStyleExt->modifiers, STYLE_Extension_Flags3);

        if (bHasExtension && pStyleExt->flags3.uUseBncElbowLength)
            {
            bHasExtension   = isOverride (pStyleExt->modifiers, STYLE_Extension_BncElbowLength);

            if (pProperty)
                *pProperty = pStyleExt->dBncElbowLength;
            }
        }

    return  bHasExtension;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          02/05
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool     mdlDim_extensionsGetFitOption
(
UInt16          *pProperty,
DimOverrides    *pOverridesIn,
UInt16          defaultValueIn
)
    {
    DimStyleExtensions      *pStyleExt  = NULL;
    bool                    bHasExtension = false;

    if (pOverridesIn->nStyleExt)
        {
        pStyleExt       = &pOverridesIn->pStyleExt->styleExt;
        bHasExtension   = isOverride (pStyleExt->modifiers, STYLE_Extension_Flags3);

        if (pProperty)
            *pProperty = pStyleExt->flags3.uFitOption;
        }
    else
        {
        if (pProperty)
            *pProperty = defaultValueIn;
        }

    return  bHasExtension;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          02/05
+---------------+---------------+---------------+---------------+---------------+------*/
bool     BentleyApi::mdlDim_extensionsGetSuppressUnfitTerminatorsFlag
(
bool            *pProperty,
DimOverrides    *pOverridesIn,
bool             defaultValueIn
)
    {
    DimStyleExtensions      *pStyleExt  = NULL;
    bool                    bHasExtension = false;

    if (pOverridesIn->nStyleExt)
        {
        pStyleExt       = &pOverridesIn->pStyleExt->styleExt;
        bHasExtension   = isOverride (pStyleExt->modifiers, STYLE_Extension_Flags3);

        if (pProperty)
            *pProperty = pStyleExt->flags3.uNoTermsOutside;
        }
    else
        {
        if (pProperty)
            *pProperty = defaultValueIn;
        }

    return  bHasExtension;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          10/05
+---------------+---------------+---------------+---------------+---------------+------*/
bool     BentleyApi::mdlDim_extensionsGetTightFitTextAbove
(
bool            *pProperty,
DimOverrides    *pOverridesIn,
bool             defaultValueIn
)
    {
    DimStyleExtensions      *pStyleExt  = NULL;
    bool                    bHasExtension = false;

    if (pOverridesIn->nStyleExt)
        {
        pStyleExt       = &pOverridesIn->pStyleExt->styleExt;
        bHasExtension   = isOverride (pStyleExt->modifiers, STYLE_Extension_Flags3);

        if (pProperty)
            *pProperty = pStyleExt->flags3.uTightFitTextAbove;
        }
    else
        {
        if (pProperty)
            *pProperty = defaultValueIn;
        }

    return  bHasExtension;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          10/05
+---------------+---------------+---------------+---------------+---------------+------*/
bool     BentleyApi::mdlDim_extensionsGetFitInclinedTextBox
(
bool            *pProperty,
DimOverrides    *pOverridesIn,
bool             defaultValueIn
)
    {
    DimStyleExtensions      *pStyleExt  = NULL;
    bool                    bHasExtension = false;

    if (pOverridesIn->nStyleExt)
        {
        pStyleExt       = &pOverridesIn->pStyleExt->styleExt;
        bHasExtension   = isOverride (pStyleExt->modifiers, STYLE_Extension_Flags3);

        if (pProperty)
            *pProperty = pStyleExt->flags3.uFitInclinedTextBox;
        }
    else
        {
        if (pProperty)
            *pProperty = defaultValueIn;
        }

    return  bHasExtension;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          10/05
+---------------+---------------+---------------+---------------+---------------+------*/
bool     BentleyApi::mdlDim_extensionsGetExtendDimLineUnderText
(
bool            *pProperty,
DimOverrides    *pOverridesIn,
bool             defaultValueIn
)
    {
    DimStyleExtensions      *pStyleExt  = NULL;
    bool                    bHasExtension = false;

    if (pOverridesIn->nStyleExt)
        {
        pStyleExt       = &pOverridesIn->pStyleExt->styleExt;
        bHasExtension   = isOverride (pStyleExt->modifiers, STYLE_Extension_Flags3);

        if (pProperty)
            *pProperty = pStyleExt->flags3.uExtendDimLineUnderText;
        }
    else
        {
        if (pProperty)
            *pProperty = defaultValueIn;
        }

    return  bHasExtension;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/08
+---------------+---------------+---------------+---------------+---------------+------*/
bool     BentleyApi::mdlDim_extensionsGetDirectionMode
(
DirectionMode  *pProperty,
DimOverrides   *pOverridesIn
)
    {
    DimStyleExtensions      *pStyleExt  = NULL;
    bool                    bHasExtension = false;

    if (pOverridesIn->nStyleExt)
        {
        pStyleExt       = &pOverridesIn->pStyleExt->styleExt;
        bHasExtension   = isOverride (pStyleExt->modifiers, STYLE_Extension_DirectionMode);
        }

    if (pProperty)
        *pProperty = bHasExtension ? (DirectionMode) pStyleExt->directionMode : DirectionMode::Azimuth;

    return  bHasExtension;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/08
+---------------+---------------+---------------+---------------+---------------+------*/
bool     BentleyApi::mdlDim_extensionsGetDirectionBaseDir
(
double         *pProperty,
DimOverrides   *pOverridesIn
)
    {
    DimStyleExtensions      *pStyleExt  = NULL;
    bool                    bHasExtension = false;

    if (pOverridesIn->nStyleExt)
        {
        pStyleExt       = &pOverridesIn->pStyleExt->styleExt;
        bHasExtension   = isOverride (pStyleExt->modifiers, STYLE_Extension_DirectionBaseDir);
        }

    if (pProperty)
        *pProperty = bHasExtension ? pStyleExt->directionBaseDir : 0.0;

    return  bHasExtension;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/08
+---------------+---------------+---------------+---------------+---------------+------*/
bool     BentleyApi::mdlDim_extensionsGetDirectionClockwise
(
bool           *pProperty,
DimOverrides   *pOverridesIn
)
    {
    DimStyleExtensions      *pStyleExt  = NULL;
    bool                    bHasExtension = false;

    if (pOverridesIn->nStyleExt)
        {
        pStyleExt       = &pOverridesIn->pStyleExt->styleExt;
        bHasExtension   = isOverride (pStyleExt->modifiers, STYLE_Extension_Flags3);
        }

    if (pProperty)
        *pProperty = bHasExtension ? pStyleExt->flags3.directionClockwise : false;

    return  bHasExtension;
    }

/*---------------------------------------------------------------------------------**//**
* Get ordinate property
* @bsimethod                                                    petri.niiranen  04/01
+---------------+---------------+---------------+---------------+---------------+------*/
Public bool     mdlDim_overridesGetOrdinateStartValueY
(
double          *pProperty,
DimOverrides    *pOverridesIn,
double          defaultValueIn
)
    {
    DimOrdinateOverrides    *pOrdinate  = NULL;
    bool                    bIsOverride = false;

    if (pOverridesIn->nOrdinate)
        {
        pOrdinate   = &pOverridesIn->pOrdinate->ordinate;
        bIsOverride = isOverride (pOrdinate->modifiers, ORDINATE_Override_StartValueX);

        if (pProperty)
            *pProperty = pOrdinate->startValueX;
        }
    else
        {
        if (pProperty)
            *pProperty = defaultValueIn;
        }

    return  bIsOverride;
    }

/*---------------------------------------------------------------------------------**//**
* Get text dimMLText blob for given segment.
* @param    ppText        <= found text
* @param    pOverridesIn  =>
* @param    segmentNo     =>
* @return   SUCCESS, if dimMLText exists; otherwise ERROR.
* @bsimethod                                                    petri.niiranen  06/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt        BentleyApi::mdlDim_overridesGetDimMLText
(
DimMLText       **ppText,
DimOverrides    *pOverridesIn,
int             segmentNo
)
    {
    DimMLText   *pText = NULL;

    pText = getDimMLTextBySegmentNo (pOverridesIn, segmentNo);

    if (ppText)
        *ppText = pText;

    return  (NULL != pText) ? SUCCESS : ERROR;
    }

/*=================================================================================**//**
*
* Public API
*
+===============+===============+===============+===============+===============+======*/
/*=================================================================================**//**
*
* Parameter get/set functions
*
+===============+===============+===============+===============+===============+======*/
/*---------------------------------------------------------------------------------------
    Parameter extraction functions are provided for convinience to get a single
    value from overrides. If entire set of overrides is preferred then user should
    call mdlDim_getXxxOverride instead.

    For large extraction, use mdlDim_overridesGet to get override collection with
    single call. Override collection functions are now restricted to core only
    as they are used in stroke context.

---------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------**//**
* Get effective witnessline property.
*
* @param        pValue          <= value or NULL if not required
* @param        dimElement            =>
* @param        pointNo         =>
* @return       true if override exists
* @bsimethod                                                    petri.niiranen  02/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool          BentleyApi::mdlDim_pointGetWitnessExtend
(
double      *pValue,
ElementHandleCR dimElement,
int         pointNo
)
    {
    DimPointOverrides       point;
    bool                    bIsOverride    = false;
    
    DimensionElm const* dim = &dimElement.GetElementCP()->ToDimensionElm();
    if (SUCCESS == mdlDim_getPointOverride (&point, dimElement, pointNo))
        bIsOverride = point_getWitnessExtend (&point, pValue);

    if (pValue)
        *pValue = bIsOverride ? point.witnessExtend : dim->geom.witExtend;

    return  bIsOverride;
    }

/*---------------------------------------------------------------------------------**//**
* Set effective witnessline property.
*
* @param        dimElement            =>
* @param        pointNo         =>
* @param        pValueIn        => value or NULL to restore default
* @return       SUCCESS if can be set
* @bsimethod                                                    petri.niiranen  02/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt     BentleyApi::mdlDim_pointSetWitnessExtend
(
EditElementHandleR dimElement,
int         pointNo,
double      *pValueIn
)
    {
    DimPointOverrides       point;

    memset (&point, 0, sizeof (point));
    mdlDim_getPointOverride (&point, dimElement, pointNo);

    if (NULL == pValueIn)
        point.modifiers &= ~POINT_Override_WitnessExtend;
    else
        {
        point.modifiers |= POINT_Override_WitnessExtend;
        point.witnessExtend = *pValueIn;
        }

    return  mdlDim_setPointOverride (dimElement, &point, pointNo);
    }

/*---------------------------------------------------------------------------------**//**
* Get effective witnessline property.
*
* @param        pValue          <= value or NULL if not required
* @param        dimElement            =>
* @param        pointNo         =>
* @return       true if override exists
* @bsimethod                                                    petri.niiranen  02/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool          BentleyApi::mdlDim_pointGetWitnessOffset
(
double      *pValue,
ElementHandleCR dimElement,
int         pointNo
)
    {
    bool                    bIsOverride    = false;
    DimPointOverrides       point;
    DimensionElm const* dim = &dimElement.GetElementCP()->ToDimensionElm();
    if (SUCCESS == mdlDim_getPointOverride (&point, dimElement, pointNo))
        bIsOverride = point_getWitnessOffset (&point, pValue);
    
    if (pValue)
        *pValue = bIsOverride ? point.witnessOffset : dim->geom.witOffset;

    return  bIsOverride;
    }

/*---------------------------------------------------------------------------------**//**
* Set effective witnessline property.
*
* @param        dimElement            =>
* @param        pointNo         =>
* @param        pValueIn        => value or NULL to restore default
* @return       SUCCESS if can be set
* @bsimethod                                                    petri.niiranen  02/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt     BentleyApi::mdlDim_pointSetWitnessOffset
(
EditElementHandleR dimElement,
int         pointNo,
double      *pValueIn
)
    {
    DimPointOverrides       point;

    memset (&point, 0, sizeof (point));
    mdlDim_getPointOverride (&point, dimElement, pointNo);

    if (NULL == pValueIn)
        point.modifiers &= ~POINT_Override_WitnessOffset;
    else
        {
        point.modifiers |= POINT_Override_WitnessOffset;
        point.witnessOffset = *pValueIn;
        }

    return  mdlDim_setPointOverride (dimElement, &point, pointNo);
    }

/*---------------------------------------------------------------------------------**//**
* Get effective witnessline property.
*
* @param        pValue          <= value or NULL if not required
* @param        dimElement            =>
* @param        pointNo         =>
* @return       true if override exists
* @bsimethod                                                    petri.niiranen  02/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool          BentleyApi::mdlDim_pointGetWitnessColor
(
UInt32      *pValue,
ElementHandleCR dimElement,
int          pointNo
)
    {
    bool                bIsOverride    = false;
    DimPointOverrides   point;
    DimensionElm const* dim = &dimElement.GetElementCP()->ToDimensionElm();
    UInt32              defaultValue = dim->GetSymbology().color;

    if (SUCCESS == mdlDim_getPointOverride (&point, dimElement, pointNo))
        bIsOverride = point_getWitnessColor (&point, pValue);

    if (pointNo < dim->nPoints && dim->GetDimTextCP(pointNo)->flags.b.altSymb)
        defaultValue = dim->altSymb.color;

    if (pValue)
        *pValue = bIsOverride ? point.color : defaultValue;

    return  bIsOverride;
    }

/*---------------------------------------------------------------------------------**//**
* Set effective witnessline property.
*
* @param        dimElement            =>
* @param        pointNo         =>
* @param        pValueIn        => value or NULL to restore default
* @return       SUCCESS if can be set
* @bsimethod                                                    petri.niiranen  02/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt     BentleyApi::mdlDim_pointSetWitnessColor
(
EditElementHandleR dimElement,
int         pointNo,
UInt32     *pValueIn
)
    {
    DimPointOverrides       point;

    memset (&point, 0, sizeof (point));
    mdlDim_getPointOverride (&point, dimElement, pointNo);

    if (NULL == pValueIn)
        point.modifiers &= ~POINT_Override_WitnessColor;
    else
        {
        point.modifiers |= POINT_Override_WitnessColor;
        point.color = *pValueIn;
        }

    return  mdlDim_setPointOverride (dimElement, &point, pointNo);
    }

/*---------------------------------------------------------------------------------**//**
* Get effective witnessline property.
*
* @param        pValue          <= value or NULL if not required
* @param        dimElement            =>
* @param        pointNo         =>
* @return       true if override exists
* @bsimethod                                                    petri.niiranen  02/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool          BentleyApi::mdlDim_pointGetWitnessWeight
(
UInt32       *pValue,
ElementHandleCR dimElement,
int          pointNo
)
    {
    bool                    bIsOverride    = false;
    DimPointOverrides       point;
    DimensionElm const* dim = &dimElement.GetElementCP()->ToDimensionElm();
    UInt32                  defaultValue = dim->GetSymbology().weight;

    if (SUCCESS == mdlDim_getPointOverride (&point, dimElement, pointNo))
        bIsOverride = point_getWitnessWeight (&point, pValue);

    if (pointNo < dim->nPoints && dim->GetDimTextCP(pointNo)->flags.b.altSymb)
        defaultValue = dim->altSymb.weight;

    if (pValue)
        *pValue = bIsOverride ? point.weight : defaultValue;

    return  bIsOverride;
    }

/*---------------------------------------------------------------------------------**//**
* Set effective witnessline property.
*
* @param        dimElement            =>
* @param        pointNo         =>
* @param        pValueIn        => value or NULL to restore default
* @return       SUCCESS if can be set
* @bsimethod                                                    petri.niiranen  02/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt     BentleyApi::mdlDim_pointSetWitnessWeight
(
EditElementHandleR dimElement,
int         pointNo,
UInt32     *pValueIn
)
    {
    DimPointOverrides       point;

    memset (&point, 0, sizeof (point));
    mdlDim_getPointOverride (&point, dimElement, pointNo);

    if (NULL == pValueIn)
        point.modifiers &= ~POINT_Override_WitnessWeight;
    else
        {
        point.modifiers |= POINT_Override_WitnessWeight;
        point.weight = *pValueIn;
        }

    return  mdlDim_setPointOverride (dimElement, &point, pointNo);
    }

/*---------------------------------------------------------------------------------**//**
* Get effective witnessline property.
*
* @param        pValue          <= value or NULL if not required
* @param        dimElement            =>
* @param        pointNo         =>
* @return       true if override exists
* @bsimethod                                                    petri.niiranen  02/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool          BentleyApi::mdlDim_pointGetWitnessStyle
(
Int32      *pValue,
ElementHandleCR dimElement,
int         pointNo
)
    {
    bool                bIsOverride    = false;
    DimPointOverrides   point;
    DimensionElm const* dim = &dimElement.GetElementCP()->ToDimensionElm();
    long                defaultValue = dim->GetSymbology().style;

    if (SUCCESS == mdlDim_getPointOverride (&point, dimElement, pointNo))
        bIsOverride = point_getWitnessStyle (&point, (UInt32*) pValue);

    if (pointNo < dim->nPoints && dim->GetDimTextCP(pointNo)->flags.b.altSymb)
        defaultValue = dim->altSymb.style;

    if (pValue)
        *pValue = bIsOverride ? point.style : defaultValue;

    return  bIsOverride;
    }

/*---------------------------------------------------------------------------------**//**
* Set effective witnessline property.
*
* @param        dimElement            =>
* @param        pointNo         =>
* @param        pValueIn        => value or NULL to restore default
* @return       SUCCESS if can be set
* @bsimethod                                                    petri.niiranen  02/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt     BentleyApi::mdlDim_pointSetWitnessStyle
(
EditElementHandleR dimElement,
int         pointNo,
Int32      *pValueIn
)
    {
    DimPointOverrides       point;

    memset (&point, 0, sizeof (point));
    mdlDim_getPointOverride (&point, dimElement, pointNo);

    if (NULL == pValueIn)
        point.modifiers &= ~POINT_Override_WitnessStyle;
    else
        {
        point.modifiers |= POINT_Override_WitnessStyle;
        point.style = *pValueIn;
        }

    return  mdlDim_setPointOverride (dimElement, &point, pointNo);
    }

/*---------------------------------------------------------------------------------**//**
* Get effective segment property.
*
* @param        pValue          <= value or NULL if not required
* @param        dimElement            =>
* @param        segmentNo       =>
* @return       true if override exists
* @bsimethod                                                    petri.niiranen  02/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool          BentleyApi::mdlDim_segmentGetTextRotation
(
double      *pValue,
ElementHandleCR dimElement,
int         segmentNo
)
    {
    bool                    bIsOverride    = false;
    DimSegmentOverrides     segment;

    if (pValue)
        *pValue = 0.0;

    if (SUCCESS == mdlDim_getSegmentOverride (&segment, dimElement, segmentNo))
        bIsOverride = segment_getTextRotation (&segment, pValue);

    return  bIsOverride;
    }

/*---------------------------------------------------------------------------------**//**
* Set effective segment property.
*
* @param        dimElement            =>
* @param        segmentNo       =>
* @param        pValueIn        => value or NULL to restore default
* @return       SUCCESS if can be set
* @bsimethod                                                    petri.niiranen  02/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt     BentleyApi::mdlDim_segmentSetTextRotation
(
EditElementHandleR dimElement,
int         segmentNo,
double      *pValueIn
)
    {
    DimSegmentOverrides     segment;

    memset (&segment, 0, sizeof (segment));
    mdlDim_getSegmentOverride (&segment, dimElement, segmentNo);

    if (NULL == pValueIn)
        segment.modifiers &= ~SEGMENT_Override_TextRotation;
    else
        {
        segment.modifiers |= SEGMENT_Override_TextRotation;
        segment.textRotation = *pValueIn;
        }

    return  mdlDim_setSegmentOverride (dimElement, &segment, segmentNo);
    }

/*---------------------------------------------------------------------------------**//**
* Get effective segment property.
*
* @param        pValue          <= value or NULL if not required
* @param        dimElement            =>
* @param        segmentNo       =>
* @return       true if override exists
* @bsimethod                                                    SunandSandurkar 11/03
+---------------+---------------+---------------+---------------+---------------+------*/
bool          BentleyApi::mdlDim_segmentGetCurveStartTangent
(
DPoint3d    *pValue,
ElementHandleCR dimElement,
int         segmentNo
)
    {
    bool                    bIsOverride    = false;
    DimSegmentOverrides     segment;

    if (pValue)
        memset (pValue, 0, sizeof (*pValue));

    if (SUCCESS == mdlDim_getSegmentOverride (&segment, dimElement, segmentNo))
        bIsOverride = segment_getCurveStartTangent (&segment, pValue);

    return  bIsOverride;
    }

/*---------------------------------------------------------------------------------**//**
* Set effective segment property.
*
* @param        dimElement            =>
* @param        segmentNo       =>
* @param        pValueIn        => value or NULL to restore default
* @return       SUCCESS if can be set
* @bsimethod                                                    SunandSandurkar 11/03
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt     BentleyApi::mdlDim_segmentSetCurveStartTangent
(
EditElementHandleR dimElement,
int         segmentNo,
DPoint3d    *pValueIn
)
    {
    DimSegmentOverrides     segment;

    memset (&segment, 0, sizeof (segment));
    mdlDim_getSegmentOverride (&segment, dimElement, segmentNo);

    if (NULL == pValueIn)
        segment.modifiers &= ~SEGMENT_Override_CurveStartTangent;
    else
        {
        segment.modifiers |= SEGMENT_Override_CurveStartTangent;
        segment.curveStartTangent = *pValueIn;
        }

    return  mdlDim_setSegmentOverride (dimElement, &segment, segmentNo);
    }

/*---------------------------------------------------------------------------------**//**
* Get effective segment property.
*
* @param        pValue          <= value or NULL if not required
* @param        dimElement            =>
* @param        segmentNo       =>
* @return       true if override exists
* @bsimethod                                                    SunandSandurkar 11/03
+---------------+---------------+---------------+---------------+---------------+------*/
bool          BentleyApi::mdlDim_segmentGetCurveEndTangent
(
DPoint3d    *pValue,
ElementHandleCR dimElement,
int         segmentNo
)
    {
    bool                    bIsOverride    = false;
    DimSegmentOverrides     segment;

    if (pValue)
        memset (pValue, 0, sizeof (*pValue));

    if (SUCCESS == mdlDim_getSegmentOverride (&segment, dimElement, segmentNo))
        bIsOverride = segment_getCurveEndTangent (&segment, pValue);

    return  bIsOverride;
    }

/*---------------------------------------------------------------------------------**//**
* Set effective segment property.
*
* @param        dimElement            =>
* @param        segmentNo       =>
* @param        pValueIn        => value or NULL to restore default
* @return       SUCCESS if can be set
* @bsimethod                                                    SunandSandurkar 11/03
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt     BentleyApi::mdlDim_segmentSetCurveEndTangent
(
EditElementHandleR dimElement,
int         segmentNo,
DPoint3d    *pValueIn
)
    {
    DimSegmentOverrides     segment;

    memset (&segment, 0, sizeof (segment));
    mdlDim_getSegmentOverride (&segment, dimElement, segmentNo);

    if (NULL == pValueIn)
        segment.modifiers &= ~SEGMENT_Override_CurveEndTangent;
    else
        {
        segment.modifiers |= SEGMENT_Override_CurveEndTangent;
        segment.curveEndTangent = *pValueIn;
        }

    return  mdlDim_setSegmentOverride (dimElement, &segment, segmentNo);
    }

/*---------------------------------------------------------------------------------**//**
* Get effective segment property.
*
* @param        pValue          <= value or NULL if not required
* @param        dimElement            =>
* @param        segmentNo       =>
* @return       true if override exists
* @bsimethod                                                    petri.niiranen  02/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool          BentleyApi::mdlDim_segmentGetTextJustification
(
UInt16      *pValue,
ElementHandleCR dimElement,
int         segmentNo
)
    {
    bool                    bIsOverride    = false;
    DimSegmentOverrides     segment;

    if (pValue)
        *pValue = static_cast<UInt16>(TextElementJustification::LeftMiddle);

    if (SUCCESS == mdlDim_getSegmentOverride (&segment, dimElement, segmentNo))
        return  segment_getTextJustification (&segment, pValue);

    return  bIsOverride;
    }

/*---------------------------------------------------------------------------------**//**
* Set effective segment property.
*
* @param        dimElement            =>
* @param        segmentNo       =>
* @param        pValueIn        => value or NULL to restore default
* @return       SUCCESS if can be set
* @bsimethod                                                    petri.niiranen  02/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt     BentleyApi::mdlDim_segmentSetTextJustification
(
EditElementHandleR dimElement,
int         segmentNo,
UInt16      *pValueIn
)
    {
    DimSegmentOverrides     segment;

    memset (&segment, 0, sizeof (segment));
    mdlDim_getSegmentOverride (&segment, dimElement, segmentNo);

    if (NULL == pValueIn)
        segment.modifiers &= ~SEGMENT_Override_TextJustification;
    else
        {
        segment.modifiers |= SEGMENT_Override_TextJustification;
        segment.textJustification = *pValueIn;
        }

    return  mdlDim_setSegmentOverride (dimElement, &segment, segmentNo);
    }

/*---------------------------------------------------------------------------------**//**
* Utility to set single flag with segment flags.
*
* @bsimethod                                                    petri.niiranen  03/01
+---------------+---------------+---------------+---------------+---------------+------*/
static int     segmentFlagSetSingleFlag
(
EditElementHandleR dimElement,
int         segmentNo,
bool        *pValueIn,
UInt32      flagMask
)
    {
    DimSegmentFlagOverrides         sgmtflg;

    memset (&sgmtflg, 0, sizeof (sgmtflg));
    mdlDim_getSegmentFlagOverride (&sgmtflg, dimElement, segmentNo);

    if (NULL == pValueIn)
        sgmtflg.modifiers &= ~flagMask;
    else
        {
        sgmtflg.modifiers |= flagMask;
        }

    return  mdlDim_setSegmentFlagOverride (dimElement, &sgmtflg, segmentNo);
    }

/*---------------------------------------------------------------------------------**//**
* Get effective segment flag property.
*
* @param        pValue          <= value or NULL if not required
* @param        dimElement            =>
* @param        segmentNo       =>
* @return       true if override exists
* @bsimethod                                                    petri.niiranen  02/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool          BentleyApi::mdlDim_segmentFlagGetUnderlineText
(
bool        *pValue,
ElementHandleCR dimElement,
int         segmentNo
)
    {
    bool                    bIsOverride    = false;
    DimSegmentFlagOverrides sgmtflg;

    if (pValue)
        *pValue = false;

    if (SUCCESS == mdlDim_getSegmentFlagOverride (&sgmtflg, dimElement, segmentNo))
        return  segmentflag_getUnderlineText (&sgmtflg, pValue);

    return  bIsOverride;
    }

/*---------------------------------------------------------------------------------**//**
* Set effective segment flag property.
*
* @param        dimElement            =>
* @param        segmentNo       =>
* @param        pValueIn        => value or NULL to restore default
* @return       SUCCESS if can be added
* @bsimethod                                                    petri.niiranen  02/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt     BentleyApi::mdlDim_segmentFlagSetUnderlineText
(
EditElementHandleR dimElement,
int         segmentNo,
bool        *pValueIn
)
    {
    return  segmentFlagSetSingleFlag (dimElement, segmentNo, pValueIn, SEGMENTFLAG_Override_UnderlineText);
    }

/*---------------------------------------------------------------------------------**//**
* Get effective segment flag property.
*
* @param        pValue          <= value or NULL if not required
* @param        dimElement            =>
* @param        segmentNo       =>
* @return       true if override exists
* @bsimethod                                                    petri.niiranen  02/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool          BentleyApi::mdlDim_segmentFlagGetSuppressLeftDimLine
(
bool        *pValue,
ElementHandleCR dimElement,
int         segmentNo
)
    {
    bool                    bIsOverride    = false;
    DimSegmentFlagOverrides sgmtflg;

    if (pValue)
        *pValue = false;

    if (SUCCESS == mdlDim_getSegmentFlagOverride (&sgmtflg, dimElement, segmentNo))
        return  segmentflag_getSuppressLeftDimLine (&sgmtflg, pValue);

    return  bIsOverride;
    }

/*---------------------------------------------------------------------------------**//**
* Set effective segment flag property.
*
* @param        dimElement            =>
* @param        segmentNo       =>
* @param        pValueIn        => value or NULL to restore default
* @return       SUCCESS if can be added
* @bsimethod                                                    petri.niiranen  02/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt     BentleyApi::mdlDim_segmentFlagSetSuppressLeftDimLine
(
EditElementHandleR dimElement,
int         segmentNo,
bool        *pValueIn
)
    {
    return  segmentFlagSetSingleFlag (dimElement, segmentNo, pValueIn, SEGMENTFLAG_Override_SuppressLeftDimLine);
    }

/*---------------------------------------------------------------------------------**//**
* Get effective segment flag property.
*
* @param        pValue          <= value or NULL if not required
* @param        dimElement            =>
* @param        segmentNo       =>
* @return       true if override exists
* @bsimethod                                                    petri.niiranen  02/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool          BentleyApi::mdlDim_segmentFlagGetSuppressRightDimLine
(
bool        *pValue,
ElementHandleCR dimElement,
int         segmentNo
)
    {
    bool                    bIsOverride    = false;
    DimSegmentFlagOverrides sgmtflg;

    if (pValue)
        *pValue = false;

    if (SUCCESS == mdlDim_getSegmentFlagOverride (&sgmtflg, dimElement, segmentNo))
        return  segmentflag_getSuppressRightDimLine (&sgmtflg, pValue);

    return  bIsOverride;
    }

/*---------------------------------------------------------------------------------**//**
* Set effective segment flag property.
*
* @param        dimElement            =>
* @param        segmentNo       =>
* @param        pValueIn        => value or NULL to restore default
* @return       SUCCESS if can be added
* @bsimethod                                                    petri.niiranen  02/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt     BentleyApi::mdlDim_segmentFlagSetSuppressRightDimLine
(
EditElementHandleR dimElement,
int         segmentNo,
bool        *pValueIn
)
    {
    return  segmentFlagSetSingleFlag (dimElement, segmentNo, pValueIn, SEGMENTFLAG_Override_SuppressRightDimLine);
    }

/*---------------------------------------------------------------------------------**//**
* Get effective segment flag property.
*
* @param        pValue          <= value or NULL if not required
* @param        dimElement            =>
* @param        segmentNo       =>
* @return       true if override exists
* @bsimethod                                                    petri.niiranen  02/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool          BentleyApi::mdlDim_segmentFlagGetPrimaryIsReference
(
bool        *pValue,
ElementHandleCR dimElement,
int         segmentNo
)
    {
    bool                    bIsOverride    = false;
    DimSegmentFlagOverrides sgmtflg;

    if (pValue)
        *pValue = false;

    if (SUCCESS == mdlDim_getSegmentFlagOverride (&sgmtflg, dimElement, segmentNo))
        return  segmentflag_isPrimaryReference (&sgmtflg, pValue);

    return  bIsOverride;
    }

/*---------------------------------------------------------------------------------**//**
* Set effective segment flag property.
*
* @param        dimElement            =>
* @param        segmentNo       =>
* @param        pValueIn        => value or NULL to restore default
* @return       SUCCESS if can be added
* @bsimethod                                                    petri.niiranen  02/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt     BentleyApi::mdlDim_segmentFlagSetPrimaryIsReference
(
EditElementHandleR dimElement,
int         segmentNo,
bool        *pValueIn
)
    {
    return  segmentFlagSetSingleFlag (dimElement, segmentNo, pValueIn, SEGMENTFLAG_Override_SuppressRightDimLine);
    }

/*---------------------------------------------------------------------------------**//**
* Get effective segment flag property.
*
* @param        pValue          <= value or NULL if not required
* @param        dimElement            =>
* @param        segmentNo       =>
* @return       true if override exists
* @bsimethod                                                    petri.niiranen  02/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool          BentleyApi::mdlDim_segmentFlagGetSecondaryIsReference
(
bool        *pValue,
ElementHandleCR dimElement,
int         segmentNo
)
    {
    bool                    bIsOverride    = false;
    DimSegmentFlagOverrides sgmtflg;

    if (pValue)
        *pValue = false;

    if (SUCCESS == mdlDim_getSegmentFlagOverride (&sgmtflg, dimElement, segmentNo))
        return  segmentflag_isSecondaryReference (&sgmtflg, pValue);

    return  bIsOverride;
    }

/*---------------------------------------------------------------------------------**//**
* Set effective segment flag property.
*
* @param        dimElement            =>
* @param        segmentNo       =>
* @param        pValueIn        => value or NULL to restore default
* @return       SUCCESS if can be added
* @bsimethod                                                    petri.niiranen  02/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt     BentleyApi::mdlDim_segmentFlagSetSecondaryIsReference
(
EditElementHandleR dimElement,
int         segmentNo,
bool        *pValueIn
)
    {
    return  segmentFlagSetSingleFlag (dimElement, segmentNo, pValueIn, SEGMENTFLAG_Override_SuppressRightDimLine);
    }

/*---------------------------------------------------------------------------------**//**
* Get effective overall property
*
* @param        pValueOut       <=
* @param        dimElement      =>
* @return       true if override exists
* @bsimethod                                                    JoshSchifter    08/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool          BentleyApi::mdlDim_overallGetRefScale
(
double          *pValueOut,
ElementHandleCR    dimElement
)
    {
    bool                    bIsOverride = false;
    DimOverallOverrides     overall;

    if (SUCCESS == mdlDim_getOverallOverride (&overall, dimElement))
        bIsOverride = isOverride (overall.modifiers, OVERALL_Override_RefScale);

    if (pValueOut)
        *pValueOut = bIsOverride ? overall.refScale : 1.0;

    return  bIsOverride;
    }

/*---------------------------------------------------------------------------------**//**
* Set effective overall property.
*
* @param        dimElement            <=>
* @param        pValueIn         =>
* @return       SUCCESS if can be set
* @bsimethod                                                    JoshSchifter    08/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt     DimensionHandler::OverallSetRefScale
(
EditElementHandleR dimElement,
double      *pValueIn
)
    {
    DimOverallOverrides    overall;

    memset (&overall, 0, sizeof (overall));
    mdlDim_getOverallOverride (&overall, dimElement);

    if (NULL == pValueIn)
        overall.modifiers &= ~OVERALL_Override_RefScale;
    else
        {
        overall.modifiers |= OVERALL_Override_RefScale;
        overall.refScale = *pValueIn;
        }

    return  mdlDim_setOverallOverride (dimElement, &overall);
    }

/*---------------------------------------------------------------------------------**//**
* Get dimension angle quadrant.
*
* @param        pValueOut   <=
* @param        dimElement        =>
* @return       true if exists
* @bsimethod                                                    petri.niiranen  08/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool          BentleyApi::mdlDim_overallGetAngleQuadrant
(
UInt16      *pValueOut,
ElementHandleCR dimElement
)
    {
    bool                    bIsOverride    = false;
    DimOverallOverrides     overall;

    if (SUCCESS == mdlDim_getOverallOverride (&overall, dimElement))
        {
        bIsOverride = isOverride (overall.modifiers, OVERALL_Override_AngleQuadrant);
        if (pValueOut)
            *pValueOut = bIsOverride ? overall.angleQuadrant : 0;
        }

    return  bIsOverride;
    }

/*---------------------------------------------------------------------------------**//**
@Description    Set dimension angle quadrant.
*
* @param        dimElement        IN the dimension element to set the value on.
* @param        pValueIn    IN value or NULL to restore default.
* @return       SUCCESS if can be set.
* @Group        "Dimension Functions"
* @bsimethod                                                    petri.niiranen  08/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt     BentleyApi::mdlDim_overallSetAngleQuadrant
(
EditElementHandleR dimElement,
UInt16      *pValueIn
)
    {
    DimensionHandler* hdlr = dynamic_cast<DimensionHandler*> (&dimElement.GetHandler());
    if (NULL == hdlr)
        return ERROR;
    
    return hdlr->OverallSetAngleQuadrant (dimElement, pValueIn);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       DimensionHandler::OverallSetAngleQuadrant (EditElementHandleR dimElement, UInt16 *pValueIn)
    {
    DimOverallOverrides     overall;

    memset (&overall, 0, sizeof (overall));
    mdlDim_getOverallOverride (&overall, dimElement);

    if (NULL == pValueIn)
        overall.modifiers &= ~OVERALL_Override_AngleQuadrant;
    else
        {
        overall.modifiers    |= OVERALL_Override_AngleQuadrant;
        overall.angleQuadrant = *pValueIn;
        }

    return  mdlDim_setOverallOverride (dimElement, &overall);
    }

/*---------------------------------------------------------------------------------**//**
* Get dimension slant angle.  Only used for 3d dimensions.
*
* @param        pValueOut   <=
* @param        dimElement        =>
* @return       true if exists
* @bsimethod                                                    JoshSchifter    10/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool          BentleyApi::mdlDim_overallGetSlantAngle
(
double      *pValueOut,
ElementHandleCR dimElement
)
    {
    bool                    bIsOverride    = false;
    DimOverallOverrides     overall;

    if (SUCCESS == mdlDim_getOverallOverride (&overall, dimElement))
        {
        bIsOverride = isOverride (overall.modifiers, OVERALL_Override_SlantAngle);
        if (pValueOut)
            *pValueOut = bIsOverride ? overall.slantAngle : 0;
        }

    return  bIsOverride;
    }

/*---------------------------------------------------------------------------------**//**
@Description    Set dimension slant angle. Only used for 3d dimensions.
*
* @param        dimElement        IN the dimension element to set the value on.
* @param        pValueIn    IN value or NULL to restore default.
* @return       SUCCESS if can be set.
* @Group        "Dimension Functions"
* @bsimethod                                                    JoshSchifter    10/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt     BentleyApi::mdlDim_overallSetSlantAngle
(
EditElementHandleR dimElement,
double      *pValueIn
)
    {
    DimOverallOverrides     overall;

    memset (&overall, 0, sizeof (overall));
    mdlDim_getOverallOverride (&overall, dimElement);

    if (NULL == pValueIn)
        overall.modifiers &= ~OVERALL_Override_SlantAngle;
    else
        {
        overall.modifiers   |= OVERALL_Override_SlantAngle;
        overall.slantAngle  = *pValueIn;
        }

    return  mdlDim_setOverallOverride (dimElement, &overall);
    }

/*---------------------------------------------------------------------------------**//**
* Get effective overall property
*
* @param        pValueOut       <=
* @param        dimElement      =>
* @return       true if override exists
* @bsimethod                                                    SunandSandurkar 10/03
+---------------+---------------+---------------+---------------+---------------+------*/
bool          BentleyApi::mdlDim_overallGetModelAnnotationScale
(
double          *pValueOut,
ElementHandleCR    dimElement
)
    {
    bool                    bIsOverride = false;
    DimOverallOverrides     overall;

    if (SUCCESS == mdlDim_getOverallOverride (&overall, dimElement))
        bIsOverride = isOverride (overall.modifiers, OVERALL_Override_ModelAnnotationScale);

    if (pValueOut)
        *pValueOut = bIsOverride ? overall.dModelAnnotationScale : 1.0;

    return  bIsOverride;
    }

/*---------------------------------------------------------------------------------**//**
* Set effective overall property.
*
* @param        dimElement            <=>
* @param        pValueIn         =>
* @return       SUCCESS if can be set
* @bsimethod                                                    SunandSandurkar 10/03
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt     BentleyApi::mdlDim_overallSetModelAnnotationScale
(
EditElementHandleR dimElement,
double      *pValueIn
)
    {
    DimOverallOverrides    overall;

    memset (&overall, 0, sizeof (overall));
    mdlDim_getOverallOverride (&overall, dimElement);

    if (NULL == pValueIn)
        overall.modifiers &= ~OVERALL_Override_ModelAnnotationScale;
    else
        {
        overall.modifiers |= OVERALL_Override_ModelAnnotationScale;
        overall.dModelAnnotationScale = *pValueIn;
        }

    return  mdlDim_setOverallOverride (dimElement, &overall);
    }

/*---------------------------------------------------------------------------------**//**
* Get primary tolerance accuracy override.
*
* @param        pValueOut   <=
* @param        dimElement        =>
* @return       true if exists
* @bsimethod                                                    JoshSchifter    09/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool          BentleyApi::mdlDim_getPrimaryToleranceAccuracy
(
UInt16      *pValueOut,
ElementHandleCR dimElement
)
    {
    bool                    bHasExtension    = false;
    DimStyleExtensions      styleExt;

    if (SUCCESS == mdlDim_getStyleExtension (&styleExt, dimElement))
        {
        bHasExtension = isOverride (styleExt.modifiers, STYLE_Extension_PrimaryToleranceAccuracy);
        if (pValueOut)
            *pValueOut = bHasExtension ? styleExt.primaryTolAcc : 0;
        }

    return  bHasExtension;
    }

/*---------------------------------------------------------------------------------**//**
@Description    Set primary tolerance accuracy override.
*
* @param        dimElement        IN the dimension element to set the value on.
* @param        pValueIn    IN value or NULL to restore default.
* @return       SUCCESS if can be set.
* @Group        "Dimension Functions"
* @bsimethod                                                    JoshSchifter    09/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt     BentleyApi::mdlDim_setPrimaryToleranceAccuracy
(
EditElementHandleR dimElement,
UInt16      *pValueIn
)
    {
    DimStyleExtensions     styleExt;

    memset (&styleExt, 0, sizeof (styleExt));
    mdlDim_getStyleExtension (&styleExt, dimElement);

    if (NULL == pValueIn)
        styleExt.modifiers &= ~STYLE_Extension_PrimaryToleranceAccuracy;
    else
        {
        styleExt.modifiers    |= STYLE_Extension_PrimaryToleranceAccuracy;
        styleExt.primaryTolAcc = *pValueIn;
        }

    return  mdlDim_setStyleExtension (dimElement, &styleExt);
    }

/*---------------------------------------------------------------------------------**//**
* Get secondary tolerance accuracy override.
*
* @param        pValueOut   <=
* @param        dimElement        =>
* @return       true if exists
* @bsimethod                                                    JoshSchifter    09/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool          BentleyApi::mdlDim_getSecondaryToleranceAccuracy
(
UInt16      *pValueOut,
ElementHandleCR dimElement
)
    {
    bool                    bHasExtensions    = false;
    DimStyleExtensions      extensions;

    if (SUCCESS == mdlDim_getStyleExtension (&extensions, dimElement))
        {
        bHasExtensions = isOverride (extensions.modifiers, STYLE_Extension_SecondaryToleranceAccuracy);
        if (pValueOut)
            *pValueOut = bHasExtensions ? extensions.secondaryTolAcc : 0;
        }

    return  bHasExtensions;
    }

/*---------------------------------------------------------------------------------**//**
@Description    Set secondary tolerance accuracy override.
*
* @param        dimElement        IN the dimension element to set the value on.
* @param        pValueIn    IN value or NULL to restore default.
* @return       SUCCESS if can be set.
* @Group        "Dimension Functions"
* @bsimethod                                                    JoshSchifter    09/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt     BentleyApi::mdlDim_setSecondaryToleranceAccuracy
(
EditElementHandleR dimElement,
UInt16      *pValueIn
)
    {
    DimStyleExtensions      extensions;

    memset (&extensions, 0, sizeof (extensions));
    mdlDim_getStyleExtension (&extensions, dimElement);

    if (NULL == pValueIn)
        extensions.modifiers &= ~STYLE_Extension_SecondaryToleranceAccuracy;
    else
        {
        extensions.modifiers    |= STYLE_Extension_SecondaryToleranceAccuracy;
        extensions.secondaryTolAcc = *pValueIn;
        }

    return  mdlDim_setStyleExtension (dimElement, &extensions);
    }

/*---------------------------------------------------------------------------------**//**
* Get multiline text note vertical justification override.
*
* @param        pValueOut   <=
* @param        dimElement        =>
* @return       true if exists
* @bsimethod                                                    SunandSandurkar 04/03
+---------------+---------------+---------------+---------------+---------------+------*/
bool          BentleyApi::mdlDim_getMultiJustVertical
(
UInt16      *pValueOut,
ElementHandleCR dimElement
)
    {
    bool                    bHasExtensions    = false;
    DimStyleExtensions      extensions;

    if (pValueOut)
        *pValueOut = DIMSTYLE_VALUE_MLNote_VerticalJustification_Top;

    if (SUCCESS == mdlDim_getStyleExtension (&extensions, dimElement))
        {
        bHasExtensions = isOverride (extensions.modifiers, STYLE_Extension_Flags);
        if (pValueOut)
            *pValueOut = bHasExtensions ? extensions.flags.uMultiJustVertical : 0;
        }

    return  bHasExtensions;
    }

/*---------------------------------------------------------------------------------**//**
@Description    Set multiline text note vertical justification override.
*
* @param        dimElement        IN the dimension element to set the value on.
* @param        pValueIn    IN value or NULL to restore default.
* @return       SUCCESS if can be set.
* @Group        "Dimension Functions"
* @bsimethod                                                    SunandSandurkar 04/03
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt     BentleyApi::mdlDim_setMultiJustVertical
(
EditElementHandleR dimElement,
UInt16      *pValueIn
)
    {
    DimStyleExtensions      extensions;

    memset (&extensions, 0, sizeof (extensions));
    mdlDim_getStyleExtension (&extensions, dimElement);

    // No effort is made to keep the flags modifier bit accurate.
    // It is derived from the flags.
    if (pValueIn)
        extensions.flags.uMultiJustVertical = *pValueIn;

    return  mdlDim_setStyleExtension (dimElement, &extensions);
    }

/*---------------------------------------------------------------------------------**//**
* Get the retain primary dim fraction denominator override.
*
* @param        pValueOut   <=
* @param        dimElement        =>
* @return       true if exists
* @bsimethod                                                    SunandSandurkar 04/03
+---------------+---------------+---------------+---------------+---------------+------*/
bool          BentleyApi::mdlDim_getNoReduceFraction
(
UInt16      *pValueOut,
ElementHandleCR dimElement
)
    {
    bool                    bHasExtensions    = false;
    DimStyleExtensions      extensions;

    if (SUCCESS == mdlDim_getStyleExtension (&extensions, dimElement))
        {
        bHasExtensions = isOverride (extensions.modifiers, STYLE_Extension_Flags);
        if (pValueOut)
            *pValueOut = bHasExtensions ? extensions.flags.uNoReduceFraction : 0;
        }

    return  bHasExtensions;
    }

/*---------------------------------------------------------------------------------**//**
@Description    Set the retain primary dim fraction denominator override.
*
* @param        dimElement        IN the dimension element to set the value on.
* @param        pValueIn    IN value or NULL to restore default.
* @return       SUCCESS if can be set.
* @Group        "Dimension Functions"
* @bsimethod                                                    SunandSandurkar 04/03
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt     BentleyApi::mdlDim_setNoReduceFraction
(
EditElementHandleR dimElement,
UInt16      *pValueIn
)
    {
    DimStyleExtensions      extensions;

    memset (&extensions, 0, sizeof (extensions));
    mdlDim_getStyleExtension (&extensions, dimElement);

    // No effort is made to keep the flags modifier bit accurate.
    // It is derived from the flags.
    if (pValueIn)
        extensions.flags.uNoReduceFraction = *pValueIn;

    return  mdlDim_setStyleExtension (dimElement, &extensions);
    }

/*---------------------------------------------------------------------------------**//**
* Get the retain primary alternate dim fraction denominator override.
*
* @param        pValueOut   <=
* @param        dimElement        =>
* @return       true if exists
* @bsimethod                                                    SunandSandurkar 04/03
+---------------+---------------+---------------+---------------+---------------+------*/
bool          BentleyApi::mdlDim_getNoReduceAltFraction
(
UInt16      *pValueOut,
ElementHandleCR dimElement
)
    {
    bool                    bHasExtensions    = false;
    DimStyleExtensions      extensions;

    if (SUCCESS == mdlDim_getStyleExtension (&extensions, dimElement))
        {
        bHasExtensions = isOverride (extensions.modifiers, STYLE_Extension_Flags);
        if (pValueOut)
            *pValueOut = bHasExtensions ? extensions.flags.uNoReduceAltFraction : 0;
        }

    return  bHasExtensions;
    }

/*---------------------------------------------------------------------------------**//**
@Description    Set the retain primary alternate dim fraction denominator override.
*
* @param        dimElement        IN the dimension element to set the value on.
* @param        pValueIn    IN value or NULL to restore default.
* @return       SUCCESS if can be set.
* @Group        "Dimension Functions"
* @bsimethod                                                    SunandSandurkar 04/03
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt     BentleyApi::mdlDim_setNoReduceAltFraction
(
EditElementHandleR dimElement,
UInt16      *pValueIn
)
    {
    DimStyleExtensions      extensions;

    memset (&extensions, 0, sizeof (extensions));
    mdlDim_getStyleExtension (&extensions, dimElement);

    // No effort is made to keep the flags modifier bit accurate.
    // It is derived from the flags.
    if (pValueIn)
        extensions.flags.uNoReduceAltFraction = *pValueIn;

    return  mdlDim_setStyleExtension (dimElement, &extensions);
    }

/*---------------------------------------------------------------------------------**//**
* Get the retain primary tolerance dim fraction denominator override.
*
* @param        pValueOut   <=
* @param        dimElement        =>
* @return       true if exists
* @bsimethod                                                    SunandSandurkar 04/03
+---------------+---------------+---------------+---------------+---------------+------*/
bool          BentleyApi::mdlDim_getNoReduceTolFraction
(
UInt16      *pValueOut,
ElementHandleCR dimElement
)
    {
    bool                    bHasExtensions    = false;
    DimStyleExtensions      extensions;

    if (SUCCESS == mdlDim_getStyleExtension (&extensions, dimElement))
        {
        bHasExtensions = isOverride (extensions.modifiers, STYLE_Extension_Flags);
        if (pValueOut)
            *pValueOut = bHasExtensions ? extensions.flags.uNoReduceTolFraction : 0;
        }

    return  bHasExtensions;
    }

/*---------------------------------------------------------------------------------**//**
@Description    Set the retain primary tolerance dim fraction denominator override.
*
* @param        dimElement        IN the dimension element to set the value on.
* @param        pValueIn    IN value or NULL to restore default.
* @return       SUCCESS if can be set.
* @Group        "Dimension Functions"
* @bsimethod                                                    SunandSandurkar 04/03
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt     BentleyApi::mdlDim_setNoReduceTolFraction
(
EditElementHandleR dimElement,
UInt16      *pValueIn
)
    {
    DimStyleExtensions      extensions;

    memset (&extensions, 0, sizeof (extensions));
    mdlDim_getStyleExtension (&extensions, dimElement);

    // No effort is made to keep the flags modifier bit accurate.
    // It is derived from the flags.
    if (pValueIn)
        extensions.flags.uNoReduceTolFraction = *pValueIn;

    return  mdlDim_setStyleExtension (dimElement, &extensions);
    }

/*---------------------------------------------------------------------------------**//**
* Get the retain secondary dim fraction denominator override.
*
* @param        pValueOut   <=
* @param        dimElement        =>
* @return       true if exists
* @bsimethod                                                    SunandSandurkar 04/03
+---------------+---------------+---------------+---------------+---------------+------*/
bool          BentleyApi::mdlDim_getNoReduceSecFraction
(
UInt16      *pValueOut,
ElementHandleCR dimElement
)
    {
    bool                    bHasExtensions    = false;
    DimStyleExtensions      extensions;

    if (SUCCESS == mdlDim_getStyleExtension (&extensions, dimElement))
        {
        bHasExtensions = isOverride (extensions.modifiers, STYLE_Extension_Flags);
        if (pValueOut)
            *pValueOut = bHasExtensions ? extensions.flags.uNoReduceSecFraction : 0;
        }

    return  bHasExtensions;
    }

/*---------------------------------------------------------------------------------**//**
@Description    Set the retain secondary dim fraction denominator override.
*
* @param        dimElement        IN the dimension element to set the value on.
* @param        pValueIn    IN value or NULL to restore default.
* @return       SUCCESS if can be set.
* @Group        "Dimension Functions"
* @bsimethod                                                    SunandSandurkar 04/03
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt     BentleyApi::mdlDim_setNoReduceSecFraction
(
EditElementHandleR dimElement,
UInt16      *pValueIn
)
    {
    DimStyleExtensions      extensions;

    memset (&extensions, 0, sizeof (extensions));
    mdlDim_getStyleExtension (&extensions, dimElement);

    // No effort is made to keep the flags modifier bit accurate.
    // It is derived from the flags.
    if (pValueIn)
        extensions.flags.uNoReduceSecFraction = *pValueIn;

    return  mdlDim_setStyleExtension (dimElement, &extensions);
    }

/*---------------------------------------------------------------------------------**//**
* Get the retain secondary alternate dim fraction denominator override.
*
* @param        pValueOut   <=
* @param        dimElement        =>
* @return       true if exists
* @bsimethod                                                    SunandSandurkar 04/03
+---------------+---------------+---------------+---------------+---------------+------*/
bool          BentleyApi::mdlDim_getNoReduceAltSecFraction
(
UInt16      *pValueOut,
ElementHandleCR dimElement
)
    {
    bool                    bHasExtensions    = false;
    DimStyleExtensions      extensions;

    if (SUCCESS == mdlDim_getStyleExtension (&extensions, dimElement))
        {
        bHasExtensions = isOverride (extensions.modifiers, STYLE_Extension_Flags);
        if (pValueOut)
            *pValueOut = bHasExtensions ? extensions.flags.uNoReduceAltSecFraction : 0;
        }

    return  bHasExtensions;
    }

/*---------------------------------------------------------------------------------**//**
@Description    Set the retain secondary alternate dim fraction denominator override.
*
* @param        dimElement        IN the dimension element to set the value on.
* @param        pValueIn    IN value or NULL to restore default.
* @return       SUCCESS if can be set.
* @Group        "Dimension Functions"
* @bsimethod                                                    SunandSandurkar 04/03
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt     BentleyApi::mdlDim_setNoReduceAltSecFraction
(
EditElementHandleR dimElement,
UInt16      *pValueIn
)
    {
    DimStyleExtensions      extensions;

    memset (&extensions, 0, sizeof (extensions));
    mdlDim_getStyleExtension (&extensions, dimElement);

    // No effort is made to keep the flags modifier bit accurate.
    // It is derived from the flags.
    if (pValueIn)
        extensions.flags.uNoReduceAltSecFraction = *pValueIn;

    return  mdlDim_setStyleExtension (dimElement, &extensions);
    }

/*---------------------------------------------------------------------------------**//**
* Get the retain secondary tolerance dim fraction denominator override.
*
* @param        pValueOut   <=
* @param        dimElement        =>
* @return       true if exists
* @bsimethod                                                    SunandSandurkar 04/03
+---------------+---------------+---------------+---------------+---------------+------*/
bool          BentleyApi::mdlDim_getNoReduceTolSecFraction
(
UInt16      *pValueOut,
ElementHandleCR dimElement
)
    {
    bool                    bHasExtensions    = false;
    DimStyleExtensions      extensions;

    if (SUCCESS == mdlDim_getStyleExtension (&extensions, dimElement))
        {
        bHasExtensions = isOverride (extensions.modifiers, STYLE_Extension_Flags);
        if (pValueOut)
            *pValueOut = bHasExtensions ? extensions.flags.uNoReduceTolSecFraction : 0;
        }

    return  bHasExtensions;
    }

/*---------------------------------------------------------------------------------**//**
@Description    Set the retain secondary tolerance dim fraction denominator override.
*
* @param        dimElement        IN the dimension element to set the value on.
* @param        pValueIn    IN value or NULL to restore default.
* @return       SUCCESS if can be set.
* @Group        "Dimension Functions"
* @bsimethod                                                    SunandSandurkar 04/03
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt     BentleyApi::mdlDim_setNoReduceTolSecFraction
(
EditElementHandleR dimElement,
UInt16      *pValueIn
)
    {
    DimStyleExtensions      extensions;

    memset (&extensions, 0, sizeof (extensions));
    mdlDim_getStyleExtension (&extensions, dimElement);

    // No effort is made to keep the flags modifier bit accurate.
    // It is derived from the flags.
    if (pValueIn)
        extensions.flags.uNoReduceTolSecFraction = *pValueIn;

    return  mdlDim_setStyleExtension (dimElement, &extensions);
    }

/*---------------------------------------------------------------------------------**//**
* Get the note leader type override.
*
* @param        pValueOut   <=
* @param        dimElement        =>
* @return       true if exists
* @bsimethod                                                    SunandSandurkar 07/03
+---------------+---------------+---------------+---------------+---------------+------*/
bool          BentleyApi::mdlDim_getNoteLeaderType
(
UInt16      *pValueOut,
ElementHandleCR dimElement
)
    {
    bool                    bHasExtensions    = false;
    DimStyleExtensions      extensions;

    if (pValueOut)
        *pValueOut = 0;

    if (SUCCESS == mdlDim_getStyleExtension (&extensions, dimElement))
        {
        bHasExtensions = isOverride (extensions.modifiers, STYLE_Extension_Flags2);
        if (pValueOut)
            *pValueOut = bHasExtensions ? extensions.flags2.uNoteLeaderType : 0;
        }

    return  bHasExtensions;
    }

/*---------------------------------------------------------------------------------**//**
@Description    Set the note leader type override.
*
* @param        dimElement        IN the dimension element to set the value on.
* @param        pValueIn    IN value or NULL to restore default.
* @return       SUCCESS if can be set.
* @Group        "Dimension Functions"
* @bsimethod                                                    SunandSandurkar 07/03
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt     BentleyApi::mdlDim_setNoteLeaderType
(
EditElementHandleR dimElement,
UInt16      *pValueIn
)
    {
    DimStyleExtensions      extensions;

    memset (&extensions, 0, sizeof (extensions));
    mdlDim_getStyleExtension (&extensions, dimElement);

    // No effort is made to keep the flags modifier bit accurate.
    // It is derived from the flags.
    if (pValueIn)
        extensions.flags2.uNoteLeaderType = *pValueIn;

    return  mdlDim_setStyleExtension (dimElement, &extensions);
    }

/*---------------------------------------------------------------------------------**//**
* Get the note leader type override.
*
* @param        pValueOut   <=
* @param        dimElement        =>
* @return       true if exists
* @bsimethod                                                    SunandSandurkar 07/03
+---------------+---------------+---------------+---------------+---------------+------*/
bool          BentleyApi::mdlDim_getNoteScaleFrame
(
bool        *pValueOut,
ElementHandleCR dimElement
)
    {
    bool                    bHasExtensions    = false;
    DimStyleExtensions      extensions;

    if (*pValueOut)
        *pValueOut = 0;

    if (SUCCESS == mdlDim_getStyleExtension (&extensions, dimElement))
        {
        bHasExtensions = isOverride (extensions.modifiers, STYLE_Extension_Flags2);
        if (pValueOut)
            *pValueOut = bHasExtensions ? extensions.flags2.uNoteScaleFrame : 0;
        }

    return  bHasExtensions;
    }

/*---------------------------------------------------------------------------------**//**
@Description    Set the note Scale Frame override.
*
* @param        dimElement        IN the dimension element to set the value on.
* @param        pValueIn    IN value or NULL to restore default.
* @return       SUCCESS if can be set.
* @Group        "Dimension Functions"
* @bsimethod                                                    SunandSandurkar 07/03
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt     BentleyApi::mdlDim_setNoteScaleFrame
(
EditElementHandleR dimElement,
bool         *pValueIn
)
    {
    DimStyleExtensions      extensions;

    memset (&extensions, 0, sizeof (extensions));
    mdlDim_getStyleExtension (&extensions, dimElement);

    // No effort is made to keep the flags modifier bit accurate.
    // It is derived from the flags.
    if (pValueIn)
        extensions.flags2.uNoteScaleFrame = *pValueIn;

    return  mdlDim_setStyleExtension (dimElement, &extensions);
    }

/*---------------------------------------------------------------------------------**//**
* Get the note terminator override.
*
* @param        pValueOut   <=
* @param        dimElement        =>
* @return       true if exists
* @bsimethod                                                    SunandSandurkar 07/03
+---------------+---------------+---------------+---------------+---------------+------*/
bool          BentleyApi::mdlDim_getNoteTerminator
(
UInt16      *pValueOut,
ElementHandleCR dimElement
)
    {
    bool                    bHasExtensions    = false;
    DimStyleExtensions      extensions;

    if (SUCCESS == mdlDim_getStyleExtension (&extensions, dimElement))
        {
        bHasExtensions = isOverride (extensions.modifiers, STYLE_Extension_Flags2);

        if (pValueOut)
            *pValueOut = static_cast <UInt16> (bHasExtensions ? adim_extensionsGetNoteTerminator (extensions) : 1);
        }

    return  bHasExtensions;
    }

/*---------------------------------------------------------------------------------**//**
@Description    Set the note terminator override.
*
* @param        dimElement        IN the dimension element to set the value on.
* @param        pValueIn    IN value or NULL to restore default.
* @return       SUCCESS if can be set.
* @Group        "Dimension Functions"
* @bsimethod                                                    SunandSandurkar 07/03
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt     BentleyApi::mdlDim_setNoteTerminator
(
EditElementHandleR dimElement,
UInt16      *pValueIn
)
    {
    DimStyleExtensions      extensions;

    memset (&extensions, 0, sizeof (extensions));
    mdlDim_getStyleExtension (&extensions, dimElement);

    // No effort is made to keep the flags modifier bit accurate.
    // It is derived from the flags.
    if (pValueIn)
        adim_extensionsSetNoteTerminator (extensions, *pValueIn);

    return  mdlDim_setStyleExtension (dimElement, &extensions);
    }

/*---------------------------------------------------------------------------------**//**
* Get the note terminator type override.
*
* @param        pValueOut   <=
* @param        dimElement        =>
* @return       true if exists
* @bsimethod                                                    SunandSandurkar 07/03
+---------------+---------------+---------------+---------------+---------------+------*/
bool          BentleyApi::mdlDim_getNoteTerminatorType
(
UInt16      *pValueOut,
ElementHandleCR dimElement
)
    {
    bool                    bHasExtensions    = false;
    DimStyleExtensions      extensions;

    if (SUCCESS == mdlDim_getStyleExtension (&extensions, dimElement))
        {
        bHasExtensions = isOverride (extensions.modifiers, STYLE_Extension_Flags2);
        if (pValueOut)
            *pValueOut = bHasExtensions ? extensions.flags2.uNoteTerminatorType : 0;
        }

    return  bHasExtensions;
    }

/*---------------------------------------------------------------------------------**//**
@Description    Set the note terminator type override.
*
* @param        dimElement        IN the dimension element to set the value on.
* @param        pValueIn    IN value or NULL to restore default.
* @return       SUCCESS if can be set.
* @Group        "Dimension Functions"
* @bsimethod                                                    SunandSandurkar 07/03
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt     mdlDim_setNoteTerminatorType
(
EditElementHandleR dimElement,
UInt16      *pValueIn
)
    {
    DimStyleExtensions      extensions;

    memset (&extensions, 0, sizeof (extensions));
    mdlDim_getStyleExtension (&extensions, dimElement);

    // No effort is made to keep the flags modifier bit accurate.
    // It is derived from the flags.
    if (pValueIn)
        extensions.flags2.uNoteTerminatorType = *pValueIn;

    return  mdlDim_setStyleExtension (dimElement, &extensions);
    }

/*---------------------------------------------------------------------------------**//**
* Get the note text rotation override.
*
* @param        pValueOut   <=
* @param        dimElement        =>
* @return       true if exists
* @bsimethod                                                    SunandSandurkar 07/03
+---------------+---------------+---------------+---------------+---------------+------*/
bool          BentleyApi::mdlDim_getNoteTextRotation
(
UInt16      *pValueOut,
ElementHandleCR dimElement
)
    {
    bool                    bHasExtensions    = false;
    DimStyleExtensions      extensions;

    if (pValueOut)
        *pValueOut = DIMSTYLE_VALUE_MLNote_TextRotation_Horizontal;

    if (SUCCESS == mdlDim_getStyleExtension (&extensions, dimElement))
        {
        bHasExtensions = isOverride (extensions.modifiers, STYLE_Extension_Flags2);
        if (pValueOut)
            *pValueOut = bHasExtensions ? extensions.flags2.uNoteTextRotation : DIMSTYLE_VALUE_MLNote_TextRotation_Horizontal;
        }

    return  bHasExtensions;
    }

/*---------------------------------------------------------------------------------**//**
@Description    Set the note text rotation override.
*
* @param        dimElement        IN the dimension element to set the value on.
* @param        pValueIn    IN value or NULL to restore default.
* @return       SUCCESS if can be set.
* @Group        "Dimension Functions"
* @bsimethod                                                    SunandSandurkar 07/03
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt     BentleyApi::mdlDim_setNoteTextRotation
(
EditElementHandleR dimElement,
UInt16      *pValueIn
)
    {
    DimStyleExtensions      extensions;

    memset (&extensions, 0, sizeof (extensions));
    mdlDim_getStyleExtension (&extensions, dimElement);

    // No effort is made to keep the flags modifier bit accurate.
    // It is derived from the flags.
    if (pValueIn)
        extensions.flags2.uNoteTextRotation = *pValueIn;

    return  mdlDim_setStyleExtension (dimElement, &extensions);
    }

/*---------------------------------------------------------------------------------**//**
* Get the note text horizontal attachment override.
*
* @param        pValueOut   <=
* @param        dimElement        =>
* @return       true if exists
* @bsimethod                                                    SunandSandurkar 07/03
+---------------+---------------+---------------+---------------+---------------+------*/
bool          BentleyApi::mdlDim_getNoteHorAttachment
(
UInt16      *pValueOut,
ElementHandleCR dimElement
)
    {
    bool                    bHasExtensions    = false;
    DimStyleExtensions      extensions;

    if (pValueOut)
        *pValueOut = DIMSTYLE_VALUE_MLNote_HorAttachment_Auto;

    if (SUCCESS == mdlDim_getStyleExtension (&extensions, dimElement))
        {
        bHasExtensions = isOverride (extensions.modifiers, STYLE_Extension_Flags2);
        if (pValueOut)
            *pValueOut = bHasExtensions ? extensions.flags2.uNoteHorAttachment : DIMSTYLE_VALUE_MLNote_HorAttachment_Auto;
        }

    return  bHasExtensions;
    }

/*---------------------------------------------------------------------------------**//**
@Description    Set the note text horizontal attachment override.
*
* @param        dimElement        IN the dimension element to set the value on.
* @param        pValueIn    IN value or NULL to restore default.
* @return       SUCCESS if can be set.
* @Group        "Dimension Functions"
* @bsimethod                                                    SunandSandurkar 07/03
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt     BentleyApi::mdlDim_setNoteHorAttachment
(
EditElementHandleR dimElement,
UInt16      *pValueIn
)
    {
    DimStyleExtensions      extensions;

    memset (&extensions, 0, sizeof (extensions));
    mdlDim_getStyleExtension (&extensions, dimElement);

    // No effort is made to keep the flags modifier bit accurate.
    // It is derived from the flags.
    if (pValueIn)
        extensions.flags2.uNoteHorAttachment = *pValueIn;

    return  mdlDim_setStyleExtension (dimElement, &extensions);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar 07/03
+---------------+---------------+---------------+---------------+---------------+------*/
void    BentleyApi::adim_getMLNoteVerLeftAttachment
(
const DimStyleExtensions            *pExtensions,
UInt16                              *pValue
)
    {
    if (NULL == pValue)
        return;

    /* Refer the notes at the end of midimstyle.h */

    switch (pExtensions->flags2.uNoteVerLeftAttachment)
        {
        case 0:
            {
            switch (pExtensions->flags.uElbowJointLocation)
                {
                case 0:
                    {
                    switch (pExtensions->flags.uMultiJustVertical)
                        {
                        case DIMSTYLE_VALUE_MLNote_VerticalJustification_Top:
                            *pValue = DIMSTYLE_VALUE_MLNote_VerAttachment_TopLine;
                            break;
                        case DIMSTYLE_VALUE_MLNote_VerticalJustification_Center:
                            *pValue = DIMSTYLE_VALUE_MLNote_VerAttachment_Middle;
                            break;
                        case DIMSTYLE_VALUE_MLNote_VerticalJustification_Bottom:
                            *pValue = DIMSTYLE_VALUE_MLNote_VerAttachment_BottomLine;
                            break;
                        case DIMSTYLE_VALUE_MLNote_VerticalJustification_Dynamic:
                        default:
                            *pValue = DIMSTYLE_VALUE_MLNote_VerAttachment_DynamicLine;
                            break;
                        }
                    break;
                    }

                case 1:
                    {
                    switch (pExtensions->flags.uMultiJustVertical)
                        {
                        case DIMSTYLE_VALUE_MLNote_VerticalJustification_Top:
                            *pValue = DIMSTYLE_VALUE_MLNote_VerAttachment_Top;
                            break;
                        case DIMSTYLE_VALUE_MLNote_VerticalJustification_Center:
                            *pValue = DIMSTYLE_VALUE_MLNote_VerAttachment_Middle;
                            break;
                        case DIMSTYLE_VALUE_MLNote_VerticalJustification_Bottom:
                            *pValue = DIMSTYLE_VALUE_MLNote_VerAttachment_Bottom;
                            break;
                        case DIMSTYLE_VALUE_MLNote_VerticalJustification_Dynamic:
                        default:
                            *pValue = DIMSTYLE_VALUE_MLNote_VerAttachment_DynamicCorner;
                            break;
                        }
                    break;
                    }

                case 2:
                default:
                    {
                    *pValue = DIMSTYLE_VALUE_MLNote_VerAttachment_Underline;
                    break;
                    }
                }

            break;
            }

        case 1:
            *pValue = DIMSTYLE_VALUE_MLNote_VerAttachment_Top;
            break;

        case 2:
            *pValue = DIMSTYLE_VALUE_MLNote_VerAttachment_TopLine;
            break;

        case 3:
            *pValue = DIMSTYLE_VALUE_MLNote_VerAttachment_Middle;
            break;

        case 4:
            *pValue = DIMSTYLE_VALUE_MLNote_VerAttachment_BottomLine;
            break;

        case 5:
            *pValue = DIMSTYLE_VALUE_MLNote_VerAttachment_Bottom;
            break;

        case 6:
            *pValue = DIMSTYLE_VALUE_MLNote_VerAttachment_DynamicLine;
            break;

        case 7:
            *pValue = DIMSTYLE_VALUE_MLNote_VerAttachment_DynamicCorner;
            break;

        case 8:
        default:
            *pValue = DIMSTYLE_VALUE_MLNote_VerAttachment_Underline;
            break;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar 07/03
+---------------+---------------+---------------+---------------+---------------+------*/
void    BentleyApi::adim_setMLNoteVerLeftAttachment
(
DimStyleExtensions                      *pExtensions,
UInt16                                  eValue
)
    {

    /* Refer the notes at the end of midimstyle.h */

    switch (eValue)
        {
        case DIMSTYLE_VALUE_MLNote_VerAttachment_Top:
            {
            pExtensions->flags2.uNoteVerLeftAttachment = 1;
            pExtensions->flags.uElbowJointLocation = 1;
            break;
            }

        case DIMSTYLE_VALUE_MLNote_VerAttachment_TopLine:
            {
            pExtensions->flags2.uNoteVerLeftAttachment = 2;
            pExtensions->flags.uElbowJointLocation = 0;
            break;
            }

        case DIMSTYLE_VALUE_MLNote_VerAttachment_Middle:
            {
            pExtensions->flags2.uNoteVerLeftAttachment = 3;
            pExtensions->flags.uElbowJointLocation = 0;
            break;
            }

        case DIMSTYLE_VALUE_MLNote_VerAttachment_BottomLine:
            {
            pExtensions->flags2.uNoteVerLeftAttachment = 4;
            pExtensions->flags.uElbowJointLocation = 0;
            break;
            }

        case DIMSTYLE_VALUE_MLNote_VerAttachment_Bottom:
            {
            pExtensions->flags2.uNoteVerLeftAttachment = 5;
            pExtensions->flags.uElbowJointLocation = 1;
            break;
            }

        case DIMSTYLE_VALUE_MLNote_VerAttachment_DynamicLine:
            {
            pExtensions->flags2.uNoteVerLeftAttachment = 6;
            pExtensions->flags.uElbowJointLocation = 0;
            break;
            }

        case DIMSTYLE_VALUE_MLNote_VerAttachment_DynamicCorner:
            {
            pExtensions->flags2.uNoteVerLeftAttachment = 7;
            pExtensions->flags.uElbowJointLocation = 1;
            break;
            }

        case DIMSTYLE_VALUE_MLNote_VerAttachment_Underline:
        default:
            {
            pExtensions->flags2.uNoteVerLeftAttachment = 8;
            pExtensions->flags.uElbowJointLocation = 2;
            break;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar 07/03
+---------------+---------------+---------------+---------------+---------------+------*/
void            BentleyApi::adim_getMLNoteVerRightAttachment
(
const DimStyleExtensions            *pExtensions,
UInt16                              *pValue
)
    {
    if (NULL == pValue)
        return;

    /* Refer the notes at the end of midimstyle.h */

    switch (pExtensions->flags2.uNoteVerRightAttachment)
        {
        case 0:
            {
            switch (pExtensions->flags.uElbowJointLocation)
                {
                case 0:
                    {
                    switch (pExtensions->flags.uMultiJustVertical)
                        {
                        case DIMSTYLE_VALUE_MLNote_VerticalJustification_Top:
                            *pValue = DIMSTYLE_VALUE_MLNote_VerAttachment_TopLine;
                            break;
                        case DIMSTYLE_VALUE_MLNote_VerticalJustification_Center:
                            *pValue = DIMSTYLE_VALUE_MLNote_VerAttachment_Middle;
                            break;
                        case DIMSTYLE_VALUE_MLNote_VerticalJustification_Bottom:
                            *pValue = DIMSTYLE_VALUE_MLNote_VerAttachment_BottomLine;
                            break;
                        case DIMSTYLE_VALUE_MLNote_VerticalJustification_Dynamic:
                        default:
                            *pValue = DIMSTYLE_VALUE_MLNote_VerAttachment_DynamicLine;
                            break;
                        }
                    break;
                    }

                case 1:
                    {
                    switch (pExtensions->flags.uMultiJustVertical)
                        {
                        case DIMSTYLE_VALUE_MLNote_VerticalJustification_Top:
                            *pValue = DIMSTYLE_VALUE_MLNote_VerAttachment_Top;
                            break;
                        case DIMSTYLE_VALUE_MLNote_VerticalJustification_Center:
                            *pValue = DIMSTYLE_VALUE_MLNote_VerAttachment_Middle;
                            break;
                        case DIMSTYLE_VALUE_MLNote_VerticalJustification_Bottom:
                            *pValue = DIMSTYLE_VALUE_MLNote_VerAttachment_Bottom;
                            break;
                        case DIMSTYLE_VALUE_MLNote_VerticalJustification_Dynamic:
                        default:
                            *pValue = DIMSTYLE_VALUE_MLNote_VerAttachment_DynamicCorner;
                            break;
                        }
                    break;
                    }

                case 2:
                default:
                    {
                    *pValue = DIMSTYLE_VALUE_MLNote_VerAttachment_Underline;
                    break;
                    }
                }

            break;
            }

        case 1:
            *pValue = DIMSTYLE_VALUE_MLNote_VerAttachment_Top;
            break;

        case 2:
            *pValue = DIMSTYLE_VALUE_MLNote_VerAttachment_TopLine;
            break;

        case 3:
            *pValue = DIMSTYLE_VALUE_MLNote_VerAttachment_Middle;
            break;

        case 4:
            *pValue = DIMSTYLE_VALUE_MLNote_VerAttachment_BottomLine;
            break;

        case 5:
            *pValue = DIMSTYLE_VALUE_MLNote_VerAttachment_Bottom;
            break;

        case 6:
            *pValue = DIMSTYLE_VALUE_MLNote_VerAttachment_DynamicLine;
            break;

        case 7:
            *pValue = DIMSTYLE_VALUE_MLNote_VerAttachment_DynamicCorner;
            break;

        case 8:
        default:
            *pValue = DIMSTYLE_VALUE_MLNote_VerAttachment_Underline;
            break;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar 07/03
+---------------+---------------+---------------+---------------+---------------+------*/
void            BentleyApi::adim_setMLNoteVerRightAttachment
(
DimStyleExtensions                      *pExtensions,
UInt16                                  eValue
)
    {

    /* Refer the notes at the end of midimstyle.h */

    switch (eValue)
        {
        case DIMSTYLE_VALUE_MLNote_VerAttachment_Top:
            {
            pExtensions->flags2.uNoteVerRightAttachment = 1;
            break;
            }

        case DIMSTYLE_VALUE_MLNote_VerAttachment_TopLine:
            {
            pExtensions->flags2.uNoteVerRightAttachment = 2;
            break;
            }

        case DIMSTYLE_VALUE_MLNote_VerAttachment_Middle:
            {
            pExtensions->flags2.uNoteVerRightAttachment = 3;
            break;
            }

        case DIMSTYLE_VALUE_MLNote_VerAttachment_BottomLine:
            {
            pExtensions->flags2.uNoteVerRightAttachment = 4;
            break;
            }

        case DIMSTYLE_VALUE_MLNote_VerAttachment_Bottom:
            {
            pExtensions->flags2.uNoteVerRightAttachment = 5;
            break;
            }

        case DIMSTYLE_VALUE_MLNote_VerAttachment_DynamicLine:
            {
            pExtensions->flags2.uNoteVerRightAttachment = 6;
            pExtensions->flags.uElbowJointLocation = 0;
            break;
            }

        case DIMSTYLE_VALUE_MLNote_VerAttachment_DynamicCorner:
            {
            pExtensions->flags2.uNoteVerRightAttachment = 7;
            pExtensions->flags.uElbowJointLocation = 1;
            break;
            }

        case DIMSTYLE_VALUE_MLNote_VerAttachment_Underline:
        default:
            {
            pExtensions->flags2.uNoteVerRightAttachment = 8;
            pExtensions->flags.uElbowJointLocation = 2;
            break;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* Get the note text vertical left attachment override.
*
* @param        pValueOut   <=
* @param        dimElement        =>
* @return       true if exists
* @bsimethod                                                    SunandSandurkar 07/03
+---------------+---------------+---------------+---------------+---------------+------*/
bool          BentleyApi::mdlDim_getNoteVerLeftAttachment
(
UInt16      *pValueOut,
ElementHandleCR dimElement
)
    {
    bool                    bHasExtensions    = false;
    DimStyleExtensions      extensions;

    if (pValueOut)
        *pValueOut = DIMSTYLE_VALUE_MLNote_VerAttachment_TopLine;

    if (SUCCESS == mdlDim_getStyleExtension (&extensions, dimElement))
        {
        bHasExtensions = isOverride (extensions.modifiers, STYLE_Extension_Flags2);
        if (pValueOut)
            adim_getMLNoteVerLeftAttachment (&extensions, pValueOut);
        }

    return  bHasExtensions;
    }

/*---------------------------------------------------------------------------------**//**
@Description    Set the note text vertical left attachment override.
*
* @param        dimElement        IN the dimension element to set the value on.
* @param        pValueIn    IN value or NULL to restore default.
* @return       SUCCESS if can be set.
* @Group        "Dimension Functions"
* @bsimethod                                                    SunandSandurkar 07/03
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt     BentleyApi::mdlDim_setNoteVerLeftAttachment
(
EditElementHandleR dimElement,
UInt16      *pValueIn
)
    {
    DimStyleExtensions      extensions;

    memset (&extensions, 0, sizeof (extensions));
    mdlDim_getStyleExtension (&extensions, dimElement);

    // No effort is made to keep the flags modifier bit accurate.
    // It is derived from the flags.
    if (pValueIn)
        adim_setMLNoteVerLeftAttachment (&extensions, *pValueIn);

    return  mdlDim_setStyleExtension (dimElement, &extensions);
    }

/*---------------------------------------------------------------------------------**//**
* Get the note text vertical right attachment override.
*
* @param        pValueOut   <=
* @param        dimElement        =>
* @return       true if exists
* @bsimethod                                                    SunandSandurkar 07/03
+---------------+---------------+---------------+---------------+---------------+------*/
bool          BentleyApi::mdlDim_getNoteVerRightAttachment
(
UInt16      *pValueOut,
ElementHandleCR dimElement
)
    {
    bool                    bHasExtensions    = false;
    DimStyleExtensions      extensions;

    if (pValueOut)
        *pValueOut = DIMSTYLE_VALUE_MLNote_VerAttachment_TopLine;

    if (SUCCESS == mdlDim_getStyleExtension (&extensions, dimElement))
        {
        bHasExtensions = isOverride (extensions.modifiers, STYLE_Extension_Flags2);
        if (pValueOut)
            adim_getMLNoteVerRightAttachment (&extensions, pValueOut);
        }

    return  bHasExtensions;
    }

/*---------------------------------------------------------------------------------**//**
@Description    Set the note text vertical right attachment override.
*
* @param        dimElement        IN the dimension element to set the value on.
* @param        pValueIn    IN value or NULL to restore default.
* @return       SUCCESS if can be set.
* @Group        "Dimension Functions"
* @bsimethod                                                    SunandSandurkar 07/03
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt     BentleyApi::mdlDim_setNoteVerRightAttachment
(
EditElementHandleR dimElement,
UInt16      *pValueIn
)
    {
    DimStyleExtensions      extensions;

    memset (&extensions, 0, sizeof (extensions));
    mdlDim_getStyleExtension (&extensions, dimElement);

    // No effort is made to keep the flags modifier bit accurate.
    // It is derived from the flags.
    if (pValueIn)
        adim_setMLNoteVerRightAttachment (&extensions, *pValueIn);

    return  mdlDim_setStyleExtension (dimElement, &extensions);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar 07/03
+---------------+---------------+---------------+---------------+---------------+------*/
UInt16          DimensionStyle::GetMLNoteFrameType() const
    {
    UInt16  value = DIMSTYLE_PROP_Invalid;

    /* Refer the notes at the end of midimstyle.h */
    switch (m_data.ad5.flags.multiFrame)
        {
        case 0:
            value = DIMSTYLE_VALUE_MLNote_FrameType_None;
            break;

        case 1:
            value = DIMSTYLE_VALUE_MLNote_FrameType_Line;
            break;

        case 2:
            value = DIMSTYLE_VALUE_MLNote_FrameType_Box;
            break;

        case 3:
            {
            switch (m_extensions.flags2.uNoteFrame)
                {
                case 0:
                    value = DIMSTYLE_VALUE_MLNote_FrameType_RotatedBox;
                    break;
                case 1:
                    value = DIMSTYLE_VALUE_MLNote_FrameType_Circle;
                    break;
                case 2:
                    value = DIMSTYLE_VALUE_MLNote_FrameType_Capsule;
                    break;
                case 3:
                    value = DIMSTYLE_VALUE_MLNote_FrameType_Hexagon;
                    break;
                case 4:
                    value = DIMSTYLE_VALUE_MLNote_FrameType_RotatedHexagon;
                    break;
                case 5:
                    value = DIMSTYLE_VALUE_MLNote_FrameType_Triangle;
                    break;
                case 6:
                    value = DIMSTYLE_VALUE_MLNote_FrameType_Pentagon;
                    break;
                case 7:
                    value = DIMSTYLE_VALUE_MLNote_FrameType_Octagon;
                    break;
                default:
                    BeAssert (false);
                }
            break;
            }
        default:
            BeAssert (false);
        }

    return value;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar 07/03
+---------------+---------------+---------------+---------------+---------------+------*/
void            DimensionStyle::SetMLNoteFrameType (UInt16 value)
    {
    /* Refer the notes at the end of midimstyle.h */
    switch (value)
        {
        case DIMSTYLE_VALUE_MLNote_FrameType_None:
            m_data.ad5.flags.multiFrame = 0;
            m_extensions.flags2.uNoteFrame = 0;
            break;

        case DIMSTYLE_VALUE_MLNote_FrameType_Line:
            m_data.ad5.flags.multiFrame = 1;
            m_extensions.flags2.uNoteFrame = 0;
            break;

        case DIMSTYLE_VALUE_MLNote_FrameType_Box:
            m_data.ad5.flags.multiFrame = 2;
            m_extensions.flags2.uNoteFrame = 0;
            break;

        case DIMSTYLE_VALUE_MLNote_FrameType_RotatedBox:
            m_data.ad5.flags.multiFrame = 3;
            m_extensions.flags2.uNoteFrame = 0;
            break;

        case DIMSTYLE_VALUE_MLNote_FrameType_Circle:
            m_data.ad5.flags.multiFrame = 3;
            m_extensions.flags2.uNoteFrame = 1;
            break;

        case DIMSTYLE_VALUE_MLNote_FrameType_Capsule:
            m_data.ad5.flags.multiFrame = 3;
            m_extensions.flags2.uNoteFrame = 2;
            break;

        case DIMSTYLE_VALUE_MLNote_FrameType_Hexagon:
            m_data.ad5.flags.multiFrame = 3;
            m_extensions.flags2.uNoteFrame = 3;
            break;

        case DIMSTYLE_VALUE_MLNote_FrameType_RotatedHexagon:
            m_data.ad5.flags.multiFrame = 3;
            m_extensions.flags2.uNoteFrame = 4;
            break;

        case DIMSTYLE_VALUE_MLNote_FrameType_Triangle:
            m_data.ad5.flags.multiFrame = 3;
            m_extensions.flags2.uNoteFrame = 5;
            break;

        case DIMSTYLE_VALUE_MLNote_FrameType_Pentagon:
            m_data.ad5.flags.multiFrame = 3;
            m_extensions.flags2.uNoteFrame = 6;
            break;

        case DIMSTYLE_VALUE_MLNote_FrameType_Octagon:
            m_data.ad5.flags.multiFrame = 3;
            m_extensions.flags2.uNoteFrame = 7;
            break;
        }
    }

/*---------------------------------------------------------------------------------**//**
* Get primary roundoff.
*
* @param        pValueOut   <=
* @param        dimElement        =>
* @return       true if exists
* @bsimethod                                                    petri.niiranen  09/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool          BentleyApi::mdlDim_getPrimaryRoundOff
(
double      *pValueOut,
ElementHandleCR dimElement
)
    {
    bool                    bHasExtensions    = false;
    DimStyleExtensions      extensions;

    if (SUCCESS == mdlDim_getStyleExtension (&extensions, dimElement))
        {
        bHasExtensions = isOverride (extensions.modifiers, STYLE_Extension_RoundOff);
        if (pValueOut)
            *pValueOut = bHasExtensions ? extensions.dRoundOff : 0.0;
        }

    return  bHasExtensions;
    }

/*---------------------------------------------------------------------------------**//**
@Description    Set secondary tolerance accuracy override.
*
* @param        dimElement        IN the dimension element to set the value on.
* @param        pValueIn    IN value or NULL to restore default.
* @return       SUCCESS if can be set.
* @Group        "Dimension Functions"
* @bsimethod                                                    petri.niiranen  09/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt     BentleyApi::mdlDim_setPrimaryRoundOff
(
EditElementHandleR dimElement,
double      *pValueIn
)
    {
    DimStyleExtensions      extensions;

    memset (&extensions, 0, sizeof (extensions));
    mdlDim_getStyleExtension (&extensions, dimElement);

    if (NULL == pValueIn)
        extensions.modifiers &= ~STYLE_Extension_RoundOff;
    else
        {
        extensions.modifiers    |= STYLE_Extension_RoundOff;
        extensions.dRoundOff = *pValueIn;
        }

    return  mdlDim_setStyleExtension (dimElement, &extensions);
    }

/*---------------------------------------------------------------------------------**//**
* Get primary roundoff.
*
* @param        pValueOut   <=
* @param        dimElement        =>
* @return       true if exists
* @bsimethod                                                    petri.niiranen  09/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool          BentleyApi::mdlDim_getSecondaryRoundOff
(
double      *pValueOut,
ElementHandleCR dimElement
)
    {
    bool                    bHasExtensions    = false;
    DimStyleExtensions      extensions;

    if (SUCCESS == mdlDim_getStyleExtension (&extensions, dimElement))
        {
        bHasExtensions = isOverride (extensions.modifiers, STYLE_Extension_SecondaryRoundOff);
        if (pValueOut)
            *pValueOut = bHasExtensions ? extensions.dSecondaryRoundOff : 0.0;
        }

    return  bHasExtensions;
    }

/*---------------------------------------------------------------------------------**//**
@Description    Set secondary tolerance accuracy override.
*
* @param        dimElement        IN the dimension element to set the value on.
* @param        pValueIn    IN value or NULL to restore default.
* @return       SUCCESS if can be set.
* @Group        "Dimension Functions"
* @bsimethod                                                    petri.niiranen  09/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt     BentleyApi::mdlDim_setSecondaryRoundOff
(
EditElementHandleR dimElement,
double      *pValueIn
)
    {
    DimStyleExtensions      extensions;

    memset (&extensions, 0, sizeof (extensions));
    mdlDim_getStyleExtension (&extensions, dimElement);

    if (NULL == pValueIn)
        extensions.modifiers &= ~STYLE_Extension_SecondaryRoundOff;
    else
        {
        extensions.modifiers    |= STYLE_Extension_SecondaryRoundOff;
        extensions.dSecondaryRoundOff = *pValueIn;
        }

    return  mdlDim_setStyleExtension (dimElement, &extensions);
    }

/*---------------------------------------------------------------------------------**//**
* Get elbow length override.
*
* @param        pValueOut   <=
* @param        dimElement        =>
* @return       true if exists
* @bsimethod                                                    SunandSandurkar 07/03
+---------------+---------------+---------------+---------------+---------------+------*/
bool          BentleyApi::mdlDim_getNoteElbowLength
(
double      *pValueOut,
ElementHandleCR dimElement
)
    {
    bool                    bHasExtensions    = false;
    double                  value = 0.0;
    DimStyleExtensions      extensions;

    if (SUCCESS == mdlDim_getStyleExtension (&extensions, dimElement) &&
        true    == (bHasExtensions = isOverride (extensions.modifiers, STYLE_Extension_NoteElbowLength)))
        value = extensions.dNoteElbowLength;
    else
        value = 2.0;

    if (pValueOut)
        *pValueOut = value;

    return  bHasExtensions;
    }

/*---------------------------------------------------------------------------------**//**
@Description    Set the elbow length override.
*
* @param        dimElement        IN the dimension element to set the value on.
* @param        pValueIn    IN value or NULL to restore default.
* @return       SUCCESS if can be set.
* @Group        "Dimension Functions"
* @bsimethod                                                   SunandSandurkar  10/02
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt     BentleyApi::mdlDim_setNoteElbowLength
(
EditElementHandleR dimElement,
double      *pValueIn
)
    {
    DimStyleExtensions      extensions;

    memset (&extensions, 0, sizeof (extensions));
    mdlDim_getStyleExtension (&extensions, dimElement);

    if (NULL == pValueIn)
        {
        extensions.modifiers    &= ~STYLE_Extension_NoteElbowLength;
        extensions.dNoteElbowLength =  0.0;
        }
    else
        {
        extensions.modifiers    |= STYLE_Extension_NoteElbowLength;
        extensions.dNoteElbowLength = *pValueIn;
        }

    return  mdlDim_setStyleExtension (dimElement, &extensions);
    }

/*---------------------------------------------------------------------------------**//**
* Get the note left margin override.
*
* @param        pValueOut   <=
* @param        dimElement        =>
* @return       true if exists
* @bsimethod                                                    SunandSandurkar 07/03
+---------------+---------------+---------------+---------------+---------------+------*/
bool          BentleyApi::mdlDim_getNoteLeftMargin
(
double      *pValueOut,
ElementHandleCR dimElement
)
    {
    bool                    bHasExtensions    = false;
    double                  value = 0.0;
    DimStyleExtensions      extensions;

    if (SUCCESS == mdlDim_getStyleExtension (&extensions, dimElement) &&
        true    == (bHasExtensions = isOverride (extensions.modifiers, STYLE_Extension_NoteLeftMargin)))
        value = extensions.dNoteLeftMargin;
    else
        value = 0.5;

    if (pValueOut)
        *pValueOut = value;

    return  bHasExtensions;
    }

/*---------------------------------------------------------------------------------**//**
@Description    Set the note left margin override.
*
* @param        dimElement        IN the dimension element to set the value on.
* @param        pValueIn    IN value or NULL to restore default.
* @return       SUCCESS if can be set.
* @Group        "Dimension Functions"
* @bsimethod                                                   SunandSandurkar  07/03
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt     BentleyApi::mdlDim_setNoteLeftMargin
(
EditElementHandleR dimElement,
double      *pValueIn
)
    {
    DimStyleExtensions      extensions;

    memset (&extensions, 0, sizeof (extensions));
    mdlDim_getStyleExtension (&extensions, dimElement);

    if (NULL == pValueIn)
        {
        extensions.modifiers    &= ~STYLE_Extension_NoteLeftMargin;
        extensions.dNoteLeftMargin =  0.0;
        }
    else
        {
        extensions.modifiers    |= STYLE_Extension_NoteLeftMargin;
        extensions.dNoteLeftMargin = *pValueIn;
        }

    return  mdlDim_setStyleExtension (dimElement, &extensions);
    }

/*---------------------------------------------------------------------------------**//**
* Get the note lower margin override.
*
* @param        pValueOut   <=
* @param        dimElement        =>
* @return       true if exists
* @bsimethod                                                    SunandSandurkar 07/03
+---------------+---------------+---------------+---------------+---------------+------*/
bool          BentleyApi::mdlDim_getNoteLowerMargin
(
double      *pValueOut,
ElementHandleCR dimElement
)
    {
    bool                    bHasExtensions    = false;
    double                  value = 0.0;
    DimStyleExtensions      extensions;

    if (SUCCESS == mdlDim_getStyleExtension (&extensions, dimElement) &&
        true    == (bHasExtensions = isOverride (extensions.modifiers, STYLE_Extension_NoteLowerMargin)))
        value = extensions.dNoteLowerMargin;
    else
        value = 0.5;

    if (pValueOut)
        *pValueOut = value;

    return  bHasExtensions;
    }

/*---------------------------------------------------------------------------------**//**
@Description    Set the note lower margin override.
*
* @param        dimElement        IN the dimension element to set the value on.
* @param        pValueIn    IN value or NULL to restore default.
* @return       SUCCESS if can be set.
* @Group        "Dimension Functions"
* @bsimethod                                                   SunandSandurkar  07/03
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt     BentleyApi::mdlDim_setNoteLowerMargin
(
EditElementHandleR dimElement,
double      *pValueIn
)
    {
    DimStyleExtensions      extensions;

    memset (&extensions, 0, sizeof (extensions));
    mdlDim_getStyleExtension (&extensions, dimElement);

    if (NULL == pValueIn)
        {
        extensions.modifiers    &= ~STYLE_Extension_NoteLowerMargin;
        extensions.dNoteLowerMargin =  0.0;
        }
    else
        {
        extensions.modifiers    |= STYLE_Extension_NoteLowerMargin;
        extensions.dNoteLowerMargin = *pValueIn;
        }

    return  mdlDim_setStyleExtension (dimElement, &extensions);
    }

/*---------------------------------------------------------------------------------**//**
* Get the annotation scale override.
*
* @param        pValueOut   <=
* @param        dimElement        =>
* @return       true if exists
* @bsimethod                                                    SunandSandurkar 09/03
+---------------+---------------+---------------+---------------+---------------+------*/
bool          BentleyApi::mdlDim_getAnnotationScale
(
double                  *pValueOut,
ElementHandleCR            dimElement
)
    {
    bool                    bHasExtensions    = false;
    DimStyleExtensions      extensions;

    if (SUCCESS == mdlDim_getStyleExtension (&extensions, dimElement))
        {
        bHasExtensions = isOverride (extensions.modifiers, STYLE_Extension_AnnotationScale);
        if (pValueOut)
            *pValueOut = bHasExtensions ? extensions.dAnnotationScale : 1.0;
        }

    return  bHasExtensions;
    }

/*---------------------------------------------------------------------------------**//**
@Description    Set the annotation scale override.
*
* @param        dimElement        IN the dimension element to set the value on.
* @param        pValueIn    IN value or NULL to restore default.
* @return       SUCCESS if can be set.
* @Group        "Dimension Functions"
* @bsimethod                                                   SunandSandurkar  09/03
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt     BentleyApi::mdlDim_setAnnotationScale
(
EditElementHandleR dimElement,
double      *pValueIn
)
    {
    DimStyleExtensions      extensions;

    memset (&extensions, 0, sizeof (extensions));
    mdlDim_getStyleExtension (&extensions, dimElement);

    if (NULL == pValueIn)
        {
        extensions.modifiers    &= ~STYLE_Extension_AnnotationScale;
        extensions.dAnnotationScale =  1.0;
        }
    else
        {
        extensions.modifiers    |= STYLE_Extension_AnnotationScale;
        extensions.dAnnotationScale = *pValueIn;
        }

    return  mdlDim_setStyleExtension (dimElement, &extensions);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SunandSandurkar 06/06
+---------------+---------------+---------------+---------------+---------------+------*/
bool             BentleyApi::mdlDim_getEffectiveAnnotationScale
(
double *                pValueOut,
ElementHandleCR dimElement
)
    {
    double              curScale        = 1.0;
    bool                usingScale      = false;
    bool                usingStyleScale = false;

    if (false == mdlDim_getNotUseModelAnnotationScaleFlag (&usingStyleScale, dimElement) || false == usingStyleScale)
        usingScale = mdlDim_overallGetModelAnnotationScale (&curScale, dimElement);
    else
        usingScale = mdlDim_getAnnotationScale (&curScale, dimElement);

    if (pValueOut)
        *pValueOut = curScale;

    return usingScale;
    }

/*---------------------------------------------------------------------------------**//**
* Get the Note Frame Scale override.
*
* @param        pValueOut   <=
* @param        dimElement        =>
* @return       true if exists
* @bsimethod                                                    SunandSandurkar 09/03
+---------------+---------------+---------------+---------------+---------------+------*/
bool          BentleyApi::mdlDim_getNoteFrameScale
(
double                  *pValueOut,
ElementHandleCR            dimElement
)
    {
    bool                    bHasExtensions    = false;
    DimStyleExtensions      extensions;

    if (SUCCESS == mdlDim_getStyleExtension (&extensions, dimElement))
        {
        bHasExtensions = isOverride (extensions.modifiers, STYLE_Extension_NoteFrameScale);
        if (pValueOut)
            *pValueOut = bHasExtensions ? extensions.dNoteFrameScale : 1.0;
        }

    return  bHasExtensions;
    }

/*---------------------------------------------------------------------------------**//**
@Description    Set the Note Frame Scale override.
*
* @param        dimElement        IN the dimension element to set the value on.
* @param        pValueIn    IN value or NULL to restore default.
* @return       SUCCESS if can be set.
* @Group        "Dimension Functions"
* @bsimethod                                                   SunandSandurkar  09/03
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt     BentleyApi::mdlDim_setNoteFrameScale
(
EditElementHandleR dimElement,
double      *pValueIn
)
    {
    DimStyleExtensions      extensions;

    memset (&extensions, 0, sizeof (extensions));
    mdlDim_getStyleExtension (&extensions, dimElement);

    if (NULL == pValueIn)
        {
        extensions.modifiers    &= ~STYLE_Extension_NoteFrameScale;
        extensions.dNoteFrameScale =  1.0;
        }
    else
        {
        extensions.modifiers    |= STYLE_Extension_NoteFrameScale;
        extensions.dNoteFrameScale = *pValueIn;
        }

    return  mdlDim_setStyleExtension (dimElement, &extensions);
    }

/*---------------------------------------------------------------------------------**//**
* Get Note Terminator Font override.
*
* @param        pValueOut   <=
* @param        dimElement        =>
* @return       true if exists
* @bsimethod                                                    SunandSandurkar 07/03
+---------------+---------------+---------------+---------------+---------------+------*/
bool          mdlDim_getNoteTerminatorFont
(
UInt32      *pValueOut,
ElementHandleCR dimElement
)
    {
    bool                    bHasExtensions    = false;
    DimStyleExtensions      extensions;

    if (SUCCESS == mdlDim_getStyleExtension (&extensions, dimElement))
        {
        bHasExtensions = isOverride (extensions.modifiers, STYLE_Extension_NoteTerminatorFont);
        if (pValueOut)
            *pValueOut = bHasExtensions ? extensions.iNoteTerminatorFont : 0;
        }

    return  bHasExtensions;
    }

/*---------------------------------------------------------------------------------**//**
@Description    Set the Note Terminator Font override.
*
* @param        dimElement        IN the dimension element to set the value on.
* @param        pValueIn    IN value or NULL to restore default.
* @return       SUCCESS if can be set.
* @Group        "Dimension Functions"
* @bsimethod                                                   SunandSandurkar  10/02
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt     mdlDim_setNoteTerminatorFont
(
EditElementHandleR dimElement,
UInt32      *pValueIn
)
    {
    DimStyleExtensions      extensions;

    memset (&extensions, 0, sizeof (extensions));
    mdlDim_getStyleExtension (&extensions, dimElement);

    if (NULL == pValueIn)
        {
        extensions.modifiers    &= ~STYLE_Extension_NoteTerminatorFont;
        extensions.iNoteTerminatorFont =  0;
        }
    else
        {
        extensions.modifiers    |= STYLE_Extension_NoteTerminatorFont;
        extensions.iNoteTerminatorFont = *pValueIn;
        }

    return  mdlDim_setStyleExtension (dimElement, &extensions);
    }

/*---------------------------------------------------------------------------------**//**
* Get Note Terminator Char override.
*
* @param        pValueOut   <=
* @param        dimElement        =>
* @return       true if exists
* @bsimethod                                                    SunandSandurkar 07/03
+---------------+---------------+---------------+---------------+---------------+------*/
bool          mdlDim_getNoteTerminatorChar
(
UInt32      *pValueOut,
ElementHandleCR dimElement
)
    {
    bool                    bHasExtensions    = false;
    DimStyleExtensions      extensions;

    if (SUCCESS == mdlDim_getStyleExtension (&extensions, dimElement))
        {
        bHasExtensions = isOverride (extensions.modifiers, STYLE_Extension_NoteTerminatorChar);
        if (pValueOut)
            *pValueOut = bHasExtensions ? extensions.iNoteTerminatorChar : 0;
        }

    return  bHasExtensions;
    }

/*---------------------------------------------------------------------------------**//**
@Description    Set the Note Terminator Char override.
*
* @param        dimElement        IN the dimension element to set the value on.
* @param        pValueIn    IN value or NULL to restore default.
* @return       SUCCESS if can be set.
* @Group        "Dimension Functions"
* @bsimethod                                                   SunandSandurkar  10/02
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt     mdlDim_setNoteTerminatorChar
(
EditElementHandleR dimElement,
UInt16      *pValueIn
)
    {
    DimStyleExtensions      extensions;

    memset (&extensions, 0, sizeof (extensions));
    mdlDim_getStyleExtension (&extensions, dimElement);

    if (NULL == pValueIn)
        {
        extensions.modifiers    &= ~STYLE_Extension_NoteTerminatorChar;
        extensions.iNoteTerminatorChar =  0;
        }
    else
        {
        extensions.modifiers    |= STYLE_Extension_NoteTerminatorChar;
        extensions.iNoteTerminatorChar = *pValueIn;
        }

    return  mdlDim_setStyleExtension (dimElement, &extensions);
    }

/*---------------------------------------------------------------------------------**//**
* Get effective ordinate property
*
* @param        dimElement            =>
* @param        segmentNo       =>
* @param        pValueIn        => value or NULL to restore default
* @return       true if override exists
* @bsimethod                                                    petri.niiranen  04/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool          BentleyApi::mdlDim_ordinateGetStartValueX
(
double          *pValueOut,
ElementHandleCR    dimElement
)
    {
    bool                    bIsOverride    = false;
    DimOrdinateOverrides    ordinate;

    if (SUCCESS == mdlDim_getOrdinateOverride (&ordinate, dimElement))
        {
        bIsOverride = isOverride (ordinate.modifiers, ORDINATE_Override_StartValueX);
        if (pValueOut)
            *pValueOut = bIsOverride ? ordinate.startValueX : 0.0;
        }

    return  bIsOverride;
    }

/*---------------------------------------------------------------------------------**//**
* Set effective ordinate property.
*
* @param        dimElement            =>
* @param        pValueIn        => value or NULL to restore default
* @return       SUCCESS if can be set
* @bsimethod                                                    petri.niiranen  02/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt     BentleyApi::mdlDim_ordinateSetStartValueX
(
EditElementHandleR dimElement,
double      *pValueIn
)
    {
    DimOrdinateOverrides    ordinate;

    memset (&ordinate, 0, sizeof (ordinate));
    mdlDim_getOrdinateOverride (&ordinate, dimElement);

    if (NULL == pValueIn)
        ordinate.modifiers &= ~ORDINATE_Override_StartValueX;
    else
        {
        ordinate.modifiers |= ORDINATE_Override_StartValueX;
        ordinate.startValueX = *pValueIn;
        }

    return  mdlDim_setOrdinateOverride (dimElement, &ordinate);
    }

/*---------------------------------------------------------------------------------**//**
* Get effective ordinate property
*
* @param        dimElement            =>
* @param        segmentNo       =>
* @param        pValueIn        => value or NULL to restore default
* @return       true if override exists
* @bsimethod                                                    petri.niiranen  04/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool          BentleyApi::mdlDim_ordinateGetStartValueY
(
double          *pValueOut,
ElementHandleCR    dimElement
)
    {
    bool                    bIsOverride    = false;
    DimOrdinateOverrides    ordinate;

    if (SUCCESS == mdlDim_getOrdinateOverride (&ordinate, dimElement))
        {
        bIsOverride = isOverride (ordinate.modifiers, ORDINATE_Override_StartValueY);
        if (pValueOut)
            *pValueOut = bIsOverride ? ordinate.startValueY : 0.0;
        }

    return  bIsOverride;
    }

/*---------------------------------------------------------------------------------**//**
* Set effective ordinate property.
*
* @param        dimElement            =>
* @param        pValueIn        => value or NULL to restore default
* @return       SUCCESS if can be set
* @bsimethod                                                    petri.niiranen  02/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt     BentleyApi::mdlDim_ordinateSetStartValueY
(
EditElementHandleR dimElement,
double      *pValueIn
)
    {
    DimOrdinateOverrides    ordinate;

    memset (&ordinate, 0, sizeof (ordinate));
    mdlDim_getOrdinateOverride (&ordinate, dimElement);

    if (NULL == pValueIn)
        ordinate.modifiers &= ~ORDINATE_Override_StartValueY;
    else
        {
        ordinate.modifiers |= ORDINATE_Override_StartValueY;
        ordinate.startValueX = *pValueIn;
        }

    return  mdlDim_setOrdinateOverride (dimElement, &ordinate);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    08/02
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt    BentleyApi::adim_allocateAndExtractTextStyleForStroke
(
LegacyTextStyle         **ppTextStyle,
ElementHandleCR      dimElement
)
    {
    StatusInt   status;
    LegacyTextStyle   textStyle;

    memset (&textStyle, 0, sizeof(textStyle));

    if (SUCCESS == (status = DimensionHandler::GetInstance().GetTextStyle (dimElement, &textStyle)))
        {
        LegacyTextStyle testTextStyle;

        memset (&testTextStyle, 0, sizeof(testTextStyle));
        memset (&textStyle.overrideFlags, 0, sizeof(textStyle).overrideFlags);

        // The style overrides are not relevent to stroking the dimension.  If the
        // dimension's text style contains only overrides, then we don't want to
        // consider the text style at all while stroking.
        if (0 == memcmp (&testTextStyle, &textStyle, sizeof(testTextStyle)))
            return ERROR;

        if (NULL == (*ppTextStyle = (LegacyTextStyle*) memutil_malloc (sizeof (LegacyTextStyle), HEAPSIG_ADRC)))
            return DGNMODEL_STATUS_OutOfMemory;

        memcpy (*ppTextStyle, &textStyle, sizeof(textStyle));
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          10/05
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt                BentleyApi::mdlDim_getBallNChainMode
(
DimStyleProp_BallAndChain_Mode  *pBncMode,
ElementHandleCR                    dimElement
)
    {
    *pBncMode = DIMSTYLE_VALUE_BallAndChain_Mode_None;

    if (dimElement.IsValid() && NULL != mdlDim_getOptionBlock(dimElement, ADBLK_EXTOFFSET, NULL))
        {
        DimStyleExtensions      extensions;

        memset (&extensions, 0, sizeof (extensions));

        mdlDim_getStyleExtension (&extensions, dimElement);

        if ((extensions.modifiers & STYLE_Extension_Flags3) && extensions.flags3.uAutoBallNChain)
            *pBncMode = DIMSTYLE_VALUE_BallAndChain_Mode_Auto;
        else
            *pBncMode = DIMSTYLE_VALUE_BallAndChain_Mode_On;
        }

    return  SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          11/05
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt                BentleyApi::mdlDim_getFitOption
(
DimStyleProp_FitOptions         *pFitOption,
ElementHandleCR                    dimElement
)
    {
    DimStyleExtensions          extensions;

    if (DIMENSION_ELM != dimElement.GetLegacyType())
        return  ERROR;

    memset (&extensions, 0, sizeof (extensions));

    if (SUCCESS == mdlDim_getStyleExtension(&extensions, dimElement) && (extensions.modifiers & STYLE_Extension_Flags3))
        *pFitOption = (DimStyleProp_FitOptions) extensions.flags3.uFitOption;
    else
        *pFitOption = (DimStyleProp_FitOptions) (dimElement.GetElementCP()->ToDimensionElm().flag.termMode);

    return  SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          11/05
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt                BentleyApi::mdlDim_setFitOption
(
EditElementHandleR                 dimElement,
DimStyleProp_FitOptions         fitOption
)
    {
    DimStyleExtensions          extensions;
    bool                        bLinkageChanged = false;
    StatusInt                   status = SUCCESS;

    if (fitOption >= DIMSTYLE_VALUE_FitOption_COUNT || !dimElement.IsValid() || DIMENSION_ELM != dimElement.GetLegacyType())
        return  ERROR;

    memset (&extensions, 0, sizeof (extensions));

    mdlDim_getStyleExtension (&extensions, dimElement);

    if (0 == (extensions.modifiers & STYLE_Extension_Flags3))
        {
        extensions.modifiers |= STYLE_Extension_Flags3;
        bLinkageChanged = true;
        }

    if (extensions.flags3.uFitOption != fitOption)
        {
        extensions.flags3.uFitOption = fitOption;
        bLinkageChanged = true;
        }

    if (bLinkageChanged)
        status = mdlDim_setStyleExtension (dimElement, &extensions);
    
    if (fitOption < DimStyleProp_FitOptions::DIMSTYLE_VALUE_FitOption_KeepTextInside)
        dimElement.GetElementP()->ToDimensionElmR().flag.termMode = fitOption;
    else
        dimElement.GetElementP()->ToDimensionElmR().flag.termMode = DIMSTYLE_VALUE_FitOption_MoveTermsFirst;

    return  status;
    }
