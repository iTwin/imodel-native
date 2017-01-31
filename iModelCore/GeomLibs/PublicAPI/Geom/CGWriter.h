/*--------------------------------------------------------------------------------------+
|
|  $Source: PublicAPI/Geom/CGWriter.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <string>

#ifndef __IStream_FWD_DEFINED__
#define __IStream_FWD_DEFINED__
struct IStream;
#endif 	/* __IStream_FWD_DEFINED__ */


/*__BENTLEY_INTERNAL_ONLY__*/

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

//! Controller for generating CG-style xml.
struct CGWriter
{
//! Write a C wchar_t string
GEOMDLLIMPEXP void Write (wchar_t const *buffer);
// Format with sprintf and one widestring arg.
GEOMDLLIMPEXP void WriteFormattedW (wchar_t const *cFormatString, wchar_t const *wString);


GEOMDLLIMPEXP void WriteStartTag (wchar_t const *name);
GEOMDLLIMPEXP void WriteEndTag (wchar_t const *name);
GEOMDLLIMPEXP void WriteEndTag (wchar_t const *name, bool lineFeed);

GEOMDLLIMPEXP void WriteXYZ (double x, double y, double z);
GEOMDLLIMPEXP void WriteXY (double x, double y);

GEOMDLLIMPEXP void WriteDouble (double x);
GEOMDLLIMPEXP void WriteInt (int x);
GEOMDLLIMPEXP void WriteSize (size_t x);
GEOMDLLIMPEXP void WriteBool (bool x);

//! Stack frame for CGWriter
struct TagFrame
{
std::wstring mTagName;
TagFrame (std::wstring &tagName);
TagFrame (wchar_t const *tagName);
};
private:

//! Stream (opened by caller) for output
IStream *mStream;

//! File (opened by caller) for output.
FILE *mFile;
//! Stack of current xml tags
bvector<TagFrame> mStack;

//! number of index tags allowed on one line of xml
size_t mMaxIndexPerLine;
size_t mMaxPackedIndexPerLine;
size_t mMinPackedIndexPerLine;
//! number of blanks per indentation level.
size_t mIndent;
//! format masks
//! 0x01 bit ===> pack
size_t m_formatMasks;
public:
//! L"CSL" ==> comma separated lists.
GEOMDLLIMPEXP void SetFormatMask (wchar_t const* maskName);

//! Setup indentation and linefeed controls to make the xml eye-friendly
GEOMDLLIMPEXP void InitReadable ();

//! Constructor -- retain file pointer internally, setup for eye-friendly output.
GEOMDLLIMPEXP CGWriter (FILE *file);

//! Constructor -- retain stream pointer internally, setup for eye-friendly output.
GEOMDLLIMPEXP CGWriter (IStream *stream);

//! Emit blanks for an indented line.
GEOMDLLIMPEXP void Indent (size_t numBack = 0);
//! Push a TagFrame on the stack. Output the tag header.
GEOMDLLIMPEXP void StartTag (wchar_t const *tagName, bool includeNameSpaceAttribute = false);
//! Pop TagFrame, output tag closure.
GEOMDLLIMPEXP void EndTag ();

//! Emit <tagname>data</tagname>
GEOMDLLIMPEXP void EmitInt (wchar_t const *tagname, int data);
//! Emit <tagname>data</tagname>
GEOMDLLIMPEXP void EmitDouble (wchar_t const *tagname, double data);
//! Emit <tagname>data</tagname>
GEOMDLLIMPEXP void EmitSize (wchar_t const *tagname, size_t data);
//! Emit <tagname>data</tagname>
GEOMDLLIMPEXP void EmitBool (wchar_t const *tagname, bool data);

//! Emit <tagname>data</tagname>
GEOMDLLIMPEXP void EmitDPoint3d (wchar_t const *tagname, DPoint3dCR data, bool indent = true);
//! Emit <tagname>data</tagname>
GEOMDLLIMPEXP void EmitDVec3d (wchar_t const *tagname, DVec3dCR data, bool indent = true);
//! Emit <tagname>data</tagname>
GEOMDLLIMPEXP void EmitDPoint2d (wchar_t const *tagname, DPoint2dCR data, bool indent = true);
//! Emit <tagname>data</tagname>
GEOMDLLIMPEXP void EmitRgbFactor (wchar_t const *tagname, RgbFactor const & data);

//! Emit an array of DPoint3d
GEOMDLLIMPEXP void Emit (bvector<DPoint3d>& data, wchar_t const *listName, wchar_t const *itemName);
//! Emit an array of DVec3d
GEOMDLLIMPEXP void Emit (bvector<DVec3d>& data, wchar_t const *listName, wchar_t const *itemName);
//! Emit an array of RgbFactor
GEOMDLLIMPEXP void Emit (bvector<RgbFactor>& data, wchar_t const *listName, wchar_t const *itemName);
//! Emit an array of DPoint2d
GEOMDLLIMPEXP void Emit (bvector<DPoint2d>& data, wchar_t const *listName, wchar_t const *itemName);
//! Emit an index vector.  Multiple indices for a face may be packed on one line.  0 triggers newline.
GEOMDLLIMPEXP void EmitIndexVector (bvector<int>& data, wchar_t const *listName, wchar_t const *itemName);
//! Emit an array of double
GEOMDLLIMPEXP void Emit (bvector<double>& data, wchar_t const *listName, wchar_t const *itemName);
//! Emit a polyface.
GEOMDLLIMPEXP void EmitPolyface (PolyfaceHeaderR mesh);
//! Emit a B-spline curve.
GEOMDLLIMPEXP void EmitCurve (MSBsplineCurveCR curve);
//! Emit a B-spline surface.
GEOMDLLIMPEXP void EmitSurface (MSBsplineSurfaceCR surface);

//! Emit curve vector.
GEOMDLLIMPEXP void EmitCurveVector (CurveVectorCR curveVector);

//! Emit curve vector.
//! @param [in] curveVector source data
//! @param [in] preferMostCompactPrimitives true to simplify singleton vectors to disks.
GEOMDLLIMPEXP void EmitCurveVector
(
CurveVectorCR curveVector,
bool preferMostCompactPrimitives
);

//! Emit curve primitive
GEOMDLLIMPEXP void EmitCurvePrimitive (ICurvePrimitiveCR curve);

//! Emit curve primitive
GEOMDLLIMPEXP void EmitPartialCurve (PartialCurveDetailCP detail);
//! Emit line segment
GEOMDLLIMPEXP void EmitDSegment3d (DSegment3dCR segment);

//! Emit circular or elliptic arc.  
//! (Same as EmitArc??)
GEOMDLLIMPEXP void EmitDEllipse3dAsArc (DEllipse3d arc);

//! Fixup magnitudes and output a placement
GEOMDLLIMPEXP void EmitPlacementZX (DPoint3dCR origin, DVec3dCR vectorZ, DVec3dCR vectorX);
//! Fixup magnitudes and output a placement
GEOMDLLIMPEXP void EmitPlacementXY (DPoint3dCR origin, DVec3dCR vectorX, DVec3dCR vectorY);
//! Fixup magnitudes and output a placement
GEOMDLLIMPEXP void EmitPlacementZX (DPoint3dCR origin, RotMatrixCR axes);

//! Emit a line string.
GEOMDLLIMPEXP void EmitLineString(bvector<DPoint3d>const &);
//! Emit a point string
GEOMDLLIMPEXP void EmitPointString(bvector<DPoint3d>const &);

//! Emit a polygon
GEOMDLLIMPEXP void EmitPolygon(bvector<DPoint3d>const &);

//! Emit an circular or elliptic arc.
GEOMDLLIMPEXP void EmitArc (DEllipse3dCR arc);
//! Emit an circular or elliptic disk
GEOMDLLIMPEXP void EmitDisk (DEllipse3dCR arc);
//! Emit a line segment
GEOMDLLIMPEXP void EmitLineSegment (DSegment3dCR segment);

//! Branch to type specific emitter
GEOMDLLIMPEXP void EmitSolidPrimitive (ISolidPrimitiveR primitive);
GEOMDLLIMPEXP void EmitDgnTorusPipeDetail (DgnTorusPipeDetail data);
GEOMDLLIMPEXP void EmitDgnConeDetail (DgnConeDetail data);
GEOMDLLIMPEXP void EmitDgnBoxDetail (DgnBoxDetail data);
GEOMDLLIMPEXP void EmitDgnSphereDetail (DgnSphereDetail data);
GEOMDLLIMPEXP void EmitDgnExtrusionDetail (DgnExtrusionDetail data);
GEOMDLLIMPEXP void EmitDgnRotationalSweepDetail (DgnRotationalSweepDetail data);
GEOMDLLIMPEXP void EmitDgnRuledSweepDetail (DgnRuledSweepDetail data);

//! Emit text (default font) in xy plane
GEOMDLLIMPEXP void EmitText (DPoint3dCR xyz, wchar_t const *text, double size);

//! Return given coordinate x unchanged if it is clearly not zero.  Force to zero if near machine precision relative to refValue.
GEOMDLLIMPEXP double SuppressNearZeroCoordinate (double x, double refValue = 1.0);

//! Emit any geometry type
GEOMDLLIMPEXP void Emit (IGeometryPtr geometry);
};



END_BENTLEY_GEOMETRY_NAMESPACE
