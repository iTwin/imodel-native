/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECPresentation/ECPresentation.h>
#include <ECPresentation/ECPresentationManager.h>
#include <ECPresentation/Rules/RelatedInstanceNodesSpecification.h>
#include "../RulesEngineTypes.h"
#include "NavNodeProviders.h"
#include "DataSourceInfo.h"

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
typedef IDataSource<NavNodePtr> INavNodesDataSource;
DEFINE_POINTER_SUFFIX_TYPEDEFS_NO_STRUCT(INavNodesDataSource)
typedef RefCountedPtr<INavNodesDataSource> INavNodesDataSourcePtr;
typedef RefCountedPtr<INavNodesDataSource const> INavNodesDataSourceCPtr;

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct EXPORT_VTABLE_ATTRIBUTE NavNodesDataSource : INavNodesDataSource
{
private:
    NavNodesProviderCPtr m_nodesProvider;

private:
    NavNodesDataSource(NavNodesProviderCR nodesProvider) : m_nodesProvider(&nodesProvider) {}
    static NavNodePtr TransformJsonNodeToNode(NavNodePtr const& node) {return node;}

protected:
    ECPRESENTATION_EXPORT NavNodePtr _Get(size_t index) const override {auto iter = m_nodesProvider->begin() + index; return iter != m_nodesProvider->end() ? *iter : nullptr;}
    size_t _GetSize() const override {return m_nodesProvider->GetNodesCount();}
    Iterator _CreateFrontIterator() const override {return Iterator(std::make_unique<TransformingIterableIteratorImpl<NavNodesProvider::Iterator, NavNodePtr, NavNodePtr>>(m_nodesProvider->begin(), &NavNodesDataSource::TransformJsonNodeToNode));}
    Iterator _CreateBackIterator() const override {return Iterator(std::make_unique<TransformingIterableIteratorImpl<NavNodesProvider::Iterator, NavNodePtr, NavNodePtr>>(m_nodesProvider->end(), &NavNodesDataSource::TransformJsonNodeToNode));}

public:
    static NavNodesDataSourcePtr Create(NavNodesProviderCR nodesProvider) {return new NavNodesDataSource(nodesProvider);}
    NavNodesProviderCPtr GetProvider() const {return m_nodesProvider;}
};

/*=================================================================================**//**
* @bsiclass
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
