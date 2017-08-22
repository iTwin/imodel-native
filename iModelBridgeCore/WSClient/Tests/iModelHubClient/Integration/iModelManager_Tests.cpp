/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/iModelHubClient/Integration/iModelManager_Tests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "IntegrationTestsBase.h"
#include "IntegrationTestsHelper.h"
#include <WebServices/iModelHub/Client/Client.h>
#include <WebServices/iModelHub/Client/iModelConnection.h>
#include <DgnPlatform/DgnPlatformLib.h>
#include <Bentley/BeTest.h>
#include <DgnPlatform/DgnMaterial.h>
#include <ECObjects/ECSchema.h>
#include "../BackDoor/PublicAPI/BackDoor/WebServices/iModelHub/BackDoor.h"
#include <DgnPlatform/DgnDbTables.h>
#include "MockHttpHandler.h"

USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_IMODELHUB
USING_NAMESPACE_BENTLEY_IMODELHUB_UNITTESTS

#define EXPECT_STATUS(STAT, EXPR)          EXPECT_EQ(RepositoryStatus:: STAT, (EXPR))
#define BIS_ECSCHEMA_CLASS_NAME(className) BIS_ECSCHEMA_NAME "." className

//---------------------------------------------------------------------------------------
//@bsimethod                                     Eligijus.Mauragas             02/2016
//---------------------------------------------------------------------------------------
struct iModelManagerTests: public IntegrationTestsBase
    {
    ClientPtr    m_client;
    iModelInfoPtr m_imodel;
    iModelConnectionPtr m_connection;

    virtual void SetUp () override
        {
        IntegrationTestsBase::SetUp ();

        auto proxy   = ProxyHttpHandler::GetFiddlerProxyIfReachable(); 
        m_client     = SetUpClient (IntegrationTestSettings::Instance().GetValidHost(), IntegrationTestSettings::Instance().GetValidAdminCredentials(), proxy);
        m_imodel = CreateNewiModel (*m_client, nullptr);
        m_connection = ConnectToiModel(*m_client, m_imodel);

        m_pHost->SetRepositoryAdmin (m_client->GetiModelAdmin());
        }

    virtual void TearDown () override
        {
        DeleteiModel(*m_client, *m_imodel);
        m_client = nullptr;
        IntegrationTestsBase::TearDown ();
        }

    IRepositoryManagerP _GetRepositoryManager(DgnDbR db) const
        {
        return m_client->GetiModelAdmin()->_GetRepositoryManager(db);
        }

    BriefcasePtr AcquireBriefcase()
        {
        return IntegrationTestsBase::AcquireBriefcase(*m_client, *m_imodel);
        }

    Utf8String GetScopeString(CodeSpecPtr codeSpec, DgnElementCR scopeElement)
        {
        return codeSpec->GetScopeElementId(scopeElement).ToString(BeInt64Id::UseHex::Yes);
        }

    Utf8String GetNonAdminUserId()
        {
        auto nonAdminClient = SetUpClient(IntegrationTestSettings::Instance().GetValidHost(), IntegrationTestSettings::Instance().GetValidNonAdminCredentials());
        auto briefcase2 = IntegrationTestsBase::AcquireBriefcase(*nonAdminClient, *m_imodel, true);
        auto connection = briefcase2->GetiModelConnectionPtr();
        return connection->QueryBriefcaseInfo(briefcase2->GetBriefcaseId())->GetResult().GetValue()->GetUserOwned();
        }
    };

enum CodeState : uint8_t
    {
    Available,  //!< The Code can be reserved for use by any briefcase
    Reserved,   //!< The Code has been reserved for use by a briefcase
    Used,       //!< A changeSet has been committed to the server in which the Code was used by an object.
    Discarded,  //!< A changeSet has been committed to the server in which the Code became discarded by the object by which it was previously used.
    };

