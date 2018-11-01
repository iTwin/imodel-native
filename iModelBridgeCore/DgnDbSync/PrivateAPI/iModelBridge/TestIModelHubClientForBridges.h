/*--------------------------------------------------------------------------------------+
|
|  $Source: PrivateAPI/iModelBridge/TestIModelHubClientForBridges.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <iModelBridge/Fwk/IModelClientForBridges.h>
#include <BeSQLite/BeSQLite.h>
//=======================================================================================
// @bsistruct                                                   Sam.Wilson   10/17
//=======================================================================================
BEGIN_BENTLEY_DGN_NAMESPACE

struct TestRepositoryAdmin : IRepositoryManager
    {
    virtual Response _ProcessRequest(Request const& req, DgnDbR db, bool queryOnly)
        {
        Response response(queryOnly ? IBriefcaseManager::RequestPurpose::Query : IBriefcaseManager::RequestPurpose::Acquire, req.Options());
        response.SetResult(RepositoryStatus::Success);
        return response;
        }

    virtual RepositoryStatus _Demote(DgnLockSet const& locks, DgnCodeSet const& codes, DgnDbR db)
        {
        return RepositoryStatus::Success;
        }
    virtual RepositoryStatus _Relinquish(Resources which, DgnDbR db)
        {
        return RepositoryStatus::Success;
        }
    virtual RepositoryStatus _QueryHeldResources(DgnLockSet& locks, DgnCodeSet& codes, DgnLockSet& unavailableLocks, DgnCodeSet& unavailableCodes, DgnDbR db)
        {
        return RepositoryStatus::Success;
        }
    virtual RepositoryStatus _QueryStates(DgnLockInfoSet& lockStates, DgnCodeInfoSet& codeStates, LockableIdSet const& locks, DgnCodeSet const& codes)
        {
        return RepositoryStatus::Success;
        }
    };

struct TestIModelHubClientForBridges : IModelHubClientForBridges
    {
    bool anyTxnsInFile(DgnDbR db)
        {
        BeSQLite::Statement stmt;
        stmt.Prepare(db, "SELECT Id FROM " DGN_TABLE_Txns " LIMIT 1");
        return (BeSQLite::BE_SQLITE_ROW == stmt.Step());
        }

    iModel::Hub::Error m_lastServerError;
    BeFileName m_serverRepo;
    BeFileName m_testWorkDir;
    DgnDbP m_briefcase;
    BeSQLite::BeBriefcaseId m_currentBriefcaseId;
    TestRepositoryAdmin m_admin;
    TestIModelHubClientForBridges(BeFileNameCR testWorkDir) : m_testWorkDir(testWorkDir), m_currentBriefcaseId(BeSQLite::BeBriefcaseId::Standalone())
        {}

    static BeFileName MakeFakeRepoPath(BeFileNameCR testWorkDir, Utf8CP repoName)
        {
        BeFileName repoPath = testWorkDir;
        repoPath.AppendToPath(L"iModelHub");
        repoPath.AppendToPath(WString(repoName, true).c_str());
        return repoPath;
        }

    bool IsConnected() const override { return true; }

    StatusInt CreateRepository(Utf8CP repoName, BeFileNameCR localDgnDb) override
        {
        m_serverRepo = MakeFakeRepoPath(m_testWorkDir, repoName);
        if (!m_serverRepo.EndsWith(L".bim"))
            m_serverRepo.append(L".bim");
        BeFileName::CreateNewDirectory(m_serverRepo.GetDirectoryName());
        EXPECT_EQ(BeFileNameStatus::Success, BeFileName::BeCopyFile(localDgnDb, m_serverRepo, false));
        return BSISUCCESS;
        }

    StatusInt AcquireBriefcase(BeFileNameCR bcFileName, Utf8CP repositoryName) override
        {
        if (m_serverRepo.empty())
            {
            m_lastServerError = iModel::Hub::Error::Id::iModelDoesNotExist;
            return BSIERROR;
            }
        EXPECT_EQ(BeFileNameStatus::Success, BeFileName::BeCopyFile(m_serverRepo, bcFileName, false));

        auto db = DgnDb::OpenDgnDb(nullptr, bcFileName, DgnDb::OpenParams(DgnDb::OpenMode::ReadWrite));
        m_currentBriefcaseId = m_currentBriefcaseId.GetNextBriefcaseId();
        db->SetAsBriefcase(m_currentBriefcaseId);
        db->SaveChanges();

        return BSISUCCESS;
        }

    StatusInt OpenBriefcase(Dgn::DgnDbR db) override
        {
        m_briefcase = &db;
        return BSISUCCESS;
        }

    void CloseBriefcase() override
        {
        m_briefcase = nullptr;
        }

    StatusInt PullMergeAndPush(Utf8CP) override
        {
        CaptureChangeSet(m_briefcase);
        return BSISUCCESS;
        }

    virtual void CaptureChangeSet(DgnDbP db)
        {
        BeAssert(db != nullptr);

        BeAssert(db->IsBriefcase());

        DgnRevisionPtr changeSet = db->Revisions().StartCreateRevision();

        if (!changeSet.IsValid())
            return;

        Dgn::RevisionStatus status = db->Revisions().FinishCreateRevision();
        BeAssert(Dgn::RevisionStatus::Success == status);
        BeSQLite::DbResult result = db->SaveChanges();
        BeAssert(BeSQLite::BE_SQLITE_OK == result);

        // *** TBD: test for expected changes
        changeSet->Dump(*db);
        }

    StatusInt PullAndMerge() override
        {
        return BSISUCCESS;
        }

    StatusInt PullAndMergeSchemaRevisions(Dgn::DgnDbPtr& db) override
        {
        return BSISUCCESS;
        }

    iModel::Hub::Error const& GetLastError() const override
        {
        return m_lastServerError;
        }

    IRepositoryManagerP GetRepositoryManager(DgnDbR db) override
        {
        return &m_admin;
        }

    StatusInt AcquireLocks(LockRequest&, DgnDbR) override
        {
        return BSISUCCESS;
        }
    };
END_BENTLEY_DGN_NAMESPACE
