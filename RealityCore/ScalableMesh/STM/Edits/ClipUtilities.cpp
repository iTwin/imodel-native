/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ScalableMeshPCH.h"
#include "../ImagePPHeaders.h"
#include "ClipUtilities.h"
#include <TerrainModel/TerrainModel.h>
#include <ScalableMesh/ScalableMeshUtilityFunctions.h>
#include <Vu/VuApi.h>
#include <TerrainModel/TerrainModel.h>
#include <TerrainModel/Core/DTMIterators.h>
#include <array>
#include "../ScalableMeshQuery.h"
#ifdef SCALABLEMESH_DEBUG_USE_GEOM_SERIALIZATION
#include <GeomSerialization/GeomLibsFlatBufferApi.h>
#endif
USING_NAMESPACE_BENTLEY_TERRAINMODEL
BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE
#define SM_TRACE_CLIPS_GETMESH 0
#define SM_TRACE_CLIPS_FULL 0
static const wchar_t* s_path = L"C:\\work\\2017q3\\tmp\\";

PUSH_DISABLE_DEPRECATION_WARNINGS
void print_polygonarray(std::string& s, const char* tag, DPoint3d* polyArray, int polySize)
    {
    s += tag;
    s += " : ARRAY " + std::to_string((long long)polySize) + " POINTS \n";
    for (int i = 0; i < polySize; i++)
        {
        s += "POINT (" + std::to_string(polyArray[i].x) + "," + std::to_string(polyArray[i].y) + "," + std::to_string(polyArray[i].z) + ")\n";
        }
    }


void FaceToUVMap::ReadFrom(const int32_t* indices, const int32_t* uvIndices, const DPoint2d* uvCoords, size_t nIndices)
    {
    for (size_t i = 0; i < nIndices; i += 3)
        {
        DPoint2d uvArray[3] = { uvCoords[uvIndices[i]-1], uvCoords[uvIndices[i + 1]-1], uvCoords[uvIndices[i + 2]-1] };
        AddFacet(indices + i, uvArray);
        }
    }

void FaceToUVMap::AddFacetOnEdge(const int32_t* indices, const DPoint2d* uvCoords, const int32_t* edge)
    {
    std::array<int32_t, 2> idx = {{ 0, 1}};
    std::sort(idx.begin(), idx.end(), [&edge] (const int32_t& a, const int32_t&b) { return edge[a] < edge[b]; });
    std::array<DPoint2d, 3> sortedUvs = std::array<DPoint2d,3>();
    int32_t lastIdx = -1;
    for (size_t i = 0; i < 3; ++i)
        {
        if (indices[i] != edge[0] && indices[i] != edge[1])
            {
            lastIdx = indices[i];
            sortedUvs[2] = uvCoords[i];
            }
        else if (edge[idx[0]] == indices[i]) sortedUvs[0] = uvCoords[i];
        else if (edge[idx[1]] == indices[i])sortedUvs[1] = uvCoords[i];
        }
    if (map.count(edge[idx[0]]) > 0 && map[edge[idx[0]]].count(edge[idx[1]]) > 0)
        {
        if (map[edge[idx[0]]][edge[idx[1]]][1].first == -1)
            map[edge[idx[0]]][edge[idx[1]]][1] = make_bpair(lastIdx, sortedUvs);
        }
    else
        {
        std::array<bpair<int32_t, std::array<DPoint2d, 3>>, 2> values = {{ make_bpair(lastIdx, sortedUvs), make_bpair(-1, sortedUvs) }};
        map[edge[idx[0]]][edge[idx[1]]] = values;
        }
    }

void FaceToUVMap::RemoveFacetFromEdge(const int32_t* indices, const int32_t* edge)
    {
    int32_t lastIdx = -1;
    for (size_t i = 0; i < 3; ++i)
        {
        if (indices[i] != edge[0] && indices[i] != edge[1])
            {
            lastIdx = indices[i];
            }
        }
    bool deleteEdge = false;
    if (map.count(edge[0]) > 0 && map[edge[0]].count(edge[1]) > 0)
        {
        if (map[edge[0]][edge[1]][0].first == lastIdx)
            {
            map[edge[0]][edge[1]][0] = map[edge[0]][edge[1]][1];
            map[edge[0]][edge[1]][1].first = -1;
            if (map[edge[0]][edge[1]][0].first == -1) deleteEdge = true;
            }
        else
            {
            map[edge[0]][edge[1]][1].first = -1;
            if (map[edge[0]][edge[1]][0].first == -1) deleteEdge = true;
            }
        }
    if (deleteEdge)map[edge[0]].erase(edge[1]);
    }

void FaceToUVMap::AddFacet(const int32_t* indices, const DPoint2d* uvCoords)
    {
#if SM_TRACE_CLIPS_GETMESH
    Utf8String nameBeforeClips = Utf8String(s_path) + "faceTrace_";
    nameBeforeClips.append(to_string(range.low.x).c_str());
    nameBeforeClips.append("_");
    nameBeforeClips.append(to_string(range.low.y).c_str());
    nameBeforeClips.append("_");
    nameBeforeClips.append(to_string(range.high.x).c_str());
    nameBeforeClips.append("_");
    nameBeforeClips.append(to_string(range.high.y).c_str());
    nameBeforeClips.append(".txt");
    std::ofstream stats;
    stats.open(nameBeforeClips.c_str(), std::ios_base::app);
    stats << "ADDING FACET " + std::to_string(indices[0]) + " " + std::to_string(indices[1]) + " " + std::to_string(indices[2]) << std::endl;
    stats << "UVs\n";
    stats << std::to_string(uvCoords[0].x) + " " + std::to_string(uvCoords[0].y) << std::endl;
    stats << std::to_string(uvCoords[1].x) + " " + std::to_string(uvCoords[1].y) << std::endl;
    stats << std::to_string(uvCoords[2].x) + " " + std::to_string(uvCoords[2].y) << std::endl;
    stats.close();
#endif

    int32_t edges[3][2] = { { indices[0], indices[1] }, { indices[1], indices[2] }, { indices[0], indices[2] } };
    for (size_t i = 0; i < 3; ++i)
        AddFacetOnEdge(indices, uvCoords, edges[i]);
    }

void FaceToUVMap::RemoveFacet(const int32_t* indices)
    {

    int32_t edges[3][2] = { { indices[0], indices[1] }, { indices[1], indices[2] }, { indices[0], indices[2] } };
    for (size_t i = 0; i < 3; ++i)
        RemoveFacetFromEdge(indices, edges[i]);
    }

bool FaceToUVMap::GetFacet(const int32_t* indices, DPoint2d* uvCoords)
    {
    std::array<int32_t, 3> idx = {{ 0, 1, 2 }};
    std::sort(idx.begin(), idx.end(), [&indices] (const int32_t& a, const int32_t&b) { return indices[a] < indices[b]; });
    if (map.count(indices[idx[0]]) > 0 && map[indices[idx[0]]].count(indices[idx[1]]) > 0)
        {
        for (auto& val : map[indices[idx[0]]][indices[idx[1]]])
            if (val.first == indices[idx[2]])
                {
                for (size_t i = 0; i < 3; ++i)
                    uvCoords[idx[i]] = val.second[i];
#if SM_TRACE_CLIPS_GETMESH
                Utf8String nameBeforeClips = Utf8String(s_path) + "faceTrace_";
                nameBeforeClips.append(to_string(range.low.x).c_str());
                nameBeforeClips.append("_");
                nameBeforeClips.append(to_string(range.low.y).c_str());
                nameBeforeClips.append("_");
                nameBeforeClips.append(to_string(range.high.x).c_str());
                nameBeforeClips.append("_");
                nameBeforeClips.append(to_string(range.high.y).c_str());
                nameBeforeClips.append(".txt");
                std::ofstream stats;
                stats.open(nameBeforeClips.c_str(), std::ios_base::app);
                stats << "GETTING FACET " + std::to_string(indices[0]) + " " + std::to_string(indices[1]) + " " + std::to_string(indices[2]) << std::endl;
                stats << "UVs\n";
                stats << std::to_string(uvCoords[0].x) + " " + std::to_string(uvCoords[0].y) << std::endl;
                stats << std::to_string(uvCoords[1].x) + " " + std::to_string(uvCoords[1].y) << std::endl;
                stats << std::to_string(uvCoords[2].x) + " " + std::to_string(uvCoords[2].y) << std::endl;
                stats.close();
#endif
                return true;
                }
        }
#if SM_TRACE_CLIPS_FULL
    Utf8String nameBeforeClips = Utf8String(s_path) + "missedHits_";
    nameBeforeClips.append(to_string(range.low.x).c_str());
    nameBeforeClips.append("_");
    nameBeforeClips.append(to_string(range.low.y).c_str());
    nameBeforeClips.append("_");
    nameBeforeClips.append(to_string(range.high.x).c_str());
    nameBeforeClips.append("_");
    nameBeforeClips.append(to_string(range.high.y).c_str());
    nameBeforeClips.append(".txt");
    std::ofstream stats;
    stats.open(nameBeforeClips.c_str(), std::ios_base::app);
    stats << "GETTING FACET " + std::to_string(indices[0]) + " " + std::to_string(indices[1]) + " " + std::to_string(indices[2]) << std::endl;
    stats << " IDX IN MAP ? ";
    stats << ((map.count(indices[idx[0]]) > 0) ? "YES\n" : "NO\n");
    if (map.count(indices[idx[0]]) > 0)
        {
        stats << "2ND IDX IN MAP ? ";
        stats << ((map[indices[idx[0]]].count(indices[idx[1]]) > 0) ? "YES\n" : "NO\n");
        }
    stats.close();
#endif
    return false;
    }

bool FaceToUVMap::SplitFacetOrEdge(DPoint2d& newUv, const int32_t newPt, const int32_t* indices, std::function<DPoint2d(std::array<DPoint2d, 3>, const int32_t*, int32_t)> interpolateCallback, bool onEdge)
    {
    std::array<DPoint2d, 3> uvCoords;
    std::array<int32_t, 3> idx = {{ 0, 1, 2 }};
    std::sort(idx.begin(), idx.end(), [&indices] (const int32_t& a, const int32_t&b) { return indices[a] < indices[b]; });
#if SM_TRACE_CLIPS_GETMESH
    Utf8String nameBeforeClips = Utf8String(s_path) + "faceTrace_";
    nameBeforeClips.append(to_string(range.low.x).c_str());
    nameBeforeClips.append("_");
    nameBeforeClips.append(to_string(range.low.y).c_str());
    nameBeforeClips.append("_");
    nameBeforeClips.append(to_string(range.high.x).c_str());
    nameBeforeClips.append("_");
    nameBeforeClips.append(to_string(range.high.y).c_str());
    nameBeforeClips.append(".txt");
    std::ofstream stats;
    stats.open(nameBeforeClips.c_str(), std::ios_base::app);
    stats << "SPLITTING FACET " + std::to_string(indices[0]) + " " + std::to_string(indices[1]) + " " + std::to_string(indices[2]) << std::endl;
    stats << "FOR PT " + std::to_string(newPt) << std::endl;
    stats << "ON EDGE " + (onEdge? std::string(" TRUE"):std::string(" FALSE")) << std::endl;
    if (onEdge)
        stats << " EDGE " + std::to_string(indices[0]) + " " + std::to_string(indices[1]) << std::endl;
    stats.close();
#endif
    if (map.count(indices[idx[0]]) > 0 && map[indices[idx[0]]].count(indices[idx[1]]) > 0)
        {
        auto pairs = map[indices[idx[0]]][indices[idx[1]]];
        for (auto& val : pairs)
            if (val.first == indices[idx[2]])
                {
                for (size_t i = 0; i < 3; ++i)
                    uvCoords[idx[i]] = val.second[i];
                newUv = interpolateCallback(uvCoords, indices, newPt);
                int32_t orderedFace[3] = { indices[idx[0]], indices[idx[1]], indices[idx[2]] };
                RemoveFacet(orderedFace);
                for (size_t i = 0; i < 3; ++i)
                    {
                    if (onEdge && ((idx[i] == 0 || idx[i] == 1) && (idx[(i + 1) % 3] == 1 || idx[(i + 1) % 3] == 0))) continue;
                    DPoint2d newFacetUvs[3] = { val.second[i], val.second[(i + 1) % 3], newUv };
                    int32_t newFaceIndices[3] = { indices[idx[i]], indices[idx[(i + 1) % 3]], newPt };
                    AddFacet(newFaceIndices, newFacetUvs);
                    }
                return true;
                }
        }
#if SM_TRACE_CLIPS_GETMESH
    nameBeforeClips = Utf8String(s_path) + "faceTrace_";
    nameBeforeClips.append(to_string(range.low.x).c_str());
    nameBeforeClips.append("_");
    nameBeforeClips.append(to_string(range.low.y).c_str());
    nameBeforeClips.append("_");
    nameBeforeClips.append(to_string(range.high.x).c_str());
    nameBeforeClips.append("_");
    nameBeforeClips.append(to_string(range.high.y).c_str());
    nameBeforeClips.append(".txt");
    stats.open(nameBeforeClips.c_str(), std::ios_base::app);
    stats << "COULD NOT SPLIT FACET " + std::to_string(indices[idx[0]]) + " " + std::to_string(indices[idx[1]]) + " " + std::to_string(indices[idx[2]]) << std::endl;
    stats << " IDX IN MAP ? ";
    stats << ((map.count(indices[idx[0]]) > 0) ? "YES\n" : "NO\n");
    if (map.count(indices[idx[0]]) > 0)
        {
        stats << "2ND IDX IN MAP ? ";
        stats << ((map[indices[idx[0]]].count(indices[idx[1]]) > 0) ? "YES\n" : "NO\n");
        }
    stats.close();
#endif
    return false;
    }


