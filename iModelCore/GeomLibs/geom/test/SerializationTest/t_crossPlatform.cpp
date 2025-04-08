/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "testHarness.h"
#include <GeomSerialization/GeomSerializationApi.h>

// Verify that the native API can read flatbuffer and json files written by the typescript and native APIs.
// Each test case consists of at least four files that encode the same single geometry: native-authored fb and json, typescript-authored fb and json
// The same tests exist in typescript core-geometry and operate on the same files; these tests and files should be kept in sync.

static char buf[MAX_PATH];

// this fixup is not needed in the TypeScript crossPlatform test because only native API deserializes in fixed-size mesh format
static void convertNativeMeshToVariableSized(IGeometryR geom)
    {
    auto mesh = geom.GetAsPolyfaceHeader();
    if (mesh.IsValid() && !mesh->IsVariableSizeIndexed())
        Check::True(mesh->ConvertToVariableSizeSignedOneBasedIndexedFaceLoops(), "successfully converted mesh from fixed to variable sized index blocking");
    };

struct TestCase
    {
    enum Platform { Native = 0, TypeScript = 1 };
    enum FileType { FlatBuffer = 0, JSON = 1 };
    // m_fileNames[Platform][FileType] are names of files of given type written by the given platform
    bvector<bvector<bvector<BeFileName>>> m_fileNames;
    TestCase()
        {
        m_fileNames.push_back(bvector<bvector<BeFileName>>(2)); // native-authored files
        m_fileNames.push_back(bvector<bvector<BeFileName>>(2)); // typescript-authored files
        }
    static BeFileName s_root;
    static BeFileNameCR GetRoot()
        {
        if (s_root.IsEmpty())
            {
            BeTest::GetHost().GetDocumentsRoot(s_root);
            s_root.AppendToPath(L"GeomLibsTestData").AppendToPath(L"crossPlatform");
            }
        return s_root;
        }
    static BeFileName NativeRoot()
        {
        BeFileName nativeRoot(GetRoot());
        nativeRoot.AppendToPath(L"native");
        return nativeRoot;
        }
    static BeFileName TypeScriptRoot()
        {
        BeFileName typeScriptRoot(GetRoot());
        typeScriptRoot.AppendToPath(L"typescript");
        return typeScriptRoot;
        }
    };
BeFileName TestCase::s_root;

// A file may contain multiple geometries. Deserialize only the first.
static IGeometryPtr deserializeFirstGeom(BeFileNameCR fileName, TestCase::FileType fileType)
    {
    bvector<IGeometryPtr> geoms;
    bool converted = false;
    if (TestCase::FlatBuffer == fileType)
        converted = GTestFileOps::FlatBufferFileToGeometry(fileName, geoms);
    else if (TestCase::JSON == fileType)
        converted = GTestFileOps::JsonFileToGeometry(fileName, geoms);
    if (converted && geoms.size() > 0)
        {
        convertNativeMeshToVariableSized(*geoms[0]);
        return geoms[0];
        }
    return nullptr;
    };

static void serialize(IGeometryCR geom, WCharCP fileName)
    {
    bvector<Byte> fbBytes;
    BentleyGeometryFlatBuffer::GeometryToBytes(geom, fbBytes);
    if (Check::False(fbBytes.empty(), "exported to flatbuffer"))
        GTestFileOps::WriteToFile(fbBytes, L"tmp\\native", nullptr, fileName, L"fb");
    Utf8String json;
    if (Check::True(IModelJson::TryGeometryToIModelJsonString(json, geom), "exported to json"))
        GTestFileOps::WriteToFile(json, L"tmp\\native", nullptr, fileName, L"imjs");
    };

