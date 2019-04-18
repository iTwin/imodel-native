/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma  once

#include "GroundDetectionTypes.h"

#pragma warning( disable : 4005 )  

#define _DEBUG
#include <CGAL/Simple_cartesian.h>
#include <CGAL/AABB_tree.h>
#include <CGAL/AABB_traits.h>
#include <CGAL/AABB_triangle_primitive.h>

typedef CGAL::Simple_cartesian<double> K;
typedef K::FT FT;
typedef K::Point_3 CPoint;
typedef K::Triangle_3 CTriangle;
typedef bvector<CTriangle>::iterator Iterator;
typedef CGAL::AABB_triangle_primitive<K, Iterator> CPrimitive;
typedef CGAL::AABB_traits<K, CPrimitive> AABB_triangle_traits;
typedef CGAL::AABB_tree<AABB_triangle_traits> CTree;

BEGIN_GROUND_DETECTION_NAMESPACE

struct TriangleSearcher : public RefCountedBase
    {
    private: 

        bool               m_needTreeRebuilding;
        CTree              m_searchTree;
        bvector<CTriangle> m_triangles;
        

       TriangleSearcher();
       virtual ~TriangleSearcher();


    public:

        static TriangleSearcherPtr Create();
               
        void AddTriangle(CTriangle& triangle);

        void SearchNearestTri(CTriangle& nearestTriangle, double& distance, DPoint3d& location);
    
    };


END_GROUND_DETECTION_NAMESPACE
