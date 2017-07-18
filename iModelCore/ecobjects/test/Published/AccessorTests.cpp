/*--------------------------------------------------------------------------------------+
|
|     $Source: test/Published/AccessorTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                               Raimondas.Rimkus   02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct ValueAccessorTests : ECTestFixture
    {
    ECSchemaPtr          m_schema;
    ECEntityClassP             m_ecClass;
    IECInstancePtr       m_instance;
    uint32_t             propIndex;

    void CreateSchema(Utf8String schemaName = "TestSchema", Utf8String className = "TestClass", Utf8String alias = "TestAlias")
        {
        ECSchema::CreateSchema (m_schema, schemaName, alias, 1, 0, 0);
        m_schema->CreateEntityClass (m_ecClass, className);
        }
    
    void CreateInstance()
        {
        m_instance = m_ecClass->GetDefaultStandaloneEnabler()->CreateInstance();
        }
        
    PrimitiveECPropertyP CreateProperty(Utf8String name, PrimitiveType primitiveType)
        {
        PrimitiveECPropertyP prop;
        m_ecClass->CreatePrimitiveProperty (prop, name, primitiveType);
        m_ecClass->GetDefaultStandaloneEnabler()->GetPropertyIndex(propIndex, name.c_str());
        return prop;
        }
        
    PrimitiveECPropertyP CreateProperty(Utf8String name)
        {
        PrimitiveECPropertyP prop;
        m_ecClass->CreatePrimitiveProperty (prop, name);
        m_ecClass->GetDefaultStandaloneEnabler()->GetPropertyIndex(propIndex, name.c_str());
        return prop;
        }
        
    PrimitiveArrayECPropertyP CreateArrayProperty(Utf8CP name)
        {
        PrimitiveArrayECPropertyP prop;
        m_ecClass->CreatePrimitiveArrayProperty (prop, name);
        m_ecClass->GetDefaultStandaloneEnabler()->GetPropertyIndex(propIndex, name);
        return prop;
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                 Raimondas.Rimkus 02/2013
+---------------+---------------+---------------+---------------+---------------+------*/    
TEST_F (ValueAccessorTests, CreateFromIterator)
    {
    CreateSchema();
    CreateProperty("Property_1");
    CreateInstance();
    
    EXPECT_EQ (m_instance->SetValue ("Property_1", ECValue("Some value 1")), ECObjectsStatus::Success);    
    ECValuesCollectionPtr m_collection = ECValuesCollection::Create (*m_instance);
    
    int count = 0;
    for (ECPropertyValueCR propertyValue : *m_collection)
        {
        ECValue value1;

        ECValueAccessorCR m_accessor = propertyValue.GetValueAccessor ();
        m_instance->GetValueUsingAccessor (value1, m_accessor);

        EXPECT_STREQ (value1.GetUtf8CP (), "Some value 1");
        count++;
        }
    EXPECT_EQ (count, 1);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                 Raimondas.Rimkus 02/2013
+---------------+---------------+---------------+---------------+---------------+------*/    
TEST_F (ValueAccessorTests, CreateFromInstance)
    {
    CreateSchema();
    CreateProperty("Property_1");
    CreateInstance();
    
    ECValueAccessor m_accessor;
    EXPECT_EQ (ECValueAccessor::PopulateValueAccessor(m_accessor, *m_instance, "Property_1"), ECObjectsStatus::Success);
    EXPECT_EQ (m_instance->SetValueUsingAccessor (m_accessor, ECValue("Some value 1")), ECObjectsStatus::Success);
    
    ECValue value1;
    m_instance->GetValueUsingAccessor (value1, m_accessor);
    EXPECT_STREQ (value1.GetUtf8CP(), "Some value 1");
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                 Raimondas.Rimkus 02/2013
+---------------+---------------+---------------+---------------+---------------+------*/    
TEST_F (ValueAccessorTests, GetAccessString)
    {
    CreateSchema();
    CreateProperty("Property_1");
    CreateInstance();
    
    ECValueAccessor m_accessor;
    EXPECT_EQ (ECValueAccessor::PopulateValueAccessor(m_accessor, *m_instance, "Property_1"), ECObjectsStatus::Success);
    EXPECT_STREQ (m_accessor.GetAccessString(), "Property_1");
    EXPECT_STREQ (m_accessor.GetPropertyName().c_str(), "Property_1");
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                 Raimondas.Rimkus 02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
//Bill Steinbock:
//This Vancouver change to use a ClassLayout/PropertyLayout scheme doesn�t lend itself to
//anyone just modifying a class on the fly as might have been easily done in the managed
//heavyweight ECInstance.
TEST_F (ValueAccessorTests, GetDefaultStandaloneEnabler)
    {
    CreateSchema();
    
    PrimitiveECPropertyP prop;
    EXPECT_EQ (m_ecClass->CreatePrimitiveProperty (prop, "Prop1"), ECObjectsStatus::Success);
    EXPECT_EQ (m_ecClass->GetDefaultStandaloneEnabler()->GetClassLayout().GetPropertyCount(), 2);
    
    EXPECT_EQ (m_ecClass->CreatePrimitiveProperty (prop, "Prop2"), ECObjectsStatus::Success);
    EXPECT_EQ (m_ecClass->GetDefaultStandaloneEnabler()->GetClassLayout().GetPropertyCount(), 3);
    
    EXPECT_EQ (m_ecClass->GetDefaultStandaloneEnabler()->GetPropertyIndex(propIndex, "Prop2"), ECObjectsStatus::Success);
    Utf8CP propertyName;
    EXPECT_EQ (m_ecClass->GetDefaultStandaloneEnabler()->GetAccessString  (propertyName,  propIndex), ECObjectsStatus::Success);
    EXPECT_STREQ (propertyName, "Prop2");
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                 Raimondas.Rimkus 02/2013
+---------------+---------------+---------------+---------------+---------------+------*/    
TEST_F (ValueAccessorTests, PushPopLocation)
    {
    CreateSchema();
    Utf8Char const* properties[4] = {"Property_1", "Property_2", "Property_3", "Property_4"};
    
    for (int i=0; i<4; i++)
        {
        PrimitiveECPropertyP prop;
        m_ecClass->CreatePrimitiveProperty (prop, properties[i]);
        }
    CreateInstance();
    
    ECValueAccessor accessor = ECValueAccessor();
    for (int i=0; i<4; i++)
        accessor.PushLocation(*m_instance, properties[i]);
    
    EXPECT_EQ (accessor.GetDepth(), 4);
    EXPECT_STREQ (accessor.GetAccessString(), "Property_4");
    EXPECT_STREQ (accessor.GetPropertyName().c_str(), "Property_4");
    for (int i=0; i<4; i++)
        {
        EXPECT_STREQ (accessor.GetAccessString(i), properties[i]);
        }
        
    accessor.PopLocation();
    EXPECT_EQ (accessor.GetDepth(), 3);
    EXPECT_STREQ (accessor.GetAccessString(), "Property_3");
    EXPECT_STREQ (accessor.GetPropertyName().c_str(), "Property_3");
    for (int i=0; i<3; i++)
        {
        EXPECT_STREQ (accessor.GetAccessString(i), properties[i]);
        }
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                 Raimondas.Rimkus 02/2013
+---------------+---------------+---------------+---------------+---------------+------*/    
TEST_F (ValueAccessorTests, MatchAccessors)
    {
    CreateSchema();
    Utf8Char const* properties[4] = {"Property_1", "Property_2", "Property_3", "Property_4"};
    IECInstancePtr instance_1;
    IECInstancePtr instance_2;
    
    for (int i=0; i<4; i++)
        {
        PrimitiveECPropertyP prop;
        m_ecClass->CreatePrimitiveProperty (prop, properties[i]);
        }
    instance_1 = m_ecClass->GetDefaultStandaloneEnabler()->CreateInstance();
    instance_2 = m_ecClass->GetDefaultStandaloneEnabler()->CreateInstance();
    
    ECValueAccessor accessor_1 = ECValueAccessor();
    ECValueAccessor accessor_2 = ECValueAccessor();
    ECValueAccessor accessor_3 = ECValueAccessor();
    for (int i=0; i<4; i++)
        {
        accessor_1.PushLocation(*instance_1, properties[i]);
        accessor_2.PushLocation(*instance_2, properties[i]);
        accessor_3.PushLocation(*instance_1, properties[i]);
        }
    accessor_3.PopLocation();
    
    EXPECT_TRUE (accessor_1 == accessor_2);
    EXPECT_FALSE (accessor_1 == accessor_3);
    EXPECT_FALSE (accessor_1 != accessor_2);
    EXPECT_TRUE (accessor_1 != accessor_3);
    }
    
END_BENTLEY_ECN_TEST_NAMESPACE
