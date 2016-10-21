/*--------------------------------------------------------------------------------------+
|
|     $Source: test/Published/SchemaDeserializationConversionTests.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"
#include "BeXml/BeXml.h"

using namespace BentleyApi::ECN;

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

struct SchemaDeserializationConversionTest : ECTestFixture { };

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
    ASSERT_TRUE(schema->IsECVersion(3, 1));

    EXPECT_EQ(0, schema->GetClassCP("ARelB")->GetRelationshipClassCP()->GetSource().GetMultiplicity().GetLowerLimit());
    EXPECT_EQ(1, schema->GetClassCP("ARelB")->GetRelationshipClassCP()->GetSource().GetMultiplicity().GetUpperLimit());
    EXPECT_EQ(0, schema->GetClassCP("ARelB")->GetRelationshipClassCP()->GetTarget().GetMultiplicity().GetLowerLimit());
    EXPECT_EQ(UINT_MAX, schema->GetClassCP("ARelB")->GetRelationshipClassCP()->GetTarget().GetMultiplicity().GetUpperLimit());
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

    EXPECT_FALSE(schema->IsECVersion(3,1));
    EXPECT_EQ(1, schema->GetClassCP("ARelB")->GetRelationshipClassCP()->GetSource().GetMultiplicity().GetLowerLimit());
    EXPECT_EQ(1, schema->GetClassCP("ARelB")->GetRelationshipClassCP()->GetSource().GetMultiplicity().GetUpperLimit());
    EXPECT_EQ(1, schema->GetClassCP("ARelB")->GetRelationshipClassCP()->GetTarget().GetMultiplicity().GetLowerLimit());
    EXPECT_EQ(1, schema->GetClassCP("ARelB")->GetRelationshipClassCP()->GetTarget().GetMultiplicity().GetUpperLimit());

    EXPECT_EQ(0, schema->GetClassCP("ARelC")->GetRelationshipClassCP()->GetSource().GetMultiplicity().GetLowerLimit());
    EXPECT_EQ(1, schema->GetClassCP("ARelC")->GetRelationshipClassCP()->GetSource().GetMultiplicity().GetUpperLimit());
    EXPECT_EQ(0, schema->GetClassCP("ARelC")->GetRelationshipClassCP()->GetTarget().GetMultiplicity().GetLowerLimit());
    EXPECT_EQ(UINT_MAX, schema->GetClassCP("ARelC")->GetRelationshipClassCP()->GetTarget().GetMultiplicity().GetUpperLimit());
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

    EXPECT_FALSE(schema->IsECVersion(3,1));
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
    ASSERT_TRUE(schema->IsECVersion(3, 1));
    EXPECT_EQ(schema->GetName().c_str(), schema->GetAlias().c_str());

    Utf8CP schemaXml2 = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='testSchema2' nameSpacePrefix='' version='01.00' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "</ECSchema>";

    ECSchemaPtr schema2;
    status = ECSchema::ReadFromXmlString(schema2, schemaXml2, *schemaContext);
    EXPECT_EQ(SchemaReadStatus::Success, status);
    EXPECT_TRUE(schema2.IsValid());
    ASSERT_TRUE(schema2->IsECVersion(3, 1));
    EXPECT_EQ(schema2->GetName().c_str(), schema2->GetAlias().c_str());

    Utf8CP schemaXml3 = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='testSchema3' version='01.00' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "</ECSchema>";

    ECSchemaPtr schema3;
    status = ECSchema::ReadFromXmlString(schema3, schemaXml3, *schemaContext);
    EXPECT_EQ(SchemaReadStatus::Success, status);
    EXPECT_TRUE(schema3.IsValid());
    ASSERT_TRUE(schema3->IsECVersion(3, 1));
    EXPECT_EQ(schema3->GetName().c_str(), schema3->GetAlias().c_str());

    Utf8CP schemaXml4 = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='testSchema4' nameSpacePrefix='' version='01.00' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "</ECSchema>";

    ECSchemaPtr schema4;
    status = ECSchema::ReadFromXmlString(schema4, schemaXml4, *schemaContext);
    EXPECT_EQ(SchemaReadStatus::Success, status);
    EXPECT_TRUE(schema4.IsValid());
    ASSERT_TRUE(schema4->IsECVersion(3, 1));
    EXPECT_EQ(schema4->GetName().c_str(), schema4->GetAlias().c_str());
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
    ASSERT_TRUE(schema->IsECVersion(3, 1));
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
    ASSERT_TRUE(schema2->IsECVersion(3, 1));
    ASSERT_EQ(true, schema2->GetClassP("ARelB")->GetRelationshipClassP()->GetSource().GetIsPolymorphic());
    ASSERT_EQ(true, schema2->GetClassP("ARelB")->GetRelationshipClassP()->GetTarget().GetIsPolymorphic());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    09/2016
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaDeserializationConversionTest, TestRoleLabelAttribute)
    {
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();

    Utf8CP schemaXml = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='testSchema' version='01.00' nameSpacePrefix='ts' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "   <ECClass typeName='A' isDomainClass='true'></ECClass>"
        "   <ECClass typeName='B' isDomainClass='true'></ECClass>"
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
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXml, *schemaContext);
    ASSERT_EQ(SchemaReadStatus::Success, status);
    ASSERT_TRUE(schema.IsValid());
    ASSERT_TRUE(schema->IsECVersion(3, 1));
    ASSERT_STREQ("ARelB", schema->GetClassCP("ARelB")->GetRelationshipClassCP()->GetSource().GetRoleLabel().c_str());
    ASSERT_STREQ("ARelB (Reversed)", schema->GetClassCP("ARelB")->GetRelationshipClassCP()->GetTarget().GetRoleLabel().c_str());

    Utf8CP schemaXml2 = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='testSchema2' version='01.00' nameSpacePrefix='ts' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "   <ECClass typeName='A' isDomainClass='true'></ECClass>"
        "   <ECClass typeName='B' isDomainClass='true'></ECClass>"
        "   <ECRelationshipClass typeName='ARelB' isDomainClass='false' strength='referencing' strengthDirection='forward'>"
        "       <Source cardinality='(1,1)' polymorphic='True' roleLabel='' >"
        "           <Class class='A' />"
        "       </Source>"
        "       <Target cardinality='(1,1)' polymorphic='True' roleLabel='' >"
        "           <Class class='B' />"
        "       </Target>"
        "   </ECRelationshipClass>"
        "</ECSchema>";

    ECSchemaPtr schema2;
    status = ECSchema::ReadFromXmlString(schema2, schemaXml2, *schemaContext);
    ASSERT_EQ(SchemaReadStatus::Success, status);
    ASSERT_TRUE(schema2.IsValid());
    ASSERT_TRUE(schema2->IsECVersion(3, 1));
    ASSERT_STREQ("ARelB", schema2->GetClassCP("ARelB")->GetRelationshipClassCP()->GetSource().GetRoleLabel().c_str());
    ASSERT_STREQ("ARelB (Reversed)", schema2->GetClassCP("ARelB")->GetRelationshipClassCP()->GetTarget().GetRoleLabel().c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    09/2016
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaDeserializationConversionTest, TestInheritedRoleLabelAttribute)
    {
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();

    Utf8CP schemaXml = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='testSchema' version='01.00' nameSpacePrefix='ts' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "   <ECClass typeName='A' isDomainClass='true'></ECClass>"
        "   <ECClass typeName='B' isDomainClass='true'>"
        "       <BaseClass>C</BaseClass>"
        "   </ECClass>"
        "   <ECClass typeName='C' isDomainClass='true'></ECClass>"
        "   <ECRelationshipClass typeName='ARelC' isDomainClass='false' strength='referencing' strengthDirection='forward'>"
        "       <Source cardinality='(1,1)' polymorphic='True' roleLabel='testSource'>"
        "           <Class class='A' />"
        "       </Source>"
        "       <Target cardinality='(1,1)' polymorphic='True' roleLabel='testTarget'>"
        "           <Class class='C' />"
        "       </Target>"
        "   </ECRelationshipClass>"
        "   <ECRelationshipClass typeName='ARelB' isDomainClass='false' strength='referencing' strengthDirection='forward'>"
        "       <BaseClass>ARelC</BaseClass>"
        "       <Source cardinality='(1,1)' polymorphic='True'>"
        "           <Class class='A' />"
        "       </Source>"
        "       <Target cardinality='(1,1)' polymorphic='True'>"
        "           <Class class='B' />"
        "       </Target>"
        "   </ECRelationshipClass>"
        "</ECSchema>";

    ECSchemaPtr schema;
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXml, *schemaContext);
    ASSERT_EQ(SchemaReadStatus::Success, status);
    ASSERT_TRUE(schema.IsValid());
    ASSERT_TRUE(schema->IsECVersion(3, 1));
    ASSERT_STREQ("testSource", schema->GetClassCP("ARelB")->GetRelationshipClassCP()->GetSource().GetRoleLabel().c_str());
    ASSERT_STREQ("testTarget", schema->GetClassCP("ARelB")->GetRelationshipClassCP()->GetTarget().GetRoleLabel().c_str());
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
    ASSERT_TRUE(schema->IsECVersion(3, 1));
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
    ASSERT_TRUE(schema->IsECVersion(3, 1));

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
    ASSERT_FALSE(schema->IsECVersion(3, 1)) << "The schema should have failed the 3.1 validation because there was no common base class between all of the Target constraint classes";
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
    ASSERT_TRUE(schema->IsECVersion(3, 1)) << "The schema should be a valid 3.1 schema because there is a shared base class between all of the Target constraint classes";

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
    ASSERT_TRUE(schema->IsECVersion(3, 1)) << "The schema should be a valid 3.1 schema because there is a shared base class between all of the Target constraint classes";

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
    ASSERT_FALSE(schema->IsECVersion(ECVersion::V3_1)) << "The schema should not validate as 3.1 because there is a not a shared base class between B and the other Target constraint classes";

    EXPECT_STREQ("A", schema->GetClassCP("TestRel")->GetRelationshipClassCP()->GetSource().GetAbstractConstraint()->GetName().c_str()) << "The Source Constraint's Abstract Constraint attribute should be implicitly set to A.";
    // The Target should be automatically set to CommonClass since it is the common base class between the constraint classes.

    ECEntityClassCP commonClass = schema->GetClassCP("CommonClass")->GetEntityClassCP();

    EXPECT_NE(ECObjectsStatus::Success, schema->GetClassCP("TestRel")->GetRelationshipClassCP()->GetTarget().SetAbstractConstraint(*commonClass)) << "The abstract constraint cannot be set on the target because all of the constraint classes do not derive from that class.";

    EXPECT_EQ(ECObjectsStatus::Success, schema->GetClassP("B")->AddBaseClass(*schema->GetClassP("CommonClass"))) << "Adding the CommonClass as a base class of B should succeed.";
    EXPECT_EQ(ECObjectsStatus::Success, schema->GetClassCP("TestRel")->GetRelationshipClassCP()->GetTarget().SetAbstractConstraint(*commonClass)) << "The abstract constraint should now be able to be CommonClass, since all of the constraint classes derive from it.";

    EXPECT_TRUE(schema->Validate()) << "The schema should now validate successfully.";
    EXPECT_TRUE(schema->IsECVersion(ECVersion::V3_1)) << "The schema should now validate to EC3.1";
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    10/2016
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(SchemaDeserializationConversionTest, TestInheritedAbstractConstraintAttribute)
    {
    {
    Utf8CP schemaXml = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='testSchema' version='01.00' nameSpacePrefix='ts' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "   <ECClass typeName='A' isDomainClass='true'/>"
        "   <ECClass typeName='B' isDomainClass='true'/>"
        "   <ECClass typeName='C' isDomainClass='true'>"
        "       <BaseClass>B</BaseClass>"
        "   </ECClass>"
        "   <ECClass typeName='D' isDomainClass='true'>"
        "       <BaseClass>C</BaseClass>"
        "       <BaseClass>E</BaseClass>"
        "   </ECClass>"
        "   <ECClass typeName='E' isDomainClass='true'>"
        "       <BaseClass>B</BaseClass>"
        "   </ECClass>"
        "   <ECRelationshipClass typeName='DerivedTestRel' isDomainClass='false' strength='referencing' strengthDirection='forward'>"
        "       <BaseClass>TestRel</BaseClass>"
        "       <Source cardinality='(1,1)' polymorphic='True'>"
        "           <Class class='A' />"
        "       </Source>"
        "       <Target cardinality='(1,1)' polymorphic='True'>"
        "           <Class class='D' />"
        "       </Target>"
        "   </ECRelationshipClass>"
        "   <ECRelationshipClass typeName='TestRel' isDomainClass='false' strength='referencing' strengthDirection='forward'>"
        "       <Source cardinality='(1,1)' polymorphic='True'>"
        "           <Class class='A' />"
        "       </Source>"
        "       <Target cardinality='(1,1)' polymorphic='True'>"
        "           <Class class='C' />"
        "           <Class class='E' />"
        "       </Target>"
        "   </ECRelationshipClass>"
        "</ECSchema>";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    SchemaReadStatus status = ECSchema::ReadFromXmlString(schema, schemaXml, *schemaContext);
    ASSERT_EQ(SchemaReadStatus::Success, status);
    ASSERT_TRUE(schema.IsValid());
    ASSERT_TRUE(schema->IsECVersion(3, 1)) << "The schema should be a valid 3.1 schema because there is a shared base class between all of the Target constraint classes";

    EXPECT_STREQ("A", schema->GetClassCP("TestRel")->GetRelationshipClassCP()->GetSource().GetAbstractConstraint()->GetName().c_str()) << "The Source Constraint's Abstract Constraint attribute should be implicitly set to A.";
    EXPECT_STREQ("B", schema->GetClassCP("TestRel")->GetRelationshipClassCP()->GetTarget().GetAbstractConstraint()->GetName().c_str()) << "The Target Constraint's Abstract Constraint attribute should have been automatically set to B because it is a common base class of the constraint classes.";

    EXPECT_STREQ("B", schema->GetClassCP("DerivedTestRel")->GetRelationshipClassCP()->GetTarget().GetAbstractConstraint()->GetName().c_str()) << "The Target Constraint's Abstract Constraint attribute should be inherited.";
    }
    }

END_BENTLEY_ECN_TEST_NAMESPACE