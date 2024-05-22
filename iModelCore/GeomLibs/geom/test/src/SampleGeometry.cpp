/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
//
//
#include "Geom/GeomApi.h"
#include "FileOps.h"
#include <Bentley/BeTest.h>

USING_NAMESPACE_BENTLEY_GEOMETRY_INTERNAL

MSBsplineSurfacePtr SurfaceWithSinusoidalControlPolygon (int uOrder, int vOrder, size_t numI, size_t numJ, double q0I, double aI, double q0J, double aJ, double weight = 0.0)
    {
    bvector<DPoint3d>poles;
    bvector<double> weights;
    for (size_t j = 0; j < numJ; j++)
        for (size_t i = 0; i < numI; i++)
            {
            DPoint3d xyz = DPoint3d::From ((double)i, (double)j, sin(q0I + aI * i) * sin (q0J + aJ * j));
            if (weight != 0.0)
                xyz.Scale (weight);
            poles.push_back (xyz);
            weights.push_back (weight);
            }

    return MSBsplineSurface::CreateFromPolesAndOrder
        (
        poles, weight != 0.0 ? &weights : nullptr,
        NULL, uOrder, (int)numI, false,
        NULL, vOrder, (int)numJ, false,
        true
        );
    }



MSBsplineSurfacePtr SurfaceBubbleWithMidsideWeights
(
double wMidSide,
double wInterior
)
    {
    bvector<DPoint3d>poles
        {
        DPoint3d::From (0,0,1),
        DPoint3d::From (1,0,1.1),
        DPoint3d::From (2,0,1),

        DPoint3d::From (0,1,1.2),
        DPoint3d::From (1,1,1.3),
        DPoint3d::From (2,1,1.5),

        DPoint3d::From (0,2,2),
        DPoint3d::From (1,2,2.5),
        DPoint3d::From (2,2,2)
        };

    bvector<double> weights
        {
        1, wMidSide, 1,
        wMidSide, wInterior, wMidSide,
        1, wMidSide, 1
        };

    return MSBsplineSurface::CreateFromPolesAndOrder
        (
        poles, &weights,
        NULL, 3, 3, false,
        NULL, 3, 3, false,
        true
        );


    }


PolyfaceHeaderPtr PolyfaceWithSinusoidalGrid (
size_t numI, size_t numJ,
double q0I, double aI,
double q0J, double aJ,
bool triangulated,
bool params
)
    {
    PolyfaceHeaderPtr mesh = triangulated ?
              PolyfaceHeader::CreateTriangleGrid ((int)numI)
            : PolyfaceHeader::CreateQuadGrid ((int)numI);

    bvector<DPoint3d>poles;
    if (params)
        mesh->Param().SetActive (true);
    for (size_t j = 0; j < numJ; j++)
        for (size_t i = 0; i < numI; i++)
            {
            mesh->Point ().push_back (DPoint3d::From ((double)i, (double)j, sin(q0I + aI * i) * sin (q0J + aJ * j)));
            if (params)
                mesh->Param ().push_back (DPoint2d::From ((double)i, (double)j));
            }
    return mesh;
    }




MSBsplineSurfacePtr HyperbolicGridSurface (size_t uOrder, size_t vOrder, size_t numI, size_t numJ, double x11, double y11, double z11, double u1, double v1)
    {
    bvector<DPoint3d>poles;
    DBilinearPatch3d patch (
            DPoint3d::From (0,0,0),
            DPoint3d::From (1,0,0),
            DPoint3d::From (0,1,0),
            DPoint3d::From (x11, y11, z11)
            );
    for (size_t j = 0; j < numJ; j++)
        for (size_t i = 0; i < numI; i++)
            {
            double u = i * u1 / (double)(numI - 1);
            double v = j * v1 / (double)(numJ - 1);
            poles.push_back (patch.Evaluate (u, v));
            }

    return MSBsplineSurface::CreateFromPolesAndOrder
        (
        poles, NULL,
        NULL, (int)uOrder, (int)numI, false,
        NULL, (int)vOrder, (int)numJ, false,
        true
        );
    }


