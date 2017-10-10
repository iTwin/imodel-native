//
//
#include "testHarness.h"

USING_NAMESPACE_BENTLEY_GEOMETRY_INTERNAL

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(PathSearch,DetailConstructors)
{
bvector<PathLocationDetail> data;
for (int i = 0; i < 3; i++)
    {
    CurveLocationDetail curveRef;
    data.push_back (PathLocationDetail (curveRef, i, i + 0.0));
    data.push_back (PathLocationDetail (curveRef, i, i + 0.2));
    data.push_back (PathLocationDetail (curveRef, i, i + 0.5));
    }

for (size_t i = 0; i < data.size (); i++)
  for (size_t j = i + 1; j < data.size (); j++)
    Check::True (PathLocationDetail::IsLessThan_ByPathDistance (data[i], data[j]));
}

void ExerciseSearcher (CurveVectorWithDistanceIndex &searcher, int numTest = 5, bool isInViewPlane = true)
    {
    PathLocationDetail location0 = searcher.AtStart ();
    PathLocationDetail location1 = searcher.AtEnd ();
    if (numTest < 2)
        numTest = 2;
    static double s_circleFraction = 0.25;
    double pathDistance = location1.DistanceFromPathStart ();
    double circleRadius = s_circleFraction * pathDistance / (double)numTest;
    for (int i = 0; i <= numTest; i++)
        {
        double f = i / (double)numTest;
        double targetDistance = DoubleOps::Interpolate (location0.DistanceFromPathStart (), f, location1.DistanceFromPathStart ());
        PathLocationDetail locationA;
        Check::True (searcher.SearchByDistanceFromPathStart (targetDistance, locationA));
        PathLocationDetail locationB = searcher.SearchClosestPointBounded (locationA.Point (), true);
        Check::Near (targetDistance, locationB.DistanceFromPathStart ());

        // If we are clearly away from the end (by more than circleRadius)
        // compute intersection (in respective directions) with circle . . 
        if (locationA.DistanceToPoint (location0) > circleRadius)
            {
            PathLocationDetail circleIntersection;
            if (Check::True (searcher.SearchFirstIntersectionWithCircleXY (locationA, -circleRadius, circleIntersection)))
                {
                Check::True (PathLocationDetail::IsLessThan_ByPathDistance (circleIntersection, locationA));
                Check::Near (circleRadius, searcher.DistanceBetweenPointsXY (circleIntersection.Point (), locationA.Point ()));
                CurveVectorPtr extract = searcher.CloneBetweenDistances(locationA.DistanceFromPathStart(), circleIntersection.DistanceFromPathStart());
                if (Check::True (extract.IsValid ()))
                    {
                    }
                }
            }
        if (locationA.DistanceToPoint (location1) > circleRadius)
            {
            PathLocationDetail circleIntersection;
            if (Check::True (searcher.SearchFirstIntersectionWithCircleXY (locationA, circleRadius, circleIntersection)))
                {
                Check::True (PathLocationDetail::IsLessThan_ByPathDistance (locationA, circleIntersection));
                Check::Near (circleRadius, searcher.DistanceBetweenPointsXY (circleIntersection.Point (), locationA.Point ()));

                CurveVectorPtr extract = searcher.CloneBetweenDistances(locationA.DistanceFromPathStart (), circleIntersection.DistanceFromPathStart ());
                if (Check::True(extract.IsValid()))
                    {
                    }
                }

            }
        }

    for (double insetFraction = 0.01; insetFraction < 0.90; insetFraction *= 2.0)
        {
        double a = pathDistance * insetFraction;
        double b = pathDistance * (1.0 - insetFraction);
        CurveVectorPtr extract = searcher.CloneBetweenDistances (a, b);
        double c = extract->Length ();
        Check::Near (c, fabs (b-a));
        }

    double maxEdgeLength = 0.1 * pathDistance;
    IFacetOptionsPtr options = IFacetOptions::CreateForCurves ();
    options->SetMaxEdgeLength (maxEdgeLength);
    bvector<PathLocationDetail> strokes;
    searcher.Stroke (strokes, *options);

    }

