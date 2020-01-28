/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECPresentation/ECPresentationTypes.h>
#include "../../../Localization/Xliffs/ECPresentation.xliff.h"
#include "../../../Localization/Xliffs/RulesEngine.xliff.h"

// nodes
ECPRESENTATION_TYPEDEFS(JsonNavNode)
ECPRESENTATION_REFCOUNTED_PTR(JsonNavNode)
ECPRESENTATION_TYPEDEFS(JsonNavNodesFactory)

// content
ECPRESENTATION_TYPEDEFS(ContentDescriptor)

// misc.
ECPRESENTATION_TYPEDEFS(RulesDrivenProviderContext)
ECPRESENTATION_REFCOUNTED_PTR(RulesDrivenProviderContext)

// nav node data sources
ECPRESENTATION_TYPEDEFS(INavNodesDataSource)
ECPRESENTATION_REFCOUNTED_PTR(INavNodesDataSource)
ECPRESENTATION_REFCOUNTED_PTR(NavNodesDataSource)
ECPRESENTATION_REFCOUNTED_PTR(PagingDataSource)
ECPRESENTATION_REFCOUNTED_PTR(CachingNavNodesDataSource)
ECPRESENTATION_REFCOUNTED_PTR(EmptyNavNodesDataSource)

// nav node providers
ECPRESENTATION_TYPEDEFS(INodesProviderContextFactory)
ECPRESENTATION_TYPEDEFS(INodesProviderFactory)
ECPRESENTATION_TYPEDEFS(NavNodesProviderContext)
ECPRESENTATION_REFCOUNTED_PTR(NavNodesProviderContext)
ECPRESENTATION_TYPEDEFS(NavNodesProvider)
ECPRESENTATION_REFCOUNTED_PTR(NavNodesProvider)
ECPRESENTATION_TYPEDEFS(MultiNavNodesProvider)
ECPRESENTATION_TYPEDEFS(EmptyNavNodesProvider)
ECPRESENTATION_TYPEDEFS(SingleNavNodeProvider)

// content data sources
ECPRESENTATION_TYPEDEFS(ContentSetDataSource)
ECPRESENTATION_REFCOUNTED_PTR(ContentSetDataSource)

// content providers
ECPRESENTATION_TYPEDEFS(ContentProviderContext)
ECPRESENTATION_REFCOUNTED_PTR(ContentProviderContext)
ECPRESENTATION_TYPEDEFS(ContentProvider)
ECPRESENTATION_REFCOUNTED_PTR(ContentProvider)
ECPRESENTATION_TYPEDEFS(SpecificationContentProvider)
ECPRESENTATION_REFCOUNTED_PTR(SpecificationContentProvider)
ECPRESENTATION_TYPEDEFS(NestedContentProvider)
ECPRESENTATION_REFCOUNTED_PTR(NestedContentProvider)
// cache
ECPRESENTATION_TYPEDEFS(IHierarchyCache)
ECPRESENTATION_TYPEDEFS(IVirtualNavNodesCache)

// contracts
ECPRESENTATION_TYPEDEFS(PresentationQueryContractField)
ECPRESENTATION_TYPEDEFS(PresentationQueryContract)
ECPRESENTATION_TYPEDEFS(NavigationQueryContract)
ECPRESENTATION_TYPEDEFS(ContentQueryContract)
ECPRESENTATION_REFCOUNTED_PTR(PresentationQueryContractField)
ECPRESENTATION_REFCOUNTED_PTR(PresentationQueryContract)
ECPRESENTATION_REFCOUNTED_PTR(NavigationQueryContract)
ECPRESENTATION_REFCOUNTED_PTR(ContentQueryContract)

// query builder
ECPRESENTATION_TYPEDEFS(NavigationQueryBuilder)
ECPRESENTATION_TYPEDEFS(ContentQueryBuilder)

// update
ECPRESENTATION_REFCOUNTED_PTR(IUpdateTask)
ECPRESENTATION_TYPEDEFS(IUpdateTask)
ECPRESENTATION_TYPEDEFS(UpdateTasksFactory)

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                04/2015
+===============+===============+===============+===============+===============+======*/
enum class NavigationQueryResultType
    {
    Invalid,
    ECInstanceNodes,
    MultiECInstanceNodes,
    ECRelationshipClassNodes,
    ClassGroupingNodes,
    BaseClassGroupingNodes,
    PropertyGroupingNodes,
    DisplayLabelGroupingNodes,
    };

