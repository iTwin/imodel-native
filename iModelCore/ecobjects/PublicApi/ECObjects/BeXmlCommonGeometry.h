/*--------------------------------------------------------------------------------------+
|
|  $Source: PublicApi/ECObjects/BeXmlCommonGeometry.h $
|
|  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/
#include <ECObjects/ECObjects.h>
#include <Geom/GeomApi.h>
#include <BeXml/BeXml.h>

BEGIN_BENTLEY_EC_NAMESPACE
struct BeXmlCGParser
{
public:
//! Try to read a curve primitive
//! @return false if this node is not a curve primitive.
//! @param [in] node xml node
//! @param [out] result curve primitive.
ECOBJECTS_EXPORT bool TryParse (BeXmlNodeP node, ICurvePrimitivePtr &result);
//! Try to read a solid primitive
//! @return false if this node is not a solid primitive.
//! @param [in] node xml node
//! @param [out] result solid primitive.
ECOBJECTS_EXPORT bool TryParse (BeXmlNodeP node, ISolidPrimitivePtr &result);
//! constructor for parse object.
ECOBJECTS_EXPORT BeXmlCGParser ();
//! Try to read any geometry types, optionally recursing through the xml tree
//!    to find geometry items.
//! @return number of geometry items found.
//! @param [in] node top node of search
//! @param [in,out] vector to receive geometry.
ECOBJECTS_EXPORT size_t AddGeometry (BeXmlNodeP node, bvector<IGeometryPtr> &geometry, size_t maxDepth = INT_MAX);
};


struct BeXmlCGWriter
{

static ECOBJECTS_EXPORT void WriteXYZ    (BeXmlWriterR dest, Utf8CP name, DPoint3dCR data);
static ECOBJECTS_EXPORT void WriteXYZ    (BeXmlWriterR dest, Utf8CP name, double x, double y, double z);
static ECOBJECTS_EXPORT void WriteXY     (BeXmlWriterR dest, Utf8CP name, double x, double y);
static ECOBJECTS_EXPORT void WriteDouble (BeXmlWriterR dest, Utf8CP name, double data);
static ECOBJECTS_EXPORT void WriteInt (BeXmlWriterR dest, Utf8CP name, int data);
static ECOBJECTS_EXPORT void WriteBool (BeXmlWriterR dest, Utf8CP name, bool data);
static ECOBJECTS_EXPORT void WriteText (BeXmlWriterR dest, Utf8CP name, wchar_t const *data);


static ECOBJECTS_EXPORT void WriteList      (BeXmlWriterR dest, bvector<DPoint3d> const & data, Utf8CP listName, Utf8CP itemName);
static ECOBJECTS_EXPORT void WriteList      (BeXmlWriterR dest, bvector<double> const & data, Utf8CP listName, Utf8CP itemName);
static ECOBJECTS_EXPORT void WriteList      (BeXmlWriterR dest, bvector<DVec3d> const & data, Utf8CP listName, Utf8CP itemName);
static ECOBJECTS_EXPORT void WriteList      (BeXmlWriterR dest, bvector<RgbFactor> const & data, Utf8CP listName, Utf8CP itemName);
static ECOBJECTS_EXPORT void WriteList      (BeXmlWriterR dest, bvector<DPoint2d> const & data, Utf8CP listName, Utf8CP itemName);
static ECOBJECTS_EXPORT void WriteIndexList (BeXmlWriterR dest, bvector<int> const & data, Utf8CP listName, Utf8CP itemName);
static ECOBJECTS_EXPORT void WriteSegment (BeXmlWriterR dest, DSegment3dCR geometry);

static ECOBJECTS_EXPORT void WriteArc (BeXmlWriterR dest, DEllipse3dCR geometry);
static ECOBJECTS_EXPORT void WriteDisk (BeXmlWriterR dest, DEllipse3dCR geometry);
static ECOBJECTS_EXPORT void WriteCurve (BeXmlWriterR dest, MSBsplineCurveCR curve);
static ECOBJECTS_EXPORT void WritePolyface (BeXmlWriterR dest, PolyfaceVectors &mesh);
static ECOBJECTS_EXPORT void WriteTextPlacement (BeXmlWriterR dest, DPoint3dCR xyz, wchar_t const *text, double charSize);
static ECOBJECTS_EXPORT void WriteLineString (BeXmlWriterR dest, bvector<DPoint3d> const &points);


static ECOBJECTS_EXPORT void WriteCurvePrimitive (BeXmlWriterR dest, ICurvePrimitiveCR curve);
static ECOBJECTS_EXPORT void WriteCurveVector (BeXmlWriterR dest, CurveVectorCR curve);
//! @param [in] dest receiver writer.
//! @param [in] curves source geometry
//! @param [in] preferMostCompactPrimitives if true, try to substitute circular disk, elliptic disk, polygon for formal curve chain over singel primitive.
static ECOBJECTS_EXPORT void WriteCurveVector (BeXmlWriterR dest, CurveVectorCR curves, bool preferMostCompactPrimitives);

static ECOBJECTS_EXPORT void WritePlacementXY
(
BeXmlWriterR dest,
DPoint3dCR center,
DVec3dCR   vectorX,
DVec3dCR   vectorY
);

static ECOBJECTS_EXPORT void WritePlacementZX
(
BeXmlWriterR dest,
DPoint3dCR center,
DVec3dCR   vectorZ,
DVec3dCR   vectorX
);

static ECOBJECTS_EXPORT void WritePlacementZX
(
BeXmlWriterR dest,
DPoint3dCR origin,
RotMatrixCR axes
);

static ECOBJECTS_EXPORT void WriteDgnTorusPipeDetail (BeXmlWriterR dest, DgnTorusPipeDetail data);
static ECOBJECTS_EXPORT void WriteDgnConeDetail (BeXmlWriterR dest, DgnConeDetail data);
static ECOBJECTS_EXPORT void BeXmlCGWriter::WriteDgnSphereDetail (BeXmlWriterR dest, DgnSphereDetail data);
static ECOBJECTS_EXPORT void BeXmlCGWriter::WriteDgnBoxDetail (BeXmlWriterR dest, DgnBoxDetail data);
static ECOBJECTS_EXPORT void BeXmlCGWriter::WriteDgnExtrusionDetail (BeXmlWriterR dest, DgnExtrusionDetail data);
static ECOBJECTS_EXPORT void BeXmlCGWriter::WriteDgnRotationalSweepDetail (BeXmlWriterR dest, DgnRotationalSweepDetail data);
static ECOBJECTS_EXPORT void BeXmlCGWriter::WriteDgnRuledSweepDetail (BeXmlWriterR dest, DgnRuledSweepDetail data);


static ECOBJECTS_EXPORT void BeXmlCGWriter::Write (BeXmlWriterR dest, ISolidPrimitiveR data);
static ECOBJECTS_EXPORT void BeXmlCGWriter::Write (BeXmlWriterR dest, IGeometryPtr data);

};

END_BENTLEY_EC_NAMESPACE