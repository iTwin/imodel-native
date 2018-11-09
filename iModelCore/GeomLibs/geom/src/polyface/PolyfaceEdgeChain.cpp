/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/polyface/PolyfaceEdgeChain.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include <set>
#include <list>

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2012
+--------------------------------------------------------------------------------------*/
CurveTopologyIdCR PolyfaceEdgeChain::GetId () const        { return m_id; }
int32_t const*    PolyfaceEdgeChain::GetIndexCP() const     { return m_vertexIndices.empty() ? NULL : &m_vertexIndices[0]; }
size_t            PolyfaceEdgeChain::GetIndexCount () const   { return m_vertexIndices.size(); }
                  PolyfaceEdgeChain::PolyfaceEdgeChain ()     {} 
                  PolyfaceEdgeChain::PolyfaceEdgeChain (CurveTopologyIdCR id) : m_id (id) {} 
                  PolyfaceEdgeChain::PolyfaceEdgeChain (CurveTopologyIdCR id, bvector<int32_t>&& indices) : m_id(id), m_vertexIndices(std::move(indices)) { }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2012
+--------------------------------------------------------------------------------------*/
PolyfaceEdgeChain::PolyfaceEdgeChain (CurveTopologyIdCR id, int32_t index0, int32_t index1) : m_id (id)
    {
    m_vertexIndices.push_back (index0);
    m_vertexIndices.push_back (index1);
    }

void PolyfaceEdgeChain::AddIndex(int32_t index)
    {
    if (m_vertexIndices.empty ()
        || index != m_vertexIndices.back ())
        m_vertexIndices.push_back (index);
    }
void PolyfaceEdgeChain::AddZeroBasedIndex (uint32_t index)
    {
    uint32_t index1 = index + 1;
    if (m_vertexIndices.empty ()
        || index1 != m_vertexIndices.back ())
        m_vertexIndices.push_back (index1);
    }

void PolyfaceEdgeChain::AddZeroBasedIndices (bvector<size_t> const &indices)
    {
    for (size_t i = 0; i < indices.size (); i++)
        AddZeroBasedIndex ((uint32_t)indices[i]);
    }

bool PolyfaceEdgeChain::GetXYZ (bvector<DPoint3d> &dest, bvector<DPoint3d> const &source) const
    {
    bool ok = true;
    size_t n = source.size ();
    size_t oldIndex = SIZE_MAX;
    size_t dups = 0;
    for (auto index1 : m_vertexIndices)
        {
        if (index1 == oldIndex)
            {
            dups++;
            }
        else if (index1 > 0 && index1 <= n)
            {
            dest.push_back (source[(size_t)(index1 - 1)]);
            oldIndex = index1;
            }
        else
            ok = false;
        }
    return ok;
    }

/*=================================================================================**//**
* @bsiclass                                                     RayBentley      12/2011
+===============+===============+===============+===============+===============+======*/
struct  OrderedIndexPair
{
    size_t                   m_index0;
    size_t                   m_index1;

