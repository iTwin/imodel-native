/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <Bentley/BeTest.h>
#include <BeSQLite/BeSQLite.h>
#include <DgnPlatform/DgnDb.h>
#include <UnitTests/BackDoor/DgnPlatform/ScopedDgnHost.h>
#include <iModelBridge/IModelClientForBridges.h>
#include <iModelBridge/TestIModelHubClientForBridges.h>
#include <WebServices/iModelHub/Client/Client.h>
#include "DwgBridgeTestsLogProvider.h"


enum class CodeScope
    {
    Model,
    Related,
    Repository
    };

struct RevisionStats
    {
    size_t nSchemaRevs {};
    size_t nDataRevs {};
    BentleyApi::bset<BentleyApi::Utf8String> descriptions;
    BentleyApi::bset<BentleyApi::Utf8String> userids;
    };

#define DEFAULT_IMODEL_NAME  L"iModelBridgeTests_Test1"
#define DEFAULT_IMODEL_NAME_A "iModelBridgeTests_Test1"

#define MAKE_ARGC_ARGV(argptrs, args)\
for (auto& arg: args)\
   argptrs.push_back(arg.c_str());\
int argc = (int)argptrs.size();\
wchar_t const** argv = argptrs.data();\

//=======================================================================================
// @bsistruct                              
//=======================================================================================
struct ScopedEnvvar
    {
    Utf8String m_var;
    ScopedEnvvar(BentleyApi::Utf8CP var, BentleyApi::Utf8StringCR value) : m_var(var)
        {
        putenv(BentleyApi::Utf8PrintfString("%s=%s", var, value.c_str()).c_str());
        }
    ~ScopedEnvvar()
        {
        putenv(BentleyApi::Utf8PrintfString("%s=", m_var.c_str()).c_str());
        }
    };

//=======================================================================================
// @bsistruct                              
//=======================================================================================
struct SetRulesFileInEnv : ScopedEnvvar
    {
    SetRulesFileInEnv(BentleyApi::BeFileNameCR fn) : ScopedEnvvar("IMODEL-BANK-RULES-FILE", BentleyApi::Utf8String(fn)) {}
    };

struct ProcessRunner;
typedef void (*T_SendKillSignal)(ProcessRunner&);

//=======================================================================================
// @bsistruct                              
//=======================================================================================
struct ProcessRunner
    {
    private:
    BentleyApi::WString m_cmd;
    int m_argc {};
    wchar_t const** m_argv {};
    void* m_hProcess {};
    void* m_hThread {};
    int m_pid {};
    int m_exitCode {};
    BentleyApi::BeFileName m_rspFile {};
    T_SendKillSignal m_sendKillSignal {};
    BentleyApi::Dgn::IShutdownIModelServer* m_shutdown {};
    BentleyApi::WString m_cmdline;

    public:
    ProcessRunner() {}
    ProcessRunner(BentleyApi::WStringCR cmd, int argc, wchar_t const** argv, BentleyApi::BeFileNameCR rspFile, T_SendKillSignal, BentleyApi::Dgn::IShutdownIModelServer*);
    ProcessRunner(ProcessRunner&&);
    ProcessRunner(ProcessRunner const&) = delete;
    ~ProcessRunner();

    void SetCmd(BentleyApi::WStringCR c) { m_cmd = c; }
    void SetArgs(int c, wchar_t const** v) { m_argc=c; m_argv=v; }
    void SetUseRspFile(BentleyApi::BeFileNameCR rspFile) {m_rspFile=rspFile;}

    BentleyApi::BentleyStatus Start(size_t waitMs);
    BentleyApi::BentleyStatus Stop(size_t waitMs);

    static int FindProcessId(BentleyApi::Utf8CP name);

    static void DoSleep(size_t ms);

    int GetExitCode() const {return m_exitCode;}
    };

//=======================================================================================
// @bsistruct                              
//=======================================================================================
struct BriefClientRepositoryAdmin : BentleyApi::Dgn::DgnPlatformLib::Host::RepositoryAdmin
{
    DEFINE_T_SUPER(RepositoryAdmin);

    BentleyApi::Dgn::IModelClientForBridges& m_client;

