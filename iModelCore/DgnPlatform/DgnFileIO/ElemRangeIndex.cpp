/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnFileIO/ElemRangeIndex.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   04/02
+---------------+---------------+---------------+---------------+---------------+------*/
static bool isValidRange (DRange3dCR range, bool check3d)
    {
    if ((range.low.x > range.high.x) || (range.low.y> range.high.y))
        return  false;

    if (check3d)
        {
        if (range.low.z > range.high.z) 
            return  false;
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
void ElemRangeIndex::AddRangeElement (PersistentElementRefP elemRef, DgnElementCR el)
    {
    DRange3dCP range = elemRef->CheckIndexRangeOfElement(el);
    if (NULL == range)
        return;

    if (NULL == m_rangeTree)
        return;

    if (isValidRange(*range, el.Is3d()))
        m_rangeTree->_AddElement(elemRef, *range, m_stamp);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt ElemRangeIndex::RemoveElement (PersistentElementRefP elemRef)
    {
    DRange3dCP range = elemRef->CheckIndexRange();
    if (NULL == range)
        return SUCCESS;

    return (NULL == m_rangeTree) ? SUCCESS : m_rangeTree->_RemoveElement(elemRef, *range, m_stamp);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
ElemRangeIndex::ElemRangeIndex (DgnModelR dgnModel) : m_dgnModel (dgnModel)
    {
    m_rangeTree = NULL;
    m_stamp = 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
ElemRangeIndex::~ElemRangeIndex()
    {
    DELETE_AND_CLEAR (m_rangeTree);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt  ElemRangeIndex::GetRangeIfKnown (DRange3dR range)
    {
    DRange3dCP modelRange;
    if (NULL == m_rangeTree || (NULL == (modelRange = m_rangeTree->_GetDgnModelRange())))
        return  ERROR;

    range = *modelRange;
    if (!m_dgnModel.Is3d())
        range.low.z = range.high.z = 0.0;

    return  SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* The caller KNOWS that this is a graphics element and that it is not dynamic range.
* @bsimethod                                    Sam.Wilson                      04/2008
+---------------+---------------+---------------+---------------+---------------+------*/
DRange3dCP ElemRangeIndex::GetIndexRange (ElementHandleCR eh)
    {
    ElementRefP ref = eh.GetElementRef();
    if (NULL != ref && ELEMENT_REF_TYPE_Persistent == ref->GetRefType() && eh.IsPersistent())
        {
        return &((PersistentElementRefP)ref)->GetIndexRange ();
        }

    return &eh.GetElementCP()->GetRange();
    }

/*---------------------------------------------------------------------------------**//**
* The caller does not know if this is a graphics element or if it is dynamic range.
* @bsimethod                                    Sam.Wilson                      04/2008
+---------------+---------------+---------------+---------------+---------------+------*/
DRange3dCP ElemRangeIndex::CheckIndexRange (ElementHandleCR eh)
    {
    DgnElementCP el = eh.GetElementCP();
    if (NULL == el)
        return NULL;

    ElementRefP ref = eh.GetElementRef();
    if (NULL != ref && ELEMENT_REF_TYPE_Persistent == ref->GetRefType() && eh.IsPersistent())
        {
        return ((PersistentElementRefP)ref)->CheckIndexRangeOfElement (*el);
        }

    return &el->GetRange();
    }
