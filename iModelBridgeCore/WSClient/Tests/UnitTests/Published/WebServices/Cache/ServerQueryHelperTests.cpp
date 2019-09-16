/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#include "../../Utils/WebServicesTestsHelper.h"
#include <WebServices/Cache/Persistence/DataReadOptions.h>
#include <WebServices/Cache/Persistence/DataSourceCacheCommon.h>
#include <WebServices/Cache/ServerQueryHelper.h>

#include "Util/MockSelectProvider.h"

USING_NAMESPACE_BENTLEY_WEBSERVICES

struct ServerQueryHelperTests : WSClientBaseTest
    {
    static ECSchemaPtr GetTestSchema();
    static void AddCalculatedECPropertySpecification(
        ECSchemaPtr schema,
        Utf8StringCR className,
        Utf8StringCR propertyName,
        Utf8StringCR ecExpression);
    };

ECSchemaPtr ServerQueryHelperTests::GetTestSchema()
    {
    Utf8String schemaXml =
        R"(<ECSchema schemaName="TestSchema" nameSpacePrefix="TS" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">)"
        R"(  <ECClass typeName="Table" >)"
        R"(    <ECProperty propertyName="Name" typeName="string" />)"
        R"(    <ECProperty propertyName="Legs" typeName="int" />)"
        R"(  </ECClass>)"
        R"(  <ECClass typeName="Furniture" >)"
        R"(    <ECProperty propertyName="Name" typeName="string" />)"
        R"(  </ECClass>)"
        R"(</ECSchema>)";

    ECSchemaPtr schema;
    ECSchema::ReadFromXmlString(schema, schemaXml.c_str(), *ECSchemaReadContext::CreateContext());
    return schema;
    }

void ServerQueryHelperTests::AddCalculatedECPropertySpecification
(
ECSchemaPtr schema,
Utf8StringCR className,
Utf8StringCR propertyName,
Utf8StringCR ecExpression
)
    {
    ECClassP ecClass = schema->GetClassP(className.c_str());

    SchemaKey beStandardsSchemaKey = SchemaKey("Bentley_Standard_CustomAttributes", 1, 0);
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ECSchemaPtr beStandardsSchema = context->LocateSchema(beStandardsSchemaKey, SchemaMatchType::LatestWriteCompatible);

    ECClassCP caClass = beStandardsSchema->GetClassCP("CalculatedECPropertySpecification");
    IECInstancePtr caInstance = caClass->GetDefaultStandaloneEnabler()->CreateInstance();

    EXPECT_EQ(ECObjectsStatus::Success, caInstance->SetValue("ECExpression", ECValue(ecExpression.c_str())));
    EXPECT_EQ(ECObjectsStatus::Success, schema->AddReferencedSchema(*beStandardsSchema));
    EXPECT_EQ(ECObjectsStatus::Success, ecClass->GetPropertyP(propertyName.c_str())->SetCustomAttribute(*caInstance.get()));
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ServerQueryHelperTests, GetAllSelectedProperties_EmptyOptions_ReturnsEmpty)
    {
    auto schema = GetTestSchema();
    bvector<ECN::ECSchemaCP> schemas;
    schemas.push_back(schema.get());

    DataReadOptions options;
    EXPECT_TRUE(ServerQueryHelper(options).GetAllSelectedProperties(schemas).empty());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ServerQueryHelperTests, GetAllSelectedProperties_AllClassesSelected_ReturnsEmpty)
    {
    auto schema = GetTestSchema();
    bvector<ECN::ECSchemaCP> schemas;
    schemas.push_back(schema.get());

    DataReadOptions options;
    options.SelectAllClasses();

    EXPECT_TRUE(ServerQueryHelper(options).GetAllSelectedProperties(schemas).empty());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ServerQueryHelperTests, GetAllSelectedProperties_MultipleClassesSelected_ReturnsEmpty)
    {
    auto schema = GetTestSchema();
    bvector<ECN::ECSchemaCP> schemas;
    schemas.push_back(schema.get());

    DataReadOptions options;
    options.SelectClass("TestSchema.Table");
    options.SelectClass("TestSchema.Furniture");

    EXPECT_TRUE(ServerQueryHelper(options).GetAllSelectedProperties(schemas).empty());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ServerQueryHelperTests, GetAllSelectedProperties_MultiplePropertiesSelectedFromClasses_ReturnsProperties)
    {
    auto schema = GetTestSchema();
    bvector<ECN::ECSchemaCP> schemas;
    schemas.push_back(schema.get());

    DataReadOptions options;
    options.SelectClassAndProperty("TestSchema.Table", "Name");
    options.SelectClassAndProperty("TestSchema.Table", "Legs");
    options.SelectClassAndProperty("TestSchema.Furniture", "Name");

    auto properties = ServerQueryHelper(options).GetAllSelectedProperties(schemas);

    EXPECT_EQ(2, properties.size());
    EXPECT_CONTAINS(properties, "Name");
    EXPECT_CONTAINS(properties, "Legs");
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ServerQueryHelperTests, GetAllSelectedProperties_DataSourceRemoteIdAndECInstanceIdPropertiesAdded_ExcludesCacheProperties)
    {
    auto schema = GetTestSchema();
    bvector<ECN::ECSchemaCP> schemas;
    schemas.push_back(schema.get());

    DataReadOptions options;
    options
        .SelectClassAndProperty("TestSchema.Table", "Legs")
        .SelectClassAndProperty("TestSchema.Table", "Name")
        .SelectClassAndProperty("TestSchema.Table", DataSourceCache_PROPERTY_RemoteId)
        .SelectClassAndProperty("TestSchema.Table", "ECInstanceId");

    auto properties = ServerQueryHelper(options).GetAllSelectedProperties(schemas);

    EXPECT_CONTAINS(properties, "Legs");
    EXPECT_CONTAINS(properties, "Name");
    EXPECT_NCONTAIN(properties, DataSourceCache_PROPERTY_RemoteId);
    EXPECT_NCONTAIN(properties, "ECInstanceId");
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ServerQueryHelperTests, GetAllSelectedProperties_ECExpressionHasInvalidProperty_ReturnsSelectedPropertyOnly)
    {
    auto schema = GetTestSchema();
    AddCalculatedECPropertySpecification(schema, "Table", "Name", R"( IIf (this.OtherProperty, "Foo", "Boo") )");

    DataReadOptions options;
    options.SelectClassAndProperty("TestSchema.Table", "Name");

    bvector<ECN::ECSchemaCP> schemas;
    schemas.push_back(schema.get());

    auto properties = ServerQueryHelper(options).GetAllSelectedProperties(schemas);

    EXPECT_EQ(1, properties.size());
    EXPECT_CONTAINS(properties, "Name");
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ServerQueryHelperTests, GetAllSelectedProperties_PropertyWithECExpression_IncludesRequiredProperties)
    {
    auto schema = GetTestSchema();
    AddCalculatedECPropertySpecification(schema, "Table", "Name", R"( IIf (this.Legs, "Foo", "Boo") )");

    DataReadOptions options;
    options.SelectClassAndProperty("TestSchema.Table", "Name");

    bvector<ECN::ECSchemaCP> schemas;
    schemas.push_back(schema.get());

    auto properties = ServerQueryHelper(options).GetAllSelectedProperties(schemas);

    EXPECT_EQ(2, properties.size());
    EXPECT_CONTAINS(properties, "Name");
    EXPECT_CONTAINS(properties, "Legs");
    }

