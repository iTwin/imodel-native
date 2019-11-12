/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <DgnPlatform/DgnPlatformApi.h>
#include <DgnPlatform/DgnPlatformLib.h>
#include <DgnPlatform/DgnMaterial.h>
#include "../TestFixture/DgnDbTestFixtures.h"

USING_NAMESPACE_BENTLEY_DGN
USING_DGNDB_UNIT_TESTS_NAMESPACE
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_DPTEST

BEGIN_UNNAMED_NAMESPACE

using ResponseOptions = IBriefcaseManager::ResponseOptions;
using RequestPurpose = IBriefcaseManager::RequestPurpose;
using Response = IBriefcaseManager::Response;

#define EXPECT_STATUS(STAT, EXPR) EXPECT_EQ(RepositoryStatus:: STAT, (EXPR))

//=======================================================================================
// We are not testing the repository manager - we are concerned with performance of the
// client-side briefcase manager. Therefore the repository manager just grants whatever
// requests we ask, does not track anything, and implements anything not relevant to our
// tests as a no-op.
// @bsistruct                                                   Paul.Connelly   10/16
//=======================================================================================
struct PerformanceRepositoryManager : IRepositoryManager
    {
    public:
        Response _ProcessRequest(Request const& req, DgnDbR db, bool queryOnly) override;
        RepositoryStatus _Demote(DgnLockSet const& locks, DgnCodeSet const& codes, DgnDbR db) override;
        RepositoryStatus _Relinquish(Resources which, DgnDbR db) override;
        RepositoryStatus _QueryHeldResources(DgnLockSet& locks, DgnCodeSet& codes, DgnLockSet& unavailableLocks, DgnCodeSet& unavailableCodes, DgnDbR db) override;
        RepositoryStatus _QueryStates(DgnLockInfoSet&, DgnCodeInfoSet& codeStates, LockableIdSet const& locks, DgnCodeSet const& codes) override;
    };

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   10/16
//=======================================================================================
struct BriefcasePerformanceTest : public DgnDbTestFixture, DgnPlatformLib::Host::RepositoryAdmin
    {
    public:
        mutable PerformanceRepositoryManager    m_server;

        BriefcasePerformanceTest() { RegisterServer(); }
        ~BriefcasePerformanceTest() { UnregisterServer(); }

        void RegisterServer() { m_host.SetRepositoryAdmin(this); }
        void UnregisterServer() { m_host.SetRepositoryAdmin(nullptr); }

        IRepositoryManagerP _GetRepositoryManager(DgnDbR) const override { return &m_server; }

        DgnDbPtr SetupDb(WCharCP testFile, BeBriefcaseId bcId = BeBriefcaseId(2));

        template<typename T> static void Time(Utf8CP descr, uint32_t numCount, T func)
            {
            printf("%s: ", Utf8String::IsNullOrEmpty(descr) ? "Execution" : descr);

            StopWatch timer(true);
            func();
            timer.Stop();
            printf("%f seconds\n", timer.GetElapsedSeconds());
            LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), numCount, descr);
            }
    };

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   10/16
//=======================================================================================
struct CodesPerformanceTest : BriefcasePerformanceTest
    {
    protected:
        int         m_codeValueIndex = 0;
        int         m_codeValueBatch = 0;

    public:
        void Setup(WCharCP testFile) { m_db = SetupDb(testFile); }

        DgnCode GetNextCode()
            {
            Utf8PrintfString name("%i-%i", m_codeValueBatch, m_codeValueIndex++);
            return RenderMaterial::CreateCode(m_db->GetDictionaryModel(), name, name);
            }

        DgnCodeSet PopulateCodeSet(uint32_t numCodes);
        void TimeReserveCodes(uint32_t numCodes);
    };

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   10/16
//=======================================================================================
struct LocksPerformanceTest : BriefcasePerformanceTest
    {
    protected:
        uint64_t        m_elementId = 0x00ff000000ff0000;

    public:
        void Setup(WCharCP testFile) { m_db = SetupDb(testFile); }

        DgnElementId GetNextElementId() { return DgnElementId(m_elementId++); }

        LockRequest PopulateLockRequest(uint32_t numLocks);
        void TimeAcquireLocks(uint32_t numLocks);
    };

