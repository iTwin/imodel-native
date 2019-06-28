#include "ScalableMeshPCH.h"

#ifdef VANCOUVER_API

#undef static_assert //needed because boost uses static_assert and apparently one of the headers we include redefines it
#include "../mki/StaticAnalysisWarningsPush.h"
#pragma warning(disable:4180) //using function for stop predicate
#include "CGALEdgeCollapse.h"
#include "CGAL_MTGGraphGraphTraits.h"
#include <CGAL/algorithm.h> 
#undef round
#include <CGAL/exceptions.h> 
#include <CGAL/squared_distance_3.h> 
// Simplification function
#include <CGAL/Surface_mesh_simplification/edge_collapse.h>
// Stop-condition policy
#include <CGAL/Surface_mesh_simplification/Policies/Edge_collapse/Count_ratio_stop_predicate.h>
#include <CGAL/Surface_mesh_simplification/Policies/Edge_collapse/Edge_length_cost.h>
#include <CGAL/Surface_mesh_simplification/Policies/Edge_collapse/Midpoint_placement.h>
#include <iostream>

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/linestring.hpp>
#include <boost/geometry/geometries/point_xy.hpp>

#include <queue>

#include "../mki/StaticAnalysisWarningsPop.h"

#include "Edits/DifferenceSet.h"

namespace SMS = CGAL::Surface_mesh_simplification;


bool boost::graph_traits<GraphWithGeometryInfo>::log = false;




template<class ECM_>
class Vertex_placement
    {
    public:

        Vertex_placement()
            {}

        template <typename Profile>
        boost::optional<typename Profile::Point> operator()(Profile const& aProfile) const
            {
            return boost::optional<typename Profile::Point>(aProfile.p0());
            }

    };
typedef SMS::Edge_profile<GraphWithGeometryInfo> Profile;
bool stop_predicate(double const&  aCurrentCost
                       , Profile const& //aEdgeProfile 
                       , size_t      //aInitialCount 
                       , size_t      //aCurrentCount 
                       )
    {

    return aCurrentCost>1e-4;

    }


template<class ECM_>
class Preserve25DCost : public SMS::LindstromTurk_cost < ECM_ >
    {
    typedef typename SMS::Edge_profile<ECM_> Profile;
    typedef typename SMS::Edge_profile<ECM_>::Point Point;
    typedef boost::optional<double> result_type;

    public:
    result_type operator() (Profile const& profile, boost::optional<Point> const& placement) const
        {
        if (placement == boost::none)
            return result_type(std::numeric_limits<double>::max());
        MTGNodeId v0v1 = (MTGNodeId)profile.v0_v1();
        MTGNodeId v1v0 = (MTGNodeId)profile.v1_v0();

        std::vector<std::array<MTGNodeId, 3>> facets;
        facets.reserve(20);
        MTGGraph* graphP = profile.surface().graphP;
        for (MTGNodeId id = profile.surface().graphP->VSucc(v0v1); id != v0v1; id = profile.surface().graphP->VSucc(id))
            {
            if (FastCountNodesAroundFace(graphP, id) == 3)
                {
                std::array<MTGNodeId, 3> nodeArray;
                size_t i = 0;
                MTGARRAY_FACE_LOOP(faceID, graphP, id)
                    {
                    nodeArray[i] = faceID;
                    ++i;
                    }
                MTGARRAY_END_FACE_LOOP(faceID, graphP, id)
                    facets.push_back(nodeArray);
                }
            }
        for (MTGNodeId id = profile.surface().graphP->VSucc(v1v0); id != v1v0; id = profile.surface().graphP->VSucc(id))
            {
            if (FastCountNodesAroundFace(graphP, id) == 3)
                {
                std::array<MTGNodeId, 3> nodeArray;
                size_t i = 0;
                MTGARRAY_FACE_LOOP(faceID, graphP, id)
                    {
                    nodeArray[i] = faceID;
                    ++i;
                    }
                MTGARRAY_END_FACE_LOOP(faceID, graphP, id)
                    facets.push_back(nodeArray);
                }
            }
        DVec3d zDir = DVec3d::From(0, 0, 1);
        bvector<DPoint3d> triangle(3);
        for (auto& facet : facets)
            {
            int indices[3];
            graphP->TryGetLabel(facet[0], 0, indices[0]);
            graphP->TryGetLabel(facet[1], 0, indices[1]);
            graphP->TryGetLabel(facet[2], 0, indices[2]);
            for (size_t i = 0; i < 3; ++i)
                {
                if (indices[i] == profile.v0() || indices[i] == profile.v1())
                    {
                    triangle[i] = DPoint3d::From(placement->x(), placement->y(), placement->z());
                    }
                else
                    {
                    triangle[i] = profile.surface().ptArray[indices[i] - 1];
                    }
                triangle[i].z = 0;
                }
            DVec3d areaNormal = PolygonOps::AreaNormal(triangle);
            areaNormal.Normalize();
            if (zDir.AngleTo(areaNormal) > PI / 2) return result_type(std::numeric_limits<double>::max());
            }
        int label = -1;
        profile.surface().graphP->TryGetLabel(v0v1, 2, label);
        if (label != -1)
            {
            auto score = SMS::LindstromTurk_cost < ECM_ >::operator()(profile, placement);
            if (score) score = *score *3.0;
            }
        return SMS::LindstromTurk_cost < ECM_ >::operator()(profile, placement);
        }
    };
