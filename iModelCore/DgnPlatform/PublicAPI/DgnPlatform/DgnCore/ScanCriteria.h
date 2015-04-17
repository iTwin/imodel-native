/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/ScanCriteria.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
/** @cond BENTLEY_SDK_Internal */

#include "DgnModel.h"

BEGIN_BENTLEY_API_NAMESPACE
typedef int (*PFScanElementCallback) (DgnElementCR, void *callbackArg, ScanCriteriaR sc);
END_BENTLEY_API_NAMESPACE


DGNPLATFORM_TYPEDEFS (ElemRangeIndex)
DGNPLATFORM_TYPEDEFS (DgnRangeTree)
DGNPLATFORM_TYPEDEFS (IRangeNodeCheck)

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

enum class ScanTestResult
{
    Pass = 0,
    Fail = 1,
};

enum ScanCriteriaIterationType
{
    MSSCANCRIT_ITERATE_ELEMENT = 1,
    MSSCANCRIT_ITERATE_ELEMENT_UNORDERED = 2,
};

enum ScanCriteriaConstants
{
    // Scanner return values.
    END_OF_DGN      = 10,
    BUFF_FULL       = 11,
};

struct RangeTreeProgressMonitor  { virtual bool _MonitorProgress (double fractionComplete) = 0; };

enum RangeMatchStatus
{
    RANGEMATCH_Ok           = 0,
    RANGEMATCH_Aborted      = 1,
    RANGEMATCH_TooManyHits  = 2,
};

//=======================================================================================
// @bsiclass                                                      Keith.Bentley   05/07
//=======================================================================================
struct  ElemRangeIndex
{
    struct  Traverser
        {
        virtual bool _CheckRangeIndexNode (DRange3dCR, bool is3d, bool isElement) const = 0;   // true == process node
        virtual RangeMatchStatus _VisitRangeIndexElem (GeometricElementCP) = 0;    // true == keep going, false == stop traversal
        };

private:
    int             m_stamp;      // Useful to tell if the range has changed.
    DgnModelCR      m_dgnModel;
    DgnRangeTreeP   m_rangeTree;

public:
    DGNPLATFORM_EXPORT ElemRangeIndex (DgnModelCR);
    DGNPLATFORM_EXPORT ~ElemRangeIndex ();

    int GetStamp () const {return m_stamp;}

    DGNPLATFORM_EXPORT void AddRangeElement(GeometricElementCR);
    StatusInt RemoveElement(GeometricElementCR, DRange3d oldRange);
    DGNPLATFORM_EXPORT RangeMatchStatus FindMatches (Traverser&);
    DGNPLATFORM_EXPORT DgnRangeTreeR GetDgnRangeTree();
    DgnRangeTreeP GetDgnRangeTreeP() {return m_rangeTree;}
    DGNPLATFORM_EXPORT DRange3dCP GetRange();
    StatusInt   GetRangeIfKnown (DRange3dR range);
};

enum
{
    MAX_SC_RANGE       = 8,
    MAX_ORDERED_HITS   = 3000,
};

//=======================================================================================
// @bsiclass                                                      Keith.Bentley   05/07
//=======================================================================================
struct ScanType
{
    unsigned int    returnOneElem:1;
    unsigned int    iteration:2;
    unsigned int    testRange:1;
    unsigned int    testSkewScan:1;
    unsigned int    testCategory:1;
};

//=======================================================================================
//! Criteria object created and used by mdlScanCriteria_* methods. You should prefer to
//! use DgnModel::GetGraphicElementsP or DgnModel::GetControlElementsP and check criteria,
//! on your own, unless you need the range-based criteria supported by the mdlScanCriteria methods.
// @bsiclass                                                      Keith.Bentley   05/07
//=======================================================================================
struct ScanCriteria: ElemRangeIndex::Traverser
{
    struct ElemFilter
        {
        virtual  ScanTestResult _FilterElement (ScanCriteriaCP scP, DgnElementCP element) = 0;
        virtual  void           Release(ScanCriteriaCP scP) = 0;
        };

