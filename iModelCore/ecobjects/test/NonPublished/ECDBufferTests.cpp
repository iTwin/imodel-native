/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"

#include <array>

using namespace BentleyApi::ECN;

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsistruct
+---------------+---------------+---------------+---------------+---------------+------*/
struct ECDBufferTests : ECTestFixture
    {
    template<typename T> void TestIsEmpty(IECInstanceR instance, Utf8CP accessor, T const& value)
        {
        ECDBuffer* buf = instance.GetECDBufferP();
        EXPECT_TRUE(buf->IsEmpty());

        ECValue v(value);
        EXPECT_EQ(ECObjectsStatus::Success, instance.SetValue(accessor, v));

        EXPECT_FALSE(buf->IsEmpty());

        v.Clear();
        EXPECT_EQ(ECObjectsStatus::Success, instance.SetValue(accessor, v));

        bool isNull = false;
        EXPECT_EQ(ECObjectsStatus::Success, instance.IsPropertyNull(isNull, accessor));
        EXPECT_TRUE(isNull);
        EXPECT_TRUE(buf->IsEmpty()) << accessor;

        buf->ClearValues();
        EXPECT_TRUE(buf->IsEmpty());
        }

    template<typename T> void TestIsEmptyArray(IECInstanceR instance, Utf8CP accessor, T const& value)
        {
        ECDBuffer& buf = *instance.GetECDBufferP();
        EXPECT_TRUE(buf.IsEmpty());

        EXPECT_EQ(ECObjectsStatus::Success, instance.AddArrayElements(accessor, 1));
        bool isNull = false;
        EXPECT_EQ(ECObjectsStatus::Success, instance.IsPropertyNull(isNull, accessor, 0));
        EXPECT_TRUE(isNull);
        EXPECT_FALSE(buf.IsEmpty());   // a non-empty array containing null elements => a non-empty IECInstance

        ECValue v(value);
        EXPECT_EQ(ECObjectsStatus::Success, instance.SetValue(accessor, v, 0));
        EXPECT_EQ(ECObjectsStatus::Success, instance.IsPropertyNull(isNull, accessor, 0));
        EXPECT_FALSE(isNull);
        EXPECT_FALSE(buf.IsEmpty());

        // Clearing out the array will not reset the null flag for the array property. But an empty array => empty IECInstance
        EXPECT_EQ(ECObjectsStatus::Success, instance.ClearArray(accessor));
        EXPECT_EQ(ECObjectsStatus::Success, instance.GetValue(v, accessor));
        EXPECT_EQ(0, v.GetArrayInfo().GetCount());
        EXPECT_TRUE(buf.IsEmpty()) << accessor;

        buf.ClearValues();
        EXPECT_TRUE(buf.IsEmpty());
        }

    struct ExpectedValue
        {
        ECValue         m_value;
        bool            m_expectExists;

        template<typename T> ExpectedValue(T const& val) : m_value(val), m_expectExists(true) {}
        ExpectedValue() : m_expectExists(false) {}
        };

    void TestValue(IECInstanceCR instance, Utf8CP accessor, ExpectedValue const& val, uint32_t arrayIndex = -1)
        {
        ECValue v;
        ECObjectsStatus status = -1 != arrayIndex ? instance.GetValue(v, accessor, arrayIndex) : instance.GetValue(v, accessor);
        EXPECT_EQ((ECObjectsStatus::Success == status), val.m_expectExists) << " for property " << accessor;

        if (ECObjectsStatus::Success == status)
            {
            if (val.m_expectExists)
                EXPECT_TRUE(val.m_value.Equals(v)) << "Expected: " << val.m_value.ToString().c_str() << " Actual: " << v.ToString().c_str() << " for property " << accessor;
            else
                printf("Expected: non-existent Actual: %s for property %s\n", v.ToString().c_str(), accessor);
            }
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String    GetTestSchemaXMLString(Utf8CP schemaName, uint32_t versionRead, uint32_t versionMinor, Utf8CP className)
    {
    Utf8Char fmt[] = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<ECSchema schemaName=\"%s\" nameSpacePrefix=\"test\" version=\"%02d.%02d\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
        "    <ECClass typeName=\"EmptyClass\" isDomainClass=\"True\">"
        "    </ECClass>"
        "    <ECClass typeName=\"Manufacturer\" isStruct=\"True\" isDomainClass=\"True\">"
        "        <ECProperty propertyName=\"Name\" typeName=\"string\" />"
        "        <ECProperty propertyName=\"AccountNo\" typeName=\"int\" />"
        "    </ECClass>"
        "    <ECClass typeName=\"CadData\" isStruct=\"True\" isDomainClass=\"True\">"
        "        <ECProperty propertyName=\"Name\"         typeName=\"string\" />"
        "        <ECProperty propertyName=\"Count\"        typeName=\"int\" />"
        "        <ECProperty propertyName=\"StartPoint\"   typeName=\"point3d\" />"
        "        <ECProperty propertyName=\"EndPoint\"     typeName=\"point3d\" />"
        "        <ECProperty propertyName=\"Size\"         typeName=\"point2d\" />"
        "        <ECProperty propertyName=\"Length\"       typeName=\"double\"  />"
        "        <ECProperty propertyName=\"Install_Date\" typeName=\"dateTime\"  />"
        "        <ECProperty propertyName=\"Service_Date\" typeName=\"dateTime\"  />"
        "        <ECProperty propertyName=\"Field_Tested\" typeName=\"boolean\"  />"
        "    </ECClass>"
        "    <ECClass typeName=\"AllPrimitives\" isStruct=\"True\" isDomainClass=\"True\">"
        "        <ECProperty propertyName=\"AString\"          typeName=\"string\" />"
        "        <ECProperty propertyName=\"AnInt\"            typeName=\"int\" />"
        "        <ECProperty propertyName=\"APoint3d\"         typeName=\"point3d\" />"
        "        <ECProperty propertyName=\"APoint2d\"         typeName=\"point2d\" />"
        "        <ECProperty propertyName=\"ADouble\"          typeName=\"double\"  />"
        "        <ECProperty propertyName=\"ADateTime\"        typeName=\"dateTime\"  />"
        "        <ECProperty propertyName=\"ABoolean\"         typeName=\"boolean\"  />"
        "        <ECProperty propertyName=\"ALong\"            typeName=\"long\"  />"
        "        <ECProperty propertyName=\"ABinary\"          typeName=\"binary\"  />"
        "        <ECProperty propertyName=\"ReadOnlyInt\"      typeName=\"int\" readOnly=\"True\"  />"
        "        <ECArrayProperty propertyName=\"SomeStrings\" typeName=\"string\" />"
        "        <ECArrayProperty propertyName=\"SomeInts\"    typeName=\"int\" />"
        "        <ECArrayProperty propertyName=\"SomePoint3ds\"    typeName=\"point3d\" />"
        "        <ECArrayProperty propertyName=\"SomePoint2ds\"    typeName=\"point2d\" />"
        "        <ECArrayProperty propertyName=\"SomeDoubles\"     typeName=\"double\"  />"
        "        <ECArrayProperty propertyName=\"SomeDateTimes\"   typeName=\"dateTime\"  />"
        "        <ECArrayProperty propertyName=\"SomeBooleans\"    typeName=\"boolean\"  />"
        "        <ECArrayProperty propertyName=\"SomeLongs\"       typeName=\"long\"  />"
        "        <ECArrayProperty propertyName=\"SomeBinaries\"    typeName=\"binary\"  />"
        "    </ECClass>"
        "    <ECClass typeName=\"FixedSizeArrayTester\" isStruct=\"True\" isDomainClass=\"True\">"
        "        <ECArrayProperty propertyName=\"FixedString1\"  typeName=\"string\"     minOccurs=\"1\"  maxOccurs=\"1\" />"
        "        <ECArrayProperty propertyName=\"FixedInt1\"     typeName=\"int\"        minOccurs=\"1\"  maxOccurs=\"1\" />"
        "        <ECArrayProperty propertyName=\"FixedString10\" typeName=\"string\"     minOccurs=\"10\" maxOccurs=\"10\" />"
        "        <ECArrayProperty propertyName=\"FixedInt10\"    typeName=\"int\"        minOccurs=\"10\" maxOccurs=\"10\" />"
        "        <ECArrayProperty propertyName=\"Struct1\"       typeName=\"BaseClass0\" minOccurs=\"1\"  maxOccurs=\"1\" />"
        "        <ECArrayProperty propertyName=\"Struct10\"      typeName=\"BaseClass0\" minOccurs=\"10\" maxOccurs=\"10\" />"
        "    </ECClass>"
        "    <ECClass typeName=\"ClassLayoutPerformanceTest0\" isStruct=\"True\" isDomainClass=\"True\">"
        "        <ECProperty propertyName=\"AString\"  typeName=\"string\" />"
        "        <ECProperty propertyName=\"AnInt\"    typeName=\"int\" />"
        "        <ECProperty propertyName=\"ADouble\"  typeName=\"double\"  />"
        "    </ECClass>"
        "    <ECClass typeName=\"ClassLayoutPerformanceTest1\" isStruct=\"True\" isDomainClass=\"True\">"
        "        <ECProperty propertyName=\"AMonkeywrench\"    typeName=\"int\" />"
        "        <ECProperty propertyName=\"ADouble\"          typeName=\"double\"  />"
        "        <ECProperty propertyName=\"AString\"          typeName=\"string\" />"
        "        <ECProperty propertyName=\"AnInt\"            typeName=\"int\" />"
        "    </ECClass>"
        "    <ECClass typeName=\"%s\" isDomainClass=\"True\">"
        "        <ECArrayProperty propertyName=\"BeginningArray\" typeName=\"string\" />"
        "        <ECProperty propertyName=\"A\" typeName=\"int\" />"
        "        <ECProperty propertyName=\"AA\" typeName=\"int\" />"
        "        <ECProperty propertyName=\"B\" typeName=\"string\" />"
        "        <ECProperty propertyName=\"C\" typeName=\"long\" />"
        "        <ECProperty propertyName=\"D\" typeName=\"double\" />"
        "        <ECProperty propertyName=\"S\" typeName=\"string\" />"
        "        <ECStructProperty propertyName=\"Manufacturer\" typeName=\"Manufacturer\" />"
        "        <ECProperty propertyName=\"Property0\" typeName=\"string\" />"
        "        <ECProperty propertyName=\"Property1\" typeName=\"string\" />"
        "        <ECProperty propertyName=\"Property2\" typeName=\"string\" />"
        "        <ECProperty propertyName=\"Property3\" typeName=\"string\" />"
        "        <ECProperty propertyName=\"Property4\" typeName=\"string\" />"
        "        <ECProperty propertyName=\"Property5\" typeName=\"string\" />"
        "        <ECProperty propertyName=\"Property6\" typeName=\"string\" />"
        "        <ECProperty propertyName=\"Property7\" typeName=\"string\" />"
        "        <ECProperty propertyName=\"Property8\" typeName=\"string\" />"
        "        <ECProperty propertyName=\"Property9\" typeName=\"string\" />"
        "        <ECArrayProperty propertyName=\"FixedArrayFixedElement\" typeName=\"int\" minOccurs=\"10\" maxOccurs=\"10\"/>"
        "        <ECProperty propertyName=\"Property10\" typeName=\"string\" />"
        "        <ECProperty propertyName=\"Property11\" typeName=\"string\" />"
        "        <ECProperty propertyName=\"Property12\" typeName=\"string\" />"
        "        <ECProperty propertyName=\"Property13\" typeName=\"string\" />"
        "        <ECProperty propertyName=\"Property14\" typeName=\"string\" />"
        "        <ECProperty propertyName=\"Property15\" typeName=\"string\" />"
        "        <ECProperty propertyName=\"Property16\" typeName=\"string\" />"
        "        <ECProperty propertyName=\"Property17\" typeName=\"string\" />"
        "        <ECArrayProperty propertyName=\"VariableArrayFixedElement\" typeName=\"int\"/>"
        "        <ECArrayProperty propertyName=\"FixedArrayVariableElement\" typeName=\"string\" minOccurs=\"12\" maxOccurs=\"12\"/>"
        "        <ECProperty propertyName=\"Property18\" typeName=\"string\" />"
        "        <ECProperty propertyName=\"Property19\" typeName=\"string\" />"
        "        <ECProperty propertyName=\"Property20\" typeName=\"string\" />"
        "        <ECProperty propertyName=\"Property21\" typeName=\"string\" />"
        "        <ECProperty propertyName=\"Property22\" typeName=\"string\" />"
        "        <ECArrayProperty propertyName=\"ManufacturerArray\" typeName=\"Manufacturer\"/>"
        "        <ECProperty propertyName=\"Property23\" typeName=\"string\" />"
        "        <ECProperty propertyName=\"Property24\" typeName=\"string\" />"
        "        <ECProperty propertyName=\"Property25\" typeName=\"string\" />"
        "        <ECProperty propertyName=\"Property26\" typeName=\"string\" />"
        "        <ECProperty propertyName=\"Property27\" typeName=\"string\" />"
        "        <ECArrayProperty propertyName=\"VariableArrayVariableElement\" typeName=\"string\"/>"
        "        <ECProperty propertyName=\"Property28\" typeName=\"string\" />"
        "        <ECProperty propertyName=\"Property29\" typeName=\"string\" />"
        "        <ECProperty propertyName=\"Property30\" typeName=\"string\" />"
        "        <ECProperty propertyName=\"Property31\" typeName=\"string\" />"
        "        <ECProperty propertyName=\"Property32\" typeName=\"string\" />"
        "        <ECProperty propertyName=\"Property33\" typeName=\"string\" />"
        "        <ECProperty propertyName=\"Property34\" typeName=\"string\" />"
        "        <ECProperty propertyName=\"Property35\" typeName=\"string\" />"
        "        <ECProperty propertyName=\"Property36\" typeName=\"string\" />"
        "        <ECProperty propertyName=\"Property37\" typeName=\"string\" />"
        "        <ECProperty propertyName=\"Property38\" typeName=\"string\" />"
        "        <ECProperty propertyName=\"Property39\" typeName=\"string\" />"
        "        <ECProperty propertyName=\"Property40\" typeName=\"string\" />"
        "        <ECProperty propertyName=\"Property41\" typeName=\"string\" />"
        "        <ECProperty propertyName=\"Property42\" typeName=\"string\" />"
        "        <ECProperty propertyName=\"Property43\" typeName=\"string\" />"
        "        <ECProperty propertyName=\"Property44\" typeName=\"string\" />"
        "        <ECProperty propertyName=\"Property45\" typeName=\"string\" />"
        "        <ECProperty propertyName=\"Property46\" typeName=\"string\" />"
        "        <ECProperty propertyName=\"Property47\" typeName=\"string\" />"
        "        <ECArrayProperty propertyName=\"EndingArray\" typeName=\"string\" />"
        "    </ECClass>"
        "    <ECClass typeName=\"NestedStructArray\" isStruct=\"True\" isDomainClass=\"True\">"
        "        <ECProperty propertyName=\"NestPropString\" typeName=\"string\" />"
        "        <ECArrayProperty propertyName=\"ManufacturerArray\" typeName=\"Manufacturer\"  minOccurs=\"0\" maxOccurs=\"unbounded\" />"
        "    </ECClass>"
        "    <ECClass typeName=\"ClassWithStructArray\" isStruct=\"True\" isDomainClass=\"True\">"
        "        <ECArrayProperty propertyName=\"StructArray\" typeName=\"AllPrimitives\"  minOccurs=\"0\" maxOccurs=\"unbounded\" />"
        "        <ECStructProperty propertyName=\"StructMember\" typeName=\"AllPrimitives\" />"
        "        <ECArrayProperty propertyName=\"ComplicatedStructArray\" typeName=\"NestedStructArray\"  minOccurs=\"0\" maxOccurs=\"unbounded\" />"
        "    </ECClass>"
        "    <ECClass typeName=\"ClassWithPolymorphicStructArray\" isStruct=\"True\" isDomainClass=\"True\">"
        "        <ECArrayProperty propertyName=\"PolymorphicStructArray\" typeName=\"BaseClass0\"  minOccurs=\"0\" maxOccurs=\"unbounded\" />"
        "    </ECClass>"
        "    <ECClass typeName=\"BaseClass0\" isStruct=\"True\" isDomainClass=\"True\">"
        "        <ECProperty propertyName=\"BaseIntProperty\" typeName=\"int\" />"
        "    </ECClass>"
        "    <ECClass typeName=\"DerivedClass0\" isStruct=\"True\" isDomainClass=\"True\">"
        "        <BaseClass>BaseClass0</BaseClass>"
        "        <ECProperty propertyName=\"DerivedStringProperty\" typeName=\"string\" />"
        "    </ECClass>"
        "    <ECClass typeName=\"DerivedClass1\" isStruct=\"True\" isDomainClass=\"True\">"
        "        <BaseClass>BaseClass0</BaseClass>"
        "        <ECProperty propertyName=\"DerivedDoubleProperty\" typeName=\"double\" />"
        "    </ECClass>"
        "</ECSchema>";

    Utf8String buff;
    buff.Sprintf(fmt, schemaName, versionRead, versionMinor, className);

    return buff;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaPtr CreateTestSchema()
    {
    Utf8String schemaXMLString = GetTestSchemaXMLString("TestSchema", 0, 0, "TestClass");

    ECSchemaReadContextPtr  schemaContext = ECSchemaReadContext::CreateContext();

    ECSchemaPtr schema;
    EXPECT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXMLString.c_str(), *schemaContext));

    return schema;
    }

/*---------------------------------------------------------------------------------**//**
* Test the ECDBuffer::IsEmpty() method. Should return true if all values are null and
* all arrays are empty.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDBufferTests, IsEmpty)
    {
    ECSchemaPtr schema = CreateTestSchema();

    IECInstancePtr instance = schema->GetClassP("Manufacturer")->GetDefaultStandaloneEnabler()->CreateInstance();
    TestIsEmpty(*instance, "AccountNo", 12345);   // fixed-sized property
    TestIsEmpty(*instance, "Name", "Ed");        // variable-sized property

    instance = schema->GetClassP("AllPrimitives")->GetDefaultStandaloneEnabler()->CreateInstance();
    TestIsEmptyArray(*instance, "SomeInts", 54321);
    TestIsEmptyArray(*instance, "SomeStrings", "abcdefg");

    instance = schema->GetClassP("ClassWithStructArray")->GetDefaultStandaloneEnabler()->CreateInstance();
    TestIsEmpty(*instance, "StructMember.AnInt", 12345);
    TestIsEmpty(*instance, "StructMember.AString", "bbbbb");
    TestIsEmptyArray(*instance, "StructMember.SomeInts", 54321);
    TestIsEmptyArray(*instance, "StructMember.SomeStrings", "lalalalala");
    }

/*---------------------------------------------------------------------------------**//**
* Simplification of above test to isolate some memory corruption when clearing the array.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDBufferTests, ClearArray)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "ArrayTest", "ts", 1, 0, 0);
    ECEntityClassP ecClass;
    schema->CreateEntityClass(ecClass, "TestClass");
    PrimitiveECPropertyP primProp;
    ecClass->CreatePrimitiveProperty(primProp, "Int", PRIMITIVETYPE_Integer);
    ecClass->CreatePrimitiveProperty(primProp, "String", PRIMITIVETYPE_String);

    PrimitiveArrayECPropertyP arrayProp;
    ecClass->CreatePrimitiveArrayProperty(arrayProp, "Ints", PRIMITIVETYPE_Integer);
    ecClass->CreatePrimitiveArrayProperty(arrayProp, "Strings", PRIMITIVETYPE_String);
    ecClass->CreatePrimitiveArrayProperty(arrayProp, "MoreInts", PRIMITIVETYPE_Integer);
    IECInstancePtr instance = ecClass->GetDefaultStandaloneEnabler()->CreateInstance();

    ECValue v;
    EXPECT_EQ(ECObjectsStatus::Success, instance->GetValue(v, "Ints"));
    EXPECT_EQ(0, v.GetArrayInfo().GetCount());
    EXPECT_EQ(ECObjectsStatus::Success, instance->GetValue(v, "Strings"));
    EXPECT_EQ(0, v.GetArrayInfo().GetCount());

    EXPECT_EQ(ECObjectsStatus::Success, instance->AddArrayElements("Ints", 1));
    EXPECT_EQ(ECObjectsStatus::Success, instance->GetValue(v, "Ints"));
    EXPECT_EQ(1, v.GetArrayInfo().GetCount());
    EXPECT_EQ(ECObjectsStatus::Success, instance->GetValue(v, "Strings"));
    EXPECT_EQ(0, v.GetArrayInfo().GetCount());

    // The problem was here:
    // After ClearArray() we fix up secondary offsets of other variable-sized properties by subtracting the number of bytes removed from the buffer
    // The Strings array had a secondary offset of zero; subtraction produced a negative offset, interpreted as positive offset into memory outside the buffer
    EXPECT_EQ(ECObjectsStatus::Success, instance->ClearArray("Ints"));
    EXPECT_EQ(ECObjectsStatus::Success, instance->GetValue(v, "Ints"));
    EXPECT_EQ(0, v.GetArrayInfo().GetCount());
    EXPECT_EQ(ECObjectsStatus::Success, instance->GetValue(v, "Strings"));
    EXPECT_EQ(0, v.GetArrayInfo().GetCount());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDBufferTests, MoreClearArrayTests)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "Test", "ts", 1, 0, 0);
    ECEntityClassP ecClass;
    schema->CreateEntityClass(ecClass, "Test");
    PrimitiveArrayECPropertyP arrayProp;
    ecClass->CreatePrimitiveArrayProperty(arrayProp, "Strings", PRIMITIVETYPE_String);
    PrimitiveECPropertyP primProp;
    ecClass->CreatePrimitiveProperty(primProp, "Stringy", PRIMITIVETYPE_String);

    IECInstancePtr inst = ecClass->GetDefaultStandaloneEnabler()->CreateInstance();
    inst->AddArrayElements("Strings", 1);
    inst->SetValue("Strings", ECValue("String", false), 0);
    inst->ClearArray("Strings");
    }

/*---------------------------------------------------------------------------------**//**
* Test the ECValue flag that returns strings as pointers into instance data rather than
* making a copy.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDBufferTests, PointersIntoInstanceMemory)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "InstancePointers", "ts", 1, 0, 0);
    ECEntityClassP ecClass;
    schema->CreateEntityClass(ecClass, "InstancePointers");
    PrimitiveECPropertyP ecprop;
    ecClass->CreatePrimitiveProperty(ecprop, "String", PRIMITIVETYPE_String);

    IECInstancePtr instance = ecClass->GetDefaultStandaloneEnabler()->CreateInstance();
    instance->SetValue("String", ECValue("string", false));

    ECValue v;
    instance->GetValue(v, "String");
    EXPECT_EQ(0, strcmp(v.GetUtf8CP(), "string"));

    // To test whether or not we got back a pointer into instance memory, we'll modify the memory. Real code would never do this of course.
    Utf8Char newStr[] = "STRING";
    Utf8Char* pStr = const_cast<Utf8Char*> (v.GetUtf8CP());
    memcpy(pStr, newStr, _countof(newStr) * sizeof(Utf8Char));

    instance->GetValue(v, "String");
    EXPECT_EQ(0, strcmp(v.GetUtf8CP(), "string"));   // did not modify instance data

    v.SetAllowsPointersIntoInstanceMemory(true);
    instance->GetValue(v, "String");

    pStr = const_cast<Utf8P> (v.GetUtf8CP());
    memcpy(pStr, newStr, _countof(newStr) * sizeof(Utf8Char));

    // The flag should not be reset when the ECValue was assigned a value
    EXPECT_TRUE(v.AllowsPointersIntoInstanceMemory());

    instance->GetValue(v, "String");
    //EXPECT_EQ (v.GetString(), pStr);                // got back pointer to same address in instance data
    //EXPECT_EQ (0, wcscmp (v.GetString(), newStr));  // modified instance memory directly through returned pointer
    }

/*---------------------------------------------------------------------------------**//**
* Test using ECDBuffer::CopyDataBuffer() to populate an ECDBuffer from another ECDBuffer
* created for a different ClassLayout.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDBufferTests, ConvertDataBuffer)
    {
    // Create initial version of class
    ECSchemaPtr schemaA;
    ECSchema::CreateSchema(schemaA, "SchemaA", "ts", 1, 0, 0);
    ECEntityClassP classA;
    schemaA->CreateEntityClass(classA, "ClassA");

    PrimitiveECPropertyP prim;
    classA->CreatePrimitiveProperty(prim, "String", PRIMITIVETYPE_String);
    classA->CreatePrimitiveProperty(prim, "Int", PRIMITIVETYPE_Integer);
    classA->CreatePrimitiveProperty(prim, "Bool", PRIMITIVETYPE_Boolean);
    classA->CreatePrimitiveProperty(prim, "RemoveThisProperty", PRIMITIVETYPE_String);

    // initialize instance of initial class layout
    StandaloneECInstancePtr instanceA = classA->GetDefaultStandaloneEnabler()->CreateInstance();
    instanceA->SetValue("String", ECValue("ABC", false));
    instanceA->SetValue("Int", ECValue(123));
    instanceA->SetValue("Bool", ECValue(true));
    instanceA->SetValue("RemoveThisProperty", ECValue("stuff", false));

    // test we can copy the data buffer using the same class layout
    StandaloneECInstancePtr instanceA2 = classA->GetDefaultStandaloneEnabler()->CreateInstance();
    EXPECT_EQ(ECObjectsStatus::Success, instanceA2->CopyDataBuffer(*instanceA, true));

    TestValue(*instanceA2, "String", "ABC");
    TestValue(*instanceA2, "Int", 123);
    TestValue(*instanceA2, "Bool", true);
    TestValue(*instanceA2, "RemoveThisProperty", "stuff");

    // Create a new version of the class with different layout
    ECSchemaPtr schemaA2;
    ECSchema::CreateSchema(schemaA2, "SchemaA", "ts", 2, 0 ,0);
    ECEntityClassP classA2;
    schemaA2->CreateEntityClass(classA2, "ClassA");

    classA2->CreatePrimitiveProperty(prim, "String", PRIMITIVETYPE_Integer);
    classA2->CreatePrimitiveProperty(prim, "Int", PRIMITIVETYPE_String);
    classA2->CreatePrimitiveProperty(prim, "AddedThisProperty", PRIMITIVETYPE_Double);
    classA2->CreatePrimitiveProperty(prim, "Bool", PRIMITIVETYPE_Boolean);

    // Create instance of new class layout and initialize from old layout
    instanceA2 = classA2->GetDefaultStandaloneEnabler()->CreateInstance();
    EXPECT_EQ(ECObjectsStatus::Success, instanceA2->CopyDataBuffer(*instanceA, true));

    TestValue(*instanceA2, "Int", "123"); // int->string converted
    TestValue(*instanceA2, "String", ECValue()); // string->int conversion failed
    TestValue(*instanceA2, "AddedThisProperty", ECValue());   // not present in old class, uninitialized
    TestValue(*instanceA2, "RemovedThisProperty", ExpectedValue());   // not present in new class
    TestValue(*instanceA2, "Bool", true); // no change, value preserved
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDBufferTests, ConvertDataBuffer_Arrays)
    {
    ECSchemaPtr schemaA;
    ECSchema::CreateSchema(schemaA, "SchemaA", "ts", 1, 0, 0);
    ECEntityClassP classA;
    schemaA->CreateEntityClass(classA, "ClassA");

    PrimitiveArrayECPropertyP prop;
    classA->CreatePrimitiveArrayProperty(prop, "IntArray", PRIMITIVETYPE_Integer);
    classA->CreatePrimitiveArrayProperty(prop, "StringArray", PRIMITIVETYPE_String);
    classA->CreatePrimitiveArrayProperty(prop, "BoolArray", PRIMITIVETYPE_Boolean);
    classA->CreatePrimitiveArrayProperty(prop, "Removed", PRIMITIVETYPE_String);

    StandaloneECInstancePtr instanceA = classA->GetDefaultStandaloneEnabler()->CreateInstance();
    instanceA->AddArrayElements("IntArray", 2);
    instanceA->SetValue("IntArray", ECValue(0), 0);
    instanceA->SetValue("IntArray", ECValue(1), 1);

    instanceA->AddArrayElements("StringArray", 2);
    instanceA->SetValue("StringArray", ECValue("abc"), 0);
    instanceA->SetValue("StringArray", ECValue("123"), 1);

    instanceA->AddArrayElements("BoolArray", 2);
    instanceA->SetValue("BoolArray", ECValue(false), 0);
    instanceA->SetValue("BoolArray", ECValue(true), 1);

    instanceA->AddArrayElements("Removed", 1);
    instanceA->SetValue("Removed", ECValue("stuff"), 0);

    ECSchemaPtr schemaA2;
    ECSchema::CreateSchema(schemaA2, "SchemaA", "ts", 2, 0, 0);
    ECEntityClassP classA2;
    schemaA2->CreateEntityClass(classA2, "ClassA");
    classA2->CreatePrimitiveArrayProperty(prop, "IntArray", PRIMITIVETYPE_String);
    classA2->CreatePrimitiveArrayProperty(prop, "StringArray", PRIMITIVETYPE_Integer);
    classA2->CreatePrimitiveArrayProperty(prop, "BoolArray", PRIMITIVETYPE_Boolean);
    classA2->CreatePrimitiveArrayProperty(prop, "Added", PRIMITIVETYPE_String);

    StandaloneECInstancePtr instanceA2 = classA2->GetDefaultStandaloneEnabler()->CreateInstance();
    EXPECT_EQ(ECObjectsStatus::Success, instanceA2->CopyDataBuffer(*instanceA, true));

    TestValue(*instanceA2, "IntArray", "0", 0);
    TestValue(*instanceA2, "IntArray", "1", 1);

    ECValue null;
    TestValue(*instanceA2, "StringArray", null, 0);
    TestValue(*instanceA2, "StringArray", ECValue(123), 1);

    TestValue(*instanceA2, "BoolArray", false, 0);
    TestValue(*instanceA2, "BoolArray", true, 1);

    TestValue(*instanceA2, "Removed", ExpectedValue(), 0);
    TestValue(*instanceA2, "Removed", ExpectedValue(), 1);

    ECValue emptyArray;
    emptyArray.SetPrimitiveArrayInfo(PRIMITIVETYPE_String, 0, false);
    TestValue(*instanceA2, "Added", emptyArray);
    }

/*---------------------------------------------------------------------------------**//**
* Copying a data buffer containing struct arrays should copy the struct identifiers
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDBufferTests, ConvertDataBuffer_StructArrays)
    {
    enum struct Case
        {
        SameClassLayout,
        DifferentStructElementTypeLayout,
        StructElementContainsEmbeddedStruct,
        MismatchingTypeKind,
        // remove properties from the class we're copying to
        MissingStruct,
        MissingStructArray,
        MissingPrimitive,
        // add properties to the class we're copying to
        NewStruct,
        NewStructArray,
        NewPrimitive,
        };

    const auto test = [&](Case case_) -> IECInstancePtr
        {
        ECSchemaPtr schema1;
        ECSchema::CreateSchema(schema1, "Schema", "ts", 1, 0, 0);

        ECStructClassP simpleStruct1;
        PrimitiveECPropertyP primProp;
        schema1->CreateStructClass(simpleStruct1, "EmbeddedStruct");
        simpleStruct1->CreatePrimitiveProperty(primProp, "PropE", PRIMITIVETYPE_String);

        ECStructClassP struct1;
        schema1->CreateStructClass(struct1, "Struct");
        struct1->CreatePrimitiveProperty(primProp, "StructElemProp", PRIMITIVETYPE_String);

        StructECPropertyP structProp;
        StructArrayECPropertyP structArrayProp;
        switch (case_)
            {
            case Case::StructElementContainsEmbeddedStruct:
                struct1->CreateStructProperty(structProp, "EmbeddedStructProp", *simpleStruct1);
                break;
            case Case::MissingStruct:
                struct1->CreateStructProperty(structProp, "MissingProp", *simpleStruct1);
                break;
            case Case::MissingStructArray:
                struct1->CreateStructArrayProperty(structArrayProp, "MissingProp", *simpleStruct1);
                break;
            case Case::MissingPrimitive:
                struct1->CreatePrimitiveProperty(primProp, "MissingProp", PRIMITIVETYPE_Double);
                break;
            }

        ECEntityClassP class1;
        schema1->CreateEntityClass(class1, "Class");
        class1->CreateStructArrayProperty(structArrayProp, "StructArray", *struct1);
        class1->CreatePrimitiveProperty(primProp, "ExtraClassProp", PRIMITIVETYPE_String);

        StandaloneECInstancePtr instance1 = class1->GetDefaultStandaloneEnabler()->CreateInstance();
        instance1->AddArrayElements("StructArray", 2);

        const auto addCaseProps = [&](IECInstancePtr structInstance)
            {
            ECValue val;
            switch (case_)
                {
                case Case::StructElementContainsEmbeddedStruct:
                    structInstance->SetValue("EmbeddedStructProp.PropE", ECValue("value"));
                    break;
                case Case::MissingStruct:
                    structInstance->SetValue("EmbeddedStructProp.PropE", ECValue("value"));
                    break;
                case Case::MissingStructArray:
                    structInstance->SetValue("MissingProp.PropE", ECValue("value1"), 0);
                    structInstance->SetValue("MissingProp.PropE", ECValue("value2"), 1);
                    break;
                case Case::MissingPrimitive:
                    structInstance->SetValue("MissingProp", ECValue(2.3));
                    break;
                }
            };
        ECValue structVal;
        IECInstancePtr structInstance;
        structInstance = struct1->GetDefaultStandaloneEnabler()->CreateInstance();
        structInstance->SetValue("StructElemProp", ECValue("1"));
        instance1->SetValue("ExtraClassProp", ECValue("extra-class-prop"));
        addCaseProps(structInstance);
        structVal.SetStruct(structInstance.get());
        instance1->SetValue("StructArray", structVal, 0);

        structInstance = struct1->GetDefaultStandaloneEnabler()->CreateInstance();
        structInstance->SetValue("StructElemProp", ECValue("2"));
        addCaseProps(structInstance);
        structVal.SetStruct(structInstance.get());
        instance1->SetValue("StructArray", structVal, 1);

        ECSchemaPtr schema2;
        ECSchema::CreateSchema(schema2, "Schema", "ts", 2, 0, 0);

        ECStructClassP simpleStruct2;
        schema2->CreateStructClass(simpleStruct2, "EmbeddedStruct");
        simpleStruct2->CreatePrimitiveProperty(primProp, "PropE", PRIMITIVETYPE_String);

        ECStructClassP struct2;
        schema2->CreateStructClass(struct2, "Struct");
        switch (case_)
            {
            case Case::StructElementContainsEmbeddedStruct:
                struct2->CreateStructProperty(structProp, "EmbeddedStructProp", *simpleStruct2);
                break;
            case Case::DifferentStructElementTypeLayout:
                struct2->CreatePrimitiveProperty(primProp, "StructElemProp", PRIMITIVETYPE_Integer);
                break;
            case Case::NewStruct:
                struct2->CreateStructProperty(structProp, "NewProp", *simpleStruct2);
                break;
            case Case::NewStructArray:
                struct2->CreateStructArrayProperty(structArrayProp, "NewProp", *simpleStruct2);
                break;
            case Case::NewPrimitive:
                struct2->CreatePrimitiveProperty(primProp, "NewProp", PRIMITIVETYPE_Integer);
                break;
            }
        if (nullptr == struct2->GetPropertyP("StructElemProp"))
            struct2->CreatePrimitiveProperty(primProp, "StructElemProp", PRIMITIVETYPE_String);

        ECEntityClassP class2;
        schema2->CreateEntityClass(class2, "Class");
        class2->CreateStructArrayProperty(structArrayProp, "StructArray", *struct2);
        if (case_ == Case::MismatchingTypeKind)
            class2->CreateStructProperty(structProp, "ExtraClassProp", *simpleStruct2);
        else
            class2->CreatePrimitiveProperty(primProp, "ExtraClassProp", PRIMITIVETYPE_String);

        StandaloneECInstancePtr instance2 = class2->GetDefaultStandaloneEnabler()->CreateInstance();
        EXPECT_EQ(ECObjectsStatus::Success, instance2->CopyDataBuffer(*instance1, true));

        ECValue arrayVal;
        arrayVal.SetStructArrayInfo(2, false);
        TestValue(*instance2, "StructArray", arrayVal);

        return instance2;
        };

    ECValue testStructValue, testStructValueInner;
    IECInstancePtr result;

    result = test(Case::SameClassLayout);
    result->GetValue(testStructValue, "StructArray", 0);
    TestValue(*testStructValue.GetStruct(), "StructElemProp", "1");
    result->GetValue(testStructValue, "StructArray", 1);
    TestValue(*testStructValue.GetStruct(), "StructElemProp", "2");

    result = test(Case::DifferentStructElementTypeLayout);
    result->GetValue(testStructValue, "StructArray", 0);
    TestValue(*testStructValue.GetStruct(), "StructElemProp", 1);
    result->GetValue(testStructValue, "StructArray", 1);
    TestValue(*testStructValue.GetStruct(), "StructElemProp", 2);

    result = test(Case::StructElementContainsEmbeddedStruct);
    result->GetValue(testStructValue, "StructArray", 0);
    TestValue(*testStructValue.GetStruct(), "StructElemProp", "1");
    TestValue(*testStructValue.GetStruct(), "EmbeddedStructProp.PropE", "value");
    result->GetValue(testStructValue, "StructArray", 1);
    TestValue(*testStructValue.GetStruct(), "StructElemProp", "2");
    TestValue(*testStructValue.GetStruct(), "EmbeddedStructProp.PropE", "value");

    result = test(Case::MismatchingTypeKind);
    result->GetValue(testStructValue, "StructArray", 0);
    TestValue(*testStructValue.GetStruct(), "StructElemProp", "1");
    TestValue(*result, "ExtraClassProp", ECValue(/*null*/));
    result->GetValue(testStructValue, "StructArray", 1);
    TestValue(*testStructValue.GetStruct(), "StructElemProp", "2");
    TestValue(*result, "ExtraClassProp", ECValue(/*null*/));

    result = test(Case::MissingStruct);
    result->GetValue(testStructValue, "StructArray", 0);
    TestValue(*testStructValue.GetStruct(), "StructElemProp", "1");
    result->GetValue(testStructValue, "StructArray", 1);
    TestValue(*testStructValue.GetStruct(), "StructElemProp", "2");

    result = test(Case::MissingStructArray);
    result->GetValue(testStructValue, "StructArray", 0);
    TestValue(*testStructValue.GetStruct(), "StructElemProp", "1");
    result->GetValue(testStructValue, "StructArray", 1);
    TestValue(*testStructValue.GetStruct(), "StructElemProp", "2");

    result = test(Case::MissingPrimitive);
    result->GetValue(testStructValue, "StructArray", 0);
    TestValue(*testStructValue.GetStruct(), "StructElemProp", "1");
    result->GetValue(testStructValue, "StructArray", 1);
    TestValue(*testStructValue.GetStruct(), "StructElemProp", "2");

    result = test(Case::NewStruct);
    result->GetValue(testStructValue, "StructArray", 0);
    TestValue(*testStructValue.GetStruct(), "StructElemProp", "1");
    TestValue(*testStructValue.GetStruct(), "NewProp", ECValue(/*null*/));
    result->GetValue(testStructValue, "StructArray", 1);
    TestValue(*testStructValue.GetStruct(), "StructElemProp", "2");
    TestValue(*testStructValue.GetStruct(), "NewProp", ECValue(/*null*/));

    result = test(Case::NewStructArray);
    ECValue emptyArrayVal;
    emptyArrayVal.SetStructArrayInfo(0, false);
    result->GetValue(testStructValue, "StructArray", 0);
    TestValue(*testStructValue.GetStruct(), "StructElemProp", "1");
    TestValue(*testStructValue.GetStruct(), "NewProp", emptyArrayVal);
    result->GetValue(testStructValue, "StructArray", 1);
    TestValue(*testStructValue.GetStruct(), "StructElemProp", "2");
    TestValue(*testStructValue.GetStruct(), "NewProp", emptyArrayVal);

    result = test(Case::NewPrimitive);
    result->GetValue(testStructValue, "StructArray", 0);
    TestValue(*testStructValue.GetStruct(), "StructElemProp", "1");
    TestValue(*testStructValue.GetStruct(), "NewProp", ECValue(/*null*/));
    result->GetValue(testStructValue, "StructArray", 1);
    TestValue(*testStructValue.GetStruct(), "StructElemProp", "2");
    TestValue(*testStructValue.GetStruct(), "NewProp", ECValue(/*null*/));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDBufferTests, ConvertDataBuffer_NonPrimitiveConversions)
    {
    enum class ValueKindCase
        {
        Primitive,
        Struct,
        StructArray,
        PrimitiveArray,
        Navigation,
        };

    // we need a different relation per property or we violate the one-property per relation rule
    std::array<ECRelationshipClassP, 7> relations;

    ECSchemaPtr schema1; // We are returning and using ECInstancePtr in the method below, which has references to the schema, so
    //it needs to stay allocated.
    const auto test = [&](ValueKindCase toKind) -> IECInstancePtr
        {
        ECSchema::CreateSchema(schema1, "Schema1", "ts1", 1, 0, 0);

        ECStructClassP simpleStruct;
        PrimitiveECPropertyP primProp;
        PrimitiveArrayECPropertyP arrayProp;
        StructECPropertyP structProp;
        StructArrayECPropertyP structArrayProp;
        NavigationECPropertyP navigationProp;

        schema1->CreateStructClass(simpleStruct, "SimpleStruct");
        simpleStruct->CreatePrimitiveProperty(primProp, "SimpleProp", PRIMITIVETYPE_String);

        ECEntityClassP baseClass, classA, classB;
        schema1->CreateEntityClass(baseClass, "BaseClass");
        schema1->CreateEntityClass(classA, "ClassA");
        classA->AddBaseClass(*baseClass);
        schema1->CreateEntityClass(classB, "ClassB");
        classA->CreatePrimitiveProperty(primProp, "PropA", PRIMITIVETYPE_String);
        classB->CreatePrimitiveProperty(primProp, "PropB", PRIMITIVETYPE_Double);

        for (int i = 0; i < relations.size(); ++i)
            {
            auto& relation = relations[i];
            Utf8PrintfString relationName("Relation%d", i);
            schema1->CreateRelationshipClass(relation, relationName.c_str());
            relation->GetSource().SetAbstractConstraint(*baseClass);
            relation->GetSource().AddClass(*classA);
            relation->GetTarget().SetAbstractConstraint(*classB);
            relation->GetTarget().AddClass(*classB);
            }

        classA->CreateStructProperty(structProp, "FromStruct", *simpleStruct);
        classA->CreatePrimitiveProperty(primProp, "FromPrimitive", PRIMITIVETYPE_Double);
        classA->CreateNavigationProperty(navigationProp, "FromNavigation", *relations[0], ECRelatedInstanceDirection::Forward);
        classA->CreateStructArrayProperty(structArrayProp, "FromStructArray", *simpleStruct);
        classA->CreatePrimitiveArrayProperty(arrayProp, "FromPrimitiveArray", PRIMITIVETYPE_String);
        classA->CreatePrimitiveArrayProperty(arrayProp, "FromIntegerableArray", PRIMITIVETYPE_String);

        StandaloneECInstancePtr instance1 = classA->GetDefaultStandaloneEnabler()->CreateInstance();
        instance1->SetValue("FromStruct.SimpleProp", ECValue("simple-prop-value"));
        instance1->SetValue("FromPrimitive", ECValue(2.3));
        instance1->SetValue("FromNavigation", ECValue(BeInt64Id(5), relations[0]));
        instance1->AddArrayElements("FromStructArray", 2);
            {
            ECValue structVal;
            IECInstancePtr structInstance;
            structInstance = simpleStruct->GetDefaultStandaloneEnabler()->CreateInstance();
            structInstance->SetValue("SimpleProp", ECValue("simple-prop-value-1"));
            structVal.SetStruct(structInstance.get());
            instance1->SetValue("FromStructArray", structVal, 0);

            structInstance = simpleStruct->GetDefaultStandaloneEnabler()->CreateInstance();
            structInstance->SetValue("SimpleProp", ECValue("simple-prop-value-2"));
            structVal.SetStruct(structInstance.get());
            instance1->SetValue("FromStructArray", structVal, 1);
            }

        instance1->AddArrayElements("FromPrimitiveArray", 2);
        instance1->SetValue("FromPrimitiveArray", ECValue("entry-1"), 0);
        instance1->SetValue("FromPrimitiveArray", ECValue("entry-2"), 1);

        instance1->AddArrayElements("FromIntegerableArray", 2);
        instance1->SetValue("FromIntegerableArray", ECValue("11"), 0); // values can be converted to integers
        instance1->SetValue("FromIntegerableArray", ECValue("12"), 1);

        ECEntityClassP classC;
        schema1->CreateEntityClass(classC, "ClassC");
        classC->AddBaseClass(*baseClass);
        for (auto& relation : relations)
            relation->GetSource().AddClass(*classC);

        switch (toKind)
            {
            case ValueKindCase::Primitive:
                classC->CreatePrimitiveProperty(primProp, "FromStruct", PRIMITIVETYPE_String);
                classC->CreatePrimitiveProperty(primProp, "FromPrimitive", PRIMITIVETYPE_String);
                classC->CreatePrimitiveProperty(primProp, "FromNavigation", PRIMITIVETYPE_String);
                classC->CreatePrimitiveProperty(primProp, "FromStructArray", PRIMITIVETYPE_String);
                classC->CreatePrimitiveProperty(primProp, "FromPrimitiveArray", PRIMITIVETYPE_String);
                classC->CreatePrimitiveProperty(primProp, "FromIntegerableArray", PRIMITIVETYPE_String);
                break;
            case ValueKindCase::Struct:
                classC->CreateStructProperty(structProp, "FromStruct", *simpleStruct);
                classC->CreateStructProperty(structProp, "FromPrimitive", *simpleStruct);
                classC->CreateStructProperty(structProp, "FromNavigation", *simpleStruct);
                classC->CreateStructProperty(structProp, "FromStructArray", *simpleStruct);
                classC->CreateStructProperty(structProp, "FromPrimitiveArray", *simpleStruct);
                classC->CreateStructProperty(structProp, "FromIntegerableArray", *simpleStruct);
                break;
            case ValueKindCase::StructArray:
                classC->CreateStructArrayProperty(structArrayProp, "FromStruct", *simpleStruct);
                classC->CreateStructArrayProperty(structArrayProp, "FromPrimitive", *simpleStruct);
                classC->CreateStructArrayProperty(structArrayProp, "FromNavigation", *simpleStruct);
                classC->CreateStructArrayProperty(structArrayProp, "FromStructArray", *simpleStruct);
                classC->CreateStructArrayProperty(structArrayProp, "FromPrimitiveArray", *simpleStruct);
                classC->CreateStructArrayProperty(structArrayProp, "FromIntegerableArray", *simpleStruct);
                break;
            case ValueKindCase::PrimitiveArray:
                classC->CreatePrimitiveArrayProperty(arrayProp, "FromStruct", PRIMITIVETYPE_String);
                classC->CreatePrimitiveArrayProperty(arrayProp, "FromPrimitive", PRIMITIVETYPE_String);
                classC->CreatePrimitiveArrayProperty(arrayProp, "FromNavigation", PRIMITIVETYPE_String);
                classC->CreatePrimitiveArrayProperty(arrayProp, "FromStructArray", PRIMITIVETYPE_String);
                classC->CreatePrimitiveArrayProperty(arrayProp, "FromPrimitiveArray", PRIMITIVETYPE_String);
                classC->CreatePrimitiveArrayProperty(arrayProp, "FromIntegerableArray", PRIMITIVETYPE_String);
                break;
            case ValueKindCase::Navigation:
                classC->CreateNavigationProperty(navigationProp, "FromStruct", *relations[1], ECRelatedInstanceDirection::Forward);
                classC->CreateNavigationProperty(navigationProp, "FromPrimitive", *relations[2], ECRelatedInstanceDirection::Forward);
                classC->CreateNavigationProperty(navigationProp, "FromNavigation", *relations[3], ECRelatedInstanceDirection::Forward);
                classC->CreateNavigationProperty(navigationProp, "FromStructArray", *relations[4], ECRelatedInstanceDirection::Forward);
                classC->CreateNavigationProperty(navigationProp, "FromPrimitiveArray", *relations[5], ECRelatedInstanceDirection::Forward);
                classC->CreateNavigationProperty(navigationProp, "FromIntegerableArray", *relations[6], ECRelatedInstanceDirection::Forward);
                break;
            }

        StandaloneECInstancePtr instance2 = classC->GetDefaultStandaloneEnabler()->CreateInstance();
        EXPECT_EQ(ECObjectsStatus::Success, instance2->CopyDataBuffer(*instance1, true));
        return instance2;
        };

    ECValue emptyStructArray; emptyStructArray.SetStructArrayInfo(0, false);
    ECValue emptyStringArray; emptyStringArray.SetPrimitiveArrayInfo(PRIMITIVETYPE_String, 0, false);

    {
    auto result = test(ValueKindCase::Primitive);
    TestValue(*result, "FromStruct", ECValue());
    TestValue(*result, "FromPrimitive", ECValue("2.2999999999999998"));
    TestValue(*result, "FromNavigation", ECValue());
    TestValue(*result, "FromStructArray", ECValue());
    TestValue(*result, "FromPrimitiveArray", ECValue());
    TestValue(*result, "FromIntegerableArray", ECValue());
    }

    {
    auto result = test(ValueKindCase::Struct);
    TestValue(*result, "FromStruct.SimpleProp", ECValue("simple-prop-value"));
    TestValue(*result, "FromPrimitive", ECValue());
    TestValue(*result, "FromNavigation", ECValue());
    TestValue(*result, "FromStructArray", ECValue());
    TestValue(*result, "FromPrimitiveArray", ECValue());
    TestValue(*result, "FromIntegerableArray", ECValue());
    }

    {
    auto result = test(ValueKindCase::StructArray);
    TestValue(*result, "FromStruct", emptyStructArray);
    TestValue(*result, "FromPrimitive", emptyStructArray);
    TestValue(*result, "FromNavigation", emptyStructArray);

    ECValue arrayInfoVal;
    ECValue arrayEntries[2];

    arrayInfoVal.SetStructArrayInfo(2, false);
    TestValue(*result, "FromStructArray", arrayInfoVal);
    result->GetValue(arrayEntries[0], "FromStructArray", 0);
    TestValue(*arrayEntries[0].GetStruct(), "SimpleProp", ECValue("simple-prop-value-1"));
    result->GetValue(arrayEntries[1], "FromStructArray", 1);
    TestValue(*arrayEntries[1].GetStruct(), "SimpleProp", ECValue("simple-prop-value-2"));

    arrayInfoVal.SetStructArrayInfo(2, false);
    TestValue(*result, "FromPrimitiveArray", arrayInfoVal);
    TestValue(*result, "FromPrimitiveArray", ECValue(), 0);
    TestValue(*result, "FromPrimitiveArray", ECValue(), 1);

    arrayInfoVal.SetStructArrayInfo(2, false);
    TestValue(*result, "FromIntegerableArray", arrayInfoVal);
    TestValue(*result, "FromIntegerableArray", ECValue(), 0);
    TestValue(*result, "FromIntegerableArray", ECValue(), 1);
    }

    {
    auto result = test(ValueKindCase::Navigation);
    TestValue(*result, "FromStruct", ECValue());
    TestValue(*result, "FromPrimitive", ECValue());
    // relation is set by each call of `test`, so if test order has to change, consider
    // returning it as part of the `test` return value to make it order independent
    TestValue(*result, "FromNavigation", ECValue(BeInt64Id(5), relations[0]));
    TestValue(*result, "FromStructArray", ECValue());
    TestValue(*result, "FromPrimitiveArray", ECValue());
    TestValue(*result, "FromIntegerableArray", ECValue());
    }

    {
    auto result = test(ValueKindCase::PrimitiveArray);
    TestValue(*result, "FromStruct", emptyStringArray);
    TestValue(*result, "FromPrimitive", emptyStringArray);
    TestValue(*result, "FromNavigation", emptyStringArray);

    ECValue arrayInfoVal;
    ECValue arrayEntries[2];

    arrayInfoVal.SetPrimitiveArrayInfo(PRIMITIVETYPE_String, 2, false);
    TestValue(*result, "FromStructArray", arrayInfoVal);
    TestValue(*result, "FromStructArray", ECValue(), 0);
    TestValue(*result, "FromStructArray", ECValue(), 1);

    arrayInfoVal.SetPrimitiveArrayInfo(PRIMITIVETYPE_String, 2, false);
    TestValue(*result, "FromPrimitiveArray", arrayInfoVal);
    TestValue(*result, "FromPrimitiveArray", ECValue("entry-1"), 0);
    TestValue(*result, "FromPrimitiveArray", ECValue("entry-2"), 1);

    arrayInfoVal.SetPrimitiveArrayInfo(PRIMITIVETYPE_String, 2, false);
    TestValue(*result, "FromIntegerableArray", arrayInfoVal);
    TestValue(*result, "FromIntegerableArray", ECValue("11"), 0);
    TestValue(*result, "FromIntegerableArray", ECValue("12"), 1);
    }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDBufferTests, ArraysAreNotNull)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "Test", "ts", 1, 0, 0);
    ECEntityClassP ecClass;
    schema->CreateEntityClass(ecClass, "Test");
    PrimitiveArrayECPropertyP arrayProp;
    ecClass->CreatePrimitiveArrayProperty(arrayProp, "Array", PRIMITIVETYPE_String);

    StandaloneECInstancePtr instance = ecClass->GetDefaultStandaloneEnabler()->CreateInstance();
    bool isNull = false;
    EXPECT_EQ(ECObjectsStatus::Success, instance->IsPropertyNull(isNull, "Array"));
    EXPECT_FALSE(isNull);

    ECValue v;
    EXPECT_EQ(ECObjectsStatus::Success, instance->GetValue(v, "Array"));
    EXPECT_FALSE(v.IsNull());
    }