PolyfaceHeaderPtr HyperbolicGridMesh (size_t numI, size_t numJ, double x11, double y11, double z11, double u1, double v1, bool triangulated = false)
    {
    MSBsplineSurfacePtr bsurf = HyperbolicGridSurface (2,2, numI, numJ, x11, y11, z11, u1, v1);
    PolyfaceHeaderPtr mesh = triangulated
            ? PolyfaceHeader::CreateTriangleGrid ((int)numI)
            : PolyfaceHeader::CreateQuadGrid ((int)numI);
    for (size_t i = 0; i < bsurf->GetNumPoles (); i++)
        {
        mesh->Point ().push_back (bsurf->GetPole (i));
        }
    return mesh;
    }

PolyfaceHeaderPtr UnitGridPolyface (DPoint3dDVec3dDVec3dCR plane, int numXEdge, int numYEdge, bool triangulated, bool coordinateOnly)
    {
    if (coordinateOnly)
        {
        PolyfaceHeaderPtr mesh = PolyfaceHeader::CreateFixedBlockCoordinates(triangulated ? 3 : 4);
        for (int j = 0; j < numYEdge; j++)
            {
            for (int i = 0; i < numXEdge; i++)
                {
                DPoint3d xyz00 = plane.Evaluate ((double)i, (double)j);
                DPoint3d xyz10 = plane.Evaluate ((double)(i+1), (double)j);
                DPoint3d xyz11 = plane.Evaluate ((double)(i+1), (double)(j+1));
                DPoint3d xyz01 = plane.Evaluate ((double)i, (double)(j+1));
                if (triangulated)
                    {
                    mesh->Point ().push_back (xyz00);
                    mesh->Point ().push_back (xyz10);
                    mesh->Point ().push_back (xyz01);

                    mesh->Point ().push_back (xyz10);
                    mesh->Point ().push_back (xyz11);
                    mesh->Point ().push_back (xyz01);
                    }
                else
                    {
                    mesh->Point ().push_back (xyz00);
                    mesh->Point ().push_back (xyz10);
                    mesh->Point ().push_back (xyz11);
                    mesh->Point ().push_back (xyz01);
                    }
                }
            }
        return mesh;
        }
    else
        {
        PolyfaceHeaderPtr mesh = triangulated ?
                  PolyfaceHeader::CreateTriangleGrid (numXEdge + 1)
                : PolyfaceHeader::CreateQuadGrid (numXEdge + 1);
        for (int j = 0; j <= numYEdge; j++)
            {
            for (int i = 0; i <= numXEdge; i++)
                mesh->Point ().push_back (plane.Evaluate ((double)i, (double)j));
            }
        return mesh;
        }
    }

PolyfaceHeaderPtr DodecahedronMesh()
    {
    // a mesh with pentagonal faces and int colors
    BeFileName dataFullPathName;
    BeTest::GetHost().GetDocumentsRoot(dataFullPathName);
    dataFullPathName.AppendToPath(L"GeomLibsTestData").AppendToPath(L"Polyface").AppendToPath(L"validation").AppendToPath(L"Dodecahedron.imjs");

    bvector<IGeometryPtr> geometry;
    if (!GTestFileOps::JsonFileToGeometry(dataFullPathName, geometry))
        return nullptr;

    auto mesh = geometry.front()->GetAsPolyfaceHeader();
    if (mesh.IsValid())
        {
        // ensure we have computable data installed
        mesh->BuildApproximateNormals();
        mesh->BuildPerFaceParameters(LocalCoordinateSelect::LOCAL_COORDINATE_SCALE_01RangeBothAxes);
        mesh->BuildPerFaceFaceData();
        }
    return mesh;
    }

