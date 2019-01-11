/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Compatibility/TestECDbCreators.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
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
        LOG.errorv("Failed to create new/upgrade test file '%s': Could not deserialize ECSchemas to import.", ecdb.GetDbFileName());
        return ERROR;
        }

    Savepoint sp(const_cast<ECDb&>(ecdb), "Schema Import");
    if (SUCCESS == ecdb.Schemas().ImportSchemas(ctx->GetCache().GetSchemas()))
        {
        sp.Commit();
        return SUCCESS;
        }

    sp.Cancel();
    LOG.errorv("Failed to create new/upgrade test file '%s': Could not import ECSchemas.", ecdb.GetDbFileName());
    return ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                    07/18
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus TestECDbCreator::_UpgradeOldFiles() const
    {
    Profile const& profile = ECDbProfile::Get();
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

//************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                    07/18
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus EC32EnumsProfileUpgradedTestECDbCreator::_UpgradeSchemas() const
    {
    for (TestFile const& testFile : ECDbProfile::Get().GetAllVersionsOfTestFile(ECDbProfile::Get().GetCreatedDataFolder(), TESTECDB_EC32ENUMS_PROFILEUPGRADED, false))
        {
        if (testFile.GetAge() != ProfileState::Age::UpToDate)
            {
            BeAssert(false && "files to upgrade schemas for should be up-to-date profilewise.");
            return ERROR;
            }

        ECDb ecdb;
        DbResult stat = ecdb.OpenBeSQLiteDb(testFile.GetSeedPath(), ECDb::OpenParams(ECDb::OpenMode::ReadWrite));
        if (BE_SQLITE_OK != stat)
            {
            LOG.errorv("Failed to upgrade schema in test file '%s': Could not open the test file read-write: %s", testFile.GetSeedPath().GetNameUtf8().c_str(), Db::InterpretDbResult(stat));
            return ERROR;
            }


        //Upgrade the enumerator names of the enums
        if (SUCCESS != ImportSchema(ecdb, SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
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