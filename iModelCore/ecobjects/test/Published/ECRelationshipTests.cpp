/*--------------------------------------------------------------------------------------+
|
|     $Source: test/Published/ECRelationshipTests.cpp $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsTestPCH.h"
#include "TestFixture.h"

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

struct ECRelationshipTests : ECTestFixture {};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    12/09
+---------------+---------------+---------------+---------------+---------------+------*/
static WString    GetTestSchemaXMLString ()
    {
    wchar_t fmt[] = L"<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                    L"<ECSchema schemaName=\"RelationshipTesting\" nameSpacePrefix=\"test\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
                    L"    <ECClass typeName=\"ClassA\" displayLabel=\"Class A\" isDomainClass=\"True\">"
                    L"        <ECProperty propertyName=\"p\" typeName=\"int\" />"
                    L"    </ECClass>"
                    L"    <ECClass typeName=\"ClassB\" displayLabel=\"Class B\" isDomainClass=\"True\">"
                    L"        <ECProperty propertyName=\"p\" typeName=\"int\" />"
                    L"        <ECProperty propertyName=\"b\" typeName=\"int\" />"
                    L"    </ECClass>"
                    L"    <ECRelationshipClass typeName=\"ALikesB\" displayLabel=\"A likes B\" strength=\"referencing\">"
                    L"        <Source cardinality=\"(1,1)\" roleLabel=\"likes\" polymorphic=\"False\">"
                    L"            <Class class=\"ClassA\" />"
                    L"        </Source>"
                    L"        <Target cardinality=\"(1,1)\" roleLabel=\"is liked by\" polymorphic=\"True\">"
                    L"            <Class class=\"ClassB\" />"
                    L"        </Target>"
                    L"        <ECProperty propertyName=\"p\" typeName=\"int\" />"
                    L"        <ECProperty propertyName=\"Name\" typeName=\"string\" />"
                    L"        <ECProperty propertyName=\"SourceOrderId\" typeName=\"int\" />"
                    L"        <ECProperty propertyName=\"TargetOrderId\" typeName=\"int\" />"
                    L"    </ECRelationshipClass>"
                    L"</ECSchema>";

    return fmt;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
static ECSchemaPtr CreateTestSchema ()
    {
    WString schemaXMLString = GetTestSchemaXMLString ();

    ECSchemaReadContextPtr  schemaContext = ECSchemaReadContext::CreateContext();

    ECSchemaPtr schema;        
    EXPECT_EQ (SUCCESS, ECSchema::ReadFromXmlString (schema, schemaXMLString.c_str(), *schemaContext));  

    return schema;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECRelationshipTests, SimpleRelationship)
    {
    ECSchemaPtr schema = CreateTestSchema();
    ASSERT_TRUE (schema.IsValid());
    
    ECClassP sourceClass = schema->GetClassP (L"ClassA");
    ASSERT_TRUE (NULL != sourceClass);
    StandaloneECEnablerPtr sourceEnabler =  sourceClass->GetDefaultStandaloneEnabler();
    ASSERT_TRUE (sourceEnabler.IsValid());

    ECClassP targetClass = schema->GetClassP (L"ClassB");
    ASSERT_TRUE (NULL != targetClass);
    StandaloneECEnablerPtr targetEnabler =  targetClass->GetDefaultStandaloneEnabler();
    ASSERT_TRUE (targetEnabler.IsValid());

    ECValue p1;
    p1.SetInteger (123);

    ECN::StandaloneECInstancePtr sourceInstance = sourceEnabler->CreateInstance();
    sourceInstance->SetValue(L"p", p1);
    sourceInstance->SetInstanceId(L"source");

    ECValue p2, b2;
    p2.SetInteger (456);
    b2.SetInteger (789);
    ECN::StandaloneECInstancePtr targetInstance = targetEnabler->CreateInstance();
    targetInstance->SetValue(L"p", p2);
    targetInstance->SetValue(L"b", b2);
    targetInstance->SetInstanceId(L"target");

    ECRelationshipClassCP relClass = schema->GetClassP (L"ALikesB")->GetRelationshipClassCP();
    ASSERT_TRUE (NULL != relClass);

    StandaloneECRelationshipEnablerPtr relationshipEnabler = StandaloneECRelationshipEnabler::CreateStandaloneRelationshipEnabler (*relClass);
    ASSERT_TRUE (relationshipEnabler.IsValid());

    StandaloneECRelationshipInstancePtr relationshipInstance = relationshipEnabler->CreateRelationshipInstance ();
    relationshipInstance->SetSource (sourceInstance.get());
    relationshipInstance->SetTarget (targetInstance.get());
    relationshipInstance->SetInstanceId (L"source->target");

    IECInstancePtr readSource = relationshipInstance->GetSource();
    ECValue readValue;
    readSource->GetValue (readValue, L"p");
    ASSERT_TRUE (readValue.Equals(p1));

    IECInstancePtr readTarget = relationshipInstance->GetTarget();
    readTarget->GetValue (readValue, L"p");
    ASSERT_TRUE (readValue.Equals(p2));
    readTarget->GetValue (readValue, L"b");
    ASSERT_TRUE (readValue.Equals(b2));
    }

 /*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECRelationshipTests, SetRelationshipProperties)
    {
    ECSchemaPtr schema = CreateTestSchema();
    ASSERT_TRUE (schema.IsValid());

    ECClassP sourceClass = schema->GetClassP (L"ClassA");
    ASSERT_TRUE (NULL != sourceClass);
    StandaloneECEnablerPtr sourceEnabler =  sourceClass->GetDefaultStandaloneEnabler();
    ASSERT_TRUE (sourceEnabler.IsValid());

    ECClassP targetClass = schema->GetClassP (L"ClassB");
    ASSERT_TRUE (NULL != targetClass);
    StandaloneECEnablerPtr targetEnabler =  targetClass->GetDefaultStandaloneEnabler();
    ASSERT_TRUE (targetEnabler.IsValid());

    ECValue p1;
    p1.SetInteger (123);

    ECN::StandaloneECInstancePtr sourceInstance = sourceEnabler->CreateInstance();
    sourceInstance->SetValue(L"p", p1);
    sourceInstance->SetInstanceId(L"source");

    ECValue p2, b2;
    p2.SetInteger (456);
    b2.SetInteger (789);
    ECN::StandaloneECInstancePtr targetInstance = targetEnabler->CreateInstance();
    targetInstance->SetValue(L"p", p2);
    targetInstance->SetValue(L"b", b2);
    targetInstance->SetInstanceId(L"target");

    ECRelationshipClassP relClass = dynamic_cast<ECRelationshipClassP>(schema->GetClassP (L"ALikesB"));
    ASSERT_TRUE (NULL != relClass);

    StandaloneECRelationshipEnablerPtr relationshipEnabler = StandaloneECRelationshipEnabler::CreateStandaloneRelationshipEnabler (*relClass);
    ASSERT_TRUE (relationshipEnabler.IsValid());

    StandaloneECRelationshipInstancePtr relationshipInstance = relationshipEnabler->CreateRelationshipInstance ();
    relationshipInstance->SetSource (sourceInstance.get());
    relationshipInstance->SetTarget (targetInstance.get());
    relationshipInstance->SetInstanceId (L"source->target");

    ECValue rel_p, rel_name, rel_sourceOrder, rel_targetOrder;
    rel_p.SetInteger (777);
    rel_sourceOrder.SetInteger (888);
    rel_targetOrder.SetInteger (999);
    rel_name.SetString (L"my_relationship");

    EXPECT_TRUE (SUCCESS == relationshipInstance->SetValue (L"p", rel_p));
    EXPECT_TRUE (SUCCESS == relationshipInstance->SetValue (L"Name", rel_name));
    EXPECT_TRUE (SUCCESS == relationshipInstance->SetValue (L"SourceOrderId", rel_sourceOrder));
    EXPECT_TRUE (SUCCESS == relationshipInstance->SetValue (L"TargetOrderId", rel_targetOrder));

    IECInstancePtr readSource = relationshipInstance->GetSource();
    ECValue readValue;
    readSource->GetValue (readValue, L"p");
    ASSERT_TRUE (readValue.Equals(p1));

    IECInstancePtr readTarget = relationshipInstance->GetTarget();
    readTarget->GetValue (readValue, L"p");
    ASSERT_TRUE (readValue.Equals(p2));
    readTarget->GetValue (readValue, L"b");
    ASSERT_TRUE (readValue.Equals(b2));

    WString ecInstanceXml;

    InstanceWriteStatus status2 = relationshipInstance->WriteToXmlString(ecInstanceXml, true, true);
    EXPECT_EQ(INSTANCE_WRITE_STATUS_Success, status2);
    }

END_BENTLEY_ECOBJECT_NAMESPACE