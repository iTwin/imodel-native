/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECPresentation/ECPresentation.h>
#include <ECPresentation/IECPresentationManager.h>
#include <ECPresentation/RulesDriven/Rules/RelatedInstanceNodesSpecification.h>
#include "RulesEngineTypes.h"
#include "NavNodeProviders.h"
#include "DataSourceInfo.h"

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                03/2017
+===============+===============+===============+===============+===============+======*/
struct DataSourceFilter
{
    struct RelatedInstanceInfo
        {
        bset<ECClassId> m_relationshipClassIds;
        bvector<ECInstanceKey> m_instanceKeys;
        RequiredRelationDirection m_direction;
        RelatedInstanceInfo() {}
        RelatedInstanceInfo(bset<ECClassId> relationshipClassIds, RequiredRelationDirection direction, bvector<ECInstanceKey> instanceKeys)
            : m_relationshipClassIds(relationshipClassIds), m_direction(direction), m_instanceKeys(instanceKeys)
            {}
        RelatedInstanceInfo(bset<ECClassId> relationshipClassIds, RequiredRelationDirection direction, bvector<ECClassInstanceKey> const& instanceKeys)
            : m_relationshipClassIds(relationshipClassIds), m_direction(direction)
            {
            std::transform(instanceKeys.begin(), instanceKeys.end(), std::back_inserter(m_instanceKeys),
                [](ECClassInstanceKeyCR k){return ECInstanceKey(k.GetClass()->GetId(), k.GetId());});
            }
        bool IsValid() const {return !m_relationshipClassIds.empty() && !m_instanceKeys.empty();}
        };

private:
    RelatedInstanceInfo const* m_relatedInstanceInfo;
    Utf8String m_instanceFilter;

private:
    void InitFromJson(RapidJsonValueCR json);

public:
    DataSourceFilter() : m_relatedInstanceInfo(nullptr) {}
    ECPRESENTATION_EXPORT DataSourceFilter(DataSourceFilter const&);
    ECPRESENTATION_EXPORT DataSourceFilter(DataSourceFilter&&);
    DataSourceFilter(RapidJsonValueCR json) : m_relatedInstanceInfo(nullptr) {InitFromJson(json);}
    DataSourceFilter(Utf8CP instanceFilter) : m_instanceFilter(instanceFilter) {}
    ECPRESENTATION_EXPORT DataSourceFilter(RelatedInstanceInfo const&, Utf8CP instanceFilter);
    ~DataSourceFilter() {DELETE_AND_CLEAR(m_relatedInstanceInfo);}
    DataSourceFilter& operator=(DataSourceFilter const&);
    DataSourceFilter& operator=(DataSourceFilter&&);
    rapidjson::Document AsJson() const;
    RelatedInstanceInfo const* GetRelatedInstanceInfo() const {return m_relatedInstanceInfo;}
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                02/2016
+===============+===============+===============+===============+===============+======*/
typedef IDataSource<NavNodePtr> INavNodesDataSource;
DEFINE_POINTER_SUFFIX_TYPEDEFS_NO_STRUCT(INavNodesDataSource)
typedef RefCountedPtr<INavNodesDataSource> INavNodesDataSourcePtr;
typedef RefCountedPtr<INavNodesDataSource const> INavNodesDataSourceCPtr;

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                09/2015
+===============+===============+===============+===============+===============+======*/
struct EXPORT_VTABLE_ATTRIBUTE NavNodesDataSource : INavNodesDataSource
{
private:
    NavNodesProviderCPtr m_nodesProvider;

private:
    NavNodesDataSource(NavNodesProviderCR nodesProvider) : m_nodesProvider(&nodesProvider) {}
    static NavNodePtr TransformJsonNodeToNode(JsonNavNodePtr const& node) {return node;}

protected:
    ECPRESENTATION_EXPORT NavNodePtr _Get(size_t index) const override;
    ECPRESENTATION_EXPORT size_t _GetSize() const override;
    Iterator _CreateFrontIterator() const override {return Iterator(std::make_unique<TransformingIterableIteratorImpl<NavNodesProvider::Iterator, JsonNavNodePtr, NavNodePtr>>(m_nodesProvider->begin(), &NavNodesDataSource::TransformJsonNodeToNode));}
    Iterator _CreateBackIterator() const override {return Iterator(std::make_unique<TransformingIterableIteratorImpl<NavNodesProvider::Iterator, JsonNavNodePtr, NavNodePtr>>(m_nodesProvider->end(), &NavNodesDataSource::TransformJsonNodeToNode));}

public:
    static NavNodesDataSourcePtr Create(NavNodesProviderCR nodesProvider) {return new NavNodesDataSource(nodesProvider);}
    JsonNavNodePtr GetNode(size_t index) const;
    NavNodesProviderCPtr GetProvider() const {return m_nodesProvider;}
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                12/2016
+===============+===============+===============+===============+===============+======*/
struct ConstNodesDataSource : IDataSource<NavNodeCPtr>
{
private:
    INavNodesDataSourceCPtr m_wrappedDataSource;
    ConstNodesDataSource(INavNodesDataSourceCR wrappedDataSource) : m_wrappedDataSource(&wrappedDataSource) {}
protected:
    NavNodeCPtr _Get(size_t index) const override {return m_wrappedDataSource->Get(index);}
    size_t _GetSize() const override {return m_wrappedDataSource->GetSize();}
    Iterator _CreateFrontIterator() const override {return Iterator(std::make_unique<IterableIteratorImpl<INavNodesDataSource::Iterator, NavNodeCPtr>>(m_wrappedDataSource->begin()));}
    Iterator _CreateBackIterator() const override {return Iterator(std::make_unique<IterableIteratorImpl<INavNodesDataSource::Iterator, NavNodeCPtr>>(m_wrappedDataSource->end()));}
public:
    static RefCountedPtr<ConstNodesDataSource> Create(INavNodesDataSourceCR wrappedDataSource) {return new ConstNodesDataSource(wrappedDataSource);}
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
