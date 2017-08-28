/*--------------------------------------------------------------------------------------+
|
|     $Source: Source/RulesDriven/RulesEngine/RulesEngineTypes.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
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

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                04/2015
+===============+===============+===============+===============+===============+======*/
enum class NavigationQueryResultType
    {
    Invalid,
    ECInstanceNodes,
    ECRelationshipClassNodes,
    SearchNodes,
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
    Holder(T& obj) : m_obj(&obj) {}
    Holder(T&& obj) : m_obj(new T(obj)) {}
    ~Holder() {DELETE_AND_CLEAR(m_obj);}
    Holder<T>& operator=(Holder<T>&& other) {m_obj = other.m_obj; other.m_obj = nullptr; return *this;}
    T* get() const {return m_obj;}
    T* operator->() const {return m_obj;}
    T& operator*() const {return *m_obj;}
    bool IsValid() const {return nullptr != m_obj;}
    };
