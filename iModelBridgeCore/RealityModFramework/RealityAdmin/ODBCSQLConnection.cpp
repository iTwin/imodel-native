/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityAdmin/ODBCSQLConnection.cpp $
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
#include <RealityAdmin/ODBCSQLConnection.h>

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
        {
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
// @bsimethod                                   Spencer.Mason            	    9/2016
//-------------------------------------------------------------------------------------
SQLINTEGER ServerConnection::SaveServer(SpatialEntityServerCR server)
    {
    CHAR preQuery[512];
    sprintf(preQuery, "SELECT * FROM [%s].[dbo].[Servers] WHERE [URL] = '%s'", m_dbName, server.GetUrl().c_str());
    CHAR serverStatement[1024];
    if (HasEntries(preQuery))
        {
        sprintf(serverStatement, "UPDATE [%s].[dbo].[Servers] SET [LastCheck] = ?, [LastTimeOnline] = ? WHERE [URL] = '%s'",
            m_dbName,
            server.GetUrl().c_str());
        }
    else
        {
        CHAR tempStatement[1024];
        CHAR tempStatement2[128];
        sprintf(tempStatement, "INSERT INTO [%s].[dbo].[Servers] ([CommunicationProtocol], [Name], [URL], [Online], [LastCheck], [LastTimeOnline]%s) VALUES ('%s', '%s', '%s', %d, ?, ?%s)",
            m_dbName,
            "%s",
            server.GetProtocol().c_str(),
            server.GetName().c_str(),
            server.GetUrl().c_str(),
            server.IsOnline(),
            "%s");

        if(server.GetContactInfo().length() > 0)
            {
            sprintf(tempStatement2, ", '%s'%s", server.GetContactInfo().c_str(), "%s");
            sprintf(serverStatement, tempStatement, ", [ServerContactInformation]%s", tempStatement2);
            memcpy(tempStatement, serverStatement, strlen(serverStatement)+1);
            }

        if (server.GetLegal().length() > 0)
            {
            sprintf(tempStatement2, ", '%s'%s", server.GetLegal().c_str(), "%s");
            sprintf(serverStatement, tempStatement, ", [Legal]%s", tempStatement2);
            memcpy(tempStatement, serverStatement, strlen(serverStatement) + 1);
            }
            
        if (server.GetLatency() > 0)
            {
            sprintf(tempStatement2, ", %f%s", server.GetLatency(), "%s");
            sprintf(serverStatement, tempStatement, ", [Latency]%s", tempStatement2);
            memcpy(tempStatement, serverStatement, strlen(serverStatement) + 1);
            }

        if (server.GetState().length() > 0)
            {
            sprintf(tempStatement2, ", '%s'%s", server.GetState().c_str(), "%s");
            sprintf(serverStatement, tempStatement, ", [State]%s", tempStatement2);
            memcpy(tempStatement, serverStatement, strlen(serverStatement) + 1);
            }

        if (server.GetType().length() > 0)
            {
            sprintf(tempStatement2, ", '%s'%s", server.GetType().c_str(), "%s");
            sprintf(serverStatement, tempStatement, ", [Type]%s", tempStatement2);
            memcpy(tempStatement, serverStatement, strlen(serverStatement) + 1);
            }

        sprintf(serverStatement, tempStatement, "", ""); //remove trailing '%s'
        }

    SQLPrepare(hStmt, (SQLCHAR*)serverStatement, SQL_NTS);

    SQL_TIMESTAMP_STRUCT checkTime = PackageDateTime(server.GetLastCheck());
    SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_TIMESTAMP, SQL_TYPE_TIMESTAMP, 23, 3, &checkTime, sizeof(SQL_TIMESTAMP_STRUCT), 0);
    SQL_TIMESTAMP_STRUCT onlineTime = PackageDateTime(server.GetLastTimeOnline());
    SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_TIMESTAMP, SQL_TYPE_TIMESTAMP, 23, 3, &onlineTime, sizeof(SQL_TIMESTAMP_STRUCT), 0);

    ExecuteSQL(hStmt);
    ReleaseStmt();

    SQLINTEGER id;
    SQLLEN len;

    CHAR tableName[256];
    sprintf(tableName, "[%s].[dbo].[Servers]", m_dbName);
    FetchTableIdentity(id, tableName, len);

    return id;
    }

