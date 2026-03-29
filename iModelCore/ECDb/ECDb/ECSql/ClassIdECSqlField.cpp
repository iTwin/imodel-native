/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

bool ClassIdECSqlField::_IsNull() const { return !m_classId.IsValid(); }
void const* ClassIdECSqlField::_GetBlob(int* blobSize) const { return NoopECSqlValue::GetSingleton().GetBlob(blobSize); }
bool ClassIdECSqlField::_GetBoolean() const { return NoopECSqlValue::GetSingleton().GetBoolean(); }
double ClassIdECSqlField::_GetDateTimeJulianDays(DateTime::Info& metadata) const { return NoopECSqlValue::GetSingleton().GetDateTimeJulianDays(metadata); }
uint64_t ClassIdECSqlField::_GetDateTimeJulianDaysMsec(DateTime::Info& metadata) const { return NoopECSqlValue::GetSingleton().GetDateTimeJulianDaysMsec(metadata); }
double ClassIdECSqlField::_GetDouble() const { return NoopECSqlValue::GetSingleton().GetDouble(); }
int ClassIdECSqlField::_GetInt() const { return NoopECSqlValue::GetSingleton().GetInt(); }
IGeometryPtr ClassIdECSqlField::_GetGeometry() const { return NoopECSqlValue::GetSingleton().GetGeometry(); }
DPoint2d ClassIdECSqlField::_GetPoint2d() const { return NoopECSqlValue::GetSingleton().GetPoint2d(); }
DPoint3d ClassIdECSqlField::_GetPoint3d() const { return NoopECSqlValue::GetSingleton().GetPoint3d(); }
IECSqlValue const& ClassIdECSqlField::_GetStructMemberValue(Utf8CP structMemberName) const { return NoopECSqlValue::GetSingleton(); }
IECSqlValueIterable const& ClassIdECSqlField::_GetStructIterable() const { return NoopECSqlValue::GetSingleton().GetStructIterable(); }
int ClassIdECSqlField::_GetArrayLength() const { return NoopECSqlValue::GetSingleton().GetArrayLength(); }
IECSqlValueIterable const& ClassIdECSqlField::_GetArrayIterable() const { return NoopECSqlValue::GetSingleton().GetArrayIterable(); }

Utf8CP ClassIdECSqlField::_GetText() const {
    if (m_idStr.empty())
        m_idStr = m_classId.ToHexStr();
    return m_idStr.c_str();
}

int64_t ClassIdECSqlField::_GetInt64() const {
    return static_cast<int64_t>(m_classId.GetValueUnchecked());
}

END_BENTLEY_SQLITE_EC_NAMESPACE
