//
//
#include "Geom/GeomApi.h"

USING_NAMESPACE_BENTLEY_GEOMETRY_INTERNAL

MSBsplineSurfacePtr SurfaceWithSinusoidalControlPolygon (int uOrder, int vOrder, size_t numI, size_t numJ, double q0I, double aI, double q0J, double aJ)
    {
    bvector<DPoint3d>poles;
    for (size_t j = 0; j < numJ; j++)
        for (size_t i = 0; i < numI; i++)
            {
            poles.push_back (DPoint3d::From ((double)i, (double)j, sin(q0I + aI * i) * sin (q0J + aJ * j)));
            }

    return MSBsplineSurface::CreateFromPolesAndOrder
        (
        poles, NULL,
        NULL, uOrder, (int)numI, false,
        NULL, vOrder, (int)numJ, false,
        true                
        );
    }


PolyfaceHeaderPtr PolyfaceWithSinusoidalGrid (size_t numI, size_t numJ, double q0I, double aI, double q0J, double aJ, bool triangulated)
    {
    PolyfaceHeaderPtr mesh = triangulated ?
              PolyfaceHeader::CreateTriangleGrid ((int)numI)
            : PolyfaceHeader::CreateQuadGrid ((int)numI);

    bvector<DPoint3d>poles;
    for (size_t j = 0; j < numJ; j++)
        for (size_t i = 0; i < numI; i++)
            {
            mesh->Point ().push_back (DPoint3d::From ((double)i, (double)j, sin(q0I + aI * i) * sin (q0J + aJ * j)));
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
            double u = u1 / (double)(numI - 1);
            double v = v1 / (double)(numJ - 1);
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

PolyfaceHeaderPtr UnitGridPolyface (DPoint3dDVec3dDVec3dCR plane, int numXEdge, int numYEdge, bool triangulated)
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
    bvector<DPoint3d> basePoints
        {
        DPoint3d::From (x0,y0),
        DPoint3d::From (x1,y0),
        DPoint3d::From (x1,y1),
        DPoint3d::From (x2,y1),
        DPoint3d::From (x2,y2),
        DPoint3d::From (x0,y2)
        //,DPoint3d::From (x0,y0)   // closure point
        };
    auto mesh = PolyfaceHeader::CreateVariableSizeIndexed ();
    mesh->AddPolygon (basePoints);
    mesh->SweepToSolid (DVec3d::From (0,0,h), false);
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

PolyfaceHeaderPtr Mesh_XYGrid (int numXEdge, int numYEdge, double edgeX, double edgeY, bool triangulate)
    {
    PolyfaceHeaderPtr mesh = PolyfaceHeader::CreateVariableSizeIndexed ();
    bvector<DPoint3d> &point = mesh->Point ();
    bvector<int> &pointIndex = mesh->PointIndex ();
    int rowStep = 1;
    int colStep = numXEdge + 1;
    double du = edgeX / numXEdge;
    double dv = edgeY / numYEdge;
    if (du == 0.0)
        du = 1.0;
    if (dv == 0.0)
        dv = 1.0;
    for (int row = 0; row <= numYEdge; row++)
        {
        for (int col = 0; col <= numXEdge; col++)
            {
            point.push_back (DPoint3d::From ((double)col * du, (double)row * dv, 0.0));
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

