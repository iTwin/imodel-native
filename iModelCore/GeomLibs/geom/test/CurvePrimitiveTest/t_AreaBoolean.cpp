//
//
#include "testHarness.h"
#include <Geom/CGWriter.h>
#include <GeomSerialization/GeomSerializationApi.h>

USING_NAMESPACE_BENTLEY_GEOMETRY_INTERNAL

#ifdef indetableDisplay
void Indent (size_t depth)
    {
    printf ("\n");
    for (size_t i = 0; i < depth; i++)
        printf (" ");
    }

void emit (wchar_t c)
    {
    printf ("%lc", c);
    }
void display (WStringR string)
    {
    bvector<int>tagsAtDepth;
    tagsAtDepth.push_back (0);
    int state = 0;    
    for (size_t i = 0; i < string.size (); i++)
        {
        if (string[i] == L'<')
            {
            if (string[i+1] == L'/')
                {
                if (tagsAtDepth.back () != 0)
                    Indent (tagsAtDepth.size ());
                state = -1;
                }
            else
                {
                Indent (tagsAtDepth.size ());
                tagsAtDepth.back () += 1;
                tagsAtDepth.push_back (0);
                state = 1;
                }
            }
        else if (string[i] == L'>')
            {
            if (state == -1)
                {
                if (tagsAtDepth.size () > 1)
                    tagsAtDepth.pop_back ();
                }
            state = 0;
            }
        emit (string[i]);
        }
//    printf ("%ls\n", string.c_str());
    }
#endif
void Print (char const *parentTagName,
    char const *nameA,
    CurveVectorPtr geometryA,
    char const *nameB,
    CurveVectorPtr geometryB,
    char const *nameC,
    CurveVectorPtr geometryC)
    {
    printf ("<%s>\n", parentTagName);
    Check::Print (geometryA, nameA); 
    Check::Print (geometryB, nameB); 
    Check::Print (geometryC, nameC); 
    printf ("</%s>\n", parentTagName);
    }

void CheckReverse (char const* title, CurveVectorPtr regionA)
    {
    if (regionA.IsValid ())
        {
        double areaA, areaB;
        double lengthA, lengthB;
        DPoint3d centroidA, centroidB;
        CurveVectorPtr regionB = regionA->CloneReversed ();
        regionA->CentroidAreaXY (centroidA, areaA);
        regionB->CentroidAreaXY (centroidB, areaB);
        Check::Near (-areaA, areaB, "reversed area");
        lengthA = regionA->Length ();
        lengthB = regionB->Length ();
        Check::Near (lengthA, lengthB, "reversed length");
        }
    }

bool CheckArea (char const* title, CurveVectorPtr region, double expectedArea)
    {
    bool stat = true;
    double area;
    DPoint3d centroid;
    DVec3d normal;
//    Check::Print (region, title);
    if (!region.IsValid ())
        {
        area = 0.0;
        }
    else
        {
        stat = Check::True (region->CentroidNormalArea (centroid, normal, area));
        }
    stat &= Check::Near (expectedArea, area, "Expected Area");
    double hatchLength, hatchArea;
    if (region.IsValid ())
        {
        double length;
        DPoint3d centroid1;
        region->WireCentroid (length, centroid1);
        static double s_hatchSpaceFactor = 0.01;
        static double s_hatchAreaErrorMultiplier = 5.0;
        double spacing = s_hatchSpaceFactor * sqrt (area);    // maybe we get 100 lines?
        DPoint3d startPoint = DPoint3d::From (0,0,0);
        double angleRadians = Angle::DegreesToRadians (30.0);
        CurveVectorPtr hatch = CurveVector::CreateXYHatch (*region, startPoint, angleRadians, spacing);
        hatchLength = hatch->Length ();
        hatchArea = hatchLength * spacing;  // APPROXIMATE !!!!
        stat &= Check::True (fabs (hatchArea - area) < s_hatchAreaErrorMultiplier * s_hatchSpaceFactor * area,
                    "hatch area approximation close to area?");
        }
    if (!stat)
        Check::Print (region, title);
        
    CheckReverse (title, region);
    return stat;
    }

void TestRectangles (double a0, double a1, double b0, double b1)
    {
    if (a1 <= a0 || b1 <= b0)
        return;
    double areaUnion = 0.0;
    double areaIntersection = 0.0;
    double areaDifference = 0.0;
    double areaParity = 0.0;
    double areaA = pow (a1 - a0, 2);
    double areaB = pow (b1 - b0, 2);
    double areaB0A1 = pow (a1 - b0, 2);
    if (a1 <= b0)
        {
        areaUnion = areaA + areaB;
        areaIntersection = 0.0;
        areaDifference = areaA;
        areaParity = areaUnion;
        }
    else if (b0 >= a0)
        {
        if (b1 <= a1)
            {
            // B inside A
            areaUnion = areaA;
            areaIntersection = areaB;
            areaDifference   = areaA - areaB;
            areaParity = areaA - areaB;
            }
        else
            {
            // B hides top right of A
            areaUnion = areaA + areaB - areaB0A1;
            areaIntersection = areaB0A1;
            areaDifference = areaA - areaB0A1;
            areaParity = areaA + areaB - 2.0 * areaB0A1;
            }
        }
    else
        return;


    CurveVectorPtr pathA = CurveVector::CreateRectangle (a0, a0, a1, a1, 0.0, CurveVector::BOUNDARY_TYPE_Outer);
    CurveVectorPtr pathB = CurveVector::CreateRectangle (b0, b0, b1, b1, 0.0, CurveVector::BOUNDARY_TYPE_Outer);

    Transform localToWorld, worldToLocal;
    Check::True (pathA->IsRectangle (localToWorld, worldToLocal));
    Check::True (pathB->IsRectangle (localToWorld, worldToLocal));
    CheckArea ("A", pathA, areaA);
    CheckArea ("B", pathB, areaB);


    CurveVectorPtr cvUnion;
    Print ("CV_Union",
            "A", pathA,
            "B", pathB,
            "Result", (cvUnion = CurveVector::AreaUnion (*pathA, *pathB))
            );
    CheckArea ("Union", cvUnion, areaUnion);
    CurveVectorPtr cvIntersection = CurveVector::AreaIntersection (*pathA, *pathB);
    CheckArea ("Intersection", cvIntersection, areaIntersection);
    CurveVectorPtr cvDifference = CurveVector::AreaDifference (*pathA, *pathB);
    CheckArea ("Difference", cvDifference, areaDifference);
    CurveVectorPtr cvParity = CurveVector::AreaParity (*pathA, *pathB);
    CheckArea ("Parity", cvParity, areaParity);
    }

