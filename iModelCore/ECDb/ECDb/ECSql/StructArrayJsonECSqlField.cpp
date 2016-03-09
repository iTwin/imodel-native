/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/StructArrayJsonECSqlField.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//************************************************
// JsonECSqlValue
//************************************************

//-----------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//+---------------+---------------+---------------+---------------+---------------+------
std::unique_ptr<JsonECSqlValue> JsonECSqlFactory::CreateValue(ECDbCR ecdb, Json::Value const& json, ECSqlColumnInfo const& columnInfo, ECSqlColumnInfo const* parentColumnInfo)
    {
    ECTypeDescriptor const& dataType = columnInfo.GetDataType();
    if (dataType.IsPrimitive())
        return std::unique_ptr<JsonECSqlValue>(new PrimitiveJsonECSqlValue(ecdb, json, columnInfo));

    if (dataType.IsStruct())
        return std::unique_ptr<JsonECSqlValue>(new StructJsonECSqlValue(ecdb, json, columnInfo));

    return std::unique_ptr<JsonECSqlValue>(new ArrayJsonECSqlValue(ecdb, json, columnInfo));
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
    m_ecdb.GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Type mismatch. Cannot call primitive IECSqlValue getters on non-primitive IECSqlValue.");
    return NoopECSqlValue::GetSingleton().GetPrimitive();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlStructValue const& JsonECSqlValue::_GetStruct() const
    {
    m_ecdb.GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Type mismatch. Cannot call GetStruct on non-struct IECSqlValue.");
    return NoopECSqlValue::GetSingleton().GetStruct();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlArrayValue const& JsonECSqlValue::_GetArray() const
    {
    m_ecdb.GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "Type mismatch. Cannot call GetArray on non-array IECSqlValue.");
    return NoopECSqlValue::GetSingleton().GetArray();
    }

//************************************************
// JsonECSqlPrimitiveValue
//************************************************


//-----------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//+---------------+---------------+---------------+---------------+---------------+------
int PrimitiveJsonECSqlValue::_GetInt() const
    {
    return GetJson().asInt();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//+---------------+---------------+---------------+---------------+---------------+------
int64_t PrimitiveJsonECSqlValue::_GetInt64() const
    {
    return BeJsonUtilities::Int64FromValue(GetJson());
    }


//************************************************
// JsonECSqlStructValue
//************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2016
//---------------------------------------------------------------------------------------
StructJsonECSqlValue::StructJsonECSqlValue(ECDbCR ecdb, Json::Value const& json, ECSqlColumnInfo const& parentColumnInfo, ECN::ECPropertyCR prop) : JsonECSqlValue(ecdb, json, parentColumnInfo, prop), IECSqlStructValue()
    {
    ECStructClassCR structType = GetColumnInfo().GetProperty()->GetAsStructProperty()->GetType();
    for (ECPropertyCP structMemberProp : structType.GetProperties(true))
        {
        Json::Value const& val = json[structMemberProp->GetName()];
        m_members.push_back(JsonECSqlValue::Create(ecdb, val, GetColumnInfo());
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
        msg.Sprintf("Index %d is out of bounds for IECSqlStructValue::GetValue.", columnIndex);
        GetECDb().GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, msg.c_str());
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
ArrayJsonECSqlValue::ArrayJsonECSqlValue(ECDbCR ecdb, Json::Value const& json, ECSqlColumnInfo const& parentColumnInfo, ECN::ECPropertyCR prop)
    : JsonECSqlValue(ecdb, json, parentColumnInfo, prop), IECSqlArrayValue(), m_jsonIterator(json.end()), m_currentElement(nullptr)
    {
    BeAssert(GetJson().isArray());
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
        return;

    m_currentElement = JsonECSqlValueFactory::Create(GetECDb(), GetJson(), GetColumnInfo());
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
    : ECSqlField(ecsqlStatement, std::move(ecsqlColumnInfo), true, true), m_sqliteColumnIndex(sqliteColumnIndex), m_json(Json::arrayValue), m_arrayElement(*ecsqlStatement.GetECDb())
    {
    m_arrayElement.Init(m_ecsqlColumnInfo);
    Reset();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      02/2016
//---------------------------------------------------------------------------------------
ECSqlStatus StructArrayJsonECSqlField::_Init()
    {
    Reset();

    BeAssert(m_json.isArray());
    m_json.clear();

    Utf8String jsonStr(GetSqliteStatement().GetValueText(m_sqliteColumnIndex));
    Json::Reader reader;
    if (!reader.Parse(jsonStr, m_json, false))
        return ReportError(ECSqlStatus::Error, "Could not deserialize struct array JSON.");

    BeAssert(m_json.isArray());
    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      02/2016
//---------------------------------------------------------------------------------------
ECSqlStatus StructArrayJsonECSqlField::_Reset()
    {
    DoReset();
    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      02/2016
//---------------------------------------------------------------------------------------
void StructArrayJsonECSqlField::DoReset() const
    {
    m_jsonIterator = m_json.begin();
    m_arrayElement.Reset();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan      07/2013
//---------------------------------------------------------------------------------------
void StructArrayJsonECSqlField::_MoveNext(bool onInitializingIterator) const
    {
    if (onInitializingIterator)
        DoReset();

    m_jsonIterator++;

    if (_IsAtEnd())
        return;

    m_arrayElement.SetValue(*m_jsonIterator);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
bool StructArrayJsonECSqlField::_IsAtEnd() const
    {
    return m_jsonIterator == m_json.end();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
IECSqlValue const* StructArrayJsonECSqlField::_GetCurrent() const
    {
    return &m_arrayElement;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan      07/2013
//---------------------------------------------------------------------------------------
int StructArrayJsonECSqlField::_GetArrayLength() const
    {
    return (int) m_json.size();
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
void StructArrayJsonECSqlField::ArrayElementValue::Init(ECSqlColumnInfoCR parentColumnInfo)
    {
    m_columnInfo = ECSqlColumnInfo::CreateForArrayElement(parentColumnInfo, -1);
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
