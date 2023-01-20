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
struct NavNodesDataSource : IDataSource<NavNodePtr>
{
private:
    bool m_supportsFiltering;
protected:
    NavNodesDataSource() : m_supportsFiltering(false) {}
public:
    //! Does the returned data source support filtering.
    bool SupportsFiltering() const {return m_supportsFiltering;}
    void SetSupportsFiltering(bool value) {m_supportsFiltering = value;}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ProviderBasedNodesDataSource : NavNodesDataSource
{
private:
    NavNodesProviderCPtr m_nodesProvider;
private:
    ProviderBasedNodesDataSource(NavNodesProviderCR nodesProvider)
        : m_nodesProvider(&nodesProvider)
        {}
protected:
    NavNodePtr _Get(size_t index) const override {auto iter = m_nodesProvider->begin() + index; return iter != m_nodesProvider->end() ? *iter : nullptr;}
    size_t _GetSize() const override {return m_nodesProvider->GetNodesCount();}
    Iterator _CreateFrontIterator() const override {return m_nodesProvider->begin();}
    Iterator _CreateBackIterator() const override {return m_nodesProvider->end();}
public:
    static RefCountedPtr<ProviderBasedNodesDataSource> Create(NavNodesProviderCR nodesProvider) {return new ProviderBasedNodesDataSource(nodesProvider);}
    NavNodesProviderCPtr GetProvider() const {return m_nodesProvider;}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ConstNodesDataSource : IDataSource<NavNodeCPtr>
{
private:
    RefCountedCPtr<IDataSource<NavNodePtr>> m_wrappedDataSource;
    ConstNodesDataSource(IDataSource<NavNodePtr> const& wrappedDataSource) : m_wrappedDataSource(&wrappedDataSource) {}
protected:
    NavNodeCPtr _Get(size_t index) const override {return m_wrappedDataSource->Get(index);}
    size_t _GetSize() const override {return m_wrappedDataSource->GetSize();}
    Iterator _CreateFrontIterator() const override {return Iterator(std::make_unique<IterableIteratorImpl<IDataSource<NavNodePtr>::Iterator, NavNodeCPtr>>(m_wrappedDataSource->begin()));}
    Iterator _CreateBackIterator() const override {return Iterator(std::make_unique<IterableIteratorImpl<IDataSource<NavNodePtr>::Iterator, NavNodeCPtr>>(m_wrappedDataSource->end()));}
public:
    static RefCountedPtr<ConstNodesDataSource> Create(IDataSource<NavNodePtr> const& wrappedDataSource) {return new ConstNodesDataSource(wrappedDataSource);}
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
