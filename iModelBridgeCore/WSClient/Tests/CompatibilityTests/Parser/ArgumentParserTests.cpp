/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/CompatibilityTests/Parser/ArgumentParserTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

//#define COMPILE_TESTS
#ifdef DEBUG
#define COMPILE_TESTS
#endif

#ifdef COMPILE_TESTS

#include "ArgumentParser.h"

#include <Bentley/BeTest.h>
#include <vector>
#include <Bentley/Base64Utilities.h>
#include <Bentley/BeDebugLog.h>

using namespace std;
using namespace testing;
typedef vector<char*> TestArgs;

Utf8String s_xmlTokenStr =
R"(<saml:Assertion MajorVersion="1" MinorVersion="1" xmlns:saml="urn:oasis:names:tc:SAML:1.0:assertion">
            <saml:Conditions 
                NotBefore   ="2000-01-01T00:00:00.000Z"
                NotOnOrAfter="2000-01-03T00:00:00.000Z">
            </saml:Conditions>
        </saml:Assertion>)";
Utf8String s_tokenFilePath;
Utf8String s_schemasDir;
Utf8String s_schemasDirWithSeperator;

char* GetTestTokenPath()
    {
    if (!s_tokenFilePath.empty())
        return (char*) s_tokenFilePath.c_str();

    BeFileName path;
    BeTest::GetHost().GetOutputRoot(path);
    BeFileName::CreateNewDirectory(path);
    path.AppendToPath(L"TestSamleToken.xml");

    BeFile file;
    file.Create(path);
    EXPECT_EQ(BeFileStatus::Success, file.Write(nullptr, s_xmlTokenStr.c_str(), (uint32_t) s_xmlTokenStr.size()));
    file.Close();

    s_tokenFilePath = path.GetNameUtf8();
    return (char*) s_tokenFilePath.c_str();
    }

BeFileName GetTestSchemasDirPath()
    {
    BeFileName path;
    BeTest::GetHost().GetOutputRoot(path);
    path.AppendToPath(L"TestSchemasDir");

    if (!path.DoesPathExist())
        EXPECT_EQ(BeFileNameStatus::Success, BeFileName::CreateNewDirectory(path));

    return path;
    }

char* GetTestSchemasDir()
    {
    if (s_schemasDir.empty())
        s_schemasDir = GetTestSchemasDirPath().GetNameUtf8();
    return (char*) s_schemasDir.c_str();
    }

char* GetTestSchemasDirWithSeperator()
    {
    if (s_schemasDirWithSeperator.empty())
        s_schemasDirWithSeperator = GetTestSchemasDirPath().AppendSeparator().GetNameUtf8();
    return (char*) s_schemasDirWithSeperator.c_str();
    }

void PrintTo(const TestArgs& value, ::std::ostream* os)
    {
    for (auto arg : value)
        *os << Utf8PrintfString("%s ", arg);
    }

struct ArgumentParserTests : Test {};

struct ArgumentParserTests_TempDirArg : TestWithParam<TestArgs> {};
INSTANTIATE_TEST_CASE_P(, ArgumentParserTests_TempDirArg, ValuesIn(vector<TestArgs>{
        {"Foo.exe", "--workdir", "TestDir\\TestFolder", "--createcache", "-url", "URL", "-r", "RId"},
        {"Foo.exe", "--createcache", "-url", "URL", "-r", "RId", "--workdir", "TestDir\\TestFolder"},
    }));

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(ArgumentParserTests_TempDirArg, Parse_ValidParameters_FillsTestDataAndReturnsZero)
    {
    auto args = GetParam();

    int logLevel;
    BeFileName workDir;
    bvector<TestRepositories> testData;
    std::stringstream err;
    std::stringstream out;

    EXPECT_EQ(0, ArgumentParser::Parse((int) args.size(), args.data(), logLevel, workDir, testData, &err, &out));
    EXPECT_EQ(L"TestDir\\TestFolder", workDir);
    EXPECT_EQ("", err.str());
    EXPECT_EQ("", out.str());

    ASSERT_EQ(1, testData.size());

    EXPECT_FALSE(testData[0].upgrade.IsValid());
    EXPECT_TRUE(testData[0].create.IsValid());

    EXPECT_EQ("URL", testData[0].create.serverUrl);
    EXPECT_EQ("RId", testData[0].create.id);
    EXPECT_EQ(Credentials(), testData[0].create.credentials);
    EXPECT_EQ(nullptr, testData[0].create.token);
    EXPECT_EQ(nullptr, testData[0].create.environment);
    }

