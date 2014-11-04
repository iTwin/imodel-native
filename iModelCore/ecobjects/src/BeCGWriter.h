/*--------------------------------------------------------------------------------------+
|
|  $Source: src/BeCGWriter.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/


BEGIN_BENTLEY_ECOBJECT_NAMESPACE


static const double s_nearZero = 9.0e-16;
static const size_t s_maxIndexPerLine = 5;

static double SuppressNearZeroCoordinate (double x, double refValue)
    {
    double tol = s_nearZero * refValue;
    return fabs (x) < tol ? 0.0 : x;
    }

static void Indent (IBeXmlWriterR m_dest)
    {
    }
struct BeCGWriter
{
IBeXmlWriterR m_dest;
bool m_textualizeXYData;  // for compatibility with .net

BeCGWriter (IBeXmlWriterR dest, bool textualizeXYData) : m_dest (dest), m_textualizeXYData (textualizeXYData)
    {
    
    }


void WriteXYZ (Utf8CP name, DPoint3dCR xyz, bool shortName = false)
    {
    WriteXYZ (name, xyz.x, xyz.y, xyz.z, shortName);
    }

void WriteText (Utf8CP name, Utf8CP data, bool shortName = false)
    {
    m_dest.WriteNamedText (name, data, shortName);
    }


void WriteXY (Utf8CP name, double x, double y, bool shortName = false)
    {
    if (m_textualizeXYData)
        {
        char buffer[1024];
        BeStringUtilities::Snprintf (buffer, _countof (buffer),
                "%.17G,%.17G", x, y);
        m_dest.WriteNamedText (name, buffer, shortName);
        }
    else
        {
        double xy[3] = {x,y};
        m_dest.WriteBlockedDoubles (name, shortName, (double*)&xy, 3);
        }
    }

void WriteXYZ (Utf8CP name, double x, double y, double z, bool shortName = false)
    {
    if (m_textualizeXYData)
        {
        char buffer[1024];
        BeStringUtilities::Snprintf (buffer, _countof (buffer),
                "%.17G,%.17G,%.17G", x, y, z);
        m_dest.WriteNamedText (name, buffer, shortName);
        }
    else
        {
        double xyz[3] = {x,y,z};
        m_dest.WriteBlockedDoubles (name, shortName, (double*)&xyz, 3);
        }
    }

void WriteDouble (Utf8CP name, double data, bool shortName = false)
    {
    m_dest.WriteNamedDouble (name, data, shortName);
    }

void WriteInt (Utf8CP name, int data, bool shortName = false)
    {
    m_dest.WriteNamedInt32 (name, data, shortName);
    }


void WriteBool (Utf8CP name, bool data, bool shortName = false)
    {
    m_dest.WriteNamedBool (name, data, shortName);
    }




void WriteList (bvector<DPoint3d> const & data, Utf8CP listName, Utf8CP shortName, Utf8CP itemName)
    {
    if (0 == data.size())
        return;

    if (m_textualizeXYData)
        {
        m_dest.WriteArrayElementStart(listName, shortName);
        for (size_t i = 0 ; i < data.size (); i++)
            WriteXYZ (itemName, data[i], true);
        m_dest.WriteArrayElementEnd (listName, shortName);
        }
    else
        {
        if (0 == data.size())
            return;
        m_dest.WriteArrayOfBlockedDoubles (listName, shortName, itemName, true,
              data.size () > 0 ? (double *)&data.front () : nullptr, 3, data.size ());
        }
    }

void WriteList (bvector<DVec3d> const & data, Utf8CP listName, Utf8CP shortName, Utf8CP itemName)
    {
    if (0 == data.size())
        return;
    m_dest.WriteArrayOfBlockedDoubles (listName, shortName, itemName, true,
          data.size () > 0 ? (double *)&data.front () : nullptr, 3, data.size ());
    }

void WriteList (bvector<RgbFactor> const & data, Utf8CP listName, Utf8CP shortName, Utf8CP itemName)
    {
    if (0 == data.size())
        return;
    m_dest.WriteArrayOfBlockedDoubles (listName, shortName, itemName, true,
          data.size () > 0 ? (double *)&data.front () : nullptr, 3, data.size ());
    }

void WriteList (bvector<DPoint2d> const & data, Utf8CP listName, Utf8CP shortName, Utf8CP itemName)
    {
    if (0 == data.size())
        return;

    m_dest.WriteArrayElementStart(listName, shortName);
    for (size_t i = 0 ; i < data.size (); i++)
        {
        Indent (m_dest);
        double refValue = data[i].Magnitude ();
        WriteXY (itemName, SuppressNearZeroCoordinate (data[i].x, refValue),
                    SuppressNearZeroCoordinate (data[i].y, refValue));
        }
    m_dest.WriteArrayElementEnd (listName, shortName);
    }


void WriteIndexList (bvector<int> const & data, Utf8CP listName, Utf8CP shortName, Utf8CP itemName)
    {
    if (0 == data.size())
        return;
    m_dest.WriteIntArray (listName, shortName, itemName, true, &data[0], data.size ());
    }

void WriteList (bvector<double> const & data, Utf8CP listName, Utf8CP shortName, Utf8CP itemName)
    {
    if (0 == data.size())
        return;

    //m_dest.WriteArrayElementStart(listName, shortName);
    m_dest.WriteDoubleArray (listName, shortName,
          itemName, true,
          (double *)&data.front (), data.size ());
    //m_dest.WriteArrayElementEnd (listName, shortName);
    }

void WritePlacementZX
(
DPoint3dCR origin,
DVec3dCR vectorZ,
DVec3dCR vectorX
)
    {
    m_dest.WriteSetElementStart("placement");
    WriteXYZ ("origin", origin);
    DVec3d unitZ, unitY, unitX;
    unitZ.Normalize (vectorZ);
    unitY.NormalizedCrossProduct (unitZ, vectorX);
    unitX.NormalizedCrossProduct (unitY, unitZ);
    WriteXYZ ("vectorZ", unitZ);
    WriteXYZ ("vectorX", unitX);
    m_dest.WriteSetElementEnd ("placement");
    }

void WritePlacementZX
(
DPoint3dCR origin,
RotMatrixCR axes
)
    {
    DVec3d vectorX, vectorY, vectorZ;
    axes.GetColumns (vectorX, vectorY, vectorZ);
    m_dest.WriteSetElementStart("placement");
    WriteXYZ ("origin", origin);
    DVec3d unitZ, unitY, unitX;
    unitZ.Normalize (vectorZ);
    unitY.NormalizedCrossProduct (unitZ, vectorX);
    unitX.NormalizedCrossProduct (unitY, unitZ);
    WriteXYZ ("vectorZ", unitZ);
    WriteXYZ ("vectorX", unitX);
    m_dest.WriteSetElementEnd ("placement");
    }


void WritePlacementXY (DPoint3dCR origin, DVec3dCR vectorX, DVec3dCR vectorY)
    {
    DVec3d unitNormal, unitX;
    unitNormal.NormalizedCrossProduct (vectorX, vectorY);
    unitX.NormalizedCrossProduct (vectorY, unitNormal);
    WritePlacementZX (origin, unitNormal, unitX);
    }




void WriteSegment (DSegment3dCR data)
    {
    m_dest.WriteSetElementStart("LineSegment");
    WriteXYZ ("startPoint", data.point[0]);
    WriteXYZ ("endPoint", data.point[1]);    
    m_dest.WriteSetElementEnd ("LineSegment");
    }

void WriteArc (DEllipse3dCR arc)
    {
    if (arc.IsCircular ())
        {
        DEllipse3d majorMinorArc = DEllipse3d::FromPerpendicularAxes (arc);
        m_dest.WriteSetElementStart("CircularArc");
        WritePlacementXY ( majorMinorArc.center, majorMinorArc.vector0, majorMinorArc.vector90);
        WriteDouble ("radius",     majorMinorArc.vector0.Magnitude ());
        WriteDouble ("startAngle",  Angle::RadiansToDegrees (majorMinorArc.start));
        WriteDouble ("sweepAngle",    Angle::RadiansToDegrees (majorMinorArc.sweep));
        m_dest.WriteSetElementEnd ("CircularArc");
        }
    else
        {
        DEllipse3d majorMinorArc = DEllipse3d::FromPerpendicularAxes (arc);
        m_dest.WriteSetElementStart("EllipticArc");
        WritePlacementXY ( majorMinorArc.center, majorMinorArc.vector0, majorMinorArc.vector90);
        WriteDouble ("radiusA",     majorMinorArc.vector0.Magnitude ());
        WriteDouble ("radiusB",     majorMinorArc.vector90.Magnitude ());
        WriteDouble ("startAngle",  Angle::RadiansToDegrees (majorMinorArc.start));
        WriteDouble ("sweepAngle",    Angle::RadiansToDegrees (majorMinorArc.sweep));
        m_dest.WriteSetElementEnd ("EllipticArc");
        }
    }

void WriteDisk (DEllipse3dCR arc)
    {
    double direction = arc.sweep > 0.0 ? 1.0 : -1.0;
    DEllipse3d majorMinorArc = DEllipse3d::FromPerpendicularAxes (arc);
    DVec3d yVector;
    yVector.Scale (majorMinorArc.vector90, direction);
    if (arc.IsCircular ())
        {
        m_dest.WriteSetElementStart("CircularDisk");
        WritePlacementXY (majorMinorArc.center, majorMinorArc.vector0, yVector);
        WriteDouble ("radius",     majorMinorArc.vector0.Magnitude ());
        m_dest.WriteSetElementEnd ("CircularDisk");
        }
    else
        {
        m_dest.WriteSetElementStart("EllipticDisk");
        WritePlacementXY (majorMinorArc.center, majorMinorArc.vector0, yVector);
        WriteDouble ("radiusA",     majorMinorArc.vector0.Magnitude ());
        WriteDouble ("radiusB",     majorMinorArc.vector90.Magnitude ());
        m_dest.WriteSetElementEnd ("EllipticDisk");
        }
    }


void WritePolyface (PolyfaceHeader &mesh)
    {
    m_dest.WriteSetElementStart("IndexedMesh");
    WriteList (mesh.Point (), "ListOfCoord", "Points", "xyz");

    if (mesh.PointIndex ().Active ())
        WriteIndexList (mesh.PointIndex (), "ListOfCoordIndex", "PointIndex", "id");

    if (mesh.Param ().Active ())
        WriteList (mesh.Param (), "ListOfParam", "Params", "uv");
    if (mesh.ParamIndex ().Active ())
        WriteIndexList (mesh.ParamIndex (), "ListOfParamIndex", "ParamIndex", "id");

    if (mesh.Normal ().Active ())
        WriteList (mesh.Normal (), "ListOfNormal", "Normals", "normal");
    if (mesh.NormalIndex ().Active ())
        WriteIndexList (mesh.NormalIndex (), "ListOfNormalIndex", "NormalIndex", "id");

    if (mesh.DoubleColor ().Active ())
        WriteList (mesh.DoubleColor (), "ListOfColor", "Colors", "Color");
    if (mesh.ColorIndex ().Active ())
        WriteIndexList (mesh.ColorIndex (), "ListOfColorIndex", "ColorIndex", "id");

    m_dest.WriteSetElementEnd ("IndexedMesh");
    }

void WriteCurve (MSBsplineCurveCR curve)
    {
    m_dest.WriteSetElementStart("BsplineCurve");

    WriteInt ("Order", curve.params.order);

    WriteBool ("Closed", curve.params.closed? true : false);

    bvector<DPoint3d> poles;
    poles.assign (curve.poles, curve.poles + curve.NumberAllocatedPoles ());
    WriteList (poles, "ListOfControlPoint", "ControlPoints", "xyz");
    
    if (curve.rational)
        {
        bvector<double> weights;
        weights.assign (curve.weights, curve.weights + curve.NumberAllocatedPoles ());
        WriteList (weights, "ListOfWeight", "Weights", "Weight");
        }

    bvector<double> knots;
    knots.assign (curve.knots, curve.knots + curve.NumberAllocatedKnots ());
    WriteList (knots, "ListOfKnot", "Knots", "Knot");

    m_dest.WriteSetElementEnd ("BsplineCurve");
    }


void WriteSurface (MSBsplineSurfaceCR surface)
    {
    m_dest.WriteSetElementStart("BsplineSurface");

    WriteInt ("OrderU", surface.uParams.order);
    WriteBool ("ClosedU", surface.uParams.closed ? true : false);
    WriteInt ("NumUControlPoint", surface.uParams.numPoles);

    WriteInt ("OrderV", surface.vParams.order);
    WriteBool ("ClosedV", surface.vParams.closed ? true : false);
    WriteInt ("NumVControlPoint", surface.vParams.numPoles);

    size_t totalPoles = (size_t)surface.uParams.numPoles * (size_t)surface.vParams.numPoles;
    bvector<DPoint3d> poles;
    poles.assign (surface.poles, surface.poles + totalPoles);
    WriteList (poles, "ListOfControlPoint", "ControlPoints", "xyz");
    
    if (surface.rational)
        {
        bvector<double> weights;
        weights.assign (surface.weights, surface.weights + totalPoles);
        WriteList (weights, "ListOfWeight", "Weights", "Weight");
        }

    bvector<double> uknots;
    int numUKnots = surface.uParams.NumberAllocatedKnots ();
    uknots.assign (surface.uKnots, surface.uKnots + numUKnots);
    WriteList (uknots, "ListOfKnotU", "KnotsU", "KnotU");

    bvector<double> vknots;
    int numVKnots = surface.vParams.NumberAllocatedKnots ();
    vknots.assign (surface.vKnots, surface.vKnots + numVKnots);
    WriteList (vknots, "ListOfKnotV", "KnotsV", "KnotV");

    m_dest.WriteSetElementEnd ("BsplineSurface");
    }


void WriteTextPlacement
(
IBeXmlWriterR m_dest,
DPoint3dCR xyz,
Utf8CP data,
double charSize
)
    {
    m_dest.WriteSetElementStart("SingleLineText");
    WritePlacementZX (xyz, DVec3d::From (0,0,1), DVec3d::From (1,0,0));
    Indent (m_dest); WriteText ("textString", data);
    Indent (m_dest); WriteText ("fontName", "ARIAL");
    Indent (m_dest); WriteDouble ("characterXSize", charSize);
    Indent (m_dest); WriteDouble ("characterYSize", charSize);
    Indent (m_dest); WriteDouble ("slantAngle", 0.0);
    m_dest.WriteSetElementEnd ("SingleLineText");
    }


void WriteLineString (bvector<DPoint3d> const &points)
    {
    m_dest.WriteSetElementStart("LineString");
    WriteList (points, "ListOfPoint", "Points", "xyz");
    m_dest.WriteSetElementEnd ("LineString");
    }

void WriteCoordinate (DPoint3dCR point)
    {
    m_dest.WriteSetElementStart("Coordinate");
    WriteXYZ ("xyz", point);
    m_dest.WriteSetElementEnd ("Coordinate");    
    }

void WritePointString (bvector<DPoint3d> const &points, bool preferMostCompactPrimitives)
    {
    if (points.size () == 1 && preferMostCompactPrimitives)
        {
        WriteCoordinate (points[0]);
        }
    else
        {
        m_dest.WriteSetElementStart("PointChain");
        m_dest.WriteSetElementStart("ListOfPoint");  // Needs work -- PointString is somehow not the same as points in linestring?
        for (size_t i = 0; i < points.size (); i++)
            WriteCoordinate (points[i]);
        m_dest.WriteSetElementEnd ("ListOfPoint");
        m_dest.WriteSetElementEnd ("PointChain");
        }
    }


void WritePolygon (bvector<DPoint3d> const &points)
    {
    m_dest.WriteSetElementStart("Polygon");
    WriteList (points, "ListOfPoint", "Points", "xyz");
    m_dest.WriteSetElementEnd ("Polygon");
    }


void Write (ICurvePrimitiveCR curve, bool preferMostCompactPrimitives)
    {
    switch (curve.GetCurvePrimitiveType ())
        {
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line:
            WriteSegment (*curve.GetLineCP ());
            break;

        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString:
            WriteLineString (*curve.GetLineStringCP ());
            break;

        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc:
            WriteArc (*curve.GetArcCP ());
            break;

        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_BsplineCurve:
            WriteCurve (*curve.GetBsplineCurveCP ());
            break;

        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_CurveVector:
            Write (*curve.GetChildCurveVectorCP ());
            break;

        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_PointString:
            WritePointString (*curve.GetPointStringCP (), preferMostCompactPrimitives);
            break;

        default:
            return;
        }
    }

void Write (ICurvePrimitiveCR curve)
    {
    Write (curve, true);
    }

void Write (CurveVectorCR curveVector)
    {
    Write (curveVector, true);
    }

void Write (CurveVectorCR curveVector, bool preferMostCompactPrimitives)
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
                WriteDisk (arc);
                }
            else if (preferMostCompactPrimitives
                && curveVector.size () == 1
                && NULL != (points = curveVector[0]->GetLineStringCP ())
                )
                {
                WritePolygon (*points);
                }
            else
                {
                m_dest.WriteSetElementStart("CurveChain");
                    m_dest.WriteArrayElementStart("ListOfCurve", "Curves");
                    for(ICurvePrimitivePtr curve : curveVector)
                        Write (*curve);
                    m_dest.WriteArrayElementEnd ("ListOfCurve", "Curves");
                m_dest.WriteSetElementEnd ("CurveChain");
                }
            break;
            }

        case CurveVector::BOUNDARY_TYPE_Open:
            {
            if (preferMostCompactPrimitives && curveVector.size () == 1)
                {
                Write (*curveVector[0]);
                }
            else
                {
                m_dest.WriteSetElementStart("CurveChain");
                    m_dest.WriteArrayElementStart("ListOfCurve", "Curves");
                    for(ICurvePrimitivePtr curve : curveVector)
                        Write (*curve);
                    m_dest.WriteArrayElementEnd ("ListOfCurve", "Curves");
                m_dest.WriteSetElementEnd ("CurveChain");
                }
            break;
            }


        case CurveVector::BOUNDARY_TYPE_ParityRegion:
            {
            m_dest.WriteSetElementStart("SurfacePatch");
            bvector <ICurvePrimitivePtr> holeLoop;
            int n = 0;
            for(ICurvePrimitivePtr curve : curveVector)
                {
                // ASSUME first child is outer loop...
                if (NULL != curve->GetChildCurveVectorCP ())
                    {
                    if (n++ == 0)
                        {
                        m_dest.WriteSetElementStart("ExteriorLoop");
                        Write (*curve->GetChildCurveVectorCP (), false);
                        m_dest.WriteSetElementEnd ("ExteriorLoop");
                        }
                    else
                        {
                        holeLoop.push_back (curve);
                        }
                    }
                }

            if (holeLoop.size() > 0)
                {
                m_dest.WriteArrayElementStart("ListOfHoleLoop","Holes");
                for (size_t i=0; i<holeLoop.size (); i++)
                    Write (*holeLoop[i]->GetChildCurveVectorCP (), false);
                m_dest.WriteArrayElementEnd ("ListOfHoleLoop","Holes");
                }
            m_dest.WriteSetElementEnd ("SurfacePatch");
            break;
            }
        
        case CurveVector::BOUNDARY_TYPE_UnionRegion:
            {
            m_dest.WriteSetElementStart("SurfaceGroup");
            for(ICurvePrimitivePtr curve : curveVector)
                Write (*curve->GetChildCurveVectorCP ());
            m_dest.WriteSetElementEnd ("SurfaceGroup");
            
            break;
            }

        case CurveVector::BOUNDARY_TYPE_None:
            {
            m_dest.WriteSetElementStart("Group");
            for(ICurvePrimitivePtr curve : curveVector)
                Write (*curve);
            m_dest.WriteSetElementEnd ("Group");
            
            break;
            }
        default:
            return;
        }
    }



void WriteDgnTorusPipeDetail (DgnTorusPipeDetail data)
    {
    DPoint3d center;
    RotMatrix axes;
    double radiusA, radiusB, sweepAngle;
    if (data.TryGetFrame (center, axes, radiusA, radiusB, sweepAngle))
        {
        m_dest.WriteSetElementStart("TorusPipe");
        WritePlacementZX (center, axes);
        WriteDouble ("radiusA", radiusA);
        WriteDouble ("radiusB", radiusB);
        WriteDouble ("startAngle", 0.0);
        WriteDouble ("sweepAngle", Angle::RadiansToDegrees (sweepAngle));
        WriteBool   ("bSolidFlag", data.m_capped);
        m_dest.WriteSetElementEnd ("TorusPipe");
        }
    }

void WriteDgnConeDetail (DgnConeDetail data)
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
                m_dest.WriteSetElementStart("CircularCylinder");
                WritePlacementZX (centerA, unitZ, unitX);
                WriteDouble ("height", height);
                WriteDouble ("radius", radiusA);
                WriteBool ("bSolidFlag", data.m_capped);
                m_dest.WriteSetElementEnd ("CircularCylinder");
                }
            else
                {
                m_dest.WriteSetElementStart("CircularCone");
                WritePlacementZX (centerA, unitZ, unitX);
                WriteDouble ("radiusA", radiusA);
                WriteDouble ("radiusB", radiusB);
                WriteDouble ("height", height);
                WriteBool ("bSolidFlag", data.m_capped);
                m_dest.WriteSetElementEnd ("CircularCone");
                }
            }
        else
            {
            m_dest.WriteSetElementStart("SkewedCone");
            WritePlacementZX (centerA, unitZ, unitX);
            WriteXYZ ("centerB", centerB);
            WriteDouble ("radiusA", radiusA);
            WriteDouble ("radiusB", radiusB);
            WriteBool ("bSolidFlag", data.m_capped);
            m_dest.WriteSetElementEnd ("SkewedCone");
            }
        }
    }


void WriteDgnBoxDetail (DgnBoxDetail data)
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
        m_dest.WriteSetElementStart("Block");
        DVec3d unitZ, unitX;
        unitZ.Normalize (zVector);
        unitX.Normalize (xVector);
        WritePlacementZX (origin, unitZ, unitX);
        WriteXYZ ("cornerA", DPoint3d::From (0,0,0));
        WriteXYZ ("cornerB", DPoint3d::From (ax, ay, zVector.Magnitude ()));
        WriteBool ("bSolidFlag", data.m_capped);
        m_dest.WriteSetElementEnd ("Block");
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
        WriteDgnRuledSweepDetail (ruledSurface);
        }                
    }

void WriteDgnSphereDetail (DgnSphereDetail data)
    {
    DPoint3d center;
    RotMatrix axes;
    double radius;
    if (data.IsTrueSphere (center, axes, radius))
        {
        m_dest.WriteSetElementStart("Sphere");
        DVec3d unitX, unitY, unitZ;
        axes.GetColumns (unitX, unitY, unitZ);
        WritePlacementZX (center, unitZ, unitX);
        WriteDouble ("radius", radius);
        m_dest.WriteSetElementEnd ("Sphere");
        }
    }

void WriteDgnExtrusionDetail (DgnExtrusionDetail data)
    {
    DPoint3d curveStart, curveEnd;
    if (data.m_baseCurve->GetStartEnd (curveStart, curveEnd))
        {
        m_dest.WriteSetElementStart(data.m_capped ? "SolidBySweptSurface" : "SurfaceBySweptCurve");
            m_dest.WriteSetElementStart("BaseGeometry");
            Write (*data.m_baseCurve);
            m_dest.WriteSetElementEnd ("BaseGeometry");
            DSegment3d segment;
            segment.point[0] = curveStart;
            segment.point[1].SumOf (curveStart, data.m_extrusionVector);
            m_dest.WriteSetElementStart("RailCurve");
            WriteSegment (segment);
            m_dest.WriteSetElementEnd ("RailCurve");
        m_dest.WriteSetElementEnd (data.m_capped ? "SolidBySweptSurface" : "SurfaceBySweptCurve");
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


void WriteDgnRotationalSweepDetail (DgnRotationalSweepDetail data)
    {
    Transform localToWorld, worldToLocal;    
    if (data.GetTransforms (localToWorld, worldToLocal))
        {
        m_dest.WriteSetElementStart(data.m_capped ? "SolidBySweptSurface" : "SurfaceBySweptCurve");
            m_dest.WriteSetElementStart("BaseGeometry");
            Write (*data.m_baseCurve);
            m_dest.WriteSetElementEnd ("BaseGeometry");
            DEllipse3d arc = BuildSweepArc (*data.m_baseCurve, localToWorld, worldToLocal, data.m_sweepAngle);
            m_dest.WriteSetElementStart("RailCurve");
            WriteArc (arc);
            m_dest.WriteSetElementEnd ("RailCurve");
        m_dest.WriteSetElementEnd (data.m_capped ? "SolidBySweptSurface" : "SurfaceBySweptCurve");
        }
    }

void WriteDgnRuledSweepDetail (DgnRuledSweepDetail data)
    {
    }


void Write (ISolidPrimitiveR primitive)
    {
    switch (primitive.GetSolidPrimitiveType ())
        {
        case SolidPrimitiveType_DgnTorusPipe:
            {
            DgnTorusPipeDetail detail;
            if (primitive.TryGetDgnTorusPipeDetail (detail))
                WriteDgnTorusPipeDetail (detail);
            }
            break;
        case SolidPrimitiveType_DgnCone:
            {
            DgnConeDetail detail;
            if (primitive.TryGetDgnConeDetail (detail))
                WriteDgnConeDetail (detail);
            }
            break;
        case SolidPrimitiveType_DgnBox:
            {
            DgnBoxDetail detail;
            if (primitive.TryGetDgnBoxDetail (detail))
                WriteDgnBoxDetail (detail);
            }
            break;
        case SolidPrimitiveType_DgnSphere:
            {
            DgnSphereDetail detail;
            if (primitive.TryGetDgnSphereDetail (detail))
                WriteDgnSphereDetail (detail);
            }
            break;
        case SolidPrimitiveType_DgnExtrusion:
            {
            DgnExtrusionDetail detail;
            if (primitive.TryGetDgnExtrusionDetail (detail))
                WriteDgnExtrusionDetail (detail);
            }
            break;
        case SolidPrimitiveType_DgnRotationalSweep:
            {
            DgnRotationalSweepDetail detail;
            if (primitive.TryGetDgnRotationalSweepDetail (detail))
                WriteDgnRotationalSweepDetail (detail);
            }
            break;
        case SolidPrimitiveType_DgnRuledSweep:
            {
            DgnRuledSweepDetail detail;
            if (primitive.TryGetDgnRuledSweepDetail (detail))
                WriteDgnRuledSweepDetail (detail);
            }
            break;
        }
    }


void Write (IGeometryPtr geometry)
    {
    if (!geometry.IsValid ())
        return;

    switch (geometry->GetGeometryType ())
        {
        case IGeometry::GeometryType::CurvePrimitive:
            {
            ICurvePrimitivePtr curvePrimitive = geometry->GetAsICurvePrimitive ();

            Write (*curvePrimitive);
            return;
            }

        case IGeometry::GeometryType::CurveVector:
            {
            CurveVectorPtr curveVector = geometry->GetAsCurveVector ();

            Write (*curveVector);
            return;
            }

        case IGeometry::GeometryType::SolidPrimitive:
            {
            ISolidPrimitivePtr solidPrimitive = geometry->GetAsISolidPrimitive ();

            Write (*solidPrimitive);
            return;
            }

        case IGeometry::GeometryType::BsplineSurface:
            {
            MSBsplineSurfacePtr bSurface = geometry->GetAsMSBsplineSurface ();

            WriteSurface (*bSurface);
            return;
            }

        case IGeometry::GeometryType::Polyface:
            {
            PolyfaceHeaderPtr polyface = geometry->GetAsPolyfaceHeader ();

            WritePolyface (*polyface);
            return;
            }

        default:
            {
            BeAssert (false);
            return;
            }
        }
    }
};
END_BENTLEY_ECOBJECT_NAMESPACE
