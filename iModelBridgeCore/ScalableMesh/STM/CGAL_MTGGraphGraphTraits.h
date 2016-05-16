/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/CGAL_MTGGraphGraphTraits.h $
|    $RCSfile: MTGGraphGraphTraits.h,v $
|   $Revision: 1.0 $
|       $Date: 2015/11/30 15:09:23 $
|     $Author: Elenie.Godzaridis $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
// This file contains glue code to use CGAL algorithms on BentleyGeometry types. It follows the traits pattern; see following external links for additional information.
// [http://doc.cgal.org/latest/Manual/devman_traits_classes.html][http://www.cs.fsu.edu/~lacher/boost_1_32_0/libs/graph/doc/graph_traits.html]
// The functions used to implement the graph_traits are specializations of global functions.
#pragma once
#include <functional>

#pragma warning(disable:4067) //for compiling boost on vs2015
#include <boost/config.hpp>
#include <boost/iterator/iterator_adaptor.hpp>
#include <boost/iterator/transform_iterator.hpp>
#include <boost/type_traits/remove_const.hpp>

#include <boost/graph/graph_traits.hpp>
#include <boost/graph/properties.hpp>
#include <CGAL/boost/graph/properties.h>
#include <ScalableMesh\ScalableMeshDefs.h>
#include <Mtg/MtgStructs.h>
#include "ScalableMesh\ScalableMeshGraph.h"
#include <boost/strong_typedef.hpp>
#include <CGAL/Iterator_range.h>
#include "CGAL_BentleyKernel.h"


BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

BOOST_STRONG_TYPEDEF(int32_t, vertex_id_t);
BOOST_STRONG_TYPEDEF(MTGNodeId, edge_id_t);
BOOST_STRONG_TYPEDEF(MTGNodeId, halfedge_id_t);


struct GraphWithGeometryInfo
    {
    MTGGraph* graphP;
    DPoint3d* ptArray;
    std::vector<CGAL::Simple_cartesian<double>::Point_3> CGALArray;
    size_t arraySize;
    uint64_t id;
    std::map<int32_t, MTGNodeId> halfedge_target_map;
    std::vector<int32_t> edge_ids;

    GraphWithGeometryInfo() : graphP(nullptr), ptArray(nullptr), arraySize(0) {};
    GraphWithGeometryInfo(MTGGraph* graph, DPoint3d* pts, size_t nPts) : graphP(graph), ptArray(pts), arraySize(nPts) 
        {
        CGALArray.resize(arraySize);
        for (size_t i = 0; i < nPts; ++i)
            {
            CGALArray[i] = CGAL::Simple_cartesian<double>::Point_3(ptArray[i].x, ptArray[i].y, ptArray[i].z);
            }
        MakeEdgeIDs();
        };
    void MakeEdgeIDs()
        {
        edge_ids.resize(graphP->GetNodeIdCount());
        size_t idx = 0;
        MTGMask m = graphP->GrabMask();
        MTGARRAY_SET_LOOP(edgeID, graphP)
            {
            if (graphP->GetMaskAt(edgeID, m)) continue;
            if (edgeID < graphP->EdgeMate(edgeID))
                {
                edge_ids[edgeID] = (int)idx;
                edge_ids[graphP->EdgeMate(edgeID)] = (int)idx + 1;
                }
            else
                {
                edge_ids[edgeID] = (int)idx + 1;
                edge_ids[graphP->EdgeMate(edgeID)] = (int)idx;
                }
            graphP->SetMaskAroundEdge(edgeID, m);
            idx += 2;
            int vtx = -1;
            graphP->TryGetLabel(graphP->EdgeMate(edgeID), 0, vtx);
            halfedge_target_map[vtx] = edgeID;
            graphP->TryGetLabel(edgeID, 0, vtx);
            halfedge_target_map[vtx] = graphP->EdgeMate(edgeID);
            }
        MTGARRAY_END_SET_LOOP(edgeID, graphP)

            graphP->ClearMask(m);
        graphP->DropMask(m);
        }
    };

class Out_edge_iterator;
class MTG_edge_iterator;
class MTG_halfedge_iterator;
class MTG_vertex_iterator;
END_BENTLEY_SCALABLEMESH_NAMESPACE
USING_NAMESPACE_BENTLEY_SCALABLEMESH

namespace boost
    {


    struct MTGTraversalCategory : public virtual boost::bidirectional_graph_tag,
        public virtual boost::vertex_list_graph_tag,
        public virtual boost::edge_list_graph_tag
        {};

    template <>
    struct graph_traits < GraphWithGeometryInfo >
        {
        //typedef vertex_id_t vertex_descriptor;
        typedef edge_id_t edge_descriptor;
        struct halfedge_descriptor
            {
            halfedge_id_t edge_desc;
            halfedge_descriptor() : edge_desc(MTG_NULL_NODEID) {}
            halfedge_descriptor(MTGNodeId id) : edge_desc((halfedge_id_t)id) {}
            halfedge_descriptor(halfedge_id_t id) : edge_desc(id) {}

            explicit operator halfedge_id_t() const { return edge_desc; }
            operator MTGNodeId() const { return (MTGNodeId)edge_desc; }
            };

        struct vertex_descriptor
            {
            vertex_id_t v_desc;
            vertex_descriptor() : v_desc(-1) {}
            vertex_descriptor(int32_t id) : v_desc((vertex_id_t)id) {}
            vertex_descriptor(vertex_id_t id) : v_desc(id) {}

            explicit operator vertex_id_t() const { return v_desc; }
            operator int32_t() const { return (int32_t)v_desc; }
            };
        typedef std::array<vertex_descriptor, 3> face_descriptor;

        typedef DPoint3d vertex_property_type;

        typedef Out_edge_iterator out_edge_iterator;
        typedef MTG_edge_iterator edge_iterator;
        typedef MTG_halfedge_iterator halfedge_iterator;
        typedef MTG_vertex_iterator vertex_iterator;

        typedef boost::directed_tag directed_category;
        typedef boost::disallow_parallel_edge_tag edge_parallel_category;
        typedef MTGTraversalCategory traversal_category;
        typedef size_t vertices_size_type;
        typedef size_t edges_size_type;
        typedef size_t degree_size_type;

        static vertex_descriptor   null_vertex() { return (vertex_descriptor)-1; }
        static face_descriptor     null_face() { return face_descriptor(); }
        static halfedge_descriptor     null_halfedge() { return (halfedge_descriptor)MTG_NULL_NODEID; }

        static bool log;
        };
    };


BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

class GraphPointMap
    {
    public:
        typedef boost::read_write_property_map_tag category;
        typedef CGAL::Simple_cartesian<double>::Point_3             value_type;
        typedef const CGAL::Simple_cartesian<double>::Point_3&      reference;

        typedef boost::graph_traits< GraphWithGeometryInfo >::vertex_descriptor key_type;

        GraphPointMap()
            : sm_(NULL)
            {}

        GraphPointMap(const GraphWithGeometryInfo& sm)
            : sm_(&sm)
            {}

        GraphPointMap(const GraphPointMap& pm)
            : sm_(pm.sm_)
            {}

        value_type operator[](key_type v)
            {
            if (v <= 0 || v > sm_->arraySize) return CGAL::Simple_cartesian<double>::Point_3(DBL_MAX, DBL_MAX, DBL_MAX);
            return sm_->CGALArray[v - 1];

            }

        inline friend reference get(const GraphPointMap& pm, key_type v)
            {
            if (v <= 0 || v > pm.sm_->arraySize) return   pm.sm_->CGALArray[0];
            return  pm.sm_->CGALArray[v - 1];

            }

        inline friend void put(const GraphPointMap& pm, key_type v, const value_type& p)
            {
            if (v <= 0 || v > pm.sm_->arraySize) return;
            const_cast<GraphWithGeometryInfo&>(*pm.sm_).ptArray[v - 1] = DPoint3d::From(p.x(), p.y(), p.z());
            const_cast<GraphWithGeometryInfo&>(*pm.sm_).CGALArray[v - 1] = p;
            }

    private:
        const GraphWithGeometryInfo* sm_;
    };

class IdentityMap
    {
    public:
        typedef boost::read_write_property_map_tag category;
        typedef int32_t             value_type;
        typedef const int32_t&      reference;

        typedef int32_t key_type;

        IdentityMap()
            {}


        value_type operator[](const key_type v) const
            {
            return v;
            }

        inline friend reference get(const IdentityMap& pm, key_type v)
            {
            const_cast<IdentityMap&>(pm).val = v;
            return pm.val;
            }

        inline friend void put(const IdentityMap& pm, key_type v, const value_type& p)
            {
            }
    private:
        value_type val;
    };


class EdgeMap
    {
    public:
        typedef boost::readable_property_map_tag category;
        typedef int32_t             value_type;
        typedef const int32_t&      reference;

        typedef MTGNodeId key_type;

        EdgeMap()
            {}

        EdgeMap(const GraphWithGeometryInfo* g) : graph(g)
            {}


        value_type operator[](const key_type v) const
            {
            return graph->edge_ids[v];
            }

    private:
        value_type val;
       const GraphWithGeometryInfo* graph;
    };

class Out_edge_iterator
    : public boost::iterator_facade <
    Out_edge_iterator                                // Derived
    , MTGNodeId  // Value
    , std::bidirectional_iterator_tag                       // CategoryOrTraversal,
    , MTGNodeId
    , MTGNodeId  // Reference
    >
    {
    public:
        Out_edge_iterator()
            : Out_edge_iterator::base(MTG_NULL_NODEID) {}

        explicit Out_edge_iterator(MTGGraph* graph, MTGNodeId node)
            : base(node), graphP(graph)
            {}

    private:
        MTGGraph* graphP;
        MTGNodeId base;
        friend class boost::iterator_core_access;
        Out_edge_iterator::reference
            dereference() const
            {
            return this->base;
            }

        bool equal(const Out_edge_iterator& other) const
            {
            return base == other.base;
            }

        void increment() { base = graphP->VSucc(base); }
        void decrement() { base = graphP->VPred(base); }

    };

class MTG_edge_iterator
    : public boost::iterator_facade <
    MTG_edge_iterator                                // Derived
    , edge_id_t  // Value
    , std::bidirectional_iterator_tag                       // CategoryOrTraversal,
    , edge_id_t
    , edge_id_t  // Reference
    >
    {
    public:
        MTG_edge_iterator()
            : MTG_edge_iterator::base(MTG_NULL_NODEID) {}

        explicit MTG_edge_iterator(MTGGraph* graph, MTGNodeId node)
            : base(node), graphP(graph)
            {}

    private:
        MTGGraph* graphP;
        MTGNodeId base;
        friend class boost::iterator_core_access;
        MTG_edge_iterator::reference
            dereference() const
            {
            return (edge_id_t)this->base;
            }

        bool equal(const MTG_edge_iterator& other) const
            {
            return base == other.base;
            }

        void increment()
            {
            do
                {
                base++;
                }
            while (base < graphP->GetNodeIdCount() && (!graphP->IsValidNodeId(base) || base > graphP->EdgeMate(base)));
                if (base >= graphP->GetNodeIdCount()) base = MTG_NULL_NODEID;
            }
        void decrement()
            {
            do
                {
                base--;
                }
            while (base >= 0 && (!graphP->IsValidNodeId(base) || base > graphP->EdgeMate(base)));
                if (base < 0) base = 0;
            }

    };

class MTG_halfedge_iterator
    : public boost::iterator_facade <
    MTG_halfedge_iterator                                // Derived
    , halfedge_id_t  // Value
    , std::bidirectional_iterator_tag                       // CategoryOrTraversal,
    , halfedge_id_t
    , halfedge_id_t  // Reference
    >
    {
    public:
        MTG_halfedge_iterator()
            : MTG_halfedge_iterator::base(MTG_NULL_NODEID) {}

        explicit MTG_halfedge_iterator(MTGGraph* graph, MTGNodeId node)
            : base(node), graphP(graph)
            {}

    private:
        MTGGraph* graphP;
        MTGNodeId base;
        friend class boost::iterator_core_access;
        MTG_halfedge_iterator::reference
            dereference() const
            {
            return (halfedge_id_t)this->base;
            }

        bool equal(const MTG_halfedge_iterator& other) const
            {
            return base == other.base;
            }

        void increment()
            {
            do
                {
                base++;
                }
            while (base < graphP->GetNodeIdCount() && !graphP->IsValidNodeId(base));
                if (base >= graphP->GetNodeIdCount()) base = MTG_NULL_NODEID;
            }
        void decrement()
            {
            do
                {
                base--;
                }
            while (base >= 0 && !graphP->IsValidNodeId(base));
                if (base < 0) base = 0;
            }

    };

