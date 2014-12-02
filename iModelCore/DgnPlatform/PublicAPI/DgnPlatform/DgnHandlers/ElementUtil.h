/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnHandlers/ElementUtil.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

#include <DgnPlatform/DgnPlatform.h>

BEGIN_BENTLEY_API_NAMESPACE

DGNPLATFORM_EXPORT void pattern_setOverrideCell (MSElementDescrCP cellEdP);
DGNPLATFORM_EXPORT MSElementDescrCP pattern_getOverrideCell ();


DGNPLATFORM_EXPORT bool      in_span
(
double          theta,
double          start,
double          sweep
);


END_BENTLEY_API_NAMESPACE

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//=======================================================================================
//! @bsiclass                                                     Brien.Bastings  02/09
//=======================================================================================
struct CellUtil
{
DGNPLATFORM_EXPORT static BentleyStatus ExtractName (WCharP cellName, int bufferSize, ElementHandleCR eh);
DGNPLATFORM_EXPORT static BentleyStatus ExtractDescription (WCharP descr, int bufferSize, ElementHandleCR eh);
DGNPLATFORM_EXPORT static BentleyStatus GetCellName (WCharP name, int bufferSize, DgnElementCR elm);
DGNPLATFORM_EXPORT static BentleyStatus GetCellDescription (WCharP descr, int bufferSize, DgnElementCR elm);
DGNPLATFORM_EXPORT static BentleyStatus SetCellName (DgnElementR elm, WCharCP name);
DGNPLATFORM_EXPORT static BentleyStatus SetCellDescription (DgnElementR elm, WCharCP descr);
DGNPLATFORM_EXPORT static bool          IsPointCell (ElementHandleCR eh);
DGNPLATFORM_EXPORT static bool          IsAnnotation (ElementHandleCR eh);
DGNPLATFORM_EXPORT static bool          IsAnonymous (ElementHandleCR eh);
DGNPLATFORM_EXPORT static BentleyStatus ExtractOrigin (DPoint3dR origin, ElementHandleCR eh);
DGNPLATFORM_EXPORT static BentleyStatus ExtractRangeDiagonal (DRange3dR range, ElementHandleCR eh);
DGNPLATFORM_EXPORT static BentleyStatus ExtractRotation (RotMatrixR rMatrix, ElementHandleCR eh);
#if defined (NEEDS_WORK_DGNITEM)
DGNPLATFORM_EXPORT static ElementRefP   FindSharedCellDefinition (DgnElementCR scInstance, DgnProjectR dgnFile);
DGNPLATFORM_EXPORT static BentleyStatus GetSharedCellDefID (ElementHandleCR eh, ElementId& elemID);
DGNPLATFORM_EXPORT static BentleyStatus GetSharedCellDefFlags (bool* pNondefaultScaleForDims, bool* pScaleMultilines, bool* pRotateDimView, bool* pIsAnnotation, ElementHandleCR eh);
DGNPLATFORM_EXPORT static BentleyStatus SetSharedCellDefFlags (EditElementHandleR eeh, bool nondefaultScaleForDims, bool scaleMultilines, bool rotateDimView, bool isAnnotation);
#endif
DGNPLATFORM_EXPORT static BentleyStatus ClipFromLinkage (ClipVectorPtr&, ElementHandleCR, TransformCP outputTransform = NULL, bool applyElementTransform = true);
DGNPLATFORM_EXPORT static BentleyStatus AddClipLinkage (EditElementHandleR, RotMatrixCR rMatrix, DPoint3dCR origin, double zFront, double zBack, bool frontClipOn, bool backClipOn, size_t nPoints, DPoint2dCP points, bool applyElementTransform = true);
DGNPLATFORM_EXPORT static BentleyStatus DeleteClipLinkage (EditElementHandleR);
};