END_UNNAMED_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
Response PerformanceRepositoryManager::_ProcessRequest(Request const& req, DgnDbR db, bool queryOnly)
    {
    Response response(queryOnly ? RequestPurpose::Query : RequestPurpose::Acquire, req.Options());
    response.SetResult(RepositoryStatus::Success);
    return response;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryStatus PerformanceRepositoryManager::_Demote(DgnLockSet const& locks, DgnCodeSet const& codes, DgnDbR db)
    {
    return RepositoryStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryStatus PerformanceRepositoryManager::_Relinquish(Resources which, DgnDbR db)
    {
    return RepositoryStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryStatus PerformanceRepositoryManager::_QueryHeldResources(DgnLockSet& locks, DgnCodeSet& codes, DgnLockSet& unavailableLocks, DgnCodeSet& unavailableCodes, DgnDbR db)
    {
    return RepositoryStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryStatus PerformanceRepositoryManager::_QueryStates(DgnLockInfoSet&, DgnCodeInfoSet& codeStates, LockableIdSet const& locks, DgnCodeSet const& codes)
    {
    return RepositoryStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbPtr BriefcasePerformanceTest::SetupDb(WCharCP testFile, BeBriefcaseId bcId)
    {
    SetupSeedProject(testFile);

    if (!m_db.IsValid())
        return nullptr;

    if (bcId.GetValue() != m_db->GetBriefcaseId().GetValue())
        {
        TestDataManager::MustBeBriefcase(m_db, Db::OpenMode::ReadWrite);
        }

    return m_db;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCodeSet CodesPerformanceTest::PopulateCodeSet(uint32_t numCodes)
    {
    DgnCodeSet codes;
    for (uint32_t i = 0; i < numCodes; i++)
        codes.insert(GetNextCode());

    return codes;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
void CodesPerformanceTest::TimeReserveCodes(uint32_t numCodes)
    {
    DgnCodeSet codes = PopulateCodeSet(numCodes);
    Utf8PrintfString descr("Reserve %u codes", numCodes);
    Time(descr.c_str(), numCodes, [&] () { EXPECT_STATUS(Success, m_db->BriefcaseManager().ReserveCodes(codes).Result()); });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
LockRequest LocksPerformanceTest::PopulateLockRequest(uint32_t numLocks)
    {
    LockRequest req;
    for (uint32_t i = 0; i < numLocks; i++)
        req.GetLockSet().insert(DgnLock(GetNextElementId(), LockLevel::Exclusive));

    return req;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
void LocksPerformanceTest::TimeAcquireLocks(uint32_t numLocks)
    {
    LockRequest req = PopulateLockRequest(numLocks);
    Utf8PrintfString descr("Acquire %u locks", numLocks);
    Time(descr.c_str(), numLocks, [&] () { EXPECT_STATUS(Success, m_db->BriefcaseManager().AcquireLocks(req).Result()); });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CodesPerformanceTest, ReserveCodes)
    {
    Setup(L"ReserveCodes.bim");

    for (uint32_t numCodes = 1; numCodes <= 100000; numCodes *= 10)
        TimeReserveCodes(numCodes);
    }

/*---------------------------------------------------------------------------------**//**
* The more codes we have in our local cache, the slower code reservation becomes, even
* though we request the same number of codes each time.
* @bsimethod                                                    Paul.Connelly   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CodesPerformanceTest, ReserveCodesInChunksOf1000)
    {
    Setup(L"ReserveCodesInChunksOf100.bim");

    for (uint32_t i = 0; i < 10; i++)
        TimeReserveCodes(1000);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(LocksPerformanceTest, AcquireLocks)
    {
    Setup(L"AcquireLocks.bim");

    for (uint32_t numLocks = 1; numLocks <= 100000; numLocks *= 10)
        TimeAcquireLocks(numLocks);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(LocksPerformanceTest, AcquireLocksInChunks)
    {
    Setup(L"AcquireLocksInChunks.bim");

    for (uint32_t i = 0; i < 10; i++)
        TimeAcquireLocks(1000);
    }
