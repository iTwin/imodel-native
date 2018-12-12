//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: STM/MeshingFunctions.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

#include <unordered_map>

void computeGridDistancesAPSS(std::vector<float>* distances, std::vector<DPoint3d>* points, std::vector<DVec3d>* normals, Eigen::Vector3f bb_min, Eigen::Vector3f bb_max,size_t level,size_t depth);
int computeWeightMatrixAndNormalConstraints(Eigen::MatrixXf& b, Eigen::MatrixXf& D, Eigen::DiagonalMatrix<float, Eigen::Dynamic>& W, std::vector<DPoint3d>* cloud, std::vector<int>* samples, Eigen::Vector3f& test_point, std::vector<DVec3d>* computed_normals);
float isosurfaceAtPointWithNormals(Eigen::MatrixXf& b, Eigen::MatrixXf& D, Eigen::DiagonalMatrix<float, Eigen::Dynamic>& W, Eigen::Vector3f& test_point);
void marchingTetrahedra(std::vector<float>* vertex_triangles, std::vector<float>* distances_grid, const Eigen::Vector3f& bb_min, const Eigen::Vector3f& bb_max);
int generate_triangles_tetrahedra(std::vector<float>* vertex_triangles, std::vector<float>* distances_grid, const Eigen::Vector3f& v0, const Eigen::Vector3f& v1, const Eigen::Vector3f& v2, const Eigen::Vector3f& v3,
                                   const Eigen::Vector3i& i0, const Eigen::Vector3i& i1, const Eigen::Vector3i& i2, const Eigen::Vector3i& i3);