struct ArgumentParserTests_SilentProvided : TestWithParam<TestArgs> {};
INSTANTIATE_TEST_CASE_P(, ArgumentParserTests_SilentProvided, ValuesIn(vector<TestArgs>{
        {"Foo.exe", "--silent", "--createcache", "-url", "URL", "-r", "RId"},
        {"Foo.exe", "--createcache", "-url", "URL", "-r", "RId", "--silent"},
    }));

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(ArgumentParserTests_SilentProvided, Parse_Silent_LogLevelZero)
    {
    auto args = GetParam();

    int logLevel;
    BeFileName workDir;
    bvector<TestRepositories> testData;
    std::stringstream err;
    std::stringstream out;

    EXPECT_EQ(0, ArgumentParser::Parse((int) args.size(), args.data(), logLevel, workDir, testData, &err, &out));
    EXPECT_EQ("", err.str());
    EXPECT_EQ("", out.str());
    EXPECT_EQ(0, logLevel);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ArgumentParserTests, Parse_SilentNotProvided_LogLevelOne)
    {
    TestArgs args = {"Foo.exe", "--createcache", "-url", "URL", "-r", "RId"};

    int logLevel;
    BeFileName workDir;
    bvector<TestRepositories> testData;
    std::stringstream err;
    std::stringstream out;

    EXPECT_EQ(0, ArgumentParser::Parse((int) args.size(), args.data(), logLevel, workDir, testData, &err, &out));
    EXPECT_EQ("", err.str());
    EXPECT_EQ("", out.str());
    EXPECT_EQ(1, logLevel);
    }

struct ArgumentParserTests_CreateWithoutCredentials : TestWithParam<TestArgs> {};
INSTANTIATE_TEST_CASE_P(, ArgumentParserTests_CreateWithoutCredentials, ValuesIn(vector<TestArgs>{
        {"Foo.exe", "--createcache", "-url", "URL", "-r", "RId"},
        {"Foo.exe", "--createcache", "-r", "RId", "-url", "URL"},
    }));

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(ArgumentParserTests_CreateWithoutCredentials, Parse_ValidParameters_FillsTestDataAndReturnsZero)
    {
    auto args = GetParam();

    int logLevel;
    BeFileName workDir;
    bvector<TestRepositories> testData;
    std::stringstream err;
    std::stringstream out;

    EXPECT_EQ(0, ArgumentParser::Parse((int) args.size(), args.data(), logLevel, workDir, testData, &err, &out));
    EXPECT_TRUE(workDir.empty());
    EXPECT_EQ("", err.str());
    EXPECT_EQ("", out.str());

    ASSERT_EQ(1, testData.size());

    EXPECT_FALSE(testData[0].upgrade.IsValid());
    EXPECT_TRUE(testData[0].create.IsValid());

    EXPECT_EQ("URL", testData[0].create.serverUrl);
    EXPECT_EQ("RId", testData[0].create.id);
    EXPECT_EQ(Credentials(), testData[0].create.credentials);
    EXPECT_EQ(nullptr, testData[0].create.token);
    EXPECT_EQ(nullptr, testData[0].create.environment);
    }

