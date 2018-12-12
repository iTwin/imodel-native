/*--------------------------------------------------------------------------------------+
|
|     $Source: test/Published/ECDBufferTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"

using namespace BentleyApi::ECN;

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   04/13
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
* @bsimethod                                                    JoshSchifter    12/09
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
* @bsimethod                                                    JoshSchifter    12/09
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
* @bsimethod                                                    Paul.Connelly   04/13
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
* @bsimethod                                                    Paul.Connelly   04/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDBufferTests, ClearArray)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "ArrayTest", "ts", 1, 0, 0);
    ECEntityClassP ecClass;
    schema->CreateEntityClass(ecClass, "TestClass");
    PrimitiveECPropertyP primProp;
    ecClass->CreatePrimitiveProperty(primProp, "Int", PRIMITIVETYPE_Integer);
    ecClass->CreatePrimitiveProperty(primProp, "String");

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
* @bsimethod                                                    Paul.Connelly   02/14
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
* @bsimethod                                                    Paul.Connelly   05/13
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
* @bsimethod                                                    Paul.Connelly   05/13
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
* @bsimethod                                                    Paul.Connelly   05/13
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
* intact, but should not copy the struct array instances themselves.
* @bsimethod                                                    Paul.Connelly   05/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECDBufferTests, ConvertDataBuffer_StructArrays)
    {
    ECSchemaPtr schema1;
    ECSchema::CreateSchema(schema1, "Schema", "ts", 1, 0, 0);
    ECStructClassP struct1;
    schema1->CreateStructClass(struct1, "Struct");
    PrimitiveECPropertyP primProp;
    struct1->CreatePrimitiveProperty(primProp, "String", PRIMITIVETYPE_String);

    ECEntityClassP class1;
    schema1->CreateEntityClass(class1, "Class");
    StructArrayECPropertyP structArrayProp;
    class1->CreateStructArrayProperty(structArrayProp, "StructArray", *struct1);

    StandaloneECInstancePtr instance1 = class1->GetDefaultStandaloneEnabler()->CreateInstance();
    instance1->AddArrayElements("StructArray", 2);
    IECInstancePtr structInstance = struct1->GetDefaultStandaloneEnabler()->CreateInstance();
    ECValue structVal;
    structVal.SetStruct(structInstance.get());
    instance1->SetValue("StructArray", structVal, 0);
    structInstance = struct1->GetDefaultStandaloneEnabler()->CreateInstance();
    structVal.SetStruct(structInstance.get());
    instance1->SetValue("StructArray", structVal, 1);

    ECSchemaPtr schema2;
    ECSchema::CreateSchema(schema2, "Schema", "ts", 2, 0, 0);
    ECStructClassP struct2;
    schema2->CreateStructClass(struct2, "Struct");
    struct2->CreatePrimitiveProperty(primProp, "String", PRIMITIVETYPE_Integer);

    ECEntityClassP class2;
    schema2->CreateEntityClass(class2, "Class");
    class2->CreatePrimitiveProperty(primProp, "Stuff", PRIMITIVETYPE_String);
    class2->CreateStructArrayProperty(structArrayProp, "StructArray", *struct2);

    StandaloneECInstancePtr instance2 = class2->GetDefaultStandaloneEnabler()->CreateInstance();
    EXPECT_EQ(ECObjectsStatus::Success, instance2->CopyDataBuffer(*instance1, true));

    ECValue arrayVal;
    arrayVal.SetStructArrayInfo(2, false);
    TestValue(*instance2, "StructArray", arrayVal);
#ifdef STRUCT_ENTRIES_COPIED
    // this used to expect the struct array entries to be null, which happened to work due to a bug in the copying code
    // After fixing the bug the struct entry IDs are copied, but (as advertised in method documentation) the struct array ECInstances themselves
    // are not, so trying to access them with invalid ID raises an error.
    TestValue(*instance2, "StructArray", ECValue(/*null*/), 0);
    TestValue(*instance2, "StructArray", ECValue(/*null*/), 1);
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/13
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

END_BENTLEY_ECN_TEST_NAMESPACE
