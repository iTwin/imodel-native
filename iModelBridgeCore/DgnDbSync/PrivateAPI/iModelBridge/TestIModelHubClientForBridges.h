/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <iModelBridge/IModelClientForBridges.h>
#include <BeSQLite/BeSQLite.h>
//=======================================================================================
// @bsistruct                                                   Sam.Wilson   10/17
//=======================================================================================
BEGIN_BENTLEY_DGN_NAMESPACE

struct TestRepositoryAdmin : IRepositoryManager
    {
    DgnLockInfoSet m_lockDb;
    DgnCodeInfoSet m_codeStates;

    static DgnLock GetSchemaLock(DgnDbR db) {return DgnLock(db.Schemas(), LockLevel::Exclusive);}

    LockLevel GetLevelHeldByBriefcase(DgnLockOwnership const& ownership, BeSQLite::BeBriefcaseId bcId)
        {
        auto const &exc = ownership.GetExclusiveOwner();
        if (exc.IsValid())
            return (exc == bcId)? LockLevel::Exclusive: LockLevel::None;
        auto const &sharedOwners = ownership.GetSharedOwners();
        return (sharedOwners.find(bcId) != sharedOwners.end())? LockLevel::Shared: LockLevel::None;
        }

    DgnLockInfo MakeLockInfo(DgnLock const& lock)
        {
        return DgnLockInfo(lock.GetLockableId());
        }


    void ReleaseLock(DgnLockInfo& held, BeSQLite::BeBriefcaseId bcId, Utf8StringCR revisionId)
        {
        DgnLockOwnership& heldOwnership = held.GetOwnership();

        if (heldOwnership.GetExclusiveOwner() == bcId)
            {
            held.Reset();
            held.SetRevisionId(revisionId);
            BeAssert(!held.IsTracked());
            return;
            }

        auto sharedOwners = heldOwnership.GetSharedOwners();
        BeAssert(sharedOwners.find(bcId) != sharedOwners.end());
        sharedOwners.erase(bcId);
        if (sharedOwners.empty())
            {
            held.Reset();
            held.SetRevisionId(revisionId);
            BeAssert(!held.IsTracked());
            return;
            }

        heldOwnership.Reset();
        for (auto const& otherOwner: sharedOwners)
            heldOwnership.AddSharedOwner(otherOwner);

        held.SetRevisionId(revisionId);
        }

    void TakeUnownedLock(DgnLock const& requestedLock, BeSQLite::BeBriefcaseId bcId, Utf8StringCR clientParentRevisionId)
        {
        auto iHeld = m_lockDb.find(MakeLockInfo(requestedLock));

        if (iHeld == m_lockDb.end())
            iHeld = m_lockDb.insert(DgnLockInfo(requestedLock.GetLockableId(), true)).first;
        else
            {
            BeAssert(iHeld->GetOwnership().GetLockLevel() == LockLevel::None);
    
            // TODO: if indexof(clientParentRevisionId) < indexof(held.GetRevisionId())
            //          Pull is required
            }

        DgnLockOwnership newOwner;
        if (requestedLock.GetLevel() == LockLevel::Exclusive)
            newOwner.SetExclusiveOwner(bcId);
        else
            newOwner.AddSharedOwner(bcId);

        iHeld->SetTracked();
        iHeld->GetOwnership() = newOwner;
        }

    Response ProcessQuery(Request const& req, DgnDbR db)
        {
        auto bcId = db.GetBriefcaseId();
        Response response(IBriefcaseManager::RequestPurpose::Query, req.Options());
        response.SetResult(RepositoryStatus::Success);
        auto& requestedLockSet = req.Locks().GetLockSet();
        for (auto const& requestedLock: requestedLockSet)
            {
            auto iHeld = m_lockDb.find(MakeLockInfo(requestedLock));
            if (iHeld != m_lockDb.end())
                response.LockStates().insert(*iHeld);       // *** NEEDS WORK: Should return only the locks that the briefcase owns?
            }
        return response;
        } 

    void ProcessedUpdateLockRequest(Response& response, DgnLock const& requestedLock, BeSQLite::BeBriefcaseId bcId, Utf8StringCR clientParentRevisionId)
        {
        auto requestedLevel = requestedLock.GetLevel();
        BeAssert(requestedLevel > LockLevel::None && "Should use _Demote to downgrade or release locks");

        auto iHeld = m_lockDb.find(MakeLockInfo(requestedLock));

        if ((iHeld == m_lockDb.end()) || (iHeld->GetOwnership().GetLockLevel() == LockLevel::None))
            {
            TakeUnownedLock(requestedLock, bcId, clientParentRevisionId);
            return;
            }

        // Lock is held.
        auto& held = *iHeld;
        auto& heldOwnership = held.GetOwnership();

        auto levelHeldByThisBriefcase = GetLevelHeldByBriefcase(heldOwnership, bcId);  // May be LockLevel::None
        if (levelHeldByThisBriefcase >= requestedLevel)
            {
            return;
            }

        if (heldOwnership.GetLockLevel() == LockLevel::Exclusive)
            {
            BeAssert(heldOwnership.GetExclusiveOwner() != bcId);
            response.SetResult(RepositoryStatus::LockAlreadyHeld);
            response.LockStates().insert(*iHeld);
            return;
            }

        BeAssert(heldOwnership.GetLockLevel() == LockLevel::Shared);
        BeAssert(levelHeldByThisBriefcase <= LockLevel::Shared); // if the lock was already held at the exclusive level, we would have returned early before getting here.

        if (requestedLevel == LockLevel::Exclusive)
            {
            if (levelHeldByThisBriefcase == LockLevel::Shared)
                {
                auto sharedOwners = heldOwnership.GetSharedOwners();
                BeAssert(sharedOwners.find(bcId) != sharedOwners.end());
                if (sharedOwners.size() == 1)
                    {
                    heldOwnership.SetExclusiveOwner(bcId);
                    return;
                    }
                response.SetResult(RepositoryStatus::LockAlreadyHeld);  // There are other shared owners. Can't upgrade to Exclusive.
                response.LockStates().insert(*iHeld);
                return;
                }
            }

        BeAssert(requestedLevel == LockLevel::Shared);
        BeAssert(levelHeldByThisBriefcase <= LockLevel::None);
        heldOwnership.AddSharedOwner(bcId);
        }

    Response ProcessUpdate(Request const& req, DgnDbR db)
        {
        auto bcId = db.GetBriefcaseId();
        Response response(IBriefcaseManager::RequestPurpose::Query, req.Options());
        response.SetResult(RepositoryStatus::Success);
        for (auto const& l: req.Locks().GetLockSet())
            {
            ProcessedUpdateLockRequest(response, l, bcId, db.Revisions().GetParentRevisionId());
            }
        return response;
        }

    virtual Response _ProcessRequest(Request const& req, DgnDbR db, bool queryOnly)
        {
        return queryOnly? ProcessQuery(req, db): ProcessUpdate(req, db);
        }

    virtual RepositoryStatus _Demote(DgnLockSet const& locks, DgnCodeSet const& codes, DgnDbR db)
        {
        auto bcId = db.GetBriefcaseId();
        for (auto const& lock: locks)
            {
            auto iHeld = m_lockDb.find(MakeLockInfo(lock));
            if (iHeld == m_lockDb.end())
                return RepositoryStatus::LockNotHeld;
            auto& held = *iHeld;
            auto& heldOwnership = held.GetOwnership();
            auto requestedLevel = lock.GetLevel();
            if (requestedLevel == LockLevel::None)
                ReleaseLock(*iHeld, bcId, db.Revisions().GetParentRevisionId());
            else
                {
                BeAssert(requestedLevel == LockLevel::Shared);
                BeAssert (heldOwnership.GetExclusiveOwner() == bcId);
                heldOwnership.Reset();
                heldOwnership.AddSharedOwner(bcId);
                }
            }
        return RepositoryStatus::Success;
        }
    virtual RepositoryStatus _Relinquish(Resources which, DgnDbR db)
        {
        auto bcId = db.GetBriefcaseId();
        for (auto& lockInfo: m_lockDb)
            {
            if (GetLevelHeldByBriefcase(lockInfo.GetOwnership(), bcId) == LockLevel::None)
                continue;
            ReleaseLock(lockInfo, bcId, db.Revisions().GetParentRevisionId());
            }
        return RepositoryStatus::Success;
        }
    virtual RepositoryStatus _QueryHeldResources(DgnLockSet& locks, DgnCodeSet& codes, DgnLockSet& unavailableLocks, DgnCodeSet& unavailableCodes, DgnDbR db)
        {
        auto bcId = db.GetBriefcaseId();
        for (auto const& lockInfo: m_lockDb)
            {
            DgnLock lock(lockInfo.GetLockableId(), lockInfo.GetOwnership().GetLockLevel());
            if (GetLevelHeldByBriefcase(lockInfo.GetOwnership(), bcId) != LockLevel::None)
               locks.insert(lock);
            else if (lockInfo.GetOwnership().GetLockLevel() != LockLevel::None)
                unavailableLocks.insert(lock);
            }
        return RepositoryStatus::Success;
        }
    virtual RepositoryStatus _QueryStates(DgnLockInfoSet& lockStates, DgnCodeInfoSet& codeStates, LockableIdSet const& locks, DgnCodeSet const& codes)
        {
        for (auto const& l: locks)
            {
            auto i = m_lockDb.find(DgnLockInfo(l));
            if (i == m_lockDb.end())
                lockStates.insert(DgnLockInfo(l));
            else
                lockStates.insert(*i);
            }
        return RepositoryStatus::Success;
        }
    };

