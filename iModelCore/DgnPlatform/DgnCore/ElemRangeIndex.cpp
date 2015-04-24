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
    DRange3dCR range = el._GetRange3d();

    if (isValidRange(range, el.Is3d()))
        m_rangeTree.AddElement(DRTEntry(range, el), m_stamp);
    }
