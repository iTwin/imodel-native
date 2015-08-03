/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Cache/Util/ECCustomAttributeHelper.cpp $
 |
 |  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/

#include <WebServices/Cache/Util/ECCustomAttributeHelper.h>

#include <Bentley/BeDebugLog.h>

USING_NAMESPACE_BENTLEY_WEBSERVICES

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECValue ECCustomAttributeHelper::GetValue (IECCustomAttributeContainerCP caContainer, WCharCP caName, WCharCP caPropertyName)
    {
    ECValue value;
    if (nullptr != caContainer)
        {
        IECInstancePtr attribute = caContainer->GetCustomAttribute (caName);
        if (!attribute.IsNull())
            {
            attribute->GetValue (value, caPropertyName);
            }
        }
    return value;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
int32_t ECCustomAttributeHelper::GetInteger (IECCustomAttributeContainerCP caContainer, WCharCP caName, WCharCP caPropertyName, int32_t defaultValue)
    {
    ECValue value = GetValue (caContainer, caName, caPropertyName);
    if (value.IsNull())
        {
        return defaultValue;
        }
    return value.GetInteger();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECCustomAttributeHelper::GetBool (IECCustomAttributeContainerCP caContainer, WCharCP caName, WCharCP caPropertyName, bool defaultValue)
    {
    ECValue value = GetValue (caContainer, caName, caPropertyName);
    if (value.IsNull())
        {
        return defaultValue;
        }
    return value.GetBoolean();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ECCustomAttributeHelper::GetString (IECCustomAttributeContainerCP caContainer, WCharCP caName, WCharCP caPropertyName)
    {
    Utf8String stringValue;
    ECValue value = GetValue (caContainer, caName, caPropertyName);
    if (!value.IsNull())
        {
        stringValue = value.GetUtf8CP();
        }
    return stringValue;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<Utf8String> ECCustomAttributeHelper::GetStringArray (IECCustomAttributeContainerCP caContainer, WCharCP caName, WCharCP caPropertyName)
    {
    bvector<Utf8String> values;

    IECInstancePtr attribute;
    ECValue arrayValue;
    if (nullptr != caContainer)
        {
        attribute = caContainer->GetCustomAttribute (caName);
        if (!attribute.IsNull())
            {
            attribute->GetValue (arrayValue, caPropertyName);
            }
        }

    if (!arrayValue.IsNull())
        {
        for (uint32_t i=0; i < arrayValue.GetArrayInfo().GetCount(); i++)
            {
            ECValue itemValue;
            attribute->GetValue (itemValue, caPropertyName, i);

            values.push_back (itemValue.GetUtf8CP());
            }
        }

    return values;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ECCustomAttributeHelper::ValidatePropertyName (ECClassCP ecClass, Utf8StringR propertyName)
    {
    if (propertyName.empty())
        {
        return;
        }
    if (ecClass->GetPropertyP (propertyName.c_str()) == nullptr)
        {
        BeDebugLog (Utf8PrintfString ("Custom attribute defines property name \"%s\" that does not exist in class \"%s\"",
                                      propertyName.c_str(), Utf8String (ecClass->GetName()).c_str()).c_str());
        BeAssert (false);
        propertyName.clear();
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ECCustomAttributeHelper::GetPropertyName (ECClassCP ecClass, WCharCP caName, WCharCP caPropertyName)
    {
    Utf8String propertyName = GetString (ecClass, caName, caPropertyName);
    ValidatePropertyName (ecClass, propertyName);
    return propertyName;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<Utf8String> ECCustomAttributeHelper::GetPropertyNameArray (ECClassCP ecClass, WCharCP caName, WCharCP caPropertyName)
    {
    bvector<Utf8String> properties;

    bvector<Utf8String> values = GetStringArray (ecClass, caName, caPropertyName);
    for (Utf8StringR propertyName : values)
        {
        ValidatePropertyName (ecClass, propertyName);

        if (!propertyName.empty())
            {
            properties.push_back (propertyName);
            }
        }

    return properties;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bset<Utf8String> ECCustomAttributeHelper::GetPropertyNameSet (ECClassCP ecClass, WCharCP caName, WCharCP caPropertyName)
    {
    bset<Utf8String> properties;

    bvector<Utf8String> values = GetStringArray (ecClass, caName, caPropertyName);
    for (Utf8StringR propertyName : values)
        {
        ValidatePropertyName (ecClass, propertyName);

        if (!propertyName.empty())
            {
            properties.insert (propertyName);
            }
        }

    return properties;
    }

