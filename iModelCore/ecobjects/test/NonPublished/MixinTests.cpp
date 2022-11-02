/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

struct MixinTest : ECTestFixture { };

//---------------------------------------------------------------------------------------
//@bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(MixinTest, MixinClassMayOnlyHaveMixinAsBaseClass_AddBaseClass)
    {
    ECSchemaPtr schema;
    ECEntityClassP entity0;
    ECEntityClassP entity1;
    ECEntityClassP entity2;
    ECEntityClassP mixin0;
    ECEntityClassP mixin1;

    ECSchema::CreateSchema(schema, "NoMixinMixing", "NMM", 1, 1, 1);
    schema->CreateEntityClass(entity0, "Entity0");
    schema->CreateEntityClass(entity1, "Entity1");
    entity1->AddBaseClass(*entity0);
    schema->CreateEntityClass(entity2, "Entity2");
    schema->CreateMixinClass(mixin0, "Mixin0", *entity0);
    schema->CreateMixinClass(mixin1, "Mixin1", *entity0);


    ASSERT_EQ(ECObjectsStatus::BaseClassUnacceptable, mixin0->AddBaseClass(*entity1)) << "Should fail when adding an entity class as a base class for a mixin";
    ASSERT_EQ(ECObjectsStatus::Success, mixin0->AddBaseClass(*mixin1)) << "Should succeed when adding a mixin class as a base class for a mixin";
    ASSERT_EQ(ECObjectsStatus::Success, entity1->AddBaseClass(*mixin0)) << "Should be able to add a mixin as a base class of a normal class if applies to constraint is met";
    ASSERT_EQ(ECObjectsStatus::BaseClassUnacceptable, entity2->AddBaseClass(*mixin0)) << "Shoudl not be able to add a mixin as a base class of a normal class if applies to constraint is not met";
    }

//---------------------------------------------------------------------------------------
//@bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(MixinTest, MixinClassMayOnlyOneBaseClass_AddBaseClass)
    {
    ECSchemaPtr schema;
    ECEntityClassP entity0;
    ECEntityClassP mixin0;
    ECEntityClassP mixin1;
    ECEntityClassP mixin2;

    ECSchema::CreateSchema(schema, "NoMixinMixing", "NMM", 1, 1, 1);
    schema->CreateEntityClass(entity0, "Entity0");
    schema->CreateMixinClass(mixin0, "Mixin0", *entity0);
    schema->CreateMixinClass(mixin1, "Mixin1", *entity0);
    schema->CreateMixinClass(mixin2, "Mixin2", *entity0);

    ASSERT_EQ(ECObjectsStatus::Success, mixin0->AddBaseClass(*mixin1)) << "Should succeed when adding a mixin class as a base class for a mixin";
    ASSERT_EQ(ECObjectsStatus::BaseClassUnacceptable, mixin0->AddBaseClass(*mixin2)) << "Should not be able to add mixin as a base class because the mixin already has a base class.";
    }

//---------------------------------------------------------------------------------------
//@bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(MixinTest, MixinClassMayOnlyOneBaseClass_XmlDeserialization)
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="utf-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECSchemaReference name="CoreCustomAttributes" version="01.00.00" alias="CoreCA" />
            <ECEntityClass typeName="Mixin0" modifier="Abstract">
                <BaseClass>Mixin1</BaseClass>
                <BaseClass>Mixin2</BaseClass>
                <ECCustomAttributes>
                    <IsMixin xmlns="CoreCustomAttributes.01.00">
                    <AppliesToEntityClass>ts:Entity0</AppliesToEntityClass>
                    </IsMixin>
                </ECCustomAttributes>
            </ECEntityClass>
            <ECEntityClass typeName="Mixin1" modifier="Abstract">
                <ECCustomAttributes>
                    <IsMixin xmlns="CoreCustomAttributes.01.00">
                    <AppliesToEntityClass>ts:Entity0</AppliesToEntityClass>
                    </IsMixin>
                </ECCustomAttributes>
            </ECEntityClass>
            <ECEntityClass typeName="Mixin2" modifier="Abstract">
                <ECCustomAttributes>
                    <IsMixin xmlns="CoreCustomAttributes.01.00">
                    <AppliesToEntityClass>ts:Entity0</AppliesToEntityClass>
                    </IsMixin>
                </ECCustomAttributes>
            </ECEntityClass>
            <ECEntityClass typeName="Entity0" />
        </ECSchema>
        )xml";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::InvalidECSchemaXml, ECSchema::ReadFromXmlString(schema, schemaXml, *context));
    }

