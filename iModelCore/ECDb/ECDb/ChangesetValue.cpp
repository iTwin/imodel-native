/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "ChangesetValue.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//**** ChangesetValue ****

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ChangesetValue::ChangesetValue(ECSqlColumnInfo const& colInfo)
    : IECSqlValue(), m_columnInfo(colInfo)
    {}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool ChangesetValue::_IsNull() const
    { return NoopECSqlValue::GetSingleton().IsNull(); }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void const* ChangesetValue::_GetBlob(int* blobSize) const
    { return NoopECSqlValue::GetSingleton().GetBlob(blobSize); }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool ChangesetValue::_GetBoolean() const
    { return NoopECSqlValue::GetSingleton().GetBoolean(); }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
double ChangesetValue::_GetDateTimeJulianDays(DateTime::Info& metadata) const
    { return NoopECSqlValue::GetSingleton().GetDateTimeJulianDays(metadata); }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
uint64_t ChangesetValue::_GetDateTimeJulianDaysMsec(DateTime::Info& metadata) const
    { return NoopECSqlValue::GetSingleton().GetDateTimeJulianDaysMsec(metadata); }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
double ChangesetValue::_GetDouble() const
    { return NoopECSqlValue::GetSingleton().GetDouble(); }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
int ChangesetValue::_GetInt() const
    { return NoopECSqlValue::GetSingleton().GetInt(); }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
int64_t ChangesetValue::_GetInt64() const
    { return NoopECSqlValue::GetSingleton().GetInt64(); }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Utf8CP ChangesetValue::_GetText() const
    { return NoopECSqlValue::GetSingleton().GetText(); }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DPoint2d ChangesetValue::_GetPoint2d() const
    { return NoopECSqlValue::GetSingleton().GetPoint2d(); }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DPoint3d ChangesetValue::_GetPoint3d() const
    { return NoopECSqlValue::GetSingleton().GetPoint3d(); }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
IGeometryPtr ChangesetValue::_GetGeometry() const
    { return NoopECSqlValue::GetSingleton().GetGeometry(); }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlValue const& ChangesetValue::_GetStructMemberValue(Utf8CP /*memberName*/) const
    { return NoopECSqlValue::GetSingleton(); }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlValueIterable const& ChangesetValue::_GetStructIterable() const
    { return NoopECSqlValue::GetSingleton().GetStructIterable(); }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
int ChangesetValue::_GetArrayLength() const
    { return NoopECSqlValue::GetSingleton().GetArrayLength(); }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlValueIterable const& ChangesetValue::_GetArrayIterable() const
    { return NoopECSqlValue::GetSingleton().GetArrayIterable(); }


