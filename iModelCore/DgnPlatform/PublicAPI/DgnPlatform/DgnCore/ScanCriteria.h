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
#include "DgnRangeTree.h"

DGNPLATFORM_TYPEDEFS (DgnRangeTree)
DGNPLATFORM_TYPEDEFS (IRangeNodeCheck)

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

typedef int (*PFScanElementCallback)(DgnElementCR, void *callbackArg, ScanCriteriaR sc);

enum class ScanTestResult
{
    Pass= 0,
    Fail = 1,
};

enum ScanCriteriaConstants
{
    // Scanner return values.
    END_OF_DGN      = 10,
    BUFF_FULL       = 11,
};

enum
{
    MAX_SC_RANGE       = 8,
};

//=======================================================================================
// @bsiclass                                                      Keith.Bentley   05/07
//=======================================================================================
struct ScanType
{
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
struct ScanCriteria : DgnRangeTree::Traverser
{
private:
    typedef bvector<GeometricElementCP>  T_RangeHits;

    // NOTE: For performance, the constructor initializes members using:
    //         memset (&m_firstMember, 0, offsetof (scanCriteria, m_lastMember)- offsetof (scanCriteria, m_firstMember));
    //         So make sure those members are first/last!

    int                     m_firstMember;
    ScanType                m_type;                // INPUT:  scan type bits
    DgnModelP               m_model;               // DERIVED or INPUT: the model we're scanning
    int                     m_numRanges;
    DPoint3d                m_skewVector;          // INPUT:  skewVector if testSkewScan true
    DRange3d                m_skewRange;           // INPUT:  skew range in doubles
    DRange3d                m_range[MAX_SC_RANGE]; // INPUT:  range limits, if testRange true
    PFScanElementCallback   m_callbackFunc;        // INPUT:  iterator function
    void*                   m_callbackArg;         // INPUT:  argument to pass to callbackFunc
    DgnCategoryIdSet const* m_categories;          // INPUT:  the categories
    IRangeNodeCheckP        m_appRangeNodeCheck;
    ViewContextP            m_viewContext;
    int                     m_lastMember;

    bool      UseRangeTree(DgnRangeTree&);
    bool      CheckElementRange(DgnElementCR) const;
    virtual bool _CheckRangeTreeNode (DRange3dCR, bool) const override;
    virtual DgnRangeTree::Match _VisitRangeTreeElem(GeometricElementCP, DRange3dCR) override;

public:
    DGNPLATFORM_EXPORT ScanCriteria ();
    DGNPLATFORM_EXPORT explicit ScanCriteria (ScanCriteriaCR);

    void         CopyRangeTest (ScanCriteriaCR from);
    ScanType     GetScanType () const {return m_type;}
    DRange3dCR   GetScanRange () const {return m_range[0];}
    DPoint3dCR   GetSkewVector () const {return m_skewVector;}
    DgnModelP    GetDgnModelP () {return m_model;}
    PFScanElementCallback GetCallbackFunc () {return m_callbackFunc;}
    void         SetRangeNodeCheck (IRangeNodeCheckP checker) {m_appRangeNodeCheck = checker;}
    DgnCategoryIdSet const* GetCategories() const {return m_categories;}

    DGNPLATFORM_EXPORT ScanTestResult CheckRange (DRange3dCR elemRange, bool isElem3d) const;
    DGNPLATFORM_EXPORT void SetSkewRangeTest (DRange3dP mainRange, DRange3dP skewRange, DPoint3dP skewVector);

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
    DGNPLATFORM_EXPORT StatusInt Scan (ViewContextP = nullptr);

    //! Get the DgnModel set by #SetDgnModel. This method is often useful from the callbackFunc set in #SetElemRefCallback.
    DGNPLATFORM_EXPORT DgnModelP GetDgnModel();

    //! Check one particular element agains this ScanCriteria
    //! @param[in] element      The element to test.
    //! @param[in] doRangeTest  Check the range.
    DGNPLATFORM_EXPORT ScanTestResult CheckElement(DgnElementCR element, bool doRangeTest) const;
};

END_BENTLEY_DGNPLATFORM_NAMESPACE

/** @endcond */