TEST(AreaBoolean, OverlappingRectangles)
    {
    TestRectangles (0, 4, 3, 5);
    }

TEST(AreaBoolean, DisjointRectangles)
    {
    TestRectangles (0, 4, 5, 6);
    }

TEST(AreaBoolean, OuterInnerRectangles)
    {
    TestRectangles (0, 4, 1, 2);
    }


#ifdef Test3WayBoolean
TEST(AreaBoolean, ABC0)
    {
    double x0 = 0;
    double y0 = 0;
    double dx0 = 20;
    double dy0 = 10;
    double x2 = 9;
    double y2 = 1;
    double dx2 = 10;
    double dy2 = 5;
    double x4 = 1;
    double y4 = 1;
    double dx4 = 3;
    double dy4 = 2;
    // rectangleA is "big"
    // rectangleB is smaller, overalps right of A.
    // rectangleC is entirely within A and outside of B
    CurveVectorPtr pathA = CurveVector::CreateRectangle (x0, y0, x0 + dx0, y0 + dy0, 0.0, CurveVector::BOUNDARY_TYPE_Outer);
    CurveVectorPtr pathB = CurveVector::CreateRectangle (x2, y2, x2 + dx2, y2 + dy2, 0.0, CurveVector::BOUNDARY_TYPE_Outer);
    CurveVectorPtr pathC = CurveVector::CreateRectangle (x4, y4, x4 + dx4, y4 + dy4, 0.0, CurveVector::BOUNDARY_TYPE_Outer);
    
    double areaA = dx0 * dy0;
    double areaB = dx2 * dy2;
    double areaAB = (x0 + dx0 - x2) * (x0 + dy0 - y2 + dy2);
    double areaC = dx4 * dy4;
    double areaAOnly = areaA - areaAB;
    double areaBOnly = areaB - areaAB;

    CheckArea ("A", pathA, areaA);
    CheckArea ("B", pathB, areaB);
    CheckArea ("C", pathC, areaC);

    CurveVectorPtr cvUnion;
    Print ("CV_Union",
            "A", pathA,
            "B", pathB,
            "Result", (cvUnion = CurveVector::AreaUnion (*pathA, *pathB))
            );
    CheckArea ("Union", cvUnion, areaUnion);
    CurveVectorPtr cvIntersection = CurveVector::AreaIntersection (*pathA, *pathB);
    CheckArea ("Intersection", cvIntersection, areaIntersection);
    CurveVectorPtr cvDifference = CurveVector::AreaDifference (*pathA, *pathB);
    CheckArea ("Difference", cvDifference, areaDifference);
    CurveVectorPtr cvParity = CurveVector::AreaParity (*pathA, *pathB);
    CheckArea ("Parity", cvParity, areaParity);
    }
#endif




TEST(AreaBoolean, Splits)
    {
    auto pathA = CurveVector::CreateLinear (
            bvector<DPoint3d> {
                DPoint3d::From (0,0,0),
                DPoint3d::From (0,10,0),
                DPoint3d::From (10,10,0)
                });

    auto pathB = CurveVector::CreateLinear (
            bvector<DPoint3d> {
                DPoint3d::From (5,5,0),
                DPoint3d::From (8,20,0),
                DPoint3d::From (-1,5,0),
                DPoint3d::From(-10,5,0)
                });

    auto cvSplitA = pathA->CloneWithSplits (*pathB);
    auto cvSplitB = pathB->CloneWithSplits (*pathA);
    Check::SaveTransformed (*pathA);
    Check::SaveTransformed (*pathB);
    Check::Shift (20,0,0);
    Check::SaveTransformed (*cvSplitA);
    Check::SaveTransformed (*cvSplitB);

    auto cvSplitA1 = pathA->CloneWithSplits (*pathB, true);
    auto cvSplitB1 = pathB->CloneWithSplits (*pathA, true);
    Check::Shift (20, 0, 0);
    Check::SaveTransformed (*cvSplitA1);
    Check::SaveTransformed (*cvSplitB1);

    Check::ClearGeometry ("AreaBoolean.Splits");
    }

