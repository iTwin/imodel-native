/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "TestIModelCreators.h"
#include "Profiles.h"
#include "TestDomain.h"
#include <DgnPlatform/FunctionalDomain.h>
#include <DgnPlatform/PhysicalMaterialDomain.h>

USING_NAMESPACE_BENTLEY_EC
//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool TestIModelCreation::s_hasRun = false;

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus TestIModelCreation::Run()
    {
    if (s_hasRun)
        return SUCCESS;

    s_hasRun = true;
    return TestFileCreation::Run(std::vector<std::shared_ptr<TestFileCreator>>(TESTIMODELCREATOR_LIST));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DgnDbPtr TestIModelCreator::CreateNewTestFile(Utf8StringCR fileName)
    {
    BeFileName filePath = DgnDbProfile::Get().GetPathForNewTestFile(fileName);
    BeFileName folder = filePath.GetDirectoryName();
    if (!folder.DoesPathExist())
        {
        if (BeFileNameStatus::Success != BeFileName::CreateNewDirectory(folder))
            {
            LOG.errorv("Failed to create new test file '%s': Could not create folder.", filePath.GetNameUtf8().c_str());
            return nullptr;
            }

        }
    else if (filePath.DoesPathExist())
        {
        if (BeFileNameStatus::Success != filePath.BeDeleteFile())
            {
            LOG.errorv("Failed to create new test file '%s': Could not delete pre-existing file.", filePath.GetNameUtf8().c_str());
            return nullptr;
            }
        }

    CreateDgnDbParams createParam(fileName.c_str());
    DbResult stat = BE_SQLITE_OK;
    DgnDbPtr bim = DgnDb::CreateIModel(&stat, filePath, createParam);
    if (BE_SQLITE_OK != stat)
        {
        LOG.errorv("Failed to create new test file '%s'.", filePath.GetNameUtf8().c_str());
        return nullptr;
        }

    return bim;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus TestIModelCreator::ImportSchemas(DgnDbR dgndb, std::vector<SchemaItem> const& schemas)
    {
    dgndb.SaveChanges();
    ECSchemaReadContextPtr ctx = DeserializeSchemas(dgndb, schemas);
    if (ctx == nullptr)
        {
        LOG.errorv("Failed to create new/upgrade test file '%s': Could not deserialize ECSchemas to import.", dgndb.GetDbFileName());
        return ERROR;
        }

    if (SchemaStatus::Success == dgndb.ImportSchemas(ctx->GetCache().GetSchemas()))
        {
        dgndb.SaveChanges();
        return SUCCESS;
        }

    dgndb.AbandonChanges();
    LOG.errorv("Failed to create new/upgrade test file '%s': Could not import ECSchemas.", dgndb.GetDbFileName());
    return ERROR;
    }

/*static*/ BentleyStatus TestIModelCreator::LoadDomainsAndSchemas(DgnDbR dgndb)
    {
    // Create a read context and set the schema locator
    auto schemaContext = ECSchemaReadContext::CreateContext(false, true);
    schemaContext->SetFinalSchemaLocater(dgndb.GetSchemaLocater());

    auto importSchema = [&dgndb, &schemaContext](WCharCP schemaFilePath)
        {
        // Read the latest schema xml from the file path supplied
        ECSchemaPtr schema;
        if (ECSchema::ReadFromXmlFile(schema, schemaFilePath, *schemaContext) != SchemaReadStatus::Success)
            return ERROR;

        // Write the schema to a xml string
        Utf8String xmlString;
        schema->WriteToXmlString(xmlString, ECVersion::Latest);

        // Import the schema into the DgnDb object
        return ImportSchema(dgndb, SchemaItem(xmlString));
        };

    // Register domains and import it's latest schema
    for (const auto& domainName : std::vector<std::wstring>{FUNCTIONAL_DOMAIN_ECSCHEMA_PATH, PHYSICAL_MATERIAL_DOMAIN_ECSCHEMA_PATH})
        {
        auto status = ERROR;
        if (domainName == FUNCTIONAL_DOMAIN_ECSCHEMA_PATH)
            status = DgnDomains::RegisterDomain(FunctionalDomain::GetDomain(), DgnDomain::Required::Yes, DgnDomain::Readonly::No);
        else if (domainName == PHYSICAL_MATERIAL_DOMAIN_ECSCHEMA_PATH)
            status = DgnDomains::RegisterDomain(PhysicalMaterialDomain::GetDomain(), DgnDomain::Required::Yes, DgnDomain::Readonly::No);

        if (status == SUCCESS)
            {
            auto schemaPathName = T_HOST.GetIKnownLocationsAdmin().GetDgnPlatformAssetsDirectory();
            schemaPathName.AppendToPath(domainName.c_str());

            if (importSchema(schemaPathName.GetName()) != SUCCESS)
                {
                LOG.errorv("Failed to import Domain schema");
                return ERROR;
                }
            }
        }

    // Add non domain schemas
    for (const auto& schemaName : std::vector<WCharCP>{L"ECSchemas/Standard/Bentley_Standard_CustomAttributes.01.13.ecschema.xml",
                                                       L"ECSchemas/Standard/SIUnitSystemDefaults.01.00.ecschema.xml",
                                                       L"ECSchemas/Standard/SchemaUpgradeCustomAttributes.01.00.00.ecschema.xml"})
        {
        auto nonDomainSchemaPathName = T_HOST.GetIKnownLocationsAdmin().GetDgnPlatformAssetsDirectory();
        nonDomainSchemaPathName.AppendToPath(schemaName);
        if (importSchema(nonDomainSchemaPathName.GetName()) != SUCCESS)
            {
            LOG.errorv("Failed to import non-Domain Schema %s", schemaName);
            return ERROR;
            }
        }

    return SUCCESS;
    }

/*static*/ BentleyStatus TestIModelCreator::UnregisterDomainsForTest()
    {
    if ((DgnDomains::UnRegisterDomain(FunctionalDomain::GetDomain()) != SUCCESS)
        || (DgnDomains::UnRegisterDomain(PhysicalMaterialDomain::GetDomain()) != SUCCESS))
        return ERROR;
    return SUCCESS;
    }

/*static*/ BentleyStatus TestIModelCreator::RegisterDomainsForTest()
    {
    if ((DgnDomains::RegisterDomain(FunctionalDomain::GetDomain(), DgnDomain::Required::Yes, DgnDomain::Readonly::No) != SUCCESS)
        || (DgnDomains::RegisterDomain(PhysicalMaterialDomain::GetDomain(), DgnDomain::Required::Yes, DgnDomain::Readonly::No) != SUCCESS))
        return ERROR;
    return SUCCESS;
    }

BentleyStatus TestIModelCreator::_UpgradeSchemas() const
    {
    auto status = BE_SQLITE_OK;
    for (const auto& testFile : DgnDbProfile::Get().GetAllVersionsOfTestFile(DgnDbProfile::Get().GetCreatedDataFolder(), m_fileName.c_str(), true))
        {
        if (testFile.GetInitialDgnDbVersion().CompareTo(DgnDbProfile::Get().GetExpectedVersion()) == 0)
            {
            auto dgnDbPtr = DgnDb::OpenIModelDb(&status, testFile.GetSeedPath(), DgnDb::OpenParams(DgnDb::OpenMode::ReadWrite));
            if (dgnDbPtr == nullptr || status != BE_SQLITE_OK)
                return ERROR;
            if (TestIModelCreator::LoadDomainsAndSchemas(*dgnDbPtr) != SUCCESS)
                return ERROR;
            }
        }
    if (UnregisterDomainsForTest() != SUCCESS)
        return ERROR;
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus TestIModelCreator::_UpgradeOldFiles() const
    {
    Profile const& profile = DgnDbProfile::Get();
    std::vector<TestFile> testFiles = profile.GetAllVersionsOfTestFile(profile.GetTestDataFolder(), m_fileName.c_str(), false);
    for (TestFile const& testFile : testFiles)
        {
        if (testFile.GetAge() != ProfileState::Age::Older)
            continue; // only older files can be upgraded

        BeFileName targetPath = profile.GetPathForNewUpgradedTestFile(testFile);
        if (BeFileNameStatus::Success != testFile.CloneSeed(targetPath))
            {
            LOG.errorv("Failed to create new upgraded test file '%s': Could not clone the old test file to its target location.", targetPath.GetNameUtf8().c_str());
            return ERROR;
            }

        DgnDb::OpenParams params(DgnDb::OpenMode::ReadWrite, BeSQLite::DefaultTxn::Yes, SchemaUpgradeOptions(SchemaUpgradeOptions::DomainUpgradeOptions::Upgrade));
        params.SetProfileUpgradeOptions(DgnDb::ProfileUpgradeOptions::Upgrade);
        DgnDbPtr bim = nullptr;
        DbResult stat = BE_SQLITE_OK;
        DgnDb::OpenIModelDb(&stat, targetPath, params);
        if (BE_SQLITE_OK != stat)
            {
            LOG.errorv("Failed to create new upgraded test file '%s': Upgrading the old test file failed: %s", targetPath.GetNameUtf8().c_str(), Db::InterpretDbResult(stat));
            return ERROR;
            }

        LOG.infov("Created new upgraded test file '%s'.", targetPath.GetNameUtf8().c_str());
        }

    return SUCCESS;
    }


//************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus EC32EnumsProfileUpgradedTestIModelCreator::_UpgradeSchemas() const
    {
    for (TestFile const& testFile : DgnDbProfile::Get().GetAllVersionsOfTestFile(DgnDbProfile::Get().GetCreatedDataFolder(), TESTIMODEL_EC32ENUMS_PROFILEUPGRADED, false))
        {
        if (testFile.GetAge() != ProfileState::Age::UpToDate)
            {
            BeAssert(false && "files to upgrade schemas for should be up-to-date profilewise.");
            return ERROR;
            }


        DbResult stat = BE_SQLITE_OK;
        DgnDbPtr bim = DgnDb::OpenIModelDb(&stat, testFile.GetSeedPath(), DgnDb::OpenParams(DgnDb::OpenMode::ReadWrite));
        if (BE_SQLITE_OK != stat)
            {
            LOG.errorv("Failed to upgrade schema in test file '%s': Could not open the test file read-write: %s", testFile.GetSeedPath().GetNameUtf8().c_str(), Db::InterpretDbResult(stat));
            return ERROR;
            }


        //Upgrade the enumerator names of the enums
        if (SUCCESS != ImportSchema(*bim, SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
                                                    <ECSchema schemaName="TestSchema" alias="ts" version="1.0.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                                                        <ECEnumeration typeName="IntEnum_EnumeratorsWithoutDisplayLabel" displayLabel="Int Enumeration with enumerators without display label" description="Int Enumeration with enumerators without display label" backingTypeName="int" isStrict="true">
                                                            <ECEnumerator name="Unknown" value="0"/>
                                                            <ECEnumerator name="On" value="1"/>
                                                            <ECEnumerator name="Off" value="2"/>
                                                        </ECEnumeration>
                                                        <ECEnumeration typeName="StringEnum_EnumeratorsWithDisplayLabel" displayLabel="String Enumeration with enumerators with display label" backingTypeName="string" isStrict="false">
                                                            <ECEnumerator name="An" value="On" displayLabel="Turned On"/>
                                                            <ECEnumerator name="Aus" value="Off" displayLabel="Turned Off"/>
                                                        </ECEnumeration>
                                                     </ECSchema>)xml")))
            {
            LOG.errorv("Failed to upgrade schema in test file '%s'.", testFile.GetSeedPath().GetNameUtf8().c_str());
            return ERROR;
            }
        }

    auto status = BE_SQLITE_OK;
    for (const auto& testFile : DgnDbProfile::Get().GetAllVersionsOfTestFile(DgnDbProfile::Get().GetCreatedDataFolder(), TESTIMODEL_EC32ENUMS_PROFILEUPGRADED, true))
        {
        if (testFile.GetInitialDgnDbVersion().CompareTo(DgnDbProfile::Get().GetExpectedVersion()) == 0)
            {
            auto dgnDbPtr = DgnDb::OpenIModelDb(&status, testFile.GetSeedPath(), DgnDb::OpenParams(DgnDb::OpenMode::ReadWrite));
            if (dgnDbPtr == nullptr || status != BE_SQLITE_OK)
                return ERROR;
            if (TestIModelCreator::LoadDomainsAndSchemas(*dgnDbPtr) != SUCCESS)
                return ERROR;
            }
        }
    if (UnregisterDomainsForTest() != SUCCESS)
        return ERROR;
    return SUCCESS;
    }


//************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus TestDomainTestIModelCreator::_Create()
    {
    if (SUCCESS != IModelEvolutionTestsDomain::Register(SchemaVersion(1, 0, 0), DgnDomain::Required::No, DgnDomain::Readonly::No))
        return ERROR;

    DgnDbPtr bim = CreateNewTestFile(m_fileName);
    if (bim == nullptr)
        return ERROR;

    if (SchemaStatus::Success != IModelEvolutionTestsDomain::GetDomain().ImportSchema(*bim))
        return ERROR;

    bim->SaveChanges();
    return SUCCESS;
    }
