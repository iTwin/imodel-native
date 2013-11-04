/*--------------------------------------------------------------------------------------+
|
|     $Source: tests/NonPublishedScenario/AccessorTests.cpp $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"

using namespace Bentley::ECN;

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                               Raimondas.Rimkus   02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct ValueAccessorTests : ECTestFixture
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
        
    PrimitiveECPropertyP CreateProperty(WString name, PrimitiveType primitiveType)
        {
        PrimitiveECPropertyP prop;
        m_ecClass->CreatePrimitiveProperty (prop, name, primitiveType);
        m_ecClass->GetDefaultStandaloneEnabler()->GetPropertyIndex(propIndex, name.c_str());
        return prop;
        }
        
    PrimitiveECPropertyP CreateProperty(WString name)
        {
        PrimitiveECPropertyP prop;
        m_ecClass->CreatePrimitiveProperty (prop, name);
        m_ecClass->GetDefaultStandaloneEnabler()->GetPropertyIndex(propIndex, name.c_str());
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
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                 Raimondas.Rimkus 02/2013
+---------------+---------------+---------------+---------------+---------------+------*/    
TEST_F (ValueAccessorTests, CreateFromInstanceIndexConstructAndClone)
    {
    CreateSchema();
    CreateProperty(L"Property_1");
    CreateInstance();
    
    ECValueAccessor m_accessor1 = ECValueAccessor(*m_instance, propIndex);
    EXPECT_EQ (m_instance->SetValueUsingAccessor (m_accessor1, ECValue(L"Some value 1")), ECOBJECTS_STATUS_Success);
    
    ECValue value1;
    m_instance->GetValueUsingAccessor (value1, m_accessor1);
    EXPECT_STREQ (value1.GetString(), L"Some value 1");
    
    ECValueAccessor m_accessor2;
    m_accessor2.Clone(m_accessor1);
    
    ECValue value2;
    m_instance->GetValueUsingAccessor (value2, m_accessor2);
    EXPECT_STREQ (value2.GetString(), L"Some value 1");
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                 Raimondas.Rimkus 02/2013
+---------------+---------------+---------------+---------------+---------------+------*/    
TEST_F (ValueAccessorTests, CreateFromEnabledIndexConstruct)
    {
    CreateSchema();
    CreateProperty(L"Property_1");
    CreateInstance();
    
    ECValueAccessor m_accessor = ECValueAccessor(*m_ecClass->GetDefaultStandaloneEnabler(), propIndex);
    EXPECT_EQ (m_instance->SetValueUsingAccessor (m_accessor, ECValue(L"Some value 1")), ECOBJECTS_STATUS_Success);
    
    ECValue value1;
    m_instance->GetValueUsingAccessor (value1, m_accessor);
    EXPECT_STREQ (value1.GetString(), L"Some value 1");
    }
    
END_BENTLEY_ECN_TEST_NAMESPACE