#ifdef USE_GTEST

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ServerQueryHelperTests, GetSelect_SelectProviderReturnsSelectAll_ReturnsEmpty)
    {
    auto schema = GetTestSchema();

    auto properties = std::make_shared<ISelectProvider::SelectProperties>();

    MockSelectProvider provider;
    EXPECT_CALL(provider, GetSelectProperties(_)).WillOnce(Return(properties));

    auto select = ServerQueryHelper(provider).GetSelect(*schema->GetClassCP("Table"));
    EXPECT_STREQ("", select.c_str());
    }
#endif

#ifdef USE_GTEST

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ServerQueryHelperTests, GetSelect_SelectProviderReturnsNull_ReturnsSelectId)
    {
    auto schema = GetTestSchema();

    MockSelectProvider provider;
    EXPECT_CALL(provider, GetSelectProperties(_)).WillOnce(Return(nullptr));

    auto select = ServerQueryHelper(provider).GetSelect(*schema->GetClassCP("Table"));
    EXPECT_STREQ("$id", select.c_str());
    }
#endif

#ifdef USE_GTEST

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ServerQueryHelperTests, GetSelect_SelectProviderReturnsSelectNoProperties_ReturnsSelectId)
    {
    auto schema = GetTestSchema();

    auto properties = std::make_shared<ISelectProvider::SelectProperties>();
    properties->SetSelectAll(false);
    properties->SetSelectInstanceId(false);

    MockSelectProvider provider;
    EXPECT_CALL(provider, GetSelectProperties(_)).WillOnce(Return(properties));

    auto select = ServerQueryHelper(provider).GetSelect(*schema->GetClassCP("Table"));
    EXPECT_STREQ("$id", select.c_str());
    }
#endif

#ifdef USE_GTEST

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ServerQueryHelperTests, GetSelect_SelectProviderReturnsSelectSpecificProperties_ReturnsSelectProperties)
    {
    auto schema = GetTestSchema();

    auto properties = std::make_shared<ISelectProvider::SelectProperties>();
    properties->AddProperty(schema->GetClassCP("Table")->GetPropertyP("Name"));
    properties->AddProperty(schema->GetClassCP("Table")->GetPropertyP("Legs"));

    MockSelectProvider provider;
    EXPECT_CALL(provider, GetSelectProperties(_)).WillOnce(Return(properties));

    auto select = ServerQueryHelper(provider).GetSelect(*schema->GetClassCP("Table"));
    EXPECT_THAT(select, AnyOf(Eq("Name,Legs"), Eq("Legs,Name")));
    }
#endif

#ifdef USE_GTEST

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ServerQueryHelperTests, GetSelect_PropertyWithECExpression_IncludesRequiredProperties)
    {
    auto schema = GetTestSchema();
    AddCalculatedECPropertySpecification(schema, "Table", "Name", R"( IIf (this.Legs, "Foo", "Boo") )");

    auto properties = std::make_shared<ISelectProvider::SelectProperties>();
    properties->AddProperty(schema->GetClassCP("Table")->GetPropertyP("Name"));

    MockSelectProvider provider;
    EXPECT_CALL(provider, GetSelectProperties(_)).WillOnce(Return(properties));

    auto select = ServerQueryHelper(provider).GetSelect(*schema->GetClassCP("Table"));
    EXPECT_THAT(select, AnyOf(Eq("Name,Legs"), Eq("Legs,Name")));
    }
#endif