struct ArgumentParserTests_CreateWithBasicAuth : TestWithParam<TestArgs> {};
INSTANTIATE_TEST_CASE_P(, ArgumentParserTests_CreateWithBasicAuth, ValuesIn(vector<TestArgs>{
        {"Foo.exe", "--createcache", "-url", "URL", "-r", "RId", "-auth:basic", "TestUser:TestPass"},
        {"Foo.exe", "--createcache", "-r", "RId", "-auth:basic", "TestUser:TestPass", "-url", "URL"},
    }));

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(ArgumentParserTests_CreateWithBasicAuth, Parse_ValidParameters_FillsTestDataAndReturnsZero)
    {
    auto args = GetParam();

    int logLevel;
    BeFileName workDir;
    bvector<TestRepositories> testData;
    std::stringstream err;
    std::stringstream out;

    EXPECT_EQ(0, ArgumentParser::Parse((int) args.size(), args.data(), logLevel, workDir, testData, &err, &out));
    EXPECT_TRUE(workDir.empty());
    EXPECT_EQ("", err.str());
    EXPECT_EQ("", out.str());

    ASSERT_EQ(1, testData.size());

    EXPECT_FALSE(testData[0].upgrade.IsValid());
    EXPECT_TRUE(testData[0].create.IsValid());

    EXPECT_EQ("URL", testData[0].create.serverUrl);
    EXPECT_EQ("RId", testData[0].create.id);
    EXPECT_EQ(Credentials("TestUser", "TestPass"), testData[0].create.credentials);
    EXPECT_EQ(nullptr, testData[0].create.token);
    EXPECT_EQ(nullptr, testData[0].create.environment);
    }

struct ArgumentParserTests_CreateWithImsAuth : TestWithParam<pair<TestArgs, UrlProvider::Environment>> {};
INSTANTIATE_TEST_CASE_P(, ArgumentParserTests_CreateWithImsAuth, ValuesIn(vector<pair<TestArgs, UrlProvider::Environment>>{
        {{"Foo.exe", "--createcache", "-url", "URL", "-r", "RId", "-auth:dev:ims", "TestUser:TestPass"}, UrlProvider::Environment::Dev},
        {{"Foo.exe", "--createcache", "-url", "URL", "-r", "RId", "-auth:qa:ims", "TestUser:TestPass"}, UrlProvider::Environment::Qa},
        {{"Foo.exe", "--createcache", "-url", "URL", "-r", "RId", "-auth:prod:ims", "TestUser:TestPass"}, UrlProvider::Environment::Release},
    }));

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(ArgumentParserTests_CreateWithImsAuth, Parse_ValidParameters_FillsTestDataAndReturnsZero)
    {
    auto param = GetParam();
    auto args = param.first;

    int logLevel;
    BeFileName workDir;
    bvector<TestRepositories> testData;
    std::stringstream err;
    std::stringstream out;

    EXPECT_EQ(0, ArgumentParser::Parse((int) args.size(), args.data(), logLevel, workDir, testData, &err, &out));
    EXPECT_TRUE(workDir.empty());
    EXPECT_EQ("", err.str());
    EXPECT_EQ("", out.str());

    ASSERT_EQ(1, testData.size());

    EXPECT_FALSE(testData[0].upgrade.IsValid());
    EXPECT_TRUE(testData[0].create.IsValid());

    EXPECT_EQ("URL", testData[0].create.serverUrl);
    EXPECT_EQ("RId", testData[0].create.id);
    EXPECT_EQ(Credentials("TestUser", "TestPass"), testData[0].create.credentials);
    EXPECT_EQ(nullptr, testData[0].create.token);

    ASSERT_NE(nullptr, testData[0].create.environment);
    EXPECT_EQ(param.second, *testData[0].create.environment);
    }

