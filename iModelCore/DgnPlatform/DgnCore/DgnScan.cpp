/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DgnScan.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/07
+---------------+---------------+---------------+---------------+---------------+------*/
ScanCriteria::ScanCriteria() 
    {
    // do this so we don't lose our vtable
    memset (&m_firstMember, 0, offsetof (ScanCriteria, m_lastMember) - offsetof (ScanCriteria, m_firstMember));

    for (int iRange = 0; iRange < MAX_SC_RANGE; iRange++)
        {
        m_range[iRange].low.x = 1000.; // set low.x > high.x (invalid range) so we can skip if never set up
        m_range[iRange].high.x = 0.0;
        }
    }

/*---------------------------------------------------------------------------------**//**
* copy CTOR
* @bsimethod                                    Keith.Bentley                   06/07
+---------------+---------------+---------------+---------------+---------------+------*/
ScanCriteria::ScanCriteria(ScanCriteriaCR from) 
    {
    memcpy (&m_firstMember, &from.m_firstMember, offsetof (ScanCriteria, m_lastMember)- offsetof (ScanCriteria, m_firstMember));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    08/01
+---------------+---------------+---------------+---------------+---------------+------*/
static double rangeVolume (DRange3dCR  range, bool threeD)
    {
    double x = (double) (range.high.x - range.low.x);
    double y = (double) (range.high.y - range.low.y);
    double z = threeD ? (double) (range.high.z - range.low.z) : 1.0;

    if (z == 0.0)
        z = 1.0;

    return  (x * y * z);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    08/01
+---------------+---------------+---------------+---------------+---------------+------*/
static void intersectRanges (DRange3dR intsct, DRange3dCR range1, DRange3dCR range2, bool threeD)
    {
    intsct.low.x  = range1.low.x  > range2.low.x  ? range1.low.x  : range2.low.x;
    intsct.low.y  = range1.low.y  > range2.low.y  ? range1.low.y  : range2.low.y;
    intsct.high.x = range1.high.x < range2.high.x ? range1.high.x : range2.high.x;
    intsct.high.y = range1.high.y < range2.high.y ? range1.high.y : range2.high.y;

    // check for the no intersection case
    if (intsct.high.x < intsct.low.x)
        intsct.high.x = intsct.low.x;

    if (intsct.high.y < intsct.low.y)
        intsct.high.y = intsct.low.y;

    if (threeD)
        {
        intsct.low.z  = range1.low.z  > range2.low.z  ? range1.low.z  : range2.low.z;
        intsct.high.z = range1.high.z < range2.high.z ? range1.high.z : range2.high.z;

        if (intsct.high.z < intsct.low.z)
            intsct.high.z = intsct.low.z;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    08/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool ScanCriteria::UseRangeTree(DgnRangeTree& rangeTree)
    {
    if (nullptr != m_appRangeNodeCheck)
        return  true;

    DRange3dCP modelRange = rangeTree.GetFullRange();
    if (nullptr == modelRange)
        return  false;

    // for skewed scans, use small range.
    DRange3dCP testRange = &m_range[m_type.testSkewScan ? 1 : 0];

    DRange3d intersectRange;
    bool is3d = m_model->Is3d();
    intersectRanges (intersectRange, *testRange, *modelRange, is3d);

    // use range tree if volume of intersection is less than half volume of whole tree
    return ((2.* rangeVolume (intersectRange, is3d)) <= rangeVolume (*modelRange, is3d));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BJB                             11/89
+---------------+---------------+---------------+---------------+---------------+------*/
static ScanTestResult checkSubRange (DRange3dCR scanRange, DRange3dCR elemRange, bool isElem3d)
    {
    /* if element xlow is greater than range xhigh, reject */
    if (elemRange.low.x > scanRange.high.x)
        return ScanTestResult::Fail;

    /* if element ylow is greater than range yhigh, reject */
    if (elemRange.low.y > scanRange.high.y)
        return ScanTestResult::Fail;

    /* if element xhigh is less than range xlow, reject */
    if (elemRange.high.x < scanRange.low.x)
        return ScanTestResult::Fail;

    /* if element yhigh is less than range ylow, reject */
    if (elemRange.high.y < scanRange.low.y)
        return ScanTestResult::Fail;

    /* if element zlow is greater than range zhigh, reject */
    if ((isElem3d ? elemRange.low.z : 0) > scanRange.high.z)
        return ScanTestResult::Fail;

    /* if element zhigh is less than range zlow, reject */
    if ((isElem3d ? elemRange.high.z : 0) < scanRange.low.z)
        return ScanTestResult::Fail;

    /* passed all tests, so accept it */
    return  ScanTestResult::Pass;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BJB             12/89
+---------------+---------------+---------------+---------------+---------------+------*/
static inline void exchangeAndNegate (double *dbl1, double *dbl2)
    {
    double temp = *dbl1;
    *dbl1 = - *dbl2;
    *dbl2 = - temp;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BJB             12/89
+---------------+---------------+---------------+---------------+---------------+------*/
static ScanTestResult checkSkewRange (DRange3dCP skewRange, DPoint3dCP skewVector, DRange3dCR elemRange, bool isElem3d)
    {
    double va1, va2, va3, va4;
    double vb1, vb2, vb3, vb4;
    double vc1, vc2, vc3, vc4;

    /* we must make a local copy of the skewVector */
    DPoint3d skVector = *skewVector;
    DPoint3d dlo;
    DPoint3d dhi;

    dlo.x = (double) elemRange.low.x  - skewRange->high.x;
    dlo.y = (double) elemRange.low.y  - skewRange->high.y;
    dhi.x = (double) elemRange.high.x - skewRange->low.x;
    dhi.y = (double) elemRange.high.y - skewRange->low.y;

    if (skVector.x < 0.0)
        {
        skVector.x = - skVector.x;
        exchangeAndNegate (&dlo.x, &dhi.x);
        }

    if (skVector.y < 0.0)
        {
        skVector.y = - skVector.y;
        exchangeAndNegate (&dlo.y, &dhi.y);
        }

    /* Check the projection of the element's xhigh to the plane where
       ylow of the element is equal to yhigh of the skewrange */
    va1 = dlo.x * skVector.y;
    vb2 = dhi.y * skVector.x;
    if (va1 > vb2)
        return ScanTestResult::Fail;

    /* Check the projection of the element's xlow to the plane where
       yhigh of the element is equal to ylow of the skewrange */
    vb1 = dlo.y * skVector.x;
    va2 = dhi.x * skVector.y;
    if (va2 < vb1)
        return ScanTestResult::Fail;

    /* now we need the Z stuff */
    dlo.z = (isElem3d ? (double) elemRange.low.z  : 0.0) - skewRange->high.z;
    dhi.z = (isElem3d ? (double) elemRange.high.z : 0.0) - skewRange->low.z;
    if (skVector.z < 0.0)
        {
        skVector.z = - skVector.z;
        exchangeAndNegate (&dlo.z, &dhi.z);
        }

    /* project onto either the xz or yz plane */
    if (va1 > vb1)
        {
        va3 = dlo.x * skVector.z;
        vc2 = dhi.z * skVector.x;
        if (va3 > vc2)
            return ScanTestResult::Fail;
        }
    else
        {
        vb3 = dlo.y * skVector.z;
        vc4 = dhi.z * skVector.y;
        if (vb3 > vc4)
            return ScanTestResult::Fail;
        }

    /* project onto the other plane */
    if (va2 < vb2)
        {
        va4 = dhi.x * skVector.z;
        vc1 = dlo.z * skVector.x;
        if (va4 < vc1)
            return ScanTestResult::Fail;
        }
    else
        {
        vb4 = dhi.y * skVector.z;
        vc3 = dlo.z * skVector.y;
        if (vb4 < vc3)
            return ScanTestResult::Fail;
        }

    return ScanTestResult::Pass;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BJB                             11/89
+---------------+---------------+---------------+---------------+---------------+------*/
ScanTestResult  ScanCriteria::CheckRange(DRange3dCR elemRange, bool is3d) const
    {
    if (m_type.testSkewScan)
        {
        if (checkSubRange(m_range[0], elemRange, is3d) == ScanTestResult::Fail)
            return ScanTestResult::Fail;

        if (checkSubRange(m_range[1], elemRange, is3d) == ScanTestResult::Pass)
            return ScanTestResult::Pass;

        return checkSkewRange(&m_skewRange, &m_skewVector, elemRange, is3d);
        }

    if (m_type.testRange)
        return checkSubRange (m_range[0], elemRange, is3d);

    return ScanTestResult::Pass;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool ScanCriteria::CheckElementRange(DgnElementCR element) const
    {
    GeometricElementCP geom = element.ToGeometricElement();
    return geom ? _CheckRangeTreeNode(geom->_GetRange3d(), geom->Is3d()) : false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      01/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool ScanCriteria::_CheckRangeTreeNode(DRange3dCR nodeRange, bool is3d) const
    {
    if (ScanTestResult::Pass != CheckRange(nodeRange, is3d))
        return false;

    return (nullptr == m_appRangeNodeCheck) ? true : (ScanTestResult::Pass == m_appRangeNodeCheck->_CheckNodeRange(*this, nodeRange, is3d));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
ScanTestResult ScanCriteria::CheckElement(DgnElementCR element, bool doRangeTest) const
    {
    if (m_type.testCategory)
        {
        if (!m_categories->Contains(element.GetCategoryId()))
            return  ScanTestResult::Fail;
        }

    /* check the range */
    if (doRangeTest)
        {
        if (!CheckElementRange(element))
            return  ScanTestResult::Fail;
        }

    return ScanTestResult::Pass;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/07
+---------------+---------------+---------------+---------------+---------------+------*/
DgnRangeTree::Match ScanCriteria::_VisitRangeTreeElem(GeometricElementCP element, DRange3dCR range)
    {
    if (ScanTestResult::Pass != CheckRange(range, element->Is3d()))
        return DgnRangeTree::Match::Ok;

    if (ScanTestResult::Fail == CheckElement(*element, false))
        return DgnRangeTree::Match::Ok;

    return (SUCCESS == m_callbackFunc(*element, m_callbackArg, *this)) ? DgnRangeTree::Match::Ok : DgnRangeTree::Match::Aborted;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    KeithBentley                    2/93
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt ScanCriteria::Scan(ViewContextP context)
    {
    if (nullptr == m_model)
        return ERROR;

    m_viewContext = context;
    DgnRangeTreeP rangeIndex;
    if (m_type.testRange && (nullptr != (rangeIndex = m_model->GetRangeIndexP(true))) && UseRangeTree(*rangeIndex))
        {
        rangeIndex->FindMatches(*this);
        return SUCCESS;
        }

    for (auto curr : *m_model)
        {
        DgnElementCR el = *curr.second.get();
        if (ScanTestResult::Pass != CheckElement(el, true))
            continue;

        if (SUCCESS != m_callbackFunc(el, m_callbackArg, *this))
            break;
        }

    return SUCCESS;
    }
