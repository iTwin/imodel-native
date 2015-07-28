/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Cache/NavigationCachingOptions.cpp $
 |
 |  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/

#include <WebServices/Cache/NavigationCachingOptions.h>

#include <WebServices/Cache/Util/ECCustomAttributeHelper.h>

USING_NAMESPACE_BENTLEY_WEBSERVICES

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
NavigationCachingOptions::NavigationCachingOptions ()
    {
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void NavigationCachingOptions::SetClassesToAlwaysCacheChildren (const bset<Utf8String>& classes)
    {
    m_classesToAlwaysCacheChildren = classes;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool NavigationCachingOptions::ShouldAlwaysCacheChildrenForClass (Utf8StringCR className) const
    {
    return m_classesToAlwaysCacheChildren.find (className) != m_classesToAlwaysCacheChildren.end ();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool NavigationCachingOptions::CanObjectHaveChildren (ObjectIdCR objectId) const
    {
    if (objectId.IsEmpty ())
        {
        return true;
        }
    return true; // TODO: false if NavNode.hasChildren == false or if WebApi2 and not NavNode. Always true for WebApi1
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool NavigationCachingOptions::IsFileClass (ECClassCP ecClass) const
    {
    return nullptr != ecClass && !ecClass->GetCustomAttribute (L"FileDependentProperties").IsNull ();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String NavigationCachingOptions::GetFileSizeProperty (ECClassCP ecClass) const
    {
    return ECCustomAttributeHelper::GetPropertyName (ecClass, L"FileDependentProperties", L"FileSize");
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String NavigationCachingOptions::GetFileNameProperty (ECClassCP ecClass) const
    {
    return ECCustomAttributeHelper::GetPropertyName (ecClass, L"FileDependentProperties", L"FileName");
    }
