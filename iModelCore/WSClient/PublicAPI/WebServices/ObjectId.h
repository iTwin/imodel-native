/*--------------------------------------------------------------------------------------+
 |
 |     $Source: PublicAPI/WebServices/ObjectId.h $
 |
 |  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <WebServices/Common.h>
#include <ECObjects/ECObjectsAPI.h>
#include <Bentley/WString.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct ObjectId
    {
    WS_EXPORT ObjectId ();
    //! Sets schemaName and className from class key ("SchemaName.ClassName"). Also deprecated support for "ClassName" instead of classKey
    WS_EXPORT ObjectId (Utf8StringCR classKey, Utf8StringCR remoteId);
    WS_EXPORT ObjectId (Utf8StringCR schemaName, Utf8StringCR className, Utf8StringCR remoteId);
    WS_EXPORT ObjectId (BentleyApi::ECN::ECClassCR ecClass, Utf8StringCR remoteId);
    WS_EXPORT ObjectId (BentleyApi::ECN::ECClassCR ecClass);

    //! Returns true if schemaName, className and remoteId is empty
    WS_EXPORT bool IsEmpty () const;
    //! Returns false if any of fields are empty thus meaning incomplete object id
    WS_EXPORT bool IsValid () const;

    WS_EXPORT Utf8String ToString () const;
    WS_EXPORT static ObjectId FromString (Utf8StringCR string);

    WS_EXPORT bool operator < (const ObjectId& other) const;
    WS_EXPORT bool operator == (const ObjectId& other) const;
    WS_EXPORT bool operator != (const ObjectId& other) const;

    Utf8String schemaName;
    Utf8String className;
    Utf8String remoteId;

    WS_EXPORT Utf8String GetClassKey () const;
    };

typedef const ObjectId& ObjectIdCR;
typedef ObjectId& ObjectIdR;

END_BENTLEY_WEBSERVICES_NAMESPACE
