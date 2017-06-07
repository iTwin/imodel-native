/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/polyface/PolyfaceEdgeChain.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include <map>

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2012
+--------------------------------------------------------------------------------------*/
CurveTopologyIdCR  PolyfaceEdgeChain::GetId () const                                       { return m_id; }
int32_t const*    PolyfaceEdgeChain::GetIndexCP() const                                   { return m_vertexIndices.empty() ? NULL : &m_vertexIndices[0]; }
size_t          PolyfaceEdgeChain::GetIndexCount () const                               { return m_vertexIndices.size(); }
                PolyfaceEdgeChain::PolyfaceEdgeChain ()                                 {} 
                PolyfaceEdgeChain::PolyfaceEdgeChain (CurveTopologyIdCR id) : m_id (id)    {} 

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2012
+--------------------------------------------------------------------------------------*/
PolyfaceEdgeChain::PolyfaceEdgeChain (CurveTopologyIdCR id, int32_t index0, int32_t index1) : m_id (id)
    {
    m_vertexIndices.push_back (index0);
    m_vertexIndices.push_back (index1);
    }

void PolyfaceEdgeChain::AddIndex(int32_t index) { m_vertexIndices.push_back (index); }
void PolyfaceEdgeChain::AddZeroBasedIndex (uint32_t index) { m_vertexIndices.push_back (index + 1); }

void PolyfaceEdgeChain::AddZeroBasedIndices (bvector<size_t> const &indices)
    {
    for (size_t i = 0; i < indices.size (); i++)
        m_vertexIndices.push_back ((int32_t) (indices[i] + 1));
    }

bool PolyfaceEdgeChain::GetXYZ (bvector<DPoint3d> &dest, bvector<DPoint3d> const &source) const
    {
    bool ok = true;
    size_t n = source.size ();
    for (auto index1 : m_vertexIndices)
        {
        if (index1 > 0 && index1 <= n)
            dest.push_back (source[(size_t)(index1 - 1)]);
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


typedef std::map <OrderedIndexPair, OrderedIndexPair>   T_EdgeFaceMap;

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




END_BENTLEY_GEOMETRY_NAMESPACE
