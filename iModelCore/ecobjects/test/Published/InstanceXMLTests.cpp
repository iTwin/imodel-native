
/*--------------------------------------------------------------------------------------+
|
|     $Source: ecobjects/nativeatp/Published/InstanceXMLTests.cpp $
|
|  $Copyright: (c) 2010 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsTestPCH.h"
#include <objbase.h>

BEGIN_BENTLEY_EC_NAMESPACE

struct  Struct1
    {
    bool    struct1BoolMember;
    int     struct1IntMember;
    };

struct  Struct2
    {
    bool            struct2StringMemberNull;
    const wchar_t*  struct2StringMember;
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
    int         baseClassMember;
    EXPECT_EQ (SUCCESS, testInstance->GetInteger (baseClassMember, L"BaseClassMember"));
    EXPECT_EQ (987, baseClassMember);

    int         integerMember;
    EXPECT_EQ (SUCCESS, testInstance->GetInteger (integerMember, L"IntegerMember"));
    EXPECT_EQ (12, integerMember);

    int         customFormatInt;
    EXPECT_EQ (SUCCESS, testInstance->GetInteger (customFormatInt, L"CustomFormatInt"));
    EXPECT_EQ (13, customFormatInt);

    bool        booleanMember;
    EXPECT_EQ (SUCCESS, testInstance->GetBoolean (booleanMember, L"BooleanMember"));
    EXPECT_EQ (false, booleanMember);

    double      doubleMember;
    EXPECT_EQ (SUCCESS, testInstance->GetDouble (doubleMember, L"DoubleMember"));
    EXPECT_EQ (456.789, doubleMember);

    const wchar_t*  stringMember;
    EXPECT_EQ (SUCCESS, testInstance->GetString (stringMember, L"StringMember"));
    EXPECT_STREQ (stringMember, L"Test string");

    const wchar_t*  fileNameMember;
    EXPECT_EQ (SUCCESS, testInstance->GetString (fileNameMember, L"FileNameMember"));
    EXPECT_STREQ (fileNameMember, L"c:\\usr\\barry\\test.txt");

    int         negativeMember;
    EXPECT_EQ (SUCCESS, testInstance->GetInteger (negativeMember, L"NegativeMember"));
    EXPECT_EQ (-42, negativeMember);

    Int64       dateTimeMember;
    EXPECT_EQ (SUCCESS, testInstance->GetDateTimeTicks (dateTimeMember, L"DateTimeMember"));
    EXPECT_EQ (633374681466664305, dateTimeMember);

    DPoint3d    startPoint;
    EXPECT_EQ (SUCCESS, testInstance->GetPoint3D (startPoint, L"StartPoint"));
    EXPECT_EQ (1.1, startPoint.x);
    EXPECT_EQ (2.2, startPoint.y);
    EXPECT_EQ (3.3, startPoint.z);

    DPoint3d    endPoint;
    EXPECT_EQ (SUCCESS, testInstance->GetPoint3D (endPoint, L"EndPoint"));
    EXPECT_EQ (4.4, endPoint.x);
    EXPECT_EQ (7.7, endPoint.y);
    EXPECT_EQ (6.6, endPoint.z);

    bool        structBoolMember;
    EXPECT_EQ (SUCCESS, testInstance->GetBoolean (structBoolMember, L"SecondEmbeddedStruct.Struct1BoolMember"));
    EXPECT_EQ (false, structBoolMember);

    int         structIntMember;
    EXPECT_EQ (SUCCESS, testInstance->GetInteger (structIntMember, L"SecondEmbeddedStruct.Struct1IntMember"));
    EXPECT_EQ (4, structIntMember);

    double      structDoubleMember;
    EXPECT_EQ (SUCCESS, testInstance->GetDouble (structDoubleMember, L"FormattedStruct.Struct3DoubleMember"));
    EXPECT_EQ (false, structBoolMember);

    int         struct3IntMember;
    EXPECT_EQ (SUCCESS, testInstance->GetInteger (struct3IntMember, L"FormattedStruct.Struct3IntMember"));
    EXPECT_EQ (531992, struct3IntMember);
    
    bool        struct3BoolMember;
    EXPECT_EQ (SUCCESS, testInstance->GetBoolean (struct3BoolMember, L"FormattedStruct.Struct3BoolMember"));
    EXPECT_EQ (true, struct3BoolMember);

    int         expectedInts[] = {0, 101, 202, 303, 404, 505, 606, 707, 808, 909};
    for (UInt32 index=0; index < _countof (expectedInts); index++)
        {
        int     arrayInt;
        EXPECT_EQ (SUCCESS, testInstance->GetInteger (arrayInt, L"FormattedArray[]", 1, &index));
        EXPECT_EQ (expectedInts[index], arrayInt);
        }

    int         moreInts[] = {41556, 32757, 56789, 32757, 21482 };
    for (UInt32 index=0; index < _countof (moreInts); index++)
        {
        int     arrayInt;
        EXPECT_EQ (SUCCESS, testInstance->GetInteger (arrayInt, L"IntArray[]", 1, &index));
        EXPECT_EQ (moreInts[index], arrayInt);
        }

    int     oneMemberIntArrayValue;
    UInt32  zero = 0;
    EXPECT_EQ (SUCCESS, testInstance->GetInteger (oneMemberIntArrayValue, L"OneMemberIntArray[]", 1, &zero));
    EXPECT_EQ (3, oneMemberIntArrayValue);

    DPoint3d    expectedPoints[] = { {4.0, 0.0, 0.0}, {1.0, 1.0, 1.0}, {2.0, 2.0, 2.0}, {3.0, 3.0, 3.0}, {4.0, 4.0, 4.0}, {5.0, 5.0, 5.0} };
    for (UInt32 index=0; index < _countof (expectedPoints); index++)
        {
        DPoint3d    arrayPoint;
        EXPECT_EQ (SUCCESS, testInstance->GetPoint3D (arrayPoint, L"PointArray[]", 1, &index));
        EXPECT_EQ (expectedPoints[index].x, arrayPoint.x);
        EXPECT_EQ (expectedPoints[index].y, arrayPoint.y);
        EXPECT_EQ (expectedPoints[index].z, arrayPoint.z);
        }

    for (UInt32 index=0; index < 300; index++)
        {
        Int64       arrayDateTime;
        EXPECT_EQ (SUCCESS, testInstance->GetDateTimeTicks (arrayDateTime, L"DateArray[]", 1, &index));
        }

    for (UInt32 index=0; index < 300; index++)
        {
        const wchar_t*      arrayString;
        EXPECT_EQ (SUCCESS, testInstance->GetString (arrayString, L"StringArray[]", 1, &index));
        wchar_t     expectedString[128];
        swprintf (expectedString, L"String %d", index%30);
        EXPECT_STREQ (expectedString, arrayString);
        }

    for (UInt32 index=0; index < 100; index++)
        {
        int     smallIntValue;
        EXPECT_EQ (SUCCESS, testInstance->GetInteger (smallIntValue, L"SmallIntArray[]", 1, &index));
        EXPECT_EQ (index, smallIntValue);
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
        EXPECT_EQ (SUCCESS, testInstance->GetValue (structArrayMember, L"StructArray[]", index));
        IECInstancePtr  structArrayInstance = structArrayMember.GetStruct();
        EXPECT_EQ (true, structArrayInstance.IsValid());
        
        const wchar_t*  stringValue;
        if (struct2ExpectedValues[index].struct2StringMemberNull)
            {
            // This throws an assert rather than returning ERROR.
            // EXPECT_EQ (ERROR, structArrayInstance->GetString (stringValue, L"Struct2StringMember"));
            }
        else
            {
            EXPECT_EQ (SUCCESS, structArrayInstance->GetString (stringValue, L"Struct2StringMember"));
            EXPECT_STREQ (struct2ExpectedValues[index].struct2StringMember, stringValue);
            }

        double          doubleValue;
        if (struct2ExpectedValues[index].struct2DoubleMemberNull)
            {
            // This throws an assert rather than returning ERROR.
            // EXPECT_EQ (ERROR, structArrayInstance->GetDouble (doubleValue, L"Struct2DoubleMember"));
            }
        else
            {
            EXPECT_EQ (SUCCESS, structArrayInstance->GetDouble (doubleValue, L"Struct2DoubleMember"));
            EXPECT_EQ (struct2ExpectedValues[index].struct2DoubleMember, doubleValue);
            }

        // now try to get the array of structs, if expected.
        if (NULL == struct2ExpectedValues[index].nestedArray)
            continue;

        for (UInt32 nestedIndex=0; nestedIndex < struct2ExpectedValues[index].arraySize; nestedIndex++)
            {
            ECValue     nestedArrayMember;
            EXPECT_EQ (SUCCESS, structArrayInstance->GetValue (nestedArrayMember, L"NestedArray[]", nestedIndex));
            IECInstancePtr  nestedInstance = nestedArrayMember.GetStruct();
            EXPECT_EQ (true, nestedInstance.IsValid());
            
            bool        nestedBoolMember;
            EXPECT_EQ (SUCCESS, nestedInstance->GetBoolean (nestedBoolMember, L"Struct1BoolMember"));
            EXPECT_EQ (struct2ExpectedValues[index].nestedArray[nestedIndex].struct1BoolMember, nestedBoolMember);

            int         nestedIntMember;
            EXPECT_EQ (SUCCESS, nestedInstance->GetInteger (nestedIntMember, L"Struct1IntMember"));
            EXPECT_EQ (struct2ExpectedValues[index].nestedArray[nestedIndex].struct1IntMember, nestedIntMember);
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
TEST(InstanceDeserializationTest, ExpectSuccessWhenDeserializingSimpleInstance)
    {
    // must call CoInitialize - schema deserialization requires it.
    ASSERT_HRESULT_SUCCEEDED (CoInitialize(NULL));

    ECSchemaPtr schema;        
    
    SchemaDeserializationStatus schemaStatus = ECSchema::ReadXmlFromFile (schema, L"SimpleTest_FirstSchema.01.00.ecschema.xml", NULL, NULL);
        
    EXPECT_EQ (SCHEMA_DESERIALIZATION_STATUS_Success, schemaStatus);

    IECInstancePtr  testInstance;
    InstanceDeserializationStatus instanceStatus = IECInstance::ReadXmlFromFile (testInstance, L"SimpleTest_Instance.xml", schema.get());

    EXPECT_EQ (INSTANCE_DESERIALIZATION_STATUS_Success, instanceStatus);
    
    testInstance->Dump();
    VerifyTestInstance (testInstance.get(), false);
    };



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/10
+---------------+---------------+---------------+---------------+---------------+------*/
void    VerifyPolymorphismInstance (IECInstanceCP testInstance)
    {
    int         inRootClass;
    EXPECT_EQ (SUCCESS, testInstance->GetInteger (inRootClass, L"InRootClass"));
    EXPECT_EQ (-1, inRootClass);

    // get and check the first member.
    ECValue         polymorphArrayMember;
    EXPECT_EQ (SUCCESS, testInstance->GetValue (polymorphArrayMember, L"PolymorphArray[]", 0));
    IECInstancePtr  polymorphArrayInstance = polymorphArrayMember.GetStruct();
    EXPECT_EQ (true, polymorphArrayInstance.IsValid());
    ECClassCR       arrayMember0Class = polymorphArrayInstance->GetClass();
    EXPECT_STREQ (L"BaseClass", arrayMember0Class.Name.c_str());

    int         inBaseClass;
    EXPECT_EQ (SUCCESS, polymorphArrayInstance->GetInteger (inBaseClass, L"InBaseClass"));
    EXPECT_EQ (0, inBaseClass);

    // get and check the second member.
    EXPECT_EQ (SUCCESS, testInstance->GetValue (polymorphArrayMember, L"PolymorphArray[]", 1));
    polymorphArrayInstance = polymorphArrayMember.GetStruct();
    EXPECT_EQ (true, polymorphArrayInstance.IsValid());
    ECClassCR arrayMember1Class = polymorphArrayInstance->GetClass();
    EXPECT_STREQ (L"SubClass1", arrayMember1Class.Name.c_str());

    EXPECT_EQ (SUCCESS, polymorphArrayInstance->GetInteger (inBaseClass, L"InBaseClass"));
    EXPECT_EQ (1, inBaseClass);

    int         inSubClass1;
    EXPECT_EQ (SUCCESS, polymorphArrayInstance->GetInteger (inSubClass1, L"InSubClass1"));
    EXPECT_EQ (2, inSubClass1);

    // get and check the third member.
    EXPECT_EQ (SUCCESS, testInstance->GetValue (polymorphArrayMember, L"PolymorphArray[]", 2));
    polymorphArrayInstance = polymorphArrayMember.GetStruct();
    EXPECT_EQ (true, polymorphArrayInstance.IsValid());
    ECClassCR arrayMember2Class = polymorphArrayInstance->GetClass();
    EXPECT_STREQ (L"SubClass2", arrayMember2Class.Name.c_str());

    EXPECT_EQ (SUCCESS, polymorphArrayInstance->GetInteger (inBaseClass, L"InBaseClass"));
    EXPECT_EQ (3, inBaseClass);

    EXPECT_EQ (SUCCESS, polymorphArrayInstance->GetInteger (inSubClass1, L"InSubClass1"));
    EXPECT_EQ (4, inSubClass1);

    int         inSubClass2;
    EXPECT_EQ (SUCCESS, polymorphArrayInstance->GetInteger (inSubClass2, L"InSubClass2"));
    EXPECT_EQ (5, inSubClass2);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(PolymorphismDeserializationTest, ExpectSuccessWhenDeserializingPolymorphismInstance)
    {
    // must call CoInitialize - schema deserialization requires it.
    ASSERT_HRESULT_SUCCEEDED (CoInitialize(NULL));

    ECSchemaPtr schema;        
    
    SchemaDeserializationStatus schemaStatus = ECSchema::ReadXmlFromFile (schema, L"Polymorphism.01.00.ecschema.xml", NULL, NULL);
        
    EXPECT_EQ (SCHEMA_DESERIALIZATION_STATUS_Success, schemaStatus);

    IECInstancePtr  testInstance;
    InstanceDeserializationStatus instanceStatus = IECInstance::ReadXmlFromFile (testInstance, L"PolymorphismInstance.xml", schema.get());

    EXPECT_EQ (INSTANCE_DESERIALIZATION_STATUS_Success, instanceStatus);
    
    testInstance->Dump();
    VerifyPolymorphismInstance (testInstance.get());
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(InstanceSerializationTest, ExpectSuccessWhenSerializingInstance)
    {
    // must call CoInitialize - schema deserialization requires it.
    ASSERT_HRESULT_SUCCEEDED (CoInitialize(NULL));

    ECSchemaPtr schema;        

    // we get the instance we want to serialize by reading the instance from XML.
    SchemaDeserializationStatus schemaStatus = ECSchema::ReadXmlFromFile (schema, L"SimpleTest_FirstSchema.01.00.ecschema.xml", NULL, NULL);
        
    EXPECT_EQ (SCHEMA_DESERIALIZATION_STATUS_Success, schemaStatus);

    // add a binary property to the class we are deserializing.
    ECClassP    testClass;
    if (NULL != (testClass = schema->GetClassP (L"TestClass")))
        {
        PrimitiveECPropertyP binaryProperty;
        testClass->CreatePrimitiveProperty (binaryProperty, L"TestBinary", PRIMITIVETYPE_Binary);
        }
    
    IECInstancePtr  testInstance;
    InstanceDeserializationStatus instanceStatus = IECInstance::ReadXmlFromFile (testInstance, L"SimpleTest_Instance.xml", schema.get());
    EXPECT_EQ (INSTANCE_DESERIALIZATION_STATUS_Success, instanceStatus);

    ECValue     binaryValue;
    binaryValue.SetBinary (testBinaryData, _countof (testBinaryData), true);
    EXPECT_EQ (SUCCESS, testInstance->SetValue (L"TestBinary", binaryValue));
    
    // verify that the instance is correct
    VerifyTestInstance (testInstance.get(), true);

    // now we write it out.
    EXPECT_EQ (INSTANCE_SERIALIZATION_STATUS_Success, testInstance->WriteXmlToFile (L"c:\\temp\\OutputInstance.xml"));
    
    // then read it back.
    IECInstancePtr  readbackInstance;
    InstanceDeserializationStatus readbackStatus = IECInstance::ReadXmlFromFile (readbackInstance, L"c:\\temp\\OutputInstance.xml", schema.get());

    EXPECT_EQ (INSTANCE_DESERIALIZATION_STATUS_Success, instanceStatus);
    VerifyTestInstance (readbackInstance.get(), true);
    };


END_BENTLEY_EC_NAMESPACE
