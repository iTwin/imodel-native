//
//
#include "testHarness.h"
#include <GeomSerialization/GeomSerializationApi.h>

USING_NAMESPACE_BENTLEY_GEOMETRY_INTERNAL
static int s_noisy = 0;

void ExercisePointsAlong (CurveVectorCR cv, size_t numPoints)
    {
    double length = cv.Length ();
    bvector<double> distances;
    double delta = length / numPoints;
    for (size_t i = 0; i <= numPoints; i++)
        distances.push_back (i * delta);
    bvector<CurveLocationDetail> points;
    cv.AddSpacedPoints (distances, points);
    if (Check::Size (numPoints + 1, points.size (), "PointsAlong count"))
        {
        // confirm "a" coordinates ..
        for (size_t i = 0; i < points.size (); i++)
            Check::Near (distances[i], points[i].a, "Distance along in computed location");
        // When consecutives are on same curve, confirm fraction advance and distance between 
        for (size_t i = 0; i + 1 < points.size (); i++)
            {
            CurveLocationDetail &detail0 = points.at(i);
            CurveLocationDetail &detail1 = points.at(i+1);
            if (detail0.curve == detail1.curve)
                {
                double distanceBetween;
                if (Check::True (detail0.curve->SignedDistanceBetweenFractions (detail0.fraction, detail1.fraction, distanceBetween)))   
                    Check::Near (distanceBetween, detail1.a - detail0.a, "Actual distance between");
                }
            }
        }
    }

void CheckIsSameStructure (CurveVectorCR curveA)
    {
    Transform transform = Transform::From (DPoint3d::From (1,2,3));
    CurveVectorPtr curveB = curveA.Clone ();
    CurveVectorPtr curveC =curveA.Clone ();
    curveC->TransformInPlace(transform);
    if (   Check::True (curveA.IsSameStructure (curveA), "A.IsSameStructure(A)")
        && Check::True (curveA.IsSameStructure (*curveB), "A.IsSameStructure(A)")
        && Check::True (curveA.IsSameStructure (*curveC), "A.IsSameStructure(A)")
        && Check::True (curveA.IsSameStructureAndGeometry (curveA), "A.IsSameStructureAndGeometry(A)")
        && Check::True (curveA.IsSameStructureAndGeometry (*curveB), "A.IsSameStructureAndGeometry(B)")
        )
        {
        Check::False (curveA.IsSameStructureAndGeometry (*curveC), "A.IsSameStructureAndGeometry (C)");
        }
    }
