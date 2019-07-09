/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "PCLWrapperPch.h"
#include "GeometricPropertyEstimators.h"

BEGIN_PCLWRAPPER_NAMESPACE

template <typename PointIn, typename PointOut>
inline IStatus::Type ComputePointNormals(boost::shared_ptr<pcl::PointCloud<PointOut>>&    pointCloudNormal,
                                         boost::shared_ptr<pcl::PointCloud<PointIn>>&     pointCloudCluster,
                                         pcl::PointIndicesPtr&                            pointIndicesPtr,
                                         int                                              kSearch,
                                         boost::shared_ptr<pcl::search::Search<PointIn>>& searchTreePtr,
                                         bool                                             useMultiThreads)
    {
    //Not implemented yet
    assert(pointIndicesPtr == 0);

    IStatus::Type status = IStatus::ERROR;

    pcl::Feature<PointIn, PointOut>::KdTreePtr featureSearchTreePtr;

    if (searchTreePtr == 0)
        {
        featureSearchTreePtr = pcl::Feature<PointIn, PointOut>::KdTreePtr(new pcl::search::KdTree<PointIn> ());

        featureSearchTreePtr->setInputCloud(pointCloudCluster);
        }
    else
        {
        featureSearchTreePtr = searchTreePtr;
        }

    if (useMultiThreads)
        {
        double vpx, vpy, vpz;

        vpx = 0;
        vpy = 0;
        vpz = 0;

        #pragma omp parallel for
        for (int idx = 0; idx < pointCloudCluster->size (); ++idx)
            {
            // Allocate enough space to hold the results
            // \note This resize is irrelevant for a radiusSearch ().
                /*
            std::vector<int> nn_indices (kSearch);
            std::vector<float> nn_dists (kSearch);
            */
            std::vector<int> nn_indices;
            std::vector<float> nn_dists;

            int found = featureSearchTreePtr->nearestKSearch (pointCloudCluster->operator[](idx), kSearch, nn_indices, nn_dists);

            // 16-bytes aligned placeholder for the XYZ centroid of a surface patch
            Eigen::Vector4d xyz_centroid;

            // Initialize to 0
            xyz_centroid.setZero ();

            // Placeholder for the 3x3 covariance matrix at each surface patch
            EIGEN_ALIGN16 Eigen::Matrix3d covariance_matrix;
            // Compute the 3x3 covariance matrix

            pcl::computeMeanAndCovarianceMatrix<PointIn>(*pointCloudCluster.get(),
                                                         nn_indices,
                                                         covariance_matrix,
                                                         xyz_centroid);

            EIGEN_ALIGN16 Eigen::Vector3d::Scalar eigen_value;
            EIGEN_ALIGN16 Eigen::Vector3d eigen_vector;
            pcl::eigen33 (covariance_matrix, eigen_value, eigen_vector);

            EIGEN_ALIGN16 Eigen::Vector3f eigen_vector_float;

            eigen_vector_float[0] = eigen_vector[0];
            eigen_vector_float[1] = eigen_vector[1];
            eigen_vector_float[2] = eigen_vector[2];

            flipNormalTowardsViewpoint (pointCloudCluster->operator[](idx), vpx, vpy, vpz,
                                        eigen_vector_float [0],
                                        eigen_vector_float [1],
                                        eigen_vector_float [2]);

            // Compute the curvature surface change
            double eig_sum = covariance_matrix.coeff (0) + covariance_matrix.coeff (4) + covariance_matrix.coeff (8);
            if (eig_sum != 0)
              pointCloudNormal->operator[](idx).curvature = fabs ( eigen_value / eig_sum );
            else
              pointCloudNormal->operator[](idx).curvature = 0;

            pointCloudNormal->operator[](idx).normal[0] = eigen_vector [0];
            pointCloudNormal->operator[](idx).normal[1] = eigen_vector [1];
            pointCloudNormal->operator[](idx).normal[2] = eigen_vector [2];
            }

        status = IStatus::SUCCESS;
        }
    else
        {
        pcl::NormalEstimationOMP<PointIn, PointOut> ne;
        ne.setSearchMethod (featureSearchTreePtr);
        ne.setInputCloud (pointCloudCluster);
        ne.setKSearch (kSearch);
        ne.compute (*pointCloudNormal);

        status = IStatus::SUCCESS;
        }

    return status;
    }

END_PCLWRAPPER_NAMESPACE