bool FaceToUVMap::SplitFacet(DPoint2d& newUv, const int32_t newPt, const int32_t* indices, std::function<DPoint2d(std::array<DPoint2d, 3>, const int32_t*, int32_t)> interpolateCallback)
    {
    return SplitFacetOrEdge(newUv, newPt, indices, interpolateCallback, false);
    }


bool FaceToUVMap::SplitEdge(DPoint2d* newUvs, const int32_t newPt, const int32_t* indices, const int32_t* pts, std::function<DPoint2d(std::array<DPoint2d, 3>, const int32_t*, int32_t)> interpolateCallback)
    {
    std::array<int32_t, 2> idx = {{ 0, 1}};
    std::sort(idx.begin(), idx.end(), [&indices] (const int32_t& a, const int32_t&b) { return indices[a] < indices[b]; });
    if (map.count(indices[idx[0]]) > 0 && map[indices[idx[0]]].count(indices[idx[1]]) > 0)
        {
        size_t i = 0;
        auto pairs = map[indices[idx[0]]][indices[idx[1]]];
        for (auto& val : pairs)
            {
            if (val.first == -1) continue;
            int32_t facetIndices[3] = { indices[0], indices[1], val.first };
            int32_t ptIdx = newPt;
            SplitFacetOrEdge(newUvs[i], ptIdx, facetIndices, interpolateCallback, true);
            ++i;
            }
        if (newUvs[0].Distance (newUvs[1]) <= 1e-3) newUvs[1] = DPoint2d::From(DBL_MAX, DBL_MAX);
        return true;
        }
#if SM_TRACE_CLIPS_GETMESH
    Utf8String nameBeforeClips = Utf8String(s_path) + "faceTrace_";
    nameBeforeClips.append(to_string(range.low.x).c_str());
    nameBeforeClips.append("_");
    nameBeforeClips.append(to_string(range.low.y).c_str());
    nameBeforeClips.append("_");
    nameBeforeClips.append(to_string(range.high.x).c_str());
    nameBeforeClips.append("_");
    nameBeforeClips.append(to_string(range.high.y).c_str());
    nameBeforeClips.append(".txt");
    std::ofstream stats;
    stats.open(nameBeforeClips.c_str(), std::ios_base::app);
    stats << "COULD NOT SPLIT EDGE " + std::to_string(indices[idx[0]]) + " " + std::to_string(indices[idx[1]]) << std::endl;
    stats << " IDX IN MAP ? ";
    stats << ((map.count(indices[idx[0]]) > 0) ? "YES\n" : "NO\n");
    if (map.count(indices[idx[0]]) > 0)
        {
        stats << "2ND IDX IN MAP ? ";
        stats << ((map[indices[idx[0]]].count(indices[idx[1]]) > 0) ? "YES\n" : "NO\n");
        }
    stats.close();
#endif
    return false;
    }

void TranslateToDTMIndices(int32_t* translatedIndices, const DPoint3d* vertices, const int32_t* indices, BENTLEY_NAMESPACE_NAME::TerrainModel::DTMPtr& tm, size_t nIndices)
    {
    bmap<DPoint3d, int32_t, DPoint3dZYXTolerancedSortComparison> mapOfPoints(DPoint3dZYXTolerancedSortComparison(1e-6, 0));
    for (size_t i = 0; i < (size_t)tm->GetBcDTM()->GetPointCount(); ++i)
        {
        DPoint3d pt;
        tm->GetBcDTM()->GetPoint((int)i, pt);
        mapOfPoints[pt] = (int)i;
        }
#if SM_TRACE_CLIPS_GETMESH
    DRange3d range;
    tm->GetRange(range);
    Utf8String nameBeforeClips = Utf8String(s_path) + "statsDTM_";
    nameBeforeClips.append(to_string(range.low.x).c_str());
    nameBeforeClips.append("_");
    nameBeforeClips.append(to_string(range.low.y).c_str());
    nameBeforeClips.append("_");
    nameBeforeClips.append(to_string(range.high.x).c_str());
    nameBeforeClips.append("_");
    nameBeforeClips.append(to_string(range.high.y).c_str());
    nameBeforeClips.append(".txt");
    std::ofstream stats;
    stats.open(nameBeforeClips.c_str(), std::ios_base::trunc);
#endif
    for (size_t i = 0; i < nIndices; ++i)
        {
        translatedIndices[i] = mapOfPoints[vertices[indices[i] - 1]];
#if SM_TRACE_CLIPS_GETMESH
        stats << "INDEX " + std::to_string(translatedIndices[i]) + " IS VERTEX: ";
        DPoint3d pt = vertices[indices[i] - 1];
        std::string s;
        print_polygonarray(s, "", &pt, 1);
        stats << s;
#endif
        }
#if SM_TRACE_CLIPS_GETMESH
    stats.close();
#endif
    }


//=======================================================================================
// @bsimethod                                                   Elenie.Godzaridis 10/15
//=======================================================================================
bool GridBasedIntersect(const bvector<DPoint3d>& pts, const DRange3d& ptsBox, const DRange3d& box)
    {
    /*if (ptsBox.IsStrictlyContainedXY(box))*/ return true;
#if 0
bool intersect[25*25] = { false };
    for (auto& pt : pts)
        {
        size_t posX = floor(24 * (pt.x - ptsBox.low.x) / (ptsBox.high.x - ptsBox.low.x));
        size_t posY = floor(24 * (pt.y - ptsBox.low.y) / (ptsBox.high.y - ptsBox.low.y));
        intersect[posX * 25 + posY] = true;
        }
    for (size_t x = 0; x < 25; ++x)
        {
        size_t minVal = 100, maxVal = 100;
        for (size_t y = 0; y < 25; ++y)
            {
            if (intersect[x * 25 + y])
                {
                if (minVal > 24) minVal = y;
                else
                    {
                    maxVal = y;
                    minVal = 100;
                    }
                }
            else
                {
                if (minVal <= 24 && minVal <= y) intersect[x * 25 + y] = true;
                }
            }
        }
    size_t posBoxMinX = box.low.x> ptsBox.low.x ? floor(24 * (box.low.x - ptsBox.low.x) / (ptsBox.high.x - ptsBox.low.x)) : 0;
    size_t posBoxMinY = box.low.y > ptsBox.low.y ? floor(24 * (box.low.y - ptsBox.low.y) / (ptsBox.high.y - ptsBox.low.y)) : 0;
    size_t posBoxMaxX = box.high.x < ptsBox.high.x ? floor(24 * (box.high.x - ptsBox.low.x) / (ptsBox.high.x - ptsBox.low.x)) : 24;
    size_t posBoxMaxY = box.high.y < ptsBox.high.y ? floor(24 * (box.high.y - ptsBox.low.y) / (ptsBox.high.y - ptsBox.low.y)) : 24;
    for (size_t x = 0; x < 25; ++x)
        for (size_t y = 0; y < 25; ++y)
            if (x >= posBoxMinX && x <= posBoxMaxX && y >= posBoxMinY && y <= posBoxMaxY && intersect[x * 25 + y]) return true;
    return false;
    #endif
    }

void PrintPoints(std::string& s, vector<DPoint3d>& points)
    {
    s += "POINTS \n";
    for (auto&pt : points)
        {
        s += " ( " + std::to_string(pt.x) + "," + std::to_string(pt.y) + "," + std::to_string(pt.z) + ")\n";
        }
    }

void PrintDiff(std::string& s, DifferenceSet& d, const DPoint3d* oldVert)
    {
    s += "DIFFERENCE \n";
    s += " ADDED FACES \n";
    std::string s2;
    bool write = false;
    for (size_t i = 0; i < d.addedFaces.size(); ++i)
        {
        if (i % 3 == 0)
            {
            s2 = "FACE\n";
            write = false;
            }
        DPoint3d pt = DPoint3d::From(0,0,0);
        assert(d.addedFaces[i] != 0);
        if (d.addedFaces[i] >= d.firstIndex && d.addedFaces[i] - d.firstIndex >= d.addedVertices.size())
            {
            s += " ASSERT! "+ std::to_string(d.addedFaces[i])+" IDX "+std::to_string(d.firstIndex);
            }
        else if (d.addedFaces[i] >= d.firstIndex)
            pt = d.addedVertices[d.addedFaces[i] - d.firstIndex];
        else
            pt = oldVert[d.addedFaces[i] - 1];
        if (pt.x > 429128 && pt.x < 420132 && pt.y >4497751 && pt.y < 4407753) write = true;
        s2 += " ( " + std::to_string(pt.x) + "," + std::to_string(pt.y) + "," + std::to_string(pt.z) + ")\n";
        if ((i + 1) % 3 == 0 && write) s += s2;
        }

    s += " REMOVED FACES \n";
    for (size_t i = 0; i < d.removedFaces.size(); ++i)
        {
        if (i % 3 == 0) s += "FACE\n";
        DPoint3d pt = oldVert[d.removedFaces[i] - 1];
        s += " ( " + std::to_string(pt.x) + "," + std::to_string(pt.y) + "," + std::to_string(pt.z) + ")\n";
        }
    }

void PolygonFromFaceList(bvector<bvector<int32_t>>& contourPolyIndices, bvector<std::array<int32_t, 3>>& faceIndices, size_t nVertices)
    {
    vector<set<int32_t>> edges(nVertices);
    for (auto& face : faceIndices)
        {

            if (edges[face[0]].count(face[1]) == 0) edges[face[0]].insert(face[1]);
            else edges[face[0]].erase(face[1]);
            if (edges[face[1]].count(face[0]) == 0) edges[face[1]].insert(face[0]);
            else edges[face[1]].erase(face[0]);

            if (edges[face[1]].count(face[2]) == 0) edges[face[1]].insert(face[2]);
            else edges[face[1]].erase(face[2]);
            if (edges[face[2]].count(face[1]) == 0) edges[face[2]].insert(face[1]);
            else edges[face[2]].erase(face[1]);

            if (edges[face[0]].count(face[2]) == 0) edges[face[0]].insert(face[2]);
            else edges[face[0]].erase(face[2]);
            if (edges[face[2]].count(face[0]) == 0) edges[face[2]].insert(face[0]);
            else edges[face[2]].erase(face[0]);

        }
    int32_t currentIdx = -1;
    int32_t lastIdx = -1;
    for (auto& neighbors : edges)
        {
        if (neighbors.size() == 0) continue;
        if (currentIdx == -1)
            {
            currentIdx = &neighbors - &edges[0];
            break;
            }
        }
    int32_t firstIdx = currentIdx;
    bvector<int32_t> currentLoop;
    while (currentIdx != -1)
        {
        currentLoop.push_back(currentIdx);
        bool end = true;
        for (int32_t i : edges[currentIdx])
            if (i == lastIdx) continue;
            else if (i == firstIdx)
                {
                edges[currentIdx].erase(i);
                edges[i].erase(currentIdx);
                contourPolyIndices.push_back(currentLoop);
                currentLoop.clear();
                for (auto& neighbors : edges)
                    {
                    if (neighbors.size() == 0) continue;
                    if (&neighbors - &edges[0] > firstIdx)
                        {
                        currentIdx = &neighbors - &edges[0];
                        firstIdx = currentIdx;
                        end = false;
                        break;
                        }
                    }
                break;
                }
            else
            {
            lastIdx = currentIdx;
            edges[currentIdx].erase(i);
            edges[i].erase(currentIdx);
            currentIdx = i;
            end = false;
            break;
            }
        if(end) currentIdx = -1;
        }
    contourPolyIndices.push_back(currentLoop);
    }


