/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include "UpdateHandler.h"
#include "ExtendedData.h"
#include "NavNodeProviders.h"
#include "ECExpressionContextsProvider.h"
#include "CustomizationHelper.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document UpdateRecord::AsJson(rapidjson::Document::AllocatorType* allocator) const
    {
    return IECPresentationManager::GetSerializer().AsJson(*this, allocator);
    }

static uint64_t s_taskId = 1;
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
IUpdateTask::IUpdateTask() : m_didPerform(false), m_id(s_taskId++) {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<IUpdateTaskPtr> IUpdateTask::Perform()
    {
    BeAssert(!m_didPerform);
    bvector<IUpdateTaskPtr> tasks = _Perform(); 
    m_didPerform = true;
    return tasks;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String IUpdateTask::GetPrintStr() const
    {
    return Utf8PrintfString("Id: %" PRIu64 ". %s",
        GetId(), _GetPrintStr().c_str());
    }

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                02/2016
+===============+===============+===============+===============+===============+======*/
struct RemapNodeIdsTask : IUpdateTask
{
private:
    NodesCache& m_cache;
    bmap<uint64_t, uint64_t> const& m_remapInfo;

protected:
    uint32_t _GetPriority() const override {return TASK_PRIORITY_RemapNodeIds;}
    bvector<IUpdateTaskPtr> _Perform() override
        {
        m_cache.RemapNodeIds(m_remapInfo);
        return bvector<IUpdateTaskPtr>();
        }
    Utf8String _GetPrintStr() const override
        {
        Utf8String str = "[RemapNodeIds]";
        if (!DidPerform() && !m_remapInfo.empty())
            {
            bool first = true;
            str.append("[");
            for (auto iter = m_remapInfo.begin(); iter != m_remapInfo.end(); ++iter)
                {
                if (!first)
                    str.append(",");
                str.append(Utf8PrintfString("%" PRIu64 "->%" PRIu64, iter->first, iter->second));
                first = false;
                }
            str.append("]");
            }
        return str;
        }
public:
    RemapNodeIdsTask(NodesCache& cache, bmap<uint64_t, uint64_t> const& remapInfo) 
        : m_cache(cache), m_remapInfo(remapInfo)
        {}
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                02/2016
+===============+===============+===============+===============+===============+======*/
struct RemoveHierarchyLevelTask : IUpdateTask
{
private:
    NodesCache& m_cache;
    BeGuid m_removalId;

protected:
    virtual uint32_t _GetPriority() const override {return TASK_PRIORITY_RemoveHierarchyLevel;}
    virtual bvector<IUpdateTaskPtr> _Perform() override
        {
        m_cache.RemoveHierarchyLevel(m_removalId);
        return bvector<IUpdateTaskPtr>();
        }
    virtual Utf8String _GetPrintStr() const override
        {
        Utf8String str = "[RemoveHierarchyLevel]";
        if (!DidPerform())
            str.append(" RemovalId = ").append(m_removalId.ToString());
        return str;
        }
public:
    RemoveHierarchyLevelTask(NodesCache& cache, BeGuid removalId) 
        : m_cache(cache), m_removalId(removalId)
        {}
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                10/2016
+===============+===============+===============+===============+===============+======*/
struct RefreshHierarchyTask : IUpdateTask
{
private:
    HierarchyUpdater const& m_updater;
    UpdateContext& m_updateContext;
    IConnectionCR m_connection;
    HierarchyLevelInfo m_hierarchyInfo;
protected:
    virtual uint32_t _GetPriority() const override {return TASK_PRIORITY_RefreshHierarchy;}
    virtual bvector<IUpdateTaskPtr> _Perform() override
        {
        HierarchyLevelInfo newInfo(m_hierarchyInfo);
        newInfo.Invalidate();
        if (nullptr != m_hierarchyInfo.GetPhysicalParentNodeId())
            {
            auto iter = m_updateContext.GetRemapInfo().find(*m_hierarchyInfo.GetPhysicalParentNodeId());
            if (m_updateContext.GetRemapInfo().end() != iter)
                newInfo.SetPhysicalParentNodeId(iter->second);
            }
        if (nullptr != m_hierarchyInfo.GetVirtualParentNodeId())
            {
            auto iter = m_updateContext.GetRemapInfo().find(*m_hierarchyInfo.GetVirtualParentNodeId());
            if (m_updateContext.GetRemapInfo().end() != iter)
                newInfo.SetVirtualParentNodeId(iter->second);
            }
        bvector<IUpdateTaskPtr> subTasks;
        m_updater.Update(subTasks, m_updateContext, m_connection, m_hierarchyInfo, newInfo);
        return subTasks;
        }
    virtual Utf8String _GetPrintStr() const override
        {
        Utf8String str = "[RefreshHierarchyTask]";
        if (!DidPerform())
            {
            str.append(" ConnectionId: ").append(m_hierarchyInfo.GetConnectionId());
            str.append(" RulesetId: ").append(m_hierarchyInfo.GetRulesetId());
            str.append(" PhysicalParentId: ").append((nullptr == m_hierarchyInfo.GetPhysicalParentNodeId()) ? "null" : std::to_string(*m_hierarchyInfo.GetPhysicalParentNodeId()).c_str());
            str.append(" VirtualParentId: ").append((nullptr == m_hierarchyInfo.GetVirtualParentNodeId()) ? "null" : std::to_string(*m_hierarchyInfo.GetVirtualParentNodeId()).c_str());
            }
        return str;
        }
public:
    RefreshHierarchyTask(HierarchyUpdater const& updater, UpdateContext& updateContext, IConnectionCR connection, HierarchyLevelInfo info) 
        : m_updater(updater), m_updateContext(updateContext), m_connection(connection), m_hierarchyInfo(info)
        {}
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                10/2016
+===============+===============+===============+===============+===============+======*/
struct InvalidateContentTask : IUpdateTask
{
private:
    UpdateTasksFactoryCR m_tasksFactory;
    UpdateContext& m_updateContext;
    ContentCache& m_contentCache;
    ContentProviderPtr m_provider;
protected:
    uint32_t _GetPriority() const override {return TASK_PRIORITY_InvalidateContent;}
    bvector<IUpdateTaskPtr> _Perform() override
        {
        bvector<IUpdateTaskPtr> subTasks;
        Utf8StringCR rulesetId = m_provider->GetContext().GetRuleset().GetRuleSetId();
        if (nullptr == m_provider->GetContentDescriptor())
            m_contentCache.ClearCache(rulesetId.c_str());
        else
            m_provider->InvalidateContent();

        if (m_updateContext.GetReportedContentRulesetIds().end() == m_updateContext.GetReportedContentRulesetIds().find(rulesetId))
            {
            subTasks.push_back(m_tasksFactory.CreateReportTask(FullUpdateRecord(rulesetId, FullUpdateRecord::UpdateTarget::Content)));
            m_updateContext.GetReportedContentRulesetIds().insert(rulesetId);
            }
        return subTasks;
        }
    Utf8String _GetPrintStr() const override
        {
        Utf8String str = "[InvalidateContentTask]";
        if (!DidPerform() && nullptr != m_provider->GetContentDescriptor())
            {
            ContentDescriptorCR descr = *m_provider->GetContentDescriptor();
            str.append(Utf8PrintfString(" DisplayType = '%s'", descr.GetPreferredDisplayType().c_str()));
            }
        return str;
        }
public:
    InvalidateContentTask(UpdateTasksFactoryCR factory, UpdateContext& updateContext, ContentCache& contentCache, ContentProviderR provider) 
        : m_tasksFactory(factory), m_updateContext(updateContext), m_contentCache(contentCache), m_provider(&provider)
        {}
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                02/2016
+===============+===============+===============+===============+===============+======*/
struct HierarchyChangeReportTask : IUpdateTask
{
private:
    IUpdateRecordsHandler& m_recordsHandler;
    UpdateRecord m_record;
private:
    static Utf8String GetString(UpdateRecord const& record)
        {
        switch (record.GetChangeType())
            {
            case ChangeType::Delete:
                return Utf8PrintfString("Delete");
            case ChangeType::Insert:
                return Utf8PrintfString("Insert at position %" PRIu64, record.GetPosition());
            case ChangeType::Update:
                return Utf8PrintfString("Update");
            }
        return "";
        }
protected:
    uint32_t _GetPriority() const override {return TASK_PRIORITY_Report;}
    bvector<IUpdateTaskPtr> _Perform() override {m_recordsHandler.Accept(m_record); return bvector<IUpdateTaskPtr>();}
    Utf8String _GetPrintStr() const override
        {
        Utf8String str = "[HierarchyChangeReport]";
        if (!DidPerform())
            {
            NavNodeCPtr node = m_record.GetNode();
            str.append(Utf8PrintfString(" %s: Type = %s, Label = %s, Id = %" PRIu64 ", ParentId = %" PRIu64, 
                GetString(m_record).c_str(), node->GetType().c_str(), node->GetLabel().c_str(), node->GetNodeId(), node->GetParentNodeId()));
            }
        return str;
        }
public:
    HierarchyChangeReportTask(IUpdateRecordsHandler& recordsHandler, UpdateRecord record) 
        : m_recordsHandler(recordsHandler), m_record(record)
        {}
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                02/2016
+===============+===============+===============+===============+===============+======*/
struct FullUpdateReportTask : IUpdateTask
{
private:
    IUpdateRecordsHandler& m_recordsHandler;
    FullUpdateRecord m_record;
protected:
    uint32_t _GetPriority() const override {return TASK_PRIORITY_Report;}
    bvector<IUpdateTaskPtr> _Perform() override {m_recordsHandler.Accept(m_record); return bvector<IUpdateTaskPtr>();}
    Utf8String _GetPrintStr() const override
        {
        return Utf8PrintfString("[FullUpdateReportTask] RulesetId = '%s'", m_record.GetRulesetId().c_str());
        }
public:
    FullUpdateReportTask(IUpdateRecordsHandler& recordsHandler, FullUpdateRecord record) 
        : m_recordsHandler(recordsHandler), m_record(record)
        {}
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
IUpdateTaskPtr UpdateTasksFactory::CreateRemapNodeIdsTask(bmap<uint64_t, uint64_t> const& remapInfo) const
    {
    if (nullptr == m_nodesCache)
        {
        BeAssert(false);
        return nullptr;
        }

    return new RemapNodeIdsTask(*m_nodesCache, remapInfo);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
IUpdateTaskPtr UpdateTasksFactory::CreateRemoveHierarchyLevelTask(BeGuidCR removalId) const
    {
    if (nullptr == m_nodesCache)
        {
        BeAssert(false);
        return nullptr;
        }

    return new RemoveHierarchyLevelTask(*m_nodesCache, removalId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
IUpdateTaskPtr UpdateTasksFactory::CreateRefreshHierarchyTask(HierarchyUpdater const& updater, UpdateContext& updateContext, 
    IConnectionCR db, HierarchyLevelInfo const& info) const
    {
    return new RefreshHierarchyTask(updater, updateContext, db, info);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
IUpdateTaskPtr UpdateTasksFactory::CreateContentInvalidationTask(ContentCache& contentCache, UpdateContext& updateContext, ContentProviderR provider) const
    {
    return new InvalidateContentTask(*this, updateContext, contentCache, provider);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
IUpdateTaskPtr UpdateTasksFactory::CreateReportTask(UpdateRecord record) const
    {
    if (nullptr == m_nodesCache)
        {
        BeAssert(false);
        return nullptr;
        }

    if (nullptr == m_recordsHandler)
        return nullptr;

    return new HierarchyChangeReportTask(*m_recordsHandler, record);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
IUpdateTaskPtr UpdateTasksFactory::CreateReportTask(FullUpdateRecord record) const
    {
    if (nullptr == m_recordsHandler)
        return nullptr;

    return new FullUpdateReportTask(*m_recordsHandler, record);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
UpdateHandler::UpdateHandler(NodesCache* nodesCache, ContentCache* contentCache, IConnectionManagerCR connections, INodesProviderContextFactoryCR contextFactory, 
    INodesProviderFactoryCR providerFactory, IECExpressionsCacheProvider& ecexpressionsCacheProvider) 
    : m_nodesCache(nodesCache), m_contentCache(contentCache), m_connections(connections), m_tasksFactory(nodesCache, contentCache, nullptr), 
    m_ecexpressionsCacheProvider(ecexpressionsCacheProvider), m_hierarchyUpdater(nullptr)
    {
    if (nullptr != nodesCache)
        m_hierarchyUpdater = new HierarchyUpdater(m_tasksFactory, *nodesCache, contextFactory, providerFactory);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
UpdateHandler::~UpdateHandler()
    {
    DELETE_AND_CLEAR(m_hierarchyUpdater);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<HierarchyLevelInfo> UpdateHandler::GetAffectedHierarchyLevels(IConnectionCR connection, bvector<ECInstanceChangeEventSource::ChangedECInstance> const& changes) const
    {
    bset<ECInstanceKey> keys;
    for (ECInstanceChangeEventSource::ChangedECInstance const& change : changes)
        keys.insert(ECInstanceKey(change.GetClass()->GetId(), change.GetInstanceId()));

    return m_nodesCache->GetRelatedHierarchyLevels(connection, keys);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<IUpdateTaskPtr> UpdateHandler::CreateUpdateTasks(UpdateContext& updateContext, IConnectionCR connection, 
    bvector<ECInstanceChangeEventSource::ChangedECInstance> const& changes) const
    {
    bvector<IUpdateTaskPtr> tasks;

    if (nullptr != m_hierarchyUpdater)
        {
        bvector<HierarchyLevelInfo> affectedHierarchies = GetAffectedHierarchyLevels(connection, changes);
        AddTasksForAffectedHierarchies(tasks, updateContext, affectedHierarchies);
        }

    if (nullptr != m_contentCache)
        {
        bvector<SpecificationContentProviderPtr> affectedContentProviders = m_contentCache->GetProviders(connection);
        for (SpecificationContentProviderPtr& provider : affectedContentProviders)
            {
            provider->GetContextR().AdoptToSameConnection(nullptr);
            AddTask(tasks, *m_tasksFactory.CreateContentInvalidationTask(*m_contentCache, updateContext, *provider));
            }
        }

    return tasks;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<IUpdateTaskPtr> UpdateHandler::CreateUpdateTasks(UpdateContext& updateContext, Utf8CP rulesetId, Utf8CP settingId) const
    {
    bvector<IUpdateTaskPtr> tasks;

    if (nullptr != m_hierarchyUpdater)
        {
        bvector<HierarchyLevelInfo> affectedHierarchies = m_nodesCache->GetRelatedHierarchyLevels(rulesetId, settingId);
        AddTasksForAffectedHierarchies(tasks, updateContext, affectedHierarchies);
        }

    if (nullptr != m_contentCache)
        {
        bvector<SpecificationContentProviderPtr> affectedContentProviders = m_contentCache->GetProviders(rulesetId, settingId);
        for (SpecificationContentProviderPtr& provider : affectedContentProviders)
            {
            provider->GetContextR().AdoptToSameConnection(nullptr);
            AddTask(tasks, *m_tasksFactory.CreateContentInvalidationTask(*m_contentCache, updateContext, *provider));
            }
        }

    return tasks;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void UpdateHandler::AddTasksForAffectedHierarchies(bvector<IUpdateTaskPtr>& tasks, UpdateContext& updateContext,
    bvector<HierarchyLevelInfo> const& affectedHierarchies) const
    {
    for (HierarchyLevelInfo const& info : affectedHierarchies)
        {
        IConnectionCP connection = m_connections.GetConnection(info.GetConnectionId().c_str());
        if (nullptr == connection)
            continue;
        AddTask(tasks, *m_tasksFactory.CreateRefreshHierarchyTask(*m_hierarchyUpdater, updateContext, *connection, info));
        }

    AddTask(tasks, *m_tasksFactory.CreateRemapNodeIdsTask(updateContext.GetRemapInfo()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void UpdateHandler::AddTask(bvector<IUpdateTaskPtr>& tasks, IUpdateTask& task) const
    {
    AddTask(tasks, 0, task);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void UpdateHandler::AddTask(bvector<IUpdateTaskPtr>& tasks, size_t startIndex, IUpdateTask& task) const
    {
    uint32_t taskPriority = task.GetPriority();
    auto iter = tasks.begin() + startIndex;
    while (iter != tasks.end() && (*iter).IsValid() && (*iter)->GetPriority() >= taskPriority)
        iter++;

    tasks.insert(iter, &task);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void UpdateHandler::ExecuteTasks(bvector<IUpdateTaskPtr>& tasks) const
    {
    BeMutexHolder lock(m_mutex, BeMutexHolder::Lock::No);
    LoggingHelper::LogMessage(Log::Update, Utf8PrintfString("Total initial update tasks: %u", tasks.size()).c_str());

    if (m_updateRecordsHandler.IsValid())
        {
        lock.lock();
        m_updateRecordsHandler->Start();
        }
    
    RefCountedPtr<PerformanceLogger> _l2 = LoggingHelper::CreatePerformanceLogger(Log::Update, "Executing tasks", NativeLogging::LOG_TRACE);

    size_t i = 0;
    while (i < tasks.size())
        {
        IUpdateTaskPtr task = tasks[i++];
        if (task.IsNull())
            continue;
            
        LoggingHelper::LogMessage(Log::Update, Utf8PrintfString("Executing task: %s", task->GetPrintStr().c_str()).c_str());

        StopWatch timer(nullptr, true);
        bvector<IUpdateTaskPtr> subTasks = task->Perform();

        LoggingHelper::LogMessage(Log::Update, Utf8PrintfString("Task executed in %.2f s. Resulted in %" PRIu64 " sub-tasks.",
            timer.GetCurrentSeconds(), (uint64_t)subTasks.size()).c_str());

        for (IUpdateTaskPtr const& subTask : subTasks)
            {
            if (subTask.IsValid())
                AddTask(tasks, i, *subTask);
            }
        }
    LoggingHelper::LogMessage(Log::Update, Utf8PrintfString("Total executed tasks: %u", i).c_str());

    if (m_updateRecordsHandler.IsValid())
        m_updateRecordsHandler->Finish();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void UpdateHandler::NotifyECInstancesChanged(IConnectionCR connection, bvector<ECInstanceChangeEventSource::ChangedECInstance> const& changes)
    {
    LoggingHelper::LogMessage(Log::Update, Utf8PrintfString("Received ECInstance change event with %u change(s)", changes.size()).c_str());
    RefCountedPtr<PerformanceLogger> _l1 = LoggingHelper::CreatePerformanceLogger(Log::Update, "Updating", NativeLogging::LOG_TRACE);
    s_taskId = 1; // reset task IDs counter

    UpdateContext updateContext;
    bvector<IUpdateTaskPtr> tasks = CreateUpdateTasks(updateContext, connection, changes);
    ExecuteTasks(tasks);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void UpdateHandler::NotifySettingChanged(Utf8CP rulesetId, Utf8CP settingId)
    {
    LoggingHelper::LogMessage(Log::Update, Utf8PrintfString("Setting '%s' changed for ruleset ID '%s'", settingId, rulesetId).c_str());
    RefCountedPtr<PerformanceLogger> _l1 = LoggingHelper::CreatePerformanceLogger(Log::Update, "Updating", NativeLogging::LOG_TRACE);
    s_taskId = 1; // reset task IDs counter

    UpdateContext updateContext;
    bvector<IUpdateTaskPtr> tasks = CreateUpdateTasks(updateContext, rulesetId, settingId);
    ExecuteTasks(tasks);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void UpdateHandler::DoFullUpdate(Utf8CP rulesetId, bool updateHierarchies, bool updateContent) const
    {
    BeMutexHolder lock(m_mutex, BeMutexHolder::Lock::No);
    if (m_updateRecordsHandler.IsValid())
        {
        lock.lock();
        m_updateRecordsHandler->Start();
        }

    int updateTarget = 0;

    if (nullptr != rulesetId)
        {
        ECExpressionsCache& expressionsCache = m_ecexpressionsCacheProvider.Get(rulesetId);
        expressionsCache.Clear();
        }

    if (nullptr != m_nodesCache && updateHierarchies)
        {
        m_nodesCache->Clear(nullptr, rulesetId);
        updateTarget |= (int)FullUpdateRecord::UpdateTarget::Hierarchy;
        }

    if (nullptr != m_contentCache && updateContent)
        {
        m_contentCache->ClearCache(rulesetId);
        updateTarget |= (int)FullUpdateRecord::UpdateTarget::Content;
        }

    if (m_updateRecordsHandler.IsValid())
        {
        if (0 != updateTarget)
            m_updateRecordsHandler->Accept(FullUpdateRecord(rulesetId, (FullUpdateRecord::UpdateTarget)updateTarget));
        m_updateRecordsHandler->Finish();
        }    
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void UpdateHandler::NotifyRulesetDisposed(PresentationRuleSetCR ruleset)
    {
    LoggingHelper::LogMessage(Log::Update, Utf8PrintfString("Requested full update due to ruleset '%s' disposal", ruleset.GetRuleSetId().c_str()).c_str());
    if (nullptr != m_contentCache)
        m_contentCache->ClearCache(ruleset.GetRuleSetId());
    DoFullUpdate(ruleset.GetRuleSetId().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void UpdateHandler::NotifyCategoriesChanged()
    {
    LoggingHelper::LogMessage(Log::Update, "Requested full content update due to CategoriesChanged notification");
    if (nullptr != m_contentCache)
        m_contentCache->ClearCache();
    DoFullUpdate(nullptr, false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodesProviderPtr HierarchyUpdater::CreateProvider(IConnectionCR connection, HierarchyLevelInfo const& info) const
    {
    // create the nodes provider context
    NavNodesProviderContextPtr context = m_contextFactory.Create(connection, info.GetRulesetId().c_str(), 
        info.GetLocale().c_str(), info.GetPhysicalParentNodeId());
    if (context.IsNull())
        return nullptr;

    context->SetUpdateContext(true);

    // create the provider
    JsonNavNodeCPtr parentNode = (nullptr != info.GetPhysicalParentNodeId()) ? m_nodesCache.GetNode(*info.GetPhysicalParentNodeId()) : nullptr;
    return m_nodesProviderFactory.Create(*context, parentNode.get(), ProviderCacheType::None);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void HierarchyUpdater::SynchronizeLists(NavNodesDataSource const& oldDs, size_t& oldIndex, NavNodesDataSource const& newDs, size_t& newIndex) const
    {
    size_t newIndexReset = newIndex;
    while (oldIndex < oldDs.GetSize())
        {
        JsonNavNodeCPtr oldNode = oldDs.GetNode(oldIndex);
        newIndex = newIndexReset;
        bool found = false;
        while (newIndex < newDs.GetSize())
            {
            JsonNavNodePtr newNode = newDs.GetNode(newIndex);
            if (oldNode->GetKey()->IsSimilar(*newNode->GetKey()))
                {
                found = true;
                break;
                }
            ++newIndex;
            }
        if (found)
            break;
        ++oldIndex;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void HierarchyUpdater::CompareDataSources(bvector<IUpdateTaskPtr>& subTasks, UpdateContext& context, 
    NavNodesProviderCR oldProvider, NavNodesProviderR newProvider) const
    {
    NavNodesDataSourcePtr oldDs = NavNodesDataSource::Create(oldProvider);
    NavNodesDataSourcePtr newDs = NavNodesDataSource::Create(newProvider);

    size_t oldIndex = 0;
    size_t newIndex = 0;
    while (oldIndex < oldDs->GetSize() && newIndex < newDs->GetSize())
        {
        size_t oldIndexStart = oldIndex;
        size_t newIndexStart = newIndex;
        SynchronizeLists(*oldDs, oldIndex, *newDs, newIndex);

        // handle removed nodes
        for (size_t i = oldIndexStart; i < oldIndex; ++i)
            {
            NavNodePtr node = oldDs->GetNode(i);
            subTasks.push_back(m_tasksFactory.CreateReportTask(UpdateRecord(*node)));
            context.GetRemovedNodeIds().insert(node->GetNodeId());
            }

        // insert added nodes
        for (size_t i = newIndexStart; i < newIndex; ++i)
            {
            JsonNavNodePtr node = newDs->GetNode(i);
            CustomizeNode(nullptr, *node, newProvider);
            subTasks.push_back(m_tasksFactory.CreateReportTask(UpdateRecord(*node, i)));
            }

        // now the lists are synchronized - iterate over both of them at the same time
        JsonNavNodePtr oldNode, newNode;
        while (oldIndex < oldDs->GetSize() && newIndex < newDs->GetSize() 
            && (oldNode = oldDs->GetNode(oldIndex))->GetKey()->IsSimilar(*(newNode = newDs->GetNode(newIndex))->GetKey()))
            {
            CompareNodes(subTasks, context, *oldNode, newProvider, *newNode);
            ++oldIndex;
            ++newIndex;
            }
        }
    
    // handle removed nodes
    for (size_t i = oldIndex; i < oldDs->GetSize(); ++i)
        {
        NavNodePtr node = oldDs->GetNode(i);
        subTasks.push_back(m_tasksFactory.CreateReportTask(UpdateRecord(*node)));
        context.GetRemovedNodeIds().insert(node->GetNodeId());
        }

    // insert added nodes
    for (size_t i = newIndex; i < newDs->GetSize(); ++i)
        {
        JsonNavNodePtr node = newDs->GetNode(i);
        CustomizeNode(nullptr, *node, newProvider);
        subTasks.push_back(m_tasksFactory.CreateReportTask(UpdateRecord(*node, i)));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void HierarchyUpdater::CompareNodes(bvector<IUpdateTaskPtr>& subTasks, UpdateContext& context, 
    JsonNavNodeCR oldNode, NavNodesProviderCR newProvider, JsonNavNodeR newNode) const
    {
    CustomizeNode(&oldNode, newNode, newProvider);
    
    if (oldNode.GetNodeId() != newNode.GetNodeId())
        {
        context.GetRemapInfo()[oldNode.GetNodeId()] = newNode.GetNodeId();
        }
    
    bvector<JsonChange> changes = NavNodesHelper::GetChanges(oldNode, newNode);
    if (!changes.empty())
        subTasks.push_back(m_tasksFactory.CreateReportTask(UpdateRecord(newNode, std::move(changes))));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void HierarchyUpdater::CheckIfParentNeedsUpdate(bvector<IUpdateTaskPtr>& subTasks, UpdateContext& context, 
    NavNodesProviderCR oldProvider, NavNodesProviderCR newProvider) const
    {
    if (nullptr == newProvider.GetContext().GetPhysicalParentNodeId())
        {
        // no parent data source to update
        return;
        }

    size_t oldNodesCount = oldProvider.GetNodesCount();
    size_t newNodesCount = newProvider.GetNodesCount();    
    if (0 != oldNodesCount && 0 != newNodesCount || oldNodesCount == newNodesCount)
        {
        // update parent data source only if the count of nodes changed from/to 0
        return;
        }

    DataSourceInfo parentDataSource = m_nodesCache.FindDataSource(newProvider.GetContext().GetPhysicalParentNodeId() ? *newProvider.GetContext().GetPhysicalParentNodeId() : 0);
    if (!parentDataSource.IsValid())
        {
        BeAssert(false);
        return;
        }
    HierarchyLevelInfo parentHierarchyLevelInfo = m_nodesCache.FindHierarchyLevel(parentDataSource.GetHierarchyLevelId());
    if (!parentHierarchyLevelInfo.IsValid())
        {
        BeAssert(false);
        return;
        }
    subTasks.push_back(m_tasksFactory.CreateRefreshHierarchyTask(*this, context, newProvider.GetContext().GetConnection(), parentHierarchyLevelInfo));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void HierarchyUpdater::CustomizeNode(JsonNavNodeCP oldNode, JsonNavNodeR nodeToCustomize, NavNodesProviderCR newNodeProvider) const
    {
    bool nodeChanged = false;
    DataSourceRelatedSettingsUpdater updater(newNodeProvider.GetContext(), &nodeToCustomize);

    // if the old node was customized, we have to customize the new one as well;
    // otherwise the comparison is incorrect
    bool shouldCustomize = (nullptr == oldNode || NavNodeExtendedData(*oldNode).IsCustomized());
    if (shouldCustomize && !NavNodeExtendedData(nodeToCustomize).IsCustomized())
        {
        CustomizationHelper::Customize(newNodeProvider.GetContext(), nodeToCustomize, NavNodesHelper::IsCustomNode(nodeToCustomize));
        nodeChanged = true;
        }

    // the same with children
    bool shouldDetermineChildren = (nullptr == oldNode || oldNode->DeterminedChildren());
    if (shouldDetermineChildren && !nodeToCustomize.DeterminedChildren())
        {
        newNodeProvider.DetermineChildren(nodeToCustomize);
        nodeChanged = true;
        }

    // if old node was expanded, we have to expand new node too
    bool isOldNodeExpanded = (nullptr != oldNode && oldNode->IsExpanded());
    if (isOldNodeExpanded && !nodeToCustomize.IsExpanded())
        {
        nodeToCustomize.SetIsExpanded(true);
        nodeChanged = true;
        }

    // notify provider that we updated the node
    if (nodeChanged)
        newNodeProvider.NotifyNodeChanged(nodeToCustomize);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool HierarchyUpdater::IsHierarchyRemoved(UpdateContext const& context, HierarchyLevelInfo const& info) const
    {
    if (nullptr == info.GetPhysicalParentNodeId())
        return false;
    
    return m_nodesCache.HasParentNode(*info.GetPhysicalParentNodeId(), context.GetRemovedNodeIds());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void HierarchyUpdater::Update(bvector<IUpdateTaskPtr>& subTasks, UpdateContext& context, IConnectionCR connection, HierarchyLevelInfo const& oldInfo, HierarchyLevelInfo const& newInfo) const
    {
    if (context.GetHandledHierarchies().end() != context.GetHandledHierarchies().find(oldInfo))
        {
        // no need to update hierarchy levels which are already updated
        LoggingHelper::LogMessage(Log::Update, "Skipping update as this hierarchy level is already updated", NativeLogging::LOG_DEBUG);
        return;
        }
    context.GetHandledHierarchies().insert(oldInfo);
    context.GetHandledHierarchies().insert(newInfo);

    BeMutexHolder lock(m_nodesCache.GetMutex());
    Savepoint txn(connection.GetDb(), "Update");
    BeAssert(txn.IsActive());

    if (IsHierarchyRemoved(context, oldInfo))
        {
        // no need to update this data source since it's already removed
        LoggingHelper::LogMessage(Log::Update, "Skipping update as this hierarchy level is removed", NativeLogging::LOG_DEBUG);
        return;
        }

    NavNodesProviderPtr oldProvider = m_nodesCache.GetCombinedHierarchyLevel(oldInfo, false, false);
    if (oldProvider.IsNull())
        {
        BeAssert(false);
        return;
        }
    // note: need to make sure oldProvider initialized its nodes cache before setting the removal id
    JsonNavNodePtr temp;
    oldProvider->GetNode(temp, 0);

    BeGuid removalId = m_nodesCache.CreateRemovalId(oldInfo);
    LoggingHelper::LogMessage(Log::Update, Utf8PrintfString("Flagged data source for removal with ID %s", removalId.ToString().c_str()).c_str(), NativeLogging::LOG_DEBUG);
 
    NavNodesProviderPtr newProvider = CreateProvider(connection, newInfo);
    if (newProvider.IsValid())
        {
        DisabledFullNodesLoadContext doNotCustomizeOld(*oldProvider);
        DisabledFullNodesLoadContext doNotCustomizeNew(*newProvider);
        if (IsHierarchyExpanded(oldInfo))
            CompareDataSources(subTasks, context, *oldProvider, *newProvider);
        else
            MarkNodesAsRemoved(context, *oldProvider);

        CheckIfParentNeedsUpdate(subTasks, context, *oldProvider, *newProvider);
        }
    subTasks.push_back(m_tasksFactory.CreateRemoveHierarchyLevelTask(removalId));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool HierarchyUpdater::IsHierarchyExpanded(HierarchyLevelInfo const& info) const
    {
    if (nullptr != info.GetPhysicalParentNodeId())
        {
        //Go up the hierarchy and look if all parents are expanded
        NavNodeCPtr parent = m_nodesCache.GetNode(*info.GetPhysicalParentNodeId());
        while (!parent.IsNull())
            {
            if (!parent->IsExpanded())
                return false;

            parent = m_nodesCache.GetNode(parent->GetParentNodeId());
            }
        }
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void HierarchyUpdater::MarkNodesAsRemoved(UpdateContext& context, NavNodesProviderCR oldProvider) const
    {
    NavNodesDataSourcePtr oldDs = NavNodesDataSource::Create(oldProvider);
    size_t size = oldDs->GetSize();
    for (size_t i = 0; i < size; i++)
        {
        NavNodePtr node = oldDs->GetNode(i);
        context.GetRemovedNodeIds().insert(node->GetNodeId());
        }
    }
