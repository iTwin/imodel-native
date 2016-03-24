/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/StructArrayJsonECSqlField.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include <GeomSerialization/GeomLibsSerialization.h>
#include <GeomSerialization/GeomLibsJsonSerialization.h>
#include <GeomSerialization/GeomSerializationApi.h>

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//************************************************
// JsonECSqlFactory
//************************************************

//-----------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//+---------------+---------------+---------------+---------------+---------------+------
std::unique_ptr<JsonECSqlValue> JsonECSqlValueFactory::CreateValue(ECDbCR ecdb, Json::Value const& json, ECSqlColumnInfo const& columnInfo)
    {
    ECTypeDescriptor const& dataType = columnInfo.GetDataType();
    if (dataType.IsPrimitive())
        {
        DateTime::Info dateTimeMetadata;
        bool hasDateTimeMetadata = false;
        if (dataType.GetPrimitiveType() == PRIMITIVETYPE_DateTime)
            {
            ECPropertyCP property = columnInfo.GetProperty();
            BeAssert(property != nullptr && "ColumnInfo::GetProperty can return null. Please double-check");
            DateTimeInfo dateTimeInfo;
            if (StandardCustomAttributeHelper::GetDateTimeInfo(dateTimeInfo, *property) != ECObjectsStatus::Success)
                {
                ecdb.GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Could not read DateTimeInfo custom attribute from the corresponding ECProperty.");
                BeAssert(false && "Could not read DateTimeInfo custom attribute from the corresponding ECProperty.");
                return nullptr;
                }

            dateTimeMetadata = dateTimeInfo.GetInfo(true);
            hasDateTimeMetadata = true;
            }

        return std::unique_ptr<JsonECSqlValue>(new PrimitiveJsonECSqlValue(ecdb, json, columnInfo, hasDateTimeMetadata ? &dateTimeMetadata : nullptr));
        }

    if (dataType.IsStruct())
        {
        if (columnInfo.GetProperty() == nullptr || !columnInfo.GetProperty()->GetIsStruct())
            {
            BeAssert(false);
            return nullptr;
            }

        ECStructClassCR structClass = columnInfo.GetProperty()->GetAsStructProperty()->GetType();
        return std::unique_ptr<JsonECSqlValue>(new StructJsonECSqlValue(ecdb, json, columnInfo, structClass));
        }

    return std::unique_ptr<JsonECSqlValue>(new ArrayJsonECSqlValue(ecdb, json, columnInfo));
    }


//-----------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//+---------------+---------------+---------------+---------------+---------------+------
std::unique_ptr<JsonECSqlValue> JsonECSqlValueFactory::CreateArrayElementValue(ECDbCR ecdb, Json::Value const& json, ECSqlColumnInfo const& columnInfo, DateTime::Info const* dateTimeMetadata, ECN::ECStructClassCP structClass)
    {
    if (columnInfo.GetDataType().IsStruct())
        {
        if (structClass == nullptr)
            {
            BeAssert(structClass != nullptr);
            return nullptr;
            }

        return std::unique_ptr<JsonECSqlValue>(new StructJsonECSqlValue(ecdb, json, columnInfo, *structClass));
        }

    if (columnInfo.GetDataType().IsPrimitive())
        return std::unique_ptr<JsonECSqlValue>(new PrimitiveJsonECSqlValue(ecdb, json, columnInfo, dateTimeMetadata));

    return CreateValue(ecdb, json, columnInfo);
    }

