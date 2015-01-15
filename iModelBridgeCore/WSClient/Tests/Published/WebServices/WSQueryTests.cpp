/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/WebServices/WSQueryTests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "WSQueryTests.h"
#include <WebServices/WSQuery.h>
#include "WebServicesTestsHelper.h"

using namespace ::testing;
using namespace ::std;
USING_NAMESPACE_BENTLEY_WEBSERVICES

TEST_F (WSQueryTests, Ctor_SchemaAndClassesPassed_SetsSchemaAndClasses)
    {
    WSQuery query ("TestSchema", set<Utf8String> {"A", "B"});
    EXPECT_STREQ (query.GetSchemaName ().c_str (), "TestSchema");
#ifdef USE_GTEST
    EXPECT_THAT (query.GetClasses (), ContainerEq (set<Utf8String>{"A", "B"}));
#endif
    }

TEST_F (WSQueryTests, Ctor_SchemaAndClass_SetsSchemaAndClass)
    {
    WSQuery query ("TestSchema", "TestClass");
    EXPECT_STREQ (query.GetSchemaName ().c_str (), "TestSchema");
#ifdef USE_GTEST
    EXPECT_THAT (query.GetClasses (), ContainerEq (set<Utf8String>{"TestClass"}));
#endif
    }

TEST_F (WSQueryTests, Ctor_ObjectIdPassed_SetsSchemaAndClassAndFilter)
    {
    WSQuery query (ObjectId ("TestSchema", "TestClass", "TestId"));
    EXPECT_STREQ (query.GetSchemaName ().c_str (), "TestSchema");
#ifdef USE_GTEST
    EXPECT_THAT (query.GetClasses (), ContainerEq (set<Utf8String>{"TestClass"}));
#endif
    EXPECT_STREQ (query.GetFilter ().c_str (), "$id+in+['TestId']");
    }

TEST_F (WSQueryTests, Ctor_ObjectIdPassed_SetsSchemaAndClassAndEscapesRemoteId)
    {
    WSQuery query (ObjectId ("Foo", "Boo", "'"));
    EXPECT_STREQ (query.GetFilter ().c_str (), "$id+in+['%27%27']");
    }

TEST_F (WSQueryTests, ToString_NoOptionsSet_ReturnsEmpty)
    {
    WSQuery query ("Foo", "Boo");
    EXPECT_STREQ ("", query.ToString ().c_str ());
    }

TEST_F (WSQueryTests, ToString_SelectOptionSet_FormatsOption)
    {
    WSQuery query ("Foo", "Boo");
    query.SetSelect ("TestSelect");
    EXPECT_STREQ ("$select=TestSelect", query.ToString ().c_str ());
    }

TEST_F (WSQueryTests, ToString_FilterOptionSet_FormatsOption)
    {
    WSQuery query ("Foo", "Boo");
    query.SetFilter ("TestFilter");
    EXPECT_STREQ ("$filter=TestFilter", query.ToString ().c_str ());
    }

TEST_F (WSQueryTests, ToString_OrderByOptionSet_FormatsOption)
    {
    WSQuery query ("Foo", "Boo");
    query.SetOrderBy ("TestOrderBy");
    EXPECT_STREQ ("$orderby=TestOrderBy", query.ToString ().c_str ());
    }

TEST_F (WSQueryTests, ToString_SkipOptionSet_FormatsOption)
    {
    WSQuery query ("Foo", "Boo");
    query.SetSkip (42);
    EXPECT_STREQ ("$skip=42", query.ToString ().c_str ());
    }

TEST_F (WSQueryTests, ToString_SkipOptionReset_ReturnsEmpty)
    {
    WSQuery query ("Foo", "Boo");
    query.SetSkip (42);
    query.SetSkip (0);
    EXPECT_STREQ ("", query.ToString ().c_str ());
    }

TEST_F (WSQueryTests, ToString_TopOptionSet_FormatsOption)
    {
    WSQuery query ("Foo", "Boo");
    query.SetTop (42);
    EXPECT_STREQ ("$top=42", query.ToString ().c_str ());
    }

TEST_F (WSQueryTests, ToString_TopOptionReset_ReturnsEmpty)
    {
    WSQuery query ("Foo", "Boo");
    query.SetTop (42);
    query.SetTop (0);
    EXPECT_STREQ ("", query.ToString ().c_str ());
    }

