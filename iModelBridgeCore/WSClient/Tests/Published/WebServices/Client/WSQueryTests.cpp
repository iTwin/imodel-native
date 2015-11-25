/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/WebServices/Client/WSQueryTests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "WSQueryTests.h"
#include <WebServices/Client/WSQuery.h>

using namespace ::testing;
using namespace ::std;
USING_NAMESPACE_BENTLEY_WEBSERVICES

TEST_F(WSQueryTests, Ctor_SchemaAndClassesPassed_SetsSchemaAndClasses)
    {
    WSQuery query("TestSchema", set<Utf8String> {"A", "B"});
    EXPECT_STREQ(query.GetSchemaName().c_str(), "TestSchema");
#ifdef USE_GTEST
    EXPECT_THAT(query.GetClasses(), ContainerEq(set<Utf8String>{"A", "B"}));
#endif
    }

TEST_F(WSQueryTests, Ctor_SchemaAndClass_SetsSchemaAndClass)
    {
    WSQuery query("TestSchema", "TestClass");
    EXPECT_STREQ(query.GetSchemaName().c_str(), "TestSchema");
#ifdef USE_GTEST
    EXPECT_THAT(query.GetClasses(), ContainerEq(set<Utf8String>{"TestClass"}));
#endif
    }

TEST_F(WSQueryTests, Ctor_ECClassPassed_SetsSchemaAndClass)
    {
    auto schema = ParseSchema(R"xml(
        <ECSchema schemaName="TestSchema" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            <ECClass typeName="TestClass" />
        </ECSchema>)xml");

    WSQuery query(*schema->GetClassCP(L"TestClass"));
    EXPECT_STREQ(query.GetSchemaName().c_str(), "TestSchema");
#ifdef USE_GTEST
    EXPECT_THAT(query.GetClasses(), ContainerEq(set<Utf8String>{"TestClass"}));
#endif
    }

TEST_F(WSQueryTests, Ctor_ECClassPassedWithPolymorphictrue_SetsSchemaAndPolymorphicClass)
    {
    auto schema = ParseSchema(R"xml(
        <ECSchema schemaName="TestSchema" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            <ECClass typeName="TestClass" />
        </ECSchema>)xml");

    WSQuery query(*schema->GetClassCP(L"TestClass"), true);
    EXPECT_STREQ(query.GetSchemaName().c_str(), "TestSchema");
#ifdef USE_GTEST
    EXPECT_THAT(query.GetClasses(), ContainerEq(set<Utf8String>{"TestClass!poly"}));
#endif
    }

TEST_F(WSQueryTests, Ctor_ObjectIdPassed_SetsSchemaAndClassAndFilter)
    {
    WSQuery query(ObjectId("TestSchema", "TestClass", "TestId"));
    EXPECT_STREQ(query.GetSchemaName().c_str(), "TestSchema");
#ifdef USE_GTEST
    EXPECT_THAT(query.GetClasses(), ContainerEq(set<Utf8String>{"TestClass"}));
#endif
    EXPECT_STREQ(query.GetFilter().c_str(), "$id+in+['TestId']");
    }

TEST_F(WSQueryTests, Ctor_ObjectIdPassed_SetsSchemaAndClassAndEscapesRemoteId)
    {
    WSQuery query(ObjectId("Foo", "Boo", "'"));
    EXPECT_STREQ(query.GetFilter().c_str(), "$id+in+['%27%27']");
    }

TEST_F(WSQueryTests, ToFullString_MultipleClassesAndSchema_AddsEverythingToQueryString)
    {
    WSQuery query("Schema", set<Utf8String>{"A", "B"});
    query.SetFilter("TestFilter");
    EXPECT_STREQ("Schema/A,B?$filter=TestFilter", query.ToFullString().c_str());
    }

TEST_F(WSQueryTests, ToQueryString_NoOptionsSet_ReturnsEmpty)
    {
    WSQuery query("Foo", "Boo");
    EXPECT_STREQ("", query.ToQueryString().c_str());
    }

TEST_F(WSQueryTests, ToQueryString_SelectOptionSet_FormatsOption)
    {
    WSQuery query("Foo", "Boo");
    query.SetSelect("TestSelect");
    EXPECT_STREQ("$select=TestSelect", query.ToQueryString().c_str());
    }

TEST_F(WSQueryTests, ToQueryString_AddSelectCalledMultipleTimes_AddsOptionsToSelectClause)
    {
    WSQuery query("Foo", "Boo");
    query.AddSelect("A");
    EXPECT_STREQ("$select=A", query.ToQueryString().c_str());
    query.AddSelect("B,C");
    EXPECT_STREQ("$select=A,B,C", query.ToQueryString().c_str());
    query.AddSelect("D");
    EXPECT_STREQ("$select=A,B,C,D", query.ToQueryString().c_str());
    }

