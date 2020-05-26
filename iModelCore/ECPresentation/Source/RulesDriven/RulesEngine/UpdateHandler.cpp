/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include "UpdateHandler.h"
#include "HierarchiesComparer.h"
#include "LoggingHelper.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document HierarchyUpdateRecord::AsJson(rapidjson::Document::AllocatorType* allocator) const
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
    HierarchyLevelIdentifier m_hierarchyLevelIdentifier;
protected:
    virtual uint32_t _GetPriority() const override {return TASK_PRIORITY_RefreshHierarchy;}
    virtual bvector<IUpdateTaskPtr> _Perform() override
        {
        HierarchyLevelIdentifier newInfo(m_hierarchyLevelIdentifier);
        newInfo.Invalidate();
        if (nullptr != m_hierarchyLevelIdentifier.GetPhysicalParentNodeId())
            {
            auto iter = m_updateContext.GetRemapInfo().find(*m_hierarchyLevelIdentifier.GetPhysicalParentNodeId());
            if (m_updateContext.GetRemapInfo().end() != iter)
                newInfo.SetPhysicalParentNodeId(iter->second);
            }
        if (nullptr != m_hierarchyLevelIdentifier.GetVirtualParentNodeId())
            {
            auto iter = m_updateContext.GetRemapInfo().find(*m_hierarchyLevelIdentifier.GetVirtualParentNodeId());
            if (m_updateContext.GetRemapInfo().end() != iter)
                newInfo.SetVirtualParentNodeId(iter->second);
            }
        bvector<IUpdateTaskPtr> subTasks;
        m_updater.Update(subTasks, m_updateContext, m_hierarchyLevelIdentifier, newInfo);
        return subTasks;
        }
    virtual Utf8String _GetPrintStr() const override
        {
        Utf8String str = "[RefreshHierarchyTask]";
        if (!DidPerform())
            {
            str.append(" ConnectionId: ").append(m_hierarchyLevelIdentifier.GetConnectionId());
            str.append(" RulesetId: ").append(m_hierarchyLevelIdentifier.GetRulesetId());
            str.append(" PhysicalParentId: ").append((nullptr == m_hierarchyLevelIdentifier.GetPhysicalParentNodeId()) ? "null" : std::to_string(*m_hierarchyLevelIdentifier.GetPhysicalParentNodeId()).c_str());
            str.append(" VirtualParentId: ").append((nullptr == m_hierarchyLevelIdentifier.GetVirtualParentNodeId()) ? "null" : std::to_string(*m_hierarchyLevelIdentifier.GetVirtualParentNodeId()).c_str());
            }
        return str;
        }
public:
    RefreshHierarchyTask(HierarchyUpdater const& updater, UpdateContext& updateContext, HierarchyLevelIdentifier info)
        : m_updater(updater), m_updateContext(updateContext), m_hierarchyLevelIdentifier(info)
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
        m_contentCache.ClearCache(rulesetId.c_str());
#ifdef wip_ruleset_variables
        if (nullptr == m_provider->GetContentDescriptor())
            m_contentCache.ClearCache(rulesetId.c_str());
        else
            m_provider->InvalidateContent();
#endif

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
    HierarchyUpdateRecord m_record;
private:
    static Utf8String GetString(HierarchyUpdateRecord const& record)
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
                GetString(m_record).c_str(), node->GetType().c_str(), node->GetLabelDefinition().GetDisplayValue().c_str(), node->GetNodeId(), node->GetParentNodeId()));
            }
        return str;
        }
