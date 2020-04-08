/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "PerformanceTests.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesEngineTests::SetUpTestCase()
    {
    NativeLogging::LoggingConfig::SetSeverity(LOGGER_NAMESPACE, NativeLogging::LOG_INFO);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesEngineTests::SetUp()
    {
    ECPresentationTest::SetUp();
    Localization::Init();

    BeFileName assetsDirectory, temporaryDirectory;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(assetsDirectory);
    BeTest::GetHost().GetTempDir(temporaryDirectory);
    ECSchemaReadContext::Initialize(assetsDirectory);

    RulesDrivenECPresentationManager::Params params(RulesDrivenECPresentationManager::Paths(assetsDirectory, temporaryDirectory));
    RulesDrivenECPresentationManager::Params::CachingParams cachingParams;
    cachingParams.SetCacheDirectoryPath(temporaryDirectory);
    params.SetCachingParams(cachingParams);
    _ConfigureManagerParams(params);
    m_manager = new RulesDrivenECPresentationManager(params);

    // set up presentation manager
    m_locater = TestRuleSetLocater::Create();
    m_manager->GetLocaters().RegisterLocater(*m_locater);
    _SetupRulesets();

    // open the project
    _SetupProjects();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesEngineTests::_ConfigureManagerParams(RulesDrivenECPresentationManager::Params& params)
    {
    params.SetLocalizationProvider(&m_localizationProvider);
    params.SetMultiThreadingParams(RulesDrivenECPresentationManager::Params::MultiThreadingParams(bpair<int, unsigned>(INT_MAX, 4)));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesEngineTests::TearDown()
    {
    delete m_manager;
    Localization::Terminate();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mantas.Kontrimas                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesEngineTests::OpenProject(ECDbR project, BeFileNameCR projectPath)
    {
    if (!projectPath.DoesPathExist())
        {
        BeAssert(false);
        return;
        }
    if (BeSQLite::DbResult::BE_SQLITE_OK != project.OpenBeSQLiteDb(projectPath, BeSQLite::Db::OpenParams(BeSQLite::Db::OpenMode::Readonly)))
        {
        BeAssert(false);
        return;
        }
    m_manager->GetConnections().CreateConnection(project);

    // we want to make sure all schemas in the dataset are fully loaded so that's not included in our performance test results
    project.Schemas().GetSchemas(true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesEngineSingleProjectTests::SetUpTestCase()
    {
    RulesEngineTests::SetUpTestCase();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mantas.Kontrimas                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesEngineSingleProjectTests::_SetupRulesets()
    {
    PresentationRuleSetPtr ruleset = _SupplyRuleset();
    if (!ruleset.IsValid())
        {
        BeAssert(false);
        return;
        }
    m_locater->AddRuleSet(*ruleset);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mantas.Kontrimas                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesEngineSingleProjectTests::_SetupProjects()
    {
    BeFileName projectPath = _SupplyProjectPath();
    ASSERT_TRUE(projectPath.DoesPathExist());
    OpenProject(m_project, projectPath);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
RulesDrivenECPresentationManager::ContentOptions RulesEngineSingleProjectTests::CreateContentOptions() const
    {
    return RulesDrivenECPresentationManager::ContentOptions(m_locater->GetRuleSetIds().front().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Timer::Timer(Utf8CP name)
    : StopWatch(name ? name : BeTest::GetNameOfCurrentTest(), true)
    {
    NativeLogging::LoggingManager::GetLogger(LOGGER_NAMESPACE)->infov("Started:  %s", GetDescription());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Timer::~Timer() {Finish();}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void Timer::Finish()
    {
    if (GetElapsed().IsTowardsFuture())
        {
        // positive elapsed time means the timer is already finished
        return;
        }

    NativeLogging::LoggingManager::GetLogger(LOGGER_NAMESPACE)->infov("Finished: %s. Elapsed: %.2f", GetDescription(), GetCurrentSeconds());
    Stop();
    }
