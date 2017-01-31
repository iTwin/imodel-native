/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/GeomSerialization/GeomLibsJsonSerialization.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
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

END_BENTLEY_GEOMETRY_NAMESPACE

