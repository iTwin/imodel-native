/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/src/ProfilesTestCase.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ProfilesTestCase.h"
#include "TestHost.h"


USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_SQLITE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
static BeFileName getWorkingDbPath (BeFileName const& baseDbPath)
    {
    WString directory = baseDbPath.GetDirectoryName();
    WString name = baseDbPath.GetFileNameWithoutExtension();
    WString extension = baseDbPath.GetExtension();

    return BeFileName (directory + name + L"_Working." + extension);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
ProfilesTestCase::ProfilesTestCase()
    : m_dbPtr (nullptr)
    , m_modelPtr (nullptr)
    {
    BeFileName baseDbPath = ProfilesTestHost::Instance().GetBaseDbPath();
    BeFileName workingDbPath = getWorkingDbPath (baseDbPath);

    BeFileNameStatus fileCopyStatus = BeFileName::BeCopyFile (baseDbPath, workingDbPath);
    BeAssert (fileCopyStatus == BeFileNameStatus::Success);

    DgnDb::OpenParams openParams (BeSQLite::Db::OpenMode::ReadWrite, BeSQLite::DefaultTxn::Yes, SchemaUpgradeOptions::DomainUpgradeOptions::Upgrade);
    BeSQLite::DbResult openStatus;
    m_dbPtr = DgnDb::OpenDgnDb (&openStatus, workingDbPath, openParams);
    BeAssert (m_dbPtr.IsValid());

    SubjectCPtr rootSubjectPtr = m_dbPtr->Elements().GetRootSubject();
    DefinitionPartitionPtr partitionPtr = DefinitionPartition::Create (*rootSubjectPtr, "TestPartition");
    m_dbPtr->BriefcaseManager().AcquireForElementInsert (*partitionPtr);

    DgnDbStatus status;
    m_dbPtr->Elements().Insert<DefinitionPartition> (*partitionPtr, &status);
    BeAssert (status == DgnDbStatus::Success);

    m_modelPtr = DefinitionModel::Create (*partitionPtr);
    status = m_modelPtr->Insert();
    BeAssert (status == DgnDbStatus::Success);
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
    BeAssert (m_modelPtr.IsValid());
    return *m_modelPtr;
    }
