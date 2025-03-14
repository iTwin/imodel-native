/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "testHarness.h"
#include <GeomSerialization/GeomSerializationApi.h>

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

// Verify that the native API can read flatbuffer and json files written by the typescript and native APIs.
// Each test case consists of at least four files that encode the same single geometry: native-authored fb and json, typescript-authored fb and json
// The same test exists in typescript core-geometry and operates on the same data files; these tests and data should be kept in sync.
TEST(CrossPlatform, IndexedMeshAuxData)
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

    // TODO: add other test cases

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
        auto serialize = [](IGeometryCR geom, WCharCP fileName) -> void {
            bvector<Byte> fbBytes;
            BentleyGeometryFlatBuffer::GeometryToBytes(geom, fbBytes);
            if (Check::False(fbBytes.empty(), "exported to flatbuffer"))
                GTestFileOps::WriteToFile(fbBytes, L"tmp\\native", nullptr, fileName, L"fb");
            Utf8String json;
            if (Check::True(IModelJson::TryGeometryToIModelJsonString(json, geom), "exported to json"))
                GTestFileOps::WriteToFile(json, L"tmp\\native", nullptr, fileName, L"imjs");
            };
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