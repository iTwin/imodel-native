/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#define COMPILE_TESTS
#ifdef COMPILE_TESTS

#include "ArgumentParser.h"

#include <Bentley/BeTest.h>
#include <vector>
#include <Bentley/Base64Utilities.h>
#include <Bentley/BeDebugLog.h>
#include <BeSQLite/BeSQLite.h>

using namespace std;
using namespace testing;
USING_NAMESPACE_BENTLEY_SQLITE

struct TestArg
    {
    Utf8String value;
    Utf8String fileContent;
    bool isFolder = false;

    TestArg(Utf8CP value) : value(value) {};
    static TestArg File(Utf8String content);
    static TestArg Folder(Utf8String folderName);
    void Prepare() const;
    };

TestArg TestArg::File(Utf8String content)
    {
    BeFileName path;
    BeTest::GetHost().GetTempDir(path);
    path.AppendToPath(BeFileName(BeGuid(true).ToString()));

    TestArg arg(path.GetNameUtf8().c_str());
    arg.fileContent = content;
    return arg;
    }

TestArg TestArg::Folder(Utf8String folderName)
    {
    BeFileName path;
    BeTest::GetHost().GetTempDir(path);
    path.AppendToPath(BeFileName(folderName));

    TestArg arg(path.GetNameUtf8().c_str());
    arg.isFolder = true;
    return arg;
    }

void TestArg::Prepare() const
    {
    BeFileName path(value);

    if (isFolder)
        {
        if (!path.DoesPathExist())
            EXPECT_EQ(BeFileNameStatus::Success, BeFileName::CreateNewDirectory(path));
        return;
        }

    if (fileContent.empty())
        return;

    BeFileName::CreateNewDirectory(path.GetDirectoryName());

    BeFile file;
    file.Create(path);
    EXPECT_EQ(BeFileStatus::Success, file.Write(nullptr, fileContent.c_str(), (uint32_t) fileContent.size()));
    file.Close();
    }

struct TestArgs
    {
    private:
        vector<TestArg> args;

    public:
        size_t resultSize = -1;
        size_t inspectIndex = -1;

        std::shared_ptr<TestArgs> parentArgs;

    public:
        TestArgs(vector<TestArg> args) : args(args) {};
        TestArgs(size_t resultSize, size_t inspectIndex, vector<TestArg> args) :
            args(args), resultSize(resultSize), inspectIndex(inspectIndex) {};
        vector<char*> GetArgs() const;
        vector<char*> GetArgsNoPrepare() const;
    };

vector<char*> TestArgs::GetArgsNoPrepare() const
    {
    vector<char*> argsOut;
    for (auto& arg : args)
        argsOut.push_back((char*) arg.value.c_str());
    return argsOut;
    }

vector<char*> TestArgs::GetArgs() const
    {
    for (auto& arg : args)
        arg.Prepare();
    if (parentArgs)
        parentArgs->GetArgs();
    return GetArgsNoPrepare();
    }

vector<TestArgs> GenerateParams(vector<TestArgs> params)
    {
    vector<TestArgs> output = params;

    // Add duplicated --config test case for each param
    for (auto& param : params)
        {
        Utf8String content;

        auto args = param.GetArgsNoPrepare();
        for (int i = 1; i < args.size(); i++)
            {
            auto arg = args[i];
            content += arg;
            content += "\n";
            }
        TestArgs newParam({"Foo.exe", "--config", TestArg::File(content)});
        newParam.parentArgs = std::make_shared<TestArgs>(param);
        newParam.inspectIndex = param.inspectIndex;
        newParam.resultSize = param.resultSize;
        output.push_back(newParam);
        }

    return output;
    }

Utf8String s_xmlTokenStr =
R"(<saml:Assertion MajorVersion="1" MinorVersion="1" xmlns:saml="urn:oasis:names:tc:SAML:1.0:assertion">
            <saml:Conditions 
                NotBefore   ="2000-01-01T00:00:00.000Z"
                NotOnOrAfter="2000-01-03T00:00:00.000Z">
            </saml:Conditions>
        </saml:Assertion>)";

void PrintTo(const TestArgs& value, ::std::ostream* os)
    {
    for (auto arg : value.GetArgsNoPrepare())
        *os << Utf8PrintfString("%s ", arg);
    }

struct ArgumentParserTests : Test {};

