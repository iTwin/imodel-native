/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityPlatform/FtpTraverser/FtpTraverser.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <windows.h>
#include <sql.h>
#include <sqlext.h>
#include <stdio.h>
#include <conio.h>
#include <tchar.h>
#include <stdlib.h>
#include <sal.h>

#include <iostream>
#include <vector>
#include <RealityPlatform/FtpTraverser/FtpTraverser.h>

BEGIN_BENTLEY_REALITYPLATFORM_NAMESPACE

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason            	    8/2016
//-------------------------------------------------------------------------------------
ServerConnection::ServerConnection()
    {
    s_instance = this;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason            	    8/2016
//-------------------------------------------------------------------------------------
ServerConnection* ServerConnection::s_instance = nullptr;
ServerConnection& ServerConnection::GetInstance()
    {
    if (nullptr == s_instance)
        s_instance = new ServerConnection();
    return *s_instance;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason            	    8/2016
//-------------------------------------------------------------------------------------
void ServerConnection::SetStrings(const char* dbName, const char* pwszConnStr)
    {
    m_dbName = dbName;
    hEnv = NULL; 
    hDbc = NULL; 
    hStmt = NULL;

    if (SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv) == SQL_ERROR)
        {
        fwprintf(stderr, L"Unable to allocate an environment handle\n");
        exit(-1);
        }

    // Register this as an application that expects 3.x behavior,
    // you must register something if you use AllocHandle

    TryODBC(hEnv,
        SQL_HANDLE_ENV,
        SQLSetEnvAttr(hEnv,
        SQL_ATTR_ODBC_VERSION,
        (SQLPOINTER)SQL_OV_ODBC3,
        0));

    // Allocate a connection
    TryODBC(hEnv,
        SQL_HANDLE_ENV,
        SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbc));

    // Connect to the driver.  Use the connection string if supplied
    // on the input, otherwise let the driver manager prompt for input.

    TryODBC(hDbc,
        SQL_HANDLE_DBC,
        SQLDriverConnect(hDbc,
        NULL,
        (SQLCHAR*)pwszConnStr,
        SQL_NTS,
        NULL,
        0,
        NULL,
        SQL_DRIVER_COMPLETE));

    TryODBC(hDbc,
        SQL_HANDLE_DBC,
        SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt));

    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason            	    8/2016
