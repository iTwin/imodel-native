/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/GeomSerialization/GeomLibsJsonSerialization.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__BENTLEY_INTERNAL_ONLY__*/

#include <Bentley/Bentley.h>
//#include <Bentley/BeConsole.h>
#include <Geom/GeomApi.h>
#include <BeJsonCpp/BeJsonUtilities.h>

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

public: static GEOMLIBS_SERIALIZATION_EXPORT bool TryIModelJsonValueToGeometry (JsonValueCR value, bvector<IGeometryPtr> &geometry);

public: static GEOMLIBS_SERIALIZATION_EXPORT bool TryIModelJsonStringToGeometry (Utf8StringCR string, bvector<IGeometryPtr> &geometry);


//! Convert a vector of IGeometry to json value with the IModelJson schema keys and structure
public: static GEOMLIBS_SERIALIZATION_EXPORT bool TryGeometryToIModelJsonValue(Json::Value &value, bvector<IGeometryPtr> const &data);
//! Convert a single IGeometry a json value with the IModelJson schema keys and structure
public: static GEOMLIBS_SERIALIZATION_EXPORT bool TryGeometryToIModelJsonValue (Json::Value &value, IGeometryCR data);

//! Convert a vector of IGeometry to string with the IModelJson schema keys and structure
public: static GEOMLIBS_SERIALIZATION_EXPORT bool TryGeometryToIModelJsonString (Utf8StringR string, IGeometryCR data);
//!  Convert a single IGeometry to string with the IModelJson schema keys and structure
public: static GEOMLIBS_SERIALIZATION_EXPORT bool TryGeometryToIModelJsonString(Utf8StringR string, bvector<IGeometryPtr> const &data);

};

END_BENTLEY_GEOMETRY_NAMESPACE

