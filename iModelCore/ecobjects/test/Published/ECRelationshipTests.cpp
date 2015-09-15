/*--------------------------------------------------------------------------------------+
|
|     $Source: test/Published/ECRelationshipTests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"
#include "regex"

BEGIN_BENTLEY_ECN_TEST_NAMESPACE
using namespace BentleyApi::ECN;

struct ECRelationshipTests : ECTestFixture
    {
    ECSchemaPtr m_schema;

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
            "        <ECArrayProperty propertyName=\"ArrayProperty\" typeName=\"string\" minOccurs=\"0\" maxOccurs=\"unbounded\"/>"
            "        <ECProperty propertyName=\"SourceOrderId\" typeName=\"int\" />"
            "        <ECProperty propertyName=\"TargetOrderId\" typeName=\"int\" />"
            "    </ECRelationshipClass>"
            "</ECSchema>";

        return fmt;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Bill.Steinbock                  04/2012
    +---------------+---------------+---------------+---------------+---------------+------*/
    void CreateTestSchema ()
        {
        Utf8String schemaXMLString = GetTestSchemaXMLString ();

        ECSchemaReadContextPtr  schemaContext = ECSchemaReadContext::CreateContext ();

        EXPECT_EQ (SUCCESS, ECSchema::ReadFromXmlString (m_schema, schemaXMLString.c_str (), *schemaContext));
        EXPECT_TRUE (m_schema.IsValid ());
        }

    bvector<Utf8String> split (Utf8String text, Utf8Char delimiter)
        {
        bvector<Utf8String> result;

        size_t start = 0;
        size_t end = text.find (delimiter, start);

        while (end != Utf8String::npos)
            {
            Utf8String token = text.substr (start, end - start);
            result.push_back (token);
            start = end + 1;
            end = text.find (delimiter, start);
            }

        return result;
        }
    };


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECRelationshipTests, SimpleRelationship)
    {
    CreateTestSchema ();

    ECClassP sourceClass = m_schema->GetClassP ("ClassA");
    ASSERT_TRUE (NULL != sourceClass);
    StandaloneECEnablerPtr sourceEnabler = sourceClass->GetDefaultStandaloneEnabler ();
    ASSERT_TRUE (sourceEnabler.IsValid ());

    ECClassP targetClass = m_schema->GetClassP ("ClassB");
    ASSERT_TRUE (NULL != targetClass);
    StandaloneECEnablerPtr targetEnabler = targetClass->GetDefaultStandaloneEnabler ();
    ASSERT_TRUE (targetEnabler.IsValid ());

    ECValue p1;
    p1.SetInteger (123);

    ECN::StandaloneECInstancePtr sourceInstance = sourceEnabler->CreateInstance ();
    sourceInstance->SetValue ("p", p1);
    sourceInstance->SetInstanceId ("source");

    ECValue p2, b2;
    p2.SetInteger (456);
    b2.SetInteger (789);
    ECN::StandaloneECInstancePtr targetInstance = targetEnabler->CreateInstance ();
    targetInstance->SetValue ("p", p2);
    targetInstance->SetValue ("b", b2);
    targetInstance->SetInstanceId ("target");

    ECRelationshipClassCP relClass = m_schema->GetClassP ("ALikesB")->GetRelationshipClassCP ();
    ASSERT_TRUE (NULL != relClass);

    StandaloneECRelationshipEnablerPtr relationshipEnabler = StandaloneECRelationshipEnabler::CreateStandaloneRelationshipEnabler (*relClass);
    ASSERT_TRUE (relationshipEnabler.IsValid ());

    StandaloneECRelationshipInstancePtr relationshipInstance = relationshipEnabler->CreateRelationshipInstance ();
    relationshipInstance->SetSource (sourceInstance.get ());
    relationshipInstance->SetTarget (targetInstance.get ());
    relationshipInstance->SetInstanceId ("source->target");

    IECInstancePtr readSource = relationshipInstance->GetSource ();
    ECValue readValue;
    readSource->GetValue (readValue, "p");
    ASSERT_TRUE (readValue.Equals (p1));

    IECInstancePtr readTarget = relationshipInstance->GetTarget ();
    readTarget->GetValue (readValue, "p");
    ASSERT_TRUE (readValue.Equals (p2));
    readTarget->GetValue (readValue, "b");
    ASSERT_TRUE (readValue.Equals (b2));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECRelationshipTests, SetRelationshipProperties)
    {
    CreateTestSchema ();

    ECClassP sourceClass = m_schema->GetClassP ("ClassA");
    ASSERT_TRUE (NULL != sourceClass);
    StandaloneECEnablerPtr sourceEnabler = sourceClass->GetDefaultStandaloneEnabler ();
    ASSERT_TRUE (sourceEnabler.IsValid ());

    ECClassP targetClass = m_schema->GetClassP ("ClassB");
    ASSERT_TRUE (NULL != targetClass);
    StandaloneECEnablerPtr targetEnabler = targetClass->GetDefaultStandaloneEnabler ();
    ASSERT_TRUE (targetEnabler.IsValid ());

    ECValue p1;
    p1.SetInteger (123);

    ECN::StandaloneECInstancePtr sourceInstance = sourceEnabler->CreateInstance ();
    sourceInstance->SetValue ("p", p1);
    sourceInstance->SetInstanceId ("source");

    ECValue p2, b2;
    p2.SetInteger (456);
    b2.SetInteger (789);
    ECN::StandaloneECInstancePtr targetInstance = targetEnabler->CreateInstance ();
    targetInstance->SetValue ("p", p2);
    targetInstance->SetValue ("b", b2);
    targetInstance->SetInstanceId ("target");

    ECRelationshipClassP relClass = dynamic_cast<ECRelationshipClassP>(m_schema->GetClassP ("ALikesB"));
    ASSERT_TRUE (NULL != relClass);

    StandaloneECRelationshipEnablerPtr relationshipEnabler = StandaloneECRelationshipEnabler::CreateStandaloneRelationshipEnabler (*relClass);
    ASSERT_TRUE (relationshipEnabler.IsValid ());

    StandaloneECRelationshipInstancePtr relationshipInstance = relationshipEnabler->CreateRelationshipInstance ();
    relationshipInstance->SetSource (sourceInstance.get ());
    relationshipInstance->SetTarget (targetInstance.get ());
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

    IECInstancePtr readSource = relationshipInstance->GetSource ();
    ECValue readValue;
    readSource->GetValue (readValue, "p");
    ASSERT_TRUE (readValue.Equals (p1));

    IECInstancePtr readTarget = relationshipInstance->GetTarget ();
    readTarget->GetValue (readValue, "p");
    ASSERT_TRUE (readValue.Equals (p2));
    readTarget->GetValue (readValue, "b");
    ASSERT_TRUE (readValue.Equals (b2));

    WString ecInstanceXml;

    InstanceWriteStatus status2 = relationshipInstance->WriteToXmlString (ecInstanceXml, true, true);
    EXPECT_EQ (INSTANCE_WRITE_STATUS_Success, status2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Raimondas.Rimkus               02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECRelationshipTests, InstanceSettersAndGetters)
    {
    CreateTestSchema ();

    ECRelationshipClassCP relClass = m_schema->GetClassP ("ALikesB")->GetRelationshipClassCP ();
    ASSERT_TRUE (NULL != relClass);

    StandaloneECRelationshipEnablerPtr relationshipEnabler = StandaloneECRelationshipEnabler::CreateStandaloneRelationshipEnabler (*relClass);
    StandaloneECRelationshipInstancePtr relationshipInstance = relationshipEnabler->CreateRelationshipInstance ();

    EXPECT_STREQ (relationshipEnabler->GetName (), "Bentley::ECN::StandaloneECEnabler");
    EXPECT_STREQ (relationshipInstance->GetName (), "");
    relationshipInstance->SetName ("Some name");
    EXPECT_STREQ (relationshipInstance->GetName (), "Some name");

    EXPECT_STREQ (relationshipInstance->GetRelationshipEnabler ().GetECEnabler ().GetClass ().GetFullName (), "RelationshipTesting:ALikesB");
    EXPECT_STREQ (relationshipInstance->GetRelationshipClass ().GetFullName (), "RelationshipTesting:ALikesB");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Raimondas.Rimkus               02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECRelationshipTests, InheritedEnablerIterator)
    {
    CreateTestSchema ();
    ECRelationshipClassCP relClass = m_schema->GetClassP ("ALikesB")->GetRelationshipClassCP ();
    StandaloneECRelationshipEnablerPtr relationshipEnabler = StandaloneECRelationshipEnabler::CreateStandaloneRelationshipEnabler (*relClass);

    Utf8CP props[] = { "p", "SourceOrderId", "TargetOrderId", "Name", "Source ECPointer", "Target ECPointer", "ArrayProperty" };

    // Target/Source ECPointer are not properties of the ECClass, they are special access strings for relationship enablers...
    EXPECT_EQ (relationshipEnabler->GetClass ().GetPropertyCount () + 2, sizeof(props) / sizeof(props[0]));
    uint32_t tempIndex = relationshipEnabler->GetFirstPropertyIndex (0);
    EXPECT_TRUE (relationshipEnabler->HasChildProperties (0));

    while (tempIndex != 0)
        {
        Utf8CP tempName = NULL;
        EXPECT_EQ (relationshipEnabler->GetAccessString (tempName, tempIndex), ECOBJECTS_STATUS_Success);
        EXPECT_FALSE (relationshipEnabler->HasChildProperties (tempIndex));

        int foundPos = -1;
        for (int i = 0; i<sizeof(props) / sizeof(props[0]); i++)
        if (props[i] != NULL && strcmp (props[i], tempName) == 0)
            {
            foundPos = i;
            break;
            }
        EXPECT_NE (foundPos, -1);
        if (foundPos > -1)
            props[foundPos] = NULL;

        tempIndex = relationshipEnabler->GetNextPropertyIndex (0, tempIndex);
        }

    for (int i = 0; i < sizeof(props) / sizeof(props[0]); i++)
        EXPECT_STREQ (props[i], NULL);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Raimondas.Rimkus               02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECRelationshipTests, InheritedEnablerIndices)
    {
    CreateTestSchema ();
    ECRelationshipClassCP relClass = m_schema->GetClassP ("ALikesB")->GetRelationshipClassCP ();
    StandaloneECRelationshipEnablerPtr relationshipEnabler = StandaloneECRelationshipEnabler::CreateStandaloneRelationshipEnabler (*relClass);

    Utf8CP props[] = { "p", "SourceOrderId", "TargetOrderId", "Name", "Source ECPointer", "Target ECPointer", "ArrayProperty" };

    bvector<uint32_t> indices;
    EXPECT_EQ (relationshipEnabler->GetPropertyIndices (indices, 0), ECOBJECTS_STATUS_Success);
    EXPECT_EQ (indices.size (), sizeof(props) / sizeof(props[0]));

    for (uint32_t i = 0; i < indices.size (); i++)
        {
        Utf8CP tempName = NULL;
        EXPECT_EQ (relationshipEnabler->GetAccessString (tempName, indices[i]), ECOBJECTS_STATUS_Success);
        EXPECT_FALSE (relationshipEnabler->HasChildProperties (indices[i]));

        int foundPos = -1;
        for (int i = 0; i<sizeof(props) / sizeof(props[0]); i++)
        if (props[i] != NULL && strcmp (props[i], tempName) == 0)
            {
            foundPos = i;
            break;
            }
        EXPECT_NE (foundPos, -1);
        if (foundPos > -1)
            props[foundPos] = NULL;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Raimondas.Rimkus               02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECRelationshipTests, InheritedSetGetValues)
    {
    CreateTestSchema ();
    ECRelationshipClassCP relClass = m_schema->GetClassP ("ALikesB")->GetRelationshipClassCP ();

    StandaloneECRelationshipEnablerPtr relationshipEnabler = StandaloneECRelationshipEnabler::CreateStandaloneRelationshipEnabler (*relClass);
    StandaloneECRelationshipInstancePtr relationshipInstance = relationshipEnabler->CreateRelationshipInstance ();

    ECValue value;
    EXPECT_EQ (relationshipInstance->SetValue ("Name", ECValue ("Some value 1")), ECOBJECTS_STATUS_Success);
    EXPECT_EQ (relationshipInstance->GetValue (value, "Name"), ECOBJECTS_STATUS_Success);
    EXPECT_STREQ ("Some value 1", value.GetUtf8CP ());

    EXPECT_EQ (relationshipInstance->SetValue ("p", ECValue (42)), ECOBJECTS_STATUS_Success);
    EXPECT_EQ (relationshipInstance->GetValue (value, "p"), ECOBJECTS_STATUS_Success);
    EXPECT_EQ (42, value.GetInteger ());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Raimondas.Rimkus               02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECRelationshipTests, InheritedSetGetArrayValues)
    {
    CreateTestSchema ();

    ECRelationshipClassCP relClass = m_schema->GetClassP ("ALikesB")->GetRelationshipClassCP ();

    StandaloneECRelationshipEnablerPtr relationshipEnabler = StandaloneECRelationshipEnabler::CreateStandaloneRelationshipEnabler (*relClass);
    StandaloneECRelationshipInstancePtr relationshipInstance = relationshipEnabler->CreateRelationshipInstance ();

    ECValue value;
    EXPECT_EQ (relationshipInstance->AddArrayElements ("ArrayProperty", 13), ECOBJECTS_STATUS_Success);
    EXPECT_EQ (relationshipInstance->GetValue (value, "ArrayProperty"), ECOBJECTS_STATUS_Success);
    EXPECT_EQ (value.GetArrayInfo ().GetCount (), 13);

    EXPECT_EQ (relationshipInstance->SetValue ("ArrayProperty", ECValue ("Seventh Value"), 7), ECOBJECTS_STATUS_Success);
    EXPECT_EQ (relationshipInstance->SetValue ("ArrayProperty", ECValue ("First Value"), 1), ECOBJECTS_STATUS_Success);
    EXPECT_EQ (relationshipInstance->GetValue (value, "ArrayProperty", 7), ECOBJECTS_STATUS_Success);
    EXPECT_STREQ (value.GetUtf8CP (), "Seventh Value");

    EXPECT_EQ (relationshipInstance->InsertArrayElements ("ArrayProperty", 2, 29), ECOBJECTS_STATUS_Success);
    EXPECT_EQ (relationshipInstance->GetValue (value, "ArrayProperty"), ECOBJECTS_STATUS_Success);
    EXPECT_EQ (value.GetArrayInfo ().GetCount (), 42);

    EXPECT_EQ (relationshipInstance->GetValue (value, "ArrayProperty", 36), ECOBJECTS_STATUS_Success);
    EXPECT_STREQ (value.GetUtf8CP (), "Seventh Value");

    EXPECT_EQ (relationshipInstance->RemoveArrayElement ("ArrayProperty", 36), ECOBJECTS_STATUS_Success);
    EXPECT_EQ (relationshipInstance->RemoveArrayElement ("ArrayProperty", 36), ECOBJECTS_STATUS_Success);
    EXPECT_EQ (relationshipInstance->RemoveArrayElement ("ArrayProperty", 0), ECOBJECTS_STATUS_Success);

    EXPECT_EQ (relationshipInstance->GetValue (value, "ArrayProperty", 0), ECOBJECTS_STATUS_Success);
    EXPECT_STREQ (value.GetUtf8CP (), "First Value");

    EXPECT_EQ (relationshipInstance->GetValue (value, "ArrayProperty"), ECOBJECTS_STATUS_Success);
    EXPECT_EQ (value.GetArrayInfo ().GetCount (), 39);

    //No idea why clearing array is not allowed
    EXPECT_EQ (relationshipInstance->ClearArray ("ArrayProperty"), ECOBJECTS_STATUS_OperationNotSupported);
    EXPECT_EQ (value.GetArrayInfo ().GetCount (), 39);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Raimondas.Rimkus               02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
//Source and dest IDs are hardcoded to 0 in StandaloneECRelationshipInstance.cpp
TEST_F (ECRelationshipTests, SourceDestOrderIDs)
    {
    int64_t sourceId, targetId;

    CreateTestSchema ();
    ECRelationshipClassCP relClass = m_schema->GetClassP ("ALikesB")->GetRelationshipClassCP ();

    StandaloneECRelationshipEnablerPtr relationshipEnabler = StandaloneECRelationshipEnabler::CreateStandaloneRelationshipEnabler (*relClass);
    StandaloneECRelationshipInstancePtr relationshipInstance = relationshipEnabler->CreateRelationshipInstance ();

    EXPECT_EQ (relationshipInstance->GetSourceOrderId (sourceId), ECOBJECTS_STATUS_Success);
    EXPECT_EQ (sourceId, 0);
    EXPECT_EQ (relationshipInstance->GetTargetOrderId (targetId), ECOBJECTS_STATUS_Success);
    EXPECT_EQ (targetId, 0);

    EXPECT_TRUE (relationshipInstance->GetSource ().IsNull ());
    EXPECT_TRUE (relationshipInstance->GetTarget ().IsNull ());

    relationshipInstance->SetSource (m_schema->GetClassP ("ClassA")->GetDefaultStandaloneEnabler ()->CreateInstance ().get ());
    relationshipInstance->SetTarget (m_schema->GetClassP ("ClassB")->GetDefaultStandaloneEnabler ()->CreateInstance ().get ());

    EXPECT_STREQ (relationshipInstance->GetSource ()->GetClass ().GetFullName (), "RelationshipTesting:ClassA");
    EXPECT_STREQ (relationshipInstance->GetTarget ()->GetClass ().GetFullName (), "RelationshipTesting:ClassB");

    EXPECT_EQ (relationshipInstance->GetSourceOrderId (sourceId), ECOBJECTS_STATUS_Success);
    EXPECT_EQ (sourceId, 0);
    EXPECT_EQ (relationshipInstance->GetTargetOrderId (targetId), ECOBJECTS_STATUS_Success);
    EXPECT_EQ (targetId, 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Raimondas.Rimkus               02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECRelationshipTests, DumpToString)
    {
    CreateTestSchema ();
    ECRelationshipClassCP relClass = m_schema->GetClassP ("ALikesB")->GetRelationshipClassCP ();

    StandaloneECRelationshipEnablerPtr relationshipEnabler = StandaloneECRelationshipEnabler::CreateStandaloneRelationshipEnabler (*relClass);
    StandaloneECRelationshipInstancePtr relationshipInstance = relationshipEnabler->CreateRelationshipInstance ();

    relationshipInstance->SetSource (m_schema->GetClassP ("ClassA")->GetDefaultStandaloneEnabler ()->CreateInstance ().get ());
    relationshipInstance->SetTarget (m_schema->GetClassP ("ClassB")->GetDefaultStandaloneEnabler ()->CreateInstance ().get ());
    EXPECT_EQ (relationshipInstance->SetValue ("Name", ECValue ("Some value 1")), ECOBJECTS_STATUS_Success);
    EXPECT_EQ (relationshipInstance->SetValue ("p", ECValue (42)), ECOBJECTS_STATUS_Success);

    Utf8String outStr = relationshipInstance->ToString ("~!@");
    bvector<Utf8String> strSplit = split (outStr, '\n');
    Utf8String strMatches[] = { "^~!@================== ECInstance Dump #\\d+ =============================$",
        "^~!@ECClass=ALikesB at address = 0x[a-f0-9]{2,8}$",
        "^~!@  \\[0x[a-f0-9]{2,8}\\]\\[   5\\] Nullflags\\[0\\] = 0x[a-f0-9]{1,8}$",
        "^~!@  \\[0x[a-f0-9]{2,8}\\]\\[   4\\] p = 42$",
        "^~!@  \\[0x[a-f0-9]{2,8}\\]\\[   8\\] SourceOrderId = <null>$",
        "^~!@  \\[0x[a-f0-9]{2,8}\\]\\[  12\\] TargetOrderId = <null>$",
        "^~!@  \\[0x[a-f0-9]{2,8}\\]\\[  16\\] -> \\[0x[a-f0-9]{2,8}\\]\\[  36\\] Name = Some value 1$",
        "^~!@  \\[0x[a-f0-9]{2,8}\\]\\[  20\\] -> \\[0x[a-f0-9]{2,8}\\]\\[  49\\] ArrayProperty = Count: 0 IsFixedSize: 0$",
        "^~!@  \\[0x[a-f0-9]{2,8}\\]\\[  24\\] -> \\[0x[a-f0-9]{2,8}\\]\\[    \\] Source ECPointer = <null>$",
        "^~!@  \\[0x[a-f0-9]{2,8}\\]\\[  28\\] -> \\[0x[a-f0-9]{2,8}\\]\\[    \\] Target ECPointer = <null>$",
        "^~!@  \\[0x[a-f0-9]{2,8}\\]\\[  32\\] Offset of TheEnd = 49$", };

    EXPECT_EQ (sizeof(strMatches) / sizeof(strMatches[0]), strSplit.size ());
    size_t check_size = sizeof(strMatches) / sizeof(strMatches[0]) < strSplit.size () ? sizeof(strMatches) / sizeof(strMatches[0]) : strSplit.size ();

    for (int i = 0; i<(int)check_size; i++)
        {
        bool match = std::regex_match (strSplit[i].c_str (), std::regex (strMatches[i].c_str ()));
        EXPECT_TRUE (match);
        if (!match)
            printf ("Line %d: %s\nDoesn't match regex: %s\n", i, strSplit[i].c_str (), strMatches[i].c_str ());
        }
    }

END_BENTLEY_ECN_TEST_NAMESPACE