//=======================================================================================
// @bsimethod                                                   Elenie.Godzaridis 11/15
//=======================================================================================
bool CGALEdgeCollapse(MTGGraph* inoutMesh, std::vector<DPoint3d>& pts, uint64_t id)
    {
    bool ret = true;
    // This is a stop predicate (defines when the algorithm terminates).  
    SMS::Count_ratio_stop_predicate<GraphWithGeometryInfo> stop(0.25);
    // This the actual call to the simplification algorithm.  
    // The surface mesh and stop conditions are mandatory arguments. 
    // The index maps are needed because the vertices and edges  
    // of this surface mesh lack an "id()" field.  


   RemoveKnotsFromGraph(inoutMesh, pts);
   GraphWithGeometryInfo graphData(inoutMesh, &pts[0], pts.size());
   graphData.id = id;




        auto params = CGAL::parameters::vertex_index_map(get(CGAL::vertex_external_index, graphData)).halfedge_index_map(get(CGAL::halfedge_external_index, graphData)).get_cost(Preserve25DCost<GraphWithGeometryInfo>()).get_placement(SMS::Midpoint_placement<GraphWithGeometryInfo>());
        try
            {
        SMS::edge_collapse(graphData, stop, params);
        }
    catch (CGAL::Assertion_exception e)
        {
        std::string s;
        s = e.message();
        ret = false;
        }


    return ret;
    }

#else
BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

#endif



struct PointWithId
    {
    DPoint3d m_point;
    size_t m_id;
    };

struct PolylineArea
    {
    std::list<PointWithId>::iterator m_listItr;
    mutable double m_area;
    PolylineArea(std::list<PointWithId>::iterator ptPtr) : m_listItr(ptPtr)
        {
        m_area = ComputeArea();
        }
    PolylineArea(const PolylineArea& other) : m_listItr(other.m_listItr), m_area(other.m_area) {}

    double ComputeArea() const
        {
        auto left = m_listItr; --left;
        auto right = m_listItr; ++right;
        double dX0 = left->m_point.x, dY0 = left->m_point.y;
        double dX1 = m_listItr->m_point.x, dY1 = m_listItr->m_point.y;
        double dX2 = right->m_point.x, dY2 = right->m_point.y;
        double dArea = ((dX1 - dX0)*(dY2 - dY0) - (dX2 - dX0)*(dY1 - dY0)) / 2.0;
        return (dArea > 0.0) ? dArea : -dArea;
        }
    bool operator <(PolylineArea const& other) const
        {
        return m_listItr->m_id != other.m_listItr->m_id && m_area < other.m_area;
        }
    };

struct priority_queue_unique : public std::set<PolylineArea>
    {
    void push(const PolylineArea& item)
        {
        std::pair<std::set<PolylineArea>::iterator, bool> ret = insert(item);
        if(!ret.second)
            {
            // update area
            PolylineArea newItem(ret.first->m_listItr);
            erase(ret.first);
            insert(newItem);
            }
        }

    PolylineArea top()
        {
        return *begin();
        }

    void pop()
        {
        erase(begin());
        }
    };

void SimplifyPolylines(bvector<bvector<DPoint3d>>& polylines)
    {
    for(auto& polyline : polylines)
        {
        bool isLoop = polyline.front().IsEqual(polyline.back(), 1.e-5);
        if(isLoop && polyline.size() < 5) continue;
        else if(polyline.size() < 3) continue;

        std::list<PointWithId> polyline2;
        for(size_t i = 0; i < polyline.size(); ++i)
            polyline2.push_back({ polyline[i], i });

        priority_queue_unique pointQueue;

        for(auto it = ++polyline2.begin(); it != --polyline2.end(); ++it)
            {
            pointQueue.push(PolylineArea(it));
            }

        size_t targetNumPointsToKeep = std::max((size_t)(polyline.size() * 0.75), (size_t)(isLoop ? 4 : 2)) - 2; // Must keep at least first and last points
        while(pointQueue.size() > targetNumPointsToKeep)
            {
            auto topItem = pointQueue.top();

            // remove point from polyline and update areas of adjacent points
            auto current = topItem.m_listItr;
            auto previous = current; --previous;
            PolylineArea previousPolylineArea(previous);
            auto next = current; ++next;
            PolylineArea nextPolylineArea(next);
            polyline2.erase(current);

            pointQueue.pop();

            if (previous->m_id > 0) pointQueue.push(previousPolylineArea);
            if (next->m_id < polyline.size() - 1) pointQueue.push(nextPolylineArea);
            }

        polyline.clear();
        for(auto const& pt: polyline2)
            polyline.push_back(pt.m_point);
        }
    // // boost uses Douglas-Peucker algorithm to simplify a polyline. This algorithm requires a tolerance
	// // parameter which is not well suited for our need to target a specified number of points.
    //typedef boost::geometry::model::d2::point_xy<double> xy;
    //for (auto& polyline : polylines)
    //    {
    //    typedef std::map<DPoint3d, double, DPoint3dYXTolerancedSortComparison> MapOfPoints;
    //    MapOfPoints pointElevationMap(DPoint3dYXTolerancedSortComparison(1e-5));
    //    DRange3d ext = DRange3d::From(polyline);
    //    boost::geometry::model::linestring<xy> line;
    //    for(auto&pt : polyline)
    //        {
    //        if(pointElevationMap.count(pt) == 0) pointElevationMap.insert(std::make_pair(pt, pt.z));
    //        line.push_back(xy(pt.x, pt.y));
    //        }
    //
    //        boost::geometry::model::linestring<xy> simplified;
    //    boost::geometry::simplify(line, simplified, 0.5);
    //
    //    polyline.clear();
    //    for(auto& pt : simplified)
    //        {
    //        DPoint3d newPoint = DPoint3d::From(pt.x(), pt.y(), 0);
    //        newPoint = DPoint3d::From(newPoint.x, newPoint.y, pointElevationMap[newPoint]);
    //        polyline.push_back(newPoint);
    //        }
    //    }

    }
END_BENTLEY_SCALABLEMESH_NAMESPACE