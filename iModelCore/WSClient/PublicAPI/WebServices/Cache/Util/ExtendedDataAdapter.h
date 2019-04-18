/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <WebServices/Cache/WebServicesCache.h>
#include <WebServices/Cache/Util/ECDbAdapter.h>
#include <WebServices/Cache/Util/IExtendedDataAdapter.h>
#include <WebServices/Cache/Util/ECSqlStatementCache.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

USING_NAMESPACE_BENTLEY_SQLITE_EC

/*--------------------------------------------------------------------------------------+
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct EXPORT_VTABLE_ATTRIBUTE ExtendedDataAdapter : public IExtendedDataAdapter
    {
    public:
        struct IDelegate;

    private:
        ECDbAdapter m_dbAdapter;
        ECSqlStatementCache m_statementCache;
        IDelegate& m_delegate;

    public:
        //! Create extended data adapter
        WSCACHE_EXPORT ExtendedDataAdapter(ObservableECDb& db, IDelegate& edDelegate);

        WSCACHE_EXPORT ExtendedData GetData(ECInstanceKeyCR ownerKey) override;
        WSCACHE_EXPORT BentleyStatus UpdateData(ExtendedData& data) override;
    };

/*--------------------------------------------------------------------------------------+
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct ExtendedDataAdapter::IDelegate
    {
    public:
        virtual ~IDelegate() {};
        //! Return class key with "Content" string property for storing data
        virtual ECClassCP GetExtendedDataClass(ECInstanceKeyCR ownerKey) = 0;
        //! Return relationship class with source "owner" instances to hold target "extended data" class instances
        virtual ECRelationshipClassCP GetExtendedDataRelationshipClass(ECInstanceKeyCR ownerKey) = 0;
        //! Return instance key that should hold extended data for specific owner so schema would match.
        virtual ECInstanceKey GetHolderKey(ECInstanceKeyCR ownerKey) { return ownerKey; };
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
