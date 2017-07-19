/*--------------------------------------------------------------------------------------+
|
|     $Source: test/Published/InstanceTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"

USING_NAMESPACE_BENTLEY_EC

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
    ECEntityClassP             m_ecClass;
    IECInstancePtr       m_instance;
    uint32_t             propIndex;

    void CreateSchema (Utf8String schemaName = "TestSchema", Utf8String className = "TestClass", Utf8String alias = "TestAlias")
        {
        ECSchema::CreateSchema (m_schema, schemaName, alias, 1, 0, 0);
        m_schema->CreateEntityClass (m_ecClass, className);
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

    PrimitiveArrayECPropertyP CreateArrayProperty (Utf8CP name)
        {
        PrimitiveArrayECPropertyP prop;
        m_ecClass->CreatePrimitiveArrayProperty (prop, name);
        m_ecClass->GetDefaultStandaloneEnabler ()->GetPropertyIndex (propIndex, name);
        return prop;
        }
    };

struct PropertyTests : InstanceTests {};

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                               Raimondas.Rimkus   02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (InstanceTests, TestIsNullPrimitive)
    {
    CreateSchema ();
    CreateProperty ("PropertyString", PRIMITIVETYPE_String);
    CreateInstance ();

    bool isNull = NULL;

    EXPECT_EQ (m_instance->IsPropertyNull (isNull, "PropertyString"), ECObjectsStatus::Success);
    EXPECT_TRUE (isNull);
    EXPECT_EQ (m_instance->IsPropertyNull (isNull, propIndex), ECObjectsStatus::Success);
    EXPECT_TRUE (isNull);

    EXPECT_EQ (m_instance->SetValue ("PropertyString", ECValue ("Some value")), ECObjectsStatus::Success);

    EXPECT_EQ (m_instance->IsPropertyNull (isNull, "PropertyString"), ECObjectsStatus::Success);
    EXPECT_FALSE (isNull);
    EXPECT_EQ (m_instance->IsPropertyNull (isNull, propIndex), ECObjectsStatus::Success);
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

    EXPECT_EQ (m_instance->IsPropertyNull (isNull, "PropertyArray"), ECObjectsStatus::Success);
    EXPECT_FALSE (isNull);
    EXPECT_EQ (m_instance->IsPropertyNull (isNull, propIndex), ECObjectsStatus::Success);
    EXPECT_FALSE (isNull);

    EXPECT_EQ (m_instance->AddArrayElements ("PropertyArray", 15), ECObjectsStatus::Success);
    EXPECT_EQ (m_instance->SetValue ("PropertyArray", ECValue ("Some value"), 3), ECObjectsStatus::Success);

    //Strangelly array property doesn't exist as PrimitiveProperty
    EXPECT_EQ (m_instance->IsPropertyNull (isNull, "PropertyArray"), ECObjectsStatus::Success);
    EXPECT_FALSE (isNull);
    EXPECT_EQ (m_instance->IsPropertyNull (isNull, propIndex), ECObjectsStatus::Success);
    EXPECT_FALSE (isNull);
    //-------------------------------------------------------------

    EXPECT_EQ (m_instance->IsPropertyNull (isNull, "PropertyArray", 3), ECObjectsStatus::Success);
    EXPECT_FALSE (isNull);
    EXPECT_EQ (m_instance->IsPropertyNull (isNull, propIndex, 3), ECObjectsStatus::Success);
    EXPECT_FALSE (isNull);

    EXPECT_EQ (m_instance->IsPropertyNull (isNull, "PropertyArray", 13), ECObjectsStatus::Success);
    EXPECT_TRUE (isNull);
    EXPECT_EQ (m_instance->IsPropertyNull (isNull, propIndex, 13), ECObjectsStatus::Success);
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

    EXPECT_EQ (m_instance->GetValue (arrayVal, "PropertyArray"), ECObjectsStatus::Success);
    EXPECT_TRUE (arrayVal.IsArray ());
    EXPECT_EQ (arrayVal.GetArrayInfo ().GetCount (), 0);

    EXPECT_EQ (m_instance->AddArrayElements ("PropertyArray", 13), ECObjectsStatus::Success);

    EXPECT_EQ (m_instance->GetValue (arrayVal, "PropertyArray"), ECObjectsStatus::Success);
    EXPECT_TRUE (arrayVal.IsArray ());
    EXPECT_EQ (arrayVal.GetArrayInfo ().GetCount (), 13);

    EXPECT_EQ (m_instance->ClearArray ("PropertyArray"), ECObjectsStatus::Success);

    EXPECT_EQ (m_instance->GetValue (arrayVal, "PropertyArray"), ECObjectsStatus::Success);
    EXPECT_TRUE (arrayVal.IsArray ());
    EXPECT_EQ (arrayVal.GetArrayInfo ().GetCount (), 0);

    EXPECT_EQ (m_instance->AddArrayElements (propIndex, 42), ECObjectsStatus::Success);

    EXPECT_EQ (m_instance->GetValue (arrayVal, propIndex), ECObjectsStatus::Success);
    EXPECT_TRUE (arrayVal.IsArray ());
    EXPECT_EQ (arrayVal.GetArrayInfo ().GetCount (), 42);

    EXPECT_EQ (m_instance->ClearArray (propIndex), ECObjectsStatus::Success);

    EXPECT_EQ (m_instance->GetValue (arrayVal, propIndex), ECObjectsStatus::Success);
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

    EXPECT_EQ (m_instance->SetValue ("Property_1", ECValue ("Some value 1")), ECObjectsStatus::Success);
    EXPECT_EQ (m_instance->GetValue (value, "Property_1"), ECObjectsStatus::Success);
    EXPECT_STREQ (value.GetUtf8CP (), "Some value 1");

    EXPECT_EQ (m_instance->SetValue (propIndex, ECValue ("Some value 2")), ECObjectsStatus::Success);
    EXPECT_EQ (m_instance->GetValue (value, propIndex), ECObjectsStatus::Success);
    EXPECT_STREQ (value.GetUtf8CP (), "Some value 2");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                 Raimondas.Rimkus 02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (InstanceTests, TestSetDisplayLabel)
    {
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext ();
    SchemaKey schemaKey ("Bentley_Standard_CustomAttributes", 1, 5);
    ECSchemaPtr customAttributesSchema = context->LocateSchema (schemaKey, SchemaMatchType::Latest);

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
    EXPECT_EQ (m_instance->GetDisplayLabel (displayLabel), ECObjectsStatus::Success);
    EXPECT_TRUE (displayLabel.Equals (m_ecClass->GetDisplayLabel ()));

    EXPECT_EQ (m_instance->SetDisplayLabel ("Some fancy instance label"), ECObjectsStatus::Success);

    EXPECT_EQ (m_instance->GetDisplayLabel (displayLabel), ECObjectsStatus::Success);
    EXPECT_STREQ (displayLabel.c_str (), "Some fancy instance label");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad.Hassan                     07/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(InstanceTests, GetInstanceAttributesUsingInstanceInterface)
    {
    CreateSchema();
    CreateProperty("StringProperty", PRIMITIVETYPE_String);
    CreateInstance();

    ASSERT_EQ(ECObjectsStatus::Success, m_instance->SetValue("StringProperty", ECValue("Some value")));

    ECN::ECInstanceInterface instanceInterface(*m_instance);

    //get value
    ECValue stringVal;
    ASSERT_EQ(ECObjectsStatus::Success, instanceInterface.GetInstanceValue(stringVal, "StringProperty"));
    ASSERT_STREQ("Some value", stringVal.GetUtf8CP());

    //get instance class name
    ECClassCP instanceClass = instanceInterface.GetInstanceClass();
    ASSERT_STREQ("TestSchema:TestClass", instanceClass->GetFullName());

    //get instanceId
    Utf8String instanceId = instanceInterface.GetInstanceId();
    ASSERT_STREQ(instanceId.c_str(), m_instance->GetInstanceId().c_str());

    //Get IECInstance
    IECInstanceCP instance = instanceInterface.ObtainECInstance();
    ASSERT_TRUE(instance != nullptr);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad.Hassan                     07/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(InstanceTests, GetInstanceOffSet)
    {
    CreateSchema();
    CreateProperty("StringProperty", PRIMITIVETYPE_String);
    CreateInstance();

    ASSERT_EQ(ECObjectsStatus::Success, m_instance->SetValue("StringProperty", ECValue("Some value")));

    size_t offSet = m_instance->GetOffsetToIECInstance();
    ASSERT_TRUE(offSet == 0);
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

    //since the instance has no value initially, it can be set.This was done so that instances could be deserialized even if they had readonly property.This is the same as in the managed API.
    EXPECT_EQ (ECObjectsStatus::Success, m_instance->SetValue ("PropertyString", ECValue ("Some value")));
    ECValue getValue;
    EXPECT_EQ (m_instance->GetValue (getValue, "PropertyString"), ECObjectsStatus::Success);
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

    //since the instance has no value initially, it can be set.This was done so that instances could be deserialized even if they had readonly property.This is the same as in the managed API.
    EXPECT_EQ (ECObjectsStatus::Success, m_instance->ChangeValue ("PropertyString", ECValue ("Other value")));
    ECValue getValue;
    EXPECT_EQ (m_instance->GetValue (getValue, "PropertyString"), ECObjectsStatus::Success);
    EXPECT_STREQ (getValue.GetUtf8CP (), "Other value");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                               Muhammad Hassan    02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (PropertyTests, SetAndChangePropertyValue)
    {
    CreateSchema ();
    CreateProperty ("PropertyString", PRIMITIVETYPE_String);
    CreateInstance ();

    EXPECT_EQ (ECObjectsStatus::Success, m_instance->SetValue ("PropertyString", ECValue ("init value")));
    ECValue getValue;
    EXPECT_EQ (m_instance->GetValue (getValue, "PropertyString"), ECObjectsStatus::Success);
    EXPECT_STREQ (getValue.GetUtf8CP (), "init value");

    EXPECT_EQ (ECObjectsStatus::Success, m_instance->ChangeValue ("PropertyString", ECValue ("Other value")));
    
    EXPECT_EQ (m_instance->GetValue (getValue, "PropertyString"), ECObjectsStatus::Success);
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


