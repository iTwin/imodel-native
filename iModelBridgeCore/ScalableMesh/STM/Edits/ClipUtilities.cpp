#include "ScalableMeshPCH.h" 
#include "../ImagePPHeaders.h"
#include "ClipUtilities.h"
#include <TerrainModel/TerrainModel.h>
#include <ScalableMesh/ScalableMeshUtilityFunctions.h>
#include <Vu/VuApi.h>
#include <TerrainModel\TerrainModel.h>
#include <TerrainModel/Core/DTMIterators.h>
#include <array>
#include "..\ScalableMeshQuery.h"
USING_NAMESPACE_BENTLEY_TERRAINMODEL
BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE
#define SM_TRACE_CLIPS_GETMESH 0
const wchar_t* s_path = L"E:\\output\\scmesh\\2016-06-04\\";

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
    std::array<int32_t, 2> idx = { 0, 1};
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
        std::array<bpair<int32_t, std::array<DPoint2d, 3>>, 2> values = { make_bpair(lastIdx, sortedUvs), make_bpair(-1, sortedUvs) };
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
    std::array<int32_t, 3> idx = { 0, 1, 2 };
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
    std::array<int32_t, 3> idx = { 0, 1, 2 };
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
    std::array<int32_t, 2> idx = { 0, 1};
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
        if (bsiDPoint2d_pointEqualTolerance(&newUvs[0], &newUvs[1], 1e-3)) newUvs[1] = DPoint2d::From(DBL_MAX, DBL_MAX);
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

struct ClippedPolyfaceOutput : public PolyfaceQuery::IClipToPlaneSetOutput
    {
    PolyfaceHeaderPtr m_mesh;
    virtual StatusInt   _ProcessUnclippedPolyface(PolyfaceQueryCR polyfaceQuery)
        {
        m_mesh = PolyfaceHeader::CreateFixedBlockCoordinates(3);
        return SUCCESS;
        }
    virtual StatusInt   _ProcessClippedPolyface(PolyfaceHeaderR polyfaceHeader)
        {
        m_mesh = PolyfaceHeader::CreateFixedBlockCoordinates(3);
        m_mesh->CopyFrom(polyfaceHeader);
        return SUCCESS;
        }

    };

void DifferenceSetWithTracking::ResolveConflicts(bvector<bvector<int32_t>>& mergedFaces, const DPoint3d* triangle, bvector<int32_t>& conflictFaces, const DPoint3d* vertices)
    {
    bvector<std::array<int32_t,3>> listOfFaces[2];
    bvector<std::array<DPoint3d,3>> faceData[2];

    size_t id = 0;
    for (int32_t face : conflictFaces)
        {
        if (face == -1)
            {
            id = 1;
            continue;
            }
        listOfFaces[id].push_back({ addedFaces[face * 3], addedFaces[face * 3 + 1], addedFaces[face * 3 + 2] });
        faceData[id].push_back({ addedFaces[face * 3] >= firstIndex ? addedVertices[addedFaces[face * 3] - firstIndex] : vertices[addedFaces[face * 3] - 1],
                           addedFaces[face * 3 + 1] >= firstIndex ? addedVertices[addedFaces[face * 3 + 1] - firstIndex] : vertices[addedFaces[face * 3 + 1] - 1],
                           addedFaces[face * 3 + 2] >= firstIndex ? addedVertices[addedFaces[face * 3 + 2] - firstIndex] : vertices[addedFaces[face * 3 + 2] - 1] });
        
        }
    bvector<bvector<int32_t>> faceContours[2];
    bvector<bvector<DPoint3d>> contourData[2];
    if (listOfFaces[0].size() == 0 || listOfFaces[1].size() == 0) return;
    PolygonFromFaceList(faceContours[0],listOfFaces[0], firstIndex+addedVertices.size()+1);
    PolygonFromFaceList(faceContours[1], listOfFaces[1],firstIndex + addedVertices.size()+1);
    bmap<DPoint3d, int32_t, DPoint3dZYXTolerancedSortComparison> mapOfPoints(DPoint3dZYXTolerancedSortComparison(1e-5, 0));
    for (auto& contour : faceContours[0])
        {
        contourData[0].push_back(bvector<DPoint3d>());
        for (auto i : contour)
            {
            contourData[0][&contour - &faceContours[0][0]].push_back(i >= firstIndex ? addedVertices[i - firstIndex] : vertices[i - 1]);
            mapOfPoints[contourData[0][&contour - &faceContours[0][0]].back()] = i;
            }
        }
    for (auto& contour : faceContours[1])
        {
        contourData[1].push_back(bvector<DPoint3d>());
        for (auto i : contour)
            {
            contourData[1][&contour - &faceContours[1][0]].push_back(i >= firstIndex ? addedVertices[i - firstIndex] : vertices[i - 1]);
            mapOfPoints[contourData[1][&contour - &faceContours[1][0]].back()] = i;
            }
        }

    auto    polyface = PolyfaceHeader::CreateVariableSizeIndexed();
    for (auto& contour : contourData[0]) polyface->AddPolygon(contour);
    ClippedPolyfaceOutput meshOut;
    for (auto& contour : contourData[1])
        {
        bvector < DPoint2d> poly2d;
        for (size_t i = 0; i < contour.size(); ++i)
            {
            poly2d.push_back(DPoint2d::From(contour[i]));
            }
        if (poly2d.size() < 3) return;
        auto clipVec = ClipVector::CreateFromPrimitive(ClipPrimitive::CreateFromShape(&poly2d[0], contour.size(), false, NULL, NULL, NULL));
        // auto    polyface = PolyfaceHeader::CreateVariableSizeIndexed();
        // polyface->AddPolygon(contourData[0]);
        clipVec->ClipPolyface(*polyface, meshOut, true);
        if (meshOut.m_mesh == nullptr) return;
        polyface->CopyFrom(*(meshOut.m_mesh));
        }
    for (PolyfaceVisitorPtr addedFacets = PolyfaceVisitor::Attach(*(meshOut.m_mesh)); addedFacets->AdvanceToNextFace();)
        {
        bvector<int32_t> face;
        for (size_t i = 0; i < 3; ++i)
            {
            face.push_back(mapOfPoints[*(addedFacets->GetPointCP() + i)]);
            }
        mergedFaces.push_back(face);
        }
    }

