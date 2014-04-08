/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ECObjects/BeXmlCommonGeometry.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__PUBLISH_SECTION_START__*/
#include <ECObjects/ECObjects.h>
#include <Geom/GeomApi.h>

/*__PUBLISH_SECTION_END__*/
#include <BeXml/BeXml.h>
/*__PUBLISH_SECTION_START__*/

BEGIN_BENTLEY_ECOBJECT_NAMESPACE
//! static methods to read Bentley.Geometry types from Common Geometry xml.
struct BeXmlCGParser
{
private:
    BeXmlCGParser ();
    /*__PUBLISH_SECTION_END__*/
    //! Try to read a curve primitive
    //! @return false if this node is not a curve primitive.
    //! @param [in] node xml node
    //! @param [out] result curve primitive.
    static bool TryParse (BeXmlNodeP node, ICurvePrimitivePtr &result);
    //! Try to read a solid primitive
    //! @return false if this node is not a solid primitive.
    //! @param [in] node xml node
    //! @param [out] result solid primitive.
    static bool TryParse (BeXmlNodeP node, ISolidPrimitivePtr &result);

    //! Try to read a bspline surface
    //! @return false if this node is not a bspline surface
    //! @param [in] node xml node
    //! @param [out] result bspline surface
    static bool TryParse (BeXmlNodeP node, MSBsplineSurfacePtr &result);

    //! Try to read any geometry types, optionally recursing through the xml tree
    //!    to find geometry items.
    //! @param [in] node top node of search
    //! @param [in,out] vector to receive geometry.
    //! @param [in] maxDepth max number of recursions for search.  0 means only look at the node itself.
    //! @return true if any geometry found
    static bool TryParse (BeXmlNodeP node, bvector<IGeometryPtr> &geometry, size_t maxDepth = INT_MAX);
    /*__PUBLISH_SECTION_START__*/

public:
    //! Try to read a curve primitive
    //! @return false if this node is not a curve primitive.
    //! @param [in] node xml node
    //! @param [out] result curve primitive.
    static ECOBJECTS_EXPORT bool TryParse (Utf8CP beXmlCGString, ICurvePrimitivePtr &result);
    //! Try to read a solid primitive
    //! @return false if this node is not a solid primitive.
    //! @param [in] node xml node
    //! @param [out] result solid primitive.
    static ECOBJECTS_EXPORT bool TryParse (Utf8CP beXmlCGString, ISolidPrimitivePtr &result);

    //! Try to read a bspline surface
    //! @return false if this node is not a bspline surface
    //! @param [in] node xml node
    //! @param [out] result bspline surface
    static ECOBJECTS_EXPORT bool TryParse (Utf8CP beXmlCGString, MSBsplineSurfacePtr &result);

    //! Try to read any geometry types, optionally recursing through the xml tree
    //!    to find geometry items.
    //! @param [in] Utf8 string containing the serialized IGeometry object in BeXml.
    //! @param [in,out] vector to receive geometry.
    //! @param [in] maxDepth max number of recursions for search.  0 means only look at the node itself.
    //! @return true if any geometry found
    static ECOBJECTS_EXPORT bool TryParse (Utf8CP beXmlCGString, bvector<IGeometryPtr> &geometry, size_t maxDepth = INT_MAX);
    
};

//! static methods to add common geometry content to xml file.
struct BeXmlCGWriter
{
private:
    /*__PUBLISH_SECTION_END__*/
    //! XML element  <name>x,y,z</name>
    //! @param [in] dest xml receiver
    //! @param [in] data coordinate data.
    static void WriteXYZ    (BeXmlWriterR dest, Utf8CP name, DPoint3dCR data);
    //! XML element  <name>x,y,z</name>
    //! @param [in] dest xml receiver
    //! @param [in] x coordinate value
    //! @param [in] y coordinate value
    //! @param [in] z coordinate value
    static void WriteXYZ    (BeXmlWriterR dest, Utf8CP name, double x, double y, double z);

