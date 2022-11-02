/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
| Based on http://cplus.about.com/od/howtodothingsi2/a/timing.htm
|
+--------------------------------------------------------------------------------------*/

#include "../ECObjectsTestPCH.h"
#include <Bentley/BeTimeUtilities.h>
#include "TestFixture.h"

USING_NAMESPACE_BENTLEY_LOGGING
USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_ECN_TEST_NAMESPACE
ECSchemaPtr ECTestFixture::s_unitsSchema;
ECSchemaPtr ECTestFixture::s_formatsSchema;

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void ECTestFixture::SetUp()
    {
    }

//----------------------------------------------------------------------------------------
// @bsimethod
//----------------------------------------------------------------------------------------
void ECTestFixture::TearDown()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECTestFixture::ECTestFixture()
    {
    BeFileName assetsDir;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory (assetsDir);
    ECN::ECSchemaReadContext::Initialize (assetsDir);
    GetUnitsSchema();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
WString ECTestFixture::GetTestDataPath(WCharCP dataFile)
    {
    return GetAssetsDataPath({L"SeedData", dataFile});
    }

//---------------------------------------------------------------------------------------
// * @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
WString ECTestFixture::GetAssetsDataPath(std::vector<WString> pathBits)
    {
    BeFileName testData;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory (testData);
    for(const auto& pathB : pathBits)
        testData.AppendToPath (pathB.c_str());
    return testData;
    }

//---------------------------------------------------------------------------------------
// * @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
WString ECTestFixture::GetAssetsGDataPath(std::vector<WString> pathBits)
    {
    BeFileName testData;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory (testData);
    testData.PopDir();
    testData.AppendToPath (L"Assets-g");
    for(const auto& pathB : pathBits)
        testData.AppendToPath (pathB.c_str());
    return testData;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
WString ECTestFixture::GetTempDataPath(WCharCP dataFile)
    {
    BeFileName testData;
    BeTest::GetHost().GetOutputRoot (testData);
    testData.AppendToPath (dataFile);
    return testData;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ECTestFixture::GetDateTime ()
    {
    struct tm timeinfo;
    BeTimeUtilities::ConvertUnixMillisToTm (timeinfo, BeTimeUtilities::GetCurrentTimeAsUnixMillis());   // GMT

    char buff[32];
    strftime (buff, sizeof(buff), "%y/%m/%d", &timeinfo);
    Utf8String dateTime (buff);
    dateTime.append (" ");
    strftime(buff, sizeof(buff), "%H:%M:%S", &timeinfo);
    dateTime.append (buff);
    return dateTime.c_str();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaPtr ECTestFixture::GetUnitsSchema(bool recreate)
    {
    if(recreate || s_unitsSchema.IsNull())
        {
        ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
        SchemaKey key("Units", 1, 0, 0);
        s_unitsSchema = context->LocateSchema(key, SchemaMatchType::LatestReadCompatible);
        }
    return s_unitsSchema;
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
ECSchemaPtr ECTestFixture::GetFormatsSchema(bool recreate)
    {
    if (recreate || s_formatsSchema.IsNull())
        {
        ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
        SchemaKey key("Formats", 1, 0, 0);
        s_formatsSchema = context->LocateSchema(key, SchemaMatchType::LatestReadCompatible);
        }
    return s_formatsSchema;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
void deserializeSchema(ECSchemaPtr& schema, ECSchemaReadContextR context, SchemaItem const& schemaItem, SchemaReadStatus expectedStatus, bool assert = false, Utf8CP failureMessage = "")
    {
    if (SchemaItem::Type::File == schemaItem.GetType())
        {
        auto filePath = ECTestFixture::GetTestDataPath(schemaItem.GetFileName().c_str());

        schema = ECSchema::LocateSchema(filePath.c_str(), context);
        if (SchemaReadStatus::Success == expectedStatus)
            ASSERT_TRUE(schema.IsValid()) << failureMessage;
        else
            ASSERT_FALSE(schema.IsValid()) << failureMessage;

        return;
        }

    SchemaReadStatus readStatus = ECSchema::ReadFromXmlString(schema, schemaItem.GetXmlString().c_str(), context);

    if (assert)
        {
        ASSERT_EQ(expectedStatus, readStatus) << failureMessage;
        if (SchemaReadStatus::Success == expectedStatus)
            ASSERT_TRUE(schema.IsValid()) << failureMessage;
        else
            ASSERT_FALSE(schema.IsValid()) << failureMessage;
        }
    else
        {
        EXPECT_EQ(expectedStatus, readStatus);
        if (SchemaReadStatus::Success == expectedStatus)
            EXPECT_TRUE(schema.IsValid()) << failureMessage;
        else
            EXPECT_FALSE(schema.IsValid()) << failureMessage;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
// static
void ECTestFixture::DeserializeSchema(ECSchemaPtr& schema, ECSchemaReadContextR context, SchemaItem const& schemaItem, SchemaReadStatus expectedStatus, Utf8CP failureMessage)
    {
    deserializeSchema(schema, context, schemaItem, expectedStatus, true, failureMessage);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
// static
void ECTestFixture::ExpectSchemaDeserializationFailure(SchemaItem const& schemaItem, SchemaReadStatus expectedError, Utf8CP failureMessage)
    {
    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    deserializeSchema(schema, *context, schemaItem, expectedError, false, failureMessage);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
// static
void ECTestFixture::AssertSchemaDeserializationFailure(SchemaItem const& schemaItem, SchemaReadStatus expectedError, Utf8CP failureMessage)
    {
    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    deserializeSchema(schema, *context, schemaItem, expectedError, true, failureMessage);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
// static
void ECTestFixture::RoundTripSchema(ECSchemaPtr& schema, SchemaItem item, ECVersion toVersion, SchemaReadStatus expectedReadStatus, SchemaWriteStatus expectedWriteStatus, Utf8CP failureMessage)
    {
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ECSchemaPtr local;
    deserializeSchema(local, *context, item, SchemaReadStatus::Success, true, "Should be able to deserialize original schema for round trip test");
    RoundTripSchema(schema, local.get(), toVersion, expectedReadStatus, expectedWriteStatus, failureMessage);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
// static
void ECTestFixture::RoundTripSchema(ECSchemaPtr& schema, ECSchemaCP inSchema, ECVersion toVersion, SchemaReadStatus expectedReadStatus, SchemaWriteStatus expectedWriteStatus, Utf8CP failureMessage)
    {
    Utf8String outXml;
    ASSERT_EQ(expectedWriteStatus, inSchema->WriteToXmlString(outXml, toVersion));
    if (SchemaWriteStatus::Success != expectedWriteStatus)
        return;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    deserializeSchema(schema, *context, SchemaItem(outXml), expectedReadStatus, true, "Should be able to deserialize the round tripped schema");
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECTestUtility::ReadJsonInputFromFile(Json::Value& jsonInput, BeFileName& jsonFilePath)
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
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool ECTestUtility::JsonDeepEqual(Json::Value const& a, Json::Value const& b)
    {
    auto astr = a.ToString();
    auto bstr = b.ToString();
    return astr == bstr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String ECTestUtility::JsonSchemasComparisonString(Json::Value const& createdSchema, Json::Value const& testDataSchema)
    {
    return "Created Schema   (minified): " + createdSchema.ToString() + '\n' +
           "Test Data Schema (minified): " + testDataSchema.ToString() + '\n' +
           "Created Schema   (pretty):\n"  + createdSchema.toStyledString() + '\n' +
           "Test Data Schema (pretty):\n"  + testDataSchema.toStyledString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
* @bsimethod
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
        LOG.errorv("CompareECInstances> Instances are not equal: Differing property values property '%s' (%d %d): actual: %s, expected: %s",
                   propertyName.c_str(), actualKind, actualType, actualValueWstr.c_str(), expectedValueWStr.c_str());
        return false;
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECTestUtility::CompareECInstances(ECN::IECInstanceCR expected, ECN::IECInstanceCR actual)
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

END_BENTLEY_ECN_TEST_NAMESPACE

