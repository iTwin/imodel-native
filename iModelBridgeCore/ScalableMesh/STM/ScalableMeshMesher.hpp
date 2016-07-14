//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: STM/ScalableMeshMesher.hpp $
//:>    $RCSfile: ScalableMeshQuadTreeBCLIBFilters.hpp,v $
//:>   $Revision: 1.28 $
//:>       $Date: 2011/04/27 17:17:56 $
//:>     $Author: Alain.Robert $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------


#include <ImagePP/all/h/HFCException.h>
#include <ScalableMesh\ScalableMeshUtilityFunctions.h>
#include <ScalableMesh\IScalableMeshQuery.h>
//#include <eigen\Eigen\Dense>
//#include <PCLWrapper\IDefines.h>
//#include <PCLWrapper\INormalCalculator.h>
#include "ScalableMeshQuery.h"
//#include "MeshingFunctions.h"
#include <ScalableMesh\ScalableMeshUtilityFunctions.h>
#include <Mtg/MtgStructs.h>
#include <Geom/bsp/bspbound.fdf>
#include "ScalableMesh\ScalableMeshGraph.h"
#include <string>
#include <queue>
#include <ctime>
#include <fstream>
#include "Edits/ClipUtilities.h"
#include "vuPolygonClassifier.h"
#include "LogUtils.h"
extern bool s_useSpecialTriangulationOnGrids;
//#define SINGLE_TILE
//#define TILE_X 2138568
#define TILE_X 68062
//#define TILE_Y 220088.85
#define TILE_Y 72035
//#define TILE_X2 2138568
#define TILE_X2 214155
#define TILE_Y2 224205
#define SM_OUTPUT_MESHES_STITCHING 0
#define SM_TRACE_FEATURE_DEFS 0
#define SM_TRACE_FEATURE_DEFINITIONS 0
#define SM_TRACE_MESH_STATS 0
//#define TILE_Y2 220088.85


/**----------------------------------------------------------------------------
 Initiates a filtering of the node. Ther filtering process
 will compute the sub-resolution and the view oriented parameters.
 -----------------------------------------------------------------------------*/
