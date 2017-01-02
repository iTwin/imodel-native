/*------------------
--------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/StructArrayECSqlBinder.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include <GeomSerialization/GeomLibsSerialization.h>
#include <GeomSerialization/GeomLibsJsonSerialization.h>
#include <GeomSerialization/GeomLibsFlatBufferApi.h>

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//#define DEBUG_JSON(header,json) {Json::FastWriter writer; printf("%s: %s\r\n", header, writer.write(json).c_str()); }
#define DEBUG_JSON(header,json)
// ************************************************
// JsonECSqlBinderFactory
// ************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
//static
std::unique_ptr<JsonECSqlBindValue> JsonECSqlBindValueFactory::CreateValue(ECDbCR ecdb, ECPropertyCR prop, bool isRoot)
    {
    ECSqlTypeInfo typeInfo(prop);
    Utf8CP propName = prop.GetName().c_str();
    if (typeInfo.IsPrimitive())
        return CreatePrimitiveValue(ecdb, typeInfo, propName, isRoot);

    if (typeInfo.IsStruct())
        return CreateStructValue(ecdb, typeInfo, propName, isRoot);

    BeAssert(typeInfo.IsArray());
    return CreateArrayValue(ecdb, typeInfo, propName, isRoot);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//---------------------------------------------------------------------------------------
//static
std::unique_ptr<JsonECSqlBindValue> JsonECSqlBindValueFactory::CreateStructValue(ECDbCR ecdb, ECSqlTypeInfo const& typeInfo, Utf8CP propName, bool isRoot)
    {
    return std::unique_ptr<JsonECSqlBindValue>(new StructJsonECSqlBindValue(ecdb, typeInfo, propName, isRoot));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//---------------------------------------------------------------------------------------
//static
std::unique_ptr<JsonECSqlBindValue> JsonECSqlBindValueFactory::CreatePrimitiveValue(ECDbCR ecdb, ECSqlTypeInfo const& typeInfo, Utf8CP propName, bool isRoot)
    {
    return std::unique_ptr<JsonECSqlBindValue>(new PrimitiveJsonECSqlBindValue(ecdb, typeInfo, propName, isRoot));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      04/2016
//---------------------------------------------------------------------------------------
//static
std::unique_ptr<ArrayJsonECSqlBindValue> JsonECSqlBindValueFactory::CreateArrayValue(ECDbCR ecdb, ECSqlTypeInfo const& typeInfo, Utf8CP propName, bool isRoot)
    {
    return std::unique_ptr<ArrayJsonECSqlBindValue>(new ArrayJsonECSqlBindValue(ecdb, typeInfo, propName, isRoot));
    }

//*****************************************************
// JsonECSqlBinder
//*****************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//---------------------------------------------------------------------------------------
ECSqlStatus JsonECSqlBindValue::BuildJson(Json::Value& json) const
    {
    Json::Value& val = IgnorePropertyNameInJson() ? json : json[m_propertyName];
    ECSqlStatus stat = _BuildJson(val);
    DEBUG_JSON("", json);
    return stat;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//---------------------------------------------------------------------------------------
IECSqlPrimitiveBinder& JsonECSqlBindValue::_BindPrimitive()
    {
    GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Type mismatch. Cannot bind primitive value to non-primitive parameter.");
    return NoopECSqlBinder::Get().BindPrimitive();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//---------------------------------------------------------------------------------------
IECSqlStructBinder& JsonECSqlBindValue::_BindStruct()
    {
    GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Type mismatch. Cannot bind struct value to non-struct parameter.");
    return NoopECSqlBinder::Get().BindStruct();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//---------------------------------------------------------------------------------------
IECSqlArrayBinder& JsonECSqlBindValue::_BindArray(uint32_t initialCapacity)
    {
    GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Type mismatch. Cannot bind array value to non-array parameter.");
    return NoopECSqlBinder::Get().BindArray(initialCapacity);
    }

//*****************************************************
// PrimitiveJsonECSqlBinder
//*****************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//---------------------------------------------------------------------------------------
ECSqlStatus PrimitiveJsonECSqlBindValue::_BindNull()
    {
    _Clear();
    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//---------------------------------------------------------------------------------------
ECSqlStatus PrimitiveJsonECSqlBindValue::_BindBoolean(bool value)
    {
    if (GetTypeInfo().GetPrimitiveType() != PRIMITIVETYPE_Boolean)
        {
        GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Type mismatch. Boolean value can only be bound to boolean parameter values.");
        return ECSqlStatus::Error;
        }

    m_value = Json::Value(value);
    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//---------------------------------------------------------------------------------------
ECSqlStatus PrimitiveJsonECSqlBindValue::_BindBlob(void const* value, int binarySize, IECSqlBinder::MakeCopy makeCopy)
    {
    if (GetTypeInfo().GetPrimitiveType() != PRIMITIVETYPE_Binary && GetTypeInfo().GetPrimitiveType() != PRIMITIVETYPE_IGeometry)
        {
        GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Type mismatch. Blob values can only be bound to Blob or IGeometry parameter values.");
        return ECSqlStatus::Error;
        }

    Byte const* blob = static_cast<Byte const*> (value);
    return SUCCESS == ECJsonUtilities::BinaryToJson(m_value, blob, binarySize) ? ECSqlStatus::Success : ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//---------------------------------------------------------------------------------------
ECSqlStatus PrimitiveJsonECSqlBindValue::_BindZeroBlob(int blobSize)
    {
    GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Type mismatch. Cannot bind Zeroblob values to elements of a struct array parameter.");
    return ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//---------------------------------------------------------------------------------------
ECSqlStatus PrimitiveJsonECSqlBindValue::_BindDateTime(double julianDay, DateTime::Info const& metadata)
    {
    if (GetTypeInfo().GetPrimitiveType() != PRIMITIVETYPE_DateTime)
        {
        GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Type mismatch. Date time values can only be bound to date time parameter values.");
        return ECSqlStatus::Error;
        }

    if (metadata.IsValid() && metadata.GetKind() == DateTime::Kind::Local)
        {
        GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "ECDb does not support to bind local date times.");
        return ECSqlStatus::Error;
        }

    ECSqlTypeInfo const& parameterTypeInfo = GetTypeInfo();
    if (!parameterTypeInfo.DateTimeInfoMatches(metadata))
        {
        GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Metadata of DateTime value to bind doesn't match the metadata on the corresponding ECProperty.");
        return ECSqlStatus::Error;
        }

    m_value = Json::Value(julianDay);
    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//---------------------------------------------------------------------------------------
ECSqlStatus PrimitiveJsonECSqlBindValue::_BindDateTime(uint64_t julianDayMsec, DateTime::Info const& metadata)
    {
    const double jd = DateTime::MsecToRationalDay(julianDayMsec);
    return _BindDateTime(jd, metadata);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//---------------------------------------------------------------------------------------
ECSqlStatus PrimitiveJsonECSqlBindValue::_BindDouble(double value)
    {
    if (GetTypeInfo().GetPrimitiveType() != PRIMITIVETYPE_Double)
        {
        GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Type mismatch. Double values can only be bound to double parameter values.");
        return ECSqlStatus::Error;
        }

    m_value = Json::Value(value);
    return ECSqlStatus::Success;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//---------------------------------------------------------------------------------------
ECSqlStatus PrimitiveJsonECSqlBindValue::_BindInt(int value)
    {
    if (GetTypeInfo().GetPrimitiveType() != PRIMITIVETYPE_Integer)
        {
        GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Type mismatch. Integer values can only be bound to integer parameter values.");
        return ECSqlStatus::Error;
        }

    m_value = Json::Value(value);
    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//---------------------------------------------------------------------------------------
ECSqlStatus PrimitiveJsonECSqlBindValue::_BindInt64(int64_t value)
    {
    if (GetTypeInfo().GetPrimitiveType() != PRIMITIVETYPE_Long)
        {
        GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Type mismatch. int64_t values can only be bound to int64_t parameter values.");
        return ECSqlStatus::Error;
        }

    m_value = BeJsonUtilities::StringValueFromInt64(value);
    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//---------------------------------------------------------------------------------------
ECSqlStatus PrimitiveJsonECSqlBindValue::_BindPoint2d(DPoint2dCR value)
    {
    if (GetTypeInfo().GetPrimitiveType() != PRIMITIVETYPE_Point2d)
        {
        GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Type mismatch. Point2d values can only be bound to Point2d parameter values.");
        return ECSqlStatus::Error;
        }

    return SUCCESS == ECJsonUtilities::Point2dToJson(m_value, value) ? ECSqlStatus::Success : ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//---------------------------------------------------------------------------------------
ECSqlStatus PrimitiveJsonECSqlBindValue::_BindPoint3d(DPoint3dCR value)
    {
    if (GetTypeInfo().GetPrimitiveType() != PRIMITIVETYPE_Point3d)
        {
        GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Type mismatch. Point3d values can only be bound to Point3d parameter values.");
        return ECSqlStatus::Error;
        }

    return SUCCESS == ECJsonUtilities::Point3dToJson(m_value, value) ? ECSqlStatus::Success : ECSqlStatus::Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//---------------------------------------------------------------------------------------
ECSqlStatus PrimitiveJsonECSqlBindValue::_BindText(Utf8CP value, IECSqlBinder::MakeCopy makeCopy, int byteCount)
    {
    if (GetTypeInfo().GetPrimitiveType() != PRIMITIVETYPE_String)
        {
        GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Type mismatch. String values can only be bound to string parameter values.");
        return ECSqlStatus::Error;
        }

    m_value = Json::Value(value);
    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//---------------------------------------------------------------------------------------
ECSqlStatus PrimitiveJsonECSqlBindValue::_BuildJson(Json::Value& json) const
    {
    json = m_value;
    return ECSqlStatus::Success;
    }

// *****************************************************
// StructJsonECSqlBinder
// *****************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//---------------------------------------------------------------------------------------
StructJsonECSqlBindValue::StructJsonECSqlBindValue(ECDbCR ecdb, ECSqlTypeInfo const& typeInfo, Utf8CP propName, bool isRoot)
    : JsonECSqlBindValue(ecdb, typeInfo, propName, isRoot), IECSqlStructBinder()
    {
    //initialize member parameter values only once and reuse them after each ClearBindings
    for (ECPropertyCP memberProp : GetTypeInfo().GetStructType().GetProperties(true))
        {
        m_members[memberProp->GetId()] = JsonECSqlBindValueFactory::CreateValue(ecdb, *memberProp, false);
        }
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//---------------------------------------------------------------------------------------
ECSqlStatus StructJsonECSqlBindValue::_BindNull()
    {
    Clear();
    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//---------------------------------------------------------------------------------------
IECSqlBinder& StructJsonECSqlBindValue::_GetMember(Utf8CP structMemberPropertyName)
    {
    ECPropertyCP memberProp = GetTypeInfo().GetStructType().GetPropertyP(structMemberPropertyName, true);
    if (memberProp == nullptr)
        {
        GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Cannot bind to struct member. Member %s does not exist.", structMemberPropertyName);
        BeAssert(false);
        return NoopECSqlBinder::Get();
        }

    return _GetMember(memberProp->GetId());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//---------------------------------------------------------------------------------------
IECSqlBinder& StructJsonECSqlBindValue::_GetMember(ECN::ECPropertyId structMemberPropertyId)
    {
    auto it = m_members.find(structMemberPropertyId);
    if (it != m_members.end())
        return *it->second;

    Utf8CP structMemberPropertyName = nullptr;
    for (ECPropertyCP prop : GetTypeInfo().GetStructType().GetProperties(true))
        {
        if (prop->GetId() == structMemberPropertyId)
            {
            structMemberPropertyName = prop->GetName().c_str();
            break;
            }
        }

    GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Cannot bind to struct member. Member %s does not exist.", structMemberPropertyName);
    return NoopECSqlBinder::Get();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//---------------------------------------------------------------------------------------
ECSqlStatus StructJsonECSqlBindValue::_BuildJson(Json::Value& json) const
    {
    for (ECPropertyCP prop : GetTypeInfo().GetStructType().GetProperties(true))
        {
        auto it = m_members.find(prop->GetId());
        if (it == m_members.end())
            continue;

        JsonECSqlBindValue& member = *it->second;

        ECSqlStatus stat = member.BuildJson(json);
        if (!stat.IsSuccess())
            return stat;

        }

    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//---------------------------------------------------------------------------------------
void StructJsonECSqlBindValue::_Clear()
    {
    for (std::pair<const ECPropertyId, std::unique_ptr<JsonECSqlBindValue>>& kvPair : m_members)
        {
        kvPair.second->Clear();
        }
    }

// *****************************************************
// ArrayJsonECSqlBinder
// *****************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//---------------------------------------------------------------------------------------
IECSqlArrayBinder& ArrayJsonECSqlBindValue::_BindArray(uint32_t initialCapacity)
    {
    Clear();
    return *this;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//---------------------------------------------------------------------------------------
ECSqlStatus ArrayJsonECSqlBindValue::_BindNull()
    {
    Clear();
    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//---------------------------------------------------------------------------------------
IECSqlBinder& ArrayJsonECSqlBindValue::_AddArrayElement()
    {
    const size_t newIndex = GetLength() + 1;
    const ECSqlStatus stat = ArrayConstraintValidator::ValidateMaximum(GetECDb(), GetTypeInfo(), (uint32_t) newIndex);
    if (!stat.IsSuccess())
        return NoopECSqlBinder::Get().AddArrayElement();

    //array elements are added to the JSON without their prop names
    std::unique_ptr<JsonECSqlBindValue> arrayElement = nullptr;
    if (GetTypeInfo().GetKind() == ECSqlTypeInfo::Kind::PrimitiveArray)
        arrayElement = JsonECSqlBindValueFactory::CreatePrimitiveValue(GetECDb(), ECSqlTypeInfo(GetTypeInfo().GetPrimitiveType()), nullptr, false);
    else
        arrayElement = JsonECSqlBindValueFactory::CreateStructValue(GetECDb(), ECSqlTypeInfo(GetTypeInfo().GetStructType()), nullptr, false);

    IECSqlBinder* arrayElementP = arrayElement.get(); //cache raw pointer to return it as smart pointer will be nulled out after push_back
    m_elements.push_back(std::move(arrayElement));
    return *arrayElementP;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//---------------------------------------------------------------------------------------
ECSqlStatus ArrayJsonECSqlBindValue::_BuildJson(Json::Value& json) const
    {
    for (std::unique_ptr<JsonECSqlBindValue>& element : m_elements)
        {
        Json::Value elementJson;
        ECSqlStatus stat = element->BuildJson(elementJson);
        if (!stat.IsSuccess())
            return stat;

        json.append(elementJson);
        }

    return ECSqlStatus::Success;
    }


// *****************************************************
// StructArrayJsonECSqlBinder
// *****************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//---------------------------------------------------------------------------------------
StructArrayECSqlBinder::StructArrayECSqlBinder(ECSqlStatementBase& stmt, ECSqlTypeInfo const& typeInfo)
    : ECSqlBinder(stmt, typeInfo, 1, true, true), IECSqlArrayBinder()
    {
    BeAssert(GetTypeInfo().GetKind() == ECSqlTypeInfo::Kind::StructArray);
    //top-level prop doesn't get a prop name in the JSON
    m_binder = JsonECSqlBindValueFactory::CreateArrayValue(*stmt.GetECDb(), typeInfo, nullptr, true);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//---------------------------------------------------------------------------------------
ECSqlStatus StructArrayECSqlBinder::_OnBeforeStep()
    {
    ECSqlStatus stat = ArrayConstraintValidator::Validate(GetECDb(), GetTypeInfo(), (uint32_t) m_binder->GetLength());
    if (!stat.IsSuccess())
        return stat;

    PrimitiveECSqlBinder jsonBinder(GetECSqlStatementR(), ECSqlTypeInfo(PRIMITIVETYPE_String));
    jsonBinder.SetSqliteIndex(m_sqliteIndex);

    if (m_binder->GetLength() == 0)
        return jsonBinder.BindNull();

    Json::Value structArrayJson(Json::arrayValue);
    stat = m_binder->BuildJson(structArrayJson);
    if (!stat.IsSuccess())
        return stat;

    DEBUG_JSON("Final", structArrayJson);

    Json::FastWriter writer;
    Utf8String jsonStr = writer.write(structArrayJson);
    return jsonBinder.BindText(jsonStr.c_str(), IECSqlBinder::MakeCopy::Yes);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//---------------------------------------------------------------------------------------
IECSqlPrimitiveBinder& StructArrayECSqlBinder::_BindPrimitive()
    {
    GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Type mismatch. Cannot bind primitive value to struct array parameter.");
    return NoopECSqlBinder::Get().BindPrimitive();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//---------------------------------------------------------------------------------------
IECSqlStructBinder& StructArrayECSqlBinder::_BindStruct()
    {
    GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Type mismatch. Cannot bind struct value to struct array parameter.");
    return NoopECSqlBinder::Get().BindStruct();
    }

END_BENTLEY_SQLITE_EC_NAMESPACE