PolyfaceHeaderPtr RhombicosidodecahedronMesh()
    {
    // a mesh with triangular, quad, and pentagonal faces and int colors
    BeFileName dataFullPathName;
    BeTest::GetHost().GetDocumentsRoot(dataFullPathName);
    dataFullPathName.AppendToPath(L"GeomLibsTestData").AppendToPath(L"Polyface").AppendToPath(L"validation").AppendToPath(L"rhombicosidodecahedron.imjs");

    bvector<IGeometryPtr> geometry;
    if (!GTestFileOps::JsonFileToGeometry(dataFullPathName, geometry))
        return nullptr;

    auto mesh = geometry.front()->GetAsPolyfaceHeader();
    if (mesh.IsValid())
        {
        // ensure we have computable data installed
        mesh->BuildApproximateNormals();
        mesh->BuildPerFaceParameters(LocalCoordinateSelect::LOCAL_COORDINATE_SCALE_01RangeBothAxes);
        mesh->BuildPerFaceFaceData();
        }
    return mesh;
    }

PolyfaceHeaderPtr SphereMesh(DPoint3dCR origin, double radius, double radianAngle)
    {
    auto sphere = DgnSphereDetail(origin, radius);
    auto options = IFacetOptions::CreateForSurfaces(0.0, radianAngle);
    auto builder = PolyfaceConstruction::Create(*options);
    if (builder->Add(sphere))
        {
        auto mesh = builder->GetClientMeshPtr();
        if (mesh.IsValid())
            return mesh->CloneWithDegenerateFacetsRemoved();    // sphere mesh has degenerate triangles at poles!!
        }
    return nullptr;
    }

/**
 * Make two origin-centered loops (diamond, circle), form parity region, triangulate, return mesh.
 * @param diagonal length of diagonal of the square diamond (measured along the x-axis)
 * @param diameter length of diameter of the circle
 * @param options how to facet the region
 */
PolyfaceHeaderPtr DiamondAndCircleParityRegionMesh(double diagonal, double diameter, IFacetOptionsP options)
    {
    double side = 0.5 * diagonal;
    bvector<DPoint3d> diamondPts{{side,0,0}, {0,side,0}, {-side,0,0}, {0,-side,0}};
    double radius = 0.5 * diameter;
    DEllipse3d hole = DEllipse3d::FromCenterRadiusXY({0,0,0}, radius);
    CurveVectorPtr region = CurveVector::Create(CurveVector::BOUNDARY_TYPE_ParityRegion);
    auto diamondIsOuter = diameter < 0.5 * sqrt(2) * diagonal;
    region->Add(CurveVector::CreateLinear(diamondPts, diamondIsOuter ? CurveVector::BOUNDARY_TYPE_Outer : CurveVector::BOUNDARY_TYPE_Inner));
    region->Add(CurveVector::CreateDisk(hole, diamondIsOuter ? CurveVector::BOUNDARY_TYPE_Inner : CurveVector::BOUNDARY_TYPE_Outer));
    auto builder = PolyfaceConstruction::Create(options ? *options : *IFacetOptions::Create());
    builder->AddRegion(*region);
    return builder->GetClientMeshPtr();
    }

bvector<DPoint3d> CreateL
(
double x0,  // x coordinate at point 0,5,6
double y0,  // y coordinate at point 0,1,6
double x1,  // x coordinate at point 1,2
double y1,  // y coordinate at point 2,3
double x2,  // x coordinate at point 3,4
double y2   // y coordinate at point 4,5
)
    {
    return bvector<DPoint3d>
        {
        DPoint3d::From (x0,y0),
        DPoint3d::From (x1,y0),
        DPoint3d::From (x1,y1),
        DPoint3d::From (x2,y1),
        DPoint3d::From (x2,y2),
        DPoint3d::From (x0,y2)
        //,DPoint3d::From (x0,y0)   // closure point
        };
    }
PolyfaceHeaderPtr CreatePolyface_ExtrudedL
(
double x0,  // x coordinate at point 0,5,6
double y0,  // y coordinate at point 0,1,6
double x1,  // x coordinate at point 1,2
double y1,  // y coordinate at point 2,3
double x2,  // x coordinate at point 3,4
double y2,   // y coordinate at point 4,5
double h    // z distance for extrusion
)
    {
    auto basePoints = CreateL (x0, y0, x1, y1, x2,y2);
    auto mesh = PolyfaceHeader::CreateVariableSizeIndexed ();
    mesh->AddPolygon (basePoints);
    mesh->SweepToSolid (DVec3d::From (0,0,h), false);
    return mesh;
    }

