/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/RealityPlatform/FtpTraverser/FtpTraverser.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__BENTLEY_INTERNAL_ONLY__

#include <RealityPlatform/RealityPlatformAPI.h>
#include <RealityPlatform/FtpTraversalEngine.h>

#include <curl/curl.h>
#include <string>

BEGIN_BENTLEY_REALITYPLATFORM_NAMESPACE

//=======================================================================================
// @bsiclass
// Observer handling downloaded data
//=======================================================================================

struct FtpTraversalObserver: public IFtpTraversalObserver
    {
    bool m_dualMode = false;
    bool m_updateMode = false;
public:

    FtpTraversalObserver(bool dualMode, bool updateMode);

    virtual void OnFileListed(bvector<Utf8String>& fileList, Utf8CP file);
    virtual void OnFileDownloaded(Utf8CP file);
    virtual void OnDataExtracted(FtpDataCR data);
    };

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
    static ServerConnection* s_instance;
    void OpenConnection();
    void HandleDiagnosticRecord(SQLHANDLE hHandle, SQLSMALLINT hType, RETCODE RetCode);

public:
    static ServerConnection& GetInstance();
    ServerConnection();
    void TryODBC(SQLHANDLE h, SQLSMALLINT ht, RETCODE x);
    SQL_TIMESTAMP_STRUCT PackageDateTime(DateTimeCR date);
    SQL_DATE_STRUCT PackageDate(DateTimeCR dateTime);
    RETCODE ExecuteSQL(CHAR* query);
    RETCODE ExecuteSQL(SQLHSTMT stmt);
    RETCODE ExecutePrepared();
    SQLRETURN FetchTableIdentity(SQLINTEGER &id, char* tableName, SQLLEN &len);
    void ReleaseStmt();
    bool IsDuplicate(Utf8CP file);
    bool IsMirror(Utf8CP file);
    bool HasEntries(CHAR* input);
    void Exit();

    void Save(FtpDataCR data, bool dualMode);
    void Update(FtpDataCR data);
    };

END_BENTLEY_REALITYPLATFORM_NAMESPACE