TEST_F(WSQueryTests, ToQueryString_FilterOptionSet_FormatsOption)
    {
    WSQuery query("Foo", "Boo");
    query.SetFilter("TestFilter");
    EXPECT_STREQ("$filter=TestFilter", query.ToQueryString().c_str());
    }

TEST_F(WSQueryTests, ToQueryString_OrderByOptionSet_FormatsOption)
    {
    WSQuery query("Foo", "Boo");
    query.SetOrderBy("TestOrderBy");
    EXPECT_STREQ("$orderby=TestOrderBy", query.ToQueryString().c_str());
    }

TEST_F(WSQueryTests, ToQueryString_SkipOptionSet_FormatsOption)
    {
    WSQuery query("Foo", "Boo");
    query.SetSkip(42);
    EXPECT_STREQ("$skip=42", query.ToQueryString().c_str());
    }

TEST_F(WSQueryTests, ToQueryString_SkipOptionReset_ReturnsEmpty)
    {
    WSQuery query("Foo", "Boo");
    query.SetSkip(42);
    query.SetSkip(0);
    EXPECT_STREQ("", query.ToQueryString().c_str());
    }

TEST_F(WSQueryTests, ToQueryString_TopOptionSet_FormatsOption)
    {
    WSQuery query("Foo", "Boo");
    query.SetTop(42);
    EXPECT_STREQ("$top=42", query.ToQueryString().c_str());
    }

TEST_F(WSQueryTests, ToQueryString_TopOptionReset_ReturnsEmpty)
    {
    WSQuery query("Foo", "Boo");
    query.SetTop(42);
    query.SetTop(0);
    EXPECT_STREQ("", query.ToQueryString().c_str());
    }

TEST_F(WSQueryTests, ToQueryString_CustomParametersSet_AddsParametersToQueryString)
    {
    WSQuery query("Foo", "Boo");
    query.SetSelect("TestSelect");
    query.SetCustomParameter("NameA", "ValueA");
    query.SetCustomParameter("NameB", "ValueB");
    EXPECT_STREQ("$select=TestSelect&NameA=ValueA&NameB=ValueB", query.ToQueryString().c_str());
    }

TEST_F(WSQueryTests, ToQueryString_CustomParametersSetWithNoValue_AddsParametersToQueryString)
    {
    WSQuery query("Foo", "Boo");
    query.SetCustomParameter("NameA", "");
    query.SetCustomParameter("NameB", "");
    EXPECT_STREQ("NameA=&NameB=", query.ToQueryString().c_str());
    }

TEST_F(WSQueryTests, ToQueryString_MultipleOptionsSet_FormatsAllOfThem)
    {
    WSQuery query("Foo", "Boo");
    query.SetFilter("TestFilter");
    query.SetSkip(4);
    query.SetTop(2);
    EXPECT_STREQ("$filter=TestFilter&$skip=4&$top=2", query.ToQueryString().c_str());
    }

TEST_F(WSQueryTests, RemoveCustomParameter_CustomParameterAdded_CustomParameterRemoved)
    {
    WSQuery query("Foo", "Boo");
    query.SetCustomParameter("NameA", "AAA");
    query.RemoveCustomParameter("NameA");
    EXPECT_STREQ("", query.ToQueryString().c_str());
    }

TEST_F(WSQueryTests, RemoveCustomParameter_NotExistingCustomParameterName_NothingRemoved)
    {
    WSQuery query("Foo", "Boo");
    query.SetCustomParameter("NameA", "AAA");
    query.RemoveCustomParameter("NotExistingName");
    EXPECT_STREQ("NameA=AAA", query.ToQueryString().c_str());
    }

TEST_F(WSQueryTests, GetAlias_CalledMultipleTimesWithDifferentPhrases_ReturnsDifferentAliases)
    {
    WSQuery query("Foo", "Boo");
    EXPECT_STREQ("@A", query.GetAlias("Phrase1").c_str());
    EXPECT_STREQ("@B", query.GetAlias("Phrase2").c_str());
    }

TEST_F(WSQueryTests, GetAlias_CalledMoreThanAlphabetLengthTimes_ReturnsAliasesPostfixedWith1)
    {
    WSQuery query("Foo", "Boo");

    for (int i = 0; i < 25; i++)
        {
        query.GetAlias(Utf8PrintfString("%d", i).c_str());
        }
    EXPECT_STREQ("@Z", query.GetAlias("Phrase0").c_str());

    EXPECT_STREQ("@A1", query.GetAlias("Phrase1").c_str());
    EXPECT_STREQ("@B1", query.GetAlias("Phrase2").c_str());
    }

