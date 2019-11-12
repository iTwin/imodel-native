/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include <GeomSerialization/GeomLibsSerialization.h>
#include <GeomSerialization/GeomLibsJsonSerialization.h>
#include <GeomSerialization/GeomSerializationApi.h>

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//************************************************
// ArrayECSqlField
//************************************************

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
            LOG.errorv("Could not deserialize struct array JSON: '%s'", jsonStr);
            return ECSqlStatus::Error;
            }
        }

    BeAssert(m_json.IsArray());
    m_value = std::make_unique<JsonECSqlValue>(m_preparedECSqlStatement.GetECDb(), m_json, GetColumnInfo());
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


//************************************************
// ArrayECSqlField::JsonECSqlValue
//************************************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      01/2017
//+---------------+---------------+---------------+---------------+---------------+------
//static
rapidjson::Value const* ArrayECSqlField::JsonECSqlValue::s_nullJson = nullptr;

//-----------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      02/2017
//+---------------+---------------+---------------+---------------+---------------+------
ArrayECSqlField::JsonECSqlValue::JsonECSqlValue(ECDbCR ecdb, rapidjson::Value const& json, ECSqlColumnInfo const& columnInfo)
    : IECSqlValue(), m_ecdb(ecdb), m_json(json), m_columnInfo(columnInfo) {}

