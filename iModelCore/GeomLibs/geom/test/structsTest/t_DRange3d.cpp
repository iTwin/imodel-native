#include "testHarness.h"

template <typename RangeType>
void CheckProperties (RangeType const &range, bool isNull, bool isSinglePoint)
    {
    Check::Bool (isNull, range.IsNull (), "Null range");
    Check::Bool (isSinglePoint, range.IsPoint (), "SinglePoint");
    }


void Extend (DRange2dR range, double x, double y, double z) {range.Extend (x,y);}
void Extend (DRange3dR range, double x, double y, double z) {range.Extend (x,y,z);}



bool Contains (DRange2dR range, double x, double y, double z) {return range.Contains (x,y);}
bool Contains (DRange3dR range, double x, double y, double z) {return range.IsContained (x,y,z);}

bool Contains (DRange2dR range, DPoint2dCR point) {return range.Contains (point);}
bool Contains (DRange2dR range, DPoint3dCR point) {return range.Contains (point);}
bool Contains (DRange3dR range, DPoint3dCR point) {return range.IsContained (point);}

bool ContainsXY (DRange2dR range, DPoint2dCR point) {return range.Contains (point);}
bool ContainsXY (DRange2dR range, DPoint3dCR point) {return range.Contains (point);}
bool ContainsXY (DRange3dR range, DPoint3dCR point) {return range.IsContainedXY (point);}



template <typename RangeType, typename PointType>
void CheckRange ()
    {
    RangeType rangeA = RangeType::NullRange ();
    RangeType rangeB;
    rangeB.Init ();
    // ASSUME points[0] and points[1] form a non-trivail range in all dimensions !!!
    DPoint3d points []=
        {
        {1,2,3},
        {-3,4,1},
        {5,6,7},
        {2,3,4},
        {10,-10,20},
        };
    // rangeA grows by component.
    // rangeB grows by points
    for (int i = 0; i < sizeof (points) / sizeof (DPoint3d); i++)
        {
        CheckProperties <RangeType>(rangeA, i == 0, i == 1);
        CheckProperties <RangeType>(rangeB, i == 0, i == 1);

        RangeType rangeA0 = rangeA;
        //RangeType rangeB0 = rangeB;
        bool inside = Contains (rangeA, points[i]);
        double extent0 = rangeA.ExtentSquared ();
        Check::Bool (i > 1, extent0 > 0.0, "Extent");
        Extend (rangeA, points[i].x, points[i].y, points[i].z);
        rangeB.Extend (points[i]);

        if (inside)
            {
            Check::True (rangeA0.IsEqual (rangeA), "Range Extend inside");
            }
        else
            {
            Check::Bool (i > 0, rangeA.ExtentSquared () > extent0, "Range Extend");
            }
        if (i > 0)
            Check::Near (rangeA, rangeB, "Evolving range");
        Check::True (Contains (rangeA, points[i]), "Contain after extend");
        Check::True (ContainsXY (rangeA, points[i]), "Contain after extend");
        Check::True (Contains (rangeA, points[i].x, points[i].y, points[i].z), "Contain after extend");
        PointType point0 = PointType::FromInterpolate (rangeA.low, -0.2, rangeA.high);
        PointType point1 = PointType::FromInterpolate (rangeA.low, 0.3, rangeA.high);
        PointType point2 = PointType::FromInterpolate (rangeA.low, 1.3, rangeA.high);
        if (i > 0)
            {
            Check::False (Contains (rangeA, point0), "point0 is outside");
            Check::True  (Contains (rangeA, point1), "point1 is inside");
            Check::False (Contains (rangeA, point2), "point2 is outside");

            Check::False (ContainsXY (rangeA, point0), "point0 is outside");
            Check::True  (ContainsXY (rangeA, point1), "point1 is inside");
            Check::False (ContainsXY (rangeA, point2), "point2 is outside");

            }

        double e = 0.001;
        RangeType rangeA1 = rangeA;
        rangeA1.Extend (e);
        Check::True (rangeA1.IsEqual (rangeA, 2.0 * e), "range equality with tol");
        Check::False (rangeA1.IsEqual (rangeA, 0.5 * e), "range equality with tol");        
        }
    }

TEST(DRange3d, Init)
    {
    CheckRange <DRange3d, DPoint3d> ();
    CheckRange <DRange2d, DPoint2d> ();
    }
    
struct Cosine
    {
    double m_a0;
    double m_a1;
    double m_a2;
    Cosine (double a0, double a1, double a2)
        {
        m_a0 = a0;
        m_a1 = a1;
        m_a2 = a2;
        }
        
    double Evaluate (int i)
        {
        double x = i;
        return m_a0 + cos (m_a1 + x * m_a2);
        }
    };
