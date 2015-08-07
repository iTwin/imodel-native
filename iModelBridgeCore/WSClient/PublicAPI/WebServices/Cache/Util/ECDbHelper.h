/*--------------------------------------------------------------------------------------+
 |
 |     $Source: PublicAPI/WebServices/Cache/Util/ECDbHelper.h $
 |
 |  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <Bentley/Bentley.h>
#include <ECDb/ECDbApi.h>
#include <ECObjects/ECObjectsAPI.h>

#include <WebServices/Cache/WebServicesCache.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

USING_NAMESPACE_EC
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC

#define WIDEN_(x)  L ## x
#define WIDEN(x)   WIDEN_(x)

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma   02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct ECDbHelper
    {
    public:
        WSCACHE_EXPORT static ECSchemaPtr LocateSchema(SchemaKeyCR schemaKey, BeFileNameCR schemaDir);

        //! Performs clean schema copy with referances to other schemas from ECDb.
        WSCACHE_EXPORT static ECSchemaPtr CopySchema(ECSchemaCR schema, IECSchemaLocater* optionalLocator = nullptr);

        WSCACHE_EXPORT static ECInstanceId ECInstanceIdFromECInstance(IECInstanceCR ecInstance);
        WSCACHE_EXPORT static ECInstanceId ECInstanceIdFromString(Utf8CP ecIdString);
        WSCACHE_EXPORT static ECInstanceId ECInstanceIdFromJsonInstance(JsonValueCR jsonInstance);
        WSCACHE_EXPORT static ECInstanceId ECInstanceIdFromJsonValue(JsonValueCR jsonValue);
        WSCACHE_EXPORT static Utf8String StringFromECInstanceId(ECInstanceId ecInstanceId);

        WSCACHE_EXPORT static Utf8String ECClassKeyFromClass(ECClassCR ecClass);
        WSCACHE_EXPORT static void ParseECClassKey(Utf8StringCR classKey, Utf8StringR schemaNameOut, Utf8StringR classNameOut);

        WSCACHE_EXPORT static ECRelationshipClassCP GetRelationshipClass(ECSchemaCP schema, WCharCP className);

        //! Convert date to ISO string if date is UTC, else assert and return empty
        WSCACHE_EXPORT static Utf8String UtcDateToString(DateTimeCR utcDate);

        //! Returns true if class instance represents object - can be created and ECDb and is not relationship
        WSCACHE_EXPORT static bool IsObjectClass(ECClassCR ecClass);

        //! ECInstanceKeyMultiMap util
        WSCACHE_EXPORT static bool IsInstanceInMultiMap(ECInstanceKeyCR instance, const ECInstanceKeyMultiMap& map);
        //! ECInstanceKeyMultiMap util
        WSCACHE_EXPORT static ECInstanceKeyMultiMapPair ToPair(ECInstanceKeyCR key);
        //! ECInstanceKeyMultiMap util
        WSCACHE_EXPORT static void Erase(ECInstanceKeyMultiMap& map, ECInstanceKeyCR key);
        //! Convert ECInstanceKeyMultiMap iterators to comma seperated ECInstanceId list
        WSCACHE_EXPORT static Utf8String ToECInstanceIdList
            (
            ECInstanceKeyMultiMap::const_iterator from,
            ECInstanceKeyMultiMap::const_iterator to
            );
    };

WSCACHE_EXPORT std::ostream& operator << (std::ostream &o, ECInstanceId ecInstanceId);

END_BENTLEY_WEBSERVICES_NAMESPACE
