/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/iModelHubClient/Integration/ProgressCallbackTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "iModelTestsBase.h"
#include "LRPJobBackdoorAPI.h"
#include <iostream>

USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_IMODELHUB
USING_NAMESPACE_BENTLEY_IMODELHUB_UNITTESTS
USING_NAMESPACE_BENTLEY_SQLITE

static const Utf8CP s_iModelName = "ProgressCallbackTests";

/*--------------------------------------------------------------------------------------+
* @bsiclass                                     Karolis.Dziedzelis              08/2018
+---------------+---------------+---------------+---------------+---------------+------*/
struct ProgressCallbackTests : public iModelTestsBase
    {
    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Dziedzelis              08/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    static void SetUpTestCase()
        {
        iModelTestsBase::SetUpTestCase();
        iModelHubHelpers::AcquireAndAddChangeSets(s_client, s_info, 10);
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Dziedzelis              08/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    static void TearDownTestCase()
        {
        iModelTestsBase::TearDownTestCase();
        }
    };

/*--------------------------------------------------------------------------------------+
* @bsiclass                                     Karolis.Dziedzelis              08/2018
+---------------+---------------+---------------+---------------+---------------+------*/
struct ContinuousProgressCallback : public TestsProgressCallback
    {
    uint32_t m_uniqueCount = 0;
public:
    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Dziedzelis              08/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    Http::Request::ProgressCallback Get() override
        {
        return [&] (double bytesTransfered, double bytesTotal)
            {
            double lastProgress = m_lastProgressBytesTransfered;
            TestsProgressCallback::Get()(bytesTransfered, bytesTotal);
            if (m_lastProgressBytesTransfered != lastProgress)
                m_uniqueCount++;
            };
        }
    
    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Karolis.Dziedzelis              08/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    void VerifyCount(uint32_t count)
        {
        EXPECT_GT(m_uniqueCount, count);
        }
    };

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Karolis.Dziedzelis              08/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ProgressCallbackTests, ContinuousBriefcaseAcquireProgress)
    {
    BeFileName outputRoot;
    BeTest::GetHost().GetOutputRoot(outputRoot);
    ContinuousProgressCallback callback;
    BriefcaseInfoResult acquireResult = s_client->AcquireBriefcaseToDir(*s_info, outputRoot, true, Client::DefaultFileNameCallback, callback.Get())->GetResult();
    ASSERT_SUCCESS(acquireResult);
    callback.VerifyCount(20);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Karolis.Dziedzelis              08/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ProgressCallbackTests, ContinuousStandaloneDownloadProgress)
    {
    BeFileName outputRoot;
    BeTest::GetHost().GetOutputRoot(outputRoot);
    ContinuousProgressCallback callback;
    BeFileNameResult downloadResult = s_client->DownloadStandaloneBriefcase(*s_info, [=] (iModelInfo imodelInfo, FileInfo fileInfo)
        {
        BeFileName filePath = OutputDir();
        filePath.AppendToPath(BeFileName(imodelInfo.GetId()));
        filePath.AppendToPath(BeFileName(fileInfo.GetFileName()));
        return filePath;
        }, callback.Get())->GetResult();
    ASSERT_SUCCESS(downloadResult);
    callback.VerifyCount(20);
    }
