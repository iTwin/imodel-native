/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"
#include "regex"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

struct ECRelationshipTestFixture : ECTestFixture 
{
public:
    static Utf8CP StrengthToString(StrengthType strength)
        {
        switch (strength)
            {
            case StrengthType::Referencing:
                return "referencing";
            case StrengthType::Holding:
                return "holding";
            case StrengthType::Embedding:
                return "embedding";
            default:
                return "";
            }
        }

    static Utf8CP StrengthDirectionToString (ECRelatedInstanceDirection direction)
        {
        switch (direction)
            {
            case ECRelatedInstanceDirection::Forward:
                return "forward";
            case ECRelatedInstanceDirection::Backward:
                return "backward";
            default:
                return "";
            }
        }
};

struct ECRelationshipInstanceTest : ECRelationshipTestFixture
    {
    ECSchemaPtr m_schema;

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    JoshSchifter    12/09
    +---------------+---------------+---------------+---------------+---------------+------*/
    static Utf8String GetTestSchemaXMLString ()
        {
        Utf8CP fmt = R"xml(<?xml version="1.0" encoding="UTF-8"?>
            <ECSchema schemaName="RelationshipTesting" nameSpacePrefix="test" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
                <ECClass typeName="ClassA" displayLabel="Class A" isDomainClass="True">
                    <ECProperty propertyName="p" typeName="int" />
                </ECClass>
                <ECClass typeName="ClassB" displayLabel="Class B" isDomainClass="True">
                    <ECProperty propertyName="p" typeName="int" />
                    <ECProperty propertyName="b" typeName="int" />
                </ECClass>
                <ECRelationshipClass typeName="ALikesB" displayLabel="A likes B" strength="referencing">
                    <Source cardinality="(1,1)" roleLabel="likes" polymorphic="False">
                        <Class class="ClassA" />
                    </Source>
                    <Target cardinality="(1,1)" roleLabel="is liked by" polymorphic="True">
                        <Class class="ClassB" />
                    </Target>
                    <ECProperty propertyName="p" typeName="int" />
                    <ECProperty propertyName="Name" typeName="string" />
                    <ECArrayProperty propertyName="ArrayProperty" typeName="string" minOccurs="0" maxOccurs="unbounded"/>
                    <ECProperty propertyName="SourceOrderId" typeName="int" />
                    <ECProperty propertyName="TargetOrderId" typeName="int" />
                </ECRelationshipClass>
            </ECSchema>)xml";

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

struct ECRelationshipClassTest : ECRelationshipTestFixture {};

struct ECRelationshipDeserializationTest : ECRelationshipTestFixture {};

//************************************************************************************
// ECRelationshipInstanceTests
//************************************************************************************
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECRelationshipInstanceTest, SimpleRelationship)
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
TEST_F (ECRelationshipInstanceTest, SetRelationshipProperties)
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
TEST_F (ECRelationshipInstanceTest, InstanceSettersAndGetters)
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
TEST_F (ECRelationshipInstanceTest, InheritedEnablerIterator)
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
TEST_F (ECRelationshipInstanceTest, InheritedEnablerIndices)
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
TEST_F (ECRelationshipInstanceTest, InheritedSetGetValues)
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
TEST_F (ECRelationshipInstanceTest, InheritedSetGetArrayValues)
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
TEST_F (ECRelationshipInstanceTest, SourceDestOrderIDs)
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
TEST_F (ECRelationshipInstanceTest, DumpToString)
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
        "^~!@ECClass=ALikesB at address = 0x[a-f0-9]{2,16}$",
        "^~!@  \\[0x[a-f0-9]{2,16}\\]\\[   0\\] Nullflags\\[0\\] = 0x[a-f0-9]{1,8}$",
        "^~!@  \\[0x[a-f0-9]{2,16}\\]\\[   4\\] p = 42$",
        "^~!@  \\[0x[a-f0-9]{2,16}\\]\\[   8\\] SourceOrderId = <null>$",
        "^~!@  \\[0x[a-f0-9]{2,16}\\]\\[  12\\] TargetOrderId = <null>$",
        "^~!@  \\[0x[a-f0-9]{2,16}\\]\\[  16\\] -> \\[0x[a-f0-9]{2,8}\\]\\[  36\\] Name = Some value 1$",
        "^~!@  \\[0x[a-f0-9]{2,16}\\]\\[  20\\] -> \\[0x[a-f0-9]{2,8}\\]\\[  49\\] ArrayProperty = Count: 0 IsFixedSize: 0$",
        "^~!@  \\[0x[a-f0-9]{2,16}\\]\\[  24\\] -> \\[0x[a-f0-9]{2,8}\\]\\[    \\] Source ECPointer = <null>$",
        "^~!@  \\[0x[a-f0-9]{2,16}\\]\\[  28\\] -> \\[0x[a-f0-9]{2,8}\\]\\[    \\] Target ECPointer = <null>$",
        "^~!@  \\[0x[a-f0-9]{2,16}\\]\\[  32\\] Offset of TheEnd = 49$", };

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

