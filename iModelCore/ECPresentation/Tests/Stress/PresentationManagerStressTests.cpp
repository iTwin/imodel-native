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

    PresentationManagerStressTests() {}

    void SetUp() override
        {
        ECPresentationTest::SetUp();

        BeFileName assetsDirectory, temporaryDirectory;
        BeTest::GetHost().GetDgnPlatformAssetsDirectory(assetsDirectory);
        BeTest::GetHost().GetTempDir(temporaryDirectory);

        BeSQLite::BeSQLiteLib::Initialize(temporaryDirectory);
        ECSchemaReadContext::Initialize(assetsDirectory);

        ECPresentationManager::Params params(ECPresentationManager::Paths(assetsDirectory, temporaryDirectory));
        ECPresentationManager::Params::CachingParams cachingParams;
        cachingParams.SetCacheDirectoryPath(temporaryDirectory);
        params.SetCachingParams(cachingParams);
        params.SetECInstanceChangeEventSources({ m_eventsSource });
        params.SetUpdateRecordsHandlers({ m_updateRecordsHandler });
        params.SetLocalState(std::make_shared<ECPresentation::JsonLocalState>(std::make_shared<RuntimeLocalState>()));
        m_manager = new ECPresentationManager(params);

        // set up presentation manager
        m_locater = TestRuleSetLocater::Create();
        m_manager->GetLocaters().RegisterLocater(*m_locater);
        SetupRulesets();
        SetupProjects();
        SetupRequests();

        NativeLogging::ConsoleLogger::GetLogger().SetSeverity(LOGGER_NAMESPACE, NativeLogging::LOG_INFO);
        NativeLogging::Logging::SetLogger(&NativeLogging::ConsoleLogger::GetLogger());
        }

    void OpenProject(ECDbR project, BeFileNameCR projectPath)
        {
        if (!projectPath.DoesPathExist())
            {
            BeAssert(false);
            return;
            }
        if (BeSQLite::DbResult::BE_SQLITE_OK != project.OpenBeSQLiteDb(projectPath, BeSQLite::Db::OpenParams(BeSQLite::Db::OpenMode::ReadWrite)))
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
        // Functions population
        // Content:
        // [0]
        m_functions.push_back([this]()
            {
            return PresentationManagerTestsHelper::GetContent(*m_manager, m_project1, ITEMS_RULESET, nullptr, *KeySet::Create(PresentationManagerTestsHelper::GetGeometricElementKeys(m_project1)), ContentDisplayType::List);
            });
        // [1]
        m_functions.push_back([this]()
            {
            SelectionInfoCPtr selection = SelectionInfo::Create("ProviderName", false);
            return PresentationManagerTestsHelper::GetContent(*m_manager, m_project1, ITEMS_RULESET, selection.get(), *KeySet::Create(PresentationManagerTestsHelper::GetGeometricElementKeys(m_project1)), ContentDisplayType::List);
            });
        // [2]
        m_functions.push_back([this]()
            {
            return PresentationManagerTestsHelper::GetContent(*m_manager, m_project1, CUSTOM_RULESET, nullptr, *KeySet::Create(PresentationManagerTestsHelper::GetGeometricElementKeys(m_project1)), ContentDisplayType::List);
            });

        // [3]
        m_functions.push_back([this]()
            {
            return PresentationManagerTestsHelper::GetContent(*m_manager, m_project1, ITEMS_RULESET, nullptr, *KeySet::Create(PresentationManagerTestsHelper::GetGeometricElementKeys(m_project1)), ContentDisplayType::PropertyPane);
            });

        // [4]
        m_functions.push_back([this]()
            {
            return PresentationManagerTestsHelper::GetContent(*m_manager, m_project2, ITEMS_RULESET, nullptr, *KeySet::Create(PresentationManagerTestsHelper::GetGeometricElementKeys(m_project2)), ContentDisplayType::Graphics);
            });
        // [5]
        m_functions.push_back([this]()
            {
            SelectionInfoCPtr selection = SelectionInfo::Create("ProviderName1", false);
            return PresentationManagerTestsHelper::GetContent(*m_manager, m_project2, ITEMS_RULESET, selection.get(), *KeySet::Create(PresentationManagerTestsHelper::GetGeometricElementKeys(m_project2)), ContentDisplayType::Graphics);
            });
        // [6]
        m_functions.push_back([this]()
            {
            return PresentationManagerTestsHelper::GetContent(*m_manager, m_project2, CUSTOM_RULESET, nullptr, *KeySet::Create(PresentationManagerTestsHelper::GetGeometricElementKeys(m_project2)), ContentDisplayType::Graphics);
            });

        // [7]
        m_functions.push_back([this]()
            {
            return PresentationManagerTestsHelper::GetContent(*m_manager, m_project2, ITEMS_RULESET, nullptr, *KeySet::Create(PresentationManagerTestsHelper::GetGeometricElementKeys(m_project2)), ContentDisplayType::Grid);
            });
        // [8]
        m_functions.push_back([this](){return PresentationManagerTestsHelper::GetContentClassesForGeometricElement(*m_manager, m_project1, ITEMS_RULESET, ContentDisplayType::Grid);});
        // [9]
        m_functions.push_back([this]()
            {
            return PresentationManagerTestsHelper::GetContentSetSize(*m_manager, m_project1, ITEMS_RULESET, nullptr, *KeySet::Create(PresentationManagerTestsHelper::GetGeometricElementKeys(m_project1)), ContentDisplayType::List);
            });

        // Navigation:
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
            auto nodesPathItems = PresentationManagerTestsHelper::GetNodesPath(*m_manager, m_project1, ITEMS_RULESET, 7).get();
            return PresentationManagerTestsHelper::GetNodesCount(*m_manager, m_project1, nodesPathItems, ITEMS_RULESET);
            });

        // [15]
        m_functions.push_back([this]()
            {
            auto nodesPathItems = PresentationManagerTestsHelper::GetNodesPath(*m_manager, m_project2, ITEMS_RULESET, 7).get();
            return PresentationManagerTestsHelper::GetNodesCount(*m_manager, m_project2, nodesPathItems, ITEMS_RULESET);
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
            auto geometricElementKeys = PresentationManagerTestsHelper::GetGeometricElementKeys(m_project1);
            m_eventsSource->NotifyECInstanceUpdated(m_project1, geometricElementKeys[0]);
            return folly::makeFuture();
            });
        // [19]
        m_functions.push_back([this]()
            {
            auto geometricElementKeys = PresentationManagerTestsHelper::GetGeometricElementKeys(m_project2);
            m_eventsSource->NotifyECInstanceUpdated(m_project2, geometricElementKeys[0]);
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
            return folly::makeFuture();
            });
        // [23]
        m_functions.push_back([this]()
            {
            m_project2.CloseDb();
            OpenSecondProject();
            return folly::makeFuture();
            });

        // [24]
        m_functions.push_back([this]()
            {
            m_locater->Clear();
            SetupRulesets();
            return folly::makeFuture();
            });

        // [25]
        m_functions.push_back([this]()
            {
            PresentationManagerStressTests::ImportSchema(m_project1);
            return folly::makeFuture();
            });
        // [26]
        m_functions.push_back([this]()
            {
            PresentationManagerStressTests::ImportSchema(m_project2);
            return folly::makeFuture();
            });
        }

    static void ImportSchema(ECDbR ecdb)
        {
        auto schemaId = BeGuid(true).ToString();
        schemaId.ReplaceAll("-", "_");

        ECSchemaReadContextPtr schemaReadContext = ECSchemaReadContext::CreateContext();
        schemaReadContext->AddSchemaLocater(ecdb.GetSchemaLocater());

        Utf8PrintfString schemaXml(
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
            "<ECSchema schemaName=\"schema_%s\" alias=\"alias_%s\" version=\"1.0\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.3.1\">"
            "  <ECSchemaReference name=\"BisCore\" version=\"01.00.00\" alias=\"bis\" />"
            "  <ECEntityClass typeName=\"X\">"
            "    <BaseClass>bis:PhysicalElement</BaseClass>"
            "  </ECEntityClass>"
            "</ECSchema>",
            schemaId.c_str(), schemaId.c_str()
        );

        ECSchemaPtr schema;
        ECSchema::ReadFromXmlString(schema, schemaXml.c_str(), *schemaReadContext);
        EXPECT_TRUE(schema.IsValid());

        ecdb.Schemas().ImportSchemas(bvector<ECSchemaCP>{ schema.get() });
        ecdb.SaveChanges();
        }
};

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
static folly::Future<folly::Unit> RunFuture
(
    int requestId,
    std::function<folly::Future<folly::Unit>()> function,
    std::function<void(std::function<folly::Future<folly::Unit>()>)> onRestart
)
    {
    return function().then([requestId]()
        {
        NativeLogging::CategoryLogger(LOGGER_NAMESPACE).infov("Future %d finished successfully", requestId);
        })
    .onError([requestId, function, onRestart](folly::exception_wrapper const& e)
        {
        if (!e)
            FAIL() << "Invalid exception";
        try
            {
            e.throwException();
            }
        catch (CancellationException const& cancellation)
            {
            if (cancellation.IsRestartRequested())
                {
                NativeLogging::CategoryLogger(LOGGER_NAMESPACE).infov("Future %d was cancelled with request to restart", requestId);
                onRestart([requestId, function, onRestart]()
                    {
                    NativeLogging::CategoryLogger(LOGGER_NAMESPACE).infov("Future %d re-scheduled.", requestId);
                    return RunFuture(requestId, function, onRestart);
                    });
                }
            else
                {
                NativeLogging::CategoryLogger(LOGGER_NAMESPACE).infov("Future %d was cancelled.", requestId);
                }
            }
        catch (...)
            {
            NativeLogging::CategoryLogger(LOGGER_NAMESPACE).infov("Future %d finished with exception: %s", requestId, e.class_name().c_str());
            FAIL();
            }
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
    NativeLogging::CategoryLogger(LOGGER_NAMESPACE).info(strategy.c_str());
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

    BeMutex m;
    bvector<folly::Future<folly::Unit>> futures;
    bvector<std::function<folly::Future<folly::Unit>()>> futureRestarts;
    for (size_t i = 0; i < functionIndexes.size(); ++i)
        {
        int functionIndex = functionIndexes[i];
        NativeLogging::CategoryLogger(LOGGER_NAMESPACE).infov("Future %d (function index %d) scheduled.", i, functionIndex);
        futures.push_back(RunFuture(i, m_functions[functionIndex], [&futureRestarts, &m](auto futureRestartFunc)
            {
            BeMutexHolder lock(m);
            futureRestarts.push_back(futureRestartFunc);
            }));
        }

    while (!futures.empty())
        {
        PresentationManagerTestsHelper::WaitForAllFutures(futures, false);

        BeMutexHolder lock(m);
        futures.clear();
        for (size_t i = 0; i < futureRestarts.size(); ++i)
            futures.push_back(futureRestarts[i]());
        futureRestarts.clear();
        }

    NativeLogging::CategoryLogger(LOGGER_NAMESPACE).info("All futures completed.");
    }