PolyfaceHeaderPtr CreatePolyface_ExtrudedLQuads
(
    double x0,  // x coordinate at point 0,5,6
    double y0,  // y coordinate at point 0,1,6
    double x1,  // x coordinate at point 1,2
    double y1,  // y coordinate at point 2,3
    double x2,  // x coordinate at point 3,4
    double y2,   // y coordinate at point 4,5
    double h    // z distance for extrusion
)
    {
    auto basePoints = CreateL(x0, y0, x1, y1, x2, y2);
    auto mesh = PolyfaceHeader::CreateVariableSizeIndexed();
    mesh->AddPolygon (bvector<DPoint3d>{ {x0,y0}, {x0,y1}, {x2,y1}, {x2,y0}});
    mesh->AddPolygon(bvector<DPoint3d>{ {x2, y0}, { x2,y1 }, { x1,y1 }, { x1,y0 }});
    mesh->AddPolygon(bvector<DPoint3d>{ {x0, y1}, { x0,y2 }, { x2,y2 }, { x2,y1 }});
    mesh->Compress ();
    mesh->SweepToSolid(DVec3d::From(0, 0, h), false);
    return mesh;
    }

MSBsplineSurfacePtr SimpleBilinearPatch (double u1, double v1, double w1)
    {
    bvector<DPoint3d>poles;
    poles.push_back (DPoint3d::From (0,0,0));
    poles.push_back (DPoint3d::From (1,0,0));
    poles.push_back (DPoint3d::From (0,1,0));
    poles.push_back (DPoint3d::From (u1, v1,w1));

    return MSBsplineSurface::CreateFromPolesAndOrder
        (
        poles, NULL,
        NULL, 2, 2, false,
        NULL, 2, 2, false,
        true
        );
    }

PolyfaceHeaderPtr Mesh_XYGrid (int numXEdge, int numYEdge, double edgeX, double edgeY, bool triangulate, bool params = false)
    {
    PolyfaceHeaderPtr mesh = PolyfaceHeader::CreateVariableSizeIndexed ();
    bvector<DPoint3d> &point = mesh->Point ();
    bvector<int> &pointIndex = mesh->PointIndex ();
    bvector<int> &paramIndex = mesh->ParamIndex();
    int rowStep = 1;
    int colStep = numXEdge + 1;
    double du = edgeX / numXEdge;
    double dv = edgeY / numYEdge;
    if (du == 0.0)
        du = 1.0;
    if (dv == 0.0)
        dv = 1.0;
    double duParam = 1.0 / numXEdge;
    double dvParam = 1.0 / numYEdge;
    for (int row = 0; row <= numYEdge; row++)
        {
        for (int col = 0; col <= numXEdge; col++)
            {
            point.push_back (DPoint3d::From ((double)col * du, (double)row * dv, 0.0));
            if (params)
                mesh->Param().push_back (DPoint2d::From ((double)col * duParam, (double)row * dvParam));
            int topRight = (int) point.size (); // 1 based -- all subtractions retain 1 based.
            if (row > 0 && col > 0)
                {
                int bottomRight = topRight - colStep;
                int bottomLeft = bottomRight - rowStep;
                int topLeft = topRight - rowStep;
                // This is the upper right of a quad
                if (triangulate)
                    {
                    pointIndex.push_back (topRight);
                    pointIndex.push_back (topLeft);
                    pointIndex.push_back (bottomLeft);
                    pointIndex.push_back (0);
                    pointIndex.push_back (bottomLeft);
                    pointIndex.push_back (bottomRight);
                    pointIndex.push_back (topRight);
                    pointIndex.push_back (0);

                    }
                else
                    {
                    pointIndex.push_back (topRight);
                    pointIndex.push_back (topLeft);
                    pointIndex.push_back (bottomLeft);
                    pointIndex.push_back (bottomRight);
                    pointIndex.push_back (0);
                    }
                if (params)
                    {
                    if (triangulate)
                        {
                        paramIndex.push_back(topRight);
                        paramIndex.push_back(topLeft);
                        paramIndex.push_back(bottomLeft);
                        paramIndex.push_back(0);
                        paramIndex.push_back(bottomLeft);
                        paramIndex.push_back(bottomRight);
                        paramIndex.push_back(topRight);
                        paramIndex.push_back(0);

                        }
                    else
                        {
                        paramIndex.push_back(topRight);
                        paramIndex.push_back(topLeft);
                        paramIndex.push_back(bottomLeft);
                        paramIndex.push_back(bottomRight);
                        paramIndex.push_back(0);
                        }
                    }
                }
            }
        }
    return mesh;
    }

