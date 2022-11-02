/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentation/Content.h>
#include "../../Helpers/TestHelpers.h"

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
            <ECProperty propertyName="Prop5" typeName="double" />
            <ECProperty propertyName="Prop6" typeName="point2d" />
            <ECProperty propertyName="Prop7" typeName="point3d" />
            <ECProperty propertyName="Prop7" typeName="DateTime" />
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
    </ECSchema>
)xml"

/*=================================================================================**//**
* @bsiclass
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

        BeFileName assetsPath;
        BeTest::GetHost().GetDgnPlatformAssetsDirectory(assetsPath);
        ECSchemaReadContext::Initialize(assetsPath);

        m_schemaContext = ECSchemaReadContext::CreateContext();
        ECSchema::ReadFromXmlString(m_schema, TEST_SCHEMA, *m_schemaContext);
        m_class = m_schema->GetClassCP("TestClass");
        }
    };

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DefaultPropertyFormatterTests, GetFormattedPropertyLabelReturnsPropertyLabel)
    {
    ECPropertyCP prop = m_class->GetPropertyP("Prop1");
    Utf8String formattedLabel;
    EXPECT_EQ(SUCCESS, m_formatter.GetFormattedPropertyLabel(formattedLabel, *prop, *m_class, RelatedClassPath(), RelationshipMeaning::SameInstance));
    EXPECT_STREQ("Custom Label", formattedLabel.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DefaultPropertyFormatterTests, GetFormattedPropertyValueHandlesEnumIntProperties)
    {
    ECPropertyCP prop = m_class->GetPropertyP("Prop2");
    ECValue value(1);
    Utf8String formattedValue;
    EXPECT_EQ(SUCCESS, m_formatter.GetFormattedPropertyValue(formattedValue, *prop, value, ECPresentation::UnitSystem::Undefined));
    EXPECT_STREQ("One", formattedValue.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DefaultPropertyFormatterTests, GetFormattedPropertyValueHandlesEnumStringProperties)
    {
    ECPropertyCP prop = m_class->GetPropertyP("Prop3");
    ECValue value("one");
    Utf8String formattedValue;
    EXPECT_EQ(SUCCESS, m_formatter.GetFormattedPropertyValue(formattedValue, *prop, value, ECPresentation::UnitSystem::Undefined));
    EXPECT_STREQ("One", formattedValue.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DefaultPropertyFormatterTests, GetFormattedPropertyValueHandlesPropertiesWithUnits_MetricUnitSystem)
    {
    ECPropertyCP prop = m_class->GetPropertyP("Prop4");
    NamedFormat namedFormat = prop->GetKindOfQuantity()->GetPresentationFormats()[1];
    Utf8Char decimalSeparator = namedFormat.GetNumericSpec()->GetDecimalSeparator();
    ECValue value(123.0);
    Utf8String formattedValue;
    EXPECT_EQ(SUCCESS, m_formatter.GetFormattedPropertyValue(formattedValue, *prop, value, ECPresentation::UnitSystem::Metric));
    EXPECT_STREQ(Utf8PrintfString("123%c0 m", decimalSeparator).c_str(), formattedValue.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DefaultPropertyFormatterTests, GetFormattedPropertyValueHandlesPropertiesWithUnits_BritishImperialUnitSystem)
    {
    ECPropertyCP prop = m_class->GetPropertyP("Prop4");
    NamedFormatCP namedFormat = prop->GetKindOfQuantity()->GetDefaultPresentationFormat();
    Utf8Char decimalSeparator = namedFormat->GetNumericSpec()->GetDecimalSeparator();
    ECValue value(123.0); // persistence value in meters; 123 m = 403.5433 ft
    Utf8String formattedValue;
    EXPECT_EQ(SUCCESS, m_formatter.GetFormattedPropertyValue(formattedValue, *prop, value, ECPresentation::UnitSystem::BritishImperial));
    EXPECT_STREQ(Utf8PrintfString("403%c5433 ft", decimalSeparator).c_str(), formattedValue.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DefaultPropertyFormatterTests, GetFormattedPropertyValueHandlesNullValues)
    {
    ECPropertyCP prop = m_class->GetPropertyP("Prop1");
    ECValue value;
    value.SetIsNull(true);
    Utf8String formattedValue;
    EXPECT_EQ(SUCCESS, m_formatter.GetFormattedPropertyValue(formattedValue, *prop, value, ECPresentation::UnitSystem::Undefined));
    EXPECT_STREQ("", formattedValue.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DefaultPropertyFormatterTests, GetFormattedPropertyValueHandlesDoubleValues)
    {
    ECPropertyCP prop = m_class->GetPropertyP("Prop5");
    ECValue value(2.0);
    Utf8String formattedValue;
    EXPECT_EQ(SUCCESS, m_formatter.GetFormattedPropertyValue(formattedValue, *prop, value, ECPresentation::UnitSystem::Undefined));
    EXPECT_STREQ("2.00", formattedValue.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DefaultPropertyFormatterTests, GetFormattedPropertyValueHandlesPoint2dValues)
    {
    ECPropertyCP prop = m_class->GetPropertyP("Prop6");
    ECValue value(DPoint2d::From(1, 2));
    Utf8String formattedValue;
    EXPECT_EQ(SUCCESS, m_formatter.GetFormattedPropertyValue(formattedValue, *prop, value, ECPresentation::UnitSystem::Undefined));
    EXPECT_STREQ("X: 1.00 Y: 2.00", formattedValue.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DefaultPropertyFormatterTests, GetFormattedPropertyValueHandlesPoint3dValues)
    {
    ECPropertyCP prop = m_class->GetPropertyP("Prop7");
    ECValue value(DPoint3d::From(1, 2, 3));
    Utf8String formattedValue;
    EXPECT_EQ(SUCCESS, m_formatter.GetFormattedPropertyValue(formattedValue, *prop, value, ECPresentation::UnitSystem::Undefined));
    EXPECT_STREQ("X: 1.00 Y: 2.00 Z: 3.00", formattedValue.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DefaultPropertyFormatterTests, GetFormattedPropertyValueHandlesDateTimeValues)
    {
    ECPropertyCP prop = m_class->GetPropertyP("Prop7");
    ECValue value(DateTime(2019, 10, 03));
    Utf8String formattedValue;
    EXPECT_EQ(SUCCESS, m_formatter.GetFormattedPropertyValue(formattedValue, *prop, value, ECPresentation::UnitSystem::Undefined));
    EXPECT_STREQ("2019-10-03", formattedValue.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DefaultPropertyFormatterTests, DefaultPropertyFormatterHandlesDefaultMap)
    {
    ECPropertyCP prop = m_class->GetPropertyP("Prop4");
    auto format = std::make_shared<Formatting::Format>(prop->GetKindOfQuantity()->GetPresentationFormats()[1]);
    Utf8Char decimalSeparator = format->GetNumericSpec()->GetDecimalSeparator();
    ECValue value(123.0);
    std::map<std::pair<Utf8String, ECPresentation::UnitSystem>, std::shared_ptr<Formatting::Format>> map;
    map.insert(std::make_pair(std::make_pair("LENGTH", ECPresentation::UnitSystem::Undefined), format));

    Utf8String formattedValue;

    EXPECT_EQ(SUCCESS, m_formatter.GetFormattedPropertyValue(formattedValue, *prop, value, ECPresentation::UnitSystem::Undefined));
    EXPECT_STREQ(Utf8PrintfString("403%c5433 ft", decimalSeparator).c_str(), formattedValue.c_str());

    m_formatter.SetDefaultFormats(map);

    EXPECT_EQ(SUCCESS, m_formatter.GetFormattedPropertyValue(formattedValue, *prop, value, ECPresentation::UnitSystem::Undefined));
    EXPECT_STREQ(Utf8PrintfString("123%c0 m", decimalSeparator).c_str(), formattedValue.c_str());
    }
