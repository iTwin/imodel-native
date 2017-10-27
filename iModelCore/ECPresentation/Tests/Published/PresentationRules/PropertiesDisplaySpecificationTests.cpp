/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/PresentationRules/PropertiesDisplaySpecificationTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PresentationRulesTests.h"
#include <ECPresentation/RulesDriven/Rules/PresentationRules.h>

USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
struct PropertiesDisplaySpecificationsTests : PresentationRulesTests
    {
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PropertiesDisplaySpecificationsTests, LoadsDisplayedPropertiesSpecificationFromXml)
    {
    static Utf8CP xmlString = R"(
        <DisplayedProperties PropertyNames="Properties" />
        )";
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlString);
    ASSERT_EQ(BEXML_Success, xmlStatus);
    
    PropertiesDisplaySpecification spec;
    EXPECT_TRUE(spec.ReadXml(xml->GetRootElement()));
    EXPECT_STREQ("Properties", spec.GetPropertyNames().c_str());
    EXPECT_TRUE(spec.IsDisplayed());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PropertiesDisplaySpecificationsTests, LoadsHiddenPropertiesSpecificationFromXml)
    {
    static Utf8CP xmlString = R"(
        <HiddenProperties PropertyNames="Properties" />
        )";
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlString);
    ASSERT_EQ(BEXML_Success, xmlStatus);
    
    PropertiesDisplaySpecification spec;
    EXPECT_TRUE(spec.ReadXml(xml->GetRootElement()));
    EXPECT_STREQ("Properties", spec.GetPropertyNames().c_str());
    EXPECT_FALSE(spec.IsDisplayed());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PropertiesDisplaySpecificationsTests, WritesDisplayedPropertiesSpecificationToXml)
    {
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateEmpty();
    xml->AddNewElement("Root", nullptr, nullptr);

    PropertiesDisplaySpecification spec("PropertyNames", 123, true);
    spec.WriteXml(xml->GetRootElement());
    
    static Utf8CP expected = ""
        "<Root>"
            R"(<DisplayedProperties PropertyNames="PropertyNames" Priority="123" />)"
        "</Root>";
    EXPECT_STREQ(ToPrettyString(*BeXmlDom::CreateAndReadFromString(xmlStatus, expected)).c_str(), ToPrettyString(*xml).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PropertiesDisplaySpecificationsTests, WritesHiddenPropertiesSpecificationToXml)
    {
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateEmpty();
    xml->AddNewElement("Root", nullptr, nullptr);

    PropertiesDisplaySpecification spec("PropertyNames", 123, false);
    spec.WriteXml(xml->GetRootElement());
    
    static Utf8CP expected = ""
        "<Root>"
            R"(<HiddenProperties PropertyNames="PropertyNames" Priority="123" />)"
        "</Root>";
    EXPECT_STREQ(ToPrettyString(*BeXmlDom::CreateAndReadFromString(xmlStatus, expected)).c_str(), ToPrettyString(*xml).c_str());
    }