//**** ChangesetPrimitiveValue ****

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ChangesetPrimitiveValue::ChangesetPrimitiveValue(ECSqlColumnInfo const& colInfo, DbValue const& value, DateTime::Info const& dtInfo)
    : ChangesetValue(colInfo), m_value(value), m_datetimeInfo(dtInfo)
    {}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool ChangesetPrimitiveValue::_IsNull() const
    { return !m_value.IsValid() || m_value.IsNull(); }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void const* ChangesetPrimitiveValue::_GetBlob(int* blobSize) const
    {
    if (!m_value.IsValid())
        return NoopECSqlValue::GetSingleton().GetBlob(blobSize);
    if (blobSize != nullptr)
        *blobSize = m_value.GetValueBytes();
    return m_value.GetValueBlob();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool ChangesetPrimitiveValue::_GetBoolean() const
    {
    if (!m_value.IsValid())
        return NoopECSqlValue::GetSingleton().GetBoolean();
    return m_value.GetValueInt() != 0;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
double ChangesetPrimitiveValue::_GetDateTimeJulianDays(DateTime::Info& metadata) const
    {
    if (!m_value.IsValid())
        return NoopECSqlValue::GetSingleton().GetDateTimeJulianDays(metadata);
    const double jd = m_value.GetValueDouble();
    metadata = m_datetimeInfo;
    return jd;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
uint64_t ChangesetPrimitiveValue::_GetDateTimeJulianDaysMsec(DateTime::Info& metadata) const
    { return DateTime::RationalDayToMsec(_GetDateTimeJulianDays(metadata)); }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
double ChangesetPrimitiveValue::_GetDouble() const
    { return m_value.IsValid() ? m_value.GetValueDouble() : NoopECSqlValue::GetSingleton().GetDouble(); }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
int ChangesetPrimitiveValue::_GetInt() const
    { return m_value.IsValid() ? m_value.GetValueInt() : NoopECSqlValue::GetSingleton().GetInt(); }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
int64_t ChangesetPrimitiveValue::_GetInt64() const
    { return m_value.IsValid() ? m_value.GetValueInt64() : NoopECSqlValue::GetSingleton().GetInt64(); }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Utf8CP ChangesetPrimitiveValue::_GetText() const
    { return m_value.IsValid() ? m_value.GetValueText() : NoopECSqlValue::GetSingleton().GetText(); }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
IGeometryPtr ChangesetPrimitiveValue::_GetGeometry() const
    {
    int blobSize = -1;
    void const* blob = _GetBlob(&blobSize);
    return IECSqlValueHelper::GeometryFromBlob(blob, blobSize);
    }


//**** ChangesetFixedInt64Value ****

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ChangesetFixedInt64Value::ChangesetFixedInt64Value(ECSqlColumnInfo const& colInfo, BeInt64Id const& id)
    : ChangesetValue(colInfo), m_id(id)
    {}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Utf8CP ChangesetFixedInt64Value::_GetText() const
    {
    if (m_idStr.empty())
        m_idStr = m_id.ToHexStr();
    return m_idStr.c_str();
    }


//**** ChangesetPoint2dValue ****

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ChangesetPoint2dValue::ChangesetPoint2dValue(ECSqlColumnInfo const& colInfo, double x, double y)
    : ChangesetValue(colInfo)
    {
    m_isNull = IECSqlValueHelper::IsNullCoord(x) || IECSqlValueHelper::IsNullCoord(y);
    if (m_isNull)
        m_point = NoopECSqlValue::GetSingleton().GetPoint2d();
    else
        m_point = DPoint2d::From(x, y);
    }


//**** ChangesetPoint3dValue ****

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ChangesetPoint3dValue::ChangesetPoint3dValue(ECSqlColumnInfo const& colInfo, double x, double y, double z)
    : ChangesetValue(colInfo)
    {
    m_isNull = IECSqlValueHelper::IsNullCoord(x) || IECSqlValueHelper::IsNullCoord(y) || IECSqlValueHelper::IsNullCoord(z);
    if (m_isNull)
        m_point = NoopECSqlValue::GetSingleton().GetPoint3d();
    else
        m_point = DPoint3d::From(x, y, z);
    }


//**** ChangesetArrayValue ****

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ChangesetArrayValue::ChangesetArrayValue(ECSqlColumnInfo const& colInfo, DbValue const& value, ECDbCR ecdb)
    : ChangesetValue(colInfo)
    {
    if (!value.IsValid() || value.IsNull())
        m_json.SetArray();
    else
        {
        Utf8CP jsonStr = value.GetValueText();
        if (jsonStr == nullptr || m_json.Parse<0>(jsonStr).HasParseError())
            {
            LOG.errorv("ChangesetArrayValue: failed to parse JSON column value.");
            m_json.SetArray();
            }
        }
    m_jsonValue = std::make_unique<JsonECSqlValue>(ecdb, m_json, _GetColumnInfo());
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool ChangesetArrayValue::_IsNull() const
    { return m_jsonValue->IsNull(); }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void const* ChangesetArrayValue::_GetBlob(int* blobSize) const
    { return m_jsonValue->GetBlob(blobSize); }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool ChangesetArrayValue::_GetBoolean() const
    { return m_jsonValue->GetBoolean(); }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
double ChangesetArrayValue::_GetDateTimeJulianDays(DateTime::Info& metadata) const
    { return m_jsonValue->GetDateTimeJulianDays(metadata); }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
uint64_t ChangesetArrayValue::_GetDateTimeJulianDaysMsec(DateTime::Info& metadata) const
    { return m_jsonValue->GetDateTimeJulianDaysMsec(metadata); }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
double ChangesetArrayValue::_GetDouble() const
    { return m_jsonValue->GetDouble(); }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
int ChangesetArrayValue::_GetInt() const
    { return m_jsonValue->GetInt(); }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
int64_t ChangesetArrayValue::_GetInt64() const
    { return m_jsonValue->GetInt64(); }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Utf8CP ChangesetArrayValue::_GetText() const
    { return m_jsonValue->GetText(); }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DPoint2d ChangesetArrayValue::_GetPoint2d() const
    { return m_jsonValue->GetPoint2d(); }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DPoint3d ChangesetArrayValue::_GetPoint3d() const
    { return m_jsonValue->GetPoint3d(); }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
IGeometryPtr ChangesetArrayValue::_GetGeometry() const
    { return m_jsonValue->GetGeometry(); }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlValue const& ChangesetArrayValue::_GetStructMemberValue(Utf8CP memberName) const
    { return (*m_jsonValue)[memberName]; }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlValueIterable const& ChangesetArrayValue::_GetStructIterable() const
    { return m_jsonValue->GetStructIterable(); }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
int ChangesetArrayValue::_GetArrayLength() const
    { return m_jsonValue->GetArrayLength(); }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlValueIterable const& ChangesetArrayValue::_GetArrayIterable() const
    { return m_jsonValue->GetArrayIterable(); }


//**** ChangesetStructValue ****
//+---------------+---------------+---------------+---------------+---------------+------
ChangesetStructValue::ChangesetStructValue(ECSqlColumnInfo const& colInfo, std::vector<ChangesetValue*> const& members)
    : ChangesetValue(colInfo)
    {
        for (auto* member : members)
            m_members.push_back(*member);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool ChangesetStructValue::_IsNull() const
    {
    for (ChangesetValue const& member : m_members)
        if (!member.IsNull())
            return false;
    return true;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlValue const& ChangesetStructValue::_GetStructMemberValue(Utf8CP memberName) const
    {
    for (size_t i = 0; i < m_members.size(); ++i)
        {
        if (m_members[i].get().GetColumnInfo().GetProperty()->GetName().EqualsIAscii(memberName))
            return m_members[i].get();
        }
    return NoopECSqlValue::GetSingleton();
    }


//**** ChangesetNavValue ****

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ChangesetNavValue::ChangesetNavValue(ECSqlColumnInfo const& colInfo, ChangesetValue const& id, ChangesetValue const& relClassId)
    : ChangesetValue(colInfo), IECSqlValueIterable(),
      m_id(id), m_relClassId(relClassId)
    {}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlValue const& ChangesetNavValue::_GetStructMemberValue(Utf8CP memberName) const
    {
    return IECSqlValueHelper::GetNavMemberValue(memberName, &m_id, &m_relClassId);
    }


//**** ChangesetNavValue::IteratorState ****

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlValue const& ChangesetNavValue::IteratorState::_GetCurrent() const
    {
    return IECSqlValueHelper::GetNavIterCurrentByStateIndex((uint8_t) m_state, &m_owner.m_id, &m_owner.m_relClassId);
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
