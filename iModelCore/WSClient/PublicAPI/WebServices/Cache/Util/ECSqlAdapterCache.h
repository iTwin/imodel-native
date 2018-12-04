/*--------------------------------------------------------------------------------------+
 |
 |     $Source: PublicAPI/WebServices/Cache/Util/ECSqlAdapterCache.h $
 |
 |  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <WebServices/Cache/WebServicesCache.h>
#include <ECObjects/ECObjectsAPI.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC

#define ECSqlUpdater_Options_IgnoreSystemAndFailReadOnlyProperties JsonUpdater::Options(JsonUpdater::SystemPropertiesOption::Ignore, JsonUpdater::ReadonlyPropertiesOption::Fail)
#define ECSqlUpdater_Options_IgnoreSystemAndUpdateReadOnlyProperties JsonUpdater::Options(JsonUpdater::SystemPropertiesOption::Ignore, JsonUpdater::ReadonlyPropertiesOption::Update)
/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    05/2016
* Cache JsonUpdaters with ECSQL options
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename TAdapter, typename TOptions = void> struct ECSqlAdapterCache
    {
    private:
        ECDb* m_ecdb;
        bmap<ECClassId, std::shared_ptr<TAdapter>> m_cache;
        TOptions m_options;

    public:
        ECSqlAdapterCache(ECDb& ecdb, TOptions const& options) : m_ecdb(&ecdb), m_options(options) {};

        //! Get cached or newly intialized adapter
        TAdapter& Get(ECClassCR ecClass)
            {
            auto it = m_cache.find(ecClass.GetId());
            if (it != m_cache.end())
                return *it->second;

            auto adapterPtr = std::make_shared<TAdapter>(*m_ecdb, ecClass, nullptr, m_options);
            m_cache.insert({ecClass.GetId(), adapterPtr});
            return *adapterPtr;
            };
    };

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    01/2015
* Cache JsonInserter, JsonUpdater and other ECSql adapters for reuse
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename TAdapter> struct ECSqlAdapterCache <TAdapter, void>
    {
    private:
        ECDb* m_ecdb;
        bmap<ECClassId, std::shared_ptr<TAdapter>> m_cache;

    public:
        ECSqlAdapterCache(ECDb& ecdb) : m_ecdb(&ecdb) {};

        //! Get cached or newly intialized adapter
        TAdapter& Get(ECClassCR ecClass)
            {
            auto it = m_cache.find(ecClass.GetId());
            if (it != m_cache.end())
                return *it->second;

            auto adapterPtr = std::make_shared<TAdapter>(*m_ecdb, ecClass, nullptr);
            m_cache.insert({ecClass.GetId(), adapterPtr});
            return *adapterPtr;
            };
    };

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                  julius.cepukenas    10/2017
* Lazy load JsonUpdater with ECSQL options
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename TAdapter, typename TOptions = void> struct ECSqlAdapterLoader
    {
    private:
        ECDb* m_ecdb;
        ECClassCP m_ecClass;
        std::shared_ptr<TAdapter> m_adapter;
        TOptions m_options;

    public:
        ECSqlAdapterLoader(ECDb& ecdb, ECClassCR ecClass, TOptions const& options) : m_ecdb(&ecdb), m_ecClass(&ecClass), m_options(options) {};

        //! Lazy initialize adapter
        TAdapter& Get()
            {
            if (nullptr == m_adapter)
                m_adapter = std::make_shared<TAdapter>(*m_ecdb, *m_ecClass, nullptr, m_options);

            return *m_adapter;
            };
    };

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    01/2015
* Lazy load JsonInserter, JsonUpdater and other ECSql adapters
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename TAdapter> struct ECSqlAdapterLoader <TAdapter, void>
    {
    private:
        ECDb* m_ecdb;
        ECClassCP m_ecClass;
        std::shared_ptr<TAdapter> m_adapter;

    public:
        ECSqlAdapterLoader(ECDb& ecdb, ECClassCR ecClass) : m_ecdb(&ecdb), m_ecClass(&ecClass) {};

        //! Lazy initialize adapter
        TAdapter& Get()
            {
            if (nullptr == m_adapter)
                m_adapter = std::make_shared<TAdapter>(*m_ecdb, *m_ecClass, nullptr);

            return *m_adapter;
            };
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
