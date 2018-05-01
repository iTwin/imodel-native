/*--------------------------------------------------------------------------------------+
|
|     $Source: test/Published/InstanceSerializationTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"

#include "BeXml/BeXml.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

struct InstanceDeserializationTest      : ECTestFixture {};
struct PolymorphismDeserializationTest  : ECTestFixture {};
struct InstanceSerializationTest        : ECTestFixture {};

struct  Struct1
    {
    bool    struct1BoolMember;
    int     struct1IntMember;
    };

struct  Struct2
    {
    bool            struct2StringMemberNull;
    Utf8CP          struct2StringMember;
    bool            struct2DoubleMemberNull;
    double          struct2DoubleMember;
    Struct1*        nestedArray;
    uint32_t        arraySize;
    };


// set the binary data member, and then
static Byte testBinaryData[] = { 0, 255,  1, 254,  2, 253,  3, 252,  4, 251,  5, 250,  6, 249,  7, 248,  8, 247,  9, 246, 10, 245, 11, 244, 12, 243, 13, 242, 14, 241, 15, 240, 16, 239, 17, 238, 18, 237,
                                19, 236, 20, 235, 21, 234, 22, 233, 23, 232, 24, 231, 25, 230, 26, 229, 27, 228, 28, 227, 29, 226, 30, 225, 31, 224, 32, 223, 33, 222, 34, 221, 35, 220, 36, 219, 37, 218,
                                38, 217, 39, 216, 40, 215, 41, 214, 42, 213, 43, 212, 44, 211, 45, 210, 46, 209, 47, 208, 48, 207, 49, 206, 50, 205, 51, 204, 52, 203, 53, 202, 54, 201, 55, 200, 56, 199, 
                                57 };

struct InSchemaClassLocater final : ECN::IECClassLocater
    {
    private:
        ECN::ECSchemaCR m_schema;

        ECN::ECClassCP _LocateClass(Utf8CP schemaName, Utf8CP className) override { return m_schema.GetClassCP(className); }
    public:
        explicit InSchemaClassLocater(ECN::ECSchemaCR schema) : m_schema(schema) {}
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/10
+---------------+---------------+---------------+---------------+---------------+------*/
void    VerifyTestInstance (IECInstanceCP testInstance, bool checkBinaryProperty = true, bool isJson = false)
    {
    ECValue ecValue;

    EXPECT_EQ (ECObjectsStatus::Success, testInstance->GetValue (ecValue, "BaseClassMember"));
    EXPECT_EQ (987, ecValue.GetInteger()) << "Expect 987 for integer value";

    EXPECT_EQ (ECObjectsStatus::Success, testInstance->GetValue (ecValue, "IntegerMember"));
    EXPECT_EQ (12, ecValue.GetInteger()) << "Expected 12 for integer value";

    EXPECT_EQ (ECObjectsStatus::Success, testInstance->GetValue (ecValue, "CustomFormatInt"));
    EXPECT_EQ (13, ecValue.GetInteger()) << "Expected 13 for integer value";

    EXPECT_EQ (ECObjectsStatus::Success, testInstance->GetValue (ecValue, "BooleanMember"));
    EXPECT_FALSE (ecValue.GetBoolean()) << "Expected false for booleanmember";

    EXPECT_EQ (ECObjectsStatus::Success, testInstance->GetValue (ecValue, "DoubleMember"));
    EXPECT_EQ (456.789, ecValue.GetDouble()) << "Expected 456.789 for double member";

    EXPECT_EQ (ECObjectsStatus::Success, testInstance->GetValue (ecValue, "StringMember"));
    EXPECT_STREQ (ecValue.GetUtf8CP(), "Test string") << "Expected 'test string'";

    EXPECT_EQ (ECObjectsStatus::Success, testInstance->GetValue (ecValue, "FileNameMember"));
    EXPECT_STREQ (ecValue.GetUtf8CP(), "c:\\usr\\barry\\test.txt") << "Wrong value for FileNameMember";

    EXPECT_EQ (ECObjectsStatus::Success, testInstance->GetValue (ecValue, "NegativeMember"));
    EXPECT_EQ (-42, ecValue.GetInteger()) << "Expected -42 for negativemember";

    EXPECT_EQ (ECObjectsStatus::Success, testInstance->GetValue (ecValue, "DateTimeMember"));
    if (isJson)
        {
        uint64_t julianDay;
        EXPECT_EQ(BentleyStatus::SUCCESS, ecValue.GetDateTime().ToJulianDay(julianDay)) << "Failed to translate DateTimeMember to Unix Milliseconds";
        EXPECT_EQ(DateTime::CommonEraMillisecondsToJulianDay(63337468146666), julianDay) << "Wrong value for DateTimeMember";
        }
    else
        EXPECT_EQ(633374681466664305, ecValue.GetDateTimeTicks()) << "Wrong value for DateTimeMember";

    EXPECT_EQ (ECObjectsStatus::Success, testInstance->GetValue (ecValue, "StartPoint"));
    EXPECT_EQ (1.1, ecValue.GetPoint3d().x) << "Incorrect x value for StartPoint";
    EXPECT_EQ (2.2, ecValue.GetPoint3d().y) << "Incorrect y value for StartPoint";
    EXPECT_EQ (3.3, ecValue.GetPoint3d().z) << "Incorrect z value for StartPoint";

    EXPECT_EQ (ECObjectsStatus::Success, testInstance->GetValue (ecValue, "EndPoint"));
    EXPECT_EQ (4.4, ecValue.GetPoint3d().x) << "Incorrect x value for EndPoint";
    EXPECT_EQ (7.7, ecValue.GetPoint3d().y) << "Incorrect y value for EndPoint";
    EXPECT_EQ (6.6, ecValue.GetPoint3d().z) << "Incorrect z value for EndPoint";

    EXPECT_EQ (ECObjectsStatus::Success, testInstance->GetValue (ecValue, "SecondEmbeddedStruct.Struct1BoolMember"));
    EXPECT_FALSE (ecValue.GetBoolean()) << "Wrong value for Struct1BoolMember";

    EXPECT_EQ (ECObjectsStatus::Success, testInstance->GetValue (ecValue, "SecondEmbeddedStruct.Struct1IntMember"));
    EXPECT_EQ (4, ecValue.GetInteger()) << "Wrong value for Struct1IntMember";

    EXPECT_EQ (ECObjectsStatus::Success, testInstance->GetValue (ecValue, "FormattedStruct.Struct3DoubleMember"));
    EXPECT_EQ (17.443, ecValue.GetDouble()) << "Wrong value for Struct3DoubleMember";

    EXPECT_EQ (ECObjectsStatus::Success, testInstance->GetValue (ecValue, "FormattedStruct.Struct3IntMember"));
    EXPECT_EQ (531992, ecValue.GetInteger()) << "Wrong value for Struct3IntMember";
    
    EXPECT_EQ (ECObjectsStatus::Success, testInstance->GetValue (ecValue, "FormattedStruct.Struct3BoolMember"));
    EXPECT_TRUE (ecValue.GetBoolean()) << "Wrong value for Struct3BoolMember";

    int         expectedInts[] = {0, 101, 202, 303, 404, 505, 606, 707, 808, 909};
    for (uint32_t index=0; index < _countof (expectedInts); index++)
        {
        EXPECT_EQ (ECObjectsStatus::Success, testInstance->GetValue (ecValue, "FormattedArray", index));
        EXPECT_EQ (expectedInts[index], ecValue.GetInteger()) << "Wrong Value for FormattedArray";
        }

    int         moreInts[] = {41556, 32757, 56789, 32757, 21482 };
    for (uint32_t index=0; index < _countof (moreInts); index++)
        {
        EXPECT_EQ (ECObjectsStatus::Success, testInstance->GetValue (ecValue, "IntArray", index));
        EXPECT_EQ (moreInts[index], ecValue.GetInteger()) << "Wrong value for IntArray";
        }

    EXPECT_EQ (ECObjectsStatus::Success, testInstance->GetValue (ecValue, "OneMemberIntArray", 0));
    EXPECT_EQ (3, ecValue.GetInteger()) << "Wrong value for OneMemberIntArray";

    DPoint3d    expectedPoints[] = { {4.0, 0.0, 0.0}, {1.0, 1.0, 1.0}, {2.0, 2.0, 2.0}, {3.0, 3.0, 3.0}, {4.0, 4.0, 4.0}, {5.0, 5.0, 5.0} };
    for (uint32_t index=0; index < _countof (expectedPoints); index++)
        {
        EXPECT_EQ (ECObjectsStatus::Success, testInstance->GetValue (ecValue, "PointArray", index));
        EXPECT_EQ (expectedPoints[index].x, ecValue.GetPoint3d().x) << "Wrong value for PointArray x";
        EXPECT_EQ (expectedPoints[index].y, ecValue.GetPoint3d().y) << "Wrong value for PointArray y";
        EXPECT_EQ (expectedPoints[index].z, ecValue.GetPoint3d().z) << "Wrong value for PointArray z";
        }

    for (uint32_t index=0; index < 300; index++)
        {
        EXPECT_EQ (ECObjectsStatus::Success, testInstance->GetValue (ecValue, "DateArray", index)) << "Wrong value for DateArray";
        }

    for (uint32_t index=0; index < 300; index++)
        {
        EXPECT_EQ (ECObjectsStatus::Success, testInstance->GetValue (ecValue, "StringArray", index));
        Utf8String expectedString;
        expectedString.Sprintf ("String %d", index%30);
        EXPECT_STREQ (expectedString.c_str(), ecValue.GetUtf8CP()) << "Wrong value for StringArray";
        }

    for (uint32_t index=0; index < 100; index++)
        {
        EXPECT_EQ (ECObjectsStatus::Success, testInstance->GetValue (ecValue, "SmallIntArray", index));
        EXPECT_EQ (index, ecValue.GetInteger()) << "Wrong value for SmallIntArray";
        }

    // test the structArray of Struct2's that contains a nested array of Struct1's. Here's the data we expect:
    Struct1  firstArray[] =
        { { true, 0 }, { false, 10 }, {true, 20}, {false, 30} };

    Struct1  secondArray[] =
        { { false, 0 }, { true, 101 }, { false, 202 }, {true, 303}, {false, 404} };

    Struct2 struct2ExpectedValues[] =
        {
            {
            false,
            "testInstance.StructArray[0].Struct2StringMember",
            false,
            123456.789,
            firstArray,
            _countof (firstArray),
            },
            {
            false,
            "testInstance.StructArray[1].Struct2StringMember",
            false,
            23456.789,
            secondArray,          
            _countof (secondArray),
            },
            {
            false,
            "Added.Struct2StringMember",
            true,
            0.0,
            NULL,
            0
            },
        };

    for (uint32_t index=0; index < _countof (struct2ExpectedValues); index++)
        {
        ECValue         structArrayMember;
        EXPECT_EQ (ECObjectsStatus::Success, testInstance->GetValue (structArrayMember, "StructArray", index));
        IECInstancePtr  structArrayInstance = structArrayMember.GetStruct();
        EXPECT_TRUE (structArrayInstance.IsValid());
        
        if (struct2ExpectedValues[index].struct2StringMemberNull)
            {
            // This throws an assert rather than returning ERROR.
            // EXPECT_EQ (ERROR, structArrayInstance->GetString (stringValue, "Struct2StringMember"));
            }
        else
            {
            EXPECT_EQ (ECObjectsStatus::Success, structArrayInstance->GetValue (ecValue, "Struct2StringMember"));
            EXPECT_STREQ (struct2ExpectedValues[index].struct2StringMember, ecValue.GetUtf8CP()) << "Wrong value for Struct2StringMember";
            }

        if (struct2ExpectedValues[index].struct2DoubleMemberNull)
            {
            // This throws an assert rather than returning ERROR.
            // EXPECT_EQ (ERROR, structArrayInstance->GetDouble (doubleValue, "Struct2DoubleMember"));
            }
        else
            {
            EXPECT_EQ (ECObjectsStatus::Success, structArrayInstance->GetValue (ecValue, "Struct2DoubleMember"));
            EXPECT_EQ (struct2ExpectedValues[index].struct2DoubleMember, ecValue.GetDouble()) << "Wrong value for Struct2DoubleMember";
            }

        // now try to get the array of structs, if expected.
        if (NULL == struct2ExpectedValues[index].nestedArray)
            continue;

        for (uint32_t nestedIndex=0; nestedIndex < struct2ExpectedValues[index].arraySize; nestedIndex++)
            {
            ECValue     nestedArrayMember;
            EXPECT_EQ (ECObjectsStatus::Success, structArrayInstance->GetValue (nestedArrayMember, "NestedArray", nestedIndex));
            IECInstancePtr  nestedInstance = nestedArrayMember.GetStruct();
            EXPECT_TRUE (nestedInstance.IsValid());
            
            EXPECT_EQ (ECObjectsStatus::Success, nestedInstance->GetValue (ecValue, "Struct1BoolMember"));
            EXPECT_EQ (struct2ExpectedValues[index].nestedArray[nestedIndex].struct1BoolMember, ecValue.GetBoolean()) << "Wrong value for Struct1BoolMember in NestedArray";

            EXPECT_EQ (ECObjectsStatus::Success, nestedInstance->GetValue (ecValue, "Struct1IntMember"));
            EXPECT_EQ (struct2ExpectedValues[index].nestedArray[nestedIndex].struct1IntMember, ecValue.GetInteger()) << "Wrong value for Struct1IntMember in NestedArray";
            }
        }

    if (!checkBinaryProperty)
        return;

    ECValue         binaryMember;
    EXPECT_EQ (ECObjectsStatus::Success, testInstance->GetValue (binaryMember, "TestBinary"));

    size_t      numBytes;
    const Byte* byteData; 
    if (NULL != (byteData = binaryMember.GetBinary (numBytes)))
        {
        EXPECT_EQ (_countof (testBinaryData), numBytes);
        for (size_t index=0; index < _countof (testBinaryData); index++)
            EXPECT_EQ (testBinaryData[index], byteData[index]);
        }
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                  Caleb.Shafer                   01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
#ifdef NOT_NOW
TEST_F(InstanceDeserializationTest, ExpectSuccessWhenDeserializingFaultyInstance)
    {
    //This test will verify that the xml deserializer will not fail on mismatched or malformed properties.
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();

    ECSchemaPtr schema;
    SchemaReadStatus schemaStatus = ECSchema::ReadFromXmlFile (schema, ECTestFixture::GetTestDataPath(L"MismatchedSchema.01.00.ecschema.xml").c_str(), *schemaContext);

    EXPECT_EQ (SchemaReadStatus::Success, schemaStatus);

    ECInstanceReadContextPtr instanceContext = ECInstanceReadContext::CreateContext (*schema);

    IECInstancePtr  testInstance;
    InstanceReadStatus instanceStatus;

    instanceStatus = IECInstance::ReadFromXmlFile (testInstance, ECTestFixture::GetTestDataPath(L"MismatchedInstance.xml").c_str(), *instanceContext);

    EXPECT_EQ (InstanceReadStatus::Success, instanceStatus);
    //Testing a struct with type errors in its values
    ECValue ecValue;
    EC_EXPECT_SUCCESS(testInstance->GetValue (ecValue, "FormattedStructWithArrays0.BinaryArray",0));

    EC_EXPECT_SUCCESS(testInstance->GetValue (ecValue, "FormattedStructWithArrays0.BooleanArray",0));
    EXPECT_TRUE (ecValue.GetBoolean());

    EC_EXPECT_SUCCESS(testInstance->GetValue (ecValue, "FormattedStructWithArrays0.DateTimeArray",0));
    EXPECT_EQ (633374681466664305, ecValue.GetDateTimeTicks());

    EC_EXPECT_SUCCESS(testInstance->GetValue (ecValue, "FormattedStructWithArrays0.DoubleArray",0));
    EXPECT_EQ (1.2, ecValue.GetDouble());

    EC_EXPECT_SUCCESS(testInstance->GetValue (ecValue, "FormattedStructWithArrays0.IntArray",1));
    EXPECT_EQ (7, ecValue.GetInteger());

    EC_EXPECT_SUCCESS(testInstance->GetValue (ecValue, "FormattedStructWithArrays0.LongArray",0));
    EXPECT_EQ (300, ecValue.GetLong());

    EC_EXPECT_SUCCESS(testInstance->GetValue (ecValue, "FormattedStructWithArrays0.Point2dArray",0));
    EXPECT_EQ (0, ecValue.GetPoint2d().x);
    EXPECT_EQ (1, ecValue.GetPoint2d().y);

    EC_EXPECT_SUCCESS(testInstance->GetValue (ecValue, "FormattedStructWithArrays0.Point3dArray",0));
    EXPECT_EQ (0, ecValue.GetPoint3d().x);
    EXPECT_EQ (1, ecValue.GetPoint3d().y);
    EXPECT_EQ (0, ecValue.GetPoint3d().z);

    EC_EXPECT_SUCCESS(testInstance->GetValue (ecValue, "FormattedStructWithArrays0.StringArray",0));
    EXPECT_STREQ (ecValue.GetUtf8CP(), "Test");

    // Testing a struct that misformatted values
    EC_EXPECT_SUCCESS(testInstance->GetValue (ecValue, "FormattedStructWithArrays1.BinaryArray",0));

    EC_EXPECT_SUCCESS(testInstance->GetValue (ecValue, "FormattedStructWithArrays1.BooleanArray",0));
    EXPECT_TRUE (ecValue.GetBoolean());

    EC_EXPECT_SUCCESS(testInstance->GetValue (ecValue, "FormattedStructWithArrays1.DateTimeArray",0));
    EXPECT_EQ (633374681466664305, ecValue.GetDateTimeTicks());

    EC_EXPECT_SUCCESS(testInstance->GetValue (ecValue, "FormattedStructWithArrays1.DoubleArray",0));
    EXPECT_EQ (1.2, ecValue.GetDouble());

    EC_EXPECT_SUCCESS(testInstance->GetValue (ecValue, "FormattedStructWithArrays1.IntArray",0));
    EXPECT_EQ (7, ecValue.GetInteger());

    EC_EXPECT_SUCCESS(testInstance->GetValue (ecValue, "FormattedStructWithArrays1.LongArray",0));
    EXPECT_EQ (300, ecValue.GetLong());

    EC_EXPECT_SUCCESS(testInstance->GetValue (ecValue, "FormattedStructWithArrays1.Point2dArray",0));
    EXPECT_EQ (0, ecValue.GetPoint2d().x);
    EXPECT_EQ (1, ecValue.GetPoint2d().y);

    EC_EXPECT_SUCCESS(testInstance->GetValue (ecValue, "FormattedStructWithArrays1.Point3dArray",0));
    EXPECT_EQ (0, ecValue.GetPoint3d().x);
    EXPECT_EQ (1, ecValue.GetPoint3d().y);
    EXPECT_EQ (0, ecValue.GetPoint3d().z);

    EC_EXPECT_SUCCESS(testInstance->GetValue (ecValue, "FormattedStructWithArrays1.StringArray",0));
    EXPECT_STREQ (ecValue.GetUtf8CP(), "");
    EC_EXPECT_SUCCESS(testInstance->GetValue (ecValue, "FormattedStructWithArrays1.StringArray",6));
    EXPECT_STREQ(ecValue.GetUtf8CP(), "Test");

    //Testing a struct with few errors among entirely bad structs in the same array
    ECValue badStructArrayMember;
    EC_EXPECT_SUCCESS(testInstance->GetValue (badStructArrayMember, "ArrayOfBadStructs",2));
    IECInstancePtr  structInstance = badStructArrayMember.GetStruct();
    EXPECT_TRUE (structInstance.IsValid());

    EC_EXPECT_SUCCESS(structInstance->GetValue (ecValue, "BinaryMember"));

    EC_EXPECT_SUCCESS(structInstance->GetValue (ecValue, "BooleanMember"));
    EXPECT_TRUE (ecValue.GetBoolean());

    EC_EXPECT_SUCCESS(structInstance->GetValue (ecValue, "DateTimeMember"));
    EXPECT_EQ (633374681466664305, ecValue.GetDateTimeTicks());

    EC_EXPECT_SUCCESS(structInstance->GetValue (ecValue, "DoubleMember"));
    EXPECT_EQ (1.2, ecValue.GetDouble());

    EC_EXPECT_SUCCESS(structInstance->GetValue (ecValue, "IntMember"));
    EXPECT_EQ (7, ecValue.GetInteger());

    EC_EXPECT_SUCCESS(structInstance->GetValue (ecValue, "LongMember"));
    EXPECT_EQ (300, ecValue.GetLong());

    EC_EXPECT_SUCCESS(structInstance->GetValue (ecValue, "Point2dMember"));
    EXPECT_EQ (0, ecValue.GetPoint2d().x);
    EXPECT_EQ (1, ecValue.GetPoint2d().y);

    EC_EXPECT_SUCCESS(structInstance->GetValue (ecValue, "Point3dMember"));
    EXPECT_EQ (0, ecValue.GetPoint3d().x);
    EXPECT_EQ (1, ecValue.GetPoint3d().y);
    EXPECT_EQ (0, ecValue.GetPoint3d().z);

    EC_EXPECT_SUCCESS(structInstance->GetValue (ecValue, "StringMember"));
    EXPECT_STREQ (ecValue.GetUtf8CP(), "Test");
    };
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                10/2011
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(InstanceDeserializationTest, ExpectSuccessWhenDeserializingInstanceWithEmptyArrayMemebers)
    {
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();

    ECSchemaPtr schema;
    SchemaReadStatus schemaStatus = ECSchema::ReadFromXmlFile (schema, ECTestFixture::GetTestDataPath(L"MismatchedSchema.01.00.ecschema.xml").c_str(), *schemaContext);

    EXPECT_EQ (SchemaReadStatus::Success, schemaStatus);

    ECInstanceReadContextPtr instanceContext = ECInstanceReadContext::CreateContext (*schema);

    IECInstancePtr  testInstance;
    InstanceReadStatus instanceStatus;

    instanceStatus = IECInstance::ReadFromXmlFile (testInstance, ECTestFixture::GetTestDataPath(L"EmptyArrayInstance.xml").c_str(), *instanceContext);

    
    EXPECT_EQ (InstanceReadStatus::Success, instanceStatus);
    //Testing a struct with type errors in its values
    ECValue ecValue;
    EXPECT_EQ (ECObjectsStatus::Success, testInstance->GetValue (ecValue, "FormattedStructWithArrays1.BinaryArray",0));

    EXPECT_EQ (ECObjectsStatus::Success, testInstance->GetValue (ecValue, "FormattedStructWithArrays1.BooleanArray",1));
    EXPECT_TRUE (ecValue.IsNull());
    EXPECT_EQ (ECObjectsStatus::Success, testInstance->GetValue (ecValue, "FormattedStructWithArrays1.BooleanArray",0));
    EXPECT_TRUE (ecValue.GetBoolean());

    EXPECT_EQ (ECObjectsStatus::Success, testInstance->GetValue (ecValue, "FormattedStructWithArrays1.DateTimeArray",0));
    EXPECT_TRUE (ecValue.IsNull());
    EXPECT_EQ (ECObjectsStatus::Success, testInstance->GetValue (ecValue, "FormattedStructWithArrays1.DateTimeArray",1));
    EXPECT_EQ (633374681466664305, ecValue.GetDateTimeTicks());

    EXPECT_EQ (ECObjectsStatus::Success, testInstance->GetValue (ecValue, "FormattedStructWithArrays1.DoubleArray",0));
    EXPECT_TRUE (ecValue.IsNull());
    EXPECT_EQ (ECObjectsStatus::Success, testInstance->GetValue (ecValue, "FormattedStructWithArrays1.DoubleArray",1));
    EXPECT_EQ (1.2, ecValue.GetDouble());

    EXPECT_EQ (ECObjectsStatus::Success, testInstance->GetValue (ecValue, "FormattedStructWithArrays1.IntArray",0));
    EXPECT_TRUE (ecValue.IsNull());
    EXPECT_EQ (ECObjectsStatus::Success, testInstance->GetValue (ecValue, "FormattedStructWithArrays1.IntArray",1));
    EXPECT_EQ (7, ecValue.GetInteger());

    EXPECT_EQ (ECObjectsStatus::Success, testInstance->GetValue (ecValue, "FormattedStructWithArrays1.LongArray",0));
    EXPECT_TRUE (ecValue.IsNull());
    EXPECT_EQ (ECObjectsStatus::Success, testInstance->GetValue (ecValue, "FormattedStructWithArrays1.LongArray",1));
    EXPECT_EQ (300, ecValue.GetLong());

    EXPECT_EQ (ECObjectsStatus::Success, testInstance->GetValue (ecValue, "FormattedStructWithArrays1.Point2DArray",0));
    EXPECT_TRUE (ecValue.IsNull());
    EXPECT_EQ (ECObjectsStatus::Success, testInstance->GetValue (ecValue, "FormattedStructWithArrays1.Point2DArray",1));
    EXPECT_EQ (0, ecValue.GetPoint2d().x);
    EXPECT_EQ (1, ecValue.GetPoint2d().y);

    EXPECT_EQ (ECObjectsStatus::Success, testInstance->GetValue (ecValue, "FormattedStructWithArrays1.Point3DArray",0));
    EXPECT_TRUE (ecValue.IsNull());
    EXPECT_EQ (ECObjectsStatus::Success, testInstance->GetValue (ecValue, "FormattedStructWithArrays1.Point3DArray",1));
    EXPECT_EQ (0, ecValue.GetPoint3d().x);
    EXPECT_EQ (1, ecValue.GetPoint3d().y);
    EXPECT_EQ (0, ecValue.GetPoint3d().z);

    EXPECT_EQ (ECObjectsStatus::Success, testInstance->GetValue (ecValue, "FormattedStructWithArrays1.StringArray",0));
    EXPECT_STREQ (ecValue.GetUtf8CP(), "");
    EXPECT_EQ (ECObjectsStatus::Success, testInstance->GetValue (ecValue, "FormattedStructWithArrays1.StringArray",6));
    EXPECT_STREQ (ecValue.GetUtf8CP(), "Test");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                Caleb.Shafer                   01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(InstanceDeserializationTest, ExpectSuccessWhenDeserializingSimpleInstance)
    {
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();

    ECSchemaPtr schema;
    SchemaReadStatus schemaStatus = ECSchema::ReadFromXmlFile (schema, ECTestFixture::GetTestDataPath(L"SimpleTest_FirstSchema.01.00.ecschema.xml").c_str(), *schemaContext);

    EXPECT_EQ (SchemaReadStatus::Success, schemaStatus);

    ECInstanceReadContextPtr instanceContext = ECInstanceReadContext::CreateContext (*schema);

    IECInstancePtr  testInstance;
    InstanceReadStatus instanceStatus = IECInstance::ReadFromXmlFile (testInstance, ECTestFixture::GetTestDataPath(L"SimpleTest_Instance.xml").c_str(), *instanceContext);

    EXPECT_EQ (InstanceReadStatus::Success, instanceStatus);

    Utf8String str = testInstance->ToString("").c_str();
    VerifyTestInstance (testInstance.get(), false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(InstanceDeserializationTest, ExpectSuccessWhenDeserializingEmptyInstance)
    {
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();

    ECSchemaPtr schema;
    SchemaReadStatus schemaStatus = ECSchema::ReadFromXmlFile (schema, ECTestFixture::GetTestDataPath(L"SimpleTest_FirstSchema.01.00.ecschema.xml").c_str(), *schemaContext);
        
    EXPECT_EQ (SchemaReadStatus::Success, schemaStatus);

    ECInstanceReadContextPtr instanceContext = ECInstanceReadContext::CreateContext (*schema);

    IECInstancePtr  testInstance;
    InstanceReadStatus instanceStatus = IECInstance::ReadFromXmlFile (testInstance, ECTestFixture::GetTestDataPath(L"SimpleTest_EmptyInstance.xml").c_str(), *instanceContext);

    EXPECT_EQ (InstanceReadStatus::Success, instanceStatus);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                  Caleb.Shafer                   01/2017  
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(InstanceDeserializationTest, ExpectSuccessWhenRoundTrippingSimpleInstanceFromString)
    {
    
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();

    ECSchemaPtr schema;
    SchemaReadStatus schemaStatus = ECSchema::ReadFromXmlFile (schema, ECTestFixture::GetTestDataPath(L"SimpleTest_FirstSchema.01.00.ecschema.xml").c_str(), *schemaContext);
    EXPECT_EQ (SchemaReadStatus::Success, schemaStatus);

    ECInstanceReadContextPtr instanceContext = ECInstanceReadContext::CreateContext (*schema);

    IECInstancePtr  testInstance;
    InstanceReadStatus instanceStatus = IECInstance::ReadFromXmlFile (testInstance, ECTestFixture::GetTestDataPath(L"SimpleTest_Instance.xml").c_str(), *instanceContext);
    EXPECT_EQ (InstanceReadStatus::Success, instanceStatus);
    
    testInstance->ToString("").c_str();
    VerifyTestInstance (testInstance.get(), false);

    Utf8String ecInstanceXml;

    InstanceWriteStatus status2 = testInstance->WriteToXmlString(ecInstanceXml, true, false);
    EXPECT_EQ(InstanceWriteStatus::Success, status2);
    
    IECInstancePtr deserializedInstance;
    InstanceReadStatus status3 = IECInstance::ReadFromXmlString(deserializedInstance, ecInstanceXml.c_str(), *instanceContext);
    EXPECT_EQ (InstanceReadStatus::Success, status3); 
#ifdef DEBUG_PRINT
    printf("Verifying schema deserialized from string.\n");
#endif
    VerifyTestInstance (deserializedInstance.get(), false);
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            11/2016
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(InstanceDeserializationTest, DeserializeManagedXmlWithBinary)
    {
    Utf8CP schemaXml = 
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<ECSchema xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\" version=\"1.0\" nameSpacePrefix=\"ts\" schemaName=\"TestSchema\">"
            "<ECClass isDomainClass=\"True\" typeName=\"Foo\">"
                "<ECProperty typeName=\"binary\" propertyName=\"BinProp\" />"
            "</ECClass>"
        "</ECSchema>";

    Utf8CP instanceXml = 
        "<Foo xmlns=\"TestSchema.01.00\">"
            "<BinProp>YWJjZGU=</BinProp>"
        "</Foo>";

    ECSchemaPtr nativeSchema;
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    ECN::SchemaReadStatus readStatus = ECN::ECSchema::ReadFromXmlString(nativeSchema, schemaXml, *schemaContext);
    ASSERT_EQ(SchemaReadStatus::Success, readStatus);

    ECN::IECInstancePtr  nativeInstance;
    ECInstanceReadContextPtr instanceContext = ECInstanceReadContext::CreateContext(*nativeSchema);
    ECN::IECInstance::ReadFromXmlString(nativeInstance, instanceXml, *instanceContext);

    ECValue         binaryMember;
    ASSERT_EQ(ECObjectsStatus::Success, nativeInstance->GetValue(binaryMember, "BinProp"));

    Byte sourceBinaryData[] = {'a', 'b', 'c', 'd', 'e'};
    size_t      numBytes;
    const Byte* byteData;
    if (NULL != (byteData = binaryMember.GetBinary(numBytes)))
        {
        ASSERT_EQ(_countof(sourceBinaryData), numBytes);
        for (int index = 0; index <_countof(sourceBinaryData); index++)
            EXPECT_EQ(sourceBinaryData[index], byteData[index]);
        }

    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Victor.Cushman                02/2018
//---------------------------------------------------------------------------------------
TEST_F(InstanceSerializationTest, TestInstanceConversionOfPointxdWorksOnStructAndArrayJsonProperties)
    {
    DPoint2d const point2d = DPoint2d::From(42, -300);
    DPoint3d const point3d = DPoint3d::From(3, 777, 101);
    Json::Value json(Json::ValueType::objectValue);
    ECJsonUtilities::Point2dToJson(json["point2dObj"], point2d);
    ECJsonUtilities::Point3dToJson(json["point3dObj"], point3d);
    json["point2dArr"] = Json::Value(Json::ValueType::arrayValue);
    json["point2dArr"][0u] = point2d.x;
    json["point2dArr"][1u] = point2d.y;
    json["point3dArr"] = Json::Value(Json::ValueType::arrayValue);
    json["point3dArr"][0u] = point3d.x;
    json["point3dArr"][1u] = point3d.y;
    json["point3dArr"][2u] = point3d.z;

    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "TestSchema", "ts", 5, 0, 5, ECVersion::Latest);
    ECEntityClassP ecClass;
    EC_ASSERT_SUCCESS(schema->CreateEntityClass(ecClass, "TestClass"));
    InSchemaClassLocater classLocater(*schema);
    PrimitiveECPropertyP propPoint2dObj;
    EC_ASSERT_SUCCESS(ecClass->CreatePrimitiveProperty(propPoint2dObj, "point2dObj", PRIMITIVETYPE_Point2d));
    PrimitiveECPropertyP propPoint3dObj;
    EC_ASSERT_SUCCESS(ecClass->CreatePrimitiveProperty(propPoint3dObj, "point3dObj", PRIMITIVETYPE_Point3d));
    PrimitiveECPropertyP propPoint2dArr;
    EC_ASSERT_SUCCESS(ecClass->CreatePrimitiveProperty(propPoint2dArr, "point2dArr", PRIMITIVETYPE_Point2d));
    PrimitiveECPropertyP propPoint3dArr;
    EC_ASSERT_SUCCESS(ecClass->CreatePrimitiveProperty(propPoint3dArr, "point3dArr", PRIMITIVETYPE_Point3d));
    IECInstancePtr instance = ecClass->GetDefaultStandaloneEnabler()->CreateInstance();

    static std::function<bool(Utf8CP)> const shouldSerializeProperty = [](Utf8CP) -> bool {return true;};
    ASSERT_EQ(SUCCESS, JsonECInstanceConverter::JsonToECInstance(*instance, json, classLocater, shouldSerializeProperty));

    ECValue point2dObjVal;
    EC_ASSERT_SUCCESS(instance->GetValue(point2dObjVal, "point2dObj"));
    ASSERT_TRUE(point2dObjVal.IsPoint2d());
    ASSERT_EQ(point2d, point2dObjVal.GetPoint2d());
    ECValue point3dObjVal;
    EC_ASSERT_SUCCESS(instance->GetValue(point3dObjVal, "point3dObj"));
    ASSERT_TRUE(point3dObjVal.IsPoint3d());
    ASSERT_EQ(point3d, point3dObjVal.GetPoint3d());

    ECValue point2dArrVal;
    EC_ASSERT_SUCCESS(instance->GetValue(point2dArrVal, "point2dArr"));
    ASSERT_TRUE(point2dArrVal.IsPoint2d());
    ASSERT_EQ(point2d, point2dArrVal.GetPoint2d());
    ECValue point3dArrVal;
    EC_ASSERT_SUCCESS(instance->GetValue(point3dArrVal, "point3dObj"));
    ASSERT_TRUE(point3dArrVal.IsPoint3d());
    ASSERT_EQ(point3d, point3dArrVal.GetPoint3d());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/10
+---------------+---------------+---------------+---------------+---------------+------*/
void VerifyPolymorphismInstance (IECInstanceCP testInstance)
    {
    ECValue ecValue;
    EXPECT_EQ (ECObjectsStatus::Success, testInstance->GetValue (ecValue, "InRootClass"));
    EXPECT_EQ (-1, ecValue.GetInteger());

    // get and check the first member.
    EXPECT_EQ (ECObjectsStatus::Success, testInstance->GetValue (ecValue, "PolymorphArray", 0));
    IECInstancePtr  polymorphArrayInstance = ecValue.GetStruct();
    EXPECT_TRUE (polymorphArrayInstance.IsValid());
    ECClassCR       arrayMember0Class = polymorphArrayInstance->GetClass();
    EXPECT_STREQ ("BaseClass", arrayMember0Class.GetName().c_str());

    EXPECT_EQ (ECObjectsStatus::Success, polymorphArrayInstance->GetValue (ecValue, "InBaseClass"));
    EXPECT_EQ (0, ecValue.GetInteger());

    // get and check the second member.
    EXPECT_EQ (ECObjectsStatus::Success, testInstance->GetValue (ecValue, "PolymorphArray", 1));
    polymorphArrayInstance = ecValue.GetStruct();
    EXPECT_TRUE (polymorphArrayInstance.IsValid());
    ECClassCR arrayMember1Class = polymorphArrayInstance->GetClass();
    EXPECT_STREQ ("SubClass1", arrayMember1Class.GetName().c_str());

    EXPECT_EQ (ECObjectsStatus::Success, polymorphArrayInstance->GetValue (ecValue, "InBaseClass"));
    EXPECT_EQ (1, ecValue.GetInteger());

    EXPECT_EQ (ECObjectsStatus::Success, polymorphArrayInstance->GetValue (ecValue, "InSubClass1"));
    EXPECT_EQ (2, ecValue.GetInteger());

    // get and check the third member.
    EXPECT_EQ (ECObjectsStatus::Success, testInstance->GetValue (ecValue, "PolymorphArray", 2));
    polymorphArrayInstance = ecValue.GetStruct();
    EXPECT_TRUE (polymorphArrayInstance.IsValid());
    ECClassCR arrayMember2Class = polymorphArrayInstance->GetClass();
    EXPECT_STREQ ("SubClass2", arrayMember2Class.GetName().c_str());

    EXPECT_EQ (ECObjectsStatus::Success, polymorphArrayInstance->GetValue (ecValue, "InBaseClass"));
    EXPECT_EQ (3, ecValue.GetInteger());

    EXPECT_EQ (ECObjectsStatus::Success, polymorphArrayInstance->GetValue (ecValue, "InSubClass1"));
    EXPECT_EQ (4, ecValue.GetInteger());

    EXPECT_EQ (ECObjectsStatus::Success, polymorphArrayInstance->GetValue (ecValue, "InSubClass2"));
    EXPECT_EQ (5, ecValue.GetInteger());
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Caleb.Shafer                   01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PolymorphismDeserializationTest, ExpectSuccessWhenDeserializingPolymorphismInstance)
    {
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();

    ECSchemaPtr schema;
    SchemaReadStatus schemaStatus = ECSchema::ReadFromXmlFile (schema, ECTestFixture::GetTestDataPath(L"Polymorphism.01.00.ecschema.xml").c_str(), *schemaContext);
        
    EXPECT_EQ (SchemaReadStatus::Success, schemaStatus);

    ECInstanceReadContextPtr instanceContext = ECInstanceReadContext::CreateContext (*schema);

    IECInstancePtr  testInstance;
    InstanceReadStatus instanceStatus = IECInstance::ReadFromXmlFile (testInstance, ECTestFixture::GetTestDataPath(L"PolymorphismInstance.xml").c_str(), *instanceContext);

    EXPECT_EQ (InstanceReadStatus::Success, instanceStatus);
    
    testInstance->ToString("").c_str();
    VerifyPolymorphismInstance (testInstance.get());
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Caleb.Shafer                   01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(InstanceSerializationTest, ExpectSuccessWhenSerializingInstance)
    {
    
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();

    // we get the instance we want to serialize by reading the instance from XML.
    ECSchemaPtr schema;
    SchemaReadStatus schemaStatus = ECSchema::ReadFromXmlFile (schema, ECTestFixture::GetTestDataPath(L"SimpleTest_FirstSchema.01.00.ecschema.xml").c_str(), *schemaContext);
        
    EXPECT_EQ (SchemaReadStatus::Success, schemaStatus);

    // add a binary property to the class we are deserializing.
    ECClassP    testClass;
    if (NULL != (testClass = schema->GetClassP ("TestClass")))
        {
        PrimitiveECPropertyP binaryProperty;
        testClass->CreatePrimitiveProperty (binaryProperty, "TestBinary", PRIMITIVETYPE_Binary);
        }
    
    ECInstanceReadContextPtr instanceContext = ECInstanceReadContext::CreateContext (*schema);

    IECInstancePtr  testInstance;
    InstanceReadStatus instanceStatus = IECInstance::ReadFromXmlFile (testInstance, ECTestFixture::GetTestDataPath(L"SimpleTest_Instance.xml").c_str(), *instanceContext);
    EXPECT_EQ (InstanceReadStatus::Success, instanceStatus);

    ECValue     binaryValue;
    binaryValue.SetBinary (testBinaryData, _countof (testBinaryData), true);
    EXPECT_EQ (ECObjectsStatus::Success, testInstance->SetValue ("TestBinary", binaryValue));
    
    // verify that the instance is correct
    VerifyTestInstance (testInstance.get(), true);

    // AZK commented out until we find a better place to write these files or write them to strings for the interim as we do in schema serialization tests
    //// now we write it out.
    //EXPECT_EQ (InstanceWriteStatus::Success, testInstance->WriteToXmlFile (L"c:\\temp\\OutputInstance.xml"));
    //
    //// then read it back.
    //IECInstancePtr  readbackInstance;
    //InstanceReadStatus readbackStatus = IECInstance::ReadFromXmlFile (readbackInstance, L"c:\\temp\\OutputInstance.xml", *schema);
    //
    //EXPECT_EQ (InstanceReadStatus::Success, instanceStatus);
    //VerifyTestInstance (readbackInstance.get(), true);
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Caleb.Shafer                   01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(InstanceSerializationTest, ExpectSuccessWhenRoundTrippingViaWCharXmlString)
    {
#ifdef _WIN32
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();

    // we get the instance we want to serialize by reading the instance from XML.
    ECSchemaPtr schema;
    SchemaReadStatus schemaStatus = ECSchema::ReadFromXmlFile (schema, ECTestFixture::GetTestDataPath(L"SimpleTest_FirstSchema.01.00.ecschema.xml").c_str(), *schemaContext);
    EXPECT_EQ (SchemaReadStatus::Success, schemaStatus);
    
    ECClassCP testClass = schema->GetClassP ("TestClass");
    Utf8CP propertyName = "StringMember";
    ECInstanceReadContextPtr instanceReadContext = ECInstanceReadContext::CreateContext (*schema);    
        
    IECInstancePtr testInstance = testClass->GetDefaultStandaloneEnabler ()->CreateInstance ();
        
        // *** WIP_PORTABILITY: Use an escape such as \u here. Don't try to use extended ascii directly
    //*** Test with WChar String
    WString expectedString (L"הצüבß");
    ECValue expectedValue (expectedString.c_str ());
    EXPECT_EQ (ECObjectsStatus::Success, testInstance->SetValue (propertyName, expectedValue));
        
    WString wcharXml;
    InstanceWriteStatus serializeStat = testInstance->WriteToXmlString (wcharXml, true, true);
    EXPECT_EQ (InstanceWriteStatus::Success, serializeStat) << L"IECInstance::WriteToXmlString with WString overload failed.";
    
    // then read it back.
    IECInstancePtr readbackInstance = NULL;
    InstanceReadStatus deserializeStatus = IECInstance::ReadFromXmlString (readbackInstance, wcharXml.c_str (), *instanceReadContext);
    EXPECT_EQ (InstanceReadStatus::Success, deserializeStatus) << L"IECInstance::ReadFromXmlString with WChar overload failed";
    
    ECValue actualValue;
    EXPECT_EQ (ECObjectsStatus::Success, testInstance->GetValue (actualValue, propertyName)) << L"Could not retrieve property value for property '" << propertyName << "'.";
    
    EXPECT_TRUE (expectedValue.Equals (actualValue)) << L"Roundtripped WChar ECValue does not match original ECValue.";
    WCharCP actualString = actualValue.GetWCharCP();
    EXPECT_STREQ (expectedString.c_str (), actualString) << L"Roundtripped WChar string does not match original string.";    
#endif
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Caleb.Shafer                   01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(InstanceSerializationTest, ExpectSuccessWhenRoundTrippingViaUtf8XmlString)
    {
#ifdef _WIN32
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();

    // we get the instance we want to serialize by reading the instance from XML.
    ECSchemaPtr schema;
    SchemaReadStatus schemaStatus = ECSchema::ReadFromXmlFile (schema, ECTestFixture::GetTestDataPath(L"SimpleTest_FirstSchema.01.00.ecschema.xml").c_str(), *schemaContext);
    EXPECT_EQ (SchemaReadStatus::Success, schemaStatus);
    
    ECClassCP testClass = schema->GetClassP ("TestClass");
    Utf8CP propertyName = "StringMember";
    ECInstanceReadContextPtr instanceReadContext = ECInstanceReadContext::CreateContext (*schema);    
        
    IECInstancePtr testInstance = testClass->GetDefaultStandaloneEnabler ()->CreateInstance ();
        
        // *** WIP_PORTABILITY: Use an escape such as \u here. Don't try to use extended ascii directly
    //*** Test with WChar String
    Utf8String expectedString;
    BeStringUtilities::WCharToUtf8 (expectedString, L"הצüßבט");
    ECValue expectedValue (expectedString.c_str ());
    EXPECT_EQ (ECObjectsStatus::Success, testInstance->SetValue (propertyName, expectedValue));
        
    Utf8String utf8Xml;
    InstanceWriteStatus serializeStat = testInstance->WriteToXmlString (utf8Xml, true, true);
    EXPECT_EQ (InstanceWriteStatus::Success, serializeStat) << "IECInstance::WriteToXmlString with Utf8String overload failed.";
    
    // then read it back.
    IECInstancePtr readbackInstance = NULL;
    InstanceReadStatus deserializeStatus = IECInstance::ReadFromXmlString (readbackInstance, utf8Xml.c_str (), *instanceReadContext);
    EXPECT_EQ (InstanceReadStatus::Success, deserializeStatus) << "IECInstance::ReadFromXmlString with Utf8Char overload failed";
    
    ECValue actualValue;
    EXPECT_EQ (ECObjectsStatus::Success, testInstance->GetValue (actualValue, propertyName)) << "Could not retrieve property value for property '" << propertyName << "'.";
    
    EXPECT_TRUE (expectedValue.Equals (actualValue)) << "Roundtripped WChar ECValue does not match original ECValue.";
    Utf8CP actualString = actualValue.GetUtf8CP ();
    EXPECT_STREQ (expectedString.c_str (), actualString) << "Roundtripped WChar string does not match original string.";    
#endif
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (InstanceSerializationTest, EmptyPropertyTags)
    {
    // Ensure native impl produces same values as managed:
    // "<StringProperty />" => L"", not a NULL string
    // "<IntOrDoubleProperty />" => NULL

    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    ECSchemaPtr schema;
    ECSchema::ReadFromXmlFile (schema, ECTestFixture::GetTestDataPath (L"SimpleTest_FirstSchema.01.00.ecschema.xml").c_str(), *schemaContext);

    ECInstanceReadContextPtr instanceContext = ECInstanceReadContext::CreateContext (*schema);
    Utf8CP instanceXml =   "<TestClass xmlns=\"SimpleTest_FirstSchema.01.00\">"
                            "  <StringMember />"
                            "  <DoubleMember />"
                            "  <LongMember />"
                            "</TestClass>";

    IECInstancePtr instance;
    IECInstance::ReadFromXmlString (instance, instanceXml, *instanceContext);

    ECValue v;
    EXPECT_EQ (ECObjectsStatus::Success, instance->GetValue (v, "StringMember"));
    EXPECT_FALSE (v.IsNull());
    EXPECT_EQ (L'\0', *v.GetUtf8CP());

    EXPECT_EQ (ECObjectsStatus::Success, instance->GetValue (v, "DoubleMember"));
    EXPECT_TRUE (v.IsNull());

    EXPECT_EQ (ECObjectsStatus::Success, instance->GetValue (v, "LongMember"));
    EXPECT_TRUE (v.IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(InstanceSerializationTest, ExpectSuccessWithIGeometryProperty)
    {
    ECSchemaPtr testSchema;
    ECSchema::CreateSchema(testSchema, "GeometrySchema", "ts", 1, 0, 0);
    ECEntityClassP geomClass;
    testSchema->CreateEntityClass(geomClass, "GeometryStore");

    PrimitiveECPropertyP stringProp;
    geomClass->CreatePrimitiveProperty(stringProp, "Name");

    PrimitiveECPropertyP geomProperty;
    geomClass->CreatePrimitiveProperty(geomProperty, "MyGeometry");
    geomProperty->SetTypeName ("Bentley.Geometry.Common.IGeometry");

    IECInstancePtr instance = geomClass->GetDefaultStandaloneEnabler()->CreateInstance();

    DEllipse3d ellipse;
    ellipse.Init (0.0, 0.0, 0.0, 10000, 0.0, 0.0, 0.0, 10000, 0.0, 0.0, msGeomConst_2pi);
    ICurvePrimitivePtr arc = ICurvePrimitive::CreateArc (ellipse);
    IGeometryPtr geometryPtr = IGeometry::Create (arc);
    ECValue v;
    BentleyStatus vStatus = v.SetIGeometry (*geometryPtr);
    ASSERT_EQ(SUCCESS ,vStatus);

    IGeometryPtr storedGeometryPtr1 = v.GetIGeometry ();
    ASSERT_TRUE(storedGeometryPtr1.IsValid());
    ECObjectsStatus status = instance->SetValue ("MyGeometry", v);
    ASSERT_EQ(ECObjectsStatus::Success,status);

    Utf8String ecInstanceXml;
    if (InstanceWriteStatus::Success != instance->WriteToXmlString(ecInstanceXml, true, false))
        ASSERT_TRUE(false);

    ECInstanceReadContextPtr instanceContext = ECInstanceReadContext::CreateContext (*testSchema);
    IECInstancePtr newInstancePtr;
    InstanceReadStatus readStatus = IECInstance::ReadFromXmlString(newInstancePtr, ecInstanceXml.c_str(), *instanceContext);
    if(InstanceReadStatus::Success != readStatus)
        EXPECT_TRUE(false);

    ECValue deserializedGeomProp;
    ECObjectsStatus stat = newInstancePtr->GetValue (deserializedGeomProp, "MyGeometry");
    ASSERT_EQ (ECObjectsStatus::Success, stat) << "IECInstance::GetValue";

    IGeometryPtr geomPtr = deserializedGeomProp.GetIGeometry();
    ASSERT_TRUE(geomPtr.IsValid());
    ASSERT_TRUE(geomPtr->IsSameStructureAndGeometry(*storedGeometryPtr1)) << "Did not get same structure for " << ecInstanceXml.c_str();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                 Raimondas.Rimkus 02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(InstanceSerializationTest, InstanceWriteReadFile)
    {
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext ();

    ECSchemaPtr schema;
    SchemaReadStatus schemaStatus = ECSchema::ReadFromXmlFile (schema, ECTestFixture::GetTestDataPath (L"SimpleTest_FirstSchema.01.00.ecschema.xml").c_str (), *schemaContext);

    EXPECT_EQ (SchemaReadStatus::Success, schemaStatus);

    ECInstanceReadContextPtr instanceContext = ECInstanceReadContext::CreateContext (*schema);

    IECInstancePtr  testInstance;
    InstanceReadStatus instanceStatus = IECInstance::ReadFromXmlFile (testInstance, ECTestFixture::GetTestDataPath (L"SimpleTest_Instance.xml").c_str (), *instanceContext);
    EXPECT_EQ (InstanceReadStatus::Success, instanceStatus);

    VerifyTestInstance (testInstance.get(), false);

    EXPECT_EQ (InstanceWriteStatus::Success, testInstance->WriteToXmlFile (ECTestFixture::GetTempDataPath (L"OutputInstance.xml").c_str (), true, false));
    IECInstancePtr  readbackInstance;
    InstanceReadStatus readbackStatus = IECInstance::ReadFromXmlFile (readbackInstance, ECTestFixture::GetTempDataPath (L"OutputInstance.xml").c_str (), *instanceContext);

    EXPECT_EQ (InstanceReadStatus::Success, readbackStatus);
    VerifyTestInstance (readbackInstance.get(), false);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad.Hassan                     07/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(InstanceSerializationTest, WriteECInstance)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "TestSchema", "ts", 1, 0, 0);

    ECEntityClassP ecClass;
    schema->CreateEntityClass(ecClass, "TestClass");
    
    uint32_t propIndex;

    PrimitiveECPropertyP prop;
    ecClass->CreatePrimitiveProperty(prop, "StringProperty", PRIMITIVETYPE_String);
    ecClass->GetDefaultStandaloneEnabler()->GetPropertyIndex(propIndex, "StringProperty");

    IECInstancePtr instance = ecClass->GetDefaultStandaloneEnabler()->CreateInstance();

    ASSERT_EQ(ECObjectsStatus::Success, instance->SetValue("StringProperty", ECValue("Some value")));

    // WriteToBeXmlNode
    BeXmlWriterPtr xmlWriter = BeXmlWriter::Create();
    ASSERT_EQ(InstanceWriteStatus::Success, instance->WriteToBeXmlNode(*xmlWriter));

    Utf8String nodeInstanceString;
    xmlWriter->ToString(nodeInstanceString);

    // WriteToBeXmlDom
    BeXmlWriterPtr xmlDOMWriter = BeXmlWriter::Create();
    Utf8String domInstanceString = "";
    ASSERT_EQ(InstanceWriteStatus::Success, instance->WriteToBeXmlDom(*xmlDOMWriter, true));
    xmlDOMWriter->ToString(domInstanceString);

    // compare strings
    ASSERT_STREQ(nodeInstanceString.c_str(), domInstanceString.c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Bill.Goehrig     01/2017
//---------------------------------------------------------------------------------------
TEST_F(InstanceSerializationTest, TestInstanceJsonRoundtrip)
    {
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();

    ECSchemaPtr schema;
    SchemaReadStatus schemaStatus = ECSchema::ReadFromXmlFile(schema, ECTestFixture::GetTestDataPath(L"SimpleTest_FirstSchema.01.00.ecschema.xml").c_str(), *schemaContext);

    EXPECT_EQ(SchemaReadStatus::Success, schemaStatus);

    ECInstanceReadContextPtr instanceContext = ECInstanceReadContext::CreateContext(*schema);

    IECInstancePtr  testInstance;
    InstanceReadStatus instanceStatus = IECInstance::ReadFromXmlFile(testInstance, ECTestFixture::GetTestDataPath(L"SimpleTest_Instance.xml").c_str(), *instanceContext);
    EXPECT_EQ(InstanceReadStatus::Success, instanceStatus);
    VerifyTestInstance(testInstance.get(), false);

    Json::Value jsonRoot(Json::objectValue);
    EXPECT_EQ(SUCCESS, JsonEcInstanceWriter::WriteInstanceToJson(jsonRoot, *testInstance, "$Instance", false));

    StandaloneECEnablerP enabler = testInstance->GetClass().GetDefaultStandaloneEnabler();
    IECInstancePtr readbackInstance = enabler->CreateInstance();
    ASSERT_TRUE(readbackInstance.IsValid());
    InSchemaClassLocater classLocater(*schema);
    BentleyStatus readbackStatus = JsonECInstanceConverter::JsonToECInstance(*readbackInstance, jsonRoot["$Instance"], classLocater);

    EXPECT_EQ(BentleyStatus::SUCCESS, readbackStatus);
    VerifyTestInstance(readbackInstance.get(), false, true);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Bill.Goehrig     01/2017
//---------------------------------------------------------------------------------------
TEST_F(InstanceSerializationTest, TestInstanceRapidJsonRoundtrip)
    {
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();

    ECSchemaPtr schema;
    SchemaReadStatus schemaStatus = ECSchema::ReadFromXmlFile(schema, ECTestFixture::GetTestDataPath(L"SimpleTest_FirstSchema.01.00.ecschema.xml").c_str(), *schemaContext);

    EXPECT_EQ(SchemaReadStatus::Success, schemaStatus);

    ECInstanceReadContextPtr instanceContext = ECInstanceReadContext::CreateContext(*schema);

    IECInstancePtr  testInstance;
    InstanceReadStatus instanceStatus = IECInstance::ReadFromXmlFile(testInstance, ECTestFixture::GetTestDataPath(L"SimpleTest_Instance.xml").c_str(), *instanceContext);
    EXPECT_EQ(InstanceReadStatus::Success, instanceStatus);
    VerifyTestInstance(testInstance.get(), false);

    Json::Value jsonRoot(Json::objectValue);
    EXPECT_EQ(SUCCESS, JsonEcInstanceWriter::WriteInstanceToJson(jsonRoot, *testInstance, "$Instance", false));

    Json::FastWriter writer;
    rapidjson::Document rapidJsonRoot;
    rapidjson::ParseResult parseResult = rapidJsonRoot.Parse(writer.ToString(jsonRoot).c_str());
    EXPECT_FALSE(parseResult.IsError());

    StandaloneECEnablerP enabler = testInstance->GetClass().GetDefaultStandaloneEnabler();
    IECInstancePtr readbackInstance = enabler->CreateInstance();
    ASSERT_TRUE(readbackInstance.IsValid());
    InSchemaClassLocater classLocater(*schema);
    BentleyStatus readbackStatus = JsonECInstanceConverter::JsonToECInstance(*readbackInstance, rapidJsonRoot["$Instance"], classLocater);

    EXPECT_EQ(BentleyStatus::SUCCESS, readbackStatus);
    VerifyTestInstance(readbackInstance.get(), false, true);
    }

void VerifyShouldNotSerializeProperties(IECInstanceP instance)
    {
    ECClassCR ecClass = instance->GetClass();
    for (ECPropertyP ecProp : ecClass.GetProperties())
        {
        bool serialize = true;
        instance->ShouldSerializeProperty(serialize, ecProp->GetName().c_str());
        EXPECT_FALSE(serialize) << "The property " << ecProp->GetName().c_str() << " should not be serialized but was returned that it should.";
        }
    }

void VerifyShouldSerializeProperty(IECInstanceR instance, Utf8CP propertyAccessor, bool shouldBeSerialized = true)
    {
    bool serialize = false;
    EXPECT_EQ(ECObjectsStatus::Success, instance.ShouldSerializeProperty(serialize, propertyAccessor));
    if (shouldBeSerialized)
        EXPECT_TRUE(serialize) << propertyAccessor << " is no longer empty so needs to be serialized";
    else
        EXPECT_FALSE(serialize) << propertyAccessor << " is empty and should not be serialized";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Caleb.Shafer     02/2017
//---------------------------------------------------------------------------------------
TEST_F(InstanceSerializationTest, TestShouldSerializeProperty)
    {
    ECSchemaPtr schema;
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    ECSchema::ReadFromXmlFile(schema, ECTestFixture::GetTestDataPath(L"SimpleTest_SecondSchema.02.00.00.ecschema.xml").c_str(), *schemaContext);
    ASSERT_TRUE(schema.IsValid());

    ECClassCP testClass = schema->GetClassCP("TestClass");
    IECInstancePtr instance = testClass->GetDefaultStandaloneEnabler()->CreateInstance();
    ASSERT_TRUE(instance.IsValid());
    VerifyShouldNotSerializeProperties(instance.get());

    EXPECT_EQ(ECObjectsStatus::Success, instance->SetValue("BooleanMember", ECValue(true)));
    VerifyShouldSerializeProperty(*instance.get(), "BooleanMember");
    
    EXPECT_EQ(ECObjectsStatus::Success, instance->SetValue("BooleanMember", ECValue()));
    VerifyShouldSerializeProperty(*instance.get(), "BooleanMember", false);
    
    EXPECT_EQ(ECObjectsStatus::Success, instance->AddArrayElements("IntArray", 2));
    VerifyShouldSerializeProperty(*instance.get(), "IntArray");

    EXPECT_EQ(ECObjectsStatus::Success, instance->RemoveArrayElement("IntArray", 1));
    VerifyShouldSerializeProperty(*instance.get(), "IntArray");

    EXPECT_EQ(ECObjectsStatus::Success, instance->RemoveArrayElement("IntArray", 0));
    VerifyShouldSerializeProperty(*instance.get(), "IntArray", false);

    EXPECT_EQ(ECObjectsStatus::Success, instance->SetValue("EmbeddedStruct.Struct1IntMember", ECValue(22)));
    VerifyShouldSerializeProperty(*instance.get(), "EmbeddedStruct");

    EXPECT_EQ(ECObjectsStatus::Success, instance->SetValue("EmbeddedStruct.Struct1IntMember", ECValue()));
    VerifyShouldSerializeProperty(*instance.get(), "EmbeddedStruct", false);

    EXPECT_EQ(ECObjectsStatus::Success, instance->AddArrayElements("StructArray", 1));
    VerifyShouldSerializeProperty(*instance.get(), "StructArray");

    ECClassCP structClass = schema->GetClassCP("Struct2");
    IECInstancePtr structInstance = structClass->GetDefaultStandaloneEnabler()->CreateInstance();
    ECValue structValue;
    structValue.SetStruct(structInstance.get());
    instance->SetValue("StructArray", structValue, 0);
    VerifyShouldSerializeProperty(*instance.get(), "StructArray");

    EXPECT_EQ(ECObjectsStatus::Success, instance->RemoveArrayElement("StructArray", 0));
    VerifyShouldSerializeProperty(*instance.get(), "StructArray", false);

    EXPECT_EQ(ECObjectsStatus::Success, instance->AddArrayElements("SecondEmbeddedStruct.NestedArray", 2));
    VerifyShouldSerializeProperty(*instance.get(), "SecondEmbeddedStruct");
    VerifyShouldSerializeProperty(*instance.get(), "SecondEmbeddedStruct.NestedArray");

    EXPECT_EQ(ECObjectsStatus::Success, instance->RemoveArrayElement("SecondEmbeddedStruct.NestedArray", 0));
    EXPECT_EQ(ECObjectsStatus::Success, instance->RemoveArrayElement("SecondEmbeddedStruct.NestedArray", 0));
    VerifyShouldSerializeProperty(*instance.get(), "SecondEmbeddedStruct", false);
    VerifyShouldSerializeProperty(*instance.get(), "SecondEmbeddedStruct.NestedArray", false);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Caleb.Shafer     08/2017
//---------------------------------------------------------------------------------------
TEST_F(InstanceSerializationTest, TestSerializeNullValues)
    {
    ECSchemaPtr schema;
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    ECSchema::ReadFromXmlFile(schema, ECTestFixture::GetTestDataPath(L"SimpleTest_FirstSchema.01.00.ecschema.xml").c_str(), *schemaContext);
    ASSERT_TRUE(schema.IsValid());

    ECClassCP testClass = schema->GetClassCP("TestClass");
    IECInstancePtr instance = testClass->GetDefaultStandaloneEnabler()->CreateInstance();
    ASSERT_TRUE(instance.IsValid());

    Json::Value instanceJson;
    EXPECT_EQ(BSISUCCESS, JsonEcInstanceWriter::WriteInstanceToJson(instanceJson, *instance, "TestClass", true, true));

    Json::StyledWriter writer = Json::StyledWriter();
    LOG.error(writer.write(instanceJson).c_str());

    StandaloneECEnablerP enabler = instance->GetClass().GetDefaultStandaloneEnabler();
    IECInstancePtr readbackInstance = enabler->CreateInstance();
    ASSERT_TRUE(readbackInstance.IsValid());
    InSchemaClassLocater classLocater(*schema);
    EXPECT_EQ(BSISUCCESS, JsonECInstanceConverter::JsonToECInstance(*readbackInstance, instanceJson["TestClass"], classLocater));
    }

/*---------------------------------------------------------------------------------**//**
* Specify the ECSchema that contains all the class/struct definitions
* @bsimethod                                                    BillSteinbock   02/12
+---------------+---------------+---------------+---------------+---------------+------*/
static ECSchemaPtr CreateTestSchema ()
    {
    Utf8CP schemaXML = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                    "<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"test\" version=\"01.00\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
                    "    <ECClass typeName=\"SampleDataClass\" isStruct=\"True\" isDomainClass=\"True\">"
                    "        <ECProperty propertyName=\"Name\"           typeName=\"string\" />"
                    "        <ECProperty propertyName=\"ID\"             typeName=\"int\" />"
                    "        <ECProperty propertyName=\"StartPoint\"     typeName=\"point3d\" />"
                    "        <ECProperty propertyName=\"EndPoint\"       typeName=\"point3d\" />"
                    "        <ECProperty propertyName=\"XYSize\"         typeName=\"point2d\" />"
                    "        <ECProperty propertyName=\"Length\"         typeName=\"double\"  />"
                    "        <ECArrayProperty propertyName=\"Readings\"  typeName=\"long\" />"
                    "        <ECProperty propertyName=\"Field_Tested\"   typeName=\"boolean\"  />"
                    "    </ECClass>"
                    "</ECSchema>";

    ECSchemaReadContextPtr  schemaContext = ECSchemaReadContext::CreateContext();

    ECSchemaPtr schema; 
    SchemaReadStatus status = ECSchema::ReadFromXmlString (schema, schemaXML, *schemaContext);
    EXPECT_EQ (SchemaReadStatus::Success, status);  
    
    return schema;
    }

//=========================================================
// Native structure
//=========================================================
struct  SampleData
    {
    // struct data
    Utf8String                  m_name;
    int                      m_id;
    DPoint3d                 m_startPoint;
    DPoint3d                 m_endPoint;
    DPoint2d                 m_xySize;
    double                   m_length;
    uint32_t                 m_numReadings;
    int64_t*                   m_readings;
    bool                     m_tested;

    SampleData (int id, uint32_t numReadings)
        {
        m_name.Sprintf ("Sample_%d", id);
        m_id = id;
        m_numReadings = numReadings;

        if (numReadings > 0)
            {
            m_readings = (int64_t*) calloc (m_numReadings, sizeof(int64_t));
            for (uint32_t i=0; i<m_numReadings; i++)
                m_readings[i] = (int64_t)i*100;     
            }
        else
            {
            m_readings = NULL;
            }

        m_length = id * 3.75;

        m_startPoint.x = id * 1.0;
        m_startPoint.y = id * 1.0;
        m_startPoint.z = id * 1.0;

        m_endPoint.x = id * 1.0 + 100.0;
        m_endPoint.y = id * 1.0 + 100.0;
        m_endPoint.z = id * 1.0 + 100.0;

        m_xySize.x = id * 0.5;
        m_xySize.y = id * 1.0 + 0.5;

        m_tested = (0 == id%2);   
        }

    ~SampleData ()
        {
        if (m_readings)
            free (m_readings);
        }
    };

//=========================================================
// SampleDataInstanceManager - holds schema and enabler that will be
// needed to create standalone instances. 
//=========================================================
struct  SampleDataInstanceManager
    {
    // Instance support
    StandaloneECEnablerPtr   m_enabler;
    ECSchemaPtr              m_schema;

    SampleDataInstanceManager ()
        {
        // create the test schema (the created schema is added to the schema cache m_schemaOwner
        m_schema = CreateTestSchema();

        // get the class that represents the native SampleData stuct and get the enabler that will be used to create instances
        ECClassP ecClass = m_schema->GetClassP ("SampleDataClass");
        if (ecClass != NULL)
            m_enabler = ecClass->GetDefaultStandaloneEnabler();
        else
            m_enabler = NULL;
        }

    // Method to populate a standalone instance from a struct
    StandaloneECInstancePtr CreateInstance(SampleData& data)
        {
        if (!m_enabler.IsValid())
            return NULL;

        // create an empty instance
        ECN::StandaloneECInstancePtr instance = m_enabler->CreateInstance();

        // populate the instance
        instance->SetValue("Name",         ECValue (data.m_name.c_str()));
        instance->SetValue("ID",           ECValue (data.m_id));
        instance->SetValue("StartPoint",   ECValue (data.m_startPoint));
        instance->SetValue("EndPoint",     ECValue (data.m_endPoint));
        instance->SetValue("XYSize",       ECValue (data.m_xySize));
        instance->SetValue("Length",       ECValue (data.m_length));
        instance->SetValue("Field_Tested", ECValue (data.m_tested));

        if (data.m_numReadings > 0)
            {
            // add room for the array members and then set the values for each
            instance->AddArrayElements("Readings", data.m_numReadings);
            for (uint32_t i=0; i<data.m_numReadings; i++)
                instance->SetValue("Readings", ECValue (data.m_readings[i]), i);
            }

        return instance;
        }
    };

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    BillSteinbock   02/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(InstanceSerializationTest, BuildInstanceAndSerializeToXML)
    {
    SampleDataInstanceManager instanceManager;
    Utf8String ecInstanceXml;

    for (int i = 0; i<5; i++)
        {
        SampleData sampleData(i, (uint32_t)(i % 2) * 3);
        ECN::StandaloneECInstancePtr testInstance = instanceManager.CreateInstance(sampleData);
        ASSERT_TRUE(testInstance.IsValid());

        InstanceWriteStatus status2 = testInstance->WriteToXmlString(ecInstanceXml, true, false);
        EXPECT_EQ(InstanceWriteStatus::Success, status2);
        }
    }

END_BENTLEY_ECN_TEST_NAMESPACE
