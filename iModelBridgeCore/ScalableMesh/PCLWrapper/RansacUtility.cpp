/*--------------------------------------------------------------------------------------+
|
|     $Source: PCLWrapper/RansacUtility.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PCLWrapperPch.h"

#include <PCLWrapper\IRansacUtility.h>
#include "Status.h"

BEGIN_PCLWRAPPER_NAMESPACE

IStatusPtr IRansacUtility::GetOutliersFromBestPlaneFit(DPoint3d*& pOutliers, DPoint3d*& pInliers, size_t& nOutliers, size_t& nInliers, const DPoint3d* pPoints, size_t numberOfPoints, double distThreshold)
    {
    IStatus::Type status = IStatus::ERROR;
    pcl::PointCloud<pcl::PointXYZ>::Ptr points(new pcl::PointCloud<pcl::PointXYZ>());
    for (int i = 0; i < numberOfPoints; i++)
        {
        points->push_back(pcl::PointXYZ(pPoints[i].x, pPoints[i].y, pPoints[i].z));
        }
    pcl::SampleConsensusModelPlane<pcl::PointXYZ>::Ptr model_p(new pcl::SampleConsensusModelPlane<pcl::PointXYZ>(points));
    pcl::RandomSampleConsensus<pcl::PointXYZ> ransac(model_p);
    ransac.setDistanceThreshold(distThreshold);
    ransac.setProbability(0.25);
    ransac.computeModel();
    std::vector<int> inliers;
    ransac.getInliers(inliers);
    status = inliers.size() > 0 ? IStatus::SUCCESS : IStatus::ERROR;
    nInliers = inliers.size();
    nOutliers = numberOfPoints - nInliers;
    pInliers = new DPoint3d[nInliers];
    pOutliers = new DPoint3d[nOutliers];
    size_t nI =0, nO = 0;
    for (size_t i = 0; i < numberOfPoints; i++)
        {
        if (std::find(inliers.begin(), inliers.end(), (int)i) != inliers.end()) pInliers[nI++] = pPoints[i];
        else pOutliers[nO++] = pPoints[i];
        }
    return IStatus::CreateStatus(status);
    }

IStatusPtr IRansacUtility::FitModelAndReturnInliers(DPoint3d*& pInliers, int*& pInlierIds, size_t& nInliers, const DPoint3d* pTestDataSet, size_t nTestDataSet, const DPoint3d* pPoints, size_t  numberOfPoints, double distThreshold)
    {
    IStatus::Type status = IStatus::ERROR;
    pcl::PointCloud<pcl::PointXYZ>::Ptr points(new pcl::PointCloud<pcl::PointXYZ>());
    for (int i = 0; i < numberOfPoints; i++)
        {
        points->push_back(pcl::PointXYZ(pPoints[i].x, pPoints[i].y, pPoints[i].z));
        }
    pcl::SampleConsensusModelPlane<pcl::PointXYZ>::Ptr model_p(new pcl::SampleConsensusModelPlane<pcl::PointXYZ>(points));
    pcl::RandomSampleConsensus<pcl::PointXYZ> ransac(model_p);
    ransac.setDistanceThreshold(distThreshold);
    ransac.setProbability(0.8);
    ransac.computeModel();
    std::vector<DPoint3d> testInliers;
    Eigen::VectorXf coeff;
    ransac.getModelCoefficients(coeff);
    std::vector<int> inlierIds;
    for (size_t i = 0; i < nTestDataSet && coeff.size() >=4; i++)
        {
        pcl::PointXYZ  p(pTestDataSet[i].x, pTestDataSet[i].y, pTestDataSet[i].z);
        double d = pcl::pointToPlaneDistance(p, coeff[0], coeff[1], coeff[2], coeff[3]);
        if (fabs(d) < distThreshold)
            {
            testInliers.push_back(pTestDataSet[i]);
            inlierIds.push_back(i);
            }
        }
    nInliers = testInliers.size();
    pInliers = new DPoint3d[nInliers];
    pInlierIds = new int[nInliers];
    memcpy(pInliers, &testInliers[0], nInliers*sizeof(DPoint3d));
    memcpy(pInlierIds, &inlierIds[0], nInliers*sizeof(int));
    return IStatus::CreateStatus(status);
    }

END_PCLWRAPPER_NAMESPACE