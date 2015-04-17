/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnHandlers/ElementUtil.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

#include <DgnPlatform/DgnPlatform.h>

BEGIN_BENTLEY_API_NAMESPACE

DGNPLATFORM_EXPORT void pattern_setOverrideCell (DgnElementCP cellEdP);
DGNPLATFORM_EXPORT DgnElementCP pattern_getOverrideCell ();


DGNPLATFORM_EXPORT bool      in_span
(
double          theta,
double          start,
double          sweep
);


END_BENTLEY_API_NAMESPACE

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//=======================================================================================
//! These methods append and extract basic data types from byte streams,
//! advancing the provided pointer when done.
//! @bsiclass                                                     Jeff.Marker     01/09
//=======================================================================================
struct ByteStreamHelper
{
DGNPLATFORM_EXPORT static void AppendRotMatrix (Byte*& buffer, RotMatrixCR value, bool is3d);
DGNPLATFORM_EXPORT static void AppendDPoint3d (Byte*& buffer, DPoint3dCR value);
DGNPLATFORM_EXPORT static void AppendDouble (Byte*& buffer, double const & value);
DGNPLATFORM_EXPORT static void AppendLong (Byte*& buffer, long const & value);
DGNPLATFORM_EXPORT static void AppendShort (Byte*& buffer, short const & value);
DGNPLATFORM_EXPORT static void AppendInt64 (Byte*& buffer, int64_t const & value);
DGNPLATFORM_EXPORT static void AppendUInt32 (Byte*& buffer, uint32_t const & value);
DGNPLATFORM_EXPORT static void AppendInt (Byte*& buffer, int const & value);
DGNPLATFORM_EXPORT static void AppendUInt16 (Byte*& buffer, uint16_t const & value);
DGNPLATFORM_EXPORT static void AppendInt32 (Byte*& buffer, int32_t const & value);
DGNPLATFORM_EXPORT static void AppendUShort (Byte*& buffer, unsigned short const & value);
DGNPLATFORM_EXPORT static void AppendULong (Byte*& buffer, unsigned long const & value);
DGNPLATFORM_EXPORT static void ExtractRotMatrix (RotMatrixR value, Byte const *& buffer, bool is3d);
DGNPLATFORM_EXPORT static void ExtractDPoint3d (DPoint3dR value, Byte const *& buffer);
DGNPLATFORM_EXPORT static void ExtractDouble (double& value, Byte const *& buffer);
DGNPLATFORM_EXPORT static void ExtractLong (long& value, Byte const *& buffer);
DGNPLATFORM_EXPORT static void ExtractShort (short& value, Byte const *& buffer);
DGNPLATFORM_EXPORT static void ExtractInt64 (int64_t& value, Byte const *& buffer);
DGNPLATFORM_EXPORT static void ExtractUInt32 (uint32_t& value, Byte const *& buffer);
DGNPLATFORM_EXPORT static void ExtractInt (int& value, Byte const *& buffer);
DGNPLATFORM_EXPORT static void ExtractUInt16 (uint16_t& value, Byte const *& buffer);
DGNPLATFORM_EXPORT static void ExtractInt32 (int32_t& value, Byte const *& buffer);
DGNPLATFORM_EXPORT static void ExtractUShort (unsigned short& value, Byte const *& buffer);
DGNPLATFORM_EXPORT static void ExtractULong (unsigned long& value, Byte const *& buffer);
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
DgnViewportP           vp
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
DgnViewportP           vp
);

}; // CurveVectorUtil

/*=================================================================================**//**
*
* @bsiclass                                                     Sam.Wilson      10/2008
+===============+===============+===============+===============+===============+======*/
struct          ElementUtil
{
static void InitScanRangeForUnion (DRange3dR, bool is3d);

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

// keys for text style linkages
#define KEY_TEXTSTYLE_DIMSTYLE      0

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/09
+---------------+---------------+---------------+---------------+---------------+------*/
static void     GetSegment (EditElementHandleR eeh, ElementHandleCR eh, int segmentNo);

}; // ElementUtil



END_BENTLEY_DGNPLATFORM_NAMESPACE
