/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/ScanCriteria.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
/** @cond BENTLEY_SDK_Internal */

#include "DgnModel.h"
#include "RangeIndex.h"

DGNPLATFORM_TYPEDEFS (RangeNodeCheck)

BEGIN_BENTLEY_DGN_NAMESPACE

//=======================================================================================
// @bsiclass                                                      Keith.Bentley   05/07
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ScanCriteria : RangeIndex::Traverser
{
public:
    enum class Reject {Yes=1, No=0};

private:
    bool m_testSkewScan = false;
    bool m_testCategory = false;
    bool m_is3d = true;
    GeometricModelP m_model = nullptr;
    DPoint3d m_skewVector;
    RangeIndex::FBox m_skewRange;
    RangeIndex::FBox m_range;
    DgnCategoryIdSet const* m_categories = nullptr;

    ScanCriteria::Reject CheckElementRange(RangeIndex::EntryCR ) const;
    DGNPLATFORM_EXPORT Stop _VisitRangeTreeEntry(RangeIndex::EntryCR) override;
    virtual Stop _OnRangeElementFound(DgnElementId) = 0;

public:
    RangeIndex::FBoxCR GetScanRange() const {return m_range;}
    DPoint3dCR GetSkewVector() const {return m_skewVector;}
    GeometricModelP GetDgnModel() {return m_model;}

    DgnCategoryIdSet const* GetCategories() const {return m_categories;}

    DGNPLATFORM_EXPORT Reject CheckRange(RangeIndex::FBoxCR elemRange) const;

    void SetSkewRangeTest(RangeIndex::FBoxCR mainRange, RangeIndex::FBoxCR skewRange, DPoint3dCR skewVector) {SetRangeTest(mainRange); m_skewRange = skewRange; m_skewVector = skewVector; m_testSkewScan = true;}

    //! Set the Categories filter. Only elements on categories that pass the category test will be returned by the scan.
    //! @param[in] categories  The set of categoryids
    void SetCategoryTest(DgnCategoryIdSet const& categories) {m_categories = &categories; m_testCategory = true;}

    //! Sets the DgnModel that is to be scanned.
    //! @param[in] model The model to be scanned.
    void SetDgnModel(GeometricModelR model) {m_model = &model; m_is3d = model.Is3d();}

    //! Sets the range testing for the scan. If scanRange is NULL, then no range testing is performed.
    //! @param[in] scanRange    The range to test. An element whose range overlaps any part of scanRange is returned by the scan.
    void SetRangeTest(RangeIndex::FBoxCR range) {m_testSkewScan = false; m_range = range;}

    //! Perform the scan, filtering elements as dictated by this ScanCriteria, calling the callback specified in #SetCallback.
    DGNPLATFORM_EXPORT StatusInt Scan();

    //! Check one particular element agains this ScanCriteria
    //! @param[in] element The element to test.
    //! @param[in] doRangeTest Check the range.
    DGNPLATFORM_EXPORT Reject CheckElement(RangeIndex::EntryCR element, bool doRangeTest) const;
};

END_BENTLEY_DGN_NAMESPACE

/** @endcond */