void DifferenceSetWithTracking::ApplySetMerge(DifferenceSetWithTracking& d, int firstIndx, const DPoint3d* vertices)
    {
    size_t firstId = addedVertices.size() + firstIndex;
    addedVertices.insert(addedVertices.end(), d.addedVertices.begin(), d.addedVertices.end());
    vector<bool> removed(firstIndex, false);
    for (auto idx : removedVertices)
        {
        removed[idx] = true;
        }
    for (auto idx : d.removedVertices)
        {
        removed[idx] = true;
        }
    removedVertices.clear();
    for (int32_t i = 0; i < firstIndex; ++i)
        {
        if (removed[i]) removedVertices.push_back(i);
        }
    bvector<int32_t> faces;
    size_t nOfRemovedFaces = removedFaces.size();
    size_t nOfAddedFaces = addedFaces.size();
    removedFaces.insert(removedFaces.end(), d.removedFaces.begin(), d.removedFaces.end());
    //faces.clear();
    for (int32_t idx : d.addedFaces)
        {
        if (idx < d.firstIndex) addedFaces.push_back(idx + firstIndx);
        else addedFaces.push_back(idx - d.firstIndex + (int32_t)firstId);
        }
    bvector<bvector<int32_t>> mapOfRemovedFaces(removedFaces.size() / 3);
    for (int32_t fId : d.splitFaces)
        {
        for (size_t i = 0; i < removedFaces.size(); i += 3)
            {
            bool isFace = ((d.removedFaces[fId * 3] == removedFaces[i] || d.removedFaces[fId * 3] == removedFaces[i + 1] || d.removedFaces[fId * 3] == removedFaces[i + 2]) &&
                           (d.removedFaces[fId * 3 + 1] == removedFaces[i] || d.removedFaces[fId * 3 + 1] == removedFaces[i + 1] || d.removedFaces[fId * 3 + 1] == removedFaces[i + 2]) &&
                           (d.removedFaces[fId * 3 + 2] == removedFaces[i] || d.removedFaces[fId * 3 + 2] == removedFaces[i + 1] || d.removedFaces[fId * 3 + 2] == removedFaces[i + 2]));
            if (isFace)
                {
                splitFaces.push_back((int32_t)i / 3);
                if(i < nOfRemovedFaces) mapOfRemovedFaces[(int32_t)i / 3].push_back(fId);
                break;
                }
            }
        }


    vector<bvector<int32_t>> conflicts(removedFaces.size() / 3);
    vector<bvector<bvector<int32_t>>> mergedFaces(removedFaces.size() / 3);
    vector<bool> facesNotAdded(addedFaces.size() / 3, false);
    for (int32_t& id : splitFaces)
        {
        if ((&id - &splitFaces[0]) == nOfAddedFaces / 3) conflicts[id].push_back(-1);
        conflicts[id].push_back(&id - &splitFaces[0]);
        }
    for (auto& conflict : conflicts)
        {
        if (conflict.size() == 0) continue;
        size_t id = &conflict - &conflicts[0];
        if (mapOfRemovedFaces[id].size() == 0) continue;
        for (auto& faceId : conflict) if (faceId != -1)facesNotAdded[faceId] = true;

        }

    faces.clear();
    bvector<int32_t> splits;
    for (size_t i = 0; i < addedFaces.size(); i += 3)
        {
        if (!facesNotAdded[i / 3])
            {
            faces.insert(faces.end(), &addedFaces[i], &addedFaces[i + 3]);
            splits.push_back(splitFaces[i / 3]);
            }
        }
    for (auto&list: mergedFaces)
        for (auto& face : list)
            {
            assert(face.size() == 3);
            faces.insert(faces.end(), face.begin(), face.end());
            splits.push_back(&list - &mergedFaces[0]);
            }
    addedFaces = faces;
    splitFaces = splits;
    }

void Clipper::ClipVertices(DifferenceSet& d, bvector<bool>& removed, const DPoint3d* poly, size_t polySize)
    {
    DPoint3d rangeTopLeft = DPoint3d::From(m_range.low.x, m_range.high.y, 0);
    DPoint3d rangeBottomRight = DPoint3d::From(m_range.low.y, m_range.high.x, 0);
    bool nodeWithinPolygon = bsiGeom_isXYPointInConvexPolygon(&m_range.low, poly, (int)polySize, 0) &&
        bsiGeom_isXYPointInConvexPolygon(&rangeTopLeft, poly, (int)polySize, 0) &&
        bsiGeom_isXYPointInConvexPolygon(&rangeBottomRight, poly, (int)polySize, 0) &&
        bsiGeom_isXYPointInConvexPolygon(&m_range.high, poly, (int)polySize, 0);
    for (size_t i = 0; i < m_nVertices; ++i)
        if (nodeWithinPolygon || bsiGeom_isXYPointInConvexPolygon(&m_vertexBuffer[i], poly, (int)polySize, 0))
            {
            d.removedVertices.push_back((int32_t)i);
            removed[i] = true;
            }
    }

DifferenceSet Clipper::TriangulateAroundHole(const DPoint3d* triangle, const vector<vector<int32_t>>& lines, const vector<DPoint3d>& pts)
    {
    DifferenceSet d;
    d.firstIndex = 4;
    BENTLEY_NAMESPACE_NAME::TerrainModel::DTMPtr dtmPtr;
    int status = CreateBcDTM(dtmPtr);
    BC_DTM_OBJ* dtmObjP(dtmPtr->GetBcDTM()->GetTinHandle());
    vector<DPoint3d> points;
    status = bcdtmObject_storeDtmFeatureInDtmObject(dtmObjP, DTMFeatureType::Breakline, dtmObjP->nullUserTag, 1, &dtmObjP->nullFeatureId, triangle, 3);
    for (auto line : lines)
        {
        points.clear();
        for (auto id : line)
            points.push_back(pts[id]);
        status = bcdtmObject_storeDtmFeatureInDtmObject(dtmObjP, DTMFeatureType::DrapeVoid, dtmObjP->nullUserTag, 1, &dtmObjP->nullFeatureId, &points[0], (long)points.size());
        }
    status = bcdtmObject_triangulateDtmObject(dtmObjP);
    DPoint3d* ptsP;
    long numPts, numMeshFaces;
    long* meshFacesP;
    bmap<size_t, int32_t> indexMap;
    status = bcdtmLoad_tinMeshFromDtmObject(dtmObjP, true, 20000, false, DTMFenceType::None, DTMFenceOption::None, NULL, 0, (DPoint3d**)&ptsP, &numPts, &meshFacesP, &numMeshFaces);
    for (size_t face = 0; face < numMeshFaces; ++face)
        {
        DPoint3d p = ptsP[meshFacesP[face] - 1];
        bool isInTriangle = false;
        for (size_t i = 0; i < 3; ++i)
            {
            if (fabs(p.x - triangle[i].x) < 1e-5 &&  fabs(p.y - triangle[i].y) < 1e-5)
                {
                d.addedFaces.push_back((int32_t)i+1);
                isInTriangle = true;
                break;
                }
            }
        if (!isInTriangle && indexMap.count(meshFacesP[face]) == 0)
            {
            d.addedVertices.push_back(p);
            indexMap[meshFacesP[face]] = (int)d.addedVertices.size() - 1 + d.firstIndex;
            d.addedFaces.push_back(indexMap[meshFacesP[face]]);
            }
        }
    d.removedFaces.push_back(1);
    d.removedFaces.push_back(2);
    d.removedFaces.push_back(3);
    return d;
    }