static void pushPoint (bvector<DPoint3d> &xyz, double x, double y, double z = 0.0)
    {
    xyz.push_back (DPoint3d::From (x,y,z));
    }
//! Create a polygon with square wave upper side.
CurveVectorPtr SquareWavePolygon
(
int numTooth,
double x0,      //!< [in] initial x coordinate
double dx0,     //!< [in] x length of parts (at y=y0)
double dx1,     //!< [in] x length of parts (at y=y10)
double y0,      //!< [in] y height for dx0 part of tooth
double y1,      //!< [in] y height for dx1 part of tooth
bool closed,     //!< [in] true to close with base line.
double ybase     //!< [in] y height for base line
)
    {
    bvector<DPoint3d> points;
    if (numTooth < 1)
        numTooth = 1;
    double x = x0;
    for (int i = 0; i < numTooth; i++)
        {
        pushPoint (points, x, y0);
        x += dx0;
        pushPoint (points, x, y0);
        pushPoint (points, x, y1);
        x += dx1;
        pushPoint (points, x, y1);
        }
    if (closed)
        {
        pushPoint (points, x, ybase);
        pushPoint (points, x0, ybase);
        pushPoint (points, x0,y0);
        return CurveVector::CreateLinear (points, CurveVector::BOUNDARY_TYPE_Outer);    // hmm ... it's clockwise with usual negative ybase
        }
    else
        {
        return CurveVector::CreateLinear (points, CurveVector::BOUNDARY_TYPE_Open);
        }
    }

 bvector<DEllipse3d> s_arcSamples
        {
        DEllipse3d::From (0,0,0,    10,0,0,  0,10,0,   0, Angle::TwoPi ()),
        DEllipse3d::From (5,4,0,    11,0,0,  0,3,0,   0, Angle::TwoPi ()),
        DEllipse3d::From (1,3,0,   6,6,0,   -2,-3,0,  0, Angle::Pi ()),
        DEllipse3d::From (-2,1,0,   4,6,0,   -2,-3,0,  0, Angle::DegreesToRadians (75.0)),
        DEllipse3d::From (4,-1,0,   1,2,0,   -2,3,0,  0, Angle::DegreesToRadians (355.0))
        };

bvector<DPoint3d> CreateOffsetStrip (DEllipse3dCR arc, double offset0, double offset1, size_t numStroke, bool close)
    {
    bvector<DPoint3d> points;
    bvector<DVec3d> perps;
    if (numStroke < 1)
        numStroke = 1;
    double df = 1.0 / numStroke;
    DVec3d derivative2;
    for (size_t i = 0; i <= numStroke; i++)
        {
        double f = i * df;
        double offsetDistance = DoubleOps::Interpolate (offset0, f, offset1);
        DPoint3d xyz;
        DVec3d tangent, perp;
        arc.FractionParameterToDerivatives (xyz, tangent, derivative2, i * df);
        points.push_back (xyz);
        perp.UnitPerpendicularXY (tangent);
        perps.push_back (offsetDistance * perp);
        }
    for (auto i = points.size (); i-- > 0;)
        {
        points.push_back (points[i] + perps[i]);
        }
    return points;
    }