class MTG_vertex_iterator
    : public boost::iterator_facade <
    MTG_vertex_iterator                                // Derived
    , vertex_id_t  // Value
    , std::bidirectional_iterator_tag                       // CategoryOrTraversal,
    , vertex_id_t
    , vertex_id_t  // Reference
    >
    {
    public:
        MTG_vertex_iterator()
            : MTG_vertex_iterator::base(0) {}

        explicit MTG_vertex_iterator(const GraphWithGeometryInfo& graph, int32_t vertex)
            : base(vertex), g(graph)
            {}

    private:
        GraphWithGeometryInfo g;
        int32_t base;
        friend class boost::iterator_core_access;
        MTG_vertex_iterator::reference
            dereference() const
            {
            return (vertex_id_t)this->base;
            }

        bool equal(const MTG_vertex_iterator& other) const
            {
            return base == other.base;
            }

        void increment()
            {
            base++;
            if (base > g.arraySize) base = 0;
            }
        void decrement()
            {
            base--;
            if (base <= 0) base = 0;
            }

    };
END_BENTLEY_SCALABLEMESH_NAMESPACE
    namespace boost
        {
        template <>
        struct property_map<GraphWithGeometryInfo, boost::vertex_point_t >
            {
            typedef  GraphPointMap type;
            typedef type const_type;
            };

        template <>
        struct property_map<GraphWithGeometryInfo, boost::halfedge_external_index_t >
            {
            typedef  EdgeMap type;
            typedef type const_type;
            };

        template <>
        struct property_map<GraphWithGeometryInfo, boost::vertex_external_index_t >
            {
            typedef  IdentityMap type;
            typedef type const_type;
            };


        GraphPointMap
            get(boost::vertex_point_t, const GraphWithGeometryInfo& g)
            {
            return GraphPointMap(g);
            }

        EdgeMap
            get(boost::halfedge_external_index_t, const GraphWithGeometryInfo& g)
            {
            return EdgeMap(&g);
            }

        void
            put(boost::vertex_point_t p, GraphWithGeometryInfo& g,
            boost::graph_traits< GraphWithGeometryInfo >::vertex_descriptor vd,
            const CGAL::Simple_cartesian<double>::Point_3& point)
            {
            put(get(p, g), vd, point);
            }

        
        boost::property_map< GraphWithGeometryInfo, boost::halfedge_external_index_t >::type
            get(boost::halfedge_external_index_t, GraphWithGeometryInfo & graph)
            {
            return boost::property_map< GraphWithGeometryInfo, boost::halfedge_external_index_t >::type(&graph);
            }

        boost::property_map< GraphWithGeometryInfo, boost::vertex_external_index_t >::type
            get(boost::vertex_external_index_t, GraphWithGeometryInfo& graph)
            {
            return  boost::property_map< GraphWithGeometryInfo, boost::vertex_external_index_t >::type();
            }
    }

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

typedef boost::graph_traits< GraphWithGeometryInfo > traits;


void log(bool activate,
    const GraphWithGeometryInfo & g)
    {
    traits::log = activate;
    }

//=======================================================================================
// @description            Returns identifier of the origin vertex from this edge.             
// @bsimethod                                                   Elenie.Godzaridis 12/15
//=======================================================================================
traits::vertex_descriptor
source(
traits::edge_descriptor e,
const GraphWithGeometryInfo& g)
    {
    int32_t vtx = -1;
    if (!g.graphP->IsValidNodeId(e)) return (traits::vertex_descriptor)vtx;
    g.graphP->TryGetLabel(e, 0, vtx);
    return (traits::vertex_descriptor)vtx;
    }

//=======================================================================================
// @description            Returns identifier of the target vertex from this edge.             
// @bsimethod                                                   Elenie.Godzaridis 12/15
//=======================================================================================
traits::vertex_descriptor
target(
traits::edge_descriptor e,
const GraphWithGeometryInfo & g)
    {
    int32_t vtx = -1;
    if (!g.graphP->IsValidNodeId(e)) return (traits::vertex_descriptor)vtx;
    g.graphP->TryGetLabel(g.graphP->EdgeMate(e), 0, vtx);
    if (traits::log)
        {
/*        std::ofstream f;
        f.open("E:\\output\\cgal.log", std::ios_base::app);
        f << " getting target for " + std::to_string(e) + " is " + std::to_string(vtx) << std::endl;
        f << " mate is edge " + std::to_string(g.graphP->EdgeMate(e)) << std::endl;
        f << " next is edge " + std::to_string(g.graphP->FSucc(e)) << std::endl;
        f.close();*/
        }
    return (traits::vertex_descriptor)vtx;
    }

//=======================================================================================
// @description            Returns pair of edge iterators (begin() and end()) over the edges
//                          of this graph
// @bsimethod                                                   Elenie.Godzaridis 12/15
//=======================================================================================
inline std::pair<
    traits::edge_iterator,
    traits::edge_iterator >
    edges(
    const GraphWithGeometryInfo& g)
    {
    typedef traits::edge_iterator Iter;

    MTGNodeId node = 0;
    while (!g.graphP->IsValidNodeId(node) && node < g.graphP->GetNodeIdCount()) ++node;
    if (node >= g.graphP->GetNodeIdCount()) node = MTG_NULL_NODEID;
    return std::make_pair(Iter(g.graphP, node), Iter(g.graphP, MTG_NULL_NODEID));
    }

//=======================================================================================
// @description            Returns identifier of the origin vertex from this halfedge.             
// @bsimethod                                                   Elenie.Godzaridis 12/15
//=======================================================================================
traits::vertex_descriptor
source(
traits::halfedge_descriptor e,
const GraphWithGeometryInfo& g)
    {
    int32_t vtx = -1;
    if (!g.graphP->IsValidNodeId(e)) return (traits::vertex_descriptor)vtx;
    g.graphP->TryGetLabel(e, 0, vtx);
    return (traits::vertex_descriptor)vtx;
    }

