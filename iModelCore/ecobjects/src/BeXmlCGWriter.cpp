/*--------------------------------------------------------------------------------------+
|
|  $Source: src/BeXmlCGWriter.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"
#include "MSXmlBinary\MSXmlBinaryWriter.h"

BEGIN_BENTLEY_ECOBJECT_NAMESPACE
static const double s_nearZero = 9.0e-16;
static const size_t s_maxIndexPerLine = 5;

static double SuppressNearZeroCoordinate (double x, double refValue)
    {
    double tol = s_nearZero * refValue;
    return fabs (x) < tol ? 0.0 : x;
    }

static void Indent (IBeXmlWriterR dest)
    {
    }

void BeXmlCGWriter::WriteList (IBeXmlWriterR dest, bvector<DPoint3d> const & data, Utf8CP listName, Utf8CP itemName)
    {
    if (0 == data.size())
        return;

    dest.WriteElementStart (listName);
    for (size_t i = 0 ; i < data.size (); i++)
        WriteXYZ (dest, itemName, data[i]);
    dest.WriteElementEnd ();
    }

void BeXmlCGWriter::WriteList (IBeXmlWriterR dest, bvector<DVec3d> const & data, Utf8CP listName, Utf8CP itemName)
    {
    if (0 == data.size())
        return;

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

void BeXmlCGWriter::WriteList (IBeXmlWriterR dest, bvector<RgbFactor> const & data, Utf8CP listName, Utf8CP itemName)
    {
    if (0 == data.size())
        return;

    dest.WriteElementStart (listName);
    for (size_t i = 0 ; i < data.size (); i++)
        {
        Indent (dest);
        WriteXYZ (dest, itemName, data[i].red, data[i].green, data[i].blue);
        }
    dest.WriteElementEnd ();
    }

void BeXmlCGWriter::WriteList (IBeXmlWriterR dest, bvector<DPoint2d> const & data, Utf8CP listName, Utf8CP itemName)
    {
    if (0 == data.size())
        return;

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


void BeXmlCGWriter::WriteIndexList (IBeXmlWriterR dest, bvector<int> const & data, Utf8CP listName, Utf8CP itemName)
    {
    if (0 == data.size())
        return;

    BeXmlStatus s = dest.WriteElementStart (listName);
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
    s = dest.WriteElementEnd ();
    }

void BeXmlCGWriter::WriteList (IBeXmlWriterR dest, bvector<double> const & data, Utf8CP listName, Utf8CP itemName)
    {
    if (0 == data.size())
        return;

    dest.WriteElementStart (listName);
    for (size_t i = 0 ; i < data.size (); i++)
        WriteDouble (dest, itemName, data[i]);
    dest.WriteElementEnd ();
    }

void BeXmlCGWriter::WritePlacementZX
(
IBeXmlWriterR dest,
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
IBeXmlWriterR dest,
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


void BeXmlCGWriter::WritePlacementXY (IBeXmlWriterR dest, DPoint3dCR origin, DVec3dCR vectorX, DVec3dCR vectorY)
    {
    DVec3d unitNormal, unitX;
    unitNormal.NormalizedCrossProduct (vectorX, vectorY);
    unitX.NormalizedCrossProduct (vectorY, unitNormal);
    WritePlacementZX (dest, origin, unitNormal, unitX);
    }

void BeXmlCGWriter::WriteXYZ (IBeXmlWriterR dest, Utf8CP name, DPoint3dCR xyz)
    {
    wchar_t buffer[1024];
    BeStringUtilities::Snwprintf (buffer, _countof (buffer), L"%.17G,%.17G,%.17G", xyz.x, xyz.y, xyz.z);
    BeXmlStatus s;
    s = dest.WriteElementStart (name);
    s = dest.WriteText (buffer);
    s = dest.WriteElementEnd (); 
    }

void BeXmlCGWriter::WriteText (IBeXmlWriterR dest, Utf8CP name, wchar_t const *data)
    {
    dest.WriteElementStart (name);
    dest.WriteText (data);
    dest.WriteElementEnd (); 
    }


void BeXmlCGWriter::WriteXY (IBeXmlWriterR dest, Utf8CP name, double x, double y)
    {
    wchar_t buffer[1024];
    BeStringUtilities::Snwprintf (buffer, _countof (buffer), L"%.17G,%.17G", x, y);
    dest.WriteElementStart (name);
    dest.WriteText (buffer);
    dest.WriteElementEnd (); 
    }

void BeXmlCGWriter::WriteXYZ (IBeXmlWriterR dest, Utf8CP name, double x, double y, double z)
    {
    wchar_t buffer[1024];
    BeStringUtilities::Snwprintf (buffer, _countof (buffer), L"%.17G,%.17G,%.17G", x, y, z);
    dest.WriteElementStart (name);
    dest.WriteText (buffer);
    dest.WriteElementEnd (); 
    }

void BeXmlCGWriter::WriteDouble (IBeXmlWriterR dest, Utf8CP name, double data)
    {
    dest.WriteElementStart (name);
    MSXmlBinaryWriter* binWriter = dynamic_cast<MSXmlBinaryWriter*>(&dest);
    if (nullptr == binWriter)
        {
        wchar_t buffer[1024];
        BeStringUtilities::Snwprintf (buffer, _countof (buffer), L"%.17G", data);
        dest.WriteText (buffer);
        }
    else
        binWriter->WriteDoubleText(data);
    dest.WriteElementEnd (); 
    }

void BeXmlCGWriter::WriteInt (IBeXmlWriterR dest, Utf8CP name, int data)
    {
    dest.WriteElementStart (name);
    MSXmlBinaryWriter* binWriter = dynamic_cast<MSXmlBinaryWriter*>(&dest);
    if (nullptr == binWriter)
        {
        wchar_t buffer[1024];
        BeStringUtilities::Snwprintf (buffer, _countof (buffer), L"%d", data);
        dest.WriteText (buffer);
        }
    else
        binWriter->WriteInt32Text(data);
    dest.WriteElementEnd (); 
    }


void BeXmlCGWriter::WriteBool (IBeXmlWriterR dest, Utf8CP name, bool data)
    {
    dest.WriteElementStart (name);
    MSXmlBinaryWriter* binWriter = dynamic_cast<MSXmlBinaryWriter*>(&dest);
    if (nullptr == binWriter)
        {
        if (data)
            dest.WriteText (L"true");
        else
            dest.WriteText (L"false");
        }
    else
        binWriter->WriteBoolText(data);
    dest.WriteElementEnd (); 
    }


void BeXmlCGWriter::WriteSegment (IBeXmlWriterR dest, DSegment3dCR data)
    {
    dest.WriteElementStart ("LineSegment");
    WriteXYZ (dest, "startPoint", data.point[0]);
    WriteXYZ (dest, "endPoint", data.point[1]);    
    dest.WriteElementEnd ();
    }

void BeXmlCGWriter::WriteArc (IBeXmlWriterR dest, DEllipse3dCR arc)
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

void BeXmlCGWriter::WriteDisk (IBeXmlWriterR dest, DEllipse3dCR arc)
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


void BeXmlCGWriter::WritePolyface (IBeXmlWriterR dest, PolyfaceHeader &mesh)
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

void BeXmlCGWriter::WriteCurve (IBeXmlWriterR dest, MSBsplineCurveCR curve)
    {
    dest.WriteElementStart ("BsplineCurve");

    WriteInt (dest, "Order", curve.params.order);

    WriteBool (dest, "Closed", curve.params.closed? true : false);

    bvector<DPoint3d> poles;
    poles.assign (curve.poles, curve.poles + curve.NumberAllocatedPoles ());
    WriteList (dest, poles, "ListOfControlPoint", "xyz");
    
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


void BeXmlCGWriter::WriteSurface (IBeXmlWriterR dest, MSBsplineSurfaceCR surface)
    {
    dest.WriteElementStart ("BsplineSurface");

    WriteInt (dest, "OrderU", surface.uParams.order);
    WriteBool (dest, "ClosedU", surface.uParams.closed ? true : false);
    WriteInt (dest, "NumUControlPoint", surface.uParams.numPoles);

    WriteInt (dest, "OrderV", surface.vParams.order);
    WriteBool (dest, "ClosedV", surface.vParams.closed ? true : false);
    WriteInt (dest, "NumVControlPoint", surface.vParams.numPoles);

    size_t totalPoles = (size_t)surface.uParams.numPoles * (size_t)surface.vParams.numPoles;
    bvector<DPoint3d> poles;
    poles.assign (surface.poles, surface.poles + totalPoles);
    WriteList (dest, poles, "ListOfControlPoint", "xyz");
    
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
IBeXmlWriterR dest,
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


void BeXmlCGWriter::WriteLineString (IBeXmlWriterR dest, bvector<DPoint3d> const &points)
    {
    dest.WriteElementStart ("LineString");
    WriteList (dest, points, "ListOfPoint", "xyz");
    dest.WriteElementEnd ();
    }

void BeXmlCGWriter::WriteCoordinate (IBeXmlWriterR dest, DPoint3dCR point)
    {
    dest.WriteElementStart ("Coordinate");
    WriteXYZ (dest, "xyz", point);
    dest.WriteElementEnd ();    
    }

void BeXmlCGWriter::WritePointString (IBeXmlWriterR dest, bvector<DPoint3d> const &points, bool preferMostCompactPrimitives)
    {
    if (points.size () == 1 && preferMostCompactPrimitives)
        {
        WriteCoordinate (dest, points[0]);
        }
    else
        {
        dest.WriteElementStart ("PointChain");
        dest.WriteElementStart ("ListOfPoint");
        for (size_t i = 0; i < points.size (); i++)
            WriteCoordinate (dest, points[0]);
        dest.WriteElementEnd ();
        dest.WriteElementEnd ();
        }
    }


void BeXmlCGWriter::WritePolygon (IBeXmlWriterR dest, bvector<DPoint3d> const &points)
    {
    dest.WriteElementStart ("Polygon");
    WriteList (dest, points, "ListOfPoint", "xyz");
    dest.WriteElementEnd ();
    }


void BeXmlCGWriter::Write (IBeXmlWriterR dest, ICurvePrimitiveCR curve, bool preferMostCompactPrimitives)
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

        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_PointString:
            WritePointString (dest, *curve.GetPointStringCP (), preferMostCompactPrimitives);
            break;

        default:
            return;
        }
    }

void BeXmlCGWriter::Write (IBeXmlWriterR dest, ICurvePrimitiveCR curve)
    {
    Write (dest, curve, true);
    }

void BeXmlCGWriter::Write (IBeXmlWriterR dest, CurveVectorCR curveVector)
    {
    Write (dest, curveVector, true);
    }

void BeXmlCGWriter::Write (IBeXmlWriterR dest, CurveVectorCR curveVector, bool preferMostCompactPrimitives)
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
                    for(ICurvePrimitivePtr curve : curveVector)
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
                    for(ICurvePrimitivePtr curve : curveVector)
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
            int n = 0;
            for(ICurvePrimitivePtr curve : curveVector)
                {
                // ASSUME first child is outer loop...
                if (NULL != curve->GetChildCurveVectorCP ())
                    {
                    if (n++ == 0)
                        {
                        dest.WriteElementStart ("ExteriorLoop");
                        Write (dest, *curve->GetChildCurveVectorCP (), false);
                        dest.WriteElementEnd ();
                        }
                    else
                        {
                        holeLoop.push_back (curve);
                        }
                    }
                }

            if (holeLoop.size() > 0)
                {
                dest.WriteElementStart ("ListOfHoleLoop");
                for (size_t i=0; i<holeLoop.size (); i++)
                    Write (dest, *holeLoop[i]->GetChildCurveVectorCP (), false);
                dest.WriteElementEnd ();
                }
            dest.WriteElementEnd ();
            break;
            }
        
        case CurveVector::BOUNDARY_TYPE_UnionRegion:
            {
            dest.WriteElementStart ("SurfaceGroup");
            for(ICurvePrimitivePtr curve : curveVector)
                Write (dest, *curve->GetChildCurveVectorCP ());
            dest.WriteElementEnd ();
            
            break;
            }

        case CurveVector::BOUNDARY_TYPE_None:
            {
            dest.WriteElementStart ("Group");
            for(ICurvePrimitivePtr curve : curveVector)
                Write (dest, *curve);
            dest.WriteElementEnd ();
            
            break;
            }
        default:
            return;
        }
    }



void BeXmlCGWriter::WriteDgnTorusPipeDetail (IBeXmlWriterR dest, DgnTorusPipeDetail data)
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
        WriteBool   (dest, "bSolidFlag", data.m_capped);
        dest.WriteElementEnd ();
        }
    }

void BeXmlCGWriter::WriteDgnConeDetail (IBeXmlWriterR dest, DgnConeDetail data)
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
                WriteDouble (dest, "height", height);
                WriteDouble (dest, "radius", radiusA);
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


void BeXmlCGWriter::WriteDgnBoxDetail (IBeXmlWriterR dest, DgnBoxDetail data)
    {
    Transform localToWorld;
    double ax, ay, bx, by;
    data.GetNonUniformTransform (localToWorld, ax, ay, bx, by);
    DPoint3d origin;
    DVec3d xVector, yVector, zVector;
    localToWorld.GetOriginAndVectors (origin, xVector, yVector, zVector);
    if (xVector.IsPerpendicularTo (yVector)
        && xVector.IsPerpendicularTo (zVector)
        && yVector.IsPerpendicularTo (zVector)
        && DoubleOps::AlmostEqual (ax, bx)
        && DoubleOps::AlmostEqual (ay, by)
        )
        {
        dest.WriteElementStart ("Block");
        DVec3d unitZ, unitX;
        unitZ.Normalize (zVector);
        unitX.Normalize (xVector);
        WritePlacementZX (dest, origin, unitZ, unitX);
        WriteXYZ (dest, "cornerA", DPoint3d::From (0,0,0));
        WriteXYZ (dest, "cornerB", DPoint3d::From (ax, ay, zVector.Magnitude ()));
        WriteBool (dest, "bSolidFlag", data.m_capped);
        dest.WriteElementEnd ();
        }
    else
        {
        // ugh .. it's not a block.  make a ruled surface.
        bvector<DPoint3d>corners;
        data.GetCorners (corners);
        DPoint3d cornerA[5] = {corners[0], corners[1], corners[3], corners[2], corners[0]};
        DPoint3d cornerB[5] = {corners[4], corners[5], corners[7], corners[6], corners[4]};
        CurveVectorPtr sectionA = CurveVector::CreateLinear (cornerA, 5, CurveVector::BOUNDARY_TYPE_Outer, false);
        CurveVectorPtr sectionB = CurveVector::CreateLinear (cornerB, 5, CurveVector::BOUNDARY_TYPE_Outer, false);
        DgnRuledSweepDetail ruledSurface (sectionA, sectionB, data.m_capped);
        WriteDgnRuledSweepDetail (dest, ruledSurface);
        }                
    }

void BeXmlCGWriter::WriteDgnSphereDetail (IBeXmlWriterR dest, DgnSphereDetail data)
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

void BeXmlCGWriter::WriteDgnExtrusionDetail (IBeXmlWriterR dest, DgnExtrusionDetail data)
    {
    DPoint3d curveStart, curveEnd;
    if (data.m_baseCurve->GetStartEnd (curveStart, curveEnd))
        {
        dest.WriteElementStart (data.m_capped ? "SolidBySweptSurface" : "SurfaceBySweptCurve");
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


void BeXmlCGWriter::WriteDgnRotationalSweepDetail (IBeXmlWriterR dest, DgnRotationalSweepDetail data)
    {
    Transform localToWorld, worldToLocal;    
    if (data.GetTransforms (localToWorld, worldToLocal))
        {
        dest.WriteElementStart (data.m_capped ? "SolidBySweptSurface" : "SurfaceBySweptCurve");
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

void BeXmlCGWriter::WriteDgnRuledSweepDetail (IBeXmlWriterR dest, DgnRuledSweepDetail data)
    {
    }


void BeXmlCGWriter::Write (IBeXmlWriterR dest, ISolidPrimitiveR primitive)
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


void BeXmlCGWriter::Write (IBeXmlWriterR dest, IGeometryPtr geometry)
    {
    if (!geometry.IsValid ())
        return;

    switch (geometry->GetGeometryType ())
        {
        case IGeometry::GeometryType::CurvePrimitive:
            {
            ICurvePrimitivePtr curvePrimitive = geometry->GetAsICurvePrimitive ();

            Write (dest, *curvePrimitive);
            return;
            }

        case IGeometry::GeometryType::CurveVector:
            {
            CurveVectorPtr curveVector = geometry->GetAsCurveVector ();

            Write (dest, *curveVector);
            return;
            }

        case IGeometry::GeometryType::SolidPrimitive:
            {
            ISolidPrimitivePtr solidPrimitive = geometry->GetAsISolidPrimitive ();

            Write (dest, *solidPrimitive);
            return;
            }

        case IGeometry::GeometryType::BsplineSurface:
            {
            MSBsplineSurfacePtr bSurface = geometry->GetAsMSBsplineSurface ();

            WriteSurface (dest, *bSurface);
            return;
            }

        case IGeometry::GeometryType::Polyface:
            {
            PolyfaceHeaderPtr polyface = geometry->GetAsPolyfaceHeader ();

            WritePolyface (dest, *polyface);
            return;
            }

        default:
            {
            BeAssert (false);
            return;
            }
        }
    }
    
/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void BeXmlCGWriter::Write(Utf8StringR cgBeXml, ICurvePrimitiveCR curve)
    {
    cgBeXml.clear();
    BeXmlWriterPtr xmlDom = BeXmlWriter::Create();
    Write(*xmlDom.get(), curve);
    xmlDom->ToString(cgBeXml);
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void BeXmlCGWriter::Write(Utf8StringR cgBeXml, CurveVectorCR curve)
    {
    cgBeXml.clear();
    BeXmlWriterPtr xmlDom = BeXmlWriter::Create();
    Write(*xmlDom.get(), curve, false);
    xmlDom->ToString(cgBeXml);
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void BeXmlCGWriter::Write(Utf8StringR cgBeXml, CurveVectorCR curve, bool preferMostCompactPrimitives)
    {
    cgBeXml.clear();
    BeXmlWriterPtr xmlDom = BeXmlWriter::Create();
    Write(*xmlDom.get(), curve, preferMostCompactPrimitives);
    xmlDom->ToString(cgBeXml);
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void BeXmlCGWriter::Write(Utf8StringR cgBeXml, ISolidPrimitiveR data)
    {
    cgBeXml.clear();
    BeXmlWriterPtr xmlDom = BeXmlWriter::Create();
    Write(*xmlDom.get(), data);
    xmlDom->ToString(cgBeXml);
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void BeXmlCGWriter::Write(Utf8StringR cgBeXml, IGeometryPtr data)
    {
    cgBeXml.clear();
    BeXmlWriterPtr xmlDom = BeXmlWriter::Create();
    Write(*xmlDom.get(), data);
    xmlDom->ToString(cgBeXml);
    }

void BeXmlCGWriter::WriteBytes(bvector<byte>& bytes, ICurvePrimitiveCR data)
    {
    MSXmlBinaryWriter* writer = new MSXmlBinaryWriter();
    Write(*writer, data);
    writer->GetBytes(bytes);
    }

void BeXmlCGWriter::WriteBytes(bvector<byte>& bytes, IGeometryPtr data)
    {
#if defined (_WIN32)
    unsigned int oldFormat = _set_output_format(_TWO_DIGIT_EXPONENT);
#endif
    MSXmlBinaryWriter* writer = new MSXmlBinaryWriter();
    Write(*writer, data);
#if defined (_WIN32)
    _set_output_format(oldFormat);
#endif

    writer->GetBytes(bytes);
    delete writer;
    }
END_BENTLEY_ECOBJECT_NAMESPACE
