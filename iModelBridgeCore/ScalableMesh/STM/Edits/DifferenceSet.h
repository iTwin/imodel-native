/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/Edits/DifferenceSet.h $
|    $RCSfile: DifferenceSet.h,v $
|   $Revision: 1.0 $
|       $Date: 2015/09/08 10:27:17 $
|     $Author: Elenie.Godzaridis $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once
#include <Bentley/bvector.h>
#include <Geom/GeomApi.h>
#include <ScalableMesh/IScalableMesh.h>
BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

/* This structure represents a modification made on a mesh. */
struct DifferenceSet
    {
    uint64_t clientID; //Stores diff set ID, if applicable.
    int32_t firstIndex; //this index represents the beginning of the array for added vertices in the "added faces" index list. Previous indexes come from the base mesh.
    bvector<DPoint3d> addedVertices;
    bvector<int32_t> removedVertices;
    bvector<int32_t> addedFaces;
    bvector<int32_t> removedFaces;
    bvector<DPoint2d> addedUvs;
    bvector<int32_t> addedUvIndices;
    bool toggledForID;
    atomic<bool> upToDate;

    DifferenceSet()
        {
        upToDate = true;
        firstIndex = 0;
        toggledForID = true;
        }

    DifferenceSet(const DifferenceSet& d) :clientID(d.clientID), firstIndex(d.firstIndex), addedVertices(d.addedVertices),
        addedFaces(d.addedFaces), removedVertices(d.removedVertices), removedFaces(d.removedFaces), addedUvs(d.addedUvs), addedUvIndices(d.addedUvIndices)
        {
        toggledForID = d.toggledForID;
        if (d.upToDate) upToDate = true;
        else upToDate = false;
        }

    DifferenceSet& operator=(const DifferenceSet& d)
        {
        clientID = d.clientID;
        firstIndex = d.firstIndex;
        addedVertices = d.addedVertices;
        addedFaces = d.addedFaces;
        removedFaces = d.removedFaces;
        removedVertices = d.removedVertices;
        addedUvs = d.addedUvs;
        addedUvIndices = d.addedUvIndices;
        toggledForID = d.toggledForID;
        if (d.upToDate) upToDate = true;
        else upToDate = false;
        return *this;
        }

    bool IsEmpty() const
        {
        return addedFaces.empty() && removedFaces.empty();
        }

    size_t WriteToBinaryStream(void*& serialized);
    void LoadFromBinaryStream(void* serialized, size_t ct);
    void ApplySet(DifferenceSet& d, int firstIndex);
    void ApplyMapped(DifferenceSet& d, const int* idxMap);
    bool ConflictsWith(DifferenceSet& d);
    DifferenceSet MergeSetWith(DifferenceSet& d, const DPoint3d* vertices);
    DifferenceSet MergeSetWith(DifferenceSet& d, const DPoint3d* vertices, bvector<DPoint3d>& clip1, bvector<DPoint3d>& clip2);
    static DifferenceSet  FromPolyfaceSet(bvector<PolyfaceHeaderPtr>& polyMesh, const DPoint3d* vertices, size_t nVertices);
    PolyfaceHeaderPtr ToPolyfaceMesh(const DPoint3d* points, size_t nofPoints);
    static DifferenceSet  FromPolyfaceSet(bvector<PolyfaceHeaderPtr>& polyMesh, map<DPoint3d, int32_t, DPoint3dZYXTolerancedSortComparison> & mapOfPoints, size_t maxPtIdx);
    static DifferenceSet  FromPolyface(PolyfaceHeaderPtr& polyMeshes, map<DPoint3d, int32_t, DPoint3dZYXTolerancedSortComparison> & mapOfPoints, size_t maxPtIdx);
    };
template<typename T>
static bool sort_yx
(
T xA,
T yA,
T xB,
T yB,
double absTol
)
    {
    double tol = absTol;

    double ntol = -tol;
    T dy = yB - yA;

    if (dy > tol)
        return false;
    if (dy < ntol)
        return true;

    T dx = xB - xA;

    if (dx > tol)
        return false;
    if (dx < ntol)
        return true;

    return false;
    }

struct DPoint2dZYXTolerancedSortComparison
    {
    double m_absTol;
    DPoint2dZYXTolerancedSortComparison(double absTol) : m_absTol(absTol)
        {

        }
    bool operator() (const DPoint2d& pointA, const DPoint2d &pointB) const
        {
        return sort_yx<double>(pointA.x, pointA.y, pointB.x, pointB.y, m_absTol);
        }
    };

END_BENTLEY_SCALABLEMESH_NAMESPACE