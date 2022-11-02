/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "testHarness.h"
#include <stdio.h>

#include "SampleGeometry.h"
#include <BeJsonCpp/BeJsonUtilities.h>
#include "FileOps.h"
#include <GeomSerialization/GeomSerializationApi.h>


bool ReadIModelJson
(
bvector<IGeometryPtr> &geometry,
WCharCP inputDirectory,
WCharCP outputDirectory,
WCharCP nameB,
WCharCP nameC,
WCharCP extension,
WCharCP flatbufferBinaryExtension = nullptr,           // optionally mirror every file to flatbuffer and write as binary file.  NOT IMPLEMENTED
WCharCP flatbufferUInt8Extension = nullptr  // optionall write flatbuffer to string "[i0,i1,...]"
)
    {
    geometry.clear ();
    BeFileName dataPath;
    BeTest::GetHost().GetDocumentsRoot(dataPath);
    dataPath.AppendToPath (L"GeomLibsTestData");
    if (inputDirectory)
        dataPath.AppendToPath (inputDirectory);
    if (nameB)
        dataPath.AppendToPath (nameB);
    if (nameC)
        dataPath.AppendToPath (nameC);
    if (extension)
        dataPath.AppendExtension (extension);
    bool stat = false;
    Check::StartScope ("ReadIModelJson");
    Utf8String string;
    printf (" ReadIModelJson file %ls\n", dataPath.c_str ());

    if (Check::True (GTestFileOps::ReadAsString (dataPath, string), "Read file to string"))
        {
        if (Check::True (IModelJson::TryIModelJsonStringToGeometry (string, geometry), "json string to geometry"))
            {
            Utf8String stringB;
            if (IModelJson::TryGeometryToIModelJsonString (stringB, geometry))
                GTestFileOps::WriteToFile (stringB, L"IModelJsonFromNative", nameB, nameC, extension);
            stat = true;

            if (flatbufferUInt8Extension != nullptr)
                {
                bvector<Byte> bytes;
                BentleyGeometryFlatBuffer::GeometryToBytes (geometry, bytes);
                GTestFileOps::WriteByteArrayToTextFile(bytes, L"IModelJsonFromNative", nameB, nameC, flatbufferUInt8Extension);

                }
            }
        else
            printf (" Failed file \n%ls\n", dataPath.c_str ());
        }
    Check::EndScope();
    return stat;
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(IModelJson,ReadFiles)
    {
    for (auto & name :
        bvector<CharCP> {
            "arc1",
            "lineSegment5",
            "arc",
            "lineSegment",
            "lineString",
            "pointString",
            "path",
            "loop",
            "parityRegion",
            "bcurve",
            "cylinder",
            "cone",
            "box",
            "sphere",
            "torusPipe",
            "linearSweep",
            "rotationalSweep",
            "ruledSweep",
            "bsurf",
            "bsurfWithBoundary",
            "indexedMesh",
            })
        {
        bvector<IGeometryPtr> geometry;
        WString wName;
        wName.AppendA(name);
        if (Check::True(ReadIModelJson (geometry, L"IModelJson", L"IModelJsonFromNative", wName.c_str(), nullptr, L"imjs", nullptr, L"fbjs"), "read imjs file"))
            {
            for (auto const& g : geometry)
                {
                if (Check::True(g.IsValid(), "Imported geometry is valid"))
                    Check::NearRoundtrip(*g, 0.0, name);
                }
            }
        }
    }
static  bvector<Byte>  jsSegmentBytes {
98,103,48,48,48,49,102,98,4,0,0,0,144,255,255,255,12,0,0,0,0,15,
6,0,8,0,4,0,6,0,0,0,4,0,0,0,2,0,0,0,92,0,0,0,12,0,0,0,8,0,12,0,
11,0,4,0,8,0,0,0,8,0,0,0,0,0,0,1,182,255,255,255,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,8,64,0,0,0,0,0,0,
8,64,0,0,0,0,0,0,0,0,0,0,0,0,8,0,10,0,9,0,4,0,8,0,0,0,12,0,0,0,0,
1,6,0,4,0,4,0,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,8,64,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
TEST(IModelJson, JsonFlatBuffer)
    {
    bvector<IGeometryPtr> geometry;
    BentleyGeometryFlatBuffer::BytesToVectorOfGeometry(jsSegmentBytes, geometry);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(IModelJson,LineSegmentInCurveCollection)
    {
    auto cv = CurveVector::Create (CurveVector::BOUNDARY_TYPE_None);
    cv->Add (ICurvePrimitive::CreateLine (DSegment3d::From (1,2,3, 4,5,6)));
    Check::SaveTransformed (*cv);
    Check::ClearGeometry ("IModelJson.LineSegmentInCurveCollection");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(IModelJson, BlockedPolyface)
    {
    auto fixedMesh3 = UnitGridPolyface (DPoint3dDVec3dDVec3d (
        DPoint3d::From (0,0,0),
        DVec3d::From (2,0,0),
        DVec3d::From (1,2,0)), 4, 5, true, false);
    auto fixedMesh4 = UnitGridPolyface(DPoint3dDVec3dDVec3d(
        DPoint3d::From(0, 0, 0),
        DVec3d::From(2, 0, 0),
        DVec3d::From(1, 2, 0)), 4, 5, false, false);
    auto json3 = Json::Value();
    auto json4 = Json::Value();
    Check::True (IModelJson::TryGeometryToIModelJsonValue(BeJsValue(json3), *IGeometry::Create (fixedMesh3)), "fixedmesh3 to json");
    Check::True (IModelJson::TryGeometryToIModelJsonValue(BeJsValue(json4), *IGeometry::Create(fixedMesh4)), "fixedmesh4 to json");
    fixedMesh3->ConvertToVariableSizeSignedOneBasedIndexedFaceLoops();
    fixedMesh4->ConvertToVariableSizeSignedOneBasedIndexedFaceLoops();
    bvector<IGeometryPtr> indexed3, indexed4;
    Check::True (IModelJson::TryIModelJsonValueToGeometry (BeJsValue(json3), indexed3), "json3 to geometry");
    Check::True(IModelJson::TryIModelJsonValueToGeometry(BeJsValue(json4), indexed4), "json4 to geometry");

    Check::ClearGeometry("IModelJson.BlockedPolyface");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Cone, AnnulusJSON)
    {
    auto centerA = DPoint3d::From(1, 0, 0);
    auto centerB = DPoint3d::From(1, 0, 0);
    auto matrix = RotMatrix::FromRowValues(
        0, 0, 1,
        1, 0, 0,
        0, 1, 0
    );
    auto coneA = ISolidPrimitive::CreateDgnCone(
        DgnConeDetail(centerA, centerB, matrix,
            1.0, 0.5, false
        ));
    auto jsonA = Json::Value();
    Check::True(IModelJson::TryGeometryToIModelJsonValue(BeJsValue(jsonA), *IGeometry::Create(coneA)));
    bvector<IGeometryPtr> geometryB;
    Check::True(IModelJson::TryIModelJsonValueToGeometry(BeJsValue(jsonA), geometryB));
    auto jsonB = Json::Value();
    Check::True(IModelJson::TryGeometryToIModelJsonValue(BeJsValue(jsonB), geometryB));
    Check::SaveTransformed(*coneA);
    for (auto &g : geometryB)
        Check::SaveTransformed(g);
    Check::ClearGeometry("Cone.AnnulusJSON");
    }

bool testIsAlmostEqual(BeJsConst a, BeJsConst b)
    {
    return a.isAlmostEqual (b);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(BeJs,isAlmostEqual)
    {
    double e0 = 1.0e-18;
    double e1 = 1.0e-8;
    for (double sign : {-1.0, 1.0})
        { 
        for (double x0 = 1.0e-10; x0 < 1.0e20; x0 *= 10.0)
            {
            double x = sign * x0;
            Check::True (testIsAlmostEqual (BeJsConst (x), BeJsConst (x)));
            Check::True(testIsAlmostEqual(BeJsConst(x), BeJsConst(x - e0)));
            Check::True(testIsAlmostEqual(BeJsConst(x), BeJsConst(x + e0)));

            Check::True(testIsAlmostEqual(BeJsConst(x), BeJsConst(x * (1.0 - e0))));
            Check::True(testIsAlmostEqual(BeJsConst(x), BeJsConst(x * (1.0 + e0))));

            Check::False(testIsAlmostEqual (BeJsConst (x), BeJsConst (x * (1.0 + e1) + sign * e1)));
            Check::False(testIsAlmostEqual(BeJsConst(x), BeJsConst(x * (1.0 - e1) - sign * e1)));
            }
        }
    }
TEST(Serialization, ExpectedClosure)
    {
    auto meshA = CreatePolyface_ExtrudedL(0, 0, 3, 4, 2, 6, 2);
    Check::True(meshA->IsClosedByEdgePairing(), "verify closure");
    for (uint32_t closure : {0, 1, 2, 0, 2, 1})
        {
        Check::StartScope("closure case", (size_t)closure);
        meshA->SetExpectedClosure(closure);
        Check::Int(closure, meshA->GetExpectedClosure());
        {
        auto jsonA = Json::Value();
        Check::True(IModelJson::TryGeometryToIModelJsonValue(BeJsValue(jsonA), *IGeometry::Create(meshA)), "geomery to json");
        bvector<IGeometryPtr> geometryB;
        Check::True(IModelJson::TryIModelJsonValueToGeometry(BeJsValue(jsonA), geometryB), "json to geometry");
        if (Check::Size(1, geometryB.size()), "singleton from json")
            {
            auto meshB = geometryB[0]->GetAsPolyfaceHeader ();
            if (Check::True(meshB.IsValid()), "singleton mesh")
                {
                Check::Int(closure, meshB->GetExpectedClosure(), "expectdClosure json");
                }
            }
        }

        {
        bvector<Byte> bytes;
        BentleyGeometryFlatBuffer::GeometryToBytes(*IGeometry::Create(meshA), bytes);
        bvector<IGeometryPtr> geometryC;
        BentleyGeometryFlatBuffer::BytesToVectorOfGeometry(bytes, geometryC);
        if (Check::Size(1, geometryC.size()), "singleton from flatbuffer")
            {
            auto meshC = geometryC[0]->GetAsPolyfaceHeader();
            if (Check::True(meshC.IsValid()), "singleton mesh")
                {
                Check::Int(closure, meshC->GetExpectedClosure(), "expectdClosure fb");
                }
            }
        }
        Check::EndScope();
        }
    }

// ASSUME arrayA and arrayB are references within dataA and dataB
// ASSUME value0 and value1 are distinct.
// ASSUME dataA and dataB are identical initially.
template <typename T>
void CheckArrayActivity(
    TaggedNumericData &dataA,
    TaggedNumericData &dataB,
    bvector<T> &arrayA,
    bvector<T> &arrayB,
    T value0,
    T value1
)
    {
    Check::True (TaggedNumericData::AreSameStructureAndGeometry (dataA, dataB), "verify initial condition");
    arrayA.push_back (value0);
    Check::False(TaggedNumericData::AreSameStructureAndGeometry (dataA, dataB), "expect unequal length");
    arrayB.push_back(value0);
    Check::True(TaggedNumericData::AreSameStructureAndGeometry (dataA, dataB), "verify equal length and contents");
    arrayB.back () = value1;
    Check::False(TaggedNumericData::AreSameStructureAndGeometry (dataA, dataB), "expect unequal contents");
    arrayA.back () = value1;
    Check::True(TaggedNumericData::AreSameStructureAndGeometry (dataA, dataB), "restore equal length and contents");
    BeJsDocument json;
    IModelJson::TaggedNumericDataToJson(json, dataA);
    TaggedNumericData dataQ (0,0);
    if (Check::True(IModelJson::TryIModelJsonValueToTaggedNumericData(json, dataQ), "to json"))
        {
        Check::True (TaggedNumericData::AreSameStructureAndGeometry (dataA, dataQ), "round trip json");
        }

    auto tetrahedron = PolyfaceHeader::CreateRegularPolyhedron(1, 0);
    tetrahedron->SetNumericTags (dataA);
    auto g = IGeometry::Create(tetrahedron);
    bvector<Byte> bytes;
    BentleyGeometryFlatBuffer::GeometryToBytes(*g, bytes);
    bvector<IGeometryPtr> geometryC;
    BentleyGeometryFlatBuffer::BytesToVectorOfGeometry(bytes, geometryC);
    if (Check::Size(1, geometryC.size(), "singleton fb round trip"))
        {
        auto meshC = geometryC.front()->GetAsPolyfaceHeader();
        Check::True(meshC.IsValid(), "mesh in round trip");
        Check::True(TaggedNumericData::AreSameStructureAndGeometry(
                    tetrahedron->GetNumericTagsCP (), meshC->GetNumericTagsCP ()));
        }
    }

TEST(Serialization, TaggedNumericDataIMJS)
    {
    TaggedNumericData dataA (1,2);


    TaggedNumericData dataC (20,0);

    TaggedNumericData dataD (20,0);
    dataD.m_intData.push_back (100);
    dataD.m_intData.push_back(200);

    dataD.m_doubleData.push_back (-0.2);
    dataD.m_doubleData.push_back(35.0);
    dataD.m_doubleData.push_back(1.23e5);
    for (auto & data : { dataA, dataC, dataD })
        {
        BeJsDocument json;
        IModelJson::TaggedNumericDataToJson(json, data);
        TaggedNumericData dataZ;
        IModelJson::TryIModelJsonValueToTaggedNumericData(json, dataZ);
        auto string = json.Stringify();
        printf("%s\n", string.c_str());
        Check::True (TaggedNumericData::AreSameStructureAndGeometry(data, dataZ), "IsSameStructureAndGeometry (json RT)");
        }
    Check::False(TaggedNumericData::AreSameStructureAndGeometry (dataA, dataC), "IsSameStructureAndGeometry (other)");
    Check::False(TaggedNumericData::AreSameStructureAndGeometry (dataA, dataD), "IsSameStructureAndGeometry (other)");
    Check::False(TaggedNumericData::AreSameStructureAndGeometry (dataC, dataD), "IsSameStructureAndGeometry (other)");
    Check::False(TaggedNumericData::AreSameStructureAndGeometry (dataD, dataC), "IsSameStructureAndGeometry (other)");
    auto dataA1 = dataA;
    CheckArrayActivity (dataA, dataA1, dataA.m_intData, dataA1.m_intData, 1,2);
    dataA1 = dataA;
    CheckArrayActivity(dataA, dataA1, dataA.m_doubleData, dataA1.m_doubleData, 1.4, 2.1);
    dataA1 = dataA;
    // CheckArrayActivity(dataA, dataA1, dataA.m_geometry, dataA1.m_geometry, gNull, g2);

    }

PolyfaceHeaderPtr RoundTripMeshFB(PolyfaceHeaderPtr & meshA)
    {
    bvector<Byte> bytes;
    BentleyGeometryFlatBuffer::GeometryToBytes(*IGeometry::Create(meshA), bytes);
    bvector<IGeometryPtr> geometryC;
    BentleyGeometryFlatBuffer::BytesToVectorOfGeometry(bytes, geometryC);
    if (Check::Size(1, geometryC.size(), "singleton from flatbuffer"))
        {
        auto meshC = geometryC[0]->GetAsPolyfaceHeader();
        if (Check::True(meshC.IsValid(), "singleton mesh"))
            {
            Check::True(meshC->IsSameStructureAndGeometry(*meshA, 0.0), "RoundTrip");
            return meshC;
            }
        }
    return nullptr;
    }

PolyfaceHeaderPtr RoundTripMeshQueryCarrier(PolyfaceHeaderPtr & meshA)
    {
    bvector<Byte> bytes;
    BentleyGeometryFlatBuffer::GeometryToBytes(*IGeometry::Create(meshA), bytes);
    bvector<IGeometryPtr> geometryC;
    PolyfaceQueryCarrier carrier(0, false, 0, 0, nullptr, nullptr);
    if (Check::True (BentleyGeometryFlatBuffer::BytesToPolyfaceQueryCarrier(
                    bytes.data (),
                    carrier, false), "Bytes to QueryCarrier"))
        {
        auto meshC = carrier.CloneAsVariableSizeIndexed (carrier);
        if (Check::True (meshC.IsValid ()))
            Check::True(meshA->IsSameStructureAndGeometry(*meshC, 0.0), "RoundTrip through query carrier");
        return meshC;
        }
    return nullptr;
    }

PolyfaceHeaderPtr RoundTripMeshIMJS(PolyfaceHeaderPtr & meshA)
    {
    auto jsonA = Json::Value();
    Check::True(IModelJson::TryGeometryToIModelJsonValue(BeJsValue(jsonA), 
            *IGeometry::Create(meshA)), "geomery to json");
    bvector<IGeometryPtr> geometryB;
    if (Check::True(IModelJson::TryIModelJsonValueToGeometry(BeJsValue(jsonA), geometryB), "json to geometry"))
        {
        auto meshB = geometryB.front()->GetAsPolyfaceHeader();
        Check::True(meshB.IsValid () && meshB->IsSameStructureAndGeometry(*meshA,
            0.0), "RoundTrip");
        return meshB;
        }
    return nullptr;
    }


TEST(Serialization, TaggedNumericDataOnPolyface)
    {
    auto meshA = PolyfaceHeader::CreateRegularPolyhedron(1, 0);
    auto meshA1 = RoundTripMeshIMJS(meshA);
    auto meshB = meshA->Clone ();
    meshB->SetNumericTags (TaggedNumericData (-1000,0));
    Check::False(meshB->IsSameStructureAndGeometry(*meshA, 0.0), "Detect tagged data");
    Check::True(meshB->IsSameStructureAndGeometry(*meshB, 0.0), "Identity with tagged data");
    auto meshB1 = RoundTripMeshFB(meshB);
    auto meshB2 = RoundTripMeshQueryCarrier (meshB);
    }

TEST(Serialization, TrimmedBSurface)
    {
    auto meshA = PolyfaceHeader::CreateRegularPolyhedron(1, 0);
    auto meshA1 = RoundTripMeshIMJS(meshA);
    auto meshB = meshA->Clone();
    meshB->SetNumericTags(TaggedNumericData(-1000, 0));
    Check::False(meshB->IsSameStructureAndGeometry(*meshA, 0.0), "Detect tagged data");
    Check::True(meshB->IsSameStructureAndGeometry(*meshB, 0.0), "Identity with tagged data");
    auto meshB1 = RoundTripMeshFB(meshB);
    auto meshB2 = RoundTripMeshQueryCarrier(meshB);
    }


void DoRoundTrip(IGeometryPtr g0, bool emitGeometry, int serializerSelect);
TEST(Serialization, BlockedPolyface)
    {
    bvector<DPoint3d> points;
    bvector<int> indices;
    int numBlock = 4;
    for (int i = 0; i <= numBlock; i++)
        {
        points.push_back (DPoint3d::From (i,0,0));
        points.push_back (DPoint3d::From (i,1,0));
        }
    for (int i = 0; i < numBlock; i++)
        {
        indices.push_back (1 + 2 * i);
        indices.push_back(1 + 2 * i + 2);
        indices.push_back(1 + 2 * i + 3);
        indices.push_back(1 + 2 * i + 1);
        }
    auto polyface = PolyfaceHeader::CreateIndexedMesh(4, points, indices);
    auto g = IGeometry::Create (polyface);
    DoRoundTrip(g, false, 0);
    DoRoundTrip (g, false, 1);
    Check::SaveTransformed (*polyface);
    }