/*=================================================================================**//**
//! note: Order of enum values is important - it defines the order of grouping handlers!
* @bsiclass                                     Grigas.Petraitis                07/2015
+===============+===============+===============+===============+===============+======*/
enum class GroupingType
    {
    SameLabelInstance = 0,
    DisplayLabel      = 1,
    Property          = 2,
    Class             = 3,
    BaseClass         = 4,
    Relationship      = 5,
    };

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                03/2016
+===============+===============+===============+===============+===============+======*/
template<typename T> struct Holder
    {
    T* m_obj;
    Holder() : m_obj(nullptr) {}
    Holder(T&& obj) : m_obj(new T(std::move(obj))) {}
    ~Holder() {DELETE_AND_CLEAR(m_obj);}
    Holder<T>& operator=(Holder<T>&& other) {DELETE_AND_CLEAR(m_obj); m_obj = other.m_obj; other.m_obj = nullptr; return *this;}
    Holder<T>& operator=(std::nullptr_t) {DELETE_AND_CLEAR(m_obj); return *this;}
    T* get() const {return m_obj;}
    T* operator->() const {return m_obj;}
    T& operator*() const {return *m_obj;}
    bool IsValid() const {return nullptr != m_obj;}
    };

#define COMPARE_VEC(lhs, rhs) \
    if (lhs.size() < rhs.size()) \
        return true; \
    if (lhs.size() > rhs.size()) \
        return false; \
    for (auto const& lhsMember : lhs) \
        { \
        for (auto const& rhsMember : rhs) \
            { \
            if (lhsMember < rhsMember) \
                return true; \
            if (rhsMember < lhsMember) \
                return false; \
            } \
        }

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                11/2016
+===============+===============+===============+===============+===============+======*/
struct RuleApplicationInfo
{
private:
    ECClassCP m_ruleClass;
    bool m_isRulePolymorphic;
public:
    RuleApplicationInfo() : m_ruleClass(nullptr) {}
    RuleApplicationInfo(ECClassCR ruleClass, bool isPolymorphic) : m_ruleClass(&ruleClass), m_isRulePolymorphic(isPolymorphic) {}
    ECClassCP GetRuleClass() const {return m_ruleClass;}
    bool IsRulePolymorphic() const {return m_isRulePolymorphic;}
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                12/2019
+===============+===============+===============+===============+===============+======*/
struct ContainerHelpers
{
private:
    ContainerHelpers() {}

public:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                12/2019
    +---------------+---------------+---------------+---------------+---------------+------*/
    template<typename TTargetContainerType, typename TSourceContainerType, typename TFunctor>
    static TTargetContainerType& TransformContainer(TTargetContainerType& target, TSourceContainerType const& source, TFunctor&& f)
        {
        std::transform(source.begin(), source.end(), std::inserter(target, target.end()), f);
        return target;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                12/2019
    +---------------+---------------+---------------+---------------+---------------+------*/
    template<typename TTargetContainerType, typename TSourceContainerType, typename TFunctor>
    static TTargetContainerType TransformContainer(TSourceContainerType const& source, TFunctor&& f)
        {
        TTargetContainerType target;
        std::transform(source.begin(), source.end(), std::inserter(target, target.end()), f);
        return target;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                12/2019
    +---------------+---------------+---------------+---------------+---------------+------*/
    template<typename TTargetContainerType, typename TSourceContainerType>
    static TTargetContainerType TransformContainer(TSourceContainerType const& source)
        {
        TTargetContainerType target;
        std::copy(source.begin(), source.end(), std::inserter(target, target.end()));
        return target;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                01/2020
    +---------------+---------------+---------------+---------------+---------------+------*/
    template<typename TTargetContainerType, typename TSourceContainerType>
    static TTargetContainerType& Push(TTargetContainerType& target, TSourceContainerType const& source)
        {
        std::copy(source.begin(), source.end(), std::inserter(target, target.end()));
        return target;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                05/2019
    +---------------+---------------+---------------+---------------+---------------+------*/
    template<typename TKey, typename TValue>
    static bvector<TKey> GetMapKeys(bmap<TKey, TValue> const& map)
        {
        return TransformContainer<bvector<TKey>>(map, [](bpair<TKey, TValue> const& entry){return entry.first;});
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                01/2020
    +---------------+---------------+---------------+---------------+---------------+------*/
    template<typename TKey, typename TValue>
    static bvector<TValue> SerializeMapListValues(bmap<TKey, bvector<TValue>> const& map)
        {
        bvector<TValue> serialized;
        for (bpair<TKey, bvector<TValue>> const& entry : map)
            Push(serialized, entry.second);
        return serialized;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                01/2020
    +---------------+---------------+---------------+---------------+---------------+------*/
    template<typename TContainerType, typename TItemType>
    static TContainerType Create(TItemType item)
        {
        TContainerType container;
        container.insert(container.end(), item);
        return container;
        }
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
