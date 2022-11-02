/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "PresentationRulesTests.h"
#include <ECPresentation/Rules/PresentationRules.h>

USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
struct LocalizationResourceKeyDefinitionTests : PresentationRulesTests
    {
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(LocalizationResourceKeyDefinitionTests, LoadsFromXml)
    {
    static Utf8CP xmlString = R"(
        <LocalizationResourceKeyDefinition Id="id" Key="key" DefaultValue="defaultValue"/>
        )";
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlString);
    ASSERT_EQ(BEXML_Success, xmlStatus);

    LocalizationResourceKeyDefinition rule;
    EXPECT_TRUE(rule.ReadXml(xml->GetRootElement()));
    EXPECT_STREQ("id", rule.GetId().c_str());
    EXPECT_STREQ("key", rule.GetKey().c_str());
    EXPECT_STREQ("defaultValue", rule.GetDefaultValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(LocalizationResourceKeyDefinitionTests, LoadsFromXmlWithDefaultValues)
    {
    static Utf8CP xmlString = "<LocalizationResourceKeyDefinition/>";
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlString);
    ASSERT_EQ(BEXML_Success, xmlStatus);

    LocalizationResourceKeyDefinition rule;
    EXPECT_TRUE(rule.ReadXml(xml->GetRootElement()));
    EXPECT_STREQ("", rule.GetId().c_str());
    EXPECT_STREQ("", rule.GetKey().c_str());
    EXPECT_STREQ("", rule.GetDefaultValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(LocalizationResourceKeyDefinitionTests, WriteToXml)
    {
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateEmpty();
    xml->AddNewElement("Root", nullptr, nullptr);

    LocalizationResourceKeyDefinition def(100, "id", "key", "defaultValue");
    def.WriteXml(xml->GetRootElement());

    static Utf8CP expected = ""
        "<Root>"
            R"(<LocalizationResourceKeyDefinition Priority="100" Id="id" Key="key" DefaultValue="defaultValue"/>)"
        "</Root>";
    EXPECT_STREQ(ToPrettyString(*BeXmlDom::CreateAndReadFromString(xmlStatus, expected)).c_str(), ToPrettyString(*xml).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(LocalizationResourceKeyDefinitionTests, ComputesCorrectHashes)
    {
    Utf8CP DEFAULT_HASH = "99b2b2a05c9eeba364ab75c1c3078d94";

    // Make sure that introducing additional attributes with default values don't affect the hash
    LocalizationResourceKeyDefinition defaultSpec;
    EXPECT_STREQ(DEFAULT_HASH, defaultSpec.GetHash().c_str());

    // Make sure that copy has the same hash
    LocalizationResourceKeyDefinition copySpec(defaultSpec);
    EXPECT_STREQ(DEFAULT_HASH, copySpec.GetHash().c_str());

    // TODO: test that each attribute affects the hash
    }