double s_timeSpentClipping = 0;
double s_timeConvexPolys = 0;
double s_timeNonConvexPolys = 0;
double s_numberOfClips = 0;
double s_numberOfConvexClips = 0;
double s_numberOfNonConvexClips = 0;
double s_minClipTime = DBL_MAX;
double s_maxClipTime = DBL_MIN;
double s_timeSpentInClipPolyFace = 0;
bool s_useTMClips = true;

    bool s_stop = false;

    class FaceLookupList
        {
        private:
            bvector<bmap<int32_t, bvector<bpair<int32_t, int32_t>>>> listOfFaces;

        public:
            FaceLookupList(size_t nVertices): listOfFaces(nVertices) {}
            void AddFace(int32_t i, int32_t j, int32_t k, int32_t faceId)
                {
                if (listOfFaces[i].count(j) == 0)
                    listOfFaces[i].insert(make_bpair(j, bvector<bpair<int32_t,int32_t>>()));
                listOfFaces[i][j].push_back(make_bpair(k, faceId));
                if (listOfFaces[i].count(k) == 0)
                    listOfFaces[i].insert(make_bpair(k, bvector<bpair<int32_t, int32_t>>()));
                listOfFaces[i][k].push_back(make_bpair(j, faceId));
                if (listOfFaces[j].count(k) == 0)
                    listOfFaces[j].insert(make_bpair(k, bvector<bpair<int32_t, int32_t>>()));
                listOfFaces[j][k].push_back(make_bpair(i, faceId));
                if (listOfFaces[j].count(i) == 0)
                    listOfFaces[j].insert(make_bpair(i, bvector<bpair<int32_t, int32_t>>()));
                listOfFaces[j][i].push_back(make_bpair(k, faceId));
                if (listOfFaces[k].count(i) == 0)
                    listOfFaces[k].insert(make_bpair(i, bvector<bpair<int32_t, int32_t>>()));
                listOfFaces[k][i].push_back(make_bpair(j, faceId));
                if (listOfFaces[k].count(j) == 0)
                    listOfFaces[k].insert(make_bpair(j, bvector<bpair<int32_t, int32_t>>()));
                listOfFaces[k][j].push_back(make_bpair(i, faceId));
                }
            int32_t LookUpFace(int32_t i, int32_t j, int32_t k)
                {
                if (listOfFaces[i].count(j) > 0)
                    {
                    for (auto pair : listOfFaces[i][j])
                        {
                        if (pair.first == k) return pair.second;
                        }
                    return -1;
                    }
                else return -1;
                }
            void FetchAllFaces(bvector<int32_t>& outFaces, int32_t i)
                {
                for (auto it = listOfFaces[i].begin(); it != listOfFaces[i].end(); ++it)
                    {
                    for (auto pair : it->second)
                        {
                        outFaces.push_back(pair.second);
                        }
                    }
                }
            void FetchAllFaces(bvector<int32_t>& outFaces, int32_t i, int32_t j)
                {
                    for (auto pair : listOfFaces[i][j])
                        {
                        outFaces.push_back(pair.second);
                        }
                }
        };

    static size_t s_number = 0;


bool s_printDTM = false;

void Clipper::MakeDTMFromIndexList(BENTLEY_NAMESPACE_NAME::TerrainModel::DTMPtr& dtmPtr)
    {
    BC_DTM_OBJ* bcDtmP = 0;
    int dtmCreateStatus = bcdtmObject_createDtmObject(&bcDtmP);
    if (dtmCreateStatus == 0)
        {
        BcDTMPtr bcDtmObjPtr;
//#ifdef VANCOUVER_API
        bcDtmObjPtr = BcDTM::CreateFromDtmHandle(*bcDtmP);
//#else
 //       bcDtmObjPtr = BcDTM::CreateFromDtmHandle(bcDtmP);
//#endif
        dtmPtr = bcDtmObjPtr.get();
        }
    else return;
    DPoint3d triangle[4];

    for (unsigned int t = 0; t < m_nIndices; t += 3)
        {
        for (int i = 0; i < 3; i++)
            triangle[i] = m_vertexBuffer[m_indexBuffer[i + t] - 1];

        triangle[3] = triangle[0];

        std::swap(triangle[1], triangle[2]);
        //DTM doesn't like colinear triangles
        if (bsiGeom_isDPoint3dArrayColinear(triangle, 3, 1e-6)) continue;

        bcdtmObject_storeDtmFeatureInDtmObject(dtmPtr->GetBcDTM()->GetTinHandle(), DTMFeatureType::GraphicBreak, dtmPtr->GetBcDTM()->GetTinHandle()->nullUserTag, 1, &dtmPtr->GetBcDTM()->GetTinHandle()->nullFeatureId, &triangle[0], 4);
        }
    bcdtmObject_triangulateStmTrianglesDtmObject(dtmPtr->GetBcDTM()->GetTinHandle());
    if (s_printDTM)
        {
#if _WIN32
        WString dtmFileName(L"d:\\clipping\\outputTM");
        WString triangleF = dtmFileName;
        dtmFileName.append(L".dtm");
        bcDtmP = 0;
        bcdtmObject_createDtmObject(&bcDtmP);
        bcdtmObject_storeDtmFeatureInDtmObject(bcDtmP, DTMFeatureType::RandomSpots, bcDtmP->nullUserTag, 1, &bcDtmP->nullFeatureId, m_vertexBuffer, (long)m_nVertices);
        bcdtmObject_triangulateDtmObject(bcDtmP);
        bcdtmWrite_toFileDtmObject(bcDtmP, dtmFileName.c_str());
        FILE* triFile = _wfopen(triangleF.c_str(), L"wb");
        for (unsigned int t = 0; t < m_nIndices; t += 3)
            {
            for (int i = 0; i < 3; i++)
                triangle[i] = m_vertexBuffer[m_indexBuffer[i + t] - 1];

            triangle[3] = triangle[0];

            std::swap(triangle[1], triangle[2]);

            fwrite(triangle, sizeof(DPoint3d), 4, triFile);
            }
        fclose(triFile);
#endif
        }
    }


DPoint2d ComputeUVs(DPoint3d pt, DRange3d ext, size_t textureWidthInPixels=1024, size_t textureHeightInPixels= 1024)
    {
    DPoint2d uv;
    double unitsPerPixelX = (ext.high.x - ext.low.x) / textureWidthInPixels;
    double unitsPerPixelY = (ext.high.y - ext.low.y) / textureHeightInPixels;
    ext.low.x -= 5 * unitsPerPixelX;
    ext.low.y -= 5 * unitsPerPixelY;
    ext.high.x += 5 * unitsPerPixelX;
    ext.high.y += 5 * unitsPerPixelY;
    uv.x = max(0.0, min((pt.x - ext.low.x) / (ext.XLength()), 1.0));
    uv.y = max(0.0, min((pt.y - ext.low.y) / (ext.YLength()), 1.0));
    return uv;
    }

void Clipper::TagUVsOnPolyface(PolyfaceHeaderPtr& poly, BENTLEY_NAMESPACE_NAME::TerrainModel::DTMPtr& dtmPtr, FaceToUVMap& faceToUVMap, bmap<int32_t, int32_t>& mapOfIndices)
    {
    vector<int32_t> indices(poly->GetPointIndexCount());
    memcpy(&indices[0], poly->GetPointIndexCP(), poly->GetPointIndexCount()*sizeof(int32_t));
    bmap<int32_t, int32_t> allPts;
    std::map<DPoint2d, int32_t, DPoint2dZYXTolerancedSortComparison> allUvs(DPoint2dZYXTolerancedSortComparison(1e-5));
    for (size_t i = 0; i < poly->GetPointIndexCount(); ++i)
        {
        DPoint3d pt;
        dtmPtr->GetBcDTM()->GetPoint(poly->GetPointIndexCP()[i], pt);
        if (allPts.count(poly->GetPointIndexCP()[i]) == 0)
            {
            allPts[poly->GetPointIndexCP()[i]] = (int)poly->Point().size();
            poly->Point().push_back(pt);
            }
        }
    size_t nFaceMisses = 0;
    poly->PointIndex().clear();
    poly->Param().SetActive(true);
    poly->ParamIndex().SetActive(true);
    for (size_t i = 0; i < indices.size(); i += 3)
        {
        DPoint2d uvCoords[3];
        int32_t newIndices[3] = { indices[i], indices[i + 1], indices[i + 2] };
        for (size_t j = 0; j < 3; ++j)
            {
            if (mapOfIndices.count(indices[i + j]) != 0)
                {
                newIndices[j] = mapOfIndices[indices[i + j]];
                }
            }
        if (!faceToUVMap.GetFacet(&newIndices[0], uvCoords))
            {
            if (poly->Param().size() == 0)
                poly->Param().push_back(DPoint2d::From(0.0, 0.0));
            nFaceMisses++;
            poly->PointIndex().push_back(allPts[newIndices[0]] + 1);
            poly->PointIndex().push_back(allPts[newIndices[ 1]] + 1);
            poly->PointIndex().push_back(allPts[newIndices[2]] + 1);
            for (size_t uvI = 0; uvI < 3; ++uvI)
                poly->ParamIndex().push_back(1);
            continue;
            }
        poly->PointIndex().push_back(allPts[newIndices[0]] + 1);
        poly->PointIndex().push_back(allPts[newIndices[1]] + 1);
        poly->PointIndex().push_back(allPts[newIndices[2]] + 1);
        for (size_t uvI = 0; uvI < 3; ++uvI)
            {
            uvCoords[uvI] = ComputeUVs(poly->Point()[allPts[newIndices[uvI]]], m_nodeRange, m_widthOfTexData, m_heightOfTexData);
            if (allUvs.count(uvCoords[uvI]) == 0 || allUvs[uvCoords[uvI]] == 0)
                {
                poly->Param().push_back(uvCoords[uvI]);
                allUvs[uvCoords[uvI]] = (int)poly->GetParamCount();
                }

            poly->ParamIndex().push_back(allUvs[uvCoords[uvI]]);
            }
        }

#if SM_TRACE_CLIPS_FULL
    Utf8String nameBeforeClips = Utf8String(s_path) + "meshtaggeds_";
    nameBeforeClips.append(to_string(m_range.low.x).c_str());
    nameBeforeClips.append("_");
    nameBeforeClips.append(to_string(m_range.low.y).c_str());
    nameBeforeClips.append("_");
    nameBeforeClips.append(to_string(m_range.high.x).c_str());
    nameBeforeClips.append("_");
    nameBeforeClips.append(to_string(m_range.high.y).c_str());
    nameBeforeClips.append("_");
    nameBeforeClips.append(to_string(poly->GetPointCount()).c_str());
    nameBeforeClips.append(".txt");
    std::ofstream stats;
    stats.open(nameBeforeClips.c_str(), std::ios_base::trunc);
    stats << " N POINTS " + std::to_string(poly->GetPointCount()) << std::endl;
    stats << " N UVS " + std::to_string(poly->GetParamCount()) << std::endl;
    for (size_t i = 0; i < poly->GetPointIndexCount(); i += 3)
        {
        stats << "TRIANGLE " + std::to_string(i / 3) + "\n";
        std::string s;
        DPoint3d pts[3] = { poly->GetPointCP()[poly->GetPointIndexCP()[i] - 1], poly->GetPointCP()[poly->GetPointIndexCP()[i + 1] - 1], poly->GetPointCP()[poly->GetPointIndexCP()[i + 2] - 1] };
        print_polygonarray(s, " POINT ",pts, 3);
        stats << s;
        stats << " UVS :" << std::endl;
        stats << std::to_string(poly->GetParamCP()[poly->GetParamIndexCP()[i] - 1].x) + " " + std::to_string(poly->GetParamCP()[poly->GetParamIndexCP()[i] - 1].y) << std::endl;
        stats << std::to_string(poly->GetParamCP()[poly->GetParamIndexCP()[i+1] - 1].x) + " " + std::to_string(poly->GetParamCP()[poly->GetParamIndexCP()[i + 1] - 1].y) << std::endl;
        stats << std::to_string(poly->GetParamCP()[poly->GetParamIndexCP()[i+2] - 1].x) + " " + std::to_string(poly->GetParamCP()[poly->GetParamIndexCP()[i + 2] - 1].y) << std::endl;
        stats << " UVINDICES :" << std::endl;
        stats << std::to_string(poly->GetParamIndexCP()[i] - 1) << std::endl;
        stats << std::to_string(poly->GetParamIndexCP()[i + 1] - 1)<< std::endl;
        stats << std::to_string(poly->GetParamIndexCP()[i + 2] - 1) << std::endl;
    }
    stats.close();
#endif

#if SM_TRACE_CLIPS_FULL
    bool dbg = false;
    if (dbg)
        {
        WString name = WString(s_path) + L"fpostclipmeshtagged_";
        name.append(to_wstring(m_range.low.x).c_str());
        name.append(L"_");
        name.append(to_wstring(m_range.low.y).c_str());
        name.append(L"_");
        name.append(to_wstring(m_range.high.x).c_str());
        name.append(L"_");
        name.append(to_wstring(m_range.high.y).c_str());
        name.append(L".mt");
        FILE* meshAfterClip = _wfopen(name.c_str(), L"wb");
        size_t ptCount = poly->GetPointCount();
        size_t faceCount = poly->GetPointIndexCount();
        size_t uvCount = poly->GetParamCount();
        fwrite(&ptCount, sizeof(size_t), 1, meshAfterClip);
        fwrite(poly->GetPointCP(), sizeof(DPoint3d), ptCount, meshAfterClip);
        fwrite(&faceCount, sizeof(size_t), 1, meshAfterClip);
        fwrite(poly->GetPointIndexCP(), sizeof(int32_t), faceCount, meshAfterClip);
        fwrite(&uvCount, sizeof(size_t), 1, meshAfterClip);
        fwrite(poly->GetParamCP(), sizeof(int32_t), uvCount, meshAfterClip);
        fwrite(&faceCount, sizeof(size_t), 1, meshAfterClip);
        fwrite(poly->GetParamIndexCP(), sizeof(int32_t), faceCount, meshAfterClip);
        fclose(meshAfterClip);
        }
#endif
    }

