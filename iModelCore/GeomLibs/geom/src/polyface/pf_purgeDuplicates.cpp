/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include <algorithm>
#include <numeric>
#include <random>

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

// clear all index arrays in the polyface
static void clearIndexArrays(PolyfaceHeaderR mesh)
    {
    mesh.PointIndex().clear();
    mesh.ParamIndex().clear();
    mesh.NormalIndex().clear();
    mesh.ColorIndex().clear();
    mesh.FaceIndex().clear();
    if (mesh.AuxData().IsValid())
        mesh.AuxData()->GetIndices().clear();
    }

// copy indices for all active data for the current face
static void appendVisitorIndices(PolyfaceHeaderR dest, PolyfaceHeaderCR source, PolyfaceVisitorCR visitor, size_t numWrapToIgnore = 0)
    {
    size_t readIndex = visitor.GetReadIndex();
    size_t numIndices = visitor.NumEdgesThisFace() - numWrapToIgnore;
    dest.PointIndex().AddAndTerminate(source.GetPointIndexCP() + readIndex, numIndices);
    if (dest.ParamIndex().Active())
        dest.ParamIndex().AddAndTerminate(source.GetParamIndexCP() + readIndex, numIndices);
    if (dest.NormalIndex().Active())
        dest.NormalIndex().AddAndTerminate(source.GetNormalIndexCP() + readIndex, numIndices);
    if (dest.ColorIndex().Active())
        dest.ColorIndex().AddAndTerminate(source.GetColorIndexCP() + readIndex, numIndices);
    if (dest.FaceIndex().Active())
        dest.FaceIndex().AddAndTerminate(source.GetFaceIndexCP() + readIndex, numIndices);
    if (dest.AuxData().IsValid())
        {
        auto destAuxIndices = dest.AuxData()->GetIndices();
        auto srcAuxIndices = visitor.GetClientAuxIndexCP();
        destAuxIndices.insert(destAuxIndices.end(), srcAuxIndices, srcAuxIndices + numIndices);
        dest.AuxData()->AddIndexTerminator();
        }
    }

PolyfaceHeaderPtr PolyfaceHeader::CloneWithDegenerateFacetsRemoved() const
    {
    auto cloneFiltered = Clone();
    clearIndexArrays(*cloneFiltered);

    for (auto visitor = PolyfaceVisitor::Attach(*this, true); visitor->AdvanceToNextFace(); )
        {
        size_t n = visitor->NumEdgesThisFace();
        auto pFaceIndices = visitor->GetClientPointIndexCP();   // zero based

        // reduce edge count by wrapped edges, e.g., 1231, 12312, 123123 -> 123
        size_t numWrap = 0;
        for (size_t iWrapStart = n - 1; iWrapStart > 0; --iWrapStart)
            {
            if (pFaceIndices[0] == pFaceIndices[iWrapStart])
                {
                bool haveWrap = true;
                for (size_t i = 1; iWrapStart + i < n && haveWrap; ++i)
                    haveWrap = pFaceIndices[i] == pFaceIndices[iWrapStart + i];
                if (haveWrap)
                    numWrap = n - iWrapStart;
                break;
                }
            }
        n -= numWrap;
        if (n < 3)
            continue;

        auto i0 = pFaceIndices[0];
        auto i1 = pFaceIndices[1];
        auto i2 = pFaceIndices[2];
        if (n == 3)
            {
            // look for a single degenerate edge
            if (i0 == i1 || i0 == i2 || i1 == i2)
                continue;
            }
        else if (n == 4)
            {
            // look for two degenerate edges
            auto i3 = pFaceIndices[3];
            if ((i0 == i1) && (i2 == i3))
                continue;
            if ((i0 == i3) && (i1 == i2))
                continue;
            // or a revisited vertex
            if (i0 == i2 || i1 == i3)
                continue;
            }
        else
            {
            ; // TODO: n > 4
            }

        appendVisitorIndices(*cloneFiltered, *this, *visitor, numWrap);
        }
    return cloneFiltered;
    }

PolyfaceHeaderPtr PolyfaceHeader::CloneWithFacetsInRandomOrder() const
    {
    size_t numFacets = GetNumFacet();
    std::vector<size_t> facetIndices(numFacets);
    std::iota(facetIndices.begin(), facetIndices.end(), 0);
    std::random_device rd;
    std::mt19937 generator(rd());
    std::shuffle(facetIndices.begin(), facetIndices.end(), generator);

    auto shuffledMesh = Clone();
    clearIndexArrays(*shuffledMesh);

    auto leanVisitor = PolyfaceVisitor::Attach(*this, false);
    for (auto const& iFacet : facetIndices)
        {
        leanVisitor->Reset();
        for (size_t i = 0; leanVisitor->AdvanceToNextFace() && i < iFacet; ++i)
            ; // yuck, linear search
        auto fullVisitor = PolyfaceVisitor::Attach(*this, true);
        if (fullVisitor->MoveToFacetByReadIndex(leanVisitor->GetReadIndex()))
            appendVisitorIndices(*shuffledMesh, *this, *fullVisitor);
        }
    return shuffledMesh;
    }

PolyfaceHeaderPtr PolyfaceHeader::CloneWithConsistentlyOrientedFacets(DVec3dCP surfaceNormal) const
    {
    auto newMesh = PolyfaceHeader::CreateVariableSizeIndexed();

    // use tight tolerances: we assume the input mesh already shares vertices
    MTGFacets* facets = jmdlMTGFacets_grab();
    bvector<MTGNodeId> meshVertexToNodeId;  // needed to preserve visible edges!
    if (PolyfaceToMTG(facets, &meshVertexToNodeId, nullptr, *this, true, Angle::SmallAngle(), Angle::SmallAngle(), 1))
        AddMTGFacetsToIndexedPolyface(facets, *newMesh);
    jmdlMTGFacets_drop(facets);
    if (!newMesh->HasFacets())
        return nullptr;
    newMesh->ReverseIndicesWithTest(surfaceNormal);
    return newMesh;
    }

END_BENTLEY_GEOMETRY_NAMESPACE
