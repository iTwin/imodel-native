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

    EXPECT_EQ(0, schema->GetClassCP("ARelB")->GetRelationshipClassCP()->GetSource().GetMultiplicity().GetLowerLimit());
    EXPECT_EQ(1, schema->GetClassCP("ARelB")->GetRelationshipClassCP()->GetSource().GetMultiplicity().GetUpperLimit());
    EXPECT_EQ(0, schema->GetClassCP("ARelB")->GetRelationshipClassCP()->GetTarget().GetMultiplicity().GetLowerLimit());
    EXPECT_EQ(UINT_MAX, schema->GetClassCP("ARelB")->GetRelationshipClassCP()->GetTarget().GetMultiplicity().GetUpperLimit());
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
    EXPECT_EQ(schema->GetName().c_str(), schema->GetAlias().c_str());

    Utf8CP schemaXml2 = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='testSchema2' nameSpacePrefix='' version='01.00' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"
        "</ECSchema>";

    ECSchemaPtr schema2;
    status = ECSchema::ReadFromXmlString(schema2, schemaXml2, *schemaContext);
    EXPECT_EQ(SchemaReadStatus::Success, status);
    EXPECT_TRUE(schema2.IsValid());
    EXPECT_EQ(schema2->GetName().c_str(), schema2->GetAlias().c_str());

    Utf8CP schemaXml3 = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='testSchema3' version='01.00' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "</ECSchema>";

    ECSchemaPtr schema3;
    status = ECSchema::ReadFromXmlString(schema3, schemaXml3, *schemaContext);
    EXPECT_EQ(SchemaReadStatus::Success, status);
    EXPECT_TRUE(schema2.IsValid());
    EXPECT_EQ(schema3->GetName().c_str(), schema3->GetAlias().c_str());

    Utf8CP schemaXml4 = "<?xml version='1.0' encoding='UTF-8'?>"
        "<ECSchema schemaName='testSchema4' nameSpacePrefix='' version='01.00' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "</ECSchema>";

    ECSchemaPtr schema4;
    status = ECSchema::ReadFromXmlString(schema4, schemaXml4, *schemaContext);
    EXPECT_EQ(SchemaReadStatus::Success, status);
    EXPECT_TRUE(schema4.IsValid());
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
    ASSERT_STREQ("testSource", schema->GetClassCP("ARelB")->GetRelationshipClassCP()->GetSource().GetRoleLabel().c_str());
    ASSERT_STREQ("testTarget", schema->GetClassCP("ARelB")->GetRelationshipClassCP()->GetTarget().GetRoleLabel().c_str());
    }

END_BENTLEY_ECN_TEST_NAMESPACE