/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#pragma once
#include <Bentley/Bentley.h>
#include <iModelBridge/iModelBridgeFwk.h>
#include <iModelDmsSupport/iModelDmsSupport.h>
#include <iModelDmsSupport/DmsHelper.h>
#include <Bentley/BeTest.h>
#include <gmock/gmock.h>
#include <Bentley/BeFileName.h>
#include <WebServices/iModelHub/Client/OidcToken.h>
#include <WebServices\Connect\ISecurityToken.h>

using namespace BentleyApi::Dgn;
using testing::Return;
using testing::DoAll;
using testing::SetArgReferee;
using testing::_;


USING_NAMESPACE_BENTLEY_DGN

struct MockDmsClient : public DmsClient
    {
    public:
        MockDmsClient()/* : DmsClient()*/ {}
        ~MockDmsClient() {}
        MOCK_METHOD0(_UnInitializeSession, bool());
        MOCK_METHOD1(_ParseInputUrlForPS, bool(Utf8String url));
        MOCK_METHOD1(_ParseInputUrlForPW, bool(Utf8String url));
        MOCK_METHOD2(_InitializeSession, bool(Utf8String fileUrl, Utf8String repositoryType));
        MOCK_METHOD2(_CreateQuery, Utf8PrintfString(Utf8String datasource, bool isWSQuery));
        MOCK_METHOD2(_GetDownloadURLs, bvector<DmsResponseData>(Utf8String token, Utf8String datasource));
        MOCK_METHOD3(_GetWorkspaceFiles, bvector<DmsResponseData>(Utf8String token, Utf8String datasource, DmsResponseData& cfgData));
    };

struct MockAzureBlobStorageHelper : public AzureBlobStorageHelper
    {
    public:
        MockAzureBlobStorageHelper() /*: AzureBlobStorageHelper() */
            {
            }
        ~MockAzureBlobStorageHelper() {}
        MOCK_METHOD0(_Initialize, bool());
        MOCK_METHOD0(_UnInitialize, bool());
        MOCK_METHOD1(_InitializeSession, bool(WStringCR pwMoniker));
        MOCK_METHOD0(_UnInitializeSession, bool());
        MOCK_METHOD2(_AsyncStageInputFile, AsyncTaskPtr<AzureResult>(BeFileNameCR fileLocation, int maxRetries));
        MOCK_METHOD1(_StageInputFile, bool(BeFileNameCR fileLocation));
    };

struct MockOidcTokenProvider : public WebServices::IConnectTokenProvider
    {
    public:
        MockOidcTokenProvider() /*: IConnectTokenProvider()*/ {}
        ~MockOidcTokenProvider() {}
        MOCK_METHOD0(UpdateToken, Tasks::AsyncTaskPtr<WebServices::ISecurityTokenPtr>());
        MOCK_METHOD0(GetToken, WebServices::ISecurityTokenPtr());
    };

struct iModelDmsSupportTests : public testing::Test
    {
    protected:
        static int s_argc;
        static char **s_argv;
        static DmsHelper* m_dmsHelper;
        static MockDmsClient* m_dmsClient;
        static MockAzureBlobStorageHelper* m_AZHelper;
        static MockOidcTokenProvider* m_oidcTProvider;

        static iModelBridgeFwk::DmsServerArgs m_dmsServerArgs;
        static iModelBridgeFwk::IModelHubArgs m_iModelHubArgs;

    public:
        static void SetArgcArgv(int c, char** v) { s_argc = c; s_argv = v; }
        virtual void SetUp() override;
        virtual void TearDown() override;
        static void SetUpTestCase();
        static void TearDownTestCase();
        static void Initialize();

        bvector<DmsResponseData> GetUrlResult();
        void ClearTestData(BeFileNameCR dirPath);

        static BentleyApi::BeFileName CreateTestDir(WCharCP testDir = nullptr);
        static BentleyApi::BeFileName GetOutputDir();

    };

struct DmsArgvMaker
    {
    private:
        BentleyApi::bvector<BentleyApi::WCharCP> m_ptrs;
    public:
        BentleyApi::bvector<WCharCP> m_bargptrs;

    public:
        DmsArgvMaker() {}
        ~DmsArgvMaker() {}

        BentleyApi::BentleyStatus ParseArgsFromRspFile(BentleyApi::WStringCR wargs);
        static BentleyApi::WString ReadRspFileFromTestData(BentleyApi::WCharCP rspFileBaseName);
        void PushArg(BentleyApi::WStringCR);
        BentleyApi::WString PopArg();
        void Clear();

        wchar_t const** GetArgV() const { return const_cast<wchar_t const**>(m_ptrs.data()); }
        int GetArgC() const { return (int)m_ptrs.size(); }
        BentleyApi::bvector<BentleyApi::WString> GetArgVector() const {return BentleyApi::bvector<BentleyApi::WString> (m_ptrs.begin(), m_ptrs.end());}
    };