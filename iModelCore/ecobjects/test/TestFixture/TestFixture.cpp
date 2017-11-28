#include "TestFixture.h"
/*--------------------------------------------------------------------------------------+
|
|     $Source: test/TestFixture/TestFixture.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
| Based on http://cplus.about.com/od/howtodothingsi2/a/timing.htm
|
+--------------------------------------------------------------------------------------*/

#include "../ECObjectsTestPCH.h"
#include <Bentley/BeTimeUtilities.h>
#include "TestFixture.h"

USING_NAMESPACE_BENTLEY_LOGGING
USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_ECN_TEST_NAMESPACE
  
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                08/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECTestFixture::ECTestFixture()
    {
    BeFileName assetsDir;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory (assetsDir);
    ECN::ECSchemaReadContext::Initialize (assetsDir);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Carole.MacDonald 02/10
+---------------+---------------+---------------+---------------+---------------+------*/
WString ECTestFixture::GetTestDataPath(WCharCP dataFile)
    {
    BeFileName testData;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory (testData);
    testData.AppendToPath (L"SeedData");
    testData.AppendToPath (dataFile);
    return testData;
    } 

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                  Raimondas.Rimkus 02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
WString ECTestFixture::GetTempDataPath(WCharCP dataFile)
    {
    BeFileName testData;
    BeTest::GetHost().GetOutputRoot (testData);
    testData.AppendToPath (dataFile);
    return testData;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  02/2011
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

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    08/2017
//---------------+---------------+---------------+---------------+---------------+-------
void deserializeSchema(ECSchemaPtr& schema, ECSchemaReadContextR context, SchemaItem const& schemaItem, SchemaReadStatus expectedStatus, bool assert = false)
    {
    if (SchemaItem::Type::File == schemaItem.GetType())
        {
        auto filePath = ECTestFixture::GetTestDataPath(schemaItem.GetFileName().c_str());

        schema = ECSchema::LocateSchema(filePath.c_str(), context);
        if (SchemaReadStatus::Success == expectedStatus)
            ASSERT_TRUE(schema.IsValid());
        else
            ASSERT_FALSE(schema.IsValid());

        return;
        }
    
    SchemaReadStatus readStatus = ECSchema::ReadFromXmlString(schema, schemaItem.GetXmlString().c_str(), context);

    if (assert)
        {
        ASSERT_EQ(expectedStatus, readStatus);
        if (SchemaReadStatus::Success == expectedStatus)
            ASSERT_TRUE(schema.IsValid());
        else 
            ASSERT_FALSE(schema.IsValid());
        }
    else
        {
        EXPECT_EQ(expectedStatus, readStatus);
        if (SchemaReadStatus::Success == expectedStatus)
            EXPECT_TRUE(schema.IsValid());
        else
            EXPECT_FALSE(schema.IsValid());
        }    
    }

void ECTestFixture::DeserializeSchema(ECSchemaPtr& schema, ECSchemaReadContextR context, SchemaItem const& schemaItem, SchemaReadStatus expectedStatus)
    {
    deserializeSchema(schema, context, schemaItem, expectedStatus, true);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    08/2017
//---------------+---------------+---------------+---------------+---------------+-------
// static
void ECTestFixture::ExpectSchemaDeserializationFailure(SchemaItem const& schemaItem, SchemaReadStatus expectedError)
    {
    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    deserializeSchema(schema, *context, schemaItem, SchemaReadStatus::InvalidECSchemaXml, false);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    08/2017
//---------------+---------------+---------------+---------------+---------------+-------
// static
void ECTestFixture::AssertSchemaDeserializationFailure(SchemaItem const& schemaItem, SchemaReadStatus expectedError)
    {
    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    deserializeSchema(schema, *context, schemaItem, SchemaReadStatus::InvalidECSchemaXml, true);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  Muhammad.Zaighum                  05/13
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

bool ECTestUtility::JsonDeepEqual(Json::Value const& a, Json::Value const& b)
    {
    return a.ToString() == b.ToString();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                           Victor.Cushman                          11/2017
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String ECTestUtility::JsonSchemasComparisonString(Json::Value const& createdSchema, Json::Value const& testDataSchema)
    {
    return "Created Schema   (minified): " + createdSchema.ToString() + '\n' +
           "Test Data Schema (minified): " + testDataSchema.ToString() + '\n' +
           "Created Schema   (pretty):\n"  + createdSchema.toStyledString() + '\n' +
           "Test Data Schema (pretty):\n"  + testDataSchema.toStyledString();
    }

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
        LOG.errorv("CompareECInstances> Instances are not equal: Differing property values property '%s' (%d %d): actual: %s, expected: %s", 
                   propertyName.c_str(), actualKind, actualType, actualValueWstr.c_str(), expectedValueWStr.c_str());
        return false;
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Affan.Khan                        03/12
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