//---------------------------------------------------------------------------------------
//@bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(MixinTest, MixinBaseClassMustHaveCompatibleAppliesToConstraint_AddBaseClass)
    {
    ECSchemaPtr schema;
    ECEntityClassP entity0;
    ECEntityClassP entity1;
    ECEntityClassP entity2;
    ECEntityClassP mixin0;
    ECEntityClassP mixin1;
    ECEntityClassP mixin2;
    ECEntityClassP mixin3;

    ECSchema::CreateSchema(schema, "NoMixinMixing", "NMM", 1, 1, 1);
    schema->CreateEntityClass(entity0, "Entity0");
    schema->CreateEntityClass(entity1, "Entity1");
    schema->CreateEntityClass(entity2, "Entity2");
    ASSERT_EQ(ECObjectsStatus::Success, entity2->AddBaseClass(*entity1)) << "Test setup failed";
    schema->CreateMixinClass(mixin0, "Mixin0", *entity0);
    schema->CreateMixinClass(mixin1, "Mixin1", *entity1);
    schema->CreateMixinClass(mixin2, "Mixin2", *entity2);
    schema->CreateMixinClass(mixin3, "Mixin3", *entity1);


    ASSERT_EQ(ECObjectsStatus::BaseClassUnacceptable, mixin0->AddBaseClass(*mixin1)) << "Should fail when adding mixin as base class when mixins have 'AppliesTo' constraints that are not in the same hierarchy";
    ASSERT_EQ(ECObjectsStatus::BaseClassUnacceptable, mixin1->AddBaseClass(*mixin2)) << "Should fail when adding mixin as base class when base mixin has 'AppliesTo' constraint which derives from derived mixin 'AppliesTo' constraint";
    ASSERT_EQ(ECObjectsStatus::Success, mixin2->AddBaseClass(*mixin1)) << "Should succeed when adding mixin as base class when base mixin has 'AppliesTo' constaint which is a base class for the derived mixins 'AppliesTo' constraint";
    ASSERT_EQ(ECObjectsStatus::Success, mixin1->AddBaseClass(*mixin3)) << "Shoudl succeed when adding mixin as base class when both mixins have the same 'AppliesTo' constraint";
    }

