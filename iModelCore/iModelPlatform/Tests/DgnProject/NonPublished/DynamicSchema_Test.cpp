/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "../TestFixture/DgnDbTestFixtures.h"

USING_NAMESPACE_BENTLEY_EC

struct DynamicSchemaTest : DgnDbTestFixture
    {
    BeFileName ImportSchema(Utf8CP schemaName, Utf8CP schemaXml);
    void TestSchemaUpgrade(Utf8CP schemaName, BeFileNameCR fileName,Utf8CP updatedSchemaXml, SchemaStatus expectedImportStatus, void (*validateSchemaImport)(DgnDbPtr));
    };

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BeFileName DynamicSchemaTest::ImportSchema(Utf8CP schemaName, Utf8CP schemaXml)
    {
    SetupSeedProject();
    BeFileName fileName = m_db->GetFileName();
    SaveDb();
    CloseDb();

    DbResult result = BE_SQLITE_OK;

    m_db = DgnDb::OpenDgnDb(&result, fileName, DgnDb::OpenParams(DgnDb::OpenMode::ReadWrite));
    EXPECT_TRUE(result == BE_SQLITE_OK);
    EXPECT_FALSE(m_db->Schemas().ContainsSchema(schemaName));

    ECSchemaPtr schema = nullptr;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    context->AddSchemaLocater(m_db->GetSchemaLocater());

    SchemaReadStatus schemaStatus = ECSchema::ReadFromXmlString(schema, schemaXml, *context);
    EXPECT_EQ(SchemaReadStatus::Success, schemaStatus);
    EXPECT_EQ(true, schema->IsDynamicSchema());

    SchemaStatus schemaImportStat = m_db->ImportSchemas(context->GetCache().GetSchemas());
    EXPECT_EQ(SchemaStatus::Success, schemaImportStat);
    m_db->SaveChanges("Imported Test schema");
    CloseDb();

    return fileName;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void DynamicSchemaTest::TestSchemaUpgrade(Utf8CP schemaName, BeFileNameCR fileName, Utf8CP updatedSchemaXml, SchemaStatus expectedImportStatus, void (*validateSchemaImport)(DgnDbPtr))
    {
    DbResult result = BE_SQLITE_OK;

    m_db = DgnDb::OpenDgnDb(&result, fileName, DgnDb::OpenParams(DgnDb::OpenMode::ReadWrite));
    EXPECT_TRUE(result == BE_SQLITE_OK);
    EXPECT_TRUE(m_db->Schemas().ContainsSchema(schemaName));

    ECSchemaPtr updatedSchema = nullptr;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    context->AddSchemaLocater(m_db->GetSchemaLocater());
    
    auto schemaStatus = ECSchema::ReadFromXmlString(updatedSchema, updatedSchemaXml, *context);
    ASSERT_EQ(SchemaReadStatus::Success, schemaStatus);


    SchemaStatus schemaImportStat = SchemaStatus::Success;
    {
    DISABLE_ASSERTS
    schemaImportStat = m_db->ImportSchemas(context->GetCache().GetSchemas());
    }
    ASSERT_EQ(expectedImportStatus, schemaImportStat);
    m_db->SaveChanges("Imported updated Test schema");

    validateSchemaImport(m_db);
    CloseDb();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(DynamicSchemaTest, CanImportDynamicSchemaWithoutVersionChange)
    {
    Utf8CP schemaXml = R"xml(
        <ECSchema schemaName="Asset" alias="asset" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="CoreCustomAttributes" version="01.00.03" alias="CoreCA"/>
            <ECCustomAttributes>
                <DynamicSchema xmlns="CoreCustomAttributes.01.00.03"/>
            </ECCustomAttributes>
        </ECSchema>
        )xml";
    auto fileName = ImportSchema("Asset", schemaXml);

    Utf8CP updatedSchemaXml = R"xml(
        <ECSchema schemaName="Asset" alias="asset" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="BisCore" version="01.00.14" alias="bis"/>
            <ECSchemaReference name="CoreCustomAttributes" version="01.00.03" alias="CoreCA"/>
            <ECCustomAttributes>
                <DynamicSchema xmlns="CoreCustomAttributes.01.00.03"/>
            </ECCustomAttributes>
            <ECEntityClass typeName="Pipe">
                <BaseClass>bis:PhysicalElement</BaseClass>
            </ECEntityClass>
        </ECSchema>
        )xml";
    TestSchemaUpgrade("Asset", fileName, updatedSchemaXml, SchemaStatus::Success, [](DgnDbPtr db) {
        EXPECT_TRUE(db->Schemas().ContainsSchema("Asset"));
        ECSchemaCP assetSchema = db->Schemas().GetSchema("Asset");
        ASSERT_NE(assetSchema, nullptr);
        EXPECT_TRUE(assetSchema->IsDynamicSchema()) << "Schema is dynamic schema.";
        ECClassCP ecClass = assetSchema->GetClassCP("Pipe");
        ASSERT_NE(ecClass, nullptr);
        });
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(DynamicSchemaTest, VersionMustBeUpdatedIfDynamicSchemaRemoved)
    {
    Utf8CP schemaXml = R"xml(
        <ECSchema schemaName="Asset" alias="asset" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="BisCore" version="01.00.14" alias="bis"/>
            <ECSchemaReference name="CoreCustomAttributes" version="01.00.03" alias="CoreCA"/>
            <ECCustomAttributes>
                <DynamicSchema xmlns="CoreCustomAttributes.01.00.03"/>
            </ECCustomAttributes>
        </ECSchema>
        )xml";
    auto fileName = ImportSchema("Asset", schemaXml);

    Utf8CP updatedSchemaXml1 = R"xml(
        <ECSchema schemaName="Asset" alias="asset" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="BisCore" version="01.00.14" alias="bis"/>
            <ECSchemaReference name="CoreCustomAttributes" version="01.00.03" alias="CoreCA"/>
        </ECSchema>
        )xml";
    TestSchemaUpgrade("Asset", fileName, updatedSchemaXml1, SchemaStatus::Success, [](DgnDbPtr db){
        EXPECT_TRUE(db->Schemas().ContainsSchema("Asset"));
        ECSchemaCP assetSchema = db->Schemas().GetSchema("Asset");
        ASSERT_NE(assetSchema, nullptr);
        EXPECT_TRUE(assetSchema->IsDynamicSchema()) << "Schema is dynamic schema as updated schema was not imported.";
        });

    Utf8CP updatedSchemaXml2 = R"xml(
        <ECSchema schemaName="Asset" alias="asset" version="1.0.1" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="BisCore" version="01.00.14" alias="bis"/>
            <ECSchemaReference name="CoreCustomAttributes" version="01.00.03" alias="CoreCA"/>
        </ECSchema>
        )xml";
    TestSchemaUpgrade("Asset", fileName, updatedSchemaXml2, SchemaStatus::Success, [](DgnDbPtr db) {
        EXPECT_TRUE(db->Schemas().ContainsSchema("Asset"));
        ECSchemaCP updatedAssetSchema = db->Schemas().GetSchema("Asset");
        ASSERT_NE(updatedAssetSchema, nullptr);
        EXPECT_FALSE(updatedAssetSchema->IsDynamicSchema()) << "Schema is not dynamic after update.";
        });
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(DynamicSchemaTest, ImportingDynamicSchemaWithECDbMapRefRequiresVersionChange)
    {
    Utf8CP schemaXml = R"xml(
        <ECSchema schemaName="Asset" alias="asset" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="BisCore" version="01.00.14" alias="bis"/>
            <ECSchemaReference name="CoreCustomAttributes" version="01.00.03" alias="CoreCA"/>
            <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
            <ECCustomAttributes>
                <DynamicSchema xmlns="CoreCustomAttributes.01.00.03"/>
            </ECCustomAttributes>
        </ECSchema>
        )xml";
    auto fileName = ImportSchema("Asset", schemaXml);

    Utf8CP updatedSchemaXml1 = R"xml(
        <ECSchema schemaName="Asset" alias="asset" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="BisCore" version="01.00.14" alias="bis"/>
            <ECSchemaReference name="CoreCustomAttributes" version="01.00.03" alias="CoreCA"/>
            <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
            <ECCustomAttributes>
                <DynamicSchema xmlns="CoreCustomAttributes.01.00.03"/>
            </ECCustomAttributes>
            <ECEntityClass typeName="Pipe">
                <BaseClass>bis:PhysicalElement</BaseClass>
            </ECEntityClass>
        </ECSchema>
        )xml";
    TestSchemaUpgrade("Asset", fileName, updatedSchemaXml1, SchemaStatus::Success, [](DgnDbPtr db) {
        EXPECT_TRUE(db->Schemas().ContainsSchema("Asset"));
        ECSchemaCP assetSchema = db->Schemas().GetSchema("Asset");
        ASSERT_NE(assetSchema, nullptr);
        EXPECT_TRUE(assetSchema->IsDynamicSchema()) << "Schema is dynamic schema.";
        ECClassCP ecClass = assetSchema->GetClassCP("Pipe");
        ASSERT_EQ(ecClass, nullptr) << "Failed to load Pipe ECEntityClass.";
        });

    Utf8CP updatedSchemaXml2 = R"xml(
        <ECSchema schemaName="Asset" alias="asset" version="1.0.1" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="BisCore" version="01.00.14" alias="bis"/>
            <ECSchemaReference name="CoreCustomAttributes" version="01.00.03" alias="CoreCA"/>
            <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
            <ECCustomAttributes>
                <DynamicSchema xmlns="CoreCustomAttributes.01.00.03"/>
            </ECCustomAttributes>
            <ECEntityClass typeName="Pipe">
                <BaseClass>bis:PhysicalElement</BaseClass>
            </ECEntityClass>
        </ECSchema>
        )xml";
    TestSchemaUpgrade("Asset", fileName, updatedSchemaXml2, SchemaStatus::Success, [](DgnDbPtr db) {
        EXPECT_TRUE(db->Schemas().ContainsSchema("Asset"));
        ECSchemaCP updatedAssetSchema = db->Schemas().GetSchema("Asset");
        ASSERT_NE(updatedAssetSchema, nullptr);
        EXPECT_TRUE(updatedAssetSchema->IsDynamicSchema()) << "Schema is dynamic schema.";
        ECClassCP updatedEcClass = updatedAssetSchema->GetClassCP("Pipe");
        ASSERT_NE(updatedEcClass, nullptr);
        });
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(DynamicSchemaTest, ImportingDynamicSchemaWithSchemaHasBehaviorRequiresVersionChange)
    {
    Utf8CP schemaXml = R"xml(
        <ECSchema schemaName="Asset" alias="asset" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="BisCore" version="01.00.14" alias="bis"/>
            <ECSchemaReference name="CoreCustomAttributes" version="01.00.03" alias="CoreCA"/>
            <ECCustomAttributes>
                <DynamicSchema xmlns="CoreCustomAttributes.01.00.03"/>
            </ECCustomAttributes>
        </ECSchema>
        )xml";
    auto fileName = ImportSchema("Asset", schemaXml);

    Utf8CP updatedSchemaXml1 = R"xml(
        <ECSchema schemaName="Asset" alias="asset" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="BisCore" version="01.00.14" alias="bis"/>
            <ECSchemaReference name="CoreCustomAttributes" version="01.00.03" alias="CoreCA"/>
            <ECCustomAttributes>
                <DynamicSchema xmlns="CoreCustomAttributes.01.00.03"/>
                <SchemaHasBehavior xmlns="BisCore.01.00.14"/>
            </ECCustomAttributes>
        </ECSchema>
        )xml";
    TestSchemaUpgrade("Asset", fileName, updatedSchemaXml1, SchemaStatus::Success, [](DgnDbPtr db) {
        EXPECT_TRUE(db->Schemas().ContainsSchema("Asset"));
        ECSchemaCP assetSchema = db->Schemas().GetSchema("Asset");
        ASSERT_NE(assetSchema, nullptr);
        EXPECT_FALSE(assetSchema->IsDefined("BisCore", "SchemaHasBehavior")) << "As dynamic schema has SchemaHasBehavior Custom Attribute and schema version is not updated, it is not imported.";
        });

    Utf8CP updatedSchemaXml2 = R"xml(
        <ECSchema schemaName="Asset" alias="asset" version="1.0.1" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="BisCore" version="01.00.14" alias="bis"/>
            <ECSchemaReference name="CoreCustomAttributes" version="01.00.03" alias="CoreCA"/>
            <ECCustomAttributes>
                <DynamicSchema xmlns="CoreCustomAttributes.01.00.03"/>
                <SchemaHasBehavior xmlns="BisCore.01.00.14"/>
            </ECCustomAttributes>
        </ECSchema>
        )xml";
    TestSchemaUpgrade("Asset", fileName, updatedSchemaXml2, SchemaStatus::Success, [](DgnDbPtr db) {
        EXPECT_TRUE(db->Schemas().ContainsSchema("Asset"));
        ECSchemaCP updatedAssetSchema = db->Schemas().GetSchema("Asset");
        ASSERT_NE(updatedAssetSchema, nullptr);
        EXPECT_TRUE(updatedAssetSchema->IsDefined("BisCore", "SchemaHasBehavior"));
        });
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(DynamicSchemaTest, CannotImportDynamicSchemaWithDifferentReadVersion)
    {
    Utf8CP schemaXml = R"xml(
        <ECSchema schemaName="Asset" alias="asset" version="2.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="CoreCustomAttributes" version="01.00.03" alias="CoreCA"/>
            <ECCustomAttributes>
                <DynamicSchema xmlns="CoreCustomAttributes.01.00.03"/>
            </ECCustomAttributes>
        </ECSchema>
        )xml";
    auto fileName = ImportSchema("Asset", schemaXml);

    Utf8CP updatedSchemaXml1 = R"xml(
        <ECSchema schemaName="Asset" alias="asset" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="BisCore" version="01.00.14" alias="bis"/>
            <ECSchemaReference name="CoreCustomAttributes" version="01.00.03" alias="CoreCA"/>
            <ECCustomAttributes>
                <DynamicSchema xmlns="CoreCustomAttributes.01.00.03"/>
            </ECCustomAttributes>
            <ECEntityClass typeName="Pipe">
                <BaseClass>bis:PhysicalElement</BaseClass>
            </ECEntityClass>
        </ECSchema>
        )xml";
    TestSchemaUpgrade("Asset", fileName, updatedSchemaXml1, SchemaStatus::SchemaTooNew, [](DgnDbPtr db) {
        EXPECT_TRUE(db->Schemas().ContainsSchema("Asset"));
        ECSchemaCP assetSchema = db->Schemas().GetSchema("Asset");
        ASSERT_NE(assetSchema, nullptr);
        ECClassCP ecClass = assetSchema->GetClassCP("Pipe");
        ASSERT_EQ(ecClass, nullptr);
        });

    Utf8CP updatedSchemaXml2 = R"xml(
        <ECSchema schemaName="Asset" alias="asset" version="3.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="BisCore" version="01.00.14" alias="bis"/>
            <ECSchemaReference name="CoreCustomAttributes" version="01.00.03" alias="CoreCA"/>
            <ECCustomAttributes>
                <DynamicSchema xmlns="CoreCustomAttributes.01.00.03"/>
            </ECCustomAttributes>
            <ECEntityClass typeName="Pipe">
                <BaseClass>bis:PhysicalElement</BaseClass>
            </ECEntityClass>
        </ECSchema>
        )xml";
    TestSchemaUpgrade("Asset", fileName, updatedSchemaXml2, SchemaStatus::SchemaTooOld, [](DgnDbPtr db) {
        EXPECT_TRUE(db->Schemas().ContainsSchema("Asset"));
        ECSchemaCP assetSchema = db->Schemas().GetSchema("Asset");
        ASSERT_NE(assetSchema, nullptr);
        ECClassCP ecClass = assetSchema->GetClassCP("Pipe");
        ASSERT_EQ(ecClass, nullptr);
        });
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(DynamicSchemaTest, CannotImportDynamicSchemaIfWriteVersionIsDecreased)
    {
    Utf8CP schemaXml = R"xml(
        <ECSchema schemaName="Asset" alias="asset" version="1.2.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="CoreCustomAttributes" version="01.00.03" alias="CoreCA"/>
            <ECCustomAttributes>
                <DynamicSchema xmlns="CoreCustomAttributes.01.00.03"/>
            </ECCustomAttributes>
        </ECSchema>
        )xml";
    auto fileName = ImportSchema("Asset", schemaXml);

    Utf8CP updatedSchemaXml = R"xml(
        <ECSchema schemaName="Asset" alias="asset" version="1.1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="BisCore" version="01.00.14" alias="bis"/>
            <ECSchemaReference name="CoreCustomAttributes" version="01.00.03" alias="CoreCA"/>
            <ECCustomAttributes>
                <DynamicSchema xmlns="CoreCustomAttributes.01.00.03"/>
            </ECCustomAttributes>
            <ECEntityClass typeName="Pipe">
                <BaseClass>bis:PhysicalElement</BaseClass>
            </ECEntityClass>
        </ECSchema>
        )xml";
    TestSchemaUpgrade("Asset", fileName, updatedSchemaXml, SchemaStatus::SchemaTooNew, [](DgnDbPtr db) {
        EXPECT_TRUE(db->Schemas().ContainsSchema("Asset"));
        ECSchemaCP assetSchema = db->Schemas().GetSchema("Asset");
        ASSERT_NE(assetSchema, nullptr);
        ECClassCP ecClass = assetSchema->GetClassCP("Pipe");
        ASSERT_EQ(ecClass, nullptr);
        });
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(DynamicSchemaTest, CanImportDynamicSchemaIfWriteVersionIsIncreased)
    {
    Utf8CP schemaXml = R"xml(
        <ECSchema schemaName="Asset" alias="asset" version="1.2.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="CoreCustomAttributes" version="01.00.03" alias="CoreCA"/>
            <ECCustomAttributes>
                <DynamicSchema xmlns="CoreCustomAttributes.01.00.03"/>
            </ECCustomAttributes>
        </ECSchema>
        )xml";
    auto fileName = ImportSchema("Asset", schemaXml);

    Utf8CP updatedSchemaXml = R"xml(
        <ECSchema schemaName="Asset" alias="asset" version="1.3.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="BisCore" version="01.00.14" alias="bis"/>
            <ECSchemaReference name="CoreCustomAttributes" version="01.00.03" alias="CoreCA"/>
            <ECCustomAttributes>
                <DynamicSchema xmlns="CoreCustomAttributes.01.00.03"/>
            </ECCustomAttributes>
            <ECEntityClass typeName="Pipe">
                <BaseClass>bis:PhysicalElement</BaseClass>
            </ECEntityClass>
        </ECSchema>
        )xml";
    TestSchemaUpgrade("Asset", fileName, updatedSchemaXml, SchemaStatus::Success, [](DgnDbPtr db) {
        EXPECT_TRUE(db->Schemas().ContainsSchema("Asset"));
        ECSchemaCP assetSchema = db->Schemas().GetSchema("Asset");
        ASSERT_NE(assetSchema, nullptr);
        ECClassCP ecClass = assetSchema->GetClassCP("Pipe");
        ASSERT_NE(ecClass, nullptr);
        });
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(DynamicSchemaTest, CanImportDynamicSchemaIfMinorVersionIsIncreased)
    {
    Utf8CP schemaXml = R"xml(
        <ECSchema schemaName="Asset" alias="asset" version="1.0.1" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="CoreCustomAttributes" version="01.00.03" alias="CoreCA"/>
            <ECCustomAttributes>
                <DynamicSchema xmlns="CoreCustomAttributes.01.00.03"/>
            </ECCustomAttributes>
        </ECSchema>
        )xml";
    auto fileName = ImportSchema("Asset", schemaXml);

    Utf8CP updatedSchemaXml1 = R"xml(
        <ECSchema schemaName="Asset" alias="asset" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="BisCore" version="01.00.14" alias="bis"/>
            <ECSchemaReference name="CoreCustomAttributes" version="01.00.03" alias="CoreCA"/>
            <ECCustomAttributes>
                <DynamicSchema xmlns="CoreCustomAttributes.01.00.03"/>
            </ECCustomAttributes>
            <ECEntityClass typeName="Pipe">
                <BaseClass>bis:PhysicalElement</BaseClass>
            </ECEntityClass>
        </ECSchema>
        )xml";
    TestSchemaUpgrade("Asset", fileName, updatedSchemaXml1, SchemaStatus::Success, [](DgnDbPtr db) {
        EXPECT_TRUE(db->Schemas().ContainsSchema("Asset"));
        ECSchemaCP assetSchema = db->Schemas().GetSchema("Asset");
        ASSERT_NE(assetSchema, nullptr);
        ECClassCP ecClass = assetSchema->GetClassCP("Pipe");
        ASSERT_EQ(ecClass, nullptr) << "Updated schema is skipped from importing as minor version is less than or equal to existing minor version.";
        });

    Utf8CP updatedSchemaXml2 = R"xml(
        <ECSchema schemaName="Asset" alias="asset" version="1.0.2" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="BisCore" version="01.00.14" alias="bis"/>
            <ECSchemaReference name="CoreCustomAttributes" version="01.00.03" alias="CoreCA"/>
            <ECCustomAttributes>
                <DynamicSchema xmlns="CoreCustomAttributes.01.00.03"/>
            </ECCustomAttributes>
            <ECEntityClass typeName="Pipe">
                <BaseClass>bis:PhysicalElement</BaseClass>
            </ECEntityClass>
        </ECSchema>
        )xml";
    TestSchemaUpgrade("Asset", fileName, updatedSchemaXml2, SchemaStatus::Success, [](DgnDbPtr db) {
        EXPECT_TRUE(db->Schemas().ContainsSchema("Asset"));
        ECSchemaCP assetSchema = db->Schemas().GetSchema("Asset");
        ASSERT_NE(assetSchema, nullptr);
        ECClassCP ecClass = assetSchema->GetClassCP("Pipe");
        ASSERT_NE(ecClass, nullptr);
        });
    }
