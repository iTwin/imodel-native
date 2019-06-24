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
                        <ECClass typeName="TestClass" isDomainClass="True">
                            <ECProperty propertyName="Prop1" typeName="string" displayLabel="Custom Label" />
                            <ECProperty propertyName="Prop2" typeName="TestIntEnum" />
                            <ECProperty propertyName="Prop3" typeName="TestStringEnum" />
                            <ECProperty propertyName="Prop4" typeName="double" kindOfQuantity="Length" />
                        </ECClass>
                        <ECEnumeration typeName="TestIntEnum" backingTypeName="int" isStrict="true">
                            <ECEnumerator value="0" displayLabel="Zero"/>
                            <ECEnumerator value="1" displayLabel="One"/>
                            <ECEnumerator value="2" displayLabel="Two"/>
                        </ECEnumeration>
                        <ECEnumeration typeName="TestStringEnum" backingTypeName="string" isStrict="true">
                            <ECEnumerator value="zero" displayLabel="Zero"/>
                            <ECEnumerator value="one" displayLabel="One"/>
                            <ECEnumerator value="two" displayLabel="Two"/>
                        </ECEnumeration>
                        <KindOfQuantity typeName="Length" displayLabel="Length" persistenceUnit="M" relativeError="1e-6"
                            presentationUnits="FT(real4u);M(real4u);FT(fi8);IN(real)" />
                    </ECSchema>)xml"

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                10/2016
+===============+===============+===============+===============+===============+======*/
struct DefaultPropertyFormatterTests : ECPresentationTest
    {
    DefaultPropertyFormatter m_formatter;
    ECSchemaReadContextPtr m_schemaContext;
    ECSchemaPtr m_schema;
    ECClassCP m_class;

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
        }

    void TearDown() override
        {
        Localization::Terminate();
        }
    };

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DefaultPropertyFormatterTests, GetFormattedPropertyLabelReturnsPropertyLabel)
    {
    ECPropertyCP prop = m_class->GetPropertyP("Prop1");
    Utf8String formattedLabel;
    EXPECT_EQ(SUCCESS, m_formatter.GetFormattedPropertyLabel(formattedLabel, *prop, *m_class, RelatedClassPath(), RelationshipMeaning::SameInstance));
    EXPECT_STREQ("Custom Label", formattedLabel.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DefaultPropertyFormatterTests, GetFormattedPropertyValueHandlesEnumIntProperties)
    {
    ECPropertyCP prop = m_class->GetPropertyP("Prop2");
    ECValue value(1);
    Utf8String formattedValue;
    EXPECT_EQ(SUCCESS, m_formatter.GetFormattedPropertyValue(formattedValue, *prop, value));
    EXPECT_STREQ("One", formattedValue.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DefaultPropertyFormatterTests, GetFormattedPropertyValueHandlesEnumStringProperties)
    {
    ECPropertyCP prop = m_class->GetPropertyP("Prop3");
    ECValue value("one");
    Utf8String formattedValue;
    EXPECT_EQ(SUCCESS, m_formatter.GetFormattedPropertyValue(formattedValue, *prop, value));
    EXPECT_STREQ("One", formattedValue.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DefaultPropertyFormatterTests, GetFormattedPropertyValueHandlesPropertiesWithUnits)
    {
    ECPropertyCP prop = m_class->GetPropertyP("Prop4");
    NamedFormatCP namedFormat = prop->GetKindOfQuantity()->GetDefaultPresentationFormat();
    Utf8Char decimalSeparator = namedFormat->GetNumericSpec()->GetDecimalSeparator();
    ECValue value(123.0); // persistence value in meters; 123 m = 403.5433 ft
    Utf8String formattedValue;
    EXPECT_EQ(SUCCESS, m_formatter.GetFormattedPropertyValue(formattedValue, *prop, value));
    EXPECT_STREQ(Utf8PrintfString("403%c5433 ft", decimalSeparator).c_str(), formattedValue.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DefaultPropertyFormatterTests, GetFormattedPropertyValueHandlesNullValues)
    {
    ECPropertyCP prop = m_class->GetPropertyP("Prop1");
    ECValue value;
    value.SetIsNull(true);
    Utf8String formattedValue;
    EXPECT_EQ(SUCCESS, m_formatter.GetFormattedPropertyValue(formattedValue, *prop, value));
    EXPECT_STREQ("", formattedValue.c_str());
    }