/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "StressTests.h"
#include "../../Source/TaskScheduler.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

#define STRATEGY_FILENAME       L"TaskManagerStressTestStrategy.json"
#define MAX_TASK_EXECUTION_TIME 200
#define MAX_WORKER_THREADS      4

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct TestActionExecuteResult
    {
    folly::Future<folly::Unit> future;
    bool isChained;
    };

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct TaskManagerStressTestAction
    {
    virtual ~TaskManagerStressTestAction() {}
    virtual TestActionExecuteResult _Execute(ECPresentationTasksManager&, folly::Future<folly::Unit>*) const = 0;
    virtual rapidjson::Document _ToJson(rapidjson::Document::AllocatorType*) const = 0;
    static std::unique_ptr<TaskManagerStressTestAction> FromJson(RapidJsonValueCR);
    template<typename TRandomizer> static std::unique_ptr<TaskManagerStressTestAction> CreateRandom(TRandomizer&);
    };

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct TaskManagerStressTestTaskAction : TaskManagerStressTestAction
    {
    static Utf8CP const s_type;
    bool m_isCancellable;
    unsigned m_execTime;
    template<typename TRandomizer>
    static std::unique_ptr<TaskManagerStressTestAction> CreateRandom(TRandomizer& randomizer)
        {
        auto action = std::make_unique<TaskManagerStressTestTaskAction>();
        action->m_isCancellable = (bool)(std::uniform_int_distribution<>(0, 1)(randomizer));
        action->m_execTime = std::uniform_int_distribution<unsigned>(0, MAX_TASK_EXECUTION_TIME)(randomizer);
        return action;
        }
    static std::unique_ptr<TaskManagerStressTestAction> FromJson(RapidJsonValueCR json)
        {
        auto action = std::make_unique<TaskManagerStressTestTaskAction>();
        action->m_isCancellable = json["IsCancellable"].GetBool();
        action->m_execTime = json["ExecTime"].GetUint();
        return action;
        }
    rapidjson::Document _ToJson(rapidjson::Document::AllocatorType* allocator) const override
        {
        rapidjson::Document json(allocator);
        json.SetObject();
        json.AddMember("Type", rapidjson::StringRef(s_type), json.GetAllocator());
        json.AddMember("IsCancellable", m_isCancellable, json.GetAllocator());
        json.AddMember("ExecTime", m_execTime, json.GetAllocator());
        return json;
        }
    TestActionExecuteResult _Execute(ECPresentationTasksManager& manager, folly::Future<folly::Unit>*) const override
        {
        ECPresentationTaskParams params;
        params.SetIsCancelable(m_isCancellable);
        auto future = manager.CreateAndExecute([this](IECPresentationTaskCR)
            {
            BeThreadUtilities::BeSleep(m_execTime);
            }, params);
        return { std::move(future), false };
        }
    };
Utf8CP const TaskManagerStressTestTaskAction::s_type = "Task";

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct TaskManagerStressTestChainedTaskAction : TaskManagerStressTestAction
    {
    static Utf8CP const s_type;
    unsigned m_execTime;
    template<typename TRandomizer>
    static std::unique_ptr<TaskManagerStressTestAction> CreateRandom(TRandomizer& randomizer)
        {
        auto action = std::make_unique<TaskManagerStressTestChainedTaskAction>();
        action->m_execTime = std::uniform_int_distribution<unsigned>(0, MAX_TASK_EXECUTION_TIME)(randomizer);
        return action;
        }
    static std::unique_ptr<TaskManagerStressTestAction> FromJson(RapidJsonValueCR json)
        {
        auto action = std::make_unique<TaskManagerStressTestChainedTaskAction>();
        action->m_execTime = json["ExecTime"].GetUint();
        return action;
        }
    rapidjson::Document _ToJson(rapidjson::Document::AllocatorType* allocator) const override
        {
        rapidjson::Document json(allocator);
        json.SetObject();
        json.AddMember("Type", rapidjson::StringRef(s_type), json.GetAllocator());
        json.AddMember("ExecTime", m_execTime, json.GetAllocator());
        return json;
        }
    TestActionExecuteResult _Execute(ECPresentationTasksManager& manager, folly::Future<folly::Unit>* prev) const override
        {
        auto future = prev 
            ? prev->then([this]()
                {
                BeThreadUtilities::BeSleep(m_execTime);
                }) 
            : folly::makeFuture();
        return { std::move(future), true };
        }
    };
