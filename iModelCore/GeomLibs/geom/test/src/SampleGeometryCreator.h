/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/test/src/SampleGeometryCreator.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once


struct SampleGeometryCreator
{
static void StrokeUnitCircle (bvector<DPoint3d> &points, size_t numPoints)
    {
    double dTheta;
    DoubleOps::SafeDivide (dTheta, Angle::TwoPi (), (double)(numPoints - 1), 0.0);
    for (size_t i = 0; i < numPoints; i++)
        {
        double theta = i * dTheta;
        points.push_back (DPoint3d::From (cos(theta), sin(theta), 0.0));
        }
    }

static void AddArcs (bvector<IGeometryPtr> &data)
    {
    data.push_back (IGeometry::Create (
            ICurvePrimitive::CreateArc (DEllipse3d::From 
                (
                0,0,0,
                1,0,0,
                0,1,0,
                0.0, Angle::TwoPi ()
                ))));
    data.push_back (IGeometry::Create (
            ICurvePrimitive::CreateArc (DEllipse3d::From 
                (
                0,0,0,
                2,0,0,
                0,1,0,
                0.0, Angle::TwoPi ()
                ))));
    data.push_back (IGeometry::Create (
            ICurvePrimitive::CreateArc (DEllipse3d::From 
                (
                0,0,0,
                2,0,0,
                0,1,0,
                0.0, Angle::TwoPi ()
                ))));
    }

static void AddLines (bvector<IGeometryPtr> &data)
    {
    data.push_back (IGeometry::Create (
            ICurvePrimitive::CreateLine (DSegment3d::From 
                (0,0,0, 1,0,0))));
    data.push_back (IGeometry::Create (
            ICurvePrimitive::CreateLine (DSegment3d::From 
                (1,2,3, 5,2,1))));
    }

static void AddSpheres (bvector<IGeometryPtr> &data, bool capped)
    {
    // <positiveAndNegativeDeterminant> X <full, positiveAndNegativeSweepsWithAndWithoutPoles>
    bvector<double> degrees {-90.0, -10.0, 45.0, 90.0};
    bvector<double> zSign {1.0, -1.0};
    double radius = 2.0;
    for (double sZ : zSign)
        {
        for (double d0 : degrees)
            {
            for (double d1 : degrees)
                {
                if (d0 != d1)
                    {
                    DgnSphereDetail sphere (DPoint3d::From (0,0,0), radius);
                    sphere.m_startLatitude = Angle::DegreesToRadians (d0);
                    sphere.m_latitudeSweep = Angle::DegreesToRadians (d1 - d0);
                    sphere.m_localToWorld.ScaleMatrixColumns (1.0, 1.0, sZ);
                    sphere.m_capped = capped;
                    data.push_back (IGeometry::Create (ISolidPrimitive::CreateDgnSphere (sphere)));
                    }
                }
            }
        }
    }

static void AddBoxes (bvector<IGeometryPtr> &data, double ax, double ay, double az, bool capped)
    {
    bvector<double>signs {1.0, -1.0};
    for (double sZ : signs)
        for (double sX : signs)
            for (double sY : signs)
                {
                DgnBoxDetail detail(
                        DPoint3d::From (0,0,0),
                        DPoint3d::From (0,0,sZ * az),
                        DVec3d::From (sX, 0, 0),
                        DVec3d::From (0, sY, 0),
                        ax, ay,
                        ax, ay,
                        capped);
                data.push_back (IGeometry::Create (ISolidPrimitive::CreateDgnBox (detail)));
                }

    }
static void AddTorusPipes (bvector<IGeometryPtr> &data, double majorRadius, double minorRadius, bool capped)
    {
    bvector<double>signs {1.0, -1.0};
    bvector <double>sweeps {-1.0, 1.0, 2.0, -2.0};
    for (double sY : signs)
        for (double sX : signs)
            for (double sSweep : signs)
                {
                DgnTorusPipeDetail detail(
                        DPoint3d::From (0,0,0),
                        DVec3d::From (sX, 0, 0),
                        DVec3d::From (0, sY, 0),
                        majorRadius,
                        minorRadius,
                        sSweep * Angle::Pi (),
                        capped);
                data.push_back (IGeometry::Create (ISolidPrimitive::CreateDgnTorusPipe (detail)));
                }
    }
static void AddSimplestSolidPrimitives (bvector<IGeometryPtr> &data, bool capped)
    {
    bvector <double>sweeps;
    sweeps.push_back (1.0);
    sweeps.push_back (-1.0);
    bvector<double>signs;
    signs.push_back (1.0);
    signs.push_back (-1.0);

    //double aX = 3.0;
    //double aY  = 5.0;

    // ====================
    for (double sZ : signs)
        for (double sX : signs)
            for (double sY : signs)
                {
                DgnSphereDetail detail (DPoint3d::From (0,0,0),
                        RotMatrix::FromScaleFactors (sX, sY, sZ),
                        6.0);
                detail.m_capped = capped;
                data.push_back (IGeometry::Create (ISolidPrimitive::CreateDgnSphere (detail)));

                }


    // ====================
    for (double sweep : sweeps)
        for (double sZ : signs)
            for (double sX : signs)
                for (double sY : signs)
                {
                DgnSphereDetail sphere (DPoint3d::From (0,0,0), 6);
                sphere.m_startLatitude = 0.0;
                sphere.m_latitudeSweep = sweep;
                sphere.m_localToWorld.ScaleMatrixColumns (sX, sY, sZ);
                sphere.m_capped = capped;
                data.push_back (IGeometry::Create (ISolidPrimitive::CreateDgnSphere (sphere)));
                }


    // ====================
    for (double s : signs)
        {
        DgnConeDetail detail(
                DPoint3d::From (0,0,0),
                DPoint3d::From (0,0,10.0 * s),
                1.0, 1.0, capped);
        data.push_back (IGeometry::Create (ISolidPrimitive::CreateDgnCone (detail)));
        }


    // ====================
    AddBoxes (data, 1,1,10, capped);
    // ====================
    for (double sY : signs)
        for (double sX : signs)
            for (double sSweep : signs)
                {
                DgnTorusPipeDetail detail(
                        DPoint3d::From (0,0,0),
                        DVec3d::From (sX, 0, 0),
                        DVec3d::From (0, sY, 0),
                        10.0,
                        1.0,
                        sSweep * Angle::Pi (),
                        capped);
                data.push_back (IGeometry::Create (ISolidPrimitive::CreateDgnTorusPipe (detail)));
                }


    }

static void AddXYSegmentsFromOrigin (bvector<ICurvePrimitivePtr> &data, bvector<DPoint3d> &targetXYZ)
    {
    for (DPoint3d xyz : targetXYZ)
        {
        data.push_back (ICurvePrimitive::CreateLine (DSegment3d::From (DPoint3d::From (0,0,0), xyz)));

        }
    }

static void AddStepLinestrings (bvector<ICurvePrimitivePtr> &data, int stepCycles)
    {
    double xx = 0.0;
    double y0 = 0.0;
    double y1 = 1.0;
    bvector<DPoint3d> stepPoints;
    for (int i = 0; i < stepCycles; i++)
        {
        stepPoints.push_back (DPoint3d::From (xx, y0, 0));
        xx += 1.0;
        stepPoints.push_back (DPoint3d::From (xx, y0, 0));
        stepPoints.push_back (DPoint3d::From (xx, y1, 0));
        xx += 1.0;
        stepPoints.push_back (DPoint3d::From (xx, y1, 0));
        data.push_back (ICurvePrimitive::CreateLineString (stepPoints));
        }
    }

static void AddUnitArcSweepVariety (bvector<ICurvePrimitivePtr> &data, size_t numStartAngle, size_t numSweepAngle)
    {
    bvector<double> degrees;
    degrees.push_back (0.0);
    degrees.push_back (45.0);
    degrees.push_back (90.0);
    degrees.push_back (135.0);
    degrees.push_back (180.0);
    degrees.push_back (270.0);
    degrees.push_back (215.0);
    degrees.push_back (315.0);
    degrees.push_back (360.0);
    degrees.push_back (30.0);

    DEllipse3d ellipse0 = DEllipse3d::FromVectors (
          DPoint3d::From (0,0,0), DVec3d::From (1,0,0), DVec3d::From (0,1,0), 0.0, Angle::Pi ()
          );
    for (size_t i0 = 0; i0 < numStartAngle && i0 < degrees.size (); i0++)
        {
        double d0 = degrees[i0];
        for (size_t i1 = 1; i1 < numSweepAngle && i1 < degrees.size (); i1++)
            {
            double d1 = degrees[i1];
            if (d1 > 0.0)
                {
                DEllipse3d ellipse = ellipse0;
                ellipse.SetSweep (
                    Angle::DegreesToRadians (d0),
                    Angle::DegreesToRadians (d1));
                data.push_back (ICurvePrimitive::CreateArc (ellipse));
                ellipse.SetSweep (
                    Angle::DegreesToRadians (d0),
                    -Angle::DegreesToRadians (d1));
                data.push_back (ICurvePrimitive::CreateArc (ellipse));
                if (d0 != 0.0)
                    {
                    ellipse.SetSweep (
                        -Angle::DegreesToRadians (d0),
                        Angle::DegreesToRadians (d1));
                    data.push_back (ICurvePrimitive::CreateArc (ellipse));
                    ellipse.SetSweep (
                        -Angle::DegreesToRadians (d0),
                        -Angle::DegreesToRadians (d1));
                    data.push_back (ICurvePrimitive::CreateArc (ellipse));
                    }
                }
            }
        }
    }

static void AddCone (bvector<IGeometryPtr> &data)
    {
    double rA = 1.0;
    double rB = 0.5;
    double zA = 0.0;
    double zB = 1.0;
    DgnConeDetail ConeData (
        DPoint3d::From (0,0,zA),
        DPoint3d::From (0,0,zB),
        rA, rB, true);
    data.push_back (IGeometry::Create (ISolidPrimitive::CreateDgnCone (ConeData)));
    }
static void AddAllCones (bvector<IGeometryPtr> &data, bool capped)
    {
    //double rA = 1.0;
    //double rB = 0.5;
    double zA = 0.0;
    double zB = 1.0;
    for (double rA : bvector<double>{1.0, 2.0, 0.0})
        {
        for (double rB : bvector<double>{1.0, 2.0, 0.0})
            {
            if (rA != 0.0 || rB != 0.0)
                {
                DgnConeDetail ConeData (
                    DPoint3d::From (0,0,zA),
                    DPoint3d::From (0,0,zB),
                    rA, rB, capped);
                data.push_back (IGeometry::Create (ISolidPrimitive::CreateDgnCone (ConeData)));
                DgnConeDetail ConeData1 (
                    DPoint3d::From (0,0,zB),
                    DPoint3d::From (0,0,zA),
                    rA, rB, capped);
                data.push_back (IGeometry::Create (ISolidPrimitive::CreateDgnCone (ConeData1)));
                }
            }
        }
    }

static void GetContours (bvector<CurveVectorPtr> &contours, double z = 0.0)
    {
    bvector<DPoint3d> points
            {
            DPoint3d::From (0,0,z),
            DPoint3d::From (3,0,z),
            DPoint3d::From (3,2,z),
            DPoint3d::From (2.1,2.0, z),
            DPoint3d::From (2,3,z)
            };
        
    contours.push_back (CurveVector::CreateLinear (points));
    contours.push_back (CurveVector::CreateRectangle (0,0, 4,3, z));
    contours.push_back (CurveVector::CreateDisk (DEllipse3d::From (2,0,z,   2,0,0,   0,1,0,  0.0, Angle::TwoPi ())));
    CurveVectorPtr bcurve = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
    bcurve->push_back (ICurvePrimitive::CreateBsplineCurve 
            (MSBsplineCurve::CreateFromPolesAndOrder (points, nullptr, nullptr, 4, false, true)));
    contours.push_back (bcurve);
    }

static void AddExtrusions (bvector<IGeometryPtr> &data, bool capped)
    {
    bvector<CurveVectorPtr> contours;

    GetContours (contours, 0.0);

    for (auto &c : contours)
        {
        if (!capped || c->IsAnyRegionType ())
            {
            for (auto dz : bvector<double> {3.0, -3.0})
                {
                data.push_back (IGeometry::Create (ISolidPrimitive::CreateDgnExtrusion
                    (
                    DgnExtrusionDetail (c, DVec3d::From (0,0, dz), capped)
                    )));
                }
            }
        }
    }


static void AddRuled (bvector<IGeometryPtr> &data, bool capped)
    {
    bvector<CurveVectorPtr> baseContours;
    GetContours (baseContours, 0.0);
    bvector<size_t> numSection {2, 3, 5};
    bvector<double> sectionScale {1.0, 0.8, 1.2, 0.9, 1.5};
    bvector<double> signs {1.0, -1.0};
    for (double sign : signs)
        {
        for (auto &base : baseContours)
            {
            if (!capped || base->IsAnyRegionType ())
                {
                for (auto n : numSection)
                    {
                    bvector<CurveVectorPtr> contours;
                    for (size_t i = 0; i < n; i++)
                        {
                        double s = sectionScale[i];
                        auto c = base->Clone (Transform::From (DVec3d::From (0,0, sign * i)) * Transform::FromScaleFactors (s, s, s));
                        contours.push_back (c);
                        }
                    data.push_back (
                        IGeometry::Create (
                        ISolidPrimitive::CreateDgnRuledSweep
                            (
                            DgnRuledSweepDetail
                                (
                                contours,
                                capped
                                )
                            )));
                    }
                }
            }
        }
    }


static void AddRotations (bvector<IGeometryPtr> &data, bool capped)
    {
    bvector<CurveVectorPtr> contours;
    GetContours (contours);

    for (auto &c : contours)
        {
        if (!capped || c->IsAnyRegionType ())
            {
            double dx = -1.0;
            for (auto sweepDegrees : bvector<double> {45.0, -45.0, 360.0, -360.0})
                {
                data.push_back (IGeometry::Create (
                    ISolidPrimitive::CreateDgnRotationalSweep
                        (
                        DgnRotationalSweepDetail
                            (
                            c,
                            DPoint3d::From (0,5,0), DVec3d::From (dx, 0, 0),
                            Angle::DegreesToRadians (sweepDegrees),
                            capped
                            )
                        )));
                }
            }
        }
    }


static void AddExtrusion (bvector<IGeometryPtr> &data)
    {
    double rA = 1.0;
    double rB = 0.5;
    // unused - double zA = 0.0;
    double zB = 1.0;
    CurveVectorPtr ellipse = CurveVector::CreateDisk (
                    DEllipse3d::From (0,0,0,   rA, 0,0,   0,rB,0,   0.0, Angle::TwoPi ()));
    DgnExtrusionDetail extrusionDetail (ellipse, DVec3d::From (0,0,zB), true);
    data.push_back (IGeometry::Create (ISolidPrimitive::CreateDgnExtrusion (extrusionDetail)));
    }

static void AddSphere (bvector<IGeometryPtr> &data)
    {
    auto sphereData = DgnSphereDetail  (
                DPoint3d::From(0,0,0),
                DVec3d::From (1,0,0),
                DVec3d::From (0,1,0),
                1,1,
                0.0, 0.1, true);
    auto solid = ISolidPrimitive::CreateDgnSphere (sphereData);
    data.push_back (IGeometry::Create (solid));
    }

static void AddBox (bvector<IGeometryPtr> &data)
    {
    DgnBoxDetail boxData (
                DPoint3d::From (1,1,1),
                DPoint3d::From (1,1,2),
                DVec3d::From (1,0,0), DVec3d::From (0,1,0), 2,2,2,2, true);
    auto solid = ISolidPrimitive::CreateDgnBox (boxData);
    data.push_back (IGeometry::Create (solid));
    }

static void AppendXYLineSegment (CurveVectorR chain, double length, Angle angle)
    {
    DPoint3d xyzA, xyzB;
    DVec3d tangentA, tangentB, tangentC;
    chain.GetStartEnd (xyzA, xyzB, tangentA, tangentB);
    tangentB.Normalize ();
    tangentC.RotateXY (tangentB, angle.Radians ());
    chain.push_back (ICurvePrimitive::CreateLine (DSegment3d::From (xyzB, xyzB + length * tangentC)));
    }
static void AddRuledSweep (bvector<IGeometryPtr> &data, size_t numSection = 2, size_t numExtraPrimitive = 0)
    {
    double sweep = 2.0;
    double scale = 1.5;
    double b = 2.0;
    DEllipse3d ellipseA = DEllipse3d::From (1,1,1,    1,0,0,   0,b,0,   0.0, sweep);
    DEllipse3d ellipseB = DEllipse3d::FromVectors (
            DPoint3d::From (1,1,2),
            DVec3d::FromScale (ellipseA.vector0, scale), 
            DVec3d::FromScale (ellipseA.vector90, scale),
            0.0, sweep); 

    CurveVectorPtr pathA = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
    pathA->push_back (ICurvePrimitive::CreateArc (ellipseA));
    if (numExtraPrimitive > 0)
        AppendXYLineSegment (*pathA, 2.0, Angle::FromDegrees(0.0));
    CurveVectorPtr pathB = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
    pathB->push_back (ICurvePrimitive::CreateArc (ellipseB));
    if (numExtraPrimitive > 0)
        AppendXYLineSegment (*pathB, 4.0, Angle::FromDegrees (10.0));

    bvector<CurveVectorPtr> ruledSections;
    ruledSections.push_back (pathA);
    ruledSections.push_back (pathB);
    for (size_t section = 2; section < numSection; section++)
        {
        DEllipse3d ellipseC = DEllipse3d::FromVectors (
            DPoint3d::From (1, 1, (double)(section + 1.0)),
            DVec3d::FromScale (ellipseA.vector0, scale),
            DVec3d::FromScale (ellipseA.vector90, scale),
            0.0, sweep);
        CurveVectorPtr pathC = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
        pathC->push_back (ICurvePrimitive::CreateArc (ellipseC));
        if (numExtraPrimitive > 0)
            AppendXYLineSegment (*pathC, 4.0, Angle::FromDegrees (10.0));
        ruledSections.push_back (pathC);
        }
    DgnRuledSweepDetail ruledSweepData (ruledSections, false);
    ISolidPrimitivePtr solid = ISolidPrimitive::CreateDgnRuledSweep (ruledSweepData);
    data.push_back (IGeometry::Create (solid));
    }

static void AddTorusPipe (bvector<IGeometryPtr> &data)
    {
    DgnTorusPipeDetail detail (
        DPoint3d::From (0,0,0),
        DVec3d::From (1,0,0),
        DVec3d::From (0,1,0),
        2,
        0.5,
        Angle::PiOver2 (),
        true);
    ISolidPrimitivePtr solid = ISolidPrimitive::CreateDgnTorusPipe (detail);
    data.push_back (IGeometry::Create (solid));
    }

static void AddRotation (bvector<IGeometryPtr> &data)
    {
    bvector<DPoint3d> points
        {
        DPoint3d::From (1,0,0),
        DPoint3d::From (2,0,0),
        DPoint3d::From (2,0,1),
        DPoint3d::From (1,0,1),
        DPoint3d::From (1,0,0)
        };
    auto section = CurveVector::CreateLinear (points, CurveVector::BOUNDARY_TYPE_Open);
    DgnRotationalSweepDetail detail
        (
        section,
        DPoint3d::From (0,0,0),
        DVec3d::From (0,0,1),
        Angle::TwoPi (),
        false
        );
    ISolidPrimitivePtr solid = ISolidPrimitive::CreateDgnRotationalSweep (detail);
    data.push_back (IGeometry::Create (solid));
    }

static void AddAllSolidTypes (bvector<IGeometryPtr> &data)
    {
    AddCone (data);
    AddSphere (data);
    AddExtrusion (data);
    AddBox (data);
    AddRuledSweep (data, 2);
    AddRuledSweep (data, 3);
    AddRuledSweep (data, 3, 1);
    AddRotation (data);
    AddTorusPipe (data);
    }


static void AddAllCurveTypes (bvector<IGeometryPtr> &data)
    {
    data.push_back (IGeometry::Create (ICurvePrimitive::CreateLine (
            DSegment3d::From (1,2,3,4,5,6))));
    data.push_back (IGeometry::Create (ICurvePrimitive::CreateArc (
            DEllipse3d::From (1,2,3,4,5,6,7,8,9,0,2))));
    bvector<DPoint3d> points;
    int n = 10;
    for (int i = 0; i < n; i++)
        points.push_back (DPoint3d::From (i,0.1 * (double)i, i));
    data.push_back (IGeometry::Create (ICurvePrimitive::CreateLineString (points)));
    MSBsplineCurvePtr curve = MSBsplineCurve::CreateFromPolesAndOrder (points, nullptr, nullptr, 4, false, true);
    data.push_back (IGeometry::Create (ICurvePrimitive::CreateBsplineCurve (curve)));
    }

static void AddAllTypes (bvector<IGeometryPtr> &data)
    {
    AddAllCurveTypes (data);
    AddAllSolidTypes (data);
    }

static void AddSizeSampler (bvector<IGeometryPtr> &data)
    {
    // All lines have same size . .
    data.push_back (IGeometry::Create (ICurvePrimitive::CreateLine (
            DSegment3d::From (1,2,3,4,5,6))));

    // All arcs have same size . .
    data.push_back (IGeometry::Create (ICurvePrimitive::CreateArc (
            DEllipse3d::From (1,2,3,4,5,6,7,8,9,0,2))));

    bvector<int> lineStringSizes;
    lineStringSizes.push_back (4);
    lineStringSizes.push_back (10);
    lineStringSizes.push_back (110);
    lineStringSizes.push_back (1110);
    for (int n : lineStringSizes)
        {
        bvector<DPoint3d> points;
        for (int i = 0; i < n; i++)
            points.push_back (DPoint3d::From (i,0.1 * (double)i, i));
        data.push_back (IGeometry::Create (ICurvePrimitive::CreateLineString (points)));

        }
#ifdef testAkima
    for (int n : lineStringSizes )
        {
        bvector<DPoint3d> points;
        StrokeUnitCircle (points, n);
        data.push_back (IGeometry::Create (ICurvePrimitive::CreateAkimaCurve (&points[0], points.size ())));
        }
#endif
    bvector<int> bcurveSizes;
    bcurveSizes.push_back (4);
    bcurveSizes.push_back (10);

    bvector<int> bcurveOrders;
    bcurveOrders.push_back (2);
    bcurveOrders.push_back (3);
    bcurveOrders.push_back (4);
    bcurveOrders.push_back (6);
    for (int order : bcurveOrders)
        {
        for (int n : lineStringSizes)
            {
            if (n < order)
                n = order;
            double dtheta = Angle::TwoPi () / (n-1);
            bvector<DPoint3d> points;
            for (int i = 0; i < n; i++)
                {
                double theta = i * dtheta;
                points.push_back (DPoint3d::From (cos(theta), sin(theta), 0.0));
                }
            MSBsplineCurvePtr curve = MSBsplineCurve::CreateFromPolesAndOrder (points, nullptr, nullptr, order, false, true);
            data.push_back (IGeometry::Create (ICurvePrimitive::CreateBsplineCurve (curve)));
            }
        }
    }

static void PromoteToCurveVector (bvector<CurveVectorPtr> &cv, bvector<ICurvePrimitivePtr> &cp)
    {
    for (size_t i = 0; i < cp.size (); i++)
        {
        CurveVectorPtr parent = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
        parent->Add (cp[i]);
        cv.push_back (parent);
        }
    }

static void PromoteToGeometry (bvector<IGeometryPtr> &geometry, bvector<CurveVectorPtr> &data)
    {
    for (size_t i = 0; i < data.size (); i++)
        {
        geometry.push_back (IGeometry::Create (data[i]));
        }
    }


static void AddXYOpenPaths (bvector<IGeometryPtr> &pathGeometry)
    {
    bvector<ICurvePrimitivePtr> paths;
    bvector<CurveVectorPtr>   pathVectors;
    bvector<DPoint3d> points;
    points.push_back (DPoint3d::From(1,0,0));
    points.push_back (DPoint3d::From (0,1,0));
    points.push_back (DPoint3d::From (1,1,1));
    AddXYSegmentsFromOrigin (paths, points);
    AddStepLinestrings (paths, 3);
    AddUnitArcSweepVariety (paths, 3, 3);
    PromoteToCurveVector (pathVectors, paths);
    PromoteToGeometry (pathGeometry, pathVectors);
    }

static void AddSpirals (bvector<IGeometryPtr> &pathGeometry)
    {
    pathGeometry.clear ();
    Transform placement = Transform::FromIdentity ();
    pathGeometry.push_back (
    IGeometry::Create (
        ICurvePrimitive::CreateSpiralBearingRadiusLengthRadius
            (
            DSpiral2dBase::TransitionType_Clothoid,
            0.0,
            0.0, 
            200.0,
            100.0,
            placement,
            0.0, 1.0
            )));

    pathGeometry.push_back(
    IGeometry::Create(
        ICurvePrimitive::CreateSpiralBearingRadiusLengthRadius
        (
            DSpiral2dBase::TransitionType_Clothoid,
            0.0,
            0.0,
            50.0,
            600,
            placement,
            0.0, 1.0
            )));

    }

static void AddMultiPrimitiveXYOpenPaths (bvector<CurveVectorPtr> &pathVectors)
    {
    bvector<DPoint3d> stringPoints
        {
        DPoint3d::From (0,0,0),
        DPoint3d::From (1,0,0),
        DPoint3d::From (2,2,0)
        }; 

    auto cv0 = CurveVector::CreateLinear (stringPoints);
    auto cv1 = cv0->Clone ();
    pathVectors.push_back (cv0);
    DPoint3d pointA, pointB;
    cv0->GetStartEnd (pointA, pointB);
    DEllipse3d arc = DEllipse3d::FromPointsOnArc (pointB, pointB + DVec3d::From (1,0,0), pointB + DVec3d::From(2,1,0));
    cv1->push_back (ICurvePrimitive::CreateArc (arc));
    pathVectors.push_back (cv1);
    }

static CurveVectorPtr CircleInRectangle ()
    {
    CurveVectorPtr disk0 = CurveVector::CreateDisk (DEllipse3d::FromVectors
        (
            DPoint3d::From (0, 0, 0),
            DVec3d::From (10, 0, 0),
            DVec3d::From (0, 10, 0),
            0.0, Angle::TwoPi ()
            ));
    CurveVectorPtr rectangle = CurveVector::CreateRectangle (
        -20, -20,
        20, 5,
        0.0,
        CurveVector::BOUNDARY_TYPE_Outer);
    CurveVectorPtr parent = CurveVector::Create (CurveVector::BOUNDARY_TYPE_ParityRegion);
    parent->Add (rectangle);
    parent->Add (disk0);
    return parent;
    }


};