//************************************************
// JsonECSqlValue
//************************************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//+---------------+---------------+---------------+---------------+---------------+------
bool JsonECSqlValue::_IsNull() const
    {
    return GetJson().isNull();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlPrimitiveValue const& JsonECSqlValue::_GetPrimitive() const
    {
    ReportError("Type mismatch. Cannot call primitive IECSqlValue getters on non-primitive IECSqlValue.");
    return NoopECSqlValue::GetSingleton().GetPrimitive();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlStructValue const& JsonECSqlValue::_GetStruct() const
    {
    ReportError("Type mismatch. Cannot call GetStruct on non-struct IECSqlValue.");
    return NoopECSqlValue::GetSingleton().GetStruct();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlArrayValue const& JsonECSqlValue::_GetArray() const
    {
    ReportError("Type mismatch. Cannot call GetArray on non-array IECSqlValue.");
    return NoopECSqlValue::GetSingleton().GetArray();
    }

//************************************************
// PrimitiveJsonECSqlValue
//************************************************
PrimitiveJsonECSqlValue::PrimitiveJsonECSqlValue(ECDbCR ecdb, Json::Value const& json, ECSqlColumnInfo const& columnInfo, DateTime::Info const* dateTimeMetadata) 
    : JsonECSqlValue(ecdb, json, columnInfo), IECSqlPrimitiveValue()
    {
    if (dateTimeMetadata != nullptr)
        m_datetimeMetadata = *dateTimeMetadata;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//+---------------+---------------+---------------+---------------+---------------+------
void const* PrimitiveJsonECSqlValue::_GetBinary(int* binarySize) const
    {
    if (GetJson().isNull() || !CanCallGetFor(PRIMITIVETYPE_Binary))
        return NoopECSqlValue::GetSingleton().GetBinary(binarySize);

    if (SUCCESS != ECJsonUtilities::JsonToBinary(m_blobCache, GetJson()))
        {
        Utf8String msg;
        msg.Sprintf("IECSqlValue::GetBinary failed for '%s'. Invalid JSON format for Blob.",
                    GetColumnInfo().GetPropertyPath().ToString().c_str());
        ReportError(msg.c_str());
        return NoopECSqlValue::GetSingleton().GetBinary(binarySize);
        }

    return m_blobCache.data();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//+---------------+---------------+---------------+---------------+---------------+------
bool PrimitiveJsonECSqlValue::_GetBoolean() const
    {
    if (GetJson().isNull() || !CanCallGetFor(PRIMITIVETYPE_Boolean))
        return NoopECSqlValue::GetSingleton().GetBoolean();

    return GetJson().asBool();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//+---------------+---------------+---------------+---------------+---------------+------
uint64_t PrimitiveJsonECSqlValue::_GetDateTimeJulianDaysHns(DateTime::Info& metadata) const
    {
    metadata = m_datetimeMetadata;

    if (GetJson().isNull() || !CanCallGetFor(PRIMITIVETYPE_DateTime))
        return NoopECSqlValue::GetSingleton().GetDateTimeJulianDaysHns(metadata);

    const double jd = _GetDateTimeJulianDays(metadata);
    return DateTime::RationalDayToHns(jd);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//+---------------+---------------+---------------+---------------+---------------+------
double PrimitiveJsonECSqlValue::_GetDateTimeJulianDays(DateTime::Info& metadata) const
    {
    metadata = m_datetimeMetadata;

    if (GetJson().isNull() || !CanCallGetFor(PRIMITIVETYPE_DateTime))
        return NoopECSqlValue::GetSingleton().GetDateTimeJulianDays(metadata);

    return GetJson().asDouble();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//+---------------+---------------+---------------+---------------+---------------+------
double PrimitiveJsonECSqlValue::_GetDouble() const
    {
    if (GetJson().isNull() || !CanCallGetFor(PRIMITIVETYPE_Double))
        return NoopECSqlValue::GetSingleton().GetDouble();

    return GetJson().asDouble();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//+---------------+---------------+---------------+---------------+---------------+------
int PrimitiveJsonECSqlValue::_GetInt() const
    {
    if (GetJson().isNull() || !CanCallGetFor(PRIMITIVETYPE_Integer))
        return NoopECSqlValue::GetSingleton().GetInt();

    return GetJson().asInt();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//+---------------+---------------+---------------+---------------+---------------+------
int64_t PrimitiveJsonECSqlValue::_GetInt64() const
    {
    if (GetJson().isNull() || !CanCallGetFor(PRIMITIVETYPE_Long))
        return NoopECSqlValue::GetSingleton().GetInt64();

    return BeJsonUtilities::Int64FromValue(GetJson());
    }


//-----------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//+---------------+---------------+---------------+---------------+---------------+------
Utf8CP PrimitiveJsonECSqlValue::_GetText() const
    {
    if (GetJson().isNull() || !CanCallGetFor(PRIMITIVETYPE_String))
        return NoopECSqlValue::GetSingleton().GetText();

    return GetJson().asCString();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//+---------------+---------------+---------------+---------------+---------------+------
DPoint2d PrimitiveJsonECSqlValue::_GetPoint2D() const
    {
    if (GetJson().isNull() || !CanCallGetFor(PRIMITIVETYPE_Point2D))
        return NoopECSqlValue::GetSingleton().GetPoint2D();

    DPoint2d pt;
    if (SUCCESS != ECJsonUtilities::JsonToPoint2D(pt, GetJson()))
        {
        Utf8String msg;
        msg.Sprintf("IECSqlValue::GetPoint2D failed for '%s'. Invalid JSON format for Point2D.",
                    GetColumnInfo().GetPropertyPath().ToString().c_str());
        ReportError(msg.c_str());
        return NoopECSqlValue::GetSingleton().GetPoint2D();
        }

    return pt;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//+---------------+---------------+---------------+---------------+---------------+------
DPoint3d PrimitiveJsonECSqlValue::_GetPoint3D() const
    {
    if (GetJson().isNull() || !CanCallGetFor(PRIMITIVETYPE_Point3D))
        return NoopECSqlValue::GetSingleton().GetPoint3D();

    DPoint3d pt;
    if (SUCCESS != ECJsonUtilities::JsonToPoint3D(pt, GetJson()))
        {
        Utf8String msg;
        msg.Sprintf("IECSqlValue::GetPoint3D failed for '%s'. Invalid JSON format for Point3D.",
                    GetColumnInfo().GetPropertyPath().ToString().c_str());
        ReportError(msg.c_str());
        return NoopECSqlValue::GetSingleton().GetPoint3D();
        }

    return pt;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//+---------------+---------------+---------------+---------------+---------------+------
IGeometryPtr PrimitiveJsonECSqlValue::_GetGeometry() const
    {
    if (GetJson().isNull() || !CanCallGetFor(PRIMITIVETYPE_IGeometry))
        return NoopECSqlValue::GetSingleton().GetGeometry();

    bvector<IGeometryPtr> geoms;
    if (!BentleyGeometryJson::TryJsonValueToGeometry(GetJson(), geoms) || geoms.size() != 1)
        {
        Utf8String msg;
        msg.Sprintf("IECSqlValue::GetGeometry failed for '%s'. Invalid JSON format for IGeometry.",
                    GetColumnInfo().GetPropertyPath().ToString().c_str());
        ReportError(msg.c_str());
        return NoopECSqlValue::GetSingleton().GetGeometry();
        }

    return geoms[0];
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//+---------------+---------------+---------------+---------------+---------------+------
void const* PrimitiveJsonECSqlValue::_GetGeometryBlob(int* blobSize) const
    {
    if (GetJson().isNull() || !CanCallGetFor(PRIMITIVETYPE_IGeometry))
        return NoopECSqlValue::GetSingleton().GetGeometryBlob(blobSize);

    IGeometryPtr geom = _GetGeometry();

    //As the API contract says that IECSqlValue owns the geometry blob, we need to cache the blob
    //in this object
    BentleyGeometryFlatBuffer::GeometryToBytes(*geom, m_blobCache);
    if (m_blobCache.empty())
        {
        Utf8String msg;
        msg.Sprintf("IECSqlValue::GetGeometryBlob failed for '%s'. Invalid JSON format for IGeometry.",
                    GetColumnInfo().GetPropertyPath().ToString().c_str());
        ReportError(msg.c_str());
        return NoopECSqlValue::GetSingleton().GetGeometryBlob(blobSize);
        }

    return m_blobCache.data();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//+---------------+---------------+---------------+---------------+---------------+------
bool PrimitiveJsonECSqlValue::CanCallGetFor(ECN::PrimitiveType getMethodType) const
    {
    const PrimitiveType actualDataType = GetColumnInfo().GetDataType().GetPrimitiveType();
    if (actualDataType != getMethodType)
        {
        Utf8String msg;
        msg.Sprintf("Type mismatch: Cannot call IECSqlValue::%s for the struct array member '%s' which is of type '%s'.",
                    ECSqlField::GetPrimitiveGetMethodName(getMethodType), GetColumnInfo().GetPropertyPath().ToString().c_str(),
                    ExpHelper::ToString(actualDataType));
        ReportError(msg.c_str());
        return false;
        }

    return true;
    }

//************************************************
// JsonECSqlStructValue
//************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//---------------------------------------------------------------------------------------
StructJsonECSqlValue::StructJsonECSqlValue(ECDbCR ecdb, Json::Value const& json, ECSqlColumnInfo const& columnInfo, ECN::ECStructClassCR structClass)
    : JsonECSqlValue(ecdb, json, columnInfo), IECSqlStructValue()
    {
    for (ECPropertyCP structMemberProp : structClass.GetProperties(true))
        {
        Json::Value const& val = json[structMemberProp->GetName().c_str()];
        ECSqlColumnInfo memberColInfo = ECSqlColumnInfo::CreateChild(GetColumnInfo(), *structMemberProp);
        m_members.push_back(JsonECSqlValueFactory::CreateValue(ecdb, val, memberColInfo));
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//---------------------------------------------------------------------------------------
IECSqlValue const& StructJsonECSqlValue::_GetValue(int columnIndex) const
    {
    if (columnIndex < 0 && columnIndex >= (int) m_members.size())
        {
        Utf8String msg;
        msg.Sprintf("IECSqlStructValue::GetValue failed for '%s': Index %d is out of bounds.",
                    GetColumnInfo().GetPropertyPath().ToString().c_str(),  columnIndex);
        ReportError(msg.c_str());
        return NoopECSqlValue::GetSingleton();
        }

    return *m_members[(size_t) columnIndex];
    }

//************************************************
// JsonECSqlArrayValue
//************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//---------------------------------------------------------------------------------------
ArrayJsonECSqlValue::ArrayJsonECSqlValue(ECDbCR ecdb, Json::Value const& json, ECSqlColumnInfo const& columnInfo)
    : JsonECSqlValue(ecdb, json, columnInfo), IECSqlArrayValue(), m_jsonIterator(json.end()), m_currentElement(nullptr), m_structArrayElementType(nullptr)
    {
    BeAssert(GetJson().isArray());

    ECTypeDescriptor const& dataType = GetColumnInfo().GetDataType();
    DateTime::Info dateTimeMetadata;
    if (dataType.IsStructArray())
        {
        BeAssert(GetColumnInfo().GetProperty() != nullptr && "ColumnInfo::GetProperty should not return null for array property");
        m_structArrayElementType = GetColumnInfo().GetProperty()->GetAsStructArrayProperty()->GetStructElementType();
        }
    else if (dataType.IsPrimitiveArray() && dataType.GetPrimitiveType() == PRIMITIVETYPE_DateTime)
        {
        DateTimeInfo dateTimeInfo;
        if (StandardCustomAttributeHelper::GetDateTimeInfo(dateTimeInfo, *GetColumnInfo().GetProperty()) == ECObjectsStatus::Success)
            m_primitiveArrayDatetimeMetadata = dateTimeInfo.GetInfo(true);
        else
            {
            Utf8String msg;
            msg.Sprintf("IECSqlValue::GetArray failed for '%s': Could not read DateTimeInfo custom attribute from the corresponding primitive array ECProperty. DateTimeInfo will be ignored.",
                        GetColumnInfo().GetPropertyPath().ToString().c_str());
            ReportError(msg.c_str());
            }
        }
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//---------------------------------------------------------------------------------------
void ArrayJsonECSqlValue::_MoveNext(bool onInitializingIterator) const
    {
    if (onInitializingIterator)
        m_jsonIterator = GetJson().begin();
    else
        m_jsonIterator++;

    if (_IsAtEnd())
        {
        m_currentElement = nullptr;
        return;
        }

    ECSqlColumnInfo elementColumnInfo = ECSqlColumnInfo::CreateForArrayElement(GetColumnInfo(), (int) m_jsonIterator.index());
    

    m_currentElement = JsonECSqlValueFactory::CreateArrayElementValue(GetECDb(), *m_jsonIterator, elementColumnInfo, &m_primitiveArrayDatetimeMetadata, m_structArrayElementType);
    BeAssert(m_currentElement != nullptr);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//---------------------------------------------------------------------------------------
bool ArrayJsonECSqlValue::_IsAtEnd() const
    {
    return m_jsonIterator == GetJson().end();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//---------------------------------------------------------------------------------------
IECSqlValue const* ArrayJsonECSqlValue::_GetCurrent() const
    {
    return m_currentElement.get();
    }

//************************************************
// StructArrayJsonECSqlField
//************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//---------------------------------------------------------------------------------------
StructArrayJsonECSqlField::StructArrayJsonECSqlField(ECSqlStatementBase& ecsqlStatement, ECSqlColumnInfo&& ecsqlColumnInfo, int sqliteColumnIndex)
    : ECSqlField(ecsqlStatement, std::move(ecsqlColumnInfo), true, true), m_sqliteColumnIndex(sqliteColumnIndex), m_json(Json::arrayValue), m_value(nullptr)
    {}

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      02/2016
//---------------------------------------------------------------------------------------
ECSqlStatus StructArrayJsonECSqlField::_OnAfterStep()
    {
    DoReset();

    BeAssert(m_json.isArray());

    if (GetSqliteStatement().IsColumnNull(m_sqliteColumnIndex))
        m_json = Json::Value(Json::arrayValue);
    else
        {
        Utf8String jsonStr(GetSqliteStatement().GetValueText(m_sqliteColumnIndex));
        Json::Reader reader;
        if (!reader.Parse(jsonStr, m_json, false))
            return ReportError(ECSqlStatus::Error, "Could not deserialize struct array JSON.");
        }

    BeAssert(m_json.isArray());
    m_value = std::unique_ptr<ArrayJsonECSqlValue>(new ArrayJsonECSqlValue(*GetECSqlStatementR().GetECDb(), m_json, GetColumnInfo()));
    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      02/2016
//---------------------------------------------------------------------------------------
ECSqlStatus StructArrayJsonECSqlField::_OnAfterReset()
    {
    DoReset();
    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      02/2016
//---------------------------------------------------------------------------------------
void StructArrayJsonECSqlField::DoReset() const
    {
    m_json.clear();
    m_value = nullptr;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlPrimitiveValue const& StructArrayJsonECSqlField::_GetPrimitive() const
    {
    ReportError(ECSqlStatus::Error, "Type mismatch. Cannot call primitive IECSqlValue getters on struct array IECSqlValue.");
    return NoopECSqlValue::GetSingleton().GetPrimitive();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlStructValue const& StructArrayJsonECSqlField::_GetStruct() const
    {
    ReportError(ECSqlStatus::Error, "Type mismatch. Cannot call GetStruct on struct array IECSqlValue.");
    return NoopECSqlValue::GetSingleton().GetStruct();
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