bool firstTile = false;
template<class POINT, class EXTENT> bool ScalableMesh2DDelaunayMesher<POINT, EXTENT>::Mesh(HFCPtr<SMMeshIndexNode<POINT, EXTENT> > node) const
    {
    bool isMeshingDone = false;
    LOG_SET_PATH("E:\\output\\scmesh\\2016-05-27\\")
    LOG_SET_PATH_W("E:\\output\\scmesh\\2016-05-27\\")
    //LOGSTRING_NODE_INFO(node, LOG_PATH_STR)
    //LOGSTRING_NODE_INFO_W(node, LOG_PATH_STR_W)
    //NEEDS_WORK_SM

    RefCountedPtr<SMMemoryPoolVectorItem<POINT>> pointsPtr(node->GetPointsPtr());

    if (pointsPtr->size() > 4)
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

        BENTLEY_NAMESPACE_NAME::TerrainModel::DTMPtr dtmPtr;

        int status = CreateBcDTM(dtmPtr);

        assert(status == SUCCESS);

#if SM_TRACE_MESH_STATS
        Utf8String nameStats = LOG_PATH_STR + "tilebeforemeshing_";
        LOGSTRING_NODE_INFO(node, nameStats)
        nameStats.append(".txt");
        std::ofstream stats;
        stats.open(nameStats.c_str(), std::ios_base::trunc);
        stats << " N OF POINTS " + std::to_string(pointsPtr->size())+"\n";
        stats << "N OF INDICES " + std::to_string(node->m_nodeHeader.m_nbFaceIndexes) + "\n";
        stats << " NODE TOTAL COUNT "+std::to_string(node->m_nodeHeader.m_totalCount)+"\n";
        stats.close();
#endif
        
        vector<DPoint3d> points(pointsPtr->size());
        
        PtToPtConverter::Transform(&points[0], &(*pointsPtr)[0], points.size());

        BC_DTM_OBJ* dtmObjP(dtmPtr->GetBcDTM()->GetTinHandle());
        RefCountedPtr<SMMemoryPoolVectorItem<int32_t>>  linearFeaturesPtr = node->GetLinearFeaturesPtr();
        bvector<bvector<int32_t>> defs;
        if (linearFeaturesPtr->size() > 0) node->GetFeatureDefinitions(defs, &*linearFeaturesPtr->begin(), linearFeaturesPtr->size());
        if (!node->m_isGrid || linearFeaturesPtr->size() > 0 || !s_useSpecialTriangulationOnGrids)
            {
            status = bcdtmObject_storeDtmFeatureInDtmObject(dtmObjP, DTMFeatureType::RandomSpots, dtmObjP->nullUserTag, 1, &dtmObjP->nullFeatureId, &points[0], (long)pointsPtr->size());

            assert(status == SUCCESS);

            for (size_t i = 0; i < defs.size(); ++i)
                {
                vector<DPoint3d> feature;
                for (size_t j = 1; j < defs[i].size(); ++j)
                    {
                    if (defs[i][j] < points.size()) feature.push_back(points[defs[i][j]]);
                    }
                if (IsClosedFeature((ISMStore::FeatureType)defs[i][0]) && DVec3d::FromStartEnd(feature.front(), feature.back()).Magnitude() > 0) feature.push_back(feature.front());
#if SM_TRACE_FEATURE_DEFINITIONS
                WString namePoly = LOG_PATH_STR_W + L"prefeaturepoly_";
                LOGSTRING_NODE_INFO_W(node, namePoly)
                    namePoly.append(L"_");
                    namePoly.append(to_wstring(i).c_str());
                namePoly.append(L".p");
                LOG_POLY_FROM_FILENAME_AND_BUFFERS_W(namePoly,feature.size(),&feature[0])
#endif
                    status = bcdtmObject_storeDtmFeatureInDtmObject(dtmObjP, (DTMFeatureType)defs[i][0], dtmObjP->nullUserTag, 1, &dtmObjP->nullFeatureId, &feature[0], (long)feature.size());
                }
            status = bcdtmObject_triangulateDtmObject(dtmObjP);
            bool dbg = false;
            if(dbg)
                {
                Utf8String namePts = LOG_PATH_STR + "mesh_tile_";
                LOGSTRING_NODE_INFO(node, namePts)
                namePts.append(".pts");
                size_t _nVertices = points.size();
                FILE* _meshFile = fopen(namePts.c_str(), "wb");
                fwrite(&_nVertices, sizeof(size_t), 1, _meshFile); 
                fwrite(&points[0], sizeof(DPoint3d), _nVertices, _meshFile); 
                fclose(_meshFile);
                }
#if 0
            WString dtmFileName(LOG_PATH_STR_W + L"meshtile_");
            LOGSTRING_NODE_INFO_W(node, dtmFileName)
                dtmFileName.append(L".tin");
            bcdtmWrite_toFileDtmObject(dtmObjP, dtmFileName.c_str());
#endif
            }
        else
            {
            std::sort(points.begin(), points.end(), [] (const DPoint3d& a, const DPoint3d&b)
                {
                if (a.x < b.x) return true;
                if (a.x > b.x) return false;
                if (a.y < b.y) return true;
                if (a.y > b.y) return false;
                if (a.z < b.z) return true;
                return false;
                });
            double currentX = points[0].x;
            double currentY = points[0].y;
            size_t nCols = 1;
            size_t nRows = 1;
            DPoint2d increment = DPoint2d::From(DBL_MAX, DBL_MAX);
            DPoint3d* lastIdx = &points[0];
            for (auto& pt : points)
                { 
                if (fabs(pt.x - currentX) > 1e-6)
                    {
                    increment.x = std::min(increment.x, pt.x - currentX);
                    currentX = pt.x;
                    ++nCols;
                   // nRows = std::max(nRows, (size_t)(&pt - lastIdx));
                    lastIdx = &pt + 1;
                    }
                else if (fabs(pt.y -currentY) > 1e-6 && pt.y - currentY > 0)
                    {
                    increment.y = std::min(increment.y, pt.y - currentY);
                    currentY = pt.y;
                    }
                //status = bcdtmObject_storeDtmFeatureInDtmObject(dtmObjP, DTMFeatureType::RandomSpots, (dtmObjP)->nullUserTag, 1, &(dtmObjP)->nullFeatureId, &pt, 1);
                }
            //nRows = std::max(nRows, (size_t)(&points.back() - lastIdx) + 1);
            nRows = 1+((ExtentOp<EXTENT>::GetYMax(node->m_nodeHeader.m_contentExtent) - ExtentOp<EXTENT>::GetYMin(node->m_nodeHeader.m_contentExtent)) / increment.y);
            if (points.size() == nRows*nCols) 
                for (auto&pt:points )
                    status = bcdtmObject_storeDtmFeatureInDtmObject(dtmObjP, DTMFeatureType::RandomSpots, (dtmObjP)->nullUserTag, 1, &(dtmObjP)->nullFeatureId, &pt, 1);

            else
                {
                vector<DPoint3d> completedPts;
                size_t currentPtInSet = 0;
                size_t currentPosX = 0, currentPosY = 0;
                    for (currentPosX = 0; currentPosX < nCols; currentPosX++)
                    {
                    for (currentPosY = 0; currentPosY < nRows; currentPosY++)
                        {
                        DPoint3d targetPt = DPoint3d::From(ExtentOp<EXTENT>::GetXMin(node->m_nodeHeader.m_contentExtent) + currentPosX*increment.x,
                                                           ExtentOp<EXTENT>::GetYMin(node->m_nodeHeader.m_contentExtent) + currentPosY*increment.y,
                                                           DBL_MIN);
                        if (currentPtInSet < points.size() && fabs(points[currentPtInSet].x - targetPt.x) < 1e-6 && fabs(points[currentPtInSet].y - targetPt.y) < 1e-6)
                            {
                            targetPt.z = points[currentPtInSet].z;
                            currentPtInSet++;
                            }
                        completedPts.push_back(targetPt);
                        }
                    }
                status = bcdtmObject_storeDtmFeatureInDtmObject(dtmObjP, DTMFeatureType::RandomSpots, (dtmObjP)->nullUserTag, 1, &(dtmObjP)->nullFeatureId, &completedPts.front(), (int)completedPts.size());
                }
            status = bcdtmLattice_createDemTinDtmObject(dtmObjP, (int)nRows, (int)nCols, DBL_MIN);
            }

      //  assert(status == SUCCESS || ((*dtmObjP).numPoints < 4));

        if (status == SUCCESS)
            {
            /* if (node->m_featureDefinitions.size() > 0)
                 {
                 WString dtmFileName(L"d:\\clipping\\tmproblems\\features\\meshtile_");
                 dtmFileName.append(std::to_wstring(node->m_nodeHeader.m_level).c_str());
                 dtmFileName.append(L"_");
                 dtmFileName.append(std::to_wstring(ExtentOp<EXTENT>::GetXMin(node->m_nodeHeader.m_nodeExtent)).c_str());
                 dtmFileName.append(L"_");
                 dtmFileName.append(std::to_wstring(ExtentOp<EXTENT>::GetYMin(node->m_nodeHeader.m_nodeExtent)).c_str());
                 dtmFileName.append(L".dtm");
                 bcdtmWrite_toFileDtmObject(dtmObjP, dtmFileName.c_str());
                 }*/
            IScalableMeshMeshPtr meshPtr;

            bcdtmInterruptLoad_triangleShadeMeshFromDtmObject(dtmPtr->GetBcDTM()->GetTinHandle(), 10000000, 2, 1, &draw, false, DTMFenceType::None, DTMFenceOption::None, 0, 0, &meshPtr);

            ScalableMeshMesh* meshP((ScalableMeshMesh*)meshPtr.get());

            if (meshP == 0)
                {
                std::cout << " ERROR: TRIANGULATION AND NO MESH AT NODE"<< node->GetBlockID().m_integerID << std::endl;
                return true;
                }

            if (meshP != 0)
                {
                pointsPtr->clear();
                RefCountedPtr<SMMemoryPoolVectorItem<int32_t>>  linearFeaturesPtr = node->GetLinearFeaturesPtr();
                vector<POINT> nodePts(meshP->GetNbPoints());
                bvector<DPoint3d> pts(meshP->GetNbPoints());
                map<DPoint3d, int32_t, DPoint3dZYXTolerancedSortComparison> pointsMap(DPoint3dZYXTolerancedSortComparison(1e-5, 0));
                for (size_t pointInd = 0; pointInd < meshP->GetNbPoints(); pointInd++)
                    {
                    nodePts[pointInd].x = meshP->GetPoints()[pointInd].x;
                    nodePts[pointInd].y = meshP->GetPoints()[pointInd].y;
                    nodePts[pointInd].z = meshP->GetPoints()[pointInd].z;
                    pts[pointInd] = meshP->GetPoints()[pointInd];
                    if (linearFeaturesPtr->size() > 0)
                        pointsMap.insert(std::make_pair(pts[pointInd], (int32_t)pointInd));
                    }
#if SM_TRACE_FEATURE_DEFS
                std::string s;
                s += "NPOINTS " + std::to_string(meshP->GetNbPoints())+"\n";
#endif
                bvector<bvector<int32_t>> defs;
                if (linearFeaturesPtr->size() > 0)node->GetFeatureDefinitions(defs, &*linearFeaturesPtr->begin(), linearFeaturesPtr->size());
                if (linearFeaturesPtr->size() > 0)
                    {
                    size_t count = 0;
                    for (size_t i = 0; i < defs.size(); ++i)
                        {
                        count += 1 + defs[i].size();
                        vector<DPoint3d> feature;
                        for (size_t j = 1; j < defs[i].size(); ++j)
                            {
#if SM_TRACE_FEATURE_DEFS
                            s += "BEFORE: " + std::to_string(node->m_featureDefinitions[i][j]) + "\n";
#endif
                            if (defs[i][j] < points.size() && pointsMap.count(points[defs[i][j]]) > 0)
                                {
                                const_cast<int32_t&>(defs[i][j]) = pointsMap[points[defs[i][j]]];
                                feature.push_back(pts[defs[i][j]]);
                                }
                            else if (defs[i][j] < points.size() && pointsMap.count(points[defs[i][j]]) == 0)
                                {
                                pts.push_back(points[defs[i][j]]);
                                nodePts.push_back(PointOp<POINT>::Create(pts.back().x, pts.back().y, pts.back().z));
                                const_cast<int32_t&>(defs[i][j]) = (int32_t)pts.size() - 1;
                                pointsMap.insert(std::make_pair(pts.back(), (int32_t)pts.size() - 1));
                                feature.push_back(pts[defs[i][j]]);
                                }
                            else
                                {
                                const_cast<int32_t&>(defs[i][j]) = INT_MAX;
                                }
#if SM_TRACE_FEATURE_DEFS
                            s += "AFTER: " + std::to_string(node->m_featureDefinitions[i][j]) + "\n";
#endif
                            }

                    {
/*                    WString namePoly = L"e:\\output\\scmesh\\2016-06-14\\postfeaturepoly_";
                    namePoly.append(std::to_wstring(node->m_nodeHeader.m_level).c_str());
                    namePoly.append(L"_");
                    namePoly.append(std::to_wstring(ExtentOp<EXTENT>::GetXMin(node->m_nodeHeader.m_nodeExtent)).c_str());
                    namePoly.append(L"_");
                    namePoly.append(std::to_wstring(ExtentOp<EXTENT>::GetYMin(node->m_nodeHeader.m_nodeExtent)).c_str());
                    namePoly.append(L"_");
                    namePoly.append(to_wstring(i).c_str());
                    namePoly.append(L".p");
                    if (feature.size() > 0)
                        {
                        FILE* polyCliPFile = _wfopen(namePoly.c_str(), L"wb");
                        size_t boundSize = feature.size();
                        fwrite(&boundSize, sizeof(size_t), 1, polyCliPFile);
                        fwrite(&feature[0], sizeof(DPoint3d), feature.size(), polyCliPFile);
                        fclose(polyCliPFile);
                        }*/
                        }
                        linearFeaturesPtr->reserve(count);
                        for (size_t i = linearFeaturesPtr->size(); i < count; ++i) linearFeaturesPtr->push_back(INT_MAX);
                        if (linearFeaturesPtr->size() > 0)node->SaveFeatureDefinitions(const_cast<int32_t*>(&*linearFeaturesPtr->begin()), count, defs);
#if SM_TRACE_FEATURE_DEFS
                        s += "\n\n----\n\n";
#endif
                        }
                    }
#if SM_TRACE_FEATURE_DEFS
                if (node->m_featureDefinitions.size() > 0)
                    {
                    std::ofstream f;
                    Utf8String namePoly = LOG_PATH_STR + "before_feature_";
                    LOGSTRING_NODE_INFO(node, namePoly)
                        namePoly.append(".txt");
                    f.open(namePoly.c_str(), ios_base::trunc);
                    f << s;
                    f.close();
                    }
#endif
                std::vector<int> faceIndexes;
                DRange3d nodeRange = DRange3d::From(ExtentOp<EXTENT>::GetXMin(node->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetYMin(node->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetZMin(node->m_nodeHeader.m_nodeExtent),
                                                    ExtentOp<EXTENT>::GetXMax(node->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetYMax(node->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetZMax(node->m_nodeHeader.m_nodeExtent));

                node->m_nodeHeader.m_nbFaceIndexes = meshP->GetNbFaceIndexes();
                ClipMeshToNodeRange<POINT, EXTENT>(faceIndexes, nodePts, pts, node->m_nodeHeader.m_contentExtent, nodeRange, meshP);

                node->m_nodeHeader.m_nbFaceIndexes = faceIndexes.size();
                pointsPtr->push_back(&nodePts[0], nodePts.size());
                bvector<int> componentPointsId;
                // SM_NEED_WORKS : textures
                if (faceIndexes.size() > 0)
                    {
                    RefCountedPtr<SMMemoryPoolGenericBlobItem<MTGGraph>> graphPtr(node->GetGraphPtr());
                    MTGGraph* newGraph = new MTGGraph();
                    CreateGraphFromIndexBuffer(newGraph, (const long*)&faceIndexes[0], (int)faceIndexes.size(), (int)nodePts.size(), componentPointsId, &pts[0]);
                    //PrintGraph(LOG_PATH_STR, std::to_string(node->GetBlockID().m_integerID).c_str(), node->GetGraphPtr());
                    RefCountedPtr<SMMemoryPoolVectorItem<int32_t>>  linearFeaturesPtr = node->GetLinearFeaturesPtr();
                    bvector<bvector<int32_t>> defs;
                    if (linearFeaturesPtr->size() > 0) node->GetFeatureDefinitions(defs, &*linearFeaturesPtr->begin(), linearFeaturesPtr->size());
                    for (size_t i = 0; i < defs.size(); ++i)
                        {
                        TagFeatureEdges(newGraph, (const DTMFeatureType)defs[i][0], defs[i].size() - 1, &defs[i][1]);
                        }
                    graphPtr->SetData(newGraph);
                    graphPtr->SetDirty();

                    }
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

                if (faceIndexes.size() > 0)
                    node->PushPtsIndices(/*meshP->GetFaceIndexes()*/&faceIndexes[0], node->m_nodeHeader.m_nbFaceIndexes);
                else
                    node->ClearPtsIndices();


                if (node->IsLeaf() && pointsPtr->size() != node->m_nodeHeader.m_totalCount)
                    {
                    node->m_nodeHeader.m_totalCount = pointsPtr->size();
                    }

#if SM_TRACE_MESH_STATS
                Utf8String nameStats = "e:\\output\\scmesh\\2015-11-18\\defects\\tileaftermeshing_";
                nameStats.append(std::to_string(node->m_nodeHeader.m_level).c_str());
                nameStats.append("_");
                nameStats.append(std::to_string(ExtentOp<EXTENT>::GetXMin(node->m_nodeHeader.m_nodeExtent)).c_str());
                nameStats.append("_");
                nameStats.append(std::to_string(ExtentOp<EXTENT>::GetYMin(node->m_nodeHeader.m_nodeExtent)).c_str());
                nameStats.append(".txt");
                std::ofstream stats;
                stats.open(nameStats.c_str(), std::ios_base::trunc);
                stats << " N OF POINTS " + std::to_string(node->size()) + "\n";
                stats << "N OF INDICES " + std::to_string(node->m_nodeHeader.m_nbFaceIndexes) + "\n";
                stats << " NODE TOTAL COUNT "+std::to_string(node->m_nodeHeader.m_totalCount)+"\n";
                stats.close();
#endif
#if 0//SM_OUTPUT_MESHES_STITCHING
                if (node->GetBlockID().m_integerID == 2702)
                    {
                    WString nameBefore = LOG_PATH_STR_W + L"postmeshmesh_";
                    LOGSTRING_NODE_INFO_W(node, nameBefore)
                        nameBefore.append(L".m");
                    auto indPtr = node->GetPtsIndicePtr();
                    LOG_MESH_FROM_FILENAME_AND_BUFFERS_W(nameBefore, pts.size(), node->m_nodeHeader.m_nbFaceIndexes, &pts[0], &(*indPtr)[0]);
                    }
#endif
                isMeshingDone = true;
                if (node->GetPointsPtr()->size() > 10 && node->GetPtsIndicePtr()->size() == 0)
                    {
                    std::cout << "NODE " << node->GetBlockID().m_integerID << " SHOULD HAVE FACES " << std::endl;
                    }

                node->SetDirty(true);
                }
            }
            else 
                std::cout << " TILE " << node->GetBlockID().m_integerID << " TRIANGULATION FAILED" << std::endl;
        }

    return isMeshingDone;
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


template<class POINT, class EXTENT> size_t ScalableMesh2DDelaunayMesher<POINT, EXTENT>::UpdateMeshNodeFromGraphs(HFCPtr<SMMeshIndexNode<POINT, EXTENT> > node, POINT** newMesh, vector<MTGGraph *>& meshGraphs, vector<std::vector<DPoint3d>>& pts, int& nFaces, DPoint3d& minPt, DPoint3d& maxPt) const
    {

    vector<int> faceIndices;
    vector<POINT> geometryData;
    bvector<DPoint3d> geomData;
    for (size_t g = 0; g < meshGraphs.size(); ++g)
        {
        int* pointsInTile = new int[pts[g].size()];
        for (size_t t = 0; t < pts[g].size(); t++) pointsInTile[t] = -1;
        MTGMask visitedMask = meshGraphs[g]->GrabMask();
        std::vector<int> indicesForFace;
        int newPtsIndex = 0;
        size_t oldSize = geometryData.size();
        MTGARRAY_SET_LOOP(edgeID, meshGraphs[g])
            {
            if (!meshGraphs[g]->GetMaskAt(edgeID, MTG_EXTERIOR_MASK) && !meshGraphs[g]->GetMaskAt(edgeID, visitedMask))
                {
                //assert(meshGraphStitched->CountNodesAroundFace(edgeID) == 3);
                int n = 0;
                int nPointsOutsideTile = 0;
                int nPointsInsideTile = 0;
                indicesForFace.clear();
                std::vector<MTGNodeId> faceNodes;
                if (meshGraphs[g]->CountNodesAroundFace(edgeID) != 3) continue;
                MTGARRAY_FACE_LOOP(faceID, meshGraphs[g], edgeID)
                    {
                    int vIndex = -1;
                    meshGraphs[g]->TryGetLabel(faceID, 0, vIndex);
                    assert(vIndex > 0);
                    assert(vIndex <= (int)pts[g].size());
                    DPoint3d pt = pts[g][vIndex - 1];
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
                        faceIndices.push_back((int)oldSize+pointsInTile[vIndex - 1] + 1);
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
                        meshGraphs[g]->TrySetLabel(faceID, 0, pointsInTile[vIndex - 1] + 1);
                        int v1 = -1;
                        meshGraphs[g]->TryGetLabel(meshGraphs[g]->FSucc(faceID), 0, v1);
                        meshGraphs[g]->TrySetLabel(meshGraphs[g]->FSucc(faceID), 0, pointsInTile[v1 - 1] + 1);
                        meshGraphs[g]->TryGetLabel(meshGraphs[g]->FPred(faceID), 0, v1);
                        meshGraphs[g]->TrySetLabel(meshGraphs[g]->FPred(faceID), 0, pointsInTile[v1 - 1] + 1);
                        }
                    //meshGraphStitched->TrySetLabel(faceID, 0, (int)geometryData.size());
                    meshGraphs[g]->SetMaskAt(faceID, visitedMask);
                    ++n;
                    }
                MTGARRAY_END_FACE_LOOP(faceID, meshGraphs[g], edgeID)
                }
            }
        MTGARRAY_END_SET_LOOP(edgeID, meshGraphs[g])
            meshGraphs[g]->ClearMask(visitedMask);
        meshGraphs[g]->DropMask(visitedMask);
        geometryData.resize(geometryData.size() + newPtsIndex);
        geomData.resize(geomData.size() + newPtsIndex);
        for (size_t j = 0; j < pts[g].size(); j++)
            {
            if (pointsInTile[j] != -1)
                {
                assert(pointsInTile[j] < newPtsIndex);
                geomData[oldSize+pointsInTile[j]] = pts[g][j];
                geometryData[oldSize+pointsInTile[j]].x = pts[g][j].x;
                geometryData[oldSize+pointsInTile[j]].y = pts[g][j].y;
                geometryData[oldSize+pointsInTile[j]].z = pts[g][j].z;
                }
            }
        pts[g].resize(newPtsIndex);
        delete[] pointsInTile;
        }
    IScalableMeshMeshPtr smPtr = IScalableMeshMesh::Create(geomData.size(), &(geomData[0]), faceIndices.size(), &faceIndices[0], 0, 0, 0, 0, 0, 0);
    ScalableMeshMesh* meshP = (ScalableMeshMesh*)smPtr.get();
    faceIndices.clear();
    DRange3d nodeRange = DRange3d::From(ExtentOp<EXTENT>::GetXMin(node->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetYMin(node->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetZMin(node->m_nodeHeader.m_nodeExtent),
                                        ExtentOp<EXTENT>::GetXMax(node->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetYMax(node->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetZMax(node->m_nodeHeader.m_nodeExtent));

    ClipMeshToNodeRange<POINT, EXTENT>(faceIndices, geometryData, geomData, node->m_nodeHeader.m_contentExtent, nodeRange, meshP);
    //ValidateTriangleMesh(faceIndices, stitchedPoints);
    for (auto& ptVec : pts) ptVec.resize(geomData.size());
#ifdef SINGLE_TILE
    if ((minPt.x > TILE_X || maxPt.x < TILE_X || minPt.y > TILE_Y || maxPt.y < TILE_Y)
        && (minPt.x > TILE_X2 || maxPt.x < TILE_X2 || minPt.y > TILE_Y2 || maxPt.y < TILE_Y2))
        {
        faceIndices.clear();
        }
#endif
    //if (NULL == node->GetGraphPtr()) node->LoadGraph();
    // *node->GetGraphPtr() = *meshGraphStitched;
   // *node->GetGraphPtr() = MTGGraph();
    RefCountedPtr<SMMemoryPoolGenericBlobItem<MTGGraph>> graphPtr(node->GetGraphPtr());
    bvector<int> componentPointsId;
    MTGGraph* newGraphP = new MTGGraph();
    CreateGraphFromIndexBuffer(newGraphP, (const long*)&faceIndices[0], (int)faceIndices.size(), (int)geomData.size(), componentPointsId, &geomData[0]);
    //node->SetGraphDirty();
    graphPtr->SetData(newGraphP);
    node->m_nodeHeader.m_nbFaceIndexes = faceIndices.size();
    assert(faceIndices.size() % 3 == 0);
    nFaces = (int)faceIndices.size();
    size_t nOfPointsUsed = geometryData.size() + (size_t)ceil(faceIndices.size()*(double)sizeof(int) / sizeof(POINT));// +(size_t)ceil((double)ct / sizeof(POINT));
    *newMesh = new POINT[nOfPointsUsed];
    memcpy(*newMesh, &geometryData[0], /*stitchedPoints.size()*/geometryData.size()*sizeof(POINT));
    //delete geometryData;
    memcpy(*newMesh + geometryData.size(), &faceIndices[0], faceIndices.size()*sizeof(int));
    return nOfPointsUsed;
    }

    template<class POINT, class EXTENT> void ScalableMesh2DDelaunayMesher<POINT, EXTENT>::SimplifyMesh(vector<int32_t>& indices, vector<POINT>& points, HFCPtr<SMMeshIndexNode<POINT, EXTENT> > node, std::string& s) const
    {
//    std::string s;
//    s += " BEFORE SIZE " + std::to_string(points.size())+"\n";
    std::map<DPoint3d, int32_t, DPoint3dYXTolerancedSortComparison> mapOfPts(DPoint3dYXTolerancedSortComparison(1e-4));
    vector<int32_t> matchedIndices(points.size(),-1);
    vector<int32_t> newIndices(points.size(), -1);
    for (auto& pt : points)
        {
        DPoint3d pt3d = DPoint3d::From(PointOp<POINT>::GetX(pt), PointOp<POINT>::GetY(pt), PointOp<POINT>::GetZ(pt));
        if (mapOfPts.count(pt3d) == 0)
            mapOfPts[pt3d] = &pt - &points[0];
        matchedIndices[&pt - &points[0]] = mapOfPts[pt3d];
        }
    vector<POINT> newSet;
    newSet.reserve(points.size());
   /* for (auto it = mapOfPts.begin(); it != mapOfPts.end(); ++it)
        {
        newSet.push_back(PointOp<POINT>::Create(it->first.x, it->first.y, it->first.z));
        newIndices[it->second] = (int)newSet.size() - 1;
        }*/
    for (size_t j = 0; j < indices.size(); j+=3)
        {
        for (size_t k = 0; k < 3; ++k)
            {
            auto& idx = indices[j + k];
            if (newIndices[matchedIndices[idx - 1]] == -1)
                {
                newSet.push_back(PointOp<POINT>::Create(points[idx - 1].x, points[idx - 1].y, points[idx - 1].z));
                newIndices[matchedIndices[idx - 1]] = (int)newSet.size() - 1;
                }
            idx = newIndices[matchedIndices[idx - 1]] + 1;
        //    s += " REPLACED " + std::to_string(oldIdx) + " WITH " + std::to_string(idx) + "\n";
            }
        if (indices[j] == indices[j + 1] || indices[j] == indices[j + 2] || indices[j + 1] == indices[j + 2])
            {
            indices.erase(indices.begin() + j, indices.begin() + j + 3);
            j -= 3;
            }
        }
    points = newSet;
    /*for (auto& feature : node->m_featureDefinitions)
        {
        for (size_t f = 1; f < feature.size(); ++f)
            {
            if (feature[f] < matchedIndices.size())
            const_cast<int32_t&>(feature[f]) = matchedIndices[feature[f]];
            else const_cast<int32_t&>(feature[f]) = INT_MAX;
            }
        }*/
//    s += " AFTER SIZE " + std::to_string(points.size());
    }

    void circumcircle(DPoint3d& center, double& radius, const DPoint3d* triangle);
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
        vector<POINT> geometryData(newPtsIndex);
    bvector<DPoint3d> geomData(newPtsIndex);
    for (size_t i = 0; i < stitchedPoints.size(); i++)
        {
        if (pointsInTile[i] != -1)
            {
            assert(pointsInTile[i] < newPtsIndex);
            geomData[pointsInTile[i]] = stitchedPoints[i];
            geometryData[pointsInTile[i]].x = stitchedPoints[i].x;
            geometryData[pointsInTile[i]].y = stitchedPoints[i].y;
            geometryData[pointsInTile[i]].z = stitchedPoints[i].z;
            }
        }
    stitchedPoints.resize(newPtsIndex);
    delete[] pointsInTile;
    IScalableMeshMeshPtr smPtr = IScalableMeshMesh::Create(newPtsIndex, &(geomData[0]), faceIndices.size(), &faceIndices[0], 0, 0, 0,0,0,0);
    ScalableMeshMesh* meshP = (ScalableMeshMesh*)smPtr.get();
    std::string str;
    if (faceIndices.size() > 0 && geometryData.size() > 0)
        {
        faceIndices.clear();
        DRange3d nodeRange = DRange3d::From(ExtentOp<EXTENT>::GetXMin(node->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetYMin(node->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetZMin(node->m_nodeHeader.m_nodeExtent),
                                            ExtentOp<EXTENT>::GetXMax(node->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetYMax(node->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetZMax(node->m_nodeHeader.m_nodeExtent));

        ClipMeshToNodeRange<POINT, EXTENT>(faceIndices, geometryData, geomData, node->m_nodeHeader.m_contentExtent, nodeRange, meshP);
        //ValidateTriangleMesh(faceIndices, stitchedPoints);
        
        SimplifyMesh(faceIndices, geometryData, node, str);
        }
    stitchedPoints.resize(geometryData.size());
#ifdef SINGLE_TILE
    if ((minPt.x > TILE_X || maxPt.x < TILE_X || minPt.y > TILE_Y || maxPt.y < TILE_Y)
        && (minPt.x > TILE_X2 || maxPt.x < TILE_X2 || minPt.y > TILE_Y2 || maxPt.y < TILE_Y2))
        {
        faceIndices.clear();
        }
#endif
   // if (NULL == node->GetGraphPtr()) node->LoadGraph(true);
   // *node->GetGraphPtr() = *meshGraphStitched;
   // *node->GetGraphPtr() = MTGGraph();
    RefCountedPtr<SMMemoryPoolGenericBlobItem<MTGGraph>> graphPtr(node->GetGraphPtr());
    MTGGraph* graphP = new MTGGraph();
    bvector<int> componentPointsId;
    if (faceIndices.size() > 0 && geometryData.size() > 0)
        CreateGraphFromIndexBuffer(graphP, (const long*)&faceIndices[0], (int)faceIndices.size(), (int)geomData.size(), componentPointsId, &geomData[0]);
    //node->SetGraphDirty();
    //node->ReleaseGraph();
    graphPtr->SetData(graphP);
    graphPtr->SetDirty();
    node->m_nodeHeader.m_nbFaceIndexes = faceIndices.size();
    assert(faceIndices.size() % 3 == 0);
    nFaces = (int)faceIndices.size();
        size_t nOfPointsUsed = geometryData.size() + (size_t)ceil(faceIndices.size()*(double)sizeof(int) / sizeof(POINT));// +(size_t)ceil((double)ct / sizeof(POINT));
        if (faceIndices.size() > 0 && geometryData.size() > 0)
            {
        *newMesh = new POINT[nOfPointsUsed];
        memcpy(*newMesh, &geometryData[0], /*stitchedPoints.size()*/geometryData.size()*sizeof(POINT));
        //delete geometryData;
        memcpy(*newMesh + geometryData.size(), &faceIndices[0], faceIndices.size()*sizeof(int));
        }
    return nOfPointsUsed;
    }

    MTGMask addedMask;
    template<class POINT, class EXTENT> void ScalableMesh2DDelaunayMesher<POINT, EXTENT>::SelectPointsToStitch(std::vector<DPoint3d>& stitchedPoints, HFCPtr<SMMeshIndexNode<POINT, EXTENT> > node, MTGGraph* meshGraph, EXTENT neighborExt, vector<int>* pointsToDestPointsMap) const
    {
    POINT* pts = nullptr;

    RefCountedPtr<SMMemoryPoolVectorItem<POINT>> pointsPtr(node->GetPointsPtr());
    DRange3d nodeExt = node->m_nodeHeader.m_nodeExtent;
    if (s_useThreadsInStitching)
        {                
        node->LockPts();
        pts = new POINT[pointsPtr->size()];
        memcpy(pts, &(*pointsPtr)[0], pointsPtr->size() * sizeof(POINT));
        node->UnlockPts();
        }

    std::queue<MTGNodeId> bounds;
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

    MTGNodeId face[3];
    MTGNodeId mate[3];
    DPoint3d facePoints[3];
    int faceIdx[3];
    MTGMask visitedMask = meshGraph->GrabMask();
    std::set<DPoint3d, compare3D> hasBeenAdded;
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
                if(vIndex <= 0)break;
                faceIdx[edgeAroundFace] = vIndex;
                
                POINT pt = s_useThreadsInStitching ? pts[vIndex - 1] : (*pointsPtr)[vIndex - 1];
                facePoints[edgeAroundFace] = DPoint3d::FromXYZ(pt.x, pt.y, pt.z);
                }
            edgeAroundFace++;
            if (edgeAroundFace > 3)break;
            }

        MTGARRAY_END_FACE_LOOP(aroundFaceIndex, meshGraph, currentID)
            if(edgeAroundFace < 3) continue;
            if (edgeAroundFace > 3)
                {
                bounds.push(meshGraph->EdgeMate(currentID));
                //bounds.pop();
                continue;
                }
            bool isColinear = false;
            if (facePoints[0].AlmostEqualXY(facePoints[1]) || facePoints[1].AlmostEqualXY(facePoints[2]) || facePoints[2].AlmostEqualXY(facePoints[0])) isColinear = true;
            else
                {
                DSegment3d triSeg = DSegment3d::From(facePoints[0], facePoints[1]);
                double param;
                DPoint3d closestPt;
                triSeg.ProjectPointXY(closestPt, param, facePoints[2]);
                if (closestPt.AlmostEqualXY(facePoints[2])) isColinear = true;
                }
            DPoint3d center = DPoint3d::From(0, 0, 0);
            double radius = 0;
            if (!isColinear)
                {
                for (size_t i = 0; i < 3; ++i) facePoints[i].DifferenceOf(facePoints[i], nodeExt.low);
                circumcircle(center, radius, facePoints);
                for (size_t i = 0; i < 3; ++i) facePoints[i].SumOf(facePoints[i], nodeExt.low);
                center.SumOf(center, nodeExt.low);
                }
            if (isColinear||DRange3d::From(center.x - radius, center.y - radius,
                center.z, center.x + radius,
                center.y + radius, center.z).IntersectsWith(neighborExt, 2) ||
            (HasOverlapWithNeighborsXY(meshGraph, currentID, s_useThreadsInStitching ? pts : &(*pointsPtr)[0])))
            {
            for (int i = 0; i < 3; i++)
                {
                meshGraph->TryGetLabel(face[i], 0, vIndex);
                assert(vIndex > 0);
                if (hasBeenAdded.count(facePoints[i]) == 0)
                    {
                    stitchedPoints.push_back(facePoints[i]);
                    hasBeenAdded.insert(facePoints[i]);
                    }
                if (nullptr != pointsToDestPointsMap) (*pointsToDestPointsMap)[vIndex - 1] = (int)stitchedPoints.size() - 1;
                if (!meshGraph->GetMaskAt(mate[i], MTG_EXTERIOR_MASK))
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
                    }
                }
            }
        else
            {
            for (int i = 0; i < 3; i++)
                {
                meshGraph->TryGetLabel(face[i], 0, vIndex);
                assert(vIndex > 0);
                if (hasBeenAdded.count(facePoints[i]) == 0)
                    {
                    stitchedPoints.push_back(facePoints[i]);
                    hasBeenAdded.insert(facePoints[i]);
                    }
                if (nullptr != pointsToDestPointsMap)  (*pointsToDestPointsMap)[vIndex - 1] = (int)stitchedPoints.size() - 1;
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
    if (s_useThreadsInStitching) delete[] pts;

    }

    
    //NEEDS_WORK_SM: Provide a specialization for this taking into account that POINT and DPoint3d are now the same
template<class POINT, class EXTENT> size_t ScalableMesh2DDelaunayMesher<POINT, EXTENT>::UpdateMeshNodeFromIndexLists(HFCPtr<SMMeshIndexNode<POINT, EXTENT> > node, POINT** newMesh, vector<vector<int32_t>>& indices, vector<std::vector<DPoint3d>>& pts, int& nFaces, DPoint3d& minPt, DPoint3d& maxPt) const
    {
    vector<int> faceIndices;
    vector<POINT> geometryData;
    bvector<DPoint3d> geomData;
  /*  size_t ptId = 7309;
    std::ofstream fn;
    Utf8String path = "E:\\output\\scmesh\\2015-12-11\\";
    path += (std::to_string((unsigned long long)node->GetParentNode().GetPtr()) + "_").c_str();
    path += (std::to_string((unsigned long long)node.GetPtr()) + "_").c_str();
    Utf8String str1 = "afterStitch_";
    fn.open((path + str1 + "_processidx.log").c_str(), std::ios_base::trunc);*/

    for (size_t g = 0; g < indices.size(); ++g)
        {
        std::vector<int> indicesForFace;
        faceIndices.reserve(faceIndices.capacity() + indices[g].size());
        // size_t oldSize = geometryData.size();
        int* pointsInTile = new int[pts[g].size()];
        for (size_t t = 0; t < pts[g].size(); t++) pointsInTile[t] = -1;
        int newPtsIndex = 0;
        size_t oldSize = geometryData.size();
        for (size_t j = 0; j < indices[g].size(); j += 3)
            {
            //assert(meshGraphStitched->CountNodesAroundFace(edgeID) == 3);
            int n = 0;
            int nPointsOutsideTile = 0;
            //int nPointsInsideTile = 0;
            indicesForFace.clear();
          //  fn << "NEW FACE DEFINITION FOR INDEX SET "+std::to_string(g) << std::endl;
            for (size_t k = 0; k < 3; ++k)
                {
                int vIndex = -1;
                vIndex = indices[g][j + k];
                assert(vIndex > 0);
                assert(vIndex <= (int)pts[g].size());
                DPoint3d pt = pts[g][vIndex - 1];
                if (pt.x < minPt.x - 1e-8 || pt.y < minPt.y - 1e-8 || pt.z < minPt.z - 1e-8 || pt.x > maxPt.x + 1e-8 || pt.y > maxPt.y + 1e-8 || pt.z > maxPt.z + 1e-8) nPointsOutsideTile++;
                // if (pt.x > minPt.x && pt.y > minPt.y && pt.z > minPt.z && pt.x < maxPt.x && pt.y < maxPt.y && pt.z < maxPt.z) nPointsInsideTile++;
                // geometryData.push_back(PointOp<POINT>::Create(stitchedPoints[vIndex - 1].x, stitchedPoints[vIndex - 1].y, stitchedPoints[vIndex - 1].z));
                if (/*nPointsOutsideTile < 3 && nPointsInsideTile < 3*/ true)
                    {
                    /*  if (pointsInTile.count(pt) == 0)
                          {
                          pointsInTile[pt] = newPtsIndex;
                          newPtsIndex++;
                          assert(pointsInTile[pt] < newPtsIndex);
                          indicesForFace.push_back(vIndex - 1);
                          }
                          faceIndices.push_back(pointsInTile[pt] + 1);*/
                    if (pointsInTile[vIndex - 1] == -1)
                        {
                        pointsInTile[vIndex - 1] = newPtsIndex;
                        newPtsIndex++;
                        assert(pointsInTile[vIndex - 1] < newPtsIndex);
                        indicesForFace.push_back(vIndex - 1);
                        }
                    faceIndices.push_back((int)oldSize + pointsInTile[vIndex - 1] + 1);
               //     fn << std::to_string((int)oldSize + pointsInTile[vIndex - 1] + 1);
                    }
                else
                    {
                    faceIndices.resize(faceIndices.size() - n); //erase last face
                    /*for (int index : indicesForFace)
                        {
                        pointsInTile.erase(pts[g][index]);
                        }
                        newPtsIndex -= (int)indicesForFace.size();*/
                    for (int index : indicesForFace)
                        {
                        pointsInTile[index] = -1;
                        }
                    newPtsIndex -= (int)indicesForFace.size();
                    indicesForFace.clear();
                    }
                ++n;
                }
      //      fn << std::endl;
            }
        geometryData.resize(geometryData.size() + newPtsIndex);
        geomData.resize(geomData.size() + newPtsIndex);
       /* for (size_t j = 0; j < pts[g].size(); j++)
            {
            if (pointsInTile.count(pts[g][j]) != 0)
                {
                assert(pointsInTile[pts[g][j]] < newPtsIndex);
                geomData[pointsInTile[pts[g][j]]] = pts[g][j];
                geometryData[pointsInTile[pts[g][j]]].x = pts[g][j].x;
                geometryData[pointsInTile[pts[g][j]]].y = pts[g][j].y;
                geometryData[pointsInTile[pts[g][j]]].z = pts[g][j].z;
                }
            }*/
        for (size_t j = 0; j < pts[g].size(); j++)
            {
            if (pointsInTile[j] != -1)
                {
                assert(pointsInTile[j] < newPtsIndex);
                geomData[oldSize + pointsInTile[j]] = pts[g][j];
                geometryData[oldSize + pointsInTile[j]].x = pts[g][j].x;
                geometryData[oldSize + pointsInTile[j]].y = pts[g][j].y;
                geometryData[oldSize + pointsInTile[j]].z = pts[g][j].z;
                }
            }
        /*if (g == 0 && node->m_featureDefinitions.size() > 0) 
            {
            for (auto& feature : node->m_featureDefinitions)
                for (size_t f = 1; f < feature.size(); ++f)
                    {
                    const_cast<int32_t&>(feature[f]) = pointsInTile[feature[f]];
                    }
            }*/
        pts[g].resize(newPtsIndex);
        delete[] pointsInTile;
        }
 //   fn.close();
    assert(geomData.size() < 100000);
    IScalableMeshMeshPtr smPtr = IScalableMeshMesh::Create(geomData.size(), &(geomData[0]), faceIndices.size(), &faceIndices[0], 0, 0, 0, 0, 0, 0);
    ScalableMeshMesh* meshP = (ScalableMeshMesh*)smPtr.get();
    std::string str;
    if (faceIndices.size() > 0 && geomData.size() > 0)
        {
    faceIndices.clear();
    DRange3d nodeRange = DRange3d::From(ExtentOp<EXTENT>::GetXMin(node->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetYMin(node->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetZMin(node->m_nodeHeader.m_nodeExtent),
                                        ExtentOp<EXTENT>::GetXMax(node->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetYMax(node->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetZMax(node->m_nodeHeader.m_nodeExtent));
        ClipMeshToNodeRange<POINT, EXTENT>(faceIndices, geometryData, geomData, node->m_nodeHeader.m_contentExtent, nodeRange, meshP);
        //ValidateTriangleMesh(faceIndices, stitchedPoints);
        SimplifyMesh(faceIndices, geometryData,node,str);
        geomData.clear();
        for (auto& pt : geometryData)
            {
            geomData.push_back(pt);
            }
        }
    if (geometryData.size() > 0)
        {
        node->m_nodeHeader.m_contentExtent = ExtentOp<EXTENT>::Create(PointOp<POINT>::GetX(geometryData[0]), PointOp<POINT>::GetY(geometryData[0]), PointOp<POINT>::GetZ(geometryData[0]),
                                                                      PointOp<POINT>::GetX(geometryData[0]), PointOp<POINT>::GetY(geometryData[0]), PointOp<POINT>::GetZ(geometryData[0]));
        for (auto& pt: geometryData)
            node->m_nodeHeader.m_contentExtent = ExtentOp<EXTENT>::MergeExtents(node->m_nodeHeader.m_contentExtent, SpatialOp<POINT, POINT, EXTENT>::GetExtent(pt));
        }
    for (auto& ptVec : pts) ptVec.resize(geometryData.size());
    assert(geomData.size() < 100000);
#ifdef SINGLE_TILE
    if ((minPt.x > TILE_X || maxPt.x < TILE_X || minPt.y > TILE_Y || maxPt.y < TILE_Y)
        && (minPt.x > TILE_X2 || maxPt.x < TILE_X2 || minPt.y > TILE_Y2 || maxPt.y < TILE_Y2))
        {
        faceIndices.clear();
        }
#endif
    // *node->GetGraphPtr() = *meshGraphStitched;
    RefCountedPtr<SMStoredMemoryPoolGenericBlobItem<MTGGraph>> storedMemoryPoolItem(new SMStoredMemoryPoolGenericBlobItem<MTGGraph>(node->GetBlockID().m_integerID, node->GetGraphStore().GetPtr(), SMPoolDataTypeDesc::Graph, (uint64_t)node->m_SMIndex));
    SMMemoryPoolItemBasePtr memPoolItemPtr(storedMemoryPoolItem.get());
    MTGGraph * tempGraph = new MTGGraph();
    bvector<int> componentPointsId;

    if (faceIndices.size() >= 3 && geomData.size() > 0)
    CreateGraphFromIndexBuffer(tempGraph, (const long*)&faceIndices[0], (int)faceIndices.size(), (int)geomData.size(), componentPointsId, &geomData[0]);
    storedMemoryPoolItem->SetData(tempGraph);
    storedMemoryPoolItem->SetDirty();
    RefCountedPtr<SMMemoryPoolGenericBlobItem<MTGGraph>> graphPtr(node->GetGraphPtr());
    SMMemoryPool::GetInstance()->ReplaceItem(memPoolItemPtr, node->m_graphPoolItemId, node->GetBlockID().m_integerID, SMPoolDataTypeDesc::Graph, (uint64_t)node->m_SMIndex);
   /* for (size_t i = 0; i < node->m_featureDefinitions.size(); ++i)
        {
        TagFeatureEdges(&tempGraph, (const DTMFeatureType)node->m_featureDefinitions[i][0], node->m_featureDefinitions[i].size() - 1, &node->m_featureDefinitions[i][1]);
        }*/

    //tempGraph.SortNodesBasedOnLabel(0);
/*    str1 += std::to_string(geomData.size()).c_str();
    PrintGraph(path, str1, &tempGraph);
    std::ofstream f;
    f.open((path + str1 + "_idx.log").c_str(), std::ios_base::trunc);
    f << str;
    f << " FACE DEFS FOR PT " + std::to_string(ptId) + "\n";
    for (size_t i = 0; i < faceIndices.size(); i += 3)
        {
        if (faceIndices[i] != (int)ptId && faceIndices[i + 1] != (int)ptId && faceIndices[i + 2] != (int)ptId) continue;
        f << "FACE DEFINITION :" + std::to_string(faceIndices[i]) + " " + std::to_string(faceIndices[i + 1]) + " " + std::to_string(faceIndices[i + 2]) << std::endl;
        std::string s;
        print_polygonarray(s, "PTS", &geomData[faceIndices[i] - 1], 1);
        print_polygonarray(s, "PTS", &geomData[faceIndices[i+1] - 1], 1);
        print_polygonarray(s, "PTS", &geomData[faceIndices[i+2] - 1], 1);
        f << "FACE PTS :" + s << std::endl;
        }
    f.close();*/
    //if (NULL == node->GetGraphPtr()) node->LoadGraph(s_useThreadsInStitching);
    //if (s_useThreadsInStitching) node->LockGraph();
   // *node->GetGraphPtr() = tempGraph;
        {
      /*  Utf8String path = "E:\\output\\scmesh\\2016-01-28\\";
        Utf8String str1 = "afterStitch_graph_";
        str1 += std::to_string(node->GetBlockID().m_integerID).c_str();
            {si
            void* graphData;
            size_t ct = tempGraph.WriteToBinaryStream(graphData);
            FILE* graphSaved = fopen((path + str1).c_str(), "wb");
            fwrite(&ct, sizeof(size_t), 1, graphSaved);
            fwrite(graphData, 1, ct, graphSaved);
            size_t npts = geomData.size();
            fwrite(&npts, sizeof(size_t), 1, graphSaved);
            fwrite(&geomData[0], sizeof(DPoint3d), npts, graphSaved);
            delete[] graphData;
            fclose(graphSaved);
            }*/
        }

    //node->SetGraphDirty();
    //if (s_useThreadsInStitching) node->UnlockGraph();
    //node->ReleaseGraph();
    node->m_nodeHeader.m_nbFaceIndexes = faceIndices.size();
    assert(faceIndices.size() % 3 == 0);
    nFaces = (int)faceIndices.size();
    if (faceIndices.size() == 0 || geometryData.size() == 0) return 0;
    size_t nOfPointsUsed = geometryData.size() + (size_t)ceil(faceIndices.size()*(double)sizeof(int) / sizeof(POINT));// +(size_t)ceil((double)ct / sizeof(POINT));
    *newMesh = new POINT[nOfPointsUsed];
    memcpy(*newMesh, &geometryData[0], /*stitchedPoints.size()*/geometryData.size()*sizeof(POINT));
    //delete geometryData;
    memcpy(*newMesh + geometryData.size(), &faceIndices[0], faceIndices.size()*sizeof(int));
    return nOfPointsUsed;
    }


template<class POINT, class EXTENT> void ScalableMesh2DDelaunayMesher<POINT, EXTENT>::SelectPointsBasedOnBox(std::vector<DPoint3d>& stitchedPoints, HFCPtr<SMMeshIndexNode<POINT, EXTENT> > node, EXTENT neighborExt) const
    {
    POINT* pts = nullptr;
    size_t nOfPts = 0;

    RefCountedPtr<SMMemoryPoolVectorItem<POINT>> pointsPtr(node->GetPointsPtr());

    if (s_useThreadsInStitching)
        {    
        node->LockPts();
        pts = new POINT[pointsPtr->size()];
        memcpy(pts, &(*pointsPtr)[0], sizeof(POINT) * pointsPtr->size());                
        nOfPts = pointsPtr->size();        
        node->UnlockPts();
        }

    DRange3d box = DRange3d::From(ExtentOp<EXTENT>::GetXMin(neighborExt), ExtentOp<EXTENT>::GetYMin(neighborExt), ExtentOp<EXTENT>::GetZMin(neighborExt),
                                  +ExtentOp<EXTENT>::GetXMax(neighborExt), ExtentOp<EXTENT>::GetYMax(neighborExt), ExtentOp<EXTENT>::GetZMax(neighborExt));
    DPlane3d planes[6];
    DPoint3d origins[6];
    DVec3d normals[6];
    box.Get6Planes(origins, normals);
    for (size_t i = 0; i < 6; ++i) planes[i] = DPlane3d::FromOriginAndNormal(origins[i], normals[i]);
    for (size_t i = 0; i < (s_useThreadsInStitching? nOfPts : pointsPtr->size()); ++i)
        {
        POINT pt = s_useThreadsInStitching? pts[i] : pointsPtr->operator[](i);
        DPoint3d pt3d = DPoint3d::From(PointOp<POINT>::GetX(pt), PointOp<POINT>::GetY(pt), PointOp<POINT>::GetZ(pt));
        for (size_t j = 0; j < 6; ++j)
            {
            if (fabs(planes[j].Evaluate(pt3d)) < 1e-6)
                {
                stitchedPoints.push_back(pt3d);
                break;
                }
            }
        }
    if (s_useThreadsInStitching) delete[] pts;
    }

    struct compare3D;

    void ProcessFeatureDefinitions(bvector<bvector<DPoint3d>>& voidFeatures, bvector<DTMFeatureType>& types, bvector<bvector<DPoint3d>>& islandFeatures, const std::vector<DPoint3d>& nodePoints, BC_DTM_OBJ* dtmObjP, bvector<bvector<int32_t>>& featureDefs);
    int AddPolygonsToDTMObject(bvector<bvector<DPoint3d>>& polygons, DTMFeatureType type, BC_DTM_OBJ* dtmObjP);
    int AddIslandsToDTMObject(bvector<bvector<DPoint3d>>& islandFeatures, bvector<bvector<DPoint3d>>& voidFeatures, BC_DTM_OBJ* dtmObjP);

template<class POINT, class EXTENT> bool ScalableMesh2DDelaunayMesher<POINT, EXTENT>::Stitch(HFCPtr<SMMeshIndexNode<POINT, EXTENT> > node) const
    {
    //return true;
    LOG_SET_PATH("E:\\output\\scmesh\\2016-06-14\\")
    LOG_SET_PATH_W("E:\\output\\scmesh\\2016-06-14\\")
    //LOGSTRING_NODE_INFO(node, LOG_PATH_STR)
    //LOGSTRING_NODE_INFO_W(node, LOG_PATH_STR_W)

    if (node->m_nodeHeader.m_nbFaceIndexes == 0) return true;
    //bool hasPtsToTrack = false;
   /* DPoint3d pts[3] = { DPoint3d::From(431508.19, 4500522, 0),
        DPoint3d::From(434180.54, 4506349.5, 0) ,
        DPoint3d::From(433999, 4506298.02, 0) };

    for (size_t i = 0; i < 3; ++i)
        if (node->m_nodeHeader.m_nodeExtent.IsContained(pts[i], 2)) hasPtsToTrack = true;*/
  //  if (NULL == node->GetGraphPtr()) node->LoadGraph(s_useThreadsInStitching);
  //  MTGGraph* meshGraphP = node->GetGraphPtr();
  //  if (NULL == meshGraphP) return true;
    RefCountedPtr<SMMemoryPoolGenericBlobItem<MTGGraph>> graphPtr(node->GetGraphPtr());
    MTGGraph meshGraph;
        {
        if (s_useThreadsInStitching) node->LockGraph();
        meshGraph = *(graphPtr->GetData());
        if (s_useThreadsInStitching) node->UnlockGraph();
        }

    //node->ReleaseGraph();
    
    size_t neighborIndices[26] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25 };
    size_t nodeIndicesInNeighbor[26] = { 7, 6, 5, 4, 3, 2, 1, 0, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8 };
    vector<DPoint3d> stitchedPoints;

    RefCountedPtr<SMMemoryPoolVectorItem<POINT>> pointsPtr(node->GetPointsPtr());

    if (s_useThreadsInStitching) node->LockPts();

    vector<int> pointsToDestPointsMap(pointsPtr->size());
    std::fill_n(pointsToDestPointsMap.begin(), pointsToDestPointsMap.size(), -1);
    
    std::vector<DPoint3d> nodePoints(pointsPtr->size());


#if SM_TRACE_MESH_STATS
    Utf8String fileName = (LOG_PATH_STR);
    LOGSTRING_NODE_INFO(node, fileName)
    fileName.append("_stats.txt");
    LOG_STATS_FOR_NODE(fileName, node)
#endif
std::map<DPoint3d, int, compare3D> stitchedSet;

for (size_t i = 0; i < pointsPtr->size(); i++)
    {
    nodePoints[i].x = (*pointsPtr)[i].x;
    nodePoints[i].y = (*pointsPtr)[i].y;
    nodePoints[i].z = (*pointsPtr)[i].z;
    }

    if (s_useThreadsInStitching) node->UnlockPts();

#if SM_OUTPUT_MESHES_STITCHING
    if (node->m_nodeHeader.m_level <= 6)
        {
        WString nameBefore = LOG_PATH_STR_W + L"pl_stitchmesh_";
        LOGSTRING_NODE_INFO_W(node, nameBefore)
            nameBefore.append(L".m");
        auto indPtr = node->GetPtsIndicePtr();
        LOG_MESH_FROM_FILENAME_AND_BUFFERS_W(nameBefore, nodePoints.size(), node->m_nodeHeader.m_nbFaceIndexes, (&nodePoints[0]), &(*indPtr)[0]);
        }
#endif
bvector<bvector<DPoint3d>> boundary;
EXTENT ext = node->GetContentExtent();
bvector<bvector<DPoint3d>> stitchedNeighborsBoundary;
std::vector<EXTENT> stitchedNeighborsExtents;

addedMask = meshGraph.GrabMask();
//std::string s;
for (size_t& neighborInd : neighborIndices)
    {
    size_t idx = &neighborInd - &neighborIndices[0];
    if ((node->m_apNeighborNodes[neighborInd].size() > 0))
        {
        for (size_t neighborSubInd = 0; neighborSubInd < node->m_apNeighborNodes[neighborInd].size(); neighborSubInd++)
            {
            MTGGraph meshGraphNeighbor;
            //if (node->m_nodeHeader.m_apAreNeighborNodesStitched[neighborInd] == false)
                {
                HFCPtr < SMMeshIndexNode<POINT, EXTENT>> meshNode = dynamic_pcast<SMMeshIndexNode<POINT, EXTENT>, SMPointIndexNode<POINT, EXTENT>>(node->m_apNeighborNodes[neighborInd][neighborSubInd]);
                if (meshNode->m_nodeHeader.m_nbFaceIndexes == 0) continue;
                meshGraphNeighbor = MTGGraph();
                if (node->m_nodeHeader.m_apAreNeighborNodesStitched[neighborInd] == false)
                    {
                   // if (NULL == meshNode->GetGraphPtr()) meshNode->LoadGraph(s_useThreadsInStitching);
                    //if (NULL == meshNode->GetGraphPtr()) continue;
                   // if (s_useThreadsInStitching)    meshNode->LockGraph();
                    RefCountedPtr<SMMemoryPoolGenericBlobItem<MTGGraph>> graphNeighborPtr(meshNode->GetGraphPtr());
                        {
                        if (s_useThreadsInStitching)    meshNode->LockGraph();
                        if (graphNeighborPtr->GetData() != nullptr) meshGraphNeighbor = *(graphNeighborPtr->GetData());
                        if (s_useThreadsInStitching)    meshNode->UnlockGraph();
                        }
                   // if (s_useThreadsInStitching)  meshNode->UnlockGraph();
                    //meshNode->ReleaseGraph();
                    }
               /* s += " CURRENT N OF POINTS TO STITCH " + std::to_string(stitchedPoints.size()) + "\n";
                s += " ADDING NEIGHBOR AT POS " + std::to_string(neighborInd) + " IDX " + std::to_string(neighborSubInd) + "\n";
                s += "NEIGHBOR NODE SIZE "+std::to_string(node->m_apNeighborNodes[neighborInd][neighborSubInd]->size())+" THIS NODE'S SIZE "+std::to_string(node->size())+
                   ( node->m_nodeHeader.m_apAreNeighborNodesStitched[neighborInd] ? " NODE STITCHED \n" : " NOT STITCHED \n");*/
                
                    SelectPointsToStitch(stitchedPoints, node, &meshGraph, node->m_apNeighborNodes[neighborInd][neighborSubInd]->GetContentExtent(), nullptr);
                                      
                   // s += " CURRENT N OF POINTS TO STITCH " + std::to_string(stitchedPoints.size()) + "\n";
                    if (node->m_apNeighborNodes[neighborInd][neighborSubInd]->m_nodeHeader.m_apAreNeighborNodesStitched[nodeIndicesInNeighbor[idx]] == false)
                        SelectPointsToStitch(stitchedPoints, static_cast<SMMeshIndexNode<POINT, EXTENT>*>(&*node->m_apNeighborNodes[neighborInd][neighborSubInd]), &meshGraphNeighbor, node->GetContentExtent(), nullptr);
                    else
                         SelectPointsBasedOnBox(stitchedPoints, static_cast<SMMeshIndexNode<POINT, EXTENT>*>(&*node->m_apNeighborNodes[neighborInd][neighborSubInd]), node->GetNodeExtent());
                   // SelectPointsToStitch(stitchedPoints, static_cast<SMMeshIndexNode<POINT, EXTENT>*>(&*node->m_apNeighborNodes[neighborInd][neighborSubInd]), &meshGraphNeighbor, node->GetContentExtent(), nullptr);
                   // s += " CURRENT N OF POINTS TO STITCH " + std::to_string(stitchedPoints.size()) + "\n";
                    if (node->m_apNeighborNodes[neighborInd][neighborSubInd]->m_nodeHeader.m_apAreNeighborNodesStitched[nodeIndicesInNeighbor[idx]] == false)
                        {
                        bvector<bvector<DPoint3d>> b;                        
                        RefCountedPtr<SMMemoryPoolVectorItem<POINT>> neigborNodePointsPtr(node->m_apNeighborNodes[neighborInd][neighborSubInd]->GetPointsPtr());

                        POINT* pts = new POINT[neigborNodePointsPtr->size()];                        
                        neigborNodePointsPtr->get(pts, neigborNodePointsPtr->size());
                        vector<DPoint3d> nPts(neigborNodePointsPtr->size());                        
                        for (int i = 0; i < nPts.size(); i++)
                            {
                            nPts[i].x = pts[i].x;
                            nPts[i].y = pts[i].y;
                            nPts[i].z = pts[i].z;
                            }
                        GetGraphExteriorBoundary(&meshGraphNeighbor, b, &nPts[0]);
                        nPts.clear();
                        if (node->m_nodeHeader.m_apAreNeighborNodesStitched[neighborInd] == false)
                            {
                            for (auto& bound : b)
                                {
                                std::set<DPoint3d, DPoint3dZYXTolerancedSortComparison> allPts(DPoint3dZYXTolerancedSortComparison(1e-6,0));
                                for (auto& pt : bound)
                                    {
                                    allPts.insert(pt);
                                    if (allPts.size() >= 4)
                                        {
                                        stitchedNeighborsBoundary.push_back(bound);
                                        break;
                                        }
                                    }
                                }
                            }
                        //delete meshGraphNeighbor;
                        delete[] pts;
                        }
                    
                    //node->m_apNeighborNodes[neighborInd][neighborSubInd]->m_nodeHeader.m_apAreNeighborNodesStitched[nodeIndicesInNeighbor[idx]] = true;
                    
                }          
            //node->m_nodeHeader.m_apAreNeighborNodesStitched[neighborInd] = true;
            }

        }
    node->m_nodeHeader.m_apAreNeighborNodesStitched[neighborInd] = true;
    }
    GetGraphExteriorBoundary(&meshGraph, boundary, &nodePoints[0], true);
    //mesh aggregated points
    MTGGraph meshGraphStitched;
    IScalableMeshMeshPtr meshPtr;
    POINT* newNodePointData = nullptr;
    int nfaces = 0;
    size_t arraySize;
    bvector<int> componentsPointsId;
    DPoint3d extentMin, extentMax;

    extentMin = DPoint3d::FromXYZ(ExtentOp<EXTENT>::GetXMin(node->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetYMin(node->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetZMin(node->m_nodeHeader.m_nodeExtent));
    extentMax = DPoint3d::FromXYZ(ExtentOp<EXTENT>::GetXMax(node->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetYMax(node->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetZMax(node->m_nodeHeader.m_nodeExtent));

if (stitchedPoints.size() != 0)// return false; //nothing to stitch here
    {
    BENTLEY_NAMESPACE_NAME::TerrainModel::DTMPtr dtmPtr;
    int status = CreateBcDTM(dtmPtr);
    assert(status == SUCCESS);
    BC_DTM_OBJ* dtmObjP(dtmPtr->GetBcDTM()->GetTinHandle());
    status = bcdtmObject_storeDtmFeatureInDtmObject(dtmObjP, DTMFeatureType::RandomSpots, dtmObjP->nullUserTag, 1, &dtmObjP->nullFeatureId, &stitchedPoints[0], (int)stitchedPoints.size());
    assert(status == SUCCESS);

    /*DPoint3d box[8];
    totalExtent.Get8Corners(box);
    status = bcdtmObject_storeDtmFeatureInDtmObject(dtmObjP, DTMFeatureType::Hull, dtmObjP->nullUserTag, 1, &dtmObjP->nullFeatureId, (DPoint3d*)&(box[0]), (long)8);
    assert(status == SUCCESS);*/
    //status = bcdtmObject_storeDtmFeatureInDtmObject(dtmObjP, DTMFeatureType::RandomSpots, dtmObjP->nullUserTag, 1, &dtmObjP->nullFeatureId, &boundary[0], (int)boundary.size());
    /*  WString dtmFileName(L"d:\\stitching\\stitchproblems\\meshtile_");
      dtmFileName.append(std::to_wstring(node->m_nodeHeader.m_level).c_str());
      dtmFileName.append(L"_");
      dtmFileName.append(std::to_wstring(ExtentOp<EXTENT>::GetXMin(node->m_nodeHeader.m_nodeExtent)).c_str());
      dtmFileName.append(L"_");
      dtmFileName.append(std::to_wstring(ExtentOp<EXTENT>::GetYMin(node->m_nodeHeader.m_nodeExtent)).c_str());
      dtmFileName.append(L".tin");
      bcdtmWrite_toFileDtmObject(dtmObjP, dtmFileName.c_str()); */

    bvector<bvector<DPoint3d>> voidFeatures;
    bvector<bvector<DPoint3d>> islandFeatures;
    bvector<DTMFeatureType> types;
    VuPolygonClassifier vu(1e-6,0);
    RefCountedPtr<SMMemoryPoolVectorItem<int32_t>>  linearFeaturesPtr = node->GetLinearFeaturesPtr();
    bvector<bvector<int32_t>> defs;
    if (linearFeaturesPtr->size() > 0) node->GetFeatureDefinitions(defs, &*linearFeaturesPtr->begin(), linearFeaturesPtr->size());
    ProcessFeatureDefinitions(voidFeatures, types, islandFeatures, nodePoints, dtmObjP, defs);



    bvector<bvector<DPoint3d>> postFeatureBoundary;
    bvector<bool> skipFeatures(voidFeatures.size(), false);
  
    for (auto& bound : boundary)
        if (bound.size() > 2)
            {
            if (!bsiDPoint3d_pointEqualTolerance(&bound.front(), &bound.back(), 1e-8)) bound.push_back(bound.front());
#if SM_OUTPUT_MESHES_STITCHING
            WString namePoly = LOG_PATH_STR_W+L"poly_";
            namePoly.append(std::to_wstring(node->GetBlockID().m_integerID).c_str());
            namePoly.append(L"_");
            namePoly.append(std::to_wstring(node->m_nodeHeader.m_level).c_str());
            namePoly.append(L"_");
            namePoly.append(std::to_wstring(ExtentOp<EXTENT>::GetXMin(node->m_nodeHeader.m_nodeExtent)).c_str());
            namePoly.append(L"_");
            namePoly.append(std::to_wstring(ExtentOp<EXTENT>::GetYMin(node->m_nodeHeader.m_nodeExtent)).c_str());
            namePoly.append(L"_");
            namePoly.append(to_wstring(&bound - &boundary[0]).c_str());
            namePoly.append(L".p");
            FILE* polyCliPFile = _wfopen(namePoly.c_str(), L"wb");
            size_t boundSize = bound.size();
            fwrite(&boundSize, sizeof(size_t), 1, polyCliPFile);
            fwrite(&bound[0], sizeof(DPoint3d), bound.size(), polyCliPFile);
            fclose(polyCliPFile);
#endif
            std::set<DPoint3d, DPoint3dZYXTolerancedSortComparison> allPts(DPoint3dZYXTolerancedSortComparison(1e-6, 0));
            bool addedBound = false;
            for (auto& pt : bound)
                {
                allPts.insert(pt);
                if (allPts.size() >= 4)
                    {
                    bvector<DPoint3d> intersectBound;
                    for (auto& pt : bound)
                        intersectBound.push_back(DPoint3d::From(pt.x, pt.y, 0));
                    if (voidFeatures.size() > 0) //if there are any features making holes in the original mesh, they must be merged with the outline of the stitched mesh
                        {
                        for (auto& feature : voidFeatures)
                            {
                            bvector<DPoint3d> voidFeature;
                            for (auto& pt : feature)
                                voidFeature.push_back(DPoint3d::From(pt.x, pt.y, 0));
                            if (bsiDPoint3dArray_polygonClashXYZ(&intersectBound.front(), (int)intersectBound.size(), &voidFeature.front(), (int)voidFeature.size()))
                                {
                                skipFeatures[&feature - &voidFeatures[0]] = true;
                                vu.ClassifyAUnionB(intersectBound, voidFeature);
                                bvector<DPoint3d> xyz;
                                for (; vu.GetFace(xyz);)
                                    {
                                    if (bsiGeom_getXYPolygonArea(&xyz[0], (int)xyz.size()) < 0) continue;
                                    else
                                        {
                                        //  postFeatureBoundary.push_back(xyz);
                                        intersectBound = xyz;

                                        }

                                    }

                                }
                            }
#if SM_OUTPUT_MESHES_STITCHING
                        WString namePoly = LOG_PATH_STR_W+L"postF_";
                        namePoly.append(std::to_wstring(node->GetBlockID().m_integerID).c_str());
                        namePoly.append(L"_");
                        namePoly.append(std::to_wstring(node->m_nodeHeader.m_level).c_str());
                        namePoly.append(L"_");
                        namePoly.append(std::to_wstring(ExtentOp<EXTENT>::GetXMin(node->m_nodeHeader.m_nodeExtent)).c_str());
                        namePoly.append(L"_");
                        namePoly.append(std::to_wstring(ExtentOp<EXTENT>::GetYMin(node->m_nodeHeader.m_nodeExtent)).c_str());
                        namePoly.append(L"_");
                        namePoly.append(to_wstring(&bound - &boundary[0]).c_str());
                        namePoly.append(L".p");
                        FILE* polyCliPFile = _wfopen(namePoly.c_str(), L"wb");
                        size_t boundSize = intersectBound.size();
                        fwrite(&boundSize, sizeof(size_t), 1, polyCliPFile);
                        fwrite(&intersectBound[0], sizeof(DPoint3d), intersectBound.size(), polyCliPFile);
                        fclose(polyCliPFile);
#endif
                        postFeatureBoundary.push_back(intersectBound);
                        break;
                        }
                     else
                            {
                            VuPolygonClassifier vu2(1e-8,0);
                            bool addedFeatures = false;
                            bvector<DPoint3d> current = bound;
                            for (auto& feature : boundary)
                                {
                                if (&feature == &bound) continue;
                                if (feature.size() <= 3) continue;
                                addedBound = true;
                                    {
                                    /*WString namePoly = L"E:\\output\\scmesh\\2016-01-14\\poly_feature_";
                                    namePoly.append(std::to_wstring(&feature - &boundary[0]).c_str());
                                    namePoly.append(L"_");
                                    namePoly.append(std::to_wstring(node->m_nodeHeader.m_level).c_str());
                                    namePoly.append(L"_");
                                    namePoly.append(std::to_wstring(ExtentOp<EXTENT>::GetXMin(node->m_nodeHeader.m_nodeExtent)).c_str());
                                    namePoly.append(L"_");
                                    namePoly.append(std::to_wstring(ExtentOp<EXTENT>::GetYMin(node->m_nodeHeader.m_nodeExtent)).c_str());
                                    //namePoly.append(L"_");
                                    //namePoly.append(to_wstring(n).c_str());
                                    namePoly.append(L".p");
                                    FILE* polyCliPFile = _wfopen(namePoly.c_str(), L"wb");
                                    size_t boundSize = feature.size();
                                    fwrite(&boundSize, sizeof(size_t), 1, polyCliPFile);
                                    fwrite(&feature[0], sizeof(DPoint3d), feature.size(), polyCliPFile);
                                    fclose(polyCliPFile);*/
                                    }
                                vu2.ClassifyAUnionB(current, feature);
                                addedFeatures = true;
                                bvector<DPoint3d> xyz;
                                size_t n2 = 0;
                                for (; vu2.GetFace(xyz);)
                                    {
                                        {
                                       /* WString namePoly = L"E:\\output\\scmesh\\2016-01-28\\poly_feature_";
                                        namePoly.append(std::to_wstring(&feature - &boundary[0]).c_str());
                                        namePoly.append(L"_union_face_");
                                        namePoly.append(std::to_wstring(n2).c_str());
                                        namePoly.append(L"_");
                                        namePoly.append(std::to_wstring(bsiGeom_getXYPolygonArea(&xyz[0], (int)xyz.size())).c_str());
                                        namePoly.append(L"_");
                                        namePoly.append(std::to_wstring(node->m_nodeHeader.m_level).c_str());
                                        namePoly.append(L"_");
                                        namePoly.append(std::to_wstring(ExtentOp<EXTENT>::GetXMin(node->m_nodeHeader.m_nodeExtent)).c_str());
                                        namePoly.append(L"_");
                                        namePoly.append(std::to_wstring(ExtentOp<EXTENT>::GetYMin(node->m_nodeHeader.m_nodeExtent)).c_str());
                                        //namePoly.append(L"_");
                                        //namePoly.append(to_wstring(n).c_str());
                                        namePoly.append(L".p");
                                        FILE* polyCliPFile = _wfopen(namePoly.c_str(), L"wb");
                                        size_t boundSize = xyz.size();
                                        fwrite(&boundSize, sizeof(size_t), 1, polyCliPFile);
                                        fwrite(&xyz[0], sizeof(DPoint3d), xyz.size(), polyCliPFile);
                                        fclose(polyCliPFile);*/
                                        }
                                    if (bsiGeom_getXYPolygonArea(&xyz[0], (int)xyz.size()) < 0) continue;
                                    else
                                        {
                                        current = xyz;
                        /*                }
                                    ++n2;
                                    }
                                }*/
                            // bvector<DPoint3d> xyz;
                            //   size_t n = 0;
                            /* for (; vu2.GetFace(xyz);)
                                    {
                                    if (bsiGeom_getXYPolygonArea(&xyz[0], (int)xyz.size()) < 0) continue;
                                    else
                                    {*/
                            if (!bsiDPoint3d_pointEqualTolerance(&current.front(), &current.back(), 1e-8)) current.push_back(current.front());
                               /* {
                                WString namePoly = L"E:\\output\\scmesh\\2016-01-14\\poly_before_untie_";
                                namePoly.append(std::to_wstring(node->m_nodeHeader.m_level).c_str());
                                namePoly.append(L"_");
                                namePoly.append(std::to_wstring(ExtentOp<EXTENT>::GetXMin(node->m_nodeHeader.m_nodeExtent)).c_str());
                                namePoly.append(L"_");
                                namePoly.append(std::to_wstring(ExtentOp<EXTENT>::GetYMin(node->m_nodeHeader.m_nodeExtent)).c_str());
                                //namePoly.append(L"_");
                                //namePoly.append(to_wstring(n).c_str());
                                namePoly.append(L".p");
                                FILE* polyCliPFile = _wfopen(namePoly.c_str(), L"wb");
                                size_t boundSize = current.size();
                                fwrite(&boundSize, sizeof(size_t), 1, polyCliPFile);
                                fwrite(&current[0], sizeof(DPoint3d), current.size(), polyCliPFile);
                                fclose(polyCliPFile);
                                }*/
                            UntieLoopsFromPolygon(current);
                                {
                               /* WString namePoly = LOG_PATH_STR_W+ +L"poly2_";
                                namePoly.append(std::to_wstring(node->GetBlockID().m_integerID).c_str());
                                namePoly.append(L"_");
                                namePoly.append(std::to_wstring(node->m_nodeHeader.m_level).c_str());
                                namePoly.append(L"_");
                                namePoly.append(std::to_wstring(ExtentOp<EXTENT>::GetXMin(node->m_nodeHeader.m_nodeExtent)).c_str());
                                namePoly.append(L"_");
                                namePoly.append(std::to_wstring(ExtentOp<EXTENT>::GetYMin(node->m_nodeHeader.m_nodeExtent)).c_str());
                                namePoly.append(L"_");
                                namePoly.append(to_wstring(n2).c_str());
                                namePoly.append(L".p");
                                FILE* polyCliPFile = _wfopen(namePoly.c_str(), L"wb");
                                size_t boundSize = current.size();
                                fwrite(&boundSize, sizeof(size_t), 1, polyCliPFile);
                                fwrite(&current[0], sizeof(DPoint3d), current.size(), polyCliPFile);
                                fclose(polyCliPFile);*/
                                }
                            status = bcdtmObject_storeDtmFeatureInDtmObject(dtmObjP, DTMFeatureType::GraphicBreak, dtmObjP->nullUserTag, 1, &dtmObjP->nullFeatureId, (DPoint3d*)&(current[0]), (long)current.size());
                            assert(status == SUCCESS);
                            status = bcdtmObject_storeDtmFeatureInDtmObject(dtmObjP, DTMFeatureType::Void, dtmObjP->nullUserTag, 1, &dtmObjP->nullFeatureId, (DPoint3d*)&(current[0]), (long)current.size());
                            assert(status == SUCCESS);
                                        }
                                    ++n2;
                                    }
                                }
                            // ++n;
                            // }

                            // }
                            if (!addedFeatures)
                                {
                                status = bcdtmObject_storeDtmFeatureInDtmObject(dtmObjP, DTMFeatureType::GraphicBreak, dtmObjP->nullUserTag, 1, &dtmObjP->nullFeatureId, (DPoint3d*)&(bound[0]), (long)bound.size());
                                assert(status == SUCCESS);
                                status = bcdtmObject_storeDtmFeatureInDtmObject(dtmObjP, DTMFeatureType::Void, dtmObjP->nullUserTag, 1, &dtmObjP->nullFeatureId, (DPoint3d*)&(bound[0]), (long)bound.size());
                                assert(status == SUCCESS);
                                }
                            break;
                                }
                    }

                }
            if (addedBound) break;

            }

    //cut islands such that they are either strictly inside or strictly out of any holes
            status = AddIslandsToDTMObject(islandFeatures, voidFeatures, dtmObjP);
            assert(status == SUCCESS);

    //if (boundary.size() == 0)
        {
        for (auto& feature : voidFeatures)
            {
#if SM_OUTPUT_MESHES_STITCHING
            WString namePoly = LOG_PATH_STR_W+L"featurepoly_";
            namePoly.append(std::to_wstring(node->m_nodeHeader.m_level).c_str());
            namePoly.append(L"_");
            namePoly.append(std::to_wstring(node->GetBlockID().m_integerID).c_str());
            namePoly.append(L"_");
            namePoly.append(std::to_wstring(ExtentOp<EXTENT>::GetXMin(node->m_nodeHeader.m_nodeExtent)).c_str());
            namePoly.append(L"_");
            namePoly.append(std::to_wstring(ExtentOp<EXTENT>::GetYMin(node->m_nodeHeader.m_nodeExtent)).c_str());
            namePoly.append(L"_");
            namePoly.append(to_wstring(&feature - &voidFeatures[0]).c_str());
            namePoly.append(L".p");
            FILE* polyCliPFile = _wfopen(namePoly.c_str(), L"wb");
            size_t boundSize = feature.size();
            fwrite(&boundSize, sizeof(size_t), 1, polyCliPFile);
            fwrite(&feature[0], sizeof(DPoint3d), feature.size(), polyCliPFile);
            fclose(polyCliPFile);
#endif
            if (skipFeatures[&feature - &voidFeatures[0]]) continue;
            status = bcdtmObject_storeDtmFeatureInDtmObject(dtmObjP, types[&feature- &voidFeatures[0]], dtmObjP->nullUserTag, 1, &dtmObjP->nullFeatureId, (DPoint3d*)&(feature[0]), (long)feature.size());
            assert(status == SUCCESS);
            }
        }
        MergePolygonSets(postFeatureBoundary);

    status = AddPolygonsToDTMObject(postFeatureBoundary, DTMFeatureType::DrapeVoid, dtmObjP);

    status = AddPolygonsToDTMObject(stitchedNeighborsBoundary, DTMFeatureType::Breakline, dtmObjP);

#if SM_OUTPUT_MESHES_STITCHING
    //if (hasPtsToTrack)
        {
        WString dtmFileName(LOG_PATH_STR_W);
        dtmFileName.append(std::to_wstring(node->GetBlockID().m_integerID).c_str());
        dtmFileName.append(L"_");
        dtmFileName.append(std::to_wstring(node->m_nodeHeader.m_level).c_str());
        dtmFileName.append(L"_");
        dtmFileName.append(std::to_wstring(ExtentOp<EXTENT>::GetXMin(node->m_nodeHeader.m_nodeExtent)).c_str());
        dtmFileName.append(L"_");
        dtmFileName.append(std::to_wstring(ExtentOp<EXTENT>::GetYMin(node->m_nodeHeader.m_nodeExtent)).c_str());
        dtmFileName.append(L".tin");
        bcdtmWrite_toFileDtmObject(dtmObjP, dtmFileName.c_str());
        }
#endif

    status = bcdtmObject_triangulateDtmObject(dtmObjP);
   // assert(status == SUCCESS || ((*dtmObjP).numPoints < 4));
    if ((*dtmObjP).numPoints < 4) return false;
    if (status != SUCCESS) return false;
    if (status == SUCCESS)
        {
        IScalableMeshMeshPtr meshPtr;

        bcdtmInterruptLoad_triangleShadeMeshFromDtmObject(dtmPtr->GetBcDTM()->GetTinHandle(), 10000000, 2, 1, &draw, false, DTMFenceType::None, DTMFenceOption::None, 0, 0, &meshPtr);

        ScalableMeshMesh* meshP((ScalableMeshMesh*)meshPtr.get());
        if (meshP == 0) return false;
        assert(meshP != 0);

#if SM_OUTPUT_MESHES_STITCHING
        if (node->m_nodeHeader.m_level <= 6)
        {
        WString nameStitched = LOG_PATH_STR_W + L"posttrimesh_new_";
        LOGSTRING_NODE_INFO_W(node, nameStitched)
        nameStitched.append(L".m");
        LOG_MESH_FROM_FILENAME_AND_BUFFERS_W(nameStitched, meshP->GetNbPoints(), meshP->GetNbFaceIndexes(), meshP->GetPoints(), (int32_t*)meshP->GetFaceIndexes())
        }
#endif
            vector<vector<int32_t>> indices;
            indices.resize(2);
            ExtractMeshIndicesFromGraph(indices[0], &meshGraph);
            indices[1].resize(meshP->GetNbFaceIndexes());
            memcpy(&indices[1][0], meshP->GetFaceIndexes(), sizeof(int32_t)*indices[1].size());
            vector<vector<DPoint3d>> pts (2);
            pts[0] = nodePoints;
            pts[1].resize(meshP->GetNbPoints());

#if SM_OUTPUT_MESHES_STITCHING
            if (node->m_nodeHeader.m_level <= 6)
                {
                WString nameStitched = LOG_PATH_STR_W + L"posttrimesh_old_";
                LOGSTRING_NODE_INFO_W(node, nameStitched)
                nameStitched.append(L".m");
                LOG_MESH_FROM_FILENAME_AND_BUFFERS_W(nameStitched, nodePoints.size(), indices[0].size(), &nodePoints[0], (int32_t*)&indices[0][0])
                }
#endif
            memcpy(&pts[1][0], meshP->GetPoints(), sizeof(DPoint3d)*pts[1].size());
                       // arraySize = UpdateMeshNodeFromGraphs(node, &newNodePointData, graphs, pts, nfaces, extentMin, extentMax);
                arraySize = UpdateMeshNodeFromIndexLists(node, &newNodePointData, indices, pts, nfaces, extentMin, extentMax);
            stitchedPoints.resize(pts[1].size());
        }
    }
    else {
        //use the non-stitched node 
      arraySize = UpdateMeshNodeFromGraph(node, &newNodePointData, &meshGraph, nodePoints, nfaces, extentMin, extentMax);
    stitchedPoints.resize(nodePoints.size());
    }
    bvector<bvector<DPoint3d>> features;
    bvector<DTMFeatureType> types;
    node->ReadFeatureDefinitions(features, types);

    assert(node->m_nodeHeader.m_nbFaceIndexes == 0 || newNodePointData != nullptr);
    //node->setNbPointsUsedForMeshIndex(0);
    
    if (newNodePointData == nullptr) return true;

    if (s_useThreadsInStitching) node->LockPts();

    pointsPtr->clear();

    if (s_useThreadsInStitching) node->UnlockPts();

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
    if (componentsPointsId.size() > 0)
        {
        memcpy(node->m_nodeHeader.m_meshComponents, componentsPointsId.data(), componentsPointsId.size()*sizeof(int));
        }
    node->m_nodeHeader.m_numberOfMeshComponents = componentsPointsId.size();
    if (node->m_nodeHeader.m_nbFaceIndexes > 0)
        {
        if (s_useThreadsInStitching) node->LockPts();

        pointsPtr->push_back(&newNodePointData[0], stitchedPoints.size());
        
        node->ReplacePtsIndices((int32_t*)&newNodePointData[stitchedPoints.size()], node->m_nodeHeader.m_nbFaceIndexes);

      
        if (node->IsLeaf()) node->m_nodeHeader.m_totalCount = stitchedPoints.size();
        else
            {
            node->m_nodeHeader.m_totalCount -= nodePoints.size();
            node->m_nodeHeader.m_totalCount += stitchedPoints.size();
            }
       
        assert(pointsPtr->size() == stitchedPoints.size());
        if (s_useThreadsInStitching) node->UnlockPts();
                
        RefCountedPtr<SMMemoryPoolVectorItem<int32_t>>  linearFeaturesPtr = node->GetLinearFeaturesPtr();
        linearFeaturesPtr->clear();
        if (features.size() > 0)
            {
//            for (auto& def : node->m_featureDefinitions) if (!def.Discarded()) def.Discard();
            for (auto& polyline : features)
                {
                DRange3d extent = DRange3d::From(polyline);
                    {
                    WString namePoly = L"E:\\output\\scmesh\\2016-06-14\\afterstitchpoly_";
                    namePoly.append(std::to_wstring(node->m_nodeHeader.m_level).c_str());
                    namePoly.append(L"_");
                    namePoly.append(std::to_wstring(node->GetBlockID().m_integerID).c_str());
                    namePoly.append(L"_");
                    namePoly.append(std::to_wstring(ExtentOp<EXTENT>::GetXMin(node->m_nodeHeader.m_nodeExtent)).c_str());
                    namePoly.append(L"_");
                    namePoly.append(std::to_wstring(ExtentOp<EXTENT>::GetYMin(node->m_nodeHeader.m_nodeExtent)).c_str());
                    namePoly.append(L"_");
                    namePoly.append(to_wstring(&polyline - &features[0]).c_str());
                    namePoly.append(L".p");
                    FILE* polyCliPFile = _wfopen(namePoly.c_str(), L"wb");
                    size_t boundSize = polyline.size();
                    fwrite(&boundSize, sizeof(size_t), 1, polyCliPFile);
                    fwrite(&polyline[0], sizeof(DPoint3d), polyline.size(), polyCliPFile);
                    fclose(polyCliPFile);
                    }
                node->AddFeatureDefinitionSingleNode((ISMStore::FeatureType)types[&polyline - &features.front()], polyline, extent);
            }
        }
#if SM_OUTPUT_MESHES_STITCHING
        if (node->m_nodeHeader.m_level <= 6)
            {
            WString nameStitched = LOG_PATH_STR_W + L"poststitchmesh_";
            LOGSTRING_NODE_INFO_W(node, nameStitched)
            nameStitched.append(L".m");
            LOG_MESH_FROM_FILENAME_AND_BUFFERS_W(nameStitched, stitchedPoints.size(), node->m_nodeHeader.m_nbFaceIndexes, newNodePointData, (int32_t*)&newNodePointData[stitchedPoints.size()])
            }
#endif
#if SM_TRACE_MESH_STATS
        Utf8String fileName2 = (LOG_PATH_STR);
        LOGSTRING_NODE_INFO(node, fileName2)
        fileName2.append("_stats_after_stitch.txt");
        LOG_STATS_FOR_NODE(fileName2, node)
#endif
        delete[] newNodePointData;
            }
    //node->m_nodeHeader.m_nbFaceIndexes = nfaces;
    return true;
        }


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
    RefCountedPtr<SMMemoryPoolVectorItem<POINT>> pointsPtr(GetPointsPtr());
    
    if (pointsPtr->size() > 4)
        {        
        vector<DPoint3d> points (pointsPtr->size ());

        PtToPtConverter::Transform(&points[0], &(*pointsPtr)[0], points.size());

        IScalableMeshMeshPtr meshPtr;
#ifndef NO_3D_MESH
        Create3dDelaunayMesh(&points[0], (int)pointsPtr->size(), &draw, &meshPtr, nullptr, m_tetGen);
#endif

        ScalableMeshMesh* meshP ((ScalableMeshMesh*)meshPtr.get ());

        assert (meshP != 0);

        if (meshP != 0)
            {
            pointsPtr->clear();

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
            pointsPtr->push_back(&nodePts[0], nodePts.size());
            bvector<int> componentPointsId; //holds the leftmost point of each connected component
            RefCountedPtr<SMMemoryPoolGenericBlobItem<MTGGraph>> graphPtr(node->GetGraphPtr());
            //if (NULL == node->GetGraphPtr()) node->CreateGraph();
            else
                {
                *node->GetGraphPtr() = MTGGraph();
                }
            MTGGraph* newGraphP = new MTGGraph();
            CreateGraphFromIndexBuffer(/*node->GetGraphPtr()*/newGraphP, (const long*)meshP->GetFaceIndexes(), (int)meshP->GetNbFaceIndexes(), (int)nodePts.size(), componentPointsId, meshP->GetPoints());
            graphPtr->SetData(newGraphP);
            graphPtr->SetDirty();
            
            //node->SetGraphDirty();
            //node->StoreGraph();
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
          
            node->PushPtsIndices(&faceIndexes[0], node->m_nodeHeader.m_nbFaceIndexes);            
            
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
    double area = 0;

    RefCountedPtr<SMMemoryPoolVectorItem<int32_t>> ptIndices(node->GetPtsIndicePtr());

    if (ptIndices != nullptr)
        {
        const POINT nodePts[3] = { node->operator[]((*ptIndices)[0] - 1), node->operator[]((*ptIndices)[1] - 1), node->operator[]((*ptIndices)[2] - 1) };
        DPoint3d triangle[3] = { DPoint3d::From(nodePts[0].x, nodePts[0].y, nodePts[0].z), DPoint3d::From(nodePts[1].x, nodePts[1].y, nodePts[1].z), DPoint3d::From(nodePts[2].x, nodePts[2].y, nodePts[2].z) };
        area = bsiGeom_getXYPolygonArea(triangle, 3);
        }
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
                if (/*nPointsOutsideTile < 3*/true)
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
    for (size_t i = 0; i < faceIndices.size(); i += 3)
        {
        const POINT nodePts[3] = { geometryData[faceIndices[i] - 1], geometryData[faceIndices[i + 1] - 1], geometryData[faceIndices[i + 2] - 1] };
        DPoint3d triangle[3] = { DPoint3d::From(nodePts[0].x, nodePts[0].y, nodePts[0].z), DPoint3d::From(nodePts[1].x, nodePts[1].y, nodePts[1].z), DPoint3d::From(nodePts[2].x, nodePts[2].y, nodePts[2].z) };
        double newArea = bsiGeom_getXYPolygonArea(triangle, 3);
        if ((newArea < 0 && area>0) || (newArea > 0 && area < 0)) std::swap(faceIndices[i + 1], faceIndices[i + 2]);
        }
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
        DPoint3d sphereCenter=DPoint3d::From(0,0,0);
        double sphereRadius=0;
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

/**----------------------------------------------------------------------------
Initiates the stitching of the mesh present in neighbor nodes. 
-----------------------------------------------------------------------------*/
struct compare3D
    {
    bool operator()(const DPoint3d& a, const DPoint3d& b) const
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
    RefCountedPtr<SMMemoryPoolVectorItem<POINT>> pointsPtr(GetPointsPtr());

    vector<int> pointsToDestPointsMap(pointsPtr->size());
    std::fill_n(pointsToDestPointsMap.begin(), pointsToDestPointsMap.size(), -1);
    
//    EXTENT ext = node->GetContentExtent();
#if DEBUG && SM_TRACE_STITCHING 
    std::string neighborsVisited;
#endif
    for (size_t& neighborInd : neighborIndices)
        {
        size_t idx = &neighborInd - &neighborIndices[0];
        if ((node->m_apNeighborNodes[neighborInd].size() > 0))
            {
#if DEBUG && SM_TRACE_STITCHING 
            neighborsVisited += " visited neighbor " + std::to_string(neighborInd);
#endif
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
#if DEBUG && SM_TRACE_STITCHING 
                neighborsVisited += " for neighbor " + std::to_string(neighborInd) + " now have " + std::to_string(stitchedPoints.size()) + " points\r\n";
#endif

            }
        }
    //mesh aggregated points
    MTGGraph meshGraphStitched;
    IScalableMeshMeshPtr meshPtr;
    DPoint3d extentMin, extentMax;
    extentMin = DPoint3d::FromXYZ(ExtentOp<EXTENT>::GetXMin(node->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetYMin(node->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetZMin(node->m_nodeHeader.m_nodeExtent));
    extentMax = DPoint3d::FromXYZ(ExtentOp<EXTENT>::GetXMax(node->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetYMax(node->m_nodeHeader.m_nodeExtent), ExtentOp<EXTENT>::GetZMax(node->m_nodeHeader.m_nodeExtent));
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
    std::vector<DPoint3d> nodePoints(pointsPtr->size());

    std::map<DPoint3d, int, compare3D> stitchedSet;
        //std::vector<std::pair<DPoint3d, int>> stitchedSet;
    for (size_t i = 0; i < pointsPtr->size(); i++)
        {
        nodePoints[i].x = (*pointsPtr)[i].x;
        nodePoints[i].y = (*pointsPtr)[i].y;
        nodePoints[i].z = (*pointsPtr)[i].z;
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
    //node->setNbPointsUsedForMeshIndex(0);



    pointsPtr->clear();
    pointsPtr->push_back(&newNodePointData[0], arraySize);
    //node->setNbPointsUsedForMeshIndex(arraySize - stitchedPoints.size());
    delete[] newNodePointData;
    if (node->IsLeaf()) node->m_nodeHeader.m_totalCount = stitchedPoints.size();
    assert(node->size() == stitchedPoints.size());
    return true;
    }