// Return a bspline surface whose control points are on a doubly sinusoidal surface.
// @param [in] uOrder u direction order
// @param [in] vOrder v direction order
// @param [in] numI number of poles in i direction
// @param [in] numJ number of poles in j direction
// @param [in] q0I start angle for u direction sine calls
// @param [in] qI angle step between successive u direction sine calls.
// @param [in] q0J start angle for v direction sine calls
// @param [in] qJ angle step between successive v direction sine calls.
// @remarks Control point i,j is at (x,y,z) = (i,j,sin(q0I + aI * i) * sin (q0J + aJ * j))
MSBsplineSurfacePtr SurfaceWithSinusoidalControlPolygon
(int uOrder, int vOrder, size_t numI, size_t numJ, double q0I, double aI, double q0J, double aJ);

// Return a bspline surface whose control points are on a doubly sinusoidal surface.
// @param [in] uOrder u direction order
// @param [in] vOrder v direction order
// @param [in] numI number of poles in i direction
// @param [in] numJ number of poles in j direction
// @param [in] q0I start angle for u direction sine calls
// @param [in] qI angle step between successive u direction sine calls.
// @param [in] q0J start angle for v direction sine calls
// @param [in] qJ angle step between successive v direction sine calls.
// @remarks Control point i,j is at (x,y,z) = (i,j,sin(q0I + aI * i) * sin (q0J + aJ * j))
PolyfaceHeaderPtr PolyfaceWithSinusoidalGrid (size_t numI, size_t numJ, double q0I, double aI, double q0J, double aJ, bool triangulated = true);


