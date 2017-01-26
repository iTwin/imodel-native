/*------------------
--------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/StructArrayECSqlBinder.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//---------------------------------------------------------------------------------------
StructArrayECSqlBinder::StructArrayECSqlBinder(ECSqlStatementBase& stmt, ECSqlTypeInfo const& typeInfo)
    : ECSqlBinder(stmt, typeInfo, 1, true, true)
    {
    BeAssert(GetTypeInfo().GetKind() == ECSqlTypeInfo::Kind::StructArray);
    Initialize();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      01/2016
//---------------------------------------------------------------------------------------
void StructArrayECSqlBinder::Initialize()
    {
    m_json = Json::arrayValue;
    // root binder refers to the entire struct array, not just the first array element
    m_rootBinder = JsonValueBinder(GetECDb(), GetTypeInfo(), m_json);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//---------------------------------------------------------------------------------------
ECSqlStatus StructArrayECSqlBinder::_OnBeforeStep()
    {
    ECSqlStatus stat = ArrayConstraintValidator::Validate(GetECDb(), GetTypeInfo(), (uint32_t) m_json.size());
    if (!stat.IsSuccess())
        return stat;

    if (m_json.empty())
        return ECSqlStatus::Success;

    PrimitiveECSqlBinder jsonBinder(GetECSqlStatementR(), ECSqlTypeInfo(PRIMITIVETYPE_String));
    jsonBinder.SetSqliteIndex(m_sqliteIndex);

    BeAssert(m_json.isArray());
    Json::FastWriter writer;
    Utf8String jsonStr = writer.write(m_json);
    return jsonBinder.BindText(jsonStr.c_str(), IECSqlBinder::MakeCopy::Yes);
    }


//******************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      01/2017
//---------------------------------------------------------------------------------------
StructArrayECSqlBinder::JsonValueBinder::JsonValueBinder(JsonValueBinder&& rhs)
    : m_ecdb(std::move(rhs.m_ecdb)), m_typeInfo(std::move(rhs.m_typeInfo)), m_json(std::move(rhs.m_json)), m_currentArrayElementBinder(std::move(rhs.m_currentArrayElementBinder))
    {
    if (!rhs.m_structMemberBinders.empty())
        m_structMemberBinders = std::move(rhs.m_structMemberBinders);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      01/2017
//---------------------------------------------------------------------------------------
StructArrayECSqlBinder::JsonValueBinder& StructArrayECSqlBinder::JsonValueBinder::operator=(JsonValueBinder&& rhs)
    {
    if (this == &rhs)
        return *this;

    m_ecdb = std::move(rhs.m_ecdb);
    m_typeInfo = std::move(rhs.m_typeInfo);
    m_json = std::move(rhs.m_json);

    if (!rhs.m_structMemberBinders.empty())
        m_structMemberBinders = std::move(rhs.m_structMemberBinders);

    m_currentArrayElementBinder = std::move(rhs.m_currentArrayElementBinder);

    return *this;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      01/2017
//---------------------------------------------------------------------------------------
ECSqlStatus StructArrayECSqlBinder::JsonValueBinder::_BindNull()
    {
    ECSqlStatus stat = FailIfInvalid();
    if (!stat.IsSuccess())
        return stat;

    m_json->clear();
    m_structMemberBinders.clear();
    m_currentArrayElementBinder = nullptr;
    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//---------------------------------------------------------------------------------------
ECSqlStatus StructArrayECSqlBinder::JsonValueBinder::_BindBoolean(bool value)
    {
    ECSqlStatus stat = FailIfTypeMismatch(PRIMITIVETYPE_Boolean);
    if (!stat.IsSuccess())
        return stat;

    *m_json = Json::Value(value);
    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//---------------------------------------------------------------------------------------
ECSqlStatus StructArrayECSqlBinder::JsonValueBinder::_BindBlob(void const* value, int binarySize, IECSqlBinder::MakeCopy makeCopy)
    {
    ECSqlStatus stat = FailIfTypeMismatch(PRIMITIVETYPE_Binary);
    if (!stat.IsSuccess())
        return stat;

    Byte const* blob = static_cast<Byte const*> (value);
    return SUCCESS == ECJsonUtilities::BinaryToJson(*m_json, blob, binarySize) ? ECSqlStatus::Success : ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//---------------------------------------------------------------------------------------
ECSqlStatus StructArrayECSqlBinder::JsonValueBinder::_BindZeroBlob(int blobSize)
    {
    ECSqlStatus stat = FailIfInvalid();
    if (!stat.IsSuccess())
        return stat;

    LOG.error("Type mismatch. Cannot bind Zeroblob values to elements of a struct array parameter.");
    return ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//---------------------------------------------------------------------------------------
ECSqlStatus StructArrayECSqlBinder::JsonValueBinder::_BindDateTime(double julianDay, DateTime::Info const& metadata)
    {
    ECSqlStatus stat = FailIfTypeMismatch(PRIMITIVETYPE_DateTime);
    if (!stat.IsSuccess())
        return stat;

    if (metadata.IsValid() && metadata.GetKind() == DateTime::Kind::Local)
        {
        LOG.error("ECDb does not support to bind local date times.");
        return ECSqlStatus::Error;
        }

    if (!m_typeInfo.DateTimeInfoMatches(metadata))
        {
        LOG.error("Metadata of DateTime value to bind doesn't match the metadata on the corresponding ECProperty.");
        return ECSqlStatus::Error;
        }

    *m_json = Json::Value(julianDay);
    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//---------------------------------------------------------------------------------------
ECSqlStatus StructArrayECSqlBinder::JsonValueBinder::_BindDateTime(uint64_t julianDayMsec, DateTime::Info const& metadata)
    {
    const double jd = DateTime::MsecToRationalDay(julianDayMsec);
    return _BindDateTime(jd, metadata);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//---------------------------------------------------------------------------------------
ECSqlStatus StructArrayECSqlBinder::JsonValueBinder::_BindDouble(double value)
    {
    ECSqlStatus stat = FailIfTypeMismatch(PRIMITIVETYPE_Double);
    if (!stat.IsSuccess())
        return stat;

    *m_json = Json::Value(value);
    return ECSqlStatus::Success;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//---------------------------------------------------------------------------------------
ECSqlStatus StructArrayECSqlBinder::JsonValueBinder::_BindInt(int value)
    {
    ECSqlStatus stat = FailIfTypeMismatch(PRIMITIVETYPE_Integer);
    if (!stat.IsSuccess())
        return stat;

    *m_json = Json::Value(value);
    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//---------------------------------------------------------------------------------------
ECSqlStatus StructArrayECSqlBinder::JsonValueBinder::_BindInt64(int64_t value)
    {
    ECSqlStatus stat = FailIfTypeMismatch(PRIMITIVETYPE_Long);
    if (!stat.IsSuccess())
        return stat;

    *m_json = Json::Value(value);
    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//---------------------------------------------------------------------------------------
ECSqlStatus StructArrayECSqlBinder::JsonValueBinder::_BindPoint2d(DPoint2dCR value)
    {
    ECSqlStatus stat = FailIfTypeMismatch(PRIMITIVETYPE_Point2d);
    if (!stat.IsSuccess())
        return stat;

    return SUCCESS == ECJsonUtilities::Point2dToJson(*m_json, value) ? ECSqlStatus::Success : ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//---------------------------------------------------------------------------------------
ECSqlStatus StructArrayECSqlBinder::JsonValueBinder::_BindPoint3d(DPoint3dCR value)
    {
    ECSqlStatus stat = FailIfTypeMismatch(PRIMITIVETYPE_Point3d);
    if (!stat.IsSuccess())
        return stat;

    return SUCCESS == ECJsonUtilities::Point3dToJson(*m_json, value) ? ECSqlStatus::Success : ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//---------------------------------------------------------------------------------------
ECSqlStatus StructArrayECSqlBinder::JsonValueBinder::_BindText(Utf8CP value, IECSqlBinder::MakeCopy makeCopy, int byteCount)
    {
    ECSqlStatus stat = FailIfTypeMismatch(PRIMITIVETYPE_String);
    if (!stat.IsSuccess())
        return stat;

    *m_json = Json::Value(value);
    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//---------------------------------------------------------------------------------------
IECSqlBinder& StructArrayECSqlBinder::JsonValueBinder::_BindStructMember(Utf8CP structMemberPropertyName)
    {
    ECSqlStatus stat = FailIfInvalid();
    if (!stat.IsSuccess())
        return NoopECSqlBinder::Get();

    if (!m_typeInfo.IsStruct())
        {
        LOG.error("Type mismatch. Can only call IECSqlBinder[] for struct parameters.");
        return NoopECSqlBinder::Get();
        }

    ECPropertyCP memberProp = m_typeInfo.GetStructType().GetPropertyP(structMemberPropertyName, true);
    if (memberProp == nullptr)
        {
        LOG.errorv("Cannot bind to struct member. Member %s does not exist.", structMemberPropertyName);
        BeAssert(false);
        return NoopECSqlBinder::Get();
        }

    auto it = m_structMemberBinders.find(memberProp->GetId());
    if (it != m_structMemberBinders.end())
        return it->second;

    return CreateStructMemberBinder(*memberProp);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//---------------------------------------------------------------------------------------
IECSqlBinder& StructArrayECSqlBinder::JsonValueBinder::_BindStructMember(ECN::ECPropertyId structMemberPropertyId)
    {
    ECSqlStatus stat = FailIfInvalid();
    if (!stat.IsSuccess())
        return NoopECSqlBinder::Get();

    if (!m_typeInfo.IsStruct())
        {
        LOG.error("Type mismatch. Can only call IECSqlBinder[] for struct parameters.");
        return NoopECSqlBinder::Get();
        }

    auto it = m_structMemberBinders.find(structMemberPropertyId);
    if (it != m_structMemberBinders.end())
        return it->second;

    for (ECPropertyCP prop : m_typeInfo.GetStructType().GetProperties(true))
        {
        if (prop->GetId() == structMemberPropertyId)
            return CreateStructMemberBinder(*prop);
        }

    LOG.errorv("Cannot bind to struct member. Member with ECPropertyId %s does not exist.", structMemberPropertyId.ToString().c_str());
    return NoopECSqlBinder::Get();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      01/2017
//---------------------------------------------------------------------------------------
IECSqlBinder& StructArrayECSqlBinder::JsonValueBinder::CreateStructMemberBinder(ECN::ECPropertyCR memberProp)
    {
    BeAssert(m_structMemberBinders.find(memberProp.GetId()) == m_structMemberBinders.end());
    Json::Value& structMemberJson = m_json->operator[](memberProp.GetName().c_str());
    auto newPairIt = m_structMemberBinders.insert(std::make_pair(memberProp.GetId(), JsonValueBinder(*m_ecdb, ECSqlTypeInfo(memberProp), structMemberJson)));
    BeAssert(newPairIt.second && "insert into std::map is expected to create a new element");
    return newPairIt.first->second;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//---------------------------------------------------------------------------------------
IECSqlBinder& StructArrayECSqlBinder::JsonValueBinder::_AddArrayElement()
    {
    ECSqlStatus stat = FailIfInvalid();
    if (!stat.IsSuccess())
        return NoopECSqlBinder::Get();

    if (!m_typeInfo.IsArray())
        {
        LOG.error("Type mismatch. AddArrayElement can only be called for array parameters.");
        return NoopECSqlBinder::Get();
        }

    if (ECSqlStatus::Success != ArrayConstraintValidator::ValidateMaximum(*m_ecdb, m_typeInfo, (uint32_t) m_json->size() + 1))
        return NoopECSqlBinder::Get();

    Json::Value* elementJson = nullptr;
    if (m_typeInfo.GetKind() == ECSqlTypeInfo::Kind::PrimitiveArray)
        elementJson = &m_json->append(Json::Value());
    else
        {
        BeAssert(m_typeInfo.GetKind() == ECSqlTypeInfo::Kind::StructArray);
        elementJson = &m_json->append(Json::objectValue);
        }

    return MoveCurrentArrayElementBinder(*m_ecdb, m_typeInfo, *elementJson);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      01/2017
//---------------------------------------------------------------------------------------
IECSqlBinder& StructArrayECSqlBinder::JsonValueBinder::MoveCurrentArrayElementBinder(ECDbCR ecdb, ECSqlTypeInfo const& arrayTypeInfo, Json::Value& newArrayElementJson)
    {
    if (m_currentArrayElementBinder != nullptr)
        {
        m_currentArrayElementBinder->m_json = &newArrayElementJson;
        m_currentArrayElementBinder->m_structMemberBinders.clear();
        m_currentArrayElementBinder->m_currentArrayElementBinder = nullptr;
        return *m_currentArrayElementBinder;
        }

    if (arrayTypeInfo.GetKind() == ECSqlTypeInfo::Kind::PrimitiveArray)
        m_currentArrayElementBinder = std::make_unique<JsonValueBinder>(ecdb, ECSqlTypeInfo(arrayTypeInfo.GetPrimitiveType()), newArrayElementJson);
    else
        {
        BeAssert(arrayTypeInfo.GetKind() == ECSqlTypeInfo::Kind::StructArray);
        m_currentArrayElementBinder = std::make_unique<JsonValueBinder>(ecdb, ECSqlTypeInfo(arrayTypeInfo.GetStructType()), newArrayElementJson);
        }

    return *m_currentArrayElementBinder;
    }



//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      01/2017
//---------------------------------------------------------------------------------------
ECSqlStatus StructArrayECSqlBinder::JsonValueBinder::FailIfTypeMismatch(ECN::PrimitiveType boundType) const
    {
    ECSqlStatus stat = FailIfInvalid();
    if (!stat.IsSuccess())
        return stat;

    if (!m_typeInfo.IsPrimitive())
        {
        LOG.error("Type mismatch. Primitive values can only be bound to primitive parameter values.");
        return ECSqlStatus::Error;
        }

    if (boundType == PRIMITIVETYPE_Binary)
        {
        if (m_typeInfo.GetPrimitiveType() != PRIMITIVETYPE_Binary && m_typeInfo.GetPrimitiveType() != PRIMITIVETYPE_IGeometry)
            {
            LOG.error("Type mismatch. BLOB values can only be bound to BLOB or IGeometry parameter values.");
            return ECSqlStatus::Error;
            }

        return ECSqlStatus::Success;
        }

    if (boundType != m_typeInfo.GetPrimitiveType())
        {
        LOG.errorv("Type mismatch. %s values can only be bound to %s parameter values.", ExpHelper::ToString(boundType));
        return ECSqlStatus::Error;
        }

    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//---------------------------------------------------------------------------------------
ECSqlStatus StructArrayECSqlBinder::JsonValueBinder::FailIfInvalid() const
    {
    if (m_json == nullptr)
        {
        LOG.error("Cannot bind values to struct array binder without calling IECSqlBinder::AddArrayElement first.");
        return ECSqlStatus::Error;
        }

    return ECSqlStatus::Success;
    }



END_BENTLEY_SQLITE_EC_NAMESPACE