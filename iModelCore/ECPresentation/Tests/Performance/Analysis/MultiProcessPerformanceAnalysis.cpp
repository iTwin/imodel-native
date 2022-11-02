/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentation/Content.h>
#include <ECPresentation/ECPresentationManager.h>
#include "HierarchyPerformanceAnalysis.h"
#include "../PerformanceTests.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

#define NODES_PAGE_SIZE 20

#define RULESET_FILE_NAME L"ModelCategoryElement.json"
#define CONFIG_FILE_NAME  L"MultiProcessAnalysisConfig.json"

// file name of the imodel to run analysis without setting up configuration file
#define DEFAULT_PROJECT_FILE_NAME ""

#define REPORTER_FIELD_Configuration "Config"
#define REPORTER_FIELD_Manager       "Manager"

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct TestCase
    {
    size_t m_processesCount;
    unsigned m_threadsCount;

    TestCase(size_t processesCount, unsigned threadsCount) : m_processesCount(processesCount), m_threadsCount(threadsCount) {}
    };

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct MultiProcessPerformanceAnalysisConfigutration
    {
    Utf8String m_projectFileName;
    bvector<TestCase> m_testCases;
    };

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct AggregateMultiProcessPerformanceMetricsStorage
{
    HierarchyPagedLoadPerformanceMetricsStorage m_warmCacheMetrics;
    HierarchyPagedLoadPerformanceMetricsStorage m_coldCacheMetrics;
    HierarchyNonPagedLoadPerformanceMetricsStorage m_nonPagedMetrics;

    AggregateMultiProcessPerformanceMetricsStorage(int sourceCount)
        : m_warmCacheMetrics(HierarchyCacheState::Warm, sourceCount), m_coldCacheMetrics(HierarchyCacheState::Cold, sourceCount), m_nonPagedMetrics(sourceCount)
        {}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct MultiProcessNonPagedLoadPerformanceMetricsStorage
{
private:
    HierarchyNonPagedLoadPerformanceMetricsStorage& m_aggregateMetricsStorage;
    HierarchyNonPagedLoadPerformanceMetricsStorage m_currentMetricsStorage;

public:
    MultiProcessNonPagedLoadPerformanceMetricsStorage(HierarchyNonPagedLoadPerformanceMetricsStorage& aggregateMetrics)
        : m_aggregateMetricsStorage(aggregateMetrics), m_currentMetricsStorage()
        {}

    void ReportTotalTime(double totalTime)
        {
        m_aggregateMetricsStorage.ReportTotalTime(totalTime);
        m_currentMetricsStorage.ReportTotalTime(totalTime);
        }

    void ReportTotalNodesCount(size_t nodesCount)
        {
        m_aggregateMetricsStorage.ReportTotalNodesCount(nodesCount);
        m_currentMetricsStorage.ReportTotalNodesCount(nodesCount);
        }

    void Save(Reporter& reporter) const
        {
        m_currentMetricsStorage.Save(reporter);
        }
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct MultiProcessPagedLoadPerformanceMetricsStorage
{
private:
    HierarchyPagedLoadPerformanceMetricsStorage& m_aggregateMetricsStorage;
    HierarchyPagedLoadPerformanceMetricsStorage m_currentMetricsStorage;

public:
    MultiProcessPagedLoadPerformanceMetricsStorage(HierarchyPagedLoadPerformanceMetricsStorage& aggregateMetrics, HierarchyCacheState cacheState)
        : m_aggregateMetricsStorage(aggregateMetrics), m_currentMetricsStorage(cacheState)
        {}

    void ReportPageLoad(int pageIndex, double pageTime, double countTime)
        {
        m_aggregateMetricsStorage.ReportPageLoad(pageIndex, pageTime, countTime);
        m_currentMetricsStorage.ReportPageLoad(pageIndex, pageTime, countTime);
        }

    void ReportTotalTime(double time)
        {
        m_aggregateMetricsStorage.ReportTotalTime(time);
        m_currentMetricsStorage.ReportTotalTime(time);
        }

    void Save(Reporter& reporter) const
        {
        m_currentMetricsStorage.Save(reporter);
        }
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ManagerPerformanceMetricsStorage
    {
    int m_id;
    std::unique_ptr<ECPresentationManager> m_manager;
    std::unique_ptr<MultiProcessPagedLoadPerformanceMetricsStorage> m_warmCacheMetrics;
    std::unique_ptr<MultiProcessPagedLoadPerformanceMetricsStorage> m_coldCacheMetrics;
    std::unique_ptr<MultiProcessNonPagedLoadPerformanceMetricsStorage> m_nonPagedMetrics;

    ManagerPerformanceMetricsStorage(std::unique_ptr<ECPresentationManager> manager, int id, AggregateMultiProcessPerformanceMetricsStorage& aggregateMetricsStorage)
        : m_manager(std::move(manager)), m_id(id),
        m_warmCacheMetrics(std::make_unique<MultiProcessPagedLoadPerformanceMetricsStorage>(aggregateMetricsStorage.m_warmCacheMetrics, HierarchyCacheState::Warm)),
        m_coldCacheMetrics(std::make_unique<MultiProcessPagedLoadPerformanceMetricsStorage>(aggregateMetricsStorage.m_coldCacheMetrics, HierarchyCacheState::Cold)),
        m_nonPagedMetrics(std::make_unique<MultiProcessNonPagedLoadPerformanceMetricsStorage>(aggregateMetricsStorage.m_nonPagedMetrics))
        {}
    };

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct MultiProcessPerformanceAnalysis : RulesEnginePerformanceAnalysisTests
{
    ECDb m_project;
    PresentationRuleSetPtr m_ruleset;
    MultiProcessPerformanceAnalysisConfigutration m_config;
    std::unique_ptr<Reporter> m_aggregateReporter;

    virtual void SetUp() override
        {
        m_reporterConfig.fileName = "MultiProcessPerformanceReport";
        m_reporterConfig.fieldNames = {
            REPORTER_FIELD_Configuration,
            REPORTER_FIELD_Manager,

            REPORT_FIELD_TimeToLoadAllHierarchy,

            COLD_CACHE_FIELD(REPORT_FIELD_AvgTimeForPage),
            COLD_CACHE_FIELD(REPORT_FIELD_MaxTimeForPage),
            COLD_CACHE_FIELD(REPORT_FIELD_NumberOfPageWithMaxTime),
            COLD_CACHE_FIELD(REPORT_FIELD_NumberOfPagesAboveThreshold),
            COLD_CACHE_FIELD(REPORT_FIELD_TimeToLoadAllHierarchyPaged),

            WARM_CACHE_FIELD(REPORT_FIELD_AvgTimeForPage),
            WARM_CACHE_FIELD(REPORT_FIELD_MaxTimeForPage),
            WARM_CACHE_FIELD(REPORT_FIELD_NumberOfPageWithMaxTime),
            WARM_CACHE_FIELD(REPORT_FIELD_NumberOfPagesAboveThreshold),
            WARM_CACHE_FIELD(REPORT_FIELD_TimeToLoadAllHierarchyPaged),
            };
        m_reporterConfig.jsonGroupingFieldNames = {
            REPORTER_FIELD_Configuration,
            REPORTER_FIELD_Manager,
            };
        m_reporterConfig.csvTestNameFieldNames = {
            REPORTER_FIELD_Configuration,
            REPORTER_FIELD_Manager,
            };
        m_reporterConfig.csvExportedFieldNames = {
            REPORT_FIELD_TimeToLoadAllHierarchy,

            COLD_CACHE_FIELD(REPORT_FIELD_AvgTimeForPage),
            COLD_CACHE_FIELD(REPORT_FIELD_MaxTimeForPage),
            COLD_CACHE_FIELD(REPORT_FIELD_NumberOfPagesAboveThreshold),
            COLD_CACHE_FIELD(REPORT_FIELD_TimeToLoadAllHierarchyPaged),

            WARM_CACHE_FIELD(REPORT_FIELD_AvgTimeForPage),
            WARM_CACHE_FIELD(REPORT_FIELD_MaxTimeForPage),
            WARM_CACHE_FIELD(REPORT_FIELD_NumberOfPagesAboveThreshold),
            WARM_CACHE_FIELD(REPORT_FIELD_TimeToLoadAllHierarchyPaged),
            };
        RulesEnginePerformanceAnalysisTests::SetUp();
        m_aggregateReporter = std::make_unique<Reporter>(m_reporterConfig.fieldNames);

        LoadConfiguration();
        ASSERT_TRUE(SUCCESS == OpenProject());
        m_ruleset = ReadRuleset();
        ASSERT_TRUE(m_ruleset.IsValid());
        m_reporter = std::make_unique<Reporter>();
        }

    void TearDown() override {RulesEnginePerformanceAnalysisTests::TearDown();}
    Reporter& _GetCsvReporter() {return *m_aggregateReporter;}

    void LoadConfiguration();
    BentleyStatus OpenProject();
    PresentationRuleSetPtr ReadRuleset();
    std::unique_ptr<ECPresentationManager> CreateManager(int threadsCount);
    void ResetManagers(bvector<ManagerPerformanceMetricsStorage>& testCases, int threadCount);
    bvector<ManagerPerformanceMetricsStorage> CreateManagers(TestCase const& testCase, AggregateMultiProcessPerformanceMetricsStorage& aggregateMetrics);
    void Report(bvector<ManagerPerformanceMetricsStorage> const& testCases, AggregateMultiProcessPerformanceMetricsStorage const& aggregateMetrics, TestCase const& config);

    folly::Future<folly::Unit> GetNodesNonPaged(bvector<ManagerPerformanceMetricsStorage>& testCases);
    folly::Future<folly::Unit> GetNodesPaged(ECPresentationManager& manager, MultiProcessPagedLoadPerformanceMetricsStorage& metrics);
    folly::Future<folly::Unit> GetNodesPaged(bvector<ManagerPerformanceMetricsStorage>& testCases, HierarchyCacheState cacheState);
    void RunTestCase(TestCase const& config);
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String ParseProjectFileNameFromJson(Json::Value& json)
    {
    if (!json.hasMember("imodel") || !json["imodel"].isString())
        return DEFAULT_PROJECT_FILE_NAME;

    return json["imodel"].asString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bvector<TestCase> ParseTestCasesFromJson(Json::Value& json)
    {
    static bvector<TestCase> s_defaultTestCases = { TestCase(1,1),TestCase(1,4),TestCase(4,1),TestCase(4,4) };
    if (!json.hasMember("testCases") || !json["testCases"].isArray())
        return s_defaultTestCases;

    bvector<TestCase> testCases;
    Json::Value& testCasesJson = json["testCases"];
    for (auto iter = testCasesJson.begin(); iter != testCasesJson.end(); iter++)
        {
        Json::Value& testCaseJson = *iter;
        int managersCount = testCaseJson["managersCount"].asInt();
        int threadsCount = testCaseJson["threadsCount"].asInt();
        if (threadsCount == 0 || managersCount == 0)
            continue;
        testCases.push_back(TestCase(managersCount, threadsCount));
        }

    return testCases.empty() ? s_defaultTestCases : testCases;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void MultiProcessPerformanceAnalysis::LoadConfiguration()
    {
    BeFileName configFilePath;
    BeTest::GetHost().GetDocumentsRoot(configFilePath);
    configFilePath.AppendToPath(L"Datasets").AppendToPath(CONFIG_FILE_NAME);
    Utf8String jsonString;
    PresentationManagerTestsHelper::ReadFileContent(configFilePath, jsonString);

    Json::Value json = Json::Reader::DoParse(jsonString);
    m_config.m_projectFileName = ParseProjectFileNameFromJson(json);
    m_config.m_testCases = ParseTestCasesFromJson(json);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus MultiProcessPerformanceAnalysis::OpenProject()
    {
    BeFileName datasetPath;
    BeTest::GetHost().GetDocumentsRoot(datasetPath);
    datasetPath.AppendToPath(L"Datasets").AppendSeparator().AppendUtf8(m_config.m_projectFileName.c_str());

    if (!datasetPath.DoesPathExist())
        {
        BeAssert(false);
        return ERROR;
        }
    if (BeSQLite::DbResult::BE_SQLITE_OK != m_project.OpenBeSQLiteDb(datasetPath, BeSQLite::Db::OpenParams(BeSQLite::Db::OpenMode::Readonly)))
        {
        BeAssert(false);
        return ERROR;
        }

    BeFileName assetsDirectory;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(assetsDirectory);
    ECSchemaReadContext::Initialize(assetsDirectory);
    // we want to make sure all schemas in the dataset are fully loaded so that's not included in our performance test results
    m_project.Schemas().GetSchemas(true);
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
PresentationRuleSetPtr MultiProcessPerformanceAnalysis::ReadRuleset()
    {
    BeFileName rulesetPath;
    BeTest::GetHost().GetDocumentsRoot(rulesetPath);
    rulesetPath.AppendToPath(L"Rulesets").AppendToPath(L"Hierarchy").AppendToPath(RULESET_FILE_NAME);
    return PresentationRuleSet::ReadFromJsonFile(rulesetPath);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void MultiProcessPerformanceAnalysis::Report(bvector<ManagerPerformanceMetricsStorage> const& managers, AggregateMultiProcessPerformanceMetricsStorage const& aggregateMetrics, TestCase const& testCase)
    {
    Utf8PrintfString configuration("Managers count - %d Threads count - %d", testCase.m_processesCount, testCase.m_threadsCount);
    for (ManagerPerformanceMetricsStorage const& metrics : managers)
        {
        m_reporter->Next();
        m_reporter->Record(REPORTER_FIELD_Configuration, configuration.c_str());
        m_reporter->Record(REPORTER_FIELD_Manager, Utf8PrintfString("Manager-%d", metrics.m_id).c_str());
        metrics.m_nonPagedMetrics->Save(*m_reporter);
        metrics.m_coldCacheMetrics->Save(*m_reporter);
        metrics.m_warmCacheMetrics->Save(*m_reporter);
        }

    m_aggregateReporter->Next();
    m_aggregateReporter->Record(REPORTER_FIELD_Configuration, configuration.c_str());
    aggregateMetrics.m_nonPagedMetrics.Save(*m_aggregateReporter);
    aggregateMetrics.m_coldCacheMetrics.Save(*m_aggregateReporter);
    aggregateMetrics.m_warmCacheMetrics.Save(*m_aggregateReporter);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::unique_ptr<ECPresentationManager> MultiProcessPerformanceAnalysis::CreateManager(int threadsCount)
    {
    BeFileName assetsDirectory, temporaryDirectory;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(assetsDirectory);
    BeTest::GetHost().GetTempDir(temporaryDirectory);

    BeFileName supplementalRulesetsDirectory(assetsDirectory);
    supplementalRulesetsDirectory.AppendToPath(L"SupplementalRulesets");

    bmap<int, unsigned> threadAllocations;
    threadAllocations.Insert(INT_MAX, threadsCount);

    ECPresentationManager::Params params(ECPresentationManager::Paths(assetsDirectory, temporaryDirectory));
    params.SetMultiThreadingParams(threadAllocations);
    params.SetMode(ECPresentationManager::Mode::ReadOnly);
    ECPresentationManager::Params::CachingParams cachingParams;
    cachingParams.SetCacheDirectoryPath(temporaryDirectory);
    params.SetCachingParams(cachingParams);

    auto manager = std::make_unique<ECPresentationManager>(params);
    manager->GetLocaters().RegisterLocater(*SupplementalRuleSetLocater::Create(*DirectoryRuleSetLocater::Create(supplementalRulesetsDirectory.GetNameUtf8().c_str())));
    TestRuleSetLocaterPtr locater = TestRuleSetLocater::Create();
    locater->AddRuleSet(*m_ruleset);
    manager->GetLocaters().RegisterLocater(*locater);
    manager->GetConnections().CreateConnection(m_project);
    return manager;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void MultiProcessPerformanceAnalysis::ResetManagers(bvector<ManagerPerformanceMetricsStorage>& managers, int threadCount)
    {
    for (ManagerPerformanceMetricsStorage& manager : managers)
        manager.m_manager = nullptr;

    BeFileName temporaryDirectory;
    BeTest::GetHost().GetTempDir(temporaryDirectory);
    BeFileName::EmptyDirectory(temporaryDirectory);

    for (ManagerPerformanceMetricsStorage& manager : managers)
        manager.m_manager = CreateManager(threadCount);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<ManagerPerformanceMetricsStorage> MultiProcessPerformanceAnalysis::CreateManagers(TestCase const& testCase, AggregateMultiProcessPerformanceMetricsStorage& aggregateMetrics)
    {
    bvector<ManagerPerformanceMetricsStorage> managers;
    for (int i = 0; i < testCase.m_processesCount; ++i)
        managers.push_back(ManagerPerformanceMetricsStorage(nullptr, i, aggregateMetrics));

    ResetManagers(managers, testCase.m_threadsCount);
    return managers;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<folly::Unit> MultiProcessPerformanceAnalysis::GetNodesNonPaged(bvector<ManagerPerformanceMetricsStorage>& managers)
    {
    bvector<folly::Future<folly::Unit>> futures;
    for (ManagerPerformanceMetricsStorage& manager : managers)
        {
        std::shared_ptr<StopWatch> timer = std::make_shared<StopWatch>(true);
        auto params = AsyncHierarchyRequestParams::Create(m_project, m_ruleset->GetRuleSetId(), RulesetVariables(), nullptr);
        futures.push_back(PresentationManagerTestsHelper::GetAllNodes(*manager.m_manager, params, [](double){})
            .then([&manager, timer](size_t nodesCount)
                {
                manager.m_nonPagedMetrics->ReportTotalNodesCount(nodesCount);
                manager.m_nonPagedMetrics->ReportTotalTime(timer->GetCurrentSeconds());
                }));
        }
    return folly::collect(futures).then();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<folly::Unit> MultiProcessPerformanceAnalysis::GetNodesPaged(ECPresentationManager& manager, MultiProcessPagedLoadPerformanceMetricsStorage& metrics)
    {
    std::shared_ptr<StopWatch> timer = std::make_shared<StopWatch>(true);
    auto params = AsyncHierarchyRequestParams::Create(m_project, m_ruleset->GetRuleSetId(), RulesetVariables(), nullptr);
    return PresentationManagerTestsHelper::GatAllNodesPaged(manager, params, NODES_PAGE_SIZE, [&metrics](int pageIndex, double pageTime, double countTime)
        {
        metrics.ReportPageLoad(pageIndex, pageTime, countTime);
        }).then([timer, &metrics]() {metrics.ReportTotalTime(timer->GetCurrentSeconds());});
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<folly::Unit> MultiProcessPerformanceAnalysis::GetNodesPaged(bvector<ManagerPerformanceMetricsStorage>& managers, HierarchyCacheState cacheState)
    {
    bvector<folly::Future<folly::Unit>> futures;
    for (ManagerPerformanceMetricsStorage& manager : managers)
        futures.push_back(GetNodesPaged(*manager.m_manager, HierarchyCacheState::Cold == cacheState ? *manager.m_coldCacheMetrics : *manager.m_warmCacheMetrics));

    return folly::collect(futures).then();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void MultiProcessPerformanceAnalysis::RunTestCase(TestCase const& testCase)
    {
    NativeLogging::LoggingManager::GetLogger(LOGGER_NAMESPACE)->infov("Getting nodes using %d managers with %d threads.", testCase.m_processesCount, testCase.m_threadsCount);

    AggregateMultiProcessPerformanceMetricsStorage aggregateMetrics(testCase.m_processesCount);
    bvector<ManagerPerformanceMetricsStorage> managers = CreateManagers(testCase, aggregateMetrics);
    // get all nodes without paging
    GetNodesNonPaged(managers).get();
    NativeLogging::LoggingManager::GetLogger(LOGGER_NAMESPACE)->infov("Finished loading all nodes without paging.");

    // clear all caches
    ResetManagers(managers, testCase.m_threadsCount);

    // get all nodes in pages with cold cache
    GetNodesPaged(managers, HierarchyCacheState::Cold).get();
    NativeLogging::LoggingManager::GetLogger(LOGGER_NAMESPACE)->infov("Finished loading all nodes in pages with cold cache.");

    // get all nodes in pages with warm cache
    GetNodesPaged(managers, HierarchyCacheState::Warm).get();
    NativeLogging::LoggingManager::GetLogger(LOGGER_NAMESPACE)->infov("Finished loading all nodes in pages with warm cache.");

    Report(managers, aggregateMetrics, testCase);
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MultiProcessPerformanceAnalysis, Run)
    {
    for (TestCase const& testCase : m_config.m_testCases)
        RunTestCase(testCase);
    }
