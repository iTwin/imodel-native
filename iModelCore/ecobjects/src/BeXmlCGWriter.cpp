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
//=======================================================================================
//! Interface of XmlWriter.  This allows users to pass in different types of writers, like the new binary XML writer in ecobjects
//
// @bsiclass                                                    Carole.MacDonald 09/2014
//=======================================================================================
struct BeCGJsonWriter : IBeXmlWriter
    {
    Utf8String m_string;
    int m_indent;
    int m_indentSize;
    size_t m_numErrors;
    enum class ElementType {
        Null,   // empty output.
        Anonymous, // unnamed element.
        Named,   // simple element that should contain only a value
        Set,    // set name has been output, awaiting elements.
        Array   // array name has been output, awaiting elements.
        };
    // The stack records the state of the elements.
    // The state has two parts:
    //    ElementType -- defined when the element was entered
    //    Count -- number of children so far.
    struct ElementState
        {
        ElementType m_type;
        int m_count;
        ElementState (ElementType type) : m_type (type), m_count(0)
            {
            }
        void IncrementCount () {m_count++;}
        int Count (){return m_count;}
        bool IsEmpty (){return m_count == 0;}
        ElementType Type (){return m_type;}
        };

    bvector<ElementState> m_state;
    ElementType TOSType (){return m_state.empty () ? ElementType::Null : m_state.back ().Type ();}
    bool IsTOSCountZero (){return m_state.empty () ? true : m_state.back ().Count () == 0;}
    void IncrementTOSCount ()
        {
        if (!m_state.empty ())
            m_state.back ().IncrementCount ();
        }


    void PushState (ElementType type)
        {
        //m_indent += m_indentSize;
        m_state.push_back (ElementState (type));
        }
    void PopState ()
        {
        if (!m_state.empty ())
            {
            m_state.pop_back ();
            //m_indent -= m_indentSize;
            }
        }

    // Step = 1 ==> indent before
    // Step = -1 ==> undent after
    // step = 0 ==> no chnage
    void NewLine (int step = 0)
        {
        if (step == 1)
            m_indent += m_indentSize;
        m_string.push_back ('\n');
        if (m_indent > 0)
            for (int i = 0; i < m_indent; i++)
                m_string.push_back (' ');
        if (step == -1)
            m_indent -= m_indentSize;
        }
    void EmitSeparator (Utf8CP separator = ",")
        {
        if (!IsTOSCountZero ())
            Emit (separator);
        IncrementTOSCount ();
        }
    void EmitQuoted (Utf8CP text)
        {
        m_string.push_back ('\"');
        Emit (text);
        m_string.push_back ('\"');
        }

    void Emit (Utf8CP text)
        {
        for (size_t i = 0; text[i] != 0; i++)
            m_string.push_back (text[i]);
        }
    void Error (Utf8CP description)
        {
        NewLine ();
        Emit ("ERROR =");
        Emit (description);
        }        

    public: BeCGJsonWriter (int indentSize)
        {
        m_indent = 0;
        m_indentSize = 2;
        m_numErrors = 0;
        }

    public: BeXmlStatus Status (){ return m_numErrors == 0 ? BEXML_Success : BEXML_ContentWrongType;}
            
    public: void ToString (Utf8StringR dest)
        {
        dest.swap(m_string);
        }


    //! Writes the start of a list element node with the provided name.
    public: BeXmlStatus virtual WriteSetElementStart (Utf8CP name) override
        {
        EmitSeparator ();
        PushState (ElementType::Set);
        NewLine ();
        EmitQuoted (name);
        Emit (":");
        NewLine (1);
        Emit ("{");
        return Status ();
        }

    //! Writes the start of named
    public: BeXmlStatus virtual WriteElementStart (Utf8CP name) override
        {
        EmitSeparator ();
        PushState (ElementType::Named);
        NewLine ();
        EmitQuoted (name);
        Emit (":");
        return Status ();
        }

    //! Writes the start of an array element node with the provided name.
    public: BeXmlStatus virtual WriteArrayElementStart (Utf8CP longName, Utf8CP shortName) override
        {
        EmitSeparator ();
        PushState (ElementType::Array);
        NewLine ();
        EmitQuoted (shortName);
        Emit (":");
        NewLine (1);
        Emit ("[");
        return Status ();
        }

    //! Writes the start of an anonymous
    public: BeXmlStatus virtual WriteShortElementStart (Utf8CP name) override
        {
        EmitSeparator ();
        PushState (ElementType::Anonymous);
        NewLine ();
        return Status ();
        }


    //! Writes the end of an element node.
    public: BeXmlStatus virtual WriteElementEnd () override
        {
        ElementType type = TOSType ();
        switch (type)
            {
            case ElementType::Null:
                {
                Error ("WriteElementEnd in Null State");
                break;
                }
            case ElementType::Set:
                {
                NewLine (-1);
                Emit ("}");
                PopState ();
                break;
                }
            case ElementType::Array:
                {
                NewLine (-1);
                Emit ("]");
                PopState ();
                break;
                }
            case ElementType::Anonymous:
                {
                PopState ();
                break;
                }
            case ElementType::Named:
                {
                PopState ();
                break;
                }
            }
        return Status ();
        }

    //! Writes a text node (plain string as content).
    public: BeXmlStatus virtual WriteText (WCharCP) override
        {
        Error ("Usupported WCharCP text");
        return Status ();
        }
    //! Writes a text node (plain string as content). 
    public: BeXmlStatus virtual WriteText(Utf8CP value) override
        {
        EmitQuoted (value);
        return Status ();
        }
    };

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

