/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
//
//
#define NO_RSC_MGR_API  // discourage use of rsc mgr
#define T_LevelIdToDefinitionMapIterator_DEFINED    // Define real T_LevelIdToDefinitionMapIterator
#include <Bentley/BeTest.h>
#include <Bentley/BeConsole.h>
#include <Geom/GeomAPI.h>
#include <stdlib.h>
#pragma warning (disable : 4125)
#pragma warning (disable : 4167)

#include <allcg.pb.h>

#include <allcg.pb.cpp>

using namespace Bentley::Geometry;
class CGProtobuffer
{
public:
static void Set (Proto::DPoint3d *data, DPoint3dCR xyz)
    {
    data->set_x (xyz.x);
    data->set_y (xyz.y);
    if (xyz.z != 0.0)
      data->set_z (xyz.z);
    }

static void Set (Proto::DVector3d *data, DVec3dCR xyz)
    {
    data->set_x (xyz.x);
    data->set_y (xyz.y);
    if (xyz.z != 0.0)
      data->set_z (xyz.z);
    }


public:
static Proto::DPoint3d *CreateDPoint3d (double x, double y, double z)
    {
    Proto::DPoint3d *data = new Proto::DPoint3d ();
    data->set_x (x);
    data->set_y (y);
    if (z != 0.0)
        data->set_z (z);
    return data;
    }

static Proto::DPoint3d *CreateDPoint3d (DPoint3dCR xyz)
    {
    Proto::DPoint3d *data = new Proto::DPoint3d ();
    Set (data, xyz);
    return data;
    }

static Proto::DVector3d *CreateDVec3d (DVec3dCR xyz)
    {
    Proto::DVector3d *data = new Proto::DVector3d ();
    data->set_x (xyz.x);
    data->set_y (xyz.y);
    if (xyz.z != 0.0)
        data->set_z (xyz.z);
    return data;
    }

static Proto::Angle *CreateAngleFromDegrees (double degrees)
    {
    Proto::Angle *data = new Proto::Angle ();
    data->set_degrees (degrees);
    }

static Proto::DEllipse3d *CreateDEllipse3d (DEllipse3dCR arc)
    {
    Proto::DEllipse3d *data = new Proto::DEllipse3d ();
    data->set_centerX (arc.center.x);
    data->set_centerY (arc.center.y);
    if (arc.center.z != 0.0)
        data->set_centerZ (arc.center.z);

    data->set_vector0X (arc.vector0.x);
    data->set_vector0Y (arc.vector0.y);
    if (arc.vector0.z != 0.0)
        data->set_vector0Z (arc.vector0.z);

    data->set_vector90X (arc.vector90.x);
    data->set_vector90Y (arc.vector90.y);
    if (arc.vector90.z != 0.0)
        data->set_vector90Z (arc.vector90.z);

    data->set_startRadians (arc.start);
    data->set_sweepRadians (arc.sweep);

    return data;
    }

static Proto::Angle *CreateAngleFromRadians (double radians)
    {
    Proto::Angle *data = new Proto::Angle ();
    data->set_degrees (Angle::RadiansToDegrees (radians));
    }


static Proto::DVector3d *CreateDVec3d (double x, double y, double z)
    {
    Proto::DVector3d *data = new Proto::DVector3d ();
    data->set_x (x);
    data->set_y (y);
    data->set_z (z);
    return data;
    }

public:
static Proto::LineSegment *CreateLineSegment (double xA, double yA, double zA, double xB, double yB, double zB)
    {
    Proto::LineSegment *lineSegment = new Proto::LineSegment ();
    lineSegment->set_allocated_startPoint (CreateDPoint3d (xA, yA, zA));
    lineSegment->set_allocated_endPoint (CreateDPoint3d (xB, yB, zB));
    return lineSegment;
    }
public:
static Proto::LineString *CreateLineString(bvector<DPoint3d> &points)
    {
    Proto::LineString *lineString = new Proto::LineString ();
    for (size_t i = 0; i < points.size (); i++)
        {
        Proto::DPoint3d *point = lineString->add_Point ();
        Set (point, points[i]);
        }
    return lineString;
    }

public:
static Proto::IPlacement*CreateIPlacement (DPoint3dCR origin, RotMatrixCR axes)
    {
    Proto::IPlacement *data = new Proto::IPlacement ();
    RotMatrix axesA;
    axesA.SquareAndNormalizeColumns (axes, 0, 1);
    DVec3d vectorX, vectorZ;
    axesA.GetColumn (vectorX, 0);
    axesA.GetColumn (vectorZ, 2);
    data->set_allocated_orgin (CreateDPoint3d (origin));
    data->set_allocated_vectorX (CreateDVec3d (vectorX));
    data->set_allocated_vectorZ (CreateDVec3d (vectorZ));
    return data;
    }

public:
static Proto::EllipticArc*CreateEllipticArc (DEllipse3dCR ellipse)
    {
    Proto::DEllipse3d *data = new Proto::DEllipse3d ();
    data->set_centerX (ellipse.center.x);
    data->set_centerY (ellipse.center.y);
    data->set_centerZ (ellipse.center.z);

    data->set_vector0X (ellipse.vector0.x);
    data->set_vector0Y (ellipse.vector0.y);
    data->set_vector0Z (ellipse.vector0.z);

    data->set_vector90X (ellipse.vector90.x);
    data->set_vector90Y (ellipse.vector90.y);
    data->set_vector90Z (ellipse.vector90.z);

    data->set_startRadians (ellipse.start);
    data->set_sweepRadians (ellipse.sweep);

    }

public:
static Proto::BsplineCurve *CreateBsplineCurve (MSBsplineCurveCR bcurve)
    {
    size_t numPoles = bcurve.GetNumPoles ();
    size_t numKnots = bcurve.GetNumKnots ();
    bool rational = NULL != bcurve.GetWeightCP ();
    bool closed = bcurve.IsClosed ();
    int order = bcurve.GetIntOrder ();
    Proto::BsplineCurve *data = new Proto::BsplineCurve ();
    data->set_Order (order);
    data->set_Closed (closed);
    for (size_t i = 0; i < numPoles; i++)
        {
        Proto::DPoint3d *point = data->add_ControlPoint ();
        Set (point, bcurve.GetPole (i));
        }
    if (rational)
        for (size_t i = 0; i < numPoles; i++)
            data->add_Weight (bcurve.GetWeight (i));

    for (size_t i = 0; i < numKnots; i++)
        data->add_Knot (bcurve.GetKnot (i));
    return data;
    }
};


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(PBSize,LineSegment_singlesets)
    {
    Proto::LineSegment *myLine = new Proto::LineSegment ();
    BeConsole::Printf ("PB:LineSegment ByteSize ()=%d\n", myLine->ByteSize ());
    Proto::DPoint3d *point0 = new Proto::DPoint3d ();
    myLine->set_allocated_startPoint (point0);
    BeConsole::Printf ("   +empty startPoint ==> %d\n", myLine->ByteSize ());
    point0->set_x (1.0);
    BeConsole::Printf ("   +point0.x(1) ==> %d\n", myLine->ByteSize ());
    point0->set_y (2.0);
    BeConsole::Printf ("   +point0.y(2) ==> %d\n", myLine->ByteSize ());
    point0->set_z (0.0);
    BeConsole::Printf ("   +point0.z(0) ==> %d\n", myLine->ByteSize ());

    Proto::DPoint3d *point1 = new Proto::DPoint3d ();
    myLine->set_allocated_endPoint (point1);
    BeConsole::Printf ("   +empty endPoint ==> %d\n", myLine->ByteSize ());
    point1->set_x (1.0);
    BeConsole::Printf ("   +point1.x(1) ==> %d\n", myLine->ByteSize ());
    point1->set_y (2.0);
    BeConsole::Printf ("   +point1.y(2) ==> %d\n", myLine->ByteSize ());
    point1->set_z (0.0);
    BeConsole::Printf ("   +point1.z(0) ==> %d\n", myLine->ByteSize ());
}    

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(PBSize,LineSegmentA)
    {
    Proto::LineSegment *data = CGProtobuffer::CreateLineSegment (1,2,3,4,5,6);
    BeConsole::Printf ("CreateLineSegment ByteSize ()=%d\n", data->ByteSize ());

    Proto::LineSegment *data1 = CGProtobuffer::CreateLineSegment (1,2,0,4,5,0);
    BeConsole::Printf ("CreateLineSegment (z=0) ByteSize ()=%d\n", data1->ByteSize ());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(PBSize,LineStringA)
    {
    bvector<DPoint3d> points;
    // make sure everything is nonzero ...
    for (size_t i = 0; i < 16; i++)
        points.push_back (DPoint3d::From (0.5 + (double) i, 1.0, 2.0));
    Proto::LineString *data = CGProtobuffer::CreateLineString (points);
    BeConsole::Printf ("CreateLineString ByteSize ()=%d\n", data->ByteSize ());

    // squash all z to zero...
    for (size_t i = 0; i < 16; i++)
        points[i].z = 0.0;

    Proto::LineString *data1 = CGProtobuffer::CreateLineString (points);
    BeConsole::Printf ("CreateLineString (Z=0) ByteSize ()=%d\n", data1->ByteSize ());

    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(PBSize,IPlacementA)
    {
    RotMatrix axes = RotMatrix::FromRowValues (1,0,0,   0,1,0,   0,0,1);
    Proto::IPlacement *data = CGProtobuffer::CreateIPlacement (DPoint3d::From (1,2,3), axes);
    BeConsole::Printf ("CreateIPlacemnet ByteSize ()=%d\n", data->ByteSize ());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(PBSize,DEllipse3dA)
    {
    Proto::DEllipse3d *data = CGProtobuffer::CreateDEllipse3d (
            DEllipse3d::From (1,2,3,4,5,6,7,8,9, Angle::Pi (), Angle::PiOver2 ()));
    BeConsole::Printf ("CreateDEllipse3d ByteSize ()=%d\n", data->ByteSize ());

    Proto::DEllipse3d *data1 = CGProtobuffer::CreateDEllipse3d (
            DEllipse3d::From (1,2,0,4,5,0,7,8,0, Angle::Pi (), Angle::PiOver2 ()));
    BeConsole::Printf ("CreateDEllipse3d (z=0) ByteSize ()=%d\n", data1->ByteSize ());
    }

void TestBsplineCurve (int numPoles, int order, double z, bool closed)
    {
    if (numPoles >= order)
        {
        bvector<DPoint3d> poles;
        bvector<double>knots;
        for (int i = 0; i < order; i++)
            knots.push_back (0.0);
        int numInteriorPoles = numPoles - order;
        double a = 1.0 / (numInteriorPoles + 1);
        for (int i = 1; i <= numInteriorPoles; i++)
            knots.push_back (i * a);
        for (int i = 0; i < order; i++)
            knots.push_back (0.0);
        for (int i = 0; i < numPoles; i++)
            poles.push_back (DPoint3d::From ((double)i, 2.5 * i, z));
        MSBsplineCurvePtr bcurve = MSBsplineCurve::CreateFromPolesAndOrder (poles, NULL, &knots, order, closed, true);
        Proto::BsplineCurve *data = CGProtobuffer::CreateBsplineCurve (*bcurve);
        BeConsole::Printf ("CreateBsplineCurve (z=%g) (order %d) (poles %d) ByteSize ()=%d\n",
              z, order, numPoles, data->ByteSize ());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(PBSize,MSBsplineCurve)
    {
    TestBsplineCurve (4, 4, 1.0, false);
    TestBsplineCurve  (4, 4, 0.0, false);
    TestBsplineCurve (13, 4, 1.0, false);
    TestBsplineCurve  (13, 4, 0.0, false);

    }