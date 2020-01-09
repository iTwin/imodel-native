/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECPresentation/ECPresentation.h>
#include "NavigationQuery.h"

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

#define RULES_ENGINE_LOCAL_STATE_NAMESPACE                  "RulesEngine"
#define RULES_ENGINE_ACTIVE_GROUPS_LOCAL_STATE_NAMESPACE    RULES_ENGINE_LOCAL_STATE_NAMESPACE "_ActiveGroups"

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                07/2017
+===============+===============+===============+===============+===============+======*/
struct IParsedInput
    {
    protected:
        virtual bvector<ECClassCP> const& _GetClasses() const = 0;
        virtual bvector<BeSQLite::EC::ECInstanceId> const& _GetInstanceIds(ECClassCR) const = 0;
    public:
        virtual ~IParsedInput() {}
        bvector<ECClassCP> const& GetClasses() const { return _GetClasses(); }
        bvector<BeSQLite::EC::ECInstanceId> const& GetInstanceIds(ECClassCR selectClass) const { return _GetInstanceIds(selectClass); }
    };

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                02/2018
+===============+===============+===============+===============+===============+======*/
struct InstanceFilteringParams
{
    struct RecursiveQueryInfo
    {
    private:
        bvector<RelatedClassPath> m_pathsFromInputToSelectClass;
    public:
        RecursiveQueryInfo(bvector<RelatedClassPath> pathsFromInputToSelectClass) : m_pathsFromInputToSelectClass(pathsFromInputToSelectClass) {}
        bvector<RelatedClassPath> const& GetPathsFromInputToSelectClass() const {return m_pathsFromInputToSelectClass;}
    };

private:
    IConnectionCR m_connection;
    ECExpressionsCache& m_ecexpressionsCache;
    SelectClassInfo const& m_selectInfo;
    IParsedInput const* m_selection;
    RecursiveQueryInfo const* m_recursiveQueryInfo;
    Utf8CP m_instanceFilter;

public:
    InstanceFilteringParams(IConnectionCR connection, ECExpressionsCache& ecexpressionsCache, IParsedInput const* selection,
        SelectClassInfo const& selectInfo, RecursiveQueryInfo const* recursiveQueryInfo, Utf8CP instanceFilter)
        : m_connection(connection), m_ecexpressionsCache(ecexpressionsCache), m_selection(selection), m_selectInfo(selectInfo),
        m_recursiveQueryInfo(recursiveQueryInfo), m_instanceFilter(instanceFilter)
        {}
    IConnectionCR GetConnection() const {return m_connection;}
    ECExpressionsCache& GetECExpressionsCache() const {return m_ecexpressionsCache;}
    IParsedInput const* GetInput() const {return m_selection;}
    SelectClassInfo const& GetSelectInfo() const {return m_selectInfo;}
    RecursiveQueryInfo const* GetRecursiveQueryInfo() const {return m_recursiveQueryInfo;}
    Utf8CP GetInstanceFilter() const {return m_instanceFilter;}
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                06/2019
+===============+===============+===============+===============+===============+======*/
enum class InstanceFilteringResult
    {
    Success,    //!< Filter appied successfully
    NoResults,  //!< Returned when it's clear the query will return no results after applying the filter
    };

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                12/2019
+===============+===============+===============+===============+===============+======*/
struct SelectClassSplitResult
{
private:
    bvector<SelectClassWithExcludes> m_splitPath;
    SelectClassWithExcludes m_selectClass;
public:
    SelectClassSplitResult() {}
    SelectClassSplitResult(SelectClassWithExcludes selectClass) : m_selectClass(selectClass) {}
    SelectClassWithExcludes const& GetSelectClass() const {return m_selectClass;}
    SelectClassWithExcludes& GetSelectClass() {return m_selectClass;}
    bvector<SelectClassWithExcludes> const& GetSplitPath() const {return m_splitPath;}
    bvector<SelectClassWithExcludes>& GetSplitPath() {return m_splitPath;}
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                04/2015
+===============+===============+===============+===============+===============+======*/
struct QueryBuilderHelpers
{
private:
    QueryBuilderHelpers() {}

public:
    template<typename T> static Utf8String GetOrderByClause(T const& query);
    template<typename T> static void SetOrUnion(RefCountedPtr<T>& target, T& source);
    template<typename T> static void Where(RefCountedPtr<T>& query, Utf8CP clause, BoundQueryValuesListCR bindings);
    template<typename T> static void Order(T& query, Utf8CP clause);
    template<typename T> static void Limit(T& query, uint64_t limit, uint64_t offset = 0);
    static Utf8String Escape(Utf8String input);

