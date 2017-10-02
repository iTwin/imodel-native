/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/iModelHubClient/Integration/IntegrationTestsBase.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "IntegrationTestsHelper.h"
#include <Bentley/BeTest.h>
#include "../BackDoor/PublicAPI/BackDoor/WebServices/iModelHub/BackDoor.h"
#include <WebServices/iModelHub/Client/iModelInfo.h>
#include <WebServices/iModelHub/Client/Client.h>
#include <WebServices/Client/ClientInfo.h>
#include <BeHttp/Credentials.h>
#include <BeHttp/ProxyHttpHandler.h>

BEGIN_BENTLEY_IMODELHUB_UNITTESTS_NAMESPACE
USING_NAMESPACE_BENTLEY_IMODELHUB
struct IntegrationTestsBase : public ::testing::Test
    {
private:
    BeFileName m_seed;
    void CreateInitialSeedDb();
    void InitLogging();
    void Initialize(ScopediModelHubHost* host);
protected:
    static double s_lastProgressBytesTransfered;
    static double s_lastProgressBytesTotal;
    static int    s_progressRetryCount;
    ScopediModelHubHost *m_pHost;
    Utf8String m_projectId;

    Request::ProgressCallback CreateProgressCallback();
    void CheckProgressNotified();
    void CheckNoProgress();
public:
    virtual void SetUp() override;
    virtual void TearDown() override;
    static void SetUpTestCase();
    static void TearDownTestCase();

    BeFileName LocalPath(Utf8StringCR baseName);
    
    DgnDbPtr CreateTestDb(Utf8StringCR baseName);
    ClientPtr SetUpClient(Credentials credentials, IHttpHandlerPtr customHandler = nullptr);
    iModelInfoPtr CreateNewiModel(ClientCR client, Utf8StringCR imodelName);
    iModelInfoPtr CreateNewiModelFromDb(ClientCR client, DgnDbR db);
    iModelConnectionPtr ConnectToiModel(ClientCR client, iModelInfoPtr imodelInfo);
    void DeleteiModel(Utf8StringCR projectId, ClientCR client, iModelInfoCR imodel);
    BriefcasePtr AcquireBriefcase(ClientCR client, iModelInfoCR imodelInfo, bool pull = false);
    void InitializeWithChangeSets(ClientCR client, iModelInfoCR imodel, uint32_t changeSetCount);
    BeSQLite::BeGuid ReplaceSeedFile(iModelConnectionPtr imodelConnection, DgnDbR db, bool changeGuid = true);
    Utf8String PushPendingChanges(Briefcase& briefcase, bool relinquishLocksCodes = false);

    void ExpectLocksCount(Briefcase& briefcase, int expectedCount);
    void ExpectCodesEqual(DgnCodeStateCR exp, DgnCodeStateCR act);
    void ExpectCodesEqual(DgnCodeInfoCR exp, DgnCodeInfoCR act);
    void ExpectCodesEqual(DgnCodeInfoSet const& expected, DgnCodeInfoSet const& actual);
    void ExpectCodeState(DgnCodeInfoSet const& expect, IRepositoryManagerP imodelManager);
    void ExpectCodeState(DgnCodeInfoCR expect, IRepositoryManagerP imodelManager);
    DgnCodeInfo CreateCodeUsed(DgnCodeCR code, Utf8StringCR changeSetId);
    DgnCode MakeModelCode(Utf8CP name, DgnDbR db);
    };
END_BENTLEY_IMODELHUB_UNITTESTS_NAMESPACE