    BriefClientRepositoryAdmin(BentleyApi::Dgn::IModelClientForBridges& client) : m_client(client) {}
    BentleyApi::Dgn::IRepositoryManagerP _GetRepositoryManager(BentleyApi::Dgn::DgnDbR db) const override
        {
        return m_client.GetRepositoryManager(db);
        }
};

struct LogProcessor;
struct FwkArgvMaker;
//=======================================================================================
// @bsistruct                              
//=======================================================================================
struct DwgBridgeTestsFixture : ::testing::Test
    {
    friend struct LogProcessor;
    protected:
    static DwgBridgeTestsLogProvider s_logProvider;
    static WString  s_dwgbridgeRegSubKey;
    BentleyApi::BeFileName m_briefcaseName;
    BentleyApi::Dgn::IModelClientForBridges* m_client {};
    BentleyApi::BeFileName GetIModelParentDir();
    BentleyApi::BeFileName GetIModelDir();

    ProcessRunner StartImodelBankServer(BentleyApi::BeFileNameCR imodelDir, BentleyApi::Dgn::IModelBankClient& bankClient);
    ProcessRunner StartImodelBridgeFwkExe(FwkArgvMaker const&, size_t msWaitForStart = 0);

    BentleyApi::WebServices::ClientInfoPtr GetClientInfo();
    void CreateImodelBankRepository(BentleyApi::BeFileNameCR seedFile);
    static BentleyApi::BeFileName CreateTestDir(WCharCP testDir = nullptr);

    BentleyApi::Dgn::IModelClientForBridges& GetClient() {return *m_client;}

    void SetupClient(BentleyApi::Utf8StringCR accessToken = ""); // Calls one of the create functions below and then sets m_client to the result

    BentleyApi::Dgn::TestIModelHubClientForBridges* CreateMockClient();
    BentleyApi::Dgn::IModelHubClient* CreateIModelHubClient(BentleyApi::Utf8StringCR accessToken);
    BentleyApi::Dgn::IModelBankClient* CreateIModelBankClient(BentleyApi::Utf8StringCR accessToken);

    BentleyApi::Dgn::IModelHubClientForBridges* GetClientAsIModelHubClientForBridges() {return dynamic_cast<BentleyApi::Dgn::IModelHubClientForBridges*>(m_client);}
    BentleyApi::Dgn::IModelClientBase* GetClientAsIModelClientBase() {return dynamic_cast<BentleyApi::Dgn::IModelClientBase*>(m_client);}
    BentleyApi::Dgn::IModelBankClient* GetClientAsIModelBank() {return dynamic_cast<BentleyApi::Dgn::IModelBankClient*>(m_client);}
    BentleyApi::Dgn::IModelHubClient*  GetClientAsIModelHub()  {return dynamic_cast<BentleyApi::Dgn::IModelHubClient*>(m_client);}
    BentleyApi::Dgn::TestIModelHubClientForBridges* GetClientAsMock() {return dynamic_cast<BentleyApi::Dgn::TestIModelHubClientForBridges*>(m_client);}

    static Utf8CP GetIModelBankServerJs() {return getenv("DWGBRIDGE_TESTS_IMODEL_BANK_SERVER_JS");}

    RevisionStats ComputeRevisionStats(BentleyApi::Dgn::DgnDbR db, size_t start = 0, size_t end = -1);

    size_t GetChangesetCount();

    void CreateRepository(Utf8CP repoName = nullptr);
    ProcessRunner StartServer();

    static BentleyApi::BeFileName GetTestDataDir();

    static BentleyApi::BeFileName GetTestDataFileName(WCharCP baseName) { auto fn = GetTestDataDir(); fn.AppendToPath(baseName); return fn; }

    static BentleyApi::BeFileName WriteRulesFile(WCharCP fname, Utf8CP rules);
    static BentleyApi::Utf8String ComputeAccessToken(Utf8CP uname);
    static BentleyApi::WString ComputeAccessTokenW(Utf8CP uname) {auto atok = ComputeAccessToken(uname); return BentleyApi::WString(atok.c_str(), true);}

    DwgBridgeTestsFixture() : m_client(nullptr) {}
    ~DwgBridgeTestsFixture();

    public:
    static BentleyApi::BeFileName GetOutputDir();
    static BentleyApi::BeFileName GetOutputFileName(BentleyApi::WCharCP filename);
    static BentleyApi::BeFileName GetSeedFilePath() { auto path = GetOutputDir(); path.AppendToPath(L"seed.bim"); return path; }
    static BentleyApi::BeFileName GetDwgBridgeDllName();
    static WCharCP GetDwgBridgeRegSubKey();
    static BentleyApi::BeFileName GetSeedFile();
    static void SetUpTestCase();
    static void TearDownTestCase();

    void MakeCopyOfFile(BentleyApi::BeFileNameR outFile, BentleyApi::WCharCP filename, BentleyApi::WCharCP suffix);
    void SetUpBridgeProcessingArgs(BentleyApi::bvector<BentleyApi::WString>& args, WCharCP stagingDir, WCharCP bridgeRegSubkey, WCharCP iModelName = nullptr);
    void AddAttachment(BentleyApi::BeFileName& inputFile, BentleyApi::BeFileNameR refFile, int32_t num, bool useOffsetForElement);
    uint64_t AddLine(BentleyApi::BeFileName& inputFile, int count = 1);
    uint64_t GetDefaultDwgModelId(BentleyApi::BeFileNameCR inputFile);

    struct DbFileInfo
        {
        BentleyApi::Dgn::ScopedDgnHost m_host;
        BentleyApi::Dgn::DgnDbPtr m_db;
        
        DbFileInfo(BentleyApi::BeFileNameCR fileName);
        ~DbFileInfo();
        int32_t GetElementCount();
        int32_t GetModelCount();
        int32_t GetPhysicalModelCount();
        int32_t GetBISClassCount(CharCP className);
        int32_t GetModelProvenanceCount(BentleyApi::BeSQLite::BeGuidCR fileGuid);
        BentleyApi::BentleyStatus GetiModelElementByDgnElementId(BentleyApi::Dgn::DgnElementId& elementId, uint64_t srcElementId);
        BentleyApi::Dgn::DgnElementId GetRepositoryLinkByFileNameLike(BentleyApi::Utf8StringCR);
        void MustFindFileByName(BentleyApi::Dgn::RepositoryLinkId& fileid, BentleyApi::BeFileNameCR fileNameIn, int expectedCount);
        void MustFindModelByDwgModelId(BentleyApi::Dgn::DgnModelId& fmid, BentleyApi::Dgn::RepositoryLinkId ffid, uint64_t dwgHandle, int expectedCount);
        void MustFindElementByDwgModelId(BentleyApi::Dgn::DgnElementId& eid, BentleyApi::Dgn::DgnModelId fmid, uint64_t dwgHandle, int expectedCount);
        BentleyApi::Dgn::SubjectCPtr GetFirstJobSubject();
        void SetRepositoryAdminFromBriefcaseClient(BentleyApi::Dgn::IModelClientForBridges&);
        void ClearRepositoryAdmin();
        BentleyApi::Dgn::DgnCodeInfoSet GetCodeInfos(BentleyApi::Dgn::DgnCodeSet const&, BentleyApi::Dgn::IModelClientForBridges&);
        BentleyApi::BentleyStatus GetCodeInfo(BentleyApi::Dgn::DgnCodeInfo&, BentleyApi::Dgn::DgnCode const&, BentleyApi::Dgn::IModelClientForBridges&);
        BentleyApi::Dgn::LockLevel QueryLockLevel(BentleyApi::Dgn::LockableId, BentleyApi::Dgn::IModelClientForBridges&);
        BentleyApi::Dgn::LockLevel QueryLockLevel(BentleyApi::Dgn::DgnModelCR model, BentleyApi::Dgn::IModelClientForBridges& client) {return QueryLockLevel(BentleyApi::Dgn::LockableId(model), client);}
        BentleyApi::Dgn::LockLevel QueryLockLevel(BentleyApi::Dgn::DgnElementCR el, BentleyApi::Dgn::IModelClientForBridges& client) {return QueryLockLevel(BentleyApi::Dgn::LockableId(el), client);}
        };

    void RunTheBridge(BentleyApi::bvector<BentleyApi::WString> const& args);
    
    static void TerminateHost();

    static void SetupTestDirectory(BentleyApi::BeFileNameR dirPath,  BentleyApi::WCharCP dirName, BentleyApi::WCharCP iModelName,
                                   BentleyApi::BeFileNameCR input1, BentleyApi::BeSQLite::BeGuidCR inputGuid, 
                                   BentleyApi::BeFileNameCR refFile, BentleyApi::BeSQLite::BeGuidCR refGuid);
    };