TEST(AreaBoolean, RectangleSplitEllipse)
    {
    double a = 10.0;
    double r = 4;
    DEllipse3d circle = DEllipse3d::From (a,0.5 * a, 0,
                r,0,0,
                0,r,0,
                Angle::Pi () * 0.0, Angle::TwoPi ());
    CurveVectorPtr rectangle = CurveVector::CreateRectangle (0,0,a,a,0);
    CurveVectorPtr disk    = CurveVector::CreateDisk      (circle);

    double aR = a * a;                      // area of rectangle
    double aHC = 0.5 * Angle::Pi () * r * r;    // area of half of disk
    CheckArea ("10x10 Rectangle", rectangle, aR);
    CheckArea ("R4 Disk Centered at rectangle right", disk, 2.0 * aHC);

    // Half the circle is outside the rectangle....
    //CheckArea ("Union",  CurveVector::AreaUnion (*rectangle, *disk), aR + aHC);
    //CheckArea ("Rectangle minus Circle",  CurveVector::AreaDifference (*rectangle, *disk), aR - aHC);
    CheckArea ("Circle minus Rectangle",  CurveVector::AreaDifference (*disk, *rectangle), aHC);
    //CheckArea ("Intersection",  CurveVector::AreaIntersection (*rectangle, *disk), aHC);
    //CheckArea ("Parity",  CurveVector::AreaParity (*rectangle, *disk), aR);


    }

#ifdef CanReadBexXMLHere
static     WCharCP s_areaA = 
L"\
<Polygon>\
<ListOfPoint>\
  <xyz>0,0,0</xyz> \
  <xyz>640,0,0</xyz> \
  <xyz>640,512,0</xyz> \
  <xyz>0,512,0</xyz> \
  <xyz>0,0,0</xyz> \
  </ListOfPoint>\
</Polygon>\
";
static wchar_t *s_areaB =
L"\
<CurveChain>\
<ListOfCurve>\
  <LineString>\
    <ListOfPoint>\
          <xyz>399.652604135361,87.0137533141434,0</xyz> \
          <xyz>399.652604135361,230.366087345624,0</xyz> \
          <xyz>203.435077605827,230.366087345624,0</xyz> \
          <xyz>203.435077605827,72.2788323185172,0</xyz> \
    </ListOfPoint>\
  </LineString>\
  <CircularArc>\
      <placement>\
      <origin>295.012391491642,166.62231679171,0</origin> \
      <vectorZ>0,0,1</vectorZ> \
      <vectorX>1,0,0</vectorX> \
      </placement>\
      <radius>131.480407213716</radius> \
      <startAngle>225.852396334914</startAngle> \
      <sweepAngle>7.95425604508042</sweepAngle> \
  </CircularArc>\
    <LineSegment>\
        <startPoint>217.371637111736,60.5138310782494,0</startPoint> \
        <endPoint>372.653145871547,60.5138310782494,0</endPoint> \
    </LineSegment>\
    <CircularArc>\
        <placement>\
        <origin>295.012391491642,166.62231679171,0</origin> \
      <vectorZ>0,0,1</vectorZ> \
      <vectorX>1,0,0</vectorX> \
        </placement>\
          <radius>131.480407213716</radius> \
          <startAngle>306.193347620005</startAngle> \
          <sweepAngle>16.5433680946062</sweepAngle> \
    </CircularArc>\
  </ListOfCurve>\
</CurveChain>\
";

CurveVectorPtr ReadCurveVector (Utf8String source)
    {
    bvector<IGeometryPtr> geometry;
    BeXmlCGStreamParser::TryParse (source, geometry);
    CurveVectorPtr result;
    ICurvePrimitivePtr primitive;
    if (geometry.size () == 1)
        {
        primitive = geometry[0]->GetAsICurvePrimitive ();
        if (primitive.IsValid ())
            {
            CurveVectorCP child = primitive->GetChildCurveVectorCP ();
            if (NULL != child)
                result = child->Clone ();
            }
        }
    return result;
    }


TEST(Mathieu,TestA)
    {
    CurveVectorPtr areaA = ReadCurveVector (s_areaA);
    CurveVectorPtr areaB = ReadCurveVector (s_areaB);
    if (areaA.IsValid () && areaB.IsValid ())
        {
        Check::Print (areaA, "A");
        Check::Print (areaB, "B");
        CurveVectorPtr areaAB = CurveVector::AreaIntersection (*areaA, *areaB);
        Check::Print (areaAB, "Intersection");
        }

    }
#endif
TEST(Mathieu,TestB)
    {
    CheckBSIBaseGeomMemory outerMemory ("MathieuTestB");
    bvector<DPoint3d> poles;
    poles.push_back(DPoint3d::From (1,0,0));
    poles.push_back(DPoint3d::From (1,1,0));
    poles.push_back(DPoint3d::From (-1,1,0));
    poles.push_back(DPoint3d::From (-1,0,0));
    MSBsplineCurvePtr upperSpline = MSBsplineCurve::CreateFromPolesAndOrder (poles, NULL, NULL, 3, false, false);
    MSBsplineCurvePtr upperSpline1 = MSBsplineCurve::CreateFromPolesAndOrder (poles, NULL, NULL, 3, false, false);
    CurveVectorPtr splineArea = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Outer);
    splineArea->push_back (ICurvePrimitive::CreateBsplineCurve (*upperSpline));
    splineArea->push_back (ICurvePrimitive::CreateLine (DSegment3d::From (-1,0,0, 1,0,0)));
    CurveVectorPtr rectangleA = CurveVector::CreateRectangle (0,-1,2,2,0,CurveVector::BOUNDARY_TYPE_Outer);
        {
        CheckBSIBaseGeomMemory mem ("AreaUnion");
        CurveVectorPtr boolA = CurveVector::AreaUnion (*splineArea, *rectangleA);
        Check::Print (boolA, "RectangleA Union CurveHalfDisk");
        }

    CurveVectorPtr rectangleB = CurveVector::CreateRectangle (0.5,0.2,2,2,0,CurveVector::BOUNDARY_TYPE_Outer);
    CurveVectorPtr boolB = CurveVector::AreaUnion (*splineArea, *rectangleB);
    Check::Print (boolB, "RectangleB Union CurveHalfDisk");


    }