void BeXmlCGWriter::WriteList (IBeXmlWriterR dest, bvector<DPoint3d> const & data, Utf8CP listName, Utf8CP shortName, Utf8CP itemName)
    {
    if (0 == data.size())
        return;

    dest.WriteArrayElementStart (listName, shortName);
    for (size_t i = 0 ; i < data.size (); i++)
        WriteXYZ (dest, itemName, data[i], true);
    dest.WriteElementEnd ();
    }

void BeXmlCGWriter::WriteList (IBeXmlWriterR dest, bvector<DVec3d> const & data, Utf8CP listName, Utf8CP shortName, Utf8CP itemName)
    {
    if (0 == data.size())
        return;

    dest.WriteArrayElementStart (listName, shortName);
    for (size_t i = 0 ; i < data.size (); i++)
        {
        Indent (dest);
        double refValue = data[i].Magnitude ();
        WriteXYZ (dest, itemName,
                    SuppressNearZeroCoordinate (data[i].x, refValue),
                    SuppressNearZeroCoordinate (data[i].y, refValue),
                    SuppressNearZeroCoordinate (data[i].z, refValue), true);
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

void BeXmlCGWriter::WriteList (IBeXmlWriterR dest, bvector<double> const & data, Utf8CP listName, Utf8CP shortName, Utf8CP itemName)
    {
    if (0 == data.size())
        return;

    dest.WriteArrayElementStart (listName, shortName);
    for (size_t i = 0 ; i < data.size (); i++)
        WriteDouble (dest, itemName, data[i], true);
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
    dest.WriteSetElementStart ("placement");
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
    dest.WriteSetElementStart ("placement");
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

void BeXmlCGWriter::WriteOptionalName (IBeXmlWriterR dest, Utf8CP name, bool shortName)
    {
    BeXmlStatus s;
    if (shortName)
        s = dest.WriteShortElementStart (name);
    else
        s = dest.WriteElementStart (name);
    }

void BeXmlCGWriter::WriteXYZ (IBeXmlWriterR dest, Utf8CP name, DPoint3dCR xyz, bool shortName)
    {
    char buffer[1024];
    BeStringUtilities::Snprintf (buffer, _countof (buffer), "%.17G,%.17G,%.17G", xyz.x, xyz.y, xyz.z);
    BeXmlStatus s;
    WriteOptionalName (dest, name, shortName);
    s = dest.WriteText (buffer);
    s = dest.WriteElementEnd (); 
    }

void BeXmlCGWriter::WriteText (IBeXmlWriterR dest, Utf8CP name, Utf8CP data)
    {
    dest.WriteElementStart (name);
    dest.WriteText (data);
    dest.WriteElementEnd (); 
    }


void BeXmlCGWriter::WriteXY (IBeXmlWriterR dest, Utf8CP name, double x, double y, bool shortName)
    {
    char buffer[1024];
    BeStringUtilities::Snprintf (buffer, _countof (buffer), "%.17G,%.17G", x, y);
    WriteOptionalName (dest, name, shortName);
    dest.WriteText (buffer);
    dest.WriteElementEnd (); 
    }

void BeXmlCGWriter::WriteXYZ (IBeXmlWriterR dest, Utf8CP name, double x, double y, double z, bool shortName)
    {
    char buffer[1024];
    BeStringUtilities::Snprintf (buffer, _countof (buffer), "%.17G,%.17G,%.17G", x, y, z);
    WriteOptionalName (dest, name, shortName);
    dest.WriteText (buffer);
    dest.WriteElementEnd (); 
    }

void BeXmlCGWriter::WriteDouble (IBeXmlWriterR dest, Utf8CP name, double data, bool shortName)
    {
    WriteOptionalName (dest, name, shortName);
    MSXmlBinaryWriter* binWriter = dynamic_cast<MSXmlBinaryWriter*>(&dest);
    if (nullptr == binWriter)
        {
        char buffer[1024];
        BeStringUtilities::Snprintf (buffer, _countof (buffer), "%.17G", data);
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
        char buffer[1024];
        BeStringUtilities::Snprintf (buffer, _countof (buffer), "%d", data);
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
            dest.WriteText ("true");
        else
            dest.WriteText ("false");
        }
    else
        binWriter->WriteBoolText(data);
    dest.WriteElementEnd (); 
    }


void BeXmlCGWriter::WriteSegment (IBeXmlWriterR dest, DSegment3dCR data)
    {
    dest.WriteSetElementStart ("LineSegment");
    WriteXYZ (dest, "startPoint", data.point[0]);
    WriteXYZ (dest, "endPoint", data.point[1]);    
    dest.WriteElementEnd ();
    }

void BeXmlCGWriter::WriteArc (IBeXmlWriterR dest, DEllipse3dCR arc)
    {
    if (arc.IsCircular ())
        {
        DEllipse3d majorMinorArc = DEllipse3d::FromPerpendicularAxes (arc);
        dest.WriteSetElementStart ("CircularArc");
        WritePlacementXY (dest,  majorMinorArc.center, majorMinorArc.vector0, majorMinorArc.vector90);
        WriteDouble (dest, "radius",     majorMinorArc.vector0.Magnitude ());
        WriteDouble (dest, "startAngle",  Angle::RadiansToDegrees (majorMinorArc.start));
        WriteDouble (dest, "sweepAngle",    Angle::RadiansToDegrees (majorMinorArc.sweep));
        dest.WriteElementEnd ();
        }
    else
        {
        DEllipse3d majorMinorArc = DEllipse3d::FromPerpendicularAxes (arc);
        dest.WriteSetElementStart ("EllipticArc");
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
        dest.WriteSetElementStart ("CircularDisk");
        WritePlacementXY (dest, majorMinorArc.center, majorMinorArc.vector0, yVector);
        WriteDouble (dest, "radius",     majorMinorArc.vector0.Magnitude ());
        dest.WriteElementEnd ();
        }
    else
        {
        dest.WriteSetElementStart ("EllipticDisk");
        WritePlacementXY (dest, majorMinorArc.center, majorMinorArc.vector0, yVector);
        WriteDouble (dest, "radiusA",     majorMinorArc.vector0.Magnitude ());
        WriteDouble (dest, "radiusB",     majorMinorArc.vector90.Magnitude ());
        dest.WriteElementEnd ();
        }
    }


void BeXmlCGWriter::WritePolyface (IBeXmlWriterR dest, PolyfaceHeader &mesh)
    {
    dest.WriteSetElementStart ("IndexedMesh");
    WriteList (dest, mesh.Point (), "ListOfCoord", "Coords", "xyz");

    if (mesh.PointIndex ().Active ())
        WriteIndexList (dest, mesh.PointIndex (), "ListOfCoordIndex", "id");

    if (mesh.Param ().Active ())
        WriteList (dest, mesh.Param (), "ListOfParam", "uv");
    if (mesh.ParamIndex ().Active ())
        WriteIndexList (dest, mesh.ParamIndex (), "ListOfParamIndex", "id");

    if (mesh.Normal ().Active ())
        WriteList (dest, mesh.Normal (), "ListOfNormal", "Normals", "normal");
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
    dest.WriteSetElementStart ("BsplineCurve");

    WriteInt (dest, "Order", curve.params.order);

    WriteBool (dest, "Closed", curve.params.closed? true : false);

    bvector<DPoint3d> poles;
    poles.assign (curve.poles, curve.poles + curve.NumberAllocatedPoles ());
    WriteList (dest, poles, "ListOfControlPoint", "ControlPoints", "xyz");
    
    if (curve.rational)
        {
        bvector<double> weights;
        weights.assign (curve.weights, curve.weights + curve.NumberAllocatedPoles ());
        WriteList (dest, weights, "ListOfWeight", "Weights", "Weight");
        }

    bvector<double> knots;
    knots.assign (curve.knots, curve.knots + curve.NumberAllocatedKnots ());
    WriteList (dest, knots, "ListOfKnot", "Knots", "Knot");

    dest.WriteElementEnd ();
    }


void BeXmlCGWriter::WriteSurface (IBeXmlWriterR dest, MSBsplineSurfaceCR surface)
    {
    dest.WriteSetElementStart ("BsplineSurface");

    WriteInt (dest, "OrderU", surface.uParams.order);
    WriteBool (dest, "ClosedU", surface.uParams.closed ? true : false);
    WriteInt (dest, "NumUControlPoint", surface.uParams.numPoles);

    WriteInt (dest, "OrderV", surface.vParams.order);
    WriteBool (dest, "ClosedV", surface.vParams.closed ? true : false);
    WriteInt (dest, "NumVControlPoint", surface.vParams.numPoles);

    size_t totalPoles = (size_t)surface.uParams.numPoles * (size_t)surface.vParams.numPoles;
    bvector<DPoint3d> poles;
    poles.assign (surface.poles, surface.poles + totalPoles);
    WriteList (dest, poles, "ListOfControlPoint", "ControlPoints", "xyz");
    
    if (surface.rational)
        {
        bvector<double> weights;
        weights.assign (surface.weights, surface.weights + totalPoles);
        WriteList (dest, weights, "ListOfWeight", "Weights", "Weight");
        }

    bvector<double> uknots;
    int numUKnots = surface.uParams.NumberAllocatedKnots ();
    uknots.assign (surface.uKnots, surface.uKnots + numUKnots);
    WriteList (dest, uknots, "ListOfKnotU", "KnotsU", "KnotU");

    bvector<double> vknots;
    int numVKnots = surface.vParams.NumberAllocatedKnots ();
    vknots.assign (surface.vKnots, surface.vKnots + numVKnots);
    WriteList (dest, vknots, "ListOfKnotV", "KnotsV", "KnotV");

    dest.WriteElementEnd ();
    }


void BeXmlCGWriter::WriteTextPlacement
(
IBeXmlWriterR dest,
DPoint3dCR xyz,
Utf8CP data,
double charSize
)
    {
    dest.WriteSetElementStart ("SingleLineText");
    WritePlacementZX (dest, xyz, DVec3d::From (0,0,1), DVec3d::From (1,0,0));
    Indent (dest); WriteText (dest, "textString", data);
    Indent (dest); WriteText (dest, "fontName", "ARIAL");
    Indent (dest); WriteDouble (dest, "characterXSize", charSize);
    Indent (dest); WriteDouble (dest, "characterYSize", charSize);
    Indent (dest); WriteDouble (dest, "slantAngle", 0.0);
    dest.WriteElementEnd ();
    }


void BeXmlCGWriter::WriteLineString (IBeXmlWriterR dest, bvector<DPoint3d> const &points)
    {
    dest.WriteSetElementStart ("LineString");
    WriteList (dest, points, "ListOfPoint", "Points", "xyz");
    dest.WriteElementEnd ();
    }

void BeXmlCGWriter::WriteCoordinate (IBeXmlWriterR dest, DPoint3dCR point)
    {
    dest.WriteSetElementStart ("Coordinate");
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
        dest.WriteSetElementStart ("PointChain");
        dest.WriteSetElementStart ("ListOfPoint");  // Needs work -- PointString is somehow not the same as points in linestring?
        for (size_t i = 0; i < points.size (); i++)
            WriteCoordinate (dest, points[i]);
        dest.WriteElementEnd ();
        dest.WriteElementEnd ();
        }
    }


void BeXmlCGWriter::WritePolygon (IBeXmlWriterR dest, bvector<DPoint3d> const &points)
    {
    dest.WriteSetElementStart ("Polygon");
    WriteList (dest, points, "ListOfPoint", "Points", "xyz");
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
                dest.WriteSetElementStart ("CurveChain");
                    dest.WriteArrayElementStart ("ListOfCurve", "Curves");
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
                    dest.WriteArrayElementStart ("ListOfCurve", "Curves");
                    for(ICurvePrimitivePtr curve : curveVector)
                        Write (dest, *curve);
                    dest.WriteElementEnd ();
                dest.WriteElementEnd ();
                }
            break;
            }


        case CurveVector::BOUNDARY_TYPE_ParityRegion:
            {
            dest.WriteSetElementStart ("SurfacePatch");
            bvector <ICurvePrimitivePtr> holeLoop;
            int n = 0;
            for(ICurvePrimitivePtr curve : curveVector)
                {
                // ASSUME first child is outer loop...
                if (NULL != curve->GetChildCurveVectorCP ())
                    {
                    if (n++ == 0)
                        {
                        dest.WriteSetElementStart ("Exterior");
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
                dest.WriteArrayElementStart ("ListOfHoleLoop","Holes");
                for (size_t i=0; i<holeLoop.size (); i++)
                    Write (dest, *holeLoop[i]->GetChildCurveVectorCP (), false);
                dest.WriteElementEnd ();
                }
            dest.WriteElementEnd ();
            break;
            }
        
        case CurveVector::BOUNDARY_TYPE_UnionRegion:
            {
            dest.WriteSetElementStart ("SurfaceGroup");
            for(ICurvePrimitivePtr curve : curveVector)
                Write (dest, *curve->GetChildCurveVectorCP ());
            dest.WriteElementEnd ();
            
            break;
            }

        case CurveVector::BOUNDARY_TYPE_None:
            {
            dest.WriteSetElementStart ("Group");
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
        dest.WriteSetElementStart ("TorusPipe");
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
                dest.WriteSetElementStart ("CircularCylinder");
                WritePlacementZX (dest, centerA, unitZ, unitX);
                WriteDouble (dest, "height", height);
                WriteDouble (dest, "radius", radiusA);
                WriteBool (dest, "bSolidFlag", data.m_capped);
                dest.WriteElementEnd ();
                }
            else
                {
                dest.WriteSetElementStart ("CircularCone");
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
            dest.WriteSetElementStart ("SkewedCone");
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
        dest.WriteSetElementStart ("Block");
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
        dest.WriteSetElementStart ("Sphere");
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
        dest.WriteSetElementStart (data.m_capped ? "SolidBySweptSurface" : "SurfaceBySweptCurve");
            dest.WriteSetElementStart ("BaseGeometry");
            Write (dest, *data.m_baseCurve);
            dest.WriteElementEnd ();
            DSegment3d segment;
            segment.point[0] = curveStart;
            segment.point[1].SumOf (curveStart, data.m_extrusionVector);
            dest.WriteSetElementStart ("RailCurve");
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
        dest.WriteSetElementStart (data.m_capped ? "SolidBySweptSurface" : "SurfaceBySweptCurve");
            dest.WriteSetElementStart ("BaseGeometry");
            Write (dest, *data.m_baseCurve);
            dest.WriteElementEnd ();
            DEllipse3d arc = BuildSweepArc (*data.m_baseCurve, localToWorld, worldToLocal, data.m_sweepAngle);
            dest.WriteSetElementStart ("RailCurve");
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

//========================================================================

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

void BeXmlCGWriter::WriteJson(Utf8StringR cgBeXml, IGeometryPtr data)
    {
    cgBeXml.clear();
    
    BeCGJsonWriter writer (2);
    writer.Emit ("{\n");
    Write(writer, data);
    writer.Emit ("\n}");
    writer.ToString(cgBeXml);
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
