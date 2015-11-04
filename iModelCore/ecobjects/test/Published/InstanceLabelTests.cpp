/*--------------------------------------------------------------------------------------+
|
|     $Source: test/Published/InstanceLabelTests.cpp $
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
        SchemaKey schemaKey ("Bentley_Standard_CustomAttributes", 1, 5);
        ECSchemaPtr customAttributesSchema = context->LocateSchema (schemaKey, SCHEMAMATCHTYPE_Latest);
        
        m_customAttributeEnabler = customAttributesSchema->GetClassP ("InstanceLabelSpecification")->GetDefaultStandaloneEnabler();

        ECSchema::CreateSchema (m_schema, "TestSchema", 1, 0);
        m_schema->AddReferencedSchema (*customAttributesSchema);
        }

    ECClassCP           CreateClass (Utf8CP className, bool hasInstanceLabelAttribute, Utf8CP instanceLabelPropertyName)
        {
        ECEntityClassP ecClass;
        m_schema->CreateEntityClass (ecClass, className);

        PrimitiveECPropertyP prop;
        ecClass->CreatePrimitiveProperty (prop, instanceLabelPropertyName, PRIMITIVETYPE_String);

        if (hasInstanceLabelAttribute)
            {
            IECInstancePtr labelAttr = m_customAttributeEnabler->CreateInstance();
            ECValue v;
            if (instanceLabelPropertyName)
                v.SetUtf8CP (instanceLabelPropertyName, false);

            labelAttr->SetValue ("PropertyName", v);
            ecClass->SetCustomAttribute (*labelAttr);
            }

        return ecClass;
        }

    void                TestInstanceLabel (bool hasInstanceLabelAttribute, Utf8CP instanceLabelPropertyName, Utf8CP instanceLabelPropertyValue)
        {
        static Utf8Char s_className[2] = "A";

        ECClassCP ecClass = CreateClass (s_className, hasInstanceLabelAttribute, instanceLabelPropertyName);
        IECInstancePtr instance = ecClass->GetDefaultStandaloneEnabler()->CreateInstance();
        if (NULL != instanceLabelPropertyValue)
            instance->SetValue (instanceLabelPropertyName, ECValue (instanceLabelPropertyValue, false));

        Utf8String displayLabel;
        EXPECT_EQ (ECObjectsStatus::Success, instance->GetDisplayLabel (displayLabel));
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

    TestInstanceLabel (true, "InstanceLabelProperty", "MyLabel");
    TestInstanceLabel (false, "DisplayLabel", NULL);   // Bill added DisplayLabel and variants as hard-coded property names to use for label if no specification present; managed does not do this; reverted it.
    TestInstanceLabel (false, "NAME", "MyName");
    TestInstanceLabel (true, "NAME", "MyName");
    TestInstanceLabel (false, "ThisIsNotAnInstanceLabel", NULL);
    }

END_BENTLEY_ECN_TEST_NAMESPACE


