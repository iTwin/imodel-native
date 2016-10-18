#include "ScalableMeshPCH.h"
#undef static_assert //needed because boost uses static_assert and apparently one of the headers we include redefines it
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

namespace SMS = CGAL::Surface_mesh_simplification;


bool boost::graph_traits<GraphWithGeometryInfo>::log = false;


BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

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

void SimplifyPolylines(bvector<bvector<DPoint3d>>& polylines)
    {
    typedef boost::geometry::model::d2::point_xy<double> xy;
    for (auto& polyline : polylines)
        {
        DRange3d ext = DRange3d::From(polyline);
        boost::geometry::model::linestring<xy> line;
        for (auto&pt : polyline)
            line.push_back(xy(pt.x, pt.y));

            boost::geometry::model::linestring<xy> simplified;
        boost::geometry::simplify(line, simplified, 0.5);

        polyline.clear();
        for (auto& pt : simplified)
            polyline.push_back(DPoint3d::From(pt.x(), pt.y(), (ext.high.z - ext.low.z) / 2));
        }

    }
END_BENTLEY_SCALABLEMESH_NAMESPACE