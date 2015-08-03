/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/WebServices/Cache/Persistence/CacheQueryHelperTests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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

TEST_F(CacheQueryHelperTests, CreateReadInfos_EmptyOptions_ReturnsSelectAllAndNoSorting)
    {
    auto schema = GetTestSchema();

    DataReadOptions options;
    auto infos = CacheQueryHelper(options).CreateReadInfos(schema->GetClassCP("Table"));

    EXPECT_EQ(1, infos.size());
    EXPECT_TRUE(infos[0].GetSelectProperties().GetSelectAll());
    EXPECT_TRUE(infos[0].GetSortProperties().empty());
    }

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

TEST_F(CacheQueryHelperTests, ECSqlSelectPropertiesByWhereClause_EmptyOptions_ConstructsSelectQueryWithAllPropertiesAndRemoteId)
    {
    auto schema = GetTestSchema();

    DataReadOptions options;
    auto infos = CacheQueryHelper(options).CreateReadInfos(schema->GetClassCP("Table"));
    auto ecSql = CacheQueryHelper::ECSql::SelectPropertiesByWhereClause(infos.front(), "NULL");

    EXPECT_TRUE(Utf8String::npos != ecSql.find("SELECT info.[RemoteId], instance.* FROM"));
    }

TEST_F(CacheQueryHelperTests, ECSqlSelectPropertiesByWhereClause_EmptyOptions_ConstructsJoinWithInfo)
    {
    auto schema = GetTestSchema();

    DataReadOptions options;
    auto infos = CacheQueryHelper(options).CreateReadInfos(schema->GetClassCP("Table"));
    auto ecSql = CacheQueryHelper::ECSql::SelectPropertiesByWhereClause(infos.front(), "NULL");

    EXPECT_TRUE(Utf8String::npos != ecSql.find("JOIN ONLY [DSC].[CachedObjectInfo] info USING [DSC].[CachedObjectInfoRelationship]"));
    }

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

TEST_F(CacheQueryHelperTests, ECSqlSelectPropertiesByWhereClause_OptionsWithSelectProperties_ConstructsQueryWithSelectedProperties)
    {
    auto schema = GetTestSchema();

    DataReadOptions options;
    options.SelectClassAndProperty("TestSchema.Table", "Legs");
    options.SelectClassAndProperty("TestSchema.Table", "Name");

    auto infos = CacheQueryHelper(options).CreateReadInfos(schema->GetClassCP("Table"));
    auto ecSql = CacheQueryHelper::ECSql::SelectPropertiesByWhereClause(infos.front(), "NULL");

    EXPECT_STREQ("SELECT instance.[Legs], instance.[Name], instance.[ECInstanceId] FROM ONLY [TestSchema].[Table] instance WHERE NULL ", ecSql.c_str());
    }

TEST_F(CacheQueryHelperTests, ECSqlSelectPropertiesByWhereClause_OptionsWithSelectNoProperties_ConstructsQueryWithSelectECInstanceId)
    {
    auto schema = GetTestSchema();

    DataReadOptions options;
    options.SelectClassWithNoProperties("TestSchema.Table");

    auto infos = CacheQueryHelper(options).CreateReadInfos(schema->GetClassCP("Table"));
    auto ecSql = CacheQueryHelper::ECSql::SelectPropertiesByWhereClause(infos.front(), "NULL");

    EXPECT_STREQ("SELECT instance.[ECInstanceId] FROM ONLY [TestSchema].[Table] instance WHERE NULL ", ecSql.c_str());
    }

TEST_F(CacheQueryHelperTests, ECSqlSelectPropertiesByWhereClause_OptionsWithRemoteIdSelected_ConstructsQueryWithJoin)
    {
    auto schema = GetTestSchema();

    DataReadOptions options;
    options.SelectClassAndProperty("TestSchema.Table", DataSourceCache_PROPERTY_RemoteId);

    auto infos = CacheQueryHelper(options).CreateReadInfos(schema->GetClassCP("Table"));
    auto ecSql = CacheQueryHelper::ECSql::SelectPropertiesByWhereClause(infos.front(), "NULL");

    EXPECT_TRUE(Utf8String::npos != ecSql.find("JOIN ONLY [DSC].[CachedObjectInfo] info USING [DSC].[CachedObjectInfoRelationship]"));
    }
