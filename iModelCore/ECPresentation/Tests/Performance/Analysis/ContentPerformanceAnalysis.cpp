/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "PerformanceAnalysisTests.h"
#include "../../../Source/RulesEngineTypes.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

#define THREADS_COUNT                               1
#define CONTENT_REQUEST_PAGE_SIZE                   20
#define PAGE_LOAD_THRESHOLD                         0.5

// unique class element (one element per unique class) load metrics
#define REPORT_FIELD_TotalElementsCount                 "Single: Total elements count"
#define REPORT_FIELD_TotalPropertiesCount               "Single: Total properties count"
#define REPORT_FIELD_AveragePropertiesCount             "Single: Average properties count per element"
#define REPORT_FIELD_AvgTimeToLoadInstanceProperties    "Single: Average time to load element properties"
#define REPORT_FIELD_MaxTimeToLoadInstanceProperties    "Single: Max time to load element properties"
// consolidated descriptor metrics
#define REPORT_FIELD_TimeToCreateConsolidatedDescriptor "Time to create consolidated content descriptor"
// content classes metrics
#define REPORT_FIELD_TimeToGetContentClasses            "Time to get content classes"
// all class elements (all elements of each class) full load metrics
#define REPORT_FIELD_TimeToLoadAllContent               "Full: Time to load all content"
#define REPORT_FIELD_TotalContentSetSize                "Full: Total content set size"
#define REPORT_FIELD_AvgTimeToLoadContentForElement     "Full: Average time to load element properties"
// all class elements (all elements of each class) paged load metrics
#define REPORT_FIELD_TimeToLoadAllContentPaged          "Paged: Time to load all content (paged)"
#define REPORT_FIELD_AvgTimeForPage                     "Paged: Avg time for a page"
#define REPORT_FIELD_MaxTimeForPage                     "Paged: Max time for a page"
#define REPORT_FIELD_NumberOfPagesAboveThreshold        "Paged: Number of pages above threshold"

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ContentPerformanceAnalysis : SingleManagerRulesEnginePerformanceAnalysisTests
    {
    /*=================================================================================**//**
    * @bsiclass
    +===============+===============+===============+===============+===============+======*/
    struct ElementPropertiesLoadPerformanceMetricsStorage
        {
        private:
            double m_max;
            double m_totalElements;
        public:
            ElementPropertiesLoadPerformanceMetricsStorage()
                {
                m_max = 0;
                m_totalElements = 0;
                }
            void ReportPropertiesLoad(double time)
                {
                if (time > m_max)
                    m_max = time;
                ++m_totalElements;
                }
            void Save(Reporter& reporter)
                {
                reporter.Record(REPORT_FIELD_MaxTimeToLoadInstanceProperties, Json::Value(m_max));
                if (m_totalElements > 0)
                    reporter.Record(REPORT_FIELD_AvgTimeToLoadInstanceProperties, Json::Value(m_max / m_totalElements));
                }
        };

    /*=================================================================================**//**
    * @bsiclass
    +===============+===============+===============+===============+===============+======*/
    struct FullLoadPerformanceMetricsStorage
        {
        private:
            double m_totalTime;
            uint64_t m_totalNumberOfElements;
            uint64_t m_totalNumberOfFields;
            uint64_t m_totalRequests;
        public:
            FullLoadPerformanceMetricsStorage()
                {
                m_totalTime = 0;
                m_totalNumberOfElements = 0;
                m_totalNumberOfFields = 0;
                m_totalRequests = 0;
                }
            void ReportLoad(size_t elementsCount, size_t fieldsCount, double time)
                {
                m_totalTime += time;
                m_totalNumberOfElements += elementsCount;
                m_totalNumberOfFields += fieldsCount;
                m_totalRequests++;
                }
            //void ReportTotalTime(double time) {m_totalTime = time;}
            double GetTotalTime() const {return m_totalTime;}
            void Save(Reporter& reporter)
                {
                reporter.Record(REPORT_FIELD_TimeToLoadAllContent, Json::Value(m_totalTime));
                reporter.Record(REPORT_FIELD_TotalContentSetSize, Json::Value(m_totalNumberOfElements));
                reporter.Record(REPORT_FIELD_AvgTimeToLoadContentForElement, Json::Value(m_totalNumberOfElements > 0 ? (m_totalTime / m_totalNumberOfElements) : 0));
                }
        };

    /*=================================================================================**//**
    * @bsiclass
    +===============+===============+===============+===============+===============+======*/
    struct PagedLoadPerformanceMetricsStorage
        {
        private:
            double m_pageLoadMax;
            double m_totalTimeLoadingPages;
            uint64_t m_totalNumberOfPages;
            uint64_t m_numberOfPagesAboveThreshold;
            double m_totalTime;
        public:
            PagedLoadPerformanceMetricsStorage()
                {
                m_pageLoadMax = 0;
                m_totalTimeLoadingPages = 0;
                m_totalNumberOfPages = 0;
                m_numberOfPagesAboveThreshold = 0;
                m_totalTime = 0;
                }
            void ReportPageLoad(double time)
                {
                if (time > m_pageLoadMax)
                    m_pageLoadMax = time;
                m_totalTimeLoadingPages += time;
                m_totalNumberOfPages++;
                if (time > PAGE_LOAD_THRESHOLD)
                    m_numberOfPagesAboveThreshold++;
                }
            void ReportTotalTime(double time) {m_totalTime += time;}
            double GetTotalTime() const {return m_totalTime;}
            void Save(Reporter& reporter)
                {
                reporter.Record(REPORT_FIELD_MaxTimeForPage, Json::Value(m_pageLoadMax));
                if (m_totalNumberOfPages > 0)
                    reporter.Record(REPORT_FIELD_AvgTimeForPage, Json::Value(m_totalTimeLoadingPages / m_totalNumberOfPages));
                reporter.Record(REPORT_FIELD_NumberOfPagesAboveThreshold, Json::Value(m_numberOfPagesAboveThreshold));
                reporter.Record(REPORT_FIELD_TimeToLoadAllContentPaged, Json::Value(m_totalTime));
                }
        };

    size_t GetElementProperties(ElementPropertiesLoadPerformanceMetricsStorage&, ECDbR, Utf8StringCR, bvector<ECClassInstanceKey> const&);
    void LoadFullContent(FullLoadPerformanceMetricsStorage&, ECDbR, ContentDescriptorCR);
    void LoadPagedContent(PagedLoadPerformanceMetricsStorage&, ECDbR, ContentDescriptorCR);

    virtual void SetUp() override
        {
        m_config.threadsCount = THREADS_COUNT;
        m_config.rulesetsSubDirectory = "Content";
        m_reporterConfig.fileName = "ContentPerformanceReport";
        m_reporterConfig.fieldNames = {
            REPORT_FIELD_Dataset,
            REPORT_FIELD_RulesetId,
            REPORT_FIELD_TimeToLoadAllSchemas,

            REPORT_FIELD_TotalElementsCount,
            REPORT_FIELD_TotalPropertiesCount,
            REPORT_FIELD_AveragePropertiesCount,
            REPORT_FIELD_AvgTimeToLoadInstanceProperties,
            REPORT_FIELD_MaxTimeToLoadInstanceProperties,

            REPORT_FIELD_TimeToCreateConsolidatedDescriptor,

            REPORT_FIELD_TimeToGetContentClasses,

            REPORT_FIELD_TimeToLoadAllContent,
            REPORT_FIELD_TotalContentSetSize,
            REPORT_FIELD_AvgTimeToLoadContentForElement,

            REPORT_FIELD_AvgTimeForPage,
            REPORT_FIELD_MaxTimeForPage,
            REPORT_FIELD_NumberOfPagesAboveThreshold,
            REPORT_FIELD_TimeToLoadAllContentPaged,
            };
        m_reporterConfig.jsonGroupingFieldNames = {
            REPORT_FIELD_Dataset,
            };
        m_reporterConfig.csvTestNameFieldNames = {
            REPORT_FIELD_Dataset,
            };
        m_reporterConfig.csvExportedFieldNames = {
            REPORT_FIELD_AvgTimeToLoadInstanceProperties,
            REPORT_FIELD_MaxTimeToLoadInstanceProperties,

            REPORT_FIELD_TimeToCreateConsolidatedDescriptor,

            REPORT_FIELD_TimeToGetContentClasses,

            REPORT_FIELD_TimeToLoadAllContent,
            REPORT_FIELD_AvgTimeToLoadContentForElement,

            REPORT_FIELD_AvgTimeForPage,
            REPORT_FIELD_MaxTimeForPage,
            REPORT_FIELD_NumberOfPagesAboveThreshold,
            REPORT_FIELD_TimeToLoadAllContentPaged,
            };
        SingleManagerRulesEnginePerformanceAnalysisTests::SetUp();
        }
    };

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
static size_t GetTotalFieldsCount(bvector<ContentDescriptor::Field*> const& rootFields)
    {
    size_t count = rootFields.size();
    for (ContentDescriptor::Field const* field : rootFields)
        {
        if (field->IsNestedContentField())
            count = count - 1 + GetTotalFieldsCount(field->AsNestedContentField()->GetFields());
        }
    return count;
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
size_t ContentPerformanceAnalysis::GetElementProperties(ElementPropertiesLoadPerformanceMetricsStorage& metrics, ECDbR project,
    Utf8StringCR rulesetId, bvector<ECClassInstanceKey> const& keys)
    {
    size_t totalPropertiesCount = 0;
    for (ECClassInstanceKey const& key : keys)
        {
        StopWatch timer("", true);
        totalPropertiesCount += m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(project, rulesetId, RulesetVariables(), "", 0, *KeySet::Create({ key })))
            .then([this, &project, &timer, &metrics](ContentDescriptorResponse descriptorResponse) mutable -> folly::Future<size_t>
                {
                return m_manager->GetContent(AsyncContentRequestParams::Create(project, **descriptorResponse))
                    .then([&timer, &metrics](ContentResponse contentResponse) -> size_t
                        {
                        metrics.ReportPropertiesLoad(timer.GetCurrent().ToSeconds());
                        return GetTotalFieldsCount(contentResponse.GetResult()->GetDescriptor().GetVisibleFields());
                        });
                }).get();
        }
    return totalPropertiesCount;
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentPerformanceAnalysis::LoadFullContent(FullLoadPerformanceMetricsStorage& metrics, ECDbR project, ContentDescriptorCR descriptor)
    {
    StopWatch timer("", true);
    ContentResponse contentResponse = m_manager->GetContent(AsyncContentRequestParams::Create(project, descriptor)).get();
    double time = timer.GetCurrentSeconds();
    metrics.ReportLoad(contentResponse.GetResult()->GetContentSet().GetSize(), GetTotalFieldsCount(descriptor.GetVisibleFields()), time);
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentPerformanceAnalysis::LoadPagedContent(PagedLoadPerformanceMetricsStorage& metrics, ECDbR project, ContentDescriptorCR descriptor)
    {
    auto params = AsyncContentRequestParams::Create(project, descriptor);
    StopWatch allContentTimer("", true);
    std::vector<folly::Future<folly::Unit>> pagesFutures;
    ContentSetSizeResponse contentSetSizeResponse = m_manager->GetContentSetSize(params).get();
    size_t pagesCount = (*contentSetSizeResponse / CONTENT_REQUEST_PAGE_SIZE) + 1;
    for (size_t page = 0; page < pagesCount; ++page)
        {
        auto pageTimer = std::make_shared<StopWatch>("", false);
        auto contentPageParams = MakePaged(params, PageOptions(page * CONTENT_REQUEST_PAGE_SIZE, CONTENT_REQUEST_PAGE_SIZE));
        contentPageParams.SetTaskStartCallback([&metrics, pageTimer](){pageTimer->Start();});
        auto pageFuture = m_manager->GetContent(contentPageParams).then([&metrics, pageTimer](auto)
            {
            metrics.ReportPageLoad(pageTimer->GetCurrentSeconds());
            });
        pagesFutures.push_back(std::move(pageFuture));
        }
    folly::collect(pagesFutures).wait();
    metrics.ReportTotalTime(allContentTimer.GetCurrentSeconds());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
static bmap<ECClassCP, bvector<ECInstanceId>> GetElementKeys(ECDbCR project)
    {
    bmap<ECClassCP, bvector<ECInstanceId>> keys;
    ECSqlStatement stmt;
    stmt.Prepare(project, "SELECT ECInstanceId, ECClassId FROM bis.GeometricElement3d");
    while (BE_SQLITE_ROW == stmt.Step())
        {
        ECClassCP ecClass = project.Schemas().GetClass(stmt.GetValueId<ECClassId>(1));
        auto iter = keys.find(ecClass);
        if (keys.end() == iter)
            iter = keys.Insert(ecClass, bvector<ECInstanceId>()).first;
        iter->second.push_back(stmt.GetValueId<ECInstanceId>(0));
        }
    return keys;
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentPerformanceAnalysis, Run)
    {
    ForEachDatasetAndRuleset([&](Reporter& reporter, ECDbR project, Utf8StringCR rulesetId)
        {
        // first we query all elements in the dataset
        bmap<ECClassCP, bvector<ECInstanceId>> groupedElementKeys = GetElementKeys(project);
        bvector<ECClassInstanceKey> allElementKeys;
        for (auto const& entry : groupedElementKeys)
            ContainerHelpers::Push(allElementKeys, ContainerHelpers::TransformContainer<bvector<ECClassInstanceKey>>(entry.second, [ecClass = entry.first](ECInstanceId const& id){return ECClassInstanceKey(ecClass, id); }));
        NativeLogging::CategoryLogger(LOGGER_NAMESPACE).infov("    Dataset has %" PRIu64 " elements", (uint64_t)allElementKeys.size());

        // then we query properties for each of them
        ElementPropertiesLoadPerformanceMetricsStorage individualElementPropertiesLoadMetrics;
        StopWatch loadPropertiesForAllElementsTime("", true);
        size_t totalPropertiesCount = GetElementProperties(individualElementPropertiesLoadMetrics, project, rulesetId, allElementKeys);
        loadPropertiesForAllElementsTime.Stop();
        double totalTimeToLoadProperties = loadPropertiesForAllElementsTime.GetElapsed().ToSeconds();
        reporter.Record(REPORT_FIELD_TotalPropertiesCount, Json::Value((uint64_t)totalPropertiesCount));
        reporter.Record(REPORT_FIELD_AveragePropertiesCount, Json::Value((uint64_t)!allElementKeys.empty() ? (totalPropertiesCount / allElementKeys.size()) : 0));
        reporter.Record(REPORT_FIELD_TotalElementsCount, Json::Value((uint64_t)allElementKeys.size()));
        reporter.Record(REPORT_FIELD_AvgTimeToLoadInstanceProperties, Json::Value((uint64_t)!allElementKeys.empty() ? (totalTimeToLoadProperties / allElementKeys.size()) : 0));
        individualElementPropertiesLoadMetrics.Save(reporter);
        NativeLogging::CategoryLogger(LOGGER_NAMESPACE).infov("    Loaded properties for all elements individually in %.2f s", totalTimeToLoadProperties);

        // create a consolidated descriptor
        StopWatch createDescriptorTime("", true);
        ContentDescriptorResponse descriptorResponse = m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(project, rulesetId, RulesetVariables(),
            "", (int)ContentFlags::DescriptorOnly, *KeySet::Create(allElementKeys))).get();
        double timeToCreateDescriptor = createDescriptorTime.GetCurrent().ToSeconds();
        reporter.Record(REPORT_FIELD_TimeToCreateConsolidatedDescriptor, timeToCreateDescriptor);
        NativeLogging::CategoryLogger(LOGGER_NAMESPACE).infov("    Created consolidated content descriptor with %d properties in %.2f s",
            (int)GetTotalFieldsCount(descriptorResponse.GetResult()->GetVisibleFields()), timeToCreateDescriptor);

        // create content classes list
        StopWatch createContentClassesTime("", true);
        ContentClassesResponse contentClassesResponse = m_manager->GetContentClasses(AsyncContentClassesRequestParams::Create(project, rulesetId, RulesetVariables(),
            "", 0, bvector<ECClassCP>{ project.Schemas().GetClass("BisCore", "Element") })).get();
        double timeToGetContentClasses = createContentClassesTime.GetCurrent().ToSeconds();
        reporter.Record(REPORT_FIELD_TimeToGetContentClasses, timeToGetContentClasses);
        NativeLogging::CategoryLogger(LOGGER_NAMESPACE).infov("    Created %" PRIu64 " content classes in %.2f s",
            (uint64_t)contentClassesResponse.GetResult().size(), timeToGetContentClasses);

        // query content for each class separately
        FullLoadPerformanceMetricsStorage fullContentLoadMetrics;
        PagedLoadPerformanceMetricsStorage pagedContentLoadMetrics;
        for (auto const& entry : groupedElementKeys)
            {
            auto classKeys = ContainerHelpers::TransformContainer<bvector<ECClassInstanceKey>>(entry.second, [ecClass = entry.first](ECInstanceId const& id) {return ECClassInstanceKey(ecClass, id); });
            ContentDescriptorResponse classDescriptorResponse = m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(project, rulesetId, RulesetVariables(), "", 0, *KeySet::Create(classKeys))).get();
            LoadFullContent(fullContentLoadMetrics, project, **classDescriptorResponse);
            LoadPagedContent(pagedContentLoadMetrics, project, **classDescriptorResponse);
            }
        fullContentLoadMetrics.Save(reporter);
        pagedContentLoadMetrics.Save(reporter);
        NativeLogging::CategoryLogger(LOGGER_NAMESPACE).infov("    Loaded all content in %.2f s", fullContentLoadMetrics.GetTotalTime());
        NativeLogging::CategoryLogger(LOGGER_NAMESPACE).infov("    Loaded all content in pages in %.2f s", pagedContentLoadMetrics.GetTotalTime());
        });
    }