struct TestIModelHubClientForBridges : IModelHubClientForBridges
    {
    iModel::Hub::iModelInfoPtr m_iModelInfo;
    bool anyTxnsInFile(DgnDbR db)
        {
        BeSQLite::Statement stmt;
        stmt.Prepare(db, "SELECT Id FROM " DGN_TABLE_Txns " LIMIT 1");
        return (BeSQLite::BE_SQLITE_ROW == stmt.Step());
        }

    iModel::Hub::Error m_lastServerError;
    BeFileName m_serverRepo;
    BeFileName m_testWorkDir;
    bvector<std::pair <DgnRevisionPtr, Utf8String>> m_revisions;
    DgnDbP m_briefcase{};
    BeSQLite::BeBriefcaseId m_currentBriefcaseId;
    TestRepositoryAdmin m_admin;
    TestIModelHubClientForBridges(BeFileNameCR testWorkDir) : m_testWorkDir(testWorkDir), m_currentBriefcaseId(BeSQLite::BeBriefcaseId::LegacyStandalone())
        {
        m_iModelInfo = new iModel::Hub::iModelInfo();
        }
    
    static BeFileName MakeFakeRepoPath(BeFileNameCR testWorkDir, Utf8CP repoName)
        {
        BeFileName repoPath = testWorkDir;
        repoPath.AppendToPath(L"iModelHub");
        repoPath.AppendToPath(WString(repoName, true).c_str());
        return repoPath;
        }

    iModel::Hub::iModelInfoPtr GetIModelInfo() override {return m_iModelInfo;}
    
    bvector<std::pair <DgnRevisionPtr, Utf8String>> GetDgnRevisions(size_t start = 0, size_t end = -1)
        {
        if (end < 0 || end > m_revisions.size())
            end = m_revisions.size();
        bvector<std::pair <DgnRevisionPtr, Utf8String>> revs;
        for (size_t i = start; i < end; ++i)
            revs.push_back(m_revisions[i]);
        return revs;
        }

    bool IsConnected() const override { return true; }

    BentleyStatus DeleteRepository() override
        {
        BeAssert(nullptr == m_briefcase);
        if (m_serverRepo.empty() || !m_serverRepo.DoesPathExist())
            {
            m_lastServerError = iModel::Hub::Error::Id::iModelDoesNotExist;
            return BSIERROR;
            }
        EXPECT_EQ(BeFileNameStatus::Success, BeFileName::EmptyAndRemoveDirectory(m_serverRepo.GetDirectoryName()));
        m_serverRepo.clear();
        m_revisions.clear();
        m_briefcase = nullptr;
        m_currentBriefcaseId = BeSQLite::BeBriefcaseId(BeSQLite::BeBriefcaseId::LegacyStandalone());
        return BSISUCCESS;
        }

    StatusInt CreateRepository(Utf8CP repoName) override
        {
        BeAssert(nullptr == m_briefcase);
        if (!m_serverRepo.empty() && m_serverRepo.DoesPathExist())
            {
            BeAssert(false);
            m_lastServerError = iModel::Hub::Error::Id::iModelAlreadyExists;
            return BSIERROR;
            }
        m_serverRepo = MakeFakeRepoPath(m_testWorkDir, repoName);
        if (!m_serverRepo.EndsWith(L".bim"))
            m_serverRepo.append(L".bim");
        EXPECT_EQ(BeFileNameStatus::Success, BeFileName::CreateNewDirectory(m_serverRepo.GetDirectoryName()));
        //
    //  We need to create a new repository.
    //
        CreateDgnDbParams createProjectParams;

        Utf8String rootSubjName(m_serverRepo);
        createProjectParams.SetRootSubjectName(rootSubjName.c_str());

        // Create the DgnDb file. All currently registered domain schemas are imported.
        BeSQLite::DbResult createStatus;
        auto db = DgnDb::CreateDgnDb(&createStatus, m_serverRepo, createProjectParams);
        if (!db.IsValid())
            return BSIERROR;

        db->SaveChanges();
        return BSISUCCESS;
        }

    StatusInt AcquireBriefcase(BeFileNameCR bcFileName, Utf8CP repositoryName) override
        {
        BeAssert(nullptr == m_briefcase);
        if (m_serverRepo.empty())
            {
            m_lastServerError = iModel::Hub::Error::Id::iModelDoesNotExist;
            return BSIERROR;
            }
        EXPECT_EQ(BeFileNameStatus::Success, BeFileName::BeCopyFile(m_serverRepo, bcFileName, false));

        bvector<DgnRevisionCP> revisions;
        for (auto rev : m_revisions)
            revisions.push_back(rev.first.get());

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
        BeAssert(nullptr == m_briefcase);
        m_briefcase = &db;
        return BSISUCCESS;
        }

    void CloseBriefcase() override
        {
        m_briefcase = nullptr;
        }

    StatusInt JustCaptureRevision(Utf8CP comment, Utf8StringCP fileName)
        {
        BeAssert(nullptr != m_briefcase);

        DgnRevisionPtr revision = CaptureChangeSet(m_briefcase, comment);
        if (revision.IsNull())
            return BSISUCCESS;
        m_revisions.push_back(std::make_pair(revision, fileName ? *fileName : ""));
        return BSISUCCESS;
        }

    StatusInt Push(iModel::Hub::PushChangeSetArgumentsPtr pushArgs) override {return JustCaptureRevision(pushArgs->GetDescription(), &pushArgs->GetBridgeProperties()->GetChangedFiles().front());}
    StatusInt PullMergeAndPush(iModel::Hub::PullChangeSetsArgumentsPtr, iModel::Hub::PushChangeSetArgumentsPtr pushArgs) override 
        {
        Utf8StringCP fileName = pushArgs->GetBridgeProperties().IsValid() ? &pushArgs->GetBridgeProperties()->GetChangedFiles().front() : NULL;
        return JustCaptureRevision(pushArgs->GetDescription(), fileName);
        }
    StatusInt RestoreBriefcase (BeFileNameCR, Utf8CP, BeSQLite::BeBriefcaseId) override {return ERROR;}
    virtual DgnRevisionPtr CaptureChangeSet(DgnDbP db, Utf8CP comment)
        {
        BeAssert(db != nullptr);
        BeAssert(!db->IsLegacyMaster());

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
        // printf("CaptureChangeset contains_schema_changes? %d user:[%s] desc:[%s]\n", changeSet->ContainsSchemaChanges(*db), changeSet->GetUserName().c_str(), changeSet->GetSummary().c_str());
        //changeSet->Dump(*db);
        return changeSet;
        }

    StatusInt PullAndMerge(iModel::Hub::PullChangeSetsArgumentsPtr) override
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
