/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <Bentley/BeDirectoryIterator.h>
#include "PerformanceAnalysisTests.h"
#include "../../../Source/PresentationManagerImpl.h"
#include "../../../Source/Hierarchies/NavNodesCache.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesEnginePerformanceAnalysisTests::SetUpTestCase()
    {
    NativeLogging::ConsoleLogger::GetLogger().SetSeverity(LOGGER_NAMESPACE, NativeLogging::LOG_INFO);
    NativeLogging::Logging::SetLogger(&NativeLogging::ConsoleLogger::GetLogger());

    BeFileName tempDir;
    BeTest::GetHost().GetTempDir(tempDir);
    BeSQLite::BeSQLiteLib::Initialize(tempDir);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesEnginePerformanceAnalysisTests::SetUp()
    {
    ECPresentationTest::SetUp();
    m_reporter = std::make_unique<Reporter>(m_reporterConfig.fieldNames);
    LoadRulesetConfigs();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesEnginePerformanceAnalysisTests::TearDown()
    {
    SaveReport();
    ECPresentationTest::TearDown();
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
Reporter& RulesEnginePerformanceAnalysisTests::_GetJsonReporter()
    {
    return *m_reporter;
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
Reporter& RulesEnginePerformanceAnalysisTests::_GetCsvReporter()
    {
    return *m_reporter;
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus RulesEnginePerformanceAnalysisTests::ParseRulesetConfig(JsonValueCR json, RulesetConfig& config)
    {
    bool hasAnyData = false;
    if (json.hasMember("maxDepth"))
        {
        hasAnyData = true;
        config.maxDepth = json["maxDepth"].asInt(-1);
        }
    return hasAnyData ? SUCCESS : ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesEnginePerformanceAnalysisTests::LoadRulesetConfigs()
    {
    BeFileName configFilePath;
    BeTest::GetHost().GetDocumentsRoot(configFilePath);
    configFilePath.AppendToPath(L"Rulesets").AppendToPath(L"Hierarchy").AppendToPath(RULESETS_CONFIG_FILENAME);
    Utf8String jsonString;
    PresentationManagerTestsHelper::ReadFileContent(configFilePath, jsonString);

    Json::Value json = Json::Reader::DoParse(jsonString);
    if (!json.isObject())
        return;

    for (auto iter = json.begin(); iter != json.end(); iter++)
        {
        Utf8String rulesetId = iter.memberName();
        JsonValueCR value = *iter;

        RulesetConfig config;
        if (SUCCESS != ParseRulesetConfig(value, config))
            {
            NativeLogging::CategoryLogger(LOGGER_NAMESPACE).errorv("Invalid config encountered for ruleset - %s", rulesetId.c_str());
            continue;
            }
        m_rulesetConfigs.Insert(rulesetId, config);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesEnginePerformanceAnalysisTests::SaveReport()
    {
    BeFileName reportPath;
    BeTest::GetHost().GetOutputRoot(reportPath);
    reportPath.AppendToPath(L"Reports");
    if (!reportPath.DoesPathExist())
        BeFileName::CreateNewDirectory(reportPath.GetName());
    reportPath.AppendToPath(WString(m_reporterConfig.fileName.c_str(), true).c_str());

    _GetJsonReporter().ToJsonFile(BeFileName(reportPath).AppendExtension(L"json"), m_reporterConfig.jsonGroupingFieldNames);
    _GetCsvReporter().ToExportCsvFile(
        BeFileName(reportPath).AppendExtension(L"export").AppendExtension(L"csv"),
        BeTest::GetNameOfCurrentTestCase(),
        m_reporterConfig.csvTestNameFieldNames,
        m_reporterConfig.csvExportedFieldNames);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SingleManagerRulesEnginePerformanceAnalysisTests::SetUp()
    {
    RulesEnginePerformanceAnalysisTests::SetUp();
    m_locater = TestRuleSetLocater::Create();
    Reset();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SingleManagerRulesEnginePerformanceAnalysisTests::TearDown()
    {
    DELETE_AND_CLEAR(m_manager);
    m_locater = nullptr;
    RulesEnginePerformanceAnalysisTests::TearDown();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SingleManagerRulesEnginePerformanceAnalysisTests::Reset(ECDb* project)
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
    threadAllocations.Insert(INT_MAX, m_config.threadsCount);

    ECPresentationManager::Params params(ECPresentationManager::Paths(assetsDirectory, temporaryDirectory));
    params.SetMultiThreadingParams(threadAllocations);
    params.SetLocalState(&m_localState);
    ECPresentationManager::Params::CachingParams cachingParams;
    cachingParams.SetCacheDirectoryPath(temporaryDirectory);
    params.SetCachingParams(cachingParams);
    m_manager = new ECPresentationManager(params);
    m_manager->GetLocaters().RegisterLocater(*SupplementalRuleSetLocater::Create(*DirectoryRuleSetLocater::Create(supplementalRulesetsDirectory.GetNameUtf8().c_str())));
    m_manager->GetLocaters().RegisterLocater(*m_locater);
    if (nullptr != project)
        m_manager->GetConnections().CreateConnection(*project);
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
void SingleManagerRulesEnginePerformanceAnalysisTests::ForEachDatasetAndRuleset(std::function<void(Reporter&, ECDbR, Utf8StringCR)> testCaseRunner)
    {
    BeFileName datasetsPath;
    BeTest::GetHost().GetDocumentsRoot(datasetsPath);
    datasetsPath.AppendToPath(L"Datasets");

    BeFileName rulesetsPath;
    BeTest::GetHost().GetDocumentsRoot(rulesetsPath);
    rulesetsPath.AppendToPath(L"Rulesets");
    rulesetsPath.AppendToPath(WString(m_config.rulesetsSubDirectory.c_str(), true).c_str());

    BeDirectoryIterator datasets(datasetsPath);
    BeFileName dataSetPath;
    bool isDirectory;
    for (; 0 == datasets.GetCurrentEntry(dataSetPath, isDirectory); datasets.ToNext())
        {
        if (isDirectory || dataSetPath.GetExtension().EqualsI(L"json"))
            continue;

        NativeLogging::CategoryLogger(LOGGER_NAMESPACE).infov(L"Dataset: %s", dataSetPath.GetFileNameAndExtension().c_str());

        // load project
        ECDb project;
        BeSQLite::DbResult result;
        if (BeSQLite::DbResult::BE_SQLITE_OK != (result = project.OpenBeSQLiteDb(dataSetPath, BeSQLite::Db::OpenParams(BeSQLite::Db::OpenMode::Readonly))))
            {
            BeAssert(false);
            NativeLogging::CategoryLogger(LOGGER_NAMESPACE).errorv("  Dataset open failed with: %d", (int)result);
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
            if (isDirectory || rulesetPath.GetExtension().EqualsI(L"config"))
                continue;

            Reset(&project);

            // load ruleset
            NativeLogging::CategoryLogger(LOGGER_NAMESPACE).infov(L"  Ruleset: %s", rulesetPath.GetFileNameAndExtension().c_str());
            PresentationRuleSetPtr ruleset;
            if (rulesetPath.GetExtension().EqualsI(L"xml"))
                ruleset = PresentationRuleSet::ReadFromXmlFile(rulesetPath);
            else if (rulesetPath.GetExtension().EqualsI(L"json"))
                ruleset = PresentationRuleSet::ReadFromJsonFile(rulesetPath);
            if (ruleset.IsNull())
                {
                BeAssert(false);
                NativeLogging::CategoryLogger(LOGGER_NAMESPACE).error("  Ruleset read failed");
                continue;
                }
            m_locater->AddRuleSet(*ruleset);

            m_reporter->Next();
            m_reporter->Record(REPORT_FIELD_Dataset, Utf8String(BeFileName(project.GetDbFileName(), true).GetFileNameAndExtension().c_str()));
            m_reporter->Record(REPORT_FIELD_RulesetId, ruleset->GetRuleSetId());
            m_reporter->Record(REPORT_FIELD_TimeToLoadAllSchemas, schemasLoadTime.GetElapsed().ToSeconds());

            testCaseRunner(*m_reporter, project, ruleset->GetRuleSetId());
            }

        project.AbandonChanges();
        project.CloseDb();
        }
    }
