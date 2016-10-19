/*--------------------------------------------------------------------------------------+
|
|     $Source: test/Published/ECRelationshipTests.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
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

    //struct ECRelationshipInstanceInterface : IECWipRelationshipInstance
    //    {
    //    private:
    //        StandaloneECInstancePtr m_standaloneInstance;
    //    public:
    //        ECRelationshipInstanceInterface (StandaloneECEnablerR enabler) : IECWipRelationshipInstance (enabler) 
    //            {
    //            m_standaloneInstance = enabler.CreateInstance ();
    //            }

    //        virtual BentleyStatus  _SetName (Utf8CP name) override
    //            {
    //            if (ECObjectsStatus::Success == m_standaloneInstance->SetValue ("Name", ECValue (name)))
    //                return SUCCESS;
    //            return ERROR;
    //            }

    //        virtual BentleyStatus  _SetSourceOrderId (int64_t sourceOrderId) override
    //            {
    //            if (ECObjectsStatus::Success == m_standaloneInstance->SetValue ("SourceOrderId", ECValue ((int32_t)sourceOrderId)))
    //                return SUCCESS;
    //            return ERROR;
    //            }

    //        virtual BentleyStatus  _SetTargetOrderId (int64_t targetOrderId) override
    //            {
    //            if (ECObjectsStatus::Success == m_standaloneInstance->SetValue ("TargetOrderId", ECValue ((int32_t)targetOrderId)))
    //                return SUCCESS;
    //            return ERROR;
    //            }
    //    };

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

        EXPECT_EQ (SchemaReadStatus::Success, ECSchema::ReadFromXmlString (m_schema, schemaXMLString.c_str (), *schemaContext));
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

    EXPECT_TRUE (ECObjectsStatus::Success == relationshipInstance->SetValue ("p", rel_p));
    EXPECT_TRUE (ECObjectsStatus::Success == relationshipInstance->SetValue ("Name", rel_name));
    EXPECT_TRUE (ECObjectsStatus::Success == relationshipInstance->SetValue ("SourceOrderId", rel_sourceOrder));
    EXPECT_TRUE (ECObjectsStatus::Success == relationshipInstance->SetValue ("TargetOrderId", rel_targetOrder));

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
    EXPECT_EQ (InstanceWriteStatus::Success, status2);
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
        EXPECT_EQ (relationshipEnabler->GetAccessString (tempName, tempIndex), ECObjectsStatus::Success);
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
        EXPECT_TRUE(nullptr == props[i]);
        // EXPECT_STREQ (props[i], NULL);
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
    EXPECT_EQ (relationshipEnabler->GetPropertyIndices (indices, 0), ECObjectsStatus::Success);
    EXPECT_EQ (indices.size (), sizeof(props) / sizeof(props[0]));

    for (uint32_t i = 0; i < indices.size (); i++)
        {
        Utf8CP tempName = NULL;
        EXPECT_EQ (relationshipEnabler->GetAccessString (tempName, indices[i]), ECObjectsStatus::Success);
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
    EXPECT_EQ (relationshipInstance->SetValue ("Name", ECValue ("Some value 1")), ECObjectsStatus::Success);
    EXPECT_EQ (relationshipInstance->GetValue (value, "Name"), ECObjectsStatus::Success);
    EXPECT_STREQ ("Some value 1", value.GetUtf8CP ());

    EXPECT_EQ (relationshipInstance->SetValue ("p", ECValue (42)), ECObjectsStatus::Success);
    EXPECT_EQ (relationshipInstance->GetValue (value, "p"), ECObjectsStatus::Success);
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
    EXPECT_EQ (relationshipInstance->AddArrayElements ("ArrayProperty", 13), ECObjectsStatus::Success);
    EXPECT_EQ (relationshipInstance->GetValue (value, "ArrayProperty"), ECObjectsStatus::Success);
    EXPECT_EQ (value.GetArrayInfo ().GetCount (), 13);

    EXPECT_EQ (relationshipInstance->SetValue ("ArrayProperty", ECValue ("Seventh Value"), 7), ECObjectsStatus::Success);
    EXPECT_EQ (relationshipInstance->SetValue ("ArrayProperty", ECValue ("First Value"), 1), ECObjectsStatus::Success);
    EXPECT_EQ (relationshipInstance->GetValue (value, "ArrayProperty", 7), ECObjectsStatus::Success);
    EXPECT_STREQ (value.GetUtf8CP (), "Seventh Value");

    EXPECT_EQ (relationshipInstance->InsertArrayElements ("ArrayProperty", 2, 29), ECObjectsStatus::Success);
    EXPECT_EQ (relationshipInstance->GetValue (value, "ArrayProperty"), ECObjectsStatus::Success);
    EXPECT_EQ (value.GetArrayInfo ().GetCount (), 42);

    EXPECT_EQ (relationshipInstance->GetValue (value, "ArrayProperty", 36), ECObjectsStatus::Success);
    EXPECT_STREQ (value.GetUtf8CP (), "Seventh Value");

    EXPECT_EQ (relationshipInstance->RemoveArrayElement ("ArrayProperty", 36), ECObjectsStatus::Success);
    EXPECT_EQ (relationshipInstance->RemoveArrayElement ("ArrayProperty", 36), ECObjectsStatus::Success);
    EXPECT_EQ (relationshipInstance->RemoveArrayElement ("ArrayProperty", 0), ECObjectsStatus::Success);

    EXPECT_EQ (relationshipInstance->GetValue (value, "ArrayProperty", 0), ECObjectsStatus::Success);
    EXPECT_STREQ (value.GetUtf8CP (), "First Value");

    EXPECT_EQ (relationshipInstance->GetValue (value, "ArrayProperty"), ECObjectsStatus::Success);
    EXPECT_EQ (value.GetArrayInfo ().GetCount (), 39);

    //No idea why clearing array is not allowed
    EXPECT_EQ (relationshipInstance->ClearArray ("ArrayProperty"), ECObjectsStatus::OperationNotSupported);
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

    EXPECT_EQ (relationshipInstance->GetSourceOrderId (sourceId), ECObjectsStatus::Success);
    EXPECT_EQ (sourceId, 0);
    EXPECT_EQ (relationshipInstance->GetTargetOrderId (targetId), ECObjectsStatus::Success);
    EXPECT_EQ (targetId, 0);

    EXPECT_TRUE (relationshipInstance->GetSource ().IsNull ());
    EXPECT_TRUE (relationshipInstance->GetTarget ().IsNull ());

    relationshipInstance->SetSource (m_schema->GetClassP ("ClassA")->GetDefaultStandaloneEnabler ()->CreateInstance ().get ());
    relationshipInstance->SetTarget (m_schema->GetClassP ("ClassB")->GetDefaultStandaloneEnabler ()->CreateInstance ().get ());

    EXPECT_STREQ (relationshipInstance->GetSource ()->GetClass ().GetFullName (), "RelationshipTesting:ClassA");
    EXPECT_STREQ (relationshipInstance->GetTarget ()->GetClass ().GetFullName (), "RelationshipTesting:ClassB");

    EXPECT_EQ (relationshipInstance->GetSourceOrderId (sourceId), ECObjectsStatus::Success);
    EXPECT_EQ (sourceId, 0);
    EXPECT_EQ (relationshipInstance->GetTargetOrderId (targetId), ECObjectsStatus::Success);
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
    EXPECT_EQ (relationshipInstance->SetValue ("Name", ECValue ("Some value 1")), ECObjectsStatus::Success);
    EXPECT_EQ (relationshipInstance->SetValue ("p", ECValue (42)), ECObjectsStatus::Success);

    Utf8String outStr = relationshipInstance->ToString ("~!@");
    bvector<Utf8String> strSplit = split (outStr, '\n');
    Utf8String strMatches[] = { "^~!@================== ECInstance Dump #\\d+ =============================$",
        "^~!@ECClass=ALikesB at address = 0x[a-f0-9]{2,8}$",
        "^~!@  \\[0x[a-f0-9]{2,8}\\]\\[   5\\] Nullflags\\[0\\] = 0x[a-f0-9]{1,8}$",
        "^~!@  \\[0x[a-f0-9]{2,8}\\]\\[   4\\] p = 42$",
        "^~!@  \\[0x[a-f0-9]{2,8}\\]\\[   8\\] SourceOrderId = <null>$",
        "^~!@  \\[0x[a-f0-9]{2,8}\\]\\[  12\\] TargetOrderId = <null>$",
        "^~!@  \\[0x[a-f0-9]{2,8}\\]\\[  16\\] -> \\[0x[a-f0-9]{2,8}\\]\\[  36\\] Name = Some value 1$",
        "^~!@  \\[0x[a-f0-9]{2,8}\\]\\[  20\\] -> \\[0x[a-f0-9]{2,8}\\]\\[  62\\] ArrayProperty = Count: 0 IsFixedSize: 0$",
        "^~!@  \\[0x[a-f0-9]{2,8}\\]\\[  24\\] -> \\[0x[a-f0-9]{2,8}\\]\\[    \\] Source ECPointer = <null>$",
        "^~!@  \\[0x[a-f0-9]{2,8}\\]\\[  28\\] -> \\[0x[a-f0-9]{2,8}\\]\\[    \\] Target ECPointer = <null>$",
        "^~!@  \\[0x[a-f0-9]{2,8}\\]\\[  32\\] Offset of TheEnd = 62$", };

    EXPECT_EQ (sizeof(strMatches) / sizeof(strMatches[0]), strSplit.size ());
    size_t check_size = sizeof(strMatches) / sizeof(strMatches[0]) < strSplit.size () ? sizeof(strMatches) / sizeof(strMatches[0]) : strSplit.size ();

    for (int i = 0; i < (int)check_size; i++)
        {
        bool match = std::regex_match (strSplit[i].c_str (), std::regex (strMatches[i].c_str ()));
        EXPECT_TRUE (match);
        if (!match)
            printf ("Line %d: %s\nDoesn't match regex: %s\n", i, strSplit[i].c_str (), strMatches[i].c_str ());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECRelationshipTests, TestsRelationshipStrengthAndDirectionConstraints)
    {
    ECSchemaPtr schemaPtr;
    ECSchema::CreateSchema(schemaPtr, "TestSchema", "ts", 1, 0, 0);

    ECSchemaP ecSchema = schemaPtr.get();

    ECRelationshipClassP relationClass1;
    ecSchema->CreateRelationshipClass(relationClass1, "RelClass1");
    relationClass1->SetStrength(StrengthType::Referencing);
    relationClass1->SetStrengthDirection(ECRelatedInstanceDirection::Forward);

    ECRelationshipClassP relationClass2;
    ecSchema->CreateRelationshipClass(relationClass2, "RelClass2");
    relationClass2->SetStrength(StrengthType::Holding);
    relationClass2->SetStrengthDirection(ECRelatedInstanceDirection::Forward);

    ECRelationshipClassP relationClass3;
    ecSchema->CreateRelationshipClass(relationClass3, "RelClass3");
    relationClass3->SetStrength(StrengthType::Referencing);
    relationClass3->SetStrengthDirection(ECRelatedInstanceDirection::Backward);

    ECRelationshipClassP relationClass4;
    ecSchema->CreateRelationshipClass(relationClass4, "RelClass4");
    relationClass4->SetStrength(StrengthType::Referencing);
    relationClass4->SetStrengthDirection(ECRelatedInstanceDirection::Forward);

    // Tests the base class validation. If strength and direction are not same, they
    // won't be assigned.

    // #1 Direction same, Strength differs
    EXPECT_EQ(ECObjectsStatus::RelationshipConstraintsNotCompatible, relationClass1->AddBaseClass(*relationClass2)) << "Strength was supposed to have different values (Referencing/Holding) so base class was rejected";
    // #2 Strength same, Direction differs
    EXPECT_EQ(ECObjectsStatus::RelationshipConstraintsNotCompatible, relationClass1->AddBaseClass(*relationClass3)) << "Strength Direction was supposed to have different values (Forward/Backward) so base class was rejected";
    // #3 Strength and Direction are same
    EXPECT_EQ(ECObjectsStatus::Success, relationClass1->AddBaseClass(*relationClass4)) << "Failing with equal Strength Direction and Strength, so baseclass should have been accepted.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECRelationshipTests, TestsRelationshipConstraints)
    {
    ECSchemaPtr schemaPtr;
    ECSchema::CreateSchema(schemaPtr, "TestSchema", "ts", 1, 0, 0);

    ECSchemaP ecSchema = schemaPtr.get();

    ECEntityClassP classA;
    ecSchema->CreateEntityClass(classA, "ClassA");

    ECEntityClassP classB;
    ecSchema->CreateEntityClass(classB, "ClassB");
    classB->AddBaseClass(*classA);

    ECEntityClassP classC;
    ecSchema->CreateEntityClass(classC, "ClassC");
    classC->AddBaseClass(*classB);

    ECRelationshipClassP relationClass;
    ecSchema->CreateRelationshipClass(relationClass, "RelClass");
    relationClass->SetStrength(StrengthType::Referencing);
    relationClass->SetStrengthDirection(ECRelatedInstanceDirection::Forward);

    ECRelationshipClassP relationClassBase;
    ecSchema->CreateRelationshipClass(relationClassBase, "RelBaseClass");
    relationClassBase->SetStrength(StrengthType::Referencing);
    relationClassBase->SetStrengthDirection(ECRelatedInstanceDirection::Forward);
    
    ECRelationshipConstraintR baseSourceContraint = relationClassBase->GetSource();
    baseSourceContraint.AddClass(*classB);
    
    EXPECT_EQ(ECObjectsStatus::Success, relationClass->AddBaseClass(*relationClassBase));

    // #1 ClassB is the constraint class so it should work...
    EXPECT_EQ(ECObjectsStatus::Success, relationClass->GetSource().AddClass(*classB));
    // #2 ClassC is deriving from ClassA so it should work too...
    EXPECT_EQ(ECObjectsStatus::Success, relationClass->GetSource().AddClass(*classC));
    // #3 ClassA is the base class of ClassB but violates the base constraints (expects ClassB or bigger)
    EXPECT_EQ(ECObjectsStatus::RelationshipConstraintsNotCompatible, relationClass->GetSource().AddClass(*classA));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECRelationshipTests, TestRelationshipMultiplicityConstraint)
    {
    ECSchemaPtr schemaPtr;
    ECSchema::CreateSchema(schemaPtr, "TestSchema", "ts", 1, 0, 0);

    ECSchemaP ecSchema = schemaPtr.get();

    ECRelationshipClassP relationClassBase;
    ecSchema->CreateRelationshipClass(relationClassBase, "RelBaseClass");
    relationClassBase->SetStrength(StrengthType::Referencing);
    relationClassBase->SetStrengthDirection(ECRelatedInstanceDirection::Forward);
    relationClassBase->GetSource().SetMultiplicity(RelationshipMultiplicity::OneMany());
    relationClassBase->GetTarget().SetMultiplicity(RelationshipMultiplicity::OneMany());

    ECRelationshipClassP relationClass;
    ecSchema->CreateRelationshipClass(relationClass, "RelClass");
    relationClass->SetStrength(StrengthType::Referencing);
    relationClass->SetStrengthDirection(ECRelatedInstanceDirection::Forward);
    
    ASSERT_EQ(ECObjectsStatus::Success, relationClass->AddBaseClass(*relationClassBase));
    ASSERT_EQ(ECObjectsStatus::RelationshipConstraintsNotCompatible, relationClass->GetSource().SetMultiplicity(RelationshipMultiplicity::ZeroMany()));
    ASSERT_EQ(ECObjectsStatus::RelationshipConstraintsNotCompatible, relationClass->GetTarget().SetMultiplicity(RelationshipMultiplicity::ZeroMany()));
    ASSERT_EQ(ECObjectsStatus::Success, relationClass->GetSource().SetMultiplicity(RelationshipMultiplicity::OneOne()));
    ASSERT_EQ(ECObjectsStatus::Success, relationClass->GetTarget().SetMultiplicity(RelationshipMultiplicity::OneOne()));

    ECRelationshipClassP relationClass2;
    ecSchema->CreateRelationshipClass(relationClass2, "RelClass2");
    relationClass2->SetStrength(StrengthType::Referencing);
    relationClass2->SetStrengthDirection(ECRelatedInstanceDirection::Forward);
    relationClass2->GetSource().SetMultiplicity(RelationshipMultiplicity::ZeroMany());
    relationClass2->GetTarget().SetMultiplicity(RelationshipMultiplicity::ZeroMany());

    ASSERT_EQ(ECObjectsStatus::BaseClassUnacceptable, relationClass2->AddBaseClass(*relationClassBase));

    relationClass2->GetSource().SetMultiplicity(RelationshipMultiplicity::OneMany());
    relationClass2->GetTarget().SetMultiplicity(RelationshipMultiplicity::OneOne());
    ASSERT_EQ(ECObjectsStatus::Success, relationClass2->AddBaseClass(*relationClassBase));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECRelationshipTests, TestRoleLabelInheritance)
    {
    ECSchemaPtr schemaPtr;
    ECSchema::CreateSchema(schemaPtr, "TestSchema", "ts", 1, 0, 0);

    ECSchemaP ecSchema = schemaPtr.get();

    ECRelationshipClassP relationClass;
    ecSchema->CreateRelationshipClass(relationClass, "RelClass");
    relationClass->SetStrength(StrengthType::Referencing);
    relationClass->SetStrengthDirection(ECRelatedInstanceDirection::Forward);
    EXPECT_TRUE(Utf8String::IsNullOrEmpty(relationClass->GetSource().GetInvariantRoleLabel().c_str())) << "The Source constraint's roleLabel is expected to be empty when a relationship class is first created.";
    EXPECT_TRUE(Utf8String::IsNullOrEmpty(relationClass->GetTarget().GetInvariantRoleLabel().c_str())) << "The Target constraint's roleLabel is expected to be empty when a relationship class is first created.";

    ECRelationshipClassP relationClassBase;
    ecSchema->CreateRelationshipClass(relationClassBase, "RelBaseClass");
    relationClassBase->SetStrength(StrengthType::Referencing);
    relationClassBase->SetStrengthDirection(ECRelatedInstanceDirection::Forward);
    relationClassBase->GetSource().SetRoleLabel("Test Source");
    relationClassBase->GetTarget().SetRoleLabel("Test Target");

    EXPECT_TRUE(relationClassBase->GetSource().IsRoleLabelDefined()) << "The Source constraint's role label was set, so it should be defined.";
    EXPECT_TRUE(relationClassBase->GetTarget().IsRoleLabelDefined()) << "The Target constraint's role label was set, so it should be defined.";
    EXPECT_TRUE(relationClassBase->GetSource().IsRoleLabelDefinedLocally()) << "The Source constraint's role label was set locally, so it should be defined locally.";
    EXPECT_TRUE(relationClassBase->GetTarget().IsRoleLabelDefinedLocally()) << "The Target constraint's role label was set locally, so it should be defined locally.";

    ASSERT_EQ(ECObjectsStatus::Success, relationClass->AddBaseClass(*relationClassBase)) << "There should not be any conflicts between the class and the base class trying to be added.";

    EXPECT_STREQ("Test Source", relationClass->GetSource().GetInvariantRoleLabel().c_str()) << "The Source constraint should find be able to find the role label from base relationship.";
    EXPECT_STREQ("Test Target", relationClass->GetTarget().GetInvariantRoleLabel().c_str()) << "The Target constraint should find be able to find the role label from base relationship.";
    EXPECT_TRUE(relationClass->GetSource().IsRoleLabelDefined()) << "The Source constraint's role label was set in a base class, so it should be defined.";
    EXPECT_TRUE(relationClass->GetTarget().IsRoleLabelDefined()) << "The Target constraint's role label was set in a base class, so it should be defined.";
    EXPECT_FALSE(relationClass->GetSource().IsRoleLabelDefinedLocally()) << "The Source constraint's role label was set in a base class, so it should not be locally defined.";
    EXPECT_FALSE(relationClass->GetTarget().IsRoleLabelDefinedLocally()) << "The Target constraint's role label was set in a base class, so it should not be locally defined.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECRelationshipTests, TestBaseClassRules)
    {
    ECSchemaPtr schemaPtr;
    ECSchema::CreateSchema(schemaPtr, "TestSchema", "ts", 1, 0, 0);

    ECSchemaP ecSchema = schemaPtr.get();

    ECEntityClassP entityClassA;
    ECEntityClassP entityClassB;
    ECEntityClassP entityClassC;
    ecSchema->CreateEntityClass(entityClassA, "A");
    ecSchema->CreateEntityClass(entityClassB, "B");
    ecSchema->CreateEntityClass(entityClassC, "C");

    ECRelationshipClassP relationClass;
    ecSchema->CreateRelationshipClass(relationClass, "ARelB");
    relationClass->SetStrength(StrengthType::Referencing);
    relationClass->SetStrengthDirection(ECRelatedInstanceDirection::Forward);
    relationClass->SetClassModifier(ECClassModifier::Abstract);
    relationClass->GetSource().AddClass(*entityClassA);
    relationClass->GetTarget().AddClass(*entityClassB);

    ECEntityClassP baseClass;
    ecSchema->CreateEntityClass(baseClass, "InvalidBaseClass");

    EXPECT_NE(ECObjectsStatus::Success, relationClass->AddBaseClass(*baseClass)) << "An ECRelationshipClass can only have an ECRelationshipClass as a base class";

    ECRelationshipClassP relationClass2;
    ecSchema->CreateRelationshipClass(relationClass2, "ARelC");
    relationClass2->SetStrength(StrengthType::Referencing);
    relationClass2->SetStrengthDirection(ECRelatedInstanceDirection::Forward);
    relationClass2->SetClassModifier(ECClassModifier::Abstract);
    relationClass2->GetSource().AddClass(*entityClassA);
    relationClass2->GetTarget().AddClass(*entityClassC);

    EXPECT_NE(ECObjectsStatus::Success, relationClass2->AddBaseClass(*relationClass)) << "An ECRelationshipClass class constraint have to be narrowing.";

    entityClassC->AddBaseClass(*entityClassB);

    EXPECT_EQ(ECObjectsStatus::Success, relationClass2->AddBaseClass(*relationClass)) << "Relationship Class ARelB should now be a valid base class for ARelC";

    ECEntityClassP entityClassD;
    ECEntityClassP entityClassE;
    ecSchema->CreateEntityClass(entityClassD, "D");
    ecSchema->CreateEntityClass(entityClassE, "E");
    entityClassD->AddBaseClass(*entityClassA);
    entityClassE->AddBaseClass(*entityClassB);
    ECRelationshipClassP baseRelationClass;
    ecSchema->CreateRelationshipClass(baseRelationClass, "BaseClass");
    baseRelationClass->SetStrength(StrengthType::Referencing);
    baseRelationClass->SetStrengthDirection(ECRelatedInstanceDirection::Forward);
    baseRelationClass->SetClassModifier(ECClassModifier::Abstract);
    baseRelationClass->GetSource().AddClass(*entityClassD);
    baseRelationClass->GetTarget().AddClass(*entityClassE);

    EXPECT_NE(ECObjectsStatus::Success, relationClass2->AddBaseClass(*baseRelationClass)) << "An ECRelationshipClass can only have one base class";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    10/2016
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(ECRelationshipTests, TestAbstractConstraint)
    {
    ECSchemaPtr schemaPtr;
    ECSchema::CreateSchema(schemaPtr, "TestSchema", "ts", 1, 0, 0);

    ECSchemaP ecSchema = schemaPtr.get();

    ECEntityClassP entityClassA;
    ECEntityClassP entityClassB;
    ECEntityClassP entityClassC;
    ecSchema->CreateEntityClass(entityClassA, "A");
    ecSchema->CreateEntityClass(entityClassB, "B");
    ecSchema->CreateEntityClass(entityClassC, "C");

    ECRelationshipClassP relationClass;
    ecSchema->CreateRelationshipClass(relationClass, "ARelB");
    relationClass->SetStrength(StrengthType::Referencing);
    relationClass->SetStrengthDirection(ECRelatedInstanceDirection::Forward);
    relationClass->SetClassModifier(ECClassModifier::Abstract);
    relationClass->GetSource().AddClass(*entityClassA);
    relationClass->GetTarget().AddClass(*entityClassC);

    EXPECT_FALSE(relationClass->GetSource().IsAbstractConstraintDefinedLocally()) << "The Source Constraint's Abstract Constraint should not be locally defined since there is only one .";
    EXPECT_TRUE(relationClass->GetSource().IsAbstractConstraintDefined()) << "The Source Constraint's Abstract Constraint is implicity set therefore should return true.";
    EXPECT_STREQ("A", relationClass->GetSource().GetAbstractConstraint()->GetName().c_str());
    EXPECT_FALSE(relationClass->GetTarget().IsAbstractConstraintDefinedLocally()) << "The Target Constraint's Abstract Constraint should be implicity set therefore not locally defined.";
    EXPECT_TRUE(relationClass->GetTarget().IsAbstractConstraintDefined()) << "The Target Constraint's Abstract Constraint should be implicity set since therefore should return true.";
    EXPECT_STREQ("C", relationClass->GetTarget().GetAbstractConstraint()->GetName().c_str());

    // WIP: Commnent out until ECDb implements abstractConstraint because we avoid this check.
    //EXPECT_EQ(ECObjectsStatus::RelationshipConstraintsNotCompatible, relationClass->GetTarget().AddClass(*entityClassB)) << "Should fail to add the second constaint class because the abstract constraint has not been explicity set.";
    EXPECT_EQ(ECObjectsStatus::RelationshipConstraintsNotCompatible, relationClass->GetTarget().SetAbstractConstraint(*entityClassB)) << "The abstract constraint cannot be set to B because C is not nor derived from B.";
    entityClassC->AddBaseClass(*entityClassB); // Making C derive from B
    EXPECT_EQ(ECObjectsStatus::Success, relationClass->GetTarget().SetAbstractConstraint(*entityClassB)) << "The abstract constraint can now be set because B is a base class of C";

    EXPECT_TRUE(relationClass->GetTarget().IsAbstractConstraintDefinedLocally()) << "The Target Constraint's Abstract Constraint has now been explicity set therefore is locally defined.";
    EXPECT_TRUE(relationClass->GetTarget().IsAbstractConstraintDefined()) << "The Target Constraint's Abstract Constraint is locally set therefore should return true.";
    EXPECT_STREQ("B", relationClass->GetTarget().GetAbstractConstraint()->GetName().c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    10/2016
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(ECRelationshipTests, TestInheritedAbstractConstraint)
    {
    ECSchemaPtr schemaPtr;
    ECSchema::CreateSchema(schemaPtr, "TestSchema", "ts", 1, 0, 0);

    ECSchemaP ecSchema = schemaPtr.get();

    ECEntityClassP entityClassA;
    ECEntityClassP entityClassB;
    ECEntityClassP entityClassC;
    ECEntityClassP entityClassD;
    ECEntityClassP commonBaseClass;
    ECEntityClassP unrelatedClass;
    ecSchema->CreateEntityClass(unrelatedClass, "UnrelatedClass");
    ecSchema->CreateEntityClass(commonBaseClass, "BaseClass");
    ecSchema->CreateEntityClass(entityClassA, "A");
    
    ecSchema->CreateEntityClass(entityClassB, "B");
    entityClassB->AddBaseClass(*commonBaseClass);

    ecSchema->CreateEntityClass(entityClassC, "C");
    entityClassC->AddBaseClass(*commonBaseClass);
    entityClassC->AddBaseClass(*entityClassB);
    
    ecSchema->CreateEntityClass(entityClassD, "D");
    entityClassD->AddBaseClass(*commonBaseClass);
    entityClassD->AddBaseClass(*entityClassC);

    ECRelationshipClassP relationClass;
    ecSchema->CreateRelationshipClass(relationClass, "BaseRelationship");
    relationClass->SetStrength(StrengthType::Referencing);
    relationClass->SetStrengthDirection(ECRelatedInstanceDirection::Forward);
    relationClass->SetClassModifier(ECClassModifier::Abstract);
    relationClass->GetSource().AddClass(*entityClassA);
    relationClass->GetTarget().SetAbstractConstraint(*commonBaseClass);
    relationClass->GetTarget().AddClass(*entityClassB);

    ECRelationshipClassP relationClass2;
    ecSchema->CreateRelationshipClass(relationClass2, "DerivedRelationship");
    relationClass2->SetStrength(StrengthType::Referencing);
    relationClass2->SetStrengthDirection(ECRelatedInstanceDirection::Forward);
    relationClass2->SetClassModifier(ECClassModifier::Abstract);

    EXPECT_EQ(ECObjectsStatus::Success, relationClass2->AddBaseClass(*relationClass)) << "Adding a base class should succeed since there are no constraint classes on the derived class.";
    
    EXPECT_TRUE(relationClass2->GetSource().IsAbstractConstraintDefined()) << "The abstract constraint should be inherited from the base relationship.";
    EXPECT_FALSE(relationClass2->GetSource().IsAbstractConstraintDefinedLocally()) << "The abstract constraint should be inherited from the base relationship, therefore not defined locally.";
    EXPECT_STREQ("A", relationClass2->GetSource().GetAbstractConstraint()->GetName().c_str()) << "The abstract constraint should still be inherited from the base relationship even though there is only one constraint class.";
    
    EXPECT_TRUE(relationClass2->GetTarget().IsAbstractConstraintDefined()) << "The abstract constraint should be inherited from the base relationship.";
    EXPECT_FALSE(relationClass2->GetTarget().IsAbstractConstraintDefinedLocally()) << "The abstract constraint should be inherited from the base relationship, therefore not defined locally.";
    EXPECT_STREQ("BaseClass", relationClass2->GetTarget().GetAbstractConstraint()->GetName().c_str()) << "The abstract constraint should still be inherited from the base relationship even though there is only one constraint class.";

    ECRelationshipClassP relationClass3;
    ecSchema->CreateRelationshipClass(relationClass3, "DerivedRelationship2");
    relationClass3->SetStrength(StrengthType::Referencing);
    relationClass3->SetStrengthDirection(ECRelatedInstanceDirection::Forward);
    relationClass3->SetClassModifier(ECClassModifier::Abstract);
    
    EXPECT_EQ(ECObjectsStatus::Success, relationClass3->AddBaseClass(*relationClass2)) << "Adding a base class should succeed since there are no constraint classes on the derived class.";
    EXPECT_TRUE(relationClass3->GetSource().IsAbstractConstraintDefined()) << "The abstract constraint should be inherited from the base relationship.";
    EXPECT_FALSE(relationClass3->GetSource().IsAbstractConstraintDefinedLocally()) << "The abstract constraint should be inherited from the base relationship, therefore not defined locally.";
    EXPECT_STREQ("A", relationClass3->GetSource().GetAbstractConstraint()->GetName().c_str()) << "The abstract constraint should still be inherited from the base relationship even though there is only one constraint class.";
    EXPECT_TRUE(relationClass3->GetTarget().IsAbstractConstraintDefined()) << "The abstract constraint should be inherited from the base relationship.";
    EXPECT_FALSE(relationClass3->GetTarget().IsAbstractConstraintDefinedLocally()) << "The abstract constraint should be inherited from the base relationship, therefore not defined locally.";
    EXPECT_STREQ("BaseClass", relationClass3->GetTarget().GetAbstractConstraint()->GetName().c_str()) << "The abstract constraint should still be inherited from the base relationship even though there is only one constraint class.";

    relationClass3->GetSource().AddClass(*entityClassC);
    EXPECT_TRUE(relationClass3->GetSource().IsAbstractConstraintDefined()) << "The abstract constraint should still be defined.";
    EXPECT_STREQ("A", relationClass3->GetSource().GetAbstractConstraint()->GetName().c_str()) << "The abstract constraint should still be inherited from the base relationship even though there is only one constraint class.";

    relationClass3->GetTarget().AddClass(*entityClassC);
    EXPECT_EQ(1, relationClass3->GetTarget().GetClasses().size()) << "The number of constraint classes should increase to 1 when EntityClassC.";;
    relationClass3->GetTarget().AddClass(*entityClassD);
    EXPECT_EQ(2, relationClass3->GetTarget().GetClasses().size()) << "The number of constraint classes should increase to 2 when EntityClassD.";

    relationClass3->GetTarget().AddClass(*unrelatedClass);
    EXPECT_EQ(2, relationClass3->GetTarget().GetClasses().size()) << "The UnrelatedClass should not be added to the constraint since it violates the abstract constraint.";
    }

END_BENTLEY_ECN_TEST_NAMESPACE