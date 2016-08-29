#include <stdint.h>
typedef uint8_t byte;
using namespace std;

class SMNodeGroup;
class SMNodeGroupMasterHeader;

#include <Bentley\Bentley.h>
#include <ImagePP/h/ImageppAPI.h>
#include <TerrainModel/Core/bcdtmClass.h>
#include <ScalableMesh\IScalableMesh.h>
#include <ScalableMesh\IScalableMeshNodeCreator.h>
#include <ScalableMesh\ScalableMeshLib.h>
/*#undef static_assert
USING_NAMESPACE_BENTLEY_TERRAINMODEL
#include <ImagePP\all\h\HCDPacket.h>
#include <ImagePP\all\h\HCDCodecZlib.h>
#include "..\STM\ScalableMeshQuery.h"
#undef static_assert*/
#include <iostream>
#include <TerrainModel/Core/DTMIterators.h>
#include "..\STM\LogUtils.h"
#include <random>
#include <queue>
#include "..\STM\ScalableMesh\ScalableMeshGraph.h"
#include <json/json.h>

void SortPoints(bvector<DPoint3d>& allVerts, bvector<int>& allIndices)
    {
    std::map<DPoint3d, int, DPoint3dZYXTolerancedSortComparison> mapOfPoints(DPoint3dZYXTolerancedSortComparison(1e-5, 0));
    bvector<DPoint3d> sortedVerts = allVerts;
    std::sort(sortedVerts.begin(), sortedVerts.end(), [] (const DPoint3d&a, const DPoint3d&b)
        {
        if (a.x < b.x) return true;
        else if (a.x > b.x) return false;
        else if (a.y < b.y) return true;
        else if (a.y > b.y) return false;
        else return a.z < b.z;
        });
    for (auto& v : sortedVerts) mapOfPoints[v] = (int)(&v - &sortedVerts.front());
    for (auto& idx : allIndices) idx = mapOfPoints[allVerts[idx - 1]] + 1;
    allVerts = sortedVerts;
    }

void MakeDTM(TerrainModel::BcDTMPtr& dtmP, bvector<DPoint3d>& allVerts, bvector<int>& allIndices)
    {
    BC_DTM_OBJ* bcDtmP = 0;
    int dtmCreateStatus = bcdtmObject_createDtmObject(&bcDtmP);
    if (dtmCreateStatus == 0)
        {
        dtmP = TerrainModel::BcDTM::CreateFromDtmHandle(bcDtmP);
        }
    else return;
    DPoint3d triangle[4];

    bvector<int> indices;
    for (unsigned int t = 0; t < allIndices.size(); t += 3)
        {
        for (int i = 0; i < 3; i++)
            triangle[i] = allVerts[allIndices[i + t] - 1];

        triangle[3] = triangle[0];

        int32_t myIndices[3] = { allIndices[t] - 1, allIndices[t + 1] - 1, allIndices[t + 2] - 1 };
        //colinearity test
        if (triangle[0].AlmostEqualXY(triangle[1]) || triangle[1].AlmostEqualXY(triangle[2]) || triangle[2].AlmostEqualXY(triangle[0])) continue;
        DSegment3d triSeg = DSegment3d::From(triangle[0], triangle[1]);
        double param;
        DPoint3d closestPt;
        triSeg.ProjectPointXY(closestPt, param, triangle[2]);
        if (closestPt.AlmostEqualXY(triangle[2])) continue;
        indices.push_back(allIndices[t] - 1);
        if (bsiGeom_getXYPolygonArea(&triangle[0], 3) < 0)
            {
            indices.push_back(allIndices[t + 2] - 1);
            indices.push_back(allIndices[t + 1] - 1);
            }
        else
            {
            indices.push_back(allIndices[t + 1] - 1);
            indices.push_back(allIndices[t + 2] - 1);
            }
        }
    int status = bcdtmObject_storeTrianglesInDtmObject(dtmP->GetTinHandle(), DTMFeatureType::GraphicBreak, &allVerts[0], (int)allVerts.size(), &indices[0], (int)indices.size() / 3);

    assert(status == SUCCESS);

    //int status = bcdtmObject_triangulateStmTrianglesDtmObjectOld(bcdtm->GetTinHandle(), m_points, (int)m_nbPoints, m_faceIndexes, (int)m_nbFaceIndexes);
    status = bcdtmObject_triangulateStmTrianglesDtmObject(dtmP->GetTinHandle());
    assert(status == SUCCESS);


    BENTLEY_NAMESPACE_NAME::TerrainModel::DTMMeshEnumeratorPtr en = BENTLEY_NAMESPACE_NAME::TerrainModel::DTMMeshEnumerator::Create(*dtmP);
    en->SetMaxTriangles(2000000);
    bvector<DPoint3d> newVertices;
    bvector<int32_t> newIndices;
    std::map<DPoint3d, int32_t, DPoint3dZYXTolerancedSortComparison> mapOfPoints(DPoint3dZYXTolerancedSortComparison(1e-12, 0));
    for (PolyfaceQueryP pf : *en)
        {
        PolyfaceHeaderPtr vec = PolyfaceHeader::CreateFixedBlockIndexed(3);
        vec->CopyFrom(*pf);
        for (PolyfaceVisitorPtr addedFacets = PolyfaceVisitor::Attach(*vec); addedFacets->AdvanceToNextFace();)
            {
            DPoint3d face[3];
            int32_t idx[3] = { -1, -1, -1 };
            for (size_t i = 0; i < 3; ++i)
                {
                face[i] = addedFacets->GetPointCP()[i];
                idx[i] = mapOfPoints.count(face[i]) != 0 ? mapOfPoints[face[i]] : -1;
                }
            for (size_t i = 0; i < 3; ++i)
                {
                if (idx[i] == -1)
                    {
                    newVertices.push_back(face[i]);
                    idx[i] = (int)newVertices.size();
                    mapOfPoints[face[i]] = idx[i] - 1;
                    }
                else idx[i]++;
                }
            newIndices.push_back(idx[0]);
            newIndices.push_back(idx[1]);
            newIndices.push_back(idx[2]);
            }
        }
    WString name = L"E:\\makeTM\\test.m";
    LOG_MESH_FROM_FILENAME_AND_BUFFERS_W(name, newVertices.size(), newIndices.size(), &newVertices[0], &newIndices[0])
    /*bvector<DTMFeatureId> listIds;
    DTMFeatureCallback browseVoids = [] (DTMFeatureType dtmFeatureType, DTMUserTag userTag, DTMFeatureId featureId, DPoint3d *points, size_t numPoints, void* userArg) ->int
        {
        if (dtmFeatureType == DTMFeatureType::Void && numPoints <= 4)
            {
            ((bvector<DTMFeatureId>*)userArg)->push_back(featureId);
            }
        return 0;
        };

    dtmP->BrowseFeatures(DTMFeatureType::Void, 20, &listIds, browseVoids);
    for (auto& id : listIds) dtmP->DeleteFeatureById(id);*/
    }