void PathTestLines (DVec3dCR upVector)
    {
    RotMatrix worldToView, viewToWorld;
    DVec3d xVec, yVec, zVec;
    upVector.GetNormalizedTriad (xVec, yVec, zVec);
    viewToWorld = RotMatrix::FromColumnVectors (xVec, yVec, zVec);
    worldToView.TransposeOf (viewToWorld);

    DPoint3d xyzA = DPoint3d::From (10,0,0);
    ICurvePrimitivePtr line = ICurvePrimitive::CreateLine (DSegment3d::From (DPoint3d::From (0,0,0), xyzA));
    CurveVectorPtr path = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
    path->push_back (line);
    auto searcher = CurveVectorWithDistanceIndex::Create (worldToView);
    searcher->SetPath (path);
    ExerciseSearcher (*searcher, 4);
    bvector<DPoint3d> lsAxyz;
    lsAxyz.push_back (xyzA);
    lsAxyz.push_back (DPoint3d::FromSumOf (xyzA, DVec3d::From (3,5,0)));
    lsAxyz.push_back (DPoint3d::FromSumOf (xyzA, DVec3d::From (7,2,0)));
    ICurvePrimitivePtr lsA = ICurvePrimitive::CreateLineString (lsAxyz);
    path->push_back (lsA);
    searcher->SetPath (path);
    ExerciseSearcher (*searcher, 4);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(PathSearch,OneLine)
    {
    PathTestLines(DVec3d::From (0,0,1));
    PathTestLines (DVec3d::From (0,1,3));
    PathTestLines (DVec3d::From (1,0,3));

    }


void TestSearcherStrokes (CurveVectorPtr &cv, size_t maxPrint = 20)
    {
    auto searcher = CurveVectorWithDistanceIndex::Create ();
    searcher->SetPath (cv);

    double maxEdgeLength = 0.1 * cv->Length ();
    IFacetOptionsPtr options = IFacetOptions::CreateForCurves ();
    options->SetMaxEdgeLength (maxEdgeLength);


    CurveVectorPtr elevation = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
    double dx = cv->Length ();
    auto elevationLine = ICurvePrimitive::CreateLine (DSegment3d::From (0,0,0,  dx, 0.0, 1.0));

    elevation->push_back (elevationLine);
    auto elevationC = CurveVectorWithXIndex::Create (*elevation);


    RotMatrix viewToWorld = RotMatrix::FromIdentity ();
    CurveVectorWithDistanceIndexPtr elevationB = CurveVectorWithDistanceIndex::Create (viewToWorld);
    elevationB->SetPath (elevation);
    bvector<DPoint3d> spacePointsB, spacePointsC;
    bvector<double>distancesB, distancesC;
    bvector<PathLocationDetailPair> locationPairsB, locationPairsC;
    CurveVectorWithDistanceIndex::StrokeHorizontalAndVerticalCurves
        (*options, *options, *searcher, *elevationB, locationPairsB);

    CurveVectorWithDistanceIndex::StrokeHorizontalAndVerticalCurves
        (*options, *options, *searcher, *elevationC, locationPairsC);

    PathLocationDetailPair::Merge (locationPairsB, &spacePointsB, &distancesB);
    PathLocationDetailPair::Merge (locationPairsC, &spacePointsC, &distancesC);

    if (spacePointsB.size () <= maxPrint)
        {
        Check::Print (cv, "xy Curve");
        Check::Print (elevation, "elevation Curve");
        Check::Print (spacePointsB, "SpacePointsB");
        Check::Print (distancesB, "distancesB");
        Check::Print (spacePointsC, "SpacePointsC");
        Check::Print (distancesC, "distancesC");
        }
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(PathSearchStrokes,Bsplines)
    {
    bvector<IGeometryPtr> curves;
    SampleGeometryCreator::AddSizeSampler (curves);
    Transform flatten = Transform::FromScaleFactors (1,1,0);

    for (IGeometryPtr &g : curves)
        {
        auto cp = g->GetAsICurvePrimitive ();
        if (cp.IsValid () && cp->GetCurvePrimitiveType () == ICurvePrimitive::CURVE_PRIMITIVE_TYPE_BsplineCurve)
            {
            cp->TransformInPlace (flatten);
            CurveVectorPtr cv = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
            cv->push_back (cp);
            TestSearcherStrokes (cv);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(PathSearchStrokes,SpiralA)
    {

    bvector<IGeometryPtr> curves;
    SampleGeometryCreator::AddSpirals (curves);

    for (IGeometryPtr &g : curves)
        {
        auto cp = g->GetAsICurvePrimitive ();
        if (cp.IsValid () && cp->GetCurvePrimitiveType () == ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Spiral)
            {
            CurveVectorPtr cv = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
            cv->push_back (cp);
            TestSearcherStrokes (cv);
            double length;
            cp->Length (length);
            for (double f = 0.01; f < 0.095; f += 0.02)
                {
                CurveOffsetOptions options (f * length);
                auto cv1 = cv->CloneOffsetCurvesXY (options);
                if (Check::True (cv1.IsValid ()))
                    {
                    TestSearcherStrokes (cv1);
                    }
                }
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(PathSearchStrokes,LineStrings)
    {
    bvector<IGeometryPtr> curves;
    SampleGeometryCreator::AddSizeSampler (curves);
    Transform flatten = Transform::FromScaleFactors (1,1,0);
#define MaxLSCount 15
    for (IGeometryPtr &g : curves)
        {
        auto cp = g->GetAsICurvePrimitive ();
        if (cp.IsValid () && cp->GetCurvePrimitiveType () == ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString
            && cp->GetLineStringCP ()->size () < MaxLSCount
            )
            {
            cp->TransformInPlace (flatten);
            CurveVectorPtr cv = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
            cv->push_back (cp);
            TestSearcherStrokes (cv);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(PathSearchStrokes,Lines)
    {
    bvector<IGeometryPtr> curves;
    SampleGeometryCreator::AddSizeSampler (curves);
    Transform flatten = Transform::FromScaleFactors (1,1,0);

    for (IGeometryPtr &g : curves)
        {
        auto cp = g->GetAsICurvePrimitive ();
        if (cp.IsValid () && cp->GetCurvePrimitiveType () == ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line)
            {
            cp->TransformInPlace (flatten);
            CurveVectorPtr cv = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
            cv->push_back (cp);
            TestSearcherStrokes (cv);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(PathSearchStrokes,Arc)
    {
    bvector<IGeometryPtr> curves;
    SampleGeometryCreator::AddSizeSampler (curves);
    Transform flatten = Transform::FromScaleFactors (1,1,0);

    for (IGeometryPtr &g : curves)
        {
        auto cp = g->GetAsICurvePrimitive ();
        if (cp.IsValid () && cp->GetCurvePrimitiveType () == ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc)
            {
            cp->TransformInPlace (flatten);
            CurveVectorPtr cv = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
            cv->push_back (cp);
            TestSearcherStrokes (cv);
            }
        }
    }





    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(PathSearch,ExtendedPath)
    {
    DPoint3d xyzA = DPoint3d::From (10,0,0);
    ICurvePrimitivePtr line = ICurvePrimitive::CreateLine (DSegment3d::From (DPoint3d::From (0,0,0), xyzA));
    CurveVectorPtr path = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
    path->push_back (line);
    RotMatrix worldToView, viewToWorld;
    DVec3d upVector = DVec3d::From (1,1,4);
    DVec3d xVec, yVec, zVec;
    upVector.GetNormalizedTriad (xVec, yVec, zVec);
    viewToWorld = RotMatrix::FromColumnVectors (xVec, yVec, zVec);
    worldToView.TransposeOf (viewToWorld);

    auto searcher = CurveVectorWithDistanceIndex::Create (worldToView);

    PathLocationDetail boundedStart, boundedEnd;
    searcher->SetExtendedPath (path,
              2.0,
              boundedStart, boundedEnd,
              true, 
              5.0);
    ExerciseSearcher (*searcher, 4, false);
    bvector<DPoint3d> lsAxyz;
    lsAxyz.push_back (xyzA);
    lsAxyz.push_back (DPoint3d::FromSumOf (xyzA, DVec3d::From (3,5,0)));
    lsAxyz.push_back (DPoint3d::FromSumOf (xyzA, DVec3d::From (7,2,0)));
    ICurvePrimitivePtr lsA = ICurvePrimitive::CreateLineString (lsAxyz);
    path->push_back (lsA);
    searcher->SetExtendedPath (path,
              2.0,
              boundedStart, boundedEnd,
              true, 
              5.0);
    ExerciseSearcher (*searcher, 4, false);

    }


void ExerciseSearcherProjections (CurveVectorCR curves, DVec3dCR upVector, int numTest = 5)
    {
    RotMatrix planeToWorld = RotMatrix::From1Vector (upVector, 2, true);
    RotMatrix worldToPlane = RotMatrix::FromTransposeOf (planeToWorld);
    RotMatrix flattenZ = RotMatrix::FromIdentity ();
    flattenZ.form3d[2][2] = 0.0;
    RotMatrix skewAbovePlane = RotMatrix::FromRowValues
          (
          1,0,0,
          0,1,0,
          2,3,0
          );
    Transform flattenToPlane = Transform::FromProduct (Transform::From(flattenZ), worldToPlane);
    CurveVectorPtr cv1 = curves.Clone ();
    cv1->TransformInPlace (flattenToPlane);
    CurveVectorPtr cv2 = cv1->Clone ();
    // both are sitting in xy plane.  Push cv2 away and rotate both back into space
    cv2->TransformInPlace (Transform::From (skewAbovePlane));
    // and then back to the space plane . . 
    cv1->TransformInPlace (Transform::From (planeToWorld));
    cv2->TransformInPlace (Transform::From (planeToWorld));
    // tell the searcher the off-plane path and transform to on-plane ...
    auto searcher = CurveVectorWithDistanceIndex::Create (worldToPlane);
    searcher->SetPath (cv2);
    Check::True (searcher->TotalPathLength () > searcher->TotalPathLengthXY ());
    PathLocationDetail location0 = searcher->AtStart ();
    PathLocationDetail location1 = searcher->AtEnd ();
    if (numTest < 2)
        numTest = 2;
    for (int i = 0; i <= numTest; i++)
        {
        double f = i / (double)numTest;
        double targetDistance = DoubleOps::Interpolate (location0.DistanceFromPathStart (), f, location1.DistanceFromPathStart ());
        PathLocationDetail locationA;
        Check::True (searcher->SearchByDistanceFromPathStart (targetDistance, locationA));
        /*PathLocationDetail locationB = */searcher->SearchClosestPointBounded (locationA.Point (), true);
        double projectedDistance;
        Check::True (searcher->DistanceXYFromPathStart (locationA, projectedDistance));
        PathLocationDetail locationC;
        Check::True (searcher->SearchByDistanceFromPathStartXY (projectedDistance, locationC));
        Check::Near (locationC.DistanceFromPathStart (), targetDistance);
        Check::Near (locationA.Point (), locationC.Point ());
        }
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(PathSearch,Projection)
    {
    DPoint3d xyzA = DPoint3d::From (3,4,0);
    ICurvePrimitivePtr line = ICurvePrimitive::CreateLine (DSegment3d::From (DPoint3d::From (0,0,0), xyzA));
    CurveVectorPtr path = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
    path->push_back (line);

    ExerciseSearcherProjections (*path, DVec3d::From (0,0,1), 4);
    ExerciseSearcherProjections (*path, DVec3d::From (0.1, 0.1, 1.0), 4);

    bvector<DPoint3d> lsAxyz;
    lsAxyz.push_back (xyzA);
    lsAxyz.push_back (DPoint3d::FromSumOf (xyzA, DVec3d::From (3,5,0)));
    lsAxyz.push_back (DPoint3d::FromSumOf (xyzA, DVec3d::From (7,2,0)));
    ICurvePrimitivePtr lsA = ICurvePrimitive::CreateLineString (lsAxyz);
    path->push_back (lsA);

    ExerciseSearcherProjections (*path, DVec3d::From (0.1, 0.1, 1.0), 4);
    }

void TestTrimA (char const *name, bvector<DPoint2d> const &points, double d0, double d1, int  noisy = 0)
    {
    Check::PrintIndent (0);
    Check::Print ("TrimTest ");
    Check::Print (name);
    Check::PrintIndent (0);

    if (DoubleOps::AlmostEqual (d0, d1))
        return;
    CurveVectorPtr curveVector2 = CurveVector::CreateLinear(&points[0], points.size());
    double totalLength = curveVector2->Length ();
    DSegment1d distanceInterval = DSegment1d (0.0, totalLength);

    DRange2d range = DRange2d::From (points);
    double rangeFraction, lengthFraction;
    double largestCoordinate = range.LargestCoordinate ();
    DoubleOps::SafeDivide (rangeFraction, range.low.Distance (range.high), largestCoordinate, 0.0);
    DoubleOps::SafeDivide (lengthFraction, totalLength, largestCoordinate, 0.0);
    if (noisy > 0)
        {
        Check::PrintIndent (2);
        Check::Print (d0, "d0");
        Check::PrintIndent (2);
        Check::Print (d1, "d1");
        Check::Print (rangeFraction, "RangeFraction");
        Check::PrintIndent (2);
        Check::Print (lengthFraction, "lengthFraction");
        Check::PrintIndent (0);
        }

    CurveVectorWithDistanceIndexPtr m_horizIdx2 = CurveVectorWithDistanceIndex::Create();
    m_horizIdx2->SetPath(const_cast<CurveVectorPtr&>(curveVector2));
    CurveVectorPtr align2 = m_horizIdx2->CloneBetweenDistancesXY(d0, d1);
    if (noisy > 1)
        Check::Print (align2);
    double a0 = fabs (d1 - d0);
    double a1 = align2->Length ();
    double errorRatio1 = fabs (a1-a0) / totalLength;
    double errorRatio2 = fabs (a1-a0) / largestCoordinate;
    if (noisy > 0)
        {
        Check::PrintIndent (2);
        Check::Print (errorRatio1, "LengthChange/TotalLength");
        Check::PrintIndent (2);
        Check::Print (errorRatio2, "LengthChange/largestCoordinate");
        Check::PrintIndent (2);
        }
    if (distanceInterval.IsInteriorOrEnd (d0) && distanceInterval.IsInteriorOrEnd (d1))
        Check::Near (largestCoordinate + a0, largestCoordinate + a1);
    bvector<DPoint3d> const *pointsA;
    if (Check::Size (1, align2->size ())
        && nullptr != (pointsA = align2->at (0)->GetLineStringCP ()))
        {
        DRange1d distanceRange;
        for (size_t i = 0; i + 1 < pointsA->size (); i++)
            distanceRange.Extend (pointsA->at(i).Distance (pointsA->at (i+1)));
        Check::True (distanceRange.Low () > 1.0e-14);
        }
    }

void TestTrim (bvector<DPoint2d> const &points, double d0, double d1)
    {
    TestTrimA ("primary d0..d1", points, d0, d1, 2);
    TestTrimA ("reversed", points, d1, d0, 1);
    TestTrimA ("   0..d0", points, 0.0, d0, 1);
    double b0 = DoubleOps::Interpolate (d0, 0.2, d1);
    double b1 = DoubleOps::Interpolate (d0, 0.7982, d1);
    TestTrimA (" b0,b1", points, b0, b1, 1);
    double scale = 1000.03298787907;
    DVec2d shift = DVec2d::From (scale *points[0].x, scale *points[0].y);
    bvector<DPoint2d> points1 = points;
    for (size_t i = 0; i < points.size (); i++)
        points1[i].Add (shift);
    TestTrimA ("shifted d0,d1", points1, d0, d1, 1);
    TestTrimA ("shifted b0,b1", points1, b0, b1, 1);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(ConceptStationTests, RoadSegmentsProblem3)
    {
    DPoint2d point0 = DPoint2d::From(303528.29747376870, 259323.12922432116);
    DPoint2d point1 = DPoint2d::From(303527.41697069019, 259372.87701679702);
    DPoint2d point2 = DPoint2d::From(303531.87319636822, 259380.42889508168);
    DPoint2d point3 = DPoint2d::From(303545.69591278891, 259385.75776086311);
    DPoint2d point4 = DPoint2d::From(303709.33260204399, 259404.39056316979);
    DPoint2d point5 = DPoint2d::From(303873.85362371965, 259455.44791809033);
    DPoint2d point6 = DPoint2d::From(303935.83300417732, 259474.09972122061);
    DPoint2d point7 = DPoint2d::From(303993.91233487125, 259477.05849573138);
    DPoint2d point8 = DPoint2d::From(304082.51812271879, 259473.19500739808);
    DPoint2d point9 = DPoint2d::From(304109.26248031587, 259460.30841941974);
    DPoint2d point10 = DPoint2d::From(304125.75770825957, 259432.31755137938);
    DPoint2d point11 = DPoint2d::From(304129.32026534475, 259400.34055616363);
    DPoint2d point12 = DPoint2d::From(304092.30647121940, 259281.30076383083);
    DPoint2d point13 = DPoint2d::From(304085.61246443214, 259248.42512119684);
    DPoint2d point14 = DPoint2d::From(304090.95735110756, 259170.24341271288);

    bvector<DPoint2d> points = {point0, point1, point2, point3, point4, point5, point6, point7, point8, point9, point10, point11, point12, point13, point14};
    TestTrim (points, 0.0, 410.29412811985270);
    TestTrim (points, 100.0, 200.3);

    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(ConceptStationTests, RoadSegmentsProblem4)
    {
    DPoint2d point0 = DPoint2d::From(303004.79963605531, 259710.44863340567);
    DPoint2d point1 = DPoint2d::From(302975.82035603252, 259682.02234753623);
    DPoint2d point2 = DPoint2d::From(302961.76992265380, 259663.15535786684);
    DPoint2d point3 = DPoint2d::From(302954.40929436497, 259642.27633013419);
    DPoint2d point4 = DPoint2d::From(302949.93801882816, 259610.30080329842);
    DPoint2d point5 = DPoint2d::From(302951.94575736724, 259596.52776858519);
    DPoint2d point6 = DPoint2d::From(302957.50958057150, 259578.53321135259);
    DPoint2d point7 = DPoint2d::From(302970.66026691615, 259555.66080700263);
    DPoint2d point8 = DPoint2d::From(302978.67461254779, 259532.33434759389);
    DPoint2d point9 = DPoint2d::From(302981.57037852664, 259513.24090922374);
    DPoint2d point10 = DPoint2d::From(303001.15854833584, 259408.39827216166);
    DPoint2d point11 = DPoint2d::From(303006.27792445914, 259373.53254644974);
    DPoint2d point12 = DPoint2d::From(302997.57420248131, 259333.56112755026);
    DPoint2d point13 = DPoint2d::From(302998.68074188224, 259304.90514643985);
    DPoint2d point14 = DPoint2d::From(303003.57571436162, 259269.81734618015);
    DPoint2d point15 = DPoint2d::From(302993.62723591237, 259205.21123938292);
    DPoint2d point16 = DPoint2d::From(303013.69102579774, 258970.09629393957);
    bvector<DPoint2d> points = {point0, point1, point2, point3, point4, point5, point6, point7, point8, point9, point10, point11, point12, point13, point14, point15, point16};


    TestTrim (points, 202.34482874266885, 328.31341798208109);
    TestTrim (points, 100.0, 200.3);

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(ConceptStationTests, RoadSegmentsProblem5)
    {

    DPoint2d point0 = DPoint2d::From(303528.29747376870, 259323.12922432116);
    DPoint2d point1 = DPoint2d::From(303709.33260204399, 259404.39056316979);
    DPoint2d point2 = DPoint2d::From(303873.85362371965, 259455.44791809033);
    DPoint2d point3 = DPoint2d::From(304090.95735110756, 259170.24341271288);
    
    bvector<DPoint2d> points = {point0, point1, point2, point3};
    TestTrim (points, 370.69818483297342, 729.13316963958653);
    TestTrim (points, 100.0, 200.3);

    }

// Confirm that a linestring with duplicate point produces a null tangent.
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(ConceptStationTests, NullTangent)
    {

    DPoint3d point0 = DPoint3d::From(303528.29747376870, 259323.12922432116);
    DPoint3d point1 = DPoint3d::From(303709.33260204399, 259404.39056316979);
    DPoint3d point2 = DPoint3d::From(303873.85362371965, 259455.44791809033);
    DPoint3d point3 = DPoint3d::From(304090.95735110756, 259170.24341271288);
    
    bvector<DPoint3d> points = {point0, point1, point2, point2, point3};
    Check::Print (points, "Points with duplicate");
    bvector<double> fractions = {0,0.2, 0.25, 0.45, 0.5, 0.55, 0.70, 0.75, 0.8};
    for (double f = 0.0; f <= 1.0; f += 0.125)
        {
        DPoint3d xyz;
        DVec3d tangent;
        bsiDPoint3d_interpolatePolyline (&xyz, &tangent, &points[0], (int)points.size (), f);
        if (f >= 0.5 && f < 0.75)
            Check::Near (0.0, tangent.Magnitude ());
        else
            {
            Check::True (tangent.Magnitude () > 1.0);
            }
#ifdef PrintTangents
        Check::PrintIndent (2);
        Check::Print (f, "fraction");
        Check::Print (tangent, "tangent");
#endif
        }

    }






/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
    TEST(ConceptStationTests, RoadSegmentsProblem6)
    {
    bvector<DPoint2d> points;
    points.push_back(DPoint2d::From(303632.41104382818, 254801.40901129815));
    points.push_back(DPoint2d::From(303641.45059259539, 254952.02620112739));
    points.push_back(DPoint2d::From(303648.76754832710, 255046.14365975681));
    points.push_back(DPoint2d::From(303667.06681812560, 255119.47866138379));
    points.push_back(DPoint2d::From(303673.31674197566, 255147.47781380668));
    points.push_back(DPoint2d::From(303675.56111937447, 255194.13695398654));
    points.push_back(DPoint2d::From(303671.10230621992, 255238.58690486132));
    points.push_back(DPoint2d::From(303663.07460445119, 255274.58518177766));
    points.push_back(DPoint2d::From(303640.33887742087, 255329.25629718631));
    points.push_back(DPoint2d::From(303600.20460706548, 255422.59302212420));
    points.push_back(DPoint2d::From(303580.57747425360, 255462.60289520054));
    points.push_back(DPoint2d::From(303530.02486224892, 255554.87556640181));
    points.push_back(DPoint2d::From(303514.36774227768, 255609.71246770798));
    points.push_back(DPoint2d::From(303516.60310497723, 255647.48619121808));
    points.push_back(DPoint2d::From(303529.10024879861, 255687.03517874825));
    points.push_back(DPoint2d::From(303563.01945343293, 255782.13654661953));
    points.push_back(DPoint2d::From(303573.28756074357, 255821.68600690766));
    points.push_back(DPoint2d::From(303575.52254816215, 255859.45976638264));
    points.push_back(DPoint2d::From(303574.01061533217, 255916.81541507068));
    points.push_back(DPoint2d::From(303572.86221715249, 255960.34307080443));
    points.push_back(DPoint2d::From(303566.64260236605, 256106.99805238034));
    points.push_back(DPoint2d::From(303562.41080002597, 256158.76749086409));
    points.push_back(DPoint2d::From(303554.61730079487, 256193.65524345200));
    points.push_back(DPoint2d::From(303546.59435261245, 256247.42458089005));
    points.push_back(DPoint2d::From(303541.69505416631, 256284.75534266356));
    points.push_back(DPoint2d::From(303541.70969240891, 256368.30058949563));
    points.push_back(DPoint2d::From(303554.67085867410, 256502.50166603114));
    points.push_back(DPoint2d::From(303561.59112941462, 256564.93174590386));
    points.push_back(DPoint2d::From(303564.78379956138, 256612.44605707872));
    points.push_back(DPoint2d::From(303568.07827088068, 256672.02233664255));
    points.push_back(DPoint2d::From(303567.41833521437, 256702.45506975456));
    points.push_back(DPoint2d::From(303564.48749463120, 256726.64617563999));
    points.push_back(DPoint2d::From(303558.05939485552, 256768.66429779335));
    points.push_back(DPoint2d::From(303544.69119217608, 256823.76744846842));
    points.push_back(DPoint2d::From(303529.76231597218, 256891.31052325509));
    points.push_back(DPoint2d::From(303521.74883809482, 256992.85032814118));
    points.push_back(DPoint2d::From(303514.63286121673, 257090.82471971895));
    points.push_back(DPoint2d::From(303512.86992146971, 257227.90525108698));
    points.push_back(DPoint2d::From(303498.16254321695, 257502.03513956006));
    points.push_back(DPoint2d::From(303482.66006266786, 257876.81541153538));
    points.push_back(DPoint2d::From(303470.88375077816, 258074.51911455981));
    points.push_back(DPoint2d::From(303469.54858981172, 258091.84601723711));
    points.push_back(DPoint2d::From(303462.87437111774, 258186.47746392462));
    points.push_back(DPoint2d::From(303455.75963468250, 258280.44260945480));
    points.push_back(DPoint2d::From(303448.46705959248, 258356.01489267714));
    points.push_back(DPoint2d::From(303446.09125658835, 258408.49514942180));
    points.push_back(DPoint2d::From(303443.49538233050, 258483.88886608119));
    points.push_back(DPoint2d::From(303442.38824235008, 258519.43096629373));
    points.push_back(DPoint2d::From(303441.27804454078, 258538.75708071917));
    points.push_back(DPoint2d::From(303440.83237264655, 258555.62845817889));
    points.push_back(DPoint2d::From(303437.27823935222, 258618.04958995833));
    points.push_back(DPoint2d::From(303431.43183964008, 258733.67296183232));
    points.push_back(DPoint2d::From(303419.38087045029, 258914.79465707607));
    points.push_back(DPoint2d::From(303417.21070787212, 258954.01336869254));
    points.push_back(DPoint2d::From(303413.64802309481, 259013.10251436412));
    points.push_back(DPoint2d::From(303408.27638450201, 259130.23645023472));
    points.push_back(DPoint2d::From(303400.27773544221, 259286.15608946077));
    points.push_back(DPoint2d::From(303399.89283914701, 259315.60045350651));
    points.push_back(DPoint2d::From(303386.73317121906, 259555.33359788946));
    points.push_back(DPoint2d::From(303385.28063023364, 259565.03017958003));
    points.push_back(DPoint2d::From(303379.43727436237, 259603.88315923169));
    points.push_back(DPoint2d::From(303375.15440941666, 259632.40644833009));
    points.push_back(DPoint2d::From(303355.45647608390, 259711.48019726510));
    points.push_back(DPoint2d::From(303353.90027581615, 259742.56864548783));
    points.push_back(DPoint2d::From(303357.24787773757, 259775.43320874992));
    points.push_back(DPoint2d::From(303369.51796051045, 259833.17539126269));
    points.push_back(DPoint2d::From(303372.64963326958, 259867.37283664878));
    points.push_back(DPoint2d::From(303370.97876668652, 259907.12467883929));
    points.push_back(DPoint2d::From(303364.70969098841, 260017.95035912684));
    points.push_back(DPoint2d::From(303363.35981948854, 260041.73045883450));
    points.push_back(DPoint2d::From(303354.54105521098, 260200.71614044748));
    points.push_back(DPoint2d::From(303349.43229188997, 260293.53741226374));
    points.push_back(DPoint2d::From(303348.98919146054, 260321.51575004245));
    points.push_back(DPoint2d::From(303351.44551460748, 260345.27288652706));
    points.push_back(DPoint2d::From(303363.72396034177, 260407.90216465137));
    points.push_back(DPoint2d::From(303379.06346427585, 260486.48036215670));
    points.push_back(DPoint2d::From(303391.16678948264, 260548.45443559560));
    points.push_back(DPoint2d::From(303399.41590314673, 260577.09749955154));
    points.push_back(DPoint2d::From(303408.78533089202, 260598.19877626310));
    points.push_back(DPoint2d::From(303421.04668242211, 260619.95481684647));
    points.push_back(DPoint2d::From(303464.07090501382, 260684.78873756557));
    points.push_back(DPoint2d::From(303506.29622640554, 260747.47943829681));
    points.push_back(DPoint2d::From(303559.09064310585, 260857.60613303940));
    points.push_back(DPoint2d::From(303600.47423025611, 260916.57674569200));
    points.push_back(DPoint2d::From(303633.91062224883, 260959.39945347543));
    points.push_back(DPoint2d::From(303670.38018550206, 261006.09815288178));
    points.push_back(DPoint2d::From(303708.17028305092, 261051.55286802101));
    points.push_back(DPoint2d::From(303712.90377035917, 261087.37192295614));
    points.push_back(DPoint2d::From(303682.45346592093, 261217.04975917126));
    points.push_back(DPoint2d::From(303651.17546998151, 261359.94512402295));
    points.push_back(DPoint2d::From(303659.02979514922, 261374.14961427572));

    TestTrim (points, 4571.6516636328242, 4811.7457230672644);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(ConceptStationTests, RoadSegmentsProblem7)
    {
    bvector<DPoint2d> points;
    points.push_back(DPoint2d::From(305272.64886154403, 261725.80828632164));
    points.push_back(DPoint2d::From(305267.02381875471, 261727.22960490576));
    points.push_back(DPoint2d::From(305254.63546996785, 261729.86115168623));
    points.push_back(DPoint2d::From(305241.48265273118, 261733.84771756391));
    points.push_back(DPoint2d::From(305221.64934795426, 261744.06489700294));
    points.push_back(DPoint2d::From(305192.23557155882, 261767.15452787775));
    points.push_back(DPoint2d::From(305154.25593808276, 261788.48895928572));
    points.push_back(DPoint2d::From(305137.85448969586, 261795.57438841957));
    points.push_back(DPoint2d::From(305090.52001644979, 261810.46670811280));
    points.push_back(DPoint2d::From(305079.70217939361, 261813.37629397894));
    points.push_back(DPoint2d::From(305076.56982723257, 261814.22029925819));
    points.push_back(DPoint2d::From(305055.17500002607, 261823.76035814668));
    points.push_back(DPoint2d::From(305038.24200603901, 261830.20179487328));
    points.push_back(DPoint2d::From(305030.44007025694, 261837.53211224201));
    points.push_back(DPoint2d::From(305020.63567195344, 261849.07190176687));
    points.push_back(DPoint2d::From(305015.80820362474, 261856.75774879503));
    points.push_back(DPoint2d::From(305012.83362395584, 261861.50031806651));
    points.push_back(DPoint2d::From(305006.27756742440, 261884.61364304024));
    points.push_back(DPoint2d::From(305005.38806219772, 261902.15145671210));
    points.push_back(DPoint2d::From(305007.48991192487, 261908.57132145512));
    points.push_back(DPoint2d::From(305012.51605945919, 261923.91013625386));
    points.push_back(DPoint2d::From(305021.18951584515, 261941.03728184040));
    points.push_back(DPoint2d::From(305024.55420802365, 261947.66822273686));
    points.push_back(DPoint2d::From(305038.58639314986, 261968.09431759949));
    points.push_back(DPoint2d::From(305045.72269950260, 261984.96598442952));
    points.push_back(DPoint2d::From(305052.05286354950, 262007.59102088894));
    points.push_back(DPoint2d::From(305055.30064226611, 262029.37182733446));
    points.push_back(DPoint2d::From(305058.19923176168, 262057.35029193416));
    points.push_back(DPoint2d::From(305061.31355479622, 262092.42609755817));
    points.push_back(DPoint2d::From(305063.76393379737, 262107.29837061308));
    points.push_back(DPoint2d::From(305066.75446671376, 262118.60533927503));
    points.push_back(DPoint2d::From(305068.98072081915, 262127.70200100011));
    points.push_back(DPoint2d::From(305056.41028261004, 262132.38866827666));
    points.push_back(DPoint2d::From(305047.81110438256, 262138.80817500345));
    points.push_back(DPoint2d::From(305037.91579461977, 262147.48235686898));
    points.push_back(DPoint2d::From(305033.32116311858, 262154.34629205364));
    points.push_back(DPoint2d::From(305027.75430518854, 262166.99691257562));
    points.push_back(DPoint2d::From(305025.75154435448, 262182.97972930159));
    points.push_back(DPoint2d::From(305027.08044294763, 262195.64167770307));
    points.push_back(DPoint2d::From(305036.66706111806, 262223.83139047708));
    points.push_back(DPoint2d::From(305041.79245870328, 262244.03507885366));
    points.push_back(DPoint2d::From(305046.82636228431, 262266.48237011611));
    points.push_back(DPoint2d::From(305051.72724515636, 262290.68455769151));
    points.push_back(DPoint2d::From(305059.52794956038, 262304.44633154414));
    points.push_back(DPoint2d::From(305068.44199791510, 262315.32035164098));
    points.push_back(DPoint2d::From(305080.92018009163, 262324.87279560260));
    points.push_back(DPoint2d::From(305087.59969958529, 262326.65016979101));
    points.push_back(DPoint2d::From(305076.23378120287, 262343.52116544201));
    points.push_back(DPoint2d::From(305066.97011432517, 262352.37304676539));
    points.push_back(DPoint2d::From(305060.86366596841, 262355.94925745885));
    points.push_back(DPoint2d::From(305049.51491929317, 262359.93624648947));
    points.push_back(DPoint2d::From(305045.70985161414, 262361.28005532484));
    points.push_back(DPoint2d::From(305033.45553852472, 262365.94456228550));
    points.push_back(DPoint2d::From(305024.62403295981, 262372.57512465800));
    points.push_back(DPoint2d::From(305014.51299530262, 262383.70397072594));
    points.push_back(DPoint2d::From(305007.01890497305, 262396.99875959876));
    points.push_back(DPoint2d::From(304999.79882145871, 262416.78001814737));
    points.push_back(DPoint2d::From(304990.31067970040, 262438.96032409585));
    points.push_back(DPoint2d::From(304982.06074879068, 262449.61166851158));
    points.push_back(DPoint2d::From(304967.80432962405, 262459.37432485848));
    points.push_back(DPoint2d::From(304955.45893470664, 262460.06267839792));
    points.push_back(DPoint2d::From(304936.38426154933, 262459.15152299526));
    points.push_back(DPoint2d::From(304918.68870203249, 262456.74100356671));
    points.push_back(DPoint2d::From(304900.19562248822, 262451.40939077764));
    points.push_back(DPoint2d::From(304885.13360650669, 262448.94345640979));
    points.push_back(DPoint2d::From(304864.09826015792, 262447.18834266003));
    points.push_back(DPoint2d::From(304848.49618402409, 262449.40961011528));
    points.push_back(DPoint2d::From(304830.53468898265, 262454.71863129764));
    points.push_back(DPoint2d::From(304803.35157000483, 262463.59301666945));
    points.push_back(DPoint2d::From(304791.09758816153, 262467.59152188181));
    points.push_back(DPoint2d::From(304768.60014569614, 262475.35534011881));


    TestTrim (points, 0.0, 214.16672945205073);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(ConceptStationTests, RoadSegmentsProblem8)
    {
    bvector<DPoint2d> points;
    points.push_back(DPoint2d::From(305029.61674626649, 260298.83680766757));
    points.push_back(DPoint2d::From(305058.26822650776, 260366.01223336189));
    points.push_back(DPoint2d::From(305065.39740874729, 260392.65791840322));
    points.push_back(DPoint2d::From(305068.07249428343, 260413.97213791154));
    points.push_back(DPoint2d::From(305066.29308450967, 260441.96143423050));
    points.push_back(DPoint2d::From(305063.16751858580, 260468.16247402626));
    points.push_back(DPoint2d::From(305015.01469200675, 260606.27508487395));
    points.push_back(DPoint2d::From(305007.88371454726, 260642.69438385620));
    points.push_back(DPoint2d::From(305005.80533270299, 260674.47115297551));
    points.push_back(DPoint2d::From(305006.06282073056, 260678.58071194618));
    points.push_back(DPoint2d::From(305007.06784389762, 260694.89677173487));
    points.push_back(DPoint2d::From(305011.49641561572, 260714.31175632431));
    points.push_back(DPoint2d::From(305016.77255926654, 260733.81562516774));
    points.push_back(DPoint2d::From(305023.45307211886, 260751.79789716779));
    points.push_back(DPoint2d::From(305037.27131028112, 260779.33234078187));
    points.push_back(DPoint2d::From(305054.43835572101, 260803.97913646448));
    points.push_back(DPoint2d::From(305071.61379545729, 260823.26134217653));
    points.push_back(DPoint2d::From(305086.72859990195, 260836.17925347510));
    points.push_back(DPoint2d::From(305114.13291598385, 260858.61633707519));
    points.push_back(DPoint2d::From(305129.45528280275, 260872.13412111934));
    points.push_back(DPoint2d::From(305140.42330059095, 260886.78463486891));
    points.push_back(DPoint2d::From(305147.17795049225, 260908.85437191959));
    points.push_back(DPoint2d::From(305150.19302849483, 260935.59993953659));
    points.push_back(DPoint2d::From(305153.16482618445, 260997.17675919022));
    points.push_back(DPoint2d::From(305153.23948063736, 260999.84241995524));
    points.push_back(DPoint2d::From(305155.83559207252, 261095.98392501130));
    points.push_back(DPoint2d::From(305156.67312610190, 261130.51534207101));
    points.push_back(DPoint2d::From(305157.91698430688, 261181.58496988501));
    points.push_back(DPoint2d::From(305158.14089218801, 261190.47050858225));
    points.push_back(DPoint2d::From(305157.66555980785, 261224.74640918925));
    points.push_back(DPoint2d::From(305159.06899980508, 261241.14027924865));
    points.push_back(DPoint2d::From(305160.55522343906, 261263.55410032469));
    points.push_back(DPoint2d::From(305164.18557155557, 261278.60414852045));
    points.push_back(DPoint2d::From(305173.63188594428, 261301.69590341701));
    points.push_back(DPoint2d::From(305178.50872180966, 261313.56944800663));
    points.push_back(DPoint2d::From(305193.86317150464, 261328.20918382547));
    points.push_back(DPoint2d::From(305213.31380719185, 261345.11499762564));
    points.push_back(DPoint2d::From(305230.04765447980, 261355.88968529209));
    points.push_back(DPoint2d::From(305253.59493811073, 261365.72075115918));
    points.push_back(DPoint2d::From(305256.86861678335, 261367.14264345291));
    points.push_back(DPoint2d::From(305264.57927720010, 261369.37562732707));
    points.push_back(DPoint2d::From(305280.39117351983, 261372.87534449558));
    points.push_back(DPoint2d::From(305323.63965000043, 261374.65547100670));
    points.push_back(DPoint2d::From(305322.50584270392, 261426.61353702686));
    points.push_back(DPoint2d::From(305326.06714872248, 261471.01924809191));
    points.push_back(DPoint2d::From(305335.87596361898, 261525.19951656676));
    points.push_back(DPoint2d::From(305339.38798430876, 261560.28653553437));
    points.push_back(DPoint2d::From(305339.26850828342, 261602.30400394771));
    points.push_back(DPoint2d::From(305339.89029810822, 261620.53050720535));

    TestTrim (points, 412.21369497198003, 763.85989015151711);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(ConceptStationTests, RoadSegmentsProblem9)
    {
    bvector<DPoint2d> points;
    points.push_back(DPoint2d::From(305256.09730626334, 261344.57334551431));
    points.push_back(DPoint2d::From(305251.74345709401, 261342.81817829265));
    points.push_back(DPoint2d::From(305247.67213135149, 261340.79646538227));
    points.push_back(DPoint2d::From(305243.44291554246, 261339.01909714501));
    points.push_back(DPoint2d::From(305234.52754462871, 261334.35364920209));
    points.push_back(DPoint2d::From(305228.28765266965, 261330.35478604148));
    points.push_back(DPoint2d::From(305222.04779222416, 261325.70062047179));
    points.push_back(DPoint2d::From(305210.45723781100, 261314.37089832249));
    points.push_back(DPoint2d::From(305204.66610769951, 261308.59497548576));
    points.push_back(DPoint2d::From(305197.75344382261, 261299.05375196255));
    points.push_back(DPoint2d::From(305191.28951652849, 261288.39076158713));
    points.push_back(DPoint2d::From(305188.62264018389, 261281.72646976385));
    points.push_back(DPoint2d::From(305185.59024741489, 261273.35169502284));
    points.push_back(DPoint2d::From(305182.37527662719, 261260.63410964358));
    points.push_back(DPoint2d::From(305182.37554828310, 261255.52493045415));
    points.push_back(DPoint2d::From(305197.36550690676, 261252.13813520345));
    points.push_back(DPoint2d::From(305199.84998555400, 261251.58292736235));
    points.push_back(DPoint2d::From(305219.60114136781, 261247.98541357304));
    points.push_back(DPoint2d::From(305225.39153363655, 261266.85639610077));
    points.push_back(DPoint2d::From(305228.73137077095, 261274.40929438482));
    points.push_back(DPoint2d::From(305233.64163190773, 261281.95118198573));
    points.push_back(DPoint2d::From(305240.32178469579, 261288.61573518842));
    points.push_back(DPoint2d::From(305247.45891395339, 261295.50246138434));
    points.push_back(DPoint2d::From(305257.92803377804, 261301.72299149289));
    points.push_back(DPoint2d::From(305256.97105892526, 261324.13668052136));
    points.push_back(DPoint2d::From(305256.09730626334, 261344.57334551431));
    points.push_back(DPoint2d::From(305256.86861678335, 261367.14264345291));

    TestTrim (points, 139.94956368885721, 250.98884090152495);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(CurveVectorWithXIndex,CloneIntervals)
    {
    auto elevationCurve = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
    DPoint3d pointA = DPoint3d::From (3,0,1);
    DPoint3d pointB = DPoint3d::From (10,0,4);
    DPoint3d pointC = DPoint3d::From (20,0,2);
    elevationCurve->push_back (ICurvePrimitive::CreateLine (DSegment3d::From (pointA, pointB)));
    elevationCurve->push_back (ICurvePrimitive::CreateLine (DSegment3d::From (pointB, pointC)));
    auto indexedElevation = CurveVectorWithXIndex::Create (*elevationCurve);
    bvector<DSegment1d> fractionalSegments
        {
        DSegment1d (0.0, 0.75),
        DSegment1d (0.2, 0.75),
        DSegment1d (0.0, 1.0),
        DSegment1d (0.5, 1.0),
        DSegment1d (0.7, 0.9)
        };
    if (Check::True (indexedElevation.IsValid ()))
        {
        DRange1d xRange = indexedElevation->XRange ();
        for (DSegment1d segment : fractionalSegments)
            {
            auto subset = indexedElevation->CloneDirectedXInterval (xRange.FractionToDouble (segment.GetStart ()),
                        xRange.FractionToDouble (segment.GetEnd ()));
            Check::Print (*subset, "partial elevation");
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(CurveVectorWithXIndex,ClonePartialWithToleranceIssue)
    {
    double stationB = 30.0;
    auto elevationCurve = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
    DPoint3d pointA = DPoint3d::From (0,0,1);
    DPoint3d pointB = DPoint3d::From (stationB,0,2);
    DPoint3d pointC = DPoint3d::From (60,0,4);
    DPoint3d pointD = DPoint3d::From (70,0,4);
    elevationCurve->push_back (ICurvePrimitive::CreateLine (DSegment3d::From (pointA, pointB)));
    elevationCurve->push_back (ICurvePrimitive::CreateLine (DSegment3d::From (pointB, pointC)));
    elevationCurve->push_back (ICurvePrimitive::CreateLine (DSegment3d::From (pointC, pointD)));
    auto indexPtr = CurveVectorWithXIndex::Create (*elevationCurve);
    Check::Print (*elevationCurve, "ElevationCurve");
    if (Check::True (indexPtr.IsValid ()))
        {
        double setback = 1.0e-6;
        for (;setback > 1.1e-12; setback /= 10.0)
            {
            double startStation = stationB - setback;
            double endStation = 70.0;
            double lengthA = endStation - startStation;
            ValidatedPathLocationDetail detailA = indexPtr->XToPathLocationDetail(startStation);
            ValidatedPathLocationDetail detailB = indexPtr->XToPathLocationDetail(endStation);
            if (detailA.IsValid () && detailB.IsValid ())
                {
                CurveVectorPtr result = indexPtr->GetCurveVectorPtr()->CloneBetweenDirectedFractions(
                            (int)detailA.Value().PathIndex(), detailA.Value().CurveFraction(),
                            (int)detailB.Value().PathIndex(), detailB.Value().CurveFraction(),
                            false, true); // false: allowExtrapolation, true: usePartialCurves
                // verify no crash in GetRange 
                DRange3d range;
                result->GetRange (range);

                // Verify length with Metric tolerance
                DPoint3d xyz0, xyz1;
                result->GetStartEnd (xyz0, xyz1);
                double lengthB = xyz1.x - xyz0.x;
                Check::True (fabs (lengthB - lengthA) <= DoubleOps::SmallMetricDistance (), "ClonePartial with known small fragment matches length");
                // (We expect this to be suppressed by volume controls)
                Check::PrintIndent (0);
                Check::Print (setback, "setback");
                Check::Print ((int)result->size (), "count");
                Check::Print (range.low, "range.low");
                Check::Print (range.high, "range.high");
                Check::Print (*result, "Partial Elevation");
                }        
            }
        }

    }

