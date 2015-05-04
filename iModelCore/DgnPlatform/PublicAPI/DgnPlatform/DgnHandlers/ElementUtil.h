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

END_BENTLEY_DGNPLATFORM_NAMESPACE
