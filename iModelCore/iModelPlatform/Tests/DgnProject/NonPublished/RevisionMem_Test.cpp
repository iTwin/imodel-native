/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ChangeTestFixture.h"
#include <DgnPlatform/DgnChangeSummary.h>
#include <DgnPlatform/GenericDomain.h>
#include <array>
#include <UnitTests/BackDoor/DgnPlatform/DgnPlatformTestDomain.h>

// #define DEBUG_REVISION_TEST_MANUAL 1
#ifdef DEBUG_REVISION_TEST_MANUAL
#include <windows.h>
#endif

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_DPTEST
USING_NAMESPACE_BENTLEY_EC

#define LOG (*BentleyApi::NativeLogging::LoggingManager::GetLogger (L"DgnCore"))

// Turn this on for debugging.
// #define DUMP_REVISION 1
// #define DUMP_CODES

//=======================================================================================
// @bsiclass
//=======================================================================================
struct RevisionMemTestFixture : ChangeTestFixture
{
DEFINE_T_SUPER(ChangeTestFixture)
protected:
    int m_z = 0;
    WCharCP m_copyTestFileName = L"RevisionMemTestFixture.bim";


    DgnRevisionPtr CreateRevision();
    void DumpRevision(DgnRevisionCR revision, Utf8CP summary = nullptr);

    void BackupTestFile();
    void RestoreTestFile(Db::OpenMode openMode = Db::OpenMode::ReadWrite);

    void ExtractCodesFromRevision(DgnCodeSet& assigned, DgnCodeSet& discarded);
    void ProcessSchemaRevision(DgnRevisionCR revision, RevisionProcessOption revisionProcessOption);
    void MergeSchemaRevision(DgnRevisionCR revision) { ProcessSchemaRevision(revision, RevisionProcessOption::Merge); }
    void ReverseSchemaRevision(DgnRevisionCR revision) { ProcessSchemaRevision(revision, RevisionProcessOption::Reverse); }
    void ReinstateSchemaRevision(DgnRevisionCR revision) { ProcessSchemaRevision(revision, RevisionProcessOption::Merge); }

    static Utf8String CodeToString(DgnCodeCR code) { return Utf8PrintfString("%s:%s\n", code.GetScopeString().c_str(), code.GetValueUtf8CP()); }
    static void ExpectCode(DgnCodeCR code, DgnCodeSet const& codes) { EXPECT_FALSE(codes.end() == codes.find(code)) << CodeToString(code).c_str(); }
    static void ExpectCodes(DgnCodeSet const& exp, DgnCodeSet const& actual)
        {
        EXPECT_EQ(exp.size(), actual.size());
        for (auto const& code : exp)
            ExpectCode(code, actual);
        }

