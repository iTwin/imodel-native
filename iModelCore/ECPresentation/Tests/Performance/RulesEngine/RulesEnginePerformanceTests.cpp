/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "RulesEnginePerformanceTests.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesEnginePerformanceTests::SetUpTestCase()
    {
    NativeLogging::LoggingConfig::SetSeverity(LOGGER_NAMESPACE, NativeLogging::LOG_INFO);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesEnginePerformanceTests::SetUp()
    {
    ECPresentationTest::SetUp();
    Localization::Init();

    BeFileName assetsDirectory, temporaryDirectory;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(assetsDirectory);
    BeTest::GetHost().GetTempDir(temporaryDirectory);
    ECSchemaReadContext::Initialize(assetsDirectory);
    m_manager = new RulesDrivenECPresentationManager(m_connections, RulesDrivenECPresentationManager::Paths(assetsDirectory, temporaryDirectory));
    ECSchemaReadContext::Initialize(assetsDirectory);
    // set up presentation manager
    PresentationRuleSetPtr ruleset = _SupplyRuleset();
    if (!ruleset.IsValid())
        {
        BeAssert(false);
        return;
        }
    m_locater = TestRuleSetLocater::Create();
    m_manager->GetLocaters().RegisterLocater(*m_locater);
    m_locater->AddRuleSet(*ruleset);

    // open the project
    BeFileName projectPath = _SupplyProjectPath();
    if (!projectPath.DoesPathExist())
        {
        BeAssert(false);
        return;
        }
    if (BeSQLite::DbResult::BE_SQLITE_OK != m_project.OpenBeSQLiteDb(projectPath, BeSQLite::Db::OpenParams(BeSQLite::Db::OpenMode::Readonly)))
        {
        BeAssert(false);
        return;
        }
    m_connections.NotifyConnectionOpened(m_project);

    // we want to make sure all schemas in the dataset are fully loaded so that's not included in our performance test results
    m_project.Schemas().GetSchemas(true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesEnginePerformanceTests::TearDown()
    {
    delete m_manager;
    Localization::Terminate();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
RulesDrivenECPresentationManager::ContentOptions RulesEnginePerformanceTests::CreateContentOptions() const
    {
    return RulesDrivenECPresentationManager::ContentOptions(m_locater->GetRuleSetIds().front().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
RulesEnginePerformanceTests::Timer::Timer(Utf8CP name)
    : StopWatch(name ? name : BeTest::GetNameOfCurrentTest(), true)
    {
    NativeLogging::LoggingManager::GetLogger(LOGGER_NAMESPACE)->infov("Started:  %s", GetDescription());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
RulesEnginePerformanceTests::Timer::~Timer() {Finish();}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesEnginePerformanceTests::Timer::Finish()
    {
    if (GetElapsed().IsTowardsFuture())
        {
        // positive elapsed time means the timer is already finished
        return;
        }

    NativeLogging::LoggingManager::GetLogger(LOGGER_NAMESPACE)->infov("Finished: %s. Elapsed: %.2f", GetDescription(), GetCurrentSeconds());
    Stop();
    }