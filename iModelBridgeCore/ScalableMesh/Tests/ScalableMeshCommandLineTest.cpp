#include <stdint.h>
typedef uint8_t byte;
#include <ScalableMesh\IScalableMesh.h>
#include <ScalableMesh\IScalableMeshNodeCreator.h>
#include <ScalableMesh\ScalableMeshLib.h>
#include <iostream>
#include "..\STM\LogUtils.h"

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
#if 0
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
    std::cout << "THE END" << std::endl;
    return 0;
}