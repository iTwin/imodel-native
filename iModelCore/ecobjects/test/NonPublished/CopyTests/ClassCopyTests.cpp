/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include "../../ECObjectsTestPCH.h"
#include "../../TestFixture/TestFixture.h"
#include "BeXml/BeXml.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

struct ClassCopyTest : CopyTestFixture 
    {
    ECClassP m_sourceClass; // This class will live inside of CopyTestFixture::m_sourceSchema
    ECClassP m_targetClass; // This class will live inside of CopyTestFixture::m_targetSchema

    protected:
        void 
        SetUp() override;
        void CopyClass(bool copyReferences);
    };

//=======================================================================================
//! ClassCopyTest
//

// These tests live inside of the SchemaCopyTests file because the CopyClass method is on
// ECSchema and not on ECClass.
//=======================================================================================

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
void ClassCopyTest::SetUp()
    {
    CreateTestSchema();

    EC_ASSERT_SUCCESS(ECSchema::CreateSchema(m_targetSchema, "TargetSchema", "ts", 1, 1, 1));
    ASSERT_TRUE(m_targetSchema.IsValid());

    CopyTestFixture::SetUp();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
void ClassCopyTest::CopyClass(bool copyReferences)
    {

    EC_EXPECT_SUCCESS(m_targetSchema->CopyClass(m_targetClass, *m_sourceClass, copyReferences));
    EXPECT_TRUE(nullptr != m_targetClass);
    EXPECT_NE(m_sourceClass, m_targetClass);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(ClassCopyTest, EntityClassWithBaseClassWithoutCopyingType)
    {
    ECEntityClassP sourceEntity;
    ECEntityClassP baseEntity;
    EC_EXPECT_SUCCESS(m_sourceSchema->CreateEntityClass(sourceEntity, "EntityClass"));
    EC_EXPECT_SUCCESS(m_sourceSchema->CreateEntityClass(baseEntity, "BaseEntity"));
    sourceEntity->AddBaseClass(*baseEntity);

    m_sourceClass = sourceEntity;


    CopyClass(false);

    EXPECT_TRUE(m_targetClass->HasBaseClasses());
    ECClassP baseClass = m_targetClass->GetBaseClasses().front();
    EXPECT_STREQ(m_sourceSchema->GetName().c_str(), baseClass->GetSchema().GetName().c_str());

    EXPECT_TRUE(ECSchema::IsSchemaReferenced(*m_targetSchema, *m_sourceSchema));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(ClassCopyTest, RelationshipClassWithContraintClassesWithoutCopyingType)
    {
    ECRelationshipClassP relClass;
    ECEntityClassP entity1;
    ECEntityClassP entity2;
    EC_EXPECT_SUCCESS(m_sourceSchema->CreateEntityClass(entity1, "Source"));
    EC_EXPECT_SUCCESS(m_sourceSchema->CreateEntityClass(entity2, "Target"));
    EC_EXPECT_SUCCESS(m_sourceSchema->CreateRelationshipClass(relClass, "RelClass", *entity1, "Source", *entity2, "Target"));

    m_sourceClass = relClass;


    CopyClass(false);

    ECRelationshipClassCP targetRelClass = m_targetClass->GetRelationshipClassCP();
    ASSERT_TRUE(nullptr != targetRelClass);
    EXPECT_EQ(1, targetRelClass->GetSource().GetConstraintClasses().size());
    ECClassCP destSourceClass = targetRelClass->GetSource().GetConstraintClasses().front();
    EXPECT_STREQ(m_sourceSchema->GetName().c_str(), destSourceClass->GetSchema().GetName().c_str());

    EXPECT_EQ(1, targetRelClass->GetTarget().GetConstraintClasses().size());
    ECClassCP destTargetClass = targetRelClass->GetTarget().GetConstraintClasses().front();
    EXPECT_STREQ(m_sourceSchema->GetName().c_str(), destTargetClass->GetSchema().GetName().c_str());

    EXPECT_TRUE(ECSchema::IsSchemaReferenced(*m_targetSchema, *m_sourceSchema));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(ClassCopyTest, RelationshipClassWithAbstractContraintWithoutCopyingType)
    {
    ECRelationshipClassP relClass;
    ECEntityClassP entity1;
    ECEntityClassP entity2;
    EC_EXPECT_SUCCESS(m_sourceSchema->CreateEntityClass(entity1, "Source"));
    EC_EXPECT_SUCCESS(m_sourceSchema->CreateEntityClass(entity2, "Target"));
    EC_EXPECT_SUCCESS(m_sourceSchema->CreateRelationshipClass(relClass, "RelClass"));
    EC_EXPECT_SUCCESS(relClass->GetSource().SetAbstractConstraint(*entity1));
    EC_EXPECT_SUCCESS(relClass->GetTarget().SetAbstractConstraint(*entity2));

    m_sourceClass = relClass;


    CopyClass(false);

    ECRelationshipClassCP targetRelClass = m_targetClass->GetRelationshipClassCP();
    ASSERT_TRUE(nullptr != targetRelClass);

    EXPECT_TRUE(targetRelClass->GetSource().IsAbstractConstraintDefined());
    ECClassCP destSourceClass = targetRelClass->GetSource().GetAbstractConstraint();
    EXPECT_STREQ(m_sourceSchema->GetName().c_str(), destSourceClass->GetSchema().GetName().c_str());

    EXPECT_TRUE(targetRelClass->GetTarget().IsAbstractConstraintDefined());
    ECClassCP destTargetClass = targetRelClass->GetTarget().GetAbstractConstraint();
    EXPECT_STREQ(m_sourceSchema->GetName().c_str(), destTargetClass->GetSchema().GetName().c_str());

    EXPECT_TRUE(ECSchema::IsSchemaReferenced(*m_targetSchema, *m_sourceSchema));
    }


//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ClassCopyTest, CopyRelationshipClassWithConstraintClassesInRefSchema)
    {
    Utf8String referenceSchemaString = R"xml(
                <ECSchema schemaName="referenceSchema" alias="rs" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                    <ECEntityClass typeName="House"/>
                    <ECEntityClass typeName="Room" />
                </ECSchema>)xml";

    Utf8String schemaString = R"xml(
                <ECSchema schemaName="testSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                    <ECSchemaReference name="referenceSchema" version="01.00.00" alias="rs"/>
                    <ECRelationshipClass typeName="HouseHasRooms" strengthDirection="Forward" modifier="Abstract">
                        <Source multiplicity="(0..1)" roleLabel="House" polymorphic="False">
                            <Class class="rs:House"/>
                        </Source>
                        <Target multiplicity="(0..*)" roleLabel="Room" polymorphic="False">
                            <Class class="rs:Room"/>
                        </Target>
                    </ECRelationshipClass>
                </ECSchema>)xml";

    auto const schemaContext = ECSchemaReadContext::CreateContext();

    ECSchemaPtr referenceSchema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(referenceSchema, referenceSchemaString.c_str(), *schemaContext));

    ECSchemaPtr copySchemaFrom;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(copySchemaFrom, schemaString.c_str(), *schemaContext));

    ECSchemaPtr copySchemaTo;
    ASSERT_EQ(ECObjectsStatus::Success, ECSchema::CreateSchema(copySchemaTo, "copySchemaTo", "copySchemaTo", 1, 0, 0));

    ECClassP ecclass = copySchemaFrom->GetClassP("HouseHasRooms");
    ASSERT_NE(ecclass, nullptr);
    ASSERT_TRUE(ecclass->IsRelationshipClass());

    // copy relationship class before adding referenced schema
    ECRelationshipClassP ecRel = ecclass->GetRelationshipClassP();
    ECClassP ecCopiedClass;

    ASSERT_EQ(ECObjectsStatus::Success, copySchemaTo->CopyClass(ecCopiedClass, *ecRel, true));
    ASSERT_EQ(ECObjectsStatus::Success, copySchemaTo->DeleteClass(*copySchemaTo->GetClassP(ecRel->GetName().c_str())));
    ASSERT_EQ(ECObjectsStatus::Success, copySchemaTo->RemoveReferencedSchema(*referenceSchema));

    // copy relationship class after adding reference schema
    ASSERT_EQ(ECObjectsStatus::Success, copySchemaTo->AddReferencedSchema(*referenceSchema));
    ASSERT_EQ(ECObjectsStatus::Success, copySchemaTo->CopyClass(ecCopiedClass, *ecRel, true));

    // assert if relationship class is copied
    ECRelationshipClassP ecCopiedRel = ecCopiedClass->GetRelationshipClassP();
    ASSERT_NE(ecCopiedRel, nullptr);
    ASSERT_EQ(ecCopiedRel->GetStrengthDirection(), ECRelatedInstanceDirection::Forward);
    ASSERT_EQ(ecCopiedRel->GetStrength(), StrengthType::Referencing);
    ASSERT_EQ(ecCopiedRel->GetClassModifier(), ECClassModifier::Abstract);

    ASSERT_STREQ(ecCopiedRel->GetSource().GetRoleLabel().c_str(), "House");
    ASSERT_FALSE(ecCopiedRel->GetSource().GetIsPolymorphic());
    ASSERT_TRUE(ecCopiedRel->GetSource().GetMultiplicity().Equals(RelationshipMultiplicity::ZeroOne()));
    ASSERT_EQ(ecCopiedRel->GetSource().GetConstraintClasses().size(), 1);
    ASSERT_EQ(ecCopiedRel->GetSource().GetConstraintClasses().at(0), referenceSchema->GetClassCP("House"));

    ASSERT_STREQ(ecCopiedRel->GetTarget().GetRoleLabel().c_str(), "Room");
    ASSERT_FALSE(ecCopiedRel->GetTarget().GetIsPolymorphic());
    ASSERT_TRUE(ecCopiedRel->GetTarget().GetMultiplicity().Equals(RelationshipMultiplicity::ZeroMany()));
    ASSERT_EQ(ecCopiedRel->GetTarget().GetConstraintClasses().size(), 1);
    ASSERT_EQ(ecCopiedRel->GetTarget().GetConstraintClasses().at(0), referenceSchema->GetClassCP("Room"));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ClassCopyTest, CopyRelationshipClassWithConstraintClassesInRefSchemaAndOriginal)
    {
    Utf8String referenceSchemaString = R"xml(
                <ECSchema schemaName="referenceSchema" alias="rs" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                    <ECEntityClass typeName="House" />
                </ECSchema>)xml";

    Utf8String schemaString = R"xml(
                <ECSchema schemaName="testSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                    <ECSchemaReference name="referenceSchema" version="01.00.00" alias="rs"/>
                    <ECRelationshipClass typeName="HouseHasRooms" strengthDirection="Forward" modifier="Abstract">
                        <Source multiplicity="(0..1)" roleLabel="House" polymorphic="False">
                            <Class class="rs:House"/>
                        </Source>
                        <Target multiplicity="(0..1)" roleLabel="Room" polymorphic="true">
                            <Class class="Room"/>
                        </Target>
                    </ECRelationshipClass>
                    <ECEntityClass typeName="Room" />
                </ECSchema>)xml";

    auto const schemaContext = ECSchemaReadContext::CreateContext();

    ECSchemaPtr referenceSchema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(referenceSchema, referenceSchemaString.c_str(), *schemaContext));

    ECSchemaPtr copySchemaFrom;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(copySchemaFrom, schemaString.c_str(), *schemaContext));

    ECSchemaPtr copySchemaTo;
    ASSERT_EQ(ECObjectsStatus::Success, ECSchema::CreateSchema(copySchemaTo, "copySchemaTo", "copySchemaTo", 1, 0, 0));

    ECClassP ecclass = copySchemaFrom->GetClassP("HouseHasRooms");
    ASSERT_NE(ecclass, nullptr);
    ASSERT_TRUE(ecclass->IsRelationshipClass());

    // copy relationship class before adding referenced schema
    ECRelationshipClassP ecRel = ecclass->GetRelationshipClassP();
    ECClassP ecCopiedClass;

    ASSERT_EQ(ECObjectsStatus::Success, copySchemaTo->CopyClass(ecCopiedClass, *ecRel, false));
    ASSERT_EQ(ECObjectsStatus::Success, copySchemaTo->DeleteClass(*copySchemaTo->GetClassP(ecRel->GetName().c_str())));
    ASSERT_EQ(ECObjectsStatus::Success, copySchemaTo->RemoveReferencedSchema(*referenceSchema));
    ASSERT_EQ(ECObjectsStatus::Success, copySchemaTo->RemoveReferencedSchema(*copySchemaFrom));

    // copy relationship class after adding reference schema
    ASSERT_EQ(ECObjectsStatus::Success, copySchemaTo->AddReferencedSchema(*referenceSchema));
    ASSERT_EQ(ECObjectsStatus::Success, copySchemaTo->CopyClass(ecCopiedClass, *ecRel, false));

    // assert if relationship class is copied
    ECRelationshipClassP ecCopiedRel = ecCopiedClass->GetRelationshipClassP();
    ASSERT_NE(ecCopiedRel, nullptr);
    ASSERT_EQ(ecCopiedRel->GetStrengthDirection(), ECRelatedInstanceDirection::Forward);
    ASSERT_EQ(ecCopiedRel->GetStrength(), StrengthType::Referencing);
    ASSERT_EQ(ecCopiedRel->GetClassModifier(), ECClassModifier::Abstract);

    ASSERT_STREQ(ecCopiedRel->GetSource().GetRoleLabel().c_str(), "House");
    ASSERT_FALSE(ecCopiedRel->GetSource().GetIsPolymorphic());
    ASSERT_TRUE(ecCopiedRel->GetSource().GetMultiplicity().Equals(RelationshipMultiplicity::ZeroOne()));
    ASSERT_EQ(ecCopiedRel->GetSource().GetConstraintClasses().size(), 1);
    ASSERT_EQ(ecCopiedRel->GetSource().GetConstraintClasses().at(0), referenceSchema->GetClassCP("House"));

    ASSERT_STREQ(ecCopiedRel->GetTarget().GetRoleLabel().c_str(), "Room");
    ASSERT_TRUE(ecCopiedRel->GetTarget().GetIsPolymorphic());
    ASSERT_TRUE(ecCopiedRel->GetTarget().GetMultiplicity().Equals(RelationshipMultiplicity::ZeroOne()));
    ASSERT_EQ(ecCopiedRel->GetTarget().GetConstraintClasses().size(), 1);
    ASSERT_EQ(ecCopiedRel->GetTarget().GetConstraintClasses().at(0), copySchemaFrom->GetClassP("Room"));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ClassCopyTest, CopyRelationshipClassWithAbstractConstraintInRefSchema)
    {
    Utf8String referenceSchemaString = R"xml(
        <ECSchema schemaName="referenceSchema" alias="rs" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECEntityClass typeName="BaseSource" />
            <ECEntityClass typeName="BaseTarget" />
            <ECEntityClass typeName="Source">
                <BaseClass>BaseSource</BaseClass>
            </ECEntityClass>
            <ECEntityClass typeName="Target">
                <BaseClass>BaseTarget</BaseClass>
            </ECEntityClass>
        </ECSchema>)xml";

    Utf8String schemaString = R"xml(
        <ECSchema schemaName="testSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="referenceSchema" version="01.00.00" alias="rs" />
            <ECRelationshipClass typeName="RelClass" modifier="None" strength="referencing">
                <Source multiplicity="(0..1)" polymorphic="true" roleLabel="Source" abstractConstraint="rs:BaseSource">
                    <Class class="rs:Source" />
                </Source>
                <Target multiplicity="(0..1)" polymorphic="true" roleLabel="Target" abstractConstraint="rs:BaseTarget">
                    <Class class="rs:Target" />
                </Target>
            </ECRelationshipClass>  
        </ECSchema>)xml";

    auto const schemaContext = ECSchemaReadContext::CreateContext();

    ECSchemaPtr referenceSchema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(referenceSchema, referenceSchemaString.c_str(), *schemaContext));

    ECSchemaPtr copySchemaFrom;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(copySchemaFrom, schemaString.c_str(), *schemaContext));

    ECSchemaPtr copySchemaTo;
    ASSERT_EQ(ECObjectsStatus::Success, ECSchema::CreateSchema(copySchemaTo, "copySchemaTo", "copySchemaTo", 1, 0, 0));

    ECClassP ecclass = copySchemaFrom->GetClassP("RelClass");
    ASSERT_NE(ecclass, nullptr);
    ASSERT_TRUE(ecclass->IsRelationshipClass());

    // copy relationship class before adding referenced schema
    ECRelationshipClassP ecRel = ecclass->GetRelationshipClassP();
    ECClassP ecCopiedClass;

    ASSERT_EQ(ECObjectsStatus::Success, copySchemaTo->CopyClass(ecCopiedClass, *ecRel, true));
    ASSERT_EQ(ECObjectsStatus::Success, copySchemaTo->DeleteClass(*copySchemaTo->GetClassP(ecRel->GetName().c_str())));
    ASSERT_EQ(ECObjectsStatus::Success, copySchemaTo->RemoveReferencedSchema(*referenceSchema));

    // copy relationship class after adding reference schema
    ASSERT_EQ(ECObjectsStatus::Success, copySchemaTo->AddReferencedSchema(*referenceSchema));
    ASSERT_EQ(ECObjectsStatus::Success, copySchemaTo->CopyClass(ecCopiedClass, *ecRel, false));

    // assert the relationship class is copied
    ECRelationshipClassP ecCopiedRel = ecCopiedClass->GetRelationshipClassP();
    ASSERT_NE(ecCopiedRel, nullptr);
    ASSERT_EQ(ecCopiedRel->GetStrengthDirection(), ECRelatedInstanceDirection::Forward);
    ASSERT_EQ(ecCopiedRel->GetStrength(), StrengthType::Referencing);
    ASSERT_EQ(ecCopiedRel->GetClassModifier(), ECClassModifier::None);

    ASSERT_STREQ(ecCopiedRel->GetSource().GetRoleLabel().c_str(), "Source");
    ASSERT_TRUE(ecCopiedRel->GetSource().GetIsPolymorphic());
    ASSERT_TRUE(ecCopiedRel->GetSource().GetMultiplicity().Equals(RelationshipMultiplicity::ZeroOne()));
    ASSERT_EQ(ecCopiedRel->GetSource().GetAbstractConstraint(), referenceSchema->GetClassCP("BaseSource"));
    ASSERT_EQ(ecCopiedRel->GetSource().GetConstraintClasses().size(), 1);
    ASSERT_EQ(ecCopiedRel->GetSource().GetConstraintClasses().at(0), referenceSchema->GetClassCP("Source"));

    ASSERT_STREQ(ecCopiedRel->GetTarget().GetRoleLabel().c_str(), "Target");
    ASSERT_TRUE(ecCopiedRel->GetTarget().GetIsPolymorphic());
    ASSERT_TRUE(ecCopiedRel->GetTarget().GetMultiplicity().Equals(RelationshipMultiplicity::ZeroOne()));
    ASSERT_EQ(ecCopiedRel->GetTarget().GetAbstractConstraint(), referenceSchema->GetClassCP("BaseTarget"));
    ASSERT_EQ(ecCopiedRel->GetTarget().GetConstraintClasses().size(), 1);
    ASSERT_EQ(ecCopiedRel->GetTarget().GetConstraintClasses().at(0), referenceSchema->GetClassCP("Target"));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ClassCopyTest, CopyRelationshipClassWithAbstractConstraintInRefSchemaAndOriginal)
    {
    Utf8String referenceSchemaString = R"xml(
        <ECSchema schemaName="referenceSchema" alias="rs" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECEntityClass typeName="BaseSource" />
            <ECEntityClass typeName="Source">
                <BaseClass>BaseSource</BaseClass>
            </ECEntityClass>
        </ECSchema>)xml";

    Utf8String schemaString = R"xml(
        <ECSchema schemaName="testSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="referenceSchema" version="01.00.00" alias="rs" />
            <ECRelationshipClass typeName="RelClass" modifier="None" strength="referencing">
                <Source multiplicity="(0..1)" polymorphic="true" roleLabel="Source" abstractConstraint="rs:BaseSource">
                    <Class class="rs:Source" />
                </Source>
                <Target multiplicity="(0..1)" polymorphic="true" roleLabel="Target" abstractConstraint="BaseTarget">
                    <Class class="Target" />
                </Target>
            </ECRelationshipClass>  
            <ECEntityClass typeName="BaseTarget" />
            <ECEntityClass typeName="Target">
                <BaseClass>BaseTarget</BaseClass>
            </ECEntityClass>
        </ECSchema>)xml";

    auto const schemaContext = ECSchemaReadContext::CreateContext();

    ECSchemaPtr referenceSchema;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(referenceSchema, referenceSchemaString.c_str(), *schemaContext));

    ECSchemaPtr copySchemaFrom;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(copySchemaFrom, schemaString.c_str(), *schemaContext));

    ECSchemaPtr copySchemaTo;
    ASSERT_EQ(ECObjectsStatus::Success, ECSchema::CreateSchema(copySchemaTo, "copySchemaTo", "copySchemaTo", 1, 0, 0));

    ECClassP ecclass = copySchemaFrom->GetClassP("RelClass");
    ASSERT_NE(ecclass, nullptr);
    ASSERT_EQ(ecclass->IsRelationshipClass(), true);

    // copy relationship class before adding referenced schema
    ECRelationshipClassP ecRel = ecclass->GetRelationshipClassP();
    ECClassP ecCopiedClass;

    ASSERT_EQ(ECObjectsStatus::Success, copySchemaTo->CopyClass(ecCopiedClass, *ecRel, false));
    ASSERT_EQ(ECObjectsStatus::Success, copySchemaTo->DeleteClass(*copySchemaTo->GetClassP(ecclass->GetName().c_str())));
    ASSERT_EQ(ECObjectsStatus::Success, copySchemaTo->RemoveReferencedSchema(*referenceSchema));

    // copy relationship class after adding reference schema
    ASSERT_EQ(ECObjectsStatus::Success, copySchemaTo->AddReferencedSchema(*referenceSchema));
    ASSERT_EQ(ECObjectsStatus::Success, copySchemaTo->CopyClass(ecCopiedClass, *ecRel, false));

    // assert the relationship class is copied
    ECRelationshipClassP ecCopiedRel = ecCopiedClass->GetRelationshipClassP();
    ASSERT_NE(ecCopiedRel, nullptr);
    ASSERT_EQ(ecCopiedRel->GetStrengthDirection(), ECRelatedInstanceDirection::Forward);
    ASSERT_EQ(ecCopiedRel->GetStrength(), StrengthType::Referencing);
    ASSERT_EQ(ecCopiedRel->GetClassModifier(), ECClassModifier::None);

    ASSERT_STREQ(ecCopiedRel->GetSource().GetRoleLabel().c_str(), "Source");
    ASSERT_TRUE(ecCopiedRel->GetSource().GetIsPolymorphic());
    ASSERT_TRUE(ecCopiedRel->GetSource().GetMultiplicity().Equals(RelationshipMultiplicity::ZeroOne()));
    ASSERT_EQ(ecCopiedRel->GetSource().GetAbstractConstraint(), referenceSchema->GetClassCP("BaseSource"));
    ASSERT_EQ(ecCopiedRel->GetSource().GetConstraintClasses().size(), 1);
    ASSERT_EQ(ecCopiedRel->GetSource().GetConstraintClasses().at(0), referenceSchema->GetClassCP("Source"));

    ASSERT_STREQ(ecCopiedRel->GetTarget().GetRoleLabel().c_str(), "Target");
    ASSERT_TRUE(ecCopiedRel->GetTarget().GetIsPolymorphic());
    ASSERT_TRUE(ecCopiedRel->GetTarget().GetMultiplicity().Equals(RelationshipMultiplicity::ZeroOne()));
    ASSERT_EQ(ecCopiedRel->GetTarget().GetAbstractConstraint(), copySchemaFrom->GetClassCP("BaseTarget"));
    ASSERT_EQ(ecCopiedRel->GetTarget().GetConstraintClasses().size(), 1);
    ASSERT_EQ(ecCopiedRel->GetTarget().GetConstraintClasses().at(0), copySchemaFrom->GetClassCP("Target"));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ClassCopyTest, CopyCustomAttributesIncludingReferences)
    {
    Utf8CP schemaString = R"(
        <ECSchema schemaName="testSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECCustomAttributeClass typeName="CustomClass" appliesTo="Any"/>
            <ECEntityClass typeName="EntityClass">
                <ECCustomAttributes>
                    <CustomClass xmlns="testSchema.01.00.00"/>
                </ECCustomAttributes>
            </ECEntityClass>
        </ECSchema>
        )";

    ECSchemaPtr schemaCopyFrom;
    auto const schemaContext = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schemaCopyFrom, schemaString, *schemaContext));

    ASSERT_NE(nullptr, schemaCopyFrom->GetClassCP("EntityClass"));
    ASSERT_NE(nullptr, schemaCopyFrom->GetClassCP("CustomClass"));
    ASSERT_TRUE(schemaCopyFrom->GetClassCP("EntityClass")->GetCustomAttribute("testSchema", "CustomClass").IsValid());
    ASSERT_EQ(schemaCopyFrom->GetClassCP("CustomClass"), &schemaCopyFrom->GetClassCP("EntityClass")->GetCustomAttribute("testSchema", "CustomClass")->GetClass());

    ECSchemaPtr schemaCopyTo;
    ECSchema::CreateSchema(schemaCopyTo, "testSchema", "ts", 2, 0, 0);

    ASSERT_EQ(nullptr, schemaCopyTo->GetClassCP("EntityClass"));
    ASSERT_EQ(nullptr, schemaCopyTo->GetClassCP("CustomClass"));

    ECClassP ecClass;

    EC_EXPECT_SUCCESS(schemaCopyTo->CopyClass(ecClass, *schemaCopyFrom->GetClassCP("EntityClass"), true));

    EXPECT_NE(nullptr, schemaCopyTo->GetClassCP("EntityClass"));
    EXPECT_NE(schemaCopyFrom->GetClassCP("EntityClass"), schemaCopyTo->GetClassCP("EntityClass"));
    EXPECT_NE(nullptr, schemaCopyTo->GetClassCP("CustomClass"));
    EXPECT_NE(schemaCopyFrom->GetClassCP("CustomClass"), schemaCopyTo->GetClassCP("CustomClass"));

    EXPECT_TRUE(schemaCopyTo->GetClassCP("EntityClass")->GetCustomAttribute("testSchema", "CustomClass").IsValid());
    EXPECT_EQ(schemaCopyTo->GetClassCP("CustomClass"), &schemaCopyTo->GetClassCP("EntityClass")->GetCustomAttribute("testSchema", "CustomClass")->GetClass());

    EXPECT_FALSE(ECSchema::IsSchemaReferenced(*schemaCopyTo, *schemaCopyFrom));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ClassCopyTest, CopyCustomAttributesWithReferencedCAClass)
    {
    Utf8CP referenceSchemaString = R"(
        <ECSchema schemaName="referenceSchema" alias="rs" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECCustomAttributeClass typeName="CustomClass" appliesTo="Any"/>
        </ECSchema>
        )";

    Utf8CP schemaString = R"(
        <ECSchema schemaName="testSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="referenceSchema" version="01.00.00" alias="rs"/>
            <ECEntityClass typeName="EntityClass">
                <ECCustomAttributes>
                    <CustomClass xmlns="referenceSchema.01.00.00"/>
                </ECCustomAttributes>
            </ECEntityClass>
        </ECSchema>
        )";

    ECSchemaPtr referenceSchema;
    auto const schemaContext = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(referenceSchema, referenceSchemaString, *schemaContext));

    ASSERT_NE(nullptr, referenceSchema->GetClassCP("CustomClass"));

    ECSchemaPtr schemaCopyFrom;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schemaCopyFrom, schemaString, *schemaContext));

    ASSERT_NE(nullptr, schemaCopyFrom->GetClassCP("EntityClass"));
    ASSERT_TRUE(schemaCopyFrom->GetClassCP("EntityClass")->GetCustomAttribute("referenceSchema", "CustomClass").IsValid());
    ASSERT_EQ(referenceSchema->GetClassCP("CustomClass"), &schemaCopyFrom->GetClassCP("EntityClass")->GetCustomAttribute("referenceSchema", "CustomClass")->GetClass());

    ECSchemaPtr schemaCopyTo;
    ECSchema::CreateSchema(schemaCopyTo, "testSchema", "ts", 2, 0, 0);

    ASSERT_EQ(nullptr, schemaCopyTo->GetClassCP("EntityClass"));

    ECClassP ecClass;

    EC_EXPECT_SUCCESS(schemaCopyTo->CopyClass(ecClass, *schemaCopyFrom->GetClassCP("EntityClass"), true));

    EXPECT_NE(nullptr, schemaCopyTo->GetClassCP("EntityClass"));
    EXPECT_NE(schemaCopyFrom->GetClassCP("EntityClass"), schemaCopyTo->GetClassCP("EntityClass"));
    EXPECT_EQ(nullptr, schemaCopyTo->GetClassCP("CustomClass"));

    EXPECT_TRUE(schemaCopyTo->GetClassCP("EntityClass")->GetCustomAttribute("referenceSchema", "CustomClass").IsValid());
    EXPECT_EQ(referenceSchema->GetClassCP("CustomClass"), &schemaCopyTo->GetClassCP("EntityClass")->GetCustomAttribute("referenceSchema", "CustomClass")->GetClass());

    EXPECT_TRUE(ECSchema::IsSchemaReferenced(*schemaCopyTo, *referenceSchema));
    EXPECT_FALSE(ECSchema::IsSchemaReferenced(*schemaCopyTo, *schemaCopyFrom));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ClassCopyTest, CopyCustomAttributesWithReferencedSourceSchema)
    {
    Utf8CP schemaString = R"(
        <ECSchema schemaName="testSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECCustomAttributeClass typeName="CustomClass" appliesTo="Any"/>
            <ECEntityClass typeName="EntityClass">
                <ECCustomAttributes>
                    <CustomClass xmlns="testSchema.01.00.00"/>
                </ECCustomAttributes>
            </ECEntityClass>
        </ECSchema>
        )";

    ECSchemaPtr schemaCopyFrom;
    auto const schemaContext = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schemaCopyFrom, schemaString, *schemaContext));

    ASSERT_NE(nullptr, schemaCopyFrom->GetClassCP("EntityClass"));
    ASSERT_NE(nullptr, schemaCopyFrom->GetClassCP("CustomClass"));
    ASSERT_TRUE(schemaCopyFrom->GetClassCP("EntityClass")->GetCustomAttribute("testSchema", "CustomClass").IsValid());
    ASSERT_EQ(schemaCopyFrom->GetClassCP("CustomClass"), &schemaCopyFrom->GetClassCP("EntityClass")->GetCustomAttribute("testSchema", "CustomClass")->GetClass());

    ECSchemaPtr schemaCopyTo;
    ECSchema::CreateSchema(schemaCopyTo, "testSchema", "ts", 2, 0, 0);

    ASSERT_EQ(nullptr, schemaCopyTo->GetClassCP("EntityClass"));
    ASSERT_EQ(nullptr, schemaCopyTo->GetClassCP("CustomClass"));

    ECClassP ecClass;

    EC_EXPECT_SUCCESS(schemaCopyTo->CopyClass(ecClass, *schemaCopyFrom->GetClassCP("EntityClass"), false));

    EXPECT_NE(nullptr, schemaCopyTo->GetClassCP("EntityClass"));
    EXPECT_NE(schemaCopyFrom->GetClassCP("EntityClass"), schemaCopyTo->GetClassCP("EntityClass"));
    EXPECT_EQ(nullptr, schemaCopyTo->GetClassCP("CustomClass"));

    EXPECT_TRUE(schemaCopyTo->GetClassCP("EntityClass")->GetCustomAttribute("testSchema", "CustomClass").IsValid());

    EXPECT_TRUE(ECSchema::IsSchemaReferenced(*schemaCopyTo, *schemaCopyFrom));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ClassCopyTest, CopyNavigationPropertyIncludingReferences)
    {
    Utf8CP schemaString = R"**(
        <ECSchema schemaName="testSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECRelationshipClass typeName="HouseHasRooms" strengthDirection="Forward" modifier="None">
                <Source multiplicity="(0..1)" roleLabel="House" polymorphic="False">
                    <Class class="House"/>
                </Source>
                <Target multiplicity="(0..*)" roleLabel="Room" polymorphic="False">
                    <Class class="Room"/>
                </Target>
            </ECRelationshipClass>
            <ECEntityClass typeName="House"/>
            <ECEntityClass typeName="Room">
                <ECNavigationProperty propertyName="NavigationPropertyToHouse" relationshipName="HouseHasRooms" direction="Backward"/>
            </ECEntityClass>
        </ECSchema>
        )**";

    ECSchemaPtr schemaCopyFrom;
    auto const schemaContext = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schemaCopyFrom, schemaString, *schemaContext));

    ECSchemaPtr schemaCopyTo;
    ECSchema::CreateSchema(schemaCopyTo, "testSchema", "ts", 2, 0, 0);

    ECClassP ecClass;

    EC_EXPECT_SUCCESS(schemaCopyTo->CopyClass(ecClass, *schemaCopyFrom->GetClassCP("Room"), true));

    EXPECT_NE(nullptr, schemaCopyTo->GetClassCP("Room"));
    EXPECT_NE(schemaCopyFrom->GetClassCP("Room"), schemaCopyTo->GetClassCP("Room"));
    EXPECT_NE(nullptr, schemaCopyTo->GetClassCP("HouseHasRooms"));
    EXPECT_NE(schemaCopyFrom->GetClassCP("HouseHasRooms"), schemaCopyTo->GetClassCP("HouseHasRooms"));
    EXPECT_NE(nullptr, schemaCopyTo->GetClassCP("House"));
    EXPECT_NE(schemaCopyFrom->GetClassCP("House"), schemaCopyTo->GetClassCP("House"));

    EXPECT_NE(nullptr, schemaCopyTo->GetClassCP("Room")->GetPropertyP("NavigationPropertyToHouse"));
    EXPECT_NE(nullptr, schemaCopyTo->GetClassCP("Room")->GetPropertyP("NavigationPropertyToHouse")->GetAsNavigationProperty());
    EXPECT_EQ(schemaCopyTo->GetClassCP("HouseHasRooms")->GetRelationshipClassCP(), schemaCopyTo->GetClassCP("Room")->GetPropertyP("NavigationPropertyToHouse")->GetAsNavigationProperty()->GetRelationshipClass());

    EXPECT_FALSE(ECSchema::IsSchemaReferenced(*schemaCopyTo, *schemaCopyFrom));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ClassCopyTest, CopyNavigationPropertyOnlyInlcudingRelationshipClass)
    {
    Utf8CP referenceSchemaString = R"(
        <ECSchema schemaName="referenceSchema" alias="rs" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECEntityClass typeName="House"/>
        </ECSchema>
        )";

    Utf8CP schemaString = R"**(
        <ECSchema schemaName="testSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="referenceSchema" version="01.00.00" alias="rs"/>
            <ECRelationshipClass typeName="HouseHasRooms" strengthDirection="Forward" modifier="None">
                <Source multiplicity="(0..1)" roleLabel="House" polymorphic="False">
                    <Class class="rs:House"/>
                </Source>
                <Target multiplicity="(0..*)" roleLabel="Room" polymorphic="False">
                    <Class class="Room"/>
                </Target>
            </ECRelationshipClass>
            <ECEntityClass typeName="Room">
                <ECNavigationProperty propertyName="NavigationPropertyToHouse" relationshipName="HouseHasRooms" direction="Backward"/>
            </ECEntityClass>
        </ECSchema>
        )**";

    ECSchemaPtr referenceSchema;
    auto const schemaContext = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(referenceSchema, referenceSchemaString, *schemaContext));

    ASSERT_NE(nullptr, referenceSchema->GetClassCP("House"));

    ECSchemaPtr schemaCopyFrom;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schemaCopyFrom, schemaString, *schemaContext));

    ECSchemaPtr schemaCopyTo;
    ECSchema::CreateSchema(schemaCopyTo, "testSchema", "ts", 2, 0, 0);
    schemaCopyTo->AddReferencedSchema(*referenceSchema);

    ECClassP ecClass;

    EC_EXPECT_SUCCESS(schemaCopyTo->CopyClass(ecClass, *schemaCopyFrom->GetClassCP("Room"), true));

    EXPECT_NE(nullptr, schemaCopyTo->GetClassCP("Room"));
    EXPECT_NE(schemaCopyFrom->GetClassCP("Room"), schemaCopyTo->GetClassCP("Room"));
    EXPECT_NE(nullptr, schemaCopyTo->GetClassCP("HouseHasRooms"));
    EXPECT_NE(schemaCopyFrom->GetClassCP("HouseHasRooms"), schemaCopyTo->GetClassCP("HouseHasRooms"));
    EXPECT_EQ(nullptr, schemaCopyTo->GetClassCP("House"));

    EXPECT_NE(nullptr, schemaCopyTo->GetClassCP("Room")->GetPropertyP("NavigationPropertyToHouse"));
    EXPECT_NE(nullptr, schemaCopyTo->GetClassCP("Room")->GetPropertyP("NavigationPropertyToHouse")->GetAsNavigationProperty());
    EXPECT_EQ(schemaCopyTo->GetClassCP("HouseHasRooms")->GetRelationshipClassCP(), schemaCopyTo->GetClassCP("Room")->GetPropertyP("NavigationPropertyToHouse")->GetAsNavigationProperty()->GetRelationshipClass());
    ASSERT_EQ(referenceSchema->GetClassCP("House"), schemaCopyTo->GetClassCP("HouseHasRooms")->GetRelationshipClassCP()->GetSource().GetConstraintClasses()[0]);

    EXPECT_FALSE(ECSchema::IsSchemaReferenced(*schemaCopyTo, *schemaCopyFrom));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ClassCopyTest, CopyNavigationPropertyWithReferencedSourceSchema)
    {
    Utf8CP schemaString = R"**(
        <ECSchema schemaName="testSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECRelationshipClass typeName="HouseHasRooms" strengthDirection="Forward" modifier="None">
                <Source multiplicity="(0..1)" roleLabel="House" polymorphic="False">
                    <Class class="House"/>
                </Source>
                <Target multiplicity="(0..*)" roleLabel="Room" polymorphic="False">
                    <Class class="Room"/>
                </Target>
            </ECRelationshipClass>
            <ECEntityClass typeName="House"/>
            <ECEntityClass typeName="Room">
                <ECNavigationProperty propertyName="NavigationPropertyToHouse" relationshipName="HouseHasRooms" direction="Backward"/>
            </ECEntityClass>
        </ECSchema>
        )**";

    ECSchemaPtr schemaCopyFrom;
    auto const schemaContext = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schemaCopyFrom, schemaString, *schemaContext));

    ECSchemaPtr schemaCopyTo;
    ECSchema::CreateSchema(schemaCopyTo, "newSchema", "ts", 2, 0, 0);

    ECClassP ecClass;

    EC_EXPECT_SUCCESS(schemaCopyTo->CopyClass(ecClass, *schemaCopyFrom->GetClassCP("Room"), false));

    EXPECT_NE(nullptr, schemaCopyTo->GetClassCP("Room"));
    EXPECT_NE(schemaCopyFrom->GetClassCP("Room"), schemaCopyTo->GetClassCP("Room"));
    EXPECT_EQ(nullptr, schemaCopyTo->GetClassCP("HouseHasRooms"));
    EXPECT_EQ(nullptr, schemaCopyTo->GetClassCP("House"));

    EXPECT_NE(nullptr, schemaCopyTo->GetClassCP("Room")->GetPropertyP("NavigationPropertyToHouse"));
    EXPECT_NE(nullptr, schemaCopyTo->GetClassCP("Room")->GetPropertyP("NavigationPropertyToHouse")->GetAsNavigationProperty());
    EXPECT_EQ(schemaCopyFrom->GetClassCP("HouseHasRooms")->GetRelationshipClassCP(), schemaCopyTo->GetClassCP("Room")->GetPropertyP("NavigationPropertyToHouse")->GetAsNavigationProperty()->GetRelationshipClass());

    EXPECT_TRUE(ECSchema::IsSchemaReferenced(*schemaCopyTo, *schemaCopyFrom));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ClassCopyTest, CopyNavigationPropertyConstraintsAreNotHeld)
    {
    Utf8CP schemaString = R"**(
        <ECSchema schemaName="testSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECRelationshipClass typeName="HouseHasRooms" strengthDirection="Forward" modifier="None">
                <Source multiplicity="(0..1)" roleLabel="House" polymorphic="False">
                    <Class class="House"/>
                </Source>
                <Target multiplicity="(0..*)" roleLabel="Room" polymorphic="False">
                    <Class class="Room"/>
                </Target>
            </ECRelationshipClass>
            <ECEntityClass typeName="House"/>
            <ECEntityClass typeName="Room">
                <ECNavigationProperty propertyName="NavigationPropertyToHouse" relationshipName="HouseHasRooms" direction="Backward"/>
            </ECEntityClass>
        </ECSchema>
        )**";

    ECSchemaPtr schemaCopyFrom;
    auto const schemaContext = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schemaCopyFrom, schemaString, *schemaContext));

    ECClassP ecClass;

    ECSchemaPtr schemaCopyTo1;
    ECSchema::CreateSchema(schemaCopyTo1, "testSchema", "ts", 1, 0, 0); // schema name and version is same

    EXPECT_EQ(ECObjectsStatus::NotFound, schemaCopyTo1->CopyClass(ecClass, *schemaCopyFrom->GetClassCP("Room"), false));
    EXPECT_EQ(nullptr, schemaCopyTo1->GetClassCP("Room")) << "Partial class not removed on failure";

    EXPECT_EQ(ECObjectsStatus::NotFound, schemaCopyTo1->CopyClass(ecClass, *schemaCopyFrom->GetClassCP("HouseHasRooms"), false));
    EXPECT_EQ(nullptr, schemaCopyTo1->GetClassCP("HouseHasRooms")) << "Partial class not removed on failure";

    EC_EXPECT_SUCCESS(schemaCopyTo1->CopyClass(ecClass, *schemaCopyFrom->GetClassCP("HouseHasRooms"), true));
    EXPECT_FALSE(ECSchema::IsSchemaReferenced(*schemaCopyTo1, *schemaCopyFrom));
    ECRelationshipClassCP relClass = (ECRelationshipClassCP)ecClass;
    EXPECT_NE(schemaCopyFrom->GetClassCP("House"), schemaCopyTo1->GetClassCP("House"));
    ECClassCP roomTo = schemaCopyTo1->GetClassCP("Room");
    EXPECT_NE(schemaCopyFrom->GetClassCP("Room"), roomTo);
    EXPECT_NE(schemaCopyFrom->GetClassCP("HouseHasRooms"), ((NavigationECPropertyP)roomTo->GetPropertyP("NavigationPropertyToHouse"))->GetRelationshipClass());
    EXPECT_EQ(roomTo, relClass->GetTarget().GetAbstractConstraint());
    EXPECT_EQ(schemaCopyTo1->GetClassCP("House"), relClass->GetSource().GetAbstractConstraint());


    ECSchemaPtr schemaCopyTo1a;
    ECSchema::CreateSchema(schemaCopyTo1a, "testSchema", "ts", 1, 3, 3); // schema name and major version is same

    EC_EXPECT_SUCCESS(schemaCopyTo1a->CopyClass(ecClass, *schemaCopyFrom->GetClassCP("Room"), true));
    EXPECT_FALSE(ECSchema::IsSchemaReferenced(*schemaCopyTo1a, *schemaCopyFrom));
    EXPECT_NE(schemaCopyFrom->GetClassCP("House"), schemaCopyTo1a->GetClassCP("House"));
    EXPECT_NE(schemaCopyFrom->GetClassCP("Room"), ecClass);
    ECRelationshipClassCP relClassFrom = (ECRelationshipClassCP)schemaCopyFrom->GetClassCP("HouseHasRooms");
    relClass = (ECRelationshipClassCP)schemaCopyTo1a->GetClassCP("HouseHasRooms");
    EXPECT_NE(relClassFrom, ((NavigationECPropertyP)ecClass->GetPropertyP("NavigationPropertyToHouse"))->GetRelationshipClass());
    EXPECT_EQ(ecClass, relClass->GetTarget().GetAbstractConstraint());
    EXPECT_EQ(schemaCopyTo1a->GetClassCP("House"), relClass->GetSource().GetAbstractConstraint());


    ECSchemaPtr schemaCopyTo2;
    ECSchema::CreateSchema(schemaCopyTo2, "testSchema", "ts", 2, 0, 0); // schema name same, but version differs

    EXPECT_EQ(ECObjectsStatus::NotFound, schemaCopyTo2->CopyClass(ecClass, *schemaCopyFrom->GetClassCP("Room"), false));
    EXPECT_EQ(nullptr, schemaCopyTo2->GetClassCP("Room")) << "Partial class not removed on failure";


    ECSchemaPtr schemaCopyTo3;
    ECSchema::CreateSchema(schemaCopyTo3, "otherSchema", "os", 3, 2, 1); // totally different schema

    EC_EXPECT_SUCCESS(schemaCopyTo3->CopyClass(ecClass, *schemaCopyFrom->GetClassCP("Room"), false));
    EXPECT_EQ(nullptr, schemaCopyTo3->GetClassCP("HouseHasRooms"));
    EXPECT_TRUE(ECSchema::IsSchemaReferenced(*schemaCopyTo3, *schemaCopyFrom));
    EXPECT_FALSE(schemaCopyTo3->GetClassP("Room")->GetPropertyP("NavigationPropertyToHouse")->GetAsNavigationPropertyP()->Verify());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ClassCopyTest, RenameCausesNameClashWithReferences_Relationships)
    {
    Utf8CP schemaString = R"**(
        <ECSchema schemaName="testSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECRelationshipClass typeName="HouseHasRooms" strengthDirection="Forward" modifier="None">
                <Source multiplicity="(0..1)" roleLabel="House" polymorphic="False">
                    <Class class="House"/>
                </Source>
                <Target multiplicity="(0..*)" roleLabel="Room" polymorphic="False">
                    <Class class="Room"/>
                </Target>
            </ECRelationshipClass>
            <ECEntityClass typeName="House"/>
            <ECEntityClass typeName="Room">
                <ECNavigationProperty propertyName="NavigationPropertyToHouse" relationshipName="HouseHasRooms" direction="Backward"/>
            </ECEntityClass>
        </ECSchema>
        )**";

    ECSchemaPtr schemaCopyFrom;
    auto const schemaContext = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schemaCopyFrom, schemaString, *schemaContext));

    ECClassP ecClass;

    ECSchemaPtr schemaCopyTo1;
    ECSchema::CreateSchema(schemaCopyTo1, "testSchema", "ts", 1, 0, 0);

    EXPECT_EQ(ECObjectsStatus::RelationshipConstraintsNotCompatible, schemaCopyTo1->CopyClass(ecClass, *schemaCopyFrom->GetClassCP("HouseHasRooms"), true, "Room"));
    EXPECT_EQ(nullptr, schemaCopyTo1->GetClassCP("Room")) << "Partial class not removed on failure";


    ECSchemaPtr schemaCopyTo1a;
    ECSchema::CreateSchema(schemaCopyTo1a, "testSchema", "ts", 1, 0, 0);

    EXPECT_EQ(ECObjectsStatus::ClassTypeNotCorrect, schemaCopyTo1a->CopyClass(ecClass, *schemaCopyFrom->GetClassCP("Room"), true, "HouseHasRooms"));
    EXPECT_EQ(nullptr, schemaCopyTo1a->GetClassCP("HouseHasRooms")) << "Partial class not removed on failure";
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ClassCopyTest, CopyEnumerationPropertyReferencesOriginalSchema)
    {
    Utf8CP schemaString = R"(
        <ECSchema schemaName="testSchema" alias="ts" version="01.02.03" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECEnumeration typeName="TestEnum" backingTypeName="int"/>
            <ECEntityClass typeName="TestClass">
                <ECProperty propertyName="TestProperty" typeName="TestEnum"/>
            </ECEntityClass>
        </ECSchema>
        )";

    ECSchemaPtr schemaCopyFrom;
    auto const schemaContext = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schemaCopyFrom, schemaString, *schemaContext));

    ECSchemaPtr schemaCopyTo;
    ECSchema::CreateSchema(schemaCopyTo, "newSchema", "ts", 2, 3, 4);

    ECClassP ecClass;

    EC_EXPECT_SUCCESS(schemaCopyTo->CopyClass(ecClass, *schemaCopyFrom->GetClassCP("TestClass"), false));

    EXPECT_NE(nullptr, schemaCopyTo->GetClassCP("TestClass"));
    EXPECT_NE(nullptr, schemaCopyTo->GetClassCP("TestClass")->GetPropertyP("TestProperty"));
    EXPECT_EQ(nullptr, schemaCopyTo->GetEnumerationP("TestEnum"));
    EXPECT_EQ(schemaCopyFrom->GetEnumerationP("TestEnum"), schemaCopyTo->GetClassCP("TestClass")->GetPropertyP("TestProperty")->GetAsPrimitivePropertyP()->GetEnumeration());

    EXPECT_TRUE(ECSchema::IsSchemaReferenced(*schemaCopyTo, *schemaCopyFrom));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ClassCopyTest, CopyEnumerationPropertyReferencesReferenceSchema)
    {
    Utf8CP referenceSchemaString = R"(
        <ECSchema schemaName="referenceSchema" alias="rs" version="03.02.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECEnumeration typeName="TestEnum" backingTypeName="int"/>
        </ECSchema>
        )";

    Utf8CP schemaString = R"(
        <ECSchema schemaName="testSchema" alias="ts" version="01.02.03" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="referenceSchema" version="03.02.01" alias="rs"/>
            <ECEntityClass typeName="TestClass">
                <ECProperty propertyName="TestProperty" typeName="rs:TestEnum"/>
            </ECEntityClass>
        </ECSchema>
        )";

    ECSchemaPtr referenceSchema;
    auto const schemaContext = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(referenceSchema, referenceSchemaString, *schemaContext));

    ECSchemaPtr schemaCopyFrom;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schemaCopyFrom, schemaString, *schemaContext));

    ECSchemaPtr schemaCopyTo;
    ECSchema::CreateSchema(schemaCopyTo, "testSchema", "ts", 2, 3, 4);

    ECClassP ecClass;

    EC_EXPECT_SUCCESS(schemaCopyTo->CopyClass(ecClass, *schemaCopyFrom->GetClassCP("TestClass"), false));

    EXPECT_NE(nullptr, schemaCopyTo->GetClassCP("TestClass"));
    EXPECT_NE(nullptr, schemaCopyTo->GetClassCP("TestClass")->GetPropertyP("TestProperty"));
    EXPECT_EQ(nullptr, schemaCopyTo->GetEnumerationP("TestEnum"));
    EXPECT_EQ(referenceSchema->GetEnumerationP("TestEnum"), schemaCopyTo->GetClassCP("TestClass")->GetPropertyP("TestProperty")->GetAsPrimitivePropertyP()->GetEnumeration());

    EXPECT_FALSE(ECSchema::IsSchemaReferenced(*schemaCopyTo, *schemaCopyFrom));
    EXPECT_TRUE(ECSchema::IsSchemaReferenced(*schemaCopyTo, *referenceSchema));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ClassCopyTest, CopyEnumerationArrayPropertyReferencesOriginalSchema)
    {
    Utf8CP schemaString = R"(
        <ECSchema schemaName="testSchema" alias="ts" version="01.02.03" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECEnumeration typeName="TestEnum" backingTypeName="int"/>
            <ECEntityClass typeName="TestClass">
                <ECArrayProperty propertyName="TestArrayProperty" typeName="TestEnum"/>
            </ECEntityClass>
        </ECSchema>
        )";

    ECSchemaPtr schemaCopyFrom;
    auto const schemaContext = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schemaCopyFrom, schemaString, *schemaContext));

    ECSchemaPtr schemaCopyTo;
    ECSchema::CreateSchema(schemaCopyTo, "testSchema", "ts", 2, 3, 4);

    ECClassP ecClass;

    EXPECT_EQ(ECObjectsStatus::NotFound, schemaCopyTo->CopyClass(ecClass, *schemaCopyFrom->GetClassCP("TestClass"), false));
    EXPECT_EQ(nullptr, schemaCopyTo->GetClassCP("TestClass"));
    EXPECT_EQ(nullptr, schemaCopyTo->GetClassCP("TestEnum"));

    ECSchema::CreateSchema(schemaCopyTo, "newSchema", "ts", 2, 3, 4);
    EC_EXPECT_SUCCESS(schemaCopyTo->CopyClass(ecClass, *schemaCopyFrom->GetClassCP("TestClass"), false));

    EXPECT_NE(nullptr, schemaCopyTo->GetClassCP("TestClass"));
    EXPECT_NE(nullptr, schemaCopyTo->GetClassCP("TestClass")->GetPropertyP("TestArrayProperty"));
    EXPECT_EQ(nullptr, schemaCopyTo->GetEnumerationP("TestEnum"));
    EXPECT_EQ(schemaCopyFrom->GetEnumerationP("TestEnum"), schemaCopyTo->GetClassCP("TestClass")->GetPropertyP("TestArrayProperty")->GetAsPrimitiveArrayPropertyP()->GetEnumeration());

    EXPECT_TRUE(ECSchema::IsSchemaReferenced(*schemaCopyTo, *schemaCopyFrom));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ClassCopyTest, RenameCausesNameClashWithReferences_Enums)
    {
    Utf8CP schemaString = R"(
        <ECSchema schemaName="testSchema" alias="ts" version="01.02.03" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECEnumeration typeName="TestEnum" backingTypeName="int"/>
            <ECEntityClass typeName="TestClass">
                <ECProperty propertyName="TestProperty" typeName="TestEnum"/>
            </ECEntityClass>
        </ECSchema>
        )";

    ECSchemaPtr schemaCopyFrom;
    auto const schemaContext = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schemaCopyFrom, schemaString, *schemaContext));

    ECSchemaPtr schemaCopyTo;
    ECSchema::CreateSchema(schemaCopyTo, "newSchema", "ts", 2, 3, 4);

    ECClassP ecClass;

    EC_EXPECT_SUCCESS(schemaCopyTo->CopyClass(ecClass, *schemaCopyFrom->GetClassCP("TestClass"), false, "TestEnum"));

    EXPECT_EQ(nullptr, schemaCopyTo->GetEnumerationCP("TestEnum"));
    ECClassCP testClass = schemaCopyTo->GetClassCP("TestEnum");
    EXPECT_NE(nullptr, testClass);
    EXPECT_NE(nullptr, testClass->GetPropertyP("TestProperty"));
    EXPECT_EQ(schemaCopyFrom->GetEnumerationP("TestEnum"), testClass->GetPropertyP("TestProperty")->GetAsPrimitiveProperty()->GetEnumeration());

    EXPECT_TRUE(ECSchema::IsSchemaReferenced(*schemaCopyTo, *schemaCopyFrom));

    ECSchemaPtr schemaCopyTo2;
    ECSchema::CreateSchema(schemaCopyTo2, "testSchema", "ts", 2, 3, 4);

    EXPECT_EQ(ECObjectsStatus::NamedItemAlreadyExists, schemaCopyTo2->CopyClass(ecClass, *schemaCopyFrom->GetClassCP("TestClass"), true, "TestEnum"));
    EXPECT_EQ(nullptr, schemaCopyTo2->GetClassCP("TestClass"));
    EXPECT_EQ(nullptr, schemaCopyTo2->GetClassCP("TestEnum"));
    EXPECT_EQ(nullptr, schemaCopyTo2->GetEnumerationCP("TestEnum"));
    EXPECT_TRUE(!ECSchema::IsSchemaReferenced(*schemaCopyTo2, *schemaCopyFrom));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ClassCopyTest, CopyEnumerationArrayPropertyReferencesReferenceSchema)
    {
    Utf8CP referenceSchemaString = R"(
        <ECSchema schemaName="referenceSchema" alias="rs" version="03.02.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECEnumeration typeName="TestEnum" backingTypeName="int"/>
        </ECSchema>
        )";

    Utf8CP schemaString = R"(
        <ECSchema schemaName="testSchema" alias="ts" version="01.02.03" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="referenceSchema" version="03.02.01" alias="rs"/>
            <ECEntityClass typeName="TestClass">
                <ECArrayProperty propertyName="TestArrayProperty" typeName="rs:TestEnum"/>
            </ECEntityClass>
        </ECSchema>
        )";

    ECSchemaPtr referenceSchema;
    auto const schemaContext = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(referenceSchema, referenceSchemaString, *schemaContext));

    ECSchemaPtr schemaCopyFrom;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schemaCopyFrom, schemaString, *schemaContext));

    ECSchemaPtr schemaCopyTo;
    ECSchema::CreateSchema(schemaCopyTo, "testSchema", "ts", 2, 3, 4);

    ECClassP ecClass;

    EC_EXPECT_SUCCESS(schemaCopyTo->CopyClass(ecClass, *schemaCopyFrom->GetClassCP("TestClass"), false));

    EXPECT_NE(nullptr, schemaCopyTo->GetClassCP("TestClass"));
    EXPECT_NE(nullptr, schemaCopyTo->GetClassCP("TestClass")->GetPropertyP("TestArrayProperty"));
    EXPECT_EQ(nullptr, schemaCopyTo->GetEnumerationP("TestEnum"));
    EXPECT_EQ(referenceSchema->GetEnumerationP("TestEnum"), schemaCopyTo->GetClassCP("TestClass")->GetPropertyP("TestArrayProperty")->GetAsPrimitiveArrayPropertyP()->GetEnumeration());

    EXPECT_FALSE(ECSchema::IsSchemaReferenced(*schemaCopyTo, *schemaCopyFrom));
    EXPECT_TRUE(ECSchema::IsSchemaReferenced(*schemaCopyTo, *referenceSchema));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ClassCopyTest, CopyStructPropertyReferencesOriginalSchema)
    {
    Utf8CP schemaString = R"(
        <ECSchema schemaName="testSchema" alias="ts" version="01.02.03" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECStructClass typeName="TestStruct"/>
            <ECEntityClass typeName="TestClass">
                <ECStructProperty propertyName="TestStructProperty" typeName="TestStruct"/>
            </ECEntityClass>
        </ECSchema>
        )";

    ECSchemaPtr schemaCopyFrom;
    auto const schemaContext = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schemaCopyFrom, schemaString, *schemaContext));

    ECSchemaPtr schemaCopyTo;
    ECSchema::CreateSchema(schemaCopyTo, "testSchema", "ts", 2, 3, 4);
    ECClassP ecClass;

    EXPECT_EQ(ECObjectsStatus::NotFound, schemaCopyTo->CopyClass(ecClass, *schemaCopyFrom->GetClassCP("TestClass"), false));
    EXPECT_EQ(nullptr, schemaCopyTo->GetClassCP("TestClass"));
    EXPECT_EQ(nullptr, schemaCopyTo->GetClassCP("TestStruct"));


    ECSchema::CreateSchema(schemaCopyTo, "newSchema", "ts", 2, 3, 4);

    EC_EXPECT_SUCCESS(schemaCopyTo->CopyClass(ecClass, *schemaCopyFrom->GetClassCP("TestClass"), false));

    EXPECT_NE(nullptr, schemaCopyTo->GetClassCP("TestClass"));
    EXPECT_NE(nullptr, schemaCopyTo->GetClassCP("TestClass")->GetPropertyP("TestStructProperty"));
    EXPECT_EQ(nullptr, schemaCopyTo->GetClassCP("TestStruct"));
    EXPECT_EQ(schemaCopyFrom->GetClassCP("TestStruct")->GetStructClassCP(), &schemaCopyTo->GetClassCP("TestClass")->GetPropertyP("TestStructProperty")->GetAsStructPropertyP()->GetType());

    EXPECT_TRUE(ECSchema::IsSchemaReferenced(*schemaCopyTo, *schemaCopyFrom));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ClassCopyTest, RenameCausesNameClashWithReferences_Structs)
    {
    Utf8CP schemaString = R"(
        <ECSchema schemaName="testSchema" alias="ts" version="01.02.03" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECStructClass typeName="TestStruct"/>
            <ECEntityClass typeName="TestClass">
                <ECStructProperty propertyName="TestStructProperty" typeName="TestStruct"/>
            </ECEntityClass>
        </ECSchema>
        )";

    ECSchemaPtr schemaCopyFrom;
    auto const schemaContext = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schemaCopyFrom, schemaString, *schemaContext));

    ECSchemaPtr schemaCopyTo;
    ECSchema::CreateSchema(schemaCopyTo, "newSchema", "ts", 2, 3, 4);

    ECClassP ecClass;

    EC_EXPECT_SUCCESS(schemaCopyTo->CopyClass(ecClass, *schemaCopyFrom->GetClassCP("TestClass"), false, "TestStruct"));

    EXPECT_EQ(nullptr, schemaCopyTo->GetClassCP("TestClass"));
    ECClassCP testClass = schemaCopyTo->GetClassCP("TestStruct");
    EXPECT_NE(nullptr, testClass);
    EXPECT_TRUE(testClass->IsEntityClass());
    EXPECT_NE(nullptr, testClass->GetPropertyP("TestStructProperty"));
    EXPECT_EQ(schemaCopyFrom->GetClassCP("TestStruct")->GetStructClassCP(), &testClass->GetPropertyP("TestStructProperty")->GetAsStructPropertyP()->GetType());

    EXPECT_TRUE(ECSchema::IsSchemaReferenced(*schemaCopyTo, *schemaCopyFrom));

    ECSchemaPtr schemaCopyTo2;
    ECSchema::CreateSchema(schemaCopyTo2, "testSchema", "ts", 2, 3, 4);

    EXPECT_EQ(ECObjectsStatus::ClassTypeNotCorrect, schemaCopyTo2->CopyClass(ecClass, *schemaCopyFrom->GetClassCP("TestClass"), true, "TestStruct"));
    EXPECT_EQ(nullptr, schemaCopyTo2->GetClassCP("TestClass"));
    EXPECT_EQ(nullptr, schemaCopyTo2->GetClassCP("TestStruct"));
    EXPECT_TRUE(!ECSchema::IsSchemaReferenced(*schemaCopyTo2, *schemaCopyFrom));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ClassCopyTest, CopyStructPropertyReferencesReferenceSchema)
    {
    Utf8CP referenceSchemaString = R"(
        <ECSchema schemaName="referenceSchema" alias="rs" version="03.02.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECStructClass typeName="TestStruct"/>
        </ECSchema>
        )";

    Utf8CP schemaString = R"(
        <ECSchema schemaName="testSchema" alias="ts" version="01.02.03" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="referenceSchema" version="03.02.01" alias="rs"/>
            <ECEntityClass typeName="TestClass">
                <ECStructProperty propertyName="TestStructProperty" typeName="rs:TestStruct"/>
            </ECEntityClass>
        </ECSchema>
        )";

    ECSchemaPtr referenceSchema;
    auto const schemaContext = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(referenceSchema, referenceSchemaString, *schemaContext));

    ECSchemaPtr schemaCopyFrom;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schemaCopyFrom, schemaString, *schemaContext));

    ECSchemaPtr schemaCopyTo;
    ECSchema::CreateSchema(schemaCopyTo, "testSchema", "ts", 2, 3, 4);

    ECClassP ecClass;

    EC_EXPECT_SUCCESS(schemaCopyTo->CopyClass(ecClass, *schemaCopyFrom->GetClassCP("TestClass"), false, "TestClass"));

    EXPECT_NE(nullptr, schemaCopyTo->GetClassCP("TestClass"));
    EXPECT_NE(nullptr, schemaCopyTo->GetClassCP("TestClass")->GetPropertyP("TestStructProperty"));
    EXPECT_EQ(nullptr, schemaCopyTo->GetClassCP("TestStruct"));
    EXPECT_EQ(referenceSchema->GetClassCP("TestStruct")->GetStructClassCP(), &schemaCopyTo->GetClassCP("TestClass")->GetPropertyP("TestStructProperty")->GetAsStructPropertyP()->GetType());

    EXPECT_FALSE(ECSchema::IsSchemaReferenced(*schemaCopyTo, *schemaCopyFrom));
    EXPECT_TRUE(ECSchema::IsSchemaReferenced(*schemaCopyTo, *referenceSchema));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ClassCopyTest, CopyStructArrayPropertyReferencesOriginalSchema)
    {
    Utf8CP schemaString = R"(
        <ECSchema schemaName="testSchema" alias="ts" version="01.02.03" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECStructClass typeName="TestStruct"/>
            <ECEntityClass typeName="TestClass">
                <ECStructArrayProperty propertyName="TestStructArrayProperty" typeName="TestStruct"/>
            </ECEntityClass>
        </ECSchema>
        )";

    ECSchemaPtr schemaCopyFrom;
    auto const schemaContext = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schemaCopyFrom, schemaString, *schemaContext));

    ECSchemaPtr schemaCopyTo;
    ECSchema::CreateSchema(schemaCopyTo, "newSchema", "ts", 2, 3, 4);

    ECClassP ecClass;

    EC_EXPECT_SUCCESS(schemaCopyTo->CopyClass(ecClass, *schemaCopyFrom->GetClassCP("TestClass"), false, "TestClass"));

    EXPECT_NE(nullptr, schemaCopyTo->GetClassCP("TestClass"));
    EXPECT_NE(nullptr, schemaCopyTo->GetClassCP("TestClass")->GetPropertyP("TestStructArrayProperty"));
    EXPECT_EQ(nullptr, schemaCopyTo->GetClassCP("TestStruct"));
    EXPECT_EQ(schemaCopyFrom->GetClassCP("TestStruct")->GetStructClassCP(), &schemaCopyTo->GetClassCP("TestClass")->GetPropertyP("TestStructArrayProperty")->GetAsStructArrayPropertyP()->GetStructElementType());

    EXPECT_TRUE(ECSchema::IsSchemaReferenced(*schemaCopyTo, *schemaCopyFrom));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ClassCopyTest, CopyStructArrayPropertyReferencesReferenceSchema)
    {
    Utf8CP referenceSchemaString = R"(
        <ECSchema schemaName="referenceSchema" alias="rs" version="03.02.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECStructClass typeName="TestStruct"/>
        </ECSchema>
        )";

    Utf8CP schemaString = R"(
        <ECSchema schemaName="testSchema" alias="ts" version="01.02.03" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="referenceSchema" version="03.02.01" alias="rs"/>
            <ECEntityClass typeName="TestClass">
                <ECStructArrayProperty propertyName="TestStructArrayProperty" typeName="rs:TestStruct"/>
            </ECEntityClass>
        </ECSchema>
        )";

    ECSchemaPtr referenceSchema;
    auto const schemaContext = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(referenceSchema, referenceSchemaString, *schemaContext));

    ECSchemaPtr schemaCopyFrom;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schemaCopyFrom, schemaString, *schemaContext));

    ECSchemaPtr schemaCopyTo;
    ECSchema::CreateSchema(schemaCopyTo, "testSchema", "ts", 2, 3, 4);

    ECClassP ecClass;

    EC_EXPECT_SUCCESS(schemaCopyTo->CopyClass(ecClass, *schemaCopyFrom->GetClassCP("TestClass"), false, "TestClass"));

    EXPECT_NE(nullptr, schemaCopyTo->GetClassCP("TestClass"));
    EXPECT_NE(nullptr, schemaCopyTo->GetClassCP("TestClass")->GetPropertyP("TestStructArrayProperty"));
    EXPECT_EQ(nullptr, schemaCopyTo->GetClassCP("TestStruct"));
    EXPECT_EQ(referenceSchema->GetClassCP("TestStruct")->GetStructClassCP(), &schemaCopyTo->GetClassCP("TestClass")->GetPropertyP("TestStructArrayProperty")->GetAsStructArrayPropertyP()->GetStructElementType());

    EXPECT_FALSE(ECSchema::IsSchemaReferenced(*schemaCopyTo, *schemaCopyFrom));
    EXPECT_TRUE(ECSchema::IsSchemaReferenced(*schemaCopyTo, *referenceSchema));
    }

END_BENTLEY_ECN_TEST_NAMESPACE