DTMInsertPointCallback Clipper::GetInsertPointCallback(FaceToUVMap& faceToUVMap, BENTLEY_NAMESPACE_NAME::TerrainModel::DTMPtr& ptr)
    {
    return[&faceToUVMap, &ptr] (int newPtNum, DPoint3dCR pt, double&elevation, bool onEdge, const int pts[])
        {
#if 0
        Utf8String nameBeforeClips = Utf8String(s_path) + "insertPts_";
        nameBeforeClips.append(to_string(faceToUVMap.range.low.x).c_str());
        nameBeforeClips.append("_");
        nameBeforeClips.append(to_string(faceToUVMap.range.low.y).c_str());
        nameBeforeClips.append("_");
        nameBeforeClips.append(to_string(faceToUVMap.range.high.x).c_str());
        nameBeforeClips.append("_");
        nameBeforeClips.append(to_string(faceToUVMap.range.high.y).c_str());
        nameBeforeClips.append(".txt");
        std::ofstream stats;
        stats.open(nameBeforeClips.c_str(), std::ios_base::app);
        stats << "ADDING PT " + std::to_string(newPtNum) + "\n";
        stats << (onEdge?" ON EDGE \n": " ON FACET \n");
        stats << " POINTS \n";
        for (size_t i = 0; i <( onEdge ? 4 : 3); ++i)
            stats << std::to_string(pts[i]) + " ";
        stats << std::endl;
        stats.close();
#endif
        if (onEdge)
            {
            DPoint2d uvCoords[2];
            faceToUVMap.SplitEdge(uvCoords, newPtNum, pts, pts + 2, [&ptr, &pt] (std::array<DPoint2d, 3> uvCoords, const int32_t* indices, int32_t newPt) -> DPoint2d
                {
                double fraction;
                DPoint3d pt1, pt2;
                ptr->GetBcDTM()->GetPoint(indices[0], pt1);
                ptr->GetBcDTM()->GetPoint(indices[1], pt2);
                DSegment3d seg = DSegment3d::From(pt1, pt2);
                seg.PointToFractionParameter(fraction, pt);
                DPoint2d newUv = DPoint2d::FromInterpolate(uvCoords[0], fraction, uvCoords[1]);
                return newUv;
                });
            }
        else
            {
            DPoint2d uv;
            faceToUVMap.SplitFacet(uv, newPtNum, pts, [&ptr, &pt] (std::array<DPoint2d, 3> uvCoords, const int32_t* indices, int32_t newPt) -> DPoint2d
                {
                DPoint3d triangle[3];
                for (size_t i = 0; i < 3; ++i)
                    ptr->GetBcDTM()->GetPoint(indices[i], triangle[i]);

                DPoint3d barycentric;
                bsiDPoint3d_barycentricFromDPoint3dTriangle(&barycentric, &pt, &triangle[0], &triangle[1], &triangle[2]);
                DPoint2d newUv;
                bsiDPoint2d_fromBarycentricAndDPoint2dTriangle(&newUv, &barycentric, &uvCoords[0], &uvCoords[1], &uvCoords[2]);
                return newUv;
                });
            }
        };
    }

size_t s_nclip = 0;

bool s_useOwnClip = false;

struct FinishClippedMesh : public PolyfaceQuery::IClipToPlaneSetOutput
    {
    PolyfaceHeaderPtr m_currentPoly;
    PolyfaceHeaderPtr m_outPoly;
    virtual StatusInt   _ProcessUnclippedPolyface(PolyfaceQueryCR polyfaceQuery) override
        {
        m_outPoly = PolyfaceHeader::New();
        m_outPoly->CopyFrom(polyfaceQuery);
        return SUCCESS;
        }
    virtual StatusInt   _ProcessClippedPolyface(PolyfaceHeaderR polyfaceHeader) override
        {
        m_currentPoly = PolyfaceHeader::New();
        m_currentPoly->CopyFrom(polyfaceHeader);
        return SUCCESS;
        }

    PolyfaceHeaderPtr GetResultMesh() { return m_currentPoly; }
    PolyfaceHeaderPtr GetOutsideMesh() { return m_outPoly; }
    FinishClippedMesh() {};
    };

PolyfaceHeaderPtr CreateFromFaceSubset(PolyfaceHeaderPtr& originalMesh, const bvector<int32_t>& facesToKeep)
    {
    bvector<DPoint3d> pts;
    bvector<int32_t> indices;
    bvector<DPoint2d> params;
    bvector<int32_t> paramIndices;
    bmap<int, int> oldToNewIndicesMap;
    bmap<int, int> oldToNewParamIndicesMap;

    PolyfaceVisitorPtr vis = PolyfaceVisitor::Attach(*originalMesh);
    bvector<int> &pointIndex = vis->ClientPointIndex();
    bvector<int> &param = vis->ClientParamIndex();
    size_t faceIdx = 0;
    size_t current = 0;
    for (vis->Reset(); vis->AdvanceToNextFace()&& current < facesToKeep.size();faceIdx++)
        {
        if (facesToKeep[current] != faceIdx) continue;

        for (size_t i = 0; i < 3; ++i)
            {
            if (oldToNewIndicesMap.count(pointIndex[i]) == 0)
                {
                pts.push_back(originalMesh->GetPointCP()[pointIndex[i]]);
                oldToNewIndicesMap[pointIndex[i]] = (int)pts.size();
                }
            if (originalMesh->GetParamCount() > 0 && oldToNewParamIndicesMap.count(param[i]) == 0)
                {
                params.push_back(originalMesh->GetParamCP()[param[i]]);
                oldToNewParamIndicesMap[param[i]] = (int)params.size();
                }
            indices.push_back(oldToNewIndicesMap[pointIndex[i]]);
            if (originalMesh->GetParamCount() > 0)  paramIndices.push_back(oldToNewParamIndicesMap[param[i]]);
            }
        current++;
        }
    PolyfaceQueryCarrier* query = new PolyfaceQueryCarrier(3, false, indices.size(), pts.size(), pts.empty() ? 0 : &pts[0], indices.empty() ? 0 : &indices[0], 0, 0, 0, params.size(),
                                                           params.empty() ? 0 : &params[0], paramIndices.empty() ? 0 : &paramIndices[0]);
    PolyfaceHeaderPtr vec = PolyfaceHeader::CreateFixedBlockIndexed(3);
    vec->CopyFrom(*query);
    delete query;
    return vec;
    }

PolyfaceHeaderPtr CreateFromSubsetAndValues(PolyfaceHeaderPtr& originalMesh, const bvector<int32_t> valuesForFaces, int32_t targetVal)
    {
    bvector<DPoint3d> pts;
    bvector<int32_t> indices;
    bvector<DPoint2d> params;
    bvector<int32_t> paramIndices;
    bmap<int, int> oldToNewIndicesMap;
    bmap<int, int> oldToNewParamIndicesMap;

    PolyfaceVisitorPtr vis = PolyfaceVisitor::Attach(*originalMesh);
    bvector<int> &pointIndex = vis->ClientPointIndex();
    bvector<int> &param = vis->ClientParamIndex();
    size_t faceIdx = 0;
    for (vis->Reset(); vis->AdvanceToNextFace(); faceIdx++)
        {
        if (valuesForFaces[faceIdx] != targetVal) continue;

        // Check and skip degenerate triangles
        DPoint3d tri[3] = { originalMesh->GetPointCP()[pointIndex[0]], originalMesh->GetPointCP()[pointIndex[1]], originalMesh->GetPointCP()[pointIndex[2]] };
        DPoint3d centroid;
        DVec3d normal;
        double area;
        PolygonOps::CentroidNormalAndArea(tri, 3, centroid, normal, area);
        if(area < 1e-10) continue;

        for (size_t i = 0; i < 3; ++i)
            {
            if (oldToNewIndicesMap.count(pointIndex[i]) == 0)
                {
                pts.push_back(originalMesh->GetPointCP()[pointIndex[i]]);
                oldToNewIndicesMap[pointIndex[i]] = (int)pts.size();
                }
            if (originalMesh->GetParamCount() > 0 && oldToNewParamIndicesMap.count(param[i]) == 0)
                {
                params.push_back(originalMesh->GetParamCP()[param[i]]);
                oldToNewParamIndicesMap[param[i]] = (int)params.size();
                }
            indices.push_back(oldToNewIndicesMap[pointIndex[i]]);
            if (originalMesh->GetParamCount() > 0) paramIndices.push_back(oldToNewParamIndicesMap[param[i]]);
            }
        }
    PolyfaceQueryCarrier* query = new PolyfaceQueryCarrier(3, false, indices.size(), pts.size(), pts.empty() ? 0 : &pts[0], indices.empty() ? 0 : &indices[0], 0, 0, 0, params.size(),
                                                           params.empty() ? 0 : &params[0], paramIndices.empty() ? 0 : &paramIndices[0]);
    PolyfaceHeaderPtr vec = PolyfaceHeader::CreateFixedBlockIndexed(3);
    vec->CopyFrom(*query);
    delete query;

    return vec;
    }

bool Process3dRegions(bvector<bvector<PolyfaceHeaderPtr>>& polyfaces, PolyfaceHeaderPtr& clippedMesh, ClipVectorCP clipPolys, std::pair<bool, uint64_t> shouldInvertBoundary = { false, -1 })
    {
    bvector<bvector<int>> idxOfFaces(clipPolys->size());
    bvector<int32_t> nCrossingPolys(clippedMesh->GetPointIndexCount() / 3, 0);

    PolyfaceVisitorPtr vis = PolyfaceVisitor::Attach(*clippedMesh);
    bvector<int> &pointIndex = vis->ClientPointIndex();
    size_t idxFace = 0;
    bvector<DPoint3d> centroids(clippedMesh->GetPointIndexCount() / 3);
    bvector<double> areas(clippedMesh->GetPointIndexCount() / 3);
    for (vis->Reset(); vis->AdvanceToNextFace();)
        {
        DPoint3d tri[3] = { clippedMesh->GetPointCP()[pointIndex[0]], clippedMesh->GetPointCP()[pointIndex[1]], clippedMesh->GetPointCP()[pointIndex[2]] };
        DVec3d normal;
        PolygonOps::CentroidNormalAndArea(tri, 3, centroids[idxFace], normal, areas[idxFace]);
        idxFace++;
        }

    idxFace = 0;
    for (vis->Reset(); vis->AdvanceToNextFace();)
        {
        // Degenerate triangles are always outside (must be filtered out)
        if(areas[idxFace] > 1e-10)
            {
                for(auto& primitive : *clipPolys)
                    {
                    if (!primitive.IsValid()) continue;
                    bool pointInside = primitive->PointInside(centroids[idxFace], 1e-8);
                    if(!pointInside)
                        {
                        idxOfFaces[&primitive - &clipPolys->front()].push_back((int)idxFace);
                        nCrossingPolys[idxFace]++;
                        }
                    }
            }
        idxFace++;
        }

    for (size_t i = 1; i < clipPolys->size() + 1; ++i)
        {
        PolyfaceHeaderPtr vec = CreateFromFaceSubset(clippedMesh, idxOfFaces[i - 1]);
        polyfaces[i].push_back(vec);
        }

    PolyfaceHeaderPtr vec2 = CreateFromSubsetAndValues(clippedMesh, nCrossingPolys, 0);

    polyfaces[0].push_back(vec2);

    if(shouldInvertBoundary.first)
        {
        std::swap(polyfaces[0], polyfaces[shouldInvertBoundary.second + 1]);
        }
    return true;
    }

struct IntersectionLocation
    {
    DPoint3d pt;
    int32_t edgeIdx;
    int32_t onVertex;
    int32_t newPtIdx;
    int32_t newParamIdx;
    };