struct ArgumentParserTests_TempDirArg : TestWithParam<TestArgs> {};
INSTANTIATE_TEST_CASE_P(, ArgumentParserTests_TempDirArg, ValuesIn(GenerateParams(vector<TestArgs>{
        {{"Foo.exe", "--workdir", "TestDir\\TestFolder", "--createcache", "-url", "URL", "-r", "RId"}},
        {{"Foo.exe", "--createcache", "-url", "URL", "-r", "RId", "--workdir", "TestDir\\TestFolder"}},
    })));

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(ArgumentParserTests_TempDirArg, Parse_ValidParameters_FillsTestDataAndReturnsZero)
    {
    auto args = GetParam().GetArgs();

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
    EXPECT_EQ(BeVersion(), testData[0].create.serviceVersion);
    EXPECT_EQ(Credentials(), testData[0].create.credentials);
    EXPECT_EQ(nullptr, testData[0].create.token);
    EXPECT_EQ(nullptr, testData[0].create.environment);
    }

struct ArgumentParserTests_SilentProvided : TestWithParam<TestArgs> {};
INSTANTIATE_TEST_CASE_P(, ArgumentParserTests_SilentProvided, ValuesIn(GenerateParams(vector<TestArgs>{
        {{"Foo.exe", "--silent", "--createcache", "-url", "URL", "-r", "RId"}},
        {{"Foo.exe", "--createcache", "-url", "URL", "-r", "RId", "--silent"}},
    })));

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(ArgumentParserTests_SilentProvided, Parse_Silent_LogLevelZero)
    {
    auto args = GetParam().GetArgs();

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
    TestArgs testArgs({"Foo.exe", "--createcache", "-url", "URL", "-r", "RId"});
    auto args = testArgs.GetArgs();

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

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ArgumentParserTests, Parse_ValidateCertificatesNotProvided_SetsValidateCertificatesToTrue)
    {
    TestArgs testArgs({"Foo.exe", "--createcache", "-url", "URL", "-r", "RId"});
    auto args = testArgs.GetArgs();

    int logLevel;
    BeFileName workDir;
    bvector<TestRepositories> testData;
    std::stringstream err;
    std::stringstream out;

    EXPECT_EQ(0, ArgumentParser::Parse((int) args.size(), args.data(), logLevel, workDir, testData, &err, &out));
    EXPECT_EQ("", err.str());
    EXPECT_EQ("", out.str());
    EXPECT_EQ(1, logLevel);

    ASSERT_EQ(1, testData.size());

    EXPECT_EQ(true, testData[0].create.validateCertificate);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ArgumentParserTests, Parse_ValidateCertificatesFalse_SetsValidateCertificatesToFalse)
    {
    TestArgs testArgs({"Foo.exe", "--createcache", "-url", "URL", "-r", "RId", "-validateCertificates", "FaLsE"});
    auto args = testArgs.GetArgs();

    int logLevel;
    BeFileName workDir;
    bvector<TestRepositories> testData;
    std::stringstream err;
    std::stringstream out;

    EXPECT_EQ(0, ArgumentParser::Parse((int) args.size(), args.data(), logLevel, workDir, testData, &err, &out));
    EXPECT_EQ("", err.str());
    EXPECT_EQ("", out.str());
    EXPECT_EQ(1, logLevel);

    ASSERT_EQ(1, testData.size());

    EXPECT_EQ(false, testData[0].create.validateCertificate);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ArgumentParserTests, Parse_ValidateCertificatesTrue_SetsValidateCertificatesToTrue)
    {
    TestArgs testArgs({"Foo.exe", "--createcache", "-url", "URL", "-r", "RId", "-validateCertificates", "tRuE"});
    auto args = testArgs.GetArgs();

    int logLevel;
    BeFileName workDir;
    bvector<TestRepositories> testData;
    std::stringstream err;
    std::stringstream out;

    EXPECT_EQ(0, ArgumentParser::Parse((int) args.size(), args.data(), logLevel, workDir, testData, &err, &out));
    EXPECT_EQ("", err.str());
    EXPECT_EQ("", out.str());
    EXPECT_EQ(1, logLevel);

    ASSERT_EQ(1, testData.size());

    EXPECT_EQ(true, testData[0].create.validateCertificate);
    }

struct ArgumentParserTests_DownloadSchemas : TestWithParam<TestArgs> {};
INSTANTIATE_TEST_CASE_P(DownloadSchemasOnly, ArgumentParserTests_DownloadSchemas, ValuesIn(GenerateParams(vector<TestArgs>{
        {1, 0, {"Foo.exe", "--downloadschemas", "-url", "URL", "-r", "RId"}},
        {1, 0, {"Foo.exe", "--downloadschemas", "-r", "RId", "-url", "URL"}},
    })));
