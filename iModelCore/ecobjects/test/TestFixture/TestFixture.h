/*--------------------------------------------------------------------------------------+
|
|     $Source: test/TestFixture/TestFixture.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <Bentley/BeTest.h>
#include <Bentley/BeFileName.h>
#include "../ECObjectsTestPCH.h"

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

#define EC_ASSERT_SUCCESS(actual) ASSERT_EQ(ECObjectsStatus::Success, actual)
#define EC_EXPECT_SUCCESS(actual) EXPECT_EQ(ECObjectsStatus::Success, actual)

//=======================================================================================
// @bsiclass                                   Krischan.Eberle                  07/15
//=======================================================================================
struct SchemaItem final
    {
    public:
        enum class Type
            {
            String,
            File
            };

    private:
        Type m_type;
        Utf8String m_xmlStringOrFileName;

        SchemaItem(Type type, Utf8StringCR xmlStringOrFileName) : m_type(type), m_xmlStringOrFileName(xmlStringOrFileName) {}

    public:
        explicit SchemaItem(Utf8StringCR xmlString) : SchemaItem(Type::String, xmlString) {}
        static SchemaItem CreateForFile(Utf8StringCR schemaFileName) { return SchemaItem(Type::File, schemaFileName); }

        Type GetType() const { return m_type; }
        Utf8StringCR GetXmlString() const { BeAssert(m_type == Type::String);  return m_xmlStringOrFileName; }
        BeFileName GetFileName() const { BeAssert(m_type == Type::File); return BeFileName(m_xmlStringOrFileName.c_str(), true); }
        Utf8StringCR ToString() const { return m_xmlStringOrFileName; }
    };

//=======================================================================================
// @bsiclass
//=======================================================================================
struct ECTestFixture : ::testing::Test
    {
protected:
    ECTestFixture();
    
public:
    static WString GetTestDataPath(WCharCP fileName);
    static WString GetTempDataPath(WCharCP fileName);
    static Utf8String GetDateTime();

    static void DeserializeSchema(ECN::ECSchemaPtr& schema, ECN::ECSchemaReadContextR context, SchemaItem const& schemaItem, ECN::SchemaReadStatus expectedStatus = ECN::SchemaReadStatus::Success);
    static void ExpectSchemaDeserializationFailure(SchemaItem const& schemaItem, ECN::SchemaReadStatus expectedStatus = ECN::SchemaReadStatus::InvalidECSchemaXml);
    static void ExpectSchemaDeserializationFailure(Utf8CP schemaXml, ECN::SchemaReadStatus expectedError = ECN::SchemaReadStatus::InvalidECSchemaXml) 
        {
        ExpectSchemaDeserializationFailure(SchemaItem(schemaXml), expectedError);
        };
    static void AssertSchemaDeserializationFailure(SchemaItem const& schemaItem, ECN::SchemaReadStatus expectedError = ECN::SchemaReadStatus::InvalidECSchemaXml);
    static void AssertSchemaDeserializationFailure(Utf8CP schemaXml, ECN::SchemaReadStatus expectedError = ECN::SchemaReadStatus::InvalidECSchemaXml)
        {
        AssertSchemaDeserializationFailure(SchemaItem(schemaXml), expectedError);
        };
    };

//=======================================================================================
// @bsiclass                                   Caleb.Shafer                     08/17
//=======================================================================================
struct ECTestUtility
    {
    static BentleyStatus ReadJsonInputFromFile(Json::Value& jsonInput, BeFileName& jsonFilePath);
    static bool CompareECInstances(ECN::IECInstanceCR expected, ECN::IECInstanceCR actual);
    };



END_BENTLEY_ECN_TEST_NAMESPACE