    static void DumpCode(DgnCodeCR code) { printf("    %s\n", CodeToString(code).c_str()); }
    static void DumpCodes(DgnCodeSet const& codes, Utf8StringCR msg="Codes:")
        {
#ifdef DUMP_CODES
        printf("%s\n", msg.c_str());
        for (auto const& code : codes)
            DumpCode(code);
#endif
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    TestElementCPtr InsertElement(Utf8StringCR val)
        {
        DgnDbR db = m_defaultModel->GetDgnDb();
        auto el = TestElement::Create(db, m_defaultModelId, m_defaultCategoryId);
        el->SetFederationGuid(BeGuid(false));

        el->SetTestElementProperty(val);
        auto cpEl = db.Elements().Insert(*el);
        EXPECT_TRUE(cpEl.IsValid());
        return cpEl;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    void UpdateRootProperty(Utf8StringCR val, DgnElementId eId)
        {
        auto el = m_db->Elements().GetForEdit<TestElement>(eId);
        ASSERT_TRUE(el.IsValid());
        el->SetTestElementProperty(val);
        ASSERT_TRUE(DgnDbStatus::Success == el->Update());

        }
public:
    static DgnPlatformSeedManager::SeedDbInfo s_seedFileInfo;
    static void SetUpTestCase();

    RevisionMemTestFixture() {}
};

DgnPlatformSeedManager::SeedDbInfo RevisionMemTestFixture::s_seedFileInfo;
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void RevisionMemTestFixture::SetUpTestCase()
    {
    //Start from parent fixture's seed Db
    ScopedDgnHost tempHost;

    //  Request a root seed file.
    DgnPlatformSeedManager::SeedDbInfo rootSeedInfo = DgnPlatformSeedManager::GetSeedDb(ChangeTestFixture::s_seedFileInfo.id, DgnPlatformSeedManager::SeedDbOptions(true, true));

    //  The group's seed file is essentially the same as the root seed file, but with a different relative path.
    //  Note that we must put our group seed file in a group-specific sub-directory
    RevisionMemTestFixture::s_seedFileInfo = rootSeedInfo;
    RevisionMemTestFixture::s_seedFileInfo.fileName.SetName(L"RevisionMemTestFixture/RevisionMemTestFixture.bim");

    DgnDbPtr db = DgnPlatformSeedManager::OpenSeedDbCopy(rootSeedInfo.fileName, RevisionMemTestFixture::s_seedFileInfo.fileName); // our seed starts as a copy of the root seed
    ASSERT_TRUE(db.IsValid());
    TestDataManager::SetAsStandaloneDb(db, Db::OpenMode::ReadWrite);

    m_defaultCodeSpecId = DgnDbTestUtils::InsertCodeSpec(*db, "TestCodeSpec");
    ASSERT_TRUE(m_defaultCodeSpecId.IsValid());

    db->SaveChanges();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void RevisionMemTestFixture::DumpRevision(DgnRevisionCR revision, Utf8CP summary)
    {
#ifdef DUMP_REVISION
    LOG.infov("---------------------------------------------------------");
    if (summary != nullptr)
        LOG.infov(summary);
    revision.Dump(*m_db);
    LOG.infov("---------------------------------------------------------");
#endif
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DgnRevisionPtr RevisionMemTestFixture::CreateRevision()
    {
    DgnRevisionPtr revision = m_db->Revisions().StartCreateRevision();
    if (!revision.IsValid())
        return nullptr;

    RevisionStatus status = m_db->Revisions().FinishCreateRevision(-1);
    if (RevisionStatus::Success != status)
        {
        BeAssert(false);
        return nullptr;
        }

    return revision;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void RevisionMemTestFixture::ProcessSchemaRevision(DgnRevisionCR revision, RevisionProcessOption revisionProcessOption)
    {
    BeFileName fileName = BeFileName(m_db->GetDbFileName(), true);
    CloseDgnDb();

    DbResult openStatus;
    DgnDb::OpenParams openParams(Db::OpenMode::ReadWrite, BeSQLite::DefaultTxn::Yes, SchemaUpgradeOptions(revision, revisionProcessOption));
    m_db = DgnDb::OpenIModelDb(&openStatus, fileName, openParams);
    ASSERT_TRUE(m_db.IsValid()) << "Could not open test project";

    m_defaultCodeSpec = m_db->CodeSpecs().GetCodeSpec(m_defaultCodeSpecId);
    ASSERT_TRUE(m_defaultCodeSpec.IsValid());

    m_defaultModel = m_db->Models().Get<PhysicalModel>(m_defaultModelId);
    ASSERT_TRUE(m_defaultModel.IsValid());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void RevisionMemTestFixture::BackupTestFile()
    {
    BeFileName fileName = BeFileName(m_db->GetDbFileName(), true);
    CloseDgnDb();
    BeFileName originalFile = fileName;
    BeFileName copyFile(originalFile.GetDirectoryName()); //DgnDbTestDgnManager::GetOutputFilePath(m_copyTestFileName);
    copyFile.AppendToPath(m_copyTestFileName);
    //BeFileName copyFile = DgnDbTestDgnManager::GetOutputFilePath(m_copyTestFileName);

    BeFileNameStatus fileStatus = BeFileName::BeCopyFile(originalFile.c_str(), copyFile.c_str());
    ASSERT_TRUE(fileStatus == BeFileNameStatus::Success);
    OpenIModelDb(fileName);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void RevisionMemTestFixture::RestoreTestFile(Db::OpenMode openMode /*= Db::OpenMode::ReadWrite*/)
    {
    BeFileName fileName = BeFileName(m_db->GetDbFileName(), true);
    CloseDgnDb();
    BeFileName backUp = fileName;
    backUp.AppendUtf8("-back.db");
    BeFileName::BeCopyFile(fileName.c_str(), backUp.c_str());

    BeFileName originalFile = fileName;
    BeFileName copyFile(originalFile.GetDirectoryName());
    copyFile.AppendToPath(m_copyTestFileName);

    BeFileNameStatus fileStatus = BeFileName::BeCopyFile(copyFile.c_str(), originalFile.c_str());
    ASSERT_TRUE(fileStatus == BeFileNameStatus::Success);
    OpenIModelDb(fileName, openMode);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(RevisionMemTestFixture, Commit_error_cause_auto_rollback_and_exception) {
    // Setup a test file
    SetupDgnDb(RevisionMemTestFixture::s_seedFileInfo.fileName, L"commit_error.bim");
    m_db->SaveChanges("Created Initial Model");
    auto dbFileName = m_db->GetFileName();
    auto elId = InsertElement("Sample Property")->GetElementId();
    TxnManager::SetOnCommitCallback([](TxnManager&, bool isCommit, Utf8CP, BeSQLite::ChangeSetCR, BeSQLite::DdlChangesCR) {
        return CallbackOnCommitStatus::Abort;
    });

    try {
        m_db->SaveChanges("test abort");
        FAIL();
    } catch (std::exception e){
        SUCCEED();
        m_db = nullptr;
    }
    TxnManager::SetOnCommitCallback(nullptr);
    OpenIModelDb(dbFileName, Db::OpenMode::Readonly);
    // check if the element w3e inserted is still there.
    ASSERT_FALSE(m_db->Elements().Get<TestElement>(elId).IsValid());
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(RevisionMemTestFixture, Donot_commit_if_change_tracking_has_changes_on_close) {
    // Setup a test file
    SetupDgnDb(RevisionMemTestFixture::s_seedFileInfo.fileName, L"commit_error.bim");
    m_db->SaveChanges("Created Initial Model");
    auto dbFileName = m_db->GetFileName();
    auto elId = InsertElement("Sample Property")->GetElementId();
    BeTest::SetFailOnAssert(false);
    m_db->CloseDb();
    BeTest::SetFailOnAssert(true);
    OpenIModelDb(dbFileName, Db::OpenMode::Readonly);
    // check if the element w3e inserted is still there.
    ASSERT_FALSE(m_db->Elements().Get<TestElement>(elId).IsValid());
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(RevisionMemTestFixture, Donot_commit_if_change_tracking_has_changes_when_db_destructor_is_called) {
    // Setup a test file
    SetupDgnDb(RevisionMemTestFixture::s_seedFileInfo.fileName, L"commit_error.bim");
    m_db->SaveChanges("Created Initial Model");
    auto dbFileName = m_db->GetFileName();
    auto elId = InsertElement("Sample Property")->GetElementId();
    BeTest::SetFailOnAssert(false);
    m_db = nullptr;
    BeTest::SetFailOnAssert(true);
    OpenIModelDb(dbFileName, Db::OpenMode::Readonly);
    // check if the element w3e inserted is still there.
    ASSERT_FALSE(m_db->Elements().Get<TestElement>(elId).IsValid());
}

#ifdef ONLY_ON_DEMAND
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(RevisionMemTestFixture, try_saving_2gb_changeset_in_dgn_txns) {
    #ifdef BENTLEYCONFIG_OS_APPLE_MACOS
        // skipping this test for MacOS.
        return;
    #endif
    //!!! This test require atleast 4 Gb of memory and 4 gb of disk space.
    SetupDgnDb(RevisionMemTestFixture::s_seedFileInfo.fileName, L"txn_spanning_overflow_columns.bim");

    // we generate random string so it cannot be compressed as most compression algorithm cannot compress uniform distributed data.
    auto generateRandomString = [](Utf8StringR str,int size) {
        str.clear();
        str.reserve(size);
        for(auto i=0; i< size; ++i) {
            str.append(1, static_cast<char>(0x20+((float)rand()/RAND_MAX)*0x5E));
        }
    };

    Utf8String randomStr;
    generateRandomString(randomStr, 10*1000*1000);
    m_db->SaveChanges("initial model");
    auto dbFileName = m_db->GetFileName();


    for (int i=0;i<200; ++i) { //2.0 GB
        InsertElement(randomStr);
    }
    ASSERT_EQ(m_db->Txns().GetMemoryUsed(), 60576);

    // values down here is from debugging snappy fail to compress random data in fact headers increased the size by 44kbytes
    /* ------changeset size--------
        compress_size  = 2000517828
        changeset_size = 2000073255
       ---------------------------- */
    // following function throws exception in case changeset was unable to save.
    ASSERT_EQ(BE_SQLITE_OK, m_db->SaveChanges("big change"));

    //Now revert all changes.
    ASSERT_EQ(DgnDbStatus::Success, m_db->Txns().ReverseAll());
    ASSERT_EQ(BE_SQLITE_OK, m_db->SaveChanges("back to seed"));
}
#endif

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(RevisionMemTestFixture, changeset_size_api) {

    //!!! This test require atleast 4 Gb of memory and 4 gb of disk space.
    SetupDgnDb(RevisionMemTestFixture::s_seedFileInfo.fileName, L"changeset_size_api.bim");

    // we generate random string so it cannot be compressed as most compression algorithm cannot compress uniform distributed data.
    auto generateRandomString = [](Utf8StringR str,int size) {
        str.clear();
        str.reserve(size);
        for(auto i=0; i< size; ++i) {
            str.append(1, static_cast<char>(0x20+((float)rand()/RAND_MAX)*0x5E));
        }
    };

    Utf8String randomStr;
    generateRandomString(randomStr, 24*1024);
    m_db->SaveChanges("initial model");
    auto dbFileName = m_db->GetFileName();
    for (int i=0;i<10; ++i) {
        InsertElement(randomStr);
    }
    ASSERT_EQ(0, m_db->Txns().GetChangesetSize()) << "Disable by default";
    ASSERT_EQ(BE_SQLITE_MISUSE, m_db->Txns().EnableChangesetSizeStats(true));
    m_db->SaveChanges();
    ASSERT_EQ(BE_SQLITE_OK, m_db->Txns().EnableChangesetSizeStats(true));

    for (int i=0;i<10; ++i) {
        InsertElement(randomStr);
    }

    ASSERT_EQ(m_db->Txns().GetChangesetSize(), 249572);
    #ifdef BENTLEYCONFIG_OS_APPLE_MACOS
        ASSERT_EQ(m_db->Txns().GetMemoryUsed(), 10448);
    #else
        ASSERT_EQ(m_db->Txns().GetMemoryUsed(), 10352);
    #endif

    m_db->SaveChanges();
    ASSERT_EQ(m_db->Txns().GetChangesetSize(), 0);
    ASSERT_EQ(m_db->Txns().GetMemoryUsed(), 0);
}