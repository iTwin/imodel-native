/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DgnScan.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

union ConstPtrUnionV8
{
    UShort const*   wd;
    DgnElementCP     elem;
} ;

/*======================================================================================*/
// Turn off runtime ESP check so we can call __stdcall native functions.
// Also turn off Frame Pointer Omission so we can access saveEsp even if the SP is messed up
// (like when calling a __stdcall function)
/*======================================================================================*/
#if defined (_MSC_VER)
    #pragma runtime_checks( "", off )
    #pragma optimize("t",off)
#endif // defined (_MSC_VER)

    StatusInt ScanCriteria::CallElemRefFunc (PersistentElementRefP el)
        {
        PFScanElemRefCallback   cbFunc = (PFScanElemRefCallback) m_callbackFunc;
        StatusInt status;
#if defined (_M_IX86)
        UInt32      saveEsp;
        _asm mov saveEsp, esp;
#endif
        status = cbFunc (el, m_callbackArg, this);
#if defined (_M_IX86)
        _asm mov esp, dword ptr saveEsp;
#endif
        return  status;
        }

    StatusInt ScanCriteria::CallElemDscrFunc (MSElementDescrP elDscr)
        {
        StatusInt status;
#if defined (_M_IX86)
        UInt32      saveEsp;
        _asm mov saveEsp, esp;
#endif
        status = m_callbackFunc (elDscr, m_callbackArg, this);
#if defined (_M_IX86)
        _asm mov esp, dword ptr saveEsp;
#endif
        return  status;
        }

#if defined (_MSC_VER)
    #pragma optimize("",on)
    #pragma runtime_checks( "", restore )
#endif // defined (_MSC_VER)

/*======================================================================================*/
// Turn optimization and frame pointer checks back on
/*======================================================================================*/

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
    m_cellName     = NULL;
    m_levelBitMask = NULL;
    m_extAttrBuf   = NULL;
    m_rangeHits    = NULL;

    if (NULL != from.m_cellName)
        SetCellNameTest (from.m_cellName);

    if (NULL != from.m_levelBitMask)
        {
        bool owned = from.m_type.freeLevelBitMaskWhenDone; // don't clone if the original doesn't own the bitmask
        SetLevelTest (owned ? BitMask::Clone (*from.m_levelBitMask) : from.m_levelBitMask, owned);
        }

    if (NULL != from.m_extAttrBuf)
        SetAttributeTest (from.m_entity, from.m_occurrence, from.m_extAttrBuf);

    // Context fields are not preserved in clone
    ResetState();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/07
