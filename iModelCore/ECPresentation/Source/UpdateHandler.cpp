/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include "UpdateHandler.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool AffectedHierarchyLevelIdentifier::operator<(AffectedHierarchyLevelIdentifier const& other) const
    {
    STR_LESS_COMPARE(m_hierarchyLevelIdentifier.GetConnectionId().c_str(), other.m_hierarchyLevelIdentifier.GetConnectionId().c_str());
    STR_LESS_COMPARE(m_hierarchyLevelIdentifier.GetRulesetId().c_str(), other.m_hierarchyLevelIdentifier.GetRulesetId().c_str());
    PTR_VALUE_LESS_COMPARE(m_parentNodeKey.get(), other.m_parentNodeKey.get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
IUpdateTask::IUpdateTask() : m_didPerform(false) {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<IUpdateTaskPtr> IUpdateTask::Perform()
    {
    bvector<IUpdateTaskPtr> tasks = _Perform();
    m_didPerform = true;
    return tasks;
    }

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct RefreshHierarchyTask : IUpdateTask
{
private:
    HierarchyUpdater const& m_updater;
    UpdateContext& m_updateContext;
    AffectedHierarchyLevelIdentifier m_identifier;
private:
    void UpdateHierarchyLocks()
        {
        for (auto const& entry : m_updateContext.GetHierarchyLocks())
            entry.second->Lock();
        }
protected:
    uint32_t _GetPriority() const override {return TASK_PRIORITY_RefreshHierarchy;}
    bvector<IUpdateTaskPtr> _Perform() override
        {
        // before performing task update all currently held hierarchy locks to avoid them timeouting
        UpdateHierarchyLocks();
        bvector<IUpdateTaskPtr> subTasks;
        m_updater.Update(subTasks, m_updateContext, m_identifier);

        // remove hierarchy lock for this hierarchy level if it was aquired when creating update tasks
        m_updateContext.GetHierarchyLocks().erase(m_identifier);
        return subTasks;
        }
    Utf8CP _GetName() const override {return "RefreshHierarchyTask";}
    Utf8String _GetPrintStr() const override
        {
        Utf8String str;
        if (!DidPerform())
            {
            str.append("ConnectionId: ").append(m_identifier.GetHierarchyLevelIdentifier().GetConnectionId()).append(",");
            str.append("RulesetId: ").append(m_identifier.GetHierarchyLevelIdentifier().GetRulesetId()).append(",");
            str.append("ParentKey: ").append(!m_identifier.GetParentNodeKey().IsValid() ? "null" : BeRapidJsonUtilities::ToString(m_identifier.GetParentNodeKey()->AsJson()).c_str());
            }
        return str;
        }
public:
    RefreshHierarchyTask(HierarchyUpdater const& updater, UpdateContext& updateContext, AffectedHierarchyLevelIdentifier info)
        : m_updater(updater), m_updateContext(updateContext), m_identifier(info)
        {}
};

/*=================================================================================**//**
* @bsiclass
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
            Utf8String ecdbFileName = m_provider->GetContext().GetConnection().GetECDb().GetDbFileName();
            subTasks.push_back(m_tasksFactory.CreateReportTask(FullUpdateRecord(rulesetId, ecdbFileName, FullUpdateRecord::UpdateTarget::Content)));
            m_updateContext.GetReportedContentRulesetIds().insert(rulesetId);
            }
        return subTasks;
        }
    Utf8CP _GetName() const override {return "InvalidateContentTask";}
    Utf8String _GetPrintStr() const override
        {
        Utf8String str;
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
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct HierarchyChangeReportTask : IUpdateTask
{
private:
    IUpdateRecordsHandler& m_recordsHandler;
    HierarchyUpdateRecord m_record;
protected:
    uint32_t _GetPriority() const override {return TASK_PRIORITY_Report;}
    bvector<IUpdateTaskPtr> _Perform() override {m_recordsHandler.Accept(m_record); return bvector<IUpdateTaskPtr>();}
    Utf8CP _GetName() const override {return "HierarchyChangeReport";}
    Utf8String _GetPrintStr() const override
        {
        Utf8String str;
        if (!DidPerform())
            {
            NavNodeCPtr parentNode = m_record.GetParentNode();
            str.append(Utf8PrintfString("ParentId = %s", parentNode.IsValid() ? parentNode->GetNodeId().ToString().c_str() : BeGuid().ToString().c_str()));
            }
        return str;
        }
public:
    HierarchyChangeReportTask(IUpdateRecordsHandler& recordsHandler, HierarchyUpdateRecord record)
        : m_recordsHandler(recordsHandler), m_record(record)
        {}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct FullUpdateReportTask : IUpdateTask
{
private:
    IUpdateRecordsHandler& m_recordsHandler;
    FullUpdateRecord m_record;
protected:
    uint32_t _GetPriority() const override {return TASK_PRIORITY_Report;}
    bvector<IUpdateTaskPtr> _Perform() override { m_recordsHandler.Accept(m_record); return bvector<IUpdateTaskPtr>(); }
    Utf8CP _GetName() const override {return "FullUpdateReportTask"; }
    Utf8String _GetPrintStr() const override
        {
        return Utf8PrintfString("RulesetId = '%s'", m_record.GetRulesetId().c_str());
        }
public:
    FullUpdateReportTask(IUpdateRecordsHandler& recordsHandler, FullUpdateRecord record)
        : m_recordsHandler(recordsHandler), m_record(record)
        {}
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
IUpdateTaskPtr UpdateTasksFactory::CreateRefreshHierarchyTask(HierarchyUpdater const& updater, UpdateContext& updateContext,
    AffectedHierarchyLevelIdentifier const& info) const
    {
    return new RefreshHierarchyTask(updater, updateContext, info);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
IUpdateTaskPtr UpdateTasksFactory::CreateContentInvalidationTask(ContentCache& contentCache, UpdateContext& updateContext, ContentProviderR provider) const
    {
    return new InvalidateContentTask(*this, updateContext, contentCache, provider);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
IUpdateTaskPtr UpdateTasksFactory::CreateReportTask(HierarchyUpdateRecord record) const
    {
    if (nullptr == m_recordsHandler)
        return nullptr;

    return new HierarchyChangeReportTask(*m_recordsHandler, record);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
IUpdateTaskPtr UpdateTasksFactory::CreateReportTask(FullUpdateRecord record) const
    {
    if (nullptr == m_recordsHandler)
        return nullptr;

    return new FullUpdateReportTask(*m_recordsHandler, record);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
UpdateHandler::UpdateHandler(INodesCacheManager const& nodesCachesManager, ContentCache* contentCache, IConnectionManagerCR connections, INodesProviderContextFactoryCR contextFactory,
    INodesProviderFactoryCR providerFactory, IECExpressionsCacheProvider& ecexpressionsCacheProvider, std::shared_ptr<IUiStateProvider> uiStateProvider)
    : m_nodesCachesManager(nodesCachesManager), m_contentCache(contentCache), m_tasksFactory(contentCache, nullptr),
    m_ecexpressionsCacheProvider(ecexpressionsCacheProvider), m_hierarchyUpdater(nullptr), m_uiStateProvider(uiStateProvider)
    {
    m_hierarchyUpdater = new HierarchyUpdater(m_tasksFactory, connections, contextFactory, providerFactory);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
UpdateHandler::~UpdateHandler()
    {
    DELETE_AND_CLEAR(m_hierarchyUpdater);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool HashPathStartsWith(bvector<Utf8String> const& prefix, bvector<Utf8String> const& path)
    {
    if (prefix.size() >= path.size())
        return false;

    for (int i = 0; i < prefix.size(); ++i)
        {
        if (prefix[i] != path[i])
            return false;
        }
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void InsertAffectedHierarchyIdentifier(bvector<AffectedHierarchyLevelIdentifier>& identifiers, CombinedHierarchyLevelIdentifier const& info, NavNodeCP parentNode)
    {
    NavNodeKeyCPtr parentNodeKey = nullptr != parentNode ? parentNode->GetKey() : nullptr;
    size_t parentHashPathLength = parentNodeKey.IsValid() ? parentNodeKey->GetHashPath().size() : 0;
    auto iter = identifiers.begin();
    while (iter != identifiers.end())
        {
        size_t existingParentHashPathLength = iter->GetParentNodeKey().IsValid() ? iter->GetParentNodeKey()->GetHashPath().size() : 0;
        if (parentHashPathLength < existingParentHashPathLength)
            break;
        iter++;
        }

    identifiers.emplace(iter, info, parentNodeKey.get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool ContainsAnyAncestorHierarchyLevel(bset< AffectedHierarchyLevelIdentifier> const& hierarchyLevels, AffectedHierarchyLevelIdentifier const& currentHierarchyLevel)
    {
    // check if current identifier is a root hierarchy level
    if (currentHierarchyLevel.GetParentNodeKey().IsNull())
        return false;

    return ContainerHelpers::Contains(hierarchyLevels, [&](AffectedHierarchyLevelIdentifier const& ancestorInfo)
        {
        // check if both identifier are from the same hierarchy
        if (currentHierarchyLevel.GetHierarchyLevelIdentifier().GetRulesetId() != ancestorInfo.GetHierarchyLevelIdentifier().GetRulesetId())
            {
            return false;
            }

        // check if there is already a root hierarchy level identifier
        if (ancestorInfo.GetParentNodeKey().IsNull())
            return true;

        // check if existing hierarchy level's parent hash path is shorter than current hierarchy levels' parent hash paths
        if (ancestorInfo.GetParentNodeKey()->GetHashPath().size() >= currentHierarchyLevel.GetParentNodeKey()->GetHashPath().size())
            return false;

        // check if current hierarchy level's parent hash path starts with existing hierarchy level's parent hash path
        return HashPathStartsWith(ancestorInfo.GetParentNodeKey()->GetHashPath(), currentHierarchyLevel.GetParentNodeKey()->GetHashPath());
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bmap<AffectedHierarchyLevelIdentifier, std::shared_ptr<IHierarchyLevelLocker>> CreateHierarchyLocks(bvector<AffectedHierarchyLevelIdentifier> const& identifiers, NodesCache& cache)
    {
    // filter top most common hierarchy levels
    // 'identifiers' should be passed sorted from top most to deepest hierarchy levels
    bset<AffectedHierarchyLevelIdentifier> filteredLevel;
    for (auto const& identifier : identifiers)
        {
        if (ContainsAnyAncestorHierarchyLevel(filteredLevel, identifier))
            continue;

        filteredLevel.insert(identifier);
        }
    return ContainerHelpers::TransformContainer<bmap<AffectedHierarchyLevelIdentifier, std::shared_ptr<IHierarchyLevelLocker>>>(filteredLevel,
        [&](AffectedHierarchyLevelIdentifier const& level)
            {
            std::shared_ptr<IHierarchyLevelLocker> locker = cache.CreateHierarchyLevelLocker(level.GetHierarchyLevelIdentifier());
            // when locking check that all child hierarchy levels are unlocked
            locker->Lock(IHierarchyLevelLocker::LockOptions::CheckLockedChildLevels);
            return make_bpair(level, locker);
            });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<AffectedHierarchyLevelIdentifier> UpdateHandler::GetAffectedHierarchyLevels(NodesCache& nodesCache, IConnectionCR connection, UpdateContext& updateContext, bvector<ECInstanceChangeEventSource::ChangedECInstance> const& changes) const
    {
    // create savepoint to avoid others writing to cache while looking for affected hierarchy levels
    IHierarchyCache::SavepointPtr savepoint = nodesCache.CreateSavepoint();
    bset<ECInstanceKey> keys;
    for (ECInstanceChangeEventSource::ChangedECInstance const& change : changes)
        keys.insert(ECInstanceKey(change.GetClass()->GetId(), change.GetInstanceId()));

    bset<CombinedHierarchyLevelIdentifier> relatedHierarchyLevels = nodesCache.GetRelatedHierarchyLevels(connection, keys);
    bvector<AffectedHierarchyLevelIdentifier> affectedHierarchyIdentifiers;
    for (CombinedHierarchyLevelIdentifier const& hierarchyLevel : relatedHierarchyLevels)
        {
        NavNodePtr parentNode = hierarchyLevel.GetPhysicalParentNodeId().IsValid() ? nodesCache.GetNode(hierarchyLevel.GetPhysicalParentNodeId()) : nullptr;
        InsertAffectedHierarchyIdentifier(affectedHierarchyIdentifiers, hierarchyLevel, parentNode.get());
        }
    savepoint = nullptr;

    // lock all top most common hierarchy levels and put those lockers in update context
    updateContext.SetHierarchyLocks(CreateHierarchyLocks(affectedHierarchyIdentifiers, nodesCache));
    return affectedHierarchyIdentifiers;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<IUpdateTaskPtr> UpdateHandler::CreateUpdateTasks(UpdateContext& updateContext, IConnectionCR connection,
    bvector<ECInstanceChangeEventSource::ChangedECInstance> const& changes) const
    {
    bvector<IUpdateTaskPtr> tasks;
    if (nullptr != m_hierarchyUpdater && nullptr != updateContext.GetNodesCache())
        {
        bvector<AffectedHierarchyLevelIdentifier> affectedHierarchies = GetAffectedHierarchyLevels(*updateContext.GetNodesCache(), connection, updateContext, changes);
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<IUpdateTaskPtr> UpdateHandler::CreateUpdateTasks(UpdateContext& updateContext, Utf8CP rulesetId, Utf8CP settingId) const
    {
    bvector<IUpdateTaskPtr> tasks;

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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void UpdateHandler::AddTasksForAffectedHierarchies(bvector<IUpdateTaskPtr>& tasks, UpdateContext& updateContext,
    bvector<AffectedHierarchyLevelIdentifier> const& affectedHierarchies) const
    {
    for (AffectedHierarchyLevelIdentifier const& info : affectedHierarchies)
        {
        if (!updateContext.HasExpandedNodesSet(info.GetHierarchyLevelIdentifier().GetRulesetId()))
            {
            INavNodeKeysContainerCPtr expandedNodes = m_uiStateProvider ? m_uiStateProvider->GetExpandedNodes(info.GetHierarchyLevelIdentifier().GetRulesetId()) : nullptr;
            updateContext.SetExpandedNodes(info.GetHierarchyLevelIdentifier().GetRulesetId(), expandedNodes);
            }

        AddTask(tasks, *m_tasksFactory.CreateRefreshHierarchyTask(*m_hierarchyUpdater, updateContext, info));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void UpdateHandler::AddTask(bvector<IUpdateTaskPtr>& tasks, IUpdateTask& task) const
    {
    AddTask(tasks, 0, task);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void UpdateHandler::ExecuteTasks(bvector<IUpdateTaskPtr>& tasks) const
    {
    BeMutexHolder lock(m_mutex, BeMutexHolder::Lock::No);

    auto scope = Diagnostics::Scope::Create("Execute update tasks");
    DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Default, LOG_DEBUG, Utf8PrintfString("Total initial update tasks: %u", tasks.size()));

    if (m_updateRecordsHandler)
        {
        lock.lock();
        m_updateRecordsHandler->Start();
        }

    size_t i = 0;
    while (i < tasks.size())
        {
        IUpdateTaskPtr task = tasks[i++];
        if (task.IsNull())
            continue;

        auto taskScope = Diagnostics::Scope::Create(Utf8PrintfString("Task[%" PRIu64 "]: %s", i, task->GetName()));
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Default, LOG_TRACE, Utf8PrintfString("Task info: %s", task->GetPrintStr().c_str()));

        bvector<IUpdateTaskPtr> subTasks = task->Perform();

        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Default, LOG_DEBUG, Utf8PrintfString("Task's execution resulted in %" PRIu64 " sub-tasks", (uint64_t)subTasks.size()));

        for (IUpdateTaskPtr const& subTask : subTasks)
            {
            if (subTask.IsValid())
                AddTask(tasks, i, *subTask);
            }
        }

    DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Default, LOG_DEBUG, Utf8PrintfString("Total executed tasks: %" PRIu64, (uint64_t)i));

    if (m_updateRecordsHandler)
        m_updateRecordsHandler->Finish();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void UpdateHandler::NotifyECInstancesChanged(IConnectionCR connection, bvector<ECInstanceChangeEventSource::ChangedECInstance> const& changes)
    {
    auto scope = Diagnostics::Scope::Create(Utf8PrintfString("ECInstances changed (%" PRIu64 "). Update.", changes.size()));
    UpdateContext updateContext;
    updateContext.SetNodesCache(m_nodesCachesManager.GetPersistentCache(connection.GetId()));
    bvector<IUpdateTaskPtr> tasks = CreateUpdateTasks(updateContext, connection, changes);
    ExecuteTasks(tasks);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void UpdateHandler::NotifySettingChanged(Utf8CP rulesetId, Utf8CP settingId)
    {
    auto scope = Diagnostics::Scope::Create(Utf8PrintfString("Setting '%s' changed for ruleset '%s'. Update.", settingId, rulesetId));
    UpdateContext updateContext;
    bvector<IUpdateTaskPtr> tasks = CreateUpdateTasks(updateContext, rulesetId, settingId);
    ExecuteTasks(tasks);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
            m_updateRecordsHandler->Accept(FullUpdateRecord(rulesetId, Utf8String(), (FullUpdateRecord::UpdateTarget)updateTarget));
        m_updateRecordsHandler->Finish();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void UpdateHandler::NotifyRulesetDisposed(PresentationRuleSetCR ruleset)
    {
    auto scope = Diagnostics::Scope::Create(Utf8PrintfString("Ruleset '%s' disposed. Update.", ruleset.GetRuleSetId().c_str()));
    if (nullptr != m_contentCache)
        m_contentCache->ClearCache(ruleset.GetRuleSetId());
    DoFullUpdate(ruleset.GetRuleSetId().c_str(), true, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static HierarchyLevelIdentifier GetParentHierarchyLevelIdentifier(NodesCache const& nodesCache, BeGuidCR parentId)
    {
    DataSourceInfo parentDataSource = nodesCache.FindDataSource(parentId);
    if (!parentDataSource.GetIdentifier().IsValid())
        return HierarchyLevelIdentifier();

    return nodesCache.FindHierarchyLevel(parentDataSource.GetIdentifier().GetHierarchyLevelId());
    }

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct NodesLocater
{
private:
    IConnectionCR m_connection;
    std::shared_ptr<NodesCache> m_nodesCache;
    INodesProviderFactoryCR m_providerFactory;
    INodesProviderContextFactoryCR m_providerContextFactory;
    std::shared_ptr<IHierarchyLevelLocker> m_hierarchyLock;

private:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    NavNodesProviderContextPtr CreateNodesProviderContext(IConnectionCR connection, Utf8CP rulesetId, NavNodeCP parent)
        {
        return m_providerContextFactory.Create(connection, rulesetId, parent, m_nodesCache, nullptr, RulesetVariables());
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    NavNodesProviderPtr GetProvider(IConnectionCR connection, Utf8StringCR rulesetId, NavNodeCP parent)
        {
        NavNodesProviderContextPtr providerContext = CreateNodesProviderContext(connection, rulesetId.c_str(), parent);
        if (providerContext.IsNull())
            return nullptr;

        return GetProvider(*providerContext);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    NavNodesProviderPtr GetProvider(NavNodesProviderContextR providerContext)
        {
        NavNodesProviderPtr provider = m_nodesCache->GetCombinedHierarchyLevel(providerContext, providerContext.GetHierarchyLevelIdentifier());
        if (provider.IsValid())
            return provider;

        // create locker for the first hierarchy level that was not found in cache and use this locker for all child hierarchy levels
        if (!m_hierarchyLock)
            m_hierarchyLock = m_nodesCache->CreateHierarchyLevelLocker(providerContext.GetHierarchyLevelIdentifier());

        providerContext.SetHierarchyLevelLocker(m_hierarchyLock);
        provider = m_providerFactory.Create(providerContext);
        return provider->PostProcess(m_providerFactory.GetPostProcessors());
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    NavNodePtr LocateNodeInHierarchy(bvector<Utf8String> const& path, int depth, NavNodeCP parentNode, NavNodesProviderContextCR parentContext)
        {
        if (path.size() <= depth)
            return nullptr;

        NavNodesProviderPtr nodesProvider = GetProvider(parentContext.GetConnection(), parentContext.GetRuleset().GetRuleSetId(), parentNode);
        if (nodesProvider.IsNull())
            return nullptr;

        DisabledFullNodesLoadContext disableFullLoad(*nodesProvider);
        NavNodePtr curr;
        bool found = false;
        for (NavNodePtr node : *nodesProvider)
            {
            bvector<Utf8String> const& nodePath = node->GetKey()->GetHashPath();
            if (nodePath.size() <= depth)
                DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Default, "Node hash path shorter than requested depth");

            if (nodePath.size() > path.size())
                break;

            curr = node;
            found = true;
            size_t virtualIndex = depth;
            for (; virtualIndex < nodePath.size() && virtualIndex < path.size(); ++virtualIndex)
                {
                if (!nodePath[virtualIndex].Equals(path[virtualIndex]))
                    {
                    found = false;
                    break;
                    }
                }

            if (found)
                {
                depth = virtualIndex - 1;
                break;
                }
            }

        if (!found)
            return nullptr;

        if (path.size() == depth + 1)
            return NodesFinalizer(nodesProvider->GetContextR()).Finalize(*curr);

        return LocateNodeInHierarchy(path, depth + 1, curr.get(), nodesProvider->GetContext());
        }

public:
    NodesLocater(IConnectionCR connection, std::shared_ptr<NodesCache> nodesCache, INodesProviderFactoryCR providerFactory, INodesProviderContextFactoryCR providerContextFactory)
        : m_connection(connection), m_nodesCache(nodesCache), m_providerFactory(providerFactory), m_providerContextFactory(providerContextFactory)
        {}

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    NavNodeCPtr LocateNode(NavNodeKeyCR nodeKey, Utf8StringCR rulesetId)
        {
        NavNodesProviderContextPtr rootContext = CreateNodesProviderContext(m_connection, rulesetId.c_str(), nullptr);
        if (rootContext.IsNull())
            return nullptr;

        NavNodeCPtr node = m_nodesCache->LocateNode(m_connection, rulesetId, nodeKey, rootContext->GetRulesetVariables());
        if (node.IsValid())
            return node;

        return LocateNodeInHierarchy(nodeKey.GetHashPath(), 0, nullptr, *rootContext);
        }
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct HierarchyRefresher
{
private:
    UpdateContext& m_context;
    std::shared_ptr<NodesCache> m_nodesCache;
    IConnectionCR m_connection;
    bvector<IUpdateTaskPtr>& m_subTasks;
    UpdateTasksFactory const& m_tasksFactory;
    INodesProviderFactoryCR m_providerFactory;
    INodesProviderContextFactoryCR m_providerContextFactory;

private:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    NavNodesProviderPtr CreateProvider(CombinedHierarchyLevelIdentifier const& identifier, NavNodeCP parent)
        {
        NavNodesProviderContextPtr providerContext = m_providerContextFactory.Create(m_connection, identifier.GetRulesetId().c_str(), parent, m_nodesCache, nullptr, RulesetVariables());
        if (providerContext.IsNull())
            return nullptr;

        // create locker for hierarchy level provider that is going to be created
        providerContext->SetHierarchyLevelLocker(m_nodesCache->CreateHierarchyLevelLocker(providerContext->GetHierarchyLevelIdentifier()));

        NavNodesProviderPtr provider = m_providerFactory.Create(*providerContext);
        return provider->PostProcess(m_providerFactory.GetPostProcessors());
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    NavNodesProviderPtr GetCachedProvider(CombinedHierarchyLevelIdentifier const& identifier, NavNodeCP parent)
        {
        NavNodesProviderContextPtr providerContext = m_providerContextFactory.Create(m_connection, identifier.GetRulesetId().c_str(), parent, m_nodesCache, nullptr, RulesetVariables());
        if (providerContext.IsNull())
            return nullptr;

        providerContext->SetRemovalId(m_nodesCache->CreateRemovalId(providerContext->GetHierarchyLevelIdentifier()));

        NavNodesProviderPtr provider = m_nodesCache->GetCombinedHierarchyLevel(*providerContext, providerContext->GetHierarchyLevelIdentifier(), false);
        if (provider.IsValid())
            return provider->PostProcess(m_providerFactory.GetPostProcessors());

        return nullptr;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    NavNodeKeySet GetExpandedChildrenKeys(bvector<Utf8String> const& parentHashPath, Utf8StringCR rulesetId)
        {
        INavNodeKeysContainerCPtr expandedNodes = m_context.GetExpandedNodes(rulesetId);
        if (expandedNodes.IsNull() || expandedNodes->empty())
            return NavNodeKeySet();

        NavNodeKeyList matchingKeys;
        size_t shortestDiff = SIZE_MAX;
        for (NavNodeKeyCPtr key : *expandedNodes)
            {
            if (!HashPathStartsWith(parentHashPath, key->GetHashPath()))
                continue;

            size_t sizeDifference = key->GetHashPath().size() - parentHashPath.size();
            if (sizeDifference < shortestDiff)
                shortestDiff = sizeDifference;
            matchingKeys.push_back(key);
            }

        NavNodeKeySet expandedChildNodes;
        for (NavNodeKeyCPtr key : matchingKeys)
            {
            if (key->GetHashPath().size() - parentHashPath.size() == shortestDiff)
                expandedChildNodes.insert(key);
            }
        return expandedChildNodes;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    bvector<HierarchyUpdateRecord::ExpandedNode> CollectExpandedChildNodes(NavNodesProviderCR provider, NavNodeKeySetCR expandedNodeKeys)
        {
        bvector<HierarchyUpdateRecord::ExpandedNode> expandedNodes;
        size_t foundChildrenCount = 0;
        size_t position = 0;

        DisabledFullNodesLoadContext disableFullLoad(provider);
        for (NavNodePtr node : provider)
            {
            // all expanded nodes were found no need to look any more
            if (foundChildrenCount == expandedNodeKeys.size())
                break;
            // this node is not expanded skip it
            if (expandedNodeKeys.end() == expandedNodeKeys.find(node->GetKey()))
                {
                position++;
                continue;
                }
            // expanded node was found.
            foundChildrenCount++;

            NodesFinalizer(provider.GetContextR()).Finalize(*node);
            expandedNodes.push_back(HierarchyUpdateRecord::ExpandedNode(*node, position));
            position++;
            }

        return expandedNodes;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    void UpdateParentHierarchy(bool oldProviderHasNodes, NavNodesProviderCR newProvider)
        {
        if (newProvider.GetContext().GetPhysicalParentNode().IsNull())
            return;

        // update parent hierarchy level if nodes count changed from/to 0
        bool newProviderHasNodes = newProvider.HasNodes();
        if (oldProviderHasNodes == newProviderHasNodes)
            return;

        HierarchyLevelIdentifier parentHierarchyLevelIdentifier = GetParentHierarchyLevelIdentifier(*m_nodesCache, newProvider.GetContext().GetPhysicalParentNode()->GetNodeId());
        if (!parentHierarchyLevelIdentifier.IsValid())
            {
            DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::HierarchiesUpdate, Utf8PrintfString("Failed to find parent hierarchy level for node %s",
                BeRapidJsonUtilities::ToString(newProvider.GetContext().GetPhysicalParentNode()->GetKey()->AsJson()).c_str()));
            }

        NavNodePtr grandParent = parentHierarchyLevelIdentifier.GetPhysicalParentNodeId().IsValid() ? m_nodesCache->GetNode(parentHierarchyLevelIdentifier.GetPhysicalParentNodeId()) : nullptr;
        NavNodeKeyCPtr grandParentKey = grandParent.IsValid() ? grandParent->GetKey().get() : nullptr;
        AffectedHierarchyLevelIdentifier grandparentLevel(parentHierarchyLevelIdentifier.GetCombined(), grandParentKey.get());

        std::shared_ptr<IHierarchyLevelLocker> grandParentHierarchyLock = m_nodesCache->CreateHierarchyLevelLocker(grandparentLevel.GetHierarchyLevelIdentifier(), true);
        RefreshHierarchyLevel(grandparentLevel);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    NavNodeCPtr LocateParentNode(AffectedHierarchyLevelIdentifier const& identifier)
        {
        if (identifier.GetParentNodeKey().IsNull())
            return nullptr;
        CombinedHierarchyLevelIdentifier const& levelIdentifier = identifier.GetHierarchyLevelIdentifier();
        return NodesLocater(m_connection, m_nodesCache, m_providerFactory, m_providerContextFactory).LocateNode(*identifier.GetParentNodeKey(), levelIdentifier.GetRulesetId());
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    NavNodesProviderPtr RefreshNavNodesProvider(AffectedHierarchyLevelIdentifier const& identifier, NavNodeCP parentNode)
        {
        // no need to remove old provider if any acestor was already handled
        if (ContainsAnyAncestorHierarchyLevel(m_context.GetHandledHierarchies(), identifier))
            return CreateProvider(identifier.GetHierarchyLevelIdentifier(), parentNode);

        // get HasNodes flag from old provider and remove old hierarchy level
        NavNodesProviderPtr oldProvider = GetCachedProvider(identifier.GetHierarchyLevelIdentifier(), parentNode);
        if (oldProvider.IsNull())
            return nullptr;

        bool oldProviderHasNodes = oldProvider->HasNodes();
        m_nodesCache->RemoveHierarchyLevel(oldProvider->GetContext().GetHierarchyLevelIdentifier().GetRemovalId());

        NavNodePtr parentNodePtr;
        if (parentNode)
            {
            parentNodePtr = parentNode->Clone();
            parentNodePtr->ResetHasChildren();
            }

        // create hierarchy level provider
        NavNodesProviderPtr newProvider = CreateProvider(identifier.GetHierarchyLevelIdentifier(), parentNodePtr.get());
        if (newProvider.IsNull())
            return nullptr;

        // update parent hierarchy level if needed
        UpdateParentHierarchy(oldProviderHasNodes, *newProvider);
        return newProvider;
        }

public:
    HierarchyRefresher(UpdateContext& context, std::shared_ptr<NodesCache> nodesCache, IConnectionCR connection, INodesProviderFactoryCR providerFactory, INodesProviderContextFactoryCR contextFactory,
        bvector<IUpdateTaskPtr>& subTasks, UpdateTasksFactory const& tasksFactory)
        : m_context(context), m_nodesCache(nodesCache), m_connection(connection), m_providerFactory(providerFactory), m_providerContextFactory(contextFactory), m_subTasks(subTasks),
        m_tasksFactory(tasksFactory)
        {}

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    void RefreshHierarchyLevel(AffectedHierarchyLevelIdentifier const& identifier)
        {
        if (m_context.GetHandledHierarchies().end() != m_context.GetHandledHierarchies().find(identifier))
            {
            // no need to update hierarchy levels which are already updated
            DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::HierarchiesUpdate, LOG_DEBUG, "Skipping update as this hierarchy level is already updated");
            return;
            }

        m_context.GetHandledHierarchies().insert(identifier);
        NavNodeCPtr parentNode = LocateParentNode(identifier);
        if (identifier.GetParentNodeKey().IsValid() && parentNode.IsNull())
            {
            // this hierarchy level is removed
            return;
            }

        // remove old hierarchy level, update parent hierarchy level if needed and create new hierarchy level
        NavNodesProviderPtr newProvider = RefreshNavNodesProvider(identifier, parentNode.get());
        if (newProvider.IsNull())
            return;

        parentNode = LocateParentNode(identifier);
        if (identifier.GetParentNodeKey().IsValid() && parentNode.IsNull())
            {
            // parent of this hierarchy level was removed while updating it's hierarchy level
            return;
            }

        INavNodeKeysContainerCPtr expandedNodes = m_context.GetExpandedNodes(identifier.GetHierarchyLevelIdentifier().GetRulesetId());
        if (expandedNodes.IsValid() && identifier.GetParentNodeKey().IsValid() && expandedNodes->end() == expandedNodes->find(identifier.GetParentNodeKey()))
            {
            // no need to update as parent of this hierarchy level is collapsed
            return;
            }

        size_t nodesCount = newProvider->GetNodesCount();
        NavNodeKeySet expandedChildren = GetExpandedChildrenKeys(identifier.GetParentNodeKey().IsValid() ? identifier.GetParentNodeKey()->GetHashPath() : bvector<Utf8String>(), identifier.GetHierarchyLevelIdentifier().GetRulesetId());
        if (expandedChildren.empty())
            {
            m_subTasks.push_back(m_tasksFactory.CreateReportTask(HierarchyUpdateRecord(identifier.GetHierarchyLevelIdentifier().GetRulesetId(), newProvider->GetContext().GetConnection().GetECDb().GetDbFileName(), parentNode.get(), nodesCount)));
            return;
            }

        bvector<HierarchyUpdateRecord::ExpandedNode> expandedChildNodes = CollectExpandedChildNodes(*newProvider, expandedChildren);
        m_subTasks.push_back(m_tasksFactory.CreateReportTask(HierarchyUpdateRecord(identifier.GetHierarchyLevelIdentifier().GetRulesetId(), newProvider->GetContext().GetConnection().GetECDb().GetDbFileName(), parentNode.get(), nodesCount, expandedChildNodes)));
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void HierarchyUpdater::Update(bvector<IUpdateTaskPtr>& subTasks, UpdateContext& context, AffectedHierarchyLevelIdentifier const& info) const
    {
    Utf8StringCR connectionId = info.GetHierarchyLevelIdentifier().GetConnectionId();
    std::shared_ptr<NodesCache> nodesCache = context.GetNodesCache();
    if (nullptr == nodesCache)
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::HierarchiesUpdate, Utf8PrintfString("Failed to get hierarchy cache for connection '%s'", connectionId.c_str()));

    IConnectionPtr connection = m_connections.GetConnection(connectionId.c_str());
    if (connection.IsNull())
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::HierarchiesUpdate, Utf8PrintfString("Failed to get connection '%s'", connectionId.c_str()));

    HierarchyRefresher refresher(context, nodesCache, *connection, m_nodesProviderFactory, m_contextFactory, subTasks, m_tasksFactory);
    refresher.RefreshHierarchyLevel(info);
    }
