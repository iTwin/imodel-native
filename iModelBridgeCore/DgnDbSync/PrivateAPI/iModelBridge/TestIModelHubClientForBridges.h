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

struct RevisionStats
    {
    size_t nSchemaRevs {};
    size_t nDataRevs {};
    bset<Utf8String> descriptions;
    bset<Utf8String> userids;
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
    bvector<DgnRevisionPtr> m_revisions;
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

    size_t GetChangesetCount() const { return m_revisions.size(); }

    bvector<DgnRevision*> GetDgnRevisions(size_t start = 0, size_t end = -1)
        {
        if (end < 0 || end > m_revisions.size())
            end = m_revisions.size();
        bvector<DgnRevision*> revs;
        for (size_t i = start; i < end; ++i)
            revs.push_back(m_revisions[i].get());
        return revs;
        }

    RevisionStats ComputeRevisionStats(DgnDbR db, size_t start = 0, size_t end = -1)
        {
        RevisionStats stats;
        for (auto rev : GetDgnRevisions(start, end))
            {
            stats.descriptions.insert(rev->GetSummary());
            stats.userids.insert(rev->GetUserName());
            if (rev->ContainsSchemaChanges(db))
                ++stats.nSchemaRevs;
            else
                ++stats.nDataRevs;
            }
        return stats;
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

        bvector<DgnRevisionCP> revisions;
        for (DgnRevisionPtr rev : m_revisions)
            revisions.push_back(rev.get());

        auto db = DgnDb::OpenDgnDb(nullptr, bcFileName, DgnDb::OpenParams (DgnDb::OpenMode::ReadWrite));
        m_currentBriefcaseId = m_currentBriefcaseId.GetNextBriefcaseId();
        db->SetAsBriefcase(m_currentBriefcaseId);
        db->SaveChanges();
        db = nullptr;
        if (revisions.empty())
            return BSISUCCESS;

        SchemaUpgradeOptions option(revisions);
        option.SetUpgradeFromDomains(SchemaUpgradeOptions::DomainUpgradeOptions::SkipCheck);// We only want to merge schema revisions. We don't also want to import or upgrade required revisions.
        DgnDb::OpenParams params(DgnDb::OpenMode::ReadWrite, BeSQLite::DefaultTxn::Yes, option);

        db = DgnDb::OpenDgnDb(nullptr, bcFileName, params);
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

    StatusInt JustCaptureRevision(Utf8CP comment)
        {
        DgnRevisionPtr revision = CaptureChangeSet(m_briefcase, comment);
        if (revision.IsNull())
            return BSISUCCESS;
        m_revisions.push_back(revision);
        return BSISUCCESS;
        }

    StatusInt Push(Utf8CP comment) override {return JustCaptureRevision(comment);}
    StatusInt PullMergeAndPush(Utf8CP comment) override {return JustCaptureRevision(comment);}

    virtual DgnRevisionPtr CaptureChangeSet(DgnDbP db, Utf8CP comment)
        {
        BeAssert(db != nullptr);

        BeAssert(db->IsBriefcase());

        DgnRevisionPtr changeSet = db->Revisions().StartCreateRevision();

        if (!changeSet.IsValid())
            return changeSet;

        if (comment)
            changeSet->SetSummary(comment);

        Dgn::RevisionStatus status = db->Revisions().FinishCreateRevision();
        BeAssert(Dgn::RevisionStatus::Success == status);
        BeSQLite::DbResult result = db->SaveChanges();
        BeAssert(BeSQLite::BE_SQLITE_OK == result);

        // *** TBD: test for expected changes
        printf("CaptureChangeset contains_schema_changes? %d user:[%s] desc:[%s]\n", changeSet->ContainsSchemaChanges(*db), changeSet->GetUserName().c_str(), changeSet->GetSummary().c_str());
        changeSet->Dump(*db);
        return changeSet;
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