+---------------+---------------+---------------+---------------+---------------+------*/
void        ScanCriteria::ResetState()
    {
    m_inComplex            = 0;
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
    if ((NULL != m_levelBitMask) && m_type.freeLevelBitMaskWhenDone)
        BitMask::FreeAndClear (&m_levelBitMask);

    if (NULL != m_cellName)
        memutil_free (m_cellName);

    if (NULL != m_extAttrBuf)
        memutil_free (m_extAttrBuf);

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
bool ScanCriteria::UseRangeTree (ElemRangeIndexP rangeTree)
    {
    if (m_type.testMultiRange)
        return  false;

    if (NULL != m_appRangeNodeCheck)
        return  true;

    DRange3dCP modelRange = rangeTree->GetRange();
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
StatusInt ScanCriteria::DoElemRefCallback ()
    {
    PersistentElementRefP cacheElm = m_iterator.GetCurrentElementRef();
    return CallElemRefFunc (cacheElm);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    01/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt ScanCriteria::DoElemDscrCallback ()
    {
    ElementRefP      cacheElm = m_iterator.GetCurrentElementRef();
    MSElementDescrPtr pDscr = cacheElm->GetElementDescr();
    return CallElemDscrFunc (pDscr.get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BJB                             10/89
+---------------+---------------+---------------+---------------+---------------+------*/
bool ScanCriteria::TransferElement (int* scanStatus)
    {
    if (1 == m_type.iteration)
        {
        if (SUCCESS != DoElemDscrCallback ())
            *scanStatus = BUFF_FULL;

        return true;
        }

    if (2 == m_type.iteration)
        {
        if (SUCCESS != DoElemRefCallback ())
            *scanStatus = BUFF_FULL;

        return true;
        }

    return  true;
    }

#if defined (NEEDS_WORK_DGNITEM)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BJB                             12/89
+---------------+---------------+---------------+---------------+---------------+------*/
ScanTestResult ScanCriteria::CheckCellName (DgnElementCP elem) const
    {
    WChar cellName[MAX_CELLNAME_LENGTH];

    if (elem && SUCCESS == CellUtil::GetCellName (cellName, sizeof(cellName)/sizeof(cellName[0]), *elem))
        return (0 == BeStringUtilities::Wcsicmp (m_cellName, cellName)) ? ScanTestResult::Pass : ScanTestResult::Fail;

    return ScanTestResult::Fail;
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BJB                             11/89
+---------------+---------------+---------------+---------------+---------------+------*/
ScanTestResult ScanCriteria::CheckEntityandOccurence (DgnElementCP el) const
    {
    ConstPtrUnionV8    p;
    ConstPtrUnionV8    endattr;
    long                        occurence;

    if (el->GetSizeWords() <= el->GetAttributeOffset())
        return  ScanTestResult::Fail;

    p.elem = el;

    /* get end of attributes */
    endattr.wd = p.wd + p.elem->GetSizeWords();

    /* get start of attributes */
    p.wd += p.elem->GetAttributeOffset();
    for ( ; p.wd < endattr.wd; p.wd +=4)
        {
        if (m_type.testAttributeEntity)
            if (*(p.wd + 1) != m_entity)
                continue;
        if (m_type.testAttributeOccurrence)
            {
            occurence  = *(p.wd + 3);
            occurence  =   occurence << 16;
            occurence |= *(p.wd + 2);
            if (occurence != m_occurrence)
                continue;
            }
        return  ScanTestResult::Pass;
        }

    return  ScanTestResult::Fail;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    BJB                             11/89
+---------------+---------------+---------------+---------------+---------------+------*/
ScanTestResult ScanCriteria::CheckExtendedAttributes (DgnElementCP el) const
    {
    ConstPtrUnionV8 p;
    ConstPtrUnionV8 endattr;
    const UShort    *elemAttrP, *checkEndP, *maskP, *valP;
    unsigned int     length;

    if (el->GetSizeWords() <= el->GetAttributeOffset())
        return  ScanTestResult::Fail;

    p.elem = el;

    /* get end of attributes */
    endattr.wd = p.wd + p.elem->GetSizeWords();

    /* get start of attributes - make sure attrindx is positive */
    if ((int)p.elem->GetAttributeOffset() < 0)
        return ScanTestResult::Fail;

    p.wd += p.elem->GetAttributeOffset();

    for ( ; p.wd < endattr.wd; p.wd += length)
        {
        LinkageHeader *linkHdrP = (LinkageHeader *)p.wd;

        /* get attribute linkage length */
        if (linkHdrP->user)
            {
            if (linkHdrP->remote)
                length = linkHdrP->wdMantissa * (1 << linkHdrP->wdExponent);
            else
                length = linkHdrP->wdMantissa + 1;
            }
        else
            length = 4;

        if (0 == length)            // Avoid infinite loop.
            return ScanTestResult::Fail;

        if (length >= (unsigned int) m_extAttrBuf->numWords)
            {
            for (elemAttrP = p.wd,
                 checkEndP = p.wd + m_extAttrBuf->numWords,
                    maskP = m_extAttrBuf->extAttData,
                        valP = maskP + m_extAttrBuf->numWords;
                            elemAttrP < checkEndP;
                                elemAttrP++, maskP++, valP++)
                {
                if ((*elemAttrP & *maskP) != *valP)
                    /* doesn't match - jump out and go to next attrib */
                    break;
                }
            /* got to end of checked data without a mismatch - it matches */
            if (elemAttrP >= checkEndP)
                return ScanTestResult::Pass;
            }
        }
    return ScanTestResult::Fail;
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

    /* is this a multirange check? */
    if (m_type.testMultiRange)
        {
        int iRange;
        DRange3dCP srP;

        for (iRange=0, srP = m_range; iRange < m_numRanges; srP++, iRange++)
            {
            if (srP->low.x > srP->high.x)
                continue;

            if (checkSubRange (*srP, elemRange, isElem3d) == ScanTestResult::Pass)
                return ScanTestResult::Pass;
            }
        return ScanTestResult::Fail;
        }

    return ScanTestResult::Pass;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool ScanCriteria::CheckElementRange (ElementHandleCR el) const
    {
    DgnElementCP elData = el.GetElementCP();
    if (NULL == elData)
        return false;

    PersistentElementRefP elRef = (PersistentElementRefP) el.GetElementRef();
    DRange3dCP range = elRef ? elRef->CheckIndexRangeOfElement (*elData) : &elData->GetRange();
    if (NULL == range)
        return true;

    // This used to call CheckRange rather than _CheckRangeIndexNode - this would skip
    // the m_appNodeRangeCheck->_CheckNodeRange call. Now with the call to _CheckRangeIndexNode
    // the extra range test is done on each individual element as well as the range tree nodes.
    // This makes wireframe work consistently with occlusion culling (TR# 368008and allows us to remove
    // the redundant and kludgey test in ViewContest::DrawElementCut   - RayBentley 7/2012
    return _CheckRangeIndexNode (*range, elData->Is3d(), true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      01/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool ScanCriteria::_CheckRangeIndexNode (DRange3dCR nodeRange, bool is3d, bool isElement) const
    {
    if (ScanTestResult::Pass != CheckRange (nodeRange, is3d))
        return false;

    return (NULL == m_appRangeNodeCheck) ? true : (ScanTestResult::Pass == m_appRangeNodeCheck->_CheckNodeRange (*this, nodeRange, is3d, isElement));
    }

/*---------------------------------------------------------------------------------**//**
* @return false if the element is OK, true if it is rejected
* @bsimethod                                                    KeithBentley    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
ScanTestResult ScanCriteria::CheckElement (ElementHandleCR elHandle, bool doRangeTest, bool doAttrTest) const
    {
    ElementRefP  elemRef = elHandle.GetElementRef();

    //  Don't get pElem until we are certain it is needed. Getting it might cause the CacheManager to swap in the
    //  element.
    DgnElementCP pElem = elHandle.GetElementCP();

#if defined (NEEDS_WORK_DGNITEM)
    /* check type if necessary */
    if (m_type.testElementType)
        {
        /* check the scanlist type mask */
        if (!(m_typeMask[(elType-1) >> 4] & 1 << ((elType-1) & 0xf)))
            return  ScanTestResult::Fail;
        }
#endif

    /* do we need to check properties ? */
    if (m_type.testProperties)
        {
        if (((m_propertiesMask & pElem->GetProperties()) ^ m_propertiesVal) & M_NOCLASS)
            return  ScanTestResult::Fail;
        }

#if defined (NEEDS_WORK_DGNITEM)
    /* do we need to check locked ? */
    if (m_type.testLocked)
        {
        if (m_lockedValue != pElem->IsLocked())
            return  ScanTestResult::Fail;
        }

    if (m_type.testCellName)
        {
        switch (elType)
            {
            case CELL_HEADER_ELM:
            case SHARED_CELL_ELM:
                if (CheckCellName (pElem) == ScanTestResult::Fail)
                    return  ScanTestResult::Fail;
            }
        }
#endif

    if (m_type.testLevel || m_type.testClass)
        {
        Handler* handler = elemRef ? elemRef->GetHandler() : NULL;
        if (NULL == handler)
            handler= &elHandle.GetHandler();

        if (ScanTestResult::Pass != handler->DoScannerTests (elHandle, m_type.testLevel ? m_levelBitMask: NULL, m_type.testClass ? &m_classMask : NULL, m_viewContext))
            return  ScanTestResult::Fail;
        }

    /* ----------------------------------------------------------------
       Back to all element checks
       ---------------------------------------------------------------- */
    /* Do the XAttribute test.  It's above the range test because we think it's a bit cheaper */
    if (m_type.testXAttributes)
        {
        XAttributeHandle curr (elemRef, m_xAttrHandlerId, m_xAttrId);
        if (!curr.IsValid())
            return ScanTestResult::Fail;
        }

    /* check the range */
    if (doRangeTest)
        {
        if (!CheckElementRange (elHandle))
            return  ScanTestResult::Fail;
        }

    if (doAttrTest)
        {
        /* see if we need to check entity or occurence numbers */
        if (m_type.testAttributeEntity || m_type.testAttributeOccurrence)
            if (ScanTestResult::Fail == CheckEntityandOccurence (pElem))
                return  ScanTestResult::Fail;

        /* see if we need to do extended attribute scanning */
        if (m_type.testAttributeExtended)
            if (ScanTestResult::Fail == CheckExtendedAttributes (pElem))
                return  ScanTestResult::Fail;
        }

    // if user has set up a scanFilter callback function, call it.
    if (NULL != m_elemfilter)
        return  m_elemfilter->_FilterElement (this, pElem);

    return  ScanTestResult::Pass;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       ScanCriteria::ProcessElemRefRangeList()
    {
    for (T_RangeHits::iterator curr = m_rangeHits->begin(); curr < m_rangeHits->end(); curr++)
        {
        if (SUCCESS != CallElemRefFunc (*curr))
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

    int             scanStatus = 0;
    bool            complexHeader = false;

    for (T_RangeHits::iterator curr = m_rangeHits->begin() + m_currRangeHit; curr < m_rangeHits->end(); curr++, m_currRangeHit++)
        {
        /* ---------------------------------------------------------------
           If we have an element already qualified, then we have
           already set up our pointers and we just continue from where we
           left off.  If we don't, we go to the next element in the list.
           --------------------------------------------------------------- */
        if (!m_elemOk && !m_inComplex)
            {
            PersistentElementRefP   tstElem = *curr;

            /* ------------------------------------------------------------
               The first step is to get a pointer to the element we are
               validating.  If the element is in the cache the PtrPair kept
               by the range tree specifies that address.  If the PtrPair gives
               us a NULL pointer, we know we have to read it from the file
               into our fileBuffer.
               ------------------------------------------------------------ */
            if (tstElem)
                {
                m_iterator.SetCurrentElm (tstElem);
                }
            }

        for (;;)
            {
            ElementRefP      currElRef = m_iterator.GetCurrentElementRef();

            /* if we encounter EOF marker, stop scan */
            if (NULL == currElRef)
                {
                m_inComplex = 0;
                scanStatus = END_OF_DGN;
                break;
                }

            bool            goToNextElement = true;
            ElementHandle   currElm (currElRef);   // to lock element data in memory

            /* was element previously accepted but would not fit? */
            if (m_elemOk)
                {
                complexHeader = (m_inComplex == ACC_CMPLX_HDR);
                goto Accept;
                }

            /* see if the element is a complex or header or not */
#if defined (NEEDS_WORK_DGNITEM)
            complexHeader = currEl->IsComplexHeaderType();
            if (m_inComplex)
                {
                if (currEl->IsComplexHeaderElement())
                    {
                    if (m_inComplex == ACCEPTING_CMPLX)
                        goto Accept;
                    else if (m_inComplex == REJECTING_CMPLX)
                        goto Reject;
                    }
                else
                    {
                    /* no longer in complex element */
                    m_inComplex = 0;
                    /* out of this loop and on to the next candidate */
                    break;
                    }
                }
#endif

Accept:
            if (!scanStatus)
                {
                /* indicate element OK in case it will not fit */
                m_elemOk = true;

#if defined (NEEDS_WORK_DGNITEM)
                if (complexHeader)
                    {
                    /* bit mask 0 represents element type 1 */
                    /* we do not use elemType any more, we can corrupt it */
                    elemType--;
                    forceAll = (m_overrideNest[elemType >> 4] & 1 << (elemType & 0xf));

                    /* if nest bit off and not force all components,
                        treat complex header just like any other element */
                    if (m_type.nestCells || forceAll)
                        {
                        m_inComplex = ACCEPTING_CMPLX;
                        }

                    /* For cmplx hdrs without NEST mode, check components */
                    else
                        {
                        m_inComplex = CHECKING_CMPLX;
                        }
                    }
#endif

                /* --------------------------------------------------------
                   Saving an element that is not a complex header or a complex
                   header in unNest mode, or a complex header when data only
                   (no pointers) is being returned.
                   -------------------------------------------------------- */
                goToNextElement = TransferElement (&scanStatus);

                if (!scanStatus)
                    {
                    if (goToNextElement)
                        m_iterator.GetNextElementRef(true);

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
                        if (!m_inComplex)
                            m_currRangeHit++;

                        scanStatus = BUFF_FULL;
                        break;
                        }

                    /* ---------------------------------------------------
                       Break out for an element that is not a complex
                       component element and not a complex header.  Also
                       break out for a complex header in nest mode, since
                       all we want is the outermost header.
                       --------------------------------------------------- */
                    if (!complexHeader && !m_inComplex)
                        break;

                    if (complexHeader && (m_inComplex == REJECTING_CMPLX))
                        {
                        m_inComplex = 0;
                        break;
                        }
                    else
                        continue;
                    }
                }
            /* ------------------------------------------------------------
               Gets here if we can't get the whole element, or buffer full.
               ------------------------------------------------------------ */
            break;

#if defined (NEEDS_WORK_DGNITEM)
    Reject:
            /* ------------------------------------------------------------
               Element Rejection Code

               For complex elements in nest mode, we set inComplex to indicate
               that we are in the middle of rejecting a complex element.
               ------------------------------------------------------------ */
            if (complexHeader)
                {
                /* bit maps start with bit 0 representing element type 1 */
                /* since we do not use elemType any more, we can corrupt it */
                elemType--;
                forceAll = (m_overrideNest[elemType >> 4] & 1 << (elemType & 0xf));

                /* if nest bit is off, treat cmplx hdr same as any element */
                if (m_type.nestCells)
                    m_inComplex = REJECTING_CMPLX;
                else
                    m_inComplex = CHECKING_CMPLX;
                }

            /* element not OK */
            m_elemOk = false;

            /* on to next element */
            m_iterator.GetNextElementRef(true);

            /* ------------------------------------------------------------
               If we have just rejected an element that is not a complex
               component, and not a complex header, break out of the testing
               loop to go back to the range tree list.
               ------------------------------------------------------------ */
            if (!complexHeader && !m_inComplex)
                break;

            if (complexHeader && (m_inComplex == REJECTING_CMPLX))
                {
                m_inComplex = 0;
                break;
                }
            else
                continue;
#endif
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

typedef int  (*PFScanElemRefNodeCallback) (ElementRefP, void* callbackArg, ScanCriteriaP);
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/07
+---------------+---------------+---------------+---------------+---------------+------*/
RangeMatchStatus ScanCriteria::_VisitRangeIndexElem (PersistentElementRefP elemRef)
    {
    ElementHandle currElm (elemRef);   // to lock element data in memory
    if (!CheckElementRange (currElm))
        return RANGEMATCH_Ok;

    if (ScanTestResult::Fail == CheckElement (currElm, false, true))
        return RANGEMATCH_Ok;

    if (UseRangeTreeOrdering())
        {
        PFScanElemRefNodeCallback   cbFunc = (PFScanElemRefNodeCallback) m_callbackFunc;
        return (SUCCESS == cbFunc (elemRef, m_callbackArg, this)) ? RANGEMATCH_Ok : RANGEMATCH_Aborted;
        }

    if (m_rangeHits->size() > MAX_ORDERED_HITS)
        return  RANGEMATCH_TooManyHits;

    m_rangeHits->push_back(elemRef);
    return  RANGEMATCH_Ok;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/07
+---------------+---------------+---------------+---------------+---------------+------*/
RangeMatchStatus ScanCriteria::FindRangeHits(ElemRangeIndexP rangeIndex)
    {
    if (NULL == m_rangeHits)
        m_rangeHits = new T_RangeHits;
    else
        m_rangeHits->clear();

    m_currRangeHit = 0;
    RangeMatchStatus stat = rangeIndex->FindMatches (*this);
    if (RANGEMATCH_Ok != stat)
        {
        ResetState(); // aborted - throw away hits
        return stat;
        }

#ifdef DGNV10FORMAT_CHANGES_WIP
    std::sort (m_rangeHits->begin(), m_rangeHits->end(), AscendingFilepos());
#endif
    return RANGEMATCH_Ok;
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
    int             scanStatus;            /* scan status */

    if (NULL == m_model)
        return BAD_FILE;

    bool wasNewCriteria = TO_BOOL (m_newCriteria);
    if (wasNewCriteria)
        {
        // clear a bunch of context stuff
        m_inComplex = m_elemOk = 0;
        m_iterator.SetModel (m_model);
        m_iterator.GetFirstElementRef(); 
        }

    m_viewContext = context;
    m_newCriteria = false;  // not a new scan anymore

    /* init input and output pointers, output buffer end, words returned */
    scanStatus         = 0;

    ElemRangeIndexP  rangeIndex;
    if (m_type.testRange && (NULL != (rangeIndex = m_model->GetRangeIndexP(true))))
        {
        if (!UseRangeTreeOrdering() && !UseRangeTree(rangeIndex))
            goto linearScan;

        if (wasNewCriteria)
            {
            switch (FindRangeHits (rangeIndex))
                {
                case RANGEMATCH_TooManyHits:
                    goto linearScan;

                case RANGEMATCH_Aborted:
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
        ElementRefP  currElRef = m_iterator.GetCurrentElementRef();

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

        bool            goToNextElement = true;
        ElementHandle   currElm (currElRef); // to lock element data in memory for remainder of tests
        DgnElementCP    currEl = currElm.GetElementCP();

        /* was element previously accepted but would not fit? */
        if (m_elemOk)
            {
#if defined (NEEDS_WORK_DGNITEM)
            complexHeader = (m_inComplex == ACC_CMPLX_HDR);
            /* we will be checking the override mask */
            if (complexHeader)
                elemType = 1;
#endif
            goto Accept;
            }

        /* if element is deleted, ignore it */
        if (currElRef->IsDeleted())
            {
            goto Reject;
            }

        /* see if this looks like an acceptable element */
        if ((currEl->GetSizeWords() < MIN_EXPANDED_ELEM))
            {
            BeAssert (false);
            scanStatus = BAD_ELEMENT;
            break;
            }

        // see if the element is a complex or header or not

#if defined (NEEDS_WORK_DGNITEM)
        complexHeader = currEl->IsComplexHeaderType();
        if (m_inComplex)
            {
            if (currEl->IsComplexHeaderElement())
                {
                /* still in the complex element */
                if (m_inComplex == ACCEPTING_CMPLX)
                    goto Accept;
                else if (m_inComplex == REJECTING_CMPLX)
                    goto Reject;
                }
            else
                /* no longer in complex element */
                m_inComplex = 0;
            }
#endif

        /* only check attr if scanStatus is OK */
        if (ScanTestResult::Fail == CheckElement (currElm, true, !scanStatus))
            goto Reject;

Accept:
        if (!scanStatus)
            {
            /* indicate element OK in case it will not fit */
            m_elemOk = true;

            /* if it's a complex header, see if we are forcing return of
               all its component elements */
#if defined (NEEDS_WORK_DGNITEM)
            if (complexHeader)
                {
                /* bit maps start with bit 0 representing element type 1 */
                /* since we do not use elemType any more, we can corrupt it */
                elemType--;
                forceAll = (m_overrideNest[elemType >> 4] & 1 << (elemType & 0xf));

                /* if nest bit is off, treat cmplx hdr same as any element */
                if (m_type.nestCells || forceAll)
                    {
                    m_inComplex = ACCEPTING_CMPLX;
                    }
                }
#endif

            /* ------------------------------------------------------------
               Saving an element that is not a complex header or a complex
               header in unNest mode, or a complex header when data only
               (no pointers) is being returned.
               ------------------------------------------------------------ */
            goToNextElement = TransferElement (&scanStatus);
            if (!scanStatus)
                {
                if (goToNextElement)
                    m_iterator.GetNextElementRef(true);

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
        /* ---------------------------------------------------------------
           Gets here if we can't get the whole element, or buffer full.
           --------------------------------------------------------------- */
        break;


Reject:
        /* --------------------------------------------------------------
           Element Rejection Code

           For complex elements in nest mode, we set inComplex to indicate
           that we are in the middle of rejecting a complex element.
           -------------------------------------------------------------- */
#if defined (NEEDS_WORK_DGNITEM)
        if (complexHeader)
            {
            /* bit maps start with bit 0 representing element type 1 */
            /* since we do not use elemType any more, we can corrupt it */
            elemType--;
            forceAll = (m_overrideNest[elemType >> 4] & 1 << (elemType & 0xf));

            /* if nest bit is off, treat complex header same as any element */
            if (m_type.nestCells)
                m_inComplex = REJECTING_CMPLX;
            else
                m_inComplex = CHECKING_CMPLX;
            }
#endif

        /* element not OK */
        m_elemOk = false;
        m_iterator.GetNextElementRef(true);
        }

    return scanStatus;
    }

