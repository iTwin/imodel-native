//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: STM/ScalableMeshMesher.hpp $
//:>    $RCSfile: ScalableMeshQuadTreeBCLIBFilters.hpp,v $
//:>   $Revision: 1.28 $
//:>       $Date: 2011/04/27 17:17:56 $
//:>     $Author: Alain.Robert $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------


#include <ImagePP/all/h/HFCException.h>
#include <ScalableMesh\ScalableMeshUtilityFunctions.h>
#include <ScalableMesh\IScalableMeshQuery.h>
#include <eigen\Eigen\Dense>
#include <PCLWrapper\IDefines.h>
#include <PCLWrapper\INormalCalculator.h>
#include "ScalableMeshQuery.h"
#include "MeshingFunctions.h"
#include <ScalableMesh\ScalableMeshUtilityFunctions.h>
#include <Mtg/MtgStructs.h>
#include <Geom/bsp/bspbound.fdf>
#include "ScalableMesh\ScalableMeshGraph.h"
#include <string>
#include <queue>
#include <ctime>
#include <fstream>
//#define SINGLE_TILE
//#define TILE_X 2138568
#define TILE_X 214155
//#define TILE_Y 220088.85
#define TILE_Y 224205
//#define TILE_X2 2138568
#define TILE_X2 214155
#define TILE_Y2 224205
//#define TILE_Y2 220088.85

void print_polygonarray(std::string& s, const char* tag, DPoint3d* polyArray, int polySize)
    {
    s += tag;
    s += " : ARRAY "+std::to_string((long long)polySize) +" POINTS \n";
    for (int i = 0; i < polySize; i++)
        {
        s += "POINT (" + std::to_string(polyArray[i].x) + "," + std::to_string(polyArray[i].y) + "," + std::to_string(polyArray[i].z) + ")\n";
        }
    }

/**----------------------------------------------------------------------------
 Initiates a filtering of the node. Ther filtering process
 will compute the sub-resolution and the view oriented parameters.
 -----------------------------------------------------------------------------*/
