/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "../PerformanceTests.h"
#include <Bentley/BeDirectoryIterator.h>
#include <ECPresentation/IECPresentationManager.h>
#include "Reporters.h"
#include "../../../Source/RulesDriven/RulesEngine/PresentationManagerImpl.h"
#include "../../../Source/RulesDriven/RulesEngine/NavNodesCache.h"
#include <memory>

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

#define THREADS_COUNT                               1
#define HIERARCHY_REQUEST_PAGE_SIZE                 20
#define PAGE_LOAD_THRESHOLD                         0.5

#define REPORT_FIELD_Dataset                        "Dataset"
#define REPORT_FIELD_RulesetId                      "RulesetId"
#define REPORT_FIELD_TimeToLoadAllSchemas           "Time to load all schemas"
#define REPORT_FIELD_TotalNodesCount                "Total nodes count"
#define REPORT_FIELD_TimeToLoadAllHierarchy         "Time to load all hierarchy"
#define REPORT_FIELD_TimeToLoadAllHierarchyPaged    "Time to load all hierarchy (paged)"
#define REPORT_FIELD_AvgTimeForFirstPage            "Avg time for first page"
#define REPORT_FIELD_MaxTimeForFirstPage            "Max time for first page"
#define REPORT_FIELD_AvgTimeForOtherPages           "Avg time for other pages"
#define REPORT_FIELD_MaxTimeForOtherPages           "Max time for other pages"
#define REPORT_FIELD_NumberOfPagesAboveThreshold    "Number of pages above threshold"
#define WARM_CACHE(field_name)                      ("[warm cache] " field_name)
#define COLD_CACHE(field_name)                      ("[cold cache] " field_name)

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                10/2019
+===============+===============+===============+===============+===============+======*/
enum class HierarchyCacheState
    {
    Cold,
    Warm,
    };

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                10/2019
+===============+===============+===============+===============+===============+======*/
struct PagedLoadPerformanceMetricsStorage
{
#define FIELD_NAME(base_name) (m_cacheState == HierarchyCacheState::Cold ? COLD_CACHE(base_name) : WARM_CACHE(base_name))
private:
    HierarchyCacheState m_cacheState;
    double m_firstPageLoadMax;
    double m_totalTimeLoadingFirstPages;
    uint64_t m_totalNumberOfFirstPages;
    double m_otherPagesLoadMax;
    double m_totalTimeLoadingOtherPages;
    uint64_t m_totalNumberOfOtherPages;
    uint64_t m_numberOfPagesAboveThreshold;
    double m_totalTime;
public:
    PagedLoadPerformanceMetricsStorage(HierarchyCacheState cacheState)
        : m_cacheState(cacheState)
        {
        m_firstPageLoadMax = 0;
        m_totalTimeLoadingFirstPages = 0;
        m_totalNumberOfFirstPages = 0;
        m_otherPagesLoadMax = 0;
        m_totalTimeLoadingOtherPages = 0;
        m_totalNumberOfOtherPages = 0;
        m_numberOfPagesAboveThreshold = 0;
        m_totalTime = 0;
        }
    void ReportFirstPageLoad(double time)
        {
        if (time > m_firstPageLoadMax)
            m_firstPageLoadMax = time;
        m_totalTimeLoadingFirstPages += time;
        m_totalNumberOfFirstPages++;
        if (time > PAGE_LOAD_THRESHOLD)
            m_numberOfPagesAboveThreshold++;
        }
    void ReportOtherPageLoad(double time)
        {
        if (time > m_otherPagesLoadMax)
            m_otherPagesLoadMax = time;
        m_totalTimeLoadingOtherPages += time;
        m_totalNumberOfOtherPages++;
        if (time > PAGE_LOAD_THRESHOLD)
            m_numberOfPagesAboveThreshold++;
        }
    void ReportTotalTime(double time)
        {
        m_totalTime = time;
        }
    void Save(Reporter& reporter)
        {
        reporter.Record(FIELD_NAME(REPORT_FIELD_MaxTimeForFirstPage), Json::Value(m_firstPageLoadMax));
        if (m_totalNumberOfFirstPages > 0)
            reporter.Record(FIELD_NAME(REPORT_FIELD_AvgTimeForFirstPage), Json::Value(m_totalTimeLoadingFirstPages / m_totalNumberOfFirstPages));
        reporter.Record(FIELD_NAME(REPORT_FIELD_MaxTimeForOtherPages), Json::Value(m_otherPagesLoadMax));
        if (m_totalNumberOfOtherPages > 0)
            reporter.Record(FIELD_NAME(REPORT_FIELD_AvgTimeForOtherPages), Json::Value(m_totalTimeLoadingOtherPages / m_totalNumberOfOtherPages));
        reporter.Record(FIELD_NAME(REPORT_FIELD_NumberOfPagesAboveThreshold), Json::Value(m_numberOfPagesAboveThreshold));
        reporter.Record(FIELD_NAME(REPORT_FIELD_TimeToLoadAllHierarchyPaged), Json::Value(m_totalTime));
        }
};

