/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include "PerformanceAnalysisTests.h"

BEGIN_ECPRESENTATIONTESTS_NAMESPACE

#define PAGE_LOAD_THRESHOLD                         0.5

#define REPORT_FIELD_TotalNodesCount                "Total nodes count"
#define REPORT_FIELD_TimeToLoadAllHierarchy         "Time to load all hierarchy"
#define REPORT_FIELD_TimeToLoadAllHierarchyPaged    "Time to load all hierarchy (paged)"
#define REPORT_FIELD_AvgTimeForPage                 "Avg time for a page"
#define REPORT_FIELD_MaxTimeForPage                 "Max time for a page"
#define REPORT_FIELD_NumberOfPageWithMaxTime        "Number of page that took max time"
#define REPORT_FIELD_NumberOfPagesAboveThreshold    "Number of pages above threshold"
#define WARM_CACHE                                  "[warm cache] "
#define COLD_CACHE                                  "[cold cache] "
#define WARM_CACHE_FIELD(field_name)                (WARM_CACHE field_name)
#define COLD_CACHE_FIELD(field_name)                (COLD_CACHE field_name)

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
enum class HierarchyCacheState
    {
    Cold,
    Warm,
    };

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct HierarchyNonPagedLoadPerformanceMetricsStorage
{
private:
    double m_totalTime;
    size_t m_totalNodesCount;

    int m_sourceCount;

    BeMutex m_mutex;

public:
    HierarchyNonPagedLoadPerformanceMetricsStorage() : m_totalTime(0), m_totalNodesCount(0), m_sourceCount(1) {}
    HierarchyNonPagedLoadPerformanceMetricsStorage(int sourceCount) : m_totalTime(0), m_totalNodesCount(0), m_sourceCount(sourceCount) {}

    void ReportTotalTime(double totalTime)
        {
        BeMutexHolder lock(m_mutex);
        m_totalTime += totalTime;
        }
    void ReportTotalNodesCount(size_t nodesCount)
        {
        BeMutexHolder lock(m_mutex);
        m_totalNodesCount = nodesCount;
        }
    void Save(Reporter& reporter) const
        {
        reporter.Record(REPORT_FIELD_TimeToLoadAllHierarchy, Json::Value(m_totalTime / m_sourceCount));
        reporter.Record(REPORT_FIELD_TotalNodesCount, Json::Value((uint64_t)m_totalNodesCount));
        }
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct HierarchyPagedLoadPerformanceMetricsStorage
    {
#define FIELD_NAME(base_name) (m_cacheState == HierarchyCacheState::Cold ? COLD_CACHE_FIELD(base_name) : WARM_CACHE_FIELD(base_name))

    struct RepeatedTaskTotals
        {
        double maxTime = 0;
        double totalTime = 0;
        uint64_t count = 0;

        void Inc(double value)
            {
            totalTime += value;
            ++count;
            if (value > maxTime)
                maxTime = value;
            }
        double Avg() const { return (count > 0) ? (totalTime / count) : 0; }
        };

    private:
        HierarchyCacheState m_cacheState;
        bpair<int, double> m_pageLoadsMax; // info about page that took most time to load: it's number and time taken to load
        RepeatedTaskTotals m_pageLoads;
        uint64_t m_numberOfPagesAboveThreshold;
        double m_totalTime;

        bvector<int> m_targetPages = { 1, 5, 10, 25, 50, 75, 100, 250, 500, 750, 1000 };
        bmap<int, RepeatedTaskTotals> m_targetPagesLoads;

        int m_sourceCount;
        BeMutex m_mutex;

    public:
        HierarchyPagedLoadPerformanceMetricsStorage(HierarchyCacheState cacheState)
            : m_cacheState(cacheState), m_sourceCount(1)
            {
            m_pageLoadsMax = make_bpair(0, 0);
            m_numberOfPagesAboveThreshold = 0;
            m_totalTime = 0;
            }
        HierarchyPagedLoadPerformanceMetricsStorage(HierarchyCacheState cacheState, int sourceCount)
            : HierarchyPagedLoadPerformanceMetricsStorage(cacheState)
            {
            m_sourceCount = sourceCount;
            }

        void ReportPageLoad(int pageIndex, double pageTime, double countTime)
            {
            BeMutexHolder lock(m_mutex);
            int pageNumber = pageIndex + 1;

            if (0 == pageIndex)
                {
                // for first page count-in the time needed to get the nodes count
                pageTime += countTime;
                }

            if (m_targetPages.end() != std::find(m_targetPages.begin(), m_targetPages.end(), pageNumber))
                m_targetPagesLoads[pageNumber].Inc(pageTime);

            m_pageLoads.Inc(pageTime);

            if (pageTime > m_pageLoadsMax.second)
                m_pageLoadsMax = make_bpair(pageNumber, pageTime);

            if (pageTime > PAGE_LOAD_THRESHOLD)
                m_numberOfPagesAboveThreshold++;
            }
        void ReportTotalTime(double time)
            {
            BeMutexHolder lock(m_mutex);
            m_totalTime += time;
            }
        void Save(Reporter& reporter) const
            {
            for (auto const& entry : m_targetPagesLoads)
                {
                Utf8String prefix = m_cacheState == HierarchyCacheState::Cold ? COLD_CACHE : WARM_CACHE;
                Json::Value pageTimes(Json::objectValue);
                pageTimes["max"] = entry.second.maxTime;
                pageTimes["avg"] = entry.second.totalTime / entry.second.count;
                reporter.Record(Utf8PrintfString("%sPage %d load times", prefix.c_str(), entry.first).c_str(), pageTimes);
                }

            reporter.Record(FIELD_NAME(REPORT_FIELD_MaxTimeForPage), Json::Value(m_pageLoadsMax.second));
            reporter.Record(FIELD_NAME(REPORT_FIELD_NumberOfPageWithMaxTime), Json::Value(m_pageLoadsMax.first));
            reporter.Record(FIELD_NAME(REPORT_FIELD_AvgTimeForPage), Json::Value(m_pageLoads.Avg()));
            reporter.Record(FIELD_NAME(REPORT_FIELD_NumberOfPagesAboveThreshold), Json::Value(m_numberOfPagesAboveThreshold));
            reporter.Record(FIELD_NAME(REPORT_FIELD_TimeToLoadAllHierarchyPaged), Json::Value(m_totalTime / m_sourceCount));
            }
    };

END_ECPRESENTATIONTESTS_NAMESPACE
