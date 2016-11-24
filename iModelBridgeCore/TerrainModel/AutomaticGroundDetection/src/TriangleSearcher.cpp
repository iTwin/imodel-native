/*--------------------------------------------------------------------------------------+
|
|     $Source: AutomaticGroundDetection/src/TriangleSearcher.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "AutomaticGroundDetectionPch.h"
#include "GroundDetectionMacros.h"

#define _DEBUG


#include <CGAL/Simple_cartesian.h>
#include <CGAL/AABB_tree.h>
#include <CGAL/AABB_traits.h>
#include <CGAL/AABB_triangle_primitive.h>

typedef CGAL::Simple_cartesian<double> K;
typedef K::FT FT;
typedef K::Ray_3 Ray;
typedef K::Line_3 Line;
typedef K::Point_3 Point;
typedef K::Triangle_3 Triangle;
typedef std::list<Triangle>::iterator Iterator;
typedef CGAL::AABB_triangle_primitive<K, Iterator> Primitive;
typedef CGAL::AABB_traits<K, Primitive> AABB_triangle_traits;
typedef CGAL::AABB_tree<AABB_triangle_traits> Tree;


/*
#include <CGAL/Simple_cartesian.h>
#include <CGAL/point_generators_2.h>
#include <CGAL/Orthogonal_k_neighbor_search.h>
#include <CGAL/Search_traits_2.h>
#include <list>
#include <cmath>
*/

BEGIN_GROUND_DETECTION_NAMESPACE


int maintest()
    {    
    Point a(1.0, 0.0, 0.0);
    Point b(0.0, 1.0, 0.0);
    Point c(0.0, 0.0, 1.0);
    Point d(0.0, 0.0, 0.0);
    std::list<Triangle> triangles;
    triangles.push_back(Triangle(a,b,c));
    triangles.push_back(Triangle(a,b,d));
    triangles.push_back(Triangle(a,d,c));
    // constructs AABB tree    
    Tree tree(triangles.begin(),triangles.end());
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

#if 0 
typedef CGAL::Simple_cartesian<double> K;
typedef K::Point_2 Point_d;
typedef CGAL::Search_traits_2<K> TreeTraits;
typedef CGAL::Orthogonal_k_neighbor_search<TreeTraits> Neighbor_search;
typedef Neighbor_search::Tree Tree;


int main() 
{  
  const unsigned int N = 1;
  std::list<Point_d> points;
  points.push_back(Point_d(0,0));
  Tree tree(points.begin(), points.end());
  Point_d query(0,0);
  // Initialize the search structure, and search all N points  
   Neighbor_search search(tree, query, N);
   // report the N nearest neighbors and their distance  // This should sort all N points by increasing distance from origin  
    for(Neighbor_search::iterator it = search.begin(); it != search.end(); ++it)
        {    
        std::cout << it->first << " "<< std::sqrt(it->second) << std::endl;
        }  

    return 0;
}
#endif

END_GROUND_DETECTION_NAMESPACE