INSTANTIATE_TEST_CASE_P(DownloadSchemasWithCreate, ArgumentParserTests_DownloadSchemas, ValuesIn(GenerateParams(vector<TestArgs>{
        {2, 1, {"Foo.exe", "--createcache", "-url", "URLX", "-r", "RIdX", "--downloadschemas", "-url", "URL", "-r", "RId"}},
        {2, 0, {"Foo.exe", "--downloadschemas", "-url", "URL", "-r", "RId", "--createcache", "-url", "URLX", "-r", "RIdX"}},
        {3, 1, {"Foo.exe", "--createcache", "-url", "URLX", "-r", "RIdX", "--downloadschemas", "-url", "URL", "-r", "RId", "--createcache", "-url", "URLX", "-r", "RIdX"}},
    })));

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(ArgumentParserTests_DownloadSchemas, Parse_ValidParameters_FillsTestDataAndReturnsZero)
    {
    auto param = GetParam();
    auto args = param.GetArgs();

    int logLevel;
    BeFileName workDir;
    bvector<TestRepositories> testData;
    std::stringstream err;
    std::stringstream out;

    EXPECT_EQ(0, ArgumentParser::Parse((int) args.size(), args.data(), logLevel, workDir, testData, &err, &out));
    EXPECT_TRUE(workDir.empty());
    EXPECT_EQ("", err.str());
    EXPECT_EQ("", out.str());

    ASSERT_EQ(param.resultSize, testData.size());

    size_t it = param.inspectIndex;
    EXPECT_EQ("URL", testData[it].downloadSchemas.serverUrl);
    EXPECT_EQ("RId", testData[it].downloadSchemas.id);
    EXPECT_EQ(BeVersion(), testData[it].downloadSchemas.serviceVersion);
    EXPECT_EQ(Credentials(), testData[it].downloadSchemas.credentials);
    EXPECT_EQ(nullptr, testData[it].downloadSchemas.token);
    EXPECT_EQ(nullptr, testData[it].downloadSchemas.environment);
    }

struct ArgumentParserTests_CreateWithoutCredentials : TestWithParam<TestArgs> {};
INSTANTIATE_TEST_CASE_P(, ArgumentParserTests_CreateWithoutCredentials, ValuesIn(GenerateParams(vector<TestArgs>{
        {{"Foo.exe", "--createcache", "-url", "URL", "-r", "RId"}},
        {{"Foo.exe", "--createcache", "-r", "RId", "-url", "URL"}},
    })));

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(ArgumentParserTests_CreateWithoutCredentials, Parse_ValidParameters_FillsTestDataAndReturnsZero)
    {
    auto args = GetParam().GetArgs();

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
    EXPECT_EQ(BeVersion(), testData[0].create.serviceVersion);
    EXPECT_EQ(Credentials(), testData[0].create.credentials);
    EXPECT_EQ(nullptr, testData[0].create.token);
    EXPECT_EQ(nullptr, testData[0].create.environment);
    }

struct ArgumentParserTests_CreateWithLabelAndComment : TestWithParam<TestArgs> {};
INSTANTIATE_TEST_CASE_P(, ArgumentParserTests_CreateWithLabelAndComment, ValuesIn(GenerateParams(vector<TestArgs>{
        {{"Foo.exe", "--createcache", "-c", "comment", "-url", "URL", "-l", "label", "-r", "RId"}},
        {{"Foo.exe", "--createcache", "-l", "label", "-r", "RId", "-url", "URL", "-c", "comment"}},
    })));

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(ArgumentParserTests_CreateWithLabelAndComment, Parse_ValidParameters_FillsTestDataAndReturnsZero)
    {
    auto args = GetParam().GetArgs();

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
    EXPECT_EQ(BeVersion(), testData[0].create.serviceVersion);
    EXPECT_EQ(Credentials(), testData[0].create.credentials);
    EXPECT_EQ(nullptr, testData[0].create.token);
    EXPECT_EQ(nullptr, testData[0].create.environment);

    EXPECT_EQ("label", testData[0].create.label);
    EXPECT_EQ("comment", testData[0].create.comment);
    }

struct ArgumentParserTests_CreateWithServiceVersion : TestWithParam<TestArgs> {};
INSTANTIATE_TEST_CASE_P(, ArgumentParserTests_CreateWithServiceVersion, ValuesIn(GenerateParams(vector<TestArgs>{
        {{"Foo.exe", "--createcache", "-sv", "4.2", "-url", "URL", "-l", "label", "-r", "RId"}},
        {{"Foo.exe", "--createcache", "-l", "label", "-r", "RId", "-url", "URL", "-sv", "4.2"}}
    })));

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(ArgumentParserTests_CreateWithServiceVersion, Parse_ValidParameters_FillsTestDataAndReturnsZero)
    {
    auto args = GetParam().GetArgs();

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
    EXPECT_EQ(BeVersion(4, 2), testData[0].create.serviceVersion);
    EXPECT_EQ(Credentials(), testData[0].create.credentials);
    EXPECT_EQ(nullptr, testData[0].create.token);
    EXPECT_EQ(nullptr, testData[0].create.environment);
    }