//---------------------------------------------------------------------------------------
//@bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(MixinTest, MixinClassMayOnlyHaveMixinAsBaseClass_XmlDeserialization)
    {
    Utf8CP schemaXmlBad = R"xml(<?xml version="1.0" encoding="utf-8"?>
        <ECSchema schemaName="BadTestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
        <ECSchemaReference name="CoreCustomAttributes" version="01.00.00" alias="CoreCA" />
          <ECEntityClass typeName="Mixin0" modifier="Abstract">
            <BaseClass>Entity0</BaseClass>
            <ECCustomAttributes>
              <IsMixin xmlns="CoreCustomAttributes.01.00">
                <AppliesToEntityClass>ts:Entity0</AppliesToEntityClass>
              </IsMixin>
            </ECCustomAttributes>
          </ECEntityClass>
          <ECEntityClass typeName="Entity0" />
        </ECSchema>
        )xml";

    Utf8CP schemaXmlGood = R"xml(<?xml version="1.0" encoding="utf-8"?>
        <ECSchema schemaName="GoodTestSchema1" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
        <ECSchemaReference name="CoreCustomAttributes" version="01.00.00" alias="CoreCA" />
          <ECEntityClass typeName="Mixin0" modifier="Abstract">
            <BaseClass>Mixin1</BaseClass>
            <ECCustomAttributes>
              <IsMixin xmlns="CoreCustomAttributes.01.00">
                <AppliesToEntityClass>Entity0</AppliesToEntityClass>
              </IsMixin>
            </ECCustomAttributes>
          </ECEntityClass>
          <ECEntityClass typeName="Mixin1" modifier="Abstract">
            <ECCustomAttributes>
              <IsMixin xmlns="CoreCustomAttributes.01.00">
                <AppliesToEntityClass>ts:Entity0</AppliesToEntityClass>
              </IsMixin>
            </ECCustomAttributes>
          </ECEntityClass>
          <ECEntityClass typeName="Entity0" />
        </ECSchema>
        )xml";

    Utf8CP schemaXmlGood2 = R"xml(
    <ECSchema schemaName="GoodTestSchema2" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
        <ECSchemaReference name='CoreCustomAttributes' version='01.00.00' alias='CoreCA'/>
        <ECEntityClass typeName='MxBase' modifier='Abstract'>
            <ECCustomAttributes>
                <IsMixin xmlns='CoreCustomAttributes.01.00'>
                    <AppliesToEntityClass>Base</AppliesToEntityClass>
                </IsMixin>
            </ECCustomAttributes>
            <ECProperty propertyName='MxBase_Prop' typeName='long' />
        </ECEntityClass>"
        <ECEntityClass typeName='MxA' modifier='Abstract'>
            <ECCustomAttributes>
                <IsMixin xmlns='CoreCustomAttributes.01.00'>
                    <AppliesToEntityClass>Base</AppliesToEntityClass>
                </IsMixin>
            </ECCustomAttributes>
            <BaseClass>MxBase</BaseClass>
            <ECProperty propertyName='MxA_Prop' typeName='long' />
        </ECEntityClass>
        <ECEntityClass typeName='MxB' modifier='Abstract'>
            <ECCustomAttributes>
                <IsMixin xmlns='CoreCustomAttributes.01.00'>
                    <AppliesToEntityClass>Base</AppliesToEntityClass>
                </IsMixin>
            </ECCustomAttributes>
            <BaseClass>MxBase</BaseClass>
            <ECProperty propertyName='MxB_Prop' typeName='long' />
        </ECEntityClass>
        <ECEntityClass typeName="Base">
            <ECProperty propertyName="Base_Prop" typeName="long" />
        </ECEntityClass>
        <ECEntityClass typeName="Child" >
            <BaseClass>Base</BaseClass>
            <BaseClass>MxA</BaseClass>
            <BaseClass>MxB</BaseClass>
            <ECProperty propertyName="Child_Prop" typeName="long" />
        </ECEntityClass>
    </ECSchema>)xml";

    Utf8CP schemaXmlGood3 = "<ECSchema schemaName='GoodTestSchema3' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
        "  <ECSchemaReference name='CoreCustomAttributes' version='01.00.00' alias='CoreCA'/>"
        "  <ECEntityClass typeName='Equipment'  modifier='Abstract'>"
        "      <ECProperty propertyName='Code' typeName='string' />"
        "  </ECEntityClass>"
        "  <ECEntityClass typeName='IEndPoint' modifier='Abstract'>"
        "      <ECCustomAttributes>"
        "          <IsMixin xmlns='CoreCustomAttributes.01.00'>"
        "              <AppliesToEntityClass>Equipment</AppliesToEntityClass>"
        "          </IsMixin>"
        "      </ECCustomAttributes>"
        "      <ECProperty propertyName='www' typeName='long' />"
        "  </ECEntityClass>"
        "  <ECRelationshipClass typeName='CarHasEndPoint' strength='holding' strengthDirection='Forward' modifier='Sealed'>"
        "      <Source multiplicity='(0..1)' polymorphic='False' roleLabel='A'>"
        "         <Class class='Car' />"
        "     </Source>"
        "      <Target multiplicity='(0..N)' polymorphic='True' roleLabel='B' abstractConstraint='IEndPoint'>"
        "        <Class class='IEndPoint' />"
        "        <Class class='Engine' />"
        "        <Class class='Sterring' />"
        "     </Target>"
        "  </ECRelationshipClass>"
        "  <ECEntityClass typeName='Car'>"
        "      <ECProperty propertyName='Name' typeName='string' />"
        "  </ECEntityClass>"
        "  <ECEntityClass typeName='Engine'>"
        "      <BaseClass>Equipment</BaseClass>"
        "      <BaseClass>IEndPoint</BaseClass>"
        "      <ECProperty propertyName='Volumn' typeName='double' />"
        "      <ECNavigationProperty propertyName='Car' relationshipName='CarHasEndPoint' direction='Backward' />"
        "  </ECEntityClass>"
        "  <ECEntityClass typeName='Sterring'>"
        "      <BaseClass>Equipment</BaseClass>"
        "      <BaseClass>IEndPoint</BaseClass>"
        "      <ECProperty propertyName='Type' typeName='string' />"
        "      <ECNavigationProperty propertyName='Car' relationshipName='CarHasEndPoint' direction='Backward' />"
        "  </ECEntityClass>"
        "  <ECEntityClass typeName='Tire'>"
        "      <BaseClass>Equipment</BaseClass>"
        "      <ECProperty propertyName='Diameter' typeName='double' />"
        "  </ECEntityClass>"
        "</ECSchema>";

    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    ECSchemaPtr ecSchema;
    EXPECT_EQ(SchemaReadStatus::InvalidECSchemaXml, ECSchema::ReadFromXmlString(ecSchema, schemaXmlBad, *schemaContext)) << "Deserialization should fail because 'Mixin0' has an entity class as base class";
    EXPECT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(ecSchema, schemaXmlGood, *schemaContext)) << "Deserialization should succeed because 'Mixin0' as a mixin class as a base class";
    EXPECT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(ecSchema, schemaXmlGood2, *schemaContext)) << "Deserialization should succeed";
    EXPECT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(ecSchema, schemaXmlGood3, *schemaContext)) << "Deserialization should succeed";
    }

