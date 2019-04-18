/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

using namespace RangeIndex;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BJB                             11/89
+---------------+---------------+---------------+---------------+---------------+------*/
static ScanCriteria::Reject checkSubRange(FBoxCR scanRange, FBoxCR elemRange, bool isElem3d)
    {
    /* if element xlow is greater than range xhigh, reject */
    if (elemRange.m_low.x > scanRange.m_high.x)
        return ScanCriteria::Reject::Yes;

    /* if element ylow is greater than range yhigh, reject */
    if (elemRange.m_low.y > scanRange.m_high.y)
        return ScanCriteria::Reject::Yes;

    /* if element xhigh is less than range xlow, reject */
    if (elemRange.m_high.x < scanRange.m_low.x)
        return ScanCriteria::Reject::Yes;

    /* if element yhigh is less than range ylow, reject */
    if (elemRange.m_high.y < scanRange.m_low.y)
        return ScanCriteria::Reject::Yes;

    /* if element zlow is greater than range zhigh, reject */
    if ((isElem3d ? elemRange.m_low.z : 0) > scanRange.m_high.z)
        return ScanCriteria::Reject::Yes;

    /* if element zhigh is less than range zlow, reject */
    if ((isElem3d ? elemRange.m_high.z : 0) < scanRange.m_low.z)
        return ScanCriteria::Reject::Yes;

    /* passed all tests, so accept it */
    return  ScanCriteria::Reject::No;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BJB             12/89
+---------------+---------------+---------------+---------------+---------------+------*/
static inline void exchangeAndNegate(double *dbl1, double *dbl2)
    {
    double temp = *dbl1;
    *dbl1 = - *dbl2;
    *dbl2 = - temp;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BJB             12/89
+---------------+---------------+---------------+---------------+---------------+------*/
static ScanCriteria::Reject checkSkewRange(FBoxCP skewRange, DPoint3dCP skewVector, FBoxCR elemRange, bool isElem3d)
    {
    double va1, va2, va3, va4;
    double vb1, vb2, vb3, vb4;
    double vc1, vc2, vc3, vc4;

    /* we must make a local copy of the skewVector */
    DPoint3d skVector = *skewVector;
    DPoint3d dlo;
    DPoint3d dhi;

    dlo.x = (double) elemRange.m_low.x  - skewRange->m_high.x;
    dlo.y = (double) elemRange.m_low.y  - skewRange->m_high.y;
    dhi.x = (double) elemRange.m_high.x - skewRange->m_low.x;
    dhi.y = (double) elemRange.m_high.y - skewRange->m_low.y;

    if (skVector.x < 0.0)
        {
        skVector.x = - skVector.x;
        exchangeAndNegate(&dlo.x, &dhi.x);
        }

    if (skVector.y < 0.0)
        {
        skVector.y = - skVector.y;
        exchangeAndNegate(&dlo.y, &dhi.y);
        }

    /* Check the projection of the element's xhigh to the plane where ylow of the element is equal to yhigh of the skewrange */
    va1 = dlo.x * skVector.y;
    vb2 = dhi.y * skVector.x;
    if (va1 > vb2)
        return ScanCriteria::Reject::Yes;

    /* Check the projection of the element's xlow to the plane where yhigh of the element is equal to ylow of the skewrange */
    vb1 = dlo.y * skVector.x;
    va2 = dhi.x * skVector.y;
    if (va2 < vb1)
        return ScanCriteria::Reject::Yes;

    /* now we need the Z stuff */
    dlo.z = (isElem3d ? (double) elemRange.m_low.z  : 0.0) - skewRange->m_high.z;
    dhi.z = (isElem3d ? (double) elemRange.m_high.z : 0.0) - skewRange->m_low.z;
    if (skVector.z < 0.0)
        {
        skVector.z = - skVector.z;
        exchangeAndNegate(&dlo.z, &dhi.z);
        }

    /* project onto either the xz or yz plane */
    if (va1 > vb1)
        {
        va3 = dlo.x * skVector.z;
        vc2 = dhi.z * skVector.x;
        if (va3 > vc2)
            return ScanCriteria::Reject::Yes;
        }
    else
        {
        vb3 = dlo.y * skVector.z;
        vc4 = dhi.z * skVector.y;
        if (vb3 > vc4)
            return ScanCriteria::Reject::Yes;
        }

    /* project onto the other plane */
    if (va2 < vb2)
        {
        va4 = dhi.x * skVector.z;
        vc1 = dlo.z * skVector.x;
        if (va4 < vc1)
            return ScanCriteria::Reject::Yes;
        }
    else
        {
        vb4 = dhi.y * skVector.z;
        vc3 = dlo.z * skVector.y;
        if (vb4 < vc3)
            return ScanCriteria::Reject::Yes;
        }

    return ScanCriteria::Reject::No;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BJB                             11/89
+---------------+---------------+---------------+---------------+---------------+------*/
ScanCriteria::Reject ScanCriteria::CheckRange(FBoxCR elemRange) const
    {
    if (m_testSkewScan)
        {
        if (checkSubRange(m_range, elemRange, m_is3d) == ScanCriteria::Reject::Yes)
            return ScanCriteria::Reject::Yes;

        if (checkSubRange(m_skewRange, elemRange, m_is3d) == ScanCriteria::Reject::No)
            return ScanCriteria::Reject::No;

        return checkSkewRange(&m_skewRange, &m_skewVector, elemRange, m_is3d);
        }

    return checkSubRange(m_range, elemRange, m_is3d);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
ScanCriteria::Reject ScanCriteria::CheckElementRange(RangeIndex::EntryCR element) const
    {
    return Accept::Yes == _CheckRangeTreeNode(element.m_range, m_is3d) ? Reject::No : Reject::Yes;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
ScanCriteria::Reject ScanCriteria::CheckElement(RangeIndex::EntryCR element, bool doRangeTest) const
    {
    if (m_testCategory)
        {
        if (!m_categories->Contains(element.m_category))
            return  ScanCriteria::Reject::Yes;
        }

    return doRangeTest ? CheckElementRange(element) : ScanCriteria::Reject::No;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/07
+---------------+---------------+---------------+---------------+---------------+------*/
Traverser::Stop ScanCriteria::_VisitRangeTreeEntry(RangeIndex::EntryCR entry)
    {
    if (ScanCriteria::Reject::No != CheckRange(entry.m_range))
        return Stop::No;

    if (ScanCriteria::Reject::Yes == CheckElement(entry, false))
        return Stop::No;

    return _OnRangeElementFound(entry.m_id);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    KeithBentley                    2/93
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt ScanCriteria::Scan()
    {
    BeAssert(m_model != nullptr);

    m_model->FillRangeIndex();

    auto rangeIndex = m_model->GetRangeIndex();
    if (nullptr == rangeIndex)
        {
        BeAssert(false);
        return ERROR;
        }

    rangeIndex->Traverse(*this);
    return SUCCESS;
    }
