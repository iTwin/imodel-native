/*--------------------------------------------------------------------------------------+
|
|     $Source: test/Published/SchemaDeserializationConversionTests.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"
#include "BeXml/BeXml.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

struct SchemaDeserializationConversionTest : ECTestFixture 
    {
    void TestRoleLabelAttribute(ECVersion ecVersion, bool useEmptyString);
    void TestInheritedRoleLabelAttribute(ECVersion ecVersion, bool noRoleLabel, Utf8CP roleLabel = "");

    void ValidateRoundTripLatestSerialization(ECSchemaPtr schema)
        {
        Utf8String schemaString;
        SchemaWriteStatus writeStatus = schema->WriteToXmlString(schemaString);
        ASSERT_EQ(SchemaWriteStatus::Success, writeStatus) << "Failed to serialize schema";

        ECSchemaPtr deserializedSchema;
        ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
        SchemaReadStatus readStatus = ECSchema::ReadFromXmlString(deserializedSchema, schemaString.c_str(), *schemaContext);
        ASSERT_EQ(SchemaReadStatus::Success, readStatus) << "Failed to deserialize schema";
        ASSERT_TRUE(deserializedSchema->IsECVersion(ECVersion::Latest));
        }
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    08/2016
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaDeserializationConversionTest, TestMultiplicityConstraint)
    {
    Utf8CP schemaXml = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='testSchema' version='01.00' nameSpacePrefix='ts' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "   <ECClass typeName='A' isDomainClass='true'></ECClass>"
        "   <ECClass typeName='B' isDomainClass='true'></ECClass>"
        "   <ECClass typeName='C' isDomainClass='true'>"
        "       <BaseClass>B</BaseClass>"
        "   </ECClass>"
        "   <ECRelationshipClass typeName='ARelB' isDomainClass='false' strength='referencing' strengthDirection='forward'>"
        "       <Source cardinality='(1,1)' polymorphic='True'>"
        "           <Class class='A' />"
        "       </Source>"
        "       <Target cardinality='(1,1)' polymorphic='True'>"
        "           <Class class='B' />"
        "       </Target>"
        "   </ECRelationshipClass>"
        "   <ECRelationshipClass typeName='ARelC' isDomainClass='false' strength='referencing' strengthDirection='forward'>"
        "       <BaseClass>ARelB</BaseClass>"
        "       <Source cardinality='(0,1)' polymorphic='True'>"
        "           <Class class='A' />"
        "       </Source>"
        "       <Target cardinality='(0,N)' polymorphic='True'>"
        "           <Class class='C' />"
        "       </Target>"
        "   </ECRelationshipClass>"
        "</ECSchema>";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXml, *schemaContext);
    EXPECT_EQ(SchemaReadStatus::Success, status);
    EXPECT_TRUE(schema.IsValid());
    ASSERT_TRUE(schema->IsECVersion(ECVersion::Latest));

    EXPECT_EQ(0, schema->GetClassCP("ARelB")->GetRelationshipClassCP()->GetSource().GetMultiplicity().GetLowerLimit());
    EXPECT_EQ(1, schema->GetClassCP("ARelB")->GetRelationshipClassCP()->GetSource().GetMultiplicity().GetUpperLimit());
    EXPECT_EQ(0, schema->GetClassCP("ARelB")->GetRelationshipClassCP()->GetTarget().GetMultiplicity().GetLowerLimit());
    EXPECT_EQ(INT_MAX, schema->GetClassCP("ARelB")->GetRelationshipClassCP()->GetTarget().GetMultiplicity().GetUpperLimit());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    08/2016
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaDeserializationConversionTest, ExpectSuccessWithViolatedMultiplicityConstraint)
    {
    Utf8CP schemaXml = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='testSchema' version='01.00' nameSpacePrefix='ts' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECEntityClass typeName='A' modifier='abstract'></ECEntityClass>"
        "   <ECEntityClass typeName='B' modifier='abstract'></ECEntityClass>"
        "   <ECEntityClass typeName='C' modifier='abstract'>"
        "       <BaseClass>B</BaseClass>"
        "   </ECEntityClass>"
        "   <ECRelationshipClass typeName='ARelB' strength='referencing' strengthDirection='forward' modifier='abstract'>"
        "       <Source cardinality='(1,1)' polymorphic='True' roleLabel='source'>"
        "           <Class class='A' />"
        "       </Source>"
        "       <Target cardinality='(1,1)' polymorphic='True' roleLabel='target'>"
        "           <Class class='B' />"
        "       </Target>"
        "   </ECRelationshipClass>"
        "   <ECRelationshipClass typeName='ARelC' strength='referencing' strengthDirection='forward' modifier='abstract'>"
        "       <BaseClass>ARelB</BaseClass>"
        "       <Source cardinality='(0,1)' polymorphic='True'>"
        "           <Class class='A' />"
        "       </Source>"
        "       <Target cardinality='(0,N)' polymorphic='True'>"
        "           <Class class='C' />"
        "       </Target>"
        "   </ECRelationshipClass>"
        "</ECSchema>";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXml, *schemaContext);
    ASSERT_EQ(SchemaReadStatus::Success, status);
    ASSERT_TRUE(schema.IsValid());

    EXPECT_FALSE(schema->IsECVersion(ECVersion::V3_1));
    EXPECT_EQ(1, schema->GetClassCP("ARelB")->GetRelationshipClassCP()->GetSource().GetMultiplicity().GetLowerLimit());
    EXPECT_EQ(1, schema->GetClassCP("ARelB")->GetRelationshipClassCP()->GetSource().GetMultiplicity().GetUpperLimit());
    EXPECT_EQ(1, schema->GetClassCP("ARelB")->GetRelationshipClassCP()->GetTarget().GetMultiplicity().GetLowerLimit());
    EXPECT_EQ(1, schema->GetClassCP("ARelB")->GetRelationshipClassCP()->GetTarget().GetMultiplicity().GetUpperLimit());

    EXPECT_EQ(0, schema->GetClassCP("ARelC")->GetRelationshipClassCP()->GetSource().GetMultiplicity().GetLowerLimit());
    EXPECT_EQ(1, schema->GetClassCP("ARelC")->GetRelationshipClassCP()->GetSource().GetMultiplicity().GetUpperLimit());
    EXPECT_EQ(0, schema->GetClassCP("ARelC")->GetRelationshipClassCP()->GetTarget().GetMultiplicity().GetLowerLimit());
    EXPECT_EQ(INT_MAX, schema->GetClassCP("ARelC")->GetRelationshipClassCP()->GetTarget().GetMultiplicity().GetUpperLimit());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    08/2016
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaDeserializationConversionTest, ExpectSuccessWithViolatedClassConstraint)
    {
    Utf8CP schemaXml = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='testSchema' version='01.00' nameSpacePrefix='ts' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECEntityClass typeName='A' modifier='abstract'></ECEntityClass>"
        "   <ECEntityClass typeName='B' modifier='abstract'></ECEntityClass>"
        "   <ECEntityClass typeName='C' modifier='abstract'></ECEntityClass>"
        "   <ECRelationshipClass typeName='ARelB' strength='referencing' strengthDirection='forward' modifier='abstract'>"
        "       <Source cardinality='(1,1)' polymorphic='True'>"
        "           <Class class='A' />"
        "       </Source>"
        "       <Target cardinality='(1,1)' polymorphic='True'>"
        "           <Class class='B' />"
        "       </Target>"
        "   </ECRelationshipClass>"
        "   <ECRelationshipClass typeName='ARelC' strength='referencing' strengthDirection='forward' modifier='abstract'>"
        "       <BaseClass>ARelB</BaseClass>"
        "       <Source cardinality='(1,1)' polymorphic='True'>"
        "           <Class class='A' />"
        "       </Source>"
        "       <Target cardinality='(1,1)' polymorphic='True'>"
        "           <Class class='C' />"
        "       </Target>"
        "   </ECRelationshipClass>"
        "</ECSchema>";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXml, *schemaContext);
    ASSERT_EQ(SchemaReadStatus::Success, status);
    ASSERT_TRUE(schema.IsValid());

    EXPECT_FALSE(schema->IsECVersion(ECVersion::V3_1));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    08/2016
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaDeserializationConversionTest, TestNamespacePrefixAttribute)
    {
    Utf8CP schemaXml = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='testSchema' version='01.00' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "</ECSchema>";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXml, *schemaContext);
    EXPECT_EQ(SchemaReadStatus::Success, status);
    EXPECT_TRUE(schema.IsValid());
    ASSERT_TRUE(schema->IsECVersion(ECVersion::Latest));
    EXPECT_STREQ(schema->GetName().c_str(), schema->GetAlias().c_str());

    Utf8CP schemaXml2 = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='testSchema2' nameSpacePrefix='' version='01.00' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "</ECSchema>";

    ECSchemaPtr schema2;
    status = ECSchema::ReadFromXmlString(schema2, schemaXml2, *schemaContext);
    EXPECT_EQ(SchemaReadStatus::Success, status);
    EXPECT_TRUE(schema2.IsValid());
    ASSERT_TRUE(schema2->IsECVersion(ECVersion::Latest));
    EXPECT_STREQ(schema2->GetName().c_str(), schema2->GetAlias().c_str());

    Utf8CP schemaXml3 = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='testSchema3' version='01.00' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "</ECSchema>";

    ECSchemaPtr schema3;
    status = ECSchema::ReadFromXmlString(schema3, schemaXml3, *schemaContext);
    EXPECT_EQ(SchemaReadStatus::Success, status);
    EXPECT_TRUE(schema3.IsValid());
    ASSERT_TRUE(schema3->IsECVersion(ECVersion::Latest));
    EXPECT_STREQ(schema3->GetName().c_str(), schema3->GetAlias().c_str());

    Utf8CP schemaXml4 = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='testSchema4' nameSpacePrefix='' version='01.00' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "</ECSchema>";

    ECSchemaPtr schema4;
    status = ECSchema::ReadFromXmlString(schema4, schemaXml4, *schemaContext);
    EXPECT_EQ(SchemaReadStatus::Success, status);
    EXPECT_TRUE(schema4.IsValid());
    ASSERT_TRUE(schema4->IsECVersion(ECVersion::Latest));
    EXPECT_STREQ(schema4->GetName().c_str(), schema4->GetAlias().c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    09/2016
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaDeserializationConversionTest, TestPolymorphicAttribute)
    {
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();

    Utf8CP schemaXml = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='testSchema' version='01.00' nameSpacePrefix='ts' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "   <ECClass typeName='A' isDomainClass='true'></ECClass>"
        "   <ECClass typeName='B' isDomainClass='true'></ECClass>"
        "   <ECRelationshipClass typeName='ARelB' isDomainClass='false' strength='referencing' strengthDirection='forward'>"
        "       <Source cardinality='(1,1)' >"
        "           <Class class='A' />"
        "       </Source>"
        "       <Target cardinality='(1,1)' >"
        "           <Class class='B' />"
        "       </Target>"
        "   </ECRelationshipClass>"
        "</ECSchema>";

    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXml, *schemaContext);
    ASSERT_EQ(SchemaReadStatus::Success, status);
    ASSERT_TRUE(schema.IsValid());
    ASSERT_TRUE(schema->IsECVersion(ECVersion::Latest));
    ASSERT_EQ(true, schema->GetClassP("ARelB")->GetRelationshipClassP()->GetSource().GetIsPolymorphic());
    ASSERT_EQ(true, schema->GetClassP("ARelB")->GetRelationshipClassP()->GetTarget().GetIsPolymorphic());

    Utf8CP schemaXml2 = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='testSchema2' version='01.00' nameSpacePrefix='ts' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "   <ECClass typeName='A' isDomainClass='true'></ECClass>"
        "   <ECClass typeName='B' isDomainClass='true'></ECClass>"
        "   <ECRelationshipClass typeName='ARelB' isDomainClass='false' strength='referencing' strengthDirection='forward'>"
        "       <Source cardinality='(1,1)' polymorphic='' >"
        "           <Class class='A' />"
        "       </Source>"
        "       <Target cardinality='(1,1)' polymorphic='' >"
        "           <Class class='B' />"
        "       </Target>"
        "   </ECRelationshipClass>"
        "</ECSchema>";

    ECSchemaPtr schema2;
    status = ECSchema::ReadFromXmlString(schema2, schemaXml2, *schemaContext);
    ASSERT_EQ(SchemaReadStatus::Success, status);
    ASSERT_TRUE(schema2.IsValid());
    ASSERT_TRUE(schema2->IsECVersion(ECVersion::Latest));
    ASSERT_EQ(true, schema2->GetClassP("ARelB")->GetRelationshipClassP()->GetSource().GetIsPolymorphic());
    ASSERT_EQ(true, schema2->GetClassP("ARelB")->GetRelationshipClassP()->GetTarget().GetIsPolymorphic());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    06/2017
//---------------+---------------+---------------+---------------+---------------+-------
void SchemaDeserializationConversionTest::TestRoleLabelAttribute(ECVersion ecVersion, bool useEmptyString)
    {
    Utf8CP schemaXml;
    if (ECVersion::V2_0 == ecVersion)
        schemaXml = R"xml(<?xml version='1.0' encoding='UTF-8'?>
            <ECSchema schemaName='testSchema' version='01.00' nameSpacePrefix='ts' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>
                <ECClass typeName='A' isDomainClass='true'></ECClass>
                <ECClass typeName='B' isDomainClass='true'></ECClass>
                <ECRelationshipClass typeName='ARelB' isDomainClass="true" strength='referencing' strengthDirection='forward'>
                    <Source cardinality='(1,1)' polymorphic='True' %s >
                        <Class class='A' />
                    </Source>
                    <Target cardinality="(1,1)" polymorphic='True' %s >
                        <Class class="B" />
                    </Target>
                </ECRelationshipClass>
            </ECSchema>)xml";
    else
        schemaXml = R"xml(<?xml version='1.0' encoding='UTF-8'?>
            <ECSchema schemaName='testSchema' version='01.00' nameSpacePrefix='ts' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                <ECEntityClass typeName='A'></ECEntityClass>
                <ECEntityClass typeName='B'></ECEntityClass>
                <ECRelationshipClass typeName='ARelB' strength='referencing' strengthDirection='forward'>
                    <Source multiplicity='(1..1)' polymorphic='True' %s >
                        <Class class='A' />
                    </Source>
                    <Target multiplicity="(1..1)" polymorphic='True' %s >
                        <Class class="B" />
                    </Target>
                </ECRelationshipClass>
            </ECSchema>)xml";

    Utf8CP roleLabelString = (useEmptyString) ? "roleLabel=''" : "";

    Utf8String formattedSchemaXml;
    formattedSchemaXml.Sprintf(schemaXml, roleLabelString, roleLabelString);

    ECSchemaPtr schema;
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, formattedSchemaXml.c_str(), *schemaContext);

    ASSERT_EQ(SchemaReadStatus::Success, status) << "Failed to deserialize an EC" << ECSchema::GetECVersionString(ecVersion) << " schema";
    ASSERT_TRUE(schema.IsValid());
    ASSERT_TRUE(schema->IsECVersion(ECVersion::Latest));

    ECRelationshipClassCP relClass = schema->GetClassCP("ARelB")->GetRelationshipClassCP();
    EXPECT_TRUE(relClass->GetSource().IsRoleLabelDefined());
    EXPECT_STREQ("ARelB", relClass->GetSource().GetRoleLabel().c_str());
    EXPECT_TRUE(relClass->GetTarget().IsRoleLabelDefined());
    EXPECT_STREQ("ARelB (Reversed)", relClass->GetTarget().GetRoleLabel().c_str());

    ValidateRoundTripLatestSerialization(schema);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    06/2017
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaDeserializationConversionTest, TestRoleLabelAttribute)
    {
    TestRoleLabelAttribute(ECVersion::V2_0, false);
    TestRoleLabelAttribute(ECVersion::V2_0, true);
    TestRoleLabelAttribute(ECVersion::V3_0, false);
    TestRoleLabelAttribute(ECVersion::V3_0, true);
    }

void SchemaDeserializationConversionTest::TestInheritedRoleLabelAttribute(ECVersion ecVersion, bool noRoleLabel, Utf8CP roleLabel)
    {
    Utf8CP schemaXml;
    if (ECVersion::V2_0 == ecVersion)
        schemaXml = R"xml(<?xml version="1.0" encoding='UTF-8'?>
            <ECSchema schemaName="testSchema" version="01.00" nameSpacePrefix="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
                <ECClass typeName="A" isDomainClass="true"/>
                <ECClass typeName="B" isDomainClass="true">
                    <BaseClass>C</BaseClass>
                </ECClass>
                <ECClass typeName="C" isDomainClass="true"/>
                <ECRelationshipClass typeName="ARelC" isDomainClass="false" strength="referencing" strengthDirection="forward">
                    <Source cardinality='(1,1)' polymorphic='True' %s>
                        <Class class='A' />
                    </Source>
                    <Target cardinality='(1,1)' polymorphic='True' %s>
                        <Class class='C' />
                    </Target>
                </ECRelationshipClass>
                <ECRelationshipClass typeName='ARelB' isDomainClass='false' strength='referencing' strengthDirection='forward'>
                    <BaseClass>ARelC</BaseClass>
                    <Source cardinality='(1,1)' polymorphic='True'>
                        <Class class='A' />
                    </Source>
                    <Target cardinality='(1,1)' polymorphic='True'>
                        <Class class='B' />
                    </Target>
                </ECRelationshipClass>
            </ECSchema>)xml";
    else
        schemaXml = R"xml(<?xml version='1.0' encoding='UTF-8'?>
            <ECSchema schemaName='testSchema' version='01.00' namespacePrefix='ts' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>
                <ECEntityClass typeName='A'/>
                <ECEntityClass typeName='B'>
                    <BaseClass>C</BaseClass>
                </ECEntityClass>
                <ECEntityClass typeName='C'/>
                <ECRelationshipClass typeName='ARelC' strength='referencing' strengthDirection='forward'>
                    <Source multiplicity='(1..1)' polymorphic='True' %s>
                        <Class class='A' />
                    </Source>
                    <Target multiplicity='(1..1)' polymorphic='True' %s>
                        <Class class='C' />
                    </Target>
                </ECRelationshipClass>
                <ECRelationshipClass typeName='ARelB' strength='referencing' strengthDirection='forward'>
                    <BaseClass>ARelC</BaseClass>
                    <Source multiplicity='(1..1)' polymorphic='True'>
                        <Class class='A' />
                    </Source>
                    <Target multiplicity='(1..1)' polymorphic='True'>
                        <Class class='B' />
                    </Target>
                </ECRelationshipClass>
            </ECSchema>)xml";

    Utf8String roleLabelString;
    if (!noRoleLabel)
        {
        roleLabelString = "roleLabel='";
        roleLabelString += roleLabel;
        roleLabelString += "'";
        }
    else
        roleLabelString = roleLabel;

    Utf8String formattedSchemaXml;
    formattedSchemaXml.Sprintf(schemaXml, roleLabelString.c_str(), roleLabelString.c_str());

    ECSchemaPtr schema;
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, formattedSchemaXml.c_str(), *schemaContext);
    ASSERT_EQ(SchemaReadStatus::Success, status) << "Failed schema:\n" << formattedSchemaXml.c_str();
    ASSERT_TRUE(schema.IsValid());
    ASSERT_TRUE(schema->IsECVersion(ECVersion::Latest));

    ECRelationshipClassCP relClass = schema->GetClassCP("ARelB")->GetRelationshipClassCP();
    EXPECT_TRUE(relClass->GetSource().IsRoleLabelDefined());

    Utf8CP testSourceRoleLabel;
    if (noRoleLabel || Utf8String::IsNullOrEmpty(roleLabel))
        testSourceRoleLabel = "ARelB";
    else
        testSourceRoleLabel = roleLabel;

    EXPECT_STREQ(testSourceRoleLabel, relClass->GetSource().GetRoleLabel().c_str());

    EXPECT_TRUE(relClass->GetTarget().IsRoleLabelDefined());

    Utf8CP testTargetRoleLabel;
    if (noRoleLabel || Utf8String::IsNullOrEmpty(roleLabel))
        testTargetRoleLabel = "ARelB (Reversed)";
    else
        testTargetRoleLabel = roleLabel;

    EXPECT_STREQ(testTargetRoleLabel, relClass->GetTarget().GetRoleLabel().c_str());

    ValidateRoundTripLatestSerialization(schema);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    06/2017
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaDeserializationConversionTest, TestInheritedRoleLabelAttribute)
    {
    TestInheritedRoleLabelAttribute(ECVersion::V2_0, true);
    TestInheritedRoleLabelAttribute(ECVersion::V2_0, false);
    TestInheritedRoleLabelAttribute(ECVersion::V2_0, false, "testRoleLabel");
    TestInheritedRoleLabelAttribute(ECVersion::V3_0, true); // no roleLabel
    TestInheritedRoleLabelAttribute(ECVersion::V3_0, false); // empty roleLabel
    TestInheritedRoleLabelAttribute(ECVersion::V3_0, false, "testRoleLabel");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    10/2016
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaDeserializationConversionTest, TestAbstractConstraintAttribute)
    {
    {
    Utf8CP schemaXml = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='testSchema' version='01.00' nameSpacePrefix='ts' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "   <ECClass typeName='A' isDomainClass='true'/>"
        "   <ECClass typeName='B' isDomainClass='true'/>"
        "   <ECRelationshipClass typeName='ARelB' isDomainClass='false' strength='referencing' strengthDirection='forward'>"
        "       <Source cardinality='(1,1)' polymorphic='True'>"
        "           <Class class='A' />"
        "       </Source>"
        "       <Target cardinality='(1,1)' polymorphic='True'>"
        "           <Class class='B' />"
        "       </Target>"
        "   </ECRelationshipClass>"
        "</ECSchema>";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXml, *schemaContext);
    ASSERT_EQ(SchemaReadStatus::Success, status);
    ASSERT_TRUE(schema.IsValid());
    ASSERT_TRUE(schema->IsECVersion(ECVersion::Latest));
    EXPECT_STREQ("A", schema->GetClassCP("ARelB")->GetRelationshipClassCP()->GetSource().GetAbstractConstraint()->GetName().c_str()) << "The Source Constraint's Abstract Constraint attribute should be implicitly set to A.";
    EXPECT_STREQ("B", schema->GetClassCP("ARelB")->GetRelationshipClassCP()->GetTarget().GetAbstractConstraint()->GetName().c_str()) << "The Target Constraint's Abstract Constraint attribute should be implicitly set to B.";
    }
    {
    Utf8CP schemaXml = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='testSchema' version='01.00' nameSpacePrefix='ts' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "   <ECClass typeName='A' isDomainClass='true'/>"
        "   <ECClass typeName='B' isDomainClass='true'/>"
        "   <ECClass typeName='C' isDomainClass='true'>"
        "       <BaseClass>B</BaseClass>"
        "   </ECClass>"
        "   <ECRelationshipClass typeName='ARelB' isDomainClass='false' strength='referencing' strengthDirection='forward'>"
        "       <Source cardinality='(1,1)' polymorphic='True'>"
        "           <Class class='A' />"
        "       </Source>"
        "       <Target cardinality='(1,1)' polymorphic='True'>"
        "           <Class class='C' />"
        "           <Class class='B' />"
        "       </Target>"
        "   </ECRelationshipClass>"
        "</ECSchema>";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXml, *schemaContext);
    ASSERT_EQ(SchemaReadStatus::Success, status);
    ASSERT_TRUE(schema.IsValid());
    ASSERT_TRUE(schema->IsECVersion(ECVersion::Latest));

    EXPECT_STREQ("A", schema->GetClassCP("ARelB")->GetRelationshipClassCP()->GetSource().GetAbstractConstraint()->GetName().c_str()) << "The Source Constraint's Abstract Constraint attribute should be implicitly set to A.";
    // The Target should be automatically set to B since it is the common base class between the constraint classes.
    EXPECT_STREQ("B", schema->GetClassCP("ARelB")->GetRelationshipClassCP()->GetTarget().GetAbstractConstraint()->GetName().c_str()) << "The Target Constraint's Abstract Constraint attribute should have been automatically set to B.";
    }
    {
    Utf8CP schemaXml = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='testSchema' version='01.00' nameSpacePrefix='ts' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "   <ECClass typeName='A' isDomainClass='true'/>"
        "   <ECClass typeName='B' isDomainClass='true'/>"
        "   <ECClass typeName='C' isDomainClass='true'/>"
        "   <ECClass typeName='D' isDomainClass='true'>"
        "       <BaseClass>B</BaseClass>"
        "   </ECClass>"
        "   <ECRelationshipClass typeName='ARelB' isDomainClass='false' strength='referencing' strengthDirection='forward'>"
        "       <Source cardinality='(1,1)' polymorphic='True'>"
        "           <Class class='A' />"
        "       </Source>"
        "       <Target cardinality='(1,1)' polymorphic='True'>"
        "           <Class class='B' />"
        "           <Class class='C' />"
        "           <Class class='D' />"
        "       </Target>"
        "   </ECRelationshipClass>"
        "</ECSchema>";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXml, *schemaContext);
    ASSERT_EQ(SchemaReadStatus::Success, status);
    ASSERT_TRUE(schema.IsValid());
    ASSERT_FALSE(schema->IsECVersion(ECVersion::Latest)) << "The schema should have failed the " << ECSchema::GetECVersionString(ECVersion::Latest) << " validation because there was no common base class between all of the Target constraint classes";
    }
    {
    Utf8CP schemaXml = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='testSchema' version='01.00' nameSpacePrefix='ts' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "   <ECClass typeName='A' isDomainClass='true'/>"
        "   <ECClass typeName='B' isDomainClass='true'/>"
        "   <ECClass typeName='C' isDomainClass='true'>"
        "       <BaseClass>B</BaseClass>"
        "   </ECClass>"
        "   <ECClass typeName='D' isDomainClass='true'>"
        "       <BaseClass>E</BaseClass>"
        "   </ECClass>"
        "   <ECClass typeName='E' isDomainClass='true'>"
        "       <BaseClass>B</BaseClass>"
        "   </ECClass>"
        "   <ECRelationshipClass typeName='TestRel' isDomainClass='false' strength='referencing' strengthDirection='forward'>"
        "       <Source cardinality='(1,1)' polymorphic='True'>"
        "           <Class class='A' />"
        "       </Source>"
        "       <Target cardinality='(1,1)' polymorphic='True'>"
        "           <Class class='C' />"
        "           <Class class='D' />"
        "       </Target>"
        "   </ECRelationshipClass>"
        "</ECSchema>";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXml, *schemaContext);
    ASSERT_EQ(SchemaReadStatus::Success, status);
    ASSERT_TRUE(schema.IsValid());
    ASSERT_TRUE(schema->IsECVersion(ECVersion::Latest)) << "The schema should be a valid " << ECSchema::GetECVersionString(ECVersion::Latest) << " schema because there is a shared base class between all of the Target constraint classes";

    EXPECT_STREQ("A", schema->GetClassCP("TestRel")->GetRelationshipClassCP()->GetSource().GetAbstractConstraint()->GetName().c_str()) << "The Source Constraint's Abstract Constraint attribute should be implicitly set to A.";
    // The Target should be automatically set to B since it is the common base class between the constraint classes.
    EXPECT_STREQ("B", schema->GetClassCP("TestRel")->GetRelationshipClassCP()->GetTarget().GetAbstractConstraint()->GetName().c_str()) << "The Target Constraint's Abstract Constraint attribute should have been automatically set to B because it is a common base class of the constraint classes.";
    }
    {
    Utf8CP schemaXml = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='testSchema' version='01.00' nameSpacePrefix='ts' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "   <ECClass typeName='A' isDomainClass='true'/>"
        "   <ECClass typeName='CommonClass' isDomainClass='true'/>"
        "   <ECClass typeName='B' isDomainClass='true'>"
        "       <BaseClass>CommonClass</BaseClass>"
        "   </ECClass>"
        "   <ECClass typeName='C' isDomainClass='true'>"
        "       <BaseClass>CommonClass</BaseClass>"
        "   </ECClass>"
        "   <ECClass typeName='D' isDomainClass='true'>"
        "       <BaseClass>C</BaseClass>"
        "   </ECClass>"
        "   <ECClass typeName='E' isDomainClass='true'>"
        "       <BaseClass>D</BaseClass>"
        "   </ECClass>"
        "   <ECRelationshipClass typeName='TestRel' isDomainClass='false' strength='referencing' strengthDirection='forward'>"
        "       <Source cardinality='(1,1)' polymorphic='True'>"
        "           <Class class='A' />"
        "       </Source>"
        "       <Target cardinality='(1,1)' polymorphic='True'>"
        "           <Class class='D' />"
        "           <Class class='E' />"
        "           <Class class='B' />"
        "       </Target>"
        "   </ECRelationshipClass>"
        "</ECSchema>";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXml, *schemaContext);
    ASSERT_EQ(SchemaReadStatus::Success, status);
    ASSERT_TRUE(schema.IsValid());
    ASSERT_TRUE(schema->IsECVersion(ECVersion::Latest)) << "The schema should be a valid " << ECSchema::GetECVersionString(ECVersion::Latest) << " schema because there is a shared base class between all of the Target constraint classes";

    EXPECT_STREQ("A", schema->GetClassCP("TestRel")->GetRelationshipClassCP()->GetSource().GetAbstractConstraint()->GetName().c_str()) << "The Source Constraint's Abstract Constraint attribute should be implicitly set to A.";
    // The Target should be automatically set to CommonClass since it is the common base class between the constraint classes.
    EXPECT_STREQ("CommonClass", schema->GetClassCP("TestRel")->GetRelationshipClassCP()->GetTarget().GetAbstractConstraint()->GetName().c_str()) << "The Target Constraint's Abstract Constraint attribute should have been automatically set to CommonClass because it is a common base class of the constraint classes.";
    }
    {
    Utf8CP schemaXml = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='testSchema' version='01.00' nameSpacePrefix='ts' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "   <ECClass typeName='A' isDomainClass='true'/>"
        "   <ECClass typeName='CommonClass' isDomainClass='true'/>"
        "   <ECClass typeName='B' isDomainClass='true'/>"
        "   <ECClass typeName='C' isDomainClass='true'>"
        "       <BaseClass>CommonClass</BaseClass>"
        "   </ECClass>"
        "   <ECClass typeName='D' isDomainClass='true'>"
        "       <BaseClass>C</BaseClass>"
        "   </ECClass>"
        "   <ECClass typeName='E' isDomainClass='true'>"
        "       <BaseClass>D</BaseClass>"
        "   </ECClass>"
        "   <ECRelationshipClass typeName='TestRel' isDomainClass='false' strength='referencing' strengthDirection='forward'>"
        "       <Source cardinality='(1,1)' polymorphic='True'>"
        "           <Class class='A' />"
        "       </Source>"
        "       <Target cardinality='(1,1)' polymorphic='True'>"
        "           <Class class='D' />"
        "           <Class class='E' />"
        "           <Class class='B' />"
        "       </Target>"
        "   </ECRelationshipClass>"
        "</ECSchema>";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXml, *schemaContext);
    ASSERT_EQ(SchemaReadStatus::Success, status);
    ASSERT_TRUE(schema.IsValid());
    ASSERT_FALSE(schema->IsECVersion(ECVersion::Latest)) << "The schema should not validate as " << ECSchema::GetECVersionString(ECVersion::Latest) << " because there is a not a shared base class between B and the other Target constraint classes";

    EXPECT_STREQ("A", schema->GetClassCP("TestRel")->GetRelationshipClassCP()->GetSource().GetAbstractConstraint()->GetName().c_str()) << "The Source Constraint's Abstract Constraint attribute should be implicitly set to A.";
    // The Target should be automatically set to CommonClass since it is the common base class between the constraint classes.

    ECEntityClassCP commonClass = schema->GetClassCP("CommonClass")->GetEntityClassCP();

    EXPECT_NE(ECObjectsStatus::Success, schema->GetClassCP("TestRel")->GetRelationshipClassCP()->GetTarget().SetAbstractConstraint(*commonClass)) << "The abstract constraint cannot be set on the target because all of the constraint classes do not derive from that class.";

    EXPECT_EQ(ECObjectsStatus::Success, schema->GetClassP("B")->AddBaseClass(*schema->GetClassP("CommonClass"))) << "Adding the CommonClass as a base class of B should succeed.";
    EXPECT_EQ(ECObjectsStatus::Success, schema->GetClassCP("TestRel")->GetRelationshipClassCP()->GetTarget().SetAbstractConstraint(*commonClass)) << "The abstract constraint should now be able to be CommonClass, since all of the constraint classes derive from it.";

    EXPECT_TRUE(schema->Validate()) << "The schema should now validate successfully.";
    EXPECT_TRUE(schema->IsECVersion(ECVersion::Latest)) << "The schema should now validate to EC" << ECSchema::GetECVersionString(ECVersion::Latest);
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    10/2016
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaDeserializationConversionTest, TestEmptyConstraints)
    {
    Utf8CP schemaXml = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='testSchema' version='01.00' nameSpacePrefix='ts' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "   <ECClass typeName='A' isDomainClass='true'/>"
        "   <ECClass typeName='B' isDomainClass='true'/>"
        "   <ECRelationshipClass typeName='TestRel' isDomainClass='false' strength='referencing' strengthDirection='forward'>"
        "       <Source cardinality='(1,1)' polymorphic='True' roleLabel='Source'>"
        "       </Source>"
        "       <Target cardinality='(1,1)' polymorphic='True' roleLabel='Target'>"
        "       </Target>"
        "   </ECRelationshipClass>"
        "</ECSchema>";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXml, *schemaContext);
    ASSERT_EQ(SchemaReadStatus::Success, status) << "The ECSchema should deserialize even though it will be an EC3.0 schema.";
    ASSERT_TRUE(schema.IsValid());
    ASSERT_TRUE(schema->IsECVersion(ECVersion::V3_0)) << "The schema should validate as an 3.0 schema because there are no constraint classes.";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    12/2016
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaDeserializationConversionTest, TestDroppingRelationshipBaseClasses)
    {
    Utf8CP schemaXml = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='testSchema' version='01.00' nameSpacePrefix='ts' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "   <ECClass typeName='A' isDomainClass='true'/>"
        "   <ECClass typeName='B' isDomainClass='true'/>"
        "   <ECRelationshipClass typeName='BaseRel1' isDomainClass='false' strength='referencing' strengthDirection='forward'>"
        "       <Source cardinality='(1,1)' polymorphic='True' roleLabel='Source'>"
        "           <Class class='A' />"
        "       </Source>"
        "       <Target cardinality='(1,1)' polymorphic='True' roleLabel='Target'>"
        "           <Class class='B' />"
        "       </Target>"
        "   </ECRelationshipClass>"
        "   <ECRelationshipClass typeName='BaseRel2' isDomainClass='false' strength='referencing' strengthDirection='forward'>"
        "       <Source cardinality='(1,1)' polymorphic='True' roleLabel='Source'>"
        "           <Class class='A' />"
        "       </Source>"
        "       <Target cardinality='(1,1)' polymorphic='True' roleLabel='Target'>"
        "           <Class class='B' />"
        "       </Target>"
        "   </ECRelationshipClass>"
        "   <ECRelationshipClass typeName='TestRel' isDomainClass='false' strength='referencing' strengthDirection='forward'>"
        "       <BaseClass>BaseRel1</BaseClass>"
        "       <BaseClass>BaseRel2</BaseClass>"
        "       <Source cardinality='(1,1)' polymorphic='True' roleLabel='Source'>"
        "           <Class class='A' />"
        "       </Source>"
        "       <Target cardinality='(1,1)' polymorphic='True' roleLabel='Target'>"
        "           <Class class='B' />"
        "       </Target>"
        "   </ECRelationshipClass>"
        "</ECSchema>";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXml, *schemaContext);
    ASSERT_EQ(SchemaReadStatus::Success, status) << "The ECSchema should deserialize even though the relationship has multiple base classes.";
    ASSERT_TRUE(schema.IsValid()) << "The schema is invalid even though success was returned";
    ASSERT_TRUE(schema->IsECVersion(ECVersion::Latest)) << "The schema should validate because the last base class defined should be the only one kept.";

    Utf8CP schemaXml2 = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='testSchema2' version='01.00' nameSpacePrefix='ts2' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "   <ECEntityClass typeName='A' />"
        "   <ECEntityClass typeName='B' />"
        "   <ECRelationshipClass typeName='BaseRel1' strength='referencing' strengthDirection='forward'>"
        "       <Source cardinality='(1,1)' polymorphic='True' roleLabel='Source'>"
        "           <Class class='A' />"
        "       </Source>"
        "       <Target cardinality='(1,1)' polymorphic='True' roleLabel='Target'>"
        "           <Class class='B' />"
        "       </Target>"
        "   </ECRelationshipClass>"
        "   <ECRelationshipClass typeName='BaseRel2' strength='referencing' strengthDirection='forward'>"
        "       <Source cardinality='(1,1)' polymorphic='True' roleLabel='Source'>"
        "           <Class class='A' />"
        "       </Source>"
        "       <Target cardinality='(1,1)' polymorphic='True' roleLabel='Target'>"
        "           <Class class='B' />"
        "       </Target>"
        "   </ECRelationshipClass>"
        "   <ECRelationshipClass typeName='TestRel' strength='referencing' strengthDirection='forward'>"
        "       <BaseClass>BaseRel1</BaseClass>"
        "       <BaseClass>BaseRel2</BaseClass>"
        "       <Source cardinality='(1,1)' polymorphic='True' roleLabel='Source'>"
        "           <Class class='A' />"
        "       </Source>"
        "       <Target cardinality='(1,1)' polymorphic='True' roleLabel='Target'>"
        "           <Class class='B' />"
        "       </Target>"
        "   </ECRelationshipClass>"
        "</ECSchema>";

    ECSchemaPtr schema2;
    status = ECSchema::ReadFromXmlString(schema2, schemaXml2, *schemaContext);
    ASSERT_EQ(SchemaReadStatus::Success, status) << "The ECSchema should deserialize even though the relationship has multiple base classes.";
    ASSERT_TRUE(schema2.IsValid()) << "The schema is invalid even though success was returned";
    ASSERT_TRUE(schema2->IsECVersion(ECVersion::Latest)) << "The schema should validate because the last base class defined should be the only one kept.";

    bvector<ECSchemaPtr> schemas;
    schemas.push_back(schema);
    schemas.push_back(schema2);

    for (ECSchemaPtr ecSchema : schemas)
        {
        ECClassCP ecClass = ecSchema->GetClassCP("TestRel");
        ASSERT_TRUE(nullptr != ecClass) << "The class TestRel is missing from the schema " << ecSchema->GetName().c_str() << ".";
        ECRelationshipClassCP relClass = ecClass->GetRelationshipClassCP();
        ASSERT_TRUE(nullptr != relClass) << "The class " << ecClass->GetFullName() << " is not a valid relationship class";

        ASSERT_EQ(1, relClass->GetBaseClasses().size()) << "The relationship " << ecClass->GetFullName() << " should only have one base class";
        ECClassP baseClass = *relClass->GetBaseClasses().begin();
        ASSERT_TRUE(nullptr != baseClass) << "There are no base classes on " << ecClass->GetFullName() << " when the size was 1.";
        EXPECT_STREQ("BaseRel2", baseClass->GetName().c_str()) << "The base class was not the expected base class on relationship " << ecClass->GetFullName() << ".";
        }
    }

void TestOverriding(Utf8CP schemaName, int readVersion, bool allowOverriding)
    {
    Utf8CP schemaXml = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='%s' namespacePrefix='ts' version='%d.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "   <ECClass typeName='CustAttribute' isCustomAttributeClass='true'>"
        "       <ECProperty propertyName='TestValue' typeName='int'/>"
        "   </ECClass>"
        "   <ECClass typeName='derived' isDomainClass='True'>"
        "       <BaseClass>child</BaseClass>"
        "       <ECProperty propertyName='IntegerProperty' typeName='int' />"
        "       <ECArrayProperty propertyName='IntArrayProperty' typeName='int' minOccurs='0' maxOccurs='1' />"
        "   </ECClass>"
        "   <ECClass typeName='base' isDomainClass='True'>"
        "       <ECProperty propertyName='IntegerProperty' typeName='int' >"
        "           <ECCustomAttributes>"
        "               <CustAttribute xmlns='%s.%d.0'>"
        "                   <TestValue>4</TestValue>"
        "               </CustAttribute>"
        "           </ECCustomAttributes>"
        "       </ECProperty>"
        "       <ECArrayProperty propertyName='IntArrayProperty' typeName='int' minOccurs='0' maxOccurs='1' />"
        "   </ECClass>"
        "   <ECClass typeName='child' isDomainClass='True'>"
        "       <BaseClass>base</BaseClass>"
        "       <ECArrayProperty propertyName='IntegerProperty' typeName='int' minOccurs='0' maxOccurs='1' />"
        "       <ECProperty propertyName='IntArrayProperty' typeName='int' />"
        "       <ECArrayProperty propertyName='childProperty' typeName='int' minOccurs='0' maxOccurs='1' />"
        "   </ECClass>"
        "   <ECClass typeName='child2' isDomainClass='True'>"
        "       <BaseClass>base</BaseClass>"
        "       <ECProperty propertyName='childProperty' typeName='int'/>"
        "   </ECClass>"
        "</ECSchema>";

    Utf8String formattedSchemaXml;
    formattedSchemaXml.Sprintf(schemaXml, schemaName, readVersion, schemaName, readVersion);

    ECSchemaPtr schema;
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, formattedSchemaXml.c_str(), *schemaContext);
    if (!allowOverriding)
        {
        ASSERT_NE(SchemaReadStatus::Success, status) << "The schema " << schemaName << " should have failed to deserialize";
        return;
        }

    ASSERT_EQ(SchemaReadStatus::Success, status) << "The schema " << schemaName << " should deserialize successfully";
    ASSERT_TRUE(schema.IsValid()) << "The schema " << schemaName << " is invalid even though returned success";
    ASSERT_TRUE(schema->IsECVersion(ECVersion::Latest)) << "The schema " << schemaName << " deserialized to the wrong version of EC.";

    bvector<Utf8CP> classNames = {"base", "child", "derived"};
    for (Utf8CP className : classNames)
        {
        ECClassCP ecClass = schema->GetClassCP(className);
        ASSERT_TRUE(nullptr != ecClass) << "The class " << schemaName << ":" << className << " could not be found.";
        ECPropertyP ecProp = ecClass->GetPropertyP("IntegerProperty", false);
        ASSERT_TRUE(nullptr != ecProp) << "The property " << ecClass->GetFullName() << ".IntegerProperty could not be found.";
        PrimitiveArrayECPropertyCP arrProp = ecProp->GetAsPrimitiveArrayProperty();
        ASSERT_TRUE(nullptr != arrProp) << "The property " << ecClass->GetFullName() << "." << ecProp->GetName().c_str() << " is not a primitive array property when it should be.";
        EXPECT_EQ(PrimitiveType::PRIMITIVETYPE_Integer, arrProp->GetPrimitiveElementType()) << "The property " << ecClass->GetFullName() << "." << ecProp->GetName().c_str() << "is not of the expected type.";

        if (ecClass->GetName().Equals("base"))
            {
            IECInstancePtr instance = arrProp->GetCustomAttributeLocal("CustAttribute");
            ASSERT_TRUE(instance.IsValid()) << "Could not find the custom attribute CustAttribute on " << ecClass->GetFullName() << "." << ecProp->GetName().c_str();
            ECValue v;
            ECObjectsStatus instanceStatus = instance->GetValue(v, "TestValue");
            ASSERT_EQ(ECObjectsStatus::Success, instanceStatus) << "Failed to read the value of ";
            ASSERT_FALSE(v.IsNull()) << "The value of the TestValue CA is null in " << ecClass->GetFullName() << "." << ecProp->GetName().c_str() << " when it shouldn't be.";
            EXPECT_EQ(4, v.GetInteger()) << "The value of the TestValue CA is not as expected in " << ecClass->GetFullName() << "." << ecProp->GetName().c_str();
            }

        ECPropertyP ecProp2 = ecClass->GetPropertyP("IntArrayProperty", false);
        ASSERT_TRUE(nullptr != ecProp2) << "The property " << ecClass->GetFullName() << ".IntArrayProperty could not be found.";
        PrimitiveArrayECPropertyCP arrProp2 = ecProp2->GetAsPrimitiveArrayProperty();
        ASSERT_TRUE(nullptr != arrProp2) << "The property " << ecClass->GetFullName() << "." << ecProp2->GetName().c_str() << " is not a primitive array property when it should be.";
        EXPECT_EQ(PrimitiveType::PRIMITIVETYPE_Integer, arrProp2->GetPrimitiveElementType()) << "The property " << ecClass->GetFullName() << "." << ecProp->GetName().c_str() << "is not of the expected type.";
        }
    
    // Make sure once removed classes were not converted
    ECClassCP ecClass = schema->GetClassCP("child");
    ASSERT_TRUE(nullptr != ecClass) << "The class " << schemaName << ":child could not be found.";
    ECPropertyP ecProp = ecClass->GetPropertyP("childProperty", false);
    ASSERT_TRUE(nullptr != ecProp) << "The property " << ecClass->GetFullName() << ".childProperty could not be found.";
    PrimitiveArrayECPropertyCP arrProp = ecProp->GetAsPrimitiveArrayProperty();
    ASSERT_TRUE(nullptr != arrProp) << "The property " << ecClass->GetFullName() << "." << ecProp->GetName().c_str() << " is not a primitive array property when it should be.";
    EXPECT_EQ(PrimitiveType::PRIMITIVETYPE_Integer, arrProp->GetPrimitiveElementType()) << "The property " << ecClass->GetFullName() << "." << ecProp->GetName().c_str() << "is not of the expected type.";
    
    ecClass = schema->GetClassCP("child2");
    ASSERT_TRUE(nullptr != ecClass) << "The class " << schemaName << ":child2 could not be found.";
    ecProp = ecClass->GetPropertyP("childProperty", false);
    ASSERT_TRUE(nullptr != ecProp) << "The property " << ecClass->GetFullName() << ".childProperty could not be found.";
    PrimitiveECPropertyCP primProp = ecProp->GetAsPrimitiveProperty();
    ASSERT_TRUE(nullptr != primProp) << "The property " << ecClass->GetFullName() << "." << ecProp->GetName().c_str() << " is not a primitive property when it should be.";
    EXPECT_EQ(PrimitiveType::PRIMITIVETYPE_Integer, primProp->GetType()) << "The property " << ecClass->GetFullName() << "." << ecProp->GetName().c_str() << "is not of the expected type.";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    12/2016
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaDeserializationConversionTest, TestArrayPropertyOverriding)
    {
    TestOverriding("TestSchema", 5, false);
    TestOverriding("jclass", 1, true);
    TestOverriding("jclass", 2, true);
    TestOverriding("ECXA_ams", 1, true);
    TestOverriding("ECXA_ams_user", 1, true);
    TestOverriding("ams", 1, true);
    TestOverriding("ams_user", 1, true);
    TestOverriding("Bentley_JSpace_CustomAttributes", 2, true);
    TestOverriding("Bentley_Plant", 6, true);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    01/2017
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaDeserializationConversionTest, TestPropertyRenamingCustomAttribute)
    {
    Utf8CP schemaXml = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='TestSchema' namespacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "   <ECClass typeName='base' isDomainClass='True'>"
        "       <ECProperty propertyName='IntegerProperty' typeName='int' />"
        "   </ECClass>"
        "   <ECClass typeName='child' isDomainClass='True'>"
        "       <BaseClass>base</BaseClass>"
        "       <ECProperty propertyName='InteGerProperty' typeName='int'/>"
        "   </ECClass>"
        "   <ECClass typeName='child2' isDomainClass='True'>"
        "       <BaseClass>base</BaseClass>"
        "       <ECProperty propertyName='IntegerProperty' typeName='double'/>"
        "   </ECClass>"
        "</ECSchema>";

    ECSchemaReadContextPtr readContext = ECSchemaReadContext::CreateContext();
    ECSchemaPtr schema;
    ECSchema::ReadFromXmlString(schema, schemaXml, *readContext);
    ASSERT_TRUE(schema.IsValid()) << "Failed to read and convert schema.";
    ASSERT_TRUE(schema->IsECVersion(ECVersion::Latest));

    // Check that only a case change to a property has not added the CA
    {
    ECClassCP child = schema->GetClassCP("child");
    ASSERT_TRUE(nullptr != child) << "Failed to find the class 'child' in the schema";

    ECPropertyP childProp = child->GetPropertyP("IntegerProperty");
    ASSERT_TRUE(nullptr != childProp) << "Failed to find the property 'IntegerProperty' on the class '" << child->GetFullName() << "', even though it should not have been renamed.";

    IECInstancePtr childRename = child->GetCustomAttributeLocal("RenamedPropertiesMapping");
    EXPECT_FALSE(childRename.IsValid()) << "The class'" << child->GetFullName() << "' contains the custom attribute 'RenamedPropertiesMapping', even though no property was renamed.";
    }

    // Check that if properties differ by DateType the property is renamed
    {
    ECClassCP child = schema->GetClassCP("child2");
    ASSERT_TRUE(nullptr != child) << "Failed to find the class 'child2' in the schema";

    ECPropertyP childProp = child->GetPropertyP("TestSchema_IntegerProperty_");
    ASSERT_TRUE(nullptr != childProp) << "Failed to find the renamed property 'TestSchema_IntegerProperty_' on the class '" << child->GetFullName() << "'.";

    IECInstancePtr childRename = child->GetCustomAttributeLocal("RenamedPropertiesMapping");
    EXPECT_TRUE(childRename.IsValid()) << "The class '" << child->GetFullName() << "' does not contain the custom attribute 'RenamedPropertiesMapping', even though a property was renamed.";

    ECValue remapping;
    EXPECT_EQ(ECObjectsStatus::Success, childRename->GetValue(remapping, "PropertyMapping"));
    EXPECT_TRUE(!remapping.IsNull()) << "The property 'PropertyMapping' in the RenamedPropertiesMapping custom attribute on ECClass '" << child->GetFullName() << "' is null when it should not be.";

    Utf8String expected("IntegerProperty|TestSchema_IntegerProperty_");
    EXPECT_TRUE(expected.EqualsI(remapping.GetUtf8CP())) << "The supplied mapping in the CustomAttribute of the 'child2' ECClass is not correct";
    }
    }

//TEST_F(SchemaDeserializationConversionTest, TestBentleyPlant)
//    {
//    ECSchemaReadContextPtr readContext = ECSchemaReadContext::CreateContext();
//    readContext->AddSchemaPath(L"D:\\Files\\ECSchemas\\GraphiteTestDataSchemas_cleaned_11-30\\GraphiteTestDataSchemas_cleaned\\");
//    readContext->AddConversionSchemaPath(L"D:\\dev\\BIM0200Dev\\src\\DgnDbSync\\DgnV8\\ECSchemas\\V3Conversion");
//    
//    SchemaKey plant("Bentley_Plant", 6, 0);
//    SchemaKey ams("ams", 1, 0);
//    SchemaKey ams_user("ams_user", 1, 0);
//    SchemaKey jspace_CA("Bentley_JSpace_CustomAttributes", 2, 0);
//    
//    bvector<SchemaKey> testKeys = {plant, ams, ams_user, jspace_CA};
//    for (SchemaKey testKey : testKeys)
//        {
//        ECSchemaPtr ecSchema = readContext->LocateSchema(testKey, SchemaMatchType::Exact);
//        ASSERT_TRUE(ecSchema.IsValid());
//        ASSERT_TRUE(ecSchema->IsECVersion(ECVersion::Latest));
//        
//        Utf8String outSchema;
//        SchemaWriteStatus writeStatus = ecSchema->WriteToXmlString(outSchema);
//        ASSERT_EQ(SchemaWriteStatus::Success, writeStatus);
//        ASSERT_FALSE(Utf8String::IsNullOrEmpty(outSchema.c_str()));
//
//        WString inSchema;
//        BeStringUtilities::Utf8ToWChar(inSchema, outSchema.c_str());
//
//        ECSchemaReadContextPtr newReadContext = ECSchemaReadContext::CreateContext();
//        newReadContext->AddSchemaPath(L"D:\\Files\\ECSchemas\\GraphiteTestDataSchemas_cleaned_11-30\\GraphiteTestDataSchemas_cleaned\\");
//        newReadContext->AddConversionSchemaPath(L"D:\\dev\\BIM0200Dev\\src\\DgnDbSync\\DgnV8\\ECSchemas\\V3Conversion");
//        ECSchemaPtr newSchema;
//        SchemaReadStatus readStatus = ECSchema::ReadFromXmlString(newSchema, inSchema.c_str(), *newReadContext);
//        ASSERT_EQ(SchemaReadStatus::Success, readStatus);
//        ASSERT_TRUE(newSchema.IsValid());
//        }
//    }

END_BENTLEY_ECN_TEST_NAMESPACE