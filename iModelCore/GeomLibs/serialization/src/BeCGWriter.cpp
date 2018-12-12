/*--------------------------------------------------------------------------------------+
|
|  $Source: serialization/src/BeCGWriter.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "serializationPCH.h"
#include "BeCGWriter.h"

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

static const double s_nearZero = 9.0e-16;
//static const size_t s_maxIndexPerLine = 5;
static const Utf8CP s_namespace = "http://www.bentley.com/schemas/Bentley.Geometry.Common.1.0";
static const Utf8CP s_extendableNamespaceUri = "http://www.bentley.com/schemas/Bentley.ECSerializable.1.0";

static double SuppressNearZeroCoordinate (double x, double refValue)
    {
    double tol = s_nearZero * refValue;
    return fabs (x) < tol ? 0.0 : x;
    }

static void Indent (IBeStructuredDataWriterR m_dest)
    {
    }

BeCGWriter::BeCGWriter
    (
    IBeStructuredDataWriterR dest,
    bmap<OrderedIGeometryPtr,
    BeExtendedData>* extendedData,
    bool textualizeXYData,
    bool compactCurveVectors,
    bool preferCGSweeps,
    bool preferMostCompactPrimitivesInCGCurveVectors
    )
    : m_dest (dest), m_extendedData(extendedData),
      m_textualizeXYData (textualizeXYData), m_depth(0),
      m_compactCurveVectors(compactCurveVectors),
      m_preferCGSweeps (preferCGSweeps),
      m_preferMostCompactPrimitivesInCGCurveVectors (preferMostCompactPrimitivesInCGCurveVectors)
    {

    }

bool BeCGWriter::PreferCGSweeps () const
    {
    return m_preferCGSweeps;
    }

void BeCGWriter::WriteXYZ (Utf8CP name, DPoint3dCR xyz, bool shortName)
    {
    WriteXYZ (name, xyz.x, xyz.y, xyz.z, shortName);
    }

void BeCGWriter::WriteText (Utf8CP name, Utf8CP data, bool shortName)
    {
    m_dest.WriteNamedText (name, data, shortName);
    }

void BeCGWriter::WriteSetElementStart (Utf8CP name)
    {
    if (m_depth == 0)
        m_dest.WriteSetElementStart(name , s_namespace );
    else
        m_dest.WriteSetElementStart(name);
    m_depth++;
    }

void BeCGWriter::WriteSetElementEnd (Utf8CP name)
    {
    m_dest.WriteSetElementEnd(name);
    m_depth--;
    }

void BeCGWriter::WriteArrayElementStart(Utf8CP longName, Utf8CP shortName)
    {
    if (m_depth == 0)
        m_dest.WriteArrayElementStart(longName, shortName, s_namespace);
    else
        m_dest.WriteArrayElementStart(longName, shortName);
    m_depth++;
    }

void BeCGWriter::WriteArrayElementEnd(Utf8CP longName, Utf8CP shortName)
    {
    m_dest.WriteArrayElementEnd(longName, shortName);
    m_depth--;
    }

void BeCGWriter::WriteNamedSetStart (Utf8CP name) 
    {
    if (m_depth == 0)
        m_dest.WriteNamedSetStart(name, s_namespace);
    else
        m_dest.WriteNamedSetStart(name);
    m_depth++;
    }
void BeCGWriter::WriteNamedSetEnd(Utf8CP name)
    {
    m_dest.WriteNamedSetEnd(name);
    m_depth--;
    }

void BeCGWriter::WriteXY (Utf8CP name, double x, double y, bool shortName)
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
        m_dest.WriteBlockedDoubles (name, shortName, (double*)&xy, 2);
        }
    }

void BeCGWriter::WriteXYZ (Utf8CP name, double x, double y, double z, bool shortName)
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

void BeCGWriter::WriteDouble (Utf8CP name, double data, bool shortName)
    {
    m_dest.WriteNamedDouble (name, data, shortName);
    }

void BeCGWriter::WriteInt (Utf8CP name, int data, bool shortName)
    {
    m_dest.WriteNamedInt32 (name, data, shortName);
    }

void BeCGWriter::WriteBool (Utf8CP name, bool data, bool shortName)
    {
    m_dest.WriteNamedBool (name, data, shortName);
    }

void BeCGWriter::WriteList (bvector<DPoint3d> const & data, Utf8CP listName, Utf8CP shortName, Utf8CP itemName)
    {
    if (0 == data.size())
        return;

    if (m_textualizeXYData)
        {
        WriteArrayElementStart(listName, shortName);
        for (size_t i = 0 ; i < data.size (); i++)
            WriteXYZ (itemName, data[i], true);
        WriteArrayElementEnd (listName, shortName);
        }
    else
        {
        if (0 == data.size())
            return;
        m_dest.WriteArrayOfBlockedDoubles (listName, shortName, itemName, true,
            data.size () > 0 ? (double *)&data.front () : nullptr, 3, data.size ());
        }
    }

void BeCGWriter::WriteList (bvector<DVec3d> const & data, Utf8CP listName, Utf8CP shortName, Utf8CP itemName)
    {
    if (0 == data.size())
        return;

    if (m_textualizeXYData)
        {
        WriteArrayElementStart(listName, shortName);
        for (size_t i = 0 ; i < data.size (); i++)
            WriteXYZ (itemName, data[i], true);
        WriteArrayElementEnd (listName, shortName);
        }
    else
        {
        if (0 == data.size())
            return;
        m_dest.WriteArrayOfBlockedDoubles (listName, shortName, itemName, true,
            data.size () > 0 ? (double *)&data.front () : nullptr, 3, data.size ());
        }
    }

void BeCGWriter::WriteList (bvector<RgbFactor> const & data, Utf8CP listName, Utf8CP shortName, Utf8CP itemName)
    {
    if (0 == data.size())
        return;
    if (m_textualizeXYData)
        {
        WriteArrayElementStart(listName, shortName);
        for (size_t i = 0 ; i < data.size (); i++)
            WriteXYZ (itemName, data[i].red, data[i].green, data[i].blue, true);
        WriteArrayElementEnd (listName, shortName);
        }
    else
        {
        m_dest.WriteArrayOfBlockedDoubles (listName, shortName, itemName, true,
            data.size () > 0 ? (double *)&data.front () : nullptr, 3, data.size ());
        }
    }

void BeCGWriter::WriteList (bvector<DPoint2d> const & data, Utf8CP listName, Utf8CP shortName, Utf8CP itemName)
    {
    if (0 == data.size())
        return;

    if (m_textualizeXYData)
        {
        WriteArrayElementStart(listName, shortName);
        for (size_t i = 0 ; i < data.size (); i++)
            {
            double refValue = data[i].Magnitude ();
            WriteXY (itemName,
                SuppressNearZeroCoordinate (data[i].x, refValue), SuppressNearZeroCoordinate (data[i].y, refValue), true);
            }
        WriteArrayElementEnd (listName, shortName);
        }
    else
        {
        m_dest.WriteArrayOfBlockedDoubles (listName, shortName, itemName, true,
            data.size () > 0 ? (double *)&data.front () : nullptr, 2, data.size ());
        }
    }

void BeCGWriter::WriteIndexList (bvector<int> const & data, Utf8CP listName, Utf8CP shortName, Utf8CP itemName)
    {
    if (0 == data.size())
        return;
    m_dest.WriteIntArray (listName, shortName, itemName, true, &data[0], data.size ());
    }

void BeCGWriter::WriteList (bvector<double> const & data, Utf8CP listName, Utf8CP shortName, Utf8CP itemName)
    {
    if (0 == data.size())
        return;
    m_dest.WriteDoubleArray (listName, shortName,
        itemName, true,
        (double *)&data.front (), data.size ());
    }

void BeCGWriter::WritePlacementZX
    (
    DPoint3dCR origin,
    DVec3dCR vectorZ,
    DVec3dCR vectorX
    )
    {
    WriteNamedSetStart("placement");
    WriteXYZ ("origin", origin);
    DVec3d unitZ, unitY, unitX;
    unitZ.Normalize (vectorZ);
    unitY.NormalizedCrossProduct (unitZ, vectorX);
    unitX.NormalizedCrossProduct (unitY, unitZ);
    WriteXYZ ("vectorZ", unitZ);
    WriteXYZ ("vectorX", unitX);
    WriteNamedSetEnd ("placement");
    }

void BeCGWriter::WritePlacementZX
    (
    DPoint3dCR origin,
    RotMatrixCR axes
    )
    {
    DVec3d vectorX, vectorY, vectorZ;
    axes.GetColumns (vectorX, vectorY, vectorZ);
    WriteNamedSetStart("placement");
    WriteXYZ ("origin", origin);
    DVec3d unitZ, unitY, unitX;
    unitZ.Normalize (vectorZ);
    unitY.NormalizedCrossProduct (unitZ, vectorX);
    unitX.NormalizedCrossProduct (unitY, unitZ);
    WriteXYZ ("vectorZ", unitZ);
    WriteXYZ ("vectorX", unitX);
    WriteNamedSetEnd ("placement");
    }

void BeCGWriter::WritePlacementXY (DPoint3dCR origin, DVec3dCR vectorX, DVec3dCR vectorY)
    {
    DVec3d unitNormal, unitX;
    unitNormal.NormalizedCrossProduct (vectorX, vectorY);
    unitX.NormalizedCrossProduct (vectorY, unitNormal);
    WritePlacementZX (origin, unitNormal, unitX);
    }

void BeCGWriter::WriteSegment (DSegment3dCR data)
    {
    if (data.point[0].DistanceSquared (data.point[1]) == 0.0)
        {
        WriteCoordinate (data.point[0]);
        }
    else
        {
        WriteSetElementStart("LineSegment");
        WriteXYZ ("startPoint", data.point[0]);
        WriteXYZ ("endPoint", data.point[1]);    
        WriteSetElementEnd ("LineSegment");
        }
    }

static double CurvatureToRadius (double curvature)
    {
    double a;
    DoubleOps::SafeDivide (a, 1.0, curvature, 0.0);
    return a;
    }

void BeCGWriter::WriteSpiralType (Utf8CP name, int typeCode)
    {
    Utf8String string;
    if (DSpiral2dBase::TransitionTypeToString (typeCode, string))
        {
        WriteText (name, string.c_str ());
        }
    else
        WriteText (name, "Unknown");
    }


void BeCGWriter::WriteSpiral (struct DSpiral2dPlacement const &spiralPlacement)
    {
    DSpiral2dDirectEvaluation const * nominalLengthSpiral = dynamic_cast <DSpiral2dDirectEvaluation const*> (spiralPlacement.spiral);
    WriteSetElementStart ("TransitionSpiral");
    //--------------------------------IPlacement placement = g.GetPlacement ();
    //--------------------------------SerializeMember (placement, "Placement");
    Transform frame = spiralPlacement.frame;
    DVec3d unitX, unitY, unitZ;
    DPoint3d origin;
    frame.GetOriginAndVectors (origin, unitX, unitY, unitZ);
    WritePlacementZX (origin, unitZ, unitX);
    WriteSpiralType( "SpiralType", spiralPlacement.spiral->GetTransitionTypeCode ());

    WriteDouble ( "StartBearing", Angle::RadiansToDegrees (spiralPlacement.spiral->mTheta0));
    WriteDouble ("StartRadius", CurvatureToRadius (spiralPlacement.spiral->mCurvature0));

    if (nominalLengthSpiral != nullptr)
        WriteDouble ("Length", nominalLengthSpiral->m_nominalLength);
    else
        WriteDouble ("EndBearing", Angle::RadiansToDegrees (spiralPlacement.spiral->mTheta1));
    WriteDouble ("EndRadius", CurvatureToRadius (spiralPlacement.spiral->mCurvature1));

    WriteDouble ("ActiveStartFraction", spiralPlacement.fractionA);
    WriteDouble ("ActiveEndFraction", spiralPlacement.fractionB);

    //------------IGeometry geometry = g.GetGeometry ();
    //------------SerializeMember (geometry, "Geometry");
    WriteSetElementEnd ("TransitionSpiral");
    }

void BeCGWriter::WritePartialCurve (PartialCurveDetailCR data)
    {
    WriteSetElementStart ("PartialCurve");

    WriteDouble ( "Fraction0", data.fraction0);
    WriteDouble ("Fraction1", data.fraction1);
    WriteSetElementStart ("ParentCurve");
    Write (*data.parentCurve);
    WriteSetElementEnd ("ParentCurve");
    WriteSetElementEnd ("PartialCurve");
    }

void BeCGWriter::WriteArc (DEllipse3dCR arc)
    {
    if (arc.IsCircular ())
        {
        DEllipse3d majorMinorArc = DEllipse3d::FromPerpendicularAxes (arc);
        WriteSetElementStart("CircularArc");
        WritePlacementXY ( majorMinorArc.center, majorMinorArc.vector0, majorMinorArc.vector90);
        WriteDouble ("radius",     majorMinorArc.vector0.Magnitude ());
        WriteDouble ("startAngle",  Angle::RadiansToDegrees (majorMinorArc.start));
        WriteDouble ("sweepAngle",    Angle::RadiansToDegrees (majorMinorArc.sweep));
        WriteSetElementEnd ("CircularArc");
        }
    else
        {
        DEllipse3d majorMinorArc = DEllipse3d::FromPerpendicularAxes (arc);
        WriteSetElementStart("EllipticArc");
        WritePlacementXY ( majorMinorArc.center, majorMinorArc.vector0, majorMinorArc.vector90);
        WriteDouble ("radiusA",     majorMinorArc.vector0.Magnitude ());
        WriteDouble ("radiusB",     majorMinorArc.vector90.Magnitude ());
        WriteDouble ("startAngle",  Angle::RadiansToDegrees (majorMinorArc.start));
        WriteDouble ("sweepAngle",    Angle::RadiansToDegrees (majorMinorArc.sweep));
        WriteSetElementEnd ("EllipticArc");
        }
    }

void BeCGWriter::WriteDisk (DEllipse3dCR arc)
    {
    double direction = arc.sweep > 0.0 ? 1.0 : -1.0;
    DEllipse3d majorMinorArc = DEllipse3d::FromPerpendicularAxes (arc);
    DVec3d yVector;
    yVector.Scale (majorMinorArc.vector90, direction);
    if (arc.IsCircular ())
        {
        WriteSetElementStart("CircularDisk");
        WritePlacementXY (majorMinorArc.center, majorMinorArc.vector0, yVector);
        WriteDouble ("radius",     majorMinorArc.vector0.Magnitude ());
        WriteSetElementEnd ("CircularDisk");
        }
    else
        {
        WriteSetElementStart("EllipticDisk");
        WritePlacementXY (majorMinorArc.center, majorMinorArc.vector0, yVector);
        WriteDouble ("radiusA",     majorMinorArc.vector0.Magnitude ());
        WriteDouble ("radiusB",     majorMinorArc.vector90.Magnitude ());
        WriteSetElementEnd ("EllipticDisk");
        }
    }

void BeCGWriter::WritePolyface (PolyfaceHeader &mesh)
    {
    WriteSetElementStart("IndexedMesh");
    WriteList (mesh.Point (), "ListOfCoord", "Coord", "xyz");

    if (mesh.PointIndex ().Active ())
        WriteIndexList (mesh.PointIndex (), "ListOfCoordIndex", "CoordIndex", "id");

    if (mesh.Param ().Active ())
        WriteList (mesh.Param (), "ListOfParam", "Param", "uv");
    if (mesh.ParamIndex ().Active ())
        WriteIndexList (mesh.ParamIndex (), "ListOfParamIndex", "ParamIndex", "id");

    if (mesh.Normal ().Active ())
        WriteList (mesh.Normal (), "ListOfNormal", "Normal", "normal");
    if (mesh.NormalIndex ().Active ())
        WriteIndexList (mesh.NormalIndex (), "ListOfNormalIndex", "NormalIndex", "id");

#if defined (NOT_NOW_MESH_COLOR)
    if (mesh.DoubleColor ().Active ())
        WriteList (mesh.DoubleColor (), "ListOfColor", "Color", "Color");
    if (mesh.ColorIndex ().Active ())
        WriteIndexList (mesh.ColorIndex (), "ListOfColorIndex", "ColorIndex", "id");
#endif

    WriteSetElementEnd ("IndexedMesh");
    }

void BeCGWriter::WriteCurve (MSBsplineCurveCR curve)
    {
    WriteSetElementStart("BsplineCurve");

    WriteInt ("order", curve.params.order);

    WriteBool ("closed", curve.params.closed? true : false);

    bvector<DPoint3d> poles;
    poles.assign (curve.poles, curve.poles + curve.NumberAllocatedPoles ());
    WriteList (poles, "ListOfControlPoint", "ControlPoint", "xyz");

    if (curve.rational)
        {
        bvector<double> weights;
        weights.assign (curve.weights, curve.weights + curve.NumberAllocatedPoles ());
        WriteList (weights, "ListOfWeight", "Weight", "Weight");
        }

    bvector<double> knots;
    knots.assign (curve.knots, curve.knots + curve.NumberAllocatedKnots ());
    WriteList (knots, "ListOfKnot", "Knot", "Knot");

    WriteSetElementEnd ("BsplineCurve");
    }

void BeCGWriter::WriteFullSurface (MSBsplineSurfaceCR surface)
    {
    WriteSetElementStart("BsplineSurface");

    WriteInt ("orderU", surface.uParams.order);
    WriteBool ("closedU", surface.uParams.closed ? true : false);
    WriteInt ("numUControlPoint", surface.uParams.numPoles);

    WriteInt ("orderV", surface.vParams.order);
    WriteBool ("closedV", surface.vParams.closed ? true : false);
    WriteInt ("numVControlPoint", surface.vParams.numPoles);

    size_t totalPoles = (size_t)surface.uParams.numPoles * (size_t)surface.vParams.numPoles;
    bvector<DPoint3d> poles;
    poles.assign (surface.poles, surface.poles + totalPoles);
    WriteList (poles, "ListOfControlPoint", "ControlPoint", "xyz");

    if (surface.rational)
        {
        bvector<double> weights;
        weights.assign (surface.weights, surface.weights + totalPoles);
        WriteList (weights, "ListOfWeight", "Weight", "Weight");
        }

    bvector<double> uknots;
    int numUKnots = surface.uParams.NumberAllocatedKnots ();
    uknots.assign (surface.uKnots, surface.uKnots + numUKnots);
    WriteList (uknots, "ListOfKnotU", "KnotU", "KnotU");

    bvector<double> vknots;
    int numVKnots = surface.vParams.NumberAllocatedKnots ();
    vknots.assign (surface.vKnots, surface.vKnots + numVKnots);
    WriteList (vknots, "ListOfKnotV", "KnotV", "KnotV");

    WriteSetElementEnd ("BsplineSurface");
    }

void BeCGWriter::WriteSurface (MSBsplineSurfaceCR surface)
    {
    size_t numBounds = surface.GetNumBounds ();
    CurveVectorPtr curves;
    if (numBounds > 0)
        curves = surface.GetUVBoundaryCurves (true, false);
    if (curves.IsValid ())
        {
        WriteSetElementStart ("ParametricSurfacePatch");

        WriteText ("loopType", "Parity");     // parity region.

        WriteSetElementStart ("surface");
        WriteFullSurface (surface);
        WriteSetElementEnd ("surface");

        WriteArrayElementStart("ListOfCurveChain", "CurveChain");

        if (curves->IsParityRegion ())
            {
            for (size_t i = 0; i < curves->size (); i++)
                {
                WriteCurveVectorAsSingleCurveChain (*curves->at(i)->GetChildCurveVectorP ());
                }
            }
        else
            {
            WriteCurveVectorAsSingleCurveChain (*curves);
            }
        WriteArrayElementEnd("ListOfCurveChain", "CurveChain");
        WriteSetElementEnd ("ParametricSurfacePatch");
        }
    else
        WriteFullSurface (surface);
    }

void BeCGWriter::WriteTextPlacement
    (
    IBeStructuredDataWriterR m_dest,
    DPoint3dCR xyz,
    Utf8CP data,
    double charSize
    )
    {
    WriteSetElementStart("SingleLineText");
    WritePlacementZX (xyz, DVec3d::From (0,0,1), DVec3d::From (1,0,0));
    Indent (m_dest); WriteText ("textString", data);
    Indent (m_dest); WriteText ("fontName", "ARIAL");
    Indent (m_dest); WriteDouble ("characterXSize", charSize);
    Indent (m_dest); WriteDouble ("characterYSize", charSize);
    Indent (m_dest); WriteDouble ("slantAngle", 0.0);
    WriteSetElementEnd ("SingleLineText");
    }

void BeCGWriter::WriteLineString (bvector<DPoint3d> const &points)
    {
    WriteSetElementStart("LineString");
    WriteList (points, "ListOfPoint", "Point", "xyz");
    WriteSetElementEnd ("LineString");
    }

void BeCGWriter::WriteCoordinate (DPoint3dCR point)
    {
    WriteSetElementStart("Coordinate");
    WriteXYZ ("xyz", point);
    WriteSetElementEnd ("Coordinate");    
    }

void BeCGWriter::WritePointString (bvector<DPoint3d> const &points, bool preferMostCompactPrimitives)
    {
    static int s_simplifySingletonPointString = false;
    if (points.size () == 1 && preferMostCompactPrimitives && s_simplifySingletonPointString)
        {
        WriteCoordinate (points[0]);
        }
    else
        {
        WriteSetElementStart("PointChain");
        WriteSetElementStart("ListOfPoint");  // Needs work -- PointString is somehow not the same as points in linestring?
        for (size_t i = 0; i < points.size (); i++)
            WriteCoordinate (points[i]);
        WriteSetElementEnd ("ListOfPoint");
        WriteSetElementEnd ("PointChain");
        }
    }

void BeCGWriter::WritePolygon (bvector<DPoint3d> const &points)
    {
    WriteSetElementStart("Polygon");
    WriteList (points, "ListOfPoint", "Point", "xyz");
    WriteSetElementEnd ("Polygon");
    }

void BeCGWriter::WriteExtendedData(BeExtendedData const& extendedData)
    {
    m_dest.WriteSetElementStart("ExtendedData");
    m_dest.WriteSetElementStart("TransientLookupCollection");
    for (auto const& entry: extendedData)
        {
        m_dest.WriteSetElementStart("Entry");
        m_dest.WriteAttribute("key", entry.Key.c_str());
        m_dest.WriteAttribute("typeCode", entry.Type.c_str());
        m_dest.WriteContent(entry.Value.c_str());
        WriteSetElementEnd("Entry");
        }

    m_dest.WriteSetElementEnd("TransientLookupCollection");
    m_dest.WriteSetElementEnd("ExtendedData");
    m_dest.WriteSetElementEnd("ExtendedObject");
    }
void BeCGWriter::Write (ICurvePrimitiveCR curve, bool preferMostCompactPrimitives)
    {
    bmap<OrderedIGeometryPtr, BeExtendedData>::const_iterator extendedDataIterator;
    bool hasExtendedData = false;
    ICurvePrimitivePtr cpPtr = const_cast <ICurvePrimitiveP>(&curve);
    IGeometryPtr geometry = IGeometry::Create (cpPtr);
    if (nullptr != m_extendedData)
        {
        if ((extendedDataIterator = m_extendedData->find(geometry)) != m_extendedData->end())
            {
            hasExtendedData = true;
            m_dest.WriteSetElementStart("ExtendedObject", s_extendableNamespaceUri);
            }
        }

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

        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Spiral:
            WriteSpiral (*curve.GetSpiralPlacementCP ());
            break;

        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_PartialCurve:
            WritePartialCurve (*curve.GetPartialCurveDetailCP ());
            break;

        default:
            return;
        }
    if (hasExtendedData)
        {
        WriteExtendedData(extendedDataIterator->second);
        }
    }

void BeCGWriter::Write (ICurvePrimitiveCR curve)
    {
    Write (curve, true);
    }

void BeCGWriter::Write (CurveVectorCR curveVector)
    {
    if (PreferCGSweeps ())
        WriteCGCurveVector (curveVector, m_preferMostCompactPrimitivesInCGCurveVectors);
    else
        WriteNativeCurveVector (curveVector);
        
    }

void BeCGWriter::WriteNativeCurveVector (CurveVectorCR curveVector)
    {
    WriteSetElementStart ("DgnCurveVector");
    WriteInt ("boundaryType", (int)curveVector.GetBoundaryType ());
    if (curveVector.size () > 0)
        {
        WriteArrayElementStart("ListOfMember", "Member");
        for (ICurvePrimitivePtr const &child : curveVector)
            {
            m_dest.WriteArrayMemberStart ();
            Write (*child, false);
            m_dest.WriteArrayMemberEnd ();
            }
        WriteArrayElementEnd("ListOfMember", "Member");
        }
    else
        {
        BeAssert (false);
        }
    WriteSetElementEnd ("DgnCurveVector");
    }

// For use as bsurf boundary
void BeCGWriter::WriteCurveVectorAsSingleCurveChain (CurveVectorCR curves)
    {
    if (curves.IsOpenPath () || curves.IsClosedPath ())
        {
        WriteSetElementStart ("CurveChain");
        WriteArrayElementStart ("ListOfCurve", "Curve");
        for (size_t i = 0; i < curves.size (); i++)
            {
            Write (*curves[i], true);
            }
        WriteArrayElementEnd ("ListOfCurve", "Curve");
        WriteSetElementEnd ("CurveChain");
        }
    }


void BeCGWriter::WriteCGCurveVector (CurveVectorCR curveVector, bool preferMostCompactPrimitives, bool wrapInElement, bool preferCurveChain)
    {
    int pathType = curveVector.GetBoundaryType ();
    if (preferCurveChain && (pathType ==CurveVector::BOUNDARY_TYPE_Outer || pathType == CurveVector::BOUNDARY_TYPE_Inner))
        pathType = CurveVector::BOUNDARY_TYPE_Open;
    switch (pathType)
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
                && nullptr != (points = curveVector[0]->GetLineStringCP ())
                )
                {
                WritePolygon (*points);
                }
            else if (curveVector.size () == 0)
                {
                // hmph.  What to do if no curves?
                }
            else
                {
                if (m_compactCurveVectors)
                    {
                      WriteSetElementStart ("SurfacePatch");
                    if (wrapInElement)
                        {
                        WriteArrayElementStart("exteriorLoop", nullptr);
                        }
                    else
                        {
                        WriteArrayElementStart("Curves", "Curves");
                        }
                    for(ICurvePrimitivePtr curve : curveVector)
                        {
                        m_dest.WriteArrayMemberStart ();
                        Write (*curve);
                        m_dest.WriteArrayMemberEnd ();
                        }
                    if (wrapInElement)
                        {
                        WriteArrayElementEnd ("exteriorLoop", nullptr);
                        WriteSetElementEnd("SurfacePatch");
                        }
                    else
                        {
                        WriteArrayElementEnd ("Curves", "Curves");
                        }
                    }
                else
                    {
                    if (wrapInElement)
                        {
                        WriteSetElementStart ("SurfacePatch");
                        WriteNamedSetStart("exteriorLoop");
                        WriteSetElementStart ("CurveChain");
                        WriteArrayElementStart("ListOfCurve", "Curve");
                        for(ICurvePrimitivePtr curve : curveVector)
                            {
                            m_dest.WriteArrayMemberStart ();
                            Write (*curve);
                            m_dest.WriteArrayMemberEnd ();
                            }
                        WriteArrayElementEnd ("ListOfCurve", "Curve");
                        WriteSetElementEnd ("CurveChain");
                          WriteNamedSetEnd("exteriorLoop");
                        WriteSetElementEnd("SurfacePatch");
                        }
                    else
                        {
                        WriteSetElementStart ("CurveChain");
                        WriteArrayElementStart("ListOfCurve", "Curve");
                        for(ICurvePrimitivePtr curve : curveVector)
                            {
                            m_dest.WriteArrayMemberStart ();
                            Write (*curve);
                            m_dest.WriteArrayMemberEnd ();
                            }
                        WriteArrayElementEnd ("ListOfCurve", "Curve");
                        WriteSetElementEnd ("CurveChain");
                        }
                    }
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
                WriteSetElementStart("CurveChain");
                if (curveVector.size () > 0)
                    {
                    WriteArrayElementStart("ListOfCurve", "Curve");
                    for(ICurvePrimitivePtr curve : curveVector)
                        {
                        m_dest.WriteArrayMemberStart ();
                        Write (*curve);
                        m_dest.WriteArrayMemberEnd ();
                        }
                    WriteArrayElementEnd ("ListOfCurve", "Curve");
                    }
                WriteSetElementEnd ("CurveChain");
                }
            break;
            }


        case CurveVector::BOUNDARY_TYPE_ParityRegion:
            {
            if (m_compactCurveVectors)
                {
                WriteSetElementStart("ParityRegion");
                WriteArrayElementStart ("loops", "loops");
                for(ICurvePrimitivePtr curve : curveVector)
                    {
                    CurveVectorCP child = curve->GetChildCurveVectorCP ();
                    if (nullptr != child && child->size () > 0)
                        {
                        WriteArrayElementStart (nullptr, nullptr);
                        for(ICurvePrimitivePtr curve : *child)
                            {
                            m_dest.WriteArrayMemberStart ();
                            Write (*curve);
                            m_dest.WriteArrayMemberEnd ();
                            }
                        WriteArrayElementEnd (nullptr, nullptr);
                        }
                    }
                WriteArrayElementEnd ("loops", "loops");
                WriteSetElementEnd ("ParityRegion");
                }
            else
                {
                WriteSetElementStart("SurfacePatch");
                bvector <ICurvePrimitivePtr> holeLoop;
                int n = 0;
                for(ICurvePrimitivePtr curve : curveVector)
                    {
                    // ASSUME first child is outer loop...
                    if (nullptr != curve->GetChildCurveVectorCP ())
                        {
                        if (n++ == 0)
                            {
                            WriteNamedSetStart("exteriorLoop");
                            WriteCGCurveVector (*curve->GetChildCurveVectorCP (), false, false);
                            WriteNamedSetEnd ("exteriorLoop");
                            }
                        else
                            {
                            holeLoop.push_back (curve);
                            }
                        }
                    }

                if (holeLoop.size() > 0)
                    {
                    if (m_compactCurveVectors)
                        {
                        WriteNamedSetStart("Holes");
                        WriteArrayElementStart ("Holes", "Holes");
                        for (size_t i=0; i<holeLoop.size (); i++)
                                {
                                m_dest.WriteArrayMemberStart ();
                                WriteCGCurveVector (*holeLoop[i]->GetChildCurveVectorCP (), false, false);
                                m_dest.WriteArrayMemberEnd ();
                                }
                        WriteArrayElementEnd ("Holes", "Holes");
                        WriteSetElementEnd ("Holes");
                        }
                    else
                        {
                        WriteArrayElementStart("ListOfHoleLoop","HoleLoop");
                        for (size_t i=0; i<holeLoop.size (); i++)
                                {
                                m_dest.WriteArrayMemberStart ();
                                WriteCGCurveVector (*holeLoop[i]->GetChildCurveVectorCP (), false, false);
                                m_dest.WriteArrayMemberEnd ();
                                }
                        WriteArrayElementEnd ("ListOfHoleLoop","HoleLoop");
                        }
                    }
                WriteSetElementEnd ("SurfacePatch");
                }
            break;
            }

        case CurveVector::BOUNDARY_TYPE_UnionRegion:
            {
            WriteSetElementStart("SurfaceGroup");
            for(ICurvePrimitivePtr curve : curveVector)
                WriteCGCurveVector (*curve->GetChildCurveVectorCP (), false, false);
            WriteSetElementEnd ("SurfaceGroup");

            break;
            }

        case CurveVector::BOUNDARY_TYPE_None:
            {
            WriteSetElementStart("Group");
            for(ICurvePrimitivePtr curve : curveVector)
                Write (*curve);
            WriteSetElementEnd ("Group");

            break;
            }
        default:
            return;
        }
    }

void BeCGWriter::WriteDgnTorusPipeDetail (DgnTorusPipeDetail data)
    {
    DPoint3d center;
    RotMatrix axes;
    double radiusA, radiusB, sweepAngle;
    bool preferCGSweeps = PreferCGSweeps ();
    if (preferCGSweeps && data.TryGetFrame (center, axes, radiusA, radiusB, sweepAngle))
        {
        WriteSetElementStart("TorusPipe");
        WritePlacementZX (center, axes);
        WriteDouble ("radiusA", radiusA);
        WriteDouble ("radiusB", radiusB);
        WriteDouble ("startAngle", 0.0);
        WriteDouble ("sweepAngle", Angle::RadiansToDegrees (sweepAngle));
        WriteBool   ("bSolidFlag", data.m_capped);
        WriteSetElementEnd ("TorusPipe");
        }
    else
        {
        WriteSetElementStart("DgnTorusPipe");
        WriteXYZ ("center", data.m_center);
        WriteXYZ ("vectorX", data.m_vectorX);
        WriteXYZ ("vectorY", data.m_vectorY);
        WriteDouble ("majorRadius", data.m_majorRadius);
        WriteDouble ("minorRadius", data.m_minorRadius);
        WriteDouble ("sweepAngle", Angle::RadiansToDegrees (data.m_sweepAngle));
        WriteBool ("capped", data.m_capped);
        WriteSetElementEnd ("DgnTorusPipe");
        }
    }

void BeCGWriter::WriteDgnConeDetail (DgnConeDetail data)
    {
    DPoint3d centerA, centerB;
    RotMatrix axes;
    double radiusA, radiusB;
    bool capped;
    bool preferCGSweeps = PreferCGSweeps ();
    if (preferCGSweeps && data.IsCircular (centerA, centerB, axes, radiusA, radiusB, capped))
        {
        DVec3d unitX, unitY, unitZ;
        axes.GetColumns (unitX, unitY, unitZ);
        double height = centerB.DotDifference (centerA, unitZ);
        double distance = centerA.Distance (centerB);
        if (DoubleOps::AlmostEqual (height, distance))
            {
            if (DoubleOps::AlmostEqual (radiusA, radiusB))
                {
                WriteSetElementStart("CircularCylinder");
                WritePlacementZX (centerA, unitZ, unitX);
                WriteDouble ("height", height);
                WriteDouble ("radius", radiusA);
                WriteBool ("bSolidFlag", data.m_capped);
                WriteSetElementEnd ("CircularCylinder");
                }
            else
                {
                WriteSetElementStart("CircularCone");
                WritePlacementZX (centerA, unitZ, unitX);
                WriteDouble ("radiusA", radiusA);
                WriteDouble ("radiusB", radiusB);
                WriteDouble ("height", height);
                WriteBool ("bSolidFlag", data.m_capped);
                WriteSetElementEnd ("CircularCone");
                }
            }
        else
            {
            WriteSetElementStart("SkewedCone");
            WritePlacementZX (centerA, unitZ, unitX);
            WriteXYZ ("centerB", centerB);
            WriteDouble ("radiusA", radiusA);
            WriteDouble ("radiusB", radiusB);
            WriteBool ("bSolidFlag", data.m_capped);
            WriteSetElementEnd ("SkewedCone");
            }
        }
    else
        {
        WriteSetElementStart("DgnCone");
        WriteXYZ ("centerA", data.m_centerA);
        WriteXYZ ("centerB", data.m_centerB);
        WriteXYZ ("vectorX", data.m_vector0);
        WriteXYZ ("vectorY", data.m_vector90);
        WriteDouble ("radiusA", data.m_radiusA);
        WriteDouble ("radiusB", data.m_radiusB);
        WriteBool ("capped", data.m_capped);
        WriteSetElementEnd ("DgnCone");
        }
    }

void BeCGWriter::WriteDgnBoxDetail (DgnBoxDetail data)
    {
    Transform localToWorld;
    double ax, ay, bx, by;
    data.GetNonUniformTransform (localToWorld, ax, ay, bx, by);
    DPoint3d origin;
    DVec3d xVector, yVector, zVector;
    localToWorld.GetOriginAndVectors (origin, xVector, yVector, zVector);
    bool preferCGSweeps = PreferCGSweeps ();
    if (preferCGSweeps && xVector.IsPerpendicularTo (yVector)
        && xVector.IsPerpendicularTo (zVector)
        && yVector.IsPerpendicularTo (zVector)
        && DoubleOps::AlmostEqual (ax, bx)
        && DoubleOps::AlmostEqual (ay, by)
        )
        {
        WriteSetElementStart("Block");
        DVec3d unitZ, unitX;
        unitZ.Normalize (zVector);
        unitX.Normalize (xVector);
        WritePlacementZX (origin, unitZ, unitX);
        WriteXYZ ("cornerA", DPoint3d::From (0,0,0));
        WriteXYZ ("cornerB", DPoint3d::From (ax, ay, zVector.Magnitude ()));
        WriteBool ("bSolidFlag", data.m_capped);
        WriteSetElementEnd ("Block");
        }
    else
        {
        WriteSetElementStart("DgnBox");
        WriteXYZ ("baseOrigin", data.m_baseOrigin);
        WriteXYZ ("topOrigin", data.m_topOrigin);
        WriteXYZ ("vectorX", data.m_vectorX);
        WriteXYZ ("vectorY", data.m_vectorY);
        WriteDouble ("baseX", data.m_baseX);
        WriteDouble ("baseY", data.m_baseY);
        WriteDouble ("topX", data.m_topX);
        WriteDouble ("topY", data.m_topY);
        WriteBool ("capped", data.m_capped);
        WriteSetElementEnd ("DgnBox");
        }
    }

void BeCGWriter::WriteDgnSphereDetail (DgnSphereDetail data)
    {
    DPoint3d center;
    RotMatrix axes;
    DVec3d unitX, unitY, unitZ;
    double radiusXY, radiusZ, radius;
    if (data.IsTrueSphere (center, axes, radius)
        && PreferCGSweeps ()
        )
        {
        WriteSetElementStart("Sphere");
        axes.GetColumns (unitX, unitY, unitZ);
        WritePlacementZX (center, unitZ, unitX);
        WriteDouble ("radius", radius);
        WriteSetElementEnd ("Sphere");
        }
    else if (data.IsTrueRotationAroundZ (center, unitX, unitY, unitZ, radiusXY, radiusZ))
        {
        WriteSetElementStart("DgnSphere");
        WriteXYZ ("center", center.x, center.y, center.z);
        WriteXYZ ("vectorX", unitX.x, unitX.y, unitX.z);
        WriteXYZ ("vectorZ", unitZ.x, unitZ.y, unitZ.z);
        WriteDouble ("radiusXY", radiusXY);
        WriteDouble ("radiusZ", radiusZ);
        WriteDouble ("startLatitude", Angle::RadiansToDegrees (data.m_startLatitude));
        WriteDouble ("latitudeSweep", Angle::RadiansToDegrees (data.m_latitudeSweep));
        WriteBool ("capped", data.m_capped);
        WriteSetElementEnd ("DgnSphere");
        }        
    }

void BeCGWriter::WriteDgnExtrusionDetail (DgnExtrusionDetail data)
    {
    DPoint3d curveStart, curveEnd;
    if (PreferCGSweeps ())
        {
        if (data.m_baseCurve->GetStartEnd (curveStart, curveEnd))
            {
            WriteSetElementStart(data.m_capped ? "SolidBySweptSurface" : "SurfaceBySweptCurve");
            WriteNamedSetStart("baseGeometry");
            WriteCGCurveVector (*data.m_baseCurve, true, true);
            WriteNamedSetEnd ("baseGeometry");
            DSegment3d segment;
            segment.point[0] = curveStart;
            segment.point[1].SumOf (curveStart, data.m_extrusionVector);
            WriteNamedSetStart("railCurve");
            WriteSegment (segment);
            WriteNamedSetEnd ("railCurve");
            WriteSetElementEnd (data.m_capped ? "SolidBySweptSurface" : "SurfaceBySweptCurve");
            }
        }
    else
        {
        WriteSetElementStart ("DgnExtrusion");
        WriteXYZ ("extrusionVector", data.m_extrusionVector.x, data.m_extrusionVector.y, data.m_extrusionVector.z);
        WriteBool ("capped", data.m_capped);
        WriteNamedSetStart("baseGeometry");
        WriteNativeCurveVector (*data.m_baseCurve);
        WriteNamedSetEnd ("baseGeometry");
        WriteSetElementEnd ("DgnExtrusion");
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
        for (size_t i = 0; i < strokes.size (); i++)
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

void BeCGWriter::WriteDgnRotationalSweepDetail (DgnRotationalSweepDetail data)
    {
    Transform localToWorld, worldToLocal;    
    if (PreferCGSweeps () && data.GetTransforms (localToWorld, worldToLocal))
        {
        WriteSetElementStart(data.m_capped ? "SolidBySweptSurface" : "SurfaceBySweptCurve");
        WriteNamedSetStart("baseGeometry");
        WriteCGCurveVector (*data.m_baseCurve, true, false);
        WriteNamedSetEnd ("baseGeometry");
        DEllipse3d arc = BuildSweepArc (*data.m_baseCurve, localToWorld, worldToLocal, data.m_sweepAngle);
        WriteNamedSetStart("railCurve");
        WriteArc (arc);
        WriteNamedSetEnd ("railCurve");
        WriteSetElementEnd (data.m_capped ? "SolidBySweptSurface" : "SurfaceBySweptCurve");
        }
    else
        {
        WriteSetElementStart ("DgnRotationalSweep");
        WriteXYZ ("center", data.m_axisOfRotation.origin);
        WriteXYZ ("axis", data.m_axisOfRotation.direction);
        WriteDouble ("sweepAngle", Angle::RadiansToDegrees (data.m_sweepAngle));
        WriteBool ("capped", data.m_capped);
        // ??? NumVRules ???
        WriteNamedSetStart("baseGeometry");
        WriteNativeCurveVector (*data.m_baseCurve);
        WriteNamedSetEnd ("baseGeometry");
        WriteSetElementEnd ("DgnRotationalSweep");
        }
    }

void BeCGWriter::WriteDgnRuledSweepDetail (DgnRuledSweepDetail data)
    {
    if (PreferCGSweeps ())
        {
        WriteSetElementStart (data.m_capped ? "SolidByRuledSweep" : "SurfaceByRuledSweep");
        WriteArrayElementStart("ListOfSection","Section");
        for (size_t i = 0; i < data.m_sectionCurves.size (); i++)
            {
            m_dest.WriteArrayMemberStart ();
            WriteNativeCurveVector (*data.m_sectionCurves[i]);
            m_dest.WriteArrayMemberEnd ();
            }
        WriteArrayElementEnd ("ListOfSection","Section");
        WriteSetElementEnd (data.m_capped ? "SolidByRuledSweep" : "SurfaceByRuledSweep");
        }
    else
        {
        WriteSetElementStart ("DgnRuledSweep");
        WriteBool ("capped", data.m_capped);
        WriteArrayElementStart("ListOfContour","Contour");
        for (size_t i = 0; i < data.m_sectionCurves.size (); i++)
            {
            m_dest.WriteArrayMemberStart ();
            WriteNativeCurveVector (*data.m_sectionCurves[i]);
            m_dest.WriteArrayMemberEnd ();
            }
        WriteArrayElementEnd ("ListOfContour","Contour");
        WriteSetElementEnd ("DgnRuledSweep");
        }
    }

void BeCGWriter::Write (ISolidPrimitiveR primitive)
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

void BeCGWriter::Write (IGeometryCR geometry)
    {
    switch (geometry.GetGeometryType ())
        {
        case IGeometry::GeometryType::CurvePrimitive:
            {
            ICurvePrimitivePtr curvePrimitive = geometry.GetAsICurvePrimitive ();

            Write (*curvePrimitive);
            break;
            }

        case IGeometry::GeometryType::CurveVector:
            {
            CurveVectorPtr curveVector = geometry.GetAsCurveVector ();

            Write (*curveVector);
            break;
            }

        case IGeometry::GeometryType::SolidPrimitive:
            {
            ISolidPrimitivePtr solidPrimitive = geometry.GetAsISolidPrimitive ();

            Write (*solidPrimitive);
            break;
            }

        case IGeometry::GeometryType::BsplineSurface:
            {
            MSBsplineSurfacePtr bSurface = geometry.GetAsMSBsplineSurface ();

            WriteSurface (*bSurface);
            break;
            }

        case IGeometry::GeometryType::Polyface:
            {
            PolyfaceHeaderPtr polyface = geometry.GetAsPolyfaceHeader ();
            if (polyface->GetMeshStyle () == MESH_ELM_STYLE_INDEXED_FACE_LOOPS)
                WritePolyface (*polyface);
            else
                {
                auto indexedPolyface = polyface->CloneAsVariableSizeIndexed (*polyface);
                WritePolyface (*indexedPolyface);
                }
            break;
            }

        default:
            {
            BeAssert (false);
            return;
            }
        }
    }

void BeCGWriter::Write(bvector<IGeometryPtr> const &geometry)
    {
    WriteSetElementStart ("Group");
    WriteArrayElementStart ("ListOfMember", "Member");
    for (size_t i = 0; i < geometry.size (); i++)
        {
        Write (*geometry[i]);
        }
    WriteArrayElementEnd ("ListOfMember", "Member");
    WriteSetElementEnd ("Group");
    }
END_BENTLEY_GEOMETRY_NAMESPACE
