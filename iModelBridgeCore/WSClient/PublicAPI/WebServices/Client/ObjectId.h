/*--------------------------------------------------------------------------------------+
 |
 |     $Source: PublicAPI/WebServices/Client/ObjectId.h $
 |
 |  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <WebServices/Client/WebServicesClient.h>
#include <ECObjects/ECObjectsAPI.h>
#include <Bentley/WString.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct ObjectId
    {
    WSCLIENT_EXPORT ObjectId();
    //! Sets schemaName and className from class key ("SchemaName.ClassName"). Also deprecated support for "ClassName" instead of classKey
    WSCLIENT_EXPORT ObjectId(Utf8StringCR classKey, Utf8StringCR remoteId);
    WSCLIENT_EXPORT ObjectId(Utf8StringCR schemaName, Utf8StringCR className, Utf8StringCR remoteId);
    WSCLIENT_EXPORT ObjectId(BentleyApi::ECN::ECClassCR ecClass, Utf8StringCR remoteId);
    WSCLIENT_EXPORT ObjectId(BentleyApi::ECN::ECClassCR ecClass);

    //! Returns true if schemaName, className and remoteId is empty
    WSCLIENT_EXPORT bool IsEmpty() const;
    //! Returns false if any of fields are empty thus meaning incomplete object id
    WSCLIENT_EXPORT bool IsValid() const;

    WSCLIENT_EXPORT Utf8String ToString() const;
    WSCLIENT_EXPORT static ObjectId FromString(Utf8StringCR string);

    WSCLIENT_EXPORT bool operator < (const ObjectId& other) const;
    WSCLIENT_EXPORT bool operator == (const ObjectId& other) const;
    WSCLIENT_EXPORT bool operator != (const ObjectId& other) const;

    Utf8String schemaName;
    Utf8String className;
    Utf8String remoteId;

    WSCLIENT_EXPORT Utf8String GetClassKey() const;
    };

typedef const ObjectId& ObjectIdCR;
typedef ObjectId& ObjectIdR;

END_BENTLEY_WEBSERVICES_NAMESPACE