//=======================================================================================
//! These methods append and extract basic data types from byte streams,
//! advancing the provided pointer when done.
//! @bsiclass                                                     Jeff.Marker     01/09
//=======================================================================================
struct ByteStreamHelper
{
DGNPLATFORM_EXPORT static void AppendRotMatrix (byte*& buffer, RotMatrixCR value, bool is3d);
DGNPLATFORM_EXPORT static void AppendDPoint3d (byte*& buffer, DPoint3dCR value);
DGNPLATFORM_EXPORT static void AppendDouble (byte*& buffer, double const & value);
DGNPLATFORM_EXPORT static void AppendLong (byte*& buffer, long const & value);
DGNPLATFORM_EXPORT static void AppendShort (byte*& buffer, short const & value);
DGNPLATFORM_EXPORT static void AppendInt64 (byte*& buffer, Int64 const & value);
DGNPLATFORM_EXPORT static void AppendUInt32 (byte*& buffer, UInt32 const & value);
DGNPLATFORM_EXPORT static void AppendInt (byte*& buffer, int const & value);
DGNPLATFORM_EXPORT static void AppendUInt16 (byte*& buffer, UInt16 const & value);
DGNPLATFORM_EXPORT static void AppendInt32 (byte*& buffer, Int32 const & value);
DGNPLATFORM_EXPORT static void AppendUShort (byte*& buffer, UShort const & value);
DGNPLATFORM_EXPORT static void AppendULong (byte*& buffer, ULong const & value);
DGNPLATFORM_EXPORT static void ExtractRotMatrix (RotMatrixR value, byte const *& buffer, bool is3d);
DGNPLATFORM_EXPORT static void ExtractDPoint3d (DPoint3dR value, byte const *& buffer);
DGNPLATFORM_EXPORT static void ExtractDouble (double& value, byte const *& buffer);
DGNPLATFORM_EXPORT static void ExtractLong (long& value, byte const *& buffer);
DGNPLATFORM_EXPORT static void ExtractShort (short& value, byte const *& buffer);
DGNPLATFORM_EXPORT static void ExtractInt64 (Int64& value, byte const *& buffer);
DGNPLATFORM_EXPORT static void ExtractUInt32 (UInt32& value, byte const *& buffer);
DGNPLATFORM_EXPORT static void ExtractInt (int& value, byte const *& buffer);
DGNPLATFORM_EXPORT static void ExtractUInt16 (UInt16& value, byte const *& buffer);
DGNPLATFORM_EXPORT static void ExtractInt32 (Int32& value, byte const *& buffer);
DGNPLATFORM_EXPORT static void ExtractUShort (UShort& value, byte const *& buffer);
DGNPLATFORM_EXPORT static void ExtractULong (ULong& value, byte const *& buffer);
};

//=======================================================================================
//! @bsiclass                                                     Brien.Bastings  02/09
//=======================================================================================
struct PatternLinkageUtil
{
DGNPLATFORM_EXPORT static void GetHatchOrigin (DPoint3dR origin, DgnElementCR elm);
DGNPLATFORM_EXPORT static int Create (HatchLinkageR linkage, PatternParamsR params, DwgHatchDefLineP pHatchLines, bool is3d);
DGNPLATFORM_EXPORT static StatusInt Extract (PatternParamsR params, DwgHatchDefLineP pHatchLines, int nMaxLines, HatchLinkageP pLinkage, bool is3d);
DGNPLATFORM_EXPORT static StatusInt Extract (PatternParamsR params, DwgHatchDefLineP pHatchLines, int nMaxLines, byte const* buffer, bool is3d);
DGNPLATFORM_EXPORT static void Transform (PatternParamsR pParams, DwgHatchDefLineP pHatchLines, TransformCR transform, bool flatten, bool allowSizeChange);
DGNPLATFORM_EXPORT static StatusInt AddToElement (DgnElementR elm, PatternParamsR params, DwgHatchDefLineP pHatchLines, int index);
DGNPLATFORM_EXPORT static StatusInt ExtractFromElement (HatchLinkageP* ppHatchLinkage, PatternParamsR params, DwgHatchDefLineP pHatchDefLines, int maxHatchDefLines, DPoint3dP pOrigin, DgnElementCR elm, DgnModelP dgnCache, int index);
DGNPLATFORM_EXPORT static int DeleteFromElement (DgnElementR elm, int index);
DGNPLATFORM_EXPORT static StatusInt OnElementTransform (EditElementHandleR el, TransformCR transform, bool allowSizeChange);
};


struct      PointVector : bvector<DPoint3d> {};