void CreatePlanes(bvector<DPlane3d>& planes, const bvector<DPoint3d>& lineSegments)
    {
    for (size_t i = 0; i < lineSegments.size() - 1; ++i)
        {
        DPlane3d plane = DPlane3d::From3Points(lineSegments[i], lineSegments[i + 1], DPoint3d::FromSumOf(lineSegments[i], DPoint3d::From(0, 0, -1)));
        planes.push_back(plane);
        }
    }

bool ComputeCut(bvector<IntersectionLocation>& foundIntersects, const DPlane3d& plane, const DPoint3d* tri, bvector<bvector<double>>* pointDistanceTable, int* pointIdxTri, size_t planeIdx)
    {
    double sign = 0;
    bool planeCutsTriangle = false;
    for (size_t j = 0; j < 3 && !planeCutsTriangle; j++)
        {
		double sideOfPoint;
		if (pointDistanceTable != nullptr && (*pointDistanceTable)[pointIdxTri[j]][planeIdx] == DBL_MAX)
		{
			(*pointDistanceTable)[pointIdxTri[j]][planeIdx] = plane.Evaluate(tri[j]);
			sideOfPoint = (*pointDistanceTable)[pointIdxTri[j]][planeIdx];
		}
		else sideOfPoint = plane.Evaluate(tri[j]);
        if (fabs(sideOfPoint) < 1e-8) sideOfPoint = 0;
        if (sign == 0) sign = sideOfPoint;
        else if ((sign > 0 && sideOfPoint < 0) || (sign < 0 && sideOfPoint > 0))
            planeCutsTriangle = true;
        }

    if (planeCutsTriangle)
        {
        DPoint3d intersectPts[2];
        int32_t cutEdges[2];
        int32_t vIds[2] = { -1, -1 };
        size_t nOfIntersects = 0;
        //intersect segments to find cut
        for (size_t j = 0; j < 3 && nOfIntersects < 2; j++)
            {
            DSegment3d edgeSegment = DSegment3d::From(tri[j], tri[(j + 1) % 3]);
            double param = -DBL_MAX;
            if (edgeSegment.Intersect(intersectPts[nOfIntersects], param, plane) && param > -1.0e-5 && param <= 1.0 + 1e-5)
                {
                cutEdges[nOfIntersects] = (int32_t)j;
                if (param < 1e-5) vIds[nOfIntersects] = (int32_t)j;
                if (param > 1 + 1e-5) vIds[nOfIntersects] = (int32_t)(j + 1) % 3;
                ++nOfIntersects;
                }
            }

            for (size_t i = 0; i < 2; ++i)
                {
                IntersectionLocation loc;
                loc.pt = intersectPts[i];
                loc.edgeIdx = cutEdges[i];
                loc.onVertex = vIds[i];
                foundIntersects.push_back(loc);
                }
            return true;
        }
    else return false;
    }

void ComputeReplacementFacets(bvector<bvector<int32_t>>& newFacets, bvector<IntersectionLocation>& foundIntersects, bvector<int32_t>& splitFacet, bool useParam = false)
    {
    int hasVertex = -1;
    if (foundIntersects[0].onVertex != -1)
        {
        hasVertex = 0;
        }
    else if(foundIntersects[1].onVertex!=-1)
        {
        hasVertex = 1;
        }
    if (hasVertex != -1)
        {
        bvector<int32_t> firstTri(3);
        firstTri[0] = useParam ? foundIntersects[(hasVertex + 1) % 2].newParamIdx : foundIntersects[(hasVertex + 1) % 2].newPtIdx;
        firstTri[1] = splitFacet[(foundIntersects[(hasVertex + 1) % 2].edgeIdx + 2) % 3];
        firstTri[2] = splitFacet[(foundIntersects[(hasVertex + 1) % 2].edgeIdx + 3) % 3];
       // firstTri[2] = splitFacet[(foundIntersects[hasVertex].onVertex + 1) % 3];
        newFacets.push_back(firstTri);
        bvector<int32_t> secondTri(3);
        //secondTri[0] = useParam ? foundIntersects[hasVertex].newParamIdx : foundIntersects[hasVertex].newPtIdx;
        secondTri[0] = useParam ? foundIntersects[(hasVertex + 1) % 2].newParamIdx : foundIntersects[(hasVertex + 1) % 2].newPtIdx;
        secondTri[1] = splitFacet[(foundIntersects[(hasVertex + 1) % 2].edgeIdx + 1) % 3];
        secondTri[2] = splitFacet[(foundIntersects[(hasVertex + 1) % 2].edgeIdx + 2) % 3];
        newFacets.push_back(secondTri);
        }
    else
        {
        int edgeId = 0;
        if ((foundIntersects[0].edgeIdx == 1 && foundIntersects[1].edgeIdx == 0)
            || (foundIntersects[1].edgeIdx == 1 && foundIntersects[0].edgeIdx == 0))
            edgeId = 2;
        else if ((foundIntersects[0].edgeIdx == 2 && foundIntersects[1].edgeIdx == 0)
            || (foundIntersects[1].edgeIdx == 2 && foundIntersects[0].edgeIdx == 0))
            edgeId = 1;

        bvector<int32_t> firstTri(3);
        firstTri[0] = useParam ? foundIntersects[0].newParamIdx : foundIntersects[0].newPtIdx;
        firstTri[1] = useParam ? foundIntersects[1].newParamIdx : foundIntersects[1].newPtIdx;
        firstTri[2] = splitFacet[(edgeId+2) % 3];
        newFacets.push_back(firstTri);
        bvector<int32_t> secondTri(3);
        secondTri[0] = useParam ? foundIntersects[0].newParamIdx : foundIntersects[0].newPtIdx;
        secondTri[1] = useParam ? foundIntersects[1].newParamIdx : foundIntersects[1].newPtIdx;
        secondTri[2] = splitFacet[edgeId];
        newFacets.push_back(secondTri);
        bvector<int32_t> thirdTri(3);
        if (foundIntersects[0].edgeIdx == ((edgeId + 1) % 3))
            thirdTri[0] = useParam ? foundIntersects[0].newParamIdx : foundIntersects[0].newPtIdx;
        else
            thirdTri[0] = useParam ? foundIntersects[1].newParamIdx : foundIntersects[1].newPtIdx;
        thirdTri[1] = splitFacet[edgeId];
        thirdTri[2] = splitFacet[(edgeId+1) % 3];
        newFacets.push_back(thirdTri);
        }
    }

void InsertCutPoints(PolyfaceHeaderPtr& inOutMesh, bvector<IntersectionLocation>& foundIntersects, const DPoint3d* tri, const int* paramIndex, const int* pointIndex, bool isTexturedMesh, bvector<bvector<double>>* pointDistanceTable, size_t nOfDistsPerPoint)
    {
    for (auto& intersect : foundIntersects)
        {
        if (intersect.onVertex == -1)
            {
            intersect.newPtIdx = (int)inOutMesh->Point().AppendAndReturnIndex(intersect.pt);
            if(pointDistanceTable != nullptr) pointDistanceTable->push_back(bvector<double>(nOfDistsPerPoint, DBL_MAX));
            if (isTexturedMesh)
                {
                DPoint3d barycentric;
                bsiDPoint3d_barycentricFromDPoint3dTriangle(&barycentric, &intersect.pt, &tri[0], &tri[1], &tri[2]);
                DPoint2d newUv;
                bsiDPoint2d_fromBarycentricAndDPoint2dTriangle(&newUv, &barycentric, inOutMesh->GetParamCP() + paramIndex[0], inOutMesh->GetParamCP() + paramIndex[1], inOutMesh->GetParamCP() + paramIndex[2]);
                intersect.newParamIdx = (int)inOutMesh->Param().AppendAndReturnIndex(newUv);
                }
            }
        else
            {
            intersect.newPtIdx = pointIndex[intersect.onVertex];
            if (isTexturedMesh) intersect.newParamIdx = paramIndex[intersect.onVertex];
            }
        }
    }

bool InsertMeshCuts(PolyfaceHeaderPtr& inOutMesh, PolyfaceVisitorPtr& vis, ClipVectorCP clipSegments, bvector<DRange3d>& faceRanges, const DRange3d& polyRange)
    {
    bool meshIsCut = false;
    bvector<DPlane3d> planesFromSegments;
    bool meshHasTexture = inOutMesh->Param().size() > 0 && inOutMesh->ParamIndex().size() > 0;
	DRange3d meshRange = DRange3d::From(inOutMesh->GetPointCP(), (int)inOutMesh->GetPointCount());
    for (auto& primitive : *clipSegments)
        {
            for (auto& planes : *(primitive->GetClipPlanes()))
                for(auto& plane : planes)
                    {
                    if(plane.IsVisible())
                        planesFromSegments.push_back(plane.GetDPlane3d());
                    }
        }

	bool doNotOptimizeDistanceChecks = false;
	if (planesFromSegments.size() > 500)
		doNotOptimizeDistanceChecks = true;
    bvector<bvector<double>> pointToPlaneDists(inOutMesh->GetPointCount());

	if(!doNotOptimizeDistanceChecks)
		for (auto& dist : pointToPlaneDists)dist.resize(planesFromSegments.size(), DBL_MAX);
    // for (auto& plane : planesFromSegments)
    //      {
    //      size_t originalNIdx = inOutMesh->GetPointIndexCount() - 1;
    bvector<int> &pointIndex = vis->ClientPointIndex();
    bvector<int> &param = vis->ClientParamIndex();
    bvector<bvector<bool>> pointToPlaneChecks(inOutMesh->GetPointIndexCount() / 3);
	if (!doNotOptimizeDistanceChecks)
	 for (auto& check : pointToPlaneChecks)check.resize(planesFromSegments.size(), true);

    for (vis->Reset(); vis->AdvanceToNextFace();)// && vis->GetReadIndex() <= originalNIdx;)
        {
        DPoint3d tri[3] = { inOutMesh->GetPointCP()[pointIndex[0]], inOutMesh->GetPointCP()[pointIndex[1]], inOutMesh->GetPointCP()[pointIndex[2]] };
        if (faceRanges[vis->GetReadIndex() / 3].IsNull())faceRanges[vis->GetReadIndex() / 3] = DRange3d::From(tri, 3);
        if (!faceRanges[vis->GetReadIndex() / 3].IntersectsWith(polyRange)) continue;
		if (inOutMesh->GetPointIndexCount() > 1000000) break;
        for (auto& plane : planesFromSegments)
            {
            bvector<IntersectionLocation> results;
            if (!doNotOptimizeDistanceChecks && !pointToPlaneChecks[vis->GetReadIndex() / 3][&plane - &planesFromSegments[0]]) continue;
            if (ComputeCut(results, plane, tri, doNotOptimizeDistanceChecks? nullptr : &pointToPlaneDists, pointIndex.data(), &plane - &planesFromSegments[0]))
                {
                assert(results.size() == 2);
                meshIsCut = true;
                InsertCutPoints(inOutMesh, results, tri, meshHasTexture ? param.data() : nullptr, pointIndex.data(), meshHasTexture, doNotOptimizeDistanceChecks ? nullptr : &pointToPlaneDists, planesFromSegments.size());
                if(!doNotOptimizeDistanceChecks)
					pointToPlaneChecks[vis->GetReadIndex() / 3][&plane - &planesFromSegments[0]] = false;
                bvector<bvector<int32_t>> newFaces;
                ComputeReplacementFacets(newFaces, results, pointIndex);
                assert(!newFaces.empty());
                if (!newFaces.empty())
                    {
                    inOutMesh->PointIndex()[vis->GetReadIndex()] = newFaces[0][0] + 1;
                    inOutMesh->PointIndex()[vis->GetReadIndex() + 1] = newFaces[0][1] + 1;
                    inOutMesh->PointIndex()[vis->GetReadIndex() + 2] = newFaces[0][2] + 1;
                    tri[0] = inOutMesh->GetPointCP()[newFaces[0][0]];
                    tri[1] = inOutMesh->GetPointCP()[newFaces[0][1]];
                    tri[2] = inOutMesh->GetPointCP()[newFaces[0][2]];
                    pointIndex[0] = newFaces[0][0];
                    pointIndex[1] = newFaces[0][1];
                    pointIndex[2] = newFaces[0][2];
                    faceRanges[vis->GetReadIndex() / 3] = DRange3d::From(&inOutMesh->Point()[newFaces[0][0]], 1);
                    faceRanges[vis->GetReadIndex() / 3].Extend(inOutMesh->Point()[newFaces[0][1]]);
                    faceRanges[vis->GetReadIndex() / 3].Extend(inOutMesh->Point()[newFaces[0][2]]);
                    for (size_t i = 1; i < newFaces.size(); ++i)
                        {
                        inOutMesh->PointIndex().push_back(newFaces[i][0] + 1);
                        inOutMesh->PointIndex().push_back(newFaces[i][1] + 1);
                        inOutMesh->PointIndex().push_back(newFaces[i][2] + 1);
                        faceRanges.push_back(DRange3d::NullRange());
						if (!doNotOptimizeDistanceChecks)
						{
							pointToPlaneChecks.push_back(pointToPlaneChecks[vis->GetReadIndex() / 3]);
							pointToPlaneChecks[(inOutMesh->PointIndex().size() - 3) / 3][&plane - &planesFromSegments[0]] = false;
						}
                        }
                    }

                if (meshHasTexture)
                    {
                    newFaces.clear();
                    ComputeReplacementFacets(newFaces, results, param, true);
                    assert(!newFaces.empty());
                    if (!newFaces.empty())
                        {
                        inOutMesh->ParamIndex()[vis->GetReadIndex()] = newFaces[0][0] + 1;
                        inOutMesh->ParamIndex()[vis->GetReadIndex() + 1] = newFaces[0][1] + 1;
                        inOutMesh->ParamIndex()[vis->GetReadIndex() + 2] = newFaces[0][2] + 1;
                        param[0] = newFaces[0][0];
                        param[1] = newFaces[0][1];
                        param[2] = newFaces[0][2];
                        for (size_t i = 1; i < newFaces.size(); ++i)
                            {
                            inOutMesh->ParamIndex().push_back(newFaces[i][0] + 1);
                            inOutMesh->ParamIndex().push_back(newFaces[i][1] + 1);
                            inOutMesh->ParamIndex().push_back(newFaces[i][2] + 1);
                            }
                        }
                    }
                }
            }
        }
    return meshIsCut;
    }