//=======================================================================================
// @bsistruct                              
//=======================================================================================
struct LogProcessor
    {
    std::function<DwgBridgeTestsLogProvider::T_IsSeverityEnabled> m_wasSev;
    std::function<DwgBridgeTestsLogProvider::T_LogMessage> m_wasProc;
    
    LogProcessor(std::function<DwgBridgeTestsLogProvider::T_IsSeverityEnabled> sevf, std::function<DwgBridgeTestsLogProvider::T_LogMessage> logf)
        {
        m_wasSev = DwgBridgeTestsFixture::s_logProvider.m_sev;
        DwgBridgeTestsFixture::s_logProvider.m_sev = sevf;
        m_wasProc = DwgBridgeTestsFixture::s_logProvider.m_proc;
        DwgBridgeTestsFixture::s_logProvider.m_proc = logf;
        }

    ~LogProcessor()
        {
        DwgBridgeTestsFixture::s_logProvider.m_sev = m_wasSev;
        DwgBridgeTestsFixture::s_logProvider.m_proc = m_wasProc;
        }
    };

//=======================================================================================
// @bsistruct                              
//=======================================================================================
struct FwkArgvMaker
    {
    private:
    BentleyApi::bvector<BentleyApi::WCharCP> m_ptrs;
    public:
    BentleyApi::bvector<WCharCP> m_bargptrs;

    public:
    FwkArgvMaker();
    ~FwkArgvMaker();

    void Clear();

    BentleyApi::BentleyStatus ParseArgsFromRspFile(BentleyApi::WStringCR wargs);
    static BentleyApi::WString ReadRspFileFromTestData(BentleyApi::WCharCP rspFileBaseName);
    void SetUpBridgeProcessingArgs(WCharCP stagingDir, WCharCP bridgeRegSubkey, BentleyApi::BeFileNameCR bridgeDllName, WCharCP iModelName, bool useBank, WCharCP rspFileName);
    void PushArg(BentleyApi::WStringCR);

    wchar_t const** GetArgV() const {return const_cast<wchar_t const**>(m_ptrs.data());}
    int GetArgC() const {return (int)m_ptrs.size();}

    static void ReplaceArgValue(BentleyApi::bvector<BentleyApi::WString>& args, WCharCP argName, WCharCP newValue);
    static void ReplaceArgValue(BentleyApi::bvector<BentleyApi::WCharCP>& ptrs, WCharCP argName, WCharCP newValue);
    void ReplaceArgValue(WCharCP argName, WCharCP newValue);

    void SetInputFileArg(BentleyApi::BeFileNameCR fn) {PushArg(BentleyApi::WPrintfString(L"--fwk-input=\"%ls\"", fn.c_str()));}
    void SetSkipAssignmentCheck() {PushArg(L"--fwk-skip-assignment-check");}
    void SetMaxRetries(int n, bool useIModelBank) {PushArg(BentleyApi::WPrintfString(useIModelBank? L"--imodel-bank-retries=%d": L"--server-retries=%d", std::max(255, n)));}

    BentleyApi::bvector<WCharCP> const& GetUnparsedArgs() {return m_bargptrs;}
    };

//=======================================================================================
// @bsistruct                              
//=======================================================================================
struct ExternalSourceAspectTests : DwgBridgeTestsFixture, ::testing::WithParamInterface<WString>
{
    uint64_t AddModel (BentleyApi::BeFileName& inputFile, BentleyApi::Utf8StringCR modelName);
    uint64_t AddLayer (BentleyApi::BeFileName& inputFile, BentleyApi::Utf8StringCR layerName);

    void ValidateElementAspect (BentleyApi::BeFileName& dbFile, uint64_t sourceId);
    void ValidateModelAspect (BentleyApi::BeFileName& dbFile, uint64_t sourceId);
    void ValidateLayerAspect (BentleyApi::BeFileName& dbFile, uint64_t sourceId);
};