TEST(Area, WindingNumber1)
    {
    // This path loops around over itself ...
    bvector<DPoint3d> corners;
    corners.push_back (DPoint3d::From (0,0));
    corners.push_back (DPoint3d::From (4,0));
    corners.push_back (DPoint3d::From (4,5));
    corners.push_back (DPoint3d::From (3,5));
    corners.push_back (DPoint3d::From (3,4));
    corners.push_back (DPoint3d::From (6,4));
    corners.push_back (DPoint3d::From (6,8));
    corners.push_back (DPoint3d::From (0,8));
    corners.push_back (DPoint3d::From (0,0));
    CurveVectorPtr region = CurveVector::CreateLinear (corners, CurveVector::BOUNDARY_TYPE_Outer, false);
    DPoint3d centroid;
    double area0, area1, area2;
    // this will see the inner square twice --
    region->CentroidAreaXY (centroid, area0);

    // Now not at all....
    CurveVectorPtr cvParity = CurveVector::AreaAnalysis (*region, AreaSelect_Parity, BoolSelect_Union, false);
    cvParity->CentroidAreaXY (centroid, area2);
    
    
    CurveVectorPtr cvOuter = CurveVector::AreaAnalysis (*region, AreaSelect_CCWPositiveWindingNumber, BoolSelect_Union, false);
    Check::Print (cvOuter, "PositiveWinding");
    cvOuter->CentroidAreaXY (centroid, area1);
    Check::Near (39.0, area2, "Parity area");
    Check::Near (41.0, area0, "Area with double counted winding");
    Check::Near (40.0, area1, "Area with corrected winding");
    }
    
TEST(Area, WindingNumber2)
    {
    double a0 = 100;
    double a1 = 10;
    double A0 = a0 * a0;
    double A1 = a1 * a1;
    CurveVectorPtr rectangleA = CurveVector::CreateRectangle(0,0,   100,100, 0);
    CurveVectorPtr rectangleB = CurveVector::CreateRectangle(10,10,  20,20, 0);
    CurveVectorPtr parent = CurveVector::Create (CurveVector::BOUNDARY_TYPE_UnionRegion);
    parent->push_back (ICurvePrimitive::CreateChildCurveVector (rectangleA));
    parent->push_back (ICurvePrimitive::CreateChildCurveVector (rectangleB));

    
    DPoint3d centroid;
    double area0, area1, area2;
    // this will see the inner square twice --
    parent->CentroidAreaXY (centroid, area0);

    // Now not at all....
    CurveVectorPtr cvParity = CurveVector::AreaAnalysis (*parent, AreaSelect_Parity, BoolSelect_Parity, false);
    cvParity->CentroidAreaXY (centroid, area2);
    Check::Print (cvParity, "Parity");

    CurveVectorPtr cvOuter = CurveVector::AreaAnalysis (*parent, AreaSelect_CCWPositiveWindingNumber, BoolSelect_Union, false);

    Check::Print (cvOuter, "PositiveWinding");
    cvOuter->CentroidAreaXY (centroid, area1);
    Check::Near (A0 - A1, area2, "Parity area");
    Check::Near (A0 + A1, area0, "Area with double counted winding");
    Check::Near (A0, area1, "Area with corrected winding");
    }


TEST(Area, WindingNumber3)
    {
    CurveVectorPtr disk0 = CurveVector::CreateDisk (DEllipse3d::FromVectors
        (
        DPoint3d::From (0,0,0),
        DVec3d::From (10,0,0),
        DVec3d::From (0,10,0),
        0.0, Angle::TwoPi ()
        ));
    CurveVectorPtr rectangle = CurveVector::CreateRectangle (
        -20,-20,
        20,5,
        0.0,
        CurveVector::BOUNDARY_TYPE_Outer);
    CurveVectorPtr parent = CurveVector::Create (CurveVector::BOUNDARY_TYPE_ParityRegion);
    parent->push_back (ICurvePrimitive::CreateChildCurveVector (disk0));
    parent->push_back (ICurvePrimitive::CreateChildCurveVector (rectangle));
    CurveVectorPtr cvParity = CurveVector::AreaAnalysis (*parent, AreaSelect_Parity, BoolSelect_Union, false);
    Check::Print (parent, "Venn Parent");
    Check::Print (cvParity, "Venn Parity");
    }


TEST(Area, WindingNumber4)
    {
    CurveVectorPtr disk0 = CurveVector::CreateDisk (DEllipse3d::FromVectors
        (
        DPoint3d::From (0,0,0),
        DVec3d::From (10,0,0),
        DVec3d::From (0,10,0),
        0.0, Angle::TwoPi ()
        ));
    CurveVectorPtr disk1 = CurveVector::CreateDisk (DEllipse3d::FromVectors
        (
        DPoint3d::From (15,0,0),
        DVec3d::From (12,0,0),
        DVec3d::From (0,12,0),
        0.0, Angle::TwoPi ()
        ));
    CurveVectorPtr disk2 = CurveVector::CreateDisk (DEllipse3d::FromVectors
        (
        DPoint3d::From (8,10,0),
        DVec3d::From (9,0,0),
        DVec3d::From (0,9,0),
        0.0, Angle::TwoPi ()
        ));
    CurveVectorPtr parent = CurveVector::Create (CurveVector::BOUNDARY_TYPE_ParityRegion);
    parent->push_back (ICurvePrimitive::CreateChildCurveVector (disk0));
    parent->push_back (ICurvePrimitive::CreateChildCurveVector (disk1));
    parent->push_back (ICurvePrimitive::CreateChildCurveVector (disk2));
    CurveVectorPtr cvParity = CurveVector::AreaAnalysis (*parent, AreaSelect_Parity, BoolSelect_Union, false);
    Check::Print (parent, "Venn Parent");
    Check::Print (cvParity, "Venn Parity");

    }