bvector<DPoint3d> CreateT (
double dxTotal, //!< [in] total width of range box
double dyTotal, //!< [in] total height of range box
double dxLeft,  //!< [in] x distance under the left bar
double dxRight, //!< [in] x distance under the right bar
double dyLeft,  //!< [in] vertical width of left bar (measured from top down
double dyRight,      //!< [in] vertical width of right bar (measuredFrom top down)
bool close
)
    {
    double dx = dxTotal - dxLeft - dxRight;
    double x = dxLeft;
    double y = 0.0;
    bvector<DPoint3d> points;

    points.push_back (DPoint3d::From (x,y));
    auto xyz0 = points.front ();
    x += dx;
    points.push_back (DPoint3d::From (x,y));
    y += dyTotal - dyRight;
    points.push_back (DPoint3d::From (x,y));
    x = dxTotal;
    points.push_back (DPoint3d::From (x,y));
    y = dyTotal;
    points.push_back (DPoint3d::From (x,y));
    x = 0.0;
    points.push_back (DPoint3d::From (x,y));
    y -= dyLeft;
    points.push_back (DPoint3d::From (x,y));
    x += dxLeft;
    points.push_back (DPoint3d::From (x,y));
    if (close)
        points.push_back (xyz0);
    return points;
    }

// return a symmetric "T" shape with elliptic fillets under the arms.
CurveVectorPtr CreateFilletedSymmetricT (
double dxTotal, //!< [in] total width of range box
double dyTotal, //!< [in] total height of range box
double dxLeft,  //!< [in] x distance under the left bar
double dyLeft,  //!< [in] vertical width of left bar (measured from top down
double dxFillet,    //!< [in] x size under elliptic fillet
double dyFillet,    //!< [in] y size under elliptic fillet
bool splitUpperEdge
)
    {
    double y0 = 0;
    double y1 = dyTotal - dyLeft - dyFillet;
    double y2 = dyTotal - dyLeft;
    double y3 = dyTotal;
    // positive measurements from centerline at x=0:
    double x3 = dxTotal * 0.5;
    double x2 = x3 - dxLeft + dxFillet;
    double x1 = x3 - dxLeft;
    // bottom of strut, left to right
    bvector<DPoint3d> xyz0 {
            DPoint3d::From (-x1, y1),
            DPoint3d::From (-x1, y0),
            DPoint3d::From (x1, y0),
            DPoint3d::From (x1, y1)
            };
    // bar, right to left
    bvector<DPoint3d> xyz1 {
            DPoint3d::From (x2, y2),
            DPoint3d::From (x3, y2),
            DPoint3d::From (x3, y3)
            };
    if (splitUpperEdge)
        xyz1.push_back (DPoint3d::From (0.0, y3));

    xyz1.push_back(DPoint3d::From (-x3,y3));
    xyz1.push_back(DPoint3d::From (-x3,y2));
    xyz1.push_back(DPoint3d::From (-x2, y2));

    auto cv = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Outer);
    cv->push_back (ICurvePrimitive::CreateLineString (xyz0));
    cv->push_back (ICurvePrimitive::CreateArc (DEllipse3d::From (x2, y1, 0,
                    -dxFillet, 0,0,
                    0,dyFillet,0,
                    0.0, Angle::DegreesToRadians (90.0))));
    cv->push_back (ICurvePrimitive::CreateLineString (xyz1));
    cv->push_back (ICurvePrimitive::CreateArc (DEllipse3d::From (-x2, y1, 0,
                    0,dyFillet,0,
                    dxFillet, 0,0,
                    0.0, Angle::DegreesToRadians (90.0))));
    return cv;

    }

// Create a typical transform for testing.
// All selects "0" is an identity.
// Defaults produce a general rigid rotation around a point near the origin.
// Translate * Rotate * Scale
// that is, in sequential construction, there is a scaling around the origin, followed by rotation around the origin, followed by translation
Transform CreateTestTransform
(
double translateX = 1.0,
double translateY = 2.0,
double translateZ = 3.0,
double yawDegrees = 10.0,
double pitchDegrees = 5.0,
double rollDegrees = -12.0,
double xScale = 1.0,
double yScale = 1.0,
double zScale = 1.0
)
    {
    Transform transform = Transform::From (translateX, translateY, translateZ);
    if (yawDegrees != 0.0 || pitchDegrees != 0.0 || rollDegrees != 0.0)
        {
        auto ypr = YawPitchRollAngles::FromDegrees (yawDegrees, pitchDegrees, rollDegrees);
        transform  = transform * ypr.ToTransform (DPoint3d::From (translateX, translateY, translateZ));
        }
    if (xScale != 1.0 || yScale != 1.0 || zScale != 1.0)
        transform.ScaleMatrixColumns (xScale, yScale, zScale);
    return transform;
    }

