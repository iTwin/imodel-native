/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
#include <ECObjects/SchemaComparer.h>
#include "../TestFixture/TestFixture.h"
#include "BeXml/BeXml.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

struct PropertyConflictTest : ECTestFixture {

    static ECSchemaPtr LoadSchemaFromString(Utf8CP schemaXml)
        {
        ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();

        ECSchemaPtr schema;
        SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXml, *schemaContext);
        if (SchemaReadStatus::Success != status)
            return nullptr;

        return schema;
        }

    static constexpr Utf8CP s_testSchemaXml = R"xml(<?xml version="1.0" encoding="utf-8"?>
    <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0"
        xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECEntityClass typeName="A">
            <ECProperty propertyName="a" typeName="double" />
        </ECEntityClass>
        <ECEntityClass typeName="B">
            <BaseClass>A</BaseClass>
            <ECProperty propertyName="b" typeName="string" />
        </ECEntityClass>
        <ECEntityClass typeName="C">
            <BaseClass>B</BaseClass>
            <ECProperty propertyName="c" typeName="bool" />
        </ECEntityClass>
    </ECSchema>)xml";

    static constexpr Utf8CP s_testSchemaWithStructsXml = R"xml(<?xml version="1.0" encoding="utf-8"?>
    <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0"
        xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECStructClass typeName="StructA" />
        <ECStructClass typeName="StructB" />
        <ECEntityClass typeName="A">
            <ECStructProperty propertyName="structProp" typeName="StructA" />
        </ECEntityClass>
        <ECEntityClass typeName="B">
            <BaseClass>A</BaseClass>
            <ECProperty propertyName="b" typeName="string" />
        </ECEntityClass>
        <ECEntityClass typeName="C">
            <BaseClass>B</BaseClass>
            <ECProperty propertyName="c" typeName="bool" />
        </ECEntityClass>
    </ECSchema>)xml";

    static ECSchemaPtr LoadTestSchema()
        {
        return LoadSchemaFromString(s_testSchemaXml);
        }

    static ECSchemaPtr LoadTestSchemaWithStructs()
        {
        return LoadSchemaFromString(s_testSchemaWithStructsXml);
        }

    void AssertSchemaEquals(ECSchemaPtr schema, Utf8CP expectedSchemaXml)
        {
        SchemaComparer comparer;
        SchemaComparer::Options comparerOptions = SchemaComparer::Options(SchemaComparer::DetailLevel::Full, SchemaComparer::DetailLevel::Full);
        SchemaDiff diff;
        ECSchemaPtr expectedSchema = LoadSchemaFromString(expectedSchemaXml);
        ASSERT_TRUE(expectedSchema.IsValid());

        bvector<ECSchemaCP> actualSchemas   { schema.get() };
        bvector<ECSchemaCP> expectedSchemas { expectedSchema.get() };

        ASSERT_EQ(BentleyStatus::SUCCESS, comparer.Compare(diff, actualSchemas, expectedSchemas, comparerOptions));
        bool isChanged = diff.Changes().IsChanged();
        if (isChanged)
            {
            Utf8String resultSchemaXml;
            schema->WriteToXmlString(resultSchemaXml, ECVersion::Latest);
            printf("Schema does not match expected result:\n%s\n", resultSchemaXml.c_str());
            printf("Expected schema:\n%s\n", expectedSchemaXml);
            FAIL() << "Schemas are not equal";
            }
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PropertyConflictTest, AddNonConflictingProperties)
    {
    ECSchemaPtr schema = LoadTestSchema();
    auto* classA = schema->GetClassP("A");
    auto* classB = schema->GetClassP("B");
    auto* classC = schema->GetClassP("C");
    ASSERT_NE(nullptr, classA);
    ASSERT_NE(nullptr, classB);
    ASSERT_NE(nullptr, classC);

    PrimitiveECPropertyP prop;
    ASSERT_EQ(ECObjectsStatus::Success, classA->CreatePrimitiveProperty(prop, "a2", PRIMITIVETYPE_Double, true));
    ASSERT_NE(nullptr, prop);
    ASSERT_STREQ("a2", prop->GetName().c_str());

    ASSERT_EQ(ECObjectsStatus::Success, classB->CreatePrimitiveProperty(prop, "b2", PRIMITIVETYPE_String, true));
    ASSERT_NE(nullptr, prop);
    ASSERT_STREQ("b2", prop->GetName().c_str());

    ASSERT_EQ(ECObjectsStatus::Success, classC->CreatePrimitiveProperty(prop, "c2", PRIMITIVETYPE_Boolean, true));
    ASSERT_NE(nullptr, prop);
    ASSERT_STREQ("c2", prop->GetName().c_str());

    static constexpr Utf8CP expectedSchemaXml = R"xml(<?xml version="1.0" encoding="utf-8"?>
    <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECEntityClass typeName="A">
            <ECProperty propertyName="a" typeName="double" />
            <ECProperty propertyName="a2" typeName="double" />
        </ECEntityClass>
        <ECEntityClass typeName="B">
            <BaseClass>A</BaseClass>
            <ECProperty propertyName="b" typeName="string" />
            <ECProperty propertyName="b2" typeName="string" />
        </ECEntityClass>
        <ECEntityClass typeName="C">
            <BaseClass>B</BaseClass>
            <ECProperty propertyName="c" typeName="bool" />
            <ECProperty propertyName="c2" typeName="bool" />
        </ECEntityClass>
    </ECSchema>)xml";
    AssertSchemaEquals(schema, expectedSchemaXml);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PropertyConflictTest, AddCompatibleAndIncompatibleOverrides)
    {
    ECSchemaPtr schema = LoadTestSchema();
    auto* classA = schema->GetClassP("A");
    auto* classB = schema->GetClassP("B");
    auto* classC = schema->GetClassP("C");
    ASSERT_NE(nullptr, classA);
    ASSERT_NE(nullptr, classB);
    ASSERT_NE(nullptr, classC);

    PrimitiveECPropertyP prop;
    ASSERT_EQ(ECObjectsStatus::Success, classC->CreatePrimitiveProperty(prop, "a", PRIMITIVETYPE_Double, true));
    ASSERT_NE(nullptr, prop);
    ASSERT_STREQ("a", prop->GetName().c_str());

    ASSERT_EQ(ECObjectsStatus::Success, classC->CreatePrimitiveProperty(prop, "b", PRIMITIVETYPE_Boolean, true));
    ASSERT_NE(nullptr, prop);
    ASSERT_STREQ("ts_b_", prop->GetName().c_str());

    static constexpr Utf8CP expectedSchemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECSchemaReference name="ECv3ConversionAttributes" version="01.00.01" alias="V2ToV3"/>
        <ECEntityClass typeName="A">
            <ECProperty propertyName="a" typeName="double"/>
        </ECEntityClass>
        <ECEntityClass typeName="B">
            <BaseClass>A</BaseClass>
            <ECProperty propertyName="b" typeName="string"/>
        </ECEntityClass>
        <ECEntityClass typeName="C">
            <BaseClass>B</BaseClass>
            <ECCustomAttributes>
                <RenamedPropertiesMapping xmlns="ECv3ConversionAttributes.01.00.01">
                    <PropertyMapping>b|ts_b_</PropertyMapping>
                </RenamedPropertiesMapping>
            </ECCustomAttributes>
            <ECProperty propertyName="c" typeName="boolean"/>
            <ECProperty propertyName="a" typeName="double"/>
            <ECProperty propertyName="ts_b_" typeName="boolean" displayLabel="b"/>
        </ECEntityClass>
    </ECSchema>)xml";
    AssertSchemaEquals(schema, expectedSchemaXml);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PropertyConflictTest, AddIncompatibleOverridesMultipleTimes)
    {
    ECSchemaPtr schema = LoadTestSchema();
    auto* classA = schema->GetClassP("A");
    auto* classB = schema->GetClassP("B");
    auto* classC = schema->GetClassP("C");
    ASSERT_NE(nullptr, classA);
    ASSERT_NE(nullptr, classB);
    ASSERT_NE(nullptr, classC);

    PrimitiveECPropertyP prop;
    ASSERT_EQ(ECObjectsStatus::Success, classB->CreatePrimitiveProperty(prop, "a", PRIMITIVETYPE_Boolean, true));
    ASSERT_NE(nullptr, prop);
    ASSERT_STREQ("ts_a_", prop->GetName().c_str());
    ASSERT_EQ(ECObjectsStatus::Success, classC->CreatePrimitiveProperty(prop, "a", PRIMITIVETYPE_Boolean, true));
    ASSERT_NE(nullptr, prop);
    ASSERT_STREQ("ts_a_", prop->GetName().c_str());
    ASSERT_EQ(ECObjectsStatus::Success, classC->CreatePrimitiveProperty(prop, "a", PRIMITIVETYPE_Boolean, true));
    ASSERT_NE(nullptr, prop);
    ASSERT_STREQ("ts_a_", prop->GetName().c_str());

    static constexpr Utf8CP expectedSchemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECSchemaReference name="ECv3ConversionAttributes" version="01.00.01" alias="V2ToV3"/>
        <ECEntityClass typeName="A">
            <ECProperty propertyName="a" typeName="double"/>
        </ECEntityClass>
        <ECEntityClass typeName="B">
            <BaseClass>A</BaseClass>
            <ECCustomAttributes>
                <RenamedPropertiesMapping xmlns="ECv3ConversionAttributes.01.00.01">
                    <PropertyMapping>a|ts_a_</PropertyMapping>
                </RenamedPropertiesMapping>
            </ECCustomAttributes>
            <ECProperty propertyName="b" typeName="string"/>
            <ECProperty propertyName="ts_a_" typeName="boolean" displayLabel="a"/>
        </ECEntityClass>
        <ECEntityClass typeName="C">
            <BaseClass>B</BaseClass>
            <ECCustomAttributes>
                <RenamedPropertiesMapping xmlns="ECv3ConversionAttributes.01.00.01">
                    <PropertyMapping>a|ts_a_</PropertyMapping>
                </RenamedPropertiesMapping>
            </ECCustomAttributes>
            <ECProperty propertyName="c" typeName="boolean"/>
            <ECProperty propertyName="ts_a_" typeName="boolean" displayLabel="a"/>
        </ECEntityClass>
    </ECSchema>)xml";
    AssertSchemaEquals(schema, expectedSchemaXml);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PropertyConflictTest, AddCompatiblePropertyMultipleTimes)
    {
    ECSchemaPtr schema = LoadTestSchema();
    auto* classA = schema->GetClassP("A");
    auto* classB = schema->GetClassP("B");
    auto* classC = schema->GetClassP("C");
    ASSERT_NE(nullptr, classA);
    ASSERT_NE(nullptr, classB);
    ASSERT_NE(nullptr, classC);

    PrimitiveECPropertyP prop;
    ASSERT_EQ(ECObjectsStatus::Success, classB->CreatePrimitiveProperty(prop, "a", PRIMITIVETYPE_Double, false));
    ASSERT_NE(nullptr, prop);
    ASSERT_STREQ("a", prop->GetName().c_str());
    ASSERT_EQ(ECObjectsStatus::Success, classC->CreatePrimitiveProperty(prop, "a", PRIMITIVETYPE_Double, true));
    ASSERT_NE(nullptr, prop);
    ASSERT_STREQ("a", prop->GetName().c_str());
    ASSERT_EQ(ECObjectsStatus::Success, classC->CreatePrimitiveProperty(prop, "a", PRIMITIVETYPE_Double, true));
    ASSERT_NE(nullptr, prop);
    ASSERT_STREQ("a", prop->GetName().c_str());
    ASSERT_EQ(ECObjectsStatus::Success, classC->CreatePrimitiveProperty(prop, "a", PRIMITIVETYPE_Double, true));
    ASSERT_NE(nullptr, prop);
    ASSERT_STREQ("a", prop->GetName().c_str());

    static constexpr Utf8CP expectedSchemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECEntityClass typeName="A">
            <ECProperty propertyName="a" typeName="double"/>
        </ECEntityClass>
        <ECEntityClass typeName="B">
            <BaseClass>A</BaseClass>
            <ECProperty propertyName="b" typeName="string"/>
            <ECProperty propertyName="a" typeName="double"/>
        </ECEntityClass>
        <ECEntityClass typeName="C">
            <BaseClass>B</BaseClass>
            <ECProperty propertyName="c" typeName="boolean"/>
            <ECProperty propertyName="a" typeName="double"/>
        </ECEntityClass>
    </ECSchema>)xml";
    AssertSchemaEquals(schema, expectedSchemaXml);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PropertyConflictTest, AddBasePropertyWhichIntroducesConflict)
    {
    ECSchemaPtr schema = LoadTestSchema();
    auto* classA = schema->GetClassP("A");
    auto* classB = schema->GetClassP("B");
    auto* classC = schema->GetClassP("C");
    ASSERT_NE(nullptr, classA);
    ASSERT_NE(nullptr, classB);
    ASSERT_NE(nullptr, classC);

    PrimitiveECPropertyP prop;
    ASSERT_EQ(ECObjectsStatus::Success, classB->CreatePrimitiveProperty(prop, "c", PRIMITIVETYPE_Double, true));
    ASSERT_NE(nullptr, prop);
    ASSERT_STREQ("c", prop->GetName().c_str());
    
    static constexpr Utf8CP expectedSchemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECSchemaReference name="ECv3ConversionAttributes" version="01.00.01" alias="V2ToV3"/>
        <ECEntityClass typeName="A">
            <ECProperty propertyName="a" typeName="double"/>
        </ECEntityClass>
        <ECEntityClass typeName="B">
            <BaseClass>A</BaseClass>
            <ECProperty propertyName="b" typeName="string"/>
            <ECProperty propertyName="c" typeName="double"/>
        </ECEntityClass>
        <ECEntityClass typeName="C">
            <BaseClass>B</BaseClass>
            <ECCustomAttributes>
                <RenamedPropertiesMapping xmlns="ECv3ConversionAttributes.01.00.01">
                    <PropertyMapping>c|ts_c_</PropertyMapping>
                </RenamedPropertiesMapping>
            </ECCustomAttributes>
            <ECProperty propertyName="ts_c_" typeName="boolean" displayLabel="c"/>
        </ECEntityClass>
    </ECSchema>)xml";
    AssertSchemaEquals(schema, expectedSchemaXml);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PropertyConflictTest, ResolveConflictsFalse_ReturnsErrorOnConflict)
    {
    ECSchemaPtr schema = LoadTestSchema();
    auto* classB = schema->GetClassP("B");
    ASSERT_NE(nullptr, classB);

    PrimitiveECPropertyP prop;
    // Attempt to create property with same name as base class property, but resolveConflicts=false
    ASSERT_EQ(ECObjectsStatus::DataTypeMismatch, classB->CreatePrimitiveProperty(prop, "a", PRIMITIVETYPE_Boolean, false));
    ASSERT_EQ(nullptr, prop);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PropertyConflictTest, ConflictWithRenamedProperty)
    {
    ECSchemaPtr schema = LoadTestSchema();
    auto* classB = schema->GetClassP("B");
    ASSERT_NE(nullptr, classB);

    PrimitiveECPropertyP prop;
    // First, add a property that will be renamed to ts_a_
    ASSERT_EQ(ECObjectsStatus::Success, classB->CreatePrimitiveProperty(prop, "a", PRIMITIVETYPE_Boolean, true));
    ASSERT_NE(nullptr, prop);
    ASSERT_STREQ("ts_a_", prop->GetName().c_str());

    // Now try to add a property literally named "ts_a_" with compatible type
    ASSERT_EQ(ECObjectsStatus::Success, classB->CreatePrimitiveProperty(prop, "ts_a_", PRIMITIVETYPE_Boolean, true));
    ASSERT_NE(nullptr, prop);
    ASSERT_STREQ("ts_a_", prop->GetName().c_str()); // Should reuse the existing property

    // Now try to add a property literally named "ts_a_" with incompatible type
    ASSERT_EQ(ECObjectsStatus::Success, classB->CreatePrimitiveProperty(prop, "ts_a_", PRIMITIVETYPE_String, true));
    ASSERT_NE(nullptr, prop);
    ASSERT_STREQ("ts_ts_a__", prop->GetName().c_str()); // Should add underscore

    static constexpr Utf8CP expectedSchemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECSchemaReference name="ECv3ConversionAttributes" version="01.00.01" alias="V2ToV3"/>
        <ECEntityClass typeName="A">
            <ECProperty propertyName="a" typeName="double"/>
        </ECEntityClass>
        <ECEntityClass typeName="B">
            <BaseClass>A</BaseClass>
            <ECCustomAttributes>
                <RenamedPropertiesMapping xmlns="ECv3ConversionAttributes.01.00.01">
                    <PropertyMapping>a|ts_ts_a__</PropertyMapping>
                </RenamedPropertiesMapping>
            </ECCustomAttributes>
            <ECProperty propertyName="b" typeName="string"/>
            <ECProperty propertyName="ts_a_" typeName="boolean" displayLabel="a"/>
            <ECProperty propertyName="ts_ts_a__" typeName="string" displayLabel="ts_a_"/>
        </ECEntityClass>
        <ECEntityClass typeName="C">
            <BaseClass>B</BaseClass>
            <ECProperty propertyName="c" typeName="boolean"/>
        </ECEntityClass>
    </ECSchema>)xml";
    AssertSchemaEquals(schema, expectedSchemaXml);
    }

