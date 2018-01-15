/*--------------------------------------------------------------------------------------+
|
|     $Source: Source/RulesDriven/RulesEngine/PresentationManager.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include <ECPresentation/RulesDriven/PresentationManager.h>
#include "PresentationManagerImpl.h"
#include "NavNodesDataSource.h"

#ifdef RULES_ENGINE_FORCE_SINGLE_THREAD
#include <folly/futures/InlineExecutor.h>
#else
#include "SingleThreadQueueExecutor.h"
#endif

const Utf8CP RulesDrivenECPresentationManager::ContentOptions::OPTION_NAME_RulesetId = "RulesetId";
const Utf8CP RulesDrivenECPresentationManager::NavigationOptions::OPTION_NAME_RulesetId = "RulesetId";
const Utf8CP RulesDrivenECPresentationManager::NavigationOptions::OPTION_NAME_RuleTargetTree = "RuleTargetTree";
const Utf8CP RulesDrivenECPresentationManager::NavigationOptions::OPTION_NAME_DisableUpdates = "DisableUpdates";

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                11/2017
+===============+===============+===============+===============+===============+======*/
struct CancelationToken : ICancelationToken
{
private:
    BeMutex& m_mutex;
    bool m_isCanceled;
protected:
    bool _IsCanceled() const {BeMutexHolder lock(m_mutex); return m_isCanceled;}
public:
    CancelationToken(BeMutex& mutex) : m_mutex(mutex), m_isCanceled(false) {}
    void SetCanceled(bool value) {BeMutexHolder lock(m_mutex); m_isCanceled = value;}
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                11/2017
+===============+===============+===============+===============+===============+======*/
struct TaskDependencies
{
private:
    Utf8String m_connectionId;
    Utf8String m_rulesetId;
    bool m_selection;
public:
    TaskDependencies(Utf8String connectionId = "", Utf8String rulesetId = "", bool selection = false)
        : m_connectionId(connectionId), m_rulesetId(rulesetId), m_selection(selection)
        {}
    Utf8StringCR GetConnectionIdDependency() const {return m_connectionId;}
    Utf8StringCR GetRulesetIdDependency() const {return m_rulesetId;}
    bool DependsOnSelection() const {return m_selection;}
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                11/2017
+===============+===============+===============+===============+===============+======*/
struct ICancelableTask
{
private:
    TaskDependencies m_dependencies;
public:
    ICancelableTask(TaskDependencies dependencies = TaskDependencies()) : m_dependencies(dependencies) {}
    virtual ~ICancelableTask() {}
    virtual void _Cancel() = 0;
    TaskDependencies const& GetDependencies() const {return m_dependencies;}
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                11/2017
+===============+===============+===============+===============+===============+======*/
struct RulesDrivenECPresentationManager::CancelableTasksStore
{
private:
    mutable BeMutex m_mutex;
    bset<ICancelableTask*> m_tasks;
private:
    void Cancel(std::function<bool(ICancelableTask const&)> predicate)
        {
        BeMutexHolder lock(m_mutex);
        bvector<ICancelableTask*> tasks;
        for (ICancelableTask* task : m_tasks)
            {
            if (!predicate || predicate(*task))
                tasks.push_back(task);
            }
        for (ICancelableTask* task : tasks)
            {
            // note: the task will get removed from m_tasks when the promise actually gets canceled
            task->_Cancel();
            }
        }
public:
    BeMutex& GetMutex() {return m_mutex;}
    bool Contains(ICancelableTask& task) const
        {
        BeMutexHolder lock(m_mutex);
        return m_tasks.end() != m_tasks.find(&task);
        }
    void Add(ICancelableTask& task)
        {
        BeMutexHolder lock(m_mutex);
        m_tasks.insert(&task);
        }
    void Remove(ICancelableTask& task)
        {
        BeMutexHolder lock(m_mutex);
        m_tasks.erase(&task);
        }
    void CancelAll() {Cancel(nullptr);}
    void CancelSelectionDependants(Utf8StringCR connectionId) {Cancel([&](ICancelableTask const& task){return task.GetDependencies().DependsOnSelection() && task.GetDependencies().GetConnectionIdDependency().Equals(connectionId);});}
    void CancelByConnectionId(Utf8StringCR connectionId) {Cancel([&](ICancelableTask const& task){return task.GetDependencies().GetConnectionIdDependency().Equals(connectionId);});}
    void CancelByRulesetId(Utf8StringCR rulesetId) {Cancel([&](ICancelableTask const& task){return task.GetDependencies().GetRulesetIdDependency().Equals(rulesetId);});}
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                11/2017
+===============+===============+===============+===============+===============+======*/
template<typename T>
struct CancelablePromise : private folly::Promise<T>, ICancelableTask
{
    DEFINE_T_SUPER(folly::Promise<T>)
private:
    BeMutex& m_mutex;
    folly::Future<T> m_future;
    RulesDrivenECPresentationManager::CancelableTasksStore& m_cancelableTasks;
    RefCountedPtr<CancelationToken> m_cancelationToken;
public:
    CancelablePromise(RulesDrivenECPresentationManager::CancelableTasksStore& cancelableTasks, TaskDependencies dependencies)
        : ICancelableTask(dependencies), m_mutex(cancelableTasks.GetMutex()), m_cancelableTasks(cancelableTasks), m_future(T_Super::getFuture())
        {
        m_cancelationToken = new CancelationToken(m_mutex);
        T_Super::setInterruptHandler([&](folly::exception_wrapper const& e)
            {
            BeAssert(e.is_compatible_with<folly::FutureCancellation>() && "Only cancellation exceptions are supported");
            m_cancelationToken->SetCanceled(true);
            m_cancelableTasks.Remove(*this);
            T_Super::setException(e);
            });
        m_cancelableTasks.Add(*this);
        }
    ~CancelablePromise()
        {
        BeAssert(!m_cancelableTasks.Contains(*this));
        m_cancelableTasks.Remove(*this);
        }
    template<class M> void SetValue(M&& value)
        {
        if (!T_Super::isFulfilled())
            T_Super::setValue(std::forward<M>(value));
        m_cancelableTasks.Remove(*this);
        }
    void SetValue() {SetValue(folly::unit);}
    folly::Future<T> GetFuture() {return m_future.then([](T&& result){return result;});}
    void _Cancel() override {m_future.cancel();}
    bool IsCanceled() const {return m_cancelationToken->IsCanceled();}
    ICancelationTokenCR GetCancelationToken() const {return *m_cancelationToken;}
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T>
static std::shared_ptr<CancelablePromise<T>> CreateCancelablePromise(RulesDrivenECPresentationManager::CancelableTasksStore& cancelableTasks, TaskDependencies dependencies = TaskDependencies())
    {
    return std::make_shared<CancelablePromise<T>>(cancelableTasks, dependencies);
    }

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                11/2017
+===============+===============+===============+===============+===============+======*/
struct RulesDrivenECPresentationManager::ECInstanceChangeEventSourceWrapper : ECInstanceChangeEventSource, ECInstanceChangeEventSource::IEventHandler
{
private:
    folly::Executor& m_executor;
    IConnectionCacheCR m_connections;
    ECInstanceChangeEventSourcePtr m_wrapped;
private:
    ECInstanceChangeEventSourceWrapper(folly::Executor& executor, IConnectionCacheCR connections, ECInstanceChangeEventSource& wrapped)
        : m_executor(executor), m_connections(connections), m_wrapped(&wrapped)
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
        if (nullptr == m_connections.GetConnection(db))
            {
            // don't forward the event if connection is not tracked
            return;
            }