struct ECDBufferTester : MemoryECInstanceBase
    {
public:
    StandaloneECEnablerCR m_enabler;
    IECInstanceP _GetAsIECInstance() const override { return nullptr; }
    ClassLayoutCR _GetClassLayout() const override { return m_enabler.GetClassLayout(); }
    
    ECDBufferTester (StandaloneECEnablerCR enabler) : MemoryECInstanceBase(enabler.GetClassLayout(), 42, false, enabler.GetClass()), m_enabler(enabler) {};
    ECObjectsStatus InsertArrayElementsAt(Utf8CP propertyAccessString, uint32_t index, uint32_t count)
        {
        uint32_t propIdx;
        m_enabler.GetPropertyIndex(propIdx, propertyAccessString);
        return InsertNullArrayElementsAt(propIdx, index, count);
        }
    ECObjectsStatus SetArrayLength(Utf8CP propertyAccessString, uint32_t desiredLength)
        {
        ECValue arrayValue;
        GetValueFromMemory(arrayValue, propertyAccessString);
        if (!arrayValue.IsArray())
            return ECObjectsStatus::DataTypeMismatch;

        uint32_t currentLength = arrayValue.GetArrayInfo().GetCount();
        if (currentLength == desiredLength)
            return ECObjectsStatus::Success;

        if (currentLength > desiredLength)
            {
            PropertyLayoutCP propertyLayout = NULL;
            uint32_t numToRemove = currentLength - desiredLength;
            GetClassLayout().GetPropertyLayout(propertyLayout, propertyAccessString);
            return RemoveArrayElementsFromMemory(*propertyLayout, 0, numToRemove);
            }

        uint32_t numToAdd = desiredLength - currentLength;
        return InsertArrayElementsAt(propertyAccessString, 0, numToAdd);
        }
    };

