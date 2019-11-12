/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
/*__PUBLISH_SECTION_START__*/
#include <pcl/registration/icp.h>
#include <PCLWrapper/IIcp.h>
BEGIN_PCLWRAPPER_NAMESPACE

struct Icp : public IIcp
    {
    /*__PUBLISH_SECTION_END__*/
    private:
        pcl::IterativeClosestPoint<pcl::PointXYZ, pcl::PointXYZ>  m_icp;

    /*__PUBLISH_SECTION_START__*/
    public:
        PCLWRAPPER_EXPORT IStatusPtr setInputCloud (const DPoint3d* pInputCloud, size_t numberOfPoints);
        PCLWRAPPER_EXPORT IStatusPtr setInputTarget (const DPoint3d* pInputTarget, size_t numberOfPoints);
        PCLWRAPPER_EXPORT void align (bvector<DPoint3d>& pFinalCloud);
        PCLWRAPPER_EXPORT double getFitnessScore();
        PCLWRAPPER_EXPORT void getFinalTransformation (DMatrix4d& transformation);
    };

END_PCLWRAPPER_NAMESPACE
/*__PUBLISH_SECTION_END__*/