    //=======================================================================================
    //! Iterator over the elements in a DgnModel.
    //! @bsiclass                                                     KeithBentley    10/00
    //=======================================================================================
    struct ElemIterator : std::iterator<std::forward_iterator_tag, DgnElementP const>
    {
    public:
        enum IteratorState {ITERATING_GraphicElms = 2, ITERATING_HitEOF = 3};

    private:
        DgnElementIterator  m_iter;
        DgnModelP           m_model;
        IteratorState       m_state;

    public:                                  // Make sure this is not constructible in the published API
        ElemIterator () {m_model = NULL; m_state = ITERATING_GraphicElms;}
        ElemIterator (DgnModel* pModel);    // iterate this model only
        ElemIterator (ElemIterator *source);

    public:
        DgnElementCP GetCurrentDgnElement() {return m_iter.GetCurrentDgnElement();}
        IteratorState GetState () {return m_state;}
        DgnModelP GetModel () {return m_model;}

        DgnElementCP GetFirstDgnElement (DgnModelP pModel, bool wantDeleted=false);
        DgnElementCP GetFirstDgnElement () { return GetFirstDgnElement (m_model, false); }
        DgnElementCP GetNextDgnElement (bool wantDeleted=false);
        void SetModel (DgnModelP dgnModel) {m_model = dgnModel; m_iter.Invalidate();}
        bool SetCurrentElm (DgnElementCP toElm);
        void SetAtEOF ();
        bool HitEOF () const {return m_state == ITERATING_HitEOF;}
    public:
        ElemIterator& operator++();
        bool operator!=(ElemIterator const& rhs) const;
        bool operator==(ElemIterator const& rhs) const {return !(*this != rhs);}

        //! Access the element data
        DgnElementCP operator* () const;
    };

private:
    typedef bvector<GeometricElementCP>  T_RangeHits;

    // NOTE: For performance, the constructor initializes members using:
    //         memset (&m_firstMember, 0, offsetof (scanCriteria, m_lastMember)- offsetof (scanCriteria, m_firstMember));
    //         So make sure those members are first/last!

    int                     m_firstMember;
    ScanType                m_type;                // INPUT:  scan type bits
    DgnModelP               m_model;               // DERIVED or INPUT: the model we're scanning
    int                     m_newCriteria;         // INPUT:  new range criteria
    int                     m_numRanges;
    DPoint3d                m_skewVector;          // INPUT:  skewVector if testSkewScan true
    DRange3d                m_skewRange;           // INPUT:  skew range in doubles
    DRange3d                m_range[MAX_SC_RANGE]; // INPUT:  range limits, if testRange true
    PFScanElementCallback   m_callbackFunc;        // INPUT:  iterator function
    void*                   m_callbackArg;         // INPUT:  argument to pass to callbackFunc
    DgnCategoryIdSet const* m_categories;          // INPUT:  the categories
    ElemFilter*             m_elemfilter;
    short                   m_elemOk;              // CONTEXT:  current element OK, wont fit in output
    ElemIterator            m_iterator;
    IRangeNodeCheckP        m_appRangeNodeCheck;
    T_RangeHits*            m_rangeHits;
    int                     m_currRangeHit;
    ViewContextP            m_viewContext;
    int                     m_lastMember;

    StatusInt CallElementFunc (DgnElementCP el);
    StatusInt ProcessRangeIndexResults ();
    bool      UseRangeTree (ElemRangeIndexP);
    bool      UseRangeTreeOrdering () {return 3 == m_type.iteration;}
    bool      IsElemRefIter () {return 3 == m_type.iteration || 2 == m_type.iteration;}
    bool      IsElemDescrIter () {return 1 == m_type.iteration;}
    bool      CheckElementRange (DgnElementCR) const;
    StatusInt DoElementCallback ();
    bool      TransferElement (int* scanStatus);
    void      ResetState ();
    void      Empty ();
    RangeMatchStatus FindRangeHits (ElemRangeIndexP);
    StatusInt ProcessElemRefRangeList ();