struct ArgumentParserTests_CreateWithConfig : TestWithParam<TestArgs> {};
INSTANTIATE_TEST_CASE_P(, ArgumentParserTests_CreateWithConfig, ValuesIn(GenerateParams(vector<TestArgs>{
        {1, 0, {"Foo.exe", "--config", TestArg::File("--createcache\n-url URL\n-r RId\n-c Comment Here")}},
        {1, 0, {"Foo.exe", "--config", TestArg::File("--createcache\n-url URL\n#This is commented out\n-r RId\n-c Comment Here")}},
        {1, 0, {"Foo.exe", "--config", TestArg::File("  --createcache  \n-url    URL\n   #This is commented out   \n\n  -r   RId\n  -c Comment Here")}},
        {1, 0, {"Foo.exe", "--config", TestArg::File("\n \n \n--createcache\n-url URL\n-r RId\n-c Comment Here  \n\n")}},
        {1, 0, {"Foo.exe", "--config", TestArg::File("--createcache\n-url URL\n\n\n-r RId\n\n-c Comment Here")}},
    })));

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(ArgumentParserTests_CreateWithConfig, Parse_ValidParameters_FillsTestDataAndReturnsZero)
    {
    auto args = GetParam().GetArgs();

    int logLevel;
    BeFileName workDir;
    bvector<TestRepositories> testData;
    std::stringstream err;
    std::stringstream out;

    EXPECT_EQ(0, ArgumentParser::Parse((int) args.size(), args.data(), logLevel, workDir, testData, &err, &out));
    EXPECT_TRUE(workDir.empty());
    EXPECT_EQ("", err.str());
    EXPECT_EQ("", out.str());

    ASSERT_EQ(GetParam().resultSize, testData.size());

    auto it = GetParam().inspectIndex;
    EXPECT_FALSE(testData[it].upgrade.IsValid());
    EXPECT_TRUE(testData[it].create.IsValid());

    EXPECT_EQ("URL", testData[it].create.serverUrl);
    EXPECT_EQ("RId", testData[it].create.id);
    EXPECT_EQ(BeVersion(), testData[it].create.serviceVersion);
    EXPECT_EQ(Credentials(), testData[it].create.credentials);
    EXPECT_EQ(nullptr, testData[it].create.token);
    EXPECT_EQ(nullptr, testData[it].create.environment);
    EXPECT_EQ("Comment Here", testData[it].create.comment);
    }

struct ArgumentParserTests_CreateWithBasicAuth : TestWithParam<TestArgs> {};
INSTANTIATE_TEST_CASE_P(, ArgumentParserTests_CreateWithBasicAuth, ValuesIn(GenerateParams(vector<TestArgs>{
        {{"Foo.exe", "--createcache", "-url", "URL", "-r", "RId", "-auth:basic", "TestUser:TestPass"}},
        {{"Foo.exe", "--createcache", "-r", "RId", "-auth:basic", "TestUser:TestPass", "-url", "URL"}},
    })));

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(ArgumentParserTests_CreateWithBasicAuth, Parse_ValidParameters_FillsTestDataAndReturnsZero)
    {
    auto args = GetParam().GetArgs();

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
    EXPECT_EQ(BeVersion(), testData[0].create.serviceVersion);
    EXPECT_EQ(Credentials("TestUser", "TestPass"), testData[0].create.credentials);
    EXPECT_EQ(nullptr, testData[0].create.token);
    EXPECT_EQ(nullptr, testData[0].create.environment);
    }

struct ArgumentParserTests_CreateWithImsAuth : TestWithParam<pair<TestArgs, UrlProvider::Environment>> {};
INSTANTIATE_TEST_CASE_P(, ArgumentParserTests_CreateWithImsAuth, ValuesIn(vector<pair<TestArgs, UrlProvider::Environment>>{
        { {{"Foo.exe", "--createcache", "-url", "URL", "-r", "RId", "-auth:dev:ims", "TestUser:TestPass"}}, UrlProvider::Environment::Dev},
        {{{"Foo.exe", "--createcache", "-url", "URL", "-r", "RId", "-auth:qa:ims", "TestUser:TestPass"}}, UrlProvider::Environment::Qa},
        {{{"Foo.exe", "--createcache", "-url", "URL", "-r", "RId", "-auth:prod:ims", "TestUser:TestPass"}}, UrlProvider::Environment::Release},
    }));

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(ArgumentParserTests_CreateWithImsAuth, Parse_ValidParameters_FillsTestDataAndReturnsZero)
    {
    auto param = GetParam();
    auto args = param.first.GetArgs();

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
    EXPECT_EQ(BeVersion(), testData[0].create.serviceVersion);
    EXPECT_EQ(Credentials("TestUser", "TestPass"), testData[0].create.credentials);
    EXPECT_EQ(nullptr, testData[0].create.token);

    ASSERT_NE(nullptr, testData[0].create.environment);
    EXPECT_EQ(param.second, *testData[0].create.environment);
    }