#ifdef SCALABLEMESH_USE_GEOMLIBS_CLIPPING
bool GetRegionsFromClipVector3D(bvector<bvector<PolyfaceHeaderPtr>>& polyfaces, bvector<size_t>& polyfaceIndices, ClipVectorCP clip, const PolyfaceQuery* meshP)
    {
    polyfaces.resize(std::max((int)clip->size() + 1, 2));
    bvector<DRange3d> triangleBoxes;
    PolyfaceHeaderPtr clippedMesh = PolyfaceHeader::CreateFixedBlockIndexed(3);
    clippedMesh->CopyFrom(*meshP);
    DRange3d polyBox;
    polyBox.Init();

    std::pair<bool, uint64_t> boundaryInfo(false, -1);
    DRange3d meshRange = DRange3d::From(clippedMesh->GetPointCP(), (int)clippedMesh->GetPointCount());
    bool hasBoundaryClip = false;
    uint64_t i = -1;
    for (ClipPrimitivePtr const& primitive : *clip)
        {
        polyfaceIndices.push_back(++i);
        bool clipIsMask = primitive->IsMask();
        if (!clipIsMask)
            {
            boundaryInfo.first = true;
            boundaryInfo.second = i;
            }
        if (!hasBoundaryClip) hasBoundaryClip = !clipIsMask;

        DRange3d        primitiveRange;
        if (!primitive->GetRange(primitiveRange, nullptr, primitive->IsMask()))
            {
            primitiveRange.low.x = primitiveRange.low.y = primitiveRange.low.z = -DBL_MAX;
            primitiveRange.high.x = primitiveRange.high.y = primitiveRange.high.z = DBL_MAX;
            }
        if (i == 0)
            {
            polyBox = primitiveRange;
            }
        else {
            polyBox.UnionOf(polyBox, primitiveRange);
            }
        }
    if (polyBox.IsEmpty()) return hasBoundaryClip; // Node is outside the boundary clip, therefore it must be clipped out

    if (clip->empty()) return false;

    i = 1;
    PolyfaceHeaderPtr result;
    PolyfaceHeaderPtr inside;
    PolyfaceHeaderPtr outside;
    for (auto& primitive : *clip)
        {
        for (auto& planes : *(primitive->GetClipPlanes()))
            {
#ifdef SCALABLEMESH_DEBUG_USE_GEOM_SERIALIZATION
            BENTLEY_NAMESPACE_NAME::Utf8String meshStr;
            auto copyForSerialization = clippedMesh->Clone();
            copyForSerialization->ConvertToVariableSizeSignedOneBasedIndexedFaceLoops();
            auto g = IGeometry::Create(copyForSerialization);
            BENTLEY_NAMESPACE_NAME::BentleyGeometryJson::TryGeometryToJsonString(meshStr, *g);
#endif

            inside = nullptr;
            outside = nullptr;
            ClipPlaneSet::ClipPlaneSetIntersectPolyface(*clippedMesh, planes, false, &inside, &outside);
            if (primitive->IsMask())
                {
                result = outside;
                }
            else
                {
                result = inside;
                }
            result->Triangulate();

#ifdef SCALABLEMESH_DEBUG_USE_GEOM_SERIALIZATION
            BENTLEY_NAMESPACE_NAME::Utf8String resultStr;
            copyForSerialization = result->Clone();
            copyForSerialization->ConvertToVariableSizeSignedOneBasedIndexedFaceLoops();
            g = IGeometry::Create(copyForSerialization);
            BENTLEY_NAMESPACE_NAME::BentleyGeometryJson::TryGeometryToJsonString(resultStr, *g);
#endif

            polyfaces[i].push_back(result);
            ++i;
            }
    }

    result = clippedMesh;
    for (auto& primitive : *clip)
        {
        for (auto& planes : *(primitive->GetClipPlanes()))
            {
            inside = nullptr;
            outside = nullptr;
            ClipPlaneSet::ClipPlaneSetIntersectPolyface(*result, planes, false, &inside, &outside);
            if (primitive->IsMask())
                {
                result = outside;
                }
            else
                {
                result = inside;
                }
            }
        }
    result->Triangulate();
    polyfaces[0].push_back(result);

    if (boundaryInfo.first)
        {
        std::swap(polyfaces[0], polyfaces[boundaryInfo.second + 1]);
        }

    return true;
}
#else
bool GetRegionsFromClipVector3D(bvector<bvector<PolyfaceHeaderPtr>>& polyfaces, bvector<size_t>& polyfaceIndices, ClipVectorCP clip, const PolyfaceQuery* meshP)
    {
    polyfaces.resize(std::max((int)clip->size() + 1, 2));
    bvector<DRange3d> triangleBoxes;
    PolyfaceHeaderPtr clippedMesh = PolyfaceHeader::CreateFixedBlockIndexed(3);
    clippedMesh->CopyFrom(*meshP);
    PolyfaceVisitorPtr vis = PolyfaceVisitor::Attach(*clippedMesh);
    triangleBoxes.resize(clippedMesh->GetPointIndexCount() / 3, DRange3d::NullRange());
    DRange3d polyBox;
    polyBox.Init();

    std::pair<bool, uint64_t> boundaryInfo(false, -1);
    DRange3d meshRange = DRange3d::From(clippedMesh->GetPointCP(), (int)clippedMesh->GetPointCount());
    bool hasBoundaryClip = false;
    uint64_t i = -1;
    for (ClipPrimitivePtr const& primitive : *clip)
        {
        polyfaceIndices.push_back(++i);
        bool clipIsMask = primitive->IsMask();
        if (!clipIsMask)
            {
            boundaryInfo.first = true;
            boundaryInfo.second = i;
            }
        if (!hasBoundaryClip) hasBoundaryClip = !clipIsMask;

        DRange3d        primitiveRange;
        if (!primitive->GetRange(primitiveRange, nullptr, primitive->IsMask()))
            {
            primitiveRange.low.x = primitiveRange.low.y = primitiveRange.low.z = -DBL_MAX;
            primitiveRange.high.x = primitiveRange.high.y = primitiveRange.high.z = DBL_MAX;
            }
        if (i == 0)
            {
            polyBox = primitiveRange;
            }
        else {
            polyBox.UnionOf(polyBox, primitiveRange);
            }
        }
    if (polyBox.IsEmpty()) return hasBoundaryClip; // Node is outside the boundary clip, therefore it must be clipped out
    bool meshIsCut = InsertMeshCuts(clippedMesh, vis, clip, triangleBoxes, polyBox);

    if (clip->empty()) return false;
    if (meshIsCut) clippedMesh->Triangulate();

    return Process3dRegions(polyfaces, clippedMesh, clip, boundaryInfo);
    }
#endif
        

void MeshClipper::TagUVsOnPolyface(PolyfaceHeaderPtr& poly, BENTLEY_NAMESPACE_NAME::TerrainModel::DTMPtr& dtmPtr, FaceToUVMap& faceToUVMap, bmap<int32_t, int32_t>& mapOfIndices)
{
    vector<int32_t> indices(poly->GetPointIndexCount());
    memcpy(&indices[0], poly->GetPointIndexCP(), poly->GetPointIndexCount() * sizeof(int32_t));
    bmap<int32_t, int32_t> allPts;
    std::map<DPoint2d, int32_t, DPoint2dZYXTolerancedSortComparison> allUvs(DPoint2dZYXTolerancedSortComparison(1e-5));
    for (size_t i = 0; i < poly->GetPointIndexCount(); ++i)
    {
        DPoint3d pt;
        dtmPtr->GetBcDTM()->GetPoint(poly->GetPointIndexCP()[i], pt);
        if (allPts.count(poly->GetPointIndexCP()[i]) == 0)
        {
            allPts[poly->GetPointIndexCP()[i]] = (int)poly->Point().size();
            poly->Point().push_back(pt);
        }
    }
    //size_t nFaceMisses = 0;
    poly->PointIndex().clear();
    poly->Param().SetActive(true);
    poly->ParamIndex().SetActive(true);
    for (size_t i = 0; i < indices.size(); i += 3)
    {
        DPoint2d uvCoords[3];
        int32_t newIndices[3] = { indices[i], indices[i + 1], indices[i + 2] };
        /* for (size_t j = 0; j < 3; ++j)
        {
        if (mapOfIndices.count(indices[i + j]) != 0)
        {
        newIndices[j] = mapOfIndices[indices[i + j]];
        }
        }
        if (!faceToUVMap.GetFacet(&newIndices[0], uvCoords))
        {
        if (poly->Param().size() == 0)
        poly->Param().push_back(DPoint2d::From(0.0, 0.0));
        nFaceMisses++;
        poly->PointIndex().push_back(allPts[newIndices[0]] + 1);
        poly->PointIndex().push_back(allPts[newIndices[ 1]] + 1);
        poly->PointIndex().push_back(allPts[newIndices[2]] + 1);
        for (size_t uvI = 0; uvI < 3; ++uvI)
        poly->ParamIndex().push_back(1);
        continue;
        }*/
        poly->PointIndex().push_back(allPts[newIndices[0]] + 1);
        poly->PointIndex().push_back(allPts[newIndices[1]] + 1);
        poly->PointIndex().push_back(allPts[newIndices[2]] + 1);
        for (size_t uvI = 0; uvI < 3; ++uvI)
        {
            uvCoords[uvI] = ComputeUVs(poly->Point()[allPts[newIndices[uvI]]], m_nodeRange, m_widthOfTexData, m_heightOfTexData);
            if (allUvs.count(uvCoords[uvI]) == 0 || allUvs[uvCoords[uvI]] == 0)
            {
                poly->Param().push_back(uvCoords[uvI]);
                allUvs[uvCoords[uvI]] = (int)poly->GetParamCount();
            }

            poly->ParamIndex().push_back(allUvs[uvCoords[uvI]]);
        }
    }
}

DTMInsertPointCallback MeshClipper::GetInsertPointCallback(FaceToUVMap& faceToUVMap, BENTLEY_NAMESPACE_NAME::TerrainModel::DTMPtr& ptr)
{
    return[&faceToUVMap, &ptr](int newPtNum, DPoint3dCR pt, double&elevation, bool onEdge, const int pts[])
    {
        if (onEdge)
        {
            DPoint2d uvCoords[2];
            faceToUVMap.SplitEdge(uvCoords, newPtNum, pts, pts + 2, [&ptr, &pt](std::array<DPoint2d, 3> uvCoords, const int32_t* indices, int32_t newPt) -> DPoint2d
            {
                double fraction;
                DPoint3d pt1, pt2;
                ptr->GetBcDTM()->GetPoint(indices[0], pt1);
                ptr->GetBcDTM()->GetPoint(indices[1], pt2);
                DSegment3d seg = DSegment3d::From(pt1, pt2);
                seg.PointToFractionParameter(fraction, pt);
                DPoint2d newUv = DPoint2d::FromInterpolate(uvCoords[0], fraction, uvCoords[1]);
                return newUv;
            });
        }
        else
        {
            DPoint2d uv;
            faceToUVMap.SplitFacet(uv, newPtNum, pts, [&ptr, &pt](std::array<DPoint2d, 3> uvCoords, const int32_t* indices, int32_t newPt) -> DPoint2d
            {
                DPoint3d triangle[3];
                for (size_t i = 0; i < 3; ++i)
                    ptr->GetBcDTM()->GetPoint(indices[i], triangle[i]);

                DPoint3d barycentric;
                bsiDPoint3d_barycentricFromDPoint3dTriangle(&barycentric, &pt, &triangle[0], &triangle[1], &triangle[2]);
                DPoint2d newUv;
                bsiDPoint2d_fromBarycentricAndDPoint2dTriangle(&newUv, &barycentric, &uvCoords[0], &uvCoords[1], &uvCoords[2]);
                return newUv;
            });
        }
    };
}