static void TriangleProducts1
(
DPoint3dCR pointA,
DPoint3dCR pointB,
DPoint3dCR pointC,
RotMatrixR products,
DVec3dR    moment1,
double     &area
)
    {
    DMatrix4d referenceMoments = DMatrix4d::FromRowValues (
        2,1,0,4,
        1,2,0,4,
        0,0,0,0,
        4,4,0,1
        );
    DVec3d vectorAB = DVec3d::FromStartEnd (pointA, pointB);
    DVec3d vectorAC = DVec3d::FromStartEnd (pointA, pointC);
    DVec3d unitPerp;
    double detJ = unitPerp.NormalizedCrossProduct (vectorAB, vectorAC);
    Transform placement = Transform::FromOriginAndVectors (pointA, vectorAB, vectorAC, unitPerp);
    DMatrix4d spaceMoments = DMatrix4d::FromSandwichProduct (placement, referenceMoments, detJ /24);
    DVec3d moment1A;
    double a;
    spaceMoments.ExtractAroundPivot (products, moment1A, moment1, a, 3);
    area = detJ * 0.5;
    }

/* UNREFERENCED LOCAL FUNCTION----------------------------------------------------------------------------

static void TetrahedralMoments
(
DPoint3dCR pointA,
DPoint3dCR pointB,
DPoint3dCR pointC,
DPoint3dCR pointD,
RotMatrixR products,
DVec3dR    moment1,
double     &volume
)
    {
    DMatrix4d referenceMoments =
        {
        2,1,1,5,
        1,2,1,5,
        1,1,2,5,
        5,5,5,1
        };
    DVec3d vectorAB = DVec3d::FromStartEnd (pointA, pointB);
    DVec3d vectorAC = DVec3d::FromStartEnd (pointA, pointC);
    DVec3d vectorAD = DVec3d::FromStartEnd (pointA, pointD);
    double detJ = vectorAB.NormalizedCrossProduct (vectorAC, vectorAD);
    Transform placement = Transform::FromOriginAndVectors (pointA, vectorAB, vectorAC, vectorAD);
    DMatrix4d spaceMoments = DMatrix4d::FromSandwichProduct (placement, referenceMoments, detJ /120);
    DVec3d moment1A;
    double a;
    spaceMoments.ExtractAroundPivot (products, moment1A, moment1, a, 3);
    area = detJ / 6.0;
    }

--------------------------------------------------------------------------------------------------------*/


static void TriangleProducts
(
DPoint3dCR pointA,
DPoint3dCR pointB,
DPoint3dCR pointC,
RotMatrixR products,
DVec3dR    moment1,
double     &area
)
    {
    static const double div3 = 1.0 / 3.0;
    static const double div4 = 0.25;
    static const double div6 = 1.0 / 6.0;
    static const double div8 = 0.125;
    static const double div12 = 1.0 / 12.0;
    DVec3d A = DVec3d::From (pointA);
    DVec3d U = DVec3d::FromStartEnd (pointA, pointB);
    DVec3d V = DVec3d::FromStartEnd (pointB, pointC);
    RotMatrix E;
    E.Zero ();
    E.AddScaledOuterProductInPlace (A, A, 0.5);

    E.AddScaledOuterProductInPlace (A, U, div3);
    E.AddScaledOuterProductInPlace (U, A, div3);

    E.AddScaledOuterProductInPlace (A, V, div6);
    E.AddScaledOuterProductInPlace (V, A, div6);

    E.AddScaledOuterProductInPlace (U, U, div4);

    E.AddScaledOuterProductInPlace (U, V, div8);
    E.AddScaledOuterProductInPlace (V, U, div8);

    E.AddScaledOuterProductInPlace (V, V, div12);
    DVec3d J;
    double a = J.NormalizedCrossProduct (U, V);
    products.ScaleColumns (E, a, a, a);
    double b = a / 6.0;
    moment1.x = (pointA.x + pointB.x + pointC.x) * b;
    moment1.y = (pointA.y + pointB.y + pointC.y) * b;
    moment1.z = (pointA.z + pointB.z + pointC.z) * b;
    area = a * 0.5;
    }


void SweptPolylineMoments (DPoint3dCR origin, DPoint3d *points, int numPoints,
                void (F)(DPoint3dCR, DPoint3dCR, DPoint3dCR, RotMatrixR, DVec3dR, double &), DMatrix4dR moments)
    {
    RotMatrix E2, F2;
    DVec3d    E1, F1;
    double e, f;
    F2.Zero ();
    F1.Zero ();
    f = 0.0;
    for (int i = 1; i < numPoints; i++)
        {
        F(origin, points[i-1], points[i], E2, E1, e);
        F2.Add (E2);
        F1.Add (E1);
        f += e;
        }
    moments.InitFromRowValues (
        F2.form3d[0][0], F2.form3d[0][1], F2.form3d[0][2], F1.x,
        F2.form3d[1][0], F2.form3d[1][1], F2.form3d[1][2], F1.y,
        F2.form3d[2][0], F2.form3d[2][1], F2.form3d[2][2], F1.z,
        F1.x,            F1.y,            F1.z,             f);
    }

