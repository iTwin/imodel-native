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