//! Create numX * numY points on an integer grid mapped to a plane, packed in a 1D array.
void UnitGridPoints (
bvector<DPoint3d> &points,
int numX,
int numY,
double x0,
double y0,
double dx,
double dy
)
    {
    points.clear ();
    for (int j = 0; j < numY; j++)
        {
        for (int i = 0; i < numX; i++)
            {
            points.push_back (DPoint3d::From (x0 + i * dx, y0 + j * dy));
            }
        }
    }

struct rangeEdgeDef
{
int corner0;
int corner1;
double fractionA;
int numB;
double fractionB;
};


void StrokeFrustum (bvector<DSegment3d> &segments, DPoint3d corners[8], bool patterns = false)
    {
    segments.clear ();
    double ax = 0.7;
    double ay = 0.5;
    double az = 0.3;
    int numC = 0;

    // Edges from "low" get ai,bi decoration (with long first stroke)
    // All others get c,ai
    // +aaaaaaaaaaaaa   aaa  aaa  aaa+
    // first period over fractiona
    // Remainder divided to numB periods with trailing a solid
    //
    bvector<rangeEdgeDef> edgeDefs {
        {0,1, ax, 1},
        {2,3, ax , numC},
        {4,5, ax , numC},
        {6,7, ax , numC},

        {0,2, ay, 2},
        {1,3, ax , numC},
        {4,6, ax , numC},
        {5,7, ax , numC},

        {0,4, az, 3},
        {1,5, ax , numC},
        {2,6, ax , numC},
        {3,7, ax , numC},
        };
    for (auto &def : edgeDefs)
        {
        auto segment = DSegment3d::From (corners[def.corner0], corners[def.corner1]);
        double fractionA = def.fractionA;
        int numB = def.numB;
        if (!segment.point[0].AlmostEqual (segment.point[1]))
            {
            if (!patterns || fractionA >= 1.0 || numB == 0 || fractionA < 0.0)
                segments.push_back (segment);
            else
                {
                DPoint3d pointA = segment.FractionToPoint (fractionA);
                if (fractionA > 0.0)
                    {
                    segments.push_back (DSegment3d::From(segment.point[0], pointA));
                    }
                if (fractionA < 1.0 && numB > 0)
                    {
                    double delta = (1.0 - fractionA) / numB;
                    for (int i = 0; i < numB; i++)
                        {
                        double g0 = fractionA + i * delta;
                        double g2 = g0 + delta;
                        double g1 = g2 - fractionA * delta;
                        segments.push_back (DSegment3d::From(segment.FractionToPoint (g1), segment.FractionToPoint (g2)));
                        }
                    }
                }
            }
        }
    }

void StrokeRange (bvector<DSegment3d> &segments, DRange3dCR range, bool patterns = false)
    {
    DPoint3d corners[8];
    range.Get8Corners (corners);
    StrokeFrustum (segments, corners, patterns);
    }
void StrokeRange (bvector<DSegment3d> &segments, TransformCR transform, DRange3dCR range, bool patterns = false)
    {
    StrokeRange (segments, range, patterns);
    for (auto &s : segments)
        transform.Multiply (s);
    }
void StrokeFrustum (bvector<DSegment3d> &segments, DMatrix4dCR transform, DPoint3d corners[8], bool patterns = false)
    {
    StrokeFrustum (segments, corners, patterns);
    for (auto &s : segments)
        {
        transform.MultiplyAndRenormalize (s.point[0], s.point[0]);
        transform.MultiplyAndRenormalize (s.point[1], s.point[1]);
        }
    }