Utf8CP const TaskManagerStressTestChainedTaskAction::s_type = "ChainedTask";

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct TaskManagerStressTestRestartAction : TaskManagerStressTestAction
    {
    static Utf8CP const s_type;
    template<typename TRandomizer>
    static std::unique_ptr<TaskManagerStressTestAction> CreateRandom(TRandomizer&)
        {
        auto action = std::make_unique<TaskManagerStressTestRestartAction>();
        return action;
        }
    static std::unique_ptr<TaskManagerStressTestAction> FromJson(RapidJsonValueCR json)
        {
        auto action = std::make_unique<TaskManagerStressTestRestartAction>();
        return action;
        }
    rapidjson::Document _ToJson(rapidjson::Document::AllocatorType* allocator) const override
        {
        rapidjson::Document json(allocator);
        json.SetObject();
        json.AddMember("Type", rapidjson::StringRef(s_type), json.GetAllocator());
        return json;
        }
    TestActionExecuteResult _Execute(ECPresentationTasksManager& manager, folly::Future<folly::Unit>*) const override
        {
        auto result = manager.Restart([](IECPresentationTaskCR){ return true; });
        return { result.GetCompletion().then(), false };
        }
    };
Utf8CP const TaskManagerStressTestRestartAction::s_type = "Restart";

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct TaskManagerStressTestCancelPreviousAction : TaskManagerStressTestAction
    {
    static Utf8CP const s_type;
    template<typename TRandomizer>
    static std::unique_ptr<TaskManagerStressTestAction> CreateRandom(TRandomizer&)
        {
        return std::make_unique<TaskManagerStressTestCancelPreviousAction>();
        }
    static std::unique_ptr<TaskManagerStressTestAction> FromJson(RapidJsonValueCR json)
        {
        auto action = std::make_unique<TaskManagerStressTestCancelPreviousAction>();
        return action;
        }
    rapidjson::Document _ToJson(rapidjson::Document::AllocatorType* allocator) const override
        {
        rapidjson::Document json(allocator);
        json.SetObject();
        json.AddMember("Type", rapidjson::StringRef(s_type), json.GetAllocator());
        return json;
        }
    TestActionExecuteResult _Execute(ECPresentationTasksManager&, folly::Future<folly::Unit>* prev) const override
        {
        if (prev)
            prev->cancel();
        return { folly::unit, false };
        }
    };
Utf8CP const TaskManagerStressTestCancelPreviousAction::s_type = "CancelPrevious";

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct TaskManagerStressTestCancelWithPredicateAction : TaskManagerStressTestAction
    {
    static Utf8CP const s_type;
    template<typename TRandomizer>
    static std::unique_ptr<TaskManagerStressTestAction> CreateRandom(TRandomizer&)
        {
        auto action = std::make_unique<TaskManagerStressTestCancelWithPredicateAction>();
        return action;
        }
    static std::unique_ptr<TaskManagerStressTestAction> FromJson(RapidJsonValueCR json)
        {
        auto action = std::make_unique<TaskManagerStressTestCancelWithPredicateAction>();
        return action;
        }
    rapidjson::Document _ToJson(rapidjson::Document::AllocatorType* allocator) const override
        {
        rapidjson::Document json(allocator);
        json.SetObject();
        json.AddMember("Type", rapidjson::StringRef(s_type), json.GetAllocator());
        return json;
        }
    TestActionExecuteResult _Execute(ECPresentationTasksManager& manager, folly::Future<folly::Unit>*) const override
        {
        auto result = manager.Cancel([](IECPresentationTaskCR){return true;});
        return { result.GetCompletion().then(), false };
        }
    };
