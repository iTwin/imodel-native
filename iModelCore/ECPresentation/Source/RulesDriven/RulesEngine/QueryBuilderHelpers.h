/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once 
#include <ECPresentation/ECPresentation.h>
#include "NavigationQuery.h"
#include "ContentSpecificationsHandler.h"

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                02/2018
+===============+===============+===============+===============+===============+======*/
struct InstanceFilteringParams
{
    struct RecursiveQueryInfo
    {
    private:
        bvector<RelatedClassPath> m_pathsToPrimary;
    public:
        RecursiveQueryInfo(bvector<RelatedClassPath> pathsToPrimary) : m_pathsToPrimary(pathsToPrimary) {}
        bvector<RelatedClassPath> const& GetPathsToPrimary() const {return m_pathsToPrimary;}
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
* @bsiclass                                     Grigas.Petraitis                04/2015
+===============+===============+===============+===============+===============+======*/
struct QueryBuilderHelpers
{
private:
    QueryBuilderHelpers() {}
    static void ProcessQueryClassesBasedOnCustomizationRules(ECClassSet& classes, bmap<ECClassCP, bool> const& customizationClasses);

public:
    template<typename T> static void SetOrUnion(RefCountedPtr<T>& target, T& source);
    template<typename T> static void Where(RefCountedPtr<T>& query, Utf8CP clause, BoundQueryValuesListCR bindings);
    template<typename T> static void Order(T& query, Utf8CP clause);
    template<typename T> static void Limit(T& query, uint64_t limit, uint64_t offset = 0);
    static Utf8String Escape(Utf8String input);

    template<typename T> static RefCountedPtr<ComplexPresentationQuery<T>> CreateNestedQuery(T& innerQuery);
    template<typename T> static bool NeedsNestingToUseAlias(T const& query, bvector<Utf8CP> const& aliases);
    template<typename T> static RefCountedPtr<T> CreateNestedQueryIfNecessary(T& query, bvector<Utf8CP> const& aliases);
    template<typename T> static RefCountedPtr<ComplexPresentationQuery<T>> CreateComplexNestedQueryIfNecessary(T& query, bvector<Utf8CP> const& aliases);
    
    template<typename T> static void ApplyInstanceFilter(ComplexPresentationQuery<T>&, InstanceFilteringParams const&, RelatedClassPath);
    
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
    static PresentationQueryContractFieldPtr CreateInstanceLabelField(Utf8CP name, bvector<InstanceLabelOverrideValueSpecification const*> const& labelOverrideValueSpecs, PresentationQueryContractField const* fallback = nullptr);
    static GenericQueryPtr CreateInstanceLabelQuery(ECClassInstanceKeyCR key, bvector<InstanceLabelOverrideValueSpecification const*> const& labelOverrideValueSpecs);

    static bvector<RelatedClass> GetRelatedInstanceClasses(ECSchemaHelper const& schemaHelper, ECClassCR selectClass, RelatedInstanceSpecificationList const& relatedInstanceSpecs, bmap<ECRelationshipClassCP, int>& relationshipUsedCount);

    static IdSet<BeInt64Id> CreateIdSetFromJsonArray(RapidJsonValueCR);
    static ECValue CreateECValueFromJson(RapidJsonValueCR);
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