    virtual bool _CheckRangeIndexNode (DRange3dCR, bool, bool) const override;
    virtual RangeMatchStatus _VisitRangeIndexElem (GeometricElementCP) override;

public:
    DGNPLATFORM_EXPORT ScanCriteria ();
    DGNPLATFORM_EXPORT explicit ScanCriteria (ScanCriteriaCR);
    DGNPLATFORM_EXPORT virtual ~ScanCriteria ();

    void         CopyRangeTest (ScanCriteriaCR from);
    ScanType     GetScanType () const {return m_type;}
    DRange3dCR   GetScanRange () const {return m_range[0];}
    DPoint3dCR   GetSkewVector () const {return m_skewVector;}
    DgnModelP    GetDgnModelP () {return m_model;}
    PFScanElementCallback GetCallbackFunc () {return m_callbackFunc;}
    void         SetRangeNodeCheck (IRangeNodeCheckP checker) {m_appRangeNodeCheck = checker;}
    DgnCategoryIdSet const* GetCategories() const {return m_categories;}

    DGNPLATFORM_EXPORT ScanTestResult CheckRange (DRange3dCR elemRange, bool isElem3d) const;
    DGNPLATFORM_EXPORT void      SetSkewRangeTest (DRange3dP mainRange, DRange3dP skewRange, DPoint3dP skewVector);
    DGNPLATFORM_EXPORT StatusInt SetReturnType(int returnType, int oneElementOnly, int nestCells);
    DGNPLATFORM_EXPORT void      SetFilter(ElemFilter* filter);
    DGNPLATFORM_EXPORT StatusInt Scan (ViewContextP);

public:
    //! Create a new instance of ScanCriteria
    DGNPLATFORM_EXPORT static ScanCriteriaP Create();

    //! Copy an existing ScanCriteria.
    //! @param[in] source   The existing ScanCriteria to copy.
    DGNPLATFORM_EXPORT static ScanCriteriaP Clone(ScanCriteriaCR source);

    //! Frees a ScanCriteria instance.
    //! @param[in] scanCriteria The ScanCriteria to free.
    DGNPLATFORM_EXPORT static void Delete(ScanCriteriaP scanCriteria);

    //! Set the Categories filter. Only elements on categories that pass the category test will be returned by the scan.
    //! @param[in] categories  The set of categoryids 
    DGNPLATFORM_EXPORT void SetCategoryTest(DgnCategoryIdSet const& categories);

    //! Sets the DgnModel that is to be scanned.
    //! @param[in] modelRef     The model to be scanned.
    DGNPLATFORM_EXPORT StatusInt SetDgnModel(DgnModelP model);

    //! Sets the function that is to be called for each acceptable element when the #Scan method is called.
    //! @param[in] callbackFunc The user function that is to be called for each accepted element.
    //! @param[in] callbackArg  A user-specified argument passed to the callbackFunc.
    DGNPLATFORM_EXPORT void SetElementCallback(PFScanElementCallback callbackFunc, CallbackArgP callbackArg);

    //! Sets the range testing for the scan. If scanRange is NULL, then no range testing is performed.
    //! @param[in] scanRange    The range to test. An element whose range overlaps any part of scanRange is returned by the scan.
    DGNPLATFORM_EXPORT void SetRangeTest(DRange3dP scanRange);

    //! Perform the scan, filtering elements as dictated by this ScanCriteria, calling the callbackFunc specified in #SetElemRefCallback.
    DGNPLATFORM_EXPORT StatusInt Scan();

    //! Get the DgnModel set by #SetDgnModel. This method is often useful from the callbackFunc set in #SetElemRefCallback.
    DGNPLATFORM_EXPORT DgnModelP GetDgnModel();

    //! Check one particular element agains this ScanCriteria
    //! @param[in] element      The element to test.
    //! @param[in] doRangeTest  Check the range.
    DGNPLATFORM_EXPORT ScanTestResult CheckElement(DgnElementCR element, bool doRangeTest) const;
};

END_BENTLEY_DGNPLATFORM_NAMESPACE

/** @endcond */