//---------------------------------------------------------------------------------------
//@bsimethod                                     julius.cepukenas             08/2016
//---------------------------------------------------------------------------------------
CodeLockSetTaskPtr QueryCodesLocksById(Briefcase& briefcase, bool byBriefcaseId, DgnCodeSet& codes, LockableIdSet& ids)
    {
    if (byBriefcaseId)
        return briefcase.GetiModelConnection().QueryCodesLocksById(codes, ids, briefcase.GetBriefcaseId());

    return briefcase.GetiModelConnection().QueryCodesLocksById(codes, ids);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Eligijus.Mauragas             01/2016
//---------------------------------------------------------------------------------------
void ExpectLocksCountById (Briefcase& briefcase, int expectedCount, bool byBriefcaseId, LockableIdSet& ids)
    {
    DgnCodeSet codes;
    auto result = QueryCodesLocksById(briefcase, byBriefcaseId, codes, ids)->GetResult();

    EXPECT_SUCCESS(result);

    int locksCount = 0;
    for (DgnLockInfo lockState : result.GetValue ().GetLockStates ())
        {
        if (LockLevel::Exclusive == lockState.GetOwnership ().GetLockLevel ())
            locksCount++;
        else
            {
            for (auto owner : lockState.GetOwnership ().GetSharedOwners ())
                locksCount++;
            }
        }
    EXPECT_TRUE (expectedCount == locksCount);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Eligijus.Mauragas             01/2016
//---------------------------------------------------------------------------------------
void ExpectLocksCountById (Briefcase& briefcase, int expectedCount, bool byBriefcaseId, LockableId id1, LockableId id2)
    {
    LockableIdSet ids;
    ids.insert (id1);
    ids.insert (id2);
    ExpectLocksCountById (briefcase, expectedCount, byBriefcaseId, ids);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Eligijus.Mauragas             01/2016
//---------------------------------------------------------------------------------------
void ExpectLocksCountById (Briefcase& briefcase, int expectedCount, bool byBriefcaseId, LockableId id1, LockableId id2, LockableId id3)
    {
    LockableIdSet ids;
    ids.insert (id1);
    ids.insert (id2);
    ids.insert (id3);
    ExpectLocksCountById (briefcase, expectedCount, byBriefcaseId, ids);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Eligijus.Mauragas             01/2016
//---------------------------------------------------------------------------------------
RepositoryStatus AcquireLock (DgnDbR db, LockLevel level = LockLevel::Exclusive)
    {
    LockRequest req;
    req.Insert (db, level);
    return db.BriefcaseManager ().AcquireLocks (req, IBriefcaseManager::ResponseOptions::None).Result ();
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Eligijus.Mauragas             01/2016
//---------------------------------------------------------------------------------------
RepositoryStatus AcquireLock (DgnDbR db, DgnElementCR element, LockLevel level = LockLevel::Exclusive)
    {
    LockRequest req;
    req.Insert (element, level);
    return db.BriefcaseManager ().AcquireLocks (req, IBriefcaseManager::ResponseOptions::None).Result ();
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Eligijus.Mauragas             01/2016
//---------------------------------------------------------------------------------------
RepositoryStatus DemoteLock (DgnModelCR model, LockLevel level = LockLevel::None)
    {
    DgnLockSet toRelease;
    toRelease.insert (DgnLock (LockableId (model.GetModelId ()), level));
    toRelease.insert (DgnLock(LockableId(model.GetModeledElementId()), level));
    return model.GetDgnDb ().BriefcaseManager ().DemoteLocks (toRelease);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Eligijus.Mauragas             01/2016
//---------------------------------------------------------------------------------------
void SetLastPulledChangeSetId (Briefcase& briefcase, Utf8StringCR changeSetId)
    {
    briefcase.GetDgnDb ().SaveBriefcaseLocalValue ("ParentChangeSetId", changeSetId);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             07/2016
//---------------------------------------------------------------------------------------
DgnCode CreateElementCode(DgnDbR db, Utf8StringCR name, Utf8CP nameSpace = nullptr)
    {
    return nullptr != nameSpace ? PhysicalMaterial::CreateCode(db.GetDictionaryModel(), name) : SpatialCategory::CreateCode(db.GetDictionaryModel(), name);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             07/2016
//---------------------------------------------------------------------------------------
DgnCodeInfo CreateCodeAvailable(DgnCodeCR code)
    {
    return DgnCodeInfo(code);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             07/2016
//---------------------------------------------------------------------------------------
DgnCodeInfo CreateCodeDiscarded(DgnCodeCR code, Utf8StringCR changeSetId)
    {
    DgnCodeInfo info(code);
    info.SetDiscarded(changeSetId);
    return info;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             07/2016
//---------------------------------------------------------------------------------------
DgnCodeInfo CreateCodeReserved(DgnCodeCR code, DgnDbR db)
    {
    DgnCodeInfo info(code);
    info.SetReserved(db.GetBriefcaseId());
    return info;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             07/2016
//---------------------------------------------------------------------------------------
void ExpectNoCodesWithState(DgnCodeInfoSet const& expect, IRepositoryManagerP imodelManager)
    {
    DgnCodeInfoSet actual;
    DgnCodeSet codes;
    for (auto const& info : expect)
        codes.insert(info.GetCode());

    EXPECT_STATUS(Success, imodelManager->QueryCodeStates(actual, codes));
    EXPECT_EQ(0, actual.size());
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             07/2016
//---------------------------------------------------------------------------------------
void ExpectNoCodeWithState(DgnCodeInfoCR expect, IRepositoryManagerP imodelManager)
    {
    DgnCodeInfoSet expectInfos;
    expectInfos.insert(expect);
    ExpectNoCodesWithState(expectInfos, imodelManager);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             07/2016
//---------------------------------------------------------------------------------------
static DgnCode MakeStyleCode(Utf8CP name, DgnDbR db)
    {
    // Because AnnotationTextStyle::CreateCodeFromName() is private for some reason...
    AnnotationTextStyle style(db);
    style.SetName(name);
    return style.GetCode();
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas             06/2016
//---------------------------------------------------------------------------------------
void ExpectCodesCount(Briefcase& briefcase, int expectedCount)
    {
    auto result = briefcase.GetiModelConnection().QueryCodesLocks(briefcase.GetBriefcaseId())->GetResult();
    EXPECT_SUCCESS(result);
    auto actualCount = result.GetValue().GetCodes().size();
    EXPECT_EQ(expectedCount, actualCount);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   julius.cepukenas             08/2016
//---------------------------------------------------------------------------------------
void ExpectCodesCountById(Briefcase& briefcase, int expectedCount, bool byBriefcaseId, DgnCodeSet& codes)
    {
    LockableIdSet ids;
    auto result = QueryCodesLocksById(briefcase, byBriefcaseId, codes, ids)->GetResult();

    EXPECT_SUCCESS(result);
    auto actualCount = result.GetValue().GetCodes().size();
    EXPECT_EQ(expectedCount, actualCount);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas             06/2016
//---------------------------------------------------------------------------------------
void CheckCodeState(BriefcasePtr briefcase, DgnCodeSet codes, CodeState expectedCodeState)
    {
    DgnCodeInfoSet codesInfo;
    EXPECT_EQ(RepositoryStatus::Success, briefcase->GetDgnDb().BriefcaseManager().QueryCodeStates(codesInfo, codes));
    DgnCodeInfoSet::iterator it;
    for (it = codesInfo.begin(); it != codesInfo.end(); ++it)
        {
        auto codeInfo = *it;
        EXPECT_TRUE(codeInfo.GetCode().IsValid());
        if (Reserved == expectedCodeState)
            {
            EXPECT_TRUE(codeInfo.IsReserved());
            }
        if (Discarded == expectedCodeState)
            {
            EXPECT_TRUE(codeInfo.IsDiscarded());
            }
        if (Available == expectedCodeState)
            {
            EXPECT_TRUE(codeInfo.IsAvailable());
            }
        if (Used == expectedCodeState)
            {
            EXPECT_TRUE(codeInfo.IsUsed());
            }
        }
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas             09/2016
//---------------------------------------------------------------------------------------
void RecoverBriefcase(ClientPtr client, DgnDbR briefcaseDb)
    {
    auto briefcaseName = briefcaseDb.GetFileName();
    briefcaseDb.CloseDb();

    auto db2 = DgnDb::OpenDgnDb(nullptr, briefcaseName, DgnDb::OpenParams(DgnDb::OpenMode::ReadWrite));
    auto refreshResult = client->RecoverBriefcase(db2, false)->GetResult();
    EXPECT_SUCCESS(refreshResult);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas             09/2016
//---------------------------------------------------------------------------------------
BriefcasePtr OpenBriefcase(ClientPtr client, BeFileName briefcaseName)
    {
    auto db3 = DgnDb::OpenDgnDb(nullptr, briefcaseName, DgnDb::OpenParams(DgnDb::OpenMode::ReadWrite));
    auto openResult = client->OpenBriefcase(db3)->GetResult();
    EXPECT_SUCCESS(openResult);

    return openResult.GetValue();
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             09/2016
//---------------------------------------------------------------------------------------
BriefcasePtr ReloadBriefcase(DgnDbR briefcaseDb, ClientPtr client)
    {
    auto briefcaseName = briefcaseDb.GetFileName();
    briefcaseDb.CloseDb();
    auto db2 = DgnDb::OpenDgnDb(nullptr, briefcaseName, DgnDb::OpenParams(DgnDb::OpenMode::ReadWrite));
    return client->OpenBriefcase(db2)->GetResult().GetValue();
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis            08/2016
//---------------------------------------------------------------------------------------
TEST_F(iModelManagerTests, ReplaceSeedFileWithLocks)
    {
    // Create imodel and acquire a briefcase
    auto db = CreateTestDb("ReplaceSeedFileWithLocksTest");
    auto createResult = m_client->CreateNewiModel(*db)->GetResult();
    EXPECT_SUCCESS(createResult);
    auto imodelInfo = createResult.GetValue();
    auto imodelConnection = ConnectToiModel(*m_client, imodelInfo);
    auto briefcase = IntegrationTestsBase::AcquireBriefcase(*m_client, *imodelInfo, true);

    // Change the file guid
    auto fileName = db->GetFileName();
    BeSQLite::BeGuid newGuid;
    newGuid.Create();
    db->ChangeDbGuid(newGuid);
    EXPECT_EQ(BeSQLite::DbResult::BE_SQLITE_OK, db->SaveChanges());

    // Acquire a lock
    AcquireLock(briefcase->GetDgnDb(), LockLevel::Exclusive);

    // Replace seed file should fail
    FileInfoPtr fileInfo = FileInfo::Create(*db, "Replacement description1");
    EXPECT_SUCCESS(imodelConnection->LockiModel()->GetResult());
    auto uploadResult = imodelConnection->UploadNewSeedFile(fileName, *fileInfo)->GetResult();
    EXPECT_FALSE(uploadResult.IsSuccess());
    EXPECT_SUCCESS(imodelConnection->UnlockiModel()->GetResult());

    // Release the db lock
    briefcase->GetDgnDb().BriefcaseManager().RelinquishLocks();

    // Replace seed file should succeed now
    fileInfo = FileInfo::Create(*db, "Replacement description2");
    EXPECT_SUCCESS(imodelConnection->LockiModel()->GetResult());
    EXPECT_SUCCESS(imodelConnection->UploadNewSeedFile(fileName, *fileInfo)->GetResult());
    DeleteiModel(*m_client, *createResult.GetValue());
    }

void CreateFileInstance(iModelConnectionPtr connection)
    {
    Json::Value createFileJson = Json::objectValue;
    createFileJson["instance"] = Json::objectValue;
    createFileJson["instance"]["schemaName"] = "iModelScope";
    createFileJson["instance"]["className"] = "SeedFile";
    createFileJson["instance"]["properties"]["FileId"] = BeSQLite::BeGuid(true).ToString();
    createFileJson["instance"]["properties"]["MergedChangeSetId"] = "";
    auto wsiModelClient = BackDoor::iModelConnection::GetRepositoryClient(connection);
    EXPECT_SUCCESS(wsiModelClient->SendCreateObjectRequest(createFileJson)->GetResult());
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas             06/2017
//---------------------------------------------------------------------------------------
void ConvertToChangeSetPointersVector(ChangeSets changeSets, bvector<DgnRevisionCP>& pointersVector)
    {
    pointersVector.clear();
    for (auto changeSetPtr : changeSets)
        {
        pointersVector.push_back(changeSetPtr.get());
        }
    }
    
//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis            08/2016
//---------------------------------------------------------------------------------------
TEST_F(iModelManagerTests, DeleteFile)
    {
    auto imodelConnection = ConnectToiModel(*m_client, m_imodel);

    EXPECT_FALSE(imodelConnection->CancelSeedFileCreation()->GetResult().IsSuccess());
    auto filesResult = imodelConnection->GetSeedFiles()->GetResult();
    EXPECT_SUCCESS(filesResult);
    EXPECT_EQ(1, filesResult.GetValue().size());

    EXPECT_SUCCESS(imodelConnection->LockiModel()->GetResult());
    CreateFileInstance(imodelConnection);
    filesResult = imodelConnection->GetSeedFiles()->GetResult();
    EXPECT_SUCCESS(filesResult);
    EXPECT_EQ(2, filesResult.GetValue().size());
    EXPECT_SUCCESS(imodelConnection->CancelSeedFileCreation()->GetResult());
    EXPECT_SUCCESS(imodelConnection->UnlockiModel()->GetResult());

    auto db = CreateTestDb("DeleteFileTest");
    FileInfoPtr fileInfo = FileInfo::Create(*db, "");
    EXPECT_SUCCESS(imodelConnection->LockiModel()->GetResult());
    EXPECT_FALSE(imodelConnection->UploadNewSeedFile(db->GetFileName(), *fileInfo)->GetResult().IsSuccess());
    EXPECT_SUCCESS(imodelConnection->UnlockiModel()->GetResult());

    filesResult = imodelConnection->GetSeedFiles()->GetResult();
    EXPECT_SUCCESS(filesResult);
    EXPECT_EQ(1, filesResult.GetValue().size());
    EXPECT_FALSE(imodelConnection->CancelSeedFileCreation()->GetResult().IsSuccess());

    db->CloseDb();
    auto fileName = db->GetFileName().GetDirectoryName();
    fileName.AppendToPath(BeFileName(filesResult.GetValue()[0]->GetFileName()));
    fileName.BeDeleteFile();
    EXPECT_EQ(BeFileNameStatus::Success, BeFileName::BeMoveFile(db->GetFileName(), fileName));
    db = DgnDb::OpenDgnDb(nullptr, fileName, DgnDb::OpenParams(DgnDb::OpenMode::ReadWrite));
    fileInfo = FileInfo::Create(*db, "");
    EXPECT_SUCCESS(imodelConnection->LockiModel()->GetResult());
    EXPECT_SUCCESS(imodelConnection->UploadNewSeedFile(db->GetFileName(), *fileInfo)->GetResult());
    EXPECT_SUCCESS(imodelConnection->UnlockiModel()->GetResult());

    filesResult = imodelConnection->GetSeedFiles()->GetResult();
    EXPECT_SUCCESS(filesResult);
    EXPECT_EQ(2, filesResult.GetValue().size());
    EXPECT_FALSE(imodelConnection->CancelSeedFileCreation()->GetResult().IsSuccess());
    
    filesResult = imodelConnection->GetSeedFiles()->GetResult();
    EXPECT_SUCCESS(filesResult);
    EXPECT_EQ(2, filesResult.GetValue().size());
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Karolis.Dziedzelis            08/2016
//---------------------------------------------------------------------------------------
TEST_F(iModelManagerTests, RecoverBriefcase)
    {
    // Create imodel and acquire a briefcase
    auto db = CreateTestDb("RecoverBriefcaseTest");
    BeSQLite::BeGuid oldGuid = db->GetDbGuid();
    auto imodelInfo = CreateNewiModelFromDb(*m_client, *db);
    auto imodelConnection = ConnectToiModel(*m_client, imodelInfo);
    auto briefcase = IntegrationTestsBase::AcquireBriefcase(*m_client, *imodelInfo, true);

    // ReplaceSeedFile
    auto newGuid = ReplaceSeedFile(imodelConnection, *db);
    
    // Open briefcase should fail without refresh
    DgnDbR briefcaseDb = briefcase->GetDgnDb();
    auto briefcaseName = briefcaseDb.GetFileName();
    briefcaseDb.CloseDb();
    auto db2 = DgnDb::OpenDgnDb(nullptr, briefcaseName, DgnDb::OpenParams(DgnDb::OpenMode::ReadWrite));
    EXPECT_EQ(oldGuid, db2->GetDbGuid());
    auto openResult = m_client->OpenBriefcase(db2)->GetResult();
    EXPECT_FALSE(openResult.IsSuccess());

    // Refresh briefcase
    auto refreshResult = m_client->RecoverBriefcase(db2, false)->GetResult();
    EXPECT_SUCCESS(refreshResult);

    // Open briefcase should now succeed
    auto db3 = DgnDb::OpenDgnDb(nullptr, briefcaseName, DgnDb::OpenParams(DgnDb::OpenMode::ReadWrite));
    EXPECT_EQ(newGuid, db3->GetDbGuid());
    openResult = m_client->OpenBriefcase(db3)->GetResult();
    EXPECT_SUCCESS(openResult);
    DeleteiModel(*m_client, *imodelInfo);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas            09/2016
//---------------------------------------------------------------------------------------
TEST_F(iModelManagerTests, EmptyFileReplaceCodesLocksChangeSets)
    {
    // Create imodel and acquire a briefcase
    auto db = CreateTestDb("EmptyFileReplaceCodesLocksChangeSetsTest");
    auto imodelInfo = CreateNewiModelFromDb(*m_client, *db);
    auto imodelConnection = ConnectToiModel(*m_client, imodelInfo);
    auto briefcase = IntegrationTestsBase::AcquireBriefcase(*m_client, *imodelInfo, true);

    // Replace seed file
    ReplaceSeedFile(imodelConnection, *db);

    // Refresh briefcase
    DgnDbR briefcaseDb = briefcase->GetDgnDb();
    auto briefcaseName = briefcaseDb.GetFileName();
    RecoverBriefcase(m_client, briefcaseDb);
    
    // Open new briefcase
    auto newBriefcase = OpenBriefcase(m_client, briefcaseName);

    DgnDbR newDb = newBriefcase->GetDgnDb();
    auto newiModelManager = _GetRepositoryManager(newDb);

    // Create code
    DgnCode code = MakeStyleCode("Code1", newDb);
    DgnCode code2 = MakeStyleCode("Code2", newDb);
    ExpectCodesCount(*newBriefcase, 0);

    // Reserve code
    EXPECT_EQ(RepositoryStatus::Success, newDb.BriefcaseManager().ReserveCode(code));
    EXPECT_EQ(RepositoryStatus::Success, newDb.BriefcaseManager().ReserveCode(code2));
    ExpectCodesCount(*newBriefcase, 2);
    ExpectCodeState(CreateCodeReserved(code, newDb), newiModelManager);
    
    // Use code + lock
    auto model1 = CreateModel("Model1", newDb);
    InsertStyle("Code1", newDb);
    newDb.SaveChanges();
    auto cs = PushPendingChanges(*newBriefcase);

    ExpectLocksCount(*newBriefcase, 6);
    ExpectCodesCount(*newBriefcase, 1);
    ExpectCodeState(CreateCodeUsed(code, cs), newiModelManager);
    ExpectCodeState(CreateCodeReserved(code2, newDb), newiModelManager);

    // Discard one code and use another
    auto pStyle = AnnotationTextStyle::GetForEdit(newDb, "Code1");
    pStyle->SetName("Code2");
    EXPECT_TRUE(pStyle->Update().IsValid());
    pStyle = nullptr;

    // Push changes
    newDb.SaveChanges();
    cs = PushPendingChanges(*newBriefcase);

    ExpectCodeState(CreateCodeUsed(code2, cs), newiModelManager);
    ExpectNoCodeWithState(CreateCodeDiscarded(code, cs), newiModelManager);

    newDb.CloseDb();
    DeleteiModel(*m_client, *imodelInfo);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas            09/2016
//---------------------------------------------------------------------------------------
TEST_F(iModelManagerTests, CanceledFileReplaceCodesLocksChangeSets)
    {
    // Create imodel and acquire a briefcase
    auto dbOriginal = CreateTestDb("CanceledFileReplaceCodesLocksChangeSetsTest");
    auto imodelInfo = CreateNewiModelFromDb(*m_client, *dbOriginal);
    auto imodelConnection = ConnectToiModel(*m_client, imodelInfo);
    auto briefcase = IntegrationTestsBase::AcquireBriefcase(*m_client, *imodelInfo, true);

    DgnDbR db = briefcase->GetDgnDb();
    auto imodelManager = _GetRepositoryManager(db);

    // Start and cancel seed file replace
    EXPECT_SUCCESS(imodelConnection->LockiModel()->GetResult());
    CreateFileInstance(imodelConnection);
    auto filesResult = imodelConnection->GetSeedFiles()->GetResult();
    EXPECT_SUCCESS(filesResult);
    EXPECT_EQ(2, filesResult.GetValue().size());
    EXPECT_SUCCESS(imodelConnection->CancelSeedFileCreation()->GetResult());
    EXPECT_SUCCESS(imodelConnection->UnlockiModel()->GetResult());

    // Create code
    DgnCode code = MakeStyleCode("Code1", db);
    ExpectCodesCount(*briefcase, 0);

    // Reserve code
    EXPECT_EQ(RepositoryStatus::Success, db.BriefcaseManager().ReserveCode(code));;
    ExpectCodesCount(*briefcase, 1);
    ExpectCodeState(CreateCodeReserved(code, db), imodelManager);

    // Use code + lock
    auto model1 = CreateModel("Model1", db);
    InsertStyle("Code1", db);
    db.SaveChanges();
    auto cs = PushPendingChanges(*briefcase);

    ExpectLocksCount(*briefcase, 6);
    ExpectCodesCount(*briefcase, 0);
    ExpectCodeState(CreateCodeUsed(code, cs), imodelManager);
    DeleteiModel(*m_client, *imodelInfo);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas            09/2016
//---------------------------------------------------------------------------------------
TEST_F(iModelManagerTests, FileReplaceInProgressCodesLocksChangeSetsDenied)
    {
    // Create imodel and acquire a briefcase
    auto dbOriginal = CreateTestDb("FileReplaceInProgressCodesLocksChangeSetsDeniedTest");
    auto imodelInfo = CreateNewiModelFromDb(*m_client, *dbOriginal);
    auto imodelConnection = ConnectToiModel(*m_client, imodelInfo);
    auto briefcase = IntegrationTestsBase::AcquireBriefcase(*m_client, *imodelInfo, true);

    DgnDbR db = briefcase->GetDgnDb();
    auto imodelManager = _GetRepositoryManager(db);

    // Start seed file replace
    EXPECT_SUCCESS(imodelConnection->LockiModel()->GetResult());
    
    // Create code
    DgnCode code = MakeStyleCode("Code1", db);
    ExpectCodesCount(*briefcase, 0);

    // Reserve code
    auto reserveCodeResult = db.BriefcaseManager().ReserveCode(code);
    EXPECT_EQ(RepositoryStatus::RepositoryIsLocked, reserveCodeResult);
    ExpectCodesCount(*briefcase, 0);
    ExpectNoCodeWithState(CreateCodeReserved(code, db), imodelManager);

    // Check create model fails
    PhysicalPartitionPtr partition = PhysicalPartition::Create(*db.Elements().GetRootSubject(), "PartitionModel1");
    EXPECT_TRUE(partition.IsValid());
    auto stat = db.BriefcaseManager().AcquireForElementInsert(*partition);
    EXPECT_EQ(RepositoryStatus::RepositoryIsLocked, stat);

    PhysicalModelPtr model = PhysicalModel::Create(*partition);
    IBriefcaseManager::Request req;
    EXPECT_EQ(RepositoryStatus::RepositoryIsLocked, db.BriefcaseManager().PrepareForModelInsert(req, *model, IBriefcaseManager::PrepareAction::Acquire));

    db.SaveChanges();
    auto cs = PushPendingChanges(*briefcase);

    // Check nothing changed
    ExpectLocksCount(*briefcase, 0);
    ExpectCodesCount(*briefcase, 0);
    ExpectNoCodeWithState(CreateCodeUsed(partition->GetCode(), cs), imodelManager);
    DeleteiModel(*m_client, *imodelInfo);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                   Algirdas.Mikoliunas            09/2016
//---------------------------------------------------------------------------------------
TEST_F(iModelManagerTests, EmptyFileReplaceOtherUser)
    {
    // Create imodel and acquire a briefcase
    auto db = CreateTestDb("EmptyFileReplaceOtherUserTest");
    auto imodelInfo = CreateNewiModelFromDb(*m_client, *db);
    auto imodelConnection = ConnectToiModel(*m_client, imodelInfo);
    auto briefcase = IntegrationTestsBase::AcquireBriefcase(*m_client, *imodelInfo, true);

    // Create new user
    auto nonAdminClient = SetUpClient(IntegrationTestSettings::Instance().GetValidHost(), IntegrationTestSettings::Instance().GetValidNonAdminCredentials());
    auto briefcase2 = IntegrationTestsBase::AcquireBriefcase(*nonAdminClient, *imodelInfo, true);
    DgnDbR dbnDb2 = briefcase2->GetDgnDb();

    m_pHost->SetRepositoryAdmin(nonAdminClient->GetiModelAdmin());

    // Do something to initialize briefcaseManager
    EXPECT_EQ(RepositoryStatus::Success, dbnDb2.BriefcaseManager().ReserveCode(MakeStyleCode("CodeInit", dbnDb2)));
    EXPECT_EQ(DgnDbStatus::Success, InsertStyle("MyStyle", dbnDb2));
    dbnDb2.SaveChanges();

    auto result = briefcase2->GetiModelConnection().RelinquishCodesLocks(briefcase2->GetBriefcaseId())->GetResult();
    EXPECT_SUCCESS(result);
    
    m_pHost->SetRepositoryAdmin(m_client->GetiModelAdmin());

    // Replace seed file
    ReplaceSeedFile(imodelConnection, *db);

    // Refresh briefcase
    DgnDbR briefcaseDb = briefcase->GetDgnDb();
    auto briefcaseName = briefcaseDb.GetFileName();
    RecoverBriefcase(m_client, briefcaseDb);

    m_pHost->SetRepositoryAdmin(nonAdminClient->GetiModelAdmin());

    // Create & reserve code should succeed
    DgnCode code2 = MakeStyleCode("Code2", dbnDb2);
    auto stat = dbnDb2.BriefcaseManager().ReserveCode(code2);
    EXPECT_EQ(RepositoryStatus::Success, stat);

    // Element creation should fail
    PhysicalPartitionPtr partition = PhysicalPartition::Create(*dbnDb2.Elements().GetRootSubject(), "PartitionName");
    EXPECT_TRUE(partition.IsValid());
    auto sta = dbnDb2.BriefcaseManager().AcquireForElementInsert(*partition);
    EXPECT_EQ(RepositoryStatus::RevisionRequired, sta);

    // Locks should fail
    LockRequest lockReq;
    lockReq.Insert(briefcaseDb, LockLevel::Exclusive);
    stat = dbnDb2.BriefcaseManager().AcquireLocks(lockReq).Result();
    EXPECT_EQ(RepositoryStatus::RevisionRequired, stat);
    
    // ChangeSet push should fail
    auto pushResult = briefcase2->PullMergeAndPush(nullptr, false)->GetResult();
    auto errId = pushResult.GetError().GetId();
    EXPECT_EQ(Error::Id::ChangeSetPointsToBadSeed, errId);

    //Recover briefcase
    auto briefcase2Name = dbnDb2.GetFileName();
    partition = nullptr;
    RecoverBriefcase(nonAdminClient, dbnDb2);
    
    // Open new briefcase
    auto newBriefcase = OpenBriefcase(nonAdminClient, briefcase2Name);
    DgnDbR newDb = newBriefcase->GetDgnDb();
    auto newiModelManager = _GetRepositoryManager(newDb);

    // Create code
    DgnCode code = MakeStyleCode("Code1", newDb);
    ExpectCodesCount(*newBriefcase, 1);

    // Reserve code
    EXPECT_EQ(RepositoryStatus::Success, newDb.BriefcaseManager().ReserveCode(code));
    ExpectCodesCount(*newBriefcase, 2);
    ExpectCodeState(CreateCodeReserved(code, newDb), newiModelManager);
    
    // Create and push model
    auto model1 = CreateModel("Model1", newDb);
    newDb.SaveChanges();
    auto cs = PushPendingChanges(*newBriefcase);

    ExpectLocksCount(*newBriefcase, 4);
    ExpectCodesCount(*newBriefcase, 2);

    m_pHost->SetRepositoryAdmin(m_client->GetiModelAdmin());

    DeleteiModel(*m_client, *imodelInfo);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Eligijus.Mauragas             01/2016
//---------------------------------------------------------------------------------------
TEST_F (iModelManagerTests, QueryLocksTest)
    {
    //Prapare imodel and acquire briefcases
    auto briefcase1 = AcquireBriefcase();
    auto briefcase2 = AcquireBriefcase();
    
    ExpectLocksCount (*briefcase1, 0);
    ExpectLocksCount (*briefcase2, 0);

    //Create two models in different briefcases. This should also acquire locks automatically.
    auto model1 = CreateModel ("Model1", briefcase1->GetDgnDb ());
    briefcase1->GetDgnDb ().SaveChanges ();
    PushPendingChanges (*briefcase1);
    auto model2 = CreateModel ("Model2", briefcase2->GetDgnDb ());
    briefcase2->GetDgnDb ().SaveChanges ();
    PushPendingChanges (*briefcase2);

    ExpectLocksCount (*briefcase1, 4);
    ExpectLocksCount (*briefcase2, 4);

    //Check if we can access locks by Id
    ExpectLocksCountById (*briefcase1, 2, true,  LockableId (*model1), LockableId (model1->GetDgnDb ()));
    ExpectLocksCountById (*briefcase1, 2, true,  LockableId (*model1), LockableId (*model2), LockableId (model1->GetDgnDb ()));
    ExpectLocksCountById (*briefcase1, 3, false, LockableId (*model1), LockableId (model1->GetDgnDb ()));
    ExpectLocksCountById (*briefcase1, 4, false, LockableId (*model1), LockableId (*model2), LockableId (model1->GetDgnDb ()));
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Karolis.Dziedzelis             06/2016
//---------------------------------------------------------------------------------------
TEST_F(iModelManagerTests, QueryUnavailableLocksTest)
    {
    //Prapare imodel and acquire briefcases
    auto briefcase1 = AcquireBriefcase();
    auto briefcase2 = AcquireBriefcase();

    ExpectUnavailableLocksCount(*briefcase1, 0);
    ExpectUnavailableLocksCount(*briefcase2, 0);

    //Create a model and push it.
    auto model1 = CreateModel("Model1", briefcase1->GetDgnDb());
    briefcase1->GetDgnDb().SaveChanges();
    PushPendingChanges(*briefcase1);

    //Both model and file locks should be unavailable to the second briefcase.
    ExpectUnavailableLocksCount(*briefcase2, 4);

    //After releasing model lock, it should still be unavailable until merge.
    EXPECT_EQ(RepositoryStatus::Success, DemoteLock(*model1));
    ExpectUnavailableLocksCount(*briefcase2, 4);

        {
        //After merge, only file lock should be unavailable
        auto pullResult = briefcase2->PullAndMerge()->GetResult();
        EXPECT_SUCCESS(pullResult);
        }
    ExpectUnavailableLocksCount(*briefcase2, 2);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Karolis.Dziedzelis             07/2016
//---------------------------------------------------------------------------------------
TEST_F(iModelManagerTests, QueryAvailableLocksTest)
    {
    //Prapare imodel and acquire briefcases
    auto briefcase1 = AcquireBriefcase();
    auto briefcase2 = AcquireBriefcase();
    DgnDbR db1 = briefcase1->GetDgnDb();
    DgnDbR db2 = briefcase2->GetDgnDb();

    ExpectUnavailableLocksCount(*briefcase1, 0);
    ExpectUnavailableLocksCount(*briefcase2, 0);

    //Create a model and push it.
    auto model1 = CreateModel("Model1", briefcase1->GetDgnDb());
    briefcase1->GetDgnDb().SaveChanges();
    PushPendingChanges(*briefcase1);

    //Locks are available for the briefcase that owns them
    IBriefcaseManager::Request req(IBriefcaseManager::ResponseOptions::None);
    IBriefcaseManager::Response response(IBriefcaseManager::RequestPurpose::Query, req.Options());
    req.Locks().GetLockSet().insert(DgnLock(LockableId(briefcase1->GetDgnDb()), LockLevel::Shared));
    req.Locks().GetLockSet().insert(DgnLock(LockableId(model1->GetModelId()), LockLevel::Exclusive));
    EXPECT_TRUE(db1.BriefcaseManager().AreResourcesAvailable(req, &response, IBriefcaseManager::FastQuery::No));

    //Demote should be possible
    req.Locks().GetLockSet().clear();
    req.Locks().GetLockSet().insert(DgnLock(LockableId(model1->GetModelId()), LockLevel::Shared));
    EXPECT_TRUE(db1.BriefcaseManager().AreResourcesAvailable(req, &response, IBriefcaseManager::FastQuery::No));

    //Locks are unavailable without pull
    req.Locks().GetLockSet().clear();
    req.Locks().GetLockSet().insert(DgnLock(LockableId(model1->GetModelId()), LockLevel::Shared));
    EXPECT_FALSE(db2.BriefcaseManager().AreResourcesAvailable(req, &response, IBriefcaseManager::FastQuery::No));

    //Shared locks are available for shared acquire
    EXPECT_SUCCESS(briefcase2->PullAndMerge()->GetResult());
    req.Locks().GetLockSet().clear();
    req.Locks().GetLockSet().insert(DgnLock(LockableId(briefcase1->GetDgnDb()), LockLevel::Shared));
    EXPECT_TRUE(db2.BriefcaseManager().AreResourcesAvailable(req, &response, IBriefcaseManager::FastQuery::No));

    //Shared locks are unavailable for exclusive acquire
    req.Locks().GetLockSet().clear();
    req.Locks().GetLockSet().insert(DgnLock(LockableId(briefcase1->GetDgnDb()), LockLevel::Exclusive));
    EXPECT_FALSE(db2.BriefcaseManager().AreResourcesAvailable(req, &response, IBriefcaseManager::FastQuery::No));

    //Exclusive locks are unavailable for both exclusvie and shared acquire
    req.Locks().GetLockSet().clear();
    req.Locks().GetLockSet().insert(DgnLock(LockableId(model1->GetModelId()), LockLevel::Exclusive));
    EXPECT_FALSE(db2.BriefcaseManager().AreResourcesAvailable(req, &response, IBriefcaseManager::FastQuery::No));

    req.Locks().GetLockSet().clear();
    req.Locks().GetLockSet().insert(DgnLock(LockableId(model1->GetModelId()), LockLevel::Shared));
    EXPECT_FALSE(db2.BriefcaseManager().AreResourcesAvailable(req, &response, IBriefcaseManager::FastQuery::No));

    //Promotion should be possible
    DemoteLock(*model1, LockLevel::Shared);
    req.Locks().GetLockSet().clear();
    req.Locks().GetLockSet().insert(DgnLock(LockableId(model1->GetModelId()), LockLevel::Exclusive));
    EXPECT_TRUE(db1.BriefcaseManager().AreResourcesAvailable(req, &response, IBriefcaseManager::FastQuery::No));


    //Released lock should be available for both exclusive and shared acquire
    DemoteLock(*model1);
    req.Locks().GetLockSet().clear();
    req.Locks().GetLockSet().insert(DgnLock(LockableId(model1->GetModelId()), LockLevel::Exclusive));
    EXPECT_TRUE(db2.BriefcaseManager().AreResourcesAvailable(req, &response, IBriefcaseManager::FastQuery::No));

    req.Locks().GetLockSet().clear();
    req.Locks().GetLockSet().insert(DgnLock(LockableId(model1->GetModelId()), LockLevel::Shared));
    EXPECT_TRUE(db2.BriefcaseManager().AreResourcesAvailable(req, &response, IBriefcaseManager::FastQuery::No));
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Viktorija.Adomauskaite        03/2017
//---------------------------------------------------------------------------------------
TEST_F(iModelManagerTests, LocksStatesResponseTest)
    {
    //Prapare imodel and acquire briefcases
    auto briefcase1 = AcquireBriefcase();
    auto briefcase2 = AcquireBriefcase();
    DgnDbR db1 = briefcase1->GetDgnDb();
    DgnDbR db2 = briefcase2->GetDgnDb();

    ExpectUnavailableLocksCount(*briefcase1, 0);
    ExpectUnavailableLocksCount(*briefcase2, 0);

    //Create a models and push them.
    auto model1 = CreateModel("Model1", briefcase1->GetDgnDb());
    briefcase1->GetDgnDb().SaveChanges();
    PushPendingChanges(*briefcase1);

    auto model2 = CreateModel("Model2", briefcase1->GetDgnDb());
    briefcase1->GetDgnDb().SaveChanges();
    PushPendingChanges(*briefcase1);

    LockRequest req;
    req.Insert(*model1, LockLevel::Exclusive);
    req.Insert(*model2, LockLevel::Shared);
    auto response = db1.BriefcaseManager().AcquireLocks(req, IBriefcaseManager::ResponseOptions::LockState);
    EXPECT_EQ(RepositoryStatus::Success, response.Result());

    //chek if LockAlreadyHeld error returns locks correctly
    req.Clear();
    req.Insert(*model1, LockLevel::Exclusive);
    response = db2.BriefcaseManager().AcquireLocks(req, IBriefcaseManager::ResponseOptions::LockState);
    EXPECT_EQ(RepositoryStatus::LockAlreadyHeld, response.Result());
    EXPECT_EQ(1, response.LockStates().size());
    auto lock = *response.LockStates().begin();
    EXPECT_EQ(LockLevel::Exclusive, lock.GetOwnership().GetLockLevel());
    EXPECT_EQ(LockableId(*model1) ,lock.GetLockableId());

    //check if all conflicting locks are returned
    req.Clear();
    req.Insert(*model1, LockLevel::Exclusive);
    req.Insert(*model2, LockLevel::Exclusive);
    response = db2.BriefcaseManager().AcquireLocks(req, IBriefcaseManager::ResponseOptions::LockState);
    EXPECT_EQ(RepositoryStatus::LockAlreadyHeld, response.Result());
    EXPECT_EQ(2, response.LockStates().size());

    //make sure that no locks are returned then ResponseOptions is None
    response = db2.BriefcaseManager().AcquireLocks(req, IBriefcaseManager::ResponseOptions::None);
    EXPECT_EQ(RepositoryStatus::LockAlreadyHeld, response.Result());
    EXPECT_EQ(0, response.LockStates().size());

    EXPECT_EQ(RepositoryStatus::Success, briefcase1->GetDgnDb().BriefcaseManager().RelinquishLocks());

    //check if ChangeSetRequired error returns locks correctly
    req.Clear();
    req.Insert(*model1, LockLevel::Exclusive);
    response = db2.BriefcaseManager().AcquireLocks(req, IBriefcaseManager::ResponseOptions::LockState);
    EXPECT_EQ(RepositoryStatus::RevisionRequired, response.Result());
    EXPECT_EQ(1, response.LockStates().size());
    lock = *response.LockStates().begin();
    EXPECT_EQ(LockableId(*model1), lock.GetLockableId());

    //make sure that no locks are returned then ResponseOptions is None
    response = db2.BriefcaseManager().AcquireLocks(req, IBriefcaseManager::ResponseOptions::None);
    EXPECT_EQ(RepositoryStatus::RevisionRequired, response.Result());
    EXPECT_EQ(0, response.LockStates().size());
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Eligijus.Mauragas             01/2016
//---------------------------------------------------------------------------------------
TEST_F (iModelManagerTests, ReleasedWithChangeSetLocksTest)
    {
    //Prapare imodel and acquire a briefcase
    auto briefcase = AcquireBriefcase();

    //Create a model, this will automatically acquire neccessary locks
    auto model = CreateModel ("AddChangeSetsModel", briefcase->GetDgnDb ());
    briefcase->GetDgnDb ().SaveChanges ();
    ExpectLocksCount (*briefcase, 2); //Db (shared)

    //Push pending changes. This will mark locks with pushed changeSet id.
    //It also should acquire explicit lock on the model automatically.
    Utf8String lastChangeSetId = PushPendingChanges (*briefcase);
    ExpectLocksCount (*briefcase, 4);

    //Release model lock
    EXPECT_EQ (RepositoryStatus::Success, DemoteLock (*model));
    ExpectLocksCount (*briefcase, 2);

    //Release all locks
    EXPECT_EQ (RepositoryStatus::Success, briefcase->GetDgnDb ().BriefcaseManager ().RelinquishLocks ());
    ExpectLocksCount (*briefcase, 0);

    //We should not be able to acquire locks if we haven't pulled a changeSet.
    SetLastPulledChangeSetId (*briefcase, "");
    EXPECT_EQ (RepositoryStatus::RevisionRequired, AcquireLock (*model, LockLevel::Exclusive));
    EXPECT_EQ (RepositoryStatus::RevisionRequired, AcquireLock (*model, LockLevel::Shared));
    ExpectLocksCount (*briefcase, 0);

    //We should be able to acquire db lock because it shared locks should not be marked with ChangeSetId during push.
    EXPECT_EQ (RepositoryStatus::Success, AcquireLock (briefcase->GetDgnDb ()));
    ExpectLocksCount (*briefcase, 1);
    EXPECT_EQ (RepositoryStatus::Success, briefcase->GetDgnDb ().BriefcaseManager ().RelinquishLocks ());
    ExpectLocksCount (*briefcase, 0);

    //We should not be able to acquire locks if changeSet id is invalid.
    SetLastPulledChangeSetId (*briefcase, "BadChangeSetId");
    EXPECT_EQ (RepositoryStatus::InvalidRequest, AcquireLock (*model));
    ExpectLocksCount (*briefcase, 0);

    //We should be able to acquire locks if changeSet id is set correctly.
    SetLastPulledChangeSetId (*briefcase, lastChangeSetId);
    EXPECT_EQ (RepositoryStatus::Success, AcquireLock (*model));
    ExpectLocksCount (*briefcase, 2);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Eligijus.Mauragas             01/2016
//---------------------------------------------------------------------------------------
TEST_F (iModelManagerTests, ShouldNotBeAbleDeletingModelLockedByAnotherBriefcase)
    {
    //Prapare imodel and acquire briefcases
    auto briefcase1 = AcquireBriefcase();
    auto briefcase2 = AcquireBriefcase();

    ExpectLocksCount (*briefcase1, 0);
    ExpectLocksCount (*briefcase2, 0);

    //Create two models in different briefcases. This should also acquire locks automatically.
    auto model1 = CreateModel ("Model1", briefcase1->GetDgnDb ());
    briefcase1->GetDgnDb ().SaveChanges ();
    PushPendingChanges (*briefcase1);
    auto model2 = CreateModel ("Model2", briefcase2->GetDgnDb ());
    briefcase2->GetDgnDb ().SaveChanges ();
    PushPendingChanges (*briefcase2);

    ExpectLocksCount (*briefcase1, 4);
    ExpectLocksCount (*briefcase2, 4);

    //Try delete Model1 from briefcase2
    DgnModelPtr model1_2 = briefcase2->GetDgnDb ().Models ().GetModel (model1->GetModelId ());
    EXPECT_NE (DgnDbStatus::Success, model1_2->Delete ());
    }


//---------------------------------------------------------------------------------------
//@bsimethod                                    Algirdas.Mikoliunas             06/2016
//---------------------------------------------------------------------------------------
TEST_F(iModelManagerTests, ReserveCodeMultipleBriefcases)
    {
    //Prapare imodel and acquire briefcases
    auto briefcase1 = AcquireBriefcase();
    auto briefcase2 = AcquireBriefcase();
    auto imodelManager1 = _GetRepositoryManager(briefcase1->GetDgnDb());
    auto imodelManager2 = _GetRepositoryManager(briefcase2->GetDgnDb());

    ExpectCodesCount(*briefcase1, 0);
    ExpectCodesCount(*briefcase2, 0);

    DgnCode code = CreateElementCode(briefcase1->GetDgnDb(), "DOOR_1", "");
    DgnCodeSet codesSet;
    codesSet.insert(code);

    // Firstly code is not reserved
    RepositoryStatus status;
    EXPECT_FALSE(briefcase1->GetDgnDb().BriefcaseManager().AreCodesReserved(codesSet, &status));
    EXPECT_EQ(RepositoryStatus::CodeNotReserved, status);
    EXPECT_FALSE(briefcase2->GetDgnDb().BriefcaseManager().AreCodesReserved(codesSet, &status));
    EXPECT_EQ(RepositoryStatus::CodeNotReserved, status);

    // Reserve code
    EXPECT_EQ(RepositoryStatus::Success, briefcase1->GetDgnDb().BriefcaseManager().ReserveCode(code));
    EXPECT_EQ(RepositoryStatus::CodeUnavailable, briefcase2->GetDgnDb().BriefcaseManager().ReserveCode(code));

    // Checks how api behaves when trying to reserve same code multiple times
    EXPECT_EQ(RepositoryStatus::Success, briefcase1->GetDgnDb().BriefcaseManager().ReserveCode(code));

    ExpectCodesCount(*briefcase1, 1);
    ExpectCodesCount(*briefcase2, 0);
    ExpectCodeState(CreateCodeReserved(code, briefcase1->GetDgnDb()), imodelManager1);

    // Check if code is reserved by first briefcase
    EXPECT_TRUE(briefcase1->GetDgnDb().BriefcaseManager().AreCodesReserved(codesSet, &status));
    EXPECT_EQ(RepositoryStatus::Success, status);

    codesSet.insert(code);
    EXPECT_FALSE(briefcase2->GetDgnDb().BriefcaseManager().AreCodesReserved(codesSet, &status));
    EXPECT_EQ(RepositoryStatus::CodeNotReserved, status);

    // Check if code is not released using wrong briefcase
    EXPECT_EQ(RepositoryStatus::CodeUnavailable, briefcase2->GetDgnDb().BriefcaseManager().ReleaseCodes(codesSet));
    EXPECT_TRUE(briefcase1->GetDgnDb().BriefcaseManager().AreCodesReserved(codesSet, &status));
    EXPECT_EQ(RepositoryStatus::Success, status);

    // Release code
    codesSet.insert(code);
    EXPECT_EQ(RepositoryStatus::Success, briefcase1->GetDgnDb().BriefcaseManager().ReleaseCodes(codesSet));
    Utf8String lastChangeSetId = PushPendingChanges(*briefcase1);
    EXPECT_FALSE(briefcase1->GetDgnDb().BriefcaseManager().AreCodesReserved(codesSet, &status));
    EXPECT_EQ(RepositoryStatus::CodeNotReserved, status);

    ExpectNoCodeWithState(CreateCodeAvailable(code), imodelManager1);

    // Check if now other briefcase is able to reserve code
    EXPECT_EQ(RepositoryStatus::Success, briefcase2->GetDgnDb().BriefcaseManager().ReserveCode(code));
    ExpectCodeState(CreateCodeReserved(code, briefcase2->GetDgnDb()), imodelManager2);

    // Relinquish
    EXPECT_EQ(RepositoryStatus::Success, briefcase2->GetDgnDb().BriefcaseManager().RelinquishCodes());
    lastChangeSetId = PushPendingChanges(*briefcase2);
    ExpectNoCodeWithState(CreateCodeDiscarded(code, lastChangeSetId), imodelManager2);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Algirdas.Mikoliunas             06/2016
//---------------------------------------------------------------------------------------
TEST_F(iModelManagerTests, ReserveMultipleCodes)
    {
    //Prapare imodel and acquire briefcases
    BriefcasePtr briefcase = AcquireBriefcase();
    DgnDbR db = briefcase->GetDgnDb();
    auto imodelManager = _GetRepositoryManager(db);

    ExpectCodesCount(*briefcase, 0);

    DgnCode code = CreateElementCode(db, "DOOR-1", "HOUSE");
    DgnCode code2 = CreateElementCode(db, "DOOR-2", "HOUSE");
    DgnCodeSet codesSet;
    codesSet.insert(code);
    codesSet.insert(code2);

    // Firstly codes are not reserved
    RepositoryStatus status;
    EXPECT_FALSE(db.BriefcaseManager().AreCodesReserved(codesSet, &status));
    EXPECT_EQ(RepositoryStatus::CodeNotReserved, status);

    // Reserve codes
    EXPECT_EQ(RepositoryStatus::Success, db.BriefcaseManager().ReserveCodes(codesSet).Result());
    ExpectCodesCount(*briefcase, 2);
    ExpectCodeState(CreateCodeReserved(code, db), imodelManager);
    ExpectCodeState(CreateCodeReserved(code2, db), imodelManager);

    // Release one code
    DgnCodeSet codes;
    codes.insert(code);
    EXPECT_STATUS(Success, db.BriefcaseManager().ReleaseCodes(codes));
    ExpectNoCodeWithState(CreateCodeAvailable(code), imodelManager);
    ExpectCodeState(CreateCodeReserved(code2, db), imodelManager);

    // Relinquish
    EXPECT_EQ(RepositoryStatus::Success, db.BriefcaseManager().RelinquishCodes());
    ExpectNoCodeWithState(CreateCodeAvailable(code), imodelManager);
    ExpectNoCodeWithState(CreateCodeAvailable(code2), imodelManager);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Algirdas.Mikoliunas             06/2016
//---------------------------------------------------------------------------------------
TEST_F(iModelManagerTests, ReserveStyleCodes)
    {
    //Prapare imodel and acquire briefcases
    auto briefcase = AcquireBriefcase();
    DgnDbR db = briefcase->GetDgnDb();
    auto imodelManager = _GetRepositoryManager(db);

    // When we insert an element without having explicitly reserved its code, an attempt to reserve it will automatically occur
    EXPECT_EQ(DgnDbStatus::Success, InsertStyle("MyStyle", db));
    ExpectCodesCount(*briefcase, 1);
    ExpectCodeState(CreateCodeReserved(MakeStyleCode("MyStyle", db), db), imodelManager);

    // An attempt to insert an element with the same code as an already-used code will fail
    EXPECT_EQ(DgnDbStatus::DuplicateCode, InsertStyle("MyStyle", db, false));

    // Updating an element and changing its code will NOT reserve the new code if we haven't done so already
    auto pStyle = AnnotationTextStyle::Get(db, "MyStyle")->CreateCopy();
    DgnDbStatus status;
    pStyle->SetName("MyRenamedStyle");
    EXPECT_FALSE(pStyle->DgnElement::Update(&status).IsValid());
    EXPECT_EQ(DgnDbStatus::CodeNotReserved, status);

    // Explicitly reserve the code
    EXPECT_EQ(RepositoryStatus::Success, db.BriefcaseManager().ReserveCode(pStyle->GetCode()));
    EXPECT_TRUE(pStyle->Update().IsValid());
    ExpectCodesCount(*briefcase, 2);
    ExpectCodeState(CreateCodeReserved(pStyle->GetCode(), db), imodelManager);
    pStyle = nullptr;

    db.SaveChanges();
    auto cs1 = PushPendingChanges(*briefcase);
    ExpectCodeState(CreateCodeUsed(MakeStyleCode("MyRenamedStyle", db), cs1), imodelManager);

    pStyle = nullptr;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Algirdas.Mikoliunas             06/2016
//---------------------------------------------------------------------------------------
TEST_F(iModelManagerTests, ReserveModelCode)
    {
    //Prapare imodel and acquire briefcases
    auto briefcase = AcquireBriefcase();
    DgnDbR db = briefcase->GetDgnDb();
    auto imodelManager = _GetRepositoryManager(db);

    auto modeledElement = CreateModeledElement ("MyModel", db);
    auto persistentModeledElement = modeledElement->Insert();
    EXPECT_TRUE(persistentModeledElement.IsValid());
    DgnCode modeledElemCode = modeledElement->GetCode();
    ExpectCodesCount(*briefcase, 1);
    ExpectCodeState(CreateCodeReserved(modeledElemCode, db), imodelManager);
    
    db.SaveChanges();
    Utf8StringCR cs1 = PushPendingChanges(*briefcase);
    ExpectCodesCount(*briefcase, 0);
    ExpectCodeState(CreateCodeUsed(modeledElemCode, cs1), imodelManager);

    // Same code reservation for the second time should pass
    auto reserveResult = db.BriefcaseManager().ReserveCode(modeledElemCode);
    EXPECT_EQ(RepositoryStatus::Success, reserveResult);

    // Changing model code to not reserved one should fail
    EXPECT_EQ(DgnDbStatus::Success, modeledElement->SetCode(MakeModelCode("MyNewModel", db)));
    EXPECT_FALSE(modeledElement->Update().IsValid());

    // Creating new model with the same code should fail
    auto partition = PhysicalPartition::Create(*db.Elements().GetRootSubject(), "MyModel");
    EXPECT_TRUE(partition.IsValid());
    EXPECT_EQ(RepositoryStatus::CodeUsed, db.BriefcaseManager().AcquireForElementInsert(*partition));
    EXPECT_FALSE(partition->Insert().IsValid());

    EXPECT_EQ(DgnDbStatus::Success, persistentModeledElement->Delete());
    briefcase->GetDgnDb().SaveChanges();
    PushPendingChanges(*briefcase);
    ExpectNoCodeWithState(CreateCodeDiscarded(MakeModelCode("MyModel", db), cs1), imodelManager);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Algirdas.Mikoliunas             06/2016
//---------------------------------------------------------------------------------------
TEST_F(iModelManagerTests, CodesWithChangeSets)
    {
    //Prapare imodel and acquire briefcases
    auto briefcase = AcquireBriefcase();
    DgnDbR db = briefcase->GetDgnDb();
    auto imodelManager = _GetRepositoryManager(db);

    DgnCode unusedCode = MakeStyleCode("Unused", db);
    DgnCode usedCode = MakeStyleCode("Used", db);
    DgnCodeSet req;
    req.insert(unusedCode);
    req.insert(usedCode);
    EXPECT_STATUS(Success, db.BriefcaseManager().ReserveCodes(req).Result());

    // Use one of the codes
    EXPECT_EQ(DgnDbStatus::Success, InsertStyle("Used", db));
    ExpectCodeState(CreateCodeReserved(unusedCode, db), imodelManager);
    ExpectCodeState(CreateCodeReserved(usedCode, db), imodelManager);

    // Commit the change as a changeSet
    briefcase->GetDgnDb ().SaveChanges ();
    Utf8String cs1 = PushPendingChanges(*briefcase);
    EXPECT_FALSE(cs1.empty());

    // The used code should not be marked as such
    ExpectCodeState(CreateCodeUsed(usedCode, cs1), imodelManager);
    ExpectCodeState(CreateCodeReserved(unusedCode, db), imodelManager);

    // Swap the code so that "Used" becomes "Unused"
    auto pStyle = AnnotationTextStyle::GetForEdit(db, "Used");
    pStyle->SetName("Unused");
    EXPECT_TRUE(pStyle->Update().IsValid());
    pStyle = nullptr;

    // Commit the changeSet
    briefcase->GetDgnDb().SaveChanges();
    Utf8String cs2 = PushPendingChanges(*briefcase);
    EXPECT_FALSE(cs2.empty());

    // "Used" is now discarded; "Unused" is now used; both in the same changeSet
    ExpectCodeState(CreateCodeUsed(unusedCode, cs2), imodelManager);
    ExpectNoCodeWithState(CreateCodeDiscarded(usedCode, cs2), imodelManager);

    // Delete the style => its code becomes discarded
    // Ugh except you are not allowed to delete text styles...rename it again instead
    pStyle = AnnotationTextStyle::GetForEdit(db, "Unused");
    pStyle->SetName("Deleted");

    // Will fail because we haven't reserved code...
    DgnDbStatus status;
    EXPECT_FALSE(pStyle->DgnElement::Update(&status).IsValid());
    EXPECT_EQ(DgnDbStatus::CodeNotReserved, status);
    EXPECT_EQ(RepositoryStatus::Success, db.BriefcaseManager().ReserveCode(MakeStyleCode("Deleted", db)));
    EXPECT_TRUE(pStyle->DgnElement::Update(&status).IsValid());
    EXPECT_EQ(DgnDbStatus::Success, status);
    pStyle = nullptr;

    // Cannot release codes if transactions are pending
    ExpectCodesCount(*briefcase, 1);
    DgnCodeSet codes;
    codes.insert(MakeStyleCode("Deleted", db));
    EXPECT_STATUS(PendingTransactions, db.BriefcaseManager().ReleaseCodes(codes));
    EXPECT_STATUS(PendingTransactions, db.BriefcaseManager().RelinquishCodes());

    // Cannot release a code which is used locally
    db.SaveChanges();
    ExpectCodesCount(*briefcase, 1);
    EXPECT_STATUS(CodeUsed, db.BriefcaseManager().ReleaseCodes(codes));
    EXPECT_STATUS(CodeUsed, db.BriefcaseManager().RelinquishCodes());

    Utf8String cs3 = PushPendingChanges(*briefcase);
    EXPECT_FALSE(cs3.empty());
    ExpectNoCodeWithState(CreateCodeDiscarded(unusedCode, cs3), imodelManager);
    ExpectNoCodeWithState(CreateCodeDiscarded(usedCode, cs2), imodelManager);

    // We can reserve either code, since they are discarded and we have the latest changeSet
    EXPECT_STATUS(Success, db.BriefcaseManager().ReserveCode(usedCode));
    EXPECT_STATUS(Success, db.BriefcaseManager().ReserveCode(unusedCode));
    ExpectCodeState(CreateCodeReserved(usedCode, db), imodelManager);
    ExpectCodeState(CreateCodeReserved(unusedCode, db), imodelManager);

    // If we release these codes, they should return to "Discarded" and retain the most recent changeSet ID in which they were discarded.
    EXPECT_STATUS(Success, db.BriefcaseManager().RelinquishCodes());
    ExpectNoCodeWithState(CreateCodeDiscarded(unusedCode, cs3), imodelManager);
    ExpectNoCodeWithState(CreateCodeDiscarded(usedCode, cs2), imodelManager);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Algirdas.Mikoliunas             09/2016
//---------------------------------------------------------------------------------------
TEST_F(iModelManagerTests, CodesWithSpecialSymbols)
    {
    //Prapare imodel and acquire briefcases
    auto briefcase = AcquireBriefcase();
    DgnDbR db = briefcase->GetDgnDb();
    auto imodelManager = _GetRepositoryManager(db);

    DgnCode code1 = MakeStyleCode("1*1", db);
    DgnCode code2 = MakeStyleCode("\t*\n", db);
    DgnCodeSet req;
    req.insert(code1);
    req.insert(code2);
    EXPECT_STATUS(Success, db.BriefcaseManager().ReserveCodes(req).Result());

    // Use one of the codes
    EXPECT_EQ(DgnDbStatus::Success, InsertStyle("1*1", db));
    ExpectCodeState(CreateCodeReserved(code1, db), imodelManager);
    ExpectCodeState(CreateCodeReserved(code2, db), imodelManager);

    // Commit the change as a changeSet
    briefcase->GetDgnDb().SaveChanges();
    Utf8String cs1 = PushPendingChanges(*briefcase);
    EXPECT_FALSE(cs1.empty());

    ExpectCodeState(CreateCodeUsed(code1, cs1), imodelManager);
    ExpectCodeState(CreateCodeReserved(code2, db), imodelManager);
    
    // Discard first code
    auto pStyle = AnnotationTextStyle::GetForEdit(db, "1*1");
    pStyle->SetName("\t*\n");
    EXPECT_TRUE(pStyle->Update().IsValid());
    pStyle = nullptr;

    // Commit the changeSet
    briefcase->GetDgnDb().SaveChanges();
    Utf8String cs2 = PushPendingChanges(*briefcase);
    EXPECT_FALSE(cs2.empty());

    ExpectCodeState(CreateCodeUsed(code2, cs2), imodelManager);
    ExpectNoCodeWithState(CreateCodeDiscarded(code1, cs2), imodelManager);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Karolis.Dziedzelis             06/2016
//---------------------------------------------------------------------------------------
TEST_F(iModelManagerTests, QueryUnavailableCodesTest)
    {
    //Prapare imodel and acquire briefcases
    auto briefcase1 = AcquireBriefcase();
    auto briefcase2 = AcquireBriefcase();
    DgnDbR db1 = briefcase1->GetDgnDb();
    DgnDbR db2 = briefcase2->GetDgnDb();
    auto imodelManager = _GetRepositoryManager(db2);

    int codesCountInSeedFile = GetCodesCount(db1);
    ExpectUnavailableCodesCount(*briefcase1, codesCountInSeedFile);
    ExpectUnavailableCodesCount(*briefcase2, codesCountInSeedFile);

    //Reserve the code
    DgnCode used = MakeStyleCode("Used", db1);
    DgnCode unused = MakeStyleCode("Unused", db1);
    DgnCodeSet req;
    req.insert(used);
    req.insert(unused);
    EXPECT_STATUS(Success, db1.BriefcaseManager().ReserveCodes(req).Result());

    //Reserved code should be unavailable
    ExpectCodeState(CreateCodeReserved(used, db1), imodelManager);
    ExpectCodeState(CreateCodeReserved(unused, db1), imodelManager);
    ExpectUnavailableCodesCount(*briefcase2, codesCountInSeedFile + 2);

    //Use the code
    EXPECT_EQ(DgnDbStatus::Success, InsertStyle("Used", db1));
    db1.SaveChanges();
    Utf8String cs1 = PushPendingChanges(*briefcase1);
    EXPECT_FALSE(cs1.empty());

    //Used code should be unavailable
    ExpectCodeState(CreateCodeUsed(used, cs1), imodelManager);
    ExpectCodeState(CreateCodeReserved(unused, db1), imodelManager);
    ExpectUnavailableCodesCount(*briefcase2, codesCountInSeedFile + 2);

    //Swap the name, to discard the first code
    auto pStyle = AnnotationTextStyle::GetForEdit(db1, "Used");
    pStyle->SetName("Unused");
    EXPECT_TRUE(pStyle->Update().IsValid());
    pStyle = nullptr;

    db1.SaveChanges();
    Utf8String cs2 = PushPendingChanges(*briefcase1);
    EXPECT_FALSE(cs2.empty());

    //Discarded code should be available
    ExpectNoCodeWithState(CreateCodeDiscarded(used, cs2), imodelManager);
    ExpectCodeState(CreateCodeUsed(unused, cs2), imodelManager);
    ExpectUnavailableCodesCount(*briefcase2, codesCountInSeedFile + 1);
        {
        //Merge the second briefcase
        auto pullResult = briefcase2->PullAndMerge()->GetResult();
        EXPECT_SUCCESS(pullResult);
        }

    //Discared code should be available
    ExpectNoCodeWithState(CreateCodeDiscarded(used, cs2), imodelManager);
    ExpectCodeState(CreateCodeUsed(unused, cs2), imodelManager);
    ExpectUnavailableCodesCount(*briefcase2, codesCountInSeedFile + 1);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Karolis.Dziedzelis             07/2016
//---------------------------------------------------------------------------------------
TEST_F(iModelManagerTests, QueryAvailableCodesTest)
    {
    //Prapare imodel and acquire briefcases
    auto briefcase1 = AcquireBriefcase();
    auto briefcase2 = AcquireBriefcase();
    DgnDbR db1 = briefcase1->GetDgnDb();
    DgnDbR db2 = briefcase2->GetDgnDb();

    DgnCode code1 = MakeStyleCode("Code1", db1);
    DgnCode code2 = MakeStyleCode("Code2", db1);

    //Reserve codes
    DgnCodeSet codes;
    codes.insert(code1);
    codes.insert(code2);
    EXPECT_STATUS(Success, db1.BriefcaseManager().ReserveCodes(codes).Result());

    //Reserved codes are unavailable
    IBriefcaseManager::Request req(IBriefcaseManager::ResponseOptions::None);
    req.Codes() = codes;
    IBriefcaseManager::Response response(IBriefcaseManager::RequestPurpose::Query, req.Options());
    EXPECT_FALSE(db2.BriefcaseManager().AreResourcesAvailable(req, &response, IBriefcaseManager::FastQuery::No));
    
    //Use a code
    InsertStyle("Code1", db1, true);
    db1.SaveChanges();
    PushPendingChanges(*briefcase1);

    //Used code is available
    EXPECT_SUCCESS(briefcase2->PullAndMerge()->GetResult());
    req.Reset();
    req.SetOptions(IBriefcaseManager::ResponseOptions::CodeState);
    req.Codes().insert(code1);  // reserved
    EXPECT_TRUE(db2.BriefcaseManager().AreResourcesAvailable(req, &response, IBriefcaseManager::FastQuery::No));

    //A set of available codes should be available
    DgnCode code3 = MakeStyleCode("Code3", db1);
    req.Reset();
    req.SetOptions(IBriefcaseManager::ResponseOptions::CodeState);
    req.Codes().insert(code3);  // new code
    EXPECT_TRUE(db2.BriefcaseManager().AreResourcesAvailable(req, &response, IBriefcaseManager::FastQuery::No));

    //Make a code available
    DgnLockSet locks;
    codes.clear();
    codes.insert(code2);
    EXPECT_STATUS(Success, db1.BriefcaseManager().Demote(locks, codes));

    //If all codes are available it should return true
    req.Reset();
    req.SetOptions(IBriefcaseManager::ResponseOptions::CodeState);
    req.Codes().insert(code2);  // available
    req.Codes().insert(code3);  // new code
    EXPECT_TRUE(db2.BriefcaseManager().AreResourcesAvailable(req, &response, IBriefcaseManager::FastQuery::No));

    //Discard a code
    EXPECT_STATUS(Success, db1.BriefcaseManager().ReserveCodes(codes).Result());
    auto pStyle = AnnotationTextStyle::GetForEdit(db1, "Code1");
    pStyle->SetName("Code2");
    EXPECT_TRUE(pStyle->Update().IsValid());
    pStyle = nullptr;
    db1.SaveChanges();
    Utf8String cs2 = PushPendingChanges(*briefcase1);

    //Discarded codes should be available
    EXPECT_SUCCESS(briefcase2->PullAndMerge()->GetResult());
    req.Reset();
    req.SetOptions(IBriefcaseManager::ResponseOptions::CodeState);
    req.Codes().insert(code1);  // discarded
    req.Codes().insert(code3);  // new code
    EXPECT_TRUE(db2.BriefcaseManager().AreResourcesAvailable(req, &response, IBriefcaseManager::FastQuery::No));
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Algirdas.Mikoliunas             08/2016
//---------------------------------------------------------------------------------------
TEST_F(iModelManagerTests, GetCodeMaximumIndex)
    {
    //Prapare imodel and acquire briefcases
    auto briefcase1 = AcquireBriefcase();
    DgnDbR db1 = briefcase1->GetDgnDb();

    // First code spec
    CodeSpecPtr codeSpec1 = CodeSpec::Create(db1, "CodeSpec1");
    ASSERT_TRUE(codeSpec1.IsValid());
    ASSERT_EQ(codeSpec1->GetScope().GetType(), CodeScopeSpec::Type::Repository);
    codeSpec1->GetFragmentSpecsR().push_back(CodeFragmentSpec::FromFixedString("PMP-"));
    auto fragmentSpec = CodeFragmentSpec::FromSequence();
    fragmentSpec.SetMinChars(4);
    codeSpec1->GetFragmentSpecsR().push_back(fragmentSpec);
    db1.CodeSpecs().Insert(*codeSpec1);

    // Second code spec
    CodeSpecPtr codeSpec2 = CodeSpec::Create(db1, "CodeSpec2");
    ASSERT_TRUE(codeSpec2.IsValid());
    ASSERT_EQ(codeSpec2->GetScope().GetType(), CodeScopeSpec::Type::Repository);
    codeSpec2->GetFragmentSpecsR().push_back(CodeFragmentSpec::FromFixedString("P-"));
    auto fragmentSpec2 = CodeFragmentSpec::FromSequence();
    fragmentSpec2.SetMinChars(4);
    codeSpec2->GetFragmentSpecsR().push_back(fragmentSpec2);
    db1.CodeSpecs().Insert(*codeSpec2);

    auto partition1_1 = CreateAndInsertModeledElement("Model1-1", db1);
    db1.SaveChanges();

    //Reserve codes
    DgnCode code1 = DgnCode(codeSpec1->GetCodeSpecId(), codeSpec1->GetScopeElementId(*partition1_1), "PMP-0010");
    DgnCode code2 = DgnCode(codeSpec1->GetCodeSpecId(), codeSpec1->GetScopeElementId(*partition1_1), "PMP-0020");
    DgnCodeSet codes;
    codes.insert(code1);
    codes.insert(code2);
    EXPECT_STATUS(Success, db1.BriefcaseManager().ReserveCodes(codes).Result());

    // Try with CodeSequence class
    auto codeSequence = CodeSequence(codeSpec1->GetCodeSpecId(), GetScopeString(codeSpec1, *partition1_1), "PMP-####");
    auto codeResult = briefcase1->GetiModelConnection().QueryCodeMaximumIndex(codeSequence)->GetResult();
    auto resultTemplate = codeResult.GetValue();
    EXPECT_EQ("0020", resultTemplate.GetValue());
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Algirdas.Mikoliunas             08/2016
//---------------------------------------------------------------------------------------
TEST_F(iModelManagerTests, GetCodeNextAvailable)
    {
    //Prapare imodel and acquire briefcases
    auto briefcase1 = AcquireBriefcase();
    DgnDbR db1 = briefcase1->GetDgnDb();

    // First code spec
    CodeSpecPtr codeSpec1 = CodeSpec::Create(db1, "CodeSpec1");
    ASSERT_TRUE(codeSpec1.IsValid());
    ASSERT_EQ(codeSpec1->GetScope().GetType(), CodeScopeSpec::Type::Repository);
    codeSpec1->GetFragmentSpecsR().push_back(CodeFragmentSpec::FromFixedString("PMP-"));
    auto fragmentSpec = CodeFragmentSpec::FromSequence();
    fragmentSpec.SetMinChars(4);
    fragmentSpec.SetStartNumber(10);
    fragmentSpec.SetNumberGap(5);
    codeSpec1->GetFragmentSpecsR().push_back(fragmentSpec);
    db1.CodeSpecs().Insert(*codeSpec1);

    // Second code spec
    CodeSpecPtr codeSpec2 = CodeSpec::Create(db1, "CodeSpec2");
    ASSERT_TRUE(codeSpec2.IsValid());
    ASSERT_EQ(codeSpec2->GetScope().GetType(), CodeScopeSpec::Type::Repository);
    codeSpec2->GetFragmentSpecsR().push_back(CodeFragmentSpec::FromFixedString("P-"));
    auto fragmentSpec2 = CodeFragmentSpec::FromSequence();
    fragmentSpec2.SetMinChars(4);
    fragmentSpec2.SetStartNumber(10);
    fragmentSpec2.SetNumberGap(5);
    codeSpec2->GetFragmentSpecsR().push_back(fragmentSpec2);
    db1.CodeSpecs().Insert(*codeSpec2);

    auto partition1_1 = CreateAndInsertModeledElement("Model1-1", db1);
    db1.SaveChanges();

    //Reserve codes
    DgnCode code1 = DgnCode(codeSpec1->GetCodeSpecId(), codeSpec1->GetScopeElementId(*partition1_1), "PMP-0010");
    DgnCode code2 = DgnCode(codeSpec1->GetCodeSpecId(), codeSpec1->GetScopeElementId(*partition1_1), "PMP-0020");
    DgnCodeSet codes;
    codes.insert(code1);
    codes.insert(code2);
    EXPECT_STATUS(Success, db1.BriefcaseManager().ReserveCodes(codes).Result());
    
    // Try with CodeSequence class
    auto codeSequence = CodeSequence(codeSpec1->GetCodeSpecId(), GetScopeString(codeSpec1, *partition1_1), "PMP-####");
    auto templatesResult = briefcase1->GetiModelConnection().QueryCodeNextAvailable(codeSequence, 10, 5)->GetResult();
    auto resultTemplate = templatesResult.GetValue();
    EXPECT_EQ("0015", resultTemplate.GetValue());
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    julius.cepukenas                 08/2016
//---------------------------------------------------------------------------------------
TEST_F(iModelManagerTests, QueryInformationAboutSpecificBriefcase)
    {
    //Setup connection
    auto imodelConnection = ConnectToiModel(*m_client, m_imodel);

    //Acquire Briefcase
    auto briefcase = AcquireBriefcase();

    //Check if only one Briefcase is retrieved
    auto result = imodelConnection->QueryBriefcaseInfo(briefcase->GetBriefcaseId())->GetResult();
    EXPECT_SUCCESS(result);

    //Check if Briefcase information contains correct data
    auto briefcaseInfo = result.GetValue();
    EXPECT_EQ(briefcase->GetBriefcaseId(), briefcaseInfo->GetId());

    // Previously UserOwned was an email. Now it is UserId, we don't have it here
    EXPECT_GT(briefcaseInfo->GetUserOwned().size(), 0);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    julius.cepukenas                 08/2016
//---------------------------------------------------------------------------------------
TEST_F(iModelManagerTests, QueryInformationAboutSpecificInvalidBriefcase)
    {
    //Setup connection
    auto imodelConnection = ConnectToiModel(*m_client, m_imodel);

    auto result = imodelConnection->QueryBriefcaseInfo(BeSQLite::BeBriefcaseId())->GetResult();
    EXPECT_FALSE(result.IsSuccess());
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    julius.cepukenas                 08/2016
//---------------------------------------------------------------------------------------
TEST_F(iModelManagerTests, QueryInformationAboutAllAvailableBriefcases)
    {
    //For this test, the information contained in briefcases information is not tested
    //Such test is performed in QueryInformationAboutSpecificBriefcase
    //This test is in order to verify the functionality of vorious briefcase information query option

    //Setup connection
    auto imodelConnection = ConnectToiModel(*m_client, m_imodel);

    //Query briefcases info when no briefcases are acquired
    auto result = imodelConnection->QueryAllBriefcasesInfo()->GetResult();
    EXPECT_SUCCESS(result);
    EXPECT_EQ(0, result.GetValue().size());

    auto nonAdminClient = SetUpClient(IntegrationTestSettings::Instance().GetValidHost(), IntegrationTestSettings::Instance().GetValidNonAdminCredentials());

    //Acquire three briefcases
    auto briefcase1 = AcquireBriefcase();
    auto briefcase2 = AcquireBriefcase();
    auto briefcase3 = IntegrationTestsBase::AcquireBriefcase(*nonAdminClient, *m_imodel);

    //Query information about all acquired briefcases
    result = imodelConnection->QueryAllBriefcasesInfo()->GetResult();
    EXPECT_SUCCESS(result);
    EXPECT_EQ(3, result.GetValue().size());
    }
//---------------------------------------------------------------------------------------
//@bsimethod                                    julius.cepukenas                 08/2016
//---------------------------------------------------------------------------------------
TEST_F(iModelManagerTests, QueryInformationAboutASubSetOfAvailableBriefcases)
    {
    //Setup connection
    auto imodelConnection = ConnectToiModel(*m_client, m_imodel);

    auto nonAdminClient = SetUpClient(IntegrationTestSettings::Instance().GetValidHost(), IntegrationTestSettings::Instance().GetValidNonAdminCredentials());

    //Acquire three briefcases
    auto briefcase1 = AcquireBriefcase();
    auto briefcase2 = AcquireBriefcase();
    auto briefcase3 = IntegrationTestsBase::AcquireBriefcase(*nonAdminClient, *m_imodel);
    
    bvector<BeSQLite::BeBriefcaseId> queryBriefcases;
    //Query information with empty set of briefcases
    auto result = imodelConnection->QueryBriefcasesInfo(queryBriefcases)->GetResult(); 
    EXPECT_SUCCESS(result);
    EXPECT_EQ(0, result.GetValue().size());

    //Query information about subset of available briefcases
    queryBriefcases.push_back(briefcase1->GetBriefcaseId());
    queryBriefcases.push_back(briefcase3->GetBriefcaseId());

    result = imodelConnection->QueryBriefcasesInfo(queryBriefcases)->GetResult();
    EXPECT_SUCCESS(result);
    EXPECT_EQ(2, result.GetValue().size());
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Andrius.Zonys                   08/2016
//---------------------------------------------------------------------------------------
TEST_F(iModelManagerTests, PushAndRelinquishCodesLocks)
    {
    //Prapare imodel and acquire briefcases
    auto briefcase1 = AcquireBriefcase();
    auto briefcase2 = AcquireBriefcase();
    DgnDbR db1 = briefcase1->GetDgnDb();
    DgnDbR db2 = briefcase2->GetDgnDb();
    auto imodelManager1 = _GetRepositoryManager(db1);
    auto imodelManager2 = _GetRepositoryManager(db2);

    ExpectCodesCount(*briefcase1, 0);
    ExpectCodesCount(*briefcase2, 0);
    ExpectLocksCount(*briefcase1, 0);
    ExpectLocksCount(*briefcase2, 0);

    //Create two models in different briefcases. This should also acquire codes and locks automatically.
    auto partition1_1 = CreateAndInsertModeledElement ("Model1-1", db1);
    auto partition2_1 = CreateAndInsertModeledElement ("Model2-1", db2);

    ExpectCodesCount(*briefcase1, 1);
    ExpectCodesCount(*briefcase2, 1);
    ExpectCodeState(CreateCodeReserved(partition1_1->GetCode(), db1), imodelManager1);
    ExpectCodeState(CreateCodeReserved(partition2_1->GetCode(), db2), imodelManager2);

    //Push changes.
    db1.SaveChanges ();
    db2.SaveChanges ();
    Utf8String changeSet1 = PushPendingChanges (*briefcase1, false);        // Don't release codes and locks.
    Utf8String changeSet2 = PushPendingChanges (*briefcase2, false);        // Don't release codes and locks.
    ExpectCodeState(CreateCodeUsed(partition1_1->GetCode(), changeSet1), imodelManager1);
    ExpectCodeState(CreateCodeUsed(partition2_1->GetCode(), changeSet2), imodelManager2);
    ExpectLocksCount(*briefcase1, 3);
    ExpectLocksCount(*briefcase2, 3);

    //Create additional two models in different briefcases. This should also acquire codes and locks automatically.
    auto partition1_2 = CreateAndInsertModeledElement ("Model1-2", db1);
    auto partition2_2 = CreateAndInsertModeledElement ("Model2-2", db2);

    //Reserve two codes without actual model.
    DgnCode modelCode1_3 = MakeModelCode("Model1-3", db1);
    DgnCode modelCode2_3 = MakeModelCode("Model2-3", db1);
    DgnCodeSet codes;
    codes.insert(modelCode1_3);
    EXPECT_STATUS(Success, db1.BriefcaseManager().ReserveCodes(codes).Result());
    codes.clear();
    codes.insert(modelCode2_3);
    EXPECT_STATUS(Success, db2.BriefcaseManager().ReserveCodes(codes).Result());

    ExpectCodesCount(*briefcase1, 2);
    ExpectCodesCount(*briefcase2, 2);
    ExpectCodeState(CreateCodeReserved(partition1_2->GetCode(), db1), imodelManager1);
    ExpectCodeState(CreateCodeReserved(modelCode1_3, db1), imodelManager1);
    ExpectCodeState(CreateCodeReserved(partition2_2->GetCode(), db2), imodelManager2);
    ExpectCodeState(CreateCodeReserved(modelCode2_3, db2), imodelManager2);

    //Delete two models and push changes.
    EXPECT_EQ(DgnDbStatus::Success, partition1_1->Delete());
    EXPECT_EQ(DgnDbStatus::Success, partition2_1->Delete());
    db1.SaveChanges ();
    db2.SaveChanges ();
    Utf8String changeSet3 = PushPendingChanges (*briefcase1, false);        // Don't release codes and locks.
    Utf8String changeSet4 = PushPendingChanges (*briefcase2, true);         // Release all codes and locks.
    ExpectCodesCount(*briefcase1, 1);
    ExpectCodesCount(*briefcase2, 0);
    ExpectNoCodeWithState(CreateCodeDiscarded(partition1_1->GetCode(), changeSet3), imodelManager1);
    ExpectCodeState(CreateCodeUsed     (partition1_2->GetCode(), changeSet3), imodelManager1);
    ExpectCodeState(CreateCodeReserved (modelCode1_3, db1),       imodelManager1);
    ExpectNoCodeWithState(CreateCodeDiscarded(partition2_1->GetCode(), changeSet4), imodelManager2);
    ExpectCodeState(CreateCodeUsed     (partition2_2->GetCode(), changeSet4), imodelManager2);
    ExpectNoCodeWithState(CreateCodeAvailable(modelCode2_3),      imodelManager2);
    ExpectLocksCount(*briefcase1, 4);
    ExpectLocksCount(*briefcase2, 0);

    //Create one more model and release all locks and codes.
    auto partition1_4 = CreateAndInsertModeledElement ("Model1-4", db1);
    ExpectCodesCount(*briefcase1, 2);
    ExpectCodeState(CreateCodeReserved(partition1_4->GetCode(), db1), imodelManager1);
    db1.SaveChanges ();
    Utf8String changeSet5 = PushPendingChanges (*briefcase1, true);         // Release all codes and locks.
    ExpectCodesCount(*briefcase1, 0);
    ExpectNoCodeWithState(CreateCodeAvailable(modelCode1_3),      imodelManager1);
    ExpectCodeState(CreateCodeUsed     (partition1_4->GetCode(), changeSet5), imodelManager1);
    ExpectLocksCount(*briefcase1, 0);

    LockableIdSet locks;
    codes.clear();
    codes.insert (partition1_1->GetCode());
    codes.insert (partition1_2->GetCode());
    codes.insert (partition1_4->GetCode());
    codes.insert (partition2_1->GetCode());
    codes.insert (partition2_2->GetCode());
    auto result = briefcase1->GetiModelConnection ().QueryCodesLocksById (codes, locks)->GetResult ();
    EXPECT_SUCCESS(result);

    //Check if we can reserve reserved and discarded codes without changeSet.
    SetLastPulledChangeSetId (*briefcase1, "");
    SetLastPulledChangeSetId (*briefcase2, "");
    codes.clear();
    codes.insert(modelCode2_3);
    EXPECT_STATUS(Success, db1.BriefcaseManager().ReserveCodes(codes).Result());
    codes.clear();
    codes.insert(modelCode1_3);
    EXPECT_STATUS(Success, db2.BriefcaseManager().ReserveCodes(codes).Result());
    ExpectCodesCount(*briefcase1, 1);
    ExpectCodesCount(*briefcase2, 1);

    //We should not be able to acquire locks if we haven't pulled a required changeSet.
    SetLastPulledChangeSetId (*briefcase1, changeSet2);
    SetLastPulledChangeSetId (*briefcase2, changeSet1);
    EXPECT_EQ (RepositoryStatus::RevisionRequired, AcquireLock (db2, *partition1_1));
    EXPECT_EQ (RepositoryStatus::RevisionRequired, AcquireLock (db2, *partition1_2));
    EXPECT_EQ (RepositoryStatus::RevisionRequired, AcquireLock (db1, *partition2_1));
    EXPECT_EQ (RepositoryStatus::RevisionRequired, AcquireLock (db1, *partition2_2));
    ExpectLocksCount (*briefcase1, 0);
    ExpectLocksCount (*briefcase2, 0);

    //We should be able to acquire locks if we pulled a required changeSet.
    SetLastPulledChangeSetId (*briefcase1, changeSet4);
    SetLastPulledChangeSetId (*briefcase2, changeSet3);
    EXPECT_EQ (RepositoryStatus::Success,          AcquireLock (db2, *partition1_1));
    EXPECT_EQ (RepositoryStatus::Success,          AcquireLock (db2, *partition1_2));
    EXPECT_EQ (RepositoryStatus::RevisionRequired, AcquireLock (db2, *partition1_4));
    EXPECT_EQ (RepositoryStatus::Success,          AcquireLock (db1, *partition2_1));
    EXPECT_EQ (RepositoryStatus::Success,          AcquireLock (db1, *partition2_2));
    ExpectLocksCount (*briefcase1, 2);
    ExpectLocksCount (*briefcase2, 2);

    SetLastPulledChangeSetId (*briefcase2, changeSet5);
    EXPECT_EQ (RepositoryStatus::Success,          AcquireLock (db2, *partition1_4));
    ExpectLocksCount (*briefcase2, 3);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Andrius.Zonys                   09/2016
//---------------------------------------------------------------------------------------
TEST_F(iModelManagerTests, RelinquishOtherUserCodes)
    {
    auto nonAdminClient = SetUpClient(IntegrationTestSettings::Instance().GetValidHost(), IntegrationTestSettings::Instance().GetValidNonAdminCredentials());

    // Prapare imodel and acquire briefcases.
    auto briefcase1 = IntegrationTestsBase::AcquireBriefcase(*nonAdminClient, *m_imodel);
    auto briefcase2 = AcquireBriefcase();
    DgnDbR db1 = briefcase1->GetDgnDb();
    DgnDbR db2 = briefcase2->GetDgnDb();
    m_pHost->SetRepositoryAdmin (nonAdminClient->GetiModelAdmin());
    auto imodelManager1 = _GetRepositoryManager(db1);
    m_pHost->SetRepositoryAdmin (m_client->GetiModelAdmin());

    DgnCode modelCode1 = MakeModelCode("Model1", db1);
    DgnCode modelCode2 = MakeModelCode("Model2", db1);
    DgnCode modelCode3 = MakeModelCode("Model3", db1);
    DgnCode modelCode4 = MakeModelCode("Model4", db1);

    // Briefcase1 acquires three model codes.
    DgnCodeSet codes;
    codes.insert(modelCode1);
    codes.insert(modelCode2);
    codes.insert(modelCode3);
    m_pHost->SetRepositoryAdmin (nonAdminClient->GetiModelAdmin());
    EXPECT_STATUS(Success, db1.BriefcaseManager().ReserveCodes(codes).Result());
    m_pHost->SetRepositoryAdmin (m_client->GetiModelAdmin());
    ExpectCodesCount(*briefcase1, 3);
    ExpectCodeState(CreateCodeReserved(modelCode1, db1), imodelManager1);
    ExpectCodeState(CreateCodeReserved(modelCode2, db1), imodelManager1);
    ExpectCodeState(CreateCodeReserved(modelCode3, db1), imodelManager1);

    // Briefcase2 relinquishes other briefcase codes.
    EXPECT_SUCCESS(briefcase2->GetiModelConnection().RelinquishCodesLocks (briefcase1->GetBriefcaseId())->GetResult());
    ExpectCodesCount(*briefcase1, 0);

    // Briefcase1 should be able to aquire codes again.
    m_pHost->SetRepositoryAdmin (nonAdminClient->GetiModelAdmin());
    db1.BriefcaseManager().RefreshFromRepository(); // BriefcaseManager says modelCode1 is already reserved if we don't call refresh.
    EXPECT_STATUS(Success, db1.BriefcaseManager().ReserveCodes(codes).Result());
    ExpectCodesCount(*briefcase1, 3);

    // Briefcase1 acquires modelCode4 and makes it used.
    auto model4 = CreateModel ("Model4", db1);
    db1.SaveChanges ();
    Utf8String changeSet1 = PushPendingChanges (*briefcase1);
    ExpectCodesCount(*briefcase1, 3);
    ExpectCodeState(CreateCodeReserved(modelCode1, db1), imodelManager1);
    ExpectCodeState(CreateCodeReserved(modelCode2, db1), imodelManager1);
    ExpectCodeState(CreateCodeReserved(modelCode3, db1), imodelManager1);
    ExpectCodeState(CreateCodeUsed(modelCode4, changeSet1), imodelManager1);
    m_pHost->SetRepositoryAdmin (m_client->GetiModelAdmin());

    // Briefcase2 should not be able to discard reserved code.
    DgnLockSet locks;
    codes.clear();
    codes.insert (modelCode3);
    auto result = briefcase2->GetiModelConnection().DemoteCodesLocks (locks, codes, briefcase2->GetBriefcaseId(), db2.GetDbGuid())->GetResult();
    EXPECT_EQ(Error::Id::CodeReservedByAnotherBriefcase, result.GetError().GetId());

    // Briefcase2 relinquishes specific codes.
    codes.clear();
    codes.insert(modelCode1);
    codes.insert(modelCode2);
    result = briefcase2->GetiModelConnection().DemoteCodesLocks (locks, codes, briefcase1->GetBriefcaseId(), db2.GetDbGuid())->GetResult();
    EXPECT_SUCCESS(result);
    ExpectCodesCount(*briefcase1, 1);
    ExpectCodeState(CreateCodeReserved(modelCode3, db1), imodelManager1);
    ExpectCodeState(CreateCodeUsed(modelCode4, changeSet1), imodelManager1);

    // Briefcase2 aquires modelCode1.
    auto model1 = CreateModel ("Model1", db2);
    db2.SaveChanges ();

    // Briefcase1 should not be able to release all other briefcase codes.
    result = briefcase1->GetiModelConnection().RelinquishCodesLocks (briefcase2->GetBriefcaseId())->GetResult();
    EXPECT_EQ(Error::Id::UserDoesNotHavePermission, result.GetError().GetId());

    // Briefcase1 should not be able to release specific other briefcase codes.
    codes.clear();
    codes.insert (modelCode1);
    result = briefcase1->GetiModelConnection().DemoteCodesLocks (locks, codes, briefcase2->GetBriefcaseId(), db1.GetDbGuid())->GetResult();
    EXPECT_EQ(Error::Id::UserDoesNotHavePermission, result.GetError().GetId());

    // Briefcase2 pushes his changes.
    Utf8String changeSet2 = PushPendingChanges (*briefcase2);
    ExpectCodesCount(*briefcase1, 1);
    ExpectCodesCount(*briefcase2, 0);
    ExpectCodeState(CreateCodeUsed(modelCode1, changeSet2), imodelManager1);
    ExpectCodeState(CreateCodeReserved(modelCode3, db1), imodelManager1);
    ExpectCodeState(CreateCodeUsed(modelCode4, changeSet1), imodelManager1);

    // Briefcase2 relinquishes other briefcase codes while some used codes exists.
    result = briefcase2->GetiModelConnection().RelinquishCodesLocks (briefcase1->GetBriefcaseId())->GetResult();
    EXPECT_SUCCESS(result);
    ExpectCodesCount(*briefcase1, 0);
    ExpectCodesCount(*briefcase2, 0);
    ExpectCodeState(CreateCodeUsed(modelCode1, changeSet2), imodelManager1);
    ExpectCodeState(CreateCodeUsed(modelCode4, changeSet1), imodelManager1);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Andrius.Zonys                   09/2016
//---------------------------------------------------------------------------------------
TEST_F(iModelManagerTests, RelinquishOtherUserLocks)
    {
    auto nonAdminClient = SetUpClient(IntegrationTestSettings::Instance().GetValidHost(), IntegrationTestSettings::Instance().GetValidNonAdminCredentials());

    // Prapare imodel and acquire briefcases.
    auto briefcase1 = IntegrationTestsBase::AcquireBriefcase(*nonAdminClient, *m_imodel);
    auto briefcase2 = AcquireBriefcase();
    DgnDbR db1 = briefcase1->GetDgnDb();
    DgnDbR db2 = briefcase2->GetDgnDb();

    // Briefcase1 creates three models. This should also acquire locks automatically.
    m_pHost->SetRepositoryAdmin (nonAdminClient->GetiModelAdmin());
    auto model1 = CreateModel ("Model1", db1);
    auto model2 = CreateModel ("Model2", db1);
    auto model3 = CreateModel ("Model3", db1);
    m_pHost->SetRepositoryAdmin (m_client->GetiModelAdmin());
    db1.SaveChanges ();
    ExpectLocksCount(*briefcase1, 2);

    // Briefcase2 relinquishes other briefcase codes and locks.
    EXPECT_SUCCESS(briefcase2->GetiModelConnection().RelinquishCodesLocks (briefcase1->GetBriefcaseId())->GetResult());
    ExpectLocksCount(*briefcase1, 0);

    // Briefcase1 should be able to push changes since nobody owns them.
    Utf8String changeSet1 = PushPendingChanges (*briefcase1);
    ExpectLocksCount(*briefcase1, 8);

    // Briefcase1 deletes two models.
    EXPECT_EQ (DgnDbStatus::Success, model1->Delete());
    EXPECT_EQ (DgnDbStatus::Success, model2->Delete());
    db1.SaveChanges ();

    // Briefcase2 relinquishes specific locks and acquires its own lock.
    DgnLockSet locks;
    locks.insert (DgnLock (LockableId (model1->GetModelId()), LockLevel::None));
    locks.insert (DgnLock (LockableId (model3->GetModelId()), LockLevel::None));
    DgnCodeSet codes;
    auto result = briefcase2->GetiModelConnection().DemoteCodesLocks (locks, codes, briefcase1->GetBriefcaseId(), db2.GetDbGuid())->GetResult();
    EXPECT_SUCCESS(result);

    EXPECT_SUCCESS(briefcase2->PullAndMerge()->GetResult());
    DgnModelPtr model1_2 = briefcase2->GetDgnDb ().Models ().GetModel (model1->GetModelId ());
    IBriefcaseManager::Request req;
    EXPECT_EQ(RepositoryStatus::Success, db2.BriefcaseManager().PrepareForModelDelete(req, *model1_2, IBriefcaseManager::PrepareAction::Acquire));
    EXPECT_EQ (DgnDbStatus::Success, model1_2->Delete ());
    db2.SaveChanges ();
    ExpectLocksCount(*briefcase1, 6);
    ExpectLocksCount(*briefcase2, 2);

    // Briefcase1 should not be able to push his changes since one lock is owned.
    m_pHost->SetRepositoryAdmin (nonAdminClient->GetiModelAdmin());
    auto pushResult = briefcase1->PullMergeAndPush()->GetResult();
    EXPECT_EQ(Error::Id::LockOwnedByAnotherBriefcase, pushResult.GetError().GetId());
    ExpectLocksCount(*briefcase1, 6);
    ExpectLocksCount(*briefcase2, 2);

    // Briefcase1 should not be able to release all other briefcase locks.
    result = briefcase1->GetiModelConnection().RelinquishCodesLocks (briefcase2->GetBriefcaseId())->GetResult();
    EXPECT_EQ(Error::Id::UserDoesNotHavePermission, result.GetError().GetId());

    // Briefcase1 should not be able to release specific other briefcase locks.
    result = briefcase1->GetiModelConnection().DemoteCodesLocks (locks, codes, briefcase2->GetBriefcaseId(), db1.GetDbGuid())->GetResult();
    m_pHost->SetRepositoryAdmin (m_client->GetiModelAdmin());
    EXPECT_EQ(Error::Id::UserDoesNotHavePermission, result.GetError().GetId());

    // Briefcase2 should be able to push changes but he will need to wait.
    pushResult = briefcase2->PullMergeAndPush()->GetResult();
    EXPECT_EQ(Error::Id::AnotherUserPushing, pushResult.GetError().GetId());

    ExpectLocksCountById (*briefcase1, 2, false, LockableId (*model3), LockableId (model1->GetDgnDb ()));
    ExpectLocksCountById (*briefcase1, 4, false, LockableId (*model1), LockableId (*model2), LockableId (model1->GetDgnDb ()));
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Andrius.Zonys                   09/2016
//---------------------------------------------------------------------------------------
TEST_F(iModelManagerTests, RelinquishOtherUserCodesLocks)
    {
    auto nonAdminClient = SetUpClient(IntegrationTestSettings::Instance().GetValidHost(), IntegrationTestSettings::Instance().GetValidNonAdminCredentials());

    // Prapare imodel and acquire briefcases.
    auto briefcase1 = IntegrationTestsBase::AcquireBriefcase(*nonAdminClient, *m_imodel);
    auto briefcase2 = AcquireBriefcase();
    DgnDbR db1 = briefcase1->GetDgnDb();
    DgnDbR db2 = briefcase2->GetDgnDb();
    m_pHost->SetRepositoryAdmin (nonAdminClient->GetiModelAdmin());
    auto imodelManager1 = _GetRepositoryManager(db1);
    m_pHost->SetRepositoryAdmin (m_client->GetiModelAdmin());
    auto imodelManager2 = _GetRepositoryManager(db2);

    // Briefcase1 creates two models. This should also acquire codes and locks automatically.
    m_pHost->SetRepositoryAdmin (nonAdminClient->GetiModelAdmin());
    auto modelElem1 = CreateAndInsertModeledElement ("Model1", db1);
    auto modelElem2 = CreateAndInsertModeledElement ("Model2", db1);
    m_pHost->SetRepositoryAdmin (m_client->GetiModelAdmin());
    db1.SaveChanges ();
    ExpectCodesCount(*briefcase1, 2);
    ExpectCodeState(CreateCodeReserved(modelElem1->GetCode(), db1), imodelManager1);
    ExpectCodeState(CreateCodeReserved(modelElem2->GetCode(), db1), imodelManager1);
    ExpectLocksCount(*briefcase1, 2);

    // Briefcase2 relinquishes other briefcase codes and locks.
    auto result = briefcase2->GetiModelConnection().RelinquishCodesLocks (briefcase1->GetBriefcaseId())->GetResult();
    EXPECT_SUCCESS(result);
    ExpectCodesCount(*briefcase1, 0);
    ExpectLocksCount(*briefcase1, 0);

    // Briefcase1 should be able to push changes since nobody owns them.
    Utf8String changeSet1 = PushPendingChanges (*briefcase1);
    ExpectCodesCount(*briefcase1, 0);
    ExpectCodeState(CreateCodeUsed(modelElem1->GetCode(), changeSet1), imodelManager1);
    ExpectCodeState(CreateCodeUsed(modelElem2->GetCode(), changeSet1), imodelManager1);
    ExpectLocksCount(*briefcase1, 4);

    // Briefcase1 deletes one model and creates two new.
    EXPECT_EQ (DgnDbStatus::Success, modelElem2->Delete());
    m_pHost->SetRepositoryAdmin (nonAdminClient->GetiModelAdmin());
    auto modelElem3 = CreateAndInsertModeledElement ("Model3", db1);
    auto modelElem4 = CreateAndInsertModeledElement ("Model4", db1);
    m_pHost->SetRepositoryAdmin (m_client->GetiModelAdmin());
    db1.SaveChanges ();

    // Briefcase2 should not be able to release reserved code.
    DgnLockSet locks;
    DgnCodeSet codes;
    codes.insert(modelElem1->GetCode());
    codes.insert(modelElem3->GetCode());
    result = briefcase2->GetiModelConnection().DemoteCodesLocks (locks, codes, briefcase2->GetBriefcaseId(), db2.GetDbGuid())->GetResult();
    EXPECT_EQ(Error::Id::CodeReservedByAnotherBriefcase, result.GetError().GetId());
    ExpectCodesCount(*briefcase1, 2);
    ExpectCodeState(CreateCodeUsed(modelElem1->GetCode(), changeSet1), imodelManager1);
    ExpectCodeState(CreateCodeUsed(modelElem2->GetCode(), changeSet1), imodelManager1);
    ExpectCodeState(CreateCodeReserved(modelElem3->GetCode(), db1), imodelManager1);
    ExpectCodeState(CreateCodeReserved(modelElem4->GetCode(), db1), imodelManager1);
    ExpectLocksCount(*briefcase1, 4);

    // Briefcase2 relinquishes specific code and lock.
    locks.clear();
    locks.insert (DgnLock (LockableId (modelElem1->GetElementId()), LockLevel::None));
    codes.clear();
    codes.insert(modelElem3->GetCode());
    result = briefcase2->GetiModelConnection().DemoteCodesLocks (locks, codes, briefcase1->GetBriefcaseId(), db2.GetDbGuid())->GetResult();
    EXPECT_SUCCESS(result);
    ExpectCodesCount(*briefcase1, 1);
    ExpectCodeState(CreateCodeUsed(modelElem1->GetCode(), changeSet1), imodelManager1);
    ExpectCodeState(CreateCodeUsed(modelElem2->GetCode(), changeSet1), imodelManager1);
    ExpectCodeState(CreateCodeReserved(modelElem4->GetCode(), db1), imodelManager1);
    ExpectLocksCount(*briefcase1, 3);

    // Briefcase2 deletes model1.
    EXPECT_SUCCESS(briefcase2->PullAndMerge()->GetResult());
    DgnElementCPtr model1_2 = briefcase2->GetDgnDb().Elements().GetElement (modelElem1->GetElementId());
    IBriefcaseManager::Request req;
    EXPECT_EQ(RepositoryStatus::Success, db2.BriefcaseManager().PrepareForElementDelete(req, *model1_2, IBriefcaseManager::PrepareAction::Acquire));
    EXPECT_EQ (DgnDbStatus::Success, model1_2->Delete ());
    db2.SaveChanges ();
    ExpectCodesCount(*briefcase1, 1);
    ExpectLocksCount(*briefcase1, 3);
    ExpectCodesCount(*briefcase2, 0);
    ExpectCodeState(CreateCodeUsed(modelElem1->GetCode(), changeSet1), imodelManager2);
    ExpectLocksCount(*briefcase2, 3);

    // Briefcase2 acquires modelCode3.
    EXPECT_STATUS(Success, db2.BriefcaseManager().ReserveCodes(codes).Result());
    ExpectCodesCount(*briefcase2, 1);
    ExpectCodeState(CreateCodeReserved(modelElem3->GetCode(), db2), imodelManager2);

    // Briefcase1 should not be able to release all other briefcase locks and codes.
    result = briefcase1->GetiModelConnection().RelinquishCodesLocks (briefcase2->GetBriefcaseId())->GetResult();
    EXPECT_EQ(Error::Id::UserDoesNotHavePermission, result.GetError().GetId());

    // Briefcase1 should not be able to release specific other briefcase locks and codes.
    locks.clear();
    locks.insert (DgnLock (LockableId (modelElem1->GetDgnDb ()), LockLevel::None));
    result = briefcase1->GetiModelConnection().DemoteCodesLocks (locks, codes, briefcase2->GetBriefcaseId(), db1.GetDbGuid())->GetResult();
    EXPECT_EQ(Error::Id::UserDoesNotHavePermission, result.GetError().GetId());

    // Briefcase2 should be able to push changes.
    Utf8String changeSet2 = PushPendingChanges (*briefcase2);

    // Briefcase1 should not be able to push his changes since one code is owned.
    m_pHost->SetRepositoryAdmin (nonAdminClient->GetiModelAdmin());
    auto pushResult = briefcase1->PullMergeAndPush()->GetResult();
    m_pHost->SetRepositoryAdmin (m_client->GetiModelAdmin());
    EXPECT_EQ(Error::Id::CodeReservedByAnotherBriefcase, pushResult.GetError().GetId());

    ExpectCodesCount(*briefcase1, 1);
    ExpectLocksCount(*briefcase1, 3);
    ExpectCodesCount(*briefcase2, 1);
    ExpectNoCodeWithState(CreateCodeDiscarded(modelElem1->GetCode(), changeSet2), imodelManager2);
    ExpectCodeState(CreateCodeReserved(modelElem3->GetCode(), db2), imodelManager2);
    ExpectLocksCount(*briefcase2, 3);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Algirdas.Mikoliunas             02/2017
//---------------------------------------------------------------------------------------
TEST_F(iModelManagerTests, LockSchemas)
    {
    //Prapare imodel and acquire briefcases
    auto briefcase1 = AcquireBriefcase();
    auto briefcase2 = AcquireBriefcase();
    DgnDbR db1 = briefcase1->GetDgnDb();
    DgnDbR db2 = briefcase2->GetDgnDb();

    ExpectUnavailableLocksCount(*briefcase1, 0);
    ExpectUnavailableLocksCount(*briefcase2, 0);

    // Lock schemas with the first briefcase
    auto response = db1.BriefcaseManager().LockSchemas().Result();
    EXPECT_EQ(RepositoryStatus::Success, response);

    // Second briefcase should not be able to lock schemas
    response = db2.BriefcaseManager().LockSchemas().Result();
    EXPECT_EQ(RepositoryStatus::LockAlreadyHeld, response);

    ExpectUnavailableLocksCount(*briefcase1, 0);
    ExpectUnavailableLocksCount(*briefcase2, 2);
    
    // First briefcase releases locks
    EXPECT_EQ(RepositoryStatus::Success, db1.BriefcaseManager().RelinquishLocks());

    // Now second briefcase sould be able to lock schemas
    response = db2.BriefcaseManager().LockSchemas().Result();
    EXPECT_EQ(RepositoryStatus::Success, response);

    IBriefcaseManager::Request req(IBriefcaseManager::ResponseOptions::LockState);
    IBriefcaseManager::Response resp(IBriefcaseManager::RequestPurpose::Query, req.Options());
    req.Locks().GetLockSet().insert(DgnLock(LockableId(LockableType::Schemas, BeInt64Id(1)), LockLevel::Exclusive));
    EXPECT_FALSE(db1.BriefcaseManager().AreResourcesAvailable(req, &resp, IBriefcaseManager::FastQuery::No));
    EXPECT_TRUE(db2.BriefcaseManager().AreResourcesAvailable(req, &resp, IBriefcaseManager::FastQuery::No));

    // Second briefcase releases all locks
    EXPECT_EQ(RepositoryStatus::Success, db2.BriefcaseManager().RelinquishLocks());

    // Locking schemas with id != 1 should not be available
    req.Locks().Clear();
    req.Locks().GetLockSet().insert(DgnLock(LockableId(LockableType::Schemas, BeInt64Id(2)), LockLevel::Exclusive));
    EXPECT_FALSE(db1.BriefcaseManager().AreResourcesAvailable(req, &resp, IBriefcaseManager::FastQuery::No));
    EXPECT_FALSE(db2.BriefcaseManager().AreResourcesAvailable(req, &resp, IBriefcaseManager::FastQuery::No));
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Algirdas.Mikoliunas             02/2017
//---------------------------------------------------------------------------------------
TEST_F(iModelManagerTests, ModifySchema)
    {
    //Prapare imodel and acquire briefcases
    auto briefcase1 = AcquireBriefcase();
    auto briefcase2 = AcquireBriefcase();
    DgnDbR db1 = briefcase1->GetDgnDb();
    DgnDbR db2 = briefcase2->GetDgnDb();
    
    // Modify schema
    db1.CreateTable("TestTable1", "Id INTEGER PRIMARY KEY, Column1 INTEGER");
    ASSERT_TRUE(db1.Txns().HasDbSchemaChanges());
    db1.SaveChanges("ChangeSet 1");
    ASSERT_FALSE(db1.Txns().HasDbSchemaChanges());

    // Push changeSet with schema changes
    Utf8String changeSet1 = PushPendingChanges(*briefcase1);
    auto changeSetResult = briefcase1->GetiModelConnection().GetChangeSetById(changeSet1)->GetResult();
    ASSERT_TRUE(changeSetResult.IsSuccess());
    ASSERT_EQ(1, changeSetResult.GetValue()->GetContainingChanges());

    auto model4 = CreateModel("Model4", db2);
    db2.SaveChanges();
    ASSERT_FALSE(db2.TableExists("TestTable1"));

    // Push changeSet without schema changes should fail
    auto pushResult = briefcase2->PullMergeAndPush(nullptr, true)->GetResult();
    EXPECT_EQ(Error::Id::MergeSchemaChangesOnOpen, pushResult.GetError().GetId());
    
    // Reload DB with upgrade options
    auto changeSets = briefcase2->GetiModelConnection().DownloadChangeSetsAfterId(briefcase2->GetLastChangeSetPulled(), briefcase2->GetDgnDb().GetDbGuid(), CreateProgressCallback())->GetResult().GetValue();
    bvector<DgnRevisionCP> changeSetsToMerge;
    ConvertToChangeSetPointersVector(changeSets, changeSetsToMerge);
    auto filePath = db2.GetFileName();
    db2.CloseDb();

    // Reload db with changesets
    BeSQLite::DbResult status;
    auto db2Ptr = Dgn::DgnDb::OpenDgnDb(&status, filePath, Dgn::DgnDb::OpenParams(Dgn::DgnDb::OpenMode::ReadWrite, BeSQLite::DefaultTxn::Yes, SchemaUpgradeOptions(changeSetsToMerge)));
    EXPECT_EQ(BeSQLite::DbResult::BE_SQLITE_OK, status);
    
    auto briefcase2Result = m_client->OpenBriefcase(db2Ptr, false)->GetResult();
    EXPECT_SUCCESS(briefcase2Result);
    briefcase2 = briefcase2Result.GetValue();
    
    // Try to push again
    Utf8String changeSet2 = PushPendingChanges(*briefcase2);

    // Check ContainsSchemaChanges set to false
    changeSetResult = briefcase2->GetiModelConnection().GetChangeSetById(changeSet2)->GetResult();
    ASSERT_TRUE(changeSetResult.IsSuccess());
    ASSERT_EQ(0, changeSetResult.GetValue()->GetContainingChanges());
    ASSERT_TRUE(db2Ptr->TableExists("TestTable1"));
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Algirdas.Mikoliunas             03/2017
//---------------------------------------------------------------------------------------
/*TEST_F(iModelManagerTests, GenerateCode)
    {
    //Prapare imodel and acquire briefcases
    auto briefcase1 = AcquireBriefcase();
    DgnDbR db1 = briefcase1->GetDgnDb();
    
    // Create code spec
    CodeSpecPtr codeSpec1 = CodeSpec::Create(db1, "CodeSpec1");
    ASSERT_TRUE(codeSpec1.IsValid());
    ASSERT_EQ(codeSpec1->GetScope().GetType(), CodeScopeSpec::Type::Repository);
    codeSpec1->GetFragmentSpecsR().push_back(CodeFragmentSpec::FromFixedString("PMP-"));
    auto fragmentSpec = CodeFragmentSpec::FromSequence();
    fragmentSpec.SetMinChars(4);
    fragmentSpec.SetMaxChars(4);
    fragmentSpec.SetStartNumber(0);
    fragmentSpec.SetNumberGap(1);
    codeSpec1->GetFragmentSpecsR().push_back(fragmentSpec);
    db1.CodeSpecs().Insert(*codeSpec1);
    
    T_HOST.GetCodeAdmin()._RegisterDefaultCodeSpec(BIS_ECSCHEMA_CLASS_NAME(BIS_CLASS_InformationPartitionElement), "CodeSpec1");
    
    // Generate first code
    PhysicalPartitionPtr partition = PhysicalPartition::Create(*db1.Elements().GetRootSubject(), "Partition");
    ASSERT_EQ(BeSQLite::DbResult::BE_SQLITE_OK, db1.SaveChanges());

    ASSERT_EQ(DgnDbStatus::Success, partition->GenerateCode(true));
    auto newCode = partition->GetCode();
    ASSERT_EQ("PMP-0000", newCode.GetValue());
    
    // Mark PMP-0001 as reserved
    ASSERT_EQ(DgnDbStatus::Success, T_HOST.GetCodeAdmin()._ReserveCode(*partition, codeSpec1->CreateCode("PMP-0001")));

    // Generate second code
    PhysicalPartitionPtr partition2 = PhysicalPartition::Create(*db1.Elements().GetRootSubject(), "Partition");
    db1.SaveChanges();
    ASSERT_EQ(DgnDbStatus::Success, partition2->GenerateCode(true));
    auto newCode2 = partition2->GetCode();
    ASSERT_EQ("PMP-0002", newCode2.GetValue());
    }*/

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas        04/2017
//---------------------------------------------------------------------------------------
TEST_F(iModelManagerTests, CodesStatesResponseTest)
    {
    //Prapare imodel and acquire briefcases
    auto briefcase1 = AcquireBriefcase();
    auto briefcase2 = AcquireBriefcase();
    DgnDbR db1 = briefcase1->GetDgnDb();
    DgnDbR db2 = briefcase2->GetDgnDb();

    //Create a models and push them.
    auto model1 = CreateModel("Model1", briefcase1->GetDgnDb());
    briefcase1->GetDgnDb().SaveChanges();
    PushPendingChanges(*briefcase1);

    auto model2 = CreateModel("Model2", briefcase1->GetDgnDb());
    briefcase1->GetDgnDb().SaveChanges();
    PushPendingChanges(*briefcase1);

    DgnCodeSet codeSet;
    DgnCode code1 = MakeStyleCode("Code1", db1);
    codeSet.insert(code1);

    auto response = db1.BriefcaseManager().ReserveCodes(codeSet, IBriefcaseManager::ResponseOptions::CodeState);
    EXPECT_EQ(RepositoryStatus::Success, response.Result());

    response = db2.BriefcaseManager().ReserveCodes(codeSet, IBriefcaseManager::ResponseOptions::CodeState);
    EXPECT_EQ(RepositoryStatus::CodeUnavailable, response.Result());
    EXPECT_EQ(1, response.CodeStates().size());
    auto codeState = *response.CodeStates().begin();
    EXPECT_EQ(briefcase1->GetBriefcaseId(), codeState.GetReservedBy());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Paulius.Valiunas               07/2017
//---------------------------------------------------------------------------------------
TEST_F(iModelManagerTests, QueryAllUsersInfoTest)
    {
    auto briefcase = AcquireBriefcase();
    auto userInfoResult = m_connection->GetUserInfoManager().QueryAllUsersInfo()->GetResult();
    EXPECT_SUCCESS(userInfoResult);

    auto wantedUserId = m_connection->QueryBriefcaseInfo(briefcase->GetBriefcaseId())->GetResult().GetValue()->GetUserOwned();
    bool success = false;

    for (auto userInfo : userInfoResult.GetValue())
        {
        if (userInfo->GetId() == wantedUserId)
            success = true;
        }

    EXPECT_TRUE(success);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Paulius.Valiunas               07/2017
//---------------------------------------------------------------------------------------
TEST_F(iModelManagerTests, QueryUserInfoTest)
    {
    auto briefcase = AcquireBriefcase();
    auto userInfoResult = m_connection->GetUserInfoManager().QueryUserInfoById(m_connection->QueryBriefcaseInfo(briefcase->GetBriefcaseId())->GetResult().GetValue()->GetUserOwned())->GetResult();
    EXPECT_SUCCESS(userInfoResult);

    auto wantedUserId = m_connection->QueryBriefcaseInfo(briefcase->GetBriefcaseId())->GetResult().GetValue()->GetUserOwned();

    EXPECT_EQ(wantedUserId, userInfoResult.GetValue()->GetId());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Paulius.Valiunas               07/2017
//---------------------------------------------------------------------------------------
TEST_F(iModelManagerTests, QueryInvalidUserInfoTest)
    {
    auto userInfoResult = m_connection->GetUserInfoManager().QueryUserInfoById("Invalid User Id")->GetResult();
    EXPECT_FALSE(userInfoResult.IsSuccess());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Paulius.Valiunas               08/2017
//---------------------------------------------------------------------------------------
TEST_F(iModelManagerTests, QueryUsersInfoByTwoIdsSeparately)
    {
    auto briefcase1 = AcquireBriefcase();
    auto wantedUserId1 = m_connection->QueryBriefcaseInfo(briefcase1->GetBriefcaseId())->GetResult().GetValue()->GetUserOwned();
    auto userInfoResult1 = m_connection->GetUserInfoManager().QueryUserInfoById(wantedUserId1)->GetResult();
    EXPECT_SUCCESS(userInfoResult1);

    // Create new user
    auto wantedUserId2 = GetNonAdminUserId();
    auto userInfoResult2 = m_connection->GetUserInfoManager().QueryUserInfoById(wantedUserId2)->GetResult();
    EXPECT_SUCCESS(userInfoResult2);

    EXPECT_EQ(wantedUserId1, userInfoResult1.GetValue()->GetId());
    EXPECT_EQ(wantedUserId2, userInfoResult2.GetValue()->GetId());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Paulius.Valiunas               08/2017
//---------------------------------------------------------------------------------------
TEST_F(iModelManagerTests, QueryUsersInfoByTwoIdsTogether)
    {
    auto briefcase1 = AcquireBriefcase();
    auto wantedUserId1 = m_connection->QueryBriefcaseInfo(briefcase1->GetBriefcaseId())->GetResult().GetValue()->GetUserOwned();

    // Create new user
    auto wantedUserId2 = GetNonAdminUserId();
    auto userInfoResult = m_connection->GetUserInfoManager().QueryUsersInfoByIds(bvector<Utf8String>{wantedUserId1, wantedUserId2})->GetResult();
    EXPECT_SUCCESS(userInfoResult);

    EXPECT_EQ(2, userInfoResult.GetValue().size());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Paulius.Valiunas               08/2017
//---------------------------------------------------------------------------------------
TEST_F(iModelManagerTests, QueryUsersInfoByTwoIdsSeparately_OneAddedLater)
    {
    Utf8String userId1 = "0cdb50ea-c6c0-4790-9d31-f3395fcf6d3d";
    Utf8String userId2 = "105dcc93-e9c5-4265-8537-167caca31c98";
    Utf8String userId3 = "5a9e0150-873c-4fcd-98c0-491ba8065efb";

    std::shared_ptr<MockHttpHandler> mockHandler = std::make_shared<MockHttpHandler>();

    mockHandler->ForAnyRequest([=] (Http::RequestCR request)
        {
        Utf8String user1Request = Utf8PrintfString("UserInfo?$filter=$id+in+%%5B'%s'%%5D", userId1);

        Utf8String user2Request = Utf8PrintfString("UserInfo?$filter=$id+in+%%5B'%s'%%5D", userId2);

        Utf8String user3Request = Utf8PrintfString("UserInfo?$filter=$id+in+%%5B'%s'%%5D", userId3);

        Utf8String user1Response = Utf8PrintfString("{\"instances\":[{\"instanceId\":\"%s\",\"schemaName\":\"iModelScope\",\"className\":\"UserInfo\",\"properties\":{\"Id\":\"%s\",\"Name\":\"BistroATP\",\"Surname\":\"Regular1\",\"Email\":\"bistroatp_reg1@mailinator.com\"},\"eTag\":\"\\\"XsHulgQuqLncjk+KB+RE/c1pr0k=\\\"\"}]}", userId1, userId1);
        
        Utf8String user2Response = Utf8PrintfString("{\"instances\":[{\"instanceId\":\"%s\",\"schemaName\":\"iModelScope\",\"className\":\"UserInfo\",\"properties\":{\"Id\":\"%s\",\"Name\":\"BistroATP\",\"Surname\":\"Admin1\",\"Email\":\"bistroATP_pmadm1@mailinator.com\"},\"eTag\":\"\\\"3zHySVdWrMmn6dVylJMugn5zUB8=\\\"\"}]}", userId2, userId2);

        if (request.GetUrl().EndsWith("UserInfo"))
            return Http::Response(Http::HttpResponseContent::Create(Http::HttpStringBody::Create(user1Response)), "", Http::ConnectionStatus::OK, Http::HttpStatus::OK);
        else if (request.GetUrl().EndsWith(user1Request))
            return Http::Response(Http::HttpResponseContent::Create(Http::HttpStringBody::Create(user1Response)), "", Http::ConnectionStatus::OK, Http::HttpStatus::OK);
        else if (request.GetUrl().EndsWith(user2Request))
            return Http::Response(Http::HttpResponseContent::Create(Http::HttpStringBody::Create(user2Response)), "", Http::ConnectionStatus::OK, Http::HttpStatus::OK);
        else if (request.GetUrl().EndsWith("/v2.0/Plugins"))
            {
            auto httpResponseContent = Http::HttpResponseContent::Create(HttpStringBody::Create());
            httpResponseContent->GetHeaders().SetValue("Server", "Bentley-WebAPI/2.4, Bentley-WSG/9.99.00.00");
            return Http::Response(httpResponseContent, "", Http::ConnectionStatus::OK, Http::HttpStatus::OK);
            }
        else if (request.GetUrl().EndsWith(user3Request))
            return Http::Response(Http::HttpResponseContent::Create(Http::HttpStringBody::Create("{\"instances\":[]}")), "", Http::ConnectionStatus::OK, Http::HttpStatus::OK);
        return Http::Response(Http::HttpResponseContent::Create(HttpStringBody::Create()), "", Http::ConnectionStatus::CouldNotConnect, Http::HttpStatus::None);
        });

    // Set other wsclient
    WebServices::ClientInfoPtr clientInfo = IntegrationTestSettings::Instance().GetClientInfo();
    auto newClient = WSRepositoryClient::Create(m_imodel->GetServerURL(), m_imodel->GetWSRepositoryName(), clientInfo, nullptr, mockHandler);
    m_connection->SetRepositoryClient(newClient);

    // Should not succeed when user does not exist
    auto user3InfoResult = m_connection->GetUserInfoManager().QueryUserInfoById(userId3)->GetResult();
    EXPECT_FALSE(user3InfoResult.IsSuccess());

    // Mock Handler should return only user1 when querying first time
    auto user1InfoResult = m_connection->GetUserInfoManager().QueryUserInfoById(userId1)->GetResult();
    EXPECT_SUCCESS(user1InfoResult);
    EXPECT_EQ(userId1, user1InfoResult.GetValue()->GetId());

    // Mock Handler should return user2
    auto user2InfoResult = m_connection->GetUserInfoManager().QueryUserInfoById(userId2)->GetResult();
    EXPECT_SUCCESS(user2InfoResult);
    EXPECT_EQ(userId2, user2InfoResult.GetValue()->GetId());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Paulius.Valiunas               08/2017
//---------------------------------------------------------------------------------------
TEST_F(iModelManagerTests, QuerySameUserInfoTwice)
    {
    auto briefcase = AcquireBriefcase();
    auto userId = m_connection->QueryBriefcaseInfo(briefcase->GetBriefcaseId())->GetResult().GetValue()->GetUserOwned();

    double start1 = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    auto userInfoResult1 = m_connection->GetUserInfoManager().QueryUserInfoById(userId)->GetResult();
    EXPECT_SUCCESS(userInfoResult1);
    EXPECT_EQ(userId, userInfoResult1.GetValue()->GetId());
    double end1 = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    double duration1 = end1 - start1;

    double start2 = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    auto userInfoResult2 = m_connection->GetUserInfoManager().QueryUserInfoById(userId)->GetResult();
    EXPECT_SUCCESS(userInfoResult2);
    EXPECT_EQ(userId, userInfoResult2.GetValue()->GetId());
    double end2 = BeTimeUtilities::GetCurrentTimeAsUnixMillisDouble();
    double duration2 = end2 - start2;

    //Expect cached retrieval to be faster than 1ms
    EXPECT_LT(duration2, 1.0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Paulius.Valiunas               08/2017
//---------------------------------------------------------------------------------------
TEST_F(iModelManagerTests, QueryUsersInfoByTwoIdsTogether_OneAddedLater)
    {
    Utf8String userId1 = "8ff4834a-4754-4bbd-80b4-46770a50fcf4";
    Utf8String userId2 = "1dbbee67-196c-4c9d-96a1-ba4ef15fced8";
    Utf8String userId3 = "4fd49aef-142f-4eae-98e3-48eb0cee5cc7";

    std::shared_ptr<MockHttpHandler> mockHandler = std::make_shared<MockHttpHandler>();

    mockHandler->ForAnyRequest([=] (Http::RequestCR request)
        {
        Utf8String user1Request = Utf8PrintfString("UserInfo?$filter=$id+in+%%5B'%s'%%5D", userId1);

        Utf8String user2Request = Utf8PrintfString("UserInfo?$filter=$id+in+%%5B'%s'%%5D", userId2);

        Utf8String user23Request = Utf8PrintfString("UserInfo?$filter=$id+in+%%5B'%s','%s'%%5D", userId2, userId3);

        Utf8String user3Request = Utf8PrintfString("UserInfo?$filter=$id+in+%%5B'%s'%%5D", userId3);

        Utf8String user1Response = Utf8PrintfString("{\"instances\":[{\"instanceId\":\"%s\",\"schemaName\":\"iModelScope\",\"className\":\"UserInfo\",\"properties\":{\"Id\":\"%s\",\"Name\":\"BistroATP\",\"Surname\":\"Admin1\",\"Email\":\"bistroATP_pmadm1@mailinator.com\"},\"eTag\":\"\\\"3zHySVdWrMmn6dVylJMugn5zUB8=\\\"\"}]}", userId1, userId1);

        Utf8String user2Response = Utf8PrintfString("{\"instances\":[{\"instanceId\":\"%s\",\"schemaName\":\"iModelScope\",\"className\":\"UserInfo\",\"properties\":{\"Id\":\"%s\",\"Name\":\"BistroATP\",\"Surname\":\"Regular1\",\"Email\":\"bistroatp_reg1@mailinator.com\"},\"eTag\":\"\\\"XsHulgQuqLncjk+KB+RE/c1pr0k=\\\"\"}]}", userId2, userId2);

        if (request.GetUrl().EndsWith("UserInfo"))
            return Http::Response(Http::HttpResponseContent::Create(Http::HttpStringBody::Create(user1Response)), "", Http::ConnectionStatus::OK, Http::HttpStatus::OK);
        else if (request.GetUrl().EndsWith(user1Request))
            return Http::Response(Http::HttpResponseContent::Create(Http::HttpStringBody::Create(user1Response)), "", Http::ConnectionStatus::OK, Http::HttpStatus::OK);
        else if (request.GetUrl().EndsWith(user2Request))
            return Http::Response(Http::HttpResponseContent::Create(Http::HttpStringBody::Create(user2Response)), "", Http::ConnectionStatus::OK, Http::HttpStatus::OK);
        else if (request.GetUrl().EndsWith("/v2.0/Plugins"))
            {
            auto httpResponseContent = Http::HttpResponseContent::Create(HttpStringBody::Create());
            httpResponseContent->GetHeaders().SetValue("Server", "Bentley-WebAPI/2.4, Bentley-WSG/9.99.00.00");
            return Http::Response(httpResponseContent, "", Http::ConnectionStatus::OK, Http::HttpStatus::OK);
            }
        else if (request.GetUrl().EndsWith(user3Request))
            return Http::Response(Http::HttpResponseContent::Create(Http::HttpStringBody::Create("{\"instances\":[]}")), "", Http::ConnectionStatus::OK, Http::HttpStatus::OK);
        else if (request.GetUrl().EndsWith(user23Request))
            return Http::Response(Http::HttpResponseContent::Create(Http::HttpStringBody::Create(user2Response)), "", Http::ConnectionStatus::OK, Http::HttpStatus::OK);
        return Http::Response(Http::HttpResponseContent::Create(HttpStringBody::Create()), "", Http::ConnectionStatus::CouldNotConnect, Http::HttpStatus::None);
        });

    // Set other wsclient
    WebServices::ClientInfoPtr clientInfo = IntegrationTestSettings::Instance().GetClientInfo();
    auto newClient = WSRepositoryClient::Create(m_imodel->GetServerURL(), m_imodel->GetWSRepositoryName(), clientInfo, nullptr, mockHandler);
    m_connection->SetRepositoryClient(newClient);

    // Mock Handler should return only user1 when querying first time
    auto user1InfoResult = m_connection->GetUserInfoManager().QueryUserInfoById(userId1)->GetResult();
    EXPECT_SUCCESS(user1InfoResult);
    EXPECT_EQ(userId1, user1InfoResult.GetValue()->GetId());

    // Mock Handler should return only user1 and user2
    auto bothUsersInfoResult = m_connection->GetUserInfoManager().QueryUsersInfoByIds(bvector<Utf8String> { userId2, userId1, userId3 })->GetResult();
    EXPECT_SUCCESS(bothUsersInfoResult);

    bool found1 = false;
    bool found2 = false;
    bool foundOther = false;
    for (auto userInfo : bothUsersInfoResult.GetValue())
        {
        if (userInfo->GetId() == userId1)
            found1 = true;
        else if (userInfo->GetId() == userId2)
            found2 = true;
        else
            foundOther = true;
        }
    EXPECT_TRUE(found1);
    EXPECT_TRUE(found2);
    EXPECT_FALSE(foundOther);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Algirdas.Mikoliunas        07/2017
//---------------------------------------------------------------------------------------
TEST_F(iModelManagerTests, CodeIdsTest)
    {
    //Prapare imodel and acquire briefcases
    auto briefcase1 = AcquireBriefcase();
    DgnDbR db1 = briefcase1->GetDgnDb();

    CodeSpecPtr codeSpec1 = CodeSpec::Create(db1, "CodeSpec1");
    ASSERT_TRUE(codeSpec1.IsValid());
    db1.CodeSpecs().Insert(*codeSpec1);
    auto partition1_1 = CreateAndInsertModeledElement("Model1-1", db1);
    db1.SaveChanges();

    // Create two codes
    DgnCodeSet codeSet;
    DgnCode code1 = DgnCode(codeSpec1->GetCodeSpecId(), codeSpec1->GetScopeElementId(*partition1_1), "PMP-0010");
    codeSet.insert(code1);
    BeSQLite::BeGuid codeGuid;
    codeGuid.Create();
    DgnCode code2 = DgnCode(codeSpec1->GetCodeSpecId(), codeGuid, "PMP-0020");
    codeSet.insert(code2);

    // Reserve codes
    auto response = db1.BriefcaseManager().ReserveCodes(codeSet, IBriefcaseManager::ResponseOptions::CodeState);
    EXPECT_EQ(RepositoryStatus::Success, response.Result());

    LockableIdSet locks; 
    DgnCodeSet codes;
    codes.insert(code1);
    codes.insert(code2);
    auto result = briefcase1->GetiModelConnection().QueryCodesLocksById(codes, locks)->GetResult();
    EXPECT_SUCCESS(result);

    auto codeStatesIterator = result.GetValue().GetCodes().begin();
    auto code1Result = *codeStatesIterator;
    codeStatesIterator++;
    auto code2Result = *codeStatesIterator;

    // Check if scope requirements were properly parsed
    EXPECT_EQ(code1.GetScopeString(), code1Result.GetScopeString());
    EXPECT_EQ(code2.GetScopeString(), code2Result.GetScopeString());
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                     Viktorija.Adomauskaite           07/2017
//---------------------------------------------------------------------------------------
TEST_F(iModelManagerTests, VersionsTest)
    {
    //set up
    auto versionManager = m_connection->GetVersionsManager();

    auto nonAdminClient = SetUpClient(IntegrationTestSettings::Instance().GetValidHost(), IntegrationTestSettings::Instance().GetValidNonAdminCredentials());
    auto nonAdminConnection = ConnectToiModel(*nonAdminClient, m_imodel);
    auto nonAdminVersionManager = nonAdminConnection->GetVersionsManager();
    
    auto briefcase = AcquireBriefcase();
    auto briefcaseInfo = m_connection->QueryBriefcaseInfo(briefcase->GetBriefcaseId())->GetResult().GetValue();

    //push some ChangeSets for Versions
    CreateModel("Model1", briefcase->GetDgnDb());
    briefcase->GetDgnDb().SaveChanges();
    auto changeSet1 = PushPendingChanges(*briefcase);

    CreateModel("Model2", briefcase->GetDgnDb());
    briefcase->GetDgnDb().SaveChanges();
    auto changeSet2 = PushPendingChanges(*briefcase);

    //test create version
    EXPECT_EQ(versionManager.GetAllVersions()->GetResult().GetValue().size(), 0);
    VersionInfoPtr version1 = new VersionInfo("Version1", "Description", changeSet1);
    auto result = versionManager.CreateVersion(*version1)->GetResult();
    EXPECT_SUCCESS(result);
    version1 = result.GetValue();
    EXPECT_NE("", version1->GetId());
    EXPECT_EQ(briefcaseInfo->GetUserOwned(), version1->GetUserCreated());
    EXPECT_EQ(versionManager.GetAllVersions()->GetResult().GetValue().size(), 1);

    auto versionToFail = VersionInfo(nullptr, nullptr, changeSet2);
    EXPECT_EQ(Error::Id::UserDoesNotHavePermission, nonAdminVersionManager.CreateVersion(versionToFail)->GetResult().GetError().GetId());
    EXPECT_EQ(Error::Id::MissingRequiredProperties, versionManager.CreateVersion(versionToFail)->GetResult().GetError().GetId());
    versionToFail = VersionInfo("Version1", nullptr, changeSet1);
    EXPECT_EQ(Error::Id::ChangeSetAlreadyHasVersion,  versionManager.CreateVersion(versionToFail)->GetResult().GetError().GetId());
    versionToFail = VersionInfo("Version1", nullptr, changeSet2);
    EXPECT_EQ(Error::Id::VersionAlreadyExists,        versionManager.CreateVersion(versionToFail)->GetResult().GetError().GetId());
    versionToFail = VersionInfo("Version2", nullptr, "NotExistingChangeSet");
    EXPECT_EQ(Error::Id::ChangeSetDoesNotExist,   versionManager.CreateVersion(versionToFail)->GetResult().GetError().GetId());
    EXPECT_EQ(versionManager.GetAllVersions()->GetResult().GetValue().size(), 1);

    VersionInfoPtr version2 = new VersionInfo("Version2", nullptr, changeSet2);
    result = versionManager.CreateVersion(*version2)->GetResult();
    EXPECT_SUCCESS(result);
    version2 = result.GetValue();
    EXPECT_EQ(versionManager.GetAllVersions()->GetResult().GetValue().size(), 2);

    //test update version
    version1->SetName("NewName");
    EXPECT_SUCCESS(versionManager.UpdateVersion(*version1)->GetResult());
    auto queryResult = versionManager.GetVersionById(version1->GetId())->GetResult();
    EXPECT_SUCCESS(queryResult);
    version1 = queryResult.GetValue();
    EXPECT_EQ("NewName", version1->GetName());
    EXPECT_EQ(changeSet1, version1->GetChangeSetId());
    EXPECT_EQ("Description", version1->GetDescription());
    EXPECT_EQ(briefcaseInfo->GetUserOwned(), version1->GetUserCreated());

    versionToFail = VersionInfo(*version2);
    EXPECT_EQ(Error::Id::UserDoesNotHavePermission, nonAdminVersionManager.UpdateVersion(versionToFail)->GetResult().GetError().GetId());
    versionToFail.SetName("NewName");
    EXPECT_EQ(Error::Id::VersionAlreadyExists, versionManager.UpdateVersion(versionToFail)->GetResult().GetError().GetId());
    versionToFail.SetName("");
    EXPECT_EQ(Error::Id::MissingRequiredProperties, versionManager.UpdateVersion(versionToFail)->GetResult().GetError().GetId());

    version2->SetDescription("NewDescription");
    EXPECT_SUCCESS(versionManager.UpdateVersion(*version2)->GetResult());
    queryResult = nonAdminVersionManager.GetVersionById(version2->GetId())->GetResult();
    EXPECT_SUCCESS(queryResult);
    version2 = queryResult.GetValue();
    EXPECT_EQ("Version2", version2->GetName());
    EXPECT_EQ(changeSet2, version2->GetChangeSetId());
    EXPECT_EQ("NewDescription", version2->GetDescription());
    EXPECT_EQ(versionManager.GetAllVersions()->GetResult().GetValue().size(), 2);
    }

TEST_F(iModelManagerTests, GetChangeSetsRelatedToVersions)
    {
    auto briefcase = AcquireBriefcase();
    auto briefcaseInfo = m_connection->QueryBriefcaseInfo(briefcase->GetBriefcaseId())->GetResult().GetValue();

    //push some ChangeSets for Versions
    CreateModel("Model1", briefcase->GetDgnDb());
    briefcase->GetDgnDb().SaveChanges();
    auto changeSet1 = PushPendingChanges(*briefcase);

    CreateModel("Model2", briefcase->GetDgnDb());
    briefcase->GetDgnDb().SaveChanges();
    auto changeSet2 = PushPendingChanges(*briefcase);

    CreateModel("Model3", briefcase->GetDgnDb());
    briefcase->GetDgnDb().SaveChanges();
    auto changeSet3 = PushPendingChanges(*briefcase);

    auto versionManager = m_connection->GetVersionsManager();
    VersionInfoPtr version1 = new VersionInfo("Version1", "Description", changeSet1);
    version1 = versionManager.CreateVersion(*version1)->GetResult().GetValue();
    VersionInfoPtr version2 = new VersionInfo("Version2", "Description", changeSet2);
    version2 = versionManager.CreateVersion(*version2)->GetResult().GetValue();
    VersionInfoPtr version3 = new VersionInfo("Version3", "Description", changeSet3);
    version3 = versionManager.CreateVersion(*version3)->GetResult().GetValue();

    auto changeSetsResult = versionManager.GetVersionChangeSets(version1->GetId(), briefcase->GetDgnDb().GetDbGuid())->GetResult();
    EXPECT_SUCCESS(changeSetsResult);
    EXPECT_EQ(1, changeSetsResult.GetValue().size());
    EXPECT_EQ(changeSet1, changeSetsResult.GetValue().at(0)->GetId());

    changeSetsResult = versionManager.GetVersionChangeSets(version2->GetId())->GetResult();
    EXPECT_SUCCESS(changeSetsResult);
    EXPECT_EQ(2, changeSetsResult.GetValue().size());
    EXPECT_EQ(changeSet1, changeSetsResult.GetValue().at(0)->GetId());
    EXPECT_EQ(changeSet2, changeSetsResult.GetValue().at(1)->GetId());
    EXPECT_EQ(Error::Id::InvalidVersion, versionManager.GetVersionChangeSets("", briefcase->GetDgnDb().GetDbGuid())->GetResult().GetError().GetId());

    changeSetsResult = versionManager.GetChangeSetsBetweenVersions(version1->GetId(), version3->GetId(), briefcase->GetDgnDb().GetDbGuid())->GetResult();
    EXPECT_SUCCESS(changeSetsResult);
    EXPECT_EQ(2, changeSetsResult.GetValue().size());
    EXPECT_EQ(changeSet2, changeSetsResult.GetValue().at(0)->GetId());
    EXPECT_EQ(changeSet3, changeSetsResult.GetValue().at(1)->GetId());

    changeSetsResult = versionManager.GetChangeSetsBetweenVersions(version3->GetId(), version1->GetId())->GetResult();
    EXPECT_SUCCESS(changeSetsResult);
    EXPECT_EQ(2, changeSetsResult.GetValue().size());
    EXPECT_EQ(changeSet2, changeSetsResult.GetValue().at(0)->GetId());
    EXPECT_EQ(changeSet3, changeSetsResult.GetValue().at(1)->GetId());
    EXPECT_EQ(Error::Id::InvalidVersion, versionManager.GetChangeSetsBetweenVersions(version1->GetId(), "", briefcase->GetDgnDb().GetDbGuid())->GetResult().GetError().GetId());
    EXPECT_EQ(Error::Id::InvalidVersion, versionManager.GetChangeSetsBetweenVersions("", version1->GetId(), briefcase->GetDgnDb().GetDbGuid())->GetResult().GetError().GetId());

    changeSetsResult = versionManager.GetChangeSetsAfterVersion(version1->GetId())->GetResult();
    EXPECT_SUCCESS(changeSetsResult);
    EXPECT_EQ(2, changeSetsResult.GetValue().size());
    EXPECT_EQ(changeSet2, changeSetsResult.GetValue().at(0)->GetId());
    EXPECT_EQ(changeSet3, changeSetsResult.GetValue().at(1)->GetId());

    changeSetsResult = versionManager.GetChangeSetsAfterVersion(version2->GetId(), briefcase->GetDgnDb().GetDbGuid())->GetResult();
    EXPECT_SUCCESS(changeSetsResult);
    EXPECT_EQ(1, changeSetsResult.GetValue().size());
    EXPECT_EQ(changeSet3, changeSetsResult.GetValue().at(0)->GetId());
    EXPECT_EQ(Error::Id::InvalidVersion, versionManager.GetChangeSetsAfterVersion("")->GetResult().GetError().GetId());

    changeSetsResult = versionManager.GetChangeSetsBetweenVersionAndChangeSet(version1->GetId(), changeSet3)->GetResult();
    EXPECT_SUCCESS(changeSetsResult);
    EXPECT_EQ(2, changeSetsResult.GetValue().size());
    EXPECT_EQ(changeSet2, changeSetsResult.GetValue().at(0)->GetId());
    EXPECT_EQ(changeSet3, changeSetsResult.GetValue().at(1)->GetId());

    changeSetsResult = versionManager.GetChangeSetsBetweenVersionAndChangeSet(version3->GetId(), changeSet1, briefcase->GetDgnDb().GetDbGuid())->GetResult();
    EXPECT_SUCCESS(changeSetsResult);
    EXPECT_EQ(2, changeSetsResult.GetValue().size());
    EXPECT_EQ(changeSet2, changeSetsResult.GetValue().at(0)->GetId());
    EXPECT_EQ(changeSet3, changeSetsResult.GetValue().at(1)->GetId());
    EXPECT_EQ(Error::Id::InvalidVersion, versionManager.GetChangeSetsBetweenVersionAndChangeSet("", changeSet1)->GetResult().GetError().GetId());


    changeSetsResult =  versionManager.GetChangeSetsBetweenVersionAndChangeSet(version2->GetId(), "")->GetResult();
    EXPECT_SUCCESS(changeSetsResult);
    EXPECT_EQ(2, changeSetsResult.GetValue().size());
    EXPECT_EQ(changeSet1, changeSetsResult.GetValue().at(0)->GetId());
    EXPECT_EQ(changeSet2, changeSetsResult.GetValue().at(1)->GetId());
    }