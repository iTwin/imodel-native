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
struct CustomRendererSpecificationTests : PresentationRulesTests
    {};

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CustomRendererSpecificationTests, ReadJson_ValidJson_Success)
    {
    static Utf8CP jsonString = R"({ "rendererName": "test_renderer" })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    ASSERT_FALSE(json.isNull());

    CustomRendererSpecification spec;
    EXPECT_TRUE(spec.ReadJson(json));
    EXPECT_STREQ("test_renderer", spec.GetRendererName().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CustomRendererSpecificationTests, ReadJson_EmptyRendererName_Failure)
    {
    static Utf8CP jsonString = R"({ "rendererName": "" })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    ASSERT_FALSE(json.isNull());

    CustomRendererSpecification spec;
    EXPECT_FALSE(spec.ReadJson(json));
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CustomRendererSpecificationTests, WriteJson_InitialisedSpec_ExpectedJson)
    {
    CustomRendererSpecification spec("test_renderer");
    Json::Value json = spec.WriteJson();
    Json::Value expected = Json::Reader::DoParse(R"({ "rendererName": "test_renderer" })");
    EXPECT_STREQ(ToPrettyString(expected).c_str(), ToPrettyString(json).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CustomRendererSpecificationTests, ComputesCorrectHashes)
    {
    Utf8CP DEFAULT_HASH = "86cbf900d3bb599ba2f46c4ad0460657";

    // Introducing additional attributes with default values don't affect the hash
    CustomRendererSpecification defaultSpec;
    EXPECT_STREQ(DEFAULT_HASH, defaultSpec.GetHash().c_str());

    // Each attribute affects the hash
    CustomRendererSpecification specWithRendererName;
    specWithRendererName.SetRendererName("test_renderer");
    EXPECT_STRNE(DEFAULT_HASH, specWithRendererName.GetHash().c_str());
    specWithRendererName.SetRendererName("");
    EXPECT_STREQ(DEFAULT_HASH, specWithRendererName.GetHash().c_str());
    }
