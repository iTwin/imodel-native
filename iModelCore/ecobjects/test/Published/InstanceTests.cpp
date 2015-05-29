/*--------------------------------------------------------------------------------------+
|
|     $Source: test/Published/InstanceTests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"

using namespace BentleyApi::ECN;

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   01/13
+---------------+---------------+---------------+---------------+---------------+------*/
struct InstanceLabelTest      : ECTestFixture
    {
    StandaloneECEnablerP    m_customAttributeEnabler;
    ECSchemaPtr             m_schema;

    void                CreateSchema()
        {
        ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
        SchemaKey schemaKey (L"Bentley_Standard_CustomAttributes", 1, 5);
        ECSchemaPtr customAttributesSchema = context->LocateSchema (schemaKey, SCHEMAMATCHTYPE_Latest);
        
        m_customAttributeEnabler = customAttributesSchema->GetClassP (L"InstanceLabelSpecification")->GetDefaultStandaloneEnabler();

        ECSchema::CreateSchema (m_schema, L"TestSchema", 1, 0);
        m_schema->AddReferencedSchema (*customAttributesSchema);
        }

    ECClassCP           CreateClass (WCharCP className, bool hasInstanceLabelAttribute, WCharCP instanceLabelPropertyName)
        {
        ECClassP ecClass;
        m_schema->CreateClass (ecClass, className);

        PrimitiveECPropertyP prop;
        ecClass->CreatePrimitiveProperty (prop, instanceLabelPropertyName, PRIMITIVETYPE_String);

        if (hasInstanceLabelAttribute)
            {
            IECInstancePtr labelAttr = m_customAttributeEnabler->CreateInstance();
            ECValue v;
            if (instanceLabelPropertyName)
                v.SetString (instanceLabelPropertyName, false);

            labelAttr->SetValue (L"PropertyName", v);
            ecClass->SetCustomAttribute (*labelAttr);
            }

        return ecClass;
        }

    void                TestInstanceLabel (bool hasInstanceLabelAttribute, WCharCP instanceLabelPropertyName, WCharCP instanceLabelPropertyValue)
        {
        static WChar s_className[2] = L"A";

        ECClassCP ecClass = CreateClass (s_className, hasInstanceLabelAttribute, instanceLabelPropertyName);
        IECInstancePtr instance = ecClass->GetDefaultStandaloneEnabler()->CreateInstance();
        if (NULL != instanceLabelPropertyValue)
            instance->SetValue (instanceLabelPropertyName, ECValue (instanceLabelPropertyValue, false));

        WString displayLabel;
        EXPECT_EQ (ECOBJECTS_STATUS_Success, instance->GetDisplayLabel (displayLabel));
        if (instanceLabelPropertyValue)
            EXPECT_TRUE (displayLabel.Equals (instanceLabelPropertyValue));
        else
            EXPECT_TRUE (displayLabel.Equals (ecClass->GetDisplayLabel()));

        ++s_className[0];
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (InstanceLabelTest, TestLabels)
    {
    CreateSchema();

    TestInstanceLabel (true, L"InstanceLabelProperty", L"MyLabel");
    TestInstanceLabel (false, L"DisplayLabel", NULL);   // Bill added DisplayLabel and variants as hard-coded property names to use for label if no specification present; managed does not do this; reverted it.
    TestInstanceLabel (false, L"NAME", L"MyName");
    TestInstanceLabel (true, L"NAME", L"MyName");
    TestInstanceLabel (false, L"ThisIsNotAnInstanceLabel", NULL);
    }

END_BENTLEY_ECN_TEST_NAMESPACE