TEST_F (WSQueryTests, ToString_CustomParametersSet_AddsParametersToQueryString)
    {
    WSQuery query ("Foo", "Boo");
    query.SetSelect ("TestSelect");
    query.SetCustomParameter ("NameA", "ValueA");
    query.SetCustomParameter ("NameB", "ValueB");
    EXPECT_STREQ ("$select=TestSelect&NameA=ValueA&NameB=ValueB", query.ToString ().c_str ());
    }

TEST_F (WSQueryTests, ToString_MultipleOptionsSet_FormatsAllOfThem)
    {
    WSQuery query ("Foo", "Boo");
    query.SetFilter ("TestFilter");
    query.SetSkip (4);
    query.SetTop (2);
    EXPECT_STREQ ("$filter=TestFilter&$skip=4&$top=2", query.ToString ().c_str ());
    }

TEST_F (WSQueryTests, GetAlias_CalledMultipleTimesWithDifferentPhrases_ReturnsDifferentAliases)
    {
    WSQuery query ("Foo", "Boo");
    EXPECT_STREQ ("@A", query.GetAlias ("Phrase1").c_str ());
    EXPECT_STREQ ("@B", query.GetAlias ("Phrase2").c_str ());
    }

TEST_F (WSQueryTests, GetAlias_CalledMoreThanAlphabetLengthTimes_ReturnsAliasesPostfixedWith1)
    {
    WSQuery query ("Foo", "Boo");

    for (int i = 0; i < 25; i++)
        {
        query.GetAlias (Utf8PrintfString ("%d", i).c_str ());
        }
    EXPECT_STREQ ("@Z", query.GetAlias ("Phrase0").c_str ());

    EXPECT_STREQ ("@A1", query.GetAlias ("Phrase1").c_str ());
    EXPECT_STREQ ("@B1", query.GetAlias ("Phrase2").c_str ());
    }

TEST_F (WSQueryTests, GetAlias_CalledMoreThanTwiceAlphabetLengthTimes_ReturnsAliasesPostfixedWith2)
    {
    WSQuery query ("Foo", "Boo");

    for (int i = 0; i < 51; i++)
        {
        query.GetAlias (Utf8PrintfString ("%d", i).c_str ());
        }
    EXPECT_STREQ ("@Z1", query.GetAlias ("Phrase0").c_str ());

    EXPECT_STREQ ("@A2", query.GetAlias ("Phrase1").c_str ());
    EXPECT_STREQ ("@B2", query.GetAlias ("Phrase2").c_str ());
    }

TEST_F (WSQueryTests, GetAlias_CalledMultipleTimesWithSamePhrase_ReturnsSameAlias)
    {
    WSQuery query ("Foo", "Boo");
    EXPECT_STREQ ("@A", query.GetAlias ("Phrase1").c_str ());
    EXPECT_STREQ ("@B", query.GetAlias ("Phrase2").c_str ());
    EXPECT_STREQ ("@B", query.GetAlias ("Phrase2").c_str ());
    }

TEST_F (WSQueryTests, GetAliasMap_NoAliasesCreated_Empty)
    {
    WSQuery query ("Foo", "Boo");
    EXPECT_TRUE (query.GetAliasMapping ().empty ());
    }

#ifdef USE_GTEST
TEST_F (WSQueryTests, GetAliasMap_AliasesCreated_ContainsAliases)
    {
    WSQuery query ("Foo", "Boo");
    Utf8String aliasA = query.GetAlias ("PhraseA");
    Utf8String aliasB = query.GetAlias ("PhraseB");
    EXPECT_THAT (query.GetAliasMapping (), ContainerEq (std::map<Utf8String, Utf8String> { {"PhraseA", aliasA}, {"PhraseB", aliasB}}));
    }
#endif

TEST_F (WSQueryTests, ToString_AliasesGenerated_FormatsAliasesToQuery)
    {
    WSQuery query ("Foo", "Boo");
    query.GetAlias ("Phrase1");
    query.GetAlias ("Phrase2");
    query.SetSkip (42);
    EXPECT_STREQ ("$skip=42&@A=Phrase1&@B=Phrase2", query.ToString ().c_str ());
    }

TEST_F (WSQueryTests, EscapeValue_StringPassed_PercentEncodesSpecialSymbols)
    {
    EXPECT_STREQ ("", WSQuery::EscapeValue ("").c_str ());
    EXPECT_STREQ ("ABC", WSQuery::EscapeValue ("ABC").c_str ());
    EXPECT_STREQ ("%27%27", WSQuery::EscapeValue ("'").c_str ());
    EXPECT_STREQ ("%3A", WSQuery::EscapeValue (":").c_str ());
    }
