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
StatusInt ScanCriteria::CallElementFunc(DgnElementCP el)
    {
    PFScanElementCallback cbFunc = (PFScanElementCallback) m_callbackFunc;
    return cbFunc (*el, m_callbackArg, *this);
    }

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

    //    Pointer fields in the ScanCriteria structure must be reallocated so that the
    //    source and destination are not pointing at the same memory.
    m_rangeHits    = NULL;

    // Context fields are not preserved in clone
    ResetState();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/07
+---------------+---------------+---------------+---------------+---------------+------*/
void        ScanCriteria::ResetState()
    {
    m_elemOk               = 0;

    DELETE_AND_CLEAR (m_rangeHits);
    m_currRangeHit         = 0;
    }

#define SCElemFilter_RELEASE_AND_CLEAR(ptr) {if(ptr){(ptr)->Release(this);ptr=NULL;}}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    08/00
+---------------+---------------+---------------+---------------+---------------+------*/
void        ScanCriteria::Empty()
    {
    ResetState();
    SCElemFilter_RELEASE_AND_CLEAR (m_elemfilter);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/07
+---------------+---------------+---------------+---------------+---------------+------*/
ScanCriteria::~ScanCriteria()
    {
    Empty();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/2008
+---------------+---------------+---------------+---------------+---------------+------*/
void    ScanCriteria::SetFilter (ElemFilter *f)
    {
    if (NULL != m_elemfilter)
        m_elemfilter->Release (this);

    m_elemfilter = f;
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
    if (NULL != m_appRangeNodeCheck)
        return  true;

    DRange3dCP modelRange = rangeTree.GetFullRange();
    if (NULL == modelRange)
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
* @bsimethod                                                    KeithBentley    08/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt ScanCriteria::DoElementCallback ()
    {
    DgnElementCP cacheElm = m_iterator.GetCurrentDgnElement();
    return CallElementFunc (cacheElm);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BJB                             10/89
+---------------+---------------+---------------+---------------+---------------+------*/
bool ScanCriteria::TransferElement (int* scanStatus)
    {
    if (2 == m_type.iteration)
        {
        if (SUCCESS != DoElementCallback ())
            *scanStatus = BUFF_FULL;

        return true;
        }

    return true;
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
    double temp;
    temp = *dbl1;
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
ScanTestResult  ScanCriteria::CheckRange (DRange3dCR elemRange, bool isElem3d) const
    {
    if (m_type.testSkewScan)
        {
        if (checkSubRange (m_range[0], elemRange, isElem3d) == ScanTestResult::Fail)
            return ScanTestResult::Fail;

        if (checkSubRange (m_range[1], elemRange, isElem3d) == ScanTestResult::Pass)
            return ScanTestResult::Pass;

        return checkSkewRange (&m_skewRange, &m_skewVector, elemRange, isElem3d);
        }

    if (m_type.testRange)
        return checkSubRange (m_range[0], elemRange, isElem3d);

    return ScanTestResult::Pass;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool ScanCriteria::CheckElementRange(DgnElementCR element) const
    {
    GeometricElementCP geom = element.ToGeometricElement();
    return geom ? _CheckRangeTreeNode(geom->_GetRange3d(), element.GetDgnModel().Is3d(), true) : false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      01/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool ScanCriteria::_CheckRangeTreeNode(DRange3dCR nodeRange, bool is3d, bool isElement) const
    {
    if (ScanTestResult::Pass != CheckRange (nodeRange, is3d))
        return false;

    return (NULL == m_appRangeNodeCheck) ? true : (ScanTestResult::Pass == m_appRangeNodeCheck->_CheckNodeRange(*this, nodeRange, is3d, isElement));
    }

/*---------------------------------------------------------------------------------**//**
* @return false if the element is OK, true if it is rejected
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

    // if user has set up a scanFilter callback function, call it.
    return m_elemfilter ? m_elemfilter->_FilterElement(this, &element) : ScanTestResult::Pass;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt ScanCriteria::ProcessElemRefRangeList()
    {
    for (T_RangeHits::iterator curr = m_rangeHits->begin(); curr < m_rangeHits->end(); curr++)
        {
        if (SUCCESS != CallElementFunc (*curr))
            break;
        }

    ResetState();
    return  END_OF_DGN;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BJB                             12/89
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt ScanCriteria::ProcessRangeIndexResults ()
    {
    if (IsElemRefIter())
        return  ProcessElemRefRangeList ();

    int  scanStatus = 0;

    for (T_RangeHits::iterator curr = m_rangeHits->begin() + m_currRangeHit; curr < m_rangeHits->end(); curr++, m_currRangeHit++)
        {
        /* ---------------------------------------------------------------
           If we have an element already qualified, then we have
           already set up our pointers and we just continue from where we
           left off.  If we don't, we go to the next element in the list.
           --------------------------------------------------------------- */
        if (!m_elemOk)
            {
            GeometricElementCP  tstElem = *curr;

            /* ------------------------------------------------------------
               The first step is to get a pointer to the element we are
               validating.  If the element is in the cache the PtrPair kept
               by the range tree specifies that address.  If the PtrPair gives
               us a NULL pointer, we know we have to read it from the file
               into our fileBuffer.
               ------------------------------------------------------------ */
            if (tstElem)
                {
                m_iterator.SetCurrentElm(tstElem);
                }
            }

        for (;;)
            {
            DgnElementCP  currElRef = m_iterator.GetCurrentDgnElement();

            /* if we encounter EOF marker, stop scan */
            if (NULL == currElRef)
                {
                scanStatus = END_OF_DGN;
                break;
                }

            bool            goToNextElement = true;
            ElementHandle   currElm (currElRef);   // to lock element data in memory

            if (!scanStatus)
                {
                /* indicate element OK in case it will not fit */
                m_elemOk = true;

                /* --------------------------------------------------------
                   Saving an element that is not a complex header or a complex
                   header in unNest mode, or a complex header when data only
                   (no pointers) is being returned.
                   -------------------------------------------------------- */
                goToNextElement = TransferElement (&scanStatus);

                if (!scanStatus)
                    {
                    if (goToNextElement)
                        m_iterator.GetNextDgnElement(true);

                    /* we have stored the element we checked */
                    m_elemOk = false;

                    /* are we returning only one element */
                    if (m_type.returnOneElem)
                        {
                        /* ------------------------------------------------
                           If we are not in a complex, set it up so next
                           time we will go to the next element in the list.
                           If we are in a complex, adjust the current test
                           element pointer to get us back into the complex.
                           ------------------------------------------------ */
                        m_currRangeHit++;

                        scanStatus = BUFF_FULL;
                        break;
                        }
                    }
                }
            }

        /* ----------------------------------------------------------------
           If scanStatus is still 0, go back and get the next element to
           test from the range tree list. Otherwise, break out.
           ---------------------------------------------------------------- */
        if (scanStatus)
            break;
        }

    /* if we detected EOF or if we got through the whole list */
    if (!scanStatus)
        {
        scanStatus = END_OF_DGN;
        ResetState();
        }

    return scanStatus;
    }

typedef int (*PFScanElementNodeCallback) (GeometricElementCP, void* callbackArg, ScanCriteriaP);
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/07
+---------------+---------------+---------------+---------------+---------------+------*/
DgnRangeTree::Match ScanCriteria::_VisitRangeTreeElem(GeometricElementCP element)
    {
    if (!CheckElementRange (*element))
        return DgnRangeTree::Match::Ok;

    if (ScanTestResult::Fail == CheckElement (*element, false))
        return DgnRangeTree::Match::Ok;

    if (UseRangeTreeOrdering())
        {
        PFScanElementNodeCallback   cbFunc = (PFScanElementNodeCallback) m_callbackFunc;
        return (SUCCESS == cbFunc (element, m_callbackArg, this)) ? DgnRangeTree::Match::Ok : DgnRangeTree::Match::Aborted;
        }

    if (m_rangeHits->size() > MAX_ORDERED_HITS)
        return  DgnRangeTree::Match::TooManyHits;

    m_rangeHits->push_back(element);
    return  DgnRangeTree::Match::Ok;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/07
+---------------+---------------+---------------+---------------+---------------+------*/
DgnRangeTree::Match ScanCriteria::FindRangeHits(DgnRangeTree* rangeIndex)
    {
    if (NULL == m_rangeHits)
        m_rangeHits = new T_RangeHits;
    else
        m_rangeHits->clear();

    m_currRangeHit = 0;
    DgnRangeTree::Match stat = rangeIndex->FindMatches (*this);
    if (DgnRangeTree::Match::Ok != stat)
        {
        ResetState(); // aborted - throw away hits
        return stat;
        }

    return DgnRangeTree::Match::Ok;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    KeithBentley                    2/93
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt ScanCriteria::Scan ()
    {
    return Scan (NULL);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    KeithBentley                    2/93
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt ScanCriteria::Scan (ViewContextP context)
    {
    int scanStatus;            /* scan status */

    if (NULL == m_model)
        return ERROR;

    bool wasNewCriteria = TO_BOOL (m_newCriteria);
    if (wasNewCriteria)
        {
        // clear a bunch of context stuff
        m_iterator.SetModel (m_model);
        m_iterator.GetFirstDgnElement(); 
        }

    m_viewContext = context;
    m_newCriteria = false;  // not a new scan anymore

    /* init input and output pointers, output buffer end, words returned */
    scanStatus = 0;

    DgnRangeTreeP rangeIndex;
    if (m_type.testRange && (NULL != (rangeIndex = m_model->GetRangeIndexP(true))))
        {
        if (!UseRangeTreeOrdering() && !UseRangeTree(*rangeIndex))
            goto linearScan;

        if (wasNewCriteria)
            {
            switch (FindRangeHits (rangeIndex))
                {
                case DgnRangeTree::Match::TooManyHits:
                    goto linearScan;

                case DgnRangeTree::Match::Aborted:
                    return  BUFF_FULL;
                }
            }

        if (NULL == m_rangeHits || m_rangeHits->empty())
            {
            return END_OF_DGN;
            }

        return  ProcessRangeIndexResults ();
        }

linearScan:

    /* start the scan */
    for (;;)
        {
        DgnElementCP  currElRef = m_iterator.GetCurrentDgnElement();

        /* -----------------------------------------------------------
           We now have enough of the element to accept or reject it.
           Start checking against the scan criteria.
           ----------------------------------------------------------- */
        /* if we encounter EOF marker, stop scan */
        if (NULL == currElRef)
            {
            scanStatus = END_OF_DGN;
            break;
            }

        bool goToNextElement = true;

        /* was element previously accepted but would not fit? */
        if (m_elemOk)
            {
            goto Accept;
            }

        /* if element is deleted, ignore it */
        if (currElRef->IsDeleted())
            {
            goto Reject;
            }

        if (ScanTestResult::Fail == CheckElement (*currElRef, true))
            goto Reject;

Accept:
        if (!scanStatus)
            {
            /* indicate element OK in case it will not fit */
            m_elemOk = true;

            /* if it's a complex header, see if we are forcing return of
               all its component elements */

            /* ------------------------------------------------------------
               Saving an element that is not a complex header or a complex
               header in unNest mode, or a complex header when data only
               (no pointers) is being returned.
               ------------------------------------------------------------ */
            goToNextElement = TransferElement (&scanStatus);
            if (!scanStatus)
                {
                if (goToNextElement)
                    m_iterator.GetNextDgnElement(true);

                /* we have stored the element we checked */
                m_elemOk = false;

                /* are we returning only one element */
                if (m_type.returnOneElem)
                    {
                    scanStatus = BUFF_FULL;
                    break;
                    }

                /* if not, keep scanning */
                continue;
                }
            }

        break;

Reject:
        /* element not OK */
        m_elemOk = false;
        m_iterator.GetNextDgnElement(true);
        }

    return scanStatus;
    }

/*---------------------------------------------------------------------------------**//**
* Set up iterator to walk the elements in this model only.
* @bsimethod                                                    SamWilson       10/00
+---------------+---------------+---------------+---------------+---------------+------*/
ScanCriteria::ElemIterator::ElemIterator (DgnModelP dgnModel)
    {
    m_model = dgnModel;
    m_state = ITERATING_GraphicElms;
    m_iter.Invalidate();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    01/01
+---------------+---------------+---------------+---------------+---------------+------*/
ScanCriteria::ElemIterator::ElemIterator (ScanCriteria::ElemIterator* source) : m_iter (source->m_iter)
    {
    m_model    = source->m_model;
    m_state    = source->m_state;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    10/00
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementCP ScanCriteria::ElemIterator::GetFirstDgnElement (DgnModelP dgnModel, bool wantDeleted)
    {
    m_model = dgnModel;

    DgnElementCP  thisElm;
    m_state = ITERATING_GraphicElms;
    if (NULL != (thisElm = m_iter.GetFirstDgnElement (*m_model, wantDeleted)))
        return thisElm;

    m_state = ITERATING_HitEOF;
    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    10/00
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementCP ScanCriteria::ElemIterator::GetNextDgnElement (bool wantDeleted)
    {
    DgnElementCP el;

    if (HitEOF ())          // this happens if caller keeps calling iterator after EOF has been reached
        return NULL;

    if (NULL != (el = m_iter.GetNextDgnElement (wantDeleted)))
        return  el;

    m_state = ITERATING_HitEOF;
    return  NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/13
+---------------+---------------+---------------+---------------+---------------+------*/
static DgnModel* getMyModel(DgnElementCP el) 
    {
    if (NULL == el)
        return  NULL;

    return &el->GetDgnModel();
    }

/*---------------------------------------------------------------------------------**//**
* set the current element for the iterator
* @bsimethod                                                    KeithBentley    10/00
+---------------+---------------+---------------+---------------+---------------+------*/
bool ScanCriteria::ElemIterator::SetCurrentElm(DgnElementCP toElm)
    {
    DgnModel* toList = getMyModel(toElm);
    if (NULL == toList)
        return  false;

    // quick case where list doesn't change
    DgnElementCP curr=m_iter.GetCurrentDgnElement();
    if ((NULL != curr) && (m_state != ITERATING_HitEOF) && (toList == getMyModel(curr)))
        {
        m_iter.SetCurrentDgnElement(toElm);
        return  true;
        }

    m_state = ITERATING_GraphicElms;
    m_model = toList;

    m_iter.SetCurrentDgnElement (toElm);
    return  true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    05/01
+---------------+---------------+---------------+---------------+---------------+------*/
void ScanCriteria::ElemIterator::SetAtEOF ()
    {
    m_iter.Invalidate();
    m_state = ITERATING_HitEOF;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/2009
+---------------+---------------+---------------+---------------+---------------+------*/
ScanCriteria::ElemIterator& ScanCriteria::ElemIterator::operator++ ()
    {
    GetNextDgnElement ();
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/2009
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementCP ScanCriteria::ElemIterator::operator* () const
    {
    return *m_iter;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/2009
+---------------+---------------+---------------+---------------+---------------+------*/
bool ScanCriteria::ElemIterator::operator!= (ScanCriteria::ElemIterator const& rhs) const
    {
    if (m_model != rhs.m_model)
        return true;

    if (HitEOF() && rhs.HitEOF())   // at EOF == at EOF, regardless of how each iterator got there
        return false;

    return m_iter != rhs.m_iter;
    }