// Return a bspline surface whose control points are evaluated on a hyperbolic patch.
// The patch has three points (00) (10) (01) at unit squre points and a fourth specified by parameters.
// @param [in] uOrder u direction order
// @param [in] vOrder v direction order
// @param [in] numI number of poles in i direction
// @param [in] numJ number of poles in j direction
// @param [in] x11 x coordinate of patch 11 point
// @param [in] y11 y coordinate of patch 11 point
// @param [in] z11 z coordinate of patch 11 point.
// @param [in] u1 upper u to evaluate grid
// @param [in] v1 upper v to evaluate grid.
MSBsplineSurfacePtr HyperbolicGridSurface (size_t uOrder, size_t vOrder, size_t numI, size_t numJ, 
double x11, double y11, double z11, double u1, double v1);

// Return a 2x2 linear bspline surface with control points (000)(100)(010)(u1 v1 w1)
MSBsplineSurfacePtr SimpleBilinearPatch (double u1, double v1, double w1);

// Return a 3x3 quadratic spline with weights
MSBsplineSurfacePtr SurfaceBubbleWithMidsideWeights
(
double wMidSide = 0.72,
double wInterior = 0.5
);


PolyfaceHeaderPtr UnitGridPolyface (DPoint3dDVec3dDVec3dCR plane, int numXEdge, int numYEdge, bool triangulated = false);