//---------------------------------------------------------------------------------------
//@bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(MixinTest, MixinBaseClassMustHaveCompatibleAppliesToConstraint_XmlDeserialization)
    {
    Utf8CP schemaXmlBad0 = R"xml(<?xml version="1.0" encoding="utf-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
        <ECSchemaReference name="CoreCustomAttributes" version="01.00.00" alias="CoreCA" />
          <ECEntityClass typeName="Mixin0" modifier="Abstract">
            <BaseClass>Mixin1</BaseClass>
            <ECCustomAttributes>
              <IsMixin xmlns="CoreCustomAttributes.01.00">
                <AppliesToEntityClass>ts:Entity0</AppliesToEntityClass>
              </IsMixin>
            </ECCustomAttributes>
          </ECEntityClass>
          <ECEntityClass typeName="Mixin1" modifier="Abstract">
            <ECCustomAttributes>
              <IsMixin xmlns="CoreCustomAttributes.01.00">
                <AppliesToEntityClass>ts:Entity1</AppliesToEntityClass>
              </IsMixin>
            </ECCustomAttributes>
          </ECEntityClass>
          <ECEntityClass typeName="Entity0" />
          <ECEntityClass typeName="Entity1" />
        </ECSchema>
        )xml";

    Utf8CP schemaXmlBad1 = R"xml(<?xml version="1.0" encoding="utf-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
        <ECSchemaReference name="CoreCustomAttributes" version="01.00.00" alias="CoreCA" />
          <ECEntityClass typeName="Mixin0" modifier="Abstract">
            <BaseClass>Mixin1</BaseClass>
            <ECCustomAttributes>
              <IsMixin xmlns="CoreCustomAttributes.01.00">
                <AppliesToEntityClass>ts:Entity0</AppliesToEntityClass>
              </IsMixin>
            </ECCustomAttributes>
          </ECEntityClass>
          <ECEntityClass typeName="Mixin1" modifier="Abstract">
            <ECCustomAttributes>
              <IsMixin xmlns="CoreCustomAttributes.01.00">
                <AppliesToEntityClass>ts:Entity1</AppliesToEntityClass>
              </IsMixin>
            </ECCustomAttributes>
          </ECEntityClass>
          <ECEntityClass typeName="Entity0" />
          <ECEntityClass typeName="Entity1" >
            <BaseClass>Entity0</BaseClass>
          </ECEntityClass>
        </ECSchema>
        )xml";

    Utf8CP schemaXmlGood = R"xml(<?xml version="1.0" encoding="utf-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
        <ECSchemaReference name="CoreCustomAttributes" version="01.00.00" alias="CoreCA" />
          <ECEntityClass typeName="Mixin0" modifier="Abstract">
            <BaseClass>Mixin1</BaseClass>
            <ECCustomAttributes>
              <IsMixin xmlns="CoreCustomAttributes.01.00">
                <AppliesToEntityClass>ts:Entity0</AppliesToEntityClass>
              </IsMixin>
            </ECCustomAttributes>
          </ECEntityClass>
          <ECEntityClass typeName="Mixin1" modifier="Abstract">
            <ECCustomAttributes>
              <IsMixin xmlns="CoreCustomAttributes.01.00">
                <AppliesToEntityClass>ts:Entity1</AppliesToEntityClass>
              </IsMixin>
            </ECCustomAttributes>
          </ECEntityClass>
          <ECEntityClass typeName="Entity0" >
            <BaseClass>Entity1</BaseClass>
          </ECEntityClass>
          <ECEntityClass typeName="Entity1" />
        </ECSchema>
        )xml";

    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    ECSchemaPtr ecSchema;
    EXPECT_EQ(SchemaReadStatus::InvalidECSchemaXml, ECSchema::ReadFromXmlString(ecSchema, schemaXmlBad0, *schemaContext)) << "Deserialization should fail when a mixin has base class and mixins have 'AppliesTo' constraints that are not in the same hierarchy";
    EXPECT_EQ(SchemaReadStatus::InvalidECSchemaXml, ECSchema::ReadFromXmlString(ecSchema, schemaXmlBad1, *schemaContext)) << "Deserialization should fail when a mixin has base class and base mixin has 'AppliesTo' constraint which derives from derived mixin 'AppliesTo' constraint";
    EXPECT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(ecSchema, schemaXmlGood, *schemaContext)) << "Deserialization should succeed when a mixin has base class and base mixin has 'AppliesTo' constaint which is a base class for the derived mixins 'AppliesTo' constraint";
    }

