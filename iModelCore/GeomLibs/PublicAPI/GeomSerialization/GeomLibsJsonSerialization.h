/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <Bentley/Bentley.h>
//#include <Bentley/BeConsole.h>
#include <Geom/GeomApi.h>
#include <BeJsonCpp/BeJsonUtilities.h>
#include <json/BeJsValue.h>

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

struct BentleyGeometryJson
{
private: BentleyGeometryJson ();
public: static GEOMLIBS_SERIALIZATION_EXPORT bool TryJsonValueToGeometry (JsonValueCR value, bvector<IGeometryPtr> &geometry);

public: static GEOMLIBS_SERIALIZATION_EXPORT bool TryJsonStringToGeometry (Utf8StringCR string, bvector<IGeometryPtr> &geometry);

//! Write to a json value.
public: static GEOMLIBS_SERIALIZATION_EXPORT bool TryGeometryToJsonValue (JsonValueR value, IGeometryCR data, bool preferNativeDgnTYpes = true);

//! Write to a json value.
public: static GEOMLIBS_SERIALIZATION_EXPORT bool TryGeometryToJsonValue(JsonValueR value, bvector<IGeometryPtr> const &data, bool preferNativeDgnTYpes = true);

//! Write to a json string.
public: static GEOMLIBS_SERIALIZATION_EXPORT bool TryGeometryToJsonString (Utf8StringR string, IGeometryCR data, bool preferNativeDgnTYpes = true);

        //! Write to a json string.
public: static GEOMLIBS_SERIALIZATION_EXPORT bool TryGeometryToJsonString(Utf8StringR string, bvector<IGeometryPtr> const &data, bool preferNativeDgnTYpes = true);
        //! Output readable json via GEOMAPI_PRINTF (i.e. for debug use only)
public: static GEOMLIBS_SERIALIZATION_EXPORT void DumpJson (Utf8StringCR string);

};


struct IModelJson
{

public: static GEOMLIBS_SERIALIZATION_EXPORT bool TryIModelJsonValueToGeometry
(
BeJsConst value,
bvector<IGeometryPtr> &geometry,
bvector<IGeometryPtr> *invalidGeometry = nullptr
);

public: static GEOMLIBS_SERIALIZATION_EXPORT bool TryIModelJsonStringToGeometry
(
Utf8StringCR string,
bvector<IGeometryPtr> &geometry
);

//! Convert a vector of IGeometry to json value with the IModelJson schema keys and structure
public: static GEOMLIBS_SERIALIZATION_EXPORT bool TryGeometryToIModelJsonValue
(
BeJsValue value,
bvector<IGeometryPtr> const &data,
bvector<IGeometryPtr> *validGeometry = nullptr,
bvector<IGeometryPtr> *invalidGeometry = nullptr
);

//! Convert a single IGeometry a json value with the IModelJson schema keys and structure
public: static GEOMLIBS_SERIALIZATION_EXPORT bool TryGeometryToIModelJsonValue
(
BeJsValue value,
IGeometryCR data);
/** Add nonzero parts of a TaggedNumericData struct to the json value.
* <ul>
* <li> m_tag is always output as "tag"
* <li> m_intValue and m_doubleValue are output as "iValue" and "dValue" if nonzero
* <li> m_xyz is output as "xyz" if any of its components is nonzero.
* <li> m_vectors is output as 0,1,2, or 3 [x,y,z] values, with trailing zero vectors omitted.
* <li>
* </ul>
*/
public: static GEOMLIBS_SERIALIZATION_EXPORT void TaggedNumericDataToJson
(
BeJsValue value,
TaggedNumericData const &data
);
//! Convert a vector of IGeometry to string with the IModelJson schema keys and structure
public: static GEOMLIBS_SERIALIZATION_EXPORT bool TryGeometryToIModelJsonString (Utf8StringR string, IGeometryCR data);
//!  Convert a single IGeometry to string with the IModelJson schema keys and structure
public: static GEOMLIBS_SERIALIZATION_EXPORT bool TryGeometryToIModelJsonString
(
Utf8StringR string,
bvector<IGeometryPtr> const &data,
bvector<IGeometryPtr> *validGeometry = nullptr,
bvector<IGeometryPtr> *invalidGeometry = nullptr
);
public: static GEOMLIBS_SERIALIZATION_EXPORT bool TryIModelJsonValueToTaggedNumericData(BeJsConst value, TaggedNumericData &data);

};

END_BENTLEY_GEOMETRY_NAMESPACE