//************************************************************************************
// ECRelationshipClassTest
//************************************************************************************
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stefan.Apfel               03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECRelationshipClassTest, TestsRelationshipStrengthAndDirectionConstraints)
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
* @bsimethod                                    Stefan.Apfel                 03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECRelationshipClassTest, TestsRelationshipConstraints)
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
    relationClass->GetSource().SetRoleLabel("Source");
    relationClass->GetTarget().SetRoleLabel("Target");

    EXPECT_FALSE(relationClass->Verify()) << "There are no constraint classes defined on the constraints so it should fail to verify.";

    ECRelationshipClassP relationClassBase;
    ecSchema->CreateRelationshipClass(relationClassBase, "RelBaseClass");
    relationClassBase->SetStrength(StrengthType::Referencing);
    relationClassBase->SetStrengthDirection(ECRelatedInstanceDirection::Forward);
    relationClassBase->GetSource().SetRoleLabel("Source");
    relationClassBase->GetTarget().SetRoleLabel("Target");

    relationClassBase->GetSource().AddClass(*classB);
    
    EXPECT_EQ(ECObjectsStatus::Success, relationClass->AddBaseClass(*relationClassBase));

    EXPECT_EQ(ECObjectsStatus::Success, relationClass->GetSource().AddClass(*classB)) << "ClassB is the constraint class so it should work ";
    EXPECT_EQ(ECObjectsStatus::Success, relationClass->GetSource().SetAbstractConstraint(*classB)) << "ClassB is the constraint class so adding it as the abstract constraint should work.";
    EXPECT_EQ(ECObjectsStatus::Success, relationClass->GetSource().AddClass(*classC)) << "ClassC is deriving from ClassA so it should work too";
    EXPECT_EQ(ECObjectsStatus::RelationshipConstraintsNotCompatible, relationClass->GetSource().AddClass(*classA)) << "ClassA is the base class of ClassB but violates the base constraints(expects ClassB or bigger)";

    relationClass->GetTarget().AddClass(*classA);

    EXPECT_FALSE(relationClassBase->Verify()) << "The base relationship class should not verify because it does not define both constraints.";
    EXPECT_FALSE(relationClass->Verify()) << "The derived relationship class should not verify because the base relationship does not define both constraints.";

    EXPECT_EQ(ECObjectsStatus::Success, relationClassBase->GetTarget().AddClass(*classA)) << "Should have been able to add a target constraint to the base relationship class";
    EXPECT_TRUE(relationClassBase->Verify()) << "The base relationship class should  verify because it defines both constraints.";
    EXPECT_TRUE(relationClass->Verify()) << "The derived relationship class should not verify because the base relationship defines both constraints and they are compatible.";

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Caleb.Shafer                  08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ECRelationshipClassTest, TestRelationshipMultiplicityConstraint)
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
* @bsimethod                                    Caleb.Shafer                10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECRelationshipClassTest, TestBaseClassRules)
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

    ECRelationshipClassP baseRelationClass;
    ecSchema->CreateRelationshipClass(baseRelationClass, "BaseClass");
    baseRelationClass->SetStrength(StrengthType::Referencing);
    baseRelationClass->SetStrengthDirection(ECRelatedInstanceDirection::Forward);
    baseRelationClass->SetClassModifier(ECClassModifier::Abstract);
    baseRelationClass->GetSource().AddClass(*entityClassA);
    baseRelationClass->GetTarget().AddClass(*entityClassB);

    EXPECT_EQ(ECObjectsStatus::RelationshipAlreadyHasBaseClass, relationClass2->AddBaseClass(*baseRelationClass)) << "An ECRelationshipClass can only have one base class";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    10/2016
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(ECRelationshipClassTest, TestAbstractConstraint_Entity)
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
    relationClass->GetSource().SetRoleLabel("ARelB");
    relationClass->GetTarget().AddClass(*entityClassC);
    relationClass->GetTarget().SetRoleLabel("ARelB (Reversed)");

    EXPECT_FALSE(relationClass->GetSource().IsAbstractConstraintDefined()) << "The Source Constraint's Abstract Constraint is implicitly set therefore should be false.";
    EXPECT_STREQ("A", relationClass->GetSource().GetAbstractConstraint()->GetName().c_str()) << "The abstract constraint should be implicitly set to the only constraint class.";
    EXPECT_FALSE(relationClass->GetTarget().IsAbstractConstraintDefined()) << "The Target Constraint's Abstract Constraint should be implicitly set since therefore should be false.";
    EXPECT_STREQ("C", relationClass->GetTarget().GetAbstractConstraint()->GetName().c_str()) << "The abstract constraint should be implicitly set to the only constraint class.";

    EXPECT_EQ(ECObjectsStatus::RelationshipConstraintsNotCompatible, relationClass->GetTarget().AddClass(*entityClassB)) << "Should fail to add the second constaint class because the abstract constraint has not been explicity set.";
    EXPECT_EQ(ECObjectsStatus::RelationshipConstraintsNotCompatible, relationClass->GetTarget().SetAbstractConstraint(*entityClassB)) << "The abstract constraint cannot be set to B because C is not nor derived from B.";
    entityClassC->AddBaseClass(*entityClassB); // Making C derive from B
    EXPECT_EQ(ECObjectsStatus::Success, relationClass->GetTarget().SetAbstractConstraint(*entityClassB)) << "The abstract constraint can now be set because B is a base class of C";

    EXPECT_TRUE(relationClass->GetTarget().IsAbstractConstraintDefined()) << "The Target Constraint's Abstract Constraint is locally set therefore should return true.";
    EXPECT_STREQ("B", relationClass->GetTarget().GetAbstractConstraint()->GetName().c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    06/2017
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(ECRelationshipClassTest, TestAbstractConstraint_Relationships)
    {
    ECSchemaPtr schemaPtr;
    ECSchema::CreateSchema(schemaPtr, "TestSchema", "ts", 1, 0, 0);

    ECSchemaP ecSchema = schemaPtr.get();

    ECEntityClassP entityClassA;
    ECEntityClassP entityClassB;
    ecSchema->CreateEntityClass(entityClassA, "A");
    ecSchema->CreateEntityClass(entityClassB, "B");

    ECRelationshipClassP relClassA;
    ECRelationshipClassP relClassB;
    ECRelationshipClassP relClassC;
    ecSchema->CreateRelationshipClass(relClassA, "RelA", *entityClassA, "Source", *entityClassB, "Target");
    ecSchema->CreateRelationshipClass(relClassB, "RelB", *entityClassA, "Source", *entityClassB, "Target");
    ecSchema->CreateRelationshipClass(relClassC, "RelC", *entityClassA, "Source", *entityClassB, "Target");

    ECRelationshipClassP relationClass;
    ecSchema->CreateRelationshipClass(relationClass, "RelARelatesToRelB");
    relationClass->SetStrength(StrengthType::Referencing);
    relationClass->SetStrengthDirection(ECRelatedInstanceDirection::Forward);
    relationClass->SetClassModifier(ECClassModifier::Abstract);
    relationClass->GetSource().AddClass(*relClassA);
    relationClass->GetSource().SetRoleLabel("ARelB");
    relationClass->GetTarget().AddClass(*relClassC);
    relationClass->GetTarget().SetRoleLabel("ARelB (Reversed)");

    EXPECT_FALSE(relationClass->GetSource().IsAbstractConstraintDefined()) << "The Source Constraint's Abstract Constraint is implicitly set therefore should be false.";
    EXPECT_STREQ("RelA", relationClass->GetSource().GetAbstractConstraint()->GetName().c_str()) << "The abstract constraint should be implicitly set to the only constraint class.";
    EXPECT_FALSE(relationClass->GetTarget().IsAbstractConstraintDefined()) << "The Target Constraint's Abstract Constraint should be implicitly set since therefore should be false.";
    EXPECT_STREQ("RelC", relationClass->GetTarget().GetAbstractConstraint()->GetName().c_str()) << "The abstract constraint should be implicitly set to the only constraint class.";

    EXPECT_EQ(ECObjectsStatus::RelationshipConstraintsNotCompatible, relationClass->GetTarget().AddClass(*relClassB)) << "Should fail to add the second constaint class because the abstract constraint has not been explicity set.";
    EXPECT_EQ(ECObjectsStatus::RelationshipConstraintsNotCompatible, relationClass->GetTarget().SetAbstractConstraint(*relClassB)) << "The abstract constraint cannot be set to B because C is not nor derived from B.";
    relClassC->AddBaseClass(*relClassB); // Making C derive from B
    EXPECT_EQ(ECObjectsStatus::Success, relationClass->GetTarget().SetAbstractConstraint(*relClassB)) << "The abstract constraint can now be set because B is a base class of C";

    EXPECT_TRUE(relationClass->GetTarget().IsAbstractConstraintDefined()) << "The Target Constraint's Abstract Constraint is locally set therefore should return true.";
    EXPECT_STREQ("RelB", relationClass->GetTarget().GetAbstractConstraint()->GetName().c_str());

    EXPECT_TRUE(ecSchema->Validate());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    10/2016
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(ECRelationshipClassTest, TestRelationshipDelayedValidation)
    {
    ECSchemaPtr schemaPtr;
    ECSchema::CreateSchema(schemaPtr, "TestSchema", "ts", 1, 0, 0);

    ECSchemaP ecSchema = schemaPtr.get();

    ECEntityClassP entityClassA;
    ECEntityClassP entityClassB;
    ecSchema->CreateEntityClass(entityClassA, "A");
    ecSchema->CreateEntityClass(entityClassB, "B");

    ECRelationshipClassP baseRelationClass;
    ecSchema->CreateRelationshipClass(baseRelationClass, "baseRelClass", false);
    ASSERT_FALSE(baseRelationClass->GetIsVerified()) << "A newly created relationship should not be verified.";
    ASSERT_FALSE(ecSchema->Validate()) << "The schema fail to validate since the relationship is invalid.";
    
    baseRelationClass->SetStrength(StrengthType::Referencing);
    baseRelationClass->SetStrengthDirection(ECRelatedInstanceDirection::Forward);
    baseRelationClass->SetClassModifier(ECClassModifier::Abstract);
    ASSERT_FALSE(baseRelationClass->Verify()) << "The relationship with invalid constraints should fail to verify.";
    ASSERT_FALSE(baseRelationClass->GetIsVerified()) << "A relationship with invalid Source and Target constraints should not be verified.";

    baseRelationClass->GetSource().SetRoleLabel("Source");
    baseRelationClass->GetTarget().SetRoleLabel("Target");
    ASSERT_FALSE(baseRelationClass->Verify());
    ASSERT_FALSE(baseRelationClass->GetIsVerified()) << "Now that the Source and Target constraints are fully defined the class should verify.";

    baseRelationClass->GetSource().AddClass(*entityClassA);
    baseRelationClass->GetTarget().AddClass(*entityClassB);
    ASSERT_TRUE(baseRelationClass->Verify());
    ASSERT_TRUE(baseRelationClass->GetIsVerified()) << "Now that the Source and Target constraints are fully defined the class should verify.";
    ASSERT_TRUE(ecSchema->Validate()) << "The schema should now validate since the relationship is now verified.";
    ASSERT_TRUE(ecSchema->IsECVersion(ECVersion::Latest)) << "The schema should now be an EC" << ECSchema::GetECVersionString(ECVersion::Latest) << " schema.";

    ECRelationshipClassP relationClass;
    ecSchema->CreateRelationshipClass(relationClass, "relClass", false);
    EXPECT_EQ(ECObjectsStatus::Success, relationClass->AddBaseClass(*baseRelationClass));
    relationClass->GetSource().SetRoleLabel("Source");
    relationClass->GetTarget().SetRoleLabel("Target");
    relationClass->GetSource().AddClass(*entityClassA);
    relationClass->GetTarget().AddClass(*entityClassB);
    EXPECT_TRUE(relationClass->Verify()) << "Most attributes that are required on a relationship can be inherited so when the base class is added it should be valid.";
    EXPECT_TRUE(relationClass->GetIsVerified());

    relationClass->RemoveBaseClass(*baseRelationClass);
    EXPECT_FALSE(relationClass->GetIsVerified()) << "The base class which made this relationship valid has been removed. The relationship should now not be valid";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    06/2017
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(ECRelationshipClassTest, TestRelationshipAsEndpoint)
    {
    ECSchemaPtr schemaPtr;
    ECSchema::CreateSchema(schemaPtr, "TestSchema", "ts", 1, 0, 0);

    ECSchemaP ecSchema = schemaPtr.get();

    ECEntityClassP entityClassA;
    ECEntityClassP entityClassB;
    ECEntityClassP entityClassC;
    ECRelationshipClassP relClassAToB;
    ECRelationshipClassP relClassCRelAToB;

    ecSchema->CreateEntityClass(entityClassA, "A");
    ecSchema->CreateEntityClass(entityClassB, "B");
    ecSchema->CreateEntityClass(entityClassC, "C");

    ecSchema->CreateRelationshipClass(relClassAToB, "AToB");
    relClassAToB->GetSource().AddClass(*entityClassA);
    relClassAToB->GetSource().SetRoleLabel("Source");
    relClassAToB->GetTarget().AddClass(*entityClassB);
    relClassAToB->GetTarget().SetRoleLabel("Target");

    ecSchema->CreateRelationshipClass(relClassCRelAToB, "CRelAToB");
    relClassCRelAToB->GetSource().AddClass(*entityClassC);
    relClassCRelAToB->GetSource().SetRoleLabel("Source");
    EXPECT_EQ(ECObjectsStatus::Success, relClassCRelAToB->GetTarget().AddClass(*relClassAToB));
    relClassCRelAToB->GetTarget().SetRoleLabel("Target");

    EXPECT_TRUE(ecSchema->Validate());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    06/2017
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(ECRelationshipClassTest, TestEntityAndRelationshipCannotBeOnTheSameEndpoint)
    {
    ECSchemaPtr schemaPtr;
    ECSchema::CreateSchema(schemaPtr, "TestSchema", "ts", 1, 0, 0);

    ECSchemaP ecSchema = schemaPtr.get();

    ECEntityClassP entityClassA;
    ECEntityClassP entityClassB;
    ECEntityClassP entityClassC;
    ECRelationshipClassP relClassAToB;
    ECRelationshipClassP relClassCRelAToB;

    ecSchema->CreateEntityClass(entityClassA, "A");
    ecSchema->CreateEntityClass(entityClassB, "B");
    ecSchema->CreateEntityClass(entityClassC, "C");

    ecSchema->CreateRelationshipClass(relClassAToB, "AToB");
    relClassAToB->GetSource().AddClass(*entityClassA);
    relClassAToB->GetTarget().AddClass(*entityClassB);

    ecSchema->CreateRelationshipClass(relClassCRelAToB, "CRelAToB");
    relClassCRelAToB->GetSource().AddClass(*entityClassC);
    EXPECT_EQ(ECObjectsStatus::Success, relClassCRelAToB->GetTarget().AddClass(*relClassAToB));
    EXPECT_EQ(ECObjectsStatus::Success, relClassCRelAToB->GetTarget().SetAbstractConstraint(*relClassAToB));
    EXPECT_EQ(ECObjectsStatus::RelationshipConstraintsNotCompatible, relClassCRelAToB->GetTarget().AddClass(*entityClassB));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    10/2016
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(ECRelationshipClassTest, TestRelationshipSerialization)
    {
    ECSchemaPtr schemaPtr;
    ECSchema::CreateSchema(schemaPtr, "TestSchema", "ts", 1, 0, 0);

    ECSchemaP ecSchema = schemaPtr.get();

    ECRelationshipClassP baseRelationClass;
    ecSchema->CreateRelationshipClass(baseRelationClass, "baseRelClass", false);
    baseRelationClass->SetStrength(StrengthType::Referencing);
    baseRelationClass->SetStrengthDirection(ECRelatedInstanceDirection::Forward);
    baseRelationClass->SetClassModifier(ECClassModifier::Abstract);
    EXPECT_FALSE(baseRelationClass->Verify()) << "The relationship with invalid constraints should fail to verify.";
    EXPECT_FALSE(baseRelationClass->GetIsVerified()) << "A relationship with invalid Source and Target constraints should not be verified.";

    Utf8String serializedSchemaXml;
    EXPECT_EQ(SchemaWriteStatus::Success, ecSchema->WriteToXmlString(serializedSchemaXml)) << "Even though the schema is invalid because of a bad relationship it should serialize.";

    ECSchemaPtr roundTripSchema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    EXPECT_EQ(SchemaReadStatus::InvalidECSchemaXml, ECSchema::ReadFromXmlString(roundTripSchema, serializedSchemaXml.c_str(), *context)) << "Schema should fail deserialization because it is an invalid 3.1 schema.";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                           Victor.Cushman                          11/2017
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECRelationshipClassTest, SerializeStandaloneRelationshipClass)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "ExampleSchema", "ex", 3, 1, 0, ECVersion::Latest);

    ECCustomAttributeClassP customAttrClass;
    schema->CreateCustomAttributeClass(customAttrClass, "ExampleCustomAttribute");
    IECInstancePtr customAttr = customAttrClass->GetDefaultStandaloneEnabler()->CreateInstance();

    ECEntityClassP sourceClass;
    schema->CreateEntityClass(sourceClass, "ExampleSource");

    ECEntityClassP targetClass;
    schema->CreateEntityClass(targetClass, "ExampleTarget");

    ECRelationshipClassP relationshipClass;
    schema->CreateRelationshipClass(relationshipClass, "ExampleRelationship");
    relationshipClass->SetClassModifier(ECClassModifier::Sealed);
    relationshipClass->SetStrength(StrengthType::Embedding);
    relationshipClass->SetStrengthDirection(ECRelatedInstanceDirection::Forward);
    relationshipClass->GetSource().AddClass(*sourceClass);
    relationshipClass->GetSource().SetRoleLabel("source roleLabel");
    relationshipClass->GetSource().SetIsPolymorphic(true);
    relationshipClass->GetSource().SetMultiplicity(RelationshipMultiplicity::ZeroOne());
    relationshipClass->GetSource().SetCustomAttribute(*customAttr);
    relationshipClass->GetTarget().AddClass(*targetClass);
    relationshipClass->GetTarget().SetRoleLabel("target roleLabel");
    relationshipClass->GetSource().SetIsPolymorphic(true);
    relationshipClass->GetTarget().SetMultiplicity(RelationshipMultiplicity::ZeroOne());

    Json::Value schemaJson;
    EXPECT_TRUE(relationshipClass->ToJson(schemaJson, true));

    Json::Value testDataJson;
    BeFileName testDataFile(ECTestFixture::GetTestDataPath(L"ECJson/StandaloneECRelationshipClass.ecschema.json"));
    auto readJsonStatus = ECTestUtility::ReadJsonInputFromFile(testDataJson, testDataFile);
    ASSERT_EQ(BentleyStatus::SUCCESS, readJsonStatus);

    EXPECT_TRUE(ECTestUtility::JsonDeepEqual(schemaJson, testDataJson)) << ECTestUtility::JsonSchemasComparisonString(schemaJson, testDataJson);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                           Victor.Cushman                          11/2017
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECRelationshipClassTest, SerializeStandaloneRelationshipClassWithAbstractConstraint)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "ExampleSchema", "ex", 3, 1, 0, ECVersion::Latest);

    ECEntityClassP classA;
    schema->CreateEntityClass(classA, "ClassA");
    ECEntityClassP classB;
    schema->CreateEntityClass(classB, "ClassB");
    classB->AddBaseClass(*classA);
    ECEntityClassP classC;
    schema->CreateEntityClass(classC, "ClassC");
    classC->AddBaseClass(*classB);

    ECRelationshipClassP relationshipClassBase;
    schema->CreateRelationshipClass(relationshipClassBase, "RelationshipClassBase");
    relationshipClassBase->SetStrength(StrengthType::Referencing);
    relationshipClassBase->SetStrengthDirection(ECRelatedInstanceDirection::Forward);
    relationshipClassBase->GetSource().SetRoleLabel("SourceRoleLabel");
    relationshipClassBase->GetSource().AddClass(*classB);
    relationshipClassBase->GetTarget().SetRoleLabel("TargetRoleLabel");
    relationshipClassBase->GetTarget().AddClass(*classA);

    ECRelationshipClassP relationshipClass;
    schema->CreateRelationshipClass(relationshipClass, "ExampleRelationshipClass");
    relationshipClass->SetStrength(StrengthType::Referencing);
    relationshipClass->SetStrengthDirection(ECRelatedInstanceDirection::Forward);
    relationshipClass->AddBaseClass(*relationshipClassBase);
    relationshipClass->GetSource().SetRoleLabel("SourceRoleLabel");
    relationshipClass->GetSource().AddClass(*classB);
    relationshipClass->GetSource().SetAbstractConstraint(*classB);
    relationshipClass->GetSource().AddClass(*classC);
    relationshipClass->GetTarget().SetRoleLabel("TargetRoleLabel");
    relationshipClass->GetTarget().AddClass(*classA);

    Json::Value schemaJson;
    EXPECT_TRUE(relationshipClass->ToJson(schemaJson, true));

    Json::Value testDataJson;
    BeFileName testDataFile(ECTestFixture::GetTestDataPath(L"ECJson/StandaloneECRelationshipClassWithAbstractConstraint.ecschema.json"));
    auto readJsonStatus = ECTestUtility::ReadJsonInputFromFile(testDataJson, testDataFile);
    ASSERT_EQ(BentleyStatus::SUCCESS, readJsonStatus);

    EXPECT_TRUE(ECTestUtility::JsonDeepEqual(schemaJson, testDataJson)) << ECTestUtility::JsonSchemasComparisonString(schemaJson, testDataJson);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                           Victor.Cushman                          11/2017
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECRelationshipClassTest, InheritedConstraintCustomAttributesShouldNotBeSerialized)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "ExampleSchema", "ex", 3, 1, 0, ECVersion::Latest);

    ECEntityClassP sourceClass;
    schema->CreateEntityClass(sourceClass, "ExampleSource");

    ECEntityClassP targetClass;
    schema->CreateEntityClass(targetClass, "ExampleTarget");

    ECCustomAttributeClassP customAttrClassRelCon;
    schema->CreateCustomAttributeClass(customAttrClassRelCon, "CustomAttributeOnRelationshipConstraint");
    IECInstancePtr customAttrRelCon = customAttrClassRelCon->GetDefaultStandaloneEnabler()->CreateInstance();

    ECRelationshipClassP baseRelationshipClass;
    schema->CreateRelationshipClass(baseRelationshipClass, "BaseRelationshipClass");
    baseRelationshipClass->GetSource().AddClass(*sourceClass);
    baseRelationshipClass->GetSource().SetCustomAttribute(*customAttrRelCon);
    baseRelationshipClass->GetTarget().AddClass(*targetClass);

    ECRelationshipClassP derivedRelationshipClass;
    schema->CreateRelationshipClass(derivedRelationshipClass, "DerivedRelationshipClass");
    derivedRelationshipClass->AddBaseClass(*baseRelationshipClass);
    derivedRelationshipClass->SetStrength(StrengthType::Referencing);
    derivedRelationshipClass->SetStrengthDirection(ECRelatedInstanceDirection::Forward);
    derivedRelationshipClass->GetSource().AddClass(*sourceClass);
    derivedRelationshipClass->GetSource().SetRoleLabel("SourceRoleLabel");
    derivedRelationshipClass->GetTarget().AddClass(*targetClass);
    derivedRelationshipClass->GetTarget().SetRoleLabel("TargetRoleLabel");

    Json::Value relationshipClassJson;
    EXPECT_TRUE(derivedRelationshipClass->ToJson(relationshipClassJson, true));

    Json::Value testDataJson;
    BeFileName relClassTestDataFile(ECTestFixture::GetTestDataPath(L"ECJson/RelationshipConstraintInheritedCustomAttributes.ecschema.json"));
    auto readJsonStatus = ECTestUtility::ReadJsonInputFromFile(testDataJson, relClassTestDataFile);
    ASSERT_EQ(BentleyStatus::SUCCESS, readJsonStatus);
    EXPECT_TRUE(ECTestUtility::JsonDeepEqual(relationshipClassJson, testDataJson)) << ECTestUtility::JsonSchemasComparisonString(relationshipClassJson, testDataJson);
    }

