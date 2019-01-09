/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/src/ProfilesTestCase.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ProfilesTestCase.h"
#include "TestHost.h"

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_PROFILES

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
static BeFileName copyWorkingDb()
    {
    BeFileName baseDbPath = ProfilesTestHost::Instance().GetBaseDbPath();
    WString directory = baseDbPath.GetDirectoryName();
    WString name = baseDbPath.GetFileNameWithoutExtension();
    WString extension = baseDbPath.GetExtension();

    BeFileName workingDbPath (directory + name + L"_Working." + extension);

    BeFileNameStatus fileCopyStatus = BeFileName::BeCopyFile (baseDbPath, workingDbPath);
    BeAssert (fileCopyStatus == BeFileNameStatus::Success);

    return workingDbPath;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
ProfilesTestCase::ProfilesTestCase()
    : m_dbPtr (nullptr)
    , m_definitionModelPtr (nullptr)
    {
    // Copy and open a fresh DB
    BeFileName workingDbPath = copyWorkingDb();

    DgnDb::OpenParams openParams (BeSQLite::Db::OpenMode::ReadWrite, BeSQLite::DefaultTxn::Yes, SchemaUpgradeOptions::DomainUpgradeOptions::Upgrade);
    BeSQLite::DbResult openStatus;
    m_dbPtr = DgnDb::OpenDgnDb (&openStatus, workingDbPath, openParams);
    BeAssert (m_dbPtr.IsValid());

    // Create default model for definition elements
    m_definitionModelPtr = InsertDgnModel<DefinitionModel, DefinitionPartition> ("ProfilesTestPartition_Definition");
    BeAssert (m_definitionModelPtr.IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
ProfilesTestCase::~ProfilesTestCase()
    {
    m_dbPtr->CloseDb();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDb& ProfilesTestCase::GetDb()
    {
    BeAssert (m_dbPtr.IsValid());
    return *m_dbPtr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModel& ProfilesTestCase::GetModel()
    {
    BeAssert (m_definitionModelPtr.IsValid());
    return *m_definitionModelPtr;
    }
