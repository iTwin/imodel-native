/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/MstnBridgeTestsFixture.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <Bentley/BeTest.h>
#include <BeSQLite/BeSQLite.h>
#include <DgnPlatform/DgnDb.h>
#include <UnitTests/BackDoor/DgnPlatform/ScopedDgnHost.h>
#include <iModelBridge/IModelClientForBridges.h>
#include <iModelBridge/TestIModelHubClientForBridges.h>
#include <WebServices/iModelHub/Client/Client.h>

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

//=======================================================================================
// @bsistruct                              
//=======================================================================================
struct MstnBridgeTestsFixture : ::testing::Test
    {
    protected:
    BentleyApi::BeFileName m_briefcaseName;
    BentleyApi::Dgn::IModelClientForBridges* m_client;
    void SetupClient(); // Called by SetUpTestCase. Sets the above member variables, based on a command-line argument
    void CreateMockClient();
    void CreateIModelHubClient();
    void CreateIModelBankClient(BentleyApi::Utf8StringCR accessToken);

    void StartImodelBankServer(BentleyApi::BeFileNameCR imodelDir);
    void StopImodelBankServer();

    std::tuple<void*, void*> StartIModelBridgeFwkExe(int argc, wchar_t const** argv, WCharCP testName);
    int WaitForIModelBridgeFwkExe(std::tuple<void*, void*> const&);
        
    static BentleyApi::WebServices::ClientInfoPtr GetClientInfo();
    static BentleyApi::BeFileName CreateImodelBankRepository(BentleyApi::BeFileNameCR seedFile);
    static BentleyApi::BeFileName CreateTestDir(WCharCP testDir = nullptr);

    BentleyApi::Dgn::IModelClientForBridges& GetClient() {return *m_client;}

    BentleyApi::Dgn::IModelHubClientForBridges* GetClientAsIModelHubClientForBridges() {return dynamic_cast<BentleyApi::Dgn::IModelHubClientForBridges*>(m_client);}
    BentleyApi::Dgn::IModelClientBase* GetClientAsIModelClientBase() {return dynamic_cast<BentleyApi::Dgn::IModelClientBase*>(m_client);}
    BentleyApi::Dgn::IModelBankClient* GetClientAsIModelBank() {return dynamic_cast<BentleyApi::Dgn::IModelBankClient*>(m_client);}
    BentleyApi::Dgn::IModelHubClient*  GetClientAsIModelHub()  {return dynamic_cast<BentleyApi::Dgn::IModelHubClient*>(m_client);}
    BentleyApi::Dgn::TestIModelHubClientForBridges* GetClientAsMock() {return dynamic_cast<BentleyApi::Dgn::TestIModelHubClientForBridges*>(m_client);}

    bool UsingIModelBank() {return nullptr != GetClientAsIModelBank();}
    bool UsingIModelHub()  {return nullptr != GetClientAsIModelHub();}
    bool UsingMockServer() {return nullptr != GetClientAsMock();}

    RevisionStats ComputeRevisionStats(BentleyApi::Dgn::DgnDbR db, size_t start = 0, size_t end = -1);

    size_t GetChangesetCount();

    void MstnBridgeTestsFixture::CreateRepository(Utf8CP repoName = nullptr);

    static BentleyApi::BeFileName GetTestDataDir();

    static BentleyApi::BeFileName GetTestDataFileName(WCharCP baseName) { auto fn = GetTestDataDir(); fn.AppendToPath(baseName); return fn; }

    static BentleyApi::BeFileName WriteRulesFile(WCharCP fname, Utf8CP rules);
    static BentleyApi::Utf8String ComputeAccessToken(Utf8CP uname);
    static BentleyApi::WString ComputeAccessTokenW(Utf8CP uname) {auto atok = ComputeAccessToken(uname); return BentleyApi::WString(atok.c_str(), true);}

    MstnBridgeTestsFixture() : m_client(nullptr) { SetupClient(); }
    ~MstnBridgeTestsFixture();

    public:
    static BentleyApi::BeFileName GetOutputDir();

    static BentleyApi::BeFileName  GetOutputFileName(BentleyApi::WCharCP filename);

    static BentleyApi::BeFileName GetSeedFilePath() { auto path = GetOutputDir(); path.AppendToPath(L"seed.bim"); return path; }

    static BentleyApi::BeFileName GetDgnv8BridgeDllName();

    static BentleyApi::BeFileName GetSeedFile();

    static void SetUpTestCase();
    
    static void TearDownTestCase();

    void MakeCopyOfFile(BentleyApi::BeFileNameR outFile, BentleyApi::WCharCP filename, BentleyApi::WCharCP suffix);

    void SetUpBridgeProcessingArgs(BentleyApi::bvector<BentleyApi::WString>& args, WCharCP stagingDir, WCharCP bridgeRegSubkey, WCharCP iModelName = nullptr);

    void AddAttachment(BentleyApi::BeFileName& inputFile, BentleyApi::BeFileNameR refV8File, int32_t num, bool useOffsetForElement);

    int64_t AddLine(BentleyApi::BeFileName& inputFile, int count = 1);

    struct DbFileInfo
        {
        BentleyApi::Dgn::DgnDbPtr m_db;
        BentleyApi::Dgn::ScopedDgnHost m_host;
        DbFileInfo(BentleyApi::BeFileNameCR fileName);
        int32_t GetElementCount();
        int32_t GetModelCount();
        int32_t GetPhysicalModelCount();
        int32_t GetBISClassCount(CharCP className);
        int32_t GetModelProvenanceCount(BentleyApi::BeSQLite::BeGuidCR fileGuid);
        BentleyApi::BentleyStatus GetiModelElementByDgnElementId(BentleyApi::Dgn::DgnElementId& elementId, int64_t srcElementId);
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

    void ReplaceArgValue(WCharCP argName, WCharCP newValue);

    void SetInputFileArg(BentleyApi::BeFileNameCR fn) {PushArg(BentleyApi::WPrintfString(L"--fwk-input=\"%ls\"", fn.c_str()));}
    void SetSkipAssignmentCheck() {PushArg(L"--fwk-skip-assignment-check");}

    BentleyApi::bvector<WCharCP> const& GetUnparsedArgs() {return m_bargptrs;}
    };

//=======================================================================================
// @bsistruct                              
//=======================================================================================
struct SynchInfoTests : public MstnBridgeTestsFixture, public ::testing::WithParamInterface<WString>
{
    int64_t AddModel (BentleyApi::BeFileName& inputFile, BentleyApi::Utf8StringCR modelName);
    int64_t AddNamedView (BentleyApi::BeFileName& inputFile, BentleyApi::Utf8StringCR viewName);
    int64_t AddLevel (BentleyApi::BeFileName& inputFile, BentleyApi::Utf8StringCR levelName);

    void ValidateNamedViewSynchInfo (BentleyApi::BeFileName& dbFile, int64_t srcId);
    void ValidateElementSynchInfo (BentleyApi::BeFileName& dbFile, int64_t srcId);
    void ValidateModelSynchInfo (BentleyApi::BeFileName& dbFile, int64_t srcId);
    void ValidateLevelSynchInfo (BentleyApi::BeFileName& dbFile, int64_t srcId);
};