//************************************************************************************
// ECRelationshipDeserializationTest
//************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    06/2017
//---------------+---------------+---------------+---------------+---------------+-------
void TestRelationshipStrengthDirectionConstraint(ECRelatedInstanceDirection baseDirection, ECRelatedInstanceDirection derivedDirection, bool expectSuccess = true)
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECEntityClass typeName="Parent"/>
            <ECEntityClass typeName="Child"/>
            <ECRelationshipClass typeName="RelBase" modifier="Abstract" strength="Referencing" strengthDirection="%s">
                <Source multiplicity="(0..1)" polymorphic="True" roleLabel="has">
                    <Class class="Parent"/>
                </Source>
                <Target multiplicity="(0..1)" polymorphic="True" roleLabel="is owned by">
                    <Class class="Child"/>
                </Target>
            </ECRelationshipClass>
            <ECRelationshipClass typeName="RelSub" modifier="None" strength="Referencing" strengthDirection="%s">
                <BaseClass>RelBase</BaseClass>
                <Source multiplicity="(0..1)" polymorphic="True" roleLabel="has">
                    <Class class="Parent"/>
                </Source>
                <Target multiplicity="(0..1)" polymorphic="True" roleLabel="is owned by">
                    <Class class="Child"/>
                </Target>
            </ECRelationshipClass>
        </ECSchema>
        )xml";

    Utf8CP baseStrengthDirectionString = ECRelationshipTestFixture::StrengthDirectionToString(baseDirection);
    Utf8CP derivedStrengthDirectionString = ECRelationshipTestFixture::StrengthDirectionToString(derivedDirection);

    Utf8String formattedSchemaXml;
    formattedSchemaXml.Sprintf(schemaXml, baseStrengthDirectionString, derivedStrengthDirectionString);

    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();

    SchemaReadStatus expectedOutcome = expectSuccess ? SchemaReadStatus::Success : SchemaReadStatus::InvalidECSchemaXml;
    ASSERT_EQ(expectedOutcome, ECSchema::ReadFromXmlString(schema, formattedSchemaXml.c_str(), *context)) <<
        "The schema has a base relationship with strength direction " << baseStrengthDirectionString << " and the derived relationship with strength direction " << derivedStrengthDirectionString << " which should " << (expectSuccess ? "successfully" : "fail") << " to deserialize.";

    if (!expectSuccess)
        return;

    ECClassCP ecRelBaseClass = schema->GetClassCP("RelBase");
    ASSERT_TRUE(nullptr != ecRelBaseClass);
    ECRelationshipClassCP relBaseClass = ecRelBaseClass->GetRelationshipClassCP();
    ASSERT_TRUE(nullptr != relBaseClass);
    EXPECT_EQ(baseDirection, relBaseClass->GetStrengthDirection());

    ECClassCP ecRelSubClass = schema->GetClassCP("RelSub");
    ASSERT_TRUE(nullptr != ecRelSubClass);
    ECRelationshipClassCP relSubClass = ecRelSubClass->GetRelationshipClassCP();
    ASSERT_TRUE(nullptr != relSubClass);
    EXPECT_EQ(derivedDirection, relSubClass->GetStrengthDirection());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    06/2017
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(ECRelationshipDeserializationTest, TestRelationshipStrengthDirectionConstraint)
    {
    TestRelationshipStrengthDirectionConstraint(ECRelatedInstanceDirection::Forward, ECRelatedInstanceDirection::Forward);
    TestRelationshipStrengthDirectionConstraint(ECRelatedInstanceDirection::Forward, ECRelatedInstanceDirection::Backward, false);
    TestRelationshipStrengthDirectionConstraint(ECRelatedInstanceDirection::Backward, ECRelatedInstanceDirection::Forward, false);
    TestRelationshipStrengthDirectionConstraint(ECRelatedInstanceDirection::Backward, ECRelatedInstanceDirection::Backward);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    06/2017
//---------------+---------------+---------------+---------------+---------------+-------
void TestRelationshipStrengthConstraint(StrengthType baseStrength, StrengthType derivedStrength, bool expectSuccess = true)
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECEntityClass typeName="Parent"/>
            <ECEntityClass typeName="Child"/>
            <ECRelationshipClass typeName="RelBase" modifier="Abstract" strength="%s" strengthDirection="Forward">
                <Source multiplicity="(0..1)" polymorphic="True" roleLabel="has">
                    <Class class="Parent"/>
                </Source>
                <Target multiplicity="(0..1)" polymorphic="True" roleLabel="is owned by">
                    <Class class="Child"/>
                </Target>
            </ECRelationshipClass>
            <ECRelationshipClass typeName="RelSub" modifier="None" strength="%s" strengthDirection="Forward">
                <BaseClass>RelBase</BaseClass>
                <Source multiplicity="(0..1)" polymorphic="True" roleLabel="has">
                    <Class class="Parent"/>
                </Source>
                <Target multiplicity="(0..1)" polymorphic="True" roleLabel="is owned by">
                    <Class class="Child"/>
                </Target>
            </ECRelationshipClass>
        </ECSchema>
        )xml";

    Utf8CP baseStrengthString = ECRelationshipTestFixture::StrengthToString(baseStrength);
    Utf8CP derivedStrengthString = ECRelationshipTestFixture::StrengthToString(derivedStrength);

    Utf8String formattedSchemaXml;
    formattedSchemaXml.Sprintf(schemaXml, baseStrengthString, derivedStrengthString);

    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();

    SchemaReadStatus expectedOutcome = expectSuccess ? SchemaReadStatus::Success : SchemaReadStatus::InvalidECSchemaXml;
    ASSERT_EQ(expectedOutcome, ECSchema::ReadFromXmlString(schema, formattedSchemaXml.c_str(), *context)) <<
        "The schema has a base relationship with strength " << baseStrengthString << " and the derived relationship with strength " << derivedStrengthString << " which should " << (expectSuccess ? "successfully" : "fail") << " to deserialize.";

    if (!expectSuccess)
        return;

    ECClassCP ecRelBaseClass = schema->GetClassCP("RelBase");
    ASSERT_TRUE(nullptr != ecRelBaseClass);
    ECRelationshipClassCP relBaseClass = ecRelBaseClass->GetRelationshipClassCP();
    ASSERT_TRUE(nullptr != relBaseClass);
    EXPECT_EQ(baseStrength, relBaseClass->GetStrength());

    ECClassCP ecRelSubClass = schema->GetClassCP("RelSub");
    ASSERT_TRUE(nullptr != ecRelSubClass);
    ECRelationshipClassCP relSubClass = ecRelSubClass->GetRelationshipClassCP();
    ASSERT_TRUE(nullptr != relSubClass);
    EXPECT_EQ(derivedStrength, relSubClass->GetStrength());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    06/2017
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(ECRelationshipDeserializationTest, TestRelationshipStrengthConstraint)
    {
    TestRelationshipStrengthConstraint(StrengthType::Referencing, StrengthType::Referencing);
    TestRelationshipStrengthConstraint(StrengthType::Referencing, StrengthType::Holding, false);
    TestRelationshipStrengthConstraint(StrengthType::Referencing, StrengthType::Embedding, false);
    TestRelationshipStrengthConstraint(StrengthType::Holding, StrengthType::Referencing, false);
    TestRelationshipStrengthConstraint(StrengthType::Holding, StrengthType::Holding);
    TestRelationshipStrengthConstraint(StrengthType::Holding, StrengthType::Embedding, false);
    TestRelationshipStrengthConstraint(StrengthType::Embedding, StrengthType::Referencing, false);
    TestRelationshipStrengthConstraint(StrengthType::Embedding, StrengthType::Holding, false);
    TestRelationshipStrengthConstraint(StrengthType::Embedding, StrengthType::Embedding);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    09/2016
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(ECRelationshipDeserializationTest, TestValidAbstractConstraint)
    {
    {
    Utf8CP schemaXml = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='testSchema' version='01.00' alias='ts' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "   <ECEntityClass typeName='A' modifier='abstract'/>"
        "   <ECEntityClass typeName='B' modifier='abstract'>"
        "       <BaseClass>A</BaseClass>"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='C' modifier='abstract'>"
        "       <BaseClass>A</BaseClass>"
        "   </ECEntityClass>"
        "   <ECRelationshipClass typeName='ARelB' modifier='abstract'>"
        "       <Source multiplicity='(0..1)' polymorphic='True' roleLabel='testSource' >"
        "           <Class class='A' />"
        "       </Source>"
        "       <Target multiplicity='(0..1)' polymorphic='True' roleLabel='testTarget' abstractConstraint='A'>"
        "           <Class class='B' />"
        "           <Class class='C' />"
        "       </Target>"
        "   </ECRelationshipClass>"
        "</ECSchema>";
    ECSchemaPtr schema;
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXml, *schemaContext);
    ASSERT_EQ(SchemaReadStatus::Success, status);
    ASSERT_TRUE(schema.IsValid());
    
    EXPECT_STREQ("A", schema->GetClassCP("ARelB")->GetRelationshipClassCP()->GetSource().GetAbstractConstraint()->GetName().c_str()) << "The Source constraint's abstract constraint is implicitly defined to be A.";
    EXPECT_STREQ("A", schema->GetClassCP("ARelB")->GetRelationshipClassCP()->GetTarget().GetAbstractConstraint()->GetName().c_str()) << "The Target constraint's abstract constraint is explicitly defined to be B.";
    }
    {
    Utf8CP schemaXml = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='testSchema' version='01.00' alias='ts' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "   <ECEntityClass typeName='A' modifier='abstract'/>"
        "   <ECEntityClass typeName='B' modifier='abstract'/>"
        "   <ECRelationshipClass typeName='ARelB' modifier='abstract'>"
        "       <Source multiplicity='(0..1)' polymorphic='True' roleLabel='testSource' >"
        "           <Class class='A' />"
        "       </Source>"
        "       <Target multiplicity='(0..1)' polymorphic='True' roleLabel='testTarget' >"
        "           <Class class='B' />"
        "       </Target>"
        "   </ECRelationshipClass>"
        "</ECSchema>";
    ECSchemaPtr schema;
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXml, *schemaContext);
    ASSERT_EQ(SchemaReadStatus::Success, status);
    ASSERT_TRUE(schema.IsValid());
    
    EXPECT_STREQ("A", schema->GetClassCP("ARelB")->GetRelationshipClassCP()->GetSource().GetAbstractConstraint()->GetName().c_str()) << "The Source constraint's abstract constraint is implicitly defined to be A.";
    EXPECT_STREQ("B", schema->GetClassCP("ARelB")->GetRelationshipClassCP()->GetTarget().GetAbstractConstraint()->GetName().c_str()) << "The Target constraint's abstract constraint is implicitly defined to be B.";
    }
    {
    Utf8CP schemaXml = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='testSchema' version='01.00' alias='ts' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "   <ECEntityClass typeName='A' modifier='abstract'/>"
        "   <ECEntityClass typeName='B' modifier='abstract'/>"
        "   <ECEntityClass typeName='C' modifier='abstract'>"
        "       <BaseClass>B</BaseClass>"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='CB' modifier='abstract'>"
        "       <BaseClass>B</BaseClass>"
        "       <BaseClass>C</BaseClass>"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='AB' modifier='abstract'>"
        "       <BaseClass>A</BaseClass>"
        "       <BaseClass>B</BaseClass>"
        "   </ECEntityClass>"
        "   <ECRelationshipClass typeName='ARelB' modifier='abstract'>"
        "       <Source multiplicity='(0..1)' polymorphic='True' roleLabel='testSource' >"
        "           <Class class='A' />"
        "       </Source>"
        "       <Target multiplicity='(0..1)' polymorphic='True' roleLabel='testTarget' abstractConstraint='B'>"
        "           <Class class='B' />"
        "           <Class class='C' />"
        "           <Class class='CB' />"
        "           <Class class='AB' />"
        "       </Target>"
        "   </ECRelationshipClass>"
        "</ECSchema>";
    ECSchemaPtr schema;
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXml, *schemaContext);
    ASSERT_EQ(SchemaReadStatus::Success, status);
    ASSERT_TRUE(schema.IsValid());

    EXPECT_STREQ("A", schema->GetClassCP("ARelB")->GetRelationshipClassCP()->GetSource().GetAbstractConstraint()->GetName().c_str()) << "The Source constraint's abstract constraint is implicitly defined to be A.";
    EXPECT_STREQ("B", schema->GetClassCP("ARelB")->GetRelationshipClassCP()->GetTarget().GetAbstractConstraint()->GetName().c_str()) << "The Target constraint's abstract constraint should be explicitly defined to B.";
    }
    {
    Utf8CP schemaXml = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='testSchema' version='01.00' alias='ts' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "   <ECEntityClass typeName='A' modifier='abstract'/>"
        "   <ECEntityClass typeName='B' modifier='abstract'/>"
        "   <ECEntityClass typeName='C' modifier='abstract'>"
        "       <BaseClass>B</BaseClass>"
        "   </ECEntityClass>"
        "   <ECRelationshipClass typeName='ARelB' modifier='abstract'>"
        "       <Source multiplicity='(0..1)' polymorphic='True' roleLabel='testSource' >"
        "           <Class class='A' />"
        "       </Source>"
        "       <Target multiplicity='(0..1)' polymorphic='True' roleLabel='testTarget' abstractConstraint='B'>"
        "           <Class class='C' />"
        "       </Target>"
        "   </ECRelationshipClass>"
        "</ECSchema>";
    ECSchemaPtr schema;
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXml, *schemaContext);
    ASSERT_EQ(SchemaReadStatus::Success, status);
    ASSERT_TRUE(schema.IsValid());

    EXPECT_STREQ("A", schema->GetClassCP("ARelB")->GetRelationshipClassCP()->GetSource().GetAbstractConstraint()->GetName().c_str()) << "The Source constraint's abstract constraint is implicitly defined to be A.";
    EXPECT_STREQ("B", schema->GetClassCP("ARelB")->GetRelationshipClassCP()->GetTarget().GetAbstractConstraint()->GetName().c_str()) << "The Target constraint's abstract constraint is explicitly defined to B.";
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    09/2016
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(ECRelationshipDeserializationTest, ExpectFailureWhenAbstractConstraintNotFoundOrEmpty)
    {
    {
    Utf8CP schemaXml = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='testSchema' version='01.00' alias='ts' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "   <ECEntityClass typeName='A' modifier='abstract'/>"
        "   <ECEntityClass typeName='B' modifier='abstract'/>"
        "   <ECEntityClass typeName='C' modifier='abstract'>"
        "       <BaseClass>B</BaseClass>"
        "   </ECEntityClass>"
        "   <ECRelationshipClass typeName='ARelB' modifier='abstract'>"
        "       <Source multiplicity='(0..1)' polymorphic='True' roleLabel='testSource' >"
        "           <Class class='A' />"
        "       </Source>"
        "       <Target multiplicity='(0..1)' polymorphic='True' roleLabel='testTarget' >"
        "           <Class class='B' />"
        "           <Class class='C' />"
        "       </Target>"
        "   </ECRelationshipClass>"
        "</ECSchema>";
    ECSchemaPtr schema;
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXml, *schemaContext);
    ASSERT_EQ(SchemaReadStatus::InvalidECSchemaXml, status) << "The schema should fail to deserialize when an abstractConstraint attribute is not found and multiple constraint classes are defined.";
    }
    {
    Utf8CP schemaXml = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='testSchema' version='01.00' alias='ts' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "   <ECEntityClass typeName='A' modifier='abstract'/>"
        "   <ECEntityClass typeName='B' modifier='abstract'/>"
        "   <ECEntityClass typeName='C' modifier='abstract'>"
        "       <BaseClass>B</BaseClass>"
        "   </ECEntityClass>"
        "   <ECRelationshipClass typeName='ARelB' modifier='abstract'>"
        "       <Source multiplicity='(0..1)' polymorphic='True' roleLabel='testSource' abstractConstraint='' >"
        "           <Class class='A' />"
        "       </Source>"
        "       <Target multiplicity='(0..1)' polymorphic='True' roleLabel='testTarget' abstractConstraint='' >"
        "           <Class class='B' />"
        "           <Class class='C' />"
        "       </Target>"
        "   </ECRelationshipClass>"
        "</ECSchema>";
    ECSchemaPtr schema;
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXml, *schemaContext);
    ASSERT_EQ(SchemaReadStatus::InvalidECSchemaXml, status) << "The schema should fail to deserialize because the abstractConstraint is empty.";
    }
    {
    Utf8CP schemaXml = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='testSchema' version='01.00' alias='ts' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "   <ECEntityClass typeName='A' modifier='abstract'/>"
        "   <ECEntityClass typeName='B' modifier='abstract'/>"
        "   <ECEntityClass typeName='C' modifier='abstract'>"
        "       <BaseClass>B</BaseClass>"
        "   </ECEntityClass>"
        "   <ECRelationshipClass typeName='ARelB' modifier='abstract'>"
        "       <Source multiplicity='(0..1)' polymorphic='True' roleLabel='testSource' abstractConstraint='D' >"
        "           <Class class='A' />"
        "       </Source>"
        "       <Target multiplicity='(0..1)' polymorphic='True' roleLabel='testTarget' abstractConstraint='E' >"
        "           <Class class='B' />"
        "           <Class class='C' />"
        "       </Target>"
        "   </ECRelationshipClass>"
        "</ECSchema>";
    ECSchemaPtr schema;
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXml, *schemaContext);
    ASSERT_EQ(SchemaReadStatus::InvalidECSchemaXml, status) << "The schema should fail to deserialize because the abstractConstraint class do not exist.";
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    09/2016
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(ECRelationshipDeserializationTest, ExpectFailureWhenAbstractConstraintViolatesNarrowing)
    {
    Utf8CP schemaXml = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='testSchema' version='01.00' alias='ts' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "   <ECEntityClass typeName='A' modifier='abstract'/>"
        "   <ECEntityClass typeName='B' modifier='abstract'/>"
        "   <ECEntityClass typeName='C' modifier='abstract'>"
        "       <BaseClass>B</BaseClass>"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='D' modifier='sealed'>"
        "       <BaseClass>C</BaseClass>"
        "   </ECEntityClass>"
        "   <ECRelationshipClass typeName='DerivedARelD' modifier='abstract'>"
        "       <BaseClass>ARelD</BaseClass>"
        "       <Source multiplicity='(0..1)' polymorphic='True' roleLabel='testSource' >"
        "           <Class class='A' />"
        "       </Source>"
        "       <Target multiplicity='(0..1)' polymorphic='True' roleLabel='testTarget' abstractConstraint='B' >"
        "           <Class class='D' />"
        "       </Target>"
        "   </ECRelationshipClass>"
        "   <ECRelationshipClass typeName='ARelD' modifier='abstract'>"
        "       <Source multiplicity='(0..1)' polymorphic='True' roleLabel='testSource' abstractConstraint='A'>"
        "           <Class class='A' />"
        "       </Source>"
        "       <Target multiplicity='(0..1)' polymorphic='True' roleLabel='testTarget' abstractConstraint='C' >"
        "           <Class class='D' />"
        "       </Target>"
        "   </ECRelationshipClass>"
        "</ECSchema>";
    ECSchemaPtr schema;
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXml, *schemaContext);
    ASSERT_EQ(SchemaReadStatus::InvalidECSchemaXml, status) << "The schema should fail because DerivedARelD has a more base class as the target abstract constraint then it's base class ARelD.";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    11/2016
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(ECRelationshipDeserializationTest, ExpectFailureWhenRelationshipConstraintsEmpty)
    {
    Utf8CP schemaXml = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='testSchema' version='01.00' alias='ts' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "   <ECEntityClass typeName='A' modifier='abstract'/>"
        "   <ECEntityClass typeName='B' modifier='abstract'/>"
        "   <ECRelationshipClass typeName='ARelD' modifier='abstract'>"
        "       <Source multiplicity='(0..1)' polymorphic='True' roleLabel='testSource' abstractConstraint='A'>"
        "       </Source>"
        "       <Target multiplicity='(0..1)' polymorphic='True' roleLabel='testTarget' abstractConstraint='B' >"
        "       </Target>"
        "   </ECRelationshipClass>"
        "</ECSchema>";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXml, *schemaContext);
    ASSERT_NE(SchemaReadStatus::Success, status) << "The schema should fail because DerivedARelD has a more base class as the target abstract constraint then it's base class ARelD.";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    12/2016
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(ECRelationshipDeserializationTest, TestMultiplicityValidation)
    {
    Utf8CP schemaXml = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='testSchema' version='01.00' alias='ts' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "   <ECEntityClass typeName='A' modifier='abstract'/>"
        "   <ECEntityClass typeName='B' modifier='abstract'/>"
        "   <ECEntityClass typeName='C' modifier='abstract'>"
        "       <BaseClass>B</BaseClass>"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='D' modifier='abstract'>"
        "       <BaseClass>C</BaseClass>"
        "   </ECEntityClass>"
        "   <ECRelationshipClass typeName='BRelD' modifier='sealed'>"
        "       <BaseClass>ARelB</BaseClass>"
        "       <Source multiplicity='(1..1)' polymorphic='True' roleLabel='testSource' abstractConstraint='A'>"
        "           <Class class='A' />"
        "       </Source>"
        "       <Target multiplicity='(0..*)' polymorphic='True' roleLabel='testTarget' abstractConstraint='B' >"
        "           <Class class='D' />"
        "       </Target>"
        "   </ECRelationshipClass>"
        "   <ECRelationshipClass typeName='ARelB' modifier='abstract'>"
        "       <Source multiplicity='(1..1)' polymorphic='True' roleLabel='testSource' abstractConstraint='A'>"
        "           <Class class='A' />"
        "       </Source>"
        "       <Target multiplicity='(0..*)' polymorphic='True' roleLabel='testTarget' abstractConstraint='B' >"
        "           <Class class='C' />"
        "       </Target>"
        "   </ECRelationshipClass>"
        "</ECSchema>";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXml, *context);
    ASSERT_EQ(SchemaReadStatus::Success, status);

    ECClassCP ecClass = schema->GetClassCP("ARelB");
    ASSERT_TRUE(nullptr != ecClass);
    ECRelationshipClassCP relClass = ecClass->GetRelationshipClassCP();
    ASSERT_TRUE(nullptr != relClass);

    ECClassCP derivedClass = schema->GetClassCP("BRelD");
    ASSERT_TRUE(nullptr != derivedClass);
    ECRelationshipClassCP derivedRelClass = ecClass->GetRelationshipClassCP();
    ASSERT_TRUE(nullptr != derivedRelClass);

    EXPECT_EQ(0, RelationshipMultiplicity::Compare(relClass->GetSource().GetMultiplicity(), RelationshipMultiplicity::OneOne()));
    EXPECT_EQ(0, RelationshipMultiplicity::Compare(relClass->GetTarget().GetMultiplicity(), RelationshipMultiplicity::ZeroMany()));
    EXPECT_EQ(0, RelationshipMultiplicity::Compare(derivedRelClass->GetSource().GetMultiplicity(), RelationshipMultiplicity::OneOne()));
    EXPECT_EQ(0, RelationshipMultiplicity::Compare(derivedRelClass->GetTarget().GetMultiplicity(), RelationshipMultiplicity::ZeroMany()));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    12/2016
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(ECRelationshipDeserializationTest, TestFailureWhenRelationshipClassModifierNotFound)
    {
    {
    Utf8CP schemaXml = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='testSchema' version='01.00' alias='ts' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "   <ECEntityClass typeName='A'/>"
        "   <ECEntityClass typeName='B'/>"
        "   <ECRelationshipClass typeName='ARelB' >"
        "       <Source multiplicity='(1..1)' polymorphic='True' roleLabel='testSource'>"
        "           <Class class='A' />"
        "       </Source>"
        "       <Target multiplicity='(0..*)' polymorphic='True' roleLabel='testTarget'>"
        "           <Class class='B' />"
        "       </Target>"
        "   </ECRelationshipClass>"
        "</ECSchema>";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXml, *context);
    ASSERT_EQ(SchemaReadStatus::InvalidECSchemaXml, status) << "The schema did not fail to deserialize even though the relationship class is missing the modifier attribute.";
    }
    {
    Utf8CP schemaXml = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='testSchema' version='01.00' alias='ts' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "   <ECEntityClass typeName='A'/>"
        "   <ECEntityClass typeName='B'/>"
        "   <ECRelationshipClass typeName='ARelB' modifier=''>"
        "       <Source multiplicity='(1..1)' polymorphic='True' roleLabel='testSource'>"
        "           <Class class='A' />"
        "       </Source>"
        "       <Target multiplicity='(0..*)' polymorphic='True' roleLabel='testTarget'>"
        "           <Class class='B' />"
        "       </Target>"
        "   </ECRelationshipClass>"
        "</ECSchema>";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXml, *context);
    ASSERT_EQ(SchemaReadStatus::InvalidECSchemaXml, status) << "The schema did not fail to deserialize even though the relationship class modifier attribute is empty.";
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    12/2016
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(ECRelationshipDeserializationTest, TestFailureWithRelationshipKeys)
    {
    {
    Utf8CP schemaXml = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='testSchema' version='01.00' alias='ts' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "   <ECEntityClass typeName='A'/>"
        "   <ECEntityClass typeName='B'/>"
        "   <ECRelationshipClass typeName='ARelB' modifier='Sealed'>"
        "       <Source multiplicity='(1..1)' polymorphic='True' roleLabel='testSource'>"
        "           <Class class='A'>"
        "               <Key>"
        "                   <Property name='Property' />"
        "               </Key>"
        "           </Class>"
        "       </Source>"
        "       <Target multiplicity='(0..*)' polymorphic='True' roleLabel='testTarget'>"
        "           <Class class='B' />"
        "       </Target>"
        "   </ECRelationshipClass>"
        "</ECSchema>";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXml, *context);
    ASSERT_EQ(SchemaReadStatus::InvalidECSchemaXml, status) << "The schema did not fail to deserialize even though the source constraint of the relationship class has a key property.";
    }
    {
    Utf8CP schemaXml = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='testSchema' version='01.00' alias='ts' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "   <ECEntityClass typeName='A'/>"
        "   <ECEntityClass typeName='B'/>"
        "   <ECRelationshipClass typeName='ARelB' modifier='Sealed'>"
        "       <Source multiplicity='(1..1)' polymorphic='True' roleLabel='testSource'>"
        "           <Class class='A'/>"
        "       </Source>"
        "       <Target multiplicity='(0..*)' polymorphic='True' roleLabel='testTarget'>"
        "           <Class class='B' >"
        "               <Key>"
        "                   <Property name='Property' />"
        "               </Key>"
        "           </Class>"
        "       </Target>"
        "   </ECRelationshipClass>"
        "</ECSchema>";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXml, *context);
    ASSERT_EQ(SchemaReadStatus::InvalidECSchemaXml, status) << "The schema did not fail to deserialize even though the target constraint of the relationship class has a key property.";
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    12/2016
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(ECRelationshipDeserializationTest, TestSuccessWithRelationshipKeysFromEC2)
    {
    // This test is only to make sure de-serialization does not fail when 2.0/3.0 schemas have key properties.
    // Even though they are allowed to have them they are dropped and there is no way to access
    // them via the API
    {
    Utf8CP schemaXml = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='testSchema' version='01.00' nameSpacePrefix='ts' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECEntityClass typeName='A'/>"
        "   <ECEntityClass typeName='B'/>"
        "   <ECRelationshipClass typeName='ARelB' >"
        "       <Source multiplicity='(1..1)' polymorphic='True' roleLabel='testSource'>"
        "           <Class class='A'>"
        "               <Key>"
        "                   <Property name='Property' />"
        "               </Key>"
        "           </Class>"
        "       </Source>"
        "       <Target multiplicity='(0..*)' polymorphic='True' roleLabel='testTarget'>"
        "           <Class class='B' />"
        "       </Target>"
        "   </ECRelationshipClass>"
        "</ECSchema>";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXml, *context);
    ASSERT_EQ(SchemaReadStatus::Success, status) << "The 3.0 schema failed even though relationship classes can have key properties.";
    }
    {
    Utf8CP schemaXml = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='testSchema' version='01.00' nameSpacePrefix='ts' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "   <ECClass typeName='A' isDomainClass='True' />"
        "   <ECClass typeName='B' isDomainClass='True' />"
        "   <ECRelationshipClass typeName='ARelB' isDomainClass='True' >"
        "       <Source multiplicity='(1..1)' polymorphic='True' roleLabel='testSource'>"
        "           <Class class='A'>"
        "               <Key>"
        "                   <Property name='Property' />"
        "               </Key>"
        "           </Class>"
        "       </Source>"
        "       <Target multiplicity='(0..*)' polymorphic='True' roleLabel='testTarget'>"
        "           <Class class='B' />"
        "       </Target>"
        "   </ECRelationshipClass>"
        "</ECSchema>";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXml, *context);
    ASSERT_EQ(SchemaReadStatus::Success, status) << "The 2.0 schema failed even though relationship classes can have key properties.";
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    09/2016
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(ECRelationshipDeserializationTest, ExpectFailureWhenMultiplicityNotFoundOrEmpty)
    {
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();

    Utf8CP schemaXml = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='testSchema' version='01.00' alias='ts' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "   <ECEntityClass typeName='A' modifier='abstract'/>"
        "   <ECEntityClass typeName='B' modifier='abstract'/>"
        "   <ECRelationshipClass typeName='ARelB' modifier='abstract'>"
        "       <Source polymorphic='True' roleLabel='test'>"
        "           <Class class='A' />"
        "       </Source>"
        "       <Target polymorphic='True' roleLabel='test'>"
        "           <Class class='B' />"
        "       </Target>"
        "   </ECRelationshipClass>"
        "</ECSchema>";

    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXml, *schemaContext);
    ASSERT_EQ(SchemaReadStatus::InvalidECSchemaXml, status) << "ECRelationshipConstraint without a mutliplicity attribute is supposed to fail to deserialize.";

    Utf8CP schemaXml2 = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='testSchema' version='01.00' alias='ts' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "   <ECEntityClass typeName='A' modifier='abstract'/>"
        "   <ECEntityClass typeName='B' modifier='abstract'/>"
        "   <ECRelationshipClass typeName='ARelB' modifier='abstract'>"
        "       <Source multiplicity='' polymorphic='True' roleLabel='test'>"
        "           <Class class='A' />"
        "       </Source>"
        "       <Target multiplicity='' polymorphic='True' roleLabel='test'>"
        "           <Class class='B' />"
        "       </Target>"
        "   </ECRelationshipClass>"
        "</ECSchema>";

    ECSchemaPtr schema2;
    status = ECSchema::ReadFromXmlString(schema2, schemaXml2, *schemaContext);
    ASSERT_EQ(SchemaReadStatus::InvalidECSchemaXml, status) << "ECRelationshipConstraint with an empty mutliplicity attribute is supposed to fail to deserialize.";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    09/2016
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(ECRelationshipDeserializationTest, ExpectFailureWhenPolymorphicNotFoundOrEmpty)
    {
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();

    Utf8CP schemaXml = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='testSchema' version='01.00' alias='ts' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "   <ECEntityClass typeName='A' modifier='abstract'/>"
        "   <ECEntityClass typeName='B' modifier='abstract'/>"
        "   <ECRelationshipClass typeName='ARelB' modifier='abstract'>"
        "       <Source multiplicity='(0..1)' roleLabel='test'>"
        "           <Class class='A' />"
        "       </Source>"
        "       <Target multiplicity='(0..1)' roleLabel='test'>"
        "           <Class class='B' />"
        "       </Target>"
        "   </ECRelationshipClass>"
        "</ECSchema>";

    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXml, *schemaContext);
    ASSERT_EQ(SchemaReadStatus::InvalidECSchemaXml, status) << "ECRelationshipConstraint without a polymorphic attribute is supposed to fail to deserialize.";

    Utf8CP schemaXml2 = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='testSchema' version='01.00' alias='ts' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "   <ECEntityClass typeName='A' modifier='abstract'/>"
        "   <ECEntityClass typeName='B' modifier='abstract'/>"
        "   <ECRelationshipClass typeName='ARelB' modifier='abstract'>"
        "       <Source multiplicity='(0..1)' polymorphic='' roleLabel='test'>"
        "           <Class class='A' />"
        "       </Source>"
        "       <Target multiplicity='(0..1)' polymorphic='' roleLabel='test'>"
        "           <Class class='B' />"
        "       </Target>"
        "   </ECRelationshipClass>"
        "</ECSchema>";

    ECSchemaPtr schema2;
    status = ECSchema::ReadFromXmlString(schema2, schemaXml2, *schemaContext);
    ASSERT_EQ(SchemaReadStatus::InvalidECSchemaXml, status) << "ECRelationshipConstraint with an empty polymorphic attribute is supposed to fail to deserialize.";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    09/2016
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(ECRelationshipDeserializationTest, ExpectFailureWhenRoleLabelNotFoundOrEmpty)
    {
    {
    Utf8CP schemaXml = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='testSchema' version='01.00' alias='ts' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "   <ECEntityClass typeName='A' modifier='abstract'/>"
        "   <ECEntityClass typeName='B' modifier='abstract'/>"
        "   <ECRelationshipClass typeName='ARelB' modifier='abstract'>"
        "       <Source multiplicity='(0..1)' polymorphic='True' >"
        "           <Class class='A' />"
        "       </Source>"
        "       <Target multiplicity='(0..1)' polymorphic='True' >"
        "           <Class class='B' />"
        "       </Target>"
        "   </ECRelationshipClass>"
        "</ECSchema>";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXml, *schemaContext);
    ASSERT_EQ(SchemaReadStatus::InvalidECSchemaXml, status) << "ECRelationshipConstraint without a roleLabel attribute is supposed to fail to deserialize.";
    }

    {
    Utf8CP schemaXml = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='testSchema' version='01.00' alias='ts' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "   <ECEntityClass typeName='A' modifier='abstract'/>"
        "   <ECEntityClass typeName='B' modifier='abstract'/>"
        "   <ECRelationshipClass typeName='ARelB' modifier='abstract'>"
        "       <Source multiplicity='(0..1)' polymorphic='True' roleLabel='' >"
        "           <Class class='A' />"
        "       </Source>"
        "       <Target multiplicity='(0..1)' polymorphic='True' roleLabel='' >"
        "           <Class class='B' />"
        "       </Target>"
        "   </ECRelationshipClass>"
        "</ECSchema>";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXml, *schemaContext);
    ASSERT_EQ(SchemaReadStatus::InvalidECSchemaXml, status) << "ECRelationshipConstraint with an empty roleLabel attribute is supposed to fail to deserialize.";
    }
    {
    Utf8CP schemaXml = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='testSchema' version='01.00' alias='ts' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "   <ECEntityClass typeName='A' modifier='abstract'/>"
        "   <ECEntityClass typeName='B' modifier='abstract'/>"
        "   <ECEntityClass typeName='C' modifier='abstract'>"
        "       <BaseClass>B</BaseClass>"
        "   </ECEntityClass>"
        "   <ECRelationshipClass typeName='ARelB' modifier='abstract'>"
        "       <Source multiplicity='(0..1)' polymorphic='True' roleLabel='' >"
        "           <Class class='A' />"
        "       </Source>"
        "       <Target multiplicity='(0..1)' polymorphic='True' roleLabel='' >"
        "           <Class class='B' />"
        "       </Target>"
        "   </ECRelationshipClass>"
        "   <ECRelationshipClass typeName='ARelC' modifier='abstract'>"
        "       <BaseClass>ARelC</BaseClass>"
        "       <Source multiplicity='(0..1)' polymorphic='True' >"
        "           <Class class='A' />"
        "       </Source>"
        "       <Target multiplicity='(0..1)' polymorphic='True' >"
        "           <Class class='C' />"
        "       </Target>"
        "   </ECRelationshipClass>"
        "</ECSchema>";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXml, *schemaContext);
    ASSERT_EQ(SchemaReadStatus::InvalidECSchemaXml, status) << "ECRelationshipConstraint with an empty roleLabel attribute is supposed to fail to deserialize.";
    }
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle   06/15
//---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECRelationshipDeserializationTest, TestMultipleConstraintClasses)
    {
    Utf8CP schemaXml = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0' schemaName='ReferencedSchema' nameSpacePrefix='ref' version='01.00' description='Description' displayLabel='Display Label' xmlns:ec='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "  <ECClass typeName = 'Class' isDomainClass = 'True'>"
        "      <ECProperty propertyName = 'Property' typeName = 'string' />"
        "  </ECClass>"
        "  <ECClass typeName = 'Class1' isDomainClass = 'True'>"
        "      <ECProperty propertyName = 'Property1' typeName = 'string' />"
        "      <ECProperty propertyName = 'Property2' typeName = 'string' />"
        "  </ECClass>"
        "  <ECClass typeName = 'Class2' isDomainClass = 'True'>"
        "      <ECProperty propertyName = 'Property3' typeName = 'string' />"
        "      <ECProperty propertyName = 'Property4' typeName = 'string' />"
        "  </ECClass>"
        "  <ECRelationshipClass typeName = 'ClassHasClass1Or2' isDomainClass = 'True' strength = 'referencing' strengthDirection = 'forward'>"
        "      <Source cardinality = '(0, 1)' polymorphic = 'True'>"
        "          <Class class = 'Class' />"
        "      </Source>"
        "      <Target cardinality = '(0, 1)' polymorphic = 'True'>"
        "          <Class class = 'Class1' />"
        "          <Class class = 'Class2' />"
        "      </Target>"
        "  </ECRelationshipClass>"
        "</ECSchema>";

    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    ECSchemaPtr ecSchema = nullptr;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(ecSchema, schemaXml, *schemaContext));
    ECRelationshipClassCP relClass = ecSchema->GetClassCP("ClassHasClass1Or2")->GetRelationshipClassCP();
    ASSERT_TRUE(relClass != nullptr);
    ASSERT_EQ(1, relClass->GetSource().GetConstraintClasses().size());
    ASSERT_STREQ("Class", relClass->GetSource().GetConstraintClasses()[0]->GetName().c_str());

    ASSERT_EQ(2, relClass->GetTarget().GetConstraintClasses().size());
    ASSERT_STREQ("Class1", relClass->GetTarget().GetConstraintClasses()[0]->GetName().c_str());
    ASSERT_STREQ("Class2", relClass->GetTarget().GetConstraintClasses()[1]->GetName().c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    08/2016
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(ECRelationshipDeserializationTest, ExpectSuccessWhenDerivedClassComesBeforeBaseClass)
    {
    Utf8CP schemaXml = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='testSchema' version='01.00' alias='ts' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "   <ECEntityClass typeName='C' modifier='sealed'>"
        "       <BaseClass>B</BaseClass>"
        "   </ECEntityClass>"
        "   <ECEntityClass typeName='A' modifier='abstract'></ECEntityClass>"
        "   <ECEntityClass typeName='B' modifier='abstract'></ECEntityClass>"
        "   <ECRelationshipClass typeName='ARelC' modifier='sealed'>"
        "       <BaseClass>ARelB</BaseClass>"
        "       <Source multiplicity='(1..1)' polymorphic='True' roleLabel='testSource' >"
        "           <Class class='A' />"
        "       </Source>"
        "       <Target multiplicity='(1..*)' polymorphic='True' roleLabel='testTarget' >"
        "           <Class class='C' />"
        "       </Target>"
        "   </ECRelationshipClass>"
        "   <ECRelationshipClass typeName='ARelB' modifier='abstract'>"
        "       <Source multiplicity='(1..1)' polymorphic='True' roleLabel='testSource' >"
        "           <Class class='A' />"
        "       </Source>"
        "       <Target multiplicity='(0..*)' polymorphic='True' roleLabel='testTarget'>"
        "           <Class class='B' />"
        "       </Target>"
        "   </ECRelationshipClass>"
        "</ECSchema>";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXml, *schemaContext);
    ASSERT_EQ(SchemaReadStatus::Success, status);
    ASSERT_TRUE(schema.IsValid());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    08/2016
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(ECRelationshipDeserializationTest, RelationshipClassAsEndpoint)
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECEntityClass typeName="A"/>
            <ECEntityClass typeName="B"/>
            <ECEntityClass typeName="C"/>
            <ECRelationshipClass typeName="AToB" strength="Referencing" strengthDirection="forward" modifier="None">
                <Source multiplicity="(0..1)" polymorphic="True" roleLabel="Source">
                    <Class class="A"/>
                </Source>
                <Target multiplicity="(0..1)" polymorphic="True" roleLabel="Target">
                    <Class class="B"/>
                </Target>
            </ECRelationshipClass>
            <ECRelationshipClass typeName="CRelAToB" strength="Referencing" strengthDirection="forward" modifier="None">
                <Source multiplicity="(0..1)" polymorphic="True" roleLabel="Source">
                    <Class class="C"/>
                </Source>
                <Target multiplicity="(0..1)" polymorphic="True" roleLabel="Target">
                    <Class class="AToB"/>
                </Target>
            </ECRelationshipClass>
        </ECSchema>)xml";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr   schemaContext = ECSchemaReadContext::CreateContext();
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXml, *schemaContext);
    ASSERT_EQ(SchemaReadStatus::Success, status);
    ASSERT_TRUE(schema.IsValid());

    ECClassCP ecClass = schema->GetClassCP("CRelAToB");
    ASSERT_NE(nullptr, ecClass);

    ECRelationshipClassCP relClass = ecClass->GetRelationshipClassCP();
    ASSERT_NE(nullptr, relClass);

    ASSERT_EQ(1, relClass->GetTarget().GetConstraintClasses().size());
    ECClassCP relatedClass = relClass->GetTarget().GetConstraintClasses()[0];
    ASSERT_NE(nullptr, relatedClass);
    ECRelationshipClassCP relatedRelationship = relatedClass->GetRelationshipClassCP();
    ASSERT_NE(nullptr, relatedRelationship);
    }

END_BENTLEY_ECN_TEST_NAMESPACE