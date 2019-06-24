/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "PCLWrapperPch.h"

#include "Icp.h"
#include "Status.h"

BEGIN_PCLWRAPPER_NAMESPACE
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                              Bentley Systems
+---------------+---------------+---------------+---------------+---------------+------*/
IStatusPtr Icp::setInputCloud (const DPoint3d* pInputCloud, size_t numberOfPoints)
    {
    pcl::PointCloud<pcl::PointXYZ>::Ptr cloud_in(new pcl::PointCloud<pcl::PointXYZ>);
    for (int i = 0; i < numberOfPoints; i++)
        {
        cloud_in->push_back(pcl::PointXYZ (pInputCloud[i].x, pInputCloud[i].y, pInputCloud[i].z));
        }
    m_icp.setInputCloud (cloud_in);

    IStatus::Type status = IStatus::SUCCESS;
    return IStatus::CreateStatus (status);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                              Bentley Systems
+---------------+---------------+---------------+---------------+---------------+------*/
IStatusPtr Icp::setInputTarget (const DPoint3d* pInputTarget, size_t numberOfPoints)
    {
    pcl::PointCloud<pcl::PointXYZ>::Ptr cloud_target (new pcl::PointCloud<pcl::PointXYZ>);
    for (int i = 0; i < numberOfPoints; i++)
        {
        cloud_target->push_back (pcl::PointXYZ(pInputTarget[i].x, pInputTarget[i].y, pInputTarget[i].z));
        }
    m_icp.setInputTarget (cloud_target);

    IStatus::Type status = IStatus::SUCCESS;
    return IStatus::CreateStatus(status);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                              Bentley Systems
+---------------+---------------+---------------+---------------+---------------+------*/
void Icp::align (bvector<DPoint3d>& pFinalCloud)
    {
    pcl::PointCloud<pcl::PointXYZ> Final;
    m_icp.align(Final);

    for (size_t i = 0; i < Final.size(); i++)
        {
        DPoint3d point;
        point.x = Final[i].x;
        point.y = Final[i].y;
        point.z = Final[i].z;
        pFinalCloud.push_back (point);
        }

    return;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                              Bentley Systems
+---------------+---------------+---------------+---------------+---------------+------*/
double Icp::getFitnessScore()
    {
    return m_icp.getFitnessScore();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                              Bentley Systems
+---------------+---------------+---------------+---------------+---------------+------*/
void Icp::getFinalTransformation (DMatrix4d& transformation)
    {
    Eigen::Matrix<float, 4, 4> matrix = m_icp.getFinalTransformation();

    for (size_t i = 0; i < 4; i++)
        for (size_t j = 0; j < 4; j++)
            transformation.coff[i][j] = matrix(i,j);

    return;
    }


END_PCLWRAPPER_NAMESPACE