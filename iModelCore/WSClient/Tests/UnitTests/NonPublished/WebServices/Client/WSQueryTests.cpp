/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "WSQueryTests.h"
#include <WebServices/Client/WSQuery.h>

using namespace ::testing;
using namespace ::std;
USING_NAMESPACE_BENTLEY_WEBSERVICES

#ifdef USE_GTEST
/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSQueryTests, Ctor_SchemaAndClassesPassed_SetsSchemaAndClasses)
    {
    WSQuery query("TestSchema", set<Utf8String> {"A", "B"});
    EXPECT_STREQ(query.GetSchemaName().c_str(), "TestSchema");
    EXPECT_THAT(query.GetClasses(), ContainerEq(set<Utf8String>{"A", "B"}));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSQueryTests, Ctor_SchemaAndClass_SetsSchemaAndClass)
    {
    WSQuery query("TestSchema", "TestClass");
    EXPECT_STREQ(query.GetSchemaName().c_str(), "TestSchema");
    EXPECT_THAT(query.GetClasses(), ContainerEq(set<Utf8String>{"TestClass"}));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSQueryTests, Ctor_SchemaAndClassWithPolymorphicTrue_SetsSchemaAndPolymorphicClass)
    {
    WSQuery query("TestSchema", "TestClass", true);
    EXPECT_STREQ(query.GetSchemaName().c_str(), "TestSchema");
    EXPECT_THAT(query.GetClasses(), ContainerEq(set<Utf8String>{"TestClass!poly"}));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSQueryTests, Ctor_ECClassPassed_SetsSchemaAndClass)
    {
    auto schema = ParseSchema(R"xml(
        <ECSchema schemaName="TestSchema" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            <ECClass typeName="TestClass" />
        </ECSchema>)xml");

    WSQuery query (*schema->GetClassCP ("TestClass"));
    EXPECT_STREQ(query.GetSchemaName().c_str(), "TestSchema");
    EXPECT_THAT(query.GetClasses(), ContainerEq(set<Utf8String>{"TestClass"}));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSQueryTests, Ctor_ECClassPassedWithPolymorphictrue_SetsSchemaAndPolymorphicClass)
    {
    auto schema = ParseSchema(R"xml(
        <ECSchema schemaName="TestSchema" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
            <ECClass typeName="TestClass" />
        </ECSchema>)xml");

    WSQuery query (*schema->GetClassCP ("TestClass"), true);
    EXPECT_STREQ(query.GetSchemaName().c_str(), "TestSchema");
    EXPECT_THAT(query.GetClasses(), ContainerEq(set<Utf8String>{"TestClass!poly"}));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSQueryTests, Ctor_ObjectIdPassed_SetsSchemaAndClassAndFilter)
    {
    WSQuery query(ObjectId("TestSchema", "TestClass", "TestId"));
    EXPECT_STREQ(query.GetSchemaName().c_str(), "TestSchema");
    EXPECT_THAT(query.GetClasses(), ContainerEq(set<Utf8String>{"TestClass"}));
    EXPECT_STREQ(query.GetFilter().c_str(), "$id+in+['TestId']");
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSQueryTests, Ctor_ObjectIdPassedWithPolymorphicTrue_SetsSchemaAndClassAndFilter)
    {
    WSQuery query(ObjectId("TestSchema", "TestClass", "TestId"), true);
    EXPECT_STREQ(query.GetSchemaName().c_str(), "TestSchema");
    EXPECT_THAT(query.GetClasses(), ContainerEq(set<Utf8String>{"TestClass!poly"}));
    EXPECT_STREQ(query.GetFilter().c_str(), "$id+in+['TestId']");
    }
