/*--------------------------------------------------------------------------------------+
|
|     $Source: Source/RulesDriven/RulesEngine/UpdateHandler.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <ECPresentation/RulesDriven/PresentationManager.h>
#include <ECPresentation/RulesDriven/Rules/SpecificationVisitor.h>
#include "RulesEngineTypes.h"
#include "NavNodesCache.h"
#include "ContentCache.h"
#include "NavNodeProviders.h"
#include "ECSchemaHelper.h"
#include "ECExpressionContextsProvider.h"
#include "LoggingHelper.h"

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
    bset<CombinedHierarchyLevelInfo> m_handledHierarchies;
    bset<Utf8String> m_reportedContentRulesetIds;
public:
    bmap<uint64_t, uint64_t>& GetRemapInfo() {return m_remapInfo;}
    bset<uint64_t>& GetRemovedNodeIds() {return m_removedNodeIds;}
    bset<uint64_t> const& GetRemovedNodeIds() const {return m_removedNodeIds;}
    bset<CombinedHierarchyLevelInfo>& GetHandledHierarchies() {return m_handledHierarchies;}
    bset<CombinedHierarchyLevelInfo> const& GetHandledHierarchies() const {return m_handledHierarchies;}
    bset<Utf8String>& GetReportedContentRulesetIds() {return m_reportedContentRulesetIds;}
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                03/2017
+===============+===============+===============+===============+===============+======*/
struct HierarchyUpdater
{
private:
    UpdateTasksFactory const& m_tasksFactory;
    NodesCache& m_nodesCache;
    INodesProviderContextFactoryCR m_contextFactory;
    INodesProviderFactoryCR m_nodesProviderFactory;

private:
    NavNodesProviderPtr CreateProvider(IConnectionCR, HierarchyLevelInfo const&) const;
    void SynchronizeLists(NavNodesDataSource const& oldDs, size_t& oldIndex, NavNodesDataSource const& newDs, size_t& newIndex) const;
    void CompareDataSources(bvector<IUpdateTaskPtr>&, UpdateContext&, NavNodesProviderCR oldProvider, NavNodesProviderR newProvider) const;
    void CompareNodes(bvector<IUpdateTaskPtr>&, UpdateContext&, JsonNavNodeCR oldNode, NavNodesProviderCR newProvider, JsonNavNodeR newNode) const;
    void CustomizeNode(JsonNavNodeCP oldNode, JsonNavNodeR newNode, NavNodesProviderCR newNodeProvider) const;
    bool IsHierarchyRemoved(UpdateContext const&, HierarchyLevelInfo const&) const;
    bool IsHierarchyExpanded(HierarchyLevelInfo const&) const;
    void CheckIfParentNeedsUpdate(bvector<IUpdateTaskPtr>&, UpdateContext&, NavNodesProviderCR, NavNodesProviderCR) const;
    void MarkNodesAsRemoved(UpdateContext&, NavNodesProviderCR) const;

public:
    HierarchyUpdater(UpdateTasksFactory const& tasksFactory, NodesCache& nodesCache, INodesProviderContextFactoryCR contextFactory, INodesProviderFactoryCR providerFactory)
        : m_tasksFactory(tasksFactory), m_nodesCache(nodesCache), m_contextFactory(contextFactory), m_nodesProviderFactory(providerFactory)
        {}
    void Update(bvector<IUpdateTaskPtr>&, UpdateContext&, IConnectionCR, HierarchyLevelInfo const&, HierarchyLevelInfo const&) const;
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                02/2016
+===============+===============+===============+===============+===============+======*/
struct UpdateTasksFactory
{
private:
    mutable IUpdateRecordsHandler* m_recordsHandler;
    NodesCache* m_nodesCache;
    
public:
    UpdateTasksFactory(NodesCache* nodesCache, ContentCache* contentCache, IUpdateRecordsHandler* recordsHandler) 
        : m_nodesCache(nodesCache), m_recordsHandler(recordsHandler)
        {}
    void SetRecordsHandler(IUpdateRecordsHandler* handler) {m_recordsHandler = handler;}

    // hierarchy-related update tasks
    ECPRESENTATION_EXPORT IUpdateTaskPtr CreateRemapNodeIdsTask(bmap<uint64_t, uint64_t> const&) const;
    ECPRESENTATION_EXPORT IUpdateTaskPtr CreateRefreshHierarchyTask(HierarchyUpdater const&, UpdateContext&, IConnectionCR, HierarchyLevelInfo const&) const;
    ECPRESENTATION_EXPORT IUpdateTaskPtr CreateRemoveHierarchyLevelTask(BeSQLite::BeGuidCR removalId) const;

    // content-related update tasks
    ECPRESENTATION_EXPORT IUpdateTaskPtr CreateContentInvalidationTask(ContentCache&, UpdateContext&, ContentProviderR) const;

    // reporting tasks
    ECPRESENTATION_EXPORT IUpdateTaskPtr CreateReportTask(UpdateRecord) const;
    ECPRESENTATION_EXPORT IUpdateTaskPtr CreateReportTask(FullUpdateRecord) const;
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                01/2016
+===============+===============+===============+===============+===============+======*/
struct UpdateHandler
{
private:
    NodesCache* m_nodesCache;
    ContentCache* m_contentCache;
    IConnectionManagerCR m_connections;
    UpdateTasksFactory m_tasksFactory;
    IECExpressionsCacheProvider& m_ecexpressionsCacheProvider;
    RefCountedPtr<IUpdateRecordsHandler> m_updateRecordsHandler;
    HierarchyUpdater* m_hierarchyUpdater;

private:
    bvector<IUpdateTaskPtr> CreateUpdateTasks(UpdateContext&, IConnectionCR, bvector<ECInstanceChangeEventSource::ChangedECInstance> const&) const;
    bvector<IUpdateTaskPtr> CreateUpdateTasks(UpdateContext&, Utf8CP rulesetId, Utf8CP settingId) const;
    void AddTasksForAffectedHierarchies(bvector<IUpdateTaskPtr>& tasks, UpdateContext&, bvector<HierarchyLevelInfo> const&) const;
    void AddTask(bvector<IUpdateTaskPtr>& tasks, IUpdateTask& task) const;
    void AddTask(bvector<IUpdateTaskPtr>& tasks, size_t startIndex, IUpdateTask& task) const;
    void ExecuteTasks(bvector<IUpdateTaskPtr>& tasks) const;
    void DoFullUpdate(Utf8CP rulesetId, bool updateHierarchies = true, bool updateContent = true) const;
    bvector<HierarchyLevelInfo> GetAffectedHierarchyLevels(IConnectionCR, bvector<ECInstanceChangeEventSource::ChangedECInstance> const&) const;
    
public:
    ECPRESENTATION_EXPORT UpdateHandler(NodesCache*, ContentCache*, IConnectionManagerCR, INodesProviderContextFactoryCR, 
        INodesProviderFactoryCR, IECExpressionsCacheProvider&);
    ECPRESENTATION_EXPORT ~UpdateHandler();
    UpdateTasksFactory const& GetTasksFactory() const {return m_tasksFactory;}
    void SetRecordsHandler(IUpdateRecordsHandler* handler) {m_updateRecordsHandler = handler; m_tasksFactory.SetRecordsHandler(handler);}

    void NotifyRulesetDisposed(PresentationRuleSetCR ruleset);
    void NotifySettingChanged(Utf8CP rulesetId, Utf8CP settingId);
    void NotifyCategoriesChanged();
    void NotifyECInstancesChanged(IConnectionCR, bvector<ECInstanceChangeEventSource::ChangedECInstance> const&);
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
