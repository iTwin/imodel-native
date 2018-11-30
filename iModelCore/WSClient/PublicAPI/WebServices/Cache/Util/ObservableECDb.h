/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/Cache/Util/ObservableECDb.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <WebServices/Cache/WebServicesCache.h>
#include <ECDb/ECDbApi.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

USING_NAMESPACE_BENTLEY_SQLITE_EC

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
struct IECDbSchemaChangeListener
    {
    public:
        //! Reset all ECSchema, ECClass, ECSqlStatement, etc. referances that point to old schema
        virtual void OnSchemaChanged() = 0;
    };

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    01/2015
* Class for detecting when ECSchemas changed on ECDb. It will automtaically notify listeners
* if schemas were changed on other connection. Changes to same connection are notified only
* by calling NotifyOnSchemaChangedListeners()
+---------------+---------------+---------------+---------------+---------------+------*/
struct EXPORT_VTABLE_ATTRIBUTE ObservableECDb : public ECDb
    {
    private:
        bset<IECDbSchemaChangeListener*> m_cacheSchemaChangeListeners;

    protected:
        WSCACHE_EXPORT virtual void _OnDbChangedByOtherConnection() override;

    public:
        //! Register for ECSchema change events.
        //! Old ECSchema, ECClass, ECSqlStatement and other resource references must be removed when schema changes.
        WSCACHE_EXPORT void RegisterSchemaChangeListener(IECDbSchemaChangeListener* listener);

        //! Unregister from ECSchema change events.
        WSCACHE_EXPORT void UnRegisterSchemaChangeListener(IECDbSchemaChangeListener* listener);

        //! Explicitly notify all listeners
        WSCACHE_EXPORT void NotifyOnSchemaChangedListeners();
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
