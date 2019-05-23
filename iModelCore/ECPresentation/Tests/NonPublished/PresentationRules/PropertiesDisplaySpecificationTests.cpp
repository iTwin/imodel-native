/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
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
* @betest                                   Aidas.Kilinskas                		04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PropertiesDisplaySpecificationsTests, LoadsFromJson)
    {
    static Utf8CP jsonString = R"({
        "propertyNames": ["p1", "p2"],
        "priority":1,
        "isDisplayed":false
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    EXPECT_FALSE(json.isNull());

    PropertiesDisplaySpecification spec;
    EXPECT_TRUE(spec.ReadJson(json));
    EXPECT_STREQ("p1,p2", spec.GetPropertyNames().c_str());
    EXPECT_FALSE(spec.IsDisplayed());
    EXPECT_EQ(1,spec.GetPriority());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                   Aidas.Kilinskas                		04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PropertiesDisplaySpecificationsTests, LoadsFromJsonWithDefaultValues)
    {
    static Utf8CP jsonString = R"({
        "propertyNames": ["p1","p2"]
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    ASSERT_FALSE(json.isNull());

    PropertiesDisplaySpecification spec;
    ASSERT_TRUE(spec.ReadJson(json));
    EXPECT_STREQ("p1,p2", spec.GetPropertyNames().c_str());
    EXPECT_TRUE(spec.IsDisplayed());
    EXPECT_EQ(1000, spec.GetPriority());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                   Aidas.Kilinskas                		04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PropertiesDisplaySpecificationsTests, LoadFromJsonFailsIfPropertyNamesNotSpecified)
    {
    static Utf8CP jsonString = "{}";
    Json::Value json = Json::Reader::DoParse(jsonString);
    EXPECT_FALSE(json.isNull());

    PropertiesDisplaySpecification spec;
    EXPECT_FALSE(spec.ReadJson(json));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PropertiesDisplaySpecificationsTests, WriteToJson)
    {
    PropertiesDisplaySpecification spec("p1, p2", 123, true);
    Json::Value json = spec.WriteJson();
    Json::Value expected = Json::Reader::DoParse(R"({
        "priority": 123,
        "propertyNames": ["p1", "p2"]
    })");
    EXPECT_STREQ(ToPrettyString(expected).c_str(), ToPrettyString(json).c_str());
    }

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
* @bsimethod                                    Aidas.Vaiksnoras                12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PropertiesDisplaySpecificationsTests, HiddenPropertiesSpecification_LoadFromXmlFailsWhenPropertyNamesAttributeIsNotSpecified)
    {
    static Utf8CP xmlString = "<HiddenProperties/>";
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlString);
    ASSERT_EQ(BEXML_Success, xmlStatus);

    PropertiesDisplaySpecification spec;
    EXPECT_FALSE(spec.ReadXml(xml->GetRootElement()));
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
