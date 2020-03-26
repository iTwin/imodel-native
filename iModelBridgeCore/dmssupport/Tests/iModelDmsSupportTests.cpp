/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include "iModelDmsSupportTests.h"
#include <Bentley/Desktop/FileSystem.h>

int iModelDmsSupportTests::s_argc;
char **iModelDmsSupportTests::s_argv;
MockDmsClient* iModelDmsSupportTests::m_dmsClient;
DmsHelper* iModelDmsSupportTests::m_dmsHelper;
MockAzureBlobStorageHelper* iModelDmsSupportTests::m_AZHelper;
MockOidcTokenProvider* iModelDmsSupportTests::m_oidcTProvider;
iModelBridgeFwk::DmsServerArgs iModelDmsSupportTests::m_dmsServerArgs;
iModelBridgeFwk::IModelHubArgs iModelDmsSupportTests::m_iModelHubArgs;


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Vishal.Shingare                  02/2020
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyApi::BeFileName iModelDmsSupportTests::GetOutputDir()
    {
    BeFileName outDirectory = Desktop::FileSystem::GetExecutableDir();

    if (outDirectory.DoesPathExist())
        outDirectory.AppendToPath(WString(L"TestData/Stagingdir").c_str());

    return outDirectory;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Vishal.Shingare                  02/2020
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyApi::BeFileName iModelDmsSupportTests::CreateTestDir(WCharCP testDir)
    {
    // Start with a COMPLETELY CLEAN SLATE
    BentleyApi::BeFileName::EmptyAndRemoveDirectory(GetOutputDir().c_str());
    EXPECT_EQ(BentleyApi::BeFileNameStatus::Success, BentleyApi::BeFileName::CreateNewDirectory(GetOutputDir().c_str()));

    if (testDir == nullptr)
        return GetOutputDir();

    // If the test wants to run in a subdirectory, make it.
    BentleyApi::BeFileName testDirPath(testDir);
    if (!testDirPath.StartsWith(GetOutputDir()))
        {
        BeAssert(!testDirPath.IsAbsolutePath() && "Test directories must be under the output directory");
        testDirPath = GetOutputDir();
        testDirPath.AppendToPath(testDir);
        }

    BeAssert(testDirPath.IsAbsolutePath() && testDirPath.StartsWith(GetOutputDir()) && "Test directories must be under the output directory");

    EXPECT_EQ(BentleyApi::BeFileNameStatus::Success, BentleyApi::BeFileName::CreateNewDirectory(testDirPath.c_str()));

    return testDirPath;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Vishal.Shingare                  02/2020
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelDmsSupportTests::SetUp()
    {
    //inject dependency for each test
    m_dmsClient = new MockDmsClient();
    m_AZHelper = new MockAzureBlobStorageHelper();
    m_oidcTProvider = new MockOidcTokenProvider();
    m_dmsHelper->_SetDependecy(m_oidcTProvider, m_dmsClient, m_AZHelper);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Vishal.Shingare                  02/2020
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelDmsSupportTests::TearDown()
    {
    ClearTestData(CreateTestDir());
    m_dmsClient = nullptr;
    m_AZHelper = nullptr;
    m_oidcTProvider = nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Vishal.Shingare                  02/2020
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelDmsSupportTests::SetUpTestCase()
    {
    CreateTestDir();
    Initialize();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Vishal.Shingare                  02/2020
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelDmsSupportTests::TearDownTestCase()
    {
    if (m_dmsHelper != nullptr)
        {
        delete m_dmsHelper;
        m_dmsHelper = nullptr;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Vishal.Shingare                  02/2020
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<DmsResponseData> iModelDmsSupportTests::GetUrlResult()
    {
    bvector<DmsResponseData> downloadUrls;

    DmsResponseData data;

    data.fileId = WString(L"06731413-cc7b-44d5-b520-a2c6ae07c2b9");
    data.fileName = WString(L"Visualization_Master.dgn");
    data.parentFolderId = WString(L"7973cc14-8fa2-47f3-9ec2-ed569f0dfd3a");
    data.downloadURL = WString(L"https://dev-connect-projectwisedocumentservice.bentley.com/pwdi-download?guid=b3ebd8f8-5c62-11ea-9207-65e331adc941");

    downloadUrls.push_back(data);
    return downloadUrls;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Vishal.Shingare                  02/2020
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelDmsSupportTests::ClearTestData(BeFileNameCR dirPath)
    {
    BeFileName clearPath(dirPath);

    if (clearPath.DoesPathExist())
        BeFileName::EmptyAndRemoveDirectory(clearPath.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Vishal.Shingare                  02/2020
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus readEntireFile(BentleyApi::WStringR contents, BentleyApi::BeFileNameCR fileName)
    {
    BentleyApi::BeFile errfile;
    if (BentleyApi::BeFileStatus::Success != errfile.Open(fileName.c_str(), BentleyApi::BeFileAccess::Read))
        return BSIERROR;

    BentleyApi::bvector<Byte> bytes;
    if (BentleyApi::BeFileStatus::Success != errfile.ReadEntireFile(bytes))
        return BSIERROR;

    if (bytes.empty())
        return BSISUCCESS;

    bytes.push_back('\0'); // End of stream

    const Byte utf8BOM[] = { 0xef, 0xbb, 0xbf };
    if (bytes[0] == utf8BOM[0] || bytes[1] == utf8BOM[1] || bytes[2] == utf8BOM[2])
        contents.AssignUtf8((Utf8CP)(bytes.data() + 3));
    else
        contents.AssignA((char*)bytes.data());

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Vishal.Shingare                  02/2020
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyApi::WString DmsArgvMaker::ReadRspFileFromTestData(BentleyApi::WCharCP rspFileBaseName)
    {
    BentleyApi::BeFileName rspFileName;
    rspFileName = iModelDmsSupportTests::GetOutputDir();
    rspFileName.PopDir();
    rspFileName.AppendToPath(rspFileBaseName);

    BentleyApi::WString wargs;
    EXPECT_EQ(BSISUCCESS, readEntireFile(wargs, rspFileName));

    return wargs;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Vishal.Shingare                  02/2020
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyApi::BentleyStatus DmsArgvMaker::ParseArgsFromRspFile(BentleyApi::WStringCR wargs)
    {
    BentleyApi::bvector<BentleyApi::WString> strings;
    BentleyApi::BeStringUtilities::ParseArguments(strings, wargs.c_str(), L"\n\r");

    if (strings.empty())
        return BentleyApi::BSIERROR;

    for (auto const& str : strings)
        {
        if (!str.empty())
            PushArg(str);
        }

    return BentleyApi::BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Vishal.Shingare                  02/2020
+---------------+---------------+---------------+---------------+---------------+------*/
void DmsArgvMaker::Clear()
    {
    for (auto ptr : m_ptrs)
        free((void*)ptr);
    m_ptrs.clear();
    m_bargptrs.clear();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Vishal.Shingare                  02/2020
+---------------+---------------+---------------+---------------+---------------+------*/
void DmsArgvMaker::PushArg(BentleyApi::WStringCR arg)
    {
    m_ptrs.push_back(_wcsdup(arg.c_str()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Vishal.Shingare                  02/2020
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyApi::WString DmsArgvMaker::PopArg()
    {
    auto lastArg = m_ptrs.back();
    m_ptrs.pop_back();
    return lastArg;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Vishal.Shingare                  02/2020
+---------------+---------------+---------------+---------------+---------------+------*/
void iModelDmsSupportTests::Initialize()
    {
    DmsArgvMaker argvMaker;
    EXPECT_EQ(BSISUCCESS, argvMaker.ParseArgsFromRspFile(argvMaker.ReadRspFileFromTestData(L"iModelDmsSupport-Input.rsp")));

    //Parse iModelHub Server Args
    BentleyApi::bvector<BentleyApi::WString> bridgeArgs;
    EXPECT_EQ(BSISUCCESS, m_iModelHubArgs.ParseCommandLine(bridgeArgs, argvMaker.GetArgVector()));
    EXPECT_EQ(3, bridgeArgs.size());

    //Parse Dms Server Args
    EXPECT_EQ(BSISUCCESS, m_dmsServerArgs.ParseCommandLine(bridgeArgs, argvMaker.GetArgVector(), false));
    EXPECT_EQ(6, bridgeArgs.size());

    if (m_dmsServerArgs.m_dmsType == iModelDmsSupport::SessionType::PWDIDMS)
        m_dmsHelper = new DmsHelper(Utf8String(), m_iModelHubArgs.m_accessToken, 1,  Utf8String("PWDI"), Utf8String(m_dmsServerArgs.m_dataSource));
    else
        m_dmsHelper = new DmsHelper(Utf8String(), m_iModelHubArgs.m_accessToken, 1);
    argvMaker.Clear();
    }

struct iModelDmsSupportTester : public iModelDmsSupportTests
    {

    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Vishal.Shingare                  02/2020
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(iModelDmsSupportTester, _StageInputFile)
    {
    auto m_token = new OidcStaticTokenProvider(m_iModelHubArgs.m_accessToken);

    EXPECT_CALL(*m_oidcTProvider, GetToken())
        .Times(1)
        .WillRepeatedly(Return(m_token->m_token));

    EXPECT_CALL(*m_dmsClient, _InitializeSession(Utf8String(m_dmsServerArgs.m_inputFileUrn), Utf8String("PWDI")))
        .Times(1)
        .WillRepeatedly(Return(true));

    EXPECT_CALL(*m_dmsClient, _UnInitializeSession())
        .WillRepeatedly(Return(true));

    auto result = GetUrlResult();
    Utf8String mtoken = Utf8String("Bearer ");
    mtoken.append(Utf8String(m_iModelHubArgs.m_accessToken));
    EXPECT_CALL(*m_dmsClient, _GetDownloadURLs(mtoken, Utf8String(m_dmsServerArgs.m_dataSource)))
        .Times(1)
        .WillRepeatedly(Return(result));

    EXPECT_CALL(*m_AZHelper, _InitializeSession(WString(L"https://dev-connect-projectwisedocumentservice.bentley.com/pwdi-download?guid=b3ebd8f8-5c62-11ea-9207-65e331adc941")))
        .Times(1)
        .WillRepeatedly(Return(true));

    EXPECT_CALL(*m_AZHelper, _UnInitializeSession())
        .WillRepeatedly(Return(true));

    EXPECT_CALL(*m_AZHelper, _UnInitialize())
        .WillRepeatedly(Return(true));

    BeFileName inFileName = GetOutputDir();
    inFileName.AppendToPath(L"Visualization_Master.dgn");
    EXPECT_CALL(*m_AZHelper, _AsyncStageInputFile(inFileName, 1))
        .Times(1)
        .WillRepeatedly(Return(Tasks::CreateCompletedAsyncTask(AzureResult::Success(AzureFileResponse()))));

    m_dmsHelper->_InitializeSession(m_dmsServerArgs.m_inputFileUrn);

    auto request = m_dmsHelper->_StageInputFile(inFileName);

    EXPECT_TRUE(request);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Vishal.Shingare                  02/2020
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(iModelDmsSupportTester, _FetchWorkspace)
    {
    auto m_token = new OidcStaticTokenProvider(m_iModelHubArgs.m_accessToken);

    EXPECT_CALL(*m_oidcTProvider, GetToken())
        .Times(1)
        .WillRepeatedly(Return(m_token->m_token));

    EXPECT_CALL(*m_dmsClient, _InitializeSession(Utf8String(m_dmsServerArgs.m_inputFileUrn), Utf8String("PWDI")))
        .Times(1)
        .WillRepeatedly(Return(true));

    EXPECT_CALL(*m_dmsClient, _UnInitializeSession())
        .WillRepeatedly(Return(true));

    auto result = GetUrlResult();

    DmsResponseData rDmsData;
    rDmsData.downloadURL = WString(L"#This file is automatically generated. Changes will not be persisted.");
    rDmsData.fileId = WString(L"06731413-cc7b-44d5-b520-a2c6ae07c2b9");
    rDmsData.parentFolderId = WString(L"Workspace");
    Utf8String mtoken = Utf8String("Bearer ");
    mtoken.append(Utf8String(m_iModelHubArgs.m_accessToken));
    EXPECT_CALL(*m_dmsClient, _GetWorkspaceFiles(mtoken, Utf8String(m_dmsServerArgs.m_dataSource), _))
        .WillOnce(DoAll(SetArgReferee<2>(rDmsData), Return(result)));

    EXPECT_CALL(*m_AZHelper, _InitializeSession(WString(L"https://dev-connect-projectwisedocumentservice.bentley.com/pwdi-download?guid=b3ebd8f8-5c62-11ea-9207-65e331adc941")))
        .Times(1)
        .WillRepeatedly(Return(true));

    EXPECT_CALL(*m_AZHelper, _UnInitializeSession())
        .WillRepeatedly(Return(true));

    EXPECT_CALL(*m_AZHelper, _UnInitialize())
        .WillRepeatedly(Return(true));

    BeFileName inFileName = GetOutputDir();
    inFileName.AppendToPath(L"7973cc14-8fa2-47f3-9ec2-ed569f0dfd3a\\Visualization_Master.dgn");

    EXPECT_CALL(*m_AZHelper, _AsyncStageInputFile(inFileName, 1))
        .Times(1)
        .WillRepeatedly(Return(Tasks::CreateCompletedAsyncTask(AzureResult::Success(AzureFileResponse()))));

    m_dmsHelper->_InitializeSession(m_dmsServerArgs.m_inputFileUrn);
    BeFileName workspaceCfgFile;
    auto response = m_dmsHelper->_FetchWorkspace(workspaceCfgFile, m_dmsServerArgs.m_inputFileUrn, GetOutputDir(), m_dmsServerArgs.m_isv8i, m_dmsServerArgs.m_additionalFilePatterns);

    EXPECT_EQ(0, response);
    }