        folly::via(&m_executor, [&, connectionId = m_connections.GetConnection(db)->GetId(), changes]()
            {
            IConnectionPtr connection = m_connections.GetConnection(connectionId.c_str());
            NotifyECInstancesChanged(connection->GetDb(), changes);
            });
        }
public:
    static RefCountedPtr<ECInstanceChangeEventSourceWrapper> Create(folly::Executor& executor, IConnectionCacheCR connections, ECInstanceChangeEventSource& wrapped)
        {
        return new ECInstanceChangeEventSourceWrapper(executor, connections, wrapped);
        }
    ECInstanceChangeEventSource& GetWrappedEventSource() const {return *m_wrapped;}
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                11/2017
+===============+===============+===============+===============+===============+======*/
struct RulesDrivenECPresentationManager::SelectionManagerWrapper : ISelectionManager, ISelectionChangesListener
{
private:
    RulesDrivenECPresentationManager& m_manager;
    ISelectionManager& m_wrapped;
    bvector<ISelectionChangesListener*> m_listeners;

protected:
    INavNodeKeysContainerCPtr _GetSelection(ECDbCR ecdb) const override {return m_wrapped.GetSelection(ecdb);}
    INavNodeKeysContainerCPtr _GetSubSelection(ECDbCR ecdb) const override {return m_wrapped.GetSubSelection(ecdb);}
    void _AddListener(ISelectionChangesListener& listener) override {m_listeners.push_back(&listener);}
    void _RemoveListener(ISelectionChangesListener& listener) override {m_listeners.erase(std::remove(m_listeners.begin(), m_listeners.end(), &listener));}
    void _AddToSelection(ECDbCR db, Utf8CP source, bool isSubSelection, INavNodeKeysContainerCR keys, RapidJsonValueCR extendedData) override
        {
        m_wrapped.AddToSelection(db, source, isSubSelection, keys, extendedData);
        }
    void _RemoveFromSelection(ECDbCR db, Utf8CP source, bool isSubSelection, INavNodeKeysContainerCR keys, RapidJsonValueCR extendedData) override
        {
        m_wrapped.RemoveFromSelection(db, source, isSubSelection, keys, extendedData);
        }
    void _ChangeSelection(ECDbCR db, Utf8CP source, bool isSubSelection, INavNodeKeysContainerCR keys, RapidJsonValueCR extendedData) override
        {
        m_wrapped.ChangeSelection(db, source, isSubSelection, keys, extendedData);
        }
    void _ClearSelection(ECDbCR db, Utf8CP source, bool isSubSelection, RapidJsonValueCR extendedData) override
        {
        m_wrapped.ClearSelection(db, source, isSubSelection, extendedData);
        }
    void _OnSelectionChanged(SelectionChangedEventCR evt) override
        {
        m_manager.m_cancelableTasks->CancelSelectionDependants(evt.GetConnection().GetId());
        folly::via(m_manager.m_executor, [&, evt]()
            {
            IConnectionPtr connection = m_manager.GetConnections().GetConnection(evt.GetConnection().GetId().c_str());
            SelectionChangedEvent evtForThisThread(*connection, evt.GetSourceName(), evt.GetChangeType(),
                evt.IsSubSelection(), evt.GetSelectedKeys());
            evtForThisThread.GetExtendedDataR().CopyFrom(evt.GetExtendedData(), evtForThisThread.GetExtendedDataAllocator());
            for (ISelectionChangesListener* listener : m_listeners)
                listener->NotifySelectionChanged(evtForThisThread);
            });
        }

public:
    SelectionManagerWrapper(RulesDrivenECPresentationManager& manager, ISelectionManager& wrapped)
        : m_manager(manager), m_wrapped(wrapped)
        {
        m_wrapped.AddListener(*this);
        }
    ~SelectionManagerWrapper()
        {
        m_wrapped.RemoveListener(*this);
        }
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                11/2017
+===============+===============+===============+===============+===============+======*/
struct RulesDrivenECPresentationManager::ConnectionManagerWrapper : IConnectionManager, IConnectionsListener
{
private:
    RulesDrivenECPresentationManager& m_manager;
    IConnectionManagerR m_wrapped;
    mutable bvector<IConnectionsListener*> m_listeners;

protected:
    IConnection* _GetConnection(Utf8CP connectionId) const override {return m_wrapped.GetConnection(connectionId);}
    IConnection* _GetConnection(ECDbCR ecdb) const override {return m_wrapped.GetConnection(ecdb);}
    IConnectionPtr _CreateConnection(ECDbR ecdb) override {return m_wrapped.CreateConnection(ecdb);}
    void _AddListener(IConnectionsListener& listener) const override {m_listeners.push_back(&listener);}
    void _DropListener(IConnectionsListener& listener) const override {m_listeners.erase(std::remove(m_listeners.begin(), m_listeners.end(), &listener));}
    void _OnConnectionEvent(ConnectionEvent const& evt) override
        {
        if (ConnectionEventType::Closed == evt.GetEventType())
            m_manager.m_cancelableTasks->CancelByConnectionId(evt.GetConnection().GetId());

        folly::via(m_manager.m_executor, [&, evt, connectionId = evt.GetConnection().GetId()]()
            {
            IConnectionPtr connection = m_wrapped.GetConnection(connectionId.c_str());
            ConnectionEvent evtForThisThread(*connection, evt.IsPrimaryConnection(), evt.GetEventType());
            for (IConnectionsListener* listener : m_listeners)
                listener->NotifyConnectionEvent(evtForThisThread);
            }).wait();
        }

public:
    ConnectionManagerWrapper(RulesDrivenECPresentationManager& manager, IConnectionManagerR wrapped)
        : m_manager(manager), m_wrapped(wrapped)
        {
        m_wrapped.AddListener(*this);
        }
    ~ConnectionManagerWrapper()
        {
        m_wrapped.DropListener(*this);
        }
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                11/2017
+===============+===============+===============+===============+===============+======*/
struct RulesDrivenECPresentationManager::RulesetLocaterManagerWrapper : IRulesetLocaterManager, IRulesetCallbacksHandler
{
private:
    RulesDrivenECPresentationManager& m_manager;
    RuleSetLocaterManager m_wrapped;

protected:
    // IRulesetLocaterManager
    void _InvalidateCache(Utf8CP rulesetId) override {m_wrapped.InvalidateCache(rulesetId);}
    void _RegisterLocater(RuleSetLocater& locater) override {m_wrapped.RegisterLocater(locater);}
    void _UnregisterLocater(RuleSetLocater const& locater) override {m_wrapped.UnregisterLocater(locater);}
    bvector<PresentationRuleSetPtr> _LocateRuleSets(ECDbCR db, Utf8CP rulesetId) const override {return m_wrapped.LocateRuleSets(db, rulesetId);}
    bvector<Utf8String> _GetRuleSetIds() const override {return m_wrapped.GetRuleSetIds();}