TEST(CrossPlatform, EquivalentFormats)
    {
    bvector<TestCase> testCases;

    TestCase auxDataMesh;
    WCharCP testName = L"indexedMesh-auxData";
    auxDataMesh.m_fileNames.at(TestCase::Native).at(TestCase::FlatBuffer).push_back(TestCase::NativeRoot().AppendToPath(testName).AppendExtension(L"fb"));
    auxDataMesh.m_fileNames.at(TestCase::Native).at(TestCase::JSON).push_back(TestCase::NativeRoot().AppendToPath(testName).AppendExtension(L"imjs"));
    auxDataMesh.m_fileNames.at(TestCase::TypeScript).at(TestCase::FlatBuffer).push_back(TestCase::TypeScriptRoot().AppendToPath(testName).AppendString(L"-new").AppendExtension(L"fb"));
    auxDataMesh.m_fileNames.at(TestCase::TypeScript).at(TestCase::FlatBuffer).push_back(TestCase::TypeScriptRoot().AppendToPath(testName).AppendString(L"-old").AppendExtension(L"fb"));
    auxDataMesh.m_fileNames.at(TestCase::TypeScript).at(TestCase::JSON).push_back(TestCase::TypeScriptRoot().AppendToPath(testName).AppendExtension(L"imjs"));
    testCases.push_back(auxDataMesh);

    TestCase auxDataMesh2;  // inspired by Scientific Visualization sandbox
    testName = L"indexedMesh-auxData2";
    auxDataMesh2.m_fileNames.at(TestCase::Native).at(TestCase::FlatBuffer).push_back(TestCase::NativeRoot().AppendToPath(testName).AppendExtension(L"fb"));
    auxDataMesh2.m_fileNames.at(TestCase::Native).at(TestCase::JSON).push_back(TestCase::NativeRoot().AppendToPath(testName).AppendExtension(L"imjs"));
    auxDataMesh2.m_fileNames.at(TestCase::Native).at(TestCase::FlatBuffer).push_back(TestCase::NativeRoot().AppendToPath(testName).AppendString(L"-size3").AppendExtension(L"fb"));
    auxDataMesh2.m_fileNames.at(TestCase::Native).at(TestCase::JSON).push_back(TestCase::NativeRoot().AppendToPath(testName).AppendString(L"-size3").AppendExtension(L"imjs"));
    auxDataMesh2.m_fileNames.at(TestCase::Native).at(TestCase::FlatBuffer).push_back(TestCase::NativeRoot().AppendToPath(testName).AppendString(L"-size4").AppendExtension(L"fb"));
    auxDataMesh2.m_fileNames.at(TestCase::Native).at(TestCase::JSON).push_back(TestCase::NativeRoot().AppendToPath(testName).AppendString(L"-size4").AppendExtension(L"imjs"));
    auxDataMesh2.m_fileNames.at(TestCase::Native).at(TestCase::FlatBuffer).push_back(TestCase::NativeRoot().AppendToPath(testName).AppendString(L"-size5").AppendExtension(L"fb"));
    auxDataMesh2.m_fileNames.at(TestCase::Native).at(TestCase::JSON).push_back(TestCase::NativeRoot().AppendToPath(testName).AppendString(L"-size5").AppendExtension(L"imjs"));
    auxDataMesh2.m_fileNames.at(TestCase::TypeScript).at(TestCase::FlatBuffer).push_back(TestCase::TypeScriptRoot().AppendToPath(testName).AppendString(L"-new").AppendExtension(L"fb"));
    auxDataMesh2.m_fileNames.at(TestCase::TypeScript).at(TestCase::FlatBuffer).push_back(TestCase::TypeScriptRoot().AppendToPath(testName).AppendString(L"-old").AppendExtension(L"fb"));
    auxDataMesh2.m_fileNames.at(TestCase::TypeScript).at(TestCase::JSON).push_back(TestCase::TypeScriptRoot().AppendToPath(testName).AppendExtension(L"imjs"));
    testCases.push_back(auxDataMesh2);

    TestCase fixedSizeMesh;
    testName = L"indexedMesh-fixedSize";
    fixedSizeMesh.m_fileNames.at(TestCase::Native).at(TestCase::FlatBuffer).push_back(TestCase::NativeRoot().AppendToPath(testName).AppendExtension(L"fb"));
    fixedSizeMesh.m_fileNames.at(TestCase::Native).at(TestCase::JSON).push_back(TestCase::NativeRoot().AppendToPath(testName).AppendExtension(L"imjs"));
    fixedSizeMesh.m_fileNames.at(TestCase::TypeScript).at(TestCase::FlatBuffer).push_back(TestCase::TypeScriptRoot().AppendToPath(testName).AppendExtension(L"fb"));
    fixedSizeMesh.m_fileNames.at(TestCase::TypeScript).at(TestCase::JSON).push_back(TestCase::TypeScriptRoot().AppendToPath(testName).AppendExtension(L"imjs"));
    testCases.push_back(fixedSizeMesh);

    // TODO: add future testcases where all formats deserialize to the same geometry

    for (size_t iTestCase = 0; iTestCase < testCases.size(); ++iTestCase)
        {
        bvector<IGeometryPtr> geometry;

        // deserialize and collect all geometries
        for (auto platform : { TestCase::Native, TestCase::TypeScript })
            for (auto fileType : { TestCase::FlatBuffer, TestCase::JSON })
                for (auto& fileName : testCases[iTestCase].m_fileNames.at(platform).at(fileType))
                    {
                    IGeometryPtr geom = deserializeFirstGeom(fileName, fileType);
                    fileName.GetNameA(buf);
                    if (Check::True(geom.IsValid(), std::string("deserialized at least one geometry from ").append(buf).c_str()))
                        geometry.push_back(geom);
                    }

        // all TestCase geometries should be equivalent
        if (Check::LessThanOrEqual(4, geometry.size(), "have at least four geometries to compare"))
            {
            for (size_t i = 1; i < geometry.size(); ++i)
                {
                snprintf(buf, sizeof buf, "testCase[%zu]: geom0 compares to geom%zu", iTestCase, i);
                Check::True(geometry[0]->IsSameStructureAndGeometry(*geometry[i]), buf);
                }
            }
        }
    }