struct ArgumentParserTests_CreateWithImsTokenAuth : TestWithParam<pair<TestArgs, UrlProvider::Environment>> {};
INSTANTIATE_TEST_CASE_P(, ArgumentParserTests_CreateWithImsTokenAuth, ValuesIn(vector<pair<TestArgs, UrlProvider::Environment>>{
        { {{"Foo.exe", "--createcache", "-url", "URL", "-r", "RId", "-auth:dev:token", TestArg::File(s_xmlTokenStr)}}, UrlProvider::Environment::Dev},
        {{{"Foo.exe", "--createcache", "-url", "URL", "-r", "RId", "-auth:qa:token", TestArg::File(s_xmlTokenStr)}}, UrlProvider::Environment::Qa},
        {{{"Foo.exe", "--createcache", "-url", "URL", "-r", "RId", "-auth:prod:token", TestArg::File(s_xmlTokenStr)}}, UrlProvider::Environment::Release},
    }));

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(ArgumentParserTests_CreateWithImsTokenAuth, Parse_ValidParameters_FillsTestDataAndReturnsZero)
    {
    auto param = GetParam();
    auto args = param.first.GetArgs();

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
    EXPECT_EQ(BeVersion(), testData[0].create.serviceVersion);
    EXPECT_EQ(Credentials(), testData[0].create.credentials);
    EXPECT_STREQ(s_xmlTokenStr.c_str(), testData[0].create.token->AsString().c_str());
    EXPECT_FALSE(testData[0].create.tokenPath.empty());

    ASSERT_NE(nullptr, testData[0].create.environment);
    EXPECT_EQ(param.second, *testData[0].create.environment);
    }

struct ArgumentParserTests_CreateAndUpgradeWithBasicAuth : TestWithParam<TestArgs> {};
INSTANTIATE_TEST_CASE_P(, ArgumentParserTests_CreateAndUpgradeWithBasicAuth, ValuesIn(GenerateParams(vector<TestArgs>{
        {{"Foo.exe", "--createcache", "-url", "URL1", "-r", "R1", "-auth:basic", "U1:P1", "--upgradecache", "-url", "URL2", "-r", "R2", "-auth:basic", "U2:P2"}},
        {{"Foo.exe", "--createcache", "-auth:basic", "U1:P1", "-url", "URL1", "-r", "R1", "--upgradecache", "-r", "R2", "-auth:basic", "U2:P2", "-url", "URL2"}},
    })));

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(ArgumentParserTests_CreateAndUpgradeWithBasicAuth, Parse_ValidParameters_FillsTestDataAndReturnsZero)
    {
    auto args = GetParam().GetArgs();

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
    EXPECT_EQ(BeVersion(), testData[0].create.serviceVersion);
    EXPECT_EQ(Credentials("U1", "P1"), testData[0].create.credentials);
    EXPECT_EQ(nullptr, testData[0].create.token);
    EXPECT_EQ(nullptr, testData[0].create.environment);

    EXPECT_EQ("URL2", testData[0].upgrade.serverUrl);
    EXPECT_EQ("R2", testData[0].upgrade.id);
    EXPECT_EQ(BeVersion(), testData[0].upgrade.serviceVersion);
    EXPECT_EQ(Credentials("U2", "P2"), testData[0].upgrade.credentials);
    EXPECT_EQ(nullptr, testData[0].upgrade.token);
    EXPECT_EQ(nullptr, testData[0].upgrade.environment);
    }

struct ArgumentParserTests_CreateAndUpgradeWithDefaultParams : TestWithParam<TestArgs> {};
INSTANTIATE_TEST_CASE_P(, ArgumentParserTests_CreateAndUpgradeWithDefaultParams, ValuesIn(GenerateParams(vector<TestArgs>{
        {1, 0, {"Foo.exe", "--createcache", "-url", "URL1", "-r", "R2", "-l", "LABEL", "-auth:basic", "U2:P2", "--upgradecache", "-url", "URL2"}},
        {1, 0, {"Foo.exe", "--createcache", "-url", "URL2", "-r", "R1", "-l", "LABEL", "-auth:basic", "U2:P2", "--upgradecache", "-r", "R2"}},
        {1, 0, {"Foo.exe", "--createcache", "-url", "URL2", "-r", "R2", "-l", "LABEL", "-auth:basic", "U1:P1", "--upgradecache", "-auth:basic", "U2:P2"}},
        {1, 0, {"Foo.exe", "--createcache", "-url", "URL2", "-r", "R2", "-l", "LABEL", "-auth:qa:ims", "U1:P1", "--upgradecache", "-auth:basic", "U2:P2"}},
        {1, 0, {"Foo.exe", "--createcache", "-url", "URL2", "-r", "R2", "-l", "LABEL", "-auth:qa:token", TestArg::File(s_xmlTokenStr), "--upgradecache", "-auth:basic", "U2:P2"}},
    })));
