/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/RealityPlatform/AwsTraverser/AwsTraverser.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__BENTLEY_INTERNAL_ONLY__

#include <RealityPlatform/RealityPlatformAPI.h>

#include <Bentley/DateTime.h>
#include <curl/curl.h>
#include <string>

BEGIN_BENTLEY_REALITYPLATFORM_NAMESPACE

struct AwsData
    {
    std::string m_id, m_downloadUrl;
    float m_cloudCover;
    DRange2d m_ftPrint;
    float m_redSize, m_blueSize, m_greenSize, m_panSize;
    SQLINTEGER serverId;

    AwsData(std::string id, std::string downloadUrl, float cloudCover,
        DRange2d ftPrint, float red, float green, float blue, float pan,
        SQLINTEGER sId);

    std::string GetId() { return m_id; }
    std::string GetUrl() { return m_downloadUrl; }
    float GetCover() { return m_cloudCover; }
    DRange2dCR GetFootprint() { return m_ftPrint; }
    float GetRedSize() { return m_redSize; }
    float GetBlueSize() { return m_blueSize; }
    float GetGreenSize() { return m_greenSize; }
    float GetPanchromaticSize() { return m_panSize; }
    SQLINTEGER GetServerId() { return serverId; }
    };

struct AwsPinger
    {
    static AwsPinger* s_instance;
    Utf8String m_certificatePath;
    CURL* m_curl;

    void ReadPage(Utf8CP url, float& redSize, float& greenSize, float& blueSize, float& panSize);
public:

    AwsPinger();
    ~AwsPinger();
    float m_redSize, m_blueSize, m_greenSize, m_panSize;
    static AwsPinger& GetInstance();
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
    const char* m_dbName;
    static ServerConnection* s_instance;
    void OpenConnection();
    void HandleDiagnosticRecord(SQLHANDLE hHandle, SQLSMALLINT hType, RETCODE RetCode);

public:
    static ServerConnection& GetInstance();
    ServerConnection();
    void SetStrings(const char* dbName, const char* pwszConnStr);
    void TryODBC(SQLHANDLE h, SQLSMALLINT ht, RETCODE x);
    SQL_TIMESTAMP_STRUCT PackageDateTime(DateTimeCR date);
    SQL_DATE_STRUCT PackageDate(DateTimeCR dateTime);
    RETCODE ExecuteSQL(CHAR* query);
    RETCODE ExecuteSQL(SQLHSTMT stmt);
    RETCODE ExecutePrepared();
    SQLRETURN FetchTableIdentity(SQLINTEGER &id, const char* tableName, SQLLEN &len);
    void ReleaseStmt();
    bool IsDuplicate(Utf8CP file);
    bool HasEntries(CHAR* input);
    void Exit();

    void Save(AwsData awsdata);
    bool CheckExists(std::string id);
    SQLINTEGER SaveServer(std::string url);
    SQLINTEGER SaveMetadata();
    };

END_BENTLEY_REALITYPLATFORM_NAMESPACE