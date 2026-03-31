/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "ChangesetValue.h"
#include <GeomSerialization/GeomSerializationApi.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//**** ChangesetValueBase ****

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ChangesetValueBase::ChangesetValueBase(ECSqlColumnInfo colInfo)
    : IECSqlValue(), m_columnInfo(std::move(colInfo))
    {}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool ChangesetValueBase::_IsNull() const
    { return NoopECSqlValue::GetSingleton().IsNull(); }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void const* ChangesetValueBase::_GetBlob(int* blobSize) const
    { return NoopECSqlValue::GetSingleton().GetBlob(blobSize); }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool ChangesetValueBase::_GetBoolean() const
    { return NoopECSqlValue::GetSingleton().GetBoolean(); }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
double ChangesetValueBase::_GetDateTimeJulianDays(DateTime::Info& metadata) const
    { return NoopECSqlValue::GetSingleton().GetDateTimeJulianDays(metadata); }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
uint64_t ChangesetValueBase::_GetDateTimeJulianDaysMsec(DateTime::Info& metadata) const
    { return NoopECSqlValue::GetSingleton().GetDateTimeJulianDaysMsec(metadata); }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
double ChangesetValueBase::_GetDouble() const
    { return NoopECSqlValue::GetSingleton().GetDouble(); }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
int ChangesetValueBase::_GetInt() const
    { return NoopECSqlValue::GetSingleton().GetInt(); }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
int64_t ChangesetValueBase::_GetInt64() const
    { return NoopECSqlValue::GetSingleton().GetInt64(); }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Utf8CP ChangesetValueBase::_GetText() const
    { return NoopECSqlValue::GetSingleton().GetText(); }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DPoint2d ChangesetValueBase::_GetPoint2d() const
    { return NoopECSqlValue::GetSingleton().GetPoint2d(); }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DPoint3d ChangesetValueBase::_GetPoint3d() const
    { return NoopECSqlValue::GetSingleton().GetPoint3d(); }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
IGeometryPtr ChangesetValueBase::_GetGeometry() const
    { return NoopECSqlValue::GetSingleton().GetGeometry(); }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlValue const& ChangesetValueBase::_GetStructMemberValue(Utf8CP /*memberName*/) const
    { return NoopECSqlValue::GetSingleton(); }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlValueIterable const& ChangesetValueBase::_GetStructIterable() const
    { return NoopECSqlValue::GetSingleton().GetStructIterable(); }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
int ChangesetValueBase::_GetArrayLength() const
    { return NoopECSqlValue::GetSingleton().GetArrayLength(); }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlValueIterable const& ChangesetValueBase::_GetArrayIterable() const
    { return NoopECSqlValue::GetSingleton().GetArrayIterable(); }


//**** ChangesetPrimitiveValue ****

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ChangesetPrimitiveValue::ChangesetPrimitiveValue(ECSqlColumnInfo colInfo, DbValue const& value, DateTime::Info const& dtInfo)
    : ChangesetValueBase(std::move(colInfo)), m_value(value), m_datetimeInfo(dtInfo)
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
    Byte const* blob = static_cast<Byte const*>(_GetBlob(&blobSize));
    if (blob == nullptr)
        return nullptr;
    BeAssert(blobSize > 0);
    const size_t blobSizeU = (size_t)blobSize;
    bvector<Byte> byteVec;
    byteVec.reserve(blobSizeU);
    byteVec.assign(blob, blob + blobSizeU);
    if (!BentleyGeometryFlatBuffer::IsFlatBufferFormat(byteVec))
        {
        LOG.error("Cannot retrieve Geometry value. The geometry is not persisted in the Bentley Geometry FlatBuffer format.");
        return nullptr;
        }
    IGeometryPtr geom = BentleyGeometryFlatBuffer::BytesToGeometry(byteVec);
    BeAssert(geom != nullptr);
    return geom;
    }