//=======================================================================================
// @description            Returns identifier of the target vertex from this halfedge.             
// @bsimethod                                                   Elenie.Godzaridis 12/15
//=======================================================================================
traits::vertex_descriptor
target(
traits::halfedge_descriptor e,
const GraphWithGeometryInfo & g)
    {
    int32_t vtx = -1;
    if (!g.graphP->IsValidNodeId(e)) return (traits::vertex_descriptor)vtx;
    g.graphP->TryGetLabel(g.graphP->EdgeMate(e), 0, vtx);
    return (traits::vertex_descriptor)vtx;
    }

//=======================================================================================
// @description            Set this halfedge's target to a new vertex. Changes also all the
//                         halfedges sharing the same target.
// @bsimethod                                                   Elenie.Godzaridis 12/15
//=======================================================================================
void
set_target(
traits::halfedge_descriptor e,
traits::vertex_descriptor v,
const GraphWithGeometryInfo & g)
    {
    g.graphP->TrySetLabel(g.graphP->EdgeMate(e), 0, v);
    }

void
set_halfedge (
traits::vertex_descriptor v,
traits::halfedge_descriptor e,
const GraphWithGeometryInfo & g)
    {

    }

void
set_halfedge(
traits::face_descriptor f,
traits::halfedge_descriptor e,
const GraphWithGeometryInfo & g)
    {

    }

//=======================================================================================
// @description            Sets identifier of face incident to edge.
//                         MTGGraph does not use separate face identifiers, so this
//                         does nothing here, but it is needed in the CGAL algorithms.
// @bsimethod                                                   Elenie.Godzaridis 12/15
//=======================================================================================
void
set_face(
traits::halfedge_descriptor e,
traits::face_descriptor f,
const GraphWithGeometryInfo & g)
    {

    }

//=======================================================================================
// @description            Returns iterator over all halfedges in the graph.             
// @bsimethod                                                   Elenie.Godzaridis 12/15
//=======================================================================================
CGAL::Iterator_range<
    traits::halfedge_iterator>
    halfedges(
    const GraphWithGeometryInfo& g)
    {
    typedef traits::halfedge_iterator Iter;

    MTGNodeId node = 0;
    while (!g.graphP->IsValidNodeId(node) && node < g.graphP->GetNodeIdCount()) ++node;
    if (node >= g.graphP->GetNodeIdCount()) node = MTG_NULL_NODEID;
    return CGAL::make_range(Iter(g.graphP, node), Iter(g.graphP, MTG_NULL_NODEID));
    }

//=======================================================================================
// @description            Returns iterator over all vertices in the graph.             
// @bsimethod                                                   Elenie.Godzaridis 12/15
//=======================================================================================
inline std::pair<
    traits::vertex_iterator,
    traits::vertex_iterator >
    vertices(
    const GraphWithGeometryInfo& g)
    {
    typedef traits::vertex_iterator Iter;

    return std::make_pair(Iter(g, 1), Iter(g, 0));
    }

//=======================================================================================
// @bsimethod                                                   Elenie.Godzaridis 12/15
//=======================================================================================
traits::vertices_size_type num_vertices(const GraphWithGeometryInfo& g)
    {
    return g.arraySize;
    }

//=======================================================================================
// @bsimethod                                                   Elenie.Godzaridis 12/15
//=======================================================================================
traits::edges_size_type num_edges(const GraphWithGeometryInfo& g)
    {
    return g.graphP->GetActiveNodeCount() / 2;
    }


//=======================================================================================
// @description            Adds a pair of halfedges between the vertices as a free component,
//                        not modifying the rest of the graph. Returns the forward(v1->v2) halfedge.         
// @bsimethod                                                   Elenie.Godzaridis 12/15
//=======================================================================================
traits::edge_descriptor
add_edge(traits::vertex_descriptor v1,
traits::vertex_descriptor v2,
GraphWithGeometryInfo& g)
    {
    MTGNodeId id1, id2;
    g.graphP->CreateEdge(id1, id2);
    g.graphP->TrySetLabel(id1, 0, v1);
    g.graphP->TrySetLabel(id2, 0, v2);
    return (traits::edge_descriptor)id1;
    }


//=======================================================================================
// @description            Drops a pair of halfedges from the graph.        
// @bsimethod                                                   Elenie.Godzaridis 12/15
//=======================================================================================
void
remove_edge(traits::edge_descriptor eiter,
GraphWithGeometryInfo& g)
    {
    std::ofstream f;
   /* f.open("E:\\output\\cgal.log", std::ios_base::app);
    f << " dropping edge for " + std::to_string(eiter) << std::endl;
    f << " mate is edge " + std::to_string(g.graphP->EdgeMate(eiter)) << std::endl;
    f << " VERTEX LOOP FOR " + std::to_string(eiter) << std::endl;
    for (MTGNodeId node = g.graphP->VSucc(eiter); node != eiter; node = g.graphP->VSucc(node))
        {
        f << " NODE " + std::to_string(node) << std::endl;
        }
    f << " VERTEX LOOP FOR " + std::to_string(g.graphP->EdgeMate(eiter)) << std::endl;
    for (MTGNodeId node = g.graphP->VSucc(g.graphP->EdgeMate(eiter)); node != g.graphP->EdgeMate(eiter); node = g.graphP->VSucc(node))
        {
        f << " NODE " + std::to_string(node) << std::endl;
        }
    f.close();*/

    g.graphP->DropEdge(eiter);
   /* f.open("E:\\output\\cgal.log", std::ios_base::app);
    f << " VERTEX LOOP FOR " + std::to_string(vsucc) << std::endl;
    for (MTGNodeId node = g.graphP->VSucc(vsucc); node != vsucc; node = g.graphP->VSucc(node))
        {
        f << " NODE " + std::to_string(node) << std::endl;
        if (!g.graphP->IsValidNodeId(node)) break;
        }
    f << " VERTEX LOOP FOR " + std::to_string(vsucc2) << std::endl;
    for (MTGNodeId node = g.graphP->VSucc(vsucc2); node != vsucc2; node = g.graphP->VSucc(node))
        {
        f << " NODE " + std::to_string(node) << std::endl;
        if (!g.graphP->IsValidNodeId(node)) break;
        }
    f.close();*/
    }

