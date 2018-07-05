/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Compatibility/TestECDbCreators.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "TestECDbCreators.h"
#include "Profiles.h"

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                    06/18
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool TestECDbCreation::s_hasRun = false;

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                    06/18
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus TestECDbCreation::Run()
    {
    if (s_hasRun)
        return SUCCESS;

    s_hasRun = true;
    return TestFileCreation::Run(std::vector<std::shared_ptr<TestFileCreator>>(TESTECDBCREATOR_LIST));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                    06/18
//+---------------+---------------+---------------+---------------+---------------+------
DbResult TestECDbCreator::CreateNewTestFile(ECDbR ecdb, Utf8StringCR fileName)
    {
    BeFileName filePath = ECDbProfile::Get().GetPathForNewTestFile(fileName);
    BeFileName folder = filePath.GetDirectoryName();
    if (!folder.DoesPathExist())
        {
        if (BeFileNameStatus::Success != BeFileName::CreateNewDirectory(folder))
            {
            LOG.errorv("Failed to create new test file '%s': Could not create folder.", filePath.GetNameUtf8().c_str());
            return BE_SQLITE_ERROR;
            }
        }
    else if (filePath.DoesPathExist())
        {
        if (BeFileNameStatus::Success != filePath.BeDeleteFile())
            {
            LOG.errorv("Failed to create new test file '%s': Could not delete pre-existing file.", filePath.GetNameUtf8().c_str());
            return BE_SQLITE_ERROR;
            }
        }

    DbResult stat = ecdb.CreateNewDb(filePath);
    if (BE_SQLITE_OK != stat)
        {
        LOG.errorv("Failed to create new test file '%s'.", filePath.GetNameUtf8().c_str());
        return stat;
        }

    return BE_SQLITE_OK;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                    06/18
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus TestECDbCreator::ImportSchemas(ECDbCR ecdb, std::vector<SchemaItem> const& schemas)
    {
    ECN::ECSchemaReadContextPtr ctx = DeserializeSchemas(ecdb, schemas);
    if (ctx == nullptr)
        {
        LOG.errorv("Failed to create new test file '%s': Could not deserialize ECSchemas to import.", ecdb.GetDbFileName());
        return ERROR;
        }

    Savepoint sp(const_cast<ECDb&>(ecdb), "Schema Import");
    if (SUCCESS == ecdb.Schemas().ImportSchemas(ctx->GetCache().GetSchemas()))
        {
        sp.Commit();
        return SUCCESS;
        }

    sp.Cancel();
    LOG.errorv("Failed to create new test file '%s': Could not import ECSchemas.", ecdb.GetDbFileName());
    return ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                    07/18
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus TestECDbCreator::_UpgradeOldFiles() const
    {
    Profile const& profile = ECDbProfile::Get();
    std::vector<TestFile> testFiles = profile.GetAllVersionsOfTestFile(m_fileName.c_str(), false);
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

        ECDb ecdb;
        DbResult stat = ecdb.OpenBeSQLiteDb(targetPath, ECDb::OpenParams(ECDb::OpenMode::ReadWrite, ECDb::ProfileUpgradeOptions::Upgrade));
        if (BE_SQLITE_OK != stat)
            {
            LOG.errorv("Failed to create new upgraded test file '%s': Upgrading the old test file failed: %s", targetPath.GetNameUtf8().c_str(), Db::InterpretDbResult(stat));
            return ERROR;
            }

        LOG.infov("Created new upgraded test file '%s'.", targetPath.GetNameUtf8().c_str());
        }

    return SUCCESS;
    }