TEST(DRange3d, Arrays)
    {
    Check::StartScope ("DRange3d From InitFrom Extend");
    bvector <DPoint3d> data_X;
    bvector <DPoint4d> data_wXw;
    bvector <DPoint3d> data_wX;
    bvector <double>   data_w;
    bvector <DPoint2d> data_xy;
    
    Cosine fx (2,4,0.3);
    Cosine fy (1,4,-0.2);
    Cosine fz (0.2, 5.0, 2.3);
    Cosine fw (1.0, 0.4, 3.4234);  // Always positive !!!
    
    DPoint3d disconnect;
    disconnect.InitDisconnect ();
    int numPoints = 20;
    DRange1d xRange = DRange1d::NullRange ();
    DRange1d yRange = DRange1d::NullRange ();
    DRange1d zRange = DRange1d::NullRange ();
    for (int i = 0; i < numPoints; i++)
        {
        double x = fx.Evaluate(i);
        double y = fy.Evaluate(i);
        double z = fz.Evaluate(i);
        double w = fw.Evaluate(i);
        xRange.Extend (x);
        yRange.Extend (y);
        zRange.Extend (z);
        DPoint3d xyz = DPoint3d::From (x,y,z);
        DPoint4d wXw = DPoint4d::From (x*w, y*w, z*w, w);
        DPoint3d wX = DPoint3d::From (x*w, y*w, z*w);
        data_X.push_back (xyz);
        if ((i + 2) % 7 == 0)
            data_X.push_back (disconnect);
        data_wX.push_back (wX);
        data_wXw.push_back (wXw);
        data_xy.push_back (DPoint2d::From (x,y));
        data_w.push_back (w);
        }

    DRange3d range_X, range_wXw, range_wX_w, range_xy_z, range_TX;
    range_X.InitFrom (data_X);
    Check::Near (DRange3d::From (data_X), range_X, "<DPoint3d>");
    
    
    Check::Near (range_X.low.x, xRange.Low (), "x0");
    Check::Near (range_X.low.y, yRange.Low (), "y0");
    Check::Near (range_X.low.z, zRange.Low (), "z0");

    Check::Near (range_X.high.x, xRange.High (), "x0");
    Check::Near (range_X.high.y, yRange.High (), "y0");
    Check::Near (range_X.high.z, zRange.High (), "z0");


    range_wX_w.InitFrom (data_wX, &data_w);
    Check::Near (DRange3d::From (data_wX, &data_w), range_wX_w, "<DPoint3d>, <double>");
    
    range_wXw.InitFrom (data_wXw);
    Check::Near (DRange3d::From (data_wXw), range_wXw, "<DPoint3d>, <double>");
    
    Check::Near (range_X, range_wX_w, "xyz:: (wxywywz,w)");
    Check::Near (range_X, range_wXw, "xyz:: (wxywywzw)");
    
    double fixedZ = 1.0523232;
    range_xy_z.InitFrom (data_xy, fixedZ);
    Check::Near (DRange3d::From (data_xy, fixedZ), range_xy_z, "<DPoint2d>, z");
    
    double ax = 2.0, bx = 3.2;
    double ay = 1.2, by = 0.2;
    double az = 4.2, bz = 2.3;  // positive scales commute to range points!!!
    Transform T = Transform::FromRowValues (
                        ax, 0, 0, bx,
                        0, ay, 0, by,
                        0, 0, az, bz
                        );

    range_TX.InitFrom (T, data_X);
    Check::Near (DRange3d::From (T, data_X), range_TX, "T*<DPoint3d>");

    DRange3d T_range_X;
    T.Multiply (T_range_X, range_X);
    Check::EndScope ();
    }
    
TEST(DRange3d, CornerRange)
    {
    DRange3d box = DRange3d::From (DPoint3d::From (1,2,3), DPoint3d::From (3,5,9));
    DRay3d ray = DRay3d::FromOriginAndVector (DPoint3d::From (9,2,3), DVec3d::From (1,1,0.2));
    DPlane3d plane = DPlane3d::FromOriginAndNormal (ray.origin, ray.direction);
    DRange1d rayRange = box.GetCornerRange (ray);
    DRange1d planeRange = box.GetCornerRange (plane);
    Check::Near (rayRange.low, planeRange.low, "plane,ray range low");
    Check::Near (rayRange.high, planeRange.high, "plane,ray range high");
    DPoint3d center = DPoint3d::FromInterpolate (box.low, 0.5, box.high);
    double centerToPlane = plane.Evaluate (center);
    double centerToRay = ray.DirectionDotVectorToTarget (center);
    double fA, fB;
    Check::True (rayRange.FractionToDouble (0.5, fA), "FractionToDouble");
    Check::True (planeRange.FractionToDouble (0.5, fB), "FractionToDouble");

    Check::Near (centerToRay, fA, "box center projection");
    Check::Near (centerToPlane, fB, "box center projection");
    }

TEST(DRange3d,Diagonal)
    {
    DVec3d zero = DVec3d::From (0,0,0);
    DRange3d range0 = DRange3d::From (1,2,3);
    Check::Near (range0.DiagonalVector (), zero);
    Check::Near (range0.DiagonalDistance (), 0.0);
    Check::Near (range0.DiagonalDistanceXY (), 0.0);
    Check::Near (range0.DiagonalVectorXY(), zero);

    DVec3d diagonalA = DVec3d::From (2,3,6);
    DVec3d diagonalA0 = diagonalA;
    diagonalA0.z = 0.0;
    range0.high = DPoint3d::FromSumOf (range0.low, diagonalA);
    // unused - DVec3d diagonalB = range0.DiagonalVector ();

    Check::Near (range0.DiagonalVector (), diagonalA);
    Check::Near (range0.DiagonalDistance (), diagonalA.Magnitude ());
    Check::Near (range0.DiagonalDistanceXY (), diagonalA.MagnitudeXY ());
    Check::Near (range0.DiagonalVectorXY(), diagonalA0);

    Check::Near (diagonalA.x, range0.XLength ());
    Check::Near (diagonalA.y, range0.YLength ());
    Check::Near (diagonalA.z, range0.ZLength ());

    }