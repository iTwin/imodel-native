/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/PresentationRules/RelatedPropertiesSpecificationTests.cpp $
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
struct RelatedPropertiesSpecificationsTests : PresentationRulesTests
    {
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RelatedPropertiesSpecificationsTests, LoadsFromXml)
    {
    static Utf8CP xmlString = R"(
        <RelatedProperties RelationshipClassNames="Relationship:Names" 
            RelatedClassNames="Related:Class,Names"
            RequiredDirection="Backward" 
            PropertyNames="Property,Names"
            RelationshipMeaning="SameInstance">
            <RelatedProperties  />
        </RelatedProperties>
        )";
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlString);
    ASSERT_EQ(BEXML_Success, xmlStatus);
    
    RelatedPropertiesSpecification spec;
    EXPECT_TRUE(spec.ReadXml(xml->GetRootElement()));
    EXPECT_STREQ("Related:Class,Names", spec.GetRelatedClassNames().c_str());
    EXPECT_STREQ("Relationship:Names", spec.GetRelationshipClassNames().c_str());
    EXPECT_STREQ("Property,Names", spec.GetPropertyNames().c_str());
    EXPECT_EQ(RequiredRelationDirection_Backward, spec.GetRequiredRelationDirection());
    EXPECT_EQ(RelationshipMeaning::SameInstance, spec.GetRelationshipMeaning());
    EXPECT_EQ(1, spec.GetNestedRelatedProperties().size());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RelatedPropertiesSpecificationsTests, WritesToXml)
    {
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateEmpty();
    xml->AddNewElement("Root", nullptr, nullptr);

    RelatedPropertiesSpecification spec(RequiredRelationDirection_Backward, "Relationship:Names",
        "Related:Class,Names", "Property,Names", RelationshipMeaning::RelatedInstance);
    spec.GetNestedRelatedPropertiesR().push_back(new RelatedPropertiesSpecification());
    spec.WriteXml(xml->GetRootElement());
    
    static Utf8CP expected = ""
        "<Root>"
            "<RelatedProperties "
                R"(RelationshipClassNames="Relationship:Names" )"
                R"(RelatedClassNames="Related:Class,Names" )"
                R"(PropertyNames="Property,Names" )"
                R"(RequiredDirection="Backward" )"
                R"(RelationshipMeaning="RelatedInstance">)"
                "<RelatedProperties "
                    R"(RelationshipClassNames="" )"
                    R"(RelatedClassNames="" )"
                    R"(PropertyNames="" )"
                    R"(RequiredDirection="Both" )"
                    R"(RelationshipMeaning="RelatedInstance" />)"
            "</RelatedProperties>"
        "</Root>";
    EXPECT_STREQ(ToPrettyString(*BeXmlDom::CreateAndReadFromString(xmlStatus, expected)).c_str(), ToPrettyString(*xml).c_str());
    }