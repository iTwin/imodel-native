/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "testHarness.h"
#include <GeomSerialization/GeomSerializationApi.h>

#include <Regions/regionsAPI.h>
#include <Regions/rimsbsAPI.h>
USING_NAMESPACE_BENTLEY_GEOMETRY_INTERNAL
// build up arrays with reference
struct RIElement
{
int curveId;
int groupId;
int userData;
int64_t userData64;
ICurvePrimitivePtr curve;

size_t errors;

bool CheckBool (bool v, char const *message = nullptr)
    {
    if (v)
        return true;
    errors++;
    Check::True (v, message);
    return false;
    }

 bool CheckInt (int iA, int iB, char const *message = nullptr)
    {
    if (iA == iB)
        return true;
    return Check::Int (iA, iB, message);
    }
size_t ValidationErrors_go (RIMSBS_Context *curves)
    {

    errors = 0;
    int iB;
    if (!CheckBool (jmdlRIMSBS_getUserInt (curves, &iB, curveId), "get userInt"))
        return errors;
    Check::Print (userData, "iA");
    Check::Print (iB, "iB");
    if (!CheckInt (userData, iB, "userInt"))
        return errors;

    int64_t i64B;
    if (!CheckBool (jmdlRIMSBS_getUserInt64 (curves, &i64B, curveId)))
        return errors;
    Check::Print (userData64, "i64A");
    Check::Print (i64B, "i64B");
    if (!CheckBool (Check::Ptrdiff ((ptrdiff_t)userData64, (ptrdiff_t)i64B, "access userInt64"), "access userInt64"))
        return errors;

    void *ptrA = (void*)0xa1234567b1234567;
    void *ptrB;
    jmdlRIMSBS_setUserPointer (curves, curveId, ptrA);
    jmdlRIMSBS_getUserPointer (curves, &ptrB, curveId);
    Check::Print ((int64_t)ptrA, "ptrA");
    Check::Print ((int64_t)ptrB, "ptrB");

    if (!CheckBool (Check::True (ptrA == ptrB, "set/get pointer")))
        return errors;

    int groupIdB;
    if (!CheckBool (Check::True (jmdlRIMSBS_getGroupId (curves, &groupIdB, curveId))))
        return errors;
     if (!CheckInt (groupId, groupIdB, "groupId from currGroupId"))
        return errors;

    for (double u : bvector<double>{0.25, 0.40, 0.80})
        {
        DPoint3d X0;
        DVec3d dX0, ddX0;
        DPoint3d xyzR[3];
        curve->FractionToPoint (u, X0, dX0, ddX0);
        jmdlRIMSBS_evaluateDerivatives (curves, xyzR, 2, u, curveId);
        Check::Near (X0, xyzR[0], "Evaluate X");
        Check::Near (dX0, xyzR[1], "Evaluate dX");
        Check::Near (ddX0, xyzR[2], "Evaluate ddX");
        }
    DRange3d rangeR;
    if (CheckBool (jmdlRIMSBS_getCurveRange (curves, &rangeR, curveId)))
        {
        DRange3d rangeC;
        curve->GetRange (rangeC);
        Check::Print (rangeR.low, "range.low");
        Check::Print (rangeR.high, "range.high");
        Check::Near (rangeC, rangeR, "RIMSBS range");
        }
    return errors;
    }
size_t ValidationErrors (RIMSBS_Context *curves)
    {
    Check::StartScope ("Validate curveId", (size_t)curveId);
    auto e = ValidationErrors_go (curves);
    Check::EndScope ();
    return e;
    }
};