#endif

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSQueryTests, Ctor_ObjectIdPassed_SetsSchemaAndClassAndEscapesRemoteId)
    {
    WSQuery query(ObjectId("Foo", "Boo", "'"));
    EXPECT_STREQ(query.GetFilter().c_str(), "$id+in+['%27%27']");
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSQueryTests, ToFullString_MultipleClassesAndSchema_AddsEverythingToQueryString)
    {
    WSQuery query("Schema", set<Utf8String>{"A", "B"});
    query.SetFilter("TestFilter");
    EXPECT_STREQ("Schema/A,B?$filter=TestFilter", query.ToFullString().c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSQueryTests, ToQueryString_NoOptionsSet_ReturnsEmpty)
    {
    WSQuery query("Foo", "Boo");
    EXPECT_STREQ("", query.ToQueryString().c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSQueryTests, ToQueryString_SelectOptionSet_FormatsOption)
    {
    WSQuery query("Foo", "Boo");
    query.SetSelect("TestSelect");
    EXPECT_STREQ("$select=TestSelect", query.ToQueryString().c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSQueryTests, ToQueryString_AddSelectEmpty_Ignores)
    {
    WSQuery query("Foo", "Boo");
    query.AddSelect("");
    query.AddSelect("");
    query.AddSelect("");
    EXPECT_STREQ("", query.ToQueryString().c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSQueryTests, ToQueryString_AddSelectEmptyOnNonEmpty_Ignores)
    {
    WSQuery query("Foo", "Boo");
    query.AddSelect("A");
    query.AddSelect("");
    query.AddSelect("");
    query.AddSelect("B");
    query.AddSelect("");
    query.AddSelect("");
    EXPECT_STREQ("$select=A,B", query.ToQueryString().c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
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

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSQueryTests, ToQueryString_FilterOptionSet_FormatsOption)
    {
    WSQuery query("Foo", "Boo");
    query.SetFilter("TestFilter");
    EXPECT_STREQ("$filter=TestFilter", query.ToQueryString().c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSQueryTests, ToQueryString_OrderByOptionSet_FormatsOption)
    {
    WSQuery query("Foo", "Boo");
    query.SetOrderBy("TestOrderBy");
    EXPECT_STREQ("$orderby=TestOrderBy", query.ToQueryString().c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSQueryTests, ToQueryString_SkipOptionSet_FormatsOption)
    {
    WSQuery query("Foo", "Boo");
    query.SetSkip(42);
    EXPECT_STREQ("$skip=42", query.ToQueryString().c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSQueryTests, ToQueryString_SkipOptionReset_ReturnsEmpty)
    {
    WSQuery query("Foo", "Boo");
    query.SetSkip(42);
    query.SetSkip(0);
    EXPECT_STREQ("", query.ToQueryString().c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSQueryTests, ToQueryString_TopOptionSet_FormatsOption)
    {
    WSQuery query("Foo", "Boo");
    query.SetTop(42);
    EXPECT_STREQ("$top=42", query.ToQueryString().c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSQueryTests, ToQueryString_TopOptionReset_ReturnsEmpty)
    {
    WSQuery query("Foo", "Boo");
    query.SetTop(42);
    query.SetTop(0);
    EXPECT_STREQ("", query.ToQueryString().c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSQueryTests, ToQueryString_CustomParametersSet_AddsParametersToQueryString)
    {
    WSQuery query("Foo", "Boo");
    query.SetSelect("TestSelect");
    query.SetCustomParameter("NameA", "ValueA");
    query.SetCustomParameter("NameB", "ValueB");
    EXPECT_STREQ("$select=TestSelect&NameA=ValueA&NameB=ValueB", query.ToQueryString().c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSQueryTests, ToQueryString_CustomParametersSetWithNoValue_AddsParametersToQueryString)
    {
    WSQuery query("Foo", "Boo");
    query.SetCustomParameter("NameA", "");
    query.SetCustomParameter("NameB", "");
    EXPECT_STREQ("NameA=&NameB=", query.ToQueryString().c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSQueryTests, ToQueryString_MultipleOptionsSet_FormatsAllOfThem)
    {
    WSQuery query("Foo", "Boo");
    query.SetFilter("TestFilter");
    query.SetSkip(4);
    query.SetTop(2);
    EXPECT_STREQ("$filter=TestFilter&$skip=4&$top=2", query.ToQueryString().c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSQueryTests, RemoveCustomParameter_CustomParameterAdded_CustomParameterRemoved)
    {
    WSQuery query("Foo", "Boo");
    query.SetCustomParameter("NameA", "AAA");
    query.RemoveCustomParameter("NameA");
    EXPECT_STREQ("", query.ToQueryString().c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSQueryTests, RemoveCustomParameter_NotExistingCustomParameterName_NothingRemoved)
    {
    WSQuery query("Foo", "Boo");
    query.SetCustomParameter("NameA", "AAA");
    query.RemoveCustomParameter("NotExistingName");
    EXPECT_STREQ("NameA=AAA", query.ToQueryString().c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSQueryTests, GetAlias_CalledMultipleTimesWithDifferentPhrases_ReturnsDifferentAliases)
    {
    WSQuery query("Foo", "Boo");
    EXPECT_STREQ("@A", query.GetAlias("Phrase1").c_str());
    EXPECT_STREQ("@B", query.GetAlias("Phrase2").c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
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

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
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

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSQueryTests, GetAlias_CalledMultipleTimesWithSamePhrase_ReturnsSameAlias)
    {
    WSQuery query("Foo", "Boo");
    EXPECT_STREQ("@A", query.GetAlias("Phrase1").c_str());
    EXPECT_STREQ("@B", query.GetAlias("Phrase2").c_str());
    EXPECT_STREQ("@B", query.GetAlias("Phrase2").c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSQueryTests, GetAliasMap_NoAliasesCreated_Empty)
    {
    WSQuery query("Foo", "Boo");
    EXPECT_TRUE(query.GetAliasMapping().empty());
    }

#ifdef USE_GTEST
/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSQueryTests, GetAliasMap_AliasesCreated_ContainsAliases)
    {
    WSQuery query("Foo", "Boo");
    Utf8String aliasA = query.GetAlias("PhraseA");
    Utf8String aliasB = query.GetAlias("PhraseB");
    EXPECT_THAT(query.GetAliasMapping(), ContainerEq(std::map<Utf8String, Utf8String> { { "PhraseA", aliasA }, {"PhraseB", aliasB}}));
    }
#endif

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSQueryTests, GetPhrase_CalledWithNotGeneratedAliases_ReturnsNull)
    {
    WSQuery query("Foo", "Boo");
    EXPECT_EQ(nullptr, query.GetPhrase(""));
    EXPECT_EQ(nullptr, query.GetPhrase("@A"));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSQueryTests, GetPhrase_CalledWithGeneratedAliases_ReturnsPhrases)
    {
    WSQuery query("Foo", "Boo");
    EXPECT_STREQ("TestPhrase", query.GetPhrase(query.GetAlias("TestPhrase")));
    EXPECT_STREQ("42", query.GetPhrase(query.GetAlias("42")));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSQueryTests, ToQueryString_AliasesGenerated_FormatsAliasesToQuery)
    {
    WSQuery query("Foo", "Boo");
    query.GetAlias("Phrase1");
    query.GetAlias("Phrase2");
    query.SetSkip(42);
    EXPECT_STREQ("$skip=42&@A=Phrase1&@B=Phrase2", query.ToQueryString().c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSQueryTests, EscapeValue_StringPassed_PercentEncodesSpecialSymbols)
    {
    EXPECT_STREQ("", WSQuery::EscapeValue("").c_str());
    EXPECT_STREQ("ABC", WSQuery::EscapeValue("ABC").c_str());
    EXPECT_STREQ("%27%27", WSQuery::EscapeValue("'").c_str());
    EXPECT_STREQ("%3A", WSQuery::EscapeValue(":").c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
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

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
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

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSQueryTests, AddFilterIdsIn_NoIds_EmptyInFilter)
    {
    std::deque<ObjectId> ids;

    WSQuery query("Schema", "Class");
    query.AddFilterIdsIn(ids);
    EXPECT_STREQ("$id+in+[]", query.GetFilter().c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSQueryTests, AddFilterIdsIn_TwoObjects_FilterForSelectedObjects)
    {
    std::deque<ObjectId> ids;
    ids.push_back({"Schema.Class", "A"});
    ids.push_back({"Schema.Class", "B"});

    WSQuery query("Schema", "Class");
    query.AddFilterIdsIn(ids);
    EXPECT_STREQ("$id+in+['A','B']", query.GetFilter().c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSQueryTests, AddFilterIdsIn_ObjectsForDiffrentSchemas_FilterForCorrectSchema)
    {
    std::deque<ObjectId> ids;
    ids.push_back({"Schema" , "Class", "A"});
    ids.push_back({"Schema" , "Class", "B"});
    ids.push_back({"SchemaB", "Class", "C"});
    ids.push_back({"Schema" , "Class", "D"});

    WSQuery query("Schema", "Class");
    query.AddFilterIdsIn(ids);
    EXPECT_STREQ("$id+in+['A','B']", query.GetFilter().c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSQueryTests, AddFilterIdsIn_ObjectsForDiffrentClasses_FilterForCorrectClasses)
    {
    std::deque<ObjectId> ids;
    ids.push_back({"Schema", "ClassA", "A"});
    ids.push_back({"Schema", "ClassB", "B"});
    ids.push_back({"Schema", "ClassA", "C"});
    ids.push_back({"Schema", "ClassC", "D"});
    ids.push_back({"Schema", "ClassA", "E"});

    std::set<Utf8String> classes;
    classes.insert("ClassA");
    classes.insert("ClassB");
    WSQuery query("Schema", classes);
    query.AddFilterIdsIn(ids);
    EXPECT_STREQ("$id+in+['A','B','C']", query.GetFilter().c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSQueryTests, AddFilterIdsIn_AddFilterForMaxTwoObjects_CorrectFilter)
    {
    std::deque<ObjectId> ids;
    ids.push_back({"Schema", "Class", "A"});
    ids.push_back({"Schema", "Class", "B"});
    ids.push_back({"Schema", "Class", "C"});
    ids.push_back({"Schema", "Class", "D"});

    WSQuery query("Schema", "Class");
    query.AddFilterIdsIn(ids, nullptr, 2);
    EXPECT_STREQ("$id+in+['A','B']", query.GetFilter().c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSQueryTests, AddFilterIdsIn_AddFilterWithMaxLength_FilterForObjectsFittingTheLenght)
    {
    std::deque<ObjectId> ids;
    ids.push_back({"Schema", "Class", "A"});
    ids.push_back({"Schema", "Class", "B"});
    ids.push_back({"Schema", "Class", "C"});
    ids.push_back({"Schema", "Class", "D"});

    WSQuery query("Schema", "Class");
    query.AddFilterIdsIn(ids, nullptr, 10, 12);
    EXPECT_STREQ("$id+in+['A','B']", query.GetFilter().c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSQueryTests, AddFilterIdsIn_TwoObjects_InCollectionIsEmpty)
    {
    std::deque<ObjectId> ids;
    ids.push_back({"Schema", "Class", "A"});
    ids.push_back({"Schema", "Class", "B"});

    WSQuery query("Schema", "Class");
    query.AddFilterIdsIn(ids);
    EXPECT_TRUE(ids.empty());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSQueryTests, AddFilterIdsIn_ObjectsForDiffrentSchemas_InCollectionHasRemainingObjects)
    {
    WSQuery query("Schema", "Class");

    std::deque<ObjectId> ids;
    ids.push_back({"Schema", "Class", "A"});
    ids.push_back({"Schema", "Class", "B"});
    ids.push_back({"SchemaB", "Class", "C"});
    ids.push_back({"SchemaC", "Class", "C"});
    ids.push_back({"Schema", "Class", "D"});

    query.AddFilterIdsIn(ids);
    EXPECT_EQ(3, ids.size());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSQueryTests, AddFilterIdsIn_TwoObjects_OutCollectionHasFilteredObjects)
    {   
    std::deque<ObjectId> ids;
    ids.push_back({"Schema", "Class", "A"});
    ids.push_back({"Schema", "Class", "B"});

    WSQuery query("Schema", "Class");

    std::set<ObjectId> idsOut;
    query.AddFilterIdsIn(ids, &idsOut);
    EXPECT_EQ(2, idsOut.size());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSQueryTests, AddFilterIdsIn_ObjectsForDiffrentSchemas_OutCollectionHasFilteredObjects)
    {
    std::deque<ObjectId> ids;
    ids.push_back({"Schema", "Class", "A"});
    ids.push_back({"Schema", "Class", "B"});
    ids.push_back({"SchemaB", "Class", "C"});

    WSQuery query("Schema", "Class");

    std::set<ObjectId> idsOut;
    query.AddFilterIdsIn(ids, &idsOut);
    EXPECT_EQ(2, idsOut.size());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSQueryTests, AddFilterIdsIn_AddFilterForMaxTwoObjects_OutCollectionHasFilteredObjects)
    {
    std::deque<ObjectId> ids;
    ids.push_back({"Schema", "Class", "A"});
    ids.push_back({"Schema", "Class", "B"});
    ids.push_back({"Schema", "Class", "C"});
    ids.push_back({"Schema", "Class", "D"});
    ids.push_back({"Schema", "Class", "E"});

    WSQuery query("Schema", "Class");

    std::set<ObjectId> idsOut;
    query.AddFilterIdsIn(ids, &idsOut, 2);
    EXPECT_EQ(2, idsOut.size());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSQueryTests, AddFilterIdsIn_AddFilterToInitialFilter_TheInitialFilterRemains)
    {
    std::deque<ObjectId> ids;
    
    WSQuery query("Schema", "Class");
    query.SetFilter("Filter");
    query.AddFilterIdsIn(ids);
    EXPECT_EQ(0, ids.size());
    EXPECT_STREQ("(Filter)+and+$id+in+[]", query.GetFilter().c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSQueryTests, AddFilterIdsIn_AddFilterForTwoIDsToInitialFilter_TheInitialFilterRemains)
    {
    std::deque<ObjectId> ids;
    ids.push_back({"Schema", "Class", "A"});
    ids.push_back({"Schema", "Class", "B"});

    WSQuery query("Schema", "Class");
    query.SetFilter("Filter");
    query.AddFilterIdsIn(ids);

    EXPECT_STREQ("(Filter)+and+$id+in+['A','B']", query.GetFilter().c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSQueryTests, AddFilterIdsIn_SetMaxObjectCountTo0_CorrectFilterWithIds)
    {
    std::deque<ObjectId> ids;
    ids.push_back({"Schema", "Class", "A"});
    ids.push_back({"Schema", "Class", "B"});

    WSQuery query("Schema", "Class");
    query.AddFilterIdsIn(ids, nullptr, 0);

    EXPECT_STREQ("$id+in+['A','B']", query.GetFilter().c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSQueryTests, AddFilterIdsIn_SetMaxLenghtTo0_CorrectFilterWithIds)
    {
    std::deque<ObjectId> ids;
    ids.push_back({"Schema", "Class", "A"});
    ids.push_back({"Schema", "Class", "B"});

    WSQuery query("Schema", "Class");
    query.AddFilterIdsIn(ids, nullptr, 100, 0);

    EXPECT_STREQ("$id+in+['A','B']", query.GetFilter().c_str());
    }
