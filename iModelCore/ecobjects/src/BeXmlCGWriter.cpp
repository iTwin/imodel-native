/*--------------------------------------------------------------------------------------+
|
|  $Source: src/BeXmlCGWriter.cpp $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"

BEGIN_BENTLEY_ECOBJECT_NAMESPACE
static const double s_nearZero = 9.0e-16;
static const size_t s_maxIndexPerLine = 5;

static double SuppressNearZeroCoordinate (double x, double refValue)
    {
    double tol = s_nearZero * refValue;
    return fabs (x) < tol ? 0.0 : x;
    }

static void Indent (BeXmlWriterR dest)
    {
    }

void BeXmlCGWriter::WriteList (BeXmlWriterR dest, bvector<DPoint3d> const & data, Utf8CP listName, Utf8CP itemName)
    {
    dest.WriteElementStart (listName);
    for (size_t i = 0 ; i < data.size (); i++)
        WriteXYZ (dest, itemName, data[i]);
    dest.WriteElementEnd ();
    }

void BeXmlCGWriter::WriteList (BeXmlWriterR dest, bvector<DVec3d> const & data, Utf8CP listName, Utf8CP itemName)
    {
    dest.WriteElementStart (listName);
    for (size_t i = 0 ; i < data.size (); i++)
        {
        Indent (dest);
        double refValue = data[i].Magnitude ();
        WriteXYZ (dest, itemName,
                    SuppressNearZeroCoordinate (data[i].x, refValue),
                    SuppressNearZeroCoordinate (data[i].y, refValue),
                    SuppressNearZeroCoordinate (data[i].z, refValue));
        }
    dest.WriteElementEnd ();
    }

void BeXmlCGWriter::WriteList (BeXmlWriterR dest, bvector<RgbFactor> const & data, Utf8CP listName, Utf8CP itemName)
    {
    dest.WriteElementStart (listName);
    for (size_t i = 0 ; i < data.size (); i++)
        {
        Indent (dest);
        WriteXYZ (dest, itemName, data[i].red, data[i].green, data[i].blue);
        }
    dest.WriteElementEnd ();
    }

void BeXmlCGWriter::WriteList (BeXmlWriterR dest, bvector<DPoint2d> const & data, Utf8CP listName, Utf8CP itemName)
    {
    dest.WriteElementStart (listName);
    for (size_t i = 0 ; i < data.size (); i++)
        {
        Indent (dest);
        double refValue = data[i].Magnitude ();
        WriteXY (dest, itemName, SuppressNearZeroCoordinate (data[i].x, refValue),
                    SuppressNearZeroCoordinate (data[i].y, refValue));
        }
    dest.WriteElementEnd ();
    }


void BeXmlCGWriter::WriteIndexList (BeXmlWriterR dest, bvector<int> const & data, Utf8CP listName, Utf8CP itemName)
    {
    dest.WriteElementStart (listName);
    size_t numThisLine = 0;
    for (size_t i = 0 ; i < data.size (); i++)
        {
        if (numThisLine == 0)
            Indent (dest);
        WriteInt (dest, itemName, data[i]);
        numThisLine++;
        bool needLineFeed = (numThisLine > s_maxIndexPerLine) || data[i] == 0;
        if (needLineFeed)
            numThisLine = 0;
        }
    dest.WriteElementEnd ();
    }

void BeXmlCGWriter::WriteList (BeXmlWriterR dest, bvector<double> const & data, Utf8CP listName, Utf8CP itemName)
    {
    dest.WriteElementStart (listName);
    for (size_t i = 0 ; i < data.size (); i++)
        WriteDouble (dest, itemName, data[i]);
    dest.WriteElementEnd ();
    }

void BeXmlCGWriter::WritePlacementZX
(
BeXmlWriterR dest,
DPoint3dCR origin,
DVec3dCR vectorZ,
DVec3dCR vectorX
)
    {
    dest.WriteElementStart ("placement");
    WriteXYZ (dest, "origin", origin);
    DVec3d unitZ, unitY, unitX;
    unitZ.Normalize (vectorZ);
    unitY.NormalizedCrossProduct (unitZ, vectorX);
    unitX.NormalizedCrossProduct (unitY, unitZ);
    WriteXYZ (dest, "vectorZ", unitZ);
    WriteXYZ (dest, "vectorX", unitX);
    dest.WriteElementEnd ();
    }

void BeXmlCGWriter::WritePlacementZX
(
BeXmlWriterR dest,
DPoint3dCR origin,
RotMatrixCR axes
)
    {
    DVec3d vectorX, vectorY, vectorZ;
    axes.GetColumns (vectorX, vectorY, vectorZ);
    dest.WriteElementStart ("placement");
    WriteXYZ (dest, "origin", origin);
    DVec3d unitZ, unitY, unitX;
    unitZ.Normalize (vectorZ);
    unitY.NormalizedCrossProduct (unitZ, vectorX);
    unitX.NormalizedCrossProduct (unitY, unitZ);
    WriteXYZ (dest, "vectorZ", unitZ);
    WriteXYZ (dest, "vectorX", unitX);
    dest.WriteElementEnd ();
    }


void BeXmlCGWriter::WritePlacementXY (BeXmlWriterR dest, DPoint3dCR origin, DVec3dCR vectorX, DVec3dCR vectorY)
    {
    DVec3d unitNormal, unitX;
    unitNormal.NormalizedCrossProduct (vectorX, vectorY);
    unitX.NormalizedCrossProduct (vectorY, unitNormal);
    WritePlacementZX (dest, origin, unitNormal, unitX);
    }

void BeXmlCGWriter::WriteXYZ (BeXmlWriterR dest, Utf8CP name, DPoint3dCR xyz)
    {
    wchar_t buffer[1024];
    BeStringUtilities::Snwprintf (buffer, _countof (buffer), L"%.15g,%.15g,%.15g", xyz.x, xyz.y, xyz.z);
    dest.WriteElementStart (name);
    dest.WriteText (buffer);
    dest.WriteElementEnd (); 
    }

void BeXmlCGWriter::WriteText (BeXmlWriterR dest, Utf8CP name, wchar_t const *data)
    {
    dest.WriteElementStart (name);
    dest.WriteText (data);
    dest.WriteElementEnd (); 
    }


void BeXmlCGWriter::WriteXY (BeXmlWriterR dest, Utf8CP name, double x, double y)
    {
    wchar_t buffer[1024];
    BeStringUtilities::Snwprintf (buffer, _countof (buffer), L"%.15g,%.15g,%.15g", x, y);
    dest.WriteElementStart (name);
    dest.WriteText (buffer);
    dest.WriteElementEnd (); 
    }

void BeXmlCGWriter::WriteXYZ (BeXmlWriterR dest, Utf8CP name, double x, double y, double z)
    {
    wchar_t buffer[1024];
    BeStringUtilities::Snwprintf (buffer, _countof (buffer), L"%.15g,%.15g,%.15g", x, y, z);
    dest.WriteElementStart (name);
    dest.WriteText (buffer);
    dest.WriteElementEnd (); 
    }

void BeXmlCGWriter::WriteDouble (BeXmlWriterR dest, Utf8CP name, double data)
    {
    wchar_t buffer[1024];
    BeStringUtilities::Snwprintf (buffer, _countof (buffer), L"%.15g", data);
    dest.WriteElementStart (name);
    dest.WriteText (buffer);
    dest.WriteElementEnd (); 
    }

void BeXmlCGWriter::WriteInt (BeXmlWriterR dest, Utf8CP name, int data)
    {
    wchar_t buffer[1024];
    BeStringUtilities::Snwprintf (buffer, _countof (buffer), L"%d", data);
    dest.WriteElementStart (name);
    dest.WriteText (buffer);
    dest.WriteElementEnd (); 
    }


void BeXmlCGWriter::WriteBool (BeXmlWriterR dest, Utf8CP name, bool data)
    {
    dest.WriteElementStart (name);
    if (data)
        dest.WriteText (L"true");
    else
        dest.WriteText (L"false");
    dest.WriteElementEnd (); 
    }


void BeXmlCGWriter::WriteSegment (BeXmlWriterR dest, DSegment3dCR data)
    {
    dest.WriteElementStart ("LineSegment");
    WriteXYZ (dest, "startPoint", data.point[0]);
    WriteXYZ (dest, "endPoint", data.point[1]);    
    dest.WriteElementEnd ();
    }

void BeXmlCGWriter::WriteArc (BeXmlWriterR dest, DEllipse3dCR arc)
    {
    if (arc.IsCircular ())
        {
        DEllipse3d majorMinorArc = DEllipse3d::FromPerpendicularAxes (arc);
        dest.WriteElementStart ("CircularArc");
        WritePlacementXY (dest,  majorMinorArc.center, majorMinorArc.vector0, majorMinorArc.vector90);
        WriteDouble (dest, "radius",     majorMinorArc.vector0.Magnitude ());
        WriteDouble (dest, "startAngle",  Angle::RadiansToDegrees (majorMinorArc.start));
        WriteDouble (dest, "sweepAngle",    Angle::RadiansToDegrees (majorMinorArc.sweep));
        dest.WriteElementEnd ();
        }
    else
        {
        DEllipse3d majorMinorArc = DEllipse3d::FromPerpendicularAxes (arc);
        dest.WriteElementStart ("EllipticArc");
        WritePlacementXY (dest,  majorMinorArc.center, majorMinorArc.vector0, majorMinorArc.vector90);
        WriteDouble (dest, "radiusA",     majorMinorArc.vector0.Magnitude ());
        WriteDouble (dest, "radiusB",     majorMinorArc.vector90.Magnitude ());
        WriteDouble (dest, "startAngle",  Angle::RadiansToDegrees (majorMinorArc.start));
        WriteDouble (dest, "sweepAngle",    Angle::RadiansToDegrees (majorMinorArc.sweep));
        dest.WriteElementEnd ();
        }
    }

void BeXmlCGWriter::WriteDisk (BeXmlWriterR dest, DEllipse3dCR arc)
    {
    double direction = arc.sweep > 0.0 ? 1.0 : -1.0;
    DEllipse3d majorMinorArc = DEllipse3d::FromPerpendicularAxes (arc);
    DVec3d yVector;
    yVector.Scale (majorMinorArc.vector90, direction);
    if (arc.IsCircular ())
        {
        dest.WriteElementStart ("CircularDisk");
        WritePlacementXY (dest, majorMinorArc.center, majorMinorArc.vector0, yVector);
        WriteDouble (dest, "radius",     majorMinorArc.vector0.Magnitude ());
        dest.WriteElementEnd ();
        }
    else
        {
        dest.WriteElementStart ("EllipticDisk");
        WritePlacementXY (dest, majorMinorArc.center, majorMinorArc.vector0, yVector);
        WriteDouble (dest, "radiusA",     majorMinorArc.vector0.Magnitude ());
        WriteDouble (dest, "radiusB",     majorMinorArc.vector90.Magnitude ());
        dest.WriteElementEnd ();
        }
    }


void BeXmlCGWriter::WritePolyface (BeXmlWriterR dest, PolyfaceHeader &mesh)
    {
    dest.WriteElementStart ("IndexedMesh");
    WriteList (dest, mesh.Point (), "ListOfCoord", "xyz");

    if (mesh.PointIndex ().Active ())
        WriteIndexList (dest, mesh.PointIndex (), "ListOfCoordIndex", "id");

    if (mesh.Param ().Active ())
        WriteList (dest, mesh.Param (), "ListOfParam", "uv");
    if (mesh.ParamIndex ().Active ())
        WriteIndexList (dest, mesh.ParamIndex (), "ListOfParamIndex", "id");

    if (mesh.Normal ().Active ())
        WriteList (dest, mesh.Normal (), "ListOfNormal", "normal");
    if (mesh.NormalIndex ().Active ())
        WriteIndexList (dest, mesh.NormalIndex (), "ListOfNormalIndex", "id");

    if (mesh.DoubleColor ().Active ())
        WriteList (dest, mesh.DoubleColor (), "ListOfColor", "Color");
    if (mesh.ColorIndex ().Active ())
        WriteIndexList (dest, mesh.ColorIndex (), "ListOfColorIndex", "id");

    dest.WriteElementEnd ();
    }

void BeXmlCGWriter::WriteCurve (BeXmlWriterR dest, MSBsplineCurveCR curve)
    {
    dest.WriteElementStart ("BsplineCurve");

    WriteInt (dest, "Order", curve.params.order);

    WriteBool (dest, "Closed", curve.params.closed? true : false);

    bvector<DPoint3d> poles;
    poles.assign (curve.poles, curve.poles + curve.NumberAllocatedPoles ());
    WriteList (dest, poles, "ListOfControlPoint", "ControlPoint");
    
    if (curve.rational)
        {
        bvector<double> weights;
        weights.assign (curve.weights, curve.weights + curve.NumberAllocatedPoles ());
        WriteList (dest, weights, "ListOfWeight", "Weight");
        }

    bvector<double> knots;
    knots.assign (curve.knots, curve.knots + curve.NumberAllocatedKnots ());
    WriteList (dest, knots, "ListOfKnot", "Knot");

    dest.WriteElementEnd ();
    }


void BeXmlCGWriter::WriteSurface (BeXmlWriterR dest, MSBsplineSurfaceCR surface)
    {
    dest.WriteElementStart ("BsplineSurface");

    WriteInt (dest, "orderU", surface.uParams.order);
    WriteInt (dest, "numUControlPoint", surface.uParams.numPoles);
    WriteBool (dest, "closedU", surface.uParams.closed ? true : false);

    WriteInt (dest, "orderV", surface.vParams.order);
    WriteInt (dest, "numVControlPoint", surface.vParams.numPoles);
    WriteBool (dest, "closedV", surface.vParams.closed ? true : false);

    size_t totalPoles = (size_t)surface.uParams.numPoles * (size_t)surface.vParams.numPoles;
    bvector<DPoint3d> poles;
    poles.assign (surface.poles, surface.poles + totalPoles);
    WriteList (dest, poles, "ListOfControlPoint", "ControlPoint");
    
    if (surface.rational)
        {
        bvector<double> weights;
        weights.assign (surface.weights, surface.weights + totalPoles);
        WriteList (dest, weights, "ListOfWeight", "Weight");
        }

    bvector<double> uknots;
    int numUKnots = surface.uParams.NumberAllocatedKnots ();
    uknots.assign (surface.uKnots, surface.uKnots + numUKnots);
    WriteList (dest, uknots, "ListOfKnotU", "KnotU");

    bvector<double> vknots;
    int numVKnots = surface.vParams.NumberAllocatedKnots ();
    vknots.assign (surface.vKnots, surface.vKnots + numVKnots);
    WriteList (dest, vknots, "ListOfKnotV", "KnotV");

    dest.WriteElementEnd ();
    }


void BeXmlCGWriter::WriteTextPlacement
(
BeXmlWriterR dest,
DPoint3dCR xyz,
wchar_t const *data,
double charSize
)
    {
    dest.WriteElementStart ("SingleLineText");
    WritePlacementZX (dest, xyz, DVec3d::From (0,0,1), DVec3d::From (1,0,0));
    Indent (dest); WriteText (dest, "textString", data);
    Indent (dest); WriteText (dest, "fontName", L"ARIAL");
    Indent (dest); WriteDouble (dest, "characterXSize", charSize);
    Indent (dest); WriteDouble (dest, "characterYSize", charSize);
    Indent (dest); WriteDouble (dest, "slantAngle", 0.0);
    dest.WriteElementEnd ();
    }


void BeXmlCGWriter::WriteLineString (BeXmlWriterR dest, bvector<DPoint3d> const &points)
    {
    dest.WriteElementStart ("LineString");
    WriteList (dest, points, "ListOfPoint", "xyz");
    dest.WriteElementEnd ();
    }

void BeXmlCGWriter::WritePolygon (BeXmlWriterR dest, bvector<DPoint3d> const &points)
    {
    dest.WriteElementStart ("Polygon");
    WriteList (dest, points, "ListOfPoint", "xyz");
    dest.WriteElementEnd ();
    }


void BeXmlCGWriter::Write (BeXmlWriterR dest, ICurvePrimitiveCR curve)
    {
    switch (curve.GetCurvePrimitiveType ())
        {
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line:
            WriteSegment (dest, *curve.GetLineCP ());
            break;

        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString:
            WriteLineString (dest, *curve.GetLineStringCP ());
            break;

        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc:
            WriteArc (dest, *curve.GetArcCP ());
            break;

        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_BsplineCurve:
            WriteCurve (dest, *curve.GetBsplineCurveCP ());
            break;

        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_CurveVector:
            Write (dest, *curve.GetChildCurveVectorCP ());
            break;
        default:
            return;
        }
    }


void BeXmlCGWriter::Write (BeXmlWriterR dest, CurveVectorCR curveVector)
    {
    Write (dest, curveVector, true);
    }

void BeXmlCGWriter::Write (BeXmlWriterR dest, CurveVectorCR curveVector, bool preferMostCompactPrimitives)
    {
    switch (curveVector.GetBoundaryType ())
        {
        case CurveVector::BOUNDARY_TYPE_Outer:
        case CurveVector::BOUNDARY_TYPE_Inner:
            {
            DEllipse3d arc;
            bvector<DPoint3d> const *points;
            if (preferMostCompactPrimitives
                && curveVector.size () == 1
                && curveVector[0]->TryGetArc (arc)
                && arc.IsFullEllipse ())
                {
                WriteDisk (dest, arc);
                }
            else if (preferMostCompactPrimitives
                && curveVector.size () == 1
                && NULL != (points = curveVector[0]->GetLineStringCP ())
                )
                {
                WritePolygon (dest, *points);
                }
            else
                {
                dest.WriteElementStart ("CurveChain");
                    dest.WriteElementStart ("ListOfCurve");
                    FOR_EACH(ICurvePrimitivePtr curve , curveVector)
                        Write (dest, *curve);
                    dest.WriteElementEnd ();
                dest.WriteElementEnd ();
                }
            break;
            }

        case CurveVector::BOUNDARY_TYPE_Open:
            {
            if (preferMostCompactPrimitives && curveVector.size () == 1)
                {
                Write (dest, *curveVector[0]);
                }
            else
                {
                dest.WriteElementStart ("CurveChain");
                    dest.WriteElementStart ("ListOfCurve");
                    FOR_EACH(ICurvePrimitivePtr curve , curveVector)
                        Write (dest, *curve);
                    dest.WriteElementEnd ();
                dest.WriteElementEnd ();
                }
            break;
            }


        case CurveVector::BOUNDARY_TYPE_ParityRegion:
            {
            dest.WriteElementStart ("SurfacePatch");
            bvector <ICurvePrimitivePtr> holeLoop;
            FOR_EACH(ICurvePrimitivePtr curve , curveVector)
                {
                if (NULL != curve->GetChildCurveVectorCP ())
                    {
                    switch (curve->GetChildCurveVectorCP ()->GetBoundaryType ())
                        {
                        case CurveVector::BOUNDARY_TYPE_Outer:
                            {
                            dest.WriteElementStart ("ExteriorLoop");
                                Write (dest, *curve->GetChildCurveVectorCP (), false);
                            dest.WriteElementEnd ();

                            break;
                            }

                        case CurveVector::BOUNDARY_TYPE_Inner:
                            {
                            holeLoop.push_back (curve);
                            break;
                            }

                        default:
                            return;
                        }
                    }
                }

            dest.WriteElementStart ("ListOfHoleLoop");
            for (size_t i=0; i<holeLoop.size (); i++)
                Write (dest, *holeLoop[i]->GetChildCurveVectorCP (), false);
            dest.WriteElementEnd ();
            dest.WriteElementEnd ();
            break;
            }
        
        case CurveVector::BOUNDARY_TYPE_UnionRegion:
            {
            dest.WriteElementStart ("SurfaceGroup");
            FOR_EACH(ICurvePrimitivePtr curve , curveVector)
                Write (dest, *curve->GetChildCurveVectorCP ());
            dest.WriteElementEnd ();
            
            break;
            }

        case CurveVector::BOUNDARY_TYPE_None:
            {
            dest.WriteElementStart ("Group");
            FOR_EACH(ICurvePrimitivePtr curve , curveVector)
                Write (dest, *curve);
            dest.WriteElementEnd ();
            
            break;
            }
        default:
            return;
        }
    }



void BeXmlCGWriter::WriteDgnTorusPipeDetail (BeXmlWriterR dest, DgnTorusPipeDetail data)
    {
    DPoint3d center;
    RotMatrix axes;
    double radiusA, radiusB, sweepAngle;
    if (data.TryGetFrame (center, axes, radiusA, radiusB, sweepAngle))
        {
        dest.WriteElementStart ("TorusPipe");
        WritePlacementZX (dest, center, axes);
        WriteDouble (dest, "radiusA", radiusA);
        WriteDouble (dest, "radiusB", radiusB);
        WriteDouble (dest, "startAngle", 0.0);
        WriteDouble (dest, "sweepAngle", Angle::RadiansToDegrees (sweepAngle));
        dest.WriteElementEnd ();
        }
    }

void BeXmlCGWriter::WriteDgnConeDetail (BeXmlWriterR dest, DgnConeDetail data)
    {
    DPoint3d centerA, centerB;
    RotMatrix axes;
    double radiusA, radiusB;
    bool capped;
    if (data.IsCircular (centerA, centerB, axes, radiusA, radiusB, capped))
        {
        DVec3d unitX, unitY, unitZ;
        axes.GetColumns (unitX, unitY, unitZ);
        double height = centerB.DotDifference (centerA, unitZ);
        double distance = centerA.Distance (centerB);
        if (DoubleOps::AlmostEqual (height, distance))
            {
            if (DoubleOps::AlmostEqual (radiusA, radiusB))
                {
                dest.WriteElementStart ("CircularCylinder");
                WritePlacementZX (dest, centerA, unitZ, unitX);
                WriteDouble (dest, "radius", radiusA);
                WriteDouble (dest, "height", height);
                WriteBool (dest, "bSolidFlag", data.m_capped);
                dest.WriteElementEnd ();
                }
            else
                {
                dest.WriteElementStart ("CircularCone");
                WritePlacementZX (dest, centerA, unitZ, unitX);
                WriteDouble (dest, "radiusA", radiusA);
                WriteDouble (dest, "radiusB", radiusB);
                WriteDouble (dest, "height", height);
                WriteBool (dest, "bSolidFlag", data.m_capped);
                dest.WriteElementEnd ();
                }
            }
        else
            {
            dest.WriteElementStart ("SkewedCone");
            WritePlacementZX (dest, centerA, unitZ, unitX);
            WriteXYZ (dest, "centerB", centerB);
            WriteDouble (dest, "radiusA", radiusA);
            WriteDouble (dest, "radiusB", radiusB);
            WriteBool (dest, "bSolidFlag", data.m_capped);
            dest.WriteElementEnd ();
            }
        }
    }


void BeXmlCGWriter::WriteDgnBoxDetail (BeXmlWriterR dest, DgnBoxDetail data)
    {
    }

void BeXmlCGWriter::WriteDgnSphereDetail (BeXmlWriterR dest, DgnSphereDetail data)
    {
    DPoint3d center;
    RotMatrix axes;
    double radius;
    if (data.IsTrueSphere (center, axes, radius))
        {
        dest.WriteElementStart ("Sphere");
        DVec3d unitX, unitY, unitZ;
        axes.GetColumns (unitX, unitY, unitZ);
        WritePlacementZX (dest, center, unitZ, unitX);
        WriteDouble (dest, "radius", radius);
        dest.WriteElementEnd ();
        }
    }

void BeXmlCGWriter::WriteDgnExtrusionDetail (BeXmlWriterR dest, DgnExtrusionDetail data)
    {
    DPoint3d curveStart, curveEnd;
    if (data.m_baseCurve->GetStartEnd (curveStart, curveEnd))
        {
        dest.WriteElementStart (data.m_capped ? "SolidBySweptCurve" : "SurfaceBySweptCurve");
            dest.WriteElementStart ("BaseGeometry");
            Write (dest, *data.m_baseCurve);
            dest.WriteElementEnd ();
            DSegment3d segment;
            segment.point[0] = curveStart;
            segment.point[1].SumOf (curveStart, data.m_extrusionVector);
            dest.WriteElementStart ("RailCurve");
            WriteSegment (dest, segment);
            dest.WriteElementEnd ();
        dest.WriteElementEnd ();
        }
    }

static DEllipse3d BuildSweepArc
(
CurveVectorCR curve,
TransformCR localToWorld,
TransformCR worldToLocal,
double sweepAngle
)
    {
    static const double s_defaultRadius = 1000.0;
    bvector<DPoint3d>strokes;
    IFacetOptionsPtr options = IFacetOptions::Create ();
    options->SetAngleTolerance (Angle::Pi () / 6.0);
    curve.AddStrokePoints (strokes, *options);
    DEllipse3d localEllipse, worldEllipse;
    // Candidate local point on the ellipse ...
    DPoint3d xyz0 = DPoint3d::From (s_defaultRadius, 0, 0);
    if (strokes.size () > 0)
        {
        DPoint3d xyzMax = DPoint3d::FromZero();
        double r2Max = 0.0;
        for (size_t i = 1; i < strokes.size (); i++)
            {
            DPoint3d xyz;
            worldToLocal.Multiply (xyz, strokes[i]);
            double r2 = xyz.MagnitudeSquaredXY ();
            if (r2 > r2Max)
                {
                r2Max = r2;
                xyzMax = xyz;
                }
            }
        if (r2Max > 0.0)
            xyz0 = xyzMax;
        }

    localEllipse = DEllipse3d::From (
                0,0, xyz0.z,
                xyz0.x, xyz0.y, 0.0,
                -xyz0.y, xyz0.x, 0.0,
                0.0, sweepAngle
                );
    localToWorld.Multiply (worldEllipse, localEllipse);
    return worldEllipse;
    }


void BeXmlCGWriter::WriteDgnRotationalSweepDetail (BeXmlWriterR dest, DgnRotationalSweepDetail data)
    {
    Transform localToWorld, worldToLocal;    
    if (data.GetTransforms (localToWorld, worldToLocal))
        {
        dest.WriteElementStart (data.m_capped ? "SolidBySweptCurve" : "SurfaceBySweptCurve");
            dest.WriteElementStart ("BaseGeometry");
            Write (dest, *data.m_baseCurve);
            dest.WriteElementEnd ();
            DEllipse3d arc = BuildSweepArc (*data.m_baseCurve, localToWorld, worldToLocal, data.m_sweepAngle);
            dest.WriteElementStart ("RailCurve");
            WriteArc (dest, arc);
            dest.WriteElementEnd ();
        dest.WriteElementEnd ();
        }
    }

void BeXmlCGWriter::WriteDgnRuledSweepDetail (BeXmlWriterR dest, DgnRuledSweepDetail data)
    {
    }


void BeXmlCGWriter::Write (BeXmlWriterR dest, ISolidPrimitiveR primitive)
    {
    switch (primitive.GetSolidPrimitiveType ())
        {
        case SolidPrimitiveType_DgnTorusPipe:
            {
            DgnTorusPipeDetail detail;
            if (primitive.TryGetDgnTorusPipeDetail (detail))
                WriteDgnTorusPipeDetail (dest, detail);
            }
            break;
        case SolidPrimitiveType_DgnCone:
            {
            DgnConeDetail detail;
            if (primitive.TryGetDgnConeDetail (detail))
                WriteDgnConeDetail (dest, detail);
            }
            break;
        case SolidPrimitiveType_DgnBox:
            {
            DgnBoxDetail detail;
            if (primitive.TryGetDgnBoxDetail (detail))
                WriteDgnBoxDetail (dest, detail);
            }
            break;
        case SolidPrimitiveType_DgnSphere:
            {
            DgnSphereDetail detail;
            if (primitive.TryGetDgnSphereDetail (detail))
                WriteDgnSphereDetail (dest, detail);
            }
            break;
        case SolidPrimitiveType_DgnExtrusion:
            {
            DgnExtrusionDetail detail;
            if (primitive.TryGetDgnExtrusionDetail (detail))
                WriteDgnExtrusionDetail (dest, detail);
            }
            break;
        case SolidPrimitiveType_DgnRotationalSweep:
            {
            DgnRotationalSweepDetail detail;
            if (primitive.TryGetDgnRotationalSweepDetail (detail))
                WriteDgnRotationalSweepDetail (dest, detail);
            }
            break;
        case SolidPrimitiveType_DgnRuledSweep:
            {
            DgnRuledSweepDetail detail;
            if (primitive.TryGetDgnRuledSweepDetail (detail))
                WriteDgnRuledSweepDetail (dest, detail);
            }
            break;
        }
    }


void BeXmlCGWriter::Write (BeXmlWriterR dest, IGeometryPtr geometry)
    {
    CurveVectorPtr cv;
    if (geometry.TryGetAs (cv))
        {
        Write (dest, *cv);
        return;
        }

    ICurvePrimitivePtr cp;
    if (geometry.TryGetAs (cp))
        {
        Write (dest, *cp);
        return;
        }

    MSBsplineCurvePtr bcurve;
    if (geometry.TryGetAs (bcurve))
        {
        WriteCurve (dest, *bcurve);
        return;
        }
    
    ISolidPrimitivePtr sp;
    if (geometry.TryGetAs (sp))
        {
        Write (dest, *sp);
        return;
        }
    
    MSBsplineSurfacePtr bsurface;
    if (geometry.TryGetAs (bsurface))
        {
        WriteSurface (dest, *bsurface);
        return;
        }
    }
END_BENTLEY_ECOBJECT_NAMESPACE