//-----------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//+---------------+---------------+---------------+---------------+---------------+------
bool ArrayECSqlField::JsonECSqlValue::_IsNull() const
    {
    IECSqlValueIterable const* iterable = nullptr;
    if (m_columnInfo.GetDataType().IsArray())
        iterable = &GetArrayIterable();
    else if (m_columnInfo.GetDataType().IsStruct())
        iterable = &GetStructIterable();

    if (iterable == nullptr)
        return m_json.IsNull();

    for (IECSqlValue const& val : *iterable)
        {
        if (!val.IsNull())
            return false;
        }

    return true;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//+---------------+---------------+---------------+---------------+---------------+------
void const* ArrayECSqlField::JsonECSqlValue::_GetBlob(int* blobSize) const
    {
    if (m_json.IsNull() || !CanCallGetFor(PRIMITIVETYPE_Binary))
        return NoopECSqlValue::GetSingleton().GetBlob(blobSize);

    m_blobCache.Resize(0);
    if (SUCCESS != JsonPersistenceHelper::JsonToBinary(m_blobCache, m_json))
        {
        if (LOG.isSeverityEnabled(NativeLogging::LOG_ERROR))
            LOG.errorv("IECSqlValue::GetBlob failed for '%s'. Invalid JSON format for Blob.", m_columnInfo.GetPropertyPath().ToString().c_str());

        return NoopECSqlValue::GetSingleton().GetBlob(blobSize);
        }

    if (blobSize != nullptr)
        *blobSize = (int) m_blobCache.size();

    return m_blobCache.data();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//+---------------+---------------+---------------+---------------+---------------+------
bool ArrayECSqlField::JsonECSqlValue::_GetBoolean() const
    {
    if (m_json.IsNull() || !CanCallGetFor(PRIMITIVETYPE_Boolean))
        return NoopECSqlValue::GetSingleton().GetBoolean();

    return m_json.GetBool();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//+---------------+---------------+---------------+---------------+---------------+------
uint64_t ArrayECSqlField::JsonECSqlValue::_GetDateTimeJulianDaysMsec(DateTime::Info& metadata) const
    {
    const double jd = _GetDateTimeJulianDays(metadata);
    return DateTime::RationalDayToMsec(jd);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//+---------------+---------------+---------------+---------------+---------------+------
double ArrayECSqlField::JsonECSqlValue::_GetDateTimeJulianDays(DateTime::Info& metadata) const
    {
    if (m_json.IsNull() || !CanCallGetFor(PRIMITIVETYPE_DateTime))
        return NoopECSqlValue::GetSingleton().GetDateTimeJulianDays(metadata);

    if (m_columnInfo.GetDateTimeInfo().IsValid())
        metadata = m_columnInfo.GetDateTimeInfo();
    else
        metadata = DateTime::Info::CreateForDateTime(DateTime::Kind::Unspecified); //default

    return m_json.GetDouble();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//+---------------+---------------+---------------+---------------+---------------+------
double ArrayECSqlField::JsonECSqlValue::_GetDouble() const
    {
    if (m_json.IsNull() || !CanCallGetFor(PRIMITIVETYPE_Double))
        return NoopECSqlValue::GetSingleton().GetDouble();

    return m_json.GetDouble();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//+---------------+---------------+---------------+---------------+---------------+------
int ArrayECSqlField::JsonECSqlValue::_GetInt() const
    {
    if (m_json.IsNull() || !CanCallGetFor(PRIMITIVETYPE_Integer))
        return NoopECSqlValue::GetSingleton().GetInt();

    return m_json.GetInt();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//+---------------+---------------+---------------+---------------+---------------+------
int64_t ArrayECSqlField::JsonECSqlValue::_GetInt64() const
    {
    if (m_json.IsNull() || !CanCallGetFor(PRIMITIVETYPE_Long))
        return NoopECSqlValue::GetSingleton().GetInt64();

    return m_json.GetInt64();
    }


//-----------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//+---------------+---------------+---------------+---------------+---------------+------
Utf8CP ArrayECSqlField::JsonECSqlValue::_GetText() const
    {
    if (m_json.IsNull() || !CanCallGetFor(PRIMITIVETYPE_String))
        return NoopECSqlValue::GetSingleton().GetText();

    return m_json.GetString();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//+---------------+---------------+---------------+---------------+---------------+------
DPoint2d ArrayECSqlField::JsonECSqlValue::_GetPoint2d() const
    {
    if (m_json.IsNull() || !CanCallGetFor(PRIMITIVETYPE_Point2d))
        return NoopECSqlValue::GetSingleton().GetPoint2d();

    DPoint2d pt;
    if (SUCCESS != JsonPersistenceHelper::JsonToPoint2d(pt, m_json))
        {
        if (LOG.isSeverityEnabled(NativeLogging::LOG_ERROR))
            LOG.errorv("IECSqlValue::GetPoint2d failed for '%s'. Invalid JSON format for Point2d.", m_columnInfo.GetPropertyPath().ToString().c_str());

        return NoopECSqlValue::GetSingleton().GetPoint2d();
        }

    return pt;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//+---------------+---------------+---------------+---------------+---------------+------
DPoint3d ArrayECSqlField::JsonECSqlValue::_GetPoint3d() const
    {
    if (m_json.IsNull() || !CanCallGetFor(PRIMITIVETYPE_Point3d))
        return NoopECSqlValue::GetSingleton().GetPoint3d();

    DPoint3d pt;
    if (SUCCESS != JsonPersistenceHelper::JsonToPoint3d(pt, m_json))
        {
        if (LOG.isSeverityEnabled(NativeLogging::LOG_ERROR))
            LOG.errorv("IECSqlValue::GetPoint3d failed for '%s'. Invalid JSON format for Point3d.", m_columnInfo.GetPropertyPath().ToString().c_str());

        return NoopECSqlValue::GetSingleton().GetPoint3d();
        }

    return pt;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//+---------------+---------------+---------------+---------------+---------------+------
IGeometryPtr ArrayECSqlField::JsonECSqlValue::_GetGeometry() const
    {
    if (m_json.IsNull() || !CanCallGetFor(PRIMITIVETYPE_IGeometry))
        return NoopECSqlValue::GetSingleton().GetGeometry();

    int blobSize = -1;
    void const* fbBlob = _GetBlob(&blobSize);

    return BentleyGeometryFlatBuffer::BytesToGeometry((Byte const*) fbBlob);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//---------------------------------------------------------------------------------------
IECSqlValue const& ArrayECSqlField::JsonECSqlValue::_GetStructMemberValue(Utf8CP memberName) const
    {
    if (!m_columnInfo.GetDataType().IsStruct())
        {
        LOG.error("IECSqlValue[] can only be called on a struct IECSqlValue.");
        return NoopECSqlValue::GetSingleton()[memberName];
        }

    auto it = m_structMemberCache.find(memberName);
    if (it != m_structMemberCache.end())
        return *it->second;

    ECStructClassCP structType = m_columnInfo.GetStructType();
    if (structType == nullptr)
        {
        BeAssert(false);
        return NoopECSqlValue::GetSingleton()[memberName];
        }

    ECPropertyCP memberProp = structType->GetPropertyP(memberName);
    if (memberProp == nullptr)
        {
        if (LOG.isSeverityEnabled(NativeLogging::LOG_ERROR))
            LOG.errorv("IECSqlValue[] failed for '%s': struct member name %s does not exist.",
                       m_columnInfo.GetPropertyPath().ToString().c_str(), memberName);
        return NoopECSqlValue::GetSingleton()[memberName];
        }

    return CreateStructMemberValue(*memberProp);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//---------------------------------------------------------------------------------------
IECSqlValueIterable const& ArrayECSqlField::JsonECSqlValue::_GetStructIterable() const
    {
    if (!m_columnInfo.GetDataType().IsStruct())
        {
        LOG.error("IECSqlValue::GetStructIterable can only be called on a struct IECSqlValue.");
        return NoopECSqlValue::GetSingleton().GetStructIterable();
        }

    return *this;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      01/2017
//---------------------------------------------------------------------------------------
int ArrayECSqlField::JsonECSqlValue::_GetArrayLength() const
    {
    if (!m_columnInfo.GetDataType().IsArray())
        {
        LOG.error("IECSqlValue::GetArrayLength can only be called on an array IECSqlValue.");
        return NoopECSqlValue::GetSingleton().GetArrayLength();
        }

    if (m_json.IsNull())
        return 0;

    return (int) m_json.Size();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//---------------------------------------------------------------------------------------
IECSqlValueIterable const& ArrayECSqlField::JsonECSqlValue::_GetArrayIterable() const
    {
    if (!m_columnInfo.GetDataType().IsArray())
        {
        LOG.error("IECSqlValue::GetArrayIterable can only be called on an array IECSqlValue.");
        return NoopECSqlValue::GetSingleton().GetArrayIterable();
        }

    return *this;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//---------------------------------------------------------------------------------------
IECSqlValueIterable::const_iterator ArrayECSqlField::JsonECSqlValue::_CreateIterator() const
    {
    ECTypeDescriptor const& dataType = m_columnInfo.GetDataType();
    if (dataType.IsArray())
        return IECSqlValueIterable::const_iterator(std::make_unique<ArrayIteratorState>(*this));

    if (dataType.IsStruct())
        {
        ECStructClassCP structType = m_columnInfo.GetStructType();
        BeAssert(structType != nullptr);
        return IECSqlValueIterable::const_iterator(std::make_unique<StructIteratorState>(*this, structType->GetProperties()));
        }

    BeAssert(false && "should have been caught before.");
    return end();
    }


//-----------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlValue const& ArrayECSqlField::JsonECSqlValue::CreateStructMemberValue(ECN::ECPropertyCR memberProp) const
    {
    Utf8CP memberName = memberProp.GetName().c_str();
    rapidjson::Value const* memberJson = &GetNullJson();
    if (!m_json.IsNull())
        {
        BeAssert(m_json.IsObject());
        rapidjson::Value::ConstMemberIterator jsonIt = m_json.FindMember(memberName);
        if (jsonIt != m_json.MemberEnd())
            memberJson = &jsonIt->value;
        }

    BeAssert(m_structMemberCache.find(memberName) == m_structMemberCache.end());
    ECSqlColumnInfo memberColInfo = ECSqlFieldFactory::CreateChildColumnInfo(m_ecdb.GetImpl().Issues(), m_columnInfo, memberProp, m_ecdb.Schemas().Main().GetSystemSchemaHelper().GetSystemPropertyInfo(memberProp).IsSystemProperty());
    std::unique_ptr<JsonECSqlValue> memberValue = std::make_unique<JsonECSqlValue>(m_ecdb, *memberJson, memberColInfo);
    JsonECSqlValue const* memberValueCP = memberValue.get();
    m_structMemberCache[memberName] = std::move(memberValue);
    return *memberValueCP;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//+---------------+---------------+---------------+---------------+---------------+------
bool ArrayECSqlField::JsonECSqlValue::CanCallGetFor(ECN::PrimitiveType getMethodType) const
    {
    if (!m_columnInfo.GetDataType().IsPrimitive())
        {
        if (LOG.isSeverityEnabled(NativeLogging::LOG_ERROR))
            LOG.errorv("Type mismatch: Cannot call IECSqlValue::%s for the struct array member '%s' which is not of a primitive type.",
                        GetPrimitiveGetMethodName(getMethodType), GetColumnInfo().GetPropertyPath().ToString().c_str());
        return false;
        }

    const PrimitiveType actualDataType = m_columnInfo.GetDataType().GetPrimitiveType();
    if (getMethodType == PRIMITIVETYPE_Binary)
        {
        if (actualDataType == PRIMITIVETYPE_Binary || actualDataType == PRIMITIVETYPE_IGeometry)
            return true;
        }

    if (getMethodType == PRIMITIVETYPE_Long)
        {
        if (actualDataType == PRIMITIVETYPE_Long || actualDataType == PRIMITIVETYPE_Integer)
            return true;
        }

    if (getMethodType == PRIMITIVETYPE_Double)
        {
        if (actualDataType == PRIMITIVETYPE_Double ||actualDataType == PRIMITIVETYPE_Long || actualDataType == PRIMITIVETYPE_Integer)
            return true;
        }

    if (actualDataType != getMethodType)
        {
        if (LOG.isSeverityEnabled(NativeLogging::LOG_ERROR))
            LOG.errorv("Type mismatch: Cannot call IECSqlValue::%s for the struct array member '%s' which is of type '%s'.",
                        GetPrimitiveGetMethodName(getMethodType), GetColumnInfo().GetPropertyPath().ToString().c_str(),
                        ExpHelper::ToString(actualDataType));

        return false;
        }

    return true;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//+---------------+---------------+---------------+---------------+---------------+------
//static
Utf8CP ArrayECSqlField::JsonECSqlValue::GetPrimitiveGetMethodName(ECN::PrimitiveType getMethodType)
    {
    switch (getMethodType)
        {
            case PRIMITIVETYPE_Binary:
                return "GetBlob";
            case PRIMITIVETYPE_Boolean:
                return "GetBoolean";
            case PRIMITIVETYPE_DateTime:
                return "GetDateTime";
            case PRIMITIVETYPE_Double:
                return "GetDouble";
            case PRIMITIVETYPE_IGeometry:
                return "GetGeometry";
            case PRIMITIVETYPE_Integer:
                return "GetInt";
            case PRIMITIVETYPE_Long:
                return "GetInt64";
            case PRIMITIVETYPE_Point2d:
                return "GetPoint2d";
            case PRIMITIVETYPE_Point3d:
                return "GetPoint3d";
            case PRIMITIVETYPE_String:
                return "GetText";

            default:
                BeAssert(false && "ECSqlField::GetPrimitiveGetMethodName needs to be adjusted to new ECN::PrimitiveType value");
                return "";
        }
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      01/2017
//+---------------+---------------+---------------+---------------+---------------+------
//static
rapidjson::Value const& ArrayECSqlField::JsonECSqlValue::GetNullJson()
    {
    if (s_nullJson == nullptr)
        s_nullJson = new rapidjson::Value(rapidjson::kNullType);

    return *s_nullJson;
    }


//**************************** ArrayECSqlField::JsonECSqlValue::ArrayIteratorState ****************

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      02/2017
//---------------------------------------------------------------------------------------
void ArrayECSqlField::JsonECSqlValue::ArrayIteratorState::_MoveToNext(bool onInitializingIterator) const
    {
    if (onInitializingIterator)
        {
        BeAssert(m_jsonIteratorIndex < 0);
        if (GetJson().IsNull())
            return;

        m_jsonIterator = GetJson().Begin();
        m_jsonIteratorIndex = 0;
        }
    else
        {
        m_jsonIterator++;
        m_jsonIteratorIndex++;
        }

    if (_IsAtEnd())
        return;

    BeAssert(m_jsonIteratorIndex < (int) GetJson().Size());
    if (m_jsonIteratorIndex >= m_value.m_arrayElementCache.size())
        {
        ECSqlColumnInfo elementColumnInfo = ECSqlFieldFactory::CreateColumnInfoForArrayElement(m_value.m_columnInfo, m_jsonIteratorIndex);
        m_value.m_arrayElementCache.push_back(std::make_unique<JsonECSqlValue>(m_value.m_ecdb, *m_jsonIterator, elementColumnInfo));
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//---------------------------------------------------------------------------------------
IECSqlValue const& ArrayECSqlField::JsonECSqlValue::ArrayIteratorState::_GetCurrent() const
    {
    if (m_jsonIteratorIndex >= (int) GetJson().Size())
        {
        BeAssert(m_jsonIteratorIndex < (int) GetJson().Size());
        return NoopECSqlValue::GetSingleton();
        }

    return *m_value.m_arrayElementCache[(size_t) m_jsonIteratorIndex];
    }

//**************************** ArrayECSqlField::JsonECSqlValue::StructIteratorState ****************

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      02/2017
//---------------------------------------------------------------------------------------
ArrayECSqlField::JsonECSqlValue::StructIteratorState::StructIteratorState(JsonECSqlValue const& val, ECN::ECPropertyIterableCR structMemberPropertyIterable) 
    : IIteratorState(), m_value(val), m_memberPropIterator(structMemberPropertyIterable.begin()), m_memberPropEndIterator(structMemberPropertyIterable.end())
    {}

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      02/2017
//---------------------------------------------------------------------------------------
void ArrayECSqlField::JsonECSqlValue::StructIteratorState::_MoveToNext(bool onInitializingIterator) const
    {
    //iterator is already initialized at construction, so don't increment it here
    if (!onInitializingIterator)
        ++m_memberPropIterator;
 
    if (_IsAtEnd())
        return;

    ECPropertyCP currentMemberProp = *m_memberPropIterator;
    BeAssert(currentMemberProp != nullptr);
    auto it = m_value.m_structMemberCache.find(currentMemberProp->GetName().c_str());
    if (it != m_value.m_structMemberCache.end())
        return;

    m_value.CreateStructMemberValue(*currentMemberProp);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//---------------------------------------------------------------------------------------
IECSqlValue const& ArrayECSqlField::JsonECSqlValue::StructIteratorState::_GetCurrent() const
    {
    Utf8CP memberName = (*m_memberPropIterator)->GetName().c_str();
    auto it = m_value.m_structMemberCache.find(memberName);
    if (it != m_value.m_structMemberCache.end())
        return *it->second;

    BeAssert(false && "Should have been caught before");
    return NoopECSqlValue::GetSingleton();
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
