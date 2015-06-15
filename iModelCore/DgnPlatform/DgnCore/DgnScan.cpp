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
* @bsimethod                                                    KeithBentley    08/01
+---------------+---------------+---------------+---------------+---------------+------*/
bool ScanCriteria::UseRangeTree(DgnRangeTree& rangeTree)
    {
    if (nullptr != m_appRangeNodeCheck)
        return  true;

    DRange3dCP modelRange = rangeTree.GetFullRange();
    return (nullptr != modelRange);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BJB                             11/89
+---------------+---------------+---------------+---------------+---------------+------*/
static ScanCriteria::Result checkSubRange (DRange3dCR scanRange, DRange3dCR elemRange, bool isElem3d)
    {
    /* if element xlow is greater than range xhigh, reject */
    if (elemRange.low.x > scanRange.high.x)
        return ScanCriteria::Result::Fail;

    /* if element ylow is greater than range yhigh, reject */
    if (elemRange.low.y > scanRange.high.y)
        return ScanCriteria::Result::Fail;

    /* if element xhigh is less than range xlow, reject */
    if (elemRange.high.x < scanRange.low.x)
        return ScanCriteria::Result::Fail;

    /* if element yhigh is less than range ylow, reject */
    if (elemRange.high.y < scanRange.low.y)
        return ScanCriteria::Result::Fail;

    /* if element zlow is greater than range zhigh, reject */
    if ((isElem3d ? elemRange.low.z : 0) > scanRange.high.z)
        return ScanCriteria::Result::Fail;

    /* if element zhigh is less than range zlow, reject */
    if ((isElem3d ? elemRange.high.z : 0) < scanRange.low.z)
        return ScanCriteria::Result::Fail;

    /* passed all tests, so accept it */
    return  ScanCriteria::Result::Pass;
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
static ScanCriteria::Result checkSkewRange (DRange3dCP skewRange, DPoint3dCP skewVector, DRange3dCR elemRange, bool isElem3d)
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
        return ScanCriteria::Result::Fail;

    /* Check the projection of the element's xlow to the plane where
       yhigh of the element is equal to ylow of the skewrange */
    vb1 = dlo.y * skVector.x;
    va2 = dhi.x * skVector.y;
    if (va2 < vb1)
        return ScanCriteria::Result::Fail;

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
            return ScanCriteria::Result::Fail;
        }
    else
        {
        vb3 = dlo.y * skVector.z;
        vc4 = dhi.z * skVector.y;
        if (vb3 > vc4)
            return ScanCriteria::Result::Fail;
        }

    /* project onto the other plane */
    if (va2 < vb2)
        {
        va4 = dhi.x * skVector.z;
        vc1 = dlo.z * skVector.x;
        if (va4 < vc1)
            return ScanCriteria::Result::Fail;
        }
    else
        {
        vb4 = dhi.y * skVector.z;
        vc3 = dlo.z * skVector.y;
        if (vb4 < vc3)
            return ScanCriteria::Result::Fail;
        }

    return ScanCriteria::Result::Pass;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BJB                             11/89
+---------------+---------------+---------------+---------------+---------------+------*/
ScanCriteria::Result  ScanCriteria::CheckRange(DRange3dCR elemRange, bool is3d) const
    {
    if (m_type.testSkewScan)
        {
        if (checkSubRange(m_range[0], elemRange, is3d) == ScanCriteria::Result::Fail)
            return ScanCriteria::Result::Fail;

        if (checkSubRange(m_range[1], elemRange, is3d) == ScanCriteria::Result::Pass)
            return ScanCriteria::Result::Pass;

        return checkSkewRange(&m_skewRange, &m_skewVector, elemRange, is3d);
        }

    if (m_type.testRange)
        return checkSubRange (m_range[0], elemRange, is3d);

    return ScanCriteria::Result::Pass;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool ScanCriteria::CheckElementRange(DgnElementCR element) const
    {
    GeometricElementCP geom = element.ToGeometricElement();
    return geom ? _CheckRangeTreeNode(geom->CalculateRange3d(), geom->Is3d()) : false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      01/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool ScanCriteria::_CheckRangeTreeNode(DRange3dCR nodeRange, bool is3d) const
    {
    if (ScanCriteria::Result::Pass != CheckRange(nodeRange, is3d))
        return false;

    return (nullptr == m_appRangeNodeCheck) ? true : (ScanCriteria::Result::Pass == m_appRangeNodeCheck->_CheckNodeRange(*this, nodeRange, is3d));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
ScanCriteria::Result ScanCriteria::CheckElement(DgnElementCR element, bool doRangeTest) const
    {
    if (m_type.testCategory)
        {
        if (!m_categories->Contains(element.GetCategoryId()))
            return  ScanCriteria::Result::Fail;
        }

    /* check the range */
    if (doRangeTest)
        {
        if (!CheckElementRange(element))
            return  ScanCriteria::Result::Fail;
        }

    return ScanCriteria::Result::Pass;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/07
+---------------+---------------+---------------+---------------+---------------+------*/
DgnRangeTree::Match ScanCriteria::_VisitRangeTreeElem(GeometricElementCP element, DRange3dCR range)
    {
    if (ScanCriteria::Result::Pass != CheckRange(range, element->Is3d()))
        return DgnRangeTree::Match::Ok;

    if (ScanCriteria::Result::Fail == CheckElement(*element, false))
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
        if (ScanCriteria::Result::Pass != CheckElement(el, true))
            continue;

        if (SUCCESS != m_callbackFunc(el, m_callbackArg, *this))
            break;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt ScanCriteria::SetDgnModel (DgnModelP model)
    {
    if (NULL == (m_model = model))
        return ERROR;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    08/00
+---------------+---------------+---------------+---------------+---------------+------*/
void ScanCriteria::SetElementCallback (PFScanElementCallback callbackFunc, CallbackArgP callbackArg)
    {
    m_callbackArg  = callbackArg;
    m_callbackFunc = callbackFunc;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/07
+---------------+---------------+---------------+---------------+---------------+------*/
void ScanCriteria::SetRangeTest (DRange3dP srP)
    {
    m_type.testSkewScan = false;
    if (NULL != srP)
        {
        m_range[0] = *srP;
        m_type.testRange = 1;
        }
    else
        {
        m_type.testRange = 0;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/07
+---------------+---------------+---------------+---------------+---------------+------*/
void ScanCriteria::SetSkewRangeTest (DRange3dP mainRange, DRange3dP skewRange, DPoint3dP skewVector)
    {
    if (NULL != mainRange)
        SetRangeTest (mainRange);

    if ( (NULL != skewRange) && (NULL != skewVector) )
        {
        m_skewRange = *skewRange;
        m_range[1] = *skewRange;
        m_skewVector = *skewVector;
        m_type.testSkewScan = 1;
        m_numRanges = 1;
        }
    else
        {
        m_type.testSkewScan = 0;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    08/00
+---------------+---------------+---------------+---------------+---------------+------*/
void ScanCriteria::SetCategoryTest(DgnCategoryIdSet const& categories)
    {
    m_categories = &categories;
    m_type.testCategory = 1;
    }
