/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/ElemRangeIndex.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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
void ElemRangeIndex::AddRangeElement(GeometricElementCR el)
    {
    if (NULL == m_rangeTree)
        return;

    DRange3dCR range = el._GetRange3d();

    if (isValidRange(range, el.Is3d()))
        m_rangeTree->_AddElement(el, range, m_stamp);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt ElemRangeIndex::RemoveElement (GeometricElementCR element, DRange3d oldRange)
    {
    return (NULL == m_rangeTree) ? SUCCESS : m_rangeTree->_RemoveElement(element, oldRange, m_stamp);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
ElemRangeIndex::ElemRangeIndex (DgnModelCR dgnModel) : m_dgnModel (dgnModel)
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
