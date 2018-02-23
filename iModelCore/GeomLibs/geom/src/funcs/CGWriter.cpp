/*--------------------------------------------------------------------------------------+
|
|  $Source: geom/src/funcs/CGWriter.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#pragma push_macro("max")
#pragma push_macro("min")
#undef max
#undef min
#include <Objidl.h>
#pragma pop_macro("max")
#pragma pop_macro("min")

BEGIN_BENTLEY_GEOMETRY_NAMESPACE
static double s_nearZero = 9.0e-16;
CGWriter::TagFrame::TagFrame (std::wstring &tagName)
    {
    mTagName = tagName;
    }
CGWriter::TagFrame::TagFrame (wchar_t const *tagName)
    {
    mTagName = std::wstring (tagName);
    }

static wchar_t const *sNamespaceAttribute = L"xmlns=\"http://www.bentley.com/schemas/Bentley.Geometry.Common.1.0\"";


double CGWriter::SuppressNearZeroCoordinate (double x, double refValue)
    {
    double tol = s_nearZero * refValue;
    return fabs (x) < tol ? 0.0 : x;
    }

void CGWriter::InitReadable ()
    {
    mMaxIndexPerLine = 8;
    mMinPackedIndexPerLine = 20;    // 
    mMaxPackedIndexPerLine = 25;
    mIndent = 2;
    }

void CGWriter::SetFormatMask (wchar_t const*name)
    {
    if (0 == wcsicmp (name, L"CSL"))
        {
        m_formatMasks |= 0x01;
        }
    }

void CGWriter::Write (wchar_t const *cString)
    {
    int numOutputs = 0;
    if (NULL != mFile)
        {
        fwprintf (mFile, cString);
        numOutputs++;
        //fprintf (mFile, cString);
        }

    if (NULL != mStream)
        {
        int n = 0;
        for (; cString[n] != 0;)
            {
            n++;
            }
        if (n > 0)
        mStream->Write (cString, n, NULL);
        numOutputs++;
        }

    if (numOutputs == 0)
        GEOMAPI_PRINTF (Utf8String(cString).c_str());
    }

void CGWriter::WriteStartTag (wchar_t const *name)
    {
    Write(L"<");
    Write(name);
    Write(L">");
    }

void CGWriter::WriteEndTag (wchar_t const *name, bool lineFeed)
    {
    Write(L"</");
    Write(name);
    Write(L">");
    if (lineFeed)
        Write (L"\n");
    }

void CGWriter::WriteEndTag (wchar_t const *name)
    {
    WriteEndTag (name, true);
    }

void CGWriter::WriteFormattedW (wchar_t const *cFormatString, wchar_t const *wString)
    {
    wchar_t buffer[2048];
    wsprintfW (buffer, cFormatString, wString);
    Write (buffer);
    }

#define SNBufferSize 1024
void CGWriter::WriteXYZ (double x, double y, double z)
    {
    wchar_t buffer[SNBufferSize];
    _snwprintf_s (buffer, SNBufferSize, _TRUNCATE, L"%.17g,%.17g,%.17g", x, y, z);
    Write (buffer);
    }

void CGWriter::WriteXY (double x, double y)
    {
    wchar_t buffer[SNBufferSize];
    _snwprintf_s (buffer,SNBufferSize, _TRUNCATE,  L"%.17lg,%.17lg", x, y);
    Write (buffer);
    }



void CGWriter::WriteDouble (double x)
    {
    wchar_t buffer[SNBufferSize];
    _snwprintf_s (buffer, SNBufferSize, _TRUNCATE, L"%.17lg", x);
    Write (buffer);
    }

void CGWriter::WriteInt (int x)
    {
    wchar_t buffer[1024];
    wsprintfW (buffer, L"%d", x);
    Write (buffer);
    }

void CGWriter::WriteSize (size_t x)
    {
    wchar_t buffer[1024];
    wsprintfW (buffer, L"%d", (int)x);
    Write (buffer);
    }


void CGWriter::WriteBool (bool x)
    {
    if (x)
        Write (L"true");
    else
        Write (L"false");
    }

CGWriter::CGWriter (FILE *file)
    {
    mFile = file;
    mStream = NULL;
    InitReadable ();
    }

CGWriter::CGWriter (IStream *stream)
    {
    mStream = stream;
    mFile = NULL;
    InitReadable ();
    }

void CGWriter::Indent (size_t numBack)
    {
    for (size_t i = 0, n = mIndent * (mStack.size () - numBack); i < n; i++)
        Write (L" ");
    }

void CGWriter::StartTag (wchar_t const *tagName, bool includeNameSpaceAttribute)
    {
    Indent ();
    mStack.push_back (TagFrame (tagName));
    WriteFormattedW (L"<%s", tagName);
    if (includeNameSpaceAttribute)
        WriteFormattedW (L" %s", sNamespaceAttribute);
    Write (L">\n");
    }

void CGWriter::EndTag ()
    {
    if (mStack.size () > 0)
        {
        Indent (1);
        WriteEndTag (mStack.back ().mTagName.c_str()); 
        mStack.pop_back ();
        }
    }

void CGWriter::EmitDPoint3d (wchar_t const *name, DPoint3dCR value, bool indent)
    {
    if (indent)
        Indent ();
    double refValue = value.Magnitude ();
    WriteStartTag (name);
    WriteXYZ (
             SuppressNearZeroCoordinate (value.x, refValue),
             SuppressNearZeroCoordinate (value.y, refValue),
             SuppressNearZeroCoordinate (value.z, refValue)
             );
    WriteEndTag (name, indent);
    }

void CGWriter::EmitDPoint2d (wchar_t const *name, DPoint2dCR value, bool indent)
    {
    if (indent)
        Indent ();
    double refValue = value.Magnitude ();
    WriteStartTag (name);
    WriteXY (
             SuppressNearZeroCoordinate (value.x, refValue),
             SuppressNearZeroCoordinate (value.y, refValue)
             );
    WriteEndTag (name, indent);
    }


void CGWriter::EmitInt (wchar_t const *name, int data)
    {
    Indent ();
    WriteStartTag (name);
    WriteInt (data);
    WriteEndTag (name);
    }

void CGWriter::EmitSize (wchar_t const *name, size_t data)
    {
    Indent ();
    WriteStartTag (name);
    WriteSize (data);
    WriteEndTag (name);
    }



void CGWriter::EmitDouble (wchar_t const *name, double data)
    {
    Indent ();
    WriteStartTag (name);
    WriteDouble (data);
    WriteEndTag (name);
    }

void CGWriter::EmitBool (wchar_t const *name, bool data)
    {
    Indent ();
    WriteStartTag (name);
    WriteBool (data);
    WriteEndTag (name);
    }

void CGWriter::Emit (bvector<DPoint3d>& data, wchar_t const *listName, wchar_t const *itemName)
    {
    StartTag (listName);
    for (size_t i = 0 ; i < data.size (); i++)
        EmitDPoint3d (itemName, data[i]);
    EndTag ();
    }

void CGWriter::Emit (bvector<DVec3d>& data, wchar_t const *listName, wchar_t const *itemName)
    {
    StartTag (listName);
    for (size_t i = 0 ; i < data.size (); i++)
        {
        Indent ();
        double refValue = data[i].Magnitude ();
        WriteStartTag (itemName);
        WriteXYZ (
                    SuppressNearZeroCoordinate (data[i].x, refValue),
                    SuppressNearZeroCoordinate (data[i].y, refValue),
                    SuppressNearZeroCoordinate (data[i].z, refValue));
        WriteEndTag (itemName);
        }
    EndTag ();
    }

void CGWriter::Emit (bvector<RgbFactor>& data, wchar_t const *listName, wchar_t const *itemName)
    {
    StartTag (listName);
    for (size_t i = 0 ; i < data.size (); i++)
        {
        Indent ();
        WriteStartTag (itemName);
        WriteXYZ (data[i].red, data[i].green, data[i].blue);
        WriteEndTag (itemName);
        }
    EndTag ();
    }

void BlockedIndent (CGWriter &writer, size_t i, size_t n, size_t perLine)
    {
    if (i % perLine == 0)
        writer.Indent ();
    }

void BlockedLineFeed (CGWriter &writer, size_t i, size_t n, size_t perLine)
    {
    size_t j = i + 1;
    if (j % perLine == 0
        || j >= n)
        writer.Write (L"\n");
    }


void CGWriter::Emit (bvector<DPoint2d>& data, wchar_t const *listName, wchar_t const *itemName)
    {
    StartTag (listName);
    for (size_t i = 0 ; i < data.size (); i++)
        {
        BlockedIndent (*this, i, data.size (), 3);
        EmitDPoint2d (itemName, data[i], false);
        BlockedLineFeed (*this, i, data.size (), 3);
        }
    EndTag ();
    }


void CGWriter::EmitIndexVector (bvector<int>& data, wchar_t const *listName, wchar_t const *itemName)
    {
    StartTag (listName);
    if (!(m_formatMasks & 0x01))
        {
        size_t numThisLine = 0;
        for (size_t i = 0 ; i < data.size (); i++)
            {
            if (numThisLine == 0)
                Indent ();
            WriteStartTag (itemName);
            WriteInt (data[i]);
            numThisLine++;
            bool needLineFeed = (numThisLine > mMaxIndexPerLine) || data[i] == 0;
            WriteEndTag (itemName, needLineFeed);
            if (needLineFeed)
                numThisLine = 0;
            }
        }
    else
        {
        size_t numThisLine = 0;
        WriteStartTag (itemName);
        for (size_t i = 0 ; i < data.size (); i++)
            {
            if (numThisLine == 0 && i > 0)
                {
                Write (L"\n");
                Indent ();
                }
            WriteInt (data[i]);
            if (i < data.size () - 1)
                Write (L",");
            numThisLine++;
            bool needLineFeed = (numThisLine > mMaxPackedIndexPerLine)
                    || (data[i] == 0 && numThisLine > mMinPackedIndexPerLine);
            if (needLineFeed)
                numThisLine = 0;
            else if (data[i] == 0)
                Indent ();
            }
        WriteEndTag (itemName, true);
        }
    EndTag ();
    }

void CGWriter::Emit (bvector<double>& data, wchar_t const *listName, wchar_t const *itemName)
    {
    StartTag (listName);
    for (size_t i = 0 ; i < data.size (); i++)
        EmitDouble (itemName, data[i]);
    EndTag ();
    }

static void WriteDRange2d (CGWriter &writer, wchar_t const *name, DRange2dCR data)
    {
    if (!data.IsNull ())
        {
        writer.Indent ();
        writer.WriteStartTag (name);
#ifdef DRange_Subtags
        writer.EmitDPoint2d (L"lo", data.low);
        writer.EmitDPoint2d (L"hi", data.high);
#else
        writer.WriteDouble (data.low.x);writer.Write (L",");
        writer.WriteDouble (data.low.y);writer.Write (L",");
        writer.WriteDouble (data.high.x);writer.Write (L",");
        writer.WriteDouble (data.high.y);
#endif
        writer.WriteEndTag   (name);
        }
    }

#ifdef FACET_FACE_DATA_NOT_USED
static void WriteDRange3d (CGWriter &writer, wchar_t const *name, DRange3dCR data)
    {
    if (!data.IsNull ())
        {
        writer.Indent ();
        writer.WriteStartTag (name);
#ifdef DRange_Subtags
        writer.EmitDPoint3d (L"lo", data.low);
        writer.EmitDPoint3d (L"hi", data.high);
#else
        writer.WriteDouble (data.low.x);writer.Write (L",");
        writer.WriteDouble (data.low.y);writer.Write (L",");
        writer.WriteDouble (data.low.z);writer.Write (L",");
        writer.WriteDouble (data.high.x);writer.Write (L",");
        writer.WriteDouble (data.high.y);writer.Write (L",");
        writer.WriteDouble (data.high.z);
#endif
        writer.WriteEndTag   (name);
        }
    }
#endif

static void EmitFaceData (CGWriter &writer, wchar_t const *name, FacetFaceDataCR data)
    {
    writer.StartTag (name);
#ifdef FACET_FACE_DATA_NOT_USED
        writer.EmitSize (L"sourceIndex", data.m_sourceIndex);
        writer.WriteStartTag (L"FaceIndex");
        writer.WriteInt ((int)data.m_faceIndices.Index0 ()); writer.Write (L",");
        writer.WriteInt ((int)data.m_faceIndices.Index1 ()); writer.Write (L",");
        writer.WriteInt ((int)data.m_faceIndices.Index2 ());
        writer.WriteEndTag (L"FaceIndex");
#endif
        WriteDRange2d (writer, L"distanceRange", data.m_paramDistanceRange);
        WriteDRange2d (writer, L"paramRange", data.m_paramRange);
#ifdef FACET_FACE_DATA_NOT_USED
        WriteDRange3d (writer, L"xyzRange", data.m_xyzRange);
        WriteDRange3d (writer, L"normalRange", data.m_normalRange);
#endif
    writer.EndTag ();
    }

void CGWriter::EmitPolyface (PolyfaceHeader &mesh)
    {
    static int s_coordinatePhase = 2;
    StartTag (L"IndexedMesh", true);
    Emit (mesh.Point (), L"ListOfCoord", L"xyz");

    if (mesh.PointIndex ().Active ())
        EmitIndexVector (mesh.PointIndex (), L"ListOfCoordIndex", L"id");

    if (mesh.Param ().Active ())
        Emit (mesh.Param (), L"ListOfParam", L"uv");
    if (mesh.ParamIndex ().Active ())
        EmitIndexVector (mesh.ParamIndex (), L"ListOfParamIndex", L"id");

    if (mesh.Normal ().Active ())
        Emit (mesh.Normal (), L"ListOfNormal", L"normal");
    if (mesh.NormalIndex ().Active ())
        EmitIndexVector (mesh.NormalIndex (), L"ListOfNormalIndex", L"id");

#if defined (NOT_NOW_MESH_COLOR)
    if (mesh.DoubleColor ().Active ())
        Emit (mesh.DoubleColor (), L"ListOfColor", L"Color");
    if (mesh.ColorIndex ().Active ())
        EmitIndexVector (mesh.ColorIndex (), L"ListOfColorIndex", L"id");
#endif

    if (mesh.FaceData ().size () > 0)
        {
        StartTag (L"ListOfFaceData");
        for (size_t i = 0; i < mesh.FaceData ().size (); i++)
            EmitFaceData (*this, L"FaceData", mesh.FaceData ()[i]);
        EndTag ();
        }
    EndTag ();
    }

void CGWriter::EmitCurve (MSBsplineCurveCR curve)
    {
    StartTag (L"BsplineCurve", mStack.size () == 0 ? true : false);

    EmitInt (L"Order", curve.params.order);

    EmitBool (L"Closed", curve.params.closed? true : false);

    bvector<DPoint3d> poles;
    curve.GetPoles (poles);
    Emit (poles, L"ListOfControlPoint", L"ControlPoint");
    
    if (curve.rational)
        {
        bvector<double> weights;
        weights.assign (curve.weights, curve.weights + curve.NumberAllocatedPoles ());
        Emit (weights, L"ListOfWeight", L"Weight");
        }

    bvector<double> knots;
    knots.assign (curve.knots, curve.knots + curve.NumberAllocatedKnots ());
    Emit (knots, L"ListOfKnot", L"Knot");

    EndTag ();
    }

void CGWriter::EmitSurface (MSBsplineSurfaceCR surface)
    {
    StartTag (L"BsplineSurface", mStack.size () == 0 ? true : false);

    EmitInt (L"OrderU", surface.GetIntUOrder ());
    EmitInt (L"OrderV", surface.GetIntVOrder ());

    EmitInt (L"numUControlPoint", surface.GetIntNumUPoles ());
    EmitInt (L"numVControlPoint", surface.GetIntNumVPoles ());

    EmitBool (L"ClosedU", surface.GetIsUClosed ());
    EmitBool (L"ClosedV", surface.GetIsVClosed ());

    bvector<DPoint3d> poles;
    surface.GetPoles (poles);
    Emit (poles, L"ListOfControlPoint", L"ControlPoint");
    
    if (surface.rational)
        {
        bvector<double> weights;
        weights.assign (surface.weights, surface.weights + surface.GetNumPoles ());
        Emit (weights, L"ListOfWeight", L"Weight");
        }

    bvector<double> knots;
    surface.GetUKnots (knots);
    Emit (knots, L"ListOfKnotU", L"KnotU");
    surface.GetVKnots (knots);
    Emit (knots, L"ListOfKnotV", L"KnotV");

    EndTag ();
    }




void CGWriter::EmitPlacementZX (DPoint3dCR origin, DVec3dCR vectorZ, DVec3dCR vectorX)
    {
    StartTag (L"placement");
    EmitDPoint3d (L"origin", origin);
    DVec3d unitZ, unitY, unitX;
    unitZ.Normalize (vectorZ);
    unitY.NormalizedCrossProduct (unitZ, vectorX);
    unitX.NormalizedCrossProduct (unitY, unitZ);
    EmitDPoint3d (L"vectorZ", unitZ);
    EmitDPoint3d (L"vectorX", unitX);
    EndTag ();
    }

void CGWriter::EmitPlacementZX (DPoint3dCR origin, RotMatrixCR axes)
    {
    DVec3d vectorX, vectorY, vectorZ;
    axes.GetColumns (vectorX, vectorY, vectorZ);
    StartTag (L"placement");
    EmitDPoint3d (L"origin", origin);
    DVec3d unitZ, unitY, unitX;
    unitZ.Normalize (vectorZ);
    unitY.NormalizedCrossProduct (unitZ, vectorX);
    unitX.NormalizedCrossProduct (unitY, unitZ);
    EmitDPoint3d (L"vectorZ", unitZ);
    EmitDPoint3d (L"vectorX", unitX);
    EndTag ();
    }


void CGWriter::EmitPlacementXY (DPoint3dCR origin, DVec3dCR vectorX, DVec3dCR vectorY)
    {
    DVec3d unitNormal, unitX;
    unitNormal.NormalizedCrossProduct (vectorX, vectorY);
    unitX.NormalizedCrossProduct (vectorY, unitNormal);
    EmitPlacementZX (origin, unitNormal, unitX);
    }

void CGWriter::EmitText
(
DPoint3dCR xyz,
wchar_t const *text,
double charSize
)
    {
    StartTag (L"SingleLineText", true);
    EmitPlacementZX (xyz, DVec3d::From (0,0,1), DVec3d::From (1,0,0));
    Indent (); WriteStartTag (L"textString");Write (text); WriteEndTag (L"textString");
    Indent (); WriteStartTag (L"fontName");Write (L"ARIAL"); WriteEndTag (L"fontName");
    Indent (); WriteStartTag (L"characterXSize");WriteDouble (charSize); WriteEndTag (L"characterXSize");
    Indent (); WriteStartTag (L"characterYSize");WriteDouble (charSize); WriteEndTag (L"characterYSize");
    Indent (); WriteStartTag (L"slantAngle");WriteInt (0); WriteEndTag (L"slantAngle");
    EndTag ();
    }


void CGWriter::EmitLineSegment
(
DSegment3dCR segment
)
    {
    StartTag (L"LineSegment", mStack.size () == 0 ? true : false);
    EmitDPoint3d (L"startPoint", segment.point[0]);
    EmitDPoint3d (L"endPoint", segment.point[1]);
    EndTag ();
    }

void CGWriter::EmitLineString
(
bvector<DPoint3d> const &points
)
    {
    StartTag (L"LineString", mStack.size () == 0 ? true : false);
        StartTag (L"ListOfPoint");
        for (size_t i = 0; i < points.size (); i++)
            {
            EmitDPoint3d (L"xyz", points[i]);
            }
        EndTag ();
    EndTag ();
    }

void CGWriter::EmitPointString
(
bvector<DPoint3d> const &points
)
    {
    StartTag (L"PointChain", mStack.size () == 0 ? true : false);
        StartTag (L"ListOfPoint");
        for (size_t i = 0; i < points.size (); i++)
            {
            EmitDPoint3d (L"xyz", points[i]);
            }
        EndTag ();
    EndTag ();
    }

void CGWriter::EmitPartialCurve
(
PartialCurveDetailCP detail
)
    {
    StartTag (L"PartialCurve", mStack.size () == 0 ? true : false);
    Indent (); WriteStartTag (L"fraction0");WriteDouble (detail->fraction0); WriteEndTag (L"fraction0");
    Indent (); WriteStartTag (L"fraction1");WriteDouble (detail->fraction1); WriteEndTag (L"fraction1");
    EmitCurvePrimitive (*detail->parentCurve);
    EndTag ();
    }

void CGWriter::EmitPolygon
(
bvector<DPoint3d> const &points
)
    {
    StartTag (L"Polygon", mStack.size () == 0 ? true : false);
        StartTag (L"ListOfPoint");
        for (size_t i = 0; i < points.size (); i++)
            {
            EmitDPoint3d (L"xyz", points[i]);
            }
        EndTag ();
    EndTag ();
    }



void CGWriter::EmitArc
(
DEllipse3dCR arc
)
    {
    if (arc.IsCircular ())
        {
        DEllipse3d majorMinorArc = DEllipse3d::FromPerpendicularAxes (arc);
        StartTag (L"CircularArc", mStack.size () == 0 ? true : false);
        EmitPlacementXY (majorMinorArc.center, majorMinorArc.vector0, majorMinorArc.vector90);
        EmitDouble (L"radius",     majorMinorArc.vector0.Magnitude ());
        EmitDouble (L"startAngle",  Angle::RadiansToDegrees (majorMinorArc.start));
        EmitDouble (L"sweepAngle",  Angle::RadiansToDegrees (majorMinorArc.sweep));
        EndTag ();
        }
    else
        {
        DEllipse3d majorMinorArc = DEllipse3d::FromPerpendicularAxes (arc);
        StartTag (L"EllipticArc", mStack.size () == 0 ? true : false);
        EmitPlacementXY (majorMinorArc.center, majorMinorArc.vector0, majorMinorArc.vector90);
        EmitDouble (L"radiusA",     majorMinorArc.vector0.Magnitude ());
        EmitDouble (L"radiusB",     majorMinorArc.vector90.Magnitude ());
        EmitDouble (L"startAngle",  Angle::RadiansToDegrees (majorMinorArc.start));
        EmitDouble (L"sweepAngle",  Angle::RadiansToDegrees (majorMinorArc.sweep));
        EndTag ();
        }
    }

void CGWriter::EmitDisk
(
DEllipse3dCR arc
)
    {
    if (arc.IsCircular ())
        {
        DEllipse3d majorMinorArc = DEllipse3d::FromPerpendicularAxes (arc);
        StartTag (L"CircularDisk", mStack.size () == 0 ? true : false);
        EmitPlacementXY (majorMinorArc.center, majorMinorArc.vector0, majorMinorArc.vector90);
        EmitDouble (L"radius",     majorMinorArc.vector0.Magnitude ());
        EndTag ();
        }
    else
        {
        DEllipse3d majorMinorArc = DEllipse3d::FromPerpendicularAxes (arc);
        StartTag (L"EllipticDisk", mStack.size () == 0 ? true : false);
        EmitPlacementXY (majorMinorArc.center, majorMinorArc.vector0, majorMinorArc.vector90);
        EmitDouble (L"radiusA",     majorMinorArc.vector0.Magnitude ());
        EmitDouble (L"radiusB",     majorMinorArc.vector90.Magnitude ());
        EndTag ();
        }
    }



void CGWriter::EmitDSegment3d 
(
DSegment3dCR segment
)
    {
    StartTag (L"LineSegment", mStack.size () == 0 ? true : false);
    EmitDPoint3d (L"startPoint", segment.point[0]);
    EmitDPoint3d (L"endPoint", segment.point[1]);
    EndTag ();
    }

void CGWriter::EmitDEllipse3dAsArc 
(
DEllipse3d arc
)
    {
    EmitArc (arc);
    }


void CGWriter::EmitCurvePrimitive 
(
ICurvePrimitiveCR curve
)
    {
    switch (curve.GetCurvePrimitiveType ())
        {
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line:
            EmitLineSegment (*curve.GetLineCP ());
            break;

        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString:
            EmitLineString (*curve.GetLineStringCP ());
            break;

        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc:
            EmitArc (*curve.GetArcCP ());
            break;

        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_BsplineCurve:
            EmitCurve (*curve.GetBsplineCurveCP ());
            break;

        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_CurveVector:
            EmitCurveVector (*curve.GetChildCurveVectorCP ());
            break;

        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_PointString:
            EmitPointString (*curve.GetPointStringCP ());
            break;

        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_PartialCurve:
            EmitPartialCurve (curve.GetPartialCurveDetailCP ());
            break;
        default:
            return;
        }
    }

void CGWriter::EmitCurveVector (CurveVectorCR curveVector)
    {
    EmitCurveVector (curveVector, true);
    }

void CGWriter::EmitCurveVector (CurveVectorCR curveVector, bool preferMostCompactPrimitives)
    {
    bvector<DPoint3d> const *points;
    switch (curveVector.GetBoundaryType ())
        {
        case CurveVector::BOUNDARY_TYPE_Outer:
        case CurveVector::BOUNDARY_TYPE_Inner:
            {
            DEllipse3d arc;
            if (preferMostCompactPrimitives
                && curveVector.size () == 1
                && curveVector[0]->TryGetArc (arc)
                && arc.IsFullEllipse ())
                {
                EmitDisk (arc);
                }
            else if (preferMostCompactPrimitives
                && curveVector.size () == 1
                && (nullptr != (points = curveVector[0]->GetLineStringCP ()))
                )
                {
                EmitPolygon (*points);
                }
            else
                {
                StartTag (L"CurveChain", mStack.size () == 0 ? true : false);
                    StartTag (L"ListOfCurve");
                    for(ICurvePrimitivePtr curve : curveVector)
                        EmitCurvePrimitive (*curve);
                    EndTag ();
                EndTag ();
                }
            break;
            }

        case CurveVector::BOUNDARY_TYPE_Open:
            {
            StartTag (L"CurveChain", mStack.size () == 0 ? true : false);
                StartTag (L"ListOfCurve");
                for(ICurvePrimitivePtr curve : curveVector)
                    EmitCurvePrimitive (*curve);
                EndTag ();
            EndTag ();

            break;
            }


        case CurveVector::BOUNDARY_TYPE_ParityRegion:
            {
            StartTag (L"SurfacePatch", mStack.size () == 0 ? true : false);
            bvector <ICurvePrimitivePtr> holeLoop;
            bool firstFound = false;
            // hm.. don't really know what loop is outer.  Ignore it --call the first one exterior.
            for(ICurvePrimitivePtr curve : curveVector)
                {
                if (NULL != curve->GetChildCurveVectorCP ())
                    {
                    if (!firstFound)
                        {
                        StartTag (L"ExteriorLoop");
                        EmitCurveVector (*curve->GetChildCurveVectorCP (), false);
                        firstFound = true;
                        EndTag ();
                        }
                    else
                        {
                        holeLoop.push_back (curve);
                        }
                    }
                }

            if (holeLoop.size () > 0)
                {
                StartTag (L"ListOfHoleLoop");
                for (size_t i=0; i<holeLoop.size (); i++)
                    EmitCurveVector (*holeLoop[i]->GetChildCurveVectorCP (), false);
                EndTag ();
                }
            EndTag ();
            break;
            }
        
        case CurveVector::BOUNDARY_TYPE_UnionRegion:
            {
            StartTag (L"SurfaceGroup", mStack.size () == 0 ? true : false);
            for(ICurvePrimitivePtr curve : curveVector)
                EmitCurveVector (*curve->GetChildCurveVectorCP ());
            EndTag ();
            
            break;
            }

        case CurveVector::BOUNDARY_TYPE_None:
            {
            StartTag (L"Group", mStack.size () == 0 ? true : false);
            for(ICurvePrimitivePtr curve : curveVector)
                EmitCurvePrimitive (*curve);
            EndTag ();
            
            break;
            }
        default:
            return;
        }
    }

void CGWriter::EmitDgnTorusPipeDetail (DgnTorusPipeDetail data)
    {
    DPoint3d center;
    RotMatrix axes;
    double radiusA, radiusB, sweepAngle;
    if (data.TryGetFrame (center, axes, radiusA, radiusB, sweepAngle))
        {
        StartTag (L"TorusPipe");
        EmitPlacementZX (center, axes);
        EmitDouble (L"radiusA", radiusA);
        EmitDouble (L"radiusB", radiusB);
        EmitDouble (L"startAngle", 0.0);
        EmitDouble (L"sweepAngle", Angle::RadiansToDegrees (sweepAngle));
        EndTag ();
        }
    }

void CGWriter::EmitDgnConeDetail (DgnConeDetail data)
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
                StartTag (L"CircularCylinder");
                EmitPlacementZX (centerA, unitZ, unitX);
                EmitDouble (L"radius", radiusA);
                EmitDouble (L"height", height);
                EmitBool (L"bSolidFlag", data.m_capped);
                EndTag ();
                }
            else
                {
                StartTag (L"CircularCone");
                EmitPlacementZX (centerA, unitZ, unitX);
                EmitDouble (L"radiusA", radiusA);
                EmitDouble (L"radiusB", radiusB);
                EmitDouble (L"height", height);
                EmitBool (L"bSolidFlag", data.m_capped);
                EndTag ();
                }
            }
        else
            {
            StartTag (L"SkewedCone");
            EmitPlacementZX (centerA, unitZ, unitX);
            EmitDPoint3d (L"centerB", centerB);
            EmitDouble (L"radiusA", radiusA);
            EmitDouble (L"radiusB", radiusB);
            EmitBool (L"bSolidFlag", data.m_capped);
            EndTag ();
            }
        }
    }

void CGWriter::EmitDgnBoxDetail (DgnBoxDetail data)
    {
    Transform skewFrame, frame, inverseFrame;
    RotMatrix skewAxes, axes;
    DPoint3d corners[8];
    data.GetCorners (corners);
    double ax, ay, bx, by;
    data.GetNonUniformTransform (skewFrame, ax, ay, bx, by);
    if (true)
        {
        //  WARNING -- this squares it all up...
        StartTag (L"Block");
        skewFrame.GetMatrix (skewAxes);
        DVec3d unitX, unitY, unitZ;
        axes.SquareAndNormalizeColumns (skewAxes, 0, 1);
        frame = Transform::From (axes, corners[0]);
        inverseFrame.InverseOf (frame);
        DPoint3d cornerB;
        inverseFrame.Multiply (cornerB, corners[7]);
        axes.GetColumns (unitX, unitY, unitZ);
        EmitPlacementZX (corners[0], unitZ, unitX);
        EmitDPoint3d (L"cornerA", DPoint3d::From (0,0,0));
        EmitDPoint3d (L"cornerB", cornerB);
        EmitBool (L"bSolidFlag", data.m_capped);
        EndTag ();
        }
    }

void CGWriter::EmitDgnSphereDetail (DgnSphereDetail data)
    {
    DPoint3d center;
    RotMatrix axes;
    double radius;
    if (data.IsTrueSphere (center, axes, radius))
        {
        StartTag (L"Sphere");
        DVec3d unitX, unitY, unitZ;
        axes.GetColumns (unitX, unitY, unitZ);
        EmitPlacementZX (center, unitZ, unitX);
        EmitDouble (L"radius", radius);
        EndTag ();
        }
    }

void CGWriter::EmitDgnExtrusionDetail (DgnExtrusionDetail data)
    {
    DPoint3d curveStart, curveEnd;
    if (data.m_baseCurve->GetStartEnd (curveStart, curveEnd))
        {
        StartTag (data.m_capped ? L"SolidBySweptSurface" : L"SurfaceBySweptCurve");
            StartTag (L"BaseGeometry");
            EmitCurveVector (*data.m_baseCurve);
            EndTag ();
            DSegment3d segment;
            segment.point[0] = curveStart;
            segment.point[1].SumOf (curveStart, data.m_extrusionVector);
            StartTag (L"RailCurve");
            EmitDSegment3d (segment);
            EndTag ();
        EndTag ();
        }
    }

static DEllipse3d BuildSweepArc (CurveVectorCR curve,
TransformCR localToWorld,
TransformCR worldToLocal,
double sweepAngle)
    {
    static double s_defaultRadius = 1000.0;
    bvector<DPoint3d>strokes;
    IFacetOptionsPtr options = IFacetOptions::Create ();
    options->SetAngleTolerance (Angle::Pi () / 6.0);
    curve.AddStrokePoints (strokes, *options);
    DEllipse3d localEllipse, worldEllipse;
    // Candidate local point on the ellipse ...
    DPoint3d xyz0 = DPoint3d::From (s_defaultRadius, 0, 0);
    if (strokes.size () > 0)
        {
        DPoint3d xyzMax;
        xyzMax.Zero ();
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

void CGWriter::EmitDgnRotationalSweepDetail (DgnRotationalSweepDetail data)
    {
    Transform localToWorld, worldToLocal;    
    if (data.GetTransforms (localToWorld, worldToLocal))
        {
        StartTag (data.m_capped ? L"SolidBySweptSurface" : L"SurfaceBySweptCurve");
            StartTag (L"BaseGeometry");
            EmitCurveVector (*data.m_baseCurve);
            EndTag ();
            DEllipse3d arc = BuildSweepArc (*data.m_baseCurve, localToWorld, worldToLocal, data.m_sweepAngle);
            StartTag (L"RailCurve");
            EmitDEllipse3dAsArc (arc);
            EndTag ();
        EndTag ();
        }
    }

void CGWriter::EmitDgnRuledSweepDetail (DgnRuledSweepDetail data)
    {
    StartTag (L"DgnRuledSweep");
      EmitBool (L"capped", data.m_capped);
      StartTag (L"ListOfContour");
        for (CurveVectorPtr &section : data.m_sectionCurves)
            {
            //StartTag(L"Contour");
              EmitCurveVector (*section);
            //EndTag ();
            }
      EndTag ();
    EndTag ();
    }


void CGWriter::EmitSolidPrimitive
(
ISolidPrimitiveR primitive
)
    {
    switch (primitive.GetSolidPrimitiveType ())
        {
        case SolidPrimitiveType_DgnTorusPipe:
            {
            DgnTorusPipeDetail detail;
            if (primitive.TryGetDgnTorusPipeDetail (detail))
                EmitDgnTorusPipeDetail (detail);
            }
            break;
        case SolidPrimitiveType_DgnCone:
            {
            DgnConeDetail detail;
            if (primitive.TryGetDgnConeDetail (detail))
                EmitDgnConeDetail (detail);
            }
            break;
        case SolidPrimitiveType_DgnBox:
            {
            DgnBoxDetail detail;
            if (primitive.TryGetDgnBoxDetail (detail))
                EmitDgnBoxDetail (detail);
            }
            break;
        case SolidPrimitiveType_DgnSphere:
            {
            DgnSphereDetail detail;
            if (primitive.TryGetDgnSphereDetail (detail))
                EmitDgnSphereDetail (detail);
            }
            break;
        case SolidPrimitiveType_DgnExtrusion:
            {
            DgnExtrusionDetail detail;
            if (primitive.TryGetDgnExtrusionDetail (detail))
                EmitDgnExtrusionDetail (detail);
            }
            break;
        case SolidPrimitiveType_DgnRotationalSweep:
            {
            DgnRotationalSweepDetail detail;
            if (primitive.TryGetDgnRotationalSweepDetail (detail))
                EmitDgnRotationalSweepDetail (detail);
            }
            break;
        case SolidPrimitiveType_DgnRuledSweep:
            {
            DgnRuledSweepDetail detail;
            if (primitive.TryGetDgnRuledSweepDetail (detail))
                EmitDgnRuledSweepDetail (detail);
            }
            break;
        }
    }

void CGWriter::Emit (IGeometryPtr geometry)
    {
    if (!geometry.IsValid ())
        return;

    switch (geometry->GetGeometryType ())
        {
        case IGeometry::GeometryType::CurvePrimitive:
            {
            ICurvePrimitivePtr curvePrimitive = geometry->GetAsICurvePrimitive ();

            EmitCurvePrimitive (*curvePrimitive);
            return;
            }

        case IGeometry::GeometryType::CurveVector:
            {
            CurveVectorPtr curveVector = geometry->GetAsCurveVector ();

            EmitCurveVector (*curveVector);
            return;
            }

        case IGeometry::GeometryType::SolidPrimitive:
            {
            ISolidPrimitivePtr solidPrimitive = geometry->GetAsISolidPrimitive ();

            EmitSolidPrimitive (*solidPrimitive);
            return;
            }

        case IGeometry::GeometryType::BsplineSurface:
            {
            MSBsplineSurfacePtr bSurface = geometry->GetAsMSBsplineSurface ();

            EmitSurface (*bSurface);
            return;
            }

        case IGeometry::GeometryType::Polyface:
            {
            PolyfaceHeaderPtr polyface = geometry->GetAsPolyfaceHeader ();

            EmitPolyface (*polyface);
            return;
            }

        default:
            {
            BeAssert (false);
            return;
            }
        }
    }
END_BENTLEY_GEOMETRY_NAMESPACE
