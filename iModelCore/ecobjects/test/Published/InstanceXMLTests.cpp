/*--------------------------------------------------------------------------------------+
|
|     $Source: test/Published/InstanceXMLTests.cpp $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsTestPCH.h"
#include "TestFixture.h"

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

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
    WCharCP  struct2StringMember;
    bool            struct2DoubleMemberNull;
    double          struct2DoubleMember;
    Struct1*        nestedArray;
    UInt32          arraySize;
    };


// set thie binary data member, and then
static byte testBinaryData[] = { 0, 255,  1, 254,  2, 253,  3, 252,  4, 251,  5, 250,  6, 249,  7, 248,  8, 247,  9, 246, 10, 245, 11, 244, 12, 243, 13, 242, 14, 241, 15, 240, 16, 239, 17, 238, 18, 237,
                                19, 236, 20, 235, 21, 234, 22, 233, 23, 232, 24, 231, 25, 230, 26, 229, 27, 228, 28, 227, 29, 226, 30, 225, 31, 224, 32, 223, 33, 222, 34, 221, 35, 220, 36, 219, 37, 218,
                                38, 217, 39, 216, 40, 215, 41, 214, 42, 213, 43, 212, 44, 211, 45, 210, 46, 209, 47, 208, 48, 207, 49, 206, 50, 205, 51, 204, 52, 203, 53, 202, 54, 201, 55, 200, 56, 199, 
                                57 };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/10
+---------------+---------------+---------------+---------------+---------------+------*/
void    VerifyTestInstance (IECInstanceCP testInstance, bool checkBinaryProperty)
    {
    ECValue ecValue;

    EXPECT_EQ (SUCCESS, testInstance->GetValue (ecValue, L"BaseClassMember"));
    EXPECT_EQ (987, ecValue.GetInteger());

    EXPECT_EQ (SUCCESS, testInstance->GetValue (ecValue, L"IntegerMember"));
    EXPECT_EQ (12, ecValue.GetInteger());

    EXPECT_EQ (SUCCESS, testInstance->GetValue (ecValue, L"CustomFormatInt"));
    EXPECT_EQ (13, ecValue.GetInteger());

    EXPECT_EQ (SUCCESS, testInstance->GetValue (ecValue, L"BooleanMember"));
    EXPECT_EQ (false, ecValue.GetBoolean());

    EXPECT_EQ (SUCCESS, testInstance->GetValue (ecValue, L"DoubleMember"));
    EXPECT_EQ (456.789, ecValue.GetDouble());

    EXPECT_EQ (SUCCESS, testInstance->GetValue (ecValue, L"StringMember"));
    EXPECT_STREQ (ecValue.GetString(), L"Test string");

    EXPECT_EQ (SUCCESS, testInstance->GetValue (ecValue, L"FileNameMember"));
    EXPECT_STREQ (ecValue.GetString(), L"c:\\usr\\barry\\test.txt");

    EXPECT_EQ (SUCCESS, testInstance->GetValue (ecValue, L"NegativeMember"));
    EXPECT_EQ (-42, ecValue.GetInteger());

    EXPECT_EQ (SUCCESS, testInstance->GetValue (ecValue, L"DateTimeMember"));
    EXPECT_EQ (633374681466664305, ecValue.GetDateTimeTicks());

    EXPECT_EQ (SUCCESS, testInstance->GetValue (ecValue, L"StartPoint"));
    EXPECT_EQ (1.1, ecValue.GetPoint3D().x);
    EXPECT_EQ (2.2, ecValue.GetPoint3D().y);
    EXPECT_EQ (3.3, ecValue.GetPoint3D().z);

    EXPECT_EQ (SUCCESS, testInstance->GetValue (ecValue, L"EndPoint"));
    EXPECT_EQ (4.4, ecValue.GetPoint3D().x);
    EXPECT_EQ (7.7, ecValue.GetPoint3D().y);
    EXPECT_EQ (6.6, ecValue.GetPoint3D().z);

    EXPECT_EQ (SUCCESS, testInstance->GetValue (ecValue, L"SecondEmbeddedStruct.Struct1BoolMember"));
    EXPECT_EQ (false, ecValue.GetBoolean());

    EXPECT_EQ (SUCCESS, testInstance->GetValue (ecValue, L"SecondEmbeddedStruct.Struct1IntMember"));
    EXPECT_EQ (4, ecValue.GetInteger());

    EXPECT_EQ (SUCCESS, testInstance->GetValue (ecValue, L"FormattedStruct.Struct3DoubleMember"));
    EXPECT_EQ (17.443, ecValue.GetDouble());

    EXPECT_EQ (SUCCESS, testInstance->GetValue (ecValue, L"FormattedStruct.Struct3IntMember"));
    EXPECT_EQ (531992, ecValue.GetInteger());
    
    EXPECT_EQ (SUCCESS, testInstance->GetValue (ecValue, L"FormattedStruct.Struct3BoolMember"));
    EXPECT_EQ (true, ecValue.GetBoolean());

    int         expectedInts[] = {0, 101, 202, 303, 404, 505, 606, 707, 808, 909};
    for (UInt32 index=0; index < _countof (expectedInts); index++)
        {
        EXPECT_EQ (SUCCESS, testInstance->GetValue (ecValue, L"FormattedArray", index));
        EXPECT_EQ (expectedInts[index], ecValue.GetInteger());
        }

    int         moreInts[] = {41556, 32757, 56789, 32757, 21482 };
    for (UInt32 index=0; index < _countof (moreInts); index++)
        {
        EXPECT_EQ (SUCCESS, testInstance->GetValue (ecValue, L"IntArray", index));
        EXPECT_EQ (moreInts[index], ecValue.GetInteger());
        }

    EXPECT_EQ (SUCCESS, testInstance->GetValue (ecValue, L"OneMemberIntArray", 0));
    EXPECT_EQ (3, ecValue.GetInteger());

    DPoint3d    expectedPoints[] = { {4.0, 0.0, 0.0}, {1.0, 1.0, 1.0}, {2.0, 2.0, 2.0}, {3.0, 3.0, 3.0}, {4.0, 4.0, 4.0}, {5.0, 5.0, 5.0} };
    for (UInt32 index=0; index < _countof (expectedPoints); index++)
        {
        EXPECT_EQ (SUCCESS, testInstance->GetValue (ecValue, L"PointArray", index));
        EXPECT_EQ (expectedPoints[index].x, ecValue.GetPoint3D().x);
        EXPECT_EQ (expectedPoints[index].y, ecValue.GetPoint3D().y);
        EXPECT_EQ (expectedPoints[index].z, ecValue.GetPoint3D().z);
        }

    for (UInt32 index=0; index < 300; index++)
        {
        EXPECT_EQ (SUCCESS, testInstance->GetValue (ecValue, L"DateArray", index));
        }

    for (UInt32 index=0; index < 300; index++)
        {
        EXPECT_EQ (SUCCESS, testInstance->GetValue (ecValue, L"StringArray", index));
        wchar_t     expectedString[128];
        swprintf (expectedString, L"String %d", index%30);
        EXPECT_STREQ (expectedString, ecValue.GetString());
        }

    for (UInt32 index=0; index < 100; index++)
        {
        EXPECT_EQ (SUCCESS, testInstance->GetValue (ecValue, L"SmallIntArray", index));
        EXPECT_EQ (index, ecValue.GetInteger());
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
            L"testInstance.StructArray[0].Struct2StringMember",
            false,
            123456.789,
            firstArray,
            _countof (firstArray),
            },
            {
            false,
            L"testInstance.StructArray[1].Struct2StringMember",
            false,
            23456.789,
            secondArray,          
            _countof (secondArray),
            },
            {
            false,
            L"Added.Struct2StringMember",
            true,
            0.0,
            NULL,
            0
            },
        };

    for (UInt32 index=0; index < _countof (struct2ExpectedValues); index++)
        {
        ECValue         structArrayMember;
        EXPECT_EQ (SUCCESS, testInstance->GetValue (structArrayMember, L"StructArray", index));
        IECInstancePtr  structArrayInstance = structArrayMember.GetStruct();
        EXPECT_EQ (true, structArrayInstance.IsValid());
        
        if (struct2ExpectedValues[index].struct2StringMemberNull)
            {
            // This throws an assert rather than returning ERROR.
            // EXPECT_EQ (ERROR, structArrayInstance->GetString (stringValue, L"Struct2StringMember"));
            }
        else
            {
            EXPECT_EQ (SUCCESS, structArrayInstance->GetValue (ecValue, L"Struct2StringMember"));
            EXPECT_STREQ (struct2ExpectedValues[index].struct2StringMember, ecValue.GetString());
            }

        if (struct2ExpectedValues[index].struct2DoubleMemberNull)
            {
            // This throws an assert rather than returning ERROR.
            // EXPECT_EQ (ERROR, structArrayInstance->GetDouble (doubleValue, L"Struct2DoubleMember"));
            }
        else
            {
            EXPECT_EQ (SUCCESS, structArrayInstance->GetValue (ecValue, L"Struct2DoubleMember"));
            EXPECT_EQ (struct2ExpectedValues[index].struct2DoubleMember, ecValue.GetDouble());
            }

        // now try to get the array of structs, if expected.
        if (NULL == struct2ExpectedValues[index].nestedArray)
            continue;

        for (UInt32 nestedIndex=0; nestedIndex < struct2ExpectedValues[index].arraySize; nestedIndex++)
            {
            ECValue     nestedArrayMember;
            EXPECT_EQ (SUCCESS, structArrayInstance->GetValue (nestedArrayMember, L"NestedArray", nestedIndex));
            IECInstancePtr  nestedInstance = nestedArrayMember.GetStruct();
            EXPECT_EQ (true, nestedInstance.IsValid());
            
            EXPECT_EQ (SUCCESS, nestedInstance->GetValue (ecValue, L"Struct1BoolMember"));
            EXPECT_EQ (struct2ExpectedValues[index].nestedArray[nestedIndex].struct1BoolMember, ecValue.GetBoolean());

            EXPECT_EQ (SUCCESS, nestedInstance->GetValue (ecValue, L"Struct1IntMember"));
            EXPECT_EQ (struct2ExpectedValues[index].nestedArray[nestedIndex].struct1IntMember, ecValue.GetInteger());
            }
        }

    if (!checkBinaryProperty)
        return;

    ECValue         binaryMember;
    EXPECT_EQ (SUCCESS, testInstance->GetValue (binaryMember, L"TestBinary"));

    size_t      numBytes;
    const byte* byteData; 
    if (NULL != (byteData = binaryMember.GetBinary (numBytes)))
        {
        EXPECT_EQ (_countof (testBinaryData), numBytes);
        for (size_t index=0; index < _countof (testBinaryData); index++)
            EXPECT_EQ (testBinaryData[index], byteData[index]);
        }
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
    /*
TEST_F(InstanceDeserializationTest, ExpectSuccessWhenDeserializingFaultyInstance)
    {
    //This test will verify that the xml deserializer will not fail on mismatched or malformed properties.

    
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();

    ECSchemaPtr schema;
    SchemaReadStatus schemaStatus = ECSchema::ReadFromXmlFile (schema, ECTestFixture::GetTestDataPath(L"MismatchedSchema.01.00.ecschema.xml").c_str(), *schemaContext);

    EXPECT_EQ (SCHEMA_READ_STATUS_Success, schemaStatus);

    ECInstanceReadContextPtr instanceContext = ECInstanceReadContext::CreateContext (*schema);

    IECInstancePtr  testInstance;
    InstanceReadStatus instanceStatus;

    instanceStatus = IECInstance::ReadFromXmlFile (testInstance, ECTestFixture::GetTestDataPath(L"MismatchedInstance.xml").c_str(), *instanceContext);

    EXPECT_EQ (INSTANCE_READ_STATUS_Success, instanceStatus);
    //Testing a struct with type errors in its values
    ECValue ecValue;
    EXPECT_EQ (SUCCESS, testInstance->GetValue (ecValue, L"FormattedStructWithArrays0.BinaryArray",0));

    EXPECT_EQ (SUCCESS, testInstance->GetValue (ecValue, L"FormattedStructWithArrays0.BooleanArray",0));
    EXPECT_EQ (true, ecValue.GetBoolean());

    EXPECT_EQ (SUCCESS, testInstance->GetValue (ecValue, L"FormattedStructWithArrays0.DateTimeArray",0));
    EXPECT_EQ (633374681466664305, ecValue.GetDateTimeTicks());

    EXPECT_EQ (SUCCESS, testInstance->GetValue (ecValue, L"FormattedStructWithArrays0.DoubleArray",0));
    EXPECT_EQ (1.2, ecValue.GetDouble());

    EXPECT_EQ (SUCCESS, testInstance->GetValue (ecValue, L"FormattedStructWithArrays0.IntArray",1));
    EXPECT_EQ (7, ecValue.GetInteger());

    EXPECT_EQ (SUCCESS, testInstance->GetValue (ecValue, L"FormattedStructWithArrays0.LongArray",0));
    EXPECT_EQ (300, ecValue.GetLong());

    EXPECT_EQ (SUCCESS, testInstance->GetValue (ecValue, L"FormattedStructWithArrays0.Point2DArray",0));
    EXPECT_EQ (0, ecValue.GetPoint2D().x);
    EXPECT_EQ (1, ecValue.GetPoint2D().y);

    EXPECT_EQ (SUCCESS, testInstance->GetValue (ecValue, L"FormattedStructWithArrays0.Point3DArray",0));
    EXPECT_EQ (0, ecValue.GetPoint3D().x);
    EXPECT_EQ (1, ecValue.GetPoint3D().y);
    EXPECT_EQ (0, ecValue.GetPoint3D().z);

    EXPECT_EQ (SUCCESS, testInstance->GetValue (ecValue, L"FormattedStructWithArrays0.StringArray",0));
    EXPECT_STREQ (ecValue.GetString(), L"Test");

    // Testing a struct that misformatted values
    EXPECT_EQ (SUCCESS, testInstance->GetValue (ecValue, L"FormattedStructWithArrays1.BinaryArray",0));

    EXPECT_EQ (SUCCESS, testInstance->GetValue (ecValue, L"FormattedStructWithArrays1.BooleanArray",0));
    EXPECT_EQ (true, ecValue.GetBoolean());

    EXPECT_EQ (SUCCESS, testInstance->GetValue (ecValue, L"FormattedStructWithArrays1.DateTimeArray",0));
    EXPECT_EQ (633374681466664305, ecValue.GetDateTimeTicks());

    EXPECT_EQ (SUCCESS, testInstance->GetValue (ecValue, L"FormattedStructWithArrays1.DoubleArray",0));
    EXPECT_EQ (1.2, ecValue.GetDouble());

    EXPECT_EQ (SUCCESS, testInstance->GetValue (ecValue, L"FormattedStructWithArrays1.IntArray",0));
    EXPECT_EQ (7, ecValue.GetInteger());

    EXPECT_EQ (SUCCESS, testInstance->GetValue (ecValue, L"FormattedStructWithArrays1.LongArray",0));
    EXPECT_EQ (300, ecValue.GetLong());

    EXPECT_EQ (SUCCESS, testInstance->GetValue (ecValue, L"FormattedStructWithArrays1.Point2DArray",0));
    EXPECT_EQ (0, ecValue.GetPoint2D().x);
    EXPECT_EQ (1, ecValue.GetPoint2D().y);

    EXPECT_EQ (SUCCESS, testInstance->GetValue (ecValue, L"FormattedStructWithArrays1.Point3DArray",0));
    EXPECT_EQ (0, ecValue.GetPoint3D().x);
    EXPECT_EQ (1, ecValue.GetPoint3D().y);
    EXPECT_EQ (0, ecValue.GetPoint3D().z);

    EXPECT_EQ (SUCCESS, testInstance->GetValue (ecValue, L"FormattedStructWithArrays1.StringArray",0));
    EXPECT_STREQ (ecValue.GetString(), L"");
    EXPECT_EQ (SUCCESS, testInstance->GetValue (ecValue, L"FormattedStructWithArrays1.StringArray",6));
    EXPECT_STREQ (ecValue.GetString(), L"Test");

    //Testing a struct with few errors among entirely bad structs in the same array
    ECValue badStructArrayMember;
    EXPECT_EQ (SUCCESS, testInstance->GetValue (badStructArrayMember, L"ArrayOfBadStructs",2));
    IECInstancePtr  structInstance = badStructArrayMember.GetStruct();
    EXPECT_EQ (true, structInstance.IsValid());

    EXPECT_EQ (SUCCESS, structInstance->GetValue (ecValue, L"BinaryMember"));

    EXPECT_EQ (SUCCESS, structInstance->GetValue (ecValue, L"BooleanMember"));
    EXPECT_EQ (true, ecValue.GetBoolean());

    EXPECT_EQ (SUCCESS, structInstance->GetValue (ecValue, L"DateTimeMember"));
    EXPECT_EQ (633374681466664305, ecValue.GetDateTimeTicks());

    EXPECT_EQ (SUCCESS, structInstance->GetValue (ecValue, L"DoubleMember"));
    EXPECT_EQ (1.2, ecValue.GetDouble());

    EXPECT_EQ (SUCCESS, structInstance->GetValue (ecValue, L"IntMember"));
    EXPECT_EQ (7, ecValue.GetInteger());

    EXPECT_EQ (SUCCESS, structInstance->GetValue (ecValue, L"LongMember"));
    EXPECT_EQ (300, ecValue.GetLong());

    EXPECT_EQ (SUCCESS, structInstance->GetValue (ecValue, L"Point2DMember"));
    EXPECT_EQ (0, ecValue.GetPoint2D().x);
    EXPECT_EQ (1, ecValue.GetPoint2D().y);

    EXPECT_EQ (SUCCESS, structInstance->GetValue (ecValue, L"Point3DMember"));
    EXPECT_EQ (0, ecValue.GetPoint3D().x);
    EXPECT_EQ (1, ecValue.GetPoint3D().y);
    EXPECT_EQ (0, ecValue.GetPoint3D().z);

    EXPECT_EQ (SUCCESS, structInstance->GetValue (ecValue, L"StringMember"));
    EXPECT_STREQ (ecValue.GetString(), L"Test");

    schemaOwner = NULL;
    };
    */

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                10/2011
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(InstanceDeserializationTest, ExpectSuccessWhenDeserializingInstanceWithEmptyArrayMemebers)
    {
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();

    ECSchemaPtr schema;
    SchemaReadStatus schemaStatus = ECSchema::ReadFromXmlFile (schema, ECTestFixture::GetTestDataPath(L"MismatchedSchema.01.00.ecschema.xml").c_str(), *schemaContext);

    EXPECT_EQ (SCHEMA_READ_STATUS_Success, schemaStatus);

    ECInstanceReadContextPtr instanceContext = ECInstanceReadContext::CreateContext (*schema);

    IECInstancePtr  testInstance;
    InstanceReadStatus instanceStatus;

    instanceStatus = IECInstance::ReadFromXmlFile (testInstance, ECTestFixture::GetTestDataPath(L"EmptyArrayInstance.xml").c_str(), *instanceContext);

    
    EXPECT_EQ (INSTANCE_READ_STATUS_Success, instanceStatus);
    //Testing a struct with type errors in its values
    ECValue ecValue;
    EXPECT_EQ (SUCCESS, testInstance->GetValue (ecValue, L"FormattedStructWithArrays1.BinaryArray",0));

    EXPECT_EQ (SUCCESS, testInstance->GetValue (ecValue, L"FormattedStructWithArrays1.BooleanArray",1));
    EXPECT_EQ (true, ecValue.IsNull());
    EXPECT_EQ (SUCCESS, testInstance->GetValue (ecValue, L"FormattedStructWithArrays1.BooleanArray",0));
    EXPECT_EQ (true, ecValue.GetBoolean());

    EXPECT_EQ (SUCCESS, testInstance->GetValue (ecValue, L"FormattedStructWithArrays1.DateTimeArray",0));
    EXPECT_EQ (true, ecValue.IsNull());
    EXPECT_EQ (SUCCESS, testInstance->GetValue (ecValue, L"FormattedStructWithArrays1.DateTimeArray",1));
    EXPECT_EQ (633374681466664305, ecValue.GetDateTimeTicks());

    EXPECT_EQ (SUCCESS, testInstance->GetValue (ecValue, L"FormattedStructWithArrays1.DoubleArray",0));
    EXPECT_EQ (true, ecValue.IsNull());
    EXPECT_EQ (SUCCESS, testInstance->GetValue (ecValue, L"FormattedStructWithArrays1.DoubleArray",1));
    EXPECT_EQ (1.2, ecValue.GetDouble());

    EXPECT_EQ (SUCCESS, testInstance->GetValue (ecValue, L"FormattedStructWithArrays1.IntArray",0));
    EXPECT_EQ (true, ecValue.IsNull());
    EXPECT_EQ (SUCCESS, testInstance->GetValue (ecValue, L"FormattedStructWithArrays1.IntArray",1));
    EXPECT_EQ (7, ecValue.GetInteger());

    EXPECT_EQ (SUCCESS, testInstance->GetValue (ecValue, L"FormattedStructWithArrays1.LongArray",0));
    EXPECT_EQ (true, ecValue.IsNull());
    EXPECT_EQ (SUCCESS, testInstance->GetValue (ecValue, L"FormattedStructWithArrays1.LongArray",1));
    EXPECT_EQ (300, ecValue.GetLong());

    EXPECT_EQ (SUCCESS, testInstance->GetValue (ecValue, L"FormattedStructWithArrays1.Point2DArray",0));
    EXPECT_EQ (true, ecValue.IsNull());
    EXPECT_EQ (SUCCESS, testInstance->GetValue (ecValue, L"FormattedStructWithArrays1.Point2DArray",1));
    EXPECT_EQ (0, ecValue.GetPoint2D().x);
    EXPECT_EQ (1, ecValue.GetPoint2D().y);

    EXPECT_EQ (SUCCESS, testInstance->GetValue (ecValue, L"FormattedStructWithArrays1.Point3DArray",0));
    EXPECT_EQ (true, ecValue.IsNull());
    EXPECT_EQ (SUCCESS, testInstance->GetValue (ecValue, L"FormattedStructWithArrays1.Point3DArray",1));
    EXPECT_EQ (0, ecValue.GetPoint3D().x);
    EXPECT_EQ (1, ecValue.GetPoint3D().y);
    EXPECT_EQ (0, ecValue.GetPoint3D().z);

    EXPECT_EQ (SUCCESS, testInstance->GetValue (ecValue, L"FormattedStructWithArrays1.StringArray",0));
    EXPECT_STREQ (ecValue.GetString(), L"");
    EXPECT_EQ (SUCCESS, testInstance->GetValue (ecValue, L"FormattedStructWithArrays1.StringArray",6));
    EXPECT_STREQ (ecValue.GetString(), L"Test");
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(InstanceDeserializationTest, ExpectSuccessWhenDeserializingSimpleInstance)
    {
    
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();

    ECSchemaPtr schema;
    SchemaReadStatus schemaStatus = ECSchema::ReadFromXmlFile (schema, ECTestFixture::GetTestDataPath(L"SimpleTest_FirstSchema.01.00.ecschema.xml").c_str(), *schemaContext);

    EXPECT_EQ (SCHEMA_READ_STATUS_Success, schemaStatus);

    ECInstanceReadContextPtr instanceContext = ECInstanceReadContext::CreateContext (*schema);

    IECInstancePtr  testInstance;
    InstanceReadStatus instanceStatus = IECInstance::ReadFromXmlFile (testInstance, ECTestFixture::GetTestDataPath(L"SimpleTest_Instance.xml").c_str(), *instanceContext);

    EXPECT_EQ (INSTANCE_READ_STATUS_Success, instanceStatus);

    WString str = testInstance->ToString(L"").c_str();
    VerifyTestInstance (testInstance.get(), false);
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(InstanceDeserializationTest, ExpectSuccessWhenDeserializingEmptyInstance)
    {
    
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();

    ECSchemaPtr schema;
    SchemaReadStatus schemaStatus = ECSchema::ReadFromXmlFile (schema, ECTestFixture::GetTestDataPath(L"SimpleTest_FirstSchema.01.00.ecschema.xml").c_str(), *schemaContext);
        
    EXPECT_EQ (SCHEMA_READ_STATUS_Success, schemaStatus);

    ECInstanceReadContextPtr instanceContext = ECInstanceReadContext::CreateContext (*schema);

    IECInstancePtr  testInstance;
    InstanceReadStatus instanceStatus = IECInstance::ReadFromXmlFile (testInstance, ECTestFixture::GetTestDataPath(L"SimpleTest_EmptyInstance.xml").c_str(), *instanceContext);

    EXPECT_EQ (INSTANCE_READ_STATUS_Success, instanceStatus);
    };

#if defined (NEEDSWORK_XML)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(InstanceDeserializationTest, ExpectSuccessWhenRoundTrippingSimpleInstanceFromStream)
    {
    // must call CoInitialize - schema deserialization requires it.
    EXPECT_EQ (S_OK, CoInitialize(NULL)); 

    
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();

    ECSchemaPtr schema;
    SchemaReadStatus schemaStatus = ECSchema::ReadFromXmlFile (schema, ECTestFixture::GetTestDataPath(L"SimpleTest_FirstSchema.01.00.ecschema.xml").c_str(), *schemaContext);
    EXPECT_EQ (SCHEMA_READ_STATUS_Success, schemaStatus);

    ECInstanceReadContextPtr instanceContext = ECInstanceReadContext::CreateContext (*schema);

    IECInstancePtr  testInstance;
    InstanceReadStatus instanceStatus = IECInstance::ReadFromXmlFile (testInstance, ECTestFixture::GetTestDataPath(L"SimpleTest_Instance.xml").c_str(), *instanceContext);
    EXPECT_EQ (INSTANCE_READ_STATUS_Success, instanceStatus);
    
    testInstance->ToString(L"").c_str();
    VerifyTestInstance (testInstance.get(), false);

    LPSTREAM stream = NULL;
    //HRESULT res = ::CreateStreamOnHGlobal(NULL,TRUE,&stream);
    ::CreateStreamOnHGlobal(NULL,TRUE,&stream);

    InstanceWriteStatus status2 = testInstance->WriteToXmlStream(stream, true, false, false);
    EXPECT_EQ(INSTANCE_WRITE_STATUS_Success, status2);
    
    LARGE_INTEGER liPos = {0};
    stream->Seek(liPos, STREAM_SEEK_SET, NULL);

    IECInstancePtr deserializedInstance;
    InstanceReadStatus status3 = IECInstance::ReadFromXmlStream(deserializedInstance, stream, *instanceContext);
    EXPECT_EQ (INSTANCE_READ_STATUS_Success, status3); 
#ifdef DEBUG_PRINT
    wprintf(L"Verifying schema deserialized from stream.\n");
#endif
    VerifyTestInstance (deserializedInstance.get(), false);

    CoUninitialize();
    };
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(InstanceDeserializationTest, ExpectSuccessWhenRoundTrippingSimpleInstanceFromString)
    {
    
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();

    ECSchemaPtr schema;
    SchemaReadStatus schemaStatus = ECSchema::ReadFromXmlFile (schema, ECTestFixture::GetTestDataPath(L"SimpleTest_FirstSchema.01.00.ecschema.xml").c_str(), *schemaContext);
    EXPECT_EQ (SCHEMA_READ_STATUS_Success, schemaStatus);

    ECInstanceReadContextPtr instanceContext = ECInstanceReadContext::CreateContext (*schema);

    IECInstancePtr  testInstance;
    InstanceReadStatus instanceStatus = IECInstance::ReadFromXmlFile (testInstance, ECTestFixture::GetTestDataPath(L"SimpleTest_Instance.xml").c_str(), *instanceContext);
    EXPECT_EQ (INSTANCE_READ_STATUS_Success, instanceStatus);
    
    testInstance->ToString(L"").c_str();
    VerifyTestInstance (testInstance.get(), false);

    WString ecInstanceXml;

    InstanceWriteStatus status2 = testInstance->WriteToXmlString(ecInstanceXml, true, false);
    EXPECT_EQ(INSTANCE_WRITE_STATUS_Success, status2);
    
    IECInstancePtr deserializedInstance;
    InstanceReadStatus status3 = IECInstance::ReadFromXmlString(deserializedInstance, ecInstanceXml.c_str(), *instanceContext);
    EXPECT_EQ (INSTANCE_READ_STATUS_Success, status3); 
#ifdef DEBUG_PRINT
    wprintf(L"Verifying schema deserialized from string.\n");
#endif
    VerifyTestInstance (deserializedInstance.get(), false);
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/10
+---------------+---------------+---------------+---------------+---------------+------*/
void    VerifyPolymorphismInstance (IECInstanceCP testInstance)
    {
    ECValue     ecValue;
    EXPECT_EQ (SUCCESS, testInstance->GetValue (ecValue, L"InRootClass"));
    EXPECT_EQ (-1, ecValue.GetInteger());

    // get and check the first member.
    EXPECT_EQ (SUCCESS, testInstance->GetValue (ecValue, L"PolymorphArray", 0));
    IECInstancePtr  polymorphArrayInstance = ecValue.GetStruct();
    EXPECT_EQ (true, polymorphArrayInstance.IsValid());
    ECClassCR       arrayMember0Class = polymorphArrayInstance->GetClass();
    EXPECT_STREQ (L"BaseClass", arrayMember0Class.GetName().c_str());

    EXPECT_EQ (SUCCESS, polymorphArrayInstance->GetValue (ecValue, L"InBaseClass"));
    EXPECT_EQ (0, ecValue.GetInteger());

    // get and check the second member.
    EXPECT_EQ (SUCCESS, testInstance->GetValue (ecValue, L"PolymorphArray", 1));
    polymorphArrayInstance = ecValue.GetStruct();
    EXPECT_EQ (true, polymorphArrayInstance.IsValid());
    ECClassCR arrayMember1Class = polymorphArrayInstance->GetClass();
    EXPECT_STREQ (L"SubClass1", arrayMember1Class.GetName().c_str());

    EXPECT_EQ (SUCCESS, polymorphArrayInstance->GetValue (ecValue, L"InBaseClass"));
    EXPECT_EQ (1, ecValue.GetInteger());

    EXPECT_EQ (SUCCESS, polymorphArrayInstance->GetValue (ecValue, L"InSubClass1"));
    EXPECT_EQ (2, ecValue.GetInteger());

    // get and check the third member.
    EXPECT_EQ (SUCCESS, testInstance->GetValue (ecValue, L"PolymorphArray", 2));
    polymorphArrayInstance = ecValue.GetStruct();
    EXPECT_EQ (true, polymorphArrayInstance.IsValid());
    ECClassCR arrayMember2Class = polymorphArrayInstance->GetClass();
    EXPECT_STREQ (L"SubClass2", arrayMember2Class.GetName().c_str());

    EXPECT_EQ (SUCCESS, polymorphArrayInstance->GetValue (ecValue, L"InBaseClass"));
    EXPECT_EQ (3, ecValue.GetInteger());

    EXPECT_EQ (SUCCESS, polymorphArrayInstance->GetValue (ecValue, L"InSubClass1"));
    EXPECT_EQ (4, ecValue.GetInteger());

    EXPECT_EQ (SUCCESS, polymorphArrayInstance->GetValue (ecValue, L"InSubClass2"));
    EXPECT_EQ (5, ecValue.GetInteger());
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PolymorphismDeserializationTest, ExpectSuccessWhenDeserializingPolymorphismInstance)
    {
    
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();

    ECSchemaPtr schema;
    SchemaReadStatus schemaStatus = ECSchema::ReadFromXmlFile (schema, ECTestFixture::GetTestDataPath(L"Polymorphism.01.00.ecschema.xml").c_str(), *schemaContext);
        
    EXPECT_EQ (SCHEMA_READ_STATUS_Success, schemaStatus);

    ECInstanceReadContextPtr instanceContext = ECInstanceReadContext::CreateContext (*schema);

    IECInstancePtr  testInstance;
    InstanceReadStatus instanceStatus = IECInstance::ReadFromXmlFile (testInstance, ECTestFixture::GetTestDataPath(L"PolymorphismInstance.xml").c_str(), *instanceContext);

    EXPECT_EQ (INSTANCE_READ_STATUS_Success, instanceStatus);
    
    testInstance->ToString(L"").c_str();
    VerifyPolymorphismInstance (testInstance.get());
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(InstanceSerializationTest, ExpectSuccessWhenSerializingInstance)
    {
    
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();

    // we get the instance we want to serialize by reading the instance from XML.
    ECSchemaPtr schema;
    SchemaReadStatus schemaStatus = ECSchema::ReadFromXmlFile (schema, ECTestFixture::GetTestDataPath(L"SimpleTest_FirstSchema.01.00.ecschema.xml").c_str(), *schemaContext);
        
    EXPECT_EQ (SCHEMA_READ_STATUS_Success, schemaStatus);

    // add a binary property to the class we are deserializing.
    ECClassP    testClass;
    if (NULL != (testClass = schema->GetClassP (L"TestClass")))
        {
        PrimitiveECPropertyP binaryProperty;
        testClass->CreatePrimitiveProperty (binaryProperty, L"TestBinary", PRIMITIVETYPE_Binary);
        }
    
    ECInstanceReadContextPtr instanceContext = ECInstanceReadContext::CreateContext (*schema);

    IECInstancePtr  testInstance;
    InstanceReadStatus instanceStatus = IECInstance::ReadFromXmlFile (testInstance, ECTestFixture::GetTestDataPath(L"SimpleTest_Instance.xml").c_str(), *instanceContext);
    EXPECT_EQ (INSTANCE_READ_STATUS_Success, instanceStatus);

    ECValue     binaryValue;
    binaryValue.SetBinary (testBinaryData, _countof (testBinaryData), true);
    EXPECT_EQ (SUCCESS, testInstance->SetValue (L"TestBinary", binaryValue));
    
    // verify that the instance is correct
    VerifyTestInstance (testInstance.get(), true);

    // AZK commented out until we find a better place to write these files or write them to strings for the interim as we do in schema serialization tests
    //// now we write it out.
    //EXPECT_EQ (INSTANCE_WRITE_STATUS_Success, testInstance->WriteToXmlFile (L"c:\\temp\\OutputInstance.xml"));
    //
    //// then read it back.
    //IECInstancePtr  readbackInstance;
    //InstanceReadStatus readbackStatus = IECInstance::ReadFromXmlFile (readbackInstance, L"c:\\temp\\OutputInstance.xml", *schema);
    //
    //EXPECT_EQ (INSTANCE_READ_STATUS_Success, instanceStatus);
    //VerifyTestInstance (readbackInstance.get(), true);
    };


END_BENTLEY_ECOBJECT_NAMESPACE
