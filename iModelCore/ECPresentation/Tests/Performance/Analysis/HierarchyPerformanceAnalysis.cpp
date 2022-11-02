/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "PerformanceAnalysisTests.h"
#include "HierarchyPerformanceAnalysis.h"
#include "../../../Source/RulesEngineTypes.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

#define THREADS_COUNT                    1
#define HIERARCHY_REQUEST_PAGE_SIZE      20

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct HierarchyPerformanceAnalysis : SingleManagerRulesEnginePerformanceAnalysisTests
    {
    virtual void SetUp() override
        {
        m_config.threadsCount = THREADS_COUNT;
        m_config.rulesetsSubDirectory = "Hierarchy";
        m_reporterConfig.fileName = "HierarchiesPerformanceReport";
        m_reporterConfig.fieldNames = {
            REPORT_FIELD_Dataset,
            REPORT_FIELD_RulesetId,
            REPORT_FIELD_TimeToLoadAllSchemas,

            REPORT_FIELD_TotalNodesCount,
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
            REPORT_FIELD_Dataset,
            REPORT_FIELD_RulesetId,
            };
        m_reporterConfig.csvTestNameFieldNames = {
            REPORT_FIELD_Dataset,
            REPORT_FIELD_RulesetId,
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

        SingleManagerRulesEnginePerformanceAnalysisTests::SetUp();
        }

    int GetHierarchyDepthLimit(Utf8StringCR rulesetId) const;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
int HierarchyPerformanceAnalysis::GetHierarchyDepthLimit(Utf8StringCR rulesetId) const
    {
    auto iter = m_rulesetConfigs.find(rulesetId);
    if (m_rulesetConfigs.end() == iter)
        return -1;

    return iter->second.maxDepth;
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
static void LogDepthLimitReachCount(PresentationManagerTestsHelper::HierarchyDepthLimiter& depthLimiter)
    {
    int count = depthLimiter.GetDepthLimitReachCount();
    if (count == 0)
        return;
    NativeLogging::LoggingManager::GetLogger(LOGGER_NAMESPACE)->infov("    Depth limit reach count - %d", count);
    depthLimiter.ResetDepthLimitReachCount();
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HierarchyPerformanceAnalysis, Run)
    {
    ForEachDatasetAndRuleset([&](Reporter& reporter, ECDbR project, Utf8StringCR rulesetId)
        {
        auto params = AsyncHierarchyRequestParams::Create(project, rulesetId, RulesetVariables(), nullptr);
        PresentationManagerTestsHelper::HierarchyDepthLimiter depthLimiter(GetHierarchyDepthLimit(rulesetId));

        // first measure the time it takes to load the hierarchy without paging (used when creating the whole hierarchy)
        StopWatch createFullHierarchyTime("", true);
        size_t totalNodesCount = PresentationManagerTestsHelper::GetAllNodes(*m_manager, params, [](double){}, &depthLimiter).get();
        createFullHierarchyTime.Stop();
        reporter.Record(REPORT_FIELD_TimeToLoadAllHierarchy, createFullHierarchyTime.GetElapsed().ToSeconds());
        reporter.Record(REPORT_FIELD_TotalNodesCount, Json::Value((uint64_t)totalNodesCount));
        NativeLogging::LoggingManager::GetLogger(LOGGER_NAMESPACE)->infov("    Loaded all hierarchy with %" PRIu64 " nodes in %.2f s", (uint64_t)totalNodesCount, createFullHierarchyTime.GetElapsed().ToSeconds());
        LogDepthLimitReachCount(depthLimiter);

        // Reset presentation manager so we don't have anything cached
        Reset(&project);

        // now load all hierarchy in pages (cold cache)
        totalNodesCount = 0;
        createFullHierarchyTime.Init(true);
        HierarchyPagedLoadPerformanceMetricsStorage metricsColdCache(HierarchyCacheState::Cold);
        totalNodesCount = PresentationManagerTestsHelper::GatAllNodesPaged(*m_manager, params, HIERARCHY_REQUEST_PAGE_SIZE, [&metricsColdCache](int pageIndex, double pageTime, double countTime)
            {
            metricsColdCache.ReportPageLoad(pageIndex, pageTime, countTime);
            }, &depthLimiter).get();
        createFullHierarchyTime.Stop();
        metricsColdCache.ReportTotalTime(createFullHierarchyTime.GetElapsed());
        metricsColdCache.Save(reporter);
        NativeLogging::LoggingManager::GetLogger(LOGGER_NAMESPACE)->infov("    Loaded all hierarchy with %" PRIu64 " nodes in pages with cold cache in %.2f s", (uint64_t)totalNodesCount, createFullHierarchyTime.GetElapsed().ToSeconds());
        LogDepthLimitReachCount(depthLimiter);

        // now load all hierarchy in pages again (warm cache)
        totalNodesCount = 0;
        createFullHierarchyTime.Init(true);
        HierarchyPagedLoadPerformanceMetricsStorage metricsWarmCache(HierarchyCacheState::Warm);
        totalNodesCount = PresentationManagerTestsHelper::GatAllNodesPaged(*m_manager, params, HIERARCHY_REQUEST_PAGE_SIZE, [&metricsWarmCache](int pageIndex, double pageTime, double countTime)
            {
            metricsWarmCache.ReportPageLoad(pageIndex, pageTime, countTime);
            }, &depthLimiter).get();
        createFullHierarchyTime.Stop();
        metricsWarmCache.ReportTotalTime(createFullHierarchyTime.GetElapsed());
        metricsWarmCache.Save(reporter);
        NativeLogging::LoggingManager::GetLogger(LOGGER_NAMESPACE)->infov("    Loaded all hierarchy with %" PRIu64 " nodes in pages with warm cache in %.2f s", (uint64_t)totalNodesCount, createFullHierarchyTime.GetElapsed().ToSeconds());
        LogDepthLimitReachCount(depthLimiter);
        });
    }
