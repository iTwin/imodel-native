/*--------------------------------------------------------------------------------------+
|
|  $Source: PublicApi/ECObjects/CGReader.h $
|
|  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__PUBLISH_SECTION_START__*/
#include <ECObjects/ECObjects.h>
#include <Geom/GeomApi.h>
#include <BeXml/BeXml.h>

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

