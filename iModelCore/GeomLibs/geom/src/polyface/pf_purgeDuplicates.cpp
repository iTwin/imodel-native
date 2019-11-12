/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include <Bentley/bset.h>

#include <Geom/XYZRangeTree.h>
#include <Geom/cluster.h>
#include <Vu/VuApi.h>


BEGIN_BENTLEY_GEOMETRY_NAMESPACE

struct FacetReadIndexAndPointIndices
{
FacetReadIndexAndPointIndices (size_t readIndex)
    {
    m_readIndex = readIndex;
    }
size_t m_readIndex;
bvector<int> m_pointIndex;
bool operator < (FacetReadIndexAndPointIndices const &other) const
    {
    if (m_pointIndex.size () < other.m_pointIndex.size ())
        return true;
    if (m_pointIndex.size () > other.m_pointIndex.size ())
        return false;
    for (size_t i = 0; i < m_pointIndex.size (); i++)
        {
        if (m_pointIndex[i] < other.m_pointIndex[i])
            return true;
        if (m_pointIndex[i] > other.m_pointIndex[i])
            return false;
        }
    return false;
    }
};

PolyfaceHeaderPtr PolyfaceHeader::CloneWithIndexedDuplicatesRemoved () const
    {
    bvector<ptrdiff_t> acceptedReadIndex;
    IdentifyDuplicates (&acceptedReadIndex, &acceptedReadIndex, nullptr, nullptr);
    acceptedReadIndex.push_back (-1);
    bvector<PolyfaceHeaderPtr> newMeshes;
    this->CopyPartitions (acceptedReadIndex, newMeshes);
    if (newMeshes.size () > 0)    // only 1 is possible !!!
        return newMeshes[0];
    return nullptr;
    }

void PolyfaceHeader::IdentifyDuplicates
    (
    bvector<ptrdiff_t> *nonduplicatedFacetReadIndex,
    bvector<ptrdiff_t> *duplicatedFacetFirstReadIndex,
    bvector<ptrdiff_t> *duplicatedFacetAdditionalReadIndex,
    bvector<ptrdiff_t> *baseIndexForAdditionalReadIndex
    ) const
    {
    if (nullptr != nonduplicatedFacetReadIndex)
        nonduplicatedFacetReadIndex->clear ();
    if (nullptr != duplicatedFacetFirstReadIndex)
        duplicatedFacetFirstReadIndex->clear ();
    if (nullptr != duplicatedFacetAdditionalReadIndex)
        duplicatedFacetAdditionalReadIndex->clear ();
    if (nullptr != baseIndexForAdditionalReadIndex)
        baseIndexForAdditionalReadIndex->clear ();


    bvector<FacetReadIndexAndPointIndices> allFacets;
    PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach (*this);
    bvector<int> &pointIndex = visitor->ClientPointIndex ();
    for (visitor->Reset (); visitor->AdvanceToNextFace ();)
        {
        FacetReadIndexAndPointIndices myIndices (visitor->GetReadIndex ());
        for (size_t i = 0; i < pointIndex.size (); i++)
            myIndices.m_pointIndex.push_back (abs (pointIndex[i]));
        std::sort (myIndices.m_pointIndex.begin (), myIndices.m_pointIndex.end ());
        allFacets.push_back (myIndices);
        }
    bvector<ptrdiff_t> uniqueReadIndices;
    std::sort (allFacets.begin (), allFacets.end ());
    for (size_t i1, i0 = 0; i0 < allFacets.size (); i0 = i1)
        {
        i1 = i0 + 1;
        while (i1 < allFacets.size () && !(allFacets[i0] < allFacets[i1]))
            {
            i1++;
            }
        ptrdiff_t n = i1 - i0;
        if (n == 1)
            {
            if (nonduplicatedFacetReadIndex != nullptr)
                nonduplicatedFacetReadIndex->push_back (allFacets[i0].m_readIndex);
            }
        else
            {
            if (duplicatedFacetFirstReadIndex != nullptr)
                duplicatedFacetFirstReadIndex->push_back (allFacets[i0].m_readIndex);
            if (duplicatedFacetAdditionalReadIndex != nullptr)
                {
                for (size_t i = i0 + 1; i < i1; i++)
                    {
                    duplicatedFacetAdditionalReadIndex->push_back (allFacets[i].m_readIndex);
                    if (baseIndexForAdditionalReadIndex != nullptr)
                        baseIndexForAdditionalReadIndex->push_back (allFacets[i0].m_readIndex);
                        
                    }
                }
            }
        }
    }

END_BENTLEY_GEOMETRY_NAMESPACE