//---------------------------------------------------------------------------------------
//@bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(MixinTest, IsMixinReturnsTrueOnlyForClassWithAttributeLocallyDefined)
    {
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    SchemaKey key = SchemaKey("CoreCustomAttributes", 1, 0, 0);
    ECSchemaPtr coreCA = context->LocateSchema(key, SchemaMatchType::LatestWriteCompatible);
    ASSERT_TRUE(coreCA.IsValid()) << "Failed to find the 'CoreCustomAttributes' schema";
    ECCustomAttributeClassCP isMixinClass = coreCA->GetClassCP("IsMixin")->GetCustomAttributeClassCP();
    ASSERT_NE(nullptr, isMixinClass) << "Failed to find 'CoreCustomAttributes:IsMixin' CA Class";

    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "Flavors", "LISP", 1, 0, 0);

    schema->AddReferencedSchema(*coreCA);

    ECEntityClassP vanilla;
    ECEntityClassP pecans;
    ECEntityClassP candiedPecans;
    ASSERT_EQ(ECObjectsStatus::Success, schema->CreateEntityClass(vanilla, "VanillaIceCream")) << "Failed to create entity class 'VanillaIceCream'";
    ASSERT_EQ(ECObjectsStatus::Success, schema->CreateEntityClass(pecans, "Pecans")) << "Falied to create mixin class 'Pecans'";
    ASSERT_EQ(ECObjectsStatus::Success, schema->CreateEntityClass(candiedPecans, "CandiedPecans")) << "Failed to create mixin class 'CandiedPecans'";
    candiedPecans->AddBaseClass(*pecans);

    EXPECT_FALSE(pecans->IsMixin()) << "The class 'Pecans' returned true to IsMixin but the IsMixin custom attribute is not applied";
    EXPECT_FALSE(candiedPecans->IsMixin()) << "The class 'CandiedPecans' returned true to IsMixin but the IsMixin custom attribute is not applied";

    IECInstancePtr mixinCA0 = isMixinClass->GetDefaultStandaloneEnabler()->CreateInstance();
    pecans->SetCustomAttribute(*mixinCA0);
    EXPECT_TRUE(pecans->IsMixin()) << "The class 'Pecans' returned false to IsMixin but the IsMixin custom attribute is applied locally";
    EXPECT_FALSE(candiedPecans->IsMixin()) << "The class 'CandiedPecans' returned true to IsMixin but the IsMixin custom attribute is not applied locally";

    IECInstancePtr mixinCA1 = isMixinClass->GetDefaultStandaloneEnabler()->CreateInstance();
    candiedPecans->SetCustomAttribute(*mixinCA1);
    EXPECT_TRUE(candiedPecans->IsMixin()) << "The class 'CandiedPecans' returned false to IsMixin but the IsMixin custom attribute is applied locally";
    }

//---------------------------------------------------------------------------------------
//@bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(MixinTest, TestMixinClass)
    {
    ECSchemaPtr ecSchema;
    ECSchema::CreateSchema(ecSchema, "TestSchema", "ts", 1, 0, 0);

    ECEntityClassP classA;
    ECEntityClassP notSupportedClass;
    ECEntityClassP mixinClass;
    ecSchema->CreateEntityClass(classA, "A");
    ecSchema->CreateEntityClass(notSupportedClass, "NotSupported");

    ASSERT_EQ(ECObjectsStatus::Success, ecSchema->CreateMixinClass(mixinClass, "Mixin", *classA)) << "Failed to create mixin class 'Mixin' with applies to " << classA->GetFullName();
    ASSERT_TRUE(mixinClass->IsMixin()) << "The class '" << mixinClass->GetFullName() << "' is not a mixin class even though success was returned";

    IECInstancePtr mixinCAInstance = mixinClass->GetCustomAttribute("IsMixin");
    ASSERT_TRUE(mixinCAInstance.IsValid()) << "Could not find the IsMixin custom attribute could not be found on the class 'Mixin' even though the IsMixin method returned true.";

    ECValue appliesToClass;
    mixinCAInstance->GetValue(appliesToClass, "AppliesToEntityClass");
    ASSERT_TRUE(appliesToClass.IsString()) << "The value in AppliesToEntityClass is not a string when it should be.";
    EXPECT_STREQ("A", appliesToClass.GetUtf8CP()) << "The AppliesToEntityClass is not the correct qualified name.";

    EXPECT_TRUE(classA->CanApply(*mixinClass)) << "The ECEntityClass '" << classA->GetFullName() << "' does not support the mixin class '" << mixinClass->GetFullName() << "' even though it is the appliesTo class in the mixin.";

    EXPECT_FALSE(notSupportedClass->CanApply(*mixinClass)) << "The ECEntityClass '" << notSupportedClass->GetFullName() << "' supports the mixin class '" << mixinClass->GetFullName() << "' even though it shouldn't.";

    // Adding Class A as a base class will now make the class supported by the mixin
    notSupportedClass->AddBaseClass(*classA);

    EXPECT_TRUE(notSupportedClass->CanApply(*mixinClass)) << "The ECEntityClass '" << notSupportedClass->GetFullName() << "' does not support the mixin class '" << mixinClass->GetFullName() << "' even though it is now derived from the appliesTo class.";
    }

