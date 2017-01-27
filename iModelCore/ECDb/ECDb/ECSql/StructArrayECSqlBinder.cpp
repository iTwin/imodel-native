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
    // root binder refers to the entire struct array, not just the first array element
    BeAssert(m_json.IsNull() || m_json.IsArray());
    if (m_json.IsArray())
        m_json.Clear();

    m_rootBinder = std::make_unique<JsonValueBinder>(GetECDb(), GetTypeInfo(), m_json, m_json.GetAllocator());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//---------------------------------------------------------------------------------------
ECSqlStatus StructArrayECSqlBinder::_OnBeforeStep()
    {
    const uint32_t arrayLength = m_json.IsNull() ? 0 : (uint32_t) m_json.Size();
    ECSqlStatus stat = ArrayConstraintValidator::Validate(GetECDb(), GetTypeInfo(), arrayLength);
    if (!stat.IsSuccess())
        return stat;

    if (arrayLength == 0)
        return ECSqlStatus::Success;

    PrimitiveECSqlBinder jsonBinder(GetECSqlStatementR(), ECSqlTypeInfo(PRIMITIVETYPE_String));
    jsonBinder.SetSqliteIndex(m_sqliteIndex);

    BeAssert(m_json.IsArray());
    rapidjson::StringBuffer jsonStr;
    rapidjson::Writer<rapidjson::StringBuffer> writer(jsonStr);
    m_json.Accept(writer);
    return jsonBinder.BindText(jsonStr.GetString(), IECSqlBinder::MakeCopy::Yes);
    }


//******************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      01/2017
//---------------------------------------------------------------------------------------
StructArrayECSqlBinder::JsonValueBinder::JsonValueBinder(ECDbCR ecdb, ECSqlTypeInfo const& typeInfo, rapidjson::Value& json, rapidjson::MemoryPoolAllocator<>& jsonAllocator) 
    : IECSqlBinder(), m_ecdb(&ecdb), m_typeInfo(typeInfo), m_json(&json), m_jsonAllocator(&jsonAllocator), m_currentArrayElementBinder(nullptr)
    {
    BeAssert(m_json != nullptr);
    BeAssert(m_jsonAllocator != nullptr);
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      01/2017
//---------------------------------------------------------------------------------------
StructArrayECSqlBinder::JsonValueBinder::JsonValueBinder(JsonValueBinder&& rhs)
    : m_ecdb(std::move(rhs.m_ecdb)), m_typeInfo(std::move(rhs.m_typeInfo)), m_json(std::move(rhs.m_json)), m_jsonAllocator(std::move(rhs.m_jsonAllocator)), m_currentArrayElementBinder(std::move(rhs.m_currentArrayElementBinder))
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
    m_jsonAllocator = std::move(rhs.m_jsonAllocator);

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

    Reset();
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

    m_json->SetBool(value);
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
    return SUCCESS == ECRapidJsonUtilities::BinaryToJson(*m_json, blob, binarySize, *m_jsonAllocator) ? ECSqlStatus::Success : ECSqlStatus::Error;
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

    m_json->SetDouble(julianDay);
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

    m_json->SetDouble(value);
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

    m_json->SetInt(value);
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

    m_json->SetInt64(value);
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

    return SUCCESS == ECRapidJsonUtilities::Point2dToJson(*m_json, value, *m_jsonAllocator) ? ECSqlStatus::Success : ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//---------------------------------------------------------------------------------------
ECSqlStatus StructArrayECSqlBinder::JsonValueBinder::_BindPoint3d(DPoint3dCR value)
    {
    ECSqlStatus stat = FailIfTypeMismatch(PRIMITIVETYPE_Point3d);
    if (!stat.IsSuccess())
        return stat;

    return SUCCESS == ECRapidJsonUtilities::Point3dToJson(*m_json, value, *m_jsonAllocator) ? ECSqlStatus::Success : ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//---------------------------------------------------------------------------------------
ECSqlStatus StructArrayECSqlBinder::JsonValueBinder::_BindText(Utf8CP value, IECSqlBinder::MakeCopy makeCopy, int stringLength)
    {
    ECSqlStatus stat = FailIfTypeMismatch(PRIMITIVETYPE_String);
    if (!stat.IsSuccess())
        return stat;

    if (stringLength < 0)
        stringLength = (int) strlen(value);

    if (makeCopy == IECSqlBinder::MakeCopy::No)
        m_json->SetString(value, (rapidjson::SizeType) stringLength);
    else
        m_json->SetString(value, (rapidjson::SizeType) stringLength, *m_jsonAllocator);

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
    BeAssert(m_json != nullptr);
    BeAssert(m_jsonAllocator != nullptr);

    if (m_json->IsNull())
        m_json->SetObject();

    BeAssert(m_structMemberBinders.find(memberProp.GetId()) == m_structMemberBinders.end());
    rapidjson::GenericStringRef<Utf8Char> memberName(memberProp.GetName().c_str(), (rapidjson::SizeType) memberProp.GetName().size());
    //Caution: return value is struct again, not the inserted member
    m_json->AddMember(memberName, rapidjson::Value().Move(), *m_jsonAllocator);
    rapidjson::Value& structMemberJson = m_json->operator[](memberName.s);
    auto newPairIt = m_structMemberBinders.insert(std::make_pair(memberProp.GetId(), JsonValueBinder(*m_ecdb, ECSqlTypeInfo(memberProp), structMemberJson, *m_jsonAllocator)));
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

    if (m_json->IsNull())
        m_json->SetArray();

    if (ECSqlStatus::Success != ArrayConstraintValidator::ValidateMaximum(*m_ecdb, m_typeInfo, (uint32_t) m_json->Size() + 1))
        return NoopECSqlBinder::Get();

    //Caution: return value is array again, not the inserted element
    return MoveCurrentArrayElementBinder(*m_ecdb, m_typeInfo);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      01/2017
//---------------------------------------------------------------------------------------
IECSqlBinder& StructArrayECSqlBinder::JsonValueBinder::MoveCurrentArrayElementBinder(ECDbCR ecdb, ECSqlTypeInfo const& arrayTypeInfo)
    {
    BeAssert(m_json->IsArray());
    m_json->PushBack(rapidjson::Value().Move(), *m_jsonAllocator);
    rapidjson::Value& newArrayElementJson = m_json->operator[](m_json->Size() - 1);

    if (m_currentArrayElementBinder != nullptr)
        {
        m_currentArrayElementBinder->Reset(newArrayElementJson);
        return *m_currentArrayElementBinder;
        }

    if (arrayTypeInfo.GetKind() == ECSqlTypeInfo::Kind::PrimitiveArray)
        m_currentArrayElementBinder = std::make_unique<JsonValueBinder>(ecdb, ECSqlTypeInfo(arrayTypeInfo.GetPrimitiveType()), newArrayElementJson, *m_jsonAllocator);
    else
        {
        BeAssert(arrayTypeInfo.GetKind() == ECSqlTypeInfo::Kind::StructArray);
        m_currentArrayElementBinder = std::make_unique<JsonValueBinder>(ecdb, ECSqlTypeInfo(arrayTypeInfo.GetStructType()), newArrayElementJson, *m_jsonAllocator);
        }

    return *m_currentArrayElementBinder;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      01/2017
//---------------------------------------------------------------------------------------
void StructArrayECSqlBinder::JsonValueBinder::Reset()
    {
    m_json->SetNull();
    m_structMemberBinders.clear();
    m_currentArrayElementBinder = nullptr;
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