DifferenceSet Clipper::Triangulate(const vector<DPoint3d>& pts, const vector<vector<int32_t>>& lines, const DPoint3d* triangle, const bool* triangleMask, const vector<vector<int32_t>>& triangleIntersects)
    {
    DifferenceSet d;
    d.firstIndex = 4;
    vector<DPoint3d> toAdd;
    vector<vector<int32_t>> polys(1);
    for (size_t i = 0; i < 3; ++i)
        {
        if (triangleMask[i]) continue;
        if (triangleIntersects[i].size() == 0) continue;
        polys.back().push_back((int)i);
        int32_t id = triangleIntersects[i].front();
        int32_t lastId = id;
        toAdd.push_back(pts[id]);
        polys.back().push_back((int32_t)(toAdd.size() - 1 + d.firstIndex));
        for (auto& line : lines)
            {
            if (line.size() == 0) continue;
            if (line.front() == id)
                {
                for (auto it = line.begin() + 1; it != line.end(); ++it)
                    {
                    toAdd.push_back(pts[*it]);
                    polys.back().push_back((int32_t)(toAdd.size() - 1 + d.firstIndex));
                    lastId = *it;
                    }
                break;
                }
            else if (line.back() == id)
                {
                for (auto it = line.rbegin() + 1; it != line.rend(); ++it)
                    {
                    toAdd.push_back(pts[*it]);
                    polys.back().push_back((int32_t)(toAdd.size() - 1 + d.firstIndex));
                    lastId = *it;
                    }
                break;
                }
            }
        for (size_t j =0; j < 3 && lastId != id; ++j)
            {
            if (triangleIntersects[j].size() == 0) continue;
            if (lastId == triangleIntersects[j].front() && !triangleMask[j])
                {
                polys.back().push_back((int32_t)j);
                break;
                }
            else if (lastId == triangleIntersects[j].back())
                {
                polys.back().push_back((int32_t)(j + 1) % 3);
                if (j == i) polys.back().push_back((int32_t)(j + 2) % 3);
                break;
                }
            }
        polys.back().push_back((int32_t)i);
        polys.push_back(vector<int32_t>());
        }
    d.addedVertices.insert(d.addedVertices.begin(), toAdd.begin(), toAdd.end());
    BENTLEY_NAMESPACE_NAME::TerrainModel::DTMPtr dtmPtr;

    for (auto& poly : polys)
        {
        if (poly.size() < 3) continue;
        vector<DPoint3d> points;
        for (int32_t polyPt : poly)
            {
            if (polyPt < 4) points.push_back(triangle[polyPt]);
            else points.push_back(toAdd[polyPt - 4]);
            }
        dtmPtr = NULL;
        std::string s;
        PrintPoints(s,points);
        int status = CreateBcDTM(dtmPtr);
        BC_DTM_OBJ* dtmObjP(dtmPtr->GetBcDTM()->GetTinHandle());
        status = bcdtmObject_storeDtmFeatureInDtmObject(dtmObjP, DTMFeatureType::Breakline, dtmObjP->nullUserTag, 1, &dtmObjP->nullFeatureId, &points[0], (long)points.size());
        status = bcdtmObject_triangulateDtmObject(dtmObjP);
        if (status != SUCCESS) continue;
        DPoint3d* ptsP = NULL;
        long numPts, numMeshFaces;
        long* meshFacesP = NULL;
        status = bcdtmLoad_tinMeshFromDtmObject(dtmObjP, true, 20000,false, DTMFenceType::None, DTMFenceOption::None, NULL, 0, (DPoint3d**)&ptsP, &numPts, &meshFacesP, &numMeshFaces);
        for (size_t face = 0; face < numMeshFaces*3; ++face)
            {
            DPoint3d p = ptsP[meshFacesP[face] - 1];
            bool found = false;
            for (size_t i = 0; i < toAdd.size(); ++i)
                {
                if (fabs(p.x - toAdd[i].x) < 1e-5 &&  fabs(p.y - toAdd[i].y) < 1e-5)
                    {
                    d.addedFaces.push_back((int)i+4);
                    found = true;
                    break;
                    }
                }
            for (size_t i = 0; i < 3 && !found; ++i)
                {
                if (fabs(p.x - triangle[i].x) < 1e-5 &&  fabs(p.y - triangle[i].y) < 1e-5)
                    {
                    d.addedFaces.push_back((int)i+1);
                    found = true;
                    break;
                    }
                }
            if (!found)
                {
                d.addedVertices.push_back(p);
                toAdd.push_back(p);
                d.addedFaces.push_back((int32_t)d.addedVertices.size() - 1 + 4);
                }
            }
        }
    d.removedFaces.push_back(1);
    d.removedFaces.push_back(2);
    d.removedFaces.push_back(3);
    return d;
    }

void Clipper::ClassifyPointForTriangle(PointClassification* classif, const DPoint3d* pt, const DPoint3d* triangle)
    {
    switch (bsiGeom_XYPolygonParity(pt, triangle, 3, 1.0e-5))
        {
        case 0:
            *classif = PointClassification::ON;
            break;
        case 1: 
            *classif = PointClassification::INSIDE;
            break;
        case -1:
            *classif = PointClassification::OUTSIDE;
            break;
        default:
            *classif = PointClassification::UNKNOWN;
            break;
        }
    }