//-------------------------------------------------------------------------------------
void ServerConnection::TryODBC(SQLHANDLE h, SQLSMALLINT ht, RETCODE x)
    {
    RETCODE rc = x;
    if (rc != SQL_SUCCESS)
        { \
        HandleDiagnosticRecord(h, ht, rc);
        }
    if (rc == SQL_ERROR)
        {
        std::cout << "Error in " << stderr << std::endl;
        Exit(); 
        } 
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason            	    8/2016
//-------------------------------------------------------------------------------------
RETCODE ServerConnection::ExecuteSQL(CHAR* query)
    {
    RETCODE retcode = 0;
    TryODBC(hStmt, 
        SQL_HANDLE_STMT, 
        retcode = SQLExecDirect(hStmt, (SQLCHAR*)query, SQL_NTS));
    
    return retcode;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason            	    8/2016
//-------------------------------------------------------------------------------------
RETCODE ServerConnection::ExecuteSQL(SQLHSTMT stmt)
    {
    RETCODE retcode = 0;
    TryODBC(hStmt,
        SQL_HANDLE_STMT,
        retcode = SQLExecute(stmt));
    return retcode;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason            	    8/2016
//-------------------------------------------------------------------------------------
SQLRETURN ServerConnection::FetchTableIdentity(SQLINTEGER &id, const char* tableName, SQLLEN &len)
    {
    SQLRETURN retCode;
    CHAR ident[256];
    sprintf(ident, "SELECT IDENT_CURRENT('%s') as [SCOPE_IDENTITY]", tableName);
    ExecuteSQL(ident);
    SQLBindCol(hStmt, 1, SQL_INTEGER, &id, 2, &len);
    TryODBC(hStmt, SQL_HANDLE_STMT, retCode = SQLFetch(hStmt));
    ReleaseStmt();
    return retCode;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason            	    8/2016
//-------------------------------------------------------------------------------------
void ServerConnection::ReleaseStmt()
    {
     TryODBC(hStmt,
         SQL_HANDLE_STMT,
         SQLFreeStmt(hStmt, SQL_CLOSE));
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason            	    8/2016
//-------------------------------------------------------------------------------------
void ShowUsage()
    {
    std::cout << "Usage: ftptraverser.exe FtpUrl [DualFtpUrl] [options]" << std::endl <<std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -h, --help              Show this help message and exit" << std::endl;
    std::cout << "  -u, --update            Enable update mode" << std::endl;
    std::cout << "  -provider:PROVIDER      Set provider name" << std::endl;
    std::cout << "  -cs, --connectionString Connection string to connect to the db (Required)" << std::endl;
    std::cout << "  if there are spaces in an argument, surround it with \"\" " << std::endl;
    std::cout << "  as in \"-cs:Driver={SQL Server}(...)\" " << std::endl;

    std::cout << std::endl << "Press any key to exit." << std::endl;
    getch();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason            	    8/2016
//-------------------------------------------------------------------------------------
Utf8CP EnumString(FtpStatus status)
    {
    switch (status)
        {
        case FtpStatus::Success:
            return "Success";
        case FtpStatus::ClientError:
            return "Client Error";
        case FtpStatus::CurlError:
            return "Curl Error";
        case FtpStatus::DataExtractError:
            return "Data Extract Error";
        case FtpStatus::DownloadError:
            return "Download Error";
        default:
            return "Unknown Error";
        }
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason            	    8/2016
//-------------------------------------------------------------------------------------
int main(int argc, char *argv[])
    {
    SetConsoleTitle((LPCTSTR)"FTP Traversal Engine");

    auto argIt = argv;
    int ftpUrlCount = 0;
    char* ftp = "ftp://";
    for (int i = 0; i < argc - 1; ++i)
        {
        char* input = *++argIt;
        if (strstr(input, ftp) != nullptr)
            ftpUrlCount++;
        }

    if (1 > ftpUrlCount || 2 < ftpUrlCount || 5 < argc)
        {
        ShowUsage();

        return 0;
        }

    bool dualMode = (2 == ftpUrlCount);
    bool updateMode = false;
    std::string provider;
    std::vector<std::string> ftpUrls = std::vector<std::string>(ftpUrlCount);
    char* substringPosition;
    std::string dbName;
    std::string pwszConnStr;
    bool hasCString = false;
    ftpUrls.clear();
    for (int i = 0; i < argc; ++i)
        {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0)
            {
            ShowUsage();
            return 0;
            }
        else if (strcmp(argv[i], "-u") == 0 || strcmp(argv[i], "--update") == 0)
            updateMode = true;
        else if (strcmp(argv[i], "-provider:") == 0)
            {
            substringPosition = strstr(argv[i], ":");
            substringPosition++;
            provider = std::string(substringPosition);
            }
        else if (strstr(argv[i], "ftp://"))
            ftpUrls.push_back(std::string(argv[i]));
        else if (strstr(argv[i], "--connectionString:") || strstr(argv[i], "-cs:"))
            {
            std::string argument = std::string(argv[i]);
            size_t index = argument.find(":");
            substringPosition = argv[i] + index;
            substringPosition ++;
            pwszConnStr = std::string(substringPosition);
            
            size_t dbIndex = argument.find("Database=");
            std::string dbWithExtra = argument.substr(dbIndex + 9);
            dbIndex = dbWithExtra.find(";");
            dbName = dbWithExtra.substr(0,dbIndex);

            hasCString = true;
            }
        }

    if (!hasCString)
        {
        ShowUsage();
        return 0;
        }

    FtpStatus status = FtpStatus::UnknownError;
    FtpClientPtr client = nullptr;
    for (int i = 0; i < ftpUrlCount; ++i)
        {
        try
            {
            std::cout << std::endl << "*****************" << std::endl;
            std::cout << "Connecting to FTP" << std::endl;
            std::cout << "*****************" << std::endl << std::endl;

            client = FtpClientPtr(FtpClient::ConnectTo((Utf8CP)ftpUrls[i].c_str(), (Utf8CP)provider.c_str()));
            if (client == nullptr)
                {
                std::cout << "Status: Could not connect to " << ftpUrls[i].c_str() << std::endl;
                continue;
                }
            std::cout << "Status: Connected to " << ftpUrls[i].c_str();

            std::cout << std::endl << "*****************" << std::endl;
            std::cout << "Retrieving data" << std::endl;
            std::cout << "*****************" << std::endl << std::endl;

            client->SetObserver(new FtpTraversalObserver(updateMode, dualMode, dbName.c_str(), pwszConnStr.c_str()));
            status = client->GetData();
            if (status != FtpStatus::Success)
                {
                std::cout << "Status: Failed, " << EnumString(status) << std::endl;
                continue;
                }
            }
        catch (int e)
            {
            std::cout << "Status: Exception occured, " << e << std::endl;
            continue;
            }

        std::cout << std::endl << "Press any key to exit" << std::endl;
        getch();
        }
    return 1;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason            	    8/2016
//-------------------------------------------------------------------------------------
FtpTraversalObserver::FtpTraversalObserver(bool updateMode, bool dualMode, const char* dbName, const char* pwszConnStr) : IFtpTraversalObserver(), m_updateMode(updateMode), m_dualMode(dualMode)
    {
    ServerConnection::GetInstance().SetStrings(dbName, pwszConnStr);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason            	    8/2016
//-------------------------------------------------------------------------------------
void FtpTraversalObserver::OnFileListed(bvector<Utf8String>& fileList, Utf8CP file)
    {
    if (nullptr == file)
        {
        std::cout << "Status: Failed, file is null." << std::endl;
        return;
        }

    if (m_updateMode)
        {
        if (!ServerConnection::GetInstance().IsDuplicate(file))
            {
            std::cout << "Status: Skipped " << file << std::endl;
            return;
            }
        }
    else
        {
        if (ServerConnection::GetInstance().IsDuplicate(file))
            {
            std::cout << "Status: Skipped " << file << std::endl;
            return;
            }
        }

    std::cout << "Status: Added " << file << " to queue." << std::endl;
    fileList.push_back(file);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason            	    8/2016
//-------------------------------------------------------------------------------------
void FtpTraversalObserver::OnFileDownloaded(Utf8CP file)
    {
    if (nullptr == file)
        return;

    std::cout << "Status: Downloaded " << file << std::endl;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason            	    8/2016
//-------------------------------------------------------------------------------------
void FtpTraversalObserver::OnDataExtracted(RealityPlatform::FtpDataCR data)
    {
    if (m_updateMode)
        ServerConnection::GetInstance().Update(data);
    else
        ServerConnection::GetInstance().Save(data, m_dualMode);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason            	    8/2016
//-------------------------------------------------------------------------------------
void ServerConnection::Save(FtpDataCR data, bool dualMode)
    {
    CHAR preQuery[512];
    sprintf(preQuery, "SELECT * FROM [%s].[dbo].[SpatialDataSources] WHERE [MainURL] = '%s'", m_dbName, data.GetUrl().c_str());
    if (HasEntries(preQuery))
        return;

    FtpMetadataCR metadata = data.GetMetadata();

    CHAR metadataQuery[2048];
    sprintf(metadataQuery, "INSERT INTO [%s].[dbo].[Metadatas] ([Provenance],[Description],[ContactInformation],[Legal],[RawMetadataFormat],[RawMetadata]) VALUES ('%s', '%s', '%s', '%s', '%s', '')",
        m_dbName,
        metadata.GetProvenance().c_str(),
        metadata.GetDescription().c_str(),
        metadata.GetContactInfo().c_str(),
        metadata.GetLegal().c_str(),
        metadata.GetFormat().c_str());

    RETCODE retCode = ExecuteSQL(metadataQuery);
    ReleaseStmt();
    SQLINTEGER metadataId;
    SQLLEN len;
    
    CHAR tableName[128];
    sprintf(tableName, "[%s].[dbo].[Metadatas]", m_dbName);

    FetchTableIdentity(metadataId, tableName, len);
    
    FtpThumbnailCR thumbnail = data.GetThumbnail();

    const bvector<Byte>& thumbnailBytes = thumbnail.GetData();
    size_t size = thumbnailBytes.size();
    unsigned char* dataArray = new unsigned char[size];
    for (int i = 0; i < size; ++i)
        dataArray[i] = thumbnailBytes[i];

    CHAR thumbnailQuery[1000000];
    sprintf(thumbnailQuery, "INSERT INTO [%s].[dbo].[Thumbnails] ([ThumbnailProvenance], [ThumbnailFormat], [ThumbnailWidth], [ThumbnailHeight], [ThumbnailStamp], [ThumbnailGenerationDetails], [ThumbnailData]) VALUES ('%s', '%s', %d, %d, '%ls', '%s', ?)",
        m_dbName,
        thumbnail.GetProvenance().c_str(),
        thumbnail.GetFormat().c_str(),
        thumbnail.GetWidth(),
        thumbnail.GetHeight(),
        thumbnail.GetStamp().ToString().c_str(),
        thumbnail.GetGenerationDetails().c_str());
    SQLPrepare(hStmt, (SQLCHAR*)thumbnailQuery, SQL_NTS);

    SQLLEN sqlLength = size;
    SQLLEN dateTimeSize = sizeof(SQL_TIMESTAMP_STRUCT);
    
    SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_BINARY, SQL_LONGVARBINARY, size, 0, dataArray, size, &sqlLength);
    
    retCode = ExecuteSQL(hStmt);
    ReleaseStmt();
    SQLINTEGER thumbnailId;
    
    sprintf(tableName, "[%s].[dbo].[Thumbnails]", m_dbName);
    FetchTableIdentity(thumbnailId, tableName, len);
        
    FtpServerCR server = data.GetServer();
    Utf8StringCR url = server.GetUrl();
    SQLINTEGER serverId;
    CHAR serverCheckQuery[512];
    sprintf(serverCheckQuery, "SELECT [Id] FROM [%s].[dbo].[Servers] WHERE [URL] = '%s'", m_dbName, url.c_str());
    retCode = ExecuteSQL(serverCheckQuery);
    if (retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
        {
        SQLBindCol(hStmt, 1, SQL_INTEGER, &serverId, 2, &len);
        retCode = SQLFetch(hStmt);
        }
    ReleaseStmt();

    if (retCode != SQL_SUCCESS)
        {
        CHAR serverQuery[1024];
        sprintf(serverQuery, "INSERT INTO [%s].[dbo].[Servers] ([CommunicationProtocol], [Name], [URL], [ServerContactInformation], [Legal], [Online], [LastCheck], [LastTimeOnLine], [Latency], [State], [Type], [MeanReachabilityStats]) VALUES ('%s', '%s', '%s', '%s', '%s', %d, ?, ?, %f, '%s', '%s', 0)",
            m_dbName,
            server.GetProtocol().c_str(),
            server.GetName().c_str(),
            url.c_str(),
            server.GetContactInfo().c_str(),
            server.GetLegal().c_str(),
            server.IsOnline(),
            server.GetLatency(),
            server.GetState().c_str(),
            server.GetType().c_str());

        SQLPrepare(hStmt, (SQLCHAR*)serverQuery, SQL_NTS);

        SQL_TIMESTAMP_STRUCT checkTime = PackageDateTime(server.GetLastCheck());
        SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_TIMESTAMP, SQL_TYPE_TIMESTAMP, 23, 3, &checkTime, dateTimeSize, 0);
        SQL_TIMESTAMP_STRUCT onlineTime = PackageDateTime(server.GetLastTimeOnline());
        SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_TIMESTAMP, SQL_TYPE_TIMESTAMP, 23, 3, &onlineTime, dateTimeSize, 0);

        retCode = ExecuteSQL(hStmt);
        ReleaseStmt();

        sprintf(tableName, "[%s].[dbo].[Servers]", m_dbName);
        FetchTableIdentity(serverId, tableName, len);
        }
    
    CHAR existingEntityBaseQuery[512];
    sprintf(existingEntityBaseQuery, "SELECT [Id] FROM [%s].[dbo].[SpatialEntityBases] WHERE [Name] = '%s'", m_dbName, data.GetName().c_str());
    retCode = ExecuteSQL(existingEntityBaseQuery);
    bool hasExisting = false;
    SQLINTEGER entityId;
    if (retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
        {
        SQLBindCol(hStmt, 1, SQL_INTEGER, &entityId, 2, &len);
        retCode = SQLFetch(hStmt);

        hasExisting = (retCode == SQL_SUCCESS);
        }
    ReleaseStmt();

    CHAR entityBaseQuery[3000];
    sprintf(entityBaseQuery, "INSERT INTO [%s].[dbo].[SpatialEntityBases] ([Name], [ResolutionInMeters], [DataProvider], [DataProviderName], [Footprint], [Date], [Metadata_Id], [Thumbnail_Id]) VALUES ('%s', '%s', '%s', '%s', geometry::STPolyFromText(?, 0), ?, %d, %d)",
        m_dbName,
        data.GetName().c_str(),
        data.GetResolution().c_str(),
        data.GetProvider().c_str(),
        data.GetProvider().c_str(),
        metadataId,
        thumbnailId);

    SQLPrepare(hStmt, (SQLCHAR*)entityBaseQuery, SQL_NTS);

    DRange2dCR Fpt = data.GetFootprint();
    double xMin = std::min(Fpt.low.x, Fpt.high.x);
    double xMax = std::max(Fpt.low.x, Fpt.high.x);
    double yMin = std::min(Fpt.low.y, Fpt.high.y);
    double yMax = std::max(Fpt.low.y, Fpt.high.y);
    char polygon[2000];
    sprintf(polygon, "POLYGON((%f %f, %f %f, %f %f, %f %f, %f %f))", xMax, yMax, xMax, yMin, xMin, yMin, xMin, yMax, xMax, yMax);
    retCode = SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_LONGVARCHAR, strlen(polygon), 0, (SQLPOINTER)polygon, strlen(polygon), NULL);
    
    DateTimeCR date = data.GetDate(); 
    CHAR baseDate[10];
    sprintf(baseDate, "%d-%d-%d", date.GetYear(), date.GetMonth(), date.GetDay());
    TryODBC(hStmt, SQL_HANDLE_STMT, SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, strlen(baseDate), 0, (SQLPOINTER)baseDate, strlen(baseDate), NULL));

    ExecuteSQL(hStmt);
    ReleaseStmt();

    if(!hasExisting)
        {
        sprintf(tableName, "[%s].[dbo].[SpatialEntityBases]", m_dbName);
        FetchTableIdentity(entityId, tableName, len);
        }

    SQLINTEGER dataSize = (int)data.GetSize();
    CHAR spatialDataSourceQuery[512];
    sprintf(spatialDataSourceQuery, "INSERT INTO [%s].[dbo].[SpatialDataSources] ([MainURL], [CompoundType], [FileSize], [DataSourceType], [LocationInCompound], [Server_Id]) VALUES ('%s', '%s', %d, '%s', '%s', %d)",
        m_dbName,
        data.GetUrl().c_str(),
        data.GetCompoundType().c_str(),
        dataSize,
        data.GetDataType().c_str(),
        data.GetLocationInCompound().c_str(),
        serverId);
    
    retCode = ExecuteSQL(spatialDataSourceQuery);
    ReleaseStmt();
    SQLINTEGER dataSourceId;
    
    sprintf(tableName, "[%s].[dbo].[SpatialDataSources]", m_dbName);
    FetchTableIdentity(dataSourceId, tableName, len);

    ReleaseStmt();

    if (dualMode && hasExisting)
        {
        CHAR existingEntityQuery[512];
        sprintf(existingEntityQuery, "SELECT * FROM [%s].[dbo].[SpatialEntities] WHERE [Id] = %d", m_dbName, entityId);
        
        if (HasEntries(existingEntityQuery))
            {
            CHAR existingSourceQuery[512];
            sprintf(existingSourceQuery, "INSERT INTO [%s].[dbo].[SpatialEntitySpatialDataSources] ([SpatialEntity_Id], [SpatialDataSource_Id]) VALUES (%d, %d)",
                m_dbName,
                entityId,
                dataSourceId);

            ExecuteSQL(existingSourceQuery);
            ReleaseStmt();
            }
        }
    else
        {
        CHAR entityQuery[255];
        sprintf(entityQuery, "IF NOT EXISTS(SELECT * FROM [%s].[dbo].[SpatialEntities] WHERE [Id] = %d) INSERT INTO [%s].[dbo].[SpatialEntities] ([Id]) VALUES (%d)",
            m_dbName,
            entityId,
            m_dbName,
            entityId);
        ExecuteSQL(entityQuery);
        ReleaseStmt();

        CHAR existingSourceQuery[512];
        sprintf(existingSourceQuery, "IF NOT EXISTS(SELECT * FROM [%s].[dbo].[SpatialEntitySpatialDataSources] WHERE [SpatialEntity_Id] = %d AND [SpatialDataSource_Id] = %d) INSERT INTO [%s].[dbo].[SpatialEntitySpatialDataSources] ([SpatialEntity_Id], [SpatialDataSource_Id]) VALUES (%d, %d)",
            m_dbName,
            entityId,
            dataSourceId,
            m_dbName,
            entityId,
            dataSourceId);

        ExecuteSQL(existingSourceQuery);
        ReleaseStmt();
        }

    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason            	    8/2016
//-------------------------------------------------------------------------------------
void ServerConnection::Update(FtpDataCR data)
    {
    CHAR preQuery[512]; 
    sprintf(preQuery, "SELECT [Id] FROM [%s].[dbo].[SpatialDataSources] WHERE [MainURL] = '%s'", m_dbName, data.GetUrl().c_str());
    RETCODE retCode = ExecuteSQL(preQuery);
    SQLINTEGER sourceId;
    SQLLEN len;
    if (retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
        {
        SQLBindCol(hStmt, 1, SQL_INTEGER, &sourceId, 2, &len);
        retCode = SQLFetch(hStmt);

        if (retCode != SQL_SUCCESS)
            return;
        }
    else
        return;

    ReleaseStmt();

    const char* url = data.GetUrl().c_str();
    CHAR sourceQuery[512];
    sprintf(sourceQuery, "UPDATE [%s].[dbo].[SpatialDataSources] SET [MainURL] = '%s', [CompoundType] = '%s', [FileSize] = %d, [DataSourceType] = '%s', [LocationInCompound] = '%s' WHERE [MainUrl] = '%s'",
        m_dbName,
        url,
        data.GetCompoundType().c_str(),
        (int)data.GetSize(),
        data.GetDataType().c_str(),
        data.GetLocationInCompound().c_str(),
        url);
    ExecuteSQL(sourceQuery);
    ReleaseStmt();

    CHAR serverIdQuery[512];
    sprintf(serverIdQuery, "SELECT [Server_Id] FROM [%s].[dbo].[SpatialDataSources] WHERE [MainURL] = '%s'", m_dbName, url);
    ExecuteSQL(serverIdQuery);
    SQLINTEGER serverId;
    SQLBindCol(hStmt, 1, SQL_INTEGER, &serverId, 2, &len);
    TryODBC(hStmt, SQL_HANDLE_STMT, SQLFetch(hStmt));
    ReleaseStmt();

    FtpServerCR server = data.GetServer();

    CHAR serverQuery[512];
    sprintf(serverQuery, "UPDATE [%s].[dbo].[Servers] SET [CommunicationProtocol] = '%s', [Name] = '%s', [URL] = '%s', [ServerContactInformation] = '%s', [Legal] = '%s', [Online] = %d, [LastCheck] = ?, [LastTimeOnline] = ?, [Latency] = %f, [State] = '%s', [Type] = '%s' WHERE [Id] = %d",
        m_dbName,
        server.GetProtocol().c_str(),
        server.GetName().c_str(),
        server.GetUrl().c_str(),
        server.GetContactInfo().c_str(),
        server.GetLegal().c_str(),
        server.IsOnline(),
        server.GetLatency(),
        server.GetState().c_str(),
        server.GetType().c_str(),
        serverId);
    SQLPrepare(hStmt, (SQLCHAR*)serverQuery, SQL_NTS);
    SQLLEN dateSize = sizeof(SQL_TIMESTAMP_STRUCT);
    SQL_TIMESTAMP_STRUCT checkTime = PackageDateTime(server.GetLastCheck());
    SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_TIMESTAMP, SQL_TYPE_TIMESTAMP, 23, 3, &checkTime, dateSize, 0);
    SQL_TIMESTAMP_STRUCT onlineTime = PackageDateTime(server.GetLastTimeOnline());
    SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_TIMESTAMP, SQL_TYPE_TIMESTAMP, 23, 3, &onlineTime, dateSize, 0);
    ExecuteSQL(hStmt);
    ReleaseStmt();

    SQLINTEGER entityId;
    CHAR entityIdQuery[256];
    sprintf(entityIdQuery, "SELECT [SpatialEntity_Id] FROM [%s].[dbo].[SpatialEntitySpatialDataSources] WHERE [SpatialDataSource_id] = %d",
        m_dbName,
        sourceId);
    ExecuteSQL(entityIdQuery);
    SQLBindCol(hStmt, 1, SQL_INTEGER, &entityId, 2, &len);
    TryODBC(hStmt, SQL_HANDLE_STMT, SQLFetch(hStmt));
    ReleaseStmt();

    DateTimeCR date = data.GetDate();
    CHAR entityBaseQuery[3000];
    sprintf(entityBaseQuery, "UPDATE [%s].[dbo].[SpatialEntityBases] SET [Name] = '%s', [ResolutionInMeters] = '%s', [DataProvider] = '%s', [DataProviderName] = '%s', [Footprint] = geometry::STPolyFromText(?, 0), [Date] = '%d-%d-%d' WHERE [Id] = %d",
        m_dbName,
        data.GetName().c_str(),
        data.GetResolution().c_str(),
        data.GetProvider().c_str(),
        data.GetProvider().c_str(),
        date.GetYear(), 
        date.GetMonth(), 
        date.GetDay(),
        entityId);
    SQLPrepare(hStmt, (SQLCHAR*) entityBaseQuery, SQL_NTS);
    
    DRange2dCR Fpt = data.GetFootprint();
    double xMin = std::min(Fpt.low.x, Fpt.high.x);
    double xMax = std::max(Fpt.low.x, Fpt.high.x);
    double yMin = std::min(Fpt.low.y, Fpt.high.y);
    double yMax = std::max(Fpt.low.y, Fpt.high.y);
    char polygon[2000];
    sprintf(polygon, "POLYGON((%f %f, %f %f, %f %f, %f %f, %f %f))", xMax, yMax, xMax, yMin, xMin, yMin, xMin, yMax, xMax, yMax);
    SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_LONGVARCHAR, strlen(polygon), 0, (SQLPOINTER)polygon, strlen(polygon), NULL);
    
    ExecuteSQL(hStmt);
    ReleaseStmt();

    SQLINTEGER metadataId;
    CHAR metaIdQuery[256];
    sprintf(metaIdQuery, "SELECT [Metadata_Id] FROM [%s].[dbo].[SpatialEntityBases] WHERE [Id] = %d", m_dbName, entityId);
    ExecuteSQL(metaIdQuery);
    SQLBindCol(hStmt, 1, SQL_INTEGER, &metadataId, 2, &len);
    TryODBC(hStmt, SQL_HANDLE_STMT, SQLFetch(hStmt));
    ReleaseStmt();
    
    FtpMetadataCR metadata = data.GetMetadata();
    CHAR metadataQuery[512];
    sprintf(metadataQuery, "UPDATE [%s].[dbo].[Metadatas] SET [Provenance] = '%s', [Description] = '%s', [ContactInformation] = '%s', [Legal] = '%s', [RawMetadataFormat] = '%s', [RawMetadata] = '' WHERE [Id] = %d",
        m_dbName,
        metadata.GetProvenance().c_str(),
        metadata.GetDescription().c_str(),
        metadata.GetContactInfo().c_str(),
        metadata.GetLegal().c_str(),
        metadata.GetFormat().c_str(),
        metadataId);
    
    ExecuteSQL(metadataQuery);
    ReleaseStmt();

    SQLINTEGER thumbnailId;
    CHAR thumbIdQuery[256];
    sprintf(thumbIdQuery, "SELECT [Thumbnail_Id] FROM [%s].[dbo].[SpatialEntityBases] WHERE [Id] = %d", m_dbName, entityId);
    ExecuteSQL(thumbIdQuery);
    SQLBindCol(hStmt, 1, SQL_INTEGER, &thumbnailId, 2, &len);
    TryODBC(hStmt, SQL_HANDLE_STMT, SQLFetch(hStmt));
    ReleaseStmt();

    FtpThumbnailCR thumbnail = data.GetThumbnail();
    CHAR thumbQuery[100000];
    sprintf(thumbQuery, "UPDATE [%s].[dbo].[Thumbnails] SET [ThumbnailProvenance] = '%s', [ThumbnailFormat] = '%s', [ThumbnailWidth] = %d, [ThumbnailHeight] = %d, [ThumbnailStamp] = ?, [ThumbnailGenerationDetails] = '%s', [ThumbnailData] = ? WHERE [Id] = %d",
        m_dbName,
        thumbnail.GetProvenance().c_str(),
        thumbnail.GetFormat().c_str(),
        thumbnail.GetWidth(),
        thumbnail.GetHeight(),
        thumbnail.GetGenerationDetails().c_str(),
        thumbnailId);
    SQLPrepare(hStmt, (SQLCHAR*)thumbQuery, SQL_NTS);
    
    SQL_TIMESTAMP_STRUCT stampTime = PackageDateTime(thumbnail.GetStamp());
    SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_TIMESTAMP, SQL_TYPE_TIMESTAMP, 23, 3, &stampTime, dateSize, 0);
    
    const bvector<Byte>& thumbnailBytes = thumbnail.GetData();
    size_t size = thumbnailBytes.size();
    unsigned char* dataArray = new unsigned char[size];
    for (int i = 0; i < size; ++i)
        dataArray[i] = thumbnailBytes[i];
    SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_BINARY, SQL_LONGVARBINARY, size, 0, dataArray, size, &len);
    ExecuteSQL(hStmt);
    ReleaseStmt();
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason            	    8/2016
//-------------------------------------------------------------------------------------
SQL_TIMESTAMP_STRUCT ServerConnection::PackageDateTime(DateTimeCR date)
    {
    SQL_TIMESTAMP_STRUCT datetime;
    datetime.year = date.GetYear();
    datetime.month = date.GetMonth();
    datetime.day = date.GetDay();
    datetime.hour = date.GetHour();
    datetime.minute = date.GetMinute();
    datetime.second = date.GetSecond();
    //datetime.fraction = date.GetMillisecond();
    datetime.fraction = 0;
    /*while (datetime.fraction > 999)
    {
        datetime.fraction /= 10; //keep only the 3 most significant number
    }*/

    return datetime;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason            	    8/2016
//-------------------------------------------------------------------------------------
SQL_DATE_STRUCT ServerConnection::PackageDate(DateTimeCR dateTime)
    {
    SQL_DATE_STRUCT date;
    date.day = dateTime.GetDay();
    date.month = dateTime.GetMonth();
    date.year = dateTime.GetYear();
    
    return date;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason            	    8/2016
//-------------------------------------------------------------------------------------
bool ServerConnection::IsDuplicate(Utf8CP file)
    {
    CHAR wszInput[512];
    sprintf(wszInput, "SELECT * FROM [%s].[dbo].[SpatialDataSources] WHERE [MainURL] = '%s'", m_dbName, file);
    
    return HasEntries(wszInput);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason            	    8/2016
//-------------------------------------------------------------------------------------
bool ServerConnection::IsMirror(Utf8CP file)
    {
    /*size_t lastPos = file.find_last_of("/\\");
    CHAR wszInput[512];
    sprintf(wszInput, "SELECT * FROM [%s].[dbo].[SpatialDataSources] WHERE CONTAINS([MainURL], '%s'", m_dbName, file.substr(lastPos + 1));

        return HasEntries(wszInput);*/
        return false; //function is never called
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason            	    8/2016
//-------------------------------------------------------------------------------------
bool ServerConnection::HasEntries(CHAR* input)
    {
    RETCODE retCode = ExecuteSQL(input);
    bool hasEntries = false;
    if (retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
        {
        retCode = SQLFetch(hStmt);

        if (retCode == SQL_SUCCESS)
            hasEntries = true;
        }
    ReleaseStmt();
    return hasEntries;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason            	    8/2016
//-------------------------------------------------------------------------------------
void ServerConnection::Exit()
    {

    // Free ODBC handles and exit

    if (hStmt)
        {
        SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
        }

    if (hDbc)
        {
        SQLDisconnect(hDbc);
        SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
        }

    if (hEnv)
        {
        SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
        }

    wprintf(L"\nOperation failed, database disconnected. \nPress any key to exit...");
    getch();
    exit(-1);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason            	    8/2016
//-------------------------------------------------------------------------------------
void ServerConnection::HandleDiagnosticRecord (SQLHANDLE      hHandle,    
                             SQLSMALLINT    hType,  
                             RETCODE        RetCode)
    {
    SQLCHAR     SqlState[6], Msg[SQL_MAX_MESSAGE_LENGTH];
    SQLINTEGER  NativeError;
    SQLSMALLINT i, MsgLen;

    if (RetCode == SQL_INVALID_HANDLE)
    {
        fwprintf(stderr, L"Invalid handle!\n");
        return;
    }
    i = 1;
    while (SQLGetDiagRec(hType,
                         hHandle,
                         i,
                         SqlState,
                         &NativeError,
                         Msg,
                         sizeof(Msg),
                         &MsgLen) != SQL_NO_DATA)
    {
        if (strncmp((CHAR*)SqlState, "01004", 5))
        {
            fwprintf(stderr, L"\n%hs %hs (%d)\n", SqlState, Msg, NativeError);
        }
            i++;
    }

    }

END_BENTLEY_REALITYPLATFORM_NAMESPACE

int main(int argc, char* argv[])
{
    return RealityPlatform::main(argc, argv);
}