/*=================================================================================**//**
* @bsiclass                                                     Brien.Bastings  05/2010
+===============+===============+===============+===============+===============+======*/
struct          CurveVectorUtil
{
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/2010
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT static BentleyStatus PartialDeleteElement
(
EditElementHandleR  outEeh1,
EditElementHandleR  outEeh2,
ElementHandleCR     eh,
DPoint3dCP          pointF,             //  => first point
DPoint3dCP          pointL,             //  => last point
DPoint3dCP          pointD,             //  => direction point
ViewportP           vp
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  05/2010
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT static BentleyStatus PartialDeleteElement
(
EditElementHandleR  outEeh1,
EditElementHandleR  outEeh2,
ElementHandleCR     eh,
DPoint3dCP          pointF,             //  => first point
DPoint3dCP          pointL,             //  => last point
DVec3dP             dirVec,             // <=> Output when computeDirection
bool                computeDirection,   //  => compute and return direction from first and last
ViewportP           vp
);

}; // CurveVectorUtil

/*=================================================================================**//**
*
* @bsiclass                                                     Sam.Wilson      10/2008
+===============+===============+===============+===============+===============+======*/
struct          ElementUtil
{
DGNPLATFORM_EXPORT static void SetRequiredFields (DgnElementR u, LevelId level, bool is3d);
DGNPLATFORM_EXPORT static void SetRequiredFields (DgnElementR out, ElementHandleCP templateEh, int defaultSize, bool copyAttributes, bool initScanRange, bool is3d, DgnModelP modelRef);

static void             CopyAttributes (DgnElementP dstElmP, DgnElementCP srcElmP);
static int              ExtractAttributes (UInt16 *bufferP, int maxAttriutebSize, DgnElementCR srcElm);
static void             StripAttributes (DgnElementR elem);
static BentleyStatus    AppendAttributes (DgnElementR elem, UInt16 *bufferP, int attribWords);

static void     InitScanRangeForUnion (DRange3dR, bool is3d);
static void InitializeGraphicsElement (EditElementHandleR eh, DgnModelR model, bool is3d, UInt16 size, bool zeroOutElementData = true);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/04
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt    NonUniformScaleAsBsplineSurf
(
bool&           wasHandled,
EditElementHandleR elemHandle,
TransformInfoCR trans,
RotMatrixP      pElementRMatrix = NULL  // Allow nonuniform scaling if only in Z of this matrix.
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/04
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt    GetIgnoreScaleDisplayTransforms
(
TransformP      newTopTransP,
TransformP      newScTransP,
ViewContextR    context
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/04
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt AddThickness (EditElementHandleR eeh, double thickness, DVec3dCP direction, bool isCapped, bool alwaysUseDirection);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/04
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt ExtractThickness (double& thickness, DVec3dR direction, bool& isCapped, bool& alwaysUseDirection, ElementHandleCR eh);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/04
+---------------+---------------+---------------+---------------+---------------+------*/
static void ApplyTransformToLinkages (EditElementHandleR eeh, TransformInfoCR transform);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/09
+---------------+---------------+---------------+---------------+---------------+------*/
static void ApplyTransformToXDataLinkage (EditElementHandleR eeh, TransformInfoCR transform);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  12/09
+---------------+---------------+---------------+---------------+---------------+------*/
static void FenceStretchXDataLinkage (EditElementHandleR eeh, TransformInfoCR transform, FenceParamsP fp);

// keys for text style linkages
#define KEY_TEXTSTYLE_DIMSTYLE      0

/*---------------------------------------------------------------------------------**//**
*  
* @param        element     IN OUT  
* @param        textStyle   IN      
* @param        linkageKey  IN      
* @return       
* @bsimethod                                                    JoshSchifter    06/02
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus AppendTextStyleAsLinkage (EditElementHandleR element, LegacyTextStyleCR textStyle, UInt32 linkageKey);

/*---------------------------------------------------------------------------------**//**
*  
* @param        textStyle   OUT 
* @param        element     IN  
* @param        linkageKey  IN  
* @return       
* @bsimethod                                                    JoshSchifter    06/02
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus ExtractTextStyleFromLinkage (LegacyTextStyleR textStyle, ElementHandleCR element, UInt32 linkageKey);

/*---------------------------------------------------------------------------------**//**
*  
* @param        textStyle   OUT 
* @param        element     IN  
* @param        linkageKey  IN  
* @return       
* @bsimethod                                                    JoshSchifter    06/02
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus DeleteTextStyleLinkage (EditElementHandleR eeh, UInt32 linkageKey);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/09
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus ExtractTextShape (ElementHandleCR eh, DPoint3dP shapePts, DPoint3dR userOrigin);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/09
+---------------+---------------+---------------+---------------+---------------+------*/
static void     GetSegment (EditElementHandleR eeh, ElementHandleCR eh, int segmentNo);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/09
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT static BentleyStatus GetIntersections
(
bvector<DPoint3d>*pPoint1,
bvector<DPoint3d>*pPoint2,
ElementHandleCR    eh1,
ElementHandleCR    eh2,
TransformCP     pathTrans1,
TransformCP     pathTrans2,
bool            extendSegment
);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/09
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT static BentleyStatus GetIntersectionPointByIndex
(
DPoint3dR       isPnt,
ElementHandleCR    eh1,
ElementHandleCR    eh2,
TransformCP     pathTrans1,
TransformCP     pathTrans2,
int             index
);

}; // ElementUtil



END_BENTLEY_DGNPLATFORM_NAMESPACE