//! Create a polygon with square wave upper side.
//! Wave moves along x axis.
CurveVectorPtr SquareWavePolygon
(
int numTooth,
double x0 = 0.0,      //!< [in] initial x coordinate
double dx0 = 1.0,     //!< [in] x length of parts (at y=y0)
double dx1 = 1.0,     //!< [in] x length of parts (at y=y10)
double y0 = 0.0,      //!< [in] y height for dx0 part of tooth
double y1 = 1.0,      //!< [in] y height for dx1 part of tooth
bool closed = true,     //!< [in] true to close with base line.
double ybase = -1.0    //!< [in] y height for base line
);


PolyfaceHeaderPtr CreatePolyface_ExtrudedL
(
double x0,  // x coordinate at point 0,5,6
double y0,  // y coordinate at point 0,1,6
double x1,  // x coordinate at point 1,2
double y1,  // y coordinate at point 2,3
double x2,  // x coordinate at point 3,4
double y2,   // y coordinate at point 4,5
double h    // z distance for extrusion
);

// Create a typical transform for testing.
// All selects "0" is an identity.
// Defaults produce a general rigid rotation around a point near the origin.
// Translate * Rotate * Scale
Transform CreateTestTransform
(
double translateX = 1.0,
double translateY = 2.0,
double translateZ = 3.0,
double yawDegrees = 10.0,
double pitchDegrees = 5.0,
double rollDegrees = -12.0,
double xScaleFactor = 1.0,
double yScaleFactor = 1.0,
double zScaleFactor = 1.0
);
