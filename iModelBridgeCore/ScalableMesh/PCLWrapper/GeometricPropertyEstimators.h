/*--------------------------------------------------------------------------------------+
|
|     $Source: PCLWrapper/GeometricPropertyEstimators.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//#include "bcSubstMdlDefs.h"

BEGIN_PCLWRAPPER_NAMESPACE


template <typename PointIn, typename PointOut>
inline IStatus::Type ComputePointNormals(boost::shared_ptr<pcl::PointCloud<PointOut>>&    pointCloudNormal,
                                         boost::shared_ptr<pcl::PointCloud<PointIn>>&     pointCloudCluster,
                                         pcl::PointIndicesPtr&                            pointIndicesPtr,
                                         int                                              kSearch,
                                         boost::shared_ptr<pcl::search::Search<PointIn>>& searchTreePtr,
                                         bool                                             useMultiThreads);

END_PCLWRAPPER_NAMESPACE

#include "GeometricPropertyEstimators.hpp"