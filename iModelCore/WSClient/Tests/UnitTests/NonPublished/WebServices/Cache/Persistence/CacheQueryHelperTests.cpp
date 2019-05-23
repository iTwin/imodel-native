/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#include "CacheQueryHelperTests.h"

#include <WebServices/Cache/Persistence/CacheQueryHelper.h>
#include <WebServices/Cache/Persistence/DataReadOptions.h> // As Stub for ISelectProvider

using namespace ::testing;
USING_NAMESPACE_BENTLEY_WEBSERVICES

ECSchemaPtr CacheQueryHelperTests::GetTestSchema()
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

struct TestSelectProvider : ISelectProvider
    {
    std::shared_ptr<SelectProperties> m_selectProperties;
    int m_sortPrority = 0;
    SortProperties m_sortProperties;

    virtual std::shared_ptr<SelectProperties> GetSelectProperties(ECClassCR ecClass) const { return m_selectProperties; };
    virtual int GetSortPriority(ECClassCR ecClass) const { return m_sortPrority; };
    virtual SortProperties GetSortProperties(ECClassCR ecClass) const { return m_sortProperties; };
    };

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CacheQueryHelperTests, CreateReadInfos_EmptyOptions_ReturnsGivenClasses)
    {
    auto schema = GetTestSchema();

    bvector<ECClassCP> classes;
    classes.push_back(schema->GetClassCP("Table"));
    classes.push_back(schema->GetClassCP("Furniture"));

    DataReadOptions options;
    auto infos = CacheQueryHelper(options).CreateReadInfos(classes);

    EXPECT_EQ(2, infos.size());
    EXPECT_EQ(schema->GetClassCP("Table"), &infos[0].GetECClass());
    EXPECT_EQ(schema->GetClassCP("Furniture"), &infos[1].GetECClass());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CacheQueryHelperTests, CreateReadInfos_OptionsWithSelectClass_ConstructsInfosWithOnlySelectedClasses)
    {
    auto schema = GetTestSchema();

    DataReadOptions options;
    options.SelectClass("TestSchema.Furniture");

    bvector<ECClassCP> classes;
    classes.push_back(schema->GetClassCP("Table"));
    classes.push_back(schema->GetClassCP("Furniture"));

    auto infos = CacheQueryHelper(options).CreateReadInfos(classes);

    EXPECT_EQ(1, infos.size());
    EXPECT_EQ(schema->GetClassCP("Furniture"), &infos[0].GetECClass());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CacheQueryHelperTests, CreateReadInfos_EmptyOptions_ReturnsSelectAllAndNoSorting)
    {
    auto schema = GetTestSchema();

    DataReadOptions options;
    auto infos = CacheQueryHelper(options).CreateReadInfos(schema->GetClassCP("Table"));

    EXPECT_EQ(1, infos.size());
    EXPECT_TRUE(infos[0].GetSelectProperties().GetSelectAll());
    EXPECT_TRUE(infos[0].GetSortProperties().empty());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CacheQueryHelperTests, CreateReadInfos_OptionsWithOrderBy_ReturnsOrderingOptions)
    {
    auto schema = GetTestSchema();

    bvector<DataReadOptions::OrderedProperty> properties;
    properties.push_back(DataReadOptions::OrderedProperty("Legs", false));
    properties.push_back(DataReadOptions::OrderedProperty("Name", true));

    DataReadOptions options;
    options.AddOrderByClassAndProperties("TestSchema.Table", properties);

    auto infos = CacheQueryHelper(options).CreateReadInfos(schema->GetClassCP("Table"));

    EXPECT_EQ(1, infos.size());
    EXPECT_EQ(2, infos[0].GetSortProperties().size());
    EXPECT_EQ("Legs", infos[0].GetSortProperties()[0].GetProperty().GetName());
    EXPECT_EQ("Name", infos[0].GetSortProperties()[1].GetProperty().GetName());
    EXPECT_EQ(false, infos[0].GetSortProperties()[0].GetSortAscending());
    EXPECT_EQ(true, infos[0].GetSortProperties()[1].GetSortAscending());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CacheQueryHelperTests, CreateReadInfos_OptionsWithSelectProperties_ReturnsSelectionOptions)
    {
    auto schema = GetTestSchema();

    DataReadOptions options;
    options.SelectClassAndProperty("TestSchema.Table", "Legs");
    options.SelectClassAndProperty("TestSchema.Table", "Name");

    auto ecClass = schema->GetClassCP("Table");
    auto infos = CacheQueryHelper(options).CreateReadInfos(ecClass);

    EXPECT_EQ(1, infos.size());
    EXPECT_EQ(ecClass, &infos[0].GetECClass());
    EXPECT_CONTAINS(infos[0].GetSelectProperties().GetProperties(), ecClass->GetPropertyP("Legs"));
    EXPECT_CONTAINS(infos[0].GetSelectProperties().GetProperties(), ecClass->GetPropertyP("Name"));
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CacheQueryHelperTests, ECSqlSelectPropertiesByWhereClause_EmptyOptions_ConstructsSelectQueryWithAllPropertiesAndRemoteId)
    {
    auto schema = GetTestSchema();

    DataReadOptions options;
    auto infos = CacheQueryHelper(options).CreateReadInfos(schema->GetClassCP("Table"));
    auto ecSql = CacheQueryHelper::ECSql::SelectPropertiesByWhereClause(infos.front(), "NULL");

    EXPECT_TRUE(Utf8String::npos != ecSql.find("SELECT info.[RemoteId], instance.* FROM"));
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CacheQueryHelperTests, ECSqlSelectPropertiesByWhereClause_EmptyOptions_ConstructsJoinWithInfo)
    {
    auto schema = GetTestSchema();

    DataReadOptions options;
    auto infos = CacheQueryHelper(options).CreateReadInfos(schema->GetClassCP("Table"));
    auto ecSql = CacheQueryHelper::ECSql::SelectPropertiesByWhereClause(infos.front(), "NULL");

    EXPECT_TRUE(Utf8String::npos != ecSql.find("JOIN ONLY [WSC].[CachedObjectInfo] info ON info.[InstanceId] = instance.ECInstanceId"));
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CacheQueryHelperTests, ECSqlSelectPropertiesByWhereClause_OptionsWithOrderBy_ConstructsQueryWithOrderBy)
    {
    auto schema = GetTestSchema();

    bvector<DataReadOptions::OrderedProperty> properties;
    properties.push_back(DataReadOptions::OrderedProperty("Legs", false));
    properties.push_back(DataReadOptions::OrderedProperty("Name", true));

    DataReadOptions options;
    options.AddOrderByClassAndProperties("TestSchema.Table", properties);
    options.SelectClassWithNoProperties("TestSchema.Table");

    auto infos = CacheQueryHelper(options).CreateReadInfos(schema->GetClassCP("Table"));
    auto ecSql = CacheQueryHelper::ECSql::SelectPropertiesByWhereClause(infos.front(), "NULL");

    EXPECT_TRUE(Utf8String::npos != ecSql.find("ORDER BY instance.[Legs] DESC, LOWER (instance.[Name]) ASC"));
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CacheQueryHelperTests, ECSqlSelectPropertiesByWhereClause_OptionsWithSelectProperties_ConstructsQueryWithSelectedProperties)
    {
    auto schema = GetTestSchema();

    DataReadOptions options;
    options.SelectClassAndProperty("TestSchema.Table", "Legs");
    options.SelectClassAndProperty("TestSchema.Table", "Name");

    auto infos = CacheQueryHelper(options).CreateReadInfos(schema->GetClassCP("Table"));
    auto ecSql = CacheQueryHelper::ECSql::SelectPropertiesByWhereClause(infos.front(), "NULL");

    EXPECT_STREQ("SELECT instance.[Legs], instance.[Name], instance.[ECInstanceId], instance.[ECClassId] FROM ONLY [TestSchema].[Table] instance WHERE NULL ", ecSql.c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CacheQueryHelperTests, ECSqlSelectPropertiesByWhereClause_OptionsWithSelectNoProperties_ConstructsQueryWithSelectECInstanceId)
    {
    auto schema = GetTestSchema();

    DataReadOptions options;
    options.SelectClassWithNoProperties("TestSchema.Table");

    auto infos = CacheQueryHelper(options).CreateReadInfos(schema->GetClassCP("Table"));
    auto ecSql = CacheQueryHelper::ECSql::SelectPropertiesByWhereClause(infos.front(), "NULL");

    EXPECT_STREQ("SELECT instance.[ECInstanceId], instance.[ECClassId] FROM ONLY [TestSchema].[Table] instance WHERE NULL ", ecSql.c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CacheQueryHelperTests, ECSqlSelectPropertiesByWhereClause_OptionsWithRemoteIdSelected_ConstructsQueryWithJoin)
    {
    auto schema = GetTestSchema();

    DataReadOptions options;
    options.SelectClassAndProperty("TestSchema.Table", DataSourceCache_PROPERTY_RemoteId);

    auto infos = CacheQueryHelper(options).CreateReadInfos(schema->GetClassCP("Table"));
    auto ecSql = CacheQueryHelper::ECSql::SelectPropertiesByWhereClause(infos.front(), "NULL");

    EXPECT_TRUE(Utf8String::npos != ecSql.find("JOIN ONLY [WSC].[CachedObjectInfo] info ON info.[InstanceId] = instance.ECInstanceId"));
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CacheQueryHelperTests, ECSqlSelectPropertiesByWhereClause_OptionsWithInstanceIdSelected_SelectsClassAndInstanceIds)
    {
    auto schema = GetTestSchema();

    TestSelectProvider provider;
    provider.m_selectProperties = std::make_shared<ISelectProvider::SelectProperties>();
    provider.m_selectProperties->SetSelectInstanceId(true);
    provider.m_selectProperties->SetSelectAll(false);

    auto infos = CacheQueryHelper(provider).CreateReadInfos(schema->GetClassCP("Table"));
    auto ecSql = CacheQueryHelper::ECSql::SelectPropertiesByWhereClause(infos.front(), "NULL");

    EXPECT_STREQ("SELECT instance.[ECInstanceId], instance.[ECClassId] FROM ONLY [TestSchema].[Table] instance WHERE NULL ", ecSql.c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     07/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CacheQueryHelperTests, ECSqlSelectPropertiesByWhereClause_OptionsWithInstanceIdNotSelected_DoesNotSelectClassAndInstanceIds)
    {
    auto schema = GetTestSchema();

    TestSelectProvider provider;
    provider.m_selectProperties = std::make_shared<ISelectProvider::SelectProperties>();
    provider.m_selectProperties->SetSelectInstanceId(false);
    provider.m_selectProperties->SetSelectAll(false);

    auto infos = CacheQueryHelper(provider).CreateReadInfos(schema->GetClassCP("Table"));
    auto ecSql = CacheQueryHelper::ECSql::SelectPropertiesByWhereClause(infos.front(), "NULL");

    EXPECT_STREQ("SELECT NULL FROM ONLY [TestSchema].[Table] instance WHERE NULL ", ecSql.c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                 Eimantas.Morkunas                   08/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CacheQueryHelperTests, ECSqlCreateOrderByClause_NoSortProperties_EmptyOrderByClause)
    {
    auto schema = GetTestSchema();
    ECClassCP ecClass = schema->GetClassCP("Table");

    ISelectProvider::SortProperties properties;
    auto orderBy = CacheQueryHelper::ECSql::CreateOrderByClause(properties, *ecClass, "alias");

    EXPECT_TRUE(orderBy.empty());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                 Eimantas.Morkunas                   08/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CacheQueryHelperTests, ECSqlCreateOrderByClause_ClassWithSortProperties_CorrectOrderByClause)
    {
    auto schema = GetTestSchema();
    ECClassCP ecClass = schema->GetClassCP("Table");

    bvector<DataReadOptions::OrderedProperty> properties;
    properties.push_back(DataReadOptions::OrderedProperty("Legs", false));
    properties.push_back(DataReadOptions::OrderedProperty("Name", true));

    DataReadOptions options;
    options.AddOrderByClassAndProperties(*ecClass, properties);

    auto orderBy = CacheQueryHelper::ECSql::CreateOrderByClause(options.GetSortProperties(*ecClass), *ecClass, "alias");

    EXPECT_STREQ("alias.[Legs] DESC, LOWER (alias.[Name]) ASC", orderBy.c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                 Eimantas.Morkunas                   08/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CacheQueryHelperTests, ECSqlCreateOrderByClause_SortOrderReversed_CorrectOrderByClause)
    {
    auto schema = GetTestSchema();
    ECClassCP ecClass = schema->GetClassCP("Table");

    bvector<DataReadOptions::OrderedProperty> properties;
    properties.push_back(DataReadOptions::OrderedProperty("Legs", false));
    properties.push_back(DataReadOptions::OrderedProperty("Name", true));

    DataReadOptions options;
    options.AddOrderByClassAndProperties(*ecClass, properties);

    auto orderBy = CacheQueryHelper::ECSql::CreateOrderByClause(options.GetSortProperties(*ecClass), *ecClass, "alias", true);

    EXPECT_STREQ("alias.[Legs] ASC, LOWER (alias.[Name]) DESC", orderBy.c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                 Petras.Sukys                   11/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CacheQueryHelperTests, ReadJsonInstance_ExistingInstance_ReturnsRapidJsonWithInstance)
    {
    auto cache = GetTestCache();
    DataReadOptions options;
    options.SelectClassAndProperty("TestSchema.Table", DataSourceCache_PROPERTY_RemoteId);
    auto instance = StubInstanceInCache(*cache, {"TestSchema.Table", "testId"});
    auto instanceClass = cache->GetAdapter().GetECClass(instance);

    auto infos = CacheQueryHelper(options).CreateReadInfos(instanceClass);
    ASSERT_EQ(1, infos.size());
    auto readInfo = infos[0];

    Utf8String idClause = "instance.ECInstanceId = ?";
    Utf8String ecSql = CacheQueryHelper::ECSql::SelectPropertiesByWhereClause(readInfo, idClause);

    ECSqlStatement statement;
    ASSERT_TRUE(SUCCESS == cache->GetAdapter().PrepareStatement(statement, ecSql));
    statement.BindId(1, instance.GetInstanceId());
    EXPECT_TRUE(DbResult::BE_SQLITE_ROW == statement.Step());
    rapidjson::Document document(rapidjson::kObjectType);
    ASSERT_TRUE(SUCCESS == CacheQueryHelper::ReadJsonInstance(readInfo, statement, document, document.GetAllocator()));
    EXPECT_STREQ("testId", document[DataSourceCache_PROPERTY_RemoteId].GetString());
    EXPECT_STREQ("TestSchema.Table", document[DataSourceCache_PROPERTY_ClassKey].GetString());
    Utf8String instanceIdString = document[DataSourceCache_PROPERTY_LocalInstanceId].GetString();
    uint64_t instanceId = 0;
    auto result = sscanf(instanceIdString.c_str(), "%" PRIx64, &instanceId);
    EXPECT_EQ(1, result);
    EXPECT_EQ(instance.GetInstanceId().GetValue(), instanceId);
    }
    
/*--------------------------------------------------------------------------------------+
* @bsitest                                 Petras.Sukys                   11/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CacheQueryHelperTests, ReadJsonInstance_ExistingInstanceWithProperties_ReturnsJsonValue)
    {
    auto cache = GetTestCache();
    DataReadOptions options;
    options.SelectClassAndProperty("TestSchema.Table", DataSourceCache_PROPERTY_RemoteId);
    auto instance = StubInstanceInCache(*cache, {"TestSchema.Table", "testId"});
    auto instanceClass = cache->GetAdapter().GetECClass(instance);

    auto infos = CacheQueryHelper(options).CreateReadInfos(instanceClass);
    ASSERT_EQ(1, infos.size());
    auto readInfo = infos[0];

    Utf8String idClause = "instance.ECInstanceId = ?";
    Utf8String ecSql = CacheQueryHelper::ECSql::SelectPropertiesByWhereClause(readInfo, idClause);

    ECSqlStatement statement;
    ASSERT_TRUE(SUCCESS == cache->GetAdapter().PrepareStatement(statement, ecSql));
    statement.BindId(1, instance.GetInstanceId());
    EXPECT_TRUE(DbResult::BE_SQLITE_ROW == statement.Step());
    Json::Value jsonValue;
    ASSERT_TRUE(SUCCESS == CacheQueryHelper::ReadJsonInstance(readInfo, statement, jsonValue));
    EXPECT_STREQ("testId", jsonValue[DataSourceCache_PROPERTY_RemoteId].asCString());
    EXPECT_STREQ("TestSchema.Table", jsonValue[DataSourceCache_PROPERTY_ClassKey].asCString());
    Utf8String instanceIdString = jsonValue[DataSourceCache_PROPERTY_LocalInstanceId].asCString();
    uint64_t instanceId = 0;
    auto result = sscanf(instanceIdString.c_str(), "%" PRIx64, &instanceId);
    EXPECT_EQ(1, result);
    EXPECT_EQ(instance.GetInstanceId().GetValue(), instanceId);
    }