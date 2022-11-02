/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <Bentley/BeTest.h>
#include <Bentley/BeFileName.h>
#include "../ECObjectsTestPCH.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

#define EC_ASSERT_SUCCESS(actual) ASSERT_EQ(ECObjectsStatus::Success, actual)
#define EC_EXPECT_SUCCESS(actual) EXPECT_EQ(ECObjectsStatus::Success, actual)

//=======================================================================================
// @bsiclass
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
private:
    static ECSchemaPtr s_unitsSchema;
    static ECSchemaPtr s_formatsSchema;
protected:
    ECTestFixture();
    virtual void SetUp() override;
    virtual void TearDown() override;

public:
    static WString GetTestDataPath(WCharCP fileName);
    static WString GetAssetsDataPath(std::vector<WString> pathBits);
    static WString GetAssetsGDataPath(std::vector<WString> pathBits);
    static WString GetTempDataPath(WCharCP fileName);
    static Utf8String GetDateTime();

    //! A static version of the standard units schema
    //! @param[in] recreate If true, the static schema will be initialized
    static ECSchemaPtr GetUnitsSchema(bool recreate = false);
    //! A static version of the standard formats schema
    //! @param[in] recreate If true, the static schema will be initialized
    static ECSchemaPtr GetFormatsSchema(bool recreate = false);

    static void DeserializeSchema(ECSchemaPtr& schema, ECSchemaReadContextR context, SchemaItem const& schemaItem, ECN::SchemaReadStatus expectedStatus = ECN::SchemaReadStatus::Success, Utf8CP failureMessage = "");
    static void ExpectSchemaDeserializationSuccess(Utf8CP schemaXml, Utf8CP failureMessage = "")
        {ExpectSchemaDeserializationFailure(SchemaItem(schemaXml), ECN::SchemaReadStatus::Success, failureMessage);}
    static void ExpectSchemaDeserializationFailure(SchemaItem const& schemaItem, ECN::SchemaReadStatus expectedStatus = ECN::SchemaReadStatus::InvalidECSchemaXml, Utf8CP failureMessage = "");
    static void ExpectSchemaDeserializationFailure(Utf8CP schemaXml, ECN::SchemaReadStatus expectedError = ECN::SchemaReadStatus::InvalidECSchemaXml, Utf8CP failureMessage = "")
        {ExpectSchemaDeserializationFailure(SchemaItem(schemaXml), expectedError, failureMessage);}
    static void AssertSchemaDeserializationFailure(SchemaItem const& schemaItem, ECN::SchemaReadStatus expectedError = ECN::SchemaReadStatus::InvalidECSchemaXml, Utf8CP failureMessage = "");
    static void AssertSchemaDeserializationFailure(Utf8CP schemaXml, ECN::SchemaReadStatus expectedError = ECN::SchemaReadStatus::InvalidECSchemaXml, Utf8CP failureMessage = "")
        {AssertSchemaDeserializationFailure(SchemaItem(schemaXml), expectedError, failureMessage);}

    static void RoundTripSchema(ECN::ECSchemaPtr& schema, SchemaItem item, ECVersion toVersion, SchemaReadStatus expectedReadStatus = ECN::SchemaReadStatus::Success, ECN::SchemaWriteStatus expectedWriteStatus = ECN::SchemaWriteStatus::Success, Utf8CP failureMessage = "");
    static void RoundTripSchema(ECN::ECSchemaPtr& schema, ECSchemaCP inSchema, ECVersion toVersion, SchemaReadStatus expectedReadStatus = ECN::SchemaReadStatus::Success, ECN::SchemaWriteStatus expectedWriteStatus = ECN::SchemaWriteStatus::Success, Utf8CP failureMessage = "");
};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct ECTestUtility
{
    static BentleyStatus ReadJsonInputFromFile(Json::Value& jsonInput, BeFileName& jsonFilePath);
    static bool JsonDeepEqual(Json::Value const& a, Json::Value const& b);
    static Utf8String JsonSchemasComparisonString(Json::Value const& createdSchema, Json::Value const& testDataSchema);
    static bool CompareECInstances(ECN::IECInstanceCR expected, ECN::IECInstanceCR actual);
};

struct CopyTestFixture : ECTestFixture 
{
ECSchemaPtr m_sourceSchema;
ECSchemaPtr m_targetSchema;

public:
    void TearDown() override { m_targetSchema = nullptr; m_sourceSchema = nullptr; }
    void CreateTestSchema() {EC_ASSERT_SUCCESS(ECSchema::CreateSchema(m_sourceSchema, "TestSchema", "ts", 1, 0, 0));}

    template<typename T>
    static void ValidateNameDescriptionAndDisplayLabel(T const & sourceItem, T const& targetItem)
        {
        EXPECT_STREQ(sourceItem.GetName().c_str(), targetItem.GetName().c_str()) <<
            "The name '" << sourceItem.GetName().c_str() << "' does not match the name '" << targetItem.GetName().c_str();
        EXPECT_NE(sourceItem.GetName().c_str(), targetItem.GetName().c_str()) <<
            "The description of '" << sourceItem.GetName().c_str() << "' is the same in memory object as the name of the target item '" << targetItem.GetName().c_str() << "'. It should be a copy.";

        EXPECT_STREQ(sourceItem.GetInvariantDescription().c_str(), targetItem.GetInvariantDescription().c_str()) <<
            "The description '" << sourceItem.GetInvariantDescription().c_str() << "' does not match the copied description '" << targetItem.GetInvariantDescription().c_str();
        EXPECT_NE(sourceItem.GetInvariantDescription().c_str(), targetItem.GetInvariantDescription().c_str()) <<
            "The description of '" << sourceItem.GetName().c_str() << "' is the same in memory object as the description of the target item '" << targetItem.GetName().c_str() << "'. It should be a copy.";

        EXPECT_STREQ(sourceItem.GetInvariantDisplayLabel().c_str(), targetItem.GetInvariantDisplayLabel().c_str()) << 
            "The display label '" << sourceItem.GetInvariantDisplayLabel().c_str() << "' does not match the copied display label '" << targetItem.GetInvariantDisplayLabel().c_str();
        EXPECT_NE(sourceItem.GetInvariantDisplayLabel().c_str(), targetItem.GetInvariantDisplayLabel().c_str()) <<
            "The display label of '" << sourceItem.GetName().c_str() << "' is the same in memory object as the display label of the target item '" << targetItem.GetName().c_str() << "'. It should be a copy.";
        }
};

struct StringSchemaLocater : IECSchemaLocater, NonCopyableClass
    {
private:
    bmap<Utf8String, Utf8String> m_schemaXml;
protected:
    ECSchemaPtr _LocateSchema(SchemaKeyR key, SchemaMatchType matchType, ECSchemaReadContextR schemaContext) override
        {
        ECSchemaPtr schema;
        auto schemaXml = m_schemaXml.find(key.m_schemaName);
        if (schemaXml != m_schemaXml.end())
            ECSchema::ReadFromXmlString(schema, schemaXml->second.c_str(), schemaContext);
        return schema;
        }
public:
    void AddSchemaString(Utf8String schemaName, Utf8String schemaXml) {m_schemaXml[schemaName] = schemaXml;}
    };

END_BENTLEY_ECN_TEST_NAMESPACE
