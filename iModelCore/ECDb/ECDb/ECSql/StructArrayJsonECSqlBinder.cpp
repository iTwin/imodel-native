/*------------------
--------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/StructArrayJsonECSqlBinder.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "StructArrayJsonECSqlBinder.h"
#include "PrimitiveToSingleColumnECSqlBinder.h"
#include <GeomSerialization/GeomLibsSerialization.h>
#include <GeomSerialization/GeomLibsJsonSerialization.h>
#include <GeomSerialization/GeomLibsFlatBufferApi.h>

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
std::unique_ptr<JsonECSqlBinder> JsonECSqlBinderFactory::Create(ECDbCR ecdb, ECPropertyCR prop, bool isRoot)
    {
    ECSqlTypeInfo typeInfo(prop);
    Utf8CP propName = prop.GetName().c_str();
    if (typeInfo.IsPrimitive())
        return CreatePrimitive(ecdb, typeInfo, propName, isRoot);

    if (typeInfo.IsStruct())
        return CreateStruct(ecdb, typeInfo, propName, isRoot);

    BeAssert(typeInfo.IsArray());
    return CreateArray(ecdb, typeInfo, propName, isRoot);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//---------------------------------------------------------------------------------------
//static
std::unique_ptr<JsonECSqlBinder> JsonECSqlBinderFactory::CreateStruct(ECDbCR ecdb, ECSqlTypeInfo const& typeInfo, Utf8CP propName, bool isRoot)
    {
    return std::unique_ptr<JsonECSqlBinder>(new StructJsonECSqlBinder(ecdb, typeInfo, propName, isRoot));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//---------------------------------------------------------------------------------------
//static
std::unique_ptr<JsonECSqlBinder> JsonECSqlBinderFactory::CreatePrimitive(ECDbCR ecdb, ECSqlTypeInfo const& typeInfo, Utf8CP propName, bool isRoot)
    {
    return std::unique_ptr<JsonECSqlBinder>(new PrimitiveJsonECSqlBinder(ecdb, typeInfo, propName, isRoot));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      04/2016
//---------------------------------------------------------------------------------------
//static
std::unique_ptr<ArrayJsonECSqlBinder> JsonECSqlBinderFactory::CreateArray(ECDbCR ecdb, ECSqlTypeInfo const& typeInfo, Utf8CP propName, bool isRoot)
    {
    return std::unique_ptr<ArrayJsonECSqlBinder>(new ArrayJsonECSqlBinder(ecdb, typeInfo, propName, isRoot));
    }

//*****************************************************
// JsonECSqlBinder
//*****************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//---------------------------------------------------------------------------------------
ECSqlStatus JsonECSqlBinder::BuildJson(Json::Value& json) const
    {
    Json::Value& val = IgnorePropertyNameInJson() ? json : json[m_propertyName];
    ECSqlStatus stat = _BuildJson(val);
    DEBUG_JSON("", json);
    return stat;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//---------------------------------------------------------------------------------------
IECSqlPrimitiveBinder& JsonECSqlBinder::_BindPrimitive()
    {
    GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Type mismatch. Cannot bind primitive value to non-primitive parameter.");
    return NoopECSqlBinderFactory::GetBinder(ECSqlStatus::Error).BindPrimitive();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//---------------------------------------------------------------------------------------
IECSqlStructBinder& JsonECSqlBinder::_BindStruct()
    {
    GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Type mismatch. Cannot bind struct value to non-struct parameter.");
    return NoopECSqlBinderFactory::GetBinder(ECSqlStatus::Error).BindStruct();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//---------------------------------------------------------------------------------------
IECSqlArrayBinder& JsonECSqlBinder::_BindArray(uint32_t initialCapacity)
    {
    GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Type mismatch. Cannot bind array value to non-array parameter.");
    return NoopECSqlBinderFactory::GetBinder(ECSqlStatus::Error).BindArray(initialCapacity);
    }

//*****************************************************
// PrimitiveJsonECSqlBinder
//*****************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//---------------------------------------------------------------------------------------
ECSqlStatus PrimitiveJsonECSqlBinder::_BindNull()
    {
    _Clear();
    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//---------------------------------------------------------------------------------------
ECSqlStatus PrimitiveJsonECSqlBinder::_BindBoolean(bool value)
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
ECSqlStatus PrimitiveJsonECSqlBinder::_BindBinary(const void* value, int binarySize, IECSqlBinder::MakeCopy makeCopy)
    {
    if (GetTypeInfo().GetPrimitiveType() != PRIMITIVETYPE_Binary)
        {
        GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Type mismatch. Blob values can only be bound to Blob parameter values.");
        return ECSqlStatus::Error;
        }

    //JSON_TODO
    return ECSqlStatus::Error;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//---------------------------------------------------------------------------------------
ECSqlStatus PrimitiveJsonECSqlBinder::_BindDateTime(double julianDay, DateTime::Info const* metadata)
    {
    if (GetTypeInfo().GetPrimitiveType() != PRIMITIVETYPE_DateTime)
        {
        GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Type mismatch. Date time values can only be bound to date time parameter values.");
        return ECSqlStatus::Error;
        }

    if (metadata != nullptr && metadata->GetKind() == DateTime::Kind::Local)
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
ECSqlStatus PrimitiveJsonECSqlBinder::_BindDateTime(uint64_t julianDayHns, DateTime::Info const* metadata)
    {
    const double jd = DateTime::HnsToRationalDay(julianDayHns);
    return _BindDateTime(jd, metadata);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//---------------------------------------------------------------------------------------
ECSqlStatus PrimitiveJsonECSqlBinder::_BindDouble(double value)
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
ECSqlStatus PrimitiveJsonECSqlBinder::_BindGeometryBlob(const void* value, int blobSize, IECSqlBinder::MakeCopy makeCopy)
    {
    if (GetTypeInfo().GetPrimitiveType() != PRIMITIVETYPE_IGeometry)
        {
        GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Type mismatch. IGeometry values can only be bound to IGeometry parameter values.");
        return ECSqlStatus::Error;
        }

    Byte const* blob = static_cast<Byte const*> (value);
    BeAssert(blobSize > 0);
    const size_t blobSizeU = (size_t) blobSize;
    bvector<Byte> byteVec;
    byteVec.reserve(blobSizeU);
    byteVec.assign(blob, blob + blobSizeU);
    if (!BentleyGeometryFlatBuffer::IsFlatBufferFormat(byteVec))
        {
        GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "BindGeometry");
        return ECSqlStatus::Error;
        }

    IGeometryPtr geom = BentleyGeometryFlatBuffer::BytesToGeometry(byteVec);
    if (geom == nullptr)
        {
        BeAssert(geom != nullptr);
        return ECSqlStatus::Error;
        }

    if (!BentleyGeometryJson::TryGeometryToJsonValue(m_value, *geom, false))
        {
        GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "BindGeometry");
        return ECSqlStatus::Error;
        }

    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//---------------------------------------------------------------------------------------
ECSqlStatus PrimitiveJsonECSqlBinder::_BindInt(int value)
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
ECSqlStatus PrimitiveJsonECSqlBinder::_BindInt64(int64_t value)
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
ECSqlStatus PrimitiveJsonECSqlBinder::_BindPoint2D(DPoint2dCR value)
    {
    if (GetTypeInfo().GetPrimitiveType() != PRIMITIVETYPE_Point2D)
        {
        GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Type mismatch. Point2D values can only be bound to Point2D parameter values.");
        return ECSqlStatus::Error;
        }

    Json::Value pointJsonValue(Json::objectValue);
    pointJsonValue["x"] = value.x;
    pointJsonValue["y"] = value.y;

    m_value = pointJsonValue;
    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//---------------------------------------------------------------------------------------
ECSqlStatus PrimitiveJsonECSqlBinder::_BindPoint3D(DPoint3dCR value)
    {
    if (GetTypeInfo().GetPrimitiveType() != PRIMITIVETYPE_Point3D)
        {
        GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Type mismatch. Point3D values can only be bound to Point3D parameter values.");
        return ECSqlStatus::Error;
        }

    Json::Value pointJsonValue(Json::objectValue);
    pointJsonValue["x"] = value.x;
    pointJsonValue["y"] = value.y;
    pointJsonValue["z"] = value.z;

    m_value = pointJsonValue;
    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//---------------------------------------------------------------------------------------
ECSqlStatus PrimitiveJsonECSqlBinder::_BindText(Utf8CP value, IECSqlBinder::MakeCopy makeCopy, int byteCount)
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
ECSqlStatus PrimitiveJsonECSqlBinder::_BuildJson(Json::Value& json) const
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
StructJsonECSqlBinder::StructJsonECSqlBinder(ECDbCR ecdb, ECSqlTypeInfo const& typeInfo, Utf8CP propName, bool isRoot)
    : JsonECSqlBinder(ecdb, typeInfo, propName, isRoot), IECSqlStructBinder()
    {
    //initialize member parameter values only once and reuse them after each ClearBindings
    for (ECPropertyCP memberProp : GetTypeInfo().GetStructType().GetProperties(true))
        {
        m_members[memberProp->GetId()] = JsonECSqlBinderFactory::Create(ecdb, *memberProp, false);
        }
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//---------------------------------------------------------------------------------------
ECSqlStatus StructJsonECSqlBinder::_BindNull()
    {
    Clear();
    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//---------------------------------------------------------------------------------------
IECSqlBinder& StructJsonECSqlBinder::_GetMember(Utf8CP structMemberPropertyName)
    {
    ECPropertyCP memberProp = GetTypeInfo().GetStructType().GetPropertyP(structMemberPropertyName, true);
    if (memberProp == nullptr)
        {
        GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Cannot bind to struct member. Member %s does not exist.", structMemberPropertyName);
        BeAssert(false);
        return NoopECSqlBinderFactory::GetBinder(ECSqlStatus::Error);
        }

    return _GetMember(memberProp->GetId());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//---------------------------------------------------------------------------------------
IECSqlBinder& StructJsonECSqlBinder::_GetMember(ECN::ECPropertyId structMemberPropertyId)
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
    return NoopECSqlBinderFactory::GetBinder(ECSqlStatus::Error);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//---------------------------------------------------------------------------------------
ECSqlStatus StructJsonECSqlBinder::_BuildJson(Json::Value& json) const
    {
    for (ECPropertyCP prop : GetTypeInfo().GetStructType().GetProperties(true))
        {
        auto it = m_members.find(prop->GetId());
        if (it == m_members.end())
            continue;

        JsonECSqlBinder& member = *it->second;

        ECSqlStatus stat = member.BuildJson(json);
        if (!stat.IsSuccess())
            return stat;

        }

    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//---------------------------------------------------------------------------------------
void StructJsonECSqlBinder::_Clear()
    {
    for (std::pair<const ECPropertyId, std::unique_ptr<JsonECSqlBinder>>& kvPair : m_members)
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
IECSqlArrayBinder& ArrayJsonECSqlBinder::_BindArray(uint32_t initialCapacity)
    {
    Clear();
    return *this;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//---------------------------------------------------------------------------------------
ECSqlStatus ArrayJsonECSqlBinder::_BindNull()
    {
    Clear();
    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//---------------------------------------------------------------------------------------
IECSqlBinder& ArrayJsonECSqlBinder::_AddArrayElement()
    {
    const size_t newIndex = GetLength() + 1;
    const ECSqlStatus stat = ArrayConstraintValidator::ValidateMaximum(GetECDb(), GetTypeInfo(), (uint32_t) newIndex);
    if (!stat.IsSuccess())
        return NoopECSqlBinderFactory::GetBinder(stat).AddArrayElement();

    //array elements are added to the JSON without their prop names
    std::unique_ptr<JsonECSqlBinder> arrayElement = nullptr;
    if (GetTypeInfo().GetKind() == ECSqlTypeInfo::Kind::PrimitiveArray)
        arrayElement = JsonECSqlBinderFactory::CreatePrimitive(GetECDb(), ECSqlTypeInfo(GetTypeInfo().GetPrimitiveType()), nullptr, false);
    else
        arrayElement = JsonECSqlBinderFactory::CreateStruct(GetECDb(), ECSqlTypeInfo(GetTypeInfo().GetStructType()), nullptr, false);

    IECSqlBinder* arrayElementP = arrayElement.get(); //cache raw pointer to return it as smart pointer will be nulled out after push_back
    m_elements.push_back(std::move(arrayElement));
    return *arrayElementP;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//---------------------------------------------------------------------------------------
ECSqlStatus ArrayJsonECSqlBinder::_BuildJson(Json::Value& json) const
    {
    for (std::unique_ptr<JsonECSqlBinder>& element : m_elements)
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
StructArrayJsonECSqlBinder::StructArrayJsonECSqlBinder(ECSqlStatementBase& stmt, ECSqlTypeInfo const& typeInfo)
    : ECSqlBinder(stmt, typeInfo, 1, true, true), IECSqlArrayBinder()
    {
    BeAssert(GetTypeInfo().GetKind() == ECSqlTypeInfo::Kind::StructArray);
    //top-level prop doesn't get a prop name in the JSON
    m_binder = JsonECSqlBinderFactory::CreateArray(*stmt.GetECDb(), typeInfo, nullptr, true);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//---------------------------------------------------------------------------------------
ECSqlStatus StructArrayJsonECSqlBinder::_OnBeforeStep()
    {
    ECSqlStatus stat = ArrayConstraintValidator::Validate(GetECDb(), GetTypeInfo(), (uint32_t) m_binder->GetLength());
    if (!stat.IsSuccess())
        return stat;

    PrimitiveToSingleColumnECSqlBinder jsonBinder(GetECSqlStatementR(), ECSqlTypeInfo(PRIMITIVETYPE_String));
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
IECSqlPrimitiveBinder& StructArrayJsonECSqlBinder::_BindPrimitive()
    {
    GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Type mismatch. Cannot bind primitive value to struct array parameter.");
    return NoopECSqlBinderFactory::GetBinder(ECSqlStatus::Error).BindPrimitive();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//---------------------------------------------------------------------------------------
IECSqlStructBinder& StructArrayJsonECSqlBinder::_BindStruct()
    {
    GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Type mismatch. Cannot bind struct value to struct array parameter.");
    return NoopECSqlBinderFactory::GetBinder(ECSqlStatus::Error).BindStruct();
    }

END_BENTLEY_SQLITE_EC_NAMESPACE