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
struct ContentInstancesOfSpecificClassesSpecificationTests : PresentationRulesTests
    {
    };

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentInstancesOfSpecificClassesSpecificationTests, LoadsFromJson)
    {
    static Utf8CP jsonString = R"({
        "specType": "ContentInstancesOfSpecificClasses",
        "classes": {"schemaName": "TestSchema", "classNames": ["TestClass"], "arePolymorphic": true},
        "excludedClasses": {"schemaName": "ExcludedTestSchema", "classNames": ["ExcludedTestClass"], "arePolymorphic": true},
        "handlePropertiesPolymorphically": true,
        "instanceFilter": "filter"
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    EXPECT_FALSE(json.isNull());

    ContentInstancesOfSpecificClassesSpecification spec;
    EXPECT_TRUE(spec.ReadJson(json));
    EXPECT_TRUE(spec.ShouldHandlePropertiesPolymorphically());
    EXPECT_STREQ("filter", spec.GetInstanceFilter().c_str());

    EXPECT_EQ(1, spec.GetClasses().size());
    auto const& multiSchemaClass = *spec.GetClasses().front();

    EXPECT_STREQ("TestSchema", multiSchemaClass.GetSchemaName().c_str());
    EXPECT_EQ(1, multiSchemaClass.GetClassNames().size());
    EXPECT_STREQ("TestClass", multiSchemaClass.GetClassNames().front().c_str());
    EXPECT_TRUE(multiSchemaClass.GetArePolymorphic());

    EXPECT_EQ(1, spec.GetExcludedClasses().size());
    auto const& excludedMultiSchemaClass = *spec.GetExcludedClasses().front();

    EXPECT_STREQ("ExcludedTestSchema", excludedMultiSchemaClass.GetSchemaName().c_str());
    EXPECT_EQ(1, excludedMultiSchemaClass.GetClassNames().size());
    EXPECT_STREQ("ExcludedTestClass", excludedMultiSchemaClass.GetClassNames().front().c_str());
    EXPECT_TRUE(excludedMultiSchemaClass.GetArePolymorphic());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentInstancesOfSpecificClassesSpecificationTests, LoadsFromJsonWithSpecifiedDefaultPolymorphismValue)
    {
    static Utf8CP jsonString = R"({
        "specType": "ContentInstancesOfSpecificClasses",
        "classes": [
            {"schemaName": "TestSchema1", "classNames": ["TestClass1"], "arePolymorphic": false},
            {"schemaName": "TestSchema2", "classNames": ["TestClass2"]}
        ],
        "excludedClasses": [
            {"schemaName": "ExcludedTestSchema1", "classNames": ["ExcludedTestClass1"], "arePolymorphic": false},
            {"schemaName": "ExcludedTestSchema2", "classNames": ["ExcludedTestClass2"]}
        ],
        "handleInstancesPolymorphically": true,
        "handlePropertiesPolymorphically": true
    })";

    Json::Value json = Json::Reader::DoParse(jsonString);
    ASSERT_FALSE(json.isNull());

    ContentInstancesOfSpecificClassesSpecification spec;
    ASSERT_TRUE(spec.ReadJson(json));

    EXPECT_EQ(2, spec.GetClasses().size());
    auto const& includedClasses = spec.GetClasses();
    EXPECT_FALSE(includedClasses[0]->GetArePolymorphic());
    EXPECT_TRUE(includedClasses[1]->GetArePolymorphic());

    EXPECT_EQ(2, spec.GetExcludedClasses().size());
    auto const& excludedClasses = spec.GetExcludedClasses();
    EXPECT_FALSE(excludedClasses[0]->GetArePolymorphic());
    EXPECT_TRUE(excludedClasses[1]->GetArePolymorphic());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentInstancesOfSpecificClassesSpecificationTests, LoadFromJsonFailsWhenClassNamesAreNotSpecified)
    {
    static Utf8CP jsonString = R"({
        "specType": "ContentInstancesOfSpecificClasses"
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    EXPECT_FALSE(json.isNull());

    ContentInstancesOfSpecificClassesSpecification spec;
    EXPECT_FALSE(spec.ReadJson(json));
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentInstancesOfSpecificClassesSpecificationTests, LoadsFromJsonWithDefaultValues)
    {
    static Utf8CP jsonString = R"({
        "specType": "ContentInstancesOfSpecificClasses",
        "classes": {"schemaName": "TestSchema", "classNames": ["TestClass"]}
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    EXPECT_FALSE(json.isNull());

    ContentInstancesOfSpecificClassesSpecification spec;
    EXPECT_TRUE(spec.ReadJson(json));
    EXPECT_FALSE(spec.ShouldHandlePropertiesPolymorphically());
    EXPECT_STREQ("", spec.GetInstanceFilter().c_str());

    EXPECT_EQ(1, spec.GetClasses().size());
    EXPECT_FALSE(spec.GetClasses().front()->GetArePolymorphic());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentInstancesOfSpecificClassesSpecificationTests, LoadsExcludedClassesFromJsonWithDefaultPolymorphismValues)
    {
    static Utf8CP jsonString = R"({
        "specType": "ContentInstancesOfSpecificClasses",
        "classes": {"schemaName": "A", "classNames": ["A1"]},
        "excludedClasses": {"schemaName": "ExcludedTestSchema", "classNames": ["ExcludedTestClass"]}
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    EXPECT_FALSE(json.isNull());

    ContentInstancesOfSpecificClassesSpecification spec;
    EXPECT_TRUE(spec.ReadJson(json));
    EXPECT_EQ(1, spec.GetExcludedClasses().size());
    EXPECT_FALSE(spec.GetExcludedClasses().front()->GetArePolymorphic());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentInstancesOfSpecificClassesSpecificationTests, LoadsFromJsonWithIncludedAndExcludedClasses)
    {
    static Utf8CP jsonString = R"({
        "specType": "ContentInstancesOfSpecificClasses",
        "classes": [{
            "schemaName": "TestSchema1",
            "classNames": ["TestClass1", "TestClass2"],
            "arePolymorphic": true
        }, {
            "schemaName": "TestSchema2",
            "classNames": ["TestClass3"],
            "arePolymorphic": false
        }],
        "excludedClasses": [{
            "schemaName": "ExcludedTestSchema1",
            "classNames": ["ExcludedTestClass1", "ExcludedTestClass2"],
            "arePolymorphic": true
        }, {
            "schemaName": "ExcludedTestSchema2",
            "classNames": ["ExcludedTestClass3"],
            "arePolymorphic": false
        }]
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    EXPECT_FALSE(json.isNull());

    ContentInstancesOfSpecificClassesSpecification spec;
    EXPECT_TRUE(spec.ReadJson(json));

    EXPECT_EQ(2, spec.GetClasses().size());
    auto const& multiSchemaClasses = spec.GetClasses();

    EXPECT_STREQ("TestSchema1", multiSchemaClasses[0]->GetSchemaName().c_str());
    EXPECT_EQ(2, multiSchemaClasses[0]->GetClassNames().size());
    EXPECT_STREQ("TestClass1", multiSchemaClasses[0]->GetClassNames()[0].c_str());
    EXPECT_STREQ("TestClass2", multiSchemaClasses[0]->GetClassNames()[1].c_str());
    EXPECT_TRUE(multiSchemaClasses[0]->GetArePolymorphic());

    EXPECT_STREQ("TestSchema2", multiSchemaClasses[1]->GetSchemaName().c_str());
    EXPECT_EQ(1, multiSchemaClasses[1]->GetClassNames().size());
    EXPECT_STREQ("TestClass3", multiSchemaClasses[1]->GetClassNames()[0].c_str());
    EXPECT_FALSE(multiSchemaClasses[1]->GetArePolymorphic());


    EXPECT_EQ(2, spec.GetExcludedClasses().size());
    auto const& excludedMultiSchemaClasses = spec.GetExcludedClasses();

    EXPECT_STREQ("ExcludedTestSchema1", excludedMultiSchemaClasses[0]->GetSchemaName().c_str());
    EXPECT_EQ(2, excludedMultiSchemaClasses[0]->GetClassNames().size());
    EXPECT_STREQ("ExcludedTestClass1", excludedMultiSchemaClasses[0]->GetClassNames()[0].c_str());
    EXPECT_STREQ("ExcludedTestClass2", excludedMultiSchemaClasses[0]->GetClassNames()[1].c_str());
    EXPECT_TRUE(excludedMultiSchemaClasses[0]->GetArePolymorphic());

    EXPECT_STREQ("ExcludedTestSchema2", excludedMultiSchemaClasses[1]->GetSchemaName().c_str());
    EXPECT_EQ(1, excludedMultiSchemaClasses[1]->GetClassNames().size());
    EXPECT_STREQ("ExcludedTestClass3", excludedMultiSchemaClasses[1]->GetClassNames()[0].c_str());
    EXPECT_FALSE(excludedMultiSchemaClasses[1]->GetArePolymorphic());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentInstancesOfSpecificClassesSpecificationTests, FailsToLoadFromJsonWithEmptyClassesArray)
    {
    static Utf8CP jsonString = R"({
        "specType": "ContentInstancesOfSpecificClasses",
        "classes": []
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    EXPECT_FALSE(json.isNull());

    ContentInstancesOfSpecificClassesSpecification spec;
    EXPECT_FALSE(spec.ReadJson(json));
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentInstancesOfSpecificClassesSpecificationTests, FailsToLoadFromJsonWithSchemaSpecifiedButClassesNot)
    {
    static Utf8CP jsonString = R"({
        "specType": "ContentInstancesOfSpecificClasses",
        "classes": {
            "schemaName": "TestSchema",
            "classNames": []
        }
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    EXPECT_FALSE(json.isNull());

    ContentInstancesOfSpecificClassesSpecification spec;
    EXPECT_FALSE(spec.ReadJson(json));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentInstancesOfSpecificClassesSpecificationTests, WriteToJson)
    {
    ContentInstancesOfSpecificClassesSpecification spec(123, true, "filter",
        bvector<MultiSchemaClass*> {
            new MultiSchemaClass("s1", true, bvector<Utf8String> {"c1", "c2"}),
            new MultiSchemaClass("s2", false, bvector<Utf8String> {"c3"})
        },
        bvector<MultiSchemaClass*> {
            new MultiSchemaClass("es1", true, bvector<Utf8String> {"ec1", "ec2"}),
            new MultiSchemaClass("es2", false, bvector<Utf8String> {"ec3"})
        }, true);

    Json::Value json = spec.WriteJson();
    Json::Value expected = Json::Reader::DoParse(R"({
        "specType": "ContentInstancesOfSpecificClasses",
        "priority": 123,
        "onlyIfNotHandled": true,
        "instanceFilter": "filter",
        "classes": [
            {"schemaName": "s1", "classNames": ["c1", "c2"], "arePolymorphic": true},
            {"schemaName": "s2", "classNames": ["c3"]}
        ],
        "excludedClasses": [
            {"schemaName": "es1", "classNames": ["ec1", "ec2"], "arePolymorphic": true},
            {"schemaName": "es2", "classNames": ["ec3"]}
        ],
        "handlePropertiesPolymorphically": true
    })");

    EXPECT_STREQ(ToPrettyString(expected).c_str(), ToPrettyString(json).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentInstancesOfSpecificClassesSpecificationTests, LoadsFromXml)
    {
    static Utf8CP xmlString = R"(
        <ContentInstancesOfSpecificClasses ClassNames="TestSchema:TestClass" ArePolymorphic="true" InstanceFilter="filter"/>
        )";
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlString);
    ASSERT_EQ(BEXML_Success, xmlStatus);

    ContentInstancesOfSpecificClassesSpecification spec;
    EXPECT_TRUE(spec.ReadXml(xml->GetRootElement()));
    EXPECT_FALSE(spec.ShouldHandlePropertiesPolymorphically());
    EXPECT_STREQ("filter", spec.GetInstanceFilter().c_str());

    EXPECT_EQ(1, spec.GetClasses().size());
    auto const& includedClass = spec.GetClasses().front();
    EXPECT_STREQ("TestSchema", includedClass->GetSchemaName().c_str());
    EXPECT_EQ(1, includedClass->GetClassNames().size());
    EXPECT_STREQ("TestClass", includedClass->GetClassNames().front().c_str());
    EXPECT_TRUE(includedClass->GetArePolymorphic());
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentInstancesOfSpecificClassesSpecificationTests, LoadFromXmlFailsWhenClassNamesAreNotSpecified)
    {
    static Utf8CP xmlString = "<ContentInstancesOfSpecificClasses/>";
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlString);
    ASSERT_EQ(BEXML_Success, xmlStatus);

    ContentInstancesOfSpecificClassesSpecification spec;
    EXPECT_FALSE(spec.ReadXml(xml->GetRootElement()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentInstancesOfSpecificClassesSpecificationTests, LoadsFromXmlWithDefaultValues)
    {
    static Utf8CP xmlString = R"(
        <ContentInstancesOfSpecificClasses ClassNames="TestSchema:ClassA"/>
        )";
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlString);
    ASSERT_EQ(BEXML_Success, xmlStatus);

    ContentInstancesOfSpecificClassesSpecification spec;
    EXPECT_TRUE(spec.ReadXml(xml->GetRootElement()));
    EXPECT_STREQ("", spec.GetInstanceFilter().c_str());

    EXPECT_EQ(1, spec.GetClasses().size());
    auto const& includedClass = spec.GetClasses().front();

    EXPECT_STREQ("TestSchema", includedClass->GetSchemaName().c_str());
    EXPECT_EQ(1, includedClass->GetClassNames().size());
    EXPECT_STREQ("ClassA", includedClass->GetClassNames().front().c_str());
    EXPECT_FALSE(includedClass->GetArePolymorphic());
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentInstancesOfSpecificClassesSpecificationTests, WritesToXml)
    {
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateEmpty();
    xml->AddNewElement("Root", nullptr, nullptr);

    ContentInstancesOfSpecificClassesSpecification spec;
    spec.SetClasses(bvector<MultiSchemaClass*> { new MultiSchemaClass("TestSchema", true, bvector<Utf8String> {"TestClass"})});
    spec.SetInstanceFilter("filter");
    spec.WriteXml(xml->GetRootElement());

    static Utf8CP expected = ""
        "<Root>"
            R"(<ContentInstancesOfSpecificClasses Priority="1000" ShowImages="false" OnlyIfNotHandled="false"
                ClassNames="TestSchema:TestClass" InstanceFilter="filter" ArePolymorphic="true"/>)"
        "</Root>";

    EXPECT_STREQ(ToPrettyString(*BeXmlDom::CreateAndReadFromString(xmlStatus, expected)).c_str(), ToPrettyString(*xml).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentInstancesOfSpecificClassesSpecificationTests, ComputesCorrectHashes)
    {
    Utf8CP DEFAULT_HASH = "031917e1e6b15bd9e57863cdd91e7e41";

    // Make sure that introducing additional attributes with default values don't affect the hash
    ContentInstancesOfSpecificClassesSpecification defaultSpec;
    EXPECT_STREQ(DEFAULT_HASH, defaultSpec.GetHash().c_str());

    // Make sure that copy has the same hash
    ContentInstancesOfSpecificClassesSpecification copySpec(defaultSpec);
    EXPECT_STREQ(DEFAULT_HASH, copySpec.GetHash().c_str());

    // Test that each attribute affects the hash
    ContentInstancesOfSpecificClassesSpecification specWithClasses(defaultSpec);
    specWithClasses.SetClasses(bvector<MultiSchemaClass*> { new MultiSchemaClass("TestSchema", true, bvector<Utf8String> {"TestClass"})});
    EXPECT_STRNE(DEFAULT_HASH, specWithClasses.GetHash().c_str());
    specWithClasses.SetClasses(bvector<MultiSchemaClass*>());
    EXPECT_STREQ(DEFAULT_HASH, specWithClasses.GetHash().c_str());

    ContentInstancesOfSpecificClassesSpecification specWithExcludedClasses(defaultSpec);
    specWithExcludedClasses.SetExcludedClasses(bvector<MultiSchemaClass*> { new MultiSchemaClass("ExcludeTestSchema", true, bvector<Utf8String> {"TestClass"})});
    EXPECT_STRNE(DEFAULT_HASH, specWithExcludedClasses.GetHash().c_str());
    specWithExcludedClasses.SetExcludedClasses(bvector<MultiSchemaClass*>());
    EXPECT_STREQ(DEFAULT_HASH, specWithExcludedClasses.GetHash().c_str());

    ContentInstancesOfSpecificClassesSpecification specWithHandlePropertiesPolymorphically(defaultSpec);
    specWithHandlePropertiesPolymorphically.SetShouldHandlePropertiesPolymorphically(true);
    EXPECT_STRNE(DEFAULT_HASH, specWithHandlePropertiesPolymorphically.GetHash().c_str());
    specWithHandlePropertiesPolymorphically.SetShouldHandlePropertiesPolymorphically(false);
    EXPECT_STREQ(DEFAULT_HASH, specWithHandlePropertiesPolymorphically.GetHash().c_str());

    ContentInstancesOfSpecificClassesSpecification specWithInstanceFilter(defaultSpec);
    specWithInstanceFilter.SetInstanceFilter("true");
    EXPECT_STRNE(DEFAULT_HASH, specWithInstanceFilter.GetHash().c_str());
    specWithInstanceFilter.SetInstanceFilter("");
    EXPECT_STREQ(DEFAULT_HASH, specWithInstanceFilter.GetHash().c_str());
    }
