/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "PerformanceTests.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesEngineTests::SetUpTestCase()
    {
    NativeLogging::LoggingConfig::SetSeverity(LOGGER_NAMESPACE, NativeLogging::LOG_INFO);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesEngineTests::SetUp()
    {
    ECPresentationTest::SetUp();

    BeFileName assetsDirectory, temporaryDirectory;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(assetsDirectory);
    BeTest::GetHost().GetTempDir(temporaryDirectory);
    ECSchemaReadContext::Initialize(assetsDirectory);

    ECPresentationManager::Params params(ECPresentationManager::Paths(assetsDirectory, temporaryDirectory));
    ECPresentationManager::Params::CachingParams cachingParams;
    cachingParams.SetCacheDirectoryPath(temporaryDirectory);
    params.SetCachingParams(cachingParams);
    _ConfigureManagerParams(params);
    m_manager = new ECPresentationManager(params);

    // set up presentation manager
    m_locater = TestRuleSetLocater::Create();
    m_manager->GetLocaters().RegisterLocater(*m_locater);
    _SetupRulesets();

    // open the project
    _SetupProjects();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesEngineTests::_ConfigureManagerParams(ECPresentationManager::Params& params)
    {
    params.SetMultiThreadingParams(ECPresentationManager::Params::MultiThreadingParams(bpair<int, unsigned>(INT_MAX, 4)));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesEngineTests::TearDown()
    {
    delete m_manager;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesEngineSingleProjectTests::SetUpTestCase()
    {
    RulesEngineTests::SetUpTestCase();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesEngineSingleProjectTests::_SetupProjects()
    {
    BeFileName projectPath = _SupplyProjectPath();
    ASSERT_TRUE(projectPath.DoesPathExist());
    OpenProject(m_project, projectPath);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR RulesEngineSingleProjectTests::GetRulesetId() const
    {
    return m_locater->GetRuleSetIds().front();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Timer::Timer(Utf8CP name)
    : StopWatch(name ? name : BeTest::GetNameOfCurrentTest(), true)
    {
    NativeLogging::LoggingManager::GetLogger(LOGGER_NAMESPACE)->infov("Started:  %s", GetDescription());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Timer::~Timer() {Finish();}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