void MeshClipper::MakeDTMFromIndexList(BENTLEY_NAMESPACE_NAME::TerrainModel::DTMPtr& dtmPtr)
    {
    BC_DTM_OBJ* bcDtmP = 0;
    int dtmCreateStatus = bcdtmObject_createDtmObject(&bcDtmP);
    if (dtmCreateStatus == 0)
        {
        BcDTMPtr bcDtmObjPtr;
        bcDtmObjPtr = BcDTM::CreateFromDtmHandle(*bcDtmP);
        dtmPtr = bcDtmObjPtr.get();
        }
    else return;
    DPoint3d triangle[4];

    for (unsigned int t = 0; t < m_nIndices; t += 3)
        {
        for (int i = 0; i < 3; i++)
            triangle[i] = m_vertexBuffer[m_indexBuffer[i + t] - 1];

        triangle[3] = triangle[0];

        std::swap(triangle[1], triangle[2]);
        //DTM doesn't like colinear triangles
        if (bsiGeom_isDPoint3dArrayColinear(triangle, 3, 1e-6)) continue;

        bcdtmObject_storeDtmFeatureInDtmObject(dtmPtr->GetBcDTM()->GetTinHandle(), DTMFeatureType::GraphicBreak, dtmPtr->GetBcDTM()->GetTinHandle()->nullUserTag, 1, &dtmPtr->GetBcDTM()->GetTinHandle()->nullFeatureId, &triangle[0], 4);
        }
    bcdtmObject_triangulateStmTrianglesDtmObject(dtmPtr->GetBcDTM()->GetTinHandle());
    }

MeshClipper::MeshClipper()
    {
    m_vertexBuffer = nullptr;
    m_nVertices =  0;
    m_indexBuffer = nullptr;
    m_nIndices = 0;
    m_uvBuffer = nullptr;
    m_uvIndices = nullptr;
    m_range= DRange3d::NullRange();
    m_nodeRange = DRange3d::NullRange();
    m_widthOfTexData = 1024;
    m_heightOfTexData = 1024;
    m_is25dData = false;
    wasClipped = false;
    }

void MeshClipper::SetSourceMesh(const PolyfaceQuery* meshSourceData, bool is25dData)
    {
    m_sourceData = meshSourceData;
    m_is25dData = is25dData;
    }

void MeshClipper::SetClipGeometry(const bvector<uint64_t>& ids, const bvector<bvector<DPoint3d>>& polygons)
    {
    for (size_t i = 0; i < ids.size() && i < polygons.size(); ++i)
        {
        ClipPolyInfo info;
        info.id = ids[i];
        info.pts = polygons[i];
        info.isMask = true;
        allPolys.push_back(info);
        }
    OrderClipGeometryList();
    }

void MeshClipper::SetClipGeometry(const bvector<uint64_t>& ids, const bvector<bvector<DPoint3d>>& polygons, const bvector<bool>& polygonIsMask)
    {
    for (size_t i = 0; i < ids.size() && i < polygons.size() && i <  polygonIsMask.size(); ++i)
    {
        ClipPolyInfo info;
        info.id = ids[i];
        info.pts = polygons[i];
        info.isMask = polygonIsMask[i];
        allPolys.push_back(info);
    }
    OrderClipGeometryList();
    }

void MeshClipper::SetClipGeometry(const bmap<size_t, uint64_t>& idsForPrimitives, ClipVectorCP clip)
    {
    bmap<uint64_t, ClipVectorInfo> mapOfVector;
    size_t n = 0;
    for (const auto& primitive : *clip)
    {
        uint64_t id = idsForPrimitives.find(n)->second;
        if (mapOfVector.count(id) == 0)
            mapOfVector[id] = ClipVectorInfo();
        mapOfVector[id].id = id;
        if (nullptr == mapOfVector[id].clip)
        {
#ifdef VANCOUVER_API
            vectorDefs.push_back(ClipVector::CreateFromPrimitive(primitive));
#else
            vectorDefs.push_back(ClipVector::CreateFromPrimitive(primitive.get()));
#endif
            mapOfVector[id].clip = vectorDefs.back().get();
        }
        else const_cast<ClipVector*>(mapOfVector[id].clip)->push_back(primitive);
        mapOfVector[id].arePrimitivesMasks.push_back(primitive->IsMask());
        n++;
    }
    for (auto& vecPair : mapOfVector)
    {
        allVectors.push_back(vecPair.second);
    }
    OrderClipGeometryList();
    }

void MeshClipper::SetClipGeometry(const bmap<size_t, uint64_t>& idsForPrimitives, ClipVectorCP clip, const bmap<size_t, bool>& isMaskForEachPrimitive)
    {
    bmap<uint64_t, ClipVectorInfo> mapOfVector;
    size_t n = 0;
    for (const auto& primitive : *clip)
       {
        uint64_t id = idsForPrimitives.find(n)->second;
        if (mapOfVector.count(id) == 0)
            mapOfVector[id] = ClipVectorInfo();
        mapOfVector[id].id =id;
        if (nullptr == mapOfVector[id].clip)
            {
#ifdef VANCOUVER_API
            vectorDefs.push_back(ClipVector::CreateFromPrimitive(primitive));
#else
            vectorDefs.push_back(ClipVector::CreateFromPrimitive(primitive.get()));
#endif
            mapOfVector[id].clip = vectorDefs.back().get();
            }
        else const_cast<ClipVector*>(mapOfVector[id].clip)->push_back(primitive);
        mapOfVector[id].arePrimitivesMasks.push_back(isMaskForEachPrimitive.find(n)->second);
        n++;
       }
    for (auto& vecPair : mapOfVector)
        {
        allVectors.push_back(vecPair.second);
        }
    OrderClipGeometryList();
    }

void MeshClipper::SetClipGeometry(uint64_t id, ClipVectorCP clip, bool isMask)
    {
    ClipVectorInfo info;
    info.id = id;
    info.clip = clip;
    info.arePrimitivesMasks.resize(clip->size(), isMask);
    OrderClipGeometryList();
    }

void MeshClipper::SetMaskInfo(uint64_t id, bool isMask)
    {
    for (auto& vec : allVectors)
        {
        if (vec.id == id)
            for (auto& prim : vec.arePrimitivesMasks)
                prim = isMask;
        }
    for (auto& poly : allPolys)
        {
        if (poly.id == id)
            poly.isMask = isMask;
        }
    }

bool MeshClipper::IsClipMask(uint64_t id)
{
    bool isMask = false;
    for (auto& info : orderedClipList)
    {
        if (info->id == id)
            if (info->type == ClipInfo::Type::Vector)
            {
                for (auto& mask : static_cast<ClipVectorInfo*>(info)->arePrimitivesMasks)
                    isMask = mask;
            }
            else if (info->type == ClipInfo::Type::Polygon)
            {
                isMask = static_cast<ClipPolyInfo*>(info)->isMask;
            }
    }
    return isMask;
}

void MeshClipper::ClearClipGeometry()
{
    orderedClipList.clear();
    allPolys.clear();
    allVectors.clear();
}

void MeshClipper::SetMeshExtents(DRange3d extentOfData, DRange3d extentOfTexture)
{
    m_range = extentOfData;
    m_nodeRange = extentOfTexture;
}

void MeshClipper::OrderClipGeometryList()
{
    orderedClipList.clear();
    for (auto& poly : allPolys)
        orderedClipList.push_back(&poly);
    for (auto& vec : allVectors)
        orderedClipList.push_back(&vec);

    std::sort(orderedClipList.begin(), orderedClipList.end(),[](ClipInfo* a, ClipInfo* b) {
        if (!a->isClipMask() && b->isClipMask())
            return true;
        return false;
    });
}

void MeshClipper::GetClipsAsVectors(bvector<ClipVectorPtr>& outVectors)
{
    for (auto& clip : orderedClipList)
    {
        if (clip->type == ClipInfo::Type::Vector)
            outVectors.push_back(ClipVectorPtr(const_cast<ClipVector*>(static_cast<ClipVectorInfo*>(clip)->clip)));
        else if (clip->type == ClipInfo::Type::Polygon)
        {
            CurveVectorPtr curvePtr = CurveVector::CreateLinear(static_cast<ClipPolyInfo*>(clip)->pts, CurveVector::BOUNDARY_TYPE_Outer);
            vectorDefs.push_back(ClipVector::CreateFromCurveVector(*curvePtr, 1e-8, 1e-8));
            outVectors.push_back(vectorDefs.back().get());
        }
    }
}


void MeshClipper::GetClipsAsSingleVector(ClipVectorPtr& outVector)
{
    for (auto& clip : orderedClipList)
    {
        if (clip->type == ClipInfo::Type::Vector)
        {
            for (size_t i = 0; i < static_cast<ClipVectorInfo*>(clip)->clip->size(); ++i)
            {
                outVector->push_back((*static_cast<ClipVectorInfo*>(clip)->clip)[i]);
                // The following can cause a crash because clips aren't thread safe
                //outVector->back()->SetIsMask(static_cast<ClipVectorInfo*>(clip)->arePrimitivesMasks[i]);
            }
        }
        else if (clip->type == ClipInfo::Type::Polygon)
        {
            CurveVectorPtr curvePtr = CurveVector::CreateLinear(static_cast<ClipPolyInfo*>(clip)->pts, CurveVector::BOUNDARY_TYPE_Outer);
            vectorDefs.push_back(ClipVector::CreateFromCurveVector(*curvePtr, 1e-8, 1e-8));
            for (size_t i = 0; i < vectorDefs.back()->size(); ++i)
            {
                outVector->push_back((*vectorDefs.back())[i]);
                // The following can cause a crash because clips aren't thread safe
                //outVector->back()->SetIsMask(static_cast<ClipPolyInfo*>(clip)->isMask);
            }
        }
    }
}

void MeshClipper::ComputeClip()
{
    bvector<bvector<PolyfaceHeaderPtr>> outputRegions;
    ClipVectorPtr unifiedVector = ClipVector::Create();
    GetClipsAsSingleVector(unifiedVector);

    bvector<size_t> polyIndices;
    if(true == GetRegionsFromClipVector3D(outputRegions, polyIndices, unifiedVector.get(), m_sourceData))
    {
        if(!polyIndices.empty())
        {
            ClippedRegion regExt;
            regExt.isExterior = true;
            regExt.id = 0;
            for(auto& m : outputRegions[0])
                regExt.meshes.push_back(m);
            computedRegions.push_back(regExt);

            size_t reg = 1, clipIdx = 0;
            for(auto& clip : orderedClipList)
            {
                if(reg >= outputRegions.size())  break;
                ClippedRegion regClipped;
                regClipped.isExterior = false;
                 regClipped.id = clip->id;
                 if(clip->type == ClipInfo::Type::Vector)
                 {
                     for(size_t i = 0; i < static_cast<ClipVectorInfo*>(clip)->clip->size(); ++i)
                     {
                         if(polyIndices[reg - 1] == clipIdx)
                         {
                             for(auto& m : outputRegions[reg])
                                 regClipped.meshes.push_back(m);
                             ++reg;
                         }
                         ++clipIdx;
                     }
                 }
                 else if(clip->type == ClipInfo::Type::Polygon)
                 {
                     if(polyIndices[reg - 1] == clipIdx)
                     {
                         for(auto& m : outputRegions[reg])
                             regClipped.meshes.push_back(m);
                         ++reg;
                     }
                     ++clipIdx;
                 }
                 if(!regClipped.meshes.empty())
                     computedRegions.push_back(regClipped);
            }
            wasClipped = computedRegions.size() > 1;
        }
    }
}

MeshClipper::RegionResult MeshClipper::GetRegions(bvector<uint64_t>& ids, bvector<bvector<PolyfaceHeaderPtr>>& polyfaces)
{

    if (!WasClipped())
        return MeshClipper::RegionResult::ClippingNotComputed;

    if (computedRegions.size() == 0)
        return MeshClipper::RegionResult::NoData;

    bvector<bpair<uint64_t, PolyfaceHeaderPtr>> vec;

    for (auto& reg : computedRegions)
    {
        ids.push_back(reg.id);
        polyfaces.push_back(reg.meshes);
    }
    return MeshClipper::RegionResult::Success;
}

