/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "TestIModelCreators.h"
#include "Profiles.h"
#include "TestDomain.h"

USING_NAMESPACE_BENTLEY_EC
//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                    06/18
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool TestIModelCreation::s_hasRun = false;

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                    06/18
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus TestIModelCreation::Run()
    {
    if (s_hasRun)
        return SUCCESS;

    s_hasRun = true;
    return TestFileCreation::Run(std::vector<std::shared_ptr<TestFileCreator>>(TESTIMODELCREATOR_LIST));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                    06/18
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
    DgnDbPtr bim = DgnDb::CreateDgnDb(&stat, filePath, createParam);
    if (BE_SQLITE_OK != stat)
        {
        LOG.errorv("Failed to create new test file '%s'.", filePath.GetNameUtf8().c_str());
        return nullptr;
        }

    return bim;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                    06/18
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

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                    07/18
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
        DgnDb::OpenDgnDb(&stat, targetPath, params);
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
// @bsimethod                                     Krischan.Eberle                    07/18
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
        DgnDbPtr bim = DgnDb::OpenDgnDb(&stat, testFile.GetSeedPath(), DgnDb::OpenParams(DgnDb::OpenMode::ReadWrite));
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
    return SUCCESS;
    }


//************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                    09/18
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
