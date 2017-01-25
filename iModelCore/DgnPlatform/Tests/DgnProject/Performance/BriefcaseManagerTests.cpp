/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Performance/BriefcaseManagerTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatform/DgnPlatformApi.h>
#include <Bentley/BeTest.h>
#include <UnitTests/BackDoor/DgnPlatform/ScopedDgnHost.h>
#include <UnitTests/BackDoor/DgnPlatform/DgnDbTestUtils.h>
#include "../TestFixture/GenericDgnModelTestFixture.h"
#include "../BackDoor/PublicAPI/BackDoor/DgnProject/DgnElementHelpers.h"
#include "../BackDoor/PublicAPI/BackDoor/DgnProject/DgnDbUtilities.h"
#include "../BackDoor/PublicAPI/BackDoor/DgnProject/DgnPlatformTestDomain.h"
#include <Bentley/BeTimeUtilities.h>
#include <DgnPlatform/DgnPlatformLib.h>
#include <DgnPlatform/DgnMaterial.h>

USING_NAMESPACE_BENTLEY_DGNPLATFORM
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
struct BriefcasePerformanceTest : ::testing::Test, DgnPlatformLib::Host::RepositoryAdmin
{
public:
    mutable PerformanceRepositoryManager    m_server;
    ScopedDgnHost                           m_host;

    BriefcasePerformanceTest() { RegisterServer(); }
    ~BriefcasePerformanceTest() { UnregisterServer(); }

    void RegisterServer() { m_host.SetRepositoryAdmin(this); }
    void UnregisterServer() { m_host.SetRepositoryAdmin(nullptr); }

    IRepositoryManagerP _GetRepositoryManager(DgnDbR) const override { return &m_server; }

    DgnDbPtr SetupDb(WCharCP testFile, BeBriefcaseId bcId=BeBriefcaseId(2), WCharCP baseFile=L"3dMetricGeneral.ibim");

    template<typename T> static void Time(Utf8CP descr, T func)
        {
        printf("%s: ", Utf8String::IsNullOrEmpty(descr) ? "Execution" : descr);

        StopWatch timer(true);
        func();
        timer.Stop();
        printf("%f seconds\n", timer.GetElapsedSeconds());
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
    DgnDbPtr    m_db;
public:
    void Setup(WCharCP testFile) { m_db = SetupDb(testFile); }

    DgnCode GetNextCode()
        {
        Utf8PrintfString name("%i-%i", m_codeValueBatch, m_codeValueIndex++);
        return DgnMaterial::CreateCode(*m_db, "HOUSE", name);
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
    DgnDbPtr        m_db;
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
DgnDbPtr BriefcasePerformanceTest::SetupDb(WCharCP testFile, BeBriefcaseId bcId, WCharCP baseFile)
    {
    BeFileName testFileName(TEST_FIXTURE_NAME, BentleyCharEncoding::Utf8);
    testFileName.AppendToPath(testFile);

    BeFileName outFileName;
    EXPECT_EQ(SUCCESS, DgnDbTestDgnManager::GetTestDataOut(outFileName, baseFile, testFileName.c_str(), __FILE__));
    auto db = DgnDb::OpenDgnDb(nullptr, outFileName, DgnDb::OpenParams(Db::OpenMode::ReadWrite));
    EXPECT_TRUE(db.IsValid());
    if (!db.IsValid())
        return nullptr;

    if (bcId.GetValue() != db->GetBriefcaseId().GetValue())
        {
        TestDataManager::MustBeBriefcase(db, Db::OpenMode::ReadWrite);
        db->ChangeBriefcaseId(bcId);
        }

    return db;
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
    Time(descr.c_str(), [&]() { EXPECT_STATUS(Success, m_db->BriefcaseManager().ReserveCodes(codes).Result()); });
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
    Time(descr.c_str(), [&]() { EXPECT_STATUS(Success, m_db->BriefcaseManager().AcquireLocks(req).Result()); });
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
TEST_F(CodesPerformanceTest, ReserveCodesInChunksOf100)
    {
    Setup(L"ReserveCodesInChunksOf100.bim");

    for (uint32_t i = 0; i < 100; i++)
        TimeReserveCodes(100);
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

    for (uint32_t i = 0; i < 100; i++)
        TimeAcquireLocks(100);
    }