    // IRulesetCallbacksHandler
    void _OnRulesetDispose(PresentationRuleSetCR ruleset) override
        {
        m_manager.m_cancelableTasks->CancelByRulesetId(ruleset.GetRuleSetId());
        folly::via(&m_manager.GetExecutor(), [&, ruleset = PresentationRuleSetCPtr(&ruleset)]()
            {
            if (nullptr != GetRulesetCallbacksHandler())
                GetRulesetCallbacksHandler()->_OnRulesetDispose(*ruleset);
            });
        }
    void _OnRulesetCreated(PresentationRuleSetCR ruleset) override
        {
        // note this callback is blocking because we need to initialize some stuff
        // like user settings and expect it to be accessible immediately after
        // ruleset is created
        folly::via(&m_manager.GetExecutor(), [&, ruleset = PresentationRuleSetCPtr(&ruleset)]()
            {
            if (nullptr != GetRulesetCallbacksHandler())
                GetRulesetCallbacksHandler()->_OnRulesetCreated(*ruleset);
            }).wait();
        }

public:
    RulesetLocaterManagerWrapper(RulesDrivenECPresentationManager& manager, IConnectionManagerCR connections)
        : m_manager(manager), m_wrapped(connections)
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
    folly::Executor& m_executor;
    UserSettingsManager m_wrapped;
protected:
    IUserSettings& _GetSettings(Utf8StringCR rulesetId) const override {return m_wrapped.GetSettings(rulesetId);}
    void _OnLocalizationProviderChanged() override {m_wrapped.SetLocalizationProvider(GetLocalizationProvider());}
    void _OnLocalStateChanged() override {m_wrapped.SetLocalState(GetLocalState());}
    void _OnSettingChanged(Utf8CP rulesetId, Utf8CP settingId) const override
        {
        folly::via(&m_executor, [&, rulesetId = Utf8String(rulesetId), settingId = Utf8String(settingId)]()
            {
            if (nullptr != GetChangesListener())
                GetChangesListener()->_OnSettingChanged(rulesetId.c_str(), settingId.c_str());
            });
        }
public:
    UserSettingsManagerWrapper(folly::Executor& executor, BeFileNameCR temporaryDirectory)
        : m_wrapped(temporaryDirectory), m_executor(executor)
        {
        m_wrapped.SetChangesListener(this);
        }
    };

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                11/2017
+===============+===============+===============+===============+===============+======*/
struct WrappedDependenciesFactory : IRulesDrivenECPresentationManagerDependenciesFactory
    {
    RulesDrivenECPresentationManager& m_manager;
    IRulesetLocaterManager* _CreateRulesetLocaterManager(IConnectionManagerCR connections) const override {return new RulesDrivenECPresentationManager::RulesetLocaterManagerWrapper(m_manager, connections);}
    IUserSettingsManager* _CreateUserSettingsManager(BeFileNameCR tempDir) const override {return new RulesDrivenECPresentationManager::UserSettingsManagerWrapper(m_manager.GetExecutor(), tempDir);}
    WrappedDependenciesFactory(RulesDrivenECPresentationManager& manager) : m_manager(manager) {}
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
std::unique_ptr<IRulesDrivenECPresentationManagerDependenciesFactory> RulesDrivenECPresentationManager::CreateDependenciesFactory()
    {
    return std::make_unique<WrappedDependenciesFactory>(*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
RulesDrivenECPresentationManager::RulesDrivenECPresentationManager(IConnectionManagerR connections, Paths const& paths, bool disableDiskCache)
    : IECPresentationManager(connections), m_selectionProviderWrapper(nullptr)
    {
#ifdef RULES_ENGINE_FORCE_SINGLE_THREAD
    m_executor = new folly::InlineExecutor();
#else
    m_executor = new SingleThreadedQueueExecutor("ECPresentation");
#endif
    m_cancelableTasks = new CancelableTasksStore();
    m_connectionsWrapper = new ConnectionManagerWrapper(*this, connections);
    m_impl = new RulesDrivenECPresentationManagerImpl(*CreateDependenciesFactory(), *m_connectionsWrapper, paths, disableDiskCache);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
RulesDrivenECPresentationManager::~RulesDrivenECPresentationManager()
    {
    // cancel all pending promises and wait for the worker thread to terminate -
    // it allows the canceled promises finish gracefully
    m_cancelableTasks->CancelAll();
    folly::via(m_executor, [&]()
        {
        // terminate on the ECPresentation thread (m_impl is not thread safe and should only
        // be used from one thread)
        DELETE_AND_CLEAR(m_impl);
        });
#ifndef RULES_ENGINE_FORCE_SINGLE_THREAD
    static_cast<SingleThreadedQueueExecutor&>(*m_executor).Terminate().wait();
#endif

    DELETE_AND_CLEAR(m_connectionsWrapper);
    DELETE_AND_CLEAR(m_selectionProviderWrapper);

    DELETE_AND_CLEAR(m_executor);
    DELETE_AND_CLEAR(m_cancelableTasks);
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
* @bsimethod                                    Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
IUserSettingsManager& RulesDrivenECPresentationManager::GetUserSettings() const {return m_impl->GetUserSettingsManager();}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<NavNodesContainer> RulesDrivenECPresentationManager::_GetRootNodes(IConnectionCR primaryConnection, PageOptionsCR pageOpts, JsonValueCR jsonOptions)
    {
    auto promise = CreateCancelablePromise<INavNodesDataSourcePtr>(*m_cancelableTasks, TaskDependencies(primaryConnection.GetId(), NavigationOptions(jsonOptions).GetRulesetId()));
    folly::via(m_executor, [&, promise, connectionId = primaryConnection.GetId(), pageOpts, jsonOptions]()
        {
        if (promise->IsCanceled())
            return;

        NavigationOptions options(jsonOptions);
        IConnectionPtr connection = GetConnections().GetConnection(connectionId.c_str());
        INavNodesDataSourcePtr source = m_impl->GetRootNodes(*connection, pageOpts, options, promise->GetCancelationToken());
        if (source.IsValid())
            source = PreloadedDataSource::Create(*source);
        promise->SetValue(source);
        });
    return promise->GetFuture().then([](INavNodesDataSourcePtr ds)
        {
        return DataContainer<NavNodeCPtr>(*ds);
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<size_t> RulesDrivenECPresentationManager::_GetRootNodesCount(IConnectionCR primaryConnection, JsonValueCR jsonOptions)
    {
    auto promise = CreateCancelablePromise<size_t>(*m_cancelableTasks, TaskDependencies(primaryConnection.GetId(), NavigationOptions(jsonOptions).GetRulesetId()));
    folly::via(m_executor, [&, promise, connectionId = primaryConnection.GetId(), jsonOptions]()
        {
        if (promise->IsCanceled())
            return;

        NavigationOptions options(jsonOptions);
        IConnectionPtr connection = GetConnections().GetConnection(connectionId.c_str());
        size_t count = m_impl->GetRootNodesCount(*connection, options, promise->GetCancelationToken());
        promise->SetValue(count);
        });
    return promise->GetFuture();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<NavNodesContainer> RulesDrivenECPresentationManager::_GetChildren(IConnectionCR primaryConnection, NavNodeCR parent, PageOptionsCR pageOpts, JsonValueCR jsonOptions)
    {
    auto promise = CreateCancelablePromise<INavNodesDataSourcePtr>(*m_cancelableTasks, TaskDependencies(primaryConnection.GetId(), NavigationOptions(jsonOptions).GetRulesetId()));
    folly::via(m_executor, [&, promise, connectionId = primaryConnection.GetId(), parent = (NavNodeCPtr)&parent, pageOpts, jsonOptions]()
        {
        if (promise->IsCanceled())
            return;

        NavigationOptions options(jsonOptions);
        IConnectionPtr connection = GetConnections().GetConnection(connectionId.c_str());
        INavNodesDataSourcePtr source = m_impl->GetChildren(*connection, *parent, pageOpts, options, promise->GetCancelationToken());
        if (source.IsValid())
            source = PreloadedDataSource::Create(*source);
        promise->SetValue(source);
        });
    return promise->GetFuture().then([](INavNodesDataSourcePtr ds)
        {
        return DataContainer<NavNodeCPtr>(*ds);
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<size_t> RulesDrivenECPresentationManager::_GetChildrenCount(IConnectionCR primaryConnection, NavNodeCR parent, JsonValueCR jsonOptions)
    {
    auto promise = CreateCancelablePromise<size_t>(*m_cancelableTasks, TaskDependencies(primaryConnection.GetId(), NavigationOptions(jsonOptions).GetRulesetId()));
    folly::via(m_executor, [&, promise, connectionId = primaryConnection.GetId(), parent = (NavNodeCPtr)&parent, jsonOptions]()
        {
        if (promise->IsCanceled())
            return;

        NavigationOptions options(jsonOptions);
        IConnectionPtr connection = GetConnections().GetConnection(connectionId.c_str());
        size_t count = m_impl->GetChildrenCount(*connection, *parent, options, promise->GetCancelationToken());
        promise->SetValue(count);
        });
    return promise->GetFuture();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<NavNodeCPtr> RulesDrivenECPresentationManager::_GetParent(IConnectionCR primaryConnection, NavNodeCR node, JsonValueCR jsonOptions)
    {
    auto promise = CreateCancelablePromise<NavNodeCPtr>(*m_cancelableTasks, TaskDependencies(primaryConnection.GetId(), NavigationOptions(jsonOptions).GetRulesetId()));
    folly::via(m_executor, [&, promise, connectionId = primaryConnection.GetId(), node = (NavNodeCPtr)&node, jsonOptions]()
        {
        if (promise->IsCanceled())
            return;

        NavigationOptions options(jsonOptions);
        IConnectionPtr connection = GetConnections().GetConnection(connectionId.c_str());
        NavNodeCPtr parentNode = m_impl->GetParent(*connection, *node, options, promise->GetCancelationToken());
        promise->SetValue(parentNode);
        });
    return promise->GetFuture();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<NavNodeCPtr> RulesDrivenECPresentationManager::_GetNode(IConnectionCR primaryConnection, uint64_t nodeId)
    {
    auto promise = CreateCancelablePromise<NavNodeCPtr>(*m_cancelableTasks, TaskDependencies(primaryConnection.GetId()));
    folly::via(m_executor, [&, promise, connectionId = primaryConnection.GetId(), nodeId]()
        {
        if (promise->IsCanceled())
            return;

        IConnectionPtr connection = GetConnections().GetConnection(connectionId.c_str());
        NavNodeCPtr node = m_impl->GetNode(*connection, nodeId, promise->GetCancelationToken());
        promise->SetValue(node);
        });
    return promise->GetFuture();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<bvector<NavNodeCPtr>> RulesDrivenECPresentationManager::_GetFilteredNodes(IConnectionCR primaryConnection, Utf8CP filterText, JsonValueCR jsonOptions)
    {
    auto promise = CreateCancelablePromise<bvector<NavNodeCPtr>>(*m_cancelableTasks, TaskDependencies(primaryConnection.GetId(), NavigationOptions(jsonOptions).GetRulesetId()));
    folly::via(m_executor, [&, promise, connectionId = primaryConnection.GetId(), filterText = Utf8String(filterText), jsonOptions]()
        {
        if (promise->IsCanceled())
            return;
        
        NavigationOptions options(jsonOptions);
        IConnectionPtr connection = GetConnections().GetConnection(connectionId.c_str());
        bvector<NavNodeCPtr> nodes = m_impl->GetFilteredNodes(*connection, filterText.c_str(), options, promise->GetCancelationToken());
        promise->SetValue(nodes);
        });
    return promise->GetFuture();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<bool> RulesDrivenECPresentationManager::_HasChild(IConnectionCR primaryConnection, NavNodeCR parent, NavNodeKeyCR childKey, JsonValueCR jsonOptions)
    {
    auto promise = CreateCancelablePromise<bool>(*m_cancelableTasks, TaskDependencies(primaryConnection.GetId(), NavigationOptions(jsonOptions).GetRulesetId()));
    folly::via(m_executor, [&, promise, connectionId = primaryConnection.GetId(), parent = (NavNodeCPtr)&parent, childKey = (NavNodeKeyCPtr)&childKey, jsonOptions]()
        {
        if (promise->IsCanceled())
            return;

        NavigationOptions options(jsonOptions);
        IConnectionPtr connection = GetConnections().GetConnection(connectionId.c_str());
        bool result = m_impl->HasChild(*connection, *parent, *childKey, options, promise->GetCancelationToken());
        promise->SetValue(result);
        });
    return promise->GetFuture();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<bvector<SelectClassInfo>> RulesDrivenECPresentationManager::_GetContentClasses(IConnectionCR primaryConnection, Utf8CP preferredDisplayType, bvector<ECClassCP> const& classes, JsonValueCR jsonOptions)
    {
    auto promise = CreateCancelablePromise<bvector<SelectClassInfo>>(*m_cancelableTasks, TaskDependencies(primaryConnection.GetId(), ContentOptions(jsonOptions).GetRulesetId()));
    folly::via(m_executor, [&, promise, connectionId = primaryConnection.GetId(), displayType = (Utf8String)preferredDisplayType, classes, jsonOptions]()
        {
        if (promise->IsCanceled())
            return;

        ContentOptions options(jsonOptions);
        IConnectionPtr connection = GetConnections().GetConnection(connectionId.c_str());
        bvector<SelectClassInfo> contentClasses = m_impl->GetContentClasses(*connection, displayType.c_str(), classes, options, promise->GetCancelationToken());
        promise->SetValue(contentClasses);
        });

    return promise->GetFuture();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<ContentDescriptorCPtr> RulesDrivenECPresentationManager::_GetContentDescriptor(IConnectionCR primaryConnection, Utf8CP preferredDisplayType, INavNodeKeysContainerCR inputKeys, SelectionInfo const* selectionInfo, JsonValueCR jsonOptions)
    {
    auto promise = CreateCancelablePromise<ContentDescriptorCPtr>(*m_cancelableTasks, TaskDependencies(primaryConnection.GetId(), ContentOptions(jsonOptions).GetRulesetId(), true));
    folly::via(m_executor, [&, promise, connectionId = primaryConnection.GetId(), displayType = (Utf8String)preferredDisplayType, input = INavNodeKeysContainerCPtr(&inputKeys), selectionInfo, jsonOptions]()
        {
        if (promise->IsCanceled())
            return;

        ContentOptions options(jsonOptions);
        IConnectionPtr connection = GetConnections().GetConnection(connectionId.c_str());
        ContentDescriptorCPtr descriptor = m_impl->GetContentDescriptor(*connection, displayType.c_str(), *input, selectionInfo, options, promise->GetCancelationToken());
        promise->SetValue(descriptor);
        });

    return promise->GetFuture();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<ContentCPtr> RulesDrivenECPresentationManager::_GetContent(ContentDescriptorCR descriptor, PageOptionsCR pageOpts)
    {
    auto promise = CreateCancelablePromise<ContentCPtr>(*m_cancelableTasks, TaskDependencies(descriptor.GetConnection().GetId(), ContentOptions(descriptor.GetOptions()).GetRulesetId(), true));
    folly::via(m_executor, [&, promise, descriptor = ContentDescriptorCPtr(&descriptor), pageOpts]()
        {
        if (promise->IsCanceled())
            return;

        ContentCPtr content = m_impl->GetContent(*descriptor, pageOpts, promise->GetCancelationToken());
        promise->SetValue(content);
        });
    return promise->GetFuture();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<size_t> RulesDrivenECPresentationManager::_GetContentSetSize(ContentDescriptorCR descriptor)
    {
    auto promise = CreateCancelablePromise<size_t>(*m_cancelableTasks, TaskDependencies(descriptor.GetConnection().GetId(), ContentOptions(descriptor.GetOptions()).GetRulesetId(), true));
    folly::via(m_executor, [&, promise, descriptor = ContentDescriptorCPtr(&descriptor)]()
        {
        if (promise->IsCanceled())
            return;

        size_t size = m_impl->GetContentSetSize(*descriptor, promise->GetCancelationToken());
        promise->SetValue(size);
        });
    return promise->GetFuture();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
IRulesetLocaterManager& RulesDrivenECPresentationManager::GetLocaters() {return m_impl->GetLocaters();}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
IUserSettings& RulesDrivenECPresentationManager::GetUserSettings(Utf8CP rulesetId) const {return m_impl->GetUserSettings(rulesetId);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenECPresentationManager::SetLocalState(IJsonLocalState* localState) {m_impl->SetLocalState(localState);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenECPresentationManager::RegisterECInstanceChangeEventSource(ECInstanceChangeEventSource& source)
    {
    RefCountedPtr<ECInstanceChangeEventSourceWrapper> wrapper = ECInstanceChangeEventSourceWrapper::Create(*m_executor, GetConnections(), source);
    m_impl->RegisterECInstanceChangeEventSource(*wrapper);
    m_ecInstanceChangeEventSourceWrappers.push_back(wrapper.get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenECPresentationManager::UnregisterECInstanceChangeEventSource(ECInstanceChangeEventSource& source)
    {
    for (auto iter = m_ecInstanceChangeEventSourceWrappers.begin(); iter != m_ecInstanceChangeEventSourceWrappers.end(); ++iter)
        {
        ECInstanceChangeEventSourceWrapper* wrapper = (*iter);
        if (&wrapper->GetWrappedEventSource() == &source)
            {
            m_ecInstanceChangeEventSourceWrappers.erase(iter);
            m_impl->UnregisterECInstanceChangeEventSource(*wrapper);
            return;
            }
        }
    BeAssert(false && "Did not find an ECInstanceChangeEventSource to remove");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenECPresentationManager::SetUpdateRecordsHandler(IUpdateRecordsHandler* handler) {m_impl->SetUpdateRecordsHandler(handler);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenECPresentationManager::SetSelectionManager(ISelectionManager* manager)
    {
    SelectionManagerWrapper* oldWrapper = m_selectionProviderWrapper;
    SelectionManagerWrapper* newWrapper = (nullptr != manager) ? new SelectionManagerWrapper(*this, *manager) : nullptr;
    m_impl->SetSelectionManager(newWrapper);
    DELETE_AND_CLEAR(oldWrapper);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ISelectionManager* RulesDrivenECPresentationManager::GetSelectionManager() const {return m_impl->GetSelectionManager();}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenECPresentationManager::SetECPropertyFormatter(IECPropertyFormatter const* formatter) {m_impl->SetECPropertyFormatter(formatter);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
IECPropertyFormatter const& RulesDrivenECPresentationManager::GetECPropertyFormatter() const {return m_impl->GetECPropertyFormatter();}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenECPresentationManager::SetCategorySupplier(IPropertyCategorySupplier* supplier) {m_impl->SetCategorySupplier(supplier);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
IPropertyCategorySupplier& RulesDrivenECPresentationManager::GetCategorySupplier() const {return m_impl->GetCategorySupplier();}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenECPresentationManager::NotifyCategoriesChanged()
    {
    folly::via(m_executor, [&]()
        {
        m_impl->NotifyCategoriesChanged();
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ILocalizationProvider const& RulesDrivenECPresentationManager::GetLocalizationProvider() const {return m_impl->GetLocalizationProvider();}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenECPresentationManager::SetLocalizationProvider(ILocalizationProvider const* provider) {m_impl->SetLocalizationProvider(provider);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenECPresentationManager::RegisterECInstanceChangeHandler(IECInstanceChangeHandler& handler)
    {
    m_impl->RegisterECInstanceChangeHandler(handler);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenECPresentationManager::UnregisterECInstanceChangeHandler(IECInstanceChangeHandler& handler)
    {
    m_impl->UnregisterECInstanceChangeHandler(handler);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<bvector<ECInstanceChangeResult>> RulesDrivenECPresentationManager::_SaveValueChange(IConnectionCR primaryConnection, bvector<ChangedECInstanceInfo> const& instances,
    Utf8CP propertyAccessor, ECValueCR value, JsonValueCR)
    {
    // note: the connection should be changed only on the primary thread so we're not
    // switching to the executor thread here.
    return m_impl->SaveValueChange(primaryConnection, instances, propertyAccessor, value);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<folly::Unit> RulesDrivenECPresentationManager::_OnNodeChecked(IConnectionCR primaryConnection, uint64_t nodeId)
    {
    auto promise = CreateCancelablePromise<folly::Unit>(*m_cancelableTasks);
    folly::via(m_executor, [&, promise, connectionId = primaryConnection.GetId(), nodeId]()
        {
        if (promise->IsCanceled())
            return;

        IConnectionPtr connection = GetConnections().GetConnection(connectionId.c_str());
        m_impl->NotifyNodeChecked(*connection, nodeId, promise->GetCancelationToken());
        promise->SetValue();
        });
    return promise->GetFuture();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<folly::Unit> RulesDrivenECPresentationManager::_OnNodeUnchecked(IConnectionCR primaryConnection, uint64_t nodeId)
    {
    auto promise = CreateCancelablePromise<folly::Unit>(*m_cancelableTasks);
    folly::via(m_executor, [&, promise, connectionId = primaryConnection.GetId(), nodeId]()
        {
        if (promise->IsCanceled())
            return;

        IConnectionPtr connection = GetConnections().GetConnection(connectionId.c_str());
        m_impl->NotifyNodeUnchecked(*connection, nodeId, promise->GetCancelationToken());
        promise->SetValue();
        });
    return promise->GetFuture();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<folly::Unit> RulesDrivenECPresentationManager::_OnNodeExpanded(IConnectionCR primaryConnection, uint64_t nodeId)
    {
    auto promise = CreateCancelablePromise<folly::Unit>(*m_cancelableTasks);
    folly::via(m_executor, [&, promise, connectionId = primaryConnection.GetId(), nodeId]()
        {
        if (promise->IsCanceled())
            return;

        IConnectionPtr connection = GetConnections().GetConnection(connectionId.c_str());
        m_impl->NotifyNodeExpanded(*connection, nodeId, promise->GetCancelationToken());
        promise->SetValue();
        });
    return promise->GetFuture();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<folly::Unit> RulesDrivenECPresentationManager::_OnNodeCollapsed(IConnectionCR primaryConnection, uint64_t nodeId)
    {
    auto promise = CreateCancelablePromise<folly::Unit>(*m_cancelableTasks);
    folly::via(m_executor, [&, promise, connectionId = primaryConnection.GetId(), nodeId]()
        {
        if (promise->IsCanceled())
            return;

        IConnectionPtr connection = GetConnections().GetConnection(connectionId.c_str());
        m_impl->NotifyNodeCollapsed(*connection, nodeId, promise->GetCancelationToken());
        promise->SetValue();
        });
    return promise->GetFuture();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<folly::Unit> RulesDrivenECPresentationManager::_OnAllNodesCollapsed(IConnectionCR primaryConnection, JsonValueCR jsonOptions)
    {
    auto promise = CreateCancelablePromise<folly::Unit>(*m_cancelableTasks);
    folly::via(m_executor, [&, promise, connectionId = primaryConnection.GetId(), jsonOptions]()
        {
        if (promise->IsCanceled())
            return;

        NavigationOptions options(jsonOptions);
        IConnectionPtr connection = GetConnections().GetConnection(connectionId.c_str());
        m_impl->NotifyAllNodesCollapsed(*connection, options, promise->GetCancelationToken());
        promise->SetValue();
        });
    return promise->GetFuture();
    }
