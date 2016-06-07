/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/Cache/Persistence/CachedResponseKey.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <WebServices/Cache/WebServicesCache.h>
#include <ECDb/ECDbApi.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

USING_NAMESPACE_BENTLEY_SQLITE_EC

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    07/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct CachedResponseKey
    {
    private:
        ECInstanceKey m_parent;
        ECInstanceKey m_holder;
        Utf8String m_name;

    public:
        //! Create invalid key
        WSCACHE_EXPORT CachedResponseKey();

        //! Create cached respose key.
        //! param[in] parent - response will be kept as long as parent instance is available. Is also holder if not specified other way.
        //! param[in] name - is unique for same parent. Using same key second time will override previous results.
        //! param[in] holder - [optional] allows specifying different instance that will have holding relationship for response instances.
        //!             If specified, will have holding relationship to results and parent will only have referencing relationship.
        //!             Parent deletion and name uniquality still applies.
        WSCACHE_EXPORT CachedResponseKey(ECInstanceKeyCR parent, Utf8StringCR name, ECInstanceKeyCR holder = ECInstanceKey());

        WSCACHE_EXPORT ECInstanceKeyCR GetParent() const;
        WSCACHE_EXPORT Utf8StringCR GetName() const;
        WSCACHE_EXPORT ECInstanceKeyCR GetHolder() const;
        WSCACHE_EXPORT void SetHolder(ECInstanceKey holder);

        WSCACHE_EXPORT bool IsValid() const;

        WSCACHE_EXPORT bool operator== (const CachedResponseKey& other) const;
    };

typedef CachedResponseKey& CachedResponseKeyR;
typedef const CachedResponseKey& CachedResponseKeyCR;

END_BENTLEY_WEBSERVICES_NAMESPACE