TEST(CrossPlatform, SkewSolidPrimitives)
    {
    // NOTE: some old native JSON files are empty or trivial because old serialization code returned error.
    bvector<TestCase> testCases;

    TestCase sphere0;
    WCharCP testName = L"sphere-nonuniform-scale";
    sphere0.m_fileNames.at(TestCase::Native).at(TestCase::FlatBuffer).push_back(TestCase::NativeRoot().AppendToPath(testName).AppendExtension(L"fb"));
    sphere0.m_fileNames.at(TestCase::Native).at(TestCase::JSON).push_back(TestCase::NativeRoot().AppendToPath(testName).AppendString(L"-new").AppendExtension(L"imjs"));
    sphere0.m_fileNames.at(TestCase::Native).at(TestCase::JSON).push_back(TestCase::NativeRoot().AppendToPath(testName).AppendString(L"-old").AppendExtension(L"imjs"));
    sphere0.m_fileNames.at(TestCase::TypeScript).at(TestCase::FlatBuffer).push_back(TestCase::TypeScriptRoot().AppendToPath(testName).AppendExtension(L"fb"));
    sphere0.m_fileNames.at(TestCase::TypeScript).at(TestCase::JSON).push_back(TestCase::TypeScriptRoot().AppendToPath(testName).AppendString(L"-new").AppendExtension(L"imjs"));
    sphere0.m_fileNames.at(TestCase::TypeScript).at(TestCase::JSON).push_back(TestCase::TypeScriptRoot().AppendToPath(testName).AppendString(L"-old").AppendExtension(L"imjs"));
    testCases.push_back(sphere0);

    TestCase sphere1;
    testName = L"sphere-skew-axes";
    sphere1.m_fileNames.at(TestCase::Native).at(TestCase::FlatBuffer).push_back(TestCase::NativeRoot().AppendToPath(testName).AppendExtension(L"fb"));
    sphere1.m_fileNames.at(TestCase::Native).at(TestCase::JSON).push_back(TestCase::NativeRoot().AppendToPath(testName).AppendString(L"-new").AppendExtension(L"imjs"));
    sphere1.m_fileNames.at(TestCase::Native).at(TestCase::JSON).push_back(TestCase::NativeRoot().AppendToPath(testName).AppendString(L"-old").AppendExtension(L"imjs"));
    sphere1.m_fileNames.at(TestCase::TypeScript).at(TestCase::FlatBuffer).push_back(TestCase::TypeScriptRoot().AppendToPath(testName).AppendExtension(L"fb"));
    sphere1.m_fileNames.at(TestCase::TypeScript).at(TestCase::JSON).push_back(TestCase::TypeScriptRoot().AppendToPath(testName).AppendString(L"-new").AppendExtension(L"imjs"));
    sphere1.m_fileNames.at(TestCase::TypeScript).at(TestCase::JSON).push_back(TestCase::TypeScriptRoot().AppendToPath(testName).AppendString(L"-old").AppendExtension(L"imjs"));
    testCases.push_back(sphere1);

    TestCase cone0;
    testName = L"cone-elliptical-perp";
    cone0.m_fileNames.at(TestCase::Native).at(TestCase::FlatBuffer).push_back(TestCase::NativeRoot().AppendToPath(testName).AppendExtension(L"fb"));
    cone0.m_fileNames.at(TestCase::Native).at(TestCase::JSON).push_back(TestCase::NativeRoot().AppendToPath(testName).AppendString(L"-new").AppendExtension(L"imjs"));
    cone0.m_fileNames.at(TestCase::Native).at(TestCase::JSON).push_back(TestCase::NativeRoot().AppendToPath(testName).AppendString(L"-old").AppendExtension(L"imjs"));
    cone0.m_fileNames.at(TestCase::TypeScript).at(TestCase::FlatBuffer).push_back(TestCase::TypeScriptRoot().AppendToPath(testName).AppendExtension(L"fb"));
    cone0.m_fileNames.at(TestCase::TypeScript).at(TestCase::JSON).push_back(TestCase::TypeScriptRoot().AppendToPath(testName).AppendString(L"-new").AppendExtension(L"imjs"));
    cone0.m_fileNames.at(TestCase::TypeScript).at(TestCase::JSON).push_back(TestCase::TypeScriptRoot().AppendToPath(testName).AppendString(L"-old").AppendExtension(L"imjs"));
    testCases.push_back(cone0);

    TestCase cone1;
    testName = L"cone-elliptical-skew";
    cone1.m_fileNames.at(TestCase::Native).at(TestCase::FlatBuffer).push_back(TestCase::NativeRoot().AppendToPath(testName).AppendExtension(L"fb"));
    cone1.m_fileNames.at(TestCase::Native).at(TestCase::JSON).push_back(TestCase::NativeRoot().AppendToPath(testName).AppendString(L"-new").AppendExtension(L"imjs"));
    cone1.m_fileNames.at(TestCase::Native).at(TestCase::JSON).push_back(TestCase::NativeRoot().AppendToPath(testName).AppendString(L"-old").AppendExtension(L"imjs"));
    cone1.m_fileNames.at(TestCase::TypeScript).at(TestCase::FlatBuffer).push_back(TestCase::TypeScriptRoot().AppendToPath(testName).AppendExtension(L"fb"));
    cone1.m_fileNames.at(TestCase::TypeScript).at(TestCase::JSON).push_back(TestCase::TypeScriptRoot().AppendToPath(testName).AppendString(L"-new").AppendExtension(L"imjs"));
    cone1.m_fileNames.at(TestCase::TypeScript).at(TestCase::JSON).push_back(TestCase::TypeScriptRoot().AppendToPath(testName).AppendString(L"-old").AppendExtension(L"imjs"));
    testCases.push_back(cone1);

    // temp code to generate files
    if (false)
        {
        DgnSphereDetail sphereDetail0(DPoint3d::FromZero(), RotMatrix::FromIdentity(), 1.0);
        sphereDetail0.m_localToWorld.ScaleMatrixColumns(1.0, 2.0, 3.0);
        DgnSphereDetail sphereDetail1(DPoint3d::FromZero(), RotMatrix::FromColumnVectors(DVec3d::From(1, 1, 0.5), DVec3d::From(-1, 0.3, 0.4), DVec3d::From(0.1, 0, 1)), 1.0);
        DgnConeDetail coneDetail0(DPoint3d::FromZero(), DPoint3d::From(0, 0, 1), DVec3d::From(0.1, 0.1), DVec3d::From(-0.2, 0.2), 0.1, 0.2, true);
        DgnConeDetail coneDetail1(DPoint3d::FromZero(), DPoint3d::From(0, 0, 1), DVec3d::From(0.1, 0.2), DVec3d::From(-0.1, 0.2), 0.1, 0.2, true);
        serialize(*IGeometry::Create(ISolidPrimitive::CreateDgnSphere(sphereDetail0)), L"sphere-nonuniform-scale-new");
        serialize(*IGeometry::Create(ISolidPrimitive::CreateDgnSphere(sphereDetail1)), L"sphere-skew-axes-new");
        serialize(*IGeometry::Create(ISolidPrimitive::CreateDgnCone(coneDetail0)), L"cone-elliptical-perp-new");
        serialize(*IGeometry::Create(ISolidPrimitive::CreateDgnCone(coneDetail1)), L"cone-elliptical-skew-new");
        }

    for (size_t iTestCase = 0; iTestCase < testCases.size(); ++iTestCase)
        {
        // all flatbuffer geometries should be equivalent
        bvector<IGeometryPtr> fbGeom;
        for (auto platform : { TestCase::Native, TestCase::TypeScript })
            {
            auto fileType = TestCase::FlatBuffer;
            for (auto& fileName : testCases[iTestCase].m_fileNames.at(platform).at(fileType))
                {
                IGeometryPtr geom = deserializeFirstGeom(fileName, fileType);
                fileName.GetNameA(buf);
                if (Check::True(geom.IsValid(), std::string("deserialized at least one FB geometry from ").append(buf).c_str()))
                    fbGeom.push_back(geom);
                }
            }
        if (!Check::True(1 <= fbGeom.size(), "have at least one FB geometry to compare"))
            continue;
        IGeometryPtr geomToCompare = fbGeom[0]; // ASSUME correct
        for (size_t i = 1; i < fbGeom.size(); ++i)
            {
            snprintf(buf, sizeof buf, "testCase[%zu]: fb0 compares to fb%zu", iTestCase, i);
            Check::True(geomToCompare->IsSameStructureAndGeometry(*fbGeom[i]), buf);
            }

        // all "new" json geometries should equate to geomToCompare
        for (auto platform : { TestCase::Native, TestCase::TypeScript })
            {
            auto fileType = TestCase::JSON;
            for (auto& fileName : testCases[iTestCase].m_fileNames.at(platform).at(fileType))
                {
                if (fileName.EndsWith(L"-new.imjs"))
                    {
                    IGeometryPtr geom = deserializeFirstGeom(fileName, fileType);
                    fileName.GetNameA(buf);
                    if (Check::True(geom.IsValid(), std::string("deserialized at least one JSON geometry from ").append(buf).c_str()))
                        Check::True(geomToCompare->IsSameStructureAndGeometry(*geom), std::string(buf).append("yields expected geometry").c_str());
                    }
                }
            }
        // NOTE: the analogous Typescript test verifies each property of old JSON is present in new JSON
        }
    }