void PolyfaceTag(PolyfaceHeaderPtr& poly, TerrainModel::BcDTMPtr& dtmPtr)
    {
    vector<int32_t> indices(poly->GetPointIndexCount());
    memcpy(&indices[0], poly->GetPointIndexCP(), poly->GetPointIndexCount()*sizeof(int32_t));
    bmap<int32_t, int32_t> allPts;
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

    poly->PointIndex().clear();
    for (size_t i = 0; i < indices.size(); i += 3)
        {

       
        poly->PointIndex().push_back(allPts[indices[i]] + 1);
        poly->PointIndex().push_back(allPts[indices[i + 1]] + 1);
        poly->PointIndex().push_back(allPts[indices[i + 2]] + 1);
      
        }
    }

int loadFunc(DTMFeatureType dtmFeatureType, int numTriangles, int numMeshPts, DPoint3d *meshPtsP, DPoint3d *meshVectorsP, int numMeshFaces, long *meshFacesP, void *userP)
    {
    size_t nNotContained = 0;
    for (size_t idx = 0; idx < numMeshFaces; idx += 3)
        {
        DPoint3d currentTri[3];
        currentTri[0] = (meshPtsP)[meshFacesP[idx] - 1];
        currentTri[1] = (meshPtsP)[meshFacesP[idx + 1] - 1];
        currentTri[2] = (meshPtsP)[meshFacesP[idx + 2] - 1];

        bvector<DPoint3d>* polyDefs = (bvector<DPoint3d>*) userP;
        DRange3d range = DRange3d::From(&polyDefs->front(), (int)polyDefs->size());
        if (!range.IsContainedXY(currentTri[0]) || !range.IsContainedXY(currentTri[1]) || !range.IsContainedXY(currentTri[2]))
            nNotContained++;
        }
    std::cout << (nNotContained > 0 ? " FAIL " : " PASS ") << std::endl;
    std::cout << nNotContained << " NOT CONTAINED " << std::endl;
    return SUCCESS;
    }

