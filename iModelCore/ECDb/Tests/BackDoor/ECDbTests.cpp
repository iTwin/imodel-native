/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/BackDoor/ECDbTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PublicAPI/BackDoor/ECDb/ECDbTests.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

//************************************************************************************
//ECDbIssueListener
//************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                             Krischan.Eberle     09/2015
//---------------------------------------------------------------------------------------
void ECDbIssueListener::_OnIssueReported(Utf8CP message) const
    {
    m_issue = ECDbIssue(message);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                             Krischan.Eberle     09/2015
//---------------------------------------------------------------------------------------
ECDbIssue ECDbIssueListener::GetIssue() const
    {
    if (!m_issue.IsIssue())
        return m_issue;

    ECDbIssue copy(m_issue);
    //reset cached issue before returning
    m_issue = ECDbIssue();
    return copy;
    }

//************************************************************************************
//ECDbTestLogger
//************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    03/2014
//---------------------------------------------------------------------------------------
//static member initialization
BentleyApi::NativeLogging::ILogger* ECDbTestLogger::s_logger = nullptr;


//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    03/2014
//---------------------------------------------------------------------------------------
//static
BentleyApi::NativeLogging::ILogger& ECDbTestLogger::Get()
    {
    if (s_logger == nullptr)
        s_logger = BentleyApi::NativeLogging::LoggingManager::GetLogger(L"ECDbTests");

    return *s_logger;
    }

//************************************************************************************
//ECDbTestUtility
//************************************************************************************

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Ramanujam.Raman                06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool CompareRelationships(IECRelationshipInstanceCR a, IECRelationshipInstanceCR b)
    {
    if (a.GetSource() == nullptr || b.GetSource() == nullptr || a.GetSource()->GetInstanceId() != b.GetSource()->GetInstanceId())
        {
        LOG.trace("CompareECInstances> Relationship instances are not equal: differing source instance ids.");
        return false;
        }

    if (a.GetTarget() == nullptr || b.GetTarget() == nullptr || a.GetTarget()->GetInstanceId() != b.GetTarget()->GetInstanceId())
        {
        LOG.trace("CompareECInstances> Relationship instances are not equal: differing target instance ids.");
            return false;
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Affan.Khan                        03/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool CompareProperties(IECInstanceCR actual, ECValuesCollectionCR expected)
    {
    for (ECPropertyValueCR expectedPropertyValue : expected)
        {
        ECValueAccessorCR valueAccessor = expectedPropertyValue.GetValueAccessor();
        const Utf8String propertyName = valueAccessor.GetPropertyName();

        if (expectedPropertyValue.HasChildValues())
            {
            if (!CompareProperties(actual, *expectedPropertyValue.GetChildValues()))
                return false;

            continue;
            }

        ECValue actualValue;
        ECObjectsStatus status = actual.GetValueUsingAccessor(actualValue, valueAccessor);
        if (status != ECObjectsStatus::Success)
            {
            BeAssert(false);
            return false;
            }

        ECValueCR expectedValue = expectedPropertyValue.GetValue();
        const bool expectedValueIsNull = expectedValue.IsNull();
        const bool actualValueIsNull = actualValue.IsNull();

        if (expectedValueIsNull != actualValueIsNull)
            {
            if (expectedValueIsNull)
                LOG.tracev("CompareProperties - Expected NULL value for property '%s' but the actual value was not NULL.", propertyName.c_str());
            else
                LOG.tracev("CompareProperties - Expected a non-NULL value for property '%s' but the actual value was NULL.", propertyName.c_str());
            
            return false;
            }

        if (expectedValue.Equals(actualValue))
            continue;

        PrimitiveType actualType = actualValue.GetPrimitiveType();
        if (actualType == PRIMITIVETYPE_DateTime)
            {
            int64_t expectedECTicks = expectedValue.GetDateTimeTicks();
            int64_t actualECTicks = actualValue.GetDateTimeTicks();
            if (expectedECTicks == actualECTicks)
                continue;
            }

        ValueKind actualKind = actualValue.GetKind();
        Utf8String expectedValueWStr = expectedValue.ToString();
        Utf8String actualValueWstr = actualValue.ToString();
        LOG.tracev("CompareECInstances> Instances are not equal: Differing property values property '%s' (%d %d): actual: %s, expected: %s", 
                   propertyName.c_str(), actualKind, actualType, actualValueWstr.c_str(), expectedValueWStr.c_str());
        return false;
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Affan.Khan                        03/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECDbTestUtility::CompareECInstances(IECInstanceCR expected, IECInstanceCR actual)
    {
    IECRelationshipInstanceCP relExpected = dynamic_cast<IECRelationshipInstanceCP> (&expected);
    IECRelationshipInstanceCP relActual = dynamic_cast<IECRelationshipInstanceCP> (&actual);
    if (relExpected != nullptr || relActual != nullptr)
        {
        if (relExpected == nullptr || relActual == nullptr)
            {
            LOG.trace("CompareECInstances> Instances are not equal. One is a relationship instance, the other is not.");
            return false; // both have to be non null
            }

        if (!CompareRelationships(*relExpected, *relActual))
            return false;
        }

    if (&expected.GetClass() == &actual.GetClass() && expected.GetClass().GetPropertyCount(true) == 0 && actual.GetClass().GetPropertyCount(true) == 0)
        return true;

    ECValuesCollectionPtr propertyValuesExpected = ECValuesCollection::Create(expected);
    if (propertyValuesExpected.IsNull())
        return false;

    return CompareProperties(actual, *propertyValuesExpected);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECDbTestUtility::CompareJsonWithECInstance(const Json::Value& json, IECInstanceCR referenceInstance)
    {
    IECRelationshipInstanceCP relationshipInstance = dynamic_cast<IECRelationshipInstanceCP> (&referenceInstance);
    if (relationshipInstance != nullptr)
        return false; // Cannot handle relationships!!

    ECValuesCollectionPtr propertyValues = ECValuesCollection::Create(referenceInstance);
    if (propertyValues.IsNull())
        return false;

    for (ECPropertyValueCR propertyValue : *propertyValues)
        {
        Utf8String propertyName = propertyValue.GetValueAccessor().GetPropertyName();
        Utf8String propertyAccessString = propertyValue.GetValueAccessor().GetAccessString();

        ECValueCR referenceValue = propertyValue.GetValue();
        if (referenceValue.IsNull() && !json.isMember(propertyName))
            continue;
        if (!json.isMember(propertyName))
            return false;

        Json::Value jsonValue = json[propertyName];
        if (!CompareJsonWithECValue(jsonValue, referenceValue, referenceInstance, propertyAccessString.c_str()))
            return false;
        }
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECDbTestUtility::CompareJsonWithECValue(const Json::Value& jsonValue, ECValueCR referenceValue, IECInstanceCR referenceInstance, Utf8CP referencePropertyAccessString)
    {
    Json::ValueType jsonValueType = jsonValue.type();

    // NULL values
    if (referenceValue.IsNull())
        return (jsonValueType == Json::nullValue);

    // Primitive values
    if (referenceValue.IsPrimitive())
        return CompareJsonWithECPrimitiveValue(jsonValue, referenceValue);

    // Struct values
    if (referenceValue.IsStruct())
        return CompareJsonWithECStructValue(jsonValue, referenceValue);

    // Array values
    if (referenceValue.IsArray())
        return CompareJsonWithECArrayValue(jsonValue, referenceValue, referenceInstance, referencePropertyAccessString);

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECDbTestUtility::CompareJsonWithECPrimitiveValue(const Json::Value& jsonValue, ECValueCR referenceValue)
    {
    if (!referenceValue.IsPrimitive())
        return false;
    Json::ValueType jsonValueType = jsonValue.type();
    PrimitiveType primitiveType = referenceValue.GetPrimitiveType();
    switch (primitiveType)
        {
            case PRIMITIVETYPE_Binary:
            {
            if (jsonValueType != Json::stringValue)
                return false;
            size_t binarySize;
            referenceValue.GetBinary(binarySize);
            SqlPrintfString referenceNativeValue("BLOB (%d bytes)", (int) binarySize);
            Utf8String jsonNativeValue = jsonValue.asString();
            if (0 != strcmp(referenceNativeValue.GetUtf8CP(), jsonNativeValue.c_str()))
                return false;
            break;
            }
            case PRIMITIVETYPE_Boolean:
            {
            if (jsonValueType != Json::booleanValue)
                return false;
            bool referenceNativeValue = referenceValue.GetBoolean();
            bool jsonNativeValue = jsonValue.asBool();
            if (referenceNativeValue != jsonNativeValue)
                return false;
            break;
            }
            case PRIMITIVETYPE_DateTime:
            {
            if (jsonValueType != Json::realValue)
                return false;

            const int64_t referenceECTicks = referenceValue.GetDateTimeTicks();
            const double jsonJd = jsonValue.asDouble();
            const uint64_t jdMsec = DateTime::RationalDayToMsec(jsonJd);
            const int64_t jsonCETicks = DateTime::JulianDayToCommonEraTicks(jdMsec);
            return referenceECTicks == jsonCETicks;
            }

            case PRIMITIVETYPE_Double:
            {
            if (jsonValueType != Json::realValue)
                return false;
            double referenceNativeValue = referenceValue.GetDouble();
            double jsonNativeValue = jsonValue.asDouble();
            if (referenceNativeValue != jsonNativeValue)
                return false;
            break;
            }
            case PRIMITIVETYPE_Integer:
            {
            if (jsonValueType != Json::intValue)
                return false;
            int referenceNativeValue = referenceValue.GetInteger();
            int jsonNativeValue = jsonValue.asInt();
            if (referenceNativeValue != jsonNativeValue)
                return false;
            break;
            }
            case PRIMITIVETYPE_Long:
            {
            if (jsonValueType != Json::intValue)
                return false;
            int64_t referenceNativeValue = referenceValue.GetLong();
            int64_t jsonNativeValue = jsonValue.asInt64();
            if (referenceNativeValue != jsonNativeValue)
                return false;
            break;
            }
            case PRIMITIVETYPE_Point2d:
            {
            if (jsonValueType != Json::objectValue)
                return false;
            DPoint2d referenceNativeValue = referenceValue.GetPoint2d();
            DPoint2d jsonNativeValue = DPoint2d::From(jsonValue["x"].asDouble(), jsonValue["y"].asDouble());
            if (!referenceNativeValue.IsEqual(jsonNativeValue))
                return false;
            break;
            }
            case PRIMITIVETYPE_Point3d:
            {
            if (jsonValueType != Json::objectValue)
                return false;
            DPoint3d referenceNativeValue = referenceValue.GetPoint3d();
            DPoint3d jsonNativeValue = DPoint3d::From(jsonValue["x"].asDouble(), jsonValue["y"].asDouble(), jsonValue["z"].asDouble());
            if (!referenceNativeValue.IsEqual(jsonNativeValue))
                return false;
            break;
            }
            case PRIMITIVETYPE_String:
            {
            if (jsonValueType != Json::stringValue)
                return false;
            Utf8String  referenceNativeValue(referenceValue.GetUtf8CP());
            Utf8String jsonNativeValue = jsonValue.asString();
            if (referenceNativeValue != jsonNativeValue)
                return false;
            break;
            }
            default:
                return false;
                break;
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECDbTestUtility::CompareJsonWithECStructValue(const Json::Value& jsonValue, ECValueCR referenceValue)
    {
    if (!referenceValue.IsStruct())
        return false;
    Json::ValueType jsonValueType = jsonValue.type();
    if (jsonValueType != Json::objectValue)
        return false;

    IECInstancePtr structInstance = referenceValue.GetStruct();
    if (structInstance.IsNull())
        return false;
    if (!CompareJsonWithECInstance(jsonValue, *structInstance))
        return false;
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECDbTestUtility::CompareJsonWithECArrayValue(const Json::Value& jsonValue, ECValueCR referenceValue, IECInstanceCR referenceInstance, Utf8CP referencePropertyAccessString)
    {
    if (referenceValue.IsNull())
        return false;
    if (!referenceValue.IsArray())
        return false;

    Json::ValueType jsonValueType = jsonValue.type();
    if (jsonValueType != Json::arrayValue)
        return false;

    ArrayInfo arrayInfo = referenceValue.GetArrayInfo();
    int referenceCount = arrayInfo.GetCount();
    if (referenceCount != jsonValue.size())
        return false;
    for (int ii = 0; ii <referenceCount; ii++)
        {
        ECValue referenceMemberValue;
        referenceInstance.GetValue(referenceMemberValue, referencePropertyAccessString, ii);
        if (!CompareJsonWithECValue(jsonValue[ii], referenceMemberValue, referenceInstance, referencePropertyAccessString))
            return false;
        }
    return true;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Krischan.Eberle                   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
//static
void ECDbTestUtility::AssertECDateTime(ECValueCR expectedECValue, const Db& db, double actualJd)
    {
    const DateTime expectedDateTime = expectedECValue.GetDateTime();
    const int64_t expectedCETicks = expectedECValue.GetDateTimeTicks();
    uint64_t expectedJdMsec = DateTime::CommonEraTicksToJulianDay(expectedCETicks);
    const uint64_t actualJdMsec = DateTime::RationalDayToMsec(actualJd);
    ASSERT_EQ(expectedJdMsec, actualJdMsec) << "EC date time assertion failed for " << expectedDateTime.ToString().c_str();

    //now compare with SQLite date time functions
    Utf8String sql;
    sql.Sprintf("select julianday('%s');", expectedDateTime.ToString().c_str());
    Statement sqliteStatement;
    ASSERT_EQ(BE_SQLITE_OK, sqliteStatement.Prepare(db, sql.c_str())) << sql.c_str();
    ASSERT_EQ(BE_SQLITE_ROW, sqliteStatement.Step()) << sql.c_str();
    const double sqliteJd = sqliteStatement.GetValueDouble(0);
    const uint64_t sqliteJdMsec = DateTime::RationalDayToMsec(sqliteJd);
    ASSERT_EQ(expectedJdMsec, sqliteJdMsec) << "Comparison of Julian Days computed by Bentley::DateTime and SQLite date function failed for " << expectedDateTime.ToString().c_str();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
int64_t ECDbTestUtility::ReadCellValueAsInt64(DbR db, Utf8CP tableName, Utf8CP columnName, Utf8CP whereClause)
    {
    Utf8String str;
    str.Sprintf("SELECT %s FROM %s %s", columnName, tableName, whereClause);
    Statement stmt;
    if (BE_SQLITE_OK != stmt.Prepare(db, str.c_str()))
        {
        LOG.errorv("Failed to prepare SQL %s. Error: %s", str.c_str(), db.GetLastError().c_str());
        return -1;
        }

    return (BE_SQLITE_ROW != stmt.Step()) ? 0 : stmt.GetValueInt64(0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
void ECDbTestUtility::DebugDumpJson(const Json::Value& jsonValue)
    {
    Utf8String strValue = Json::StyledWriter().write(jsonValue);
    int len = (int) strValue.size();
    for (int ii = 0; ii < len; ii += 1000)
        {
        // Split the string up - logging can't seem to handle long strings
        Utf8String subStr = strValue.substr(ii, 1000);
        LOG.infov("%s", subStr.c_str());
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  Muhammad.Zaighum                  05/13
//+---------------+---------------+---------------+---------------+---------------+------
//static
BentleyStatus ECDbTestUtility::ReadJsonInputFromFile(Json::Value& jsonInput, BeFileName& jsonFilePath)
    {
    const Byte utf8BOM[] = {0xef, 0xbb, 0xbf};

    Utf8String fileContent;

    BeFile file;
    if (BeFileStatus::Success != file.Open(jsonFilePath, BeFileAccess::Read))
        return ERROR;

    uint64_t rawSize;
    if (BeFileStatus::Success != file.GetSize(rawSize) || rawSize > UINT32_MAX)
        return ERROR;

    uint32_t sizeToRead = (uint32_t) rawSize;

    uint32_t sizeRead;
    ScopedArray<Byte> scopedBuffer(sizeToRead);
    Byte* buffer = scopedBuffer.GetData();
    if (BeFileStatus::Success != file.Read(buffer, &sizeRead, sizeToRead) || sizeRead != sizeToRead)
        return ERROR;

    if (buffer[0] != utf8BOM[0] || buffer[1] != utf8BOM[1] || buffer[2] != utf8BOM[2])
        {
        LOG.error("Json file is expected to be encoded in UTF-8");
        return ERROR;
        }

    for (uint32_t ii = 3; ii < sizeRead; ii++)
        {
        if (buffer[ii] == '\n' || buffer[ii] == '\r')
            continue;
        fileContent.append(1, buffer[ii]);
        }

    file.Close();

    return Json::Reader::Parse(fileContent, jsonInput) ? SUCCESS : ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan     03/12
//---------------------------------------------------------------------------------------
//static
IECInstancePtr ECDbTestUtility::CreateArbitraryECInstance(ECClassCR ecClass, PopulatePrimitiveValueCallback populatePrimitiveValueCallback, bool skipStructs, bool skipArrays, bool skipReadOnlyProps)
    {
    IECInstancePtr instance = ecClass.GetDefaultStandaloneEnabler()->CreateInstance(0);
    PopulateECInstance(instance, populatePrimitiveValueCallback, skipStructs, skipArrays, skipReadOnlyProps);
    return instance;
    }


#define MAX_ARRAY_TEST_ENTRIES 3

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan     03/12
//---------------------------------------------------------------------------------------
void ECDbTestUtility::PopulateECInstance(ECN::IECInstancePtr ecInstance, PopulatePrimitiveValueCallback populatePrimitiveValueCallback, bool skipStructs, bool skipArrays, bool skipReadOnlyProps)
    {
    ECValue value;
    for (ECPropertyCP ecProperty : ecInstance->GetClass().GetProperties(true))
        {
        if (ecProperty->GetIsReadOnly() && skipReadOnlyProps)
            continue;

        if (!skipStructs && ecProperty->GetIsStruct())
            {
            PopulateStructValue(value, ecProperty->GetAsStructProperty()->GetType(), populatePrimitiveValueCallback);
            CopyStruct(*ecInstance, *value.GetStruct(), ecProperty->GetName().c_str());
            }
        else if (ecProperty->GetIsPrimitive())
            {
            populatePrimitiveValueCallback(value, ecProperty->GetAsPrimitiveProperty()->GetType(), ecProperty);
            ecInstance->SetValue(ecProperty->GetName().c_str(), value);
            }
        else if (!skipArrays && ecProperty->GetIsArray())
            {
            ArrayECPropertyCP arrayProperty = ecProperty->GetAsArrayProperty();
            if (arrayProperty->GetIsPrimitiveArray() && arrayProperty->GetAsPrimitiveArrayProperty()->GetPrimitiveElementType() == PRIMITIVETYPE_IGeometry)
                continue;

            uint32_t arrayCount = MAX_ARRAY_TEST_ENTRIES;
            if (arrayCount < arrayProperty->GetMinOccurs())
                arrayCount = arrayProperty->GetMinOccurs();
            else if (arrayCount > arrayProperty->GetMaxOccurs())
                arrayCount = arrayProperty->GetMaxOccurs();

            ecInstance->AddArrayElements(ecProperty->GetName().c_str(), arrayCount);
            if (arrayProperty->GetIsStructArray())
                {
                StructArrayECPropertyCP structArrayProperty = ecProperty->GetAsStructArrayProperty();
                for (uint32_t i = 0; i < arrayCount; i++)
                    {
                    PopulateStructValue(value, structArrayProperty->GetStructElementType(), populatePrimitiveValueCallback);
                    ecInstance->SetValue(ecProperty->GetName().c_str(), value, i);
                    }
                }
            else if (arrayProperty->GetIsPrimitiveArray())
                {
                PrimitiveArrayECPropertyCP primitiveArrayProperty = ecProperty->GetAsPrimitiveArrayProperty();
                for (uint32_t i = 0; i < arrayCount; i++)
                    {
                    populatePrimitiveValueCallback(value, primitiveArrayProperty->GetPrimitiveElementType(), ecProperty);
                    ecInstance->SetValue(ecProperty->GetName().c_str(), value, i);
                    }
                }
            }
        }
    }

//---------------------------------------------------------------------------------------
//! @remarks Currently only supports primitive properties (except for IGeometry).
//! @bsimethod                                 Krischan.Eberle                09/2012
//+---------------+---------------+---------------+---------------+---------------+------
//static
void ECDbTestUtility::AssignRandomValueToECInstance (ECValueP createdValue, IECInstancePtr instance, Utf8CP propertyName)
    {
    ECPropertyP ecProperty = instance->GetClass().GetPropertyP(propertyName);
    ASSERT_TRUE(ecProperty != nullptr);
    ASSERT_TRUE(ecProperty->GetIsPrimitive());

    PrimitiveType type = ecProperty->GetAsPrimitiveProperty()->GetType();

    ECValue value;
    GenerateRandomValue(value, type, ecProperty);

    instance->SetValue(propertyName, value);
    //if requested, return the value created (as copy)
    if (createdValue != nullptr)
        {
        *createdValue = value;
        }
    }

//---------------------------------------------------------------------------------------
//! @remarks Currently only supports primitive properties (except for IGeometry).
//! @bsimethod                                 Krischan.Eberle                09/2012
//+---------------+---------------+---------------+---------------+---------------+------
//static
void ECDbTestUtility::GenerateRandomValue(ECValueR value, PrimitiveType type, ECPropertyCP ecProperty)
    {
    srand((uint32_t)(BeTimeUtilities::QueryMillisecondsCounter() & 0xFFFFFFFF));
    int randomNumber = rand();
    switch (type)
        {
            case PRIMITIVETYPE_String:
            {
            Utf8String text;
            text.Sprintf("Sample text with random number: %d", randomNumber);
            value.SetUtf8CP(text.c_str(), true);
            }
            break;

            case PRIMITIVETYPE_Integer:
            {
            value.SetInteger(randomNumber);
            }
            break;

            case PRIMITIVETYPE_Long:
            {
            const int32_t intMax = std::numeric_limits<int32_t>::max();
            const int64_t longValue = static_cast<int64_t> (intMax) + randomNumber;
            value.SetLong(longValue);
            }
            break;

            case PRIMITIVETYPE_Double:
            {
            value.SetDouble(randomNumber * PI);
            }
            break;

            case PRIMITIVETYPE_DateTime:
            {
            DateTime dt;
            DateTime::Info dti;
            if (ecProperty != nullptr && StandardCustomAttributeHelper::GetDateTimeInfo(dti, *ecProperty) == ECObjectsStatus::Success)
                {
                if (!dti.IsValid())
                    dti = DateTime::Info::CreateForDateTime(DateTime::Kind::Unspecified);

                if (dti.GetKind() == DateTime::Kind::Local)
                    {
                    //local date times are not supported by ECDb
                    break;
                    }

                uint64_t jd = 0;
                DateTime::GetCurrentTimeUtc().ToJulianDay(jd);

                DateTime::FromJulianDay(dt, jd, dti);
                }
            else
                {
                dt = DateTime::GetCurrentTimeUtc();
                }

            value.SetDateTime(dt);
            }
            break;

            case PRIMITIVETYPE_Boolean:
            {
            value.SetBoolean(randomNumber % 2 != 0);
            }
            break;

            case PRIMITIVETYPE_Point2d:
            {
            DPoint2d point2d;
            point2d.x = randomNumber * 1.0;
            point2d.y = randomNumber * 1.8;
            value.SetPoint2d(point2d);
            break;
            }
            case PRIMITIVETYPE_Point3d:
            {
            DPoint3d point3d;
            point3d.x = randomNumber * 1.0;
            point3d.y = randomNumber * 1.8;
            point3d.z = randomNumber * 2.9;
            value.SetPoint3d(point3d);
            break;
            }

            case PRIMITIVETYPE_Binary:
            {
            Byte blob[] = {0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x1a, 0xaa, 0xfa, 0x00};
            value.SetBinary(blob, 10);
            break;
            }

            case PRIMITIVETYPE_IGeometry:
            {
            LOG.debug(L"Generating random values for IGeometry not supported by ECDbTest yet.");
            break;
            }
            default:
                break;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Affan.Khan                          03/12
+---------------+---------------+---------------+---------------+---------------+------*/
//static
void ECDbTestUtility::PopulateStructValue(ECValueR value, ECClassCR structType, PopulatePrimitiveValueCallback populatePrimitiveValueCallback)
    {
    value.Clear();
    IECInstancePtr inst = CreateArbitraryECInstance(structType, populatePrimitiveValueCallback);
    value.SetStruct(inst.get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Affan.Khan                          03/12
+---------------+---------------+---------------+---------------+---------------+------*/
void ECDbTestUtility::PopulatePrimitiveValue(ECValueR value, PrimitiveType primitiveType, ECPropertyCP ecProperty)
    {
    value.Clear();
    switch (primitiveType)
        {
            case PRIMITIVETYPE_String:
                value.SetUtf8CP("Sample string"); break;
            case PRIMITIVETYPE_Integer:
                value.SetInteger(123); break;
            case PRIMITIVETYPE_Long:
                value.SetLong(123456789); break;
            case PRIMITIVETYPE_Double:
                value.SetDouble(PI); break;
            case PRIMITIVETYPE_DateTime:
            {
            DateTime::Info dti;
            if (ecProperty != nullptr && StandardCustomAttributeHelper::GetDateTimeInfo(dti, *ecProperty) == ECObjectsStatus::Success)
                {
                if (dti.GetKind() == DateTime::Kind::Local)
                    {
                    //local date times are not supported by ECObjects
                    break;
                    }

                if (!dti.IsValid())
                    dti = DateTime::Info::CreateForDateTime(DateTime::Kind::Unspecified);

                DateTime dt;
                DateTime::FromJulianDay(dt, 2456341.75, dti);
                value.SetDateTime(dt);
                }
            break;
            }

            case PRIMITIVETYPE_Binary:
            {
            Byte blob[] = {0x0c, 0x0b, 0x0c, 0x0b, 0x0c, 0x0b, 65, 66, 67, 68, 0x0c};
            value.SetBinary(blob, 11, true);
            break;
            }
            case PRIMITIVETYPE_Boolean:
                value.SetBoolean(true); break;
            case PRIMITIVETYPE_Point2d:
            {
            DPoint2d point2d;
            point2d.x = 11.25;
            point2d.y = 22.16;
            value.SetPoint2d(point2d);
            break;
            }
            case PRIMITIVETYPE_Point3d:
            {
            DPoint3d point3d;
            point3d.x = 11.23;
            point3d.y = 22.14;
            point3d.z = 33.12;
            value.SetPoint3d(point3d);
            break;
            }

            case PRIMITIVETYPE_IGeometry:
            {
            IGeometryPtr line = IGeometry::Create(ICurvePrimitive::CreateLine(DSegment3d::From(0.0, 0.0, 0.0, 1.0, 1.0, 1.0)));
            value.SetIGeometry(*line);
            break;
            }
        }
    }

//-------------------------------------------------------------------------------------
/// <author>Carole.MacDonald</author>                     <date>11/2013</date>
//---------------+---------------+---------------+---------------+---------------+-----
void ECDbTestUtility::PopulatePrimitiveValueWithRandomValues(ECValueR ecValue, PrimitiveType primitiveType, ECPropertyCP ecProperty)
    {
    ecValue.Clear();

    int randomNumber = rand();
    switch (primitiveType)
        {
            case PRIMITIVETYPE_String:
            {
            Utf8String text;
            text.Sprintf("Sample text with random number: %d", randomNumber);
            ecValue.SetUtf8CP(text.c_str(), true);
            }
            break;

            case PRIMITIVETYPE_Integer:
            {
            ecValue.SetInteger(randomNumber);
            }
            break;

            case PRIMITIVETYPE_Long:
            {
            const int32_t intMax = std::numeric_limits<int32_t>::max();
            const int64_t longValue = static_cast<int64_t> (intMax) + randomNumber;
            ecValue.SetLong(longValue);
            }
            break;

            case PRIMITIVETYPE_Double:
            {
            ecValue.SetDouble(randomNumber * PI);
            }
            break;

            case PRIMITIVETYPE_DateTime:
            {
            DateTime utcTime = DateTime::GetCurrentTimeUtc();
            ecValue.SetDateTime(utcTime);
            }
            break;

            case PRIMITIVETYPE_Boolean:
            {
            ecValue.SetBoolean(randomNumber % 2 != 0);
            }
            break;

            case PRIMITIVETYPE_Point2d:
            {
            DPoint2d point2d;
            point2d.x = randomNumber * 1.0;
            point2d.y = randomNumber * 1.8;
            ecValue.SetPoint2d(point2d);
            break;
            }
            case PRIMITIVETYPE_Point3d:
            {
            DPoint3d point3d;
            point3d.x = randomNumber * 1.0;
            point3d.y = randomNumber * 1.8;
            point3d.z = randomNumber * 2.9;
            ecValue.SetPoint3d(point3d);
            break;
            }

            case PRIMITIVETYPE_IGeometry:
            {
            IGeometryPtr line = IGeometry::Create(ICurvePrimitive::CreateLine(DSegment3d::From(randomNumber, randomNumber*2.0, randomNumber*3.0,
                                                                                               -randomNumber, randomNumber*(-2.0), randomNumber*(-3.0))));
            ecValue.SetIGeometry(*line);
            break;
            }

            default:
                break;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan     9/2013
//---------------------------------------------------------------------------------------
ECObjectsStatus ECDbTestUtility::CopyStruct(IECInstanceR source, ECValuesCollectionCR collection, Utf8CP baseAccessPath)
    {
    ECObjectsStatus status = ECObjectsStatus::Success;
    for (auto& propertyValue : collection)
        {
        auto pvAccessString = propertyValue.GetValueAccessor().GetPropertyName();
        auto accessString = baseAccessPath == nullptr ? pvAccessString : Utf8String(baseAccessPath) + "." + pvAccessString;

        if (propertyValue.HasChildValues())
            {
            status = CopyStruct(source, *propertyValue.GetChildValues(), accessString.c_str());
            if (status != ECObjectsStatus::Success)
                {
                return status;
                }
            continue;
            }

        auto& location = propertyValue.GetValueAccessor().DeepestLocationCR();

        //auto property = location.GetECProperty(); 
        //BeAssert(property != nullptr);
        if (location.GetArrayIndex() >= 0)
            {
            source.AddArrayElements(accessString.c_str(), 1);
            status = source.SetValue(accessString.c_str(), propertyValue.GetValue(), location.GetArrayIndex());
            }
        else
            status = source.SetValue(accessString.c_str(), propertyValue.GetValue());

        if (status != ECObjectsStatus::Success)
            {
            return status;
            }
        }
    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan     9/2013
//---------------------------------------------------------------------------------------
ECObjectsStatus ECDbTestUtility::CopyStruct(IECInstanceR target, IECInstanceCR structValue, Utf8CP propertyName)
    {
    return CopyStruct(target, *ECValuesCollection::Create(structValue), propertyName);
    }

END_ECDBUNITTESTS_NAMESPACE

//************************************************************************************
//GTest PrintTo customizations
//************************************************************************************

BEGIN_BENTLEY_NAMESPACE
//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  06/17
//+---------------+---------------+---------------+---------------+---------------+------
void PrintTo(BentleyStatus stat, std::ostream* os)
    {
    switch (stat)
        {
            case SUCCESS:
                *os << "SUCCESS";
                break;

            case ERROR:
                *os << "ERROR";
                break;

            default:
                *os << "Unhandled BentleyStatus. Adjust the PrintTo method";
                break;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  05/17
//+---------------+---------------+---------------+---------------+---------------+------
void PrintTo(BeInt64Id id, std::ostream* os) {  *os << id.GetValueUnchecked();  }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  05/17
//+---------------+---------------+---------------+---------------+---------------+------
void PrintTo(DateTime const& dt, std::ostream* os) { *os << dt.ToString().c_str(); }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  07/17
//+---------------+---------------+---------------+---------------+---------------+------
void PrintTo(std::vector<Utf8CP> const& vec, std::ostream* os)
    {
    *os << "{";
    bool isFirstItem = true;
    for (Utf8CP str : vec)
        {
        if (!isFirstItem)
            *os << ",";

        *os << str;
        isFirstItem = false;
        }
    *os << "}";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  07/17
//+---------------+---------------+---------------+---------------+---------------+------
void PrintTo(std::vector<Utf8String> const& vec, std::ostream* os)
    {
    *os << "{";
    bool isFirstItem = true;
    for (Utf8StringCR str : vec)
        {
        if (!isFirstItem)
            *os << ",";

        *os << str.c_str();
        isFirstItem = false;
        }
    *os << "}";
    }
END_BENTLEY_NAMESPACE

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  05/17
//+---------------+---------------+---------------+---------------+---------------+------
void PrintTo(ECClassId id, std::ostream* os) { PrintTo((BeInt64Id) id, os); }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  06/17
//+---------------+---------------+---------------+---------------+---------------+------
void PrintTo(ECValue const& val, std::ostream* os) {  *os << val.ToString().c_str(); }

END_BENTLEY_ECOBJECT_NAMESPACE

BEGIN_BENTLEY_SQLITE_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  06/17
//+---------------+---------------+---------------+---------------+---------------+------
void PrintTo(DbResult r, std::ostream* os)
    {
    *os << Db::InterpretDbResult(r);
    }

END_BENTLEY_SQLITE_NAMESPACE

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  06/17
//+---------------+---------------+---------------+---------------+---------------+------
void PrintTo(ECInstanceId id, std::ostream* os) { PrintTo((BeInt64Id) id, os); }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  06/17
//+---------------+---------------+---------------+---------------+---------------+------
void PrintTo(ECInstanceKey const& key, std::ostream* os) 
    { 
    *os << "{ECInstanceId:";
    PrintTo(key.GetInstanceId(), os);
    *os << ",ECClassId:";
    PrintTo(key.GetClassId(), os);
    *os << "}";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  05/17
//+---------------+---------------+---------------+---------------+---------------+------
void PrintTo(ECSqlStatus stat, std::ostream* os)
    {
    if (stat.IsSuccess())
        {
        *os << "ECSqlStatus::Success";
        return;
        }

    if (stat.IsSQLiteError())
        {
        *os << "ECSqlStatus::SQLiteError " << stat.GetSQLiteError();
        return;
        }

    switch (stat.Get())
        {
            case ECSqlStatus::Status::Error:
                *os << "ECSqlStatus::Error";
                return;

            case ECSqlStatus::Status::InvalidECSql:
                *os << "ECSqlStatus::InvalidECSql";
                return;

            default:
                *os << "Unhandled ECSqlStatus. Adjust the PrintTo method";
                break;
        }
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