TEST(CrossPlatform, IndexedMeshTopo)
    {
    bvector<TestCase> testCases;

    TestCase mesh0;
    WCharCP testName = L"indexedMeshTopo";
    mesh0.m_fileNames.at(TestCase::Native).at(TestCase::FlatBuffer).push_back(TestCase::NativeRoot().AppendToPath(testName).AppendString(L"-fixed-old").AppendExtension(L"fb"));
    mesh0.m_fileNames.at(TestCase::Native).at(TestCase::FlatBuffer).push_back(TestCase::NativeRoot().AppendToPath(testName).AppendString(L"-variable-old").AppendExtension(L"fb"));
    mesh0.m_fileNames.at(TestCase::Native).at(TestCase::JSON).push_back(TestCase::NativeRoot().AppendToPath(testName).AppendString(L"-fixed-old").AppendExtension(L"imjs"));
    mesh0.m_fileNames.at(TestCase::Native).at(TestCase::JSON).push_back(TestCase::NativeRoot().AppendToPath(testName).AppendString(L"-variable-old").AppendExtension(L"imjs"));
    /*
    mesh0.m_fileNames.at(TestCase::Native).at(TestCase::FlatBuffer).push_back(TestCase::NativeRoot().AppendToPath(testName).AppendString(L"-fixed-new").AppendExtension(L"fb"));
    mesh0.m_fileNames.at(TestCase::Native).at(TestCase::FlatBuffer).push_back(TestCase::NativeRoot().AppendToPath(testName).AppendString(L"-variable-new").AppendExtension(L"fb"));
    mesh0.m_fileNames.at(TestCase::Native).at(TestCase::JSON).push_back(TestCase::NativeRoot().AppendToPath(testName).AppendString(L"-fixed-new").AppendExtension(L"imjs"));
    mesh0.m_fileNames.at(TestCase::Native).at(TestCase::JSON).push_back(TestCase::NativeRoot().AppendToPath(testName).AppendString(L"-variable-new").AppendExtension(L"imjs"));
    */
    mesh0.m_fileNames.at(TestCase::TypeScript).at(TestCase::FlatBuffer).push_back(TestCase::TypeScriptRoot().AppendToPath(testName).AppendString(L"-old").AppendExtension(L"fb"));
    mesh0.m_fileNames.at(TestCase::TypeScript).at(TestCase::FlatBuffer).push_back(TestCase::TypeScriptRoot().AppendToPath(testName).AppendString(L"-new").AppendExtension(L"fb"));
    mesh0.m_fileNames.at(TestCase::TypeScript).at(TestCase::JSON).push_back(TestCase::TypeScriptRoot().AppendToPath(testName).AppendString(L"-old").AppendExtension(L"imjs"));
    mesh0.m_fileNames.at(TestCase::TypeScript).at(TestCase::JSON).push_back(TestCase::TypeScriptRoot().AppendToPath(testName).AppendString(L"-new").AppendExtension(L"imjs"));
    testCases.push_back(mesh0);

    // temp code to generate files
    if (false)
        {
        bvector<DPoint3d> vertices = {{0,0,0.7664205912849231}, {0,0.5,0.48180614748985123}, {0,0.25,0.802582585192784}, {0.5,0,0.43491424436272463}, {0.25,0,0.8188536774560378}, {0,0.75,0.33952742318739915}, {0.75,0,0.25206195068490395}, {0.25,0.5,0.504569375013316}, {0,1,0.2703371615911343}, {1,0,0.10755755225803061}, {0.25,0.75,0.2724132516081212}, {0.5,0.25,0.5381121104277191}, {0.5,0.5,0.3257620892806842}, {0.5,0.75,0.04371942681056798}, {0.25,1,0.2222400805896964}, {0.25,0.25,1.1652833229746615}, {1,0.25,0.23021761065562}, {0.75,0.25,0.5893585652638186}, {0.5,1,0.14597916468688915}, {0.75,0.75,0.11596980253736251}, {0.75,0.5,0.40804791637969084}, {1,0.5,0.16102556400617096}, {0.75,1,0.08104741694308121}, {1,0.75,0.050360274157937215}, {1,1,0.03586959238610449}};
        bvector<int> oneBasedIndices = {1,5,16,3,0,5,4,12,16,0,4,7,18,12,0,7,10,17,18,0,3,16,8,2,0,16,12,13,8,0,12,18,21,13,0,18,17,22,21,0,2,8,11,6,0,8,13,14,11,0,13,21,20,14,0,21,22,24,20,0,6,11,15,9,0,11,14,19,15,0,14,20,23,19,0,20,24,25,23,0};
        auto mesh0 = PolyfaceHeader::CreateIndexedMesh(1, vertices, oneBasedIndices);
        auto mesh1 = PolyfaceHeader::CreateIndexedMesh(5, vertices, oneBasedIndices); // each quad face loop has a pad; same geometry as mesh0!
        // IndexedPolyfaceWalker.buildEdgeMateIndices(mesh0);
        // IndexedPolyfaceWalker.buildEdgeMateIndices(mesh1);
        serialize(*IGeometry::Create(mesh0), L"indexedMeshTopo-variable-new");
        serialize(*IGeometry::Create(mesh1), L"indexedMeshTopo-fixed-new");
        }

    // Since native geomlibs does not yet support edgeMateIndices, all files should deserialize the same.
    // TODO: when implement native edgeMateIndex support, uncomment above, run the temp code, copy the new files to data\crossPlatform\native (and to iTwin), then uncomment below.
    for (size_t iTestCase = 0; iTestCase < testCases.size(); ++iTestCase)
        {
        bvector<IGeometryPtr> geometry;

        for (size_t iTestCase = 0; iTestCase < testCases.size(); ++iTestCase)
            {
            PolyfaceHeaderPtr mesh;
            IGeometryPtr refGeom;
            // IGeometryPtr refGeomWithTopo;
            for (auto platform : { TestCase::Native, TestCase::TypeScript }) if (refGeom.IsNull() /* || refGeomWithTopo.IsNull() */)
                {
                for (auto fileType : { TestCase::FlatBuffer, TestCase::JSON }) if (refGeom.IsNull() /* || refGeomWithTopo.IsNull() */)
                    {
                    for (auto& fileName : testCases[iTestCase].m_fileNames.at(platform).at(fileType)) if (refGeom.IsNull() /* || refGeomWithTopo.IsNull() */)
                        {
                        IGeometryPtr geom = deserializeFirstGeom(fileName, fileType);
                        mesh = geom.IsValid() ? geom->GetAsPolyfaceHeader() : nullptr;
                        if (mesh.IsValid())
                            {
                            // if (refGeomWithTopo.IsNull() && mesh->GetEdgeMateIndexCP())
                            //     refGeomWithTopo = geom;
                            if (refGeom.IsNull() /* && !mesh->GetEdgeMateIndexCP() */)
                                refGeom = geom;
                            }
                        }
                    }
                }

            if (Check::True(refGeom.IsValid(), "found ref geom") /* && Check::True(refGeomWithTopo.IsValid(), "found ref geom with topo") */)
              for (auto platform : { TestCase::Native, TestCase::TypeScript })
                  for (auto fileType : {TestCase::FlatBuffer, TestCase::JSON })
                      for (auto& fileName : testCases[iTestCase].m_fileNames.at(platform).at(fileType))
                        {
                        IGeometryPtr geom = deserializeFirstGeom(fileName, fileType);
                        fileName.GetNameA(buf);
                        if (Check::True(geom.IsValid(), std::string("deserialized at least one geom from ").append(buf).c_str()))
                            {
                            // if (fileName.Contains(L"-new."))
                            //     Check::True(refGeomWithTopo->IsSameStructureAndGeometry(*geom), std::string(buf).append("encodes expected geom + topo").c_str());
                            // else
                                Check::True(refGeom->IsSameStructureAndGeometry(*geom), std::string(buf).append("encodes expected geom").c_str());
                            }
                        }
            }
        }
    }

