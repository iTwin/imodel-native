/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECPresentation/ECPresentationManager.h>
#include "RulesEngineTypes.h"
#include "Hierarchies/NavNodesCache.h"
#include "Content/ContentCache.h"

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
enum TaskPriority
    {
    TASK_PRIORITY_RefreshHierarchy = 1000,
    TASK_PRIORITY_InvalidateContent = 500,
    TASK_PRIORITY_Report = 100,
    TASK_PRIORITY_RefreshSelection = 200,
    };

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct IUpdateTask : RefCountedBase
{
friend struct UpdateHandler;
private:
    bool m_didPerform;
protected:
    virtual bvector<IUpdateTaskPtr> _Perform() = 0;
    virtual uint32_t _GetPriority() const = 0;
    virtual Utf8String _GetPrintStr() const = 0;
    virtual Utf8CP _GetName() const = 0;
public:
    ECPRESENTATION_EXPORT IUpdateTask();
    ECPRESENTATION_EXPORT bvector<IUpdateTaskPtr> Perform();
    Utf8CP GetName() const {return _GetName();}
    Utf8String GetPrintStr() const {return _GetPrintStr();}
    uint32_t GetPriority() const {return _GetPriority();}
    bool DidPerform() const {return m_didPerform;}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct AffectedHierarchyLevelIdentifier
{
private:
    CombinedHierarchyLevelIdentifier m_hierarchyLevelIdentifier;
    NavNodeKeyCPtr m_parentNodeKey;
public:
    AffectedHierarchyLevelIdentifier(CombinedHierarchyLevelIdentifier const& identifier, NavNodeKeyCP parentNodeKey)
        : m_hierarchyLevelIdentifier(identifier), m_parentNodeKey(parentNodeKey)
        {}
    CombinedHierarchyLevelIdentifier const& GetHierarchyLevelIdentifier() const {return m_hierarchyLevelIdentifier;}
    NavNodeKeyCPtr GetParentNodeKey() const { return m_parentNodeKey; }
    bool operator<(AffectedHierarchyLevelIdentifier const& other) const;
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct UpdateContext
{
private:
    std::shared_ptr<NodesCache> m_nodesCache;
    bset<AffectedHierarchyLevelIdentifier> m_handledHierarchies;
    bset<Utf8String> m_reportedContentRulesetIds;
    bmap<Utf8String, std::shared_ptr<UiState>> m_uiState;
    bmap<AffectedHierarchyLevelIdentifier, std::shared_ptr<IHierarchyLevelLocker>> m_hierarchyLocks;
public:
    bset<AffectedHierarchyLevelIdentifier>& GetHandledHierarchies() {return m_handledHierarchies;}
    bset<AffectedHierarchyLevelIdentifier> const& GetHandledHierarchies() const {return m_handledHierarchies;}
    bset<Utf8String>& GetReportedContentRulesetIds() {return m_reportedContentRulesetIds;}
    void SetHierarchyLocks(bmap<AffectedHierarchyLevelIdentifier, std::shared_ptr<IHierarchyLevelLocker>> const& locks) {m_hierarchyLocks = locks;}
    bmap<AffectedHierarchyLevelIdentifier, std::shared_ptr<IHierarchyLevelLocker>>& GetHierarchyLocks() {return m_hierarchyLocks;}
    void SetNodesCache(std::shared_ptr<NodesCache> cache) {m_nodesCache = cache;}
    std::shared_ptr<NodesCache> GetNodesCache() const {return m_nodesCache;}
    void SetUiState(bmap<Utf8String, std::shared_ptr<UiState>> rulesetsUiState) {m_uiState = rulesetsUiState;}
    bmap<Utf8String, std::shared_ptr<UiState>> const& GetUiState() const {return m_uiState;}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct HierarchyUpdater
{
private:
    UpdateTasksFactory const& m_tasksFactory;
    IConnectionCacheCR m_connections;
    INodesProviderContextFactoryCR m_contextFactory;
    INodesProviderFactoryCR m_nodesProviderFactory;

public:
    HierarchyUpdater(UpdateTasksFactory const& tasksFactory, IConnectionCacheCR connections, INodesProviderContextFactoryCR contextFactory, INodesProviderFactoryCR providerFactory)
        : m_tasksFactory(tasksFactory), m_connections(connections), m_contextFactory(contextFactory), m_nodesProviderFactory(providerFactory)
        {}
    void Update(bvector<IUpdateTaskPtr>&, UpdateContext&, AffectedHierarchyLevelIdentifier const&) const;
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct UpdateTasksFactory
{
private:
    mutable IUpdateRecordsHandler* m_recordsHandler;

public:
    UpdateTasksFactory(ContentCache* contentCache, IUpdateRecordsHandler* recordsHandler)
        : m_recordsHandler(recordsHandler)
        {}
    void SetRecordsHandler(IUpdateRecordsHandler* handler) {m_recordsHandler = handler;}

    // hierarchy-related update tasks
    ECPRESENTATION_EXPORT IUpdateTaskPtr CreateRefreshHierarchyTask(HierarchyUpdater const&, UpdateContext&, AffectedHierarchyLevelIdentifier const&) const;

    // content-related update tasks
    ECPRESENTATION_EXPORT IUpdateTaskPtr CreateContentInvalidationTask(ContentCache&, UpdateContext&, ContentProviderR) const;

    // reporting tasks
    ECPRESENTATION_EXPORT IUpdateTaskPtr CreateReportTask(HierarchyUpdateRecord) const;
    ECPRESENTATION_EXPORT IUpdateTaskPtr CreateReportTask(FullUpdateRecord) const;
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct UpdateHandler
{
private:
    INodesCacheManager const& m_nodesCachesManager;
    ContentCache* m_contentCache;
    UpdateTasksFactory m_tasksFactory;
    IECExpressionsCacheProvider& m_ecexpressionsCacheProvider;
    std::unique_ptr<IUpdateRecordsHandler> m_updateRecordsHandler;
    HierarchyUpdater* m_hierarchyUpdater;
    std::shared_ptr<IUiStateProvider> m_uiStateProvider;
    mutable BeMutex m_mutex;

private:
    bvector<IUpdateTaskPtr> CreateUpdateTasks(UpdateContext&, IConnectionCR, bvector<ECInstanceChangeEventSource::ChangedECInstance> const&) const;
    bvector<IUpdateTaskPtr> CreateUpdateTasks(UpdateContext&, Utf8CP rulesetId, Utf8CP settingId) const;
    void AddTasksForAffectedHierarchies(bvector<IUpdateTaskPtr>& tasks, UpdateContext&, bvector<AffectedHierarchyLevelIdentifier> const&) const;
    void AddTask(bvector<IUpdateTaskPtr>& tasks, IUpdateTask& task) const;
    void AddTask(bvector<IUpdateTaskPtr>& tasks, size_t startIndex, IUpdateTask& task) const;
    void ExecuteTasks(bvector<IUpdateTaskPtr>& tasks) const;
    void DoFullUpdate(Utf8CP rulesetId, bool updateHierarchies = true, bool updateContent = true) const;
    bvector<AffectedHierarchyLevelIdentifier> GetAffectedHierarchyLevels(NodesCache&, IConnectionCR, UpdateContext&, bvector<ECInstanceChangeEventSource::ChangedECInstance> const&) const;

public:
    ECPRESENTATION_EXPORT UpdateHandler(INodesCacheManager const&, ContentCache*, IConnectionManagerCR, INodesProviderContextFactoryCR,
        INodesProviderFactoryCR, IECExpressionsCacheProvider&, std::shared_ptr<IUiStateProvider>);
    ECPRESENTATION_EXPORT ~UpdateHandler();
    UpdateTasksFactory const& GetTasksFactory() const {return m_tasksFactory;}
    void SetRecordsHandler(std::unique_ptr<IUpdateRecordsHandler> handler) {m_updateRecordsHandler = std::move(handler); m_tasksFactory.SetRecordsHandler(m_updateRecordsHandler.get());}

    void NotifyRulesetDisposed(PresentationRuleSetCR ruleset);
    void NotifySettingChanged(Utf8CP rulesetId, Utf8CP settingId);
    void NotifyECInstancesChanged(IConnectionCR, bvector<ECInstanceChangeEventSource::ChangedECInstance> const&);
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