INSTANTIATE_TEST_CASE_P(DownloadSchemasInvolved, ArgumentParserTests_CreateAndUpgradeWithDefaultParams, ValuesIn(GenerateParams(vector<TestArgs>{
        {2, 0, {"Foo.exe", "--createcache", "-url", "URL1", "-r", "R2", "-auth:basic", "U2:P2", "-l", "LABEL", "--upgradecache", "-url", "URL2", "--downloadschemas", "-url", "URLX", "-r", "RX"}},
        {2, 1, {"Foo.exe", "--downloadschemas", "-url", "URLX", "-r", "RX", "--createcache", "-url", "URL1", "-r", "R2", "-auth:basic", "U2:P2", "-l", "LABEL", "--upgradecache", "-url", "URL2"}},
    })));

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(ArgumentParserTests_CreateAndUpgradeWithDefaultParams, Parse_UpgradeCacheDefaultsParametersFromCreateCache_FillsTestDataAndReturnsZero)
    {
    auto param = GetParam();
    auto args = param.GetArgs();

    int logLevel;
    BeFileName workDir;
    bvector<TestRepositories> testData;
    std::stringstream err;
    std::stringstream out;

    EXPECT_EQ(0, ArgumentParser::Parse((int) args.size(), args.data(), logLevel, workDir, testData, &err, &out));
    EXPECT_TRUE(workDir.empty());
    EXPECT_EQ("", err.str());
    EXPECT_EQ("", out.str());

    ASSERT_EQ(param.resultSize, testData.size());

    size_t it = param.inspectIndex;

    EXPECT_TRUE(testData[it].create.IsValid());
    EXPECT_TRUE(testData[it].upgrade.IsValid());

    EXPECT_EQ("URL2", testData[it].upgrade.serverUrl);
    EXPECT_EQ("R2", testData[it].upgrade.id);
    EXPECT_EQ(BeVersion(), testData[it].upgrade.serviceVersion);
    EXPECT_EQ(Credentials("U2", "P2"), testData[it].upgrade.credentials);
    EXPECT_EQ(nullptr, testData[it].upgrade.token);
    EXPECT_EQ(nullptr, testData[it].upgrade.environment);
    EXPECT_EQ("LABEL", testData[it].upgrade.label);
    }

struct ArgumentParserTests_CreateWithSchemas : TestWithParam<TestArgs> {};
INSTANTIATE_TEST_CASE_P(, ArgumentParserTests_CreateWithSchemas, ValuesIn(GenerateParams(vector<TestArgs>{
        {{"Foo.exe", "--createcache", "-schemas", TestArg::Folder("Test")}},
        {{"Foo.exe", "--createcache", "-schemas", TestArg::Folder("Test/")}},
    })));

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(ArgumentParserTests_CreateWithSchemas, Parse_ValidParameters_FillsTestDataAndReturnsZero)
    {
    auto args = GetParam().GetArgs();

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

    EXPECT_STREQ(BeFileName(TestArg::Folder("Test/").value).c_str(), testData[0].create.schemasDir.c_str());
    EXPECT_EQ("", testData[0].create.serverUrl);
    EXPECT_EQ("", testData[0].create.id);
    EXPECT_EQ(BeVersion(), testData[0].create.serviceVersion);
    EXPECT_EQ(Credentials(), testData[0].create.credentials);
    EXPECT_EQ(nullptr, testData[0].create.token);
    EXPECT_EQ(nullptr, testData[0].create.environment);
    }

