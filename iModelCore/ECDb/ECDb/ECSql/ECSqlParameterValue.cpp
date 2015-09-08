/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECSqlParameterValue.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "ECSqlParameterValue.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

// ************************************************
// ECSqlParameterValueFactory
// ************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
//static
std::unique_ptr<ECSqlParameterValue> ECSqlParameterValueFactory::Create(ECSqlStatusContext& statusContext, ECSqlTypeInfo const& typeInfo)
    {
    if (typeInfo.IsPrimitive())
        return CreatePrimitive(statusContext, typeInfo);
    else if (typeInfo.IsStruct())
        return CreateStruct(statusContext, typeInfo);
    else
        {
        BeAssert(typeInfo.IsArray());
        return std::unique_ptr<ECSqlParameterValue> (new ArrayECSqlParameterValue(statusContext, typeInfo));
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
//static
std::unique_ptr<ECSqlParameterValue> ECSqlParameterValueFactory::CreateStruct(ECSqlStatusContext& statusContext, ECSqlTypeInfo const& typeInfo)
    {
    return std::unique_ptr<ECSqlParameterValue> (new StructECSqlParameterValue(statusContext, typeInfo));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
//static
std::unique_ptr<ECSqlParameterValue> ECSqlParameterValueFactory::CreatePrimitive(ECSqlStatusContext& statusContext, ECSqlTypeInfo const& typeInfo)
    {
    return std::unique_ptr<ECSqlParameterValue> (new PrimitiveECSqlParameterValue(statusContext, typeInfo));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      04/2014
//---------------------------------------------------------------------------------------
//static
std::unique_ptr<ArrayECSqlParameterValue> ECSqlParameterValueFactory::CreateArray(ECSqlStatusContext& statusContext, ECSqlTypeInfo const& typeInfo)
    {
    return std::unique_ptr<ArrayECSqlParameterValue> (new ArrayECSqlParameterValue(statusContext, typeInfo));
    }

// ************************************************
// ECSqlParameterValue
// ************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
ECSqlParameterValue::ECSqlParameterValue(ECSqlStatusContext& statusContext, ECSqlTypeInfo const& typeInfo) 
: IECSqlBinder(), IECSqlValue(), m_typeInfo(typeInfo), m_statusContext(statusContext)
    {}

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
IECSqlPrimitiveBinder& ECSqlParameterValue::_BindPrimitive()
    {
    return BindPrimitive();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2014
//---------------------------------------------------------------------------------------
IECSqlPrimitiveBinder& ECSqlParameterValue::BindPrimitive() const
    {
    auto stat = m_statusContext.SetError(ECSqlStatus::UserError, "Type mismatch. Cannot bind primitive value to non-primitive parameter.");
    return NoopECSqlBinderFactory::GetBinder(stat).BindPrimitive();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
IECSqlStructBinder& ECSqlParameterValue::_BindStruct()
    {
    auto stat = m_statusContext.SetError(ECSqlStatus::UserError, "Type mismatch. Cannot bind struct value to non-struct parameter.");
    return NoopECSqlBinderFactory::GetBinder(stat).BindStruct();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
IECSqlArrayBinder& ECSqlParameterValue::_BindArray(uint32_t initialCapacity)
    {
    auto stat = m_statusContext.SetError(ECSqlStatus::UserError, "Type mismatch. Cannot bind array value to non-array parameter.");
    return NoopECSqlBinderFactory::GetBinder(stat).BindArray(initialCapacity);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
IECSqlPrimitiveValue const& ECSqlParameterValue::_GetPrimitive() const
    {
    BeAssert(false && "Cannot call _GetPrimitive on non primitive parameter value.");
    return NoopECSqlValue::GetSingleton().GetPrimitive();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
IECSqlStructValue const& ECSqlParameterValue::_GetStruct() const
    {
    BeAssert(false && "Cannot call _GetStruct on parameter value which is no struct.");
    return NoopECSqlValue::GetSingleton().GetStruct();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
IECSqlArrayValue const& ECSqlParameterValue::_GetArray() const
    {
    BeAssert(false && "Cannot call _GetArray on non-aray parameter value.");
    return NoopECSqlValue::GetSingleton().GetArray();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
ECSqlColumnInfoCR ECSqlParameterValue::_GetColumnInfo() const
    {
    BeAssert(false && "Cannot call GetColumnInfo on ECSqlParameterValue. Call GetTypeInfo instead.");
    return NoopECSqlValue::GetSingleton().GetColumnInfo();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2014
//---------------------------------------------------------------------------------------
void ECSqlParameterValue::Clear()
    {
    _Clear();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlParameterValue::ResetStatus() const
    {
    return m_statusContext.Reset();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      04/2014
//---------------------------------------------------------------------------------------
ECSqlStatus ECSqlParameterValue::BindTo(IECSqlBinder& targetBinder) const
    {
    return BindTo(GetStatusContext(), *this, targetBinder);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      04/2014
//---------------------------------------------------------------------------------------
//static
ECSqlStatus ECSqlParameterValue::BindTo(ECSqlStatusContext& statusContext, ECSqlParameterValue const& from, IECSqlBinder& to)
    {
    if (from.IsNull())
        return to.BindNull();

    auto const& typeInfo = from.GetTypeInfo();
    if (typeInfo.IsPrimitive())
        {
        auto primitiveType = typeInfo.GetPrimitiveType();
        switch (primitiveType)
            {
                case PRIMITIVETYPE_Binary:
                    {
                    int blobSize = -1;
                    auto blob = from.GetBinary(&blobSize);
                    //Don't make copy: parameter value (from) is expected to live as long as the target binder
                    return to.BindBinary(blob, blobSize, IECSqlBinder::MakeCopy::No);
                    }

                case PRIMITIVETYPE_Boolean:
                    return to.BindBoolean(from.GetBoolean());

                case PRIMITIVETYPE_DateTime:
                    {
                    //Use Julian Days to avoid to unnecessary computation of DateTime from JD
                    //and back from DateTime to JD again
                    DateTime::Info metadata;
                    uint64_t jdHns = from.GetDateTimeJulianDays(metadata);
                    return to.BindDateTime(jdHns, &metadata);
                    }

                case PRIMITIVETYPE_Double:
                    return to.BindDouble(from.GetDouble());

                case PRIMITIVETYPE_Integer:
                    return to.BindInt(from.GetInt());

                case PRIMITIVETYPE_Long:
                    return to.BindInt64(from.GetInt64());

                case PRIMITIVETYPE_Point2D:
                    return to.BindPoint2D (from.GetPoint2D ());

                case PRIMITIVETYPE_Point3D:
                    return to.BindPoint3D (from.GetPoint3D ());

                case PRIMITIVETYPE_String:
                    //Don't make copy: parameter value (from) is expected to live as long as the target binder
                    return to.BindText(from.GetText(), IECSqlBinder::MakeCopy::No);

                default:
                    {
                    auto stat = statusContext.SetError(ECSqlStatus::ProgrammerError, "ECSqlParameterValue::BindTo failed. Unhandled primitive type.");
                    BeAssert(false && "Unhandled primitive type in ECSqlParameterValue::BindTo");
                    return stat;
                    }
            }
        }
    else if (typeInfo.IsStruct())
        {
        ECClassCR structType = typeInfo.GetStructType();
        BeAssert(dynamic_cast<StructECSqlParameterValue const*> (&from.GetStruct()) != nullptr);
        auto fromStructParamValue = static_cast<StructECSqlParameterValue const*> (&from.GetStruct());
        auto& toStructBinder = to.BindStruct();

        for (ECPropertyCP structMemberProp : structType.GetProperties(true))
            {
            ECN::ECPropertyId memberPropId = structMemberProp->GetId();
            auto const& memberValue = fromStructParamValue->GetValue(memberPropId);
            BeAssert(dynamic_cast<ECSqlParameterValue const*> (&memberValue) != nullptr);
            auto const& memberValueAsParameterValue = static_cast<ECSqlParameterValue const&> (memberValue);
            auto stat = BindTo(statusContext, memberValueAsParameterValue, toStructBinder.GetMember(memberPropId));
            if (stat != ECSqlStatus::Success)
                return stat;
            }
        }
    else
        {
        BeAssert(typeInfo.IsArray());
        auto const& fromArrayValue = from.GetArray();
        auto& toArrayBinder = to.BindArray(fromArrayValue.GetArrayLength()); //GetArrayLength is fast as fromArrayValue is always an ECSqlParameterValue
        for (IECSqlValue const* fromArrayElement : fromArrayValue)
            {
            BeAssert(dynamic_cast<ECSqlParameterValue const*> (fromArrayElement) != nullptr);
            auto fromArrayValueAsParameterValue = static_cast<ECSqlParameterValue const*> (fromArrayElement);
            auto& arrayElementBinder = toArrayBinder.AddArrayElement();
            auto stat = BindTo(statusContext, *fromArrayValueAsParameterValue, arrayElementBinder);
            if (stat != ECSqlStatus::Success)
                return stat;
            }
        }

    return ECSqlStatus::Success;
    }

// ************************************************
// PrimitiveECSqlParameterValue
// ************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
PrimitiveECSqlParameterValue::PrimitiveECSqlParameterValue(ECSqlStatusContext& statusContext, ECSqlTypeInfo const& typeInfo)
: ECSqlParameterValue(statusContext, typeInfo), IECSqlPrimitiveBinder(), IECSqlPrimitiveValue()
    {}

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
ECSqlStatus PrimitiveECSqlParameterValue::_BindNull()
    {
    DoClear();
    return ResetStatus();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
ECSqlStatus PrimitiveECSqlParameterValue::_BindBoolean(bool value)
    {
    if (!CanBindValue(PRIMITIVETYPE_Boolean))
        return GetStatusContext().SetError(ECSqlStatus::UserError, "Type mismatch. Boolean value can only be bound to boolean parameter values.");

    const auto ecstat = m_value.SetBoolean(value);
    const auto stat = ToECSqlStatus(ecstat);
    if (stat != ECSqlStatus::Success)
        return GetStatusContext().SetError(stat, "BindBoolean");
    
    return ResetStatus();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
ECSqlStatus PrimitiveECSqlParameterValue::_BindBinary(const void* value, int binarySize, IECSqlBinder::MakeCopy makeCopy)
    {
    if (!CanBindValue(PRIMITIVETYPE_Binary))
        return GetStatusContext().SetError(ECSqlStatus::UserError, "Type mismatch. Blob values can only be bound to Blob parameter values.");

    auto ecstat = m_value.SetBinary((const Byte*) value, (size_t) binarySize, makeCopy == MakeCopy::Yes);
    const auto stat = ToECSqlStatus(ecstat);
    if (stat != ECSqlStatus::Success)
        return GetStatusContext().SetError(stat, "BindBinary");

    return ResetStatus();
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
ECSqlStatus PrimitiveECSqlParameterValue::_BindDateTime(double julianDay, DateTime::Info const* metadata)
    {
    const uint64_t jdHns = DateTime::RationalDayToHns(julianDay);
    return _BindDateTime(jdHns, metadata);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
ECSqlStatus PrimitiveECSqlParameterValue::_BindDateTime(uint64_t julianDayHns, DateTime::Info const* metadata)
    {
    if (!CanBindValue(PRIMITIVETYPE_DateTime))
        return GetStatusContext().SetError(ECSqlStatus::UserError, "Type mismatch. Date time values can only be bound to date time parameter values.");

    const int64_t ceTicks = DateTime::JulianDayToCommonEraTicks(julianDayHns);
    const auto ecstat = metadata != nullptr ? m_value.SetDateTimeTicks(ceTicks, *metadata) : m_value.SetDateTimeTicks(ceTicks);
    const auto stat = ToECSqlStatus(ecstat);
    if (stat != ECSqlStatus::Success)
        return GetStatusContext().SetError(stat, "BindDateTime");

    return ResetStatus();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
ECSqlStatus PrimitiveECSqlParameterValue::_BindDouble(double value)
    {
    if (!CanBindValue(PRIMITIVETYPE_Double))
        return GetStatusContext().SetError(ECSqlStatus::UserError, "Type mismatch. Double values can only be bound to double parameter values.");
    
    auto ecstat = m_value.SetDouble(value);
    const auto stat = ToECSqlStatus(ecstat);
    if (stat != ECSqlStatus::Success)
        return GetStatusContext().SetError(stat, "BindDouble");

    return ResetStatus();
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      11/2014
//---------------------------------------------------------------------------------------
ECSqlStatus PrimitiveECSqlParameterValue::_BindGeometryBlob(const void* value, int blobSize, IECSqlBinder::MakeCopy makeCopy)
    {
    if (!CanBindValue(PRIMITIVETYPE_IGeometry))
        return GetStatusContext().SetError(ECSqlStatus::UserError, "Type mismatch. IGeometry values can only be bound to IGeometry parameter values.");

    auto ecstat = m_value.SetBinary(static_cast<Byte const*>(value), blobSize, makeCopy == IECSqlBinder::MakeCopy::Yes);
    const auto stat = ToECSqlStatus(ecstat);
    if (stat != ECSqlStatus::Success)
        return GetStatusContext().SetError(stat, "BindGeometry");

    return ResetStatus();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
ECSqlStatus PrimitiveECSqlParameterValue::_BindInt(int value)
    {
    if (!CanBindValue(PRIMITIVETYPE_Integer))
        return GetStatusContext().SetError(ECSqlStatus::UserError, "Type mismatch. Integer values can only be bound to integer parameter values.");

    auto ecstat = m_value.SetInteger(value);
    const auto stat = ToECSqlStatus(ecstat);
    if (stat != ECSqlStatus::Success)
        return GetStatusContext().SetError(stat, "BindInt");

    return ResetStatus();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
ECSqlStatus PrimitiveECSqlParameterValue::_BindInt64(int64_t value)
    {
    if (!CanBindValue(PRIMITIVETYPE_Long))
        return GetStatusContext().SetError(ECSqlStatus::UserError, "Type mismatch. int64_t values can only be bound to int64_t parameter values.");

    auto ecstat = m_value.SetLong(value);
    const auto stat = ToECSqlStatus(ecstat);
    if (stat != ECSqlStatus::Success)
        return GetStatusContext().SetError(stat, "BindInt64");

    return ResetStatus();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
ECSqlStatus PrimitiveECSqlParameterValue::_BindPoint2D (DPoint2dCR value)
    {
    if (!CanBindValue(PRIMITIVETYPE_Point2D))
        return GetStatusContext().SetError(ECSqlStatus::UserError, "Type mismatch. Point2D values can only be bound to Point2D parameter values.");

    auto ecstat = m_value.SetPoint2D (value);
    const auto stat = ToECSqlStatus(ecstat);
    if (stat != ECSqlStatus::Success)
        return GetStatusContext().SetError(stat, "BindPoint2D");

    return ResetStatus();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
ECSqlStatus PrimitiveECSqlParameterValue::_BindPoint3D (DPoint3dCR value)
    {
    if (!CanBindValue(PRIMITIVETYPE_Point3D))
        return GetStatusContext().SetError(ECSqlStatus::UserError, "Type mismatch. Point3D values can only be bound to Point3D parameter values.");

    auto ecstat = m_value.SetPoint3D (value);
    const auto stat = ToECSqlStatus(ecstat);
    if (stat != ECSqlStatus::Success)
        return GetStatusContext().SetError(stat, "BindPoint3D");

    return ResetStatus();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
ECSqlStatus PrimitiveECSqlParameterValue::_BindText(Utf8CP value, IECSqlBinder::MakeCopy makeCopy, int byteCount)
    {
    if (!CanBindValue(PRIMITIVETYPE_String))
        return GetStatusContext().SetError(ECSqlStatus::UserError, "Type mismatch. String values can only be bound to string parameter values.");

    auto ecstat = m_value.SetUtf8CP (value, makeCopy == MakeCopy::Yes);
    const auto stat = ToECSqlStatus(ecstat);
    if (stat != ECSqlStatus::Success)
        return GetStatusContext().SetError(stat, "BindText");

    return ResetStatus();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
IECSqlPrimitiveBinder& PrimitiveECSqlParameterValue::_BindPrimitive()
    {
    return *this;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
bool PrimitiveECSqlParameterValue::_IsNull() const
    {
    return m_value.IsNull();
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
void const* PrimitiveECSqlParameterValue::_GetBinary(int* binarySize) const
    {
    size_t size = 0;
    auto blob = m_value.GetBinary(size);

    if (binarySize != nullptr)
        *binarySize = static_cast<int> (size);

    return static_cast<void const *> (blob);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
bool PrimitiveECSqlParameterValue::_GetBoolean() const
    {
    return m_value.GetBoolean();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      07/2014
//---------------------------------------------------------------------------------------
uint64_t PrimitiveECSqlParameterValue::_GetDateTimeJulianDays(DateTime::Info& metadata) const
    {
    bool hasMetdata = false;
    const int64_t ceTicks = m_value.GetDateTimeTicks(hasMetdata, metadata);
    return DateTime::CommonEraTicksToJulianDay(ceTicks);
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
double PrimitiveECSqlParameterValue::_GetDouble() const
    {
    return m_value.GetDouble();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
int PrimitiveECSqlParameterValue::_GetInt() const
    {
    return m_value.GetInteger();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
int64_t PrimitiveECSqlParameterValue::_GetInt64() const
    {
    return m_value.GetLong();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
Utf8CP PrimitiveECSqlParameterValue::_GetText() const
    {
    return m_value.GetUtf8CP ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
DPoint2d PrimitiveECSqlParameterValue::_GetPoint2D () const
    {
    return m_value.GetPoint2D ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
DPoint3d PrimitiveECSqlParameterValue::_GetPoint3D () const
    {
    return m_value.GetPoint3D ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      06/2015
//---------------------------------------------------------------------------------------
IGeometryPtr PrimitiveECSqlParameterValue::_GetGeometry() const
    {
    return m_value.GetIGeometry();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      11/2014
//---------------------------------------------------------------------------------------
void const* PrimitiveECSqlParameterValue::_GetGeometryBlob(int* blobSize) const
    {
    size_t size = 0;
    auto blob = m_value.GetBinary(size);

    if (blobSize != nullptr)
        *blobSize = static_cast<int> (size);

    return static_cast<void const *> (blob);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
IECSqlPrimitiveValue const& PrimitiveECSqlParameterValue::_GetPrimitive() const
    {
    ResetStatus();
    return *this;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2014
//---------------------------------------------------------------------------------------
void PrimitiveECSqlParameterValue::_Clear()
    {
    DoClear();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2014
//---------------------------------------------------------------------------------------
void PrimitiveECSqlParameterValue::DoClear()
    {
    m_value.SetIsNull(true);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
bool PrimitiveECSqlParameterValue::CanBindValue(ECN::PrimitiveType actualType) const
    {
    return actualType == GetTypeInfo().GetPrimitiveType();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
//static
ECSqlStatus PrimitiveECSqlParameterValue::ToECSqlStatus(BentleyStatus bStat)
    {
    if (bStat == SUCCESS)
        return ECSqlStatus::Success;

    return ECSqlStatus::UserError;
    }



// ************************************************
// StructECSqlParameterValue
// ************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
StructECSqlParameterValue::StructECSqlParameterValue(ECSqlStatusContext& statusContext, ECSqlTypeInfo const& typeInfo)
: ECSqlParameterValue(statusContext, typeInfo), IECSqlStructBinder(), IECSqlStructValue()
    {
    //initialize member parameter values only once and reuse them after each ClearBindings
    for (auto memberProp : GetTypeInfo().GetStructType().GetProperties(true))
        {
        m_propertyValueMap[memberProp->GetId()] = ECSqlParameterValueFactory::Create(GetStatusContext(), ECSqlTypeInfo(*memberProp));
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
ECSqlStatus StructECSqlParameterValue::_BindNull()
    {
    DoClear();
    return ResetStatus();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
IECSqlStructBinder& StructECSqlParameterValue::_BindStruct()
    {
    ResetStatus();
    return *this;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
bool StructECSqlParameterValue::_IsNull() const
    {
    ResetStatus();
    return false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
IECSqlStructValue const& StructECSqlParameterValue::_GetStruct() const
    {
    ResetStatus();
    return *this;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
IECSqlBinder& StructECSqlParameterValue::_GetMember(ECN::ECPropertyId structMemberPropertyId)
    {
    auto it = m_propertyValueMap.find(structMemberPropertyId);
    if (it == m_propertyValueMap.end())
        {
        Utf8CP structMemberPropertyName = nullptr;
        for (ECPropertyCP prop : GetTypeInfo().GetStructType().GetProperties(true))
            {
            if (prop->GetId() == structMemberPropertyId)
                {
                structMemberPropertyName = prop->GetName().c_str();
                break;
                }
            }

        auto stat = GetStatusContext().SetError(ECSqlStatus::UserError, "Struct member '%s.%s' does not exist",
                                                GetTypeInfo().GetStructType().GetFullName(), structMemberPropertyName);
        return NoopECSqlBinderFactory::GetBinder(stat);
        }

    return *it->second;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
IECSqlBinder& StructECSqlParameterValue::_GetMember(Utf8CP structMemberPropertyName)
    {
    auto structMemberProp = GetTypeInfo().GetStructType().GetPropertyP (structMemberPropertyName, true);
    if (structMemberProp == nullptr)
        {
        auto stat = GetStatusContext().SetError(ECSqlStatus::UserError, "Struct member '%s.%s' does not exist",
            GetTypeInfo().GetStructType().GetFullName(),
            structMemberPropertyName);
        return NoopECSqlBinderFactory::GetBinder(stat);
        }

    return _GetMember(structMemberProp->GetId());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
IECSqlValue const& StructECSqlParameterValue::GetValue(ECN::ECPropertyId structMemberPropertyId) const
    {
    auto it = m_propertyValueMap.find(structMemberPropertyId);
    if (it == m_propertyValueMap.end())
        {
        Utf8CP structMemberPropertyName = nullptr;
        for (auto prop : GetTypeInfo().GetStructType().GetProperties(true))
            {
            if (prop->GetId() == structMemberPropertyId)
                {
                structMemberPropertyName = prop->GetName().c_str();
                break;
                }
            }

        GetStatusContext().SetError(ECSqlStatus::UserError, "Struct member '%s.%s' does not exist", 
            GetTypeInfo().GetStructType().GetFullName(),
            structMemberPropertyName);
        return NoopECSqlValue::GetSingleton();
        }

    return *it->second;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
IECSqlValue const& StructECSqlParameterValue::_GetValue(int columnIndex) const
    {
    BeAssert(false && "StructECSqlParameterValue::GetValue (columnIndex) is not expected to be called.");
    return NoopECSqlValue::GetSingleton();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
int StructECSqlParameterValue::_GetMemberCount() const
    {
    return static_cast<int> (m_propertyValueMap.size());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2014
//---------------------------------------------------------------------------------------
void StructECSqlParameterValue::_Clear()
    {
    DoClear();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2014
//---------------------------------------------------------------------------------------
void StructECSqlParameterValue::DoClear()
    {
    for (auto const& kvPair : m_propertyValueMap)
        {
        kvPair.second->Clear();
        }
    }

// ************************************************
// ArrayECSqlParameterValue
// ************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
ArrayECSqlParameterValue::ArrayECSqlParameterValue(ECSqlStatusContext& statusContext, ECSqlTypeInfo const& typeInfo)
    : ECSqlParameterValue(statusContext, typeInfo), m_iterator(m_arrayElementValues.end())
    {}

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
ECSqlStatus ArrayECSqlParameterValue::_BindNull()
    {
    DoClear();
    return ResetStatus();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
IECSqlArrayBinder& ArrayECSqlParameterValue::_BindArray(uint32_t initialCapacity)
    {
    ResetStatus();
    DoClear();
    m_arrayElementValues.Reserve(initialCapacity);
    return *this;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
IECSqlBinder& ArrayECSqlParameterValue::_AddArrayElement()
    {
    auto const& typeInfo = GetTypeInfo();
    const auto stat = ArrayConstraintValidator::Validate(GetStatusContext(), typeInfo, (uint32_t) m_arrayElementValues.Size() + 1);
    if (stat != ECSqlStatus::Success)
        return NoopECSqlBinderFactory::GetBinder(stat).AddArrayElement();
    
    IECSqlBinder* arrayElementP = nullptr;
    if (m_arrayElementValues.HasNextAllocated())
        arrayElementP = m_arrayElementValues.GetNextAllocated();
    else
        {
        std::unique_ptr<ECSqlParameterValue> arrayElement = nullptr;
        if (GetTypeInfo().GetKind() == ECSqlTypeInfo::Kind::PrimitiveArray)
            arrayElement = ECSqlParameterValueFactory::CreatePrimitive(GetStatusContext(), ECSqlTypeInfo(GetTypeInfo().GetPrimitiveType()));
        else
            arrayElement = ECSqlParameterValueFactory::CreateStruct(GetStatusContext(), ECSqlTypeInfo(GetTypeInfo().GetStructType()));

        arrayElementP = arrayElement.get(); //cache raw pointer to return it as smart pointer will be nulled out after push_back
        m_arrayElementValues.Append(arrayElement);
        }

    ResetStatus();
    BeAssert(arrayElementP != nullptr);
    return *arrayElementP;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
bool ArrayECSqlParameterValue::_IsNull() const
    {
    return m_arrayElementValues.Size() == 0;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
void ArrayECSqlParameterValue::_MoveNext(bool onInitializingIterator /*= false*/) const
    {
    if (onInitializingIterator)
        {
        m_iterator = m_arrayElementValues.begin(); //iterator returned is already at first position, so do not increment here
        }
    else
        ++m_iterator;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
bool ArrayECSqlParameterValue::_IsAtEnd() const
    {
    return m_iterator == m_arrayElementValues.end();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
IECSqlValue const* ArrayECSqlParameterValue::_GetCurrent() const
    {
    return *m_iterator;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
int ArrayECSqlParameterValue::_GetArrayLength() const
    {
    return (int) m_arrayElementValues.Size();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2014
//---------------------------------------------------------------------------------------
void ArrayECSqlParameterValue::_Clear()
    {
    DoClear();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2014
//---------------------------------------------------------------------------------------
void ArrayECSqlParameterValue::DoClear()
    {
    m_arrayElementValues.Clear();
    }


// *****************************************************
// ArrayECSqlParameterValue::Collection
// *****************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2014
//---------------------------------------------------------------------------------------
ArrayECSqlParameterValue::Collection::Collection() 
    : m_size(0)
    {}

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2014
//---------------------------------------------------------------------------------------
ECSqlParameterValue* ArrayECSqlParameterValue::Collection::operator[] (size_t index) const
    {
    if (index >= Size())
        return nullptr;

    return m_collection[index].get();
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2014
//---------------------------------------------------------------------------------------
void ArrayECSqlParameterValue::Collection::Append(std::unique_ptr<ECSqlParameterValue>& value)
    {
    m_collection.push_back(std::move(value));
    m_size++;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2014
//---------------------------------------------------------------------------------------
bool ArrayECSqlParameterValue::Collection::HasNextAllocated() const
    {
    return m_size < Capacity();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2014
//---------------------------------------------------------------------------------------
ECSqlParameterValue* ArrayECSqlParameterValue::Collection::GetNextAllocated() const
    {
    if (!HasNextAllocated())
        return nullptr;

    auto next = m_collection[m_size].get();
    m_size++;
    return next;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2014
//---------------------------------------------------------------------------------------
void ArrayECSqlParameterValue::Collection::Clear()
    {
    if (m_size > 0)
        {
        for (auto& val : m_collection)
            {
            val->Clear();
            }

        m_size = 0;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2014
//---------------------------------------------------------------------------------------
size_t ArrayECSqlParameterValue::Collection::Size() const
    {
    return m_size;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2014
//---------------------------------------------------------------------------------------
size_t ArrayECSqlParameterValue::Collection::Capacity() const
    {
    return m_collection.size();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2014
//---------------------------------------------------------------------------------------
void ArrayECSqlParameterValue::Collection::Reserve(size_t count)
    {
    m_collection.reserve(count);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2014
//---------------------------------------------------------------------------------------
ArrayECSqlParameterValue::Collection::const_iterator ArrayECSqlParameterValue::Collection::begin() const
    {
    return const_iterator(*this, false);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2014
//---------------------------------------------------------------------------------------
ArrayECSqlParameterValue::Collection::const_iterator ArrayECSqlParameterValue::Collection::end() const
    {
    return const_iterator(*this, true);
    }

// *****************************************************
// ArrayECSqlParameterValue::Collection::const_iterator
// *****************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2014
//---------------------------------------------------------------------------------------
ArrayECSqlParameterValue::Collection::const_iterator::const_iterator(Collection const& coll, bool isEndIterator) 
    : m_collection(&coll), m_maxIndex(coll.Size()), m_currentIndex(isEndIterator ? coll.Size() : 0)
    {}

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2014
//---------------------------------------------------------------------------------------
ArrayECSqlParameterValue::Collection::const_iterator::const_iterator(const_iterator const& rhs) 
    : m_collection(rhs.m_collection), m_maxIndex(rhs.m_maxIndex), m_currentIndex(rhs.m_currentIndex)
    {}

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2014
//---------------------------------------------------------------------------------------
ArrayECSqlParameterValue::Collection::const_iterator& ArrayECSqlParameterValue::Collection::const_iterator::operator=(const_iterator const& rhs)
    {
    if (this != &rhs)
        {
        m_collection = rhs.m_collection;
        m_maxIndex = rhs.m_maxIndex;
        m_currentIndex = rhs.m_currentIndex;
        }

    return *this;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2014
//---------------------------------------------------------------------------------------
ECSqlParameterValue const* ArrayECSqlParameterValue::Collection::const_iterator::operator*() const
    {
    if (m_currentIndex >= m_maxIndex)
        return nullptr;

    return (*m_collection)[m_currentIndex];
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2014
//---------------------------------------------------------------------------------------
ArrayECSqlParameterValue::Collection::const_iterator& ArrayECSqlParameterValue::Collection::const_iterator::operator++()
    {
    if (m_currentIndex < m_maxIndex)
        m_currentIndex++;

    return *this;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2014
//---------------------------------------------------------------------------------------
bool ArrayECSqlParameterValue::Collection::const_iterator::operator==(const_iterator const& rhs) const
    {
    return m_currentIndex == rhs.m_currentIndex;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2014
//---------------------------------------------------------------------------------------
bool ArrayECSqlParameterValue::Collection::const_iterator::operator!=(const_iterator const& rhs) const
    {
    return !(*this == rhs);
    }

END_BENTLEY_SQLITE_EC_NAMESPACE

