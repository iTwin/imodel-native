/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/ScanCriteria.h $
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

BEGIN_BENTLEY_DGN_NAMESPACE

//=======================================================================================
// @bsiclass                                                      Keith.Bentley   05/07
//=======================================================================================
struct ScanCriteria : DgnRangeTree::Traverser
{
public:
    enum class Result {Pass= 0, Fail = 1,};
    typedef int (*PFScanElementCallback)(DgnElementCR, void *callbackArg, ScanCriteriaR sc);

private:
    enum {MAX_SC_RANGE = 8,};
    struct ScanType
    {
        unsigned int testRange:1;
        unsigned int testSkewScan:1;
        unsigned int testCategory:1;
    };

    int                     m_firstMember;
    ScanType                m_type;
    DgnModelP               m_model;
    int                     m_numRanges;
    DPoint3d                m_skewVector;
    DRange3d                m_skewRange;
    DRange3d                m_range[MAX_SC_RANGE];
    PFScanElementCallback   m_callbackFunc;
    void*                   m_callbackArg;
    DgnCategoryIdSet const* m_categories;
    IRangeNodeCheckP        m_appRangeNodeCheck;
    ViewContextP            m_viewContext;
    int                     m_lastMember;

    bool UseRangeTree(DgnRangeTree&);
    bool CheckElementRange(DgnElementCR) const;
    virtual bool _CheckRangeTreeNode (DRange3dCR, bool) const override;
    virtual DgnRangeTree::Match _VisitRangeTreeElem(GeometricElementCP, DRange3dCR) override;

public:
    DGNPLATFORM_EXPORT ScanCriteria();

    ScanType GetScanType() const {return m_type;}
    DRange3dCR GetScanRange() const {return m_range[0];}
    DPoint3dCR GetSkewVector() const {return m_skewVector;}
    DgnModelP GetDgnModelP() {return m_model;}
    PFScanElementCallback GetCallbackFunc() {return m_callbackFunc;}
    void SetRangeNodeCheck (IRangeNodeCheckP checker) {m_appRangeNodeCheck = checker;}
    DgnCategoryIdSet const* GetCategories() const {return m_categories;}

    DGNPLATFORM_EXPORT Result CheckRange(DRange3dCR elemRange, bool isElem3d) const;
    DGNPLATFORM_EXPORT void SetSkewRangeTest(DRange3dP mainRange, DRange3dP skewRange, DPoint3dP skewVector);

    //! Set the Categories filter. Only elements on categories that pass the category test will be returned by the scan.
    //! @param[in] categories  The set of categoryids
    DGNPLATFORM_EXPORT void SetCategoryTest(DgnCategoryIdSet const& categories);

    //! Sets the DgnModel that is to be scanned.
    //! @param[in] model The model to be scanned.
    DGNPLATFORM_EXPORT StatusInt SetDgnModel(DgnModelP model);

    //! Sets the function that is to be called for each acceptable element when the #Scan method is called.
    //! @param[in] callbackFunc The function that is to be called for each accepted element.
    //! @param[in] callbackArg  A argument passed to the callbackFunc.
    DGNPLATFORM_EXPORT void SetElementCallback(PFScanElementCallback callbackFunc, CallbackArgP callbackArg);

    //! Sets the range testing for the scan. If scanRange is NULL, then no range testing is performed.
    //! @param[in] scanRange    The range to test. An element whose range overlaps any part of scanRange is returned by the scan.
    DGNPLATFORM_EXPORT void SetRangeTest(DRange3dP scanRange);

    //! Perform the scan, filtering elements as dictated by this ScanCriteria, calling the callbackFunc specified in #SetElementCallback.
    DGNPLATFORM_EXPORT StatusInt Scan(ViewContextP=nullptr);

    //! Get the DgnModel set by #SetDgnModel.
    DGNPLATFORM_EXPORT DgnModelP GetDgnModel();

    //! Check one particular element agains this ScanCriteria
    //! @param[in] element The element to test.
    //! @param[in] doRangeTest Check the range.
    DGNPLATFORM_EXPORT Result CheckElement(DgnElementCR element, bool doRangeTest) const;
};

END_BENTLEY_DGN_NAMESPACE

/** @endcond */