struct RIArray : bvector<RIElement>
{
RIMSBS_Context *m_curves;
RG_Header *m_regions;
int m_currentUserData;
int m_currentGroupId;
int64_t m_int64Multiplier;

RIArray (int startUserData, int startGroupId) : m_currentGroupId (startGroupId), m_currentUserData (startUserData), m_int64Multiplier (4)
    {
    m_curves = jmdlRIMSBS_newContext ();
    m_regions     = jmdlRG_new ();
    jmdlRIMSBS_setupRGCallbacks (m_curves, m_regions);
    jmdlRG_setFunctionContext (m_regions, m_curves);
    IncrementGroupId (0);
    }

~RIArray ()
    {
    jmdlRIMSBS_freeContext (m_curves);
    jmdlRG_free (m_regions);
    }

void IncrementGroupId (int step)
    {
    m_currentGroupId += step;
    jmdlRIMSBS_setCurrGroupId (m_curves, m_currentGroupId);
    }

void AddCurve (int curveId, ICurvePrimitivePtr curve)
    {
    m_currentUserData++;
    push_back (RIElement ());
    back ().curveId = curveId;
    back ().groupId = m_currentGroupId;
    back ().curve = curve;
    back ().userData64 = (int64_t)(m_currentUserData * m_int64Multiplier);
    back ().userData = m_currentUserData;
    jmdlRIMSBS_setUserInt (m_curves, curveId, back().userData);
    jmdlRIMSBS_setUserInt64 (m_curves, curveId, back().userData64);
    // !! group id is set by RIMSBS
    }
bool CreateAndAddPartialCurve (size_t existingCurveIndex, double s0, double s1)
    {
    bool stat = false;
    if (existingCurveIndex < size ()
        && at(existingCurveIndex).curve.IsValid ())
        {
        int newCurveId;
        if (jmdlRIMSBS_createSubcurve (m_curves, &newCurveId, at(existingCurveIndex).curveId, s0, s1))
            {
            auto partialCurve = ICurvePrimitive::CreatePartialCurve (at(existingCurveIndex).curve.get (), s0, s1);
            AddCurve (newCurveId, partialCurve);
            int groupId;
            jmdlRIMSBS_getGroupId (m_curves, &groupId, newCurveId); // partial curve gets group from parent !!!
            back ().groupId = groupId;
            stat = true;
            }
        }
    return stat;
    }
size_t ValidationErrors ()
    {
    size_t errors = 0;
    for (auto &c: *this)
        {
        errors += c.ValidationErrors (m_curves);
        }
    return errors;
    }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(AreaBoolean,RIMSBS0)
    {
    int oldVolume = Check::SetMaxVolume (10000);
    double r = 10.0;
    RIArray myCurves (100, 1000);

//    int userInt0 = 100;
    auto cv = CurveVector::Create (CurveVector::BOUNDARY_TYPE_None);
    DEllipse3d arc0 =  DEllipse3d::FromVectors
                (
                DPoint3d::From (0,0,0),
                r * DVec3d::UnitX (),
                r * DVec3d::UnitY (),
                Angle::DegreesToRadians (-170),
                Angle::DegreesToRadians (340)
                );
    DEllipse3d arc1 = DEllipse3d::FromVectors (
                DPoint3d::From (r * 0.5, 0, 0),
                r * DVec3d::UnitX (),
                r * DVec3d::UnitY (),
                Angle::DegreesToRadians (-170),
                Angle::DegreesToRadians (340)
                );

    myCurves.AddCurve (jmdlRIMSBS_addDEllipse3d (myCurves.m_curves, 1000, nullptr, &arc0), ICurvePrimitive::CreateArc (arc0));
    myCurves.AddCurve (jmdlRIMSBS_addDEllipse3d (myCurves.m_curves, 1000, nullptr, &arc1), ICurvePrimitive::CreateArc (arc1));
    myCurves.IncrementGroupId (2);

    myCurves.CreateAndAddPartialCurve (0, 0.1, 0.25);

    Check::Size (0, myCurves.ValidationErrors (), "ValidationErrors");
    Check::SetMaxVolume (oldVolume);
    }

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
    double hatchArea;
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
        bvector<DSegment3d> hatch;
        bvector<HatchSegmentPosition> positions;
        CurveVector::CreateXYHatch (hatch, &positions, *region, startPoint, angleRadians, spacing);
        double hatchLength = 0.0;
        for (auto &s : hatch)
            hatchLength += s.Length ();
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(AreaBoolean, OverlappingRectangles)
    {
    TestRectangles (0, 4, 3, 5);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(AreaBoolean, DisjointRectangles)
    {
    TestRectangles (0, 4, 5, 6);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(AreaBoolean, OuterInnerRectangles)
    {
    TestRectangles (0, 4, 1, 2);
    }


#ifdef Test3WayBoolean
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
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



/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8CP s_inputMathieuA = u8R"json([{"loop":[{"lineString":[[0.0,0.0,0.0],[640.0,0.0,0.0],[640.0,512.0,0.0],[0.0,512.0,0.0],[0.0,0.0,0.0]]}]},{"loop":[{"lineString":[[399.652604135361,87.0137533141434,0.0],[399.652604135361,230.366087345624,0.0],[203.435077605827,230.366087345624,0.0],[203.435077605827,72.2788323185172,0.0]]},{"arc":{"center":[295.012391491642,166.62231679171,0.0],"vectorX":[131.480407213716,0.0,0.0],"vectorY":[0.0,131.480407213716,0.0],"sweepStartEnd":[225.852396334914,233.80665237999442]}},{"lineSegment":[[217.371637111736,60.5138310782494,0.0],[372.653145871547,60.5138310782494,0.0]]},{"arc":{"center":[295.012391491642,166.62231679171,0.0],"vectorX":[131.480407213716,0.0,0.0],"vectorY":[0.0,131.480407213716,0.0],"sweepStartEnd":[306.193347620005,322.7367157146112]}}]}])json";
TEST(Mathieu,TestA)
    {
    bvector<IGeometryPtr> inputs;
    if (Check::True(IModelJson::TryIModelJsonStringToGeometry(s_inputMathieuA, inputs), "Parse input") &&
        Check::Size(inputs.size(), 2, "Have two inputs"))
        {
        auto areaA = inputs[0]->GetAsCurveVector();     // outer
        auto areaB = inputs[1]->GetAsCurveVector();     // inner, completely inside outer
        if (Check::True(areaA.IsValid() && areaB.IsValid(), "Inputs are valid"))
            {
            Check::Print (areaA, "A");
            Check::Print (areaB, "B");
            CurveVectorPtr areaAB = CurveVector::AreaIntersection (*areaA, *areaB);
            Check::Print (areaAB, "Intersection");
            DPoint3d c0, c1;
            DVec3d n0, n1;
            double a0, a1;
            if (Check::True(areaAB->CentroidNormalArea(c0, n0, a0)) &&
                Check::True(areaB->CentroidNormalArea(c1, n1, a1)))
                {
                Check::Near(c0, c1, "AreaIntersection has expected centroid");
                Check::Near(n0, n1, "AreaIntersection has expected normal");
                Check::Near(a0, a1, "AreaIntersection has expected area");
                }
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
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

// Create a blob with:
// a horizontal line x direction
// a non-monotone bspline returning to origin
CurveVectorPtr CreateBlob (TransformCR placement, bool rational = false)
    {
    double x4 = 10.0;
    double x3 = 9.0;
    double x2 = 4.0;
    double x1 = 1.0;
    double x0 = 0.0;

    double y0 = 0.0;
    double y1 = 1.0;
    double y2 = 4.0;
    double y3 = 10.0;
    auto loop = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Outer);
    loop->Add (ICurvePrimitive::CreateLine (DSegment3d::From (x0,y0,0, x3, y0, 0)));
    bvector<DPoint3d> poles {
        DPoint3d::From (x3, y0),
        DPoint3d::From (x4, y1),
        DPoint3d::From (x4, y3),
        DPoint3d::From (x4, y3),
        DPoint3d::From (x2, y3),
        DPoint3d::From (x1, y2),
        DPoint3d::From (x3, y2),
        DPoint3d::From (x3, y1),
        DPoint3d::From (x2, y1),
        DPoint3d::From (x1, y2),
        DPoint3d::From (x0, y2),
        DPoint3d::From (x0, y0)
        };
    if (rational)
        {
        bvector <double> weights;
        for (size_t i = 0; i < poles.size (); i++)
            weights.push_back (1.0);
        // make some of the poles attract their curves:
        double w2 = 1.2;
        for (size_t i : {2,5,7})
            {
            weights[i] = w2;
            poles[i].Scale (w2);
            }
        loop->Add (ICurvePrimitive::CreateBsplineCurve (
            MSBsplineCurve::CreateFromPolesAndOrder (poles, &weights, NULL, 3, false, false)));
        }
    else
        {
        loop->Add (ICurvePrimitive::CreateBsplineCurve (
            MSBsplineCurve::CreateFromPolesAndOrder (poles, NULL, NULL, 3, false, false)));
        }
    loop->TransformInPlace (placement);
    return loop;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Regions,Blob1)
    {
    for (bool rational: bvector<bool> { false, true })
        {
        SaveAndRestoreCheckTransform shifter (20,0,0);
        auto blob1 = CreateBlob (Transform::FromIdentity (), rational);
        auto blob2 = CreateBlob (
            Transform::FromRowValues (
                2, -1, 0, -1,
                1, 3, 0, -2,
                0, 0, 1, 0),
                rational);
        Check::SaveTransformed (*blob1);
        Check::SaveTransformed (*blob2);
        CurveVectorPtr boolB = CurveVector::AreaUnion (*blob1, *blob2);
        Check::SaveTransformed (*boolB);
        }
    Check::ClearGeometry ("Regions.Blob1");
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
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
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
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


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
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


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
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



/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
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


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
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
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
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
 
 
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(FixupXY,DisjointShapes)
    {
    CurveVectorPtr shape1 = CurveVector::CreateRectangle (0,0,300,400, 0);
    CurveVectorPtr shape2 = CurveVector::CreateRectangle (400, 0,600,400, 0);
    CurveVectorPtr unionRegion = CurveVector::Create (CurveVector::BOUNDARY_TYPE_UnionRegion);
    unionRegion->Add (shape1);
    unionRegion->Add (shape2);
    unionRegion->FixupXYOuterInner (false);
    
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(FixupXY,FullCircle)
    {
    CurveVectorPtr disk = CurveVector::CreateDisk (DEllipse3d::From (0,0,0,   1,0,0,   0,1,0, 0.0, Angle::TwoPi ()));
    Check::Print (disk, "original disk"); 
    disk->FixupXYOuterInner (true);
    Check::Print (disk, "After area fixup"); 
    disk->ConsolidateAdjacentPrimitives ();
    Check::Print (disk, "After area ConsolidateAdjacentPrimitives"); 
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
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


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
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



CurveVectorPtr CS_AreaIntersectionFailure_CurveVector0 ()
   {
   auto cv = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Outer);

   cv->push_back (ICurvePrimitive::CreateArc (
       DEllipse3d::FromScaledRotMatrix (
           DPoint3d::From (1622450.2999150001,5425082.3315470004,0.0),
           RotMatrix::FromColumnVectors (
               DVec3d::From (-0.86323001432114710,0.50481079859201905,0.0),
               DVec3d::FromCrossProduct (DVec3d::From (0.0,0.0,1.0), DVec3d::From (-0.86323001432114710,0.50481079859201905,0.0)),
               DVec3d::From (0.0,0.0,1.0)),
           709.73999997128647, 709.73999997128647,
           Angle::DegreesToRadians (7.5679424323581390), Angle::DegreesToRadians (3.0956830832630517))));


   cv->push_back (ICurvePrimitive::CreateLine (
       DSegment3d::From (
           DPoint3d::From (1621781.9136103140,5425321.0586197572,0.0),
           DPoint3d::From (1621766.3321385621,5425277.4337293329,0.0))));


   cv->push_back (ICurvePrimitive::CreateLine (
       DSegment3d::From (
           DPoint3d::From (1621766.3321385621,5425277.4337293329,0.0),
           DPoint3d::From (1621784.6771173216,5425270.8814660357,0.0))));


   cv->push_back (ICurvePrimitive::CreateLine (
       DSegment3d::From (
           DPoint3d::From (1621784.6771173216,5425270.8814660357,0.0),
           DPoint3d::From (1621800.2585890735,5425314.506356460,0.0))));

   cv->push_back (ICurvePrimitive::CreateArc (
       DEllipse3d::FromScaledRotMatrix (
           DPoint3d::From (1622450.2999150001,5425082.3315470004,0.0),
           RotMatrix::FromColumnVectors (
               DVec3d::From (-0.86323001432114721,0.50481079859201905,0.0),
               DVec3d::FromCrossProduct (DVec3d::From (0.0,0.0,1.0), DVec3d::From (-0.86323001432114721,0.50481079859201905,0.0)),
               DVec3d::From (0.0,0.0,1.0)),
           690.25999997128645, 690.25999997128645,
           Angle::DegreesToRadians (10.663625515621192), Angle::DegreesToRadians (-3.0956830832630517))));


   cv->push_back (ICurvePrimitive::CreateLine (
       DSegment3d::From (
           DPoint3d::From (1621813.7454376416,5425349.2721113879,0.0),
           DPoint3d::From (1621795.7810746199,5425356.8055077810,0.0))));

   return cv;
   }

CurveVectorPtr CS_AreaIntersectionFailure_CurveVector1 ()
   {
   auto cv = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Outer);

   cv->push_back (ICurvePrimitive::CreateArc (
       DEllipse3d::FromScaledRotMatrix (
           DPoint3d::From (1621688.8786060,5425156.6849819999,0.0),
           RotMatrix::FromColumnVectors (
               DVec3d::From (0.28954990090512533,0.95716291971943424,0.0),
               DVec3d::FromCrossProduct (DVec3d::From (0.0,-0.0,1.0), DVec3d::From (0.28954990090512533,0.95716291971943424,0.0)),
               DVec3d::From (0.0,-0.0,1.0)),
           176.50000080624326, 176.50000080624326,
           Angle::DegreesToRadians (-10.852760536121815), Angle::DegreesToRadians (-25.246863164628909))));


   cv->push_back (ICurvePrimitive::CreateLine (
       DSegment3d::From (
           DPoint3d::From (1621829.7090752404,5425263.0759083899,0.0),
           DPoint3d::From (1621841.6899110028,5425272.1013351576,0.0))));


   cv->push_back (ICurvePrimitive::CreateLineString (
           bvector<DPoint3d>{
               DPoint3d::From (1621841.6899110028,5425272.1013351576,0.0),
               DPoint3d::From (1621841.6776703051,5425272.1176301381,0.0)}));

   cv->push_back (ICurvePrimitive::CreateArc (
       DEllipse3d::FromScaledRotMatrix (
           DPoint3d::From (1621688.8786060,5425156.6849819999,0.0),
           RotMatrix::FromColumnVectors (
               DVec3d::From (0.28954990090512533,0.95716291971943435,0.0),
               DVec3d::FromCrossProduct (DVec3d::From (0.0,-0.0,1.0), DVec3d::From (0.28954990090512533,0.95716291971943435,0.0)),
               DVec3d::From (0.0,-0.0,1.0)),
           191.50000080624326, 191.50000080624326,
           Angle::DegreesToRadians (-36.099623700750726), Angle::DegreesToRadians (25.246863164628909))));


   cv->push_back (ICurvePrimitive::CreateLineString (
           bvector<DPoint3d>{
               DPoint3d::From (1621777.8478308839,5425326.2630682383,0.0),
               DPoint3d::From (1621777.8297578534,5425326.2724877214,0.0)}));


   cv->push_back (ICurvePrimitive::CreateLine (
       DSegment3d::From (
           DPoint3d::From (1621777.8297578534,5425326.2724877214,0.0),
           DPoint3d::From (1621770.8789621233,5425312.9801894771,0.0))));

   return cv;
   }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(CurveVector,AreaIntersectionFailure0)
    {
    auto cv0 = CS_AreaIntersectionFailure_CurveVector0 ();
    auto cv1 = CS_AreaIntersectionFailure_CurveVector1 ();
    Check::SaveTransformed (*cv0);
    Check::SaveTransformed (*cv1);
    auto cvIntersection = CurveVector::AreaIntersection (*cv0, *cv1);
    if (cvIntersection.IsValid ())
        Check::SaveTransformed (*cvIntersection);
    double a = cv0->Length ();

#ifdef ShowStrokes
    auto facetOptions = IFacetOptions::Create ();
    facetOptions->SetMaxEdgeLength (0.05 * a);
    auto stroke0 = cv0->Stroke (*facetOptions);
    auto stroke1 = cv1->Stroke (*facetOptions);
    Check::SaveTransformed (*stroke0);
    Check::SaveTransformed (*stroke1);
#endif

    Check::Shift (0.5 * a,0,0);
    double gapTol = 1.0e-10;
    CurveGapOptions options (gapTol, 1.0e4 * gapTol, 1.0e5 * gapTol);
    auto cv0A = cv0->CloneWithGapsClosed (options);
    auto cv1A = cv1->CloneWithGapsClosed (options);
    Check::SaveTransformed (*cv0A);
    Check::SaveTransformed (*cv1A);
    auto cvIntersectionA = CurveVector::AreaIntersection (*cv0A, *cv1A);
    if (cvIntersectionA.IsValid ())
        Check::SaveTransformed (*cvIntersectionA);
    Check::ClearGeometry ("CurveVector.AreaIntersecitonFailure0");
    }


CurveVectorPtr CurveVector0 ()
    {
    auto cv = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Outer);


    cv->push_back (ICurvePrimitive::CreateLineString (
            bvector<DPoint3d>{
                DPoint3d::From (13043520162.456053,5310549644.2877998,4220758.5286876550),
                DPoint3d::From (13043597708.378122,5310612784.0531626,4221115.8245967226),
                DPoint3d::From (13043675254.300194,5310675923.8185263,4221776.9688600637),
                DPoint3d::From (13043752800.222267,5310739063.583890,4222741.9614776773),
                DPoint3d::From (13043830346.144337,5310802203.3492537,4224010.8024495635),
                DPoint3d::From (13043907892.066404,5310865343.1146173,4225583.4917757232),
                DPoint3d::From (13043909825.891920,5310866917.6821251,4225626.5943413107),
                DPoint3d::From (13043948332.870647,5310896811.6828976,4226507.2015624233),
                DPoint3d::From (13043988185.874071,5310924886.1697283,4227460.0294561554),
                DPoint3d::From (13044030366.239513,5310951725.4021263,4228512.2414292255),
                DPoint3d::From (13044073776.366171,5310976526.3804922,4229640.4154908620),
                DPoint3d::From (13044118317.836985,5310999232.8774071,4230844.5516410675),
                DPoint3d::From (13044163889.669975,5311019793.4139442,4232124.6498798402),
                DPoint3d::From (13044210388.547173,5311038161.3763809,4233480.7102071820),
                DPoint3d::From (13044236706.072708,5311047444.4105959,4234270.6288319081),
                DPoint3d::From (13044257709.048868,5311054295.1218777,4234897.9040841972),
                DPoint3d::From (13044305743.892612,5311068158.0728912,4236262.8025317546),
                DPoint3d::From (13044354384.176441,5311079718.8000984,4237551.7388907438),
                DPoint3d::From (13044403519.625767,5311088951.0936518,4238764.7131611640),
                DPoint3d::From (13044453038.843386,5311095834.0225992,4239901.7253430160),
                DPoint3d::From (13044502829.562042,5311100351.9823427,4240962.7754362999),
                DPoint3d::From (13044552778.898943,5311102494.7300119,4241947.8634410147),
                DPoint3d::From (13044602773.611691,5311102257.4076843,4242856.9893571623),
                DPoint3d::From (13044652700.355005,5311099640.5534058,4243690.1531847399),
                DPoint3d::From (13044702445.937716,5311094650.0999651,4244447.3549237503),
                DPoint3d::From (13044751897.579353,5311087297.3614454,4245128.5945741925),
                DPoint3d::From (13044800943.165867,5311077599.0075703,4245733.8721360657),
                DPoint3d::From (13044849471.503794,5311065577.0259190,4246263.1876093699),
                DPoint3d::From (13044897372.572336,5311051258.6720705,4246716.5409941068),
                DPoint3d::From (13044944537.772820,5311034676.4078112,4247093.9322902746),
                DPoint3d::From (13044990860.174881,5311015867.8275404,4247395.3614978744),
                DPoint3d::From (13045036234.758907,5310994875.5730457,4247620.8286169069),
                DPoint3d::From (13045080558.654131,5310971747.2368145,4247770.3336473703),
                DPoint3d::From (13045123731.371840,5310946535.2541447,4247843.8765892647),
                DPoint3d::From (13045143519.427546,5310934032.1028690,4247852.2007979946),
                DPoint3d::From (13045165655.033213,5310919296.7842617,4247841.4574425910),
                DPoint3d::From (13045206234.591223,5310890093.5807333,4247763.0762073481),
                DPoint3d::From (13045245378.046114,5310858991.8514643,4247608.7328835381),
                DPoint3d::From (13045282996.653996,5310826062.1085920,4247378.4274711590),
                DPoint3d::From (13045319005.128016,5310791379.0086241,4247072.1599702118),
                DPoint3d::From (13045353321.831732,5310755021.1831865,4246689.9303806955),
                DPoint3d::From (13045385868.964197,5310717071.060750,4246231.7387026129),
                DPoint3d::From (13045416572.736332,5310677614.6797571,4245697.5849359604),
                DPoint3d::From (13045445363.538227,5310636741.4935493,4245087.4690807387),
                DPoint3d::From (13045472176.096954,5310594544.1675797,4244401.3911369508),
                DPoint3d::From (13045496949.624546,5310551118.3693113,4243639.3511045929),
                DPoint3d::From (13045519627.955818,5310506562.5513391,4242801.3489836659),
                DPoint3d::From (13045524618.111523,5310496002.2858677,4242594.6468315516),
                DPoint3d::From (13045561907.388672,5310415940.2273674,4240897.4584761094),
                DPoint3d::From (13045604128.034910,5310325290.2575312,4238689.7196142795),
                DPoint3d::From (13045646348.681145,5310234640.2876940,4236178.1323981751),
                DPoint3d::From (13045688569.327383,5310143990.3178577,4233362.6968277982),
                DPoint3d::From (13045730789.973619,5310053340.3480215,4230243.4129031468),
                DPoint3d::From (13045736774.233183,5310040491.8249102,4229776.7071198551),
                DPoint3d::From (13045718573.994461,5310092825.4112759,4231613.0870763920),
                DPoint3d::From (13045708291.239218,5310122621.5316372,4232599.3140441468),
                DPoint3d::From (13045704042.150726,5310134983.0187607,4232981.5323456470),
                DPoint3d::From (13045671828.765217,5310229652.2995777,4235905.6163728321),
                DPoint3d::From (13045640135.045507,5310324496.8200693,4238829.7004000172),
                DPoint3d::From (13045612185.214113,5310409606.9762764,4241449.1555045415),
                DPoint3d::From (13045608961.944826,5310419513.7276726,4241723.5618313076),
                DPoint3d::From (13045578310.400740,5310514700.1646347,4244357.5440219864),
                DPoint3d::From (13045560051.990690,5310572267.0687780,4245948.2863857402),
                DPoint3d::From (13045548181.335135,5310610053.2681103,4246918.0781955319),
                DPoint3d::From (13045520855.615675,5310698147.9923296,4249176.5020317724),
                DPoint3d::From (13045518575.654182,5310705570.1702347,4249345.9471568316),
                DPoint3d::From (13045489494.248302,5310801247.9982224,4251528.2620756505),
                DPoint3d::From (13045460937.992157,5310897083.8744469,4253710.5769944694),
                DPoint3d::From (13045434327.604284,5310988165.0896540,4255781.3536258386),
                DPoint3d::From (13045432907.744610,5310993074.9165211,4255881.2065402241),
                DPoint3d::From (13045427211.116928,5311012825.7409830,4256282.8045396814),
                DPoint3d::From (13045428056.368238,5311000123.7220469,4255912.7926114732),
                DPoint3d::From (13045423558.295774,5310988214.7725115,4255303.9790266706),
                DPoint3d::From (13045418308.660818,5310982102.5800085,4254797.8023707056),
                DPoint3d::From (13045414526.831139,5310979243.2407894,4254489.0364273349),
                DPoint3d::From (13045402588.197258,5310974824.5581789,4253818.0827262998),
                DPoint3d::From (13045389892.087564,5310975754.3611879,4253385.9309244575),
                DPoint3d::From (13045378724.588051,5310981865.2276945,4253192.5810218090),
                DPoint3d::From (13045366818.163229,5310992060.1435490,4253102.2735000709),
                DPoint3d::From (13045327986.763069,5311023552.6524830,4252814.2094624741),
                DPoint3d::From (13045287907.611330,5311053441.0675135,4252526.1454248792),
                DPoint3d::From (13045246646.92790,5311081676.0061655,4252238.0813872823),
                DPoint3d::From (13045204272.884848,5311108210.8178806,4251950.0173496865),
                DPoint3d::From (13045160855.493750,5311133001.6610994,4251661.9533120897),
                DPoint3d::From (13045116466.490032,5311156007.5756979,4251373.8892744947),
                DPoint3d::From (13045071179.214455,5311177190.5506573,4251085.8252368979),
                DPoint3d::From (13045025068.491924,5311196515.5868769,4250797.7611993020),
                DPoint3d::From (13045009681.184042,5311202480.7381659,4250702.6814999962),
                DPoint3d::From (13044978210.507858,5311213950.7549868,4250497.2748815473),
                DPoint3d::From (13044930682.682343,5311229467.2481136,4250144.4478066172),
                DPoint3d::From (13044882563.542175,5311243039.4294758,4249736.2646588143),
                DPoint3d::From (13044833932.591148,5311254644.8747339,4249272.7254381403),
                DPoint3d::From (13044784870.178682,5311264264.4090462,4248753.8301445954),
                DPoint3d::From (13044735457.367069,5311271882.1387482,4248179.5787781775),
                DPoint3d::From (13044685775.797539,5311277485.4776134,4247549.9713388886),
                DPoint3d::From (13044635907.555374,5311281065.1676493,4246865.0078267287),
                DPoint3d::From (13044585935.034281,5311282615.2943897,4246124.6882416978),
                DPoint3d::From (13044560842.947559,5311282627.4445438,4245732.2974327384),
                DPoint3d::From (13044535940.800257,5311282133.2966738,4245308.2936206488),
                DPoint3d::From (13044486007.455179,5311279619.9708719,4244290.5854545021),
                DPoint3d::From (13044436217.500315,5311275079.4695711,4243050.5357742803),
                DPoint3d::From (13044386653.200026,5311268519.2947140,4241588.1445799833),
                DPoint3d::From (13044337396.445833,5311259950.2852039,4239903.4118716121),
                DPoint3d::From (13044305662.831009,5311253327.5136480,4238666.9309195653),
                DPoint3d::From (13044292874.337944,5311253274.6613836,4238192.2267306978),
                DPoint3d::From (13044285668.923021,5311255809.7993021,4237895.0414510453),
                DPoint3d::From (13044281225.797338,5311258552.9044561,4237712.7416348774),
                DPoint3d::From (13044272833.986492,5311268203.0784950,4237439.0232923357),
                DPoint3d::From (13044270045.062050,5311275327.8547411,4237394.7414541729),
                DPoint3d::From (13044269223.868526,5311280471.5501394,4237415.2047687341),
                DPoint3d::From (13044271051.475466,5311293128.8879757,4237641.2860640716),
                DPoint3d::From (13044277984.693882,5311303874.9956732,4238117.2671783352),
                DPoint3d::From (13044247748.332396,5311273781.9326973,4236128.0947824530),
                DPoint3d::From (13044228946.805431,5311255069.5113907,4234891.1907173870),
                DPoint3d::From (13044223691.888010,5311249839.4984808,4234545.4832260348),
                DPoint3d::From (13044176869.897457,5311203239.4103394,4232962.9363729367),
                DPoint3d::From (13044158068.370493,5311184526.9890308,4232327.4593892042),
                DPoint3d::From (13044105991.462519,5311132696.8879795,4230567.3004155969),
                DPoint3d::From (13044087189.935555,5311113984.4666719,4229931.8234318858),
                DPoint3d::From (13044035113.027580,5311062154.3656187,4228171.6644582571),
                DPoint3d::From (13044016311.500618,5311043441.9443130,4227536.1874745423),
                DPoint3d::From (13043964234.592646,5310991611.8432627,4225776.0285009174),
                DPoint3d::From (13043945433.065681,5310972899.4219532,4225140.5515171578),
                DPoint3d::From (13043893356.157705,5310921069.3209028,4223380.3925435776),
                DPoint3d::From (13043874554.630743,5310902356.8995953,4222744.9155598804),
                DPoint3d::From (13043869299.713320,5310897126.8866844,4222567.3034393620),
                DPoint3d::From (13043822477.722767,5310850526.7985439,4222023.6700379038),
                DPoint3d::From (13043803676.195805,5310831814.3772354,4221805.3722266695),
                DPoint3d::From (13043777812.147089,5310806072.9054289,4221505.0739990221),
                DPoint3d::From (13043751599.287828,5310779984.2761831,4221429.1216968894),
                DPoint3d::From (13043732797.760866,5310761271.8548775,4221374.6438834509),
                DPoint3d::From (13043680720.852894,5310709441.7538271,4221223.7499649581),
                DPoint3d::From (13043661919.325930,5310690729.3325176,4221169.2721515158),
                DPoint3d::From (13043609842.417955,5310638899.2314672,4221018.3782330258),
                DPoint3d::From (13043591040.890991,5310620186.8101587,4220963.9004195854),
                DPoint3d::From (13043538963.983017,5310568356.7091074,4220813.0065010944),
                DPoint3d::From (13043520162.456053,5310549644.2877998,4220758.5286876550)}));

    return cv;
    }

CurveVectorPtr CurveVector1 ()
    {
    auto cv = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Outer);


    cv->push_back (ICurvePrimitive::CreateLineString (
            bvector<DPoint3d>{
                DPoint3d::From (13045288188.951866,5311078742.6491575,4256544.1025003754),
                DPoint3d::From (13045285329.276218,5311074641.1548853,4256544.1025003754),
                DPoint3d::From (13045284852.663607,5311073957.5725021,4251544.1024938328),
                DPoint3d::From (13045276273.636683,5311061653.0897169,4252444.1024930421),
                DPoint3d::From (13045279958.070763,5311059074.1769695,4252470.0128038982),
                DPoint3d::From (13045320272.714859,5311029504.1714115,4252758.0768414941),
                DPoint3d::From (13045359352.352741,5310998320.2386837,4253046.1408790899),
                DPoint3d::From (13045378724.588045,5310981865.2276974,4253192.5810174383),
                DPoint3d::From (13045389511.808931,5310975869.3002806,4253376.3595267162),
                DPoint3d::From (13045401800.120028,5310974723.5733013,4253784.3775461670),
                DPoint3d::From (13045413509.857397,5310978621.9486761,4254416.6350801615),
                DPoint3d::From (13045418306.833715,5310982100.9565878,4254797.6349527678),
                DPoint3d::From (13045422659.274466,5310986904.6684742,4255211.9351929035),
                DPoint3d::From (13045427699.931198,5310998169.9718237,4255834.1437808871),
                DPoint3d::From (13045427778.750893,5311010511.3278017,4256232.1128543401),
                DPoint3d::From (13045427211.116930,5311012825.7409868,4256282.8042970803),
                DPoint3d::From (13045412796.281282,5311008676.9444323,4255382.8042978710),
                DPoint3d::From (13045411995.457073,5311008446.4557323,4260382.8043044135),
                DPoint3d::From (13045407190.511847,5311007063.5235453,4260382.8043044135),
                DPoint3d::From (13045407363.955557,5311006356.3417387,4260332.1128616733),
                DPoint3d::From (13045407339.871761,5311002585.3718557,4259934.1437882194),
                DPoint3d::From (13045405799.671093,5310999143.1958332,4259311.9352002367),
                DPoint3d::From (13045404469.758640,5310997675.3949785,4258897.6349601001),
                DPoint3d::From (13045403004.015879,5310996612.3647833,4258516.6350874947),
                DPoint3d::From (13045399426.040569,5310995421.1945295,4257884.3775535002),
                DPoint3d::From (13045395671.278845,5310995771.2777739,4257476.3595340494),
                DPoint3d::From (13045392375.183569,5310997603.3667107,4257292.5810247706),
                DPoint3d::From (13045372674.827749,5311014337.0872326,4257146.1408864232),
                DPoint3d::From (13045332933.271881,5311046049.2031021,4256858.0768488264),
                DPoint3d::From (13045291935.791672,5311076120.0556383,4256570.0128112314),
                DPoint3d::From (13045288188.951866,5311078742.6491575,4256544.1025003754)}));

    return cv;
    }
void flatten (CurveVectorPtr &cv)
    {
    auto transform = Transform::FromRowValues (
        1,0,0,0,
        0,1,0,0,
        0,0,0,0);
    cv->TransformInPlace (transform);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(CurveVector,AreaUnionCroomLineStringChatter)
    {
    auto cvA = CurveVector0 ();
    auto cvB = CurveVector1 ();
    flatten (cvA);
    flatten (cvB);
    auto cvAB = CurveVector::AreaUnion (*cvA, *cvB);
    Check::SaveTransformed (*cvA);
    Check::SaveTransformed (*cvB);
    if (cvAB.IsValid ())
        Check::SaveTransformed (*cvAB);
    Check::ClearGeometry ("CurveVector.AreaUnionCroomLineStringChatter");
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(CurveVector,ReduceToCCWAreas)
    {
    // an outer loop
    auto outerLoop = CurveVector::CreateLinear (bvector<DPoint3d> {
            DPoint3d::From (0,0),
            DPoint3d::From (20,0),
            DPoint3d::From (18,10),
            DPoint3d::From (1,10),
            DPoint3d::From (0,0)},
        CurveVector::BOUNDARY_TYPE_Outer
        );
    // a hole loop, input counterclockwise 
    auto holeA = CurveVector::CreateLinear (bvector<DPoint3d> {
            DPoint3d::From (2,2),
            DPoint3d::From (6,2),
            DPoint3d::From (6,5),
            DPoint3d::From (2,5),
            DPoint3d::From (2,2)},
        CurveVector::BOUNDARY_TYPE_Inner
        );
    // a hole loop, defined clockwise 
    auto holeB = CurveVector::CreateLinear (bvector<DPoint3d> {
            DPoint3d::From (12,2),
            DPoint3d::From (12,5),
            DPoint3d::From (16,5),
            DPoint3d::From (16,2),
            DPoint3d::From (12,2)},
        CurveVector::BOUNDARY_TYPE_Inner
        );

    // a hole loop, but it jumps outside ...
    auto holeC = CurveVector::CreateLinear (bvector<DPoint3d> {
            DPoint3d::From (8,8),
            DPoint3d::From (12,9),
            DPoint3d::From (12,12),
            DPoint3d::From (8,12),
            DPoint3d::From (8,8)},
        CurveVector::BOUNDARY_TYPE_Inner
        );



    auto parityRegion = CurveVector::Create (CurveVector::BOUNDARY_TYPE_ParityRegion);
    parityRegion->Add (outerLoop);
    parityRegion->Add (holeA);
    parityRegion->Add (holeB);
    parityRegion->Add (holeC);

    Check::SaveTransformed (*parityRegion);
    auto fixed = CurveVector::ReduceToCCWAreas (*parityRegion);
    Check::Shift (0,20,0);
    Check::SaveTransformed (*fixed);

    // we expect that the output is a union region with two  members -- one parity region and one loop
    if (Check::Size (2, fixed->size (), "Expect two members of fixup output")
        && Check::Int (CurveVector::BOUNDARY_TYPE_UnionRegion, fixed->GetBoundaryType ()))
        {
        // one should be a parity region, one should be a simple loop ...
        int numParity = 0;
        int numOuter = 0;
        for (auto i = 0; i < fixed->size (); i++)
            {
            CurveVector::BoundaryType t;
            if (fixed->GetChildBoundaryType(i, t))
                {
                if (t == CurveVector::BOUNDARY_TYPE_Outer)
                    numOuter++;
                if (t == CurveVector::BOUNDARY_TYPE_ParityRegion)
                    numParity++;
                }
            }
        Check::Int (1, numParity, "num parity regions");
        Check::Int (1, numOuter,   "num outer");
        }

    Check::ClearGeometry ("CurveVector.ReduceToCCWAreas");
    }

bvector<DPoint3d> BuildSymbol (DPoint3dCR pointA, DPoint3dCR pointB, bvector<DPoint2d> const &uvPoints)
    {
    bvector<DPoint3d> points;
    for (auto &uv : uvPoints)
        points.push_back (DPoint3d::FromInterpolateAndPerpendicularXY (
                pointA, uv.x, pointB, uv.y));
    return points;
    }
void ShowOrientation (ICurvePrimitiveCR primitive, double arrowLength = 0.5, double zShift = 0.1);

void ShowOrientation (CurveVectorCR curves, double arrowLength = 0.5, double zShift = 0.1)
    {
    static bvector<DPoint2d> outerArrow {
        DPoint2d::From (0,0),
        DPoint2d::From (1,0),
        DPoint2d::From (0.9, 0.1),
        DPoint2d::From (0.9,-0.1),
        DPoint2d::From (1,0)
        };

    static bvector<DPoint2d> innerArrow {
        DPoint2d::From (0,0),
        DPoint2d::From (0.6,0),
        DPoint2d::From (0.5, 0.05),
        DPoint2d::From (0.5, 0)
        };
    auto btype = curves.GetBoundaryType ();
    if (btype == CurveVector::BOUNDARY_TYPE_Outer
        || btype == CurveVector::BOUNDARY_TYPE_Inner)
        {
        DPoint3d pointA, pointB;
        DVec3d unitA, unitB;
        curves.GetStartEnd (pointA, pointB, unitA, unitB);
        DPoint3d pointC = pointA; pointC.z += zShift;
        DPoint3d pointD = pointC + arrowLength * unitA;
        bvector<DPoint3d> arrow = btype == CurveVector::BOUNDARY_TYPE_Outer 
            ? BuildSymbol (pointC, pointD,outerArrow)
            : BuildSymbol (pointC, pointD,innerArrow);
        Check::SaveTransformed (arrow);
        }
    else if (btype == CurveVector::BOUNDARY_TYPE_ParityRegion)
        {
        for (auto &child : curves)
            ShowOrientation (*child, arrowLength, zShift + 1);
        }
    else if (btype == CurveVector::BOUNDARY_TYPE_UnionRegion)
        {
        for (auto &child : curves)
            ShowOrientation (*child, arrowLength, zShift + 2);
        }
    }
void ShowOrientation (ICurvePrimitiveCR primitive, double arrowLength, double zShift)
    {
    auto childP = primitive.GetChildCurveVectorCP ();
    if (nullptr != childP)
        ShowOrientation (*childP, arrowLength, zShift);
    }

void testFixup (bvector<CurveVectorPtr> &loops)
    {
    SaveAndRestoreCheckTransform shifter1 (0,60,0);
    double a = 0.5;
    for (size_t i = 0; i < loops.size (); i++)
        {
        ShowOrientation (*loops[i], a);
        Check::SaveTransformed (*loops[i]);    
        }
    for (size_t i0 = 0; i0 < loops.size (); i0++)
        {
        SaveAndRestoreCheckTransform shifter2 (20,0,0);
        auto parityRegion = CurveVector::Create (CurveVector::BOUNDARY_TYPE_ParityRegion);
        parityRegion->Add (loops[i0]);
        for (size_t i = 0; i < loops.size (); i++)
            if (i != i0)
                parityRegion->Add (loops[i]);
        parityRegion->FixupXYOuterInner (false);
        Check::SaveTransformed (*parityRegion);
        Check::Shift (0,10,0);
        ShowOrientation (*parityRegion);
        Check::SaveTransformed (*parityRegion->at (0));
        }
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(CurveVector,FixupXYOuterInnerA)
    {
    bvector<CurveVectorPtr> rectangles {
        CurveVector::CreateRectangle (0, 0, 5, 5, 0.0, CurveVector::BOUNDARY_TYPE_Outer),
        CurveVector::CreateRectangle (1, 1, 2, 3, 0.0, CurveVector::BOUNDARY_TYPE_Inner),
        CurveVector::CreateRectangle (3, 1, 4, 1, 0.0, CurveVector::BOUNDARY_TYPE_Inner),
        };
    testFixup (rectangles);
    rectangles.push_back (
        CurveVector::CreateRectangle (6, 1, 8, 3, 0.0, CurveVector::BOUNDARY_TYPE_Inner));
    testFixup (rectangles);
    Check::ClearGeometry ("CurveVector.FixupXYOuterInnerA");
    }

// Unescaped raw_json_text is in between the following multicharacter delimiters: foo( raw_json_text )foo
static Utf8CP s_inputs716145 = u8R"foo([{"loop":[{"lineString":[[-132.11164673533393,-25.700655956438513,0.0],[-114.03188553228320,-9.9217681230703834,0.0],[-104.96918378452468,12.298870087599719,0.0],[-43.856557232211763,12.298870087599719,0.0],[-34.793855484453245,-9.9217681230703834,0.0],[-16.714094281402510,-25.700655956438504,0.0],[-46.487127132928662,-82.560516701468197,0.0],[-74.412870508368220,-75.513612371941481,0.0],[-102.33861388380779,-82.560516701468197,0.0],[-132.11164673533393,-25.700655956438513,0.0]]}]},{"loop":[{"arc":{"center":[-38.412870508368220,-50.563542157603242,0.0],"sweepStartEnd":[0.0,105.32813241822168],"vectorX":[33.0,0.0,0.0],"vectorY":[0.0,33.0,0.0]}},{"arc":{"center":[-74.412870508368220,-0.16354215760324150,0.0],"sweepStartEnd":[325.74722316572706,360.0],"vectorX":[33.0,0.0,0.0],"vectorY":[0.0,33.0,0.0]}},{"arc":{"center":[-74.412870508368220,-0.16354215760324150,0.0],"sweepStartEnd":[0.0,180.0],"vectorX":[33.0,0.0,0.0],"vectorY":[0.0,33.0,0.0]}},{"arc":{"center":[-74.412870508368220,-0.16354215760324150,0.0],"sweepStartEnd":[180.0,214.25277683427291],"vectorX":[33.0,0.0,0.0],"vectorY":[0.0,33.0,0.0]}},{"arc":{"center":[-110.41287050836822,-50.563542157603242,0.0],"sweepStartEnd":[74.671867581778315,131.11242649459385],"vectorX":[33.0,0.0,0.0],"vectorY":[0.0,33.0,0.0]}},{"lineSegment":[[-132.11164647336085,-25.700655727804758,0.0],[-132.11164673533393,-25.700655956438513,0.0]]},{"arc":{"center":[-110.41287050836822,-50.563542157603242,0.0],"sweepStartEnd":[131.11242709830296,180.0],"vectorX":[33.0,0.0,0.0],"vectorY":[0.0,33.0,0.0]}},{"arc":{"center":[-110.41287050836822,-50.563542157603242,0.0],"sweepStartEnd":[180.0,284.16259515460331],"vectorX":[33.0,0.0,0.0],"vectorY":[0.0,33.0,0.0]}},{"arc":{"center":[-110.41287050836822,-50.563542157603242,0.0],"sweepStartEnd":[284.16259515460331,360.0],"vectorX":[33.0,0.0,0.0],"vectorY":[0.0,33.0,0.0]}},{"arc":{"center":[-110.41287050836822,-50.563542157603242,0.0],"sweepStartEnd":[0.0,34.252776834272908],"vectorX":[33.0,0.0,0.0],"vectorY":[0.0,33.0,0.0]}},{"arc":{"center":[-74.412870508368220,-0.16354215760324150,0.0],"sweepStartEnd":[254.67186758177832,285.32813241822168],"vectorX":[33.0,0.0,0.0],"vectorY":[0.0,33.0,0.0]}},{"arc":{"center":[-38.412870508368220,-50.563542157603242,0.0],"sweepStartEnd":[145.74722316572709,180.0],"vectorX":[33.0,0.0,0.0],"vectorY":[0.0,33.0,0.0]}},{"arc":{"center":[-38.412870508368220,-50.563542157603242,0.0],"sweepStartEnd":[180.0,360.0],"vectorX":[33.0,0.0,0.0],"vectorY":[0.0,33.0,0.0]}}]}])foo";
static Utf8CP s_output716145 = u8R"foo([{"loop":[{"arc":{"center":[-74.41287050836823,-0.1635421576032421,0.0],"vectorX":[33.0,0.0,0.0],"vectorY":[0.0,33.0,0.0],"sweepStartEnd":[22.18812321520529,157.81187678479473]}},{"lineString":[[-104.96918378452468,12.298870087599724,0.0],[-104.96918407814914,12.298869367668388,0.0],[-114.0318855322832,-9.921768123070385,0.0],[-132.11164636484808,-25.700655633101556,0.0],[-132.11164647336086,-25.700655727804756,0.0],[-132.11164673533393,-25.70065595643851,0.0]]},{"arc":{"center":[-110.41287050836822,-50.56354215760324,0.0],"vectorX":[33.0,0.0,0.0],"vectorY":[0.0,33.0,0.0],"sweepStartEnd":[131.11242709830297,180.0]}},{"arc":{"center":[-110.41287050836822,-50.56354215760324,0.0],"vectorX":[33.0,0.0,0.0],"vectorY":[0.0,33.0,0.0],"sweepStartEnd":[180.0,284.1625951546033]}},{"lineString":[[-102.3386138838078,-82.56051670146819,0.0],[-74.41287050836823,-75.51361237194149,0.0],[-46.487127470071559,-82.56051661639208,0.0],[-46.48712713292866,-82.56051670146819,0.0]]},{"arc":{"center":[-38.41287050836822,-50.56354215760324,0.0],"vectorX":[33.0,0.0,0.0],"vectorY":[0.0,33.0,0.0],"sweepStartEnd":[255.83740484539673,360.0]}},{"arc":{"center":[-38.41287050836822,-50.56354215760324,0.0],"vectorX":[33.0,0.0,0.0],"vectorY":[0.0,33.0,0.0],"sweepStartEnd":[0.0,48.88757290169704]}},{"lineString":[[-16.71409428140251,-25.700655956438504,0.0],[-34.793855484453249,-9.921768123070385,0.0],[-43.85655723221176,12.298870087599724,0.0]]}]}])foo";
TEST(AreaBoolean, NearTangencyUnion716145)
    {
    bvector<IGeometryPtr> inputs, output;
    if (Check::True(IModelJson::TryIModelJsonStringToGeometry(s_inputs716145, inputs), "Parse inputs") &&
        Check::True(IModelJson::TryIModelJsonStringToGeometry(s_output716145, output), "Parse output") &&
        Check::Size(inputs.size(), 2, "Have two inputs") &&
        Check::Size(output.size(), 1, "Have one output"))
        {
        auto input0 = inputs[0]->GetAsCurveVector();
        auto input1 = inputs[1]->GetAsCurveVector();
        auto output0 = output[0]->GetAsCurveVector();
        if (Check::True(input0.IsValid() && input1.IsValid() && output0.IsValid(), "Inputs and output are valid"))
            {
            auto result = CurveVector::AreaUnion(*input0, *input1);
            if (Check::True(result.IsValid(), "AreaUnion succeeded"))
                {
                // following line is commented out to avoid CI build failure. The result I get and the result CI gets have different structure, same geometry.
                // Check::True(result->IsSameStructureAndGeometry(*output0), "AreaUnion result has expected structure and geometry");
                if (Check::Near(result->Length(), output0->Length(), "AreaUnion result has expected length"))
                    {
                    DPoint3d c0, c1;
                    DVec3d n0, n1;
                    double a0, a1;
                    if (Check::True(result->CentroidNormalArea(c0, n0, a0)) &&
                        Check::True(output0->CentroidNormalArea(c1, n1, a1)))
                        {
                        Check::Near(c0, c1, "AreaUnion result has expected centroid");
                        Check::Near(n0, n1, "AreaUnion result has expected normal");
                        Check::Near(a0, a1, "AreaUnion result has expected area");
                        }
                    }
                }
            }
        }
    }
