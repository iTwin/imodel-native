/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include <ECPresentation/RulesDriven/PresentationManager.h>
#include <ECPresentation/RulesDriven/Rules/SpecificationVisitor.h>
#include "RulesPreprocessor.h"
#include "QueryBuilder.h"
#include "ExtendedData.h"
#include "ECSchemaHelper.h"
#include "LoggingHelper.h"
#include "NavNodeProviders.h"
#include "NavNodesDataSource.h"
#include "NavNodesCache.h"
#include "QueryContracts.h"
#include "CustomFunctions.h"
#include "ECExpressionContextsProvider.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void UsedClassesHelper::NotifyListenerWithUsedClasses(IUsedClassesListener& listener, ECSchemaHelper const& schemaHelper, ECExpressionsCache& ecexpressionsCache, Utf8StringCR ecexpression)
    {
    bvector<Utf8String> const& usedClasses = ECExpressionsHelper(ecexpressionsCache).GetUsedClasses(ecexpression);
    for (Utf8StringCR usedClassName : usedClasses)
        {
        Utf8String schemaName, className;
        if (ECObjectsStatus::Success != ECClass::ParseClassName(schemaName, className, usedClassName))
            {
            BeAssert(false);
            continue;
            }
        if (!schemaName.empty())
            {
            ECClassCP usedClass = schemaHelper.GetECClass(schemaName.c_str(), className.c_str());
            if (nullptr == usedClass)
                {
                BeAssert(false);
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
* @bsimethod                                    Grigas.Petraitis                11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T>
static void NotifyListenerWithCustomizationRuleClasses(IUsedClassesListener& listener, ECSchemaHelper const& schemaHelper, ECExpressionsCache& ecexpressionsCache, bvector<T*> const& rules)
    {
    for (T const* rule : rules)
        UsedClassesHelper::NotifyListenerWithUsedClasses(listener, schemaHelper, ecexpressionsCache, rule->GetCondition());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void UsedClassesHelper::NotifyListenerWithRulesetClasses(IUsedClassesListener& listener, ECSchemaHelper const& schemaHelper, ECExpressionsCache& ecexpressionsCache, PresentationRuleSetCR ruleset)
    {
    NotifyListenerWithCustomizationRuleClasses(listener, schemaHelper, ecexpressionsCache, ruleset.GetGroupingRules());
    NotifyListenerWithCustomizationRuleClasses(listener, schemaHelper, ecexpressionsCache, ruleset.GetSortingRules());
    NotifyListenerWithCustomizationRuleClasses(listener, schemaHelper, ecexpressionsCache, ruleset.GetLabelOverrides());
    NotifyListenerWithCustomizationRuleClasses(listener, schemaHelper, ecexpressionsCache, ruleset.GetImageIdOverrides());
    NotifyListenerWithCustomizationRuleClasses(listener, schemaHelper, ecexpressionsCache, ruleset.GetStyleOverrides());
    NotifyListenerWithCustomizationRuleClasses(listener, schemaHelper, ecexpressionsCache, ruleset.GetCheckBoxRules());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void UsedClassesHelper::NotifyListenerWithUsedClasses(IECDbUsedClassesListener& listener, ECExpressionsCache& ecexpressionsCache, IConnectionCR connection, Utf8StringCR ecexpression)
    {
    ECSchemaHelper schemaHelper(connection, nullptr, nullptr, nullptr);
    ECDbUsedClassesListenerWrapper wrapper(connection, listener);
    NotifyListenerWithUsedClasses(wrapper, schemaHelper, ecexpressionsCache, ecexpression);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void UsedClassesHelper::NotifyListenerWithRulesetClasses(IECDbUsedClassesListener& listener, ECExpressionsCache& ecexpressionsCache, IConnectionCR connection, PresentationRuleSetCR ruleset)
    {
    ECSchemaHelper schemaHelper(connection, nullptr, nullptr, nullptr);
    ECDbUsedClassesListenerWrapper wrapper(connection, listener);
    NotifyListenerWithRulesetClasses(wrapper, schemaHelper, ecexpressionsCache, ruleset);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
NavigationQueryBuilder::NavigationQueryBuilder(NavigationQueryBuilderParameters params)
    : m_params(params)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
NavigationQueryBuilder::~NavigationQueryBuilder()
    {
    }

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                04/2015
+===============+===============+===============+===============+===============+======*/
struct NavigationQueryBuilder::SpecificationsVisitor : PresentationRuleSpecificationVisitor
{
private:
    NavigationQueryBuilder const& m_queryBuilder;
    JsonNavNodeCP m_parentNode;
    RootNodeRuleCP m_rootNodeRule;
    ChildNodeRuleCP m_childNodeRule;
    bvector<NavigationQueryPtr> m_queries;

private:
    template<typename T> void HandleSpecification(T const& specification)
        {
        BeAssert(m_queries.empty());
        if (nullptr != m_parentNode)
            {
            BeAssert(nullptr != m_childNodeRule);
            m_queries = m_queryBuilder.GetQueries(*m_parentNode, specification, *m_childNodeRule);
            }
        else
            {
            BeAssert(nullptr != m_rootNodeRule);
            m_queries = m_queryBuilder.GetQueries(specification, *m_rootNodeRule);
            }
        }

protected:
    void _Visit(AllInstanceNodesSpecification const& specification) override {HandleSpecification(specification);}
    void _Visit(AllRelatedInstanceNodesSpecification const& specification) override {HandleSpecification(specification);}
    void _Visit(RelatedInstanceNodesSpecification const& specification) override {HandleSpecification(specification);}
    void _Visit(InstanceNodesOfSpecificClassesSpecification const& specification) override {HandleSpecification(specification);}
    void _Visit(SearchResultInstanceNodesSpecification const& specification) override {HandleSpecification(specification);}

public:
    SpecificationsVisitor(NavigationQueryBuilder const& queryBuilder, RootNodeRuleCR rule)
        : m_queryBuilder(queryBuilder), m_rootNodeRule(&rule), m_parentNode(nullptr), m_childNodeRule(nullptr)
        {}
    SpecificationsVisitor(NavigationQueryBuilder const& queryBuilder, ChildNodeRuleCR rule, JsonNavNodeCP parentNode)
        : m_queryBuilder(queryBuilder), m_rootNodeRule(nullptr), m_parentNode(parentNode), m_childNodeRule(&rule)
        {}
    bvector<NavigationQueryPtr> const& GetQueries() const {return m_queries;}
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<NavigationQueryPtr> NavigationQueryBuilder::GetQueries(ChildNodeRuleCR rule, ChildNodeSpecificationCR spec, JsonNavNodeCR parentNode) const
    {
    BeAssert(m_params.GetNodesCache().GetNode(parentNode.GetNodeId()).IsValid());
    SpecificationsVisitor visitor(*this, rule, &parentNode);
    spec.Accept(visitor);
    return visitor.GetQueries();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<NavigationQueryPtr> NavigationQueryBuilder::GetQueries(RootNodeRuleCR rule, ChildNodeSpecificationCR spec) const
    {
    SpecificationsVisitor visitor(*this, rule);
    spec.Accept(visitor);
    return visitor.GetQueries();
    }

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                11/2016
+===============+===============+===============+===============+===============+======*/
enum class SelectionPurpose
    {
    Include,
    Exclude,
    };

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                11/2016
+===============+===============+===============+===============+===============+======*/
struct SelectQueryInfo
    {
    private:
        ChildNodeSpecificationCP m_specification;
        ECClassCP m_selectClass;
        RelatedClassPath m_relatedClassPath;
        bool m_selectPolymorphically;
        bvector<RelatedClass> m_relatedClasses;
        SelectionPurpose m_flag;
        bvector<InstanceLabelOverrideValueSpecification const*> m_labelOverrideValueSpecs;
    public:
        SelectQueryInfo() : m_selectClass(nullptr) {}
        SelectQueryInfo(ChildNodeSpecificationCR spec, ECClassCR selectClass, bool selectPolymorphically,
            SelectionPurpose flag = SelectionPurpose::Include, bvector<RelatedClass> relatedClasses = bvector<RelatedClass>())
            : m_specification(&spec), m_selectClass(&selectClass), m_selectPolymorphically(selectPolymorphically), m_flag(flag), m_relatedClasses(relatedClasses)
            {}
        ChildNodeSpecificationCP GetSpecification() const {return m_specification;}
        void SetSpecification(ChildNodeSpecificationCR spec) {m_specification = &spec;}

        ECClassCP GetSelectClass() const {return m_selectClass;}
        void SetSelectClass(ECClassCR selectClass) {m_selectClass = &selectClass;}

        bool GetSelectPolymorphically() const {return m_selectPolymorphically;}
        void SetSelectPolymorphically(bool value) {m_selectPolymorphically = value;}

        SelectionPurpose GetSelectFlag() const {return m_flag;}

        RelatedClassPath const& GetRelatedClassPath() const {return m_relatedClassPath;}
        RelatedClassPath& GetRelatedClassPath() {return m_relatedClassPath;}

        bvector<RelatedClass> const& GetRelatedClasses() const {return m_relatedClasses;}
        bvector<RelatedClass>& GetRelatedClasses() {return m_relatedClasses;}

        bvector<ECClassCP> CreateSelectClassList() const
            {
            bvector<ECClassCP> list;
            list.push_back(m_selectClass);
            for (RelatedClassCR relatedPathClass : m_relatedClassPath)
                list.push_back(relatedPathClass.GetRelationship());
            for (RelatedClassCR relatedSelectClass : m_relatedClasses)
                list.push_back(relatedSelectClass.GetTargetClass());
            return list;
            }

        bvector<InstanceLabelOverrideValueSpecification const*> const& GetLabelOverrideValueSpecs() const { return m_labelOverrideValueSpecs; }
        void SetLabelOverrideValueSpecs(bvector<InstanceLabelOverrideValueSpecification const*> const& overrides) { m_labelOverrideValueSpecs = overrides; }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static void OnSelected(ECClassCR ecClass, bool polymorphically, NavigationQueryBuilderParameters const& params)
    {
    if (nullptr != params.GetUsedClassesListener())
        params.GetUsedClassesListener()->_OnClassUsed(ecClass, polymorphically);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static void OnSelected(SelectQueryInfo const& selectInfo, NavigationQueryBuilderParameters const& params)
    {
    if (nullptr == params.GetUsedClassesListener())
        return;

    params.GetUsedClassesListener()->_OnClassUsed(*selectInfo.GetSelectClass(), selectInfo.GetSelectPolymorphically());
    for (RelatedClass const& related : selectInfo.GetRelatedClassPath())
        {
        params.GetUsedClassesListener()->_OnClassUsed(*related.GetTargetClass(), related.IsPolymorphic());
        params.GetUsedClassesListener()->_OnClassUsed(*related.GetRelationship(), true);
        }

    for (RelatedClass const& related : selectInfo.GetRelatedClasses())
        {
        params.GetUsedClassesListener()->_OnClassUsed(*related.GetTargetClass(), related.IsPolymorphic());
        params.GetUsedClassesListener()->_OnClassUsed(*related.GetRelationship(), true);
        }
    }

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                12/2019
+===============+===============+===============+===============+===============+======*/
enum class ApplyGroupingResult
    {
    Error,      //!< failed to apply grouping
    Handled,    //!< the query was handled successfully
    Stored,     //!< the query was handled successfully and stored inside the handler - should not be passed on to other handlers
    };

struct AdvancedGroupingHandler;
/*=================================================================================**//**
* Abstract class for all grouping handlers.
* @bsiclass                                     Grigas.Petraitis                07/2015
+===============+===============+===============+===============+===============+======*/
struct GroupingHandler
{
    struct Compare
        {
        bool operator() (GroupingHandler const& lhs, GroupingHandler const& rhs) const;
        bool operator() (GroupingHandler const* lhs, GroupingHandler const* rhs) const;
        };

protected:
    virtual GroupingType _GetType() const = 0;
    virtual NavigationQueryContractPtr _GetContract(SelectQueryInfo const&) const = 0;
    virtual bool _AppliesForClass(ECClassCR ecClass, bool polymorphic) const = 0;
    virtual bvector<GroupingHandler*> _FindMatchingHandlersOfTheSameType(bvector<GroupingHandler*>::iterator begin, bvector<GroupingHandler*>::iterator end) const {return bvector<GroupingHandler*>();}
    virtual bool _IsAppliedTo(NavNodeCR node) const
        {
        if (node.GetType().Equals(NAVNODE_TYPE_ECInstancesNode))
            return false;

        NavNodeExtendedData extendedData(node);
        return extendedData.HasGroupingType() && (GetType() == (GroupingType)extendedData.GetGroupingType());
        }
    virtual NavigationQueryPtr _GetStoredQuery() const {return nullptr;}
    virtual ApplyGroupingResult _ApplyClassGrouping(ComplexNavigationQueryPtr& query, SelectQueryInfo const&) {return ApplyGroupingResult::Error;}
    virtual bool _ApplyOuterGrouping(NavigationQueryPtr& query)
        {
        if (query.IsValid())
            query->GetResultParametersR().GetNavNodeExtendedDataR().SetGroupingType((int)GetType());
        return false;
        }
    virtual bool _ApplyFilter(ComplexNavigationQueryPtr& query, SelectQueryInfo const&, NavNodeCR filteringNode) const {return false;}
    virtual AdvancedGroupingHandler* _AsAdvancedGroupingHandler() {return nullptr;}

public:
    virtual ~GroupingHandler() {}
    GroupingType GetType() const {return _GetType();}
    NavigationQueryContractPtr GetContract(SelectQueryInfo const& selectInfo) const {return _GetContract(selectInfo);}
    bvector<GroupingHandler*> FindMatchingHandlersOfTheSameType(bvector<GroupingHandler*>::iterator begin, bvector<GroupingHandler*>::iterator end) {return _FindMatchingHandlersOfTheSameType(begin, end);}
    bool AppliesForClass(ECClassCR ecClass, bool polymorphic) const {return _AppliesForClass(ecClass, polymorphic);}
    bool IsAppliedTo(NavNodeCR node) const {return _IsAppliedTo(node);}
    NavigationQueryPtr GetStoredQuery() const {return _GetStoredQuery();}
    ApplyGroupingResult ApplyClassGrouping(ComplexNavigationQueryPtr& query, SelectQueryInfo const& selectInfo) {return _ApplyClassGrouping(query, selectInfo);}
    bool ApplyOuterGrouping(NavigationQueryPtr& query) {return _ApplyOuterGrouping(query);}
    bool ApplyFilter(ComplexNavigationQueryPtr& query, SelectQueryInfo const& selectInfo, NavNodeCR filteringNode) const {return _ApplyFilter(query, selectInfo, filteringNode);}
    AdvancedGroupingHandler const* AsAdvancedGroupingHandler() const {return const_cast<GroupingHandler*>(this)->_AsAdvancedGroupingHandler();}
    AdvancedGroupingHandler* AsAdvancedGroupingHandler() {return _AsAdvancedGroupingHandler();}
};

typedef bvector<GroupingHandler*> GroupingHandlersList;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool GroupingHandler::Compare::operator() (GroupingHandler const& lhs, GroupingHandler const& rhs) const {return lhs.GetType() > rhs.GetType();}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool GroupingHandler::Compare::operator() (GroupingHandler const* lhs, GroupingHandler const* rhs) const {return lhs->GetType() > rhs->GetType();}

/*=================================================================================**//**
* Handles GroupByRelationship grouping
* @bsiclass                                     Grigas.Petraitis                07/2015
+===============+===============+===============+===============+===============+======*/
struct RelationshipGroupingHandler : GroupingHandler
{
private:
    mutable RefCountedPtr<ECRelationshipGroupingNodesQueryContract> m_contract;
protected:
    GroupingType _GetType() const override {return GroupingType::Relationship;}
    NavigationQueryContractPtr _GetContract(SelectQueryInfo const&) const override
        {
        if (m_contract.IsNull())
            m_contract = ECRelationshipGroupingNodesQueryContract::Create();
        return m_contract;
        }
    bool _AppliesForClass(ECClassCR, bool) const override {return true;}
    bool _IsAppliedTo(NavNodeCR node) const override {return node.GetType().Equals(NAVNODE_TYPE_ECRelationshipGroupingNode);}
};

/*=================================================================================**//**
* Handles GroupByClass grouping
* @bsiclass                                     Grigas.Petraitis                07/2015
+===============+===============+===============+===============+===============+======*/
struct ClassGroupingHandler : GroupingHandler
{
DEFINE_T_SUPER(GroupingHandler)

private:
    bool m_doNotSort;
    mutable RefCountedPtr<ECClassGroupingNodesQueryContract> m_contract;

private:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                06/2015
    +---------------+---------------+---------------+---------------+---------------+------*/
    NavigationQueryContractPtr GetContract() const
        {
        if (m_contract.IsNull())
            m_contract = ECClassGroupingNodesQueryContract::Create();
        return m_contract;
        }

protected:
    GroupingType _GetType() const override {return GroupingType::Class;}
    NavigationQueryContractPtr _GetContract(SelectQueryInfo const&) const override {return GetContract();}
    bool _AppliesForClass(ECClassCR, bool) const override {return true;}
    ApplyGroupingResult _ApplyClassGrouping(ComplexNavigationQueryPtr& query, SelectQueryInfo const&) override {return ApplyGroupingResult::Handled;}

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                06/2015
    +---------------+---------------+---------------+---------------+---------------+------*/
    bool _ApplyOuterGrouping(NavigationQueryPtr& query) override
        {
        T_Super::_ApplyOuterGrouping(query);

        NavigationQueryContractCR contract = *GetContract();
        ComplexNavigationQueryPtr complexQuery = QueryBuilderHelpers::CreateComplexNestedQueryIfNecessary<NavigationQuery>(*query, contract.GetGroupingAliases());
        complexQuery->GroupByContract(contract);
        query = complexQuery;

        if (!m_doNotSort)
            {
            static Utf8PrintfString sortedDisplayLabel("%s([%s])", FUNCTION_NAME_GetSortingValue, ECClassGroupingNodesQueryContract::DisplayLabelFieldName);

            bvector<Utf8CP> sortingAliases;
            sortingAliases.push_back(ECClassGroupingNodesQueryContract::DisplayLabelFieldName);

            query = QueryBuilderHelpers::CreateNestedQueryIfNecessary(*query, sortingAliases);
            QueryBuilderHelpers::Order(*query, sortedDisplayLabel.c_str());
            }

        return true;
        }

public:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                06/2015
    +---------------+---------------+---------------+---------------+---------------+------*/
    ClassGroupingHandler(bool doNotSort) : m_doNotSort(doNotSort) {}
};

/*=================================================================================**//**
* Handles GroupByLabel grouping
* @bsiclass                                     Grigas.Petraitis                07/2015
+===============+===============+===============+===============+===============+======*/
struct DisplayLabelGroupingHandler : GroupingHandler
{
DEFINE_T_SUPER(GroupingHandler)

private:
    bool m_doNotSort;

protected:
    GroupingType _GetType() const override {return GroupingType::DisplayLabel;}
    NavigationQueryContractPtr _GetContract(SelectQueryInfo const& selectInfo) const override
        {
        return DisplayLabelGroupingNodesQueryContract::Create(selectInfo.GetSelectClass(), selectInfo.GetRelatedClasses(), selectInfo.GetLabelOverrideValueSpecs());
        }
    bool _AppliesForClass(ECClassCR, bool) const override {return true;}
    bool _IsAppliedTo(NavNodeCR node) const override {return node.GetType().Equals(NAVNODE_TYPE_DisplayLabelGroupingNode);}
    ApplyGroupingResult _ApplyClassGrouping(ComplexNavigationQueryPtr& query, SelectQueryInfo const&) override {return ApplyGroupingResult::Handled;}

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                06/2015
    +---------------+---------------+---------------+---------------+---------------+------*/
    bool _ApplyOuterGrouping(NavigationQueryPtr& query) override
        {
        T_Super::_ApplyOuterGrouping(query);

        NavigationQueryContractCR contract = *query->GetContract();
        ComplexNavigationQueryPtr complexQuery = QueryBuilderHelpers::CreateComplexNestedQueryIfNecessary<NavigationQuery>(*query, contract.GetGroupingAliases());
        complexQuery->GroupByContract(contract);

        if (!m_doNotSort)
            {
            static Utf8PrintfString sortedDisplayLabel("%s([%s])", FUNCTION_NAME_GetSortingValue, DisplayLabelGroupingNodesQueryContract::DisplayLabelFieldName);
            complexQuery->OrderBy(sortedDisplayLabel.c_str());
            }

        complexQuery->GetResultParametersR().GetNavNodeExtendedDataR().SetHideIfOnlyOneChild(true);
        query = complexQuery;
        return true;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                06/2015
    +---------------+---------------+---------------+---------------+---------------+------*/
    bool _ApplyFilter(ComplexNavigationQueryPtr& query, SelectQueryInfo const&, NavNodeCR filteringNode) const override
        {
        NavNodeExtendedData extendedData(filteringNode);
        IdsFilteringHelper<IdSet<ECInstanceId>> filteringHelper(extendedData.GetInstanceIds());
        PresentationQueryContractFieldCPtr ecInstanceIdField = query->GetContract()->GetField(ECInstanceNodesQueryContract::ECInstanceIdFieldName);
        query->Where(filteringHelper.CreateWhereClause(ecInstanceIdField->GetSelectClause(query->GetSelectPrefix()).c_str()).c_str(), filteringHelper.CreateBoundValues());
        return true;
        }

public:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                06/2015
    +---------------+---------------+---------------+---------------+---------------+------*/
    DisplayLabelGroupingHandler(bool doNotSort) : m_doNotSort(doNotSort) {}
};

/*=================================================================================**//**
* Base class for handlers which group based on grouping rules (as opposed to GroupByClass,
* GroupByRelationship, GroupByLabel).
* @bsiclass                                     Grigas.Petraitis                07/2015
+===============+===============+===============+===============+===============+======*/
struct AdvancedGroupingHandler : GroupingHandler
{
protected:
    ECSchemaHelper const& m_schemaHelper;
    GroupingRuleCR m_rule;
private:
    ECClassCP GetRuleClass() const {return m_schemaHelper.GetECClass(m_rule.GetSchemaName().c_str(), m_rule.GetClassName().c_str());}
protected:
    AdvancedGroupingHandler(GroupingRuleCR rule, ECSchemaHelper const& helper) : m_rule(rule), m_schemaHelper(helper) {}
    bvector<GroupingHandler*> _FindMatchingHandlersOfTheSameType(bvector<GroupingHandler*>::iterator begin, bvector<GroupingHandler*>::iterator end) const override
        {
        bvector<GroupingHandler*> matches;
        ECClassCP myRuleClass = GetRuleClass();
        if (!myRuleClass)
            {
            BeAssert(false);
            return matches;
            }
        for (auto iter = begin; iter != end; ++iter)
            {
            AdvancedGroupingHandler* handler = (*iter)->AsAdvancedGroupingHandler();
            if (!handler)
                continue;
            ECClassCP handlerRuleClass = handler->GetRuleClass();
            if (!handlerRuleClass)
                {
                BeAssert(false);
                continue;
                }
            if (handlerRuleClass->Is(myRuleClass))
                matches.push_back(handler);
            }
        return matches;
        }
    bool _AppliesForClass(ECClassCR ecClass, bool polymorphic) const override
        {
        ECClassCP ruleClass = GetRuleClass();
        if (nullptr == ruleClass)
            {
            BeAssert(false);
            return false;
            }

        if (ecClass.Is(ruleClass))
            return true;

        if (polymorphic && ruleClass->Is(&ecClass))
            return true;

        return false;
        }
    AdvancedGroupingHandler* _AsAdvancedGroupingHandler() override {return this;}
public:
    GroupingRuleCR GetRule() const {return m_rule;}
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                07/2015
+===============+===============+===============+===============+===============+======*/
struct BaseClassGroupingHandler : AdvancedGroupingHandler
{
DEFINE_T_SUPER(AdvancedGroupingHandler)

private:
    bool m_doNotSort;
    ClassGroupCR m_specification;
    mutable NavigationQueryContractPtr m_contract;
    mutable ECClassId m_baseClassId;

private:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                06/2015
    +---------------+---------------+---------------+---------------+---------------+------*/
    ECClassId GetBaseECClassId() const
        {
        if (!m_baseClassId.IsValid())
            {
            Utf8String schemaName = m_specification.GetSchemaName();
            Utf8String baseClassName = m_specification.GetBaseClassName();
            if (schemaName.empty() || baseClassName.empty())
                {
                schemaName = m_rule.GetSchemaName();
                baseClassName = m_rule.GetClassName();
                }
            if (schemaName.empty() || baseClassName.empty())
                BeAssert(false);

            ECClassCP baseClass = m_schemaHelper.GetECClass(schemaName.c_str(), baseClassName.c_str());
            m_baseClassId = baseClass->GetId();
            }
        return m_baseClassId;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                06/2015
    +---------------+---------------+---------------+---------------+---------------+------*/
    NavigationQueryContractPtr GetContract() const
        {
        if (m_contract.IsNull())
            m_contract = BaseECClassGroupingNodesQueryContract::Create(GetBaseECClassId());
        return m_contract;
        }

protected:
    GroupingType _GetType() const override {return GroupingType::BaseClass;}
    NavigationQueryContractPtr _GetContract(SelectQueryInfo const& selectInfo) const override {return GetContract();}
    ApplyGroupingResult _ApplyClassGrouping(ComplexNavigationQueryPtr& query, SelectQueryInfo const& selectInfo) override {return ApplyGroupingResult::Handled;}

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                06/2015
    +---------------+---------------+---------------+---------------+---------------+------*/
    bool _IsAppliedTo(NavNodeCR node) const override
        {
        if (!T_Super::_IsAppliedTo(node))
            return false;

        NavNodeExtendedData extendedData(node);
        return GetBaseECClassId() == extendedData.GetECClassId();
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                06/2015
    +---------------+---------------+---------------+---------------+---------------+------*/
    bool _ApplyOuterGrouping(NavigationQueryPtr& query) override
        {
        T_Super::_ApplyOuterGrouping(query);

        NavigationQueryContractCR contract = *GetContract();
        ComplexNavigationQueryPtr complexQuery = QueryBuilderHelpers::CreateComplexNestedQueryIfNecessary<NavigationQuery>(*query, contract.GetGroupingAliases());
        complexQuery->GroupByContract(contract);
        query = complexQuery;

        if (!m_doNotSort)
            {
            static Utf8PrintfString sortedDisplayLabel("%s([%s])", FUNCTION_NAME_GetSortingValue, BaseECClassGroupingNodesQueryContract::DisplayLabelFieldName);

            bvector<Utf8CP> sortingAliases;
            sortingAliases.push_back(BaseECClassGroupingNodesQueryContract::DisplayLabelFieldName);

            query = QueryBuilderHelpers::CreateNestedQueryIfNecessary(*query, sortingAliases);
            QueryBuilderHelpers::Order(*query, sortedDisplayLabel.c_str());
            }

        if (!m_specification.GetCreateGroupForSingleItem())
            query->GetResultParametersR().GetNavNodeExtendedDataR().SetHideIfOnlyOneChild(true);

        return true;
        }

public:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                06/2015
    +---------------+---------------+---------------+---------------+---------------+------*/
    BaseClassGroupingHandler(ECSchemaHelper const& schemaHelper, GroupingRuleCR rule, ClassGroupCR specification, bool doNotSort)
        : T_Super(rule, schemaHelper), m_specification(specification), m_doNotSort(doNotSort)
        {}
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                07/2015
+===============+===============+===============+===============+===============+======*/
struct PropertyGroupingHandler : AdvancedGroupingHandler
{
DEFINE_T_SUPER(AdvancedGroupingHandler)

private:
    PropertyGroupCR m_specification;
    bool m_doNotSort;
    mutable RelatedClass m_foreignKeyClass;
    bmap<ECPropertyCP, NavigationQueryPtr> m_groupedQueries;

private:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                06/2015
    +---------------+---------------+---------------+---------------+---------------+------*/
    RefCountedPtr<ECPropertyGroupingNodesQueryContract> GetContract(Utf8CP relatedClassAlias = nullptr) const
        {
        ECClassCP ecClass = m_schemaHelper.GetECClass(m_rule.GetSchemaName().c_str(), m_rule.GetClassName().c_str());
        if (nullptr == ecClass)
            {
            BeAssert(false);
            return nullptr;
            }

        ECPropertyCP ecProperty = ecClass->GetPropertyP(m_specification.GetPropertyName().c_str());
        if (nullptr == ecProperty)
            {
            BeAssert(false);
            return nullptr;
            }

        m_foreignKeyClass = m_schemaHelper.GetForeignKeyClass(*ecProperty);
        return ECPropertyGroupingNodesQueryContract::Create(*ecClass, *ecProperty, relatedClassAlias, m_specification, m_foreignKeyClass.GetTargetClass());
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                05/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    static ECValue GetRangeValue(ECPropertyCR prop, Utf8StringCR rangeValueStr)
        {
        if (!prop.GetIsPrimitive())
            {
            BeAssert(false);
            return ECValue();
            }

        return ValueHelpers::GetECValueFromString(prop.GetAsPrimitiveProperty()->GetType(), rangeValueStr);
        }

protected:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                06/2015
    +---------------+---------------+---------------+---------------+---------------+------*/
    Utf8CP GetSelectPrefix(SelectQueryInfo const& selectInfo) const
        {
        // first, check the primary select class
        if (AppliesForClass(*selectInfo.GetSelectClass(), selectInfo.GetSelectPolymorphically()))
            return nullptr;

        // then, check the related class path in case we want to group by relationship property
        if (!selectInfo.GetRelatedClassPath().empty())
            {
            for (RelatedClass const& relatedClass : selectInfo.GetRelatedClassPath())
                {
                if (AppliesForClass(*relatedClass.GetRelationship(), true))
                    return relatedClass.GetRelationshipAlias();
                }
            }

        // finally, check the additional related classes
        for (RelatedClass const& relatedClass : selectInfo.GetRelatedClasses())
            {
            if (AppliesForClass(*relatedClass.GetTargetClass(), relatedClass.IsPolymorphic()))
                return relatedClass.GetTargetClassAlias();
            }

        return nullptr;
        }

protected:
    virtual GroupingType _GetType() const override {return GroupingType::Property;}

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                06/2015
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual NavigationQueryContractPtr _GetContract(SelectQueryInfo const& selectInfo) const override
        {
        return GetContract(GetSelectPrefix(selectInfo));
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                06/2015
    +---------------+---------------+---------------+---------------+---------------+------*/
    bool _IsAppliedTo(NavNodeCR node) const override
        {
        if (!node.GetType().Equals(NAVNODE_TYPE_ECPropertyGroupingNode))
            return false;

        NavNodeExtendedData extendedData(node);
        if (!extendedData.HasECClassId() || !extendedData.HasPropertyName())
            {
            BeAssert(false);
            return false;
            }

        ECClassCP nodeClass = m_schemaHelper.GetECClass(extendedData.GetECClassId());
        if (nullptr == nodeClass)
            {
            BeAssert(false);
            return false;
            }

        return nodeClass->Is(m_rule.GetSchemaName().c_str(), m_rule.GetClassName().c_str()) && m_specification.GetPropertyName().Equals(extendedData.GetPropertyName());
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                05/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    NavigationQueryPtr _GetStoredQuery() const override
        {
        RefCountedPtr<ECPropertyGroupingNodesQueryContract> contract = GetContract();
        if (contract.IsNull())
            {
            BeAssert(false);
            return nullptr;
            }
        auto iter = m_groupedQueries.find(&contract->GetProperty());
        if (m_groupedQueries.end() == iter)
            return nullptr;
        return iter->second;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                06/2015
    +---------------+---------------+---------------+---------------+---------------+------*/
    ApplyGroupingResult _ApplyClassGrouping(ComplexNavigationQueryPtr& query, SelectQueryInfo const& selectInfo) override
        {
        if (m_foreignKeyClass.IsValid())
            {
            Utf8String relationshipAlias = Utf8String("rel_").append(m_foreignKeyClass.GetRelationship()->GetSchema().GetAlias()).append("_").append(m_foreignKeyClass.GetRelationship()->GetName());
            if (m_foreignKeyClass.GetSourceClass()->IsEntityClass())
                {
                RelatedClass relatedParent(*selectInfo.GetSelectClass(), *m_foreignKeyClass.GetTargetClass(), *m_foreignKeyClass.GetRelationship(),
                    !m_foreignKeyClass.IsForwardRelationship(), "parentInstance", relationshipAlias.c_str());
                query->Join(relatedParent);
                }
            else
                {
                BeAssert(!selectInfo.GetRelatedClassPath().empty());
                RelatedClassPath joinPath = selectInfo.GetRelatedClassPath();
                for (auto iter = joinPath.begin(); iter != joinPath.end(); ++iter)
                    {
                    RelatedClassCR related = *iter;
                    if (m_foreignKeyClass.GetSourceClass() == related.GetRelationship())
                        {
                        joinPath.erase(iter + 1, joinPath.end());
                        break;
                        }
                    }
                joinPath.push_back(RelatedClass(*m_foreignKeyClass.GetSourceClass(), *m_foreignKeyClass.GetTargetClass(), *m_foreignKeyClass.GetRelationship(),
                    !m_foreignKeyClass.IsForwardRelationship(), "parentInstance", relationshipAlias.c_str()));
                query->Join(joinPath, true);
                }
            }

        RefCountedPtr<ECPropertyGroupingNodesQueryContract> contract = GetContract();
        if (contract.IsNull())
            {
            BeAssert(false);
            return ApplyGroupingResult::Error;
            }

        auto iter = m_groupedQueries.find(&contract->GetProperty());
        if (m_groupedQueries.end() == iter)
            m_groupedQueries[&contract->GetProperty()] = query;
        else
            m_groupedQueries[&contract->GetProperty()] = UnionNavigationQuery::Create(*iter->second, *query);

        return ApplyGroupingResult::Stored;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                06/2015
    +---------------+---------------+---------------+---------------+---------------+------*/
    bool _ApplyOuterGrouping(NavigationQueryPtr& query) override
        {
        RefCountedPtr<ECPropertyGroupingNodesQueryContract> contract = GetContract();
        if (contract.IsValid())
            {
            ComplexNavigationQueryPtr complexQuery = QueryBuilderHelpers::CreateComplexNestedQueryIfNecessary<NavigationQuery>(*query, contract->GetGroupingAliases());
            complexQuery->GroupByContract(*contract);
            query = complexQuery;
            }

        if (!m_doNotSort)
            {
            Utf8String orderByClause = nullptr;
            switch (m_specification.GetSortingValue())
                {
                case PropertyGroupingValue::PropertyValue:
                    if (PropertyGroupingValue::PropertyValue != m_specification.GetPropertyGroupingValue())
                        {
                        LoggingHelper::LogMessage(Log::Navigation, "Sorting property grouping nodes by property value is only possible when they are grouped by property value. "
                            "Switching to display label grouping.", NativeLogging::LOG_ERROR);
                        }
                    else
                        {
                        ECPropertyCR groupingProperty = GetContract()->GetProperty();
                        ECEnumerationCP enumeration = groupingProperty.GetIsPrimitive() ? groupingProperty.GetAsPrimitiveProperty()->GetEnumeration() : nullptr;
                        PrimitiveECPropertyCP primitiveGroupingProperty = groupingProperty.GetAsPrimitiveProperty();
                        if (groupingProperty.GetIsPrimitive() && (PRIMITIVETYPE_String == primitiveGroupingProperty->GetType() || nullptr != enumeration))
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
                        query = QueryBuilderHelpers::CreateNestedQueryIfNecessary(*query, {ECPropertyGroupingNodesQueryContract::GroupingValueFieldName});
                        break;
                        }
                case PropertyGroupingValue::DisplayLabel:
                    orderByClause = Utf8PrintfString("%s([%s])", FUNCTION_NAME_GetSortingValue, ECPropertyGroupingNodesQueryContract::DisplayLabelFieldName);
                    query = QueryBuilderHelpers::CreateNestedQueryIfNecessary(*query, {ECPropertyGroupingNodesQueryContract::DisplayLabelFieldName});
                    break;
                default:
                    BeAssert(false);
                }
            if (!orderByClause.empty())
                QueryBuilderHelpers::Order(*query, orderByClause.c_str());
            }

        if (!m_specification.GetCreateGroupForSingleItem())
            query->GetResultParametersR().GetNavNodeExtendedDataR().SetHideIfOnlyOneChild(true);

        if (!m_specification.GetCreateGroupForUnspecifiedValues() && m_specification.GetRanges().empty())
            query->GetResultParametersR().GetNavNodeExtendedDataR().SetHideIfGroupingValueNotSpecified(true);

        NavigationQueryExtendedData(*query).AddRangesData(GetContract()->GetProperty(), m_specification);

        T_Super::_ApplyOuterGrouping(query);
        return true;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                06/2015
    +---------------+---------------+---------------+---------------+---------------+------*/
    bool _ApplyFilter(ComplexNavigationQueryPtr& query, SelectQueryInfo const& selectInfo, NavNodeCR filteringNode) const override
        {
        NavNodeExtendedData extendedData(filteringNode);
        if (!extendedData.HasECClassId() || !extendedData.HasPropertyName() || !(extendedData.HasPropertyValue() || extendedData.HasPropertyValueRangeIndex()))
            {
            BeAssert(false);
            return false;
            }
        ECClassCP ecPropertyClass = m_schemaHelper.GetECClass(extendedData.GetECClassId());
        if (nullptr == ecPropertyClass)
            {
            BeAssert(false);
            return false;
            }
        ECPropertyCP ecProperty = ecPropertyClass->GetPropertyP(extendedData.GetPropertyName());
        if (nullptr == ecProperty)
            {
            BeAssert(false);
            return false;
            }

        Utf8CP prefix = GetSelectPrefix(selectInfo);
        if (nullptr == prefix)
            prefix = query->GetSelectPrefix();

        BoundQueryValuesList whereClauseBindings;
        Utf8String whereClause;
        if (ecProperty->GetIsPrimitive() && PRIMITIVETYPE_Point3d == ecProperty->GetAsPrimitiveProperty()->GetType())
            {
            whereClause.Sprintf("%s([%s].[%s].x, [%s].[%s].y, [%s].[%s].z)", FUNCTION_NAME_GetPointAsJsonString,
                prefix, extendedData.GetPropertyName(), prefix, extendedData.GetPropertyName(), prefix, extendedData.GetPropertyName());
            }
        else if (ecProperty->GetIsPrimitive() && PRIMITIVETYPE_Point2d == ecProperty->GetAsPrimitiveProperty()->GetType())
            {
            whereClause.Sprintf("%s([%s].[%s].x, [%s].[%s].y)", FUNCTION_NAME_GetPointAsJsonString,
                prefix, extendedData.GetPropertyName(), prefix, extendedData.GetPropertyName());
            }
        else
            {
            whereClause.Sprintf("[%s].[%s]", prefix, extendedData.GetPropertyName());
            }

        if (ecProperty->GetIsNavigation())
            whereClause.append(".[Id]");
        if (extendedData.HasPropertyValue())
            {
            rapidjson::Value const* value = extendedData.GetPropertyValue();
            if (value->IsNull())
                {
                whereClause.append(" IS NULL");
                }
            else if (value->IsArray())
                {
                // WIP: use IdsFilteringHelper
                if (ecProperty->GetIsNavigation())
                    {
                    IdsFilteringHelper<IdSet<BeInt64Id>> helper(QueryBuilderHelpers::CreateIdSetFromJsonArray(*value));
                    whereClause = helper.CreateWhereClause(whereClause.c_str());
                    whereClauseBindings = helper.CreateBoundValues();
                    }
                else
                    {
                    whereClause = Utf8String("InVirtualSet(?, ").append(whereClause).append(")");
                    whereClauseBindings.push_back(new BoundRapidJsonValueSet(*value, ecProperty->GetAsPrimitiveProperty()->GetType()));
                    }
                }
            else
                {
                whereClause.append(" = ?");
                if (ecProperty->GetIsNavigation())
                    whereClauseBindings.push_back(new BoundQueryId(ECInstanceId(value->GetUint64())));
                else
                    whereClauseBindings.push_back(new BoundQueryECValue(QueryBuilderHelpers::CreateECValueFromJson(*value)));
                }
            }
        else if (extendedData.HasPropertyValueRangeIndex())
            {
            if (extendedData.GetPropertyValueRangeIndex() < 0)
                {
                bool first = true;
                for (PropertyRangeGroupSpecificationCP range : m_specification.GetRanges())
                    {
                    if (!first)
                        whereClause.append(Utf8PrintfString(" AND [%s].[%s]", query->GetSelectPrefix(), extendedData.GetPropertyName()));
                    whereClause.append(" NOT BETWEEN ? AND ?");
                    whereClauseBindings.push_back(new BoundQueryECValue(GetRangeValue(*ecProperty, range->GetFromValue())));
                    whereClauseBindings.push_back(new BoundQueryECValue(GetRangeValue(*ecProperty, range->GetToValue())));
                    first = false;
                    }
                }
            else if (extendedData.GetPropertyValueRangeIndex() < (int)m_specification.GetRanges().size())
                {
                PropertyRangeGroupSpecificationCP range = m_specification.GetRanges()[extendedData.GetPropertyValueRangeIndex()];
                whereClause.append(" BETWEEN ? AND ?");
                whereClauseBindings.push_back(new BoundQueryECValue(GetRangeValue(*ecProperty, range->GetFromValue())));
                whereClauseBindings.push_back(new BoundQueryECValue(GetRangeValue(*ecProperty, range->GetToValue())));
                }
            else
                {
                BeAssert(false);
                }
            }

        query->Where(whereClause.c_str(), whereClauseBindings, true);
        return true;
        }

public:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                06/2015
    +---------------+---------------+---------------+---------------+---------------+------*/
    PropertyGroupingHandler(ECSchemaHelper const& schemaHelper, GroupingRuleCR rule, PropertyGroupCR specification, bool doNotSort)
        : T_Super(rule, schemaHelper), m_specification(specification), m_doNotSort(doNotSort)
        {}
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                07/2015
+===============+===============+===============+===============+===============+======*/
struct SameLabelInstanceGroupingHandler : AdvancedGroupingHandler
{
DEFINE_T_SUPER(AdvancedGroupingHandler)

protected:
    GroupingType _GetType() const override {return GroupingType::SameLabelInstance;}
    NavigationQueryContractPtr _GetContract(SelectQueryInfo const& selectInfo) const override
        {
        return MultiECInstanceNodesQueryContract::Create(selectInfo.GetSelectClass(), true, selectInfo.GetRelatedClasses(), selectInfo.GetLabelOverrideValueSpecs());
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                11/2019
    +---------------+---------------+---------------+---------------+---------------+------*/
    bool _IsAppliedTo(NavNodeCR node) const override
        {
        if (!node.GetKey()->AsECInstanceNodeKey())
            return false;

        ECClassCP ecClass = m_schemaHelper.GetECClass(m_rule.GetSchemaName().c_str(), m_rule.GetClassName().c_str());
        if (nullptr == ecClass)
            {
            BeAssert(false);
            return false;
            }

        for (ECClassInstanceKeyCR key : node.GetKey()->AsECInstanceNodeKey()->GetInstanceKeys())
            {
            if (!key.GetClass()->Is(ecClass))
                return false;
            }

        return true;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                11/2019
    +---------------+---------------+---------------+---------------+---------------+------*/
    bool _ApplyFilter(ComplexNavigationQueryPtr& query, SelectQueryInfo const& selectInfo, NavNodeCR filteringNode) const override
        {
        NavNodeExtendedData extendedData(filteringNode);
        IdsFilteringHelper<IdSet<ECInstanceId>> filteringHelper(extendedData.GetInstanceIds());
        PresentationQueryContractFieldCPtr ecInstanceIdField = query->GetContract()->GetField(ECInstanceNodesQueryContract::ECInstanceIdFieldName);
        query->Where(filteringHelper.CreateWhereClause(ecInstanceIdField->GetSelectClause(query->GetSelectPrefix()).c_str()).c_str(), filteringHelper.CreateBoundValues());
        return true;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                06/2015
    +---------------+---------------+---------------+---------------+---------------+------*/
    ApplyGroupingResult _ApplyClassGrouping(ComplexNavigationQueryPtr& query, SelectQueryInfo const& selectInfo) override
        {
        NavigationQueryContractPtr contract = GetContract(selectInfo);
        query = QueryBuilderHelpers::CreateComplexNestedQueryIfNecessary<NavigationQuery>(*query, contract->GetGroupingAliases());
        query->GroupByContract(*contract);
        return ApplyGroupingResult::Handled;
        }

public:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                06/2015
    +---------------+---------------+---------------+---------------+---------------+------*/
    SameLabelInstanceGroupingHandler(ECSchemaHelper const& schemaHelper, GroupingRuleCR rule)
        : T_Super(rule, schemaHelper)
        {}
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                07/2015
+===============+===============+===============+===============+===============+======*/
struct GroupingNodeAndHandler
{
private:
    NavNodeCPtr m_node;
    GroupingHandler const* m_handler;
public:
    GroupingNodeAndHandler() : m_node(nullptr), m_handler(nullptr) {}
    GroupingNodeAndHandler(NavNodeCR node, GroupingHandler const& handler) : m_node(&node), m_handler(&handler) {}
    NavNodeCR GetNode() const {return *m_node;}
    GroupingHandler const& GetHandler() const {return *m_handler;}
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                06/2015
+===============+===============+===============+===============+===============+======*/
struct GroupingResolver
{
    /*=============================================================================**//**
    * @bsiclass                                 Grigas.Petraitis                06/2015
    +===============+===============+===============+===============+===============+==*/
    struct GroupingSpecificationsVisitor : GroupingRuleSpecificationVisitor
    {
    private:
        ECSchemaHelper const& m_schemaHelper;
        bool m_doNotSort;
        GroupingRuleCP m_rule;
        GroupingHandlersList& m_groupingHandlers;

    protected:
        virtual void _Visit(SameLabelInstanceGroupCR specification)
            {
            if (specification.GetApplicationStage() == SameLabelInstanceGroupApplicationStage::Query)
                m_groupingHandlers.push_back(new SameLabelInstanceGroupingHandler(m_schemaHelper, *m_rule));
            }
        virtual void _Visit(ClassGroupCR specification) {m_groupingHandlers.push_back(new BaseClassGroupingHandler(m_schemaHelper, *m_rule, specification, m_doNotSort));}
        virtual void _Visit(PropertyGroupCR specification) {m_groupingHandlers.push_back(new PropertyGroupingHandler(m_schemaHelper, *m_rule, specification, m_doNotSort));}

    public:
        GroupingSpecificationsVisitor(GroupingHandlersList& groupingHandlers, ECSchemaHelper const& schemaHelper, bool doNotSort)
            : m_groupingHandlers(groupingHandlers), m_schemaHelper(schemaHelper), m_doNotSort(doNotSort)
            {}
        void SetRule(GroupingRuleCR rule) {m_rule = &rule;}
    };

private:
    ECSchemaHelper const& m_schemaHelper;
    NavigationQueryBuilderParameters const& m_queryBuilderParams;
    ChildNodeSpecificationCR m_specification;
    Utf8StringCR m_specificationHash;
    JsonNavNodeCP m_parentNode;
    JsonNavNodeCPtr m_parentInstanceNode;

    GroupingHandlersList m_groupingHandlers;
    GroupingHandler* m_parentGrouping;
    ECEntityClassCP m_groupingClass;
    ECRelationshipClassCP m_groupingRelationship;
    ECRelatedInstanceDirection m_groupingRelationshipDirection;

private:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                06/2015
    +---------------+---------------+---------------+---------------+---------------+------*/
    bool SpecificationMatches(NavNodeCR node) const
        {
        NavNodeExtendedData extendedData(node);
        return extendedData.HasSpecificationHash() && 0 == strcmp(extendedData.GetSpecificationHash(), m_specificationHash.c_str());
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                06/2015
    +---------------+---------------+---------------+---------------+---------------+------*/
    void Init(JsonNavNodeCP node)
        {
        m_parentNode = node;
        m_parentInstanceNode = m_parentNode;
        m_parentGrouping = nullptr;
        m_groupingClass = nullptr;
        m_groupingRelationship = nullptr;

        while (m_parentInstanceNode.IsValid() && nullptr == m_parentInstanceNode->GetKey()->AsECInstanceNodeKey())
            {
            uint64_t parentId = NavNodeExtendedData(*m_parentInstanceNode).GetVirtualParentId();
            if (0 == parentId)
                parentId = m_parentInstanceNode->GetParentNodeId();
            m_parentInstanceNode = (0 != parentId) ? m_queryBuilderParams.GetNodesCache().GetNode(parentId).get() : nullptr;
            }
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                06/2015
    +---------------+---------------+---------------+---------------+---------------+------*/
    void ResolveGrouping(bool groupByRelationship, bool groupByClass, bool groupByLabel)
        {
        if (groupByRelationship)
            m_groupingHandlers.push_back(new RelationshipGroupingHandler());
        if (groupByClass)
            m_groupingHandlers.push_back(new ClassGroupingHandler(m_specification.GetDoNotSort()));
        if (groupByLabel)
            m_groupingHandlers.push_back(new DisplayLabelGroupingHandler(m_specification.GetDoNotSort()));

        GroupingSpecificationsVisitor visitor(m_groupingHandlers, m_schemaHelper, m_specification.GetDoNotSort());
        RulesPreprocessor preprocessor(m_queryBuilderParams.GetConnections(), m_queryBuilderParams.GetConnection(),
            GetQueryBuilderParams().GetRuleset(), GetQueryBuilderParams().GetLocale(), GetQueryBuilderParams().GetUserSettings(),
            GetQueryBuilderParams().GetUsedSettingsListener(), GetQueryBuilderParams().GetECExpressionsCache());
        RulesPreprocessor::AggregateCustomizationRuleParameters params(m_parentInstanceNode.get(), m_specificationHash);
        bvector<GroupingRuleCP> groupingRules = preprocessor.GetGroupingRules(params);
        for (GroupingRuleCP rule : groupingRules)
            {
            if (rule->GetGroups().empty())
                continue;

            visitor.SetRule(*rule);

            GroupSpecificationCP activeSpecification = QueryBuilderHelpers::GetActiveGroupingSpecification(*rule, GetQueryBuilderParams().GetLocalState());
            if (nullptr != activeSpecification)
                activeSpecification->Accept(visitor);
            }

        std::sort(m_groupingHandlers.begin(), m_groupingHandlers.end(), GroupingHandler::Compare());

        ResolveParentGrouping();
        ResolveGroupingClass();
        ResolveGroupingRelationship();
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                07/2015
    +---------------+---------------+---------------+---------------+---------------+------*/
    void ResolveParentGrouping()
        {
        if (nullptr == m_parentNode)
            return;

        NavNodeExtendedData extendedData(*m_parentNode);
        if (!extendedData.HasGroupingType())
            return;

        for (auto iter = m_groupingHandlers.rbegin(); iter != m_groupingHandlers.rend(); ++iter)
            {
            GroupingHandler* handler = *iter;
            if (handler->IsAppliedTo(*m_parentNode))
                {
                m_parentGrouping = handler;
                return;
                }
            }

        BeAssert(nullptr != m_parentGrouping);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                07/2015
    +---------------+---------------+---------------+---------------+---------------+------*/
    void ResolveGroupingClass()
        {
        ECEntityClassCP bestClassMatch = nullptr;
        RefCountedPtr<NavNode const> node = m_parentNode;
        while (node.IsValid() && SpecificationMatches(*node))
            {
            if (node->GetType().Equals(NAVNODE_TYPE_ECClassGroupingNode))
                {
                ECClassCP candidateClass = m_schemaHelper.GetECClass(NavNodeExtendedData(*node).GetECClassId());
                if (nullptr != candidateClass)
                    {
                    ECEntityClassCP candidateEntity = candidateClass->GetEntityClassCP();
                    if (nullptr == candidateEntity)
                        {
                        BeAssert(false);
                        }
                    else
                        {
                        NavNodeExtendedData extendedData(*node);
                        if (GroupingType::Class == (GroupingType)extendedData.GetGroupingType())
                            {
                            if (nullptr == bestClassMatch || candidateEntity->Is(bestClassMatch))
                                bestClassMatch = candidateEntity;
                            }
                        }
                    }
                }
            node = (0 != node->GetParentNodeId()) ? GetQueryBuilderParams().GetNodesCache().GetNode(node->GetParentNodeId()).get() : nullptr;
            }
        m_groupingClass = bestClassMatch;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                07/2015
    +---------------+---------------+---------------+---------------+---------------+------*/
    void ResolveGroupingRelationship()
        {
        JsonNavNodeCPtr node = m_parentNode;
        while (node.IsValid() && SpecificationMatches(*node))
            {
            if (node->GetType().Equals(NAVNODE_TYPE_ECRelationshipGroupingNode))
                {
                ECClassCP candidateClass = m_schemaHelper.GetECClass(NavNodeExtendedData(*node).GetECClassId());
                if (nullptr != candidateClass && nullptr != candidateClass->GetRelationshipClassCP())
                    {
                    NavNodeExtendedData extendedData(*node);
                    BeAssert(extendedData.HasRelationshipDirection());
                    m_groupingRelationship = candidateClass->GetRelationshipClassCP();
                    m_groupingRelationshipDirection = extendedData.GetRelationshipDirection();
                    return;
                    }
                }
            node = (0 != node->GetParentNodeId()) ? GetQueryBuilderParams().GetNodesCache().GetNode(node->GetParentNodeId()).get() : nullptr;
            }
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                07/2015
    +---------------+---------------+---------------+---------------+---------------+------*/
    GroupingHandlersList GetGroupingHandlers(GroupingType thisLevel) const
        {
        GroupingHandlersList handlers;
        int currentLevel = (int)thisLevel;
        while (0 <= currentLevel && handlers.empty())
            {
            for (GroupingHandler* handler : m_groupingHandlers)
                {
                if ((int)handler->GetType() < currentLevel)
                    break;

                if ((int)handler->GetType() == currentLevel)
                    handlers.push_back(handler);
                }
            currentLevel--;
            }
        return handlers;
        }

public:
    GroupingResolver(ECSchemaHelper const& schemaHelper, NavigationQueryBuilderParameters const& params, JsonNavNodeCP node, Utf8StringCR specificationHash, AllInstanceNodesSpecificationCR specification)
        : m_schemaHelper(schemaHelper), m_queryBuilderParams(params), m_specification(specification), m_specificationHash(specificationHash)
        {
        Init(node);
        if (!specification.GetHideNodesInHierarchy())
            ResolveGrouping(false, specification.GetGroupByClass(), specification.GetGroupByLabel());
        }
    GroupingResolver(ECSchemaHelper const& schemaHelper, NavigationQueryBuilderParameters const& params, JsonNavNodeCP node, Utf8StringCR specificationHash, AllRelatedInstanceNodesSpecificationCR specification)
        : m_schemaHelper(schemaHelper), m_queryBuilderParams(params), m_specification(specification), m_specificationHash(specificationHash)
        {
        Init(node);
        if (!specification.GetHideNodesInHierarchy())
            ResolveGrouping(specification.GetGroupByRelationship(), specification.GetGroupByClass(), specification.GetGroupByLabel());
        }
    GroupingResolver(ECSchemaHelper const& schemaHelper, NavigationQueryBuilderParameters const& params, JsonNavNodeCP node, Utf8StringCR specificationHash, RelatedInstanceNodesSpecificationCR specification)
        : m_schemaHelper(schemaHelper), m_queryBuilderParams(params), m_specification(specification), m_specificationHash(specificationHash)
        {
        Init(node);
        if (!specification.GetHideNodesInHierarchy())
            ResolveGrouping(specification.GetGroupByRelationship(), specification.GetGroupByClass(), specification.GetGroupByLabel());
        }
    GroupingResolver(ECSchemaHelper const& schemaHelper, NavigationQueryBuilderParameters const& params, JsonNavNodeCP node, Utf8StringCR specificationHash, InstanceNodesOfSpecificClassesSpecificationCR specification)
        : m_schemaHelper(schemaHelper), m_queryBuilderParams(params), m_specification(specification), m_specificationHash(specificationHash)
        {
        Init(node);
        if (!specification.GetHideNodesInHierarchy())
            ResolveGrouping(false, specification.GetGroupByClass(), specification.GetGroupByLabel());
        }
    GroupingResolver(ECSchemaHelper const& schemaHelper, NavigationQueryBuilderParameters const& params, JsonNavNodeCP node, Utf8StringCR specificationHash, SearchResultInstanceNodesSpecificationCR specification)
        : m_schemaHelper(schemaHelper), m_queryBuilderParams(params), m_specification(specification), m_specificationHash(specificationHash)
        {
        Init(node);
        if (!specification.GetHideNodesInHierarchy())
            ResolveGrouping(false, specification.GetGroupByClass(), specification.GetGroupByLabel());
        }

    ~GroupingResolver()
        {
        for (GroupingHandler* handler : m_groupingHandlers)
            delete handler;
        }

    JsonNavNodeCP GetParentNode() const {return m_parentNode;}
    JsonNavNodeCP GetParentInstanceNode() const {return m_parentInstanceNode.get();}
    NavigationQueryBuilderParameters const& GetQueryBuilderParams() const {return m_queryBuilderParams;}
    ChildNodeSpecificationCR GetSpecification() const {return m_specification;}
    Utf8StringCR GetSpecificationHash() const {return m_specificationHash;}
    ECSchemaHelper const& GetSchemaHelper() const {return m_schemaHelper;}

    bool IsGroupedByClass() const {return nullptr != m_groupingClass;}
    ECEntityClassCP GetGroupingClass() const {return m_groupingClass;}

    bool IsGroupedByRelationship() const {return nullptr != m_groupingRelationship;}
    ECRelationshipClassCP GetGroupingRelationship() const {return m_groupingRelationship;}
    ECRelatedInstanceDirection GetGroupingRelationshipDirection() const {return m_groupingRelationshipDirection;}

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                11/2016
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

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                07/2015
    +---------------+---------------+---------------+---------------+---------------+------*/
    bvector<GroupingHandler*> GetHandlersForNextGroupingLevel(bvector<ECClassCP> const& selectClasses, bool polymorphic) const
        {
        bvector<GroupingHandler*> matchingHandlers;
        if (m_groupingHandlers.empty())
            return matchingHandlers;

        if (nullptr != m_parentGrouping)
            {
            if (0 == (int)m_parentGrouping->GetType())
                return matchingHandlers;

            // check if there're any handlers of the same grouping type as the
            // parent (the case of multiple nested property grouping nodes)
            GroupingType parentGroupingType = m_parentGrouping->GetType();
            GroupingHandlersList handlersOfTheSameType = GetGroupingHandlers(parentGroupingType);
            bool foundParentGroupingHandler = false;

            for (auto iter = handlersOfTheSameType.begin(); iter != handlersOfTheSameType.end(); ++iter)
                {
                GroupingHandler* handler = *iter;
                if (foundParentGroupingHandler && DoesHandlerApply(*handler, selectClasses, polymorphic))
                    {
                    matchingHandlers.push_back(handler);
                    return matchingHandlers;
                    }

                if (m_parentGrouping == handler)
                    {
                    foundParentGroupingHandler = true;
                    GroupingHandlersList parentMatches = m_parentGrouping->FindMatchingHandlersOfTheSameType(iter + 1, handlersOfTheSameType.end());
                    GroupingHandlersList parentMatchesThatApply;
                    std::copy_if(parentMatches.begin(), parentMatches.end(), std::back_inserter(parentMatchesThatApply),
                        [&](GroupingHandler* handler){return DoesHandlerApply(*handler, selectClasses, polymorphic);});
                    if (!parentMatchesThatApply.empty())
                        return parentMatchesThatApply;
                    }
                }
            }

        int nextGroupingType = (nullptr == m_parentGrouping) ? (int)GroupingType::Relationship : ((int)m_parentGrouping->GetType() - 1);
        GroupingHandlersList handlers = GetGroupingHandlers((GroupingType)nextGroupingType--);
        while (!handlers.empty())
            {
            for (GroupingHandler* handler : handlers)
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
    * @bsimethod                                    Grigas.Petraitis                07/2015
    +---------------+---------------+---------------+---------------+---------------+------*/
    bvector<GroupingNodeAndHandler> GetFilterHandlers() const
        {
        bvector<GroupingNodeAndHandler> result;

        // get filtering grouping handlers
        GroupingHandlersList filters;
        for (GroupingHandler* handler : m_groupingHandlers)
            {
            filters.push_back(handler);
            if (m_parentGrouping == handler)
                break;
            }

        // match handlers with grouping nodes
        GroupingHandlersList::const_reverse_iterator iter = filters.rbegin();
        NavNodeCPtr node = m_parentNode;
        while (node.IsValid() && SpecificationMatches(*node) && iter != filters.rend())
            {
            GroupingHandler* handler = *iter++;
            if (!handler->IsAppliedTo(*node))
                continue;

            result.push_back(GroupingNodeAndHandler(*node, *handler));
            node = (0 != node->GetParentNodeId()) ? GetQueryBuilderParams().GetNodesCache().GetNode(node->GetParentNodeId()).get() : nullptr;
            }

        return result;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                03/2016
    +---------------+---------------+---------------+---------------+---------------+------*/
    bvector<GroupingRuleCP> GetAppliedGroupingRules(bvector<ECClassCP> const& selectClasses) const
        {
        bvector<GroupingRuleCP> rules;

        bvector<GroupingNodeAndHandler> filterHandlers = GetFilterHandlers();
        for (GroupingNodeAndHandler const& handler : filterHandlers)
            {
            AdvancedGroupingHandler const* advancedHandler = handler.GetHandler().AsAdvancedGroupingHandler();
            if (nullptr != advancedHandler)
                rules.push_back(&advancedHandler->GetRule());
            }

        bvector<GroupingHandler*> nextLevelHandlers = GetHandlersForNextGroupingLevel(selectClasses, true);
        for (GroupingHandler const* handler : nextLevelHandlers)
            {
            if (nullptr != handler->AsAdvancedGroupingHandler())
                rules.push_back(&handler->AsAdvancedGroupingHandler()->GetRule());
            }

        return rules;
        }
};

typedef RefCountedPtr<struct IQueryContext> IQueryContextPtr;
/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                07/2015
+===============+===============+===============+===============+===============+======*/
struct IQueryContext : RefCountedBase
{
private:
    IQueryContextPtr m_base;
    ECSchemaHelper const* m_schemaHelper;
    IConnectionManagerCP m_connections;
    IConnectionCP m_connection;
    PresentationRuleSetCP m_ruleset;
    Utf8StringCP m_locale;
    JsonNavNodeCP m_parentNode;
    JsonNavNodeCP m_parentInstanceNode;

protected:
    IQueryContext(ECSchemaHelper const& schemaHelper, IConnectionManagerCR connections, IConnectionCR connection,
        PresentationRuleSetCR ruleset, Utf8StringCR locale, JsonNavNodeCP parentNode, JsonNavNodeCP parentInstanceNode)
        : m_schemaHelper(&schemaHelper), m_connections(&connections), m_connection(&connection), m_ruleset(&ruleset),
        m_locale(&locale), m_parentNode(parentNode), m_parentInstanceNode(parentInstanceNode)
        {}
    IQueryContext(IQueryContext& base)
        : m_base(&base), m_connections(nullptr), m_connection(nullptr), m_schemaHelper(nullptr), m_ruleset(nullptr),
        m_locale(nullptr), m_parentNode(nullptr), m_parentInstanceNode(nullptr)
        {}
    IQueryContextPtr GetBaseContext() const {return m_base;}

    virtual bool _Accept(NavigationQueryPtr& query, SelectQueryInfo const& selectInfo) = 0;
    virtual void _DiscardQueries() = 0;
    virtual NavigationQueryPtr _GetQuery() const = 0;
    virtual NavigationQueryContractPtr _GetContract(SelectQueryInfo const&) const = 0;

public:
    bool Accept(NavigationQueryPtr& query, SelectQueryInfo const& selectInfo) {return _Accept(query, selectInfo);}
    void DiscardQueries() {_DiscardQueries(); }
    NavigationQueryPtr GetQuery() const {return _GetQuery();}
    ECSchemaHelper const& GetSchemaHelper() const {return m_base.IsValid() ? m_base->GetSchemaHelper() : *m_schemaHelper;}
    IConnectionManagerCR GetConnections() const {return m_base.IsValid() ? m_base->GetConnections() : *m_connections;}
    IConnectionCR GetConnection() const {return m_base.IsValid() ? m_base->GetConnection() : *m_connection;}
    PresentationRuleSetCR GetRuleset() const {return m_base.IsValid() ? m_base->GetRuleset() : *m_ruleset;}
    Utf8StringCR GetLocale() const {return m_base.IsValid() ? m_base->GetLocale() : *m_locale;}
    JsonNavNodeCP GetParentNode() const {return m_base.IsValid() ? m_base->GetParentNode() : m_parentNode;}
    JsonNavNodeCP GetParentInstanceNode() const {return m_base.IsValid() ? m_base->GetParentInstanceNode() : m_parentInstanceNode;}
    NavigationQueryContractPtr GetContract(SelectQueryInfo const& selectInfo) const {return _GetContract(selectInfo);}
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                07/2015
+===============+===============+===============+===============+===============+======*/
struct BaseQueryContext : IQueryContext
{
private:
    bvector<NavigationQueryPtr> m_queriesIncluded;
    bvector<NavigationQueryPtr> m_queriesExcluded;

protected:
    BaseQueryContext(IQueryContext& base) : IQueryContext(base) {}
    BaseQueryContext(ECSchemaHelper const& schemaHelper, IConnectionManagerCR connections, IConnectionCR connection, PresentationRuleSetCR ruleset,
        Utf8StringCR locale, JsonNavNodeCP parentNode, JsonNavNodeCP parentInstanceNode)
        : IQueryContext(schemaHelper, connections, connection, ruleset, locale, parentNode, parentInstanceNode)
        {}
    bvector<NavigationQueryPtr> const& GetIncludedQueries() const {return m_queriesIncluded;}
    bvector<NavigationQueryPtr> const& GetExcludedQueries() const {return m_queriesExcluded;}

    bool _Accept(NavigationQueryPtr& query, SelectQueryInfo const& selectInfo) override
        {
        if (selectInfo.GetSelectFlag() == SelectionPurpose::Include)
            m_queriesIncluded.push_back(query);
        else
            m_queriesExcluded.push_back(query);
        return true;
        }
    void _DiscardQueries() override
        {
        m_queriesIncluded.clear();
        m_queriesExcluded.clear();
        }
    NavigationQueryPtr _GetQuery() const override
        {
        NavigationQueryPtr query;
        for (NavigationQueryPtr includedQuery : m_queriesIncluded)
            QueryBuilderHelpers::SetOrUnion(query, *includedQuery);

        if (query.IsValid())
            {
            for (NavigationQueryPtr excludedQuery : m_queriesExcluded)
                query = ExceptNavigationQuery::Create(*query, *excludedQuery);
            }

        return query;
        }
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                07/2015
+===============+===============+===============+===============+===============+======*/
struct ECInstanceQueryContext : BaseQueryContext
{
protected:
    ECInstanceQueryContext(ECSchemaHelper const& schemaHelper, IConnectionManagerCR connections, IConnectionCR connection, PresentationRuleSetCR ruleset,
        Utf8StringCR locale, JsonNavNodeCP parentNode, JsonNavNodeCP parentInstanceNode)
        : BaseQueryContext(schemaHelper, connections, connection, ruleset, locale, parentNode, parentInstanceNode)
        {}
    NavigationQueryContractPtr _GetContract(SelectQueryInfo const& selectInfo) const override
        {
        return ECInstanceNodesQueryContract::Create(selectInfo.GetSelectClass(), selectInfo.GetRelatedClasses(), selectInfo.GetLabelOverrideValueSpecs());
        }
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                07/2015
+===============+===============+===============+===============+===============+======*/
struct ECInstanceSortingQueryContext : ECInstanceQueryContext
{
    struct ClassSortingRule
    {
    private:
        ECClassCP m_class;
        Utf8String m_classAlias;
        SortingRuleCP m_rule;
    public:
        ClassSortingRule() : m_class(nullptr), m_rule(nullptr) {}
        ClassSortingRule(ECClassCR ecClass, Utf8CP classAlias, SortingRuleCR rule) : m_class(&ecClass), m_classAlias(classAlias), m_rule(&rule) {}
        ECClassCP GetClass() const {return m_class;}
        Utf8StringCR GetClassAlias() const {return m_classAlias;}
        SortingRuleCP GetRule() const {return m_rule;}
    };

private:
    bool m_doNotSort;
    mutable bvector<SortingRuleCP> const* m_sortingRules;
    bvector<NavigationQueryPtr> m_queriesRulesSorted;
    bvector<NavigationQueryPtr> m_queriesLabelSorted;
    IUserSettings const& m_userSettings;
    IUsedUserSettingsListener* m_usedSettingsListener;
    ECExpressionsCache& m_ecexpressionsCache;
    Utf8StringCR m_specificationHash;

private:
    bvector<SortingRuleCP> const& GetAllSortingRules() const
        {
        if (nullptr == m_sortingRules)
            {
            RulesPreprocessor preprocessor(GetConnections(), GetConnection(), GetRuleset(), GetLocale(), m_userSettings,
                m_usedSettingsListener, m_ecexpressionsCache);
            RulesPreprocessor::AggregateCustomizationRuleParameters params(GetParentInstanceNode(), m_specificationHash);
            m_sortingRules = new bvector<SortingRuleCP>(preprocessor.GetSortingRules(params));
            }
        return *m_sortingRules;
        }
    static void AppendRule(bvector<ClassSortingRule>& rules, SortingRuleCR rule, SelectQueryInfo const& selectInfo, std::function<bool(ECClassCR)> const& cond)
        {
        if (cond(*selectInfo.GetSelectClass()))
            rules.push_back(ClassSortingRule(*selectInfo.GetSelectClass(), nullptr, rule));
        for (RelatedClass const& relatedClass : selectInfo.GetRelatedClasses())
            {
            if (cond(*relatedClass.GetTargetClass()))
                rules.push_back(ClassSortingRule(*relatedClass.GetTargetClass(), relatedClass.GetTargetClassAlias(), rule));
            }
        }
    bvector<ClassSortingRule> GetSortingRules(SelectQueryInfo const& selectInfo) const
        {
        bvector<SortingRuleCP> const& allRules = GetAllSortingRules();
        bvector<ClassSortingRule> rules;
        for (SortingRuleCP rule : allRules)
            {
            if (rule->GetSchemaName().empty())
                {
                rules.push_back(ClassSortingRule(*selectInfo.GetSelectClass(), nullptr, *rule));
                continue;
                }

            ECSchemaCP ruleSchema = GetSchemaHelper().GetSchema(rule->GetSchemaName().c_str());
            if (nullptr == ruleSchema)
                {
                BeAssert(false);
                continue;
                }

            if (rule->GetClassName().empty())
                {
                AppendRule(rules, *rule, selectInfo, [&ruleSchema](ECClassCR selectClass){return ruleSchema == &selectClass.GetSchema();});
                continue;
                }

            ECClassCP ruleClass = ruleSchema->GetClassCP(rule->GetClassName().c_str());
            if (nullptr == ruleClass || !ruleClass->IsEntityClass())
                {
                BeAssert(false);
                continue;
                }

            AppendRule(rules, *rule, selectInfo, [&ruleClass, &rule](ECClassCR selectClass)
                {
                return ruleClass == &selectClass
                    || rule->GetIsPolymorphic() && selectClass.Is(ruleClass);
                });
            }
        return rules;
        }
    static void ToMultiECInstanceNodeQuery(NavigationQueryPtr& query, SelectQueryInfo const& selectInfo)
        {
        if (query->GetResultParameters().GetResultType() == NavigationQueryResultType::MultiECInstanceNodes)
            return;

        // set to invalid so NavigationQuery doesn't attempt to set resultQuery result type to nested query result type
        query->GetResultParametersR().SetResultType(NavigationQueryResultType::Invalid);

        RefCountedPtr<MultiECInstanceNodesQueryContract> contract = MultiECInstanceNodesQueryContract::Create(selectInfo.GetSelectClass(), false,
            selectInfo.GetRelatedClasses(), selectInfo.GetLabelOverrideValueSpecs());
        ComplexNavigationQueryPtr resultQuery = ComplexNavigationQuery::Create();
        resultQuery->SelectContract(*contract);
        resultQuery->From(*query);
        query = resultQuery;
        }
protected:
    ECInstanceSortingQueryContext(ECSchemaHelper const& schemaHelper, IConnectionManagerCR connections, IConnectionCR connection, PresentationRuleSetCR ruleset,
        Utf8StringCR locale, IUserSettings const& userSettings, IUsedUserSettingsListener* usedSettingsListener, ECExpressionsCache& ecexpressionsCache,
        JsonNavNodeCP parentNode, JsonNavNodeCP parentInstanceNode, bool doNotSort, Utf8StringCR specicificationHash)
        : ECInstanceQueryContext(schemaHelper, connections, connection, ruleset, locale, parentNode, parentInstanceNode), m_userSettings(userSettings), m_usedSettingsListener(usedSettingsListener),
        m_ecexpressionsCache(ecexpressionsCache), m_sortingRules(nullptr), m_doNotSort(doNotSort), m_specificationHash(specicificationHash)
        {}
    ~ECInstanceSortingQueryContext() {DELETE_AND_CLEAR(m_sortingRules);}

    bool _Accept(NavigationQueryPtr& query, SelectQueryInfo const& selectInfo) override
        {
        if (NavigationQueryResultType::MultiECInstanceNodes != query->GetResultParameters().GetResultType()
            && NavigationQueryResultType::ECInstanceNodes != query->GetResultParameters().GetResultType())
            {
            return false;
            }

        if (m_doNotSort || SelectionPurpose::Exclude == selectInfo.GetSelectFlag())
            {
            ToMultiECInstanceNodeQuery(query, selectInfo);
            return ECInstanceQueryContext::_Accept(query, selectInfo);
            }

        bvector<ClassSortingRule> sortingRules = GetSortingRules(selectInfo);
        if (sortingRules.empty())
            {
            ToMultiECInstanceNodeQuery(query, selectInfo);
            m_queriesLabelSorted.push_back(query);
            return true;
            }

        // if the rule of highest priority tells to not sort, we don't care about other rules
        if (sortingRules[0].GetRule()->GetDoNotSort())
            {
            ToMultiECInstanceNodeQuery(query, selectInfo);
            return ECInstanceQueryContext::_Accept(query, selectInfo);
            }

        Utf8String orderByClause;
        for (ClassSortingRule const& rule : sortingRules)
            {
            if (rule.GetRule()->GetDoNotSort())
                {
                LoggingHelper::LogMessage(Log::Navigation, Utf8PrintfString("SortingRule requests no sorting for ECClass '%s'",
                    rule.GetClass()->GetFullName()).c_str(), NativeLogging::LOG_INFO);
                continue;
                }

            ECPropertyCP ecProperty = rule.GetClass()->GetPropertyP(rule.GetRule()->GetPropertyName().c_str());
            if (nullptr == ecProperty)
                {
                BeAssert(false && "An ECProperty with the specified name does not exist in the specified ECClass");
                LoggingHelper::LogMessage(Log::Navigation, Utf8PrintfString("An ECProperty with name '%s' does not exist in the ECClass '%s'",
                    rule.GetRule()->GetPropertyName().c_str(), rule.GetClass()->GetFullName()).c_str(), NativeLogging::LOG_ERROR);
                continue;
                }

            if (!orderByClause.empty())
                orderByClause.append(", ");

            bool useSortingValueFunction = false;
            ECEnumerationCP enumeration = ecProperty->GetIsPrimitive() ? ecProperty->GetAsPrimitiveProperty()->GetEnumeration() : nullptr;
            PrimitiveECPropertyCP primitiveGroupingProperty = ecProperty->GetAsPrimitiveProperty();
            if (ecProperty->GetIsPrimitive() && (PRIMITIVETYPE_String == primitiveGroupingProperty->GetType() || nullptr != enumeration))
                {
                useSortingValueFunction = true;
                orderByClause.append(FUNCTION_NAME_GetSortingValue).append("(");
                if (nullptr != enumeration)
                    {
                    orderByClause.append(FUNCTION_NAME_GetECEnumerationValue).append("('").append(enumeration->GetSchema().GetName()).append("',");
                    orderByClause.append("'").append(enumeration->GetName()).append("',");
                    }
                }

            Utf8CP prefix = nullptr;
            if (!rule.GetClassAlias().empty())
                prefix = rule.GetClassAlias().c_str();
            else if (nullptr != query->AsComplexQuery())
                prefix = query->AsComplexQuery()->GetSelectPrefix();
            if (prefix && 0 != *prefix)
                orderByClause.append("[").append(prefix).append("].");

            orderByClause.append("[").append(rule.GetRule()->GetPropertyName()).append("]");

            if (useSortingValueFunction)
                {
                if (nullptr != enumeration)
                    orderByClause.append(")");
                orderByClause.append(")");
                }

            if (!rule.GetRule()->GetSortAscending())
                orderByClause.append(" DESC");
            }
        if (orderByClause.empty())
            {
            ToMultiECInstanceNodeQuery(query, selectInfo);
            return ECInstanceQueryContext::_Accept(query, selectInfo);
            }

        QueryBuilderHelpers::Order(*query, orderByClause.c_str());
        ToMultiECInstanceNodeQuery(query, selectInfo);
        m_queriesRulesSorted.push_back(query);
        return true;
        }
    void _DiscardQueries() override
        {
        m_queriesRulesSorted.clear();
        m_queriesLabelSorted.clear();
        ECInstanceQueryContext::_DiscardQueries();
        }
    NavigationQueryPtr _GetQuery() const override
        {
        size_t notSortedQueriesCount = GetIncludedQueries().size();
        size_t excludedQueriesCount = GetExcludedQueries().size();

        // union all rules-sorted queries
        size_t rulesSortedQueriesCount = 0;
        NavigationQueryPtr rulesSortedQuery;
        for (NavigationQueryPtr const& customSortedNestedQuery : m_queriesRulesSorted)
            {
            QueryBuilderHelpers::SetOrUnion(rulesSortedQuery, *customSortedNestedQuery);
            rulesSortedQueriesCount++;
            }

        // union all display label-sorted queries
        NavigationQueryPtr labelSortedUnion;
        for (NavigationQueryPtr query : m_queriesLabelSorted)
            QueryBuilderHelpers::SetOrUnion(labelSortedUnion, *query);
        if (labelSortedUnion.IsValid())
            {
            // create a single label-sorted query
            static Utf8PrintfString sortedDisplayLabel("%s([%s])", FUNCTION_NAME_GetSortingValue, ECInstanceNodesQueryContract::DisplayLabelFieldName);

            bvector<Utf8CP> aliases;
            aliases.push_back(ECInstanceNodesQueryContract::DisplayLabelFieldName);
            labelSortedUnion = QueryBuilderHelpers::CreateNestedQueryIfNecessary(*labelSortedUnion, aliases);
            QueryBuilderHelpers::Order(*labelSortedUnion, sortedDisplayLabel.c_str());

            if (0 != rulesSortedQueriesCount || 0 != notSortedQueriesCount || 0 != excludedQueriesCount)
                {
                // add the single label-sorted query to the wrapper one
                // note: the queries must be nested in order to union them ordered
                labelSortedUnion = QueryBuilderHelpers::CreateNestedQuery(*labelSortedUnion);
                }
            }

        // union all not sorted queries
        NavigationQueryPtr notSortedQuery;
        for (NavigationQueryPtr const& query : GetIncludedQueries())
            QueryBuilderHelpers::SetOrUnion(notSortedQuery, *query);

        // union all queries
        NavigationQueryPtr unionQuery = rulesSortedQuery;
        if (labelSortedUnion.IsValid())
            QueryBuilderHelpers::SetOrUnion(unionQuery, *labelSortedUnion);
        if (notSortedQuery.IsValid())
            QueryBuilderHelpers::SetOrUnion(unionQuery, *notSortedQuery);
        if (!unionQuery.IsValid())
            return nullptr;

        // handle excluded queries
        NavigationQueryPtr excludedQueries;
        for (NavigationQueryPtr const& excluded : GetExcludedQueries())
            excludedQueries = excludedQueries.IsNull() ? excluded.get() : ExceptNavigationQuery::Create(*excludedQueries, *excluded).get();
        if (excludedQueries.IsValid())
            unionQuery = ExceptNavigationQuery::Create(*unionQuery, *excludedQueries);

        return unionQuery;
        }

public:
    static IQueryContextPtr Create(ECSchemaHelper const& schemaHelper, IConnectionManagerCR connections, IConnectionCR connection, PresentationRuleSetCR ruleset,
        Utf8StringCR locale, IUserSettings const& userSettings, IUsedUserSettingsListener* usedSettingsListener, ECExpressionsCache& ecexpressionsCache,
        JsonNavNodeCP parentNode, JsonNavNodeCP parentInstanceNode, bool doNotSort, Utf8StringCR specicificationHash)
        {
        return new ECInstanceSortingQueryContext(schemaHelper, connections, connection, ruleset, locale, userSettings, usedSettingsListener,
            ecexpressionsCache, parentNode, parentInstanceNode, doNotSort, specicificationHash);
        }
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                07/2015
+===============+===============+===============+===============+===============+======*/
struct PostProcessQueryContext : IQueryContext
{
private:
    ChildNodeSpecificationCR m_specification;

private:
    NavigationQueryPtr PostProcess(NavigationQuery* query) const
        {
        if (nullptr == query)
            return nullptr;

        // handle HideNodesInHierarchy
        if (m_specification.GetHideNodesInHierarchy())
            {
            query->GetResultParametersR().GetNavNodeExtendedDataR().SetHideNodesInHierarchy(true);
            }
        else
            {
            // handle HideIfNoChildren
            if (m_specification.GetHideIfNoChildren())
                query->GetResultParametersR().GetNavNodeExtendedDataR().SetHideIfNoChildren(true);

            // handle HideExpression
            if (!m_specification.GetHideExpression().empty())
                query->GetResultParametersR().GetNavNodeExtendedDataR().SetHideExpression(m_specification.GetHideExpression());
            }

        // handle HasChildren hint
        if (ChildrenHint::Unknown != m_specification.GetHasChildren())
            query->GetResultParametersR().GetNavNodeExtendedDataR().SetChildrenHint(m_specification.GetHasChildren());

        // preserve ruleset ID in resulting nodes for later use
        query->GetResultParametersR().GetNavNodeExtendedDataR().SetRulesetId(GetRuleset().GetRuleSetId().c_str());

        // preserve specification ID in resulting nodes for later use
        query->GetResultParametersR().GetNavNodeExtendedDataR().SetSpecificationHash(m_specification.GetHash());

        return query;
        }

protected:
    PostProcessQueryContext(IQueryContext& base, ChildNodeSpecificationCR specification) : IQueryContext(base), m_specification(specification) {}
    NavigationQueryContractPtr _GetContract(SelectQueryInfo const& selectInfo) const override {return GetBaseContext()->GetContract(selectInfo);}
    bool _Accept(NavigationQueryPtr& query, SelectQueryInfo const& selectInfo) override {return GetBaseContext()->Accept(query, selectInfo);}
    void _DiscardQueries() override {GetBaseContext()->DiscardQueries();}
    NavigationQueryPtr _GetQuery() const override {return PostProcess(GetBaseContext()->GetQuery().get());}

public:
    static IQueryContextPtr Create(IQueryContext& base, ChildNodeSpecificationCR specification) {return new PostProcessQueryContext(base, specification);}
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                07/2015
+===============+===============+===============+===============+===============+======*/
struct GroupingHandlerQueryContext : BaseQueryContext
{
private:
    GroupingHandler& m_handler;

protected:
    GroupingHandlerQueryContext(IQueryContext& base, GroupingHandler& handler) : BaseQueryContext(base), m_handler(handler) {}
    NavigationQueryContractPtr _GetContract(SelectQueryInfo const& selectInfo) const override {return m_handler.GetContract(selectInfo);}
    bool _Accept(NavigationQueryPtr& query, SelectQueryInfo const& selectInfo) override
        {
        ComplexNavigationQueryPtr queryToProcess = query->AsComplexQuery();
        if (queryToProcess.IsNull())
            queryToProcess = QueryBuilderHelpers::CreateNestedQuery(*query);

        if (ApplyGroupingResult::Stored != m_handler.ApplyClassGrouping(queryToProcess, selectInfo))
            {
            query = queryToProcess;
            if (GetBaseContext()->Accept(query, selectInfo))
                return true;

            return BaseQueryContext::_Accept(query, selectInfo);
            }

        return true;
        }
    NavigationQueryPtr _GetQuery() const override
        {
        NavigationQueryPtr query = m_handler.GetStoredQuery();
        if (query.IsNull())
            query = BaseQueryContext::_GetQuery();
        if (query.IsNull())
            {
            query = GetBaseContext()->GetQuery();
            GetBaseContext()->DiscardQueries();
            }
        if (query.IsNull())
            return nullptr;

        m_handler.ApplyOuterGrouping(query);
        return query;
        }

public:
    static RefCountedPtr<GroupingHandlerQueryContext> Create(IQueryContext& base, GroupingHandler& handler) {return new GroupingHandlerQueryContext(base, handler);}
};

typedef RefCountedPtr<struct MultiQueryContext> MultiQueryContextPtr;
/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                07/2015
+===============+===============+===============+===============+===============+======*/
struct MultiQueryContext : RefCountedBase
{
private:
    IQueryContextPtr m_base;
    GroupingResolver const& m_resolver;
    bool m_isSearchContext;
    mutable bvector<IQueryContextPtr> m_groupingContexts;
    mutable bmap<GroupingHandler const*, IQueryContextPtr> m_groupingContextsByHandler;
    bmap<ECRelationshipClassCP, int> m_relationshipsUseCounter;

private:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                07/2015
    +---------------+---------------+---------------+---------------+---------------+------*/
    GroupingHandler* GetGroupingHandler(SelectQueryInfo const& selectInfo) const
        {
        bvector<ECClassCP> selectClasses = selectInfo.CreateSelectClassList();
        GroupingHandlersList handlers = m_resolver.GetHandlersForNextGroupingLevel(selectClasses, false);
        GroupingHandler* handler = nullptr;
        if (!handlers.empty())
            {
            // the handlers list contains all handlers that can handle the given select
            // class - we assume that they're  in the order we want them applied, so just
            // take the first one
            handler = handlers[0];
            }
        return handler;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                07/2015
    +---------------+---------------+---------------+---------------+---------------+------*/
    IQueryContextPtr CreateGroupingContext(GroupingHandler& handler) const
        {
        IQueryContextPtr context = GroupingHandlerQueryContext::Create(*m_base, handler);
        context = PostProcessQueryContext::Create(*context, m_resolver.GetSpecification());
        return context;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                07/2015
    +---------------+---------------+---------------+---------------+---------------+------*/
    IQueryContextPtr GetGroupingContext(GroupingHandler& handler) const
        {
        auto contextIter = m_groupingContextsByHandler.find(&handler);
        if (m_groupingContextsByHandler.end() == contextIter)
            {
            IQueryContextPtr context = CreateGroupingContext(handler);
            m_groupingContexts.push_back(context);
            contextIter = m_groupingContextsByHandler.Insert(&handler, context).first;
            }
        return contextIter->second;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                07/2015
    +---------------+---------------+---------------+---------------+---------------+------*/
    void ApplyFilters(NavigationQueryPtr& query, SelectQueryInfo const& selectInfo) const
        {
        ComplexNavigationQueryPtr queryToFilter = query->AsComplexQuery();
        if (queryToFilter.IsNull())
            queryToFilter = QueryBuilderHelpers::CreateNestedQuery(*query);

        bool didApply = false;
        bvector<GroupingNodeAndHandler> filters = m_resolver.GetFilterHandlers();
        for (GroupingNodeAndHandler const& filter : filters)
            didApply |= filter.GetHandler().ApplyFilter(queryToFilter, selectInfo, filter.GetNode());

        if (didApply)
            query = queryToFilter;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                11/2016
    +---------------+---------------+---------------+---------------+---------------+------*/
    void JoinRelatedClasses(ComplexNavigationQuery& query, bvector<RelatedClass> const& relatedClasses) const
        {
        for (RelatedClass const& relatedClass : relatedClasses)
            query.Join(relatedClass);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                07/2015
    +---------------+---------------+---------------+---------------+---------------+------*/
    bool IsAnyECClassAccepted(bvector<ECClassCP> classes) const
        {
        bvector<GroupingNodeAndHandler> filters = m_resolver.GetFilterHandlers();
        for (GroupingNodeAndHandler const& filter : filters)
            {
            bool accepted = false;
            for (ECClassCP ecClass : classes)
                {
                if (filter.GetHandler().AppliesForClass(*ecClass, false))
                    {
                    accepted = true;
                    break;
                    }
                }
            if (!accepted)
                return false;
            }
        return true;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                07/2015
    +---------------+---------------+---------------+---------------+---------------+------*/
    void AddToListOrUnion(bvector<NavigationQueryPtr>& queries, NavigationQuery& query) const
        {
        bool unioned = false;
        for (NavigationQueryPtr& queryInList : queries)
            {
            if (queryInList->GetResultParameters().GetResultType() == query.GetResultParameters().GetResultType())
                {
                Utf8String lhsOrderByClause = QueryBuilderHelpers::GetOrderByClause(*queryInList);
                NavigationQueryPtr rhsQuery = &query;
                Utf8String rhsOrderByClause = QueryBuilderHelpers::GetOrderByClause(*rhsQuery);
                if (lhsOrderByClause.Equals(rhsOrderByClause))
                    {
                    // if the two queries have similar order by clauses, just remove them and move the clause to union
                    QueryBuilderHelpers::Order(*queryInList, "");
                    QueryBuilderHelpers::Order(*rhsQuery, "");
                    }
                else
                    {
                    // if the clauses are different, need to nest the queries..
                    if (!lhsOrderByClause.empty())
                        queryInList = QueryBuilderHelpers::CreateNestedQuery(*queryInList);
                    if (!rhsOrderByClause.empty())
                        rhsQuery = QueryBuilderHelpers::CreateNestedQuery(*rhsQuery);
                    }
                queryInList = &UnionNavigationQuery::Create(*queryInList, *rhsQuery)->OrderBy(lhsOrderByClause.c_str());
                unioned = true;
                break;
                }
            }
        if (!unioned)
            queries.push_back(&query);
        }

protected:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                07/2015
    +---------------+---------------+---------------+---------------+---------------+------*/
    MultiQueryContext(IQueryContext& base, GroupingResolver const& resolver, bool isSearchContext)
        : m_base(&base), m_resolver(resolver), m_isSearchContext(isSearchContext)
        {}

public:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                07/2015
    +---------------+---------------+---------------+---------------+---------------+------*/
    static MultiQueryContextPtr Create(IQueryContext& base, GroupingResolver const& resolver, bool isSearchContext)
        {
        return new MultiQueryContext(base, resolver, isSearchContext);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                07/2015
    +---------------+---------------+---------------+---------------+---------------+------*/
    bool Accept(ComplexNavigationQueryPtr const& query, SelectQueryInfo const& selectInfo)
        {
        if (!IsAnyECClassAccepted(selectInfo.CreateSelectClassList()))
            return false;

        // add ids of relationship classes used by the query
        if (!selectInfo.GetRelatedClassPath().empty())
            {
            // add ids of all relationships in the related class path
            for (RelatedClassCR related : selectInfo.GetRelatedClassPath())
                query->GetResultParametersR().GetMatchingRelationshipIds().insert(related.GetRelationship()->GetId());
            }
        for (RelatedClassCR relatedClass : selectInfo.GetRelatedClasses())
            query->GetResultParametersR().GetMatchingRelationshipIds().insert(relatedClass.GetRelationship()->GetId());

        // set query specification
        BeAssert(nullptr != selectInfo.GetSpecification());
        query->GetResultParametersR().SetSpecification(selectInfo.GetSpecification());

        JoinRelatedClasses(*query, selectInfo.GetRelatedClasses());

        NavigationQueryPtr filteredQuery = query;
        ApplyFilters(filteredQuery, selectInfo);

        GroupingHandler* handler = GetGroupingHandler(selectInfo);
        if (nullptr == handler || !GetGroupingContext(*handler)->Accept(filteredQuery, selectInfo))
            {
            if (!m_base->Accept(filteredQuery, selectInfo))
                {
                BeAssert(false);
                return false;
                }
            }

        return true;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                07/2015
    +---------------+---------------+---------------+---------------+---------------+------*/
    bvector<NavigationQueryPtr> GetQueries() const
        {
        bvector<NavigationQueryPtr> queries;
        for (IQueryContextPtr const& groupingContext : m_groupingContexts)
            {
            NavigationQueryPtr query = groupingContext->GetQuery();
            if (query.IsNull())
                continue;

            AddToListOrUnion(queries, *query);
            }

        NavigationQueryPtr baseQuery = m_base->GetQuery();
        if (baseQuery.IsNull())
            return queries;

        AddToListOrUnion(queries, *baseQuery);
        return queries;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                07/2015
    +---------------+---------------+---------------+---------------+---------------+------*/
    NavigationQueryContractPtr GetContract(SelectQueryInfo const& selectInfo) const
        {
        NavigationQueryContractPtr contract;
        GroupingHandler* handler = GetGroupingHandler(selectInfo);
        if (nullptr != handler)
            contract = handler->GetContract(selectInfo);
        else
            contract = m_base->GetContract(selectInfo);

        if (contract.IsNull())
            return nullptr;

        if (m_isSearchContext)
            {
            contract->SetECClassIdFieldName(SEARCH_QUERY_FIELD_ECClassId);
            contract->SetECInstanceIdFieldName(SEARCH_QUERY_FIELD_ECInstanceId);
            }

        return contract;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                01/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    bmap<ECRelationshipClassCP, int>& GetRelationshipUseCounter() {return m_relationshipsUseCounter;}

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                01/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    int GetRelationshipUseCount(ECRelationshipClassCR relationship, bool increase = true)
        {
        if (increase)
            return m_relationshipsUseCounter[&relationship]++;
        return m_relationshipsUseCounter[&relationship];
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
static MultiQueryContextPtr CreateQueryContext(GroupingResolver const& resolver, bool isSearchContext = false)
    {
    IQueryContextPtr context = ECInstanceSortingQueryContext::Create(resolver.GetSchemaHelper(), resolver.GetQueryBuilderParams().GetConnections(),
        resolver.GetQueryBuilderParams().GetConnection(), resolver.GetQueryBuilderParams().GetRuleset(), resolver.GetQueryBuilderParams().GetLocale(),
        resolver.GetQueryBuilderParams().GetUserSettings(), resolver.GetQueryBuilderParams().GetUsedSettingsListener(),
        resolver.GetQueryBuilderParams().GetECExpressionsCache(),
        resolver.GetParentNode(), resolver.GetParentInstanceNode(), resolver.GetSpecification().GetDoNotSort(), resolver.GetSpecificationHash());
    context = PostProcessQueryContext::Create(*context, resolver.GetSpecification());
    return MultiQueryContext::Create(*context, resolver, isSearchContext);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename SpecificationType>
Utf8String NavigationQueryBuilder::GetSupportedSchemas(SpecificationType const& specification) const
    {
    if (!specification.GetSupportedSchemas().empty())
        return specification.GetSupportedSchemas();

    return m_params.GetRuleset().GetSupportedSchemas();
    }

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                11/2016
+===============+===============+===============+===============+===============+======*/
struct RuleInfo
{
private:
    ECClassCP m_ruleClass;
    bool m_isRulePolymorphic;
public:
    RuleInfo() : m_ruleClass(nullptr) {}
    RuleInfo(ECClassCR ruleClass, bool isPolymorphic) : m_ruleClass(&ruleClass), m_isRulePolymorphic(isPolymorphic) {}
    ECClassCP GetRuleClass() const {return m_ruleClass;}
    bool IsRulePolymorphic() const {return m_isRulePolymorphic;}
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                11/2016
+===============+===============+===============+===============+===============+======*/
struct SelectQueryInfoComparer
    {
    bool operator()(SelectQueryInfo const& lhs, SelectQueryInfo const& rhs) const
        {
        int selectClassNameCmp = strcmp(lhs.GetSelectClass()->GetFullName(), rhs.GetSelectClass()->GetFullName());
        if (selectClassNameCmp < 0)
            return true;
        if (selectClassNameCmp > 0)
            return false;
        COMPARE_VEC(lhs.GetRelatedClassPath(), rhs.GetRelatedClassPath());
        COMPARE_VEC(lhs.GetRelatedClasses(), rhs.GetRelatedClasses());
        if (!lhs.GetSelectPolymorphically() && rhs.GetSelectPolymorphically())
            return true;
        if (lhs.GetSelectPolymorphically() && !rhs.GetSelectPolymorphically())
            return false;
        return false;
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static bool ShouldSplitIntoDerivedClasses(RuleInfo const& ruleInfo, ECClassCR candidate)
    {
    // expand ECClass into its subclasses in 2 cases:
    // - rule applies to one of the child classes
    // - rule applies to this class, but is not polymorphic (applies to this class, but not its children)
    return ruleInfo.GetRuleClass()->Is(&candidate) && ruleInfo.GetRuleClass() != &candidate
        || ruleInfo.GetRuleClass() == &candidate && !ruleInfo.IsRulePolymorphic() && !candidate.GetDerivedClasses().empty();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
static void ProcessQueryClassesBasedOnCustomizationRules(bvector<SelectQueryInfo>& selectInfos, bvector<RuleInfo> const& customizationRuleInfos)
    {
    bset<SelectQueryInfo*> selectInfosWithBasePrimaryClasses;
    bset<RelatedClass*> baseRelatedClasses;
    bset<SelectQueryInfo, SelectQueryInfoComparer> infosToAppend;
    for (SelectQueryInfo& selectInfo : selectInfos)
        {
        for (RuleInfo const& ruleInfo : customizationRuleInfos)
            {
            // check the primary select class
            if (selectInfo.GetSelectPolymorphically() && ShouldSplitIntoDerivedClasses(ruleInfo, *selectInfo.GetSelectClass()))
                {
                // the rule wants to customize a subclass of the class and leave other subclasses unsorted -
                // this means we have to expand the ecClass into its subclasses
                bvector<SelectQueryInfo> subclassSelectInfos;
                for (ECClassCP derived : selectInfo.GetSelectClass()->GetDerivedClasses())
                    {
                    SelectQueryInfo copy(selectInfo);
                    copy.SetSelectClass(*derived->GetEntityClassCP());
                    copy.SetSelectPolymorphically(true);
                    if (!copy.GetRelatedClassPath().empty())
                        copy.GetRelatedClassPath().front().SetSourceClass(*derived->GetEntityClassCP());
                    subclassSelectInfos.push_back(std::move(copy));
                    }

                ProcessQueryClassesBasedOnCustomizationRules(subclassSelectInfos, customizationRuleInfos);
                selectInfosWithBasePrimaryClasses.insert(&selectInfo);
                infosToAppend.insert(subclassSelectInfos.begin(), subclassSelectInfos.end());
                }

            // check all related classes
            for (size_t relatedClassIndex = 0; relatedClassIndex < selectInfo.GetRelatedClasses().size(); ++relatedClassIndex)
                {
                RelatedClass& relatedClass = selectInfo.GetRelatedClasses()[relatedClassIndex];
                if (relatedClass.IsPolymorphic() && ShouldSplitIntoDerivedClasses(ruleInfo, *relatedClass.GetTargetClass()))
                    {
                    bvector<SelectQueryInfo> subclassSelectInfos;
                    for (ECClassCP derived : relatedClass.GetTargetClass()->GetDerivedClasses())
                        {
                        SelectQueryInfo copy(selectInfo);
                        copy.GetRelatedClasses()[relatedClassIndex].SetTargetClass(*derived->GetEntityClassCP());
                        subclassSelectInfos.push_back(std::move(copy));
                        }

                    ProcessQueryClassesBasedOnCustomizationRules(subclassSelectInfos, customizationRuleInfos);
                    baseRelatedClasses.insert(&relatedClass);
                    infosToAppend.insert(subclassSelectInfos.begin(), subclassSelectInfos.end());
                    }
                }
            }
        }

    // the primary and related classes that were polymorphic and have been split into subclasses
    // should be changed to be selected non-polymorphically
    for (SelectQueryInfo* selectInfo : selectInfosWithBasePrimaryClasses)
        selectInfo->SetSelectPolymorphically(false);
    for (RelatedClass* relatedClass : baseRelatedClasses)
        relatedClass->SetIsPolymorphic(false);

    // don't want to append selects that are already in the input vector
    for (SelectQueryInfo const& selectInfo : selectInfos)
        {
        auto iter = infosToAppend.find(selectInfo);
        if (infosToAppend.end() != iter)
            infosToAppend.erase(iter);
        }

    // append the additional selects
    selectInfos.insert(selectInfos.end(), infosToAppend.begin(), infosToAppend.end());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
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
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static void ProcessQueryClassesBasedOnCustomizationRules(bvector<SelectQueryInfo>& selectInfos, GroupingResolver const& resolver,
    bvector<ECClassCP> const& instanceLabelOverrideClasses, JsonNavNodeCP parentNode, NavigationQueryBuilderParameters const& params)
    {
    bvector<RuleInfo> customizationRuleInfos;

    for (SelectQueryInfo const& info : selectInfos)
        {
        bvector<GroupingRuleCP> groupingRules = resolver.GetAppliedGroupingRules(info.CreateSelectClassList());
        for (GroupingRuleCP groupingRule : groupingRules)
            {
            ECClassCP ruleClass = params.GetSchemaHelper().GetECClass(groupingRule->GetSchemaName().c_str(), groupingRule->GetClassName().c_str());
            if (ruleClass->IsEntityClass())
                customizationRuleInfos.push_back(RuleInfo(*ruleClass->GetEntityClassCP(), true));
            }
        }

    RulesPreprocessor preprocessor(params.GetConnections(), params.GetConnection(), params.GetRuleset(), params.GetLocale(),
        params.GetUserSettings(), params.GetUsedSettingsListener(), params.GetECExpressionsCache());
    RulesPreprocessor::AggregateCustomizationRuleParameters preprocessorParams(parentNode, resolver.GetSpecificationHash());
    CallbackOnRuleClasses<SortingRule>(preprocessor.GetSortingRules(preprocessorParams), params.GetSchemaHelper(),
        [&customizationRuleInfos](SortingRuleCR rule, ECEntityClassCR ecClass)
        {
        customizationRuleInfos.push_back(RuleInfo(ecClass, rule.GetIsPolymorphic()));
        });

    for (ECClassCP instanceLabelOverrideClass : instanceLabelOverrideClasses)
        customizationRuleInfos.push_back(RuleInfo(*instanceLabelOverrideClass, true));

    ProcessQueryClassesBasedOnCustomizationRules(selectInfos, customizationRuleInfos);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<NavigationQueryPtr> NavigationQueryBuilder::GetQueries(AllInstanceNodesSpecification const& specification, RootNodeRuleCR rule) const
    {
    return GetQueries(nullptr, specification, rule);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<NavigationQueryPtr> NavigationQueryBuilder::GetQueries(AllRelatedInstanceNodesSpecification const& specification, RootNodeRuleCR rule) const
    {
    RelatedInstanceNodesSpecification relatedInstanceNodesSpecification(specification.GetPriority(), specification.GetHasChildren(),
        specification.GetHideNodesInHierarchy(), specification.GetHideIfNoChildren(), specification.GetGroupByClass(),
        specification.GetGroupByLabel(), specification.GetSkipRelatedLevel(), "", specification.GetRequiredRelationDirection(), GetSupportedSchemas(specification), "", "");
    if (specification.GetGroupByRelationship())
        relatedInstanceNodesSpecification.SetGroupByRelationship(true);

    bvector<NavigationQueryPtr> queries = GetQueries(nullptr, relatedInstanceNodesSpecification, specification.GetHash(), rule);
    for (NavigationQueryPtr query : queries)
        {
        query->GetResultParametersR().SetSpecification(&specification);
        query->GetResultParametersR().GetNavNodeExtendedDataR().SetSpecificationHash(specification.GetHash());
        }
    return queries;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<NavigationQueryPtr> NavigationQueryBuilder::GetQueries(RelatedInstanceNodesSpecification const& specification, RootNodeRuleCR rule) const
    {
    return GetQueries(nullptr, specification, specification.GetHash(), rule);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<NavigationQueryPtr> NavigationQueryBuilder::GetQueries(InstanceNodesOfSpecificClassesSpecification const& specification, RootNodeRuleCR rule) const
    {
    return GetQueries(nullptr, specification, rule);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<NavigationQueryPtr> NavigationQueryBuilder::GetQueries(SearchResultInstanceNodesSpecification const& specification, RootNodeRuleCR rule) const
    {
    return GetQueries(nullptr, specification, rule);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<NavigationQueryPtr> NavigationQueryBuilder::GetQueries(JsonNavNodeCR parentNode, AllInstanceNodesSpecification const& specification, ChildNodeRuleCR rule) const
    {
    return GetQueries(&parentNode, specification, rule);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<NavigationQueryPtr> NavigationQueryBuilder::GetQueries(JsonNavNodeCR parentNode, AllRelatedInstanceNodesSpecification const& specification, ChildNodeRuleCR rule) const
    {
    RelatedInstanceNodesSpecification relatedInstanceNodesSpecification(specification.GetPriority(), specification.GetHasChildren(),
        specification.GetHideNodesInHierarchy(), specification.GetHideIfNoChildren(), specification.GetGroupByClass(),
        specification.GetGroupByLabel(), specification.GetSkipRelatedLevel(), "", specification.GetRequiredRelationDirection(), GetSupportedSchemas(specification), "", "");
    if (specification.GetGroupByRelationship())
        relatedInstanceNodesSpecification.SetGroupByRelationship(true);

    bvector<NavigationQueryPtr> queries = GetQueries(&parentNode, relatedInstanceNodesSpecification, specification.GetHash(), rule);
    for (NavigationQueryPtr query : queries)
        {
        query->GetResultParametersR().SetSpecification(&specification);
        query->GetResultParametersR().GetNavNodeExtendedDataR().SetSpecificationHash(specification.GetHash());
        }
    return queries;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<NavigationQueryPtr> NavigationQueryBuilder::GetQueries(JsonNavNodeCR parentNode, RelatedInstanceNodesSpecification const& specification, ChildNodeRuleCR rule) const
    {
    if (specification.GetRelatedClassNames().empty() && specification.GetRelationshipClassNames().empty())
        {
        LoggingHelper::LogMessage(Log::Navigation, "Either RelatedClassNames or RelationshipClassNames must be specified "
            "for RelatedInstanceNodes specification", NativeLogging::LOG_ERROR);
        return bvector<NavigationQueryPtr>();
        }

    return GetQueries(&parentNode, specification, specification.GetHash(), rule);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<NavigationQueryPtr> NavigationQueryBuilder::GetQueries(JsonNavNodeCR parentNode, InstanceNodesOfSpecificClassesSpecification const& specification, ChildNodeRuleCR rule) const
    {
    return GetQueries(&parentNode, specification, rule);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<NavigationQueryPtr> NavigationQueryBuilder::GetQueries(JsonNavNodeCR parentNode, SearchResultInstanceNodesSpecification const& specification, ChildNodeRuleCR rule) const
    {
    return GetQueries(&parentNode, specification, rule);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
static bset<unsigned> GetUsedParentInstanceLevels(Utf8String instanceFilter)
    {
    bset<unsigned> levels;
    size_t startPos = 0, endPos = 0;
    while (Utf8String::npos != (endPos = instanceFilter.find("parent.", endPos)))
        {
        endPos += 7; // strlen("parent.") = 7
        startPos = instanceFilter.find("parent", startPos);
        Utf8String selector(instanceFilter.begin() + startPos, instanceFilter.begin() + endPos);
        startPos = endPos;
        unsigned count = 1;
        for (Utf8Char const& c : selector)
            {
            if (c == '_')
                count++;
            }
        levels.insert(count);
        }
    return levels;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static NavNodeCPtr GetParentNodeByLevel(IHierarchyCacheCR nodesCache, NavNodeCR currNode, unsigned currNodeLevel, unsigned targetNodeLevel)
    {
    NavNodeCPtr curr = &currNode;
    for (unsigned i = currNodeLevel; i < targetNodeLevel; ++i)
        {
        if (curr.IsNull() || !NavNodeExtendedData(*curr).HasVirtualParentId())
            {
            BeAssert(false);
            return nullptr;
            }
        uint64_t parentId = NavNodeExtendedData(*curr).GetVirtualParentId();
        curr = nodesCache.GetNode(parentId);
        }
    return curr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2019
+---------------+---------------+---------------+---------------+---------------+------*/
static bset<ECClassCP> GetInstanceKeyClasses(bvector<ECClassInstanceKey> const& keys)
    {
    bset<ECClassCP> classes;
    for (ECClassInstanceKey const& key : keys)
        classes.insert(key.GetClass());
    return classes;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2019
+---------------+---------------+---------------+---------------+---------------+------*/
static bvector<ECInstanceId> GetInstanceIdsWithClass(bvector<ECClassInstanceKey> const& keys, ECClassCR ecClass)
    {
    bvector<ECInstanceId> ids;
    for (ECClassInstanceKeyCR key : keys)
        {
        if (key.GetClass() == &ecClass)
            ids.push_back(key.GetId());
        }
    return ids;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus AppendParents(ComplexNavigationQuery& query, bset<unsigned> const& usedParents,
    NavigationQueryBuilderParameters const& params, NavNodeCR parentNode)
    {
    NavNodeCPtr previousParent = &parentNode;
    unsigned previousLevel = 1;
    Utf8String parentAlias = "parent";
    for (unsigned targetLevel : usedParents)
        {
        if (previousParent.IsNull())
            {
            BeAssert(false);
            return ERROR;
            }

        NavNodeCPtr parent = GetParentNodeByLevel(params.GetNodesCache(), *previousParent, previousLevel, targetLevel);
        if (parent.IsNull() || nullptr == parent->GetKey()->AsECInstanceNodeKey())
            {
            BeAssert(false);
            return ERROR;
            }

        ECInstancesNodeKey const& parentNodeKey = *parent->GetKey()->AsECInstanceNodeKey();
        bset<ECClassCP> parentClasses = GetInstanceKeyClasses(parentNodeKey.GetInstanceKeys());
        ECClassCP parentClass = *parentClasses.begin();
        if (parentClasses.size() > 1)
            {
            LoggingHelper::LogMessage(Log::Navigation, Utf8PrintfString("Used parent node '%s' represents instances of more than 1 ECClass. Using just the first one: '%s'.",
                parent->GetLabel().c_str(), parentClass->GetFullName()).c_str(), NativeLogging::LOG_WARNING);
            }

        for (unsigned i = previousLevel; i < targetLevel; ++i)
            parentAlias.append("_parent");

        query.From(*parentClass, false, parentAlias.c_str(), true);

        IdsFilteringHelper<bvector<ECInstanceId>> helper(GetInstanceIdsWithClass(parentNodeKey.GetInstanceKeys(), *parentClass));
        query.Where(helper.CreateWhereClause(Utf8PrintfString("[%s].[ECInstanceId]", parentAlias.c_str()).c_str()).c_str(), helper.CreateBoundValues(), true);
        OnSelected(*parentClass, false, params);

        previousLevel = targetLevel;
        previousParent = parent;
        }
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
static void ApplyInstanceFilter(ComplexNavigationQuery& query, SelectQueryInfo const& selectInfo, NavigationQueryBuilderParameters const& params,
    Utf8StringCR instanceFilter, NavNodeCP parentNode)
    {
    if (instanceFilter.empty())
        return;

    bset<unsigned> usedParentInstanceLevels = GetUsedParentInstanceLevels(instanceFilter);
    if (usedParentInstanceLevels.empty() || parentNode && SUCCESS == AppendParents(query, usedParentInstanceLevels, params, *parentNode))
        query.Where(ECExpressionsHelper(params.GetECExpressionsCache()).ConvertToECSql(instanceFilter).c_str(), BoundQueryValuesList());

    if (nullptr != params.GetUsedClassesListener())
        {
        UsedClassesHelper::NotifyListenerWithUsedClasses(*params.GetUsedClassesListener(),
            params.GetSchemaHelper(), params.GetECExpressionsCache(), instanceFilter);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String FormatInstanceFilter(Utf8String filter)
    {
    filter.ReplaceAll(".parent", "_parent");
    return filter;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2019
+---------------+---------------+---------------+---------------+---------------+------*/
static void ApplyRelatedInstanceFilter(ComplexNavigationQueryR query, bvector<ECInstanceId> const& relatedInstanceIds)
    {
    IdsFilteringHelper<bvector<ECInstanceId>> helper(relatedInstanceIds);
    query.Where(helper.CreateWhereClause("[related].[ECInstanceId]").c_str(), helper.CreateBoundValues());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
static ComplexNavigationQueryPtr CreateQuery(NavigationQueryContract& contract, SelectQueryInfo const& selectInfo,
    NavigationQueryBuilderParameters const& params, NavNodeCR parentInstanceNode, ECClassCR parentClass,
    bvector<ECInstanceId> const& parentInstanceIds, Utf8StringCR instanceFilter, bool groupByContract)
    {
    ComplexNavigationQueryPtr query = ComplexNavigationQuery::Create();
    query->SelectContract(contract, "this");
    query->From(*selectInfo.GetSelectClass(), selectInfo.GetSelectPolymorphically(), "this");
    query->Join(selectInfo.GetRelatedClassPath(), false);
    ApplyRelatedInstanceFilter(*query, parentInstanceIds);
    ApplyInstanceFilter(*query, selectInfo, params, instanceFilter, &parentInstanceNode);

    if (groupByContract)
        {
        ComplexNavigationQueryPtr grouped = ComplexNavigationQuery::Create();
        grouped->SelectAll();
        grouped->From(*query);
        grouped->GroupByContract(contract);
        query = grouped;
        }

    // set the relationship path for the contract so it can include any skipped related instance keys into the select clause
    contract.SetRelationshipPath(selectInfo.GetRelatedClassPath());

    // note: isPreviousRelationshipForward considers "this -> related" relationship direction, but the property in extended data
    // should tell about "related -> this" direction
    query->GetResultParametersR().GetNavNodeExtendedDataR().SetRelationshipDirection(selectInfo.GetRelatedClassPath().front().IsForwardRelationship() ? ECRelatedInstanceDirection::Backward : ECRelatedInstanceDirection::Forward);

    return query;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename TKey, typename TValue>
static bvector<TKey> GetMapKeys(bmap<TKey, TValue> const& map)
    {
    bvector<TKey> keys;
    std::transform(map.begin(), map.end(), std::back_inserter(keys), [](bpair<TKey, TValue> const& entry) {return entry.first;});
    return keys;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
static SelectQueryInfo CreateSelectInfo(ChildNodeSpecificationCR specification, ECClassCR selectClass, bool selectPolymorphically,
    SelectionPurpose purpose = SelectionPurpose::Include)
    {
    return SelectQueryInfo(specification, selectClass, selectPolymorphically, purpose);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static SelectQueryInfo CreateSelectInfo(ECSchemaHelper const& helper, MultiQueryContext& ctx, ECEntityClassCR parentInstanceClass,
    RelatedClassPath const& path, SelectionPurpose purpose, ChildNodeSpecificationCR specification)
    {
    RelatedClassPath relatedClassPath = path;
    relatedClassPath.Reverse("related", path.back().IsPolymorphic());

    SelectQueryInfo selectInfo = CreateSelectInfo(specification, *path.back().GetTargetClass(), path.back().IsPolymorphic(), purpose);
    selectInfo.GetRelatedClassPath() = relatedClassPath;
    selectInfo.GetRelatedClasses() = QueryBuilderHelpers::GetRelatedInstanceClasses(helper, *selectInfo.GetSelectClass(), specification.GetRelatedInstances(), ctx.GetRelationshipUseCounter());
    return selectInfo;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2019
+---------------+---------------+---------------+---------------+---------------+------*/
static bmap<ECClassCP, bvector<ECInstanceId>> GroupClassInstanceKeys(bvector<ECClassInstanceKey> const& vec)
    {
    bmap<ECClassCP, bvector<ECInstanceId>> map;
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
* @bsimethod                                    Grigas.Petraitis                04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<NavigationQueryPtr> NavigationQueryBuilder::GetQueries(JsonNavNodeCP parentNode, AllInstanceNodesSpecification const& specification, ChildNodeRuleCR rule) const
    {
    GroupingResolver groupingResolver(m_params.GetSchemaHelper(), m_params, parentNode, specification.GetHash(), specification);
    MultiQueryContextPtr queryContext = CreateQueryContext(groupingResolver);

    // find the classes to query instances from
    bvector<SelectQueryInfo> selectInfos;
    if (groupingResolver.IsGroupedByClass())
        {
        selectInfos.push_back(CreateSelectInfo(specification, *groupingResolver.GetGroupingClass(), false));
        }
    else
        {
        Utf8String supportedSchemas = GetSupportedSchemas(specification);
        if (supportedSchemas.empty())
            {
            LoggingHelper::LogMessage(Log::Navigation, "Neither rule set nor AllInstanceNodesSpecification specification have specified supported schemas. "
                "Consider specifying a value for this property as it can affect performance significantly", NativeLogging::LOG_WARNING, true);
            }
        ECClassSet queryClasses = m_params.GetSchemaHelper().GetECClassesFromSchemaList(supportedSchemas);
        for (auto pair : queryClasses)
            selectInfos.push_back(CreateSelectInfo(specification, *pair.first, pair.second));
        }

    // quick return if nothing to select from
    if (selectInfos.empty())
        return bvector<NavigationQueryPtr>();

    // determine instance label overrides
    bmap<ECClassCP, bvector<InstanceLabelOverrideValueSpecification const*>> labelOverridingProperties = QueryBuilderHelpers::GetLabelOverrideValuesMap(m_params.GetSchemaHelper(),
        m_params.GetRuleset().GetInstanceLabelOverrides());

    // determine additional related joins
    for (SelectQueryInfo& info : selectInfos)
        {
        info.GetRelatedClasses() = QueryBuilderHelpers::GetRelatedInstanceClasses(m_params.GetSchemaHelper(), *info.GetSelectClass(),
            specification.GetRelatedInstances(), queryContext->GetRelationshipUseCounter());
        }

    // may need to split some base classes into their derived classes if there are customization rules that only apply for
    // derived classes.
    ProcessQueryClassesBasedOnCustomizationRules(selectInfos, groupingResolver, GetMapKeys(labelOverridingProperties),
        groupingResolver.GetParentInstanceNode(), m_params);

    // create a query for each class
    for (SelectQueryInfo& info : selectInfos)
        {
        info.SetLabelOverrideValueSpecs(QueryBuilderHelpers::SerializeECClassMapPolymorphically(labelOverridingProperties, *info.GetSelectClass()));

        NavigationQueryContractCPtr contract = queryContext->GetContract(info);
        if (contract.IsNull())
            continue;

        ComplexNavigationQueryPtr query = ComplexNavigationQuery::Create();
        query->SelectContract(*contract, "this");
        query->From(*info.GetSelectClass(), info.GetSelectPolymorphically(), "this");

        if (queryContext->Accept(query, info))
            OnSelected(info, m_params);
        }

    return queryContext->GetQueries();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<NavigationQueryPtr> NavigationQueryBuilder::GetQueries(JsonNavNodeCP parentNode, RelatedInstanceNodesSpecification const& specification, Utf8StringCR specificationHash, ChildNodeRuleCR rule) const
    {
    Utf8String supportedSchemas = GetSupportedSchemas(specification);
    if (supportedSchemas.empty())
        {
        LoggingHelper::LogMessage(Log::Navigation, "Neither rule set nor RelatedInstanceNodesSpecification have specified supported schemas. "
            "Consider specifying a value for this property as it can affect performance significantly", NativeLogging::LOG_WARNING, true);
        }

    GroupingResolver groupingResolver(m_params.GetSchemaHelper(), m_params, parentNode, specificationHash, specification);
    MultiQueryContextPtr queryContext = CreateQueryContext(groupingResolver);

    // this specification can be used only if parent node is ECInstance node
    if (nullptr == groupingResolver.GetParentInstanceNode() || nullptr == groupingResolver.GetParentInstanceNode()->GetKey()->AsECInstanceNodeKey())
        {
        LoggingHelper::LogMessage(Log::Navigation, "AllRelatedInstanceNodes and RelatedInstanceNodes specifications can only be used "
            "if parent node or any of of its ancestor nodes is an ECInstance node", NativeLogging::LOG_ERROR);
        return bvector<NavigationQueryPtr>();
        }

    // determine instance label overrides
    bmap<ECClassCP, bvector<InstanceLabelOverrideValueSpecification const*>> labelOverridingProperties = QueryBuilderHelpers::GetLabelOverrideValuesMap(m_params.GetSchemaHelper(),
        m_params.GetRuleset().GetInstanceLabelOverrides());

    // get the parent instance keys
    bvector<ECClassInstanceKey> const& parentInstanceKeys = groupingResolver.GetParentInstanceNode()->GetKey()->AsECInstanceNodeKey()->GetInstanceKeys();
    bmap<ECClassCP, bvector<ECInstanceId>> parentClassInstanceIdsMap = GroupClassInstanceKeys(parentInstanceKeys);

    // iterate over all parent classes
    for (auto const& entry : parentClassInstanceIdsMap)
        {
        ECEntityClassCR parentClass = *entry.first->GetEntityClassCP();
        bvector<ECInstanceId> const& parentInstanceIds = entry.second;
        if (groupingResolver.IsGroupedByRelationship())
            {
            ECRelationshipClassCP groupingRelationship = groupingResolver.GetGroupingRelationship();
            ECRelatedInstanceDirection groupingRelationshipDirection = groupingResolver.GetGroupingRelationshipDirection();
            ECRelationshipConstraintClassList relationshipEnds = m_params.GetSchemaHelper().GetRelationshipConstraintClasses(*groupingRelationship,
                groupingRelationshipDirection, supportedSchemas);
            bool isForwardJoin = (ECRelatedInstanceDirection::Backward == groupingRelationshipDirection);
            for (ECClassCP relationshipEnd : relationshipEnds)
                {
                Utf8String relationshipAlias = Utf8String("rel_").append(groupingRelationship->GetSchema().GetAlias()).append("_").append(groupingRelationship->GetName());
                RelatedClassPath path;
                path.push_back(RelatedClass(parentClass, *relationshipEnd, *groupingRelationship, isForwardJoin, nullptr, relationshipAlias.c_str()));
                SelectQueryInfo selectInfo = CreateSelectInfo(m_params.GetSchemaHelper(), *queryContext,
                    parentClass, path, SelectionPurpose::Include, specification);
                selectInfo.SetLabelOverrideValueSpecs(QueryBuilderHelpers::SerializeECClassMapPolymorphically(labelOverridingProperties, *selectInfo.GetSelectClass()));

                NavigationQueryContractPtr contract = queryContext->GetContract(selectInfo);
                if (contract.IsNull())
                    continue;

                ComplexNavigationQueryPtr query = CreateQuery(*contract, selectInfo, m_params, *parentNode, parentClass, parentInstanceIds,
                    FormatInstanceFilter(specification.GetInstanceFilter()), 0 != specification.GetSkipRelatedLevel());
                if (queryContext->Accept(query, selectInfo))
                    OnSelected(selectInfo, m_params);
                }
            }
        else
            {
            // determine the relationship direction
            int relationshipDirection = 0;
            switch (specification.GetRequiredRelationDirection())
                {
                case RequiredRelationDirection_Forward:  relationshipDirection = (int)ECRelatedInstanceDirection::Forward; break;
                case RequiredRelationDirection_Backward: relationshipDirection = (int)ECRelatedInstanceDirection::Backward; break;
                case RequiredRelationDirection_Both:     relationshipDirection = (int)ECRelatedInstanceDirection::Forward | (int)ECRelatedInstanceDirection::Backward; break;
                }

            // find all applying relationship paths
            ECSchemaHelper::RelationshipClassPathOptions options(parentClass, relationshipDirection, specification.GetSkipRelatedLevel(),
                supportedSchemas.c_str(), specification.GetRelationshipClassNames().c_str(), specification.GetRelatedClassNames().c_str(),
                queryContext->GetRelationshipUseCounter(), groupingResolver.GetGroupingClass());
            bvector<bpair<RelatedClassPath, bool>> relationshipClassPaths = m_params.GetSchemaHelper().GetRelationshipClassPaths(options);

            // create a select info for each path
            bool hasIncludes = false;
            bvector<SelectQueryInfo> selectInfos;
            for (bpair<RelatedClassPath, bool> const& pair : relationshipClassPaths)
                {
                selectInfos.push_back(CreateSelectInfo(m_params.GetSchemaHelper(), *queryContext,
                    parentClass, pair.first, pair.second ? SelectionPurpose::Include : SelectionPurpose::Exclude, specification));
                if (pair.second)
                    hasIncludes = true;
                }

            // quick return if there are no includes
            if (!hasIncludes)
                return bvector<NavigationQueryPtr>();

            // may need to split some base classes into their derived classes if there are customization rules that only apply for
            // derived classes.
            ProcessQueryClassesBasedOnCustomizationRules(selectInfos, groupingResolver, GetMapKeys(labelOverridingProperties),
                groupingResolver.GetParentInstanceNode(), m_params);

            // union everything
            for (SelectQueryInfo& info : selectInfos)
                {
                info.SetLabelOverrideValueSpecs(QueryBuilderHelpers::SerializeECClassMapPolymorphically(labelOverridingProperties, *info.GetSelectClass()));
                NavigationQueryContractPtr contract = queryContext->GetContract(info);
                if (contract.IsValid())
                    {
                    ComplexNavigationQueryPtr query = CreateQuery(*contract, info, m_params, *parentNode, parentClass, parentInstanceIds,
                        FormatInstanceFilter(specification.GetInstanceFilter()), 0 != specification.GetSkipRelatedLevel());
                    if (queryContext->Accept(query, info))
                        OnSelected(info, m_params);
                    }
                }
            }
        }

    return queryContext->GetQueries();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<NavigationQueryPtr> NavigationQueryBuilder::GetQueries(JsonNavNodeCP parentNode, InstanceNodesOfSpecificClassesSpecification const& specification, ChildNodeRuleCR rule) const
    {
    GroupingResolver groupingResolver(m_params.GetSchemaHelper(), m_params, parentNode, specification.GetHash(), specification);
    MultiQueryContextPtr queryContext = CreateQueryContext(groupingResolver);
    bool areQueriesPolymorphic = specification.GetArePolymorphic() && !groupingResolver.IsGroupedByClass();

    bvector<SelectQueryInfo> selectInfos;
    if (groupingResolver.IsGroupedByClass())
        {
        selectInfos.push_back(CreateSelectInfo(specification, *groupingResolver.GetGroupingClass(), false));
        }
    else
        {
        // WIP: in case of class grouping, need to query against a metadata table to pick all matching classes
        SupportedEntityClassInfos queryClassesInfos = m_params.GetSchemaHelper().GetECClassesFromClassList(specification.GetClassNames(), false);
        for (SupportedEntityClassInfo const& queryClassInfo : queryClassesInfos)
            {
            bool isPolymorphic = areQueriesPolymorphic && queryClassInfo.IsPolymorphic();
            selectInfos.push_back(CreateSelectInfo(specification, queryClassInfo.GetClass(), isPolymorphic));
            }
        }

    // quick return if nothing to select from
    if (selectInfos.empty())
        return bvector<NavigationQueryPtr>();

    // determine instance label overrides
    bmap<ECClassCP, bvector<InstanceLabelOverrideValueSpecification const*>> labelOverridingProperties = QueryBuilderHelpers::GetLabelOverrideValuesMap(m_params.GetSchemaHelper(),
        m_params.GetRuleset().GetInstanceLabelOverrides());

    // determine additional related joins
    for (SelectQueryInfo& info : selectInfos)
        {
        info.GetRelatedClasses() = QueryBuilderHelpers::GetRelatedInstanceClasses(m_params.GetSchemaHelper(), *info.GetSelectClass(),
            specification.GetRelatedInstances(), queryContext->GetRelationshipUseCounter());
        }

    // may need to split some base classes into their derived classes if there are customization rules that only apply for
    // derived classes.
    ProcessQueryClassesBasedOnCustomizationRules(selectInfos, groupingResolver, GetMapKeys(labelOverridingProperties),
        groupingResolver.GetParentInstanceNode(), m_params);

    // union everything
    for (SelectQueryInfo& info : selectInfos)
        {
        info.SetLabelOverrideValueSpecs(QueryBuilderHelpers::SerializeECClassMapPolymorphically(labelOverridingProperties, *info.GetSelectClass()));
        NavigationQueryContractCPtr contract = queryContext->GetContract(info);
        if (contract.IsNull())
            continue;

        ComplexNavigationQueryPtr query = ComplexNavigationQuery::Create();
        query->SelectContract(*contract, "this");
        query->From(*info.GetSelectClass(), info.GetSelectPolymorphically(), "this");
        ApplyInstanceFilter(*query, info, m_params, FormatInstanceFilter(specification.GetInstanceFilter()), groupingResolver.GetParentInstanceNode());

        if (queryContext->Accept(query, info))
            OnSelected(info, m_params);
        }

    return queryContext->GetQueries();
    }

#define SEARCH_QUERY_INJECTION      ", ECInstanceId AS [" SEARCH_QUERY_FIELD_ECInstanceId "], ECClassId AS [" SEARCH_QUERY_FIELD_ECClassId "]"
/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                11/2016
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
    * @bsimethod                                    Grigas.Petraitis            11/2016
    +---------------+---------------+---------------+---------------+-----------+------*/
    void _Visit(StringQuerySpecificationCR spec) override
        {
        m_query = spec.GetQuery();
        InjectRulesEngineFields(m_query);
        }

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis            11/2016
    +---------------+---------------+---------------+---------------+-----------+------*/
    void _Visit(ECPropertyValueQuerySpecificationCR spec) override
        {
        if (nullptr == m_parentNode || nullptr == m_parentNode->GetKey()->AsECInstanceNodeKey())
            {
            LoggingHelper::LogMessage(Log::Navigation, "ECPropertyValueQuerySpecification can only be used when there is a parent ECInstance "
                "node up in the hierarchy", NativeLogging::LOG_ERROR);
            return;
            }

        ECInstancesNodeKey const& key = *m_parentNode->GetKey()->AsECInstanceNodeKey();
        bmap<ECClassCP, bvector<ECInstanceId>> parentClassInstanceIds = GroupClassInstanceKeys(key.GetInstanceKeys());
        ECClassInstanceKey usedParentInstanceKey;
        for (auto const& entry : parentClassInstanceIds)
            {
            ECClassCP parentClass = entry.first;
            BeAssert(parentClass->IsEntityClass());

            ECPropertyCP queryProperty = parentClass->GetPropertyP(spec.GetParentPropertyName().c_str());
            if (nullptr == queryProperty)
                {
                LoggingHelper::LogMessage(Log::Navigation, Utf8PrintfString("Parent instance ECClass %s doesn't contain an ECProperty %s specified by "
                    "ECPropertyValueQuerySpecification", parentClass->GetFullName(), spec.GetParentPropertyName().c_str()).c_str(), NativeLogging::LOG_INFO);
                continue;
                }
            if (!queryProperty->GetIsPrimitive() || PRIMITIVETYPE_String != queryProperty->GetAsPrimitiveProperty()->GetType())
                {
                LoggingHelper::LogMessage(Log::Navigation, Utf8PrintfString("The ECProperty %s in ECClass %s is not of string type. ECPropertyValueQuerySpecification "
                    "requires a string property", queryProperty->GetName().c_str(), parentClass->GetFullName()).c_str(), NativeLogging::LOG_INFO);
                continue;
                }

            for (ECInstanceId instanceId : entry.second)
                {
                ECValue propertyValue = ECInstancesHelper::GetValue(m_helper.GetConnection(), ECInstanceKey(parentClass->GetId(), instanceId), queryProperty->GetName().c_str());
                if (!propertyValue.IsString())
                    {
                    BeAssert(false);
                    LoggingHelper::LogMessage(Log::Navigation, "The ECProperty value used by ECPropertyValueQuerySpecification must be a string", NativeLogging::LOG_ERROR);
                    continue;
                    }

                usedParentInstanceKey = ECClassInstanceKey(parentClass, instanceId);
                m_query = propertyValue.GetUtf8CP();
                break;
                }

            if (usedParentInstanceKey.IsValid())
                break;
            }

        if (nullptr != m_usedClassesListener && usedParentInstanceKey.IsValid())
            m_usedClassesListener->_OnClassUsed(*usedParentInstanceKey.GetClass(), false);

        InjectRulesEngineFields(m_query);
        }

public:
    SearchQueryCreator(ECSchemaHelper const& helper, NavNodeCP parentNode, IUsedClassesListener* usedClassesListener)
        : m_helper(helper), m_parentNode(parentNode), m_usedClassesListener(usedClassesListener)
        {}
    Utf8StringCR GetQuery() const {return m_query;}
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String GetQuery(ECSchemaHelper const& helper, QuerySpecification const& specification, NavNodeCP parentNode, IUsedClassesListener* usedClassesListener)
    {
    SearchQueryCreator creator(helper, parentNode, usedClassesListener);
    specification.Accept(creator);
    return creator.GetQuery();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<NavigationQueryPtr> NavigationQueryBuilder::GetQueries(JsonNavNodeCP parentNode, SearchResultInstanceNodesSpecification const& specification, ChildNodeRuleCR rule) const
    {
    if (specification.GetQuerySpecifications().empty())
        {
        LoggingHelper::LogMessage(Log::Navigation, "SearchResultInstanceNodes specification has no queries specified.", NativeLogging::LOG_ERROR);
        return bvector<NavigationQueryPtr>();
        }

    GroupingResolver groupingResolver(m_params.GetSchemaHelper(), m_params, parentNode, specification.GetHash(), specification);
    MultiQueryContextPtr queryContext = CreateQueryContext(groupingResolver, true);

    // determine instance label overrides
    bmap<ECClassCP, bvector<InstanceLabelOverrideValueSpecification const*>> labelOverridingProperties = QueryBuilderHelpers::GetLabelOverrideValuesMap(m_params.GetSchemaHelper(),
        m_params.GetRuleset().GetInstanceLabelOverrides());

    // create a query for each class
    for (QuerySpecification* querySpecification : specification.GetQuerySpecifications())
        {
        ECClassCP ecClass = m_params.GetSchemaHelper().GetECClass(querySpecification->GetSchemaName().c_str(), querySpecification->GetClassName().c_str());
        if (nullptr == ecClass)
            {
            LoggingHelper::LogMessage(Log::Navigation, Utf8PrintfString("The specified search query class %s:%s not found.",
                querySpecification->GetSchemaName().c_str(), querySpecification->GetClassName().c_str()).c_str(), NativeLogging::LOG_ERROR);
            continue;
            }

        ECEntityClassCP entityClass = ecClass->GetEntityClassCP();
        if (nullptr == entityClass)
            {
            LoggingHelper::LogMessage(Log::Navigation, Utf8PrintfString("Search query class %s:%s is not an entity class.",
                querySpecification->GetSchemaName().c_str(), querySpecification->GetClassName().c_str()).c_str(), NativeLogging::LOG_ERROR);
            continue;
            }

        if (groupingResolver.IsGroupedByClass() && !groupingResolver.GetGroupingClass()->Is(entityClass))
            continue;

        SelectQueryInfo selectInfo = CreateSelectInfo(specification, *entityClass, true);
        selectInfo.GetRelatedClasses() = QueryBuilderHelpers::GetRelatedInstanceClasses(m_params.GetSchemaHelper(), *selectInfo.GetSelectClass(),
            specification.GetRelatedInstances(), queryContext->GetRelationshipUseCounter());

        // may need to split some base classes into their derived classes if there are customization rules that only apply for
        // derived classes.
        bvector<SelectQueryInfo> selectInfos;
        selectInfos.push_back(std::move(selectInfo));
        ProcessQueryClassesBasedOnCustomizationRules(selectInfos, groupingResolver, GetMapKeys(labelOverridingProperties),
            groupingResolver.GetParentInstanceNode(), m_params);

        for (SelectQueryInfo& info : selectInfos)
            {
            info.SetLabelOverrideValueSpecs(QueryBuilderHelpers::SerializeECClassMapPolymorphically(labelOverridingProperties, *info.GetSelectClass()));
            NavigationQueryContractCPtr contract = queryContext->GetContract(info);
            if (contract.IsNull())
                continue;

            Utf8String searchQuery = GetQuery(m_params.GetSchemaHelper(), *querySpecification, groupingResolver.GetParentInstanceNode(), m_params.GetUsedClassesListener());
            if (searchQuery.empty())
                {
                BeAssert(false);
                continue;
                }

            ComplexNavigationQueryPtr query = ComplexNavigationQuery::Create();
            query->SelectContract(*contract, SEARCH_QUERY_Alias);
            query->From(*StringNavigationQuery::Create(searchQuery), SEARCH_QUERY_Alias);

            if (groupingResolver.IsGroupedByClass())
                {
                query->Where(Utf8PrintfString("[" SEARCH_QUERY_Alias "].[%s] = ?", SEARCH_QUERY_FIELD_ECClassId).c_str(),
                    {new BoundQueryId(groupingResolver.GetGroupingClass()->GetId())});
                }
            else if (!info.GetSelectPolymorphically())
                {
                query->Where(Utf8PrintfString("[" SEARCH_QUERY_Alias "].[%s] = ?", SEARCH_QUERY_FIELD_ECClassId).c_str(),
                    {new BoundQueryId(info.GetSelectClass()->GetId())});
                }

            if (queryContext->Accept(query, info))
                OnSelected(info, m_params);
            }
        }

    return queryContext->GetQueries();
    }
