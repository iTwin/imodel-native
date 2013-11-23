/*--------------------------------------------------------------------------------------+
|
|     $Source: src/ecinstanceiterable.cpp $
|
|   $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"
#include <ECObjects\ECObjectsAPI.h>
USING_NAMESPACE_EC


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECInstanceIterable::ECInstanceIterable(IECInstanceCollectionAdapter* collection)
    :m_collectionPtr(collection)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECInstanceIterable::const_iterator  ECInstanceIterable::begin () const
    {
    return m_collectionPtr.IsNull() ? const_iterator(NULL) : m_collectionPtr->begin();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECInstanceIterable::const_iterator ECInstanceIterable::end() const
    {
    return m_collectionPtr.IsNull() ? const_iterator(NULL) : m_collectionPtr->end();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ECInstanceIterable::empty() const
    {
    return m_collectionPtr.IsNull() || m_collectionPtr->begin() == m_collectionPtr->end();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ECInstanceIterable::IsNull () const
    {
    return m_collectionPtr.IsNull();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECRelationshipIterable::ECRelationshipIterable(IECRelationshipCollectionAdapter* collection)
    :m_collectionPtr(collection)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECRelationshipIterable::const_iterator  ECRelationshipIterable::begin () const
    {
    return m_collectionPtr.IsNull() ? const_iterator(NULL) : m_collectionPtr->begin();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECRelationshipIterable::const_iterator ECRelationshipIterable::end() const
    {
    return m_collectionPtr.IsNull() ? const_iterator(NULL) : m_collectionPtr->end();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ECRelationshipIterable::empty() const
    {
    return m_collectionPtr.IsNull() || m_collectionPtr->begin() == m_collectionPtr->end();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ECRelationshipIterable::IsNull () const
    {
    return m_collectionPtr.IsNull();
    }