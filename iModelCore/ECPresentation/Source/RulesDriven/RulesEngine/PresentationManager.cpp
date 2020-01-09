/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include <ECPresentation/RulesDriven/PresentationManager.h>
#include "PresentationManagerImpl.h"
#include "NavNodesDataSource.h"
#include "LoggingHelper.h"
#include "NodePathsHelper.h"
#include "TaskScheduler.h"
#include <folly/futures/InlineExecutor.h>
#include <folly/BeFolly.h>

const Utf8CP RulesDrivenECPresentationManager::CommonOptions::OPTION_NAME_RulesetId = "RulesetId";
const Utf8CP RulesDrivenECPresentationManager::CommonOptions::OPTION_NAME_Locale = "Locale";
const Utf8CP RulesDrivenECPresentationManager::CommonOptions::OPTION_NAME_Priority = "Priority";

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                11/2017
+===============+===============+===============+===============+===============+======*/
struct RulesDrivenECPresentationManager::ECInstanceChangeEventSourceWrapper : ECInstanceChangeEventSource, ECInstanceChangeEventSource::IEventHandler
{
private:
    RulesDrivenECPresentationManager& m_manager;
    ECInstanceChangeEventSourcePtr m_wrapped;
private:
    ECInstanceChangeEventSourceWrapper(RulesDrivenECPresentationManager& manager, ECInstanceChangeEventSource& wrapped)
        : m_manager(manager), m_wrapped(&wrapped)
        {
        m_wrapped->RegisterEventHandler(*this);
        }
    ~ECInstanceChangeEventSourceWrapper()
        {
        m_wrapped->UnregisterEventHandler(*this);
        }
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
        RefCountedPtr<ECPresentationTask> task = new ECPresentationTask(m_manager.GetTasksManager().GetMutex(),
            [&, connectionId = connection->GetId(), changes](IECPresentationTaskCR task)
                {
                NotifyECInstancesChanged(connections.GetConnection(connectionId.c_str())->GetECDb(), changes);
                }
            );
        task->SetDependencies(TaskDependencies(connection->GetId()));
        task->SetBlockedTasksPredicate([connectionId = connection->GetId()](IECPresentationTaskCR task)
            {
            return task.GetDependencies().GetConnectionId().Equals(connectionId);
            });
        m_manager.GetTasksManager().Execute(*task);
        }
