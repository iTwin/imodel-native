/*--------------------------------------------------------------------------------------+
|
|     $Source: test/scenario/InstanceTests.cpp $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsTestPCH.h"
#include "TestFixture.h"

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

struct  Struct1
    {
    bool    struct1BoolMember;
    int     struct1IntMember;
    };

struct  Struct2
    {
    bool            struct2StringMemberNull;
    WCharCP         struct2StringMember;
    bool            struct2DoubleMemberNull;
    double          struct2DoubleMember;
    Struct1*        nestedArray;
    UInt32          arraySize;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/10
+---------------+---------------+---------------+---------------+---------------+------*/
void VerifyTestInstance (IECInstanceCP testInstance)
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
    }

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                               Raimondas.Rimkus   02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct InstanceTests : ECTestFixture
    {
    ECSchemaPtr          m_schema;
    ECClassP             m_ecClass;
    IECInstancePtr       m_instance;
    UInt32               propIndex;

    void CreateSchema(WString schemaName = L"TestSchema", WString className = L"TestClass")
        {
        ECSchema::CreateSchema (m_schema, schemaName, 1, 0);
        m_schema->CreateClass (m_ecClass, className);
        }
    
    void CreateInstance()
        {
        m_instance = m_ecClass->GetDefaultStandaloneEnabler()->CreateInstance();
        }
        
    PrimitiveECPropertyP CreateProperty(WCharCP name, PrimitiveType primitiveType)
        {
        PrimitiveECPropertyP prop;
        m_ecClass->CreatePrimitiveProperty (prop, name, primitiveType);
        m_ecClass->GetDefaultStandaloneEnabler()->GetPropertyIndex(propIndex, name);
        return prop;
        }
        
    PrimitiveECPropertyP CreateProperty(WCharCP name)
        {
        PrimitiveECPropertyP prop;
        m_ecClass->CreatePrimitiveProperty (prop, name);
        m_ecClass->GetDefaultStandaloneEnabler()->GetPropertyIndex(propIndex, name);
        return prop;
        }
        
    ArrayECPropertyP CreateArrayProperty(WCharCP name)
        {
        ArrayECPropertyP prop;
        m_ecClass->CreateArrayProperty (prop, name);
        m_ecClass->GetDefaultStandaloneEnabler()->GetPropertyIndex(propIndex, name);
        return prop;
        }
    };

struct PropertyTests : InstanceTests{};
    
