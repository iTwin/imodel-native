/*--------------------------------------------------------------------------------------+
|
|     $Source: PCLWrapper/NormalCalculator.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PCLWrapperPch.h"

#include "NormalCalculator.h"
#include "GeometricPropertyEstimators.h"
#include "Status.h"

BEGIN_PCLWRAPPER_NAMESPACE

IStatusPtr INormalCalculator::ComputeNormals(DVec3d*        pNormals,
                                             const DPoint3d* pPoints,
                                             size_t          numberOfPoints)
    {
    IStatus::Type status = IStatus::ERROR;
    //Probably better to normalize the points to avoid double to float precision problem. 
    //You can check CylinderExtractor::SetSource, line 1228 in 
    //d:\BSI\DescartesScalableMeshMoveUp\src\Descartes\PointCloud\DcPointCloudCore\CylinderExtractor.cpp 
    //for an example of normalization done for Descartes cylinder extraction tool. 

    //I have copied the function ComputePointNormals in src\TerrainModel\PCLWrapper\GeometricPropertyEstimators.h 
    //from the Descartes code. 
    pcl::PointCloud<pcl::Normal>::Ptr normals(new pcl::PointCloud<pcl::Normal>());
    pcl::PointCloud<pcl::PointXYZ>::Ptr points(new pcl::PointCloud<pcl::PointXYZ>());
    pcl::PointIndicesPtr indices(/*new pcl::PointIndices()*/NULL);

    for (int i = 0; i < numberOfPoints; i++)
        {
        points->push_back(pcl::PointXYZ(pPoints[i].x, pPoints[i].y, pPoints[i].z));
        //indices->indices.push_back(points->size() - 1);
        }
    pcl::search::Search<pcl::PointXYZ>::Ptr searchMethod(/*new pcl::search::KdTree<pcl::PointXYZ>()*/NULL);
    status = ComputePointNormals(normals, points, indices, 10, searchMethod, false);
	pcl::PointXYZ minP, maxP;
	pcl::getMinMax3D(*points, minP, maxP);
	Eigen::Vector3f vp(minP.x + (maxP.x - minP.x) / 2, minP.y + (maxP.y - minP.y) / 2, maxP.z+(maxP.z-minP.z));
    for (int i = 0; i < normals->points.size(); i++)
        {
		pcl::flipNormalTowardsViewpoint(points->points[i], vp[0], vp[1], vp[2], normals->points[i].normal_x, normals->points[i].normal_y, normals->points[i].normal_z);
        pNormals[i].x = normals->points[i].normal_x;
        pNormals[i].y = normals->points[i].normal_y;
        pNormals[i].z = normals->points[i].normal_z;
        }
    return IStatus::CreateStatus(status);
    }


IStatusPtr INormalCalculator::InitKdTree(void** pKdTree,
                                            const DPoint3d* pPoints,
                                           size_t          numberOfPoints)
    {
    IStatus::Type status = IStatus::ERROR;
    pcl::PointCloud<pcl::PointXYZ>::Ptr points(new pcl::PointCloud<pcl::PointXYZ>());
    for (int i = 0; i < numberOfPoints; i++)
        {
        points->push_back(pcl::PointXYZ(pPoints[i].x, pPoints[i].y, pPoints[i].z));
        }
    pcl::search::KdTree<pcl::PointXYZ> * tree = new pcl::search::KdTree<pcl::PointXYZ>();
    tree->setInputCloud(points);
    *pKdTree = (void*)tree; //not to be dereferenced by caller
    status = IStatus::SUCCESS;
    return IStatus::CreateStatus(status);
    }

IStatusPtr INormalCalculator::InitOctree(void** pOctree,
                                         const DPoint3d* pPoints,
                                         size_t          numberOfPoints,
                                         double resolution)
    {
    IStatus::Type status = IStatus::ERROR;
    pcl::PointCloud<pcl::PointXYZ>::Ptr points(new pcl::PointCloud<pcl::PointXYZ>());
    for (int i = 0; i < numberOfPoints; i++)
        {
        points->push_back(pcl::PointXYZ(pPoints[i].x, pPoints[i].y, pPoints[i].z));
        }
    pcl::search::Octree<pcl::PointXYZ> * tree = new pcl::search::Octree<pcl::PointXYZ>(resolution);
    tree->setInputCloud(points);
    *pOctree = (void*)tree; //not to be dereferenced by caller
    status = IStatus::SUCCESS;
    return IStatus::CreateStatus(status);
    }
//indices, size, handle, point, neighborRad, max_nn
IStatusPtr INormalCalculator::RadiusSearch(int* pIndices,
                                           size_t* size,
                                           void* handle,
                                           const Eigen::Vector3f& point,
                                           float radius,
                                           int k_neighbors)
    {
    IStatus::Type status = IStatus::ERROR;
    pcl::search::Search<pcl::PointXYZ>* tree = static_cast<pcl::search::Search<pcl::PointXYZ>*>(handle);
    std::vector<int> indices;
    std::vector<float> distances;
    tree->radiusSearch(pcl::PointXYZ(point[0], point[1], point[2]), radius, indices, distances, k_neighbors);
    if (indices.size() > 0)
        {
        *size = indices.size();
        memcpy (pIndices, &indices[0], sizeof (indices[0]) * *size);
        status = IStatus::SUCCESS;
        }
    return IStatus::CreateStatus(status);
    }

IStatusPtr INormalCalculator::NearestKSearch(int* pIndices,
                                           size_t* size,
                                           void* handle,
                                           const Eigen::Vector3f& point,
                                           float radius,
                                           int k_neighbors)
    {
    IStatus::Type status = IStatus::ERROR;
    pcl::search::Search<pcl::PointXYZ>* tree = static_cast<pcl::search::Search<pcl::PointXYZ>*>(handle);
    std::vector<int> indices;
    std::vector<float> distances;
    tree->nearestKSearch(pcl::PointXYZ(point[0], point[1], point[2]), k_neighbors, indices, distances);
    double sqrRad = pow(radius, 2);
    if (indices.size() > 0)
        {
        int count = 0;
        for (int i = 0; i < indices.size(); i++)
            {
            if (distances[i] < sqrRad)
                {
                pIndices[count] = indices[i];
                count++;
                }
            }
        *size = count;
        status = IStatus::SUCCESS;
        }
    return IStatus::CreateStatus(status);
    }
    
IStatusPtr INormalCalculator::ReleaseKdTree(void* pKdTree)
    {
    IStatus::Type status = IStatus::ERROR;
    pcl::search::KdTree<pcl::PointXYZ> * tree = static_cast<pcl::search::KdTree<pcl::PointXYZ> *>(pKdTree);
    delete tree;
    status = IStatus::SUCCESS;
    return IStatus::CreateStatus(status);
    }

IStatusPtr INormalCalculator::ReleaseOctree(void* pOctree)
    {
    IStatus::Type status = IStatus::ERROR;
    pcl::search::Octree<pcl::PointXYZ> * tree = static_cast<pcl::search::Octree<pcl::PointXYZ> *>(pOctree);
    delete tree;
    status = IStatus::SUCCESS;
    return IStatus::CreateStatus(status);
    }
END_PCLWRAPPER_NAMESPACE