//---------------------------------------------------------------------------------------
//@bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(MixinTest, TestMixinClassInReferencedSchema)
    {
    ECSchemaPtr ecSchema;
    ECSchemaPtr schemaWithMixin;
    ECSchema::CreateSchema(ecSchema, "TestSchema", "ts", 1, 0, 0);
    ECSchema::CreateSchema(schemaWithMixin, "SchemaWithMixin", "mixin", 1, 0, 0);

    ECEntityClassP baseClass;
    ecSchema->CreateEntityClass(baseClass, "BaseClass");

    ECEntityClassP classA;
    ECEntityClassP mixinClass;

    ASSERT_EQ(ECObjectsStatus::Success, schemaWithMixin->CreateMixinClass(mixinClass, "Mixin", *baseClass)) << "Failed to create mixin class 'Mixin' with applies to '" << baseClass->GetFullName() << "' even though the containing schema is now a referenced schema.";
    EXPECT_TRUE(ECSchema::IsSchemaReferenced(*schemaWithMixin, *ecSchema)) << "The CreateMixin method should automatically add a reference to the schema containing the appliesTo class";
    ASSERT_TRUE(mixinClass->IsMixin()) << "The class '" << mixinClass->GetFullName() << "' is not a mixin class even though success was returned";

    IECInstancePtr mixinCAInstance = mixinClass->GetCustomAttribute("IsMixin");
    ASSERT_TRUE(mixinCAInstance.IsValid()) << "Could not find the IsMixin custom attribute could not be found on the class 'Mixin' even though the IsMixin method returned true.";

    ECValue appliesToClass;
    mixinCAInstance->GetValue(appliesToClass, "AppliesToEntityClass");
    ASSERT_TRUE(appliesToClass.IsString()) << "The value in AppliesToEntityClass is not a string when it should be.";
    EXPECT_STREQ("ts:BaseClass", appliesToClass.GetUtf8CP()) << "The AppliesToEntityClass is not the correct qualified name.";

    schemaWithMixin->CreateEntityClass(classA, "A");
    classA->AddBaseClass(*baseClass);

    EXPECT_TRUE(classA->CanApply(*mixinClass)) << "The ECEntityClass '" << classA->GetFullName() << "' does not support the mixin class '" << mixinClass->GetFullName() << "' even though it is the appliesTo class in the mixin.";
    }

//---------------------------------------------------------------------------------------
//@bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(MixinTest, TestFailureWhenMixinClassHasCircular)
    {
    ECSchemaPtr ecSchema;
    ECSchemaPtr refSchema;

    ECSchema::CreateSchema(ecSchema, "TestSchema", "ts", 1, 0, 0);
    ECSchema::CreateSchema(refSchema, "RefSchema", "ref", 1, 0, 0);

    ECEntityClassP appliesTo;
    ECEntityClassP mixinClass;

    refSchema->CreateEntityClass(appliesTo, "AppliesTo");

    EXPECT_EQ(ECObjectsStatus::Success, ecSchema->AddReferencedSchema(*refSchema)) << "Failed to add 'RefSchema' as a reference to 'TestSchema'";
    ecSchema->CreateMixinClass(mixinClass, "Mixin", *appliesTo);

    EXPECT_FALSE(appliesTo->CanApply(*mixinClass)) << "This should be false since the mixin class '" << mixinClass->GetFullName() << "' already references the schema " << refSchema->GetFullSchemaName().c_str() << " and adding the class as a mixin to the '" << appliesTo->GetFullName() << "' class would create a circular reference.";
    }

//---------------------------------------------------------------------------------------
//@bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(MixinTest, RelationshipConstraints_MixinsNarrowByAppliesToConstraint)
    {
    ECSchemaPtr maceSchema;
    ECSchema::CreateSchema(maceSchema, "MixinsAsConstraintEndpoints", "MACE", 1, 9, 92);

    ECEntityClassP baseSourceConstraint;
    maceSchema->CreateEntityClass(baseSourceConstraint, "BaseSource");
    ECEntityClassP baseTargetConstraint;
    maceSchema->CreateEntityClass(baseTargetConstraint, "BaseTarget");
    ECRelationshipClassP baseRelClass;
    maceSchema->CreateRelationshipClass(baseRelClass, "BaseRel", *baseSourceConstraint, "Source", *baseTargetConstraint, "Target");

    ECEntityClassP mixinSource;
    maceSchema->CreateMixinClass(mixinSource, "MixinSource", *baseSourceConstraint);

    ECRelationshipClassP relClassWithMixin;
    maceSchema->CreateRelationshipClass(relClassWithMixin, "MixinRel", *mixinSource, "Source", *mixinSource, "Target");
    EXPECT_EQ(ECObjectsStatus::BaseClassUnacceptable, relClassWithMixin->AddBaseClass(*baseRelClass)) <<
        "Should not have been able to add base relationship because target constraint is a mixin whose apply to constraint isn't compatible with the base relationship";

    relClassWithMixin->GetTarget().RemoveConstraintClasses();
    relClassWithMixin->GetTarget().AddClass(*baseTargetConstraint);
    EXPECT_EQ(ECObjectsStatus::Success, relClassWithMixin->AddBaseClass(*baseRelClass)) << 
        "Should have been able to add base relationship because source constraint is a mixin whose apply to constraint is compatible with the base constraint";
    
    ECEntityClassP sourceConstraint;
    maceSchema->CreateEntityClass(sourceConstraint, "Source");
    sourceConstraint->AddBaseClass(*baseSourceConstraint);

    ECEntityClassP mixinSource2;
    maceSchema->CreateMixinClass(mixinSource2, "MixinSource2", *sourceConstraint);
    EXPECT_EQ(ECObjectsStatus::Success, mixinSource2->AddBaseClass(*mixinSource));

    ECRelationshipClassP relClass2;
    maceSchema->CreateRelationshipClass(relClass2, "MixinRel2", *mixinSource2, "Source2", *baseTargetConstraint, "Target");
    EXPECT_EQ(ECObjectsStatus::Success, relClass2->AddBaseClass(*relClassWithMixin)) << 
        "Should have been able to add base rel because source constraint is a mixin whose applies to constraint narrows the base constraint";

    // TODO: if the base constraint class is a mixin then the derived mixin must sub class it
    //ECEntityClassP mixinSource3;
    //maceSchema->CreateMixinClass(mixinSource3, "MixinSource3", *baseSourceConstraint);
    //ECRelationshipClassP relClass3;
    //maceSchema->CreateRelationshipClass(relClass3, "MixinRel3", *mixinSource3, "Source3", *baseTargetConstraint, "Target");
    //EXPECT_EQ(ECObjectsStatus::BaseClassUnacceptable, relClass2->AddBaseClass(*relClassWithMixin)) <<
    //    "Should not have been able to add base rel because source constraint is a mixin whose applies to constraint narrows the base constraint";
    }