void CheckWireMoments (CurveVectorCR curve)
    {
    CheckIsSameStructure (curve);
    Check::StartScope ("Compare old,new moments");
    DMatrix4d products;
    DVec3d centroid0, centroid1, moment1;
    double length0, length1;
    RotMatrix axes1;
    curve.WireCentroid (length0, centroid0);
    curve.ComputeSecondMomentWireProducts (products);
    products.ConvertInertiaProductsToPrincipalMoments (length1, centroid1, axes1, moment1);
    Check::Near (length0, length1);
    Check::Near (centroid0, centroid1);
    Check::EndScope ();

    ExercisePointsAlong (curve, 12);
    ExercisePointsAlong (curve, 28);

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(CurveVectorA, CloneLocal)
    {
    DEllipse3d ellipseData = DEllipse3d::From (1,2,3,
                        0,0,2,
                        0,3,0,
                        0.0, Angle::TwoPi ());
    ICurvePrimitivePtr ellipse = ICurvePrimitive::CreateArc (ellipseData);
    CurveVectorPtr pathA = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
    pathA->push_back (ellipse);

    Transform localToWorldB, worldToLocalB;
    DRange3d rangeB, rangeB1;
    CurveVectorPtr pathB = pathA->CloneInLocalCoordinates (LOCAL_COORDINATE_SCALE_UnitAxesAtStart, localToWorldB, worldToLocalB, rangeB);
    pathB->GetRange (rangeB1);
    Check::Near (rangeB, rangeB1, "Local Range (0,false)");

    Transform localToWorldC, worldToLocalC;
    DRange3d rangeC, rangeC1;
    CurveVectorPtr pathC = pathA->CloneInLocalCoordinates (LOCAL_COORDINATE_SCALE_01RangeLargerAxis, localToWorldC, worldToLocalC, rangeC);
    pathC->GetRange (rangeC1);

    Check::Near (rangeC, rangeC1, "Local Range (1,true)");
    Check::Near (rangeC.low.x, 0.0, "low x");
    Check::Near (rangeC.low.y, 0.0, "low y");
    Check::Near (rangeC.low.z, 0.0, "low z");
    Check::Near (rangeC.high.z, 0.0, "high z");

    }

// Confirma that x,y,0 maps to a local 01
void CheckMappedCorner
(
double x,
double y,
TransformCR localToWorld,
TransformCR worldToLocal
)
    {
    DPoint3d xyz = DPoint3d::From (x,y,0.0);
    DPoint3d uvw, xyz1;
    worldToLocal.Multiply (uvw, xyz);
    double a = 0.5 + 1.0e-12;
    Check::True (a >= fabs (uvw.x - 0.5), "x corner map");
    Check::True (a >= fabs (uvw.y - 0.5), "y corner map");
    localToWorld.Multiply (xyz1, uvw);
    Check::Near (xyz, xyz1);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(CurveVectorA, CloneLocalL)
    {
    double a0 = 0.1;
    double b0 = 0.25;
    double a1 = 3.0;
    double b1 = 4.0;
    bvector<DPoint3d> xyz;
    // First edge is to right.
    // Natural frame is unrotated and origin at (a,b)
    xyz.push_back (DPoint3d::From (1,1,0));
    xyz.push_back (DPoint3d::From (2,1,0));
    xyz.push_back (DPoint3d::From (2,b0,0));
    xyz.push_back (DPoint3d::From (a1,b0,0));
    xyz.push_back (DPoint3d::From (a1,b1,0));
    xyz.push_back (DPoint3d::From (a0,b1,0));
    xyz.push_back (DPoint3d::From (a0,2,0));
    xyz.push_back (DPoint3d::From (1,1,0));

    CurveVectorPtr region = CurveVector::CreateLinear (xyz, CurveVector::BOUNDARY_TYPE_Outer, false);
    Transform localToWorld, worldToLocal;
    DRange3d localRange;
    CurveVectorPtr local = region->CloneInLocalCoordinates (LOCAL_COORDINATE_SCALE_01RangeLargerAxis, localToWorld, worldToLocal, localRange);
    DRange3d cloneRange;
    local->GetRange (cloneRange);
    Check::Near (cloneRange, localRange, "clone ranges");
    Check::Near (localRange.low.x, 0.0, "low x");
    Check::Near (localRange.low.y, 0.0, "low y");
    Check::Near (localRange.low.x, 0.0, " high x");
    Check::Near (localRange.low.y, 0.0, "high y");

    CheckMappedCorner (a0, b0, localToWorld, worldToLocal);
    CheckMappedCorner (a1, b0, localToWorld, worldToLocal);
    CheckMappedCorner (a1, b1, localToWorld, worldToLocal);
    CheckMappedCorner (a0, b1, localToWorld, worldToLocal);
    
    }

double MaxMidPointError (DEllipse3dCR ellipse, bvector<DPoint3d> &strokePoints)
    {
    double dMax = 0.0;
    for (size_t i = 1; i < strokePoints.size (); i++)
        {
        double d;
        DPoint3d midPoint = DPoint3d::FromInterpolate (strokePoints[i-1], 0.5, strokePoints[i]);
        DPoint3d nearPoint;
        double theta, d2;
        ellipse.ClosestPointXYBounded (theta, d2, nearPoint, midPoint);
        d = sqrt (d2);
        if (d > dMax)
            dMax = d;
        }
    return dMax;
    }

void TestEllipseStroke (double r0, double r90, double chordTol)
    {
    DEllipse3d ellipse = DEllipse3d::From (0,0,0, r0,0,0, 0,r90,0, 0, Angle::TwoPi ());

    CurveVectorPtr disk = CurveVector::CreateDisk (ellipse);
    IFacetOptionsPtr options = IFacetOptions::CreateForCurves ();
    bvector<DPoint3d> strokePoints;
    if (chordTol > 0.0)
        {
        options->SetAngleTolerance (0.0);
        options->SetChordTolerance (chordTol);
        disk->AddStrokePoints (strokePoints, *options);
        double dMax = MaxMidPointError (ellipse, strokePoints);
        Check::True(dMax < chordTol);
        }
    }

class MomentIntegrandWithExtraXPower : BSIVectorIntegrandXY 
{ 
public: 
double ax, ay, ux, uy, vx, vy; 
virtual int GetVectorIntegrandCount() {return 10;} 
virtual void EvaluateVectorIntegrand (double s, double t, double *pF) 
{ 
double x = ax + ux * s + vx * t; 
double y = ay + uy * s + vy * t; 
int k = 0; 
for (int i = 0; i <= 3; i++) 
{ 
for (int j = 0; i + j <= 3; j++) 
{ 
pF[k++] = pow (x, j) * pow(y,i); 
} 
} 
} 
}; 



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (CurveVectorA, StrokeEllipse)
    {
    MomentIntegrandWithExtraXPower F;
    F.ax = 0.0;
    TestEllipseStroke (10,10, 0.01);
    }

// assume input curve vector really is planar (EXCEPT:: If it just a line !!!!)
// Make a line from point1 to point2
// sping around it by angle
// verify that the transforms go along.
void CheckPlanarity (CurveVectorR curves, DPoint3dCR point0, DPoint3dCR point1, double angle)
    {
    DRange3d range2, range1;
    
    bool isLine = curves.size () == 1 && curves[0]->GetCurvePrimitiveType () == ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line;
    
    Transform spinner = Transform::FromLineAndRotationAngle (point0, point1, angle);
    CurveVectorPtr curves2 = curves.Clone ();
    curves2->TransformInPlace (spinner);
    Transform localToWorld2, worldToLocal2;
    Transform localToWorld1, worldToLocal1;
    if (isLine)
        {
        if (Check::True(curves.IsPlanar (localToWorld1, worldToLocal1, range1), "IsPlanar expect false"))
            {
            Check::True (range1.IsAlmostZeroY (), "local y range of line almost zero");
            }
        
        }
    else if (Check::True (curves.IsPlanar (localToWorld1, worldToLocal1, range1), "IsPlanar"))
        {
        Check::True (curves2->IsPlanar (localToWorld2, worldToLocal2, range2), "spun curve planar");
        if (true)
            {
            DVec3d localZ1, localY1, localX1, localX3, localY3, localZ3, localZ2;
            DPoint3d origin1;
            localToWorld1.GetOriginAndVectors   (origin1, localX1, localY1, localZ1);
            spinner.MultiplyMatrixOnly (localX3, localX1);
            spinner.MultiplyMatrixOnly (localY3, localY1);
            localZ3.CrossProduct (localX3, localY3);    // hmph.  spinner should be orthogonal, but don't trust its z
            localToWorld2.GetMatrixColumn   (localZ2, 2);
            Check::Parallel (localZ3, localZ2, "spun curve z direction");
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(CurveVectorA, PointIsPlanar)
    {
    DPoint3d xyz = DPoint3d::From (1,2,4);
    CurveVectorPtr curves = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Outer);
    curves->push_back (ICurvePrimitive::CreateLine (DSegment3d::From (xyz, xyz)));
    CheckPlanarity (*curves, DPoint3d::From (0,0,0), DPoint3d::From (5,2,-3), 0.2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (CurveVectorA, LocalRange)
    {
    CurveVectorPtr curves = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Outer);
    curves->push_back (ICurvePrimitive::CreateLine (DSegment3d::From (0,-1,0, 1,0,0)));
    curves->push_back (ICurvePrimitive::CreateArc  (DEllipse3d::FromPointsOnArc (
                DPoint3d::From (1,0,0),
                DPoint3d::From (2,1,0),
                DPoint3d::From (1,2,0))));
    bvector<DPoint3d> poles;
    poles.push_back (DPoint3d::From (1,2,0));
    poles.push_back (DPoint3d::From (1,1,0));
    poles.push_back (DPoint3d::From (0,0,0));
    poles.push_back (DPoint3d::From (0,-1,0));
    MSBsplineCurvePtr curve = MSBsplineCurve::CreateFromPolesAndOrder (poles, NULL, NULL, 3, false, false);
    curves->push_back (ICurvePrimitive::CreateBsplineCurve (*curve));
    DRange3d range0;
    
    if (Check::True (curves->GetRange (range0), "Simple range"))
        {
        double dz = range0.high.z - range0.low.z;
        
        Check::True (DoubleOps::AlmostEqual (dz, 0.0), "Original curve range is flat");
        CheckPlanarity (*curves, DPoint3d::From (0,0,0), DPoint3d::From (0,1,0), 0.1);
        CheckPlanarity (*curves, DPoint3d::From (0,0,0), DPoint3d::From (0,0,1), 0.1);
        CheckPlanarity (*curves, DPoint3d::From (0,0,0), DPoint3d::From (0,.1,1), 0.01);
        CheckPlanarity (*curves, DPoint3d::From (1,0,1), DPoint3d::From (1,1,1), Angle::PiOver2 ());
        CheckPlanarity (*curves, DPoint3d::From (1,0,1), DPoint3d::From (1,1,1), 0.01);
        CheckPlanarity (*curves, DPoint3d::From (1,2,4), DPoint3d::From (0.4,0.3, 1.9), 0.4);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (CurveVectorA, IsPlanar_line)
    {
    CurveVectorPtr curves = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
    curves->push_back (ICurvePrimitive::CreateLine (DSegment3d::From (0,-1,0, 1,0,0)));
    DRange3d range0;
    if (Check::True (curves->GetRange (range0), "Simple range"))
        {
        double dz = range0.high.z - range0.low.z;
        static double fFactor = 981.0 / 13.0;
        static double fMax = 1e10;
        Check::True (DoubleOps::AlmostEqual (dz, 0.0), "Original curve range is flat");
        for (double f = 13.0 / 7.0; f < fMax; f *= fFactor)
            {
            CheckPlanarity (*curves, DPoint3d::From (0,0,0), DPoint3d::From (0,f,0), 0.1);
            CheckPlanarity (*curves, DPoint3d::From (0,0,0), DPoint3d::From (0,0,f), 0.1);
            CheckPlanarity (*curves, DPoint3d::From (0,0,0), DPoint3d::From (0,.1 * f,f), 0.01);
            CheckPlanarity (*curves, DPoint3d::From (f,0,f), DPoint3d::From (f,f,f), Angle::PiOver2 ());
            CheckPlanarity (*curves, DPoint3d::From (f,0,f), DPoint3d::From (f,f,f), 0.01);
            CheckPlanarity (*curves, DPoint3d::From (f, 2.0 * f, 4.0 * f), DPoint3d::From (0.4 * f,0.3 * f, 1.9 * f), 0.4);
            }
        }
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (CurveVectorA, DefaultPlaneNormal)
    {
    CurveVectorPtr curves = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Outer);
    DPoint3d pointA = DPoint3d::From (0,-1,0);
    DPoint3d pointB = DPoint3d::From (2,0,0.1);
    curves->push_back (ICurvePrimitive::CreateLine (DSegment3d::From (pointA, pointB)));
    DRange3d range0, range1;
    Transform worldToLocal, localToWorld;
    
    Check::True (curves->IsPlanarWithDefaultNormal (localToWorld, worldToLocal, range0, NULL),
                "Planarity for single line, default normal");
    DVec3d normal1 = DVec3d::From (1,2,4);
    Check::True (curves->IsPlanarWithDefaultNormal (localToWorld, worldToLocal, range1, &normal1),
                "Planarity for single line, explicit normal");
    }



// Construct a line segment between specified fractions of given segment. 
// Expand it so that the intersection points are at fB0 and fB1.
// Confirm that the computed intersection matches.
// optionalB = caller supplied exact data for segmentB (to debug tolerance issues)
static bool VerifySegmentOverlap (DSegment3dCR segmentA, double fB0, double fB1, DSegment3dCP optionalB = NULL)
    {
    bool stat = false;
    Check::StartScope ("SegmentOverlap");
    // Create B with specified limits ...
    DSegment1d fractionsBonA (fB0, fB1);
    DSegment1d fractions01 (0,1);
    
    double a0B, a1B;
    fractionsBonA.PointToFraction (0.0, a0B);
    fractionsBonA.PointToFraction (1.0, a1B);
    DSegment1d fractionsAonB (a0B, a1B);

    auto intersectionFractionsOnA = fractionsBonA.DirectedOverlap (fractions01);
    auto intersectionFractionsOnB = fractionsAonB.DirectedOverlap (fractions01);
    
    int numIntersection =  (intersectionFractionsOnA.IsValid () && intersectionFractionsOnB.IsValid ())
                        ? 1 : 0;
    if (fB0 > fB1)
        {
        //intersectionFractionsOnB.ReverseInPlace ();
        intersectionFractionsOnA.Value ().ReverseInPlace ();
        }
    DSegment3d segmentB = DSegment3d::FromFractionInterval (segmentA, fB0, fB1);
    if (NULL != optionalB)
        segmentB = *optionalB;

    CurveVectorPtr intersectionsA = CurveVector::Create (CurveVector::BOUNDARY_TYPE_None);
    CurveVectorPtr intersectionsB = CurveVector::Create (CurveVector::BOUNDARY_TYPE_None);

    CurveVectorPtr cVectorA = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
    CurveVectorPtr cVectorB = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);

    cVectorA->push_back (ICurvePrimitive::CreateLine(segmentA));
    cVectorB->push_back (ICurvePrimitive::CreateLine(segmentB));

    DMatrix4d matrix;
    matrix.InitIdentity ();
    PartialCurveDetail detailA, detailB;
    CurveCurve::IntersectionsXY(*intersectionsA, *intersectionsB, *cVectorA, *cVectorB, &matrix);
    if (numIntersection == 0)
        {
        stat = Check::Size (intersectionsA->size (), numIntersection, "Segment Overlap")
            && Check::Size (intersectionsB->size (), numIntersection, "Segment Overlap");
        }
    else if (   Check::Size (intersectionsA->size (), numIntersection, "Segment Overlap")
        && Check::Size (intersectionsB->size (), numIntersection, "Segment Overlap")
        && Check::True (CurveCurve::GetPartialCurveDetailPair (*intersectionsA, *intersectionsB, 0, detailA, detailB), "DetailAccess")
        )
        {
        stat = Check::Near (intersectionFractionsOnA.Value ().GetStart (), detailA.fraction0, "CCI fractionA0")
            && Check::Near (intersectionFractionsOnA.Value ().GetEnd (),   detailA.fraction1, "CCI fractionA1")
            && Check::Near (intersectionFractionsOnB.Value ().GetStart (), detailB.fraction0, "CCI fractionB0")
            && Check::Near (intersectionFractionsOnB.Value ().GetEnd (),   detailB.fraction1, "CCI fractionB1");
        }
    Check::EndScope ();
    return stat;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (CurveVectorA, SegmentOverlap)
    {
    VerifySegmentOverlap (DSegment3d::From (0,0,0, 1,0,0), 0, 1);
    VerifySegmentOverlap (DSegment3d::From (0,0,0, 1,0,0), 0.5, 1);
    VerifySegmentOverlap (DSegment3d::From (0,0,0, 1,0,0), 0.5, 1.5);
    VerifySegmentOverlap (DSegment3d::From (1,1,0, 2,4,0), 0.5, 1.5);

    VerifySegmentOverlap (DSegment3d::From (0,0,0, 1,0,0), 1, 0);
    VerifySegmentOverlap (DSegment3d::From (0,0,0, 1,0,0), 1, 0.5);
    VerifySegmentOverlap (DSegment3d::From (0,0,0, 1,0,0), 1.5, 0.5);
    VerifySegmentOverlap (DSegment3d::From (1,1,0, 2,4,0), 1.5, 0.5);

    VerifySegmentOverlap (DSegment3d::From (0,0,0, 1,0,0), 0.5, -0.2);
    VerifySegmentOverlap (DSegment3d::From (1,1,0, 2,4,0), 1.5, -0.5);
    DSegment3d segmentB = DSegment3d::From (40,0,0, 20,0,0);
    DSegment3d segmentA = DSegment3d::From (0,0,0, 100,0,0);
    VerifySegmentOverlap (segmentA, 0.40, 0.20, &segmentB);
    VerifySegmentOverlap (segmentB, 2.0, -3.0, &segmentA);
    VerifySegmentOverlap (DSegment3d::From (0,0,0, 100,0,0), 0.25, 0.50);
    

    }



// Check that principal moments are computable and agree (allowing for applied scale -- caller scaled geometry for product1)
void ComparePrincipalMoments
    (
    DMatrix4d product0,
    DMatrix4d product1,
    double appliedScale = 1.0,
    int domainDimension = 2
    )
    {
    char message[1024];
    sprintf (message, "MomentComparison (dim %d) (scale %g)", domainDimension, appliedScale);
    Check::StartScope ("Compare moments");
    double q0, q1;
    DVec3d centroid0, centroid1;
    RotMatrix axes0, axes1;
    DVec3d moment0, moment1;
    if (Check::True (product0.ConvertInertiaProductsToPrincipalMoments (q0, centroid0, axes0, moment0), "principalmoment0")
        && Check::True (product1.ConvertInertiaProductsToPrincipalMoments (q1, centroid1, axes1, moment1), "principalMoment1"))
        {
        DVec3d scaledMoment0;
        scaledMoment0.Scale (moment0, pow (appliedScale, domainDimension + 2));
        Check::Near (scaledMoment0, moment1, "moment diagonal");
        }
    Check::EndScope ();
    }
        


void ExerciseRegion (CurveVectorCR region)
    {
    IFacetOptionsPtr options = IFacetOptions::Create ();
    
    // Get a nontrivial rigid tranformation 
    DPoint3d origin1 = DPoint3d::From (1,2,3);
    RotMatrix axes1 = RotMatrix::FromVectorAndRotationAngle (DVec3d::From( 0,2,3), 1.5);
    Transform transform1 = Transform::From (axes1, origin1);

    // facet the original region
    options->SetMaxPerFace (3);
    IPolyfaceConstructionPtr builder = IPolyfaceConstruction::Create (*options);
    builder->AddRegion (region);
    PolyfaceHeaderPtr facets = builder->GetClientMeshPtr ();
    
    // get an xy form of the curvevector ......
    Transform localToWorld, worldToLocal;
    DRange3d localRange;
    CurveVectorPtr xyRegion = region.CloneInLocalCoordinates (LOCAL_COORDINATE_SCALE_UnitAxesAtLowerLeft, localToWorld, worldToLocal, localRange);
    CurveVectorPtr region1 = xyRegion->Clone ();
    // push the xy form to a back out into space ....
    region1->TransformInPlace (transform1);    
    
    if (Check::True (xyRegion.IsValid (), "CloneInLocalCoordinates"))
        {
        PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach (*facets);
        double a = 1.0 / 3.0;
        bvector<DPoint3d> &points = visitor->Point ();
        for (visitor->Reset (); visitor->AdvanceToNextFace ();)
            {
            if (Check::Size (3, points.size (), "Region facet is triangle"))
                {
                // facet centroid is on original CV
                DPoint3d spaceCentroid = DPoint3d::FromSumOf (points[0], a, points[1], a, points[2], a);
                // back to the local uvw
                DPoint3d planeCentroid = DPoint3d::FromProduct (worldToLocal, spaceCentroid);
                CurveVector::InOutClassification inout = xyRegion->PointInOnOutXY (planeCentroid);
                Check::True (inout == CurveVector::INOUT_In, "Facet centroid is IN its parent region");
                DRay3d ray1 = DRay3d::FromOriginAndVector (planeCentroid, DVec3d::From (0,0,1));
                DRay3d ray2;
                // RayPierceInOnOut is "the same test" by free of xy assumptions.
                // detail1 confirms this with a z ray.
                // detail2 confirms this with a ray and region that are both transformed.
                SolidLocationDetail detail1, detail2;
                CurveVector::InOutClassification inout1 = xyRegion->RayPierceInOnOut (ray1, detail1);
                DPoint3d xyz1;
                transform1.Multiply (ray2, ray1);
                transform1.Multiply (xyz1, detail1.GetXYZ ());
                CurveVector::InOutClassification inout2 = region1->RayPierceInOnOut (ray2, detail2);
                Check::Int ((int)inout1, (int)inout2, "ray pierce");
                Check::Near (xyz1, detail2.GetXYZ ());
                
                DPoint3d spacePoint3 = ray2.FractionParameterToPoint (5.5);
                DPoint3d closePoint3;
                Check::True (CurveVector::INOUT_In == region1->ClosestCurveOrRegionPoint (spacePoint3, closePoint3), "ClosestPoint IN");
                Check::Near (xyz1, closePoint3, "ClosestCurveOrRegionPoint xyz");
                }
            }
        bvector<MSBsplineSurfacePtr> bsurfs;
        MSBsplineSurface::CreateTrimmedSurfaces (bsurfs, region);
        
        }
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (CurveVectorA, Centroid)
    {
    double a = 3.0;
    double b = 1.0;
    double z0 = 0.0;
    double div180 = 1.0 / 180.0;
    double theta = 30*div180*Angle::Pi();
    double cs = cos(theta);
    double sn = sin(theta);
    DPoint3d centroid;
    DVec3d normal;
    double area;
    Transform rot;
    rot.InitFromRowValues 
    (
    cs, -sn, 0, 0,
    sn, cs,  0, 0,
    0,  0,   1, 0
    );

    CurveVectorPtr rectangle = CurveVector::CreateRectangle
                (
                0, 0,
                a, b, z0,
                CurveVector::BOUNDARY_TYPE_Outer
                );
    CurveVectorPtr rectangle1 = CurveVector::CreateRectangle
                (
                -0.5*a, -0.5*b,
                0.5*a,0.5*b, z0,
                CurveVector::BOUNDARY_TYPE_Outer
                );
    CurveVectorPtr rotrec = rectangle1->Clone ();
    rotrec->TransformInPlace (rot);
 
    if (Check::True (rectangle->CentroidNormalArea (centroid, normal, area)))
        {
        Check::Near (DPoint3d::From (a * 0.5, b * 0.5, z0), centroid, "Rectangle centroid");
        Check::Near (DVec3d::From (0,0,1), normal, "Rectangle normal");
        }
    DMatrix4d product0 = DMatrix4d::FromZero(), product1 = DMatrix4d::FromZero();
    if (Check::True (rectangle1->ComputeSecondMomentAreaProducts (product0) && Check::True (rotrec->ComputeSecondMomentAreaProducts (product1))))
        {
        ComparePrincipalMoments (product0, product1);
        }
        
    CheckWireMoments (*rectangle);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (CurveVectorA, AreaMomentsEllipse1)
    {
    double scaleFactor = 4.0;
    double div180 = 1.0 / 180.0;
    double theta = 30*div180*Angle::Pi();
    double cs = cos(theta);
    double sn = sin(theta);
    
    int siz = 6;
    double devi[6];
    
    Transform rot;
    rot.InitFromRowValues 
        (
        cs, -sn, 0, 0,
        sn, cs,  0, 0,
        0,  0,   1, 0
        );
    Transform scale;
    scale.InitFromRowValues 
        (
        scaleFactor, 0,   0, 0,
        0, scaleFactor,   0, 0,
        0,   0, scaleFactor, 0
        );

    DPoint3d center = DPoint3d::From (0, 0, 0);
    DPoint3d point0 = DPoint3d::From (2, 0, 0);
    DPoint3d point90 = DPoint3d::From (0, 1, 0);

    DEllipse3d ellipseData = DEllipse3d::FromPoints (center, point0, point90, 0.0, Angle::TwoPi());
    
    CurveVectorPtr ellipse = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Outer);
    ellipse->push_back (ICurvePrimitive::CreateArc  (ellipseData));           
        
    CurveVectorPtr rotellip = ellipse->Clone ();
    rotellip->TransformInPlace (rot);
    
    CurveVectorPtr sclellip = ellipse->Clone ();
    sclellip->TransformInPlace (scale);
    
    IFacetOptionsPtr options = IFacetOptions::CreateForCurves ();
    DMatrix4d product0 = DMatrix4d::FromZero(), product1 = DMatrix4d::FromZero();
    for(int i = 0; i< siz; i++)
        {
        options->SetAngleTolerance (theta/pow(2.0,i));
        CurveVectorPtr strokedShape = ellipse->Stroke(*options);
        //int numStroke = ((int)strokedShape->at(0)->GetLineStringCP ()->size());

        if (   Check::True (ellipse->ComputeSecondMomentAreaProducts (product0)
                            && strokedShape->ComputeSecondMomentAreaProducts (product1)))
            {
            devi[i] = fabs(product0.coff[0][0] - product1.coff[0][0]);
            //printf ("xx delta = %22.5f numstrokes = %i\n", devi[i], numStroke);
            }   
        }
    if (Check::True (ellipse->ComputeSecondMomentAreaProducts (product0)
                    && rotellip->ComputeSecondMomentAreaProducts (product1)))
        {
        ComparePrincipalMoments (product0, product1);
        }   
    
    if (Check::True (   ellipse->ComputeSecondMomentAreaProducts (product0)
                     && sclellip->ComputeSecondMomentAreaProducts (product1)))
        {
        ComparePrincipalMoments (product0, product1, scaleFactor, 2);
        }
        
    CurveVectorPtr spline = ellipse->CloneAsBsplines ();
    if (Check::True (ellipse->ComputeSecondMomentAreaProducts (product0)
                    && spline->ComputeSecondMomentAreaProducts (product1)))
        {
        //ComparePrincipalMoments (product0, product1);
        double dQ, dX, dY, dW;
        double aQ, aX, aY, aW;
        product0.MaxAbsDiff (product1, dQ, dX, dY, dW);
        product0.MaxAbs     (aQ, aX, aY, aW);
#ifdef PrintEllispse1Details
        printf (" (dQ %8.1le) (dX %8.1le) (dW %8.1le)\n", dQ, dX, dW);
        printf (" (aQ %8.1le) (aX %8.1le) (aW %8.1le)\n", aQ, aX, aW);
        printf (" (ellipse area %.16g) (spline area %.16g) (diff %8.1le)\n",
                    product0.coff[3][3], product1.coff[3][3],
                    product0.coff[3][3] - product1.coff[3][3]);
#endif
        Check::Near (product0.coff[3][3], product1.coff[3][3], "Ellipse, spline areas");
        Check::Near (aQ, aQ + dQ, "Ellipse, spline rotational diff");
        Check::Near (aX, aX + dX, "Ellipse, spline rotational diff");
        Check::Near (aW, aW + dW, "Ellipse, spline weight diff");
        }
    Transform translation = Transform::From (1, 2, 0);

    CurveVectorPtr ellipse1 = ellipse->Clone ();
    CurveVectorPtr spline1  = spline->Clone ();
    ellipse1->TransformInPlace (translation);
    spline1->TransformInPlace (translation);
    DMatrix4d product2 = DMatrix4d::FromZero(), product3 = DMatrix4d::FromZero();
    if (Check::True (ellipse1->ComputeSecondMomentAreaProducts (product2)
                    && spline1->ComputeSecondMomentAreaProducts (product3)))
        {
        ComparePrincipalMoments (product0, product2);
        ComparePrincipalMoments (product1, product3);
        }
    CheckWireMoments (*ellipse);
    CheckWireMoments (*spline);
#ifdef abc
    DMatrix4d product4 = DMatrix4d::FromZero(), product5 = DMatrix4d::FromZero();
    if (Check::True (ellipse1->ComputeSecondMomentWireProducts (product4)
                    && spline1->ComputeSecondMomentWireProducts (product5)))
        {
        ComparePrincipalMoments (product4, product5);
        }
#endif
    }
    
#ifdef abc
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
    TEST (CurveVectorA, test)
    {
    int result1 = (int)pow(3.0,2.0);
    int result4 = (int)pow(3.0,2.0);
    double result2 = exp(2.0 * log(3.0));
    int result3 = (int)result2;
    printf ("%d %24.15le %d %d\n", result1, result2, result3, result4);

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
    TEST (CurveVectorA, fib1)
    {
    double fib[20];
    fib[0] = 0;
    fib[1] = 1;
    double pwr[4];
    pwr[0] = 2;
    pwr[1] = 3;
    pwr[2] = 5;
    pwr[3] = 8;
    double res[80];
    
    for (int i = 2; i < 20; i++)
        {
        fib[i] = fib[i-1] + fib[i-2];
        }
        
    for (int i = 0; i < 20; i++)
        {
        res[4*i] = pow(fib[i], pwr[0]);
        res[4*i + 1] = pow(fib[i], pwr[1]);
        res[4*i + 2] = pow(fib[i], pwr[2]);
        res[4*i + 3] = pow(fib[i], pwr[3]);
        printf ("%f %22.20le \n", fib[i], res[i]);
        }
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
    TEST (CurveVectorA, fib2)
    {
    double fib[20];
    fib[0] = 0;
    fib[1] = 1;
    double pwr = 1.0 / 3.0;
    double res[80];
    
    for (int i = 2; i < 20; i++)
        {
        fib[i] = fib[i-1] + fib[i-2];
        }
        
    for (int i = 0; i < 20; i++)
        {
        res[i] = pow(fib[i]*fib[i]*fib[i], pwr);
        printf ("%22.5f  %22.5f %22.15f\n", fib[i], pwr, res[i]);
        }
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (CurveVectorA, fib3)
    {
    double fib[20];
    fib[0] = 0;
    fib[1] = 1;
    double pwr = 1.0 / 3.0;
    double res[80];
    
    for (int i = 2; i < 20; i++)
        {
        fib[i] = fib[i-1] + fib[i-2];
        }
        
    for (int i = 0; i < 20; i++)
        {
        res[i] = pow(fib[i], pwr)*pow(fib[i], pwr)*pow(fib[i], pwr);
        printf ("%22.5f  %22.5f %22.15f\n", fib[i], pwr, res[i]);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (CurveVectorA, elliperror)
    {
    double div2 = 1.0 / 2.0;
    double pi = Angle::Pi();
    double conv = pi / 180.0;
    double start = 30.0;
    double end = 60.0;
    double sweepdeg = end - start;
    double t0 = start * conv;
    double t1 = end * conv;
    double a = 2.0;
    double a2 = a * a;
    double diva = 1  / a;
    double diva2 = diva * diva;
    double b = 1.0;
    double tn0 = tan(t0);
    double tn1 = tan(t1);
    
    /*double cs0 = cos(t0);
    double sn0 = sin(t0);
    double cs1 = cos(t1);
    double sn1 = sin(t1);*/
    
    double x0 = a*b / sqrt(b*b + a*a*tn0*tn0);
    double x1 = a*b / sqrt(b*b + a*a*tn1*tn1);
    double x02 = x0 * x0;
    double x12 = x1 * x1;
    
    double perc = 1 - diva2 * sqrt(a2*(x12 + x02) - 2*x12*x02) / (fabs(asin(x1*diva) - asin(x0*diva)) + div2 * fabs((x1*sqrt(1 - x12*diva2) - x0*sqrt(1 - x02*diva2))));
    
    printf ("percent error: %22.5f\n sweep angle: %lf\n", 100*perc, sweepdeg);
    }
#endif


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (CurveVectorA, Holes)
    {
    DRange2d outer = DRange2d::From (0,0, 20,10);
    DRange2d innerA = DRange2d::From (1,1,10,3);
    DRange2d innerB = DRange2d::From (11,2,14,3);

    CurveVectorPtr parityRegion = CurveVector::CreateRectangle (outer.low.x, outer.low.y, outer.high.x, outer.high.y, 0,
            CurveVector::BOUNDARY_TYPE_ParityRegion);
    parityRegion->push_back (
        ICurvePrimitive::CreateChildCurveVector_SwapFromSource (
            *CurveVector::CreateRectangle (innerA.low.x, innerA.low.y, innerA.high.x, innerA.high.y,  0,
                            CurveVector::BOUNDARY_TYPE_Inner)));
    parityRegion->push_back (
        ICurvePrimitive::CreateChildCurveVector_SwapFromSource (
                *CurveVector::CreateRectangle (innerB.low.x, innerB.low.y, innerB.high.x, innerB.high.y,  0,
                            CurveVector::BOUNDARY_TYPE_Inner)));
    double area = outer.Area () - innerA.Area () - innerB.Area ();
    DMatrix4d products;
    Check::True (parityRegion->ComputeSecondMomentAreaProducts (products), "ComputedMoments");
    Check::Near (area, products.coff[3][3], "computed area");
    
    ExerciseRegion (*parityRegion);
    ExercisePointsAlong (*parityRegion, 25);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (CurveVector, MeshHolesAndLinearFeatures)
    {

    // Create a parent BOUNDARY_TYPE_ParityRegion
    CurveVectorPtr parityRegion = CurveVector::Create (CurveVector::BOUNDARY_TYPE_ParityRegion);

    // Insert the outer loop . . .
    parityRegion->Add (
        CurveVector::CreateLinear (
                bvector<DPoint3d> {
                    DPoint3d::From (0,0),
                    DPoint3d::From (20,0),
                    DPoint3d::From (25,5),
                    DPoint3d::From (20,10),
                    DPoint3d::From (0,10),
                    DPoint3d::From (0,0)},
                CurveVector::BOUNDARY_TYPE_Outer));

    // Insert inner loops . . .
    parityRegion->Add (
        CurveVector::CreateLinear (
                bvector<DPoint3d> {
                    DPoint3d::From (1,1),
                    DPoint3d::From (9,1),
                    DPoint3d::From (9,3),
                    DPoint3d::From (8,3),
                    DPoint3d::From (8,3),
                    DPoint3d::From (5,5),
                    DPoint3d::From (1,3),
                    DPoint3d::From (1,1)},
                CurveVector::BOUNDARY_TYPE_Inner));

    parityRegion->Add (
        CurveVector::CreateLinear (
                bvector<DPoint3d> {
                    DPoint3d::From (10,2),
                    DPoint3d::From (15,3),
                    DPoint3d::From (14,3.5),
                    DPoint3d::From (10,3.5),
                    DPoint3d::From (10,2)},
                CurveVector::BOUNDARY_TYPE_Inner));

    // Create a polyface builder . . . 
    IFacetOptionsPtr options = IFacetOptions::Create ();
    // facet the original region
    options->SetMaxPerFace (3);
    IPolyfaceConstructionPtr builder1 = IPolyfaceConstruction::Create (*options);

    // Add the parity region to the mesh  . . .
    builder1->AddRegion (*parityRegion);

    // Grab the mesh from the builder . . .
    PolyfaceHeaderPtr facets = builder1->GetClientMeshPtr ();
    facets->MarkAllEdgesVisible ();

    Check::SaveTransformed (*parityRegion);
    Check::Shift (30,0,0);
    Check::SaveTransformed (facets);
    Check::Shift (30,0,0);

    // Now another mesh, this time with linear features imposed .  ..

    CurveVectorPtr linearFeatures = CurveVector::Create (CurveVector::BOUNDARY_TYPE_None);
    linearFeatures->push_back (ICurvePrimitive::CreateLineString (
            bvector<DPoint3d> {
                DPoint3d::From (5,8), DPoint3d::From (7,8), DPoint3d::From (12,9), DPoint3d::From (15,9)
            }));


    PolyfaceHeaderPtr facets2 = PolyfaceHeader::CreateConstrainedTriangulation (*parityRegion, linearFeatures.get (), nullptr);
    facets2->MarkAllEdgesVisible ();
    Check::SaveTransformed (facets2);

    Check::ClearGeometry ("CurveVector.MeshHolesAndLinearFeatures");
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (CurveVectorA, Consolidate)
    {
    CurveVectorPtr pathA = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
    DPoint3d pointA = DPoint3d::From (0,0,0);
    DPoint3d pointB = DPoint3d::From (1,0,0);
    DPoint3d pointC = DPoint3d::From (1,1,0);
    DPoint3d pointD = DPoint3d::From (2,1,0);
    DPoint3d pointE = DPoint3d::From (3,1,0);
    pathA->push_back (ICurvePrimitive::CreateLine (DSegment3d::From (pointA, pointB)));
    // Exact match ...
    pathA->push_back (ICurvePrimitive::CreateLine (DSegment3d::From (pointB, pointC)));
    // Gap ... 
    pathA->push_back (ICurvePrimitive::CreateLine (DSegment3d::From (pointD, pointE)));
    CurveVectorPtr pathB = pathA->Clone ();
    pathB->ConsolidateAdjacentPrimitives ();
        ExercisePointsAlong (*pathB, 25);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(MSBsplineSurface, Disk)
    {
    Check::StartScope ("bsurf disk bounds checks");
    DEllipse3d disk = DEllipse3d::From (
            1,2,0,
            1,0,0,
            0,1,0,
            0.0, Angle::TwoPi ());
            
    MSBsplineSurfacePtr diskSurface = MSBsplineSurface::CreateTrimmedDisk (disk);
    DPoint2d uv;
    uv.Init (0.55, 0.52);
    Check::True (bsputil_pointInBounds (&uv, diskSurface->boundaries, diskSurface->numBounds, diskSurface->holeOrigin != 0), "(in)");
    Check::True (bsputil_pointOnSurface (&uv, diskSurface.get ()),"_pointOnSurface");
    uv.Init (0.91, 0.93);
    Check::False (bsputil_pointInBounds (&uv, diskSurface->boundaries, diskSurface->numBounds, diskSurface->holeOrigin != 0), "(out)");
    
    bvector<double> fractions;
    bspsurf_intersectBoundariesWithUVLine (&fractions, 0.51, diskSurface.get (), true);
    Check::Size (2, fractions.size (), "Horizontal Cut");
    Check::EndScope ();
    }
    

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(MSBsplineSurface, LinearBoundaries)
    {
    Check::StartScope ("bsurf linear bounds checks");
    auto linearPrim = CurveVector::CreateLinear (
        bvector<DPoint3d> {
            DPoint3d::From (0.3, 0.3),
            DPoint3d::From (0.8, 0.3),
            DPoint3d::From (0.7,0.6),
            DPoint3d::From (0.4, 0.6),
            DPoint3d::From (0.3, 0.3)},
            CurveVector::BOUNDARY_TYPE_Outer);

    auto bsurf = SurfaceWithSinusoidalControlPolygon (
                3,4,4, 7, 0.0, 0.4, 0.1, 0.8,
                0.0
                );
    bsurf->SetTrim (*linearPrim);
    Check::SaveTransformed (bsurf);
    DPoint2d uv;
    uv.Init (0.55, 0.52);
    Check::True (bsputil_pointInBounds (&uv, bsurf->boundaries, bsurf->numBounds, bsurf->holeOrigin != 0), "(in)");
    Check::True (bsputil_pointOnSurface (&uv, bsurf.get ()),"_pointOnSurface");
    uv.Init (0.91, 0.93);
    Check::False (bsputil_pointInBounds (&uv, bsurf->boundaries, bsurf->numBounds, bsurf->holeOrigin != 0), "(out)");
    
    bvector<double> fractions;
    bspsurf_intersectBoundariesWithUVLine (&fractions, 0.51, bsurf.get (), true);
    Check::Size (2, fractions.size (), "Horizontal Cut");
    bspsurf_intersectBoundariesWithUVLine (&fractions, 0.51, bsurf.get (), false);
    Check::Size (2, fractions.size (), "Vertical Cut");
    Check::EndScope ();
    Check::ClearGeometry ("MSBsplineSurface.LinearBoundaries");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(MSBsplineSurface,PCurveStroke)
    {
    ISolidPrimitivePtr cylinder = ISolidPrimitive::CreateDgnCone (
                DgnConeDetail (
                        DPoint3d::From (0,0,0),
                        DPoint3d::From (0,0,3),
                        2,
                        3,
                        false)
                );
    bvector<MSBsplineSurfacePtr> surfaces;
    MSBsplineSurface::CreateTrimmedSurfaces (surfaces, *cylinder);
    Check::Size (1, surfaces.size (), "Open cone converts to one surface");
    bvector <DPoint3d> pcurvePoints;
    pcurvePoints.push_back (DPoint3d::From (0.0,0.2, 0));
    pcurvePoints.push_back (DPoint3d::From (0.1,0.2, 0));
    pcurvePoints.push_back (DPoint3d::From (0.4,0.5, 0));
    pcurvePoints.push_back (DPoint3d::From (0.8,0.5, 0));
    MSBsplineCurvePtr curve = MSBsplineCurve::CreateFromPolesAndOrder (pcurvePoints, NULL, NULL, 3, false, true);
    bvector<DPoint3d> curvePoints, surfacePoints;
    Check::Int (SUCCESS, bspsurf_appendPCurveStrokes (&curvePoints, &surfacePoints, 0.001, 0.001, 5,
            curve.get (),
            surfaces[0].get (), 0.0, 1.0),
                "PCurve strokes");
    Check::True (curvePoints.size () > 5, "Curve points");
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(CurvePrimitive,CCI0)
    {
    ICurvePrimitivePtr line0 = ICurvePrimitive::CreateLine (DSegment3d::From (0,0,0, 10,0,0));
    ICurvePrimitivePtr arc1 = ICurvePrimitive::CreateArc (DEllipse3d::From (5,10,0,    10,0,0,   0, 10,0,  0.0, Angle::TwoPi ()));
    Check::False (line0->IsSameStructure (*arc1), "line !=structure=! arc");
    Check::True (arc1->IsSameStructure (*arc1), "arc == self");
    Check::True (line0->IsSameStructure (*line0), "arc == self");
    DPoint3d akimaPoints [] =
        {
        {5,-5,0},
        {5,-4,0},
        {5,-3,0},
        {5.9,0.1,0},    // Hm, where does this really go?
        {7,2,0},
        {8,3,0},
        {9,4,0},
        };
    ICurvePrimitivePtr akima0 = ICurvePrimitive::CreateAkimaCurve (akimaPoints, 7);
    CurveVectorPtr curvesA = CurveVector::Create (CurveVector::BOUNDARY_TYPE_None);
    CurveVectorPtr curvesC = CurveVector::Create (CurveVector::BOUNDARY_TYPE_None);
    
    curvesC->push_back (line0);
    curvesC->push_back (arc1);
    curvesA->push_back (ICurvePrimitive::CreateChildCurveVector (curvesC));
    CurveVectorPtr curvesB = CurveVector::Create (CurveVector::BOUNDARY_TYPE_None);
    curvesB->push_back (akima0);
    CurveVectorPtr intersectionsA0 = CurveVector::Create (CurveVector::BOUNDARY_TYPE_None);
    CurveVectorPtr intersectionsB0 = CurveVector::Create (CurveVector::BOUNDARY_TYPE_None);

    CurveVectorPtr intersectionsA1 = CurveVector::Create (CurveVector::BOUNDARY_TYPE_None);
    CurveVectorPtr intersectionsB1 = CurveVector::Create (CurveVector::BOUNDARY_TYPE_None);

    CurveCurve::IntersectionsXY (*intersectionsA0, *intersectionsB0, *curvesA, *curvesB, NULL);
    
    CurveCurve::IntersectionsXY (*intersectionsB1, *intersectionsA1, *curvesB, *curvesA, NULL);
    size_t expectedIntersections = 2;
    Check::Size (expectedIntersections, intersectionsA0->size (), "AB intersection count");
    Check::Size (expectedIntersections, intersectionsA1->size (), "BA intersection count");
    }
    
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(PointString, ClosestPointBounded0)
    {
    Check::StartScope ("Point String");
    
    bvector<DPoint3d> points;
    points.push_back (DPoint3d::From (0,0,0));
    points.push_back (DPoint3d::From (10,0,0));
    points.push_back (DPoint3d::From (5,1,0));
    
    ICurvePrimitivePtr cp = ICurvePrimitive::CreatePointString (points);
    // This point is exactly on the midpoint of the (nonexistent) first segment.  The closest
    // pointstring xyz is the last point  ...
    DPoint3d xyz = DPoint3d::From (5,0,0);
    CurveLocationDetail detail;
    Check::True (cp->ClosestPointBounded (xyz, detail), "ClosestPointBounded");
    Check::Near (points.back (), detail.point, "ClosestPointBounded");


    CurveLocationDetail detail1;
    Check::True (cp->ClosestPointBoundedXY (xyz, NULL, detail1), "ClosestPointBoundedXY");
    Check::Near (points.back (), detail1.point, "ClosestPointBoundedXYB");



    Check::EndScope ();
    }
    
void ConfirmClosestPointBounded (ICurvePrimitiveCR curve, DPoint3dCR spacePoint, CurveLocationDetailCR detail)
    {
    double a = spacePoint.Distance (detail.point);
    size_t n = 64;
    size_t numFail = 0;
    for (size_t i = 0; i <= n; i++)
        {
        double f = i / (double)n;
        DPoint3d xyz;
        curve.FractionToPoint (f, xyz);
        double b = spacePoint.Distance (xyz);
        if (DoubleOps::AlmostEqual (a, b) || a < b)
            {
            }
        else
            {
            numFail++;
            }
        }
    Check::Size (0, numFail, "ClosestPointBounded sample");        
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(ClosestPointBounded, EllipseCenter)
    {
    Check::StartScope ("ClosestPointBounded_EllipseCenter");
    CurveLocationDetail detail;
    DPoint3d center = DPoint3d::From (0,0,0);    
    DEllipse3d ellipse12 = DEllipse3d::FromVectors (
                center,
                DVec3d::From (1,0,0),
                DVec3d::From (0,2,0),
                -Angle::Pi () * 0.25,
                Angle::Pi ()
                );
                
    DEllipse3d ellipse11 = DEllipse3d::FromVectors (
                center,
                DVec3d::From (1,0,0),
                DVec3d::From (0,1,0),
                -Angle::Pi () * 0.25,
                Angle::Pi ()
                );

    DEllipse3d ellipse12Skew = DEllipse3d::FromVectors (
                center,
                DVec3d::From (1,0,0),
                DVec3d::From (0.1,2,0),
                -Angle::Pi () * 0.25,
                Angle::Pi ()
                );
    
    ICurvePrimitivePtr cp12 = ICurvePrimitive::CreateArc (ellipse12);
    ICurvePrimitivePtr cp11 = ICurvePrimitive::CreateArc (ellipse11);
    ICurvePrimitivePtr cp12Skew = ICurvePrimitive::CreateArc (ellipse12Skew);

    Check::True (cp12->ClosestPointBounded (ellipse12.center, detail), "ellipse12 center");
    ConfirmClosestPointBounded (*cp12, center, detail);
    Check::True (cp11->ClosestPointBounded (ellipse11.center, detail), "ellipse11 center");
    ConfirmClosestPointBounded (*cp11, center, detail);
    Check::True (cp12Skew->ClosestPointBounded (ellipse12Skew.center, detail), "ellipse12Skew center");
    ConfirmClosestPointBounded (*cp12Skew, center, detail);
    
    Check::EndScope ();
    }

size_t FindIntersectionIndex (DPoint3dCR xyz, bvector<CurveLocationDetailPair> &data, double tolerance)
    {
    for (size_t i = 0; i < data.size (); i++)
        {
        if (xyz.AlmostEqual (data[i].detailA.point, tolerance)
            || xyz.AlmostEqual (data[i].detailB.point, tolerance)
            )
            return i;
        }
    return data.size ();
    }

void CheckPrimitive_go (ICurvePrimitiveCR primitive)
    {
    double trueLength;
    double fastLength;
    DPoint3d xyz0, xyz1, xyz2;
    DVec3d tangent1, tangent2;
    DVec3d torsion2;
    double tolerance = Angle::SmallAngle () * primitive.FastMaxAbs ();
    // fluffy tolerance when bcurve is approximation .. .
    if (nullptr != primitive.GetProxyBsplineCurveCP ()
        && nullptr == primitive.GetBsplineCurveCP ()
        )
        tolerance *= 1.0e6;

    for (double f : bvector<double>{0, 0.25, 0.4, 0.5, 0.75, 1.0})
        {
        primitive.FractionToPoint (f, xyz0);
        primitive.FractionToPoint (f, xyz1, tangent1);
        primitive.FractionToPoint (f, xyz2, tangent2, torsion2);
        Check::Near (xyz0, xyz1, "FractionToPoint");
        Check::Near (xyz0, xyz2, "FractionToPoint");
        Check::Near (tangent1, tangent2, "FractionToPoint");
        DPlane3d plane = DPlane3d::FromOriginAndNormal (xyz1, tangent1);
        bvector<CurveLocationDetailPair> intersections;
        primitive.AppendCurvePlaneIntersections (plane, intersections, tolerance);

        Check::True (FindIntersectionIndex (xyz0, intersections, tolerance) < intersections.size (),
                "Transverse plane intersects curve");
        CurveLocationDetail cld;
        primitive.ClosestPointBounded (xyz0, cld);
        Check::True (xyz0.AlmostEqual (cld.point, tolerance));
        double curvature, torsion;
        Transform frame;
        if (Check::True (primitive.FractionToFrenetFrame (f, frame, curvature, torsion)))
            {
            }
        }
        
    Check::True (primitive.Length (trueLength), "True Length");
    Check::True (primitive.FastLength (fastLength), "Fast Length");
    // Hmmm.. don't know how close to expect.
    static double s_lengthRelTol = 2.0;
    Check::True (fabs (trueLength - fastLength) < s_lengthRelTol * trueLength, "reasonable fast length error");

    }    
void CheckPrimitive (ICurvePrimitiveCR primitive, bool doPartial = false)
    {
    CheckPrimitive_go (primitive);
    if (doPartial)
        {
        double a = 0.10;
        double b = 0.20;
        auto child = ICurvePrimitive::CreatePartialCurve (const_cast<ICurvePrimitiveP>(&primitive), a, b);
        CheckPrimitive_go (*child);
        }
    }    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(CurvePrimitive,TrivialPrimitives)
    {
    static bool s_doPartials = true;
    Transform frenet;
    double curvature, torsion;
    ICurvePrimitivePtr line = ICurvePrimitive::CreateLine (DSegment3d::From (1,2,3, 4,2,9));
    CheckPrimitive (*line, s_doPartials);
    if (Check::True (line->FractionToFrenetFrame (0.235, frenet, curvature, torsion)))
        {
        Check::Near (0.0, curvature);
        Check::Near (0.0, torsion);
        }
    bvector<DPoint3d> points;
    points.push_back (DPoint3d::From (0,0,0));
    points.push_back (DPoint3d::From (10,0,0));
    points.push_back (DPoint3d::From (5,1,0));
    points.push_back (DPoint3d::From (5,1,1));
    points.push_back (DPoint3d::From (5,10,1));
    
    ICurvePrimitivePtr linestring = ICurvePrimitive::CreateLineString (points);
    CheckPrimitive (*linestring, s_doPartials);
    // circular arc
    double r = 2.0;
    ICurvePrimitivePtr arc = ICurvePrimitive::CreateArc (DEllipse3d::From (0,0,0, r,0,0, 0,r,0, 0.3, 1.0));
    CheckPrimitive (*arc);
    if (Check::True (arc->FractionToFrenetFrame (0.234, frenet, curvature, torsion)))
        {
        Check::Near (1.0 / r, curvature);
        Check::Near (0.0, torsion);
        }

    // major/minor ellipse
    double a0 = 3.0;
    double a90 = 2.0;
    double kurvature0 = a0 / (a90 * a90);
    double kurvature90 = a90 / (a0 * a0);
    ICurvePrimitivePtr arc1 = ICurvePrimitive::CreateArc (DEllipse3d::From (0,0,0, a0,0,0, 0,a90,0, 0.0, Angle::TwoPi ()));
    CheckPrimitive (*arc1, s_doPartials);
    double curvature0, curvature90, curvature180, curvature270;
    if (Check::True (arc1->FractionToFrenetFrame (0.0, frenet, curvature0, torsion))
        && Check::True (arc1->FractionToFrenetFrame (0.25, frenet, curvature90, torsion))
        && Check::True (arc1->FractionToFrenetFrame (0.50, frenet, curvature180, torsion))
        && Check::True (arc1->FractionToFrenetFrame (0.75, frenet, curvature270, torsion))
        )
        {
        Check::Near (curvature0, curvature180);
        Check::Near (curvature90, curvature270);
        Check::Near (curvature0, kurvature0);
        Check::Near (curvature90, kurvature90);
        //Check::LessThanOrEqual (curvature90, 0.9 * curvature0);
        }

    // skew elliptic arc
    ICurvePrimitivePtr arc2 = ICurvePrimitive::CreateArc (DEllipse3d::From (0,0,0, 2,0,0, 0,3,1, 0.3, 1.0));
    CheckPrimitive (*arc2, s_doPartials);



    for (int order = 2; order < (int)points.size (); order++)
        {
        MSBsplineCurvePtr bcurvePtr = MSBsplineCurve::CreateFromPolesAndOrder (points, NULL, NULL, order, false, true);
        ICurvePrimitivePtr bcurvePrimPtr = ICurvePrimitive::CreateBsplineCurve (bcurvePtr);
        CheckPrimitive (*bcurvePrimPtr, s_doPartials);
        }
    DPoint3dDVec3dDVec3d dTri3d = DPoint3dDVec3dDVec3d(DPoint3d::From(0, 0, 0), DVec3d::From(2, 0, 0), DVec3d::From(0, 2, 0));

    auto cp0 = ICurvePrimitive::CreateCatenary(10, dTri3d, 2, 20);
    CheckPrimitive(*cp0, false);
    }    


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(CurvePrimitive,SizeSampler)
    {
    bvector<IGeometryPtr> curves;
    SampleGeometryCreator::AddSizeSampler (curves);  
    auto identity = DPoint3dDVec3dDVec3d::FromXYPlane ();
    ICurvePrimitivePtr catenary = ICurvePrimitive::CreateCatenary (1.0, identity, 0, 2);
    curves.push_back (IGeometry::Create (catenary));
    curves.push_back (IGeometry::Create (ICurvePrimitive::CreateCatenary (1.0, identity, 1,2)));
    curves.push_back (IGeometry::Create (ICurvePrimitive::CreateCatenary (1.0, identity, 0,-1)));
    curves.push_back (IGeometry::Create (ICurvePrimitive::CreateCatenary (1.0, identity, -1,2)));
    for (auto &gp : curves)
        {
        auto cp = gp->GetAsICurvePrimitive ();
        if (cp.IsValid ())
            CheckPrimitive (*cp);
        }
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(CurvePrimitive,GarbageDataFrenetFrame)
    {
    ICurvePrimitivePtr line0 = ICurvePrimitive::CreateLine (DSegment3d::From (1,2,3, 4,2,9));
    ICurvePrimitivePtr line1 = ICurvePrimitive::CreateLine (DSegment3d::From (4,3,2, 5,2,2));
    
    CurveVectorPtr collection = CurveVector::Create (CurveVector::BOUNDARY_TYPE_None);
    collection->push_back (line0);
    collection->push_back (line1);
    
    Transform frame;
    if (Check::True (collection->GetAnyFrenetFrame (frame), "Frenet frame accessed from BOUNDARY_TYPE_None"))
        {
        }    
    }    
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(CurvePrimitive,ParallelLinesFrenetFrame)
    {
    ICurvePrimitivePtr line0 = ICurvePrimitive::CreateLine (DSegment3d::From (0,0,0, 1,0,0));
    ICurvePrimitivePtr line1 = ICurvePrimitive::CreateLine (DSegment3d::From (0,0,1, 1,0,1));
    
    CurveVectorPtr collection = CurveVector::Create (CurveVector::BOUNDARY_TYPE_None);
    collection->push_back (line0);
    collection->push_back (line1);
    
    Transform frame;
    if (Check::True (collection->GetAnyFrenetFrame (frame), "Frenet frame on parallel lines."))
        {
        }    
    }    
        
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(CurvePrimitive,PointsAlongMatthieu)
    {
    bvector<DPoint3d> linePoints;
    linePoints.push_back ( DPoint3d::From (2510509487.2163668, 51854370000.000000, 185551.44200153163));
    linePoints.push_back ( DPoint3d::From (2510981987.2163668, 51855235000.000000, 185551.44200153163));
    linePoints.push_back ( DPoint3d::From (2511441987.2163658, 51855817500.000000, 175551.44200153163));
    linePoints.push_back ( DPoint3d::From (2511626987.2163658, 51856395000.000000, 175551.44200153163));
    linePoints.push_back ( DPoint3d::From (2511519487.2163668, 51856870000.000000, 185551.44200153163));
    linePoints.push_back ( DPoint3d::From (2511754487.2163658, 51857005000.000000, 185551.44200153163));
    linePoints.push_back ( DPoint3d::From (2511826987.2163668, 51857425000.000000, 175551.44200153163));
    linePoints.push_back ( DPoint3d::From (2511336987.2163658, 51857172500.000000, 215551.44200153160));
    linePoints.push_back ( DPoint3d::From (2511171987.2163668, 51856495000.000000, 185551.44200153163));

    ICurvePrimitivePtr linestring = ICurvePrimitive::CreateLineString (linePoints);
    double interval = 1000000;
    double startFraction = 0.0;
    double signedDistance = 0.0;
    bool allowExtension = false;
    CurveLocationDetail location;
    size_t count = 0;
    while (linestring->PointAtSignedDistanceFromFraction (startFraction, signedDistance, allowExtension, location) && count++ < 20)
        {
        printf (" (requestedDistance %#.17g) (fraction %g) (distance %#.17g) (point %#.17g,%#.17g,%#.17g)\n", signedDistance, location.fraction, location.a, location.point.x, location.point.y, location.point.z);
        signedDistance += interval;
        if (location.fraction >= 1.0)
            count += 20;
        }
    }

/// <summary>Find the curve point closest to spacePoint.  Return it as the CurveLocationDetail, and 
///    also compute the distance (along curve) from the path start to the close point.
///</summary>
bool ClosestPointAndDistanceAlong (CurveVectorCR path, DPoint3dCR spacePoint, CurveLocationDetailR location, double &distanceAlong)
    {
    distanceAlong = 0.0;
    if (path.ClosestPointBounded (spacePoint, location))
        {
        double d;
        size_t partialPrimitiveIndex = path.CurveLocationDetailIndex (location);
        for (size_t i = 0; i < partialPrimitiveIndex; i++)
            {
            path[i]->Length (d);
            distanceAlong += d;
            }
        path[partialPrimitiveIndex]->SignedDistanceBetweenFractions (0.0, location.fraction, d);
        distanceAlong += d;
        return true;
        }
    return false;
    }
    
void TestProjectedDistanceAlong (ICurvePrimitivePtr &primitive0)
    {
    // make sure it's a true leaf ..
    DPoint3d xyz;
    if (!primitive0->FractionToPoint (0.4, xyz))
        return;
    // pull the curves up onto a plane not parallel to xy
    Transform lift = Transform::FromRowValues
        (
        1,0,0,0,
        0,1,0,0,
        1,2,0,0
        );
    // collapse back to xy
    Transform drop = Transform::FromRowValues
        (
        1,0,0,0,
        0,1,0,0,
        0,0,0,0
        );
    RotMatrix dropVectors = RotMatrix::From (drop);
            
    ICurvePrimitivePtr primitive1 = primitive0->Clone ();
    primitive1->TransformInPlace(lift);
    ICurvePrimitivePtr primitive2 = primitive0->Clone ();
    primitive2->TransformInPlace (drop);
    double length0, length1, length2, length1xy;

    Check::True (primitive0->Length (length0));
    Check::True (primitive1->Length (length1));
    Check::True (primitive2->Length (length2));
    Check::True (primitive1->Length (&dropVectors, length1xy));
    Check::True (length1 > length0);
    Check::True (length2 < length1);
    Check::Near (length1xy, length2);
    // !!!! Fractions survive the projective transform !!!!
    double f0 = 0.2;
    double f1 = 0.4;
    ICurvePrimitivePtr partial1 = primitive1->CloneBetweenFractions (f0, f1, false);
    ICurvePrimitivePtr partial2 = primitive2->CloneBetweenFractions (f0, f1, false);
    double length2a, length1a, length1axy;
    Check::True (partial1->Length (length1a));
    Check::True (partial2->Length (length2a));
    Check::True (partial1->Length (&dropVectors, length1axy));
    Check::True (length2a < length1a);
    Check::Near (length1axy, length2a);

    double length1xyBetween;
    Check::True (primitive1->SignedDistanceBetweenFractions (&dropVectors, f0, f1, length1xyBetween));
    Check::Near (length2a, length1xyBetween);

    CurveLocationDetail location;
    Check::True (primitive1->PointAtSignedDistanceFromFraction (&dropVectors,
                  f0, length1axy, false, location));
    Check::Near (f1, location.fraction);
    }
    
void TestProjectedDistanceAlong (CurveVectorPtr &source)
    {
    for (ICurvePrimitivePtr & primitive : *source)
      TestProjectedDistanceAlong (primitive);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(ProjectedLength,Test0)
    {
    bvector<IGeometryPtr> candidates;
    SampleGeometryCreator::AddSizeSampler (candidates);
    for (IGeometryPtr &candidate : candidates)
        {
        ICurvePrimitivePtr primitive = candidate->GetAsICurvePrimitive ();
        if (primitive.IsValid ())
            TestProjectedDistanceAlong (primitive);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(ClosestPointAndDistance,Test0)
    {
    double a = 10.0;
    ICurvePrimitivePtr cpA = ICurvePrimitive::CreateLine (DSegment3d::From (0,0,0, a,0,0));

    bvector<DPoint3d> points;
    points.push_back (DPoint3d::From (a,0,0));
    points.push_back (DPoint3d::From (a,a,0));
    points.push_back (DPoint3d::From (2*a,a,0));
    ICurvePrimitivePtr cpB = ICurvePrimitive::CreateLineString (points);

    DPoint3d arcPoint0 = points.back ();
    DPoint3d arcPoint1 = DPoint3d::From (arcPoint0.x + a, arcPoint0.y + 0.25 * a, arcPoint0.z);
    DPoint3d arcPoint2 = DPoint3d::From (arcPoint1.x + a, arcPoint0.x, arcPoint0.z);
    DEllipse3d arc = DEllipse3d::FromPointsOnArc (arcPoint0, arcPoint1, arcPoint2);
    ICurvePrimitivePtr cpC = ICurvePrimitive::CreateArc (arc);
    CurveVectorPtr cv = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
    cv->Add (cpA);
    cv->Add (cpB);
    cv->Add (cpC);

    points.clear ();
    points.push_back (arcPoint2);
    points.push_back (DPoint3d::FromSumOf (arcPoint2, DVec3d::From (1,0,0)));
    points.push_back (DPoint3d::FromSumOf (arcPoint2, DVec3d::From (2,1,0)));
    points.push_back (DPoint3d::FromSumOf (arcPoint2, DVec3d::From (3,1,0)));
    points.push_back (DPoint3d::FromSumOf (arcPoint2, DVec3d::From (4,0,0)));
    points.push_back (DPoint3d::FromSumOf (arcPoint2, DVec3d::From (5,0,0)));
    MSBsplineCurvePtr bcurve = MSBsplineCurve::CreateFromPolesAndOrder (points, nullptr, nullptr, 3, false, true);
    ICurvePrimitivePtr cpD = ICurvePrimitive::CreateBsplineCurve (bcurve);
    cv->Add (cpD);
    double bcurveLength;
    cpD->Length (bcurveLength);
    CurveLocationDetail location;
    double sumdA = 0.0;
    double dA1, dA0;
    for (size_t prim = 0; prim < cv->size (); prim++)
        {
        ICurvePrimitivePtr cp = cv->at(prim);
        for (double f = 0.1; f < 1.0; f += 0.2)
            {
            DPoint3d xyzA;
            cp->FractionToPoint (f, xyzA);
            cp->SignedDistanceBetweenFractions (0.0, f, dA0);
            Check::True (ClosestPointAndDistanceAlong (*cv, xyzA, location, dA1));
            Check::Near (sumdA + dA0, dA1, "distance along");

            CurveKeyPoint_ClosestPointCollector collector (xyzA);  // defaults to endpoints and perpendiculars
            collector.EnableKeyPointType (CurveKeyPointCollector::KeyPointType::EndPoint, false);
            cp->AnnounceKeyPoints (xyzA, collector, false, false);
            CurveKeyPointCollector::KeyPointType selector;
            CurveLocationDetail detailB;
            if (collector.GetResult(detailB, selector))
                {
                Check::Near (xyzA, detailB.point);
                }

            }
        cp->Length (dA1);
        sumdA += dA1; 
        }

    } 
    


    

#if defined (_WIN32) && !defined(BENTLEY_WINRT)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Intersection,AllanB00)
    {
    double ax = 0.0, bx =1.0;
    double ay = 1.0, by = 0.0;


    ax = 47.978845608028678;
    bx = 352.0211543919713;
    ay = 250.0;
    by = 0.0;

    bvector<DPoint3d> polygonPoints;
    polygonPoints.push_back (DPoint3d::From (ax, ay));
    polygonPoints.push_back (DPoint3d::From (ax, by));
    polygonPoints.push_back (DPoint3d::From (bx, by));
    polygonPoints.push_back (DPoint3d::From (bx, ay));
    DPoint3d point0 = polygonPoints[0];
    polygonPoints.push_back (point0);

    CurveVectorPtr polygon = CurveVector::CreateLinear (polygonPoints, CurveVector::BOUNDARY_TYPE_Outer);
    static int s_noisySplitter = 0;
    // make extensions of each edge to confirm "on" behavior . . 
    for (size_t i0 = 0; i0 + 1 < polygonPoints.size (); i0++)
        {
        DPoint3d pointA = DPoint3d::FromInterpolate (polygonPoints[i0], -2.5, polygonPoints[i0+1]);
        DPoint3d pointB = DPoint3d::FromInterpolate (polygonPoints[i0],  302.519872633423, polygonPoints[i0+1]);
        double edgeLength = polygonPoints[i0].Distance (polygonPoints[i0 + 1]);
        double ABLength = pointA.Distance (pointB);
        ICurvePrimitivePtr primitive = ICurvePrimitive::CreateLine (DSegment3d::From (pointA, pointB));
        CurveVectorPtr curves = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
        curves->Add (primitive);
        CurveVectorPtr split0 = CurveVector::Create (CurveVector::BOUNDARY_TYPE_None);
        CurveVectorPtr split1 = CurveVector::Create (CurveVector::BOUNDARY_TYPE_None);
        CurveVectorPtr split2 = CurveVector::Create (CurveVector::BOUNDARY_TYPE_None);
        curves->AppendSplitCurvesByRegion (*polygon, split0.get (), split1.get (), split2.get());
        if (Check::Size (split0->size (), 0, "ON extension has no inside parts")
            && Check::Size (split1->size (), 2, "ON extension has 2 outside parts")
            && Check::Size (split2->size (), 1, "ON extension has 1 ON part")
            )
            {
            Check::Near (edgeLength, split2->Length (), "OUT length");
            Check::Near (ABLength, split1->Length () + split2->Length (), "OUT length");
            }
        }
    }

#endif

// force DSegment4d logic into true 4d case . . .
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Intersection,DSegment4dSpecialCase)
    {
    DSegment4d segmentA = {{{0,0,0,1}, {1,0,0,1}}};
    DSegment4d segmentB = {{{0.5, -1,0,1}, {0.5, 2, 0, 1}}};
    DPoint4d pointA0, pointB0;
    DPoint4d pointA1, pointB1;
    double paramA0, paramB0, paramA1, paramB1;
    Check::True (
          DSegment4d::IntersectXY (pointA0, paramA0, pointB0, paramB0,
                  segmentA, segmentB));
    // Get to nonunit weights ... note that this changes the parameterization on segmentB . ..
    segmentB.point[0].Scale (2.0);
    Check::True (
          DSegment4d::IntersectXY  (pointA1, paramA1, pointB1, paramB1,
                  segmentA, segmentB));
    double d0, d1;

    Check::True (pointA0.RealDistanceXY (d0, pointB0));
    Check::True (pointA1.RealDistanceXY (d1, pointB1));
    Check::Near (0.0, d0);
    Check::Near (0.0, d1);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(CurveVector,FlattenEllipse)
    {
    DEllipse3d ellipse0;
    ellipse0.Init (9534.3811451171714, -23133.955400956540, 2000.0000000000036, 500.00000000000000, 0.0, 0.0, 0.0, 500.00000000000000, 0.0, 0.96249377175206530, 1.2166051100856627);

    ICurvePrimitivePtr curvePrimitiveP = ICurvePrimitive::CreateArc (ellipse0);
    IGeometryPtr geometry = IGeometry::Create(curvePrimitiveP);
    GEOMAPI_PRINTF("\nOriginal Ellipse\n");
    Check::Print(*curvePrimitiveP);

    CurveVectorPtr curveVectorPtr = CurveVector::Create(curvePrimitiveP);


    Utf8String string0, string1;
    BentleyGeometryJson::TryGeometryToJsonString(string0, *geometry);
    CurveVectorPtr cloneCurve = curveVectorPtr->Clone ();

    Transform flatten;
    flatten.InitIdentity ();
    flatten.form3d[1][1] = 0.0;
    flatten.form3d[1][3] = -22723.645777193437;

    curveVectorPtr->TransformInPlace (flatten);
    GEOMAPI_PRINTF("Flattened Ellipse\n");
    DEllipse3d ellipse1;
    curvePrimitiveP->TryGetArc (ellipse1);
    Check::Print(*curvePrimitiveP);

//    BentleyGeometryJson::TryGeometryToJsonString(string1, *geometry);
//    GEOMAPI_PRINTF("Orignial json%s\n", string0);
//    GEOMAPI_PRINTF("Flattened json %s\n", string1);
    }


void AddPointIfDistinctFromBack (bvector<DPoint3d> &points, DPoint3d xyz)
    {
    if (points.size () == 0 || !points.back ().AlmostEqual (xyz))
        points.push_back (xyz);
    }

void CollectPointsFromLinearPrimitives (CurveVectorCR curves, bvector<bvector<DPoint3d>> &paths)
    {
    if (curves.IsUnionRegion () || curves.IsParityRegion ())
        {
        for (auto &prim : curves)
            {
            CollectPointsFromLinearPrimitives (*prim->GetChildCurveVectorP (), paths);
            }
        }
    else if (curves.IsOpenPath () || curves.IsClosedPath ())
        {
        paths.push_back (bvector<DPoint3d>());
        for (auto &prim : curves)
            {
            DSegment3d segment;
            if (prim->TryGetLine (segment))
                {
                AddPointIfDistinctFromBack (paths.back (), segment.point[0]);
                AddPointIfDistinctFromBack (paths.back (), segment.point[1]);
                }
            else
                {
                auto linestringPoints = prim->GetLineStringCP ();
                if (nullptr != linestringPoints)
                    {
                    for (DPoint3d xyz : *linestringPoints)
                        {
                        AddPointIfDistinctFromBack (paths.back (), xyz);
                        }
                    }
                }
            }
        }
    }


bool CheckSame (bvector<DPoint3d> const &xyzA, bvector<DPoint3d> const &xyzB)
    {
    Check::NamedScope scope("Leaf loops");
    if (Check::Size (xyzA.size (), xyzB.size (), "xyz array sizes"))
        {
        for (size_t i = 0; i < xyzA.size (); i++)
            if (!Check::Near (xyzA[i], xyzB[i]))
                return false;
        return true;
        }
    return false;
    }

bool CheckSame
(
bvector<bvector<DPoint3d>> const &xyzA, 
bvector<bvector<DPoint3d>> const &xyzB
)
    {
    Check::NamedScope scope("Parity region");
    if (Check::Size (xyzA.size (), xyzB.size (), "Parity Region sizes"))
        {
        for (size_t i = 0; i < xyzA.size (); i++)
            if (!CheckSame (xyzA[i], xyzB[i]))
                return false;

        return true;
        }
    return false;
    }

bool CheckSame
(
bvector<bvector<bvector<DPoint3d>>> const &xyzA, 
bvector<bvector<bvector<DPoint3d>>> const &xyzB,
char const *name
)
    {
    Check::NamedScope scope(name);
    if (Check::Size (xyzA.size (), xyzB.size (), "Parity Collection sizes"))
        {
        for (size_t i = 0; i < xyzA.size (); i++)
            if (!CheckSame (xyzA[i], xyzB[i]))
                return false;
        return true;
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(CurveVector,RecursiveCollectPoints)
    {
    auto unionRegion = CurveVector::Create (CurveVector::BOUNDARY_TYPE_UnionRegion);
    unionRegion->Add (CurveVector::CreateRectangle (0,0, 1,1, 0));
    unionRegion->Add (CurveVector::CreateRectangle (2,2, 3, 4, 0));
    bvector<bvector<bvector<DPoint3d>>> xyzA, xyzB, xyzC;
    unionRegion->CollectLinearGeometry (xyzA);
    // explode to individual segments ....
    auto unionRegionWithJustLines = unionRegion->CloneWithExplodedLinestrings ();
    unionRegion->CollectLinearGeometry (xyzB);
    CheckSame (xyzA, xyzB, "CollectLinearGeometry");
    Check::SaveTransformed(*unionRegion);
    Check::ClearGeometry("CurveVector.RecursiveCollectPoints");
    }

 // return (if possible) a DEllipse3d which starts at pointA, with initial tangent vector towards pointB, ends on the line containing pointB and pointC
 ValidatedDEllipse3d ArcFromStartShoulderTarget
 (
 DPoint3dCR pointA, // Start point of arc
 DPoint3dCR pointB, // shoulder point
 DPoint3dCR pointC  // target point for outbound tangent
 )
    {
    DVec3d vectorAB = DVec3d::FromStartEnd (pointA, pointB);
    DVec3d vectorBC = DVec3d::FromStartEnd (pointB, pointC);
    double dab = pointA.Distance (pointB);
    auto unitNormal = DVec3d::FromCrossProduct (vectorAB, vectorBC).ValidatedNormalize ();
    if (!unitNormal.IsValid ())
        return ValidatedDEllipse3d ();  // That will be invalid 
    auto unitPerp   = DVec3d::FromCrossProduct (unitNormal, vectorAB).ValidatedNormalize ();
    if (!unitPerp.IsValid ())
        return ValidatedDEllipse3d ();  // That will be invalid 

    double beta = vectorAB.AngleTo (vectorBC);      // positive angle as viewed from cross product upwards.
    double alpha = 0.5 * beta;
    double radius = dab / tan (alpha);
    DPoint3d center = pointA + unitPerp * radius;
    DVec3d vector0 = pointA - center;
    DVec3d vector90 = DVec3d::FromCrossProduct (unitNormal, vector0);   // cross product with unit normal preserves length
    return ValidatedDEllipse3d (DEllipse3d::FromVectors (center, vector0, vector90, 0.0, beta), true);
    }

// Construct two arcs and a line segment such that:
// start at pointA.
// first arc tangent is directionA.
// shoulder point of first arc is distanceA along that tangent.
// end at pointB
// second arc tangent direction at pointB is directionB.
// shoulder point of second arc is distanceB along the tangent.
// NOTE NOTE NOTE both tangent directions are "inbound" to the arcs -- expect directionA to point "forward towards" pointB and directionB "backwards towards" pointA
//
CurveVectorPtr ConstructDoubleFillet (
DPoint3dCR pointA,
DVec3dCR directionA,
double distanceA,
DPoint3dCR pointB,
DVec3dCR directionB,
double distanceB
)
    {
    auto unitA = directionA.ValidatedNormalize ();
    auto unitB = directionB.ValidatedNormalize ();
    DPoint3d shoulderA = pointA + distanceA * unitA;
    DPoint3d shoulderB = pointB + distanceB * unitB;
    auto arcA = ArcFromStartShoulderTarget (pointA, shoulderA, shoulderB);
    auto arcB = ArcFromStartShoulderTarget (pointB, shoulderB, shoulderA);
    DPoint3d pointA2 = pointA;
    DPoint3d pointB2 = pointB;
    if (arcA.IsValid ())
        arcA.Value ().FractionParameterToPoint (pointA2, 1.0);
    if (arcB.IsValid ())
        arcB.Value ().FractionParameterToPoint (pointB2, 1.0);

    auto curves = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
    if (arcA.IsValid ())
        curves->push_back (ICurvePrimitive::CreateArc (arcA.Value ()));
    if (!pointA2.AlmostEqual (pointB2))
        curves->push_back (ICurvePrimitive::CreateLine (DSegment3d::From (pointA2, pointB2)));
    if (arcB.IsValid ())
        curves->push_back (ICurvePrimitive::CreateArc (DEllipse3d::FromReversed (arcB.Value())));
    return curves;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(CurveVector,InflexFilletConstruction)
    {
    DPoint3d pointA = DPoint3d::From (1,0,0);
    DPoint3d pointB = DPoint3d::From (10,1,0);
    DVec3d   vectorA = DVec3d::From (10,0,0);
    DVec3d   vectorB = DVec3d::From (-5,0.3,0);

    double distanceA = 2.0;
    double distanceB = 1.0;
    auto curves = ConstructDoubleFillet (pointA, vectorA, distanceA, pointB, vectorB, distanceB);
    Check::Print (*curves, "double fillet");
    double diagonal = pointA.Distance (pointB);
    double length = curves->Length ();
    Check::True (length > diagonal && length < diagonal + distanceA + distanceB);
    Check::Size (3, curves->size (), "Expect arc+line+arc");
    for (size_t i = 0; i < 2; i++)
        {
        DRay3d rayA = curves->at (i)->FractionToPointAndUnitTangent (1.0);
        DRay3d rayB = curves->at(i+1)->FractionToPointAndUnitTangent (0.0);
        Check::Near (rayA.origin, rayB.origin);
        Check::Near (0.0, rayA.direction.AngleTo (rayB.direction));
        }
    DPoint3d pointA1, pointB1;
    DVec3d unitA1, unitB1;
    curves->GetStartEnd (pointA1, pointB1, unitA1, unitB1);
    unitB1.Negate ();
    Check::Near (pointA, pointA1);
    Check::Near (pointB, pointB1);
    Check::Near (0.0, vectorA.AngleTo (unitA1));
    Check::Near (0.0, vectorB.AngleTo (unitB1));
    }

void CheckCentroidAndArea (CurveVectorCR region, DPoint3dCR centroid0, DVec3dCR normal0, double area0)
    {
    DPoint3d centroid;
    double area;
    DVec3d normal;
    region.CentroidNormalArea (centroid, normal, area);
    Check::Near (centroid0, centroid, "Polygon Centroid");
    Check::Near (area0, area, "Polygon area");
    Check::Near (normal0, normal, "Polygon normal");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(CurveVector,RegularPolygonConstructor)
    {
    DPoint3d center = DPoint3d::From (1,2,3);
    DVec3d normal = DVec3d::From (0,0,1);
    double a = 5.0;
    auto square = CurveVector::CreateRegularPolygonXY (center, a, 4, false);
    auto diamond = CurveVector::CreateRegularPolygonXY (center, a, 4, true);
    CheckCentroidAndArea (*square, center, normal, 4.0 * a * a);
    CheckCentroidAndArea (*diamond, center, normal, 2.0 * a * a);
    if (s_noisy)
        {
        Check::Print (*square, "Square from RegularPolygonXY");
        Check::Print (*diamond, "Diamond from RegularPolygonXY");
        }
    }
	
	
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DSpiral2D,SpiralPrimitive)
    {
    DPoint3d startPoint {10,0,0};
    Transform placement = Transform::From (startPoint);
    double rA = 1000;
    double rB =  500;
    //double dAB = 100;

    double bearing0 = Angle::DegreesToRadians (45);
    double bearing1 = Angle::DegreesToRadians (65);
    ICurvePrimitivePtr spiral = ICurvePrimitive::CreateSpiralBearingRadiusBearingRadius
          (
          DSpiral2dBase::TransitionType_Clothoid,
          bearing0, rA, bearing1, rB,
          placement, 0,1
          );
    DPoint3d xyz;
    static int n = 8;
    for (int i = 0; i <= n; i++)
        {
        double f = i / (double)n;
        spiral->FractionToPoint (f, xyz);
        Check::PrintIndent (4);
        Check::Print (f, "f");
        Check::Print (xyz, "xyz");
        }
    DPoint3d xyzA, xyzB;
    spiral->FractionToPoint (0.0, xyzA);
    spiral->FractionToPoint (1.0, xyzB);
    Check::Print (xyzA, "startPoint");
    Check::Print (xyzB, "endPoint");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(CurvePrimitive,IntersectLineRange)
    {
    DRange3d range;
    range.Init ();
    range.Extend (DPoint3d::From (1,2,3));
    range.Extend (DPoint3d::From (-2,5,9));

    auto worldToLocal = Transform::FromIdentity ();
    auto localToWorld = Transform::FromIdentity ();
    LocalRange localRange (localToWorld, worldToLocal, range);

    bvector<double> params {-0.1, 0.4, 1.2};
    DPoint3d interiorPoint = range.LocalToGlobal (0.4, 0.2, 0.6);
    for (double ux : params)
        {
        for (double uy : params)
            {
            for (double uz : params)
                {
                auto segment = DSegment3d::From (interiorPoint, range.LocalToGlobal (ux, uy, uz));
                auto cp = ICurvePrimitive::CreateLine (segment);
                bvector<PartialCurveDetail> intervals;
                cp->AppendCurveRangeIntersections (localRange, intervals);
                if (Check::Size (1, intervals.size ()))
                    {
                    auto detail = intervals.front();
                    double f = detail.ChildFractionToParentFraction (0.5);
                    DPoint3d xyzf;
                    cp->FractionToPoint (f, xyzf);
                    Check::True (range.IsContained (xyzf));
                    double df = detail.fraction1 - detail.fraction0;
                    if (DoubleOps::IsIn01 (ux) && DoubleOps::IsIn01 (uy) && DoubleOps::IsIn01(uz))
                        {
                        Check::Near (1.0, df, "Interior segment has full parameter range");
                        }
                    else
                        {
                        // we know the segment started inside and left .. find an "out" fraction between detail.fraction1 and the end . . ..
                        double f1 = DoubleOps::Interpolate (detail.fraction1, 0.5, 1.0);
                        cp->FractionToPoint (f1, xyzf);
                        Check::False(range.IsContained (xyzf));
                        Check::True (df < 1.0, "crossing segment");
                        Check::Near (detail.fraction0, 0.0, "clippee starts inside");
                        }
                    }
                }
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(CurvePrimitive,IntersectLineStringRange)
    {
    DRange3d range;
    range.Init ();
    range.Extend (DPoint3d::From (1,2,3));
    range.Extend (DPoint3d::From (-2,5,9));
    bvector<double> params {-0.1, 0.4, 1.2};

    auto worldToLocal = Transform::FromIdentity ();
    auto localToWorld = Transform::FromIdentity ();
    LocalRange localRange (localToWorld, worldToLocal, range);
    bvector<DPoint3d> points;
    points.push_back (range.LocalToGlobal (0.5, 0.5, 0.5)); // IN
    points.push_back (range.LocalToGlobal (1.5, 0.5, 0.5)); // IN,OUT
    points.push_back (range.LocalToGlobal (1.5, 0.6, 0.5)); // OUT
    points.push_back (range.LocalToGlobal (0.5, 0.6, 0.5)); // OUT,IN
    points.push_back (range.LocalToGlobal (0.5, 0.7, 0.5)); // IN
    points.push_back (range.LocalToGlobal (0.5, 0.7, 1.5)); // IN, OUT
    points.push_back (range.LocalToGlobal (0.5, 0.8, 0.9)); // OUT, IN
    auto cp = ICurvePrimitive::CreateLineString (points);
    bvector<PartialCurveDetail> intervals;
    cp->AppendCurveRangeIntersections (localRange, intervals);

    MSBsplineCurve bCurve;
    if (cp->GetMSBsplineCurve (bCurve, 0.0, 1.0))
        {
        bvector<PartialCurveDetail> intervalsB;
        auto cpB = ICurvePrimitive::CreateBsplineCurve (bCurve);
        cpB->ICurvePrimitive::AppendCurveRangeIntersections (localRange, intervalsB);
        Check::Size (intervals.size (), intervalsB.size (), "bcurve intersection");
        bCurve.ReleaseMem ();
        }
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(CPLineString,TwoSidedDerivative)
    {
    bvector<DPoint3d> points {
        DPoint3d::From(0,0,0),
        DPoint3d::From (1,2,4),
        DPoint3d::From (1,2,0),
        DPoint3d::From (1,5,1),
        };
    auto cp = ICurvePrimitive::CreateLineString (points);
    // We know there is a cusp at each uniform fraction
    bvector<double> fractions;
    double df = 1.0 / (points.size () - 1);
    fractions.push_back (0.0);
    for (size_t i = 0; i + 1< points.size (); i++)
        {
        double f0 = i * df;
        double f1 = (i+1) * df;
        fractions.push_back (DoubleOps::Interpolate (f0, 0.5, f1));
        fractions.push_back (f1);
        }
    double epsilon = Angle::SmallAngle () * 1000.0;
    DPoint3d point, point0, point1, point2;
    DVec3d   tangentA0, tangentA, tangentB, tangentB1;
    // linestring derivatives are constant within each segment.
    // at midpoint, expect the same on both sides, whether from epsilon shift or two sided
    // at vertex, expect 2 values, but simple evaluator will match them at epsilong shifts.
    for (double f: fractions)
        {
        Check::StartScope ("Fraction", f);
        cp->FractionToPointWithTwoSidedDerivative (f, point, tangentA, tangentB);
        cp->FractionToPoint (f, point2);
        cp->FractionToPoint (f-epsilon, point0, tangentA0);
        cp->FractionToPoint (f+epsilon, point1, tangentB1);
        Check::Near (tangentA, tangentA0);
        Check::Near (tangentB, tangentB1);
        Check::Near (point, point2);
        Check::EndScope ();
        }
    }	

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(CurvePrimitive,ZeroDerivativeFrenetFrames)
    {
    Transform frame;
    auto cp = ICurvePrimitive::CreateArc
            (
            DEllipse3d::From (0,0,0,   0,0,0,   0,0,0, 0, Angle::TwoPi ())
            );
    // zero radius arc should fail ..
    Check::False (cp->FractionToFrenetFrame (0.0, frame), "Expected Frenet Frame Failure on zero-radius circle.");
    
    // line should succeed with arbitrary spin ...
    DPoint3d pointA0 = DPoint3d::From (1,2,3);
    DPoint3d pointA1 = DPoint3d::From (3,2,9);
    cp = ICurvePrimitive::CreateLine (DSegment3d::From(pointA0, pointA1));

    Check::True (cp->FractionToFrenetFrame(0.0, frame), "Expected default spin Frenet Frame");
    ValidatedDRay3d unitTangent = cp->FractionToPointAndUnitTangent (0.0);
    if (Check::True (unitTangent.IsValid (), "Well defined line tangent"))
        {
        DVec3d columnX;
        frame.GetMatrixColumn(columnX, 0);
        Check::Near (unitTangent.Value ().origin, pointA0, "frame at start");
        Check::Near (unitTangent.Value().direction, columnX, "X axis along tangnet");
        }
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (CurvePrimitive, CloneTransformed)
    {
    bvector<IGeometryPtr> curves;
    SampleGeometryCreator::AddSizeSampler (curves);
    // scale and translation ..
    double s = 2.0;
    Transform transform2 = Transform::FromRowValues
            (
            s,0,0,3,
            0,s,0,2,
            0,0,s,1
            );
    DMatrix4d matrix2 = DMatrix4d::From (transform2);
    for (auto &gp : curves)
        {
        auto cp = gp->GetAsICurvePrimitive ();
        auto curveA = cp->Clone (transform2);
        auto curveB = cp->Clone (matrix2);
        double lA, lB;
        curveA->Length (lA);
        curveB->Length (lB);
        Check::Near (lA, lB, "simple 3d,4d transforms match length");
        }

    auto cv0 = SampleGeometryCreator::CircleInRectangle ();
    auto cv1 = cv0->Clone (transform2);
    auto cv2 = cv0->Clone (matrix2);
    Check::Near (cv1->Length (), cv2->Length (), "length after 3d, 4d deep clone with transform");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(CPLineString, ReverseCurve) 
    {
    bvector<DPoint3d> points {
        DPoint3d::From(0,0,0),
        DPoint3d::From (1,2,4),
        DPoint3d::From (1,2,0),
        DPoint3d::From (1,5,1),
        };
    bvector<DPoint3d> pointsSet2 {
        DPoint3d::From (1,5,1),
        DPoint3d::From (1,2,0),
        DPoint3d::From (1,2,4),
        DPoint3d::From (0,0,0),
        };
    auto cp = ICurvePrimitive::CreateLineString (points);
    auto cpReq = ICurvePrimitive::CreateLineString (pointsSet2);
    Check::SaveTransformed(*cp);
    cp->ReverseCurvesInPlace();
    Check::Shift(10, 0, 0);
    Check::SaveTransformed(*cp);
    Check::Shift(20, 0, 0);
    Check::SaveTransformed(*cpReq);
    Check::ClearGeometry ("CPLineString.ReverseCurve");
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(CPLineString, CheckShapes)
    {
    auto cShapeLine = ICurvePrimitive::CreateLine(DSegment3d::From(DPoint3d::From(0,0,0), DPoint3d::From(5,5,5)));
    DEllipse3d ellipse = DEllipse3d::FromXYMajorMinor (0.0, 0.0, 0.0, 4.0, 3.0, 2, 3, 3);
    auto cShapeArc = ICurvePrimitive::CreateArc(ellipse);
    auto cShapeRectangle = ICurvePrimitive::CreateRectangle(4, 0, 8, 8, 3);
    DPoint3d points[] = { DPoint3d::From(0,0,1), DPoint3d::From(0,3,3), DPoint3d::From(3,0,4), DPoint3d::From(5,3,0) };
    auto cShapePointString = ICurvePrimitive::CreatePointString(points, 4);

    Check::SaveTransformed(*cShapePointString);
    auto cpShapeLine2 = cShapeLine->Clone();
    Check::Shift(2, 0, 0);
    Check::SaveTransformed(*cShapeLine);
    Check::Shift(5, 0, 0);
    Check::SaveTransformed(*cShapeLine);
    Check::Shift(20, 0, 0);
    Check::SaveTransformed(*cShapeArc);
    Check::Shift(20, 0, 0);
    Check::SaveTransformed(*cShapeRectangle);
    Check::ClearGeometry("CPLineString.CheckShapes");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(CPLineString, SameStructure)
    {
    DPoint3d points[5] = { DPoint3d::From(1,0,0),
                           DPoint3d::From(2,2,0),
                           DPoint3d::From(2,2,4),
                           DPoint3d::From(2,2,6),
                           DPoint3d::From(6,6,6) };
    auto cShapePointString = ICurvePrimitive::CreatePointString(points, 5);
    auto cShapePointStringCloned = cShapePointString->Clone();
    cShapePointStringCloned->TransformInPlace(Transform::From(DPoint3d::From(10, 0, 0)));
    Check::True(cShapePointString->IsSameStructure(*cShapePointStringCloned));
    Check::False(cShapePointString->IsSameStructureAndGeometry(*cShapePointStringCloned));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(CPLineString, LineLineStringClone)
    {
    DPoint3d points[5] = {  DPoint3d::From(1,0,0),
                            DPoint3d::From(2,2,0),
                            DPoint3d::From(2,2,4),
                            DPoint3d::From(2,2,6),
                            DPoint3d::From(6,6,6) };
    auto cShapeLineString = ICurvePrimitive::CreateLineString(points, 5);
    bvector<DPoint3d> *pointsCopy = cShapeLineString->GetLineStringP();
    auto cShapeLineStringCopy = ICurvePrimitive::CreateLineString(*pointsCopy);
    Check::True(cShapeLineStringCopy->IsSameStructureAndGeometry(*cShapeLineString->Clone()));
    }

void swapCurvePrimitives(CurveVector::BoundaryType cvBT)
    {
    CurveVectorPtr curve = CurveVector::Create(cvBT);
    DEllipse3d ellip = DEllipse3d::FromPointsOnArc (
        DPoint3d::From (1,0,0),
        DPoint3d::From (2,1,0),
        DPoint3d::From (1,2,0));
    DSegment3d seg = DSegment3d::From(DPoint3d::From(0, -1, 0), DPoint3d::From(1, 0, 0));
    
    curve->push_back(ICurvePrimitive::CreateLine(seg));
    curve->push_back(ICurvePrimitive::CreateArc( ellip));

    Check::SaveTransformed(*curve);
    Check::Shift(10, 0, 0);
    Check::True( curve->SwapAt(0, 1));
    Check::SaveTransformed(*curve);
    Check::Shift(10, 0, 0);

    CurveVectorPtr curve2 = CurveVector::Create(cvBT);
    curve2->push_back(ICurvePrimitive::CreateArc( ellip));
    curve2->push_back(ICurvePrimitive::CreateLine( seg));
    Check::SaveTransformed(*curve2);
    Check::Shift(10, 0, 0);

    Check::True(curve->IsSameStructureAndGeometry(*curve2));
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(CurveVector, SwapIndices)
    {
    swapCurvePrimitives(CurveVector::BOUNDARY_TYPE_Inner);
    swapCurvePrimitives(CurveVector::BOUNDARY_TYPE_None);
    swapCurvePrimitives(CurveVector::BOUNDARY_TYPE_Open);
    swapCurvePrimitives(CurveVector::BOUNDARY_TYPE_Outer);
    Check::ClearGeometry("CurveVector.SwapIndices");
    }



void checkChildBoundaryTypes(CurveVectorPtr parent,
                             bvector<CurveVector::BoundaryType> boundaryTypeChild,
                             bvector<CurveVectorPtr> curveChild) 
    {
    for (size_t i = 1; i <= curveChild.size(); i++) 
        {
        
        parent->push_back(ICurvePrimitive::CreateChildCurveVector(curveChild[i-1]));
        Check::True(parent->SetChildBoundaryType(i, boundaryTypeChild[i - 1]));
        CurveVector::BoundaryType cvB;
        Check::True(parent->GetChildBoundaryType(i, cvB));
        Check::True(cvB == boundaryTypeChild[i-1]);
        }


    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(CurveVector, ChildVectorType)
    {
    ////
    bvector<CurveVector::BoundaryType> boundaryTypeChild = { CurveVector::BOUNDARY_TYPE_Inner,
                                                            CurveVector::BOUNDARY_TYPE_Outer,
                                                            CurveVector::BOUNDARY_TYPE_None };

    
    DRange2d outer = DRange2d::From(0, 0, 20, 10);
    bvector<CurveVectorPtr> curveChild = { CurveVector::CreateRectangle(outer.low.x + 2, outer.low.y + 2, outer.high.x - 2, outer.high.y - 2, 0),
                                           CurveVector::CreateRectangle(outer.low.x + 4, outer.low.y + 4, outer.high.x - 4, outer.high.y - 4, 0),
                                           CurveVector::CreateRectangle(outer.low.x + 4, outer.low.y + 4, outer.high.x - 4, outer.high.y - 4, 0) };

    
    CurveVectorPtr parent = CurveVector::CreateRectangle(outer.low.x, outer.low.y, outer.high.x, outer.high.y, 0,
                                                              CurveVector::BOUNDARY_TYPE_Outer);
    checkChildBoundaryTypes(parent, boundaryTypeChild, curveChild);
    
    
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(CurveVector, ReverseLeafCurves) 
    {
    bvector<CurveVector::BoundaryType> cvType = { CurveVector::BOUNDARY_TYPE_Inner,
                                                  CurveVector::BOUNDARY_TYPE_Outer,
                                                  CurveVector::BOUNDARY_TYPE_Open };
    for (int i = 0; i < cvType.size(); i++)
        {

        CurveVectorPtr curve = CurveVector::Create(cvType[i]);
        DEllipse3d ellip = DEllipse3d::FromPointsOnArc(DPoint3d::From(1, 0, 0),
                                                       DPoint3d::From(2, 1, 0),
                                                       DPoint3d::From(1, 2, 0));
        curve->push_back(ICurvePrimitive::CreateArc(ellip));
        DSegment3d seg = DSegment3d::From(DPoint3d::From(0, -1, 0), DPoint3d::From(1, 0, 0));
        curve->push_back(ICurvePrimitive::CreateLine(seg));
        // unused - DPoint3dDVec3dDVec3dCR identity = DPoint3dDVec3dDVec3d(DPoint3d::From(0, 0, 0), DVec3d::From(1, 0, 0), DVec3d::From(0, 1, 0));

        bvector<DPoint3d> pointsLineString = { DPoint3d::From(0,0,0),DPoint3d::From(4,5,2),DPoint3d::From(9,8,10) };
        curve->push_back(ICurvePrimitive::CreateLineString(pointsLineString));
        Check::True(curve->ReverseCurvesInPlace());
        ICurvePrimitivePtr curvePrim = curve->GetCyclic(0);
        Check::True(ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString == curvePrim->GetCurvePrimitiveType());
        curvePrim = curve->GetCyclic(1);
        Check::True(ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line == curvePrim->GetCurvePrimitiveType());
        curvePrim = curve->GetCyclic(2);
        Check::True(ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc == curvePrim->GetCurvePrimitiveType());
        }

    bvector<CurveVector::BoundaryType> cvTypeN = { CurveVector::BOUNDARY_TYPE_UnionRegion,
                                                  CurveVector::BOUNDARY_TYPE_ParityRegion,
                                                  CurveVector::BOUNDARY_TYPE_None };
    
    for (int i = 0; i < cvTypeN.size(); i++)
        {

        CurveVectorPtr curveN = CurveVector::Create(cvTypeN[i]);
        DEllipse3d ellipN = DEllipse3d::FromPointsOnArc(DPoint3d::From(1, 0, 0),
                                                       DPoint3d::From(2, 1, 0),
                                                       DPoint3d::From(1, 2, 0));
        curveN->push_back(ICurvePrimitive::CreateArc(ellipN));
        DSegment3d segN = DSegment3d::From(DPoint3d::From(0, -1, 0), DPoint3d::From(1, 0, 0));
        curveN->push_back(ICurvePrimitive::CreateLine(segN));
        // unused - DPoint3dDVec3dDVec3dCR identity = DPoint3dDVec3dDVec3d(DPoint3d::From(0, 0, 0), DVec3d::From(1, 0, 0), DVec3d::From(0, 1, 0));

        bvector<DPoint3d> pointsLineStringN = { DPoint3d::From(0,0,0),DPoint3d::From(4,5,2),DPoint3d::From(9,8,10) };
        curveN->push_back(ICurvePrimitive::CreateLineString(pointsLineStringN));
        Check::True(curveN->ReverseCurvesInPlace());
        ICurvePrimitivePtr curvePrim = curveN->GetCyclic(0);
        //unchanged order
        Check::True(ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc == curvePrim->GetCurvePrimitiveType());
        curvePrim = curveN->GetCyclic(1);
        Check::True(ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line == curvePrim->GetCurvePrimitiveType());
        curvePrim = curveN->GetCyclic(2);
        Check::True(ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString == curvePrim->GetCurvePrimitiveType());
        }
    
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(CurveVector, LengthOfPrimitives) 
    {
    CurveVectorPtr curve = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Inner);
    bvector<DPoint3d> pointsLineStringN = { DPoint3d::From(0,0,0), DPoint3d::From(4,5,2), DPoint3d::From(9,8,10) };
    ICurvePrimitivePtr linePrim = ICurvePrimitive::CreateLineString(pointsLineStringN);
    curve->push_back(linePrim);
    DSegment3d segN = DSegment3d::From(DPoint3d::From(0, -1, 0), DPoint3d::From(1, 0, 0));
    ICurvePrimitivePtr segPrim = ICurvePrimitive::CreateLine(segN);
    curve->push_back(segPrim);
    DEllipse3d ellipN = DEllipse3d::FromPointsOnArc(DPoint3d::From(1, 0, 0),
                                                    DPoint3d::From(2, 1, 0),
                                                    DPoint3d::From(1, 2, 0));
    ICurvePrimitivePtr ellipsePrim = ICurvePrimitive::CreateArc(ellipN);
    curve->push_back(ellipsePrim);
    double flength =curve->FastLength();
    double prim1, prim2, prim3;
    Check::True(linePrim->FastLength(prim1)); Check::True(segPrim->FastLength(prim2)); Check::True(ellipsePrim->FastLength(prim3));
    double flengthExp = prim1 + prim2 + prim3;
    Check::Near(flength, flengthExp);

    // length post rotation
    double prior = curve->Length();
    
    //double posterior;
    //linePrim->Length(posterior);
    RotMatrix rot[] = { RotMatrix::FromAxisAndRotationAngle(2, Angle::FromDegrees(45).Cos()) };
    double postRot = curve->Length(rot);
    Check::Near(prior, postRot);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(CurveVector, CloneBetweenFractions) 
    {
    DPoint3d points[5] = { DPoint3d::From(1,0,0),
                           DPoint3d::From(2,2,0),
                           DPoint3d::From(2,2,4),
                           DPoint3d::From(2,2,6),
                           DPoint3d::From(6,6,6) };
    auto cShapeLineString = ICurvePrimitive::CreateLineString(points, 5);
    CurveVectorPtr cVec = CurveVector::Create(CurveVector::BOUNDARY_TYPE_None);
    cVec->push_back(cShapeLineString);
    

    CurveVectorPtr curve = CurveVector::Create(CurveVector::BOUNDARY_TYPE_None);
    bvector<DPoint3d> pointsLineStringN = { DPoint3d::From(0,0,0), DPoint3d::From(4,5,2), DPoint3d::From(9,8,10) };
    ICurvePrimitivePtr linePrim = ICurvePrimitive::CreateLineString(pointsLineStringN);
    curve->push_back(linePrim);
    DSegment3d segN = DSegment3d::From(DPoint3d::From(0, -1, 0), DPoint3d::From(1, 0, 0));
    ICurvePrimitivePtr segPrim = ICurvePrimitive::CreateLine(segN);
    curve->push_back(segPrim);
    DEllipse3d ellipN = DEllipse3d::FromPointsOnArc(DPoint3d::From(10, 0, 0),
                                                    DPoint3d::From(12, 1, 0),
                                                    DPoint3d::From(11, 2, 0));
    ICurvePrimitivePtr ellipsePrim = ICurvePrimitive::CreateArc(ellipN);
    curve->push_back(ellipsePrim);
    Check::SaveTransformed(*curve);
    Check::Shift(10, 0, 0);
    Check::SaveTransformed(*curve->CloneBetweenDirectedFractions(1, 0.5, 2, 0.7, false));
    Check::Shift(0, 10, 0);
    //checking cyclic indexing
    Check::SaveTransformed(*curve->CloneBetweenCyclicIndexedFractions(1, 0.5, 2, 0.7));
    Check::ClearGeometry("CurveVector.CloneBetweenFractions");
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(CurveVector, ChangeAreaByOffset)
    {
    CurveOffsetOptions offset(5);
    CurveVectorPtr curve = CurveVector::CreateRectangle(2, 2, 6, 6, CurveVector::BOUNDARY_TYPE_None);
    Check::SaveTransformed(*curve);
    Check::Shift(15, 0, 0);

    auto offsetCurve = curve->AreaOffset(offset);
    Check::SaveTransformed(*offsetCurve);
    Check::Shift(15, 0, 0);
    offset.SetOffsetDistance(15);
    offsetCurve = curve->AreaOffset(offset);
    Check::SaveTransformed(*offsetCurve);
    Check::Shift(15, 0, 0);
    offset.SetOffsetDistance(10);
    offsetCurve = curve->AreaOffset(offset);
    Check::SaveTransformed(*offsetCurve);

    Check::ClearGeometry("CurveVector.ChangeAreaByOffset");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(CurveVector, CloneOffsetCurvesXY)
    {

    CurveVectorPtr curve = CurveVector::Create(CurveVector::BOUNDARY_TYPE_None);
    bvector<DPoint3d> pointsLineStringN = { DPoint3d::From(0,0,0), DPoint3d::From(4,5,0), DPoint3d::From(9,8,0) };
    ICurvePrimitivePtr linePrim = ICurvePrimitive::CreateLineString(pointsLineStringN);
    curve->push_back(linePrim);
    DSegment3d segN = DSegment3d::From(DPoint3d::From(0, -1, 0), DPoint3d::From(1, 0, 0));
    ICurvePrimitivePtr segPrim = ICurvePrimitive::CreateLine(segN);
    curve->push_back(segPrim);
    DEllipse3d ellipN = DEllipse3d::FromPointsOnArc(DPoint3d::From(0, 0, 0),
                                                    DPoint3d::From(2, 1, 0),
                                                    DPoint3d::From(1, 2, 0));
    ICurvePrimitivePtr ellipsePrim = ICurvePrimitive::CreateArc(ellipN);
    curve->push_back(ellipsePrim);

    Check::SaveTransformed(*curve);
    Check::Shift(20, 0, 0);
    CurveOffsetOptions offset(5);
    auto offsetCurve = curve->CloneOffsetCurvesXY(offset);
    Check::SaveTransformed(*offsetCurve);
    Check::Shift(20, 0, 0);
    auto offsetCurve2 = curve->CloneOffsetCurvesXY(offset);//offset only curves
    Check::SaveTransformed(*offsetCurve2);
    Check::ClearGeometry("CurveVector.CloneOffsetCurvesXY");
    }

#if defined(WIP)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(CurveVector, GeometricConstructions) 
    {// Arc
    DEllipse3d ellipN = DEllipse3d::FromPointsOnArc(DPoint3d::From(0, 0, 0),
                                                    DPoint3d::From(2, 1, 0),
                                                    DPoint3d::From(1, 2, 0));
    ICurvePrimitivePtr ellipsePrim = ICurvePrimitive::CreateArc(ellipN);
    Check::SaveTransformed(*ellipsePrim);
    Check::Shift(10, 0, 0);

    //Partial Curve
    double a = 0.10;
    double b = 0.20;
    auto child = ICurvePrimitive::CreatePartialCurve ((ICurvePrimitive*)&ellipsePrim, a, b);
    Check::SaveTransformed(*child);
    Check::Shift(10, 0, 0);

    //Bspline Curve
    bvector<DPoint3d> poles;
    poles.push_back (DPoint3d::From (1,2,0));
    poles.push_back (DPoint3d::From (1,1,0));
    poles.push_back (DPoint3d::From (0,0,0));
    poles.push_back (DPoint3d::From (0,-1,0));
    MSBsplineCurvePtr curve = MSBsplineCurve::CreateFromPolesAndOrder (poles, NULL, NULL, 3, false, false);
    ICurvePrimitivePtr bsplineCurve = ICurvePrimitive::CreateBsplineCurve(*curve);

    Check::SaveTransformed(*bsplineCurve);
    Check::Shift(10, 0, 0);

    //BsplineCurveFromSource
    ICurvePrimitivePtr bsplineCurveFromSource = ICurvePrimitive::CreateBsplineCurveSwapFromSource(*curve);
    Check::SaveTransformed(*bsplineCurveFromSource);
    Check::Shift(10, 0, 0);

    DPoint3d akimaPoints [] =
        {
                {5,-5,0},
                {5,-4,0},
                {5,-3,0},
                {5.9,0.1,0},    // Hm, where does this really go?
                {7,2,0},
                {8,3,0},
                {9,4,0},
        };

    //AkimaCurve
    ICurvePrimitivePtr akima0 = ICurvePrimitive::CreateAkimaCurve (akimaPoints, 7);
    Check::SaveTransformed(*akima0);
    Check::Shift(10, 0, 0);

    //point string 
    bvector<DPoint3d> points;
    points.push_back (DPoint3d::From (0,0,0));
    points.push_back (DPoint3d::From (10,0,0));
    points.push_back (DPoint3d::From (5,1,0));

    ICurvePrimitivePtr cp = ICurvePrimitive::CreatePointString (points);
    Check::SaveTransformed(*cp);
    Check::Shift(10, 0, 0);



    Check::ClearGeometry("CurveVector.GeometricConstructions");
    }


#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     09/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(CPLineString,EndFuzz)
    {
    // "getStartEnd" did not ping the bits on the endpoint ... verify it with various length linestrings
    DPoint3d xyzBase = DPoint3d::From (-9.4347826089999991,0.0,-1.8773913040000001);
    bvector<DPoint3d> points {DPoint3d::From (-0.56521739100000001,0.0,-1.8773913040000001)};

    for (int i = 0; i < 14; i++)
        {
        // force another point into the front ...
        points.insert (points.begin (), DPoint3d::From (xyzBase.x + i, xyzBase.y, xyzBase.z));
        auto linestring = ICurvePrimitive::CreateLineString (points);
        DPoint3d xyz0, xyz1;
        linestring->GetStartEnd (xyz0, xyz1);
        auto d0 = xyz0.Distance (points.front ());
        auto d1 = xyz1.Distance (points.back ());
        Check::ExactDouble (0, d0, "LineString start");
        Check::ExactDouble (0, d1, " LineString end");
        double localFraction = 0.234248977923;
        for (size_t k = 0; k + 1 < points.size (); k++)
            {
            auto xyz = DPoint3d::FromInterpolate (points[k], localFraction, points[k+1]);
            DPoint3d xyzF;
            double globalFraction = (k + localFraction) / (points.size () - 1.0);
            linestring->FractionToPoint (globalFraction, xyzF);
            Check::Near (xyz, xyzF, "Linestring FractionToPoint on specific segment");
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Nate.Rex		01/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(CurveVector, VariableNumberOfArgs)
	{
	DEllipse3d arc1 = DEllipse3d::From(
		0, 0, 0,
		0.5, 1, 0,
		-0.2, 0.2, 0,
		Angle::DegreesToRadians(-120),
		Angle::DegreesToRadians(240));
	auto cpArc1 = ICurvePrimitive::CreateArc(arc1);
	DEllipse3d arc2 = DEllipse3d::From(
		5, 6, 0,
		0.5, 1, 0,
		-0.2, 0.2, 0,
		Angle::DegreesToRadians(-120),
		Angle::DegreesToRadians(240));
	auto cpArc2 = ICurvePrimitive::CreateArc(arc2);
	DEllipse3d arc3 = DEllipse3d::From(
		8, -10, 0,
		0.5, 1, 0,
		-0.2, 0.2, 0,
		Angle::DegreesToRadians(-120),
		Angle::DegreesToRadians(240));
	auto cpArc3 = ICurvePrimitive::CreateArc(arc3);

	CurveVectorPtr curveVector0 = CurveVector::Create(CurveVector::BoundaryType::BOUNDARY_TYPE_Open, {cpArc1, cpArc2, cpArc3});
	CurveVectorPtr curveVector2 = CurveVector::Create(CurveVector::BoundaryType::BOUNDARY_TYPE_Open, cpArc1);

	// re-extract them
	bvector<ICurvePrimitivePtr> primitives;
	for (ICurvePrimitivePtr& curvePrimitive : *curveVector0) {
		primitives.push_back(curvePrimitive);
	}
	}

void ShowRay (DRay3dCR ray, double f0, double f1, int capType, double capRadius = 0.0)
    {
    DVec3d xVec, yVec, zVec;
    ray.direction.GetNormalizedTriad (xVec, yVec, zVec);
    double a = capRadius;
    if (a <= 0.0)
        a = ray.direction.Magnitude ();
    DPoint3d xyz0 = ray.FractionParameterToPoint (f0);
    DPoint3d xyz1 = ray.FractionParameterToPoint (f1);

    Check::SaveTransformed (DSegment3d::From (xyz0, xyz1));
    if (capType == 1)
        {
        Check::SaveTransformed (DSegment3d::From (xyz0 + a * xVec, xyz0 - a * xVec));
        Check::SaveTransformed (DSegment3d::From (xyz1 + a * xVec, xyz1 - a * xVec));
        }
    else if (capType == 2)
        {
        for (auto xyz : {xyz0, xyz1})
            {
            bvector<DPoint3d> rectangle {
                xyz + a * xVec + a * yVec,
                xyz - a * xVec + a * yVec,
                xyz - a * xVec - a * yVec,
                xyz + a * xVec - a * yVec,
                };
            auto cap = CurveVector::CreateLinear (rectangle, CurveVector::BOUNDARY_TYPE_Outer);
            Check::SaveTransformed (*cap);
            }
        }
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                  Earlin.Lutz   05/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(CurveVector, ProjectedParameterRange)
	{
    bvector<CurveVectorPtr> contours;
    SampleGeometryCreator::GetContours (contours);
    contours.push_back (SampleGeometryCreator::CircleInRectangle ());
    for (auto ray : {
            DRay3d::FromOriginAndTarget (DPoint3d::From (1,1,2),  DPoint3d::From (2,1,2)),
            DRay3d::FromOriginAndTarget (DPoint3d::From (1,1,2),  DPoint3d::From (3,1,-1))
            })
        {
        SaveAndRestoreCheckTransform shifter (0, 50,0);
        for (auto &cv : contours)
            {
            SaveAndRestoreCheckTransform shifter (50,0,0);
            auto rayRange = cv->ProjectedParameterRange (ray);
            Check::SaveTransformed (*cv);
            ShowRay (ray, 0, 1, 0);
            ShowRay (ray, rayRange.low, rayRange.high, 2, 4.0);
            }
        }
    Check::ClearGeometry ("CurveVector.ProjectedParameterRange");
    }
void ShowCentroidArea (DPoint3dCR centroid, double area, DVec3dCR normal)
    {
    DVec3d xVec, yVec, zVec;
    normal.GetNormalizedTriad (xVec, yVec, zVec);
    double a = 0.5 * sqrt (area);
    bvector<DPoint3d> rectangle {
        centroid + a * xVec + a * yVec,
        centroid - a * xVec + a * yVec,
        centroid - a * xVec - a * yVec,
        centroid + a * xVec - a * yVec,
        centroid + a * xVec + a * yVec
        };
    auto cap = CurveVector::CreateLinear (rectangle, CurveVector::BOUNDARY_TYPE_Open);
    Check::SaveTransformedMarker (centroid, 0.05 * a);
    Check::SaveTransformed (*cap);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                  Earlin.Lutz   05/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(CurveVector, Centroid)
	{
    bvector<CurveVectorPtr> regions;
    SampleGeometryCreator::CreateXYRegions (regions);
    auto transform = YawPitchRollAngles::FromDegrees (10,30,45).ToTransform (DPoint3d::From (0.1, 8.3, 0.4));
    for (auto &cv : regions)
        {
        SaveAndRestoreCheckTransform shifter (10,0,0);
        Check::SaveTransformed (*cv);
        DPoint3d centroid;
        double area;
        cv->CentroidAreaXY (centroid, area);
        ShowCentroidArea (centroid, area, DVec3d::From (0,0,1));
        auto cv1 = cv->Clone (transform);
        Check::SaveTransformed (*cv1);
        DVec3d normal;
        cv1->CentroidNormalArea (centroid, normal, area);
        ShowCentroidArea (centroid, area, normal);
        }
    Check::ClearGeometry ("CurveVector.Centroid");
    }
