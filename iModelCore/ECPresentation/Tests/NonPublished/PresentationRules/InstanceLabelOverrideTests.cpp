/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "PresentationRulesTests.h"
#include <ECPresentation/RulesDriven/Rules/PresentationRules.h>

USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras               12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
struct InstanceLabelOverrideTests : PresentationRulesTests
    {
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(InstanceLabelOverrideTests, LoadsFromJson)
    {
    static Utf8CP jsonString = R"({
        "ruleType": "InstanceLabelOverride",
        "class": {"schemaName": "TestSchema", "className": "TestClass"},
        "values": [{
            "specType": "Composite",
            "separator": "-",
            "parts": [{
                "spec": {
                    "specType": "Property",
                    "propertyName": "prop1"
                }
            }, {
                "spec": {
                    "specType": "BriefcaseId"
                },
                "isRequired": true
            }]
        }, {
            "specType": "Property",
            "propertyName": "prop2"
        }, {
            "specType": "ClassName",
            "full": true
        }, {
            "specType": "ClassLabel"
        }, {
            "specType": "BriefcaseId"
        }, {
            "specType": "LocalId"
        }, {
            "specType": "String",
            "value": "test"
        }]
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    ASSERT_FALSE(json.isNull());

    InstanceLabelOverride rule;

    ASSERT_TRUE(rule.ReadJson(json));
    EXPECT_STREQ("TestSchema:TestClass", rule.GetClassName().c_str());
    ASSERT_EQ(7, rule.GetValueSpeficications().size());

    InstanceLabelOverrideCompositeValueSpecification* s1 = dynamic_cast<InstanceLabelOverrideCompositeValueSpecification*>(rule.GetValueSpeficications()[0]);
    ASSERT_TRUE(nullptr != s1);
    EXPECT_STREQ("-", s1->GetSeparator().c_str());
    ASSERT_EQ(2, s1->GetValueParts().size());
    EXPECT_TRUE(nullptr != dynamic_cast<InstanceLabelOverridePropertyValueSpecification*>(s1->GetValueParts()[0]->GetSpecification()));
    EXPECT_FALSE(s1->GetValueParts()[0]->IsRequired());
    EXPECT_TRUE(nullptr != dynamic_cast<InstanceLabelOverrideBriefcaseIdValueSpecification*>(s1->GetValueParts()[1]->GetSpecification()));
    EXPECT_TRUE(s1->GetValueParts()[1]->IsRequired());

    InstanceLabelOverridePropertyValueSpecification* s2 = dynamic_cast<InstanceLabelOverridePropertyValueSpecification*>(rule.GetValueSpeficications()[1]);
    ASSERT_TRUE(nullptr != s2);
    EXPECT_STREQ("prop2", s2->GetPropertyName().c_str());

    InstanceLabelOverrideClassNameValueSpecification* s3 = dynamic_cast<InstanceLabelOverrideClassNameValueSpecification*>(rule.GetValueSpeficications()[2]);
    ASSERT_TRUE(nullptr != s3);
    EXPECT_TRUE(s3->ShouldUseFullName());

    ASSERT_TRUE(nullptr != dynamic_cast<InstanceLabelOverrideClassLabelValueSpecification*>(rule.GetValueSpeficications()[3]));

    ASSERT_TRUE(nullptr != dynamic_cast<InstanceLabelOverrideBriefcaseIdValueSpecification*>(rule.GetValueSpeficications()[4]));

    ASSERT_TRUE(nullptr != dynamic_cast<InstanceLabelOverrideLocalIdValueSpecification*>(rule.GetValueSpeficications()[5]));

    InstanceLabelOverrideStringValueSpecification* s7 = dynamic_cast<InstanceLabelOverrideStringValueSpecification*>(rule.GetValueSpeficications()[6]);
    ASSERT_TRUE(nullptr != s7);
    EXPECT_STREQ("test", s7->GetValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(InstanceLabelOverrideTests, WriteToJson)
    {
    bvector<InstanceLabelOverrideValueSpecification*> specs;
    specs.push_back(new InstanceLabelOverrideCompositeValueSpecification(
        {
        new InstanceLabelOverrideCompositeValueSpecification::Part(*new InstanceLabelOverridePropertyValueSpecification("prop1")),
        new InstanceLabelOverrideCompositeValueSpecification::Part(*new InstanceLabelOverrideLocalIdValueSpecification(), true),
        }, ","));
    specs.push_back(new InstanceLabelOverridePropertyValueSpecification(" prop2   "));
    specs.push_back(new InstanceLabelOverrideClassNameValueSpecification(true));
    specs.push_back(new InstanceLabelOverrideClassLabelValueSpecification());
    specs.push_back(new InstanceLabelOverrideBriefcaseIdValueSpecification());
    specs.push_back(new InstanceLabelOverrideLocalIdValueSpecification());
    specs.push_back(new InstanceLabelOverrideStringValueSpecification(" test "));
    InstanceLabelOverride rule(123, true, "s:c", specs);

    Json::Value json = rule.WriteJson();
    Json::Value expected = Json::Reader::DoParse(R"({
        "ruleType": "InstanceLabelOverride",
        "priority": 123,
        "onlyIfNotHandled": true,
        "class": {"schemaName": "s", "className": "c"},
        "values": [{
            "specType": "Composite",
            "separator": ",",
            "parts": [{
                "spec": {
                    "specType": "Property",
                    "propertyName": "prop1"
                }
            }, {
                "spec": {
                    "specType": "LocalId"
                },
                "isRequired": true
            }]
        }, {
            "specType": "Property",
            "propertyName": "prop2"
        }, {
            "specType": "ClassName",
            "full": true
        }, {
            "specType": "ClassLabel"
        }, {
            "specType": "BriefcaseId"
        }, {
            "specType": "LocalId"
        }, {
            "specType": "String",
            "value": " test "
        }]
    })");
    EXPECT_STREQ(ToPrettyString(expected).c_str(), ToPrettyString(json).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(InstanceLabelOverrideTests, LoadsFromXml)
    {
    static Utf8CP xmlString = R"(
        <InstanceLabelOverride ClassName="TestClass" PropertyNames="prop1,   prop2"/>
        )";
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlString);
    ASSERT_EQ(BEXML_Success, xmlStatus);

    InstanceLabelOverride rule;
    EXPECT_TRUE(rule.ReadXml(xml->GetRootElement()));

    EXPECT_STREQ("TestClass", rule.GetClassName().c_str());

    ASSERT_EQ(2, rule.GetValueSpeficications().size());

    InstanceLabelOverridePropertyValueSpecification* s1 = dynamic_cast<InstanceLabelOverridePropertyValueSpecification*>(rule.GetValueSpeficications()[0]);
    ASSERT_TRUE(nullptr != s1);
    EXPECT_STREQ("prop1", s1->GetPropertyName().c_str());

    InstanceLabelOverridePropertyValueSpecification* s2 = dynamic_cast<InstanceLabelOverridePropertyValueSpecification*>(rule.GetValueSpeficications()[1]);
    ASSERT_TRUE(nullptr != s2);
    EXPECT_STREQ("prop2", s2->GetPropertyName().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(InstanceLabelOverrideTests, WriteToXml)
    {
    bvector<InstanceLabelOverrideValueSpecification*> specs;
    specs.push_back(new InstanceLabelOverrideCompositeValueSpecification(
        {
        new InstanceLabelOverrideCompositeValueSpecification::Part(*new InstanceLabelOverridePropertyValueSpecification("prop1")),
        new InstanceLabelOverrideCompositeValueSpecification::Part(*new InstanceLabelOverrideLocalIdValueSpecification(), true),
        }, ","));
    specs.push_back(new InstanceLabelOverridePropertyValueSpecification(" prop2   "));
    specs.push_back(new InstanceLabelOverrideClassNameValueSpecification(true));
    specs.push_back(new InstanceLabelOverrideClassLabelValueSpecification());
    specs.push_back(new InstanceLabelOverrideBriefcaseIdValueSpecification());
    specs.push_back(new InstanceLabelOverrideLocalIdValueSpecification());
    specs.push_back(new InstanceLabelOverrideStringValueSpecification(" test "));
    InstanceLabelOverride rule(123, true, "s:c", specs);

    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateEmpty();
    xml->AddNewElement("Root", nullptr, nullptr);
    rule.WriteXml(xml->GetRootElement());

    static Utf8CP expected = ""
        "<Root>"
            R"(<InstanceLabelOverride Priority="123" OnlyIfNotHandled="true" ClassName="s:c" PropertyNames="prop2"/>)"
        "</Root>";
    EXPECT_STREQ(ToPrettyString(*BeXmlDom::CreateAndReadFromString(xmlStatus, expected)).c_str(), ToPrettyString(*xml).c_str());
    }