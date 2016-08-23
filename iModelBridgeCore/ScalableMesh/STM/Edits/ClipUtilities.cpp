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
#ifdef VANCOUVER_API
        bcDtmObjPtr = BcDTM::CreateFromDtmHandle(*bcDtmP);
#else
        bcDtmObjPtr = BcDTM::CreateFromDtmHandle(bcDtmP);
#endif
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

bool s_useOwnClip = false;


END_BENTLEY_SCALABLEMESH_NAMESPACE