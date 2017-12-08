/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/PresentationRules/AllRelatedInstanceNodesSpecificationTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PresentationRulesTests.h"
#include <ECPresentation/RulesDriven/Rules/PresentationRules.h>

USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras               12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
struct AllRelatedInstanceNodesSpecificationTests : PresentationRulesTests
    {
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras               12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(AllRelatedInstanceNodesSpecificationTests, LoadsFromXml)
    {
    static Utf8CP xmlString = R"(
        <AllRelatedInstances GroupByClass="false" GroupByRelationship="true" GroupByLabel="false" 
         SkipRelatedLevel="3" SupportedSchemas="TestSchema" RequiredDirection="Forward"/>
        )";
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlString);
    ASSERT_EQ(BEXML_Success, xmlStatus);
    
    AllRelatedInstanceNodesSpecification spec;
    EXPECT_TRUE(spec.ReadXml(xml->GetRootElement()));
    EXPECT_FALSE(spec.GetGroupByClass());
    EXPECT_TRUE(spec.GetGroupByRelationship());
    EXPECT_FALSE(spec.GetGroupByLabel());
    EXPECT_EQ(3, spec.GetSkipRelatedLevel());
    EXPECT_STREQ("TestSchema", spec.GetSupportedSchemas().c_str());
    EXPECT_EQ(RequiredRelationDirection::RequiredRelationDirection_Forward, spec.GetRequiredRelationDirection());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras               12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(AllRelatedInstanceNodesSpecificationTests, LoadsFromXmlWithDefaultValues)
    {
    static Utf8CP xmlString = "<AllRelatedInstances/>";
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlString);
    ASSERT_EQ(BEXML_Success, xmlStatus);
    
    AllRelatedInstanceNodesSpecification spec;
    EXPECT_TRUE(spec.ReadXml(xml->GetRootElement()));
    EXPECT_TRUE(spec.GetGroupByClass());
    EXPECT_FALSE(spec.GetGroupByRelationship());
    EXPECT_TRUE(spec.GetGroupByLabel());
    EXPECT_EQ(0, spec.GetSkipRelatedLevel());
    EXPECT_STREQ("", spec.GetSupportedSchemas().c_str());
    EXPECT_EQ(RequiredRelationDirection::RequiredRelationDirection_Both, spec.GetRequiredRelationDirection());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras               12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(AllRelatedInstanceNodesSpecificationTests, WriteToXml)
    {
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateEmpty();
    xml->AddNewElement("Root", nullptr, nullptr);

    AllRelatedInstanceNodesSpecification spec;
    spec.SetGroupByClass(false);
    spec.SetGroupByLabel(true);
    spec.SetGroupByRelationship(false);
    spec.SetSkipRelatedLevel(3);
    spec.SetSupportedSchemas("TestSchema");
    spec.SetRequiredRelationDirection(RequiredRelationDirection::RequiredRelationDirection_Forward);
    spec.WriteXml(xml->GetRootElement());

    static Utf8CP expected = ""
        "<Root>"
            R"(<AllRelatedInstances Priority="1000" AlwaysReturnsChildren="false" HideNodesInHierarchy="false" 
                HideIfNoChildren="false" ExtendedData="" DoNotSort="false" GroupByClass="false" 
                GroupByRelationship="false" GroupByLabel="true" SkipRelatedLevel="3" 
                SupportedSchemas="TestSchema" RequiredDirection="Forward"/>)"
        "</Root>";
    EXPECT_STREQ(ToPrettyString(*BeXmlDom::CreateAndReadFromString(xmlStatus, expected)).c_str(), ToPrettyString(*xml).c_str());
    }