//---------------------------------------------------------------------------------------
//@bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(MixinTest, RelationshipConstraints_MixinsNarrowByMixinInheritance)
    {
    ECSchemaPtr maceSchema;
    ECSchema::CreateSchema(maceSchema, "MixinsAsConstraintEndpoints", "MACE", 2, 0, 01);

    ECEntityClassP baseSourceConstraint;
    maceSchema->CreateEntityClass(baseSourceConstraint, "BaseSource");
    ECEntityClassP baseTargetConstraint;
    maceSchema->CreateEntityClass(baseTargetConstraint, "BaseTarget");
    ECRelationshipClassP baseRelClass;
    maceSchema->CreateRelationshipClass(baseRelClass, "BaseRel", *baseSourceConstraint, "Source", *baseTargetConstraint, "Target");

    ECEntityClassP mixinSource;
    maceSchema->CreateMixinClass(mixinSource, "MixinSource", *baseSourceConstraint);
    ECEntityClassP mixinSource2;
    maceSchema->CreateMixinClass(mixinSource2, "MixinSource2", *baseSourceConstraint);
    mixinSource2->AddBaseClass(*mixinSource);

    ECRelationshipClassP relClassWithMixin;
    maceSchema->CreateRelationshipClass(relClassWithMixin, "MixinRel", *mixinSource2, "Source", *baseTargetConstraint, "Target");
    EXPECT_EQ(ECObjectsStatus::Success, relClassWithMixin->AddBaseClass(*baseRelClass)) <<
        "Should have been able to add base relationship because source constraint is a mixin whose apply to constraint is compatible with the base constraint";


    ECRelationshipClassP relClass2;
    maceSchema->CreateRelationshipClass(relClass2, "MixinRel2", *mixinSource, "Source2", *baseTargetConstraint, "Target");
    // TODO: if the base constraint class is a mixin then the derived mixin must sub class it
    //EXPECT_EQ(ECObjectsStatus::BaseClassUnacceptable, relClass2->AddBaseClass(*relClassWithMixin)) <<
    //    "Should not have been able to add base rel because source constraint is a mixin which is a base class of the base source constraint mixin";

    relClassWithMixin->RemoveBaseClass(*baseRelClass);
    relClass2->RemoveBaseClass(*relClassWithMixin);
    relClass2->AddBaseClass(*baseRelClass);
    EXPECT_EQ(ECObjectsStatus::Success, relClassWithMixin->AddBaseClass(*relClass2)) <<
        "Should have been able to add base relationship because source constraint is a mixin that derives from the base constraint class";

    }

