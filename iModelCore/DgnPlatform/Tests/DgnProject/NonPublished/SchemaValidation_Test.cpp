/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#include "../TestFixture/DgnDbTestFixtures.h"
#include <DgnPlatform/PhysicalMaterialDomain.h>
#include <DgnPlatform/FunctionalDomain.h>

USING_NAMESPACE_BENTLEY_EC

struct SchemaValidationTest : DgnDbTestFixture
    {
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                Eimantas.Morkunas                    03/19
//---------------------------------------------------------------------------------------
TEST_F(SchemaValidationTest, ValidateCoreSchemas)
    {
    SetupSeedProject();

    // BisCore and Generic schemas should be imported by default
    FunctionalDomain::GetDomain().ImportSchema(*m_db);
    PhysicalMaterialDomain::GetDomain().ImportSchema(*m_db);

    ECSchemaValidator validator;
    Utf8String coreSchemaNames[] = { "BisCore", "Functional", "Generic", "PhysicalMaterial" };

    for (Utf8StringCR schemaName : coreSchemaNames)
        {
        ECSchemaCP schema = m_db->Schemas().GetSchema(schemaName);
        ASSERT_NE(schema, nullptr) << "Failed to load '" << schemaName << "' schema.";
        EXPECT_TRUE(validator.Validate(*schema)) << "Schema '" << schema->GetFullSchemaName() << "' failed validation.";
        }
    }