public:
    HierarchyChangeReportTask(IUpdateRecordsHandler& recordsHandler, HierarchyUpdateRecord record)
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
IUpdateTaskPtr UpdateTasksFactory::CreateRemapNodeIdsTask(NodesCache& nodesCache, bmap<uint64_t, uint64_t> const& remapInfo) const
    {
    return new RemapNodeIdsTask(nodesCache, remapInfo);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
IUpdateTaskPtr UpdateTasksFactory::CreateRemoveHierarchyLevelTask(NodesCache& nodesCache, BeGuidCR removalId) const
    {
    return new RemoveHierarchyLevelTask(nodesCache, removalId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
IUpdateTaskPtr UpdateTasksFactory::CreateRefreshHierarchyTask(HierarchyUpdater const& updater, UpdateContext& updateContext,
    HierarchyLevelIdentifier const& info) const
    {
    return new RefreshHierarchyTask(updater, updateContext, info);
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
IUpdateTaskPtr UpdateTasksFactory::CreateReportTask(HierarchyUpdateRecord record) const
    {
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
UpdateHandler::UpdateHandler(INodesCacheManager const& nodesCachesManager, ContentCache* contentCache, IConnectionManagerCR connections, INodesProviderContextFactoryCR contextFactory,
    INodesProviderFactoryCR providerFactory, IECExpressionsCacheProvider& ecexpressionsCacheProvider)
    : m_nodesCachesManager(nodesCachesManager), m_contentCache(contentCache), m_connections(connections), m_tasksFactory(contentCache, nullptr),
    m_ecexpressionsCacheProvider(ecexpressionsCacheProvider), m_hierarchyUpdater(nullptr)
    {
    m_hierarchyUpdater = new HierarchyUpdater(m_tasksFactory, connections, nodesCachesManager, contextFactory, providerFactory);
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
bvector<HierarchyLevelIdentifier> UpdateHandler::GetAffectedHierarchyLevels(IConnectionCR connection, bvector<ECInstanceChangeEventSource::ChangedECInstance> const& changes) const
    {
    NodesCache* nodesCache = m_nodesCachesManager.GetCache(connection.GetId());
    if (nullptr == nodesCache)
        return bvector<HierarchyLevelIdentifier>();

    bset<ECInstanceKey> keys;
    for (ECInstanceChangeEventSource::ChangedECInstance const& change : changes)
        keys.insert(ECInstanceKey(change.GetClass()->GetId(), change.GetInstanceId()));

    return nodesCache->GetRelatedHierarchyLevels(connection, keys);
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
        bvector<HierarchyLevelIdentifier> affectedHierarchies = GetAffectedHierarchyLevels(connection, changes);
        AddTasksForAffectedHierarchies(tasks, updateContext, affectedHierarchies);
        }

    if (nullptr != m_contentCache)
        {
        bvector<SpecificationContentProviderPtr> affectedContentProviders = m_contentCache->GetProviders(connection);
        for (SpecificationContentProviderPtr& provider : affectedContentProviders)
            {
            provider->GetContextR().ShallowAdoptToSameConnection(nullptr);
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
        bvector<HierarchyLevelIdentifier> affectedHierarchies;
        bvector<NodesCache*> nodesCaches = m_nodesCachesManager.GetAllNodeCaches();
        for (auto cache : nodesCaches)
            {
            bvector<HierarchyLevelIdentifier> perCacheAffectedHierarchies = cache->GetRelatedHierarchyLevels(rulesetId, settingId);
            ContainerHelpers::Push(affectedHierarchies, perCacheAffectedHierarchies);
            }
        AddTasksForAffectedHierarchies(tasks, updateContext, affectedHierarchies);
        }

    if (nullptr != m_contentCache)
        {
        bvector<SpecificationContentProviderPtr> affectedContentProviders = m_contentCache->GetProviders(rulesetId, settingId);
        for (SpecificationContentProviderPtr& provider : affectedContentProviders)
            {
            provider->GetContextR().ShallowAdoptToSameConnection(nullptr);
            AddTask(tasks, *m_tasksFactory.CreateContentInvalidationTask(*m_contentCache, updateContext, *provider));
            }
        }

    return tasks;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void UpdateHandler::AddTasksForAffectedHierarchies(bvector<IUpdateTaskPtr>& tasks, UpdateContext& updateContext,
    bvector<HierarchyLevelIdentifier> const& affectedHierarchies) const
    {
    for (HierarchyLevelIdentifier const& info : affectedHierarchies)
        {
        IConnectionCP connection = m_connections.GetConnection(info.GetConnectionId().c_str());
        if (nullptr == connection)
            continue;
        NodesCache* nodesCache = m_nodesCachesManager.GetCache(connection->GetId());
        if (nullptr == nodesCache)
            {
            BeAssert(false);
            continue;
            }
        AddTask(tasks, *m_tasksFactory.CreateRefreshHierarchyTask(*m_hierarchyUpdater, updateContext, info));
        AddTask(tasks, *m_tasksFactory.CreateRemapNodeIdsTask(*nodesCache, updateContext.GetRemapInfo()));
        }
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

    if (m_updateRecordsHandler)
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

    if (m_updateRecordsHandler)
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
    if (m_updateRecordsHandler)
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

    if (updateHierarchies)
        {
        m_nodesCachesManager.ClearCaches(rulesetId);
        updateTarget |= (int)FullUpdateRecord::UpdateTarget::Hierarchy;
        }

    if (nullptr != m_contentCache && updateContent)
        {
        m_contentCache->ClearCache(rulesetId);
        updateTarget |= (int)FullUpdateRecord::UpdateTarget::Content;
        }

    if (m_updateRecordsHandler)
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
* @bsimethod                                    Saulius.Skliutas                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void HierarchyUpdater::UpdateParentHierarchy(bvector<IUpdateTaskPtr>& subTasks, UpdateContext& context,
    NodesCache const& nodesCache, NavNodesProviderCR oldProvider, NavNodesProviderCR newProvider) const
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

    DataSourceInfo parentDataSource = nodesCache.FindDataSource(newProvider.GetContext().GetPhysicalParentNodeId() ? *newProvider.GetContext().GetPhysicalParentNodeId() : 0);
    if (!parentDataSource.GetIdentifier().IsValid())
        {
        BeAssert(false);
        return;
        }
    HierarchyLevelIdentifier parentHierarchyLevelIdentifier = nodesCache.FindHierarchyLevel(parentDataSource.GetIdentifier().GetHierarchyLevelId());
    if (!parentHierarchyLevelIdentifier.IsValid())
        {
        BeAssert(false);
        return;
        }
    subTasks.push_back(m_tasksFactory.CreateRefreshHierarchyTask(*this, context, parentHierarchyLevelIdentifier));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool HierarchyUpdater::IsHierarchyRemoved(UpdateContext const& context, NodesCache const& nodesCache, HierarchyLevelIdentifier const& info) const
    {
    if (nullptr == info.GetPhysicalParentNodeId())
        return false;

    return nodesCache.HasParentNode(*info.GetPhysicalParentNodeId(), context.GetRemovedNodeIds());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static void MarkNodesAsRemoved(UpdateContext& context, NavNodesProviderCR oldProvider)
    {
    NavNodesDataSourcePtr oldDs = NavNodesDataSource::Create(oldProvider);
    size_t size = oldDs->GetSize();
    for (size_t i = 0; i < size; i++)
        {
        NavNodePtr node = oldDs->GetNode(i);
        context.GetRemovedNodeIds().insert(node->GetNodeId());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static bool IsHierarchyExpanded(IHierarchyCacheCR nodesCache, HierarchyLevelIdentifier const& info)
    {
#ifdef not_tracking_node_expansion_in_imodeljs
    if (nullptr != info.GetPhysicalParentNodeId())
        {
        //Go up the hierarchy and look if all parents are expanded
        NavNodeCPtr parent = nodesCache.GetNode(*info.GetPhysicalParentNodeId());
        while (!parent.IsNull())
            {
            if (!parent->IsExpanded())
                return false;

            parent = nodesCache.GetNode(parent->GetParentNodeId());
            }
        }
#endif
    return true;
    }

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                04/2020
+===============+===============+===============+===============+===============+======*/
struct HierarchyUpdater::CompareReporter : IHierarchyChangesReporter
{
private:
    UpdateContext& m_context;
    bvector<IUpdateTaskPtr>& m_tasks;
    HierarchyUpdater const& m_updater;
    NodesCache& m_nodesCache;
    bmap<NavNodesProviderCP, BeGuid> m_removalIds;

protected:
    void _OnFoundLhsProvider(NavNodesProviderCR provider, CombinedHierarchyLevelIdentifier const& hlInfo) override
        {
        // need to make sure the provider initialized its nodes cache before setting the removal id
        JsonNavNodePtr temp;
        provider.GetNode(temp, 0);

        // provider's hierarchy level might be different from the requested hierarchy level
        // that was requested to update due to post-processing - we consider both of these levels handled
        m_context.GetHandledHierarchies().insert(hlInfo);
        m_context.GetHandledHierarchies().insert(provider.GetContext().GetHierarchyLevelIdentifier());

        BeGuid removalId = m_nodesCache.CreateRemovalId(hlInfo);
        m_removalIds.Insert(&provider, removalId);
        LoggingHelper::LogMessage(Log::Update, Utf8PrintfString("Flagged data source for removal with ID %s", removalId.ToString().c_str()).c_str(), NativeLogging::LOG_DEBUG);
        }

    bool _OnStartCompare(NavNodesProviderCR lhs, NavNodesProviderCR rhs) override
        {
        m_context.GetHandledHierarchies().insert(rhs.GetContext().GetHierarchyLevelIdentifier());
        if (!IsHierarchyExpanded(m_nodesCache, lhs.GetContext().GetHierarchyLevelIdentifier()))
            {
            MarkNodesAsRemoved(m_context, lhs);
            return false;
            }
        return true;
        }

    void _OnEndCompare(NavNodesProviderCR lhs, NavNodesProviderCP rhs) override
        {
        if (rhs)
            m_updater.UpdateParentHierarchy(m_tasks, m_context, m_nodesCache, lhs, *rhs);

        auto removalIdIter = m_removalIds.find(&lhs);
        if (m_removalIds.end() == removalIdIter)
            {
            BeAssert(false);
            return;
            }
        m_tasks.push_back(m_updater.m_tasksFactory.CreateRemoveHierarchyLevelTask(m_nodesCache, removalIdIter->second));
        m_removalIds.erase(removalIdIter);
        }

    void _Added(HierarchyLevelIdentifier const& hli, JsonNavNodeCR node, size_t index) override
        {
        m_tasks.push_back(m_updater.m_tasksFactory.CreateReportTask(HierarchyUpdateRecord(hli.GetRulesetId(), node, index)));
        }

    void _Removed(HierarchyLevelIdentifier const& hli, JsonNavNodeCR node) override
        {
        m_tasks.push_back(m_updater.m_tasksFactory.CreateReportTask(HierarchyUpdateRecord(hli.GetRulesetId(), node)));
        m_context.GetRemovedNodeIds().insert(node.GetNodeId());
        }

    void _Changed(HierarchyLevelIdentifier const& hli, JsonNavNodeCR lhsNode, JsonNavNodeCR rhsNode, bvector<JsonChange> const& changes) override
        {
        if (lhsNode.GetNodeId() != rhsNode.GetNodeId())
            m_context.GetRemapInfo()[lhsNode.GetNodeId()] = rhsNode.GetNodeId();
        if (!changes.empty())
            m_tasks.push_back(m_updater.m_tasksFactory.CreateReportTask(HierarchyUpdateRecord(hli.GetRulesetId(), rhsNode, changes)));
        }

public:
    CompareReporter(UpdateContext& context, bvector<IUpdateTaskPtr>& tasks, HierarchyUpdater const& updater, NodesCache& nodesCache)
        : m_context(context), m_tasks(tasks), m_updater(updater), m_nodesCache(nodesCache)
        {}
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void HierarchyUpdater::Update(bvector<IUpdateTaskPtr>& subTasks, UpdateContext& context, HierarchyLevelIdentifier const& oldInfo, HierarchyLevelIdentifier const& newInfo) const
    {
    if (context.GetHandledHierarchies().end() != context.GetHandledHierarchies().find(oldInfo))
        {
        // no need to update hierarchy levels which are already updated
        LoggingHelper::LogMessage(Log::Update, "Skipping update as this hierarchy level is already updated", NativeLogging::LOG_DEBUG);
        return;
        }

    NodesCache* nodesCache = m_nodesCacheManager.GetCache(newInfo.GetConnectionId());
    if (nullptr == nodesCache)
        {
        BeAssert(false);
        return;
        }

    BeMutexHolder lock(nodesCache->GetMutex());

    if (IsHierarchyRemoved(context, *nodesCache, oldInfo))
        {
        // no need to update this data source since it's already removed
        LoggingHelper::LogMessage(Log::Update, "Skipping update as this hierarchy level is removed", NativeLogging::LOG_DEBUG);
        return;
        }

    CompareReporter reporter(context, subTasks, *this, *nodesCache);
    HierarchiesComparer comparer(HierarchiesComparer::Params(*nodesCache, m_contextFactory, m_nodesProviderFactory, reporter, false, std::make_unique<RulesetVariables>()));
    comparer.Compare(m_connections, oldInfo, newInfo);
    }
