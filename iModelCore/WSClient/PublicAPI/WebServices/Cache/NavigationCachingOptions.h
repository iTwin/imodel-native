/*--------------------------------------------------------------------------------------+
 |
 |     $Source: PublicAPI/WebServices/Cache/NavigationCachingOptions.h $
 |
 |  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <BeJsonCpp/BeJsonUtilities.h>

#include <WebServices/Cache/WebServicesCache.h>
#include <WebServices/Cache/Persistence/IDataSourceCache.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma   05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct NavigationCachingOptions
    {
    protected:
        bset<Utf8String> m_classesToAlwaysCacheChildren;

    public:
        WSCACHE_EXPORT NavigationCachingOptions();

        WSCACHE_EXPORT void SetClassesToAlwaysCacheChildren(const bset<Utf8String>& classes);

        WSCACHE_EXPORT bool ShouldAlwaysCacheChildrenForClass(Utf8StringCR className) const;

        WSCACHE_EXPORT bool CanObjectHaveChildren(ObjectIdCR objectId) const;

        // Uses FileDependentProperties custom attribute
        WSCACHE_EXPORT bool IsFileClass(ECClassCP ecClass) const;
        WSCACHE_EXPORT Utf8String GetFileSizeProperty(ECClassCP ecClass) const;
        WSCACHE_EXPORT Utf8String GetFileNameProperty(ECClassCP ecClass) const;
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
