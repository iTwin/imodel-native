/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Performance/RulesEngine/PresentationManagerStressTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <random>
#include <ECPresentation/Content.h>
#include "../PerformanceTests.h"
#include "../TestsHelper.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

#define NUM_OF_REQUESTS 10
#define ITEMS_RULESET "Items"
#define CUSTOM_RULESET "Custom"

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                02/2016
+===============+===============+===============+===============+===============+======*/
struct TestUpdateRecordsHandler : IUpdateRecordsHandler
{
protected:
    void _Start() override {}
    void _Accept(UpdateRecord const&) override {}
    void _Accept(FullUpdateRecord const&) override {}
    void _Finish() override {}
public:
    static RefCountedPtr<TestUpdateRecordsHandler> Create() {return new TestUpdateRecordsHandler();}
    };

/*=================================================================================**//**
* @bsiclass                                     Mantas.Kontrimas                07/2018
+===============+===============+===============+===============+===============+======*/
struct StressTests : RulesEngineTests
{
    ECDb m_project1;
    ECDb m_project2;
    bvector<std::function<folly::Future<folly::Unit>()>> m_functions;

    RefCountedPtr<TestECInstanceChangeEventsSource> m_eventsSource = TestECInstanceChangeEventsSource::Create();
    RefCountedPtr<TestUpdateRecordsHandler> m_updateRecordsHandler = TestUpdateRecordsHandler::Create();
    RuntimeJsonLocalState m_localState;

    // Data
    bvector<ECClassInstanceKey> m_project1GeometricElementKeys;
    bvector<ECClassInstanceKey> m_project2GeometricElementKeys;
    bvector<NavNodeCPtr> m_nodesPath1Items;
    bvector<NavNodeCPtr> m_nodesPath2Items;

    void SetUp() override
        {
        RulesEngineTests::SetUp();
        SetupRequests();
        }

    void TearDown() override
        {
        RulesEngineTests::TearDown();
        }

    virtual void _ConfigureManagerParams(RulesDrivenECPresentationManager::Params& params) override
        {
        RulesEngineTests::_ConfigureManagerParams(params);
        params.SetECInstanceChangeEventSources({ m_eventsSource });
        params.SetUpdateRecordsHandlers({ m_updateRecordsHandler });
        params.SetLocalState(&m_localState);
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

    void _SetupProjects() override
        {
        OpenFirstProject();
        OpenSecondProject();
        }

    PresentationRuleSetPtr CreateCustomRuleSet()
        {
        PresentationRuleSetPtr customRuleSet = PresentationRuleSet::CreateInstance(CUSTOM_RULESET, 1, 0, false, "", "", "", false);
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
        contentRule->AddSpecification(*new ContentInstancesOfSpecificClassesSpecification(1, "", "BisCore:Element", true));

        customRuleSet->AddPresentationRule(*settingsGroup);
        customRuleSet->AddPresentationRule(*rootNavRule);
        customRuleSet->AddPresentationRule(*childNodeRule);
        customRuleSet->AddPresentationRule(*contentRule);

        return customRuleSet;
        }

    void AddRulesets()
        {
        m_locater->AddRuleSet(*PresentationManagerTestsHelper::GetItemsRuleset());
        m_locater->AddRuleSet(*CreateCustomRuleSet());
        }

    void _SetupRulesets() override
        {
        AddRulesets();
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

        // Events:
        // [17]
        m_functions.push_back([this]()
            {
            ECInstanceId instanceId = m_project1GeometricElementKeys[0].GetId();
            ECClassCP ecClass = m_project1GeometricElementKeys[0].GetClass();
            m_eventsSource->NotifyECInstanceUpdated(m_project1, instanceId, *ecClass);
            return folly::makeFuture();
            });
        // [18]
        m_functions.push_back([this]()
            {
            ECInstanceId instanceId = m_project2GeometricElementKeys[0].GetId();
            ECClassCP ecClass = m_project2GeometricElementKeys[0].GetClass();
            m_eventsSource->NotifyECInstanceUpdated(m_project2, instanceId, *ecClass);
            return folly::makeFuture();
            });

        // [19]
        m_functions.push_back([this]()
            {
            m_manager->GetUserSettings(CUSTOM_RULESET).SetSettingBoolValue("TestSetting", false);
            return folly::makeFuture();
            });
        // [20]
        m_functions.push_back([this]()
            {
            m_manager->GetUserSettings(CUSTOM_RULESET).SetSettingBoolValue("TestSetting", true);
            return folly::makeFuture();
            });

        // [21]
        m_functions.push_back([this]()
            {
            m_project1.CloseDb();
            OpenFirstProject();
            m_project1GeometricElementKeys = PresentationManagerTestsHelper::GetGeometricElementKeys(m_project1);
            return PresentationManagerTestsHelper::GetNodesPath(*m_manager, m_project1, ITEMS_RULESET, 7).then([&](bvector<NavNodeCPtr> path){m_nodesPath1Items = path;});
            });
        // [22]
        m_functions.push_back([this]()
            {
            m_project2.CloseDb();

            OpenSecondProject();
            m_project2GeometricElementKeys = PresentationManagerTestsHelper::GetGeometricElementKeys(m_project2);
            return PresentationManagerTestsHelper::GetNodesPath(*m_manager, m_project2, ITEMS_RULESET, 7).then([&](bvector<NavNodeCPtr> path){m_nodesPath2Items = path;});
            });

        // [23]
        m_functions.push_back([this]()
            {
            m_locater->Clear();
            AddRulesets();
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
* @betest                                       Mantas.Kontrimas                07/2018
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
* @betest                                       Grigas.Petraitis                09/2019
+---------------+---------------+---------------+---------------+---------------+------*/
static bool GetStrategy(bvector<unsigned>& functionIndexes)
    {
    BeFileName strategyFileName;
    BeTest::GetHost().GetDocumentsRoot(strategyFileName);
    strategyFileName.AppendToPath(L"strategy.txt");
    if (!strategyFileName.DoesPathExist())
        return false;
        
    BeFile strategyFile;
    strategyFile.Open(strategyFileName.c_str(), BeFileAccess::Read);
    ByteStream bytes;
    if (BeFileStatus::Success != strategyFile.ReadEntireFile(bytes))
        return false;

    Utf8String content((Utf8CP)bytes.begin(), (Utf8CP)bytes.end());
    strategyFile.Close();

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
* @betest                                       Grigas.Petraitis                09/2019
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
* @betest                                       Mantas.Kontrimas                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(StressTests, ExecutingMultipleRequests)
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
        futures.push_back(std::move(RunFuture(m_functions[functionIndex], i)));
        }
    PresentationManagerTestsHelper::WaitForAllFutures(futures, false);
    }