    //! XML element  <name>x,y</name>
    //! @param [in] dest xml receiver
    //! @param [in] x coordinate value
    //! @param [in] y coordinate value
    static void WriteXY     (BeXmlWriterR dest, Utf8CP name, double x, double y);
    //! XML element  <name>a</name>
    //! @param [in] dest xml receiver
    //! @param [in] a double value
    static void WriteDouble (BeXmlWriterR dest, Utf8CP name, double a);
    //! XML element  <name>a</name>
    //! @param [in] dest xml receiver
    //! @param [in] a int value
    static void WriteInt (BeXmlWriterR dest, Utf8CP name, int a);
    //! XML element  <name>true</name> or <name>false</name>
    //! @param [in] dest xml receiver
    //! @param [in] data bool data
    static void WriteBool (BeXmlWriterR dest, Utf8CP name, bool data);
    //! XML element  <name>[buffer]true</name>
    //! @param [in] dest xml receiver
    //! @param [in] buffer null terminated string
    static void WriteText (BeXmlWriterR dest, Utf8CP name, wchar_t const *buffer);


    //! XML element  <listName><name>x0,y0,z0</name>......</listName>
    //! @param [in] dest xml receiver
    static void WriteList      (BeXmlWriterR dest, bvector<DPoint3d> const & data, Utf8CP listName, Utf8CP itemName);
    //! XML element  <listName><name>a0</name><name>a1</name>......</listName>
    //! @param [in] dest xml receiver
    static void WriteList      (BeXmlWriterR dest, bvector<double> const & data, Utf8CP listName, Utf8CP itemName);
    //! XML element  <listName><name>x0,y0,z0</name>......</listName>
    //! @param [in] dest xml receiver
    static void WriteList      (BeXmlWriterR dest, bvector<DVec3d> const & data, Utf8CP listName, Utf8CP itemName);
    //! XML element  <listName><name>r0,g0,b0</name>......</listName>
    //! @param [in] dest xml receiver
    static void WriteList      (BeXmlWriterR dest, bvector<RgbFactor> const & data, Utf8CP listName, Utf8CP itemName);
    //! XML element  <listName><name>x,y</name>......</listName>
    //! @param [in] dest xml receiver
    static void WriteList      (BeXmlWriterR dest, bvector<DPoint2d> const & data, Utf8CP listName, Utf8CP itemName);
    //! XML element  <listName><name>i0</name><name>i1</name>......</listName>
    //! @param [in] dest xml receiver
    static void WriteIndexList (BeXmlWriterR dest, bvector<int> const & data, Utf8CP listName, Utf8CP itemName);

    //! Write a common geometry line segment
    //! @param [in] dest xml receiver
    static void WriteSegment (BeXmlWriterR dest, DSegment3dCR geometry);
    //! Write a common geometry circular or elliptic arc
    //! @param [in] dest xml receiver
    static void WriteArc (BeXmlWriterR dest, DEllipse3dCR geometry);
    //! Write a common geometry circular or elliptic disk
    //! @param [in] dest xml receiver
    static void WriteDisk (BeXmlWriterR dest, DEllipse3dCR geometry);
    //! Write a common geometry bspline curve
    //! @param [in] dest xml receiver
    static void WriteCurve (BeXmlWriterR dest, MSBsplineCurveCR curve);
    //! Write a common geometry bspline surface
    //! @param [in] dest xml receiver
    static void WriteSurface (BeXmlWriterR dest, MSBsplineSurfaceCR surface);

    //! Write a common geometry indexed mesh
    //! @param [in] dest xml receiver
    static void WritePolyface (BeXmlWriterR dest, PolyfaceHeader &mesh);
    //! Write a common geometry text object
    //! @param [in] dest xml receiver
    static void WriteTextPlacement (BeXmlWriterR dest, DPoint3dCR xyz, wchar_t const *text, double charSize);
    //! Write a common geometry linestring
    //! @param [in] dest xml receiver
    static void WriteLineString (BeXmlWriterR dest, bvector<DPoint3d> const &points);
    //! Write a common geometry polygon
    //! @param [in] dest xml receiver
    static void WritePolygon (BeXmlWriterR dest, bvector<DPoint3d> const &points);