bool firstTile = false;
template<class POINT, class EXTENT> bool ScalableMesh2DDelaunayMesher<POINT, EXTENT>::Mesh(HFCPtr<SMMeshIndexNode<POINT, EXTENT> > node) const
    {
    bool isMeshingDone = false;

    //NEEDS_WORK_SM
    if (node->size() > 4)
        {
#ifdef SINGLE_TILE
        EXTENT ext = node->GetContentExtent();
        if ((ExtentOp<EXTENT>::GetXMin(ext) > TILE_X || ExtentOp<EXTENT>::GetXMax(ext) < TILE_X || ExtentOp<EXTENT>::GetYMin(ext) > TILE_Y || ExtentOp<EXTENT>::GetYMax(ext) < TILE_Y)
            && (ExtentOp<EXTENT>::GetXMin(ext) > TILE_X2 || ExtentOp<EXTENT>::GetXMax(ext)< TILE_X2 || ExtentOp<EXTENT>::GetYMin(ext) > TILE_Y2 || ExtentOp<EXTENT>::GetYMax(ext) < TILE_Y2))
            {
            return true;
            }
        if (firstTile)
            {
            //firstTile = false;
            return true;
            }
#endif
        Bentley::TerrainModel::DTMPtr dtmPtr;

        int status = CreateBcDTM(dtmPtr);

        assert(status == SUCCESS);

        vector<DPoint3d> points(node->size());

        std::transform(node->begin(), node->end(), &points[0], ToBcPtConverter());

        BC_DTM_OBJ* dtmObjP(dtmPtr->GetBcDTM()->GetTinHandle());

        status = bcdtmObject_storeDtmFeatureInDtmObject(dtmObjP, DTMFeatureType::RandomSpots, dtmObjP->nullUserTag, 1, &dtmObjP->nullFeatureId, &points[0], (long)node->size());

        assert(status == SUCCESS);

        for (size_t i = 0; i < node->m_featureDefinitions.size(); ++i)
            {
            vector<DPoint3d> feature;
            for (size_t j = 1; j < node->m_featureDefinitions[i].size(); ++j)
                {
                if (node->m_featureDefinitions[i][j] <= points.size()) feature.push_back(points[node->m_featureDefinitions[i][j]]);
                }
            status = bcdtmObject_storeDtmFeatureInDtmObject(dtmObjP, (DTMFeatureType)node->m_featureDefinitions[i][0], dtmObjP->nullUserTag, 1, &dtmObjP->nullFeatureId, &feature[0], (long)feature.size());
            }

        status = bcdtmObject_triangulateDtmObject(dtmObjP);

      //  assert(status == SUCCESS || ((*dtmObjP).numPoints < 4));

        if (status == SUCCESS)
            {
            IScalableMeshMeshPtr meshPtr;

            bcdtmInterruptLoad_triangleShadeMeshFromDtmObject(dtmPtr->GetBcDTM()->GetTinHandle(), 10000000, 2, 1, &draw, false, DTMFenceType::None, DTMFenceOption::None, 0, 0, &meshPtr);

            ScalableMeshMesh* meshP((ScalableMeshMesh*)meshPtr.get());

            if (meshP == 0)
                {
                return true;
                }

            if (meshP != 0)
                {
                node->clear();
                
                vector<POINT> nodePts(meshP->GetNbPoints());
                bvector<DPoint3d> pts(meshP->GetNbPoints());
                for (size_t pointInd = 0; pointInd < meshP->GetNbPoints(); pointInd++)
                    {
                    nodePts[pointInd].x = meshP->GetPoints()[pointInd].x;
                    nodePts[pointInd].y = meshP->GetPoints()[pointInd].y;
                    nodePts[pointInd].z = meshP->GetPoints()[pointInd].z;
                    pts[pointInd] = meshP->GetPoints()[pointInd];
                    }
                std::vector<int> faceIndexes;
                DRange3d nodeRange = DRange3d::From(ExtentOp<EXTENT>::GetXMin(node->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetYMin(node->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetZMin(node->m_nodeHeader.m_nodeExtent),
                                                    ExtentOp<EXTENT>::GetXMax(node->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetYMax(node->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetZMax(node->m_nodeHeader.m_nodeExtent));
                DPoint3d origins[6];
                DVec3d normals[6];
                nodeRange.Get6Planes(origins, normals);
                    DPlane3d planes[6];
                    ClipPlane clipPlanes[6];
                    for (size_t i = 0; i < 6; ++i)
                        {
                        planes[i] = DPlane3d::FromOriginAndNormal(origins[i], normals[i]);
                        clipPlanes[i] = ClipPlane(planes[i]);
                        }
                    ClipPlaneSet planeSet(clipPlanes, 6);
                std::string s;
                node->m_nodeHeader.m_nbFaceIndexes = meshP->GetNbFaceIndexes();
                for (size_t i = 0; i < (size_t)meshP->GetNbFaceIndexes(); i += 3)
                    {
                    DPoint3d triangle[3] = { meshP->GetPoints()[meshP->GetFaceIndexes()[i]-1], meshP->GetPoints()[meshP->GetFaceIndexes()[i + 1]-1], meshP->GetPoints()[meshP->GetFaceIndexes()[i + 2]-1] };
                    if (!nodeRange.IsContained(triangle[0]) && !nodeRange.IsContained(triangle[1]) && !nodeRange.IsContained(triangle[2])) continue;
                    if (nodeRange.IsContained(triangle[0]) && nodeRange.IsContained(triangle[1]) && nodeRange.IsContained(triangle[2]))
                        {
                        faceIndexes.push_back(meshP->GetFaceIndexes()[i]);
                        faceIndexes.push_back(meshP->GetFaceIndexes()[i+1]);
                        faceIndexes.push_back(meshP->GetFaceIndexes()[i+2]);
                        continue;
                        }
                    DPoint3d polygonArray[10];
                    DPoint3d polygonArray2[10];
                    int polySize = 3;
                    int nLoops;
                    polygonArray[0] = triangle[0];
                    polygonArray[1] = triangle[1];
                    polygonArray[2] = triangle[2];
                    for (auto& plane : planes)
                        {
                        double sign = 0;
                        bool planeCutsPoly = false;
                        for (size_t j = 0; j < polySize && !planeCutsPoly; j++)
                            {
                            double sideOfPoint = plane.Evaluate(polygonArray[j]);
                            if (sign == 0) sign = sideOfPoint;
                            else if ((sign > 0 && sideOfPoint < 0) || (sign < 0 && sideOfPoint > 0))
                                planeCutsPoly = true;
                            }
                        if (!planeCutsPoly) continue;
                        s += "PLANE ORIGIN (" + std::to_string(plane.origin.x) + "," + std::to_string(plane.origin.y) + "," + std::to_string(plane.origin.z) + ")\n";
                        s += "PLANE NORMAL (" + std::to_string(plane.normal.x) + "," + std::to_string(plane.normal.y) + "," + std::to_string(plane.normal.z) + ")\n";
                        int nPlaneClipSize = polySize;
                        print_polygonarray(s, "BEFORE", polygonArray, polySize);
                        bsiPolygon_clipToPlane(polygonArray2, &nPlaneClipSize, &nLoops, 10, polygonArray, polySize, &plane);
                        memcpy(polygonArray, polygonArray2, nPlaneClipSize*sizeof(DPoint3d));
                        if (nPlaneClipSize > 0) polySize = nPlaneClipSize;
                        print_polygonarray(s, "AFTER", polygonArray, polySize);
                        }
                    vector<int> polyIndexes(polySize);
                    size_t n = 0;
                    for (size_t j = 0; j < polySize; ++j)
                        {
                        int idx = -1;
                        for (size_t k = 0; k < 3&& idx == -1; ++k)
                            if (bsiDPoint3d_pointEqualTolerance(&polygonArray[j], &triangle[k], 1.0e-8))
                                idx = meshP->GetFaceIndexes()[i + k];
                        if (idx == -1 && polygonArray[j].x < DBL_MAX)
                            {
                            nodePts.push_back(PointOp<POINT>::Create(polygonArray[j].x, polygonArray[j].y, polygonArray[j].z));
                            pts.push_back(polygonArray[j]);
                            idx = (int)nodePts.size();
                            }
                        if (idx != -1 && std::find(polyIndexes.begin(), polyIndexes.end(), idx) == polyIndexes.end())
                            {
                            polyIndexes[n] = idx;
                            ++n;
                            }
                        }
                    polySize = (int) n;
                    for (size_t j = 1; j < polySize - 1; ++j)
                        {
                        faceIndexes.push_back(polyIndexes[0]);
                        faceIndexes.push_back(polyIndexes[j]);
                        faceIndexes.push_back(polyIndexes[j+1]);
                        }
                    }
                /*PolyfaceHeaderPtr clippedMesh;
                PolyfaceCoordinateMapPtr insidePts;

                PolyfaceQueryP polyface = const_cast<PolyfaceQueryP>(meshP->GetPolyfaceQuery());
                clippedMesh = PolyfaceHeader::CreateVariableSizeIndexed();
                clippedMesh->ActivateVectorsForIndexing(*polyface);
                insidePts = PolyfaceCoordinateMap::New(*clippedMesh);
                bool incomplete;
                PolyfaceCoordinateMap::AddClippedPolyface(*polyface, insidePts.get(), NULL, incomplete, &planeSet, true);

                vector<POINT> nodePts(clippedMesh->Point().size());

                for (size_t pointInd = 0; pointInd < clippedMesh->GetPointCount(); pointInd++)
                    {
                    nodePts[pointInd].x = clippedMesh->GetPointCP()[pointInd].x;
                    nodePts[pointInd].y = clippedMesh->GetPointCP()[pointInd].y;
                    nodePts[pointInd].z = clippedMesh->GetPointCP()[pointInd].z;
                    }
                std::vector<int> faceIndexes;
                faceIndexes.insert(faceIndexes.end(), clippedMesh->GetPointIndexCP(), clippedMesh->GetPointIndexCP()+clippedMesh->GetPointIndexCount());*/
                node->m_nodeHeader.m_nbFaceIndexes = faceIndexes.size();
                node->push_back(&nodePts[0], nodePts.size());
                bvector<int> componentPointsId;
                if (NULL == node->GetGraphPtr()) node->CreateGraph();
                CreateGraphFromIndexBuffer(node->GetGraphPtr(), (const long*)&faceIndexes[0], (int)faceIndexes.size(), (int)nodePts.size(), componentPointsId,&pts[0]);
                node->SetGraphDirty();
                node->StoreGraph();
              /*  MTGGraph* meshGraph = node->GetGraphPtr();
                 MTGMask visitedMask = meshGraph->GrabMask();
                MTGARRAY_SET_LOOP(edgeID, meshGraph)
                    {
                    bool isComponent = true;
                    int vtx = -1;
                    meshGraph->TryGetLabel(edgeID, 1, vtx);
                    //if (vtx > 0) for (int j = 0; j < componentPointsId.size() && !isComponent; j++) if (componentPointsId[j] == vtx - 1) isComponent = true;
                    if (isComponent && meshGraph->GetMaskAt(edgeID, MTG_EXTERIOR_MASK))
                        {
                        MTGARRAY_FACE_LOOP(extID, meshGraph, edgeID)
                            {
                            if (meshGraph->GetMaskAt(extID, visitedMask) || !meshGraph->GetMaskAt(extID, MTG_EXTERIOR_MASK)) break;
                            MTGARRAY_FACE_LOOP(faceID, meshGraph, meshGraph->EdgeMate(extID))
                                {
                                int vtx2 = -1;
                                meshGraph->TryGetLabel(faceID, 0, vtx2);
                                faceIndexes.push_back(vtx2);
                                }
                            MTGARRAY_END_FACE_LOOP(faceID, meshGraph, meshGraph->EdgeMate(extID))
                                meshGraph->SetMaskAt(extID, visitedMask);
                            }
                        MTGARRAY_END_FACE_LOOP(extID, meshGraph, edgeID)
                        }
                    }
                MTGARRAY_END_SET_LOOP(edgeID, meshGraph)
                    meshGraph->ClearMask(visitedMask);
                meshGraph->DropMask(visitedMask);*/
                if (componentPointsId.size() > 0)
                    {
                    if (node->m_nodeHeader.m_meshComponents == nullptr) node->m_nodeHeader.m_meshComponents = new int[componentPointsId.size()];
                    else if (node->m_nodeHeader.m_numberOfMeshComponents != componentPointsId.size())
                        {
                        delete[] node->m_nodeHeader.m_meshComponents;
                        node->m_nodeHeader.m_meshComponents = new int[componentPointsId.size()];
                        }
                    node->m_nodeHeader.m_numberOfMeshComponents = componentPointsId.size();
                    memcpy(node->m_nodeHeader.m_meshComponents, componentPointsId.data(), componentPointsId.size()*sizeof(int));
                    }

                //NEEDS_WORK_SM - Ugly hack, piggyback the face indexes at the end of the points
                size_t nbPointsForFaceInd = (size_t)ceil((node->m_nodeHeader.m_nbFaceIndexes * (double)sizeof(int32_t)) / (double)sizeof(POINT));

                POINT* pPiggyBackMeshIndexes = new POINT[nbPointsForFaceInd];

                memcpy(pPiggyBackMeshIndexes, &faceIndexes[0], node->m_nodeHeader.m_nbFaceIndexes * sizeof(int32_t));

                node->push_back(pPiggyBackMeshIndexes, nbPointsForFaceInd);
                    
                node->setNbPointsUsedForMeshIndex(nbPointsForFaceInd);   

                //NEEDS_WORK_SM Avoid some assert                            
                delete [] pPiggyBackMeshIndexes; 
                if (node->IsLeaf() && node->size() != node->m_nodeHeader.m_totalCount)
                    {
                    node->m_nodeHeader.m_totalCount = node->size();
                    }
               // firstTile = true;
                isMeshingDone = true;

                node->SetDirty(true);
                }
            }
        }

    return isMeshingDone;
    }


/**----------------------------------------------------------------------------
Merry 2013 / Guennebaud 2007 MLS mesher.
See following papers for reference:
Gael Guennebaud & Markus Gross. Algebraic Point Set Surfaces. ACM Transactions on Graphics (TOG) - Proceedings of ACM SIGGRAPH 2007 26(3) (2007)
http://dx.doi.org/10.1145/1275808.1276406
Bruce Merry, James Gain and Patrick Marais. Moving Least Squares reconstruction of large models with GPUs. TRANSACTIONS ON VISUALIZATION AND COMPUTER GRAPHICS 20(2) (2014)
http://dx.doi.org/10.1109/TVCG.2013.118

For this algorithm nodes provided in input should have duplicated points (include all neighbor points that may influence this node's surface).
-----------------------------------------------------------------------------*/
    template<class POINT, class EXTENT> bool ScalableMeshAPSSOutOfCoreMesher<POINT, EXTENT>::Init(const SMMeshIndex<POINT, EXTENT>& pointIndex)
    {
    m_totalPointCount = pointIndex.GetCount();
    m_numberOfLevels = pointIndex.GetDepth() + 1;

    return true;
    } 

//#pragma optimize("", off)
    template<class POINT, class EXTENT> bool ScalableMeshAPSSOutOfCoreMesher<POINT, EXTENT>::Mesh(HFCPtr<SMMeshIndexNode<POINT, EXTENT> > node) const
    {
    vector<DPoint3d> points(node->size());
    vector<DVec3d> normals(node->size());
    vector<float> * distances = new std::vector<float>();
    std::vector<float> * vertex = new std::vector<float>();
    EXTENT extent = node->GetContentExtent();
    double x = ExtentOp<EXTENT>::GetXMin(extent) + (ExtentOp<EXTENT>::GetXMax(extent) - ExtentOp<EXTENT>::GetXMin(extent)) / 2;
    double y = ExtentOp<EXTENT>::GetYMin(extent) + (ExtentOp<EXTENT>::GetYMax(extent) - ExtentOp<EXTENT>::GetYMin(extent)) / 2;
    double z = ExtentOp<EXTENT>::GetZMin(extent) + (ExtentOp<EXTENT>::GetZMax(extent) - ExtentOp<EXTENT>::GetZMin(extent)) / 2;
    std::transform(node->begin(), node->end(), &points[0], ToBcPtConverter());
    if (0 == points.size()) 
        return false;
    //normalize point coordinates so that origin is closer to 0 (large values for coords can cause PCL trouble)
    std::for_each(points.begin(), points.end(), [x, y, z] (DPoint3d& p)
        {
        p.x -= x;
        p.y -= y;
        p.z -= z;
        });
    //compute normals for point set
    Bentley::PCLUtility::INormalCalculator::ComputeNormals(&normals[0], &points[0], node->size());
    //create distances for cell grid
    Eigen::Vector3f boxmin(ExtentOp<EXTENT>::GetXMin(extent)-x, ExtentOp<EXTENT>::GetYMin(extent)-y, ExtentOp<EXTENT>::GetZMin(extent)-z);
    Eigen::Vector3f boxmax(ExtentOp<EXTENT>::GetXMax(extent)-x, ExtentOp<EXTENT>::GetYMax(extent)-y, ExtentOp<EXTENT>::GetZMax(extent)-z);
    size_t level = node->GetLevel();
    size_t depth = node->GetDepth();
    computeGridDistancesAPSS(distances, &points, &normals, boxmin, boxmax,level,depth);
    //marching tetrahedra to compute mesh
    marchingTetrahedra(vertex, distances, boxmin, boxmax);
    delete distances;
    if (0 == vertex->size()) 
        return false;
    //extract indices and coordinate arrays from MT result
    std::unordered_map<std::string, int> coordMap;
    std::vector<POINT> pts;
    std::vector<int> indices;
    POINT pt;
    for (size_t i = 0; i < vertex->size(); i += 9)
        {
        for (size_t j = 0; j < 3; j++)
            {
            pt.x = (*vertex)[i + (j * 3) + 0];
            pt.y = (*vertex)[i + (j * 3) + 1];
            pt.z = (*vertex)[i + (j * 3) + 2];
            int x, y, z; //to_string only formats 6 decimals...can't believe I wrote this
            memcpy(&x, &((*vertex)[i + (j * 3) + 0]), sizeof(int));
            memcpy(&y, &((*vertex)[i + (j * 3) + 1]), sizeof(int));
            memcpy(&z, &((*vertex)[i + (j * 3) + 2]), sizeof(int));
            std::string ptKey(std::to_string(x) + "@" + std::to_string(y) + "@"+std::to_string(z));
            if (coordMap.count(ptKey) == 0)
                {
                pts.push_back(pt);
                coordMap[ptKey] = (int)pts.size();
                indices.push_back(coordMap[ptKey]);
                }
            else
                {
                indices.push_back(coordMap[ptKey]);
                }
            }
        }
    delete vertex;
    //cleaning
    //write triangles to node
    std::for_each(pts.begin(), pts.end(), [x, y, z] (POINT& p)
        {
        p.x += x;
        p.y += y;
        p.z += z;
        });
    node->clear();
    node->m_nodeHeader.m_totalCountDefined = false;
    if (node->IsLeaf())
        {
        node->m_nodeHeader.m_totalCount = pts.size();
        }
    node->push_back(&pts[0], pts.size());
    node->m_nodeHeader.m_nbFaceIndexes = indices.size();
    size_t nbPointsForFaceInd = (size_t)ceil((node->m_nodeHeader.m_nbFaceIndexes * (double)sizeof(int32_t)) / (double)sizeof(POINT));
    //this should probably be replaced by something like node->setMeshIndexes() in the future to prevent code duplication
    POINT* pPiggyBackMeshIndexes = new POINT[nbPointsForFaceInd];
    memcpy(pPiggyBackMeshIndexes, &indices[0], node->m_nodeHeader.m_nbFaceIndexes * sizeof(int32_t));
    node->push_back(pPiggyBackMeshIndexes, nbPointsForFaceInd);
    node->setNbPointsUsedForMeshIndex(nbPointsForFaceInd);
    
    delete[] pPiggyBackMeshIndexes;
    return true;
    }
//#pragma optimize("", on)

/**----------------------------------------------------------------------------
Initiates the stitching of the mesh present in neighbor nodes.
-----------------------------------------------------------------------------*/
template<class POINT, class EXTENT> bool ScalableMeshAPSSOutOfCoreMesher<POINT, EXTENT>::Stitch(HFCPtr<SMMeshIndexNode<POINT, EXTENT> > node) const
    {
    //assert(!"Not yet implemented yet");
    return true;
    }

/**----------------------------------------------------------------------------
 Initiates a filtering of the node. Ther filtering process
 will compute the sub-resolution and the view oriented parameters.
-----------------------------------------------------------------------------*/
inline void AddTriangleToDTM(BC_DTM_OBJ* dtmObjP, vector<DPoint3d>& points, int32_t* faceIndexes, size_t nbFaces)
    {
    vector<DPoint3d> edgePoints(2);

    for (size_t faceInd = 0; faceInd < nbFaces; faceInd++)
        {                                                
        edgePoints[0] = points[faceIndexes[faceInd * 3]];
        edgePoints[1] = points[faceIndexes[faceInd * 3 + 1]];

        int status = bcdtmObject_storeDtmFeatureInDtmObject(dtmObjP, DTMFeatureType::Breakline, dtmObjP->nullUserTag, 1, &dtmObjP->nullFeatureId, &edgePoints[0], 2);
        assert(status == SUCCESS);

        edgePoints[0] = points[faceIndexes[faceInd * 3 + 1]];
        edgePoints[1] = points[faceIndexes[faceInd * 3 + 2]];

        status = bcdtmObject_storeDtmFeatureInDtmObject(dtmObjP, DTMFeatureType::Breakline, dtmObjP->nullUserTag, 1, &dtmObjP->nullFeatureId, &edgePoints[0], 2);
        assert(status == SUCCESS);

        edgePoints[0] = points[faceIndexes[faceInd * 3 + 2]];
        edgePoints[1] = points[faceIndexes[faceInd * 3]];

        status = bcdtmObject_storeDtmFeatureInDtmObject(dtmObjP, DTMFeatureType::Breakline, dtmObjP->nullUserTag, 1, &dtmObjP->nullFeatureId, &edgePoints[0], 2);
        assert(status == SUCCESS);
        }    
    }

template<class POINT, class EXTENT> void ScalableMesh2DDelaunayMesher<POINT, EXTENT>::LoadAdjacencyData(MTGGraph* graph, POINT* pPoints, size_t size) const
    {
    size_t ct = size* sizeof(POINT);
    graph->LoadFromBinaryStream(pPoints, ct);
    }

inline void ValidateTriangleMesh(vector<int> faces, vector<DPoint3d> stitchedPoints)
    {
    std::map<long long, int> edges;
    std::vector<DSegment3d> lines;
    for (size_t i = 0; i < faces.size(); i += 3)
        {
        assert(faces[i] <= (int)stitchedPoints.size() && faces[i] > 0);
        assert(faces[i+1] <= (int)stitchedPoints.size() && faces[i+1] > 0);
        assert(faces[i + 2] <= (int)stitchedPoints.size() && faces[i + 2] > 0);
        long long edge1 = (((long long)faces[i] << 32) | faces[i + 1]);
        long long edge2 = (((long long)faces[i + 1] << 32) | faces[i + 2]);
        long long edge3 = (((long long)faces[i + 2] << 32) | faces[i]);
        long long edge4 = (((long long)faces[i + 1] << 32) | faces[i]);
        long long edge5 = (((long long)faces[i + 2] << 32) | faces[i + 1]);
        long long edge6 = (((long long)faces[i] << 32) | faces[i + 2]);
        if (edges.count(edge1) == 0)
            {
            edges[edge1] = edges[edge4] = 1;
            DSegment3d edgeSeg = DSegment3d::From(stitchedPoints[faces[i]-1], stitchedPoints[faces[i + 1]-1]);
            double param1, param2;
            DPoint3d pt1, pt2;
            for (size_t j = 0; j < lines.size(); j++)
                {
                DSegment3d::ClosestApproachBounded(param1, param2, pt1, pt2, edgeSeg, lines[j]);
                assert(param1 <= 0.00001 || param1 >= 1 || param2 <= 0.00001 || param2 >= 1);
                }
            lines.push_back(edgeSeg);
            }
        else{
            assert(edges[edge4] == edges[edge1]);
            edges[edge4] = ++edges[edge1];
            assert(edges[edge1] <= 2);
            }
        if (edges.count(edge2) == 0)
            {
            edges[edge2] = edges[edge5] = 1;
            DSegment3d edgeSeg = DSegment3d::From(stitchedPoints[faces[i + 1]-1], stitchedPoints[faces[i + 2]-1]);
            double param1, param2;
            DPoint3d pt1, pt2;
            for (size_t j = 0; j < lines.size(); j++)
                {
                DSegment3d::ClosestApproachBounded(param1, param2, pt1, pt2, edgeSeg, lines[j]);
                assert(param1 <= 0.00001 || param1 >= 1 || param2 <= 0.00001 || param2 >= 1);
                }
            lines.push_back(edgeSeg);
            }
        else
            {
            assert(edges[edge5] == edges[edge2]);
            edges[edge5] = ++edges[edge2];
            assert(edges[edge2] <= 2);
            }
        if (edges.count(edge3) == 0)
            {
            edges[edge3] = edges[edge6] = 1;
            DSegment3d edgeSeg = DSegment3d::From(stitchedPoints[faces[i + 2]-1], stitchedPoints[faces[i]-1]);
            double param1, param2;
            DPoint3d pt1, pt2;
            for (size_t j = 0; j < lines.size(); j++)
                {
                DSegment3d::ClosestApproachBounded(param1, param2, pt1, pt2, edgeSeg, lines[j]);
                assert(param1 <= 0.00001 || param1 >= 1 || param2 <= 0.00001 || param2 >= 1);
                }
            lines.push_back(edgeSeg);
            }
        else
            {
            assert(edges[edge6] == edges[edge3]);
            edges[edge6] = ++edges[edge3];
            assert(edges[edge3] <= 2);
            }
        }
    }

template<class POINT, class EXTENT> size_t ScalableMesh2DDelaunayMesher<POINT, EXTENT>::UpdateMeshNodeFromGraph(HFCPtr<SMMeshIndexNode<POINT, EXTENT> > node, POINT** newMesh, MTGGraph * meshGraphStitched, std::vector<DPoint3d>& stitchedPoints, int& nFaces, DPoint3d& minPt, DPoint3d& maxPt) const
    {

    //POINT* geometryData = new POINT[stitchedPoints.size()];
    vector<int> faceIndices;
    /*for (size_t i = 0; i < stitchedPoints.size(); i++)
    {
    geometryData[i].x = stitchedPoints[i].x;
    geometryData[i].y = stitchedPoints[i].y;
    geometryData[i].z = stitchedPoints[i].z;
    }*/
    int* pointsInTile = new int[stitchedPoints.size()];
    for (size_t i = 0; i < stitchedPoints.size(); i++) pointsInTile[i] = -1;
    MTGMask visitedMask = meshGraphStitched->GrabMask();
    std::vector<int> indicesForFace;
    int newPtsIndex = 0;
    MTGARRAY_SET_LOOP(edgeID, meshGraphStitched)
        {
        if (!meshGraphStitched->GetMaskAt(edgeID, MTG_EXTERIOR_MASK) && !meshGraphStitched->GetMaskAt(edgeID, visitedMask))
            {
            //assert(meshGraphStitched->CountNodesAroundFace(edgeID) == 3);
            int n = 0;
            int nPointsOutsideTile = 0;
            int nPointsInsideTile = 0;
            indicesForFace.clear();
            std::vector<MTGNodeId> faceNodes;
            if (meshGraphStitched->CountNodesAroundFace(edgeID) != 3) continue;
            MTGARRAY_FACE_LOOP(faceID, meshGraphStitched, edgeID)
                {
                int vIndex = -1;
                meshGraphStitched->TryGetLabel(faceID, 0, vIndex);
                assert(vIndex > 0);
                assert(vIndex <= (int)stitchedPoints.size());
                DPoint3d pt = stitchedPoints[vIndex - 1];
                if (pt.x < minPt.x || pt.y < minPt.y || pt.z < minPt.z || pt.x > maxPt.x || pt.y > maxPt.y || pt.z > maxPt.z) nPointsOutsideTile++;
               // if (pt.x > minPt.x && pt.y > minPt.y && pt.z > minPt.z && pt.x < maxPt.x && pt.y < maxPt.y && pt.z < maxPt.z) nPointsInsideTile++;
                // geometryData.push_back(PointOp<POINT>::Create(stitchedPoints[vIndex - 1].x, stitchedPoints[vIndex - 1].y, stitchedPoints[vIndex - 1].z));
                if (nPointsOutsideTile < 3 && nPointsInsideTile < 3)
                    {
                    if (pointsInTile[vIndex - 1] == -1)
                        {
                        pointsInTile[vIndex - 1] = newPtsIndex;
                        newPtsIndex++;
                        assert(pointsInTile[vIndex - 1] < newPtsIndex);
                        indicesForFace.push_back(vIndex - 1);
                        }
                    faceIndices.push_back(pointsInTile[vIndex - 1] + 1);
                    }
                else
                    {
                    faceIndices.resize(faceIndices.size() - n); //erase last face
                    for (int index : indicesForFace)
                        {
                        pointsInTile[index] = -1;
                        }
                    newPtsIndex -= (int)indicesForFace.size();
                    indicesForFace.clear();
                    }
                if (n == 2 && nPointsOutsideTile < 3 && nPointsInsideTile < 3)
                    {
                    meshGraphStitched->TrySetLabel(faceID, 0, pointsInTile[vIndex - 1] + 1);
                    int v1 = -1;
                    meshGraphStitched->TryGetLabel(meshGraphStitched->FSucc(faceID), 0, v1);
                    meshGraphStitched->TrySetLabel(meshGraphStitched->FSucc(faceID), 0, pointsInTile[v1 - 1] + 1);
                    meshGraphStitched->TryGetLabel(meshGraphStitched->FPred(faceID), 0, v1);
                    meshGraphStitched->TrySetLabel(meshGraphStitched->FPred(faceID), 0, pointsInTile[v1 - 1] + 1);
                    }
                //meshGraphStitched->TrySetLabel(faceID, 0, (int)geometryData.size());
                meshGraphStitched->SetMaskAt(faceID, visitedMask);
                ++n;
                }
            MTGARRAY_END_FACE_LOOP(faceID, meshGraphStitched, edgeID)
            }
        }
    MTGARRAY_END_SET_LOOP(edgeID, meshGraphStitched)
        meshGraphStitched->ClearMask(visitedMask);
    meshGraphStitched->DropMask(visitedMask);
    MTGARRAY_SET_LOOP(toDeleteID, meshGraphStitched)
        {
        int index = -1;
        meshGraphStitched->TryGetLabel(toDeleteID, 0, index);
        if (index == -1 || index > newPtsIndex)  meshGraphStitched->DropEdge(toDeleteID);
        }
    MTGARRAY_END_SET_LOOP(toDeleteID, meshGraphStitched)
        POINT* geometryData = new POINT[newPtsIndex];
    for (size_t i = 0; i < stitchedPoints.size(); i++)
        {
        if (pointsInTile[i] != -1)
            {
            assert(pointsInTile[i] < newPtsIndex);
            geometryData[pointsInTile[i]].x = stitchedPoints[i].x;
            geometryData[pointsInTile[i]].y = stitchedPoints[i].y;
            geometryData[pointsInTile[i]].z = stitchedPoints[i].z;
            }
        }
    stitchedPoints.resize(newPtsIndex);
    delete[] pointsInTile;
    //ValidateTriangleMesh(faceIndices, stitchedPoints);
#ifdef SINGLE_TILE
    if ((minPt.x > TILE_X || maxPt.x < TILE_X || minPt.y > TILE_Y || maxPt.y < TILE_Y)
        && (minPt.x > TILE_X2 || maxPt.x < TILE_X2 || minPt.y > TILE_Y2 || maxPt.y < TILE_Y2))
        {
        faceIndices.clear();
        }
#endif
    if (NULL == node->GetGraphPtr()) node->LoadGraph();
    *node->GetGraphPtr() = *meshGraphStitched;
    node->SetGraphDirty();
    node->m_nodeHeader.m_nbFaceIndexes = faceIndices.size();
    assert(faceIndices.size() % 3 == 0);
    nFaces = (int)faceIndices.size();
    size_t nOfPointsUsed = stitchedPoints.size() + (size_t)ceil(faceIndices.size()*(double)sizeof(int) / sizeof(POINT));// +(size_t)ceil((double)ct / sizeof(POINT));
    *newMesh = new POINT[nOfPointsUsed];
    memcpy(*newMesh, geometryData, /*stitchedPoints.size()*/newPtsIndex*sizeof(POINT));
    delete geometryData;
    memcpy(*newMesh + stitchedPoints.size(), &faceIndices[0], faceIndices.size()*sizeof(int));
    return nOfPointsUsed;
    }

/*long long ncalls = 0;
double nIts = 0;
double nPtsProducedPerCall = 0;
double nTimeTaken =0.0;*/
    MTGMask addedMask;
    template<class POINT, class EXTENT> void ScalableMesh2DDelaunayMesher<POINT, EXTENT>::SelectPointsToStitch(std::vector<DPoint3d>& stitchedPoints, HFCPtr<SMMeshIndexNode<POINT, EXTENT> > node, MTGGraph* meshGraph, EXTENT neighborExt, vector<int>* pointsToDestPointsMap) const
    {
    // ++ncalls;
    //clock_t begin = clock();
    /*  MTGNodeId boundaryEdgeID = -1;
      MTGARRAY_SET_LOOP(edgeID, meshGraph)
      {
      if (meshGraph->GetMaskAt(edgeID, MTG_BOUNDARY_MASK))
      {
      boundaryEdgeID = edgeID;
      break;
      }
      }
      MTGARRAY_END_SET_LOOP(boundaryEdgeID, meshGraph)


      assert(boundaryEdgeID != -1);
      std::queue<MTGNodeId> bounds;
      bounds.push(boundaryEdgeID);
      //get face of current edge
      MTGNodeId face[3];
      DPoint3d facePoints[3];
      MTGMask visitedMask = meshGraph->GrabMask();
      while (bounds.size() > 0)
      {
      // ++nIts;
      MTGNodeId currentID = bounds.front();
      if (!meshGraph->GetMaskAt(currentID, MTG_BOUNDARY_MASK) || meshGraph->GetMaskAt(currentID, visitedMask))
      {
      bounds.pop();
      continue;
      }

      //        MTG_Node* ptrs[3];
      int edgeAroundFace = 0, vIndex;

      meshGraph->SetMaskAt(currentID, visitedMask);
      MTGARRAY_FACE_LOOP(aroundFaceIndex, meshGraph, currentID)
      {
      if (edgeAroundFace < 3)
      {
      face[edgeAroundFace] = aroundFaceIndex;
      meshGraph->TryGetLabel(aroundFaceIndex, 0, vIndex);
      assert(vIndex > 0);
      POINT pt = (*node)[vIndex - 1];
      facePoints[edgeAroundFace] = DPoint3d::FromXYZ(pt.x, pt.y, pt.z);
      }
      edgeAroundFace++;
      if (edgeAroundFace > 3)break;
      }
      MTGARRAY_END_FACE_LOOP(aroundFaceIndex, meshGraph, currentID)
      if (edgeAroundFace>3)
      {
      if (!meshGraph->GetMaskAt(meshGraph->EdgeMate(currentID), MTG_EXTERIOR_MASK))
      {
      meshGraph->SetMaskAt(meshGraph->EdgeMate(currentID), MTG_BOUNDARY_MASK);
      meshGraph->SetMaskAt(currentID, MTG_EXTERIOR_MASK);
      meshGraph->ClearMaskAt(currentID, MTG_BOUNDARY_MASK);
      bounds.push(meshGraph->EdgeMate(currentID));
      }
      bounds.pop();
      continue;
      }
      DPoint3d sphereCenter;
      double sphereRadius;
      bound_sphereCompute(&sphereCenter, &sphereRadius, facePoints, 3);
      //EXTENT neighborExt = node->m_apNeighborNodes[neighborInd][neighborSubInd]->GetContentExtent();
      if (ExtentOp<EXTENT>::Overlap(neighborExt, ExtentOp<EXTENT>::Create(sphereCenter.x - sphereRadius, sphereCenter.y - sphereRadius,
      sphereCenter.z - sphereRadius, sphereCenter.x + sphereRadius,
      sphereCenter.y + sphereRadius, sphereCenter.z + sphereRadius)))
      {
      for (int i = 0; i < 3; i++)
      {
      meshGraph->TryGetLabel(face[i], 0, vIndex);
      assert(vIndex > 0);
      stitchedPoints.push_back(facePoints[i]);
      if (nullptr != pointsToDestPointsMap) (*pointsToDestPointsMap)[vIndex - 1] = (int)stitchedPoints.size() - 1;
      if (!meshGraph->GetMaskAt(meshGraph->EdgeMate(face[i]), MTG_EXTERIOR_MASK))
      {
      meshGraph->SetMaskAt(meshGraph->EdgeMate(face[i]), MTG_BOUNDARY_MASK);
      bounds.push(meshGraph->EdgeMate(face[i]));
      meshGraph->ClearMaskAt(face[i], MTG_BOUNDARY_MASK);
      meshGraph->SetMaskAt(face[i], MTG_EXTERIOR_MASK);
      }
      else
      {
      meshGraph->TryGetLabel(meshGraph->EdgeMate(face[i]), 0, vIndex);
      assert(vIndex > 0);
      assert((vIndex - 1) <= node->size());
      POINT pt = (*node)[vIndex - 1];
      stitchedPoints.push_back(DPoint3d::FromXYZ(pt.x, pt.y, pt.z));
      if (nullptr != pointsToDestPointsMap)  (*pointsToDestPointsMap)[vIndex - 1] = (int)stitchedPoints.size() - 1;
      // ++nPtsProducedPerCall;
      }
      }
      if (!meshGraph->GetMaskAt(meshGraph->EdgeMate(currentID), MTG_EXTERIOR_MASK))
      {
      meshGraph->ClearMaskAt(currentID, MTG_BOUNDARY_MASK);
      meshGraph->SetMaskAt(currentID, MTG_EXTERIOR_MASK);
      }
      else meshGraph->DropEdge(currentID);
      }
      else
      {
      if (bounds.size() == 1)
      {
      MTGARRAY_SET_LOOP(edgeID, meshGraph)
      {
      if (edgeID < currentID && meshGraph->GetMaskAt(edgeID, MTG_BOUNDARY_MASK) && !meshGraph->GetMaskAt(edgeID, MTG_EXTERIOR_MASK))
      {
      bounds.push(edgeID);
      }
      }
      MTGARRAY_END_SET_LOOP(boundaryEdgeID, meshGraph)
      }
      }
      bounds.pop();
      }
      meshGraph->DropMask(visitedMask);
      }*/
    //MTGNodeId boundaryEdgeID = -1;
    //int* boundaryPoints = node->m_nodeHeader.m_meshComponents;
   // std::set<int> pts;
    std::queue<MTGNodeId> bounds;
   /* if (boundaryPoints != nullptr)
        {
        for (int i = 0; i < node->m_nodeHeader.m_numberOfMeshComponents; i++) pts.insert(boundaryPoints[i]);
        }*/
    MTGARRAY_SET_LOOP(edgeID, meshGraph)
        {
        int vtx = -1;
        meshGraph->TryGetLabel(edgeID, 0, vtx);
      //  if (pts.count(vtx - 1) != 0) bounds.push(edgeID);
        if (meshGraph->GetMaskAt(edgeID, MTG_BOUNDARY_MASK))
            {
            bounds.push(edgeID);
            }
        }
    MTGARRAY_END_SET_LOOP(edgeID, meshGraph)
        //get face of current edge
    volatile int nDropped = 0;
    MTGNodeId face[3];
    MTGNodeId mate[3];
    DPoint3d facePoints[3];
    MTGMask visitedMask = meshGraph->GrabMask();
    while (bounds.size() > 0)
        {
        MTGNodeId currentID = bounds.front();
        bounds.pop();
        if (!meshGraph->GetMaskAt(currentID, MTG_BOUNDARY_MASK) || meshGraph->GetMaskAt(currentID, visitedMask))
            {
            if (meshGraph->GetMaskAt(currentID, MTG_EXTERIOR_MASK) && !meshGraph->GetMaskAt(currentID, visitedMask) && !meshGraph->GetMaskAt(meshGraph->EdgeMate(currentID), visitedMask)) bounds.push(meshGraph->EdgeMate(currentID));
            meshGraph->SetMaskAt(currentID, visitedMask);
            continue;
            }
        meshGraph->SetMaskAt(currentID, visitedMask);
        int edgeAroundFace = 0, vIndex;

        MTGARRAY_FACE_LOOP(aroundFaceIndex, meshGraph, currentID)
            {
            if (edgeAroundFace < 3)
                {
                face[edgeAroundFace] = aroundFaceIndex;
                mate[edgeAroundFace] = meshGraph->EdgeMate(aroundFaceIndex);
                meshGraph->TryGetLabel(aroundFaceIndex, 0, vIndex);
                assert(vIndex > 0);
                POINT pt = (*node)[vIndex - 1];
                facePoints[edgeAroundFace] = DPoint3d::FromXYZ(pt.x, pt.y, pt.z);
                }
            edgeAroundFace++;
            if (edgeAroundFace > 3)break;
            }
        MTGARRAY_END_FACE_LOOP(aroundFaceIndex, meshGraph, currentID)
            if (edgeAroundFace > 3)
                {
               /* if (!meshGraph->GetMaskAt(meshGraph->EdgeMate(currentID), MTG_EXTERIOR_MASK))
                    {
                    meshGraph->SetMaskAt(meshGraph->EdgeMate(currentID), MTG_BOUNDARY_MASK);
                    meshGraph->SetMaskAt(currentID, MTG_EXTERIOR_MASK);
                    meshGraph->ClearMaskAt(currentID, MTG_BOUNDARY_MASK);
                    }*/
                bounds.push(meshGraph->EdgeMate(currentID));
                //bounds.pop();
                continue;
                }
        DPoint3d sphereCenter = DPoint3d::From(0,0,0);
        double sphereRadius = 0.0;
        //bound_sphereCompute(&sphereCenter, &sphereRadius, facePoints, 3);
        //EXTENT neighborExt = node->m_apNeighborNodes[neighborInd][neighborSubInd]->GetContentExtent();
        if (ExtentOp<EXTENT>::Overlap(neighborExt, ExtentOp<EXTENT>::Create(sphereCenter.x - sphereRadius, sphereCenter.y - sphereRadius,
            sphereCenter.z, sphereCenter.x + sphereRadius,
            sphereCenter.y + sphereRadius, sphereCenter.z)))
            {
            for (int i = 0; i < 3; i++)
                {
                meshGraph->TryGetLabel(face[i], 0, vIndex);
                assert(vIndex > 0);
                stitchedPoints.push_back(facePoints[i]);
                if (nullptr != pointsToDestPointsMap) (*pointsToDestPointsMap)[vIndex - 1] = (int)stitchedPoints.size() - 1;
                if (!meshGraph->GetMaskAt(mate[i], MTG_EXTERIOR_MASK))
                    {
                    meshGraph->SetMaskAt(mate[i], MTG_BOUNDARY_MASK);
                   // meshGraph->SetMaskAt(meshGraph->EdgeMate(face[i]), visitedMask);
                    bounds.push(mate[i]);
                    meshGraph->ClearMaskAt(face[i], MTG_BOUNDARY_MASK);
                    meshGraph->SetMaskAt(face[i], MTG_EXTERIOR_MASK);
                 /*   MTGARRAY_FACE_LOOP(mateFaceIdx, meshGraph, meshGraph->EdgeMate(face[i]))
                        {
                        meshGraph->TryGetLabel(mateFaceIdx, 0, vIndex);
                        assert(vIndex > 0);
                        assert((vIndex - 1) <= node->size());
                        POINT pt = (*node)[vIndex - 1];
                        stitchedPoints.push_back(DPoint3d::FromXYZ(pt.x, pt.y, pt.z));
                        if (nullptr != pointsToDestPointsMap)  (*pointsToDestPointsMap)[vIndex - 1] = (int)stitchedPoints.size() - 1;
                        }
                    MTGARRAY_END_FACE_LOOP(mateFaceIdx, meshGraph, meshGraph->EdgeMate(face[i]))*/
                    }
                else
                    {
                    /*meshGraph->TryGetLabel(meshGraph->EdgeMate(face[i]), 0, vIndex);
                    assert(vIndex > 0);
                    assert((vIndex - 1) <= node->size());
                    POINT pt = (*node)[vIndex - 1];
                    assert(!(pt.x > 2137580 && pt.x < 2137590 && pt.y >220618 && pt.y < 220628));
                    stitchedPoints.push_back(DPoint3d::FromXYZ(pt.x, pt.y, pt.z));
                    if (nullptr != pointsToDestPointsMap)  (*pointsToDestPointsMap)[vIndex - 1] = (int)stitchedPoints.size() - 1;*/
                    meshGraph->DropEdge(face[i]);
                    nDropped++;
                    }
                }
            /*if (!meshGraph->GetMaskAt(meshGraph->EdgeMate(currentID), MTG_EXTERIOR_MASK))
            {
            meshGraph->ClearMaskAt(currentID, MTG_BOUNDARY_MASK);
            meshGraph->SetMaskAt(currentID, MTG_EXTERIOR_MASK);
            }
            else meshGraph->DropEdge(currentID);*/
            }
        else
            {
            for (int i = 0; i < 3; i++)
                {
                meshGraph->TryGetLabel(face[i], 0, vIndex);
                assert(vIndex > 0);
                stitchedPoints.push_back(facePoints[i]);
                if (nullptr != pointsToDestPointsMap)  (*pointsToDestPointsMap)[vIndex - 1] = (int)stitchedPoints.size() - 1;
               /* if (!meshGraph->GetMaskAt(mate[i], MTG_EXTERIOR_MASK))
                    {
                    meshGraph->SetMaskAt(mate[i], MTG_BOUNDARY_MASK);
                    // meshGraph->SetMaskAt(meshGraph->EdgeMate(face[i]), visitedMask);
                    bounds.push(mate[i]);
                    meshGraph->ClearMaskAt(face[i], MTG_BOUNDARY_MASK);
                    meshGraph->SetMaskAt(face[i], MTG_EXTERIOR_MASK);
                    }
                else
                    {
                    meshGraph->DropEdge(face[i]);
                    nDropped++;
                    }*/
                }
           // if (!meshGraph->GetMaskAt(meshGraph->FSucc(currentID), visitedMask)) bounds.push(meshGraph->FSucc(currentID));
            if (meshGraph->GetMaskAt(meshGraph->FSucc(meshGraph->EdgeMate(currentID)), MTG_EXTERIOR_MASK) && !meshGraph->GetMaskAt(meshGraph->FSucc(meshGraph->EdgeMate(currentID)), visitedMask)) bounds.push(meshGraph->FSucc(meshGraph->EdgeMate(currentID)));
            if (bounds.size() == 0)
            {
            MTGARRAY_SET_LOOP(edgeID, meshGraph)
            {
            if (!meshGraph->GetMaskAt(edgeID, visitedMask) && meshGraph->GetMaskAt(edgeID, MTG_BOUNDARY_MASK) && !meshGraph->GetMaskAt(edgeID, MTG_EXTERIOR_MASK))
            {
            bounds.push(edgeID);
            break;
            }
            }
            MTGARRAY_END_SET_LOOP(boundaryEdgeID, meshGraph)
            }
            }
        }
        meshGraph->ClearMask(visitedMask);
    meshGraph->DropMask(visitedMask);
    }
//#pragma optimize("", off)

    struct compare3D;
    template<class POINT, class EXTENT> bool ScalableMesh2DDelaunayMesher<POINT, EXTENT>::Stitch(HFCPtr<SMMeshIndexNode<POINT, EXTENT> > node) const
    {
    //NEEDS_WORK_SM : Deactivated for now.
    //return true;
    if (node->m_nodeHeader.m_nbFaceIndexes == 0) return true;
    if (NULL == node->GetGraphPtr()) node->LoadGraph();
MTGGraph* meshGraph = node->GetGraphPtr();
if (NULL == meshGraph) return true;
size_t neighborIndices[26] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25 };//{ 1, 3, 4, 6, 12, 21 }; //only neighbors that share a face at the moment
size_t nodeIndicesInNeighbor[26] = { 7, 6, 5, 4, 3, 2, 1, 0, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8 };//{ 6, 4, 3, 1, 21, 12 };
vector<DPoint3d> stitchedPoints;
vector<int> pointsToDestPointsMap(node->size());
std::fill_n(pointsToDestPointsMap.begin(), pointsToDestPointsMap.size(), -1);//
//MTGGraph* meshGraphNeighbor = new MTGGraph();
std::vector<DPoint3d> nodePoints(node->size());
std::map<DPoint3d, int, compare3D> stitchedSet;

for (size_t i = 0; i < node->size(); i++)
    {
    nodePoints[i].x = (*node)[i].x;
    nodePoints[i].y = (*node)[i].y;
    nodePoints[i].z = (*node)[i].z;
    }
bvector<DPoint3d> boundary;
EXTENT ext = node->GetContentExtent();
std::vector<bvector<DPoint3d>> stitchedNeighborsBoundary;
std::vector<EXTENT> stitchedNeighborsExtents;
addedMask = meshGraph->GrabMask();
if (!(ExtentOp<EXTENT>::GetXMin(ext) > TILE_X || ExtentOp<EXTENT>::GetXMax(ext) < TILE_X || ExtentOp<EXTENT>::GetYMin(ext) > TILE_Y || ExtentOp<EXTENT>::GetYMax(ext) < TILE_Y))
    {
   // GetGraphExteriorBoundary(meshGraph, boundary, &nodePoints[0]);
    }
for (size_t& neighborInd : neighborIndices)
    {
    size_t idx = &neighborInd - &neighborIndices[0];
    if ((node->m_apNeighborNodes[neighborInd].size() > 0))
        {
        if (!(ExtentOp<EXTENT>::GetXMin(ext) > TILE_X || ExtentOp<EXTENT>::GetXMax(ext) < TILE_X || ExtentOp<EXTENT>::GetYMin(ext) > TILE_Y || ExtentOp<EXTENT>::GetYMax(ext) < TILE_Y))
            {
            // GetGraphExteriorBoundary(meshGraph, boundary, &nodePoints[0]);
            }
        for (size_t neighborSubInd = 0; neighborSubInd < node->m_apNeighborNodes[neighborInd].size(); neighborSubInd++)
            {
            if (node->m_nodeHeader.m_apAreNeighborNodesStitched[neighborInd] == false)
                {
                HFCPtr < SMMeshIndexNode<POINT, EXTENT>> meshNode = dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(node->m_apNeighborNodes[neighborInd][neighborSubInd]);
                if (meshNode->m_nodeHeader.m_nbFaceIndexes == 0) continue;
                if (NULL == meshNode->GetGraphPtr()) meshNode->LoadGraph();
                MTGGraph* meshGraphNeighbor = meshNode->GetGraphPtr();
                if(NULL == meshGraphNeighbor) continue;
                    SelectPointsToStitch(stitchedPoints, node, meshGraph, node->m_apNeighborNodes[neighborInd][neighborSubInd]->GetContentExtent(), nullptr);
                  
#ifdef SINGLE_TILE
                   /* if (!(ExtentOp<EXTENT>::GetXMin(ext) > TILE_X || ExtentOp<EXTENT>::GetXMax(ext) < TILE_X || ExtentOp<EXTENT>::GetYMin(ext) > TILE_Y || ExtentOp<EXTENT>::GetYMax(ext) < TILE_Y))
                        {
                        MTGMask visitedMask = meshGraphNeighbor->GrabMask();
                        MTGARRAY_SET_LOOP(edgeID, meshGraphNeighbor)
                            {
                            int vtx = -1;
                            meshGraphNeighbor->TryGetLabel(edgeID, 1, vtx);
                            if (meshGraphNeighbor->GetMaskAt(edgeID, MTG_EXTERIOR_MASK))
                                {
                                MTGARRAY_FACE_LOOP(extID, meshGraphNeighbor, edgeID)
                                    {
                                    if (meshGraphNeighbor->GetMaskAt(extID, visitedMask) || !meshGraphNeighbor->GetMaskAt(extID, MTG_EXTERIOR_MASK)) break;
                                    MTGARRAY_FACE_LOOP(faceID, meshGraphNeighbor, meshGraphNeighbor->EdgeMate(extID))
                                        {
                                        int vtx2 = -1;
                                        meshGraphNeighbor->TryGetLabel(faceID, 0, vtx2);
                                        stitchedPoints.push_back(DPoint3d::From(pts[vtx2 - 1].x, pts[vtx2 - 1].y, pts[vtx2 - 1].z));
                                        }
                                    MTGARRAY_END_FACE_LOOP(faceID, meshGraphNeighbor, meshGraphNeighbor->EdgeMate(extID))
                                        meshGraphNeighbor->SetMaskAt(extID, visitedMask);
                                    }
                                MTGARRAY_END_FACE_LOOP(extID, meshGraphNeighbor, edgeID)
                                }
                            }
                        MTGARRAY_END_SET_LOOP(edgeID, meshGraphNeighbor)
                            meshGraphNeighbor->ClearMask(visitedMask);
                        meshGraphNeighbor->DropMask(visitedMask);
                        }
                    else */
#endif
                    SelectPointsToStitch(stitchedPoints, static_cast<SMMeshIndexNode<POINT, EXTENT>*>(&*node->m_apNeighborNodes[neighborInd][neighborSubInd]), meshGraphNeighbor, node->GetContentExtent(), nullptr);
                   // delete meshGraphNeighbor;
                  //  delete pts;
                    node->m_apNeighborNodes[neighborInd][neighborSubInd]->m_nodeHeader.m_apAreNeighborNodesStitched[nodeIndicesInNeighbor[idx]] = true;
                    
                }
            else
                {
               /* size_t nbPointsForFaceInd = (size_t)ceil((node->m_apNeighborNodes[neighborInd][neighborSubInd]->m_nodeHeader.m_nbFaceIndexes * (double)sizeof(Int32)) / (double)sizeof(POINT));
                size_t graphSize = (size_t)ceil(node->m_apNeighborNodes[neighborInd][neighborSubInd]->m_nodeHeader.m_sizeMeshGraph / (double)sizeof(POINT));
                pts = new POINT[node->m_apNeighborNodes[neighborInd][neighborSubInd]->sizeTotal()];
                node->m_apNeighborNodes[neighborInd][neighborSubInd]->get(pts, node->m_apNeighborNodes[neighborInd][neighborSubInd]->sizeTotal());
                LoadAdjacencyData(meshGraphNeighbor, (&pts[0]) + node->m_apNeighborNodes[neighborInd][neighborSubInd]->size() + nbPointsForFaceInd, graphSize);
                bvector<DPoint3d> b;
                vector<DPoint3d> nPts(node->m_apNeighborNodes[neighborInd][neighborSubInd]->size());
                for (int i = 0; i < node->m_apNeighborNodes[neighborInd][neighborSubInd]->size(); i++)
                    {
                    nPts[i].x = pts[i].x;
                    nPts[i].y = pts[i].y;
                    nPts[i].z = pts[i].z;
                    }
                GetGraphExteriorBoundary(meshGraphNeighbor, b, &nPts[0]);
                nPts.clear();
                stitchedNeighborsBoundary.push_back(b);
                delete meshGraphNeighbor;
                delete pts;*/
                stitchedNeighborsExtents.push_back(node->m_apNeighborNodes[neighborInd][neighborSubInd]->GetContentExtent());
                std::vector<DPoint3d> vectorPoints;
               // MTGMask visitedMask = meshGraph->GrabMask();
              //  meshGraph->ClearMask(visitedMask);
              //  meshGraph->DropMask(visitedMask);
                SelectPointsToStitch(vectorPoints, node, meshGraph, node->m_apNeighborNodes[neighborInd][neighborSubInd]->GetContentExtent(), nullptr);
                }
            node->m_nodeHeader.m_apAreNeighborNodesStitched[neighborInd] = true;
            }

        }
    }
//MTGMask visitedMask = meshGraph->GrabMask();
//meshGraph->ClearMask(visitedMask);
//meshGraph->DropMask(visitedMask);
////////GetGraphExteriorBoundary(meshGraph, boundary, &nodePoints[0]);
//mesh aggregated points
MTGGraph meshGraphStitched;
IScalableMeshMeshPtr meshPtr;
POINT* newNodePointData = nullptr;
int nfaces = 0;
size_t arraySize;
bvector<int> componentsPointsId;
DPoint3d extentMin, extentMax;
extentMin = DPoint3d::FromXYZ(ExtentOp<EXTENT>::GetXMin(ext), ExtentOp<EXTENT>::GetYMin(ext), ExtentOp<EXTENT>::GetZMin(ext));
extentMax = DPoint3d::FromXYZ(ExtentOp<EXTENT>::GetXMax(ext), ExtentOp<EXTENT>::GetYMax(ext), ExtentOp<EXTENT>::GetZMax(ext));
if (stitchedPoints.size() != 0)// return false; //nothing to stitch here
    {
    Bentley::TerrainModel::DTMPtr dtmPtr;
    int status = CreateBcDTM(dtmPtr);
    assert(status == SUCCESS);
    BC_DTM_OBJ* dtmObjP(dtmPtr->GetBcDTM()->GetTinHandle());
    status = bcdtmObject_storeDtmFeatureInDtmObject(dtmObjP, DTMFeatureType::RandomSpots, dtmObjP->nullUserTag, 1, &dtmObjP->nullFeatureId, &stitchedPoints[0], (int)stitchedPoints.size());
    assert(status == SUCCESS);

    //status = bcdtmObject_storeDtmFeatureInDtmObject(dtmObjP, DTMFeatureType::RandomSpots, dtmObjP->nullUserTag, 1, &dtmObjP->nullFeatureId, &boundary[0], (int)boundary.size());
    if (boundary.size() > 0)
        {
        //    boundary.push_back(boundary[0]);
        // status = bcdtmObject_storeDtmFeatureInDtmObject(dtmObjP, DTMFeatureType::Hull, dtmObjP->nullUserTag, 1, &dtmObjP->nullFeatureId, (DPoint3d*)&(boundary[0]), (long)boundary.size());
        status = bcdtmObject_storeDtmFeatureInDtmObject(dtmObjP, DTMFeatureType::GraphicBreak, dtmObjP->nullUserTag, 1, &dtmObjP->nullFeatureId, (DPoint3d*)&(boundary[0]), (long)boundary.size());
        assert(status == SUCCESS);
        status = bcdtmObject_storeDtmFeatureInDtmObject(dtmObjP, DTMFeatureType::Void, dtmObjP->nullUserTag, 1, &dtmObjP->nullFeatureId, (DPoint3d*)&(boundary[0]), (long)boundary.size());
        assert(status == SUCCESS);
        }
    for (size_t i = 0; i < stitchedNeighborsBoundary.size(); i++)
        {
        status = bcdtmObject_storeDtmFeatureInDtmObject(dtmObjP, DTMFeatureType::GraphicBreak, dtmObjP->nullUserTag, 1, &dtmObjP->nullFeatureId, (DPoint3d*)&(stitchedNeighborsBoundary[i][0]), (long)stitchedNeighborsBoundary[i].size());
        assert(status == SUCCESS);
        status = bcdtmObject_storeDtmFeatureInDtmObject(dtmObjP, DTMFeatureType::Void, dtmObjP->nullUserTag, 1, &dtmObjP->nullFeatureId, (DPoint3d*)&(stitchedNeighborsBoundary[i][0]), (long)stitchedNeighborsBoundary[i].size());
        assert(status == SUCCESS);
        }
    status = bcdtmObject_triangulateDtmObject(dtmObjP);
    assert(status == SUCCESS || ((*dtmObjP).numPoints < 4));
    if ((*dtmObjP).numPoints < 4) return false;
    if (status == SUCCESS)
        {
        IScalableMeshMeshPtr meshPtr;

        bcdtmInterruptLoad_triangleShadeMeshFromDtmObject(dtmPtr->GetBcDTM()->GetTinHandle(), 10000000, 2, 1, &draw, false, DTMFenceType::None, DTMFenceOption::None, 0, 0, &meshPtr);

        ScalableMeshMesh* meshP((ScalableMeshMesh*)meshPtr.get());

        assert(meshP != 0);
        bvector<int> contourPointsID;
        CreateGraphFromIndexBuffer(&meshGraphStitched, (const long*)meshP->GetFaceIndexes(), (int)meshP->GetNbFaceIndexes(), (int)meshP->GetNbPoints(), contourPointsID, meshP->GetPoints());
        MTGARRAY_SET_LOOP(edgeID, (&meshGraphStitched))
            {
            assert((&meshGraphStitched)->CountNodesAroundFace(edgeID) == 3 || (&meshGraphStitched)->CountNodesAroundFace((&meshGraphStitched)->EdgeMate(edgeID)) == 3);
            int v = -1;
            (&meshGraphStitched)->TryGetLabel(edgeID, 0, v);
            assert(v > 0 && v <= (int)meshP->GetNbPoints());
            }
        MTGARRAY_END_SET_LOOP(edgeID, (&meshGraphStitched))
            stitchedPoints.resize(meshP->GetNbPoints());
        memcpy(&stitchedPoints[0], meshP->GetPoints(), meshP->GetNbPoints()*sizeof(DPoint3d));
        //merge meshes
        for (int i = 0; i < (int)stitchedPoints.size(); i++)
            {
            stitchedSet.insert(std::make_pair(stitchedPoints[i], i));
            //stitchedSet.push_back(std::make_pair(stitchedPoints[i], i));
            }

        for (size_t i = 0; i < nodePoints.size(); i++)
            {
            DPoint3d pt = nodePoints[i];
            if (stitchedSet.count(pt) != 0) pointsToDestPointsMap[i] = stitchedSet[pt];
            else pointsToDestPointsMap[i] = -1;
            }
        std::vector<DPoint3d> newMesh;
        //RemovePolygonInteriorFromGraph(&meshGraphStitched, boundary, &stitchedPoints[0], extentMin, extentMax);
        for (size_t i = 0; i < stitchedNeighborsExtents.size(); i++)
            {
            DPoint3d extMin, extMax;
            extMin = DPoint3d::FromXYZ(ExtentOp<EXTENT>::GetXMin(stitchedNeighborsExtents[i]), ExtentOp<EXTENT>::GetYMin(stitchedNeighborsExtents[i]), ExtentOp<EXTENT>::GetZMin(stitchedNeighborsExtents[i]));
            extMax = DPoint3d::FromXYZ(ExtentOp<EXTENT>::GetXMax(stitchedNeighborsExtents[i]), ExtentOp<EXTENT>::GetYMax(stitchedNeighborsExtents[i]), ExtentOp<EXTENT>::GetZMax(stitchedNeighborsExtents[i]));
            RemoveTrianglesWithinExtent(&meshGraphStitched, &stitchedPoints[0], extMin, extMax);
            }

        MergeGraphs(&meshGraphStitched, stitchedPoints, meshGraph, nodePoints, extentMin, extentMax, pointsToDestPointsMap, componentsPointsId);

/*#ifdef SINGLE_TILE
        if ((ExtentOp<EXTENT>::GetXMin(ext) > TILE_X || ExtentOp<EXTENT>::GetXMax(ext) < TILE_X || ExtentOp<EXTENT>::GetYMin(ext) > TILE_Y || ExtentOp<EXTENT>::GetYMax(ext) < TILE_Y)
            && (ExtentOp<EXTENT>::GetXMin(ext) > TILE_X2 || ExtentOp<EXTENT>::GetXMax(ext) < TILE_X2 || ExtentOp<EXTENT>::GetYMin(ext) > TILE_Y2 || ExtentOp<EXTENT>::GetYMax(ext) < TILE_Y2))
            {
            delete meshGraph;
            meshGraph = new MTGGraph();
            pts = new POINT[node->sizeTotal()];
            node->get(pts, node->sizeTotal());
            LoadAdjacencyData(meshGraph, &pts[0] + node->size() + nbPointsForFaceInd, graphSize);
            delete pts;
            // arraySize = UpdateMeshNodeFromGraph(node, &newNodePointData, meshGraph, nodePoints, nfaces, extentMin, extentMax);
            stitchedPoints.resize(nodePoints.size());
            void* serializedGraph = nullptr;
            size_t ct = meshGraph->WriteToBinaryStream(serializedGraph);
            node->m_nodeHeader.m_nbFaceIndexes = 0;
            node->m_nodeHeader.m_sizeMeshGraph = ct;
            size_t nOfPointsUsed = stitchedPoints.size() + (size_t)ceil((double)ct / sizeof(POINT));
            newNodePointData = new POINT[nOfPointsUsed];
            memcpy(newNodePointData, &nodePoints[0], nodePoints.size()*sizeof(POINT));
            memcpy(newNodePointData + nodePoints.size(), serializedGraph, ct);
            free(serializedGraph);
            arraySize = nOfPointsUsed;
            }
        else
#endif*/
            arraySize = UpdateMeshNodeFromGraph(node, &newNodePointData, &meshGraphStitched, stitchedPoints, nfaces, extentMin, extentMax);
        }
    }
    else {
      arraySize = UpdateMeshNodeFromGraph(node, &newNodePointData, meshGraph, nodePoints, nfaces, extentMin, extentMax);
    stitchedPoints.resize(nodePoints.size());
    }
    assert(newNodePointData != nullptr);
    node->setNbPointsUsedForMeshIndex(0);
    node->clear();
   // delete meshGraph;
    size_t removed = 0;
    for (auto& id : componentsPointsId)
        {
        if (id > (int)stitchedPoints.size()) componentsPointsId[(&id - &componentsPointsId[0])] = componentsPointsId[componentsPointsId.size() - (1 + removed)];
        ++removed;
        }
    componentsPointsId.resize(componentsPointsId.size() - removed);
    assert(componentsPointsId.size() <= 10000);
    if (node->m_nodeHeader.m_meshComponents == nullptr) node->m_nodeHeader.m_meshComponents = new int[componentsPointsId.size()];
    else if (node->m_nodeHeader.m_numberOfMeshComponents != componentsPointsId.size())
        {
        delete[] node->m_nodeHeader.m_meshComponents;
        node->m_nodeHeader.m_meshComponents = new int[componentsPointsId.size()];
        }
    memcpy(node->m_nodeHeader.m_meshComponents, componentsPointsId.data(), componentsPointsId.size()*sizeof(int));
    node->m_nodeHeader.m_numberOfMeshComponents = componentsPointsId.size();
    node->push_back(&newNodePointData[0], arraySize);
    node->setNbPointsUsedForMeshIndex(arraySize-stitchedPoints.size());
    if (node->IsLeaf()) node->m_nodeHeader.m_totalCount = stitchedPoints.size();
    delete[] newNodePointData;
    assert(node->size() == stitchedPoints.size());
    //node->m_nodeHeader.m_nbFaceIndexes = nfaces;
    return true;
    }
//return false;
//    }
//#pragma optimize("", on)  

    void Create3dDelaunayMesh (DPoint3dCP points, int numPoints, int (*draw) (DTMFeatureType dtmFeatureType, int numTriangles, int numMeshPts, DPoint3d *meshPtsP, DPoint3d *meshVectorsP, int numMeshFaces, long *meshFacesP, void *userP), void* userP, MTGGraph* graphP, bool tetGen, double trimLength = -1);

/**----------------------------------------------------------------------------
Initiates the meshing of the node. Ther meshing process
will compute the mesh.
-----------------------------------------------------------------------------*/
template<class POINT, class EXTENT> bool ScalableMesh3DDelaunayMesher<POINT, EXTENT>::Mesh(HFCPtr<SMMeshIndexNode<POINT, EXTENT> > node) const
    {  
#ifdef NO_3D_MESH	
return true;
#else
    //NEEDS_WORK_SM
    if (node->size () > 4)
        {
        vector<DPoint3d> points (node->size ());
        std::transform (node->begin (), node->end (), &points[0], ToBcPtConverter ());

        IScalableMeshMeshPtr meshPtr;

#ifndef NO_3D_MESH	
        Create3dDelaunayMesh(&points[0], (int)node->size(), &draw, &meshPtr, nullptr, m_tetGen);
#endif

        //std::sort (points.begin (), points.end (), [](DPoint3d& a, DPoint3d& b)
        //    {
        //    if (a.x < b.x) return true;
        //    if (a.x > b.x) return false;
        //    return (a.y < b.y);
        //    }
        //);
        ScalableMeshMesh* meshP ((ScalableMeshMesh*)meshPtr.get ());

        assert (meshP != 0);

        if (meshP != 0)
            {
            node->clear();

            //NEEDS_WORK_SM Avoid some assert            
            if (points.size() != meshP->GetNbPoints() && node->IsLeaf())
                {
                node->m_nodeHeader.m_totalCount = meshP->GetNbPoints();
                }

            vector<POINT> nodePts(meshP->GetNbPoints());

            for (size_t pointInd = 0; pointInd < meshP->GetNbPoints(); pointInd++)
                {
                nodePts[pointInd].x = meshP->GetPoints()[pointInd].x;
                nodePts[pointInd].y = meshP->GetPoints()[pointInd].y;
                nodePts[pointInd].z = meshP->GetPoints()[pointInd].z;
                }
            node->push_back(&nodePts[0], nodePts.size());
            bvector<int> componentPointsId; //holds the leftmost point of each connected component
            if (NULL == node->GetGraphPtr()) node->CreateGraph();
            CreateGraphFromIndexBuffer(node->GetGraphPtr(), (const long*)meshP->GetFaceIndexes(), (int)meshP->GetNbFaceIndexes(), (int)nodePts.size(), componentPointsId, meshP->GetPoints());
            node->SetGraphDirty();
            node->StoreGraph();
            vector<int> faceIndexes;
            const int*  indexList = meshP->GetFaceIndexes();
           for (int i = 0; i < meshP->GetNbFaceIndexes(); i+=3)
                {
                bool includeInList = true;
                if (includeInList)
                    {
                    faceIndexes.push_back(indexList[i]);
                    faceIndexes.push_back(indexList[i+1]);
                    faceIndexes.push_back(indexList[i+2]);
                    }
                }
           /* MTGGraph* meshGraph = node->GetGraphPtr();
            MTGMask visitedMask = meshGraph->GrabMask();
            MTGARRAY_SET_LOOP(edgeID, meshGraph)
                {
                bool isComponent = true;
                int vtx = -1;
                meshGraph->TryGetLabel(edgeID, 1, vtx);
                //if (vtx > 0) for (int j = 0; j < componentPointsId.size() && !isComponent; j++) if (componentPointsId[j] == vtx - 1) isComponent = true;
                if (isComponent && meshGraph->GetMaskAt(edgeID, MTG_EXTERIOR_MASK))
                    {
                    MTGARRAY_FACE_LOOP(extID, meshGraph, edgeID)
                        {
                        if (meshGraph->GetMaskAt(extID, visitedMask) || !meshGraph->GetMaskAt(extID, MTG_EXTERIOR_MASK)) break;
                        MTGARRAY_FACE_LOOP(faceID, meshGraph, meshGraph->EdgeMate(extID))
                            {
                            int vtx2 = -1;
                            meshGraph->TryGetLabel(faceID, 0, vtx2);
                            faceIndexes.push_back(vtx2);
                            }
                        MTGARRAY_END_FACE_LOOP(faceID, meshGraph, meshGraph->EdgeMate(extID))
                        meshGraph->SetMaskAt(extID, visitedMask);
                        }
                    MTGARRAY_END_FACE_LOOP(extID, meshGraph, edgeID)
                    }
                }
            MTGARRAY_END_SET_LOOP(edgeID, meshGraph)
                meshGraph->ClearMask(visitedMask);
                meshGraph->DropMask(visitedMask);*/
            //NEEDS_WORK_SM - Bad name - should be nbFacesIndexes
            node->m_nodeHeader.m_nbFaceIndexes = faceIndexes.size();//meshP->GetNbFaceIndexes();
            if (componentPointsId.size() > 0)
                {
                if (node->m_nodeHeader.m_meshComponents == nullptr) node->m_nodeHeader.m_meshComponents = new int[componentPointsId.size()];
                else if (node->m_nodeHeader.m_numberOfMeshComponents != componentPointsId.size())
                    {
                    delete[] node->m_nodeHeader.m_meshComponents;
                    node->m_nodeHeader.m_meshComponents = new int[componentPointsId.size()];
                    }
                node->m_nodeHeader.m_numberOfMeshComponents = componentPointsId.size();
                memcpy(node->m_nodeHeader.m_meshComponents, componentPointsId.data(), componentPointsId.size()*sizeof(int));
                }
            //NEEDS_WORK_SM - Ugly hack, piggyback the face indexes at the end of the points
            size_t nbPointsForFaceInd = (size_t)ceil((node->m_nodeHeader.m_nbFaceIndexes * (double)sizeof(int32_t)) / (double)sizeof(POINT));

            POINT* pPiggyBackMeshIndexes = new POINT[nbPointsForFaceInd];

            memcpy(pPiggyBackMeshIndexes, /*meshP->GetFaceIndexes()*/ &faceIndexes[0], node->m_nodeHeader.m_nbFaceIndexes * sizeof(int32_t));

            node->push_back(pPiggyBackMeshIndexes, nbPointsForFaceInd);


            /*//NEEDS_WORK_SM : Remove if normal not computed at generation time
            DVec3d* calculatedNormals = 0;

            CalcNormals (calculatedNormals,
                         node->m_nodeHeader.m_nbFaceIndexes,                  
                  const DVec3d& viewNormalParam, 
                  size_t        nbPoints, 
                  DPoint3d*     pPoints, 
                  size_t        nbFaceIndexes, 
                  Int32*        pFaceIndexes) 
                  */


            node->setNbPointsUsedForMeshIndex(nbPointsForFaceInd);
            delete[] pPiggyBackMeshIndexes;
            assert(node->size() == nodePts.size());
            if (node->IsLeaf() && node->size() != node->m_nodeHeader.m_totalCount)
                {
                node->m_nodeHeader.m_totalCount = node->size();
                }

          //  isMeshingDone = true;

           

            }
        }

    return true;
#endif
    }


    template<class POINT, class EXTENT> size_t ScalableMesh3DDelaunayMesher<POINT, EXTENT>::UpdateMeshNodeFromGraph(HFCPtr<SMMeshIndexNode<POINT, EXTENT> > node, POINT** newMesh, MTGGraph * meshGraphStitched, std::vector<DPoint3d>& stitchedPoints, int& nFaces, DPoint3d& minPt, DPoint3d& maxPt) const
    {
    
    //POINT* geometryData = new POINT[stitchedPoints.size()];
    vector<int> faceIndices;
    /*for (size_t i = 0; i < stitchedPoints.size(); i++)
        {
        geometryData[i].x = stitchedPoints[i].x;
        geometryData[i].y = stitchedPoints[i].y;
        geometryData[i].z = stitchedPoints[i].z;
        }*/
    int* pointsInTile = new int[stitchedPoints.size()];
    for (size_t i = 0; i < stitchedPoints.size(); i++) pointsInTile[i] = -1;
    MTGMask visitedMask = meshGraphStitched->GrabMask();
    std::vector<int> indicesForFace;
    int newPtsIndex = 0;
    MTGARRAY_SET_LOOP(edgeID, meshGraphStitched)
        {
        if (!meshGraphStitched->GetMaskAt(edgeID, MTG_EXTERIOR_MASK) && !meshGraphStitched->GetMaskAt(edgeID, visitedMask))
            {
            //assert(meshGraphStitched->CountNodesAroundFace(edgeID) == 3);
            int n = 0;
            int nPointsOutsideTile = 0;
            indicesForFace.clear();
            std::vector<MTGNodeId> faceNodes;
            if (meshGraphStitched->CountNodesAroundFace(edgeID) != 3) continue;
            MTGARRAY_FACE_LOOP(faceID, meshGraphStitched, edgeID)
                {
                int vIndex = -1;
                meshGraphStitched->TryGetLabel(faceID, 0, vIndex);
                assert(vIndex > 0);
                assert(vIndex <= (int)stitchedPoints.size());
                DPoint3d pt = stitchedPoints[vIndex - 1];
                if (pt.x < minPt.x || pt.y < minPt.y || pt.z < minPt.z || pt.x > maxPt.x || pt.y > maxPt.y || pt.z > maxPt.z) nPointsOutsideTile++;
                                // geometryData.push_back(PointOp<POINT>::Create(stitchedPoints[vIndex - 1].x, stitchedPoints[vIndex - 1].y, stitchedPoints[vIndex - 1].z));
                if (nPointsOutsideTile < 3)
                    {
                    if (pointsInTile[vIndex - 1] == -1)
                        {
                        pointsInTile[vIndex - 1] = newPtsIndex;
                        newPtsIndex++;
                        assert(pointsInTile[vIndex - 1] < newPtsIndex);
                        indicesForFace.push_back(vIndex - 1);
                        }
                    faceIndices.push_back(pointsInTile[vIndex - 1]+1);
                    }
                else
                    {
                    faceIndices.resize(faceIndices.size() - n); //erase last face
                    for (int index : indicesForFace)
                        {
                        pointsInTile[index] = -1;
                        }
                    newPtsIndex -= (int) indicesForFace.size();
                    indicesForFace.clear();
                    }
                if (n == 2 && nPointsOutsideTile < 3)
                    {
                    meshGraphStitched->TrySetLabel(faceID, 0, pointsInTile[vIndex - 1]+1);
                    int v1 = -1;
                    meshGraphStitched->TryGetLabel(meshGraphStitched->FSucc(faceID), 0, v1);
                    meshGraphStitched->TrySetLabel(meshGraphStitched->FSucc(faceID), 0, pointsInTile[v1 - 1] + 1);
                    meshGraphStitched->TryGetLabel(meshGraphStitched->FPred(faceID), 0, v1);
                    meshGraphStitched->TrySetLabel(meshGraphStitched->FPred(faceID), 0, pointsInTile[v1 - 1] + 1);
                    }
                                //meshGraphStitched->TrySetLabel(faceID, 0, (int)geometryData.size());
                meshGraphStitched->SetMaskAt(faceID, visitedMask);
                ++n;
                }
            MTGARRAY_END_FACE_LOOP(faceID, meshGraphStitched, edgeID)
            }
        }
    MTGARRAY_END_SET_LOOP(edgeID, meshGraphStitched)
    meshGraphStitched->ClearMask(visitedMask);
    meshGraphStitched->DropMask(visitedMask);
    MTGARRAY_SET_LOOP(toDeleteID, meshGraphStitched)
        {
        int index = -1;
        meshGraphStitched->TryGetLabel(toDeleteID, 0, index);
        if (index == -1 || index > newPtsIndex)  meshGraphStitched->DropEdge(toDeleteID);
        }
    MTGARRAY_END_SET_LOOP(toDeleteID, meshGraphStitched)
    POINT* geometryData = new POINT[newPtsIndex]; 
    for (size_t i = 0; i < stitchedPoints.size(); i++)
        {
        if (pointsInTile[i] != -1)
            {
            assert(pointsInTile[i] < newPtsIndex);
            geometryData[pointsInTile[i]].x = stitchedPoints[i].x;
            geometryData[pointsInTile[i]].y = stitchedPoints[i].y;
            geometryData[pointsInTile[i]].z = stitchedPoints[i].z;
            }
        }
    stitchedPoints.resize(newPtsIndex);
    delete[] pointsInTile;
   //ValidateTriangleMesh(faceIndices, stitchedPoints);
    *node->GetGraphPtr() = *meshGraphStitched;
    node->SetGraphDirty();
    node->m_nodeHeader.m_nbFaceIndexes = faceIndices.size();
    assert(faceIndices.size() % 3 == 0);
    nFaces = (int)faceIndices.size();
    size_t nOfPointsUsed = stitchedPoints.size() + (size_t)ceil(faceIndices.size()*(double)sizeof(int) / sizeof(POINT)); 
    *newMesh = new POINT[nOfPointsUsed];
    memcpy(*newMesh, geometryData, /*stitchedPoints.size()*/newPtsIndex*sizeof(POINT));
    delete geometryData;
    memcpy(*newMesh + stitchedPoints.size(), &faceIndices[0], faceIndices.size()*sizeof(int));
    return nOfPointsUsed;
    }

    template<class POINT, class EXTENT> void ScalableMesh3DDelaunayMesher<POINT, EXTENT>::SelectPointsToStitch(std::vector<DPoint3d>& stitchedPoints, HFCPtr<SMMeshIndexNode<POINT, EXTENT> > node, MTGGraph* meshGraph, EXTENT neighborExt, vector<int>* pointsToDestPointsMap) const
    {
    MTGNodeId boundaryEdgeID = -1;
    int* boundaryPoints = node->m_nodeHeader.m_meshComponents;
    std::set<int> pts;
    std::queue<MTGNodeId> bounds;
    if (boundaryPoints != nullptr)
        {
        for (size_t i = 0; i < node->m_nodeHeader.m_numberOfMeshComponents;i++) pts.insert(boundaryPoints[i]);
        }
    MTGARRAY_SET_LOOP(edgeID, meshGraph)
        {
        int vtx = -1;
        meshGraph->TryGetLabel(edgeID, 0, vtx);
        if (pts.count(vtx-1) != 0) bounds.push(edgeID);
        if (meshGraph->GetMaskAt(edgeID, MTG_BOUNDARY_MASK))
            {
            boundaryEdgeID = edgeID;
            //break;
            }
        }
    MTGARRAY_END_SET_LOOP(edgeID, meshGraph)
        if (bounds.size() == 0)
            {
            //assert(boundaryEdgeID != -1);
            bounds.push(boundaryEdgeID);
            }
    //get face of current edge
    MTGNodeId face[3];
    DPoint3d facePoints[3];
    MTGMask visitedMask = meshGraph->GrabMask();
    while (bounds.size() > 0)
        {
        MTGNodeId currentID = bounds.front();
        if (!meshGraph->GetMaskAt(currentID, MTG_BOUNDARY_MASK) || meshGraph->GetMaskAt(currentID, visitedMask))
            {
            if (meshGraph->GetMaskAt(currentID, MTG_EXTERIOR_MASK) && !meshGraph->GetMaskAt(currentID, visitedMask) && !meshGraph->GetMaskAt(meshGraph->EdgeMate(currentID), visitedMask)) bounds.push(meshGraph->EdgeMate(currentID));
            meshGraph->SetMaskAt(currentID, visitedMask);
            bounds.pop();
            continue;
            }
        meshGraph->SetMaskAt(currentID, visitedMask);
        int edgeAroundFace = 0, vIndex;

        MTGARRAY_FACE_LOOP(aroundFaceIndex, meshGraph, currentID)
            {
            if (edgeAroundFace < 3)
                {
                face[edgeAroundFace] = aroundFaceIndex;
                meshGraph->TryGetLabel(aroundFaceIndex, 0, vIndex);
                assert(vIndex > 0);
                POINT pt = (*node)[vIndex - 1];
                facePoints[edgeAroundFace] = DPoint3d::FromXYZ(pt.x, pt.y, pt.z);
                }
            edgeAroundFace++;
            if (edgeAroundFace > 3)break;
            }
        MTGARRAY_END_FACE_LOOP(aroundFaceIndex, meshGraph, currentID)
        if (edgeAroundFace>3)
            {
            if (!meshGraph->GetMaskAt(meshGraph->EdgeMate(currentID), MTG_EXTERIOR_MASK))
                {
                meshGraph->SetMaskAt(meshGraph->EdgeMate(currentID), MTG_BOUNDARY_MASK);
                meshGraph->SetMaskAt(currentID, MTG_EXTERIOR_MASK);
                meshGraph->ClearMaskAt(currentID, MTG_BOUNDARY_MASK);
                bounds.push(meshGraph->EdgeMate(currentID));
                }
            bounds.pop();
            continue;
            }
        DPoint3d sphereCenter(DPoint3d::FromZero());        
        double sphereRadius = 0;
        //bound_sphereCompute(&sphereCenter, &sphereRadius, facePoints,3);
        //EXTENT neighborExt = node->m_apNeighborNodes[neighborInd][neighborSubInd]->GetContentExtent();
        if (ExtentOp<EXTENT>::Overlap(neighborExt, ExtentOp<EXTENT>::Create(sphereCenter.x - sphereRadius, sphereCenter.y - sphereRadius,
            sphereCenter.z - sphereRadius, sphereCenter.x + sphereRadius,
            sphereCenter.y + sphereRadius, sphereCenter.z + sphereRadius)))
            {
            for (int i = 0; i < 3; i++)
                {
                meshGraph->TryGetLabel(face[i], 0, vIndex);
                assert(vIndex > 0);
                stitchedPoints.push_back(facePoints[i]);
                if (nullptr != pointsToDestPointsMap) (*pointsToDestPointsMap)[vIndex - 1] = (int)stitchedPoints.size() - 1;
                if (!meshGraph->GetMaskAt(meshGraph->EdgeMate(face[i]), MTG_EXTERIOR_MASK))
                    {
                    meshGraph->SetMaskAt(meshGraph->EdgeMate(face[i]), MTG_BOUNDARY_MASK);
                    bounds.push(meshGraph->EdgeMate(face[i]));
                    meshGraph->ClearMaskAt(face[i], MTG_BOUNDARY_MASK);
                    meshGraph->SetMaskAt(face[i], MTG_EXTERIOR_MASK);
                    MTGARRAY_FACE_LOOP(mateFaceIdx, meshGraph, meshGraph->EdgeMate(face[i]))
                        {
                        meshGraph->TryGetLabel(mateFaceIdx, 0, vIndex);
                        assert(vIndex > 0);
                        assert((vIndex - 1) <= (int)node->size());
                        POINT pt = (*node)[vIndex - 1];
                        stitchedPoints.push_back(DPoint3d::FromXYZ(pt.x, pt.y, pt.z));
                        if (nullptr != pointsToDestPointsMap)  (*pointsToDestPointsMap)[vIndex - 1] = (int)stitchedPoints.size() - 1;
                        }
                    MTGARRAY_END_FACE_LOOP(mateFaceIdx, meshGraph, meshGraph->EdgeMate(face[i]))
                    }
                else
                    {
                    meshGraph->TryGetLabel(meshGraph->EdgeMate(face[i]), 0, vIndex);
                    assert(vIndex > 0);
                    assert((vIndex - 1) <= (int)node->size());
                    POINT pt = (*node)[vIndex - 1];
                    stitchedPoints.push_back(DPoint3d::FromXYZ(pt.x, pt.y, pt.z));
                    if (nullptr != pointsToDestPointsMap)  (*pointsToDestPointsMap)[vIndex - 1] = (int)stitchedPoints.size() - 1;
                    meshGraph->DropEdge(face[i]);
                    }
                }
            /*if (!meshGraph->GetMaskAt(meshGraph->EdgeMate(currentID), MTG_EXTERIOR_MASK))
                {
                meshGraph->ClearMaskAt(currentID, MTG_BOUNDARY_MASK);
                meshGraph->SetMaskAt(currentID, MTG_EXTERIOR_MASK);
                }
            else meshGraph->DropEdge(currentID);*/
            }
        else
            {
            for (int i = 0; i < 3; i++)
                {
                meshGraph->TryGetLabel(face[i], 0, vIndex);
                assert(vIndex > 0);
                stitchedPoints.push_back(facePoints[i]);
                if (nullptr != pointsToDestPointsMap)  (*pointsToDestPointsMap)[vIndex - 1] = (int)stitchedPoints.size() - 1;
                }
            //if (!meshGraph->GetMaskAt(meshGraph->FSucc(currentID), visitedMask)) bounds.push(meshGraph->FSucc(currentID));
            if (meshGraph->GetMaskAt(meshGraph->FSucc(meshGraph->EdgeMate(currentID)), MTG_EXTERIOR_MASK) && !meshGraph->GetMaskAt(meshGraph->FSucc(meshGraph->EdgeMate(currentID)), visitedMask)) bounds.push(meshGraph->FSucc(meshGraph->EdgeMate(currentID)));
            /*if (bounds.size() == 1)
                {
                MTGARRAY_SET_LOOP(edgeID, meshGraph)
                    {
                    if (edgeID < currentID && meshGraph->GetMaskAt(edgeID, MTG_BOUNDARY_MASK) && !meshGraph->GetMaskAt(edgeID, MTG_EXTERIOR_MASK))
                        {
                        bounds.push(edgeID);
                        }
                    }
                MTGARRAY_END_SET_LOOP(boundaryEdgeID, meshGraph)
                }*/
            }
        bounds.pop();
        }
    meshGraph->DropMask(visitedMask);
    }
//#pragma optimize("", on)
/**----------------------------------------------------------------------------
Initiates the stitching of the mesh present in neighbor nodes. 
-----------------------------------------------------------------------------*/
struct compare3D
    {
    bool operator()(const DPoint3d& a, const DPoint3d& b)
         {
         if (a.x < b.x) return true;
         if (a.x > b.x) return false;
         if (a.y < b.y) return true;
         if (a.y > b.y) return false;
         if (a.z < b.z) return true;
         return false;
         }
     };

template<class POINT, class EXTENT> bool ScalableMesh3DDelaunayMesher<POINT, EXTENT>::Stitch(HFCPtr<SMMeshIndexNode<POINT, EXTENT> > node) const
    {
    return true; //deactivated for now

    if (NULL == node->GetGraphPtr()) node->LoadGraph();
    MTGGraph* meshGraph = node->GetGraphPtr();
    size_t neighborIndices[27] = { 0,1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25 };//{ 1, 3, 4, 6, 12, 21 }; //only neighbors that share a face at the moment
    size_t nodeIndicesInNeighbor[27] = { 7, 6, 5, 4, 3, 2, 1, 0, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8 };//{ 6, 4, 3, 1, 21, 12 };
    vector<DPoint3d> stitchedPoints;
    vector<int> pointsToDestPointsMap(node->size());
    std::fill_n(pointsToDestPointsMap.begin(), pointsToDestPointsMap.size(), -1);
    
    EXTENT ext = node->GetContentExtent();
    
    std::string neighborsVisited;
    for (size_t& neighborInd : neighborIndices)
        {
        size_t idx = &neighborInd - &neighborIndices[0];
        if ((node->m_apNeighborNodes[neighborInd].size() > 0))
            {
            neighborsVisited += " visited neighbor " + std::to_string(neighborInd);
            for (size_t neighborSubInd = 0; neighborSubInd < node->m_apNeighborNodes[neighborInd].size(); neighborSubInd++)
                {
                if (node->m_apNeighborNodes[neighborInd][neighborSubInd]->m_nodeHeader.m_nbFaceIndexes == 0) continue;
              //  EXTENT neighborExt = node->m_apNeighborNodes[neighborInd][neighborSubInd]->GetContentExtent();
                
                if (node->m_nodeHeader.m_apAreNeighborNodesStitched[neighborInd] == false)
                    {
                    HFCPtr < SMMeshIndexNode<POINT, EXTENT>> meshNode = dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(node->m_apNeighborNodes[neighborInd][neighborSubInd]);
                    if (NULL == meshNode->GetGraphPtr()) meshNode->LoadGraph();
                    MTGGraph* meshGraphNeighbor = meshNode->GetGraphPtr();
                    SelectPointsToStitch(stitchedPoints, node, meshGraph, node->m_apNeighborNodes[neighborInd][neighborSubInd]->GetContentExtent(), nullptr);
                    SelectPointsToStitch(stitchedPoints, static_cast<SMMeshIndexNode<POINT,EXTENT>*>(&*node->m_apNeighborNodes[neighborInd][neighborSubInd]), meshGraphNeighbor, node->GetContentExtent(), nullptr);
                    node->m_apNeighborNodes[neighborInd][neighborSubInd]->m_nodeHeader.m_apAreNeighborNodesStitched[nodeIndicesInNeighbor[idx]] = true;
                    }
                else
                    {
                    //std::vector<DPoint3d> stitchedPointsTemp;
                    //SelectPointsToStitch(stitchedPointsTemp, node, meshGraph, neighborExt, nullptr);
                    }
                               //node->m_nodeHeader.m_apAreNeighborNodesStitched[neighborInd] = true;
                }
                neighborsVisited += " for neighbor " + std::to_string(neighborInd) + " now have " + std::to_string(stitchedPoints.size()) + " points\r\n";

            }
        }
    //mesh aggregated points
    MTGGraph meshGraphStitched;
    IScalableMeshMeshPtr meshPtr;
    DPoint3d extentMin, extentMax;
    extentMin = DPoint3d::FromXYZ(ExtentOp<EXTENT>::GetXMin(ext), ExtentOp<EXTENT>::GetYMin(ext), ExtentOp<EXTENT>::GetZMin(ext));
    extentMax = DPoint3d::FromXYZ(ExtentOp<EXTENT>::GetXMax(ext), ExtentOp<EXTENT>::GetYMax(ext), ExtentOp<EXTENT>::GetZMax(ext));
    if (stitchedPoints.size() <= 3) return false;
#ifndef NO_3D_MESH	
    Create3dDelaunayMesh (&stitchedPoints[0], (int)stitchedPoints.size (), &draw, &meshPtr, nullptr, m_tetGen);
#endif
    ScalableMeshMesh* meshP((ScalableMeshMesh*)meshPtr.get());
    assert(meshP != 0);
    bvector<int> contourPointsId;
    CreateGraphFromIndexBuffer(&meshGraphStitched, (const long*)meshP->GetFaceIndexes(), (int)meshP->GetNbFaceIndexes(), (int)meshP->GetNbPoints(), contourPointsId, meshP->GetPoints());
#ifndef NDEBUG
  /*  MTGARRAY_SET_LOOP(edgeID, (&meshGraphStitched))
        {
        assert((&meshGraphStitched)->CountNodesAroundFace(edgeID) == 3 || (&meshGraphStitched)->CountNodesAroundFace((&meshGraphStitched)->EdgeMate(edgeID)) == 3);
        }
    MTGARRAY_END_SET_LOOP(edgeID, (&meshGraphStitched))*/
#endif
    stitchedPoints.resize(meshP->GetNbPoints());
    memcpy(&stitchedPoints[0], meshP->GetPoints(), meshP->GetNbPoints()*sizeof(DPoint3d));
    //merge meshes
    std::vector<DPoint3d> nodePoints(node->size());

    std::map<DPoint3d, int, compare3D> stitchedSet;
        //std::vector<std::pair<DPoint3d, int>> stitchedSet;
    for (size_t i = 0; i < node->size(); i++)
        {
        nodePoints[i].x = (*node)[i].x;
        nodePoints[i].y = (*node)[i].y;
        nodePoints[i].z = (*node)[i].z;
        }

    for (int i = 0; i < (int)stitchedPoints.size(); i++)
        {
        stitchedSet.insert(std::make_pair(stitchedPoints[i], i));
                //stitchedSet.push_back(std::make_pair(stitchedPoints[i], i));
        }

    for (size_t i = 0; i < nodePoints.size(); i++)
        {
        DPoint3d pt = nodePoints[i];
        if (stitchedSet.count(pt) != 0) pointsToDestPointsMap[i] = stitchedSet[pt];
        else pointsToDestPointsMap[i] = -1;
        }
    std::vector<DPoint3d> newMesh;
    bvector<int> componentsPointsId;
    MergeGraphs(&meshGraphStitched, stitchedPoints, meshGraph, nodePoints, extentMin, extentMax, pointsToDestPointsMap, componentsPointsId);
    POINT* newNodePointData = nullptr;
    int nfaces = 0;
    size_t arraySize = UpdateMeshNodeFromGraph(node, &newNodePointData, &meshGraphStitched, stitchedPoints, nfaces, extentMin, extentMax);
   // delete meshGraph;
    assert(newNodePointData != nullptr);
    size_t removed = 0;
    string size = "";
    for (auto& id : componentsPointsId)
        {
        if (id > (int)stitchedPoints.size()) componentsPointsId[(&id - &componentsPointsId[0])] = componentsPointsId[componentsPointsId.size() - (1 + removed)];
        ++removed;
        }
    componentsPointsId.resize(componentsPointsId.size() - removed);
    assert(componentsPointsId.size() <= 10000);
    if (node->m_nodeHeader.m_meshComponents == nullptr) node->m_nodeHeader.m_meshComponents = new int[componentsPointsId.size()];
    else if (node->m_nodeHeader.m_numberOfMeshComponents != componentsPointsId.size())
        {
        delete[] node->m_nodeHeader.m_meshComponents;
        node->m_nodeHeader.m_meshComponents = new int[componentsPointsId.size()];
        }
    size += std::to_string(arraySize);
    memcpy(node->m_nodeHeader.m_meshComponents, componentsPointsId.data(), componentsPointsId.size()*sizeof(int));
    node->m_nodeHeader.m_numberOfMeshComponents = componentsPointsId.size();
    node->setNbPointsUsedForMeshIndex(0);
    node->clear();
    node->push_back(&newNodePointData[0], arraySize);
    node->setNbPointsUsedForMeshIndex(arraySize - stitchedPoints.size());
    delete[] newNodePointData;
    if (node->IsLeaf()) node->m_nodeHeader.m_totalCount = stitchedPoints.size();
    assert(node->size() == stitchedPoints.size());
    return true;
    }
