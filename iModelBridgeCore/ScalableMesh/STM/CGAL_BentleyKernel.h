/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/CGAL_BentleyKernel.h $
|    $RCSfile: MTGGraphGraphTraits.h,v $
|   $Revision: 1.0 $
|       $Date: 2015/12/14 10:09:23 $
|     $Author: Elenie.Godzaridis $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
// This file contains glue code to use CGAL algorithms on BentleyGeometry types. It follows the traits pattern; see following external links for additional information.
// [http://doc.cgal.org/latest/Manual/devman_traits_classes.html][http://www.cs.fsu.edu/~lacher/boost_1_32_0/libs/graph/doc/graph_traits.html]
// This file contains the definitions for CGAL functions on Bentley types based on the actual geom implementation, it does not use the CGAL kernel.
#pragma once
#include <CGAL/Iterator_range.h>
#include <CGAL/Vector_3.h>
#include <array>
#include <CGAL/Simple_cartesian.h>

namespace CGAL
    {
    struct DVec3d2 : public BENTLEY_NAMESPACE_NAME::DVec3d
        {
        DVec3d2();
        DVec3d2(const Null_vector& vec)
            {};
        DVec3d2(const BENTLEY_NAMESPACE_NAME::DVec3d& vec)
            :DVec3d(vec)
            {};
        DVec3d2(const BENTLEY_NAMESPACE_NAME::DPoint3d& vec)
            :DVec3d(DVec3d::From(vec.x, vec.y, vec.z))
            {};
        };

    BENTLEY_NAMESPACE_NAME::DPoint3d operator-(const BENTLEY_NAMESPACE_NAME::DPoint3d& a, const BENTLEY_NAMESPACE_NAME::DPoint3d&b)
        {
        DPoint3d sub;
        sub.DifferenceOf(a, b);
        return sub;
        }

    BENTLEY_NAMESPACE_NAME::DPoint3d operator-(const BENTLEY_NAMESPACE_NAME::DPoint3d& a, const Origin&b)
        {
        return a;
        }

    double operator* (const DVec3d2 a, const DVec3d2 b)
        {
        return a.DotProduct(b);
        }

    BENTLEY_NAMESPACE_NAME::DVec3d operator*(const BENTLEY_NAMESPACE_NAME::DVec3d& a, const double b)
        {
        return DVec3d::FromScale(a, b);
        }


    BENTLEY_NAMESPACE_NAME::DVec3d operator*(const double b, const BENTLEY_NAMESPACE_NAME::DVec3d& a)
        {
        return DVec3d::FromScale(a, b);
        }

    struct BentleyKernel
        {
        typedef DPoint3d Point_3;
        typedef DVec3d2 Vector_3;
        typedef double FT;

        typedef struct
            {
            bool operator()(const Point_3& a, const Point_3& b)
                {
                return bsiDPoint3d_pointEqualTolerance(&a, &b, 1e-6);
                }

            bool  operator()(const Vector_3& a, const Vector_3& b)
                {
                return bsiDPoint3d_pointEqualTolerance(&a, &b, 1e-6);
                }
            } Equal_3;
        Equal_3 equal_3_object() { return Equal_3(); }

        typedef struct
            {
            FT operator()(const Point_3& a, const Point_3& b)
                {
                return a.DistanceSquared(b);
                }

            } ComputeSquaredDistance_3;
        ComputeSquaredDistance_3 compute_squared_distance_3_object() { return ComputeSquaredDistance_3(); }

        typedef struct
            {
            Vector_3 operator()(const Point_3& a, const Point_3& b)
                {
                return DVec3d::FromStartEnd(a, b);
                }
            Vector_3 operator()(const Null_vector& vec)
                {
                return DVec3d();
                }
            } ConstructVector_3;
        ConstructVector_3 construct_vector_3_object() { return ConstructVector_3(); }

        typedef struct
            {
            Vector_3 operator()(const Vector_3& a, const Vector_3& b)
                {
                return DVec3d::FromCrossProduct(a, b);
                }
            } ConstructCrossProductVector_3;
        ConstructCrossProductVector_3  construct_cross_product_vector_3_object() { return ConstructCrossProductVector_3(); }

        typedef struct
            {
            FT operator()(const Vector_3& a, const Vector_3& b)
                {
                return a.DotProduct(b);
                }
            } ComputeScalarProduct_3;
        ComputeScalarProduct_3 compute_scalar_product_3_object() { return ComputeScalarProduct_3(); }
        };

    template <>
    struct Kernel_traits < DPoint3d >
        {
        typedef BentleyKernel Kernel;
        };


    BentleyKernel::FT squared_distance(const BentleyKernel::Point_3&a, const BentleyKernel::Point_3& b)
        {
        BentleyKernel::ComputeSquaredDistance_3 dist;
        return dist(a, b);
        }

    BentleyKernel::Point_3 midpoint(const BentleyKernel::Point_3&a, const BentleyKernel::Point_3& b)
        {
        return DPoint3d::FromInterpolate(a, 0.5, b);
        }

    BentleyKernel::Vector_3 cross_product(const BentleyKernel::Vector_3&a, const BentleyKernel::Vector_3& b)
        {
        return DVec3d::FromCrossProduct(a, b);
        }

    };