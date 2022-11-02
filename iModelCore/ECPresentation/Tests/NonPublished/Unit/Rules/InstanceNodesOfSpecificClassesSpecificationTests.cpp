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
struct InstanceNodesOfSpecificClassesSpecificationTests : PresentationRulesTests
    {
    };

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(InstanceNodesOfSpecificClassesSpecificationTests, LoadsFromJson)
    {
    static Utf8CP jsonString = R"({
        "specType": "InstanceNodesOfSpecificClasses",
        "classes": {"schemaName": "TestSchema", "classNames": ["TestClass"], "arePolymorphic": true},
        "excludedClasses": {"schemaName": "ExcludedTestSchema", "classNames": ["ExcludedTestClass"], "arePolymorphic": true},
        "groupByClass": false,
        "groupByLabel": false,
        "instanceFilter": "filter"
    })";

    Json::Value json = Json::Reader::DoParse(jsonString);
    EXPECT_FALSE(json.isNull());

    InstanceNodesOfSpecificClassesSpecification spec;
    EXPECT_TRUE(spec.ReadJson(json));
    EXPECT_FALSE(spec.GetGroupByClass());
    EXPECT_FALSE(spec.GetGroupByLabel());
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
TEST_F(InstanceNodesOfSpecificClassesSpecificationTests, LoadsFromJsonWithSpecifiedDefaultPolymorphismValue)
    {
    static Utf8CP jsonString = R"({
        "specType": "InstanceNodesOfSpecificClasses",
        "classes": [
            {"schemaName": "TestSchema1", "classNames": ["TestClass1"], "arePolymorphic": false},
            {"schemaName": "TestSchema2", "classNames": ["TestClass2"]}
        ],
        "excludedClasses": [
            {"schemaName": "ExcludedTestSchema1", "classNames": ["ExcludedTestClass1"], "arePolymorphic": false},
            {"schemaName": "ExcludedTestSchema2", "classNames": ["ExcludedTestClass2"]}
        ],
        "arePolymorphic": true
    })";

    Json::Value json = Json::Reader::DoParse(jsonString);
    ASSERT_FALSE(json.isNull());

    InstanceNodesOfSpecificClassesSpecification spec;
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
TEST_F(InstanceNodesOfSpecificClassesSpecificationTests, LoadsFromJsonWithDefaultValues)
    {
    static Utf8CP jsonString = R"({
        "specType": "InstanceNodesOfSpecificClasses",
        "classes": {"schemaName": "TestSchema", "classNames": ["TestClass"]}
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    ASSERT_FALSE(json.isNull());

    InstanceNodesOfSpecificClassesSpecification spec;
    ASSERT_TRUE(spec.ReadJson(json));
    EXPECT_TRUE(spec.GetGroupByClass());
    EXPECT_TRUE(spec.GetGroupByLabel());
    EXPECT_STREQ("", spec.GetInstanceFilter().c_str());

    EXPECT_EQ(1, spec.GetClasses().size());
    EXPECT_FALSE(spec.GetClasses().front()->GetArePolymorphic());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(InstanceNodesOfSpecificClassesSpecificationTests, LoadsExcludedClassesFromJsonWithDefaultPolymorphismValues)
    {
    static Utf8CP jsonString = R"({
        "specType": "InstanceNodesOfSpecificClasses",
        "classes": {"schemaName": "A", "classNames": ["A1"]},
        "excludedClasses": {"schemaName": "ExcludedTestSchema", "classNames": ["ExcludedTestClass"]}
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    EXPECT_FALSE(json.isNull());

    InstanceNodesOfSpecificClassesSpecification spec;
    EXPECT_TRUE(spec.ReadJson(json));
    EXPECT_EQ(1, spec.GetExcludedClasses().size());
    EXPECT_FALSE(spec.GetExcludedClasses().front()->GetArePolymorphic());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(InstanceNodesOfSpecificClassesSpecificationTests, LoadsFromJsonFailsWhenClassesAttributeIsNotSpecified)
    {
    static Utf8CP jsonString = R"({
        "specType": "InstanceNodesOfSpecificClasses"
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    EXPECT_FALSE(json.isNull());

    InstanceNodesOfSpecificClassesSpecification spec;
    EXPECT_FALSE(spec.ReadJson(json));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(InstanceNodesOfSpecificClassesSpecificationTests, WriteToJson)
    {
    InstanceNodesOfSpecificClassesSpecification spec(456, ChildrenHint::Always, true, true, true, true, "filter",
        bvector<MultiSchemaClass*> {
            new MultiSchemaClass("s1", true, bvector<Utf8String> {"c1", "c2"}),
            new MultiSchemaClass("s2", false, bvector<Utf8String> {"c3"})
        },
        bvector<MultiSchemaClass*> {
            new MultiSchemaClass("es1", true, bvector<Utf8String> {"ec1", "ec2"}),
            new MultiSchemaClass("es2", false, bvector<Utf8String> {"ec3"})
        });

    Json::Value json = spec.WriteJson();
    Json::Value expected = Json::Reader::DoParse(R"({
        "specType": "InstanceNodesOfSpecificClasses",
        "priority": 456,
        "hasChildren": "Always",
        "hideNodesInHierarchy": true,
        "hideIfNoChildren": true,
        "classes": [
            {"schemaName": "s1", "classNames": ["c1", "c2"], "arePolymorphic": true},
            {"schemaName": "s2", "classNames": ["c3"]}
        ],
        "excludedClasses": [
            {"schemaName": "es1", "classNames": ["ec1", "ec2"], "arePolymorphic": true},
            {"schemaName": "es2", "classNames": ["ec3"]}
        ],
        "instanceFilter": "filter"
    })");

    EXPECT_STREQ(ToPrettyString(expected).c_str(), ToPrettyString(json).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(InstanceNodesOfSpecificClassesSpecificationTests, LoadsFromXml)
    {
    static Utf8CP xmlString = R"(
        <InstancesOfSpecificClasses ClassNames="TestSchema:TestClass" ArePolymorphic="true" GroupByClass="false"
         GroupByLabel="false" ShowEmptyGroups="true" InstanceFilter="filter"/>
        )";
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlString);
    ASSERT_EQ(BEXML_Success, xmlStatus);

    InstanceNodesOfSpecificClassesSpecification spec;
    EXPECT_TRUE(spec.ReadXml(xml->GetRootElement()));
    EXPECT_FALSE(spec.GetGroupByClass());
    EXPECT_FALSE(spec.GetGroupByLabel());
    EXPECT_TRUE(spec.GetShowEmptyGroups());
    EXPECT_STREQ("filter", spec.GetInstanceFilter().c_str());

    EXPECT_EQ(1, spec.GetClasses().size());
    auto const& multiSchemaClass = *spec.GetClasses().front();

    EXPECT_STREQ("TestSchema", multiSchemaClass.GetSchemaName().c_str());
    EXPECT_EQ(1, multiSchemaClass.GetClassNames().size());
    EXPECT_STREQ("TestClass", multiSchemaClass.GetClassNames().front().c_str());
    EXPECT_TRUE(multiSchemaClass.GetArePolymorphic());
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(InstanceNodesOfSpecificClassesSpecificationTests, LoadsFromXmlWithDefaultValues)
    {
    static Utf8CP xmlString = R"(
        <InstancesOfSpecificClasses ClassNames="TestSchema:TestClass"/>
        )";
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlString);
    ASSERT_EQ(BEXML_Success, xmlStatus);

    InstanceNodesOfSpecificClassesSpecification spec;
    EXPECT_TRUE(spec.ReadXml(xml->GetRootElement()));
    EXPECT_TRUE(spec.GetGroupByClass());
    EXPECT_TRUE(spec.GetGroupByLabel());
    EXPECT_FALSE(spec.GetShowEmptyGroups());
    EXPECT_STREQ("", spec.GetInstanceFilter().c_str());

    EXPECT_EQ(1, spec.GetClasses().size());
    auto const& multiSchemaClass = *spec.GetClasses().front();

    EXPECT_STREQ("TestSchema", multiSchemaClass.GetSchemaName().c_str());
    EXPECT_EQ(1, multiSchemaClass.GetClassNames().size());
    EXPECT_STREQ("TestClass", multiSchemaClass.GetClassNames().front().c_str());
    EXPECT_FALSE(multiSchemaClass.GetArePolymorphic());
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(InstanceNodesOfSpecificClassesSpecificationTests, LoadsFromXmlFailsWhenClassNamesAttributeIsNotSpecified)
    {
    static Utf8CP xmlString = "<InstancesOfSpecificClasses/>";
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlString);
    ASSERT_EQ(BEXML_Success, xmlStatus);

    InstanceNodesOfSpecificClassesSpecification spec;
    EXPECT_FALSE(spec.ReadXml(xml->GetRootElement()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(InstanceNodesOfSpecificClassesSpecificationTests, WritesToXml)
    {
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateEmpty();
    xml->AddNewElement("Root", nullptr, nullptr);

    InstanceNodesOfSpecificClassesSpecification spec;
    spec.SetClasses(bvector<MultiSchemaClass*>{new MultiSchemaClass("TestSchema", true, bvector<Utf8String>{ "TestClass" })});
    spec.SetGroupByClass(false);
    spec.SetGroupByLabel(false);
    spec.SetShowEmptyGroups(true);
    spec.WriteXml(xml->GetRootElement());

    static Utf8CP expected = ""
        "<Root>"
           R"(<InstancesOfSpecificClasses Priority="1000" HasChildren="Unknown" HideNodesInHierarchy="false"
               HideIfNoChildren="false" DoNotSort="false" ClassNames="TestSchema:TestClass"
               GroupByClass="false" GroupByLabel="false" ShowEmptyGroups="true" InstanceFilter="" ArePolymorphic="true"/>)"
        "</Root>";
    EXPECT_STREQ(ToPrettyString(*BeXmlDom::CreateAndReadFromString(xmlStatus, expected)).c_str(), ToPrettyString(*xml).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(InstanceNodesOfSpecificClassesSpecificationTests, ComputesCorrectHashes)
    {
    Utf8CP DEFAULT_HASH = "c65c2c7d801c5ac37d9dab26b2eac21c";

    // first we make sure that introducing additional attributes with default values don't affect the hash
    InstanceNodesOfSpecificClassesSpecification defaultSpec;
    EXPECT_STREQ(DEFAULT_HASH, defaultSpec.GetHash().c_str());

    // then we make sure that copy has the same hash
    InstanceNodesOfSpecificClassesSpecification copySpec(defaultSpec);
    EXPECT_STREQ(DEFAULT_HASH, copySpec.GetHash().c_str());

    // then we test that each attribute affects the hash
    InstanceNodesOfSpecificClassesSpecification specWithGroupByClass(defaultSpec);
    specWithGroupByClass.SetGroupByClass(false);
    EXPECT_STRNE(defaultSpec.GetHash().c_str(), specWithGroupByClass.GetHash().c_str());
    specWithGroupByClass.SetGroupByClass(true);
    EXPECT_STREQ(defaultSpec.GetHash().c_str(), specWithGroupByClass.GetHash().c_str());

    InstanceNodesOfSpecificClassesSpecification specWithGroupByLabel(defaultSpec);
    specWithGroupByLabel.SetGroupByLabel(false);
    EXPECT_STRNE(defaultSpec.GetHash().c_str(), specWithGroupByLabel.GetHash().c_str());
    specWithGroupByLabel.SetGroupByLabel(true);
    EXPECT_STREQ(defaultSpec.GetHash().c_str(), specWithGroupByLabel.GetHash().c_str());

    InstanceNodesOfSpecificClassesSpecification specWithClasses(defaultSpec);
    specWithClasses.SetClasses(bvector<MultiSchemaClass*> {new MultiSchemaClass()});
    EXPECT_STRNE(defaultSpec.GetHash().c_str(), specWithClasses.GetHash().c_str());
    specWithClasses.SetClasses(bvector<MultiSchemaClass*>());
    EXPECT_STREQ(defaultSpec.GetHash().c_str(), specWithClasses.GetHash().c_str());

    InstanceNodesOfSpecificClassesSpecification specWithExcludedClasses(defaultSpec);
    specWithExcludedClasses.SetExcludedClasses(bvector<MultiSchemaClass*> {new MultiSchemaClass()});
    EXPECT_STRNE(defaultSpec.GetHash().c_str(), specWithExcludedClasses.GetHash().c_str());
    specWithExcludedClasses.SetExcludedClasses(bvector<MultiSchemaClass*>());
    EXPECT_STREQ(defaultSpec.GetHash().c_str(), specWithExcludedClasses.GetHash().c_str());

    InstanceNodesOfSpecificClassesSpecification specWithInstanceFilter(defaultSpec);
    specWithInstanceFilter.SetInstanceFilter("a");
    EXPECT_STRNE(defaultSpec.GetHash().c_str(), specWithInstanceFilter.GetHash().c_str());
    specWithInstanceFilter.SetInstanceFilter("");
    EXPECT_STREQ(defaultSpec.GetHash().c_str(), specWithInstanceFilter.GetHash().c_str());

    InstanceNodesOfSpecificClassesSpecification specWithPriority(defaultSpec);
    specWithPriority.SetPriority(1);
    EXPECT_STRNE(defaultSpec.GetHash().c_str(), specWithPriority.GetHash().c_str());
    specWithPriority.SetPriority(1000);
    EXPECT_STREQ(defaultSpec.GetHash().c_str(), specWithPriority.GetHash().c_str());

    InstanceNodesOfSpecificClassesSpecification specWithHasChildren(defaultSpec);
    specWithHasChildren.SetHasChildren(ChildrenHint::Always);
    EXPECT_STRNE(defaultSpec.GetHash().c_str(), specWithHasChildren.GetHash().c_str());
    specWithHasChildren.SetHasChildren(ChildrenHint::Unknown);
    EXPECT_STREQ(defaultSpec.GetHash().c_str(), specWithHasChildren.GetHash().c_str());

    InstanceNodesOfSpecificClassesSpecification specWithHideNodesInHierarchy(defaultSpec);
    specWithHideNodesInHierarchy.SetHideNodesInHierarchy(true);
    EXPECT_STRNE(defaultSpec.GetHash().c_str(), specWithHideNodesInHierarchy.GetHash().c_str());
    specWithHideNodesInHierarchy.SetHideNodesInHierarchy(false);
    EXPECT_STREQ(defaultSpec.GetHash().c_str(), specWithHideNodesInHierarchy.GetHash().c_str());

    InstanceNodesOfSpecificClassesSpecification specWithHideIfNoChildren(defaultSpec);
    specWithHideIfNoChildren.SetHideIfNoChildren(true);
    EXPECT_STRNE(defaultSpec.GetHash().c_str(), specWithHideIfNoChildren.GetHash().c_str());
    specWithHideIfNoChildren.SetHideIfNoChildren(false);
    EXPECT_STREQ(defaultSpec.GetHash().c_str(), specWithHideIfNoChildren.GetHash().c_str());

    InstanceNodesOfSpecificClassesSpecification specWithSuppressSimilarAncestorsCheck(defaultSpec);
    specWithSuppressSimilarAncestorsCheck.SetSuppressSimilarAncestorsCheck(true);
    EXPECT_STRNE(defaultSpec.GetHash().c_str(), specWithSuppressSimilarAncestorsCheck.GetHash().c_str());
    specWithSuppressSimilarAncestorsCheck.SetSuppressSimilarAncestorsCheck(false);
    EXPECT_STREQ(defaultSpec.GetHash().c_str(), specWithSuppressSimilarAncestorsCheck.GetHash().c_str());

    InstanceNodesOfSpecificClassesSpecification specWithHideExpression(defaultSpec);
    specWithHideExpression.SetHideExpression("a");
    EXPECT_STRNE(defaultSpec.GetHash().c_str(), specWithHideExpression.GetHash().c_str());
    specWithHideExpression.SetHideExpression("");
    EXPECT_STREQ(defaultSpec.GetHash().c_str(), specWithHideExpression.GetHash().c_str());

    InstanceNodesOfSpecificClassesSpecification specWithDoNotSort(defaultSpec);
    specWithDoNotSort.SetDoNotSort(true);
    EXPECT_STRNE(defaultSpec.GetHash().c_str(), specWithDoNotSort.GetHash().c_str());
    specWithDoNotSort.SetDoNotSort(false);
    EXPECT_STREQ(defaultSpec.GetHash().c_str(), specWithDoNotSort.GetHash().c_str());

    InstanceNodesOfSpecificClassesSpecification specWithRelatedInstances(defaultSpec);
    specWithRelatedInstances.AddRelatedInstance(*new RelatedInstanceSpecification());
    EXPECT_STRNE(defaultSpec.GetHash().c_str(), specWithRelatedInstances.GetHash().c_str());

    InstanceNodesOfSpecificClassesSpecification specWithNestedRules(defaultSpec);
    specWithNestedRules.AddNestedRule(*new ChildNodeRule());
    EXPECT_STRNE(defaultSpec.GetHash().c_str(), specWithNestedRules.GetHash().c_str());
    }