void RunDTMClipTest()
    {
    WString pathMeshes = L"E:\\dgndb06SM\\testData\\meshes\\";
    WString pathPolys = L"E:\\dgndb06SM\\testData\\polys\\";
    bvector<int> meshes(1);
    meshes[0] = 0;
    bvector<int> polys(3);
    polys[0] = 0;
    polys[1] = 1;
    polys[2] = 2;

    bvector<bvector<DPoint3d>> polyDefs(polys.size());
    vector<TerrainModel::BcDTMPtr> meshDefs(meshes.size());

    for (size_t i = 0; i < meshes.size(); ++i)
        {
        WString path = pathMeshes + WString(std::to_wstring(meshes[i]).c_str()) + WString(L".m");
        FILE* mesh = _wfopen(path.c_str(), L"rb");
        size_t nVerts = 0;
        size_t nIndices = 0;
        fread(&nVerts, sizeof(size_t), 1, mesh);
        bvector<DPoint3d> allVerts(nVerts);
        fread(&allVerts[0], sizeof(DPoint3d), nVerts, mesh);
        fread(&nIndices, sizeof(size_t), 1, mesh);
        bvector<int32_t> allIndices(nIndices);
        fread(&allIndices[0], sizeof(int32_t), nIndices, mesh);
        MakeDTM(meshDefs[i], allVerts, allIndices);
        }
    for (size_t i = 0; i < polys.size(); ++i)
        {
        WString path = pathPolys + WString(std::to_wstring(polys[i]).c_str()) + WString(L".p");
        FILE* mesh = _wfopen(path.c_str(), L"rb");
        size_t nVerts = 0;
        fread(&nVerts, sizeof(size_t), 1, mesh);
        polyDefs[i].resize(nVerts);
        fread(&polyDefs[i][0], sizeof(DPoint3d), nVerts, mesh);
        }

    size_t nNotContained = 0;
    for (auto& mesh : meshDefs)
        {
        DTMUserTag    userTag = 0;
        DTMFeatureId* textureRegionIdsP = 0;
        long          numRegionTextureIds = 0;
        bvector<DTMFeatureId> featureIds(polys.size());
        for (auto& poly : polyDefs)
            {
            bcdtmInsert_internalDtmFeatureMrDtmObject(mesh->GetBcDTM()->GetTinHandle(),
                                                      DTMFeatureType::Region,
                                                      1,
                                                      2,
                                                      userTag,
                                                      &textureRegionIdsP,
                                                      &numRegionTextureIds,
                                                      &poly[0],
                                                      (long)poly.size(),
                                                      nullptr);

            featureIds[userTag] = textureRegionIdsP[0];
            /*for (size_t regionId = 0; regionId < numRegionTextureIds; regionId++)
            {
            static long maxTriangles = 65000;  // Maximum Triangles To Pass Back
            static long vectorOption = 2;  // Averaged Triangle Surface Normals
            static double zAxisFactor = 1.0;  // Am\ount To Exaggerate Z Axis
            static long regionOption = 1;  // Include Internal Regions Until Fully Tested
            static long indexOption = 2;  // Use Feature Id
            bcdtmLoad_triangleShadeMeshForRegionDtmObject((BC_DTM_OBJ*)mesh->GetBcDTM()->GetTinHandle(),
            maxTriangles,
            vectorOption,
            zAxisFactor,
            regionOption,
            indexOption,
            textureRegionIdsP[regionId],
            &loadFunc,
            &poly
            );
            }*/
            userTag++;
            }
        BENTLEY_NAMESPACE_NAME::TerrainModel::DTMMeshEnumeratorPtr en = BENTLEY_NAMESPACE_NAME::TerrainModel::DTMMeshEnumerator::Create(*mesh->GetBcDTM());
        //en->SetUseRealPointIndexes(true);
        en->SetExcludeAllRegions();
        en->SetMaxTriangles(2000000);
        bmap<DPoint3d, int32_t, DPoint3dZYXTolerancedSortComparison> mapOfPoints(DPoint3dZYXTolerancedSortComparison(1e-6, 0));
        for (size_t i = 0; i < (size_t)mesh->GetBcDTM()->GetPointCount(); ++i)
            {
            DPoint3d pt;
            mesh->GetBcDTM()->GetPoint((int)i, pt);
            mapOfPoints[pt] = (int)i;
            }
        for (PolyfaceQueryP pf : *en)
            {
            PolyfaceHeaderPtr vec = PolyfaceHeader::CreateFixedBlockIndexed(3);
            vec->CopyFrom(*pf);
            }
        for (size_t j = 0; j < polys.size(); ++j)
            {
            en->Reset();
            en->SetFilterRegionByUserTag(j);
            for (PolyfaceQueryP pf : *en)
                {
                PolyfaceHeaderPtr vec = PolyfaceHeader::CreateFixedBlockIndexed(3);
                vec->CopyFrom(*pf);
                //PolyfaceTag(vec, mesh);
                for (size_t idx = 0; idx < vec->GetPointIndexCount(); idx += 3)
                    {
                    DPoint3d currentTri[3];
                    currentTri[0] = (vec->GetPointCP())[vec->GetPointIndexCP()[idx] - 1];
                    currentTri[1] = (vec->GetPointCP())[vec->GetPointIndexCP()[idx + 1] - 1];
                    currentTri[2] = (vec->GetPointCP())[vec->GetPointIndexCP()[idx + 2] - 1];

                    DRange3d range = DRange3d::From(&polyDefs[j][0], (int)polyDefs[j].size());
                    if (!range.IsContainedXY(currentTri[0]) || !range.IsContainedXY(currentTri[1]) || !range.IsContainedXY(currentTri[2]))
                        {
                        std::cout << " TRIANGLE FAIL " << mapOfPoints[currentTri[0]] << "," << mapOfPoints[currentTri[1]] << "," << mapOfPoints[currentTri[2]] << std::endl;
                        nNotContained++;
                        }
                    }
                }
            /*static long maxTriangles = 65000;  // Maximum Triangles To Pass Back
            static long vectorOption = 2;  // Averaged Triangle Surface Normals
            static double zAxisFactor = 1.0;  // Am\ount To Exaggerate Z Axis
            static long regionOption = 1;  // Include Internal Regions Until Fully Tested
            static long indexOption = 2;  // Use Feature Id
            bcdtmLoad_triangleShadeMeshForRegionDtmObject((BC_DTM_OBJ*)mesh->GetBcDTM()->GetTinHandle(),
            maxTriangles,
            vectorOption,
            zAxisFactor,
            regionOption,
            indexOption,
            featureIds[j],
            &loadFunc,
            &polyDefs[j]
            );*/

            }
        }
    std::cout << (nNotContained > 0 ? " FAIL " : " PASS ") << std::endl;
    std::cout << nNotContained << " NOT CONTAINED " << std::endl;
    }

