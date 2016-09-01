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
TEST_F(SchemaDeserializationConversionTest, TestMultiplicityConstraintConversion)
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
TEST_F(SchemaDeserializationConversionTest, TestMissingNamespacePrefixConversion)
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
        "<ECSchema schemaName='testSchema2' version='01.00' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "</ECSchema>";

    ECSchemaPtr schema2;
    status = ECSchema::ReadFromXmlString(schema2, schemaXml2, *schemaContext);
    EXPECT_EQ(SchemaReadStatus::Success, status);
    EXPECT_TRUE(schema2.IsValid());
    EXPECT_EQ(schema2->GetName().c_str(), schema2->GetAlias().c_str());
    }

END_BENTLEY_ECN_TEST_NAMESPACE