/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "PresentationRulesTests.h"
#include <ECPresentation/Rules/PresentationRules.h>

USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct MultiSchemaClassTests : PresentationRulesTests
    {};

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MultiSchemaClassTests, LoadsFromJsonObject)
    {
    Utf8CP const jsonString = R"(
        {
            "schemaName": "testSchemaName",
            "classNames": ["testClassName1", "testClassName2"],
            "arePolymorphic": true
        }
    )";
    Json::Value json = Json::Reader::DoParse(jsonString);
    ASSERT_FALSE(json.isNull());

    MultiSchemaClass multiSchemaClass;

    ASSERT_TRUE(multiSchemaClass.ReadJson(json));

    EXPECT_STREQ("testSchemaName", multiSchemaClass.GetSchemaName().c_str());

    ASSERT_EQ(2, multiSchemaClass.GetClassNames().size());
    EXPECT_STREQ("testClassName1", multiSchemaClass.GetClassNames()[0].c_str());
    EXPECT_STREQ("testClassName2", multiSchemaClass.GetClassNames()[1].c_str());

    ASSERT_EQ(true, multiSchemaClass.GetArePolymorphic());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/

TEST_F(MultiSchemaClassTests, ComputesCorrectHashes)
    {
    Utf8CP const DEFAULT_HASH = "4a0639da6c1fd34025658658ed0f21a5";

    // Make sure that introducing additional attributes with default values don't affect the hash
    MultiSchemaClass defaultMultiSchemaClass;
    EXPECT_STREQ(DEFAULT_HASH, defaultMultiSchemaClass.GetHash().c_str());

    // Make sure that copy has the same hash
    MultiSchemaClass multiSchemaClassCopy(defaultMultiSchemaClass);
    EXPECT_STREQ(DEFAULT_HASH, multiSchemaClassCopy.GetHash().c_str());

    // Test that each attribute affects the hash
    MultiSchemaClass multiSchemaClassNewSchemaName(defaultMultiSchemaClass);
    multiSchemaClassNewSchemaName.SetSchemaName("newSchemaName");
    EXPECT_STRNE(defaultMultiSchemaClass.GetHash().c_str(), multiSchemaClassNewSchemaName.GetHash().c_str());
    multiSchemaClassNewSchemaName.SetSchemaName("");
    EXPECT_STREQ(defaultMultiSchemaClass.GetHash().c_str(), multiSchemaClassNewSchemaName.GetHash().c_str());

    MultiSchemaClass multiSchemaClassNewClassNames(defaultMultiSchemaClass);
    multiSchemaClassNewClassNames.SetClassNames(bvector<Utf8String>{ "newClassName1" });
    EXPECT_STRNE(defaultMultiSchemaClass.GetHash().c_str(), multiSchemaClassNewClassNames.GetHash().c_str());
    multiSchemaClassNewClassNames.SetClassNames(bvector<Utf8String>());
    EXPECT_STREQ(defaultMultiSchemaClass.GetHash().c_str(), multiSchemaClassNewClassNames.GetHash().c_str());

    MultiSchemaClass multiSchemaClassNewArePolymorphic(defaultMultiSchemaClass);
    multiSchemaClassNewArePolymorphic.SetArePolymorphic(true);
    EXPECT_STRNE(defaultMultiSchemaClass.GetHash().c_str(), multiSchemaClassNewArePolymorphic.GetHash().c_str());
    multiSchemaClassNewArePolymorphic.SetArePolymorphic(false);
    EXPECT_STREQ(defaultMultiSchemaClass.GetHash().c_str(), multiSchemaClassNewArePolymorphic.GetHash().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MultiSchemaClassTests, WritesToJson)
    {
    MultiSchemaClass multiSchemaClass;
    multiSchemaClass.SetSchemaName("someSchemaName");
    multiSchemaClass.SetClassNames(bvector<Utf8String>{ "className1", "className2" });
    multiSchemaClass.SetArePolymorphic(true);

    Json::Value generatedJson = multiSchemaClass.WriteJson();
    Json::Value expectedJson = Json::Reader::DoParse(R"(
        {
            "schemaName": "someSchemaName",
            "classNames": ["className1", "className2"],
            "arePolymorphic": true
        }
    )");

    EXPECT_STREQ(ToPrettyString(expectedJson).c_str(), ToPrettyString(generatedJson).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MultiSchemaClassTests, LoadsMultiSchemaClassWithDefaultPolymorphism)
    {
    Utf8CP const jsonString = R"(
        {
            "schemaName": "testSchemaName",
            "classNames": ["testClassName1", "testClassName2"]
        }
    )";
    Json::Value json = Json::Reader::DoParse(jsonString);
    ASSERT_FALSE(json.isNull());

    auto const multiSchemaClassTrue = std::unique_ptr<MultiSchemaClass>(MultiSchemaClass::LoadFromJson(json, true));
    ASSERT_TRUE(multiSchemaClassTrue != nullptr);
    ASSERT_TRUE(multiSchemaClassTrue->GetArePolymorphic());

    auto const multiSchemaClassFalse = std::unique_ptr<MultiSchemaClass>(MultiSchemaClass::LoadFromJson(json, false));
    ASSERT_TRUE(multiSchemaClassFalse != nullptr);
    ASSERT_FALSE(multiSchemaClassFalse->GetArePolymorphic());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MultiSchemaClassTests, LoadsMultiSchemaClassWithSpecifiedPolymorphism)
    {
    Utf8CP const jsonStringArePolymorphicTrue = R"(
        {
            "schemaName": "testSchemaName",
            "classNames": ["testClassName1", "testClassName2"],
            "arePolymorphic": true
        }
    )";
    Json::Value jsonArePolymorphicTrue = Json::Reader::DoParse(jsonStringArePolymorphicTrue);
    ASSERT_FALSE(jsonArePolymorphicTrue.isNull());

    auto const multiSchemaClassTrue = std::unique_ptr<MultiSchemaClass>(MultiSchemaClass::LoadFromJson(jsonArePolymorphicTrue, false));
    ASSERT_TRUE(multiSchemaClassTrue != nullptr);
    ASSERT_TRUE(multiSchemaClassTrue->GetArePolymorphic());

    Utf8CP const jsonStringArePolymorphicFalse = R"(
        {
            "schemaName": "testSchemaName",
            "classNames": ["testClassName1", "testClassName2"],
            "arePolymorphic": false
        }
    )";
    Json::Value jsonArePolymorphicFalse = Json::Reader::DoParse(jsonStringArePolymorphicFalse);
    ASSERT_FALSE(jsonArePolymorphicFalse.isNull());

    auto const multiSchemaClassFalse = std::unique_ptr<MultiSchemaClass>(MultiSchemaClass::LoadFromJson(jsonArePolymorphicFalse, true));
    ASSERT_TRUE(multiSchemaClassFalse != nullptr);
    ASSERT_FALSE(multiSchemaClassFalse->GetArePolymorphic());
    }
