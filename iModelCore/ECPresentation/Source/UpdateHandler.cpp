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
    return false;
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
protected:
    uint32_t _GetPriority() const override {return TASK_PRIORITY_RefreshHierarchy;}
    bvector<IUpdateTaskPtr> _Perform() override
        {
        bvector<IUpdateTaskPtr> subTasks;
        m_updater.Update(subTasks, m_updateContext, m_identifier);
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
    INodesProviderFactoryCR providerFactory, IECExpressionsCacheProvider& ecexpressionsCacheProvider)
    : m_nodesCachesManager(nodesCachesManager), m_contentCache(contentCache), m_tasksFactory(contentCache, nullptr),
    m_ecexpressionsCacheProvider(ecexpressionsCacheProvider), m_hierarchyUpdater(nullptr)
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
bvector<AffectedHierarchyLevelIdentifier> UpdateHandler::GetAffectedHierarchyLevels(NodesCache& nodesCache, IConnectionCR connection, bvector<ECInstanceChangeEventSource::ChangedECInstance> const& changes) const
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
        bvector<AffectedHierarchyLevelIdentifier> affectedHierarchies = GetAffectedHierarchyLevels(*updateContext.GetNodesCache(), connection, changes);
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
        AddTask(tasks, *m_tasksFactory.CreateRefreshHierarchyTask(*m_hierarchyUpdater, updateContext, info));
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

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct UpdateRecordsScope
{
private:
    BeMutexHolder& m_lock;
    IUpdateRecordsHandler& m_updateRecordsHandler;
public:
    UpdateRecordsScope(BeMutexHolder& lock, IUpdateRecordsHandler& updateRecordsHandler)
        : m_updateRecordsHandler(updateRecordsHandler), m_lock(lock)
        {
        m_lock.lock();
        m_updateRecordsHandler.Start();
        }
    ~UpdateRecordsScope()
        {
        m_updateRecordsHandler.Finish();
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void UpdateHandler::ExecuteTasks(bvector<IUpdateTaskPtr>& tasks) const
    {
    BeMutexHolder lock(m_mutex, BeMutexHolder::Lock::No);

    auto scope = Diagnostics::Scope::Create("Execute update tasks");
    DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Update, LOG_TRACE, Utf8PrintfString("Total initial update tasks: %u", tasks.size()));

    auto updateRecordsScope = m_updateRecordsHandler ? std::make_unique<UpdateRecordsScope>(lock, *m_updateRecordsHandler) : nullptr;

    size_t i = 0;
    while (i < tasks.size())
        {
        IUpdateTaskPtr task = tasks[i++];
        if (task.IsNull())
            continue;

        auto taskScope = Diagnostics::Scope::Create(Utf8PrintfString("Task[%" PRIu64 "]: %s", i, task->GetName()));
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Update, LOG_TRACE, Utf8PrintfString("Task info: %s", task->GetPrintStr().c_str()));

        bvector<IUpdateTaskPtr> subTasks = task->Perform();

        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Update, LOG_TRACE, Utf8PrintfString("Task's execution resulted in %" PRIu64 " sub-tasks", (uint64_t)subTasks.size()));

        for (IUpdateTaskPtr const& subTask : subTasks)
            {
            if (subTask.IsValid())
                AddTask(tasks, i, *subTask);
            }
        }

    DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Update, LOG_TRACE, Utf8PrintfString("Total executed tasks: %" PRIu64, (uint64_t)i));
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
    NavNodesProviderContextPtr CreateProviderContext(Utf8StringCR rulesetId, NavNodeCP parent) const
        {
        NavNodesProviderContextPtr providerContext = m_providerContextFactory.Create(m_connection, rulesetId.c_str(), parent, m_nodesCache, nullptr, RulesetVariables());
        if (providerContext.IsNull())
            return nullptr;

        return providerContext;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    NavNodesProviderPtr CreateProvider(CombinedHierarchyLevelIdentifier const& identifier, std::shared_ptr<IHierarchyLevelLocker> locker, NavNodeCP parent)
        {
        NavNodesProviderContextPtr providerContext = CreateProviderContext(identifier.GetRulesetId(), parent);
        if (providerContext.IsNull())
            DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::HierarchiesUpdate, "Failed to create refreshed nodes provider context");

        providerContext->SetHierarchyLevelLocker(locker);

        NavNodesProviderPtr provider = m_providerFactory.Create(*providerContext);
        return provider->PostProcess(m_providerFactory.GetPostProcessors());
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    NavNodesProviderPtr GetCachedProvider(CombinedHierarchyLevelIdentifier const& identifier, NavNodeCP parent)
        {
        NavNodesProviderContextPtr providerContext = CreateProviderContext(identifier.GetRulesetId(), parent);
        if (providerContext.IsNull())
            DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::HierarchiesUpdate, "Failed to create cached nodes provider context");

        providerContext->SetRemovalId(identifier.GetRemovalId());

        NavNodesProviderPtr provider = m_nodesCache->GetCombinedHierarchyLevel(*providerContext, providerContext->GetHierarchyLevelIdentifier(), false);
        if (provider.IsValid())
            return provider->PostProcess(m_providerFactory.GetPostProcessors());

        return nullptr;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    NavNodeCPtr LocateParentNode(AffectedHierarchyLevelIdentifier const& identifier)
        {
        if (identifier.GetParentNodeKey().IsNull())
            return nullptr;

        return m_nodesCache->LocateNode(m_connection, identifier.GetHierarchyLevelIdentifier().GetRulesetId(), *identifier.GetParentNodeKey());
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    void UpdateParentHierarchy(bool oldProviderHasNodes, NavNodesProviderCR newProvider)
        {
        if (newProvider.GetContext().GetPhysicalParentNode().IsNull())
            return;

        // update parent hierarchy level if nodes count changed from/to 0 or if parent node is a grouping node
        bool newProviderHasNodes = newProvider.HasNodes();
        auto parent = newProvider.GetContext().GetPhysicalParentNode();
        if (oldProviderHasNodes == newProviderHasNodes && !(parent.IsValid() && parent->GetKey()->AsGroupingNodeKey()))
            return;
        
        auto grandParent = parent.IsValid()
            ? m_nodesCache->GetPhysicalParentNode(parent->GetNodeId(), newProvider.GetContext().GetRulesetVariables(), newProvider.GetContext().GetInstanceFilter())
            : nullptr;

        CombinedHierarchyLevelIdentifier parentHierarchyLevelIdentifier = newProvider.GetContext().GetHierarchyLevelIdentifier();
        parentHierarchyLevelIdentifier.SetPhysicalParentNodeId(grandParent.IsValid() ? grandParent->GetNodeId() : BeGuid());

        AffectedHierarchyLevelIdentifier grandparentLevel(parentHierarchyLevelIdentifier, grandParent.IsValid() ? grandParent->GetKey().get() : nullptr);

        newProvider.GetContext().GetHierarchyLevelLocker().Unlock();
        RefreshHierarchyLevel(grandparentLevel);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    void RefreshNavNodesProvider(AffectedHierarchyLevelIdentifier const& identifier, NavNodeCP parentNode)
        {
        if (ContainsAnyAncestorHierarchyLevel(m_context.GetHandledHierarchies(), identifier))
            return;

        // flag the hierarchy level for removal
        CombinedHierarchyLevelIdentifier removedHierarchyLevelIdentifier(identifier.GetHierarchyLevelIdentifier());
        removedHierarchyLevelIdentifier.SetRemovalId(BeGuid(true));
        m_nodesCache->SetRemovalId(removedHierarchyLevelIdentifier);

        // create locker for hierarchy level provider that is going to be created
        auto locker = m_nodesCache->CreateHierarchyLevelLocker(identifier.GetHierarchyLevelIdentifier());

        // get HasNodes flag from old provider
        NavNodesProviderPtr oldProvider = GetCachedProvider(removedHierarchyLevelIdentifier, parentNode);
        if (oldProvider.IsNull())
            return;

        bool oldProviderHasNodes = oldProvider->HasNodes();
        // remove old provider
        m_nodesCache->RemoveHierarchyLevels(removedHierarchyLevelIdentifier.GetRemovalId());

        NavNodePtr parentNodePtr;
        if (parentNode)
            {
            parentNodePtr = parentNode->Clone();
            parentNodePtr->ResetHasChildren();
            }

        // create hierarchy level provider
        NavNodesProviderPtr newProvider = CreateProvider(identifier.GetHierarchyLevelIdentifier(), locker, parentNodePtr.get());
        if (newProvider.IsNull())
            return;

        // update parent hierarchy level if needed
        UpdateParentHierarchy(oldProviderHasNodes, *newProvider);
        }

public:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
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
            DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::HierarchiesUpdate, LOG_TRACE, "Skipping update as this hierarchy level is already updated");
            return;
            }

        m_context.GetHandledHierarchies().insert(identifier);
        NavNodeCPtr parentNode = LocateParentNode(identifier);
        if (identifier.GetParentNodeKey().IsValid() && parentNode.IsNull())
            {
            // this hierarchy level is removed
            return;
            }

        // remove old hierarchy level, update parent hierarchy level if needed
        RefreshNavNodesProvider(identifier, parentNode.get());
        m_subTasks.push_back(m_tasksFactory.CreateReportTask(FullUpdateRecord(identifier.GetHierarchyLevelIdentifier().GetRulesetId(), m_connection.GetECDb().GetDbFileName(), FullUpdateRecord::UpdateTarget::Hierarchy)));
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
