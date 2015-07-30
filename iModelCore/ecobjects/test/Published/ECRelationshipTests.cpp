/*--------------------------------------------------------------------------------------+
|
|     $Source: test/Published/ECRelationshipTests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"


BEGIN_BENTLEY_ECN_TEST_NAMESPACE
using namespace BentleyApi::ECN;

struct ECRelationshipTests : ECTestFixture {};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    12/09
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String    GetTestSchemaXMLString ()
    {
    Utf8Char fmt[] = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                    "<ECSchema schemaName=\"RelationshipTesting\" nameSpacePrefix=\"test\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
                    "    <ECClass typeName=\"ClassA\" displayLabel=\"Class A\" isDomainClass=\"True\">"
                    "        <ECProperty propertyName=\"p\" typeName=\"int\" />"
                    "    </ECClass>"
                    "    <ECClass typeName=\"ClassB\" displayLabel=\"Class B\" isDomainClass=\"True\">"
                    "        <ECProperty propertyName=\"p\" typeName=\"int\" />"
                    "        <ECProperty propertyName=\"b\" typeName=\"int\" />"
                    "    </ECClass>"
                    "    <ECRelationshipClass typeName=\"ALikesB\" displayLabel=\"A likes B\" strength=\"referencing\">"
                    "        <Source cardinality=\"(1,1)\" roleLabel=\"likes\" polymorphic=\"False\">"
                    "            <Class class=\"ClassA\" />"
                    "        </Source>"
                    "        <Target cardinality=\"(1,1)\" roleLabel=\"is liked by\" polymorphic=\"True\">"
                    "            <Class class=\"ClassB\" />"
                    "        </Target>"
                    "        <ECProperty propertyName=\"p\" typeName=\"int\" />"
                    "        <ECProperty propertyName=\"Name\" typeName=\"string\" />"
                    "        <ECProperty propertyName=\"SourceOrderId\" typeName=\"int\" />"
                    "        <ECProperty propertyName=\"TargetOrderId\" typeName=\"int\" />"
                    "    </ECRelationshipClass>"
                    "</ECSchema>";

    return fmt;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
static ECSchemaPtr CreateTestSchema ()
    {
    Utf8String schemaXMLString = GetTestSchemaXMLString ();

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
    
    ECClassP sourceClass = schema->GetClassP ("ClassA");
    ASSERT_TRUE (NULL != sourceClass);
    StandaloneECEnablerPtr sourceEnabler =  sourceClass->GetDefaultStandaloneEnabler();
    ASSERT_TRUE (sourceEnabler.IsValid());

    ECClassP targetClass = schema->GetClassP ("ClassB");
    ASSERT_TRUE (NULL != targetClass);
    StandaloneECEnablerPtr targetEnabler =  targetClass->GetDefaultStandaloneEnabler();
    ASSERT_TRUE (targetEnabler.IsValid());

    ECValue p1;
    p1.SetInteger (123);

    ECN::StandaloneECInstancePtr sourceInstance = sourceEnabler->CreateInstance();
    sourceInstance->SetValue("p", p1);
    sourceInstance->SetInstanceId("source");

    ECValue p2, b2;
    p2.SetInteger (456);
    b2.SetInteger (789);
    ECN::StandaloneECInstancePtr targetInstance = targetEnabler->CreateInstance();
    targetInstance->SetValue("p", p2);
    targetInstance->SetValue("b", b2);
    targetInstance->SetInstanceId("target");

    ECRelationshipClassCP relClass = schema->GetClassP ("ALikesB")->GetRelationshipClassCP();
    ASSERT_TRUE (NULL != relClass);

    StandaloneECRelationshipEnablerPtr relationshipEnabler = StandaloneECRelationshipEnabler::CreateStandaloneRelationshipEnabler (*relClass);
    ASSERT_TRUE (relationshipEnabler.IsValid());

    StandaloneECRelationshipInstancePtr relationshipInstance = relationshipEnabler->CreateRelationshipInstance ();
    relationshipInstance->SetSource (sourceInstance.get());
    relationshipInstance->SetTarget (targetInstance.get());
    relationshipInstance->SetInstanceId ("source->target");

    IECInstancePtr readSource = relationshipInstance->GetSource();
    ECValue readValue;
    readSource->GetValue (readValue, "p");
    ASSERT_TRUE (readValue.Equals(p1));

    IECInstancePtr readTarget = relationshipInstance->GetTarget();
    readTarget->GetValue (readValue, "p");
    ASSERT_TRUE (readValue.Equals(p2));
    readTarget->GetValue (readValue, "b");
    ASSERT_TRUE (readValue.Equals(b2));
    }

 /*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECRelationshipTests, SetRelationshipProperties)
    {
    ECSchemaPtr schema = CreateTestSchema();
    ASSERT_TRUE (schema.IsValid());

    ECClassP sourceClass = schema->GetClassP ("ClassA");
    ASSERT_TRUE (NULL != sourceClass);
    StandaloneECEnablerPtr sourceEnabler =  sourceClass->GetDefaultStandaloneEnabler();
    ASSERT_TRUE (sourceEnabler.IsValid());

    ECClassP targetClass = schema->GetClassP ("ClassB");
    ASSERT_TRUE (NULL != targetClass);
    StandaloneECEnablerPtr targetEnabler =  targetClass->GetDefaultStandaloneEnabler();
    ASSERT_TRUE (targetEnabler.IsValid());

    ECValue p1;
    p1.SetInteger (123);

    ECN::StandaloneECInstancePtr sourceInstance = sourceEnabler->CreateInstance();
    sourceInstance->SetValue("p", p1);
    sourceInstance->SetInstanceId("source");

    ECValue p2, b2;
    p2.SetInteger (456);
    b2.SetInteger (789);
    ECN::StandaloneECInstancePtr targetInstance = targetEnabler->CreateInstance();
    targetInstance->SetValue("p", p2);
    targetInstance->SetValue("b", b2);
    targetInstance->SetInstanceId("target");

    ECRelationshipClassP relClass = dynamic_cast<ECRelationshipClassP>(schema->GetClassP ("ALikesB"));
    ASSERT_TRUE (NULL != relClass);

    StandaloneECRelationshipEnablerPtr relationshipEnabler = StandaloneECRelationshipEnabler::CreateStandaloneRelationshipEnabler (*relClass);
    ASSERT_TRUE (relationshipEnabler.IsValid());

    StandaloneECRelationshipInstancePtr relationshipInstance = relationshipEnabler->CreateRelationshipInstance ();
    relationshipInstance->SetSource (sourceInstance.get());
    relationshipInstance->SetTarget (targetInstance.get());
    relationshipInstance->SetInstanceId ("source->target");

    ECValue rel_p, rel_name, rel_sourceOrder, rel_targetOrder;
    rel_p.SetInteger (777);
    rel_sourceOrder.SetInteger (888);
    rel_targetOrder.SetInteger (999);
    rel_name.SetUtf8CP ("my_relationship");

    EXPECT_TRUE (SUCCESS == relationshipInstance->SetValue ("p", rel_p));
    EXPECT_TRUE (SUCCESS == relationshipInstance->SetValue ("Name", rel_name));
    EXPECT_TRUE (SUCCESS == relationshipInstance->SetValue ("SourceOrderId", rel_sourceOrder));
    EXPECT_TRUE (SUCCESS == relationshipInstance->SetValue ("TargetOrderId", rel_targetOrder));

    IECInstancePtr readSource = relationshipInstance->GetSource();
    ECValue readValue;
    readSource->GetValue (readValue, "p");
    ASSERT_TRUE (readValue.Equals(p1));

    IECInstancePtr readTarget = relationshipInstance->GetTarget();
    readTarget->GetValue (readValue, "p");
    ASSERT_TRUE (readValue.Equals(p2));
    readTarget->GetValue (readValue, "b");
    ASSERT_TRUE (readValue.Equals(b2));

    WString ecInstanceXml;

    InstanceWriteStatus status2 = relationshipInstance->WriteToXmlString(ecInstanceXml, true, true);
    EXPECT_EQ(INSTANCE_WRITE_STATUS_Success, status2);
    }

END_BENTLEY_ECN_TEST_NAMESPACE