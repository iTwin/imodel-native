#include "ClipUtilities.h"
#include <TerrainModel/TerrainModel.h>
#include <ScalableMesh/ScalableMeshUtilityFunctions.h>
#include <Vu/VuApi.h>
#include <TerrainModel/Core/DTMIterators.h>
#include "..\ScalableMeshQuery.h"

USING_NAMESPACE_BENTLEY_TERRAINMODEL
BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE
//#define DEBUG 1
//#define SM_TRACE_CLIP_MESH 1
//=======================================================================================
// @bsimethod                                                   Elenie.Godzaridis 08/15
//=======================================================================================
template<class POINT, class EXTENT> void ClipMeshToNodeRange(vector<int>& faceIndexes, vector<POINT>& nodePts, bvector<DPoint3d>& pts, EXTENT& contentExtent, DRange3d& nodeRange, ScalableMeshMesh* meshP)
    {
    if (meshP->GetNbFaceIndexes() == 0) return;
    DPoint3d origins[6];
    DVec3d normals[6];
    nodeRange.Get6Planes(origins, normals);
    DPlane3d planes[6];
    ClipPlane clipPlanes[6];

    for (size_t i = 0; i < 6; ++i)
        {
        normals[i].Negate();
        planes[i] = DPlane3d::FromOriginAndNormal(origins[i], normals[i]);
        clipPlanes[i] = ClipPlane(planes[i]);
        planes[i].normal.Negate();
        }
    ClipPlaneSet planeSet(clipPlanes, 6);
    DPoint3d firstTriangle[3];
    firstTriangle[0] = pts[meshP->GetFaceIndexes()[0] - 1];
    firstTriangle[1] = pts[meshP->GetFaceIndexes()[1] - 1];
    firstTriangle[2] = pts[meshP->GetFaceIndexes()[2] - 1];
    double area = bsiGeom_getXYPolygonArea(firstTriangle, 3);
#if DEBUG && SM_TRACE_CLIP_MESH 
    std::string s;
    s += "START CLIP\n";
#endif
    for (size_t i = 0; i < (size_t)meshP->GetNbFaceIndexes(); i += 3)
        {
        DPoint3d triangle[3] = { meshP->GetPoints()[meshP->GetFaceIndexes()[i] - 1], meshP->GetPoints()[meshP->GetFaceIndexes()[i + 1] - 1], meshP->GetPoints()[meshP->GetFaceIndexes()[i + 2] - 1] };
#if DEBUG && SM_TRACE_CLIP_MESH 
        print_polygonarray(s, "TRIANGLE", triangle, 3);
#endif
        DRange3d triRange = DRange3d::From(triangle, 3);
        triRange.high.Add(DVec3d::From(1e-8, 1e-8, 1e-8));
        triRange.low.Add(DVec3d::From(-1e-8, -1e-8, -1e-8));
        nodeRange.high.Add(DVec3d::From(1e-8, 1e-8, 1e-8));
        nodeRange.low.Add(DVec3d::From(-1e-8, -1e-8, -1e-8));
        if (!triRange.IntersectsWith(nodeRange)) continue;
        if (nodeRange.IsContained(triangle[0]) && nodeRange.IsContained(triangle[1]) && nodeRange.IsContained(triangle[2]))
            {
#if DEBUG && SM_TRACE_CLIP_MESH 
            s+="ADDED TRIANGLE\n";
#endif
            faceIndexes.push_back(meshP->GetFaceIndexes()[i]);
            faceIndexes.push_back(meshP->GetFaceIndexes()[i + 1]);
            faceIndexes.push_back(meshP->GetFaceIndexes()[i + 2]);
            continue;
            }
        DPoint3d polygonArray[10];
        DPoint3d polygonArray2[10];
        int polySize = 3;
        int nLoops;
        polygonArray[0] = triangle[0];
        polygonArray[1] = triangle[1];
        polygonArray[2] = triangle[2];
        bool wasCut = false;
        for (auto& plane : planes)
            {
            double sign = 0;
            bool planeCutsPoly = false;
            for (size_t j = 0; j < polySize && !planeCutsPoly; j++)
                {
                double sideOfPoint = plane.Evaluate(polygonArray[j]);
                if (fabs(sideOfPoint) < 1e-6) sideOfPoint = 0;
                if (sign == 0) sign = sideOfPoint;
                else if ((sign > 0 && sideOfPoint < 0) || (sign < 0 && sideOfPoint > 0))
                    planeCutsPoly = true;
                }
            if (!planeCutsPoly) continue;
#if DEBUG && SM_TRACE_CLIP_MESH 
            s += "PLANE ORIGIN (" + std::to_string(plane.origin.x) + "," + std::to_string(plane.origin.y) + "," + std::to_string(plane.origin.z) + ")\n";
            s += "PLANE NORMAL (" + std::to_string(plane.normal.x) + "," + std::to_string(plane.normal.y) + "," + std::to_string(plane.normal.z) + ")\n";
#endif                  
            wasCut = true;
            int nPlaneClipSize = polySize;
#if DEBUG && SM_TRACE_CLIP_MESH 
            print_polygonarray(s, "BEFORE", polygonArray, polySize);
#endif
            bsiPolygon_clipToPlane(polygonArray2, &nPlaneClipSize, &nLoops, 10, polygonArray, polySize, &plane);
            memcpy(polygonArray, polygonArray2, nPlaneClipSize*sizeof(DPoint3d));
            if (nPlaneClipSize > 0) polySize = nPlaneClipSize;
#if DEBUG && SM_TRACE_CLIP_MESH 
            print_polygonarray(s, "AFTER", polygonArray, polySize);
#endif
            }
        if (!wasCut)
            {
#if DEBUG && SM_TRACE_CLIP_MESH 
            s += "NO CUT FOR TRIANGLE\n";
#endif
            continue;
            }
#if DEBUG && SM_TRACE_CLIP_MESH 
        print_polygonarray(s, "TESTING ARRAY", polygonArray, polySize);
#endif
        //we test all points in the cut polygon in case there was a triangle whose box intersected the node but where the triangle itself did not intersect.
        size_t nPts = 0;
        for (nPts = 0; nPts < polySize; ++nPts)
            {
            if (polygonArray[nPts].x < DBL_MAX && !nodeRange.IsContained(polygonArray[nPts])) break;
            }
        if (nPts != polySize) continue;
#if DEBUG && SM_TRACE_CLIP_MESH 
        print_polygonarray(s, "INSERTING ARRAY", polygonArray, polySize);
#endif
        vector<int> polyIndexes(polySize);
        size_t n = 0;
        for (size_t j = 0; j < polySize; ++j)
            {
            int idx = -1;
            for (size_t k = 0; k < 3 && idx == -1; ++k)
                if (bsiDPoint3d_pointEqualTolerance(&polygonArray[j], &triangle[k], 1.0e-8))
                    idx = meshP->GetFaceIndexes()[i + k];
            if (idx == -1 && polygonArray[j].x < DBL_MAX)
                {
                nodePts.push_back(PointOp<POINT>::Create(polygonArray[j].x, polygonArray[j].y, polygonArray[j].z));
                pts.push_back(polygonArray[j]);
                idx = (int)nodePts.size();
                contentExtent = ExtentOp<EXTENT>::MergeExtents(contentExtent, SpatialOp<POINT, POINT, EXTENT>::GetExtent(nodePts[idx - 1]));
                }
            if (idx != -1 && std::find(polyIndexes.begin(), polyIndexes.end(), idx) == polyIndexes.end())
                {
                polyIndexes[n] = idx;
                ++n;
                }
            }
        polySize = (int)n;
        for (size_t j = 1; j < polySize - 1; ++j)
            {
            DPoint3d newTri[3];
            newTri[0] = pts[polyIndexes[0] - 1];
            newTri[1] = pts[polyIndexes[1] - 1];
            newTri[2] = pts[polyIndexes[2] - 1];
            if (bsiDPoint3d_pointEqualTolerance(&newTri[0], &newTri[1], 1.0e-8) || bsiDPoint3d_pointEqualTolerance(&newTri[0], &newTri[2], 1.0e-8) ||
                bsiDPoint3d_pointEqualTolerance(&newTri[2], &newTri[1], 1.0e-8)) continue;
            double signedArea = bsiGeom_getXYPolygonArea(newTri, 3); //make sure new triangles are oriented the same as the original ones
            if ((signedArea < 0 && area > 0) || (signedArea > 0 && area < 0))
                {
                faceIndexes.push_back(polyIndexes[0]);
                faceIndexes.push_back(polyIndexes[j + 1]);
                faceIndexes.push_back(polyIndexes[j]);
                }
            else
                {
                faceIndexes.push_back(polyIndexes[0]);
                faceIndexes.push_back(polyIndexes[j]);
                faceIndexes.push_back(polyIndexes[j + 1]);
                }
            }
        }
#if DEBUG && SM_TRACE_CLIP_MESH 
        s += "END CLIP\n";
        std::ofstream f;
        f.open("d:\\stitching\\meshes\\logclip_" + std::to_string(nodeRange.low.x) + "_" + std::to_string(nodeRange.low.y) + "_" + std::to_string(nodeRange.high.x) + "_" + std::to_string(nodeRange.high.y),ios_base::app);
        f << s << endl;
        f.close();
#endif     
    }
#undef DEBUG
END_BENTLEY_SCALABLEMESH_NAMESPACE