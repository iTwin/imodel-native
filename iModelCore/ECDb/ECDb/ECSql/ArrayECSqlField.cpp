/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ArrayECSqlField.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
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
std::unique_ptr<JsonECSqlValue> JsonECSqlValueFactory::CreateValue(ECDbCR ecdb, rapidjson::Value const& json, ECSqlColumnInfo const& columnInfo)
    {
    ECTypeDescriptor const& dataType = columnInfo.GetDataType();
    if (dataType.IsPrimitive())
        {
        DateTime::Info dateTimeMetadata;
        if (dataType.GetPrimitiveType() == PRIMITIVETYPE_DateTime)
            {
            ECPropertyCP property = columnInfo.GetProperty();
            BeAssert(property != nullptr && "ColumnInfo::GetProperty can return null. Please double-check");
            if (StandardCustomAttributeHelper::GetDateTimeInfo(dateTimeMetadata, *property) != ECObjectsStatus::Success)
                {
                LOG.error("Could not read DateTimeInfo custom attribute from the corresponding ECProperty.");
                BeAssert(false && "Could not read DateTimeInfo custom attribute from the corresponding ECProperty.");
                return nullptr;
                }

            if (!dateTimeMetadata.IsValid())
                dateTimeMetadata = DateTime::Info::CreateForDateTime(DateTime::Kind::Unspecified); //default
            }

        return std::unique_ptr<JsonECSqlValue>(new PrimitiveJsonECSqlValue(ecdb, json, columnInfo, dateTimeMetadata));
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
std::unique_ptr<JsonECSqlValue> JsonECSqlValueFactory::CreateArrayElementValue(ECDbCR ecdb, rapidjson::Value const& json, ECSqlColumnInfo const& columnInfo, DateTime::Info const& dateTimeMetadata, ECN::ECStructClassCP structClass)
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
// @bsimethod                                                Krischan.Eberle      01/2017
//+---------------+---------------+---------------+---------------+---------------+------
//static
rapidjson::Value const* JsonECSqlValue::s_nullJson = nullptr;

//-----------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//+---------------+---------------+---------------+---------------+---------------+------
bool JsonECSqlValue::_IsNull() const { return GetJson().IsNull(); }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlPrimitiveValue const& JsonECSqlValue::_GetPrimitive() const
    {
    LOG.error("Type mismatch. Cannot call primitive IECSqlValue getters on non-primitive IECSqlValue.");
    return NoopECSqlValue::GetSingleton().GetPrimitive();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlStructValue const& JsonECSqlValue::_GetStruct() const
    {
    LOG.error("Type mismatch. Cannot call GetStruct on non-struct IECSqlValue.");
    return NoopECSqlValue::GetSingleton().GetStruct();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      01/2017
//+---------------+---------------+---------------+---------------+---------------+------
//static
rapidjson::Value const& JsonECSqlValue::GetNullJson()
    {
    if (s_nullJson == nullptr)
        s_nullJson = new rapidjson::Value(rapidjson::kNullType);

    return *s_nullJson;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlArrayValue const& JsonECSqlValue::_GetArray() const
    {
    LOG.error("Type mismatch. Cannot call GetArray on non-array IECSqlValue.");
    return NoopECSqlValue::GetSingleton().GetArray();
    }

//************************************************
// PrimitiveJsonECSqlValue
//************************************************
PrimitiveJsonECSqlValue::PrimitiveJsonECSqlValue(ECDbCR ecdb, rapidjson::Value const& json, ECSqlColumnInfo const& columnInfo, DateTime::Info const& dateTimeMetadata)
    : JsonECSqlValue(ecdb, json, columnInfo), IECSqlPrimitiveValue(), m_datetimeMetadata(dateTimeMetadata)
    {}

//-----------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//+---------------+---------------+---------------+---------------+---------------+------
void const* PrimitiveJsonECSqlValue::_GetBlob(int* blobSize) const
    {
    if (GetJson().IsNull() || !CanCallGetFor(PRIMITIVETYPE_Binary))
        return NoopECSqlValue::GetSingleton().GetBlob(blobSize);

    m_blobCache.Resize(0);
    if (SUCCESS != ECRapidJsonUtilities::JsonToBinary(m_blobCache, GetJson()))
        {
        if (LOG.isSeverityEnabled(NativeLogging::LOG_ERROR))
            LOG.errorv("IECSqlValue::GetBlob failed for '%s'. Invalid JSON format for Blob.", GetColumnInfo().GetPropertyPath().ToString().c_str());
        
        return NoopECSqlValue::GetSingleton().GetBlob(blobSize);
        }

    return m_blobCache.data();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//+---------------+---------------+---------------+---------------+---------------+------
bool PrimitiveJsonECSqlValue::_GetBoolean() const
    {
    if (GetJson().IsNull() || !CanCallGetFor(PRIMITIVETYPE_Boolean))
        return NoopECSqlValue::GetSingleton().GetBoolean();

    return GetJson().GetBool();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//+---------------+---------------+---------------+---------------+---------------+------
uint64_t PrimitiveJsonECSqlValue::_GetDateTimeJulianDaysMsec(DateTime::Info& metadata) const
    {
    metadata = m_datetimeMetadata;

    if (GetJson().IsNull() || !CanCallGetFor(PRIMITIVETYPE_DateTime))
        return NoopECSqlValue::GetSingleton().GetDateTimeJulianDaysMsec(metadata);

    const double jd = _GetDateTimeJulianDays(metadata);
    return DateTime::RationalDayToMsec(jd);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//+---------------+---------------+---------------+---------------+---------------+------
double PrimitiveJsonECSqlValue::_GetDateTimeJulianDays(DateTime::Info& metadata) const
    {
    metadata = m_datetimeMetadata;

    if (GetJson().IsNull() || !CanCallGetFor(PRIMITIVETYPE_DateTime))
        return NoopECSqlValue::GetSingleton().GetDateTimeJulianDays(metadata);

    return GetJson().GetDouble();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//+---------------+---------------+---------------+---------------+---------------+------
double PrimitiveJsonECSqlValue::_GetDouble() const
    {
    if (GetJson().IsNull() || !CanCallGetFor(PRIMITIVETYPE_Double))
        return NoopECSqlValue::GetSingleton().GetDouble();

    return GetJson().GetDouble();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//+---------------+---------------+---------------+---------------+---------------+------
int PrimitiveJsonECSqlValue::_GetInt() const
    {
    if (GetJson().IsNull() || !CanCallGetFor(PRIMITIVETYPE_Integer))
        return NoopECSqlValue::GetSingleton().GetInt();

    return GetJson().GetInt();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//+---------------+---------------+---------------+---------------+---------------+------
int64_t PrimitiveJsonECSqlValue::_GetInt64() const
    {
    if (GetJson().IsNull() || !CanCallGetFor(PRIMITIVETYPE_Long))
        return NoopECSqlValue::GetSingleton().GetInt64();

    return GetJson().GetInt64();
    }


//-----------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//+---------------+---------------+---------------+---------------+---------------+------
Utf8CP PrimitiveJsonECSqlValue::_GetText() const
    {
    if (GetJson().IsNull() || !CanCallGetFor(PRIMITIVETYPE_String))
        return NoopECSqlValue::GetSingleton().GetText();

    return GetJson().GetString();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//+---------------+---------------+---------------+---------------+---------------+------
DPoint2d PrimitiveJsonECSqlValue::_GetPoint2d() const
    {
    if (GetJson().IsNull() || !CanCallGetFor(PRIMITIVETYPE_Point2d))
        return NoopECSqlValue::GetSingleton().GetPoint2d();

    DPoint2d pt;
    if (SUCCESS != ECRapidJsonUtilities::JsonToPoint2d(pt, GetJson()))
        {
        if (LOG.isSeverityEnabled(NativeLogging::LOG_ERROR))
            LOG.errorv("IECSqlValue::GetPoint2d failed for '%s'. Invalid JSON format for Point2d.", GetColumnInfo().GetPropertyPath().ToString().c_str());

        return NoopECSqlValue::GetSingleton().GetPoint2d();
        }

    return pt;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//+---------------+---------------+---------------+---------------+---------------+------
DPoint3d PrimitiveJsonECSqlValue::_GetPoint3d() const
    {
    if (GetJson().IsNull() || !CanCallGetFor(PRIMITIVETYPE_Point3d))
        return NoopECSqlValue::GetSingleton().GetPoint3d();

    DPoint3d pt;
    if (SUCCESS != ECRapidJsonUtilities::JsonToPoint3d(pt, GetJson()))
        {
        if (LOG.isSeverityEnabled(NativeLogging::LOG_ERROR))
            LOG.errorv("IECSqlValue::GetPoint3d failed for '%s'. Invalid JSON format for Point3d.", GetColumnInfo().GetPropertyPath().ToString().c_str());

        return NoopECSqlValue::GetSingleton().GetPoint3d();
        }

    return pt;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//+---------------+---------------+---------------+---------------+---------------+------
IGeometryPtr PrimitiveJsonECSqlValue::_GetGeometry() const
    {
    if (GetJson().IsNull() || !CanCallGetFor(PRIMITIVETYPE_IGeometry))
        return NoopECSqlValue::GetSingleton().GetGeometry();

    int blobSize = -1;
    void const* fbBlob = _GetBlob(&blobSize);

    return BentleyGeometryFlatBuffer::BytesToGeometry((Byte const*) fbBlob);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//+---------------+---------------+---------------+---------------+---------------+------
bool PrimitiveJsonECSqlValue::CanCallGetFor(ECN::PrimitiveType getMethodType) const
    {
    const PrimitiveType actualDataType = GetColumnInfo().GetDataType().GetPrimitiveType();
    if (getMethodType == PRIMITIVETYPE_Binary)
        {
        if (actualDataType == PRIMITIVETYPE_Binary || actualDataType == PRIMITIVETYPE_IGeometry)
            return true;
        }

    if (actualDataType != getMethodType)
        {
        if (LOG.isSeverityEnabled(NativeLogging::LOG_ERROR))
            LOG.errorv("Type mismatch: Cannot call IECSqlValue::%s for the struct array member '%s' which is of type '%s'.",
                    ECSqlField::GetPrimitiveGetMethodName(getMethodType), GetColumnInfo().GetPropertyPath().ToString().c_str(),
                    ExpHelper::ToString(actualDataType));

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
StructJsonECSqlValue::StructJsonECSqlValue(ECDbCR ecdb, rapidjson::Value const& json, ECSqlColumnInfo const& columnInfo, ECN::ECStructClassCR structClass)
    : JsonECSqlValue(ecdb, json, columnInfo), IECSqlStructValue()
    {
    for (ECPropertyCP structMemberProp : structClass.GetProperties(true))
        {
        rapidjson::Value const* memberJson = &GetNullJson();

        if (!json.IsNull())
            {
            BeAssert(json.IsObject());
            rapidjson::Value::ConstMemberIterator it = json.FindMember(structMemberProp->GetName().c_str());
            if (it != json.MemberEnd())
                memberJson = &it->value;
            }

        ECSqlColumnInfo memberColInfo = ECSqlColumnInfo::CreateChild(GetColumnInfo(), *structMemberProp);
        m_members.push_back(JsonECSqlValueFactory::CreateValue(ecdb, *memberJson, memberColInfo));
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      01/2017
//---------------------------------------------------------------------------------------
bool StructJsonECSqlValue::_IsNull() const
    {
    for (std::unique_ptr<JsonECSqlValue> const& member : m_members)
        {
        if (!member->IsNull())
            return false;
        }

    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//---------------------------------------------------------------------------------------
IECSqlValue const& StructJsonECSqlValue::_GetValue(int columnIndex) const
    {
    if (columnIndex < 0 && columnIndex >= (int) m_members.size())
        {
        if (LOG.isSeverityEnabled(NativeLogging::LOG_ERROR))
            LOG.errorv("IECSqlStructValue::GetValue failed for '%s': Index %d is out of bounds.",
                    GetColumnInfo().GetPropertyPath().ToString().c_str(),  columnIndex);

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
ArrayJsonECSqlValue::ArrayJsonECSqlValue(ECDbCR ecdb, rapidjson::Value const& json, ECSqlColumnInfo const& columnInfo)
    : JsonECSqlValue(ecdb, json, columnInfo), IECSqlArrayValue(), m_jsonIteratorIndex(-1), m_currentElement(nullptr), m_structArrayElementType(nullptr)
    {
    if (json.IsNull())
        return;

    BeAssert(GetJson().IsArray());

    ECTypeDescriptor const& dataType = GetColumnInfo().GetDataType();
    if (dataType.IsStructArray())
        {
        BeAssert(GetColumnInfo().GetProperty() != nullptr && "ColumnInfo::GetProperty should not return null for array property");
        m_structArrayElementType = &GetColumnInfo().GetProperty()->GetAsStructArrayProperty()->GetStructElementType();
        }
    else if (dataType.IsPrimitiveArray() && dataType.GetPrimitiveType() == PRIMITIVETYPE_DateTime)
        {
        if (StandardCustomAttributeHelper::GetDateTimeInfo(m_primitiveArrayDatetimeMetadata, *GetColumnInfo().GetProperty()) != ECObjectsStatus::Success)
            {
            if (LOG.isSeverityEnabled(NativeLogging::LOG_ERROR))
                LOG.errorv("IECSqlValue::GetArray failed for '%s': Could not read DateTimeInfo custom attribute from the corresponding primitive array ECProperty. DateTimeInfo will be ignored.",
                        GetColumnInfo().GetPropertyPath().ToString().c_str());
            }

        if (!m_primitiveArrayDatetimeMetadata.IsValid())
            m_primitiveArrayDatetimeMetadata = DateTime::Info::CreateForDateTime(DateTime::Kind::Unspecified); // default
        }
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//---------------------------------------------------------------------------------------
void ArrayJsonECSqlValue::_MoveNext(bool onInitializingIterator) const
    {
    if (onInitializingIterator)
        {
        if (GetJson().IsNull())
            {
            BeAssert(m_currentElement == nullptr);
            return;
            }

        m_jsonIterator = GetJson().Begin();
        m_jsonIteratorIndex = 0;
        }
    else
        {
        m_jsonIterator++;
        m_jsonIteratorIndex++;
        }

    if (_IsAtEnd())
        {
        m_currentElement = nullptr;
        return;
        }

    ECSqlColumnInfo elementColumnInfo = ECSqlColumnInfo::CreateForArrayElement(GetColumnInfo(), m_jsonIteratorIndex);
    
    m_currentElement = JsonECSqlValueFactory::CreateArrayElementValue(GetECDb(), *m_jsonIterator, elementColumnInfo, m_primitiveArrayDatetimeMetadata, m_structArrayElementType);
    BeAssert(m_currentElement != nullptr);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//---------------------------------------------------------------------------------------
bool ArrayJsonECSqlValue::_IsAtEnd() const
    {
    if (GetJson().IsNull())
        return true;

    BeAssert(GetJson().IsArray());
    return m_jsonIterator == GetJson().End();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//---------------------------------------------------------------------------------------
IECSqlValue const* ArrayJsonECSqlValue::_GetCurrent() const { return m_currentElement.get(); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      01/2017
//---------------------------------------------------------------------------------------
int ArrayJsonECSqlValue::_GetArrayLength() const
    {
    if (GetJson().IsNull()) 
        return 0; 
    
    return (int) GetJson().Size();
    }

//************************************************
// StructArrayJsonECSqlField
//************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//---------------------------------------------------------------------------------------
ArrayECSqlField::ArrayECSqlField(ECSqlStatementBase& ecsqlStatement, ECSqlColumnInfo const& ecsqlColumnInfo, int sqliteColumnIndex)
    : ECSqlField(ecsqlStatement, ecsqlColumnInfo, true, true), m_sqliteColumnIndex(sqliteColumnIndex), m_json(rapidjson::kArrayType), m_value(nullptr)
    {}

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      02/2016
//---------------------------------------------------------------------------------------
ECSqlStatus ArrayECSqlField::_OnAfterStep()
    {
    DoReset();

    if (GetSqliteStatement().IsColumnNull(m_sqliteColumnIndex))
        m_json.SetArray();
    else
        {
        Utf8CP jsonStr = GetSqliteStatement().GetValueText(m_sqliteColumnIndex);
        if (m_json.Parse<0>(jsonStr).HasParseError())
            {
            LOG.error("Could not deserialize struct array JSON.");
            return ECSqlStatus::Error;
            }
        }

    BeAssert(m_json.IsArray());
    m_value = std::unique_ptr<ArrayJsonECSqlValue>(new ArrayJsonECSqlValue(*GetECSqlStatementR().GetECDb(), m_json, GetColumnInfo()));
    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      02/2016
//---------------------------------------------------------------------------------------
ECSqlStatus ArrayECSqlField::_OnAfterReset()
    {
    DoReset();
    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      02/2016
//---------------------------------------------------------------------------------------
void ArrayECSqlField::DoReset() const
    {
    m_json.Clear();
    m_value = nullptr;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlPrimitiveValue const& ArrayECSqlField::_GetPrimitive() const
    {
    LOG.error("Type mismatch. Cannot call primitive IECSqlValue getters on struct array IECSqlValue.");
    return NoopECSqlValue::GetSingleton().GetPrimitive();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlStructValue const& ArrayECSqlField::_GetStruct() const
    {
    LOG.error("Type mismatch. Cannot call GetStruct on struct array IECSqlValue.");
    return NoopECSqlValue::GetSingleton().GetStruct();
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
