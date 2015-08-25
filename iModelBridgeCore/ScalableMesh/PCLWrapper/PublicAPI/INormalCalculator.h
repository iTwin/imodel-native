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
#include <eigen\Eigen\Dense>
BEGIN_PCLWRAPPER_NAMESPACE

struct INormalCalculator;

typedef Bentley::RefCountedPtr<INormalCalculator> INormalCalculatorPtr;

/*=================================================================================**//**
* Interface implemented by MRDTM engines.
* @bsiclass                                                     Bentley Systems
+===============+===============+===============+===============+===============+======*/
struct INormalCalculator : public Bentley::RefCountedBase
    {
    private:

    /*__PUBLISH_SECTION_END__*/
    /*__PUBLISH_CLASS_VIRTUAL__*/
    protected:

        
    /*__PUBLISH_SECTION_START__*/
    public:
        //! Gets the number of points of the DTM.
        //! @return The number of points of the DTM..

        //! Gets the draping interface.
        //! @return The draping interface.
       
        PCLWRAPPER_EXPORT static IStatusPtr ComputeNormals(DVec3d*         pNormals,
                                                           const DPoint3d* pPoints,
                                                           size_t          numberOfPoints);
		PCLWRAPPER_EXPORT static IStatusPtr InitKdTree(void** pKdTree,
                                            const DPoint3d* pPoints,
                                           size_t          numberOfPoints);
		PCLWRAPPER_EXPORT static IStatusPtr InitOctree(void** pOctree,
                                            const DPoint3d* pPoints,
                                           size_t          numberOfPoints,
                                           double resolution = 100.0);
		PCLWRAPPER_EXPORT static IStatusPtr RadiusSearch(int* pIndices,
                                           size_t* size,
                                           void* handle,
                                           const Eigen::Vector3f& point,
                                           float radius,
                                           int k_neighbors);
		PCLWRAPPER_EXPORT static IStatusPtr NearestKSearch(int* pIndices,
                                           size_t* size,
                                           void* handle,
                                           const Eigen::Vector3f& point,
                                           float radius,
                                           int k_neighbors);
		PCLWRAPPER_EXPORT static IStatusPtr ReleaseKdTree(void* pKdTree);
        PCLWRAPPER_EXPORT static IStatusPtr ReleaseOctree(void* pOctree);

    };

END_PCLWRAPPER_NAMESPACE