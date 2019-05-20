/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/BeTest.h>
#include <ECPresentation/Content.h>
#include "../NonPublished/RulesEngine/TestHelpers.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

#define TEST_SCHEMA R"xml(<?xml version="1.0" encoding="UTF-8"?>
                    <ECSchema schemaName="TestSchema" nameSpacePrefix="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
                        <PropertyCategory typeName="Custom" displayLabel="Custom" description="Custom category" priority="123" />
                        <ECClass typeName="TestClass" isDomainClass="True">
                            <ECProperty propertyName="Prop1" typeName="string" />
                            <ECProperty propertyName="Prop2" typeName="string" category="Custom" />
                        </ECClass>
                        <ECRelationshipClass typeName="ElementOwnsChildElements" strength="embedding" modifier="None">
                            <Source multiplicity="(0..1)" roleLabel="owns child" polymorphic="true">
                                <Class class="TestClass"/>
                            </Source>
                            <Target multiplicity="(0..*)" roleLabel="is owned by parent" polymorphic="true">
                                <Class class="TestClass"/>
                            </Target>
                        </ECRelationshipClass>                    
                    </ECSchema>)xml"

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                10/2016
+===============+===============+===============+===============+===============+======*/
struct DefaultCategorySupplierTests : ECPresentationTest
    {
    DefaultCategorySupplier m_supplier;
    ECSchemaReadContextPtr m_schemaContext;
    ECSchemaPtr m_schema;
    ECClassCP m_class;
    ECRelationshipClassCP m_relationship;

    void SetUp() override
        {
        ECPresentationTest::SetUp();
        Localization::Init();

        BeFileName assetsPath;
        BeTest::GetHost().GetDgnPlatformAssetsDirectory(assetsPath);
        ECSchemaReadContext::Initialize(assetsPath);

        m_schemaContext = ECSchemaReadContext::CreateContext();
        ECSchema::ReadFromXmlString(m_schema, TEST_SCHEMA, *m_schemaContext);
        m_class = m_schema->GetClassCP("TestClass");
        m_relationship = m_schema->GetClassCP("ElementOwnsChildElements")->GetRelationshipClassCP();
        }

    void TearDown() override
        {
        Localization::Terminate();
        }
    };

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DefaultCategorySupplierTests, ReturnsCustomCategoryWhenItIsSet)
    {
    ECPropertyCP prop = m_class->GetPropertyP("Prop2");
    ContentDescriptor::Category category = m_supplier.GetCategory(*m_class, RelatedClassPath(), *prop, RelationshipMeaning::SameInstance);
    EXPECT_STREQ("Custom", category.GetName().c_str());
    EXPECT_STREQ("Custom", category.GetLabel().c_str());
    EXPECT_STREQ("Custom category", category.GetDescription().c_str());
    EXPECT_EQ(123, category.GetPriority());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DefaultCategorySupplierTests, ReturnsPropertyClassCategoryWhenThereIsNoCustomCategoryAndPropertyIsRelated)
    {
    ECPropertyCP prop = m_class->GetPropertyP("Prop1");
    ContentDescriptor::Category category = m_supplier.GetCategory(*m_class, {RelatedClass(*m_class, *m_class, *m_relationship, true)}, 
        *prop, RelationshipMeaning::RelatedInstance);
    EXPECT_STREQ(m_class->GetName().c_str(), category.GetName().c_str());
    EXPECT_STREQ(m_class->GetDisplayLabel().c_str(), category.GetLabel().c_str());
    EXPECT_STREQ(m_class->GetDescription().c_str(), category.GetDescription().c_str());
    EXPECT_EQ(DefaultCategorySupplier::NESTED_CONTENT_CATEGORY_PRIORITY, category.GetPriority());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DefaultCategorySupplierTests, ReturnsDefaultCategoryWhenThereIsNoCustomCategoryAndPropertyIsNotRelated)
    {
    ECPropertyCP prop = m_class->GetPropertyP("Prop1");
    ContentDescriptor::Category category = m_supplier.GetCategory(*m_class, RelatedClassPath(), *prop, RelationshipMeaning::SameInstance);
    EXPECT_EQ(ContentDescriptor::Category::GetDefaultCategory(), category);
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DefaultCategorySupplierTests, ReturnsClassBasedCategoryWhenRequestingCategoryForNestedContent)
    {
    ContentDescriptor::Category expected(m_class->GetName(), m_class->GetDisplayLabel(), "", DefaultCategorySupplier::NESTED_CONTENT_CATEGORY_PRIORITY);
    ContentDescriptor::Category actual = m_supplier.GetCategory(*m_class, RelatedClassPath(), *m_class);
    EXPECT_EQ(expected, actual);
    }
