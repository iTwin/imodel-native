/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ECObjects/ECJsonUtilities.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "ECSchema.h"
#include "ECInstance.h"
#include <Geom/GeomApi.h>
#include <BeJsonCpp/BeJsonUtilities.h>
#include <rapidjson/BeRapidJson.h>

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

/*=================================================================================**//**
* @bsiclass                                                 Ramanujam.Raman      01/2014
+===============+===============+===============+===============+===============+======*/
struct ECJsonUtilities
    {
private:
    ECJsonUtilities();
    ~ECJsonUtilities();

    static BentleyStatus PointCoordinateFromJson(double&, Json::Value const&, Utf8CP coordinateKey);

    static BentleyStatus ECInstanceFromJsonValue(ECN::IECInstanceR, Json::Value const&, ECN::ECClassCR currentClass, Utf8StringCR currentAccessString);
    static BentleyStatus ECArrayValueFromJsonValue(ECN::IECInstanceR, Json::Value const&, ECN::ECPropertyCR, Utf8StringCR currentAccessString);
    static BentleyStatus ECPrimitiveValueFromJsonValue(ECN::ECValueR, Json::Value const&, ECN::PrimitiveType);

public:
    ECOBJECTS_EXPORT static BentleyStatus ECInstanceFromJsonValue(ECN::IECInstanceR, Json::Value const&);

    ECOBJECTS_EXPORT static BentleyStatus BinaryToJson(Json::Value&, Byte const*, size_t binarySize);
    ECOBJECTS_EXPORT static BentleyStatus JsonToBinary(bvector<Byte>&, Json::Value const&);
    ECOBJECTS_EXPORT static BentleyStatus Point2DToJson(Json::Value&, DPoint2d);
    ECOBJECTS_EXPORT static BentleyStatus JsonToPoint2D(DPoint2d&, Json::Value const&);
    ECOBJECTS_EXPORT static BentleyStatus Point3DToJson(Json::Value&, DPoint3d);
    ECOBJECTS_EXPORT static BentleyStatus JsonToPoint3D(DPoint3d&, Json::Value const&);
    };

/*=================================================================================**//**
* @bsiclass                                     Shaun.Sewall                    01/2014
+===============+===============+===============+===============+===============+======*/
struct ECRapidJsonUtilities
    {
private:
    ECRapidJsonUtilities();
    ~ECRapidJsonUtilities();

#if !defined (DOCUMENTATION_GENERATOR)
    static BentleyStatus ECInstanceFromJsonValue(ECN::IECInstanceR instance, RapidJsonValueCR jsonValue, ECN::ECClassCR currentClass, Utf8StringCR currentAccessString);
    static BentleyStatus ECArrayValueFromJsonValue(ECN::IECInstanceR instance, RapidJsonValueCR jsonValue, ECN::ECPropertyCR, Utf8StringCR currentAccessString);
    static BentleyStatus ECPrimitiveValueFromJsonValue(ECN::ECValueR ecValue, RapidJsonValueCR jsonValue, ECN::PrimitiveType primitiveType);
#endif

public:
    //! Return an Int64 value from a RapidJsonValueCR that may be a number or a string.
    //! @param[in] value the source RapidJsonValueCR
    //! @param[in] defaultOnError the Int64 value to return if it is not possible to convert the source value to Int64.
    ECOBJECTS_EXPORT static int64_t Int64FromValue(RapidJsonValueCR value, int64_t defaultOnError = 0);

    ECOBJECTS_EXPORT static BentleyStatus ECInstanceFromJsonValue(ECN::IECInstanceR instance, RapidJsonValueCR jsonValue);
    };

END_BENTLEY_ECOBJECT_NAMESPACE