//=======================================================================================
// @description            Returns the face at h. Note that since MTG does not have
//                         unique identifiers for faces, I use the vertex IDs sorted in
//                         ascending order. The outside face is a "null face".
// @bsimethod                                                   Elenie.Godzaridis 12/15
//=======================================================================================
traits::face_descriptor
face(traits::halfedge_descriptor h,
const GraphWithGeometryInfo& g)
    {
    traits::face_descriptor f;
    if (FastCountNodesAroundFace(g.graphP, h) > 3) return traits::null_face();
    f[0] = f[1] = f[2] = 1;
    return f;
    /*
    f[0] = source((traits::edge_descriptor)h, g);
    f[1] = target((traits::edge_descriptor)h, g);
    f[2] = target((traits::edge_descriptor)g.graphP->FSucc(h), g);
    if (f[0] <= 0 || f[0] > g.arraySize || f[1] <= 0 || f[1] > g.arraySize || f[2] <= 0 || f[2] > g.arraySize) return traits::null_face();
    std::sort(f.begin(), f.end());
    return f;*/
    }

traits::halfedge_descriptor
halfedge(traits::vertex_descriptor v,
const GraphWithGeometryInfo& g)
    {
    MTGNodeId node = MTG_NULL_NODEID;
    if (v <= 0 || v > g.arraySize) return (traits::halfedge_descriptor)node;
    auto& mymap = const_cast<GraphWithGeometryInfo&>(g).halfedge_target_map;
    if (mymap.count(v) != 0 && g.graphP->IsValidNodeId(mymap[v])) return mymap[v];
   /* std::ofstream f;
    f.open("E:\\output\\cgal.log", std::ios_base::app);
    f << " MISSED HALFEDGE FOR " + std::to_string(v) << std::endl;
    f.close();*/
    MTGARRAY_SET_LOOP(id, g.graphP)
        if (target((traits::halfedge_descriptor)id, g) == v)
            {
            node = id;
            mymap[v] = node;
            break;
            }
    MTGARRAY_END_SET_LOOP(id, g.graphP)
    return (traits::halfedge_descriptor)node;
    }

//=======================================================================================
// @description            Returns a halfedge of the face f. For us this is equivalent
//                         to a vertex lookup, as we do not store faces.
// @bsimethod                                                   Elenie.Godzaridis 12/15
//=======================================================================================
traits::halfedge_descriptor
halfedge(traits::face_descriptor f,
const GraphWithGeometryInfo& g)
    {
    MTGNodeId node = MTG_NULL_NODEID;
    node = halfedge(f[0], g);
    return (traits::halfedge_descriptor)node;
    }

std::pair<traits::halfedge_descriptor,
    bool>
halfedge(traits::vertex_descriptor v1,
traits::vertex_descriptor v2,
const GraphWithGeometryInfo& g)
    {
    MTGNodeId node = MTG_NULL_NODEID;
    node = halfedge(v1, g);
    if (target((traits::halfedge_descriptor)node, g) == v2) return std::make_pair((traits::halfedge_descriptor)node, g.graphP->IsValidNodeId(node));
    MTGNodeId n = node;
    do
        {
        if (target((traits::halfedge_descriptor)n, g) == v2) break;
        n = g.graphP->VSucc(n);
        }
    while (n != node);
    if (n == node) return std::make_pair((traits::halfedge_descriptor)MTG_NULL_NODEID, false);
    return std::make_pair((traits::halfedge_descriptor)n, g.graphP->IsValidNodeId(n));
    }

traits::halfedge_descriptor
halfedge(traits::edge_descriptor e,
const GraphWithGeometryInfo& g)
    {
    return (traits::halfedge_descriptor)e;
    }


traits::halfedge_descriptor
opposite(traits::halfedge_descriptor e,
const GraphWithGeometryInfo& g)
    {
    if (g.graphP->EdgeMate(e) == MTG_NULL_NODEID || !g.graphP->IsValidNodeId(g.graphP->EdgeMate(e)))
        {
     /*   std::ofstream f;
        f.open("E:\\output\\cgal.log", std::ios_base::app);
        f << " getting mate edge for " + std::to_string(e) << std::endl;
        f << " mate is edge " + std::to_string(g.graphP->EdgeMate(e)) << std::endl;
        f << " next is edge " + std::to_string(g.graphP->FSucc(e)) << std::endl;
        f << " prev is edge " + std::to_string(g.graphP->FPred(e)) << std::endl;
        f.close();*/
        }
    return (traits::halfedge_descriptor)g.graphP->EdgeMate(e);
    }

traits::edge_descriptor
edge(traits::halfedge_descriptor h,
const GraphWithGeometryInfo& g)
    {
    return (traits::edge_descriptor)h;
    }

//=======================================================================================
// @description            Removes vertex "without adjusting anything". We do not wish to
//                          resize the point array too often, so this does nothing.
// @bsimethod                                                   Elenie.Godzaridis 12/15
//=======================================================================================
void
remove_vertex(traits::vertex_descriptor v,
const GraphWithGeometryInfo& g)
    {
    }

//=======================================================================================
// @description            Removes face "without adjusting anything". We do not store
//                          face information, so this does nothing.
// @bsimethod                                                   Elenie.Godzaridis 12/15
//=======================================================================================
void
remove_face(traits::face_descriptor f,
const GraphWithGeometryInfo& g)
    {}

traits::halfedge_descriptor
next(traits::halfedge_descriptor h,
const GraphWithGeometryInfo& g)
    {
    assert(h != MTG_NULL_NODEID && g.graphP->IsValidNodeId(h));
    if (!g.graphP->IsValidNodeId(h) || g.graphP->FSucc(h) == MTG_NULL_NODEID || !g.graphP->IsValidNodeId(g.graphP->FSucc(h)))
        {
  /*      std::ofstream f;
        f.open("E:\\output\\cgal.log", std::ios_base::app);
        f << " getting next edge for " + std::to_string(h) << std::endl;
        f << " mate is edge " + std::to_string(g.graphP->EdgeMate(h)) << std::endl;
        f << " next is edge " + std::to_string(g.graphP->FSucc(h)) << std::endl;
        f << " prev is edge " + std::to_string(g.graphP->FPred(h)) << std::endl;
        f.close();*/
        }
/*     std::ofstream f;
    f.open("E:\\output\\cgal.log", std::ios_base::app);
    f << " getting next edge for " + std::to_string(h) << std::endl;
    f << " mate is edge " + std::to_string(g.graphP->EdgeMate(h)) << std::endl;
    f << " next is edge " + std::to_string(g.graphP->FSucc(h)) << std::endl;
    f.close();*/
    return (traits::halfedge_descriptor)g.graphP->FSucc(h);
    }

