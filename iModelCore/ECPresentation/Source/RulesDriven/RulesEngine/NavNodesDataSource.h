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
