/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "testHarness.h"
#include <stdio.h>
static size_t s_maxPolyfacePrint = 2;
extern IFacetOptionsPtr CreateFacetOptions(bool normals = true, bool params = true, bool edgeChains = true);
void ExaminePolyface(PolyfaceHeaderR mesh, char const* title);

static double sCallerSize = 4.0;
void TestPolyfaceConstructionTriangulation(int numPerFace, bool convexRequired)
    {
    IFacetOptionsPtr options = CreateFacetOptions();
    options->SetMaxPerFace(numPerFace);
    options->SetConvexFacetsRequired(convexRequired);
    options->SetParamsRequired(true);
    options->SetEdgeChainsRequired(true);
    IPolyfaceConstructionPtr builder = IPolyfaceConstruction::Create(*options);
    double mySize = sCallerSize;

    // Regular polygons ...
    double r = mySize / 4.0;
    double z = 0.0;
    double deltaZ = 0.5 * mySize;
    double deltaX = 2.5 * r;
    SaveAndRestoreCheckTransform shifter1(0.0, 20.0 * r, 0);
    double bigStep = 30.0 * r;
    double oneStep = 4.0 * r;
    for (size_t numPoint = 3; numPoint < 10; numPoint++, z += deltaZ)
        {
        SaveAndRestoreCheckTransform shifter2(bigStep, 0, 0);

        double dTheta = msGeomConst_2pi / numPoint;
        bvector<DPoint3d> points;
        for (size_t i = 0; i < numPoint; i++)
            {
            double theta = i * dTheta;
            points.push_back(DPoint3d::FromXYZ(r * cos(theta), r * sin(theta), z));
            }
        char title[200];
        builder->Clear();
        builder->AddTriangulation(points);
        sprintf(title, "TriangulatedRegularPolygon%d", (int)numPoint);
        ExaminePolyface(builder->GetClientMeshR(), title);
        Check::SaveTransformed(builder->GetClientMeshR());
        Check::Shift(oneStep, 0, 0);
        SaveEdgeChains(builder->GetClientMeshR(), true);
        size_t numFacet = builder->GetClientMeshR().GetNumFacet();
        if (numPerFace == 3)
            Check::Size(numFacet, numPoint - 2);
        else
            Check::True(numFacet <= numPoint - 2);

        Check::Shift(2.0 * oneStep, 0, 0);
        // Add origin as final point -- figure becomes pie with one piece missing
        points.push_back(DPoint3d::FromXYZ(0, 0, z));
        for (size_t i = 0; i < points.size(); i++)
            points[i].x += deltaX;
        builder->Clear();
        builder->AddTriangulation(points);
        sprintf(title, "TriangulatedRegularPolygon%d_minusOneSlice", (int)numPoint);
        ExaminePolyface(builder->GetClientMeshR(), title);
        Check::SaveTransformed(builder->GetClientMeshR());
        Check::Shift(oneStep, 0, 0);
        SaveEdgeChains(builder->GetClientMeshR(), true);
        numFacet = builder->GetClientMeshR().GetNumFacet();
        if (numPerFace == 3)
            Check::Size(numFacet, numPoint - 1);
        else
            Check::True(numFacet <= numPoint - 1);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(PolyfaceConstruction, Triangulation)
    {
    // unnecessary shift -- but it makes the regression files match
    Check::Shift(0, 70, 0);
    TestPolyfaceConstructionTriangulation(3, false);
    TestPolyfaceConstructionTriangulation(4, false);
    TestPolyfaceConstructionTriangulation(40, false);
    TestPolyfaceConstructionTriangulation(40, true);
    Check::ClearGeometry("PolyfaceConstruction.Triangulation");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(PolyfaceConstruction, SpaceTriangulation)
    {
    bvector<DPoint3d> points{
        DPoint3d::From(7.9409722328896093, 0.32205006789987412, 0.70928575018344764),
        DPoint3d::From(7.9409722328896093, 0.32205006789987412, 0.83628575018344764),
        DPoint3d::From(7.8390560117188715, 0.31791680499400471, 0.83628575018344764),
        DPoint3d::From(7.8390560117188715, 0.31791680499400471, 0.70928575018344764),
        DPoint3d::From(7.9409722328896093, 0.32205006789987412, 0.70928575018344764)
        };

    IFacetOptionsPtr options = CreateFacetOptions();
    options->SetMaxPerFace(3);
    options->SetParamsRequired(true);
    IPolyfaceConstructionPtr builder = IPolyfaceConstruction::Create(*options);
    // double mySize = SetTransformToNewGridSpot (*builder, true);
    builder->Clear();
    builder->AddTriangulation(points);
    Check::ClearGeometry("PolyfaceConstruction.SpaceTriangulation");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(PolyfaceConstruction, NearlyColinearTriangulation)
    {
    // Under vu_triangulateXYPolygonExt2, this nearly colinear polygon yields a quad under
    // vertex clustering that gets pinched by edge splitting into 3 null faces and a 6-sided
    // exterior face whose area is 5.6e-38. (The exterior face should be negative but vertex
    // clustering causes sign flip.) We expect empty triangulation on this pathological polygon.
    bvector<DPoint3d> points{
        DPoint3d::From(-8.5698957263957709e-08, -2.0689913071691990e-07),
        DPoint3d::From(6.2451617850456387e-06, -2.8292270144447684e-06),
        DPoint3d::From(6.1436046053131577e-05, -2.5690078473417088e-05),
        DPoint3d::From(6.1436090618371964e-05, -2.5689969334052876e-05),
        DPoint3d::From(0.23096993103536079, -0.095670742452057311, 8.8817841970012523e-16),
        DPoint3d::From(0.23096993103536079, -0.095670742452057311, 8.1412832031446669e-10),
        DPoint3d::From(-8.5695319285150617e-08, -2.0691368263214827e-07, 8.1360695958210272e-10),
        };
    Check::SaveTransformed(points);

    bool isColinear = PolylineOps::IsColinear(points);
    Check::False(isColinear, "Polygon is not colinear.");

    double area = PolygonOps::AreaXY(points);
    Check::True(fabs(area) < 1.0e-12, "Polygon has miniscule area.");

    IFacetOptionsPtr options = CreateFacetOptions();
    options->SetMaxPerFace(3);
    IPolyfaceConstructionPtr builder = IPolyfaceConstruction::Create(*options);
    builder->Clear();
    Check::False(builder->AddTriangulation(points), "Triangulator returns no facets.");
    Check::ClearGeometry("PolyfaceConstruction.NearlyColinearTriangulation");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Polyface, ConstrainedTriangulation)
    {
    bvector<bvector<DPoint3d>> loops;
    bvector<bvector<DPoint3d>> paths;
    loops.push_back(
        bvector<DPoint3d>
        {
        DPoint3d::From(0, 0, 0),
            DPoint3d::From(8, 0, 0),
            DPoint3d::From(8, 3, 0),
            DPoint3d::From(4, 3, 0),
            DPoint3d::From(4, 6, 0),
            DPoint3d::From(0, 6, 0),
            DPoint3d::From(0, 0, 0)
        }
    );

    paths.push_back(
        bvector<DPoint3d>
        {
        DPoint3d::From(7, 1, 1),
            DPoint3d::From(2, 1, 1),
            DPoint3d::From(2, 5, 1),
        }
    );
    paths.push_back(
        bvector<DPoint3d>
            {
            DPoint3d::From(4, 2, 1),
            DPoint3d::From(3, 2, 1),
            DPoint3d::From(3, 4, 1)
            }
    );

    paths.push_back (
        bvector<DPoint3d>
            {
            DPoint3d::From(5, 1, 1),
            DPoint3d::From(5, 4, 1),
            DPoint3d::From(3, 7, 1),
            }
    );

    bvector<DPoint3d> isolatedPoints{
        DPoint3d::From(1,2,2),
        DPoint3d::From(1,4,5),
        DPoint3d::From (8,8,2),
        DPoint3d::From (10,7,2)
        };

    {
    SaveAndRestoreCheckTransform shifter(20, 0, 0);
    Check::SaveTransformed(loops);
    Check::SaveTransformed(paths);
    Check::SaveTransformedMarkers (isolatedPoints, 0.05);
    Check::Shift(0, 10, 0);
    PolyfaceHeaderPtr mesh = PolyfaceHeader::CreateConstrainedTriangulation(loops, &paths, &isolatedPoints);
    if (Check::True(mesh.IsValid()))
        {
        Transform localToWorld, worldToLocal;
        mesh->BuildXYParameters(LOCAL_COORDINATE_SCALE_01RangeBothAxes, localToWorld, worldToLocal);
        PrintPolyface(*mesh, "constrained triangulation", stdout, s_maxPolyfacePrint, false);
        Check::SaveTransformed(*mesh);
        }
    }

    auto cvLoops = CurveVector::Create(CurveVector::BOUNDARY_TYPE_ParityRegion);
    for (auto &loop : loops)
        {
        cvLoops->Add(CurveVector::CreateLinear(loop, CurveVector::BOUNDARY_TYPE_Outer));
        }

    auto cvPaths = CurveVector::Create(CurveVector::BOUNDARY_TYPE_None);
    for (auto &path : paths)
        {
        cvPaths->Add(ICurvePrimitive::CreateLineString(path));
        }

    {
    SaveAndRestoreCheckTransform shifter(20, 0, 0);
    Check::SaveTransformed(*cvLoops);
    Check::SaveTransformed(*cvPaths);
    Check::Shift(0, 10, 0);
    PolyfaceHeaderPtr mesh = PolyfaceHeader::CreateConstrainedTriangulation(*cvLoops, cvPaths.get(), &isolatedPoints);
    if (Check::True(mesh.IsValid()))
        {
        Transform localToWorld, worldToLocal;
        mesh->BuildXYParameters(LOCAL_COORDINATE_SCALE_01RangeBothAxes, localToWorld, worldToLocal);
        PrintPolyface(*mesh, "constrained triangulation", stdout, s_maxPolyfacePrint, false);
        Check::SaveTransformed(*mesh);
        }
    }
    Check::ClearGeometry("Polyface.ConstrainedTriangulation");
    }

bool ReadDgnjsGeometry(bvector<IGeometryPtr> &geometry, size_t minGeometry, WCharCP nameA, WCharCP nameB, WCharCP nameC);
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Polyface, ConstrainedTriangulationB)
    {
    bvector<IGeometryPtr> originalMesh, originalPolygon;
    ReadDgnjsGeometry(originalMesh, 1, L"Polyface", L"ConstrainedTriangulation", L"original.dgnjs");
    ReadDgnjsGeometry(originalPolygon, 1, L"Polyface", L"ConstrainedTriangulation", L"polygon.dgnjs");
    if (originalMesh.size() == 1 && originalPolygon.size() == 1) {
        auto mesh = originalMesh[0]->GetAsPolyfaceHeader();
        auto polygon = originalPolygon[0]->GetAsCurveVector();
        if (mesh.IsValid() && polygon.IsValid())
            {
            PolyfaceHeaderPtr clippedMesh = PolyfaceHeader::CreateConstrainedTriangulation(*polygon, nullptr,  &mesh->Point ());
            if (clippedMesh.IsValid ())
                Check::SaveTransformed (*clippedMesh);
            }
        }
    Check::ClearGeometry("Polyface.ConstrainedTriangulationB");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Polyface, SmallGapsInContourTriangulation)
    {

    bvector<IGeometryPtr> geometryFromFile;
    ReadDgnjsGeometry(geometryFromFile, 1, L"CurveVector", L"1904AContourStroking", L"cap.imjs");
    if (geometryFromFile.size() == 1 && geometryFromFile.size() == 1) {
        auto region = geometryFromFile[0]->GetAsCurveVector();
        if (region.IsValid())
            {
            Check::SaveTransformed(*region);
            Check::Shift (0.0, 2.0, 0.0);
            double priorArea = 0.0095;
            for (double degrees : {90.0, 45.0, 30.0, 10.0})
                {
                Check::StartScope("Mesh AngleTolerance degrees", degrees);
                Check::Shift (0.1, 0.1);
                IFacetOptionsPtr options = CreateFacetOptions();
                options->SetMaxPerFace(3);
                options->SetAngleTolerance (Angle::DegreesToRadians (degrees));
                IPolyfaceConstructionPtr builder = IPolyfaceConstruction::Create(*options);
                builder->Clear();
                builder->AddRegion(*region);
                auto mesh = builder->GetClientMeshPtr ();
                Check::True (mesh.IsValid (), "triangulation created");
                if (mesh.IsValid())
                    {
                    double meshArea = mesh->SumFacetAreas ();
                    Check::LessThanOrEqual (meshArea, 0.011);
                    Check::LessThanOrEqual (priorArea, meshArea); // more facets should increase area.
                    if (degrees == 90.0)
                        Check::Size(12, mesh->GetNumFacet ());
                    Check::SaveTransformed(*mesh);
                    priorArea = meshArea;
                    }
                Check::EndScope();
                }
            }
        }
    Check::ClearGeometry("Polyface.SmallGapsInContourTriangulation");
    }
BEGIN_BENTLEY_GEOMETRY_NAMESPACE
GEOMDLLIMPEXP PolyfaceHeaderPtr MTGFacets_CloneCatmullClarkSubdivision(MTGFacets &in, double &maxChange, int boundaryAction = 1);
END_BENTLEY_GEOMETRY_NAMESPACE
PolyfaceHeaderPtr CreateGridMesh(size_t nx, size_t ny, bool doNormals = true, bool doParams = true, bvector<std::pair<size_t, size_t>> *omitXY = nullptr);

PolyfaceHeaderPtr ToPolyface (ISolidPrimitivePtr &primitive)
    {
    IFacetOptionsPtr options = IFacetOptions::Create();
    options->SetMaxPerFace (4);
    options->SetAngleTolerance (Angle::DegreesToRadians (45.0));
    IPolyfaceConstructionPtr builder = IPolyfaceConstruction::Create(*options);
    builder->AddSolidPrimitive(*primitive);
    return builder->GetClientMeshPtr();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Polyface, SubDivide)
    {

    bvector<PolyfaceHeaderPtr> baseMesh;
    for (bool skew : { false, true})
        {
        for (uint32_t n : {1, 2, 3})
            {
            uint32_t numX = n;
            uint32_t numY = n;
            auto polyfaceA = CreateGridMesh(numX, numY, false, false);
            if (skew)
                {
                double e = 0.5 / n;
                Transform skewTransform = Transform::FromRowValues(
                    1.0, e, 0, 0,
                    0, 1.0, 0, 0,
                    0, 0, 1, 0);
                skewTransform.Multiply (polyfaceA->Point ());
                }
            baseMesh.push_back(polyfaceA);
            }
        }
    baseMesh.push_back(PolyfaceHeader::CreateRegularPolyhedron(1.0, 1, 2, nullptr));

    for (auto triangulate : {false, true})
        {
        auto meshA = PolyfaceWithSinusoidalGrid(10, 13, 0.1, 0.4, 1.2, 0.8, triangulate);
        meshA->ConvertToVariableSizeSignedOneBasedIndexedFaceLoops();
        baseMesh.push_back (meshA);

        auto meshB = PolyfaceWithSinusoidalGrid(5, 8, 0.1, 0.8, 1.2, 1.1, triangulate);
        meshB->ConvertToVariableSizeSignedOneBasedIndexedFaceLoops();
        baseMesh.push_back(meshB);
        }

    baseMesh.push_back(CreatePolyface_ExtrudedL(0, 0, 6, 1, 2, 5, 1.5));
    baseMesh.push_back(CreatePolyface_ExtrudedLQuads(0, 0, 6, 1, 2, 5, 1.5));

    bvector<IGeometryPtr> simpleSolids;
    SampleGeometryCreator::AddSimplestSolidPrimitives(simpleSolids, true);
    for (auto &g : simpleSolids)
        {
        ISolidPrimitivePtr s = g->GetAsISolidPrimitive();
        if (s.IsValid ())
            {
            auto m = ToPolyface(s);
            if (m.IsValid ())
                baseMesh.push_back (m);
            }
        }

    for (auto mesh : baseMesh)
        {
        Check::SaveTransformed(mesh);
        auto range = DRange3d::From (mesh->Point ());
        SaveAndRestoreCheckTransform shifter (2.0 * range.XLength(), 0, 0);
        MTGFacets mtgA;
        if (Check::True(PolyfaceToMTG_FromPolyfaceConnectivity(&mtgA, *mesh)))
            {
            auto meshB = mesh;
            for (int i = 0; i < 3; i++)
                {
                MTGFacets mtgB;
                if (Check::True(PolyfaceToMTG_FromPolyfaceConnectivity(&mtgB, *meshB)))
                    {
                    double maxChange;
                    auto meshC = MTGFacets_CloneCatmullClarkSubdivision(mtgB, maxChange);
                    Check::Shift(0, 1.25 * range.YLength(), 0);
                    Check::SaveTransformed(meshC);
                    meshB = meshC;
                    }
                else
                    break;
                }
            }
        }
      
    Check::ClearGeometry("Polyface.Subdivide");
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Polyface, SubDivideLoop)
    {

    bvector<PolyfaceHeaderPtr> baseMesh;
    for (bool skew : { false, true})
        {
        for (uint32_t n : {1, 2, 3})
            {
            uint32_t numX = n;
            uint32_t numY = n;
            auto polyfaceA = CreateGridMesh(numX, numY, false, true);
            polyfaceA->Triangulate ();
            if (skew)
                {
                double e = 0.5 / n;
                Transform skewTransform = Transform::FromRowValues(
                    1.0, e, 0, 0,
                    0, 1.0, 0, 0,
                    0, 0, 1, 0);
                skewTransform.Multiply(polyfaceA->Point());
                }
            baseMesh.push_back(polyfaceA);
            }
        }
    for (auto triangulate : { true })
        {
        auto meshA = PolyfaceWithSinusoidalGrid(10, 13, 0.1, 0.4, 1.2, 0.8, triangulate);
        meshA->ConvertToVariableSizeSignedOneBasedIndexedFaceLoops();
        baseMesh.push_back(meshA);

        auto meshB = PolyfaceWithSinusoidalGrid(5, 8, 0.1, 0.8, 1.2, 1.1, triangulate);
        meshB->ConvertToVariableSizeSignedOneBasedIndexedFaceLoops();
        baseMesh.push_back(meshB);
        }

    baseMesh.push_back (PolyfaceHeader::CreateRegularPolyhedronWithSubtriangulation(1.0, 4, 0, nullptr));
    baseMesh.push_back(PolyfaceHeader::CreateRegularPolyhedronWithSubtriangulation(1.0, 4, 1, nullptr));

    bvector<IGeometryPtr> simpleSolids;
    SampleGeometryCreator::AddTorusPipe (simpleSolids);
    SampleGeometryCreator::AddRotatedArc (simpleSolids, 3, 0, 1,
            Angle::FromDegrees (135), Angle::FromDegrees (225),
            Angle::FromDegrees (80));
    SampleGeometryCreator::AddRotatedArc(simpleSolids, 3, 0, 1,
        Angle::FromDegrees(0), Angle::FromDegrees(225),
        Angle::FromDegrees(160));
    SampleGeometryCreator::AddSimplestSolidPrimitives(simpleSolids, true, 2);
    SampleGeometryCreator::AddMeshedSolidPrimtives(baseMesh, simpleSolids,
            3, Angle::DegreesToRadians (15));
    SampleGeometryCreator::AddMeshedSolidPrimtives(baseMesh, simpleSolids,
        3, Angle::DegreesToRadians(45));

    for (auto meshA : baseMesh)
        {
        auto range = DRange3d::From(meshA->Point());
        SaveAndRestoreCheckTransform shifter(2.0 * range.XLength(), 0, 0);
        Check::SaveTransformed(meshA);
        for (uint32_t depth : {1,2, 3})
            {
            if (depth > 1 && pow(4.0, depth) * meshA->Point ().size () > 800)
                break;
            auto meshB = PolyfaceHeader::CloneSubdivided (*meshA, 1, depth);
            if (Check::True(meshB.IsValid()))
                {
                Check::Shift(0, 1.25 * range.YLength(), 0);
                Check::SaveTransformed(meshB);
                }
            }
        }

    Check::ClearGeometry("Polyface.SubdivideLoop");
    }

PolyfaceHeaderPtr CreatePylon(double r0, double hz, uint32_t n)
    {
    PolyfaceHeaderPtr mesh = PolyfaceHeader::CreateVariableSizeIndexed ();
    bvector<int> &pointIndex = mesh->PointIndex ();
    double dTheta = Angle::TwoPi() / (double)n;
    for (uint32_t k = 0; k < n; k++)
        {
        double theta = k * dTheta;
        mesh->Point().push_back (DPoint3d::From (r0 * cos (theta), r0 * sin (theta), 0.0));
        }
    mesh->Point ().push_back (DPoint3d::From (0,0, hz));
    for (uint32_t i = 0; i < n; i++)
        {
        pointIndex.push_back (i + 1);
        pointIndex.push_back (1 + ((i+1) % n));
        pointIndex.push_back (1 + n);
        pointIndex.push_back (0);
        }
    return mesh;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Polyface, SubDivideRegular)
    {
    bvector<PolyfaceHeaderPtr> baseMesh;

    for (double hz : {1.0})
        {
        baseMesh.push_back(CreatePylon(1.0, hz, 6));
        baseMesh.push_back(CreatePylon(1.0, hz, 4));
        baseMesh.push_back(CreatePylon(1.0, hz, 3));
        }

    baseMesh.push_back(PolyfaceHeader::CreateRegularPolyhedron(1.0, 0, 2, nullptr));
    baseMesh.push_back(PolyfaceHeader::CreateRegularPolyhedron(1.0, 2, 2, nullptr));
    baseMesh.push_back(PolyfaceHeader::CreateRegularPolyhedron(1.0, 4, 2, nullptr));

    baseMesh.push_back(PolyfaceHeader::CreateRegularPolyhedronWithSubtriangulation(1.0, 0, 0, nullptr));
    baseMesh.push_back(PolyfaceHeader::CreateRegularPolyhedronWithSubtriangulation(1.0, 0, 1, nullptr));

    baseMesh.push_back(PolyfaceHeader::CreateRegularPolyhedron(1.0, 1, 2, nullptr));
    baseMesh.push_back(PolyfaceHeader::CreateRegularPolyhedronWithSubtriangulation(1.0, 1, 0, nullptr));
    baseMesh.push_back(PolyfaceHeader::CreateRegularPolyhedronWithSubtriangulation(1.0, 1, 1, nullptr));
    baseMesh.push_back(PolyfaceHeader::CreateRegularPolyhedronWithSubtriangulation(1.0, 4, 0, nullptr));
    baseMesh.push_back(PolyfaceHeader::CreateRegularPolyhedronWithSubtriangulation(1.0, 4, 1, nullptr));

    for (auto meshA : baseMesh)
        {
        auto range = DRange3d::From(meshA->Point());
        SaveAndRestoreCheckTransform shifter(2.0 * range.XLength(), 0, 0);
        Check::SaveTransformed(meshA);
        for (uint32_t depth : {1, 3})
            {
            if (pow(4.0, depth) * meshA->Point().size() > 600)
                break;
            auto meshB = PolyfaceHeader::CloneSubdivided(*meshA, 1, depth);
            if (Check::True(meshB.IsValid()))
                {
                Check::Shift(0, 1.25 * range.YLength(), 0);
                Check::SaveTransformed(meshB);
                }
            }
        }

    Check::ClearGeometry("Polyface.SubdivideRegular");
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Polyface, SubDivideThroughTags)
    {
    auto pylon4 = CreatePylon(2.0, 1.0, 4);
    auto pylon6 = CreatePylon(2.0, 1.0, 6);
    auto meshL = CreatePolyface_ExtrudedLQuads(0, 0, 5, 2, 2, 5, 2);
    auto meshSphere0 = PolyfaceHeader::CreateRegularPolyhedronWithSubtriangulation(1.0, 0, 0, nullptr);
    auto meshSphere2 = PolyfaceHeader::CreateRegularPolyhedronWithSubtriangulation(1.0, 2, 0, nullptr);
    auto meshSphere4 = PolyfaceHeader::CreateRegularPolyhedronWithSubtriangulation(1.0, 4, 0, nullptr);

    // auto meshSphere0A = PolyfaceHeader::CreateRegularPolyhedronWithSubtriangulation(1.0, 0, 1, nullptr);
    // auto meshSphere2A = PolyfaceHeader::CreateRegularPolyhedronWithSubtriangulation(1.0, 2, 1, nullptr);

    bvector<TaggedNumericData> allTags;
    for (int subDType : {3, 2,1, 0})
        {
        // push a placeholder to signal a break ..
        allTags.push_back(TaggedNumericData(0, 0));
        // Sample TaggedNumericData setups:

        for (int32_t depth = 1; depth < 4; depth++)
            {
            // Request subdivision at fixed depth:
            TaggedNumericData tag = TaggedNumericData(TaggedNumericData::TagType::SubdivisionSurface, subDType);
            tag.PushInts(TaggedNumericData::SubdivisionControlCode::FixedDepth, depth);
            allTags.push_back (tag);
            }
        for (double relTol : {0.20, 0.10, 0.05, 0.025, 0.0125})
            {
            // Request subdivision to a tolerance that is a fraction of the base polyface range:
            TaggedNumericData tag = TaggedNumericData(TaggedNumericData::TagType::SubdivisionSurface, subDType);
            tag.PushIndexedDouble(TaggedNumericData::SubdivisionControlCode::FractionOfRangeBoxTolerance, relTol);
            allTags.push_back(tag);
            }
        }
    bvector<PolyfaceHeaderPtr> allMesh { pylon4, pylon6, meshSphere0, meshL, meshSphere2, meshSphere4 };

    auto options = CreateFacetOptions ();
    options->SetChordTolerance(0.0);
    for (uint32_t i = 0; i < allMesh.size (); i++)
        {
        auto meshA = allMesh[i];
        auto range = DRange3d::From(meshA->Point());
        double dy = 2.5 * range.YLength();
        SaveAndRestoreCheckTransform shifter(4.0 * range.XLength(), 0, 0);
        for (auto &tag : allTags)
            {
            if (tag.IsTagA (TaggedNumericData::TagType::SubdivisionSurface))
                {
                Check::SaveTransformed(DSegment3d::From(0, 0, 0, 0, dy, 0));
                Check::Shift(0, dy, 0);
                auto meshB = meshA->Clone ();
                meshB->SetNumericTags (tag);
                auto meshC = PolyfaceQuery::ComputeAlternatePolyface (*meshB, *options);
                meshC->MarkAllEdgesVisible ();
                Check::SaveTransformed(meshC);
                }
            else
                {
                Check::Shift(0, 2.0 * dy, 0);
                Check::SaveTransformed(meshA);
                }
            }
        }
    // move over for samples with tolerance from facet options
    Check::Shift (10,0,0);
    for (uint32_t i = 0; i < allMesh.size(); i++)
        {
        auto meshA = allMesh[i];
        auto range = DRange3d::From(meshA->Point());
        double dy = 1.5 * range.YLength();
        SaveAndRestoreCheckTransform shifter(4.0 * range.XLength(), 0, 0);
        for (int subDType : {3, 2, 1})
            {
            TaggedNumericData tag = TaggedNumericData(TaggedNumericData::TagType::SubdivisionSurface, subDType);
            auto meshE = meshA->Clone();
            meshE->SetNumericTags (tag);
            Check::SaveTransformed(meshE);
            for (double chordTolerance : {0.60, 0.03})
                {
                Check::Shift(0, dy, 0);
                auto oldChordTolerance = options->GetChordTolerance();
                options->SetChordTolerance(chordTolerance);
                // request subdivision, but no tolerances, so a tolerance from FacetOptions is used:
                options->SetChordTolerance (chordTolerance);
                auto meshF = PolyfaceQuery::ComputeAlternatePolyface(*meshE, *options);
                options->SetChordTolerance(oldChordTolerance);
                meshF->MarkAllEdgesVisible();
                Check::SaveTransformed(meshF);
                }
            Check::Shift(0, 2.0 * dy, 0);
            }
        }

    Check::ClearGeometry("Polyface.SubDivideThroughTags");
    }


TEST(Polyface, SubdivideFromFiles)
    {
    bvector<IGeometryPtr> imjsGeometry;
    ReadDgnjsGeometry(imjsGeometry, 1, L"Polyface", L"subd", L"test_subdmesh1.imjs");
    double ax = 15.0;
    double ay = 20.0;
    if (imjsGeometry.size() == 1)
        {
        auto meshA = imjsGeometry[0]->GetAsPolyfaceHeader();
        if (meshA.IsValid())
            {
            auto range = meshA->PointRange();
            auto shiftToOrigin = Transform::From (-range.low.x, -range.low.y, -range.low.z);
            meshA->Transform (shiftToOrigin);
            auto scale = 10.0 / range.XLength ();
            auto scaleAboutOrigin = Transform::FromScaleFactors(scale, scale, scale);
            meshA->Transform(scaleAboutOrigin);

            for (int method : {1,2,3})
                {
                SaveAndRestoreCheckTransform shifter (0, ay, 0);
                Check::SaveTransformed(*meshA);
                Check::Shift (ax, 0, 0);
                auto meshB0 = PolyfaceHeader::CloneSubdivided(*meshA, method, 1, 0.0, 0);
                Check::SaveTransformed (*meshB0);
                Check::Shift(ax, 0, 0);
                auto meshB1 = PolyfaceHeader::CloneSubdivided(*meshA, method, 1, 0.0, 1);
                Check::SaveTransformed(*meshB1);
                }
            }
        }
    Check::ClearGeometry("Polyface.SubdivideFromFiles");
    }