TEST_F(WSQueryTests, GetAlias_CalledMoreThanTwiceAlphabetLengthTimes_ReturnsAliasesPostfixedWith2)
    {
    WSQuery query("Foo", "Boo");

    for (int i = 0; i < 51; i++)
        {
        query.GetAlias(Utf8PrintfString("%d", i).c_str());
        }
    EXPECT_STREQ("@Z1", query.GetAlias("Phrase0").c_str());

    EXPECT_STREQ("@A2", query.GetAlias("Phrase1").c_str());
    EXPECT_STREQ("@B2", query.GetAlias("Phrase2").c_str());
    }

TEST_F(WSQueryTests, GetAlias_CalledMultipleTimesWithSamePhrase_ReturnsSameAlias)
    {
    WSQuery query("Foo", "Boo");
    EXPECT_STREQ("@A", query.GetAlias("Phrase1").c_str());
    EXPECT_STREQ("@B", query.GetAlias("Phrase2").c_str());
    EXPECT_STREQ("@B", query.GetAlias("Phrase2").c_str());
    }

TEST_F(WSQueryTests, GetAliasMap_NoAliasesCreated_Empty)
    {
    WSQuery query("Foo", "Boo");
    EXPECT_TRUE(query.GetAliasMapping().empty());
    }

#ifdef USE_GTEST
TEST_F(WSQueryTests, GetAliasMap_AliasesCreated_ContainsAliases)
    {
    WSQuery query("Foo", "Boo");
    Utf8String aliasA = query.GetAlias("PhraseA");
    Utf8String aliasB = query.GetAlias("PhraseB");
    EXPECT_THAT(query.GetAliasMapping(), ContainerEq(std::map<Utf8String, Utf8String> { { "PhraseA", aliasA }, {"PhraseB", aliasB}}));
    }
#endif

TEST_F(WSQueryTests, GetPhrase_CalledWithNotGeneratedAliases_ReturnsNull)
    {
    WSQuery query("Foo", "Boo");
    EXPECT_EQ(nullptr, query.GetPhrase(""));
    EXPECT_EQ(nullptr, query.GetPhrase("@A"));
    }

TEST_F(WSQueryTests, GetPhrase_CalledWithGeneratedAliases_ReturnsPhrases)
    {
    WSQuery query("Foo", "Boo");
    EXPECT_STREQ("TestPhrase", query.GetPhrase(query.GetAlias("TestPhrase")));
    EXPECT_STREQ("42", query.GetPhrase(query.GetAlias("42")));
    }

TEST_F(WSQueryTests, ToQueryString_AliasesGenerated_FormatsAliasesToQuery)
    {
    WSQuery query("Foo", "Boo");
    query.GetAlias("Phrase1");
    query.GetAlias("Phrase2");
    query.SetSkip(42);
    EXPECT_STREQ("$skip=42&@A=Phrase1&@B=Phrase2", query.ToQueryString().c_str());
    }

TEST_F(WSQueryTests, EscapeValue_StringPassed_PercentEncodesSpecialSymbols)
    {
    EXPECT_STREQ("", WSQuery::EscapeValue("").c_str());
    EXPECT_STREQ("ABC", WSQuery::EscapeValue("ABC").c_str());
    EXPECT_STREQ("%27%27", WSQuery::EscapeValue("'").c_str());
    EXPECT_STREQ("%3A", WSQuery::EscapeValue(":").c_str());
    }

TEST_F(WSQueryTests, Equals_EqualQueries_True)
    {
    EXPECT_EQ(WSQuery("A", "B"), WSQuery("A", "B"));
    EXPECT_EQ(WSQuery("A", std::set<Utf8String> {"B", "C"}), WSQuery("A", std::set<Utf8String>{"C", "B"}));

    WSQuery query1("A", "B");
    WSQuery query2("A", "B");
    query1.SetSelect("Foo");
    query2.SetSelect("Foo");

    EXPECT_EQ(query1, query2);
    }

TEST_F(WSQueryTests, Equals_NonEqualQueries_False)
    {
    EXPECT_FALSE(WSQuery("A", "B") == WSQuery("A", "Other"));
    EXPECT_FALSE(WSQuery("A", "B") == WSQuery("Other", "B"));
    EXPECT_FALSE(WSQuery("A", std::set<Utf8String> {"B", "C"}) == WSQuery("A", std::set<Utf8String>{"C"}));

    WSQuery query1("A", "B");
    WSQuery query2("A", "B");
    query1.SetSelect("Foo");
    query2.SetSelect("Boo");

    EXPECT_FALSE(query1 == query2);
    }
