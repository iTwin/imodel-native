/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "AutomaticGroundDetectionPch.h"
#include "GroundDetectionMacros.h"

#include "TriangleSearcher.h"


/*
#include <CGAL/Simple_cartesian.h>
#include <CGAL/point_generators_2.h>
#include <CGAL/Orthogonal_k_neighbor_search.h>
#include <CGAL/Search_traits_2.h>
#include <list>
#include <cmath>
*/

typedef CTree::Point_and_primitive_id Point_and_primitive_id;

BEGIN_GROUND_DETECTION_NAMESPACE

#if 0 
int maintest()
    {    
    Point a(1.0, 0.0, 0.0);
    Point b(0.0, 1.0, 0.0);
    Point c(0.0, 0.0, 1.0);
    Point d(0.0, 0.0, 0.0);
    bvector<Triangle> triangles;
    triangles.push_back(Triangle(a,b,c));
    triangles.push_back(Triangle(a,b,d));
    triangles.push_back(Triangle(a,d,c));
    // constructs AABB tree    
    CTree tree(triangles.begin(),triangles.end());

    tree.accelerate_distance_queries();

    // counts #intersections
/*
    Ray ray_query(a,b);
    std::cout << tree.number_of_intersected_primitives(ray_query)        << " intersections(s) with ray query" << std::endl;
*/
    // compute closest point and squared distance    
    Point point_query(2.0, 2.0, 2.0);
    Point closest_point = tree.closest_point(point_query);
#if 0 
    std::cerr << "closest point is: " << closest_point << std::endl;
    FT sqd = tree.squared_distance(point_query);
    std::cout << "squared distance: " << sqd << std::endl;
#endif
    return EXIT_SUCCESS;
    }
#endif


TriangleSearcher::TriangleSearcher()
    {
    m_needTreeRebuilding = false;    
    }

TriangleSearcher::~TriangleSearcher()
    {

    }

TriangleSearcherPtr TriangleSearcher::Create()
    {
    return new TriangleSearcher;
    }

void TriangleSearcher::AddTriangle(CTriangle& triangle)
    {
    bvector<CTriangle> triangles;
    triangles.push_back(triangle);
    
    m_searchTree.insert(CPrimitive(triangles.begin()));

    //m_triangles.push_back(triangle);
    m_needTreeRebuilding = true;        
    }

void TriangleSearcher::SearchNearestTri(CTriangle& nearestTriangle, double& distance, DPoint3d& location)
    {
    if (m_needTreeRebuilding)
        {        
    //    m_searchTree.rebuild(m_triangles.begin(), m_triangles.end());
        m_searchTree.accelerate_distance_queries();
        m_needTreeRebuilding = false;
        }

    CPoint searchPt(location.x, location.y, location.z);    
    Point_and_primitive_id pp = m_searchTree.closest_point_and_primitive(searchPt);
    nearestTriangle = *pp.second;
    }



END_GROUND_DETECTION_NAMESPACE