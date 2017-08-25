/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/DefaultCategorySupplierTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/BeTest.h>
#include <ECPresentation/Content.h>
#include "../NonPublished/RulesEngine/TestHelpers.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

#define TEST_SCHEMA \
    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"                                                                                             \
    "<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"ts\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">" \
    "    <ECClass typeName=\"TestClass\" isDomainClass=\"True\">"                                                                            \
    "        <ECProperty propertyName=\"Prop1\" typeName=\"string\" />"                                                                      \
    "        <ECProperty propertyName=\"Prop2\" typeName=\"string\">"                                                                        \
    "           <ECCustomAttributes>"                                                                                                        \
    "               <Category xmlns=\"EditorCustomAttributes.01.00\">"                                                                       \
    "                   <Standard>1</Standard>"                                                                                              \
    "                   <Name>Custom</Name>"                                                                                                 \
    "                   <DisplayLabel>Custom</DisplayLabel>"                                                                                 \
    "                   <Description>Custom category</Description>"                                                                          \
    "                   <Priority>123</Priority>"                                                                                            \
    "                   <Expand>true</Expand>"                                                                                               \
    "               </Category>"                                                                                                             \
    "           </ECCustomAttributes>"                                                                                                       \
    "        </ECProperty>"                                                                                                                  \
    "        <ECProperty propertyName=\"Prop3\" typeName=\"string\">"                                                                        \
    "           <ECCustomAttributes>"                                                                                                        \
    "               <Category xmlns=\"EditorCustomAttributes.01.00\">"                                                                       \
    "                   <Standard>32</Standard>"                                                                                             \
    "               </Category>"                                                                                                             \
    "           </ECCustomAttributes>"                                                                                                       \
    "        </ECProperty>"                                                                                                                  \
    "    </ECClass>"                                                                                                                         \
    "</ECSchema>"

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                10/2016
+===============+===============+===============+===============+===============+======*/
struct DefaultCategorySupplierTests : ::testing::Test
    {
    DefaultCategorySupplier m_supplier;
    ECSchemaReadContextPtr m_schemaContext;
    ECSchemaPtr m_schema;
    ECClassCP m_class;

    void SetUp() override
        {
        Localization::Init();

        BeFileName assetsPath;
        BeTest::GetHost().GetDgnPlatformAssetsDirectory(assetsPath);
        ECSchemaReadContext::Initialize(assetsPath);

        m_schemaContext = ECSchemaReadContext::CreateContext();
        ECSchema::ReadFromXmlString(m_schema, TEST_SCHEMA, *m_schemaContext);
        m_class = m_schema->GetClassCP("TestClass");
        }

    void TearDown() override
        {
        Localization::Terminate();
        }
    };

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DefaultCategorySupplierTests, ReturnsMiscellaneousCategoryWhenNoCustomAttributeIsSet)
    {
    ECPropertyCP prop = m_class->GetPropertyP("Prop1");
    ContentDescriptor::Category category = m_supplier.GetCategory(*m_class, RelatedClassPath(), *prop);
    EXPECT_EQ(ContentDescriptor::Category::GetCategory(ContentDescriptor::Category::Standard::Miscellaneous), category);
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DefaultCategorySupplierTests, ReturnsCustomCategoryWhenCustomAttributeNameIsSet)
    {
    ECPropertyCP prop = m_class->GetPropertyP("Prop2");
    ContentDescriptor::Category category = m_supplier.GetCategory(*m_class, RelatedClassPath(), *prop);
    EXPECT_STREQ("Custom", category.GetName().c_str());
    EXPECT_STREQ("Custom", category.GetLabel().c_str());
    EXPECT_EQ(123, category.GetPriority());
    EXPECT_TRUE(category.ShouldExpand());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DefaultCategorySupplierTests, ReturnsStandardCategoryWhenOnlyStandardInCustomAttributeIsSet)
    {
    ECPropertyCP prop = m_class->GetPropertyP("Prop3");
    ContentDescriptor::Category category = m_supplier.GetCategory(*m_class, RelatedClassPath(), *prop);
    EXPECT_EQ(ContentDescriptor::Category::GetCategory(ContentDescriptor::Category::Standard::Material), category);
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DefaultCategorySupplierTests, ReturnsClassBasedCategoryWhenRequestingCategoryForNestedContent)
    {
    ContentDescriptor::Category expected(m_class->GetName(), m_class->GetDisplayLabel(), DefaultCategorySupplier::NESTED_CONTENT_CATEGORY_PRIORITY, false);
    ContentDescriptor::Category actual = m_supplier.GetCategory(*m_class, RelatedClassPath(), *m_class);
    EXPECT_EQ(expected, actual);
    }
