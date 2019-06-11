/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "RulesEnginePerformanceTests.h"
#include <Bentley\BeTextFile.h>
#include <Bentley\BeDirectoryIterator.h>
#include <ECPresentation/IECPresentationManager.h>

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

#define HIERARCHY_REQUEST_PAGE_SIZE 20
#define PAGE_LOAD_THRESHOLD 0.2
#define REPORT_FILE_SEPARATOR L","

/*=================================================================================**//**
* @bsiclass                                     Haroldas.Vitunskas              01/2019
+===============+===============+===============+===============+===============+======*/
struct HierarchyPerformanceAnalysis : ECPresentationTest
    {
    struct PageInfo
        {
        private:
            Utf8String m_nodePath;
            size_t m_pageNo;
            double m_timeToLoad;

            Utf8String GetNodePath(bvector<NavNodeCP> nodePath)
                {
                Utf8String nodePathLabels = "";
                for (NavNodeCP node : nodePath)
                    nodePathLabels = nodePathLabels.append("::").append(node->GetLabel());

                return nodePathLabels;
                }

        public:
            PageInfo() : m_nodePath() {}

            PageInfo(bvector<NavNodeCP> nodePath, size_t pageNo)
                : m_pageNo(pageNo), m_nodePath(GetNodePath(nodePath)) {}

            void SetLoadTime(double duration) { m_timeToLoad = duration; }

            WString ToCSV()
                {
                WCharCP csvFormat =
                    L"%s" REPORT_FILE_SEPARATOR     /*Node Path*/
                    L"%d" REPORT_FILE_SEPARATOR     /*Page number*/
                    L"%.4f";                        /*Time to load*/

                WString asCSV = L"";
                asCSV.Sprintf(csvFormat, WString(m_nodePath.c_str(), BentleyCharEncoding::Locale).c_str(),
                    m_pageNo,
                    m_timeToLoad);

                return asCSV;
                }

            size_t GetPageIndex() { return m_pageNo; }
        };

    struct HierarchyPerformanceResults
        {
        private:
            WString m_datasetFileName;
            WString m_rulesetFileName;
            double m_schemasLoadTime;
            double m_threshold;

            bvector<double> m_firstPageLoadDurations;
            bvector<double> m_notFirstPageLoadDurations;

            uint64_t m_totalNodes;
            RulesEnginePerformanceTests::Timer m_timerTotal;

            StopWatch m_timerPageLoad;
            PageInfo m_currentPage;
            bvector<PageInfo> m_pagesAboveThreshold;

            WString ToCSV()
                {
                WCharCP csvFormat =
                    L"%s"       REPORT_FILE_SEPARATOR   /*Dataset*/
                    L"%s"       REPORT_FILE_SEPARATOR   /*Ruleset ID*/
                    L"%.4f"     REPORT_FILE_SEPARATOR   /*Time to load all schemas*/
                    L"%" PRIu64 REPORT_FILE_SEPARATOR   /*Total nodes*/
                    L"%.4f"     REPORT_FILE_SEPARATOR   /*Total time*/
                    L"%d"       REPORT_FILE_SEPARATOR   /*Pages above threshold count*/
                    L"%.4f"     REPORT_FILE_SEPARATOR   /*Max time for first page*/
                    L"%.4f";                            /*Max time for not first page*/

                WString asCSV = L"";
                asCSV.Sprintf(csvFormat,
                    m_datasetFileName.c_str(),
                    m_rulesetFileName.c_str(),
                    m_schemasLoadTime,
                    m_totalNodes,
                    m_timerTotal.GetElapsedSeconds(),
                    m_pagesAboveThreshold.size(),
                    m_firstPageLoadDurations.empty() ? 0 : *std::max_element(m_firstPageLoadDurations.begin(), m_firstPageLoadDurations.end()),
                    m_notFirstPageLoadDurations.empty() ? 0 : *std::max_element(m_notFirstPageLoadDurations.begin(), m_notFirstPageLoadDurations.end()));

                return asCSV;
                }

            void ReportPages()
                {
                BeFileName path;
                BeTest::GetHost().GetDocumentsRoot(path);
                path.AppendToPath(L"Reports");
                if (!path.DoesPathExist())
                    BeFileName::CreateNewDirectory(path.GetName());

                WCharCP caseFormat = L"%s.%s.csv"; // Dataset.Ruleset.csv
                WPrintfString caseFileName(caseFormat, m_datasetFileName.c_str(), m_rulesetFileName.c_str());

                path.AppendToPath(caseFileName.c_str());

                BeFileStatus status;
                BeTextFilePtr pagesFile = BeTextFile::Open(status, path.GetName(), TextFileOpenType::Write, TextFileOptions::None);
                EXPECT_EQ(BeFileStatus::Success, status);

                WCharCP headers =
                    L"Parent node path"    REPORT_FILE_SEPARATOR
                    L"Page index"          REPORT_FILE_SEPARATOR
                    L"Load time";
                pagesFile->PutLine(headers, true);

                for (PageInfo info : m_pagesAboveThreshold)
                    pagesFile->PutLine(info.ToCSV().c_str(), true);

                pagesFile->Close();
                }

        public:
            HierarchyPerformanceResults(WString datasetName, WString rulesetName, double schemasLoadTime, double threshold)
                : m_datasetFileName(datasetName), m_rulesetFileName(rulesetName), m_schemasLoadTime(schemasLoadTime), m_threshold(threshold), m_totalNodes(0)
                {}

            void StartPageLoad(PageInfo page)
                {
                m_currentPage = page;
                m_timerPageLoad.Init(true);
                }

            void StopPageLoad()
                {
                m_timerPageLoad.Stop();
                double duration = m_timerPageLoad.GetElapsedSeconds();

                if (0 == m_currentPage.GetPageIndex())
                    m_firstPageLoadDurations.push_back(duration);
                else
                    m_notFirstPageLoadDurations.push_back(duration);

                if (duration >= m_threshold)
                    {
                    m_currentPage.SetLoadTime(duration);
                    m_pagesAboveThreshold.push_back(m_currentPage);
                    };
                }

            void IncrementNodeCount() { ++m_totalNodes; }

            void StartTotalTime() { m_timerTotal.Start(); }
            void StopTotalTime() { m_timerTotal.Finish(); }

            void LogReport()
                {
                NativeLogging::LoggingManager::GetLogger(LOGGER_NAMESPACE)->infov("Dataset: %s, Ruleset: %s",
                    Utf8String(m_datasetFileName.c_str()).c_str(), Utf8String(m_rulesetFileName.c_str()).c_str());
                NativeLogging::LoggingManager::GetLogger(LOGGER_NAMESPACE)->infov("Time to load all ECSchemas: %.4f", m_schemasLoadTime);
                NativeLogging::LoggingManager::GetLogger(LOGGER_NAMESPACE)->infov("Total nodes: %" PRIu64, m_totalNodes);
                NativeLogging::LoggingManager::GetLogger(LOGGER_NAMESPACE)->infov("Total time: %.4f", m_timerTotal.GetElapsedSeconds());
                NativeLogging::LoggingManager::GetLogger(LOGGER_NAMESPACE)->infov("First pages loaded: %d", m_firstPageLoadDurations.size());
                if (!m_firstPageLoadDurations.empty())
                    NativeLogging::LoggingManager::GetLogger(LOGGER_NAMESPACE)->infov("Max time to load the first page: %.4f", *std::max_element(m_firstPageLoadDurations.begin(), m_firstPageLoadDurations.end()));
                NativeLogging::LoggingManager::GetLogger(LOGGER_NAMESPACE)->infov("Non-first pages loaded: %d", m_notFirstPageLoadDurations.size());
                if (!m_notFirstPageLoadDurations.empty())
                    NativeLogging::LoggingManager::GetLogger(LOGGER_NAMESPACE)->infov("Max time to load non-first page: %.4f", *std::max_element(m_notFirstPageLoadDurations.begin(), m_notFirstPageLoadDurations.end()));
                NativeLogging::LoggingManager::GetLogger(LOGGER_NAMESPACE)->infov("Pages with load time above %.4f count: %d", m_threshold, m_pagesAboveThreshold.size());
                NativeLogging::LoggingManager::GetLogger(LOGGER_NAMESPACE)->infov("==============================================");
                }

            void ReportCase(BeTextFile& reportFile)
                {
                reportFile.PutLine(ToCSV().c_str(), true);

                if (m_pagesAboveThreshold.size() > 0)
                    ReportPages();
                }

            static BeTextFilePtr CreateReportFile(WCharCP reportFileName)
                {
                BeFileName path;
                BeTest::GetHost().GetDocumentsRoot(path);
                path.AppendToPath(L"Reports");
                if (!path.DoesPathExist())
                    BeFileName::CreateNewDirectory(path.GetName());

                path.AppendToPath(reportFileName);

                BeFileStatus status;
                BeTextFilePtr reportFile = BeTextFile::Open(status, path.GetName(), TextFileOpenType::Write, TextFileOptions::None);

                WCharCP headers =
                    L"Dataset"                  REPORT_FILE_SEPARATOR
                    L"Ruleset ID"               REPORT_FILE_SEPARATOR
                    L"Time to load all schemas" REPORT_FILE_SEPARATOR
                    L"Total nodes"              REPORT_FILE_SEPARATOR
                    L"Total time"               REPORT_FILE_SEPARATOR
                    L"Pages above threshold"    REPORT_FILE_SEPARATOR
                    L"Max time for first page"  REPORT_FILE_SEPARATOR
                    L"Max time for not first page";
                reportFile->PutLine(headers, true);

                return reportFile;
                }
        };

    ConnectionManager m_connections;
    RulesDrivenECPresentationManager* m_manager;
    TestRuleSetLocaterPtr m_locater;

    void GetAllNodes(HierarchyPerformanceResults& results, ECDbR project, bvector<NavNodeCP> nodePath, Utf8String rulesetId);
    NavNodesContainer GetNodes(ECDbR project, PageOptions const& pageOptions, Json::Value const& options, NavNodeCP parentNode);
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

    // set up presentation manager
    BeFileName assetsDirectory, temporaryDirectory;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(assetsDirectory);
    BeTest::GetHost().GetTempDir(temporaryDirectory);
    ECSchemaReadContext::Initialize(assetsDirectory);
    m_manager = new RulesDrivenECPresentationManager(m_connections, RulesDrivenECPresentationManager::Paths(assetsDirectory, temporaryDirectory));

    m_locater = TestRuleSetLocater::Create();
    m_manager->GetLocaters().RegisterLocater(*m_locater);
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
* @betest                                       Haroldas.Vitunskas              01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void HierarchyPerformanceAnalysis::GetAllNodes(HierarchyPerformanceResults& results, ECDbR project, bvector<NavNodeCP> nodePath, Utf8String rulesetId)
    {
    Json::Value options = RulesDrivenECPresentationManager::NavigationOptions(rulesetId, TargetTree_MainTree).GetJson();

    size_t pageCount = 1;
    size_t nodesCount = 0;

    results.StartPageLoad(PageInfo(nodePath, 0));

    if (nodePath.empty())
        nodesCount = m_manager->GetRootNodesCount(project, options).get();
    else
        nodesCount = m_manager->GetChildrenCount(project, *nodePath.back(), options).get();
    pageCount = (size_t)ceil(1.0 * nodesCount / HIERARCHY_REQUEST_PAGE_SIZE);

    for (size_t pageIndex = 0; pageIndex < pageCount; ++pageIndex)
        {
        PageOptions pageOptions;
        pageOptions.SetPageStart(pageIndex * HIERARCHY_REQUEST_PAGE_SIZE);
        pageOptions.SetPageSize(HIERARCHY_REQUEST_PAGE_SIZE);

        DataContainer<NavNodeCPtr> nodes = GetNodes(project, pageOptions, options, nodePath.empty() ? nullptr : nodePath.back());
        results.StopPageLoad();
        results.StartPageLoad(PageInfo(nodePath, pageIndex));

        for (size_t nodeIndex = 0; nodeIndex < nodes.GetSize(); ++nodeIndex)
            {
            NavNodeCPtr node = nodes[nodeIndex];
            results.IncrementNodeCount();
            if (node->HasChildren())
                {
                nodePath.push_back(node.get());
                GetAllNodes(results, project, nodePath, rulesetId);
                nodePath.pop_back();
                }
            }
        }

    results.StopPageLoad();
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Haroldas.Vitunskas              01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodesContainer HierarchyPerformanceAnalysis::GetNodes(ECDbR project, PageOptions const& pageOptions, Json::Value const& options, NavNodeCP parentNode)
    {
    if (nullptr == parentNode)
        return m_manager->GetRootNodes(project, pageOptions, options).get();

    return m_manager->GetChildren(project, *parentNode, pageOptions, options).get();
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Haroldas.Vitunskas              01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HierarchyPerformanceAnalysis, AnalyseAllCases)
    {
    // Set up
    BeTextFilePtr reportFile = HierarchyPerformanceResults::CreateReportFile(L"HierarchyPerformanceReport.csv");
    ASSERT_TRUE(reportFile.IsValid());

    BeFileName datasetsPath;
    BeTest::GetHost().GetDocumentsRoot(datasetsPath);
    datasetsPath.AppendToPath(L"Performance");

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

        // Load project
        ECDb project;
        if (BeSQLite::DbResult::BE_SQLITE_OK != project.OpenBeSQLiteDb(dataSetPath, BeSQLite::Db::OpenParams(BeSQLite::Db::OpenMode::Readonly)))
            {
            BeAssert(false);
            return;
            }
        m_connections.NotifyConnectionOpened(project);

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

            // Load ruleset
            PresentationRuleSetPtr ruleset;
            if (rulesetPath.GetExtension().EqualsI(L"xml"))
                ruleset = PresentationRuleSet::ReadFromXmlFile(rulesetPath);
            else if (rulesetPath.GetExtension().EqualsI(L"json"))
                ruleset = PresentationRuleSet::ReadFromJsonFile(rulesetPath);
            if (ruleset.IsNull())
                continue;

            m_locater->AddRuleSet(*ruleset);

            HierarchyPerformanceResults results(dataSetPath.GetFileNameWithoutExtension(),
                rulesetPath.GetFileNameWithoutExtension(), schemasLoadTime.GetElapsed().ToSeconds(), PAGE_LOAD_THRESHOLD);
            results.StartTotalTime();
            GetAllNodes(results, project, bvector<NavNodeCP>(), ruleset->GetRuleSetId());
            results.StopTotalTime();
            results.LogReport();
            results.ReportCase(*reportFile);
            }

        project.CloseDb();
        }

    reportFile->Close();
    }