    //! Write a placement from X and Y vectors.
    //! @param [in] origin origin of placement system.
    //! @param [in] vectorX x direction vector
    //! @param [in] vectorY y direction vector
    static void WritePlacementXY (BeXmlWriterR dest, DPoint3dCR origin, DVec3dCR   vectorX, DVec3dCR   vectorY);

    //! Write a placement from Z and X vectors appearing directly.
    //! @param [in]static ECOBJECTS_EXPORT void WritePlacementZX (BeXmlWriterR dest, DPoint3dCR center, DVec3dCR   vectorZ, DVec3dCR   vectorX);
    static void WritePlacementZX (BeXmlWriterR dest, DPoint3dCR origin, DVec3dCR   vectorZ, DVec3dCR   vectorX);

    //! Write a placement from Z and X vectors in rotmatrix form.
    //! @param [in] origin origin of placement system
    //! @param [in] axes X,Y,Z axes. Z and X are normalized for use in the placement.
    static void WritePlacementZX (BeXmlWriterR dest, DPoint3dCR origin, RotMatrixCR axes);

    //! @param [in] dest xml receiver
    //! @param [in] data torus pipe data.
    static void WriteDgnTorusPipeDetail (BeXmlWriterR dest, DgnTorusPipeDetail data);
    //! @param [in] dest xml receiver
    //! @param [in] data cone data
    static void WriteDgnConeDetail (BeXmlWriterR dest, DgnConeDetail data);
    //! @param [in] dest xml receiver
    //! @param [in] data sphere data
    static void WriteDgnSphereDetail (BeXmlWriterR dest, DgnSphereDetail data);
    //! @param [in] dest xml receiver
    //! @param [in] data box data
    static void WriteDgnBoxDetail (BeXmlWriterR dest, DgnBoxDetail data);
    //! @param [in] dest xml receiver
    //! @param [in] data extrusion data
    static void WriteDgnExtrusionDetail (BeXmlWriterR dest, DgnExtrusionDetail data);
    //! @param [in] dest xml receiver
    //! @param [in] data rotational sweep data
    static void WriteDgnRotationalSweepDetail (BeXmlWriterR dest, DgnRotationalSweepDetail data);
    //! @param [in] dest xml receiver
    //! @param [in] data ruled sweep data
    static void WriteDgnRuledSweepDetail (BeXmlWriterR dest, DgnRuledSweepDetail data);


    //! Write common geometry xml for curve primitive.
    //! @param [in] dest xml receiver
    static void Write (BeXmlWriterR dest, ICurvePrimitiveCR curve);
    //! Write common geometry xml for curve vector.
    //! @param [in] dest xml receiver
    static void Write (BeXmlWriterR dest, CurveVectorCR curve);
    //! @param [in] dest receiver writer.
    //! @param [in] curves source geometry
    //! @param [in] preferMostCompactPrimitives if true, try to substitute circular disk, elliptic disk for formal curve chain over single primitive.
    static void Write (BeXmlWriterR dest, CurveVectorCR curves, bool preferMostCompactPrimitives);

    //! @param [in] dest xml receiver
    static void Write (BeXmlWriterR dest, ISolidPrimitiveR data);
    //! @param [in] dest xml receiver
    static void Write (BeXmlWriterR dest, IGeometryPtr data);

    /*__PUBLISH_SECTION_START__*/
public:
    //! @param [in] cgBeXml xml receiver
    static ECOBJECTS_EXPORT void Write (Utf8StringR cgBeXml, ISolidPrimitiveR data);
    //! @param [in] cgBeXml xml receiver
    static ECOBJECTS_EXPORT void Write (Utf8StringR cgBeXml, IGeometryPtr data);

    //! Write common geometry xml for curve primitive.
    //! @param [in] dest xml receiver
    static ECOBJECTS_EXPORT void Write (Utf8StringR cgBeXml, ICurvePrimitiveCR curve);

};

END_BENTLEY_ECOBJECT_NAMESPACE
