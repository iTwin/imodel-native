/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "PresentationRulesTests.h"

USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct RequiredSchemaSpecificationTests : PresentationRulesTests
    {
    };

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RequiredSchemaSpecificationTests, CopyConstructorCopiesProperties)
    {
    // create source
    RequiredSchemaSpecification source("TestSchema", Version(1, 2, 3), Version(4, 5, 6));
    EXPECT_STREQ("TestSchema", source.GetName().c_str());
    EXPECT_TRUE(Version(1, 2, 3) == source.GetMinVersion().Value());
    EXPECT_TRUE(Version(4, 5, 6) == source.GetMaxVersion().Value());

    // create another rule via copy constructor
    RequiredSchemaSpecification target(source);
    EXPECT_STREQ("TestSchema", target.GetName().c_str());
    EXPECT_TRUE(Version(1, 2, 3) == target.GetMinVersion().Value());
    EXPECT_TRUE(Version(4, 5, 6) == target.GetMaxVersion().Value());
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RequiredSchemaSpecificationTests, LoadsFromJson)
    {
    static Utf8CP jsonString = R"({
        "name": "TestSchema",
        "minVersion": "01.02.03",
        "maxVersion": "04.05.06"
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    EXPECT_FALSE(json.isNull());

    RequiredSchemaSpecification rule;
    EXPECT_TRUE(rule.ReadJson(json));

    EXPECT_STREQ("TestSchema", rule.GetName().c_str());
    EXPECT_TRUE(Version(1, 2, 3) == rule.GetMinVersion().Value());
    EXPECT_TRUE(Version(4, 5, 6) == rule.GetMaxVersion().Value());
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RequiredSchemaSpecificationTests, LoadsFromJsonWithDefaultValues)
    {
    static Utf8CP jsonString = R"({
        "name": "TestSchema"
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    EXPECT_FALSE(json.isNull());

    RequiredSchemaSpecification rule;
    EXPECT_TRUE(rule.ReadJson(json));

    EXPECT_STREQ("TestSchema", rule.GetName().c_str());
    EXPECT_TRUE(rule.GetMinVersion().IsNull());
    EXPECT_TRUE(rule.GetMaxVersion().IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RequiredSchemaSpecificationTests, WriteToJson)
    {
    RequiredSchemaSpecification rule("TestSchema", Version(1, 2, 3), Version(4, 5, 6));
    Json::Value json = rule.WriteJson();
    Json::Value expected = Json::Reader::DoParse(R"({
        "name": "TestSchema",
        "minVersion": "1.2.3",
        "maxVersion": "4.5.6"
    })");
    EXPECT_STREQ(ToPrettyString(expected).c_str(), ToPrettyString(json).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RequiredSchemaSpecificationTests, ComputesCorrectHashes)
    {
    Utf8CP DEFAULT_HASH = "7cea5b35c5f7cc8b17fcb661c64565c6";

    // Make sure that introducing additional attributes with default values don't affect the hash
    RequiredSchemaSpecification defaultSpec("");
    EXPECT_STREQ(DEFAULT_HASH, defaultSpec.GetHash().c_str());

    // Make sure that copy has the same hash
    RequiredSchemaSpecification copySpec(defaultSpec);
    EXPECT_STREQ(DEFAULT_HASH, copySpec.GetHash().c_str());

    // Test that each attribute affects the hash
    RequiredSchemaSpecification specWithMinVersion(defaultSpec);
    specWithMinVersion.SetMinVersion(Version(7,8,9));
    EXPECT_STRNE(DEFAULT_HASH, specWithMinVersion.GetHash().c_str());
    specWithMinVersion.SetMinVersion(nullptr);
    EXPECT_STREQ(DEFAULT_HASH, specWithMinVersion.GetHash().c_str());

    RequiredSchemaSpecification specWithMaxVersion(defaultSpec);
    specWithMaxVersion.SetMaxVersion(Version(7, 8, 9));
    EXPECT_STRNE(DEFAULT_HASH, specWithMaxVersion.GetHash().c_str());
    specWithMaxVersion.SetMaxVersion(nullptr);
    EXPECT_STREQ(DEFAULT_HASH, specWithMaxVersion.GetHash().c_str());
    }