void RunPrecisionTest(WString& stmFileName)
    {

    StatusInt status;
    ScalableMesh::IScalableMeshPtr meshP = ScalableMesh::IScalableMesh::GetFor(stmFileName.c_str(), true, true, status);
    ScalableMesh::IScalableMeshMeshQueryPtr meshQueryInterface = meshP->GetMeshQueryInterface(ScalableMesh::MESH_QUERY_FULL_RESOLUTION);
    auto draping = meshP->GetDTMInterface()->GetDTMDraping();
    int drapeType;
    DRange3d range;

    meshP->GetRange(range);
   // range.low.x += (range.XLength() / 2);
   // range.low.y += (range.YLength() / 2);
    range.ScaleAboutCenter(range, 0.5);
    std::random_device rd;

    std::default_random_engine e1(rd());
    std::uniform_real_distribution<double> val_x(range.low.x, range.high.x);
    std::uniform_real_distribution<double> val_y(range.low.y, range.high.y);

    DPoint3d corners[8];
    range.Get8Corners(corners);
    size_t nbResolutions = meshP->GetNbResolutions();
    bvector<double> precisionVals(nbResolutions, 0.0);
    bvector<size_t> nVals(nbResolutions, 0);
    bvector<DPoint3d> allTestPts;
    for (size_t i = 0; i < 1000; i++)
        {
        DPoint3d tmpPoint;
        tmpPoint.x = val_x(e1);
        tmpPoint.y = val_y(e1);
        draping->DrapePoint(&tmpPoint.z, NULL, NULL, NULL, &drapeType, tmpPoint);
        if(tmpPoint.z != DBL_MAX) allTestPts.push_back(tmpPoint);
        }
    for (size_t i = 0; i < nbResolutions - 1; ++i)
        {
        bvector<ScalableMesh::IScalableMeshNodePtr> returnedNodes;
        ScalableMesh::IScalableMeshMeshQueryParamsPtr params = ScalableMesh::IScalableMeshMeshQueryParams::CreateParams();
        params->SetLevel(i);
        meshQueryInterface->Query(returnedNodes, corners, 8, params);

        for (auto& node : returnedNodes)
            {
            for (auto& tmpPoint : allTestPts)
                {
                if (node->GetNodeExtent().IsContainedXY(tmpPoint))
                    {
                    double z = tmpPoint.z;
                    auto dtm = node->GetBcDTM();
                    if (dtm.get() != nullptr)
                        {
                        node->GetBcDTM()->DrapePoint(&z, NULL, NULL, NULL, &drapeType, &tmpPoint);
                        if (z != DBL_MAX)
                            {
                            precisionVals[i] += fabs(z - tmpPoint.z);
                            nVals[i]++;
                            }
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

    }

void RunIntersectRay()
    {
    DPoint3d testPt = DPoint3d::From(303665.63996983197,256052.27654610365,11299.552886447620);
    DPoint3d endPt = DPoint3d::From(303665.65128480532,256052.54358510373,11298.589267153598);

    TerrainModel::BcDTMPtr dtm = TerrainModel::BcDTM::CreateFromTinFile(L"E:\\makeTM\\ptPick.tin");
    DPoint3d ptOut;
    dtm->IntersectVector(ptOut, testPt, endPt);

    DPlane3d plane = DPlane3d::From3Points(DPoint3d::From(303780.30394857755, 259172.41251152655, 52.999999999999993),
                                           DPoint3d::From(303810.29302139767, 259171.62779438784, 49.999999999999993),
                                           DPoint3d::From(303809.50776461099, 259141.63913370154, 53.999999999999993));
    DRay3d ray = DRay3d::FromOriginAndVector(testPt, DVec3d::FromStartEnd(testPt, endPt));
    double param;
    DPoint3d rayOut;
    ray.Intersect(rayOut, param, plane);
    }

void RunWriteTileTest(WString& stmFileName, const wchar_t* tileID)
    {
 /*   LOG_SET_PATH_W("E:\\output\\scmesh\\2016-04-15\\")
    StatusInt status;
    ScalableMesh::IScalableMeshPtr meshP = ScalableMesh::IScalableMesh::GetFor(stmFileName.c_str(), true, true, status);
    wchar_t *end;
    int nodeID = wcstol(tileID, &end, 10);
    ScalableMesh::IScalableMeshMeshQueryPtr meshQueryInterface = meshP->GetMeshQueryInterface(ScalableMesh::MESH_QUERY_FULL_RESOLUTION);
    size_t nbResolutions = meshP->GetNbResolutions();
    for (size_t i = 0; i < nbResolutions; ++i)
        {
        bvector<ScalableMesh::IScalableMeshNodePtr> returnedNodes;
        ScalableMesh::IScalableMeshMeshQueryParamsPtr params = ScalableMesh::IScalableMeshMeshQueryParams::CreateParams();
        params->SetLevel(i);
        meshQueryInterface->Query(returnedNodes, nullptr, 0, params);

        for (auto& node : returnedNodes)
            {
            if (node->GetNodeId() == nodeID)
                {
                ScalableMesh::IScalableMeshMeshFlagsPtr flags = ScalableMesh::IScalableMeshMeshFlags::Create();
                bvector<bool> clips;
                auto meshPtr = node->GetMesh(flags, clips);
                WString name = LOG_PATH_STR_W + L"mesh_";
                name.append(std::to_wstring(nodeID).c_str());
                    name.append(L".m");
                    ScalableMesh::ScalableMeshMesh * meshP = dynamic_cast<ScalableMesh::ScalableMeshMesh*>(meshPtr.get());
                LOG_MESH_FROM_FILENAME_AND_BUFFERS_W(name, meshP->GetNbPoints(), meshP->GetNbFaceIndexes(), meshP->GetPoints(), (int32_t*)meshP->GetFaceIndexes())
                    break;
                }
            }
        }*/
    }

void RunDTMTriangulateTest()
    {
    //while (true)
        {
        WString pathMeshes = L"E:\\makeTM\\submesh";
        bvector<DPoint3d> allPts;
        for (size_t i = 0; i < 4; ++i)
            {
            WString path = pathMeshes + WString(std::to_wstring(i).c_str()) + WString(L".pts");
            FILE* mesh = _wfopen(path.c_str(), L"rb");
            size_t nVerts = 0;
            fread(&nVerts, sizeof(size_t), 1, mesh);
            bvector<DPoint3d> allVerts(nVerts);
            fread(&allVerts[0], sizeof(DPoint3d), nVerts, mesh);
            fclose(mesh);
            size_t offset = allPts.size();
            allPts.resize(allPts.size() + nVerts);

            for (auto&pt : allVerts) pt.z = 0;

            std::random_shuffle(allVerts.begin(), allVerts.end());

            size_t count = (allVerts.size() / 4) + 1;

            memcpy(&allPts[offset], &allVerts[0], sizeof(DPoint3d)*std::min(count, allVerts.size()));
            }
        TerrainModel::DTMPtr dtmPtr;
        BC_DTM_OBJ* bcDtmP = 0;
        int dtmCreateStatus = bcdtmObject_createDtmObject(&bcDtmP);
        if (dtmCreateStatus == 0)
            {
            TerrainModel::BcDTMPtr bcDtmObjPtr;
            bcDtmObjPtr = TerrainModel::BcDTM::CreateFromDtmHandle(bcDtmP);
            dtmPtr = bcDtmObjPtr.get();
            }
        bcdtmObject_storeDtmFeatureInDtmObject(bcDtmP, DTMFeatureType::RandomSpots, bcDtmP->nullUserTag, 1, &bcDtmP->nullFeatureId, &allPts[0], (long)allPts.size());
        int status = bcdtmObject_triangulateDtmObject(bcDtmP);
        if (status != SUCCESS)
            {
            std::cout << "ERROR!" << std::endl;
            WString str(L"e:\\test");
            str.append(L".dtm");
            bcdtmWrite_toFileDtmObject(bcDtmP, str.c_str());
            }
        }
    }

void RunDTMSTMTriangulateTest()
    {
    WString pathMeshes = L"E:\\makeTM\\mesh";
    WString path = pathMeshes + WString(L".m");
    FILE* mesh = _wfopen(path.c_str(), L"rb");
    size_t nVerts = 0;
    size_t nIndices = 0;
    fread(&nVerts, sizeof(size_t), 1, mesh);
    bvector<DPoint3d> allVerts(nVerts);
    fread(&allVerts[0], sizeof(DPoint3d), nVerts, mesh);
    fread(&nIndices, sizeof(size_t), 1, mesh);
    bvector<int32_t> allIndices(nIndices);
    fread(&allIndices[0], sizeof(int32_t), nIndices, mesh);
    TerrainModel::BcDTMPtr bcdtm;
    SortPoints(allVerts, allIndices);
    MakeDTM(bcdtm, allVerts, allIndices);
    }

void LoadMesh(bvector<DPoint3d>& vertices, bvector<int>& indices, WString& path)
    {
    FILE* mesh = _wfopen(path.c_str(), L"rb");
    size_t nVerts = 0;
    size_t nIndices = 0;
    fread(&nVerts, sizeof(size_t), 1, mesh);
    vertices.resize(nVerts);
    fread(&vertices[0], sizeof(DPoint3d), nVerts, mesh);
    fread(&nIndices, sizeof(size_t), 1, mesh);
    indices.resize(nIndices);
    fread(&indices[0], sizeof(int32_t), nIndices, mesh);
    fclose(mesh);
    }

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

void circumcircle(DPoint3d& center, double& radius, const DPoint3d* triangle)
    {
    double det = RotMatrix::FromRowValues(triangle[0].x, triangle[0].y, 1, triangle[1].x, triangle[1].y, 1, triangle[2].x, triangle[2].y, 1).Determinant();
    double vals[3] = { (triangle[0].x * triangle[0].x + triangle[0].y * triangle[0].y),
        (triangle[1].x * triangle[1].x + triangle[1].y * triangle[1].y),
        (triangle[2].x * triangle[2].x + triangle[2].y * triangle[2].y) };
    double det1 = -(RotMatrix::FromRowValues(vals[0], triangle[0].y, 1, vals[1], triangle[1].y, 1, vals[2], triangle[2].y, 1).Determinant());
    double det2 = RotMatrix::FromRowValues(vals[0], triangle[0].x, 1, vals[1], triangle[1].x, 1, vals[2], triangle[2].x, 1).Determinant();
    center.x = -det1 / (2 * det);
    center.y = -det2 / (2 * det);
    double detR = RotMatrix::FromRowValues(vals[0], triangle[0].x, triangle[0].y, vals[1], triangle[1].x, triangle[1].y, vals[2], triangle[2].x, triangle[2].y).Determinant();
    radius = sqrt(det1*det1 + det2*det2 - 4 * det*detR) / (2 * abs(det));
    }

void circumcircle2(DPoint3d& center, double& radius, const DPoint3d* triangle)
    {
    double det = RotMatrix::FromRowValues(triangle[0].x, triangle[0].y, 1, triangle[1].x, triangle[1].y, 1, triangle[2].x, triangle[2].y, 1).Determinant();
    double mags[3] = { triangle[0].MagnitudeSquaredXY(),
        triangle[1].MagnitudeSquaredXY(),
        triangle[2].MagnitudeSquaredXY() };
    double det1 = 0.5*(RotMatrix::FromRowValues(mags[0], triangle[0].y, 1, mags[1], triangle[1].y, 1, mags[2], triangle[2].y, 1).Determinant());
    double det2 = 0.5*RotMatrix::FromRowValues(triangle[0].x, mags[0], 1, triangle[1].x, mags[1], 1, triangle[2].x, mags[2], 1).Determinant();
    center.x = det1 /  det;
    center.y = det2 / det;
    double detR = RotMatrix::FromRowValues(triangle[0].x, triangle[0].y, mags[0], triangle[1].x, triangle[1].y, mags[1], triangle[2].x, triangle[2].y, mags[2]).Determinant();
    radius = sqrt(detR/det + DPoint3d::From(det1, det2, 0).MagnitudeSquaredXY()/(det*det));
    }

void SelectPointsToStitch(bvector<DPoint3d>& stitchedPoints, MTGGraph* meshGraph, DRange3d& neighborExt, bvector<DPoint3d>& pts)
    {
    DRange3d nodeExt = DRange3d::From(pts);
    std::queue<MTGNodeId> bounds;
    MTGARRAY_SET_LOOP(edgeID, meshGraph)
        {
        int vtx = -1;
        meshGraph->TryGetLabel(edgeID, 0, vtx);
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
                if (vIndex <= 0)break;
                faceIdx[edgeAroundFace] = vIndex;

                DPoint3d pt = pts[vIndex - 1];
                facePoints[edgeAroundFace] = DPoint3d::FromXYZ(pt.x, pt.y, pt.z);
                }
            edgeAroundFace++;
            if (edgeAroundFace > 3)break;
            }

        MTGARRAY_END_FACE_LOOP(aroundFaceIndex, meshGraph, currentID)
            if (edgeAroundFace < 3) continue;
        if (edgeAroundFace > 3)
            {
            bounds.push(meshGraph->EdgeMate(currentID));
            continue;
            }
        for (size_t i = 0; i < 3; ++i) facePoints[i].DifferenceOf(facePoints[i], nodeExt.low);
        DPoint3d center = DPoint3d::From(0, 0, 0);
        double radius = 0;
        circumcircle(center, radius, facePoints);
        for (size_t i = 0; i < 3; ++i) facePoints[i].SumOf(facePoints[i], nodeExt.low);
        center.SumOf(center, nodeExt.low);
        if (DRange3d::From(center.x - radius, center.y - radius,
            center.z, center.x + radius,
            center.y + radius, center.z).IntersectsWith(neighborExt, 2))
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

void RunSelectPointsTest()
    {
    WString pathMeshes = L"E:\\makeTM\\select";
    WString path = pathMeshes + WString(L".m");
    bvector<DPoint3d> allVerts1;
    bvector<int32_t> allIndices1;
    LoadMesh(allVerts1, allIndices1, path);
    WString path2 = pathMeshes + WString(L"2.m");
    bvector<DPoint3d> allVerts2;
    bvector<int32_t> allIndices2;
    LoadMesh(allVerts2, allIndices2, path);

    DRange3d range2 = DRange3d::From(allVerts2);
    MTGGraph g;
    bvector<int> componentPointsId;
    ScalableMesh::CreateGraphFromIndexBuffer(&g, (const long*)&allIndices1[0], (int)allIndices1.size(), (int)allVerts1.size(), componentPointsId, &allVerts1[0]);

    bvector<DPoint3d> stitchedPts;
    SelectPointsToStitch(stitchedPts,&g, range2, allVerts1);
    }

void ParseNodeInfo(ScalableMesh::IScalableMeshNodePtr& node, bvector<size_t>& ptsAtLevel, bvector<size_t>& nodesAtLevel)
    {


    ScalableMesh::IScalableMeshMeshFlagsPtr flags = ScalableMesh::IScalableMeshMeshFlags::Create();
    flags->SetLoadGraph(false);
    ScalableMesh::IScalableMeshMeshPtr mesh = node->GetMesh(flags);
    size_t ptCount = 0;
    if (mesh.get() == nullptr)
        {
        //std::cout << " NO MESH FOR NODE " << node->GetNodeId() << std::endl;
        }
    else
        {
       // std::cout << " NODE " << node->GetNodeId() << " HAS " << mesh->GetNbFaces() << " FACES " << std::endl;
        ptCount = mesh->GetNbPoints();
        }
    nodesAtLevel[node->GetLevel()]++;
    ptsAtLevel[node->GetLevel()] += ptCount;


    if (ptCount > 65000)
        std::cout << " NODE " << node->GetNodeId() << " HAS " << ptCount << " POINTS AT LEVEL " << node->GetLevel() << std::endl;

    bvector<ScalableMesh::IScalableMeshNodePtr> childrenNodes = node->GetChildrenNodes();
    if (mesh.get() == nullptr && childrenNodes.size() > 0 && childrenNodes.front()->GetPointCount() > 0)
        {
        std::cout << " NODE " << node->GetNodeId() << " HAS " << ptCount << " POINTS AT LEVEL " << node->GetLevel() << std::endl;
        std::cout << " NODE " << node->GetNodeId() << " HAS " << childrenNodes.size() << " CHILDREN " << std::endl;
        std::cout << " NODE " << childrenNodes.front()->GetNodeId() << " HAS " << childrenNodes.front()->GetPointCount() << " POINTS AT LEVEL " << childrenNodes.front()->GetLevel() << std::endl;
        }
    //std::cout << " NODE " << node->GetNodeId() << " HAS " << childrenNodes.size() << " CHILDREN " << std::endl;
    for (auto child : childrenNodes)
        {
        ParseNodeInfo(child, ptsAtLevel, nodesAtLevel);
        }
    }

void RunParseTree(WString& stmFileName)
    {

    StatusInt status;
    ScalableMesh::IScalableMeshPtr meshP = ScalableMesh::IScalableMesh::GetFor(stmFileName.c_str(), true, true, status);

    bvector<size_t> nPointsAtLevel((size_t)meshP->GetNbResolutions());
    bvector<size_t> nNodesAtLevel((size_t)meshP->GetNbResolutions());
    ScalableMesh::IScalableMeshNodePtr root = meshP->GetRootNode();


    ParseNodeInfo(root, nPointsAtLevel, nNodesAtLevel);

    for (size_t i = 0; i < (size_t)meshP->GetNbResolutions(); ++i)
        {
        std::cout << " AT LEVEL " << i << " WE HAVE " << nPointsAtLevel[i] << " POINTS AND " << nNodesAtLevel[i] << " NODES " << std::endl;
        }


    }

void RunIntersectRayMetadata(WString& stmFileName)
    {
    StatusInt status;
    ScalableMesh::IScalableMeshPtr meshP = ScalableMesh::IScalableMesh::GetFor(stmFileName.c_str(), true, true, status);



    DPoint3d tmpPoint = DPoint3d::From(19068292.12, 7531835.83, 3000000);

    DVec3d direction = DVec3d::From(0,0,-1);
    DRay3d ray = DRay3d::FromOriginAndVector(tmpPoint, direction);
    DPoint3d intersectPoint_l;

    //ScalableMesh::IScalableMeshNodeQueryParamsPtr params = ScalableMesh::IScalableMeshNodeQueryParams::CreateParams();
    //ScalableMesh::IScalableMeshNodeRayQueryPtr query = meshP->GetNodeQueryInterface();
    ScalableMesh::IScalableMeshMeshQueryPtr meshQueryInterface = meshP->GetMeshQueryInterface(ScalableMesh::MESH_QUERY_FULL_RESOLUTION);
    ScalableMesh::IScalableMeshMeshQueryParamsPtr params = ScalableMesh::IScalableMeshMeshQueryParams::CreateParams();
    params->SetLevel(meshP->GetTerrainDepth());
    bvector<ScalableMesh::IScalableMeshNodePtr> nodes;
   // params->SetDirection(direction);
    meshQueryInterface->Query(nodes, NULL, 0, params);
    Json::Value val;
    for (auto& node : nodes)
        {
        DSegment3d clipped;
        DRange1d fraction;
        if (!ray.ClipToRange(node->GetContentExtent(), clipped, fraction)) continue;
        if (node->IntersectRay(intersectPoint_l, ray, val))
            {
            std::cout << val["elementId"].asInt64() << std::endl;
            return;
            }
        }

    }

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

#if 0
    if(argc < 2) 
        {
        std::cout << "Please input name of Scalable mesh file as first argument" << std::endl;
        return 0;
        }

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

   /* WString stmFileName(argv[1]);
    RunPrecisionTest(stmFileName);*/


    //RunDTMClipTest();
    RunDTMTriangulateTest();
    //RunDTMSTMTriangulateTest();
   // RunSelectPointsTest();
    //RunIntersectRay();
    //WString stmFileName(argv[1]);
   // RunParseTree(stmFileName);
   // RunIntersectRayMetadata(stmFileName);
    /*WString stmFileName(argv[1]);
    RunWriteTileTest(stmFileName, argv[2]);*/
    std::cout << "THE END" << std::endl;
    return 0;
}