    template<typename T> static RefCountedPtr<ComplexPresentationQuery<T>> CreateNestedQuery(T& innerQuery);
    template<typename T> static bool NeedsNestingToUseAlias(T const& query, bvector<Utf8CP> const& aliases);
    template<typename T> static RefCountedPtr<T> CreateNestedQueryIfNecessary(T& query, bvector<Utf8CP> const& aliases);
    template<typename T> static RefCountedPtr<ComplexPresentationQuery<T>> CreateComplexNestedQueryIfNecessary(T& query, bvector<Utf8CP> const& aliases);

    template<typename T> static InstanceFilteringResult ApplyInstanceFilter(ComplexPresentationQuery<T>&, InstanceFilteringParams const&, RelatedClassPath);
    template<typename T> static void FilterOutExcludes(ComplexPresentationQuery<T>&, Utf8CP alias, bvector<SelectClass> const& excludes, SchemaManagerCR);

    static bvector<SelectClassSplitResult> ProcessSelectClassesBasedOnCustomizationRules(bvector<SelectClass> const& selectClasses,
        bvector<RuleApplicationInfo> const& customizationRuleInfos, SchemaManagerCR schemas);
    static bvector<RelatedClassPath> ProcessRelationshipPathsBasedOnCustomizationRules(bvector<RelatedClassPath> const& relationshipPaths,
        bvector<RuleApplicationInfo> const& customizationRuleInfos, SchemaManagerCR);

    template<typename T> static bvector<T> SerializeECClassMapPolymorphically(bmap<ECClassCP, bvector<T>> const& map, ECClassCR ecClass)
        {
        bvector<T> list;
        auto iter = map.find(&ecClass);
        if (iter != map.end())
            list.insert(list.end(), iter->second.begin(), iter->second.end());
        for (ECClassCP baseClass : ecClass.GetBaseClasses())
            {
            bvector<T> baseList = SerializeECClassMapPolymorphically(map, *baseClass);
            std::move(baseList.begin(), baseList.end(), std::back_inserter(list));
            }
        return list;
        }

    static void ApplyDescriptorOverrides(RefCountedPtr<ContentQuery>& query, ContentDescriptorCR ovr, ECExpressionsCache&);
    static void ApplyPagingOptions(RefCountedPtr<ContentQuery>& query, PageOptionsCR opts);
    static void ApplyDefaultContentFlags(ContentDescriptorR descriptor, Utf8CP displayType, ContentSpecificationCR);
    static void AddCalculatedFields(ContentDescriptorR, CalculatedPropertiesSpecificationList const&, ILocalizationProvider const*, Utf8StringCR, PresentationRuleSetCR, ECClassCP);
    static void Aggregate(ContentDescriptorPtr& aggregateDescriptor, ContentDescriptorR inputDescriptor);
    static ContentQueryPtr CreateMergedResultsQuery(ContentQueryR, ContentDescriptorR);

    static bmap<ECClassCP, bvector<InstanceLabelOverrideValueSpecification const*>> GetLabelOverrideValuesMap(ECSchemaHelper const& helper, InstanceLabelOverrideList labelOverrides);
    static PresentationQueryContractFieldPtr CreateInstanceLabelField(Utf8CP name, bvector<InstanceLabelOverrideValueSpecification const*> const& labelOverrideValueSpecs, PresentationQueryContractField const* fallback = nullptr, PresentationQueryContractFieldCP instanceIdField = nullptr, PresentationQueryContractFieldCP classIdField = nullptr);
    static GenericQueryPtr CreateInstanceLabelQuery(ECClassInstanceKeyCR key, bvector<InstanceLabelOverrideValueSpecification const*> const& labelOverrideValueSpecs);
    static Utf8String CreateDisplayLabelValueClause(Utf8CP fieldName);

    static bvector<RelatedClass> GetRelatedInstanceClasses(ECSchemaHelper const& schemaHelper, ECClassCR selectClass, RelatedInstanceSpecificationList const& relatedInstanceSpecs, bmap<ECRelationshipClassCP, int>& relationshipUsedCount);

    static IdSet<BeInt64Id> CreateIdSetFromJsonArray(RapidJsonValueCR);
    static ECValue CreateECValueFromJson(RapidJsonValueCR);

    static GroupSpecificationCP GetActiveGroupingSpecification(GroupingRuleCR, IJsonLocalState const*);

    static Utf8String CreateClassNameForDescriptor(ECClassCR ecClass) {return Utf8String(ecClass.GetSchema().GetAlias()).append("_").append(ecClass.GetName());}
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
