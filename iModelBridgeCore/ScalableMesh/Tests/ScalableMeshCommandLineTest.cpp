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
    WString pathMeshes = L"E:\\output\\scmesh\\2016-04-13\\mesh_tile_2875_13_-183.424623_-50.422123";
    WString path = pathMeshes + WString(L".pts");
    FILE* mesh = _wfopen(path.c_str(), L"rb");
    size_t nVerts = 0;
    fread(&nVerts, sizeof(size_t), 1, mesh);
    bvector<DPoint3d> allVerts(nVerts);
    fread(&allVerts[0], sizeof(DPoint3d), nVerts, mesh);
    fclose(mesh);

    for (auto&pt : allVerts) pt.z = 0;
    TerrainModel::DTMPtr dtmPtr;
    BC_DTM_OBJ* bcDtmP = 0;
    int dtmCreateStatus = bcdtmObject_createDtmObject(&bcDtmP);
    if (dtmCreateStatus == 0)
        {
        TerrainModel::BcDTMPtr bcDtmObjPtr;
        bcDtmObjPtr = TerrainModel::BcDTM::CreateFromDtmHandle(bcDtmP);
        dtmPtr = bcDtmObjPtr.get();
        }
    bcdtmObject_storeDtmFeatureInDtmObject(bcDtmP, DTMFeatureType::RandomSpots, bcDtmP->nullUserTag, 1, &bcDtmP->nullFeatureId, &allVerts[0], (long)nVerts);
    bcdtmObject_triangulateDtmObject(bcDtmP);
    WString str(L"e:\\test");
    str.append(L".dtm");
    bcdtmWrite_toFileDtmObject(bcDtmP, str.c_str());
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
    //RunDTMTriangulateTest();
    RunDTMSTMTriangulateTest();
    /*WString stmFileName(argv[1]);
    RunWriteTileTest(stmFileName, argv[2]);*/
    std::cout << "THE END" << std::endl;
    return 0;
}