            OrderedIndexPair () { }
    bool    operator ==    (OrderedIndexPair const& rhs) const { return m_index0 == rhs.m_index0 && m_index1 == rhs.m_index1; }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/2011
+---------------+---------------+---------------+---------------+---------------+------*/
OrderedIndexPair (size_t index0, size_t index1) 
    {
    // Ignore signs; we just want to pair manifold edges
    if (index0 < index1)
        {
        m_index0 = index0;
        m_index1 = index1;
        }
    else
        {
        m_index0 = index1;
        m_index1 = index0;
        }
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void AddIndex (size_t index)
    {
    if (index < m_index0)
        {
        m_index1 = m_index0;
        m_index0= index;
        }
    else
        {
        m_index1 = index;
        }
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      12/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool operator < (OrderedIndexPair const& rhs) const
    {
    if (m_index0 == rhs.m_index0)
        return m_index1 < rhs.m_index1;
    else 
        return m_index0 < rhs.m_index0;
    }
};  //  OrderedIndexPair


typedef bmap <OrderedIndexPair, OrderedIndexPair>   T_EdgeFaceMap;

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2012
+--------------------------------------------------------------------------------------*/
BentleyStatus PolyfaceHeader::AddEdgeChains (size_t)
    {
    if (0 != GetEdgeChainCount())
        return ERROR;           // Edge chains already exist.
    
    T_EdgeFaceMap       edgeFaceMap;
    PolyfaceVisitorPtr  visitor = PolyfaceVisitor::Attach (*this);

    visitor->SetNumWrap (1);
    for (size_t facetIndex = 0; visitor->AdvanceToNextFace();)
        {
        if (visitor->NumEdgesThisFace() < 3)
            continue;

        facetIndex++;                            // Match erroneous increment in SS3.

        int32_t const* index   = visitor->GetClientPointIndexCP();
        bool    const* visible = visitor->GetVisibleCP();
        for (size_t i=0; i < visitor->NumEdgesThisFace(); i++)
            {
            if (visible[i])
                {
                OrderedIndexPair        thisEdge (index[i], index[i+1]);

                T_EdgeFaceMap::iterator found = edgeFaceMap.find (thisEdge);

                if (found == edgeFaceMap.end())
                    edgeFaceMap[thisEdge] = OrderedIndexPair (facetIndex, facetIndex);
                else
                    found->second.AddIndex (facetIndex);
                }
            }
        facetIndex++;
        }

    

    for (visitor->Reset (); visitor->AdvanceToNextFace();)
        {
        if (visitor->NumEdgesThisFace() < 3)
            continue;

        for (int32_t const* index = visitor->GetClientPointIndexCP(), *end = index + visitor->NumEdgesThisFace(); index < end; index++)
            {
            int32_t         thisIndex = *index, nextIndex = *(index + 1);

            T_EdgeFaceMap::iterator found = edgeFaceMap.find (OrderedIndexPair (thisIndex, nextIndex));

            if (found != edgeFaceMap.end())
                {
                CurveTopologyId         topologyId = (found->second.m_index0 == found->second.m_index1) ? CurveTopologyId::FromMeshEdgeVertices (thisIndex, nextIndex) :
                                                                                                          CurveTopologyId::FromMeshSharedEdge (found->second.m_index0, found->second.m_index1);

                m_edgeChain.push_back (PolyfaceEdgeChain (topologyId, thisIndex + 1, nextIndex + 1));
                edgeFaceMap.erase (found);
                }
            }
        }
    return SUCCESS;
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
PolyfaceEdgeChain::PolyfaceEdgeChain(CurveTopologyIdCR id, bvector<PolyfaceEdge>&& edges) : m_id(id)
    {
    Build(std::move(edges));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
PolyfaceEdgeChain::PolyfaceEdgeChain(CurveTopologyIdCR id, PolyfaceQueryCR polyface) : m_id(id)
    {
    bvector<PolyfaceEdge>   edges;
    for (PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach(polyface); visitor->AdvanceToNextFace(); /**/)
        {
        uint32_t    edgeCount = visitor->NumEdgesThisFace();
        for (size_t i=0; i<edgeCount; i++)
            {
            if (visitor->GetVisibleCP()[i])
                edges.push_back(PolyfaceEdge(visitor->GetClientPointIndexCP()[i]+1, visitor->GetClientPointIndexCP()[(i+1)%edgeCount]+1));
            }
        }
    Build(std::move(edges));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void PolyfaceEdgeChain::Build(bvector<PolyfaceEdge>&& edges)
    {
    struct  EdgeSegment
        {
        PolyfaceEdge        m_edge;
        bool                m_processed;

        EdgeSegment(PolyfaceEdge const& edge) : m_edge (edge), m_processed(false) { }
        };

    struct MapEntry
        {
        uint32_t        m_endIndex;
        EdgeSegment*    m_segment;

        MapEntry() { }
        MapEntry(uint32_t endIndex, EdgeSegment* segment) : m_endIndex(endIndex), m_segment(segment) { }
        };
    
    bvector<EdgeSegment>            edgeSegments;
    std::set<PolyfaceEdge>          edgeSet;

    for (auto& edge : edges)
        {
        auto    insertPair = edgeSet.insert(edge);
        if (insertPair.second)
            edgeSegments.push_back(EdgeSegment(edge));
        }
        
    
    bmultimap <uint32_t, MapEntry>  segmentMap;

    for (auto& edgeSegment : edgeSegments)
        {
        segmentMap.Insert (edgeSegment.m_edge.m_indices[0], MapEntry(edgeSegment.m_edge.m_indices[1], &edgeSegment));
        segmentMap.Insert (edgeSegment.m_edge.m_indices[1], MapEntry(edgeSegment.m_edge.m_indices[0], &edgeSegment));
        }

    for (auto& edgeSegment : edgeSegments)
        {
        if (edgeSegment.m_processed)
            continue;
        
        std::list<uint32_t> indexList;

        indexList.push_back (edgeSegment.m_edge.m_indices[0]);
        indexList.push_back (edgeSegment.m_edge.m_indices[1]);

        edgeSegment.m_processed = true;
        
        bool    linkFound = false;
        
        do
            {
            linkFound = false;
            for (bmultimap <uint32_t, MapEntry>::iterator curr = segmentMap.lower_bound (indexList.back()), end = segmentMap.upper_bound(indexList.back()); !linkFound && curr != end; curr++)
                {
                if (!curr->second.m_segment->m_processed)
                    {
                    linkFound = true;
                    indexList.push_back(curr->second.m_endIndex);
                    curr->second.m_segment->m_processed = true;
                    break;
                    }
                }
            for (bmultimap <uint32_t, MapEntry>::iterator curr = segmentMap.lower_bound (indexList.front()), end = segmentMap.upper_bound(indexList.front()); !linkFound && curr != end; curr++)
                {
                if (!curr->second.m_segment->m_processed)
                    {
                    linkFound = true;
                    indexList.push_front(curr->second.m_endIndex);
                    curr->second.m_segment->m_processed = true;
                    break;
                    }
                }
            } while (linkFound);

        if(!m_vertexIndices.empty() && !indexList.empty())
            m_vertexIndices.push_back(0); // Seperator.
       
        for (auto& index : indexList)
            m_vertexIndices.push_back(index);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
PolyfaceEdge::PolyfaceEdge(uint32_t index0, uint32_t index1)
    {
    if (index0 < index1)
        {
        m_indices[0] = index0;
        m_indices[1] = index1;
        }
    else
        {
        m_indices[0] = index1;
        m_indices[1] = index0;
        }

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool PolyfaceEdge::operator < (PolyfaceEdge const& rhs) const
    {
    return m_indices[0] == rhs.m_indices[0] ? (m_indices[1] < rhs.m_indices[1]) :  (m_indices[0] < rhs.m_indices[0]);
    }


END_BENTLEY_GEOMETRY_NAMESPACE
