/*--------------------------------------------------------------------------------------+
|
|
|   $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

#include "IStatus.h"

BEGIN_PCLWRAPPER_NAMESPACE

struct IRansacUtility;

typedef Bentley::RefCountedPtr<IRansacUtility> IRansacUtilityPtr;

struct IRansacUtility : public Bentley::RefCountedBase
    {
    private:

        /*__PUBLISH_SECTION_END__*/
        /*__PUBLISH_CLASS_VIRTUAL__*/
    protected:


        /*__PUBLISH_SECTION_START__*/
    public:

        PCLWRAPPER_EXPORT static IStatusPtr GetOutliersFromBestPlaneFit(DPoint3d*& pOutliers,
                                                            DPoint3d*& pInliers,
                                                            size_t& nOutliers,
                                                            size_t& nInliers,
                                                           const DPoint3d* pPoints,
                                                           size_t          numberOfPoints,
                                                           double distThreshold = 0.1); //10 centimeters is the default threshold

        PCLWRAPPER_EXPORT static IStatusPtr FitModelAndReturnInliers(DPoint3d*& pInliers,
                                                                        int*& pInlierIds,
                                                                        size_t& nInliers,
                                                                        const DPoint3d* pTestDataSet,
                                                                        size_t nTestDataSet,
                                                                        const DPoint3d* pPoints,
                                                                        size_t          numberOfPoints,
                                                                        double distThreshold = 0.1); //10 centimeters is the default threshold

    };

END_PCLWRAPPER_NAMESPACE