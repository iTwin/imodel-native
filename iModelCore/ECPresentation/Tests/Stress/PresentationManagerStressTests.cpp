/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "StressTests.h"
#include <UnitTests/ECPresentation/TestRuleSetLocater.h>
#include <UnitTests/ECPresentation/TestECInstanceChangeEventsSource.h>
#include <ECPresentation/Content.h>

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

#define NUM_OF_REQUESTS 10
#define ITEMS_RULESET "Items"
#define CUSTOM_RULESET "Custom"

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct TestUpdateRecordsHandler : IUpdateRecordsHandler
{
protected:
    void _Start() override {}
    void _Accept(HierarchyUpdateRecord const&) override {}
    void _Accept(FullUpdateRecord const&) override {}
    void _Finish() override {}
public:
    TestUpdateRecordsHandler() {}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct TestHierarchyChangeRecordsHandler : IHierarchyChangeRecordsHandler
    {
    protected:
        void _Start() override {}
        void _Accept(HierarchyChangeRecord const&) override {}
        void _Finish() override {}
    public:
        TestHierarchyChangeRecordsHandler() {}
    };

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct PresentationManagerStressTests : ECPresentationTest
{
    TestRuleSetLocaterPtr m_locater;
    ECPresentationManager* m_manager;

    ECDb m_project1;
    ECDb m_project2;
    bvector<std::function<folly::Future<folly::Unit>()>> m_functions;

    std::shared_ptr<TestECInstanceChangeEventsSource> m_eventsSource = std::make_shared<TestECInstanceChangeEventsSource>();
    std::shared_ptr<TestUpdateRecordsHandler> m_updateRecordsHandler = std::make_shared<TestUpdateRecordsHandler>();
    std::shared_ptr<TestHierarchyChangeRecordsHandler> m_changeRecordsHandler = std::make_shared<TestHierarchyChangeRecordsHandler>();
    RuntimeJsonLocalState m_localState;

    // Data
    bvector<ECClassInstanceKey> m_project1GeometricElementKeys;
    bvector<ECClassInstanceKey> m_project2GeometricElementKeys;
    bvector<NavNodeCPtr> m_nodesPath1Items;
    bvector<NavNodeCPtr> m_nodesPath2Items;

    void SetUp() override
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
        params.SetECInstanceChangeEventSources({ m_eventsSource });
        params.SetUpdateRecordsHandlers({ m_updateRecordsHandler });
        params.SetLocalState(&m_localState);
        m_manager = new ECPresentationManager(params);

        // set up presentation manager
        m_locater = TestRuleSetLocater::Create();
        m_manager->GetLocaters().RegisterLocater(*m_locater);
        SetupRulesets();
        SetupProjects();
        SetupRequests();
        }

    void OpenProject(ECDbR project, BeFileNameCR projectPath)
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

    void OpenFirstProject()
        {
        BeFileName directory;
        BeTest::GetHost().GetDocumentsRoot(directory);
        directory.AppendToPath(L"Performance");

        BeFileName project1Path(directory);
        project1Path.AppendToPath(L"Oakland.ibim");
        OpenProject(m_project1, project1Path);
        }

    void OpenSecondProject()
        {
        BeFileName directory;
        BeTest::GetHost().GetDocumentsRoot(directory);
        directory.AppendToPath(L"Performance");

        BeFileName projectPath2(directory);
        projectPath2.AppendToPath(L"Plant.bim");
        OpenProject(m_project2, projectPath2);
        }

    void SetupProjects()
        {
        OpenFirstProject();
        OpenSecondProject();
        }

    PresentationRuleSetPtr CreateCustomRuleSet()
        {
        PresentationRuleSetPtr customRuleSet = PresentationRuleSet::CreateInstance(CUSTOM_RULESET);
        UserSettingsGroupP settingsGroup = new UserSettingsGroup();
        settingsGroup->AddSettingsItem(*new UserSettingsItem("TestSetting", "TestSetting", "BoolValue", "false"));

        customRuleSet->AddPresentationRule(*new LabelOverride(R"(ThisNode.IsInstanceNode ANDALSO this.IsOfClass("Element", "BisCore") ANDALSO GetSettingBoolValue("TestSetting"))", 100, "this.CodeValue", ""));
        customRuleSet->AddPresentationRule(*new LabelOverride(R"(ThisNode.IsInstanceNode ANDALSO this.IsOfClass("Element", "BisCore") ANDALSO NOT GetSettingBoolValue("TestSetting"))", 100, "this.UserLabel", ""));

        RootNodeRule* rootNavRule = new RootNodeRule();
        rootNavRule->AddSpecification(*new InstanceNodesOfSpecificClassesSpecification(1, false, false, false, false, false, true, "", "BisCore:Model", true));

        ChildNodeRule* childNodeRule = new ChildNodeRule();
        childNodeRule->AddSpecification(*new RelatedInstanceNodesSpecification(1, false, false, false, false, false, false, false, 0,
            "", RequiredRelationDirection_Forward, "BisCore", "BisCore:ModelContainsElements", "BisCore:GeometricElement"));

        ContentRuleP contentRule = new ContentRule("", 1, true);
        contentRule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", "BisCore:Element", true, false));

        customRuleSet->AddPresentationRule(*settingsGroup);
        customRuleSet->AddPresentationRule(*rootNavRule);
        customRuleSet->AddPresentationRule(*childNodeRule);
        customRuleSet->AddPresentationRule(*contentRule);

        return customRuleSet;
        }

    void SetupRulesets()
        {
        m_locater->AddRuleSet(*PresentationManagerTestsHelper::GetItemsRuleset());
        m_locater->AddRuleSet(*CreateCustomRuleSet());
        }

    void SetupRequests()
        {
        // Preparation
        m_project1GeometricElementKeys = PresentationManagerTestsHelper::GetGeometricElementKeys(m_project1);
        m_project2GeometricElementKeys = PresentationManagerTestsHelper::GetGeometricElementKeys(m_project2);

        // Functions population
        // Content:
        // [0]
        m_functions.push_back([this]()
            {
            return PresentationManagerTestsHelper::GetContent(*m_manager, m_project1, ITEMS_RULESET, nullptr, *KeySet::Create(m_project1GeometricElementKeys), ContentDisplayType::List);
            });
        // [1]
        m_functions.push_back([this]()
            {
            SelectionInfoCPtr selection = SelectionInfo::Create("ProviderName", false);
            return PresentationManagerTestsHelper::GetContent(*m_manager, m_project1, ITEMS_RULESET, selection.get(), *KeySet::Create(m_project1GeometricElementKeys), ContentDisplayType::List);
            });
        // [2]
        m_functions.push_back([this]()
            {
            return PresentationManagerTestsHelper::GetContent(*m_manager, m_project1, CUSTOM_RULESET, nullptr, *KeySet::Create(m_project1GeometricElementKeys), ContentDisplayType::List);
            });

        // [3]
        m_functions.push_back([this]()
            {
            return PresentationManagerTestsHelper::GetContent(*m_manager, m_project1, ITEMS_RULESET, nullptr, *KeySet::Create(m_project1GeometricElementKeys), ContentDisplayType::PropertyPane);
            });

        // [4]
        m_functions.push_back([this]()
            {
            return PresentationManagerTestsHelper::GetContent(*m_manager, m_project2, ITEMS_RULESET, nullptr, *KeySet::Create(m_project2GeometricElementKeys), ContentDisplayType::Graphics);
            });
        // [5]
        m_functions.push_back([this]()
            {
            SelectionInfoCPtr selection = SelectionInfo::Create("ProviderName1", false);
            return PresentationManagerTestsHelper::GetContent(*m_manager, m_project2, ITEMS_RULESET, selection.get(), *KeySet::Create(m_project2GeometricElementKeys), ContentDisplayType::Graphics);
            });
        // [6]
        m_functions.push_back([this]()
            {
            return PresentationManagerTestsHelper::GetContent(*m_manager, m_project2, CUSTOM_RULESET, nullptr, *KeySet::Create(m_project2GeometricElementKeys), ContentDisplayType::Graphics);
            });

        // [7]
        m_functions.push_back([this]()
            {
            return PresentationManagerTestsHelper::GetContent(*m_manager, m_project2, ITEMS_RULESET, nullptr, *KeySet::Create(m_project2GeometricElementKeys), ContentDisplayType::Grid);
            });
        // [8]
        m_functions.push_back([this](){return PresentationManagerTestsHelper::GetContentClassesForGeometricElement(*m_manager, m_project1, ITEMS_RULESET, ContentDisplayType::Grid);});
        // [9]
        m_functions.push_back([this]()
            {
            return PresentationManagerTestsHelper::GetContentSetSize(*m_manager, m_project1, ITEMS_RULESET, nullptr, *KeySet::Create(m_project1GeometricElementKeys), ContentDisplayType::List);
            });

        // Navigation:
        m_nodesPath1Items = PresentationManagerTestsHelper::GetNodesPath(*m_manager, m_project1, ITEMS_RULESET, 7).get();
        m_nodesPath2Items = PresentationManagerTestsHelper::GetNodesPath(*m_manager, m_project2, ITEMS_RULESET, 7).get();

        // [10]
        m_functions.push_back([this]()
            {
            return PresentationManagerTestsHelper::GetFullHierarchy(*m_manager, m_project1, ITEMS_RULESET);
            });
        // [11]
        m_functions.push_back([this]()
            {
            return PresentationManagerTestsHelper::GetFullHierarchy(*m_manager, m_project1, CUSTOM_RULESET);
            });
        // [12]
        m_functions.push_back([this]()
            {
            return PresentationManagerTestsHelper::GetFullHierarchy(*m_manager, m_project2, ITEMS_RULESET);
            });
        // [13]
        m_functions.push_back([this]()
            {
            return PresentationManagerTestsHelper::GetFullHierarchy(*m_manager, m_project2, CUSTOM_RULESET);
            });

        // [14]
        m_functions.push_back([this]()
            {
            return PresentationManagerTestsHelper::GetNodesCount(*m_manager, m_project1, m_nodesPath1Items, ITEMS_RULESET);
            });

        // [15]
        m_functions.push_back([this]()
            {
            return PresentationManagerTestsHelper::GetNodesCount(*m_manager, m_project2, m_nodesPath2Items, ITEMS_RULESET);
            });

        // [16]
        m_functions.push_back([this]()
            {
            return PresentationManagerTestsHelper::FilterNodes(*m_manager, m_project1, ITEMS_RULESET, "ist");
            });
        // [17]
        m_functions.push_back([this]()
            {
            bvector<folly::Future<folly::Unit>> initHierarchies;
            initHierarchies.push_back(PresentationManagerTestsHelper::GetFullHierarchy(*m_manager, m_project1, ITEMS_RULESET));
            initHierarchies.push_back(PresentationManagerTestsHelper::GetFullHierarchy(*m_manager, m_project1, CUSTOM_RULESET));
            return folly::collect(initHierarchies).then([this]()
                {
                return m_manager->CompareHierarchies(AsyncHierarchyCompareRequestParams::Create(m_project1, m_changeRecordsHandler, ITEMS_RULESET, RulesetVariables(), CUSTOM_RULESET, RulesetVariables(), NavNodeKeyList())).then();
                });
            });

        // Events:
        // [18]
        m_functions.push_back([this]()
            {
            ECInstanceId instanceId = m_project1GeometricElementKeys[0].GetId();
            ECClassCP ecClass = m_project1GeometricElementKeys[0].GetClass();
            m_eventsSource->NotifyECInstanceUpdated(m_project1, ECClassInstanceKey(*ecClass, instanceId));
            return folly::makeFuture();
            });
        // [19]
        m_functions.push_back([this]()
            {
            ECInstanceId instanceId = m_project2GeometricElementKeys[0].GetId();
            ECClassCP ecClass = m_project2GeometricElementKeys[0].GetClass();
            m_eventsSource->NotifyECInstanceUpdated(m_project2, ECClassInstanceKey(*ecClass, instanceId));
            return folly::makeFuture();
            });

        // [20]
        m_functions.push_back([this]()
            {
            m_manager->GetUserSettings(CUSTOM_RULESET).SetSettingBoolValue("TestSetting", false);
            return folly::makeFuture();
            });
        // [21]
        m_functions.push_back([this]()
            {
            m_manager->GetUserSettings(CUSTOM_RULESET).SetSettingBoolValue("TestSetting", true);
            return folly::makeFuture();
            });

        // [22]
        m_functions.push_back([this]()
            {
            m_project1.CloseDb();
            OpenFirstProject();
            m_project1GeometricElementKeys = PresentationManagerTestsHelper::GetGeometricElementKeys(m_project1);
            return PresentationManagerTestsHelper::GetNodesPath(*m_manager, m_project1, ITEMS_RULESET, 7).then([&](bvector<NavNodeCPtr> path){m_nodesPath1Items = path;});
            });
        // [23]
        m_functions.push_back([this]()
            {
            m_project2.CloseDb();

            OpenSecondProject();
            m_project2GeometricElementKeys = PresentationManagerTestsHelper::GetGeometricElementKeys(m_project2);
            return PresentationManagerTestsHelper::GetNodesPath(*m_manager, m_project2, ITEMS_RULESET, 7).then([&](bvector<NavNodeCPtr> path){m_nodesPath2Items = path;});
            });

        // [24]
        m_functions.push_back([this]()
            {
            m_locater->Clear();
            SetupRulesets();
            m_project1GeometricElementKeys = PresentationManagerTestsHelper::GetGeometricElementKeys(m_project1);
            m_project2GeometricElementKeys = PresentationManagerTestsHelper::GetGeometricElementKeys(m_project2);
            std::vector<folly::Future<bvector<NavNodeCPtr>>> futures;
            futures.push_back(PresentationManagerTestsHelper::GetNodesPath(*m_manager, m_project1, ITEMS_RULESET, 7));
            futures.push_back(PresentationManagerTestsHelper::GetNodesPath(*m_manager, m_project2, ITEMS_RULESET, 7));
            return folly::collect(futures)
                .then([&](std::vector<bvector<NavNodeCPtr>> paths)
                {
                m_nodesPath1Items = paths[0];
                m_nodesPath2Items = paths[1];
                });
            });
        }
};

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
static folly::Future<folly::Unit> RunFuture(std::function<folly::Future<folly::Unit>()>& function, int requestId)
    {
    return function().then([requestId]()
        {
        NativeLogging::LoggingManager::GetLogger(LOGGER_NAMESPACE)->infov("Future %d finished successfuly", requestId);
        })
    .onError([requestId](folly::exception_wrapper const& e)
        {
        NativeLogging::LoggingManager::GetLogger(LOGGER_NAMESPACE)->infov("Future %d finished with exception: %s", requestId, e.class_name().c_str());
        });
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
static bool GetStrategy(bvector<unsigned>& functionIndexes)
    {
    BeFileName strategyFileName;
    BeTest::GetHost().GetDocumentsRoot(strategyFileName);
    strategyFileName.AppendToPath(L"strategy.txt");
    Utf8String content;
    if (SUCCESS != PresentationManagerTestsHelper::ReadFileContent(strategyFileName, content))
        return false;

    bvector<Utf8String> indexesStr;
    BeStringUtilities::Split(content.c_str(), " ", indexesStr);

    if (indexesStr.empty())
        return false;

    functionIndexes.clear();
    for (Utf8StringCR indexStr : indexesStr)
        functionIndexes.push_back(atoi(indexStr.c_str()));
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
static void PrintStrategy(bvector<unsigned> const& indexes)
    {
    Utf8String strategy("Using strategy: ");
    for (size_t i = 0; i < indexes.size(); ++i)
        {
        if (i > 0)
            strategy.append(" ");
        strategy.append(std::to_string(indexes[i]).c_str());
        }
    NativeLogging::LoggingManager::GetLogger(LOGGER_NAMESPACE)->info(strategy.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PresentationManagerStressTests, ExecutingMultipleRequests)
    {
    bvector<unsigned> functionIndexes;
    if (!GetStrategy(functionIndexes))
        {
        // Use random seed different every test run in order to test thread safety
        std::random_device rd;
        std::mt19937 mt(rd());
        std::uniform_int_distribution<unsigned> val(0, m_functions.size() - 1);
        for (int i = 0; i < NUM_OF_REQUESTS; ++i)
            functionIndexes.push_back(val(mt));
        }
    PrintStrategy(functionIndexes);

    bvector<folly::Future<folly::Unit>> futures;
    for (size_t i = 0; i < functionIndexes.size(); ++i)
        {
        int functionIndex = functionIndexes[i];
        NativeLogging::LoggingManager::GetLogger(LOGGER_NAMESPACE)->infov("Future %d (function index %d) scheduled.", i, functionIndex);
        futures.push_back(RunFuture(m_functions[functionIndex], i));
        }
    PresentationManagerTestsHelper::WaitForAllFutures(futures, false);
    }