/*---------------------------------------------------------------------------------**//**
* @bsistruct                                               Raimondas.Rimkus   02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (InstanceTests, TestIsNullPrimitive)
    {
    CreateSchema();
    CreateProperty(L"PropertyString", PRIMITIVETYPE_String);
    CreateInstance();
    
    bool isNull = NULL;
    
    EXPECT_EQ (m_instance->IsPropertyNull(isNull, L"PropertyString"), ECOBJECTS_STATUS_Success);
    EXPECT_TRUE (isNull);
    EXPECT_EQ (m_instance->IsPropertyNull(isNull, propIndex), ECOBJECTS_STATUS_Success);
    EXPECT_TRUE (isNull);
    
    EXPECT_EQ (m_instance->SetValue (L"PropertyString", ECValue(L"Some value")), ECOBJECTS_STATUS_Success);
    
    EXPECT_EQ (m_instance->IsPropertyNull(isNull, L"PropertyString"), ECOBJECTS_STATUS_Success);
    EXPECT_FALSE (isNull);
    EXPECT_EQ (m_instance->IsPropertyNull(isNull, propIndex), ECOBJECTS_STATUS_Success);
    EXPECT_FALSE (isNull);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                 Raimondas.Rimkus 02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (InstanceTests, TestIsNullArray)
    {
    CreateSchema();
    CreateArrayProperty(L"PropertyArray");
    CreateInstance();
    
    bool isNull;
    
    EXPECT_EQ (m_instance->IsPropertyNull(isNull, L"PropertyArray"), ECOBJECTS_STATUS_Success);
    EXPECT_TRUE (isNull);
    EXPECT_EQ (m_instance->IsPropertyNull(isNull, propIndex), ECOBJECTS_STATUS_Success);
    EXPECT_TRUE (isNull);
    
    EXPECT_EQ (m_instance->AddArrayElements (L"PropertyArray", 15), ECOBJECTS_STATUS_Success);
    EXPECT_EQ (m_instance->SetValue (L"PropertyArray", ECValue(L"Some value"), 3), ECOBJECTS_STATUS_Success);
    
    //Strangelly array property doesn't exist as PrimitiveProperty
    EXPECT_EQ (m_instance->IsPropertyNull(isNull, L"PropertyArray"), ECOBJECTS_STATUS_Success);
    EXPECT_TRUE (isNull);
    EXPECT_EQ (m_instance->IsPropertyNull(isNull, propIndex), ECOBJECTS_STATUS_Success);
    EXPECT_TRUE (isNull);
    //-------------------------------------------------------------
    
    EXPECT_EQ (m_instance->IsPropertyNull(isNull, L"PropertyArray", 3), ECOBJECTS_STATUS_Success);
    EXPECT_FALSE (isNull);
    EXPECT_EQ (m_instance->IsPropertyNull(isNull, propIndex, 3), ECOBJECTS_STATUS_Success);
    EXPECT_FALSE (isNull);
    
    EXPECT_EQ (m_instance->IsPropertyNull(isNull, L"PropertyArray", 13), ECOBJECTS_STATUS_Success);
    EXPECT_TRUE (isNull);
    EXPECT_EQ (m_instance->IsPropertyNull(isNull, propIndex, 13), ECOBJECTS_STATUS_Success);
    EXPECT_TRUE (isNull);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                 Raimondas.Rimkus 02/2013
+---------------+---------------+---------------+---------------+---------------+------*/    
TEST_F (InstanceTests, TestArraySize)
    {
    CreateSchema();
    CreateArrayProperty(L"PropertyArray");
    CreateInstance();
    
    ECValue arrayVal;
    
    EXPECT_EQ (m_instance->GetValue (arrayVal, L"PropertyArray"), ECOBJECTS_STATUS_Success);
    EXPECT_TRUE (arrayVal.IsArray());
    EXPECT_EQ (arrayVal.GetArrayInfo().GetCount(), 0);
    
    EXPECT_EQ (m_instance->AddArrayElements (L"PropertyArray", 13), ECOBJECTS_STATUS_Success);
    
    EXPECT_EQ (m_instance->GetValue (arrayVal, L"PropertyArray"), ECOBJECTS_STATUS_Success);
    EXPECT_TRUE (arrayVal.IsArray());
    EXPECT_EQ (arrayVal.GetArrayInfo().GetCount(), 13);
    
    EXPECT_EQ (m_instance->ClearArray (L"PropertyArray"), ECOBJECTS_STATUS_Success);
    
    EXPECT_EQ (m_instance->GetValue (arrayVal, L"PropertyArray"), ECOBJECTS_STATUS_Success);
    EXPECT_TRUE (arrayVal.IsArray());
    EXPECT_EQ (arrayVal.GetArrayInfo().GetCount(), 0);
    
    EXPECT_EQ (m_instance->AddArrayElements (propIndex, 42), ECOBJECTS_STATUS_Success);
    
    EXPECT_EQ (m_instance->GetValue (arrayVal, propIndex), ECOBJECTS_STATUS_Success);
    EXPECT_TRUE (arrayVal.IsArray());
    EXPECT_EQ (arrayVal.GetArrayInfo().GetCount(), 42);
    
    EXPECT_EQ (m_instance->ClearArray (propIndex), ECOBJECTS_STATUS_Success);
    
    EXPECT_EQ (m_instance->GetValue (arrayVal, propIndex), ECOBJECTS_STATUS_Success);
    EXPECT_TRUE (arrayVal.IsArray());
    EXPECT_EQ (arrayVal.GetArrayInfo().GetCount(), 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                 Raimondas.Rimkus 02/2013
+---------------+---------------+---------------+---------------+---------------+------*/    
TEST_F (InstanceTests, TestSetValueString)
    {
    CreateSchema();
    CreateProperty(L"Property_1");
    CreateInstance();
    
    ECValue value;
    
    EXPECT_EQ (m_instance->SetValue (L"Property_1", ECValue(L"Some value 1")), ECOBJECTS_STATUS_Success);
    EXPECT_EQ (m_instance->GetValue (value, L"Property_1"), ECOBJECTS_STATUS_Success);
    EXPECT_STREQ (value.GetString(), L"Some value 1");
    
    EXPECT_EQ (m_instance->SetValue (propIndex, ECValue(L"Some value 2")), ECOBJECTS_STATUS_Success);
    EXPECT_EQ (m_instance->GetValue (value, propIndex), ECOBJECTS_STATUS_Success);
    EXPECT_STREQ (value.GetString(), L"Some value 2");
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                 Raimondas.Rimkus 02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (InstanceTests, TestSetDisplayLabel)
    {
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    SchemaKey schemaKey (L"Bentley_Standard_CustomAttributes", 1, 5);
    ECSchemaPtr customAttributesSchema = context->LocateSchema (schemaKey, SCHEMAMATCHTYPE_Latest);
        
    StandaloneECEnablerP m_customAttributeEnabler = customAttributesSchema->GetClassP (L"InstanceLabelSpecification")->GetDefaultStandaloneEnabler();
    CreateSchema();
    m_schema->AddReferencedSchema (*customAttributesSchema);

    PrimitiveECPropertyP prop;
    m_ecClass->CreatePrimitiveProperty (prop, L"InstanceLabel", PRIMITIVETYPE_String);

    IECInstancePtr labelAttr = m_customAttributeEnabler->CreateInstance();
    labelAttr->SetValue (L"PropertyName", ECValue(L"InstanceLabel"));
    m_ecClass->SetCustomAttribute (*labelAttr);
        
    IECInstancePtr m_instance = m_ecClass->GetDefaultStandaloneEnabler()->CreateInstance();
    
    WString displayLabel;
    EXPECT_EQ (m_instance->GetDisplayLabel (displayLabel), ECOBJECTS_STATUS_Error);
    
    EXPECT_EQ (m_instance->SetDisplayLabel(L"Some fancy instance label"), ECOBJECTS_STATUS_Success);

    EXPECT_EQ (m_instance->GetDisplayLabel (displayLabel), ECOBJECTS_STATUS_Success);
    EXPECT_STREQ (displayLabel.c_str(), L"Some fancy instance label");
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                 Raimondas.Rimkus 02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(InstanceTests, InstanceWriteReadFile)
    {
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();

    ECSchemaPtr schema;
    SchemaReadStatus schemaStatus = ECSchema::ReadFromXmlFile (schema, ECTestFixture::GetTestDataPath(L"SimpleTest_FirstSchema.01.00.ecschema.xml").c_str(), *schemaContext);
        
    EXPECT_EQ (SCHEMA_READ_STATUS_Success, schemaStatus);
    
    ECInstanceReadContextPtr instanceContext = ECInstanceReadContext::CreateContext (*schema);

    IECInstancePtr  testInstance;
    InstanceReadStatus instanceStatus = IECInstance::ReadFromXmlFile (testInstance, ECTestFixture::GetTestDataPath(L"SimpleTest_Instance.xml").c_str(), *instanceContext);
    EXPECT_EQ (INSTANCE_READ_STATUS_Success, instanceStatus);
    
    VerifyTestInstance (testInstance.get());

    EXPECT_EQ (INSTANCE_WRITE_STATUS_Success, testInstance->WriteToXmlFile (ECTestFixture::GetTempDataPath(L"OutputInstance.xml").c_str(), true, false));
    IECInstancePtr  readbackInstance;
    InstanceReadStatus readbackStatus = IECInstance::ReadFromXmlFile (readbackInstance, ECTestFixture::GetTempDataPath(L"OutputInstance.xml").c_str(), *instanceContext);
    
    EXPECT_EQ (INSTANCE_READ_STATUS_Success, readbackStatus);
    VerifyTestInstance (readbackInstance.get());
    };
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                 Raimondas.Rimkus 02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PropertyTests, DISABLED_SetReadOnlyAndSetValue)
    {
    CreateSchema();
    CreateProperty(L"PropertyString", PRIMITIVETYPE_String);
    m_ecClass->GetPropertyP (L"PropertyString")->SetIsReadOnly(true);
    CreateInstance();
    
    ECValue getValue;
    EXPECT_TRUE (getValue.IsReadOnly());
    
    EXPECT_EQ (m_instance->SetValue (L"PropertyString", ECValue(L"Some value")), ECOBJECTS_STATUS_UnableToSetReadOnlyInstance);
    EXPECT_EQ (m_instance->GetValue (getValue, L"PropertyString"), ECOBJECTS_STATUS_Success);
    EXPECT_STREQ (getValue.GetString(), L"Some value");
    EXPECT_TRUE (m_ecClass->GetPropertyP (L"PropertyString")->GetIsReadOnly());
    };
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                 Raimondas.Rimkus 02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PropertyTests, DISABLED_SetReadOnlyAndChangeValue)
    {
    CreateSchema();
    CreateProperty(L"PropertyString", PRIMITIVETYPE_String);
    m_ecClass->GetPropertyP (L"PropertyString")->SetIsReadOnly(true);
    CreateInstance();
    
    ECValue getValue;
    EXPECT_TRUE (getValue.IsReadOnly());
   
    EXPECT_EQ (m_instance->ChangeValue (L"PropertyString", ECValue(L"Other value")), ECOBJECTS_STATUS_UnableToSetReadOnlyInstance);
    EXPECT_EQ (m_instance->GetValue (getValue, L"PropertyString"), ECOBJECTS_STATUS_Success);
    EXPECT_STREQ (getValue.GetString(), L"Other value");
    EXPECT_TRUE (m_ecClass->GetPropertyP (L"PropertyString")->GetIsReadOnly());
    };
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                 Raimondas.Rimkus 02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PropertyTests, GetValueFromInstance)
    {
    CreateSchema();
    CreateProperty(L"Property_1");
    CreateInstance();
    
    m_instance->SetValue (L"Property_1", ECValue(L"Some value"));
    ECPropertyValuePtr propValue = ECPropertyValue::GetPropertyValue (*m_instance, L"Property_1");
    
    IECInstanceCR instance = propValue->GetInstance();
    EXPECT_STREQ (m_instance->GetInstanceId().c_str(), instance.GetInstanceId().c_str());
    ECValueCR value = propValue->GetValue();
    EXPECT_STREQ (value.ToString().c_str(), L"Some value");
    ECValueAccessorCR accessor = propValue->GetValueAccessor();
    EXPECT_STREQ (accessor.GetAccessString(), L"Property_1");
    EXPECT_FALSE (propValue->HasChildValues());
    }
    
END_BENTLEY_ECOBJECT_NAMESPACE


