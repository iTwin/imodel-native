/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include <ECPresentation/ECPresentationManager.h>
#include <ECPresentation/Rules/SpecificationVisitor.h>
#include "../Shared/Queries/CustomFunctions.h"
#include "../Shared/Queries/QueryBuilderHelpers.h"
#include "../Shared/ECExpressions/ECExpressionContextsProvider.h"
#include "../Shared/ExtendedData.h"
#include "../Shared/ECSchemaHelper.h"
#include "NavigationQueryBuilder.h"
#include "NavNodeProviders.h"
#include "NavNodesDataSource.h"
#include "NavNodesCache.h"
#include "NavNodesHelper.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void UsedClassesHelper::NotifyListenerWithUsedClasses(IUsedClassesListener& listener, ECSchemaHelper const& schemaHelper, Utf8StringCR ecexpression)
    {
    bvector<Utf8String> usedClassNames = ECExpressionsHelper(schemaHelper.GetECExpressionsCache()).GetUsedClasses(ecexpression);
    for (Utf8StringCR usedClassName : usedClassNames)
        {
        Utf8String schemaName, className;
        if (ECObjectsStatus::Success != ECClass::ParseClassName(schemaName, className, usedClassName))
            {
            DIAGNOSTICS_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, LOG_ERROR, Utf8PrintfString("Failed to parse ECClass name: '%s'", usedClassName.c_str()));
            continue;
            }
        if (!schemaName.empty())
            {
            ECClassCP usedClass = schemaHelper.GetECClass(schemaName.c_str(), className.c_str());
            if (nullptr == usedClass)
                {
                DIAGNOSTICS_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, LOG_ERROR, Utf8PrintfString("Requested ECClass does not exist: '%s:%s'", schemaName.c_str(), className.c_str()));
                continue;
                }
            listener._OnClassUsed(*usedClass, true);
            }
        else
            {
            bvector<ECClassCP> usedClasses = schemaHelper.GetECClassesByName(className.c_str());
            for (ECClassCP usedClass : usedClasses)
                listener._OnClassUsed(*usedClass, true);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T>
static void NotifyListenerWithCustomizationRuleClasses(IUsedClassesListener& listener, ECSchemaHelper const& schemaHelper, bvector<T const*> const& rules)
    {
    for (T const* rule : rules)
        UsedClassesHelper::NotifyListenerWithUsedClasses(listener, schemaHelper, rule->GetCondition());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void UsedClassesHelper::NotifyListenerWithRulesetClasses(IUsedClassesListener& listener, ECSchemaHelper const& schemaHelper, IRulesPreprocessorR rules)
    {
    for (auto rule : rules.GetInstanceLabelOverrides())
        {
        ECClassCP ruleClass = schemaHelper.GetECClass(rule->GetClassName().c_str());
        if (!ruleClass)
            {
            DIAGNOSTICS_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, LOG_ERROR, Utf8PrintfString("Requested ECClass does not exist: '%s'", rule->GetClassName().c_str()));
            continue;
            }
        listener._OnClassUsed(*ruleClass, true);
        }

    NotifyListenerWithCustomizationRuleClasses(listener, schemaHelper, rules.GetGroupingRules());
    NotifyListenerWithCustomizationRuleClasses(listener, schemaHelper, rules.GetSortingRules());
    NotifyListenerWithCustomizationRuleClasses(listener, schemaHelper, rules.GetLabelOverrides());
    NotifyListenerWithCustomizationRuleClasses(listener, schemaHelper, rules.GetImageIdOverrides());
    NotifyListenerWithCustomizationRuleClasses(listener, schemaHelper, rules.GetStyleOverrides());
    NotifyListenerWithCustomizationRuleClasses(listener, schemaHelper, rules.GetCheckboxRules());
    NotifyListenerWithCustomizationRuleClasses(listener, schemaHelper, rules.GetExtendedDataRules());
    NotifyListenerWithCustomizationRuleClasses(listener, schemaHelper, rules.GetNodeArtifactRules());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void UsedClassesHelper::NotifyListenerWithUsedClasses(IECDbUsedClassesListener& listener, ECExpressionsCache& ecexpressionsCache, IConnectionCR connection, Utf8StringCR ecexpression)
    {
    ECSchemaHelper schemaHelper(connection, nullptr, &ecexpressionsCache);
    ECDbUsedClassesListenerWrapper wrapper(connection, listener);
    NotifyListenerWithUsedClasses(wrapper, schemaHelper, ecexpression);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void UsedClassesHelper::NotifyListenerWithRulesetClasses(IECDbUsedClassesListener& listener, ECExpressionsCache& ecexpressionsCache, IConnectionCR connection, IRulesPreprocessorR rules)
    {
    ECSchemaHelper schemaHelper(connection, nullptr, &ecexpressionsCache);
    ECDbUsedClassesListenerWrapper wrapper(connection, listener);
    NotifyListenerWithRulesetClasses(wrapper, schemaHelper, rules);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool ReturnsInstanceNodes(PresentationQueryBuilderCR query)
    {
    return query.GetNavigationResultParameters().GetResultType() == NavigationQueryResultType::ECInstanceNodes
        || query.GetNavigationResultParameters().GetResultType() == NavigationQueryResultType::MultiECInstanceNodes;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename SpecificationType>
static Utf8StringCR GetSupportedSchemas(SpecificationType const& specification, PresentationRuleSetCR ruleset)
    {
    if (!specification.GetSupportedSchemas().empty())
        return specification.GetSupportedSchemas();
    return ruleset.GetSupportedSchemas();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
NavigationQueryBuilder::NavigationQueryBuilder(NavigationQueryBuilderParameters params)
    : m_params(params)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
NavigationQueryBuilder::~NavigationQueryBuilder()
    {
    }

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct NavigationQueryBuilder::SpecificationsVisitor : PresentationRuleSpecificationVisitor
{
private:
    NavigationQueryBuilder const& m_queryBuilder;
    NavNodeCP m_parentNode;
    ChildNodeRuleCR m_rule;
    bvector<PresentationQueryBuilderPtr> m_queries;

protected:
    void _Visit(AllInstanceNodesSpecification const& specification) override
        {
        m_queries = m_queryBuilder.GetQueries(m_parentNode, specification, m_rule);
        }
    void _Visit(AllRelatedInstanceNodesSpecification const& specification) override
        {
        RelatedInstanceNodesSpecification relatedInstanceNodesSpecification(specification.GetPriority(), specification.GetHasChildren(),
            specification.GetHideNodesInHierarchy(), specification.GetHideIfNoChildren(), specification.GetGroupByClass(),
            specification.GetGroupByLabel(), specification.GetSkipRelatedLevel(), "", specification.GetRequiredRelationDirection(),
            GetSupportedSchemas(specification, m_queryBuilder.GetParameters().GetRuleset()), "", "");
        m_queries = m_queryBuilder.GetQueries(m_parentNode, relatedInstanceNodesSpecification, specification.GetHash(), m_rule);
        for (auto const& query : m_queries)
            query->GetNavigationResultParameters().SetSpecification(&specification);
        }
    void _Visit(RelatedInstanceNodesSpecification const& specification) override
        {
        m_queries = m_queryBuilder.GetQueries(m_parentNode, specification, specification.GetHash(), m_rule);
        }
    void _Visit(InstanceNodesOfSpecificClassesSpecification const& specification) override
        {
        m_queries = m_queryBuilder.GetQueries(m_parentNode, specification, m_rule);
        }
    void _Visit(SearchResultInstanceNodesSpecification const& specification) override
        {
        m_queries = m_queryBuilder.GetQueries(m_parentNode, specification, m_rule);
        }

public:
    SpecificationsVisitor(NavigationQueryBuilder const& queryBuilder, RootNodeRuleCR rule)
        : m_queryBuilder(queryBuilder), m_rule(rule), m_parentNode(nullptr)
        {}
    SpecificationsVisitor(NavigationQueryBuilder const& queryBuilder, ChildNodeRuleCR rule, NavNodeCR parentNode)
        : m_queryBuilder(queryBuilder), m_rule(rule), m_parentNode(&parentNode)
        {}
    bvector<PresentationQueryBuilderPtr> const& GetQueries() const {return m_queries;}
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<PresentationQueryBuilderPtr> NavigationQueryBuilder::GetQueries(ChildNodeRuleCR rule, ChildNodeSpecificationCR spec, NavNodeCR parentNode) const
    {
    SpecificationsVisitor visitor(*this, rule, parentNode);
    spec.Accept(visitor);
    return visitor.GetQueries();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<PresentationQueryBuilderPtr> NavigationQueryBuilder::GetQueries(RootNodeRuleCR rule, ChildNodeSpecificationCR spec) const
    {
    SpecificationsVisitor visitor(*this, rule);
    spec.Accept(visitor);
    return visitor.GetQueries();
    }

struct GroupingHandler;
/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct GroupingNodeAndHandler
{
private:
    NavNodeCPtr m_node;
    GroupingHandler const* m_handler;
public:
    GroupingNodeAndHandler() : m_node(nullptr), m_handler(nullptr) {}
    GroupingNodeAndHandler(NavNodeCR node, GroupingHandler const& handler) : m_node(&node), m_handler(&handler) {}
    NavNodeCR GetNode() const { return *m_node; }
    GroupingHandler const& GetHandler() const { return *m_handler; }
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct SelectQueryInfo
{
private:
    SelectClassWithExcludes<ECClass> m_selectClass;
    RelatedClassPath m_pathFromParentToSelectClass;
    bvector<ECInstanceId> m_parentInstanceIds;
    bvector<RelatedClassPath> m_pathsFromSelectClassToRelatedInstanceClasses;
    bvector<InstanceLabelOverrideValueSpecification const*> m_labelOverrideValueSpecs;
    bvector<InstanceFilterDefinitionCP> m_instanceFilterDefinitions;
    QueryClauseAndBindings m_instanceFilterECSqlExpression;
    std::function<NavigationQueryContractPtr(SelectQueryInfo const&, bvector<GroupingNodeAndHandler> const&)> m_forcedGroupingContractFactory;

public:
    SelectQueryInfo() {}
    SelectQueryInfo(SelectClassWithExcludes<ECClass> selectClass, bvector<RelatedClassPath> relatedInstancePaths = bvector<RelatedClassPath>())
        : m_selectClass(selectClass), m_pathsFromSelectClassToRelatedInstanceClasses(relatedInstancePaths)
        {}

    SelectClassWithExcludes<ECClass> const& GetSelectClass() const {return m_selectClass;}
    SelectClassWithExcludes<ECClass>& GetSelectClass() {return m_selectClass;}
    void SetSelectClass(SelectClassWithExcludes<ECClass> selectClass) {m_selectClass = selectClass;}

    void SetInstanceFilterDefinitions(bvector<InstanceFilterDefinitionCP> value) {m_instanceFilterDefinitions = value;}
    bvector<InstanceFilterDefinitionCP> const& GetInstanceFilterDefinitions() const {return m_instanceFilterDefinitions;}

    void SetInstanceFilterECSqlExpression(QueryClauseAndBindings value) {m_instanceFilterECSqlExpression = value;}
    QueryClauseAndBindings const& GetInstanceFilterECSqlExpression() const {return m_instanceFilterECSqlExpression;}

    RelatedClassPath const& GetPathFromParentToSelectClass() const {return m_pathFromParentToSelectClass;}
    RelatedClassPath& GetPathFromParentToSelectClass() {return m_pathFromParentToSelectClass;}

    void SetParentInstanceIds(bvector<ECInstanceId> ids) {m_parentInstanceIds = ids;}
    bvector<ECInstanceId> const& GetParentInstanceIds() const {return m_parentInstanceIds;}

    bvector<RelatedClassPath> const& GetRelatedInstancePaths() const {return m_pathsFromSelectClassToRelatedInstanceClasses;}
    bvector<RelatedClassPath>& GetRelatedInstancePaths() {return m_pathsFromSelectClassToRelatedInstanceClasses;}

    bvector<ECClassCP> CreateSelectClassList() const
        {
        bvector<ECClassCP> list;
        list.push_back(&m_selectClass.GetClass());
        for (RelatedClassCR relatedPathClass : m_pathFromParentToSelectClass)
            {
            if (relatedPathClass.GetRelationship().IsValid())
                list.push_back(&relatedPathClass.GetRelationship().GetClass());
            }
        for (RelatedClassPathCR relatedInstancePath : m_pathsFromSelectClassToRelatedInstanceClasses)
            {
            if (!relatedInstancePath.empty())
                list.push_back(&relatedInstancePath.back().GetTargetClass().GetClass());
            }
        return list;
        }

    bvector<InstanceLabelOverrideValueSpecification const*> const& GetLabelOverrideValueSpecs() const {return m_labelOverrideValueSpecs;}
    void SetLabelOverrideValueSpecs(bvector<InstanceLabelOverrideValueSpecification const*> const& overrides) {m_labelOverrideValueSpecs = overrides;}

    std::function<NavigationQueryContractPtr(SelectQueryInfo const&, bvector<GroupingNodeAndHandler> const&)> GetForcedGroupingContractFactory() const {return m_forcedGroupingContractFactory;}
    void SetForcedGroupingContractFactory(std::function<NavigationQueryContractPtr(SelectQueryInfo const&, bvector<GroupingNodeAndHandler> const&)> value) {m_forcedGroupingContractFactory = value;}
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
template<class TClass>
static void OnSelected(SelectClass<TClass> const& selectClass, IUsedClassesListener& listener)
    {
    listener._OnClassUsed(selectClass.GetClass(), selectClass.IsSelectPolymorphic());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void OnSelected(SelectClass<ECClass> const& selectClass, NavigationQueryBuilderParameters const& params)
    {
    if (nullptr != params.GetUsedClassesListener())
        OnSelected(selectClass, *params.GetUsedClassesListener());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void OnSelected(SelectQueryInfo const& selectInfo, NavigationQueryBuilderParameters const& params)
    {
    if (nullptr == params.GetUsedClassesListener())
        return;

    OnSelected(selectInfo.GetSelectClass(), *params.GetUsedClassesListener());
    for (RelatedClass const& related : selectInfo.GetPathFromParentToSelectClass())
        {
        OnSelected(SelectClass<ECClass>(*related.GetSourceClass(), "", true), *params.GetUsedClassesListener());
        OnSelected(related.GetTargetClass(), *params.GetUsedClassesListener());
        if (related.GetRelationship().IsValid())
            OnSelected(related.GetRelationship(), *params.GetUsedClassesListener());
        }

    for (RelatedClassPathCR relatedPath : selectInfo.GetRelatedInstancePaths())
        {
        for (RelatedClassCR related : relatedPath)
            {
            OnSelected(related.GetTargetClass(), *params.GetUsedClassesListener());
            if (related.GetRelationship().IsValid())
                OnSelected(related.GetRelationship(), *params.GetUsedClassesListener());
            }
        }
    }

struct AdvancedGroupingHandler;
/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct GroupingHandler
{
protected:
    virtual GroupingType _GetType() const = 0;
    virtual bool _AppliesForClass(ECClassCR ecClass, bool polymorphic) const = 0;
    virtual bvector<GroupingHandler const*> _FindMatchingHandlersOfTheSameType(bvector<GroupingHandler const*>::iterator begin, bvector<GroupingHandler const*>::iterator end) const {return bvector<GroupingHandler const*>();}
    virtual bool _IsAppliedTo(NavNodeCR node) const = 0;
    virtual AdvancedGroupingHandler* _AsAdvancedGroupingHandler() {return nullptr;}
public:
    virtual ~GroupingHandler() {}
    GroupingType GetType() const {return _GetType();}
    Utf8CP GetName() const
        {
        switch (GetType())
            {
            case GroupingType::BaseClass: return "BaseClassGrouping";
            case GroupingType::Class: return "ClassGrouping";
            case GroupingType::DisplayLabel: return "LabelGrouping";
            case GroupingType::Property: return "PropertyGrouping";
            case GroupingType::SameLabelInstance: return "SameLabelGrouping";
            }
        return "";
        }
    bvector<GroupingHandler const*> FindMatchingHandlersOfTheSameType(bvector<GroupingHandler const*>::iterator begin, bvector<GroupingHandler const*>::iterator end) const {return _FindMatchingHandlersOfTheSameType(begin, end);}
    bool AppliesForClass(ECClassCR ecClass, bool polymorphic) const {return _AppliesForClass(ecClass, polymorphic);}
    bool IsAppliedTo(NavNodeCR node) const {return _IsAppliedTo(node);}
    AdvancedGroupingHandler const* AsAdvancedGroupingHandler() const {return const_cast<GroupingHandler*>(this)->_AsAdvancedGroupingHandler();}
    AdvancedGroupingHandler* AsAdvancedGroupingHandler() {return _AsAdvancedGroupingHandler();}
};

/*=================================================================================**//**
* Handles GroupByClass grouping
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ClassGroupingHandler : GroupingHandler
{
DEFINE_T_SUPER(GroupingHandler)
protected:
    GroupingType _GetType() const override {return GroupingType::Class;}
    bool _AppliesForClass(ECClassCR, bool) const override {return true;}
    bool _IsAppliedTo(NavNodeCR node) const override {return node.GetKey()->AsECClassGroupingNodeKey() && !node.GetKey()->AsECClassGroupingNodeKey()->IsPolymorphic();}
public:
    ClassGroupingHandler() {}
};

/*=================================================================================**//**
* Handles GroupByLabel grouping
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct DisplayLabelGroupingHandler : GroupingHandler
{
DEFINE_T_SUPER(GroupingHandler)
protected:
    GroupingType _GetType() const override {return GroupingType::DisplayLabel;}
    bool _AppliesForClass(ECClassCR, bool) const override {return true;}
    bool _IsAppliedTo(NavNodeCR node) const override {return node.GetType().Equals(NAVNODE_TYPE_DisplayLabelGroupingNode);}
public:
    DisplayLabelGroupingHandler() {}
};

/*=================================================================================**//**
* Base class for handlers which group based on grouping rules (as opposed to GroupByClass,
* GroupByRelationship, GroupByLabel).
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct AdvancedGroupingHandler : GroupingHandler
{
protected:
    ECSchemaHelper const& m_schemaHelper;
    GroupingRuleCR m_rule;
    mutable ECClassCP m_ruleClass;
    mutable ECClassCP m_targetClass;
protected:
    AdvancedGroupingHandler(GroupingRuleCR rule, ECSchemaHelper const& helper) : m_ruleClass(nullptr), m_targetClass(nullptr), m_rule(rule), m_schemaHelper(helper) {}
    ECClassCP GetRuleClass() const
        {
        if (nullptr == m_ruleClass)
            {
            m_ruleClass = m_schemaHelper.GetECClass(m_rule.GetSchemaName().c_str(), m_rule.GetClassName().c_str());
            if (!m_ruleClass)
                {
                DIAGNOSTICS_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, LOG_ERROR, Utf8PrintfString("Grouping rule target class not found: '%s:%s'",
                    m_rule.GetSchemaName().c_str(), m_rule.GetClassName().c_str()));
                }
            }
        return m_ruleClass;
        }
    virtual ECClassCP _GetTargetClass() const {return GetRuleClass();}
    bvector<GroupingHandler const*> _FindMatchingHandlersOfTheSameType(bvector<GroupingHandler const*>::iterator begin, bvector<GroupingHandler const*>::iterator end) const override
        {
        bvector<GroupingHandler const*> matches;
        ECClassCP myTargetClass = GetTargetClass();
        if (nullptr == myTargetClass)
            return matches;

        for (auto iter = begin; iter != end; ++iter)
            {
            AdvancedGroupingHandler const* handler = (*iter)->AsAdvancedGroupingHandler();
            if (!handler)
                continue;

            ECClassCP handlerTargetClass = handler->GetTargetClass();
            if (!handlerTargetClass)
                continue;

            if (handlerTargetClass->Is(myTargetClass))
                matches.push_back(handler);
            }
        return matches;
        }
    bool _AppliesForClass(ECClassCR ecClass, bool polymorphic) const override
        {
        ECClassCP targetClass = GetTargetClass();
        if (nullptr == targetClass)
            return false;

        if (ecClass.Is(targetClass))
            return true;

        if (polymorphic && targetClass->Is(&ecClass))
            return true;

        return false;
        }
    AdvancedGroupingHandler* _AsAdvancedGroupingHandler() override {return this;}
public:
    GroupingRuleCR GetRule() const {return m_rule;}
    ECClassCP GetTargetClass() const
        {
        if (nullptr == m_targetClass)
            m_targetClass = _GetTargetClass();
        return m_targetClass;
        }
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct BaseClassGroupingHandler : AdvancedGroupingHandler
{
DEFINE_T_SUPER(AdvancedGroupingHandler)

private:
    ClassGroupCR m_specification;
    mutable ECClassCP m_specClass;

private:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    ECClassCP GetGroupingSpecificationClass() const
        {
        if (nullptr == m_specClass && !m_specification.GetSchemaName().empty() && !m_specification.GetBaseClassName().empty())
            {
            m_specClass = m_schemaHelper.GetECClass(m_specification.GetSchemaName().c_str(), m_specification.GetBaseClassName().c_str());
            if (!m_specClass)
                {
                DIAGNOSTICS_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, LOG_ERROR, Utf8PrintfString("Base class grouping specification class not found: '%s:%s'",
                    m_specification.GetSchemaName().c_str(), m_specification.GetBaseClassName().c_str()));
                }
            }
        return m_specClass;
        }
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    ECClassCP GetGroupingClass() const
        {
        auto specClass = GetGroupingSpecificationClass();
        return specClass ? specClass : GetRuleClass();
        }

protected:
    GroupingType _GetType() const override {return GroupingType::BaseClass;}

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    ECClassCP _GetTargetClass() const override
        {
        auto ruleClass = GetRuleClass();
        auto specClass = GetGroupingSpecificationClass();
        return (specClass && specClass->Is(ruleClass)) ? specClass : ruleClass;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    bool _IsAppliedTo(NavNodeCR node) const override
        {
        auto key = node.GetKey()->AsECClassGroupingNodeKey();
        return key && key->GetECClassId() == GetBaseECClassId() && key->IsPolymorphic();
        }

public:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    BaseClassGroupingHandler(ECSchemaHelper const& schemaHelper, GroupingRuleCR rule, ClassGroupCR specification)
        : T_Super(rule, schemaHelper), m_specification(specification), m_specClass(nullptr)
        {}

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    ClassGroupCR GetGroupingSpecification() const {return m_specification;}

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    ECClassId GetBaseECClassId() const
        {
        auto groupingClass = GetGroupingClass();
        return groupingClass ? groupingClass->GetId() : ECClassId();
        }
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct PropertyGroupingHandler : AdvancedGroupingHandler
{
DEFINE_T_SUPER(AdvancedGroupingHandler)

private:
    PropertyGroupCR m_specification;
    mutable ECPropertyCP m_groupingProperty;

protected:
    virtual GroupingType _GetType() const override {return GroupingType::Property;}

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    bool _IsAppliedTo(NavNodeCR node) const override
        {
        auto key = node.GetKey()->AsECPropertyGroupingNodeKey();
        return key
            && key->GetECClass().Is(GetTargetClass())
            && key->GetPropertyName().Equals(m_specification.GetPropertyName());
        }

public:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    PropertyGroupingHandler(ECSchemaHelper const& schemaHelper, GroupingRuleCR rule, PropertyGroupCR specification)
        : T_Super(rule, schemaHelper), m_specification(specification), m_groupingProperty(nullptr)
        {}

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    PropertyGroupCR GetGroupingSpecification() const {return m_specification;}

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    ECPropertyCP GetGroupingProperty() const
        {
        if (nullptr == m_groupingProperty)
            {
            ECClassCP ecClass = GetTargetClass();
            if (!ecClass)
                return nullptr;

            m_groupingProperty = ecClass->GetPropertyP(m_specification.GetPropertyName().c_str());
            if (nullptr == m_groupingProperty)
                {
                DIAGNOSTICS_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, LOG_ERROR, Utf8PrintfString("Requested property does not exist in class: '%s.%s'",
                    ecClass->GetFullName(), m_specification.GetPropertyName().c_str()));
                }
            }
        return m_groupingProperty;
        }
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct SameLabelInstanceGroupingHandler : AdvancedGroupingHandler
{
DEFINE_T_SUPER(AdvancedGroupingHandler)

protected:
    GroupingType _GetType() const override {return GroupingType::SameLabelInstance;}

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    bool _IsAppliedTo(NavNodeCR node) const override
        {
        auto key = node.GetKey()->AsECInstanceNodeKey();
        return key && ContainerHelpers::Contains(key->GetInstanceKeys(), [this](auto const& instanceKey){return instanceKey.GetClass()->Is(GetTargetClass());});
        }

public:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    SameLabelInstanceGroupingHandler(ECSchemaHelper const& schemaHelper, GroupingRuleCR rule)
        : T_Super(rule, schemaHelper)
        {}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct GroupingResolver
{
    /*=============================================================================**//**
    * @bsiclass
    +===============+===============+===============+===============+===============+==*/
    struct GroupingSpecificationsVisitor : GroupingRuleSpecificationVisitor
    {
    private:
        ECSchemaHelper const& m_schemaHelper;
        GroupingRuleCP m_rule;
        bvector<std::unique_ptr<GroupingHandler const>>& m_groupingHandlers;

    protected:
        virtual void _Visit(SameLabelInstanceGroupCR specification)
            {
            if (specification.GetApplicationStage() == SameLabelInstanceGroupApplicationStage::Query)
                m_groupingHandlers.push_back(std::make_unique<SameLabelInstanceGroupingHandler>(m_schemaHelper, *m_rule));
            }
        virtual void _Visit(ClassGroupCR specification) {m_groupingHandlers.push_back(std::make_unique<BaseClassGroupingHandler>(m_schemaHelper, *m_rule, specification));}
        virtual void _Visit(PropertyGroupCR specification) {m_groupingHandlers.push_back(std::make_unique<PropertyGroupingHandler>(m_schemaHelper, *m_rule, specification));}

    public:
        GroupingSpecificationsVisitor(bvector<std::unique_ptr<GroupingHandler const>>& groupingHandlers, ECSchemaHelper const& schemaHelper)
            : m_groupingHandlers(groupingHandlers), m_schemaHelper(schemaHelper)
            {}
        void SetRule(GroupingRuleCR rule) {m_rule = &rule;}
    };

private:
    NavigationQueryBuilderParameters const& m_queryBuilderParams;
    ChildNodeSpecificationCR m_specification;
    Utf8StringCR m_specificationIdentifier;
    NavNodeCP m_parentNode;
    NavNodeCPtr m_parentInstanceNode;

    bvector<std::unique_ptr<GroupingHandler const>> m_groupingHandlers;
    GroupingHandler const* m_parentGrouping;

    std::unique_ptr<SelectClass<ECClass>> m_groupingClass;

private:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    bool SpecificationMatches(NavNodeCR node) const
        {
        return node.GetKey()->GetSpecificationIdentifier().Equals(m_specificationIdentifier);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    void Init(NavNodeCP node)
        {
        m_parentNode = node;
        m_parentGrouping = nullptr;
        m_parentInstanceNode = node ? HierarchiesInstanceFilteringHelper::GetParentInstanceNode(m_queryBuilderParams.GetNodesCache(), *node) : nullptr;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    void ResolveGrouping(bool groupByClass, bool groupByLabel)
        {
        if (groupByClass)
            m_groupingHandlers.push_back(std::make_unique<ClassGroupingHandler>());
        if (groupByLabel)
            m_groupingHandlers.push_back(std::make_unique<DisplayLabelGroupingHandler>());

        GroupingSpecificationsVisitor visitor(m_groupingHandlers, m_queryBuilderParams.GetSchemaHelper());
        IRulesPreprocessor::AggregateCustomizationRuleParameters params(m_parentInstanceNode.get(), m_specificationIdentifier);
        bvector<GroupingRuleCP> groupingRules = m_queryBuilderParams.GetRulesPreprocessor().GetGroupingRules(params);
        for (GroupingRuleCP rule : groupingRules)
            {
            DiagnosticsHelpers::ReportRule(*rule);
            if (rule->GetGroups().empty())
                continue;

            visitor.SetRule(*rule);

            GroupSpecificationCP activeSpecification = QueryBuilderHelpers::GetActiveGroupingSpecification(*rule, GetQueryBuilderParams().GetLocalState());
            if (nullptr != activeSpecification)
                activeSpecification->Accept(visitor);
            }

        std::stable_sort(m_groupingHandlers.begin(), m_groupingHandlers.end(), [](auto const& lhs, auto const& rhs){return lhs->GetType() > rhs->GetType();});

        ResolveParentGrouping();
        ResolveGroupingClass();
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    void ResolveParentGrouping()
        {
        if (nullptr == m_parentNode)
            return;

        if (!m_parentNode->GetKey()->AsGroupingNodeKey())
            return;

        for (auto iter = m_groupingHandlers.rbegin(); iter != m_groupingHandlers.rend(); ++iter)
            {
            GroupingHandler const& handler = **iter;
            if (handler.IsAppliedTo(*m_parentNode))
                {
                m_parentGrouping = &handler;
                DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, Utf8PrintfString("Parent grouping: `%s`", handler.GetName()));
                return;
                }
            }

        DIAGNOSTICS_ASSERT_SOFT(DiagnosticsCategory::Hierarchies, (nullptr != m_parentGrouping), "Failed to determine parent grouping");
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    void ResolveGroupingClass()
        {
        SelectClass<ECClass> bestClassMatch;
        RefCountedPtr<NavNode const> node = m_parentNode;
        while (node.IsValid() && SpecificationMatches(*node))
            {
            if (node->GetKey()->AsECInstanceNodeKey())
                {
                // if we found an ECInstance parent created using the same specification, it means the specification
                // is used to create multiple hierarchy levels - when looking for class grouping node we have to break
                // as soon as we find the first parent ECInstance node or otherwise we'll be grouping children by
                // parent's class
                break;
                }

            if (node->GetKey()->AsECClassGroupingNodeKey())
                {
                auto classKey = node->GetKey()->AsECClassGroupingNodeKey();
                if (!bestClassMatch.IsValid())
                    {
                    bestClassMatch = SelectClass<ECClass>(classKey->GetECClass(), "", classKey->IsPolymorphic());
                    }
                else if (classKey->GetECClass().Is(&bestClassMatch.GetClass()))
                    {
                    if (&classKey->GetECClass() != &bestClassMatch.GetClass())
                        bestClassMatch = SelectClass<ECClass>(classKey->GetECClass(), "", classKey->IsPolymorphic());
                    else if (!classKey->IsPolymorphic())
                        bestClassMatch.SetIsSelectPolymorphic(classKey->IsPolymorphic());
                    }
                }

            node = GetQueryBuilderParams().GetNodesCache().GetPhysicalParentNode(
                node->GetNodeId(),
                GetQueryBuilderParams().GetRulesetVariables(),
                GetQueryBuilderParams().GetInstanceFilter());
            }
        if (bestClassMatch.IsValid())
            {
            m_groupingClass = std::make_unique<SelectClass<ECClass>>(bestClassMatch);
            DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, Utf8PrintfString("Grouping class: `%s`", m_groupingClass->GetClass().GetFullName()));
            }
        else
            {
            DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, "No grouping class");
            }
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    bvector<GroupingHandler const*> GetGroupingHandlers(GroupingType thisLevel) const
        {
        bvector<GroupingHandler const*> handlers;
        int currentLevel = (int)thisLevel;
        while (0 <= currentLevel && handlers.empty())
            {
            for (auto const& handler : m_groupingHandlers)
                {
                if ((int)handler->GetType() < currentLevel)
                    break;

                if ((int)handler->GetType() == currentLevel)
                    handlers.push_back(handler.get());
                }
            currentLevel--;
            }
        return handlers;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    static bool DoesHandlerApply(GroupingHandler const& handler, bvector<ECClassCP> const& selectClasses, bool polymorphically)
        {
        for (size_t i = 0; i < selectClasses.size(); ++i)
            {
            ECClassCP selectClass = selectClasses[i];
            if (handler.AppliesForClass(*selectClass, polymorphically))
                return true;
            }
        return false;
        }

public:
    GroupingResolver(NavigationQueryBuilderParameters const& params, NavNodeCP node, Utf8StringCR specificationHash, AllInstanceNodesSpecificationCR specification)
        : m_queryBuilderParams(params), m_specification(specification), m_specificationIdentifier(specificationHash)
        {
        Init(node);
        ResolveGrouping(specification.GetGroupByClass(), specification.GetGroupByLabel());
        }
    GroupingResolver(NavigationQueryBuilderParameters const& params, NavNodeCP node, Utf8StringCR specificationHash, AllRelatedInstanceNodesSpecificationCR specification)
        : m_queryBuilderParams(params), m_specification(specification), m_specificationIdentifier(specificationHash)
        {
        Init(node);
        ResolveGrouping(specification.GetGroupByClass(), specification.GetGroupByLabel());
        }
    GroupingResolver(NavigationQueryBuilderParameters const& params, NavNodeCP node, Utf8StringCR specificationHash, RelatedInstanceNodesSpecificationCR specification)
        : m_queryBuilderParams(params), m_specification(specification), m_specificationIdentifier(specificationHash)
        {
        Init(node);
        ResolveGrouping(specification.GetGroupByClass(), specification.GetGroupByLabel());
        }
    GroupingResolver(NavigationQueryBuilderParameters const& params, NavNodeCP node, Utf8StringCR specificationHash, InstanceNodesOfSpecificClassesSpecificationCR specification)
        : m_queryBuilderParams(params), m_specification(specification), m_specificationIdentifier(specificationHash)
        {
        Init(node);
        ResolveGrouping(specification.GetGroupByClass(), specification.GetGroupByLabel());
        }
    GroupingResolver(NavigationQueryBuilderParameters const& params, NavNodeCP node, Utf8StringCR specificationHash, SearchResultInstanceNodesSpecificationCR specification)
        : m_queryBuilderParams(params), m_specification(specification), m_specificationIdentifier(specificationHash)
        {
        Init(node);
        ResolveGrouping(specification.GetGroupByClass(), specification.GetGroupByLabel());
        }

    NavNodeCP GetParentNode() const {return m_parentNode;}
    NavNodeCP GetParentInstanceNode() const {return m_parentInstanceNode.get();}
    NavigationQueryBuilderParameters const& GetQueryBuilderParams() const {return m_queryBuilderParams;}
    ChildNodeSpecificationCR GetSpecification() const {return m_specification;}
    Utf8StringCR GetSpecificationIdentifier() const {return m_specificationIdentifier;}

    SelectClass<ECClass> const* GetGroupingClass() const {return m_groupingClass.get();}

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    bvector<GroupingHandler const*> GetHandlersForNextGroupingLevel(bvector<ECClassCP> const& selectClasses, bool polymorphic) const
        {
        bvector<GroupingHandler const*> matchingHandlers;
        if (m_groupingHandlers.empty())
            return matchingHandlers;

        if (nullptr != m_parentGrouping)
            {
            if (0 == (int)m_parentGrouping->GetType())
                return matchingHandlers;

            // check if there're any handlers of the same grouping type as the
            // parent (the case of multiple nested property grouping nodes)
            GroupingType parentGroupingType = m_parentGrouping->GetType();
            bvector<GroupingHandler const*> handlersOfTheSameType = GetGroupingHandlers(parentGroupingType);
            bool foundParentGroupingHandler = false;

            for (auto iter = handlersOfTheSameType.begin(); iter != handlersOfTheSameType.end(); ++iter)
                {
                GroupingHandler const* handler = *iter;
                if (foundParentGroupingHandler && DoesHandlerApply(*handler, selectClasses, polymorphic))
                    {
                    matchingHandlers.push_back(handler);
                    return matchingHandlers;
                    }

                if (m_parentGrouping == handler)
                    {
                    foundParentGroupingHandler = true;
                    bvector<GroupingHandler const*> parentMatches = m_parentGrouping->FindMatchingHandlersOfTheSameType(iter + 1, handlersOfTheSameType.end());
                    ContainerHelpers::RemoveIf(parentMatches, [&](auto const& handler){return !DoesHandlerApply(*handler, selectClasses, polymorphic);});
                    if (!parentMatches.empty())
                        return parentMatches;
                    }
                }
            }

        int nextGroupingType = (nullptr == m_parentGrouping) ? (int)GroupingType::Relationship : ((int)m_parentGrouping->GetType() - 1);
        bvector<GroupingHandler const*> handlers = GetGroupingHandlers((GroupingType)nextGroupingType--);
        while (!handlers.empty())
            {
            for (GroupingHandler const* handler : handlers)
                {
                if (DoesHandlerApply(*handler, selectClasses, polymorphic))
                    matchingHandlers.push_back(handler);
                }
            if (!matchingHandlers.empty())
                break;

            handlers = GetGroupingHandlers((GroupingType)nextGroupingType--);
            }
        return matchingHandlers;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    bvector<GroupingNodeAndHandler> GetFilterHandlers() const
        {
        bvector<GroupingNodeAndHandler> result;

        // get filtering grouping handlers
        bvector<GroupingHandler const*> filters;
        for (auto const& handler : m_groupingHandlers)
            {
            filters.push_back(handler.get());
            if (m_parentGrouping == handler.get())
                break;
            }

        // match handlers with grouping nodes
        auto iter = filters.rbegin();
        NavNodeCPtr node = m_parentNode;
        while (node.IsValid() && SpecificationMatches(*node) && iter != filters.rend())
            {
            GroupingHandler const* handler = *iter++;
            if (!handler->IsAppliedTo(*node))
                continue;

            result.push_back(GroupingNodeAndHandler(*node, *handler));
            node = GetQueryBuilderParams().GetNodesCache().GetPhysicalParentNode(
                node->GetNodeId(),
                GetQueryBuilderParams().GetRulesetVariables(),
                GetQueryBuilderParams().GetInstanceFilter());
            }

        return result;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    bvector<AdvancedGroupingHandler const*> GetAppliedGroupingRuleHandlers(bvector<ECClassCP> const& selectClasses) const
        {
        bvector<AdvancedGroupingHandler const*> handlers;

        bvector<GroupingNodeAndHandler> filterHandlers = GetFilterHandlers();
        for (GroupingNodeAndHandler const& handler : filterHandlers)
            {
            AdvancedGroupingHandler const* advancedHandler = handler.GetHandler().AsAdvancedGroupingHandler();
            if (nullptr != advancedHandler)
                handlers.push_back(advancedHandler);
            }

        bvector<GroupingHandler const*> nextLevelHandlers = GetHandlersForNextGroupingLevel(selectClasses, true);
        for (GroupingHandler const* handler : nextLevelHandlers)
            {
            AdvancedGroupingHandler const* advancedHandler = handler->AsAdvancedGroupingHandler();
            if (nullptr != advancedHandler)
                handlers.push_back(advancedHandler);
            }

        return handlers;
        }
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct InstanceFilterHelper
{
private:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    InstanceFilterHelper() {}

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    static void ApplyParentFiltering(ComplexQueryBuilder& query, bvector<HierarchiesInstanceFilteringHelper::ParentClassInstanceIds> const& filters)
        {
        for (auto const& filter : filters)
            {
            query.From(filter.selectClass);

            ValuesFilteringHelper helper(filter.instanceIds);
            query.Where(helper.CreateWhereClause(Utf8PrintfString("[%s].[ECInstanceId]", filter.selectClass.GetAlias().c_str()).c_str()).c_str(), helper.CreateBoundValues());
            }
        }

public:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    static void ApplyInstanceFilter(ComplexQueryBuilder& query, SelectQueryInfo const& selectInfo, NavigationQueryBuilderParameters const& params, NavNodeCP parentInstanceNode, NavNodeCP parentNode)
        {
        if (!selectInfo.GetInstanceFilterECSqlExpression().GetClause().empty())
            query.Where(selectInfo.GetInstanceFilterECSqlExpression());

        if (selectInfo.GetInstanceFilterDefinitions().empty())
            return;

        ECClassUseCounter relatedInstanceClassesCounter;

        for (auto const& instanceFilterDef : selectInfo.GetInstanceFilterDefinitions())
            {
            if (!instanceFilterDef || instanceFilterDef->GetExpression().empty())
                continue;

            auto parentInstanceFilteringInfo = HierarchiesInstanceFilteringHelper::CreateParentInstanceFilteringInfo(params.GetNodesCache(),
                parentInstanceNode, instanceFilterDef->GetExpression());
            ApplyParentFiltering(query, parentInstanceFilteringInfo.classInstanceIds);

            for (auto const& filter : parentInstanceFilteringInfo.classInstanceIds)
                OnSelected(filter.selectClass, params);

            for (RelatedClassPath relatedInstancePath : instanceFilterDef->GetRelatedInstances())
                {
                for (auto& step : relatedInstancePath)
                    {
                    if (step.GetRelationship().GetAlias().empty())
                        step.GetRelationship().SetAlias(RULES_ENGINE_RELATED_CLASS_ALIAS(step.GetRelationship().GetClass(), relatedInstanceClassesCounter.Inc(&step.GetRelationship().GetClass())));
                    if (step.GetTargetClass().GetAlias().empty())
                        step.GetTargetClass().SetAlias(RULES_ENGINE_RELATED_CLASS_ALIAS(step.GetTargetClass().GetClass(), relatedInstanceClassesCounter.Inc(&step.GetTargetClass().GetClass())));
                    }
                query.Join(relatedInstancePath);
                }

            ECExpressionContextsProvider::NodeRulesContextParameters contextParams(parentNode, params.GetConnection(),
                params.GetRulesetVariables(), params.GetUsedVariablesListener());
            auto expressionContext = ECExpressionContextsProvider::GetNodeRulesContext(contextParams);
            query.Where(ECExpressionsHelper(params.GetECExpressionsCache()).ConvertToECSql(parentInstanceFilteringInfo.modifiedInstanceFilter,
                nullptr, expressionContext.get()));

            if (nullptr != params.GetUsedClassesListener())
                {
                UsedClassesHelper::NotifyListenerWithUsedClasses(*params.GetUsedClassesListener(), params.GetSchemaHelper(),
                    parentInstanceFilteringInfo.modifiedInstanceFilter);
                }
            }
        }
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct RelatedClassFilteringHelper
{
    /*---------------------------------------------------------------------------------**//**
    * @bsiclass
    +---------------+---------------+---------------+---------------+---------------+------*/
    struct FilteringHandler
        {
        private:
            RelatedClassPathCR m_pathFromSelectToParentClass;
            bvector<ECInstanceId> const& m_parentInstanceIds;

        protected:
            virtual void _ApplyFiltering(ComplexQueryBuilderR) const = 0;

        protected:
            FilteringHandler(RelatedClassPathCR pathFromSelectToParentClass, bvector<ECInstanceId> const& parentInstanceIds)
                : m_pathFromSelectToParentClass(pathFromSelectToParentClass), m_parentInstanceIds(parentInstanceIds)
                {}
            RelatedClassPathCR GetPathFromSelectToParentClass() const {return m_pathFromSelectToParentClass;}
            bvector<ECInstanceId> const& GetParentInstanceIds() const {return m_parentInstanceIds;}

        public:
            virtual ~FilteringHandler() {}
            void ApplyFiltering(ComplexQueryBuilderR query) const {_ApplyFiltering(query);}

        };

    /*---------------------------------------------------------------------------------**//**
    * @bsiclass
    +---------------+---------------+---------------+---------------+---------------+------*/
    struct JoinFilteringHandler : FilteringHandler
        {
        protected:
            void _ApplyFiltering(ComplexQueryBuilderR query) const override
                {
                query.Where(ValuesFilteringHelper(GetParentInstanceIds()).Create("[related].[ECInstanceId]"));
                query.Join(GetPathFromSelectToParentClass());
                }
        public:
            JoinFilteringHandler(RelatedClassPathCR pathFromSelectToParentClass, bvector<ECInstanceId> const& parentInstanceIds)
                : FilteringHandler(pathFromSelectToParentClass, parentInstanceIds)
                {}
        };

    /*---------------------------------------------------------------------------------**//**
    * @bsiclass
    +---------------+---------------+---------------+---------------+---------------+------*/
    struct WhereExistsFilteringHandler : FilteringHandler
        {
        private:
            NavigationECPropertyCR m_navProp;

        protected:
            void _ApplyFiltering(ComplexQueryBuilderR query) const override
                {
                auto whereQuery = ComplexQueryBuilder::Create();
                RelatedClassCR firstPathStep = GetPathFromSelectToParentClass().front();
                Utf8String propertyClause = QueryHelpers::Wrap(m_navProp.GetName()).append(".[Id]");
                Utf8String sourceJoinIdClause, targetJoinIdClause;
                if (firstPathStep.IsForwardRelationship() == (ECRelatedInstanceDirection::Backward == m_navProp.GetDirection()))
                    {
                    targetJoinIdClause = propertyClause;
                    sourceJoinIdClause = "[ECInstanceId]";
                    }
                else
                    {
                    targetJoinIdClause = "[ECInstanceId]";
                    sourceJoinIdClause = propertyClause;
                    }

                SelectClass<ECClass> firstStepTargetSelectClass(firstPathStep.GetTargetClass());
                if (GetPathFromSelectToParentClass().size() > 1)
                    firstStepTargetSelectClass.SetShouldDisqualify(true);

                RefCountedPtr<SimpleQueryContract> queryContract = SimpleQueryContract::Create({
                    PresentationQueryContractSimpleField::Create("", "1", false, false, FieldVisibility::Inner)
                    });
                whereQuery->SelectContract(*queryContract, firstPathStep.GetTargetClass().GetAlias().c_str());
                whereQuery->From(firstStepTargetSelectClass);
                if (GetPathFromSelectToParentClass().size() > 1)
                    {
                    // remove fist path step because query is selecting from first step target
                    RelatedClassPath copy(GetPathFromSelectToParentClass());
                    copy.erase(copy.begin());
                    whereQuery->Join(copy);
                    }
                whereQuery->Where(Utf8PrintfString("[this].%s = [%s].%s", sourceJoinIdClause.c_str(), firstPathStep.GetTargetClass().GetAlias().c_str(), targetJoinIdClause.c_str()).c_str(), BoundQueryValuesList());
                whereQuery->Where(ValuesFilteringHelper(GetParentInstanceIds()).Create("[related].[ECInstanceId]"));

                query.Where(Utf8String("EXISTS (").append(whereQuery->GetQuery()->GetQueryString()).append(")").c_str(), whereQuery->GetQuery()->GetBindings());
                }

        public:
            WhereExistsFilteringHandler(RelatedClassPathCR pathFromSelectToParentClass, bvector<ECInstanceId> const& parentInstanceIds, NavigationECPropertyCR navProp)
                : FilteringHandler(pathFromSelectToParentClass, parentInstanceIds), m_navProp(navProp)
                {}
        };

    /*---------------------------------------------------------------------------------**//**
    * @bsiclass
    +---------------+---------------+---------------+---------------+---------------+------*/
    struct WhereInFilteringHandler : FilteringHandler
        {
        private:
            SelectClass<ECClass> const& m_selectClass;

        protected:
            void _ApplyFiltering(ComplexQueryBuilderR query) const override
                {
                ComplexQueryBuilderPtr whereQuery = ComplexQueryBuilder::Create();
                RefCountedPtr<SimpleQueryContract> queryContract = SimpleQueryContract::Create({
                        PresentationQueryContractSimpleField::Create("/RelatedInstanceId/", "ECInstanceId", true, false, FieldVisibility::Inner)
                    });
                SelectClass<ECClass> relatedSelectClass(m_selectClass);
                relatedSelectClass.SetAlias("relatedInstances");
                whereQuery->SelectContract(*queryContract, relatedSelectClass.GetAlias().c_str());
                whereQuery->From(relatedSelectClass);
                whereQuery->Join(GetPathFromSelectToParentClass());
                whereQuery->Where(ValuesFilteringHelper(GetParentInstanceIds()).Create("[related].[ECInstanceId]"));

                query.Where(Utf8String("[this].[ECInstanceId] IN (").append(whereQuery->GetQuery()->GetQueryString()).append(")").c_str(), whereQuery->GetQuery()->GetBindings());
                }

        public:
            WhereInFilteringHandler(RelatedClassPathCR pathFromSelectToParentClass, bvector<ECInstanceId> const& parentInstanceIds, SelectClass<ECClass> const& selectClass)
                : FilteringHandler(pathFromSelectToParentClass, parentInstanceIds), m_selectClass(selectClass)
                {}
        };

private:
    RelatedClassFilteringHelper() {}

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    static bool HasXToManyRelationship(RelatedClassPathCR classpath)
        {
        for (auto& path : classpath)
            {
            if (!path.GetRelationship().IsValid())
                continue;

            ECRelationshipClassCR rel = path.GetRelationship().GetClass();
            ECRelationshipConstraintCR target = path.IsForwardRelationship() ? rel.GetTarget() : rel.GetSource();
            if (target.GetMultiplicity().IsUpperLimitUnbounded() || target.GetMultiplicity().GetUpperLimit() > 1)
                return true;
            }
        return false;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    static bool IsGroupingByRelationshipProperty(GroupingHandler const& handler)
        {
        if (handler.GetType() != GroupingType::Property)
            return false;

        ECPropertyCP groupingProperty = static_cast<PropertyGroupingHandler const&>(handler).GetGroupingProperty();
        return groupingProperty && groupingProperty->GetClass().IsRelationshipClass();
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    static std::unique_ptr<FilteringHandler> GetFilteringHandler(SelectClass<ECClass> const& selectClass, RelatedClassPathCR pathFromSelectToParentClass,
        bvector<ECInstanceId> const& parentInstanceIds, bvector<GroupingHandler const*> const& filterHandlers)
        {
        if (!HasXToManyRelationship(pathFromSelectToParentClass) || ContainerHelpers::Contains(filterHandlers, [](auto const& handler) {return IsGroupingByRelationshipProperty(*handler); }))
            return std::make_unique<JoinFilteringHandler>(pathFromSelectToParentClass, parentInstanceIds);

        auto navProp = pathFromSelectToParentClass.front().GetNavigationProperty();
        if (nullptr != navProp)
            return std::make_unique<WhereExistsFilteringHandler>(pathFromSelectToParentClass, parentInstanceIds, *navProp);

        return std::make_unique<WhereInFilteringHandler>(pathFromSelectToParentClass, parentInstanceIds, selectClass);
        }

public:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    static void FilterByRelatedClass(ComplexQueryBuilderR query, SelectClass<ECClass> const& selectClass, RelatedClassPathCR pathFromSelectToParentClass,
        bvector<ECInstanceId> const& parentInstanceIds, bvector<GroupingHandler const*> const& filterHandlers)
        {
        auto handler = GetFilteringHandler(selectClass, pathFromSelectToParentClass, parentInstanceIds, filterHandlers);
        handler->ApplyFiltering(query);
        }
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct SelectQueryHandler
{
    struct AcceptResult
        {
        enum class Status
            {
            Reject = 0,
            AcceptPartially = 1,
            AcceptFully = 2,
            };

        private:
            Status m_status;
            std::shared_ptr<SelectQueryInfo> m_partialQueryInfo;
            std::function<bool(SelectQueryHandler const&)> m_partialAcceptCriteria;
        private:
            AcceptResult(Status status) : m_status(status) {}
        public:
            static AcceptResult CreateRejected() {return AcceptResult(Status::Reject);}
            static AcceptResult CreateFullyAccepted() {return AcceptResult(Status::AcceptFully);}
            static AcceptResult CreatePartiallyAccepted(std::shared_ptr<SelectQueryInfo> leftoverSelectQueryInfo)
                {
                AcceptResult result(Status::AcceptPartially);
                result.SetPartialQueryInfo(leftoverSelectQueryInfo);
                return result;
                }
            Status GetStatus() const {return m_status;}
            std::shared_ptr<SelectQueryInfo> GetPartialQueryInfo() const {return m_partialQueryInfo;}
            void SetPartialQueryInfo(std::shared_ptr<SelectQueryInfo> value) {m_partialQueryInfo = value;}
            std::function<bool(SelectQueryHandler const&)> GetPartialAcceptCriteria() const {return m_partialAcceptCriteria;}
            void SetPartialAcceptCriteria(std::function<bool(SelectQueryHandler const&)> value) {m_partialAcceptCriteria = value;}
        };

private:
    NavigationQueryBuilderParameters const& m_params;
    NavNodeCP m_parentNode;
    NavNodeCP m_parentInstanceNode;
    ChildNodeSpecification const& m_specification;
    Utf8StringCR m_specificationIdentifier;
    std::function<uint64_t()> m_selectContractIdAllocator;

protected:
    NavigationQueryBuilderParameters const& GetQueryBuilderParams() const {return m_params;}
    NavNodeCP GetParentNode() const {return m_parentNode;}
    NavNodeCP GetParentInstanceNode() const {return m_parentInstanceNode;}
    ChildNodeSpecification const& GetSpecification() const {return m_specification;}
    Utf8StringCR GetSpecificationIdentifier() const {return m_specificationIdentifier;}
    Utf8StringCR GetSpecificationIdentifierForContract() const
        {
        if (!m_params.ShouldUseSpecificationIdentifierInContracts())
            {
            static Utf8String const s_empty;
            return s_empty;
            }
        return m_specificationIdentifier;
        }
    uint64_t AllocateContractId() const {return m_selectContractIdAllocator();}

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    void JoinRelatedInstancePaths(ComplexQueryBuilder& query, bvector<RelatedClassPath> const& relatedInstancePaths) const
        {
        for (RelatedClassPathCR relatedInstancePath : relatedInstancePaths)
            query.Join(relatedInstancePath);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    ComplexQueryBuilderPtr CreateQueryBase(PresentationQueryContractR contract, SelectQueryInfo const& selectInfo, bvector<GroupingNodeAndHandler> const& filters) const
        {
        PresentationQueryContractPtr queryContract = &contract;

        bool groupByContract = false;
        if (queryContract->AsNavigationQueryContract())
            {
            if (selectInfo.GetForcedGroupingContractFactory())
                {
                // in some cases (e.g. SameLabelInstance grouping) higher priority select query handlers might want to
                // replace the contract and use it for grouping
                auto contractFactory = selectInfo.GetForcedGroupingContractFactory();
                queryContract = contractFactory(selectInfo, filters);
                groupByContract = true;
                }
            if (0 == queryContract->GetId())
                queryContract->SetId(AllocateContractId());
            }

        ComplexQueryBuilderPtr query = ComplexQueryBuilder::Create();
        query->From(selectInfo.GetSelectClass());

        JoinRelatedInstancePaths(*query, selectInfo.GetRelatedInstancePaths());

        // in case of related instance specifications, join the parent node instance(s)
        if (!selectInfo.GetPathFromParentToSelectClass().empty())
            {
            RelatedClassPath pathFromSelectToParentClass(selectInfo.GetPathFromParentToSelectClass());
            pathFromSelectToParentClass.Reverse("related", true);

            if (selectInfo.GetPathFromParentToSelectClass().size() == pathFromSelectToParentClass.size())
                {
                // the reversed path always becomes shorter if there are recursive relationships involved. if not, then
                // we just need to filter by the end of the join
                RelatedClassFilteringHelper::FilterByRelatedClass(*query, selectInfo.GetSelectClass(), pathFromSelectToParentClass, selectInfo.GetParentInstanceIds(),
                    ContainerHelpers::TransformContainer<bvector<GroupingHandler const*>>(filters, [](auto const& entry){return &entry.GetHandler();}));
                }
            else if (pathFromSelectToParentClass.empty())
                {
                // reversed path becomes empty only if the first step is recursive in which case it means we have
                // to filter 'this' by path-from-input-to-select-class target ids
                DIAGNOSTICS_ASSERT_SOFT(DiagnosticsCategory::Hierarchies, (1 == selectInfo.GetPathFromParentToSelectClass().size()), Utf8PrintfString("Expected path from parent to select select length to be 1 step, but got: %" PRIu64, selectInfo.GetPathFromParentToSelectClass().size()));
                DIAGNOSTICS_ASSERT_SOFT(DiagnosticsCategory::Hierarchies, !selectInfo.GetPathFromParentToSelectClass().back().GetTargetIds().empty(), "Expected path from parent to select class to have target ECInstance IDs");
                bvector<ECInstanceId> const& ids = selectInfo.GetPathFromParentToSelectClass().back().GetTargetIds();
                query->Where(ValuesFilteringHelper(ids).Create("[this].[ECInstanceId]"));
                }
            else
                {
                // otherwise the appropriate filtering gets applied when the reversed path is joined
                query->Join(pathFromSelectToParentClass);
                }

#ifdef wip_skipped_instance_keys_performance_issue
            // set the relationship path for the contract so it can include any skipped related instance keys into the select clause
            auto relatedQueryContract = queryContract->Clone();
            relatedQueryContract->SetPathFromSelectToParentClass(pathFromSelectToParentClass);
            queryContract = relatedQueryContract;
#endif

            // TODO: since the path can include multiple relationships with different directions, this extended
            // data attribute is ambiguous - consider removal
            query->GetNavigationResultParameters().GetNavNodeExtendedDataR().SetRelationshipDirection(selectInfo.GetPathFromParentToSelectClass().back().IsForwardRelationship() ? ECRelatedInstanceDirection::Forward : ECRelatedInstanceDirection::Backward);
            }

        // set the contract
        query->SelectContract(*queryContract, selectInfo.GetSelectClass().GetAlias().c_str());

        // apply filters from parent grouping nodes
        for (GroupingNodeAndHandler const& filter : filters)
            {
            auto filteringHandler = Create(filter.GetHandler(), GetQueryBuilderParams(), GetParentNode(), GetParentInstanceNode(), GetSpecification(), GetSpecificationIdentifier(), m_selectContractIdAllocator);
            if (filteringHandler)
                filteringHandler->ApplyFilter(query, selectInfo, filter.GetNode());
            }

        // apply instance filter from rule specification
        InstanceFilterHelper::ApplyInstanceFilter(*query, selectInfo, m_params, GetParentInstanceNode(), GetParentNode());

        // add select class
        if (ReturnsInstanceNodes(*query))
            query->GetNavigationResultParameters().GetSelectInstanceClasses().insert(&selectInfo.GetSelectClass().GetClass());

        // add relationship classes used by the query
        for (RelatedClassCR related : selectInfo.GetPathFromParentToSelectClass())
            {
            if (related.GetRelationship().IsValid())
                query->GetNavigationResultParameters().GetUsedRelationshipClasses().insert(&related.GetRelationship().GetClass());
            }
        for (RelatedClassPathCR relatedInstancePath : selectInfo.GetRelatedInstancePaths())
            {
            for (RelatedClassCR relatedInstanceClass : relatedInstancePath)
                query->GetNavigationResultParameters().GetUsedRelationshipClasses().insert(&relatedInstanceClass.GetRelationship().GetClass());
            }

        // if force grouping - do that
        if (groupByContract)
            {
            auto groupedQuery = QueryBuilderHelpers::CreateComplexNestedQueryIfNecessary(*query, queryContract->GetGroupingAliases());
            groupedQuery->GroupByContract(*queryContract);
            query = groupedQuery;
            }

        return query;
        }

protected:
    SelectQueryHandler(NavigationQueryBuilderParameters const& params, NavNodeCP parentNode, NavNodeCP parentInstanceNode, ChildNodeSpecification const& specification,
        Utf8StringCR specificationHash, std::function<uint64_t()> selectContractIdAllocator)
        : m_params(params), m_parentNode(parentNode), m_parentInstanceNode(parentInstanceNode), m_specification(specification), m_specificationIdentifier(specificationHash), m_selectContractIdAllocator(selectContractIdAllocator)
        {}
    virtual Utf8CP _GetName() const = 0;
    virtual int _GetOrderInUnion() const = 0;
    virtual AcceptResult _Accept(SelectQueryInfo&) const = 0;
    virtual PresentationQueryBuilderPtr _CreateQuery(bvector<std::shared_ptr<SelectQueryInfo const>> const&, bvector<GroupingNodeAndHandler> const& filters) const = 0;
    virtual void _ApplyFilter(ComplexQueryBuilderPtr& query, SelectQueryInfo const& selectInfo, NavNodeCR filteringNode) const {}

public:
    virtual ~SelectQueryHandler() {}
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    static std::unique_ptr<SelectQueryHandler const> Create(GroupingHandler const& groupingHandler, NavigationQueryBuilderParameters const& queryBuilderParams,
        NavNodeCP parentNode, NavNodeCP parentInstanceNode, ChildNodeSpecificationCR specification, Utf8StringCR specificationHash, std::function<uint64_t()> selectContractIdAllocator);
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    static std::unique_ptr<SelectQueryHandler const> Create(GroupingHandler const& groupingHandler, GroupingResolver const& groupingResolver, std::function<uint64_t()> selectContractIdAllocator)
        {
        return Create(groupingHandler, groupingResolver.GetQueryBuilderParams(), groupingResolver.GetParentNode(),
            groupingResolver.GetParentInstanceNode(), groupingResolver.GetSpecification(), groupingResolver.GetSpecificationIdentifier(),
            selectContractIdAllocator);
        }
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    Utf8CP GetName() const {return _GetName();}
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    int GetOrderInUnion() const {return _GetOrderInUnion();}
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    AcceptResult Accept(SelectQueryInfo& info) const {return _Accept(info);}
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    PresentationQueryBuilderPtr CreateQuery(bvector<std::shared_ptr<SelectQueryInfo const>> const& infos, bvector<GroupingNodeAndHandler> const& filters) const {return _CreateQuery(infos, filters);}
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    void ApplyFilter(ComplexQueryBuilderPtr& query, SelectQueryInfo const& selectInfo, NavNodeCR filteringNode) const {_ApplyFilter(query, selectInfo, filteringNode);}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ECInstanceSelectQueryHandler : SelectQueryHandler
{
private:
    mutable std::unique_ptr<bvector<SortingRuleCP>> m_sortingRules;

private:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    bvector<SortingRuleCP> const& GetSortingRulesForSpecification() const
        {
        if (nullptr == m_sortingRules)
            {
            IRulesPreprocessor::AggregateCustomizationRuleParameters params(GetParentInstanceNode(), GetSpecificationIdentifier());
            m_sortingRules = std::make_unique<bvector<SortingRuleCP>>(GetQueryBuilderParams().GetRulesPreprocessor().GetSortingRules(params));
            }
        return *m_sortingRules;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    void GroupSelectInfosBySortingType(bvector<std::shared_ptr<SelectQueryInfo const>> const& selectInfos, bvector<std::shared_ptr<SelectQueryInfo const>>& notSorted,
        bvector<std::shared_ptr<SelectQueryInfo const>>& labelSorted, bvector<bpair<std::shared_ptr<SelectQueryInfo const>, bvector<ClassSortingRule>>>& rulesSorted) const
        {
        auto scope = Diagnostics::Scope::Create("Determine sorting");
        for (auto const& selectInfo : selectInfos)
            {
            auto classScope = Diagnostics::Scope::Create(Utf8PrintfString("Handle class `%s`", selectInfo->GetSelectClass().GetClass().GetFullName()));

            if (GetSpecification().GetDoNotSort())
                {
                DIAGNOSTICS_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, LOG_INFO, "Requested nodes to not be sorted.");
                notSorted.push_back(selectInfo);
                continue;
                }

            bvector<ClassSortingRule> sortingRules = QueryBuilderHelpers::GetClassSortingRules(GetSortingRulesForSpecification(), selectInfo->GetSelectClass(), selectInfo->GetRelatedInstancePaths(), GetQueryBuilderParams().GetSchemaHelper());
            if (sortingRules.empty())
                {
                DIAGNOSTICS_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, LOG_INFO, "No sorting rules apply.");
                labelSorted.push_back(selectInfo);
                continue;
                }

            // if the rule of highest priority tells to not sort, we don't care about other rules
            if (sortingRules.front().GetRule().GetDoNotSort())
                {
                DIAGNOSTICS_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, LOG_INFO, "Highest priority sorting rule requests nodes to not be sorted.");
                notSorted.push_back(selectInfo);
                continue;
                }

            DIAGNOSTICS_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, LOG_INFO, Utf8PrintfString("Found %" PRIu64 " sorting rules that apply", (uint64_t)sortingRules.size()));
            rulesSorted.push_back(make_bpair(selectInfo, sortingRules));
            }
        }

    /*---------------------------------------------------------------------------------**//**
    * Note: we need to have inner query based on ECInstanceNodesQueryContract so we can filter by ECInstanceId
    * and only afterwards we can nest it into a query based on MultiECInstanceNodesQueryContract
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    void ToMultiECInstanceNodeQuery(ComplexQueryBuilderPtr& query, SelectQueryInfo const& info) const
        {
        if (query->GetNavigationResultParameters().GetResultType() == NavigationQueryResultType::MultiECInstanceNodes)
            return;

        // set to invalid so NavigationQuery doesn't attempt to set resultQuery result type to nested query result type
        query->GetNavigationResultParameters().SetResultType(NavigationQueryResultType::Invalid);

        auto const* contract = query->GetContract()->AsNavigationQueryContract();
        auto const& instanceKeysSelectQuery = contract->GetInstanceKeysSelectQuery();

        // note: we don't need to create a valid label field here because we're just wrapping another query - the valid clause is set there
        auto displayLabelField = PresentationQueryContractSimpleField::Create(MultiECInstanceNodesQueryContract::DisplayLabelFieldName, "", false);
        auto resultContract = MultiECInstanceNodesQueryContract::Create(contract->GetId(), GetSpecificationIdentifierForContract(), instanceKeysSelectQuery, &info.GetSelectClass().GetClass(), displayLabelField, false, info.GetRelatedInstancePaths());
        ComplexQueryBuilderPtr resultQuery = ComplexQueryBuilder::Create();
        resultQuery->SelectContract(*resultContract);
        resultQuery->From(*query);

        query = resultQuery;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    PresentationQueryBuilderPtr CreateQueryCommonWrapped(SelectQueryInfo const& info, bvector<GroupingNodeAndHandler> const& filters) const
        {
        auto displayLabelField = QueryBuilderHelpers::CreateDisplayLabelField(ECInstanceNodesQueryContract::DisplayLabelFieldName, GetQueryBuilderParams().GetSchemaHelper(),
            info.GetSelectClass(), nullptr, nullptr, info.GetRelatedInstancePaths(), info.GetLabelOverrideValueSpecs());
        auto instanceKeysSelectQuery = CreateQueryBase(*ECClassGroupedInstancesQueryContract::Create(), info, filters);
        auto contract = ECInstanceNodesQueryContract::Create(0, GetSpecificationIdentifierForContract(), *instanceKeysSelectQuery, &info.GetSelectClass().GetClass(), displayLabelField, info.GetRelatedInstancePaths());
        auto query = CreateQueryBase(*contract, info, filters);
        ToMultiECInstanceNodeQuery(query, info);
        return query;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    ComplexQueryBuilderPtr CreateQueryCommon(SelectQueryInfo const& info, bvector<GroupingNodeAndHandler> const& filters) const
        {
        auto displayLabelField = QueryBuilderHelpers::CreateDisplayLabelField(ECInstanceNodesQueryContract::DisplayLabelFieldName, GetQueryBuilderParams().GetSchemaHelper(),
            info.GetSelectClass(), nullptr, nullptr, info.GetRelatedInstancePaths(), info.GetLabelOverrideValueSpecs());
        auto instanceKeysSelectQuery = CreateQueryBase(*ECClassGroupedInstancesQueryContract::Create(), info, filters);
        auto contract = ECInstanceNodesQueryContract::Create(0, GetSpecificationIdentifierForContract(), *instanceKeysSelectQuery, &info.GetSelectClass().GetClass(), displayLabelField, info.GetRelatedInstancePaths());
        return CreateQueryBase(*contract, info, filters);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    PresentationQueryBuilderPtr CreateRulesSortedQuery(bvector<bpair<std::shared_ptr<SelectQueryInfo const>, bvector<ClassSortingRule>>> const& rulesSorted, bvector<std::shared_ptr<SelectQueryInfo const>>& labelSorted, bvector<GroupingNodeAndHandler> const& filters) const
        {
        // union all rules-sorted queries
        PresentationQueryBuilderPtr rulesSortedQuery;
        for (auto const& rulesSortedEntry : rulesSorted)
            {
            SelectQueryInfo const& selectInfo = *rulesSortedEntry.first;
            bvector<ClassSortingRule> const& sortingRules = rulesSortedEntry.second;
            Utf8String orderByClause = QueryBuilderHelpers::CreatePropertySortingClause(sortingRules, selectInfo.GetSelectClass());
            if (orderByClause.empty())
                {
                labelSorted.push_back(rulesSortedEntry.first);
                continue;
                }

            auto query = CreateQueryCommon(selectInfo, filters);
            QueryBuilderHelpers::Order(*query, orderByClause.c_str());
            ToMultiECInstanceNodeQuery(query, selectInfo);
            QueryBuilderHelpers::SetOrUnion(rulesSortedQuery, *query);
            }
        return rulesSortedQuery;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    PresentationQueryBuilderPtr CreateLabelSortedQuery(bvector<std::shared_ptr<SelectQueryInfo const>> const& labelSorted, bvector<GroupingNodeAndHandler> const& filters) const
        {
        // union all display label-sorted queries
        PresentationQueryBuilderPtr labelSortedUnion;
        for (auto const& selectInfo : labelSorted)
            QueryBuilderHelpers::SetOrUnion(labelSortedUnion, *CreateQueryCommonWrapped(*selectInfo, filters));

        if (labelSortedUnion.IsValid())
            {
            // create a single label-sorted query
            static Utf8PrintfString const sortedDisplayLabel("%s([%s]) IS NULL, %s([%s])", FUNCTION_NAME_GetSortingValue, ECInstanceNodesQueryContract::DisplayLabelFieldName, FUNCTION_NAME_GetSortingValue, ECInstanceNodesQueryContract::DisplayLabelFieldName);

            labelSortedUnion = QueryBuilderHelpers::CreateNestedQueryIfNecessary(*labelSortedUnion, {ECInstanceNodesQueryContract::DisplayLabelFieldName});
            QueryBuilderHelpers::Order(*labelSortedUnion, sortedDisplayLabel.c_str());
            }
        return labelSortedUnion;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    PresentationQueryBuilderPtr CreateUnsortedQuery(bvector<std::shared_ptr<SelectQueryInfo const>> const& notSorted, bvector<GroupingNodeAndHandler> const& filters) const
        {
        // union all not sorted queries
        PresentationQueryBuilderPtr notSortedQuery;
        for (auto const& selectInfo : notSorted)
            QueryBuilderHelpers::SetOrUnion(notSortedQuery, *CreateQueryCommonWrapped(*selectInfo, filters));
        return notSortedQuery;
        }

protected:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    Utf8CP _GetName() const override {return "ECInstances";}

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    int _GetOrderInUnion() const override {return 0;}

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    AcceptResult _Accept(SelectQueryInfo&) const override {return AcceptResult::CreateFullyAccepted();}

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    PresentationQueryBuilderPtr _CreateQuery(bvector<std::shared_ptr<SelectQueryInfo const>> const& selectInfos, bvector<GroupingNodeAndHandler> const& filters) const override
        {
        auto scope = Diagnostics::Scope::Create("Create ECInstance nodes query");

        bvector<std::shared_ptr<SelectQueryInfo const>> notSorted;
        bvector<std::shared_ptr<SelectQueryInfo const>> labelSorted;
        bvector<bpair<std::shared_ptr<SelectQueryInfo const>, bvector<ClassSortingRule>>> rulesSorted;
        GroupSelectInfosBySortingType(selectInfos, notSorted, labelSorted, rulesSorted);

        auto rulesSortedQuery = CreateRulesSortedQuery(rulesSorted, labelSorted, filters);
        auto labelSortedQuery = CreateLabelSortedQuery(labelSorted, filters);
        auto unsortedQuery = CreateUnsortedQuery(notSorted, filters);

        if (labelSortedQuery.IsValid() && (rulesSortedQuery.IsValid() || unsortedQuery.IsValid()))
            {
            // the queries must be nested in order to union them ordered
            labelSortedQuery = QueryBuilderHelpers::CreateNestedQuery(*labelSortedQuery);
            }

        // union all queries
        PresentationQueryBuilderPtr unionQuery = rulesSortedQuery;
        if (labelSortedQuery.IsValid())
            QueryBuilderHelpers::SetOrUnion(unionQuery, *labelSortedQuery);
        if (unsortedQuery.IsValid())
            QueryBuilderHelpers::SetOrUnion(unionQuery, *unsortedQuery);
        return unionQuery;
        }

public:
    ECInstanceSelectQueryHandler(NavigationQueryBuilderParameters const& params, NavNodeCP parentNode, NavNodeCP parentInstanceNode, ChildNodeSpecification const& specification,
        Utf8StringCR specificationHash, std::function<uint64_t()> selectContractIdAllocator)
        : SelectQueryHandler(params, parentNode, parentInstanceNode, specification, specificationHash, selectContractIdAllocator)
        {}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
template<class TGroupingHandler = GroupingHandler>
struct GroupingSelectQueryHandler : SelectQueryHandler
{
private:
    TGroupingHandler const& m_groupingHandler;
protected:
    Utf8CP _GetName() const override {return m_groupingHandler.GetName();}
    virtual PresentationQueryBuilderPtr _CreateGroupedQuery(bvector<std::shared_ptr<SelectQueryInfo const>> const& selectInfos, bvector<GroupingNodeAndHandler> const& filters) const = 0;
    PresentationQueryBuilderPtr _CreateQuery(bvector<std::shared_ptr<SelectQueryInfo const>> const& selectInfos, bvector<GroupingNodeAndHandler> const& filters) const override
        {
        return _CreateGroupedQuery(selectInfos, filters);
        }
public:
    GroupingSelectQueryHandler(TGroupingHandler const& groupingHandler, NavigationQueryBuilderParameters const& params, NavNodeCP parentNode, NavNodeCP parentInstanceNode, ChildNodeSpecification const& specification,
        Utf8StringCR specificationHash, std::function<uint64_t()> selectContractIdAllocator)
        : SelectQueryHandler(params, parentNode, parentInstanceNode, specification, specificationHash, selectContractIdAllocator), m_groupingHandler(groupingHandler)
        {}
    TGroupingHandler const& GetGroupingHandler() const {return m_groupingHandler;}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ClassGroupingSelectQueryHandler : GroupingSelectQueryHandler<>
{
protected:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    int _GetOrderInUnion() const override {return 4;}
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    AcceptResult _Accept(SelectQueryInfo& selectInfo) const override
        {
        ECClassCP parentGroupingClass = (GetParentNode() && GetParentNode()->GetKey()->AsECClassGroupingNodeKey())
            ? &GetParentNode()->GetKey()->AsECClassGroupingNodeKey()->GetECClass() : nullptr;
        if (!parentGroupingClass)
            {
            DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, "Did not find parent grouping class - accept the select fully.");
            return AcceptResult::CreateFullyAccepted();
            }

        // note: we don't want to create another grouping node for a class that is used for the parent node, so
        // we split the select into 2:
        // - one selects everything EXCEPT instances of the parent grouping class (non-polymorphically), and that gets grouped by class
        // - second selects only instances of parent grouping class (non-polymorphically), and that gets skipped by this query
        //   handler, allowing other, lower priority, query handlers to do their job on it
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, Utf8PrintfString("Found parent grouping class `%s` - accept the select partially.", parentGroupingClass->GetFullName()));
        auto ungroupedSelectInfo = std::make_shared<SelectQueryInfo>(selectInfo);
        ungroupedSelectInfo->SetSelectClass(SelectClass<ECClass>(*parentGroupingClass, selectInfo.GetSelectClass().GetAlias(), false));
        selectInfo.GetSelectClass().GetDerivedExcludedClasses().push_back(SelectClass<ECClass>(*parentGroupingClass, "", false));

        return AcceptResult::CreatePartiallyAccepted(ungroupedSelectInfo);
        }
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    PresentationQueryBuilderPtr _CreateGroupedQuery(bvector<std::shared_ptr<SelectQueryInfo const>> const& selectInfos, bvector<GroupingNodeAndHandler> const& filters) const override
        {
        auto scope = Diagnostics::Scope::Create("Create class grouping query");

        auto groupedInstanceKeysContract = ECClassGroupedInstancesQueryContract::Create();
        PresentationQueryBuilderPtr instanceKeysSelectQuery;
        for (auto const& selectInfo : selectInfos)
            QueryBuilderHelpers::SetOrUnion(instanceKeysSelectQuery, *CreateQueryBase(*groupedInstanceKeysContract, *selectInfo, filters));

        if (instanceKeysSelectQuery.IsNull())
            return nullptr;

        auto contract = ECClassGroupingNodesQueryContract::Create(AllocateContractId(), GetSpecificationIdentifierForContract(), *instanceKeysSelectQuery);
        PresentationQueryBuilderPtr unionQuery;
        for (auto const& selectInfo : selectInfos)
            QueryBuilderHelpers::SetOrUnion(unionQuery, *CreateQueryBase(*contract, *selectInfo, filters));

        if (unionQuery.IsNull())
            return nullptr;

        ComplexQueryBuilderPtr groupedQuery = QueryBuilderHelpers::CreateComplexNestedQueryIfNecessary(*unionQuery, contract->GetGroupingAliases());
        groupedQuery->GroupByContract(*contract);
        unionQuery = groupedQuery;

        if (!GetSpecification().GetDoNotSort())
            {
            DIAGNOSTICS_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, LOG_INFO, "Sorting by label.");
            static Utf8PrintfString const sortedDisplayLabel("%s([%s]) IS NULL, %s([%s])", FUNCTION_NAME_GetSortingValue, ECClassGroupingNodesQueryContract::DisplayLabelFieldName, FUNCTION_NAME_GetSortingValue, ECClassGroupingNodesQueryContract::DisplayLabelFieldName);
            unionQuery = QueryBuilderHelpers::CreateNestedQueryIfNecessary(*unionQuery, {ECClassGroupingNodesQueryContract::DisplayLabelFieldName});
            QueryBuilderHelpers::Order(*unionQuery, sortedDisplayLabel.c_str());
            }

        return unionQuery;
        }

public:
    ClassGroupingSelectQueryHandler(GroupingHandler const& groupingHandler, NavigationQueryBuilderParameters const& params, NavNodeCP parentNode, NavNodeCP parentInstanceNode, ChildNodeSpecification const& specification,
        Utf8StringCR specificationHash, std::function<uint64_t()> selectContractIdAllocator)
        : GroupingSelectQueryHandler(groupingHandler, params, parentNode, parentInstanceNode, specification, specificationHash, selectContractIdAllocator)
        {}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct LabelGroupingSelectQueryHandler : GroupingSelectQueryHandler<>
{
protected:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    int _GetOrderInUnion() const override {return 2;}

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    AcceptResult _Accept(SelectQueryInfo&) const override {return AcceptResult::CreateFullyAccepted();}

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    PresentationQueryBuilderPtr _CreateGroupedQuery(bvector<std::shared_ptr<SelectQueryInfo const>> const& selectInfos, bvector<GroupingNodeAndHandler> const& filters) const override
        {
        auto scope = Diagnostics::Scope::Create("Create label grouping query");

        uint64_t groupingContractId = AllocateContractId();
        PresentationQueryBuilderPtr instanceKeysSelectQuery;
        PresentationQueryBuilderPtr unionQuery;
        for (auto const& selectInfo : selectInfos)
            {
            auto displayLabelField = QueryBuilderHelpers::CreateDisplayLabelField(DisplayLabelGroupingNodesQueryContract::DisplayLabelFieldName, GetQueryBuilderParams().GetSchemaHelper(),
                selectInfo->GetSelectClass(), nullptr, nullptr, selectInfo->GetRelatedInstancePaths(), selectInfo->GetLabelOverrideValueSpecs());

            auto instanceKeysSelectQueryContract = DisplayLabelGroupedInstancesQueryContract::Create(displayLabelField);
            ComplexQueryBuilderPtr thisInstanceKeysSelectQuery = CreateQueryBase(*instanceKeysSelectQueryContract, *selectInfo, filters);
            QueryBuilderHelpers::SetOrUnion(instanceKeysSelectQuery, *thisInstanceKeysSelectQuery);

            auto contract = DisplayLabelGroupingNodesQueryContract::Create(AllocateContractId(), GetSpecificationIdentifierForContract(), *thisInstanceKeysSelectQuery, &selectInfo->GetSelectClass().GetClass(), displayLabelField);
            QueryBuilderHelpers::SetOrUnion(unionQuery, *CreateQueryBase(*contract, *selectInfo, filters));
            }

        if (unionQuery.IsNull() || instanceKeysSelectQuery.IsNull())
            return nullptr;

        auto groupedQueryLabelField = PresentationQueryContractSimpleField::Create(DisplayLabelGroupingNodesQueryContract::DisplayLabelFieldName, "", false);
        groupedQueryLabelField->SetGroupingClause(QueryBuilderHelpers::CreateDisplayLabelValueClause(groupedQueryLabelField->GetName()));
        groupedQueryLabelField->SetResultType(PresentationQueryFieldType::LabelDefinition);

        NavigationQueryContractPtr groupingContract = DisplayLabelGroupingNodesQueryContract::Create(groupingContractId, GetSpecificationIdentifierForContract(), *instanceKeysSelectQuery, nullptr, groupedQueryLabelField);
        unionQuery = &ComplexQueryBuilder::Create()
            ->SelectContract(*groupingContract)
            .From(ComplexQueryBuilder::Create()->SelectContract(*groupingContract).From(*unionQuery))
            .GroupByContract(*groupingContract);

        if (!GetSpecification().GetDoNotSort())
            {
            DIAGNOSTICS_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, LOG_INFO, "Sort by label.");
            static Utf8PrintfString const sortedDisplayLabel("%s([%s]) IS NULL, %s([%s])", FUNCTION_NAME_GetSortingValue, DisplayLabelGroupingNodesQueryContract::DisplayLabelFieldName, FUNCTION_NAME_GetSortingValue, DisplayLabelGroupingNodesQueryContract::DisplayLabelFieldName);
            unionQuery = QueryBuilderHelpers::CreateNestedQueryIfNecessary(*unionQuery, {DisplayLabelGroupingNodesQueryContract::DisplayLabelFieldName});
            QueryBuilderHelpers::Order(*unionQuery, sortedDisplayLabel.c_str());
            }

        unionQuery->GetNavigationResultParameters().GetNavNodeExtendedDataR().SetHideIfOnlyOneChild(true);
        DIAGNOSTICS_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, LOG_INFO, "Will hide nodes if they have only one child.");

        return unionQuery;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    void _ApplyFilter(ComplexQueryBuilderPtr& query, SelectQueryInfo const& selectInfo, NavNodeCR filteringNode) const override
        {
        auto scope = Diagnostics::Scope::Create(Utf8PrintfString("Filter instances based on parent label grouping node. Label: `%s`", filteringNode.GetLabelDefinition().GetDisplayValue().c_str()));

        auto key = filteringNode.GetKey()->AsLabelGroupingNodeKey();
        if (!key)
            DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Hierarchies, "Node is not a label grouping node");

        // if the key stores grouped instance keys, we want to filter by those keys - it's going to be much more
        // efficient compared to filtering by label
        if (key->GetGroupedInstanceKeys())
            {
            auto ids = ContainerHelpers::TransformContainer<bvector<ECInstanceId>>(*key->GetGroupedInstanceKeys(), [](auto const& key){return key.GetInstanceId();});
            ValuesFilteringHelper filteringHelper(ids);
            PresentationQueryContractFieldCPtr ecInstanceIdField = query->GetContract()->GetField(ECInstanceNodesQueryContract::ECInstanceIdFieldName);
            query->Where(filteringHelper.Create(ecInstanceIdField->GetSelectClause(query->GetSelectPrefix()).GetClause().c_str()));
            }
        else
            {
            auto displayLabelField = QueryBuilderHelpers::CreateDisplayLabelField(DisplayLabelGroupingNodesQueryContract::DisplayLabelFieldName, GetQueryBuilderParams().GetSchemaHelper(),
                selectInfo.GetSelectClass(), nullptr, nullptr, selectInfo.GetRelatedInstancePaths(), selectInfo.GetLabelOverrideValueSpecs());
            auto displayLabelSelectClause = displayLabelField->GetSelectClause(selectInfo.GetSelectClass().GetAlias().c_str());
            Utf8PrintfString whereClause("%s = ?", displayLabelSelectClause.GetClause().c_str());
            BoundQueryValuesList whereClauseBindings(displayLabelSelectClause.GetBindings());
            whereClauseBindings.push_back(std::make_unique<BoundQueryECValue>(ECValue(filteringNode.GetLabelDefinition().ToJsonString().c_str())));
            query->Where(whereClause.c_str(), whereClauseBindings);
            }
        }

public:
    LabelGroupingSelectQueryHandler(GroupingHandler const& groupingHandler, NavigationQueryBuilderParameters const& params, NavNodeCP parentNode, NavNodeCP parentInstanceNode, ChildNodeSpecification const& specification,
        Utf8StringCR specificationHash, std::function<uint64_t()> selectContractIdAllocator)
        : GroupingSelectQueryHandler(groupingHandler, params, parentNode, parentInstanceNode, specification, specificationHash, selectContractIdAllocator)
        {}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct BaseClassGroupingSelectQueryHandler : GroupingSelectQueryHandler<BaseClassGroupingHandler>
{
protected:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    int _GetOrderInUnion() const override {return 5;}
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    AcceptResult _Accept(SelectQueryInfo& info) const override
        {
        // The handler always accepts the given SelectQueryInfo, but returns partial accepted status
        // with a handler criteria to allow other base class grouping handlers to handle the same
        // SelectQueryInfo, but only if they group by a class that doesn't derive from this handler's class.
        auto result = AcceptResult::CreatePartiallyAccepted(std::make_unique<SelectQueryInfo>(info));
        result.SetPartialAcceptCriteria([&](SelectQueryHandler const& candidate)
            {
            // can compare as pointers, since names are static
            if (candidate.GetName() != GetName())
                return false;

            // we know the candidate is always a BaseClassGroupingSelectQueryHandler if the above check passed
            auto const& candidateGroupingHandler = static_cast<BaseClassGroupingSelectQueryHandler const*>(&candidate)->GetGroupingHandler();
            if (!candidateGroupingHandler.GetTargetClass() || !GetGroupingHandler().GetTargetClass())
                return false;

            return !candidateGroupingHandler.GetTargetClass()->Is(GetGroupingHandler().GetTargetClass());
            });
        return result;
        }
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    PresentationQueryBuilderPtr _CreateGroupedQuery(bvector<std::shared_ptr<SelectQueryInfo const>> const& selectInfos, bvector<GroupingNodeAndHandler> const& filters) const override
        {
        auto scope = Diagnostics::Scope::Create("Create base class grouping query");

        auto groupedInstanceKeysContract = ECClassGroupedInstancesQueryContract::Create();
        PresentationQueryBuilderPtr instanceKeysSelectQuery;
        for (auto const& selectInfo : selectInfos)
            QueryBuilderHelpers::SetOrUnion(instanceKeysSelectQuery, *CreateQueryBase(*groupedInstanceKeysContract, *selectInfo, filters));

        if (instanceKeysSelectQuery.IsNull())
            return nullptr;

        auto contract = ECClassGroupingNodesQueryContract::Create(AllocateContractId(), GetSpecificationIdentifierForContract(), *instanceKeysSelectQuery, GetGroupingHandler().GetBaseECClassId(), true);
        PresentationQueryBuilderPtr unionQuery;
        for (auto const& selectInfo : selectInfos)
            QueryBuilderHelpers::SetOrUnion(unionQuery, *CreateQueryBase(*contract, *selectInfo, filters));

        if (unionQuery.IsNull())
            return nullptr;

        ComplexQueryBuilderPtr groupedQuery = QueryBuilderHelpers::CreateComplexNestedQueryIfNecessary(*unionQuery, contract->GetGroupingAliases());
        groupedQuery->GroupByContract(*contract);
        unionQuery = groupedQuery;

        if (!GetSpecification().GetDoNotSort())
            {
            DIAGNOSTICS_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, LOG_INFO, "Sorting by label.");
            static Utf8PrintfString const sortedDisplayLabel("%s([%s]) IS NULL, %s([%s])", FUNCTION_NAME_GetSortingValue, ECClassGroupingNodesQueryContract::DisplayLabelFieldName, FUNCTION_NAME_GetSortingValue, ECClassGroupingNodesQueryContract::DisplayLabelFieldName);
            unionQuery = QueryBuilderHelpers::CreateNestedQueryIfNecessary(*unionQuery, {ECClassGroupingNodesQueryContract::DisplayLabelFieldName});
            QueryBuilderHelpers::Order(*unionQuery, sortedDisplayLabel.c_str());
            }

        if (!GetGroupingHandler().GetGroupingSpecification().GetCreateGroupForSingleItem())
            {
            unionQuery->GetNavigationResultParameters().GetNavNodeExtendedDataR().SetHideIfOnlyOneChild(true);
            DIAGNOSTICS_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, LOG_INFO, "May hide grouping nodes due to 'create group for single item' flag not being set.");
            }

        return unionQuery;
        }

public:
    BaseClassGroupingSelectQueryHandler(BaseClassGroupingHandler const& groupingHandler, NavigationQueryBuilderParameters const& params, NavNodeCP parentNode, NavNodeCP parentInstanceNode, ChildNodeSpecification const& specification,
        Utf8StringCR specificationHash, std::function<uint64_t()> selectContractIdAllocator)
        : GroupingSelectQueryHandler(groupingHandler, params, parentNode, parentInstanceNode, specification, specificationHash, selectContractIdAllocator)
        {}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct PropertyGroupingSelectQueryHandler : GroupingSelectQueryHandler<PropertyGroupingHandler>
{
private:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    Utf8CP GetSelectAlias(SelectQueryInfo const& selectInfo) const
        {
        // first, check the primary select class
        if (GetGroupingHandler().AppliesForClass(selectInfo.GetSelectClass().GetClass(), selectInfo.GetSelectClass().IsSelectPolymorphic()))
            return selectInfo.GetSelectClass().GetAlias().c_str();

        // then, check the related class path in case we want to group by relationship property
        for (RelatedClass const& relatedClass : selectInfo.GetPathFromParentToSelectClass())
            {
            if (relatedClass.GetRelationship().IsValid() && GetGroupingHandler().AppliesForClass(relatedClass.GetRelationship().GetClass(), relatedClass.GetRelationship().IsSelectPolymorphic()))
                return relatedClass.GetRelationship().GetAlias().c_str();
            }

        // finally, check the additional related instance paths
        for (RelatedClassPathCR relatedInstancePath : selectInfo.GetRelatedInstancePaths())
            {
            if (!relatedInstancePath.empty() && GetGroupingHandler().AppliesForClass(relatedInstancePath.back().GetTargetClass().GetClass(), relatedInstancePath.back().GetTargetClass().IsSelectPolymorphic()))
                return relatedInstancePath.back().GetTargetClass().GetAlias().c_str();
            }

        return nullptr;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    static Utf8String CreatePropertyValueSelector(ECPropertyCR ecProperty, Utf8CP prefix)
        {
        Utf8String propertyValueSelector;
        if (ecProperty.GetIsPrimitive() && PRIMITIVETYPE_Point3d == ecProperty.GetAsPrimitiveProperty()->GetType())
            {
            propertyValueSelector.Sprintf("%s([%s].[%s].x, [%s].[%s].y, [%s].[%s].z)", FUNCTION_NAME_GetPointAsJsonString,
                prefix, ecProperty.GetName().c_str(), prefix, ecProperty.GetName().c_str(), prefix, ecProperty.GetName().c_str());
            }
        else if (ecProperty.GetIsPrimitive() && PRIMITIVETYPE_Point2d == ecProperty.GetAsPrimitiveProperty()->GetType())
            {
            propertyValueSelector.Sprintf("%s([%s].[%s].x, [%s].[%s].y)", FUNCTION_NAME_GetPointAsJsonString,
                prefix, ecProperty.GetName().c_str(), prefix, ecProperty.GetName().c_str());
            }
        else
            {
            propertyValueSelector.Sprintf("[%s].[%s]", prefix, ecProperty.GetName().c_str());
            }

        if (ecProperty.GetIsNavigation())
            propertyValueSelector.append(".[Id]");

        return propertyValueSelector;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    ComplexQueryBuilderPtr CreateSelectInfoQuery(PresentationQueryContractR contract, SelectQueryInfo const& selectInfo, bvector<GroupingNodeAndHandler> const& filters, RelatedClassCR relatedClass) const
        {
        ComplexQueryBuilderPtr query = CreateQueryBase(contract, selectInfo, filters);
        if (relatedClass.IsValid())
            {
            if (relatedClass.GetSourceClass()->IsEntityClass())
                {
                // source class is the source of the navigation property
                query->Join(relatedClass);
                }
            else
                {
                // one of the relationships is the source of the navigation property
                DIAGNOSTICS_ASSERT_SOFT(DiagnosticsCategory::Hierarchies, !selectInfo.GetPathFromParentToSelectClass().empty(), "Expected path from parent to select class to not be empty");
                RelatedClassPath joinPath = selectInfo.GetPathFromParentToSelectClass();
                joinPath.Reverse("related", true);
                for (auto iter = joinPath.begin(); iter != joinPath.end(); ++iter)
                    {
                    RelatedClassR related = *iter;
                    if (relatedClass.GetSourceClass() == &related.GetRelationship().GetClass())
                        {
                        related.GetTargetClass().SetAlias("");
                        joinPath.erase(iter + 1, joinPath.end());
                        break;
                        }
                    }
                joinPath.push_back(relatedClass);
                query->Join(joinPath);
                }
            }
        return query;
        }

protected:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    int _GetOrderInUnion() const override {return 3;}

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    AcceptResult _Accept(SelectQueryInfo&) const override {return AcceptResult::CreateFullyAccepted();}

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    PresentationQueryBuilderPtr _CreateGroupedQuery(bvector<std::shared_ptr<SelectQueryInfo const>> const& selectInfos, bvector<GroupingNodeAndHandler> const& filters) const override
        {
        auto scope = Diagnostics::Scope::Create("Create property grouping query");

        if (selectInfos.empty())
            {
            DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, "Not creating property grouping query as there are not classes to group.");
            return nullptr;
            }

        ECPropertyCP groupingProperty = GetGroupingHandler().GetGroupingProperty();
        if (nullptr == groupingProperty)
            {
            DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, "Failed to create the query as there's no grouping property.");
            return nullptr;
            }

        RelatedClass foreignKeyClass = GetQueryBuilderParams().GetSchemaHelper().GetForeignKeyClass(*groupingProperty);
        if (foreignKeyClass.IsValid())
            {
            foreignKeyClass.GetTargetClass().SetAlias("parentInstance");
            foreignKeyClass.GetRelationship().SetAlias(NAVIGATION_QUERY_BUILDER_NAV_CLASS_ALIAS(foreignKeyClass.GetRelationship().GetClass()));
            DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, "Grouping by navigation property.");
            }

        uint64_t groupingContractId = AllocateContractId();
        PresentationQueryBuilderPtr instanceKeysSelectQuery;
        PresentationQueryBuilderPtr unionQuery;
        for (auto const& selectInfo : selectInfos)
            {
            Utf8CP selectAlias = GetSelectAlias(*selectInfo);
            SelectClass<ECClass> selectClass(*GetGroupingHandler().GetTargetClass(), selectAlias);
            Utf8String propertyValueSelector = CreatePropertyValueSelector(*groupingProperty, selectAlias);

            auto instanceKeysSelectQueryContract = ECPropertyGroupedInstancesQueryContract::Create(propertyValueSelector);
            ComplexQueryBuilderPtr thisInstanceKeysSelectQuery = CreateSelectInfoQuery(*instanceKeysSelectQueryContract, *selectInfo, filters, foreignKeyClass);
            QueryBuilderHelpers::SetOrUnion(instanceKeysSelectQuery, *thisInstanceKeysSelectQuery);

            auto contract = ECPropertyGroupingNodesQueryContract::Create(AllocateContractId(), GetSpecificationIdentifierForContract(), *thisInstanceKeysSelectQuery, selectClass, *groupingProperty,
                GetGroupingHandler().GetGroupingSpecification(), foreignKeyClass.IsValid() ? &foreignKeyClass.GetTargetClass() : nullptr);
            ComplexQueryBuilderPtr query = CreateSelectInfoQuery(*contract, *selectInfo, filters, foreignKeyClass);
            QueryBuilderHelpers::SetOrUnion(unionQuery, *query);
            }

        if (unionQuery.IsNull() || instanceKeysSelectQuery.IsNull())
            return nullptr;

        auto groupingContract = ECPropertyGroupingNodesQueryContract::Create(groupingContractId, GetSpecificationIdentifierForContract(),
            *instanceKeysSelectQuery, *groupingProperty, GetGroupingHandler().GetGroupingSpecification());
        unionQuery = &ComplexQueryBuilder::Create()
            ->SelectContract(*groupingContract)
            .From(ComplexQueryBuilder::Create()->SelectContract(*groupingContract).From(*unionQuery))
            .GroupByContract(*groupingContract);

        if (!GetSpecification().GetDoNotSort())
            {
            Utf8String orderByClause = nullptr;
            switch (GetGroupingHandler().GetGroupingSpecification().GetSortingValue())
                {
                case PropertyGroupingValue::PropertyValue:
                    {
                    DIAGNOSTICS_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, LOG_INFO, "Requested sorting by property raw value.");
                    PrimitiveECPropertyCP primitiveGroupingProperty = groupingProperty->GetAsPrimitiveProperty();
                    ECEnumerationCP enumeration = primitiveGroupingProperty ? primitiveGroupingProperty->GetEnumeration() : nullptr;
                    if (primitiveGroupingProperty && (PRIMITIVETYPE_String == primitiveGroupingProperty->GetType() || nullptr != enumeration))
                        {
                        orderByClause = Utf8String(FUNCTION_NAME_GetSortingValue).append("(");
                        if (nullptr != enumeration)
                            {
                            orderByClause.append(FUNCTION_NAME_GetECEnumerationValue).append("('").append(enumeration->GetSchema().GetName()).append("',");
                            orderByClause.append("'").append(enumeration->GetName()).append("',");
                            }
                        orderByClause.append(QueryHelpers::Wrap(ECPropertyGroupingNodesQueryContract::GroupingValueFieldName));
                        if (nullptr != enumeration)
                            orderByClause.append(")");
                        orderByClause.append(")");
                        }
                    else
                        orderByClause = QueryHelpers::Wrap(ECPropertyGroupingNodesQueryContract::GroupingValueFieldName);
                    unionQuery = QueryBuilderHelpers::CreateNestedQueryIfNecessary(*unionQuery, {ECPropertyGroupingNodesQueryContract::GroupingValueFieldName});
                    break;
                    }
                case PropertyGroupingValue::DisplayLabel:
                    {
                    DIAGNOSTICS_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, LOG_INFO, "Requested sorting by property display value.");
                    orderByClause = Utf8PrintfString("%s([%s]) IS NULL, %s([%s])", FUNCTION_NAME_GetSortingValue, ECPropertyGroupingNodesQueryContract::DisplayLabelFieldName, FUNCTION_NAME_GetSortingValue, ECPropertyGroupingNodesQueryContract::DisplayLabelFieldName);
                    unionQuery = QueryBuilderHelpers::CreateNestedQueryIfNecessary(*unionQuery, {ECPropertyGroupingNodesQueryContract::DisplayLabelFieldName});
                    break;
                    }
                default:
                    DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Hierarchies, Utf8PrintfString("Unhandled sorting value type: %d", (int)GetGroupingHandler().GetGroupingSpecification().GetSortingValue()))
                }
            if (!orderByClause.empty())
                QueryBuilderHelpers::Order(*unionQuery, orderByClause.c_str());
            }
        else
            {
            DIAGNOSTICS_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, LOG_INFO, "Not sorting due to 'do not sort' flag.");
            }

        if (!GetGroupingHandler().GetGroupingSpecification().GetCreateGroupForSingleItem())
            {
            unionQuery->GetNavigationResultParameters().GetNavNodeExtendedDataR().SetHideIfOnlyOneChild(true);
            DIAGNOSTICS_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, LOG_INFO, "May hide property grouping nodes due to 'hide if only one child' flag.");
            }

        if (!GetGroupingHandler().GetGroupingSpecification().GetCreateGroupForUnspecifiedValues() && GetGroupingHandler().GetGroupingSpecification().GetRanges().empty())
            {
            unionQuery->GetNavigationResultParameters().GetNavNodeExtendedDataR().SetHideIfGroupingValueNotSpecified(true);
            DIAGNOSTICS_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, LOG_INFO, "Will hide property grouping node that groups by unspecified values.");
            }

        NavigationQueryExtendedData(*unionQuery).AddRangesData(*groupingProperty, GetGroupingHandler().GetGroupingSpecification());

        return unionQuery;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    void _ApplyFilter(ComplexQueryBuilderPtr& query, SelectQueryInfo const& selectInfo, NavNodeCR filteringNode) const override
        {
        auto scope = Diagnostics::Scope::Create(Utf8PrintfString("Filter class `%s` instances based on parent property grouping node.", selectInfo.GetSelectClass().GetClass().GetFullName()));

        NavNodeExtendedData extendedData(filteringNode);
        if (!extendedData.HasECClassId() || !extendedData.HasPropertyName() || !(extendedData.HasPropertyValues() || extendedData.HasPropertyValueRangeIndexes()))
            DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Hierarchies, "Property grouping node doesn't have all required parameters");

        ECClassCP ecPropertyClass = GetQueryBuilderParams().GetSchemaHelper().GetECClass(extendedData.GetECClassId());
        if (nullptr == ecPropertyClass)
            DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Hierarchies, Utf8PrintfString("Property grouping node points to invalid ECClass: %" PRIu64, extendedData.GetECClassId().GetValue()));

        ECPropertyCP ecProperty = ecPropertyClass->GetPropertyP(extendedData.GetPropertyName());
        if (nullptr == ecProperty)
            DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Hierarchies, Utf8PrintfString("Property grouping node points to invalid ECProperty: '%s.%s'", ecPropertyClass->GetFullName(), extendedData.GetPropertyName()));

        Utf8CP prefix = GetSelectAlias(selectInfo);
        if (nullptr == prefix)
            prefix = query->GetSelectPrefix();

        Utf8String propertyValueSelector = CreatePropertyValueSelector(*ecProperty, prefix);

        query->Where(QueryBuilderHelpers::CreatePropertyGroupFilteringClause(*ecProperty, propertyValueSelector, GetGroupingHandler().GetGroupingSpecification(),
            extendedData.HasPropertyValueRangeIndexes() ? extendedData.GetPropertyValueRangeIndexesJson() : extendedData.GetPropertyValues()));
        }

public:
    PropertyGroupingSelectQueryHandler(PropertyGroupingHandler const& groupingHandler, NavigationQueryBuilderParameters const& params, NavNodeCP parentNode, NavNodeCP parentInstanceNode, ChildNodeSpecification const& specification,
        Utf8StringCR specificationHash, std::function<uint64_t()> selectContractIdAllocator)
        : GroupingSelectQueryHandler(groupingHandler, params, parentNode, parentInstanceNode, specification, specificationHash, selectContractIdAllocator)
        {}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct SameLabelGroupingSelectQueryHandler : GroupingSelectQueryHandler<SameLabelInstanceGroupingHandler>
{
protected:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    int _GetOrderInUnion() const override {return 1;}

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    AcceptResult _Accept(SelectQueryInfo& info) const override
        {
        // enforce `MultiECInstanceNodesQueryContract` for same-label-grouped nodes and return 'Reject' so the given `SelectQueryInfo`
        // gets handled by `ECInstanceSelectQueryHandler`
        info.SetForcedGroupingContractFactory([this](SelectQueryInfo const& selectInfo, bvector<GroupingNodeAndHandler> const& filters)
            {
            auto displayLabelField = QueryBuilderHelpers::CreateDisplayLabelField(MultiECInstanceNodesQueryContract::DisplayLabelFieldName, GetQueryBuilderParams().GetSchemaHelper(),
                selectInfo.GetSelectClass(), nullptr, nullptr, selectInfo.GetRelatedInstancePaths(), selectInfo.GetLabelOverrideValueSpecs());
            SelectQueryInfo infoNoForceGrouping(selectInfo);
            infoNoForceGrouping.SetForcedGroupingContractFactory(nullptr);
            ComplexQueryBuilderPtr instanceKeysSelectQuery = CreateQueryBase(*ECClassGroupedInstancesQueryContract::Create(), infoNoForceGrouping, filters);
            return MultiECInstanceNodesQueryContract::Create(AllocateContractId(), GetSpecificationIdentifierForContract(), *instanceKeysSelectQuery, &selectInfo.GetSelectClass().GetClass(), displayLabelField, true, selectInfo.GetRelatedInstancePaths());
            });
        return AcceptResult::CreateRejected();
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    PresentationQueryBuilderPtr _CreateGroupedQuery(bvector<std::shared_ptr<SelectQueryInfo const>> const& selectInfos, bvector<GroupingNodeAndHandler> const& filters) const override
        {
        DIAGNOSTICS_ASSERT_SOFT(DiagnosticsCategory::Hierarchies, false, "SameLabelGroupingSelectQueryHandler doesn't accept any select infos, so this method should never be called");
        return nullptr;
        }

public:
    SameLabelGroupingSelectQueryHandler(SameLabelInstanceGroupingHandler const& groupingHandler, NavigationQueryBuilderParameters const& params, NavNodeCP parentNode, NavNodeCP parentInstanceNode, ChildNodeSpecification const& specification,
        Utf8StringCR specificationHash, std::function<uint64_t()> selectContractIdAllocator)
        : GroupingSelectQueryHandler(groupingHandler, params, parentNode, parentInstanceNode, specification, specificationHash, selectContractIdAllocator)
        {}
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::unique_ptr<SelectQueryHandler const> SelectQueryHandler::Create(GroupingHandler const& groupingHandler, NavigationQueryBuilderParameters const& queryBuilderParams,
    NavNodeCP parentNode, NavNodeCP parentInstanceNode, ChildNodeSpecificationCR specification, Utf8StringCR specificationHash, std::function<uint64_t()> selectContractIdAllocator)
    {
    switch (groupingHandler.GetType())
        {
        case GroupingType::Class:
            return std::make_unique<ClassGroupingSelectQueryHandler>(groupingHandler, queryBuilderParams, parentNode, parentInstanceNode, specification, specificationHash, selectContractIdAllocator);
        case GroupingType::DisplayLabel:
            return std::make_unique<LabelGroupingSelectQueryHandler>(groupingHandler, queryBuilderParams, parentNode, parentInstanceNode, specification, specificationHash, selectContractIdAllocator);
        case GroupingType::BaseClass:
            return std::make_unique<BaseClassGroupingSelectQueryHandler>(static_cast<BaseClassGroupingHandler const&>(groupingHandler), queryBuilderParams, parentNode, parentInstanceNode, specification, specificationHash, selectContractIdAllocator);
        case GroupingType::Property:
            return std::make_unique<PropertyGroupingSelectQueryHandler>(static_cast<PropertyGroupingHandler const&>(groupingHandler), queryBuilderParams, parentNode, parentInstanceNode, specification, specificationHash, selectContractIdAllocator);
        case GroupingType::SameLabelInstance:
            return std::make_unique<SameLabelGroupingSelectQueryHandler>(static_cast<SameLabelInstanceGroupingHandler const&>(groupingHandler), queryBuilderParams, parentNode, parentInstanceNode, specification, specificationHash, selectContractIdAllocator);
        }
    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8CP GetSelectQueryAcceptStatusStr(SelectQueryHandler::AcceptResult::Status status)
    {
    switch (status)
        {
        case SelectQueryHandler::AcceptResult::Status::AcceptFully: return "AcceptFully";
        case SelectQueryHandler::AcceptResult::Status::AcceptPartially: return "AcceptPartially";
        }
    return "Reject";
    }

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct RootQueryContext
{
private:
    GroupingResolver const& m_groupingResolver;
    std::unique_ptr<ECInstanceSelectQueryHandler> m_ecInstancesSelectHandler;
    std::unordered_map<GroupingHandler const*, std::unique_ptr<SelectQueryHandler const>> m_groupingSelectQueryHandlers;
    std::unordered_map<SelectQueryHandler const*, bvector<std::shared_ptr<SelectQueryInfo const>>> m_selectsByHandler;
    bvector<SelectQueryHandler const*> selectQueryHandlersOrder;
    bvector<GroupingNodeAndHandler> m_groupingFilters;
    uint64_t m_contractIdCounter;

private:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    void SetCommonQueryResultParameters(PresentationQueryBuilderR query) const
        {
        ChildNodeSpecificationCR specification = m_groupingResolver.GetSpecification();

        // handle hiding attributes
        if (specification.GetHideNodesInHierarchy() && ReturnsInstanceNodes(query))
            query.GetNavigationResultParameters().GetNavNodeExtendedDataR().SetHideNodesInHierarchy(true);
        if (specification.GetHideIfNoChildren())
            query.GetNavigationResultParameters().GetNavNodeExtendedDataR().SetHideIfNoChildren(true);
        if (!specification.GetHideExpression().empty())
            query.GetNavigationResultParameters().GetNavNodeExtendedDataR().SetHideExpression(specification.GetHideExpression());

        // handle HasChildren hint
        if (ChildrenHint::Unknown != specification.GetHasChildren())
            query.GetNavigationResultParameters().GetNavNodeExtendedDataR().SetChildrenHint(specification.GetHasChildren());

        if (specification.ShouldSuppressSimilarAncestorsCheck())
            query.GetNavigationResultParameters().GetNavNodeExtendedDataR().SetAllowedSimilarAncestors(MAX_ALLOWED_SIMILAR_ANCESTORS_WHEN_SUPPRESSED);

        // preserve specification ID in resulting nodes for later use
        query.GetNavigationResultParameters().SetSpecification(&m_groupingResolver.GetSpecification());
        }

public:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    RootQueryContext(GroupingResolver const& groupingResolver)
        : m_groupingResolver(groupingResolver), m_contractIdCounter(0)
        {
        m_ecInstancesSelectHandler = std::make_unique<ECInstanceSelectQueryHandler>(groupingResolver.GetQueryBuilderParams(),
            groupingResolver.GetParentNode(), groupingResolver.GetParentInstanceNode(), groupingResolver.GetSpecification(),
            groupingResolver.GetSpecificationIdentifier(), [&](){return ++m_contractIdCounter;});
        m_groupingFilters = groupingResolver.GetFilterHandlers();
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, Utf8PrintfString("Found grouping node filters: %" PRIu64, (uint64_t)m_groupingFilters.size()));
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    bool Accept(SelectQueryInfo const& selectInfo)
        {
        auto scope = Diagnostics::Scope::Create(Utf8PrintfString("Accept class `%s`", selectInfo.GetSelectClass().GetClass().GetFullName()));
        bvector<ECClassCP> selectClasses = selectInfo.CreateSelectClassList();

        // skip selects that are completely filtered-out by our filter handlers
        for (auto const& groupingFilter : m_groupingFilters)
            {
            bool isAtLeastOneClassAccepted = ContainerHelpers::Contains(selectClasses, [&](ECClassCP selectClass)
                {
                return groupingFilter.GetHandler().AppliesForClass(*selectClass, false);
                });
            if (!isAtLeastOneClassAccepted)
                {
                DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, "One of grouping filters completely filters-out the select. Skip it.");
                return false;
                }
            }

        // build a list of handlers for given select query info. the handlers are sorted by priority they should be used
        bvector<GroupingHandler const*> groupingHandlers = m_groupingResolver.GetHandlersForNextGroupingLevel(selectClasses, false);
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, Utf8PrintfString("Total grouping handlers that apply to this select: %" PRIu64, (uint64_t)groupingHandlers.size()));

        bvector<SelectQueryHandler const*> selectQueryHandlers = ContainerHelpers::TransformContainer<bvector<SelectQueryHandler const*>>(groupingHandlers, [&](auto handler)
            {
            auto iter = m_groupingSelectQueryHandlers.find(handler);
            if (m_groupingSelectQueryHandlers.end() == iter)
                iter = m_groupingSelectQueryHandlers.insert(std::make_pair(handler, SelectQueryHandler::Create(*handler, m_groupingResolver, [&](){return ++m_contractIdCounter;}))).first;
            return iter->second.get();
            });
        selectQueryHandlers.push_back(m_ecInstancesSelectHandler.get());

        // iterate over handlers and allow them to check if they can handle the given select query info
        auto currSelectInfo = std::make_shared<SelectQueryInfo>(selectInfo);
        bvector<std::function<bool(SelectQueryHandler const&)>> queryHandlersCriteria;
        SelectQueryHandler::AcceptResult::Status acceptStatus = SelectQueryHandler::AcceptResult::Status::Reject;
        for (auto const& handler : selectQueryHandlers)
            {
            bool skipHandler = false;
            for (auto const& criteria : queryHandlersCriteria)
                {
                if (!criteria(*handler))
                    {
                    DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, Utf8PrintfString("Query handler `%s` skipped - it doesn't match the criteria", handler->GetName()));
                    skipHandler = true;
                    break;
                    }
                }
            if (skipHandler)
                continue;

            SelectQueryHandler::AcceptResult result = handler->Accept(*currSelectInfo);
            DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, Utf8PrintfString("Accept result from handler `%s`: `%s`", handler->GetName(), GetSelectQueryAcceptStatusStr(result.GetStatus())));

            if (result.GetStatus() > acceptStatus)
                acceptStatus = result.GetStatus();

            if (result.GetPartialAcceptCriteria())
                queryHandlersCriteria.push_back(result.GetPartialAcceptCriteria());

            auto selectHandlerIter = m_selectsByHandler.find(handler);
            if (m_selectsByHandler.end() == selectHandlerIter)
                {
                selectHandlerIter = m_selectsByHandler.insert(std::make_pair(handler, bvector<std::shared_ptr<SelectQueryInfo const>>())).first;
                selectQueryHandlersOrder.push_back(handler);
                }

            if (result.GetStatus() == SelectQueryHandler::AcceptResult::Status::AcceptFully)
                {
                selectHandlerIter->second.push_back(currSelectInfo);
                break;
                }
            if (result.GetStatus() == SelectQueryHandler::AcceptResult::Status::AcceptPartially)
                {
                selectHandlerIter->second.push_back(currSelectInfo);
                currSelectInfo = result.GetPartialQueryInfo();
                continue;
                }
            }

        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, Utf8PrintfString("Overall accept result: `%s`", GetSelectQueryAcceptStatusStr(acceptStatus)));
        return acceptStatus != SelectQueryHandler::AcceptResult::Status::Reject;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    bvector<PresentationQueryBuilderPtr> GetQueries() const
        {
        auto scope = Diagnostics::Scope::Create("Create queries");

        bvector<bpair<SelectQueryHandler const*, bvector<std::shared_ptr<SelectQueryInfo const>>>> orderedSelects;
        ContainerHelpers::TransformContainer(orderedSelects, selectQueryHandlersOrder, [&](auto const& handler)
            {
            return make_bpair(handler, m_selectsByHandler.find(handler)->second);
            });
        std::stable_sort(orderedSelects.begin(), orderedSelects.end(), [](auto const& lhs, auto const& rhs){return lhs.first->GetOrderInUnion() > rhs.first->GetOrderInUnion();});
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, Utf8PrintfString("Total select handlers: %" PRIu64, (uint64_t)orderedSelects.size()));

        bvector<PresentationQueryBuilderPtr> queries;
        for (auto const& entry : orderedSelects)
            {
            if (entry.second.empty())
                continue;

            PresentationQueryBuilderPtr query = entry.first->CreateQuery(entry.second, m_groupingFilters);
            if (query.IsNull())
                {
                DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, "Select handler returned NULL query - skip.");
                continue;
                }
            DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, Utf8PrintfString("Created query: %s", query->GetQuery()->GetQueryString().c_str()));

            NavigationQueryResultType resultType = query->GetNavigationResultParameters().GetResultType();
            if (queries.size() > 0 && queries.back()->GetNavigationResultParameters().GetResultType() == resultType)
                {
                QueryBuilderHelpers::SetOrUnion(queries.back(), *query);
                DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, "Unioned to previous query of the same type.");
                }
            else
                {
                queries.push_back(query);
                DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, "Previous query is of different type. Add as a new query to set.");
                }
            }

        for (auto const& query : queries)
            SetCommonQueryResultParameters(*query);

        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, Utf8PrintfString("Total queries in set: %" PRIu64, (uint64_t)queries.size()));
        return queries;
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename RuleType>
static void CallbackOnRuleClasses(bvector<RuleType const*> const& rules, ECSchemaHelper const& helper, std::function<void(RuleType const&, ECEntityClassCR)> const& callback)
    {
    for (RuleType const* rule : rules)
        {
        // we only care about sorting rules that have schema and class specified
        if (rule->GetSchemaName().empty() || rule->GetClassName().empty())
            continue;

        ECClassCP ruleClass = helper.GetECClass(rule->GetSchemaName().c_str(), rule->GetClassName().c_str());
        if (nullptr == ruleClass || !ruleClass->IsEntityClass())
            continue;

        callback(*rule, *ruleClass->GetEntityClassCP());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bvector<RuleApplicationInfo> GetCustomizationRuleInfos(bvector<SelectClassWithExcludes<ECClass> const*> const& selectClasses, GroupingResolver const& resolver,
    bvector<ECClassCP> const& instanceLabelOverrideClasses, NavNodeCP parentNode, NavigationQueryBuilderParameters const& params)
    {
    bvector<RuleApplicationInfo> customizationRuleInfos;

    auto groupingRuleHandlers = resolver.GetAppliedGroupingRuleHandlers(ContainerHelpers::TransformContainer<bvector<ECClassCP>>(selectClasses, [](auto const& s){return &s->GetClass();}));
    for (auto groupingRuleHandler : groupingRuleHandlers)
        {
        ECClassCP ruleTargetClass = groupingRuleHandler->GetTargetClass();
        if (ruleTargetClass)
            customizationRuleInfos.push_back(RuleApplicationInfo(*ruleTargetClass, true));
        }

    IRulesPreprocessor::AggregateCustomizationRuleParameters preprocessorParams(parentNode, resolver.GetSpecificationIdentifier());
    CallbackOnRuleClasses<SortingRule>(resolver.GetQueryBuilderParams().GetRulesPreprocessor().GetSortingRules(preprocessorParams), params.GetSchemaHelper(),
        [&customizationRuleInfos](SortingRuleCR rule, ECEntityClassCR ecClass)
        {
        customizationRuleInfos.push_back(RuleApplicationInfo(ecClass, rule.GetIsPolymorphic()));
        });

    for (ECClassCP instanceLabelOverrideClass : instanceLabelOverrideClasses)
        customizationRuleInfos.push_back(RuleApplicationInfo(*instanceLabelOverrideClass, true));

    return customizationRuleInfos;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bvector<SelectClassWithExcludes<ECClass>> ProcessSelectClassesBasedOnCustomizationRules(bvector<SelectClassWithExcludes<ECClass>> const& selectClasses, GroupingResolver const& resolver,
    bvector<ECClassCP> const& instanceLabelOverrideClasses, NavNodeCP parentNode, NavigationQueryBuilderParameters const& params)
    {
    auto selectClassPtrs = ContainerHelpers::TransformContainer<bvector<SelectClassWithExcludes<ECClass> const*>>(selectClasses, [](auto const& s) {return &s; });
    bvector<RuleApplicationInfo> customizationRuleInfos = GetCustomizationRuleInfos(selectClassPtrs, resolver, instanceLabelOverrideClasses, parentNode, params);
    bvector<SelectClassSplitResult> splitResult = QueryBuilderHelpers::ProcessSelectClassesBasedOnCustomizationRules(selectClasses, customizationRuleInfos, params.GetConnection().GetECDb().Schemas());
    return ContainerHelpers::TransformContainer<bvector<SelectClassWithExcludes<ECClass>>>(splitResult, [&](SelectClassSplitResult const& sr)
        {
        return sr.GetSelectClass();
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bvector<RelatedClassPath> ProcessSelectPathsBasedOnCustomizationRules(bvector<RelatedClassPath> const& selectPaths, GroupingResolver const& resolver,
    bvector<ECClassCP> const& instanceLabelOverrideClasses, NavNodeCP parentNode, NavigationQueryBuilderParameters const& params)
    {
    auto selectClassPtrs = ContainerHelpers::TransformContainer<bvector<SelectClassWithExcludes<ECClass> const*>>(selectPaths,
        [](RelatedClassPath const& path) {return &path.back().GetTargetClass(); });
    bvector<RuleApplicationInfo> customizationRuleInfos = GetCustomizationRuleInfos(selectClassPtrs, resolver, instanceLabelOverrideClasses, parentNode, params);
    return QueryBuilderHelpers::ProcessRelationshipPathsBasedOnCustomizationRules(selectPaths, customizationRuleInfos, params.GetConnection().GetECDb().Schemas());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void JoinRelatedInstancePaths(bvector<SelectQueryInfo>& infos, bvector<RelatedClassPath> const& paths)
    {
    bvector<SelectQueryInfo> newInfos;
    for (SelectQueryInfo const& info : infos)
        {
        for (RelatedClassPath const& path : paths)
            {
            SelectQueryInfo copy(info);
            copy.GetRelatedInstancePaths().push_back(path);
            newInfos.push_back(copy);
            }
        }
    infos.swap(newInfos);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static size_t CountRelatedInstanceJoins(bmap<Utf8String, bvector<RelatedClassPath>> const& selectInfos)
    {
    bset<Utf8String> uniqueAliases;
    for (auto& entry : selectInfos)
        QueryBuilderHelpers::CollectRelatedClassPathAliases(uniqueAliases, entry.second);
    return uniqueAliases.size();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void AssignRelatedInstanceClasses(bvector<SelectQueryInfo>& infos, ChildNodeSpecificationCR specification, GroupingResolver const& resolver,
    bvector<ECClassCP> const& instanceLabelOverrideClasses, NavNodeCP parentNode, NavigationQueryBuilderParameters const& params,
    ECClassUseCounter& relationshipUseCounter)
    {
    bvector<SelectQueryInfo> newInfos;
    for (SelectQueryInfo const& info : infos)
        {
        // get related instance paths that suit the given select info
        bmap<Utf8String, bvector<RelatedClassPath>> relatedInstancePaths = params.GetSchemaHelper().GetRelatedInstancePaths(
            info.GetSelectClass().GetClass(), specification.GetRelatedInstances(), relationshipUseCounter);

        size_t joinsCount = CountRelatedInstanceJoins(relatedInstancePaths);
        if (RELATED_CLASS_PATH_ALIASES_THRESHOLD < joinsCount)
            {
            DIAGNOSTICS_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, LOG_ERROR, Utf8PrintfString("Maximum amount of related instance specifications exceeded for class '%s'. "
                "Required JOINs: %" PRIu64 ", allowed JOINs: %d", info.GetSelectClass().GetClass().GetFullName(), (uint64_t)joinsCount, RELATED_CLASS_PATH_ALIASES_THRESHOLD));
            continue;
            }

        bvector<SelectQueryInfo> thisInfoSplit{info};
        for (auto& pathEntry : relatedInstancePaths)
            {
            bvector<RelatedClassPath> entryPaths;
            for (RelatedClassPath const& entryPath : pathEntry.second)
                {
                ContainerHelpers::Push(entryPaths, ProcessSelectPathsBasedOnCustomizationRules({entryPath},
                    resolver, instanceLabelOverrideClasses, parentNode, params));
                }
            JoinRelatedInstancePaths(thisInfoSplit, entryPaths);
            }
        ContainerHelpers::Push(newInfos, thisInfoSplit);
        }
    // replace infos with the new list
    infos.swap(newInfos);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bvector<ECClassCP> GetInstanceLabelOverrideClasses(ECSchemaHelper const& schemaHelper, bvector<InstanceLabelOverrideCP> const& overrides)
    {
    bvector<ECClassCP> uniqueClasses;
    for (auto ovr : overrides)
        {
        auto ovrClass = schemaHelper.GetECClass(ovr->GetClassName().c_str());
        if (!ovrClass || ContainerHelpers::Contains(uniqueClasses, ovrClass))
            continue;

        uniqueClasses.push_back(ovrClass);
        }
    return uniqueClasses;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename TSpecification>
static bvector<SelectQueryInfo> CreateSelectInfos(TSpecification const& spec, bvector<SelectClassWithExcludes<ECClass>> const& selectClasses, GroupingResolver const& resolver,
    bvector<InstanceLabelOverrideCP> const& instanceLabelOverrides, NavNodeCP parentNode,
    bvector<InstanceFilterDefinitionCP> const& instanceFilterDefinitions, QueryClauseAndBindings instanceFilterECSqlExpression,
    NavigationQueryBuilderParameters const& params, ECClassUseCounter& relationshipUseCounter)
    {
    bvector<ECClassCP> instanceLabelOverrideClasses = GetInstanceLabelOverrideClasses(params.GetSchemaHelper(), instanceLabelOverrides);
    bvector<SelectClassWithExcludes<ECClass>> selects = ProcessSelectClassesBasedOnCustomizationRules(selectClasses, resolver,
        instanceLabelOverrideClasses, parentNode, params);
    bvector<SelectQueryInfo> selectInfos = ContainerHelpers::TransformContainer<bvector<SelectQueryInfo>>(selects, [&](auto const& sc)
        {
        SelectQueryInfo info(sc);
        info.SetLabelOverrideValueSpecs(QueryBuilderHelpers::GetInstanceLabelOverrideSpecsForClass(params.GetSchemaHelper(), instanceLabelOverrides, info.GetSelectClass().GetClass()));
        info.SetInstanceFilterDefinitions(instanceFilterDefinitions);
        info.SetInstanceFilterECSqlExpression(instanceFilterECSqlExpression);
        return info;
        });
    AssignRelatedInstanceClasses(selectInfos, spec, resolver, instanceLabelOverrideClasses, parentNode, params, relationshipUseCounter);
    return selectInfos;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bvector<SelectQueryInfo> CreateSelectInfos(RelatedInstanceNodesSpecification const& spec, bvector<RelatedClassPath> const& pathsFromParentToSelectClass, GroupingResolver const& resolver,
    bvector<InstanceLabelOverrideCP> const& instanceLabelOverrides, NavNodeCP parentNode, bvector<ECInstanceId> const& parentInstanceIds,
    bvector<InstanceFilterDefinitionCP> const& instanceFilterDefinitions, NavigationQueryBuilderParameters const& params, ECClassUseCounter& relationshipUseCounter)
    {
    bvector<ECClassCP> instanceLabelOverrideClasses = GetInstanceLabelOverrideClasses(params.GetSchemaHelper(), instanceLabelOverrides);
    bvector<RelatedClassPath> processedPathsFromParentToSelectClass = ProcessSelectPathsBasedOnCustomizationRules(pathsFromParentToSelectClass, resolver,
        instanceLabelOverrideClasses, parentNode, params);
    bvector<SelectQueryInfo> selectInfos = ContainerHelpers::TransformContainer<bvector<SelectQueryInfo>>(processedPathsFromParentToSelectClass, [&](RelatedClassPath const& path)
        {
        SelectQueryInfo info(path.back().GetTargetClass());
        info.GetSelectClass().SetAlias("this");
        info.SetInstanceFilterDefinitions(instanceFilterDefinitions);
        info.SetLabelOverrideValueSpecs(QueryBuilderHelpers::GetInstanceLabelOverrideSpecsForClass(params.GetSchemaHelper(), instanceLabelOverrides, info.GetSelectClass().GetClass()));
        info.GetPathFromParentToSelectClass() = path;
        for (RelatedClass& rc : info.GetPathFromParentToSelectClass())
            rc.SetIsTargetOptional(false);
        info.SetParentInstanceIds(parentInstanceIds);
        return info;
        });
    AssignRelatedInstanceClasses(selectInfos, spec, resolver, instanceLabelOverrideClasses, parentNode, params, relationshipUseCounter);
    return selectInfos;
    }

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
enum class SelectClassAcceptStatus
    {
    Accept,
    Drop,
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static SelectClassAcceptStatus ApplyClassFilter(SelectClassWithExcludes<>& selectClass, SelectClass<> const& filterClass)
    {
    if (&filterClass.GetClass() == &selectClass.GetClass())
        {
        selectClass.SetIsSelectPolymorphic(selectClass.IsSelectPolymorphic() && filterClass.IsSelectPolymorphic());
        }
    else if (filterClass.GetClass().Is(&selectClass.GetClass()) && selectClass.IsSelectPolymorphic())
        {
        selectClass.SetClass(filterClass.GetClass());
        selectClass.SetIsSelectPolymorphic(filterClass.IsSelectPolymorphic());
        }
    else if (selectClass.GetClass().Is(&filterClass.GetClass()) && filterClass.IsSelectPolymorphic())
        {
        // no need to do anything - we're selecting from derived class and grouping by its base class
        }
    else
        {
        // the select class and the grouping class are unrelated - no need to select at all
        return SelectClassAcceptStatus::Drop;
        }
    return SelectClassAcceptStatus::Accept;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void ApplyClassFilter(bvector<SelectClassWithExcludes<>>& selectClasses, SelectClass<> const& groupingClass)
    {
    bvector<SelectClassWithExcludes<> const*> toErase;
    for (size_t i = 0; i < selectClasses.size(); ++i)
        {
        auto& selectClass = selectClasses[i];
        bool shouldErase = SelectClassAcceptStatus::Drop == ApplyClassFilter(selectClass, groupingClass);
        for (size_t j = 0; j < i && !shouldErase; ++j)
            shouldErase |= (selectClasses[j] == selectClass);
        if (shouldErase)
            toErase.push_back(&selectClass);
        }
    ContainerHelpers::RemoveIf(selectClasses, [&toErase](auto const& item)
        {
        return ContainerHelpers::Contains(toErase, &item);
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void ApplyClassFilter(bvector<RelatedClassPath>& selectPaths, SelectClass<> const& groupingClass)
    {
    bvector<RelatedClassPath const*> toErase;
    for (size_t i = 0; i < selectPaths.size(); ++i)
        {
        RelatedClassPathR path = selectPaths[i];
        auto& pathTarget = path.back().GetTargetClass();
        bool shouldErase = SelectClassAcceptStatus::Drop == ApplyClassFilter(pathTarget, groupingClass);
        for (size_t j = 0; j < i && !shouldErase; ++j)
            shouldErase |= (selectPaths[j] == path);
        if (shouldErase)
            toErase.push_back(&path);
        }
    ContainerHelpers::RemoveIf(selectPaths, [&toErase](auto const& item)
        {
        return ContainerHelpers::Contains(toErase, &item);
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<PresentationQueryBuilderPtr> NavigationQueryBuilder::GetQueries(NavNodeCP parentNode, AllInstanceNodesSpecification const& specification, ChildNodeRuleCR rule) const
    {
    ECClassUseCounter classesCounter;
    GroupingResolver groupingResolver(m_params, parentNode, specification.GetHash(), specification);
    RootQueryContext queryContext(groupingResolver);

    // find the classes to query instances from
    Utf8String supportedSchemas = GetSupportedSchemas(specification, m_params.GetRuleset());
    ECClassSet queryClasses = m_params.GetSchemaHelper().GetECClassesFromSchemaList(supportedSchemas);
    auto selectClasses = ContainerHelpers::TransformContainer<bvector<SelectClassWithExcludes<ECClass>>>(queryClasses, [](auto const& entry)
        {
        return SelectClassWithExcludes<ECClass>(*entry.first, "this", entry.second);
        });
    if (m_params.GetInstanceFilter() && m_params.GetInstanceFilter()->GetSelectClass())
        ApplyClassFilter(selectClasses, SelectClass<>(*m_params.GetInstanceFilter()->GetSelectClass(), ""));
    if (groupingResolver.GetGroupingClass())
        ApplyClassFilter(selectClasses, *groupingResolver.GetGroupingClass());

    // quick return if nothing to select from
    if (selectClasses.empty())
        {
        DIAGNOSTICS_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, LOG_INFO, "Did not find any select classes - no query created");
        return bvector<PresentationQueryBuilderPtr>();
        }

    // determine instance label overrides
    bvector<InstanceLabelOverrideCP> instanceLabelOverrides = m_params.GetRulesPreprocessor().GetInstanceLabelOverrides(IRulesPreprocessor::CustomizationRuleBySpecParameters(specification));

    // create select infos
    bvector<SelectQueryInfo> selectInfos = CreateSelectInfos(specification, selectClasses, groupingResolver,
        instanceLabelOverrides, groupingResolver.GetParentInstanceNode(), { m_params.GetInstanceFilter() }, QueryClauseAndBindings(), m_params, classesCounter);
    DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, Utf8PrintfString("Total select infos: %" PRIu64, (uint64_t)selectInfos.size()));

    // create a query for each class
    for (SelectQueryInfo const& info : selectInfos)
        {
        if (queryContext.Accept(info))
            OnSelected(info, m_params);
        }

    return queryContext.GetQueries();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bmap<ECClassCP, bvector<ECInstanceId>, ECClassNameComparer> GroupClassInstanceKeys(bvector<ECClassInstanceKey> const& vec)
    {
    bmap<ECClassCP, bvector<ECInstanceId>, ECClassNameComparer> map;
    for (ECClassInstanceKeyCR key : vec)
        {
        auto iter = map.find(key.GetClass());
        if (map.end() == iter)
            iter = map.Insert(key.GetClass(), bvector<ECInstanceId>()).first;
        iter->second.push_back(key.GetId());
        }
    return map;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<PresentationQueryBuilderPtr> NavigationQueryBuilder::GetQueries(NavNodeCP parentNode, RelatedInstanceNodesSpecification const& specification, Utf8StringCR specificationHash, ChildNodeRuleCR rule) const
    {
    Utf8String supportedSchemas = GetSupportedSchemas(specification, m_params.GetRuleset());
    ECClassUseCounter classesCounter;
    GroupingResolver groupingResolver(m_params, parentNode, specificationHash, specification);
    RootQueryContext queryContext(groupingResolver);

    // this specification can be used only if parent node is ECInstance node
    if (nullptr == groupingResolver.GetParentInstanceNode() || nullptr == groupingResolver.GetParentInstanceNode()->GetKey()->AsECInstanceNodeKey())
        {
        DIAGNOSTICS_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, LOG_ERROR, Utf8PrintfString("`%s` specification can only be used "
            "if parent node or any of of its ancestor nodes is an ECInstance node - no query created.", specification.GetJsonElementType()));
        return bvector<PresentationQueryBuilderPtr>();
        }

    // determine instance label overrides
    bvector<InstanceLabelOverrideCP> instanceLabelOverrides = m_params.GetRulesPreprocessor().GetInstanceLabelOverrides(IRulesPreprocessor::CustomizationRuleBySpecParameters(specification));

    // get the parent instance keys
    bvector<ECClassInstanceKey> const& parentInstanceKeys = groupingResolver.GetParentInstanceNode()->GetKey()->AsECInstanceNodeKey()->GetInstanceKeys();
    auto parentClassInstanceIdsMap = GroupClassInstanceKeys(parentInstanceKeys);

    // preserve specification instance filter
    auto specificationInstanceFilter = std::make_unique<InstanceFilterDefinition>(specification.GetInstanceFilter());

    // iterate over all parent classes
    for (auto const& entry : parentClassInstanceIdsMap)
        {
        ECEntityClassCR parentClass = *entry.first->GetEntityClassCP();
        bvector<ECInstanceId> const& parentInstanceIds = entry.second;

        // find all applying relationship paths
        bvector<RelatedClassPath> relationshipClassPaths;
        if (specification.GetRelationshipPaths().empty())
            {
            // deprecated:
            int relationshipDirection = 0;
            switch (specification.GetRequiredRelationDirection())
                {
                case RequiredRelationDirection_Forward:  relationshipDirection = (int)ECRelatedInstanceDirection::Forward; break;
                case RequiredRelationDirection_Backward: relationshipDirection = (int)ECRelatedInstanceDirection::Backward; break;
                case RequiredRelationDirection_Both:     relationshipDirection = (int)ECRelatedInstanceDirection::Forward | (int)ECRelatedInstanceDirection::Backward; break;
                }
            ECSchemaHelper::RelationshipClassPathOptionsDeprecated options(parentClass, relationshipDirection, specification.GetSkipRelatedLevel(),
                supportedSchemas.c_str(), specification.GetRelationshipClassNames().c_str(), specification.GetRelatedClassNames().c_str(), true,
                classesCounter, groupingResolver.GetGroupingClass() ? &groupingResolver.GetGroupingClass()->GetClass() : nullptr);
            relationshipClassPaths = m_params.GetSchemaHelper().GetRelationshipClassPathsDeprecated(options);
            }
        else
            {
            relationshipClassPaths = m_params.GetSchemaHelper().GetRecursiveRelationshipClassPaths(parentClass, parentInstanceIds,
                specification.GetRelationshipPaths(), classesCounter, true, false);
            if (groupingResolver.GetGroupingClass())
                ApplyClassFilter(relationshipClassPaths, *groupingResolver.GetGroupingClass());
            }
        if (m_params.GetInstanceFilter() && m_params.GetInstanceFilter()->GetSelectClass())
            ApplyClassFilter(relationshipClassPaths, SelectClass<>(*m_params.GetInstanceFilter()->GetSelectClass(), ""));

        // create select infos
        bvector<SelectQueryInfo> selectInfos = CreateSelectInfos(specification, relationshipClassPaths, groupingResolver,
            instanceLabelOverrides, groupingResolver.GetParentInstanceNode(), parentInstanceIds,
            { specificationInstanceFilter.get(), m_params.GetInstanceFilter() }, m_params, classesCounter);
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, Utf8PrintfString("Total select infos: %" PRIu64, (uint64_t)selectInfos.size()));

        // union everything
        for (SelectQueryInfo const& info : selectInfos)
            {
            if (queryContext.Accept(info))
                OnSelected(info, m_params);
            }
        }

    return queryContext.GetQueries();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<PresentationQueryBuilderPtr> NavigationQueryBuilder::GetQueries(NavNodeCP parentNode, InstanceNodesOfSpecificClassesSpecification const& specification, ChildNodeRuleCR rule) const
    {
    ECClassUseCounter classesCounter;
    GroupingResolver groupingResolver(m_params, parentNode, specification.GetHash(), specification);
    RootQueryContext queryContext(groupingResolver);

    SupportedClassInfos excludedQueryClassInfos = m_params.GetSchemaHelper().GetECClassesFromClassList(specification.GetExcludedClasses(), true);
    auto const excludedClasses = ContainerHelpers::TransformContainer<bvector<SelectClass<ECClass>>>(excludedQueryClassInfos, [&](auto const& eci)
        {
        return SelectClass<ECClass>(eci.GetClass(), "", eci.IsPolymorphic());
        });

    SupportedClassInfos requestedClasses = m_params.GetSchemaHelper().GetECClassesFromClassList(specification.GetClasses(), false);
    auto selectClasses = ContainerHelpers::TransformContainer<bvector<SelectClassWithExcludes<ECClass>>>(requestedClasses, [&excludedClasses](auto const& requestedClass)
        {
        SelectClassWithExcludes<ECClass> selectClass(requestedClass.GetClass(), "this", requestedClass.IsPolymorphic());
        selectClass.GetDerivedExcludedClasses() = excludedClasses;
        return selectClass;
        });
    if (m_params.GetInstanceFilter() && m_params.GetInstanceFilter()->GetSelectClass())
        ApplyClassFilter(selectClasses, SelectClass<>(*m_params.GetInstanceFilter()->GetSelectClass(), ""));
    if (groupingResolver.GetGroupingClass())
        ApplyClassFilter(selectClasses, *groupingResolver.GetGroupingClass());

    // quick return if nothing to select from
    if (selectClasses.empty())
        {
        DIAGNOSTICS_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, LOG_INFO, "Did not find any select classes - no query created");
        return bvector<PresentationQueryBuilderPtr>();
        }

    // determine instance label overrides
    bvector<InstanceLabelOverrideCP> instanceLabelOverrides = m_params.GetRulesPreprocessor().GetInstanceLabelOverrides(IRulesPreprocessor::CustomizationRuleBySpecParameters(specification));

    // preserve specification instance filter
    auto specificationInstanceFilter = std::make_unique<InstanceFilterDefinition>(specification.GetInstanceFilter());

    // create select infos
    bvector<SelectQueryInfo> selectInfos = CreateSelectInfos(specification, selectClasses, groupingResolver, instanceLabelOverrides, groupingResolver.GetParentInstanceNode(),
        { specificationInstanceFilter.get(), m_params.GetInstanceFilter() }, QueryClauseAndBindings(), m_params, classesCounter);
    DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, Utf8PrintfString("Total select infos: %" PRIu64, (uint64_t)selectInfos.size()));

    // union everything
    for (SelectQueryInfo const& info : selectInfos)
        {
        if (queryContext.Accept(info))
            OnSelected(info, m_params);
        }

    return queryContext.GetQueries();
    }

#define SEARCH_QUERY_INJECTION      ", ECInstanceId AS [" SEARCH_QUERY_FIELD_ECInstanceId "]"
/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct SearchQueryCreator : QuerySpecificationVisitor
{
private:
    ECSchemaHelper const& m_helper;
    NavNodeCP m_parentNode;
    IUsedClassesListener* m_usedClassesListener;
    Utf8String m_query;

private:
    static void InjectRulesEngineFields(Utf8StringR query)
        {
        Utf8String inputUppercase = query;
        inputUppercase.ToUpper();

        size_t pos;
        while (Utf8String::npos != (pos = inputUppercase.rfind(" FROM", Utf8String::npos)))
            {
            query.insert(pos, SEARCH_QUERY_INJECTION);
            inputUppercase = inputUppercase.erase(pos);
            }
        }

protected:
    /*-----------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+-----------+------*/
    void _Visit(StringQuerySpecificationCR spec) override
        {
        auto scope = Diagnostics::Scope::Create(Utf8PrintfString("Create query from %s", DiagnosticsHelpers::CreateRuleIdentifier(spec).c_str()));
        m_query = spec.GetQuery();
        DIAGNOSTICS_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, LOG_INFO, Utf8PrintfString("Using query: `%s`", m_query.c_str()));
        InjectRulesEngineFields(m_query);
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, Utf8PrintfString("Query after injecting internal fields: `%s`", m_query.c_str()));
        }

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+-----------+------*/
    void _Visit(ECPropertyValueQuerySpecificationCR spec) override
        {
        auto scope = Diagnostics::Scope::Create(Utf8PrintfString("Create query from %s", DiagnosticsHelpers::CreateRuleIdentifier(spec).c_str()));

        if (nullptr == m_parentNode || nullptr == m_parentNode->GetKey()->AsECInstanceNodeKey())
            {
            DIAGNOSTICS_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, LOG_ERROR, "ECPropertyValueQuerySpecification can only be used when its parent "
                "or any of its ancestors is an ECInstance node - ignoring the specification.");
            return;
            }

        ECInstancesNodeKey const& key = *m_parentNode->GetKey()->AsECInstanceNodeKey();
        auto parentClassInstanceIds = GroupClassInstanceKeys(key.GetInstanceKeys());
        DIAGNOSTICS_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, LOG_INFO, Utf8PrintfString("Parent ECInstance node is based on %" PRIu64 " instances", (uint64_t)key.GetInstanceKeys().size()));

        ECClassInstanceKey usedParentInstanceKey;
        for (auto const& entry : parentClassInstanceIds)
            {
            ECClassCP parentClass = entry.first;
            auto classScope = Diagnostics::Scope::Create(Utf8PrintfString("Handling class: %s", parentClass->GetFullName()));

            ECPropertyCP queryProperty = parentClass->GetPropertyP(spec.GetParentPropertyName().c_str());
            if (nullptr == queryProperty)
                {
                DIAGNOSTICS_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, LOG_ERROR, Utf8PrintfString("The class `%s` doesn't contain requested ECProperty `%s`.",
                    parentClass->GetFullName(), spec.GetParentPropertyName().c_str()));
                continue;
                }
            if (!queryProperty->GetIsPrimitive() || PRIMITIVETYPE_String != queryProperty->GetAsPrimitiveProperty()->GetType())
                {
                DIAGNOSTICS_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, LOG_ERROR, Utf8PrintfString("ECProperty `%s.%s` is not of string type. "
                    "The specification requires a string property", parentClass->GetFullName(), queryProperty->GetName().c_str()));
                continue;
                }
            DIAGNOSTICS_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, LOG_INFO, Utf8PrintfString("Using property `%s.%s`.",
                parentClass->GetFullName(), spec.GetParentPropertyName().c_str()));

            for (ECInstanceId instanceId : entry.second)
                {
                auto instanceScope = Diagnostics::Scope::Create(Utf8PrintfString("Handling ECInstance: %" PRIu64, instanceId.GetValueUnchecked()));

                ECValue propertyValue = ECInstancesHelper::GetValue(m_helper.GetConnection(), ECInstanceKey(parentClass->GetId(), instanceId), queryProperty->GetName().c_str());
                if (propertyValue.IsUninitialized() || propertyValue.IsNull())
                    {
                    DIAGNOSTICS_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, LOG_INFO, "Property value not set, skipping this instance ID.");
                    continue;
                    }

                if (!propertyValue.IsString())
                    {
                    DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Hierarchies, Utf8PrintfString("String property has non-string value: `%s`", propertyValue.ToString().c_str()));
                    continue;
                    }

                usedParentInstanceKey = ECClassInstanceKey(parentClass, instanceId);
                m_query = propertyValue.GetUtf8CP();
                DIAGNOSTICS_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, LOG_INFO, Utf8PrintfString("Using query: `%s`", m_query.c_str()));
                break;
                }

            if (usedParentInstanceKey.IsValid())
                break;
            }

        if (nullptr != m_usedClassesListener && usedParentInstanceKey.IsValid())
            m_usedClassesListener->_OnClassUsed(*usedParentInstanceKey.GetClass(), false);

        InjectRulesEngineFields(m_query);
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, Utf8PrintfString("Query after injecting internal fields: `%s`", m_query.c_str()));
        }

public:
    SearchQueryCreator(ECSchemaHelper const& helper, NavNodeCP parentNode, IUsedClassesListener* usedClassesListener)
        : m_helper(helper), m_parentNode(parentNode), m_usedClassesListener(usedClassesListener)
        {}
    Utf8StringCR GetQuery() const {return m_query;}
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String GetQuery(ECSchemaHelper const& helper, QuerySpecification const& specification, NavNodeCP parentNode, IUsedClassesListener* usedClassesListener)
    {
    SearchQueryCreator creator(helper, parentNode, usedClassesListener);
    specification.Accept(creator);
    return creator.GetQuery();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<PresentationQueryBuilderPtr> NavigationQueryBuilder::GetQueries(NavNodeCP parentNode, SearchResultInstanceNodesSpecification const& specification, ChildNodeRuleCR rule) const
    {
    if (specification.GetQuerySpecifications().empty())
        {
        DIAGNOSTICS_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, LOG_WARNING, "SearchResultInstanceNodes specification has no queries specified");
        return bvector<PresentationQueryBuilderPtr>();
        }

    ECClassUseCounter classesCounter;
    GroupingResolver groupingResolver(m_params, parentNode, specification.GetHash(), specification);
    RootQueryContext queryContext(groupingResolver);
    bvector<InstanceLabelOverrideCP> instanceLabelOverrides = m_params.GetRulesPreprocessor().GetInstanceLabelOverrides(IRulesPreprocessor::CustomizationRuleBySpecParameters(specification));

    // create a query for each class
    for (QuerySpecification* querySpecification : specification.GetQuerySpecifications())
        {
        ECClassCP queryClass = m_params.GetSchemaHelper().GetECClass(querySpecification->GetSchemaName().c_str(), querySpecification->GetClassName().c_str());
        if (nullptr == queryClass)
            {
            DIAGNOSTICS_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, LOG_ERROR, Utf8PrintfString("Requested search query class not found: '%s:%s'",
                querySpecification->GetSchemaName().c_str(), querySpecification->GetClassName().c_str()));
            continue;
            }

        SelectClassWithExcludes<ECClass> selectClass(*queryClass, "this", true);
        if (m_params.GetInstanceFilter() && m_params.GetInstanceFilter()->GetSelectClass())
            {
            if (SelectClassAcceptStatus::Drop == ApplyClassFilter(selectClass, SelectClass<>(*m_params.GetInstanceFilter()->GetSelectClass(), "")))
                continue;
            }
        if (groupingResolver.GetGroupingClass())
            {
            if (SelectClassAcceptStatus::Drop == ApplyClassFilter(selectClass, *groupingResolver.GetGroupingClass()))
                continue;
            }

        // create the search query
        Utf8String searchQuery = GetQuery(m_params.GetSchemaHelper(), *querySpecification, groupingResolver.GetParentInstanceNode(), m_params.GetUsedClassesListener());
        if (searchQuery.empty())
            {
            DIAGNOSTICS_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, LOG_ERROR, "Failed to create a search query for given query specification");
            continue;
            }

        QueryClauseAndBindings ecsqlInstanceFilter(Utf8PrintfString("[this].[ECInstanceId] IN (SELECT [" SEARCH_QUERY_FIELD_ECInstanceId "] FROM (%s))", searchQuery.c_str()));

        // create select infos
        bvector<SelectQueryInfo> selectInfos = CreateSelectInfos(specification, {selectClass}, groupingResolver,
            instanceLabelOverrides, parentNode, { m_params.GetInstanceFilter() }, ecsqlInstanceFilter, m_params, classesCounter);

        for (SelectQueryInfo const& info : selectInfos)
            {
            if (queryContext.Accept(info))
                OnSelected(info, m_params);
            }
        }

    return queryContext.GetQueries();
    }