TEST(Moments, Test1)
    {
    DPoint3d point[3] = {
        {1,0,0},
        {1,1,0},
        {0,1,1}
        };
    RotMatrix E[3];
    DVec3d    M[3];
    double    area[3];
    for (int i = 0; i < 3; i++)
        {
        int j = (i + 1) % 3;
        int k = (j + 1) % 3;
        TriangleProducts (point[i], point[j], point[k], E[i], M[i], area[i]);
        }
    for (int i = 1; i < 3; i++)
        {
        Check::Near (E[0], E[i], "Integral2 independent of order");
        Check::Near (M[0], M[i], "Integral1 independent of order");
        Check::Near (area[0], area[i], "Integral0 independent of order");
        }
    }

TEST(Moments, Test2)
    {
    DPoint3d centeredSquare[5] =
        {
        {1,-1,0},
        {1,1,0},
        {-1,1,0},
        {-1,-1,0},
        {1,-1,0}
        };
    DPoint3d Zero;
    Zero.Zero ();
    // 2x2 square centered at zero ===> xx,yy integrals are 4/3
    double a = 4.0 / 3.0;
    DMatrix4d E0 = DMatrix4d::FromRowValues (
        a,0,0, 0,
        0,a,0, 0,
        0,0,0, 0,
        0,0,0, 4);
    DMatrix4d E, F;
    SweptPolylineMoments (Zero, centeredSquare, 5, TriangleProducts, E);
    SweptPolylineMoments (Zero, centeredSquare, 5, TriangleProducts1, F);
    Check::Near (E0, E, "centered square");
    Check::Near (E0, F, "centered square");
    // 1x1 square with lower left at 00 ===> xx,yy integrals are 1/3
    DPoint3d unitSquare [5] =
        {
        {0,0,0},
        {1,0,0},
        {1,1,0},
        {0,1,0},
        {0,0,0}
        };
    double b = 1.0 / 3.0;
    double c = 0.25;
    double d = 0.5;
    E0 = DMatrix4d::FromRowValues
            (
            b,c,0, d,
            c,b,0, d,
            0,0,0, 0,
            d,d,0, 1);
    SweptPolylineMoments (unitSquare[3], unitSquare, 5, TriangleProducts, E);
    SweptPolylineMoments (unitSquare[3], unitSquare, 5, TriangleProducts, F);
    Check::Near (E0, E, "square from lower left");
    Check::Near (E0, F, "square from lower left");
                
    
    }



TEST(Moments, Test3)
    {
    DPoint3d point[3] = {
        {0,0,0},
        {1,0,0},
        {0,1,0}
        };
    RotMatrix E[3], F[3];
    DVec3d    M[3], N[3];
    double    area[3], detj[3];
    for (int i = 0; i < 3; i++)
        {
        int j = (i + 1) % 3;
        int k = (j + 1) % 3;
        TriangleProducts (point[i], point[j], point[k], E[i], M[i], area[i]);
        TriangleProducts1 (point[i], point[j], point[k], F[i], N[i], detj[i]);
        }
    for (int i = 1; i < 3; i++)
        {
        Check::Near (E[0], E[i], "Integral2 independent of order");
        Check::Near (M[0], M[i], "Integral1 independent of order");
        Check::Near (area[0], area[i], "Integral0 independent of order");
        }
    }


TEST(SplitCurves, Test1)
    {
    bvector<DPoint3d> rectanglePoints;
    rectanglePoints.push_back (DPoint3d::From (-200,-200,0));
    rectanglePoints.push_back (DPoint3d::From ( 200,-200,0));
    rectanglePoints.push_back (DPoint3d::From ( 200, 100,0));
    rectanglePoints.push_back (DPoint3d::From (-200, 100,0));
    rectanglePoints.push_back (DPoint3d::From (-200,-200,0));
    CurveVectorPtr rectangle = CurveVector::CreateLinear (rectanglePoints, CurveVector::BOUNDARY_TYPE_Outer);
    double radius = 100.0 * sqrt (2.0);
    CurveVectorPtr disk = CurveVector::CreateDisk (
                    DEllipse3d::From (0,0,0,   radius, 0,0,   0,radius,0,  0.0, Angle::TwoPi ()));
                    
    CurveVectorPtr insideCurves = CurveVector::Create (CurveVector::BOUNDARY_TYPE_None);
    CurveVectorPtr onCurves = CurveVector::Create (CurveVector::BOUNDARY_TYPE_None);
    CurveVectorPtr outsideCurves = CurveVector::Create (CurveVector::BOUNDARY_TYPE_None);
    disk->AppendSplitCurvesByRegion (*rectangle, insideCurves.get (), outsideCurves.get (), onCurves.get ());
    Check::Print (rectangle, "rectangle");
    Check::Print (disk, "disk");
    Check::Print (insideCurves, "inside");
    Check::Print (outsideCurves, "outside");
    Check::Print (onCurves, "on");
    
    }

bool CheckAreaXY (CurveVectorCR curves, double expectedArea, char *name)
    {
    double area;
    DPoint3d centroid;
    return
        Check::True (curves.CentroidAreaXY (centroid, area), name)
        && Check::Near (expectedArea, area, name);
    }    