//---------------------------------------------------------------------------------------//
//* @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------//
TEST_F(ECDBufferTests, ECDBufferCanInsertArrayEntriesWhenUsingWriteBuffer)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "Test", "ts", 1, 0, 0);
    ECEntityClassP ecClass;
    schema->CreateEntityClass(ecClass, "Test");
    PrimitiveArrayECPropertyP arrayProp;
    ecClass->CreatePrimitiveArrayProperty(arrayProp, "Array", PRIMITIVETYPE_String);

    ECDBufferTester tester(*ecClass->GetDefaultStandaloneEnabler());
    tester.InsertArrayElementsAt("Array", 0, 4);
    ECValue v;
    EXPECT_EQ(ECObjectsStatus::Success, tester.GetValueFromMemory(v, "Array", true, 1));
    EXPECT_TRUE(v.IsNull());

    tester.InsertArrayElementsAt("Array", 0, 4);
    EXPECT_EQ(ECObjectsStatus::Success, tester.GetValueFromMemory(v, "Array", true, 1));
    EXPECT_TRUE(v.IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDBufferTests, CopyFromBuffer)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "Test", "test", 1, 0, 0);
    EXPECT_TRUE(schema.IsValid());
    ECEntityClassP class1;
    ECObjectsStatus status = schema->CreateEntityClass(class1, "Class1");
    EXPECT_EQ(ECObjectsStatus::Success, status);

    PrimitiveECPropertyP booleanProp1, booleanProp2, booleanProp3;
    PrimitiveECPropertyP dateTimeProp1, dateTimeProp2, dateTimeProp3;
    PrimitiveECPropertyP doubleProp1, doubleProp2, doubleProp3;
    PrimitiveECPropertyP intProp1, intProp2, intProp3;
    PrimitiveECPropertyP longProp1, longProp2, longProp3;
    PrimitiveECPropertyP pointProp1, pointProp2, pointProp3;
    PrimitiveECPropertyP stringProp1, stringProp2, stringProp3;
    PrimitiveArrayECPropertyP arrayProp1, arrayProp2, arrayProp3;

    EXPECT_EQ(ECObjectsStatus::Success, class1->CreatePrimitiveProperty(longProp1, "L1", PRIMITIVETYPE_Long));
    EXPECT_EQ(ECObjectsStatus::Success, class1->CreatePrimitiveProperty(doubleProp1, "D1", PRIMITIVETYPE_Double));
    EXPECT_EQ(ECObjectsStatus::Success, class1->CreatePrimitiveArrayProperty(arrayProp1, "A1", PRIMITIVETYPE_Double));
    EXPECT_EQ(ECObjectsStatus::Success, class1->CreatePrimitiveProperty(stringProp1, "S1", PRIMITIVETYPE_String));
    EXPECT_EQ(ECObjectsStatus::Success, class1->CreatePrimitiveProperty(intProp1, "I1", PRIMITIVETYPE_Integer));
    EXPECT_EQ(ECObjectsStatus::Success, class1->CreatePrimitiveProperty(booleanProp1, "B1", PRIMITIVETYPE_Boolean));
    EXPECT_EQ(ECObjectsStatus::Success, class1->CreatePrimitiveProperty(pointProp1, "P1", PRIMITIVETYPE_Point3d));
    EXPECT_EQ(ECObjectsStatus::Success, class1->CreatePrimitiveProperty(dateTimeProp1, "DT1", PRIMITIVETYPE_DateTime));
    EXPECT_EQ(ECObjectsStatus::Success, class1->CreatePrimitiveProperty(longProp2, "L2", PRIMITIVETYPE_Long));
    EXPECT_EQ(ECObjectsStatus::Success, class1->CreatePrimitiveProperty(doubleProp2, "D2", PRIMITIVETYPE_Double));
    EXPECT_EQ(ECObjectsStatus::Success, class1->CreatePrimitiveArrayProperty(arrayProp2, "A2", PRIMITIVETYPE_Integer));
    EXPECT_EQ(ECObjectsStatus::Success, class1->CreatePrimitiveProperty(stringProp2, "S2", PRIMITIVETYPE_String));
    EXPECT_EQ(ECObjectsStatus::Success, class1->CreatePrimitiveProperty(intProp2, "I2", PRIMITIVETYPE_Integer));
    EXPECT_EQ(ECObjectsStatus::Success, class1->CreatePrimitiveProperty(booleanProp2, "B2", PRIMITIVETYPE_Boolean));
    EXPECT_EQ(ECObjectsStatus::Success, class1->CreatePrimitiveProperty(pointProp2, "P2", PRIMITIVETYPE_Point3d));
    EXPECT_EQ(ECObjectsStatus::Success, class1->CreatePrimitiveProperty(dateTimeProp2, "DT2", PRIMITIVETYPE_DateTime));
    EXPECT_EQ(ECObjectsStatus::Success, class1->CreatePrimitiveProperty(longProp3, "L3", PRIMITIVETYPE_Long));
    EXPECT_EQ(ECObjectsStatus::Success, class1->CreatePrimitiveProperty(doubleProp3, "D3", PRIMITIVETYPE_Double));
    EXPECT_EQ(ECObjectsStatus::Success, class1->CreatePrimitiveArrayProperty(arrayProp3, "A3", PRIMITIVETYPE_String));
    EXPECT_EQ(ECObjectsStatus::Success, class1->CreatePrimitiveProperty(stringProp3, "S3", PRIMITIVETYPE_String));
    EXPECT_EQ(ECObjectsStatus::Success, class1->CreatePrimitiveProperty(intProp3, "I3", PRIMITIVETYPE_Integer));
    EXPECT_EQ(ECObjectsStatus::Success, class1->CreatePrimitiveProperty(booleanProp3, "B3", PRIMITIVETYPE_Boolean));
    EXPECT_EQ(ECObjectsStatus::Success, class1->CreatePrimitiveProperty(pointProp3, "P3", PRIMITIVETYPE_Point3d));
    EXPECT_EQ(ECObjectsStatus::Success, class1->CreatePrimitiveProperty(dateTimeProp3, "DT3", PRIMITIVETYPE_DateTime));

    ECDBufferTester instance1A(*class1->GetDefaultStandaloneEnabler());
    ECDBufferTester instance1B(*class1->GetDefaultStandaloneEnabler());

    EXPECT_EQ(ECObjectsStatus::Success, instance1A.SetValueToMemory("S1", ECValue("One")));
    EXPECT_EQ(ECObjectsStatus::Success, instance1A.SetValueToMemory("S3", ECValue("Three")));

    const int numIterations = 1000;

    for (int i=0; i<numIterations; i++)
        {
        ECValue v;
        int int1 = 10 * numIterations + i;
        int int2 = 20 * numIterations + i;
        int int3 = 30 * numIterations + i;

        EXPECT_EQ(ECObjectsStatus::Success, instance1A.SetValueToMemory("I1", ECValue(int1)));
        EXPECT_EQ(ECObjectsStatus::Success, instance1A.SetValueToMemory("I2", ECValue(int2)));
        EXPECT_EQ(ECObjectsStatus::Success, instance1A.SetValueToMemory("I3", ECValue(int3)));

        EXPECT_EQ(ECObjectsStatus::Success, instance1A.SetValueToMemory("D1", ECValue((double)int1)));
        EXPECT_EQ(ECObjectsStatus::Success, instance1A.SetValueToMemory("D2", ECValue((double)int2)));
        EXPECT_EQ(ECObjectsStatus::Success, instance1A.SetValueToMemory("D3", ECValue((double)int3)));

        int numArrayMembers = i % 10 + 1;

        EXPECT_EQ(ECObjectsStatus::Success, instance1A.SetArrayLength("A1", numArrayMembers));
        EXPECT_EQ(ECObjectsStatus::Success, instance1A.SetArrayLength("A2", numArrayMembers));
        EXPECT_EQ(ECObjectsStatus::Success, instance1A.SetArrayLength("A3", numArrayMembers));

        for (int j=0; j<numArrayMembers; j++)
            {
            EXPECT_EQ(ECObjectsStatus::Success, instance1A.SetValueToMemory("A1", ECValue((double)j), true, j));
            EXPECT_EQ(ECObjectsStatus::Success, instance1A.SetValueToMemory("A2", ECValue(j), true, j));
            Utf8PrintfString s("s%d", j);
            EXPECT_EQ(ECObjectsStatus::Success, instance1A.SetValueToMemory("A3", ECValue(s.c_str()), true, j));
            }

        DPoint3d point1 = DPoint3d::From(int1, 0, 0);
        DPoint3d point2 = DPoint3d::From(int1, int2, 0);
        DPoint3d point3 = DPoint3d::From(int1, int2, int3);

        EXPECT_EQ(ECObjectsStatus::Success, instance1A.SetValueToMemory("P1", ECValue(point1)));
        EXPECT_EQ(ECObjectsStatus::Success, instance1A.SetValueToMemory("P2", ECValue(point2)));
        EXPECT_EQ(ECObjectsStatus::Success, instance1A.SetValueToMemory("P3", ECValue(point3)));

        switch (i % 3)
            {
            case 0:
                instance1A.SetValueToMemory("B1", ECValue(true));
                instance1A.SetValueToMemory("B2", ECValue(false));
                instance1A.SetValueToMemory("B3", ECValue());
                instance1A.SetValueToMemory("I3", ECValue());
                instance1A.SetValueToMemory("D3", ECValue());
                break;
            case 1:
                instance1A.SetValueToMemory("B1", ECValue());
                instance1A.SetValueToMemory("B2", ECValue(true));
                instance1A.SetValueToMemory("B3", ECValue(false));
                instance1A.SetValueToMemory("I1", ECValue());
                instance1A.SetValueToMemory("D1", ECValue());
                break;
            case 2:
                instance1A.SetValueToMemory("B1", ECValue(false));
                instance1A.SetValueToMemory("B2", ECValue());
                instance1A.SetValueToMemory("B3", ECValue(true));
                instance1A.SetValueToMemory("I2", ECValue());
                instance1A.SetValueToMemory("D2", ECValue());
                break;
            }

        // Copy into instance1B from instance1A
        EXPECT_EQ(ECObjectsStatus::Success, instance1B.CopyFromBuffer(instance1A));

        switch (i % 3)
            {
            case 0:
                EXPECT_EQ(ECObjectsStatus::Success, instance1B.GetValueFromMemory(v, "B1"));
                EXPECT_EQ(v.GetBoolean(), true);
                EXPECT_EQ(ECObjectsStatus::Success, instance1B.GetValueFromMemory(v, "B2"));
                EXPECT_EQ(v.GetBoolean(), false);
                EXPECT_EQ(ECObjectsStatus::Success, instance1B.GetValueFromMemory(v, "B3"));
                EXPECT_TRUE(v.IsNull());
                EXPECT_EQ(ECObjectsStatus::Success, instance1B.GetValueFromMemory(v, "I1"));
                EXPECT_EQ(v.GetInteger(), int1);
                EXPECT_EQ(ECObjectsStatus::Success, instance1B.GetValueFromMemory(v, "I2"));
                EXPECT_EQ(v.GetInteger(), int2);
                EXPECT_EQ(ECObjectsStatus::Success, instance1B.GetValueFromMemory(v, "I3"));
                EXPECT_TRUE(v.IsNull());
                EXPECT_EQ(ECObjectsStatus::Success, instance1B.GetValueFromMemory(v, "D1"));
                EXPECT_EQ(v.GetDouble(), int1);
                EXPECT_EQ(ECObjectsStatus::Success, instance1B.GetValueFromMemory(v, "D2"));
                EXPECT_EQ(v.GetDouble(), int2);
                EXPECT_EQ(ECObjectsStatus::Success, instance1B.GetValueFromMemory(v, "D3"));
                EXPECT_TRUE(v.IsNull());
                break;
            case 1:
                EXPECT_EQ(ECObjectsStatus::Success, instance1B.GetValueFromMemory(v, "B1"));
                EXPECT_TRUE(v.IsNull());
                EXPECT_EQ(ECObjectsStatus::Success, instance1B.GetValueFromMemory(v, "B2"));
                EXPECT_EQ(v.GetBoolean(), true);
                EXPECT_EQ(ECObjectsStatus::Success, instance1B.GetValueFromMemory(v, "B3"));
                EXPECT_EQ(v.GetBoolean(), false);
                EXPECT_EQ(ECObjectsStatus::Success, instance1B.GetValueFromMemory(v, "I1"));
                EXPECT_TRUE(v.IsNull());
                EXPECT_EQ(ECObjectsStatus::Success, instance1B.GetValueFromMemory(v, "I2"));
                EXPECT_EQ(v.GetInteger(), int2);
                EXPECT_EQ(ECObjectsStatus::Success, instance1B.GetValueFromMemory(v, "I3"));
                EXPECT_EQ(v.GetInteger(), int3);
                EXPECT_EQ(ECObjectsStatus::Success, instance1B.GetValueFromMemory(v, "D1"));
                EXPECT_TRUE(v.IsNull());
                EXPECT_EQ(ECObjectsStatus::Success, instance1B.GetValueFromMemory(v, "D2"));
                EXPECT_EQ(v.GetDouble(), int2);
                EXPECT_EQ(ECObjectsStatus::Success, instance1B.GetValueFromMemory(v, "D3"));
                EXPECT_EQ(v.GetDouble(), int3);
                break;
            case 2:
                EXPECT_EQ(ECObjectsStatus::Success, instance1B.GetValueFromMemory(v, "B1"));
                EXPECT_EQ(v.GetBoolean(), false);
                EXPECT_EQ(ECObjectsStatus::Success, instance1B.GetValueFromMemory(v, "B2"));
                EXPECT_TRUE(v.IsNull());
                EXPECT_EQ(ECObjectsStatus::Success, instance1B.GetValueFromMemory(v, "B3"));
                EXPECT_EQ(v.GetBoolean(), true);
                EXPECT_EQ(ECObjectsStatus::Success, instance1B.GetValueFromMemory(v, "I1"));
                EXPECT_EQ(v.GetInteger(), int1);
                EXPECT_EQ(ECObjectsStatus::Success, instance1B.GetValueFromMemory(v, "I2"));
                EXPECT_TRUE(v.IsNull());
                EXPECT_EQ(ECObjectsStatus::Success, instance1B.GetValueFromMemory(v, "I3"));
                EXPECT_EQ(v.GetInteger(), int3);
                EXPECT_EQ(ECObjectsStatus::Success, instance1B.GetValueFromMemory(v, "D1"));
                EXPECT_EQ(v.GetDouble(), int1);
                EXPECT_EQ(ECObjectsStatus::Success, instance1B.GetValueFromMemory(v, "D2"));
                EXPECT_TRUE(v.IsNull());
                EXPECT_EQ(ECObjectsStatus::Success, instance1B.GetValueFromMemory(v, "D3"));
                EXPECT_EQ(v.GetDouble(), int3);
                break;
            }

        EXPECT_EQ(ECObjectsStatus::Success, instance1B.GetValueFromMemory(v, "P1"));
        EXPECT_EQ(v.GetPoint3d(), point1);
        EXPECT_EQ(ECObjectsStatus::Success, instance1B.GetValueFromMemory(v, "P2"));
        EXPECT_EQ(v.GetPoint3d(), point2);
        EXPECT_EQ(ECObjectsStatus::Success, instance1B.GetValueFromMemory(v, "P3"));
        EXPECT_EQ(v.GetPoint3d(), point3);

        EXPECT_EQ(ECObjectsStatus::Success, instance1B.GetValueFromMemory(v, "A1"));
        EXPECT_EQ(v.GetArrayInfo().GetCount(), numArrayMembers);
        EXPECT_EQ(ECObjectsStatus::Success, instance1B.GetValueFromMemory(v, "A2"));
        EXPECT_EQ(v.GetArrayInfo().GetCount(), numArrayMembers);
        EXPECT_EQ(ECObjectsStatus::Success, instance1B.GetValueFromMemory(v, "A3"));
        EXPECT_EQ(v.GetArrayInfo().GetCount(), numArrayMembers);

        for (int j=0; j<numArrayMembers; j++)
            {
            EXPECT_EQ(ECObjectsStatus::Success, instance1B.GetValueFromMemory(v, "A1", true, j));
            EXPECT_EQ(v.GetDouble(), j);
            }

        int64_t long1 = 10000LL + i;
        int64_t long2 = 20000LL + i;
        int64_t long3 = 30000LL + i;

        EXPECT_EQ(ECObjectsStatus::Success, instance1A.SetValueToMemory("L1", ECValue(long1)));
        EXPECT_EQ(ECObjectsStatus::Success, instance1A.SetValueToMemory("L2", ECValue(long2)));
        EXPECT_EQ(ECObjectsStatus::Success, instance1A.SetValueToMemory("L3", ECValue(long3)));

        switch (i % 3)
            {
            case 0:
                EXPECT_EQ(ECObjectsStatus::Success, instance1A.SetValueToMemory("L1", ECValue()));
                break;
            case 1:
                EXPECT_EQ(ECObjectsStatus::Success, instance1A.SetValueToMemory("L2", ECValue()));
                break;
            case 2:
                EXPECT_EQ(ECObjectsStatus::Success, instance1A.SetValueToMemory("L3", ECValue()));
                break;
            }

        // Copy into instance1B from instance1A
        EXPECT_EQ(ECObjectsStatus::Success, instance1B.CopyFromBuffer(instance1A));

        ECValue longValue1;
        ECValue longValue2;
        ECValue longValue3;
        EXPECT_EQ(ECObjectsStatus::Success, instance1B.GetValueFromMemory(longValue1, "L1"));
        EXPECT_EQ(ECObjectsStatus::Success, instance1B.GetValueFromMemory(longValue2, "L2"));
        EXPECT_EQ(ECObjectsStatus::Success, instance1B.GetValueFromMemory(longValue3, "L3"));

        switch (i % 3)
            {
            case 0:
                EXPECT_TRUE(longValue1.IsNull());
                EXPECT_EQ(longValue2.GetLong(), long2);
                EXPECT_EQ(longValue3.GetLong(), long3);
                break;
            case 1:
                EXPECT_EQ(longValue1.GetLong(), long1);
                EXPECT_TRUE(longValue2.IsNull());
                EXPECT_EQ(longValue3.GetLong(), long3);
                break;
            case 2:
                EXPECT_EQ(longValue1.GetLong(), long1);
                EXPECT_EQ(longValue2.GetLong(), long2);
                EXPECT_TRUE(longValue3.IsNull());
                break;
            }

        EXPECT_EQ(ECObjectsStatus::Success, instance1A.GetValueFromMemory(v, "S1"));
        Utf8PrintfString string1("%s%d", v.GetUtf8CP(), i % 10);
        EXPECT_EQ(ECObjectsStatus::Success, instance1A.SetValueToMemory("S1", ECValue(string1.c_str())));

        EXPECT_EQ(ECObjectsStatus::Success, instance1B.GetValueFromMemory(v, "S2"));
        EXPECT_TRUE(v.IsNull());
        
        EXPECT_EQ(ECObjectsStatus::Success, instance1A.GetValueFromMemory(v, "S3"));
        Utf8PrintfString string3("%s----%d", v.GetUtf8CP(), i % 10);
        EXPECT_EQ(ECObjectsStatus::Success, instance1A.SetValueToMemory("S3", ECValue(string3.c_str())));

        // Copy into instance1B from instance1A
        EXPECT_EQ(ECObjectsStatus::Success, instance1B.CopyFromBuffer(instance1A));

        EXPECT_EQ(ECObjectsStatus::Success, instance1B.GetValueFromMemory(v, "S1"));
        EXPECT_STREQ(v.GetUtf8CP(), string1.c_str());
        EXPECT_EQ(ECObjectsStatus::Success, instance1B.GetValueFromMemory(v, "S2"));
        EXPECT_TRUE(v.IsNull());
        EXPECT_EQ(ECObjectsStatus::Success, instance1B.GetValueFromMemory(v, "S3"));
        EXPECT_STREQ(v.GetUtf8CP(), string3.c_str());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDBufferTests, CopyDataBuffer)
    {
    ECSchemaPtr sourceSchema;
    ECSchema::CreateSchema(sourceSchema, "Source", "source", 1, 0, 0);
    EXPECT_TRUE(sourceSchema.IsValid());
    ECEntityClassP sourceClass;
    PrimitiveECPropertyP sourceProp;
    EXPECT_EQ(ECObjectsStatus::Success, sourceSchema->CreateEntityClass(sourceClass, "AbsDuctType"));
    EXPECT_EQ(ECObjectsStatus::Success, sourceClass->CreatePrimitiveProperty(sourceProp, "ModelId", PRIMITIVETYPE_Long));
    EXPECT_EQ(ECObjectsStatus::Success, sourceClass->CreatePrimitiveProperty(sourceProp, "ModelRelClassId", PRIMITIVETYPE_Long));
    EXPECT_EQ(ECObjectsStatus::Success, sourceClass->CreatePrimitiveProperty(sourceProp, "ModelNavPropFlags", PRIMITIVETYPE_Boolean));
    EXPECT_EQ(ECObjectsStatus::Success, sourceClass->CreatePrimitiveProperty(sourceProp, "LastMod", PRIMITIVETYPE_DateTime));
    EXPECT_EQ(ECObjectsStatus::Success, sourceClass->CreatePrimitiveProperty(sourceProp, "CodeSpecId", PRIMITIVETYPE_Long));
    EXPECT_EQ(ECObjectsStatus::Success, sourceClass->CreatePrimitiveProperty(sourceProp, "CodeSpecRelClassId", PRIMITIVETYPE_Long));
    EXPECT_EQ(ECObjectsStatus::Success, sourceClass->CreatePrimitiveProperty(sourceProp, "CodeSpecNavPropFlags", PRIMITIVETYPE_Boolean));
    EXPECT_EQ(ECObjectsStatus::Success, sourceClass->CreatePrimitiveProperty(sourceProp, "CodeScopeId", PRIMITIVETYPE_Long));
    EXPECT_EQ(ECObjectsStatus::Success, sourceClass->CreatePrimitiveProperty(sourceProp, "CodeScopeRelClassId", PRIMITIVETYPE_Long));
    EXPECT_EQ(ECObjectsStatus::Success, sourceClass->CreatePrimitiveProperty(sourceProp, "CodeScopeNavPropFlags", PRIMITIVETYPE_Boolean));
    EXPECT_EQ(ECObjectsStatus::Success, sourceClass->CreatePrimitiveProperty(sourceProp, "ParentId", PRIMITIVETYPE_Long));
    EXPECT_EQ(ECObjectsStatus::Success, sourceClass->CreatePrimitiveProperty(sourceProp, "ParentRelClassId", PRIMITIVETYPE_Long));
    EXPECT_EQ(ECObjectsStatus::Success, sourceClass->CreatePrimitiveProperty(sourceProp, "ParentNavPropFlags", PRIMITIVETYPE_Boolean));
    EXPECT_EQ(ECObjectsStatus::Success, sourceClass->CreatePrimitiveProperty(sourceProp, "IsPrivate", PRIMITIVETYPE_Boolean));
    EXPECT_EQ(ECObjectsStatus::Success, sourceClass->CreatePrimitiveProperty(sourceProp, "RecipeId", PRIMITIVETYPE_Long));
    EXPECT_EQ(ECObjectsStatus::Success, sourceClass->CreatePrimitiveProperty(sourceProp, "RecipeRelClassId", PRIMITIVETYPE_Long));
    EXPECT_EQ(ECObjectsStatus::Success, sourceClass->CreatePrimitiveProperty(sourceProp, "RecipeNavPropFlags", PRIMITIVETYPE_Boolean));
    // PhysicalMaterial specifically excluded to have different offsets between source and target
    EXPECT_EQ(ECObjectsStatus::Success, sourceClass->CreatePrimitiveProperty(sourceProp, "CodeValue", PRIMITIVETYPE_String));
    EXPECT_EQ(ECObjectsStatus::Success, sourceClass->CreatePrimitiveProperty(sourceProp, "UserLabel", PRIMITIVETYPE_String));
    EXPECT_EQ(ECObjectsStatus::Success, sourceClass->CreatePrimitiveProperty(sourceProp, "FederationGuid", PRIMITIVETYPE_Binary));
    EXPECT_EQ(ECObjectsStatus::Success, sourceClass->CreatePrimitiveProperty(sourceProp, "JsonProperties", PRIMITIVETYPE_String));
    EXPECT_EQ(ECObjectsStatus::Success, sourceClass->CreatePrimitiveProperty(sourceProp, "ELEM_CATEGORY_PARAM", PRIMITIVETYPE_String));
    EXPECT_EQ(ECObjectsStatus::Success, sourceClass->CreatePrimitiveProperty(sourceProp, "ALL_MODEL_TYPE_NAME", PRIMITIVETYPE_String));

    ECSchemaPtr targetSchema;
    ECSchema::CreateSchema(targetSchema, "Target", "target", 1, 0, 0);
    EXPECT_TRUE(targetSchema.IsValid());
    ECEntityClassP targetClass;
    PrimitiveECPropertyP targetProp;
    EXPECT_EQ(ECObjectsStatus::Success, targetSchema->CreateEntityClass(targetClass, "AbsDuctType"));
    EXPECT_EQ(ECObjectsStatus::Success, targetClass->CreatePrimitiveProperty(targetProp, "ModelId", PRIMITIVETYPE_Long));
    EXPECT_EQ(ECObjectsStatus::Success, targetClass->CreatePrimitiveProperty(targetProp, "ModelRelClassId", PRIMITIVETYPE_Long));
    EXPECT_EQ(ECObjectsStatus::Success, targetClass->CreatePrimitiveProperty(targetProp, "ModelNavPropFlags", PRIMITIVETYPE_Boolean));
    EXPECT_EQ(ECObjectsStatus::Success, targetClass->CreatePrimitiveProperty(targetProp, "LastMod", PRIMITIVETYPE_DateTime));
    EXPECT_EQ(ECObjectsStatus::Success, targetClass->CreatePrimitiveProperty(targetProp, "CodeSpecId", PRIMITIVETYPE_Long));
    EXPECT_EQ(ECObjectsStatus::Success, targetClass->CreatePrimitiveProperty(targetProp, "CodeSpecRelClassId", PRIMITIVETYPE_Long));
    EXPECT_EQ(ECObjectsStatus::Success, targetClass->CreatePrimitiveProperty(targetProp, "CodeSpecNavPropFlags", PRIMITIVETYPE_Boolean));
    EXPECT_EQ(ECObjectsStatus::Success, targetClass->CreatePrimitiveProperty(targetProp, "CodeScopeId", PRIMITIVETYPE_Long));
    EXPECT_EQ(ECObjectsStatus::Success, targetClass->CreatePrimitiveProperty(targetProp, "CodeScopeRelClassId", PRIMITIVETYPE_Long));
    EXPECT_EQ(ECObjectsStatus::Success, targetClass->CreatePrimitiveProperty(targetProp, "CodeScopeNavPropFlags", PRIMITIVETYPE_Boolean));
    EXPECT_EQ(ECObjectsStatus::Success, targetClass->CreatePrimitiveProperty(targetProp, "ParentId", PRIMITIVETYPE_Long));
    EXPECT_EQ(ECObjectsStatus::Success, targetClass->CreatePrimitiveProperty(targetProp, "ParentRelClassId", PRIMITIVETYPE_Long));
    EXPECT_EQ(ECObjectsStatus::Success, targetClass->CreatePrimitiveProperty(targetProp, "ParentNavPropFlags", PRIMITIVETYPE_Boolean));
    EXPECT_EQ(ECObjectsStatus::Success, targetClass->CreatePrimitiveProperty(targetProp, "IsPrivate", PRIMITIVETYPE_Boolean));
    EXPECT_EQ(ECObjectsStatus::Success, targetClass->CreatePrimitiveProperty(targetProp, "RecipeId", PRIMITIVETYPE_Long));
    EXPECT_EQ(ECObjectsStatus::Success, targetClass->CreatePrimitiveProperty(targetProp, "RecipeRelClassId", PRIMITIVETYPE_Long));
    EXPECT_EQ(ECObjectsStatus::Success, targetClass->CreatePrimitiveProperty(targetProp, "RecipeNavPropFlags", PRIMITIVETYPE_Boolean));
    EXPECT_EQ(ECObjectsStatus::Success, targetClass->CreatePrimitiveProperty(targetProp, "PhysicalMaterialId", PRIMITIVETYPE_Long));
    EXPECT_EQ(ECObjectsStatus::Success, targetClass->CreatePrimitiveProperty(targetProp, "PhysicalMaterialRelClassId", PRIMITIVETYPE_Long));
    EXPECT_EQ(ECObjectsStatus::Success, targetClass->CreatePrimitiveProperty(targetProp, "PhysicalMaterialNavPropFlags", PRIMITIVETYPE_Boolean));
    EXPECT_EQ(ECObjectsStatus::Success, targetClass->CreatePrimitiveProperty(targetProp, "CodeValue", PRIMITIVETYPE_String));
    EXPECT_EQ(ECObjectsStatus::Success, targetClass->CreatePrimitiveProperty(targetProp, "UserLabel", PRIMITIVETYPE_String));
    EXPECT_EQ(ECObjectsStatus::Success, targetClass->CreatePrimitiveProperty(targetProp, "FederationGuid", PRIMITIVETYPE_Binary));
    EXPECT_EQ(ECObjectsStatus::Success, targetClass->CreatePrimitiveProperty(targetProp, "JsonProperties", PRIMITIVETYPE_String));
    EXPECT_EQ(ECObjectsStatus::Success, targetClass->CreatePrimitiveProperty(targetProp, "ELEM_CATEGORY_PARAM", PRIMITIVETYPE_String));
    EXPECT_EQ(ECObjectsStatus::Success, targetClass->CreatePrimitiveProperty(targetProp, "ALL_MODEL_TYPE_NAME", PRIMITIVETYPE_String));

    ECDBufferTester sourceInstance(*sourceClass->GetDefaultStandaloneEnabler());
    ECDBufferTester targetInstance(*targetClass->GetDefaultStandaloneEnabler());

    EXPECT_EQ(ECObjectsStatus::Success, sourceInstance.SetValueToMemory("ELEM_CATEGORY_PARAM", ECValue("Ducts")));
    EXPECT_EQ(ECObjectsStatus::Success, sourceInstance.SetValueToMemory("ALL_MODEL_TYPE_NAME", ECValue("Default")));

    EXPECT_EQ(ECObjectsStatus::Success, targetInstance.CopyDataBuffer(sourceInstance, true));

    ECValue v;
    EXPECT_EQ(ECObjectsStatus::Success, targetInstance.GetValueFromMemory(v, "CodeValue"));
    EXPECT_TRUE(v.IsNull());
    EXPECT_EQ(ECObjectsStatus::Success, targetInstance.GetValueFromMemory(v, "UserLabel"));
    EXPECT_TRUE(v.IsNull());
    EXPECT_EQ(ECObjectsStatus::Success, targetInstance.GetValueFromMemory(v, "FederationGuid"));
    EXPECT_TRUE(v.IsNull());
    EXPECT_EQ(ECObjectsStatus::Success, targetInstance.GetValueFromMemory(v, "JsonProperties"));
    EXPECT_TRUE(v.IsNull());
    EXPECT_EQ(ECObjectsStatus::Success, targetInstance.GetValueFromMemory(v, "ELEM_CATEGORY_PARAM"));
    EXPECT_STREQ(v.GetUtf8CP(), "Ducts");
    EXPECT_EQ(ECObjectsStatus::Success, targetInstance.GetValueFromMemory(v, "ALL_MODEL_TYPE_NAME"));
    EXPECT_STREQ(v.GetUtf8CP(), "Default");
    }

END_BENTLEY_ECN_TEST_NAMESPACE
