/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#pragma once

#include <Bentley/bvector.h>
#include <Geom/GeomApi.h>
#include <ScalableMesh/IScalableMesh.h>
BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

typedef struct { float x, y; } FloatXY;
typedef struct { float x, y, z; } FloatXYZ;

/* This structure represents a modification made on a mesh. */
struct DifferenceSet
    {
    uint64_t clientID; //Stores diff set ID, if applicable. 
                       //Note that there are 2 reserved clientId. 
                       //   -1 is used to specified if all the diffsets are up-to-date (only the upToDate member is used). 
                       //    0 is used to speficied the mesh outside all clip masks (i.e. : the only mesh visible if all clip masks are turned on).    
    int32_t firstIndex; //this index represents the beginning of the array for added vertices in the "added faces" index list. Previous indexes come from the base mesh.
    bvector<DPoint3d> addedVertices;
    bvector<int32_t> removedVertices;
    bvector<int32_t> addedFaces;
    bvector<int32_t> removedFaces;
    bvector<DPoint2d> addedUvs;
    bvector<int32_t> addedUvIndices;
    bool toggledForID;
    std::atomic<bool> upToDate;

    DifferenceSet()
        {
        upToDate = true;
        firstIndex = 0;
        toggledForID = true;
        clientID = -1;
        }

    DifferenceSet(const DifferenceSet& d) :clientID(d.clientID), firstIndex(d.firstIndex), addedVertices(d.addedVertices),
        addedFaces(d.addedFaces), removedVertices(d.removedVertices), removedFaces(d.removedFaces), addedUvs(d.addedUvs), addedUvIndices(d.addedUvIndices)
        {
        toggledForID = d.toggledForID;
        if (d.upToDate) upToDate = true;
        else upToDate = false;
        }

    virtual ~DifferenceSet()
        {
        }
    
    DifferenceSet& operator=(const DifferenceSet& d)
        {        
        //TFS# 919528 - Ensure that the capacity is reset to zero to avoid the capacity to grow up uncontrollably over time.
        addedVertices = bvector<DPoint3d>();
        removedVertices = bvector<int32_t>();
        addedFaces = bvector<int32_t>();    
        removedFaces = bvector<int32_t>();
        addedUvs = bvector<DPoint2d>();
        addedUvIndices = bvector<int32_t>();
        
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
		
    void Empty();

    uint64_t WriteToBinaryStream(void*& serialized, bool willBeRecomputed=false);
    void LoadFromBinaryStream(void* serialized, uint64_t ct);
    template <class PointType3D, class PointType2D>
    void ApplyClipDiffSetToMesh(PointType3D*& points, size_t& nbPoints,
                                int32_t*& faceIndexes, size_t& nbFaceIndexes,
                                PointType2D*& pUv, const int32_t*& pUvIndex, size_t& uvCount,
                                PointType3D const* inPoints, size_t inNbPoints,
                                int32_t const*  inFaceIndexes, size_t inNbFaceIndexes,
                                const DPoint2d* pInUv, const int32_t* pInUvIndex, size_t inUvCount,
                                const DPoint3d& ptTranslation) const
        {
        points = new PointType3D[this->addedVertices.size() + inNbPoints];

        for (size_t ind = inNbPoints; ind < this->addedVertices.size() + inNbPoints; ind++)
            {
            points[ind].x = (float)(this->addedVertices[ind - inNbPoints].x - ptTranslation.x);
            points[ind].y = (float)(this->addedVertices[ind - inNbPoints].y - ptTranslation.y);
            points[ind].z = (float)(this->addedVertices[ind - inNbPoints].z - ptTranslation.z);
            }

        if (inNbPoints > 0)
            {
            memcpy(points, inPoints, sizeof(PointType3D) * inNbPoints);
            }

        if (this->addedUvIndices.size() > 0)
            {
            pUv = new PointType2D[this->addedUvs.size() + inUvCount];

            for (size_t ind = inUvCount; ind < this->addedUvs.size() + inUvCount; ind++)
                {
                pUv[ind].x = this->addedUvs[ind - inUvCount].x;
                pUv[ind].y = this->addedUvs[ind - inUvCount].y;
                }

            if (inUvCount > 0)
                {
                for (size_t ind = 0; ind < inUvCount; ind++)
                    {
                    pUv[ind].x = pInUv[ind].x;
                    pUv[ind].y = pInUv[ind].y;
                    }
                }

            for (size_t uvI = 0; uvI < this->addedUvs.size() + inUvCount; ++uvI)
                {
                if (pUv[uvI].x < 0)  pUv[uvI].x = 0;
                if (pUv[uvI].y < 0)  pUv[uvI].y = 0;
                if (pUv[uvI].x > 1)  pUv[uvI].x = 1;
                if (pUv[uvI].y > 1)  pUv[uvI].y = 1;
                }
            }
        else if (pInUvIndex)
            {
            pUvIndex = 0;
            }

        if (this->addedFaces.size() >= 3 && this->addedFaces.size() < 1024 * 1024)
            {
            size_t newMaxNIndexes = this->addedFaces.size();
            int32_t* newfaceIndexes = new int32_t[newMaxNIndexes];
            size_t newNIndexes = 0;
            int32_t* newUvIndices = this->addedUvIndices.size() > 0 ? new int32_t[newMaxNIndexes] : nullptr;
            for (int i = 0; i + 2 < this->addedFaces.size(); i += 3)
                {
                if (!(this->addedFaces[i] - 1 >= 0 && this->addedFaces[i] - 1 < inNbPoints + this->addedVertices.size() && this->addedFaces[i + 1] - 1 >= 0 && this->addedFaces[i + 1] - 1 < inNbPoints + this->addedVertices.size()
                    && this->addedFaces[i + 2] - 1 >= 0 && this->addedFaces[i + 2] - 1 < inNbPoints + this->addedVertices.size()))
                    {
#if SM_TRACE_CLIP_MESH
                    std::string s;
                    s += "INDICES " + std::to_string(this->addedFaces[i]) + " " + std::to_string(this->addedFaces[i + 1]) + " " + std::to_string(this->addedFaces[i + 2]);
#endif
                    continue;
                    }
                assert(this->addedFaces[i] - 1 >= 0 && this->addedFaces[i] - 1 < inNbPoints + this->addedVertices.size() && this->addedFaces[i + 1] - 1 >= 0 && this->addedFaces[i + 1] - 1 < inNbPoints + this->addedVertices.size()
                    && this->addedFaces[i + 2] - 1 >= 0 && this->addedFaces[i + 2] - 1 < inNbPoints + this->addedVertices.size());
                for (size_t j = 0; j < 3 && newNIndexes <newMaxNIndexes; ++j)
                    {
                    int32_t idx = (int32_t)(this->addedFaces[i + j] >= this->firstIndex ? this->addedFaces[i + j] - this->firstIndex + inNbPoints + 1 : this->addedFaces[i + j]);
                    assert(idx > 0 && idx <= inNbPoints + this->addedVertices.size());
                    newfaceIndexes[newNIndexes] = idx;

                    if (this->addedUvIndices.size() > 0)
                        {
                        if (i + j > this->addedUvIndices.size()) newUvIndices[newNIndexes] = 1;
                        newUvIndices[newNIndexes] = this->addedUvIndices[i + j] + (int32_t)inUvCount;
                        assert(newUvIndices[newNIndexes] <= inUvCount + this->addedUvs.size());
                        }
                    newNIndexes++;
                    }
                }
            nbFaceIndexes = newNIndexes;
            faceIndexes = newfaceIndexes;
            if (this->addedUvIndices.size() > 0)
                {
                pUvIndex = newUvIndices;
                }
            }
        else
            {
            nbFaceIndexes = 0;
            faceIndexes = nullptr;
            }

        nbPoints = inNbPoints + this->addedVertices.size();

        if (this->addedUvIndices.size() > 0) uvCount = inUvCount + this->addedUvs.size();
        }

    void ApplySet(DifferenceSet& d, int firstIndex);
    void ApplyMapped(DifferenceSet& d, const int* idxMap);
    bool ConflictsWith(const DifferenceSet& d);
    DifferenceSet MergeSetWith(DifferenceSet& d, const DPoint3d* vertices);
    DifferenceSet MergeSetWith(DifferenceSet& d, const DPoint3d* vertices, bvector<DPoint3d>& clip1, bvector<DPoint3d>& clip2);
    static DifferenceSet  FromPolyfaceSet(bvector<PolyfaceHeaderPtr>& polyMesh, const DPoint3d* vertices, size_t nVertices);
    PolyfaceHeaderPtr ToPolyfaceMesh(const DPoint3d* points, size_t nofPoints);
    static DifferenceSet  FromPolyfaceSet(bvector<PolyfaceHeaderPtr>& polyMesh, std::map<DPoint3d, int32_t, DPoint3dZYXTolerancedSortComparison> & mapOfPoints, size_t maxPtIdx);
    static DifferenceSet  FromPolyface(PolyfaceHeaderPtr& polyMeshes,std::map<DPoint3d, int32_t, DPoint3dZYXTolerancedSortComparison> & mapOfPoints, size_t maxPtIdx);
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
    explicit DPoint2dZYXTolerancedSortComparison(double absTol) : m_absTol(absTol)
        {}
    bool operator() (const DPoint2d& pointA, const DPoint2d &pointB) const
        {
        return sort_yx<double>(pointA.x, pointA.y, pointB.x, pointB.y, m_absTol);
        }
    };


struct DPoint3dYXTolerancedSortComparison
    {
    double m_absTol;
    explicit DPoint3dYXTolerancedSortComparison(double absTol) : m_absTol(absTol)
        {}
    bool operator() (const DPoint3d& pointA, const DPoint3d &pointB) const
        {
        return sort_yx<double>(pointA.x, pointA.y, pointB.x, pointB.y, m_absTol);
        }
    };

END_BENTLEY_SCALABLEMESH_NAMESPACE