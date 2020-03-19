/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include <ECPresentation/RulesDriven/PresentationManager.h>
#include "NavNodesDataSource.h"
#include "NavNodeProviders.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
JsonNavNodePtr NavNodesDataSource::GetNode(size_t index) const
    {
    JsonNavNodePtr node;
    if (!m_nodesProvider->GetNode(node, index))
        return nullptr;
    return node;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodePtr NavNodesDataSource::_GetNode(size_t index) const {return GetNode(index);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
size_t NavNodesDataSource::_GetSize() const {return m_nodesProvider->GetNodesCount();}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodePtr PagingDataSource::_GetNode(size_t index) const
    {
    if (0 != m_pageSize && index >= m_pageSize)
        return nullptr;

    size_t actualIndex = m_pageStart + index;
    size_t sourceSize = m_source->GetSize();
    if (actualIndex >= sourceSize)
        return nullptr;

    return m_source->GetNode(actualIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
size_t PagingDataSource::_GetSize() const
    {
    size_t sourceSize = m_source->GetSize();
    if (m_pageStart >= sourceSize)
        return 0;
    
    if (0 != m_pageSize && sourceSize - m_pageStart > m_pageSize)
        return m_pageSize;

    return sourceSize - m_pageStart;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2020
+---------------+---------------+---------------+---------------+---------------+------*/
PagingDataSource::Iterator PagingDataSource::_CreateFrontIterator() const
    {
    size_t offset = (m_pageStart < m_source->GetSize()) ? m_pageStart : m_source->GetSize();
    return Iterator(std::make_unique<IterableIteratorImpl<INavNodesDataSource::Iterator, NavNodePtr>>(m_source->begin() += offset));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2020
+---------------+---------------+---------------+---------------+---------------+------*/
PagingDataSource::Iterator PagingDataSource::_CreateBackIterator() const 
    {
    size_t offset = ((0 != m_pageSize) && (m_pageStart + m_pageSize < m_source->GetSize())) ? (m_pageStart + m_pageSize) : m_source->GetSize();
    return Iterator(std::make_unique<IterableIteratorImpl<INavNodesDataSource::Iterator, NavNodePtr>>(m_source->begin() += offset));
    }