/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityAdmin/ODBCSQLConnection.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__BENTLEY_INTERNAL_ONLY__

#include <RealityPlatform/RealityPlatformAPI.h>
#include <RealityPlatform/SpatialEntityData.h>

BEGIN_BENTLEY_REALITYPLATFORM_NAMESPACE

//=======================================================================================
// @bsiclass
// Connection to the SQL Server, handles all transactions
//=======================================================================================
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
    REALITYDATAPLATFORM_EXPORT static ServerConnection& GetInstance();
    ServerConnection();
    REALITYDATAPLATFORM_EXPORT void SetStrings(const char* dbName, const char* pwszConnStr);
    void TryODBC(SQLHANDLE h, SQLSMALLINT ht, RETCODE x);
    SQL_TIMESTAMP_STRUCT PackageDateTime(DateTimeCR date);
    SQL_DATE_STRUCT PackageDate(DateTimeCR dateTime);
    RETCODE ExecuteSQL(CHAR* query);
    RETCODE ExecuteSQL(SQLHSTMT stmt);
    RETCODE ExecutePrepared();
    SQLRETURN FetchTableIdentity(SQLINTEGER &id, const char* tableName, SQLLEN &len);
    void ReleaseStmt();
    REALITYDATAPLATFORM_EXPORT bool IsDuplicate(Utf8CP file);
    bool IsMirror(Utf8CP file);
    bool HasEntries(CHAR* input);
    void Exit();

    REALITYDATAPLATFORM_EXPORT void Save(SpatialEntityDataCR data, bool dualMode);
    REALITYDATAPLATFORM_EXPORT void Update(SpatialEntityDataCR data);
    REALITYDATAPLATFORM_EXPORT bool CheckExists(Utf8String id);
    REALITYDATAPLATFORM_EXPORT SQLINTEGER SaveServer(std::string url);
};

END_BENTLEY_REALITYPLATFORM_NAMESPACE