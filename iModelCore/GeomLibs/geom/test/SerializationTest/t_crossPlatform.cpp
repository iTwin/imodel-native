/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "testHarness.h"
#include <GeomSerialization/GeomSerializationApi.h>

// this fixup is not needed in the TypeScript crossPlatform test because only native API deserializes in fixed-size mesh format
static void convertNativeMeshToVariableSized(IGeometryR geom)
    {
    auto mesh = geom.GetAsPolyfaceHeader();
    if (mesh.IsValid() && !mesh->IsVariableSizeIndexed())
        Check::True(mesh->ConvertToVariableSizeSignedOneBasedIndexedFaceLoops(), "successfully converted mesh from fixed to variable sized index blocking");
    };

// Verify that the native API can read flatbuffer and json files written by the typescript and native APIs.
// Each test case consists of at least four files that encode the same single geometry: native-authored fb and json, typescript-authored fb and json
// The same test exists in typescript core-geometry and operates on the same data files; these tests and data should be kept in sync.
TEST(CrossPlatform, Equivalence)
    {
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
        };

    BeFileName root;
    BeTest::GetHost().GetDocumentsRoot(root);
    root.AppendToPath(L"GeomLibsTestData").AppendToPath(L"crossPlatform");
    BeFileName nativeRoot(root);
    nativeRoot.AppendToPath(L"native");
    BeFileName typeScriptRoot(root);
    typeScriptRoot.AppendToPath(L"typescript");

    bvector<TestCase> testCases;

    TestCase auxDataMesh;
    WCharCP testName = L"indexedMesh-auxData";
    auxDataMesh.m_fileNames.at(TestCase::Native).at(TestCase::FlatBuffer).push_back(BeFileName(nativeRoot).AppendToPath(testName).AppendExtension(L"fb"));
    auxDataMesh.m_fileNames.at(TestCase::Native).at(TestCase::JSON).push_back(BeFileName(nativeRoot).AppendToPath(testName).AppendExtension(L"imjs"));
    auxDataMesh.m_fileNames.at(TestCase::TypeScript).at(TestCase::FlatBuffer).push_back(BeFileName(typeScriptRoot).AppendToPath(testName).AppendString(L"-new").AppendExtension(L"fb"));
    auxDataMesh.m_fileNames.at(TestCase::TypeScript).at(TestCase::FlatBuffer).push_back(BeFileName(typeScriptRoot).AppendToPath(testName).AppendString(L"-old").AppendExtension(L"fb"));
    auxDataMesh.m_fileNames.at(TestCase::TypeScript).at(TestCase::JSON).push_back(BeFileName(typeScriptRoot).AppendToPath(testName).AppendExtension(L"imjs"));
    testCases.push_back(auxDataMesh);

    TestCase auxDataMesh2;  // inspired by Scientific Visualization sandbox
    testName = L"indexedMesh-auxData2";
    auxDataMesh2.m_fileNames.at(TestCase::Native).at(TestCase::FlatBuffer).push_back(BeFileName(nativeRoot).AppendToPath(testName).AppendExtension(L"fb"));
    auxDataMesh2.m_fileNames.at(TestCase::Native).at(TestCase::JSON).push_back(BeFileName(nativeRoot).AppendToPath(testName).AppendExtension(L"imjs"));
    auxDataMesh2.m_fileNames.at(TestCase::Native).at(TestCase::FlatBuffer).push_back(BeFileName(nativeRoot).AppendToPath(testName).AppendString(L"-size3").AppendExtension(L"fb"));
    auxDataMesh2.m_fileNames.at(TestCase::Native).at(TestCase::JSON).push_back(BeFileName(nativeRoot).AppendToPath(testName).AppendString(L"-size3").AppendExtension(L"imjs"));
    auxDataMesh2.m_fileNames.at(TestCase::Native).at(TestCase::FlatBuffer).push_back(BeFileName(nativeRoot).AppendToPath(testName).AppendString(L"-size4").AppendExtension(L"fb"));
    auxDataMesh2.m_fileNames.at(TestCase::Native).at(TestCase::JSON).push_back(BeFileName(nativeRoot).AppendToPath(testName).AppendString(L"-size4").AppendExtension(L"imjs"));
    auxDataMesh2.m_fileNames.at(TestCase::Native).at(TestCase::FlatBuffer).push_back(BeFileName(nativeRoot).AppendToPath(testName).AppendString(L"-size5").AppendExtension(L"fb"));
    auxDataMesh2.m_fileNames.at(TestCase::Native).at(TestCase::JSON).push_back(BeFileName(nativeRoot).AppendToPath(testName).AppendString(L"-size5").AppendExtension(L"imjs"));
    auxDataMesh2.m_fileNames.at(TestCase::TypeScript).at(TestCase::FlatBuffer).push_back(BeFileName(typeScriptRoot).AppendToPath(testName).AppendString(L"-new").AppendExtension(L"fb"));
    auxDataMesh2.m_fileNames.at(TestCase::TypeScript).at(TestCase::FlatBuffer).push_back(BeFileName(typeScriptRoot).AppendToPath(testName).AppendString(L"-old").AppendExtension(L"fb"));
    auxDataMesh2.m_fileNames.at(TestCase::TypeScript).at(TestCase::JSON).push_back(BeFileName(typeScriptRoot).AppendToPath(testName).AppendExtension(L"imjs"));
    testCases.push_back(auxDataMesh2);

    TestCase fixedSizeMesh;
    testName = L"indexedMesh-fixedSize";
    fixedSizeMesh.m_fileNames.at(TestCase::Native).at(TestCase::FlatBuffer).push_back(BeFileName(nativeRoot).AppendToPath(testName).AppendExtension(L"fb"));
    fixedSizeMesh.m_fileNames.at(TestCase::Native).at(TestCase::JSON).push_back(BeFileName(nativeRoot).AppendToPath(testName).AppendExtension(L"imjs"));
    fixedSizeMesh.m_fileNames.at(TestCase::TypeScript).at(TestCase::FlatBuffer).push_back(BeFileName(typeScriptRoot).AppendToPath(testName).AppendExtension(L"fb"));
    fixedSizeMesh.m_fileNames.at(TestCase::TypeScript).at(TestCase::JSON).push_back(BeFileName(typeScriptRoot).AppendToPath(testName).AppendExtension(L"imjs"));
    testCases.push_back(fixedSizeMesh);



    // TODO: add other test cases

    bvector<IGeometryPtr> geometry;
    char buf[MAX_PATH];
    auto pushFirstDeserializedGeom = [&](BeFileNameCR fileName, TestCase::FileType fileType) -> void
        {
        fileName.GetNameA(buf);
        bvector<IGeometryPtr> geom;
        bool status = (TestCase::FileType::FlatBuffer == fileType) ? GTestFileOps::FlatBufferFileToGeometry(fileName, geom) : GTestFileOps::JsonFileToGeometry(fileName, geom);
        if (Check::True(status && geom.size() > 0, std::string("deserialized at least one geometry from ").append(buf).c_str()))
            {
            convertNativeMeshToVariableSized(*geom[0]);
            geometry.push_back(geom[0]);
            }
        };

    for (size_t iTestCase = 0; iTestCase < testCases.size(); ++iTestCase)
        {
        geometry.clear();
        for (auto platform : { TestCase::Native, TestCase::TypeScript })
            {
            for (auto fileType : { TestCase::FlatBuffer, TestCase::JSON })
                {
                for (auto& fileName : testCases[iTestCase].m_fileNames.at(platform).at(fileType))
                    pushFirstDeserializedGeom(fileName, fileType);
                }
            }
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

TEST(CrossPlatform, TestCaseCtor)
    {
    bvector<DPoint3d> vertices = {{0,0,0.7664205912849231}, {0,0.5,0.48180614748985123}, {0,0.25,0.802582585192784}, {0.5,0,0.43491424436272463}, {0.25,0,0.8188536774560378}, {0,0.75,0.33952742318739915}, {0.75,0,0.25206195068490395}, {0.25,0.5,0.504569375013316}, {0,1,0.2703371615911343}, {1,0,0.10755755225803061}, {0.25,0.75,0.2724132516081212}, {0.5,0.25,0.5381121104277191}, {0.5,0.5,0.3257620892806842}, {0.5,0.75,0.04371942681056798}, {0.25,1,0.2222400805896964}, {0.25,0.25,1.1652833229746615}, {1,0.25,0.23021761065562}, {0.75,0.25,0.5893585652638186}, {0.5,1,0.14597916468688915}, {0.75,0.75,0.11596980253736251}, {0.75,0.5,0.40804791637969084}, {1,0.5,0.16102556400617096}, {0.75,1,0.08104741694308121}, {1,0.75,0.050360274157937215}, {1,1,0.03586959238610449}};
    bvector<int> oneBasedIndices = {1,5,16,3,0,5,4,12,16,0,4,7,18,12,0,7,10,17,18,0,3,16,8,2,0,16,12,13,8,0,12,18,21,13,0,18,17,22,21,0,2,8,11,6,0,8,13,14,11,0,13,21,20,14,0,21,22,24,20,0,6,11,15,9,0,11,14,19,15,0,14,20,23,19,0,20,24,25,23,0};
    auto mesh = PolyfaceHeader::CreateIndexedMesh(1, vertices, oneBasedIndices);    // could do 1 and 5 for 
    if (Check::True(mesh.IsValid(), "created mesh"))
        {
            BentleyGeometryFlatBuffer::GeometryToBytes (PolyfaceQueryCR polyfaceQuery, bvector<Byte>& buffer)

        const fbBytes = BentleyGeometryFlatBuffer.geometryToBytes(mesh, true);
        if (ck.testDefined(fbBytes, "exported to flatbuffer"))
        GeometryCoreTestIO.writeBytesToFile(fbBytes, "c:\\tmp\\typescript\\indexedMesh-topo-new.fb");

        const json = IModelJson.Writer.toIModelJson(mesh);
        fs.writeFileSync("c:\\tmp\\typescript\\indexedMesh-topo-new.imjs", JSON.stringify(json));
    }
    expect(ck.getNumErrors()).toBe(0);
    }
