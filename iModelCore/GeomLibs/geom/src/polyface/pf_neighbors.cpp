/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/polyface/pf_neighbors.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include "pf_halfEdgeArray.h"
BEGIN_BENTLEY_GEOMETRY_NAMESPACE


bool NeighborVector::IsValidReadIndex (size_t index) const
    {
    size_t n = size ();
    return index < n && at(index).m_fsucc < n && at(index).m_edgeMate < n;
    }

// Convention: FSucc, FPred, and EdgeMate implentations check validity.  Others that invoke these do NOT check validity, allowing
// invalid index to percolate back.
size_t NeighborVector::FSucc (size_t index) const {return IsValidReadIndex (index) ? at(index).m_fsucc : Neighbor::InvalidIndex;}
size_t NeighborVector::FPred (size_t index) const {return IsValidReadIndex (index) ? at(index).m_fpred : Neighbor::InvalidIndex;}
size_t NeighborVector::EdgeMate (size_t index) const {return IsValidReadIndex (index) ? at(index).m_edgeMate : Neighbor::InvalidIndex;}
size_t NeighborVector::PointIndex (size_t index) const {return IsValidReadIndex (index) ? at(index).m_pointIndex: Neighbor::InvalidIndex;}
void   NeighborVector::SetPointIndex (size_t index, int value)
    {
    if (IsValidReadIndex (index))
        at(index).m_pointIndex = value;
    }


bool NeighborVector::HasMask (size_t index, Neighbor::MaskType mask) const
    {
    return IsValidReadIndex (index) ? at(index).HasMask (mask) : false;
    }

void NeighborVector::SetMask (size_t index, Neighbor::MaskType mask)
    {
    if (IsValidReadIndex (index))
        at(index).SetMask (mask);
    }


size_t NeighborVector::ManifoldMate (size_t index) const
    {
    size_t mate = EdgeMate (index);
    if (mate != index && index == EdgeMate (mate))
        return mate;
    return Neighbor::InvalidIndex;
    }

size_t NeighborVector::VPred (size_t index) const
    {
    return FSucc (ManifoldMate (index));
    }

size_t NeighborVector::VSucc (size_t index) const
    {
    return ManifoldMate (FPred (index));
    }

bool NeighborVector::IsBoundary (size_t index) const {return IsValidReadIndex (index) && at(index).m_fsucc == index;}
bool NeighborVector::IsManifoldEdge (size_t index) const {return IsValidReadIndex (index) && !at(index).HasMask (s_maskNonManifold);}

void NeighborVector::GetAroundFacet (size_t index0, bvector<size_t> readIndices) const 
    {
    size_t n = size ();
    size_t index = index0;
    size_t count = 0;
    readIndices.clear ();
    // REMARK: Every facet loop should close to its start.  If count reaches n something is badly wrong
    while (index < n && count++ < n)
        {
        readIndices.push_back (index);
        index = FSucc (index);
        if (index == index0)
            break;
        }
    }

void NeighborVector::GetAroundVertex (size_t seedIndex, IndexVectorAndSingleton &data) const
    {
    data.Clear ();
    data.index0 = Neighbor::InvalidIndex;;

    size_t n = size ();

    // do VPred until boundary or return to index0 ...
    size_t index = seedIndex;
    size_t count = 0;

    // REMARK: Every facet loop should close to its start.  If count reaches n something is badly wrong
    while (index < n && count++ < n)
        {
        data.indices.push_back (index);
        index = VPred (index);
        if (index == seedIndex)
            return;     // full loop at interior vertex !!!
        }

    std::reverse (data.indices.begin (), data.indices.end ());

    // do VSucc until boundary ..
    for (index = seedIndex;;)
        {
        auto nextIndex = VSucc (index);
        if (nextIndex == seedIndex)
            break;      // but this cannot happen -- vertex with boundary on one side must have boundary on the other also.
        if (!IsValidReadIndex (nextIndex))
            {
            data.index0 = FPred (index);
            break;
            }
        data.indices.push_back (nextIndex);
        index = nextIndex;
        }
    std::reverse (data.indices.begin (), data.indices.end ());
    }


size_t NeighborVector::CountAroundFacet (size_t index0) const
    {
    size_t n = size ();
    size_t index = index0;
    size_t count = 0;
    // REMARK: Every facet loop should close to its start.  If count reaches n something is badly wrong
    while (index < n && count++ < n)
        {
        index = FSucc (index);
        if (index == index0)
            break;
        }
    return count;
    }

void NeighborVector::CollectVertexLoops (bvector<size_t> *interiorSeeds, bvector<size_t> *boundarySeeds)
    {
    if (nullptr != interiorSeeds)
        interiorSeeds->clear ();

    if (nullptr != boundarySeeds)
        boundarySeeds->clear ();

    Neighbor::MaskType visited = s_maskInternalA;
    ClearMask (visited);
    // First pass: catch exterior loops at their exposed mate end
    // NOTE: This loop must be executed even if boundarySeeds is nullptr.   The interiorSeeds step needs the masks
    // set on the boundary vertices.
    size_t n = size ();
    for (size_t tail = 0; tail < n; tail++)
        {
        if (!IsManifoldEdge (tail))
            {
            size_t vertexSeed = FSucc (tail);
            if (!HasMask (vertexSeed, visited))
                {
                if (boundarySeeds != nullptr)
                    boundarySeeds->push_back (vertexSeed);
                SetMaskAroundManifoldVPredPath (vertexSeed, visited);
                }
            }
        }

    if (nullptr != interiorSeeds)
        {
        // second pass: all unvisited loops must be pure manifold.
        for (size_t vertexSeed = 0; vertexSeed < n; vertexSeed++)
            {
            if (!HasMask (vertexSeed, visited))
                {
                interiorSeeds->push_back (vertexSeed);
                SetMaskAroundManifoldVPredPath (vertexSeed, visited);
                }
            }
        }
    }

void NeighborVector::ClearMask (Neighbor::MaskType mask)
    {
    for (auto &node : *this)
        node.ClearMask (mask);
    }


void NeighborVector::SetMaskAroundManifoldVPredPath (size_t seedIndex, Neighbor::MaskType mask)
    {
    size_t index = seedIndex;
    while (IsValidReadIndex (index))
        {
        at(index).SetMask (mask);
        index = ManifoldMate (index);
        if (index == seedIndex)
            break;
        }
    }

void NeighborVector::SetMaskAroundFacet (size_t seedIndex, Neighbor::MaskType mask)
    {
    size_t index = seedIndex;
    while (IsValidReadIndex (index))
        {
        at(index).SetMask (mask);
        index = FSucc (index);
        if (index == seedIndex)
            break;
        }
    }


 void NeighborVector::SetMask (IndexVectorAndSingleton const &data, Neighbor::MaskType mask, bool setInVector, bool setAtSingleton)
    {
    if (setInVector)
        for (size_t k : data.indices)
            SetMask (k, mask);
    if (setAtSingleton)
        SetMask (data.index0, mask);

    }
 void NeighborVector::SetPointIndex (IndexVectorAndSingleton const &data, int value, bool setInVector, bool setAtSingleton)
    {
    if (setInVector)
        for (size_t k : data.indices)
            SetPointIndex (k, value);
    if (setAtSingleton)
        SetPointIndex (data.index0, value);
    }

END_BENTLEY_GEOMETRY_NAMESPACE