//**** ChangesetFixedInt64Value ****

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ChangesetFixedInt64Value::ChangesetFixedInt64Value(ECSqlColumnInfo colInfo, BeInt64Id id)
    : ChangesetValueBase(std::move(colInfo)), m_id(id)
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
ChangesetPoint2dValue::ChangesetPoint2dValue(ECSqlColumnInfo colInfo, DbValue const& x, DbValue const& y)
    : ChangesetValueBase(std::move(colInfo))
    {
    auto isCoordNull = [](DbValue const& v) -> bool
        {
        if (!v.IsValid() || v.IsNull()) return true;
        const double d = v.GetValueDouble();
        return std::isinf(d) || std::isnan(d);
        };
    m_isNull = isCoordNull(x) || isCoordNull(y);
    if (m_isNull)
        m_point = NoopECSqlValue::GetSingleton().GetPoint2d();
    else
        m_point = DPoint2d::From(x.GetValueDouble(), y.GetValueDouble());
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ChangesetPoint2dValue::ChangesetPoint2dValue(ECSqlColumnInfo colInfo, double x, double y)
    : ChangesetValueBase(std::move(colInfo))
    {
    m_isNull = std::isinf(x) || std::isnan(x) || std::isinf(y) || std::isnan(y);
    if (m_isNull)
        m_point = NoopECSqlValue::GetSingleton().GetPoint2d();
    else
        m_point = DPoint2d::From(x, y);
    }


//**** ChangesetPoint3dValue ****

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ChangesetPoint3dValue::ChangesetPoint3dValue(ECSqlColumnInfo colInfo, DbValue const& x, DbValue const& y, DbValue const& z)
    : ChangesetValueBase(std::move(colInfo))
    {
    auto isCoordNull = [](DbValue const& v) -> bool
        {
        if (!v.IsValid() || v.IsNull()) return true;
        const double d = v.GetValueDouble();
        return std::isinf(d) || std::isnan(d);
        };
    m_isNull = isCoordNull(x) || isCoordNull(y) || isCoordNull(z);
    if (m_isNull)
        m_point = NoopECSqlValue::GetSingleton().GetPoint3d();
    else
        m_point = DPoint3d::From(x.GetValueDouble(), y.GetValueDouble(), z.GetValueDouble());
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ChangesetPoint3dValue::ChangesetPoint3dValue(ECSqlColumnInfo colInfo, double x, double y, double z)
    : ChangesetValueBase(std::move(colInfo))
    {
    m_isNull = std::isinf(x) || std::isnan(x) || std::isinf(y) || std::isnan(y) || std::isinf(z) || std::isnan(z);
    if (m_isNull)
        m_point = NoopECSqlValue::GetSingleton().GetPoint3d();
    else
        m_point = DPoint3d::From(x, y, z);
    }


//**** ChangesetArrayValue ****

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ChangesetArrayValue::ChangesetArrayValue(ECSqlColumnInfo colInfo, DbValue const& value, ECDbCR ecdb)
    : ChangesetValueBase(std::move(colInfo))
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
    m_jsonValue = std::make_unique<ArrayECSqlField::JsonECSqlValue>(ecdb, m_json, _GetColumnInfo());
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
ChangesetStructValue::ChangesetStructValue(ECSqlColumnInfo colInfo)
    : ChangesetValueBase(std::move(colInfo)), IECSqlValueIterable()
    {}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void ChangesetStructValue::AppendMember(Utf8StringCR name, std::unique_ptr<IECSqlValue> member)
    {
    m_names.push_back(name);
    m_members.push_back(std::move(member));
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool ChangesetStructValue::_IsNull() const
    {
    for (auto const& member : m_members)
        if (!member->IsNull())
            return false;
    return true;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlValue const& ChangesetStructValue::_GetStructMemberValue(Utf8CP memberName) const
    {
    for (size_t i = 0; i < m_names.size(); ++i)
        {
        if (m_names[i].EqualsIAscii(memberName))
            return *m_members[i];
        }
    return NoopECSqlValue::GetSingleton();
    }


//**** ChangesetNavValue ****

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ChangesetNavValue::ChangesetNavValue(ECSqlColumnInfo colInfo, std::unique_ptr<IECSqlValue> id, std::unique_ptr<IECSqlValue> relClassId)
    : ChangesetValueBase(std::move(colInfo)), IECSqlValueIterable(),
      m_id(std::move(id)), m_relClassId(std::move(relClassId))
    {}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlValue const& ChangesetNavValue::_GetStructMemberValue(Utf8CP memberName) const
    {
    if (BeStringUtilities::StricmpAscii(memberName, ECDBSYS_PROP_NavPropId) == 0)
        return m_id != nullptr ? *m_id : (IECSqlValue const&)NoopECSqlValue::GetSingleton();
    if (BeStringUtilities::StricmpAscii(memberName, ECDBSYS_PROP_NavPropRelECClassId) == 0)
        return m_relClassId != nullptr ? *m_relClassId : (IECSqlValue const&)NoopECSqlValue::GetSingleton();
    return NoopECSqlValue::GetSingleton();
    }


//**** ChangesetNavValue::IteratorState ****

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlValue const& ChangesetNavValue::IteratorState::_GetCurrent() const
    {
    if (m_state == State::Id)
        return m_owner.m_id != nullptr ? *m_owner.m_id : (IECSqlValue const&)NoopECSqlValue::GetSingleton();
    if (m_state == State::RelClassId)
        return m_owner.m_relClassId != nullptr ? *m_owner.m_relClassId : (IECSqlValue const&)NoopECSqlValue::GetSingleton();
    return NoopECSqlValue::GetSingleton();
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