struct ArgumentParserTests_CreateWithImsTokenAuth : TestWithParam<pair<TestArgs, UrlProvider::Environment>> {};
INSTANTIATE_TEST_CASE_P(, ArgumentParserTests_CreateWithImsTokenAuth, ValuesIn(vector<pair<TestArgs, UrlProvider::Environment>>{
        {{"Foo.exe", "--createcache", "-url", "URL", "-r", "RId", "-auth:dev:token", GetTestTokenPath()}, UrlProvider::Environment::Dev},
        {{"Foo.exe", "--createcache", "-url", "URL", "-r", "RId", "-auth:qa:token", GetTestTokenPath()}, UrlProvider::Environment::Qa},
        {{"Foo.exe", "--createcache", "-url", "URL", "-r", "RId", "-auth:prod:token", GetTestTokenPath()}, UrlProvider::Environment::Release},
    }));

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(ArgumentParserTests_CreateWithImsTokenAuth, Parse_ValidParameters_FillsTestDataAndReturnsZero)
    {
    auto param = GetParam();
    auto args = param.first;

    int logLevel;
    BeFileName workDir;
    bvector<TestRepositories> testData;
    std::stringstream err;
    std::stringstream out;

    EXPECT_EQ(0, ArgumentParser::Parse((int) args.size(), args.data(), logLevel, workDir, testData, &err, &out));
    EXPECT_TRUE(workDir.empty());
    EXPECT_EQ("", err.str());
    EXPECT_EQ("", out.str());

    ASSERT_EQ(1, testData.size());

    EXPECT_FALSE(testData[0].upgrade.IsValid());
    EXPECT_TRUE(testData[0].create.IsValid());

    EXPECT_EQ("URL", testData[0].create.serverUrl);
    EXPECT_EQ("RId", testData[0].create.id);
    EXPECT_EQ(Credentials(), testData[0].create.credentials);
    EXPECT_STREQ(s_xmlTokenStr.c_str(), testData[0].create.token->AsString().c_str());

    ASSERT_NE(nullptr, testData[0].create.environment);
    EXPECT_EQ(param.second, *testData[0].create.environment);
    }

struct ArgumentParserTests_CreateAndUpgradeWithBasicAuth : TestWithParam<TestArgs> {};
INSTANTIATE_TEST_CASE_P(, ArgumentParserTests_CreateAndUpgradeWithBasicAuth, ValuesIn(vector<TestArgs>{
        {"Foo.exe", "--createcache", "-url", "URL1", "-r", "R1", "-auth:basic", "U1:P1", "--upgradecache", "-url", "URL2", "-r", "R2", "-auth:basic", "U2:P2"},
        {"Foo.exe", "--createcache", "-auth:basic", "U1:P1", "-url", "URL1", "-r", "R1", "--upgradecache", "-r", "R2", "-auth:basic", "U2:P2", "-url", "URL2"},
    }));

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(ArgumentParserTests_CreateAndUpgradeWithBasicAuth, Parse_ValidParameters_FillsTestDataAndReturnsZero)
    {
    auto args = GetParam();

    int logLevel;
    BeFileName workDir;
    bvector<TestRepositories> testData;
    std::stringstream err;
    std::stringstream out;

    EXPECT_EQ(0, ArgumentParser::Parse((int) args.size(), args.data(), logLevel, workDir, testData, &err, &out));
    EXPECT_TRUE(workDir.empty());
    EXPECT_EQ("", err.str());
    EXPECT_EQ("", out.str());

    ASSERT_EQ(1, testData.size());

    EXPECT_TRUE(testData[0].create.IsValid());
    EXPECT_TRUE(testData[0].upgrade.IsValid());

    EXPECT_EQ("URL1", testData[0].create.serverUrl);
    EXPECT_EQ("R1", testData[0].create.id);
    EXPECT_EQ(Credentials("U1", "P1"), testData[0].create.credentials);
    EXPECT_EQ(nullptr, testData[0].create.token);
    EXPECT_EQ(nullptr, testData[0].create.environment);

    EXPECT_EQ("URL2", testData[0].upgrade.serverUrl);
    EXPECT_EQ("R2", testData[0].upgrade.id);
    EXPECT_EQ(Credentials("U2", "P2"), testData[0].upgrade.credentials);
    EXPECT_EQ(nullptr, testData[0].upgrade.token);
    EXPECT_EQ(nullptr, testData[0].upgrade.environment);
    }

