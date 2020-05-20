/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECPresentation/RulesDriven/PresentationManager.h>
#include "RulesEngineTypes.h"
#include "NavNodesCache.h"
#include "ContentCache.h"

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

struct HierarchyUpdater;

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                11/2016
+===============+===============+===============+===============+===============+======*/
enum TaskPriority
    {
    TASK_PRIORITY_RemapNodeIds = 700,
    TASK_PRIORITY_RemoveHierarchyLevel = 500,
    TASK_PRIORITY_RefreshHierarchy = 1000,
    TASK_PRIORITY_InvalidateContent = 500,
    TASK_PRIORITY_Report = 100,
    TASK_PRIORITY_RefreshSelection = 200,
    };

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                02/2016
+===============+===============+===============+===============+===============+======*/
struct IUpdateTask : RefCountedBase
{
friend struct UpdateHandler;
private:
    uint64_t m_id;
    bool m_didPerform;
protected:
    virtual bvector<IUpdateTaskPtr> _Perform() = 0;
    virtual uint32_t _GetPriority() const = 0;
    virtual Utf8String _GetPrintStr() const = 0;
public:
    ECPRESENTATION_EXPORT IUpdateTask();
    ECPRESENTATION_EXPORT bvector<IUpdateTaskPtr> Perform();
    ECPRESENTATION_EXPORT Utf8String GetPrintStr() const;
    uint64_t GetId() const {return m_id;}
    uint32_t GetPriority() const {return _GetPriority();}
    bool DidPerform() const {return m_didPerform;}
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                03/2017
+===============+===============+===============+===============+===============+======*/
struct UpdateContext
{
private:
    bmap<uint64_t, uint64_t> m_remapInfo;
    bset<uint64_t> m_removedNodeIds;
    bset<CombinedHierarchyLevelIdentifier> m_handledHierarchies;
    bset<Utf8String> m_reportedContentRulesetIds;
public:
    bmap<uint64_t, uint64_t>& GetRemapInfo() {return m_remapInfo;}
    bset<uint64_t>& GetRemovedNodeIds() {return m_removedNodeIds;}
    bset<uint64_t> const& GetRemovedNodeIds() const {return m_removedNodeIds;}
    bset<CombinedHierarchyLevelIdentifier>& GetHandledHierarchies() {return m_handledHierarchies;}
    bset<CombinedHierarchyLevelIdentifier> const& GetHandledHierarchies() const {return m_handledHierarchies;}
    bset<Utf8String>& GetReportedContentRulesetIds() {return m_reportedContentRulesetIds;}
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                03/2017
+===============+===============+===============+===============+===============+======*/
struct HierarchyUpdater
{
    struct CompareReporter;

private:
    UpdateTasksFactory const& m_tasksFactory;
    IConnectionCacheCR m_connections;
    INodesCacheManager const& m_nodesCacheManager;
    INodesProviderContextFactoryCR m_contextFactory;
    INodesProviderFactoryCR m_nodesProviderFactory;

private:
    bool IsHierarchyRemoved(UpdateContext const&, NodesCache const&, HierarchyLevelIdentifier const&) const;
    void UpdateParentHierarchy(bvector<IUpdateTaskPtr>&, UpdateContext&, NodesCache const&, NavNodesProviderCR, NavNodesProviderCR) const;

public:
    HierarchyUpdater(UpdateTasksFactory const& tasksFactory, IConnectionCacheCR connections, INodesCacheManager const& nodesCacheManager, INodesProviderContextFactoryCR contextFactory, INodesProviderFactoryCR providerFactory)
        : m_tasksFactory(tasksFactory), m_connections(connections), m_nodesCacheManager(nodesCacheManager), m_contextFactory(contextFactory), m_nodesProviderFactory(providerFactory)
        {}
    void Update(bvector<IUpdateTaskPtr>&, UpdateContext&, HierarchyLevelIdentifier const&, HierarchyLevelIdentifier const&) const;
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                02/2016
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
    ECPRESENTATION_EXPORT IUpdateTaskPtr CreateRemapNodeIdsTask(NodesCache&, bmap<uint64_t, uint64_t> const&) const;
    ECPRESENTATION_EXPORT IUpdateTaskPtr CreateRefreshHierarchyTask(HierarchyUpdater const&, UpdateContext&, HierarchyLevelIdentifier const&) const;
    ECPRESENTATION_EXPORT IUpdateTaskPtr CreateRemoveHierarchyLevelTask(NodesCache&, BeSQLite::BeGuidCR removalId) const;

    // content-related update tasks
    ECPRESENTATION_EXPORT IUpdateTaskPtr CreateContentInvalidationTask(ContentCache&, UpdateContext&, ContentProviderR) const;

    // reporting tasks
    ECPRESENTATION_EXPORT IUpdateTaskPtr CreateReportTask(HierarchyUpdateRecord) const;
    ECPRESENTATION_EXPORT IUpdateTaskPtr CreateReportTask(FullUpdateRecord) const;
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                01/2016
+===============+===============+===============+===============+===============+======*/
struct UpdateHandler
{
private:
    INodesCacheManager const& m_nodesCachesManager;
    ContentCache* m_contentCache;
    IConnectionManagerCR m_connections;
    UpdateTasksFactory m_tasksFactory;
    IECExpressionsCacheProvider& m_ecexpressionsCacheProvider;
    std::unique_ptr<IUpdateRecordsHandler> m_updateRecordsHandler;
    HierarchyUpdater* m_hierarchyUpdater;
    mutable BeMutex m_mutex;

private:
    bvector<IUpdateTaskPtr> CreateUpdateTasks(UpdateContext&, IConnectionCR, bvector<ECInstanceChangeEventSource::ChangedECInstance> const&) const;
    bvector<IUpdateTaskPtr> CreateUpdateTasks(UpdateContext&, Utf8CP rulesetId, Utf8CP settingId) const;
    void AddTasksForAffectedHierarchies(bvector<IUpdateTaskPtr>& tasks, UpdateContext&, bvector<HierarchyLevelIdentifier> const&) const;
    void AddTask(bvector<IUpdateTaskPtr>& tasks, IUpdateTask& task) const;
    void AddTask(bvector<IUpdateTaskPtr>& tasks, size_t startIndex, IUpdateTask& task) const;
    void ExecuteTasks(bvector<IUpdateTaskPtr>& tasks) const;
    void DoFullUpdate(Utf8CP rulesetId, bool updateHierarchies = true, bool updateContent = true) const;
    bvector<HierarchyLevelIdentifier> GetAffectedHierarchyLevels(IConnectionCR, bvector<ECInstanceChangeEventSource::ChangedECInstance> const&) const;

public:
    ECPRESENTATION_EXPORT UpdateHandler(INodesCacheManager const&, ContentCache*, IConnectionManagerCR, INodesProviderContextFactoryCR,
        INodesProviderFactoryCR, IECExpressionsCacheProvider&);
    ECPRESENTATION_EXPORT ~UpdateHandler();
    UpdateTasksFactory const& GetTasksFactory() const {return m_tasksFactory;}
    void SetRecordsHandler(std::unique_ptr<IUpdateRecordsHandler> handler) {m_updateRecordsHandler = std::move(handler); m_tasksFactory.SetRecordsHandler(m_updateRecordsHandler.get());}

    void NotifyRulesetDisposed(PresentationRuleSetCR ruleset);
    void NotifySettingChanged(Utf8CP rulesetId, Utf8CP settingId);
    void NotifyCategoriesChanged();
    void NotifyECInstancesChanged(IConnectionCR, bvector<ECInstanceChangeEventSource::ChangedECInstance> const&);
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