//=======================================================================================
// @description            Sets the "next" halfedge of h and the "previous" halfedge of next.
// @bsimethod                                                   Elenie.Godzaridis 12/15
//=======================================================================================
void
set_next(traits::halfedge_descriptor h,
traits::halfedge_descriptor next,
const GraphWithGeometryInfo& g)
    {
    assert(g.graphP->FPred(next) != MTG_NULL_NODEID);
 /*   std::ofstream f;
    f.open("E:\\output\\cgal.log", std::ios_base::app);
    f << " setting next edge for " + std::to_string(h) + " to " + std::to_string(next) << std::endl;
    f << " mate is edge " + std::to_string(g.graphP->EdgeMate(h)) << std::endl;
    f << " next is edge " + std::to_string(g.graphP->FSucc(h)) << std::endl;
    f << " vsucc is edge " + std::to_string(g.graphP->VSucc(g.graphP->FSucc(h))) << std::endl;
    f << " new vsucc is edge " + std::to_string(g.graphP->VSucc(next)) << std::endl;
    f << " VERTEX LOOP FOR " + std::to_string(h) << std::endl;
    for (MTGNodeId node = g.graphP->VSucc(h); node != h; node = g.graphP->VSucc(node))
        {
        f << " NODE " + std::to_string(node) << std::endl;
        if (!g.graphP->IsValidNodeId(node)) break;
        }
    f << " VERTEX LOOP FOR " + std::to_string(g.graphP->EdgeMate(h)) << std::endl;
    for (MTGNodeId node = g.graphP->VSucc(g.graphP->EdgeMate(h)); node != g.graphP->EdgeMate(h); node = g.graphP->VSucc(node))
        {
        f << " NODE " + std::to_string(node) << std::endl;
        if (!g.graphP->IsValidNodeId(node)) break;
        }
    f.close();*/
   /*MTGNodeId oldVSucc = g.graphP->VSucc(g.graphP->FSucc(h));
    MTGNodeId oldFSucc = g.graphP->FSucc(h);
    MTGNodeId oldMate = g.graphP->EdgeMate(h);
    g.graphP->FSucc(h) = next;
    if (g.graphP->EdgeMate(h) != oldMate)
        {
        g.graphP->VSucc(oldFSucc) = g.graphP->VSucc(next);
        g.graphP->VSucc(next) = oldVSucc;
        }*/
    /*
    f.open("E:\\output\\cgal.log", std::ios_base::app);
    f << " mate is edge " + std::to_string(g.graphP->EdgeMate(h)) << std::endl;
    f << " next is edge " + std::to_string(g.graphP->FSucc(h)) << std::endl;
    f << " VERTEX LOOP FOR " + std::to_string(h) << std::endl;
    for (MTGNodeId node = g.graphP->VSucc(h); node != h; node = g.graphP->VSucc(node))
        {
        f << " NODE " + std::to_string(node) << std::endl;
        if (!g.graphP->IsValidNodeId(node)) break;
        }
    f << " VERTEX LOOP FOR " + std::to_string(g.graphP->EdgeMate(h)) << std::endl;
    for (MTGNodeId node = g.graphP->VSucc(g.graphP->EdgeMate(h)); node != g.graphP->EdgeMate(h); node = g.graphP->VSucc(node))
        {
        f << " NODE " + std::to_string(node) << std::endl;
        if (!g.graphP->IsValidNodeId(node)) break;
        }
    f.close();*/
    }

traits::halfedge_descriptor
prev(traits::halfedge_descriptor h,
const GraphWithGeometryInfo& g)
    {
    if (g.graphP->FPred(h) == MTG_NULL_NODEID || !g.graphP->IsValidNodeId(g.graphP->FPred(h)))
        {
 /*           std::ofstream f;
        f.open("E:\\output\\cgal.log", std::ios_base::app);
        f << " getting prev edge for " + std::to_string(h) << std::endl;
        f << " mate is edge " + std::to_string(g.graphP->EdgeMate(h)) << std::endl;
        f << " next is edge " + std::to_string(g.graphP->FSucc(h)) << std::endl;
        f << " prev is edge " + std::to_string(g.graphP->FPred(h)) << std::endl;
        f.close();*/
        }
    return (traits::halfedge_descriptor)g.graphP->FPred(h);
    }

//=======================================================================================
// @description            Sets the "next" halfedge of prev and the "previous" halfedge of h.
// @bsimethod                                                   Elenie.Godzaridis 12/15
//=======================================================================================
void
set_prev(traits::halfedge_descriptor h,
traits::halfedge_descriptor prev,
const GraphWithGeometryInfo& g)
    {
    assert(g.graphP->FPred(h) != MTG_NULL_NODEID);
    //g.graphP->FSucc(prev) = h;
    set_next(prev, h,g);
//    g.graphP->FPred(h) = prev;
    }

//=======================================================================================
// @description            Returns pair of edge iterators (begin() and end()) over the
//                          outgoing edges of a vertex
// @bsimethod                                                   Elenie.Godzaridis 12/15
//=======================================================================================
inline std::pair<
    traits::out_edge_iterator,
    traits::out_edge_iterator >
    out_edges(
    traits::vertex_descriptor u,
    const GraphWithGeometryInfo& g)
    {
    typedef traits::out_edge_iterator Iter;

    MTGNodeId node;
    node = halfedge(u, g);
    return std::make_pair(Iter(g.graphP, g.graphP->IsValidNodeId(node) ?node : MTG_NULL_NODEID), Iter(g.graphP, MTG_NULL_NODEID));
    }