struct ArgumentParserTests_UpgradeFromSchemas : TestWithParam<TestArgs> {};
INSTANTIATE_TEST_CASE_P(, ArgumentParserTests_UpgradeFromSchemas, ValuesIn(GenerateParams(vector<TestArgs>{
        {{"Foo.exe", "--createcache", "-schemas", TestArg::Folder("Test"), "--upgradecache", "-url", "URL", "-r", "R"}},
        {{"Foo.exe", "--createcache", "-schemas", TestArg::Folder("Test/"), "--upgradecache", "-url", "URL", "-r", "R"}},
    })));

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(ArgumentParserTests_UpgradeFromSchemas, Parse_ValidParameters_FillsTestDataAndReturnsZero)
    {
    auto args = GetParam().GetArgs();

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

    EXPECT_STREQ(BeFileName(TestArg::Folder("Test/").value).c_str(), testData[0].create.schemasDir.c_str());
    EXPECT_EQ("", testData[0].create.serverUrl);
    EXPECT_EQ("", testData[0].create.id);
    EXPECT_EQ(BeVersion(), testData[0].create.serviceVersion);
    EXPECT_EQ(Credentials(), testData[0].create.credentials);
    EXPECT_EQ(nullptr, testData[0].create.token);
    EXPECT_EQ(nullptr, testData[0].create.environment);

    EXPECT_STREQ(L"", testData[0].upgrade.schemasDir.c_str());
    EXPECT_EQ("URL", testData[0].upgrade.serverUrl);
    EXPECT_EQ("R", testData[0].upgrade.id);
    EXPECT_EQ(BeVersion(), testData[0].upgrade.serviceVersion);
    EXPECT_EQ(Credentials(), testData[0].upgrade.credentials);
    EXPECT_EQ(nullptr, testData[0].upgrade.token);
    EXPECT_EQ(nullptr, testData[0].upgrade.environment);
    }

struct ArgumentParserTests_UpgradeToSchemas : TestWithParam<TestArgs> {};
INSTANTIATE_TEST_CASE_P(, ArgumentParserTests_UpgradeToSchemas, ValuesIn(GenerateParams(vector<TestArgs>{
        {{"Foo.exe", "--createcache", "-url", "URL", "-r", "R", "--upgradecache", "-schemas", TestArg::Folder("Test/")}},
    })));

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(ArgumentParserTests_UpgradeToSchemas, Parse_ValidParameters_FillsTestDataAndReturnsZero)
    {
    auto args = GetParam().GetArgs();

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
    EXPECT_EQ(BeVersion(), testData[0].create.serviceVersion);
    EXPECT_EQ(Credentials(), testData[0].create.credentials);
    EXPECT_EQ(nullptr, testData[0].create.token);
    EXPECT_EQ(nullptr, testData[0].create.environment);

    EXPECT_STREQ(BeFileName(TestArg::Folder("Test/").value).c_str(), testData[0].upgrade.schemasDir.c_str());
    EXPECT_EQ("", testData[0].upgrade.serverUrl);
    EXPECT_EQ("", testData[0].upgrade.id);
    EXPECT_EQ(Credentials(), testData[0].upgrade.credentials);
    EXPECT_EQ(nullptr, testData[0].upgrade.token);
    EXPECT_EQ(nullptr, testData[0].upgrade.environment);
    }

struct ArgumentParserTests_InvalidParameters : TestWithParam<TestArgs> {};
INSTANTIATE_TEST_CASE_P(NoParameters, ArgumentParserTests_InvalidParameters, ValuesIn(GenerateParams(vector<TestArgs>{
        {{"Foo.exe"}},
    })));
INSTANTIATE_TEST_CASE_P(NoCreateCache, ArgumentParserTests_InvalidParameters, ValuesIn(GenerateParams(vector<TestArgs>{
        {{"Foo.exe", "--upgradecache", "-url", "URL2", "-r", "R2"}},
        {{"Foo.exe", "--downloadschemas", "-url", "URL1", "-r", "R1", "--upgradecache", "-url", "URL2", "-r", "R2"}},
        {{"Foo.exe", "-url", "URL2", "-r", "R2"}},
    })));
INSTANTIATE_TEST_CASE_P(NoUrl, ArgumentParserTests_InvalidParameters, ValuesIn(GenerateParams(vector<TestArgs>{
        {{"Foo.exe", "--createcache", "-r", "R"}},
        {{"Foo.exe", "--createcache", "-r", "R", "-token", TestArg::File(s_xmlTokenStr)}},
        {{"Foo.exe", "--createcache", "-r", "R", "-imsenv", "PROD"}},
    })));
INSTANTIATE_TEST_CASE_P(NoRepoId, ArgumentParserTests_InvalidParameters, ValuesIn(GenerateParams(vector<TestArgs>{
        {{"Foo.exe", "--createcache", "-url", "URL"}},
        {{"Foo.exe", "--createcache", "-url", "URL", "-token", TestArg::File(s_xmlTokenStr)}},
        {{"Foo.exe", "--createcache", "-url", "URL", "-imsenv", "PROD"}},
    })));
INSTANTIATE_TEST_CASE_P(InvalidServiceVersion, ArgumentParserTests_InvalidParameters, ValuesIn(GenerateParams(vector<TestArgs>{
        {{"Foo.exe", "--createcache", "-url", "URL", "-r", "RId", "-sv", "nonVersion"}},
        {{"Foo.exe", "--createcache", "-url", "URL", "-r", "RId", "-sv", "1.2.3.4"}},
        {{"Foo.exe", "--createcache", "-url", "URL", "-r", "RId", "-sv", "1.2.3"}},
        {{"Foo.exe", "--createcache", "-url", "URL", "-r", "RId", "-sv", "1.2t"}},
        {{"Foo.exe", "--createcache", "-url", "URL", "-r", "RId", "-sv", "t1.2"}},
        {{"Foo.exe", "--createcache", "-url", "URL", "-r", "RId", "-sv", "1"}},
        {{"Foo.exe", "--createcache", "-url", "URL", "-r", "RId", "-sv"}},
    })));
