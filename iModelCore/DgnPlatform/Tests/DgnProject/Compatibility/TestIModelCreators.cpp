/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Compatibility/TestIModelCreators.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "TestIModelCreators.h"
#include "Profiles.h"

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
        LOG.errorv("Failed to create new test file '%s': Could not deserialize ECSchemas to import.", dgndb.GetDbFileName());
        return ERROR;
        }

    if (SchemaStatus::Success == dgndb.ImportSchemas(ctx->GetCache().GetSchemas()))
        {
        dgndb.SaveChanges();
        return SUCCESS;
        }

    dgndb.AbandonChanges();
    LOG.errorv("Failed to create new test file '%s': Could not import ECSchemas.", dgndb.GetDbFileName());
    return ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                    07/18
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus TestIModelCreator::_UpgradeOldFiles() const
    {
    Profile const& profile = DgnDbProfile::Get();
    std::vector<TestFile> testFiles = profile.GetAllVersionsOfTestFileFromOldData(m_fileName.c_str(), false);
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

        DgnDb::OpenParams params(DgnDb::OpenMode::ReadWrite);
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