bool ServerConnection::CheckExists(Utf8String id)
    {
    CHAR existsQuery[256];
    sprintf(existsQuery, "SELECT * FROM [%s].[dbo].[MultibandSources] WHERE [OriginalId] = '%s'",
        m_dbName,
        id.c_str());
    return HasEntries(existsQuery);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason            	    8/2016
//-------------------------------------------------------------------------------------
void ServerConnection::Save(SpatialEntityDataCR data, bool dualMode)
    {
    CHAR preQuery[512];
    sprintf(preQuery, "SELECT * FROM [%s].[dbo].[SpatialDataSources] WHERE [MainURL] = '%s'", m_dbName, data.GetUrl().c_str());
    if (HasEntries(preQuery))
        return;
    
    SpatialEntityMetadataCR metadata = data.GetMetadata();
    RETCODE retCode;
    
    if (metadata.GetMetadataUrl().length() > 0)
        {
        Utf8String metadataUrl = metadata.GetMetadataUrl();

        CHAR serverStatement[512];
        sprintf(serverStatement, "INSERT INTO [%s].[dbo].[Metadatas] ([Description], [Provenance], [MetadataURL], [Legal]) VALUES ('%s', '%s', '%s', '%s')",
            m_dbName,
            metadata.GetDescription().c_str(),
            metadata.GetProvenance().c_str(),
            metadata.GetMetadataUrl().c_str(),
            metadata.GetLegal().c_str());
        ExecuteSQL(serverStatement);
        ReleaseStmt();
        }
    else
        {
        CHAR* metadataQuery = new CHAR[2048];
        sprintf(metadataQuery, "INSERT INTO [%s].[dbo].[Metadatas] ([Provenance],[Description],[ContactInformation],[Legal],[RawMetadataFormat],[RawMetadata]) VALUES ('%s', '%s', '%s', '%s', '%s', '')",
            m_dbName,
            metadata.GetProvenance().c_str(),
            metadata.GetDescription().c_str(),
            metadata.GetContactInfo().c_str(),
            metadata.GetLegal().c_str(),
            metadata.GetFormat().c_str());

        retCode = ExecuteSQL(metadataQuery);
        ReleaseStmt();
        delete[] metadataQuery;
        }

    SQLINTEGER metadataId;
    SQLLEN len;
    CHAR tableName[128];
    sprintf(tableName, "[%s].[dbo].[Metadatas]", m_dbName);
    FetchTableIdentity(metadataId, tableName, len);

    DateTimeCR date = DateTime::GetCurrentTimeUtc();
    bool thumbnailPresent = false;
    SQLINTEGER thumbnailId = 0;

    SpatialEntityThumbnailCR thumbnail = data.GetThumbnail();

    if(thumbnail.GetThumbnailUrl().length() > 0)
        {
        thumbnailPresent = true;
        Utf8String thumbUrl = thumbnail.GetThumbnailUrl();
        CHAR* thumbnailQuery = new CHAR[1024];
        sprintf(thumbnailQuery, "INSERT INTO [%s].[dbo].[Thumbnails] ([ThumbnailProvenance], [ThumbnailFormat], [ThumbnailStamp], [ThumbnailGenerationDetails], [ThumbnailUrl]) VALUES ('%s', '%s', '%ls', '%s', '%s')",
            m_dbName,
            thumbnail.GetProvenance().c_str(),
            thumbnail.GetFormat().c_str(),
            date.ToString().c_str(),
            thumbnail.GetProvenance().c_str(),
            thumbnail.GetThumbnailUrl().c_str());

        retCode = ExecuteSQL(thumbnailQuery);
        ReleaseStmt();
        delete[] thumbnailQuery;
        }
    else
        {
        const bvector<Byte>& thumbnailBytes = thumbnail.GetData();
        size_t size = thumbnailBytes.size();
        if (0 != size)
            {
            thumbnailPresent = true;
            unsigned char* dataArray = new unsigned char[size];
            for (int i = 0; i < size; ++i)
                dataArray[i] = thumbnailBytes[i];

            CHAR* thumbnailQuery = new CHAR[1048576];
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


            SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_BINARY, SQL_LONGVARBINARY, size, 0, dataArray, size, &sqlLength);

            retCode = ExecuteSQL(hStmt);
            ReleaseStmt();
            delete[] thumbnailQuery;
            delete[] dataArray;
            }
        }

    if(thumbnailPresent)
        {
        sprintf(tableName, "[%s].[dbo].[Thumbnails]", m_dbName);
        FetchTableIdentity(thumbnailId, tableName, len);
        }

    CHAR existingEntityBaseQuery[512];
    sprintf(existingEntityBaseQuery, "SELECT [Id] FROM [%s].[dbo].[SpatialEntityBases] WHERE [Name] = '%s'", m_dbName, data.GetName().c_str());
    retCode = ExecuteSQL(existingEntityBaseQuery);
    bool hasExisting = false;
    SQLINTEGER entityId = 0;
    if (retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
    {
        SQLBindCol(hStmt, 1, SQL_INTEGER, &entityId, 2, &len);
        retCode = SQLFetch(hStmt);

        hasExisting = (retCode == SQL_SUCCESS);
    }
    ReleaseStmt();

    DRange2dCR Fpt = data.GetFootprintExtents();
    double xMin = std::min(Fpt.low.x, Fpt.high.x);
    double xMax = std::max(Fpt.low.x, Fpt.high.x);
    double yMin = std::min(Fpt.low.y, Fpt.high.y);
    double yMax = std::max(Fpt.low.y, Fpt.high.y);


    CHAR* entityBaseQuery = new CHAR[4096];
    CHAR* entityBaseQueryBase = new CHAR[2048];
    CHAR thumbnailIdStr[32];
    sprintf(entityBaseQueryBase, "INSERT INTO [%s].[dbo].[SpatialEntityBases] ([Name], [DataProvider], [DataProviderName], [Dataset], [Footprint], [MinX], [MinY], [MaxX], [MaxY], [Date], [Metadata_Id], [DataSourceTypesAvailable], [ResolutionInMeters], [Classification]%s) VALUES ('%s', '%s', '%s', '%s', geometry::STPolyFromText(?, 4326), %f, %f, %f, %f, ?, %d, '%s', '%s', '%s'%s)",
        m_dbName,
        "%s",
        data.GetName().c_str(),
        data.GetProvider().c_str(),
        data.GetProviderName().c_str(),
        data.GetDataset().c_str(),
        xMin,
        yMin,
        xMax,
        yMax,
        metadataId,
        data.GetDataType().c_str(),
        data.GetResolution().c_str(),
        data.GetClassification().c_str(),
        "%s");

    if(data.GetIsMultiband())
        {
        sprintf(thumbnailIdStr, ", %d", thumbnailId);

        sprintf(entityBaseQuery, entityBaseQueryBase,
            ", [Thumbnail_Id]",
            thumbnailIdStr);
        }
    else
        {
        if(thumbnailPresent)
            {
            sprintf(thumbnailIdStr, "%d", thumbnailId);

            sprintf(entityBaseQuery, entityBaseQueryBase,
                ", [Thumbnail_Id]",
                thumbnailIdStr);
            }
        else
            sprintf(entityBaseQuery, entityBaseQueryBase, "", "");
        }

    SQLPrepare(hStmt, (SQLCHAR*)entityBaseQuery, SQL_NTS);


    char* polygon = new char[2048];
    sprintf(polygon, "POLYGON((%f %f, %f %f, %f %f, %f %f, %f %f))", xMax, yMax, xMax, yMin, xMin, yMin, xMin, yMax, xMax, yMax);
    retCode = SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_LONGVARCHAR, strlen(polygon), 0, (SQLPOINTER)polygon, strlen(polygon), NULL);

        
    CHAR baseDate[32];
    sprintf(baseDate, "%d-%d-%d", date.GetYear(), date.GetMonth(), date.GetDay());
    TryODBC(hStmt, SQL_HANDLE_STMT, SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, strlen(baseDate), 0, (SQLPOINTER)baseDate, strlen(baseDate), NULL));

    ExecuteSQL(hStmt);
    ReleaseStmt();
    delete[] polygon;
    delete[] entityBaseQuery;
    delete[] entityBaseQueryBase;

    if(!hasExisting)
        {
        sprintf(tableName, "[%s].[dbo].[SpatialEntityBases]", m_dbName);
        FetchTableIdentity(entityId, tableName, len);
        }

    CHAR spatialDataSourceQuery[512];
    SQLINTEGER dataSize = (int)data.GetSize();
    SQLINTEGER serverId = (int)data.GetServerId();
    if(serverId < 0)
        serverId = SaveServer(data.GetServer());
    if(data.GetGeoCS().size() == 0)
        {
        if(data.GetCompoundType().length() > 0)
            sprintf(spatialDataSourceQuery, "INSERT INTO [%s].[dbo].[SpatialDataSources] ([MainURL], [CompoundType], [FileSize], [DataSourceType], [LocationInCompound], [Server_Id]) VALUES ('%s', '%s', %d, '%s', '%s', %d)",
                m_dbName,
                data.GetUrl().c_str(),
                data.GetCompoundType().c_str(),
                dataSize,
                data.GetDataType().c_str(),
                data.GetLocationInCompound().c_str(),
                serverId);
        else
            sprintf(spatialDataSourceQuery, "INSERT INTO [%s].[dbo].[SpatialDataSources] ([MainURL], [DataSourceType], [NoDataValue], [FileSize], [Server_Id]) VALUES ('%s', '%s', %d, %f, %d)",
                m_dbName,
                data.GetUrl().c_str(),
                data.GetDataType().c_str(),
                (int)data.GetNoDataValue(),
                data.GetRedBandSize() + data.GetGreenBandSize() + data.GetBlueBandSize() + data.GetPanchromaticBandSize(),
                data.GetServerId());
        }
    else
        sprintf(spatialDataSourceQuery, "INSERT INTO [%s].[dbo].[SpatialDataSources] ([MainURL], [CompoundType], [FileSize], [CoordinateSystem], [DataSourceType], [LocationInCompound], [Server_Id]) VALUES ('%s', '%s', %d, '%s', '%s', '%s', %d)",
            m_dbName,
            data.GetUrl().c_str(),
            data.GetCompoundType().c_str(),
            dataSize,
            data.GetGeoCS().c_str(),
            data.GetDataType().c_str(),
            data.GetLocationInCompound().c_str(),
            serverId);

    retCode = ExecuteSQL(spatialDataSourceQuery);
    ReleaseStmt();
    SQLINTEGER dataSourceId;

    sprintf(tableName, "[%s].[dbo].[SpatialDataSources]", m_dbName);
    FetchTableIdentity(dataSourceId, tableName, len);
    ReleaseStmt();
        
    CHAR entityQuery[256];
    if(data.GetCloudCover() >= 0.0f)
        sprintf(entityQuery, "INSERT INTO [%s].[dbo].[SpatialEntities] ([Id], [Occlusion]) VALUES (%d, %f)",
            m_dbName,
            entityId,
            data.GetCloudCover());
    else
        sprintf(entityQuery, "INSERT INTO [%s].[dbo].[SpatialEntities] ([Id]) VALUES (%d)",
            m_dbName,
            entityId);
    ExecuteSQL(entityQuery);
    ReleaseStmt();

    CHAR existingSourceQuery[512];
    sprintf(existingSourceQuery, "INSERT INTO [%s].[dbo].[SpatialEntitySpatialDataSources] ([SpatialEntity_Id], [SpatialDataSource_Id]) VALUES (%d, %d)",
        m_dbName,
        entityId,
        dataSourceId);

    ExecuteSQL(existingSourceQuery);
    ReleaseStmt();
                
    if(data.GetIsMultiband())
        {
        Utf8String blueUrl;
        Utf8String greenUrl;
        Utf8String redUrl;
        Utf8String panUrl;
        data.GetMultibandUrls(redUrl, greenUrl, blueUrl, panUrl);

        CHAR* multiBQuery = new CHAR[1024];
        sprintf(multiBQuery, "INSERT INTO [%s].[dbo].[MultibandSources] ([Id], [OriginalId], [RedBandURL], [RedBandFileSize], [GreenBandURL], [GreenBandFileSize], [BlueBandURL], [BlueBandFileSize], [PanchromaticBandURL], [PanchromaticBandFileSize]) VALUES ( %d, '%s', '%s', %f, '%s', %f, '%s', %f, '%s', %f)",
            m_dbName,
            dataSourceId,
            data.GetName().c_str(),
            redUrl.c_str(),
            data.GetRedBandSize(),
            greenUrl.c_str(),
            data.GetGreenBandSize(),
            blueUrl.c_str(),
            data.GetBlueBandSize(),
            panUrl.c_str(),
            data.GetPanchromaticBandSize());

        ExecuteSQL(multiBQuery);
        ReleaseStmt();
        delete[] multiBQuery;
        }
    }