public:
    static RefCountedPtr<ECInstanceChangeEventSourceWrapper> Create(RulesDrivenECPresentationManager& manager, ECInstanceChangeEventSource& wrapped)
        {
        return new ECInstanceChangeEventSourceWrapper(manager, wrapped);
        }
    ECInstanceChangeEventSource& GetWrappedEventSource() const {return *m_wrapped;}
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                11/2017
+===============+===============+===============+===============+===============+======*/
struct RulesDrivenECPresentationManager::ConnectionManagerWrapper : IConnectionManager, IConnectionsListener
{
private:
    ECPresentationTasksManager& m_tasksManager;
    IConnectionManagerR m_wrapped;
    mutable bmap<int, bset<IConnectionsListener*>> m_listeners;
    mutable BeMutex m_mutex;

protected:
    IConnection* _GetConnection(Utf8CP connectionId) const override {return m_wrapped.GetConnection(connectionId);}
    IConnection* _GetConnection(ECDbCR ecdb) const override {return m_wrapped.GetConnection(ecdb);}
    IConnectionPtr _CreateConnection(ECDbR ecdb) override {return m_wrapped.CreateConnection(ecdb);}
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
        RefCountedPtr<ECPresentationTasksBlocker> block;
        if (ConnectionEventType::Closed == evt.GetEventType())
            {
            BeMutexHolder lock(m_tasksManager.GetMutex());
            TasksCancelationResult cancelation = m_tasksManager.Cancel([&](IECPresentationTaskCR task)
                {
                return task.GetDependencies().GetConnectionId().Equals(evt.GetConnection().GetId());
                });
            block = m_tasksManager.Block([&](IECPresentationTaskCR task)
                {
                return task.GetDependencies().GetConnectionId().Equals(evt.GetConnection().GetId())
                    && cancelation.GetTasks().end() == cancelation.GetTasks().find(&task);
                });
            lock.unlock();
            cancelation.GetCompletion().wait();
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
    ConnectionManagerWrapper(RulesDrivenECPresentationManager& manager, IConnectionManagerP wrapped)
        : m_tasksManager(manager.GetTasksManager()),
        m_wrapped(nullptr != wrapped ? *wrapped : *new ConnectionManager())
        {
        m_wrapped.AddListener(*this);
        }
    ~ConnectionManagerWrapper()
        {
        m_wrapped.DropListener(*this);
        delete &m_wrapped;
        }
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                11/2017
+===============+===============+===============+===============+===============+======*/
struct RulesDrivenECPresentationManager::RulesetLocaterManagerWrapper : IRulesetLocaterManager, IRulesetCallbacksHandler
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
            return task.GetDependencies().GetRulesetId().Equals(ruleset.GetRuleSetId());
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
    RulesetLocaterManagerWrapper(RulesDrivenECPresentationManager& manager, IConnectionManagerCR connections)
        : m_tasksManager(manager.GetTasksManager()), m_wrapped(connections)
        {
        m_wrapped.SetRulesetCallbacksHandler(this);
        }
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                11/2017
+===============+===============+===============+===============+===============+======*/
struct RulesDrivenECPresentationManager::UserSettingsManagerWrapper : IUserSettingsManager, IUserSettingsChangeListener
{
private:
    ECPresentationTasksManager& m_tasksManager;
    UserSettingsManager m_wrapped;
protected:
    IUserSettings& _GetSettings(Utf8StringCR rulesetId) const override {return m_wrapped.GetSettings(rulesetId);}
    void _OnLocalizationProviderChanged() override {m_wrapped.SetLocalizationProvider(GetLocalizationProvider());}
    void _OnLocalStateChanged() override {m_wrapped.SetLocalState(GetLocalState());}
    void _OnSettingChanged(Utf8CP rulesetId, Utf8CP settingId) const override
        {
        if (nullptr == GetChangesListener())
            return;

        RefCountedPtr<ECPresentationTask> task = new ECPresentationTask(m_tasksManager.GetMutex(),
            [&, rulesetId = Utf8String(rulesetId), settingId = Utf8String(settingId)](IECPresentationTaskCR task)
                {
                BeMutexHolder lock(GetMutex());
                GetChangesListener()->_OnSettingChanged(rulesetId.c_str(), settingId.c_str());
                }
            );
        task->SetBlockedTasksPredicate([rulesetId = Utf8String(rulesetId)](IECPresentationTaskCR)
            {
            // we don't know which connections this task can affect - need to block everything..
            return true;
            });
        m_tasksManager.Execute(*task);
        }
public:
    UserSettingsManagerWrapper(RulesDrivenECPresentationManager& manager, BeFileNameCR temporaryDirectory)
        : m_wrapped(temporaryDirectory), m_tasksManager(manager.GetTasksManager())
        {
        m_wrapped.SetChangesListener(this);
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
static RulesDrivenECPresentationManager::Params CreateParams(IConnectionManagerR connections, RulesDrivenECPresentationManager::Paths const& paths, bool disableDiskCache)
    {
    RulesDrivenECPresentationManager::Params::CachingParams cachingParams;
    cachingParams.SetDisableDiskCache(disableDiskCache);
    RulesDrivenECPresentationManager::Params params(paths);
    params.SetConnections(&connections);
    params.SetCachingParams(cachingParams);
    return params;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
RulesDrivenECPresentationManager::RulesDrivenECPresentationManager(IConnectionManagerR connections, Paths const& paths, bool disableDiskCache)
    : RulesDrivenECPresentationManager(CreateParams(connections, paths, disableDiskCache))
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
RulesDrivenECPresentationManager::RulesDrivenECPresentationManager(Params const& params)
    {
    m_tasksManager = new ECPresentationTasksManager(params.GetMultiThreadingParams().GetBackgroundThreadAllocations());
    RulesDrivenECPresentationManagerImpl* impl = new RulesDrivenECPresentationManagerImpl(CreateImplParams(params));
    m_impl = impl;
    impl->Initialize();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
RulesDrivenECPresentationManager::~RulesDrivenECPresentationManager()
    {
    DELETE_AND_CLEAR(m_tasksManager);
    DELETE_AND_CLEAR(m_impl);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenECPresentationManager::SetImpl(Impl& impl)
    {
    DELETE_AND_CLEAR(m_impl);
    m_impl = &impl;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2019
+---------------+---------------+---------------+---------------+---------------+------*/
RulesDrivenECPresentationManager::Params RulesDrivenECPresentationManager::CreateImplParams(Params const& source)
    {
    RulesDrivenECPresentationManager::Params result(source);
    result.SetConnections(new RulesDrivenECPresentationManager::ConnectionManagerWrapper(*this, source.GetConnections()));
    result.SetUserSettings(new RulesDrivenECPresentationManager::UserSettingsManagerWrapper(*this, source.GetPaths().GetTemporaryDirectory()));
    result.SetRulesetLocaters(new RulesDrivenECPresentationManager::RulesetLocaterManagerWrapper(*this, *result.GetConnections()));

    bvector<ECInstanceChangeEventSourcePtr> ecInstanceChangeEventSources;
    for (ECInstanceChangeEventSourcePtr const& evtSource : source.GetECInstanceChangeEventSources())
        {
        auto wrapper = RulesDrivenECPresentationManager::ECInstanceChangeEventSourceWrapper::Create(*this, *evtSource);
        ecInstanceChangeEventSources.push_back(wrapper);
        }
    result.SetECInstanceChangeEventSources(ecInstanceChangeEventSources);

    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2019
+---------------+---------------+---------------+---------------+---------------+------*/
unsigned RulesDrivenECPresentationManager::GetBackgroundThreadsCount() const {return m_tasksManager->GetThreadsCount();}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
IUserSettingsManager& RulesDrivenECPresentationManager::GetUserSettings() const {return m_impl->GetUserSettingsManager();}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2019
+---------------+---------------+---------------+---------------+---------------+------*/
IConnectionManagerCR RulesDrivenECPresentationManager::GetConnectionsCR() const {return m_impl->GetConnections();}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2019
+---------------+---------------+---------------+---------------+---------------+------*/
IConnectionManagerR RulesDrivenECPresentationManager::_GetConnections() {return m_impl->GetConnections();}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2019
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP RulesDrivenECPresentationManager::GetConnectionId(ECDbCR db) const
    {
    IConnectionCP connection = GetConnectionsCR().GetConnection(db);
    if (!connection)
        return nullptr;
    return connection->GetId().c_str();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2019
+---------------+---------------+---------------+---------------+---------------+------*/
IConnectionCPtr RulesDrivenECPresentationManager::GetTaskConnection(IECPresentationTaskCR task) const
    {
    return GetConnectionsCR().GetConnection(task.GetDependencies().GetConnectionId().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2019
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<folly::Unit> RulesDrivenECPresentationManager::GetTasksCompletion() const
    {
    return m_tasksManager->GetAllTasksCompletion();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<NavNodesContainer> RulesDrivenECPresentationManager::_GetRootNodes(ECDbCR db, PageOptionsCR pageOpts, JsonValueCR jsonOptions, PresentationRequestContextCR context)
    {
    Utf8CP connectionId = GetConnectionId(db);
    if (!connectionId)
        {
        BeAssert(false && "ECDb not registered as a connection");
        return NavNodesContainer();
        }
    NavigationOptions options(jsonOptions);
    return m_tasksManager->CreateAndExecute<NavNodesContainer>([&, pageOpts, options, context](IECPresentationTaskWithResult<NavNodesContainer> const& task)
        {
        context.OnTaskStart();
        INavNodesDataSourcePtr source = m_impl->GetRootNodes(*GetTaskConnection(task), pageOpts, options, *task.GetCancelationToken());
        if (source.IsValid())
            {
            source = PreloadedDataSource::Create(*source);
            return DataContainer<NavNodeCPtr>(*source);
            }
        return DataContainer<NavNodeCPtr>();
        }, TaskDependencies(connectionId, options.GetRulesetId()), true, options.GetPriority());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<size_t> RulesDrivenECPresentationManager::_GetRootNodesCount(ECDbCR db, JsonValueCR jsonOptions, PresentationRequestContextCR context)
    {
    Utf8CP connectionId = GetConnectionId(db);
    if (!connectionId)
        {
        BeAssert(false && "ECDb not registered as a connection");
        return 0;
        }
    NavigationOptions options(jsonOptions);
    return m_tasksManager->CreateAndExecute<size_t>([&, options, context](IECPresentationTaskWithResult<size_t> const& task)
        {
        context.OnTaskStart();
        return m_impl->GetRootNodesCount(*GetTaskConnection(task), options, *task.GetCancelationToken());
        }, TaskDependencies(connectionId, options.GetRulesetId()), true, options.GetPriority());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<NavNodesContainer> RulesDrivenECPresentationManager::_GetChildren(ECDbCR db, NavNodeCR parent, PageOptionsCR pageOpts, JsonValueCR jsonOptions, PresentationRequestContextCR context)
    {
    Utf8CP connectionId = GetConnectionId(db);
    if (!connectionId)
        {
        BeAssert(false && "ECDb not registered as a connection");
        return NavNodesContainer();
        }
    NavigationOptions options(jsonOptions);
    return m_tasksManager->CreateAndExecute<NavNodesContainer>([&, parent = NavNodeCPtr(&parent), pageOpts, options, context](IECPresentationTaskWithResult<NavNodesContainer> const& task)
        {
        context.OnTaskStart();
        INavNodesDataSourcePtr source = m_impl->GetChildren(*GetTaskConnection(task), *parent, pageOpts, options, *task.GetCancelationToken());
        if (source.IsValid())
            {
            source = PreloadedDataSource::Create(*source);
            return DataContainer<NavNodeCPtr>(*source);
            }
        return DataContainer<NavNodeCPtr>();
        }, TaskDependencies(connectionId, options.GetRulesetId()), true, options.GetPriority());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<size_t> RulesDrivenECPresentationManager::_GetChildrenCount(ECDbCR db, NavNodeCR parent, JsonValueCR jsonOptions, PresentationRequestContextCR context)
    {
    Utf8CP connectionId = GetConnectionId(db);
    if (!connectionId)
        {
        BeAssert(false && "ECDb not registered as a connection");
        return 0;
        }
    NavigationOptions options(jsonOptions);
    return m_tasksManager->CreateAndExecute<size_t>([&, parent = NavNodeCPtr(&parent), options, context](IECPresentationTaskWithResult<size_t> const& task)
        {
        context.OnTaskStart();
        return m_impl->GetChildrenCount(*GetTaskConnection(task), *parent, options, *task.GetCancelationToken());
        }, TaskDependencies(connectionId, options.GetRulesetId()), true, options.GetPriority());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<NavNodeCPtr> RulesDrivenECPresentationManager::_GetParent(ECDbCR db, NavNodeCR node, JsonValueCR jsonOptions, PresentationRequestContextCR context)
    {
    Utf8CP connectionId = GetConnectionId(db);
    if (!connectionId)
        {
        BeAssert(false && "ECDb not registered as a connection");
        return folly::makeFuture(nullptr);
        }
    NavigationOptions options(jsonOptions);
    return m_tasksManager->CreateAndExecute<NavNodeCPtr>([&, node = NavNodeCPtr(&node), options, context](IECPresentationTaskWithResult<NavNodeCPtr> const& task)
        {
        context.OnTaskStart();
        return m_impl->GetParent(*GetTaskConnection(task), *node, options, *task.GetCancelationToken());
        }, TaskDependencies(connectionId, options.GetRulesetId()), true, options.GetPriority());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<NavNodeCPtr> RulesDrivenECPresentationManager::_GetNode(ECDbCR db, NavNodeKeyCR nodeKey, JsonValueCR jsonOptions, PresentationRequestContextCR context)
    {
    Utf8CP connectionId = GetConnectionId(db);
    if (!connectionId)
        {
        BeAssert(false && "ECDb not registered as a connection");
        return folly::makeFuture(nullptr);
        }
    NavigationOptions options(jsonOptions);
    return m_tasksManager->CreateAndExecute<NavNodeCPtr>([&, nodeKey = NavNodeKeyCPtr(&nodeKey), options, context](IECPresentationTaskWithResult<NavNodeCPtr> const& task)
        {
        context.OnTaskStart();
        return m_impl->GetNode(*GetTaskConnection(task), *nodeKey, options, *task.GetCancelationToken());
        }, TaskDependencies(connectionId, options.GetRulesetId()), true, options.GetPriority());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<bvector<NavNodeCPtr>> RulesDrivenECPresentationManager::_GetFilteredNodes(ECDbCR db, Utf8CP filterText, JsonValueCR jsonOptions, PresentationRequestContextCR context)
    {
    Utf8CP connectionId = GetConnectionId(db);
    if (!connectionId)
        {
        BeAssert(false && "ECDb not registered as a connection");
        return bvector<NavNodeCPtr>();
        }
    NavigationOptions options(jsonOptions);
    return m_tasksManager->CreateAndExecute<bvector<NavNodeCPtr>>([&, filterText = Utf8String(filterText), options, context](IECPresentationTaskWithResult<bvector<NavNodeCPtr>> const& task)
        {
        context.OnTaskStart();
        return m_impl->GetFilteredNodes(*GetTaskConnection(task), filterText.c_str(), options, *task.GetCancelationToken());
        }, TaskDependencies(connectionId, options.GetRulesetId()), true, options.GetPriority());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<bvector<NodesPathElement>> RulesDrivenECPresentationManager::_GetFilteredNodePaths(ECDbCR db, Utf8CP filterText, JsonValueCR jsonOptions, PresentationRequestContextCR context)
    {
    Utf8String escapedString(filterText);
    escapedString.ReplaceAll("\\", "\\\\");
    escapedString.ReplaceAll("%", "\\%");
    escapedString.ReplaceAll("_", "\\_");

    return GetFilteredNodes(db, escapedString.c_str(), jsonOptions, context)
        .then([&, context, filterText = Utf8String(filterText).ToLower(), jsonOptions](bvector<NavNodeCPtr> filteredNodes)
        {
        context.OnTaskStart();
        return NodePathsHelper::CreateHierarchy(*this, db, jsonOptions, filteredNodes, filterText.c_str());
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<bvector<NodesPathElement>> RulesDrivenECPresentationManager::_GetNodePaths(ECDbCR db, bvector<bvector<ECInstanceKey>> const& keyPaths, int64_t markedIndex, JsonValueCR jsonOptions, PresentationRequestContextCR context)
    {
    Utf8CP connectionId = GetConnectionId(db);
    if (!connectionId)
        {
        BeAssert(false && "ECDb not registered as a connection");
        return bvector<NodesPathElement>();
        }
    bvector<folly::Future<NodesPathElement>> pathFutures;
    for (size_t i = 0; i < keyPaths.size(); ++i)
        {
        bvector<ECInstanceKey> const& keyPath = keyPaths[i];
        pathFutures.push_back(GetNodePath(db, keyPath, jsonOptions, context));
        }
    return folly::collect(pathFutures).then([markedIndex, context](std::vector<NodesPathElement> paths) -> bvector<NodesPathElement>
        {
        context.OnTaskStart();
        return NodePathsHelper::MergePaths(paths, &markedIndex);
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static folly::Future<NodesPathElement> FindNode(IECPresentationManagerR manager, ECDbCR db, NavNodeCP parentNode, ECInstanceKeyCR lookupKey, JsonValueCR jsonOptions, PresentationRequestContextCR context)
    {
    folly::Future<DataContainer<NavNodeCPtr>> nodesFuture = (nullptr == parentNode)
        ? manager.GetRootNodes(db, PageOptions(), jsonOptions, context)
        : manager.GetChildren(db, *parentNode, PageOptions(), jsonOptions, context);
    return nodesFuture.then([&, lookupKey, jsonOptions, context](DataContainer<NavNodeCPtr> nodes) -> NodesPathElement
        {
        context.OnTaskStart();
        for (size_t i = 0; i < nodes.GetSize(); ++i)
            {
            NavNodeCPtr node = nodes[i];
            if (node->GetKey()->AsECInstanceNodeKey() && node->GetKey()->AsECInstanceNodeKey()->HasInstanceKey(lookupKey))
                return NodesPathElement(*node, i);
            if (nullptr != node->GetKey()->AsGroupingNodeKey())
                {
                NavNodeExtendedData extendedData(*node);
                if (extendedData.HasGroupingType())
                    {
                    bvector<ECInstanceKey> groupedKeys = extendedData.GetInstanceKeys();
                    auto iter = std::find(groupedKeys.begin(), groupedKeys.end(), lookupKey);
                    if (groupedKeys.end() != iter)
                        return NodesPathElement(*node, i);
                    }
                }
            }
        return NodesPathElement();
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2019
+---------------+---------------+---------------+---------------+---------------+------*/
static folly::Future<NodesPathElement> CreateNodePath(IECPresentationManagerR manager, ECDbCR db, NavNodeCP parentNode, bvector<ECInstanceKey> keyPath, JsonValueCR jsonOptions, PresentationRequestContextCR context)
    {
    if (keyPath.empty())
        return NodesPathElement();

    return FindNode(manager, db, parentNode, keyPath.front(), jsonOptions, context).then([&, keyPath, jsonOptions, context](NodesPathElement parentEl) mutable
        {
        context.OnTaskStart();

        if (!parentEl.GetNode().IsValid())
            {
            BeAssert(false && "Provided nodes path doesn't exist in the hierarchy");
            return folly::makeFuture(NodesPathElement());
            }

        if (parentEl.GetNode()->GetKey()->AsECInstanceNodeKey() && parentEl.GetNode()->GetKey()->AsECInstanceNodeKey()->HasInstanceKey(keyPath.front()))
            keyPath.erase(keyPath.begin());

        return CreateNodePath(manager, db, parentEl.GetNode().get(), keyPath, jsonOptions, context).then([parentEl](NodesPathElement childEl) mutable
            {
            if (childEl.GetNode().IsValid())
                parentEl.GetChildren().push_back(childEl);
            return parentEl;
            });
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2019
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<NodesPathElement> RulesDrivenECPresentationManager::_GetNodePath(ECDbCR db, bvector<ECInstanceKey> const& keyPath, JsonValueCR jsonOptions, PresentationRequestContextCR context)
    {
    return CreateNodePath(*this, db, nullptr, keyPath, jsonOptions, context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<bvector<SelectClassInfo>> RulesDrivenECPresentationManager::_GetContentClasses(ECDbCR db, Utf8CP preferredDisplayType, int contentFlags,
    bvector<ECClassCP> const& classes, JsonValueCR jsonOptions, PresentationRequestContextCR context)
    {
    Utf8CP connectionId = GetConnectionId(db);
    if (!connectionId)
        {
        BeAssert(false && "ECDb not registered as a connection");
        return bvector<SelectClassInfo>();
        }
    ContentOptions options(jsonOptions);
    return m_tasksManager->CreateAndExecute<bvector<SelectClassInfo>>([&, displayType = Utf8String(preferredDisplayType), contentFlags, classes, options, context](IECPresentationTaskWithResult<bvector<SelectClassInfo>> const& task)
        {
        context.OnTaskStart();
        return m_impl->GetContentClasses(*GetTaskConnection(task), displayType.c_str(), contentFlags, classes, options, *task.GetCancelationToken());
        }, TaskDependencies(connectionId, options.GetRulesetId()), true, options.GetPriority());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2019
+---------------+---------------+---------------+---------------+---------------+------*/
static bool ShouldCancelContentRequest(IECPresentationTaskCR request, Utf8CP displayType, Utf8CP connectionId, SelectionInfoCR selectionInfo)
    {
    return request.GetDependencies().GetDisplayType().Equals(displayType ? displayType : "")
        && request.GetDependencies().GetConnectionId().Equals(connectionId ? connectionId : "")
        && request.GetDependencies().GetSelectionInfo()
        && selectionInfo.GetSelectionProviderName().Equals(request.GetDependencies().GetSelectionInfo()->GetSelectionProviderName())
        && selectionInfo.GetTimestamp() != request.GetDependencies().GetSelectionInfo()->GetTimestamp();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<ContentDescriptorCPtr> RulesDrivenECPresentationManager::_GetContentDescriptor(ECDbCR db, Utf8CP preferredDisplayType, int contentFlags,
    KeySetCR inputKeys, SelectionInfo const* selectionInfo, JsonValueCR jsonOptions, PresentationRequestContextCR context)
    {
    Utf8CP connectionId = GetConnectionId(db);
    if (!connectionId)
        {
        BeAssert(false && "ECDb not registered as a connection");
        return folly::makeFuture(nullptr);
        }

    if (selectionInfo)
        {
        m_tasksManager->Cancel([&](IECPresentationTaskCR task)
            {
            return ShouldCancelContentRequest(task, preferredDisplayType, connectionId, *selectionInfo);
            });
        }

    ContentOptions options(jsonOptions);
    return m_tasksManager->CreateAndExecute<ContentDescriptorCPtr>([&, displayType = Utf8String(preferredDisplayType), contentFlags, input = KeySetCPtr(&inputKeys), selectionInfo = SelectionInfoCPtr(selectionInfo), options, context](IECPresentationTaskWithResult<ContentDescriptorCPtr> const& task)
        {
        context.OnTaskStart();
        return m_impl->GetContentDescriptor(*GetTaskConnection(task), displayType.c_str(), contentFlags, *input, selectionInfo.get(), options, *task.GetCancelationToken());
        }, TaskDependencies(connectionId, options.GetRulesetId(), preferredDisplayType, selectionInfo), true, options.GetPriority());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<ContentCPtr> RulesDrivenECPresentationManager::_GetContent(ContentDescriptorCR descriptor, PageOptionsCR pageOpts, PresentationRequestContextCR context)
    {
    if (descriptor.GetSelectionInfo())
        {
        m_tasksManager->Cancel([&](IECPresentationTaskCR task)
            {
            return ShouldCancelContentRequest(task, descriptor.GetPreferredDisplayType().c_str(), descriptor.GetConnectionId().c_str(), *descriptor.GetSelectionInfo());
            });
        }

    ContentOptions options(descriptor.GetOptions());
    return m_tasksManager->CreateAndExecute<ContentCPtr>([&, descriptor = ContentDescriptorCPtr(&descriptor), pageOpts, context](IECPresentationTaskWithResult<ContentCPtr> const& task)
        {
        context.OnTaskStart();
        return m_impl->GetContent(*GetTaskConnection(task), *descriptor, pageOpts, *task.GetCancelationToken());
        }, TaskDependencies(descriptor.GetConnectionId(), options.GetRulesetId(), descriptor.GetPreferredDisplayType(), descriptor.GetSelectionInfo()), true, options.GetPriority());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<size_t> RulesDrivenECPresentationManager::_GetContentSetSize(ContentDescriptorCR descriptor, PresentationRequestContextCR context)
    {
    if (descriptor.GetSelectionInfo())
        {
        m_tasksManager->Cancel([&](IECPresentationTaskCR task)
            {
            return ShouldCancelContentRequest(task, descriptor.GetPreferredDisplayType().c_str(), descriptor.GetConnectionId().c_str(), *descriptor.GetSelectionInfo());
            });
        }

    ContentOptions options(descriptor.GetOptions());
    return m_tasksManager->CreateAndExecute<size_t>([&, descriptor = ContentDescriptorCPtr(&descriptor), context](IECPresentationTaskWithResult<size_t> const& task)
        {
        context.OnTaskStart();
        return m_impl->GetContentSetSize(*GetTaskConnection(task), *descriptor, *task.GetCancelationToken());
        }, TaskDependencies(descriptor.GetConnectionId(), options.GetRulesetId(), descriptor.GetPreferredDisplayType(), descriptor.GetSelectionInfo()), true, options.GetPriority());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<LabelDefinitionCPtr> RulesDrivenECPresentationManager::_GetDisplayLabel(ECDbCR db, KeySetCR keys, JsonValueCR jsonOptions, PresentationRequestContextCR context)
    {
    Utf8CP connectionId = GetConnectionId(db);
    if (!connectionId)
        {
        BeAssert(false && "ECDb not registered as a connection");
        return LabelDefinition::Create();
        }
    CommonOptions options(jsonOptions);
    return m_tasksManager->CreateAndExecute<LabelDefinitionCPtr>([&, keys = KeySetCPtr(&keys), context](IECPresentationTaskWithResult<LabelDefinitionCPtr> const& task)
        {
        context.OnTaskStart();
        return m_impl->GetDisplayLabel(*GetTaskConnection(task), *keys, *task.GetCancelationToken());
        }, TaskDependencies(connectionId, DISPLAY_LABEL_RULESET_ID, ContentDisplayType::List, nullptr), true, options.GetPriority());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
IRulesetLocaterManager& RulesDrivenECPresentationManager::GetLocaters() {return m_impl->GetLocaters();}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
IUserSettings& RulesDrivenECPresentationManager::GetUserSettings(Utf8CP rulesetId) const {return m_impl->GetUserSettings(rulesetId);}
