/*--------------------------------------------------------------------------------------+
|
|     $Source: test/Published/InstanceTests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"

using namespace ECN;

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

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


/*---------------------------------------------------------------------------------**//**
* @bsistruct                                               Raimondas.Rimkus   02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct InstanceTests : ECTestFixture
    {
    ECSchemaPtr          m_schema;
    ECClassP             m_ecClass;
    IECInstancePtr       m_instance;
    uint32_t             propIndex;

    void CreateSchema (Utf8String schemaName = "TestSchema", Utf8String className = "TestClass")
        {
        ECSchema::CreateSchema (m_schema, schemaName, 1, 0);
        m_schema->CreateClass (m_ecClass, className);
        }

    void CreateInstance ()
        {
        m_instance = m_ecClass->GetDefaultStandaloneEnabler ()->CreateInstance ();
        }

    PrimitiveECPropertyP CreateProperty (Utf8CP name, PrimitiveType primitiveType)
        {
        PrimitiveECPropertyP prop;
        m_ecClass->CreatePrimitiveProperty (prop, name, primitiveType);
        m_ecClass->GetDefaultStandaloneEnabler ()->GetPropertyIndex (propIndex, name);
        return prop;
        }

    PrimitiveECPropertyP CreateProperty (Utf8CP name)
        {
        PrimitiveECPropertyP prop;
        m_ecClass->CreatePrimitiveProperty (prop, name);
        m_ecClass->GetDefaultStandaloneEnabler ()->GetPropertyIndex (propIndex, name);
        return prop;
        }

    ArrayECPropertyP CreateArrayProperty (Utf8CP name)
        {
        ArrayECPropertyP prop;
        m_ecClass->CreateArrayProperty (prop, name);
        m_ecClass->GetDefaultStandaloneEnabler ()->GetPropertyIndex (propIndex, name);
        return prop;
        }
    };

struct PropertyTests : InstanceTests {};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/10
+---------------+---------------+---------------+---------------+---------------+------*/
void VerifyTestInstance (IECInstanceCP testInstance)
    {
    ECValue ecValue;

    EXPECT_EQ (SUCCESS, testInstance->GetValue (ecValue, "BaseClassMember"));
    EXPECT_EQ (987, ecValue.GetInteger ());

    EXPECT_EQ (SUCCESS, testInstance->GetValue (ecValue, "IntegerMember"));
    EXPECT_EQ (12, ecValue.GetInteger ());

    EXPECT_EQ (SUCCESS, testInstance->GetValue (ecValue, "CustomFormatInt"));
    EXPECT_EQ (13, ecValue.GetInteger ());

    EXPECT_EQ (SUCCESS, testInstance->GetValue (ecValue, "BooleanMember"));
    EXPECT_FALSE (ecValue.GetBoolean ());

    EXPECT_EQ (SUCCESS, testInstance->GetValue (ecValue, "DoubleMember"));
    EXPECT_EQ (456.789, ecValue.GetDouble ());

    EXPECT_EQ (SUCCESS, testInstance->GetValue (ecValue, "StringMember"));
    EXPECT_STREQ (ecValue.GetUtf8CP (), "Test string");

    EXPECT_EQ (SUCCESS, testInstance->GetValue (ecValue, "FileNameMember"));
    EXPECT_STREQ (ecValue.GetUtf8CP (), "c:\\usr\\barry\\test.txt");

    EXPECT_EQ (SUCCESS, testInstance->GetValue (ecValue, "NegativeMember"));
    EXPECT_EQ (-42, ecValue.GetInteger ());

    EXPECT_EQ (SUCCESS, testInstance->GetValue (ecValue, "DateTimeMember"));
    EXPECT_EQ (633374681466664305, ecValue.GetDateTimeTicks ());

    EXPECT_EQ (SUCCESS, testInstance->GetValue (ecValue, "StartPoint"));
    EXPECT_EQ (1.1, ecValue.GetPoint3D ().x);
    EXPECT_EQ (2.2, ecValue.GetPoint3D ().y);
    EXPECT_EQ (3.3, ecValue.GetPoint3D ().z);

    EXPECT_EQ (SUCCESS, testInstance->GetValue (ecValue, "EndPoint"));
    EXPECT_EQ (4.4, ecValue.GetPoint3D ().x);
    EXPECT_EQ (7.7, ecValue.GetPoint3D ().y);
    EXPECT_EQ (6.6, ecValue.GetPoint3D ().z);

    EXPECT_EQ (SUCCESS, testInstance->GetValue (ecValue, "SecondEmbeddedStruct.Struct1BoolMember"));
    EXPECT_FALSE (ecValue.GetBoolean ());

    EXPECT_EQ (SUCCESS, testInstance->GetValue (ecValue, "SecondEmbeddedStruct.Struct1IntMember"));
    EXPECT_EQ (4, ecValue.GetInteger ());

    EXPECT_EQ (SUCCESS, testInstance->GetValue (ecValue, "FormattedStruct.Struct3DoubleMember"));
    EXPECT_EQ (17.443, ecValue.GetDouble ());

    EXPECT_EQ (SUCCESS, testInstance->GetValue (ecValue, "FormattedStruct.Struct3IntMember"));
    EXPECT_EQ (531992, ecValue.GetInteger ());

    EXPECT_EQ (SUCCESS, testInstance->GetValue (ecValue, "FormattedStruct.Struct3BoolMember"));
    EXPECT_TRUE (ecValue.GetBoolean ());

    int         expectedInts[] = { 0, 101, 202, 303, 404, 505, 606, 707, 808, 909 };
    for (uint32_t index = 0; index < _countof (expectedInts); index++)
        {
        EXPECT_EQ (SUCCESS, testInstance->GetValue (ecValue, "FormattedArray", index));
        EXPECT_EQ (expectedInts[index], ecValue.GetInteger ());
        }

    int         moreInts[] = { 41556, 32757, 56789, 32757, 21482 };
    for (uint32_t index = 0; index < _countof (moreInts); index++)
        {
        EXPECT_EQ (SUCCESS, testInstance->GetValue (ecValue, "IntArray", index));
        EXPECT_EQ (moreInts[index], ecValue.GetInteger ());
        }

    EXPECT_EQ (SUCCESS, testInstance->GetValue (ecValue, "OneMemberIntArray", 0));
    EXPECT_EQ (3, ecValue.GetInteger ());

    DPoint3d    expectedPoints[] = { { 4.0, 0.0, 0.0 }, { 1.0, 1.0, 1.0 }, { 2.0, 2.0, 2.0 }, { 3.0, 3.0, 3.0 }, { 4.0, 4.0, 4.0 }, { 5.0, 5.0, 5.0 } };
    for (uint32_t index = 0; index < _countof (expectedPoints); index++)
        {
        EXPECT_EQ (SUCCESS, testInstance->GetValue (ecValue, "PointArray", index));
        EXPECT_EQ (expectedPoints[index].x, ecValue.GetPoint3D ().x);
        EXPECT_EQ (expectedPoints[index].y, ecValue.GetPoint3D ().y);
        EXPECT_EQ (expectedPoints[index].z, ecValue.GetPoint3D ().z);
        }

    for (uint32_t index = 0; index < 300; index++)
        {
        EXPECT_EQ (SUCCESS, testInstance->GetValue (ecValue, "DateArray", index));
        }

    for (uint32_t index = 0; index < 300; index++)
        {
        EXPECT_EQ (SUCCESS, testInstance->GetValue (ecValue, "StringArray", index));
        Utf8Char     expectedString[128];
        sprintf (expectedString, "String %d", index % 30);
        EXPECT_STREQ (expectedString, ecValue.GetUtf8CP ());
        }

    for (uint32_t index = 0; index < 100; index++)
        {
        EXPECT_EQ (SUCCESS, testInstance->GetValue (ecValue, "SmallIntArray", index));
        EXPECT_EQ (index, ecValue.GetInteger ());
        }

    // test the structArray of Struct2's that contains a nested array of Struct1's. Here's the data we expect:
    Struct1  firstArray[] =
        { { true, 0 }, { false, 10 }, { true, 20 }, { false, 30 } };

    Struct1  secondArray[] =
        { { false, 0 }, { true, 101 }, { false, 202 }, { true, 303 }, { false, 404 } };

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

    for (uint32_t index = 0; index < _countof (struct2ExpectedValues); index++)
        {
        ECValue         structArrayMember;
        EXPECT_EQ (SUCCESS, testInstance->GetValue (structArrayMember, "StructArray", index));
        IECInstancePtr  structArrayInstance = structArrayMember.GetStruct ();
        EXPECT_TRUE (structArrayInstance.IsValid ());

        if (struct2ExpectedValues[index].struct2StringMemberNull)
            {
            // This throws an assert rather than returning ERROR.
            // EXPECT_EQ (ERROR, structArrayInstance->GetString (stringValue, "Struct2StringMember"));
            }
        else
            {
            EXPECT_EQ (SUCCESS, structArrayInstance->GetValue (ecValue, "Struct2StringMember"));
            EXPECT_STREQ (struct2ExpectedValues[index].struct2StringMember, ecValue.GetUtf8CP ());
            }

        if (struct2ExpectedValues[index].struct2DoubleMemberNull)
            {
            // This throws an assert rather than returning ERROR.
            // EXPECT_EQ (ERROR, structArrayInstance->GetDouble (doubleValue, "Struct2DoubleMember"));
            }
        else
            {
            EXPECT_EQ (SUCCESS, structArrayInstance->GetValue (ecValue, "Struct2DoubleMember"));
            EXPECT_EQ (struct2ExpectedValues[index].struct2DoubleMember, ecValue.GetDouble ());
            }

        // now try to get the array of structs, if expected.
        if (NULL == struct2ExpectedValues[index].nestedArray)
            continue;

        for (uint32_t nestedIndex = 0; nestedIndex < struct2ExpectedValues[index].arraySize; nestedIndex++)
            {
            ECValue     nestedArrayMember;
            EXPECT_EQ (SUCCESS, structArrayInstance->GetValue (nestedArrayMember, "NestedArray", nestedIndex));
            IECInstancePtr  nestedInstance = nestedArrayMember.GetStruct ();
            EXPECT_TRUE (nestedInstance.IsValid ());

            EXPECT_EQ (SUCCESS, nestedInstance->GetValue (ecValue, "Struct1BoolMember"));
            EXPECT_EQ (struct2ExpectedValues[index].nestedArray[nestedIndex].struct1BoolMember, ecValue.GetBoolean ());

            EXPECT_EQ (SUCCESS, nestedInstance->GetValue (ecValue, "Struct1IntMember"));
            EXPECT_EQ (struct2ExpectedValues[index].nestedArray[nestedIndex].struct1IntMember, ecValue.GetInteger ());
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                               Raimondas.Rimkus   02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (InstanceTests, TestIsNullPrimitive)
    {
    CreateSchema ();
    CreateProperty ("PropertyString", PRIMITIVETYPE_String);
    CreateInstance ();

    bool isNull = NULL;

    EXPECT_EQ (m_instance->IsPropertyNull (isNull, "PropertyString"), ECOBJECTS_STATUS_Success);
    EXPECT_TRUE (isNull);
    EXPECT_EQ (m_instance->IsPropertyNull (isNull, propIndex), ECOBJECTS_STATUS_Success);
    EXPECT_TRUE (isNull);

    EXPECT_EQ (m_instance->SetValue ("PropertyString", ECValue ("Some value")), ECOBJECTS_STATUS_Success);

    EXPECT_EQ (m_instance->IsPropertyNull (isNull, "PropertyString"), ECOBJECTS_STATUS_Success);
    EXPECT_FALSE (isNull);
    EXPECT_EQ (m_instance->IsPropertyNull (isNull, propIndex), ECOBJECTS_STATUS_Success);
    EXPECT_FALSE (isNull);
    }

/*---------------------------------------------------------------------------------**//**
* The array property value itself is *never* null.
* @bsimethod                                                 Raimondas.Rimkus 02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (InstanceTests, TestIsNullArray)
    {
    CreateSchema ();
    CreateArrayProperty ("PropertyArray");
    CreateInstance ();

    bool isNull = false;

    EXPECT_EQ (m_instance->IsPropertyNull (isNull, "PropertyArray"), ECOBJECTS_STATUS_Success);
    EXPECT_FALSE (isNull);
    EXPECT_EQ (m_instance->IsPropertyNull (isNull, propIndex), ECOBJECTS_STATUS_Success);
    EXPECT_FALSE (isNull);

    EXPECT_EQ (m_instance->AddArrayElements ("PropertyArray", 15), ECOBJECTS_STATUS_Success);
    EXPECT_EQ (m_instance->SetValue ("PropertyArray", ECValue ("Some value"), 3), ECOBJECTS_STATUS_Success);

    //Strangelly array property doesn't exist as PrimitiveProperty
    EXPECT_EQ (m_instance->IsPropertyNull (isNull, "PropertyArray"), ECOBJECTS_STATUS_Success);
    EXPECT_FALSE (isNull);
    EXPECT_EQ (m_instance->IsPropertyNull (isNull, propIndex), ECOBJECTS_STATUS_Success);
    EXPECT_FALSE (isNull);
    //-------------------------------------------------------------

    EXPECT_EQ (m_instance->IsPropertyNull (isNull, "PropertyArray", 3), ECOBJECTS_STATUS_Success);
    EXPECT_FALSE (isNull);
    EXPECT_EQ (m_instance->IsPropertyNull (isNull, propIndex, 3), ECOBJECTS_STATUS_Success);
    EXPECT_FALSE (isNull);

    EXPECT_EQ (m_instance->IsPropertyNull (isNull, "PropertyArray", 13), ECOBJECTS_STATUS_Success);
    EXPECT_TRUE (isNull);
    EXPECT_EQ (m_instance->IsPropertyNull (isNull, propIndex, 13), ECOBJECTS_STATUS_Success);
    EXPECT_TRUE (isNull);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                 Raimondas.Rimkus 02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (InstanceTests, TestArraySize)
    {
    CreateSchema ();
    CreateArrayProperty ("PropertyArray");
    CreateInstance ();

    ECValue arrayVal;

    EXPECT_EQ (m_instance->GetValue (arrayVal, "PropertyArray"), ECOBJECTS_STATUS_Success);
    EXPECT_TRUE (arrayVal.IsArray ());
    EXPECT_EQ (arrayVal.GetArrayInfo ().GetCount (), 0);

    EXPECT_EQ (m_instance->AddArrayElements ("PropertyArray", 13), ECOBJECTS_STATUS_Success);

    EXPECT_EQ (m_instance->GetValue (arrayVal, "PropertyArray"), ECOBJECTS_STATUS_Success);
    EXPECT_TRUE (arrayVal.IsArray ());
    EXPECT_EQ (arrayVal.GetArrayInfo ().GetCount (), 13);

    EXPECT_EQ (m_instance->ClearArray ("PropertyArray"), ECOBJECTS_STATUS_Success);

    EXPECT_EQ (m_instance->GetValue (arrayVal, "PropertyArray"), ECOBJECTS_STATUS_Success);
    EXPECT_TRUE (arrayVal.IsArray ());
    EXPECT_EQ (arrayVal.GetArrayInfo ().GetCount (), 0);

    EXPECT_EQ (m_instance->AddArrayElements (propIndex, 42), ECOBJECTS_STATUS_Success);

    EXPECT_EQ (m_instance->GetValue (arrayVal, propIndex), ECOBJECTS_STATUS_Success);
    EXPECT_TRUE (arrayVal.IsArray ());
    EXPECT_EQ (arrayVal.GetArrayInfo ().GetCount (), 42);

    EXPECT_EQ (m_instance->ClearArray (propIndex), ECOBJECTS_STATUS_Success);

    EXPECT_EQ (m_instance->GetValue (arrayVal, propIndex), ECOBJECTS_STATUS_Success);
    EXPECT_TRUE (arrayVal.IsArray ());
    EXPECT_EQ (arrayVal.GetArrayInfo ().GetCount (), 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                 Raimondas.Rimkus 02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (InstanceTests, TestSetValueString)
    {
    CreateSchema ();
    CreateProperty ("Property_1");
    CreateInstance ();

    ECValue value;

    EXPECT_EQ (m_instance->SetValue ("Property_1", ECValue ("Some value 1")), ECOBJECTS_STATUS_Success);
    EXPECT_EQ (m_instance->GetValue (value, "Property_1"), ECOBJECTS_STATUS_Success);
    EXPECT_STREQ (value.GetUtf8CP (), "Some value 1");

    EXPECT_EQ (m_instance->SetValue (propIndex, ECValue ("Some value 2")), ECOBJECTS_STATUS_Success);
    EXPECT_EQ (m_instance->GetValue (value, propIndex), ECOBJECTS_STATUS_Success);
    EXPECT_STREQ (value.GetUtf8CP (), "Some value 2");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                 Raimondas.Rimkus 02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (InstanceTests, TestSetDisplayLabel)
    {
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext ();
    SchemaKey schemaKey ("Bentley_Standard_CustomAttributes", 1, 5);
    ECSchemaPtr customAttributesSchema = context->LocateSchema (schemaKey, SCHEMAMATCHTYPE_Latest);

    StandaloneECEnablerP m_customAttributeEnabler = customAttributesSchema->GetClassP ("InstanceLabelSpecification")->GetDefaultStandaloneEnabler ();
    CreateSchema ();
    m_schema->AddReferencedSchema (*customAttributesSchema);

    PrimitiveECPropertyP prop;
    m_ecClass->CreatePrimitiveProperty (prop, "InstanceLabel", PRIMITIVETYPE_String);

    IECInstancePtr labelAttr = m_customAttributeEnabler->CreateInstance ();
    labelAttr->SetValue ("PropertyName", ECValue ("InstanceLabel"));
    m_ecClass->SetCustomAttribute (*labelAttr);

    IECInstancePtr m_instance = m_ecClass->GetDefaultStandaloneEnabler ()->CreateInstance ();

    Utf8String displayLabel;
    EXPECT_EQ (m_instance->GetDisplayLabel (displayLabel), ECOBJECTS_STATUS_Success);
    EXPECT_TRUE (displayLabel.Equals (m_ecClass->GetDisplayLabel ()));

    EXPECT_EQ (m_instance->SetDisplayLabel ("Some fancy instance label"), ECOBJECTS_STATUS_Success);

    EXPECT_EQ (m_instance->GetDisplayLabel (displayLabel), ECOBJECTS_STATUS_Success);
    EXPECT_STREQ (displayLabel.c_str (), "Some fancy instance label");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                 Raimondas.Rimkus 02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (InstanceTests, InstanceWriteReadFile)
    {
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext ();

    ECSchemaPtr schema;
    SchemaReadStatus schemaStatus = ECSchema::ReadFromXmlFile (schema, ECTestFixture::GetTestDataPath (L"SimpleTest_FirstSchema.01.00.ecschema.xml").c_str (), *schemaContext);

    EXPECT_EQ (SCHEMA_READ_STATUS_Success, schemaStatus);

    ECInstanceReadContextPtr instanceContext = ECInstanceReadContext::CreateContext (*schema);

    IECInstancePtr  testInstance;
    InstanceReadStatus instanceStatus = IECInstance::ReadFromXmlFile (testInstance, ECTestFixture::GetTestDataPath (L"SimpleTest_Instance.xml").c_str (), *instanceContext);
    EXPECT_EQ (INSTANCE_READ_STATUS_Success, instanceStatus);

    VerifyTestInstance (testInstance.get ());

    EXPECT_EQ (INSTANCE_WRITE_STATUS_Success, testInstance->WriteToXmlFile (ECTestFixture::GetTempDataPath (L"OutputInstance.xml").c_str (), true, false));
    IECInstancePtr  readbackInstance;
    InstanceReadStatus readbackStatus = IECInstance::ReadFromXmlFile (readbackInstance, ECTestFixture::GetTempDataPath (L"OutputInstance.xml").c_str (), *instanceContext);

    EXPECT_EQ (INSTANCE_READ_STATUS_Success, readbackStatus);
    VerifyTestInstance (readbackInstance.get ());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                 Raimondas.Rimkus 02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (PropertyTests, SetReadOnlyAndSetValue)
    {
    CreateSchema ();
    CreateProperty ("PropertyString", PRIMITIVETYPE_String);
    m_ecClass->GetPropertyP ("PropertyString")->SetIsReadOnly (true);
    CreateInstance ();

    EXPECT_TRUE (m_ecClass->GetPropertyP ("PropertyString")->GetIsReadOnly ());

    EXPECT_EQ (ECOBJECTS_STATUS_UnableToSetReadOnlyInstance, m_instance->SetValue ("PropertyString", ECValue ("Some value")));
    ECValue getValue;
    EXPECT_EQ (m_instance->GetValue (getValue, "PropertyString"), ECOBJECTS_STATUS_Success);
    EXPECT_STREQ (getValue.GetUtf8CP (), "Some value");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                 Raimondas.Rimkus 02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (PropertyTests, SetReadOnlyAndChangeValue)
    {
    CreateSchema ();
    CreateProperty ("PropertyString", PRIMITIVETYPE_String);
    m_ecClass->GetPropertyP ("PropertyString")->SetIsReadOnly (true);
    CreateInstance ();

    EXPECT_TRUE (m_ecClass->GetPropertyP ("PropertyString")->GetIsReadOnly ());

    EXPECT_EQ (ECOBJECTS_STATUS_UnableToSetReadOnlyInstance, m_instance->ChangeValue ("PropertyString", ECValue ("Other value")));
    ECValue getValue;
    EXPECT_EQ (m_instance->GetValue (getValue, "PropertyString"), ECOBJECTS_STATUS_Success);
    EXPECT_STREQ (getValue.GetUtf8CP (), "Other value");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                 Raimondas.Rimkus 02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (PropertyTests, GetValueFromInstance)
    {
    CreateSchema ();
    CreateProperty ("Property_1");
    CreateInstance ();

    m_instance->SetValue ("Property_1", ECValue ("Some value"));
    ECPropertyValuePtr propValue = ECPropertyValue::GetPropertyValue (*m_instance, "Property_1");

    IECInstanceCR instance = propValue->GetInstance ();
    EXPECT_STREQ (m_instance->GetInstanceId ().c_str (), instance.GetInstanceId ().c_str ());
    ECValueCR value = propValue->GetValue ();
    EXPECT_STREQ (value.ToString ().c_str (), "Some value");
    ECValueAccessorCR accessor = propValue->GetValueAccessor ();
    EXPECT_STREQ (accessor.GetAccessString (), "Property_1");
    EXPECT_FALSE (propValue->HasChildValues ());
    }

END_BENTLEY_ECN_TEST_NAMESPACE