Utf8CP const TaskManagerStressTestCancelWithPredicateAction::s_type = "CancelWithPredicate";

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::unique_ptr<TaskManagerStressTestAction> TaskManagerStressTestAction::FromJson(RapidJsonValueCR json)
    {
    typedef std::unique_ptr<TaskManagerStressTestAction> TFactory(RapidJsonValueCR);
    std::map<Utf8String, TFactory*> factories = {
        std::make_pair(TaskManagerStressTestTaskAction::s_type, &TaskManagerStressTestTaskAction::FromJson),
        std::make_pair(TaskManagerStressTestChainedTaskAction::s_type, &TaskManagerStressTestChainedTaskAction::FromJson),
        std::make_pair(TaskManagerStressTestCancelPreviousAction::s_type, &TaskManagerStressTestCancelPreviousAction::FromJson),
        std::make_pair(TaskManagerStressTestCancelWithPredicateAction::s_type, &TaskManagerStressTestCancelWithPredicateAction::FromJson),
        };
    auto iter = factories.find(json["Type"].GetString());
    if (factories.end() != iter)
        return iter->second(json);
    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename TRandomizer>
std::unique_ptr<TaskManagerStressTestAction> TaskManagerStressTestAction::CreateRandom(TRandomizer& randomizer)
    {
    typedef std::unique_ptr<TaskManagerStressTestAction> TFactory(TRandomizer&);
    bvector<TFactory*> factories = {
        &TaskManagerStressTestTaskAction::CreateRandom,
        &TaskManagerStressTestChainedTaskAction::CreateRandom,
        &TaskManagerStressTestCancelPreviousAction::CreateRandom,
        &TaskManagerStressTestCancelWithPredicateAction::CreateRandom,
        };
    auto factory = factories[(size_t)std::uniform_int_distribution<unsigned>(0, factories.size() - 1)(randomizer)];
    return factory(randomizer);
    }

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct TaskManagerStressTestStrategy
    {
    unsigned m_threadsCount;
    bvector<std::unique_ptr<TaskManagerStressTestAction>> m_actions;

    static BeFileName GetStrategyFilePath()
        {
        BeFileName path;
        BeTest::GetHost().GetDocumentsRoot(path);
        path.AppendToPath(STRATEGY_FILENAME);
        return path;
        }

    rapidjson::Document ToJson() const
        {
        rapidjson::Document json(rapidjson::kObjectType);
        json.AddMember("ThreadsCount", m_threadsCount, json.GetAllocator());
        json.AddMember("Actions", rapidjson::Value(rapidjson::kArrayType), json.GetAllocator());
        for (auto const& action : m_actions)
            json["Actions"].PushBack(action->_ToJson(&json.GetAllocator()), json.GetAllocator());
        return json;
        }
    void ToFile() const
        {
        BeFile f;
        auto jsonString = BeRapidJsonUtilities::ToPrettyString(ToJson());
        EXPECT_EQ(BeFileStatus::Success, f.Create(GetStrategyFilePath().c_str()));
        EXPECT_EQ(BeFileStatus::Success, f.Write(nullptr, &jsonString[0], jsonString.size()));
        EXPECT_EQ(BeFileStatus::Success, f.Close());
        }
    void ToConsole() const
        {
        auto jsonString = BeRapidJsonUtilities::ToPrettyString(ToJson());
        printf("%s\n", jsonString.c_str());
        }

    static std::unique_ptr<TaskManagerStressTestStrategy> FromJson(RapidJsonValueCR json)
        {
        auto strategy = std::make_unique<TaskManagerStressTestStrategy>();
        if (json.HasMember("ThreadsCount"))
            strategy->m_threadsCount = json["ThreadsCount"].GetUint();
        if (json.HasMember("Actions"))
            {
            for (rapidjson::SizeType i = 0; i < json["Actions"].Size(); ++i)
                {
                auto action = TaskManagerStressTestAction::FromJson(json["Actions"][i]);
                if (action)
                    strategy->m_actions.push_back(std::move(action));
                }
            }
        return strategy;
        }
    static std::unique_ptr<TaskManagerStressTestStrategy> FromFile()
        {
        auto path = GetStrategyFilePath();
        if (!path.DoesPathExist())
            return nullptr;

        BeFile f;
        if (BeFileStatus::Success != f.Open(path.c_str(), BeFileAccess::Read))
            return nullptr;

        bvector<Byte> fileContents;
        if (BeFileStatus::Success != f.ReadEntireFile(fileContents))
            {
            f.Close();
            return nullptr;
            }
        f.Close();

        rapidjson::Document json;
        json.Parse((Utf8CP)&*fileContents.begin(), fileContents.size());
        if (json.IsNull())
            return nullptr;

        return FromJson(json);
        }

    static std::unique_ptr<TaskManagerStressTestStrategy> CreateRandom()
        {
        std::random_device rd;
        std::mt19937 mt(rd());
        auto strategy = std::make_unique<TaskManagerStressTestStrategy>();
        strategy->m_threadsCount = std::uniform_int_distribution<unsigned>(1, MAX_WORKER_THREADS)(mt);
        for (size_t i = 0; i < 10; ++i)
            strategy->m_actions.push_back(TaskManagerStressTestAction::CreateRandom(mt));
        return strategy;
        }
    };

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct TaskManagerStressTests : ECPresentationTest
    {
    static void SetUpTestCase()
        {
        BeFileName tempDir;
        BeTest::GetHost().GetTempDir(tempDir);
        BeSQLiteLib::Initialize(tempDir);
        }

    std::unique_ptr<TaskManagerStressTestStrategy> GetStrategy()
        {
        auto strategy = TaskManagerStressTestStrategy::FromFile();
        if (strategy != nullptr)
            {
            printf("Loaded strategy from file\n");
            return strategy;
            }

        strategy = TaskManagerStressTestStrategy::CreateRandom();
        strategy->ToConsole();
        return strategy;
        }

    void ExecuteStrategy(TaskManagerStressTestStrategy const& s)
        {
        TThreadAllocationsMap threadAllocations;
        threadAllocations.Insert(INT_MAX, s.m_threadsCount);
        auto taskManager = std::make_unique<ECPresentationTasksManager>(threadAllocations);

        std::vector<TestActionExecuteResult> results;
        for (auto const& action : s.m_actions)
            results.push_back(action->_Execute(*taskManager, results.empty() ? nullptr : &results.back().future));

        std::vector<folly::Future<folly::Unit>> futures;
        for (size_t i = 0; i < results.size(); ++i)
            {
            if (i == results.size() - 1 || !results[i + 1].isChained)
                futures.push_back(std::move(results[i].future));
            }
        folly::collectAll(futures).wait();

        taskManager = nullptr;
        }
    };

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(TaskManagerStressTests, ExecutingMultipleRequests)
    {
    auto strategy = GetStrategy();
    ExecuteStrategy(*strategy);
    }