struct ArgumentParserTests_CreateAndUpgradeWithDefaultParams : TestWithParam<TestArgs> {};
INSTANTIATE_TEST_CASE_P(, ArgumentParserTests_CreateAndUpgradeWithDefaultParams, ValuesIn(vector<TestArgs>{
        {"Foo.exe", "--createcache", "-url", "URL1", "-r", "R2", "-auth:basic", "U2:P2", "--upgradecache", "-url", "URL2"},
        {"Foo.exe", "--createcache", "-url", "URL2", "-r", "R1", "-auth:basic", "U2:P2", "--upgradecache", "-r", "R2"},
        {"Foo.exe", "--createcache", "-url", "URL2", "-r", "R2", "-auth:basic", "U1:P1", "--upgradecache", "-auth:basic", "U2:P2"},
        {"Foo.exe", "--createcache", "-url", "URL2", "-r", "R2", "-auth:qa:ims", "U1:P1", "--upgradecache", "-auth:basic", "U2:P2"},
        {"Foo.exe", "--createcache", "-url", "URL2", "-r", "R2", "-auth:qa:token", GetTestTokenPath(), "--upgradecache", "-auth:basic", "U2:P2"},
    }));

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(ArgumentParserTests_CreateAndUpgradeWithDefaultParams, Parse_UpgradeCacheDefaultsParametersFromCreateCache_FillsTestDataAndReturnsZero)
    {
    auto args = GetParam();

    int logLevel;
    BeFileName workDir;
    bvector<TestRepositories> testData;
    std::stringstream err;
    std::stringstream out;

    EXPECT_EQ(0, ArgumentParser::Parse((int) args.size(), args.data(), logLevel, workDir, testData, &err, &out));
    EXPECT_TRUE(workDir.empty());
    EXPECT_EQ("", err.str());
    EXPECT_EQ("", out.str());

    ASSERT_EQ(1, testData.size());

    EXPECT_TRUE(testData[0].create.IsValid());
    EXPECT_TRUE(testData[0].upgrade.IsValid());

    EXPECT_EQ("URL2", testData[0].upgrade.serverUrl);
    EXPECT_EQ("R2", testData[0].upgrade.id);
    EXPECT_EQ(Credentials("U2", "P2"), testData[0].upgrade.credentials);
    EXPECT_EQ(nullptr, testData[0].upgrade.token);
    EXPECT_EQ(nullptr, testData[0].upgrade.environment);
    }

struct ArgumentParserTests_CreateWithSchemas : TestWithParam<TestArgs> {};
INSTANTIATE_TEST_CASE_P(, ArgumentParserTests_CreateWithSchemas, ValuesIn(vector<TestArgs>{
        {"Foo.exe", "--createcache", "-schemas", GetTestSchemasDir()},
        {"Foo.exe", "--createcache", "-schemas", GetTestSchemasDirWithSeperator()},
    }));

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(ArgumentParserTests_CreateWithSchemas, Parse_ValidParameters_FillsTestDataAndReturnsZero)
    {
    auto args = GetParam();

    int logLevel;
    BeFileName workDir;
    bvector<TestRepositories> testData;
    std::stringstream err;
    std::stringstream out;

    EXPECT_EQ(0, ArgumentParser::Parse((int) args.size(), args.data(), logLevel, workDir, testData, &err, &out));
    EXPECT_TRUE(workDir.empty());
    EXPECT_EQ("", err.str());
    EXPECT_EQ("", out.str());

    ASSERT_EQ(1, testData.size());

    EXPECT_FALSE(testData[0].upgrade.IsValid());
    EXPECT_TRUE(testData[0].create.IsValid());

    EXPECT_STREQ(GetTestSchemasDirPath().AppendSeparator().c_str(), testData[0].create.schemasDir.c_str());
    EXPECT_EQ("", testData[0].create.serverUrl);
    EXPECT_EQ("", testData[0].create.id);
    EXPECT_EQ(Credentials(), testData[0].create.credentials);
    EXPECT_EQ(nullptr, testData[0].create.token);
    EXPECT_EQ(nullptr, testData[0].create.environment);
    }