/*=================================================================================**//**
* @bsiclass                                     Haroldas.Vitunskas              01/2019
+===============+===============+===============+===============+===============+======*/
struct HierarchyPerformanceAnalysis : ECPresentationTest
    {
    RulesDrivenECPresentationManager* m_manager = nullptr;
    RuntimeJsonLocalState m_localState;
    TestRuleSetLocaterPtr m_locater;

    folly::Future<size_t> GetNodesCount(ECDbR project, JsonValueCR options, NavNodeCP parentNode);
    folly::Future<NavNodesContainer> GetNodes(ECDbR project, PageOptions const& pageOptions, JsonValueCR options, NavNodeCP parentNode);
    void GetAllNodesPaged(PagedLoadPerformanceMetricsStorage& metrics, ECDbR project, bvector<NavNodeCPtr> const& nodePath, RulesDrivenECPresentationManager::NavigationOptions const&);
    folly::Future<size_t> GetAllNodes(ECDbR project, RulesDrivenECPresentationManager::NavigationOptions const&, NavNodeCPtr parent = nullptr);
    void AnalyzeHierarchy(Reporter& reporter, ECDbR project, Utf8StringCR rulesetId, BeDuration const& schemasLoadTime);

    void Reset(ECDb* = nullptr);
    virtual void SetUp() override;
    virtual void TearDown() override;
    static void SetUpTestCase();
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Haroldas.Vitunskas              01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void HierarchyPerformanceAnalysis::SetUpTestCase()
    {
    NativeLogging::LoggingConfig::SetSeverity(LOGGER_NAMESPACE, NativeLogging::LOG_INFO);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Haroldas.Vitunskas              01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void HierarchyPerformanceAnalysis::SetUp()
    {
    ECPresentationTest::SetUp();
    Localization::Init();
    m_locater = TestRuleSetLocater::Create();
    Reset();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void HierarchyPerformanceAnalysis::TearDown()
    {
    DELETE_AND_CLEAR(m_manager);
    m_locater = nullptr;
    ECPresentationTest::TearDown();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void HierarchyPerformanceAnalysis::Reset(ECDb* project)
    {
    DELETE_AND_CLEAR(m_manager);

    BeFileName assetsDirectory, temporaryDirectory;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(assetsDirectory);
    BeTest::GetHost().GetTempDir(temporaryDirectory);
    ECSchemaReadContext::Initialize(assetsDirectory);
    BeFileName::EmptyDirectory(temporaryDirectory);

    BeFileName supplementalRulesetsDirectory(assetsDirectory);
    supplementalRulesetsDirectory.AppendToPath(L"SupplementalRulesets");

    bmap<int, unsigned> threadAllocations;
    threadAllocations.Insert(INT_MAX, THREADS_COUNT);

    RulesDrivenECPresentationManager::Params params(RulesDrivenECPresentationManager::Paths(assetsDirectory, temporaryDirectory));
    params.SetMultiThreadingParams(threadAllocations);
    params.SetLocalState(&m_localState);
    params.SetLocalizationProvider(new SQLangLocalizationProvider());
    params.SetMode(RulesDrivenECPresentationManager::Mode::ReadOnly);
    m_manager = new RulesDrivenECPresentationManager(params);
    m_manager->GetLocaters().RegisterLocater(*SupplementalRuleSetLocater::Create(*DirectoryRuleSetLocater::Create(supplementalRulesetsDirectory.GetNameUtf8().c_str())));
    m_manager->GetLocaters().RegisterLocater(*m_locater);
    if (nullptr != project)
        m_manager->GetConnections().CreateConnection(*project);
    }

/*---------------------------------------------------------------------------------**//**
* Note: this method tries to replicate real world scenario where a user expands a delay-loaded,
* paged tree. In that case we first ask for nodes count and then request nodes in pages.
* @betest                                       Grigas.Petraitis                10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void HierarchyPerformanceAnalysis::GetAllNodesPaged(PagedLoadPerformanceMetricsStorage& metrics, ECDbR project, bvector<NavNodeCPtr> const& nodePath, RulesDrivenECPresentationManager::NavigationOptions const& options)
    {
    StopWatch timer(true);
    NavNodeCPtr parent = nodePath.empty() ? nullptr : nodePath.back();
    size_t nodesCount = GetNodesCount(project, options.GetJson(), parent.get()).get();
    size_t pageCount = (size_t)ceil(1.0 * nodesCount / HIERARCHY_REQUEST_PAGE_SIZE);
    for (size_t pageIndex = 0; pageIndex < pageCount; ++pageIndex)
        {
        PageOptions pageOptions(pageIndex * HIERARCHY_REQUEST_PAGE_SIZE, HIERARCHY_REQUEST_PAGE_SIZE);
        NavNodesContainer nodes = GetNodes(project, pageOptions, options.GetJson(), parent.get()).get();

        if (0 == pageIndex)
            metrics.ReportFirstPageLoad(timer.GetCurrent());
        else
            metrics.ReportOtherPageLoad(timer.GetCurrent());

        for (size_t nodeIndex = 0; nodeIndex < nodes.GetSize(); ++nodeIndex)
            {
            NavNodeCPtr node = nodes[nodeIndex];
            if (node->HasChildren())
                {
                bvector<NavNodeCPtr> path(nodePath);
                path.push_back(node);
                GetAllNodesPaged(metrics, project, path, options);
                }
            }

        timer.Init(true);
        }
    }

/*---------------------------------------------------------------------------------**//**
* Note: this is basically a copy of LoadHierarchyIncrementally in ECPresentationUtils.cpp
* in iModelJsNodeAddon.
* @betest                                       Grigas.Petraitis                10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<size_t> HierarchyPerformanceAnalysis::GetAllNodes(ECDbR project, RulesDrivenECPresentationManager::NavigationOptions const& options, NavNodeCPtr parent)
    {
    return GetNodes(project, PageOptions(), options.GetJson(), parent.get()).then([this, &project, options](NavNodesContainer nodes)
        {
        std::vector<folly::Future<size_t>> childrenFutures;
        for (NavNodeCPtr node : nodes)
            {
            if (node->HasChildren())
                childrenFutures.push_back(GetAllNodes(project, options, node.get()));
            }
        size_t nodesCount = nodes.GetSize();
        return folly::collect(childrenFutures).then([nodesCount](std::vector<size_t> counts) mutable
            {
            for (size_t count : counts)
                nodesCount += count;
            return nodesCount;
            });
        });
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<size_t> HierarchyPerformanceAnalysis::GetNodesCount(ECDbR project, JsonValueCR options, NavNodeCP parentNode)
    {
    if (nullptr == parentNode)
        return m_manager->GetRootNodesCount(project, options);
    return m_manager->GetChildrenCount(project, *parentNode, options);
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Haroldas.Vitunskas              01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<NavNodesContainer> HierarchyPerformanceAnalysis::GetNodes(ECDbR project, PageOptions const& pageOptions, JsonValueCR options, NavNodeCP parentNode)
    {
    if (nullptr == parentNode)
        return m_manager->GetRootNodes(project, pageOptions, options);
    return m_manager->GetChildren(project, *parentNode, pageOptions, options);
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void HierarchyPerformanceAnalysis::AnalyzeHierarchy(Reporter& reporter, ECDbR project, Utf8StringCR rulesetId, BeDuration const& schemasLoadTime)
    {
    RulesDrivenECPresentationManager::NavigationOptions options(rulesetId);

    reporter.Next();
    reporter.Record(REPORT_FIELD_Dataset, Utf8String(BeFileName(project.GetDbFileName(), true).GetFileNameAndExtension().c_str()));
    reporter.Record(REPORT_FIELD_RulesetId, rulesetId);
    reporter.Record(REPORT_FIELD_TimeToLoadAllSchemas, schemasLoadTime.ToSeconds());

    // first measure the time it takes to load the hierarchy without paging (used when creating the whole hierarchy)
    StopWatch createFullHierarchyTime(true);
    size_t totalNodesCount = GetAllNodes(project, options).get();
    createFullHierarchyTime.Stop();
    reporter.Record(REPORT_FIELD_TimeToLoadAllHierarchy, createFullHierarchyTime.GetElapsed().ToSeconds());
    reporter.Record(REPORT_FIELD_TotalNodesCount, Json::Value((uint64_t)totalNodesCount));
    NativeLogging::LoggingManager::GetLogger(LOGGER_NAMESPACE)->infov("    Loaded all hierarchy with %" PRIu64 " nodes in %.2f s", (uint64_t)totalNodesCount, createFullHierarchyTime.GetElapsed().ToSeconds());

    // Reset presentation manager so we don't have anything cached
    Reset(&project);

    // now load all hierarchy in pages (cold cache)
    createFullHierarchyTime.Init(true);
    PagedLoadPerformanceMetricsStorage metricsColdCache(HierarchyCacheState::Cold);
    GetAllNodesPaged(metricsColdCache, project, bvector<NavNodeCPtr>(), options);
    createFullHierarchyTime.Stop();
    metricsColdCache.ReportTotalTime(createFullHierarchyTime.GetElapsed());
    metricsColdCache.Save(reporter);
    NativeLogging::LoggingManager::GetLogger(LOGGER_NAMESPACE)->infov("    Loaded all hierarchy in pages with cold cache in %.2f s", createFullHierarchyTime.GetElapsed().ToSeconds());

    // now load all hierarchy in pages again (warm cache)
    createFullHierarchyTime.Init(true);
    PagedLoadPerformanceMetricsStorage metricsWarmCache(HierarchyCacheState::Warm);
    GetAllNodesPaged(metricsWarmCache, project, bvector<NavNodeCPtr>(), options);
    createFullHierarchyTime.Stop();
    metricsWarmCache.ReportTotalTime(createFullHierarchyTime.GetElapsed());
    NativeLogging::LoggingManager::GetLogger(LOGGER_NAMESPACE)->infov("    Loaded all hierarchy in pages with warm cache in %.2f s", createFullHierarchyTime.GetElapsed().ToSeconds());
    metricsWarmCache.Save(reporter);
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Haroldas.Vitunskas              01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HierarchyPerformanceAnalysis, AnalyseAllCases)
    {
    Reporter reporter(
        {
        REPORT_FIELD_Dataset,
        REPORT_FIELD_RulesetId,

        REPORT_FIELD_TimeToLoadAllSchemas,

        REPORT_FIELD_TotalNodesCount,

        REPORT_FIELD_TimeToLoadAllHierarchy,

        COLD_CACHE(REPORT_FIELD_AvgTimeForFirstPage),
        COLD_CACHE(REPORT_FIELD_MaxTimeForFirstPage),
        COLD_CACHE(REPORT_FIELD_AvgTimeForOtherPages),
        COLD_CACHE(REPORT_FIELD_MaxTimeForOtherPages),
        COLD_CACHE(REPORT_FIELD_NumberOfPagesAboveThreshold),
        COLD_CACHE(REPORT_FIELD_TimeToLoadAllHierarchyPaged),

        WARM_CACHE(REPORT_FIELD_AvgTimeForFirstPage),
        WARM_CACHE(REPORT_FIELD_MaxTimeForFirstPage),
        WARM_CACHE(REPORT_FIELD_AvgTimeForOtherPages),
        WARM_CACHE(REPORT_FIELD_MaxTimeForOtherPages),
        WARM_CACHE(REPORT_FIELD_NumberOfPagesAboveThreshold),
        WARM_CACHE(REPORT_FIELD_TimeToLoadAllHierarchyPaged),
        });

    BeFileName datasetsPath;
    BeTest::GetHost().GetDocumentsRoot(datasetsPath);
    datasetsPath.AppendToPath(L"Datasets");

    BeFileName rulesetsPath;
    BeTest::GetHost().GetDocumentsRoot(rulesetsPath);
    rulesetsPath.AppendToPath(L"Rulesets");

    // Iterate over datasets
    BeDirectoryIterator datasets(datasetsPath);
    BeFileName dataSetPath;
    bool isDirectory;
    for (; 0 == datasets.GetCurrentEntry(dataSetPath, isDirectory); datasets.ToNext())
        {
        if (isDirectory)
            continue;

        NativeLogging::LoggingManager::GetLogger(LOGGER_NAMESPACE)->infov(L"Dataset: %s", dataSetPath.GetFileNameAndExtension().c_str());

        // load project
        ECDb project;
        BeSQLite::DbResult result;
        if (BeSQLite::DbResult::BE_SQLITE_OK != (result = project.OpenBeSQLiteDb(dataSetPath, BeSQLite::Db::OpenParams(BeSQLite::Db::OpenMode::Readonly))))
            {
            BeAssert(false);
            NativeLogging::LoggingManager::GetLogger(LOGGER_NAMESPACE)->errorv("  Dataset open failed with: %d", (int)result);
            return;
            }
        m_manager->GetConnections().CreateConnection(project);

        // we want to make sure all schemas in the dataset are fully loaded so that's not included in our performance test results
        StopWatch schemasLoadTime(true);
        project.Schemas().GetSchemas(true);
        schemasLoadTime.Stop();

        BeDirectoryIterator rulesets(rulesetsPath);
        BeFileName rulesetPath;
        for (; 0 == rulesets.GetCurrentEntry(rulesetPath, isDirectory); rulesets.ToNext())
            {
            if (isDirectory)
                continue;

            Reset(&project);

            // load ruleset
            NativeLogging::LoggingManager::GetLogger(LOGGER_NAMESPACE)->infov(L"  Ruleset: %s", rulesetPath.GetFileNameAndExtension().c_str());
            PresentationRuleSetPtr ruleset;
            if (rulesetPath.GetExtension().EqualsI(L"xml"))
                ruleset = PresentationRuleSet::ReadFromXmlFile(rulesetPath);
            else if (rulesetPath.GetExtension().EqualsI(L"json"))
                ruleset = PresentationRuleSet::ReadFromJsonFile(rulesetPath);
            if (ruleset.IsNull())
                {
                BeAssert(false);
                NativeLogging::LoggingManager::GetLogger(LOGGER_NAMESPACE)->error("  Ruleset read failed");
                continue;
                }
            m_locater->AddRuleSet(*ruleset);

            AnalyzeHierarchy(reporter, project, ruleset->GetRuleSetId(), schemasLoadTime.GetElapsed());
            }

        project.CloseDb();
        }

    BeFileName reportPath;
    BeTest::GetHost().GetOutputRoot(reportPath);
    reportPath.AppendToPath(L"Reports");
    if (!reportPath.DoesPathExist())
        BeFileName::CreateNewDirectory(reportPath.GetName());
    reportPath.AppendToPath(L"HierarchyPerformanceReport");
    reporter.ToCsvFile(BeFileName(reportPath).AppendExtension(L"csv"));
    reporter.ToJsonFile(BeFileName(reportPath).AppendExtension(L"json"), { REPORT_FIELD_Dataset, REPORT_FIELD_RulesetId });
    }
