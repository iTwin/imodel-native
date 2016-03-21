#include <stdint.h>
typedef uint8_t byte;
#include <ScalableMesh\IScalableMesh.h>
#include <ScalableMesh\IScalableMeshNodeCreator.h>
#include <ScalableMesh\ScalableMeshLib.h>
#include <iostream>
#include <TerrainModel/Core/DTMIterators.h>
#include "..\STM\LogUtils.h"
#include <random>

int wmain(int argc, wchar_t* argv[])
{
struct  SMHost : ScalableMesh::ScalableMeshLib::Host
    {

    SMHost()
        {}
    ScalableMesh::ScalableMeshAdmin& _SupplyScalableMeshAdmin()
        {
        return *new ScalableMesh::ScalableMeshAdmin();
        };
    };
    ScalableMesh::ScalableMeshLib::Initialize(*new SMHost());
    BeFileName geocoordinateDataPath(L".\\GeoCoordinateData\\");
    GeoCoordinates::BaseGCS::Initialize(geocoordinateDataPath.c_str());
    
    if(argc < 2) 
        {
        std::cout << "Please input name of Scalable mesh file as first argument" << std::endl;
        return 0;
        }
#if 0
    BC_DTM_OBJ* bcDtmP = 0;
    bcdtmRead_fromFileDtmObject(&bcDtmP, argv[1]);
    FILE* mesh = _wfopen(argv[2], L"rb");
    std::vector<std::vector<DPoint3d>> polys;
    while (!feof(mesh))
        {
        size_t nVerts = 0;
        fread(&nVerts, sizeof(size_t), 1, mesh);
        std::vector<DPoint3d> polyPts(nVerts);
        size_t nRead = 0;
        nRead = fread(&polyPts[0], sizeof(DPoint3d), nVerts, mesh);
        polys.push_back(polyPts);
        }
    DTMUserTag    userTag = 0;
    DTMFeatureId* textureRegionIdsP = 0;
    long          numRegionTextureIds = 0;
    for (auto& polyPts : polys)
        {
        int stat = DTM_SUCCESS;
        if(polyPts.size() > 0) stat = bcdtmInsert_internalDtmFeatureMrDtmObject(bcDtmP,
                                                             DTMFeatureType::Region,
                                                             1,
                                                             2,
                                                             userTag,
                                                             &textureRegionIdsP,
                                                             &numRegionTextureIds,
                                                             &polyPts[0],
                                                             (long)polyPts.size(),
                                                             [] (int newPtNum, DPoint3dCR pt, double&elevation, bool onEdge, const int pts[])
            {
            std::cout << "ADDING " << newPtNum << " ON " << (onEdge ? "EDGE " : "FACET ") << pts[0] << " " << pts[1];
            if (!onEdge) std::cout << " " << pts[2];
            std::cout << std::endl;
            });
        if (stat != DTM_SUCCESS) std::cout << "ERROR" << std::endl;
        userTag++;
        }
    BENTLEY_NAMESPACE_NAME::TerrainModel::BcDTMPtr dtmP = BENTLEY_NAMESPACE_NAME::TerrainModel::BcDTM::CreateFromDtmHandle(bcDtmP);
    BENTLEY_NAMESPACE_NAME::TerrainModel::DTMMeshEnumeratorPtr en = BENTLEY_NAMESPACE_NAME::TerrainModel::DTMMeshEnumerator::Create(*dtmP);
    en->SetExcludeAllRegions();
    en->SetMaxTriangles(2000000);
    for (PolyfaceQueryP pf : *en)
        {
        PolyfaceHeaderPtr vec = PolyfaceHeader::CreateFixedBlockIndexed(3);
        vec->CopyFrom(*pf);

            WString name = WString(argv[1]) + WString(L"_mesh.m");
            FILE* meshAfterClip = _wfopen(name.c_str(), L"wb");
            size_t ptCount = vec->GetPointCount();
            size_t faceCount = vec->GetPointIndexCount();
            fwrite(&ptCount, sizeof(size_t), 1, meshAfterClip);
            fwrite(vec->GetPointCP(), sizeof(DPoint3d), ptCount, meshAfterClip);
            fwrite(&faceCount, sizeof(size_t), 1, meshAfterClip);
            fwrite(vec->GetPointIndexCP(), sizeof(int32_t), faceCount, meshAfterClip);
            fclose(meshAfterClip);
            
        }

    if(argc < 3) 
        {
        std::cout << "Please input name of output dir as second argument" << std::endl;
        return 0;
        }
    WString stmFileName(argv[1]);

    WString outDir(argv[2]);
    StatusInt status;
    ScalableMesh::IScalableMeshPtr meshP = ScalableMesh::IScalableMesh::GetFor(stmFileName.c_str(),true,true, status);
    ScalableMesh::IScalableMeshMeshQueryPtr meshQueryInterface = meshP->GetMeshQueryInterface(ScalableMesh::MESH_QUERY_FULL_RESOLUTION);
    for(size_t n = 0; n < (size_t)meshP->GetNbResolutions(); ++n)
        {
        bvector<ScalableMesh::IScalableMeshNodePtr> returnedNodes;
        ScalableMesh::IScalableMeshMeshQueryParamsPtr params = ScalableMesh::IScalableMeshMeshQueryParams::CreateParams();
        params->SetDepth(n);
        meshQueryInterface->Query(returnedNodes, NULL, 0, params);
        size_t nAnomalies = 0;
        for(auto& node: returnedNodes)
            {
            TerrainModel::BcDTMPtr dtmP;
            bvector<bool> clips;
            auto meshP = node->GetMesh(false, clips);
            if (meshP == nullptr) continue;
            DTMStatusInt ret = DTM_SUCCESS;
            try
                {
                ret = meshP->GetAsBcDTM(dtmP);
                }
            catch (...) {
                ret = DTM_ERROR;
                }
            if(DTM_SUCCESS != ret)
                {
                ++nAnomalies;
                WString path = outDir + std::to_wstring(node->GetNodeId()).c_str();
                path+= L"_node.m";
                const PolyfaceQuery* polyface = meshP->GetPolyfaceQuery();
                LOG_MESH_FROM_FILENAME_AND_BUFFERS_W(path,(size_t)polyface->GetPointCount(),(size_t)polyface->GetPointIndexCount(),polyface->GetPointCP(),polyface->GetPointIndexCP())
                }
            }
        std::cout << returnedNodes.size() << " RETURNED NODES "<< nAnomalies<< " ERRORS " << std::endl;
        }

    FILE* mesh = _wfopen(stmFileName.c_str(), L"rb");
        size_t nVerts = 0;
        size_t nIndices = 0;

        fread(&nVerts, sizeof(size_t), 1, mesh);
        std::vector<DPoint3d> allVerts (nVerts);
        fread(&allVerts[0], sizeof(DPoint3d), nVerts, mesh);
        fread(&nIndices, sizeof(size_t), 1, mesh);
        std::vector<int32_t> allIndices(nIndices);
        fread(&allIndices[0], sizeof(int32_t), nIndices, mesh);
        fclose(mesh);
        TerrainModel::BcDTMPtr bcdtm;
            BC_DTM_OBJ* bcDtmP = 0;
    int dtmCreateStatus = bcdtmObject_createDtmObject(&bcDtmP);
    if (dtmCreateStatus == 0)
        {
        bcdtm = TerrainModel::BcDTM::CreateFromDtmHandle(bcDtmP);
        }
    else return 1;
    DPoint3d triangle[4];

    for (unsigned int t = 0; t < nIndices; t += 3)
        {
        for (int i = 0; i < 3; i++)
            triangle[i] = allVerts[allIndices[i + t] - 1];

        triangle[3] = triangle[0];

        std::swap(triangle[1], triangle[2]);
        //apparently the below colinearity test requires at least first and last point to be distinct
        if (triangle[0].AlmostEqualXY(triangle[1]) || triangle[1].AlmostEqualXY(triangle[2]) || triangle[2].AlmostEqualXY(triangle[0])) continue;
        DSegment3d triSeg = DSegment3d::From(triangle[0], triangle[1]);
        double param;
        DPoint3d closestPt;
        triSeg.ProjectPointXY(closestPt, param, triangle[2]);
        if (closestPt.AlmostEqualXY(triangle[2])) continue;
        //DTM doesn't like colinear triangles
        //if (bsiGeom_isDPoint3dArrayColinear(triangle, 3, 1e-6)) continue;
        if (fabs(triangle[0].y - triangle[1].y) < 1e-4 && fabs(triangle[2].y - triangle[1].y) < 1e-4 && fabs(triangle[0].y - triangle[2].y) < 1e-4)
            std::cout << " ERROR COLINEAR TRIANGLE" << std::endl;
        bcdtmObject_storeDtmFeatureInDtmObject(bcdtm->GetTinHandle(), DTMFeatureType::GraphicBreak, bcdtm->GetTinHandle()->nullUserTag, 1, &bcdtm->GetTinHandle()->nullFeatureId, &triangle[0], 4);
        }
    bcdtmObject_triangulateStmTrianglesDtmObject(bcdtm->GetTinHandle());
#endif

    WString stmFileName(argv[1]);

    StatusInt status;
    ScalableMesh::IScalableMeshPtr meshP = ScalableMesh::IScalableMesh::GetFor(stmFileName.c_str(), true, true, status);
    ScalableMesh::IScalableMeshMeshQueryPtr meshQueryInterface = meshP->GetMeshQueryInterface(ScalableMesh::MESH_QUERY_FULL_RESOLUTION);
    auto draping = meshP->GetDTMInterface()->GetDTMDraping();
    int drapeType;
    DRange3d range;

    meshP->GetRange(range);
    range.ScaleAboutCenter(range,0.5);
    std::random_device rd;

    std::default_random_engine e1(rd());
    std::uniform_real_distribution<double> val_x(range.low.x, range.high.x);
    std::uniform_real_distribution<double> val_y(range.low.y, range.high.y);

    DPoint3d corners[8];
    range.Get8Corners(corners);
    size_t nbResolutions = meshP->GetNbResolutions();
    bvector<double> precisionVals(nbResolutions,0.0);
    bvector<size_t> nVals(nbResolutions, 0);
    bvector<DPoint3d> allTestPts;
    for (size_t i = 0; i < 1000; i++)
        {
        DPoint3d tmpPoint;
        tmpPoint.x = val_x(e1);
        tmpPoint.y = val_y(e1);
        draping->DrapePoint(&tmpPoint.z, NULL, NULL, NULL, &drapeType, tmpPoint);
        allTestPts.push_back(tmpPoint);
        }
    for (size_t i = 0; i < nbResolutions-1; ++i)
        {
        bvector<ScalableMesh::IScalableMeshNodePtr> returnedNodes;
        ScalableMesh::IScalableMeshMeshQueryParamsPtr params = ScalableMesh::IScalableMeshMeshQueryParams::CreateParams();
        params->SetLevel(i);
        meshQueryInterface->Query(returnedNodes, corners, 8, params);
        
        for (auto& node : returnedNodes)
            {
            for (auto& tmpPoint: allTestPts)
                {
                if (node->GetNodeExtent().IsContainedXY(tmpPoint))
                    {
                    double z = tmpPoint.z;
                    auto dtm = node->GetBcDTM();
                    if (dtm.get() != nullptr)
                        {
                        node->GetBcDTM()->DrapePoint(&z, NULL, NULL, NULL, &drapeType, &tmpPoint);
                        precisionVals[i] += fabs(z - tmpPoint.z);
                        nVals[i]++;
                        }
                    break;
                    }
                }
            }

        }

    for (size_t i = 0; i < nbResolutions - 1; ++i)
        {
        std::cout << " at resolution " << i << " precision is " << precisionVals[i] / nVals[i] << " m" << std::endl;
        }
    std::cout << "THE END" << std::endl;
    return 0;
}