/*---------------------------------------------------------------------------------**//**
* Test that struct properties can have same type as base - compatible override
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PropertyConflictTest, StructProperty_CompatibleOverride)
    {
    ECSchemaPtr schema = LoadTestSchemaWithStructs();
    auto* classB = schema->GetClassP("B");
    auto* structA = schema->GetClassCP("StructA")->GetStructClassCP();
    ASSERT_NE(nullptr, classB);
    ASSERT_NE(nullptr, structA);

    StructECPropertyP prop;
    // Try to add compatible struct property with same type - should succeed
    ASSERT_EQ(ECObjectsStatus::Success, classB->CreateStructProperty(prop, "structProp", *structA));
    ASSERT_NE(nullptr, prop);
    ASSERT_STREQ("structProp", prop->GetName().c_str());

    static constexpr Utf8CP expectedSchemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECStructClass typeName="StructA"/>
        <ECStructClass typeName="StructB"/>
        <ECEntityClass typeName="A">
            <ECStructProperty propertyName="structProp" typeName="StructA"/>
        </ECEntityClass>
        <ECEntityClass typeName="B">
            <BaseClass>A</BaseClass>
            <ECProperty propertyName="b" typeName="string"/>
            <ECStructProperty propertyName="structProp" typeName="StructA"/>
        </ECEntityClass>
        <ECEntityClass typeName="C">
            <BaseClass>B</BaseClass>
            <ECProperty propertyName="c" typeName="boolean"/>
        </ECEntityClass>
    </ECSchema>)xml";
    AssertSchemaEquals(schema, expectedSchemaXml);
    }

/*---------------------------------------------------------------------------------**//**
* Test that struct properties with different types conflict
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PropertyConflictTest, StructProperty_IncompatibleOverride)
    {
    ECSchemaPtr schema = LoadTestSchemaWithStructs();
    auto* classB = schema->GetClassP("B");
    auto* structB = schema->GetClassCP("StructB")->GetStructClassCP();
    ASSERT_NE(nullptr, classB);
    ASSERT_NE(nullptr, structB);

    StructECPropertyP prop;
    // Try to add struct property with different type - should fail (no resolveConflicts available)
    ASSERT_EQ(ECObjectsStatus::DataTypeMismatch, classB->CreateStructProperty(prop, "structProp", *structB));
    }

/*---------------------------------------------------------------------------------**//**
* Test that struct property conflicts with primitive property
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PropertyConflictTest, StructProperty_ConflictWithPrimitiveProperty)
    {
    ECSchemaPtr schema = LoadTestSchemaWithStructs();
    auto* classB = schema->GetClassP("B");
    auto* structA = schema->GetClassCP("StructA")->GetStructClassCP();
    ASSERT_NE(nullptr, classB);
    ASSERT_NE(nullptr, structA);

    StructECPropertyP structProp;
    // Try to add a struct property with same name as existing primitive property - should fail
    ASSERT_EQ(ECObjectsStatus::NamedItemAlreadyExists, classB->CreateStructProperty(structProp, "b", *structA));
    }

/*---------------------------------------------------------------------------------**//**
* Test that array properties can have same type as base - compatible override
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PropertyConflictTest, ArrayProperty_CompatibleOverride)
    {
    ECSchemaPtr schema = LoadTestSchema();
    auto* classB = schema->GetClassP("B");
    ASSERT_NE(nullptr, classB);

    PrimitiveArrayECPropertyP arrayProp;
    // First add an array property to class A
    auto* classA = schema->GetClassP("A");
    ASSERT_EQ(ECObjectsStatus::Success, classA->CreatePrimitiveArrayProperty(arrayProp, "arrayProp", PRIMITIVETYPE_String));
    ASSERT_NE(nullptr, arrayProp);

    // Now try to add compatible array property (same type) in class B - should succeed
    ASSERT_EQ(ECObjectsStatus::Success, classB->CreatePrimitiveArrayProperty(arrayProp, "arrayProp", PRIMITIVETYPE_String));
    ASSERT_NE(nullptr, arrayProp);
    ASSERT_STREQ("arrayProp", arrayProp->GetName().c_str());

    static constexpr Utf8CP expectedSchemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECEntityClass typeName="A">
            <ECProperty propertyName="a" typeName="double"/>
            <ECArrayProperty propertyName="arrayProp" typeName="string" minOccurs="0" maxOccurs="unbounded"/>
        </ECEntityClass>
        <ECEntityClass typeName="B">
            <BaseClass>A</BaseClass>
            <ECProperty propertyName="b" typeName="string"/>
            <ECArrayProperty propertyName="arrayProp" typeName="string" minOccurs="0" maxOccurs="unbounded"/>
        </ECEntityClass>
        <ECEntityClass typeName="C">
            <BaseClass>B</BaseClass>
            <ECProperty propertyName="c" typeName="boolean"/>
        </ECEntityClass>
    </ECSchema>)xml";
    AssertSchemaEquals(schema, expectedSchemaXml);
    }

/*---------------------------------------------------------------------------------**//**
* Test that array properties with different types conflict
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PropertyConflictTest, ArrayProperty_IncompatibleOverride)
    {
    ECSchemaPtr schema = LoadTestSchema();
    auto* classB = schema->GetClassP("B");
    ASSERT_NE(nullptr, classB);

    PrimitiveArrayECPropertyP arrayProp;
    // First add an array property to class A
    auto* classA = schema->GetClassP("A");
    ASSERT_EQ(ECObjectsStatus::Success, classA->CreatePrimitiveArrayProperty(arrayProp, "arrayProp", PRIMITIVETYPE_String));
    ASSERT_NE(nullptr, arrayProp);

    // Now try to add incompatible array property (different type) in class B - should fail (no resolveConflicts)
    ASSERT_EQ(ECObjectsStatus::DataTypeMismatch, classB->CreatePrimitiveArrayProperty(arrayProp, "arrayProp", PRIMITIVETYPE_Integer));
    }

/*---------------------------------------------------------------------------------**//**
* Test that array property conflicts with primitive property
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PropertyConflictTest, ArrayProperty_ConflictWithPrimitiveProperty)
    {
    ECSchemaPtr schema = LoadTestSchema();
    auto* classB = schema->GetClassP("B");
    ASSERT_NE(nullptr, classB);

    PrimitiveArrayECPropertyP arrayProp;
    // Try to add array property with same name as existing primitive property - should fail
    ASSERT_EQ(ECObjectsStatus::NamedItemAlreadyExists, classB->CreatePrimitiveArrayProperty(arrayProp, "b", PRIMITIVETYPE_String));
    }

/*---------------------------------------------------------------------------------**//**
* Test that struct array properties can have same type as base - compatible override
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PropertyConflictTest, StructArrayProperty_CompatibleOverride)
    {
    ECSchemaPtr schema = LoadTestSchemaWithStructs();
    auto* classB = schema->GetClassP("B");
    auto* structA = schema->GetClassCP("StructA")->GetStructClassCP();
    ASSERT_NE(nullptr, classB);
    ASSERT_NE(nullptr, structA);

    StructArrayECPropertyP arrayProp;
    // First add a struct array property to class A
    auto* classA = schema->GetClassP("A");
    ASSERT_EQ(ECObjectsStatus::Success, classA->CreateStructArrayProperty(arrayProp, "structArrayProp", *structA));
    ASSERT_NE(nullptr, arrayProp);

    // Now try to add compatible struct array property (same type) in class B - should succeed
    ASSERT_EQ(ECObjectsStatus::Success, classB->CreateStructArrayProperty(arrayProp, "structArrayProp", *structA));
    ASSERT_NE(nullptr, arrayProp);
    ASSERT_STREQ("structArrayProp", arrayProp->GetName().c_str());

    static constexpr Utf8CP expectedSchemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECStructClass typeName="StructA"/>
        <ECStructClass typeName="StructB"/>
        <ECEntityClass typeName="A">
            <ECStructProperty propertyName="structProp" typeName="StructA"/>
            <ECStructArrayProperty propertyName="structArrayProp" typeName="StructA" minOccurs="0" maxOccurs="unbounded"/>
        </ECEntityClass>
        <ECEntityClass typeName="B">
            <BaseClass>A</BaseClass>
            <ECProperty propertyName="b" typeName="string"/>
            <ECStructArrayProperty propertyName="structArrayProp" typeName="StructA" minOccurs="0" maxOccurs="unbounded"/>
        </ECEntityClass>
        <ECEntityClass typeName="C">
            <BaseClass>B</BaseClass>
            <ECProperty propertyName="c" typeName="boolean"/>
        </ECEntityClass>
    </ECSchema>)xml";
    AssertSchemaEquals(schema, expectedSchemaXml);
    }

/*---------------------------------------------------------------------------------**//**
* Test that struct array properties with different types conflict
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PropertyConflictTest, StructArrayProperty_IncompatibleOverride)
    {
    ECSchemaPtr schema = LoadTestSchemaWithStructs();
    auto* classB = schema->GetClassP("B");
    auto* structA = schema->GetClassCP("StructA")->GetStructClassCP();
    auto* structB = schema->GetClassCP("StructB")->GetStructClassCP();
    ASSERT_NE(nullptr, classB);
    ASSERT_NE(nullptr, structA);
    ASSERT_NE(nullptr, structB);

    StructArrayECPropertyP arrayProp;
    // First add a struct array property to class A
    auto* classA = schema->GetClassP("A");
    ASSERT_EQ(ECObjectsStatus::Success, classA->CreateStructArrayProperty(arrayProp, "structArrayProp", *structA));
    ASSERT_NE(nullptr, arrayProp);

    // Now try to add incompatible struct array property (different struct type) in class B - should fail
    ASSERT_EQ(ECObjectsStatus::DataTypeMismatch, classB->CreateStructArrayProperty(arrayProp, "structArrayProp", *structB));
    }

/*---------------------------------------------------------------------------------**//**
* Test that struct array property conflicts with struct property
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PropertyConflictTest, StructArrayProperty_ConflictWithStructProperty)
    {
    ECSchemaPtr schema = LoadTestSchemaWithStructs();
    auto* classB = schema->GetClassP("B");
    auto* structA = schema->GetClassCP("StructA")->GetStructClassCP();
    ASSERT_NE(nullptr, classB);
    ASSERT_NE(nullptr, structA);

    StructArrayECPropertyP arrayProp;
    // Try to add struct array property with same name as existing struct property - should fail
    ASSERT_EQ(ECObjectsStatus::InvalidPrimitiveOverrride, classB->CreateStructArrayProperty(arrayProp, "structProp", *structA));
    }

END_BENTLEY_ECN_TEST_NAMESPACE