DifferenceSet Clipper::ClipTriangle(const DPoint3d* triangle, const bool* triangleMask, const DPoint3d* poly, size_t polySize, DRange3d polyExt)
    {
    DifferenceSet d;
    DRange3d triangleExt = DRange3d::From(triangle, 3);
    if (!triangleExt.IntersectsWith(polyExt, 2)) return d;
    vector<DPoint3d> trianglePts;
    for (size_t i = 0; i < 3; ++i) if (!triangleMask[i]) trianglePts.push_back(triangle[i]);
    vector<DPoint3d> pts;
    vector<vector<int32_t>> triangleIntersects(3);
    vector<bmap<double, int32_t>> intersectOrder(3);
    vector<PointClassification> ptsClass;
    bool cut = false;
    for (size_t j = 0; j < polySize; ++j)
        {
        DRange3d segRange = DRange3d::From(poly[j], poly[(j + 1)%polySize]);
        DSegment3d polyEdge = DSegment3d::From(poly[j], poly[(j + 1)%polySize]);
        if (polyEdge.LengthSquared() < 1.0e-5) continue;
        pts.push_back(poly[j]);
        ptsClass.push_back(PointClassification::UNKNOWN);
        if (!triangleExt.IntersectsWith(segRange,2))
            {
            ptsClass.back() = PointClassification::OUTSIDE;
            continue;
            }
        DRay3d ray = DRay3d::FromOriginAndVector(poly[j], DVec3d::From(0, 0, -1));
        DPoint3d bary, projectedPt;
        double param = 0;
        bsiDRay3d_intersectTriangle(&ray, &projectedPt, &bary, &param, triangle);
        pts.back() = projectedPt;
        for (size_t k = 0; k < 3; ++k)
            {
            DSegment3d triangleEdge = DSegment3d::From(triangle[k], triangle[(k + 1) % 3]);
            double f0, f1;
            DPoint3d pt0, pt1;
            bool hasIntersect = DSegment3d::IntersectXY(f0, f1, pt0, pt1, polyEdge, triangleEdge);
            //intersection is not on a triangle vertex
            if (hasIntersect && f0 > -1.0e-5 && f0 < 1 + 1.0e-5 && f1 > 1.0e-5 && f1 < 1 - 1.0e-5)
                {
                intersectOrder[k][f1] = (int)pts.size();
                pts.push_back(pt1);
                ptsClass.push_back(PointClassification::ON);
                cut = true;
                }
            else if (hasIntersect && f1 > -1.0e-5 && f1 < 1 + 1.0e-5)
                {
                intersectOrder[k][f1] = (int)pts.size();
                pts.push_back(pt1);
                ptsClass.push_back(PointClassification::ON);
                }
            }
        }
    size_t nInsidePts = 0;
    size_t nOnPts = 0;
    vector<vector<int32_t>> cutLine(1);
    for (size_t i = 0; i < pts.size(); ++i)
        {
        if (ptsClass[i] == PointClassification::UNKNOWN) ClassifyPointForTriangle(&ptsClass[i], &pts[i], triangle);
        if (ptsClass[i] == PointClassification::INSIDE)
            {
            cutLine.back().push_back((int32_t)i);
            ++nInsidePts;
            }
        else if (ptsClass[i] == PointClassification::ON)
            {
            cutLine.back().push_back((int32_t)i);
            ++nOnPts;
            }
        else if (ptsClass[i] == PointClassification::OUTSIDE && cutLine.back().size() > 0)
            {
            cutLine.push_back(vector<int32_t>());
            }
        }
    for (size_t k = 0; k < 3;++k)
    for (auto it = intersectOrder[k].begin(); it != intersectOrder[k].end(); ++it)
        {
        triangleIntersects[k].push_back(it->second);
        }
    if (nOnPts == 0 && nInsidePts > 0) //all points inside
        {
        d = TriangulateAroundHole(triangle, cutLine, pts);
        }
    else if (cut == true || nInsidePts > 0) d = Triangulate(pts, cutLine, triangle, triangleMask, triangleIntersects);
        return d;
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
DifferenceSet Clipper::ClipNonConvexPolygon2D(const DPoint3d* poly, size_t polySize)
    {
    clock_t timeBegin = clock();
    if (s_useTMClips || bsiGeom_testXYPolygonConvex(poly, (int) polySize))
        {
        DifferenceSet d= ClipConvexPolygon2D(poly, polySize);
        double totalTime = (double)(clock() - timeBegin) / CLOCKS_PER_SEC;
        s_timeSpentClipping += totalTime;
        s_timeConvexPolys += totalTime;
        s_numberOfClips++;
        s_numberOfConvexClips++;
        if (s_minClipTime > totalTime)s_minClipTime = totalTime;
        if (s_maxClipTime < totalTime)s_maxClipTime = totalTime;
        return d;
        }
    //bvector<int> idx;
    vector<DPoint3d> nonConstPoly(polySize);
    memcpy(&nonConstPoly[0], poly, polySize*sizeof(DPoint3d));
    for (auto& pt : nonConstPoly) pt.z = 0;
    //vu_triangulateSpacePolygon(&idx,&nonConstPoly[0], (int)polySize, 1e-8, 0, true);

    bvector<bvector<DPoint3d>> convexPolys;
    vu_splitToConvexParts(&nonConstPoly.front(), (int)polySize, &convexPolys, [] (DPoint3d* pPts, int* flags, int size, void* pArg)
        {
        auto arg = (bvector<bvector<DPoint3d>>*) pArg;
        bvector<DPoint3d> vec(size);
        memcpy(&vec[0], pPts, size*sizeof(DPoint3d));
        arg->push_back(vec);
        });
    /*bvector<DPoint3d> convexPoly;
    for (size_t j = 0; j < idx.size(); ++j)
        {
        if (idx[j] == 0 && convexPoly.size() > 0)
            {
            convexPolys.push_back(convexPoly);
            convexPoly.clear();
            }
        else convexPoly.push_back(poly[abs(idx[j]) - 1]);
        }
    if (convexPoly.size() > 2)
        {
        convexPolys.push_back(convexPoly);
        convexPoly.clear();
        }*/
    DifferenceSetWithTracking d;
    d.firstIndex = (int32_t)m_nVertices+1;
    for (auto& polygon : convexPolys)
        {
        if (polygon.size() < 3) continue;
        DRange3d clipExt = DRange3d::From(&polygon[0], (int32_t)polygon.size());
        if (clipExt.IntersectsWith(m_range, 2))
            {
            DifferenceSetWithTracking polySet = ClipConvexPolygon2D(&polygon[0], polygon.size());
            d.ApplySetMerge(polySet,0,m_vertexBuffer);
            //d.ApplySet(polySet, 0);
            }
        }
    double totalTime = (double)(clock() - timeBegin) / CLOCKS_PER_SEC;
    s_timeSpentClipping += totalTime;
    s_timeNonConvexPolys += totalTime;
    s_numberOfClips++;
    s_numberOfNonConvexClips++;
    if (s_minClipTime > totalTime)s_minClipTime = totalTime;
    if (s_maxClipTime < totalTime)s_maxClipTime = totalTime;
    return d;
    }

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
    void CollectDifferenceInfoFromPolyfaceMesh(DifferenceSetWithTracking& d, PolyfaceQueryCR pf, vector<bool>& faceRemoved, vector<bool>& vertexRemoved, FaceLookupList&l, map<DPoint3d, int32_t, DPoint3dZYXTolerancedSortComparison> &mapOfPoints)
        {
        for (PolyfaceVisitorPtr addedFacets = PolyfaceVisitor::Attach(pf); addedFacets->AdvanceToNextFace();)
            {
            DPoint3d face[3];
            int32_t idx[3] = { -1, -1, -1 };
            for (size_t i = 0; i < 3; ++i)
                {
                face[i] = addedFacets->GetPointCP()[i];
                idx[i] = mapOfPoints.count(face[i]) != 0 ? mapOfPoints[face[i]] : -1;
                if (idx[i] != -1) vertexRemoved[idx[i]] = false;
                }
            if (idx[0] != -1 && idx[1] != -1 && idx[2] != -1)
                {
                int32_t existingFace = l.LookUpFace(idx[0] + 1, idx[1] + 1, idx[2] + 1);
                if (existingFace != -1)
                    {
                    faceRemoved[existingFace] = false;
                    continue;
                    }
                }
            for (size_t i = 0; i < 3; ++i)
                {
                if (idx[i] == -1)
                    {
                    idx[i] = (int)d.addedVertices.size() + d.firstIndex;
                    d.addedVertices.push_back(face[i]);
                    }
                else idx[i]++;
                }
            d.addedFaces.push_back(idx[0]);
            d.addedFaces.push_back(idx[1]);
            d.addedFaces.push_back(idx[2]);
            }
        }

DifferenceSetWithTracking Clipper::ClipPolygon2DDTM(const DPoint3d* poly, size_t polySize)
    {
    DifferenceSetWithTracking d;
    d.firstIndex = (int)m_nVertices+1;
    if (m_nVertices < 2000 || polySize < 200 || polySize > 1000) return d;
    map<DPoint3d, int32_t, DPoint3dZYXTolerancedSortComparison> mapOfPoints(DPoint3dZYXTolerancedSortComparison(1e-5,0));
    FaceLookupList l(m_nVertices+1);
    s_number++;    
    vector<bool> faceRemoved(m_nIndices / 3, true);

    for (size_t i = 0; i < m_nVertices; ++i)
        mapOfPoints[m_vertexBuffer[i]] = (int)i;
    for (size_t i = 0; i < m_nIndices; i += 3)
        {
        l.AddFace(m_indexBuffer[i], m_indexBuffer[i + 1], m_indexBuffer[i + 2], (int)i / 3);
        }
    vector<bool> vertexRemoved(m_nVertices, true);
    BENTLEY_NAMESPACE_NAME::TerrainModel::DTMPtr dtmPtr;
    MakeDTMFromIndexList(dtmPtr);
   // assert(status == SUCCESS);
    DTMUserTag    userTag = 0;
    DTMFeatureId* textureRegionIdsP = 0;
    long          numRegionTextureIds = 0;
    vector<DPoint3d> nonConstPoly(polySize);
    memcpy(&nonConstPoly[0], poly, polySize*sizeof(DPoint3d));
   /* WString nameBefore = L"d:\\clipping\\preclipmesh_";
    nameBefore.append(to_wstring(s_number).c_str());
    nameBefore.append(L".m");
    FILE* meshBeforeClip = _wfopen(nameBefore.c_str(), L"wb");
    fwrite(&m_nVertices, sizeof(size_t), 1, meshBeforeClip);
    fwrite(m_vertexBuffer, sizeof(DPoint3d), m_nVertices, meshBeforeClip);
    fwrite(&m_nIndices, sizeof(size_t), 1, meshBeforeClip);
    fwrite(m_indexBuffer, sizeof(int32_t), m_nIndices, meshBeforeClip);
    fclose(meshBeforeClip);
    WString namePoly = L"d:\\clipping\\preclippoly_";
    namePoly.append(to_wstring(s_number).c_str());
    namePoly.append(L".p");
    FILE* polyCliPFile = _wfopen(namePoly.c_str(), L"wb");
    fwrite(&polySize, sizeof(size_t), 1, polyCliPFile);
    fwrite(poly, sizeof(DPoint3d), polySize, polyCliPFile);
    fclose(polyCliPFile);*/

    bcdtmInsert_internalDtmFeatureMrDtmObject(dtmPtr->GetBcDTM()->GetTinHandle(),
                                                       DTMFeatureType::Region,
                                                       1,
                                                       2,
                                                       userTag,
                                                       &textureRegionIdsP,
                                                       &numRegionTextureIds,
                                                       &nonConstPoly[0],
                                                       (long)polySize);

   // assert(status == SUCCESS);
    BENTLEY_NAMESPACE_NAME::TerrainModel::DTMMeshEnumeratorPtr en = BENTLEY_NAMESPACE_NAME::TerrainModel::DTMMeshEnumerator::Create(*dtmPtr->GetBcDTM());

    en->SetExcludeAllRegions();
    en->SetMaxTriangles(dtmPtr->GetBcDTM()->GetTrianglesCount() * 2);
    size_t n = 0;
    for (PolyfaceQueryP pf : *en)
        {
        /*WString name = L"d:\\clipping\\postclipmesh_";
        name.append(to_wstring(s_number).c_str());
        name.append(L"_");
        name.append(to_wstring(n).c_str());
        name.append(L".m");
        FILE* meshAfterClip = _wfopen(name.c_str(), L"wb");
        size_t ptCount = pf->GetPointCount();
        size_t faceCount = pf->GetPointIndexCount();
        fwrite(&ptCount, sizeof(size_t), 1, meshAfterClip);
        fwrite(pf->GetPointCP(), sizeof(DPoint3d), ptCount, meshAfterClip);
        fwrite(&faceCount, sizeof(size_t), 1, meshAfterClip);
        fwrite(pf->GetPointIndexCP(), sizeof(int32_t), faceCount, meshAfterClip);
        fclose(meshAfterClip);*/
        n++;
        CollectDifferenceInfoFromPolyfaceMesh(d, *pf, faceRemoved,vertexRemoved, l, mapOfPoints);
        }
        if (n == 0)
            {
            return d;
            }
    for (size_t i = 0; i < faceRemoved.size(); ++i)
        {
        if (faceRemoved[i]) d.removedFaces.insert(d.removedFaces.end(), &m_indexBuffer[i * 3], &m_indexBuffer[i * 3 + 3]);
        }
    s_stop = true;
    return d;
    }

bool s_printDTM = false;

void Clipper::MakeDTMFromIndexList(BENTLEY_NAMESPACE_NAME::TerrainModel::DTMPtr& dtmPtr)
    {
    BC_DTM_OBJ* bcDtmP = 0;
    int dtmCreateStatus = bcdtmObject_createDtmObject(&bcDtmP);
    if (dtmCreateStatus == 0)
        {
        BcDTMPtr bcDtmObjPtr;
        bcDtmObjPtr = BcDTM::CreateFromDtmHandle(bcDtmP);
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
        WString dtmFileName(L"d:\\clipping\\outputTM");
        WString triangleF = dtmFileName;
        dtmFileName.append(L".dtm");
        BC_DTM_OBJ* bcDtmP = 0;
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
        }
    }


void Clipper::TagUVsOnPolyface(PolyfaceHeaderPtr& poly, BENTLEY_NAMESPACE_NAME::TerrainModel::DTMPtr& dtmPtr, FaceToUVMap& faceToUVMap)
    {
    vector<int32_t> indices(poly->GetPointIndexCount());
    memcpy(&indices[0], poly->GetPointIndexCP(), poly->GetPointIndexCount()*sizeof(int32_t));
    bmap<int32_t, int32_t> allPts;
    bmap<DPoint2d, int32_t, DPoint2dZYXTolerancedSortComparison> allUvs(DPoint2dZYXTolerancedSortComparison(1e-10));
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
    for (size_t i = 0; i < indices.size(); i += 3)
        {
        DPoint2d uvCoords[3];
        if (!faceToUVMap.GetFacet(&indices[i], uvCoords))
            {
            if (poly->Param().size() == 0)
                poly->Param().push_back(DPoint2d::From(0.0, 0.0));
            nFaceMisses++;
            poly->PointIndex().push_back(allPts[indices[i]] + 1);
            poly->PointIndex().push_back(allPts[indices[i + 1]] + 1);
            poly->PointIndex().push_back(allPts[indices[i + 2]] + 1);
            for (size_t uvI = 0; uvI < 3; ++uvI)
                poly->ParamIndex().push_back(1);
            continue;
            }
        poly->PointIndex().push_back(allPts[indices[i]] + 1);
        poly->PointIndex().push_back(allPts[indices[i + 1]] + 1);
        poly->PointIndex().push_back(allPts[indices[i + 2]] + 1);
        for (size_t uvI = 0; uvI < 3; ++uvI)
            {
            if (allUvs.count(uvCoords[uvI]) == 0)
                {
                poly->Param().push_back(uvCoords[uvI]);
                allUvs[uvCoords[uvI]] = (int)poly->Param().size();
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
    }

bool Clipper::GetRegionsFromClipPolys(bvector<bvector<PolyfaceHeaderPtr>>& polyfaces, bvector<bvector<DPoint3d>>& polygons)
    {
    BENTLEY_NAMESPACE_NAME::TerrainModel::DTMPtr dtmPtr;
    MakeDTMFromIndexList(dtmPtr);
    bvector<bpair<double, int>> metadata(polygons.size(), make_bpair(0,0));
    return GetRegionsFromClipPolys(polyfaces, polygons, metadata, dtmPtr);
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

bool Clipper::GetRegionsFromClipPolys(bvector<bvector<PolyfaceHeaderPtr>>& polyfaces, bvector<bvector<DPoint3d>>& polygons, bvector<bpair<double, int>>& metadata, BENTLEY_NAMESPACE_NAME::TerrainModel::DTMPtr& dtmPtr)
    {
    bool dbg = false;
    if (dbg)
        {
        WString nameBefore = WString(s_path)+L"fpreclipmeshregion_";
        nameBefore.append(to_wstring(s_nclip).c_str());
        nameBefore.append(L"_");
        nameBefore.append(to_wstring(m_range.low.x).c_str());
        nameBefore.append(L"_");
        nameBefore.append(to_wstring(m_range.low.y).c_str());
        nameBefore.append(L".m");
        FILE* meshBeforeClip = _wfopen(nameBefore.c_str(), L"wb");
        fwrite(&m_nVertices, sizeof(size_t), 1, meshBeforeClip);
        fwrite(m_vertexBuffer, sizeof(DPoint3d), m_nVertices, meshBeforeClip);
        fwrite(&m_nIndices, sizeof(size_t), 1, meshBeforeClip);
        fwrite(m_indexBuffer, sizeof(int32_t), m_nIndices, meshBeforeClip);
        fclose(meshBeforeClip);
        for (size_t j = 0; j < polygons.size(); ++j)
            {
            WString namePoly = WString(s_path) + L"fpreclippolyreg_";
            namePoly.append(to_wstring(s_nclip).c_str());
            namePoly.append(L"_");
            namePoly.append(to_wstring(j).c_str());
            namePoly.append(L"_");
            namePoly.append(to_wstring(m_range.low.x).c_str());
            namePoly.append(L"_");
            namePoly.append(to_wstring(m_range.low.y).c_str());
            namePoly.append(L".p");
            FILE* polyCliPFile = _wfopen(namePoly.c_str(), L"wb");
            size_t polySize = polygons[j].size();
            fwrite(&polySize, sizeof(size_t), 1, polyCliPFile);
            fwrite(&polygons[j][0], sizeof(DPoint3d), polySize, polyCliPFile);
            fclose(polyCliPFile);
            }
        }
    DTMUserTag    userTag = 0;
    DTMFeatureId* textureRegionIdsP = 0;
    long          numRegionTextureIds = 0;
    FaceToUVMap originalFaceMap(m_range);
    if (m_uvBuffer && m_uvIndices)
        {
        int32_t* toDTMIndexBuffer = new int32_t[m_nIndices];
        TranslateToDTMIndices(toDTMIndexBuffer, m_vertexBuffer, m_indexBuffer, dtmPtr, m_nIndices);
        originalFaceMap.ReadFrom(toDTMIndexBuffer, m_uvIndices, m_uvBuffer, m_nIndices);
        delete[] toDTMIndexBuffer;
        }
    if (dtmPtr->GetBcDTM()->GetTinHandle()->dtmState != DTMState::Tin) return false;
    polyfaces.resize(polygons.size() + 1);
    if (dbg) bcdtmWrite_toFileDtmObject(dtmPtr->GetBcDTM()->GetTinHandle(), (WString(s_path) + WString(L"featurepolytest") + WString(std::to_wstring(s_nclip).c_str()) + WString(L".tin")).c_str());
    int stat = DTM_SUCCESS;
    for (auto& poly : polygons)
        {
        bool applyClipPoly = metadata[&poly - &polygons[0]].second == 0;
        if (!applyClipPoly)
            {
            if (metadata[&poly - &polygons[0]].second == 1)
                {
                auto maxLength = std::max(m_range.XLength(), m_range.YLength());
                if (metadata[&poly - &polygons[0]].first / maxLength > 0.0025) applyClipPoly = true;
                }
            else if (metadata[&poly - &polygons[0]].second == 2)
                {
                if (metadata[&poly - &polygons[0]].first / (m_range.XLength()* m_range.YLength()) > 0.001) applyClipPoly = true;
                }
            }
        if (applyClipPoly)
            {

            stat = bcdtmInsert_internalDtmFeatureMrDtmObject(dtmPtr->GetBcDTM()->GetTinHandle(),
                                                             DTMFeatureType::Region,
                                                             1,
                                                             2,
                                                             userTag,
                                                             &textureRegionIdsP,
                                                             &numRegionTextureIds,
                                                             &poly[0],
                                                             (long)poly.size(),
                                                             m_uvBuffer && m_uvIndices ? GetInsertPointCallback(originalFaceMap, dtmPtr) : nullptr);

            userTag++;
            }
        }

    if (dbg) bcdtmWrite_toFileDtmObject(dtmPtr->GetBcDTM()->GetTinHandle(), (WString(s_path) + WString(L"featurepolytest") + WString(std::to_wstring(s_nclip).c_str()) + WString(L"_after.tin")).c_str());
    BENTLEY_NAMESPACE_NAME::TerrainModel::DTMMeshEnumeratorPtr en = BENTLEY_NAMESPACE_NAME::TerrainModel::DTMMeshEnumerator::Create(*dtmPtr->GetBcDTM());
    if (m_uvBuffer && m_uvIndices)en->SetUseRealPointIndexes(true);
    en->SetExcludeAllRegions();
    en->SetMaxTriangles(2000000);
    size_t no = 0;
    for (PolyfaceQueryP pf : *en)
        {
        PolyfaceHeaderPtr vec = PolyfaceHeader::CreateFixedBlockIndexed(3);
        vec->CopyFrom(*pf);
        if (m_uvBuffer && m_uvIndices) TagUVsOnPolyface(vec, dtmPtr, originalFaceMap);
        if (dbg)
            {
            WString name = WString(s_path) + L"fpostclipmeshnoutsideregion_";
            name.append(to_wstring(s_nclip).c_str());
            name.append(L"_");
            name.append(to_wstring(no).c_str());
            name.append(L"_");
            name.append(to_wstring(m_range.low.x).c_str());
            name.append(L"_");
            name.append(to_wstring(m_range.low.y).c_str());
            name.append(L".m");
            FILE* meshAfterClip = _wfopen(name.c_str(), L"wb");
            size_t ptCount = vec->GetPointCount();
            size_t faceCount = vec->GetPointIndexCount();
            fwrite(&ptCount, sizeof(size_t), 1, meshAfterClip);
            fwrite(vec->GetPointCP(), sizeof(DPoint3d), ptCount, meshAfterClip);
            fwrite(&faceCount, sizeof(size_t), 1, meshAfterClip);
            fwrite(vec->GetPointIndexCP(), sizeof(int32_t), faceCount, meshAfterClip);
            fclose(meshAfterClip);
            }
        polyfaces[0].push_back(vec);
        }
    for (size_t n = 0; n < polygons.size() && n < (size_t)userTag; ++n)
        {
        size_t n2 = 0;

        en->Reset();
        en->SetFilterRegionByUserTag(n);
        for (PolyfaceQueryP pf : *en)
            {
            PolyfaceHeaderPtr vec = PolyfaceHeader::CreateFixedBlockIndexed(3);
            vec->CopyFrom(*pf);
            if (m_uvBuffer && m_uvIndices) TagUVsOnPolyface(vec, dtmPtr, originalFaceMap);

            if (dbg)
                {
                WString name = WString(s_path) + L"fpostclipmeshregion_";
                name.append(to_wstring(s_nclip).c_str());
                name.append(L"_");
                name.append(to_wstring(n).c_str());
                name.append(L"_");
                name.append(to_wstring(n2).c_str());
                name.append(L"_");
                name.append(to_wstring(m_range.low.x).c_str());
                name.append(L"_");
                name.append(to_wstring(m_range.low.y).c_str());
                name.append(L".m");
                FILE* meshAfterClip = _wfopen(name.c_str(), L"wb");
                size_t ptCount = vec->GetPointCount();
                size_t faceCount = vec->GetPointIndexCount();
                fwrite(&ptCount, sizeof(size_t), 1, meshAfterClip);
                fwrite(vec->GetPointCP(), sizeof(DPoint3d), ptCount, meshAfterClip);
                fwrite(&faceCount, sizeof(size_t), 1, meshAfterClip);
                fwrite(vec->GetPointIndexCP(), sizeof(int32_t), faceCount, meshAfterClip);
                fclose(meshAfterClip);
                }
            ++n2;
            polyfaces[n + 1].push_back(vec);
            }

        }
    if (textureRegionIdsP != 0)
        {
        free(textureRegionIdsP);
        textureRegionIdsP = 0;
        }
    ++s_nclip;
    for (auto& polygon : polyfaces)
        if (polygon.size() > 0) return true;
    IPolyfaceConstructionPtr builder = IPolyfaceConstruction::New(*IFacetOptions::Create());
    PolyfaceQueryCarrier* poly = new PolyfaceQueryCarrier(3, false, m_nIndices, m_nVertices, m_vertexBuffer, m_indexBuffer, 0, 0, 0, m_uvBuffer && m_uvIndices ? m_nVertices : 0, m_uvBuffer && m_uvIndices ? m_uvBuffer : 0, m_uvIndices);
    builder->AddPolyface(*poly);
    delete poly;
    polyfaces[0].push_back(builder->GetClientMeshPtr());
    return true;
    }

DifferenceSet Clipper::ClipSeveralPolygons(bvector<bvector<DPoint3d>>& polygons)
    {
    DifferenceSetWithTracking d;
    d.firstIndex = (int)m_nVertices + 1;
    map<DPoint3d, int32_t, DPoint3dZYXTolerancedSortComparison> mapOfPoints(DPoint3dZYXTolerancedSortComparison(1e-5, 0));
    FaceLookupList l(m_nVertices + 1);
    s_number++;
    vector<bool> faceRemoved(m_nIndices / 3, true);

    for (size_t i = 0; i < m_nVertices; ++i)
        mapOfPoints[m_vertexBuffer[i]] = (int)i;
    for (size_t i = 0; i < m_nIndices; i += 3)
        {
        l.AddFace(m_indexBuffer[i], m_indexBuffer[i + 1], m_indexBuffer[i + 2], (int)i / 3);
        }
    vector<bool> vertexRemoved(m_nVertices, true);
    BENTLEY_NAMESPACE_NAME::TerrainModel::DTMPtr dtmPtr;
    MakeDTMFromIndexList(dtmPtr);
    DTMUserTag    userTag = 0;
    DTMFeatureId* textureRegionIdsP = 0;
    long          numRegionTextureIds = 0;
    for (auto& poly : polygons)
        {
        bcdtmInsert_internalDtmFeatureMrDtmObject(dtmPtr->GetBcDTM()->GetTinHandle(),
                                                  DTMFeatureType::Region,
                                                  1,
                                                  2,
                                                  userTag,
                                                  &textureRegionIdsP,
                                                  &numRegionTextureIds,
                                                  &poly[0],
                                                  (long)poly.size());
        userTag++;
        }
    BENTLEY_NAMESPACE_NAME::TerrainModel::DTMMeshEnumeratorPtr en = BENTLEY_NAMESPACE_NAME::TerrainModel::DTMMeshEnumerator::Create(*dtmPtr->GetBcDTM());

    en->SetExcludeAllRegions();
    en->SetMaxTriangles(dtmPtr->GetBcDTM()->GetTrianglesCount() * 2);
    size_t n = 0;
    for (PolyfaceQueryP pf : *en)
        {
        n++;
        CollectDifferenceInfoFromPolyfaceMesh(d, *pf, faceRemoved, vertexRemoved, l, mapOfPoints);
        }
    if (n == 0)
        {
        return d;
        }
    for (size_t i = 0; i < faceRemoved.size(); ++i)
        {
        if (faceRemoved[i]) d.removedFaces.insert(d.removedFaces.end(), &m_indexBuffer[i * 3], &m_indexBuffer[i * 3 + 3]);
        }
    return d;
    }

bool s_useOwnClip = false;
DifferenceSetWithTracking Clipper::ClipConvexPolygon2D(const DPoint3d* poly, size_t polySize)
    {
    if (s_useOwnClip)
        {
        return ClipConvexPolygon2DCustom(poly, polySize);
        }
    else if (s_useTMClips)
        {
        return ClipPolygon2DDTM(poly, polySize);
        }
    else
        {
      /*  auto polyface = new PolyfaceQueryCarrier(3, false, m_nIndices, m_nVertices, m_vertexBuffer, m_indexBuffer, 0, 0, 0);
        bvector < DPoint2d> poly2d;
        for (size_t i = 0; i < polySize; ++i)
            {
            poly2d.push_back(DPoint2d::From(poly[i]));
            }
        DRange3d polyExt = DRange3d::From(poly, (int)polySize);
        auto clipVec = ClipVector::CreateFromPrimitive(ClipPrimitive::CreateFromShape(&poly2d[0], polySize, true,NULL,NULL,NULL));
        DifferenceSetFromClip clipOutput(m_nVertices, m_vertexBuffer, &polyExt);
        bvector<bool> removed(m_nVertices, false);
        ClipVertices(clipOutput.GetDifferences(), removed, poly, polySize);
        if (clipOutput.GetDifferences().removedVertices.size() >= m_nVertices) 
            return clipOutput.GetDifferences();
        clock_t timeBegin = clock();
        clipVec->ClipPolyface(*polyface, clipOutput, true);
        double totalTime = (double)(clock() - timeBegin) / CLOCKS_PER_SEC;
        s_timeSpentInClipPolyFace += totalTime;
        delete polyface;
        DifferenceSetWithTracking d = clipOutput.GetDifferences();
        vector<bool> wasFaceRemoved(d.removedFaces.size() / 3, false);
        vector<int32_t> newId(d.removedFaces.size() / 3);
        for (auto splitVal : d.splitFaces) wasFaceRemoved[splitVal] = true;
        bvector<int32_t> newRemovedFaces;
        for (size_t j = 0; j < d.removedFaces.size(); j+= 3)
            {
            if (!wasFaceRemoved[j / 3]) continue;
            newId[j / 3] = (int)newRemovedFaces.size() /3;
            newRemovedFaces.insert(newRemovedFaces.end(), &d.removedFaces[j], &d.removedFaces[j+3]);
            }
        for (auto& val : d.splitFaces)
            {
            if (wasFaceRemoved[val]) val = newId[val];
            }
        d.removedFaces = newRemovedFaces;
        vector<DPoint3d> rangeP;
        rangeP.push_back(m_range.low);
        rangeP.push_back(m_range.high);
        std::string s;
        s += "RANGE\n";
        PrintPoints(s, rangeP);
        s += "DIFFERENCE\n";
        PrintDiff(s, d, m_vertexBuffer);*/
        return DifferenceSetWithTracking();
        }
    }

DifferenceSetWithTracking Clipper::ClipConvexPolygon2DCustom(const DPoint3d* poly, size_t polySize)
    {
    DifferenceSetWithTracking d;
    d.firstIndex = (int32_t)m_nVertices+1;
    bvector<bool> removed(m_nVertices, false);
    ClipVertices(d, removed, poly, polySize);
    if (d.removedVertices.size() == m_nVertices) return d;
    DRange3d polyExt = DRange3d::From(poly, (int)polySize);
    for (size_t i = 0; i < m_nIndices; i += 3)
        {
        DPoint3d triangle[3] = { m_vertexBuffer[m_indexBuffer[i] - 1], m_vertexBuffer[m_indexBuffer[i + 1] - 1], m_vertexBuffer[m_indexBuffer[i + 2] - 1] };
        std::string s;
        std::vector<DPoint3d> pts(3);
        memcpy(&pts[0], triangle, 3 * sizeof(DPoint3d));
        PrintPoints(s, pts);
        if (removed[m_indexBuffer[i] - 1] && removed[m_indexBuffer[i + 1] - 1] && removed[m_indexBuffer[i + 2] - 1])
            {
            continue; //triangle wholly within polygon, no points to add
            }
        bool triangleMask[3] = { removed[m_indexBuffer[i] - 1], removed[m_indexBuffer[i + 1] - 1], removed[m_indexBuffer[i + 2] - 1] };
        DifferenceSet triangleDiff = ClipTriangle(triangle, triangleMask, poly, polySize, polyExt);
        if (triangleDiff.addedFaces.size() > 0 || triangleDiff.removedFaces.size() > 0)
            {
            s += "DIFFERENCE\n";
            PrintDiff(s, triangleDiff, triangle);
            d.ApplyMapped(triangleDiff, &m_indexBuffer[i]);
            }
        }
    vector<DPoint3d> rangeP;
    rangeP.push_back(m_range.low);
    rangeP.push_back(m_range.high);
    std::string s;
    s += "RANGE\n";
    PrintPoints(s, rangeP);
    s += "DIFFERENCE\n";
    PrintDiff(s, d, m_vertexBuffer);
    //d.PrintToFile("d:\\diffset.txt");
    return d;

    }


END_BENTLEY_SCALABLEMESH_NAMESPACE