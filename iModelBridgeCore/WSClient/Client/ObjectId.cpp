/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Client/ObjectId.cpp $
 |
 |  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#include "ClientInternal.h"
#include <BeSQLite/BeSQLite.h>
USING_NAMESPACE_BENTLEY_SQLITE

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma   05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ObjectId::ObjectId() :
schemaName(),
className(),
remoteId()
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma   04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ObjectId::ObjectId(Utf8StringCR classKey, Utf8StringCR remoteId) :
schemaName(),
className(),
remoteId(remoteId)
    {
    auto index = classKey.find('.');
    if (Utf8String::npos == index)
        {
        className = classKey;
        return;
        }
    schemaName = classKey.substr(0, index);
    className = classKey.substr(index + 1);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma   04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ObjectId::ObjectId(Utf8StringCR schemaName, Utf8StringCR className, Utf8StringCR remoteId) :
schemaName(schemaName),
className(className),
remoteId(remoteId)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma   07/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ObjectId::ObjectId(BentleyApi::ECN::ECClassCR ecClass, Utf8StringCR remoteId) :
schemaName(ecClass.GetSchema().GetName()),
className(ecClass.GetName()),
remoteId(remoteId)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Vytenis.Navalinskas   12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ObjectId::ObjectId(BentleyApi::ECN::ECClassCR ecClass) :
ObjectId(ecClass, "")
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma   04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool ObjectId::IsEmpty() const
    {
    return schemaName.empty() && className.empty() && remoteId.empty();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma   08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool ObjectId::IsValid() const
    {
    return !schemaName.empty() && !className.empty() && !remoteId.empty();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool ObjectId::operator < (const ObjectId& other) const
    {
    int compareResult = schemaName.CompareTo(other.schemaName);
    if (0 != compareResult)
        {
        return compareResult < 0;
        }
    compareResult = className.CompareTo(other.className);
    if (0 != compareResult)
        {
        return compareResult < 0;
        }
    compareResult = remoteId.CompareTo(other.remoteId);
    return compareResult < 0;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool ObjectId::operator == (const ObjectId& other) const
    {
    if (!remoteId.Equals(other.remoteId))
        {
        return false;
        }
    if (!className.Equals(other.className))
        {
        return false;
        }
    if (!schemaName.Equals(other.schemaName))
        {
        return false;
        }
    return true;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool ObjectId::operator != (const ObjectId& other) const
    {
    return !(*this == other);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Benjamin.Brown  01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ObjectId::ToString() const
    {
    return Utf8PrintfString("%s:%s:%s", schemaName.c_str(), className.c_str(), remoteId.c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Benjamin.Brown  01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ObjectId ObjectId::FromString(Utf8StringCR string)
    {
    bvector<Utf8String> tokens;
    BeStringUtilities::Split(string.c_str(), ":", NULL, tokens);

    if (tokens.size() != 3)
        {
        return ObjectId();
        }

    return ObjectId(tokens[0], tokens[1], tokens[2]);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ObjectId::GetClassKey() const
    {
    return Utf8PrintfString("%s.%s", schemaName.c_str(), className.c_str());
    }