#ifdef abc
TEST(ParityFixup, Test1)
    {
    bvector<DPoint3d> rectanglePoints;
    double a = 200.0;
    double b = 100.0;
    rectanglePoints.push_back (DPoint3d::From (-a,-a,0));
    rectanglePoints.push_back (DPoint3d::From ( a,-a,0));
    rectanglePoints.push_back (DPoint3d::From ( a, b,0));
    rectanglePoints.push_back (DPoint3d::From (-a, b,0));
    rectanglePoints.push_back (DPoint3d::From (-a,-a,0));
    CurveVectorPtr rectangle1 = CurveVector::CreateLinear (rectanglePoints, CurveVector::BOUNDARY_TYPE_Outer);

    double expectedArea= 2 * a * (a + b);
    CheckAreaXY (*rectangle1, expectedArea, "CW rectangle area");
    Transform mirrorX = Transform::FromRowValues (
            -1,  0,0, 0,
            0,   1,0, 0,
            0,  0, 1, 0);   
    CurveVectorPtr rectangle2 = rectangle1->Clone ();
    rectangle2->TransformInPlace (mirrorX);
    CheckAreaXY (*rectangle2, -expectedArea, "CW rectangle area");
    
    CurveVectorPtr rectangle3 = CurveVector::AreaWindingFixup (*rectangle2);
    CheckAreaXY (*rectangle3, 0.0, "CW after winding fixup");
    }
 #endif
 
 
 TEST(FixupXY,DisjointShapes)
    {
    CurveVectorPtr shape1 = CurveVector::CreateRectangle (0,0,300,400, 0);
    CurveVectorPtr shape2 = CurveVector::CreateRectangle (400, 0,600,400, 0);
    CurveVectorPtr unionRegion = CurveVector::Create (CurveVector::BOUNDARY_TYPE_UnionRegion);
    unionRegion->Add (shape1);
    unionRegion->Add (shape2);
    unionRegion->FixupXYOuterInner (false);
    
    }

 TEST(FixupXY,FullCircle)
    {
    CurveVectorPtr disk = CurveVector::CreateDisk (DEllipse3d::From (0,0,0,   1,0,0,   0,1,0, 0.0, Angle::TwoPi ()));
    Check::Print (disk, "original disk"); 
    disk->FixupXYOuterInner (true);
    Check::Print (disk, "After area fixup"); 
    disk->ConsolidateAdjacentPrimitives ();
    Check::Print (disk, "After area ConsolidateAdjacentPrimitives"); 
    }


TEST(FixupXY,BowTie)
    {
    bvector<DPoint3d> poles
        {
            DPoint3d::From (2,4,0),
            DPoint3d::From (4,0,0),
            DPoint3d::From (6,0,0),
            DPoint3d::From (6,4,0),
            DPoint3d::From (4,4,0),
            DPoint3d::From (2,0,0),
            DPoint3d::From (0,0,0),
            DPoint3d::From (0,4,0)
        };
    MSBsplineCurvePtr bcurve0 = MSBsplineCurve::CreateFromPolesAndOrder (poles, nullptr, nullptr, 3, true, true);
    ICurvePrimitivePtr cp0 = ICurvePrimitive::CreateBsplineCurve (*bcurve0);
    CurveVectorPtr cv0 = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Outer);
    cv0->Add (cp0);

   Check::Print (cv0, "original disk"); 
    cv0->FixupXYOuterInner (true);
    Check::Print (cv0, "After area fixup"); 
    cv0->ConsolidateAdjacentPrimitives ();
    Check::Print (cv0, "After area ConsolidateAdjacentPrimitives"); 


    }

TEST(ClipToPlane,ElenieA)
{
bvector<DPoint3d> triangle
    {
    DPoint3d::From (2141280.703423,224609.158903,788.480080),
    DPoint3d::From (2141279.740000,224612.010000,788.15000),
    DPoint3d::From (2141266.370000,224628.760000,788.370000),
    };
DPlane3d plane;
plane.origin = DPoint3d::From (2141445.293750,224617.222500,14144.170000);
plane.normal = DVec3d::From (0.000000,1.000000,0.000000);
#define MAX_CLIP_POINTS 1000
DPoint3d clipPoints[MAX_CLIP_POINTS];
int nClip;
int numLoop;
bsiPolygon_clipToPlane(clipPoints, &nClip, &numLoop, MAX_CLIP_POINTS,
    &triangle[0], (int)triangle.size (), &plane);
#ifdef PrintClip
for (size_t i = 0; i < nClip; i++)
    {
    if (!clipPoints[i].IsDisconnect ())
        printf (" %.17g", plane.Evaluate (clipPoints[i]));
    }
#endif
    Check::Int (numLoop, 1);
    Check::Int (nClip, 6);
}

TEST(ClipToPlane,ElenieB)
{
bvector<DPoint3d> triangle
    {
    DPoint3d::From (-1,-1,0),
    DPoint3d::From (1,-1,0),
    DPoint3d::From (1,1,0),
    };
DPlane3d plane;
plane.origin = DPoint3d::From (0,0.25,0);
plane.normal = DVec3d::From (0.000000,1.000000,0.000000);
#define MAX_CLIP_POINTS 1000
DPoint3d clipPoints[MAX_CLIP_POINTS];
int nClip;
int numLoop;
bsiPolygon_clipToPlane(clipPoints, &nClip, &numLoop, MAX_CLIP_POINTS,
    &triangle[0], (int)triangle.size (), &plane);
    Check::Int (numLoop, 1);
    Check::Int (nClip, 6);
}

