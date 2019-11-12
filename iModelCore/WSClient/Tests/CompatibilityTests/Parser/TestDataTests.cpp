/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#define COMPILE_TESTS
#ifdef COMPILE_TESTS

#include "TestData.h"

#include <Bentley/BeTest.h>

using namespace std;
using namespace testing;

struct TestDataTests : Test {};

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(TestDataTests, PrintToTestRepository_ServerAndRepositoryId_PrintsAsArgs)
    {
    TestRepository repo;
    repo.serverUrl = "TestURL";
    repo.id = "TestId";

    std::stringstream str;
    PrintTo(repo, &str);
    EXPECT_STREQ("-url TestURL -r TestId ", str.str().c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(TestDataTests, PrintToTestRepository_LabelAndServerAndRepositoryId_PrintsAsArgs)
    {
    TestRepository repo;
    repo.serverUrl = "TestURL";
    repo.id = "TestId";
    repo.label = "TestLabel";

    std::stringstream str;
    PrintTo(repo, &str);
    EXPECT_STREQ("-l TestLabel -url TestURL -r TestId ", str.str().c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(TestDataTests, PrintToTestRepository_Schemas_PrintsAsArgs)
    {
    TestRepository repo;
    repo.schemasDir = BeFileName("TestDir");

    std::stringstream str;
    PrintTo(repo, &str);
    EXPECT_STREQ("-schemas TestDir ", str.str().c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(TestDataTests, PrintToTestRepository_LabelAndSchemas_PrintsAsArgs)
    {
    TestRepository repo;
    repo.schemasDir = BeFileName("TestDir");
    repo.label = "TestLabel";

    std::stringstream str;
    PrintTo(repo, &str);
    EXPECT_STREQ("-l TestLabel -schemas TestDir ", str.str().c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(TestDataTests, PrintToTestRepository_CredentialsUsed_PrintsAsArgs)
    {
    TestRepository repo;
    repo.schemasDir = BeFileName("TestDir");
    repo.credentials = Credentials("A", "B");

    std::stringstream str;
    PrintTo(repo, &str);
    EXPECT_STREQ("-schemas TestDir -auth:basic A:B ", str.str().c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(TestDataTests, PrintToTestRepository_CredentialsImsDevUsed_PrintsAsArgs)
    {
    TestRepository repo;
    repo.schemasDir = BeFileName("TestDir");
    repo.credentials = Credentials("A", "B");
    repo.environment = std::make_shared<UrlProvider::Environment>(UrlProvider::Environment::Dev);

    std::stringstream str;
    PrintTo(repo, &str);
    EXPECT_STREQ("-schemas TestDir -auth:dev:ims A:B ", str.str().c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(TestDataTests, PrintToTestRepository_CredentialsImsQaUsed_PrintsAsArgs)
    {
    TestRepository repo;
    repo.schemasDir = BeFileName("TestDir");
    repo.credentials = Credentials("A", "B");
    repo.environment = std::make_shared<UrlProvider::Environment>(UrlProvider::Environment::Qa);

    std::stringstream str;
    PrintTo(repo, &str);
    EXPECT_STREQ("-schemas TestDir -auth:qa:ims A:B ", str.str().c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(TestDataTests, PrintToTestRepository_CredentialsImsProdUsed_PrintsAsArgs)
    {
    TestRepository repo;
    repo.schemasDir = BeFileName("TestDir");
    repo.credentials = Credentials("A", "B");
    repo.environment = std::make_shared<UrlProvider::Environment>(UrlProvider::Environment::Release);

    std::stringstream str;
    PrintTo(repo, &str);
    EXPECT_STREQ("-schemas TestDir -auth:prod:ims A:B ", str.str().c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(TestDataTests, PrintToTestRepository_TokenImsDevUsed_PrintsAsArgs)
    {
    TestRepository repo;
    repo.schemasDir = BeFileName("TestDir");
    repo.tokenPath = BeFileName("TokenPath");
    repo.environment = std::make_shared<UrlProvider::Environment>(UrlProvider::Environment::Dev);

    std::stringstream str;
    PrintTo(repo, &str);
    EXPECT_STREQ("-schemas TestDir -auth:dev:token TokenPath ", str.str().c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(TestDataTests, PrintToTestRepository_TokenImsQaUsed_PrintsAsArgs)
    {
    TestRepository repo;
    repo.schemasDir = BeFileName("TestDir");
    repo.tokenPath = BeFileName("TokenPath");
    repo.environment = std::make_shared<UrlProvider::Environment>(UrlProvider::Environment::Qa);

    std::stringstream str;
    PrintTo(repo, &str);
    EXPECT_STREQ("-schemas TestDir -auth:qa:token TokenPath ", str.str().c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(TestDataTests, PrintToTestRepository_TokenImsProdUsed_PrintsAsArgs)
    {
    TestRepository repo;
    repo.schemasDir = BeFileName("TestDir");
    repo.tokenPath = BeFileName("TokenPath");
    repo.environment = std::make_shared<UrlProvider::Environment>(UrlProvider::Environment::Release);

    std::stringstream str;
    PrintTo(repo, &str);
    EXPECT_STREQ("-schemas TestDir -auth:prod:token TokenPath ", str.str().c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(TestDataTests, PrintToTestRepository_ValidateCertificateFalse_PrintsAsArgs)
    {
    TestRepository repo;
    repo.schemasDir = BeFileName("TestDir");
    repo.validateCertificate = false;

    std::stringstream str;
    PrintTo(repo, &str);
    EXPECT_STREQ("-schemas TestDir -validateCertificate false ", str.str().c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(TestDataTests, PrintToTestRepository_ValidateCertificateTrue_PrintsAsArgs)
    {
    TestRepository repo;
    repo.schemasDir = BeFileName("TestDir");
    repo.validateCertificate = true;

    std::stringstream str;
    PrintTo(repo, &str);
    EXPECT_STREQ("-schemas TestDir ", str.str().c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(TestDataTests, PrintToTestRepositories_Create_PrintsAsArgs)
    {
    TestRepositories repos;
    repos.create.schemasDir = BeFileName("TestDir");

    std::stringstream str;
    PrintTo(repos, &str);
    EXPECT_STREQ("--createcache -schemas TestDir ", str.str().c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(TestDataTests, PrintToTestRepositories_Upgrade_PrintsAsArgs)
    {
    TestRepositories repos;
    repos.create.schemasDir = BeFileName("TestDirA");
    repos.upgrade.schemasDir = BeFileName("TestDirB");

    std::stringstream str;
    PrintTo(repos, &str);
    EXPECT_STREQ("--createcache -schemas TestDirA --upgradecache -schemas TestDirB ", str.str().c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(TestDataTests, PrintToTestRepositories_DownloadSchemas_PrintsAsArgs)
    {
    TestRepositories repos;
    repos.downloadSchemas.schemasDir = BeFileName("TestDir");

    std::stringstream str;
    PrintTo(repos, &str);
    EXPECT_STREQ("--downloadSchemas -schemas TestDir ", str.str().c_str());
    }

#endif
