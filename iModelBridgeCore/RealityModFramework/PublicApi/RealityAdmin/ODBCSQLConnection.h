/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/RealityAdmin/ODBCSQLConnection.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__BENTLEY_INTERNAL_ONLY__

#include <RealityPlatform/RealityPlatformAPI.h>
#include <RealityPlatform/SpatialEntityData.h>

BEGIN_BENTLEY_REALITYPLATFORM_NAMESPACE

//=====================================================================================
//! Connection to the SQL Server, handles all transactions.
//! @bsiclass                                   Spencer.Mason            	     8/2016
//=====================================================================================
struct ServerConnection
    {
    private:
        SQLHENV     hEnv;
        SQLHDBC     hDbc;
        SQLHSTMT    hStmt;
        const char* m_dbName;
        static ServerConnection* s_instance;

        void OpenConnection();
        void HandleDiagnosticRecord(SQLHANDLE hHandle, SQLSMALLINT hType, RETCODE RetCode);

    public:
        ServerConnection();

        REALITYDATAPLATFORM_EXPORT static ServerConnection& GetInstance();

        SQL_TIMESTAMP_STRUCT PackageDateTime(DateTimeCR date);
        REALITYDATAPLATFORM_EXPORT SQL_DATE_STRUCT PackageDate(DateTimeCR dateTime);

        RETCODE ExecuteSQL(CHAR* query);
        RETCODE ExecuteSQL(SQLHSTMT stmt);
        RETCODE ExecutePrepared();

        REALITYDATAPLATFORM_EXPORT void SetStrings(const char* dbName, const char* pwszConnStr);
        void TryODBC(SQLHANDLE h, SQLSMALLINT ht, RETCODE x);
        SQLRETURN FetchTableIdentity(SQLINTEGER &id, const char* tableName, SQLLEN &len);
        void ReleaseStmt();
        void Exit();

        REALITYDATAPLATFORM_EXPORT bool IsDuplicate(Utf8CP file);
        bool IsMirror(Utf8CP file);
        bool HasEntries(CHAR* input);

        REALITYDATAPLATFORM_EXPORT void Save(SpatialEntityDataCR data, bool dualMode);
        REALITYDATAPLATFORM_EXPORT void Update(SpatialEntityDataCR data);
        REALITYDATAPLATFORM_EXPORT bool CheckExists(Utf8String id);
        REALITYDATAPLATFORM_EXPORT SQLINTEGER SaveServer(SpatialEntityServerCR server);
    };

END_BENTLEY_REALITYPLATFORM_NAMESPACE