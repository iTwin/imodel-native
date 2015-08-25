/*----------------------------------------------------------------------+
|
|   $Source: PCLWrapper/PCLWrapperPch.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+----------------------------------------------------------------------*/
#pragma once

#include <pcl/common/centroid.h>
#include <pcl/ModelCoefficients.h>
#include <pcl/point_cloud.h>
#include <pcl/point_types.h>
#include <pcl/filters/extract_indices.h>
#include <pcl/filters/passthrough.h>
#include <pcl/filters/radius_outlier_removal.h>
#include <pcl/filters/statistical_outlier_removal.h>
#include <pcl/features/boundary.h>
#include <pcl/features/impl/boundary.hpp>
#include <pcl/features/normal_3d.h>
#include <pcl/features/impl/normal_3d.hpp>
#include <pcl/features/normal_3d_omp.h>
#include <pcl/octree/octree.h>
#include <pcl/octree/octree_iterator.h>
#include <pcl/octree/octree_nodes.h>
#include <pcl/octree/octree_pointcloud.h>
#include <pcl/octree/impl/octree_iterator.hpp>
#include <pcl/sample_consensus/method_types.h>
#include <pcl/sample_consensus/model_types.h>
#include <pcl/sample_consensus/sac_model_line.h>
#include <pcl/segmentation/extract_clusters.h>
#include <pcl/segmentation/region_growing.h>
#include <pcl/segmentation/impl/region_growing.hpp>
#include <pcl/segmentation/sac_segmentation.h>
#include <pcl/segmentation/impl/sac_segmentation.hpp>
#include <pcl/surface/gp3.h>
#include <pcl/surface/impl/gp3.hpp>
#include <pcl/surface/mls.h>
#include <pcl/surface/impl/mls.hpp>
#include <pcl/surface/marching_cubes.h>
#include <unsupported/Eigen/NonLinearOptimization>

//std
using namespace std;

#include <math.h>


#include <PCLWrapper\IDefines.h>


#include <Bentley\RefCounted.h>
#include <Bentley\Bentley.r.h>
#include <Bentley\bvector.h>

#include <Geom\msgeomstructs_typedefs.h>
#include <Geom\dpoint3d.h>
#include <Geom\dvec3d.h>


USING_NAMESPACE_BENTLEY
