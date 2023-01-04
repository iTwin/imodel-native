/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include <folly/futures/InlineExecutor.h>
#include <folly/BeFolly.h>
#include <ECPresentation/ECPresentationManager.h>
#include <ECPresentation/DefaultECPresentationSerializer.h>
#include "Content/ContentCache.h"
#include "Hierarchies/NavNodesDataSource.h"
#include "Hierarchies/NodePathsHelper.h"
#include "Shared/ValueHelpers.h"
#include "PresentationManagerImpl.h"
#include "TaskScheduler.h"

IECPresentationSerializer const* ECPresentationManager::s_serializer = nullptr;

#define CALL_TASK_START_CALLBACK(params) \
    if (params.GetTaskStartCallback()) { params.GetTaskStartCallback()(); }

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ECPresentationManager::ECInstanceChangeEventSourceWrapper : ECInstanceChangeEventSource, ECInstanceChangeEventSource::IEventHandler
{
private:
    ECPresentationManager& m_manager;
    std::shared_ptr<ECInstanceChangeEventSource> m_wrapped;
protected:
    void _OnClassUsed(ECDbCR db, ECClassCR ecClass, bool polymorphically) override
        {
        // wip: may need to pass to DgnClientFx executor
        m_wrapped->NotifyClassUsed(db, ecClass, polymorphically);
        }
    void _OnECInstancesChanged(ECDbCR db, bvector<ChangedECInstance> changes) override
        {
        IConnectionCacheCR connections = m_manager.GetConnections();
        IConnectionPtr connection = connections.GetConnection(db);
        if (connection.IsNull())
            {
            // don't forward the event if connection is not tracked
            return;
            }
        ECPresentationTaskParams taskParams;
        taskParams.SetDependencies(TaskDependencies{std::make_shared<TaskDependencyOnConnection>(connection->GetId())});
        taskParams.SetOtherTasksBlockingPredicate([connectionId = connection->GetId()](IECPresentationTaskCR task)
            {
            return task.GetDependencies().Has(TaskDependencyOnConnection(connectionId));
            });
        m_manager.GetTasksManager().CreateAndExecute([&, connectionId = connection->GetId(), changes](IECPresentationTaskR task)
            {
            IConnectionPtr taskConnection = connections.GetConnection(connectionId.c_str());
            task.SetTaskConnection(*taskConnection);
            NotifyECInstancesChanged(taskConnection->GetECDb(), changes);
            }, taskParams);
        }
public:
    ECInstanceChangeEventSourceWrapper(ECPresentationManager& manager, std::shared_ptr<ECInstanceChangeEventSource> wrapped)
        : m_manager(manager), m_wrapped(std::move(wrapped))
        {
        m_wrapped->RegisterEventHandler(*this);
        }
    ~ECInstanceChangeEventSourceWrapper()
        {
        m_wrapped->UnregisterEventHandler(*this);
        }
    ECInstanceChangeEventSource& GetWrappedEventSource() const {return *m_wrapped;}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ECPresentationManager::ConnectionManagerWrapper : IConnectionManager, IConnectionsListener
{
    enum InterruptAction
        {
        CANCEL,
        RESTART,
        };

    struct InterruptResult
        {
        TasksCancelationResult cancelation;
        RefCountedPtr<ECPresentationTasksBlocker> blocker;
        };

private:
    ECPresentationTasksManager& m_tasksManager;
    std::shared_ptr<IConnectionManager> m_wrapped;
    mutable bmap<int, bset<IConnectionsListener*>> m_listeners;
    mutable BeMutex m_mutex;
    bmap<Utf8String, RefCountedPtr<ECPresentationTasksBlocker>> m_suspendedConnectionBlockers;

private:
    InterruptResult BlockInterruptTasks(InterruptAction action, IConnectionCR connection)
        {
        BeMutexHolder lock(m_tasksManager.GetMutex());
        auto tasksPredicate = [&](IECPresentationTaskCR task)
            {
            return task.GetDependencies().Has(TaskDependencyOnConnection(connection.GetId()));
            };
        TasksCancelationResult cancelation =
            (CANCEL == action) ? m_tasksManager.Cancel(tasksPredicate) :
            (RESTART == action) ? m_tasksManager.Restart(tasksPredicate) :
            TasksCancelationResult(bset<IECPresentationTaskCPtr>());
        auto blocker = m_tasksManager.Block([connectionId = connection.GetId(), tasks = cancelation.GetTasks()](IECPresentationTaskCR task)
            {
            return task.GetDependencies().Has(TaskDependencyOnConnection(connectionId));
            });
        return InterruptResult{ std::move(cancelation), blocker };
        }

protected:
    IConnection* _GetConnection(Utf8CP connectionId) const override {return m_wrapped->GetConnection(connectionId);}
    IConnection* _GetConnection(ECDbCR ecdb) const override {return m_wrapped->GetConnection(ecdb);}
    IConnectionPtr _CreateConnection(ECDbR ecdb) override {return m_wrapped->CreateConnection(ecdb);}
    void _CloseConnections() override {m_wrapped->CloseConnections();}

    void _AddListener(IConnectionsListener& listener) const override
        {
        BeMutexHolder lock(m_mutex);
        auto iter = m_listeners.find(listener.GetPriority());
        if (m_listeners.end() == iter)
            iter = m_listeners.Insert(listener.GetPriority(), bset<IConnectionsListener*>()).first;
        iter->second.insert(&listener);
        }

    void _DropListener(IConnectionsListener& listener) const override
        {
        BeMutexHolder lock(m_mutex);
        auto iter = m_listeners.find(listener.GetPriority());
        if (m_listeners.end() != iter)
            iter->second.erase(&listener);
        }

    void _OnConnectionEvent(ConnectionEvent const& evt) override
        {
        RefCountedPtr<ECPresentationTasksBlocker> connectionCloseBlocker;
        if (ConnectionEventType::Closed == evt.GetEventType())
            {
            auto result = BlockInterruptTasks(InterruptAction::CANCEL, evt.GetConnection());
            connectionCloseBlocker = result.blocker;
            result.cancelation.GetCompletion().wait();
            }
        else if (ConnectionEventType::Suspended == evt.GetEventType())
            {
            auto result = BlockInterruptTasks(InterruptAction::RESTART, evt.GetConnection());
            BeMutexHolder blockersLock(m_mutex);
            m_suspendedConnectionBlockers.Insert(evt.GetConnection().GetId(), result.blocker);
            blockersLock.unlock();
            result.cancelation.GetCompletion().wait();
            }
        else if (ConnectionEventType::Resumed == evt.GetEventType())
            {
            BeMutexHolder blockersLock(m_mutex);
            m_suspendedConnectionBlockers.erase(evt.GetConnection().GetId());
            }

        BeMutexHolder listenerLock(m_mutex);
        bmap<int, bset<IConnectionsListener*>> listeners = m_listeners;
        listenerLock.unlock();
        for (auto entry : listeners)
            {
            for (IConnectionsListener* listener : entry.second)
                listener->NotifyConnectionEvent(evt);
            }
        }

public:
    ConnectionManagerWrapper(ECPresentationManager& manager, std::shared_ptr<IConnectionManager> wrapped)
        : m_tasksManager(manager.GetTasksManager()),
        m_wrapped(nullptr != wrapped ? wrapped : std::make_shared<ConnectionManager>())
        {
        m_wrapped->AddListener(*this);
        }
    ~ConnectionManagerWrapper()
        {
        m_wrapped->DropListener(*this);
        }
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ECPresentationManager::RulesetLocaterManagerWrapper : IRulesetLocaterManager, IRulesetCallbacksHandler
{
private:
    ECPresentationTasksManager& m_tasksManager;
    RuleSetLocaterManager m_wrapped;

protected:
    // IRulesetLocaterManager
    void _InvalidateCache(Utf8CP rulesetId) override {m_wrapped.InvalidateCache(rulesetId);}
    void _RegisterLocater(RuleSetLocater& locater) override {m_wrapped.RegisterLocater(locater);}
    void _UnregisterLocater(RuleSetLocater& locater) override {m_wrapped.UnregisterLocater(locater);}
    bvector<PresentationRuleSetPtr> _LocateRuleSets(IConnectionCR connection, Utf8CP rulesetId) const override {return m_wrapped.LocateRuleSets(connection, rulesetId);}
    bvector<Utf8String> _GetRuleSetIds() const override {return m_wrapped.GetRuleSetIds();}
    BeMutex& _GetMutex() const override { return m_wrapped.GetMutex(); }

    // IRulesetCallbacksHandler
    void _OnRulesetDispose(RuleSetLocaterCR locater, PresentationRuleSetR ruleset) override
        {
        if (nullptr == GetRulesetCallbacksHandler())
            return;

        m_tasksManager.Cancel([&](IECPresentationTaskCR task)
            {
            return task.GetDependencies().Has(TaskDependencyOnRuleset(ruleset.GetRuleSetId()));
            }).GetCompletion().wait();
        GetRulesetCallbacksHandler()->_OnRulesetDispose(locater, ruleset);
        }
    void _OnRulesetCreated(RuleSetLocaterCR locater, PresentationRuleSetR ruleset) override
        {
        if (nullptr == GetRulesetCallbacksHandler())
            return;

        GetRulesetCallbacksHandler()->_OnRulesetCreated(locater, ruleset);
        }

public:
    RulesetLocaterManagerWrapper(ECPresentationManager& manager, IConnectionManagerCR connections)
        : m_tasksManager(manager.GetTasksManager()), m_wrapped(connections)
        {
        m_wrapped.SetRulesetCallbacksHandler(this);
        }
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ECPresentationManager::UserSettingsManagerWrapper : IUserSettingsManager, IUserSettingsChangeListener
{
private:
    ECPresentationTasksManager& m_tasksManager;
    UserSettingsManager m_wrapped;
protected:
    IUserSettings& _GetSettings(Utf8StringCR rulesetId) const override {return m_wrapped.GetSettings(rulesetId);}
    void _OnLocalStateChanged() override {m_wrapped.SetLocalState(GetLocalState());}
    void _OnSettingChanged(Utf8CP rulesetId, Utf8CP settingId) const override
        {
        if (nullptr == GetChangesListener())
            return;

        ECPresentationTaskParams taskParams;
        taskParams.SetOtherTasksBlockingPredicate([](IECPresentationTaskCR)
            {
            // we don't know which connections this task can affect - need to block everything..
            return true;
            });
        taskParams.SetDependencies(TaskDependencies{std::make_shared<TaskDependencyOnConnection>("*")});
        m_tasksManager.CreateAndExecute([&, rulesetId = Utf8String(rulesetId), settingId = Utf8String(settingId)](IECPresentationTaskCR)
            {
            BeMutexHolder lock(GetMutex());
            GetChangesListener()->_OnSettingChanged(rulesetId.c_str(), settingId.c_str());
            }, taskParams);
        }
public:
    UserSettingsManagerWrapper(ECPresentationManager& manager, BeFileNameCR temporaryDirectory)
        : m_wrapped(temporaryDirectory), m_tasksManager(manager.GetTasksManager())
        {
        m_wrapped.SetChangesListener(this);
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::unique_ptr<ECPresentationManager::ImplParams> ECPresentationManager::CreateImplParams(ECPresentationManager::Params const& source)
    {
    auto result = std::make_unique<ECPresentationManager::ImplParams>(source);
    result->SetConnections(std::make_shared<ECPresentationManager::ConnectionManagerWrapper>(*this, source.GetConnections()));
    result->SetUserSettings(std::make_shared<ECPresentationManager::UserSettingsManagerWrapper>(*this, source.GetPaths().GetTemporaryDirectory()));
    result->SetRulesetLocaters(std::make_shared<ECPresentationManager::RulesetLocaterManagerWrapper>(*this, *result->GetConnections()));

    bvector<std::shared_ptr<ECInstanceChangeEventSource>> ecInstanceChangeEventSources;
    for (std::shared_ptr<ECInstanceChangeEventSource> const& evtSource : source.GetECInstanceChangeEventSources())
        {
        auto wrapper = std::make_shared<ECPresentationManager::ECInstanceChangeEventSourceWrapper>(*this, evtSource);
        ecInstanceChangeEventSources.push_back(wrapper);
        }
    result->SetECInstanceChangeEventSources(ecInstanceChangeEventSources);

    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECPresentationManager::ECPresentationManager(Params const& params)
    {
    m_tasksManager = new ECPresentationTasksManager(params.GetMultiThreadingParams().GetBackgroundThreadAllocations());
    RulesDrivenECPresentationManagerImpl* impl = new RulesDrivenECPresentationManagerImpl(*CreateImplParams(params));
    m_impl = impl;
    impl->Initialize();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECPresentationManager::~ECPresentationManager()
    {
    GetImpl().GetConnections().CloseConnections();
    DELETE_AND_CLEAR(m_tasksManager);
    DELETE_AND_CLEAR(m_impl);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ECPresentationManager::SetImpl(Impl& impl)
    {
    DELETE_AND_CLEAR(m_impl);
    m_impl = &impl;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ECPresentationManager::SetSerializer(IECPresentationSerializer const* serializer)
    {
    DELETE_AND_CLEAR(s_serializer);
    s_serializer = serializer;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
IECPresentationSerializer const& ECPresentationManager::GetSerializer()
    {
    if (nullptr == s_serializer)
        {
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Serialization, LOG_ERROR, "Attempting to use serializer, but it's not set. Using default.");
        SetSerializer(new DefaultECPresentationSerializer());
        }
    return *s_serializer;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
unsigned ECPresentationManager::GetBackgroundThreadsCount() const {return m_tasksManager->GetThreadsCount();}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
IUserSettingsManager& ECPresentationManager::GetUserSettings() const {return m_impl->GetUserSettingsManager();}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
IConnectionManagerCR ECPresentationManager::GetConnections() const {return m_impl->GetConnections();}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
IConnectionManagerR ECPresentationManager::GetConnections() {return m_impl->GetConnections();}

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
IECPropertyFormatter const& ECPresentationManager::GetFormatter() const {return m_impl->GetECPropertyFormatter();}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
IConnectionCR ECPresentationManager::GetConnection(ECDbCR db) const
    {
    auto scope = Diagnostics::Scope::Create("Get task connection");
    IConnectionCPtr connection = GetConnections().GetConnection(db);
    if (connection.IsNull())
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Connections, Utf8PrintfString("Did not find a connection for given ECDb `%s`", db.GetDbFileName()));
    if (!connection->IsOpen())
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Connections, Utf8PrintfString("Found a connection for given ECDb `%s`, but it's not open", db.GetDbFileName()));
    return *connection;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
IConnectionCR ECPresentationManager::GetConnection(Utf8CP connectionId) const
    {
    auto scope = Diagnostics::Scope::Create("Get task connection");
    IConnectionCPtr connection = GetConnections().GetConnection(connectionId);
    if (connection.IsNull())
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Connections, Utf8PrintfString("Did not find a connection with given ID `%s`", connectionId));
    if (!connection->IsOpen())
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Connections, Utf8PrintfString("Found a connection with given ID `%s`, but it's not open", connectionId));
    return *connection;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<folly::Unit> ECPresentationManager::GetTasksCompletion() const
    {
    return m_tasksManager->GetAllTasksCompletion();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static ECPresentationTaskParams CreateNodesTaskParams(ECPresentationManager::Impl const& impl, IConnectionCR connection, AsyncHierarchyRequestParams const& requestParams)
    {
    ECPresentationTaskParams taskParams;

    BeGuid parentId;
    if (requestParams.GetParentNode())
        {
        parentId = requestParams.GetParentNode()->GetNodeId();
        }
    else if (requestParams.GetParentNodeKey())
        {
        // note: we may not find the node by key here, but it's okay - in that case we lock
        // the whole hierarchy starting from the root, because we'll need to create it up until
        // the requested parent node is found
        auto cache = impl.GetHierarchyCache(connection.GetId());
        NavNodeCPtr parentNode = cache->LocateNode(connection, requestParams.GetRulesetId().c_str(), *requestParams.GetParentNodeKey());
        if (parentNode.IsValid())
            parentId = parentNode->GetNodeId();
        }

    taskParams.SetDependencies(TaskDependencies
        {
        std::make_shared<TaskDependencyOnConnection>(connection.GetId()),
        std::make_shared<TaskDependencyOnRuleset>(requestParams.GetRulesetId()),
        std::make_shared<TaskDependencyOnParentNode>(parentId)
        });
    taskParams.SetIsCancelable(true);
    taskParams.SetPriority(requestParams.GetRequestPriority());

    // set predicate which checks if this task blocks other tasks
    taskParams.SetOtherTasksBlockingPredicate([parentId](auto const& task)
        {
        return task.GetDependencies().Has(TaskDependencyOnParentNode(parentId));
        });

    // set predicate which checks if this task is blocked
    taskParams.SetThisTaskBlockingPredicate([&impl, connectionId = connection.GetId(), identifier = CombinedHierarchyLevelIdentifier(connection.GetId(), requestParams.GetRulesetId(), parentId)]()
        {
        auto cache = impl.GetHierarchyCache(connectionId);
        return nullptr != cache && cache->IsHierarchyLevelLocked(identifier);
        });
    return taskParams;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename TParams>
static ImplTaskParams<TParams> CreateImplRequestParams(TParams const& managerParams, IConnectionCR connection, ICancelationTokenCR cancellationToken)
    {
    return ImplTaskParams<TParams>::Create(connection, &cancellationToken, managerParams);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename TParams>
static ImplTaskParams<TParams> CreateImplRequestParams(WithAsyncTaskParams<TParams> const& managerParams, IConnectionCR connection, ICancelationTokenCR cancellationToken)
    {
    return ImplTaskParams<TParams>::Create(connection, &cancellationToken, managerParams);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename TParams>
static WithPageOptions<ImplTaskParams<TParams>> CreateImplRequestParams(WithPageOptions<WithAsyncTaskParams<TParams>> const& managerParams, IConnectionCR connection, ICancelationTokenCR cancellationToken)
    {
    WithPageOptions<ImplTaskParams<TParams>> implParams(CreateImplRequestParams((TParams const&)managerParams, connection, cancellationToken));
    implParams.SetPageOptions(managerParams.GetPageOptions());
    return implParams;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<NodesResponse> ECPresentationManager::GetNodes(WithPageOptions<AsyncHierarchyRequestParams> const& params)
    {
    auto diagnostics = Diagnostics::Scope::Create("Get child nodes");
    Diagnostics::SetCapturedAttributes({ DIAGNOSTICS_SCOPE_ATTRIBUTE_Rules });

    IConnectionCR connection = GetConnection(params.GetECDb());
    return m_tasksManager->CreateAndExecute<NodesResponse>([&, params, connectionId = connection.GetId()](auto& task) mutable
        {
        CALL_TASK_START_CALLBACK(params);

        IConnectionCR taskConnection = GetConnection(connectionId.c_str());
        task.SetTaskConnection(taskConnection);

        INavNodesDataSourcePtr source = m_impl->GetNodes(CreateImplRequestParams(params, taskConnection, *task.GetCancelationToken()));
        if (source.IsValid())
            return NavNodesContainer(*ConstNodesDataSource::Create(*source));
        return NavNodesContainer();
        }, CreateNodesTaskParams(*m_impl, connection, params));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<NodesCountResponse> ECPresentationManager::GetNodesCount(AsyncHierarchyRequestParams const& params)
    {
    auto diagnostics = Diagnostics::Scope::Create("Get child nodes count");
    Diagnostics::SetCapturedAttributes({ DIAGNOSTICS_SCOPE_ATTRIBUTE_Rules });

    IConnectionCR connection = GetConnection(params.GetECDb());
    return m_tasksManager->CreateAndExecute<NodesCountResponse>([&, params, connectionId = connection.GetId()](auto& task) mutable
        {
        CALL_TASK_START_CALLBACK(params);

        IConnectionCR taskConnection = GetConnection(connectionId.c_str());
        task.SetTaskConnection(taskConnection);

        return m_impl->GetNodesCount(CreateImplRequestParams(params, taskConnection, *task.GetCancelationToken()));
        }, CreateNodesTaskParams(*m_impl, connection, params));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<NodePathsResponse> ECPresentationManager::GetNodePaths(AsyncNodePathsFromFilterTextRequestParams const& params)
    {
    auto diagnostics = Diagnostics::Scope::Create("Get filtered node paths");
    Diagnostics::SetCapturedAttributes({ DIAGNOSTICS_SCOPE_ATTRIBUTE_Rules });

    IConnectionCR connection = GetConnection(params.GetECDb());

    ECPresentationTaskParams taskParams;
    taskParams.SetDependencies({
        std::make_shared<TaskDependencyOnConnection>(connection.GetId()),
        std::make_shared<TaskDependencyOnRuleset>(params.GetRulesetId())
        });
    taskParams.SetPriority(params.GetRequestPriority());
    taskParams.SetOtherTasksBlockingPredicate([connectionId = connection.GetId(), rulesetId = params.GetRulesetId()](auto const& task)
        {
        return task.GetDependencies().Has(TaskDependencyOnConnection(connectionId)) && task.GetDependencies().Has(TaskDependencyOnRuleset(rulesetId));
        });

    return m_tasksManager->CreateAndExecute<NodePathsResponse>([&, params, connectionId = connection.GetId()](auto& task) mutable
        {
        CALL_TASK_START_CALLBACK(params);

        IConnectionCR taskConnection = GetConnection(connectionId.c_str());
        task.SetTaskConnection(taskConnection);

        Utf8String escapedString(params.GetFilterText());
        escapedString.ToLower();

        auto implParams = CreateImplRequestParams(params, taskConnection, *task.GetCancelationToken());
        implParams.SetFilterText(escapedString);

        bvector<NavNodeCPtr> nodes = m_impl->GetFilteredNodes(implParams);
        auto parentGetter = [&](NavNodeCR child) -> NavNodeCPtr
            {
            return m_impl->GetParent(NodeParentRequestImplParams::Create(taskConnection, task.GetCancelationToken(), params.GetRulesetId(), params.GetRulesetVariables(), child));
            };
        return NodePathsHelper::CreateHierarchy(nodes, parentGetter, escapedString);
        }, taskParams);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<NodePathsResponse> ECPresentationManager::GetNodePaths(AsyncNodePathsFromInstanceKeyPathsRequestParams const& params)
    {
    auto diagnostics = Diagnostics::Scope::Create("Get node paths");
    Diagnostics::SetCapturedAttributes({ DIAGNOSTICS_SCOPE_ATTRIBUTE_Rules });

    IConnectionCR connection = GetConnection(params.GetECDb());

    ECPresentationTaskParams taskParams;
    taskParams.SetDependencies({
        std::make_shared<TaskDependencyOnConnection>(connection.GetId()),
        std::make_shared<TaskDependencyOnRuleset>(params.GetRulesetId())
        });
    taskParams.SetPriority(params.GetRequestPriority());
    taskParams.SetOtherTasksBlockingPredicate([connectionId = connection.GetId(), rulesetId = params.GetRulesetId()](auto const& task)
        {
        return task.GetDependencies().Has(TaskDependencyOnConnection(connectionId)) && task.GetDependencies().Has(TaskDependencyOnRuleset(rulesetId));
        });
    return m_tasksManager->CreateAndExecute<NodePathsResponse>([&, params, connectionId = connection.GetId()](auto& task) mutable
        {
        CALL_TASK_START_CALLBACK(params);

        IConnectionCR taskConnection = GetConnection(connectionId.c_str());
        task.SetTaskConnection(taskConnection);

        auto pathParamsBase = NodePathFromInstanceKeyPathRequestImplParams::Create(taskConnection, task.GetCancelationToken(), HierarchyRequestParams(params), bvector<ECInstanceKey>());
        auto paths = ContainerHelpers::TransformContainer<bvector<NodesPathElement>>(params.GetInstanceKeyPaths(), [&](auto const& keyPath)
            {
            auto pathParams = pathParamsBase;
            pathParams.SetInstanceKeyPath(keyPath);
            return NodePathsHelper::CreateNodePath(*m_impl, pathParams);
            });
        return NodePathsHelper::MergePaths(paths, params.GetMarkedIndex());
        }, taskParams);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<ContentClassesResponse> ECPresentationManager::GetContentClasses(AsyncContentClassesRequestParams const& params)
    {
    auto diagnostics = Diagnostics::Scope::Create("Get content classes");
    Diagnostics::SetCapturedAttributes({ DIAGNOSTICS_SCOPE_ATTRIBUTE_Rules });

    IConnectionCR connection = GetConnection(params.GetECDb());

    ECPresentationTaskParams taskParams;
    taskParams.SetDependencies({
        std::make_shared<TaskDependencyOnConnection>(connection.GetId()),
        std::make_shared<TaskDependencyOnRuleset>(params.GetRulesetId()),
        });
    taskParams.SetPriority(params.GetRequestPriority());
    return m_tasksManager->CreateAndExecute<ContentClassesResponse>([&, params, connectionId = connection.GetId()](auto& task) mutable
        {
        CALL_TASK_START_CALLBACK(params);

        IConnectionCR taskConnection = GetConnection(connectionId.c_str());
        task.SetTaskConnection(taskConnection);

        return m_impl->GetContentClasses(CreateImplRequestParams(params, taskConnection, *task.GetCancelationToken()));
        }, taskParams);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool ShouldCancelContentRequest(IECPresentationTaskCR request, Utf8StringCR displayType, Utf8StringCR connectionId, SelectionInfoCR selectionInfo)
    {
    return request.GetDependencies().Has(TaskDependencyOnDisplayType(displayType))
        && request.GetDependencies().Has(TaskDependencyOnConnection(connectionId))
        && request.GetDependencies().Has(TaskDependencyOnSelection::CreatePredicate([&selectionInfo](SelectionInfoCR dependencySelectionInfo)
            {
            return dependencySelectionInfo.GetSelectionProviderName().Equals(selectionInfo.GetSelectionProviderName())
                && dependencySelectionInfo.GetTimestamp() != selectionInfo.GetTimestamp();
            }));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<ContentDescriptorResponse> ECPresentationManager::GetContentDescriptor(AsyncContentDescriptorRequestParams const& params)
    {
    auto diagnostics = Diagnostics::Scope::Create("Get content descriptor");
    Diagnostics::SetCapturedAttributes({ DIAGNOSTICS_SCOPE_ATTRIBUTE_Rules });

    IConnectionCR connection = GetConnection(params.GetECDb());

    if (params.GetSelectionInfo())
        {
        m_tasksManager->Cancel([&](IECPresentationTaskCR task)
            {
            return ShouldCancelContentRequest(task, params.GetPreferredDisplayType(), connection.GetId(), *params.GetSelectionInfo());
            });
        }

    TaskDependencies dependencies
        {
        std::make_shared<TaskDependencyOnConnection>(connection.GetId()),
        std::make_shared<TaskDependencyOnRuleset>(params.GetRulesetId()),
        std::make_shared<TaskDependencyOnDisplayType>(params.GetPreferredDisplayType()),
        };
    if (params.GetSelectionInfo())
        dependencies.push_back(std::make_shared<TaskDependencyOnSelection>(*params.GetSelectionInfo()));

    ECPresentationTaskParams taskParams;
    taskParams.SetDependencies(dependencies);
    taskParams.SetPriority(params.GetRequestPriority());
    return m_tasksManager->CreateAndExecute<ContentDescriptorResponse>([&, params, connectionId = connection.GetId()](auto& task) mutable
        {
        CALL_TASK_START_CALLBACK(params);

        IConnectionCR taskConnection = GetConnection(connectionId.c_str());
        task.SetTaskConnection(taskConnection);

        return m_impl->GetContentDescriptor(CreateImplRequestParams(params, taskConnection, *task.GetCancelationToken()));
        }, taskParams);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<ContentResponse> ECPresentationManager::GetContent(WithPageOptions<AsyncContentRequestParams> const& params)
    {
    auto diagnostics = Diagnostics::Scope::Create("Get content");
    Diagnostics::SetCapturedAttributes({ DIAGNOSTICS_SCOPE_ATTRIBUTE_Rules });
    if (params.GetContentDescriptor().GetSelectionInfo())
        {
        m_tasksManager->Cancel([&](IECPresentationTaskCR task)
            {
            return ShouldCancelContentRequest(task, params.GetContentDescriptor().GetPreferredDisplayType(), params.GetContentDescriptor().GetConnectionId().c_str(), *params.GetContentDescriptor().GetSelectionInfo());
            });
        }

    TaskDependencies dependencies
        {
        std::make_shared<TaskDependencyOnConnection>(params.GetContentDescriptor().GetConnectionId()),
        std::make_shared<TaskDependencyOnRuleset>(params.GetContentDescriptor().GetRuleset().GetRuleSetId()),
        std::make_shared<TaskDependencyOnDisplayType>(params.GetContentDescriptor().GetPreferredDisplayType()),
        };
    if (params.GetContentDescriptor().GetSelectionInfo())
        dependencies.push_back(std::make_shared<TaskDependencyOnSelection>(*params.GetContentDescriptor().GetSelectionInfo()));

    ECPresentationTaskParams taskParams;
    taskParams.SetDependencies(dependencies);
    taskParams.SetPriority(params.GetRequestPriority());
    return m_tasksManager->CreateAndExecute<ContentResponse>([&, params](auto& task) mutable
        {
        CALL_TASK_START_CALLBACK(params);

        IConnectionCR taskConnection = GetConnection(params.GetContentDescriptor().GetConnectionId().c_str());
        task.SetTaskConnection(taskConnection);

        ContentCPtr content = m_impl->GetContent(CreateImplRequestParams(params, taskConnection, *task.GetCancelationToken()));
        if (content.IsValid())
            return ContentCPtr(Content::Create(content->GetDescriptor(), *PreloadedDataSource<ContentSetItemCPtr>::Create(content->GetContentSet().GetDataSource())));
        return ContentCPtr();
        }, taskParams);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<ContentSetSizeResponse> ECPresentationManager::GetContentSetSize(AsyncContentRequestParams const& params)
    {
    auto diagnostics = Diagnostics::Scope::Create("Get content set size");
    Diagnostics::SetCapturedAttributes({ DIAGNOSTICS_SCOPE_ATTRIBUTE_Rules });
    if (params.GetContentDescriptor().GetSelectionInfo())
        {
        m_tasksManager->Cancel([&](IECPresentationTaskCR task)
            {
            return ShouldCancelContentRequest(task, params.GetContentDescriptor().GetPreferredDisplayType(), params.GetContentDescriptor().GetConnectionId().c_str(), *params.GetContentDescriptor().GetSelectionInfo());
            });
        }

    TaskDependencies dependencies
        {
        std::make_shared<TaskDependencyOnConnection>(params.GetContentDescriptor().GetConnectionId()),
        std::make_shared<TaskDependencyOnRuleset>(params.GetContentDescriptor().GetRuleset().GetRuleSetId()),
        std::make_shared<TaskDependencyOnDisplayType>(params.GetContentDescriptor().GetPreferredDisplayType()),
        };
    if (params.GetContentDescriptor().GetSelectionInfo())
        dependencies.push_back(std::make_shared<TaskDependencyOnSelection>(*params.GetContentDescriptor().GetSelectionInfo()));

    ECPresentationTaskParams taskParams;
    taskParams.SetDependencies(dependencies);
    taskParams.SetPriority(params.GetRequestPriority());
    return m_tasksManager->CreateAndExecute<ContentSetSizeResponse>([&, params](auto& task) mutable
        {
        CALL_TASK_START_CALLBACK(params);

        IConnectionCR taskConnection = GetConnection(params.GetContentDescriptor().GetConnectionId().c_str());
        task.SetTaskConnection(taskConnection);

        return m_impl->GetContentSetSize(CreateImplRequestParams(params, taskConnection, *task.GetCancelationToken()));
        }, taskParams);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<DisplayLabelResponse> ECPresentationManager::GetDisplayLabel(AsyncECInstanceDisplayLabelRequestParams const& params)
    {
    ECClassCP ecClass = params.GetECDb().Schemas().GetClass(params.GetInstanceKey().GetClassId());
    KeySetPtr keys = KeySet::Create({ ECClassInstanceKey(ecClass, params.GetInstanceKey().GetInstanceId()) });
    return GetDisplayLabel(AsyncKeySetDisplayLabelRequestParams::Create(KeySetDisplayLabelRequestParams(params, *keys), params));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<DisplayLabelResponse> ECPresentationManager::GetDisplayLabel(AsyncKeySetDisplayLabelRequestParams const& params)
    {
    auto diagnostics = Diagnostics::Scope::Create("Get display label");
    Diagnostics::SetCapturedAttributes({ DIAGNOSTICS_SCOPE_ATTRIBUTE_Rules });

    IConnectionCR connection = GetConnection(params.GetECDb());

    TaskDependencies dependencies
        {
        std::make_shared<TaskDependencyOnConnection>(connection.GetId()),
        std::make_shared<TaskDependencyOnDisplayType>(ContentDisplayType::List),
        };
    if (!params.GetRulesetId().empty())
        dependencies.push_back(std::make_shared<TaskDependencyOnRuleset>(DISPLAY_LABEL_RULESET_ID));

    ECPresentationTaskParams taskParams;
    taskParams.SetDependencies(dependencies);
    taskParams.SetPriority(params.GetRequestPriority());
    return m_tasksManager->CreateAndExecute<DisplayLabelResponse>([&, params, connectionId = connection.GetId()](auto& task) mutable
        {
        CALL_TASK_START_CALLBACK(params);

        IConnectionCR taskConnection = GetConnection(connectionId.c_str());
        task.SetTaskConnection(taskConnection);

        return m_impl->GetDisplayLabel(CreateImplRequestParams(params, taskConnection, *task.GetCancelationToken()));
        }, taskParams);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<DistinctValuesResponse> ECPresentationManager::GetDistinctValues(WithPageOptions<AsyncDistinctValuesRequestParams> const& params)
    {
    auto diagnostics = Diagnostics::Scope::Create("Get distinct values");
    Diagnostics::SetCapturedAttributes({ DIAGNOSTICS_SCOPE_ATTRIBUTE_Rules });

    TaskDependencies dependencies
        {
        std::make_shared<TaskDependencyOnConnection>(params.GetContentDescriptor().GetConnectionId()),
        std::make_shared<TaskDependencyOnRuleset>(params.GetContentDescriptor().GetRuleset().GetRuleSetId()),
        std::make_shared<TaskDependencyOnDisplayType>(params.GetContentDescriptor().GetPreferredDisplayType()),
        };
    if (params.GetContentDescriptor().GetSelectionInfo())
        dependencies.push_back(std::make_shared<TaskDependencyOnSelection>(*params.GetContentDescriptor().GetSelectionInfo()));

    ECPresentationTaskParams taskParams;
    taskParams.SetDependencies(dependencies);
    taskParams.SetPriority(params.GetRequestPriority());
    return m_tasksManager->CreateAndExecute<DistinctValuesResponse>([&, params](auto& task) mutable
        {
        CALL_TASK_START_CALLBACK(params);

        IConnectionCR taskConnection = GetConnection(params.GetContentDescriptor().GetConnectionId().c_str());
        task.SetTaskConnection(taskConnection);

        PagingDataSourceCPtr<DisplayValueGroupCPtr> source = m_impl->GetDistinctValues(CreateImplRequestParams(params, taskConnection, *task.GetCancelationToken()));
        if (source.IsValid())
            return PagedDataContainer<DisplayValueGroupCPtr>(*source);
        return PagedDataContainer<DisplayValueGroupCPtr>();
        }, taskParams);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
IRulesetLocaterManager& ECPresentationManager::GetLocaters() {return m_impl->GetLocaters();}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
IUserSettings& ECPresentationManager::GetUserSettings(Utf8CP rulesetId) const {return m_impl->GetUserSettings(rulesetId);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<HierarchiesCompareResponse> ECPresentationManager::CompareHierarchies(AsyncHierarchyCompareRequestParams const& params)
    {
    auto diagnostics = Diagnostics::Scope::Create("Compare hierarchies");
    Diagnostics::SetCapturedAttributes({ DIAGNOSTICS_SCOPE_ATTRIBUTE_Rules });

    IConnectionCR connection = GetConnection(params.GetECDb());

    ECPresentationTaskParams taskParams;
    taskParams.SetDependencies({
        std::make_shared<TaskDependencyOnConnection>(connection.GetId()),
        std::make_shared<TaskDependencyOnRuleset>(params.GetLhsRulesetId()),
        std::make_shared<TaskDependencyOnRuleset>(params.GetRhsRulesetId()),
        });
    taskParams.SetPriority(params.GetRequestPriority());
    return m_tasksManager->CreateAndExecute<HierarchiesCompareResponse>([&, params, connectionId = connection.GetId()](auto& task) mutable
        {
        CALL_TASK_START_CALLBACK(params);

        IConnectionCR taskConnection = GetConnection(connectionId.c_str());
        task.SetTaskConnection(taskConnection);

        return m_impl->CompareHierarchies(CreateImplRequestParams(params, taskConnection, *task.GetCancelationToken()));
        }, taskParams);
    }
