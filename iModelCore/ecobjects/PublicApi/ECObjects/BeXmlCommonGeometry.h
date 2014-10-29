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

    //! Try to read a curve vector
    //! @return false if this node is not a curve vector
    //! @param [in] node xml node
    //! @param [out] result curve vector
    static bool TryParse (BeXmlNodeP node, CurveVectorPtr &result);
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
    //! @param [out] result curve primitive.
    static ECOBJECTS_EXPORT bool TryParse (Utf8CP beXmlCGString, ICurvePrimitivePtr &result);
    //! Try to read a solid primitive
    //! @return false if this node is not a solid primitive.
    //! @param [out] result solid primitive.
    static ECOBJECTS_EXPORT bool TryParse (Utf8CP beXmlCGString, ISolidPrimitivePtr &result);

    //! Try to read a bspline surface
    //! @return false if this node is not a bspline surface
    //! @param [out] result bspline surface
    static ECOBJECTS_EXPORT bool TryParse (Utf8CP beXmlCGString, MSBsplineSurfacePtr &result);

    //! Try to read any geometry types, optionally recursing through the xml tree
    //!    to find geometry items.
    //! @param [in] beXmlCGString string containing the serialized IGeometry object in BeXml.
    //! @param [in,out] vector to receive geometry.
    //! @param [in] maxDepth max number of recursions for search.  0 means only look at the node itself.
    //! @return true if any geometry found
    static ECOBJECTS_EXPORT bool TryParse (Utf8CP beXmlCGString, bvector<IGeometryPtr> &geometry, size_t maxDepth = INT_MAX);
    
};

struct BeXmlCGStreamReader
{
private: BeXmlCGStreamReader ();
public: static ECOBJECTS_EXPORT bool TryParse (Utf8CP beXmlCGString, bvector<IGeometryPtr> &geometry, size_t maxDepth = INT_MAX);
        static ECOBJECTS_EXPORT bool TryParse (byte* buffer, int bufferLength, bvector<IGeometryPtr> &geometry, size_t maxDepth);
};

//! static methods to add common geometry content to xml file.
struct BeXmlCGWriter
{

public:

    //! Write common geometry xml for an IGeometryPtr object
    //! @param [out] cgBeXml xml string
    //! @param [in]  data IGeometryPtr common geometry object to be serialized
    ECOBJECTS_EXPORT static void Write (Utf8StringR cgBeXml, IGeometryPtr data);

    //! Write common geometry json for an IGeometryPtr object
    //! @param [out] dest json string
    //! @param [in]  data IGeometryPtr common geometry object to be serialized
    ECOBJECTS_EXPORT static void WriteJson (Utf8StringR dest, IGeometryPtr data);

    //! Write common geometry binary xml (Microsoft .net) for an IGeometryPtr object
    //! @param [out] bytes binary bytes.
    //! @param [in]  data IGeometryPtr common geometry object to be serialized
    ECOBJECTS_EXPORT static void WriteBytes (bvector<byte>& bytes, IGeometryPtr data);

};

END_BENTLEY_ECOBJECT_NAMESPACE