//---------------------------------------------------------------------------------------
//@bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(MixinTest, RelationshipConstraints_MixinsNarrowByAppliesToConstraint_XmlDeserialization)
    {
    Utf8CP schemaXmlBad = R"xml(<?xml version="1.0" encoding="utf-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
          <ECSchemaReference name="CoreCustomAttributes" version="01.00.00" alias="CoreCA" />
          <ECEntityClass typeName="Mixin0" modifier="Abstract">
            <ECCustomAttributes>
              <IsMixin xmlns="CoreCustomAttributes.01.00">
                <AppliesToEntityClass>ts:Entity1</AppliesToEntityClass>
              </IsMixin>
            </ECCustomAttributes>
          </ECEntityClass>
          <ECEntityClass typeName="Mixin1" modifier="Abstract">
            <ECCustomAttributes>
              <IsMixin xmlns="CoreCustomAttributes.01.00">
                <AppliesToEntityClass>ts:Entity2</AppliesToEntityClass>
              </IsMixin>
            </ECCustomAttributes>
          </ECEntityClass>
          <ECEntityClass typeName="Entity0" />
          <ECEntityClass typeName="Entity1">
            <BaseClass>Entity0</BaseClass>
          </ECEntityClass>
          <ECEntityClass typeName="Entity2" />
          <ECRelationshipClass typeName="BaseRel" displayLabel="Base Rel" modifier="None" strength="referencing">
              <Source multiplicity="(0..*)" roleLabel="references" polymorphic="true">
                  <Class class="Entity0"/>
              </Source>
              <Target multiplicity="(0..1)" roleLabel="references" polymorphic="true">
                  <Class class="Entity2"/>
              </Target>
          </ECRelationshipClass>
          <ECRelationshipClass typeName="DerivedRel" displayLabel="Derived Rel" modifier="None" strength="referencing">
              <BaseClass>BaseRel</BaseClass>
              <Source multiplicity="(0..*)" roleLabel="references" polymorphic="true">
                  <Class class="Mixin1"/>
              </Source>
              <Target multiplicity="(0..1)" roleLabel="references" polymorphic="true">
                  <Class class="Entity2"/>
              </Target>
          </ECRelationshipClass>
        </ECSchema>
        )xml";

    Utf8CP schemaXmlGood = R"xml(<?xml version="1.0" encoding="utf-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
          <ECSchemaReference name="CoreCustomAttributes" version="01.00.00" alias="CoreCA" />
          <ECEntityClass typeName="Mixin0" modifier="Abstract">
            <ECCustomAttributes>
              <IsMixin xmlns="CoreCustomAttributes.01.00">
                <AppliesToEntityClass>ts:Entity1</AppliesToEntityClass>
              </IsMixin>
            </ECCustomAttributes>
          </ECEntityClass>
          <ECEntityClass typeName="Mixin1" modifier="Abstract">
            <ECCustomAttributes>
              <IsMixin xmlns="CoreCustomAttributes.01.00">
                <AppliesToEntityClass>ts:Entity2</AppliesToEntityClass>
              </IsMixin>
            </ECCustomAttributes>
          </ECEntityClass>
          <ECEntityClass typeName="Entity0" />
          <ECEntityClass typeName="Entity1">
            <BaseClass>Entity0</BaseClass>
          </ECEntityClass>
          <ECEntityClass typeName="Entity2" />
          <ECRelationshipClass typeName="BaseRel" displayLabel="Base Rel" modifier="None" strength="referencing">
              <Source multiplicity="(0..*)" roleLabel="references" polymorphic="true">
                  <Class class="Entity0"/>
              </Source>
              <Target multiplicity="(0..1)" roleLabel="references" polymorphic="true">
                  <Class class="Entity2"/>
              </Target>
          </ECRelationshipClass>
          <ECRelationshipClass typeName="DerivedRel" displayLabel="Derived Rel" modifier="None" strength="referencing">
              <BaseClass>BaseRel</BaseClass>
              <Source multiplicity="(0..*)" roleLabel="references" polymorphic="true">
                  <Class class="Mixin0"/>
              </Source>
              <Target multiplicity="(0..1)" roleLabel="references" polymorphic="true">
                  <Class class="Entity2"/>
              </Target>
          </ECRelationshipClass>
        </ECSchema>
        )xml";


    ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
    ECSchemaPtr ecSchema;
    EXPECT_EQ(SchemaReadStatus::InvalidECSchemaXml, ECSchema::ReadFromXmlString(ecSchema, schemaXmlBad, *schemaContext)) << "Deserialization should fail because 'Mixin1' applies to 'Entity2' but is used on a constraint which must narrow 'Entity0'";
    EXPECT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(ecSchema, schemaXmlGood, *schemaContext)) << "Deserialization should succeed because 'Mixin0' applies to 'Entity1' but is used on a constraint which must narrow 'Entity0', 'Entity1' derives from 'Entity0'";
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(MixinTest, SerializeStandaloneMixin)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "ExampleSchema", "ex", 3, 1, 0, ECVersion::Latest);

    ECEntityClassP entityClass;
    schema->CreateEntityClass(entityClass, "ExampleEntity");
    ECEntityClassP mixin;
    schema->CreateMixinClass(mixin, "ExampleMixin", *entityClass);
    mixin->SetClassModifier(ECClassModifier::Abstract);

    Json::Value schemaJson;
    EXPECT_TRUE(mixin->ToJson(schemaJson, true));

    Json::Value testDataJson;
    BeFileName testDataFile(ECTestFixture::GetTestDataPath(L"ECJson/StandaloneMixin.ecschema.json"));
    auto readJsonStatus = ECTestUtility::ReadJsonInputFromFile(testDataJson, testDataFile);
    ASSERT_EQ(BentleyStatus::SUCCESS, readJsonStatus);

    EXPECT_TRUE(ECTestUtility::JsonDeepEqual(schemaJson, testDataJson)) << ECTestUtility::JsonSchemasComparisonString(schemaJson, testDataJson);
    }

END_BENTLEY_ECN_TEST_NAMESPACE

