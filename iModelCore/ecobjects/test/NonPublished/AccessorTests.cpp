/*--------------------------------------------------------------------------------------+
|
|     $Source: test/NonPublished/AccessorTests.cpp $
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
        
    PrimitiveArrayECPropertyP CreatePrimitiveArrayProperty(Utf8CP name)
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
TEST_F (ValueAccessorTests, CreateFromInstanceIndexConstructAndClone)
    {
    CreateSchema();
    CreateProperty("Property_1");
    CreateInstance();
    
    ECValueAccessor m_accessor1 = ECValueAccessor(*m_instance, propIndex);
    EXPECT_EQ (m_instance->SetValueUsingAccessor (m_accessor1, ECValue("Some value 1")), ECObjectsStatus::Success);
    
    ECValue value1;
    m_instance->GetValueUsingAccessor (value1, m_accessor1);
    EXPECT_STREQ (value1.GetUtf8CP(), "Some value 1");
    
    ECValueAccessor m_accessor2;
    m_accessor2.Clone(m_accessor1);
    
    ECValue value2;
    m_instance->GetValueUsingAccessor (value2, m_accessor2);
    EXPECT_STREQ (value2.GetUtf8CP(), "Some value 1");
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                 Raimondas.Rimkus 02/2013
+---------------+---------------+---------------+---------------+---------------+------*/    
TEST_F (ValueAccessorTests, CreateFromEnabledIndexConstruct)
    {
    CreateSchema();
    CreateProperty("Property_1");
    CreateInstance();
    
    ECValueAccessor m_accessor = ECValueAccessor(*m_ecClass->GetDefaultStandaloneEnabler(), propIndex);
    EXPECT_EQ (m_instance->SetValueUsingAccessor (m_accessor, ECValue("Some value 1")), ECObjectsStatus::Success);
    
    ECValue value1;
    m_instance->GetValueUsingAccessor (value1, m_accessor);
    EXPECT_STREQ (value1.GetUtf8CP(), "Some value 1");
    }
    
END_BENTLEY_ECN_TEST_NAMESPACE