/*--------------------------------------------------------------------------------------+
 |
 |     $Source: PublicAPI/WebServices/Cache/Util/ECSqlAdapterCache.h $
 |
 |  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <WebServices/Cache/WebServicesCache.h>
#include <ECObjects/ECObjectsAPI.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

USING_NAMESPACE_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    01/2015
* Cache JsonInserter, JsonUpdater and other ECSql adapters for reuse
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename A> struct ECSqlAdapterCache
    {
    private:
        ECDb* m_ecdb;
        bmap<ECClassId, std::shared_ptr<A>> m_cache;

    public:
        ECSqlAdapterCache(ECDb& ecdb) : m_ecdb(&ecdb)
            {};

        //! Get cached or newly intialized adapter
        A& Get(ECClassCR ecClass)
            {
            auto it = m_cache.find(ecClass.GetId());
            if (it != m_cache.end())
                {
                return *it->second;
                }

            auto adapterPtr = std::make_shared<A>(*m_ecdb, ecClass);
            m_cache.insert({ecClass.GetId(), adapterPtr});
            return *adapterPtr;
            };
    };

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    01/2015
* Lazy load JsonInserter, JsonUpdater and other ECSql adapters
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename A> struct ECSqlAdapterLoader
    {
    private:
        ECDb* m_ecdb;
        ECClassCP m_ecClass;
        std::shared_ptr<A> m_adapter;

    public:
        ECSqlAdapterLoader(ECDb& ecdb, ECClassCR ecClass) : m_ecdb(&ecdb), m_ecClass(&ecClass)
            {};

        //! Lazy initialize adapter
        A& Get()
            {
            if (nullptr == m_adapter)
                {
                m_adapter = std::make_shared<A>(*m_ecdb, *m_ecClass);
                }

            return *m_adapter;
            };
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
