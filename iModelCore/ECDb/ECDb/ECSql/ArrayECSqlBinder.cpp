/*------------------
--------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ArrayECSqlBinder.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//---------------------------------------------------------------------------------------
ArrayECSqlBinder::ArrayECSqlBinder(ECSqlPrepareContext& ctx, ECSqlTypeInfo const& typeInfo, SqlParamNameGenerator& paramNameGen)
    : ECSqlBinder(ctx, typeInfo, paramNameGen, 1, true, true)
    {
    BeAssert(GetTypeInfo().IsArray());
    Initialize();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      01/2016
//---------------------------------------------------------------------------------------
void ArrayECSqlBinder::Initialize()
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
ECSqlStatus ArrayECSqlBinder::_OnBeforeStep()
    {
    const uint32_t arrayLength = m_json.IsNull() ? 0 : (uint32_t) m_json.Size();
    // from the API we cannot tell between binding NULL and binding an empty array. so we treat them
    // the same and treat it as NULL which means to *not* validate the bounds.
    if (arrayLength == 0)
        return ECSqlStatus::Success;

    const ECSqlStatus stat = ArrayConstraintValidator::Validate(GetECDb(), GetTypeInfo(), arrayLength);
    if (!stat.IsSuccess())
        return stat;

    BeAssert(m_json.IsArray());
    rapidjson::StringBuffer jsonStr;
    rapidjson::Writer<rapidjson::StringBuffer> writer(jsonStr);
    m_json.Accept(writer);

    Statement& sqliteStmt = GetSqliteStatement();
    BeAssert(GetMappedSqlParameterNames().size() == 1 && !GetMappedSqlParameterNames()[0].empty());
    const int sqlParamIx = sqliteStmt.GetParameterIndex(GetMappedSqlParameterNames()[0].c_str());
    const DbResult dbRes = sqliteStmt.BindText(sqlParamIx, jsonStr.GetString(), Statement::MakeCopy::Yes);
    if (BE_SQLITE_OK == dbRes)
        return ECSqlStatus::Success;

    return ECSqlStatus(dbRes);
    }


//******************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      01/2017
//---------------------------------------------------------------------------------------
ArrayECSqlBinder::JsonValueBinder::JsonValueBinder(ECDbCR ecdb, ECSqlTypeInfo const& typeInfo, rapidjson::Value& json, rapidjson::MemoryPoolAllocator<>& jsonAllocator) 
    : IECSqlBinder(), m_ecdb(&ecdb), m_typeInfo(typeInfo), m_json(&json), m_jsonAllocator(&jsonAllocator), m_currentArrayElementBinder(nullptr)
    {
    BeAssert(m_json != nullptr);
    BeAssert(m_jsonAllocator != nullptr);
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      01/2017
//---------------------------------------------------------------------------------------
ArrayECSqlBinder::JsonValueBinder::JsonValueBinder(JsonValueBinder&& rhs)
    : m_ecdb(std::move(rhs.m_ecdb)), m_typeInfo(std::move(rhs.m_typeInfo)), m_json(std::move(rhs.m_json)), m_jsonAllocator(std::move(rhs.m_jsonAllocator)), m_currentArrayElementBinder(std::move(rhs.m_currentArrayElementBinder))
    {
    if (!rhs.m_structMemberBinders.empty())
        m_structMemberBinders = std::move(rhs.m_structMemberBinders);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      01/2017
//---------------------------------------------------------------------------------------
ArrayECSqlBinder::JsonValueBinder& ArrayECSqlBinder::JsonValueBinder::operator=(JsonValueBinder&& rhs)
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
ECSqlStatus ArrayECSqlBinder::JsonValueBinder::_BindNull()
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
ECSqlStatus ArrayECSqlBinder::JsonValueBinder::_BindBoolean(bool value)
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
ECSqlStatus ArrayECSqlBinder::JsonValueBinder::_BindBlob(void const* value, int binarySize, IECSqlBinder::MakeCopy makeCopy)
    {
    ECSqlStatus stat = FailIfTypeMismatch(PRIMITIVETYPE_Binary);
    if (!stat.IsSuccess())
        return stat;

    Byte const* blob = static_cast<Byte const*> (value);
    return SUCCESS == JsonPersistenceHelper::BinaryToJson(*m_json, blob, binarySize, *m_jsonAllocator) ? ECSqlStatus::Success : ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//---------------------------------------------------------------------------------------
ECSqlStatus ArrayECSqlBinder::JsonValueBinder::_BindZeroBlob(int blobSize)
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
ECSqlStatus ArrayECSqlBinder::JsonValueBinder::_BindDateTime(double julianDay, DateTime::Info const& metadata)
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
ECSqlStatus ArrayECSqlBinder::JsonValueBinder::_BindDateTime(uint64_t julianDayMsec, DateTime::Info const& metadata)
    {
    const double jd = DateTime::MsecToRationalDay(julianDayMsec);
    return _BindDateTime(jd, metadata);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//---------------------------------------------------------------------------------------
ECSqlStatus ArrayECSqlBinder::JsonValueBinder::_BindDouble(double value)
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
ECSqlStatus ArrayECSqlBinder::JsonValueBinder::_BindInt(int value)
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
ECSqlStatus ArrayECSqlBinder::JsonValueBinder::_BindInt64(int64_t value)
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
ECSqlStatus ArrayECSqlBinder::JsonValueBinder::_BindPoint2d(DPoint2dCR value)
    {
    ECSqlStatus stat = FailIfTypeMismatch(PRIMITIVETYPE_Point2d);
    if (!stat.IsSuccess())
        return stat;

    return SUCCESS == JsonPersistenceHelper::Point2dToJson(*m_json, value, *m_jsonAllocator) ? ECSqlStatus::Success : ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//---------------------------------------------------------------------------------------
ECSqlStatus ArrayECSqlBinder::JsonValueBinder::_BindPoint3d(DPoint3dCR value)
    {
    ECSqlStatus stat = FailIfTypeMismatch(PRIMITIVETYPE_Point3d);
    if (!stat.IsSuccess())
        return stat;

    return SUCCESS == JsonPersistenceHelper::Point3dToJson(*m_json, value, *m_jsonAllocator) ? ECSqlStatus::Success : ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//---------------------------------------------------------------------------------------
ECSqlStatus ArrayECSqlBinder::JsonValueBinder::_BindText(Utf8CP value, IECSqlBinder::MakeCopy makeCopy, int stringLength)
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
IECSqlBinder& ArrayECSqlBinder::JsonValueBinder::_BindStructMember(Utf8CP structMemberPropertyName)
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
IECSqlBinder& ArrayECSqlBinder::JsonValueBinder::_BindStructMember(ECN::ECPropertyId structMemberPropertyId)
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
IECSqlBinder& ArrayECSqlBinder::JsonValueBinder::CreateStructMemberBinder(ECN::ECPropertyCR memberProp)
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
IECSqlBinder& ArrayECSqlBinder::JsonValueBinder::_AddArrayElement()
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
IECSqlBinder& ArrayECSqlBinder::JsonValueBinder::MoveCurrentArrayElementBinder(ECDbCR ecdb, ECSqlTypeInfo const& arrayTypeInfo)
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
        m_currentArrayElementBinder = std::make_unique<JsonValueBinder>(ecdb, ECSqlTypeInfo(arrayTypeInfo.GetPrimitiveType(), false, &arrayTypeInfo.GetDateTimeInfo()), newArrayElementJson, *m_jsonAllocator);
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
void ArrayECSqlBinder::JsonValueBinder::Reset()
    {
    m_json->SetNull();
    m_structMemberBinders.clear();
    m_currentArrayElementBinder = nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      01/2017
//---------------------------------------------------------------------------------------
ECSqlStatus ArrayECSqlBinder::JsonValueBinder::FailIfTypeMismatch(ECN::PrimitiveType boundType) const
    {
    ECSqlStatus stat = FailIfInvalid();
    if (!stat.IsSuccess())
        return stat;

    if (!m_typeInfo.IsPrimitive())
        {
        LOG.error("Type mismatch. Primitive values can only be bound to primitive parameter values.");
        return ECSqlStatus::Error;
        }

    //For DateTimes and Geometry column type and BindXXX type must match. All other types are implicitly
    //converted to each other by SQLite.
    const PrimitiveType binderDataType = m_typeInfo.GetPrimitiveType();
    switch (binderDataType)
        {
            case PRIMITIVETYPE_DateTime:
            {
            if (boundType != PRIMITIVETYPE_DateTime)
                {
                LOG.error("Type mismatch: only BindDateTime can be called for a column of the DateTime type.");
                return ECSqlStatus::Error;
                }
            else
                break;
            }
            case PRIMITIVETYPE_IGeometry:
            {
            if (boundType != PRIMITIVETYPE_IGeometry && boundType != PRIMITIVETYPE_Binary)
                {
                LOG.error("Type mismatch: only BindGeometry or BindBlob can be called for a column of the IGeometry type.");
                return ECSqlStatus::Error;
                }
            else
                break;
            }

            case PRIMITIVETYPE_Point2d:
            {
            if (boundType != PRIMITIVETYPE_Point2d)
                {
                LOG.error("Type mismatch: only BindPoint2d can be called for a column of the Point2d type.");
                return ECSqlStatus::Error;
                }
            else
                break;
            }

            case PRIMITIVETYPE_Point3d:
            {
            if (boundType != PRIMITIVETYPE_Point3d)
                {
                LOG.error("Type mismatch: only BindPoint3d can be called for a column of the Point3d type.");
                return ECSqlStatus::Error;
                }
            else
                break;
            }

            default:
                break;
        }

    switch (boundType)
        {
            case PRIMITIVETYPE_DateTime:
            {
            if (PRIMITIVETYPE_DateTime != binderDataType)
                {
                LOG.error("Type mismatch: BindDateTime cannot be called for a column which is not of the DateTime type.");
                return ECSqlStatus::Error;
                }
            else
                break;
            }
            case PRIMITIVETYPE_IGeometry:
            {
            if (PRIMITIVETYPE_IGeometry != binderDataType)
                {
                LOG.error("Type mismatch: BindGeometry cannot be called for a column which is not of the IGeometry type.");
                return ECSqlStatus::Error;
                }
            else
                break;
            }
            case PRIMITIVETYPE_Point2d:
            {
            if (binderDataType != PRIMITIVETYPE_Point2d)
                {
                LOG.error("Type mismatch: BindPoint2d cannot be called for a column which is not of the Point2d type.");
                return ECSqlStatus::Error;
                }
            else
                break;
            }
            case PRIMITIVETYPE_Point3d:
            {
            if (binderDataType != PRIMITIVETYPE_Point3d)
                {
                LOG.error("Type mismatch: BindPoint3d cannot be called for a column which is not of the Point3d type.");
                return ECSqlStatus::Error;
                }
            else
                break;
            }

            default:
                break;
        }

    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//---------------------------------------------------------------------------------------
ECSqlStatus ArrayECSqlBinder::JsonValueBinder::FailIfInvalid() const
    {
    if (m_json == nullptr)
        {
        LOG.error("Cannot bind values to struct array binder without calling IECSqlBinder::AddArrayElement first.");
        return ECSqlStatus::Error;
        }

    return ECSqlStatus::Success;
    }



END_BENTLEY_SQLITE_EC_NAMESPACE