MeshClipper::RegionResult MeshClipper::GetInOrOutRegion(PolyfaceHeaderPtr& mesh, const bool value)
    {
    if (!WasClipped())
        return MeshClipper::RegionResult::ClippingNotComputed;

    if (computedRegions.size() == 0)
        return MeshClipper::RegionResult::NoData;

    mesh = PolyfaceHeader::CreateFixedBlockIndexed(3);
    auto reg = std::find_if(computedRegions.begin(), computedRegions.end(), [&value](ClippedRegion& r) { return r.isExterior == value; });
    if (reg == computedRegions.end() || reg->meshes.empty() || reg->meshes[0] == nullptr)
        return RegionResult::NoData;
    mesh->CopyFrom(*reg->meshes[0]);

    for (size_t i = 1; i < reg->meshes.size(); ++i)
        mesh->AddIfMatchedLayout(*reg->meshes[i]);
    return MeshClipper::RegionResult::Success;
    }

MeshClipper::RegionResult MeshClipper::GetExteriorRegion(PolyfaceHeaderPtr& mesh)
    {
    return GetInOrOutRegion(mesh, false);
    }

MeshClipper::RegionResult MeshClipper::GetInteriorRegion(PolyfaceHeaderPtr& mesh)
    {
    return GetInOrOutRegion(mesh, true);
    }

MeshClipper::RegionResult MeshClipper::GetClippedMesh(PolyfaceHeaderPtr& mesh)
    {
    if(wasClipped)
        {
        if(orderedClipList[0]->isClipMask())
            mesh = computedRegions[0].meshes.front();
        else
            mesh = computedRegions[1].meshes.front();

        return MeshClipper::RegionResult::Success;
        }

    if (orderedClipList[0]->isClipMask())
        return MeshClipper::RegionResult::NoIntersectionWithClips;
    else
        return MeshClipper::RegionResult::Success;

    //for(auto& region : computedRegions)
    //    {
    //    if(region.id == 0)
    //        {
    //        mesh = region.meshes.front();
    //        break;
    //        }
    //    }
    //SelectRegions(RegionFilter::Boundary, RegionFilterMode::IncludeSelected);
    //if(selectionError != MeshClipper::RegionResult::Success)
    //    return selectionError;
    //GetSelectedRegion(mesh);
    //if(selectionError != MeshClipper::RegionResult::Success)
    //    {
    //    // Maybe the set of clips doesn't include a boundary clip
    //    SelectRegions(RegionFilter::Exterior, RegionFilterMode::IncludeSelected);
    //    if(selectionError != MeshClipper::RegionResult::Success)
    //        return selectionError;
    //    GetSelectedRegion(mesh);
    //    if(selectionError != MeshClipper::RegionResult::Success)
    //        return selectionError;
    //    }
    }

bool MeshClipper::WasClipped()
{
    return wasClipped;
}

MeshClipper::ClipInfo* MeshClipper::FindMatchingClip(uint64_t id)
{
    auto iter = std::find_if(orderedClipList.begin(), orderedClipList.end(), [id](ClipInfo* a) {
        return (a->id == id);
    });
    if (iter == orderedClipList.end())
        return nullptr;
    return *iter;
}

void MeshClipper::SelectRegions(MeshClipper::RegionFilter filter, MeshClipper::RegionFilterMode mode)
{
    if (!WasClipped())
    {
        selectionError = MeshClipper::RegionResult::ClippingNotComputed;
        return;
    }

    if (computedRegions.size() == 0)
    {
        selectionError = MeshClipper::RegionResult::NoData;
        return;
    }

    if (mode == MeshClipper::RegionFilterMode::SelectedIDs || mode == MeshClipper::RegionFilterMode::ExcludeSelectedIDs)
    {
        selectionError = MeshClipper::RegionResult::InsufficientRegionFilterArguments;
        return;
    }
    for (auto& region : computedRegions)
    {
        bool isRegionSuitable = false;
        if (FindMatchingClip(region.id) == nullptr)
            continue;
        switch (filter)
        {
        case MeshClipper::RegionFilter::Boundary:
            isRegionSuitable = (mode == MeshClipper::RegionFilterMode::IncludeSelected && !FindMatchingClip(region.id)->isClipMask()) ||
                (mode == MeshClipper::RegionFilterMode::ExcludeSelected && FindMatchingClip(region.id)->isClipMask());
            break;
        case MeshClipper::RegionFilter::Mask:
            isRegionSuitable = (mode == MeshClipper::RegionFilterMode::IncludeSelected && FindMatchingClip(region.id)->isClipMask()) ||
                (mode == MeshClipper::RegionFilterMode::ExcludeSelected && !FindMatchingClip(region.id)->isClipMask());
            break;
        case MeshClipper::RegionFilter::BoundaryOrExterior:
            isRegionSuitable = (mode == MeshClipper::RegionFilterMode::IncludeSelected && (region.isExterior || !FindMatchingClip(region.id)->isClipMask())) ||
                (mode == MeshClipper::RegionFilterMode::ExcludeSelected && !(region.isExterior || !FindMatchingClip(region.id)->isClipMask()));
            break;
        case MeshClipper::RegionFilter::MaskOrExterior:
            isRegionSuitable = (mode == MeshClipper::RegionFilterMode::IncludeSelected && (region.isExterior || FindMatchingClip(region.id)->isClipMask())) ||
                (mode == MeshClipper::RegionFilterMode::ExcludeSelected && !(region.isExterior || FindMatchingClip(region.id)->isClipMask()));
            break;
        case MeshClipper::RegionFilter::Exterior:
            isRegionSuitable = (mode == MeshClipper::RegionFilterMode::IncludeSelected && region.isExterior) ||
                (mode == MeshClipper::RegionFilterMode::ExcludeSelected && !region.isExterior);
            break;
        case MeshClipper::RegionFilter::All:
            isRegionSuitable = mode == MeshClipper::RegionFilterMode::IncludeSelected;
            break;
        default:
            break;
        };

        if (isRegionSuitable)
            selectedRegions.push_back(&region);
    }
    selectionError = MeshClipper::RegionResult::Success;
}
void MeshClipper::SelectRegions(MeshClipper::RegionFilter filter, MeshClipper::RegionFilterMode mode, bvector<uint64_t>& idsFilter)
{
    if (!WasClipped())
    {
        selectionError = MeshClipper::RegionResult::ClippingNotComputed;
        return;
    }

    if (computedRegions.size() == 0)
    {
        selectionError = MeshClipper::RegionResult::NoData;
        return;
    }
    for (auto& region : computedRegions)
    {
        bool isRegionSuitable = false;
        switch (filter)
        {
        case MeshClipper::RegionFilter::Boundary:
            isRegionSuitable = (mode == MeshClipper::RegionFilterMode::IncludeSelected && !FindMatchingClip(region.id)->isClipMask()) ||
                (mode == MeshClipper::RegionFilterMode::ExcludeSelected && FindMatchingClip(region.id)->isClipMask()) ||
                (mode == MeshClipper::RegionFilterMode::SelectedIDs && !FindMatchingClip(region.id)->isClipMask() && std::find(idsFilter.begin(), idsFilter.end(), region.id) != idsFilter.end()) ||
                (mode == MeshClipper::RegionFilterMode::ExcludeSelectedIDs && FindMatchingClip(region.id)->isClipMask() && std::find(idsFilter.begin(), idsFilter.end(), region.id) == idsFilter.end());
            break;
        case MeshClipper::RegionFilter::Mask:
            isRegionSuitable = (mode == MeshClipper::RegionFilterMode::IncludeSelected && FindMatchingClip(region.id)->isClipMask()) ||
                (mode == MeshClipper::RegionFilterMode::ExcludeSelected && !FindMatchingClip(region.id)->isClipMask()) ||
                (mode == MeshClipper::RegionFilterMode::SelectedIDs && FindMatchingClip(region.id)->isClipMask() && std::find(idsFilter.begin(), idsFilter.end(), region.id) != idsFilter.end()) ||
                (mode == MeshClipper::RegionFilterMode::ExcludeSelectedIDs && !FindMatchingClip(region.id)->isClipMask() && std::find(idsFilter.begin(), idsFilter.end(), region.id) == idsFilter.end());
            break;
        case MeshClipper::RegionFilter::BoundaryOrExterior:
            isRegionSuitable = (mode == MeshClipper::RegionFilterMode::IncludeSelected && (region.isExterior || !FindMatchingClip(region.id)->isClipMask())) ||
                (mode == MeshClipper::RegionFilterMode::ExcludeSelected && !(region.isExterior || !FindMatchingClip(region.id)->isClipMask())) ||
                (mode == MeshClipper::RegionFilterMode::SelectedIDs && (region.isExterior || !FindMatchingClip(region.id)->isClipMask()) && std::find(idsFilter.begin(), idsFilter.end(), region.id) != idsFilter.end()) ||
                (mode == MeshClipper::RegionFilterMode::ExcludeSelectedIDs && !(region.isExterior || !FindMatchingClip(region.id)->isClipMask()) && std::find(idsFilter.begin(), idsFilter.end(), region.id) == idsFilter.end());
            break;
        case MeshClipper::RegionFilter::MaskOrExterior:
            isRegionSuitable = (mode == MeshClipper::RegionFilterMode::IncludeSelected && (region.isExterior || FindMatchingClip(region.id)->isClipMask())) ||
                (mode == MeshClipper::RegionFilterMode::ExcludeSelected && !(region.isExterior || FindMatchingClip(region.id)->isClipMask())) ||
                (mode == MeshClipper::RegionFilterMode::SelectedIDs && (region.isExterior || FindMatchingClip(region.id)->isClipMask()) && std::find(idsFilter.begin(), idsFilter.end(), region.id) != idsFilter.end()) ||
                (mode == MeshClipper::RegionFilterMode::ExcludeSelectedIDs && !(region.isExterior || FindMatchingClip(region.id)->isClipMask()) && std::find(idsFilter.begin(), idsFilter.end(), region.id) == idsFilter.end());
            break;
        case MeshClipper::RegionFilter::Exterior:
            isRegionSuitable = (mode == MeshClipper::RegionFilterMode::IncludeSelected && region.isExterior) ||
                (mode == MeshClipper::RegionFilterMode::ExcludeSelected && !region.isExterior) ||
                (mode == MeshClipper::RegionFilterMode::SelectedIDs && region.isExterior && std::find(idsFilter.begin(), idsFilter.end(), region.id) != idsFilter.end()) ||
                (mode == MeshClipper::RegionFilterMode::ExcludeSelectedIDs && !region.isExterior && std::find(idsFilter.begin(), idsFilter.end(), region.id) == idsFilter.end());
            break;
        case  MeshClipper::RegionFilter::All:
            isRegionSuitable = mode == MeshClipper::RegionFilterMode::IncludeSelected ||
                mode == MeshClipper::RegionFilterMode::SelectedIDs && std::find(idsFilter.begin(), idsFilter.end(), region.id) != idsFilter.end() ||
                mode == MeshClipper::RegionFilterMode::ExcludeSelectedIDs && std::find(idsFilter.begin(), idsFilter.end(), region.id) == idsFilter.end();
            break;
        default:
            break;
        };

        if (isRegionSuitable)
            selectedRegions.push_back(&region);
    }
    selectionError = MeshClipper::RegionResult::Success;
}
MeshClipper::RegionResult MeshClipper::GetSelectedRegion(PolyfaceHeaderPtr& mesh)
{
    if (selectionError != MeshClipper::RegionResult::Success)
        return selectionError;
    if (selectedRegions.empty())
        return MeshClipper::RegionResult::NoRegionsMatchingSelection;

    if(selectedRegions.front()->meshes.empty())
        return MeshClipper::RegionResult::NoData;
    mesh = selectedRegions.front()->meshes.front();
    if (selectedRegions.size() > 1)
        return MeshClipper::RegionResult::MoreThanOneRegionMatchingSelection;
    return selectionError;
}

bvector<bpair<uint64_t, PolyfaceHeaderPtr>> MeshClipper::GetSelectedRegions(MeshClipper::RegionResult& result)
{
    bvector<bpair<uint64_t, PolyfaceHeaderPtr>> vec;
    if (selectionError != MeshClipper::RegionResult::Success)
    {
        result = selectionError;
        return vec;
    }


    if (selectedRegions.empty())
        result = MeshClipper::RegionResult::NoRegionsMatchingSelection;

    for (auto& reg : selectedRegions)
    {
        for (auto& m : reg->meshes)
            vec.push_back(make_bpair(reg->id, m));
    }
    result = MeshClipper::RegionResult::Success;
    return vec;
}

void MeshClipper::ClearSelection()
    {
    selectedRegions.clear();
    }

END_BENTLEY_SCALABLEMESH_NAMESPACE
POP_DISABLE_DEPRECATION_WARNINGS