TEST(CrossPlatform, MeshColor)
    {
    bvector<TestCase> testCases;

    TestCase fixedSizeMesh; // has color
    WCharCP testName = L"indexedMesh-fixedSize";
    fixedSizeMesh.m_fileNames.at(TestCase::Native).at(TestCase::FlatBuffer).push_back(TestCase::NativeRoot().AppendToPath(testName).AppendExtension(L"fb"));
    fixedSizeMesh.m_fileNames.at(TestCase::Native).at(TestCase::JSON).push_back(TestCase::NativeRoot().AppendToPath(testName).AppendExtension(L"imjs"));
    fixedSizeMesh.m_fileNames.at(TestCase::TypeScript).at(TestCase::FlatBuffer).push_back(TestCase::TypeScriptRoot().AppendToPath(testName).AppendExtension(L"fb"));
    fixedSizeMesh.m_fileNames.at(TestCase::TypeScript).at(TestCase::JSON).push_back(TestCase::TypeScriptRoot().AppendToPath(testName).AppendExtension(L"imjs"));
    testCases.push_back(fixedSizeMesh);

    for (size_t iTestCase = 0; iTestCase < testCases.size(); ++iTestCase)
        {
        bvector<IGeometryPtr> geometry;

        // deserialize and collect all geometries
        for (auto platform : { TestCase::Native, TestCase::TypeScript })
            for (auto fileType : { TestCase::FlatBuffer, TestCase::JSON })
                for (auto& fileName : testCases[iTestCase].m_fileNames.at(platform).at(fileType))
                    {
                    IGeometryPtr geom = deserializeFirstGeom(fileName, fileType);
                    fileName.GetNameA(buf);
                    if (Check::True(geom.IsValid(), std::string("deserialized at least one geometry from ").append(buf).c_str()))
                        geometry.push_back(geom);
                    }

        // all TestCase geometries should be equivalent
        if (Check::LessThanOrEqual(4, geometry.size(), "have at least four geometries to compare"))
            {
            for (size_t i = 1; i < geometry.size(); ++i)
                {
                snprintf(buf, sizeof buf, "testCase[%zu]: geom0 compares to geom%zu", iTestCase, i);
                Check::True(geometry[0]->IsSameStructureAndGeometry(*geometry[i]), buf);
                }
            }

        IGeometryPtr baselineGeom = geometry[0];
        if (Check::True(baselineGeom.IsValid() && baselineGeom->GetAsPolyfaceHeader().IsValid(), "deserialized geometry is a mesh"))
            {
            auto meshWithColor = baselineGeom->GetAsPolyfaceHeader();
            if (Check::True(meshWithColor->GetColorCount() > 0, "deserialized meshes have color"))
                {
                // roundtrip through FB
                bvector<Byte> fbBytes;
                BentleyGeometryFlatBuffer::GeometryToBytes(*baselineGeom, fbBytes);
                if (Check::False(fbBytes.empty(), "exported to flatbuffer"))
                    {
                    auto geomFromFB = BentleyGeometryFlatBuffer::BytesToGeometry(fbBytes, true);
                    if (Check::True(geomFromFB.IsValid(), "imported from flatbuffer"))
                        Check::True(baselineGeom->IsSameStructureAndGeometry(*geomFromFB), "roundtrip through flatbuffer");
                    }
                // roundtrip through JSON
                Utf8String json;
                if (Check::True(IModelJson::TryGeometryToIModelJsonString(json, *baselineGeom), "exported to json"))
                    {
                    bvector<IGeometryPtr> geomsFromJson;
                    if (Check::True(IModelJson::TryIModelJsonStringToGeometry(json, geomsFromJson), "imported from json"))
                        if (Check::True(1 == geomsFromJson.size(), "imported a single geometry"))
                            Check::True(baselineGeom->IsSameStructureAndGeometry(*geomsFromJson[0]), "roundtrip through json");
                    }
                }
            }
        }
    }