struct ArgumentParserTests_UpgradeFromSchemas : TestWithParam<TestArgs> {};
INSTANTIATE_TEST_CASE_P(, ArgumentParserTests_UpgradeFromSchemas, ValuesIn(vector<TestArgs>{
        {"Foo.exe", "--createcache", "-schemas", GetTestSchemasDir(), "--upgradecache", "-url", "URL", "-r", "R"},
        {"Foo.exe", "--createcache", "-schemas", GetTestSchemasDirWithSeperator(), "--upgradecache", "-url", "URL", "-r", "R"},
    }));

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(ArgumentParserTests_UpgradeFromSchemas, Parse_ValidParameters_FillsTestDataAndReturnsZero)
    {
    auto args = GetParam();

    int logLevel;
    BeFileName workDir;
    bvector<TestRepositories> testData;
    std::stringstream err;
    std::stringstream out;

    EXPECT_EQ(0, ArgumentParser::Parse((int) args.size(), args.data(), logLevel, workDir, testData, &err, &out));
    EXPECT_TRUE(workDir.empty());
    EXPECT_EQ("", err.str());
    EXPECT_EQ("", out.str());

    ASSERT_EQ(1, testData.size());

    EXPECT_TRUE(testData[0].create.IsValid());
    EXPECT_TRUE(testData[0].upgrade.IsValid());

    EXPECT_STREQ(GetTestSchemasDirPath().AppendSeparator().c_str(), testData[0].create.schemasDir.c_str());
    EXPECT_EQ("", testData[0].create.serverUrl);
    EXPECT_EQ("", testData[0].create.id);
    EXPECT_EQ(Credentials(), testData[0].create.credentials);
    EXPECT_EQ(nullptr, testData[0].create.token);
    EXPECT_EQ(nullptr, testData[0].create.environment);

    EXPECT_STREQ(L"", testData[0].upgrade.schemasDir.c_str());
    EXPECT_EQ("URL", testData[0].upgrade.serverUrl);
    EXPECT_EQ("R", testData[0].upgrade.id);
    EXPECT_EQ(Credentials(), testData[0].upgrade.credentials);
    EXPECT_EQ(nullptr, testData[0].upgrade.token);
    EXPECT_EQ(nullptr, testData[0].upgrade.environment);
    }

struct ArgumentParserTests_UpgradeToSchemas : TestWithParam<TestArgs> {};
INSTANTIATE_TEST_CASE_P(, ArgumentParserTests_UpgradeToSchemas, ValuesIn(vector<TestArgs>{
        {"Foo.exe", "--createcache", "-url", "URL", "-r", "R", "--upgradecache", "-schemas", GetTestSchemasDir() },
    }));

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(ArgumentParserTests_UpgradeToSchemas, Parse_ValidParameters_FillsTestDataAndReturnsZero)
    {
    auto args = GetParam();

    int logLevel;
    BeFileName workDir;
    bvector<TestRepositories> testData;
    std::stringstream err;
    std::stringstream out;

    EXPECT_EQ(0, ArgumentParser::Parse((int) args.size(), args.data(), logLevel, workDir, testData, &err, &out));
    EXPECT_TRUE(workDir.empty());
    EXPECT_EQ("", err.str());
    EXPECT_EQ("", out.str());

    ASSERT_EQ(1, testData.size());

    EXPECT_TRUE(testData[0].create.IsValid());
    EXPECT_TRUE(testData[0].upgrade.IsValid());

    EXPECT_STREQ(L"", testData[0].create.schemasDir.c_str());
    EXPECT_EQ("URL", testData[0].create.serverUrl);
    EXPECT_EQ("R", testData[0].create.id);
    EXPECT_EQ(Credentials(), testData[0].create.credentials);
    EXPECT_EQ(nullptr, testData[0].create.token);
    EXPECT_EQ(nullptr, testData[0].create.environment);

    EXPECT_STREQ(GetTestSchemasDirPath().AppendSeparator().c_str(), testData[0].upgrade.schemasDir.c_str());
    EXPECT_EQ("", testData[0].upgrade.serverUrl);
    EXPECT_EQ("", testData[0].upgrade.id);
    EXPECT_EQ(Credentials(), testData[0].upgrade.credentials);
    EXPECT_EQ(nullptr, testData[0].upgrade.token);
    EXPECT_EQ(nullptr, testData[0].upgrade.environment);
    }