TEST(ClipToPlane,ElenieC)
{
#define MAX_CLIP_POINTS 1000
DPoint3d clipPoints[MAX_CLIP_POINTS];
int nClip;
int numLoop;
for (double scale = 1.0; scale > 2.0e-7; scale *= 0.01)
    {
    //GEOMAPI_PRINTF ("Triangle scale %.17g\n", scale);
    bvector<DPoint3d> triangle
        {
        DPoint3d::From (2141280.703423,224609.158903,788.480080),
        DPoint3d::From (2141279.740000,224612.010000,788.15000),
        DPoint3d::From (2141266.370000,224628.760000,788.370000),
        };
    for (DPoint3d &xyz : triangle)
        {
        xyz.Scale (scale);
        }
    DPlane3d plane;
    plane.origin = DPoint3d::From (scale * 2141445.293750, scale * 224617.222500,scale *14144.170000);
    plane.normal = DVec3d::From (0.000000,1.000000,0.000000);


    bsiPolygon_clipToPlane(clipPoints, &nClip, &numLoop, MAX_CLIP_POINTS,
        &triangle[0], (int)triangle.size (), &plane);
    Check::Int (numLoop, 1);
    Check::Int (nClip, 6);
    }
}


TEST(CurveVector,AreaBooleanMultipleAreas)
    {
    auto oldVolume = Check::SetMaxVolume (1000);
    auto nullShapes = CurveVector::Create (CurveVector::BOUNDARY_TYPE_UnionRegion);
    auto allShapes = CurveVector::Create (CurveVector::BOUNDARY_TYPE_UnionRegion);
    for (int i = 0; i < 5; i++)
        {
        double x0 = (double)i;
        auto r = CurveVector::CreateRectangle (x0, 0.0, x0 + 1, 1.0, 0.0, CurveVector::BOUNDARY_TYPE_Outer);
        allShapes->Add (r);
        }
    auto result = CurveVector::AreaUnion (*allShapes, *nullShapes);
    Check::Print (*allShapes, "Input");
    Check::PrintStructure (result.get ());
    if (result.IsValid ())
        {
        Check::Print (*result, "Union");
        result->ConsolidateAdjacentPrimitives (true);
        Check::Print (*result, "After ConsolidateAdjacentPrimitives");
        }
    Check::SetMaxVolume (oldVolume);
    }

TEST(Polyface,BoundaryFromCompressedFacets)
    {
    auto oldVolume = Check::SetMaxVolume (1000);
    PolyfaceHeaderPtr facets = PolyfaceHeader::CreateVariableSizeIndexed ();
    double y0 = 0;
    int maxI = 14;
    int iStep = maxI / 3;
    int numJ = 0;
    for (int numI = maxI; numI > 0; numI -= iStep)
        {
        numJ++;
        double y1 = y0 + 1;
        for (int i = 0; i < numI; i++)
            {
            double x0 = (double)i;
            double x1 = x0 + 1;
            bvector<DPoint3d> points
                {
                DPoint3d::From (x0, y0),
                DPoint3d::From (x1, y0),
                DPoint3d::From (x1, y1),
                DPoint3d::From (x0, y1)
                };
            facets->AddPolygon (points);
            }
        y0 += 1;
        }
    facets->Compress ();
    size_t numOpen = 0;
    size_t numClosed = 0;
    facets->MarkTopologicalBoundariesVisible (false);
    auto boundaries = facets->ExtractBoundaryStrings (numOpen, numClosed);
    Check::Size (0, numOpen, "NumOpen");
    Check::Size (1, numClosed, "NumClosed");
    double outerLength = 2.0 * (maxI + numJ);
    Check::Near (outerLength, boundaries->Length (), "Boundary length");
    {
    SaveAndRestoreCheckTransform shifter (maxI + 2.0, 0,0);
    Check::SaveTransformed (*facets);
    Check::Shift (0,y0 + 2,0);
    Check::SaveTransformed (*boundaries);
    }
//    PrintPolyface (*facets, "facets", stdout, 10000, true);
//    Check::Print (*boundaries, "Boundaries from Compressed facets");

    bvector<int> &pointIndex = facets->PointIndex ();
    size_t numReadIndex = pointIndex.size ();
    // for "various" readIndices, toggle their visibility and make sure ExtractBoundaryString returns with at most 2 paths
    size_t skip = 7;//numReadIndex / 5;
    for (size_t readIndex = 0; readIndex < numReadIndex; readIndex += skip)
        {
        int ri = pointIndex[readIndex];
        if (ri != 0)
            {
            // flip its visibility
            pointIndex[readIndex] = -ri;
            auto b1 = facets->ExtractBoundaryStrings (numOpen, numClosed);
            {
            SaveAndRestoreCheckTransform shifter (maxI + 2.0, 0,0);
            Check::SaveTransformed (*facets);
            Check::Shift (0,y0 + 2,0);
            Check::SaveTransformed (*b1);
            }

            if (ri > 0)
                {
                // an outer edge was hidden -- there is a break in the outer loop.  BUT .. the stitcher
                //    forces it visbile. So we expect no change
                Check::True (numClosed == 1, "Forced fill of outer boundary error");
                Check::Size (0, numOpen, "Forced fill of outer boundary error");
                Check::Near (outerLength, b1->Length (), "Expect reduced length of boundary");
                }
            else
                {
                // an interior edge was made visible (but only on one side?)  
                Check::True (numOpen + numClosed <= 2, "Expected limit on boundary paths?");
                double l1 = b1->Length ();
                Check::True (l1 > outerLength && l1 <= outerLength + 2.0, "Expect one or two additional visible edges in interior");
                }

            pointIndex[readIndex] = ri;
            }
        }
    Check::SetMaxVolume (oldVolume);
    Check::ClearGeometry ("Polyface.BoundaryFromCompressedFacets");
    }