INSTANTIATE_TEST_CASE_P(NoUserOrPass, ArgumentParserTests_InvalidParameters, ValuesIn(GenerateParams(vector<TestArgs>{
        {{"Foo.exe", "--createcache", "-url", "URL", "-r", "R", "-auth:basic", "UserName"}},
    })));
INSTANTIATE_TEST_CASE_P(InvalidAuth, ArgumentParserTests_InvalidParameters, ValuesIn(GenerateParams(vector<TestArgs>{
        {{"Foo.exe", "--createcache", "-url", "URL", "-r", "R", "-auth:foo", "A:B"}},
        {{"Foo.exe", "--createcache", "-url", "URL", "-r", "R", "-auth:qa:foo", "A:B"}},
        {{"Foo.exe", "--createcache", "-url", "URL", "-r", "R", "-auth:foo:ims", "A:B"}},
        {{"Foo.exe", "--createcache", "-url", "URL", "-r", "R", "-auth:foo:token", "A:B"}},
        {{"Foo.exe", "--createcache", "-url", "URL", "-r", "R", "-auth:qa:token", "NotTokenPath"}},
    })));
INSTANTIATE_TEST_CASE_P(MissingValues, ArgumentParserTests_InvalidParameters, ValuesIn(GenerateParams(vector<TestArgs>{
        {{"Foo.exe", "--createcache", "-url", "-r", "R", "-auth:basic", "A:B"}},
        {{"Foo.exe", "--createcache", "-url", "URL", "-r", "-auth:basic", "A:B"}},
        {{"Foo.exe", "--createcache", "-url", "URL", "-r", "R", "-auth:basic"}},
        {{"Foo.exe", "--createcache", "-url", "URL", "-r", "R", "-auth:basic", "-imsenv"}},
        {{"Foo.exe", "--createcache", "-url", "URL", "-r", "R", "-auth:basic", "--upgradecache", "-url", "URL2", "-r", "R2", "-u", "U2", "-p", "P2"}},
    })));
INSTANTIATE_TEST_CASE_P(CreateWithSchemasNonExistingDir, ArgumentParserTests_InvalidParameters, ValuesIn(GenerateParams(vector<TestArgs>{
        {{"Foo.exe", "--createcache", "-schemas", "C:\\nonexistingdir\\"}},
        {{"Foo.exe", "--createcache", "-schemas", "C:\\nonexistingdir"}},
        {{"Foo.exe", "--createcache", "-schemas", "foo_not_a_dir"}},
        {{"Foo.exe", "--createcache", "-schemas", TestArg::File(s_xmlTokenStr)}},
    })));
INSTANTIATE_TEST_CASE_P(UknownParameters, ArgumentParserTests_InvalidParameters, ValuesIn(GenerateParams(vector<TestArgs>{
        {{"Foo.exe", "--foo", "--createcache", "-url", "URL", "-r", "R"}},
        {{"Foo.exe", "--createcache", "-url", "URL", "-r", "R", "-foo", "boo"}},
    })));

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(ArgumentParserTests_InvalidParameters, Parse_InvalidParameters_RetrurnsAndPrintsErrorWithoutHelp)
    {
    auto args = GetParam().GetArgs();

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
        {{"Foo.exe", "--help", "--silent"}},
        {{"Foo.exe", "--createcache", "-url", "URL", "--help", "RId"}},
        {{"Foo.exe", "--createcache", "-url", "URL", "-r", "RId", "--help"}},
    }));

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(ArgumentParserTests_Help, Parse_HelpSupplied_PrintsHelpWithoutError)
    {
    auto args = GetParam().GetArgs();

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
INSTANTIATE_TEST_CASE_P(GTestArguments, ArgumentParserTests_GTestArguments, ValuesIn(GenerateParams(vector<TestArgs>{
        {{"Foo.exe", "--createcache", "-url", "URL", "-r", "RId", "--gtest_foo"}},
        {{"Foo.exe", "--gtest_foo", "--createcache", "-url", "URL", "-r", "RId"}},
        {{"Foo.exe", "--createcache", "-url", "URL", "--gtest_foo=*aa*", "-r", "RId"}},
    })));

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_P(ArgumentParserTests_GTestArguments, Parse_GTestArguments_IgnoresThem)
    {
    auto args = GetParam().GetArgs();

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
