/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"
#include <set>

USING_NAMESPACE_BENTLEY_EC
BEGIN_ECDBUNITTESTS_NAMESPACE


struct SchemaManagerDispatcherTests : ECDbTestFixture
    {};

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaManagerDispatcherTests, Bug556748ImportSeparateSchemaWithMixinWithOnlyMixinBases)
    {
    // iModelCore/DgnPlatform/DgnCore/DgnDbSchema.cpp::PickSchemasToImport drops the BisCore
    // schema from import during the ifc converter run. Previously, iModelCore/ECDb/ECDb/SchemaManagerDispatcher.cpp::GatherRootClasses
    // would never consider that a class with only mixin bases could itself be a mixin. So if you
    // imported a mixin with only mixin bases, class mapping could fail on that class. Mixin hierarchies are mapped first,
    // but since the derived mixin was considered a root class, it is mapped after the mixins and some base classes may not
    // exist when the non-mixin classes are being mapped.
    // This behavior was not noticed when importing the schema containing the mixin simultaneously, because the root mixin class
    // is gathered as a root mixin.
    // That is also why it only showed up when BisCore was not imported but a mixin in class-being-imported was deriving
    // a BisCore mixin
    // If you import schema2 and schema3 simultaneously, this test will pass on the old code

    Utf8CP schema1Xml = R"xml(<?xml version="1.0" encoding="utf-8" ?>
        <ECSchema schemaName="TestSchema1" alias="ts1" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECEntityClass typeName="RootBaseClass">
                <ECProperty propertyName="PropD" typeName="boolean" />
            </ECEntityClass>
        </ECSchema>
    )xml";

    Utf8CP schema2Xml = R"xml(<?xml version="1.0" encoding="utf-8" ?>
        <ECSchema schemaName="TestSchema2" alias="ts2" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECSchemaReference name="CoreCustomAttributes" version="01.00" alias="CoreCA" />
            <ECSchemaReference name="TestSchema1" version="01.00" alias="ts1" />
            <ECEntityClass typeName="IBaseBaseMixin" modifier="Abstract">
                <ECCustomAttributes>
                    <IsMixin xmlns="CoreCustomAttributes.01.00">
                        <AppliesToEntityClass>ts1:RootBaseClass</AppliesToEntityClass>
                    </IsMixin>"
                </ECCustomAttributes>
                <ECProperty propertyName="PropA" typeName="boolean"/>
            </ECEntityClass>
        </ECSchema>
    )xml";

    Utf8CP schema3Xml = R"xml(<?xml version="1.0" encoding="utf-8" ?>
        <ECSchema schemaName="TestSchema3" alias="ts3" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECSchemaReference name="CoreCustomAttributes" version="01.00" alias="CoreCA" />
            <ECSchemaReference name="TestSchema1" version="01.00" alias="ts1" />
            <ECSchemaReference name="TestSchema2" version="01.00" alias="ts2" />
            <ECEntityClass typeName="IBaseMixin" modifier="Abstract">
                <ECCustomAttributes>
                    <IsMixin xmlns="CoreCustomAttributes.01.00">
                        <AppliesToEntityClass>ts1:RootBaseClass</AppliesToEntityClass>
                    </IsMixin>"
                </ECCustomAttributes>
                <BaseClass>ts2:IBaseBaseMixin</BaseClass>
                <ECProperty propertyName="PropB" typeName="boolean"/>
            </ECEntityClass>
            <ECEntityClass typeName="IDerivedMixin" modifier="Abstract">
                <ECCustomAttributes>
                    <IsMixin xmlns="CoreCustomAttributes.01.00">
                        <AppliesToEntityClass>ts1:RootBaseClass</AppliesToEntityClass>
                    </IsMixin>"
                </ECCustomAttributes>
                <BaseClass>IBaseMixin</BaseClass>
                <ECProperty propertyName="PropC" typeName="boolean"/>
            </ECEntityClass>
            <ECEntityClass typeName="ConcreteClass">
                <BaseClass>ts1:RootBaseClass</BaseClass>
                <BaseClass>IDerivedMixin</BaseClass>
                <ECProperty propertyName="PropE" typeName="boolean" />
            </ECEntityClass>
        </ECSchema>
    )xml";

    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("SchemaManagerDispatcher_ImportSchemaWithMixinWithOnlyMixinBases.ecdb"));
    ECSchemaReadContextPtr readContext = ECSchemaReadContext::CreateContext();

    ECSchemaPtr schema1;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema1, schema1Xml, *readContext));
    ASSERT_EQ(BentleyStatus::SUCCESS, m_ecdb.Schemas().ImportSchemas(readContext->GetCache().GetSchemas()));

    ECSchemaPtr schema2;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema2, schema2Xml, *readContext));
    ASSERT_EQ(BentleyStatus::SUCCESS, m_ecdb.Schemas().ImportSchemas(readContext->GetCache().GetSchemas()));

    ECSchemaPtr schema3;
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema3, schema3Xml, *readContext));
    ASSERT_EQ(BentleyStatus::SUCCESS, m_ecdb.Schemas().ImportSchemas(readContext->GetCache().GetSchemas()));
    }

END_ECDBUNITTESTS_NAMESPACE
