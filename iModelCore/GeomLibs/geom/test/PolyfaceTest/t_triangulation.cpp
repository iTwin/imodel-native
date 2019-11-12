/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
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
    SaveAndRestoreCheckTransform shifter(0.0, 20.0 * r, 0);
    double bigStep = 30.0 * r;
    double oneStep = 4.0 * r;
    for (size_t numPoint = 3; numPoint < 10; numPoint++, z += deltaZ)
        {
        SaveAndRestoreCheckTransform shifter(bigStep, 0, 0);

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
* @bsimethod                                                     Earlin.Lutz  10/17
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
* @bsimethod                                                     Earlin.Lutz  10/17
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
* @bsimethod                                                     Earlin.Lutz  10/17
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
* @bsimethod                                                     Earlin.Lutz  10/17
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
* @bsimethod                                                     Earlin.Lutz  10/17
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