//-------------------------------------------------------------------------------------
// @bsimethod                                   Spencer.Mason            	    8/2016
//-------------------------------------------------------------------------------------
void ServerConnection::Update(SpatialEntityDataCR data)
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
    if (data.GetGeoCS().size() == 0)
        sprintf(sourceQuery, "UPDATE [%s].[dbo].[SpatialDataSources] SET [MainURL] = '%s', [CompoundType] = '%s', [FileSize] = %d, [DataSourceType] = '%s', [LocationInCompound] = '%s' WHERE [MainUrl] = '%s'",
            m_dbName,
            url,
            data.GetCompoundType().c_str(),
            (int)data.GetSize(),
            data.GetDataType().c_str(),
            data.GetLocationInCompound().c_str(),
            url);
    else
        sprintf(sourceQuery, "UPDATE [%s].[dbo].[SpatialDataSources] SET [MainURL] = '%s', [CompoundType] = '%s', [FileSize] = %d, [CoordinateSystem] = '%s', [DataSourceType] = '%s', [LocationInCompound] = '%s' WHERE [MainUrl] = '%s'",
            m_dbName,
            url,
            data.GetCompoundType().c_str(),
            (int)data.GetSize(),
            data.GetGeoCS().c_str(),
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

    SpatialEntityServerCR server = data.GetServer();

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

    DRange2dCR Fpt = data.GetFootprintExtents();
    double xMin = std::min(Fpt.low.x, Fpt.high.x);
    double xMax = std::max(Fpt.low.x, Fpt.high.x);
    double yMin = std::min(Fpt.low.y, Fpt.high.y);
    double yMax = std::max(Fpt.low.y, Fpt.high.y);

    DateTimeCR date = data.GetDate();
    CHAR* entityBaseQuery = new CHAR[4096];
    sprintf(entityBaseQuery, "UPDATE [%s].[dbo].[SpatialEntityBases] SET [Name] = '%s', [ResolutionInMeters] = '%s', [DataProvider] = '%s', [DataProviderName] = '%s', [Dataset] = '%s', [Classification] = '%s', [Footprint] = geometry::STPolyFromText(?, 4326), [MinX] = %f, [MinY] = %f, [MaxX] = %f, [MaxY] = %f, [Date] = '%d-%d-%d' , [DataSourceTypesAvailable] = '%s' WHERE [Id] = %d",
        m_dbName,
        data.GetName().c_str(),
        data.GetResolution().c_str(),
        data.GetProvider().c_str(),
        data.GetProvider().c_str(),
        data.GetDataset().c_str(),
        data.GetClassification().c_str(),
        xMin,
        yMin,
        xMax,
        yMax,
        date.GetYear(),
        date.GetMonth(),
        date.GetDay(),
        data.GetDataType().c_str(),
        entityId);
    SQLPrepare(hStmt, (SQLCHAR*)entityBaseQuery, SQL_NTS);

    char* polygon = new char[2048];
    sprintf(polygon, "POLYGON((%f %f, %f %f, %f %f, %f %f, %f %f))", xMax, yMax, xMax, yMin, xMin, yMin, xMin, yMax, xMax, yMax);
    SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_LONGVARCHAR, strlen(polygon), 0, (SQLPOINTER)polygon, strlen(polygon), NULL);

    ExecuteSQL(hStmt);
    ReleaseStmt();
    delete[] entityBaseQuery;
    delete[] polygon;

    SQLINTEGER metadataId;
    CHAR metaIdQuery[256];
    sprintf(metaIdQuery, "SELECT [Metadata_Id] FROM [%s].[dbo].[SpatialEntityBases] WHERE [Id] = %d", m_dbName, entityId);
    ExecuteSQL(metaIdQuery);
    SQLBindCol(hStmt, 1, SQL_INTEGER, &metadataId, 2, &len);
    TryODBC(hStmt, SQL_HANDLE_STMT, SQLFetch(hStmt));
    ReleaseStmt();

    SpatialEntityMetadataCR metadata = data.GetMetadata();
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

    SpatialEntityThumbnailCR thumbnail = data.GetThumbnail();
    const bvector<Byte>& thumbnailBytes = thumbnail.GetData();
    size_t size = thumbnailBytes.size();

    if (size != 0)
        {
        CHAR* thumbQuery = new CHAR[1048576];
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


        unsigned char* dataArray = new unsigned char[size];
        for (int i = 0; i < size; ++i)
            dataArray[i] = thumbnailBytes[i];
        SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_BINARY, SQL_LONGVARBINARY, size, 0, dataArray, size, &len);
        ExecuteSQL(hStmt);
        ReleaseStmt();
        delete[] dataArray;
        delete[] thumbQuery;
        }
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
void ServerConnection::HandleDiagnosticRecord(SQLHANDLE      hHandle,
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