struct ArgumentParserTests_InvalidParameters : TestWithParam<TestArgs> {};
INSTANTIATE_TEST_CASE_P(NoParameters, ArgumentParserTests_InvalidParameters, ValuesIn(vector<TestArgs>{
        {"Foo.exe"},
    }));
INSTANTIATE_TEST_CASE_P(NoCreateCache, ArgumentParserTests_InvalidParameters, ValuesIn(vector<TestArgs>{
        {"Foo.exe", "--upgradecache", "-url", "URL2", "-r", "R2"},
        {"Foo.exe", "-url", "URL2", "-r", "R2"},
    }));
INSTANTIATE_TEST_CASE_P(NoUrl, ArgumentParserTests_InvalidParameters, ValuesIn(vector<TestArgs>{
        {"Foo.exe", "--createcache", "-r", "R"},
        {"Foo.exe", "--createcache", "-r", "R", "-token", GetTestTokenPath()},
        {"Foo.exe", "--createcache", "-r", "R", "-imsenv", "PROD"},
    }));
INSTANTIATE_TEST_CASE_P(NoRepoId, ArgumentParserTests_InvalidParameters, ValuesIn(vector<TestArgs>{
        {"Foo.exe", "--createcache", "-url", "URL"},
        {"Foo.exe", "--createcache", "-url", "URL", "-token", GetTestTokenPath()},
        {"Foo.exe", "--createcache", "-url", "URL", "-imsenv", "PROD"},
    }));
INSTANTIATE_TEST_CASE_P(NoUserOrPass, ArgumentParserTests_InvalidParameters, ValuesIn(vector<TestArgs>{
        {"Foo.exe", "--createcache", "-url", "URL", "-r", "R", "-auth:basic", "UserName"},
    }));
INSTANTIATE_TEST_CASE_P(InvalidAuth, ArgumentParserTests_InvalidParameters, ValuesIn(vector<TestArgs>{
        {"Foo.exe", "--createcache", "-url", "URL", "-r", "R", "-auth:foo", "A:B"},
        {"Foo.exe", "--createcache", "-url", "URL", "-r", "R", "-auth:qa:foo", "A:B"},
        {"Foo.exe", "--createcache", "-url", "URL", "-r", "R", "-auth:foo:ims", "A:B"},
        {"Foo.exe", "--createcache", "-url", "URL", "-r", "R", "-auth:foo:token", "A:B"},
        {"Foo.exe", "--createcache", "-url", "URL", "-r", "R", "-auth:qa:token", "NotTokenPath"},
    }));
INSTANTIATE_TEST_CASE_P(MissingValues, ArgumentParserTests_InvalidParameters, ValuesIn(vector<TestArgs>{
        {"Foo.exe", "--createcache", "-url", "-r", "R", "-auth:basic", "A:B"},
        {"Foo.exe", "--createcache", "-url", "URL", "-r", "-auth:basic", "A:B"},
        {"Foo.exe", "--createcache", "-url", "URL", "-r", "R", "-auth:basic"},
        {"Foo.exe", "--createcache", "-url", "URL", "-r", "R", "-auth:basic", "-imsenv"},
        {"Foo.exe", "--createcache", "-url", "URL", "-r", "R", "-auth:basic", "--upgradecache", "-url", "URL2", "-r", "R2", "-u", "U2", "-p", "P2"},
    }));