//=======================================================================================
// @bsimethod                                                   Elenie.Godzaridis 12/15
//=======================================================================================
traits::degree_size_type out_degree(traits::vertex_descriptor v,
                                    const GraphWithGeometryInfo& g)
    {
    size_t nOfOutgoingEdges = 0;
    MTGNodeId node = halfedge(v, g);
    if (!g.graphP->IsValidNodeId(node)) return 0;
    MTGNodeId target = node;
    do
        {
        target = g.graphP->VSucc(target);
        nOfOutgoingEdges++;
        }
    while (target != node);
        return nOfOutgoingEdges;
    }

//=======================================================================================
// @bsimethod                                                   Elenie.Godzaridis 12/15
//=======================================================================================
traits::degree_size_type degree(traits::vertex_descriptor v,
                                const GraphWithGeometryInfo& g)
    {
    return 2 * out_degree(v, g);
    }


END_BENTLEY_SCALABLEMESH_NAMESPACE
int s_collapseN = 0;
namespace CGAL
    {

    void print_graph(GraphWithGeometryInfo& graph, boost::graph_traits<GraphWithGeometryInfo>::vertex_descriptor v)
        {
        Utf8String path = "E:\\output\\scmesh\\2016-01-28\\";
        Utf8String str = "onDemand_n";
        str += std::to_string(s_collapseN).c_str();
        PrintGraph(path, str, graph.graphP);
        Utf8String name = "E:\\output\\scmesh\\2016-01-28\\";
        name.append("failures.txt");
        std::ofstream f;
        f.open(name.c_str(), std::ios_base::app);
        f << " TILE FAILED : ID " + std::to_string(graph.id) + " COLLAPSE " + std::to_string(s_collapseN) + " AT VTX "+std::to_string(v);
        f<< std::endl;
        f.close();
        /*str += "_.g";
        void* graphData;
        size_t ct = graph.graphP->WriteToBinaryStream(graphData);
        FILE* graphSaved = fopen((path + str).c_str(), "wb");
        fwrite(&ct, sizeof(size_t), 1, graphSaved);
        fwrite(graphData, 1, ct, graphSaved);
        fclose(graphSaved);*/
        }
    namespace Euler
        {

        bool is_border(boost::graph_traits<GraphWithGeometryInfo>::halfedge_descriptor hd, const GraphWithGeometryInfo& g)
            {
            return FastCountNodesAroundFace(g.graphP, hd) > 3;
            }

        bool RemovePinch(MTGGraph* graphP, MTGNodeId node)
            {
            if (FastCountNodesAroundFace(graphP, node) != 3)
                {
                MTGNodeId nodes[6] = { graphP->FPred(graphP->FPred(node)), graphP->FPred(node), node, graphP->FSucc(node), graphP->FSucc(graphP->FSucc(node)) };
                int labels[6];
                for (size_t i = 0; i < 6; ++i) graphP->TryGetLabel(nodes[i], 0, labels[i]);
                for (size_t i = 0; i + 3 < 6; ++i)
                    {
                    if (labels[i] == labels[i + 3])
                        {
                        graphP->VertexTwist(nodes[i], nodes[i + 3]);
                            {
                           /* Utf8String path1 = "E:\\output\\scmesh\\2016-01-08\\";
                            Utf8String str1 = "vtwist_";
                            str1 += std::to_string(s_collapseN).c_str();
                            PrintGraph(path1, str1, graphP);*/
                            }
                        return true;
                        }
                    }
                }
            return false;
            }

        boost::graph_traits<GraphWithGeometryInfo>::vertex_descriptor
            collapse_edge(boost::graph_traits<GraphWithGeometryInfo>::edge_descriptor v0v1,
            GraphWithGeometryInfo& g)
            {
           /* std::string s;
            s += "collapsing edge..." + std::to_string(v0v1);*/
            boost::graph_traits<GraphWithGeometryInfo>::vertex_descriptor v = source(v0v1, g);
            assert(v != -1);
            if (v == -1) return v;
            int label = -1;
            g.graphP->TryGetLabel(v0v1,2 , label);

            int featureComponentId = -1;
            g.graphP->TryGetLabel(v0v1, 3, featureComponentId);
           /* if (label != -1)
                {
                std::ofstream f;
                f.open("E:\\output\\cgal.log", std::ios_base::app);
                f << " COLLAPSING HALFEDGE " + std::to_string(v0v1) + " vtx " + std::to_string(v) + " to " + std::to_string(target(v0v1, g)) << std::endl;
                f.close();
                }*/
            bool printG = (v == 89 || target(v0v1, g) == 89 || target((boost::graph_traits<GraphWithGeometryInfo>::halfedge_descriptor)g.graphP->FSucc(v0v1), g) == 89 ||
                           target((boost::graph_traits<GraphWithGeometryInfo>::halfedge_descriptor)g.graphP->FSucc(g.graphP->EdgeMate(v0v1)), g) == 89);
           /*printG = printG || (v == 1367 || target(v0v1, g) == 1367 || target((boost::graph_traits<GraphWithGeometryInfo>::halfedge_descriptor)g.graphP->FSucc(v0v1), g) == 1367 ||
                                target((boost::graph_traits<GraphWithGeometryInfo>::halfedge_descriptor)g.graphP->FSucc(g.graphP->EdgeMate(v0v1)), g) == 1367);*/
            printG = printG && traits::log;
            if (printG /*||(label != -1 && g.id == 15)*/)
                {
                Utf8String path1 = "E:\\output\\scmesh\\2016-05-05\\";
                Utf8String str1 = "beforeCollapse_";
                str1 += std::to_string(s_collapseN).c_str();
                str1 += "_";
                str1+=std::to_string(v0v1).c_str();
                str1 += "_";
                str1 += std::to_string(g.id).c_str();
                PrintGraph(path1, str1, g.graphP);
                }
            MTGNodeId v1next = g.graphP->FSucc(v0v1);
           // int v2 = target(v1next, g);
            MTGNodeId v1v0 = g.graphP->EdgeMate(v0v1);
            MTGNodeId v0next = g.graphP->FSucc(v1v0);
            //MTGNodeId startsAtV1 = g.graphP->EdgeMate(g.graphP->FPred(v1v0));
            bvector<MTGNodeId> v1edges;
            MTGARRAY_VERTEX_LOOP(id, g.graphP, v1v0)
                {
                v1edges.push_back(id);
                }
            MTGARRAY_END_VERTEX_LOOP(id, g.graphP, v1v0)
                MTGARRAY_VERTEX_LOOP(id, g.graphP, v0v1)
                {
                v1edges.push_back(id);
                }
            MTGARRAY_END_VERTEX_LOOP(id, g.graphP, v0v1)
            //This first handles the collapse of an isolated triangle. All edges are dropped.
            if (g.graphP->EdgeMate(g.graphP->FSucc(v0next)) == v1next && g.graphP->EdgeMate(g.graphP->FSucc(v1next)) == v0next)
                {
                g.graphP->DropEdge(v0v1);
                g.graphP->DropEdge(v0next);
                g.graphP->DropEdge(v1next);
                g.halfedge_target_map[v] = MTG_NULL_NODEID;
                return v;
                }
            g.graphP->VertexTwist(v0v1, v0next);
            g.graphP->VertexTwist(v1next, v1v0);
            g.graphP->VertexTwist(v1next, v0next);
            if (g.graphP->FSucc(g.graphP->FSucc(v1next)) == v1next)
                {
                MTGNodeId mate = g.graphP->EdgeMate(v1next);
                g.graphP->ExciseSliverFace(v1next);
                MTGNodeId newNode = g.graphP->EdgeMate(mate);
                int id = g.edge_ids[min(mate, newNode)];
                if (id % 2 == 1)
                    {
                    g.edge_ids[min(mate, newNode)] = id - 1;
                    g.edge_ids[max(mate, newNode)] = id;
                    }
                else g.edge_ids[max(mate, newNode)] = id+1;
                }
            if (g.graphP->FSucc(g.graphP->FSucc(v0next)) == v0next)
                {
                MTGNodeId mate = g.graphP->EdgeMate(v0next);
                g.graphP->ExciseSliverFace(v0next);
                MTGNodeId newNode = g.graphP->EdgeMate(mate);
                int id = g.edge_ids[min(mate, newNode)];
                if (id % 2 == 1)
                    {
                    g.edge_ids[min(mate, newNode)] = id - 1;
                    g.edge_ids[max(mate, newNode)] = id;
                    }
                else g.edge_ids[max(mate, newNode)] = id + 1;
                }

            g.graphP->DropEdge(v0v1);
            MTGNodeId startsAtV1 = MTG_NULL_NODEID, endsAtV1 = MTG_NULL_NODEID;
            for (auto& id : v1edges)
                {
                if (!g.graphP->IsValidNodeId(id)) continue;
                g.graphP->TrySetLabel(id, 0, v);
                endsAtV1 = g.graphP->EdgeMate(id);
                g.halfedge_target_map[v] = endsAtV1;
                boost::graph_traits<GraphWithGeometryInfo>::vertex_descriptor vnext = target(id, g);
                g.halfedge_target_map[vnext] = id;
                startsAtV1 = id;
                }
            if (startsAtV1 != MTG_NULL_NODEID && endsAtV1 != MTG_NULL_NODEID)
                {
                MTGNodeId toCollapse = MTG_NULL_NODEID;
                int vEdge;
                g.graphP->TryGetLabel(endsAtV1, 0, vEdge);
                if (vEdge == v)
                    {
                    toCollapse = startsAtV1;
                    g.halfedge_target_map[v] = g.graphP->EdgeMate(g.graphP->VSucc(startsAtV1));
                    }
                for (MTGNodeId vSucc = g.graphP->VSucc(startsAtV1); g.graphP->IsValidNodeId(startsAtV1) && g.graphP->IsValidNodeId(vSucc) && vSucc != startsAtV1; vSucc = g.graphP->VSucc(vSucc))
                    {
                    g.graphP->TrySetLabel(vSucc, 0, v);
                    if (featureComponentId != -1) //if this was the end of a feature, tag end identifier onto incoming feature edges of the same tag.
                        {
                        int existingFID = -1;
                        g.graphP->TryGetLabel(vSucc, 2, existingFID);
                        if (existingFID == label)
                            {
                            g.graphP->TrySetLabel(vSucc, 3, featureComponentId);
                            g.graphP->TrySetLabel(g.graphP->EdgeMate(vSucc), 3, featureComponentId);
                            }
                        }
                  /*  int vtx = -1;
                    g.graphP->TryGetLabel(g.graphP->EdgeMate(vSucc), 0, vtx);
                    if (label != -1 && FastCountNodesAroundFace(g.graphP, vSucc) > 3) g.graphP->TrySetLabel(vSucc, 2, label);*/
                    /*int vEdge;
                    g.graphP->TryGetLabel(g.graphP->EdgeMate(vSucc), 0, vEdge);
                    assert(vEdge != -1);
                    g.halfedge_target_map[vEdge] = vSucc;*/
                    if (vEdge == v) toCollapse = vSucc; //self-loop, should collapse
                    }
                }
            if (printG /*|| (label != -1 && g.id == 15)*/)
                {
                Utf8String path = "E:\\output\\scmesh\\2016-05-05\\";
                Utf8String str = "afterCollapse_";
                str += std::to_string(s_collapseN).c_str();
                str += "_";
                str+=std::to_string(startsAtV1).c_str();
                str += "_";
                str += std::to_string(g.id).c_str();
                PrintGraph(path, str, g.graphP);
                }
            s_collapseN++;

          /*  if (toCollapse != MTG_NULL_NODEID && g.graphP->IsValidNodeId(toCollapse)) // we do not support having multiple edges between 2 vertices or having a single edge start and end on the same vertex. 
                {
                int v2 = collapse_edge((boost::graph_traits<GraphWithGeometryInfo>::edge_descriptor)toCollapse, g);
                if (v2 != -1) v = v2;
                }*/
            return v;
            }

        template<typename EdgeIsConstrainedMap>
        boost::graph_traits<GraphWithGeometryInfo>::vertex_descriptor
            collapse_edge(boost::graph_traits<GraphWithGeometryInfo>::edge_descriptor v0v1,
            GraphWithGeometryInfo& g
            , EdgeIsConstrainedMap Edge_is_constrained_map)
            {
            collapse_edge(v01v1, g);
            }

        }
    }

#undef BOOST_STRONG_TYPEDEF