INSTANTIATE_TEST_CASE_P(CreateWithSchemasNonExistingDir, ArgumentParserTests_InvalidParameters, ValuesIn(vector<TestArgs>{
        {"Foo.exe", "--createcache", "-schemas", "C:\\nonexistingdir\\"},
        {"Foo.exe", "--createcache", "-schemas", "C:\\nonexistingdir"},
        {"Foo.exe", "--createcache", "-schemas", "foo_not_a_dir"},
        {"Foo.exe", "--createcache", "-schemas", GetTestTokenPath()},
    }));
INSTANTIATE_TEST_CASE_P(UknownParameters, ArgumentParserTests_InvalidParameters, ValuesIn(vector<TestArgs>{
        {"Foo.exe", "--foo", "--createcache", "-url", "URL", "-r", "R"},
        {"Foo.exe", "--createcache", "-url", "URL", "-r", "R", "-foo", "boo"},
    }));

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(ArgumentParserTests_InvalidParameters, Parse_InvalidParameters_RetrurnsAndPrintsErrorWithoutHelp)
    {
    auto args = GetParam();

    int logLevel;
    BeFileName workDir;
    bvector<TestRepositories> testData;
    std::stringstream err;
    std::stringstream out;

    ASSERT_NE(0, ArgumentParser::Parse((int) args.size(), args.data(), logLevel, workDir, testData, &err, &out));
    EXPECT_TRUE(workDir.empty());
    EXPECT_EQ(0, testData.size());
    EXPECT_NE("", err.str());
    EXPECT_EQ("", out.str());
    //BeDebugLog(err.str().c_str());
    //BeDebugLog(out.str().c_str());
    }

struct ArgumentParserTests_Help : TestWithParam<TestArgs> {};
INSTANTIATE_TEST_CASE_P(HelpSupplied, ArgumentParserTests_Help, ValuesIn(vector<TestArgs>{
        {"Foo.exe", "--help", "--silent"},
        {"Foo.exe", "--createcache", "-url", "URL", "--help", "RId"},
        {"Foo.exe", "--createcache", "-url", "URL", "-r", "RId", "--help"},
    }));

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(ArgumentParserTests_Help, Parse_HelpSupplied_PrintsHelpWithoutError)
    {
    auto args = GetParam();

    int logLevel;
    BeFileName workDir;
    bvector<TestRepositories> testData;
    std::stringstream err;
    std::stringstream out;

    EXPECT_EQ(0, ArgumentParser::Parse((int) args.size(), args.data(), logLevel, workDir, testData, &err, &out));
    EXPECT_EQ("", err.str());
    EXPECT_NE("", out.str());
    EXPECT_EQ(0, testData.size());
    EXPECT_EQ(1, logLevel);
    }

struct ArgumentParserTests_GTestArguments : TestWithParam<TestArgs> {};
INSTANTIATE_TEST_CASE_P(GTestArguments, ArgumentParserTests_GTestArguments, ValuesIn(vector<TestArgs>{
        {"Foo.exe", "--createcache", "-url", "URL", "-r", "RId", "--gtest_foo"},
        {"Foo.exe", "--gtest_foo", "--createcache", "-url", "URL", "-r", "RId"},
        {"Foo.exe", "--createcache", "-url", "URL", "--gtest_foo=*aa*", "-r", "RId"},
    }));

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(ArgumentParserTests_GTestArguments, Parse_GTestArguments_IgnoresThem)
    {
    auto args = GetParam();

    int logLevel;
    BeFileName workDir;
    bvector<TestRepositories> testData;
    std::stringstream err;
    std::stringstream out;

    EXPECT_EQ(0, ArgumentParser::Parse((int) args.size(), args.data(), logLevel, workDir, testData, &err, &out));
    EXPECT_EQ("", err.str());
    EXPECT_EQ("", out.str());
    EXPECT_EQ(1, testData.size());
    EXPECT_EQ(1, logLevel);
    }

#endif
